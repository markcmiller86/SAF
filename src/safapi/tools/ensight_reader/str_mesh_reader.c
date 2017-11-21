/* 
 * Copyright(C) 1999-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 1999-2005 Sandia Corporation.  
 *     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
 *     on behalf of the U.S. Government.  Export  of this program may require a license from the United
 *     States Government.
 * 
 * Disclaimer:
 *     This document was  prepared as an account of  work sponsored by an agency  of  the United States
 *     Government. Neither the United States  Government nor the United States Department of Energy nor
 *     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any
 *     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the
 *     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process
 *     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference
 *     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark,
 *     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement,
 *     recommendation, or favoring by the  United States Government   or the University of  California.
 *     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
 *     the  United  States Government or  the   University of California   and shall  not be  used  for
 *     advertising or product endorsement purposes.
 * 
 * 
 * Active Developers:
 *     Peter K. Espen              SNL
 *     Eric A. Illescas            SNL
 *     Jake S. Jones               SNL
 *     Robb P. Matzke              LLNL
 *     Greg Sjaardema              SNL
 * 
 * Inactive Developers:
 *     William J. Arrighi          LLNL
 *     Ray T. Hitt                 SNL
 *     Mark C. Miller              LLNL
 *     Matthew O'Brien             LLNL
 *     James F. Reus               LLNL
 *     Larry A. Schoof             SNL
 * 
 * Acknowledgements:
 *     Marty L. Barnaby            SNL - Red parallel perf. study/tuning
 *     David M. Butler             LPS - Data model design/implementation Spec.
 *     Albert K. Cheng             NCSA - Parallel HDF5 support
 *     Nancy Collins               IBM - Alpha/Beta user
 *     Linnea M. Cook              LLNL - Management advocate
 *     Michael J. Folk             NCSA - Management advocate 
 *     Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
 *     Wilbur R. Johnson           SNL - Early developer
 *     Quincey Koziol              NCSA - Serial HDF5 Support 
 *     Celeste M. Matarazzo        LLNL - Management advocate
 *     Tyce T. McLarty             LLNL - parallel perf. study/tuning
 *     Tom H. Robey                SNL - Early developer
 *     Reinhard W. Stotzer         SNL - Early developer
 *     Judy Sturtevant             SNL - Red parallel perf. study/tuning 
 *     Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
 * 
 */ 
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Description:
 *
 * This file contains a set of functions that can be used to read structured mesh data
 * for display in a visualization tool. 
 *
 * Note that MY_PRECISION (the reader's desired data precision) is defined here as float,
 * for Ensight's sake.
 *
 * C vs C++:    This file (str_mesh_reader.c) can be compiled in C or C++. In C, the global variables
 * are all members of the single instance of the struct pointed to by g_strGlobals. The code is used
 * by first calling init_str_mesh_reader, then other functions, and finally finalize_str_mesh_reader.
 * Because there is only one instance of g_strGlobals, only one database (per process) can be opened
 * at a time (note that Ensight seems to spawn new processes for each database, so it is not
 * an issue). In C++ however, there is a class SAFStrMeshReader, which has one member variable: a 
 * pointer to a structure named g_strGlobals. Because of this, functions will access global
 * variables the same way in C and C++, e.g. g_strGlobals->m_variable. So in C++, the code is 
 * used by first creating a SAFStrMeshReader instance, then calling the method init_str_mesh_reader,
 * etc. Any number of SAFStrMeshReader objects can be created, up to any limits imposed by the SAF
 * and HDF5 libraries themselves.
 *
 * Note that global variables are prepended by g_, member variables by m_, local variables l_, and 
 * argument variables by a_.
 *--------------------------------------------------------------------------------------------------- */

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Introduction
 * Description:
 *
 * This section contains a description of the code used to create the Ensight reader and
 * three other tools: fake_ensight, create_str_mesh and read_str_mesh. This code can be (and
 * has been) used as a basis for creating other SAF readers and tools. Its purpose is to read
 * field, set and relation data from the database (SAF information), and then translate it into 
 * element blocks and variables (visualizer information). 
 *
 * There are many ways to describe the same dataset in SAF, and this reader has been
 * designed with that in mind. The most common variation in how a database is written is probably 
 * in where the nodes are stored. One SAF database writing program might store all nodes
 * on a single SAF_Set, and then link all of the element blocks (other sets) to it with
 * topology relations. Another program might instead store each element block's nodes on the same
 * set as the block. And yet other programs might do something in between, e.g. store 
 * the nodes for each group of element blocks (one group per processor) in individual node sets.
 * This reader has been designed to be able to understand all of these different ways of 
 * locating nodes. There are many other ways SAF database writing styles can vary,
 * in addition to where the nodes are stored. This reader attempts to handle them with varying 
 * degrees of effectiveness.
 *
 * One of the questions that a reader must answer, when translating from set and relation data 
 * (SAF information) to element blocks and variables (visualizer information), is which fields
 * correspond to which variables. For example, one simulation program might write out many
 * SAF fields with the same name, "pressure", with the intent that each field represents
 * the same idea at different points in time, i.e. each "pressure" field is part of what the
 * reader should call the "pressure" variable. This would seem to be the easiest case
 * for the reader to handle, unless of course the database has other unrelated "pressure"
 * fields for some reason. Another example would be the simulation program that gives
 * each field a unique name, e.g. "pressure_timestep_0_block_5". This case would be more 
 * difficult for the reader to solve. The Ensight reader code attempts to use a combination of 
 * factors (including the field name, units, quantity and location) to determine the best 
 * correspondence of fields to variables. This part of the reader code currently has some 
 * outstanding issues, and does not handle all cases as well as desired.
 *
 * The ultimate goal for this reader would be that it could take any correctly written SAF 
 * database and interpret it correctly as the intended set of element blocks and variables. The
 * reader has not reached this goal yet, but it will continue to be improved to widen
 * the group of SAF database styles it can understand.
 * 
 * In addition to the reader code, there are also three related tools documented in this section:
 * fake_ensight, create_str_mesh and read_str_mesh. The first tool exercises all of the Ensight
 * reader code without actually running Ensight, and prints results to stdout. The second tool
 * creates a test SAF database containing just structured mesh elements and node clouds. The third
 * tool, read_str_mesh, opens a SAF database, and reads and prints information about any structured
 * mesh objects in the database.
 *--------------------------------------------------------------------------------------------------- */

                                                                                                                                               
/*
 * HAVE_MPI_CPP should be defined by mpich's mpiCC, and LAM_IS_COMPILING_CPP by lam's mpiCC
 * (note that I have only checked mpich). We need to include mpi.h here because it
 * will be included later from saf.h, but inside of an extern "C". If that happens,
 * then there will be a compile error like "mpi2c++_map.h:36: template with C linkage"
 * because using mpiCC causes some C++-only files to be included. Including the
 * header here precludes it from happening later (inside of an extern "C").
 */
#ifdef HAVE_MPI_CPP
#   include <mpi.h>
#else
#ifdef LAM_IS_COMPILING_CPP
#   include <mpi.h>
#endif
#endif

#include <math.h>
#include <stdarg.h>
#include <str_mesh_reader.h>
#include <variable_names.h>


/*
**Added these two functions because sslib doesnt have them, like old saf.
*/
hbool_t my_saf_is_valid_field_handle(SAF_Field *a_field) 
{
  return( SS_FIELD(a_field) ? 1:0 );
}
hbool_t my_saf_is_valid_ftmpl_handle(SAF_FieldTmpl *a_ftmpl) 
{
  return( SS_FIELDTMPL(a_ftmpl) ? 1:0 );
}
hbool_t my_saf_is_valid_set_handle(SAF_Set *a_set)
{
  return( SS_SET(a_set) ? 1:0 );
}



#define USE_CACHED_SUBSET_REL_COUNTS /*a short-cut: works ok*/
/*need to fix this for sslib? #define SORT_FIELDS_BY_ROW *//*another short-cut: works ok, june2003*/

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Global Variables
 *
 * Description: 
 *
 * This section defines the global variable structure, instantiates the structure,
 * and defines some things that are needed for the dual C/C++ trick.
 *
 *--------------------------------------------------------------------------------------------------- */
#ifdef __cplusplus 
class str_mesh_reader_t 
{
 public:
#else
  typedef struct str_mesh_reader_t 
  {
#endif


    int m_strMeshReaderGlobalsAreInitialized; /*This must be the first entry in the structure, or the
					initialization will not go as planned.*/

    SAF_Db *m_db;                /* Handle to the SAF database. */
    int m_rank;
    int m_nprocs;

    int m_totalNumFields; /*Used for matching fields with timesteps. This is the number of
			    entries in *m_allFields. If add_fields_not_in_suites_to_list has 
			    not been called, then this is the number of fields that are 
			    mentioned in the suites (i.e. have been given time values), 
			    otherwise, it is the total number of fields in the saf database.
			    Currently, add_fields_not_in_suites_to_list is never called.*/
    SAF_Field *m_allFields; /*See the comment after m_totalNumFields.*/
    SAF_FieldTmpl *m_allFieldTemplates; /*The corresponding field templates for each field
					  in *m_allFields*/
    SAF_Set *m_allFieldSets;/*The corresponding set for each field in *m_allFields*/
    int m_numTimesteps;/*The number of distinct timesteps (different time values) in the 
			 suites. Note: if two different states (possibly from different suites
			 or state groups) have the same time value, then read_all_suites()
			 will make them the same timestep*/
    MY_PRECISION *m_timeValues;
    int *m_timestepsPerField;/*Timestep corresponding to each entry in *m_allFields*/
    int *m_whichTopsetPerField;/*The entry in m_sets corresponding to each entry in *m_allFields.
				 Set to -1 if there is no corresponding structured top set.*/
    SAF_Set *m_sets;/*Sets that are considered "structured blocks".*/
    int m_numSets;
    SAF_Set *m_subsets;/*Sets that are node subsets of "structured blocks" in m_sets*/
    int m_numSubsets;
    SAF_Set *m_edgeSubsets,*m_faceSubsets,*m_elemSubsets;/*see comment after m_subsets*/
    int m_numEdgeSubsets,m_numFaceSubsets,m_numElemSubsets;
    int m_numNonTopSets;/*= m_numSubsets+m_numEdgeSubsets+m_numFaceSubsets+m_numElemSubsets*/
    SAF_Set *m_allNonTopSets;/*combination of m_subsets, m_edgeSubsets, m_faceSubsets and m_elemSubsets*/


    /*List of indirect fields not found by read_all_suites. We need this because there
      are three types of field: those mentioned in the suites (and therefore have a 
      definite time value), those that are indirectly connected to fields mentioned in
      the suites (and have an implied time value), and those that have no time value at all.*/
    int m_totalNumIndirectFields;
    SAF_Field *m_allIndirectFields;
    int *m_timestepsPerIndirectField;



    /*NEEDED FOR ENSIGHT:*/
    int *m_numVarsPerBlock;/*num blocks == num sets*/
    int m_maxNumVars;
    SAF_Field ***m_instancePerTimestepPerVarPerBlock;
    SAF_FieldTmpl **m_templatePerVarPerBlock;
    saf_str_mesh_var_type *m_strMeshVarType;
    char **m_strMeshVarUnitName;
    char **m_strMeshVarQuantName;
    int *m_whichVarIsThisField;

    char **m_strVarNames;
    int m_safIsInitialized;

#ifdef USE_CACHED_SUBSET_REL_COUNTS
    int **m_numSubsetRelsPerNonTopSetPerTopset;/*global var used for caching subset relations*/
    int **m_numSubsetRelsPerNonTopSetPerNonTopset;/*global var used for caching subset relations*/
#endif

    /* global vars for printinfo */
    FILE *m_printinfoFp;
    int m_couldntOpenFileForWriting;
    int m_printinfoGoesToFile;

#ifdef WIN32
    int m_rerouteStdoutStderr;
#endif

    /* used with checkBufferSize and freeBuffer */
    MY_PRECISION *m_buffer;
    int m_bufferSize;

    char *m_printinfoFilename;

    int m_lookForRegistryInEnsightTree;

#ifdef __cplusplus 
  };
#else
} str_mesh_reader_t;
#endif


#ifdef __cplusplus 
#  define STR_CLASS_NAME_IF_CPP SAFStrMeshReader::
#else              
#  define STR_CLASS_NAME_IF_CPP 
#endif
           

#ifdef __cplusplus 
   int SAFStrMeshReader::m_numInstances=0; /* instantiate static variable */
#else 
   str_mesh_reader_t g_strGlobals_instance = { 0 };/*Note that the first entry must remain g_strMeshReaderGlobalsAreInitialized,
						  otherwise there will be problems because of unpredictable values for
						  the rest of the members*/
   str_mesh_reader_t *g_strGlobals = &g_strGlobals_instance;
#endif
           


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Constructor
 * Description:
 * 
 *--------------------------------------------------------------------------------------------------- */
#ifdef __cplusplus 
SAFStrMeshReader::SAFStrMeshReader()
{
  g_strGlobals = (str_mesh_reader_t *)malloc( sizeof(str_mesh_reader_t) );

  initialize_str_globals();

  m_variableNames = new VariableNames();

#ifdef HAVE_PARALLEL
  MPI_Comm_rank(MPI_COMM_WORLD, &g_strGlobals->m_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &g_strGlobals->m_nprocs);
#endif

  m_whichInstance = 0;

}
#endif



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Destructor
 * Description:
 * 
 *--------------------------------------------------------------------------------------------------- */
#ifdef __cplusplus 
SAFStrMeshReader::~SAFStrMeshReader()
{
  free(g_strGlobals);
  g_strGlobals=NULL;

  delete m_variableNames;
  m_variableNames=NULL;
}
#endif


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Some temporary functions for read_str_mesh
 * Description:
 * These functions are here for now to give read_str_mesh access to some of the global
 * variables. read_str_mesh was written long ago, and needs updating.
 * 
 *--------------------------------------------------------------------------------------------------- */

SAF_Db  *
STR_CLASS_NAME_IF_CPP
getDatabase() { return(g_strGlobals->m_db); }

int 
STR_CLASS_NAME_IF_CPP
getNumStrSets() { return(g_strGlobals->m_numSets); }

int 
STR_CLASS_NAME_IF_CPP
getNumStrSubsets() { return(g_strGlobals->m_numSubsets); }

int 
STR_CLASS_NAME_IF_CPP
getNumStrEdgeSubsets() { return(g_strGlobals->m_numEdgeSubsets); }

int 
STR_CLASS_NAME_IF_CPP
getNumStrFaceSubsets() { return(g_strGlobals->m_numFaceSubsets); }

int 
STR_CLASS_NAME_IF_CPP
getNumStrElemSubsets() { return(g_strGlobals->m_numElemSubsets); }

SAF_Set *
STR_CLASS_NAME_IF_CPP
getStrSets() { return(g_strGlobals->m_sets); }

SAF_Set *
STR_CLASS_NAME_IF_CPP
getStrSubsets() { return(g_strGlobals->m_subsets); }

SAF_Set *
STR_CLASS_NAME_IF_CPP
getStrEdgeSubsets() { return(g_strGlobals->m_edgeSubsets); }

SAF_Set *
STR_CLASS_NAME_IF_CPP
getStrFaceSubsets() { return(g_strGlobals->m_faceSubsets); }

SAF_Set *
STR_CLASS_NAME_IF_CPP
getStrElemSubsets() { return(g_strGlobals->m_elemSubsets); }

int 
STR_CLASS_NAME_IF_CPP
getMaxNumStrVars() { return(g_strGlobals->m_maxNumVars); }

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Set the name of the comment file
 * Description:
 * Set the file name to print to. If name is NULL, then print to stdout
 * 
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
printinfo_set_filename( const char *name )
{
  /*If a file is already open for printing, close it.*/
  if(g_strGlobals->m_strMeshReaderGlobalsAreInitialized && 
     g_strGlobals->m_printinfoGoesToFile && 
     ( g_strGlobals->m_printinfoFp || g_strGlobals->m_couldntOpenFileForWriting ) )
    {
      if(g_strGlobals->m_printinfoFp) 
	{
	  if(name) fprintf(g_strGlobals->m_printinfoFp,"\nClosing this output file, and opening instead: %s\n\n",name);
	  fclose(g_strGlobals->m_printinfoFp);
	  g_strGlobals->m_printinfoFp=NULL;
	}
      g_strGlobals->m_couldntOpenFileForWriting=0;
    }

  if( !name )
    {
      g_strGlobals->m_printinfoGoesToFile=0;
    }
  else
    {
      g_strGlobals->m_printinfoGoesToFile=1;
      g_strGlobals->m_printinfoFilename=ens_strdup(name);
    }
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Initialize the Members of the Global Structure
 * Description:
 *
 * 
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
initialize_str_globals()
{
  g_strGlobals->m_strMeshReaderGlobalsAreInitialized=1; /*the reader is initialized by calling this function*/
				
  g_strGlobals->m_db=NULL;              
  g_strGlobals->m_rank=0;
  g_strGlobals->m_nprocs=1;

#ifdef HAVE_PARALLEL
  MPI_Comm_rank(MPI_COMM_WORLD, &g_strGlobals->m_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &g_strGlobals->m_nprocs);
#endif

  g_strGlobals->m_totalNumFields=0; 
	
  g_strGlobals->m_allFields=0; 
  g_strGlobals->m_allFieldTemplates=0; 
			
  g_strGlobals->m_allFieldSets=0;
  g_strGlobals->m_numTimesteps=0;
	
	
  g_strGlobals->m_timeValues=0;
  g_strGlobals->m_timestepsPerField=0;
  g_strGlobals->m_whichTopsetPerField=0;
			
  g_strGlobals->m_sets=0;
  g_strGlobals->m_numSets=0;
  g_strGlobals->m_subsets=0;
  g_strGlobals->m_numSubsets=0;
  g_strGlobals->m_edgeSubsets=0;
  g_strGlobals->m_faceSubsets=0;
  g_strGlobals->m_elemSubsets=0;

  g_strGlobals->m_numEdgeSubsets=0;
  g_strGlobals->m_numFaceSubsets=0;
  g_strGlobals->m_numElemSubsets=0;
  g_strGlobals->m_numNonTopSets=0;
  g_strGlobals->m_allNonTopSets=0;

  g_strGlobals->m_totalNumIndirectFields=0;
  g_strGlobals->m_allIndirectFields=0;
  g_strGlobals->m_timestepsPerIndirectField=0;

  g_strGlobals->m_numVarsPerBlock=0;
  g_strGlobals->m_maxNumVars=0;
  g_strGlobals->m_instancePerTimestepPerVarPerBlock=0;
  g_strGlobals->m_templatePerVarPerBlock=0;
  g_strGlobals->m_strMeshVarType=0;
  g_strGlobals->m_strMeshVarUnitName=0;
  g_strGlobals->m_strMeshVarQuantName=0;
  g_strGlobals->m_whichVarIsThisField=0;


  g_strGlobals->m_strVarNames=0;
  g_strGlobals->m_safIsInitialized=0;

#ifdef USE_CACHED_SUBSET_REL_COUNTS
  g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset=0;
  g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset=0;
#endif

  g_strGlobals->m_printinfoFp=0;
  g_strGlobals->m_couldntOpenFileForWriting=0;
  g_strGlobals->m_printinfoGoesToFile=1;
  g_strGlobals->m_printinfoFilename= ens_strdup("saf_reader_stdout.txt");


#ifdef WIN32
/*disabled for now, all those text files are annoying
  g_strGlobals->m_rerouteStdoutStderr=1;
*/
#endif

  g_strGlobals->m_buffer=0;
  g_strGlobals->m_bufferSize=0;


  g_strGlobals->m_lookForRegistryInEnsightTree=0;
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     
 * Description:
 *

 *--------------------------------------------------------------------------------------------------- */
void
STR_CLASS_NAME_IF_CPP
look_for_registry_in_ensight_tree( )
{
  g_strGlobals->m_lookForRegistryInEnsightTree=1;
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     
 * Description:
 *

 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
is_file_a_saf_database( const char *a_safFilename )
{
  SAF_PathInfo l_pathinfo;
  int l_result;

  if(!g_strGlobals->m_safIsInitialized)
    {
      printinfo("calling saf_init in is_file_a_saf_database\n");

      if( g_strGlobals->m_lookForRegistryInEnsightTree )
	{

#if (SAF_VERSION_MAJOR<1)|| \
    ((SAF_VERSION_MAJOR==1)&&(SAF_VERSION_MINOR<7))|| \
    ((SAF_VERSION_MAJOR==1)&&(SAF_VERSION_MINOR==7)&&(SAF_VERSION_RELEASE==0))

	  SAF_LibProps l_props = saf_createProps_lib();
#else
	  SAF_LibProps *l_props = saf_createProps_lib();
#endif


	  char *l_env = getenv("CEI_HOME");
	  if(!l_env)
	    {
	      printinfo("\nWarning: could not find env var CEI_HOME. A \"Registry.saf\" file might not be found!!\n\n");
	      saf_init(l_props);
	    }
	  else
	    {
	      FILE *fp=NULL;
	      const char *l_filename = "/lib/Registry.saf";
	      char *l_str = (char *)malloc( strlen(l_env) + strlen(l_filename) + 1 );
	      sprintf(l_str,"%s%s",l_env,l_filename);

	      printinfo("If there is no registry file in the current directory, will use: %s\n",l_str);

	      fp = fopen(l_str,"r");
	      if(!fp)
		{
		  printinfo("\nWarning: could not open file %s    A \"Registry.saf\" file might not be found!!\n\n",l_str);
		  saf_init(l_props);
		}
	      else
		{
		  fclose(fp);
		  saf_setProps_Registry(l_props,l_str);
		  saf_init(l_props);
		}	
	      free(l_str);
	    }
	}
      else
	{
	  saf_init(SAF_DEFAULT_LIBPROPS);
	}

      g_strGlobals->m_safIsInitialized=1;
    }

  l_pathinfo = saf_readInfo_path(a_safFilename,0);
  l_result = saf_getInfo_isSAFdatabase(l_pathinfo);
  saf_freeInfo_path(l_pathinfo);
  return(l_result);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Initialize the Structured Mesh Reader
 * Description:
 *
 * This function counts the structured objects in the database, allocates the 
 * appropriate global arrays, and then reads the timestep data and places it 
 * into global arrays also. 
 *
 * Note that this function must be called before init_unstr_mesh_reader because of 
 * the timestep data, even if the structured mesh data is not desired.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
init_str_mesh_reader( const char *a_safFilename )
{
  int i,j,k;
  SAF_DbProps *l_dbprops;      /* Handle to the SAF database properties. */
  FILE *fp;
  saf_str_mesh_var_type *l_varTypesPerField=NULL;

 
#ifndef __cplusplus
  /*we dont want to do this in c++, because the constructor handles it? Is this still true?*/
  if(!g_strGlobals->m_strMeshReaderGlobalsAreInitialized) initialize_str_globals();
#endif


  printinfo("\n*******BEGIN   init_str_mesh_reader on file: %s\n",a_safFilename);

  printinfo("Opening file: %s\n",a_safFilename);

  /*see if the file exists, to avoid finding out via a saf error*/
  fp = fopen(a_safFilename,"r");
  if(!fp)
    {
      printinfo("init_str_mesh_reader cant open file %s\n\n",a_safFilename);
      return(-1);
    }
  fclose(fp);     


#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  if(!g_strGlobals->m_safIsInitialized)
    {
      printinfo("Calling saf_init in init_str_mesh_reader\n");



      if( g_strGlobals->m_lookForRegistryInEnsightTree )
	{

#if (SAF_VERSION_MAJOR<1)|| \
    ((SAF_VERSION_MAJOR==1)&&(SAF_VERSION_MINOR<7))|| \
    ((SAF_VERSION_MAJOR==1)&&(SAF_VERSION_MINOR==7)&&(SAF_VERSION_RELEASE==0))

	  SAF_LibProps l_props = saf_createProps_lib();
#else
	  SAF_LibProps *l_props = saf_createProps_lib();
#endif


	  char *l_env = getenv("CEI_HOME");
	  if(!l_env)
	    {
	      printinfo("\nWarning: could not find env var CEI_HOME. A \"Registry.saf\" file might not be found!!\n\n");
	      saf_init(l_props);
	    }
	  else
	    {
	      FILE *l_regFp=NULL;
	      const char *l_filename = "/lib/Registry.saf";
	      char *l_str = (char *)malloc( strlen(l_env) + strlen(l_filename) + 1 );
	      sprintf(l_str,"%s%s",l_env,l_filename);

	      printinfo("If there is no registry file in the current directory, will use: %s\n",l_str);

	      l_regFp = fopen(l_str,"r");
	      if(!l_regFp)
		{
		  printinfo("\nWarning: could not open file %s    A \"Registry.saf\" file might not be found!!\n\n",l_str);
		  saf_init(l_props);
		}
	      else
		{
		  fclose(l_regFp);
		  saf_setProps_Registry(l_props,l_str);
		  saf_init(l_props);
		}	
	      free(l_str);
	    }
	}
      else
	{
	  saf_init(SAF_DEFAULT_LIBPROPS);
	}

 
      g_strGlobals->m_safIsInitialized=1;
    }
  else
    printinfo("NOT calling saf_init in init_str_mesh_reader\n");


#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif


  /*check the saf library version and the hdf5 library version*/
  {
    char l_annot[16];
    SAF_PathInfo l_pathinfo;

    int l_major=0,l_minor=0,l_patch=0;

    l_pathinfo = saf_readInfo_path(a_safFilename,0);

    if(saf_getInfo_isSAFdatabase(l_pathinfo))
      {
	saf_getInfo_libversion(l_pathinfo,&l_major,&l_minor,&l_patch,l_annot);

	if( l_major!=SAF_VERSION_MAJOR ||
	    l_minor!=SAF_VERSION_MINOR ||
	    l_patch!=SAF_VERSION_RELEASE )
	  {
	    printinfo("\nwarning: FILE WAS CREATED BY SAF %d.%d.%d-%s, BUT THE READER IS %d.%d.%d-%s\n\n",
		      l_major,l_minor,l_patch,l_annot,
		      SAF_VERSION_MAJOR,SAF_VERSION_MINOR,SAF_VERSION_RELEASE,SAF_VERSION_ANNOT );
	  }

	saf_getInfo_hdfversion(l_pathinfo,&l_major,&l_minor,&l_patch,l_annot);
	if( l_major!=H5_VERS_MAJOR ||
	    l_minor!=H5_VERS_MINOR ||
	    l_patch!=H5_VERS_RELEASE )
	  {
	    printinfo("\nwarning: FILE WAS CREATED BY HDF %d.%d.%d-%s, BUT THE READER IS %d.%d.%d-%s\n\n",
		      l_major,l_minor,l_patch,l_annot,
		      H5_VERS_MAJOR,H5_VERS_MINOR,H5_VERS_RELEASE,H5_VERS_SUBRELEASE );
	  }
      }
    else
      {
	printinfo("saf_getInfo_isSAFdatabase says file %s is NOT a saf database\n", a_safFilename);
	return(-1);
      }



    saf_freeInfo_path(l_pathinfo);
  }

  if(g_strGlobals->m_sets)
    {
      printinfo("error init_str_mesh_reader: m_sets!=NULL, a database is already open\n");
      return(-1);
    }

  l_dbprops = saf_createProps_database();
  saf_setProps_ReadOnly(l_dbprops);
  g_strGlobals->m_db = saf_open_database(a_safFilename,l_dbprops);


  g_strGlobals->m_numSets = get_num_block_sets( &g_strGlobals->m_sets );
  printinfo("Found %d blocks:\n",g_strGlobals->m_numSets);
  for(i=0;i<g_strGlobals->m_numSets;i++)
    {
      char *l_name=0;
      saf_describe_set(SAF_ALL, &(g_strGlobals->m_sets[i]), &l_name, NULL, NULL, NULL, NULL, NULL, NULL );
      printinfo("\t %s\n",l_name);
      if(l_name) free(l_name);
    }

  g_strGlobals->m_numSubsets = get_num_node_subsets( &g_strGlobals->m_subsets );
  printinfo("Found %d node subsets:\n",g_strGlobals->m_numSubsets);
  for(i=0;i<g_strGlobals->m_numSubsets;i++)
    {
      char *l_name=0;
      saf_describe_set(SAF_ALL, &(g_strGlobals->m_subsets[i]), &l_name, NULL, NULL, NULL, NULL, NULL, NULL );
      printinfo("\t %s\n",l_name);
      if(l_name) free(l_name);
    }

  g_strGlobals->m_numEdgeSubsets = get_num_edge_subsets( &g_strGlobals->m_edgeSubsets );
  printinfo("Found %d edge subsets:\n",g_strGlobals->m_numEdgeSubsets);
  for(i=0;i<g_strGlobals->m_numEdgeSubsets;i++)
    {
      char *l_name=0;
      saf_describe_set(SAF_ALL, &(g_strGlobals->m_edgeSubsets[i]), &l_name, NULL, NULL, NULL, NULL, NULL, NULL );
      printinfo("\t %s\n",l_name);
      if(l_name) free(l_name);
    }

  g_strGlobals->m_numFaceSubsets = get_num_face_subsets( &g_strGlobals->m_faceSubsets );
  printinfo("Found %d face subsets:\n",g_strGlobals->m_numFaceSubsets);
  for(i=0;i<g_strGlobals->m_numFaceSubsets;i++)
    {
      char *l_name=0;
      saf_describe_set(SAF_ALL, &(g_strGlobals->m_faceSubsets[i]), &l_name, NULL, NULL, NULL, NULL, NULL, NULL );
      printinfo("\t %s\n",l_name);
      if(l_name) free(l_name);
    }

  g_strGlobals->m_numElemSubsets = get_num_elem_subsets( &g_strGlobals->m_elemSubsets );
  printinfo("Found %d elem subsets:\n",g_strGlobals->m_numElemSubsets);
  for(i=0;i<g_strGlobals->m_numElemSubsets;i++)
    {
      char *l_name=0;
      saf_describe_set(SAF_ALL, &(g_strGlobals->m_elemSubsets[i]), &l_name, NULL, NULL, NULL, NULL, NULL, NULL );
      printinfo("\t %s\n",l_name);
      if(l_name) free(l_name);
    }

  /* READ SUITES - timesteps*/
  if( read_all_suites(  ) )
    {
      return(-1);/*read_all_suites returned an error*/
    }



  /*pick up any fields that were not mentioned in the suites, and assume
    they are valid at timestep 0*/
  /* NOT USING THIS BECAUSE OF THE ALE3D DATA 10/31/2002
     add_fields_not_in_suites_to_list( );
  */







#if 0 /*get the non-suite fields, print some info, then exit*/
  printinfo("\n\nFOR DEBUG, CALLING add_fields_not_in_suites_to_list\n");
  add_fields_not_in_suites_to_list( );
  printinfo("\n\nFOR DEBUG, PRINTING EVERY FIELD FOUND\n");
  for(j=0;j<g_strGlobals->m_totalNumFields;j++)
    {
      int l_count=0;
      char *l_catName=NULL,*l_fieldName=NULL,*l_setName=NULL;
      SAF_Set l_thisset;
      SAF_Cat l_cat;
      SAF_CellType l_cellType;
      saf_describe_field(SAF_ALL, &(g_strGlobals->m_allFields[j]), NULL, &l_fieldName, &l_thisset, NULL,
			 NULL, NULL, &l_cat, NULL, NULL, NULL, NULL,  
			 NULL, NULL, NULL, NULL);
      saf_describe_set(SAF_ALL, &(l_thisset), &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
      saf_describe_category(SAF_ALL,&l_cat,&l_catName,NULL,NULL);
      saf_describe_collection(SAF_ALL,&l_thisset,&l_cat,&l_cellType,&l_count,NULL,NULL,NULL);
      printinfo("      %d: field=%s  set=%s  cat=%s  collcount=%d  celltype=%s\n",
		j,l_fieldName,l_setName,l_catName,l_count,get_celltype_desc(l_cellType));
      free(l_catName);
      free(l_setName);
      free(l_fieldName);
    }
  printinfo("\n\nFOR DEBUG, FINISHED PRINTING EVERY FIELD FOUND, NOW EXITING\n");
  exit(-1);
#endif








  get_timesteps_for_all_indirect_fields();




 

#if 0
  /*SAF_IndexSpec test*/
  printinfo("\nBegin init_str_mesh SAF_IndexSpec test for all fields:\n");
  for(i=0;i<g_strGlobals->m_totalNumFields;i++)
    {
      SAF_IndexSpec l_ispec;
      SAF_FieldTmpl l_fieldTemplate;
      SAF_Set l_set;
      SAF_Cat l_cat;
      char *l_fieldName=0;

      saf_describe_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), &l_fieldTemplate, &l_fieldName, &l_set, NULL,
			 NULL, NULL, &l_cat, NULL, NULL,
			 NULL, NULL, NULL, NULL, NULL, NULL);

      saf_describe_collection(SAF_ALL,&l_set,&l_cat,NULL,NULL,&l_ispec,NULL,NULL);
      for( j=0; j<l_ispec.ndims; j++ )
        {
	  if( l_ispec.order[j]<0 || l_ispec.order[j]>=l_ispec.ndims )
	    {
	      printinfo("init_str_mesh error: bad SAF_IndexSpec orders l_ispec.order[%d]=%d  l_ispec.ndims=%d\n",
			j, l_ispec.order[j], l_ispec.ndims );
	      printinfo("  i=%d of %d\n",i,g_strGlobals->m_totalNumFields);
	      printinfo("  l_fieldName=%s\n",l_fieldName);

	      exit(-1);
	    }
        }

      if(l_fieldName) free(l_fieldName);
    }
  printinfo("Finish init_str_mesh SAF_IndexSpec test for all fields.\n\n");
  exit(-1);
#endif







#if 0
  /*print some info
    note: this statement is not true if add_fields_not_in_suites_to_list was called!*/
  printinfo("These are the %d fields that have associated timesteps:\n",g_strGlobals->m_totalNumFields);
  for(i=0;i<g_strGlobals->m_totalNumFields;i++) 
    {
      char *l_name=0, *l_tmplName=0, *l_setname=0, *l_topsetname=0;

      if(!my_saf_is_valid_field_handle(&(g_strGlobals->m_allFields[i]))) 
	{
	  printinfo("   field %d is not a valid field handle\n",i);
	  continue;
	}

      saf_describe_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), NULL, &l_name, NULL, NULL, NULL,
			 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      saf_describe_field_tmpl(SAF_ALL,&(g_strGlobals->m_allFieldTemplates[i]),&l_tmplName,NULL,NULL,NULL,NULL,NULL);
      saf_describe_set(SAF_ALL, &(g_strGlobals->m_allFieldSets[i]), &l_setname, NULL, NULL, NULL, NULL, NULL, NULL );

      if(g_strGlobals->m_whichTopsetPerField[i]>=0) 
	{
	  saf_describe_set(SAF_ALL, &(g_strGlobals->m_sets[g_strGlobals->m_whichTopsetPerField[i]]), &l_topsetname, NULL, NULL, NULL, NULL, NULL, NULL );
	  printinfo("   %d: time %d, name=%s, template=%s, set=%s, topset=%s\n",i,
		    g_strGlobals->m_timestepsPerField[i],l_name,l_tmplName,l_setname,
		    l_topsetname ? l_topsetname:"" );
	}
      else
	{
	  printinfo("   %d: time %d, name=%s, template=%s, set=%s\n",i,
		    g_strGlobals->m_timestepsPerField[i],l_name,l_tmplName,l_setname );
	}

      if(l_name) free(l_name);
      if(l_tmplName) free(l_tmplName);
      if(l_setname) free(l_setname);
      if(l_topsetname) free(l_topsetname);
    }
  printinfo("\n");
#endif







  /*FILL SOME ARRAYS NEEDED FOR ENSIGHT*/
  g_strGlobals->m_maxNumVars=0;
  if( g_strGlobals->m_numSets )
    {
      g_strGlobals->m_numVarsPerBlock = (int *)malloc(g_strGlobals->m_numSets*sizeof(int));
      g_strGlobals->m_instancePerTimestepPerVarPerBlock = (SAF_Field ***)malloc(g_strGlobals->m_numSets*sizeof(SAF_Field **));
      g_strGlobals->m_templatePerVarPerBlock = (SAF_FieldTmpl **)malloc(g_strGlobals->m_numSets*sizeof(SAF_FieldTmpl *));
      for(i=0; i<g_strGlobals->m_numSets; i++)
	{
	  g_strGlobals->m_numVarsPerBlock[i]=0;
	  g_strGlobals->m_instancePerTimestepPerVarPerBlock[i]=0;
	  g_strGlobals->m_templatePerVarPerBlock[i]=0;
	}
    }
  else
    {
      /*printinfo("init_str_mesh_reader: g_strGlobals->m_numSets=0, skipping  mallocs\n");*/
    }

  if( g_strGlobals->m_totalNumFields )
    {
      g_strGlobals->m_strMeshVarType = (saf_str_mesh_var_type *)malloc(g_strGlobals->m_totalNumFields*sizeof(saf_str_mesh_var_type));
      g_strGlobals->m_strMeshVarUnitName = (char **)malloc(g_strGlobals->m_totalNumFields*sizeof(char *));
      g_strGlobals->m_strMeshVarQuantName = (char **)malloc(g_strGlobals->m_totalNumFields*sizeof(char *));
      g_strGlobals->m_whichVarIsThisField = (int *)malloc(g_strGlobals->m_totalNumFields*sizeof(int));
      for(i=0; i<g_strGlobals->m_totalNumFields; i++)
	{
	  g_strGlobals->m_strMeshVarType[i] = SAF_STR_MESH_UNKNOWN;
	  g_strGlobals->m_strMeshVarUnitName[i]=0;
	  g_strGlobals->m_strMeshVarQuantName[i]=0;
	  g_strGlobals->m_whichVarIsThisField[i]=-1;
	}
    }
  else
    {
      /*printinfo("init_str_mesh_reader: g_strGlobals->m_totalNumFields=0, skipping  mallocs\n");*/
    }

  if( g_strGlobals->m_totalNumFields )
    {
      l_varTypesPerField = (saf_str_mesh_var_type *)malloc(g_strGlobals->m_totalNumFields*sizeof(saf_str_mesh_var_type));
    }

  for(i=0; i<g_strGlobals->m_totalNumFields; i++)
    {
      hbool_t l_isCoordField;
      SAF_Unit l_unit;
      SAF_Quantity l_quantity;
      char *l_unitName=0;
      char *l_quantName=0;

      l_varTypesPerField[i]=SAF_STR_MESH_UNKNOWN;

      if(my_saf_is_valid_field_handle(&(g_strGlobals->m_allFields[i])) && g_strGlobals->m_whichTopsetPerField[i]>=0 ) 
	{

	  saf_describe_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), NULL,NULL, NULL, &l_unit, 
			     &l_isCoordField, NULL, NULL, NULL, NULL, 
			     NULL, NULL, NULL, NULL, NULL, NULL);

	  if(l_isCoordField)
	    {
	      /*printinfo("field %d of %d is a coord field\n",i,g_strGlobals->m_totalNumFields);*/
	    }
	  else
	    {
	      int l_orig=1;
	      saf_str_mesh_var_type l_strMeshType=
		get_variable_type_from_field( g_strGlobals->m_sets[g_strGlobals->m_whichTopsetPerField[i]], 
					      g_strGlobals->m_allFields[i] );
	      l_varTypesPerField[i]=l_strMeshType;
	      saf_describe_unit(SAF_ALL,&l_unit,&l_unitName,NULL,NULL,NULL,NULL,NULL,NULL, &l_quantity );
	      saf_describe_quantity(SAF_ALL,&l_quantity,&l_quantName,NULL,NULL,NULL,NULL);
	      for(j=0; j<i; j++)
		{
		  /*This is where it is decided which fields are considered to be the "same variable"
		    Currently using the same criteria as unstructured vars, plus the requirement that the
		    variable type is similar (i.e. the same, or both are edges, or both are faces)*/ 
		  if( my_saf_is_valid_field_handle(&(g_strGlobals->m_allFields[j])) && g_strGlobals->m_whichTopsetPerField[j]>=0 )
		    {
		      saf_str_mesh_var_type l_prevStrMeshType=l_varTypesPerField[j];

		      if( are_similar_str_mesh_var_types(l_strMeshType,l_prevStrMeshType) &&
			  are_these_two_fields_from_the_same_var(&(g_strGlobals->m_allFields[i]),&(g_strGlobals->m_allFields[j])) )
			{
			  int l_thisVar=g_strGlobals->m_whichVarIsThisField[j];
			  l_orig=0;
			  g_strGlobals->m_whichVarIsThisField[i] = l_thisVar;

			  if(l_thisVar+1 > g_strGlobals->m_numVarsPerBlock[ g_strGlobals->m_whichTopsetPerField[i] ] )
			    {
			      g_strGlobals->m_numVarsPerBlock[ g_strGlobals->m_whichTopsetPerField[i] ] = l_thisVar+1;
			    }
			  break;
			}
		    }
		}

	      if(l_orig && g_strGlobals->m_numSets && g_strGlobals->m_whichTopsetPerField[i]>=0 )
		{
		  int l_thisVar=0;

		  while( g_strGlobals->m_strMeshVarType[l_thisVar]!=SAF_STR_MESH_UNKNOWN &&
			 ( g_strGlobals->m_strMeshVarType[l_thisVar]!=l_strMeshType ||
			   strcmp(l_unitName,g_strGlobals->m_strMeshVarUnitName[l_thisVar]) ||
			   strcmp(l_quantName,g_strGlobals->m_strMeshVarQuantName[l_thisVar]) ))
		    {
		      l_thisVar++;
		    }

		  if( g_strGlobals->m_strMeshVarType[l_thisVar]==SAF_STR_MESH_UNKNOWN )
		    { /*first use of this var slot*/
		      g_strGlobals->m_strMeshVarType[l_thisVar] = l_strMeshType;
		      g_strGlobals->m_strMeshVarUnitName[l_thisVar] = ens_strdup(l_unitName);
		      g_strGlobals->m_strMeshVarQuantName[l_thisVar] = ens_strdup(l_quantName);
		    }
		  if(l_thisVar+1 > g_strGlobals->m_numVarsPerBlock[ g_strGlobals->m_whichTopsetPerField[i] ] )
		    {
		      g_strGlobals->m_numVarsPerBlock[ g_strGlobals->m_whichTopsetPerField[i] ] = l_thisVar+1;
		    }
		  if(l_thisVar+1 > g_strGlobals->m_maxNumVars )
		    {
		      g_strGlobals->m_maxNumVars = l_thisVar+1;
		    }

		  g_strGlobals->m_whichVarIsThisField[i] = l_thisVar;
	       
		  /*printinfo("ORIG template i=%d, topset=%d, unitname=%s, \tvar type=%d, var slot=%d\n",i, 
		    g_strGlobals->m_whichTopsetPerField[i],l_unitName,l_strMeshType,g_strGlobals->m_whichVarIsThisField[i]);*/
		}
	    }

	  if(l_unitName) free(l_unitName);
	  if(l_quantName) free(l_quantName);
	}
    }

  if(l_varTypesPerField) free(l_varTypesPerField); l_varTypesPerField=NULL;

  for(i=0;i<g_strGlobals->m_numSets;i++) 
    {
      printinfo("Block %d has %d possibly distinct variables.\n",i,g_strGlobals->m_numVarsPerBlock[i]);

      if( g_strGlobals->m_numVarsPerBlock[i] )
	{
	  g_strGlobals->m_instancePerTimestepPerVarPerBlock[i]=(SAF_Field **)malloc(g_strGlobals->m_numVarsPerBlock[i]*sizeof(SAF_Field *));
	  g_strGlobals->m_templatePerVarPerBlock[i] = (SAF_FieldTmpl *)malloc(g_strGlobals->m_numVarsPerBlock[i]*sizeof(SAF_FieldTmpl));
	  for(j=0;j<g_strGlobals->m_numVarsPerBlock[i];j++) 
	    {
	      g_strGlobals->m_templatePerVarPerBlock[i][j]=SS_FIELDTMPL_NULL;
	      if( g_strGlobals->m_numTimesteps )
		{
		  g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][j]=(SAF_Field *)malloc(g_strGlobals->m_numTimesteps*sizeof(SAF_Field));
		  for(k=0;k<g_strGlobals->m_numTimesteps;k++) 
		    {
		      g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][j][k] = SS_FIELD_NULL;      
		    }
		}
	    }
	}
    }

  /*printinfo("now going thru all m_totalNumFields=%d in init_str_mesh_reader (ensight area):\n",g_strGlobals->m_totalNumFields);*/
  for(i=0; i<g_strGlobals->m_totalNumFields; i++)
    {
      int l_whichTopset = g_strGlobals->m_whichTopsetPerField[i];
      if( l_whichTopset >= 0 )
	{
	  int l_whichVar = g_strGlobals->m_whichVarIsThisField[i];
	  int l_whichTimestep = g_strGlobals->m_timestepsPerField[i];
  
	  if( l_whichVar<0 || l_whichTimestep<0 || !g_strGlobals->m_templatePerVarPerBlock[l_whichTopset] ) 
	    {
	      printinfo("is this an error? init_str_mesh_reader var=%d time=%d field#=%d  g_strGlobals->m_templatePerVarPerBlock[%d]=%s\n",
			l_whichVar, l_whichTimestep, i, l_whichTopset, 
			g_strGlobals->m_templatePerVarPerBlock[l_whichTopset]?"non-zero":"zero" );
	    }
	  else
	    {
	      if( !my_saf_is_valid_ftmpl_handle(&(g_strGlobals->m_templatePerVarPerBlock[l_whichTopset][l_whichVar])) )
		{
		  /*havent set the template yet: first usage of this var in the list*/
		  memcpy(&g_strGlobals->m_templatePerVarPerBlock[l_whichTopset][l_whichVar],&g_strGlobals->m_allFieldTemplates[i],sizeof(SAF_FieldTmpl));
		}
	      memcpy(&g_strGlobals->m_instancePerTimestepPerVarPerBlock[l_whichTopset][l_whichVar][l_whichTimestep],
		     &g_strGlobals->m_allFields[i],sizeof(SAF_Field));
	    }
	}
      /*
	else printinfo("field %d of %d has no topset\n",i,g_strGlobals->m_totalNumFields);
      */
    }



  /*recount m_numVarsPerBlock[*], to cut off unused slots at the end*/
  g_strGlobals->m_maxNumVars = 0;
  for(i=0;i<g_strGlobals->m_numSets;i++)
    {
      if( g_strGlobals->m_numVarsPerBlock[i] )
	{
	  int l_lastGoodOne=0;
	  for(j=0;j<g_strGlobals->m_numVarsPerBlock[i];j++)
	    {
	      if( my_saf_is_valid_ftmpl_handle( &(g_strGlobals->m_templatePerVarPerBlock[i][j])) )
		{
		  l_lastGoodOne=j;
		}
	    }

	  printinfo("For block %d, reducing %d variables to %d.\n",i,g_strGlobals->m_numVarsPerBlock[i],l_lastGoodOne+1);
	  g_strGlobals->m_numVarsPerBlock[i] = l_lastGoodOne+1;

	  if(g_strGlobals->m_numVarsPerBlock[i]>g_strGlobals->m_maxNumVars) g_strGlobals->m_maxNumVars = g_strGlobals->m_numVarsPerBlock[i];
	}
      else
	{
	  printinfo("For block %d there are no variables to reduce.\n",i);
	}
    }
	   


  printinfo("*******END     init_str_mesh_reader***************************\n\n");



