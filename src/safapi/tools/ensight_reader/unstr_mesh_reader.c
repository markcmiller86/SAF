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
 * Chapter:     Unstructured Mesh Reader
 * Description:
 *
 * This file contains a set of functions that can be used to read unstructured mesh data
 * for display in a visualization tool. 
 *
 * The function init_unstr_mesh_reader must be called before any other functions in
 * this file, except for is_unstr_mesh_reader_initialized. It reads field, set and relation
 * data from the database (SAF information), and then translates that into parts and variables
 * (visualizer information). Much sorted information is stored in global arrays in this
 * file, which are accessed by most functions (note that the global arrays are member
 * variables in C++, see below).
 *
 * Note that init_str_mesh_reader (from str_mesh_reader.h) must be called before
 * init_unstr_mesh_reader, because it also initializes the timestep data.
 * 
 * Note that the filename for the saf database is specified as an argument
 * to init_str_mesh_reader (from str_mesh_reader.h)
 *
 * C vs C++:    This file (unstr_mesh_reader.c) can be compiled in C or C++. In C, the global variables
 * are all members of the single instance of the struct pointed to by g_unstrGlobals. The code is used
 * by first calling init_unstr_mesh_reader, then other functions, and finally finalize_unstr_mesh_reader.
 * Because there is only one instance of g_unstrGlobals, only one database (per process) can be opened
 * at a time (note that Ensight seems to spawn new processes for each database, so it is not
 * an issue). In C++ however, there is a class SAFUnstrMeshReader, which has one member variable: a 
 * pointer to a structure named g_unstrGlobals. Because of this, functions will access global
 * variables the same way in C and C++, e.g. g_unstrGlobals->m_variable. So in C++, the code is 
 * used by first creating a SAFUnstrMeshReader instance, then calling the method init_unstr_mesh_reader,
 * etc. Any number of SAFUnstrMeshReader objects can be created, up to any limits imposed by the SAF
 * and HDF5 libraries themselves.
 *
 * Note that in C++, you must link the unstructured mesh instance to the structured mesh instance
 * using SAFUnstrMeshReader::setStrReader.
 *
 * Note that global variables are prepended by g_, member variables by m_, local variables l_, and 
 * argument variables by a_.
 *--------------------------------------------------------------------------------------------------- */

/* HAVE_MPI_CPP should be defined by mpich's mpiCC, and LAM_IS_COMPILING_CPP by lam's mpiCC
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
#include <string.h>
#include <unstr_mesh_reader.h>
#include <variable_names.h>




#include "subset_transforms.h" 


#define USE_CONNECTIVITY_REDUCTION_AND_CACHE   /*began 26nov2003*/


#define USE_TOPO_RELS_CACHE /*a short-cut: works ok, june2003*/
#define USE_SUBSETS_OF_SET_CACHE /*a short-cut: works ok, june2003*/

#define NUM_NODES_IN_3BALL 1
#define NUM_NODES_IN_2BALL 1
#define NUM_NODES_IN_1BALL 1
/*#define NUM_NODES_IN_0BALL 1 not in sslib?*/





#define ALLOW_MULTI_FIELDS_PER_VAR_PATCH






/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Global Variables
 * Description:
 *
 * This section defines the global variable structure, instantiates the structure,
 * and defines some things that are needed for the dual C/C++ trick.
 *--------------------------------------------------------------------------------------------------- */
#ifdef __cplusplus 
class unstr_mesh_reader_t 
{
 public:
#else
  typedef struct unstr_mesh_reader_t 
  {
#endif


    int m_unstrMeshReaderIsInitialized;/*note this must be first in the structure*/

    int m_numUnstrParts;/*currently any set with unstr
			  points and unstr elements is considered a separate 'Part', or entity
			  that the visualization tool can control (e.g. turn on/off, map variables to)*/
		
    int *m_setIndexPerPart;
    char **m_partNames;

       
    int **m_setIndexWhereNodesAreStoredPerPartPerTimestep;
    int ***m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform;/*do not free the int *, it is stored elsewhere*/



    int m_maxNumUnstrVars;/*Each part may have a different number of variables. m_maxNumUnstrVars is
			    the largest number of variables that any part has.*/

    SAF_Field ***m_instancePerTimestepPerVarPerPart;/*a variable for a given part at a given time is
						      the data in a field*/

    SAF_CellType **m_celltypePerVarPerPart;

    char **m_unstrMeshVarUnitName;
    char **m_unstrMeshVarQuantName;
    char **m_unstrVarNames;

    int m_numAllSets;
    SAF_Set *m_allSets;/*this is a list of all non-suite sets (SAF_SPACE only!)*/
    SAF_Field **m_allSetsFields;/*all fields on each of the m_allSets*/
    int *m_allSetsNumFields;/*Num entries in m_allSetsFields*/

    SAF_Field **m_allSetsCoordFields;/*all coord fields on each of the m_allSets, from saf_find_coords*/
    int *m_allSetsNumCoordFields;/*Num entries in m_allSetsCoordFields*/

    int **m_allSetsFieldsTimestep;/*timestep for each field in m_allSetsFields if the field
				    can be a variable (passes can_this_field_be_a_variable test),
				    set to -2 if the field CANNOT be a variable,
				    set to -1 if the field can be a var but has no timestep*/
    int **m_allSetsFieldsVariable;/*variable index for each field in m_allSetsFields if the field
				    can be a variable (passes can_this_field_be_a_variable test),
				    set to -2 if the field CANNOT be a variable (actually any
				    number less than zero will do.
				    note: this var index will start out being valid only for this
				    particular set, so var N on set 0 will not match var N on set 1*/


#ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE

    int *m_pointsAreTimeDependentPerPart;

    int **m_numPointsUsedPerPartPerTimestep;
    MY_PRECISION ***m_pointsUsedPerPartPerTimestep;

    int **m_pointElementsPerPartPerTimestep;
    int **m_triElementsPerPartPerTimestep;
    int **m_quadElementsPerPartPerTimestep;
    int **m_tetElementsPerPartPerTimestep;
    int **m_hexElementsPerPartPerTimestep;
    int **m_pyramidElementsPerPartPerTimestep;
    int **m_prismElementsPerPartPerTimestep;
    int **m_lineElementsPerPartPerTimestep;
    /*int **m_0ballElementsPerPartPerTimestep;*/
    int **m_1ballElementsPerPartPerTimestep;
    int **m_2ballElementsPerPartPerTimestep;
    int **m_3ballElementsPerPartPerTimestep;
    int **m_quadraticTriElementsPerPartPerTimestep;
    int **m_quadraticQuadElementsPerPartPerTimestep;
    int **m_quadraticTetElementsPerPartPerTimestep;
    int **m_quadraticHexElementsPerPartPerTimestep;
    int **m_quadraticPyramidElementsPerPartPerTimestep;
    int **m_quadraticPrismElementsPerPartPerTimestep;
    int **m_quadraticLineElementsPerPartPerTimestep;

    int ***m_pointConnectivityPerPartPerTimestep;
    int ***m_triConnectivityPerPartPerTimestep;
    int ***m_quadConnectivityPerPartPerTimestep;
    int ***m_tetConnectivityPerPartPerTimestep;
    int ***m_hexConnectivityPerPartPerTimestep;
    int ***m_pyramidConnectivityPerPartPerTimestep;
    int ***m_prismConnectivityPerPartPerTimestep;
    int ***m_lineConnectivityPerPartPerTimestep;
    /*int ***m_0ballConnectivityPerPartPerTimestep;*/
    int ***m_1ballConnectivityPerPartPerTimestep;
    int ***m_2ballConnectivityPerPartPerTimestep;
    int ***m_3ballConnectivityPerPartPerTimestep;
    int ***m_quadraticTriConnectivityPerPartPerTimestep;
    int ***m_quadraticQuadConnectivityPerPartPerTimestep;
    int ***m_quadraticTetConnectivityPerPartPerTimestep;
    int ***m_quadraticHexConnectivityPerPartPerTimestep;
    int ***m_quadraticPyramidConnectivityPerPartPerTimestep;
    int ***m_quadraticPrismConnectivityPerPartPerTimestep;
    int ***m_quadraticLineConnectivityPerPartPerTimestep;


    int ***m_pointsTransformPerPartPerTimestep; /*02jan2004*/

#endif


#ifdef USE_TOPO_RELS_CACHE
    /*
    ** Note this cache works like USE_SUBSETS_OF_SET_CACHE. 
    ** I could make this one faster though: we know that for each set we will call with
    ** a predetermined list of celltypes, so we could do all the celltypes at once. This
    ** doesnt look like it would save much time, so I'm not doing it, for now.
    */
    int m_topoRelsCacheMaxSize;
    int m_topoRelsCacheSize;

    int *m_topoRelsCacheSetRows;/*one of the 3 input params*/
    SAF_CellType *m_topoRelsCacheCelltypes;/*one of the 3 input params*/
    int *m_topoRelsCacheCelltypeIsQuadratic;/*one of the 3 input params*/

    int *m_topoRelsCacheNumRels;/*one of the 3 outputs*/
    int *m_topoRelsCacheNumElems;/*one of the 3 outputs*/
    SAF_Rel **m_topoRelsCacheRels;/*one of the 3 outputs*/
#endif



    /*
    ** After noticing how much time was being lost in get_num_subsets_of_set, I
    ** created a cache to go with it. When the function is called for a given 
    ** set, subset and celltype, the cache is checked to see if this combination
    ** was requested before. If so, then the same results as before are returned
    */

#ifdef USE_SUBSETS_OF_SET_CACHE
    int m_subsetsOfSetCacheMaxSize;
    int m_subsetsOfSetCacheSize;
    int *m_subsetsOfSetCacheSubsetRows;/*one of the 3 input params*/
    int *m_subsetsOfSetCacheSetRows;/*one of the 3 input params*/
    SAF_CellType *m_subsetsOfSetCacheCelltypes;/*one of the 3 input params*/
    int *m_subsetsOfSetCacheNumRels;/*one of the 2 outputs*/
    SAF_Rel **m_subsetsOfSetCacheRels;/*one of the 2 outputs*/
#endif




    struct_subset_transform **m_subsetTransforms;
    int m_subsetTransformsNumCats;


#ifdef ALLOW_MULTI_FIELDS_PER_VAR_PATCH
    int m_multiFieldsListMaxLength;
    int m_multiFieldsListLength;
    SAF_Field *m_multiFieldsList;
#endif


    int m_useQuadraticElements;


#ifdef __cplusplus 
  };
#else
} unstr_mesh_reader_t;
#endif


#ifndef __cplusplus 
unstr_mesh_reader_t g_unstrGlobals_instance = { 0 };/*Note that the first entry must remain m_unstrMeshReaderIsInitialized,
						      otherwise there will be problems because of unpredictable values for
					     the rest of the members*/
unstr_mesh_reader_t *g_unstrGlobals = &g_unstrGlobals_instance;
#endif


#ifdef __cplusplus 
#  define UNSTR_CLASS_NAME_IF_CPP SAFUnstrMeshReader::
#else
#  define UNSTR_CLASS_NAME_IF_CPP 
#endif


#ifdef __cplusplus 
/* This is a hopefully temporary abomination. XXX */
#define printinfo if(m_strReader) m_strReader->printinfo
#endif




#ifdef __cplusplus 
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Constructor
 * Description:
 *
 * 
 *--------------------------------------------------------------------------------------------------- */
SAFUnstrMeshReader::SAFUnstrMeshReader()
{
  g_unstrGlobals = (unstr_mesh_reader_t *)malloc( sizeof(unstr_mesh_reader_t) );
  initialize_unstr_globals();
  m_variableNames = new VariableNames();
  m_strReader=NULL;
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Destructor
 * Description:
 *
 * 
 *--------------------------------------------------------------------------------------------------- */
SAFUnstrMeshReader::~SAFUnstrMeshReader()
{
  free(g_unstrGlobals);
  g_unstrGlobals=NULL;
  delete m_variableNames;
  m_variableNames=NULL;
}
#endif



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Prototypes
 * Description:
 *
 * These are prototypes that C++ wont accept.
 *
 *--------------------------------------------------------------------------------------------------- */

#ifdef __cplusplus 
extern "C" {
# endif
  const char *get_topotype_desc( SAF_RelRep a_type );/*in this file*/
# ifdef __cplusplus
}
#endif



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Initialize the Members of the Global Structure
 * Description:
 *
 * 
 *--------------------------------------------------------------------------------------------------- */
void 
UNSTR_CLASS_NAME_IF_CPP
initialize_unstr_globals()
{

  g_unstrGlobals->m_numUnstrParts=0;		       

  g_unstrGlobals->m_setIndexPerPart=0;
  g_unstrGlobals->m_partNames=NULL;
  g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep=0;
  g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform=0;
  g_unstrGlobals->m_maxNumUnstrVars=0;

  g_unstrGlobals->m_instancePerTimestepPerVarPerPart=0;
  g_unstrGlobals->m_unstrMeshVarUnitName=0;
  g_unstrGlobals->m_unstrMeshVarQuantName=0;

  g_unstrGlobals->m_celltypePerVarPerPart=NULL;


  g_unstrGlobals->m_unstrMeshReaderIsInitialized=0;
  g_unstrGlobals->m_unstrVarNames=0;

  g_unstrGlobals->m_numAllSets=0;
  g_unstrGlobals->m_allSets=0;
  g_unstrGlobals->m_allSetsFields=0;
  g_unstrGlobals->m_allSetsNumFields=0;

  g_unstrGlobals->m_allSetsCoordFields=0;
  g_unstrGlobals->m_allSetsNumCoordFields=0;
  g_unstrGlobals->m_allSetsFieldsTimestep=0;
  g_unstrGlobals->m_allSetsFieldsVariable=0;



#ifdef USE_TOPO_RELS_CACHE
  g_unstrGlobals->m_topoRelsCacheMaxSize=0;
  g_unstrGlobals->m_topoRelsCacheSize=0;

  g_unstrGlobals->m_topoRelsCacheSetRows=0;
  g_unstrGlobals->m_topoRelsCacheCelltypes=0;
  g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic=0;

  g_unstrGlobals->m_topoRelsCacheNumRels=0;
  g_unstrGlobals->m_topoRelsCacheNumElems=0;
  g_unstrGlobals->m_topoRelsCacheRels=0;
#endif




#ifdef USE_SUBSETS_OF_SET_CACHE
  g_unstrGlobals->m_subsetsOfSetCacheMaxSize=0;
  g_unstrGlobals->m_subsetsOfSetCacheSize=0;
  g_unstrGlobals->m_subsetsOfSetCacheSubsetRows=0;
  g_unstrGlobals->m_subsetsOfSetCacheSetRows=0;
  g_unstrGlobals->m_subsetsOfSetCacheCelltypes=0;
  g_unstrGlobals->m_subsetsOfSetCacheNumRels=0;
  g_unstrGlobals->m_subsetsOfSetCacheRels=0;
#endif



#ifdef ALLOW_MULTI_FIELDS_PER_VAR_PATCH
    g_unstrGlobals->m_multiFieldsListMaxLength=0;
    g_unstrGlobals->m_multiFieldsListLength=0;
    g_unstrGlobals->m_multiFieldsList=NULL;
#endif

#ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE

    g_unstrGlobals->m_pointsAreTimeDependentPerPart=NULL;

    g_unstrGlobals->m_numPointsUsedPerPartPerTimestep=NULL;
    g_unstrGlobals->m_pointsUsedPerPartPerTimestep=NULL;

    g_unstrGlobals->m_pointElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_triElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_tetElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_hexElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_pyramidElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_prismElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_lineElementsPerPartPerTimestep=NULL;
    /*g_unstrGlobals->m_0ballElementsPerPartPerTimestep=NULL;*/
    g_unstrGlobals->m_1ballElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_2ballElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_3ballElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticTriElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticQuadElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticTetElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticHexElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticPyramidElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticPrismElementsPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticLineElementsPerPartPerTimestep=NULL;

    g_unstrGlobals->m_pointConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_triConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_tetConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_hexConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_pyramidConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_prismConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_lineConnectivityPerPartPerTimestep=NULL;
    /*g_unstrGlobals->m_0ballConnectivityPerPartPerTimestep=NULL;*/
    g_unstrGlobals->m_1ballConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_2ballConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_3ballConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticTriConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticQuadConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticTetConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticHexConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticPyramidConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticPrismConnectivityPerPartPerTimestep=NULL;
    g_unstrGlobals->m_quadraticLineConnectivityPerPartPerTimestep=NULL;


    g_unstrGlobals->m_pointsTransformPerPartPerTimestep=NULL; 

#endif



    g_unstrGlobals->m_useQuadraticElements=0;/*this should be 0 by default. Only paraview can handle?*/
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Initialize the Reader
 *
 * Description: 
 * This is the second function that should be called to use this reader. The first is
 * init_str_mesh_reader, which reads the time data. This function reads and sorts information
 * regarding the unstructured parts and variables, and stores it in global arrays which are
 * required by the rest of the functions in the reader.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
init_unstr_mesh_reader( int a_allowQuadraticElements )
{
  int i,j,k;

  printinfo("\n*******BEGIN   init_unstr_mesh_reader***************************\n");

  if( !is_str_mesh_reader_initialized() )
    {
      printinfo("init_unstr_mesh_reader error: init_str_mesh_reader not called yet\n");
      return(-1);
    }
  if(!get_num_timesteps())
    {
      printinfo("init_unstr_mesh_reader: get_num_timesteps()=0, exiting\n");
      return(0);
    }


  if( g_unstrGlobals->m_unstrMeshReaderIsInitialized )
    {
      printinfo("error: unstructured mesh reader is already initialized\n");
      return(-1);
    }
  initialize_unstr_globals();

  g_unstrGlobals->m_useQuadraticElements= a_allowQuadraticElements;


  /*
  ** Figure out how many 'unstructured' sets there are in the database 
  ** and then determine which of them should be considered
  ** 'parts' for the visualization tool (e.g. Exodus' element blocks). Put the
  ** list of parts in m_setIndexPerPart. Also allocate and fill
  ** m_setIndexWhereNodesAreStoredPerPartPerTimestep to keep track of sets that
  ** have their nodes stored indirectly on another set.
  ** 
  */
  {
    if( g_unstrGlobals->m_numUnstrParts )
      {
	printinfo("error init_unstr_mesh_reader: m_numUnstrParts!=0, a database is already open\n");
	return(-1);
      }


    get_all_unstr_sets( );

    if( !g_unstrGlobals->m_numUnstrParts )
      {
	printinfo("Found no unstr sets. Leaving init_unstr_mesh_reader\n");
	return(0);
      }

    /*allocate (to maximum) and initialize the array that maps ensight 'parts' to saf 'sets'*/
    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform 
      = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
    if( !g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep ||
	!g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform )
      {
	printinfo("error allocating m_setIndexWhereNodesAreStoredPerPartPerTimestep\n");
	exit(-1);
      }  
    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
      {
	g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
	g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
	if(!g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i] ||
	   !g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform[i])
	  {
	    printinfo("error allocating m_setIndexWhereNodesAreStoredPerPartPerTimestep[%d]\n",i,i);
	    exit(-1);
	  }  
	for(j=0;j<get_num_timesteps();j++) 
	  {
	    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] = -1;
	    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform[i][j] = NULL;
	  }
      }

   

    /*Fill the array that maps ensight 'parts' to saf 'sets'. For now,
      it is simply one part per set. This section fills in the array m_setIndexWhereNodesAreStoredPerPartPerTimestep*/
    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
      {
	int l_found=0;
	int l_setIndex = g_unstrGlobals->m_setIndexPerPart[i];

	for(j=0;j<get_num_timesteps();j++) 
	  { /*check for 3d coordinates, stored on the set, at each time*/
	    if(does_this_set_have_unstr_point_coords_for_this_time(g_unstrGlobals->m_allSets[l_setIndex],j,3))
	      {
		l_found=1;
		g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] = l_setIndex;
	      }
	  } 
	if(!l_found)
	  {
	    /*check for 3d coordinates, stored on the set, with NO time (i.e. static)*/
	    if(does_this_set_have_unstr_point_coords_for_this_time(g_unstrGlobals->m_allSets[l_setIndex],-1,3))
	      {
		l_found=1;
		for(j=0;j<get_num_timesteps();j++) 
		  {
		    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] = l_setIndex;
		  }
	      }
	  }
	if(!l_found)
	  { /*check for 2d coordinates (stored on the set, at each time) since we found no 3d coords at all*/
	    for(j=0;j<get_num_timesteps();j++) 
	      { 
		if( does_this_set_have_unstr_point_coords_for_this_time(g_unstrGlobals->m_allSets[l_setIndex],j,2))
		  {
		    l_found=1;
		    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] = l_setIndex;
		  }
	      }
	  } 
	if(!l_found)
	  {
	    /*check for 2d coordinates, stored on the set, with NO time (i.e. static)*/
	    if(does_this_set_have_unstr_point_coords_for_this_time(g_unstrGlobals->m_allSets[l_setIndex],-1,2))
	      {
		l_found=1;
		for(j=0;j<get_num_timesteps();j++) 
		  {
		    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] = l_setIndex;
		  }
	      }
	  }
	if(!l_found)
	  {/*there are no 2d or 3d coords on this set at any time: check if they are on another set, with time (via topo rel)*/
	    int *l_xform=NULL;
	    for(j=0;j<get_num_timesteps();j++) 
	      {
		int l_index = on_which_other_set_index_are_this_sets_nodes_stored(g_unstrGlobals->m_allSets[l_setIndex],j,&l_xform);
		if( l_index>=0 )
		  {
		    l_found = 1;
		    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] = l_index;
		    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform[i][j] = l_xform;
		  }
	      }
	  }
	if(!l_found)
	  {/*this is the last possibility: that the nodes are stored on another set, and with no time (via topo rel)*/
	    int l_index;
	    int *l_xform=NULL;
	    l_index = on_which_other_set_index_are_this_sets_nodes_stored(g_unstrGlobals->m_allSets[l_setIndex],-1,&l_xform);

	    if( l_index>=0 )
	      {
		l_found=1;
		for(j=0;j<get_num_timesteps();j++) 
		  {
		    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] = l_index;
		    g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform[i][j] = l_xform;
		  }
	      }
	  }

      }
  }



  /*
  ** Print a different kind of truth table.
  */
  printinfo("\nnew truth table: time (horiz) vs part: DOES THE PART SET HAVE ITS OWN COORDS?\n");
  for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
    {
      for(j=0;j<get_num_timesteps();j++)
	{
	  if( g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] < 0 ) printinfo("-");/*- is a bad thing!!*/
	  else if( g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][j] 
		   == g_unstrGlobals->m_setIndexPerPart[i] ) printinfo("1");
	  else printinfo("0");
	}
      {
	char l_str[256];
	get_unstr_part_name( i, l_str, 255 );
	printinfo("  %s",l_str);
      }
      printinfo("\n");
    }





  printinfo("\n");/*extra space after last truth table*/


  /*
  ** Check if this is a trivial case, and if so, leave early.
  */
  if( !get_num_timesteps() || !g_unstrGlobals->m_numUnstrParts )
    {
      printinfo("*******END     init_unstr_mesh_reader***************************\n\n");
      g_unstrGlobals->m_unstrMeshReaderIsInitialized=1;
      return(0);
    }




  /*Go through all fields (m_allSetsFields) and assign each one a variable index
    (m_allSetsFieldsVariable). For each new field, a search is made through all
    previous fields until one is found that should be from the same variable. If
    no such field is found, then the current var index (l_currentVar, later to 
    become m_maxNumUnstrVars) is incremented, and that field is considered to 
    belong to a new variable.*/
  {
    int l_currentVar=0;
    for(i=0;i<g_unstrGlobals->m_numAllSets;i++) 
      {
	for(j=0;j<g_unstrGlobals->m_allSetsNumFields[i];j++) 
	  {

	    /*if this field is not on one of our known categories, we dont want it*/
	    {
	      int l_found=0;
	      char *l_name=NULL;
	      SAF_Cat l_cat;	   
	      saf_describe_field(SAF_ALL, &(g_unstrGlobals->m_allSetsFields[i][j]), NULL, &l_name, NULL, NULL,
				 NULL, NULL, &l_cat, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );

	      for(k=0;k<g_unstrGlobals->m_subsetTransformsNumCats;k++)
		{
		  if( SAF_EQUIV(&l_cat,&(g_unstrGlobals->m_subsetTransforms[k][i].m_cat)) )
		    {
		      l_found=1;
		      break;
		    }
		}
	      if(!l_found)
		{

#if 0 /*print out why this field is not a variable*/
		  hid_t l_dsltype;
		  size_t l_count=0;
		  char *l_catName=NULL;
		  saf_describe_category(&l_cat,&l_catName,NULL,NULL);
		  
		  saf_get_count_and_type_for_field(SAF_ALL,&(g_unstrGlobals->m_allSetsFields[i][j]),NULL,&l_count,&l_dsltype);

		  printinfo("warning: found field(\"%s\" count=%d type=%s) with unwanted category(%s), not a variable\n",
			    l_name,l_count,get_dsl_type_string(l_dsltype),l_catName);
		  
		  if(l_catName) free(l_catName);
#endif

		  if(l_name) free(l_name);
		  continue;
		}
	      if(l_name) free(l_name);
	    }


	    
#define ODD_SIERRA_NO_TIMES_PATCH /*This little patch handles the case when there are no timesteps (i.e. get_num_timesteps is 1)
				    and we get field whose timestep is -1. Might as well make them time 0, since that is 
				    the only step there is.*/

	    /*Is it a field with a timestep, or a field with no timestep AND there are no timesteps*/
	    if(g_unstrGlobals->m_allSetsFieldsTimestep[i][j]>=0 
#ifdef ODD_SIERRA_NO_TIMES_PATCH
	       || (g_unstrGlobals->m_allSetsFieldsTimestep[i][j]==-1 && get_num_timesteps()==1)
#endif
	       )
	      {
		int l_match=-1;

#ifdef ODD_SIERRA_NO_TIMES_PATCH
		if(g_unstrGlobals->m_allSetsFieldsTimestep[i][j]==-1) g_unstrGlobals->m_allSetsFieldsTimestep[i][j]=0;
#endif


		/*Look in this set for another field that appears to be from the same variable.*/
		for(k=0;k<j;k++)
		  {
		    /*check that it is a var with a timestep (and a different timestep from ours!)*/
		    if(g_unstrGlobals->m_allSetsFieldsTimestep[i][k]>=0 
		       && g_unstrGlobals->m_allSetsFieldsTimestep[i][j]!=g_unstrGlobals->m_allSetsFieldsTimestep[i][k] )
		      {
			if(are_these_two_fields_from_the_same_var(&(g_unstrGlobals->m_allSetsFields[i][j]),
								  &(g_unstrGlobals->m_allSetsFields[i][k])))
			  {
			    l_match = g_unstrGlobals->m_allSetsFieldsVariable[i][k];
			    break;
			  }
		      }
		  }

		if(l_match<0)
		  {
		    /*We didnt find, in this set, another field that appears to be from the same variable.
		      Look in all of the previous sets*/
		    int ii,jj;
		    for(ii=0;ii<i;ii++) 
		      {
			for(jj=0;jj<g_unstrGlobals->m_allSetsNumFields[ii];jj++) 
			  {
			    if(g_unstrGlobals->m_allSetsFieldsTimestep[ii][jj]>=0)/*i.e. it is a var with a timestep*/
			      {
				if(are_these_two_fields_from_the_same_var(&(g_unstrGlobals->m_allSetsFields[i][j]),
									  &(g_unstrGlobals->m_allSetsFields[ii][jj])))
				  {
				    l_match = g_unstrGlobals->m_allSetsFieldsVariable[ii][jj];
				    break;
				  }
			      }
			  }
			if(l_match>=0) break;
		      }
		  }
		if(l_match>=0)
		  {
		    g_unstrGlobals->m_allSetsFieldsVariable[i][j] = l_match;
		  }
		else
		  {
		    g_unstrGlobals->m_allSetsFieldsVariable[i][j] = l_currentVar++;
		  }		  
	      }
	    else if(g_unstrGlobals->m_allSetsFieldsTimestep[i][j]==-1)/*i.e. it is a var with NO timestep*/
	      {
		char *l_name=0;
		saf_describe_field(SAF_ALL, &(g_unstrGlobals->m_allSetsFields[i][j]), NULL, &l_name, 
				   NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

		{
		  static int l_alreadyprinted=0;
		  if(!l_alreadyprinted)
		    {
		      printinfo("warning: found a field(\"%s\") with no timestep, not calling it a variable\n",
				l_name);
		      printinfo("       (will not print any more warnings of this type)\n");
		      l_alreadyprinted=1;
		    }
		  g_unstrGlobals->m_allSetsFieldsVariable[i][j] = -1;
		}

		if(l_name) free(l_name);
	      }
	  }

	/*{
	  char *l_setName=NULL;
	  saf_describe_set(SAF_ALL, &(g_unstrGlobals->m_allSets[i]), &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
	  printinfo("   set %d of %d(%s) has %d cumulative vars out of %d fields\n",
	  i,g_unstrGlobals->m_numAllSets,l_setName,l_currentVar,g_unstrGlobals->m_allSetsNumFields[i]);
	  free(l_setName);
	  }*/

      }

    g_unstrGlobals->m_maxNumUnstrVars=l_currentVar;

    printinfo("After assigning a variable index to every field, setting g_maxNumUnstrVars to %d\n",g_unstrGlobals->m_maxNumUnstrVars);



  }


  /*
  ** Allocate and initialize m_instancePerTimestepPerVarPerPart[][][]
  */
  printinfo("Begin alloc of m_instancePerTimestepPerVarPerPart[%d][%d][%d], need about %d bytes.\n",
	    g_unstrGlobals->m_numUnstrParts,g_unstrGlobals->m_maxNumUnstrVars,get_num_timesteps(),
	    g_unstrGlobals->m_maxNumUnstrVars*g_unstrGlobals->m_numUnstrParts*get_num_timesteps()*sizeof(SAF_Field) );

#if 0
  if( g_unstrGlobals->m_maxNumUnstrVars*g_unstrGlobals->m_numUnstrParts*get_num_timesteps()*sizeof(SAF_Field) > 500000000 )
    {
      printinfo("error surely too much memory\n");
      exit(-1);
    }
#endif

  if(get_num_timesteps())
    {
      g_unstrGlobals->m_instancePerTimestepPerVarPerPart=(SAF_Field***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(SAF_Field**));
      for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
	{

	  g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i]=(SAF_Field**)malloc(g_unstrGlobals->m_maxNumUnstrVars*sizeof(SAF_Field*));
	  if(!g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i])
	    {
	      printinfo("error: cannot alloc m_instancePerTimestepPerVarPerPart[%d] to size %d\n",
			i,g_unstrGlobals->m_maxNumUnstrVars*sizeof(SAF_Field *));
	      exit(-1);
	    }
	  for(j=0;j<g_unstrGlobals->m_maxNumUnstrVars;j++)
	    {
	      g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j]=(SAF_Field*)malloc(get_num_timesteps()*sizeof(SAF_Field));
	      if(!g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j])
		{
		  printinfo("error: cannot alloc m_instancePerTimestepPerVarPerPart[%d][%d] to size %d\n",
			    i,j,get_num_timesteps()*sizeof(SAF_Field));
		  exit(-1);
		}
	      for(k=0;k<get_num_timesteps();k++)
		{
		  g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k]=SS_FIELD_NULL;
		}
	    }
	}
    }
  else
    {
      /*There are no timesteps: this should never happen.*/
      g_unstrGlobals->m_instancePerTimestepPerVarPerPart=NULL;
    }



  /*
  ** Populate m_instancePerTimestepPerVarPerPart for the worst case, i.e. if each
  ** field on each part was its own variable, unrelated to any others. The 'instance'
  ** is the SAF_Field that contains the data for that part-variable-timestep combo.
  ** Note this will only handle fields that have an associated timestep.
  */
  printinfo("Begin initial population of m_instancePerTimestepPerVarPerPart:\n");
  if( g_unstrGlobals->m_maxNumUnstrVars )
    {
      /*In a database with no fields on subsets of parts (like all sierra data),
	m_maxNumUnstrVars would be the limit, because there would never be >1
	field for a given part-var-timestep combo. However, since it could happen
	(see create_unstr_mesh.c), we will increase the size to make room. If
	that is not enough.......XXX for now, just fail quietly.*/

#if 0 /*Would set to 1 to handle subset vars quietly, with 1 field per var allowed. Note
	that if there are >1 field for any var, it still may fail later if we end up discarding
	some other field, and keeping >1 field for a particular var. THIS SHOULD STAY AT 1 FOR NOW*/
 int l_numAllocated = g_unstrGlobals->m_maxNumUnstrVars;
#else
 int l_numAllocated = g_unstrGlobals->m_maxNumUnstrVars<10  ? 20 : g_unstrGlobals->m_maxNumUnstrVars*2;
#endif

 SAF_Field *l_fields = (SAF_Field *)malloc(l_numAllocated*sizeof(SAF_Field));
 if(!l_fields)
   {
     printinfo("error couldnt alloc l_fields to size %d bytes\n",l_numAllocated*sizeof(SAF_Field));
     exit(-1);
   }

 for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
   {
     for(k=0;k<get_num_timesteps();k++)
       {
	 int l_numFound=0;

	 for(j=0;j<l_numAllocated;j++) l_fields[j]=SS_FIELD_NULL;

	 /*note this was where p1.saf was failing.....and dtest? was a mem leak in DSL_match_pattern_against_data*/
	 l_numFound = get_all_unstr_fields_for_set_and_cat( g_unstrGlobals->m_setIndexPerPart[i], SAF_ANY_CAT, k, 
							    l_numAllocated, l_fields );


#if 0 /*print some  info*/
	 if(l_numFound)
	 {
	   char *l_setName=NULL;
	   SAF_Set l_thisset;
	   l_thisset = g_unstrGlobals->m_allSets[g_unstrGlobals->m_setIndexPerPart[i]];
	   saf_describe_set(SAF_ALL, &l_thisset, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
	   printinfo("\nget_all_unstr_fields_for_set_and_cat for set %s (part=%d time=%d)\n",l_setName,i,k);
	   for(j=0;j<l_numFound;j++)
	     {
	       int l_count=0;
	       char *l_catName=NULL,*l_fieldName=NULL;
	       SAF_Cat l_cat;
	       SAF_CellType l_cellType;
	       saf_describe_field(SAF_ALL, &(l_fields[j]), NULL, &l_fieldName, NULL, NULL,
				  NULL, NULL, &l_cat, NULL, NULL, NULL, NULL,  
				  NULL, NULL, NULL, NULL, NULL);
	       saf_describe_category(SAF_ALL,&l_cat,&l_catName,NULL,NULL);
	       saf_describe_collection(SAF_ALL,&l_thisset,&l_cat,&l_cellType,&l_count,NULL,NULL,NULL);
	       printinfo("      %d: field=%s  cat=%s  collcount=%d  celltype=%s\n",
			 j,l_fieldName,l_catName,l_count,get_celltype_desc(l_cellType));
	       free(l_catName);
	       free(l_fieldName);
	     }
	   free(l_setName);
	 }
#endif




	 if( l_numFound > l_numAllocated )
	   {
	     /*This should never happen unless l_numAllocated>m_maxNumUnstrVars: 
	       otherwise the error would be handled quietly in get_all_unstr_fields_for_set_and_cat*/
	     printinfo("error found more vars than allocated l_numFound=%d l_numAllocated=%d m_maxNumUnstrVars=%d\n",
		       l_numFound,l_numAllocated,g_unstrGlobals->m_maxNumUnstrVars);
	     printinfo("\nexiting in init_unstr_mesh_reader\n\n");
	     exit(-1);
	   }

	 if( l_numFound > g_unstrGlobals->m_maxNumUnstrVars )
	   {
	     /*This should never happen unless l_numAllocated>m_maxNumUnstrVars: 
	       otherwise the error would be handled quietly in get_all_unstr_fields_for_set_and_cat*/
	     printinfo("warning found more vars than expected l_numFound=%d m_maxNumUnstrVars=%d\n",
		       l_numFound,g_unstrGlobals->m_maxNumUnstrVars);
	   }


	 /*For each entry in l_fields:
	   figure out what set it is on,
	   find which slot in m_allSetsFields it is in,
	   read that slot in m_allSetsFieldsVariable[][] to get the appropriate variable*/
	 for(j=0;j<l_numFound;j++)
	   {
	     SAF_Set l_set;
	     SAF_Field *l_fieldsForSet;
	     int *l_times,*l_vars,kk,l_num,l_index=-1;

	     saf_describe_field(SAF_ALL, &(l_fields[j]),NULL,NULL, &l_set, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	     l_num = get_all_fields_for_set( l_set, &l_fieldsForSet, &l_times, &l_vars, NULL, NULL );
	     for(kk=0;kk<l_num;kk++) /*XXX make faster: could sort */
	       {
		 if( SAF_EQUIV(&(l_fieldsForSet[kk]),&(l_fields[j])) )
		   {
		     l_index = kk;
		     break;
		   }
	       }

	     if(l_index>=0)
	       {
		 if( my_saf_is_valid_field_handle(&(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][l_vars[l_index]][l_times[l_index]])))
		   {
#ifndef ALLOW_MULTI_FIELDS_PER_VAR_PATCH
		     printinfo("warning >1 fields for part %d var %d time %d (l_index=%d)\n",i,
			       l_vars[l_index],l_times[l_index],l_index);

#else/* ALLOW_MULTI_FIELDS_PER_VAR_PATCH*/
		     {
		       SAF_Field l_firstField =
			 g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][l_vars[l_index]][l_times[l_index]];
		       SAF_Field l_thisField = l_fields[j];
		       int l_slot=-1;

		       /*Resize the cache if needed. the +3 is in case we have to add 2 fields and a spacer to the list.*/
		       if( g_unstrGlobals->m_multiFieldsListLength+2 >= g_unstrGlobals->m_multiFieldsListMaxLength )
			 {
			   SAF_Field *l_newArray;
			   int l_newMaxLen = g_unstrGlobals->m_multiFieldsListMaxLength 
			     ? 2*g_unstrGlobals->m_multiFieldsListMaxLength : 20;
			   l_newArray = (SAF_Field *)malloc(l_newMaxLen*sizeof(SAF_Field));
			   if( g_unstrGlobals->m_multiFieldsList )
			     {
			       memcpy(l_newArray,g_unstrGlobals->m_multiFieldsList,
				      g_unstrGlobals->m_multiFieldsListLength*sizeof(SAF_Field));
			       free(g_unstrGlobals->m_multiFieldsList);
			     }
			   g_unstrGlobals->m_multiFieldsList = l_newArray;
			   g_unstrGlobals->m_multiFieldsListMaxLength = l_newMaxLen;
			   for(kk=g_unstrGlobals->m_multiFieldsListLength;kk<g_unstrGlobals->m_multiFieldsListMaxLength;kk++)
			     {
			       g_unstrGlobals->m_multiFieldsList[kk]=SS_FIELD_NULL;
			     }
			 }

		       /*find the correct place in the cache*/
		       for(kk=0;kk<g_unstrGlobals->m_multiFieldsListLength;kk++)
			 {
			   if(SAF_EQUIV(&(g_unstrGlobals->m_multiFieldsList[kk]),&l_firstField))
			     {
			       l_slot=kk;
			       do {
				 l_slot++;
			       } while( my_saf_is_valid_field_handle(&(g_unstrGlobals->m_multiFieldsList[l_slot])) );
			       break;
			     }			  
			 }

		       /*add the field to the cache*/
		       if(l_slot<0)
			 {
			   /*we must add both fields to the list*/
			   g_unstrGlobals->m_multiFieldsList[ g_unstrGlobals->m_multiFieldsListLength++ ] = l_firstField;
			   g_unstrGlobals->m_multiFieldsList[ g_unstrGlobals->m_multiFieldsListLength++ ] = l_thisField;
			   g_unstrGlobals->m_multiFieldsList[ g_unstrGlobals->m_multiFieldsListLength++ ] = SS_FIELD_NULL;
			 }
		       else if(l_slot==g_unstrGlobals->m_multiFieldsListLength-1)
			 {
			   /*this is the last entry in the list*/
			   g_unstrGlobals->m_multiFieldsList[ g_unstrGlobals->m_multiFieldsListLength-1 ] = l_thisField;
			   g_unstrGlobals->m_multiFieldsList[ g_unstrGlobals->m_multiFieldsListLength++ ] = SS_FIELD_NULL;
			 }
		       else
			 {
			   /*this new entry is in the middle of the list, so we need to shuffle the last part away*/
			   for(kk=g_unstrGlobals->m_multiFieldsListLength; kk>l_slot; kk--)
			     {
			       g_unstrGlobals->m_multiFieldsList[kk] = g_unstrGlobals->m_multiFieldsList[kk-1];
			     }
			   g_unstrGlobals->m_multiFieldsList[l_slot]=l_thisField;
			   g_unstrGlobals->m_multiFieldsList[ g_unstrGlobals->m_multiFieldsListLength++ ] = SS_FIELD_NULL;
			 }

		       printinfo("ALLOW_MULTI_FIELDS_PER_VAR_PATCH: \n");
		       for(kk=0;kk<g_unstrGlobals->m_multiFieldsListMaxLength;kk++)
			 {
			   if( kk==g_unstrGlobals->m_multiFieldsListLength ) printinfo(" ** ");
			   printinfo("? ");/*sslib? has no 'theRow'*/
			 }
		       printinfo("\n");

		     }
#endif /*ALLOW_MULTI_FIELDS_PER_VAR_PATCH*/

		   }
		 else
		   {
		     g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][l_vars[l_index]][l_times[l_index]] = l_fields[j];
		   }
	       }
	     else
	       {
		 printinfo("error couldnt find matching field in m_allSetsField\n");
		 exit(-1);
	       }
	   }
       }
   }
 free(l_fields);
    }
  printinfo("Finished initial population of m_instancePerTimestepPerVarPerPart.\n");


  /*XXX todo 19nov2003 
    Need to handle the case when there are fields like: stressA stressB, with valid units and quants,
    that get linked like:
    var 0 = stressA stressA stressB stressA
    var 0 = stressB stressA stressB stressB
    Note that this wont happen as long as sierra doesnt write units correctly.
  */


#if 0 /************************************************************************/
  /* THIS IS NO LONGER NECESSARY, BUT KEEPING IT FOR THE OCCASIONAL DEBUG TEST.
  ** Reshuffle m_instancePerTimestepPerVarPerPart, by going through it and consolidating
  ** variables.
  */
  printinfo("Begin reshuffling of m_instancePerTimestepPerVarPerPart:\n");
  {
    int l_numMoved=0;
    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
      {
	for(j=0;j<g_unstrGlobals->m_maxNumUnstrVars;j++)
	  {
	    for(k=0;k<get_num_timesteps();k++)
	      {
		SAF_Field l_fieldA = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k];
		if( my_saf_is_valid_field_handle(&l_fieldA))
		  {
		    int ii,jj;
		    int l_found=0;
		    for(ii=0;ii<=i;ii++) /*all parts up to and including this one*/
		      {

			for(jj=0;jj<g_unstrGlobals->m_maxNumUnstrVars;jj++)
			  {
			    int l_time=0;
			    SAF_Field l_fieldB = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[ii][jj][l_time];

			    if( ii==i && jj==j ) break;/*loop2 caught up to loop1*/
			    if( jj==j ) continue;/*skip the 'this var = that var' case*/

			    if( my_saf_is_valid_field_handle(&l_fieldB) )
			      {
				SAF_Field l_fieldInTargetSpot = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][jj][k];

				/*note: in addition to checking if the target spot is empty,
				  we check if it is the same field. This handles the larry1w case
				  where the user repeated the field throughout the suite timesteps,
				  to show a constant field*/
				if( SAF_EQUIV(&l_fieldInTargetSpot,&l_fieldA) ||
				    !my_saf_is_valid_field_handle(&l_fieldInTargetSpot) &&
				     are_these_two_fields_from_the_same_var(&l_fieldA,&l_fieldB)) )
				  {
				    g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][jj][k] = l_fieldA;
				    g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k] = SS_FIELD_NULL;
					
				    l_numMoved++;
				    l_found=1;
				    break;
				  }
			      }
			  }
			if(l_found) break;
		      }
		  }
	      }
	  }
      }
    printinfo("Finished reshuffling of m_instancePerTimestepPerVarPerPart, number of fields moved=%d.\n",l_numMoved);
    if(l_numMoved) printinfo("warning: should not have moved any fields!\n\n");/*since it is no longer needed*/
  }
#endif /************************************************************************/


#if 1 /************************************************************************/
  /*  THIS IS STILL NECESSARY(e.g. for reading test_str_mesh.saf)
  ** Now that m_instancePerTimestepPerVarPerPart has been reshuffled, count the new
  ** number of variables, m_maxNumUnstrVars. If there are entire blank rows (variable
  ** slots) then shift back all the later variables.
  */
  {
    int l_numUnusedVarSlots=0;
    for(j=0;j<g_unstrGlobals->m_maxNumUnstrVars;j++)
      {
	int l_found=0;
	for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
	  {

	    for(k=0;k<get_num_timesteps();k++)
	      {
		if( my_saf_is_valid_field_handle(&(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k])) ) 
		  {
		    l_found=1;

		    /*shift it back, if called for*/
		    if( l_numUnusedVarSlots )
		      {
			if( my_saf_is_valid_field_handle(&(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j-l_numUnusedVarSlots][k])) )
			  {
			    printinfo("  error: Removing unused variable slots, slot not empty: part=%d slot=%d time=%d\n",
				      i,j-l_numUnusedVarSlots,k );
			  }
			else
			  {
			    g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j-l_numUnusedVarSlots][k]
			      = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k];
			    g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k] = SS_FIELD_NULL;
			    /*printinfo("   moving field from %d %d %d  back to  %d %d %d\n", i,j,k,i,
			      j-l_numUnusedVarSlots,k);*/
			  }
		      }
		  }
	      }
	  }
	if(!l_found)
	  {
	    printinfo("warning: found unused variable slot #%d\n",j);
	    l_numUnusedVarSlots++;
	  }
      }

    if(l_numUnusedVarSlots)
      {
	int l_oldNum = g_unstrGlobals->m_maxNumUnstrVars;
	g_unstrGlobals->m_maxNumUnstrVars -= l_numUnusedVarSlots;
	for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
	  {
	    for(j=g_unstrGlobals->m_maxNumUnstrVars;j<l_oldNum;j++)
	      {
		if( g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j] )
		  {
		    free( g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j] );
		    g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j]=0;
		  }
	      }
	  }
      }

    printinfo("Finished removing %d unused variable slots, max num variables now = %d\n",
	      l_numUnusedVarSlots,g_unstrGlobals->m_maxNumUnstrVars);
  }
#endif /************************************************************************/


  /*debugging test only*/
  /*
    printinfo("Begining test that all fields in a var seem to be from same var.\n");
    for(j=0;j<g_unstrGlobals->m_maxNumUnstrVars;j++)
    {
    SAF_Field l_firstField;
    int l_gotFirst=0;
    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
    {
    for(k=0;k<get_num_timesteps();k++)
    {
    SAF_Field l_thisField = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k];
    if( my_saf_is_valid_field_handle(&l_thisField) )
    {
    if(!l_gotFirst)
    {
    l_gotFirst=1;
    l_firstField = l_thisField;
    }
    else
    {
    if( !are_these_two_fields_from_the_same_var(&l_thisField,&l_firstField) )
    {
    printinfo("   error: field %d %d %d is wrong var\n",i,j,k);
    exit(-1);
    }
    }
    }
    }
    }
    }
    printinfo("Finished test that all fields in a var seem to be from same var.\n");
  */
      

  /*
  ** Get the unit and quantity names for the variables.
  */
  g_unstrGlobals->m_unstrMeshVarUnitName = (char **)malloc(g_unstrGlobals->m_maxNumUnstrVars*sizeof(char *));
  g_unstrGlobals->m_unstrMeshVarQuantName = (char **)malloc(g_unstrGlobals->m_maxNumUnstrVars*sizeof(char *));

  for(i=0;i<g_unstrGlobals->m_maxNumUnstrVars;i++)
    {
      int l_done=0;
      g_unstrGlobals->m_unstrMeshVarUnitName[i]=NULL;
      g_unstrGlobals->m_unstrMeshVarQuantName[i]=NULL;
      for(j=0;j<g_unstrGlobals->m_numUnstrParts;j++)
	{
	  for(k=0;k<get_num_timesteps();k++)
	    {	    
	      if( my_saf_is_valid_field_handle( &(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[j][i][k])) ) 
		{
		  SAF_Unit l_unit;
		  SAF_Quantity l_quantity;
		  SAF_Field l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[j][i][k];
		  saf_describe_field(SAF_ALL, &l_field, NULL,NULL, NULL, &l_unit,NULL,NULL,NULL,NULL,NULL, 
				     NULL, NULL, NULL, NULL, NULL, NULL);
		  saf_describe_unit(SAF_ALL,&l_unit,&g_unstrGlobals->m_unstrMeshVarUnitName[i],NULL,NULL,NULL,NULL,NULL,NULL, &l_quantity );
		  saf_describe_quantity(SAF_ALL,&l_quantity,&g_unstrGlobals->m_unstrMeshVarQuantName[i],NULL,NULL,NULL,NULL);

		  /*printinfo("var=%d   unit=%s   quant=%s\n",i,g_unstrGlobals->m_unstrMeshVarUnitName[i],
		    g_unstrGlobals->m_unstrMeshVarQuantName[i] );*/
		  l_done=1;
		  break;
		}
	    }
	  if(l_done) break;
	}
    }


  /*
  ** Print out a truth table. 
  */
  if(g_unstrGlobals->m_maxNumUnstrVars)
    {
      printinfo("\nKEY: 1=there is a var field on the set, a subset, or superset\n");
      printinfo("     *=there is a field on the topo related set\n");
      printinfo("     0=there is no field at all\n");
      for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
	{
	  char l_partname[256];
	  get_unstr_part_name(i,l_partname,255);

	  printinfo("\npart %d of %d: \"%s\" var(vert) vs time(horiz) truth table:\n",i,g_unstrGlobals->m_numUnstrParts,l_partname);
	  for(j=0;j<g_unstrGlobals->m_maxNumUnstrVars;j++)
	    {
	      char *l_fieldName=0;
	      for(k=0;k<get_num_timesteps();k++)
		{
		  if( my_saf_is_valid_field_handle( &(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k])) ) 
		    {
		      if(!l_fieldName)
			{
			  saf_describe_field(SAF_ALL, &(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k]), NULL, 
					     &l_fieldName,
					     NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
			}
		      printinfo("1");
		    }
		  else
		    {
		      /*using is_there_an_unstr_var_instance means that the set that contains this
			var's points will also be checked*/
		      if( is_there_an_unstr_var_instance(i,j,k) ) printinfo("*"); else printinfo("0");
		    }
		}

	      if(l_fieldName) printinfo(" \"%s\" ",l_fieldName); else printinfo(" () ");

	      if( is_this_unstr_var_mapped_to_point(j) ) printinfo(" (point)");
	      else if( is_this_unstr_var_mapped_to_tri(j) ) printinfo(" (tri)");
	      else if( is_this_unstr_var_mapped_to_quad(j) ) printinfo(" (quad)");
	      else if( is_this_unstr_var_mapped_to_hex(j) ) printinfo(" (hex)");
	      else if( is_this_unstr_var_mapped_to_pyramid(j) ) printinfo(" (pyramid)");
	      else if( is_this_unstr_var_mapped_to_tet(j) ) printinfo(" (tet)");
	      else if( is_this_unstr_var_mapped_to_prism(j) ) printinfo(" (prism)");
	      else if( is_this_unstr_var_mapped_to_line(j) ) printinfo(" (line)");
	      /*else if( is_this_unstr_var_mapped_to_0ball(j) ) printinfo(" (0ball)");*/
	      else if( is_this_unstr_var_mapped_to_1ball(j) ) printinfo(" (1ball)");
	      else if( is_this_unstr_var_mapped_to_2ball(j) ) printinfo(" (2ball)");
	      else if( is_this_unstr_var_mapped_to_3ball(j) ) printinfo(" (3ball)");
	      else if( is_this_unstr_var_mapped_to_multiple_cell_types(j) ) printinfo(" (multiple cell types)");
	      else if( is_this_unstr_var_mapped_to_mixed(j) ) printinfo(" (mixed)");
	      else printinfo(" (unknown cell type)");

	      if( is_this_unstr_var_scalar(j) ) printinfo(" (scalar)");
	      else if( is_this_unstr_var_vector(j) ) printinfo(" (vector)");
	      else if( is_this_unstr_var_tensor(j) ) printinfo(" (tensor)");
	      else if( is_this_unstr_var_symtensor(j) ) printinfo(" (symtensor)");
	      else printinfo(" (unknown algebraic type)");

	      {
		char *l_unitname = get_unstr_var_unit_name(j);
		char *l_quantname = get_unstr_var_quant_name(j);
	    
		if(l_unitname) printinfo(" (%s)", l_unitname ); else printinfo(" (unk unit name)");
		if(l_quantname) printinfo(" (%s)", l_quantname ); else printinfo(" (unk quant name)");

		if(l_unitname) free(l_unitname);
		if(l_quantname) free(l_quantname);
	      }


#if 0
	      /*print out each field name*/  
	      printinfo("\t\t");
	      for(k=0;k<get_num_timesteps();k++)
		{
		  if( my_saf_is_valid_field_handle(&(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k]) ))
		    {
		      char *l_name=0;
		      saf_describe_field(SAF_ALL, &(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k]),  
					 NULL, &l_name, NULL, NULL,
					 NULL, NULL, NULL, NULL, NULL, NULL, NULL,NULL,
					 NULL, NULL, NULL, NULL);	
		      printinfo(" \"%s\"",l_name);
		      if(l_name) free(l_name);
		    }
		}	      
#endif


	      if(l_fieldName) free(l_fieldName);

	      printinfo("\n");
	    }
	}
      printinfo("\n");/*extra space after last truth table*/
    }






  /*debugging test only
    printinfo("\n**Begin test of all non-not-set fields in g_unstrGlobals->m_instancePerTimestepPerVarPerPart\n");
    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
    {
    for(j=0;j<g_unstrGlobals->m_maxNumUnstrVars;j++)
    {
    for(k=0;k<get_num_timesteps();k++)
    {
    if( my_saf_is_valid_field_handle(&(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k])) )
    {
    hid_t l_dsltype;
    size_t l_count=0;
    hbool_t l_hasBeenWritten;
    saf_data_has_been_written_to_field(SAF_ALL,&(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k]),&l_hasBeenWritten);
    saf_get_count_and_type_for_field(SAF_ALL, &(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j][k]), 
    NULL,&l_count, &l_dsltype);
    printinfo("\t ijk=%d,%d,%d    l_count=%d   l_dsltype=%s  hasBeenWritten=%d\n",
    i,j,k,(int)l_count,get_dsl_type_string(l_dsltype),
    l_hasBeenWritten );
    }
    }
    }
    }
    printinfo("**End test of all non-not-set fields in g_unstrGlobals->m_instancePerTimestepPerVarPerPart\n\n");
  */

    

#ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE

  g_unstrGlobals->m_pointsAreTimeDependentPerPart = (int *)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int));

  g_unstrGlobals->m_numPointsUsedPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));

  g_unstrGlobals->m_pointsUsedPerPartPerTimestep = (MY_PRECISION ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(MY_PRECISION **));

  g_unstrGlobals->m_pointsTransformPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));/*02jan2004*/

  g_unstrGlobals->m_pointElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_triElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_quadElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_tetElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_hexElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_pyramidElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_prismElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_lineElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  /*g_unstrGlobals->m_0ballElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));*/
  g_unstrGlobals->m_1ballElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_2ballElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  g_unstrGlobals->m_3ballElementsPerPartPerTimestep = (int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));

  if(g_unstrGlobals->m_useQuadraticElements)
  {
    g_unstrGlobals->m_quadraticTriElementsPerPartPerTimestep=(int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
    g_unstrGlobals->m_quadraticQuadElementsPerPartPerTimestep=(int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
    g_unstrGlobals->m_quadraticTetElementsPerPartPerTimestep=(int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
    g_unstrGlobals->m_quadraticHexElementsPerPartPerTimestep=(int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
    g_unstrGlobals->m_quadraticPyramidElementsPerPartPerTimestep=(int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
    g_unstrGlobals->m_quadraticPrismElementsPerPartPerTimestep=(int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
    g_unstrGlobals->m_quadraticLineElementsPerPartPerTimestep=(int **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
  }


  g_unstrGlobals->m_pointConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_triConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_quadConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_tetConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_hexConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_pyramidConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_prismConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_lineConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  /*g_unstrGlobals->m_0ballConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));*/
  g_unstrGlobals->m_1ballConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_2ballConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  g_unstrGlobals->m_3ballConnectivityPerPartPerTimestep = (int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));

  if(g_unstrGlobals->m_useQuadraticElements)
  {
    g_unstrGlobals->m_quadraticTriConnectivityPerPartPerTimestep=(int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
    g_unstrGlobals->m_quadraticQuadConnectivityPerPartPerTimestep=(int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
    g_unstrGlobals->m_quadraticTetConnectivityPerPartPerTimestep=(int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
    g_unstrGlobals->m_quadraticHexConnectivityPerPartPerTimestep=(int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
    g_unstrGlobals->m_quadraticPyramidConnectivityPerPartPerTimestep=(int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
    g_unstrGlobals->m_quadraticPrismConnectivityPerPartPerTimestep=(int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
    g_unstrGlobals->m_quadraticLineConnectivityPerPartPerTimestep=(int ***)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int **));
  }

  for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
    {
      g_unstrGlobals->m_pointsAreTimeDependentPerPart[i] = -1;

      g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));

      g_unstrGlobals->m_pointsUsedPerPartPerTimestep[i] = (MY_PRECISION **)malloc(get_num_timesteps()*sizeof(MY_PRECISION *));

      g_unstrGlobals->m_pointsTransformPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));/*02jan2004*/

      g_unstrGlobals->m_pointElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_triElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_quadElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_tetElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_hexElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_pyramidElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_prismElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_lineElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      /*g_unstrGlobals->m_0ballElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));*/
      g_unstrGlobals->m_1ballElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_2ballElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      g_unstrGlobals->m_3ballElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));

      if(g_unstrGlobals->m_useQuadraticElements)
      {
        g_unstrGlobals->m_quadraticTriElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
        g_unstrGlobals->m_quadraticQuadElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
        g_unstrGlobals->m_quadraticTetElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
        g_unstrGlobals->m_quadraticHexElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
        g_unstrGlobals->m_quadraticPyramidElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
        g_unstrGlobals->m_quadraticPrismElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
        g_unstrGlobals->m_quadraticLineElementsPerPartPerTimestep[i] = (int *)malloc(get_num_timesteps()*sizeof(int));
      }

      g_unstrGlobals->m_pointConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_triConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_quadConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_tetConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_hexConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_pyramidConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_prismConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_lineConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      /*g_unstrGlobals->m_0ballConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));*/
      g_unstrGlobals->m_1ballConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_2ballConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));
      g_unstrGlobals->m_3ballConnectivityPerPartPerTimestep[i] = (int **)malloc(get_num_timesteps()*sizeof(int *));

      if(g_unstrGlobals->m_useQuadraticElements)
      {
        g_unstrGlobals->m_quadraticTriConnectivityPerPartPerTimestep[i]=(int **)malloc(get_num_timesteps()*sizeof(int *));
        g_unstrGlobals->m_quadraticQuadConnectivityPerPartPerTimestep[i]=(int **)malloc(get_num_timesteps()*sizeof(int *));
        g_unstrGlobals->m_quadraticTetConnectivityPerPartPerTimestep[i]=(int **)malloc(get_num_timesteps()*sizeof(int *));
        g_unstrGlobals->m_quadraticHexConnectivityPerPartPerTimestep[i]=(int **)malloc(get_num_timesteps()*sizeof(int *));
        g_unstrGlobals->m_quadraticPyramidConnectivityPerPartPerTimestep[i]=(int **)malloc(get_num_timesteps()*sizeof(int *));
        g_unstrGlobals->m_quadraticPrismConnectivityPerPartPerTimestep[i]=(int **)malloc(get_num_timesteps()*sizeof(int *));
        g_unstrGlobals->m_quadraticLineConnectivityPerPartPerTimestep[i]=(int **)malloc(get_num_timesteps()*sizeof(int *));
      }

      for(j=0;j<get_num_timesteps();j++) 
	{
	  g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[i][j] = -1;

	  g_unstrGlobals->m_pointsUsedPerPartPerTimestep[i][j] = NULL;

	  g_unstrGlobals->m_pointsTransformPerPartPerTimestep[i][j] = NULL;/*02jan2004*/

	  g_unstrGlobals->m_pointElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_triElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_quadElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_tetElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_hexElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_pyramidElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_prismElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_lineElementsPerPartPerTimestep[i][j] = -1;
	  /*g_unstrGlobals->m_0ballElementsPerPartPerTimestep[i][j] = -1;*/
	  g_unstrGlobals->m_1ballElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_2ballElementsPerPartPerTimestep[i][j] = -1;
	  g_unstrGlobals->m_3ballElementsPerPartPerTimestep[i][j] = -1;

          if(g_unstrGlobals->m_useQuadraticElements)
          {
	    g_unstrGlobals->m_quadraticTriElementsPerPartPerTimestep[i][j] = -1;
	    g_unstrGlobals->m_quadraticQuadElementsPerPartPerTimestep[i][j] = -1;
	    g_unstrGlobals->m_quadraticTetElementsPerPartPerTimestep[i][j] = -1;
	    g_unstrGlobals->m_quadraticHexElementsPerPartPerTimestep[i][j] = -1;
	    g_unstrGlobals->m_quadraticPyramidElementsPerPartPerTimestep[i][j] = -1;
	    g_unstrGlobals->m_quadraticPrismElementsPerPartPerTimestep[i][j] = -1;
	    g_unstrGlobals->m_quadraticLineElementsPerPartPerTimestep[i][j] = -1;
          }

	  g_unstrGlobals->m_pointConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_triConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_quadConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_tetConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_hexConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_pyramidConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_prismConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_lineConnectivityPerPartPerTimestep[i][j] = NULL;
	  /*g_unstrGlobals->m_0ballConnectivityPerPartPerTimestep[i][j] = NULL;*/
	  g_unstrGlobals->m_1ballConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_2ballConnectivityPerPartPerTimestep[i][j] = NULL;
	  g_unstrGlobals->m_3ballConnectivityPerPartPerTimestep[i][j] = NULL;

          if(g_unstrGlobals->m_useQuadraticElements)
          {
	    g_unstrGlobals->m_quadraticTriConnectivityPerPartPerTimestep[i][j] = NULL;
	    g_unstrGlobals->m_quadraticQuadConnectivityPerPartPerTimestep[i][j] = NULL;
	    g_unstrGlobals->m_quadraticTetConnectivityPerPartPerTimestep[i][j] = NULL;
	    g_unstrGlobals->m_quadraticHexConnectivityPerPartPerTimestep[i][j] = NULL;
	    g_unstrGlobals->m_quadraticPyramidConnectivityPerPartPerTimestep[i][j] = NULL;
	    g_unstrGlobals->m_quadraticPrismConnectivityPerPartPerTimestep[i][j] = NULL;
	    g_unstrGlobals->m_quadraticLineConnectivityPerPartPerTimestep[i][j] = NULL;
          }

	}
    }
#endif




  printinfo("*******END     init_unstr_mesh_reader***************************\n\n");



  g_unstrGlobals->m_unstrMeshReaderIsInitialized=1;

  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description:
 *
 * This is used to replace theRow.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_index_of_set(SAF_Set *a_set)
{
  int i;
  for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
  {
    int l_retval = ss_pers_cmp((ss_pers_t*)a_set, (ss_pers_t*)(&(g_unstrGlobals->m_allSets[i])), NULL);
    if(!l_retval) return(i);
  }
  return(-1);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description:
 *
 * This is used to replace theRow.
 *--------------------------------------------------------------------------------------------------- */
SAF_Set * 
UNSTR_CLASS_NAME_IF_CPP
get_set_of_index(int a_index)
{
  if(a_index<0 || a_index>=g_unstrGlobals->m_numAllSets) return(NULL);/*sslib hooey it was: return((SAF_Set *)(&(SS_SET_NULL)));*/
  else return(&(g_unstrGlobals->m_allSets[a_index]));
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is the Reader Initialized?
 *
 * Description: 
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_unstr_mesh_reader_initialized()
{
  return(g_unstrGlobals->m_unstrMeshReaderIsInitialized);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of Unstructured Variables
 *
 * Description: 
 * When the unstructured mesh reader is initialized (using init_unstr_mesh_reader), all fields that
 * can be considered variables (i.e. suitable for color mapping onto some object) are found using
 * can_this_field_be_a_variable. Then the fields are grouped and sorted according to which fields
 * go together (e.g. are two different timesteps of the same variable, or are the same variable
 * instance on two different objects) using the function are_these_two_fields_from_the_same_var.
 * Finally, init_unstr_mesh_reader records the total number of distinct variables. This function
 * simply gets that value and returns it.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_unstr_vars( )
{
    return( g_unstrGlobals->m_maxNumUnstrVars );
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get and Check all Unstructured Variable Names
 *
 * Description: 
 * This function is called the first time that get_unstr_var_name is called. 
 * It gets the names for every variable at once, and stores them in a global array.
 * The reason for doing this is to prevent conflicting names from occuring.
 *--------------------------------------------------------------------------------------------------- */
void 
UNSTR_CLASS_NAME_IF_CPP
get_all_unstr_var_names( unsigned int a_maxlen )
{
  int i,j,l_var;
  SAF_Field *l_list=0;

  if(!g_unstrGlobals->m_maxNumUnstrVars || !g_unstrGlobals->m_numUnstrParts || !get_num_timesteps()) return;
  g_unstrGlobals->m_unstrVarNames = (char **)malloc(g_unstrGlobals->m_maxNumUnstrVars*sizeof(char *));
  for(i=0;i<g_unstrGlobals->m_maxNumUnstrVars;i++) g_unstrGlobals->m_unstrVarNames[i]=0;
  l_list = (SAF_Field *)malloc(g_unstrGlobals->m_numUnstrParts*get_num_timesteps()*sizeof(SAF_Field));

  /*make a list of all of the fields that comprise this variable, then call
    get_variable_name_for_field_list */
  for(l_var=0;l_var<g_unstrGlobals->m_maxNumUnstrVars;l_var++)
  {
    int l_count=0;
    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
      {
	if( g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][l_var] )
	  {
	    for(j=0;j<get_num_timesteps();j++)
	      {
		if( my_saf_is_valid_field_handle( &(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][l_var][j])) )
		  {
		    l_list[l_count++] = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][l_var][j];
		  }
	      }
	  }
      }
#ifdef __cplusplus
    g_unstrGlobals->m_unstrVarNames[l_var] = m_variableNames->get_variable_name_for_field_list(l_count,l_list,a_maxlen);
#else
    g_unstrGlobals->m_unstrVarNames[l_var] = get_variable_name_for_field_list(l_count,l_list,a_maxlen);
#endif


    printinfo("get_all_unstr_var_names unstr var %d of %d = %s  (num fields used = %d)\n",
	      l_var,g_unstrGlobals->m_maxNumUnstrVars,g_unstrGlobals->m_unstrVarNames[l_var],l_count);
  }
  free(l_list);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Name of an Unstructured Variable
 *
 * Description: 
 * Up to a_maxlen characters of the name will be written to the preallocated array a_str.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_unstr_var_name( int a_whichVar, char *a_str, int a_maxlen )
{
  if( a_whichVar<0 || a_whichVar>=g_unstrGlobals->m_maxNumUnstrVars )
    {
      printinfo("get_unstr_var_name error, a_whichVar=%d is out of range\n",a_whichVar);
      return(0);
    }  

  /*if first time thru, allocate the global vars and get the names*/
  if(!g_unstrGlobals->m_unstrVarNames)
    {
      get_all_unstr_var_names( (unsigned int) a_maxlen);
    }

  /*if we have already found this var name, return it*/
  if( g_unstrGlobals->m_unstrVarNames[a_whichVar] )
    {
      strncpy(a_str,g_unstrGlobals->m_unstrVarNames[a_whichVar],(size_t)a_maxlen);
      return(0);
    }

  /*otherwise, return an error string*/
  snprintf(a_str,(size_t)a_maxlen,"unstr_var_name_error" );

  return(-1);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get all Fields on a Set
 *
 * Description:
 *
 * Find a_set in the global list g_allSets, and then set the arg a_fields
 * to the associated global pointer to that set's field list, and return
 * the number of fields in the list. This function simply gets info from global arrays. 
 * The caller must not free any of the argument arrays!
 *
 * XXX todo sort the list, make access faster
 *
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_all_fields_for_set( SAF_Set a_set, SAF_Field **a_fields, int **a_times, int **a_var, 
			    int *a_numCoordFields, SAF_Field **a_coordFields )
{
  int i;
  for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
    {
      if( SAF_EQUIV(&(g_unstrGlobals->m_allSets[i]),&a_set) )
	{
	  if(a_fields) a_fields[0] = g_unstrGlobals->m_allSetsFields[i];
	  if(a_coordFields) a_coordFields[0] = g_unstrGlobals->m_allSetsCoordFields[i];
	  if(a_times) a_times[0] = g_unstrGlobals->m_allSetsFieldsTimestep[i];
	  if(a_var) a_var[0] = g_unstrGlobals->m_allSetsFieldsVariable[i];
	  if(a_numCoordFields) a_numCoordFields[0] = g_unstrGlobals->m_allSetsNumCoordFields[i];
	  return(g_unstrGlobals->m_allSetsNumFields[i]);
	}
    }
  /*note: we should never get here!*/
  printinfo("error in get_all_fields_for_set\n");
  exit(-1);

  if(a_fields) a_fields[0]=NULL;
  if(a_coordFields) a_coordFields[0]=NULL;
  if(a_times) a_times[0]=NULL;
  if(a_var) a_var[0]=NULL;
  if(a_numCoordFields) a_numCoordFields[0] = 0;
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Units Name of a Variable
 *
 * Description: 
 *
 * Get the units name for the variable with index a_which, e.g. Kelvin, Celsius.
 *--------------------------------------------------------------------------------------------------- */
char *
UNSTR_CLASS_NAME_IF_CPP
get_unstr_var_unit_name(int a_which)
{
  if(a_which>=0 && a_which<g_unstrGlobals->m_maxNumUnstrVars)
    {
      if( g_unstrGlobals->m_unstrMeshVarUnitName && g_unstrGlobals->m_unstrMeshVarUnitName[a_which] )
	{
	    return( ens_strdup(g_unstrGlobals->m_unstrMeshVarUnitName[a_which])  );
	}
    }
    return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Quantity Name of a Variable
 *
 * Description: 
 *
 * Get the quantity name for the variable with index a_which, e.g. Temperature, Frequency.
 *--------------------------------------------------------------------------------------------------- */
char *
UNSTR_CLASS_NAME_IF_CPP
get_unstr_var_quant_name(int a_which)
{
  if(a_which>=0 && a_which<g_unstrGlobals->m_maxNumUnstrVars)
    {
      if( g_unstrGlobals->m_unstrMeshVarUnitName && g_unstrGlobals->m_unstrMeshVarUnitName[a_which] )
	{
	    return( ens_strdup(g_unstrGlobals->m_unstrMeshVarQuantName[a_which])  );
	}
    } 
    return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is an Unstructured Variable of a Given Algebraic Type?
 *
 * Description: 
 *
 * Finds the first instance of a variable and checks its algebraic type. If it
 * matches, 1 is returned, otherwise 0 is returned. Note that all fields of a
 * variable will have the same algebraic type, according to the function are_these_two_fields_from_the_same_var.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_unstr_var_of_this_algebraic_type( int a_whichVar, SAF_Algebraic *a_algType )
{
  int i,j;
  for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
    {
      if(!g_unstrGlobals->m_instancePerTimestepPerVarPerPart)
	{
	  printinfo("error is_this_unstr_var_of_this_algebraic_type: instances not allocated\n");
	  exit(-1);
	}
      for(j=0;j<get_num_timesteps();j++)
	{
	  if( a_whichVar>=0 && a_whichVar<g_unstrGlobals->m_maxNumUnstrVars )
	    {
	      SAF_Field l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][a_whichVar][j];
	      if( my_saf_is_valid_field_handle(&l_field) )
		{
		  SAF_FieldTmpl l_tmpl;
		  SAF_Algebraic l_algebraicType;
		  saf_describe_field(SAF_ALL, &l_field, &l_tmpl, NULL, NULL, NULL,
				     NULL, NULL, NULL, NULL, NULL, NULL, NULL,  
				     NULL, NULL, NULL, NULL);
		  if(my_saf_is_valid_ftmpl_handle(&l_tmpl))
		    {
		      saf_describe_field_tmpl(SAF_ALL,&l_tmpl,NULL,&l_algebraicType,NULL,NULL,NULL,NULL);
		      if( SAF_EQUIV(&l_algebraicType,a_algType) ) 
			{
			  return(1);
			}
		      else
			{
			  return(0);
			}
		    }
		  else
		    {
		      printinfo("\nerror is_this_unstr_var_of_this_algebraic_type: invalid field template\n\n");
		      return(0);
		    }
		}
	    }
	}
    }
  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is this Unstructured Variable a Scalar?
 *
 * Description: 
 * Finds the first instance of a variable and checks its algebraic type. If it
 * matches, 1 is returned, otherwise 0 is returned. Note that all fields of a
 * variable will have the same algebraic type, according to the function are_these_two_fields_from_the_same_var.
 * 
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_unstr_var_scalar( int a_whichVar )
{
  return( is_this_unstr_var_of_this_algebraic_type( a_whichVar, SAF_ALGTYPE_SCALAR ) );
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is this Unstructured Variable a Vector?
 *
 * Description: 
 * Finds the first instance of a variable and checks its algebraic type. If it
 * matches, 1 is returned, otherwise 0 is returned. Note that all fields of a
 * variable will have the same algebraic type, according to the function are_these_two_fields_from_the_same_var.
 *
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_unstr_var_vector( int a_whichVar )
{
  return( is_this_unstr_var_of_this_algebraic_type( a_whichVar, SAF_ALGTYPE_VECTOR ) );
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is this Unstructured Variable a Tensor?
 *
 * Description: 
 * Finds the first instance of a variable and checks its algebraic type. If it
 * matches, 1 is returned, otherwise 0 is returned. Note that all fields of a
 * variable will have the same algebraic type, according to the function are_these_two_fields_from_the_same_var.
 *
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_unstr_var_tensor( int a_whichVar )
{
  return( is_this_unstr_var_of_this_algebraic_type( a_whichVar, SAF_ALGTYPE_TENSOR ) );
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is this Unstructured Variable a Symtensor?
 *
 * Description: 
 * Finds the first instance of a variable and checks its algebraic type. If it
 * matches, 1 is returned, otherwise 0 is returned. Note that all fields of a
 * variable will have the same algebraic type, according to the function are_these_two_fields_from_the_same_var.
 *
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_unstr_var_symtensor( int a_whichVar )
{
  return( is_this_unstr_var_of_this_algebraic_type( a_whichVar, SAF_ALGTYPE_SYMTENSOR ) );
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 *
 * Description: 
 *
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_unstr_var_mapped_to_this_celltype_on_this_part( int a_whichVar, SAF_CellType a_cellType, int a_whichPart )
{
  SAF_CellType l_cellType;

  /*Make sure the cache is initialized*/
  is_this_unstr_var_mapped_to_this_celltype( a_whichVar, a_cellType );

  /*Get the value from the cache*/
  l_cellType = g_unstrGlobals->m_celltypePerVarPerPart[a_whichVar][a_whichPart];

  return(l_cellType==a_cellType);
}





/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is a Given Variable Mapped to a Given Cell Type on a Given Part?
 *
 * Description: 
 * This macro is used to create all the instances of is_this_unstr_var_mapped_to_*_on_this_part
 * (hex, quad, etc.).
 *--------------------------------------------------------------------------------------------------- */

#define IS_THIS_VAR_MAPPED_ON_PART(MA_TYPELOWERCASE,MA_SAFCELLTYPE) \
int \
UNSTR_CLASS_NAME_IF_CPP \
is_this_unstr_var_mapped_to_ ## MA_TYPELOWERCASE ## _on_this_part( int a_whichVar, int a_whichPart ) \
{ \
  return( is_this_unstr_var_mapped_to_this_celltype_on_this_part(a_whichVar,MA_SAFCELLTYPE,a_whichPart) ); \
} 

IS_THIS_VAR_MAPPED_ON_PART(hex,SAF_CELLTYPE_HEX)
IS_THIS_VAR_MAPPED_ON_PART(quad,SAF_CELLTYPE_QUAD)
IS_THIS_VAR_MAPPED_ON_PART(tet,SAF_CELLTYPE_TET)
IS_THIS_VAR_MAPPED_ON_PART(tri,SAF_CELLTYPE_TRI)
IS_THIS_VAR_MAPPED_ON_PART(pyramid,SAF_CELLTYPE_PYRAMID)
IS_THIS_VAR_MAPPED_ON_PART(point,SAF_CELLTYPE_POINT)
IS_THIS_VAR_MAPPED_ON_PART(prism,SAF_CELLTYPE_PRISM)
IS_THIS_VAR_MAPPED_ON_PART(line,SAF_CELLTYPE_LINE)
/*IS_THIS_VAR_MAPPED_ON_PART(0ball,SAF_CELLTYPE_0BALL) not in sslib?*/
IS_THIS_VAR_MAPPED_ON_PART(1ball,SAF_CELLTYPE_1BALL)
IS_THIS_VAR_MAPPED_ON_PART(2ball,SAF_CELLTYPE_2BALL)
IS_THIS_VAR_MAPPED_ON_PART(3ball,SAF_CELLTYPE_3BALL)
IS_THIS_VAR_MAPPED_ON_PART(mixed,SAF_CELLTYPE_MIXED)

IS_THIS_VAR_MAPPED_ON_PART(quadraticHex,SAF_CELLTYPE_HEX)
IS_THIS_VAR_MAPPED_ON_PART(quadraticQuad,SAF_CELLTYPE_QUAD)
IS_THIS_VAR_MAPPED_ON_PART(quadraticTet,SAF_CELLTYPE_TET)
IS_THIS_VAR_MAPPED_ON_PART(quadraticTri,SAF_CELLTYPE_TRI)
IS_THIS_VAR_MAPPED_ON_PART(quadraticPyramid,SAF_CELLTYPE_PYRAMID)
IS_THIS_VAR_MAPPED_ON_PART(quadraticPrism,SAF_CELLTYPE_PRISM)
IS_THIS_VAR_MAPPED_ON_PART(quadraticLine,SAF_CELLTYPE_LINE)

#undef IS_THIS_VAR_MAPPED_ON_PART



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is a Given Variable Mapped to a Given Cell Type?
 *
 * Description: 
 * Finds the first instance of a variable and checks its cell type. If it
 * matches, 1 is returned, otherwise 0 is returned. 
 * 
 * Warning the following statement is still undecided:
 * Note that all fields of a variable will have the same cell type, 
 * according to the function are_these_two_fields_from_the_same_var.
 *
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_unstr_var_mapped_to_this_celltype( int a_whichVar, SAF_CellType a_cellType )
{
  int i,j;
  SAF_CellType l_cellType=SAF_CELLTYPE_SET;

  if( g_unstrGlobals->m_numUnstrParts<=0 || g_unstrGlobals->m_maxNumUnstrVars<=0 ) return(0);
  if(!g_unstrGlobals->m_instancePerTimestepPerVarPerPart)
    {
      printinfo("error is_this_unstr_var_mapped_to_this_celltype: instances not allocated\n");
      exit(-1);
    }
  if(a_cellType==SAF_CELLTYPE_SET)
    {/*This would mess up my later use of SAF_CELLTYPE_SET to mean "not set yet" and "no instance"*/
      printinfo("error is_this_unstr_var_mapped_to_this_celltype: argument cannot be SAF_CELLTYPE_SET\n");
      exit(-1);
    }



  /*If the cache is not initialized, do it now*/
  if( !g_unstrGlobals->m_celltypePerVarPerPart )
    {
      g_unstrGlobals->m_celltypePerVarPerPart=(SAF_CellType **)malloc(g_unstrGlobals->m_maxNumUnstrVars*sizeof(SAF_CellType *));
      for(i=0;i<g_unstrGlobals->m_maxNumUnstrVars;i++)
	{
	  g_unstrGlobals->m_celltypePerVarPerPart[i]=(SAF_CellType *)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(SAF_CellType));
	  for(j=0;j<g_unstrGlobals->m_numUnstrParts;j++)
	    {
	      g_unstrGlobals->m_celltypePerVarPerPart[i][j] = SAF_CELLTYPE_SET;/*here this signifies "not set yet" or "no instance"*/
	    }
	}
    }

  /*Try to find the answer in the cache already*/
  l_cellType = g_unstrGlobals->m_celltypePerVarPerPart[a_whichVar][0];
  for(j=1;j<g_unstrGlobals->m_numUnstrParts;j++)
    {
      if( l_cellType != g_unstrGlobals->m_celltypePerVarPerPart[a_whichVar][j] )
	{
	  if(l_cellType==SAF_CELLTYPE_SET)
	    {
	      l_cellType = g_unstrGlobals->m_celltypePerVarPerPart[a_whichVar][j];
	    }
	  else if(g_unstrGlobals->m_celltypePerVarPerPart[a_whichVar][j]!=SAF_CELLTYPE_SET)
	    {
	      /*This variable is not all the same SAF_CellType*/
	      l_cellType=SAF_CELLTYPE_ANY;
	      break;
	    }
	}
    }
  if( l_cellType == SAF_CELLTYPE_SET )
    {
      /*The cache has apparently not been set yet. Continue on and do so.*/
    }
  else
    {
      return(l_cellType==a_cellType);
    }
  

  /*The answer was not in the cache, so find it the normal way and put the result there*/
  l_cellType = SAF_CELLTYPE_SET;
  for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
    {
      SAF_CellType l_cellTypeForPart = SAF_CELLTYPE_SET;/*here this signifies "not set yet" or "no instance"*/
      for(j=0;j<get_num_timesteps();j++)
	{
	  if( a_whichVar>=0 && a_whichVar<g_unstrGlobals->m_maxNumUnstrVars )
	    {
	      SAF_Field l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][a_whichVar][j];
	      if( my_saf_is_valid_field_handle(&l_field) )
		{
		  SAF_Cat l_cat;
		  SAF_Set l_fieldset,l_partset;
		  SAF_CellType l_cellTypeForPartAndTime;

		  saf_describe_field(SAF_ALL, &l_field, NULL, NULL, &l_fieldset, NULL,
				     NULL, NULL, &l_cat, NULL, NULL, NULL, NULL,  
				     NULL, NULL, NULL, NULL);
		  saf_describe_collection(SAF_ALL,&l_fieldset,&l_cat,&l_cellTypeForPartAndTime,NULL,NULL,NULL,NULL);


		  /*
		  ** If the field is not on the part set, there is a chance that the collections
		  ** on the part's and the field's sets have different celltypes. If they do,
		  ** we want to use the celltype that is on the PART's set, because that is 
		  ** where the data will be mapped to in the end.
		  */
		  if(g_unstrGlobals->m_setIndexPerPart[i]>=0)
		    {
		      l_partset = g_unstrGlobals->m_allSets[g_unstrGlobals->m_setIndexPerPart[i]];
		      if( !SAF_EQUIV(&l_fieldset,&l_partset) )
			{
			  SAF_CellType l_otherCellType;
			  saf_describe_collection(SAF_ALL,&l_partset,&l_cat,&l_otherCellType,NULL,NULL,NULL,NULL);
			  if( l_cellTypeForPartAndTime!=l_otherCellType )
			    {
			      /*printinfo(" warning: part set=%s, field set=%s\n",
				get_celltype_desc(l_otherCellType),get_celltype_desc(l_cellTypeForPartAndTime) );*/
			      l_cellTypeForPartAndTime=l_otherCellType;
			    }
			}
		    }


		  if(l_cellTypeForPartAndTime==SAF_CELLTYPE_SET)
		    {/*This would mess up my later use of SAF_CELLTYPE_SET to mean "not set yet" and "no instance"*/
		      printinfo("error is_this_unstr_var_mapped_to_this_celltype: var field cannot be SAF_CELLTYPE_SET\n");
		      exit(-1);
		    }

		  if( l_cellTypeForPart == SAF_CELLTYPE_SET )
		    { /*This is the first instance we found*/
		      l_cellTypeForPart=l_cellTypeForPartAndTime;
		    }
		  else if( l_cellTypeForPart != l_cellTypeForPartAndTime )
		    {
		      /*This variable is not all the same SAF_CellType*/
		      l_cellTypeForPart=SAF_CELLTYPE_ANY;
		    }		      
		}
	    }
	}
      g_unstrGlobals->m_celltypePerVarPerPart[a_whichVar][i]=l_cellTypeForPart;

      if(l_cellType==SAF_CELLTYPE_SET)
	{
	  l_cellType = l_cellTypeForPart;
	}
      else if( l_cellType!=SAF_CELLTYPE_ANY && l_cellTypeForPart!=SAF_CELLTYPE_SET && l_cellType!=l_cellTypeForPart )
	{

	  /*printinfo("is_this_unstr_var_mapped_to_this_celltype var type old=%s  new=%s  so now is SAF_CELLTYPE_ANY\n",
	    get_celltype_desc(l_cellType),get_celltype_desc(l_cellTypeForPart));*/

	  /*This variable is not all the same SAF_CellType*/
	  l_cellType=SAF_CELLTYPE_ANY;
	}
    }




  return(l_cellType==a_cellType);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is this Unstructured Variable Mapped to this Cell Type?
 *
 * Description: 
 * This macro is used to create instances of the function is_this_unstr_var_mapped_to_*
 * (hex, quad, etc)
 *
 * Finds the first instance of a variable and checks its cell type. If it
 * matches, 1 is returned, otherwise 0 is returned. Note that all fields of a
 * variable will have the same cell type, according to the function are_these_two_fields_from_the_same_var.
 *
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */

#define IS_THIS_VAR_MAPPED(MA_TYPELOWERCASE,MA_SAFCELLTYPE) \
int \
UNSTR_CLASS_NAME_IF_CPP \
is_this_unstr_var_mapped_to_ ## MA_TYPELOWERCASE ( int a_whichVar) \
{ \
  return( is_this_unstr_var_mapped_to_this_celltype(a_whichVar,MA_SAFCELLTYPE) ); \
} 
IS_THIS_VAR_MAPPED(hex,SAF_CELLTYPE_HEX)
IS_THIS_VAR_MAPPED(quad,SAF_CELLTYPE_QUAD)
IS_THIS_VAR_MAPPED(tet,SAF_CELLTYPE_TET)
IS_THIS_VAR_MAPPED(tri,SAF_CELLTYPE_TRI)
IS_THIS_VAR_MAPPED(pyramid,SAF_CELLTYPE_PYRAMID)
IS_THIS_VAR_MAPPED(point,SAF_CELLTYPE_POINT)
IS_THIS_VAR_MAPPED(prism,SAF_CELLTYPE_PRISM)
IS_THIS_VAR_MAPPED(line,SAF_CELLTYPE_LINE)
/*IS_THIS_VAR_MAPPED(0ball,SAF_CELLTYPE_0BALL) not in sslib?*/
IS_THIS_VAR_MAPPED(1ball,SAF_CELLTYPE_1BALL)
IS_THIS_VAR_MAPPED(2ball,SAF_CELLTYPE_2BALL)
IS_THIS_VAR_MAPPED(3ball,SAF_CELLTYPE_3BALL)
IS_THIS_VAR_MAPPED(mixed,SAF_CELLTYPE_MIXED)
IS_THIS_VAR_MAPPED(multiple_cell_types,SAF_CELLTYPE_ANY)

#undef IS_THIS_VAR_MAPPED



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is there a Variable Instance for a Given Part, Variable and Timestep?
 *
 * Description: 
 * Return: 1 if true, 0 otherwise
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_there_an_unstr_var_instance( int a_whichPart,
				    int a_whichVar,
				    int a_whichTimestep )
{
  SAF_Field l_field;

  if(!is_part_arg_valid(a_whichPart)) return(0);
  if(!is_time_arg_valid(a_whichTimestep)) return(0);

  if( a_whichVar<0 || a_whichVar>=g_unstrGlobals->m_maxNumUnstrVars )
    {
      return(0);/*print nothing, because this could be normal*/
    }
  if(!g_unstrGlobals->m_instancePerTimestepPerVarPerPart)
    {
      printinfo("error is_there_an_unstr_var_instance: instances not allocated\n");
      exit(-1);
    }
  l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[a_whichPart][a_whichVar][a_whichTimestep];
  if( !my_saf_is_valid_field_handle(&l_field) )
    {


      int l_otherSetIndex=g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[a_whichPart][a_whichTimestep];
      if( l_otherSetIndex != g_unstrGlobals->m_setIndexPerPart[a_whichPart] &&
	  is_this_unstr_var_mapped_to_point(a_whichVar) )
	{
	  /*check if this var is mapped to set from which we get our points*/
	  int i;
          /*int l_otherPart=-1;*/
	  for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
	    {
	      if( l_otherSetIndex==g_unstrGlobals->m_setIndexPerPart[i] )
		{
		  l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][a_whichVar][a_whichTimestep];
		  if( my_saf_is_valid_field_handle(&l_field) )
		    {
		      /*l_otherPart = i;*/
		      return(1);
		    }
		  else
		    {
		      return(0);
		    }
		  /*break;*/
		}
	    }

	  /*XXX If we get here then the set with our nodes, l_otherSetIndex, is not actually a part!
	    This must be because SETS_THAT_HAVE_ONLY_CONN_POINTS_FOR_OTHER_SETS_ARE_NOT_PARTS is defined
	   */

	}     
      return(0);/*print nothing, because this is normal*/
    }
  return(1);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Read a Scalar Variable
 *
 * Description: 
 * Reads a scalar variable into the array a_data, which must be preallocated to size a_howManyEntriesIExpect.
 * If a_howManyEntriesIExpect is not correct for the field, there will be an error. The variable instance
 * is specified by the given part-variable-timestep triplet.
 *
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
read_unstr_scalar_variable( int a_whichPart,
			    int a_whichVar,
			    int a_whichTimestep,
			    MY_PRECISION *a_data,
			    int a_howManyEntriesIExpect )
{
  return( read_unstr_variable_internal(  a_whichPart,
					 a_whichVar,
					 a_whichTimestep,
					 a_data,
					 a_howManyEntriesIExpect,
					 0,
					 SAF_INTERLEAVE_NONE
					 ) );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
read_unstr_variable_internal( int a_whichPart,
			      int a_whichVar,
			      int a_whichTimestep,
			      MY_PRECISION *a_data,
			      int a_howManyEntriesIExpect,/*this does not include # of components, i.e. the
							    value is the same for a scalar var or for reading
							    an entire vector*/
			      const int a_whichComponent, /*-1 to get the entire vector*/
			      SAF_Interleave a_interleave )/*only matters if a_whichComponent==-1*/
{
  SAF_Field l_field;
  hid_t l_varType;
  int l_numComponents=0,i,j,k,l_entriesPerComponent=0;
  size_t l_count=0;
  int l_partSetIndex=-1,l_nodeSetIndex=-1,l_fieldSetIndex=-1;
  SAF_FieldTmpl l_tmpl;
  SAF_Cat l_fieldCat;
  SAF_Set l_fieldSet,l_partSet,l_nodeSet;
  int l_isPointVar=-1;
  SAF_Interleave l_interleave;
  MY_PRECISION *l_tempData=NULL;
  int *l_truthData=NULL;
  int l_gotSubsetData=0;

  if(!is_part_arg_valid(a_whichPart)) return(-1);
  if(!is_time_arg_valid(a_whichTimestep)) return(-1);

  if( a_whichVar<0 || a_whichVar>=g_unstrGlobals->m_maxNumUnstrVars )
    {
      return(-1);/*print nothing, because this could be normal*/
    }
  if(!g_unstrGlobals->m_instancePerTimestepPerVarPerPart)
    {
      printinfo("error read_unstr_variable_internal: instances not allocated\n");
      exit(-1);
    }

  l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[a_whichPart][a_whichVar][a_whichTimestep];
  l_partSetIndex = g_unstrGlobals->m_setIndexPerPart[a_whichPart];
  l_nodeSetIndex = g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[a_whichPart][a_whichTimestep];
  l_isPointVar = is_this_unstr_var_mapped_to_point(a_whichVar);


  if( !my_saf_is_valid_field_handle(&l_field) && l_isPointVar && l_nodeSetIndex!=l_partSetIndex )
    {
      /*We dont have a field for this var. But since it is a point var,
	and our nodes are stored on another set, there is the possibility
	that our node set contains this var.*/
      for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
	{
	  if( l_nodeSetIndex==g_unstrGlobals->m_setIndexPerPart[i] )
	    {
	      l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][a_whichVar][a_whichTimestep];
	      break;
	    }
	}
    }

  if(!my_saf_is_valid_field_handle(&l_field)) 
    {
      /*This is normal: the var was never written for this part&time*/
      return(-1);
    }




  saf_describe_field(SAF_ALL, &l_field, &l_tmpl, NULL, &l_fieldSet, NULL,
		     NULL, NULL, &l_fieldCat, NULL, NULL, NULL, &l_varType,  
		     &l_numComponents, NULL, &l_interleave, NULL);

  if( a_whichComponent >= l_numComponents ) 
    {
      printinfo("   read_unstr_variable_internal not prepared to handle l_numComponents=%d a_whichComponent=%d (type was %s)\n",
		l_numComponents,a_whichComponent,get_dsl_type_string( l_varType ));
      return(-1);
    }

  if( l_numComponents>1 && l_interleave==SAF_INTERLEAVE_NONE )
    {
      printinfo("error read_unstr_variable_internal not prepared to handle l_numComponents=%d and SAF_INTERLEAVE_NONE\n",
		l_numComponents);
      return(-1);
    }

  saf_get_count_and_type_for_field(SAF_ALL,&l_field, NULL,&l_count, NULL);


  if(!l_count)
    { /*check if the data was written to a composite field, and if so, get
	the count from the collection*/
      hbool_t l_result;
      saf_data_has_been_written_to_comp_field(SAF_ALL,&l_field,&l_result);
      if( l_result )
	{
	  int l_newCount=0;
	  printinfo("   read_unstr_variable_internal taking a chance that the comp field is correct\n");
	  saf_describe_collection(SAF_ALL,&l_fieldSet,&l_fieldCat,NULL,&l_newCount,NULL,NULL,NULL);

	  l_entriesPerComponent = (size_t)l_newCount;
	  l_count = l_entriesPerComponent*l_numComponents;

	  /*note: read_whole_field_to_my_precision will fail, because
	    we will be passing it a type of H5I_INVALID_HID. Need to figure out
	    what the correct l_varType is XXX .......for now, just dont
	    let component fields become vars */
	}
      if(!l_count)
	{
	  printinfo("   read_unstr_variable_internal error field has l_count=0\n");
	  return(-1);
	}
	  
    }


  l_entriesPerComponent = l_count/l_numComponents;


  l_partSet = g_unstrGlobals->m_allSets[ l_partSetIndex ];
  l_nodeSet = g_unstrGlobals->m_allSets[ l_nodeSetIndex ];


  if( SAF_EQUIV(&l_fieldSet,&l_partSet) )
    {
      l_fieldSetIndex=l_partSetIndex;
    }
  else if( SAF_EQUIV(&l_fieldSet,&l_nodeSet) )
    {
      l_fieldSetIndex=l_nodeSetIndex;
    }
  else
    {
      for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
	{
	  if( SAF_EQUIV(&l_fieldSet, &(g_unstrGlobals->m_allSets[i])) )
	    {
	      l_fieldSetIndex=i;
	      break;
	    }
	}
    }












  /*
  ** READ VARS FROM SUBSETS
  */
  if( (l_isPointVar && !SAF_EQUIV(&l_fieldSet,&l_nodeSet)) ||
      (!l_isPointVar && !SAF_EQUIV(&l_fieldSet,&l_partSet)) )
    {
      /*This is a hard case. The first possibility field is mapped to the points, but the
	points themselves are on another set. (see test_unstr_mesh.saf, 
	set MIXED_QUAD_TRI_TREE_SET_1a2,var nodal_1a2).The other possibility is that the 
	field is mapped to elements on another set. We will have to 
	look for a subset relation between the field set and the node/part set. If we
	find one, then we will read the field and transform it as if it
	were actually on the node/part set. Then we continue on to handle the 
	data as regular node/elem data.*/

      int l_catIndex=-1;
      if(l_isPointVar)
	{
	  l_catIndex= 0; /*we know this because l_isPointSet is true and we always sort the cats so nodes is 0*/
	}
      else
	{
	  for(i=0;i<g_unstrGlobals->m_subsetTransformsNumCats;i++)
	    {
	      if( SAF_EQUIV(&l_fieldCat,&(g_unstrGlobals->m_subsetTransforms[i][0].m_cat)) )
		{
		  l_catIndex=i;
		  break;
		}
	    }
	}
      if(l_catIndex>=0)
	{
	  int l_superSetIndex;
	  SAF_Set l_superSet;
	  if(l_isPointVar)
	    {
	      l_superSet=l_nodeSet;
	      l_superSetIndex=l_nodeSetIndex;
	    }
	  else 
	    {
	      l_superSet=l_partSet;
	      l_superSetIndex=l_partSetIndex;
	    }
	
	  for(i=1;i<g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex].m_length;i++)
	    {
	      if(SAF_EQUIV(&l_fieldSet, &(g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex].m_sets[i])))
		{
		  if(!read_field_from_subset_check( l_field, l_fieldSet, g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex]))
		    {

		      int l_newCount=0;
		      saf_describe_collection(SAF_ALL,&l_superSet,&l_fieldCat,NULL,&l_newCount,NULL,NULL,NULL);
		      l_entriesPerComponent = (size_t)l_newCount;
		      l_count = l_entriesPerComponent*l_numComponents;


		      l_gotSubsetData=1;
		      l_tempData=(MY_PRECISION *)malloc(l_count*sizeof(MY_PRECISION));
		      l_truthData=(int *)malloc(l_entriesPerComponent*sizeof(int));
		      for(j=0;j<l_entriesPerComponent;j++) l_truthData[j]=0;


		      read_field_from_subset( 
#ifdef __cplusplus
					     m_strReader,
#endif
					     l_field, l_fieldSet, 
					     g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex], 
					     &l_tempData, &l_truthData );

		      printinfo("    read_unstr_variable_internal set l_gotSubsetData=1 %s!!!\n",
				l_isPointVar ? "with a point var":"elem var from subset");


#if 0
		      printinfo("data :");
		      for(j=0;j<l_count;j++) 
			{
			  printinfo(" %.1f",l_tempData[j]);



			  if(l_numComponents>1 && l_interleave==SAF_INTERLEAVE_VECTOR && !((j+1)%l_numComponents)) 
			    printinfo(",");
			  else if(l_numComponents>1 && 
				  (l_interleave==SAF_INTERLEAVE_COMPONENT||l_interleave==SAF_INTERLEAVE_INDEPENDENT) && 
				  !((j+1)%l_entriesPerComponent)) 
			    printinfo(",");

			}
		      printinfo("\n");

		      printinfo("truth:");
		      for(j=0;j<l_entriesPerComponent;j++) printinfo(" %d",l_truthData[j]);
		      printinfo("\n\n");
#endif



#ifdef ALLOW_MULTI_FIELDS_PER_VAR_PATCH
		      /*Because of the subset relation, there is a chance that there are multiple
			fields for this part-time-variable combo. For example, a part may have two
			subsets of hex elems, each with a hex-mapped field.*/
		      {
			int kk;
			int l_slot=-1;
			/*find the correct place in the cache*/
			for(kk=0;kk<g_unstrGlobals->m_multiFieldsListLength;kk++)
			  {
			    if( SAF_EQUIV(&(g_unstrGlobals->m_multiFieldsList[kk]),&l_field))
			      {
				l_slot=kk+1;
				break;
			      }			  
			  }

			/*If the field was found in the cache, map the other fields over the data also*/
			if(l_slot>=0)
			  {
			    while( my_saf_is_valid_field_handle(&(g_unstrGlobals->m_multiFieldsList[l_slot])) )
			      {
				SAF_Field l_extraField = g_unstrGlobals->m_multiFieldsList[l_slot];
				SAF_Set l_extraSet;
				SAF_Cat l_extraCat;
				int l_extraNumComponents;
				SAF_Interleave l_extraInterleave;
				hid_t l_extraVarType;
				int l_found=0;

				if(!my_saf_is_valid_field_handle(&l_extraField))
				  {
				    printinfo("\nerror read_unstr_variable_internal invalid extra subset field\n\n");
				    exit(-1);
				  }

				saf_describe_field(SAF_ALL, &l_extraField, NULL, NULL, &l_extraSet, NULL,
						   NULL, NULL, &l_extraCat,  NULL, NULL, NULL, &l_extraVarType,  
						   &l_extraNumComponents, NULL, &l_extraInterleave, NULL);

				/*These conditions might be too strict*/
				if(l_numComponents==l_extraNumComponents && SAF_EQUIV(&l_extraCat,&l_fieldCat) )
				  {
				    for(kk=1;kk<g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex].m_length;kk++)
				      {
					if(SAF_EQUIV(&l_extraSet, 
						     &(g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex].m_sets[kk])))
					  {
					    if(!read_field_from_subset_check( l_extraField, l_extraSet, 
									      g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex]))
					      {

						if(l_interleave==l_extraInterleave || l_numComponents==1)
						  {
						    /*this is the normal case when the subset interleaves match*/
						    read_field_from_subset( 
#ifdef __cplusplus
									   m_strReader,
#endif
									   l_extraField, l_extraSet, 
									   g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex], 
									   &l_tempData, &l_truthData );
						  }
						else
						  {
						    /*This is for the odd case where some of the subset fields are SAF_INTERLEAVE_VECTOR
						      and others are SAF_INTERLEAVE_COMPONENT. Is not very efficient, but rarely used/ */
						    MY_PRECISION *l_tempTempData=NULL;
						    int *l_tempTruthData=NULL;
						    l_tempTempData=(MY_PRECISION *)malloc(l_count*sizeof(MY_PRECISION));
						    l_tempTruthData=(int *)malloc(l_entriesPerComponent*sizeof(int));
						    for(j=0;j<l_entriesPerComponent;j++) l_tempTruthData[j]=0;

						    read_field_from_subset( 
#ifdef __cplusplus
									   m_strReader,
#endif
									   l_extraField, l_extraSet, 
									   g_unstrGlobals->m_subsetTransforms[l_catIndex][l_superSetIndex], 
									   &l_tempTempData, &l_tempTruthData );


						    if(l_interleave==SAF_INTERLEAVE_VECTOR && 
						       (l_extraInterleave==SAF_INTERLEAVE_COMPONENT
							||l_extraInterleave==SAF_INTERLEAVE_INDEPENDENT) )
						      {
							change_from_component_to_vector_interleave(l_numComponents,
												   (size_t)l_entriesPerComponent,
												   l_tempTempData);
						      }
						    else if((l_interleave==SAF_INTERLEAVE_COMPONENT
							     ||l_interleave==SAF_INTERLEAVE_INDEPENDENT) && 
							    l_extraInterleave==SAF_INTERLEAVE_VECTOR)
						      {
							change_from_vector_to_component_interleave(l_numComponents,
												   (size_t)l_entriesPerComponent,
												   l_tempTempData);
						      }
						    else printinfo("\nerror read_unstr_variable_internal bad interleave\n\n");

						    if(l_interleave==SAF_INTERLEAVE_VECTOR )
						      {
							for(j=0;j<l_entriesPerComponent;j++)
							  {
							    if( l_tempTruthData[j] )
							      {
								for(k=0;k<l_numComponents;k++)	      
								  {
								    l_tempData[l_numComponents*j+k]=
								      l_tempTempData[l_numComponents*j+k];
								  }
								l_truthData[j]=1;
							      }
							  }
						      }
						    else
						      {
							for(j=0;j<l_entriesPerComponent;j++)
							  {
							    if( l_tempTruthData[j] )
							      {
								for(k=0;k<l_numComponents;k++)	      
								  {
								    l_tempData[k*l_entriesPerComponent+j]=
								      l_tempTempData[k*l_entriesPerComponent+j];
								  }
								l_truthData[j]=1;
							      }
							  }
						      }

						    free(l_tempTempData);
						    free(l_tempTruthData);
						  }


						l_found=1;
						printinfo("    read_unstr_variable_internal got additional %s var subset field!\n",
							  l_isPointVar?"node":"elem");



#if 0 /****************just printing*************/
						printinfo("data :");
						for(j=0;j<l_count;j++) 
						  {
						    printinfo(" %.1f",l_tempData[j]);
						    if(l_numComponents>1 && l_interleave==SAF_INTERLEAVE_VECTOR 
						       && !((j+1)%l_numComponents)) 
						      printinfo(",");
						    else if(l_numComponents>1 && (l_interleave==SAF_INTERLEAVE_COMPONENT 
										  ||l_interleave==SAF_INTERLEAVE_INDEPENDENT)
							    && !((j+1)%l_entriesPerComponent)) 
						      printinfo(",");
						  }
						printinfo("\ntruth:");
						for(j=0;j<l_entriesPerComponent;j++) printinfo(" %d",l_truthData[j]);
						printinfo("\n");
#endif  /******************************************/


					      }
					  }
				      }
				    if(!l_found)
				      {
					printinfo("warning read_unstr_variable_internal cant get additional %s var subset field!\n",
						  l_isPointVar?"node":"elem");
				      }
				  }				  
				l_slot++;
			      }
			  }
		      }
#endif /*ALLOW_MULTI_FIELDS_PER_VAR_PATCH*/


		    }
		}
	    }
	}
    }



  /*
  ** READ VARS FROM SUPERSETS
  */
  if(!l_gotSubsetData)
    {
      if( !l_isPointVar && !SAF_EQUIV(&l_fieldSet,&l_partSet) )
	{
	  /*This could be a hard case. The field is mapped to elements on another set. 
	    We will have to look for a subset relation between the field set and the part set.
	    If we find one, then we will read the field and transform it as if it
	    were actually on the part set. Then we continue on to handle the 
	    data as regular elem data.*/
	  int l_catIndex = -1;
	  for(i=0;i<g_unstrGlobals->m_subsetTransformsNumCats;i++)
	    {
	      if( SAF_EQUIV(&l_fieldCat,&(g_unstrGlobals->m_subsetTransforms[i][0].m_cat)) )
		{
		  l_catIndex=i;
		  break;
		}
	    }
	  if(l_catIndex>=0)
	    {
	      /*Look for a case when the part set is a subset of the field set.
		(see test_unstr_mesh.saf: MIXED_QUAD_TRI_TREE_SET_1 global_elem_var)*/
	      for(i=1;i<g_unstrGlobals->m_subsetTransforms[l_catIndex][l_fieldSetIndex].m_length;i++)
		{
		  if(SAF_EQUIV(&l_partSet, &(g_unstrGlobals->m_subsetTransforms[l_catIndex][l_fieldSetIndex].m_sets[i])))
		    {
		      if(!read_field_from_superset_check( l_field, l_partSet, 
							  g_unstrGlobals->m_subsetTransforms[l_catIndex][l_fieldSetIndex]))
			{

			  int l_newCount=0;
			  saf_describe_collection(SAF_ALL,&l_partSet,&l_fieldCat,NULL,&l_newCount,NULL,NULL,NULL);
			  l_entriesPerComponent = (size_t)l_newCount;
			  l_count = l_entriesPerComponent*l_numComponents;

			  l_gotSubsetData=1;
			  l_tempData=(MY_PRECISION *)malloc(l_count*sizeof(MY_PRECISION));


			  read_field_from_superset( 
#ifdef __cplusplus
						   m_strReader,
#endif
						   l_field, l_partSet, 
						   g_unstrGlobals->m_subsetTransforms[l_catIndex][l_fieldSetIndex], 
						   &l_tempData );

#if 0
			  printinfo("from superset data :");
			  for(j=0;j<l_count;j++) 
			    {
			      printinfo(" %.1f",l_tempData[j]);
			      if(l_numComponents>1 && l_interleave==SAF_INTERLEAVE_VECTOR && !((j+1)%l_numComponents)) 
				printinfo(",");
			      else if(l_numComponents>1 && (l_interleave==SAF_INTERLEAVE_COMPONENT
							    ||l_interleave==SAF_INTERLEAVE_INDEPENDENT) && !((j+1)%l_entriesPerComponent)) 
				printinfo(",");
			    }
			  printinfo("\n");
#endif


			  printinfo("    read_unstr_variable_internal set l_gotSubsetData=1 elem var from superset!!!\n");
			}
		    }
		}
	    }
	}
    }






#ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE
  /******************READ A POINT VAR THAT IS ON THE NODESET, IF THE NODES HAVE BEEN REDUCED***************/
  if( l_isPointVar && (SAF_EQUIV(&l_fieldSet,&l_nodeSet) || l_gotSubsetData) &&
      ((g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart]==1 
	&& g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][a_whichTimestep]) ||
       (!g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart] 
	&& g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][0]))
      )
    {
      /* This would have been a simple case of reading a point var on the nodeset, except that
	 the variable field we will read will have more entries than we want, because
	 we removed all the unused points for this part. Here we need to transform the 
	 read array into the desired array.
      */
      int l_entriesFound=0;
      MY_PRECISION *l_fromPtr;
      int l_reducedNumber=0, l_pointsTimestep=0;

      if(g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart]==1)  l_pointsTimestep=a_whichTimestep;
      l_reducedNumber = g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][l_pointsTimestep];

      if( a_howManyEntriesIExpect != l_reducedNumber )
	{
	  printinfo("   read_unstr_variable_internal cache error l_count=%d l_reducedNumber=%d a_howManyEntriesIExpect=%d\n",
		    (int)l_count, l_reducedNumber, a_howManyEntriesIExpect );
	  return(-1);
	}

      if( !a_data )
	{
	  if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	  if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	  return(0);/* just a dry run: return success */
	}


      if( !l_gotSubsetData )
	{
	  l_tempData=(MY_PRECISION *)malloc(l_count*sizeof(MY_PRECISION));

	  if( read_whole_field_to_my_precision(&l_field,l_varType,l_count,l_tempData) )
	    {
	      printinfo("    read_unstr_variable_internal error failed simple field reading\n");
	      { free(l_tempData); l_tempData=NULL; }
	      return(-1);
	    }
	}

      /*XXX this is inefficient. Should m_pointsTransformPerPartPerTimestep be of length l_reducedNumber instead of l_count?*/

      l_fromPtr=l_tempData;
      if(a_whichComponent>=0 && l_interleave==SAF_INTERLEAVE_VECTOR)
	{
	  for(j=0;j<l_entriesPerComponent;j++) 
	    {
	      int l_pt = g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][l_pointsTimestep][j];
	      if(l_pt>=0 && l_pt<l_reducedNumber)
		{
		  l_entriesFound++;
		  a_data[l_pt] = l_fromPtr[a_whichComponent]; 
		}
	      l_fromPtr += l_numComponents;
	    }
	}
      else if(l_numComponents==1 || (a_whichComponent>=0 && 
				     (l_interleave==SAF_INTERLEAVE_COMPONENT||l_interleave==SAF_INTERLEAVE_INDEPENDENT)) )
	{
	  if(a_whichComponent>0) l_fromPtr += a_whichComponent*l_entriesPerComponent;
	  for(j=0;j<l_entriesPerComponent;j++) 
	    {
	      int l_pt = g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][l_pointsTimestep][j];
	      if(l_pt>=0 && l_pt<l_reducedNumber)
		{
		  l_entriesFound++;
		  a_data[l_pt] = l_fromPtr[0]; 
		}
	      l_fromPtr ++;
	    }
	}
      else if(a_whichComponent<0 && l_interleave==SAF_INTERLEAVE_VECTOR)
	{
	  if(a_interleave==SAF_INTERLEAVE_VECTOR)
	    {
	      for(j=0;j<l_entriesPerComponent;j++) 
		{
		  int l_pt = g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][l_pointsTimestep][j];
		  if(l_pt>=0 && l_pt<l_reducedNumber)
		    {
		      l_entriesFound++;
		      for(k=0;k<l_numComponents;k++) a_data[l_pt*l_numComponents+k] = l_fromPtr[k]; 
		    }
		  l_fromPtr += l_numComponents;		      
		}
	    }
	  else if(a_interleave==SAF_INTERLEAVE_COMPONENT||a_interleave==SAF_INTERLEAVE_INDEPENDENT)
	    {
	      for(j=0;j<l_entriesPerComponent;j++) 
		{
		  int l_pt = g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][l_pointsTimestep][j];
		  if(l_pt>=0 && l_pt<l_reducedNumber)
		    {
		      l_entriesFound++;
		      for(k=0;k<l_numComponents;k++) a_data[l_pt+k*l_entriesPerComponent] = l_fromPtr[k]; 
		    }
		  l_fromPtr += l_numComponents;		      
		}
	    }
	  else 
	    {
	      if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	      if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	      printinfo("error read_unstr_variable_internal l_numComponents=%d, bad interleave argument\n",l_numComponents);
	      return(-1);
	    }
	}
      else if(a_whichComponent<0 && (l_interleave==SAF_INTERLEAVE_COMPONENT||l_interleave==SAF_INTERLEAVE_INDEPENDENT))
	{
	  if(a_interleave==SAF_INTERLEAVE_VECTOR)
	    {
	      for(j=0;j<l_entriesPerComponent;j++) 
		{
		  int l_pt = g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][l_pointsTimestep][j];
		  if(l_pt>=0 && l_pt<l_reducedNumber)
		    {
		      l_entriesFound++;
		      for(k=0;k<l_numComponents;k++) a_data[l_pt*l_numComponents+k] = l_tempData[k*l_entriesPerComponent+j]; 
		    }
		}
	    }
	  else if(a_interleave==SAF_INTERLEAVE_COMPONENT||a_interleave==SAF_INTERLEAVE_INDEPENDENT)
	    {
	      for(j=0;j<l_entriesPerComponent;j++) 
		{
		  int l_pt = g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][l_pointsTimestep][j];
		  if(l_pt>=0 && l_pt<l_reducedNumber)
		    {
		      l_entriesFound++;
		      for(k=0;k<l_numComponents;k++) a_data[l_pt+k*l_entriesPerComponent] = l_tempData[k*l_entriesPerComponent+j]; 
		    }
		}
	    }
	  else 
	    {
	      printinfo("error read_unstr_variable_internal l_numComponents=%d, bad interleave argument\n",l_numComponents);
	      if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	      if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	      return(-1);
	    }
	      
	}
      else 
	{
	  printinfo("error read_unstr_variable_internal l_numComponents=%d, bad interleave\n",l_numComponents);
	  if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	  if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	  return(-1);
	}


      if( l_entriesFound != a_howManyEntriesIExpect )
	{ /*not fatal, for now. Might just be poor data?*/

	  static int l_alreadyBeenHere=0;
	  if( !l_alreadyBeenHere )
	    {
	      printinfo("\n");
	      printinfo("warning read_unstr_variable_internal l_entriesFound=%d l_reducedNumber=%d a_howManyEntriesIExpect=%d l_entriesPerComponent=%d\n",
			l_entriesFound, l_reducedNumber, a_howManyEntriesIExpect,l_entriesPerComponent );
	      printinfo("       NOT GOING TO PRINT ANY MORE warnings LIKE THIS\n");
	      printinfo("\n");
	      l_alreadyBeenHere=1;
	    }

	}

      if(l_tempData) { free(l_tempData); l_tempData=NULL; }
      if(l_truthData) { free(l_truthData); l_truthData=NULL; }
      return(0);
    }
  else
#endif /*USE_CONNECTIVITY_REDUCTION_AND_CACHE*/

    /******************READ A POINT VAR THAT IS ON THE NODESET************************/
    /******************READ AN ELEMENT VAR THAT IS ON THE PARTSET************************/
    if(  (!l_isPointVar && (SAF_EQUIV(&l_fieldSet,&l_partSet) || l_gotSubsetData)) ||
	 ( l_isPointVar && (SAF_EQUIV(&l_fieldSet,&l_nodeSet) || l_gotSubsetData))  )
      {
	/*This is the simplest case. We are either reading a point var on the nodeset or
	  reading an element var on the part set. There will be no subset or topo issues.
	  Any nodal subset issues will have been  handled already in 
	  get_unstr_point_coords_used_by_part*/
 
	if( l_count!= (size_t)a_howManyEntriesIExpect*l_numComponents )
	  {
	    /*If this happens, I think the file must be bad*/
	    printinfo("   read_unstr_variable_internal error l_count=%d a_howManyEntriesIExpect=%d partset=%d nodeset=%d\n",
		      (int)l_count, a_howManyEntriesIExpect,l_partSetIndex, l_nodeSetIndex );
	    if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	    if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	    return(-1);
	  }
	if( !a_data )
	  {
	    if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	    if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	    return(0);/* just a dry run: return success */
	  }


	if( l_numComponents==1 || a_whichComponent<0 )
	  {
	    if(l_gotSubsetData)
	      {
		memcpy(a_data,l_tempData,l_count*sizeof(MY_PRECISION));
	      }
	    else 
	      {
		if( read_whole_field_to_my_precision(&l_field,l_varType,l_count,a_data) )
		  {
		    printinfo("    read_unstr_variable_internal error failed simple field reading\n");
		    return(-1);
		  }
	      }
	    if(a_whichComponent<0 && l_interleave==SAF_INTERLEAVE_VECTOR && (a_interleave==SAF_INTERLEAVE_COMPONENT
									     ||a_interleave==SAF_INTERLEAVE_INDEPENDENT) )
	      {
		change_from_vector_to_component_interleave(l_numComponents,(size_t)l_entriesPerComponent,a_data);
	      }
	    else if(a_whichComponent<0 && 
		    (l_interleave==SAF_INTERLEAVE_COMPONENT||l_interleave==SAF_INTERLEAVE_INDEPENDENT)
		    && a_interleave==SAF_INTERLEAVE_VECTOR)
	      {
		change_from_component_to_vector_interleave(l_numComponents,(size_t)l_entriesPerComponent,a_data);
	      }
	  }
	else
	  {
	    MY_PRECISION *l_fromPtr;

	    if(!l_gotSubsetData)
	      {
		l_tempData=(MY_PRECISION *)malloc(l_count*sizeof(MY_PRECISION));
		if( read_whole_field_to_my_precision(&l_field,l_varType,l_count,l_tempData) )
		  {
		    printinfo("    read_unstr_variable_internal error failed simple field reading\n");
		    if(l_tempData) { free(l_tempData); l_tempData=NULL; }
		    if(l_truthData) { free(l_truthData); l_truthData=NULL; }
		    return(-1);
		  }
	      }

	    l_fromPtr=l_tempData;
	    if(a_whichComponent>=0 && l_interleave==SAF_INTERLEAVE_VECTOR)
	      {
		for(j=0;j<l_entriesPerComponent;j++) 
		  {
		    a_data[j] = l_fromPtr[a_whichComponent]; 
		    l_fromPtr += l_numComponents;
		  }
	      }
	    else if(a_whichComponent>=0 && (l_interleave==SAF_INTERLEAVE_COMPONENT||l_interleave==SAF_INTERLEAVE_INDEPENDENT))
	      {
		l_fromPtr += a_whichComponent*l_entriesPerComponent;
		memcpy(a_data,l_fromPtr,l_entriesPerComponent*sizeof(MY_PRECISION));
	      }
	    else 
	      {
		printinfo("error read_unstr_variable_internal l_numComponents=%d, bad interleave\n",l_numComponents);
		if(l_tempData) { free(l_tempData); l_tempData=NULL; }
		if(l_truthData) { free(l_truthData); l_truthData=NULL; }
		return(-1);
	      }

#if 0
	    printinfo("final data :");
	    for(j=0;j<l_entriesPerComponent;j++) 
	      {
		printinfo(" %.1f",a_data[j]);
	      }
	    printinfo("\n");
#endif


	  }





	if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	return(0);

      }
  /******************READ A POINT VAR THAT IS <NOT> ON THE NODESET WITH NO SUBSET REL************************/
    else if( l_isPointVar  )
      {
	printinfo("    read_unstr_variable_internal cant handle POINT VAR NOT ON THE NODESET WITH NO SUBSET REL\n");
	if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	return(-1);	
      }
  /******************READ AN ELEMENT VAR THAT IS <NOT> ON THE PARTSET WITH NO SUBSET REL************************/
    else 
      {
	printinfo("    read_unstr_variable_internal cant handle ELEM VAR NOT ON THE PARTSET WITH NO SUBSET REL\n");
	if(l_tempData) { free(l_tempData); l_tempData=NULL; }
	if(l_truthData) { free(l_truthData); l_truthData=NULL; }
	return(-1);
      }
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Read a Vector Variable
 *
 * Description: 
 * Reads a vector variable into the array a_data, which must be preallocated to size a_howManyEntriesIExpect.
 * If a_howManyEntriesIExpect is not correct for the field, there will be an error. The variable instance
 * is specified by the given part-variable-timestep triplet.
 *
 *--------------------------------------------------------------------------------------------------- */


int 
UNSTR_CLASS_NAME_IF_CPP
read_unstr_vector_variable_entire( int a_whichPart,
				   int a_whichVar,
				   int a_whichTimestep,
				   MY_PRECISION *a_data,
				   int a_howManyEntriesIExpect )
{
  return( read_unstr_variable_internal(  a_whichPart,
					 a_whichVar,
					 a_whichTimestep,
					 a_data,
					 a_howManyEntriesIExpect,
					 -1,
					 SAF_INTERLEAVE_VECTOR
					 ) );
}



int 
UNSTR_CLASS_NAME_IF_CPP
read_unstr_vector_variable( int a_whichPart,
			    int a_whichVar,
			    int a_whichTimestep,
			    int a_whichComponent,
			    MY_PRECISION *a_data,
			    int a_howManyEntriesIExpect )
{
  return( read_unstr_variable_internal(  a_whichPart,
					 a_whichVar,
					 a_whichTimestep,
					 a_data,
					 a_howManyEntriesIExpect,
					 a_whichComponent,
					 SAF_INTERLEAVE_NONE
					 ) );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Read a Symtensor Variable
 *
 * Description: 
 * Reads a symtensor variable into the array a_data, which must be preallocated to size a_howManyEntriesIExpect.
 * If a_howManyEntriesIExpect is not correct for the field, there will be an error. The variable instance
 * is specified by the given part-variable-timestep triplet.
 *
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
read_unstr_symtensor_variable( int a_whichPart,
			       int a_whichVar,
			       int a_whichTimestep,
			       int a_whichComponent,
			       MY_PRECISION *a_data,
			       int a_howManyEntriesIExpect )
{
  return( read_unstr_variable_internal(  a_whichPart,
					 a_whichVar,
					 a_whichTimestep,
					 a_data,
					 a_howManyEntriesIExpect,
					 a_whichComponent,
					 SAF_INTERLEAVE_NONE
					 ) );


#if 0 /*2feb2004 XXX Note: this section was in the old version: need to determine what to do with this*/
      if( l_numComponents == 3 )
	{
	  printinfo("    warning translating 2d symtensor to 3d\n");
	  /*this is a 2d symtensor (as in larry1w): need to translate it to 3d tensor for ensight*/
	  if(a_whichComponent==0) a_whichComponent=0;
	  else if(a_whichComponent==1) a_whichComponent=1;
	  else if(a_whichComponent==3) a_whichComponent=2;
	  else a_whichComponent=-1;
	}
#endif

}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Read a Tensor Variable
 *
 * Description: 
 * Reads a tensor variable into the array a_data, which must be preallocated to size a_howManyEntriesIExpect.
 * If a_howManyEntriesIExpect is not correct for the field, there will be an error. The variable instance
 * is specified by the given part-variable-timestep triplet.
 *
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
read_unstr_tensor_variable( int a_whichPart,
				int a_whichVar,
				int a_whichTimestep,
				int a_whichComponent,
				MY_PRECISION *a_data,
				int a_howManyEntriesIExpect )
{
  return( read_unstr_variable_internal(  a_whichPart,
					 a_whichVar,
					 a_whichTimestep,
					 a_data,
					 a_howManyEntriesIExpect,
					 a_whichComponent,
					 SAF_INTERLEAVE_NONE
					 ) );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is this Variable a Coordinate? CURRENTLY DOESNT APPLY!
 *
 * Description: 
 *
 * Returns 1 if this var is made up of coordinate fields (the first field, at least).
 * This would imply (usually?) that the field represents the displacement from
 * the time-independent default coordinates. CURRENTLY THIS WILL ALWAYS RETURN 0, BECAUSE
 * can_this_field_be_a_variable IS SET TO NEVER CALL A COORD FIELD A VARIABLE.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_a_coordinate_var( int a_whichVar )
{
  int i,j;
  for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
    {
      if(!g_unstrGlobals->m_instancePerTimestepPerVarPerPart)
	{
	  printinfo("error is_this_a_coordinate_var: instances not allocated\n");
	  exit(-1);
	}
      for(j=0;j<get_num_timesteps();j++)
	{
	  if( a_whichVar>=0 && a_whichVar<g_unstrGlobals->m_maxNumUnstrVars )
	    {
	      SAF_Field l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][a_whichVar][j];
	      if( my_saf_is_valid_field_handle(&l_field) )
		{
		  hbool_t l_isCoordField;
		  saf_describe_field(SAF_ALL, &l_field, NULL, NULL, NULL, NULL,
				     &l_isCoordField, NULL, NULL, NULL, NULL, NULL, NULL,  
				     NULL, NULL, NULL, NULL);		  
		  if( l_isCoordField )
		    return(1);
		  else
		    return(0);
		}
	    }
	}
    }
  return(0);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Is this Unstructured Variable Mapped to Nodes?
 *
 * Description: 
 *
 * Finds the first instance of this variable and checks the collection. 
 *
 * Return: 1 if true, 0 if false, -1 for error
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_a_nodal_unstr_var( int a_whichVar )
{
  int i,k;
  if( a_whichVar<0 || a_whichVar>=g_unstrGlobals->m_maxNumUnstrVars )
    {
      printinfo("is_a_nodal_unstr_var error, a_whichVar=%d is out of range\n",a_whichVar);
      return(-1);
    }
  for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
    {
      if( a_whichVar < g_unstrGlobals->m_maxNumUnstrVars )
	{
	  for(k=0;k<get_num_timesteps();k++)
	    {
	      SAF_Field l_field = g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][a_whichVar][k];
	      if( my_saf_is_valid_field_handle(&l_field) )
		{
		  SAF_Set l_set;
		  SAF_Cat l_cat;
		  SAF_CellType l_cellType;
		  saf_describe_field(SAF_ALL, &l_field,  NULL, NULL, &l_set, NULL,
				     NULL, NULL, &l_cat, NULL, NULL, NULL, NULL,NULL,
				     NULL, NULL, NULL);
		  saf_describe_collection(SAF_ALL,&l_set,&l_cat,&l_cellType,NULL,NULL,NULL,NULL);
		  if( l_cellType==SAF_CELLTYPE_POINT )
		    return(1);
		  else
		    return(0);
		}
	    }
	}
    }
  printinfo("is_a_nodal_unstr_var error: never found a field: var=%d numvars=%d numparts=%d numtimes=%d\n",
	a_whichVar,g_unstrGlobals->m_maxNumUnstrVars,g_unstrGlobals->m_numUnstrParts,get_num_timesteps() );
  return(-1);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of Unstructured Parts
 *
 * Description: 
 *
 * If a set passes the is_this_a_valid_unstr_set test, then it is considered a "part". Note
 * that init_unstr_mesh_reader must have already been called.
 *
 * Return: the number of parts in the set.
 * 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_unstr_parts()
{
  return(g_unstrGlobals->m_numUnstrParts);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Name of an Unstructured Part
 *
 * Description: 
 *
 * The name will be of the form "[set #]:[set name]". Up to a_maxlen characters of the name will be 
 * written to the preallocated array a_str.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_unstr_part_name( int a_whichPart, char *a_str, int a_maxlen )
{
  name_all_parts();
  snprintf(a_str,(size_t)a_maxlen,g_unstrGlobals->m_partNames[a_whichPart]);
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Find all Unstructured Parts in the Database
 *
 * Description: 
 * Makes a global array of the number of sets that pass the is_this_a_valid_unstr_set test.
 * Note that this function also allocates and fills several other global arrays. It should
 * only be called by init_unstr_mesh_reader.
 *
 *--------------------------------------------------------------------------------------------------- */
void 
UNSTR_CLASS_NAME_IF_CPP
get_all_unstr_sets( )
{
  int i,j,l_num;
  int *l_areValidUnstrSets=0;
  int l_numValid=0;

  g_unstrGlobals->m_numUnstrParts=0;

  printinfo("Beginning get_all_unstr_sets: check all sets, keep only those with points and elems.\n");

  g_unstrGlobals->m_numAllSets = get_matching_sets( &g_unstrGlobals->m_allSets, SAF_TOP_TORF,
						    SAF_ANY_TOPODIM, /*a_topoDim*/ /*XXX jake is this correct?*/
						    -1,/*a_numColl*/
						    -1, /*a_collDim*/
						    SAF_CELLTYPE_ANY, 0 );

  printinfo("There are %d SAF_SPACE sets in the database.\n",g_unstrGlobals->m_numAllSets);

  if( !g_unstrGlobals->m_numAllSets ) return;

  /* Find all the fields on each set for later use. */
  printinfo("Begin getting m_allSetsFields and m_allSetsFieldsTimestep\n");
  g_unstrGlobals->m_allSetsFields = (SAF_Field **)malloc( g_unstrGlobals->m_numAllSets*sizeof(SAF_Field *) );
  g_unstrGlobals->m_allSetsCoordFields = (SAF_Field **)malloc( g_unstrGlobals->m_numAllSets*sizeof(SAF_Field *) );
  g_unstrGlobals->m_allSetsFieldsTimestep = (int **)malloc( g_unstrGlobals->m_numAllSets*sizeof(int *) );
  g_unstrGlobals->m_allSetsFieldsVariable = (int **)malloc( g_unstrGlobals->m_numAllSets*sizeof(int *) );
  g_unstrGlobals->m_allSetsNumFields = (int *)malloc( g_unstrGlobals->m_numAllSets*sizeof(int) );
  g_unstrGlobals->m_allSetsNumCoordFields = (int *)malloc( g_unstrGlobals->m_numAllSets*sizeof(int) );






  /*just printing: print all cats*/
  /*{
    int l_numCatsUniverse=0;
    SAF_Cat *l_catsUniverse=NULL;
    #ifdef __cplusplus
    saf_find_categories(SAF_ALL,m_strReader->getDatabase(),SAF_UNIVERSE(m_strReader->getDatabase()),SAF_ANY_NAME,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&l_numCatsUniverse,&l_catsUniverse);
    #else
    saf_find_categories(SAF_ALL,getDatabase(),SAF_UNIVERSE(getDatabase()),SAF_ANY_NAME,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&l_numCatsUniverse,&l_catsUniverse);
    #endif
    printf("Found %d cats in universe.\n",l_numCatsUniverse);
    for(i=0;i<l_numCatsUniverse;i++)
    {
    char *l_catName=NULL,*l_roleName=NULL;
    int l_dim=0;
    SAF_Role l_role;
    saf_describe_category(SAF_ALL,&(l_catsUniverse[i]),&l_catName,&l_role,&l_dim);
    l_roleName = get_role_desc(l_role);
    printf("    i=%d  name=%s  dim=%d  role=%s\n",i,l_catName,l_dim,l_roleName);
    free(l_catName);free(l_roleName);
    }
    }*/

  {
    int l_numCats=0;
    SAF_Cat *l_cats=NULL;

#ifdef __cplusplus
    saf_find_categories(SAF_ALL,m_strReader->getDatabase(),SAF_UNIVERSE(m_strReader->getDatabase()),SAF_ANY_NAME,SAF_TOPOLOGY,SAF_ANY_TOPODIM,&l_numCats,&l_cats);
#else
    saf_find_categories(SAF_ALL,getDatabase(),SAF_UNIVERSE(getDatabase()),SAF_ANY_NAME,SAF_TOPOLOGY,SAF_ANY_TOPODIM,&l_numCats,&l_cats);
#endif

    /*printf("Found %d SAF_TOPOLOGY cats in universe.\n",l_numCats);*/

    /*Note the "stategroups" cat is SAF_TOPOLOGY, dim 0. Need to avoid building vars from it.*/
    for(i=0;i<l_numCats;i++)
      {
	char *l_catName=NULL;
	int l_dim=0;
	saf_describe_category(SAF_ALL,&(l_cats[i]),&l_catName,NULL,&l_dim);
	if( !strcmp(l_catName,"stategroups") && !l_dim )
	  {
	    /*printf("removing stategroups category\n");*/
	    for(j=i;j<l_numCats-1;j++) l_cats[j] = l_cats[j+1];
	    l_numCats--;
	  }
	free(l_catName);
      }

    /*sort by ascending dim*/
    {
      int l_didSomething=1;
      while(l_didSomething)
	{
	  l_didSomething=0;
	  for(i=0;i<l_numCats-1;i++)
	    {
	      int l_dim1=0, l_dim2=0;
	      saf_describe_category(SAF_ALL,&(l_cats[i]),NULL,NULL,&l_dim1);
	      saf_describe_category(SAF_ALL,&(l_cats[i+1]),NULL,NULL,&l_dim2);
	      if( l_dim1 > l_dim2 )
		{
		  SAF_Cat l_temp = l_cats[i];
		  l_cats[i] = l_cats[i+1];
		  l_cats[i+1] = l_temp;
		  l_didSomething=1;
		}
	    }
	}
    }

    /*just printing info*/
    /*printf("After removing stategroups cat, now have %d SAF_TOPOLOGY cats in universe.\n",l_numCats);
      for(i=0;i<l_numCats;i++)
      {
      SAF_Role l_role;
      char *l_catName=NULL,*l_roleName=NULL;
      int l_dim=0;
      saf_describe_category(SAF_ALL,&(l_cats[i]),&l_catName,&l_role,&l_dim);
      l_roleName = get_role_desc(l_role);
      printf("    i=%d  name=%s  dim=%d  role=%s\n",i,l_catName,l_dim,l_roleName);
      free(l_catName);free(l_roleName);
      }*/

    if(!l_numCats)
      {
	printinfo("error no SAF_Cat's with role SAF_TOPOLOGY\n");
	exit(-1);
      }

    g_unstrGlobals->m_subsetTransformsNumCats=l_numCats;
    g_unstrGlobals->m_subsetTransforms=(struct_subset_transform **)malloc(l_numCats*sizeof(struct_subset_transform*));	    
    for(i=0;i<l_numCats;i++)
      {
	g_unstrGlobals->m_subsetTransforms[i]=(struct_subset_transform *)malloc(g_unstrGlobals->m_numAllSets*sizeof(struct_subset_transform));
	for(j=0;j<g_unstrGlobals->m_numAllSets;j++)
	  {
	    init_subset_transform( &g_unstrGlobals->m_subsetTransforms[i][j] );
	    g_unstrGlobals->m_subsetTransforms[i][j].m_cat = l_cats[i];
	  }
      }	

  }



  for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
    {
      g_unstrGlobals->m_allSetsFields[i]=0;
      g_unstrGlobals->m_allSetsCoordFields[i]=0;
      g_unstrGlobals->m_allSetsFieldsTimestep[i]=0;
      g_unstrGlobals->m_allSetsFieldsVariable[i]=0;
      g_unstrGlobals->m_allSetsNumFields[i]=0;
      g_unstrGlobals->m_allSetsNumCoordFields[i]=0;

#ifdef __cplusplus
      saf_find_fields(SAF_ALL,m_strReader->getDatabase(), &(g_unstrGlobals->m_allSets[i]), SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, 
		      SAF_ANY_BASIS, SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, 
		      &g_unstrGlobals->m_allSetsNumFields[i], &g_unstrGlobals->m_allSetsFields[i]);
      saf_find_coords(SAF_ALL,m_strReader->getDatabase(),&(g_unstrGlobals->m_allSets[i]),&g_unstrGlobals->m_allSetsNumCoordFields[i], 
		      &g_unstrGlobals->m_allSetsCoordFields[i]);
#else
      saf_find_fields(SAF_ALL,getDatabase(), &(g_unstrGlobals->m_allSets[i]), SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, 
		      SAF_ANY_BASIS, SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, 
		      &g_unstrGlobals->m_allSetsNumFields[i], &g_unstrGlobals->m_allSetsFields[i]);
      saf_find_coords(SAF_ALL,getDatabase(),&(g_unstrGlobals->m_allSets[i]),&g_unstrGlobals->m_allSetsNumCoordFields[i], 
		      &g_unstrGlobals->m_allSetsCoordFields[i]);
#endif


      if( g_unstrGlobals->m_allSetsNumFields[i] )
	{
	  g_unstrGlobals->m_allSetsFieldsTimestep[i]=(int *)malloc(g_unstrGlobals->m_allSetsNumFields[i]*sizeof(int));
	  g_unstrGlobals->m_allSetsFieldsVariable[i]=(int *)malloc(g_unstrGlobals->m_allSetsNumFields[i]*sizeof(int));
	  for(j=0;j<g_unstrGlobals->m_allSetsNumFields[i];j++)
	    {
	      g_unstrGlobals->m_allSetsFieldsTimestep[i][j]=-2;/*-2 means "cannot be a variable"*/
	      g_unstrGlobals->m_allSetsFieldsVariable[i][j]=-2;/*-2 means "cannot be a variable"*/

	      if( can_this_field_be_a_variable(g_unstrGlobals->m_allSets[i],g_unstrGlobals->m_allSetsFields[i][j]) )
		{
		  g_unstrGlobals->m_allSetsFieldsTimestep[i][j]=get_timestep_for_field(g_unstrGlobals->m_allSetsFields[i][j]);
		}
	    }
	}




      /* dont want to limit this
	 if( g_unstrGlobals->m_allSetsNumCoordFields[i] )
      */

      for(j=0;j<g_unstrGlobals->m_subsetTransformsNumCats;j++)
	{
#ifdef __cplusplus
	  get_subset_transforms( *(m_strReader->getDatabase()), g_unstrGlobals->m_allSets[i], 
				 g_unstrGlobals->m_subsetTransforms[j][i].m_cat,
				 &g_unstrGlobals->m_subsetTransforms[j][i] );
#else
	  get_subset_transforms( *(getDatabase()), g_unstrGlobals->m_allSets[i], 
				 g_unstrGlobals->m_subsetTransforms[j][i].m_cat,
				 &g_unstrGlobals->m_subsetTransforms[j][i] );
#endif
	}

#if 0 /*printing only*/
      for(j=0;j<g_unstrGlobals->m_subsetTransformsNumCats;j++)
	{
	  if( g_unstrGlobals->m_subsetTransforms[j][i].m_length > 1 )
	    {
	      int k;
	      printinfo("\nSubset transforms for set %s cat %s\n",get_set_name(g_unstrGlobals->m_allSets[i]),
			get_cat_name(g_unstrGlobals->m_subsetTransforms[j][i].m_cat) );
	      for(k=1;k<g_unstrGlobals->m_subsetTransforms[j][i].m_length;k++)
		{
		  printinfo("          %s\n", get_set_name(g_unstrGlobals->m_subsetTransforms[j][i].m_sets[k]) );
		}
	    }
	}
      if( i==g_unstrGlobals->m_numAllSets-1 ) printf("\n");
#endif








    }
  printinfo("Finished getting m_allSetsFields and m_allSetsFieldsTimestep\n");



  /*
  ** Check each set to determine if it is a valid unstructured set, i.e. has
  ** points or elems.
  */
  l_areValidUnstrSets = (int *)malloc(g_unstrGlobals->m_numAllSets*sizeof(int));
  if(!l_areValidUnstrSets)
    {
      printinfo("get_all_unstr_sets error allocating l_areValidUnstrSets\n");
      exit(-1);
    }
  for( i=0; i<g_unstrGlobals->m_numAllSets; i++ )
    {
      l_areValidUnstrSets[i] = is_this_a_valid_unstr_set( g_unstrGlobals->m_allSets[i], NULL );
      if(l_areValidUnstrSets[i]) l_numValid++;
    }
  if( !l_numValid ) 
    {
      free( l_areValidUnstrSets );
      return;
    }




  /*
  ** Finally, create the array that links parts to set indices.
  */
  g_unstrGlobals->m_numUnstrParts = l_numValid;
  g_unstrGlobals->m_setIndexPerPart = (int *)malloc(g_unstrGlobals->m_numUnstrParts *sizeof(int));
  if(!g_unstrGlobals->m_setIndexPerPart)
    {
      printinfo("get_all_unstr_sets error allocating m_setIndexPerPart\n");
      exit(-1);
    }
  l_num=0;
  for( i=0; i<g_unstrGlobals->m_numAllSets; i++ )
    {
      if(l_areValidUnstrSets[i]) 
	{
	  g_unstrGlobals->m_setIndexPerPart[l_num++] = i;
	}
    }
  free( l_areValidUnstrSets );
  printinfo("Finished get_all_unstr_sets: keeping %d of %d sets.\n",l_numValid,g_unstrGlobals->m_numAllSets);



 
  return;
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Topo Relations of a Given Celltype on a Given Set
 *
 * Description: 
 *
 * This function returns the number of self-stored(?) topo relations on the set that 
 * have a range set collection SAF_CellType of SAF_CELLTYPE_POINT, a storage decomposition
 * SAF_CellType of SAF_CELLTYPE_SET, and a containing set collection SAF_CellType of
 * a_cellType (or at least some members of the collection match a_cellType). THIS IS
 * A VERY SMALL SUBSET OF ALL TOPO REL POSSIBILITIES, BUT IT SEEMS TO COVER THE MOST
 * TYPICAL USE OF A TOPO RELATION: TO CONNECT ELEMENTS ON THIS SET TO NODES ON ANOTHER
 * SET
 *
 * If a valid pointer for a_totalNumElems is provided, then this function will set 
 * a_totalNumElems[0] equal to the number of elements defined (that match a_cellType) 
 * by all the accepted relations.
 *
 * If a valid pointer for a_rels is provided, then a_rels[0] will be allocated and
 * set as the list of relations that pass the requirements.
 *
 * Note there are some comments about SAF_ARBITRARY, I dont think it is handled well
 * here or in other functions (not prepared for multiple types of elements). Havent
 * had any data to test it, either.
 *
 * Currently only handles SAF_INT.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_topo_rels_in_set( SAF_Set a_set, SAF_CellType a_cellType, int *a_totalNumElems, SAF_Rel **a_rels,
			  int a_isQuadraticCellType )
{
  int j,k;
  int l_numTopoRels=0;
  int l_totalNumElems=0;
  SAF_Rel *l_rels=0;
  int l_numRels=0;
  SAF_Rel *l_relList=0;

  /*printinfo(" get_num_topo_rels_in_set type=%s(%d)  %p %p  isQuadratic=%d\n",
    get_celltype_desc(a_cellType),a_cellType,a_totalNumElems,a_rels,a_isQuadraticCellType);*/

#ifdef USE_TOPO_RELS_CACHE
  /*Check if this function has already been called with these inputs, and
    if so, return the same results found before*/
  {
    int l_which=-1,i;
    for(i=0;i<g_unstrGlobals->m_topoRelsCacheSize;i++)
      {
	if( SAF_EQUIV(&a_set,get_set_of_index(g_unstrGlobals->m_topoRelsCacheSetRows[i])) &&
	    a_cellType==g_unstrGlobals->m_topoRelsCacheCelltypes[i] &&
	    a_isQuadraticCellType==g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic[i] )
	  {
	    l_which=i;
	    break;
	  }
      }
    if(l_which>=0)
      {
	if(a_rels)
	  {
	    if(!g_unstrGlobals->m_topoRelsCacheNumRels[l_which])
	      {
		a_rels[0]=NULL;/*no relations were found, so clear the pointer*/
	      }
	    else
	      {
		a_rels[0] = (SAF_Rel *)malloc( g_unstrGlobals->m_topoRelsCacheNumRels[l_which]*sizeof(SAF_Rel) );
		if(!a_rels[0])
		  {
		    printinfo("error get_num_topo_rels_in_set failed to alloc a_rels[0] to size %d bytes\n",
			      g_unstrGlobals->m_topoRelsCacheNumRels[l_which]*sizeof(SAF_Rel) );
		    exit(-1);
		  }
		memcpy(a_rels[0],g_unstrGlobals->m_topoRelsCacheRels[l_which],
		       g_unstrGlobals->m_topoRelsCacheNumRels[l_which]*sizeof(SAF_Rel));
	      }
	  }
	if(a_totalNumElems) a_totalNumElems[0]=g_unstrGlobals->m_topoRelsCacheNumElems[l_which];

	/*printinfo(" get_num_topo_rels_in_set got from cache %d: %s    %d %d\n",l_which,
	  get_celltype_desc(a_cellType),  g_unstrGlobals->m_topoRelsCacheNumRels[l_which], g_unstrGlobals->m_topoRelsCacheNumElems[l_which] );*/

	return( g_unstrGlobals->m_topoRelsCacheNumRels[l_which] );
      }
  }
#endif




  if(a_totalNumElems) a_totalNumElems[0]=0;

#ifdef __cplusplus
  saf_find_topo_relations(SAF_ALL, m_strReader->getDatabase(),&a_set, NULL, &l_numRels, &l_rels );
#else
  saf_find_topo_relations(SAF_ALL, getDatabase(),&a_set, NULL, &l_numRels, &l_rels );
#endif
  if( l_numRels<=0 ) return(0);


#ifndef USE_TOPO_RELS_CACHE
  if( a_rels ) 
#endif
    {
      l_relList=(SAF_Rel *)malloc(l_numRels*sizeof(SAF_Rel));
      if(!l_relList)
	{
	  printinfo("error allocating l_relList\n");
	  exit(-1);
	}      
    }
      
  for( j=0; j<l_numRels; j++ )
    {
      SAF_Set l_containingSet=SS_SET_NULL;
      SAF_Cat l_collOfPiecesSewnTogether=SS_CAT_NULL;
      SAF_Set l_rangeSet=SS_SET_NULL;
      SAF_Cat l_rangeColl=SS_CAT_NULL;
      SAF_Cat l_collOfSetWhereRelActuallyStored=SS_CAT_NULL;
      SAF_RelRep l_topoRelType;
      hid_t l_dataType;
      SAF_CellType l_rangeCellType;
      SAF_IndexSpec l_rangeIndexSpec;
      int l_numEntriesInRangeColl=0;
      SAF_CellType l_containerCellType;
      SAF_IndexSpec l_containerIndexSpec;
      int l_numEntriesInContainerColl=0;
      SAF_CellType l_storedCellType;
      SAF_IndexSpec l_storedIndexSpec;
      int l_numEntriesInStoredColl=0;
      int l_rangeSetTopoDim,l_containerSetTopoDim;
      hbool_t l_isSelfStored;
      size_t l_abuf_sz=0,l_bbuf_sz=0;


      saf_is_self_stored_topo_relation(SAF_ALL,&(l_rels[j]),&l_isSelfStored);
      if( !l_isSelfStored )
	{
	  continue;
	}

      saf_describe_topo_relation(SAF_ALL,&(l_rels[j]),&l_containingSet,&l_collOfPiecesSewnTogether,
				 &l_rangeSet,&l_rangeColl,&l_collOfSetWhereRelActuallyStored,
				 &l_topoRelType, NULL );



#if 1 /*Looking for error with saf_describe_topo_relation. Exiting harshly if found.*/
      if(!my_saf_is_valid_set_handle(&l_containingSet) )
	{
	  printinfo("   l_containingSet is SS_SET_NULL\n"); exit(-1);
	}
      if(!my_saf_is_valid_set_handle(&l_rangeSet) )
	{
	  printinfo("   l_rangeSet is SS_SET_NULL\n"); exit(-1);
	}
      if( SAF_EQUIV(&l_collOfPiecesSewnTogether, &(SS_CAT_NULL)) )
	{
	  printinfo("   l_collOfPiecesSewnTogether is SS_CAT_NULL\n"); exit(-1);
	}
      if( SAF_EQUIV(&l_rangeColl, &(SS_CAT_NULL)) )
	{
	  printinfo("   l_rangeColl is SS_CAT_NULL\n"); exit(-1);
	}
      if( SAF_EQUIV(&l_collOfSetWhereRelActuallyStored, &(SS_CAT_NULL)) )
	{
	  printinfo("   l_collOfSetWhereRelActuallyStored is SS_CAT_NULL\n"); exit(-1);
	}
#endif


      /*added this check with saf 1.5.3...not sure if it was because larry1w was not
	finished with the new states&suites, or whether this would actually happen,
	but it seems like a good idea anyway*/
      saf_get_count_and_type_for_topo_relation(SAF_ALL,&(l_rels[j]),NULL,NULL,&l_abuf_sz,NULL,&l_bbuf_sz,NULL);


      if( (l_abuf_sz || l_bbuf_sz) && 
	  (SAF_EQUIV(&l_topoRelType,SAF_UNSTRUCTURED) || SAF_EQUIV(&l_topoRelType,SAF_ARBITRARY))  ) 
	{
	  char *l_rangeSetName=0,*l_containerSetName=0;

	  saf_describe_topo_relation(SAF_ALL,&(l_rels[j]),NULL,NULL,NULL,NULL,NULL,NULL,&l_dataType);
	  
	  saf_describe_set(SAF_ALL, &l_rangeSet, &l_rangeSetName, &l_rangeSetTopoDim, NULL, NULL, NULL, NULL, NULL );
	  saf_describe_set(SAF_ALL,&l_containingSet,&l_containerSetName,&l_containerSetTopoDim,NULL,
			   NULL,NULL,NULL,NULL);
	  saf_describe_collection(SAF_ALL,&l_rangeSet,&l_rangeColl,&l_rangeCellType,&l_numEntriesInRangeColl,
				  &l_rangeIndexSpec,NULL,NULL);
	  saf_describe_collection(SAF_ALL,&l_containingSet,&l_collOfPiecesSewnTogether,&l_containerCellType,
				  &l_numEntriesInContainerColl, &l_containerIndexSpec,NULL,NULL);
	  saf_describe_collection(SAF_ALL,&a_set,&l_collOfSetWhereRelActuallyStored,&l_storedCellType,
				  &l_numEntriesInStoredColl, &l_storedIndexSpec,NULL,NULL);
	  
	  if( l_rangeCellType==SAF_CELLTYPE_POINT && 
	      l_storedCellType==SAF_CELLTYPE_SET && !H5Tequal(l_dataType,SAF_INT) )
	    {
	      printinfo("\nwarning: get_num_topo_rels_in_set found datatype %s, but currently only handles SAF_INT\n",
			get_dsl_type_string(l_dataType) );
	    }

	  if( l_rangeCellType==SAF_CELLTYPE_POINT && 
	      l_storedCellType==SAF_CELLTYPE_SET && H5Tequal(l_dataType,SAF_INT) )
	    {

	      if( SAF_EQUIV(&l_topoRelType,SAF_UNSTRUCTURED) && 
		  (l_containerCellType==a_cellType || a_cellType==SAF_CELLTYPE_ANY) )
		{
		   int l_okToRead=1;



	         /*These are the only cell types that vtk has quadratic versions of*/
    		 if( g_unstrGlobals->m_useQuadraticElements && a_cellType!=SAF_CELLTYPE_ANY &&
		    (a_cellType == SAF_CELLTYPE_LINE ||
		     a_cellType == SAF_CELLTYPE_TRI ||
		     a_cellType == SAF_CELLTYPE_QUAD ||
		     a_cellType == SAF_CELLTYPE_TET ||
		     a_cellType == SAF_CELLTYPE_HEX ||
		     a_cellType == SAF_CELLTYPE_PRISM ||
		     a_cellType == SAF_CELLTYPE_PYRAMID) ) 
		 {
 		  int *rabuf=NULL,*rbbuf=NULL;
		  int l_numPtsInType=get_num_nodes_in_celltype(a_cellType,a_isQuadraticCellType);

		  /*note: wasting a load of rbbuf here*/
		  saf_read_topo_relation(SAF_ALL, &(l_rels[j]), NULL,(void**) &rabuf, (void**) &rbbuf);
	  	  if( rabuf[0] != l_numPtsInType ) l_okToRead=0;

		  if(rabuf) free(rabuf);
		  if(rbbuf) free(rbbuf);
		 }



		  if(l_okToRead)
		  {
		    /*This is the most common setting. Any more complicated settings might cause trouble!*/
		    l_totalNumElems += l_numEntriesInContainerColl;
		    if(a_totalNumElems) a_totalNumElems[0] += l_numEntriesInContainerColl;
		    if( l_relList ) l_relList[l_numTopoRels]=l_rels[j];
		    l_numTopoRels++;
		  }
		}
	      else if( SAF_EQUIV(&l_topoRelType,SAF_ARBITRARY) && a_cellType==SAF_CELLTYPE_ANY )
		{
		  printinfo("\nwarning: found SAF_ARBITRARY get_num_topo_rels_in_set: not handled well! (SAF_CELLTYPE_ANY)\n\n");
		  l_totalNumElems += l_numEntriesInContainerColl;
		  if(a_totalNumElems) a_totalNumElems[0] += l_numEntriesInContainerColl;
		  if( l_relList )  l_relList[l_numTopoRels]=l_rels[j];
		  l_numTopoRels++;
		}
	      else if( SAF_EQUIV(&l_topoRelType,SAF_ARBITRARY) && a_cellType!=SAF_CELLTYPE_ANY )
		{
		  /*count how many of the arbitrary polys have the same number
		    of points as a_cellType*/
		  int l_numPtsInType=-1,l_numFound=0;
		  int *rabuf=0,*rbbuf=0;

		  printinfo("\nwarning: found SAF_ARBITRARY get_num_topo_rels_in_set: not handled well! (!=SAF_CELLTYPE_ANY)\n\n");

		  /*Note this is flawed, you cant decide the celltype only by the number
		    of points. Isnt there a way to read the actual celltype for individual entries?*/
		  l_numPtsInType=get_num_nodes_in_celltype(a_cellType,a_isQuadraticCellType);
		  if(l_numPtsInType<=0)
		    {
		      printinfo("error get_num_topo_rels_in_set: not prepared for a_cellType=%s\n",
				get_celltype_desc(a_cellType) );
		    }

		  saf_read_topo_relation(SAF_ALL, &(l_rels[j]), NULL,(void**) &rabuf, (void**) &rbbuf);
	 
		/*jake here  warning quadratic arbitrary elements not handled here yet! 26feb2004*/


		  /*printinfo("\n=====================================\n");
		    printinfo("     rabuf[%d]: ",(int)l_abuf_sz);
		    for(k=0;k<l_abuf_sz;k++) printinfo("%d ",rabuf[k]);
		    printinfo("\n");
		    printinfo("     rbbuf[%d]: ",(int)l_bbuf_sz);
		    for(k=0;k<l_bbuf_sz;k++) printinfo("%d ",rbbuf[k]);
		    printinfo("\n");
		    printinfo("=====================================\n\n");*/

		  for(k=0;k<(int)l_abuf_sz;k++)
		    {
		      if( l_numPtsInType == rabuf[k] ) l_numFound++;
		    }
		  if(l_numFound)
		    {
		      l_totalNumElems += l_numFound;
		      if(a_totalNumElems) a_totalNumElems[0] += l_numFound;
		      if( l_relList ) l_relList[l_numTopoRels]=l_rels[j];
		      l_numTopoRels++;
		    }
		}
	    }
	  else
	    {
	      printinfo("\nerror get_num_topo_rels_in_set: cant handle range=%s(%s), stored=%d, type=%s\n\n",
			get_celltype_desc(l_rangeCellType), l_rangeSetName, get_celltype_desc(l_storedCellType),
			get_dsl_type_string(l_dataType) );
	      /*dont exit, just continue as if that topo rel wasnt found*/
	    }

	  if(l_rangeSetName) free(l_rangeSetName);
	  if(l_containerSetName) free(l_containerSetName);
	}
    }









#ifdef USE_TOPO_RELS_CACHE
  /*Add the 2 inputs and 3 outputs to the cache*/
  {
    if( !g_unstrGlobals->m_topoRelsCacheMaxSize )
      {
	/* alloc cache for the first time: currently using only 9 different SAF_CELLTYPE_'s */
	g_unstrGlobals->m_topoRelsCacheMaxSize= (g_unstrGlobals->m_numAllSets*9>50) ? g_unstrGlobals->m_numAllSets*9:50;

	/*printinfo(" m_topoRelsCache alloc first time %d (g_numAllSets*9=%d)\n",
	  g_unstrGlobals->m_topoRelsCacheMaxSize,g_unstrGlobals->m_numAllSets*9);*/

	g_unstrGlobals->m_topoRelsCacheSetRows=(int *)malloc( g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(int) );
	g_unstrGlobals->m_topoRelsCacheCelltypes=(SAF_CellType *)malloc(g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(SAF_CellType));
	g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic=(int *)malloc(g_unstrGlobals->m_topoRelsCacheMaxSize*sizeof(int));

	g_unstrGlobals->m_topoRelsCacheNumRels=(int *)malloc( g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(int) );
	g_unstrGlobals->m_topoRelsCacheNumElems=(int *)malloc( g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(int) );
	g_unstrGlobals->m_topoRelsCacheRels=(SAF_Rel **)malloc(g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(SAF_Rel *));
      }
    else if( g_unstrGlobals->m_topoRelsCacheSize >= g_unstrGlobals->m_topoRelsCacheMaxSize )
      {
	/*resize the array*/
	int *l_newtopoRelsCacheNumElems=0;
	int *l_newtopoRelsCacheSetRows=0;
	SAF_CellType *l_newtopoRelsCacheCelltypes=0;
	int *l_newtopoRelsCacheCelltypeIsQuadratic=0;
	int *l_newtopoRelsCacheNumRels=0;
	SAF_Rel **l_newtopoRelsCacheRels=0;

	g_unstrGlobals->m_topoRelsCacheMaxSize *= 2;
	/*printinfo(" m_topoRelsCache resize cache, new size=%d\n",g_unstrGlobals->m_topoRelsCacheMaxSize);*/

	l_newtopoRelsCacheNumElems=(int *)malloc( g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(int) );
	l_newtopoRelsCacheSetRows=(int *)malloc( g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(int) );
	l_newtopoRelsCacheNumRels=(int *)malloc( g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(int) );
	l_newtopoRelsCacheCelltypes=(SAF_CellType *)malloc(g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(SAF_CellType));
	l_newtopoRelsCacheCelltypeIsQuadratic=(int *)malloc(g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(int));
	l_newtopoRelsCacheRels=(SAF_Rel **)malloc(g_unstrGlobals->m_topoRelsCacheMaxSize * sizeof(SAF_Rel *));

	memcpy(l_newtopoRelsCacheNumElems,g_unstrGlobals->m_topoRelsCacheNumElems,g_unstrGlobals->m_topoRelsCacheSize*sizeof(int));
	memcpy(l_newtopoRelsCacheSetRows,g_unstrGlobals->m_topoRelsCacheSetRows,g_unstrGlobals->m_topoRelsCacheSize*sizeof(int));
	memcpy(l_newtopoRelsCacheNumRels,g_unstrGlobals->m_topoRelsCacheNumRels,g_unstrGlobals->m_topoRelsCacheSize*sizeof(int));
	memcpy(l_newtopoRelsCacheCelltypes,g_unstrGlobals->m_topoRelsCacheCelltypes,g_unstrGlobals->m_topoRelsCacheSize*sizeof(SAF_CellType));
	memcpy(l_newtopoRelsCacheCelltypeIsQuadratic,g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic,g_unstrGlobals->m_topoRelsCacheSize*sizeof(int));
	memcpy(l_newtopoRelsCacheRels,g_unstrGlobals->m_topoRelsCacheRels,g_unstrGlobals->m_topoRelsCacheSize*sizeof(SAF_Rel *));

	free(g_unstrGlobals->m_topoRelsCacheNumElems);
	free(g_unstrGlobals->m_topoRelsCacheSetRows);
	free(g_unstrGlobals->m_topoRelsCacheNumRels);
	free(g_unstrGlobals->m_topoRelsCacheCelltypes);
	free(g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic);
	free(g_unstrGlobals->m_topoRelsCacheRels);

	g_unstrGlobals->m_topoRelsCacheNumElems=l_newtopoRelsCacheNumElems;
	g_unstrGlobals->m_topoRelsCacheSetRows=l_newtopoRelsCacheSetRows;
	g_unstrGlobals->m_topoRelsCacheNumRels=l_newtopoRelsCacheNumRels;
	g_unstrGlobals->m_topoRelsCacheCelltypes=l_newtopoRelsCacheCelltypes;
	g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic=l_newtopoRelsCacheCelltypeIsQuadratic;
	g_unstrGlobals->m_topoRelsCacheRels=l_newtopoRelsCacheRels;
      }
 
    
    g_unstrGlobals->m_topoRelsCacheNumElems[g_unstrGlobals->m_topoRelsCacheSize]=l_totalNumElems;
    g_unstrGlobals->m_topoRelsCacheSetRows[g_unstrGlobals->m_topoRelsCacheSize]=get_index_of_set(&a_set);
    g_unstrGlobals->m_topoRelsCacheCelltypes[g_unstrGlobals->m_topoRelsCacheSize]=a_cellType;
    g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic[g_unstrGlobals->m_topoRelsCacheSize]=a_isQuadraticCellType;
    g_unstrGlobals->m_topoRelsCacheNumRels[g_unstrGlobals->m_topoRelsCacheSize]=l_numTopoRels;
    if(!l_numTopoRels)
      {
	g_unstrGlobals->m_topoRelsCacheRels[g_unstrGlobals->m_topoRelsCacheSize]=NULL;
      }
    else
      {
	g_unstrGlobals->m_topoRelsCacheRels[g_unstrGlobals->m_topoRelsCacheSize] = l_relList;
      }

    /* printinfo("get_num_topo_rels_in_set added to cache entry %d: %s     %d %d                (l_numRels=%d)(kept ptr=%p)\n",
      g_unstrGlobals->m_topoRelsCacheSize, get_celltype_desc(a_cellType),   
      l_numTopoRels, l_totalNumElems, l_numRels,g_unstrGlobals->m_topoRelsCacheRels[g_unstrGlobals->m_topoRelsCacheSize] );*/

    g_unstrGlobals->m_topoRelsCacheSize++;
  }
#endif









  if( a_rels && l_numTopoRels )
    {
      a_rels[0] = (SAF_Rel *)malloc(l_numTopoRels*sizeof(SAF_Rel));
      if(!a_rels[0])
	{
	  printinfo("error allocating a_rels[0]\n");
	  exit(-1);
	}  
      for(k=0;k<l_numTopoRels;k++) a_rels[0][k]=l_relList[k];
    }


#ifdef USE_TOPO_RELS_CACHE
  /*if saf_find_topo_relations found some rels, but we are not keeping any of them,
    then we didnt store the rel list on the cache, so it is ok to free it*/
  if(l_numRels && !l_numTopoRels) 
#endif
    if( l_relList )
      {
	free(l_relList);
      }

  if(l_rels) free(l_rels);

  return(l_numTopoRels);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Subset Relations of a Given Celltype between a Given Set and Subset
 *
 * Description: 
 * Note that a_set and a_subset can be SAF_ANY_SET. (sslib using SS_SET_NULL!)
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_subsets_of_set( SAF_Set a_set, SAF_Set a_subset, SAF_CellType a_cellType, SAF_Rel **a_rels )
{
  SAF_Rel *l_rels=0,*l_relPtr;
  int l_numRels=0,l_numRelsKept=0,*l_keep=0;
  char *l_setName=0;
  SAF_Set *l_allSubSets=0,*l_allSuperSets=0;
  int l_numAllSubSets=0, l_numAllSuperSets=0, l_whichSubSet, l_whichSuperSet, l_whichRel;

  /*printinfo("entering get_num_subsets_of_set: type=%s\n",
    get_celltype_desc(a_cellType));*/

  /*note 30jul03 it was the repeated calling of get_num_subsets_of_set from get_num_var_fields_on_subsets_of_set
    that was causing the memory explosion with the jew data....
    fixed the mem leak in DSL_match_pattern_against_data "takeMe" alloc*/


#ifdef USE_SUBSETS_OF_SET_CACHE
  /*Check if this function has already been called with these inputs, and
    if so, return the same results found before*/
  {
    int l_which=-1,i;
    for(i=0;i<g_unstrGlobals->m_subsetsOfSetCacheSize;i++)
      {
	if( a_cellType==g_unstrGlobals->m_subsetsOfSetCacheCelltypes[i] )
	  {
	    if( SAF_EQUIV(&a_set,get_set_of_index(g_unstrGlobals->m_subsetsOfSetCacheSetRows[i])) &&
	        SAF_EQUIV(&a_subset,get_set_of_index(g_unstrGlobals->m_subsetsOfSetCacheSetRows[i])) )
	      {
		l_which=i;
		break;
	      }
#if 0/*took this out 11/2004 for sslib (no theRow)*/
	    /*If the cache has a record of this-set/any-set/zero-rels, then
	      we know that this-set/that-set/ will also have zero rels.
	      NOTE that this never happens....7/28/03 is it a mistake?*/
	    if( g_unstrGlobals->m_subsetsOfSetCacheNumRels[i] == 0 )
	      {
		if( (a_set.theRow==g_unstrGlobals->m_subsetsOfSetCacheSetRows[i] &&     
                       SAF_ANY_ROW==g_unstrGlobals->m_subsetsOfSetCacheSubsetRows[i]) ||
		    ( SAF_ANY_ROW==g_unstrGlobals->m_subsetsOfSetCacheSetRows[i] && 
                       a_subset.theRow==g_unstrGlobals->m_subsetsOfSetCacheSubsetRows[i] ) )
		  {
		    l_which=i;
		    printinfo("get_num_subsets_of_set warning: SAF_ANY_ROW trick usually never happens!\n");
		    break;
		  }
	      }
#endif
	  }
      }
    if(l_which>=0)
      {
	if(a_rels)
	  {
	    if(!g_unstrGlobals->m_subsetsOfSetCacheNumRels[l_which])
	      {
		a_rels[0]=NULL;/*no relations were found, so clear the pointer*/
	      }
	    else
	      {
		a_rels[0] = (SAF_Rel *)malloc( g_unstrGlobals->m_subsetsOfSetCacheNumRels[l_which]*sizeof(SAF_Rel) );
		if(!a_rels[0])
		  {
		    printinfo("error get_num_subsets_of_set couldnt alloc a_rels to size %d bytes\n",
			      g_unstrGlobals->m_subsetsOfSetCacheNumRels[l_which]*sizeof(SAF_Rel)); 
		    exit(-1);
		  }
		memcpy(a_rels[0],g_unstrGlobals->m_subsetsOfSetCacheRels[l_which],g_unstrGlobals->m_subsetsOfSetCacheNumRels[l_which]*sizeof(SAF_Rel));
	      }
	  }
	/*printinfo("get_num_subsets_of_set got from cache %d: %s\n",l_which,
	  get_celltype_desc(a_cellType) );*/
	return( g_unstrGlobals->m_subsetsOfSetCacheNumRels[l_which] );
      }
  }
#endif



  if( !my_saf_is_valid_set_handle(&a_set) )
    {
      l_numAllSuperSets = g_unstrGlobals->m_numAllSets;
      l_allSuperSets = g_unstrGlobals->m_allSets;
    }
  else
    {
      l_numAllSuperSets=1;
      l_allSuperSets = (SAF_Set *)malloc(sizeof(SAF_Set));
      l_allSuperSets[0]=a_set;
    }

  if( !my_saf_is_valid_set_handle(&a_subset) )
    {
      l_numAllSubSets = g_unstrGlobals->m_numAllSets;
      l_allSubSets = g_unstrGlobals->m_allSets;
    }
  else
    {
      l_numAllSubSets=1;
      l_allSubSets = (SAF_Set *)malloc(sizeof(SAF_Set));
      l_allSubSets[0]=a_subset;
    }

  if(!l_numAllSubSets || !l_numAllSuperSets) 
    {
      /*printinfo("get_num_subsets_of_set l_numAllSubSets==l_numAllSuperSets==0   returning 0\n");*/
      return(0);
    }

  if( my_saf_is_valid_set_handle(&a_set) )
    {
      saf_describe_set(SAF_ALL, &a_set, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL );
    }
  else
    {
      l_setName = ens_strdup("SAF_ANY_SET");
    }


  /* Count all the subset relations, and then allocate for them.
     Note that you cannot pass SAF_ANY_SET to saf_find_subset_relations*/
  l_numRels=0;
  for( l_whichSuperSet=0; l_whichSuperSet<l_numAllSuperSets; l_whichSuperSet++ )
    {
      for( l_whichSubSet=0; l_whichSubSet<l_numAllSubSets; l_whichSubSet++ )
	{
	  int l_numRelsHere=0;
#ifdef __cplusplus
	  saf_find_subset_relations(SAF_ALL,m_strReader->getDatabase(),&(l_allSuperSets[l_whichSuperSet]), &(l_allSubSets[l_whichSubSet]),SAF_ANY_CAT,
				    SAF_ANY_CAT,SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,&l_numRelsHere,NULL);
#else
	  saf_find_subset_relations(SAF_ALL,getDatabase(),&(l_allSuperSets[l_whichSuperSet]), &(l_allSubSets[l_whichSubSet]),SAF_ANY_CAT,
				    SAF_ANY_CAT,SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,&l_numRelsHere,NULL);
#endif
	  l_numRels += l_numRelsHere;
	}
    }


  if( l_numRels )
    {
      l_rels = (SAF_Rel *)malloc(l_numRels*sizeof(SAF_Rel));
      if(!l_rels)
	{
	  printinfo("error get_num_subsets_of_set cant allocate l_rels to %d SAF_Rel's\n",l_numRels);
	  exit(-1);
	}

      /* now get all the subset relations */
      l_relPtr = l_rels;
      for( l_whichSuperSet=0; l_whichSuperSet<l_numAllSuperSets; l_whichSuperSet++ )
	{
	  int l_numRelsHere=0;
	  for( l_whichSubSet=0; l_whichSubSet<l_numAllSubSets; l_whichSubSet++ )
	    {
	      l_numRelsHere=l_numRels-l_numRelsKept;/*this tells saf how much l_relPtr is allocated for*/

#ifdef __cplusplus
	      saf_find_subset_relations(SAF_ALL,m_strReader->getDatabase(),&(l_allSuperSets[l_whichSuperSet]),&(l_allSubSets[l_whichSubSet]),SAF_ANY_CAT,
					SAF_ANY_CAT,SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,&l_numRelsHere,&l_relPtr);
#else
	      saf_find_subset_relations(SAF_ALL,getDatabase(),&(l_allSuperSets[l_whichSuperSet]),&(l_allSubSets[l_whichSubSet]),SAF_ANY_CAT,
					SAF_ANY_CAT,SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,&l_numRelsHere,&l_relPtr);
#endif

	      /*note: saf_find_subset_relations will have changed the value of l_numRelsHere!*/
	      l_relPtr += l_numRelsHere;
	      l_numRelsKept += l_numRelsHere;

	      if( l_numRels <= l_numRelsKept )
		{ /*we already got all the relations, so we can quit*/
		  break;
		}
	    }

	  if( l_numRels <= l_numRelsKept )
	    { /*we already got all the relations, so we can quit*/
	      break;
	    }
	}

      /*now remove any relations we dont want?*/
      l_keep = (int *)malloc(l_numRels*sizeof(int));
      if(!l_keep)
	{
	  printinfo("error get_num_subsets_of_set couldnt alloc l_keep to size %d bytes\n",l_numRels*sizeof(int));
	  exit(-1);
	}
      l_numRelsKept=0;
      for( l_whichRel=0; l_whichRel<l_numRels; l_whichRel++ )
	{
	  SAF_Set l_superSet;
	  SAF_Cat l_subCat,l_superCat;
	  l_keep[l_whichRel]=1;

	  saf_describe_subset_relation(SAF_ALL,&(l_rels[l_whichRel]),&l_superSet,NULL,&l_superCat,&l_subCat,
				       NULL,NULL,NULL,NULL);

	  if( SAF_VALID(&l_superCat) && SAF_VALID(&l_subCat) )
	    {
	      SAF_CellType l_superCellType;
	      saf_describe_collection(SAF_ALL,&l_superSet,&l_superCat,&l_superCellType,NULL,NULL,NULL,NULL);

	      if( a_cellType!=SAF_CELLTYPE_ANY && a_cellType!=l_superCellType )/*or should it be l_subCellType?*/
		{
		  l_keep[l_whichRel]=0;
		}
	    }
	  else
	    {/*note: not keeping SAF_GENERAL relations.....is this right?*/
	      printinfo("note: get_num_subsets_of_set found invalid cats, probably a SAF_GENERAL relation\n");
	      l_keep[l_whichRel]=0;
	    }

	  if(l_keep[l_whichRel]) l_numRelsKept++;
	}

      /*return only the rels we want to keep*/
      if( l_numRelsKept != l_numRels )
	{
	  /*rather than reallocate, just shuffle down over the unused rels*/
	  int l_numRemoved=0;
	  for( l_whichRel=0; l_whichRel<l_numRels; l_whichRel++ )
	    {
	      if( !l_keep[l_whichRel] )
		{
		  l_numRemoved++;
		}
	      else if( l_numRemoved )
		{
		  l_rels[l_whichRel-l_numRemoved]=l_rels[l_whichRel];
		}
	    }
	}

      if(l_keep) free(l_keep);

    }


  if(l_allSubSets && l_allSubSets!=g_unstrGlobals->m_allSets) free(l_allSubSets);
  if(l_allSuperSets && l_allSuperSets!=g_unstrGlobals->m_allSets) free(l_allSuperSets);
  if(l_setName) free(l_setName);

  /*printinfo("   End get_num_subsets_of_set, returning %d of %d\n\n",l_numRelsKept,l_numRels);*/
  



#ifdef USE_SUBSETS_OF_SET_CACHE
  /*Add the 3 inputs and 2 outputs to the cache*/
  {
    if( !g_unstrGlobals->m_subsetsOfSetCacheMaxSize )
      {
	/* alloc cache for the first time */
	g_unstrGlobals->m_subsetsOfSetCacheMaxSize= (g_unstrGlobals->m_numAllSets>100) ? g_unstrGlobals->m_numAllSets:100;

	printinfo("m_subsetsOfSetCache alloc first time %d (m_numAllSets=%d)\n",
		  g_unstrGlobals->m_subsetsOfSetCacheMaxSize,g_unstrGlobals->m_numAllSets);
	g_unstrGlobals->m_subsetsOfSetCacheSubsetRows=(int *)malloc( g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(int) );
	g_unstrGlobals->m_subsetsOfSetCacheSetRows=(int *)malloc( g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(int) );
	g_unstrGlobals->m_subsetsOfSetCacheNumRels=(int *)malloc( g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(int) );
	g_unstrGlobals->m_subsetsOfSetCacheCelltypes=(SAF_CellType *)malloc(g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(SAF_CellType));
	g_unstrGlobals->m_subsetsOfSetCacheRels=(SAF_Rel **)malloc(g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(SAF_Rel *));
      }
    else if( g_unstrGlobals->m_subsetsOfSetCacheSize >= g_unstrGlobals->m_subsetsOfSetCacheMaxSize )
      {
	/*resize the array*/
	int *l_newsubsetsOfSetCacheSubsetRows=0;
	int *l_newsubsetsOfSetCacheSetRows=0;
	SAF_CellType *l_newsubsetsOfSetCacheCelltypes=0;
	int *l_newsubsetsOfSetCacheNumRels=0;
	SAF_Rel **l_newsubsetsOfSetCacheRels=0;

	g_unstrGlobals->m_subsetsOfSetCacheMaxSize *= 2;
	printinfo("m_subsetsOfSetCache resize cache, new size=%d\n",g_unstrGlobals->m_subsetsOfSetCacheMaxSize);

	l_newsubsetsOfSetCacheSubsetRows=(int *)malloc( g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(int) );
	l_newsubsetsOfSetCacheSetRows=(int *)malloc( g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(int) );
	l_newsubsetsOfSetCacheNumRels=(int *)malloc( g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(int) );
	l_newsubsetsOfSetCacheCelltypes=(SAF_CellType *)malloc(g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(SAF_CellType));
	l_newsubsetsOfSetCacheRels=(SAF_Rel **)malloc(g_unstrGlobals->m_subsetsOfSetCacheMaxSize * sizeof(SAF_Rel *));

	memcpy(l_newsubsetsOfSetCacheSubsetRows,g_unstrGlobals->m_subsetsOfSetCacheSubsetRows,g_unstrGlobals->m_subsetsOfSetCacheSize*sizeof(int));
	memcpy(l_newsubsetsOfSetCacheSetRows,g_unstrGlobals->m_subsetsOfSetCacheSetRows,g_unstrGlobals->m_subsetsOfSetCacheSize*sizeof(int));
	memcpy(l_newsubsetsOfSetCacheNumRels,g_unstrGlobals->m_subsetsOfSetCacheNumRels,g_unstrGlobals->m_subsetsOfSetCacheSize*sizeof(int));
	memcpy(l_newsubsetsOfSetCacheCelltypes,g_unstrGlobals->m_subsetsOfSetCacheCelltypes,g_unstrGlobals->m_subsetsOfSetCacheSize*sizeof(SAF_CellType));
	memcpy(l_newsubsetsOfSetCacheRels,g_unstrGlobals->m_subsetsOfSetCacheRels,g_unstrGlobals->m_subsetsOfSetCacheSize*sizeof(SAF_Rel *));

	free(g_unstrGlobals->m_subsetsOfSetCacheSubsetRows);
	free(g_unstrGlobals->m_subsetsOfSetCacheSetRows);
	free(g_unstrGlobals->m_subsetsOfSetCacheNumRels);
	free(g_unstrGlobals->m_subsetsOfSetCacheCelltypes);
	free(g_unstrGlobals->m_subsetsOfSetCacheRels);

	g_unstrGlobals->m_subsetsOfSetCacheSubsetRows=l_newsubsetsOfSetCacheSubsetRows;
	g_unstrGlobals->m_subsetsOfSetCacheSetRows=l_newsubsetsOfSetCacheSetRows;
	g_unstrGlobals->m_subsetsOfSetCacheNumRels=l_newsubsetsOfSetCacheNumRels;
	g_unstrGlobals->m_subsetsOfSetCacheCelltypes=l_newsubsetsOfSetCacheCelltypes;
	g_unstrGlobals->m_subsetsOfSetCacheRels=l_newsubsetsOfSetCacheRels;
      }
 
    
    g_unstrGlobals->m_subsetsOfSetCacheSubsetRows[g_unstrGlobals->m_subsetsOfSetCacheSize]=get_index_of_set(&a_subset);
    g_unstrGlobals->m_subsetsOfSetCacheSetRows[g_unstrGlobals->m_subsetsOfSetCacheSize]=get_index_of_set(&a_set);
    g_unstrGlobals->m_subsetsOfSetCacheCelltypes[g_unstrGlobals->m_subsetsOfSetCacheSize]=a_cellType;
    g_unstrGlobals->m_subsetsOfSetCacheNumRels[g_unstrGlobals->m_subsetsOfSetCacheSize]=l_numRelsKept;
    if(!l_numRelsKept)
      {
	g_unstrGlobals->m_subsetsOfSetCacheRels[g_unstrGlobals->m_subsetsOfSetCacheSize]=NULL;
      }
    else
      {
	g_unstrGlobals->m_subsetsOfSetCacheRels[g_unstrGlobals->m_subsetsOfSetCacheSize] = (SAF_Rel *)malloc( l_numRelsKept*sizeof(SAF_Rel) );
	memcpy(g_unstrGlobals->m_subsetsOfSetCacheRels[g_unstrGlobals->m_subsetsOfSetCacheSize],l_rels,l_numRelsKept*sizeof(SAF_Rel));
      }

    /*printinfo("get_num_subsets_of_set added to cache entry %d: %s  \t num rels=%d\n",
      g_unstrGlobals->m_subsetsOfSetCacheSize, get_celltype_desc(a_cellType),l_numRelsKept );*/

    g_unstrGlobals->m_subsetsOfSetCacheSize++;
  }
#endif



  if(a_rels && l_rels) a_rels[0]=l_rels; else if(l_rels) free(l_rels);


  return(l_numRelsKept);
}/*get_num_subsets_of_set*/





/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of 3D Coord Points in a Set at a Given Time
 *
 * Description: 
 *
 * Find the first 1d collection of SAF_CELLTYPE_POINTS that has >0 entries.
 * (There should be only one! If there are more collections that fit
 * this description, there will be an error)
 *
 * If this collection has any 3-component coord field on it, that
 * matches a_timestep (set a_timestep to -1 for 'dont care'), then
 * return the number of entries in the collection 
 *
 * XXX should it check if the field has been written?
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_3d_coord_points_in_set( SAF_Set a_set, int a_timestep )
{
  SAF_IndexSpec l_indexSpec;
  int l_count=0, l_numCollections=0, i,j;
  SAF_Cat *l_cats=0;
  SAF_Field l_theCoordField=SS_FIELD_NULL;
  int l_num3dCoordPoints=0;
  char *l_name=0;
  int l_foundTheCollection=0;

  saf_describe_set(SAF_ALL,&a_set,&l_name,NULL,NULL,NULL,NULL,&l_numCollections,&l_cats);

  for(i=0;i<l_numCollections;i++)
    {
      SAF_CellType l_cellType;
      saf_describe_collection(SAF_ALL,&a_set,&(l_cats[i]),&l_cellType,&l_count,&l_indexSpec,NULL,NULL);
      if( l_indexSpec.ndims==1 && l_count && l_cellType==SAF_CELLTYPE_POINT )
	{
	  /*check that this collection is not part of the states & suites*/
	  SAF_Role l_role;
	  int l_catDim=-1;
	  saf_describe_category(SAF_ALL,&l_cats[i],NULL,&l_role,&l_catDim);

	  /*Added l_catDim check 12feb2004, because of first FUEGO saf results files*/
	  if( SAF_EQUIV(&l_role,SAF_SPACE_SLICE) || SAF_EQUIV(&l_role,SAF_PARAM_SLICE) || l_catDim!=0 )
	    {
	      /* note: should we exclude any other categories here? */
	      continue;
	    }

	  /*check that this is the only such collection*/
	  if(l_foundTheCollection)
	    {
	      printinfo("\nget_num_3d_coord_points_in_set error: found >1 1D POINT collections on set %s\n",l_name);

#if 1
	      /*print info on all the cats*/
	      printinfo("    error was found at cat #%d\n",i);
	      for(j=0;j<l_numCollections;j++)
		{
		  char *l_catName=NULL,*l_roleName=NULL;
		  int l_dim=0;
		  SAF_Role l_role;
		  SAF_CellType l_type;
		  saf_describe_collection(SAF_ALL,&a_set,&(l_cats[j]),&l_type,NULL,NULL,NULL,NULL);
		  saf_describe_category(SAF_ALL,&(l_cats[j]),&l_catName,&l_role,&l_dim);
		  l_roleName = get_role_desc(l_role);
		  printf("    cat #%d  name=%s  dim=%d  role=%s    celltype=%s\n",j,l_catName,l_dim,l_roleName,get_celltype_desc(l_type));
		  free(l_catName);free(l_roleName);
		}
#endif
	      printinfo("exiting\n\n");

	      exit(-1);
	    }			
	  l_foundTheCollection=1;


	  /*first, check if the default coord field fits the parameters,
	    so maybe we can save some time not calling saf_find_fields*/
	  {
	    SAF_Field l_defaultCoordField;
	    saf_find_default_coords(SAF_ALL,&a_set,&l_defaultCoordField);

	    /*must also check the .nbufs because saf_find_default_coords sets it to
	      zero if there is a problem....I think it should set the field
	      to SS_FIELD_NULL instead .....change this later*/
	    if( /* l_defaultCoordField.nbufs not in sslib &&*/
		 my_saf_is_valid_field_handle(&l_defaultCoordField)   )
	      {
		hbool_t l_isCoordField;/*Is this redundant? Can the 'default coord field' not be a 'coord field'?*/
		int l_numComponents=0;
		char *l_fieldName=0;
		saf_describe_field(SAF_ALL, &l_defaultCoordField,  NULL, &l_fieldName, NULL, NULL,
				   &l_isCoordField, NULL, NULL, NULL, NULL, NULL, NULL,
				   &l_numComponents, NULL, NULL, NULL);

		if( l_numComponents==3 && l_isCoordField )
		  {
		    if( a_timestep<0 || is_this_field_on_this_timestep(l_defaultCoordField,a_timestep) || 
			get_timestep_for_field(l_defaultCoordField)<0 )
		      {
			l_num3dCoordPoints = l_count;
			l_theCoordField = l_defaultCoordField;
			/*printinfo("get_num_3d_coord_points_in_set: set:%s field:%s (firsttime %d)(a_timestep=%d) %d pts) DEFAULT COORD\n",
			  l_name,l_fieldName,l_firsttime,a_timestep,l_count);*/
		      }
		  }
		if(l_fieldName) free(l_fieldName);
	      }
	  }


	  if( !my_saf_is_valid_field_handle( &l_theCoordField ) )
	    {
	      int l_numCoordFields=0;
	      SAF_Field *l_coordFields=0; 

	      get_all_fields_for_set( a_set, NULL,NULL,NULL, &l_numCoordFields,&l_coordFields);

	      for( j=0; j<l_numCoordFields; j++ )
		{
		  int l_numComponents=0;
		  if( !my_saf_is_valid_field_handle(&(l_coordFields[j])) ) continue;
		  saf_describe_field(SAF_ALL, &(l_coordFields[j]),  NULL, NULL, NULL, NULL,
				     NULL, NULL, NULL, NULL, NULL, NULL, NULL,  
				     &l_numComponents, NULL, NULL, NULL);
		  if( l_numComponents==3 )
		    {
		      if( a_timestep<0 || is_this_field_on_this_timestep(l_coordFields[j],a_timestep) || 
			  get_timestep_for_field(l_coordFields[j])<0 )
			{
			  l_num3dCoordPoints = l_count;
			  l_theCoordField = l_coordFields[j];
			  break;/*dont need to look at any more fields*/
			}
		    }
		}
	      /*if(l_coordFields) free(l_coordFields); do not free, because of get_all_fields_for_set*/
	    }
	  
	  /*If we found the field, and if it is a specific field at a specific time
	    (i.e. a_timestep was not set to -1 or "any time"), then change the # of points if 
	    the field count is different from the collection count.  If we chose a_timestep
	    as "any time", then we should stick with the collection count, because it should(?)
	    be the maximum of the two. */
	  if( a_timestep>=0 && my_saf_is_valid_field_handle(&l_theCoordField) )
	    {
	      size_t l_actualCount=0; 
	      int l_numComponents=0;
	      SAF_Field *l_componentFields=NULL;

	      saf_describe_field(SAF_ALL, &l_theCoordField,  NULL, NULL, NULL, NULL,
				 NULL, NULL, NULL, NULL, NULL, NULL, NULL,  
				 &l_numComponents, &l_componentFields, NULL, NULL);
	      saf_get_count_and_type_for_field(SAF_ALL,&l_theCoordField, NULL,&l_actualCount, NULL);

	      if( l_actualCount )
		{
		  l_actualCount = l_actualCount/l_numComponents;
		  if( l_count!=(int)l_actualCount)
		    {
		      /*printinfo(" changing l_count from %d to %d from orig field\n",l_count,l_actualCount);*/
		      l_count = (int)l_actualCount;
		    }
		}
	      else if( l_componentFields )
		{
		  saf_get_count_and_type_for_field(SAF_ALL,&(l_componentFields[0]), NULL,&l_actualCount, NULL);		      
		  if( l_actualCount && l_count!=(int)l_actualCount)
		    {
		      /*printinfo(" changing l_count from %d to %d from component field\n",l_count,l_actualCount);*/
		      l_count = (int)l_actualCount;
		    }
		}

	      l_num3dCoordPoints=l_count;
	      if(l_componentFields) free(l_componentFields);
	    }


	}
    }


  /*printinfo("get_num_3d_coord_points_in_set: leaving set:%s a_timestep=%d pts=%d\n",l_name,a_timestep,l_num3dCoordPoints);*/

  if(l_name) free(l_name);
  if(l_cats) free(l_cats);

  return(l_num3dCoordPoints);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of 3D Coord Points in a Set at a Given Time
 *
 * Description: 
 * Note that this function is needed for reading larry1w.saf
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_2d_coord_points_in_set( SAF_Set a_set, int a_timestep )
{
  SAF_IndexSpec l_indexSpec;
  int l_num2dCoordPoints=0;
  int l_count=0, j;
  int l_numCoordFields=0;
  SAF_Field *l_coordFields=0; 

  if( !my_saf_is_valid_set_handle(&a_set) )
    {
      printinfo("error get_num_2d_coord_points_in_set set is not valid\n");
      exit(-1);
    }

  get_all_fields_for_set( a_set, NULL,NULL,NULL, &l_numCoordFields,&l_coordFields);

  for( j=0; j<l_numCoordFields; j++ )
    {
      int l_numComponents=0;
      SAF_Cat l_cat;
      SAF_CellType l_cellType;
      if( !my_saf_is_valid_field_handle( &(l_coordFields[j])) ) continue;
      saf_describe_field(SAF_ALL, &(l_coordFields[j]), NULL, NULL, NULL, NULL,
			 NULL, NULL, &l_cat, NULL, NULL, NULL, NULL,  
			 &l_numComponents, NULL, NULL, NULL);
      saf_describe_collection(SAF_ALL,&a_set,&l_cat,&l_cellType,&l_count,&l_indexSpec,NULL,NULL);
      if( l_indexSpec.ndims==1 && l_count && l_cellType==SAF_CELLTYPE_POINT )
	{

	  { /*Added 12feb2004 with similar fix in get_num_3d_coord_points_in_set.
	      Does it really need to be here? Is this possible?*/
	    SAF_Role l_role;
	    int l_catDim=-1;
	    saf_describe_category(SAF_ALL,&l_cat,NULL,&l_role,&l_catDim);
	    if( SAF_EQUIV(&l_role,SAF_SPACE_SLICE) || SAF_EQUIV(&l_role,SAF_PARAM_SLICE) || l_catDim!=0 )
	      {
		continue;
	      }
	  }

	  if( l_numComponents==2 )
	    {
	      if( a_timestep<0 || is_this_field_on_this_timestep(l_coordFields[j],a_timestep) || 
		  get_timestep_for_field(l_coordFields[j])<0 )
		{
		  if( !l_num2dCoordPoints )/*****TEMP?*****/
		    {
		      l_num2dCoordPoints += l_count;
		      /*printinfo("  get_num_2d_coord_points_in_set %d coord pts in field\n",
			l_count);*/
		    }
		  else
		    {
		      /*printinfo("  get_num_2d_coord_points_in_set %d coord pts in field MORE THAN ONE TIME?\n",
			l_count);*/
		    }
		}
	    }
	}
    }
  if( l_num2dCoordPoints ) printinfo("\n\nwarning......found some 2d coord points\n\n");
  /*if(l_coordFields) free(l_coordFields); do not free, because of get_all_fields_for_set*/
  return(l_num2dCoordPoints);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of Coord Points Actually Stored on a Set for a Given Timestep
 *
 * Description: 
 *
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_points_in_set( SAF_Set a_set, int a_timestep )
{
  /*
  ** note: cant yet handle both 2d and 3d points in a set. So,
  ** if we find 3d points, dont bother looking for 2d points,
  ** to save time. THIS MAY CAUSE A SILENT ERROR SOMETIME!!
  */
  int l_numPoints=get_num_3d_coord_points_in_set( a_set, a_timestep );
  if(!l_numPoints)
    {
      l_numPoints= get_num_2d_coord_points_in_set( a_set, a_timestep );
    }
  return(l_numPoints);
}





/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Change the Interleave of an Array from Component to Vector
 *
 * Description: 
 * Changes, in place, the interleave of an array from SAF_INTERLEAVE_COMPONENT to SAF_INTERLEAVE_VECTOR.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
change_from_component_to_vector_interleave( int a_numComponents, size_t a_numEntries, MY_PRECISION *a_data )
{
  MY_PRECISION *l_temp=0,*l_ptr=0;
  size_t i;
  int j;

  l_temp = (MY_PRECISION *)malloc(a_numEntries*a_numComponents*sizeof(MY_PRECISION));
  if(!l_temp)
    {
      printinfo("change_from_component_to_vector_interleave error allocating l_temp\n");
      exit(-1);
    }  
  memcpy( l_temp, a_data, a_numEntries*a_numComponents*sizeof(MY_PRECISION) );
  l_ptr = a_data;

  for( i=0; i<a_numEntries; i++ )
  {
    for( j=0; j<a_numComponents; j++ )
    {
      l_ptr[0] = l_temp[ j*a_numEntries + i ];
      l_ptr++;
    }
  }
  free(l_temp);
  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Change the Interleave of an Array from Vector to Component
 *
 * Description: 
 * Changes, in place, the interleave of an array from SAF_INTERLEAVE_VECTOR to SAF_INTERLEAVE_COMPONENT.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
change_from_vector_to_component_interleave( int a_numComponents, size_t a_numEntries, MY_PRECISION *a_data )
{
  MY_PRECISION *l_temp=0,*l_ptr=0;
  int j;
  size_t i;

  l_temp = (MY_PRECISION *)malloc(a_numEntries*a_numComponents*sizeof(MY_PRECISION));
  if(!l_temp)
    {
      printinfo("change_from_vector_to_component_interleave error allocating l_temp\n");
      exit(-1);
    }  
  memcpy( l_temp, a_data, a_numEntries*a_numComponents*sizeof(MY_PRECISION) );
  l_ptr = a_data;

  for( j=0; j<a_numComponents; j++ )
    {
      for( i=0; i<a_numEntries; i++ )
	{
	  l_ptr[0] = l_temp[ i*a_numComponents + j ];
	  l_ptr++;
	}
    }
  free(l_temp);
  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Change a 2D array to 3D by Filling the Third Dimension with Zeroes
 *
 * Description: 
 * The array a_data must have room to add a third dimension. The array will be changed
 * from { x,y,x,y,x,y...} to { x,y,0,x,y,0,x,y,0...}.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
change_to_3d_points( int a_numComponents, int a_numEntries, MY_PRECISION *a_data )
{
  MY_PRECISION *l_temp=0,*l_fromPtr=0,*l_toPtr=0;
  int i,j;

  /*  printinfo("\nCHANGING a_data from %dd to 3d\n\n",a_numComponents);*/

  l_temp = (MY_PRECISION *)malloc(a_numEntries*a_numComponents*sizeof(MY_PRECISION));
  if(!l_temp)
    {
      printinfo("change_to_3d_points error allocating l_temp\n");
      exit(-1);
    }  
  memcpy( l_temp, a_data, a_numEntries*a_numComponents*sizeof(MY_PRECISION) );
  l_toPtr = a_data;
  l_fromPtr = l_temp;

  for( i=0; i<a_numEntries; i++ )
  {
    for( j=0; j<a_numComponents; j++ )
    {
      l_toPtr[0] = l_fromPtr[0];
      l_toPtr++;
      l_fromPtr++;
    }
    for( j=a_numComponents; j<3; j++ )
    {
      l_toPtr[0] = 0;
      l_toPtr++;
    }
  }
  free(l_temp);
  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get Unstructured Point Coords Stored on the Set
 *
 * Description: 
 *
 * Gets the point coords actually stored on the set. If no 3D coords are found, it will
 * look for 2D coords. If a_data is not NULL, then it must be preallocated to 3N entries,
 * where N is the number of point coords in the set.
 *
 * Return: the number of point coords found
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_unstr_point_coords_in_set( SAF_Set a_set, MY_PRECISION *a_data, int a_timestep )
{
  SAF_IndexSpec l_indexSpec;
  int l_count=0, l_numCollections=0,i,j, l_foundTheField=0;
  SAF_Cat *l_cats=0;
  char *l_setName=0;
  SAF_Field l_defaultCoordField;

  if(!my_saf_is_valid_set_handle(&a_set))
    {
      printinfo("get_unstr_point_coords_in_set: invalid set handle, returning 0\n");
      return(0);
    }

  saf_describe_set(SAF_ALL,&a_set,&l_setName,NULL,NULL,NULL,NULL,&l_numCollections,&l_cats);

  /* printinfo("in get_unstr_point_coords_in_set %s: a_data=%s a_timestep=%d  numcoll=%d\n",
     l_setName,a_data?"non-zero":"zero",a_timestep,l_numCollections);*/

  saf_find_default_coords(SAF_ALL,&a_set,&l_defaultCoordField);

  for(i=0;i<l_numCollections;i++)
    {
      SAF_CellType l_cellType;


      { /*Added 12feb2004 with similar fix in get_num_3d_coord_points_in_set.
	  Does it really need to be here? Is this possible?*/
	SAF_Role l_role;
	int l_catDim=-1;
	saf_describe_category(SAF_ALL,&(l_cats[i]),NULL,&l_role,&l_catDim);
	if( SAF_EQUIV(&l_role,SAF_SPACE_SLICE) || SAF_EQUIV(&l_role,SAF_PARAM_SLICE) || l_catDim!=0 )
	  {
	    continue;
	  }
      }


      saf_describe_collection(SAF_ALL,&a_set,&(l_cats[i]),&l_cellType,&l_count,&l_indexSpec,NULL,NULL);
      if( l_indexSpec.ndims==1 && l_count && l_cellType==SAF_CELLTYPE_POINT )
	{
	  int l_numFields=0; /*note: these are coord fields*/
	  SAF_Field *l_fields=0;  /*note: these are coord fields*/
	  /* We have found the 1D POINT collection. We should only be here once.*/

	  get_all_fields_for_set( a_set, NULL,NULL,NULL, &l_numFields,&l_fields);

	  for( j = 0 ; j < l_numFields; j++ )
	    {
	      hid_t l_varType;
	      int l_numComponents=0;
	      char *l_fieldName=0;
	      SAF_Interleave l_interleave;
	      int l_isTheDefaultCoordField=0;
	      SAF_Cat l_thisCat;
	      SAF_Field *l_componentFields=NULL;

	      if(!my_saf_is_valid_field_handle(&(l_fields[j]))) continue;

	      saf_describe_field(SAF_ALL, &(l_fields[j]), NULL, &l_fieldName, NULL, NULL,
				 NULL,  NULL, &l_thisCat, NULL, NULL, NULL, &l_varType,  
				 &l_numComponents, &l_componentFields, &l_interleave, NULL);

	      if( l_numComponents>1 && l_interleave==SAF_INTERLEAVE_NONE )
		{
		  printinfo("error get_unstr_point_coords_in_set not prepared to handle l_numComponents=%d and SAF_INTERLEAVE_NONE\n",
			    l_numComponents);
		  return(-1);
		}


	      if(!SAF_EQUIV(&l_thisCat,&(l_cats[i]))) 
		{
		  if(l_componentFields) free(l_componentFields);
		  continue;/*added this for get_all_fields_for_set*/
		}

	      if(SAF_EQUIV(&l_defaultCoordField,&(l_fields[j]))) l_isTheDefaultCoordField=1;

	      /*note there will be an error if we have both 3d and 2d point*/

	      if( (l_numComponents==3 || l_numComponents==2) && 
		  (a_timestep<0 || is_this_field_on_this_timestep(l_fields[j],a_timestep) || 
		   get_timestep_for_field(l_fields[j])<0) )
		{
		  if(!l_foundTheField)
		    {
		      /* printinfo("    get_unstr_point_coords_in_set %s found: field \"%s\" (#%d of %d fields on set) (%d comps) (is default coord=%d), a_timestep=%d\n",
				l_setName,l_fieldName,j,l_numFields, l_numComponents,l_isTheDefaultCoordField,a_timestep );*/
		      
		    }
		  else
		    {
#if 0
		      printinfo("    get_unstr_point_coords_in_set %s: field %s (%d of %d) (default=%d), a_timestep=%d NOT USED\n",
				l_setName,l_fieldName,j,l_numFields,l_isTheDefaultCoordField,a_timestep );
		      if(l_fieldName) free(l_fieldName);
		      if(l_componentFields) free(l_componentFields);
		      continue;
#else
		      /*this setting is for the ALE3D sample data, which has the orig coord data
			repeated at every timestep*/
		      printinfo("    get_unstr_point_coords_in_set %s: field %s (%d of %d) (default=%d), a_timestep=%d OVERRIDING PREVIOUS\n",
				l_setName,l_fieldName,j,l_numFields,l_isTheDefaultCoordField,a_timestep );
#endif
		    }

		  /*If we dont have a valid type, check the type of the first component field*/
		  if( l_varType<0/*sslib H5I_INVALID_HID*/ && l_componentFields )
		    {
		      hid_t l_componentVarType;
		      saf_describe_field(SAF_ALL, &(l_componentFields[0]), NULL, NULL, NULL, NULL,
					 NULL,  NULL, NULL, NULL, NULL, NULL, &l_componentVarType,  
					 NULL,NULL, NULL, NULL);
		      l_varType = l_componentVarType;
		      /*printinfo(" set type from component to: %s\n",get_dsl_type_string(l_varType) );*/
		    }


		  /*Is this still necessary? I think I wrote guess_dsl_var_type_for_composite_coord_field
		    when I barely understood saf.*/
		  if( l_varType < 0 ) /*sslib? == H5I_INVALID_HID )*/
		    {
		      printinfo("trying to guess type for field: %s (coll %d)\n",l_fieldName,i);
		      l_varType = guess_dsl_var_type_for_composite_coord_field(l_numFields,l_fields);
		    }


		  l_foundTheField=1;

		  /*now change l_count from the # in the collection to the actual # in the field, if needed*/
		  {
		    size_t l_actualCount=0;
		    saf_get_count_and_type_for_field(SAF_ALL,&(l_fields[j]), NULL,&l_actualCount, NULL);
		    if( l_actualCount )
		      {
			l_actualCount = l_actualCount/l_numComponents;
			if( l_count!=(int)l_actualCount)
			  {
			    /*printinfo(" changing l_count from %d to %d from orig field\n",l_count,l_actualCount);*/
			    l_count = (int)l_actualCount;
			  }
		      }
		    else if( l_componentFields )
		      {
			saf_get_count_and_type_for_field(SAF_ALL,&(l_componentFields[0]), NULL,&l_actualCount, NULL);		      
			if( l_actualCount && l_count!=(int)l_actualCount)
			  {
			    /*printinfo(" changing l_count from %d to %d from component field\n",l_count,l_actualCount);*/
			    l_count = (int)l_actualCount;
			  }
		      }
		  }


		  if( a_data )
		    {
		      if( read_whole_field_to_my_precision(&(l_fields[j]),l_varType,
							   (size_t)(l_numComponents*l_count),a_data) )
			{
			  printinfo("get_unstr_point_coords_in_set error failed reading field?\n");
			  if(l_componentFields) free(l_componentFields);
			  return(-1);
			}
		      if( l_interleave == SAF_INTERLEAVE_COMPONENT ||l_interleave==SAF_INTERLEAVE_INDEPENDENT)
			{
			  printinfo("changing from SAF_INTERLEAVE_COMPONENT to _VECTOR\n");
			  change_from_component_to_vector_interleave( l_numComponents, (size_t)l_count, a_data );
			}
		      if( l_numComponents == 2 )
			{
			  printinfo("changing from 2d point coords to 3d\n");
			  change_to_3d_points( l_numComponents, l_count, a_data );
			}



#if 0 /******************************************/
		      {
			int k;
			size_t l_myCount=0;
			float l_min,l_max;
			float l_numEntries = l_numComponents*l_count;
			l_min=l_max=a_data[0];
			for(k=0;k<l_numEntries;k++)
			  {
			    if(l_min>a_data[k]) l_min = a_data[k];
			    if(l_max<a_data[k]) l_max = a_data[k];
			  }
			saf_get_count_and_type_for_field(SAF_ALL,&(l_fields[j]), NULL,&l_myCount, NULL);

			printinfo("get_unstr_point_coords_in_set field=%s (count=%d comp=%d) l_min=%f l_max=%f    l_myCount=%d\n\n",
				  l_fieldName,l_count,l_numComponents,l_min,l_max,l_myCount);
		      }
#endif /******************************************/


		    }
		  /*break; NOT BREAKING HERE SO THAT IF THERE ARE EXTRA COORD FIELDS, INFO WILL BE PRINTED*/
		}

	      if(l_componentFields) free(l_componentFields);
	      if(l_fieldName) free(l_fieldName);
	    }

	  /*if(l_fields) free(l_fields); dont free this if using get_all_fields_for_set*/

	  /*If we get here, then we have already encountered the 1D POINT collection.
	    There should only be one. If there were more than one, then
	    get_num_3d_coord_points_in_set would have returned an error. So we
	    can break here. (XXX should actually check for an error here too) */
	  break;
	}
    }

  if(l_cats) free(l_cats);
  if(l_setName) free(l_setName);

  /*printinfo("      get_unstr_point_coords_in_set returning %d (=%d * %d)\n",l_foundTheField*l_count,l_foundTheField,l_count);*/



  return(l_foundTheField*l_count);
}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get Unstructured Point Coords Stored on a Part
 *
 * Description: 
 *
 * Gets the point coords actually stored on the part. If no 3D coords are found, it will
 * look for 2D coords. If a_data is not NULL, then it must be preallocated to 3N entries,
 * where N is the number of point coords in the part.
 *
 * Return: the number of point coords found
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_unstr_point_coords_in_part( int a_whichPart, int a_whichTimestep, MY_PRECISION *a_data )
{
  int l_numActuallyFound=0, l_setIndex=-1;
  printinfo("ENTERING get_unstr_point_coords_in_part, a_whichPart=%d a_whichTimestep=%d a_data=%s\n",
	 a_whichPart,a_whichTimestep, a_data?"non-zero":"zero" );
  if(!is_part_arg_valid(a_whichPart)) return(-1);
  if(!is_time_arg_valid(a_whichTimestep)) return(-1);

  l_setIndex = g_unstrGlobals->m_setIndexPerPart[a_whichPart];
  if( l_setIndex < 0 )
    {
      return(0);/*no data for this timestep*/
    }
  l_numActuallyFound = get_unstr_point_coords_in_set( g_unstrGlobals->m_allSets[l_setIndex], a_data, a_whichTimestep );

  if(!l_numActuallyFound && a_data)
    {
      printinfo("get_unstr_point_coords_in_part error, couldnt find the right field?\n");
      return(-1);
    }
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get Unstructured Point Coords Used by a Part
 *
 * Description: 
 *
 * Gets the point coords stored on the part OR stored elsewhere, but used by the part. If no 3D coords 
 * are found, it will look for 2D coords. If a_data is not NULL, then it must be preallocated to 3N entries,
 * where N is the number of point coords used by the part.
 *
 * Return: the number of point coords found
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_unstr_point_coords_used_by_part( int a_whichPart, int a_whichTimestep, MY_PRECISION *a_data )
{
  int l_setIndex=-1,i;
  /*
    printinfo("ENTERING get_unstr_point_coords_used_by_part, a_whichPart=%d a_whichTimestep=%d a_data=%s\n",
    a_whichPart,a_whichTimestep, a_data?"non-zero":"zero" );
  */
  if(!is_part_arg_valid(a_whichPart)) return(-1);
  if(!is_time_arg_valid(a_whichTimestep)) return(-1);



  l_setIndex = g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[a_whichPart][a_whichTimestep];


  /*printinfo("get_unstr_point_coords_used_by_part, a_whichPart=%d a_whichTimestep=%d setind=%d partsetind=%d\n",
    a_whichPart,a_whichTimestep, l_setIndex, g_unstrGlobals->m_setIndexPerPart[a_whichPart]);*/


  if( l_setIndex < 0 )
    {
      return(0);
    }
  else
    {
      int l_numPoints=0;
      int *l_xform=NULL;

      /*Do we need to use a subset transform?*/
      if(g_unstrGlobals->m_setIndexPerPart[a_whichPart] != l_setIndex)
	{
	  if(g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform[a_whichPart][a_whichTimestep])
	    {
	      /*yes, we will need to use a subset transform*/
	      l_xform = g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform[a_whichPart][a_whichTimestep];

#ifndef USE_CONNECTIVITY_REDUCTION_AND_CACHE
	      /*This should never happen: I expect to always use the cache, and so far the only datasets
		that have a subset transform are of my own creation. 30jan2004*/
	      printinfo("error get_unstr_point_coords_used_by_part subset xforms not written in USE_CONNECTIVITY_REDUCTION_AND_CACHE\n");
	      exit(-1);
#endif

	    }
	}




#ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE


      {


	if( g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart]<0 )
	  {
	    /*Whether the points are time independent has not been determined yet, so do it now.
	      Note that this also would imply that the connectivities are time independent also,
	      because there is only one set per part for all time.*/
	    g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart] =
	      !are_unstr_point_coords_used_by_part_time_independent(a_whichPart);


	    if(!g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart])
	      printinfo("The points for unstr part %d are not time dependent.\n",a_whichPart);
	    else 
	      printinfo("The points for unstr part %d are time dependent.\n",a_whichPart);

	  }

	if(!g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart])
	  {
	    a_whichTimestep=0;/*Just use time 0*/
	  }


	if( g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep] >=0 )
	  { /*we have already cached the data, so just return it*/
	    if(a_data && g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep])
	      {
		memcpy( a_data,
			g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep],
			3*g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep]*sizeof(MY_PRECISION)
			);
	      }
	    return( g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep] );
	  }



	/*cache the data*/
	l_numPoints = get_unstr_point_coords_in_set( g_unstrGlobals->m_allSets[l_setIndex], NULL, a_whichTimestep );





	if(!l_numPoints && !g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart])
	  {
	    /*If we get here then get_unstr_point_coords_in_set has somehow failed to 
	      get the points. If there really are no points, then l_setIndex would be -1.
	      (and the part should not be a part) */



	    /*jake hooey SHOULD I READ FROM SUBSETS/SUPERSETS HERE?*/


	    printinfo("\nwarning get_unstr_point_coords_used_by_part: there are no points for any time for part %d. why is it a part?\n\n",
		      a_whichPart);
	  }






	/*Note this has to be set before we start calling get_*_connectivity, or else there will be
	  an infinite loop, because each get_*_connectivity could call this function.*/
	g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep] = l_numPoints;

	if(l_numPoints)
	  {
	    g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep] = (MY_PRECISION *)
	      malloc(3*l_numPoints*sizeof(MY_PRECISION));

	    get_unstr_point_coords_in_set( g_unstrGlobals->m_allSets[l_setIndex], 
					   g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep], 
					   a_whichTimestep );

	    g_unstrGlobals->m_pointElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_point_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_triElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_tri_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_quadElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_quad_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_tetElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_tet_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_hexElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_hex_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_pyramidElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_pyramid_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_prismElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_prism_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_lineElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_line_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    /*g_unstrGlobals->m_0ballElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_0ball_elements_in_unstr_part(a_whichPart,a_whichTimestep);*/
	    g_unstrGlobals->m_1ballElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_1ball_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_2ballElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_2ball_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    g_unstrGlobals->m_3ballElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	      = get_num_3ball_elements_in_unstr_part(a_whichPart,a_whichTimestep);


  	    if(g_unstrGlobals->m_useQuadraticElements)
	    {
	      g_unstrGlobals->m_quadraticHexElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	        = get_num_quadraticHex_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	      g_unstrGlobals->m_quadraticQuadElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	        = get_num_quadraticQuad_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	      g_unstrGlobals->m_quadraticTetElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	        = get_num_quadraticTet_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	      g_unstrGlobals->m_quadraticTriElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	        = get_num_quadraticTri_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	      g_unstrGlobals->m_quadraticPyramidElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	        = get_num_quadraticPyramid_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	      g_unstrGlobals->m_quadraticPrismElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	        = get_num_quadraticPrism_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	      g_unstrGlobals->m_quadraticLineElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]
	        = get_num_quadraticLine_elements_in_unstr_part(a_whichPart,a_whichTimestep);
	    }
	
	  }
	else
	  {
	    g_unstrGlobals->m_pointElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_triElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_quadElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_tetElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_hexElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_pyramidElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_prismElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_lineElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    /*g_unstrGlobals->m_0ballElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;*/
	    g_unstrGlobals->m_1ballElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_2ballElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	    g_unstrGlobals->m_3ballElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]=0;
	  }



#define ALLOC_AND_GET_ELEMENT_CONN_CACHE(MA_TYPENAME,MA_NODESPERELEM) \
	if(g_unstrGlobals->m_ ## MA_TYPENAME ## ElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]) \
	  { \
	    g_unstrGlobals->m_ ## MA_TYPENAME ## ConnectivityPerPartPerTimestep[a_whichPart][a_whichTimestep] = (int *) \
	      malloc( MA_NODESPERELEM* \
		g_unstrGlobals->m_ ## MA_TYPENAME ## ElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]* \
		sizeof(int) ); \
	    g_unstrGlobals->m_ ## MA_TYPENAME ## ConnectivityPerPartPerTimestep[a_whichPart][a_whichTimestep][0]=-1; \
	    get_ ## MA_TYPENAME ## _element_connectivity(a_whichPart, a_whichTimestep,  \
		   g_unstrGlobals->m_ ## MA_TYPENAME ## ConnectivityPerPartPerTimestep[a_whichPart][a_whichTimestep]); \
	  }


	ALLOC_AND_GET_ELEMENT_CONN_CACHE(hex,8)
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(quad,4) 
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(tet,4)
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(tri,3)
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(pyramid,5)
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(point,1)
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(prism,6)
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(line,2)
	/*ALLOC_AND_GET_ELEMENT_CONN_CACHE(0ball,NUM_NODES_IN_0BALL) not in sslib? */
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(1ball,NUM_NODES_IN_1BALL)
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(2ball,NUM_NODES_IN_2BALL)
	ALLOC_AND_GET_ELEMENT_CONN_CACHE(3ball,NUM_NODES_IN_3BALL)
  	if(g_unstrGlobals->m_useQuadraticElements)
	{
	  ALLOC_AND_GET_ELEMENT_CONN_CACHE(quadraticHex,20)
	  ALLOC_AND_GET_ELEMENT_CONN_CACHE(quadraticQuad,8)
	  ALLOC_AND_GET_ELEMENT_CONN_CACHE(quadraticTet,10)
	  ALLOC_AND_GET_ELEMENT_CONN_CACHE(quadraticTri,6)
	  ALLOC_AND_GET_ELEMENT_CONN_CACHE(quadraticPyramid,13)
	  ALLOC_AND_GET_ELEMENT_CONN_CACHE(quadraticPrism,15)
	  ALLOC_AND_GET_ELEMENT_CONN_CACHE(quadraticLine,3)
	}
#undef ALLOC_AND_GET_ELEMENT_CONN_CACHE







	/*XXX need to check if is time dependent and reduce accordingly. Also look for another record of time dependence?*/


	
	if(l_xform)
	  {
	    /*There is a subset relation(s) between us and the points. Need to transform all the connectivities.*/


#define TRANSFORM_CONNECTIVITY(MA_TYPENAME,MA_NODESPERELEM) \
	    for(i=0;i<MA_NODESPERELEM* \
		      g_unstrGlobals->m_ ## MA_TYPENAME ## ElementsPerPartPerTimestep[a_whichPart][a_whichTimestep];i++) \
	      { \
		g_unstrGlobals->m_ ## MA_TYPENAME ## ConnectivityPerPartPerTimestep[a_whichPart][a_whichTimestep][i] = \
		  l_xform[ \
		    g_unstrGlobals->m_ ## MA_TYPENAME ## ConnectivityPerPartPerTimestep[a_whichPart][a_whichTimestep][i]]; \
	      }
	TRANSFORM_CONNECTIVITY(hex,8)
	TRANSFORM_CONNECTIVITY(quad,4) 
	TRANSFORM_CONNECTIVITY(tet,4)
	TRANSFORM_CONNECTIVITY(tri,3)
	TRANSFORM_CONNECTIVITY(pyramid,5)
	TRANSFORM_CONNECTIVITY(point,1)
	TRANSFORM_CONNECTIVITY(prism,6)
	TRANSFORM_CONNECTIVITY(line,2)
	/*TRANSFORM_CONNECTIVITY(0ball,NUM_NODES_IN_0BALL) not in sslib? */
	TRANSFORM_CONNECTIVITY(1ball,NUM_NODES_IN_1BALL)
	TRANSFORM_CONNECTIVITY(2ball,NUM_NODES_IN_2BALL)
	TRANSFORM_CONNECTIVITY(3ball,NUM_NODES_IN_3BALL)
  	if(g_unstrGlobals->m_useQuadraticElements)
	{
	  TRANSFORM_CONNECTIVITY(quadraticHex,20)
	  TRANSFORM_CONNECTIVITY(quadraticQuad,8)
	  TRANSFORM_CONNECTIVITY(quadraticTet,10)
	  TRANSFORM_CONNECTIVITY(quadraticTri,6)
	  TRANSFORM_CONNECTIVITY(quadraticPyramid,13)
	  TRANSFORM_CONNECTIVITY(quadraticPrism,15)
	  TRANSFORM_CONNECTIVITY(quadraticLine,3)
	}
#undef TRANSFORM_CONNECTIVITY



	  }



	{
	  /*check whether we should reduce or not*/
	  int l_doReduce = 1;


	  /*If the nodes are on the part set itself, it is unlikely that reducing will do anything*/
	  if(g_unstrGlobals->m_setIndexPerPart[a_whichPart] == l_setIndex) l_doReduce=0;
	    
	  if(l_doReduce)
	    {
	      /*Check that it is a simple topo rel case, otherwise we dont want to reduce*/
	      int l_numPolys=0, l_numRels=0;
	      SAF_Rel *l_rels=NULL;
	      l_doReduce=0;

	      l_numRels = get_num_topo_rels_in_set( g_unstrGlobals->m_allSets[g_unstrGlobals->m_setIndexPerPart[a_whichPart]], 
						    SAF_CELLTYPE_ANY, &l_numPolys, &l_rels, 0  );
	      if( l_numPolys )
		{
		  for(i=0;i<l_numRels;i++)
		    {
		      SAF_Set l_rangeSet=SS_SET_NULL;
		      saf_describe_topo_relation(SAF_ALL,&(l_rels[i]),NULL,NULL,&l_rangeSet,NULL,NULL,NULL,NULL);
		      if( SAF_EQUIV(&l_rangeSet,&(g_unstrGlobals->m_allSets[l_setIndex])) )
			{
			  l_doReduce=1;
			  break;
			}
		    }
		}
	      if(l_rels) free(l_rels);
	    }

	  if( !l_doReduce)
	    {
	      printinfo("get_unstr_point_coords_used_by_part not reducing points for part %d\n",a_whichPart);
	    }
      


	  /*
	  ** This section is the whole reason behind USE_CONNECTIVITY_REDUCTION_AND_CACHE: reduce the data
	  */
	  if(l_doReduce && l_numPoints)
	    {
	      int l_num;
	      int *l_pointUsed = (int *)malloc(l_numPoints*sizeof(int));
	      int l_numPointsUsed = l_numPoints;
	      for(i=0;i<l_numPoints;i++) l_pointUsed[i]=-1;




#define MARK_USED_POINT(MA_TYPENAME,MA_NODESPERELEM) \
	      l_num = MA_NODESPERELEM* \
	              g_unstrGlobals->m_ ## MA_TYPENAME ## ElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]; \
	      for(i=0;i<l_num;i++) \
		{ \
		  l_pointUsed[ \
		   g_unstrGlobals->m_ ## MA_TYPENAME ## ConnectivityPerPartPerTimestep[a_whichPart][a_whichTimestep][i]]=1; \
		}

	MARK_USED_POINT(hex,8)
	MARK_USED_POINT(quad,4) 
	MARK_USED_POINT(tet,4)
	MARK_USED_POINT(tri,3)
	MARK_USED_POINT(pyramid,5)
	MARK_USED_POINT(point,1)
	MARK_USED_POINT(prism,6)
	MARK_USED_POINT(line,2)
	/*MARK_USED_POINT(0ball,NUM_NODES_IN_0BALL) not in sslib?*/
	MARK_USED_POINT(1ball,NUM_NODES_IN_1BALL)
	MARK_USED_POINT(2ball,NUM_NODES_IN_2BALL)
	MARK_USED_POINT(3ball,NUM_NODES_IN_3BALL)
  	if(g_unstrGlobals->m_useQuadraticElements)
	{
	  MARK_USED_POINT(quadraticHex,20)
	  MARK_USED_POINT(quadraticQuad,8)
	  MARK_USED_POINT(quadraticTet,10)
	  MARK_USED_POINT(quadraticTri,6)
	  MARK_USED_POINT(quadraticPyramid,13)
	  MARK_USED_POINT(quadraticPrism,15)
	  MARK_USED_POINT(quadraticLine,3)
	}
#undef MARK_USED_POINT





	  
	      /*02jan2004 now change l_pointUsed from "-1:not used, 1:used" to "-1:not used, n:new index",
		and count the final number of points*/
	      {
		int l_count=0;
		for(i=0;i<l_numPoints;i++) if(l_pointUsed[i]>=0) l_pointUsed[i] = l_count++;
		l_numPointsUsed=l_count;
	      }

	  



#define REDUCE_CONNECTIVITY(MA_TYPENAME,MA_NODESPERELEM) \
	      l_num = MA_NODESPERELEM* \
		       g_unstrGlobals->m_ ## MA_TYPENAME ## ElementsPerPartPerTimestep[a_whichPart][a_whichTimestep]; \
	      for(i=0;i<l_num;i++) \
		{ \
		  g_unstrGlobals->m_ ## MA_TYPENAME ## ConnectivityPerPartPerTimestep[a_whichPart][a_whichTimestep][i] = \
		    l_pointUsed[ \
		     g_unstrGlobals->m_ ## MA_TYPENAME ## ConnectivityPerPartPerTimestep[a_whichPart][a_whichTimestep][i]]; \
		}

	REDUCE_CONNECTIVITY(hex,8)
	REDUCE_CONNECTIVITY(quad,4) 
	REDUCE_CONNECTIVITY(tet,4)
	REDUCE_CONNECTIVITY(tri,3)
	REDUCE_CONNECTIVITY(pyramid,5)
	REDUCE_CONNECTIVITY(point,1)
	REDUCE_CONNECTIVITY(prism,6)
	REDUCE_CONNECTIVITY(line,2)
	/*REDUCE_CONNECTIVITY(0ball,NUM_NODES_IN_0BALL) not in sslib?*/
	REDUCE_CONNECTIVITY(1ball,NUM_NODES_IN_1BALL)
	REDUCE_CONNECTIVITY(2ball,NUM_NODES_IN_2BALL)
	REDUCE_CONNECTIVITY(3ball,NUM_NODES_IN_3BALL)
  	if(g_unstrGlobals->m_useQuadraticElements)
	{
	  REDUCE_CONNECTIVITY(quadraticHex,20)
	  REDUCE_CONNECTIVITY(quadraticQuad,8)
	  REDUCE_CONNECTIVITY(quadraticTet,10)
	  REDUCE_CONNECTIVITY(quadraticTri,6)
	  REDUCE_CONNECTIVITY(quadraticPyramid,13)
	  REDUCE_CONNECTIVITY(quadraticPrism,15)
	  REDUCE_CONNECTIVITY(quadraticLine,3)
	}
#undef REDUCE_CONNECTIVITY









	      /*02jan2004 now reduce the points list */
	      /*reallocate and reduce*/
	      if(g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep])
		{
		  MY_PRECISION *l_ptr = (MY_PRECISION *)malloc(3*l_numPointsUsed*sizeof(MY_PRECISION));
		  int l_num=0;
		  for(i=0;i<l_numPoints;i++) if(l_pointUsed[i]>=0)
		    {
		      l_ptr[3*l_num+0] = g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep][3*i+0];
		      l_ptr[3*l_num+1] = g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep][3*i+1];
		      l_ptr[3*l_num+2] = g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep][3*i+2];
		      l_num++;
		    }

		  free(g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep]);
		  g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep] = l_ptr;
		}


	      if( l_numPointsUsed != l_numPoints )
		{
		  /*update the number of points   02jan2004*/
		  g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep] = l_numPointsUsed;


		  /*save the transform  02jan2004*/
		  g_unstrGlobals->m_pointsTransformPerPartPerTimestep[a_whichPart][a_whichTimestep] = l_pointUsed;
		}
	      else 
		{
		  free(l_pointUsed);
		  printinfo("get_unstr_point_coords_used_by_part not reducing points for part %d - all pts used\n",a_whichPart);
		}

	    }


	}






	/*we just cached the data, now just return it*/
	if(a_data && g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep])
	  {
	    memcpy( a_data,
		    g_unstrGlobals->m_pointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep],
		    3*g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep]*sizeof(MY_PRECISION)
		    );
	  }
	return( g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][a_whichTimestep] );

      }

   

#else /*USE_CONNECTIVITY_REDUCTION_AND_CACHE*/

 /*this was unreachable 11/4/04 ... changed to #else  .... hooey*/

      l_numPoints = get_unstr_point_coords_in_set( g_unstrGlobals->m_allSets[l_setIndex], a_data, a_whichTimestep );


      return(l_numPoints);
#endif /*USE_CONNECTIVITY_REDUCTION_AND_CACHE*/
    }
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Are this Part's Coordinates Time-independent?
 *
 * Description: 
 *
 * Answers the question: if we call get_unstr_point_coords_used_by_part() for the
 * same part but with different timesteps, will we get the same data?
 * Return -1 for error, 0 for no, 1 for yes.
 *
 * Note this is currently very strict, in that it could return no when
 * the answer is really yes.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
are_unstr_point_coords_used_by_part_time_independent( int a_whichPart )
{
  int l_setIndex=-1,i;
  if(!is_part_arg_valid(a_whichPart)) return(-1);


  /*First: check that the sets for each time are the same. If not, exit early with a "No"*/
  l_setIndex = g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[a_whichPart][0];
  for(i=1;i<get_num_timesteps();i++)
    {
      if( l_setIndex != g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[a_whichPart][i] ) return(0);
    }
  if( l_setIndex < 0 )
    {
      printinfo("are_unstr_point_coords_used_by_part_time_independent error: l_setIndex<0 for part %d\n",a_whichPart);
      return(-1);
    }

  /*Next: find the default coords field for this set, and verify that it is not associated with a time.
   *If this is true, then the coords are constant, and return "Yes"
   *Note this could be false and the coords still be constant, but we are ignoring that
   *possibility for now*/
  {
    int l_time=-1;
    SAF_Field l_defaultCoordField;

    saf_find_default_coords(SAF_ALL,&(g_unstrGlobals->m_allSets[l_setIndex]),&l_defaultCoordField);
    if(!my_saf_is_valid_field_handle(&l_defaultCoordField)) return(-1);
    l_time=get_timestep_for_field(l_defaultCoordField);
    if(l_time<0)
      {
	return(1);/*yes, the coords are constant over time*/
      }
  }

  return(0);
}












/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Set that has this Set's Nodes
 *
 * Description: 
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set 
UNSTR_CLASS_NAME_IF_CPP
on_which_other_set_are_this_sets_nodes_stored(SAF_Set a_set, int a_timestep, int **a_xform)
{
  int i, l_numPolys=0, l_numRels=0;
  SAF_Rel *l_rels=NULL;
  SAF_Set l_topoRangeSource=SS_SET_NULL;
  l_numRels = get_num_topo_rels_in_set( a_set, SAF_CELLTYPE_ANY, &l_numPolys, &l_rels, 0  );

  /*printinfo("*****on_which_other_set_are_this_sets_nodes_stored a_time=%d l_numRels=%d l_numPolys=%d\n",
    a_timestep,l_numRels,l_numPolys);*/

  if( l_numPolys )
    {
      for(i=0;i<l_numRels;i++)
	{
	  SAF_Set l_rangeSet=SS_SET_NULL;
	  int l_found=0;	  
	  saf_describe_topo_relation(SAF_ALL,&(l_rels[i]),NULL,NULL,&l_rangeSet,NULL,NULL, NULL,NULL);
	  l_found = does_this_set_have_unstr_point_coords_for_this_time(l_rangeSet, a_timestep, 3);
	  if(!l_found)
	    {
	      l_found = does_this_set_have_unstr_point_coords_for_this_time(l_rangeSet, a_timestep, 2);
	    }
	  if(l_found) 
	    {
	      if( my_saf_is_valid_set_handle(&l_topoRangeSource) && !SAF_EQUIV(&l_topoRangeSource,&l_rangeSet) )
		{
		  printinfo("on_which_other_set_are_this_sets_nodes_stored error: found points from topo rel in >1 set. Exiting\n");
		  exit(-1);
		}
	      l_topoRangeSource = l_rangeSet;
	    }
	}
    }



  if( l_numRels && !my_saf_is_valid_set_handle(&l_topoRangeSource) )
    {/*There are topo rels but we did not find one that connects to a set
       with nodes. Maybe it is a topo rel connected to a subset of nodes?*/
      int j,k;
      for(k=0;k<l_numRels;k++)
	{
	  SAF_Cat l_rangeCat=SS_CAT_NULL;
	  SAF_Set l_rangeSet=SS_SET_NULL;
	  saf_describe_topo_relation(SAF_ALL,&(l_rels[k]),NULL,NULL,&l_rangeSet,&l_rangeCat,NULL,NULL,NULL);

	  for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
	    {
	      if( !SAF_EQUIV(&l_rangeCat,&(g_unstrGlobals->m_subsetTransforms[0][i].m_cat)) ) continue;
		  
	      for(j=1;j<g_unstrGlobals->m_subsetTransforms[0][i].m_length;j++)
		{
		  if(SAF_EQUIV(&l_rangeSet,&(g_unstrGlobals->m_subsetTransforms[0][i].m_sets[j])))
		    {
		      /*Note: assuming there will be only one topo rel that fits*/

		      if(a_xform) a_xform[0] = g_unstrGlobals->m_subsetTransforms[0][i].m_transform[j];

#if 0 /*should it be the top set or the range set?*/
		      l_topoRangeSource = l_rangeSet;
#else
		      l_topoRangeSource = g_unstrGlobals->m_subsetTransforms[0][i].m_sets[0];
#endif
		      break;
		    }
		}
	      if( my_saf_is_valid_set_handle(&l_topoRangeSource) ) break;
	    }
	}
    }


#if 1
  /*Check that we can indeed read points from this set*/
  if(my_saf_is_valid_set_handle(&l_topoRangeSource))
    {
      if(!get_unstr_point_coords_in_set( l_topoRangeSource, NULL, a_timestep ))
	{
	  /*
	    char *l_setName=NULL, *l_setName2=NULL;
	    saf_describe_set(SAF_ALL, &l_topoRangeSource, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
	    saf_describe_set(SAF_ALL, &a_set, &l_setName2, NULL, NULL, NULL, NULL, NULL, NULL ); 
	    printinfo("       warning time %d, set %s should have nodes on set %s but doesnt?\n", 
	    a_timestep,l_setName2,l_setName);
	    free(l_setName);
	    free(l_setName2);
	    printinfo("      warning replacing set with SS_SET_NULL\n");
	  */

	  /*By doing this, the nodes will later be found thru subset relations?*/
	  l_topoRangeSource=SS_SET_NULL;
	}
    }
#endif




#if 0 /*just printing*/
  if(my_saf_is_valid_set_handle(&l_topoRangeSource))
    {
      char *l_setName=NULL;
      saf_describe_set(SAF_ALL, &l_topoRangeSource, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
      printinfo("        got set %s\n",l_setName );
      free(l_setName);
    }
  else
    {
      printinfo("        got SS_SET_NULL\n");
    }
#endif





  if( l_numRels && l_rels ) free(l_rels);
  return(l_topoRangeSource);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Index of the Set Which Carries this Set's Coord Nodes
 *
 * Description: 
 * Set a_whichTimestep to <0 "for any time".
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
on_which_other_set_index_are_this_sets_nodes_stored(SAF_Set a_set, int a_timestep, int **a_xform)
{
  int i,l_setIndex=-1;
  SAF_Set l_topoRangeSource=SS_SET_NULL;
  l_topoRangeSource = on_which_other_set_are_this_sets_nodes_stored(a_set,a_timestep,a_xform);
  if( my_saf_is_valid_set_handle(&l_topoRangeSource) )
    {
      for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
	{
	  if( SAF_EQUIV(&l_topoRangeSource,&(g_unstrGlobals->m_allSets[i])) )
	    {
	      l_setIndex = i;
	      break;
	    }
	}

      if(l_setIndex<0)
	{
	  printinfo("error on_which_other_set_index_are_this_sets_nodes_stored couldnt find set in list. exiting\n");
	  exit(-1);
	}
    }
  return(l_setIndex);
} 




/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Does this Set Have Coordinates at a Given Time?
 *
 * Description: 
 * If a_whichTimestep is set to <0 "for any time", then the function will return
 * true if there are unstructured point coords on the set at any timestep.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
does_this_set_have_unstr_point_coords_for_this_time( SAF_Set a_set, int a_whichTimestep, int a_numDims )
{

  int l_numCoordFields=0,i;
  SAF_Field *l_coordFields=0; 

  get_all_fields_for_set( a_set, NULL,NULL,NULL, &l_numCoordFields,&l_coordFields);

  for(i=0;i<l_numCoordFields;i++)
    {
      SAF_Cat l_cat;
      SAF_IndexSpec l_indexSpec;
      int l_numComponents=0;

      if(!my_saf_is_valid_field_handle(&(l_coordFields[i]))) continue;

      saf_describe_field(SAF_ALL, &(l_coordFields[i]), NULL, NULL, NULL, NULL,
			 NULL, NULL, &l_cat, NULL, NULL, NULL, NULL,  
			 &l_numComponents, NULL, NULL, NULL);
      saf_describe_collection(SAF_ALL,&a_set,&l_cat,NULL,NULL,&l_indexSpec,NULL,NULL);


      { /*Added 12feb2004 with similar fix in get_num_3d_coord_points_in_set.
	  Does it really need to be here? Is this possible?*/
	SAF_Role l_role;
	int l_catDim=-1;
	saf_describe_category(SAF_ALL,&l_cat,NULL,&l_role,&l_catDim);
	if( SAF_EQUIV(&l_role,SAF_SPACE_SLICE) || SAF_EQUIV(&l_role,SAF_PARAM_SLICE) || l_catDim!=0 )
	  {
	    continue;
	  }
      }

      if( l_indexSpec.ndims==1 && l_numComponents==a_numDims )
	{
	  if( a_whichTimestep<0 || is_this_field_on_this_timestep(l_coordFields[i],a_whichTimestep) )
	    {
	      /*if(l_coordFields) free(l_coordFields); do not free, because of get_all_fields_for_set*/
	      return(1);
	    }
	}
    }
  /*if(l_coordFields) free(l_coordFields); do not free, because of get_all_fields_for_set*/
  return(0);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get Topo Relation Connectivity Data
 *
 * Description: 
 *
 * a_data must be already allocated, and will contain 0-based data,
 * a_set is the set of interest, where the field of interest is located 
 * a_pointSet is where the points that are referred to are actually stored,
 * a_numRels and a_rels are the known topo rels between a_set and a_pointSet 
 * a_cellType describes the type of topo rel we are looking for
 *
 * Issue: will this work ok if a_numRels>1? has there been an example of this?
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_topo_rel_connectivity( SAF_Set a_set, SAF_Set a_pointSet, int a_numRels, SAF_Rel *a_rels, 
			   int *a_data, SAF_CellType a_cellType, int a_isQuadraticCellType )
{
  int j,k;
  int *l_destPtr=a_data;

  if(!a_numRels || !a_rels || !a_data) return(0);


  for( j=0; j<a_numRels; j++ )
    {
      SAF_Set l_containingSet=SS_SET_NULL;
      SAF_Cat l_collOfPiecesSewnTogether=SS_CAT_NULL;
      SAF_Set l_rangeSet=SS_SET_NULL;
      SAF_Cat l_rangeColl=SS_CAT_NULL;
      SAF_Cat l_collOfSetWhereRelActuallyStored=SS_CAT_NULL;
      SAF_RelRep l_topoRelType;
      hid_t l_dataType;
      SAF_CellType l_rangeCellType;
      SAF_IndexSpec l_rangeIndexSpec;
      int l_numEntriesInRangeColl=0;      
      SAF_CellType l_containerCellType;
      SAF_IndexSpec l_containerIndexSpec;
      int l_numEntriesInContainerColl=0;
      SAF_CellType l_storedCellType;
      SAF_IndexSpec l_storedIndexSpec;
      int l_numEntriesInStoredColl=0;
      int l_rangeSetTopoDim,l_containerSetTopoDim;
      char *l_rangeSetName=0,*l_containerSetName=0;
      hbool_t l_isSelfStored;
      int *rabuf=0,*rbbuf=0;
      int l_numInB=0;
      int l_numPtsInType=-1;
      size_t l_abuf_sz=0,l_bbuf_sz=0;


      saf_is_self_stored_topo_relation(SAF_ALL,&(a_rels[j]),&l_isSelfStored);
      if( !l_isSelfStored )
	{
	  continue;
	}

      saf_describe_topo_relation(SAF_ALL,&(a_rels[j]),&l_containingSet,&l_collOfPiecesSewnTogether,
				 &l_rangeSet,&l_rangeColl,&l_collOfSetWhereRelActuallyStored,
				 &l_topoRelType, &l_dataType );
      
      if( !H5Tequal(l_dataType,SAF_INT) )
	{
	  printinfo("get_topo_rel_connectivity error:  l_dataType != SAF_INT, instead is: %s\n", get_dsl_type_string(l_dataType) );
	  continue; 
	}



      /*if( !SAF_EQUIV(&a_pointSet,&l_rangeSet) )
	{
	printinfo("NEW_SUBSET warning get_topo_rel_connectivity: a_pointSet=%s l_rangeSet=%s...............................\n",
	get_set_name(a_pointSet),get_set_name(l_rangeSet) );
	} */

 	  

      if( !SAF_EQUIV(&l_topoRelType, SAF_UNSTRUCTURED) && !SAF_EQUIV(&l_topoRelType, SAF_ARBITRARY) )
	{
	  printinfo("error get_topo_rel_connectivity: TOPO REL IS %s\n",get_topotype_desc(l_topoRelType));
	  exit(-1);
	} 
      saf_describe_set(SAF_ALL, &l_rangeSet, &l_rangeSetName, &l_rangeSetTopoDim, NULL, NULL, NULL, NULL, NULL );
      saf_describe_set(SAF_ALL, &l_containingSet, &l_containerSetName, &l_containerSetTopoDim, NULL, NULL, NULL, NULL, NULL );

      saf_describe_collection(SAF_ALL,&l_rangeSet,&l_rangeColl,&l_rangeCellType,&l_numEntriesInRangeColl,&l_rangeIndexSpec,NULL,NULL);
      saf_describe_collection(SAF_ALL,&l_containingSet,&l_collOfPiecesSewnTogether,&l_containerCellType,&l_numEntriesInContainerColl,
			      &l_containerIndexSpec,NULL,NULL);
      saf_describe_collection(SAF_ALL,&a_set,&l_collOfSetWhereRelActuallyStored,&l_storedCellType,&l_numEntriesInStoredColl,
			      &l_storedIndexSpec,NULL,NULL);
 
      if( l_containerIndexSpec.ndims != 1 )
	{
	  printinfo("error get_topo_rel_connectivity l_containerIndexSpec.ndims=%d exiting\n",l_containerIndexSpec.ndims);
	  exit(-1);
	}
      
 
      saf_get_count_and_type_for_topo_relation(SAF_ALL,&(a_rels[j]),NULL,NULL,&l_abuf_sz,NULL,&l_bbuf_sz,NULL);
	  
      l_numPtsInType=get_num_nodes_in_celltype(a_cellType,a_isQuadraticCellType);
      if(l_numPtsInType<=0)
      {
	      printinfo("error get_topo_rel_connectivity: not prepared for a_cellType=%s\n",
			get_celltype_desc(a_cellType) );
      }


      saf_read_topo_relation(SAF_ALL, &(a_rels[j]), NULL,(void**) &rabuf, (void**) &rbbuf);
	  


      /*printinfo("     rabuf[%d]: ",l_numEntriesInStoredColl);
	for(k=0;k<l_numEntriesInStoredColl;k++) printinfo("%d ",rabuf[k]);
	printinfo("\n");*/

      /*
	printinfo("     rabuf[%d]: ",l_numEntriesInStoredColl);
	for(k=0;k<l_numEntriesInStoredColl;k++) printinfo("%d ",rabuf[k]);
	printinfo("\n");

	printinfo("      rbbuf[%d]: ",l_bbuf_sz);
	for(k=0;k<l_bbuf_sz;k++) printinfo("%d ",rbbuf[k]);
	printinfo("\n");
      */



      /*Dont need to xform the subsets here anymore. Is being done in conjunction with point
	reduction elsewhere
	if( !SAF_EQUIV(&a_pointSet,&l_rangeSet) )
	{
	struct_subset_transform *l_sh = NULL;

	for(k=0;k<g_unstrGlobals->m_numAllSets;k++)
	{
	if(SAF_EQUIV(&(g_unstrGlobals->m_subsetTransforms[0][k].m_sets[0]), &a_pointSet))
	{
	l_sh = &g_unstrGlobals->m_subsetTransforms[0][k];
	break;
	}
	}
	if( l_sh )
	{
	transform_connectivity_list( l_rangeSet, *l_sh, l_bbuf_sz, rbbuf );
	}
	} 
      */
	  



 

      if( SAF_EQUIV(&l_topoRelType, SAF_UNSTRUCTURED) )
	{


	  /* 
	   * This was the condition that caused the dtest data set to fail 07/24/2003
	   */
	  if( rabuf[0] != l_numPtsInType )
	    {
	/*jake here*/ 
	      static int l_alreadyBeenHere=0;
	      if( !l_alreadyBeenHere )
		{
		  printinfo("\n");
		  printinfo("error: get_topo_rel_connectivity: l_numPtsInType=%d(%s) != rabuf[0]=%d,   l_abuf_sz=%d l_bbuf_sz=%d\n",
			    l_numPtsInType,get_celltype_desc(a_cellType),rabuf[0],l_abuf_sz,l_bbuf_sz);
		  printinfo("       using l_numPtsInType to avoid seg fault.....topo rel was written wrong?\n");
		  printinfo("       this was topo rel %d of %d\n",j,a_numRels);
		  printinfo("       l_rangeCellType=%s\n", get_celltype_desc(l_rangeCellType) );
		  printinfo("       l_containerCellType=%s\n", get_celltype_desc(l_containerCellType) );
		  printinfo("       l_storedCellType=%s\n", get_celltype_desc(l_storedCellType) );
		  printinfo("       NOT GOING TO PRINT ANY MORE errors LIKE THIS\n");
		  printinfo("\n");
		  l_alreadyBeenHere=1;
		}

	      if( rabuf[0] > l_numPtsInType ) rabuf[0] = l_numPtsInType;/*this prevents the seg fault*/
	      /*should I exit or continue?*/
	    }     
	      

	  l_numInB = l_numEntriesInContainerColl * rabuf[0];

	  /*now copy the b buffer to the correct place in the output array*/
	  memcpy(l_destPtr,rbbuf,l_numInB*sizeof(int));
	  l_destPtr += l_numInB;

	}
      else
	{ /*SAF_ARBITRARY*/
	  int *l_fromPtr=rbbuf;

	  for(k=0;k<(int)l_abuf_sz;k++)
	    {
	      if( l_numPtsInType == rabuf[k] ) 
		{
		  memcpy(l_destPtr,l_fromPtr,rabuf[k]*sizeof(int));
		  l_destPtr += rabuf[k];
		}
	      l_fromPtr += rabuf[k];
	      l_numInB += rabuf[k];
	    }
	}



      /*Check the indexspec of the RANGE set (i.e. where the indices are referring to)*/
      if( l_rangeIndexSpec.origins[0] )
	{/*is not 0-based: shift all down to 0-based*/

	  /*printinfo("get_topo_rel_connectivity shifting data from %d-based to 0-based SAF_IndexSpec, %d entries\n",
	    l_rangeIndexSpec.origins[0],l_numInB);*/

	  for(k=0;k<l_numInB;k++) a_data[k] -= l_rangeIndexSpec.origins[0];
	}
      /*else printinfo("get_topo_rel_connectivity NOT shifting data to 0-based\n");*/

      /*printinfo("get_topo_rel_connectivity SAF_IndexSpec origins: (%d %d %d)\n",
	l_containerIndexSpec.origins[0],
	l_rangeIndexSpec.origins[0],
	l_storedIndexSpec.origins[0]
	);*/



      /*added these 06jan2004: how did I miss them?*/
      if(rabuf) { free(rabuf); rabuf=NULL; }
      if(rbbuf) { free(rbbuf); rbbuf=NULL; }



      if(l_rangeSetName) { free(l_rangeSetName); l_rangeSetName=NULL; }
      if(l_containerSetName) { free(l_containerSetName); l_containerSetName=NULL; }
    }

  return(0);
}







/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get Connectivity for Elements for a Given Part, Timestep, and Cell Type
 *
 * Description: 
 * This macro creates instances of the function get_*_element_connectivity (hex, quad etc.).
 *
 * The argument a_data must be preallocated. The connectivity data is 0-based.
 *--------------------------------------------------------------------------------------------------- */

#define GET_ELEM_CONNECTIVITY(MA_TYPELOWERCASE,MA_SAFCELLTYPE,MA_NODESPERELEM,MA_ISQUADRATIC) \
int \
UNSTR_CLASS_NAME_IF_CPP \
get_ ## MA_TYPELOWERCASE ## _element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data ) \
{ \
  int l_setIndex=-1,l_nodeSetIndex=-1; \
  int l_numRels=0,l_numPolys=0; \
  SAF_Rel *l_rels=0; \
  if(!is_part_arg_valid(a_whichPart)) return(-1); \
  if(!is_time_arg_valid(a_whichTimestep)) return(-1); \
  if(!g_unstrGlobals->m_useQuadraticElements && MA_ISQUADRATIC) return(-1); \
 \
/*note this assumes that .... #ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE */ \
  { \
    int l_useThisTimestep=0; \
    if(g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart]) l_useThisTimestep=a_whichTimestep; \
    if(g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[a_whichPart][l_useThisTimestep]<0) \
      { \
	/*Call this without actually getting the points, in order to generate the cache. \
	  This is only necessary if the program happens to call get_*_connectivity before \
	  get_unstr_point_coords_used_by_part.*/ \
	get_unstr_point_coords_used_by_part( a_whichPart, l_useThisTimestep, NULL ); \
      } \
    if(g_unstrGlobals->m_ ## MA_TYPELOWERCASE ## ConnectivityPerPartPerTimestep[a_whichPart][l_useThisTimestep] && \
       g_unstrGlobals->m_ ## MA_TYPELOWERCASE ## ConnectivityPerPartPerTimestep[a_whichPart][l_useThisTimestep][0]>=0 \
       )/*i.e. is data valid yet*/ \
      { \
	memcpy(a_data, \
	       g_unstrGlobals->m_ ## MA_TYPELOWERCASE ## ConnectivityPerPartPerTimestep[a_whichPart][l_useThisTimestep], \
	       MA_NODESPERELEM* \
	       g_unstrGlobals->m_ ## MA_TYPELOWERCASE ## ElementsPerPartPerTimestep[a_whichPart][l_useThisTimestep]* \
	       sizeof(int) ); \
	return(0); \
      } \
  } \
/*#endif */ \
 \
  if(MA_SAFCELLTYPE==SAF_CELLTYPE_POINT) \
  { \
    /*for now: using all points, and indexing them in order*/ \
    int i,l_num=0; \
    l_num = get_num_point_elements_in_unstr_part( a_whichPart, a_whichTimestep ); \
    for(i=0;i<l_num;i++) a_data[i]=i; \
  } \
  else \
  { \
    l_setIndex = g_unstrGlobals->m_setIndexPerPart[a_whichPart]; \
    if( l_setIndex < 0 ) { return(0); } \
    l_numRels=get_num_topo_rels_in_set( g_unstrGlobals->m_allSets[l_setIndex], MA_SAFCELLTYPE, &l_numPolys, &l_rels, \
			MA_ISQUADRATIC ); \
    l_nodeSetIndex = g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[a_whichPart][a_whichTimestep]; \
    get_topo_rel_connectivity( g_unstrGlobals->m_allSets[l_setIndex], g_unstrGlobals->m_allSets[l_nodeSetIndex], \
			       l_numRels, l_rels, a_data, MA_SAFCELLTYPE, MA_ISQUADRATIC ); \
    if( l_numRels && l_rels ) free(l_rels); \
  } \
  return(0); \
} 

GET_ELEM_CONNECTIVITY(hex,SAF_CELLTYPE_HEX,8,0) /*creates get_hex_element_connectivity*/
GET_ELEM_CONNECTIVITY(quad,SAF_CELLTYPE_QUAD,4,0) /*creates get_quad_element_connectivity*/
GET_ELEM_CONNECTIVITY(tet,SAF_CELLTYPE_TET,4,0) /*creates get_tet_element_connectivity*/
GET_ELEM_CONNECTIVITY(tri,SAF_CELLTYPE_TRI,3,0) /*creates get_tri_element_connectivity*/
GET_ELEM_CONNECTIVITY(pyramid,SAF_CELLTYPE_PYRAMID,5,0) /*creates get_pyramid_element_connectivity*/
GET_ELEM_CONNECTIVITY(point,SAF_CELLTYPE_POINT,1,0) /*creates get_point_element_connectivity*/
GET_ELEM_CONNECTIVITY(prism,SAF_CELLTYPE_PRISM,6,0) /*creates get_prism_element_connectivity*/
GET_ELEM_CONNECTIVITY(line,SAF_CELLTYPE_LINE,2,0) /*creates get_line_element_connectivity*/
/*GET_ELEM_CONNECTIVITY(0ball,SAF_CELLTYPE_0BALL,NUM_NODES_IN_0BALL,0) not in sslib?*/
GET_ELEM_CONNECTIVITY(1ball,SAF_CELLTYPE_1BALL,NUM_NODES_IN_1BALL,0) /*creates get_1ball_element_connectivity*/
GET_ELEM_CONNECTIVITY(2ball,SAF_CELLTYPE_2BALL,NUM_NODES_IN_2BALL,0) /*creates get_2ball_element_connectivity*/
GET_ELEM_CONNECTIVITY(3ball,SAF_CELLTYPE_3BALL,NUM_NODES_IN_3BALL,0) /*creates get_3ball_element_connectivity*/

GET_ELEM_CONNECTIVITY(quadraticHex,SAF_CELLTYPE_HEX,20,1) /*creates get_quadraticHex_element_connectivity*/
GET_ELEM_CONNECTIVITY(quadraticQuad,SAF_CELLTYPE_QUAD,8,1) /*creates get_quadraticQuad_element_connectivity*/
GET_ELEM_CONNECTIVITY(quadraticTet,SAF_CELLTYPE_TET,10,1) /*creates get_quadraticTet_element_connectivity*/
GET_ELEM_CONNECTIVITY(quadraticTri,SAF_CELLTYPE_TRI,6,1) /*creates get_quadraticTri_element_connectivity*/
GET_ELEM_CONNECTIVITY(quadraticPyramid,SAF_CELLTYPE_PYRAMID,13,1) /*creates get_quadraticPyramid_element_connectivity*/
GET_ELEM_CONNECTIVITY(quadraticPrism,SAF_CELLTYPE_PRISM,15,1) /*creates get_quadraticPrism_element_connectivity*/
GET_ELEM_CONNECTIVITY(quadraticLine,SAF_CELLTYPE_LINE,3,1) /*creates get_quadraticLine_element_connectivity*/

#undef GET_ELEM_CONNECTIVITY






/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 *
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_unknown_elements_in_set( SAF_Set a_set )
{
  int l_numElems = 0;
  int l_num;

  l_num=0;
  get_num_topo_rels_in_set( a_set, SAF_CELLTYPE_MIXED, &l_num, NULL, 0  );
  if(l_num) printinfo("get_num_unknown_elements_in_set found %d SAF_CELLTYPE_MIXED\n",l_num);
  l_numElems += l_num;

  l_num=0;
  get_num_topo_rels_in_set( a_set, SAF_CELLTYPE_ARB, &l_num, NULL, 0  );
  if(l_num) printinfo("get_num_unknown_elements_in_set found %d SAF_CELLTYPE_ARB\n",l_num);
  l_numElems += l_num;

  l_num=0;
  get_num_topo_rels_in_set(a_set, SAF_CELLTYPE_SET, &l_num, NULL, 0  );
  if(l_num) printinfo("get_num_unknown_elements_in_set found %d SAF_CELLTYPE_SET\n",l_num);
  l_numElems += l_num;

  return(l_numElems);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of Unknown Elements in a Part and Timestep
 *
 * Description: 
 * Note that the elements counted are not necessarily "unknown", but could also be
 * "known" elements that are just not yet handled by this reader.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_unknown_elements_in_unstr_part( int a_whichPart, int a_whichTimestep )
{
  int l_setIndex = -1;
  int l_numElems = 0;

  if(!is_part_arg_valid(a_whichPart)) return(-1);
  if(!is_time_arg_valid(a_whichTimestep)) return(-1);


  l_setIndex = g_unstrGlobals->m_setIndexPerPart[a_whichPart];
  if( l_setIndex < 0 )
    {
      l_numElems=0;
    }
  else
    {
      int l_num;

      l_num=0;
      get_num_topo_rels_in_set( g_unstrGlobals->m_allSets[l_setIndex], SAF_CELLTYPE_MIXED, &l_num, NULL, 0  );
      if(l_num) printinfo("get_num_unknown_elements_in_unstr_part found %d SAF_CELLTYPE_MIXED\n",l_num);
      l_numElems += l_num;

      l_num=0;
      get_num_topo_rels_in_set( g_unstrGlobals->m_allSets[l_setIndex], SAF_CELLTYPE_ARB, &l_num, NULL, 0  );
      if(l_num) printinfo("get_num_unknown_elements_in_unstr_part found %d SAF_CELLTYPE_ARB\n",l_num);
      l_numElems += l_num;

      l_num=0;
      get_num_topo_rels_in_set( g_unstrGlobals->m_allSets[l_setIndex], SAF_CELLTYPE_SET, &l_num, NULL, 0  );
      if(l_num) printinfo("get_num_unknown_elements_in_unstr_part found %d SAF_CELLTYPE_SET\n",l_num);
      l_numElems += l_num;

    }

  return(l_numElems);
}







/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of Elements of a Given Cell Type in a Set
 *
 * Description: 
 * This macro is used to create all the instances of get_num_*_elements_in_set
 * (hex, quad, etc.).
 * Returns the number of SAF_CELLTYPE_* elements found in topology relations in a given set.
 *--------------------------------------------------------------------------------------------------- */
#define GET_NUM_ELEM_IN_SET(MA_TYPELOWERCASE,MA_SAFCELLTYPE,MA_ISQUADRATIC) \
int \
UNSTR_CLASS_NAME_IF_CPP \
get_num_ ## MA_TYPELOWERCASE ## _elements_in_set( SAF_Set a_set ) \
{ \
  int l_numElems=0; \
  if(!g_unstrGlobals->m_useQuadraticElements && MA_ISQUADRATIC) return(0); \
  get_num_topo_rels_in_set( a_set, MA_SAFCELLTYPE, &l_numElems, NULL, MA_ISQUADRATIC  ); \
  return(l_numElems); \
}

GET_NUM_ELEM_IN_SET(hex,SAF_CELLTYPE_HEX,0)
GET_NUM_ELEM_IN_SET(quad,SAF_CELLTYPE_QUAD,0)
GET_NUM_ELEM_IN_SET(tet,SAF_CELLTYPE_TET,0)
GET_NUM_ELEM_IN_SET(tri,SAF_CELLTYPE_TRI,0)
GET_NUM_ELEM_IN_SET(pyramid,SAF_CELLTYPE_PYRAMID,0)
/*handled differently? GET_NUM_ELEM_IN_SET(point,SAF_CELLTYPE_POINT,0)*/
GET_NUM_ELEM_IN_SET(prism,SAF_CELLTYPE_PRISM,0)
GET_NUM_ELEM_IN_SET(line,SAF_CELLTYPE_LINE,0)
/*GET_NUM_ELEM_IN_SET(0ball,SAF_CELLTYPE_0BALL,0) not in sslib?*/
GET_NUM_ELEM_IN_SET(1ball,SAF_CELLTYPE_1BALL,0)
GET_NUM_ELEM_IN_SET(2ball,SAF_CELLTYPE_2BALL,0)
GET_NUM_ELEM_IN_SET(3ball,SAF_CELLTYPE_3BALL,0)
GET_NUM_ELEM_IN_SET(mixed,SAF_CELLTYPE_MIXED,0)
GET_NUM_ELEM_IN_SET(quadraticTri,SAF_CELLTYPE_TRI,1)
GET_NUM_ELEM_IN_SET(quadraticLine,SAF_CELLTYPE_LINE,1)
GET_NUM_ELEM_IN_SET(quadraticQuad,SAF_CELLTYPE_QUAD,1)
GET_NUM_ELEM_IN_SET(quadraticTet,SAF_CELLTYPE_TET,1)
GET_NUM_ELEM_IN_SET(quadraticHex,SAF_CELLTYPE_HEX,1)
GET_NUM_ELEM_IN_SET(quadraticPrism,SAF_CELLTYPE_PRISM,1)
GET_NUM_ELEM_IN_SET(quadraticPyramid,SAF_CELLTYPE_PYRAMID,1)

#undef GET_NUM_ELEM_IN_SET



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of Elements of a Particular Cell Type in a Part and Timestep
 *
 * Description: 
 * This macro is used to create all the instances of get_num_*_elements_in_unstr_part
 * (hex, quad, etc.).
 * Returns the number of SAF_CELLTYPE_* elements found in topology relations in a given part and timestep.
 *--------------------------------------------------------------------------------------------------- */
#define GET_NUM_ELEM_IN_PART(MA_TYPELOWERCASE,MA_ISQUADRATIC) \
int \
UNSTR_CLASS_NAME_IF_CPP \
get_num_ ## MA_TYPELOWERCASE ## _elements_in_unstr_part( int a_whichPart, int a_whichTimestep ) \
{ \
  int l_setIndex = -1; \
  int l_numElems = 0; \
 \
  if(!is_part_arg_valid(a_whichPart)) return(-1); \
  if(!is_time_arg_valid(a_whichTimestep)) return(-1); \
  if(!g_unstrGlobals->m_useQuadraticElements && MA_ISQUADRATIC) return(0); \
 \
/*#ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE*/ \
  { \
    int l_useThisTimestep=0; \
    if(g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart]) l_useThisTimestep=a_whichTimestep; \
 \
    if(g_unstrGlobals->m_ ## MA_TYPELOWERCASE ## ElementsPerPartPerTimestep[a_whichPart][l_useThisTimestep]>=0) \
      { \
	return(g_unstrGlobals->m_ ## MA_TYPELOWERCASE ## ElementsPerPartPerTimestep[a_whichPart][l_useThisTimestep]); \
      } \
  } \
/*#endif*/ \
 \
  l_setIndex = g_unstrGlobals->m_setIndexPerPart[a_whichPart]; \
  if( l_setIndex < 0 ) \
    { \
      l_numElems=0; \
    } \
  else \
    { \
      l_numElems = get_num_ ## MA_TYPELOWERCASE ## _elements_in_set( g_unstrGlobals->m_allSets[l_setIndex] ); \
    } \
  return(l_numElems); \
}

GET_NUM_ELEM_IN_PART(hex,0)
GET_NUM_ELEM_IN_PART(quad,0)
GET_NUM_ELEM_IN_PART(tet,0)
GET_NUM_ELEM_IN_PART(tri,0)
GET_NUM_ELEM_IN_PART(pyramid,0)
/*handled differently? GET_NUM_ELEM_IN_PART(point)*/ /*creates get_num_point_elements_in_unstr_part*/
GET_NUM_ELEM_IN_PART(prism,0)
GET_NUM_ELEM_IN_PART(line,0)
/*GET_NUM_ELEM_IN_PART(0ball,0)*/
GET_NUM_ELEM_IN_PART(1ball,0)
GET_NUM_ELEM_IN_PART(2ball,0)
GET_NUM_ELEM_IN_PART(3ball,0)
/*GET_NUM_ELEM_IN_PART(mixed)*/
GET_NUM_ELEM_IN_PART(quadraticTri,1)
GET_NUM_ELEM_IN_PART(quadraticLine,1)
GET_NUM_ELEM_IN_PART(quadraticQuad,1)
GET_NUM_ELEM_IN_PART(quadraticTet,1)
GET_NUM_ELEM_IN_PART(quadraticHex,1)
GET_NUM_ELEM_IN_PART(quadraticPrism,1)
GET_NUM_ELEM_IN_PART(quadraticPyramid,1)

#undef GET_NUM_ELEM_IN_PART


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Number of Point Elements on a Part at a Given Time
 *
 * Description: 
 * 
 * All points store on the part set itself are considered "elements", and points
 * stored on other sets and accessed by relations are not.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_point_elements_in_unstr_part( int a_whichPart, int a_whichTimestep )
{
  int l_setIndex = -1;
  int l_numPoints = 0;

  if(!is_part_arg_valid(a_whichPart)) return(-1);
  if(!is_time_arg_valid(a_whichTimestep)) return(-1);


#ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE
  {
    int l_useThisTimestep=0;
    if(g_unstrGlobals->m_pointsAreTimeDependentPerPart[a_whichPart]) l_useThisTimestep=a_whichTimestep;

    if(g_unstrGlobals->m_pointElementsPerPartPerTimestep[a_whichPart][l_useThisTimestep]>=0)
      {
	return(g_unstrGlobals->m_pointElementsPerPartPerTimestep[a_whichPart][l_useThisTimestep]);
      }
  }
#endif

  l_setIndex = g_unstrGlobals->m_setIndexPerPart[a_whichPart];
  if( l_setIndex < 0 )
    {
      l_numPoints=0;
    }
  else
    {
      l_numPoints=get_num_points_in_set( g_unstrGlobals->m_allSets[l_setIndex], a_whichTimestep );
    }

  return(l_numPoints);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get Variable Fields Stored on a Set
 *
 * Description: 
 * Finds all the variable fields (i.e. not coordinate fields) stored ON a set that have the
 * same timestep as a_whichTimestep. If a_whichTimestep is negative, then the timestep of the fields 
 * is not considered. The array a_fields must be preallocated to size a_maxNum. If a_fields
 * is NULL, then no fields are kept, but the count is still returned.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_var_fields_on_set_itself( SAF_Set a_set, SAF_Cat a_cat, int a_whichTimestep, int a_maxNum, 
				  SAF_Field *a_fields, int *a_error )
{
  int i,l_numVars=0;
  int l_numFieldsOnPartSet=0;
  SAF_Field *l_fieldsOnPartSet=0; 
  int *l_times=0;
  if(a_error) *a_error=0;

  l_numFieldsOnPartSet = get_all_fields_for_set( a_set, &l_fieldsOnPartSet, &l_times, NULL, NULL, NULL );

  for(i=0;i<l_numFieldsOnPartSet;i++)
    {
      if( l_times[i]!=a_whichTimestep && a_whichTimestep>=0 ) continue;

      if( !SAF_EQUIV(&a_cat,SAF_ANY_CAT) )
	{
	  SAF_Cat l_thisCat;
	  saf_describe_field(SAF_ALL, &(l_fieldsOnPartSet[i]), NULL, NULL, NULL, NULL,
			     NULL, NULL, &l_thisCat, NULL, NULL, NULL, NULL,  
			      NULL, NULL, NULL, NULL);
	  if( !SAF_EQUIV(&a_cat,&l_thisCat) ) continue;
	}

      /*
	if( can_this_field_be_a_variable(a_set,l_fieldsOnPartSet[i]) &&
	is_this_field_on_this_timestep(l_fieldsOnPartSet[i],a_whichTimestep) )
      */

      if(a_fields)
	{
	  if( l_numVars<a_maxNum )
	    {
	      a_fields[l_numVars]=l_fieldsOnPartSet[i];
	    }
	  else
	    {
	      printinfo("\n*************************\n");
	      printinfo("get_num_var_fields_on_set_itself error l_numVars(%d)>=a_maxNum(%d)     time=%d g_maxNumUnstrVars=%d\n",
			l_numVars,a_maxNum, a_whichTimestep, g_unstrGlobals->m_maxNumUnstrVars );

	      /*6feb2004 this error is likely because you have >1 subsets of the part, each with
		a same-var field, so you will end up having >1 field instances for that part and var*/
#if 1
	      {/*print some more error info*/
		int j;
		int *l_vars=0,l_numc=0;
		char *l_setName=NULL, *l_catName=NULL;
		saf_describe_category(SAF_ALL,&a_cat,&l_catName,NULL,NULL);
		saf_describe_set(SAF_ALL, &a_set, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
		printinfo("    a_set: %s     a_cat: %s\n",l_setName,l_catName);
		get_all_fields_for_set( a_set, NULL, NULL,&l_vars, &l_numc,NULL); 
		printinfo("    l_numFieldsOnPartSet=%d\n",l_numFieldsOnPartSet);
		for(j=0;j<l_numFieldsOnPartSet;j++) 
		  {
		    if( l_times[j]>=0 || l_vars[j]>=0 )
		      {
			char *l_name=0;
			saf_describe_field(SAF_ALL, &(l_fieldsOnPartSet[j]), NULL, &l_name, NULL, NULL,
					   NULL, NULL, NULL, NULL, NULL, NULL, NULL,  
					   NULL, NULL, NULL, NULL);
			printinfo("         field %d: %s     time=%d    var=%d\n",j,l_name,l_times[j],l_vars[j]);
			free(l_name);
		      }
		  }
		printinfo("    num coord fld = %d\n",l_numc);
	      }
	      printinfo("*************************\n\n");
#endif


	      if(a_error) *a_error=1;
	      return(l_numVars);

	    }
	}	    
      l_numVars++;			   
    }


  return(l_numVars);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Name of a SAF_CellType
 *
 * Description: 
 * 
 * Returns the name of a SAF_CellType, e.g. "SAF_CELLTYPE_HEX".
 *
 *--------------------------------------------------------------------------------------------------- */
const char *
UNSTR_CLASS_NAME_IF_CPP
get_celltype_desc( SAF_CellType a_type )
{
  if( a_type == SAF_CELLTYPE_POINT ) return("SAF_CELLTYPE_POINT");
  if( a_type == SAF_CELLTYPE_HEX ) return("SAF_CELLTYPE_HEX");
  if( a_type == SAF_CELLTYPE_LINE ) return("SAF_CELLTYPE_LINE");
  if( a_type == SAF_CELLTYPE_TRI ) return("SAF_CELLTYPE_TRI");
  if( a_type == SAF_CELLTYPE_QUAD ) return("SAF_CELLTYPE_QUAD");
  if( a_type == SAF_CELLTYPE_TET ) return("SAF_CELLTYPE_TET");
  if( a_type == SAF_CELLTYPE_PYRAMID ) return("SAF_CELLTYPE_PYRAMID");
  if( a_type == SAF_CELLTYPE_PRISM ) return("SAF_CELLTYPE_PRISM");
  /*if( a_type == SAF_CELLTYPE_0BALL ) return("SAF_CELLTYPE_0BALL"); not in sslib?*/
  if( a_type == SAF_CELLTYPE_1BALL ) return("SAF_CELLTYPE_1BALL");
  if( a_type == SAF_CELLTYPE_2BALL ) return("SAF_CELLTYPE_2BALL");
  if( a_type == SAF_CELLTYPE_3BALL ) return("SAF_CELLTYPE_3BALL");
  if( a_type == SAF_CELLTYPE_MIXED ) return("SAF_CELLTYPE_MIXED");
  if( a_type == SAF_CELLTYPE_ARB ) return("SAF_CELLTYPE_ARB");
  if( a_type == SAF_CELLTYPE_SET ) return("SAF_CELLTYPE_SET");
  if( a_type == SAF_CELLTYPE_ANY ) return("SAF_CELLTYPE_ANY");
  return("SAF_CELLTYPE_unknown");
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 *
 * Description: 
 * 
 * 
 *
 *--------------------------------------------------------------------------------------------------- */
char *
UNSTR_CLASS_NAME_IF_CPP
get_role_desc( SAF_Role a_role )
{
  char *l_name=NULL;
  if( SAF_EQUIV(&a_role,SAF_SPACE_SLICE) ) return(strdup("(SAF_SPACE_SLICE)"));
  if( SAF_EQUIV(&a_role,SAF_PARAM_SLICE) ) return(strdup("(SAF_PARAM_SLICE)"));
  if( SAF_EQUIV(&a_role,SAF_TOPOLOGY) ) return(strdup("(SAF_TOPOLOGY)"));
  if( SAF_EQUIV(&a_role,SAF_PROCESSOR) ) return(strdup("(SAF_PROCESSOR)"));
  if( SAF_EQUIV(&a_role,SAF_BLOCK) ) return(strdup("(SAF_BLOCK)"));
  if( SAF_EQUIV(&a_role,SAF_DOMAIN) ) return(strdup("(SAF_DOMAIN)"));
  if( SAF_EQUIV(&a_role,SAF_ASSEMBLY) ) return(strdup("(SAF_ASSEMBLY)"));
  if( SAF_EQUIV(&a_role,SAF_MATERIAL) ) return(strdup("(SAF_MATERIAL)"));
	    
  saf_describe_role(SAF_ALL,&a_role, &l_name, NULL);
  return(l_name);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Name of a SAF_RelRep
 *
 * Description: 
 *
 * Returns the name of a SAF_RelRep, e.g. "SAF_UNSTRUCTURED".
 * 
 *--------------------------------------------------------------------------------------------------- */
#ifdef __cplusplus 
extern "C" {
# endif
const char *get_topotype_desc( SAF_RelRep a_type )
{
  if( SAF_EQUIV(&a_type, SAF_UNSTRUCTURED) ) return("SAF_UNSTRUCTURED");
  if( SAF_EQUIV(&a_type, SAF_STRUCTURED) ) return("SAF_STRUCTURED");
  if( SAF_EQUIV(&a_type, SAF_ARBITRARY) ) return("SAF_ARBITRARY");

  return("(unknown topo rel type?)");
}
# ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Get the Name of a SAF_Algebraic
 *
 * Description: 
 *
 * Returns the name of a SAF_Algebraic, e.g. "SAF_ALGTYPE_SCALAR".
 *
 *--------------------------------------------------------------------------------------------------- */
const char *
UNSTR_CLASS_NAME_IF_CPP
get_algebraictype_desc( SAF_Algebraic a_type )
{
  if( SAF_EQUIV(&a_type, SAF_ALGTYPE_SCALAR) ) return("SAF_ALGTYPE_SCALAR");
  if( SAF_EQUIV(&a_type, SAF_ALGTYPE_VECTOR) ) return("SAF_ALGTYPE_VECTOR");
  if( SAF_EQUIV(&a_type, SAF_ALGTYPE_TENSOR) ) return("SAF_ALGTYPE_TENSOR");
  if( SAF_EQUIV(&a_type, SAF_ALGTYPE_SYMTENSOR) ) return("SAF_ALGTYPE_SYMTENSOR");
  if( SAF_EQUIV(&a_type, SAF_ALGTYPE_COMPONENT) ) return("SAF_ALGTYPE_COMPONENT");
  if( SAF_EQUIV(&a_type, SAF_ALGTYPE_TUPLE) ) return("SAF_ALGTYPE_TUPLE");
  if( SAF_EQUIV(&a_type, SAF_ALGTYPE_FIELD) ) return("SAF_ALGTYPE_FIELD");
  if( SAF_EQUIV(&a_type, SAF_ALGTYPE_ANY) ) return("SAF_ALGTYPE_ANY");

  return("(unknown algebraic type?)");
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Can this Field be a Variable?
 *
 * Description: 
 * There are several conditions that might make a field NOT able to be a variable:
 * having an unsupported (i.e. not yet supported by this reader) algebraic type, having an unsupported 
 * celltype, and having never actually been written. Currently a coordinate field cannot
 * be a variable, though this might change.
 * 
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
can_this_field_be_a_variable( SAF_Set a_set, SAF_Field a_field )
{
  hbool_t l_isCoordField;
  hid_t l_varType;
  int l_numComponents=0,l_collcount=0;
  SAF_FieldTmpl l_fieldTemplate;
  SAF_CellType l_cellType;
  SAF_Cat l_cat;
  SAF_Algebraic l_algebraicType;    
  size_t l_count=0;
  hid_t l_dsltype;

  if(!my_saf_is_valid_field_handle(&a_field)) return(0);

  saf_describe_field(SAF_ALL, &a_field,  &l_fieldTemplate, NULL, NULL, NULL, 
		     &l_isCoordField, NULL, &l_cat, NULL, NULL, NULL, &l_varType,  
		     &l_numComponents, NULL, NULL, NULL);


#if 0
  {
    char *l_setName=0,*l_fieldName=0; 
    saf_describe_set(SAF_ALL, &a_set, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
    saf_describe_field(SAF_ALL, &a_field,  NULL, &l_fieldName, NULL, NULL,
		       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    printinfo("\ncan_this_field_be_a_variable testing field=%s set=%s\n",l_fieldName, l_setName);
    free(l_setName);
    free(l_fieldName);
  }
#endif


  if(l_isCoordField)
    {
      /*This setting means that coord fields cannot be variables. For SIERRA, this 
	appears to be a good assumption. Must keep an eye on this.*/
#if 0
      char *l_setName=0,*l_fieldName=0; 
      saf_describe_set(SAF_ALL, &a_set, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
      saf_describe_field(SAF_ALL, &a_field,  NULL, &l_fieldName, NULL, NULL,
			 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      printinfo("warning can_this_field_be_a_variable COORD field=%s set=%s\n",	l_fieldName, l_setName);
      free(l_setName);
      free(l_fieldName);
#endif
      return(0);
    }

  saf_describe_field_tmpl(SAF_ALL,&l_fieldTemplate,NULL,&l_algebraicType,NULL,NULL,NULL,NULL);

  /*Make sure we can handle this algebraic type. If not, print a message and code it in later.*/
  if( SAF_EQUIV(&l_algebraicType,SAF_ALGTYPE_SCALAR) ||
      SAF_EQUIV(&l_algebraicType,SAF_ALGTYPE_VECTOR) || 
      SAF_EQUIV(&l_algebraicType,SAF_ALGTYPE_TENSOR) || 
      SAF_EQUIV(&l_algebraicType,SAF_ALGTYPE_SYMTENSOR) )
    {
      /*ok*/
    }
  else
    {
#if 0
      char *l_setName=0,*l_fieldName=0; 
      int l_numComponents=0;
      saf_describe_set(SAF_ALL, &a_set, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
      saf_describe_field(SAF_ALL, &a_field,  NULL, &l_fieldName, NULL, NULL,
			 NULL, NULL, NULL, NULL, NULL, NULL, NULL, &l_numComponents, NULL, NULL, NULL, NULL);

      printinfo("can_this_field_be_a_variable doesnt understand alg type %s on field=%s set=%s numcomp=%d\n",
		get_algebraictype_desc(l_algebraicType), l_fieldName, l_setName, l_numComponents );

      if(l_setName) free(l_setName); 
      if(l_fieldName) free(l_fieldName); 
#endif
      return(0);
    }
		


  /*Make sure we can handle this celltype. If not, print a message and code it in later.
    Note this is where SAF_CELLTYPE_MIXED falls out.*/
  saf_describe_collection(SAF_ALL,&a_set,&l_cat,&l_cellType,&l_collcount,NULL,NULL,NULL);
  if( l_cellType==SAF_CELLTYPE_POINT || l_cellType==SAF_CELLTYPE_TRI ||
      l_cellType==SAF_CELLTYPE_QUAD || l_cellType==SAF_CELLTYPE_PYRAMID ||
      l_cellType==SAF_CELLTYPE_HEX || l_cellType==SAF_CELLTYPE_TET || 
      l_cellType==SAF_CELLTYPE_PRISM  || l_cellType==SAF_CELLTYPE_LINE 
      || l_cellType==SAF_CELLTYPE_MIXED 
      || l_cellType==SAF_CELLTYPE_3BALL 
      || l_cellType==SAF_CELLTYPE_2BALL 
      || l_cellType==SAF_CELLTYPE_1BALL 
      /*|| l_cellType==SAF_CELLTYPE_0BALL not in sslib?*/
      )
    {
      /*ok*/
    }
  else
    {

      /*try to limit the number of printed warnings*/
      {
	static SAF_CellType l_forType[10];
	static int l_numPrinted=0;
	int k=0, l_already=0;
	for(k=0;k<l_numPrinted;k++)
	  {
	    if( l_forType[k]==l_cellType )
	      {
		l_already=1;
		break;
	      }
	  }
	if( !l_already )
	  {
	    char *l_setName=0,*l_fieldName=0; 
	    saf_describe_set(SAF_ALL, &a_set, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
	    saf_describe_field(SAF_ALL, &a_field,  NULL, &l_fieldName, NULL, NULL,
			       NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			       NULL, NULL, NULL, NULL);
	    printinfo("\nwarning can_this_field_be_a_variable doesnt understand celltype %s field=%s set=%s count=%d\n\n",
		      get_celltype_desc(l_cellType), l_fieldName, l_setName,l_collcount);

	    if(l_setName) free(l_setName); 
	    if(l_fieldName) free(l_fieldName); 
	    if(l_numPrinted<10)
	      {
		l_forType[l_numPrinted++]=l_cellType;
	      }
	  }		  
      }
      return(0);
    }
    
		
  /*make sure the field can be read. This means no component/composite fields that are
    not directly written to will be found. Must add code in read_unstr_*_variable to
    figure out what the correct DSL type to pass to read_whole_field_to_my_precision is
    in order to change this*/
  saf_get_count_and_type_for_field(SAF_ALL,&a_field, NULL,&l_count,  &l_dsltype);
  if( l_count<=0 )
    {
#if 0
      printinfo("can_this_field_be_a_variable doesnt understand l_count=%d on field (probably component/composite)\n",
		l_count);
#endif
      return(0);
    }


#if 0 /*jake hooey test*/
  {
    char *l_roleName=NULL,*l_fieldName=NULL;
    SAF_Role l_role;
    saf_describe_field(SAF_ALL, &a_field,  NULL, &l_fieldName, NULL, NULL,
		       NULL, NULL, &l_cat, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    saf_describe_category(SAF_ALL,&l_cat,&l_roleName,&l_role,NULL);
    if( !SAF_EQUIV(&l_role,SAF_TOPOLOGY) )
      {
	printinfo("can_this_field_be_a_variable wont handle role=%s %s  on field %s\n",
		  l_roleName, get_role_desc(l_role), l_fieldName);
	free(l_roleName);
	return(0);
      }
    free(l_roleName);
  }
#endif

     
  {	      
    hbool_t l_writtenToField=false,l_writtenToCompField=false;

    saf_data_has_been_written_to_field(SAF_ALL, &a_field, &l_writtenToField);
	      
    if(!l_writtenToField)
      { /*note: if we call this and THERE IS data written to the field itself,
	  will there be a saf error? Need to fix this in saf, jake.*/
	saf_data_has_been_written_to_comp_field(SAF_ALL, &a_field, &l_writtenToCompField);
      }
	      
    if( l_writtenToField || l_writtenToCompField )
      {

#if 0
	/*printing every field that can be a variable*/
	{
	  char *l_setName=0,*l_fieldName=0,*l_catName=0,*l_roleName; 
	  int l_numComponents=0;
	  SAF_Cat l_cat;
          SAF_Role l_role;
	  saf_describe_set(SAF_ALL, &a_set, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
	  saf_describe_field(SAF_ALL, &a_field,  NULL, &l_fieldName, NULL, NULL,
			     NULL, NULL, &l_cat, NULL, NULL, NULL, NULL, &l_numComponents, NULL, NULL, NULL, NULL);
          saf_describe_category(SAF_ALL,&l_cat,&l_catName,&l_role,NULL);
          l_roleName = get_role_desc(l_role);

	  printinfo("can_this_field_be_a_variable:YES type=%s field=%s set=%s count=%d numcomp=%d %s cat=%s rolename=%s\n",
		    get_celltype_desc(l_cellType), l_fieldName, l_setName,l_collcount,l_numComponents,
		    (l_writtenToField ? "WRITTEN-TO-FIELD" : "WRITTEN-TO-COMP-FIELD"), l_catName, l_roleName );

	  free(l_setName); 
	  free(l_fieldName); 
	  free(l_catName); 
	  free(l_roleName); 
	}
#endif


	return(1);
      }
    else if( l_dsltype == SAF_HANDLE )
      {
	/*NOW WE HAVE THE FIELDS ON THIS SET....WHAT ABOUT THE FIELDS ON SUPERSETS? INDIRECT FIELDS?*/
	printinfo("can_this_field_be_a_variable error got a SAF_HANDLE: need to follow to another set!\n");
	/*wont find them here?*/
	exit(-1);
      }
  }
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Can this Set be a Valid Unstructured Part?
 *
 * Description: 
 *
 * If a set has unstructured elements (e.g. hex, line, quad) and either has point coordinates
 * on the set or has a relation with another set's coordinates, then it could be called an
 * unstructured part.
 *
 * If a_isPointsOnly is not NULL, then *a_isPointsOnly is set to 1 if the set is a valid unstructured
 * set and has self-stored points and no other elements, and is set to 0 otherwise.
 *
 * Return: 1 if true, 0 if false
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_a_valid_unstr_set( SAF_Set a_set, int *a_isPointsOnly )
{

/* #define PRINT_INFO_ON_ALL_TRIED_UNSTR_SETS */


  int l_hasItsOwnPoints=0;






#ifdef PRINT_INFO_ON_ALL_TRIED_UNSTR_SETS
  int l_addedWithNewMethod=0;
  {
    char *l_setName=NULL;
    saf_describe_set(SAF_ALL,  &a_set, &l_setName, NULL, NULL, NULL, NULL, NULL, NULL ); 
    printinfo("is_this_a_valid_unstr_set: set %s\n",l_setName);
    if(get_num_hex_elements_in_set(a_set)>0) printinfo("\t %d hex elements\n", get_num_hex_elements_in_set(a_set));
    if(get_num_quad_elements_in_set(a_set)>0) printinfo("\t %d quad elements\n", get_num_quad_elements_in_set(a_set));
    if(get_num_pyramid_elements_in_set(a_set)>0) printinfo("\t %d pyramid elements\n", get_num_pyramid_elements_in_set(a_set));
    if(get_num_tet_elements_in_set(a_set)>0) printinfo("\t %d tet elements\n", get_num_tet_elements_in_set(a_set));
    if(get_num_tri_elements_in_set(a_set)>0) printinfo("\t %d tri elements\n", get_num_tri_elements_in_set(a_set));
    if(get_num_prism_elements_in_set(a_set)>0) printinfo("\t %d prism elements\n", get_num_prism_elements_in_set(a_set));
    if(get_num_line_elements_in_set(a_set)>0) printinfo("\t %d line elements\n", get_num_line_elements_in_set(a_set));
    /*if(get_num_0ball_elements_in_set(a_set)>0) printinfo("\t %d 0ball elements\n", get_num_0ball_elements_in_set(a_set));*/
    if(get_num_1ball_elements_in_set(a_set)>0) printinfo("\t %d 1ball elements\n", get_num_1ball_elements_in_set(a_set));
    if(get_num_2ball_elements_in_set(a_set)>0) printinfo("\t %d 2ball elements\n", get_num_2ball_elements_in_set(a_set));
    if(get_num_3ball_elements_in_set(a_set)>0) printinfo("\t %d 3ball elements\n", get_num_3ball_elements_in_set(a_set));

    if(g_unstrGlobals->m_useQuadraticElements)
    {
      if( get_num_quadraticHex_elements_in_set(a_set)>0 ) printinfo("\t %d quadraticHex elements\n", 
								get_num_quadraticHex_elements_in_set(a_set));
      if( get_num_quadraticQuad_elements_in_set(a_set)>0 ) printinfo("\t %d quadraticQuad elements\n", 
								get_num_quadraticQuad_elements_in_set(a_set));
      if( get_num_quadraticPyramid_elements_in_set(a_set)>0 ) printinfo("\t %d quadraticPyramid elements\n", 
								get_num_quadraticPyramid_elements_in_set(a_set));
      if( get_num_quadraticTet_elements_in_set(a_set)>0 ) printinfo("\t %d quadraticTet elements\n", 
								get_num_quadraticTet_elements_in_set(a_set));
      if( get_num_quadraticTri_elements_in_set(a_set)>0 ) printinfo("\t %d quadraticTri elements\n", 
								get_num_quadraticTri_elements_in_set(a_set));
      if( get_num_quadraticPrism_elements_in_set(a_set)>0 ) printinfo("\t %d quadraticPrism elements\n", 
								get_num_quadraticPrism_elements_in_set(a_set));
      if( get_num_quadraticLine_elements_in_set(a_set)>0 ) printinfo("\t %d quadraticLine elements\n", 
								get_num_quadraticLine_elements_in_set(a_set));
    }

    if(get_num_unknown_elements_in_set(a_set)>0) printinfo("\t %d unknown elements\n", get_num_unknown_elements_in_set(a_set));
    free(l_setName);
  }
#endif







  if(a_isPointsOnly) *a_isPointsOnly=0;

  /*Check if this set either has coordinates or has a relationship with another
    set's coordinates*/
  /*passing -1 for time argument to say 'dont care'*/
  if( !does_this_set_have_unstr_point_coords_for_this_time(a_set,-1,3) &&
      !does_this_set_have_unstr_point_coords_for_this_time(a_set,-1,2) )
    {
      int *l_xform=NULL;
      SAF_Set l_otherSet = on_which_other_set_are_this_sets_nodes_stored(a_set,-1,&l_xform);


      if( !my_saf_is_valid_set_handle(&l_otherSet) )
	{
	  /* If we get here then this set doesnt have its own coords, and for some
	     reason doesnt know what set its nodes might be on
	  
	     **XXX jake hooey need to reexamine this 30jan2004
	     */
	  int i,j,l_numMatches=0;
	  int l_catIndex= 0; /*we know this because we always sort the cats so nodes is 0*/

	  for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
	    {
	      for(j=1;j<g_unstrGlobals->m_subsetTransforms[l_catIndex][i].m_length;j++)
		{
		  /*this is wrong. need something like a looser on_which_other_set_are_this_sets_nodes_stored
		    that lets me know there is a topo rel to another set, or to this set

		    All this does is determine whether there is a nodes subset rel between this set
		    and some other set.  MORE WORK TO DO HERE*/
		  if(SAF_EQUIV(&a_set,&(g_unstrGlobals->m_subsetTransforms[l_catIndex][i].m_sets[j])))
		    {
		      l_numMatches++;
		    }
		}
	    }
	  if(!l_numMatches)
	    {


#ifdef PRINT_INFO_ON_ALL_TRIED_UNSTR_SETS
	      printinfo("\t NOT a valid unstr set, no points and failed subset xform check\n");
#endif

	      return(0);
	    }
#ifdef PRINT_INFO_ON_ALL_TRIED_UNSTR_SETS
	  l_addedWithNewMethod=1;
#endif
	}

    }
  else
    {
      l_hasItsOwnPoints=1;
    }




  if( get_num_hex_elements_in_set(a_set)>0 ) return(1);
  if( get_num_quad_elements_in_set(a_set)>0 ) return(1);
  if( get_num_pyramid_elements_in_set(a_set)>0 ) return(1);
  if( get_num_tet_elements_in_set(a_set)>0 ) return(1);
  if( get_num_tri_elements_in_set(a_set)>0 ) return(1);
  if( get_num_prism_elements_in_set(a_set)>0 ) return(1);
  if( get_num_line_elements_in_set(a_set)>0 ) return(1);
  /*if( get_num_0ball_elements_in_set(a_set)>0 ) return(1);*/
  if( get_num_1ball_elements_in_set(a_set)>0 ) return(1);
  if( get_num_2ball_elements_in_set(a_set)>0 ) return(1);
  if( get_num_3ball_elements_in_set(a_set)>0 ) return(1);

  if(g_unstrGlobals->m_useQuadraticElements)
  {
    if( get_num_quadraticHex_elements_in_set(a_set)>0 ) return(1);
    if( get_num_quadraticQuad_elements_in_set(a_set)>0 ) return(1);
    if( get_num_quadraticPyramid_elements_in_set(a_set)>0 ) return(1);
    if( get_num_quadraticTet_elements_in_set(a_set)>0 ) return(1);
    if( get_num_quadraticTri_elements_in_set(a_set)>0 ) return(1);
    if( get_num_quadraticPrism_elements_in_set(a_set)>0 ) return(1);
    if( get_num_quadraticLine_elements_in_set(a_set)>0 ) return(1);
  }

  /*pass -1 for time argument to say 'dont care'*/
  if( get_num_points_in_set(a_set,-1)>0 )
    {

      if(a_isPointsOnly&&l_hasItsOwnPoints) *a_isPointsOnly=1;



#ifdef PRINT_INFO_ON_ALL_TRIED_UNSTR_SETS
      printinfo("\t is a valid unstr set because it has points (but no elems) %s\n",l_addedWithNewMethod?"":"PASSED SUBSET XFORM CHECK");
#endif



      return(1);/*same as point elements for now*/
    }


  {/*We dont handle SAF_CELLTYPE_MIXED yet: print out a warning. Fix it someday.*/
    int l_numMixed = get_num_mixed_elements_in_set(a_set);
    if( l_numMixed )
      {
	printinfo("warning: set has %d SAF_CELLTYPE_MIXED elements\n",l_numMixed);
      }
  }
 

  /*note: nothing is being added with this method*/



#ifdef PRINT_INFO_ON_ALL_TRIED_UNSTR_SETS
  /*if(l_addedWithNewMethod) printf("NEW_SUBSET did not add a new set\n");*/
  printinfo("\t is NOT a valid unstr set (no elems, no points) %s\n",l_addedWithNewMethod?"":"PASSED SUBSET XFORM CHECK");
#endif


  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     Finalize the Reader
 *
 * Description: 
 * Frees and resets all global variables.
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
finalize_unstr_mesh_reader( )
{
  int i,j;

  printinfo("in finalize_unstr_mesh_reader\n");

  if( g_unstrGlobals->m_setIndexPerPart )
    {
      for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
	{
	  if( g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i] ) 
	    {
	      free(g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i]);

	      /*do not free the int *, it is stored elsewhere*/
	      free(g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform[i]);
	    }

	  if(g_unstrGlobals->m_partNames && g_unstrGlobals->m_partNames[i])
	    {
	      free(g_unstrGlobals->m_partNames[i]);
	      g_unstrGlobals->m_partNames[i]=NULL;
	    }
	}
      if(g_unstrGlobals->m_partNames)
	{
	  free(g_unstrGlobals->m_partNames);
	  g_unstrGlobals->m_partNames=NULL;
	}
      free(g_unstrGlobals->m_setIndexPerPart);
      free(g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep);
      free(g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform); 
      g_unstrGlobals->m_setIndexPerPart=0;
      g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep=0;
      g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestepSubsetTransform=0; 
    }



  if( g_unstrGlobals->m_celltypePerVarPerPart )
    {
      for(i=0;i<g_unstrGlobals->m_maxNumUnstrVars;i++)
	{
	  free(g_unstrGlobals->m_celltypePerVarPerPart[i]);
	}
      free(g_unstrGlobals->m_celltypePerVarPerPart);
      g_unstrGlobals->m_celltypePerVarPerPart=NULL;
    }



    

#ifdef USE_CONNECTIVITY_REDUCTION_AND_CACHE

  if( g_unstrGlobals->m_pointsAreTimeDependentPerPart )
    {
      free(g_unstrGlobals->m_pointsAreTimeDependentPerPart); g_unstrGlobals->m_pointsAreTimeDependentPerPart=NULL;
    }

  /*If m_numPointsUsedPerPartPerTimestep is allocated, then everything is allocated (in init_unstr_mesh_reader)*/
  if( g_unstrGlobals->m_numPointsUsedPerPartPerTimestep )
    {
      for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
	{
	  for(j=0;j<get_num_timesteps();j++) 
	    {
	      free(g_unstrGlobals->m_pointsUsedPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_pointsTransformPerPartPerTimestep[i][j]);


	      free(g_unstrGlobals->m_pointConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_triConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_quadConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_tetConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_hexConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_pyramidConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_prismConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_lineConnectivityPerPartPerTimestep[i][j]);
	      /*free(g_unstrGlobals->m_0ballConnectivityPerPartPerTimestep[i][j]);*/
	      free(g_unstrGlobals->m_1ballConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_2ballConnectivityPerPartPerTimestep[i][j]);
	      free(g_unstrGlobals->m_3ballConnectivityPerPartPerTimestep[i][j]);

  	      if(g_unstrGlobals->m_useQuadraticElements)
	      {
		    free(g_unstrGlobals->m_quadraticTriConnectivityPerPartPerTimestep[i][j]);
		    free(g_unstrGlobals->m_quadraticQuadConnectivityPerPartPerTimestep[i][j]);
		    free(g_unstrGlobals->m_quadraticTetConnectivityPerPartPerTimestep[i][j]);
		    free(g_unstrGlobals->m_quadraticHexConnectivityPerPartPerTimestep[i][j]);
		    free(g_unstrGlobals->m_quadraticPyramidConnectivityPerPartPerTimestep[i][j]);
		    free(g_unstrGlobals->m_quadraticPrismConnectivityPerPartPerTimestep[i][j]);
		    free(g_unstrGlobals->m_quadraticLineConnectivityPerPartPerTimestep[i][j]);
	      }
	    }


	  free(g_unstrGlobals->m_numPointsUsedPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_pointsUsedPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_pointsTransformPerPartPerTimestep[i]);

	  free(g_unstrGlobals->m_pointElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_triElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_quadElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_tetElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_hexElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_pyramidElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_prismElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_lineElementsPerPartPerTimestep[i]);
	  /*free(g_unstrGlobals->m_0ballElementsPerPartPerTimestep[i]);*/
	  free(g_unstrGlobals->m_1ballElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_2ballElementsPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_3ballElementsPerPartPerTimestep[i]);

          if(g_unstrGlobals->m_useQuadraticElements)
	  {
	    free(g_unstrGlobals->m_quadraticTriElementsPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticQuadElementsPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticTetElementsPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticHexElementsPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticPyramidElementsPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticPrismElementsPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticLineElementsPerPartPerTimestep[i]);
	  }

	  free(g_unstrGlobals->m_pointConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_triConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_quadConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_tetConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_hexConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_pyramidConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_prismConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_lineConnectivityPerPartPerTimestep[i]);
	  /*free(g_unstrGlobals->m_0ballConnectivityPerPartPerTimestep[i]);*/
	  free(g_unstrGlobals->m_1ballConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_2ballConnectivityPerPartPerTimestep[i]);
	  free(g_unstrGlobals->m_3ballConnectivityPerPartPerTimestep[i]);

          if(g_unstrGlobals->m_useQuadraticElements)
	  {
	    free(g_unstrGlobals->m_quadraticTriConnectivityPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticQuadConnectivityPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticTetConnectivityPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticHexConnectivityPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticPyramidConnectivityPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticPrismConnectivityPerPartPerTimestep[i]);
	    free(g_unstrGlobals->m_quadraticLineConnectivityPerPartPerTimestep[i]);
	  }
	   
	}
      free(g_unstrGlobals->m_numPointsUsedPerPartPerTimestep); g_unstrGlobals->m_numPointsUsedPerPartPerTimestep=NULL;

      free(g_unstrGlobals->m_pointsUsedPerPartPerTimestep); g_unstrGlobals->m_pointsUsedPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_pointsTransformPerPartPerTimestep); g_unstrGlobals->m_pointsTransformPerPartPerTimestep=NULL;

      free(g_unstrGlobals->m_pointElementsPerPartPerTimestep); g_unstrGlobals->m_pointElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_triElementsPerPartPerTimestep); g_unstrGlobals->m_triElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_quadElementsPerPartPerTimestep); g_unstrGlobals->m_quadElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_tetElementsPerPartPerTimestep); g_unstrGlobals->m_tetElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_hexElementsPerPartPerTimestep); g_unstrGlobals->m_hexElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_pyramidElementsPerPartPerTimestep); g_unstrGlobals->m_pyramidElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_prismElementsPerPartPerTimestep); g_unstrGlobals->m_prismElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_lineElementsPerPartPerTimestep); g_unstrGlobals->m_lineElementsPerPartPerTimestep=NULL;
      /*free(g_unstrGlobals->m_0ballElementsPerPartPerTimestep); g_unstrGlobals->m_0ballElementsPerPartPerTimestep=NULL;*/
      free(g_unstrGlobals->m_1ballElementsPerPartPerTimestep); g_unstrGlobals->m_1ballElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_2ballElementsPerPartPerTimestep); g_unstrGlobals->m_2ballElementsPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_3ballElementsPerPartPerTimestep); g_unstrGlobals->m_3ballElementsPerPartPerTimestep=NULL;

      if(g_unstrGlobals->m_useQuadraticElements)
      {
        free(g_unstrGlobals->m_quadraticTriElementsPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticTriElementsPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticQuadElementsPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticQuadElementsPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticTetElementsPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticTetElementsPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticHexElementsPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticHexElementsPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticPyramidElementsPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticPyramidElementsPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticPrismElementsPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticPrismElementsPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticLineElementsPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticLineElementsPerPartPerTimestep=NULL;
      }


      free(g_unstrGlobals->m_pointConnectivityPerPartPerTimestep); g_unstrGlobals->m_pointConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_triConnectivityPerPartPerTimestep); g_unstrGlobals->m_triConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_quadConnectivityPerPartPerTimestep); g_unstrGlobals->m_quadConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_tetConnectivityPerPartPerTimestep); g_unstrGlobals->m_tetConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_hexConnectivityPerPartPerTimestep); g_unstrGlobals->m_hexConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_pyramidConnectivityPerPartPerTimestep); g_unstrGlobals->m_pyramidConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_prismConnectivityPerPartPerTimestep); g_unstrGlobals->m_prismConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_lineConnectivityPerPartPerTimestep); g_unstrGlobals->m_lineConnectivityPerPartPerTimestep=NULL;
      /*free(g_unstrGlobals->m_0ballConnectivityPerPartPerTimestep); g_unstrGlobals->m_0ballConnectivityPerPartPerTimestep=NULL;*/
      free(g_unstrGlobals->m_1ballConnectivityPerPartPerTimestep); g_unstrGlobals->m_1ballConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_2ballConnectivityPerPartPerTimestep); g_unstrGlobals->m_2ballConnectivityPerPartPerTimestep=NULL;
      free(g_unstrGlobals->m_3ballConnectivityPerPartPerTimestep); g_unstrGlobals->m_3ballConnectivityPerPartPerTimestep=NULL;

      if(g_unstrGlobals->m_useQuadraticElements)
      {
        free(g_unstrGlobals->m_quadraticTriConnectivityPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticTriConnectivityPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticQuadConnectivityPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticQuadConnectivityPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticTetConnectivityPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticTetConnectivityPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticHexConnectivityPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticHexConnectivityPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticPyramidConnectivityPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticPyramidConnectivityPerPartPerTimestep=NULL;
        free(g_unstrGlobals->m_quadraticPrismConnectivityPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticPrismConnectivityPerPartPerTimestep=NULL;
        free( g_unstrGlobals->m_quadraticLineConnectivityPerPartPerTimestep); 
	g_unstrGlobals->m_quadraticLineConnectivityPerPartPerTimestep=NULL;
      }

    }


#endif




  if(g_unstrGlobals->m_subsetTransforms)
    {
      for(j=0;j<g_unstrGlobals->m_subsetTransformsNumCats;j++)
	{
	  if(g_unstrGlobals->m_subsetTransforms[j])
	    {
	      for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
		{
		  delete_subset_transform( &g_unstrGlobals->m_subsetTransforms[j][i] );
		}
	      free(g_unstrGlobals->m_subsetTransforms[j]);
	    }
	}
      free(g_unstrGlobals->m_subsetTransforms);
      g_unstrGlobals->m_subsetTransforms=NULL;
    }






  if( g_unstrGlobals->m_instancePerTimestepPerVarPerPart )
    {
      for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) 
	{
	  if( g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i] && g_unstrGlobals->m_maxNumUnstrVars )
	    {
	      for(j=0;j<g_unstrGlobals->m_maxNumUnstrVars;j++)
		{
		  if(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j])
		    free( g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i][j] );
		}
	      free(g_unstrGlobals->m_instancePerTimestepPerVarPerPart[i]);
	    }
	}
      free( g_unstrGlobals->m_instancePerTimestepPerVarPerPart );
      g_unstrGlobals->m_instancePerTimestepPerVarPerPart=0;
    }


  g_unstrGlobals->m_numUnstrParts=0;


  if( g_unstrGlobals->m_unstrVarNames )
    {
      for(i=0;i<g_unstrGlobals->m_maxNumUnstrVars;i++)
	{
	  if( g_unstrGlobals->m_unstrVarNames[i] )
	    {
	      free(g_unstrGlobals->m_unstrVarNames[i]);
	      g_unstrGlobals->m_unstrVarNames[i]=0;
	    }
	}
      free(g_unstrGlobals->m_unstrVarNames);
      g_unstrGlobals->m_unstrVarNames=0;
    }



  if( g_unstrGlobals->m_unstrMeshVarUnitName )
    {
      for(i=0;i<g_unstrGlobals->m_maxNumUnstrVars;i++)
	{
	  if( g_unstrGlobals->m_unstrMeshVarUnitName[i] )
	    {
	      free(g_unstrGlobals->m_unstrMeshVarUnitName[i]);
	      g_unstrGlobals->m_unstrMeshVarUnitName[i]=0;
	    }
	}
      free(g_unstrGlobals->m_unstrMeshVarUnitName);
      g_unstrGlobals->m_unstrMeshVarUnitName=0;
    }
  if( g_unstrGlobals->m_unstrMeshVarQuantName )
    {
      for(i=0;i<g_unstrGlobals->m_maxNumUnstrVars;i++)
	{
	  if( g_unstrGlobals->m_unstrMeshVarQuantName[i] )
	    {
	      free(g_unstrGlobals->m_unstrMeshVarQuantName[i]);
	      g_unstrGlobals->m_unstrMeshVarQuantName[i]=0;
	    }
	}
      free(g_unstrGlobals->m_unstrMeshVarQuantName);
      g_unstrGlobals->m_unstrMeshVarQuantName=0;
    }


  g_unstrGlobals->m_maxNumUnstrVars = 0;

  if(g_unstrGlobals->m_allSetsFields)
    {
      for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
	{
	  if( g_unstrGlobals->m_allSetsFields[i] )
	    {
	      free(g_unstrGlobals->m_allSetsFields[i]);
	      g_unstrGlobals->m_allSetsFields[i]=0;
	    }
	}
      free(g_unstrGlobals->m_allSetsFields);
      g_unstrGlobals->m_allSetsFields=0;
    }

  if(g_unstrGlobals->m_allSetsCoordFields)
    {
      for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
	{
	  if( g_unstrGlobals->m_allSetsCoordFields[i] )
	    {
	      free(g_unstrGlobals->m_allSetsCoordFields[i]);
	      g_unstrGlobals->m_allSetsCoordFields[i]=0;
	    }
	}
      free(g_unstrGlobals->m_allSetsCoordFields);
      g_unstrGlobals->m_allSetsCoordFields=0;
    }

  if(g_unstrGlobals->m_allSetsFieldsTimestep)
    {
      for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
	{
	  if( g_unstrGlobals->m_allSetsFieldsTimestep[i] )
	    {
	      free(g_unstrGlobals->m_allSetsFieldsTimestep[i]);
	      g_unstrGlobals->m_allSetsFieldsTimestep[i]=0;
	    }
	}
      free(g_unstrGlobals->m_allSetsFieldsTimestep);
      g_unstrGlobals->m_allSetsFieldsTimestep=0;
    }


  if(g_unstrGlobals->m_allSetsFieldsVariable)
    {
      for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
	{
	  if( g_unstrGlobals->m_allSetsFieldsVariable[i] )
	    {
	      free(g_unstrGlobals->m_allSetsFieldsVariable[i]);
	      g_unstrGlobals->m_allSetsFieldsVariable[i]=0;
	    }
	}
      free(g_unstrGlobals->m_allSetsFieldsVariable);
      g_unstrGlobals->m_allSetsFieldsVariable=0;
    }


  if(g_unstrGlobals->m_allSetsNumFields)
    {
      free(g_unstrGlobals->m_allSetsNumFields);
      g_unstrGlobals->m_allSetsNumFields=0;
    }

  if(g_unstrGlobals->m_allSetsNumCoordFields)
    {
      free(g_unstrGlobals->m_allSetsNumCoordFields);
      g_unstrGlobals->m_allSetsNumCoordFields=0;
    }


  if(g_unstrGlobals->m_allSets)
    {
      free(g_unstrGlobals->m_allSets);
      g_unstrGlobals->m_allSets=0;
    }
  g_unstrGlobals->m_numAllSets=0;


  if( g_unstrGlobals->m_subsetsOfSetCacheMaxSize )
    {
      free(g_unstrGlobals->m_subsetsOfSetCacheSubsetRows); g_unstrGlobals->m_subsetsOfSetCacheSubsetRows=NULL;
      free(g_unstrGlobals->m_subsetsOfSetCacheSetRows); g_unstrGlobals->m_subsetsOfSetCacheSetRows=NULL;
      free(g_unstrGlobals->m_subsetsOfSetCacheCelltypes); g_unstrGlobals->m_subsetsOfSetCacheCelltypes=NULL;
      free(g_unstrGlobals->m_subsetsOfSetCacheNumRels); g_unstrGlobals->m_subsetsOfSetCacheNumRels=NULL;
      for(i=0;i<g_unstrGlobals->m_subsetsOfSetCacheSize;i++)
	{
	  if(g_unstrGlobals->m_subsetsOfSetCacheRels[i]) free(g_unstrGlobals->m_subsetsOfSetCacheRels[i]);
	}
      free(g_unstrGlobals->m_subsetsOfSetCacheRels); g_unstrGlobals->m_subsetsOfSetCacheRels=NULL;
    }
  g_unstrGlobals->m_subsetsOfSetCacheMaxSize=0;
  g_unstrGlobals->m_subsetsOfSetCacheSize=0;



#ifdef USE_TOPO_RELS_CACHE
  if( g_unstrGlobals->m_topoRelsCacheMaxSize )
    {

      /*printinfo(" m_topoRelsCache finalize, used %d of %d slots (%.2f)\n",g_unstrGlobals->m_topoRelsCacheSize,
		g_unstrGlobals->m_topoRelsCacheMaxSize,
		(float)g_unstrGlobals->m_topoRelsCacheSize/(float)g_unstrGlobals->m_topoRelsCacheMaxSize );*/

      free(g_unstrGlobals->m_topoRelsCacheNumElems); g_unstrGlobals->m_topoRelsCacheNumElems=NULL;
      free(g_unstrGlobals->m_topoRelsCacheSetRows); g_unstrGlobals->m_topoRelsCacheSetRows=NULL;
      free(g_unstrGlobals->m_topoRelsCacheCelltypes); g_unstrGlobals->m_topoRelsCacheCelltypes=NULL;
      free(g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic); g_unstrGlobals->m_topoRelsCacheCelltypeIsQuadratic=NULL;
      free(g_unstrGlobals->m_topoRelsCacheNumRels); g_unstrGlobals->m_topoRelsCacheNumRels=NULL;
      for(i=0;i<g_unstrGlobals->m_topoRelsCacheSize;i++)
	{
	  if(g_unstrGlobals->m_topoRelsCacheRels[i]) free(g_unstrGlobals->m_topoRelsCacheRels[i]);
	}
      free(g_unstrGlobals->m_topoRelsCacheRels); g_unstrGlobals->m_topoRelsCacheRels=NULL;
    }
  g_unstrGlobals->m_topoRelsCacheMaxSize=0;
  g_unstrGlobals->m_topoRelsCacheSize=0;
#endif



#ifdef ALLOW_MULTI_FIELDS_PER_VAR_PATCH
  if(g_unstrGlobals->m_multiFieldsList)
    {
      free(g_unstrGlobals->m_multiFieldsList);
      g_unstrGlobals->m_multiFieldsList=NULL;
      g_unstrGlobals->m_multiFieldsListMaxLength=0;
      g_unstrGlobals->m_multiFieldsListLength=0;
    }
#endif



  g_unstrGlobals->m_unstrMeshReaderIsInitialized=0;


  printinfo("leaving finalize_unstr_mesh_reader\n");

  return(0);
}







#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_timesteps()
{
  if(m_strReader)
    return m_strReader->get_num_timesteps();
  else
    {
      printinfo("error unstr's get_num_timesteps: no m_strReader?\n");
      exit(-1);
    }
}
#endif 


#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_str_mesh_reader_initialized()
{
  if(m_strReader)
    return m_strReader->is_str_mesh_reader_initialized();
  else
    {
      printinfo("error unstr's is_str_mesh_reader_initialized: no m_strReader?\n");
      exit(-1);
    }
}
#endif 

#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
are_these_two_fields_from_the_same_var(SAF_Field *a_fieldA, SAF_Field *a_fieldB)
{
  if(m_strReader)
    return m_strReader->are_these_two_fields_from_the_same_var(a_fieldA,a_fieldB); 
  else
    {
      printinfo("error unstr's are_these_two_fields_from_the_same_var: no m_strReader?\n");
      exit(-1);
    }
}
#endif 



#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
const char * 
UNSTR_CLASS_NAME_IF_CPP
get_dsl_type_string( hid_t a_varType )
{
  if(m_strReader)
    return m_strReader->get_dsl_type_string(a_varType);
  else
    {
      printinfo("error unstr's get_dsl_type_string: no m_strReader?\n");
      exit(-1);
    }
}
#endif 



#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
read_whole_field_to_my_precision( SAF_Field *a_field, hid_t a_varType, 
				  size_t a_numEntries, MY_PRECISION *a_data )
{
  if(m_strReader)
    return m_strReader->read_whole_field_to_my_precision(  a_field,  a_varType, 
							   a_numEntries, a_data );
  else
    {
      printinfo("error unstr's read_whole_field_to_my_precision: no m_strReader?\n");
      exit(-1);
    }
}
#endif 



#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_matching_sets(  SAF_Set **a_subsets, SAF_TopMode a_topMode, int a_topoDim,
		    int a_numColl, int a_collDim, SAF_CellType a_cellType,
		    int a_mustHaveAllTheStructuredMeshCollections )
{
  if(m_strReader)
    {
      return( m_strReader->get_matching_sets(a_subsets,a_topMode,a_topoDim,a_numColl,  
					     a_collDim,a_cellType,a_mustHaveAllTheStructuredMeshCollections ));
    }
  else
    {
      printinfo("error unstr's get_matching_sets: no m_strReader?\n");
      exit(-1);
    }
}
#endif 


#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_timestep_for_field(SAF_Field a_field)
{
  if(m_strReader)
    return m_strReader->get_timestep_for_field(a_field);
  else
    {
      printinfo("error unstr's get_timestep_for_field: no m_strReader?\n");
      exit(-1);
    }
}
#endif 




#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_this_field_on_this_timestep( SAF_Field a_field, int a_timestep )
{
  if(m_strReader)
    return m_strReader->is_this_field_on_this_timestep(  a_field,  a_timestep );
  else
    {
      printinfo("error unstr's is_this_field_on_this_timestep: no m_strReader?\n");
      exit(-1);
    }
}
#endif 



#ifdef __cplusplus
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
hid_t 
UNSTR_CLASS_NAME_IF_CPP
guess_dsl_var_type_for_composite_coord_field( int a_numFields, SAF_Field *a_fields )
{
  if(m_strReader)
    return m_strReader->guess_dsl_var_type_for_composite_coord_field( a_numFields, a_fields );
  else
    {
      printinfo("error unstr's guess_dsl_var_type_for_composite_coord_field: no m_strReader?\n");
      exit(-1);
    }
}
#endif 


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: Note the index applies to m_allSets
 *--------------------------------------------------------------------------------------------------- */
void 
UNSTR_CLASS_NAME_IF_CPP
get_list_of_supersets( SAF_Set a_set, SAF_Cat a_cat, int *a_numSupersets, int **a_supersetIndices )
{
  int i,j;
  int l_catIndex=-1;
  int l_num=0;
  a_numSupersets[0]=0;

  /*figure out the index for the given category*/
  for(i=0;i<g_unstrGlobals->m_subsetTransformsNumCats;i++)
    {
      if(SAF_EQUIV(&a_cat,&(g_unstrGlobals->m_subsetTransforms[i][0].m_cat)))
	{
	  l_catIndex=i;
	  break;
	}
    }
  if(l_catIndex<0) return;

  /*count the number of times the given set is in another set's subset tree*/
  for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
    {
      for(j=1;j<g_unstrGlobals->m_subsetTransforms[l_catIndex][i].m_length;j++)
	{
	  if(SAF_EQUIV(&a_set,&(g_unstrGlobals->m_subsetTransforms[l_catIndex][i].m_sets[j])))
	    {
	      l_num++;
	      break;
	    }
	}
    }
  if(!l_num) return;

  /*allocate and gather all the indices*/
  a_supersetIndices[0] = (int *)malloc(l_num*sizeof(int));
  l_num=0;
  for(i=0;i<g_unstrGlobals->m_numAllSets;i++)
    {
      for(j=1;j<g_unstrGlobals->m_subsetTransforms[l_catIndex][i].m_length;j++)
	{
	  if(SAF_EQUIV(&a_set,&(g_unstrGlobals->m_subsetTransforms[l_catIndex][i].m_sets[j])))
	    {
	      a_supersetIndices[0][l_num] = i;
	      l_num++;
	      break;
	    }
	}
    }

#if 0 /* printing only*/
  printinfo("\nget_list_of_supersets found %d for set %s cat %s\n",l_num,get_set_name(a_set),get_cat_name(a_cat));
  for(i=0;i<l_num;i++) printinfo("     %s    index=%d\n",
				 get_set_name( g_unstrGlobals->m_allSets[ a_supersetIndices[0][i] ] ),
				 a_supersetIndices[0][i]);
  printinfo("\n"); 
#endif


  a_numSupersets[0]=l_num;
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: Note the index applies to m_allSets
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_all_unstr_fields_for_set_and_cat( int a_setIndex, SAF_Cat *a_selectedCat, int a_whichTimestep, 
				      int a_maxNum, SAF_Field *a_fields )
{
  int i,j;
  int l_numCatsOnThisSet=0;
  int l_numVars=0,l_numVarsBefore=0;
  SAF_Cat *l_catsOnThisSet=NULL;
  SAF_Set l_set = g_unstrGlobals->m_allSets[a_setIndex];
  SAF_Field *l_fieldsPtr = NULL;

  if( SAF_EQUIV(a_selectedCat,SAF_ANY_CAT) )
    {
      saf_find_collections(SAF_ALL,&l_set,NULL/*sslib? SAF_EMPTY_HANDLE*/,SAF_CELLTYPE_ANY,SAF_ANY_TOPODIM,SAF_DECOMP_TORF,
			   &l_numCatsOnThisSet,&l_catsOnThisSet);
    }
  else
    {
      l_catsOnThisSet=(SAF_Cat *)malloc(1*sizeof(SAF_Cat));
      l_catsOnThisSet[0] = *a_selectedCat;
      l_numCatsOnThisSet=1;
    }

  for(i=0;i<g_unstrGlobals->m_subsetTransformsNumCats;i++)
    {
      int l_found=0;
      SAF_Cat l_cat = g_unstrGlobals->m_subsetTransforms[i][a_setIndex].m_cat;
      SAF_CellType l_cellType;
      int l_numEntriesInColl=0;
      int l_numSupersets=0;
      int *l_supersetIndices=NULL;
      int l_error=0;

      /*Verify that this cat is used on our set, otherwise move on to the next cat*/
      for(j=0;j<l_numCatsOnThisSet;j++)
	{
	  if( SAF_EQUIV(&l_cat,&(l_catsOnThisSet[j])) )
	    {
	      l_found=1;
	      break;
	    }
	}
      if(!l_found) continue;

      /*Do we have a topo relation using this cat on our set? i.e. are there elements on the set?
	Or, will we be drawing something that we can paint a variable onto?
	If not, then move on to the next cat*/
      l_found=0;
      saf_describe_collection(SAF_ALL,&l_set,&l_cat,&l_cellType,&l_numEntriesInColl,NULL,NULL,NULL);

      if(l_cellType==SAF_CELLTYPE_POINT)
	{
	  /*There are 3 possibilities 
	    1: The nodes are stored on the set itself.
	    2: The nodes are stored on another set, that is connected to this set by a nodes subset.
	    The topo relations point to the set itself.(rare)
	    3: The nodes are stored on another set. The topo relations point to that set. There may
	    or may not be a nodes collection on the set itself, but we cant be sure how it relates
	    to the drawn nodes(?).

	    With 1 & 2, a nodes field written on the set itself will be handled correctly. With 3, a
	    nodes field written on the set itself will only be correct if the nodes collection is the
	    same as the other set's. Note that case 3 is very ugly, and a saf file shouldnt be written
	    that way. Note that for subsets and supersets, the above is also valid.
	  */

	  l_found=1;
	}
      else
	{
	  int l_numElems=0;
	  int l_numRels = get_num_topo_rels_in_set( l_set, l_cellType, &l_numElems, NULL, 0  );
	  if(!l_numRels || !l_numElems)
	    {
	      /*There were no normal elems, try quadratic elems instead*/
	      l_numRels = get_num_topo_rels_in_set( l_set, l_cellType, &l_numElems, NULL, 1  );
	    }
	  if(l_numRels && l_numElems) l_found=1;
	}
      if(!l_found) continue;

      /*We know now that all fields (on this cat) on supersets are potentially variables on this set*/

      /*Get all fields on the set itself*/
      if( a_fields ) l_fieldsPtr = a_fields + l_numVars;
      l_numVars += get_num_var_fields_on_set_itself(l_set,l_cat,a_whichTimestep, a_maxNum-l_numVars, l_fieldsPtr, &l_error );

      if(l_error) 
	{
	  printinfo("error get_all_unstr_fields_for_set_and_cat found too many vars in set itself\n");
	  break;
	}

      /*Get all fields on supersets of this set. */
      get_list_of_supersets( l_set, l_cat, &l_numSupersets, &l_supersetIndices );


      for(j=0;j<l_numSupersets;j++)
	{		
	  l_numVarsBefore=l_numVars;
	  if( a_fields ) l_fieldsPtr = a_fields + l_numVars;
	  l_numVars += get_num_var_fields_on_set_itself(g_unstrGlobals->m_allSets[l_supersetIndices[j]],
							l_cat,a_whichTimestep, a_maxNum-l_numVars, l_fieldsPtr, &l_error );
	  if(l_error) break;
	}

      if(l_error) 
	{
	  printinfo("error get_all_unstr_fields_for_set_and_cat found too many vars in supersets\n");
	  if(l_supersetIndices) free(l_supersetIndices);
	  break;
	}

	

      /*Get all fields on subsets of this set*/
      for(j=1;j<g_unstrGlobals->m_subsetTransforms[i][a_setIndex].m_length;j++)
	{		
	  l_numVarsBefore=l_numVars;
	  if( a_fields ) l_fieldsPtr = a_fields + l_numVars;
	  l_numVars += get_num_var_fields_on_set_itself(g_unstrGlobals->m_subsetTransforms[i][a_setIndex].m_sets[j],
							l_cat,a_whichTimestep, a_maxNum-l_numVars, l_fieldsPtr, &l_error );
	  if(l_error) break;
	}

      if(l_error) 
	{
	  printinfo("error get_all_unstr_fields_for_set_and_cat found too many vars in subsets\n");
	  if(l_supersetIndices) free(l_supersetIndices);
	  break;
	}


      if(l_supersetIndices) free(l_supersetIndices);
    }
    
  free(l_catsOnThisSet);

  return(l_numVars);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_part_arg_valid( int a_whichPart )
{
  if( a_whichPart<0 || a_whichPart>=g_unstrGlobals->m_numUnstrParts )
    {
      printinfo("\nis_part_arg_valid error: a_whichPart=%d out of range\n",a_whichPart);
      return(0);
    }
  return(1);
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
is_time_arg_valid( int a_whichTimestep )
{
  if( a_whichTimestep<0 || a_whichTimestep>=get_num_timesteps() )
    {
      printinfo("is_time_arg_valid error: a_whichTimestep=%d out of range\n",a_whichTimestep);
      return(0);
    }
  return(1);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
void 
UNSTR_CLASS_NAME_IF_CPP
name_all_parts(void)
{
  int i;
  if( g_unstrGlobals->m_partNames || !g_unstrGlobals->m_numUnstrParts ) return;

  g_unstrGlobals->m_partNames=(char **)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(char *));

  /*First, give each part the name of its set*/
  for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
    {
      int l_setIndex = g_unstrGlobals->m_setIndexPerPart[i];
      if( l_setIndex < 0 || l_setIndex >= g_unstrGlobals->m_numAllSets )
	{ /*This should never happen*/
	  char l_str[256];
	  sprintf(l_str,"(unstrpart %d)",i);
	  g_unstrGlobals->m_partNames[i] = ens_strdup(l_str);
	}
      else
	{
	  char *l_name = 0;
	  saf_describe_set(SAF_ALL, &(g_unstrGlobals->m_allSets[l_setIndex]), &l_name, NULL, NULL, NULL, NULL, NULL, NULL);
	  g_unstrGlobals->m_partNames[i] = ens_strdup(l_name);
	  free(l_name);
	}
    }

  /*Second, if any names are the same, combine them with the names of their node locations.
    This was done 17feb2004 for FUEGO data.*/
  {
    int *l_rename = (int *)malloc(g_unstrGlobals->m_numUnstrParts*sizeof(int *));
    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++) l_rename[i]=0;

    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
      {
	int j;
	int l_renamingAllMatchesToThisPart=0;
	if(l_rename[i]) continue;
	for(j=i+1;j<g_unstrGlobals->m_numUnstrParts;j++)
	  {	    
	    if(!strcmp(g_unstrGlobals->m_partNames[i],g_unstrGlobals->m_partNames[j]) )
	      {
		/*If their node sets are different, then the renaming method might work*/
		if(l_renamingAllMatchesToThisPart ||
		   g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][0] !=
		   g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[j][0])
		  {
		    l_renamingAllMatchesToThisPart=1;
		    l_rename[i]=1;
		    l_rename[j]=1;		
		  }
	      }
	  }
      }
    
    for(i=0;i<g_unstrGlobals->m_numUnstrParts;i++)
      {
	if(l_rename[i] && g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep )
	  {
	    int l_setIndex = g_unstrGlobals->m_setIndexWhereNodesAreStoredPerPartPerTimestep[i][0];
	    if( l_setIndex < 0 || l_setIndex >= g_unstrGlobals->m_numAllSets )
	      { 
		/*do nothing: there are no nodes for time 0, and this shouldnt be so complicated*/
	      }
	    else
	      {
		char *l_name=NULL, *l_newName=NULL;
		char *l_oldName = g_unstrGlobals->m_partNames[i];
		saf_describe_set(SAF_ALL, &(g_unstrGlobals->m_allSets[l_setIndex]), &l_name, NULL, NULL, NULL, NULL, NULL, NULL);		
		l_newName = (char *)malloc( (strlen(l_name)+strlen(l_oldName)+2)*sizeof(char));
		sprintf(l_newName,"%s_%s",l_oldName,l_name);
		g_unstrGlobals->m_partNames[i] = l_newName;
		free(l_name);
		free(l_oldName);
	      }
	  }
      }
 
    free(l_rename);
  }

}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Unstructured Mesh Reader
 * Purpose:     
 * Description: 
 *--------------------------------------------------------------------------------------------------- */
int 
UNSTR_CLASS_NAME_IF_CPP
get_num_nodes_in_celltype( SAF_CellType a_cellType, int a_isQuadratic )
{
  int l_numPtsInType=-1;
  if( !a_isQuadratic )
  {
	  if( a_cellType == SAF_CELLTYPE_LINE ) l_numPtsInType=2;
	  else if( a_cellType == SAF_CELLTYPE_TRI ) l_numPtsInType=3;
	  else if( a_cellType == SAF_CELLTYPE_QUAD ) l_numPtsInType=4;
	  else if( a_cellType == SAF_CELLTYPE_TET ) l_numPtsInType=4;
	  else if( a_cellType == SAF_CELLTYPE_HEX ) l_numPtsInType=8;
	  else if( a_cellType == SAF_CELLTYPE_PRISM ) l_numPtsInType=6;
	  else if( a_cellType == SAF_CELLTYPE_PYRAMID ) l_numPtsInType=5;
	  else if( a_cellType == SAF_CELLTYPE_POINT ) l_numPtsInType=1;
	  else if( a_cellType == SAF_CELLTYPE_1BALL ) l_numPtsInType=NUM_NODES_IN_1BALL;
	  else if( a_cellType == SAF_CELLTYPE_2BALL ) l_numPtsInType=NUM_NODES_IN_2BALL;
	  else if( a_cellType == SAF_CELLTYPE_3BALL ) l_numPtsInType=NUM_NODES_IN_3BALL;
	  /*else if( a_cellType == SAF_CELLTYPE_0BALL ) l_numPtsInType=NUM_NODES_IN_0BALL; not in sslib?*/
  }
  else
  {
	  if( a_cellType == SAF_CELLTYPE_LINE ) l_numPtsInType=3;
	  else if( a_cellType == SAF_CELLTYPE_TRI ) l_numPtsInType=6;
	  else if( a_cellType == SAF_CELLTYPE_QUAD ) l_numPtsInType=8;
	  else if( a_cellType == SAF_CELLTYPE_TET ) l_numPtsInType=10;
	  else if( a_cellType == SAF_CELLTYPE_HEX ) l_numPtsInType=20;
	  else if( a_cellType == SAF_CELLTYPE_PRISM ) l_numPtsInType=15;
	  else if( a_cellType == SAF_CELLTYPE_PYRAMID ) l_numPtsInType=13;
  }
  return(l_numPtsInType);
}