#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif


  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Is the Reader Initialized?
 * Description:
 *
 * Returns 0 if the reader is not initialized. Also returns 0 there are no timesteps and no structured mesh
 * sets. Returns 1 otherwise.
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
is_str_mesh_reader_initialized()
{
  if( !g_strGlobals->m_safIsInitialized ) return(0);
  if( !g_strGlobals->m_numTimesteps && !g_strGlobals->m_numSets ) return(0);
  return(1);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Are the globals Initialized?
 * Description:
 *
 * Returns 0 if the globals are not initialized. Returns 1 otherwise.
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
are_str_mesh_reader_globals_initialized()
{
  if( !g_strGlobals->m_strMeshReaderGlobalsAreInitialized ) return(0);
  return(1);
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Reader Name
 * Description:
 *
 * The argument a_str (preallocated to length a_maxlen) is written over with a string 
 * resembling "SAF1.6.0,HDF1.6.0"
 * 
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
get_reader_name( char *a_str, unsigned int a_maxlen )
{
  snprintf(a_str,a_maxlen,"SAF%d.%d.%d,HDF%d.%d.%d",
	   SAF_VERSION_MAJOR,SAF_VERSION_MINOR,SAF_VERSION_RELEASE,H5_VERS_MAJOR,H5_VERS_MINOR,H5_VERS_RELEASE );
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Structured Mesh Reader Version String
 * Description:
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_str_mesh_reader_version( char *a_buffer, size_t a_size )
{
  char l_str[256]="SAF Reader, SAF ";
  size_t l_length = strlen(l_str);

  if(!g_strGlobals->m_safIsInitialized)
    {
      printinfo("calling saf_init in get_str_mesh_reader_version\n");


      if( g_strGlobals->m_lookForRegistryInEnsightTree )
	{

#if (SAF_VERSION_MAJOR<1)|| \
    ((SAF_VERSION_MAJOR==1)&&(SAF_VERSION_MINOR<7))|| \
    ((SAF_VERSION_MAJOR==1)&&(SAF_VERSION_MINOR==7)&&(SAF_VERSION_RELEASE==0))

	  SAF_LibProps l_props = saf_createProps_lib();
#else
	  SAF_LibProps *l_props = saf_createProps_lib();
#endif


	  char *l_env = getenv("CEI_HOME");
	  if(!l_env)
	    {
	      printinfo("\nWarning: could not find env var CEI_HOME. A \"Registry.saf\" file might not be found!!\n\n");
	      saf_init(l_props);
	    }
	  else
	    {
	      FILE *fp=NULL;
	      const char *l_filename = "/lib/Registry.saf";
	      char *l_regFileName = (char *)malloc( strlen(l_env) + strlen(l_filename) + 1 );
	      sprintf(l_regFileName,"%s%s",l_env,l_filename);

	      printinfo("If there is no registry file in the current directory, will use: %s\n",l_regFileName);

	      fp = fopen(l_regFileName,"r");
	      if(!fp)
		{
		  printinfo("\nWarning: could not open file %s    A \"Registry.saf\" file might not be found!!\n\n",
			l_regFileName);
		  saf_init(l_props);
		}
	      else
		{
		  fclose(fp);
		  saf_setProps_Registry(l_props,l_regFileName);
		  saf_init(l_props);
		}	
	      free(l_regFileName);
	    }
	}
      else
	{
	  saf_init(SAF_DEFAULT_LIBPROPS);
	}


      g_strGlobals->m_safIsInitialized=1;
    }

  saf_version_string(1, l_str+l_length, 256-l_length );
  l_length = (a_size>strlen(l_str))? strlen(l_str): a_size;
  strncpy(a_buffer,l_str, l_length );
  a_buffer[l_length]='\0';

  /*NOTE because dsl will fail if we call saf_init saf_final saf_init,
    then what we have to do is this: open each available ensight reader
    library (a bunch of saf_init's).....and leave them all open?
  */

  /*if(!g_strGlobals->m_safIsInitialized)
    {
    printinfo("calling saf_final in get_str_mesh_reader_version\n");
    saf_final();
    }*/

  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Structured Variables
 * Description:
 *
 * Returns the number of structured variables found by init_str_mesh_reader.
 * 
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_str_vars()
{
  return( g_strGlobals->m_maxNumVars );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Time Steps
 * Description:
 *
 * Returns the number of time steps found by init_str_mesh_reader.
 * 
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_timesteps()
{
  return( g_strGlobals->m_numTimesteps );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Structured Blocks
 * Description:
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_blocks( )
{
    return( g_strGlobals->m_numSets ); 
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Name of a Structured Block
 * Description:
 *
 * Up to a_maxlen characters of the name will be written to the preallocated array a_str.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_block_name( int a_which, char *a_str, int a_maxlen )
{
  char *l_name=0;
  if( a_which<0 || a_which>=g_strGlobals->m_numSets )
    {
      printinfo("\nget_block_name error: a_which=%d out of range\n",a_which);
      strncpy(a_str,"error",(size_t)(a_maxlen-1));
      return(-1);
    }
  saf_describe_set(SAF_ALL, &(g_strGlobals->m_sets[a_which]), &l_name, NULL, NULL, NULL, NULL, NULL, NULL );
  strncpy(a_str,l_name,(size_t)(a_maxlen-1));
  if(l_name) free(l_name);
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Size of a Structured Block from its Index
 * Description:
 *
 * If this function is called with a valid structured block index a_whichBlock, then the other
 * three arguments will be set with X Y and Z size of the block.
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
get_block_size( int a_whichBlock, int *a_xCoords, int *a_yCoords, int *a_zCoords )
{
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets )
    {
      a_xCoords[0]=0;
      a_yCoords[0]=0;
      a_zCoords[0]=0;
    }
  get_block_size_from_set( &(g_strGlobals->m_sets[a_whichBlock]), a_xCoords, a_yCoords, a_zCoords );
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get Attributes from a Given Block in String Form
 * Description:
 *
 * Gets any attributes associated with a structured block and returns them in comma-separated string
 * form. Up to a_maxlen characters of the name will be written to the preallocated array a_str.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_block_attributes_as_string( int a_whichBlock, char *a_str, size_t a_maxlen )
{
  int l_count=0,i;
  size_t l_currentPos;
  char **l_names=NULL;
  hid_t l_type = 0;
  a_str[0]='\0';
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets )
    {
      printinfo("get_block_attributes_as_string error, a_whichBlock=%d is out of range\n",a_whichBlock);
      return(-1);
    }
  saf_get_set_att(SAF_ALL, &(g_strGlobals->m_sets[a_whichBlock]), SAF_ATT_NAMES, &l_type, &l_count, (void **)&l_names );

  for(i=0;i<l_count;i++)
    {
      char *l_strings=NULL,*l_ptr;
      int l_countAgain=0,j;
      hid_t l_typeAgain = H5T_STRING;
      saf_get_set_att(SAF_ALL, &(g_strGlobals->m_sets[a_whichBlock]), l_names[i], &l_typeAgain, &l_countAgain, (void **)&l_strings );
      for( j=0; j<l_countAgain; j++ )
	{
	  l_currentPos = strlen( a_str );
	  l_ptr = a_str + l_currentPos;
	  if(H5Tequal(l_typeAgain,H5T_STRING))
	    {
	      if(l_countAgain!=1)
		{
		  /*do nothing. This is broken? see getAtt.c*/
		}
	      else
		snprintf(l_ptr,a_maxlen-l_currentPos,"%s=%s", l_names[i], l_strings );
	    }
	  else if(H5Tequal(l_typeAgain,SAF_FLOAT)) {
	    snprintf(l_ptr,a_maxlen-l_currentPos,"%s=%f",l_names[i],((float *)l_strings)[j] );
	  } else if(H5Tequal(l_typeAgain,SAF_DOUBLE)) {
	    snprintf(l_ptr,a_maxlen-l_currentPos,"%s=%f",l_names[i],((float *)l_strings)[j] );
	  } else if(H5Tequal(l_typeAgain,SAF_INT)) {
	    snprintf(l_ptr,a_maxlen-l_currentPos,"%s=%d",l_names[i],((int *)l_strings)[j] );
	  } else {
	    /*DO NOTHING printinfo("  attribute %d of %d, unknown type\n",i,l_count);*/
	  }
	  if( j<l_countAgain-1 || i<l_count-1 )
	    {
	      l_currentPos = strlen( a_str );
	      l_ptr = a_str + l_currentPos;
	      snprintf(l_ptr,a_maxlen-l_currentPos,", ");
	    }
	}
    }
  /*add an ellipsis at the end, if we overflowed the buffer*/
  l_currentPos = strlen( a_str );
  if( l_currentPos >= a_maxlen-1 )
    {
      a_str[a_maxlen-1]='\0';
      a_str[a_maxlen-2]='.';
      a_str[a_maxlen-3]='.';
      a_str[a_maxlen-4]='.';
    }
  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Does this Block have Separable 3D Coords?
 * Description:
 *
 * Returns 1 if the block has separable (i.e. a block of size X*Y*Z is described by 3 1d arrays
 * of size X, Y and Z) coordinates.  
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
is_block_separable( int a_whichBlock )
{
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets )
    {
      printinfo("error is_block_separable: a_whichBlock out of range: a_whichBlock=%d g_strGlobals->m_numSets=%d\n",
		a_whichBlock,g_strGlobals->m_numSets);
      exit(-1);
    }

  return( !read_separable_3d_coords(g_strGlobals->m_sets[a_whichBlock],0,0,0) );
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Temporary Replacement for strdup
 * Description:
 *
 * Added this temporary fix 02/26/03 because strdup doesnt seem to work on sass3276
 *
 *--------------------------------------------------------------------------------------------------- */
char *
ens_strdup(const char *a_str)
{
  int l_len;
  char *l_ptr=0;
  l_len = strlen(a_str);
  /*if(l_len<=0) return(0);*/ /*if length 0, then allocate a length 1 string, containing only '\0'*/
  l_ptr = (char *)malloc( (l_len+1)*sizeof(char) );/*+1 for the ending '\0'*/
  memcpy(l_ptr,a_str,l_len*sizeof(char));
  l_ptr[l_len]='\0';/*ok because of the +1 above*/

  return(l_ptr);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Units Name for a Variable
 * Description:
 *
 * Get the units name for the structured variable with index a_which, e.g. Kelvin, Celsius.
 *
 *--------------------------------------------------------------------------------------------------- */
char *
STR_CLASS_NAME_IF_CPP
get_str_var_unit_name(int a_which)
{
    if( g_strGlobals->m_strMeshVarUnitName )
    {
        if( a_which>=0 || a_which<g_strGlobals->m_totalNumFields )
	{
	    return( ens_strdup(g_strGlobals->m_strMeshVarUnitName[a_which])  );
	}
    }
    return(NULL);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Quantity Name for a Variable
 * Description:
 *
 * Get the quantity name for the structured variable with index a_which, e.g. Temperature, Frequency.
 *
 *--------------------------------------------------------------------------------------------------- */
char *
STR_CLASS_NAME_IF_CPP
get_str_var_quant_name(int a_which)
{
    if( g_strGlobals->m_strMeshVarQuantName )
    {
        if( a_which>=0 || a_which<g_strGlobals->m_totalNumFields )
	{
	    return( ens_strdup(g_strGlobals->m_strMeshVarQuantName[a_which]) );
	}
    }
    return(NULL);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get Variable Name
 * Description:
 *
 * Gets the name of a structured variable. Up to a_maxlen characters of the name will be 
 * written to the preallocated array a_str.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_str_var_name( int a_whichVar, char *a_str, unsigned int a_maxlen )
{
  if( a_whichVar<0 || a_whichVar>=g_strGlobals->m_maxNumVars )
    {
      printinfo("get_str_var_name error, a_whichVar=%d is out of range\n",a_whichVar);
      return(0);
    }  

  /*if first time thru, allocate the global vars and get the names*/
  if(!g_strGlobals->m_strVarNames)
    {
      get_all_str_var_names(a_maxlen);
    }

  /*if we have already found this var name, return it*/
  if( g_strGlobals->m_strVarNames[a_whichVar] )
    {
      strncpy(a_str,g_strGlobals->m_strVarNames[a_whichVar],a_maxlen);
      return(0);
    }

  /*otherwise, return an error string*/
  snprintf(a_str,a_maxlen,"str_var_name_error" );

  return(-1);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Temporary Replacement for snprintf on WIN32
 * Description:
 *
 * For some reason, the WIN32 multi-threaded library doesnt seem to contain snprintf. For now, using 
 * this dumb snprintf. NOTE THIS TEMPORARY SNPRINTF DOESNT CHECK FOR MEMORY OVERFLOWS! THERE
 * COULD EASILY BE A MEMORY ERROR IF YOU ARE NOT CAREFUL!
 *--------------------------------------------------------------------------------------------------- */
#if defined WIN32 || defined JANUS
int snprintf(char *buf, size_t count, const char *fmt, ... )
{
  va_list args;
  va_start(args,fmt);
  vsprintf(buf, fmt, args);

  if(strlen(buf)>=count)
  {
	  printf("WIN32 snprintf error here!! %d %d\n",strlen(buf),count);
	  exit(-1);
  }
  return(0);
}
#endif


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Replacement for printf that Routes Comments to a Text File
 * Description:
 *
 * All user messages, warnings and errors are given using printinfo. By setting
 * global variables, one can control whether the messages go to stdout or to a
 * file. 
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
printinfo( const char *fmt, ... )
{
#if 1 /*set to 0 to completely suppress output*/
  va_list args;


#if 0 /*sslib hooey this is not working as intended*/

  /*no output at all if SAF_READER_DEBUG is not defined*/
  static int s_alreadyGotEnvVar=0;
  static char *s_env=NULL;
  if(!s_alreadyGotEnvVar)
    {
      s_env = getenv("SAF_READER_DEBUG");
      s_alreadyGotEnvVar=1;
    }
  if(!s_env) return; 
#endif




  va_start(args,fmt);

  if(!g_strGlobals->m_strMeshReaderGlobalsAreInitialized || !g_strGlobals->m_printinfoGoesToFile)
    {
      /*print to stdout*/
      vfprintf(stdout, fmt, args);
    }
  else
    {
      /*print to a file if possible, otherwise to stdout*/
      if(!g_strGlobals->m_printinfoFp && !g_strGlobals->m_couldntOpenFileForWriting) 
	{


#ifdef __cplusplus 
	  m_whichInstance = m_numInstances++;

	  /* overwrite the filename that was set in initialize_str_globals */
	  /* with a name that can help discern between different instantiated readers */
	  char l_str[256];
	  sprintf(l_str,"saf_reader_stdout.pr%d.%d.txt",g_strGlobals->m_rank,m_whichInstance);
	  g_strGlobals->m_printinfoFilename=ens_strdup(l_str);
#endif


#ifdef WIN32
	   if( g_strGlobals->m_rerouteStdoutStderr )
	  {
	    /*because WIN32 often doesnt have a stderr & stdout? redirect to files*/ 
	    freopen("stdout.txt","w", stdout);
	    freopen("stderr.txt","w", stderr);
	    fprintf(stdout,"Routing stdout to file stdout.txt at first use of printinfo\n");
	    fprintf(stderr,"Routing stderr to file stderr.txt at first use of printinfo\n");
	  }
#endif

	  /*first: open with one proc and erase the file*/
	  if(!g_strGlobals->m_rank)
	    {
	      g_strGlobals->m_printinfoFp = fopen(g_strGlobals->m_printinfoFilename, "w");
	      if(g_strGlobals->m_printinfoFp)
		{
		  fprintf(g_strGlobals->m_printinfoFp,"printinfo started file %s\n",g_strGlobals->m_printinfoFilename);
		  fclose(g_strGlobals->m_printinfoFp);
		  g_strGlobals->m_printinfoFp=0;
		}
	    }

	  /*second: open for write/append with all procs*/
	  g_strGlobals->m_printinfoFp = fopen(g_strGlobals->m_printinfoFilename, "a");
	  if(!g_strGlobals->m_printinfoFp)
	    {
	      printf("proc %d COULDNT OPEN %s FOR WRITING, PRINTING TO STDOUT INSTEAD\n",g_strGlobals->m_rank,g_strGlobals->m_printinfoFilename);
	      g_strGlobals->m_couldntOpenFileForWriting=1;
	    }
	}
      if(g_strGlobals->m_couldntOpenFileForWriting)
	{
	  vfprintf(stdout, fmt, args);
	}
      else if(g_strGlobals->m_printinfoFp) 
	{
	  vfprintf(g_strGlobals->m_printinfoFp, fmt, args);
	  fflush(g_strGlobals->m_printinfoFp);
	}
    }
  va_end(args);
#endif
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Check or Resize the Global Buffer
 * Description:
 *
 * In this file, using g_strGlobals->m_buffer as temporary space for transferring/translating data,
 * to avoid lots of alloc's. 
 * 
 * Must call checkBufferSize before using g_strGlobals->m_buffer to ensure it has enough
 * room allocated.
 *
 * Must call freeBuffer to free memory, at the end of the program.
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
checkBufferSize( int a_size )
{
  if(a_size<=0)
    {
      printinfo("error in checkBufferSize: a_size=%d\n\n",a_size);
      exit(-1);
    }
  if( a_size>g_strGlobals->m_bufferSize )
    {
      printinfo("checkBufferSize re-allocating buffer to size %d\n",a_size);
      g_strGlobals->m_buffer = (MY_PRECISION *)realloc(g_strGlobals->m_buffer,a_size*sizeof(MY_PRECISION));
      g_strGlobals->m_bufferSize=a_size;
    }
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Free the Global Buffer
 * Description:
 *
 * In this file, using g_strGlobals->m_buffer as temporary space for transferring/translating data,
 * to avoid lots of alloc's. 
 * 
 * Must call checkBufferSize before using g_strGlobals->m_buffer to ensure it has enough
 * room allocated.
 *
 * Must call freeBuffer to free memory, at the end of the program.
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
freeBuffer( void )
{
  if( g_strGlobals->m_buffer ) free(g_strGlobals->m_buffer);
  g_strGlobals->m_buffer=0;
  g_strGlobals->m_bufferSize=0;
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Time Value for a Given Timestep
 * Description:
 *
 * Returns the time value for a given time step found by init_str_mesh_reader.
 * 
 *--------------------------------------------------------------------------------------------------- */
MY_PRECISION 
STR_CLASS_NAME_IF_CPP
get_time_value_for_timestep(int a_which)
{
  if( a_which<0 || a_which>=g_strGlobals->m_numTimesteps )
  {
     printinfo("get_time_value_for_timestep out of range: a_which=%d, m_numTimesteps=%d\n",
	a_which,g_strGlobals->m_numTimesteps );
     exit(-1);
  }
  return( g_strGlobals->m_timeValues[a_which] );
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get Name of a DSL or SAF Type
 * Description:
 *
 * Returns a const char string like "SAF_INT" or "SAF_FLOAT".
 * 
 *--------------------------------------------------------------------------------------------------- */
const char *
STR_CLASS_NAME_IF_CPP
get_dsl_type_string( hid_t a_varType )
{
/*
      if(H5Tequal(a_varType,SAF_INT )) return("SAF_INT");
      if(H5Tequal(a_varType,SAF_FLOAT )) return("SAF_FLOAT");
      if(H5Tequal(a_varType,SAF_DOUBLE )) return("SAF_DOUBLE");
      if(H5Tequal(a_varType,SAF_CHAR )) return("SAF_CHAR");
      if(H5Tequal(a_varType,SAF_LONG )) return("SAF_LONG");
*/
      /*
      if(H5Tequal(a_varType,SAF_INT8 )) return("SAF_INT8");
      if(H5Tequal(a_varType,SAF_INT16 )) return("SAF_INT16");
      if(H5Tequal(a_varType,SAF_INT32 )) return("SAF_INT32");
      if(H5Tequal(a_varType,SAF_INT64 )) return("SAF_INT64");
      if(H5Tequal(a_varType,SAF_LONG_LONG )) return("SAF_LONG_LONG");
      if(H5Tequal(a_varType,SAF_OFFSET )) return("SAF_OFFSET");
      if(H5Tequal(a_varType,SAF_REFERENCE )) return("SAF_REFERENCE");
      if(H5Tequal(a_varType,SAF_SHORT )) return("SAF_SHORT");
      if(H5Tequal(a_varType,SAF_UNSIGNED_CHAR )) return("SAF_UNSIGNED_CHAR");
      if(H5Tequal(a_varType,SAF_UNSIGNED_INT )) return("SAF_UNSIGNED_INT");
      if(H5Tequal(a_varType,SAF_UNSIGNED_INT8 )) return("SAF_UNSIGNED_INT8");
      if(H5Tequal(a_varType,SAF_UNSIGNED_INT16 )) return("SAF_UNSIGNED_INT16");
      if(H5Tequal(a_varType,SAF_UNSIGNED_INT32 )) return("SAF_UNSIGNED_INT32");
      if(H5Tequal(a_varType,SAF_UNSIGNED_INT64 )) return("SAF_UNSIGNED_INT64");
      if(H5Tequal(a_varType,SAF_UNSIGNED_LONG )) return("SAF_UNSIGNED_LONG");
      if(H5Tequal(a_varType,SAF_UNSIGNED_LONG_LONG )) return("SAF_UNSIGNED_LONG_LONG");
      if(H5Tequal(a_varType,SAF_UNSIGNED_SHORT )) return("SAF_UNSIGNED_SHORT");
      */

      if( a_varType < 0 ) return("INVALID");/*sslib*/

      if(H5Tequal(a_varType,H5T_NATIVE_CHAR         )) return("H5T_NATIVE_CHAR");
      if(H5Tequal(a_varType,H5T_NATIVE_SCHAR        )) return("H5T_NATIVE_SCHAR");
      if(H5Tequal(a_varType,H5T_NATIVE_UCHAR        )) return("H5T_NATIVE_UCHAR");
      if(H5Tequal(a_varType,H5T_NATIVE_SHORT        )) return("H5T_NATIVE_SHORT");
      if(H5Tequal(a_varType,H5T_NATIVE_USHORT       )) return("H5T_NATIVE_USHORT");
      if(H5Tequal(a_varType,H5T_NATIVE_INT          )) return("H5T_NATIVE_INT");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT         )) return("H5T_NATIVE_UINT");
      if(H5Tequal(a_varType,H5T_NATIVE_LONG         )) return("H5T_NATIVE_LONG");
      if(H5Tequal(a_varType,H5T_NATIVE_ULONG        )) return("H5T_NATIVE_ULONG");
      if(H5Tequal(a_varType,H5T_NATIVE_LLONG        )) return("H5T_NATIVE_LLONG");
      if(H5Tequal(a_varType,H5T_NATIVE_ULLONG       )) return("H5T_NATIVE_ULLONG");
      if(H5Tequal(a_varType,H5T_NATIVE_FLOAT_g        )) return("H5T_NATIVE_FLOAT");
      if(H5Tequal(a_varType,H5T_NATIVE_DOUBLE       )) return("H5T_NATIVE_DOUBLE");
      if(H5Tequal(a_varType,H5T_NATIVE_LDOUBLE      )) return("H5T_NATIVE_LDOUBLE");
      if(H5Tequal(a_varType,H5T_NATIVE_B8           )) return("H5T_NATIVE_B8");
      if(H5Tequal(a_varType,H5T_NATIVE_B16          )) return("H5T_NATIVE_B16");
      if(H5Tequal(a_varType,H5T_NATIVE_B32          )) return("H5T_NATIVE_B32");
      if(H5Tequal(a_varType,H5T_NATIVE_B64          )) return("H5T_NATIVE_B64");
      if(H5Tequal(a_varType,H5T_NATIVE_OPAQUE       )) return("H5T_NATIVE_OPAQUE");
      if(H5Tequal(a_varType,H5T_NATIVE_HADDR        )) return("H5T_NATIVE_HADDR");
      if(H5Tequal(a_varType,H5T_NATIVE_HSIZE        )) return("H5T_NATIVE_HSIZE");
      if(H5Tequal(a_varType,H5T_NATIVE_HSSIZE       )) return("H5T_NATIVE_HSSIZE");
      if(H5Tequal(a_varType,H5T_NATIVE_HERR         )) return("H5T_NATIVE_HERR");
      if(H5Tequal(a_varType,H5T_NATIVE_HBOOL        )) return("H5T_NATIVE_HBOOL");

      if(H5Tequal(a_varType,H5T_NATIVE_INT8          )) return("H5T_NATIVE_INT8");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT8         )) return("H5T_NATIVE_UINT8");
      if(H5Tequal(a_varType,H5T_NATIVE_INT_LEAST8          )) return("H5T_NATIVE_INT_LEAST8");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT_LEAST8         )) return("H5T_NATIVE_UINT_LEAST8");
      if(H5Tequal(a_varType,H5T_NATIVE_INT_FAST8          )) return("H5T_NATIVE_INT_FAST8");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT_FAST8         )) return("H5T_NATIVE_UINT_FAST8");

      if(H5Tequal(a_varType,H5T_NATIVE_INT16          )) return("H5T_NATIVE_INT16");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT16         )) return("H5T_NATIVE_UINT16");
      if(H5Tequal(a_varType,H5T_NATIVE_INT_LEAST16          )) return("H5T_NATIVE_INT_LEAST16");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT_LEAST16         )) return("H5T_NATIVE_UINT_LEAST16");
      if(H5Tequal(a_varType,H5T_NATIVE_INT_FAST16          )) return("H5T_NATIVE_INT_FAST16");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT_FAST16         )) return("H5T_NATIVE_UINT_FAST16");

      if(H5Tequal(a_varType,H5T_NATIVE_INT32          )) return("H5T_NATIVE_INT32");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT32         )) return("H5T_NATIVE_UINT32");
      if(H5Tequal(a_varType,H5T_NATIVE_INT_LEAST32          )) return("H5T_NATIVE_INT_LEAST32");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT_LEAST32         )) return("H5T_NATIVE_UINT_LEAST32");
      if(H5Tequal(a_varType,H5T_NATIVE_INT_FAST32          )) return("H5T_NATIVE_INT_FAST32");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT_FAST32         )) return("H5T_NATIVE_UINT_FAST32");

      if(H5Tequal(a_varType,H5T_NATIVE_INT64          )) return("H5T_NATIVE_INT64");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT64         )) return("H5T_NATIVE_UINT64");
      if(H5Tequal(a_varType,H5T_NATIVE_INT_LEAST64          )) return("H5T_NATIVE_INT_LEAST64");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT_LEAST64         )) return("H5T_NATIVE_UINT_LEAST64");
      if(H5Tequal(a_varType,H5T_NATIVE_INT_FAST64          )) return("H5T_NATIVE_INT_FAST64");
      if(H5Tequal(a_varType,H5T_NATIVE_UINT_FAST64         )) return("H5T_NATIVE_UINT_FAST64");


      if(H5Tequal(a_varType,H5T_IEEE_F32BE)) return("H5T_IEEE_F32BE");
      if(H5Tequal(a_varType,H5T_IEEE_F32LE)) return("H5T_IEEE_F32LE");
      if(H5Tequal(a_varType,H5T_IEEE_F64BE)) return("H5T_IEEE_F64BE");
      if(H5Tequal(a_varType,H5T_IEEE_F64LE)) return("H5T_IEEE_F64LE");


      if(H5Tequal(a_varType,H5T_STD_I8BE            )) return("H5T_STD_I8BE");
      if(H5Tequal(a_varType,H5T_STD_I8LE            )) return("H5T_STD_I8LE");
      if(H5Tequal(a_varType,H5T_STD_I16BE           )) return("H5T_STD_I16BE");
      if(H5Tequal(a_varType,H5T_STD_I16LE           )) return("H5T_STD_I16LE");
      if(H5Tequal(a_varType,H5T_STD_I32BE           )) return("H5T_STD_I32BE");
      if(H5Tequal(a_varType,H5T_STD_I32LE           )) return("H5T_STD_I32LE");
      if(H5Tequal(a_varType,H5T_STD_I64BE           )) return("H5T_STD_I64BE");
      if(H5Tequal(a_varType,H5T_STD_I64LE           )) return("H5T_STD_I64LE");
      if(H5Tequal(a_varType,H5T_STD_U8BE            )) return("H5T_STD_U8BE");
      if(H5Tequal(a_varType,H5T_STD_U8LE            )) return("H5T_STD_U8LE");
      if(H5Tequal(a_varType,H5T_STD_U16BE           )) return("H5T_STD_U16BE");
      if(H5Tequal(a_varType,H5T_STD_U16LE           )) return("H5T_STD_U16LE");
      if(H5Tequal(a_varType,H5T_STD_U32BE           )) return("H5T_STD_U32BE");
      if(H5Tequal(a_varType,H5T_STD_U32LE           )) return("H5T_STD_U32LE");
      if(H5Tequal(a_varType,H5T_STD_U64BE           )) return("H5T_STD_U64BE");
      if(H5Tequal(a_varType,H5T_STD_U64LE           )) return("H5T_STD_U64LE");
      if(H5Tequal(a_varType,H5T_STD_B8BE            )) return("H5T_STD_B8BE");
      if(H5Tequal(a_varType,H5T_STD_B8LE            )) return("H5T_STD_B8LE");
      if(H5Tequal(a_varType,H5T_STD_B16BE           )) return("H5T_STD_B16BE");
      if(H5Tequal(a_varType,H5T_STD_B16LE           )) return("H5T_STD_B16LE");
      if(H5Tequal(a_varType,H5T_STD_B32BE           )) return("H5T_STD_B32BE");
      if(H5Tequal(a_varType,H5T_STD_B32LE           )) return("H5T_STD_B32LE");
      if(H5Tequal(a_varType,H5T_STD_B64BE           )) return("H5T_STD_B64BE");
      if(H5Tequal(a_varType,H5T_STD_B64LE           )) return("H5T_STD_B64LE");
      if(H5Tequal(a_varType,H5T_STD_REF_OBJ         )) return("H5T_STD_REF_OBJ");
      if(H5Tequal(a_varType,H5T_STD_REF_DSETREG     )) return("H5T_STD_REF_DSETREG");


      if(H5Tequal(a_varType,H5T_UNIX_D32BE          )) return("H5T_UNIX_D32BE");
      if(H5Tequal(a_varType,H5T_UNIX_D32LE          )) return("H5T_UNIX_D32LE");
      if(H5Tequal(a_varType,H5T_UNIX_D64BE          )) return("H5T_UNIX_D64BE");
      if(H5Tequal(a_varType,H5T_UNIX_D64LE          )) return("H5T_UNIX_D64LE");
      if(H5Tequal(a_varType,H5T_C_S1                )) return("H5T_C_S1");

      /*if(H5Tequal(a_varType,H5I_INVALID_HID )) return("H5I_INVALID_HID");*/


      return("unknown");
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Guess the DSL type for a Composite Coord Field
 * Description:
 *
 * I need this function because saf_describe_field might return H5I_INVALID_HID 
 * for a hid_t, because the data is written on individual fields rather than
 * the composite. In this case, saf_write_field would still succeed. 
 *
 * Note this is a flaw in saf, and could be corrected at any time.
 *
 * Note there is a better way to do this, so I would not have to use the
 * term "guess".
 *
 *--------------------------------------------------------------------------------------------------- */
hid_t 
STR_CLASS_NAME_IF_CPP
guess_dsl_var_type_for_composite_coord_field( int a_numFields, SAF_Field *a_fields )
{
  /* XXX jake hooey 18mar2004: why didnt I just get the actual component fields here?
     I guess I wrote this when I was brand new on saf? */

  int i;
  for(i=0;i<a_numFields;i++)
    {
      hbool_t l_isCoordField;
      hid_t l_varType;
      int l_numComponents=0;
      char *l_name=0;

      saf_describe_field(SAF_ALL, &(a_fields[i]), NULL, &l_name, NULL, NULL,
			 &l_isCoordField, NULL, NULL, NULL, NULL, NULL, &l_varType,  
			 &l_numComponents, NULL, NULL, NULL);

      if( l_numComponents==1 && l_isCoordField && l_varType>=0 )/*sslib? !H5Tequal(l_varType,H5I_INVALID_HID) )*/
	{
	  printinfo("guess_dsl_var_type_for_composite_coord_field found definite field %s out of %d fields\n",
		l_name,a_numFields);
	  if(l_name) free(l_name);
	  return(l_varType);
	}
      if(l_name) free(l_name);
    }

  for(i=0;i<a_numFields;i++)
    {
      hid_t l_varType;
      int l_numComponents=0;
      char *l_name=0;

      saf_describe_field(SAF_ALL, &(a_fields[i]), NULL, &l_name, NULL, NULL,
			 NULL, NULL, NULL, NULL, NULL, NULL, &l_varType,  
			 &l_numComponents, NULL, NULL, NULL);

      if( l_numComponents==1 &&  l_varType>=0 ) /*!H5Tequal(l_varType,H5I_INVALID_HID) )*/
	{
	  printinfo("guess_dsl_var_type_for_composite_coord_field found maybe field %s out of %d fields\n",
		l_name,a_numFields);
	  if(l_name) free(l_name);
	  return(l_varType);
	}
      if(l_name) free(l_name);
    }
  return(H5I_INVALID_HID);
}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Size of a Structured Block from the Set
 * Description:
 *
 * Given a set that is a valid structured mesh block, set the argument pointers with its 
 * X Y and Z sizes.
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
get_block_size_from_set( SAF_Set *a_set, int *a_xCoords, int *a_yCoords, int *a_zCoords )
{
  SAF_IndexSpec l_indexSpec;
  int l_numCollections=0, i;
  SAF_Cat *l_cats=0;
  SAF_CellType l_cellType;

  *a_xCoords=0;
  *a_yCoords=0;
  *a_zCoords=0;
  if(!my_saf_is_valid_set_handle(a_set))
    {
      return;
    }

  saf_describe_set(SAF_ALL,a_set,NULL,NULL,NULL,NULL,NULL,&l_numCollections,&l_cats);
  for( i=0; i<l_numCollections; i++ )
    {
      saf_describe_collection(SAF_ALL,a_set,&(l_cats[i]),&l_cellType,NULL,&l_indexSpec,NULL,NULL);
      if( l_indexSpec.ndims==3 && l_cellType==SAF_CELLTYPE_POINT )
	{
	  *a_xCoords = l_indexSpec.sizes[0];
	  *a_yCoords = l_indexSpec.sizes[1];
	  *a_zCoords = l_indexSpec.sizes[2];

        if( l_indexSpec.order[0]<0 || l_indexSpec.order[0]>=3 ||
            l_indexSpec.order[1]<0 || l_indexSpec.order[1]>=3 ||
            l_indexSpec.order[2]<0 || l_indexSpec.order[2]>=3 )
          {
            printinfo("get_block_size_from_set error: bad SAF_IndexSpec orders %d %d %d\n",
                        l_indexSpec.order[0], l_indexSpec.order[1],l_indexSpec.order[2] );
            exit(-1);
          }
	  return;
	}
    }
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Is There a Variable Instance for Given Block, Variable and Time Indices?
 * Description:
 *
 * Returns 1 if true, 0 otherwise.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
does_var_exist_on_block( int a_whichBlock, int a_whichVar, int a_whichTimestep )
{
  SAF_Field l_field;
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets || a_whichVar<0 || 
      a_whichVar>=g_strGlobals->m_numVarsPerBlock[a_whichBlock] || a_whichTimestep<0 || 
      a_whichTimestep>=g_strGlobals->m_numTimesteps )
    {
      return(0);
    }
  if( !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock] ||
      !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar] )
    {
      return(0);
    }

  l_field = g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar][a_whichTimestep];
  if( my_saf_is_valid_field_handle( &l_field ) ) 
    {
      return(1);
    }
  return(0);
}





/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Given a Field, Get the Structured Mesh Variable Type
 * Description:
 *
 * Gets the structured mesh variable type, one of:  SAF_STR_MESH_NODE, SAF_STR_MESH_XEDGE,
 * SAF_STR_MESH_YEDGE, SAF_STR_MESH_ZEDGE, SAF_STR_MESH_XFACE, SAF_STR_MESH_YFACE, SAF_STR_MESH_ZFACE,
 * SAF_STR_MESH_ELEM or SAF_STR_MESH_UNKNOWN.
 *--------------------------------------------------------------------------------------------------- */
saf_str_mesh_var_type 
STR_CLASS_NAME_IF_CPP
get_variable_type_from_field( SAF_Set a_set, SAF_Field a_field )
{
  hbool_t l_isCoordField;
  SAF_Cat l_cat;
  SAF_CellType l_cellType;
  SAF_IndexSpec l_indexSpec;
  int l_xDim=0,l_yDim=0,l_zDim=0,l_x,l_y,l_z;
  saf_str_mesh_var_type l_result=SAF_STR_MESH_UNKNOWN;

  if( !my_saf_is_valid_field_handle(&a_field) )
    {
      /*printinfo("get_variable_type_from_field trying to map a SS_FIELD_NULL\n");*/
      return(SAF_STR_MESH_UNKNOWN);
    }

  saf_describe_field(SAF_ALL, &(a_field), NULL, NULL, NULL, NULL,
		     &l_isCoordField, NULL, &l_cat, NULL, NULL, 
		     NULL, NULL, NULL, NULL, NULL, NULL);

  /*this shouldn't happen: can remove the check later*/
  if( l_isCoordField ) 
    {
      printinfo("Err in get_variable_type_from_field: variable is a coord field?\n");
      exit(-1);
    }

  saf_describe_collection(SAF_ALL,&a_set,&l_cat,&l_cellType,NULL,&l_indexSpec,NULL,NULL);

  get_block_size_from_set(&a_set,&l_xDim,&l_yDim,&l_zDim);

  l_x = l_indexSpec.sizes[0];
  l_y = l_indexSpec.sizes[1];
  l_z = l_indexSpec.sizes[2];

  if( l_cellType == SAF_CELLTYPE_POINT )
    {
      l_result = SAF_STR_MESH_NODE;
    }
  else if( l_cellType == SAF_CELLTYPE_LINE )
    {
      if( l_xDim-1 == l_x ) l_result=SAF_STR_MESH_XEDGE;
      else if( l_yDim-1 == l_y ) l_result=SAF_STR_MESH_YEDGE;
      else if( l_zDim-1 == l_z ) l_result=SAF_STR_MESH_ZEDGE;
      else l_result=SAF_STR_MESH_UNKNOWN;
    }
  else if( l_cellType == SAF_CELLTYPE_QUAD )
    {
      if( l_xDim == l_x ) l_result=SAF_STR_MESH_XFACE;
      else if( l_yDim == l_y ) l_result=SAF_STR_MESH_YFACE;
      else if( l_zDim == l_z ) l_result=SAF_STR_MESH_ZFACE;
      else l_result=SAF_STR_MESH_UNKNOWN;
    }
  else if( l_cellType == SAF_CELLTYPE_HEX )
    {
      l_result=SAF_STR_MESH_ELEM;
    }

  if(l_result == SAF_STR_MESH_UNKNOWN)
    {
      char *l_fname=0,*l_sname=0;

      if( my_saf_is_valid_set_handle(&a_set) )
	{
	  saf_describe_set(SAF_ALL, &(a_set), &l_sname, NULL, NULL, NULL, NULL, NULL, NULL );
	}
      if( my_saf_is_valid_field_handle(&a_field) )
	{
	  saf_describe_field(SAF_ALL, &(a_field), NULL, &l_fname,NULL, NULL, NULL,
			     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	}

      printinfo("\nget_variable_type_from_field failed to get type for field %s set %s\n\n",
			    l_fname==0 ? "(invalid)":l_fname,  l_sname==0 ? "(invalid)":l_sname);

      if(l_fname) free(l_fname);
      if(l_sname) free(l_sname);
    }



#if 0
  {
    char *l_fname=0,*l_sname=0;
    saf_describe_field(SAF_ALL, &(a_field), NULL, &l_fname,NULL, NULL, NULL,
		       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    saf_describe_set(SAF_ALL, &(a_set), &l_sname, NULL, NULL, NULL, NULL, NULL, NULL );
    printinfo("get_variable_type_from_field got type %d for field %s set %s\n",l_result,l_fname,l_sname);
    if(l_fname) free(l_fname);
    if(l_sname) free(l_sname);
  }
#endif



  return(l_result);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Given a Block-Variable-Timestep Triplet, Get the Structured Mesh Variable Type
 * Description:
 *
 * Gets the structured mesh variable type, one of:  SAF_STR_MESH_NODE, SAF_STR_MESH_XEDGE,
 * SAF_STR_MESH_YEDGE, SAF_STR_MESH_ZEDGE, SAF_STR_MESH_XFACE, SAF_STR_MESH_YFACE, SAF_STR_MESH_ZFACE,
 * SAF_STR_MESH_ELEM or SAF_STR_MESH_UNKNOWN.
 *--------------------------------------------------------------------------------------------------- */
saf_str_mesh_var_type 
STR_CLASS_NAME_IF_CPP
get_variable_type( int a_whichBlock, int a_whichVar, int a_whichTimestep )
{
  SAF_Field l_field;
  saf_str_mesh_var_type result=SAF_STR_MESH_UNKNOWN;

  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets )
    {
      printinfo("error get_variable_type a_whichBlock out of range: a_whichBlock=%d g_strGlobals->m_numSets=%d\n",
		a_whichBlock,g_strGlobals->m_numSets);
      exit(-1);
    }

  if( a_whichVar<0 || 
      a_whichVar>=g_strGlobals->m_numVarsPerBlock[a_whichBlock] || a_whichTimestep<0 || 
      a_whichTimestep>=g_strGlobals->m_numTimesteps  )
    {
      /*printinfo("get_variable_type something out of range:  a_whichVar=%d(%d), a_whichTimestep=%d(%d)\n",
			    a_whichVar,g_strGlobals->m_numVarsPerBlock[a_whichBlock],a_whichTimestep,g_strGlobals->m_numTimesteps );*/
      return(SAF_STR_MESH_UNKNOWN);
    }

  if( !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock] ||
      !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar] )
    {
      printinfo("get_variable_type m_instancePerTimestepPerVarPerBlock[%d][%d] not allocated\n",a_whichBlock,a_whichVar);
      return(SAF_STR_MESH_UNKNOWN);
    }
  l_field = g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar][a_whichTimestep];

  result = get_variable_type_from_field( g_strGlobals->m_sets[a_whichBlock], l_field );

  /*printinfo("    get_variable_type(%d,%d,%d) returns %d\n",a_whichBlock,a_whichVar,a_whichTimestep,result);*/

  return(result);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Size of a Variable
 * Description:
 * 
 * Given a block-variable-timestep triplet, get the X,Y Z size of the actual field data.
 * If the variable is read with read_simple_variable, then XYZ is the size of the data array.
 * If the variable is read with read_variable, then XYZ is be the size BEFORE
 * it is interpolated into a nodal variable (note that hex element variables are NOT
 * interpolated into nodal variables, because most visualization tools have nodal
 * and hex element color mapping capabilities, but few have edge and face capabilities).
 *
 * Return:      0 on success, -1 otherwise
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_variable_size( int a_whichBlock,
		       int a_whichVar,
		       int a_whichTimestep, int *a_x, int *a_y, int *a_z )
{
  SAF_Field l_field;
  SAF_Set l_set;
  SAF_Cat l_cat;
  SAF_FieldTmpl l_fieldTemplate;
  SAF_IndexSpec l_indexSpec;
  int l_numEntriesInColl=0;

  a_x[0]=a_y[0]=a_z[0]=0;

  l_field = g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar][a_whichTimestep];
  if( !my_saf_is_valid_field_handle(&l_field) ) 
    {
      printinfo("error get_variable_size trying to map a SS_FIELD_NULL\n");
      return(-1);
    }

  saf_describe_field(SAF_ALL, &(l_field), &l_fieldTemplate, NULL, &l_set, NULL, 
		     NULL, NULL, &l_cat, NULL, NULL, 
		     NULL, NULL, NULL, NULL, NULL, NULL);

  saf_describe_collection(SAF_ALL,&l_set,&l_cat,NULL,&l_numEntriesInColl,&l_indexSpec,NULL,NULL);
  
  if( l_indexSpec.ndims == 3 )
    {
      a_x[0] = l_indexSpec.sizes[0];
      a_y[0] = l_indexSpec.sizes[1];
      a_z[0] = l_indexSpec.sizes[2];
      if( a_x[0]*a_y[0]*a_z[0] != l_numEntriesInColl )
	{
	  printinfo("error get_variable_size: %d*%d*%d != %d\n",a_x[0],a_y[0],a_z[0],l_numEntriesInColl);
	  exit(-1);
	}
    }
  else if( l_indexSpec.ndims == 2 )
    {
      a_x[0] = l_indexSpec.sizes[0];
      a_y[0] = l_indexSpec.sizes[1];
      a_z[0] = 1;
      if( a_x[0]*a_y[0] != l_numEntriesInColl )
	{
	  printinfo("error get_variable_size: %d*%d != %d\n",a_x[0],a_y[0],l_numEntriesInColl);
	  exit(-1);
	}
    }
  else if( l_indexSpec.ndims == 1 )
    {
      a_x[0] = l_indexSpec.sizes[0];
      a_y[0] = 1;
      a_z[0] = 1;
      if( a_x[0] != l_numEntriesInColl )
	{
	  printinfo("error get_variable_size: %d != %d\n",a_x[0],l_numEntriesInColl);
	  exit(-1);
	}
    }


  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read a Variable
 * Description:
 * 
 * Given a block-variable-timestep triplet, read the variable data into a_data.
 * If no instance is found then a_data will be written over with zeros. The
 * calling function must allocate a_data to the correct size. If the variable
 * is a hex element variable, the correct size is (X-1)(Y-1)(Z-1), where X, Y and Z
 * are the size of the block, found by get_block_size. If the variable is not
 * a hex element variable, then the correct size is XYZ.
 *
 * Note that this function will call read_edge_or_face_variable_as_node_variable if the variable is 
 * an edge or face type, otherwise it will call read_node_variable.
 *
 * Return:      0 on success, -1 on error, and 1 if no instance was found
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_variable( int a_whichBlock,
		   int a_whichVar,
		   int a_whichTimestep,
		   MY_PRECISION *a_data )
{
  int l_result=-1;
  saf_str_mesh_var_type l_strMeshVarType = get_variable_type( a_whichBlock, a_whichVar, a_whichTimestep );

  printinfo(" entering str mesh read_variable, bl=%d, var=%d, ts=%d,  l_strMeshVarType=%d\n", 
	 a_whichBlock,a_whichVar,a_whichTimestep,l_strMeshVarType );

  if( l_strMeshVarType == SAF_STR_MESH_NODE )
    {
      l_result=read_node_variable(a_whichBlock,a_whichVar,a_whichTimestep,a_data);
    }
  else if( l_strMeshVarType == SAF_STR_MESH_ELEM )
    {
      l_result=read_node_variable(a_whichBlock,a_whichVar,a_whichTimestep,a_data);
    }
  else if( l_strMeshVarType==SAF_STR_MESH_XEDGE ||
	   l_strMeshVarType==SAF_STR_MESH_YEDGE ||
	   l_strMeshVarType==SAF_STR_MESH_ZEDGE ||
	   l_strMeshVarType==SAF_STR_MESH_XFACE ||
	   l_strMeshVarType==SAF_STR_MESH_YFACE ||
	   l_strMeshVarType==SAF_STR_MESH_ZFACE )
    {
      l_result=read_edge_or_face_variable_as_node_variable(a_whichBlock,a_whichVar,a_whichTimestep,a_data);
    }
  else printinfo("  IN READ_VARIABLE, UNPREPARED FOR l_strMeshVarType=%d\n",l_strMeshVarType);

  if( l_result )
    {
      /*For some reason, the variable read failed. Write over the data with
	zeros so it will be apparent to the viewer*/
      int i;
      int l_blockXDim=0,l_blockYDim=0,l_blockZDim=0,l_numEntries=0;


      get_block_size_from_set(&(g_strGlobals->m_sets[a_whichBlock]),&l_blockXDim,&l_blockYDim,&l_blockZDim);
      l_strMeshVarType = g_strGlobals->m_strMeshVarType[a_whichVar];

      printinfo("read_variable failed for some reason: writing over buffer with 0's (block size %d %d %d)(type %s)\n",
			    l_blockXDim,l_blockYDim,l_blockZDim,get_var_type_name(l_strMeshVarType) );

      if( l_strMeshVarType == SAF_STR_MESH_NODE ||
	  l_strMeshVarType == SAF_STR_MESH_XEDGE ||
	  l_strMeshVarType == SAF_STR_MESH_YEDGE ||
	  l_strMeshVarType == SAF_STR_MESH_ZEDGE ||
	  l_strMeshVarType == SAF_STR_MESH_XFACE ||
	  l_strMeshVarType == SAF_STR_MESH_YFACE ||
	  l_strMeshVarType == SAF_STR_MESH_ZFACE )
	{
	  l_numEntries=l_blockXDim*l_blockYDim*l_blockZDim;
	  for(i=0;i<l_numEntries;i++) a_data[i]=0;
	  l_result=0;/*note:if we pass an error back to ensight, the buffer isnt refreshed*/
	}
      else if( l_strMeshVarType == SAF_STR_MESH_ELEM )
	{
	  if( l_blockXDim>0 && l_blockYDim>0 && l_blockZDim>0 )
	  {
	    l_numEntries=(l_blockXDim-1)*(l_blockYDim-1)*(l_blockZDim-1);
	    for(i=0;i<l_numEntries;i++) a_data[i]=0;
	  }
 	  else l_numEntries=0;
	  l_result=0;/*note:if we pass an error back to ensight, the buffer isnt refreshed*/
	}
      else 
	{
	  printinfo("  warning: dont yet know how to zero var type %s in read_variable\n",
				get_var_type_name(l_strMeshVarType) );
	}
    }


#if 0 /*print out the min/max*/
    else
    {
      int i,l_blockXDim=0,l_blockYDim=0,l_blockZDim=0,l_numEntries=0;
      MY_PRECISION l_min,l_max;
      get_block_size_from_set(&(g_strGlobals->m_sets[a_whichBlock]),&l_blockXDim,&l_blockYDim,&l_blockZDim);
      if( l_strMeshVarType == SAF_STR_MESH_NODE ||
          l_strMeshVarType == SAF_STR_MESH_XEDGE ||
          l_strMeshVarType == SAF_STR_MESH_YEDGE ||
          l_strMeshVarType == SAF_STR_MESH_ZEDGE ||
          l_strMeshVarType == SAF_STR_MESH_XFACE ||
          l_strMeshVarType == SAF_STR_MESH_YFACE ||
          l_strMeshVarType == SAF_STR_MESH_ZFACE )
        {
          l_numEntries=l_blockXDim*l_blockYDim*l_blockZDim;
        }
      else if( l_strMeshVarType == SAF_STR_MESH_ELEM )
        {
          l_numEntries=(l_blockXDim-1)*(l_blockYDim-1)*(l_blockZDim-1);
        }
      l_min=l_max=a_data[0];
      for(i=0;i<l_numEntries;i++)
        {
                if(l_min>a_data[i]) l_min = a_data[i];
                if(l_max<a_data[i]) l_max = a_data[i];
        }
        printinfo("read_variable l_min=%f l_max=%f\n",l_min,l_max);
    }
#endif




  return(l_result);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read an Edge or Face Variable as a Node Variable
 * Description:
 *
 * Given a block-variable-timestep triplet, read edge or face variable data into a_data.
 * The edge or face variable data is linearly interpolated into nodal data.
 *
 * The calling function must allocate a_data to the correct size. The correct size is XYZ, where 
 * X, Y and Z are the size of the block, found by get_block_size.
 *
 * Note that this function will be called by read_variable if the variable is an edge or face type.
 *
 * Return:      0 on success, -1 otherwise
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_edge_or_face_variable_as_node_variable( int a_whichBlock,
						 int a_whichVar,
						 int a_whichTimestep,
						 MY_PRECISION *a_data )
{
  hid_t l_varType;
  SAF_Cat l_cat;
  SAF_Field l_field;
  SAF_FieldTmpl l_fieldTemplate;
  SAF_Set l_set;
  SAF_CellType l_cellType;
  SAF_IndexSpec l_indexSpec;
  int l_numEntriesInColl=0;
  int l_blockXDim=0,l_blockYDim=0,l_blockZDim=0,l_numEntries=0,i,j,k;
  int l_collXDim=0,l_collYDim=0,l_collZDim=0;
  struct_hyperslab l_slab;
  int l_finalX,l_finalY,l_finalZ;
  int *l_countBuffer=0;
  MY_PRECISION *l_ptr=0;
  int l_strMeshVarType = get_variable_type( a_whichBlock, a_whichVar, a_whichTimestep );
  int l_xCollPosition,l_yCollPosition,l_zCollPosition;


  printinfo("\nentering read_edge_or_face_variable_as_node_variable\n");

  /*do we need all of these checks?*/
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets || a_whichVar<0 || 
      a_whichVar>=g_strGlobals->m_numVarsPerBlock[a_whichBlock] || a_whichTimestep<0 || 
      a_whichTimestep>=g_strGlobals->m_numTimesteps )
    {
      printinfo("read_edge_or_face_variable_as_node_variable error something out of range: a_whichBlock=%d, a_whichVar=%d, a_whichTimestep=%d\n",
		a_whichBlock,a_whichVar,a_whichTimestep );
      return(-1);
    }
  if( !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock] ||
      !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar] )
    {
      printinfo("read_edge_or_face_variable_as_node_variable error m_instancePerTimestepPerVarPerBlock[%d][%d] not allocated\n",
		a_whichBlock,a_whichVar);
      return(-1);
    }


  l_field = g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar][a_whichTimestep];
  if( !my_saf_is_valid_field_handle(&l_field) ) 
    {
      printinfo("read_edge_or_face_variable_as_node_variable error trying to map a SS_FIELD_NULL\n");
      return(-1);
    }

  saf_describe_field(SAF_ALL, &(l_field), &l_fieldTemplate, NULL, &l_set, NULL, 
		     NULL, NULL, &l_cat, NULL, NULL, 
		     NULL, &l_varType, NULL, NULL, NULL, NULL);
 

  /*verify that the field is mapped to edges or faces*/
  saf_describe_collection(SAF_ALL,&l_set,&l_cat,&l_cellType,&l_numEntriesInColl,&l_indexSpec,NULL,NULL);
  if( l_cellType!=SAF_CELLTYPE_LINE && l_cellType!=SAF_CELLTYPE_QUAD )
    {
      printinfo("Skipping read_edge_or_face_variable_as_node_variable: variable is not a SAF_CELLTYPE_LINE or QUAD\n");
      return(-1);
    }
  if(l_numEntriesInColl<=0)
    {
      printinfo("read_edge_or_face_variable_as_node_variable, l_numEntriesInColl=%d?\n",l_numEntriesInColl);
      return(0);
    }


  get_block_size_from_set(&(g_strGlobals->m_sets[a_whichBlock]),&l_blockXDim,&l_blockYDim,&l_blockZDim);
  l_numEntries=l_blockXDim*l_blockYDim*l_blockZDim;


  l_collXDim = l_indexSpec.sizes[0];
  l_collYDim = l_indexSpec.sizes[1];
  l_collZDim = l_indexSpec.sizes[2];


  /*set the hyperslab to the default: every entry*/
  l_slab.x_start = 0;
  l_slab.y_start = 0;
  l_slab.z_start = 0;
  l_slab.x_count = l_collXDim;
  l_slab.y_count = l_collYDim;
  l_slab.z_count = l_collZDim;
  l_slab.x_skip = 1;
  l_slab.y_skip = 1;
  l_slab.z_skip = 1;


  l_countBuffer = (int *)malloc(l_numEntries*sizeof(int));
  for(i=0;i<l_numEntries;i++) 
    {
      a_data[i]=0;
      l_countBuffer[i]=0;
    }

  /*if this is a subset, get the correct hyperslab*/
  if( !SAF_EQUIV( &(g_strGlobals->m_sets[a_whichBlock]), &l_set ) )
    {

#if 1
      /*9/19/02 fix: was using l_cat, but should only be using the "nodes" category,
	so this was failing if l_cat is "edges" e.g. ......I havent got the nodes 
	category stored globally, so will try SAF_ANY_CAT.....might be errors later*/
      if( get_subset_hyperslab(&(g_strGlobals->m_sets[a_whichBlock]),&l_set,SAF_ANY_CAT,&l_slab) )
#else 
	if( get_subset_hyperslab(&(g_strGlobals->m_sets[a_whichBlock]),&l_set,&l_cat,&l_slab) )
#endif
	  {
	    printinfo("read_edge_or_face_variable_as_node_variable failed: cant get subset: maybe subset of subset, not written?\n");
	    return(-1);
	  }

      /*If the SAF_IndexSpec.origin's are not zero, then the hyperslab will
	not be the way we want it (0-based). Change it.*/
      l_slab.x_start -= l_indexSpec.origins[0];
      l_slab.y_start -= l_indexSpec.origins[1];
      l_slab.z_start -= l_indexSpec.origins[2];
    }

  l_finalX = l_slab.x_start + l_slab.x_count*l_slab.x_skip;
  l_finalY = l_slab.y_start + l_slab.y_count*l_slab.y_skip;
  l_finalZ = l_slab.z_start + l_slab.z_count*l_slab.z_skip;


  /*Check that the slab makes sense. In addition to other possible errors, 
    I'm not sure that everyone agrees that a hyperslab should be 1-based
    if the collection is 1-based*/
  if( l_slab.x_start<0 || l_slab.y_start<0 || l_slab.z_start<0 ||
      l_finalX-l_slab.x_skip>=l_blockXDim || l_finalY-l_slab.y_skip>=l_blockYDim 
      || l_finalZ-l_slab.z_skip>=l_blockZDim )
    {
      printinfo("read_edge_or_face_variable_as_node_variable error: hyperslab is bad\n");
      printinfo("  l_final*: %d %d %d\n",l_finalX,l_finalY,l_finalZ);
      printinfo("  block dim: %d %d %d\n",l_blockXDim,l_blockYDim,l_blockZDim);
      printinfo("  l_slab starts: %d %d %d\n",l_slab.x_start,l_slab.y_start,l_slab.z_start );
      printinfo("  l_slab counts: %d %d %d\n",l_slab.x_count,l_slab.y_count,l_slab.z_count );
      printinfo("  l_slab skips: %d %d %d\n",l_slab.x_skip,l_slab.y_skip,l_slab.z_skip );
      printinfo("  ispec origins: %d %d %d\n",l_indexSpec.origins[0],l_indexSpec.origins[1],l_indexSpec.origins[2] );

      exit(-1);
    }


  checkBufferSize(l_numEntriesInColl);
  if( read_whole_field_to_my_precision(&l_field,l_varType,(size_t)l_numEntriesInColl,g_strGlobals->m_buffer) )
    {
      printinfo("read_edge_or_face_variable_as_node_variable error failed to read field?\n");
      return(-1);
    }


  /*Change the indexing mode to fortran order. Note: this doesnt cost
    anything if the ordering was already fortran*/
  {
    SAF_IndexSpec l_indexSpecCorrected = l_indexSpec;
    l_indexSpecCorrected.order[0]=2;
    l_indexSpecCorrected.order[1]=1;
    l_indexSpecCorrected.order[2]=0;
    change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, sizeof(MY_PRECISION), (void *)g_strGlobals->m_buffer );
  }



  l_ptr=g_strGlobals->m_buffer;


  /*Translate the edge or face subset values into nodal values on the whole set.

  The size of the whole set of nodes is: 
  l_numEntries=l_blockXDim*l_blockYDim*l_blockZDim 
  (a_data and l_countBuffer are this size)

  The size of the node subset (a subset of the whole set) on which the var is
  mapped is: l_slab.x_count*l_slab.y_count*l_slab.z_count

  The size of the collection on the subset, to which the var is mapped is:
  l_numEntriesInColl=l_collXDim*l_collYDim*l_collZDim (l_indexSpec)
  (m_buffer has this size of valid data)
  */
  l_zCollPosition=0;
  for(k=l_slab.z_start;k<l_finalZ;k+=l_slab.z_skip)
    {
      l_yCollPosition=0;
      for(j=l_slab.y_start;j<l_finalY;j+=l_slab.y_skip)
	{
	  l_xCollPosition=0;
	  for(i=l_slab.x_start;i<l_finalX;i+=l_slab.x_skip)
	    {
	      int l_index1=0,l_index2=0,l_index3=0,l_index4=0;
	      if( l_strMeshVarType == SAF_STR_MESH_XEDGE )
		{
		  l_index1=i+  j*l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index2=i+1+j*l_blockXDim+k*l_blockXDim*l_blockYDim;
		}
	      else if( l_strMeshVarType == SAF_STR_MESH_YEDGE )
		{
		  l_index1=i+(j)  *l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index2=i+(j+1)*l_blockXDim+k*l_blockXDim*l_blockYDim;
		}
	      else if( l_strMeshVarType == SAF_STR_MESH_ZEDGE )
		{
		  l_index1=i+j*l_blockXDim+(k  )*l_blockXDim*l_blockYDim;
		  l_index2=i+j*l_blockXDim+(k+1)*l_blockXDim*l_blockYDim;
		}
	      else if( l_strMeshVarType == SAF_STR_MESH_XFACE )
		{
		  l_index1=i+j*l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index2=i+(j+1)*l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index3=i+(j+1)*l_blockXDim+(k+1)*l_blockXDim*l_blockYDim;
		  l_index4=i+j*l_blockXDim+(k+1)*l_blockXDim*l_blockYDim;
		}
	      else if( l_strMeshVarType == SAF_STR_MESH_YFACE )
		{
		  l_index1=i+j*l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index2=i+1+j*l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index3=i+1+j*l_blockXDim+(k+1)*l_blockXDim*l_blockYDim;
		  l_index4=i+j*l_blockXDim+(k+1)*l_blockXDim*l_blockYDim;
		}
	      else if( l_strMeshVarType == SAF_STR_MESH_ZFACE )
		{
		  l_index1=i+j*l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index2=i+1+j*l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index3=i+1+(j+1)*l_blockXDim+k*l_blockXDim*l_blockYDim;
		  l_index4=i+(j+1)*l_blockXDim+k*l_blockXDim*l_blockYDim;
		}

	      a_data[l_index1] += l_ptr[0];
	      a_data[l_index2] += l_ptr[0];
	      l_countBuffer[l_index1] += 1;
	      l_countBuffer[l_index2] += 1;

	      if( l_cellType==SAF_CELLTYPE_QUAD )
		{
		  a_data[l_index3] += l_ptr[0];
		  a_data[l_index4] += l_ptr[0];
		  l_countBuffer[l_index3] += 1;
		  l_countBuffer[l_index4] += 1;
		}
	      /*
		printinfo("l_indexes=%d %d   data=%f   ijk=%d %d %d    counts=%d %d\n",l_index1,l_index2,l_ptr[0],i,j,k , 
		l_countBuffer[l_index1], l_countBuffer[l_index2] );
	      */
	      if(l_index1>=l_numEntries || l_index2>=l_numEntries || l_index3>=l_numEntries || l_index4>=l_numEntries)
		{
		  printinfo("read_edge_or_face_variable_as_node_variable error l_index's=%d %d %d %d   l_numEntries=%d\n",
			    l_index1,l_index2,l_index3,l_index4,l_numEntries );

		  printinfo("         l_numEntriesInColl=%d\n",l_numEntriesInColl);
		  printinfo("         hyperslab: %d,%d,%d   %d,%d,%d   %d,%d,%d\n\n",
			    l_slab.x_start,l_slab.x_count,l_slab.x_skip,
			    l_slab.y_start,l_slab.y_count,l_slab.y_skip,
			    l_slab.z_start,l_slab.z_count,l_slab.z_skip );

		  exit(-1);
		}
	      l_ptr++;

	      l_xCollPosition++;
	      if(l_xCollPosition>=l_collXDim) break;
	    }
	  l_yCollPosition++;
	  if(l_yCollPosition>=l_collYDim) break;
	}
      l_zCollPosition++;
      if(l_zCollPosition>=l_collZDim) break;
    }

  {
    int l_numMissed=0;
    for(i=0;i<l_numEntries;i++) 
      {
	if(l_countBuffer[i])
	  {
	    a_data[i] = a_data[i]/l_countBuffer[i];
	  }
	else
	  {
	    /*what to do here? zero it? an average? nearest neighbor?*/
	    l_numMissed++;
	  }
      }
    printinfo("read_edge_or_face_variable_as_node_variable: num entries with no value=%d of %d \t(%.2f%)\n",
	      l_numMissed, l_numEntries, 100*(double)l_numMissed/(double)l_numEntries );
  }

  if(l_countBuffer) free(l_countBuffer);

  return(0);
}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read a Node or Hex Element Variable
 * Description:
 *
 * Given a block-variable-timestep triplet, read the variable into a_data.
 *
 * The calling function must allocate a_data to the correct size. If it is a nodal variable, then the correct  
 * size is XYZ, where X, Y and Z are the size of the block, found by get_block_size. If it is a hex element
 * variable, the the correct size is (X-1)(Y-1)(Z-1).
 *
 * Note that this function will be called by read_variable if the variable is a node or elem type.
 *
 * Return:      0 on success, -1 otherwise
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_node_variable( int a_whichBlock,
			int a_whichVar,
			int a_whichTimestep,
			MY_PRECISION *a_data )
{
  hbool_t l_isCoordField;
  hid_t l_varType;
  SAF_Cat l_cat;
  SAF_Field l_field;
  SAF_FieldTmpl l_fieldTemplate;
  SAF_Set l_set;
  SAF_CellType l_cellType;
  SAF_IndexSpec l_indexSpec;
  int l_numEntriesInColl=0;

  /*do we need all of these checks?*/
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets || a_whichVar<0 || 
      a_whichVar>=g_strGlobals->m_numVarsPerBlock[a_whichBlock] || a_whichTimestep<0 || 
      a_whichTimestep>=g_strGlobals->m_numTimesteps )
    {
      printinfo("read_node_variable something out of range: a_whichBlock=%d, a_whichVar=%d, a_whichTimestep=%d\n",
		a_whichBlock,a_whichVar,a_whichTimestep );
      return(-1);
    }
  if( !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock] ||
      !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar] )
    {
      printinfo("read_node_variable m_instancePerTimestepPerVarPerBlock[%d][%d] not allocated\n",a_whichBlock,a_whichVar);
      return(-1);
    }


  l_field = g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar][a_whichTimestep];
  if( !my_saf_is_valid_field_handle(&l_field)  ) 
    {
      printinfo("read_node_variable trying to map a SS_FIELD_NULL\n");
      return(-1);
    }

  saf_describe_field(SAF_ALL, &(l_field), &l_fieldTemplate, NULL, &l_set, NULL, 
		     &l_isCoordField, NULL, &l_cat, NULL, NULL, 
		     NULL, &l_varType, NULL, NULL, NULL, NULL);


  /*this shouldn't happen: can remove the check later*/
  if( l_isCoordField ) 
    {
      printinfo("error in read_node_variable: variable is a coord field?\n");
      return(-1);
    }


  /*verify that the field is mapped to points: havent written edges,faces,or elems yet*/
  saf_describe_collection(SAF_ALL,&l_set,&l_cat,&l_cellType,&l_numEntriesInColl,&l_indexSpec,NULL,NULL);
  
  if( l_cellType!=SAF_CELLTYPE_POINT && l_cellType!=SAF_CELLTYPE_HEX )
    {
      printinfo("Skipping read_node_variable: variable is not a SAF_CELLTYPE_POINT or _HEX\n");
      return(-1);
    }
  
  if(l_numEntriesInColl<=0)
    {
      printinfo("read_node_variable, l_numEntriesInColl=%d?\n",l_numEntriesInColl);
      return(0);
    }


  /*now check if this var is mapped to the topset or to a subset*/
  if( SAF_EQUIV( &(g_strGlobals->m_sets[a_whichBlock]), &l_set ) )
    {
      /*printinfo("  field is mapped to topset\n");*/
      if( read_whole_field_to_my_precision(&l_field,l_varType,(size_t)l_numEntriesInColl,a_data) )
	{
	  printinfo("read_node_variable error: failed to read field?\n");
	  return(-1);
	}

      /*Change the indexing mode to fortran order. Note: this doesnt cost
	anything if the ordering was already fortran*/
      {
	SAF_IndexSpec l_indexSpecCorrected = l_indexSpec;
	l_indexSpecCorrected.order[0]=2;
	l_indexSpecCorrected.order[1]=1;
	l_indexSpecCorrected.order[2]=0;
	change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, sizeof(MY_PRECISION), a_data );
      }

    }
  else
    {
      /*the field is on a subset of the top set*/
      struct_hyperslab l_slab;

      int l_blockXDim=0,l_blockYDim=0,l_blockZDim=0;
      checkBufferSize(l_numEntriesInColl);
      /*printinfo("  field is mapped to SUBset\n");*/

      get_block_size_from_set(&(g_strGlobals->m_sets[a_whichBlock]),&l_blockXDim,&l_blockYDim,&l_blockZDim);
      /*l_numEntries=l_blockXDim*l_blockYDim*l_blockZDim;*/


      if( l_cellType == SAF_CELLTYPE_POINT )
	{
	  /*ok as is*/
	}
      else if( l_cellType == SAF_CELLTYPE_HEX )
	{
	  l_blockXDim -= 1;
	  l_blockYDim -= 1;
	  l_blockZDim -= 1;
	  /*l_numEntries=l_blockXDim*l_blockYDim*l_blockZDim;*/
	}



#if 1
      /*9/19/02 fix: was using l_cat, but should only be using the "nodes" category,
	so this was failing if l_cat is "edges" e.g. ......I havent got the nodes 
	category stored globally, so will try SAF_ANY_CAT.....might be errors later*/
      if( get_subset_hyperslab(&(g_strGlobals->m_sets[a_whichBlock]),&l_set,SAF_ANY_CAT,&l_slab) )
#else 
	if( get_subset_hyperslab(&(g_strGlobals->m_sets[a_whichBlock]),&l_set,&l_cat,&l_slab) )
#endif
	  {
	    printinfo("read_node_variable error: failed to get subset hyperslab?\n");
	    return(-1);
	  }

      /*If the SAF_IndexSpec.origin's are not zero, then the hyperslab will
	not be the way we want it (0-based). Change it.*/
      l_slab.x_start -= l_indexSpec.origins[0];
      l_slab.y_start -= l_indexSpec.origins[1];
      l_slab.z_start -= l_indexSpec.origins[2];


      /*Check that the slab makes sense. In addition to other possible errors, 
	I'm not sure that everyone agrees that a hyperslab should be 1-based
	if the collection is 1-based*/
      {
	int l_finalX = l_slab.x_start + l_slab.x_count*l_slab.x_skip;
	int l_finalY = l_slab.y_start + l_slab.y_count*l_slab.y_skip;
	int l_finalZ = l_slab.z_start + l_slab.z_count*l_slab.z_skip;
	if( l_slab.x_start<0 || l_slab.y_start<0 || l_slab.z_start<0 ||
	    l_finalX-l_slab.x_skip>=l_blockXDim || l_finalY-l_slab.y_skip>=l_blockYDim 
	    || l_finalZ-l_slab.z_skip>=l_blockZDim  )
	  {
	    printinfo("read_node_variable error: hyperslab is bad\n");
	    printinfo("  l_final*: %d %d %d\n",l_finalX,l_finalY,l_finalZ);
	    printinfo("  block dim: %d %d %d\n",l_blockXDim,l_blockYDim,l_blockZDim);
	    printinfo("  l_slab starts: %d %d %d\n",l_slab.x_start,l_slab.y_start,l_slab.z_start );
	    printinfo("  l_slab counts: %d %d %d\n",l_slab.x_count,l_slab.y_count,l_slab.z_count );
	    printinfo("  l_slab skips: %d %d %d\n",l_slab.x_skip,l_slab.y_skip,l_slab.z_skip );
	    printinfo("  ispec origins: %d %d %d\n",l_indexSpec.origins[0],l_indexSpec.origins[1],l_indexSpec.origins[2] );

	    exit(-1);
	  }
      }



      if( read_whole_field_to_my_precision(&l_field,l_varType,(size_t)l_numEntriesInColl,g_strGlobals->m_buffer) )
	{
	  printinfo("read_node_variable error failed to read field?\n");
	  return(-1);
	}


      /*Change the indexing mode to fortran order. Note: this doesnt cost
	anything if the ordering was already fortran*/
      {
	SAF_IndexSpec l_indexSpecCorrected = l_indexSpec;
	l_indexSpecCorrected.order[0]=2;
	l_indexSpecCorrected.order[1]=1;
	l_indexSpecCorrected.order[2]=0;
	change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, sizeof(MY_PRECISION), g_strGlobals->m_buffer );
      }



      if( fill_data_from_hyperslab(l_blockXDim,l_blockYDim,l_blockZDim,l_slab,g_strGlobals->m_buffer,a_data) )
	{
	  return(-1);
	}
    }


#if 0 /*********************BEGIN TEMPORARY TEST******************************/
  /*testing change_ordering_scheme error temporary*/
  /*note: this only tests that you can do it twice and get
    back to where you started*/
  {
    SAF_IndexSpec l_ispecA = l_indexSpec;
    SAF_IndexSpec l_ispecB = l_indexSpec;
    MY_PRECISION *l_temp;
    int i, l_num,l_blockXDim,l_blockYDim,l_blockZDim;
    printinfo("\ntesting change_ordering_scheme in read_node_variable:\n");

    get_block_size_from_set(&(g_strGlobals->m_sets[a_whichBlock]),&l_blockXDim,&l_blockYDim,&l_blockZDim);
    l_num = l_blockXDim*l_blockYDim*l_blockZDim;

    l_temp = (MY_PRECISION *)malloc(l_num*sizeof(MY_PRECISION));
    memcpy( l_temp, a_data, l_num*sizeof(MY_PRECISION) );
    
    l_ispecB.order[0] = 0; l_ispecB.order[1] = 1; l_ispecB.order[2] = 2;
    change_ordering_scheme( l_ispecA, l_ispecB, (int)H5Tget_size(MY_SAF_PRECISION), (void *)a_data );
    
    l_ispecA = l_ispecB; l_ispecB = l_indexSpec;
    change_ordering_scheme( l_ispecA, l_ispecB, (int)H5Tget_size(MY_SAF_PRECISION), (void *)a_data );
    
    for(i=0;i<l_num;i++)
      {
	if(l_temp[i] != a_data[i])
	  {
	    printinfo("  error i=%d doesnt match: %f %f\n",i,l_temp[i],a_data[i]);
	    exit(-1);
	  }
      }
    free(l_temp);
    printinfo("finished testing change_ordering_scheme:\n\n");
  }
  

  /*testing change_ordering_scheme error temporary:
    now try testing with a different type*/
  {
    SAF_IndexSpec l_ispecA = l_indexSpec;
    SAF_IndexSpec l_ispecB = l_indexSpec;
    double *l_tempin, *l_tempout;
    int i, l_num,l_blockXDim,l_blockYDim,l_blockZDim;
    printinfo("\ntesting change_ordering_scheme in read_node_variable: part 2\n");

    get_block_size_from_set(&(g_strGlobals->m_sets[a_whichBlock]),&l_blockXDim,&l_blockYDim,&l_blockZDim);
    l_num = l_blockXDim*l_blockYDim*l_blockZDim;

    l_tempin = (double *)malloc(l_num*sizeof(double));
    l_tempout = (double *)malloc(l_num*sizeof(double));
    for(i=0;i<l_num;i++) l_tempin[i] = (double)rand();
    memcpy(l_tempout,l_tempin,l_num*sizeof(double));
    
    l_ispecB.order[0] = 0; l_ispecB.order[1] = 1; l_ispecB.order[2] = 2;
    change_ordering_scheme( l_ispecA, l_ispecB, sizeof(double), (void *)l_tempin );
    
    l_ispecA = l_ispecB; l_ispecB = l_indexSpec;
    change_ordering_scheme( l_ispecA, l_ispecB, sizeof(double), (void *)l_tempin );
    
    for(i=0;i<l_num;i++)
      {
	if(l_tempin[i] != l_tempout[i])
	  {
	    printinfo("  error i=%d doesnt match: %f %f\n",i,l_tempin[i],l_tempout[i]);
	    exit(-1);
	  }
      }
    free(l_tempin);
    free(l_tempout);
    printinfo("finished testing change_ordering_scheme: part 2\n\n");
  }
  
#endif /*************************END OF TEMPORARY TEST**************************/



  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read a Variable Simply
 * Description:
 *
 * Reads a variable simply by reading the field: not worrying about
 * translating the data to fit a block, subset, or whatever.
 *
 * The calling function must allocate a_data to the correct size. The correct size is XYZ, where 
 * X, Y and Z are the size of the actual field data, found by get_variable_size.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_simple_variable( int a_whichBlock,
			  int a_whichVar,
			  int a_whichTimestep,
			  MY_PRECISION *a_data )
{
  hbool_t l_isCoordField;
  hid_t l_varType;
  SAF_Cat l_cat;
  SAF_Field l_field;
  SAF_FieldTmpl l_fieldTemplate;
  SAF_Set l_set;
  SAF_CellType l_cellType;
  SAF_IndexSpec l_indexSpec;
  int l_numEntriesInColl=0;

  /*do we need all of these checks?*/
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets || a_whichVar<0 || 
      a_whichVar>=g_strGlobals->m_numVarsPerBlock[a_whichBlock] || a_whichTimestep<0 || 
      a_whichTimestep>=g_strGlobals->m_numTimesteps )
    {
      printinfo("read_simple_variable something out of range: a_whichBlock=%d, a_whichVar=%d, a_whichTimestep=%d\n",
	     a_whichBlock,a_whichVar,a_whichTimestep );
      return(-1);
    }
  if( !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock] ||
      !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar] )
    {
      printinfo("read_simple_variable m_instancePerTimestepPerVarPerBlock[%d][%d] not allocated\n",a_whichBlock,a_whichVar);
      return(-1);
    }

  l_field = g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar][a_whichTimestep];
  if( !my_saf_is_valid_field_handle(&l_field) ) 
    {
      printinfo("read_simple_variable trying to map a SS_FIELD_NULL\n");
      return(-1);
    }

  saf_describe_field(SAF_ALL, &(l_field), &l_fieldTemplate, NULL, &l_set, NULL, 
		     &l_isCoordField, NULL, &l_cat, NULL, NULL, 
		     NULL, &l_varType, NULL, NULL, NULL, NULL);

  /*verify that the field is mapped to points: havent written edges,faces,or elems yet*/
  saf_describe_collection(SAF_ALL,&l_set,&l_cat,&l_cellType,&l_numEntriesInColl,&l_indexSpec,NULL,NULL);
  
  if(l_numEntriesInColl<=0)
    {
      printinfo("read_simple_variable, l_numEntriesInColl=%d?\n",l_numEntriesInColl);
      return(0);
    }

  /*note: we dont care what the SAF_IndexSpec.origin's are: would not affect the result*/

  if( read_whole_field_to_my_precision(&l_field,l_varType,(size_t)l_numEntriesInColl,a_data) )
    {
      printinfo("read_simple_variable error failed to read field?\n");
      return(-1);
    }

  /*Change the indexing mode to fortran order. Note: this doesnt cost
    anything if the ordering was already fortran*/
  {
    SAF_IndexSpec l_indexSpecCorrected = l_indexSpec;
    l_indexSpecCorrected.order[0]=2;
    l_indexSpecCorrected.order[1]=1;
    l_indexSpecCorrected.order[2]=0;
    change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, sizeof(MY_PRECISION), (void *)a_data );
  }

  return(0);
}/*read_simple_variable*/


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read a Field  
 * Description:
 *
 * Reads a given field. a_data must already be allocated by the calling function.
 * a_varType contains the type of the data in the a_field, a_numEntries
 * is the number of entries in the field. The read data is cast into type MY_PRECISION.
 *
 * Return:      0 on success, -1 otherwise
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_whole_field_to_my_precision( SAF_Field *a_field, hid_t a_varType, 
				  size_t a_numEntries, MY_PRECISION *a_data )
{
  size_t k;
  /*printinfo("read_whole_field_to_my_precision: a_numEntries=%d, a_data=%p\n",a_numEntries,a_data);*/

  if(!a_data || a_numEntries<=0 )
    {
      printinfo("error in read_whole_field_to_my_precision: a_data=%s a_numEntries=%d\n",
	a_data?"non-zero":"zero", a_numEntries);
      return(-1);
    }
  if(H5Tequal(a_varType,MY_SAF_PRECISION))
    {
      saf_read_field(SAF_ALL,a_field,NULL,SAF_WHOLE_FIELD,(void **)&a_data);
    }
  else if(H5Tequal(a_varType,SAF_FLOAT))
    {
      /*there is a better way to do this......fix it later*/
      float *l_buffer=(float *)malloc(a_numEntries*sizeof(float));
      if(!l_buffer)
	{
	  printinfo("error read_whole_field_to_my_precision failed to alloc float *l_buffer\n");
	  return(-1);
	}
      saf_read_field(SAF_ALL,a_field,NULL,SAF_WHOLE_FIELD,(void **)&l_buffer);
      for(k=0;k<a_numEntries;k++)
	{
	  a_data[k]= (MY_PRECISION)l_buffer[k];
	}
      free(l_buffer);
    }
  else if(H5Tequal(a_varType,SAF_DOUBLE))
    {
      /*there is a better way to do this......fix it later*/
      double *l_buffer=(double *)malloc(a_numEntries*sizeof(double));
      if(!l_buffer)
	{
	  printinfo("error read_whole_field_to_my_precision failed to alloc double *l_buffer\n");
	  return(-1);
	}
      saf_read_field(SAF_ALL,a_field,NULL,SAF_WHOLE_FIELD,(void **)&l_buffer);

      for(k=0;k<a_numEntries;k++)
	{
	  a_data[k]= (MY_PRECISION)l_buffer[k];
	}

      free(l_buffer);
    }
  else if(H5Tequal(a_varType,SAF_INT))
    {
      /*there is a better way to do this......fix it later*/
      int *l_buffer=(int *)malloc(a_numEntries*sizeof(int));
      if(!l_buffer)
	{
	  printinfo("error read_whole_field_to_my_precision failed to alloc int *l_buffer\n");
	  return(-1);
	}
      saf_read_field(SAF_ALL,a_field,NULL,SAF_WHOLE_FIELD,(void **)&l_buffer);
      for(k=0;k<a_numEntries;k++)
	{
	  a_data[k]= (MY_PRECISION)l_buffer[k];
	}
      free(l_buffer);
    }
  else if(H5Tequal(a_varType,SAF_LONG))
    {
      /*there is a better way to do this......fix it later*/
      long *l_buffer=(long *)malloc(a_numEntries*sizeof(long));
      if(!l_buffer)
	{
	  printinfo("error read_whole_field_to_my_precision failed to alloc long *l_buffer\n");
	  return(-1);
	}
      saf_read_field(SAF_ALL,a_field,NULL,SAF_WHOLE_FIELD,(void **)&l_buffer);
      for(k=0;k<a_numEntries;k++)
	{
	  a_data[k]= (MY_PRECISION)l_buffer[k];
	}
      free(l_buffer);
    }
  else 
    {
      printinfo("   error: unexpected a_varType %s in read_whole_field_to_my_precision (might be comp field)\n",
	     get_dsl_type_string(a_varType) );

      return(-1);
    }
  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Is the Given Field on the Given Set?     
 * Description:
 * 
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
is_field_on_set( int a_whichField, SAF_Set *a_set )
{
  if( SAF_EQUIV(a_set,&(g_strGlobals->m_allFieldSets[a_whichField])) )
    {
      return(1);
    }
  return(0);
}

#ifdef USE_CACHED_SUBSET_REL_COUNTS
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Does a Given Field Reside Below the Given Non-Top Set?   
 * Description:
 *
 * a_whichField refers to an entry in the array m_allFields, 
 * a_whichNonTopset refers to entries in the array m_allNonTopSets.
 *
 * The two cache arrays m_numSubsetRelsPerNonTopSetPerTopset and m_numSubsetRelsPerNonTopSetPerNonTopset
 * are used.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
is_field_below_nontopset_cached( int a_whichField, int a_whichNonTopset )
{
  int k;
  for( k=0; k<g_strGlobals->m_numNonTopSets; k++ )
    {
      int l_numRels=0;
      if( k == a_whichNonTopset ) continue;
      if( g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset[a_whichNonTopset][k]<0 )
	{
	  saf_find_subset_relations(SAF_ALL, g_strGlobals->m_db, 
				&(g_strGlobals->m_allNonTopSets[a_whichNonTopset]), 
				&(g_strGlobals->m_allNonTopSets[k]), 
				SAF_ANY_CAT, 
				    SAF_ANY_CAT, SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE,
				    &l_numRels, NULL );
	  g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset[a_whichNonTopset][k] = l_numRels;
	}
      else
	{
	  l_numRels = g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset[a_whichNonTopset][k];
	}

      if( l_numRels > 1 )
	{
	  printinfo("is_field_below_topset_cached error: expect only 1 relation between set and subset, found %d\n",l_numRels);
	  exit(-1);
	}
      if( l_numRels > 0 )
	{
	  int l_found = is_field_on_set( a_whichField, &(g_strGlobals->m_allNonTopSets[k]) );
	  if( l_found ) return(1);
	  l_found = is_field_below_nontopset_cached( a_whichField, k );
	  if( l_found ) return(1);
	}
    }
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Does a Given Field Reside Below the Given Top Set?
 * Description:
 *
 * a_whichField refers to an entry in the array m_allFields,
 * a_whichTopset refers to an entry in the array m_sets.
 *
 * The two cache arrays m_numSubsetRelsPerNonTopSetPerTopset and m_numSubsetRelsPerNonTopSetPerNonTopset
 * are used.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
is_field_below_topset_cached( int a_whichField, int a_whichTopset )
{
  int j,k;

  /*get a list of all sets that are not topsets (i.e. not in the m_sets array)*/
  if( !g_strGlobals->m_numNonTopSets )
    {
      int l_which=0;
      g_strGlobals->m_numNonTopSets = g_strGlobals->m_numSubsets+g_strGlobals->m_numEdgeSubsets+g_strGlobals->m_numFaceSubsets+g_strGlobals->m_numElemSubsets;
      g_strGlobals->m_allNonTopSets = (SAF_Set *)malloc(g_strGlobals->m_numNonTopSets*sizeof(SAF_Set));

      memcpy( g_strGlobals->m_allNonTopSets+l_which, g_strGlobals->m_subsets, g_strGlobals->m_numSubsets*sizeof(SAF_Set) );
      l_which += g_strGlobals->m_numSubsets;

      memcpy( g_strGlobals->m_allNonTopSets+l_which, g_strGlobals->m_edgeSubsets, g_strGlobals->m_numEdgeSubsets*sizeof(SAF_Set) );
      l_which += g_strGlobals->m_numEdgeSubsets;

      memcpy( g_strGlobals->m_allNonTopSets+l_which, g_strGlobals->m_faceSubsets, g_strGlobals->m_numFaceSubsets*sizeof(SAF_Set) );
      l_which += g_strGlobals->m_numFaceSubsets;

      memcpy( g_strGlobals->m_allNonTopSets+l_which, g_strGlobals->m_elemSubsets, g_strGlobals->m_numElemSubsets*sizeof(SAF_Set) );
      l_which += g_strGlobals->m_numElemSubsets;

      printinfo("(is_field_below_topset_cached setting m_numNonTopSets=%d)\n",g_strGlobals->m_numNonTopSets);
    }

  /*allocate the cache*/
  if(!g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset)
    {
      g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset = (int **)malloc(g_strGlobals->m_numSets*sizeof(int *));
      for(k=0;k<g_strGlobals->m_numSets;k++)
	{
	  g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset[k] = (int *)malloc(g_strGlobals->m_numNonTopSets*sizeof(int));
	  for(j=0;j<g_strGlobals->m_numNonTopSets;j++)
	    {
	      g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset[k][j]=-1;
	    }
	}
      g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset = (int **)malloc(g_strGlobals->m_numNonTopSets*sizeof(int *));
      for(k=0;k<g_strGlobals->m_numNonTopSets;k++)
	{
	  g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset[k] = (int *)malloc(g_strGlobals->m_numNonTopSets*sizeof(int));
	  for(j=0;j<g_strGlobals->m_numNonTopSets;j++)
	    {
	      g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset[k][j]=-1;
	    }
	}
    }	  

  for( k=0; k<g_strGlobals->m_numNonTopSets; k++ )
    {
      int l_numRels=0;
      if( g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset[a_whichTopset][k]<0 )
	{
	  saf_find_subset_relations(SAF_ALL, g_strGlobals->m_db, 
				&(g_strGlobals->m_sets[a_whichTopset]), 
				&(g_strGlobals->m_allNonTopSets[k]), SAF_ANY_CAT, 
				    SAF_ANY_CAT, SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE,
				    &l_numRels, NULL );
	  g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset[a_whichTopset][k] = l_numRels;
	}
      else
	{
	  l_numRels = g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset[a_whichTopset][k];
	}

      if( l_numRels > 1 )
	{
	  printinfo("is_field_below_topset_cached error: expect only 1 relation between set and subset, found %d\n",l_numRels);
	  exit(-1);
	}
      if( l_numRels > 0 )
	{
	  int l_found = is_field_on_set( a_whichField, &(g_strGlobals->m_allNonTopSets[k]) );
	  if( l_found ) return(1);

	  l_found = is_field_below_nontopset_cached( a_whichField, k );
	  if( l_found ) return(1);
	}
    }
  return(0);
}

#endif /*USE_CACHED_SUBSET_REL_COUNTS*/

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Does a Given Field Reside Below the Given Set?   
 * Description:
 *
 * a_whichField refers to an entry in the array m_allFields.
 * This version uses no cache.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
is_field_below_set( int a_whichField, SAF_Set *a_set )
{
  int k;
  if( !g_strGlobals->m_numNonTopSets )
    {
      saf_find_matching_sets(SAF_ALL, g_strGlobals->m_db, SAF_ANY_NAME, SAF_SPACE, SAF_ANY_TOPODIM,
			     SAF_EXTENDIBLE_TORF, SAF_TOP_FALSE, &g_strGlobals->m_numNonTopSets, &g_strGlobals->m_allNonTopSets);

      printinfo("is_field_below_set setting m_numNonTopSets=%d\n",g_strGlobals->m_numNonTopSets);
    }
  for( k=0; k<g_strGlobals->m_numNonTopSets; k++ )
    {
      int l_numRels=0;
      saf_find_subset_relations(SAF_ALL, g_strGlobals->m_db, a_set, 
				&(g_strGlobals->m_allNonTopSets[k]), SAF_ANY_CAT, 
				SAF_ANY_CAT, SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE,
				&l_numRels, NULL );
      if( l_numRels > 1 )
	{
	  printinfo("is_field_below_set error: expect only 1 relation between set and subset, found %d\n",l_numRels);
	}
      if( l_numRels > 0 )
	{
	  int l_found = is_field_on_set( a_whichField, &(g_strGlobals->m_allNonTopSets[k]) );
	  if( l_found ) return(1);
	  l_found = is_field_below_set( a_whichField, &(g_strGlobals->m_allNonTopSets[k]) );
	  if( l_found ) return(1);
	}
    }
  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Which Top-Set is this Field From?
 * Description:
 *
 * a_whichField refers to an entry in the array m_allFields.
 *
 * Return: -1 on failure, otherwise an index from 0 to m_numSets-1 that refers to
 * an entry in the m_allSets array.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
which_topset_is_field_from( int a_whichField )
{
  int j;

  if( !my_saf_is_valid_field_handle( &(g_strGlobals->m_allFields[a_whichField])) ) return(-1);

  for( j=0; j<g_strGlobals->m_numSets; j++ )
  {
    if( is_field_on_set(a_whichField, &(g_strGlobals->m_sets[j])) ) return(j);
  }
  for( j=0; j<g_strGlobals->m_numSets; j++ )
  {
#ifdef USE_CACHED_SUBSET_REL_COUNTS
    if( is_field_below_topset_cached(a_whichField,j) ) return(j);
#else
    if( is_field_below_set(a_whichField,&(g_strGlobals->m_sets[j])) ) return(j);
#endif
  }
  return(-1);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Add Fields Not Found in Suites to Global List     
 * Description:
 *
 * This was written as an afterthought, when I found that in larry1w not
 * all variables get a place in the suites. If it is used, it should be
 * called immediately after read_all_suites(). It may or may not be a good
 * idea to call it, depending on what the data is. Hopefully this will
 * become clearer as saf develops and gets more users.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
add_fields_not_in_suites_to_list( )
{
  int i,j,l_numAllFields=0, *l_newTimestepsPerField=0, *l_newTopsetPerField=0;
  int l_numAdded=0,l_maxSize=0;
  SAF_Field *l_allFields=0,*l_newFields=0;
  SAF_FieldTmpl *l_newFieldTemplates=0;
  SAF_Set *l_newFieldSets=0;
  int l_totalNumFieldsBefore = g_strGlobals->m_totalNumFields;

  printinfo("\nentering add_fields_not_in_suites_to_list\n");

  saf_find_fields(SAF_ALL, g_strGlobals->m_db, SAF_UNIVERSE(g_strGlobals->m_db), SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
		  SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numAllFields, &l_allFields);

  printinfo(" add_fields_not_in_suites_to_list: l_numAllFields=%d, g_strGlobals->m_totalNumFields=%d    diff=%d\n",
	 l_numAllFields, g_strGlobals->m_totalNumFields, l_numAllFields-g_strGlobals->m_totalNumFields);

  if( !l_numAllFields )
    {
      return(0);
    }

  /*note: one would think that the fields found by saf_find_fields above would INCLUDE
    all the fields found in the suites, but it is not so. Dont know if this is a saf
    bug or not, but just to be safe with saf, allocate for the worst case.*/
  l_maxSize = l_numAllFields + g_strGlobals->m_totalNumFields;

  l_newTimestepsPerField = (int *)malloc(l_maxSize*sizeof(int));
  l_newTopsetPerField = (int *)malloc(l_maxSize*sizeof(int));
  l_newFields = (SAF_Field *)malloc(l_maxSize*sizeof(SAF_Field));
  l_newFieldTemplates = (SAF_FieldTmpl *)malloc(l_maxSize*sizeof(SAF_FieldTmpl));
  l_newFieldSets = (SAF_Set *)malloc(l_maxSize*sizeof(SAF_Set));

  memcpy(l_newTimestepsPerField,g_strGlobals->m_timestepsPerField,g_strGlobals->m_totalNumFields*sizeof(int));
  memcpy(l_newTopsetPerField,g_strGlobals->m_whichTopsetPerField,g_strGlobals->m_totalNumFields*sizeof(int));
  memcpy(l_newFields,g_strGlobals->m_allFields,g_strGlobals->m_totalNumFields*sizeof(SAF_Field));
  memcpy(l_newFieldTemplates,g_strGlobals->m_allFieldTemplates,g_strGlobals->m_totalNumFields*sizeof(SAF_FieldTmpl));
  memcpy(l_newFieldSets,g_strGlobals->m_allFieldSets,g_strGlobals->m_totalNumFields*sizeof(SAF_Set));

  for( i=0; i<l_numAllFields; i++ )
    {
      int l_found=0;
      char *l_name=0;
      SAF_FieldTmpl l_tmpl;
      SAF_Set l_set;
      SAF_SilRole l_role;

      for( j=0; j<g_strGlobals->m_totalNumFields; j++ )
	{
	  if(SAF_EQUIV(&(l_allFields[i]),&(l_newFields[j])))
	    {
	      l_found=1;	
	      break;
	    }
	}
      if(l_found) continue;

      if( !my_saf_is_valid_field_handle( &(l_allFields[i])) ) continue;

      /*this field is not in the list, check if it is part of a suite*/

      saf_describe_field(SAF_ALL, &(l_allFields[i]), &l_tmpl, &l_name, &l_set, NULL, NULL, 
			 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

      if( !my_saf_is_valid_set_handle(&l_set) ) continue;

      saf_describe_set(SAF_ALL,&(l_set),NULL,NULL,&l_role,NULL, NULL,NULL,NULL );
      if(l_role != SAF_SPACE) continue;


      l_newTimestepsPerField[g_strGlobals->m_totalNumFields+l_numAdded] = 0;
      l_newFields[g_strGlobals->m_totalNumFields+l_numAdded] = l_allFields[i];
      l_newFieldTemplates[g_strGlobals->m_totalNumFields+l_numAdded] = l_tmpl;
      l_newFieldSets[g_strGlobals->m_totalNumFields+l_numAdded] = l_set;
      
      l_numAdded++;

      printinfo(" add_fields_not_in_suites_to_list i=%d l_numAdded=%d name=%s\n",i,l_numAdded,l_name);
      if(l_name) free(l_name);
    }

    
  if( g_strGlobals->m_totalNumFields )
    {
      free( g_strGlobals->m_allFields );
      free( g_strGlobals->m_allFieldTemplates );
      free( g_strGlobals->m_allFieldSets );
      free( g_strGlobals->m_timestepsPerField );
      free( g_strGlobals->m_whichTopsetPerField );
    }
  g_strGlobals->m_allFields = l_newFields;
  g_strGlobals->m_allFieldTemplates = l_newFieldTemplates;
  g_strGlobals->m_allFieldSets = l_newFieldSets;
  g_strGlobals->m_timestepsPerField = l_newTimestepsPerField;
  g_strGlobals->m_whichTopsetPerField = l_newTopsetPerField;
  g_strGlobals->m_totalNumFields += l_numAdded;


  for(i=l_totalNumFieldsBefore; i<g_strGlobals->m_totalNumFields; i++)
    {
      g_strGlobals->m_whichTopsetPerField[i] = which_topset_is_field_from( i );

      printinfo("......add_fields_not_in_suites_to_list field %d of %d is from topset %d\n",
			    i,g_strGlobals->m_totalNumFields,g_strGlobals->m_whichTopsetPerField[i]);	    
    }

  if(l_allFields) free(l_allFields);

  printinfo("leaving add_fields_not_in_suites_to_list\n\n");
  return(0);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read Timestep Data from all Suites
 * Description:
 *
 * Gets the timestep info from the saf file, and associates a timestep with each field. This
 * function fills many of the global arrays that are used by other functions.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_all_suites( )
{
  int i,j,k;
  int l_numSuites=0;
  SAF_Set *l_suites=0;
  int **l_numTimestepsPerGroupPerSuite=0;
  MY_PRECISION ***l_timeValuesPerGroupPerSuite=0;
  SAF_Field ****l_fieldsPerStatePerGroupPerSuite=0;
  int **l_numFieldsPerGroupPerSuite=0;
  int *l_numGroupsPerSuite=0;
  int l_whichGroup=0;


  printinfo("\n*******BEGIN read_all_suites (post saf-1.5.3 version)*****************\n");


  if(g_strGlobals->m_numTimesteps)
    {
      printinfo("error init_str_mesh_reader: m_numTimesteps!=0, a database is already open\n");
      return(-1);
    }

  saf_find_suites( SAF_ALL, g_strGlobals->m_db, SAF_ANY_NAME, &l_numSuites, &l_suites );

  printinfo("Found %d suites.\n",l_numSuites);
  
  if( l_numSuites )
    {
      l_fieldsPerStatePerGroupPerSuite = (SAF_Field ****)malloc( l_numSuites * sizeof(SAF_Field ***) );
      l_timeValuesPerGroupPerSuite = (MY_PRECISION ***)malloc( l_numSuites * sizeof(MY_PRECISION **) );
      l_numTimestepsPerGroupPerSuite = (int **)malloc( l_numSuites * sizeof(int *) );
      l_numFieldsPerGroupPerSuite = (int **)malloc( l_numSuites * sizeof(int *) );
      l_numGroupsPerSuite = (int *)malloc( l_numSuites * sizeof(int) );
    }
  else
    {
      /*printinfo("read_all_suites: l_numSuites=0, skipping mallocs\n");*/
    }
  for( i=0; i<l_numSuites; i++ ) 
    {
      l_numTimestepsPerGroupPerSuite[i]=0;
      l_timeValuesPerGroupPerSuite[i]=0;
      l_fieldsPerStatePerGroupPerSuite[i]=0;
      l_numFieldsPerGroupPerSuite[i]=0;
      l_numGroupsPerSuite[i]=0;
    }


  /*read in the time values for each suite*/
  for( i=0; i<l_numSuites; i++ )
    {
      SAF_StateTmpl l_stateTemplate;
      char *l_suiteName=0;
      hid_t l_varType;
      SAF_Set *l_param_set=0;
      SAF_StateGrp *l_state_grps=0;
      SAF_Suite l_testSuite;
      SAF_Quantity  l_quantity;

      saf_describe_suite (SAF_ALL, &(l_suites[i]), &l_suiteName, NULL, NULL, &l_param_set);
      saf_find_state_groups(SAF_ALL, &(l_suites[i]), NULL, &l_numGroupsPerSuite[i], &l_state_grps);

      if( l_numGroupsPerSuite[i] <=0 )
	{
	  printinfo("Suite %d of %d (%s) has no state groups? Continuing...\n",i,l_numSuites,l_suiteName);
	  continue;
	}

      l_fieldsPerStatePerGroupPerSuite[i] = (SAF_Field ***)malloc( l_numGroupsPerSuite[i] * sizeof(SAF_Field **) );	  
      l_numTimestepsPerGroupPerSuite[i] = (int *)malloc( l_numGroupsPerSuite[i] * sizeof(int) );
      l_timeValuesPerGroupPerSuite[i]= (MY_PRECISION **)malloc( l_numGroupsPerSuite[i] * sizeof(MY_PRECISION *) );
      l_numFieldsPerGroupPerSuite[i] = (int *)malloc( l_numGroupsPerSuite[i] * sizeof(int) );

      for( l_whichGroup=0; l_whichGroup<l_numGroupsPerSuite[i]; l_whichGroup++ )
	{
	  char *l_stateGroupName=0;
	  l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup]=0;
	  l_numTimestepsPerGroupPerSuite[i][l_whichGroup]=0;
	
	  saf_describe_state_group(SAF_ALL, &(l_state_grps[l_whichGroup]), &l_stateGroupName, &l_testSuite, &l_stateTemplate, 
				   &l_quantity,NULL, &l_varType, &l_numTimestepsPerGroupPerSuite[i][l_whichGroup] );
	  saf_describe_state_tmpl(SAF_ALL,&l_stateTemplate,NULL, &l_numFieldsPerGroupPerSuite[i][l_whichGroup], NULL );

	  printinfo("  suite %d of %d (%s), group %d of %d,  numtimesteps=%d groupname=%s vartype=%s\n",
		    i,l_numSuites,l_suiteName,l_whichGroup,l_numGroupsPerSuite[i],
		    l_numTimestepsPerGroupPerSuite[i][l_whichGroup],l_stateGroupName,
		    get_dsl_type_string(l_varType) );


	  l_timeValuesPerGroupPerSuite[i][l_whichGroup]=0;
	  l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup]=0;

	  if( l_numTimestepsPerGroupPerSuite[i][l_whichGroup] > 0 )
	    {
	      l_timeValuesPerGroupPerSuite[i][l_whichGroup]= 
		(MY_PRECISION *)malloc(l_numTimestepsPerGroupPerSuite[i][l_whichGroup]*sizeof(MY_PRECISION));
	      l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup] = 
		(SAF_Field **)malloc( l_numTimestepsPerGroupPerSuite[i][l_whichGroup] * sizeof(SAF_Field *) );	  
	      for(j=0;j<l_numTimestepsPerGroupPerSuite[i][l_whichGroup];j++)
		{
		  float l_thisTimeValueFloat=-1;
		  double l_thisTimeValueDouble=-1;
		  int l_thisTimeValueInt=-1;
		  void *l_timePtr=0;

		  if(H5Tequal(l_varType , SAF_FLOAT)) l_timePtr=&l_thisTimeValueFloat;
		  else if(H5Tequal(l_varType , SAF_DOUBLE)) l_timePtr=&l_thisTimeValueDouble;
		  else if(H5Tequal(l_varType , SAF_INT)) l_timePtr=&l_thisTimeValueInt;
		  else 
		    {
		      printinfo("read_all_suites: error getting time value, l_varType=%s\n",get_dsl_type_string(l_varType));

		      exit(-1);
		    }
		  
		  l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup][j]=0;

		  saf_read_state(SAF_ALL, &(l_state_grps[l_whichGroup]), j, NULL, NULL, l_timePtr, 
				 &l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup][j] ); 





		  if(H5Tequal(l_varType , SAF_FLOAT)) 
			l_timeValuesPerGroupPerSuite[i][l_whichGroup][j] = l_thisTimeValueFloat;
		  else if(H5Tequal(l_varType , SAF_DOUBLE)) 
			l_timeValuesPerGroupPerSuite[i][l_whichGroup][j] = l_thisTimeValueDouble;
		  else if(H5Tequal(l_varType , SAF_INT)) 
			l_timeValuesPerGroupPerSuite[i][l_whichGroup][j] = l_thisTimeValueInt;





		  /*printinfo("    j=%d of %d, l_timeValuesPerGroupPerSuite[i][l_whichGroup][j]=%e  l_varType=%s\n",
		    j,l_numTimestepsPerGroupPerSuite[i][l_whichGroup],
		    l_timeValuesPerGroupPerSuite[i][l_whichGroup][j], get_dsl_type_string(l_varType) );*/
		}
	    }
	  else
	    {
	      /*printinfo("read_all_suites: numtimesteps=%d, skipping mallocs\n",
		l_numTimestepsPerGroupPerSuite[i][l_whichGroup]);*/
	    }

	  if(l_stateGroupName) free(l_stateGroupName);
	}
      if(l_suiteName) free(l_suiteName);
    } /*for(i=0;i<l_numSuites....*/

  printinfo("Finished reading suite data (post saf-1.5.3 version)\n");

  /* create a single sorted array of time values, store it in a_timeValues, and
     store the length of the array in a_numTimesteps*/
  {
    MY_PRECISION *l_ptr;
    MY_PRECISION *l_tempTimeValues;
    int l_maxPossibleTimesteps=0;
    g_strGlobals->m_numTimesteps = 0;

    /*count how many distinct timesteps there would be if all time values were different*/
    for( i=0; i<l_numSuites; i++ ) 
      {
	for( l_whichGroup=0; l_whichGroup<l_numGroupsPerSuite[i]; l_whichGroup++ )
	  {
	    l_maxPossibleTimesteps += l_numTimestepsPerGroupPerSuite[i][l_whichGroup];
	  }
      }
    printinfo("There are possibly as many as %d distinct timesteps.\n",l_maxPossibleTimesteps);

    if( l_maxPossibleTimesteps )
      {
	int l_didSomething=0;

	/*make a temporary list of all the time values*/
	l_tempTimeValues = (MY_PRECISION *)malloc( l_maxPossibleTimesteps * sizeof(MY_PRECISION) );
	l_ptr = l_tempTimeValues;
	for( i=0; i<l_numSuites; i++ ) 
	  {
	    for( l_whichGroup=0; l_whichGroup<l_numGroupsPerSuite[i]; l_whichGroup++ )
	      {
		for(j=0;j<l_numTimestepsPerGroupPerSuite[i][l_whichGroup];j++)
		  {
		    l_ptr[0] = l_timeValuesPerGroupPerSuite[i][l_whichGroup][j];
		    l_ptr++;
		  }
	      }
	  }

	/*go thru the list of time values, remove duplicates, and bubble sort*/
	do {
	  int l_duplicate=-1;
	  l_didSomething=0;	  
	  for( i=0; i<l_maxPossibleTimesteps-1; i++ ) 
	    {
	      if( l_tempTimeValues[i] == l_tempTimeValues[i+1] )
		{
		  l_duplicate = i+1;
		  l_didSomething = 1;
		  break;
		}
	      else if( l_tempTimeValues[i] > l_tempTimeValues[i+1] )
		{
		  MY_PRECISION l_temp = l_tempTimeValues[i+1];
		  l_tempTimeValues[i+1] = l_tempTimeValues[i];
		  l_tempTimeValues[i] = l_temp;
		  l_didSomething = 1;
		}
	    }
	  if( l_duplicate >= 0 )
	    { /*shuffle down the remaining entries in the list*/
	      for( i=l_duplicate; i<l_maxPossibleTimesteps; i++ ) 
		{
		  l_tempTimeValues[i] = l_tempTimeValues[i+1];
		}
	      l_maxPossibleTimesteps--;
	    }
	} while( l_didSomething );

	/*copy the resulting list to a_numTimesteps and a_timeValues*/
	g_strGlobals->m_numTimesteps=l_maxPossibleTimesteps;
	g_strGlobals->m_timeValues=(MY_PRECISION *)malloc( g_strGlobals->m_numTimesteps * sizeof(MY_PRECISION) );
	memcpy(g_strGlobals->m_timeValues,l_tempTimeValues,g_strGlobals->m_numTimesteps*sizeof(MY_PRECISION));

	free(l_tempTimeValues);
      }
  }/* END OF create a single sorted array of time values, store it in a_timeValues, and
      store the length of the array in a_numTimesteps*/


  printinfo("Finished sorting time values into one list: there are %d distinct timesteps.\n",g_strGlobals->m_numTimesteps);
  for(i=0;i<g_strGlobals->m_numTimesteps;i++)
    {
      printinfo(" %d=%e",i,g_strGlobals->m_timeValues[i]);
      if( !((i+1)%5) || i==g_strGlobals->m_numTimesteps-1 ) printinfo("\n");
    }


  /*create a single array of field pointers and a single 
    array of integer indices showing which timestep each belongs to. Store the results
    in a_numFields, a_fields and a_timestepsPerField*/
  {
    SAF_Field *l_ptr;
    int *l_intptr;
    g_strGlobals->m_totalNumFields = 0;

    /*count the total number of fields stored in the suites*/
    for( i=0; i<l_numSuites; i++ ) 
      {
	for( l_whichGroup=0; l_whichGroup<l_numGroupsPerSuite[i]; l_whichGroup++ )
	  {
	    g_strGlobals->m_totalNumFields += l_numFieldsPerGroupPerSuite[i][l_whichGroup]*l_numTimestepsPerGroupPerSuite[i][l_whichGroup];
	  }
      }
    printinfo("There are %d total fields in the suites.\n",g_strGlobals->m_totalNumFields);

    if( g_strGlobals->m_totalNumFields )
      {
	g_strGlobals->m_allFields=(SAF_Field *)malloc( g_strGlobals->m_totalNumFields * sizeof(SAF_Field) );
	g_strGlobals->m_timestepsPerField = (int *)malloc( g_strGlobals->m_totalNumFields * sizeof(int) );
	g_strGlobals->m_allFieldTemplates=(SAF_FieldTmpl *)malloc( g_strGlobals->m_totalNumFields * sizeof(SAF_FieldTmpl) );
	g_strGlobals->m_allFieldSets=(SAF_Set *)malloc( g_strGlobals->m_totalNumFields * sizeof(SAF_Set) );
	g_strGlobals->m_whichTopsetPerField =  (int *)malloc( g_strGlobals->m_totalNumFields * sizeof(int) );
      }
    
    for(i=0;i<g_strGlobals->m_totalNumFields;i++)
      {
	g_strGlobals->m_allFields[i]= SS_FIELD_NULL;
	g_strGlobals->m_allFieldTemplates[i]= SS_FIELDTMPL_NULL;
	g_strGlobals->m_allFieldSets[i]= SS_SET_NULL;
	g_strGlobals->m_timestepsPerField[i]=-1;
	g_strGlobals->m_whichTopsetPerField[i]=-1;
      }


    l_intptr = g_strGlobals->m_timestepsPerField;
    l_ptr = g_strGlobals->m_allFields;
    for( i=0; i<l_numSuites; i++ )
      {
	for( l_whichGroup=0; l_whichGroup<l_numGroupsPerSuite[i]; l_whichGroup++ )
	  {
	    for( j=0; j<l_numTimestepsPerGroupPerSuite[i][l_whichGroup]; j++ )
	      {
		int jj;
		for( jj=0; jj<l_numFieldsPerGroupPerSuite[i][l_whichGroup]; jj++ )
		  {
		    memcpy( l_ptr, &l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup][j][jj], sizeof(SAF_Field) );


                    if( /*l_ptr[0].theRow>=0 &&   sslib?*/
			my_saf_is_valid_field_handle(l_ptr) )
		      {
			l_ptr ++;
		  
			for( k=0; k<g_strGlobals->m_numTimesteps; k++ )
			  {
			    if( l_timeValuesPerGroupPerSuite[i][l_whichGroup][j] == g_strGlobals->m_timeValues[k] )
			      {
				l_intptr[0] = k;
				break;
			      }
			  }
			if( k==g_strGlobals->m_numTimesteps )
			  {
			    printinfo("read_all_suites error: found no match between l_timeValuesPerGroupPerSuite and a_timeValues\n");
			    exit(-1);
			  }
		      
			l_intptr++;
		      }
		    else
		      {
			/*something was wrong with this field!*/
			printinfo("warning: read_all_suites: not valid field handle?\n");
			l_ptr[0] = SS_FIELD_NULL;
		        l_ptr++;
			l_intptr[0] = -1;
		        l_intptr++;
		      }

		  }
	      }
	  }
      }  
  }/*END OF create a single array of field pointers  and a single 
     array of integer indices showing which timestep each belongs to. Store the results
     in a_numFields, a_fields, and a_timestepsPerField*/
  
  

  
#ifdef SORT_FIELDS_BY_ROW
  {
    int l_didSomething;

    /*go thru the list of time values, remove duplicates, and bubble sort*/
    do {
      l_didSomething=0;	  
      for( i=0; i<g_strGlobals->m_totalNumFields-1; i++ ) 
	{
	  if( g_strGlobals->m_allFields[i].theRow > g_strGlobals->m_allFields[i+1].theRow )
	    {
	      int l_tempint = g_strGlobals->m_timestepsPerField[i+1];
	      SAF_Field l_tempfld = g_strGlobals->m_allFields[i+1];
	      g_strGlobals->m_timestepsPerField[i+1] = g_strGlobals->m_timestepsPerField[i];
	      g_strGlobals->m_timestepsPerField[i] = l_tempint;
	      g_strGlobals->m_allFields[i+1] = g_strGlobals->m_allFields[i];
	      g_strGlobals->m_allFields[i] = l_tempfld;
	      l_didSomething = 1;
	    }
	}
    } while( l_didSomething );
  }
#endif

  /*note: this is the post-1.5.3 version of read_all_suites*/

  /*get the template for each field (put in a_fieldTemplates), and get which topset the
    field is associated with*/
  for(i=0;i<g_strGlobals->m_totalNumFields;i++)
    {
      if( !my_saf_is_valid_field_handle( &(g_strGlobals->m_allFields[i])) )
	{
	  printinfo("field %d of %d is not a valid field handle\n",i,g_strGlobals->m_totalNumFields);
	}
      else
	{
	  saf_describe_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), &g_strGlobals->m_allFieldTemplates[i], NULL, &g_strGlobals->m_allFieldSets[i], NULL, NULL,
			     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

#if 0
	  {/*verbose*/
	    char *l_setname=0,*l_fieldname=0;
	    saf_describe_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), NULL, &l_fieldname,NULL, NULL, NULL,
			       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	    saf_describe_set(SAF_ALL, &(g_strGlobals->m_allFieldSets[i]), &l_setname, NULL, NULL, NULL, NULL, NULL, NULL );
	    printinfo("\t\t XXX %d: field %s is on set %s\n",i,l_fieldname,l_setname);

	    if(l_setname) free(l_setname);
	    if(l_fieldname) free(l_fieldname);
	  }
#endif
	}
    }


  /* Find the topset for each field. */
  if( g_strGlobals->m_numSets )
    {
      for(i=0;i<g_strGlobals->m_totalNumFields;i++)
	{
	  g_strGlobals->m_whichTopsetPerField[i] = which_topset_is_field_from( i );	   

	  /*print some debug info
	    if( my_saf_is_valid_field_handle( &(g_strGlobals->m_allFields[i])) )
	    {
	    char *l_setname=0,*l_fieldname=0;
	    saf_describe_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), NULL, &l_fieldname,NULL, NULL, NULL,
	    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	    if(g_strGlobals->m_whichTopsetPerField[i]>=0) 
	    {
	    saf_describe_set(SAF_ALL, &(g_strGlobals->m_sets[g_strGlobals->m_whichTopsetPerField[i]]), &l_setname, NULL, NULL, NULL, NULL, NULL, NULL );
	    printinfo("   field %s (%d of %d) is from topset %s (%d)\n",l_fieldname,i,g_strGlobals->m_totalNumFields,
	    l_setname,g_strGlobals->m_whichTopsetPerField[i]);
	    if(l_setname) free(l_setname);
	    }
	    else
	    {
	    printinfo("   field %s (%d of %d) is from topset (%d)\n",l_fieldname,i,g_strGlobals->m_totalNumFields,
	    g_strGlobals->m_whichTopsetPerField[i]);
	    }
	    if(l_fieldname) free(l_fieldname);
	    }
	  */
	}
    }


  /*free local memory*/
  if( l_fieldsPerStatePerGroupPerSuite ) 
    {
      for(i=0;i<l_numSuites;i++)
	{
	  for( l_whichGroup=0; l_whichGroup<l_numGroupsPerSuite[i]; l_whichGroup++ )
	    {
	      if( l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup] ) 
		{
		  for(j=0;j<l_numTimestepsPerGroupPerSuite[i][l_whichGroup];j++)
		    {
		      if(l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup][j])
			free(l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup][j]);
		    }
		}
	      free(l_fieldsPerStatePerGroupPerSuite[i][l_whichGroup]);
	    }
	  if(l_fieldsPerStatePerGroupPerSuite[i]) free(l_fieldsPerStatePerGroupPerSuite[i]);
	}
      free(l_fieldsPerStatePerGroupPerSuite);
    }
  if( l_timeValuesPerGroupPerSuite ) 
    {
      for(i=0;i<l_numSuites;i++)
	{
	  if(l_timeValuesPerGroupPerSuite[i] && l_numGroupsPerSuite)
	    {
	      for( l_whichGroup=0; l_whichGroup<l_numGroupsPerSuite[i]; l_whichGroup++ )
		{
		  if( l_timeValuesPerGroupPerSuite[i][l_whichGroup] ) 
		    free(l_timeValuesPerGroupPerSuite[i][l_whichGroup]);
		}
	      free(l_timeValuesPerGroupPerSuite[i]);
	    }
	}
      free(l_timeValuesPerGroupPerSuite);
    }
  if( l_numTimestepsPerGroupPerSuite ) 
    {
      for(i=0;i<l_numSuites;i++)
	{
	  if(l_numTimestepsPerGroupPerSuite[i]) free(l_numTimestepsPerGroupPerSuite[i]);
	}
      free(l_numTimestepsPerGroupPerSuite);
    }
  if( l_numFieldsPerGroupPerSuite ) 
    {
      for(i=0;i<l_numSuites;i++)
	{
	  if(l_numFieldsPerGroupPerSuite[i]) free(l_numFieldsPerGroupPerSuite[i]);
	}
      free(l_numFieldsPerGroupPerSuite);
    }
  if( l_numGroupsPerSuite ) free(l_numGroupsPerSuite);


  if( !g_strGlobals->m_numTimesteps )
    {
      /*There were no timesteps. So that things will will not crash in the future,
	create a fake timestep.*/
      printinfo("\nwarning: THERE WERE NO TIMESTEPS. CREATING ONE FAKE TIMESTEP AT TIME 0.\n\n");
      g_strGlobals->m_numTimesteps=1;
      g_strGlobals->m_timeValues=(MY_PRECISION *)malloc(g_strGlobals->m_numTimesteps*sizeof(MY_PRECISION));
      g_strGlobals->m_timeValues[0]=0;
    }


  printinfo("*******END   read_all_suites (post saf-1.5.3 version)*****************\n\n");

  if(l_suites) free(l_suites);

  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get Timesteps for Indirect Fields
 * Description:
 *
 * This is called after read_all_suites.
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
get_timesteps_for_all_indirect_fields(  )
{
  int i,l_numIndirects=0, l_pos;

  /*get the max possible number of indirect fields with timesteps*/
  for( i=0; i<g_strGlobals->m_totalNumFields; i++ )
    {
      if( my_saf_is_valid_field_handle( &(g_strGlobals->m_allFields[i])) )
	{
	  hid_t l_dsltype;
	  size_t l_count=0;
	  saf_get_count_and_type_for_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), NULL, &l_count, &l_dsltype);
	  if( l_dsltype == SAF_HANDLE )
	    {
	      l_numIndirects += l_count;
	    }
	}
    }

  if(!l_numIndirects) return;

  /*allocate the global vars*/
  g_strGlobals->m_allIndirectFields = (SAF_Field *)malloc(l_numIndirects*sizeof(SAF_Field));
  g_strGlobals->m_timestepsPerIndirectField = (int *)malloc(l_numIndirects*sizeof(int));
  for( i=0; i<l_numIndirects; i++ ) 
    {
      g_strGlobals->m_allIndirectFields[i] = SS_FIELD_NULL;
      g_strGlobals->m_timestepsPerIndirectField[i] = -1;
    }
  g_strGlobals->m_totalNumIndirectFields=l_numIndirects;



  /*go thru the list again, and for each indirect field, give it a timestep*/
  l_pos=0;
  for( i=0; i<g_strGlobals->m_totalNumFields; i++ )
    {
      int l_thisTime = g_strGlobals->m_timestepsPerField[i];
      if( l_thisTime >= 0 )
	{
	  if( my_saf_is_valid_field_handle( &(g_strGlobals->m_allFields[i])) )
	    {
	      hid_t l_dsltype;
	      size_t l_count=0;
	      saf_get_count_and_type_for_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), NULL, &l_count, &l_dsltype);
	      if( l_dsltype == SAF_HANDLE )
		{
		  size_t j;
		  void *l_handles=0;
		  saf_read_field(SAF_ALL, &(g_strGlobals->m_allFields[i]), NULL,SAF_WHOLE_FIELD, &l_handles);
	    
		  for( j = 0 ; j < l_count; j++ ) 
		    {
		      SAF_Field l_thisField = ((SAF_Field *)l_handles)[j];
		      g_strGlobals->m_allIndirectFields[l_pos] = l_thisField;
		      g_strGlobals->m_timestepsPerIndirectField[l_pos] = l_thisTime;
		      l_pos++;
		    }
		}
	    }
	}
    }
  printinfo("get_timesteps_for_all_indirect_fields found times for %d indirect fields (max was %d)\n",
			l_pos, g_strGlobals->m_totalNumIndirectFields );

  /*NOTE THERE MIGHT BE REPEATS IN THE LIST, AND THERE MIGHT BE FEWER ENTRIES
    THAN WE EXPECTED, BECAUSE SOME FIELDS MIGHT HAVE TIME = -1*/
  g_strGlobals->m_totalNumIndirectFields=l_pos;

}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Time Step for a Given Field
 * Description:
 *
 * Search the global timestep array and find the timestep index for the field.
 * If a field is on more than one timestep, this will only return the first one found.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_timestep_for_field( SAF_Field a_field )
{
  int i;

#ifdef SORT_FIELDS_BY_ROW
  if(g_strGlobals->m_totalNumFields)
    {
      int l_last=-1,l_nextlast=1;
      int l_index=g_strGlobals->m_totalNumFields/2;
      int l_step=l_index/2;

      while(1)
	{
	  if( g_strGlobals->m_allFields[l_index].theRow == a_field.theRow ) break;

	  l_nextlast=l_last;
	  l_last=l_index;

	  if( g_strGlobals->m_allFields[l_index].theRow > a_field.theRow )
	    l_index -= l_step;
	  else
	    l_index += l_step;

	  if(l_index==l_nextlast) break;
	  if(l_index<0 || l_index>=g_strGlobals->m_totalNumFields) break;

	  if(l_step>1) l_step = l_step/2;
	}
      if( l_index>=0 && l_index<g_strGlobals->m_totalNumFields && SAF_EQUIV( a_field, g_strGlobals->m_allFields[l_index] ) )
	{
	  return( g_strGlobals->m_timestepsPerField[l_index] );
	}
    }
#else
  /*go thru the list, and check if any field matches a_field*/
  for( i=0; i<g_strGlobals->m_totalNumFields; i++ )
    {
      if( SAF_EQUIV( &a_field, &(g_strGlobals->m_allFields[i]) ) )
	{
	  return( g_strGlobals->m_timestepsPerField[i] );
	}
    }
#endif

  /*XXX could also sort the indirect fields*/
  /*go thru the indirect list, and check if any field matches a_field*/
  for( i=0; i<g_strGlobals->m_totalNumIndirectFields; i++ )
    {
      if( SAF_EQUIV( &a_field, &(g_strGlobals->m_allIndirectFields[i]) ) )
	{
	  return( g_strGlobals->m_timestepsPerIndirectField[i] );
	}
    }
  return(-1);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Is the Given Field on the Given Time Step?
 * Description:
 *
 * Use this function rather than get_timestep_for_field, if there
 * is a chance that a single field is repeated over time, like a
 * constant.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
is_this_field_on_this_timestep( SAF_Field a_field, int a_timestep )
{
  int i=0;/*start at zero if there is no sorting*/


#ifdef SORT_FIELDS_BY_ROW
  if(g_strGlobals->m_totalNumFields)
    {
      int l_last=-1,l_nextlast=1;
      int l_index=g_strGlobals->m_totalNumFields/2;
      int l_step=l_index/2;

      while(1)
	{
	  if( g_strGlobals->m_allFields[l_index].theRow == a_field.theRow ) break;

	  l_nextlast=l_last;
	  l_last=l_index;

	  if( g_strGlobals->m_allFields[l_index].theRow > a_field.theRow )
	    l_index -= l_step;
	  else
	    l_index += l_step;

	  if(l_index==l_nextlast) break;
	  if(l_index<0 || l_index>=g_strGlobals->m_totalNumFields) break;

	  if(l_step>1) l_step = l_step/2;
	}

            
      if( l_index>=0 && l_index<g_strGlobals->m_totalNumFields )
	{
	  i=l_index;

	  /*in case there is a cluster of the same field, scoot back to the begin of the cluster*/
	  while( i>0 && g_strGlobals->m_allFields[i-1].theRow == a_field.theRow ) i--;
	}
      else
	{
	  i=g_strGlobals->m_totalNumFields;/*out of range*/
	}
    }
#endif



  /*go thru the list, and check if any field matches a_field*/
  while( i<g_strGlobals->m_totalNumFields )
    {
#ifdef SORT_FIELDS_BY_ROW
      /*we know the list is sorted by row*/
      if( a_field.theRow > g_strGlobals->m_allFields[i].theRow ) break;
#endif      

      if( SAF_EQUIV( &a_field, &(g_strGlobals->m_allFields[i]) ) )
	{
	  if( g_strGlobals->m_timestepsPerField[i] == a_timestep ) return(1);
	}
      i++;
    }

  /*XXX could also sort the indirect fields*/
  /*go thru the indirect list, and check if any field matches a_field*/
  for( i=0; i<g_strGlobals->m_totalNumIndirectFields; i++ )
    {
      if( SAF_EQUIV( &a_field, &(g_strGlobals->m_allIndirectFields[i]) ) )
	{
	  if( g_strGlobals->m_timestepsPerIndirectField[i] == a_timestep ) return(1);
	}
    }
  return(0);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Hyperslab that Defines a Subset
 * Description:
 *
 * Returns 0 if found the slab, 1 for no slab, -1 for error.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_subset_hyperslab( SAF_Set *a_superSet, SAF_Set *a_subSet, SAF_Cat *a_coll, struct_hyperslab *a_slab )
{
  int l_numRels=0;
  SAF_Rel *l_rels=0;
  saf_find_subset_relations(SAF_ALL, g_strGlobals->m_db, a_superSet, a_subSet, a_coll, a_coll, SAF_BOUNDARY_FALSE, 
			    SAF_BOUNDARY_FALSE, &l_numRels, &l_rels );

#if 1 /*print some info*/
  {
    char *l_supersetname=0, *l_subsetname=0, *l_catname=0;
    saf_describe_set(SAF_ALL, a_superSet, &l_supersetname, NULL, NULL, NULL, NULL, NULL, NULL );
    saf_describe_set(SAF_ALL, a_subSet, &l_subsetname, NULL, NULL, NULL, NULL, NULL, NULL );
    if( !SAF_EQUIV(a_coll,SAF_ANY_CAT) ) saf_describe_category(SAF_ALL,a_coll,&l_catname,NULL,NULL);
    else l_catname=ens_strdup("SAF_ANY_CAT");
    printinfo("get_subset_hyperslab: numRels=%d  superset=\"%s\" subset=\"%s\"  cat=\"%s\" \n",
			  l_numRels,l_supersetname,l_subsetname,l_catname);

    if(l_supersetname) free(l_supersetname);
    if(l_subsetname) free(l_subsetname);
    if(l_catname) free(l_catname);
  }
#endif

  if( l_numRels>1 )
    {
      printinfo("get_subset_hyperslab error: found %d subset relations, expected 0 or 1\n",l_numRels);
      if(l_rels) free(l_rels);
      return(-1);
    }
  else if( l_numRels==1 )
    {
      int *l_bbuf=NULL;
      size_t l_abuf_sz,l_bbuf_sz;
      hid_t   l_abuf_type,l_bbuf_type;
      saf_get_count_and_type_for_subset_relation(SAF_ALL,&(l_rels[0]),NULL,&l_abuf_sz,&l_abuf_type,&l_bbuf_sz,&l_bbuf_type );
      if(l_abuf_sz!=9 || !H5Tequal(l_abuf_type,SAF_INT) || l_bbuf_sz!=0)
	{
	  printinfo("error: get_subset_hyperslab subset relation is not a 3d hslab\n");
	  return(-1);
	}
      saf_read_subset_relation(SAF_ALL,&(l_rels[0]), NULL,(void **)&a_slab, (void **)&l_bbuf );
      if(l_rels) free(l_rels);

      return(0);
    }
  else
    {
      printinfo("get_subset_hyperslab: this must be a subset of a subset...not written yet\n");
      return(1);
    }
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Name of the Structured Mesh Variable Type
 * Description:
 *
 * Return: Returns one of the following character strings: "node", "xEdge", "yEdge", "zEdge", "xFace",
 * "yFace", "zFace", "elem", "unknown"
 *--------------------------------------------------------------------------------------------------- */
const char *
STR_CLASS_NAME_IF_CPP
get_var_type_name( saf_str_mesh_var_type a_which )
{
  if( a_which == SAF_STR_MESH_NODE ) return("node");
  else if( a_which == SAF_STR_MESH_XEDGE ) return("xEdge");
  else if( a_which == SAF_STR_MESH_YEDGE ) return("yEdge");
  else if( a_which == SAF_STR_MESH_ZEDGE ) return("zEdge");
  else if( a_which == SAF_STR_MESH_XFACE ) return("xFace");
  else if( a_which == SAF_STR_MESH_YFACE ) return("yFace");
  else if( a_which == SAF_STR_MESH_ZFACE ) return("zFace");
  else if( a_which == SAF_STR_MESH_ELEM ) return("elem");

  return("unknown");
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Names for all Variables and Store them in the Global List
 * Description:
 *
 * Calc the name for all variables at once, storing in global var g_strGlobals->m_strVarNames.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_all_str_var_names( unsigned int a_maxlen )
{
  int i,j,l_var;
  SAF_Field *l_list=0;

  if(!g_strGlobals->m_maxNumVars || !g_strGlobals->m_numSets || !g_strGlobals->m_numTimesteps) return(-1);
  g_strGlobals->m_strVarNames = (char **)malloc(g_strGlobals->m_maxNumVars*sizeof(char *));
  for(i=0;i<g_strGlobals->m_maxNumVars;i++) g_strGlobals->m_strVarNames[i]=0;
  l_list = (SAF_Field *)malloc(g_strGlobals->m_numSets*g_strGlobals->m_numTimesteps*sizeof(SAF_Field));

  /*make a list of all of the fields that comprise this variable, then call
    get_variable_name_for_field_list */
  for(l_var=0;l_var<g_strGlobals->m_maxNumVars;l_var++)
  {
    int l_count=0;
    for(i=0;i<g_strGlobals->m_numSets;i++)
      {
	if( l_var<g_strGlobals->m_numVarsPerBlock[i] && g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][l_var] )
	  {
	    for(j=0;j<g_strGlobals->m_numTimesteps;j++)
	      {
		if( my_saf_is_valid_field_handle( &(g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][l_var][j])) )
		  {
		    l_list[l_count++] = g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][l_var][j];
		  }
	      }
	  }
      }

#ifdef __cplusplus
    g_strGlobals->m_strVarNames[l_var] = m_variableNames->get_variable_name_for_field_list(l_count,l_list,a_maxlen);
#else
    g_strGlobals->m_strVarNames[l_var] = get_variable_name_for_field_list(l_count,l_list,a_maxlen);
#endif

    printinfo("str var %d of %d = %s  (num fields used = %d)\n",l_var,g_strGlobals->m_maxNumVars,g_strGlobals->m_strVarNames[l_var],l_count);
  }
  free(l_list);
  return(0);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get Field Name of Variable Instance
 * Description:
 *
 * Gets the name of the first field found for a given variable and timestep combination. Note that
 * there might be as many matching fields as there are structured blocks.
 *
 * If a_str is NULL and  a_maxlen then the function will just return 1 if it found a field, 0 if not.
 * Up to a_maxlen characters of the name will be written to the preallocated array a_str.
 * This function is particularly for the EMPHASIS IDL reader.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_field_name_for_var( int a_whichVar, int a_whichTimestep, char *a_str, size_t a_maxlen )
{
  int i,l_found=0;
  size_t l_currentPos=0;
  if(a_str && a_maxlen) a_str[0]='\0';

  if( a_whichVar<0 || a_whichVar>=g_strGlobals->m_maxNumVars )
    {
      printinfo("get_var_names_as_string error, a_whichVar=%d is out of range\n",a_whichVar);
      return(0);
    }  
  for(i=0;i<g_strGlobals->m_numSets;i++)
    {
      char *l_name=0,*l_ptr;
      if( a_whichVar<g_strGlobals->m_numVarsPerBlock[i] && 
	  g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][a_whichVar] &&
	  my_saf_is_valid_field_handle( &(g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][a_whichVar][a_whichTimestep])) )
	{

	  saf_describe_field(SAF_ALL, &(g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][a_whichVar][a_whichTimestep]),
			     NULL, &l_name, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			     NULL, NULL, NULL, NULL, NULL, NULL);


	  if(a_str && a_maxlen)
	    {
	      l_currentPos = strlen( a_str );
	      l_ptr = a_str + l_currentPos;
	      snprintf(l_ptr,a_maxlen-l_currentPos,"%s", l_name );
	    }
	  l_found=1;

	  if(l_name) free(l_name);
	  break;
	}
    }  

  /*add an ellipsis at the end, if we overflowed the buffer*/
  if(a_str && a_maxlen)
    {
      l_currentPos = strlen( a_str );
      if( l_currentPos >= a_maxlen-1 )
	{
	  a_str[a_maxlen-1]='\0';
	  a_str[a_maxlen-2]='.';
	  a_str[a_maxlen-3]='.';
	  a_str[a_maxlen-4]='.';
	}
    }
  return(l_found);
}







/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Variable Instances on a Set
 * Description:
 *
 * a_fields must be already allocated to correct size
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_variables_on_set( SAF_Set a_set, SAF_Field *a_fields )
{
  int l_numFields=0,i,l_count=0;
  SAF_Field *l_fields=0;
  saf_find_fields(SAF_ALL, g_strGlobals->m_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
		  SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);
  for( i = 0 ; i < l_numFields; i++ )
    {
      hbool_t l_isCoordField;
      hid_t l_varType;

      if( !my_saf_is_valid_field_handle( &(l_fields[i])) ) continue;

      saf_describe_field(SAF_ALL, &(l_fields[i]), NULL, NULL, NULL, NULL,
			 &l_isCoordField, NULL, NULL, NULL, NULL, 
			 NULL, &l_varType, NULL, NULL, NULL, NULL);

      if( !l_isCoordField )
	{
	  memcpy( &(a_fields[l_count]), &(l_fields[i]), sizeof(SAF_Field) );
	  l_count++;
	}
    }
  if(l_fields) free(l_fields);
  return(l_count);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Variable Fields on a Set
 * Description:
 *
 * Returns the number of non-coordinate fields on a set.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_variable_fields_on_set( SAF_Set a_set )
{
  int l_numFields=0,i,l_count=0;
  SAF_Field *l_fields=0;
  saf_find_fields(SAF_ALL, g_strGlobals->m_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
		  SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);
  for( i = 0 ; i < l_numFields; i++ )
    {
      hbool_t l_isCoordField;
      hid_t l_varType;

      if( !my_saf_is_valid_field_handle( &(l_fields[i])) ) continue;

      saf_describe_field(SAF_ALL, &(l_fields[i]), NULL, NULL, NULL, NULL, 
			 &l_isCoordField, NULL, NULL, NULL, NULL, 
			 NULL, &l_varType, NULL, NULL, NULL, NULL);


      if( !l_isCoordField ) l_count++;
    }
  if(l_fields) free(l_fields);
  return(l_count);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Is this Set a Valid Structured Mesh Block?
 * Description:
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
does_set_have_the_8_structured_collections( SAF_Set a_set )
{
  int i,j,l_numCollections=0;  
  SAF_Cat *l_cats=0;

  int l_found[8]={ 0,0,0,0, 0,0,0,0 };
  SAF_CellType l_cellTypes[8] = {
    SAF_CELLTYPE_POINT,
    SAF_CELLTYPE_LINE,
    SAF_CELLTYPE_LINE,
    SAF_CELLTYPE_LINE,
    SAF_CELLTYPE_QUAD,
    SAF_CELLTYPE_QUAD,
    SAF_CELLTYPE_QUAD,
    SAF_CELLTYPE_HEX
  };
  SAF_IndexSpec l_pointIndexSpec;

  int l_xIndexDifference[8] = { 0, -1,0,0, 0,-1,-1, -1 };
  int l_yIndexDifference[8] = { 0, 0,-1,0, -1,0,-1, -1 };
  int l_zIndexDifference[8] = { 0, 0,0,-1, -1,-1,0, -1 };

  if(!my_saf_is_valid_set_handle(&a_set)) return(0);


  /*First check if the set has the token SAF_STRUCTURED topo relation between
    hex elems and nodes. Note this relation is redundant, and the whole idea
    needs reworking.
  */
  {
    int l_numRels=0,l_foundRel=0;
    SAF_Rel *l_rels=0;
    SAF_RelRep l_relType;
    saf_find_topo_relations(SAF_ALL, g_strGlobals->m_db,&a_set,NULL,&l_numRels,&l_rels);
    for(i=0;i<l_numRels;i++)
      {
	saf_describe_topo_relation(SAF_ALL,&(l_rels[i]),NULL,NULL,NULL,NULL,NULL,&l_relType,NULL);
	if(SAF_EQUIV(&l_relType,SAF_STRUCTURED))
	  {
	    l_foundRel=1;
	    break;
	  }
      }
    if(!l_foundRel)
      {
	/*printinfo("This set doesnt have the required SAF_STRUCTURED topo relation (out of %d rels)\n",l_numRels);*/
	if(l_rels) free(l_rels);
	return(0);
      }
    if(l_rels) free(l_rels);
    /*printinfo("This set DOES have the required SAF_STRUCTURED topo relation (out of %d rels)\n",l_numRels);*/
  }



  saf_describe_set(SAF_ALL, &(a_set), NULL, NULL, NULL, NULL, 
		   NULL, &l_numCollections, &l_cats);
  if(l_numCollections<8) return(0);/*must have at least 8 collections*/

  /*find the point collection first (index 0 of the above arrays)*/
  for( i=0; i<l_numCollections; i++ )
    {
      SAF_CellType l_cellType;
      SAF_IndexSpec l_indexSpec;
      saf_describe_collection(SAF_ALL,&a_set,&(l_cats[i]),&l_cellType,NULL,&l_indexSpec,NULL,NULL);

      if( l_cellType==SAF_CELLTYPE_POINT && l_indexSpec.ndims==3  )
	{
	  if(!l_found[0])
	    {
	      l_found[0]=1;
	      l_pointIndexSpec=l_indexSpec;
	    }
	  else
	    {
	      printinfo("error: >1 SAF_CELLTYPE_POINT COLLECTIONS, NOT YET PREPARED FOR THIS\n");
	      exit(-1);
	    }
	}
    }
  if(!l_found[0]) return(0);/*must have a point collection*/


  for( i=0; i<l_numCollections; i++ )
    {
      SAF_CellType l_cellType;
      SAF_IndexSpec l_indexSpec;
      saf_describe_collection(SAF_ALL,&a_set,&(l_cats[i]),&l_cellType,NULL,&l_indexSpec,NULL,NULL);

      for(j=0;j<8;j++)
	{
	  if( !l_found[j] && l_cellTypes[j]==l_cellType && l_indexSpec.ndims==3 )
	    {
	      if( l_indexSpec.sizes[0] == l_pointIndexSpec.sizes[0]+l_xIndexDifference[j] &&
		  l_indexSpec.sizes[1] == l_pointIndexSpec.sizes[1]+l_yIndexDifference[j] &&
		  l_indexSpec.sizes[2] == l_pointIndexSpec.sizes[2]+l_zIndexDifference[j] )
		{
		  l_found[j]=1;
		}
	    }
	}
    }

  for(i=0;i<8;i++)
    {
      if(!l_found[i]) return(0);
    }

  return(1);
}/*does_set_have_the_8_structured_collections*/



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Find Sets Matching a Pattern
 * Description:
 *
 * set a_numColl to -1 if it doesnt matter how many collections there are
 * set a_collDim to -1 if it doesnt matter what dimension any collection is
 * set a_cellType to SAF_CELLTYPE_ANY if it doesnt matter what cell type any collection is
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_matching_sets(  SAF_Set **a_subsets, SAF_TopMode a_topMode, int a_topoDim,
		    int a_numColl, int a_collDim, SAF_CellType a_cellType,
		    int a_mustHaveAllTheStructuredMeshCollections )
{
  SAF_Set *l_sets=0;
  int l_numSets=0,l_count=0,i,j;  
  int *l_whichSetsMatch=0;   
  saf_find_matching_sets(SAF_ALL, g_strGlobals->m_db, SAF_ANY_NAME, SAF_SPACE, a_topoDim,
			 SAF_EXTENDIBLE_TORF, a_topMode, &l_numSets, &l_sets);

  if(!l_numSets)
    {
      return(0);
    }
  l_whichSetsMatch= (int *)malloc(l_numSets*sizeof(int));

  for( i = 0 ; i < l_numSets; i++ ) 
    {
      SAF_Cat *l_cats=0;
      int l_numCollections=0;
      l_whichSetsMatch[i]=0;

      if(!my_saf_is_valid_set_handle(&(l_sets[i]))) continue;

      saf_describe_set(SAF_ALL, &(l_sets[i]), NULL, NULL, NULL, NULL, 
		       NULL, &l_numCollections, &l_cats);
      if( l_numCollections==a_numColl || a_numColl<0 )
	{
	  if( l_numCollections == 0 )
	    {
	      /*looking for a set with no collections: found it*/
	      l_whichSetsMatch[i]=1;
	      l_count++;
	    }
	  else
	    {
	      for( j=0; j<l_numCollections; j++ )
		{
		  SAF_CellType l_cellType;
		  SAF_IndexSpec l_indexSpec;
		  int l_numInCollection=0;
		  saf_describe_collection(SAF_ALL,&(l_sets[i]),&(l_cats[j]),&l_cellType,&l_numInCollection,&l_indexSpec,NULL,NULL);
		  if( (a_collDim<0 || l_indexSpec.ndims==a_collDim)  && 
		      (a_cellType==SAF_CELLTYPE_ANY || a_cellType==a_cellType) )
		    {
		      if( !a_mustHaveAllTheStructuredMeshCollections ||
			  (a_mustHaveAllTheStructuredMeshCollections && 
			   does_set_have_the_8_structured_collections(l_sets[i])) )
			{
			  /*at least one of the collections had the a_collDim and a_cellType 
			    we were looking for*/
			  l_whichSetsMatch[i]=1;
			  l_count++;
			  break;
			}
		    }		  
		}
	    }
	}
    }
  if( a_subsets && l_count )
    {
      int l_which=0;
      a_subsets[0]=(SAF_Set *)malloc(l_count*sizeof(SAF_Set));
      for( i = 0 ; i < l_numSets; i++ )
	{
	  if(l_whichSetsMatch[i])
	    {
	      a_subsets[0][l_which++]=l_sets[i];
	    }
	}
    }
  free(l_whichSetsMatch);
  if(l_sets) free(l_sets);

  return(l_count);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Structured Blocks and the Corresponding Sets
 * Description:
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_block_sets( SAF_Set **a_blocks )
{
  int i,j,l_numPossible=0, l_numActual=0, *l_areTrueBlocks=0,l_which;
  SAF_Set *l_tempBlocks=0;

  l_numPossible = get_matching_sets( &l_tempBlocks, SAF_TOP_TORF, 3, -1, -1, 
				     SAF_CELLTYPE_ANY, 1/*must have the 8 str mesh coll!*/ );

  if(l_numPossible<=0) return(0);

  l_areTrueBlocks = (int *)malloc(l_numPossible*sizeof(int));
  for(i=0;i<l_numPossible;i++) l_areTrueBlocks[i]=1;

  /*go thru the list and if any of them are subsets of another one in the 
    list, then remove the subset*/
  l_numActual = l_numPossible;
  for(i=0;i<l_numPossible;i++)
    {
      if( l_areTrueBlocks[i] )
	{
	  for(j=0;j<l_numPossible;j++)
	    {
	      if( l_areTrueBlocks[j] && i!=j )
		{
		  int l_numRels=0;
		  saf_find_subset_relations(SAF_ALL,g_strGlobals->m_db, &(l_tempBlocks[i]),&(l_tempBlocks[j]),SAF_ANY_CAT,SAF_ANY_CAT,
					    SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE, &l_numRels, NULL );
		  if(l_numRels)
		    {
		      l_areTrueBlocks[j]=0;
		      l_numActual--;
		    }
		}
	    }
	}
    }
	
  /*if requested, allocate the returned argument and copy to it*/
  if(a_blocks && l_numActual)
    {
      a_blocks[0] = (SAF_Set *)malloc(l_numActual*sizeof(SAF_Set));
      l_which=0;
      for(i=0;i<l_numPossible;i++)
	{
	  if(l_areTrueBlocks[i])
	    {
	      a_blocks[0][l_which++]=l_tempBlocks[i];
	    }
	}
    }
  free(l_tempBlocks);
  free(l_areTrueBlocks);

  return(l_numActual);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Node Subsets and the Corresponding SAF_Sets
 * Description:
 *
 * NOTE THIS IS ONLY SLIGHTLY DIFFERENT FROM (THE DUAL OF) get_num_block_sets()
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_node_subsets( SAF_Set **a_subsets )
{
  int i,j,l_numPossible=0, l_numActual=0, *l_areTrueBlocks=0,l_which;
  SAF_Set *l_tempBlocks=0;

  l_numPossible = get_matching_sets( &l_tempBlocks, SAF_TOP_TORF, 3, -1, -1, 
				     SAF_CELLTYPE_ANY, 1/*must have the 8 str mesh coll!*/ );

  if(l_numPossible<=0) return(0);

  l_areTrueBlocks = (int *)malloc(l_numPossible*sizeof(int));
  for(i=0;i<l_numPossible;i++) l_areTrueBlocks[i]=0;

  /*go thru the list and keep only those that are a subset of another
    in the list*/
  l_numActual = 0;
  for(i=0;i<l_numPossible;i++)
    {
      for(j=0;j<l_numPossible;j++)
	{
	  if( i!=j )
	    {
	      int l_numRels=0;
	      saf_find_subset_relations(SAF_ALL,g_strGlobals->m_db, &(l_tempBlocks[j]),&(l_tempBlocks[i]),
					SAF_ANY_CAT,SAF_ANY_CAT,
					SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE, &l_numRels, NULL );
	      if(l_numRels)
		{
		  l_areTrueBlocks[i]=1;
		  l_numActual++;
		  break;
		}
	    }
	}
    }

  /*if requested, allocate the returned argument and copy to it*/
  if(a_subsets && l_numActual)
    {
      a_subsets[0] = (SAF_Set *)malloc(l_numActual*sizeof(SAF_Set));
      l_which=0;
      for(i=0;i<l_numPossible;i++)
	{
	  if(l_areTrueBlocks[i])
	    {
	      a_subsets[0][l_which++]=l_tempBlocks[i];
	    }
	}
    }
  free(l_tempBlocks);
  free(l_areTrueBlocks);

  return(l_numActual);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Edge Subsets and the Corresponding SAF_Sets 
 * Description:
 *
 * NOTE THIS MAY NO LONGER BE CORRECT: 1 collection is too restrictive
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_edge_subsets(  SAF_Set **a_subsets  )
{
  int l_count;
  l_count = get_matching_sets( a_subsets, SAF_TOP_FALSE, 1, 1, 3, SAF_CELLTYPE_LINE,0 );
  return(l_count);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Face Subsets and the Corresponding SAF_Sets 
 * Description:
 *
 * NOTE THIS MAY NO LONGER BE CORRECT: 1 collection is too restrictive
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_face_subsets(  SAF_Set **a_subsets  )
{
  int l_count;
  l_count = get_matching_sets( a_subsets, SAF_TOP_FALSE, 2, 1, 3, SAF_CELLTYPE_QUAD,0 );
  return(l_count);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Hex Element Subsets and the Corresponding SAF_Sets
 * Description:
 *
 * NOTE THIS MAY NO LONGER BE CORRECT: 1 collection is too restrictive
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_elem_subsets(  SAF_Set **a_subsets  )
{
  int l_count;
  l_count = get_matching_sets( a_subsets, SAF_TOP_FALSE, 3, 1, 3, SAF_CELLTYPE_HEX,0 );
  return(l_count);
}






/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read a Block's Coords as Separable
 * Description:
 *
 * Read coords on the specified block, and assume they are separable, whether they are
 * stored as separable or not. If is_block_separable returned 0, then this
 * function might return garbage coordinates, though it will not fail. If 
 * is_block_separable returned 1, then this function should be used.
 * If an argument array pointer is NULL, then it will be ignored. Otherwise is
 * must be pre-allocated to length X Y or Z, where X Y and Z are the lengths returned
 * by get_block_size().
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_block_coords_as_separable( int a_whichBlock, MY_PRECISION *a_xCoords, 
				    MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords )
{
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets )
    {
      printinfo("error read_block_coords_as_separable: a_whichBlock out of range: a_whichBlock=%d m_numSets=%d\n",
		a_whichBlock,g_strGlobals->m_numSets);
      exit(-1);
    }
  return( read_set_coords_as_separable(g_strGlobals->m_sets[a_whichBlock],a_xCoords,a_yCoords,a_zCoords) );
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read Coords from a Given Block Set as Separable
 * Description:
 *
 * read block coords, whether they are stored as curvilinear or separable,
 * as separable coords (i.e. block size AxBxC 
 * representable by 3 1-D arrays of size A B and C)
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_set_coords_as_separable( SAF_Set a_set, MY_PRECISION *a_xCoords, 
				    MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords )
{
  SAF_IndexSpec l_indexSpec;
  int l_count, l_numCollections=0, i, j, k, l_done=0;
  SAF_Cat *l_cats=0;
  SAF_CellType l_cellType;
  
  if(!my_saf_is_valid_set_handle(&a_set)) return(-1);

  if(!read_separable_3d_coords(a_set,0,0,0))
    {
      /*
      ** READ SEPARABLE COORDINATE DATA
      */
      if(read_separable_3d_coords( a_set, a_xCoords, a_yCoords, a_zCoords))
	{
	  printinfo("read_curvilinear_block_coords_as_separable : read_separable_3d_coords error\n");
	}
    }
  else
    {
      /*
      ** READ CURVILINEAR COORDINATE DATA
      */
      if(!does_block_contain_3d_curvilinear_coords(a_set)) return(-1);

      saf_describe_set(SAF_ALL,&(a_set),NULL,NULL,NULL,NULL,NULL,&l_numCollections,&l_cats);

      for( i=0; i<l_numCollections; i++ )
	{
	  saf_describe_collection(SAF_ALL,&a_set,&(l_cats[i]),&l_cellType,&l_count,&l_indexSpec,NULL,NULL);
	  if( l_indexSpec.ndims == 3 )
	    {
	      int l_x,l_y,l_z;
	      int l_numFields=0;
	      SAF_Field *l_fields=0;

	      l_x = l_indexSpec.sizes[0];
	      l_y = l_indexSpec.sizes[1];
	      l_z = l_indexSpec.sizes[2];

	      /*note: we dont care what the SAF_IndexSpec.origin's are: would not affect the result*/

	      printinfo("read_curvilinear_block_coords_as_separable found %d nodes: %dx%dx%d\n",l_count,l_x,l_y,l_z );

	      saf_find_fields(SAF_ALL, g_strGlobals->m_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
			      &(l_cats[i]), SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);

	      for( j = 0 ; j < l_numFields; j++ )
		{
		  SAF_FieldTmpl *l_fieldTemplate=0;
		  SAF_Field *l_componentFields=0;
		  hbool_t l_isCoordField;
		  int l_numComponents;
		  hid_t l_varType;

		  if(!my_saf_is_valid_field_handle( &(l_fields[j]))) continue;

		  saf_describe_field(SAF_ALL, &(l_fields[j]), l_fieldTemplate, NULL, NULL, NULL,
				     &l_isCoordField, NULL, NULL, NULL, NULL, 
				     NULL, &l_varType, &l_numComponents, &l_componentFields, NULL, NULL);


		  if( l_numComponents==3 && l_isCoordField )
		    {
		      MY_PRECISION *l_xBuf,*l_yBuf,*l_zBuf;

		      if( l_varType<0 ) /*sslib H5Tequal(l_varType,H5I_INVALID_HID) )*/
			{
			  l_varType = guess_dsl_var_type_for_composite_coord_field(l_numFields,l_fields);
			}


		      l_xBuf = (MY_PRECISION *)malloc(l_count*sizeof(MY_PRECISION));
		      l_yBuf = (MY_PRECISION *)malloc(l_count*sizeof(MY_PRECISION));
		      l_zBuf = (MY_PRECISION *)malloc(l_count*sizeof(MY_PRECISION));

		      if( l_xBuf==0 || l_yBuf==0 || l_zBuf==0 )
			{
			  printinfo("read_set_coords_as_separable error: malloc failed\n");
			  exit(-1);
			}

		      if(read_whole_field_to_my_precision(&(l_componentFields[0]),l_varType,(size_t)l_count,l_xBuf))
			{
			  printinfo("read_set_coords_as_separable error reading x field\n");
			  exit(-1);
			}
		      if(read_whole_field_to_my_precision(&(l_componentFields[1]),l_varType,(size_t)l_count,l_yBuf))
			{
			  printinfo("read_set_coords_as_separable error reading y field\n");
			  exit(-1);
			}
		      if(read_whole_field_to_my_precision(&(l_componentFields[2]),l_varType,(size_t)l_count,l_zBuf))
			{
			  printinfo("read_set_coords_as_separable error reading z field\n");
			  exit(-1);
			}



		      /* change the ordering if needed */
		      {
			SAF_IndexSpec l_indexSpecCorrected = l_indexSpec;
			l_indexSpecCorrected.order[0]=2;
			l_indexSpecCorrected.order[1]=1;
			l_indexSpecCorrected.order[2]=0;


			if( l_indexSpec.order[0]!=l_indexSpecCorrected.order[0] || 
			    l_indexSpec.order[1]!=l_indexSpecCorrected.order[1] ||
			    l_indexSpec.order[2]!=l_indexSpecCorrected.order[2] )
			  {
			    printinfo("read_set_coords_as_separable: changing ordering from %d %d %d  to %d %d %d\n",
				      l_indexSpec.order[0],
				      l_indexSpec.order[1],
				      l_indexSpec.order[2],
				      l_indexSpecCorrected.order[0],
				      l_indexSpecCorrected.order[1],
				      l_indexSpecCorrected.order[2] );
			  }

			if(a_xCoords) change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, 
							      (int)H5Tget_size(l_varType), (void *)(l_xBuf) );
			if(a_yCoords) change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, 
							      (int)H5Tget_size(l_varType), (void *)(l_yBuf) );		 
			if(a_zCoords) change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, 
							      (int)H5Tget_size(l_varType), (void *)(l_zBuf) );		      
		      }


		      /*pick out the separable coord values from the curvilinear arrays*/
		      {
			int l_step;
			if( a_xCoords )
			  {
			    l_step = 1;
			    for(k=0;k<l_x;k++)
			      {
				a_xCoords[k]= (l_xBuf[k*l_step]);
			      }
			  }
		       
			if( a_yCoords )
			  {
			    l_step = l_x;
			    for(k=0;k<l_y;k++)
			      {
				a_yCoords[k]= (l_yBuf[k*l_step]);
			      }
			  }
		       
			if( a_zCoords )
			  {
			    l_step=l_x*l_y;
			    for(k=0;k<l_z;k++)
			      {
				a_zCoords[k]= (l_zBuf[k*l_step]);
			      }
			  }
		      }





		      free(l_xBuf);
		      free(l_yBuf);
		      free(l_zBuf);

		      l_done=1;
		      break;
		    }
		}
	      if(l_fields) free(l_fields);
	    }
	  else
	    {
	      printinfo("read_curvilinear_block_coords_as_separable: error? l_indexSpec.ndims=%d\n",l_indexSpec.ndims);
	      exit(-1);
	    }
	  if(l_done) break;
	}
    } /*end of curvilinear data section*/


  /*print out the results*/
  {
    int l_x,l_y,l_z;
    get_block_size_from_set(&a_set,&l_x,&l_y,&l_z);

    if( a_xCoords )
      {
	printinfo("X coords[%d]: ",l_x);
	for(k=0;k<l_x;k++)
	  {
	    if( k>10 )
	      {
		printinfo(" etc.");
		break;
	      }
	    printinfo("%f ",a_xCoords[k]);
	  }
	printinfo("\n");
      }

    if( a_yCoords )
      {
	printinfo("Y coords[%d]: ",l_y);
	for(k=0;k<l_y;k++)
	  {
	    if( k>10 )
	      {
		printinfo(" etc.");
		break;
	      }
	    printinfo("%f ",a_yCoords[k]);
	  }
	printinfo("\n");
      }
		       
    if( a_zCoords )
      {
	printinfo("Z coords[%d]: ",l_z);
	for(k=0;k<l_z;k++)
	  {
	    if( k>10 )
	      {
		printinfo(" etc.");
		break;
	      }
	    printinfo("%f ",a_zCoords[k]);
	  }
	printinfo("\n");
      }    
  }


  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read Block Coords as Separable for a Given Variable     
 * Description:
 *
 * Created for the IDL reader.
 * read block coords assuming they are separable (i.e. block size AxBxC 
 * representable by 3 1-D arrays of size A B and C)
 * then keep only the values for which a_whichVar has data also. For example,
 * if a_whichVar represents a var that is defined on a hyperslab of the block,
 * then return a_x a_y and a_z values only for that hyperslab.
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_block_coords_for_selected_variable_as_separable( int a_whichBlock,
					int a_whichVar,
					int a_whichTimestep,
					MY_PRECISION *a_x, MY_PRECISION *a_y, MY_PRECISION *a_z )
{
  int l_strMeshVarType;
  hbool_t l_isCoordField;
  hid_t l_varType;
  SAF_Cat l_cat;
  SAF_Field l_field;
  SAF_FieldTmpl l_fieldTemplate;
  SAF_Set l_set;
  SAF_CellType l_cellType;
  SAF_IndexSpec l_indexSpec;
  int l_numEntriesInColl=0;
  int l_numEntriesInBlock=0;
  int l_blockXDim=0,l_blockYDim=0,l_blockZDim=0;
  MY_PRECISION *l_xBuf=0, *l_yBuf=0, *l_zBuf=0;
  int l_collX,l_collY,l_collZ;

  /*do we need all of these checks?*/
  if( a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets || a_whichVar<0 || 
      a_whichVar>=g_strGlobals->m_numVarsPerBlock[a_whichBlock] || a_whichTimestep<0 || 
      a_whichTimestep>=g_strGlobals->m_numTimesteps )
    {
      printinfo("read_block_coords_for_selected_variable_as_separable something out of range: a_whichBlock=%d, a_whichVar=%d, a_whichTimestep=%d\n",
		a_whichBlock,a_whichVar,a_whichTimestep );
      return(-1);
    }
  if( !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock] ||
      !g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar] )
    {
      printinfo("read_block_coords_for_selected_variable_as_separable m_instancePerTimestepPerVarPerBlock[%d][%d] not allocated\n",a_whichBlock,a_whichVar);
      return(-1);
    }
  l_field = g_strGlobals->m_instancePerTimestepPerVarPerBlock[a_whichBlock][a_whichVar][a_whichTimestep];
  if( !my_saf_is_valid_field_handle(&l_field)  ) 
    {
      printinfo("read_block_coords_for_selected_variable_as_separable trying to map a SS_FIELD_NULL\n");
      return(-1);
    }

  l_strMeshVarType = get_variable_type( a_whichBlock, a_whichVar, a_whichTimestep );


  saf_describe_field(SAF_ALL, &(l_field), &l_fieldTemplate, NULL, &l_set, NULL, 
		     &l_isCoordField, NULL, &l_cat, NULL, NULL, 
		     NULL, &l_varType, NULL, NULL, NULL, NULL);


  saf_describe_collection(SAF_ALL,&l_set,&l_cat,&l_cellType,&l_numEntriesInColl,&l_indexSpec,NULL,NULL);
  
  if(l_numEntriesInColl<=0)
    {
      printinfo("read_block_coords_for_selected_variable_as_separable, l_numEntriesInColl=%d?\n",l_numEntriesInColl);
      return(0);
    }

  l_collX = l_indexSpec.sizes[0];
  l_collY = l_indexSpec.sizes[1];
  l_collZ = l_indexSpec.sizes[2];

  get_block_size_from_set(&(g_strGlobals->m_sets[a_whichBlock]),&l_blockXDim,&l_blockYDim,&l_blockZDim);
  l_numEntriesInBlock=l_blockXDim*l_blockYDim*l_blockZDim;

  l_xBuf = (MY_PRECISION *)malloc(l_numEntriesInBlock*sizeof(MY_PRECISION));
  l_yBuf = (MY_PRECISION *)malloc(l_numEntriesInBlock*sizeof(MY_PRECISION));
  l_zBuf = (MY_PRECISION *)malloc(l_numEntriesInBlock*sizeof(MY_PRECISION));

  
  /*treat the coords as separable, whether they are or not*/
  read_set_coords_as_separable( g_strGlobals->m_sets[a_whichBlock], l_xBuf, l_yBuf, l_zBuf );
  

  /*now check if this var is mapped to the topset or to a subset*/
  if( !SAF_EQUIV( &(g_strGlobals->m_sets[a_whichBlock]), &l_set ) )
    {
      /*check to see if l_set is a subset of the block set*/
      struct_hyperslab l_slab;
      int l_finalX,l_finalY,l_finalZ,k;
      MY_PRECISION *l_ptr;

      /*9/19/02 fix: was using l_cat, but should only be using the "nodes" category,
	so this was failing if l_cat is "edges" e.g. ......I havent got the nodes 
	category stored globally, so will try SAF_ANY_CAT.....might be errors later*/
      if( get_subset_hyperslab(&(g_strGlobals->m_sets[a_whichBlock]),&l_set,SAF_ANY_CAT,&l_slab) )
	{
	  free(l_xBuf);
	  free(l_yBuf);
	  free(l_zBuf);
	  printinfo("read_block_coords_for_selected_variable_as_separable failed: couldnt get subset: probably subset of subset, not yet written\n");
	  exit(-1);
	}     



      /*If the SAF_IndexSpec.origin's are not zero, then the hyperslab will
	not be the way we want it (0-based). Change it.*/
      l_slab.x_start -= l_indexSpec.origins[0];
      l_slab.y_start -= l_indexSpec.origins[1];
      l_slab.z_start -= l_indexSpec.origins[2];


      l_finalX = l_slab.x_start + l_slab.x_count*l_slab.x_skip;
      l_finalY = l_slab.y_start + l_slab.y_count*l_slab.y_skip;
      l_finalZ = l_slab.z_start + l_slab.z_count*l_slab.z_skip;




      /*Check that the slab makes sense. In addition to other possible errors, 
	I'm not sure that everyone agrees that a hyperslab should be 1-based
	if the collection is 1-based*/
      if( l_slab.x_start<0 || l_slab.y_start<0 || l_slab.z_start<0 ||
	  l_finalX-l_slab.x_skip>=l_blockXDim || l_finalY-l_slab.y_skip>=l_blockYDim 
	  || l_finalZ-l_slab.z_skip>=l_blockZDim  )
	{
	  printinfo("read_block_coords_for_selected_variable_as_separable error: hyperslab is bad\n");
	  printinfo("  l_final*: %d %d %d\n",l_finalX,l_finalY,l_finalZ);
	  printinfo("  block dim: %d %d %d\n",l_blockXDim,l_blockYDim,l_blockZDim);
	  printinfo("  l_slab starts: %d %d %d\n",l_slab.x_start,l_slab.y_start,l_slab.z_start );
	  printinfo("  l_slab counts: %d %d %d\n",l_slab.x_count,l_slab.y_count,l_slab.z_count );
	  printinfo("  l_slab skips: %d %d %d\n",l_slab.x_skip,l_slab.y_skip,l_slab.z_skip );
	  printinfo("  ispec origins: %d %d %d\n",l_indexSpec.origins[0],l_indexSpec.origins[1],l_indexSpec.origins[2] );

	  exit(-1);
	}





      l_ptr = l_xBuf;
      for(k=l_slab.x_start;k<l_finalX;k+=l_slab.x_skip)
	{
	  l_ptr[0] = l_xBuf[k];
	  l_ptr++;
	}

      l_ptr = l_yBuf;
      for(k=l_slab.y_start;k<l_finalY;k+=l_slab.y_skip)
	{
	  l_ptr[0] = l_yBuf[k];
	  l_ptr++;
	}

      l_ptr = l_zBuf;
      for(k=l_slab.z_start;k<l_finalZ;k+=l_slab.z_skip)
	{
	  l_ptr[0] = l_zBuf[k];
	  l_ptr++;
	}
    }




  memcpy(a_x,l_xBuf,l_collX*sizeof(MY_PRECISION));
  memcpy(a_y,l_yBuf,l_collY*sizeof(MY_PRECISION));
  memcpy(a_z,l_zBuf,l_collZ*sizeof(MY_PRECISION));

  free(l_xBuf);
  free(l_yBuf);
  free(l_zBuf);

  return(0);
}/*read_block_coords_for_selected_variable_as_separable*/



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read Coords from a Given Block Set as Curvilinear    
 * Description:
 *
 * read block coords, whether they are stored as curvilinear or separable,
 * as curvilinear coords (i.e. block size AxBxC 
 * representable by 3 1-D arrays of size A*B*C)
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_block_coords_as_curvilinear_from_set( SAF_Set a_set, MY_PRECISION *a_xCoords, 
					       MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords )
{
  if(!read_separable_3d_coords(a_set,0,0,0))
    {
      /*
      ** READ SEPARABLE COORDINATE DATA
      */
      int l_xDim,l_yDim,l_zDim,l_count,i,j,k;
      MY_PRECISION *l_xBuf,*l_yBuf,*l_zBuf;
      get_block_size_from_set(&a_set,&l_xDim,&l_yDim,&l_zDim);

      if( l_xDim<=0 || l_yDim<=0 || l_zDim<=0 )
	{
	  printinfo("read_block_coords_as_curvilinear_from_set error: block size %dx%dx%d\n",l_xDim,l_yDim,l_zDim);
	  exit(-1);
	}

      l_xBuf = (MY_PRECISION *)malloc(l_xDim*sizeof(MY_PRECISION));
      l_yBuf = (MY_PRECISION *)malloc(l_yDim*sizeof(MY_PRECISION));
      l_zBuf = (MY_PRECISION *)malloc(l_zDim*sizeof(MY_PRECISION));

      if( l_xBuf==0 || l_yBuf==0 || l_zBuf==0 )
	{
	  printinfo("read_block_coords_as_curvilinear_from_set error: malloc failed\n");
	  exit(-1);
	}

      if(read_separable_3d_coords( a_set, l_xBuf, l_yBuf, l_zBuf ))
	{
	  printinfo("read_block_coords_as_curvilinear_from_set: read_separable_3d_coords error\n");
	}

      l_count=0;
      for(k=0; k<l_zDim; k++ )
	{
	  for(j=0; j<l_yDim; j++)
	    {
	      for(i=0; i<l_xDim; i++)
		{
		  if(a_xCoords) a_xCoords[l_count]=l_xBuf[i];
		  if(a_yCoords) a_yCoords[l_count]=l_yBuf[j];
		  if(a_zCoords) a_zCoords[l_count]=l_zBuf[k];
		  l_count++;
		}
	    }
	}

      free(l_xBuf);
      free(l_yBuf);
      free(l_zBuf);
    }
  else
    {
      /*
      ** READ CURVILINEAR COORDINATE DATA
      */
      if(!does_block_contain_3d_curvilinear_coords(a_set)) return(-1);

      read_curvilinear_3d_coords( a_set, a_xCoords, a_yCoords, a_zCoords );
    } 

  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read Coords from a Given Block as Curvilinear   
 * Description:
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_block_coords_as_curvilinear( int a_whichBlock, MY_PRECISION *a_xCoords, 
				      MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords )
{
  if( !g_strGlobals->m_sets || a_whichBlock<0 || a_whichBlock>=g_strGlobals->m_numSets )
    {
      printinfo("read_block_coords_as_curvilinear error: a_whichBlock=%d m_numSets=%d\n",a_whichBlock,g_strGlobals->m_numSets);
      exit(-1);
    }
  
  return( read_block_coords_as_curvilinear_from_set( g_strGlobals->m_sets[a_whichBlock], a_xCoords, a_yCoords, a_zCoords ) );
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read Curvilinear Coords from a Given Set
 * Description:
 *
 * read block coords assuming they are NOT separable (i.e. block size AxBxC 
 * representable by 3 1-D arrays of size AxBxC)
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_curvilinear_3d_coords( SAF_Set a_set, MY_PRECISION *a_xCoords, MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords )
{
  SAF_IndexSpec l_indexSpec;
  int l_count, l_numCollections=0, i, j, l_done=0;
  SAF_Cat *l_cats=0;
  SAF_CellType l_cellType;
  int l_x,l_y,l_z;

  if(!my_saf_is_valid_set_handle(&a_set)) return(-1);

  saf_describe_set(SAF_ALL,&(a_set),NULL,NULL,NULL,NULL,NULL,&l_numCollections,&l_cats);

  for( i=0; i<l_numCollections; i++ )
    {
      int l_numFields=0;
      SAF_Field *l_fields=0;
      saf_describe_collection(SAF_ALL,&a_set,&(l_cats[i]),&l_cellType,&l_count,&l_indexSpec,NULL,NULL);
      if( l_indexSpec.ndims == 3 )
	{
	  l_x = l_indexSpec.sizes[0];
	  l_y = l_indexSpec.sizes[1];
	  l_z = l_indexSpec.sizes[2];


	  /*note: we dont care what the SAF_IndexSpec.origin's are: would not affect the result*/

	  printinfo("read_curvilinear_3d_coords found %d nodes: %dx%dx%d\n",l_count,l_x,l_y,l_z );

	  saf_find_fields(SAF_ALL, g_strGlobals->m_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
			  &(l_cats[i]), SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);

	  for( j = 0 ; j < l_numFields; j++ )
	    {
	      SAF_FieldTmpl *l_fieldTemplate=0;
	      SAF_Field *l_componentFields=0;
	      hbool_t l_isCoordField;
	      int l_numComponents;
	      hid_t l_varType;

	      if(!my_saf_is_valid_field_handle(&(l_fields[j]))) continue;

	      saf_describe_field(SAF_ALL, &(l_fields[j]), l_fieldTemplate, NULL, NULL, NULL,
				 &l_isCoordField, NULL, NULL, NULL, NULL, 
				 NULL, &l_varType, &l_numComponents, &l_componentFields, NULL, NULL);


	      if( l_numComponents==3 && l_isCoordField )
		{
		  if( l_varType<0 ) /*sslib? H5Tequal(l_varType , H5I_INVALID_HID) )*/
		    {
		      l_varType = guess_dsl_var_type_for_composite_coord_field(l_numFields,l_fields);
		    }
		  if(a_xCoords)
		    if(read_whole_field_to_my_precision(&(l_componentFields[0]),l_varType,(size_t)l_count,a_xCoords))
		      {
			printinfo("read_curvilinear_3d_coords error reading x field\n");
			exit(-1);
		      }
		  if(a_yCoords)
		    if(read_whole_field_to_my_precision(&(l_componentFields[1]),l_varType,(size_t)l_count,a_yCoords))
		      {
			printinfo("read_curvilinear_3d_coords error reading y field\n");
			exit(-1);
		      }
		  if(a_zCoords)
		    if(read_whole_field_to_my_precision(&(l_componentFields[2]),l_varType,(size_t)l_count,a_zCoords))
		      {
			printinfo("read_curvilinear_3d_coords error reading z field\n");
			exit(-1);
		      }



		  /* change the ordering if needed */
		  {
		    SAF_IndexSpec l_indexSpecCorrected = l_indexSpec;
		    l_indexSpecCorrected.order[0]=2;
		    l_indexSpecCorrected.order[1]=1;
		    l_indexSpecCorrected.order[2]=0;


		    if( l_indexSpec.order[0]!=l_indexSpecCorrected.order[0] || 
			l_indexSpec.order[1]!=l_indexSpecCorrected.order[1] ||
			l_indexSpec.order[2]!=l_indexSpecCorrected.order[2] )
		      {
			printinfo("read_curvilinear_3d_coords: changing ordering from %d %d %d  to %d %d %d\n",
				  l_indexSpec.order[0],
				  l_indexSpec.order[1],
				  l_indexSpec.order[2],
				  l_indexSpecCorrected.order[0],
				  l_indexSpecCorrected.order[1],
				  l_indexSpecCorrected.order[2] );
		      }

		    if(a_xCoords) change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, 
							  (int)H5Tget_size(l_varType), (void *)(a_xCoords) );
		    if(a_yCoords) change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, 
							  (int)H5Tget_size(l_varType), (void *)(a_yCoords) );		 
		    if(a_zCoords) change_ordering_scheme( l_indexSpec, l_indexSpecCorrected, 
							  (int)H5Tget_size(l_varType), (void *)(a_zCoords) );		      
		  }




		  l_done=1;
		  break;
		}
	    }

	}
      else
	{
	  printinfo("read_curvilinear_3d_coords: l_indexSpec.ndims=%d, this must not be a 3d structured block set!\n",l_indexSpec.ndims);
	}
      if(l_done) break;
    }

  return(0);
} /* end of read_curvilinear_3d_coords */




/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Eight Structured Mesh Collections     
 * Description:
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_str_mesh_collections( SAF_Set a_set,SAF_Cat **a_nodes, SAF_Cat **a_xEdges, 
			      SAF_Cat **a_yEdges, SAF_Cat **a_zEdges, SAF_Cat **a_xFaces, 
			      SAF_Cat **a_yFaces, SAF_Cat **a_zFaces, SAF_Cat **a_elems )
{
  SAF_CellType l_cellType;
  SAF_IndexSpec l_indexSpec;
  SAF_Cat *l_cats=0;
  int j,l_xDim,l_yDim,l_zDim;
  int l_numCollections=0;

  *a_nodes=0;*a_xEdges=0;*a_yEdges=0;*a_zEdges=0;*a_xFaces=0;*a_yFaces=0;*a_zFaces=0;*a_elems=0;
  get_block_size_from_set(&a_set,&l_xDim,&l_yDim,&l_zDim);

  saf_describe_set(SAF_ALL, &(a_set), NULL, NULL, NULL, NULL, NULL, &l_numCollections, &l_cats);

  for( j=0; j<l_numCollections; j++ )
    {
      int l_x,l_y,l_z;
      saf_describe_collection(SAF_ALL,&a_set,&(l_cats[j]),&l_cellType,NULL,&l_indexSpec,NULL,NULL);	
      
      l_x = l_indexSpec.sizes[0];
      l_y = l_indexSpec.sizes[1];
      l_z = l_indexSpec.sizes[2];
      
      if( l_cellType == SAF_CELLTYPE_POINT )
	{
	  *a_nodes = &l_cats[j];
	}
      else if( l_cellType == SAF_CELLTYPE_LINE )
	{
	  if( l_xDim-1 == l_x ) *a_xEdges = &l_cats[j];
	  else if( l_yDim-1 == l_y ) *a_yEdges = &l_cats[j];
	  else if( l_zDim-1 == l_z ) *a_zEdges = &l_cats[j];
	  else printinfo("   unexpected collection of SAF_CELLTYPE_LINE?\n");
	}
      else if( l_cellType == SAF_CELLTYPE_QUAD )
	{
	  if( l_xDim == l_x ) *a_xFaces = &l_cats[j];
	  else if( l_yDim == l_y ) *a_yFaces = &l_cats[j];
	  else if( l_zDim == l_z ) *a_zFaces = &l_cats[j];
	  else printinfo("   unexpected collection of SAF_CELLTYPE_QUAD?\n");
	}
      else if( l_cellType == SAF_CELLTYPE_HEX )
	{
	  *a_elems = &l_cats[j];
	}
      else 
	printinfo("    l_cellType == unexpected type?\n");
    }

  if( !*a_nodes || !*a_xEdges || !*a_yEdges || !*a_zEdges || !*a_xFaces || !*a_yFaces || !*a_zFaces || !*a_elems )
    {
      printinfo("error? get_str_mesh_collections didnt find all the expected collections\n");
      exit(-1);
    }
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Variable Instances on Subsets of a Given Set
 * Description:
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_var_instances_on_subsets_of_set( SAF_Set *a_set, SAF_Cat *a_coll, int a_numSubsets, SAF_Set *a_subsets  )
{
  int j, l_numVarInstances=0;
  for( j=0; j<a_numSubsets; j++ )
    {
      int l_numRels=0;
      saf_find_subset_relations(SAF_ALL, g_strGlobals->m_db, a_set, &(a_subsets[j]), a_coll, a_coll, 
				SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE,	&l_numRels, NULL );
      if( l_numRels>1 )
	{
	  printinfo("get_num_var_instances_on_subsets_of_set error? found %d subset relations\n",l_numRels);
	}
      else if( l_numRels==1 )
	{
	  l_numVarInstances += get_num_variable_fields_on_set( a_subsets[j] );

	  /*note: should call get_num_variable_fields_on_set here, go recursively thru the whole tree*/
	}
    }
  return(l_numVarInstances);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Total Number of Variable Instances on all Structured Sets
 * Description:
 *
 * This is currently not used.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_total_num_var_instances( )
{
  int l_numVars=0, i;
  for(i=0;i<g_strGlobals->m_numSets;i++)
    {
      SAF_Cat *l_nodes=0,*l_xEdges=0,*l_yEdges=0,*l_zEdges=0,*l_xFaces=0,*l_yFaces=0,*l_zFaces=0,*l_elems=0; 
      l_numVars += get_num_variable_fields_on_set( g_strGlobals->m_sets[i] );
      get_str_mesh_collections( g_strGlobals->m_sets[i], &l_nodes, &l_xEdges, &l_yEdges, &l_zEdges, &l_xFaces, 
				&l_yFaces, &l_zFaces, &l_elems );
      l_numVars += get_num_var_instances_on_subsets_of_set( &(g_strGlobals->m_sets[i]), l_nodes, g_strGlobals->m_numSubsets, g_strGlobals->m_subsets  );
      l_numVars += get_num_var_instances_on_subsets_of_set( &(g_strGlobals->m_sets[i]), l_xEdges, g_strGlobals->m_numEdgeSubsets, g_strGlobals->m_edgeSubsets  );
      l_numVars += get_num_var_instances_on_subsets_of_set( &(g_strGlobals->m_sets[i]), l_yEdges, g_strGlobals->m_numEdgeSubsets, g_strGlobals->m_edgeSubsets  );
      l_numVars += get_num_var_instances_on_subsets_of_set( &(g_strGlobals->m_sets[i]), l_zEdges, g_strGlobals->m_numEdgeSubsets, g_strGlobals->m_edgeSubsets  );
      l_numVars += get_num_var_instances_on_subsets_of_set( &(g_strGlobals->m_sets[i]), l_xFaces, g_strGlobals->m_numFaceSubsets, g_strGlobals->m_faceSubsets  );
      l_numVars += get_num_var_instances_on_subsets_of_set( &(g_strGlobals->m_sets[i]), l_yFaces, g_strGlobals->m_numFaceSubsets, g_strGlobals->m_faceSubsets  );      
      l_numVars += get_num_var_instances_on_subsets_of_set( &(g_strGlobals->m_sets[i]), l_zFaces, g_strGlobals->m_numFaceSubsets, g_strGlobals->m_faceSubsets  );
      l_numVars += get_num_var_instances_on_subsets_of_set( &(g_strGlobals->m_sets[i]), l_elems, g_strGlobals->m_numElemSubsets, g_strGlobals->m_elemSubsets  );
    }
  return(l_numVars);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Get the Number of Variable Instances on Full Blocks
 * Description:
 *
 * This is currently not used.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
get_num_var_instances_on_full_blocks( int **a_timesteps, int **a_blockNumbers )
{
  int l_numVars=0, i, j;
  for(i=0;i<g_strGlobals->m_numSets;i++)
    {
      l_numVars += get_num_variable_fields_on_set( g_strGlobals->m_sets[i] );
    }
  if( a_blockNumbers && l_numVars )
    {
      int *l_ptr;
      a_blockNumbers[0] = (int *)malloc(l_numVars*sizeof(int));
      l_ptr = a_blockNumbers[0];
      for(i=0;i<g_strGlobals->m_numSets;i++)
	{
	  int l_num = get_num_variable_fields_on_set( g_strGlobals->m_sets[i] );
	  for(j=0;j<l_num;j++)
	    {
	      l_ptr[0]=i;
	      l_ptr++;
	    }
	}
    }
  if( a_timesteps && l_numVars )
    {
      a_timesteps[0] = (int *)malloc(l_numVars*sizeof(int));

    }
  return(l_numVars);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Does a Set Contain 3D Curvilinear Coords?
 * Description:
 *
 * Returns 1 if true, 0 otherwise.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
does_block_contain_3d_curvilinear_coords( SAF_Set a_set )
{
  SAF_IndexSpec l_indexSpec;
  int l_count, l_numCollections=0, i, j, l_success=0;
  SAF_Cat *l_cats=0;
  SAF_CellType l_cellType;
  char *l_setName=0;
  
  saf_describe_set(SAF_ALL,&(a_set),&l_setName,NULL,NULL,NULL,NULL,&l_numCollections,&l_cats);
  for( i=0; i<l_numCollections; i++ )
    {
      saf_describe_collection(SAF_ALL,&a_set,&(l_cats[i]),&l_cellType,&l_count,&l_indexSpec,NULL,NULL);
      if( l_indexSpec.ndims==3 && l_cellType==SAF_CELLTYPE_POINT && l_count )
	{
	  int l_numFields=0;
	  SAF_Field *l_fields=0;
	  if(l_success)
	    {
	      printinfo("does_block_contain_3d_curvilinear_coords(%s) warning: >1 3d POINT cat\n",l_setName);
	    }
	  saf_find_fields(SAF_ALL, g_strGlobals->m_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
			  &(l_cats[i]), SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);

	  for( j = 0 ; j < l_numFields; j++ )
	    {
	      SAF_Field *l_componentFields=0;
	      char *l_fieldName=0;
	      hbool_t l_isCoordField;
	      int l_numComponents=0,l_whichComp;

	      if(!my_saf_is_valid_field_handle(&(l_fields[j]))) continue;

	      saf_describe_field(SAF_ALL, &(l_fields[j]), NULL, &l_fieldName, NULL, NULL,
				 &l_isCoordField, NULL, NULL, NULL, NULL, 
				 NULL, NULL, &l_numComponents, &l_componentFields, NULL, NULL);


	      if( l_numComponents==3 && l_isCoordField )
		{
		  int l_allCompsOk=1;/*true for now*/

		  if(l_success)
		    {
		      printinfo("does_block_contain_3d_curvilinear_coords(%s) warning: >1 3d coord field\n",l_setName);
		    }

		  for(l_whichComp=0;l_whichComp<l_numComponents;l_whichComp++)
		    {
		      char *l_compFieldName=0;
		      hbool_t l_compIsCoordField;
		      int l_numCompComponents=0, l_assocRatio=0;
		      SAF_Cat l_compCat;

		      if(!my_saf_is_valid_field_handle( &(l_componentFields[l_whichComp]))) 
			{
			  l_allCompsOk=0;/*this component is NOT ok*/
			  break;
			}
			  

		      saf_describe_field(SAF_ALL, &(l_componentFields[l_whichComp]), NULL, &l_compFieldName, NULL, NULL,
					 &l_compIsCoordField, NULL, &l_compCat, &l_assocRatio, NULL, 
					 NULL, NULL, &l_numCompComponents, NULL, NULL, NULL);


		      if( SAF_EQUIV(&l_compCat,&(l_cats[i])) && l_compIsCoordField && l_assocRatio==1 
			  && l_numCompComponents==1 )
			{
			  /*this component is ok*/
			}
		      else 
			{
			  l_allCompsOk=0;/*this component is NOT ok*/
			  break;
			}
		    }
		  if(l_success && l_allCompsOk)
		    {
		      printinfo("does_block_contain_3d_curvilinear_coords(%s) warning: >1 valid curv coords\n",l_setName);
		    }
		  else if(l_allCompsOk) 
		    {
		      l_success=1;
		    }
		}
	    }
	  if(l_fields) free(l_fields);
	}
    }

  /*printinfo("does_block_contain_3d_curvilinear_coords(%s) returns %d\n",l_setName,l_success);*/


  if(l_setName) free(l_setName);

  return(l_success);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Structured Mesh Reader
 * Purpose:     Read Separable 3D Coords from a Set
 * Description:
 *
 * returns 0 on success (if the set does contain separable coords)
 *
 * If a_xCoords is non-zero, then the x coords will be copied to it. etc
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
read_separable_3d_coords( SAF_Set a_set, MY_PRECISION *a_xCoords,
			      MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords )
{
  SAF_IndexSpec l_indexSpec;
  int l_count, l_numCollections=0, i, j, l_success=0;
  SAF_Cat *l_cats=0;
  SAF_CellType l_cellType;
  char *l_setName=0;

  saf_describe_set(SAF_ALL,&(a_set),&l_setName,NULL,NULL,NULL,NULL,&l_numCollections,&l_cats);
  for( i=0; i<l_numCollections; i++ )
    {
      saf_describe_collection(SAF_ALL,&a_set,&(l_cats[i]),&l_cellType,&l_count,&l_indexSpec,NULL,NULL);
      if( l_indexSpec.ndims==3 && l_cellType==SAF_CELLTYPE_POINT && l_count )
	{
	  int l_numFields=0;
	  SAF_Field *l_fields=0;
	  if(l_success)
	    {
	      printinfo("read_separable_3d_coords(%s) warning: >1 3d POINT cat\n",l_setName);
	    }
	  saf_find_fields(SAF_ALL, g_strGlobals->m_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
			  &(l_cats[i]), SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);

	  for( j = 0 ; j < l_numFields; j++ )
	    {
	      SAF_Field *l_componentFields=0;
	      char *l_fieldName=0;
	      hbool_t l_isCoordField;
	      int l_numComponents=0,l_whichComp;

	      if(!my_saf_is_valid_field_handle( &(l_fields[j])) ) continue;

	      saf_describe_field(SAF_ALL, &(l_fields[j]), NULL, &l_fieldName, NULL, NULL,
				 &l_isCoordField, NULL, NULL, NULL, NULL, 
				 NULL, NULL, &l_numComponents, &l_componentFields, NULL, NULL);

	      if( l_numComponents==3 && l_isCoordField )
		{
		  int l_allCompsOk=1;/*true for now*/

		  if(l_success)
		    {
		      printinfo("read_separable_3d_coords(%s) warning: >1 3d coord field\n",l_setName);
		    }

		  for(l_whichComp=0;l_whichComp<l_numComponents;l_whichComp++)
		    {
		      char *l_compFieldName=0;
		      hbool_t l_compIsCoordField;
		      int l_numCompComponents=0, l_assocRatio=0;
		      SAF_Cat l_compCat;
		      hid_t l_dsltype;
		      size_t l_thisCount=0;
		      saf_get_count_and_type_for_field(SAF_ALL, &(l_componentFields[l_whichComp]), NULL, &l_thisCount, &l_dsltype);

		      saf_describe_field(SAF_ALL, &(l_componentFields[l_whichComp]), NULL, &l_compFieldName, NULL, NULL,
					 &l_compIsCoordField, NULL, &l_compCat, &l_assocRatio, NULL, 
					 NULL, NULL, &l_numCompComponents, NULL, NULL, NULL);


		      if( !SAF_EQUIV(&l_compCat,&(l_cats[i])) && l_compIsCoordField 
			  && l_assocRatio==l_indexSpec.sizes[l_whichComp] && l_numCompComponents==1
			  && (int)l_thisCount==l_assocRatio )
			{
			  /*this component is ok*/
			  MY_PRECISION *l_ptr=0;
			  if(l_whichComp==0) l_ptr=a_xCoords;
			  else if(l_whichComp==1) l_ptr=a_yCoords;
			  else l_ptr=a_zCoords;
			  if( l_ptr )
			    {
			      if(read_whole_field_to_my_precision(&(l_componentFields[l_whichComp]),l_dsltype,(size_t)l_thisCount,l_ptr))
				{
				  printinfo("read_separable_3d_coords(%s)(%s)(%s) error: read_whole_field_to_my_precision failed\n",
					    l_setName,l_fieldName,l_compFieldName);
				  exit(-1);
				}
			    }
			}
		      else 
			{
			  l_allCompsOk=0;/*this component is NOT ok*/
			  break;
			}
		    }
		  if(l_success && l_allCompsOk)
		    {
		      printinfo("read_separable_3d_coords(%s) warning: >1 valid curv coords\n",l_setName);
		    }
		  else if(l_allCompsOk) 
		    {
		      l_success=1;
		    }
		}
	    }
	  if(l_fields) free(l_fields);
	}
    }
  if(l_success)
    {
      /*printinfo("read_separable_3d_coords(set %s): this is separable data\n",l_setName);*/
      if(l_setName) free(l_setName);
      return(0);
    }
  else
    {
      if(l_setName) free(l_setName);
      return(-1);
    }
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Are Two Structured Mesh Variable Types Similar?     
 * Description:
 *
 * Returns 1 if the given structured variable types are "similar": i.e. equal, both are edges (xedge,
 * yedge or zedge), or both are faces (xface, yface or zface). Otherwise returns 0.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
are_similar_str_mesh_var_types( saf_str_mesh_var_type a_left, saf_str_mesh_var_type a_right )
{
  if(a_left==a_right) return(1);/*they are the same*/

  if(a_left==SAF_STR_MESH_XEDGE || a_left==SAF_STR_MESH_YEDGE || a_left==SAF_STR_MESH_ZEDGE)
    if(a_right==SAF_STR_MESH_XEDGE || a_right==SAF_STR_MESH_YEDGE || a_right==SAF_STR_MESH_ZEDGE)
      {
	/*both are edges, call them similar*/
	return(1);
      }

  if(a_left==SAF_STR_MESH_XFACE || a_left==SAF_STR_MESH_YFACE || a_left==SAF_STR_MESH_ZFACE)
    if(a_right==SAF_STR_MESH_XFACE || a_right==SAF_STR_MESH_YFACE || a_right==SAF_STR_MESH_ZFACE)
      {
	/*both are faces, call them similar*/
	return(1);
      }
  
  return(0); /*not similar*/
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Determine if Two Fields Should Belong to the Same Variable
 *
 * Description: 
 *
 * This is an important question for a reader: how does a it
 * know if two fields with no set in common are supposed to represent 
 * the same thing (i.e. are different instances of the same variable)? This function 
 * compares the field templates, units, quantity, cell type, and coordinate status.
 *
 * Issue:
 * SIERRA files do not always
 * use units and quantities, and use the same field templates for different variables.
 * I think the only quantities SIERRA uses are "not applicable" and "length".
 *
 * Return: 1 if the fields could be the same variable, 0 if not
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
are_these_two_fields_from_the_same_var(SAF_Field *a_fieldA, SAF_Field *a_fieldB )
{
  hbool_t l_isCoordFieldA,l_isCoordFieldB;
  SAF_Unit l_unitA,l_unitB;
  SAF_Quantity l_quantityA,l_quantityB;
  char *l_fieldNameA=0, *l_fieldNameB=0;
  SAF_FieldTmpl l_fieldTemplateA, l_fieldTemplateB;
  SAF_Cat l_catA, l_catB;
  SAF_Set l_setA, l_setB;
  SAF_CellType l_cellTypeA, l_cellTypeB;
  SAF_Algebraic l_algTypeA, l_algTypeB;
  int l_result = -1;/*-1 means undecided*/

  if( !my_saf_is_valid_field_handle(a_fieldA) || !my_saf_is_valid_field_handle(a_fieldB) ) return(0);

  saf_describe_field(SAF_ALL, a_fieldA, &l_fieldTemplateA, &l_fieldNameA, &l_setA, &l_unitA, 
		     &l_isCoordFieldA, NULL, &l_catA, NULL, NULL, 
		     NULL, NULL, NULL, NULL, NULL, NULL);
  saf_describe_field(SAF_ALL, a_fieldB, &l_fieldTemplateB, &l_fieldNameB, &l_setB, &l_unitB, 
		     &l_isCoordFieldB, NULL, &l_catB, NULL, NULL, 
		     NULL, NULL, NULL, NULL, NULL, NULL);
 


  /*If the template is the same, of course they are the same var. Note that having the 
    same template means they have to have the same algebraic type and quantity (see saf_declare_field_tmpl)*/
  if( SAF_EQUIV(&l_fieldTemplateA,&l_fieldTemplateB) ) 
    {
      l_result=1;
    }
  



#if 1
  /*Note that if we require the cats to be the same, then two structured cats like xEdges and yEdges
    will not be allowed to be on the same variable. Instead, we require the cat dimension to be the
    same. Fortuneately in the unstr reader many unwanted categories will have already been removed.
    Added this because of the unstructured new subset handling 26no2003
  */
  /*Note NEED TO CHECK THAT THIS TEST DOESNT TAKE THE PLACE OF SOME OF THE LATER ONES XXX jake hooey*/
  if( l_result<0 )
    {
      int l_dimA=0,l_dimB=0;
      char *l_nameA=NULL,*l_nameB=NULL;
      saf_describe_category(SAF_ALL,&l_catA,&l_nameA,NULL,&l_dimA);
      saf_describe_category(SAF_ALL,&l_catB,&l_nameB,NULL,&l_dimB);

      /*if( !SAF_EQUIV(&l_catA,&l_catB) )*/
      if( l_dimA!=l_dimB )
	{
	  /*
	    printf("are_these_two_fields_from_the_same_var fields %s %s \tcats %s %s \tdim %d %d\n",
	    l_fieldNameA,l_fieldNameB,l_nameA,l_nameB, l_dimA, l_dimB);
	  */
	  l_result=0;
	}

      free(l_nameA);free(l_nameB);
    }
#endif
  



  if( l_result<0 && !SAF_EQUIV(&l_unitA,&l_unitB) ) 
    {
      l_result=0;
    }

  if( l_result<0 && l_isCoordFieldA != l_isCoordFieldB ) 
    {
      l_result=0;
    }

  if( l_result<0 )
    {
      saf_describe_unit(SAF_ALL,&l_unitA,NULL,NULL,NULL,NULL,NULL,NULL,NULL, &l_quantityA );
      saf_describe_unit(SAF_ALL,&l_unitB,NULL,NULL,NULL,NULL,NULL,NULL,NULL, &l_quantityB );
      if( !SAF_EQUIV(&l_quantityA,&l_quantityB) )
	{
	  l_result=0;
	} 
    }


  if( l_result<0 )
    {
      saf_describe_field_tmpl(SAF_ALL,&(l_fieldTemplateA),NULL,&l_algTypeA,NULL,NULL,NULL,NULL);
      saf_describe_field_tmpl(SAF_ALL,&(l_fieldTemplateB),NULL,&l_algTypeB,NULL,NULL,NULL,NULL);
      if( !SAF_EQUIV(&l_algTypeA,&l_algTypeB) ) 
	{
	  l_result=0;
	}
    }



  /*Finally, are the field names the same? This should be too strict, but because
    of the way the first SIERRA files are written, we need it for unstr meshes. And
    because of the way the first EMPHASIS files are written, we need to turn it off
    for str meshes.*/
  if( l_result<0 )
    {
      int l_sameNameSameVariable=0;
      char *l_unitNameA=0,*l_unitNameB=0;
      saf_describe_unit(SAF_ALL,&l_unitA,&l_unitNameA,NULL,NULL,NULL,NULL,NULL,NULL, NULL );
      saf_describe_unit(SAF_ALL,&l_unitB,&l_unitNameB,NULL,NULL,NULL,NULL,NULL,NULL, NULL );

      /*Note that at this point we already know that l_unitA==l_unitB*/
      if( SAF_EQUIV(SAF_NOT_APPLICABLE_UNIT,&l_unitA)
	  /*|| SAF_EQUIV(SAF_NOT_SET_UNIT,&l_unitA) hooey causes warning in sslib.....do we need all this anymore?*/
	  || SAF_EQUIV(SAF_NOT_IMPLEMENTED_UNIT,&l_unitA)
	  || SAF_EQUIV(SAF_ANY_UNIT,&l_unitA)
	  || !strlen(l_unitNameA) )
	{
	  l_sameNameSameVariable=1;
	}
      if(l_unitNameA) free(l_unitNameA);
      if(l_unitNameB) free(l_unitNameB);

      if( l_sameNameSameVariable && !strcmp(l_fieldNameA,l_fieldNameB) )
	{	  
	  l_result=1;
	}



#if 1
      /*Only require the celltypes to be the same if we dont really know the units.
	This is just in case it was written by a sierra file. Normally, having the
	celltypes the same shouldnt be required, e.g. stress on a hex elem is like stress
	on a tet elem.*/
      if( l_sameNameSameVariable && l_result<0 )
	{
	  saf_describe_collection(SAF_ALL,&l_setA,&l_catA,&l_cellTypeA,NULL,NULL,NULL,NULL);
	  saf_describe_collection(SAF_ALL,&l_setB,&l_catB,&l_cellTypeB,NULL,NULL,NULL,NULL);
	  if( l_cellTypeA != l_cellTypeB ) 
	    {
	      l_result=0;
	    }
	}
#endif


    }


  if(l_fieldNameA) free(l_fieldNameA);
  if(l_fieldNameB) free(l_fieldNameB);

  if( l_result<0 ) l_result=0;/*if still undecided, then they are NOT the same var*/

  return(l_result);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Change an Array's Indexing Order
 * Description:
 *
 * Takes an array a_data whose ordering (e.g. C-style, Fortran-style) is defined
 * by a_oldIspec, and change it to the ordering specified by a_newIspec. See the
 * definition of SAF_IndexSpec.
 * 
 *--------------------------------------------------------------------------------------------------- */
void 
STR_CLASS_NAME_IF_CPP
change_ordering_scheme( SAF_IndexSpec a_oldIspec, SAF_IndexSpec a_newIspec,
			     int a_bytesPerType, void *a_data )
{
  int i,j,l_dim,l_numEntries;
  int *l_oldIndex=0,*l_newIndex=0,*l_realIndex=0;
  int *l_oldLoopSize=0,*l_newLoopSize=0;

  /*check that the dimensions of the ispecs are the same*/
  if( a_oldIspec.ndims != a_newIspec.ndims )
    {
      printinfo("change_ordering_scheme error  a_oldIspec.ndims=%d a_newIspec.ndims=%d\n",
		a_oldIspec.ndims, a_newIspec.ndims );
      exit(-1);
    }
  l_dim = a_oldIspec.ndims;
  if(l_dim<2) return;/*ordering is meaningless for a 1d array*/
 
  /*check that the requested ordering is actually different from the existing*/
  {
    int l_isDifferent=0;
    for( i=0; i<l_dim; i++ )
      {
	if( a_oldIspec.order[i] != a_newIspec.order[i] )
	  {
	    l_isDifferent=1;
	    break;
	  }
      }
    if(!l_isDifferent) return; /*do nothing: ordering is the same*/
  }

  /*check that the orders make some sense*/
  for( i=0; i<l_dim; i++ )
    {
      if( a_oldIspec.order[i]<0 || a_oldIspec.order[i]>=l_dim ||
	  a_newIspec.order[i]<0 || a_newIspec.order[i]>=l_dim )
	{
	  printinfo("change_ordering_scheme error: bad SAF_IndexSpec orders old[%d]=%d new[%d]=%d  l_dim=%d\n",
		    i, a_oldIspec.order[i], i, a_newIspec.order[i], l_dim );
	  exit(-1);
	}
    }


  l_numEntries = a_oldIspec.sizes[0];
  for( i=1; i<l_dim; i++ ) l_numEntries *= a_oldIspec.sizes[i];

  /*note: not checking that the new ispec sizes match the old*/

  checkBufferSize( l_numEntries*a_bytesPerType/ (int)sizeof(MY_PRECISION) );
  memcpy( g_strGlobals->m_buffer, a_data, (size_t)(l_numEntries*a_bytesPerType) );



  l_oldLoopSize = (int *)malloc(l_dim*sizeof(int));
  l_oldLoopSize[l_dim-1]=1;/*the inner loop size is 1, of course*/
  for( j=l_dim-2; j>=0; j-- )/*j==0 is the outer loop*/
    {
      int l_lastDimIndex = a_oldIspec.order[j+1];

      l_oldLoopSize[j] = l_oldLoopSize[j+1]*a_oldIspec.sizes[l_lastDimIndex];
    }



  l_newLoopSize = (int *)malloc(l_dim*sizeof(int));
  l_newLoopSize[l_dim-1]=1;/*the inner loop size is 1, of course*/
  for( j=l_dim-2; j>=0; j-- )/*j==0 is the outer loop*/
    {
      int l_lastDimIndex = a_newIspec.order[j+1];
      l_newLoopSize[j] = l_newLoopSize[j+1]*a_newIspec.sizes[l_lastDimIndex];
    }


  l_oldIndex = (int *)malloc(l_dim*sizeof(int));
  l_newIndex = (int *)malloc(l_dim*sizeof(int));
  l_realIndex = (int *)malloc(l_dim*sizeof(int));
  for( i=0; i<l_numEntries; i++ )
    {
      int l_remainder, l_iNew;

      /*figure what i,j,k (so to speak) index this entry is in the OLD ordering*/
      l_remainder = i;
      for( j=0; j<l_dim; j++ )
	{
	  l_oldIndex[j] = l_remainder/l_oldLoopSize[j];
	  l_remainder -= l_oldIndex[j]*l_oldLoopSize[j];
	}
      if( l_remainder!=0 )
	{
	  printinfo("change_ordering_scheme error calculating oldIndex l_remainder\n");
	  exit(-1);
	}

      /*figure out what xyz index this is in the default (natural) xyz ordering*/
      for( j=0; j<l_dim; j++ )
	{
	  l_realIndex[j] = l_oldIndex[ a_oldIspec.order[j] ];
	}

      /*figure out what i,j,k indices that translates to in the NEW ordering*/
      for( j=0; j<l_dim; j++ )
	{
	  l_newIndex[ j ] = l_realIndex[ a_newIspec.order[j] ];
	}

      /*figure out what 1-d index the OLD ijk indices refer to*/
      l_iNew = 0;
      for( j=0; j<l_dim; j++ )
	{
	  l_iNew += l_newIndex[j]*l_newLoopSize[j];
	}
      if( l_iNew<0 || l_iNew>=l_numEntries )
	{
	  printinfo("change_ordering_scheme error calculating l_iNew=%d l_numEntries=%d i=%d\n",l_iNew,l_numEntries,i);
	  printinfo("l_oldIndex[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",l_oldIndex[j]);
	  printinfo("\n");

	  printinfo("l_realIndex[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",l_realIndex[j]);
	  printinfo("\n");

	  printinfo("l_newIndex[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",l_newIndex[j]);
	  printinfo("\n");

	  printinfo("l_oldLoopSize[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",l_oldLoopSize[j]);
	  printinfo("\n");

	  printinfo("l_newLoopSize[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",l_newLoopSize[j]);
	  printinfo("\n");


	  printinfo("a_oldIspec.sizes[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",a_oldIspec.sizes[j]);
	  printinfo("\n");

	  printinfo("a_newIspec.sizes[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",a_newIspec.sizes[j]);
	  printinfo("\n");


	  printinfo("a_oldIspec.order[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",a_oldIspec.order[j]);
	  printinfo("\n");

	  printinfo("a_newIspec.order[%d] = ",l_dim);
	  for( j=0; j<l_dim; j++ ) printinfo("%d ",a_newIspec.order[j]);
	  printinfo("\n");

	  exit(-1);
	}


      /*transfer the data*/
      /*a_data[l_iNew] = g_strGlobals->m_buffer[i];*/
      memcpy( (void *)(((long int)a_data) + l_iNew*a_bytesPerType), 
	      (void *)(((long int)g_strGlobals->m_buffer) + i*a_bytesPerType), 
	      (size_t)a_bytesPerType );


    }
  free(l_oldIndex);
  free(l_newIndex);
  free(l_realIndex);
  free(l_oldLoopSize);
  free(l_newLoopSize);


}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Fill in Data From a Hyperslab
 * Description:
 *
 * a_from contains a hyperslab(a_slab) of the full dataset
 * of size a_x*a_y*a_z. Put the data from a_from into the
 * correct places in a_to. Use nearest neighbor to fill in
 * the rest of a_to.
 *
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
fill_data_from_hyperslab(int a_x, int a_y, int a_z, struct_hyperslab a_slab,
			     MY_PRECISION *a_from, MY_PRECISION *a_to )
{
  int i,j,k,*l_valid,*l_iptr;
  int l_finalX = a_slab.x_start + a_slab.x_count*a_slab.x_skip;
  int l_finalY = a_slab.y_start + a_slab.y_count*a_slab.y_skip;
  int l_finalZ = a_slab.z_start + a_slab.z_count*a_slab.z_skip;
  MY_PRECISION *l_ptr;

  /*possibly adjust the limits down, in case the slab is wrong?*/
    if(l_finalX>a_x) l_finalX=a_x;
    if(l_finalY>a_y) l_finalY=a_y;
    if(l_finalZ>a_z) l_finalZ=a_z;

  l_valid = (int *)calloc((size_t)(a_x*a_y*a_z),sizeof(int));/*set all to zero*/
  l_ptr = a_from;


  /*copy from a_from to a_to*/
  for(k=a_slab.z_start; k<l_finalZ; k+= a_slab.z_skip )
    {
      for(j=a_slab.y_start; j<l_finalY; j+= a_slab.y_skip )
	{
	  for(i=a_slab.x_start; i<l_finalX; i+= a_slab.x_skip )
	    {
	      int l_index = i + j*a_x + k*a_x*a_y;
	      a_to[l_index] = l_ptr[0];
	      l_valid[l_index] = 1;
	      l_ptr++;
	    }
	}
    }

  /*
    printinfo("filling from hyperslab: %d,%d,%d   %d,%d,%d   %d,%d,%d\n",
            a_slab.x_start,a_slab.x_count,a_slab.x_skip,
	    a_slab.y_start,a_slab.y_count,a_slab.y_skip,
	    a_slab.z_start,a_slab.z_count,a_slab.z_skip );
  */


  /*fill in the empty spots in a_to*/
  {
    int l_prevGoodX=-1, l_prevGoodY=-1, l_prevGoodZ=-1;
    int l_nextGoodX=a_slab.x_start;
    int l_nextGoodY=a_slab.y_start;
    int l_nextGoodZ=a_slab.z_start;
    const int l_lastPossibleX=a_slab.x_start+(a_slab.x_count-1)*a_slab.x_skip;
    const int l_lastPossibleY=a_slab.y_start+(a_slab.y_count-1)*a_slab.y_skip;
    const int l_lastPossibleZ=a_slab.z_start+(a_slab.z_count-1)*a_slab.z_skip;
    int l_nearestXIndex=0, l_nearestYIndex=0, l_nearestZIndex=0;
    l_ptr = a_to;
    l_iptr = l_valid;




    l_prevGoodZ=-1;
    l_nextGoodZ=a_slab.z_start;
    for(k=0; k<a_z; k++ )
      {
	if( k==l_nextGoodZ && l_nextGoodZ<=l_lastPossibleZ  ) 
	  {
	    l_prevGoodZ = l_nextGoodZ;
	    l_nextGoodZ += a_slab.z_skip;
	  }    
	if( l_prevGoodZ>=0 && (l_nextGoodZ>l_lastPossibleZ || (k-l_prevGoodZ<=l_nextGoodZ-k)) )
	  {
	    l_nearestZIndex = l_prevGoodZ;
	  }
	else if( l_nextGoodZ<=l_lastPossibleZ )
	  {
	    l_nearestZIndex = l_nextGoodZ;
	  }
	else  
	  {
	    printinfo("Indexing error in fill_data_from_hyperslab\n");
	    exit(-1);
	  }




	l_prevGoodY=-1;
	l_nextGoodY=a_slab.y_start;
	for(j=0; j<a_y; j++ )
	  {
	    if( j==l_nextGoodY && l_nextGoodY<=l_lastPossibleY  ) 
	      {
		l_prevGoodY = l_nextGoodY;
		l_nextGoodY += a_slab.y_skip;
	      }
	    if( l_prevGoodY>=0 && (l_nextGoodY>l_lastPossibleY || (j-l_prevGoodY <= l_nextGoodY-j)) )
	      {
		l_nearestYIndex = l_prevGoodY;
	      }
	    else if( l_nextGoodY<=l_lastPossibleY )
	      {
		l_nearestYIndex = l_nextGoodY;
	      }
	    else 
	      {
		printinfo("Indexing error in fill_data_from_hyperslab\n");
		exit(-1);
	      }




	    l_prevGoodX=-1;
	    l_nextGoodX=a_slab.x_start;
	    for(i=0; i<a_x; i++ )
	      {
		if( i==l_nextGoodX && l_nextGoodX<=l_lastPossibleX ) 
		  {
		    l_prevGoodX = l_nextGoodX;
		    l_nextGoodX += a_slab.x_skip;
		  }
		if( l_prevGoodX>=0 && (l_nextGoodX>l_lastPossibleX || (i-l_prevGoodX <= l_nextGoodX-i)) )
		  {
		    l_nearestXIndex = l_prevGoodX;
		  }
		else if( l_nextGoodX<=l_lastPossibleX )
		  {
		    l_nearestXIndex = l_nextGoodX;
		  }
		else 
		  {
		    printinfo("Indexing error in fill_data_from_hyperslab\n");
		    exit(-1);
		  }


	      
	      if( !l_iptr[0] )
		{
		  l_ptr[0] = a_to[ l_nearestXIndex + l_nearestYIndex*a_x + l_nearestZIndex*a_x*a_y ];
		}
	      l_ptr++;
	      l_iptr++;
	    }
	}
    }
  }

  free(l_valid);
  return(0);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Structured Mesh Reader
 * Purpose:     Finalize the Structured Mesh Reader
 * Description:
 *
 * Closes the database, frees and resets all of the global variables. If the argument
 * a_finalizeSAF is non-zero, then saf_final is called as well.
 *--------------------------------------------------------------------------------------------------- */
int 
STR_CLASS_NAME_IF_CPP
finalize_str_mesh_reader( int a_finalizeSAF )
{

  printinfo("in finalize_str_mesh_reader: proc %d of %d\n",g_strGlobals->m_rank,g_strGlobals->m_nprocs);
  
  if( g_strGlobals->m_safIsInitialized && a_finalizeSAF )
    {
      printinfo("...because m_safIsInitialized, calling saf_close_database and saf_final\n");
      saf_close_database(g_strGlobals->m_db);
      saf_final();
      g_strGlobals->m_safIsInitialized=0;
      printinfo("...........finished calling saf_close_database and saf_final\n");
    }
  else if( g_strGlobals->m_safIsInitialized )
    {
      printinfo("... calling saf_close_database BUT NOT saf_final\n");
      saf_close_database(g_strGlobals->m_db);
      printinfo("...........finished calling saf_close_database\n");
    }
  
  if( g_strGlobals->m_instancePerTimestepPerVarPerBlock )
    {
      int i,j;
      for(i=0;i<g_strGlobals->m_numSets;i++)
	{
	  if( g_strGlobals->m_instancePerTimestepPerVarPerBlock[i] )
	    {
	      for(j=0;j<g_strGlobals->m_numVarsPerBlock[i]; j++)
		{
		  if( g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][j] )
		    {
		      free( g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][j] );
		      g_strGlobals->m_instancePerTimestepPerVarPerBlock[i][j] = 0;
		    }
		}
	      free( g_strGlobals->m_instancePerTimestepPerVarPerBlock[i] );
	      g_strGlobals->m_instancePerTimestepPerVarPerBlock[i] = 0;
	    }
	}
      free( g_strGlobals->m_instancePerTimestepPerVarPerBlock );
      g_strGlobals->m_instancePerTimestepPerVarPerBlock=0;
    }


  if( g_strGlobals->m_templatePerVarPerBlock )
    {
      int i;
      for(i=0;i<g_strGlobals->m_numSets;i++)
	{
	  if( g_strGlobals->m_templatePerVarPerBlock[i] )
	    {
	      free( g_strGlobals->m_templatePerVarPerBlock[i] );
	      g_strGlobals->m_templatePerVarPerBlock[i] = 0;
	    }
	}
      free( g_strGlobals->m_templatePerVarPerBlock ); 
      g_strGlobals->m_templatePerVarPerBlock=0;
    }  
  


  if( g_strGlobals->m_strMeshVarUnitName)
    {
      int i;
      for(i=0;i<g_strGlobals->m_totalNumFields;i++)
	{
	  if(g_strGlobals->m_strMeshVarUnitName[i]) free(g_strGlobals->m_strMeshVarUnitName[i]);
	  g_strGlobals->m_strMeshVarUnitName[i] = 0;
	}
      free(g_strGlobals->m_strMeshVarUnitName);
      g_strGlobals->m_strMeshVarUnitName=0;
    }
  if( g_strGlobals->m_strMeshVarQuantName)
    {
      int i;
      for(i=0;i<g_strGlobals->m_totalNumFields;i++)
	{
	  if(g_strGlobals->m_strMeshVarQuantName[i]) free(g_strGlobals->m_strMeshVarQuantName[i]);
	  g_strGlobals->m_strMeshVarQuantName[i] = 0;
	}
      free(g_strGlobals->m_strMeshVarQuantName);
      g_strGlobals->m_strMeshVarQuantName=0;
    }


  if( g_strGlobals->m_totalNumFields )
    {
      free( g_strGlobals->m_allFields );
      free( g_strGlobals->m_allFieldTemplates );
      free( g_strGlobals->m_allFieldSets );
      free( g_strGlobals->m_timestepsPerField );
      free( g_strGlobals->m_whichTopsetPerField );
      g_strGlobals->m_allFields=0;
      g_strGlobals->m_allFieldTemplates=0;
      g_strGlobals->m_allFieldSets=0;
      g_strGlobals->m_totalNumFields=0; 
      g_strGlobals->m_timestepsPerField=0;
      g_strGlobals->m_whichTopsetPerField=0;
    }
  
  if( g_strGlobals->m_timeValues )
    {
      free( g_strGlobals->m_timeValues );
      g_strGlobals->m_timeValues=0;
    }
  g_strGlobals->m_numTimesteps=0;



#ifdef USE_CACHED_SUBSET_REL_COUNTS
  if(g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset)
    {
      int k;
      for(k=0;k<g_strGlobals->m_numSets;k++)
	{
	  free(g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset[k]);
	  g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset[k]=0;
	}
      free(g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset);
      g_strGlobals->m_numSubsetRelsPerNonTopSetPerTopset=0;
    }
  if(g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset)
    {
      int k;
      for(k=0;k<g_strGlobals->m_numNonTopSets;k++)
	{
	  free(g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset[k]);
	  g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset[k]=0;
	}
      free(g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset);
      g_strGlobals->m_numSubsetRelsPerNonTopSetPerNonTopset=0;
    }
#endif  

  if(g_strGlobals->m_strVarNames)
    {
      int i;
      for(i=0;i<g_strGlobals->m_maxNumVars;i++)
	{
	  if( g_strGlobals->m_strVarNames[i] )
	    {
	      free( g_strGlobals->m_strVarNames[i] );
	      g_strGlobals->m_strVarNames[i] = 0;
	    }
	}
      free(g_strGlobals->m_strVarNames);
    }
  g_strGlobals->m_maxNumVars=0;

  if( g_strGlobals->m_numSets )
    {
      free( g_strGlobals->m_sets );
      free( g_strGlobals->m_numVarsPerBlock );

      g_strGlobals->m_sets=0;
      g_strGlobals->m_numSets=0;
    }
  if( g_strGlobals->m_numSubsets )
    {
      free( g_strGlobals->m_subsets );
      g_strGlobals->m_subsets=0;
      g_strGlobals->m_numSubsets=0;
    }
  if( g_strGlobals->m_numEdgeSubsets )
    {
      free( g_strGlobals->m_edgeSubsets );
      g_strGlobals->m_edgeSubsets=0;
      g_strGlobals->m_numEdgeSubsets=0;
    }
  if( g_strGlobals->m_numFaceSubsets )
    {
      free( g_strGlobals->m_faceSubsets );
      g_strGlobals->m_faceSubsets=0;
      g_strGlobals->m_numFaceSubsets=0;
    }
  if( g_strGlobals->m_numElemSubsets )
    {
      free( g_strGlobals->m_elemSubsets );
      g_strGlobals->m_elemSubsets=0;
      g_strGlobals->m_numElemSubsets=0;
    }

  if( g_strGlobals->m_strMeshVarType)
    {
      free(g_strGlobals->m_strMeshVarType);
      g_strGlobals->m_strMeshVarType=0;
    }

  if( g_strGlobals->m_whichVarIsThisField )
    {
      free(g_strGlobals->m_whichVarIsThisField);
      g_strGlobals->m_whichVarIsThisField=0;
    }
 
  if( g_strGlobals->m_allIndirectFields ) 
    {
      free(g_strGlobals->m_allIndirectFields);
      g_strGlobals->m_allIndirectFields=0;
    }
  if( g_strGlobals->m_timestepsPerIndirectField ) 
    {
      free(g_strGlobals->m_timestepsPerIndirectField);
      g_strGlobals->m_timestepsPerIndirectField=0;
    }
  g_strGlobals->m_totalNumIndirectFields=0;
  
  freeBuffer();

  g_strGlobals->m_numNonTopSets = 0;
  if(g_strGlobals->m_allNonTopSets)
    {
      free( g_strGlobals->m_allNonTopSets );
      g_strGlobals->m_allNonTopSets = 0;
    }

  

#ifdef __cplusplus
    m_variableNames->free_variable_name_for_field_list();
#else
    free_variable_name_for_field_list();
#endif

  if(g_strGlobals->m_printinfoFilename)
    {
      free(g_strGlobals->m_printinfoFilename);
      g_strGlobals->m_printinfoFilename=NULL;
    }


  printinfo("leaving finalize_str_mesh_reader\n\n\n");

  g_strGlobals->m_strMeshReaderGlobalsAreInitialized=0;/*note this forces printinfo to stdout from here on*/


  if( g_strGlobals->m_printinfoFp )
    {
      fclose(g_strGlobals->m_printinfoFp);
      g_strGlobals->m_printinfoFp=NULL;
    }

  return(0);
}

