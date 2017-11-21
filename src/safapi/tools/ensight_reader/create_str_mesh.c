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
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Description:
 *
 * This program creates a sample SAF structured mesh database using a variety of different
 * configurations of structured mesh data. The various functions used by the program can
 * also be used individually to help create a SAF structured mesh writer for an application
 * code.
 *
 * It works in serial and parallel, writing two 3D structured blocks (one separable, one
 * curvilinear) per processor. It writes variables on nodes, x-pointing edges, y-pointing edges,
 * z-pointing edges, x-normal faces, y-normal faces, z-normal faces, and hex elements. It also
 * writes variables on hyperslab subsets of each of those types of variable (e.g. a node variable
 * defined only on a subset of nodes on a block, with the subset defined by the 3D hyperslab:
 * x start,count,step=0,5,2; y=1,4,1; z= 2,6,4).
 *
 * The functions here use a handful of global variables defined in this file (global vars are 
 * prepended by g_, local vars are prepended by l_, and argument vars are prepended by a_). The
 * primary purpose of the global arrays is to keep track of all blocks and variables created so that
 * they can be associated with timesteps at the end of the process.
 *
 *--------------------------------------------------------------------------------------------------- */


#include <math.h>
#include <saf.h>

/*Note: must set this '#if' according to whether str_mesh_reader.c is compiled too.*/
#if 1
  /*Compiling with str_mesh_reader.c*/
  #include <str_mesh_reader.h>
#else
  /*NOT compiling with str_mesh_reader.c*/
  #include <stdarg.h>
  #define MY_PRECISION float
  #define MY_SAF_PRECISION H5T_NATIVE_FLOAT /*SAF_FLOAT*/
  typedef struct {
    int x_start; int y_start; int z_start;
    int x_count; int y_count; int z_count; 
    int x_skip; int y_skip; int z_skip;
  } struct_hyperslab;
  void printinfo_set_filename(char *name) {}
  void printinfo( const char *fmt, ... ) 
  {
    va_list args;
    va_start(args,fmt);
    vfprintf(stdout, fmt, args);/*same as printf*/
  }
#endif


#include <str_mesh_test.h>
 
 

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Global Variables
 *
 * Description: 
 *
 * These are global variables.
 *
 *--------------------------------------------------------------------------------------------------- */

static int g_rank=0;
static int g_nprocs=1;

#define MY_SAF_INDEXSPEC(nx,ny,nz) _saf_indexspec(3, nx,ny,nz, 0,0,0, 2,1,0)/*fortran ordering, but with origins at 0*/
#define MY_SAF_1D_INDEXSPEC(nx) _saf_indexspec(1, nx, 0, 0)/*fortran ordering, but with origins at 0*/
MY_PRECISION g_fakeXCoordStart=0;

int g_dataUnstrNodeCloudNumNodes =150;

int g_diffTestOption=0;/*slight differences in the file, for testing safdiff*/

static SAF_Db *g_db;                /* Handle to the SAF database. */
static SAF_Db /*SAF_File*/ *g_safFile;        /* Handle to the saf file. */

SAF_Cat g_nodes, g_elems; /* Handles to the node and elements categories. */
SAF_Cat g_xEdges, g_yEdges, g_zEdges; 
SAF_Cat g_xFaces, g_yFaces, g_zFaces; 
int g_numSubsetsCreated=0;/*used to give new subsets a different name,
			    not sure if this is because of a bug or not*/

SAF_Set g_suiteset;/*a single set above all 'topsets'*/
SAF_Suite g_suite;/*a single suite*/

/*
** g_fields,g_fieldTemplates,g_timestepsPerAddedField contain the template,field,timestep
** triplets. After all fields are written, then these three arrays are used
** to put each field in the appropriate timestep-suite. 
*/
  int g_maxNumFields=0;
  int g_currentNumFields=0;
  int g_numTimes = -1;
  SAF_Field *g_fields=0;
  SAF_FieldTmpl *g_fieldTemplates=0;
  int *g_timestepsPerAddedField=0;
  int g_numSuitesWritten=0;



  /*need to keep track of blocks and unstr nodessets added for new states&suites*/
  int g_numBlocksAdded=0;
  int g_blocksAndUnstrNodesetsAddedListSize=0;
  SAF_Set *g_blocksAndUnstrNodesetsAdded=0;


/*Now that field templates are not attached to a set, we can use the
 same templates for all coordinate fields*/
SAF_FieldTmpl g_coordsFieldTemplate;
SAF_FieldTmpl g_coordsIndivFieldTemplates[3];

int g_useRandom=1;
MY_PRECISION g_dataMultiplier = 1;

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Prototypes
 *
 * Description: 
 *
 * These are prototypes.
 *
 *--------------------------------------------------------------------------------------------------- */
MY_PRECISION my_rand(void);
void add_set_to_suites_list( SAF_Set a_set );
void change_data_multiplier(void);
void create_fake_data_var( int a_x, int a_y, int a_z, MY_PRECISION **a_data, int a_timestep, int a_flag );
void create_fake_data_node_cloud( int a_count, MY_PRECISION **a_data, int a_timestep );
void create_fake_data_coords( int a_x, int a_y, int a_z, MY_PRECISION **a_xdata, MY_PRECISION **a_ydata, MY_PRECISION **a_zdata );
void destroy_fake_data( MY_PRECISION **a_data );
void declare_all_structured_categories(void);
void declare_all_structured_collections(SAF_Set a_set, int a_x, int a_y, int a_z);
void add_field_to_timestep_data( SAF_Field *a_field, SAF_FieldTmpl *a_template, int a_timestep );
void sort_timestep_data( int a_num, SAF_Field *a_fields, 
			 SAF_FieldTmpl *a_fieldTemplates, int *a_timestepsPerAddedField );
SAF_Set *create_node_subset(SAF_Set *a_blockNodeSet,struct_hyperslab a_doList );
SAF_Set *create_elem_subset(SAF_Set *a_blockSet,struct_hyperslab a_doList );
SAF_Set *create_x_edge_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList);
SAF_Set *create_y_edge_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList);
SAF_Set *create_z_edge_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList);
SAF_Set *create_x_face_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList);
SAF_Set *create_y_face_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList);
SAF_Set *create_z_face_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList);
SAF_FieldTmpl *write_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
			      MY_PRECISION *a_varData, int a_timestep, SAF_Cat a_cat);
void write_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate,
					    const char *a_varName, const char *a_varUnits, 
					    MY_PRECISION *a_varData, int a_timestep, SAF_Cat a_cat );
SAF_FieldTmpl *write_node_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
				   MY_PRECISION *a_varData, int a_timestep );
void write_node_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName,
						 const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep );
SAF_FieldTmpl *write_x_edge_variable(SAF_Set *a_set,const char *a_varName, const char *a_varUnits,
				     MY_PRECISION *a_varData, int a_timestep );
SAF_FieldTmpl *write_y_edge_variable(SAF_Set *a_set,const char *a_varName, const char *a_varUnits,
				     MY_PRECISION *a_varData, int a_timestep );
SAF_FieldTmpl *write_z_edge_variable(SAF_Set *a_set,const char *a_varName, const char *a_varUnits,
				     MY_PRECISION *a_varData, int a_timestep );
void write_x_edge_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep );
void write_y_edge_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep );
void write_z_edge_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep );
SAF_FieldTmpl *write_x_face_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
				     MY_PRECISION *a_varData, int a_timestep );
SAF_FieldTmpl *write_y_face_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
				     MY_PRECISION *a_varData, int a_timestep );
SAF_FieldTmpl *write_z_face_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
				     MY_PRECISION *a_varData, int a_timestep );
void write_x_face_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep );
void write_y_face_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep );
void write_z_face_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep );
SAF_FieldTmpl *write_elem_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
				   MY_PRECISION *a_varData, int a_timestep );
void write_elem_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						 const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep );
SAF_Set *add_separable_block_as_curvilinear(  const char *a_blockName, const char *a_units, int a_xDim, int a_yDim, int a_zDim,
					      MY_PRECISION *a_xCoords, MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );
SAF_Set *add_curvilinear_block(  const char *a_blockName, const char *a_units, int a_xDim, int a_yDim, int a_zDim,
				 MY_PRECISION *a_xCoords, MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );
SAF_Set *add_unstructured_nodeset(  const char *a_blockName, const char *a_units,int a_timestep,
				    int a_numNodes, MY_PRECISION *a_xyzCoords );
int add_coords_to_unstructured_nodeset( SAF_Set a_set, const char *a_units, int a_timestep,
					int a_numNodes, MY_PRECISION *a_xyzCoords );
int is_field_on_or_below_set( SAF_Field a_field, SAF_Set a_set );
int which_of_these_topsets_is_field_from( SAF_Field a_field, int a_numTopsets, SAF_Set *a_topsets );
SAF_Set create_TOP_SET(void);
void write_timestep_data( MY_PRECISION *a_timeValues );
SAF_Set *add_separable_block( const char *a_blockName, const char *a_units, int a_xDim, int a_yDim, int a_zDim,
			      MY_PRECISION *a_xCoords, MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Return a Random Number if Enabled
 *
 * Description: 
 *
 * Allows the option to use random numbers or not: with random, the visualization
 * is more interesting, without random, can compare files created on
 * different platforms.
 *
 * Setting g_useRandom to zero also stops the use of other things
 * that might cause different results on different platforms: if you 
 * want to compare structured mesh files from different platforms, then you
 * should use "-n" to set g_useRandom to zero.
 *
 *---------------------------------------------------------------------------------------------------*/
MY_PRECISION my_rand(void)
{
  if( g_useRandom )
    {
      int l_rand = rand();

      double l_num = (double)l_rand / (double)RAND_MAX;

      return( (MY_PRECISION)l_num );
    }
  else return( .5 );
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Add a Set to the Global Suite List
 *
 * Description: 
 * This function is called when a structured block is created.
 *
 *--------------------------------------------------------------------------------------------------- */
void add_set_to_suites_list( SAF_Set a_set )
{
 if(!g_blocksAndUnstrNodesetsAddedListSize)
   { /*nothing allocated yet*/
     g_blocksAndUnstrNodesetsAddedListSize = 50;
     g_blocksAndUnstrNodesetsAdded = (SAF_Set *)malloc(g_blocksAndUnstrNodesetsAddedListSize*sizeof(SAF_Set));
     if(!g_blocksAndUnstrNodesetsAdded) { printinfo("malloc failed for g_blocksAndUnstrNodesetsAdded\n");  exit(-1); }
   }
 else if( g_numBlocksAdded >= g_blocksAndUnstrNodesetsAddedListSize )
   { /*need to resize the array*/
     /*XXX do this eventually, for now just exit*/
     printinfo("\n\nerror...NEED TO RESIZE g_blocksAndUnstrNodesetsAdded....havent written it yet....easy fix\n");
     printinfo(".........g_numBlocksAdded=%d  >=  g_blocksAndUnstrNodesetsAddedListSize=%d\n",
	       g_numBlocksAdded, g_blocksAndUnstrNodesetsAddedListSize );
     exit(-1);
   }
 g_blocksAndUnstrNodesetsAdded[g_numBlocksAdded] = a_set;
 g_numBlocksAdded++;
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Change the Dummy Data Multiplier
 *
 * Description: 
 *
 * Changes the factor by which dummy variables are scaled. Used for making steps through
 * time more visible in the dummy data.
 *
 *--------------------------------------------------------------------------------------------------- */
void change_data_multiplier()
{
  const MY_PRECISION l_max = 1e20;

  if( g_dataMultiplier>1.0 && g_dataMultiplier<l_max ) g_dataMultiplier = (1/g_dataMultiplier)/10;
  else if( g_dataMultiplier<1.0 ) g_dataMultiplier = (1/g_dataMultiplier)*10;
  else if( g_dataMultiplier>=l_max ) g_dataMultiplier=10;

  printinfo("g_dataMultiplier=%e\n",g_dataMultiplier);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create Dummy Variable Data
 *
 * Description: 
 *
 * Creates a slab of data with a periodic variation for visualization purposes.
 *
 *--------------------------------------------------------------------------------------------------- */
void create_fake_data_var( int a_x, int a_y, int a_z, MY_PRECISION **a_data, int a_timestep, int a_flag )
{
  int i,j,k,l_max;
  int l_count = a_x*a_y*a_z;
  MY_PRECISION *l_ptr;

  if( g_dataMultiplier != 1.0 ) change_data_multiplier();
 
  if( !a_data[0] )
    {
      if( l_count <=0 )
	{
	  a_data[0]=0;
	  return;
	}
      a_data[0] = (MY_PRECISION *)malloc( l_count*sizeof(MY_PRECISION) );
      if(!a_data[0]) 
	{
	  printinfo("error: create_fake_data_var failed to allocate memory\n");
	  return;
	}
    }

  if(!a_flag)
    l_max = a_y-1;
  else
    l_max = (a_z-1)+(a_y-1);

  if(l_max<1) l_max=1;

  /*Use the same sequence of random numbers, so that each database created
    (with the same options) will be the same. The random numbers are helpful
    for testing the visualization tool.*/
  srand(123);

  l_ptr=a_data[0];
  for(k=0;k<a_z;k++)
    for(j=0;j<a_y;j++)
      for(i=0;i<a_x;i++)
	{
	  MY_PRECISION l_rand = my_rand();
	  double l_sinA = g_useRandom ? sin((double)k*2.0*3.14/30.0) : 0;
	  double l_sinB = g_useRandom ? sin((double)i*2.0*3.14/20.0) : 0;
	  double l_sinC = g_useRandom ? sin((double)i*2.0*3.14/10.0) : 0;

	  if(!a_flag)
	    l_ptr[0]= ( (MY_PRECISION)(j%l_max) + l_sinA - l_sinC + (double)a_timestep*.75+ .0625*l_rand)*g_dataMultiplier;
	  else
	    l_ptr[0]= ( (MY_PRECISION)((k+j)%l_max) + l_sinA - l_sinB + (double)a_timestep*2.5 + .0625*l_rand)*g_dataMultiplier;
	  l_ptr++;
	}
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create Dummy Node Cloud Coordinates
 *
 * Description: 
 *
 * Creates a set of a_count 3D coordinates that are grouped together and move with time.
 *
 *--------------------------------------------------------------------------------------------------- */
void create_fake_data_node_cloud( int a_count, MY_PRECISION **a_data, int a_timestep )
{
  int i;

  if( g_dataMultiplier != 1.0 ) change_data_multiplier();
 
  if( !a_data[0] )
    {
      if( a_count <=0 )
	{
	  a_data[0]=0;
	  return;
	}
      a_data[0] = (MY_PRECISION *)malloc( a_count*sizeof(MY_PRECISION) );
      if(!a_data[0]) 
	{
	  printinfo("error: create_fake_data_node_cloud failed to allocate memory\n");
	  return;
	}
    }

  /*Use the same sequence of random numbers, so that each database created
    (with the same options) will be the same. The random numbers are helpful
    for testing the visualization tool.*/
  srand(123);

  for(i=0;i<a_count;i+=3) 
    {
      MY_PRECISION l_randVal[3];
      int j;

      if(i+2>=a_count) break;/*dont let this happen!*/

      for( j=0; j<3; j++ )
	{
	  MY_PRECISION l_rand1=my_rand();
	  MY_PRECISION l_rand2=my_rand();
	  l_randVal[j] = (l_rand1 - .5) *(l_rand2 - .5);
	}

      a_data[0][i+0]= ( 12*l_randVal[0] + .25*(i%16) + a_timestep*2 )*g_dataMultiplier;
      a_data[0][i+1]= ( 10*l_randVal[1] + .25*(i/16) + 4 )*g_dataMultiplier;
      a_data[0][i+2]= (-40*l_randVal[2] + 30 )*g_dataMultiplier;
    }
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create Dummy Structured Block Coordinates
 *
 * Description: 
 *
 * Creates dummy separable coordinate data for a 3d structured mesh block. 
 *
 *--------------------------------------------------------------------------------------------------- */
void create_fake_data_coords( int a_x, int a_y, int a_z, MY_PRECISION **a_xdata, MY_PRECISION **a_ydata, MY_PRECISION **a_zdata )
{
  int i;
  if( !a_x || !a_y || !a_z ) return;

  a_xdata[0] = (MY_PRECISION *)malloc( a_x*sizeof(MY_PRECISION) );
  a_ydata[0] = (MY_PRECISION *)malloc( a_y*sizeof(MY_PRECISION) );
  a_zdata[0] = (MY_PRECISION *)malloc( a_z*sizeof(MY_PRECISION) );
  if(!a_xdata[0] || !a_ydata[0] || !a_zdata[0]) 
    {
      printinfo("error: create_fake_data_coords failed to allocate memory\n");
      return;
    }
  /*make the step length continually increasing*/
  a_xdata[0][0]=g_fakeXCoordStart;
  a_ydata[0][0]=g_rank*40.0;
  a_zdata[0][0]=0;

  for(i=1;i<a_x;i++) a_xdata[0][i]=a_xdata[0][i-1] + 1.0 + ((MY_PRECISION)(i%5))*.125;
  for(i=1;i<a_y;i++) a_ydata[0][i]=a_ydata[0][i-1] + 1.25 + ((MY_PRECISION)(i%8))*.125;
  for(i=1;i<a_z;i++) a_zdata[0][i]=a_zdata[0][i-1] + 1.5 + ((MY_PRECISION)(i%4))*.125;

  g_fakeXCoordStart = a_xdata[0][a_x-1];

    {
      printinfo("X coords[%d]: ",a_x);
      for(i=0;i<a_x;i++) 
	{
	  if( i>10 )
	    {
	      printinfo("...");
	      break;
	    }
	  printinfo("%f ",a_xdata[0][i]);
	}
      printinfo("\n");
      printinfo("Y coords[%d]: ",a_y);
      for(i=0;i<a_y;i++) 
	{
	  if( i>10 )
	    {
	      printinfo("...");
	      break;
	    }
	  printinfo("%f ",a_ydata[0][i]);
	}
      printinfo("\n");
      printinfo("Z coords[%d]: ",a_z);
      for(i=0;i<a_z;i++) 
	{
	  if( i>10 )
	    {
	      printinfo("...");
	      break;
	    }
	  printinfo("%f ",a_zdata[0][i]);
	}
      printinfo("\n");
    }
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Free Dummy Data
 *
 * Description: 
 *
 * Simply frees and zeroes a pointer. 
 *
 *--------------------------------------------------------------------------------------------------- */
void destroy_fake_data( MY_PRECISION **a_data )
{
  if( a_data[0] ) free( a_data[0] );
  a_data[0] = 0;
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Declare the Required Structured Mesh Categories for a Database
 *
 * Description: 
 *
 * This function declares the categories that are required to define a 3D
 * SAF structured mesh set. There are eight categories: nodes, x edges, y edges,
 * z edges, x faces, y faces, z faces, and hex elements. 
 *
 * The categories are all global variables in the file, so this only needs to be done once.
 *--------------------------------------------------------------------------------------------------- */
void declare_all_structured_categories(void)
{
  saf_declare_category(SAF_ALL,g_db, "nodes",  SAF_TOPOLOGY, 0, &g_nodes);
  saf_declare_category(SAF_ALL,g_db, "xEdges",  SAF_TOPOLOGY, 1, &g_xEdges);
  saf_declare_category(SAF_ALL,g_db, "yEdges",  SAF_TOPOLOGY, 1, &g_yEdges);
  saf_declare_category(SAF_ALL,g_db, "zEdges",  SAF_TOPOLOGY, 1, &g_zEdges);
  saf_declare_category(SAF_ALL,g_db, "xFaces",  SAF_TOPOLOGY, 2, &g_xFaces);
  saf_declare_category(SAF_ALL,g_db, "yFaces",  SAF_TOPOLOGY, 2, &g_yFaces);
  saf_declare_category(SAF_ALL,g_db, "zFaces",  SAF_TOPOLOGY, 2, &g_zFaces);
  saf_declare_category(SAF_ALL,g_db, "elems",  SAF_TOPOLOGY, 3, &g_elems);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Declare the Collections on a Structured Mesh Block
 *
 * Description: 
 *
 * This function declares the collections and the topology relation that define
 * a SAF structured mesh set. There are eight collections: nodes, x edges, y edges,
 * z edges, x faces, y faces, z faces, and hex elements. The arguments a_x, a_y, and 
 * a_z specify the size (i.e. number of nodes) of the structured block in each direction.  
 * The sizes of the other seven collections are related to the size of the node collection.
 * For example, the size of the x edges collection is (a_x-1) * a_y * a_z, and the
 * size of the hex elem collection is (a_x-1) * (a_y-1) * (a_z-1). 
 *
 * The existence of these eight collections is one of the two factors that a reader,
 * with no prior knowledge of a set, will use to determine that the set represents
 * a structured mesh element block.
 *
 * The second factor is the SAF_STRUCTURED topology relation between the hex collection
 * and the node collection. This function creates that topology relation on the given set.
 *
 *--------------------------------------------------------------------------------------------------- */
void declare_all_structured_collections(SAF_Set a_set, int a_x, int a_y, int a_z)
{
  int l_numNodes,l_numXEdges, l_numYEdges, l_numZEdges;
  int l_numXFaces, l_numYFaces, l_numZFaces, l_numElems;
  SAF_Rel l_rel;

  /*note: the repeated use of  ((a_x-1>=0)?(a_x-1):0)  instead of  (a_x-1)
    is to make sure that no negative numbers are used in case a trivial
    argument (e.g. a_x==0 or a_y==0 or a_z==0) is passed
  */
  l_numNodes = a_x*a_y*a_z;
  l_numXEdges = ((a_x-1>=0)?(a_x-1):0)  *a_y*a_z;
  l_numYEdges = a_x* ((a_y-1>=0)?(a_y-1):0) *a_z;
  l_numZEdges = a_x*a_y*  ((a_z-1>=0)?(a_z-1):0);
  l_numXFaces = a_x* ((a_y-1>=0)?(a_y-1):0) * ((a_z-1>=0)?(a_z-1):0);
  l_numYFaces = ((a_x-1>=0)?(a_x-1):0) *a_y* ((a_z-1>=0)?(a_z-1):0);
  l_numZFaces = ((a_x-1>=0)?(a_x-1):0) * ((a_y-1>=0)?(a_y-1):0) *a_z;
  l_numElems = ((a_x-1>=0)?(a_x-1):0)*((a_y-1>=0)?(a_y-1):0)*((a_z-1>=0)?(a_z-1):0);


  saf_declare_collection(SAF_EACH, &a_set, &g_nodes,  SAF_CELLTYPE_POINT, l_numNodes,
			 MY_SAF_INDEXSPEC(a_x,a_y,a_z), SAF_DECOMP_FALSE);

  saf_declare_collection(SAF_EACH, &a_set, &g_xEdges,  SAF_CELLTYPE_LINE, l_numXEdges,
			 MY_SAF_INDEXSPEC( ((a_x-1>=0) ? (a_x-1):0), a_y,a_z), 
			 SAF_DECOMP_FALSE);

  saf_declare_collection(SAF_EACH, &a_set, &g_yEdges,  SAF_CELLTYPE_LINE, l_numYEdges,
			 MY_SAF_INDEXSPEC(a_x, ((a_y-1>=0) ? (a_y-1):0), a_z), 
			 SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, &a_set, &g_zEdges,  SAF_CELLTYPE_LINE, l_numZEdges,
			 MY_SAF_INDEXSPEC(a_x,a_y,  ((a_z-1>=0) ? (a_z-1):0) ), 
			 SAF_DECOMP_FALSE);

  saf_declare_collection(SAF_EACH, &a_set, &g_xFaces,  SAF_CELLTYPE_QUAD, l_numXFaces,
			 MY_SAF_INDEXSPEC(a_x,((a_y-1>=0) ? (a_y-1):0), ((a_z-1>=0) ? (a_z-1):0)), 
			 SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, &a_set, &g_yFaces,  SAF_CELLTYPE_QUAD, l_numYFaces,
			 MY_SAF_INDEXSPEC(((a_x-1>=0) ? (a_x-1):0),a_y,((a_z-1>=0) ? (a_z-1):0)), 
			 SAF_DECOMP_FALSE);
  saf_declare_collection(SAF_EACH, &a_set, &g_zFaces,  SAF_CELLTYPE_QUAD, l_numZFaces,
			 MY_SAF_INDEXSPEC(((a_x-1>=0) ? (a_x-1):0),((a_y-1>=0) ? (a_y-1):0),a_z), 
			 SAF_DECOMP_FALSE);

  saf_declare_collection(SAF_EACH, &a_set, &g_elems,  SAF_CELLTYPE_HEX, l_numElems,
			 MY_SAF_INDEXSPEC( ((a_x-1>=0) ? (a_x-1):0),
					   ((a_y-1>=0) ? (a_y-1):0),
					   ((a_z-1>=0) ? (a_z-1):0) ), 
			 SAF_DECOMP_TRUE);



  /*Note: this relation really doesnt do anything. It is only part of the signal to 
    the reader that this is indeed a structured mesh. The other part is the
    8 collections above. Need to revisit SAF_STRUCTURED in lib/rel.c. */
  saf_declare_topo_relation(SAF_EACH,g_db,&a_set,&g_elems,&a_set,&g_nodes,SAF_SELF(g_db),
			    &a_set,SAF_STRUCTURED,SAF_INT,NULL,SAF_INT,NULL,&l_rel );

}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Separable Block
 *
 * Description: 
 * Given 3 1D arrays of length X, Y and Z, this function creates a separable structured
 * block (a SAF_Set containing the eight required collections and one topology relation) of size XYZ.
 *
 * Return:      On success, returns the created set.
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *add_separable_block(  const char *a_blockName, /*the name to give to the block*/
		     const char *a_units, /*the distance units for the coordinates*/
		     int a_xDim, int a_yDim, int a_zDim, 
		     MY_PRECISION *a_xCoords, MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords
		     )
{
  SAF_Field l_compositeField, l_componentField[3]; 
  SAF_Unit *l_units;
  SAF_Set *l_blockNodeSet= (SAF_Set *)malloc( sizeof(SAF_Set) );

  /*note: if we wanted to make these coordinates time dependent, we would have to
    declare l_coordsField and l_coordsFieldTemplate as pointers, and malloc them
    here, so that they still exist when we create the states & suites*/

  if(!l_blockNodeSet) { printinfo("add_separable_block: malloc failed for l_blockNodeSet\n");  exit(-1); }

  printinfo("\nproc %d trying to add_separable_block named %s, of size %dx%dx%d\n",g_rank,a_blockName,a_xDim,a_yDim,a_zDim);

  saf_declare_set(SAF_EACH, g_db, a_blockName, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_blockNodeSet );

  declare_all_structured_collections(*l_blockNodeSet,a_xDim,a_yDim,a_zDim);

   /* Get a handle to the units for this field. */
  l_units = saf_find_one_unit(g_db, a_units,NULL);

  /* Declare the fields. */
  saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "X_separable_coord_fld",l_blockNodeSet,l_units,SAF_SELF(g_db),   
		    /*SAF_DECOMP(g_nodes), */
		    SAF_SELF(g_db), a_xDim, SAF_SELF(g_db), SAF_SPACE_PWCONST,
		    MY_SAF_PRECISION,NULL,SAF_INTERLEAVE_NONE,SAF_IDENTITY,NULL, l_componentField);

  saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "Y_separable_coord_fld",l_blockNodeSet,l_units,SAF_SELF(g_db),   
		    /*SAF_DECOMP(g_nodes),*/
		    SAF_SELF(g_db), a_yDim, SAF_SELF(g_db), SAF_SPACE_PWCONST,
		    MY_SAF_PRECISION,NULL,SAF_INTERLEAVE_NONE,SAF_IDENTITY,NULL, l_componentField+1);
  saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "Z_separable_coord_fld",l_blockNodeSet,l_units,SAF_SELF(g_db),   
		    /*SAF_DECOMP(g_nodes),*/
		    SAF_SELF(g_db), a_zDim, SAF_SELF(g_db), SAF_SPACE_PWCONST,
		    MY_SAF_PRECISION,NULL,SAF_INTERLEAVE_NONE,SAF_IDENTITY,NULL, l_componentField+2);

  saf_declare_field(SAF_EACH, g_db,&g_coordsFieldTemplate, "separable_coord_fld", l_blockNodeSet, l_units,
		    SAF_SELF(g_db),  SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
		    l_componentField, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL,/*_COMPONENT? _NONE?*/
		    &l_compositeField);

  /*write the fields*/
  saf_write_field(SAF_EACH, l_componentField, SAF_WHOLE_FIELD, 1,
		  0, (void**)&a_xCoords, g_safFile);
  saf_write_field(SAF_EACH, &(l_componentField[1]), SAF_WHOLE_FIELD, 1,
		  0, (void**)&a_yCoords, g_safFile);
  saf_write_field(SAF_EACH, &(l_componentField[2]), SAF_WHOLE_FIELD, 1,
		  0, (void**)&a_zCoords, g_safFile);


  saf_declare_coords(SAF_EACH, &l_compositeField);
  saf_declare_coords(SAF_EACH, &(l_componentField[0]) );
  saf_declare_coords(SAF_EACH, &(l_componentField[1]) );
  saf_declare_coords(SAF_EACH, &(l_componentField[2]) );
   

  saf_declare_default_coords(SAF_EACH,l_blockNodeSet,&l_compositeField);

  add_set_to_suites_list( *l_blockNodeSet );

  printinfo("proc %d in add_separable_block finished %s\n",g_rank,a_blockName);

  return(l_blockNodeSet);
} /*end of add_separable_block*/


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Curvilinear Block
 *
 * Description: 
 * Given 3 1D arrays of length XYZ, this function creates a curvilinear structured
 * block (a SAF_Set containing the eight required collections and one topology relation) of size XYZ.
 *
 * Return:      On success, returns the created set.
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *add_curvilinear_block(  const char *a_blockName, /*the name to give to the block*/
		     const char *a_units, /*the distance units for the coordinates*/
		     int a_xDim, int a_yDim, int a_zDim,
		     MY_PRECISION *a_xCoords, MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords
		     )
{
   SAF_Field l_coordsField,           /* Handle to the coordinate field. */
             l_coordsComponentField[3];  /* Handle to the 3 components of the coordinate field. */
   SAF_Unit *l_units;            /* Handle to the units for the coordinates. */
   SAF_Set *l_blockNodeSet= (SAF_Set *)malloc( sizeof(SAF_Set) );

   /*note: if we wanted to make these coordinates time dependent, we would have to
     declare l_coordsField and l_coordsFieldTemplate as pointers, and malloc them
     here, so that they still exist when we create the states & suites*/

   if(!l_blockNodeSet) { printinfo("malloc failed for l_blockNodeSet\n");  exit(-1); }

   printinfo("\nproc %d trying to add_curvilinear_block named %s, of size %dx%dx%d\n",g_rank,a_blockName,a_xDim,a_yDim,a_zDim);

   saf_declare_set(SAF_EACH, g_db, a_blockName, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_blockNodeSet );


   declare_all_structured_collections(*l_blockNodeSet,a_xDim,a_yDim,a_zDim);

   /* Get a handle to the units for this field. */
   l_units = saf_find_one_unit(g_db, a_units,NULL);

   /* Declare the fields. */
   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "X_coord_field",    l_blockNodeSet,       l_units,
                     SAF_SELF(g_db),   SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField);
   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "Y_coord_field",    l_blockNodeSet,       l_units,
                     SAF_SELF(g_db),   SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField+1);
   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "Z_coord_field",    l_blockNodeSet,       l_units,
                     SAF_SELF(g_db),  SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField+2);

   saf_declare_field(SAF_EACH, g_db,&g_coordsFieldTemplate, "coord_field", l_blockNodeSet, l_units,
                     SAF_SELF(g_db),  SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
                     l_coordsComponentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                     &l_coordsField);



   /* Write the coordinate fields. */
   saf_write_field(SAF_EACH,&(l_coordsComponentField[0]),SAF_WHOLE_FIELD,1,0,(void**)&a_xCoords,g_safFile);
   saf_write_field(SAF_EACH,&(l_coordsComponentField[1]),SAF_WHOLE_FIELD,1,0,(void**)&a_yCoords,g_safFile);
   saf_write_field(SAF_EACH,&(l_coordsComponentField[2]),SAF_WHOLE_FIELD,1,0,(void**)&a_zCoords,g_safFile);


   /* Specify that it is a coordinate field  */
   saf_declare_coords(SAF_EACH, &l_coordsField);

   /* Specify that they are all coordinate fields */
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[0]) );
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[1]) );
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[2]) );
   

   saf_declare_default_coords(SAF_EACH,l_blockNodeSet,&l_coordsField);

   add_set_to_suites_list( *l_blockNodeSet );

   return(l_blockNodeSet);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Assign a Timestep to a Field
 *
 * Description: 
 *
 * Adds a field and template to the global list that will later be used to
 * create the states and suites.
 *
 *--------------------------------------------------------------------------------------------------- */
void add_field_to_timestep_data( SAF_Field *a_field, SAF_FieldTmpl *a_template, int a_timestep )
{
 if(!g_maxNumFields)
   { /*nothing allocated yet*/
     g_maxNumFields = 500;
     g_fields = (SAF_Field *)malloc(g_maxNumFields*sizeof(SAF_Field));
     if(!g_fields) { printinfo("malloc failed for g_fields\n");  exit(-1); }
     g_fieldTemplates = (SAF_FieldTmpl *)malloc(g_maxNumFields*sizeof(SAF_FieldTmpl));
     if(!g_fieldTemplates) { printinfo("malloc failed for g_fieldTemplates\n");  exit(-1); }
     g_timestepsPerAddedField = (int *)malloc(g_maxNumFields*sizeof(int));
     if(!g_timestepsPerAddedField) { printinfo("malloc failed for g_timestepsPerAddedField\n");  exit(-1); }
   }
 else if( g_currentNumFields >= g_maxNumFields )
   { /*need to resize the arrays*/
     /*XXX do this eventually, for now just exit*/
     printinfo("\n\nerror...NEED TO RESIZE g_fields....havent written it yet....easy fix\n");
     exit(-1);
   }


 if( !my_saf_is_valid_field_handle(a_field) ) printf("warning add_field_to_timestep_data field is invalid\n");

 g_fields[g_currentNumFields] = *a_field;
 g_fieldTemplates[g_currentNumFields] = *a_template;
 g_timestepsPerAddedField[g_currentNumFields] = a_timestep;
 g_currentNumFields++;

 if( a_timestep >= g_numTimes ) g_numTimes = a_timestep+1;

}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Sort the Global Timestep Data
 *
 * Description: 
 *
 * Sort the global template, field, timestep triplets by timestep.
 *
 *--------------------------------------------------------------------------------------------------- */
void sort_timestep_data( int a_num, SAF_Field *a_fields, 
			 SAF_FieldTmpl *a_fieldTemplates, int *a_timestepsPerAddedField )
{
  int l_didSomething,i;
  if(!g_rank) printinfo("sort_timestep_data a_num=%d\n",a_num);
  do {
    l_didSomething=0;
    for(i=0;i<a_num-1;i++)
      {
	if( a_timestepsPerAddedField[i] > a_timestepsPerAddedField[i+1] )
	  {
	    int l_tmpTime;
	    SAF_Field l_tmpField;
	    SAF_FieldTmpl l_tmpTmpl;
	    l_didSomething=1;
	    l_tmpField = a_fields[i];
	    l_tmpTmpl = a_fieldTemplates[i];
	    l_tmpTime = a_timestepsPerAddedField[i];
	    a_fields[i] = a_fields[i+1];
	    a_fieldTemplates[i] = a_fieldTemplates[i+1];
	    a_timestepsPerAddedField[i] = a_timestepsPerAddedField[i+1];
	    a_fields[i+1] = l_tmpField;
	    a_fieldTemplates[i+1] = l_tmpTmpl;
	    a_timestepsPerAddedField[i+1] = l_tmpTime;
	  }
      }
  } while( l_didSomething );



#if 0
  if(g_numTimes)
    {
      int *l_numFieldsPerTime=0;

      l_numFieldsPerTime = (int *)malloc( g_numTimes*sizeof(int) );
      for(i=0;i<g_numTimes;i++) l_numFieldsPerTime[i]=0;
      for(i=0;i<a_num;i++) l_numFieldsPerTime[ a_timestepsPerAddedField[i] ]++;

      for(i=0;i<g_numTimes;i++) printinfo("l_numFieldsPerTime[%d]=%d\n",i,l_numFieldsPerTime[i]);
    }

  for(i=0;i<a_num;i++)
    {/*verbose*/
      char *l_setname=0,*l_fieldname=0;
      SAF_Set l_set;
      saf_describe_field(SAF_ALL, a_fields[i], NULL, &l_fieldname,&l_set, NULL, NULL,
			 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      saf_describe_set(SAF_ALL, &l_set, &l_setname, NULL, NULL, NULL, NULL, NULL, NULL );
      printinfo("time %d: \t %s \t\t\t %s\n",g_timestepsPerAddedField[i],l_fieldname,l_setname);
      if(l_fieldname) free(l_fieldname);
      if(l_setname) free(l_setname);
    }

  exit(-1);
#endif

}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Node Subset Defined by a Hyperslab
 *
 * Description: 
 * Creates a new structured mesh set from a subset of a given
 * structured mesh set's nodes. The subset is defined by a struct_hyperslab
 * 3D hyperslab structure.
 *
 * Return:      On success, returns a structured mesh set whose nodes are
 * related to the argument set by a hyperslab subset relation.
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *create_node_subset(SAF_Set *a_blockNodeSet, /*the original node set*/
			    struct_hyperslab a_doList /*definition of the hyperslab*/
			    )
{
  char l_str[256];
  SAF_Rel rel;
  SAF_Set *l_subBlockNodeSet = (SAF_Set*)malloc(1*sizeof(SAF_Set));
  if(!l_subBlockNodeSet) { printinfo("malloc failed for l_subBlockNodeSet\n");  exit(-1); }

  printinfo("create_node_subset hslab=(%d,%d,%d)(%d,%d,%d)(%d,%d,%d)",
	    a_doList.x_start,a_doList.x_count,a_doList.x_skip,
	    a_doList.y_start,a_doList.y_count,a_doList.y_skip,
	    a_doList.z_start,a_doList.z_count,a_doList.z_skip );


  sprintf(l_str,"node_var_set%d_pr%d",g_numSubsetsCreated++,g_rank);
  saf_declare_set(SAF_EACH, g_db, l_str, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_subBlockNodeSet );

  /*
  **     NOTE: because the node subsets have all 8 collections on them, just like
  **     the blocks themselves, one could create subset elems that span many of the
  **     elems in the top level block!
  */
  declare_all_structured_collections(*l_subBlockNodeSet,a_doList.x_count,a_doList.y_count,a_doList.z_count);

  saf_declare_subset_relation(SAF_EACH, g_db, a_blockNodeSet, l_subBlockNodeSet, SAF_COMMON(&g_nodes), 
			      SAF_HSLAB, SAF_INT, NULL, 0, NULL, &rel);
  saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, (int *)&a_doList, 0, NULL, g_safFile);

  return(l_subBlockNodeSet);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Hex Element Subset of a Structured Mesh Set
 *
 * Description: 
 * Creates a new set with a hex element collection that is a hyperslab subset
 * of the argument structured mesh set's hex elements. The subset is 
 * defined by a struct_hyperslab 3D hyperslab structure. The returned set is not itself
 * a structured mesh set (i.e. it doesnt contain the eight collections or the topology
 * relation).
 *
 * Return:      On success, returns a set that contains a collection of hex elements
 * defined as a subset of the given structured mesh block's hex elements.  
 * 
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *create_elem_subset(SAF_Set *a_blockSet, /*the original hex set or node set*/
			    struct_hyperslab a_doList /*definition of the hyperslab*/
			    )
{
   char l_str[256];
   int l_numHexes,l_x,l_y,l_z;
   SAF_Rel rel;
   SAF_Set *l_subBlockHexSet = (SAF_Set*)malloc(1*sizeof(SAF_Set));
   if(!l_subBlockHexSet) { printinfo("malloc failed for l_subBlockHexSet\n");  exit(-1); }

   /*Fix any degeneracy(social engineering)*/
   l_x = a_doList.x_count; if(l_x<0) l_x=0;
   l_y = a_doList.y_count; if(l_y<0) l_y=0;
   l_z = a_doList.z_count; if(l_z<0) l_z=0;
   l_numHexes = l_x*l_y*l_z;

   printinfo("create_elem_subset hslab=(%d,%d,%d)(%d,%d,%d)(%d,%d,%d)",
	   a_doList.x_start,l_x,a_doList.x_skip,
	   a_doList.y_start,l_y,a_doList.y_skip,
	   a_doList.z_start,l_z,a_doList.z_skip );

   sprintf(l_str,"hex_var_set%d_pr%d",g_numSubsetsCreated++,g_rank);
   saf_declare_set(SAF_EACH, g_db, l_str, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_subBlockHexSet );
   saf_declare_collection(SAF_EACH, l_subBlockHexSet, &g_elems,  SAF_CELLTYPE_HEX, l_numHexes,
                          MY_SAF_INDEXSPEC(l_x,l_y,l_z), SAF_DECOMP_TRUE);
   saf_declare_subset_relation(SAF_EACH, g_db, a_blockSet, l_subBlockHexSet, SAF_COMMON(&g_elems), 
			       SAF_HSLAB, SAF_INT, NULL, 0, NULL, &rel);
   saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, (int *)&a_doList, 0, NULL, g_safFile);

   return(l_subBlockHexSet);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create an X-Edge Element Subset of a Structured Mesh Set
 *
 * Description: 
 * Creates a new set with an x-pointing edge element collection that is a hyperslab subset
 * of the argument structured mesh set's x-pointing edge elements. The subset is 
 * defined by a struct_hyperslab 3D hyperslab structure. The returned set is not itself
 * a structured mesh set (i.e. it doesnt contain the eight collections or the topology
 * relation).
 *
 * Return:      On success, returns a set that contains a collection of x-pointing edge elements
 * defined as a subset of the given structured mesh block's x-pointing edge elements. 
 * 
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *create_x_edge_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList)
{
  int l_numXEdges,l_x,l_y,l_z;
  char l_str[256];
  SAF_Rel rel;
  SAF_Set *l_subSet = (SAF_Set*)malloc(1*sizeof(SAF_Set));
  if(!l_subSet) { printinfo("malloc failed for l_subSet\n");  exit(-1); }


  /*Fix any degeneracy(social engineering)*/
  l_x = a_doList.x_count; if(l_x<0) l_x=0;
  l_y = a_doList.y_count; if(l_y<0) l_y=0;
  l_z = a_doList.z_count; if(l_z<0) l_z=0;
  l_numXEdges = l_x*l_y*l_z;


  sprintf(l_str,"x_edge_var_set%d_pr%d",g_numSubsetsCreated++,g_rank);
  saf_declare_set(SAF_EACH, g_db, l_str, 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_subSet );
  saf_declare_collection(SAF_EACH, l_subSet, &g_xEdges,  SAF_CELLTYPE_LINE, l_numXEdges,
			 MY_SAF_INDEXSPEC(l_x,l_y,l_z), SAF_DECOMP_TRUE);
  saf_declare_subset_relation(SAF_EACH, g_db, a_blockSet, l_subSet, SAF_COMMON(&g_xEdges), 
			      SAF_HSLAB, SAF_INT, NULL, 0, NULL, &rel);

  saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, (int *)&a_doList, 0, NULL, g_safFile);

  return(l_subSet);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Y-Edge Element Subset of a Structured Mesh Set
 *
 * Description:  
 * Creates a new set with a y-pointing edge element collection that is a hyperslab subset
 * of the argument structured mesh set's y-pointing edge elements. The subset is 
 * defined by a struct_hyperslab 3D hyperslab structure. The returned set is not itself
 * a structured mesh set (i.e. it doesnt contain the eight collections or the topology
 * relation).
 *
 * Return:      On success, returns a set that contains a collection of y-pointing edge elements
 * defined as a subset of the given structured mesh block's y-pointing edge elements. 
 * 
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *create_y_edge_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList)
{
  int l_numYEdges,l_x,l_y,l_z;
  char l_str[256];
  SAF_Rel rel;
  SAF_Set *l_subSet = (SAF_Set*)malloc(1*sizeof(SAF_Set));
  if(!l_subSet) { printinfo("malloc failed for l_subSet\n");  exit(-1); }

  /*Fix any degeneracy(social engineering)*/
  l_x = a_doList.x_count; if(l_x<0) l_x=0;
  l_y = a_doList.y_count; if(l_y<0) l_y=0;
  l_z = a_doList.z_count; if(l_z<0) l_z=0;
  l_numYEdges = l_x*l_y*l_z;


  sprintf(l_str,"y_edge_var_set%d_pr%d",g_numSubsetsCreated++,g_rank);
  saf_declare_set(SAF_EACH, g_db, l_str, 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_subSet );
  saf_declare_collection(SAF_EACH, l_subSet, &g_yEdges,  SAF_CELLTYPE_LINE, l_numYEdges,
			 MY_SAF_INDEXSPEC(l_x,l_y,l_z), SAF_DECOMP_TRUE);
  saf_declare_subset_relation(SAF_EACH, g_db, a_blockSet, l_subSet, SAF_COMMON(&g_yEdges), 
			      SAF_HSLAB, SAF_INT, NULL, 0, NULL, &rel);
  saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, (int *)&a_doList, 0, NULL, g_safFile);
  return(l_subSet);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Z-Edge Element Subset of a Structured Mesh Set
 *
 * Description:  
 * Creates a new set with a z-pointing edge element collection that is a hyperslab subset
 * of the argument structured mesh set's z-pointing edge elements. The subset is 
 * defined by a struct_hyperslab 3D hyperslab structure. The returned set is not itself
 * a structured mesh set (i.e. it doesnt contain the eight collections or the topology
 * relation).
 *
 * Return:      On success, returns a set that contains a collection of z-pointing edge elements
 * defined as a subset of the given structured mesh block's z-pointing edge elements. 
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *create_z_edge_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList)
{
  int l_numZEdges,l_x,l_y,l_z;
  char l_str[256];
  SAF_Rel rel;
  SAF_Set *l_subSet = (SAF_Set*)malloc(1*sizeof(SAF_Set));
  if(!l_subSet) { printinfo("malloc failed for l_subSet\n");  exit(-1); }

  /*Fix any degeneracy(social engineering)*/
  l_x = a_doList.x_count; if(l_x<0) l_x=0;
  l_y = a_doList.y_count; if(l_y<0) l_y=0;
  l_z = a_doList.z_count; if(l_z<0) l_z=0;
  l_numZEdges = l_x*l_y*l_z;


  sprintf(l_str,"z_edge_var_set%d_pr%d",g_numSubsetsCreated++,g_rank);
  saf_declare_set(SAF_EACH, g_db, l_str, 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_subSet );
  saf_declare_collection(SAF_EACH, l_subSet, &g_zEdges,  SAF_CELLTYPE_LINE, l_numZEdges,
			 MY_SAF_INDEXSPEC(l_x,l_y,l_z), SAF_DECOMP_TRUE);
  saf_declare_subset_relation(SAF_EACH, g_db, a_blockSet, l_subSet, SAF_COMMON(&g_zEdges), 
			      SAF_HSLAB, SAF_INT, NULL, 0, NULL, &rel);
  saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, (int *)&a_doList, 0, NULL, g_safFile);
  return(l_subSet);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create an X-Face Element Subset of a Structured Mesh Set
 *
 * Description: 
 * Creates a new set with an x-facing face element collection that is a hyperslab subset
 * of the argument structured mesh set's x-facing face elements. The subset is 
 * defined by a struct_hyperslab 3D hyperslab structure. The returned set is not itself
 * a structured mesh set (i.e. it doesnt contain the eight collections or the topology
 * relation).
 *
 * Return:      On success, returns a set that contains a collection of x-facing face elements
 * defined as a subset of the given structured mesh block's x-facing face elements. 
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *create_x_face_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList)
{
  int l_numXFaces,l_x,l_y,l_z;
  char l_str[256];
  SAF_Rel rel;
  SAF_Set *l_subSet = (SAF_Set*)malloc(1*sizeof(SAF_Set));
  if(!l_subSet) { printinfo("malloc failed for l_subSet\n");  exit(-1); }

  /*Fix any degeneracy(social engineering)*/
  l_x = a_doList.x_count; if(l_x<0) l_x=0;
  l_y = a_doList.y_count; if(l_y<0) l_y=0;
  l_z = a_doList.z_count; if(l_z<0) l_z=0;
  l_numXFaces = l_x*l_y*l_z;


  sprintf(l_str,"x_face_var_set%d_pr%d",g_numSubsetsCreated++,g_rank);
  saf_declare_set(SAF_EACH, g_db, l_str, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_subSet );
  saf_declare_collection(SAF_EACH, l_subSet, &g_xFaces,  SAF_CELLTYPE_QUAD, l_numXFaces,
			 MY_SAF_INDEXSPEC(l_x,l_y,l_z), SAF_DECOMP_TRUE);
  saf_declare_subset_relation(SAF_EACH, g_db, a_blockSet, l_subSet, SAF_COMMON(&g_xFaces), 
			      SAF_HSLAB, SAF_INT, NULL, 0, NULL, &rel);
  { 
    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, (int *)&a_doList, 0, NULL, g_safFile);
  }
  return(l_subSet);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Y-Face Element Subset of a Structured Mesh Set
 *
 * Description: 
 * Creates a new set with a y-facing face element collection that is a hyperslab subset
 * of the argument structured mesh set's y-facing face elements. The subset is 
 * defined by a struct_hyperslab 3D hyperslab structure. The returned set is not itself
 * a structured mesh set (i.e. it doesnt contain the eight collections or the topology
 * relation).
 *
 * Return:      On success, returns a set that contains a collection of y-facing face elements
 * defined as a subset of the given structured mesh block's y-facing face elements. 
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *create_y_face_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList)
{
  int l_numYFaces,l_x,l_y,l_z;
  char l_str[256];
  SAF_Rel rel;
  SAF_Set *l_subSet = (SAF_Set*)malloc(1*sizeof(SAF_Set));
  if(!l_subSet) { printinfo("malloc failed for l_subSet\n");  exit(-1); }

  /*Fix any degeneracy(social engineering)*/
  l_x = a_doList.x_count; if(l_x<0) l_x=0;
  l_y = a_doList.y_count; if(l_y<0) l_y=0;
  l_z = a_doList.z_count; if(l_z<0) l_z=0;
  l_numYFaces = l_x*l_y*l_z;


  sprintf(l_str,"y_face_var_set%d_pr%d",g_numSubsetsCreated++,g_rank);
  saf_declare_set(SAF_EACH, g_db, l_str, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_subSet );
  saf_declare_collection(SAF_EACH, l_subSet, &g_yFaces,  SAF_CELLTYPE_QUAD, l_numYFaces,
			 MY_SAF_INDEXSPEC(l_x,l_y,l_z), SAF_DECOMP_TRUE);
  saf_declare_subset_relation(SAF_EACH, g_db, a_blockSet, l_subSet, SAF_COMMON(&g_yFaces), 
			      SAF_HSLAB, SAF_INT, NULL, 0, NULL, &rel);
  { 
    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, (int *)&a_doList, 0, NULL, g_safFile);
  }
  return(l_subSet);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Z-Face Element Subset of a Structured Mesh Set
 *
 * Description: 
 * Creates a new set with a z-facing face element collection that is a hyperslab subset
 * of the argument structured mesh set's z-facing face elements. The subset is 
 * defined by a struct_hyperslab 3D hyperslab structure. The returned set is not itself
 * a structured mesh set (i.e. it doesnt contain the eight collections or the topology
 * relation).
 *
 * Return:      On success, returns a set that contains a collection of z-facing face elements
 * defined as a subset of the given structured mesh block's z-facing face elements. 
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *create_z_face_subset(SAF_Set *a_blockSet, struct_hyperslab a_doList)
{
  int l_numZFaces,l_x,l_y,l_z;
  char l_str[256];
  SAF_Rel rel;
  SAF_Set *l_subSet = (SAF_Set*)malloc(1*sizeof(SAF_Set));
  if(!l_subSet) { printinfo("malloc failed for l_subSet\n");  exit(-1); }

  /*Fix any degeneracy(social engineering)*/
  l_x = a_doList.x_count; if(l_x<0) l_x=0;
  l_y = a_doList.y_count; if(l_y<0) l_y=0;
  l_z = a_doList.z_count; if(l_z<0) l_z=0;
  l_numZFaces = l_x*l_y*l_z;


  sprintf(l_str,"z_face_var_set%d_pr%d",g_numSubsetsCreated++,g_rank);
  saf_declare_set(SAF_EACH, g_db, l_str, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_subSet );
  saf_declare_collection(SAF_EACH, l_subSet, &g_zFaces,  SAF_CELLTYPE_QUAD, l_numZFaces,
			 MY_SAF_INDEXSPEC(l_x,l_y,l_z), SAF_DECOMP_TRUE);
  saf_declare_subset_relation(SAF_EACH, g_db, a_blockSet, l_subSet, SAF_COMMON(&g_zFaces), 
			      SAF_HSLAB, SAF_INT, NULL, 0, NULL, &rel);
  { 
    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, (int *)&a_doList, 0, NULL, g_safFile);
  }
  return(l_subSet);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Variable Instance
 *
 * Description: 
 * Creates and writes a field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. This function is not directly called by the writer. Instead, functions like
 * write_node_variable are called, which in turn call this function.
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_variable(SAF_Set *a_set,        /*the set to write to*/
			      const char *a_varName, /*the name of the variable*/
			      const char *a_varUnits,/*the variable units*/
			      MY_PRECISION *a_varData,     /*array of variable values, column-major ordered*/
			      int a_timestep,         /*an integer >= 0*/
			      SAF_Cat a_cat
			      )
{
  SAF_FieldTmpl *l_varFieldTemplate = 0;
  SAF_Unit *l_units;
  SAF_Field l_varField;
  char l_str[256];
  int l_maxTopoDim=0;
  SAF_Cat *l_highestCat=0;
  int l_isDegenerateSubset=0;
  char *l_catName=0;
  int l_numFieldTemplatesFound=0;

  sprintf(l_str,"%s_fld_tmpl",a_varName);

  /*note: field templates always use SAF_ALL, because they are independent of the set*/
  
  /*try to find an existing field template that matches the one we would create*/
  saf_find_field_tmpls(SAF_ALL,g_db,l_str,SAF_ALGTYPE_SCALAR, SAF_UNITY,
		       SAF_QNAME(g_db,a_varName),&l_numFieldTemplatesFound,
		       &l_varFieldTemplate);

  if(l_numFieldTemplatesFound>1)
    {
      /*this shouldnt happen, because we are only creating field templates when
	we cant find one already created*/
      printinfo("write_variable error: found >1 matching template?\n");
      exit(-1);
    }

  /*we found no existing field template, so create one*/
  if(!l_numFieldTemplatesFound)
    {
      l_varFieldTemplate = (SAF_FieldTmpl *)malloc(sizeof(SAF_FieldTmpl));
      if(!l_varFieldTemplate) { printinfo("malloc failed for l_varFieldTemplate\n");  exit(-1); }
      saf_declare_field_tmpl(SAF_ALL, g_db, l_str, SAF_ALGTYPE_SCALAR, SAF_UNITY,  
			     SAF_QNAME(g_db,a_varName), 1, NULL, l_varFieldTemplate );
    }


  l_units = saf_find_one_unit(g_db, a_varUnits,NULL);

  saf_describe_category(SAF_ALL,&a_cat,&l_catName,NULL,NULL);

  /*create the name of the field*/
  {
    char *l_quantName=0;
    saf_describe_quantity(SAF_ALL,SAF_QNAME(g_db,a_varName),&l_quantName,NULL,NULL,NULL,NULL);
    sprintf(l_str,"%s%d",l_quantName,a_timestep);
    free(l_quantName);
  }

  if(!g_rank ) printinfo("\n write_variable %s\n",l_str);

  saf_describe_set(SAF_EACH,a_set,NULL,&l_maxTopoDim,NULL,NULL,NULL,NULL,NULL );

  if( l_maxTopoDim==3 ) l_highestCat = &g_elems;
  else if( l_maxTopoDim==0 )l_highestCat = &g_nodes;
  else
    {
      /*since the highest dim collection on this set is apparently a edge or face
	collection, (i.e. this is an edge or face subset), we must assume it can no
	longer be mapped back to a node set (p.s. degenerate is probably not the
	right term to use */
      l_isDegenerateSubset=1;
    }

  if( l_isDegenerateSubset )
    {
      saf_declare_field(SAF_EACH, g_db,l_varFieldTemplate, l_str,    a_set,        l_units,
			SAF_SELF(g_db),  
			SAF_ZONAL(&a_cat), 
			MY_SAF_PRECISION,
			NULL, SAF_INTERLEAVE_NONE,SAF_IDENTITY, NULL,
			&l_varField);
    }
  else
    {
      saf_declare_field(SAF_EACH, g_db,l_varFieldTemplate, l_str,    a_set,       l_units,
			SAF_SELF(g_db),   
			SAF_NODAL(&a_cat,l_highestCat), 
			MY_SAF_PRECISION,
			NULL, SAF_INTERLEAVE_NONE,SAF_IDENTITY, NULL,
			&l_varField);
    }

  saf_write_field(SAF_EACH, &l_varField, SAF_WHOLE_FIELD, 1,
		  0, (void**)&a_varData, g_safFile);


  add_field_to_timestep_data( &l_varField, l_varFieldTemplate, a_timestep);

  if(l_catName) free(l_catName);

  return(l_varFieldTemplate);
}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Variable Instance Using a Given Template
 *
 * Description: 
 * Creates and writes a field on the given set using the given field template, and associates 
 * it with the given  timestep in the global list. This function is not directly called by the writer. 
 * Instead, functions like write_node_variable_using_existing_template are called, which in turn call this function.
 * 
 *--------------------------------------------------------------------------------------------------- */
void write_variable_using_existing_template(SAF_Set *a_set, 
					    SAF_FieldTmpl *a_varFieldTemplate,
					    const char *a_varName, 
					    const char *a_varUnits, 
					    MY_PRECISION *a_varData,/*array of variable values, column-major ordered*/
					    int a_timestep,
					    SAF_Cat a_cat
						   )
{
  SAF_Unit *l_units;
  SAF_Field l_varField;
  char l_str[256];
  int l_maxTopoDim=0;
  SAF_Cat *l_highestCat=0;
  int l_isDegenerateSubset=0;
  char *l_catName=0;

  l_units = saf_find_one_unit(g_db, a_varUnits,NULL);



  /*create the name of the field*/
#if 0
  /*pre-022003*/
  saf_describe_category(SAF_ALL,a_cat,&l_catName,NULL,NULL);
  sprintf(l_str,"%s%d",l_catName,a_timestep);
#else
  {/*made this change when I added variable_names.c to make naming more intelligent*/
    char *l_quantName=0;
    saf_describe_quantity(SAF_ALL,SAF_QNAME(g_db,a_varName),&l_quantName,NULL,NULL,NULL,NULL);
    sprintf(l_str,"%s%d",l_quantName,a_timestep);
    free(l_quantName);
  }
#endif





  if(!g_rank ) printinfo("\n write_variable_using_existing_template %s\n",l_str);

  /*figure out whether the set is nodes,edges,faces or elems*/
  saf_describe_set(SAF_EACH,a_set,NULL,&l_maxTopoDim,NULL,NULL,NULL,NULL,NULL );

  if( l_maxTopoDim==3 ) l_highestCat = &g_elems;
  else if( l_maxTopoDim==0 ) l_highestCat = &g_nodes;
  else
    {
      /*since the highest dim collection on this set is apparently a edge or face
	collection, (i.e. this is an edge or face subset), we must assume it can no
	longer be mapped back to a node set (p.s. degenerate is probably not the
	right term to use */
      l_isDegenerateSubset=1;
    }

  if( l_isDegenerateSubset )
    {
      saf_declare_field(SAF_EACH, g_db,a_varFieldTemplate, l_str,    a_set,       l_units,
			SAF_SELF(g_db),   
			SAF_ZONAL(&a_cat), 
			MY_SAF_PRECISION,
			NULL, SAF_INTERLEAVE_NONE,SAF_IDENTITY, NULL,
			&l_varField);
    }
  else
    {
      saf_declare_field(SAF_EACH, g_db,a_varFieldTemplate, l_str,    a_set,       l_units,
			SAF_SELF(g_db),   
			SAF_NODAL(&a_cat,l_highestCat),
			MY_SAF_PRECISION,
			NULL, SAF_INTERLEAVE_NONE,SAF_IDENTITY, NULL,
			&l_varField);
    }

  saf_write_field(SAF_EACH, &l_varField, SAF_WHOLE_FIELD, 1,
		  0, (void**)&a_varData, g_safFile);
  add_field_to_timestep_data( &l_varField, a_varFieldTemplate, a_timestep );


  if(l_catName) free(l_catName);

  return;
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Node Variable
 *
 * Description: 
 * Creates and writes a node variable field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. 
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_node_variable(SAF_Set *a_set,        /*the set to write to*/
				   const char *a_varName, /*the name of the variable*/
				   const char *a_varUnits,/*the variable units*/
				   MY_PRECISION *a_varData,     /*array of variable values, column-major ordered*/
				   int a_timestep         /*an integer >= 0*/
				   )
{
  SAF_FieldTmpl *l_tmpl;
  l_tmpl = write_variable( a_set,a_varName,a_varUnits,a_varData,a_timestep, g_nodes );
  return( l_tmpl );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Node Variable Using a Given Template
 *
 * Description: 
 * Creates and writes a node variable field on the given set using a given template, and associates it 
 * with the given timestep in the global list.
 *
 *--------------------------------------------------------------------------------------------------- */
void write_node_variable_using_existing_template(SAF_Set *a_set, 
						 SAF_FieldTmpl *a_varFieldTemplate,
						 const char *a_varName, 
						 const char *a_varUnits, 
						 MY_PRECISION *a_varData,/*array of variable values, column-major ordered*/
						 int a_timestep
						 )
{
  write_variable_using_existing_template(a_set,a_varFieldTemplate,a_varName,a_varUnits,a_varData,a_timestep,g_nodes);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write an X Edge Variable
 *
 * Description: 
 * Creates and writes an x-pointing edge variable field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. 
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_x_edge_variable(SAF_Set *a_set,const char *a_varName, const char *a_varUnits,
				     MY_PRECISION *a_varData, int a_timestep )
{
  SAF_FieldTmpl *l_tmpl;
  l_tmpl=write_variable( a_set,a_varName,a_varUnits,a_varData,a_timestep, g_xEdges );
  return( l_tmpl );
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Y Edge Variable
 *
 * Description: 
 * Creates and writes a y-pointing edge variable field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. 
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_y_edge_variable(SAF_Set *a_set,const char *a_varName, const char *a_varUnits,
				   MY_PRECISION *a_varData, int a_timestep )
{
  SAF_FieldTmpl *l_tmpl;
   l_tmpl=write_variable( a_set,a_varName,a_varUnits,a_varData,a_timestep, g_yEdges );
  return( l_tmpl );
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Z Edge Variable
 *
 * Description: 
 * Creates and writes a z-pointing edge variable field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. 
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_z_edge_variable(SAF_Set *a_set,const char *a_varName, const char *a_varUnits,
				   MY_PRECISION *a_varData, int a_timestep )
{
  SAF_FieldTmpl *l_tmpl;
  l_tmpl = write_variable( a_set,a_varName,a_varUnits,a_varData,a_timestep, g_zEdges );
  return( l_tmpl );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write an X Edge Variable Using a Given Template
 *
 * Description: 
 * Creates and writes an x-pointing edge variable field on the given set using a given template,  
 * and associates it with the given timestep in the global list.
 *
 *--------------------------------------------------------------------------------------------------- */
void write_x_edge_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep )
{
  write_variable_using_existing_template(a_set,a_varFieldTemplate,a_varName,a_varUnits,a_varData,a_timestep,g_xEdges);
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Y Edge Variable Using a Given Template
 *
 * Description: 
 * Creates and writes a y-pointing edge variable field on the given set using a given template,  
 * and associates it with the given timestep in the global list.
 *
 *--------------------------------------------------------------------------------------------------- */
void write_y_edge_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep )
{
  write_variable_using_existing_template(a_set,a_varFieldTemplate,a_varName,a_varUnits,a_varData,a_timestep,g_yEdges);
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Z Edge Variable Using a Given Template
 *
 * Description: 
 * Creates and writes a z-pointing edge variable field on the given set using a given template,  
 * and associates it with the given timestep in the global list.
 *
 *--------------------------------------------------------------------------------------------------- */
void write_z_edge_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						   const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep )
{
  write_variable_using_existing_template(a_set,a_varFieldTemplate,a_varName,a_varUnits,a_varData,a_timestep,g_zEdges);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write an X Face Variable
 *
 * Description: 
 * Creates and writes an x-facing face variable field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. 
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_x_face_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
				   MY_PRECISION *a_varData, int a_timestep )
{
  SAF_FieldTmpl *l_tmpl;
  l_tmpl=write_variable( a_set,a_varName,a_varUnits,a_varData,a_timestep, g_xFaces );
  return( l_tmpl );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Y Face Variable
 *
 * Description: 
 * Creates and writes a y-facing face variable field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. 
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_y_face_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
				   MY_PRECISION *a_varData, int a_timestep )
{
  SAF_FieldTmpl *l_tmpl;
  l_tmpl=write_variable( a_set,a_varName,a_varUnits,a_varData,a_timestep, g_yFaces );
  return( l_tmpl );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Z Face Variable
 *
 * Description: 
 * Creates and writes a z-facing face variable field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. 
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_z_face_variable(SAF_Set *a_set, const char *a_varName, const char *a_varUnits,
				   MY_PRECISION *a_varData, int a_timestep )
{
  SAF_FieldTmpl *l_tmpl;
  l_tmpl=write_variable( a_set,a_varName,a_varUnits,a_varData,a_timestep, g_zFaces );
  return( l_tmpl );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write an X Face Variable Using a Given Template
 *
 * Description: 
 * Creates and writes an x-facing face variable field on the given set using a given template,  
 * and associates it with the given timestep in the global list.
 *
 *--------------------------------------------------------------------------------------------------- */
void write_x_face_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						 const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep )
{
  write_variable_using_existing_template(a_set,a_varFieldTemplate,a_varName,a_varUnits,a_varData,a_timestep,g_xFaces);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Y Face Variable Using a Given Template
 *
 * Description: 
 * Creates and writes a y-facing face variable field on the given set using a given template,  
 * and associates it with the given timestep in the global list.
 *
 *--------------------------------------------------------------------------------------------------- */
void write_y_face_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						 const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep )
{
  write_variable_using_existing_template(a_set,a_varFieldTemplate,a_varName,a_varUnits,a_varData,a_timestep,g_yFaces);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Z Face Variable Using a Given Template
 *
 * Description: 
 * Creates and writes a z-facing face variable field on the given set using a given template,  
 * and associates it with the given timestep in the global list.
 *
 *--------------------------------------------------------------------------------------------------- */
void write_z_face_variable_using_existing_template(SAF_Set *a_set, SAF_FieldTmpl *a_varFieldTemplate, const char *a_varName, 
						 const char *a_varUnits, MY_PRECISION *a_varData, int a_timestep )
{
   write_variable_using_existing_template(a_set,a_varFieldTemplate,a_varName,a_varUnits,a_varData,a_timestep,g_zFaces);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Hex Element Variable
 *
 * Description: 
 * Creates and writes a hex element variable field on the given set, associates it with the given
 * timestep in the global list, and creates a new field template if a suitable existing one is
 * not found. 
 * 
 * Return:      On success, returns a pointer to the field template that was used.
 *--------------------------------------------------------------------------------------------------- */
SAF_FieldTmpl *write_elem_variable(SAF_Set *a_set,        /*the set to write to*/
				     const char *a_varName, /*the name of the variable*/
				     const char *a_varUnits,/*the variable units*/
				   MY_PRECISION *a_varData,     /*array of variable values, column-major ordered*/
				     int a_timestep         /*an integer >= 0*/
				     )
{
  SAF_FieldTmpl *l_tmpl;
  l_tmpl=write_variable( a_set,a_varName,a_varUnits,a_varData,a_timestep, g_elems );
  return( l_tmpl );
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write a Hex Element Variable Using a Given Template
 *
 * Description: 
 * Creates and writes a hex element variable field on the given set using a given template, and associates it 
 * with the given timestep in the global list.
 *
 *--------------------------------------------------------------------------------------------------- */
void write_elem_variable_using_existing_template(SAF_Set *a_set, 
						 SAF_FieldTmpl *a_varFieldTemplate,
						 const char *a_varName, 
						 const char *a_varUnits, 
						 MY_PRECISION *a_varData,/*array of variable values, column-major ordered*/
						 int a_timestep
						 )
{
  write_variable_using_existing_template(a_set,a_varFieldTemplate,a_varName,a_varUnits,a_varData,a_timestep,g_elems);
}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Curvilinear Structured Mesh Block Using Separable Data
 *
 * Description: 
 * Given 3 1D arrays of length X, Y and Z, this function creates a structured
 * block (a SAF_Set containing the eight required collections and one topology relation) of size XYZ.
 * Although the data is passed to the function as separable (i.e. defined by 3 1D arrays of size
 * X, Y and Z rather than 3 1D arrays of size XYZ), it is stored in the set as curvilinear. 
 *
 * This method deliberately wastes a lot of space: there is no good reason to use it. It
 * only exists to make testing a bit easier.
 *
 * Return:      On success, returns the created set.
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *add_separable_block_as_curvilinear(  const char *a_blockName, /*the name to give to the block*/
		     const char *a_units, /*the distance units for the coordinates*/
		     int a_xDim, int a_yDim, int a_zDim, 
		     MY_PRECISION *a_xCoords, MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords 
		     )
{
   int l_numNodes = a_xDim*a_yDim*a_zDim;
   SAF_Field l_coordsField,           /* Handle to the coordinate field. */
             l_coordsComponentField[3];  /* Handle to the 3 components of the coordinate field. */
   SAF_Unit *l_units;            /* Handle to the units for the coordinates. */
   SAF_Set *l_blockNodeSet= (SAF_Set *)malloc( sizeof(SAF_Set) );

   /*note: if we wanted to make these coordinates time dependent, we would have to
     declare l_coordsField and l_coordsFieldTemplate as pointers, and malloc them
     here, so that they still exist when we create the states & suites*/

   if(!l_blockNodeSet) { printinfo("malloc failed for l_blockNodeSet\n");  exit(-1); }

   printinfo("\nproc %d trying to add_separable_block_as_curvilinear named %s, of size %dx%dx%d\n",g_rank,a_blockName,a_xDim,a_yDim,a_zDim);

   saf_declare_set(SAF_EACH, g_db, a_blockName, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_blockNodeSet );


   declare_all_structured_collections(*l_blockNodeSet,a_xDim,a_yDim,a_zDim);

   /* Get a handle to the units for this field. */
   l_units = saf_find_one_unit(g_db, a_units,NULL);


   /* Declare the fields. */
   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "X_coord_field",    l_blockNodeSet,       l_units,
                     SAF_SELF(g_db),   SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField);
   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "Y_coord_field",    l_blockNodeSet,       l_units,
                     SAF_SELF(g_db),   SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField+1);
   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "Z_coord_field",    l_blockNodeSet,       l_units,
                     SAF_SELF(g_db),  SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField+2);

   saf_declare_field(SAF_EACH, g_db,&g_coordsFieldTemplate, "coord_field", l_blockNodeSet, l_units,
                     SAF_SELF(g_db),  SAF_NODAL(&g_nodes,&g_elems), MY_SAF_PRECISION,
                     l_coordsComponentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                     &l_coordsField);




   /* Write the coordinate field. */
   {
     int i,j,k;
     MY_PRECISION *l_data=0;
     MY_PRECISION *l_ptr;

     l_data = (MY_PRECISION *)malloc(3*l_numNodes*sizeof(MY_PRECISION));
     if(!l_data) { printinfo("\nmalloc of %d bytes failed for l_data\n",(int)(3*l_numNodes*sizeof(MY_PRECISION)));  exit(-1); }
     l_ptr=l_data;

     /*transfer the data from the 3 argument arrays into l_data,
       using column-major (fortran)(1st col loaded before 2nd col, etc) order */
     for(k=0;k<a_zDim;k++)
       for(j=0;j<a_yDim;j++)
	 for(i=0;i<a_xDim;i++)
	   {
	     l_ptr[0]=a_xCoords[i];
	     l_ptr[1]=a_yCoords[j];
	     l_ptr[2]=a_zCoords[k];
	     l_ptr += 3;
	   }

     saf_write_field(SAF_EACH, &l_coordsField, SAF_WHOLE_FIELD, 1,
                   0, (void**)&l_data, g_safFile);

     free(l_data);
   }
  

   /* Specify that it is a coordinate field  */
   saf_declare_coords(SAF_EACH, &l_coordsField);

   /* Specify that they are all coordinate fields */
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[0]) );
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[1]) );
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[2]) );
   

   saf_declare_default_coords(SAF_EACH,l_blockNodeSet,&l_coordsField);

   add_set_to_suites_list( *l_blockNodeSet );

   return(l_blockNodeSet);
}





/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Add an Unstructured Node Cloud
 *
 * Description: 
 * This function has nothing to do with structured meshes. It creates a set
 * containing a 1D array of points, with no topology and no variables.
 *
 * Return:      Returns the created set on success.
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set *add_unstructured_nodeset(  const char *a_blockName, /*the name to give to the block*/
				    const char *a_units, /*the distance units for the coordinates*/
				    int a_timestep,
				    int a_numNodes, MY_PRECISION *a_xyzCoords
				    )
{
   SAF_Field l_coordsField;/* Handle to the coordinate field. */
   SAF_Field l_coordsComponentField[3];  /* Handle to the 3 components of the coordinate field. */
   SAF_Unit *l_units;            /* Handle to the units for the coordinates. */
   SAF_Set *l_unstrNodeSet= (SAF_Set *)malloc( sizeof(SAF_Set) );

   if(!l_unstrNodeSet) { printinfo("malloc failed for l_unstrNodeSet\n");  exit(-1); }

   /*   printinfo("\nproc %d trying to add_unstructured_nodeset named %s\n",g_rank,a_blockName);*/


   saf_declare_set(SAF_EACH, g_db, a_blockName, 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, l_unstrNodeSet );

   saf_declare_collection(SAF_EACH, l_unstrNodeSet, &g_nodes,  SAF_CELLTYPE_POINT, a_numNodes,
                          SAF_1DF(a_numNodes), SAF_DECOMP_TRUE);

   /* Get a handle to the units for this field. */
   l_units = saf_find_one_unit(g_db, a_units,NULL);

   /* Declare the fields. */

   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "X_coord_field",    l_unstrNodeSet,        l_units,
                     SAF_SELF(g_db),  SAF_ZONAL(&g_nodes), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField);
   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "Y_coord_field",     l_unstrNodeSet,       l_units,
                     SAF_SELF(g_db),  SAF_ZONAL(&g_nodes), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField+1);
   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), "Z_coord_field",   l_unstrNodeSet,        l_units,
                     SAF_SELF(g_db),  SAF_ZONAL(&g_nodes), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField+2);

   saf_declare_field(SAF_EACH, g_db,&g_coordsFieldTemplate, "coord_field", l_unstrNodeSet,  l_units,
                     SAF_SELF(g_db),  SAF_ZONAL(&g_nodes), MY_SAF_PRECISION,
                     l_coordsComponentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                     &l_coordsField);


   /* Write the coordinate field. */
   saf_write_field(SAF_EACH, &l_coordsField, SAF_WHOLE_FIELD, 1,
                   0, (void**)&a_xyzCoords, g_safFile);

   /* Specify that it is a coordinate field */
   saf_declare_coords(SAF_EACH, &l_coordsField);

   /* Specify that they are all coordinate fields */
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[0]) );
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[1]) );
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[2]) );

   saf_declare_default_coords(SAF_EACH,l_unstrNodeSet,&l_coordsField);

   add_field_to_timestep_data( &l_coordsField, &g_coordsFieldTemplate, a_timestep );

   add_set_to_suites_list( *l_unstrNodeSet );

   return(l_unstrNodeSet);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Add Nodes to an Unstructured Node Cloud
 *
 * Description: 
 * This function has nothing to do with structured meshes. It adds another set of points
 * (e.g. at another timestep) to an existing set of unstructured nodes.
 *
 * Issue:
 * Adding a different number of nodes is a problem. The conflict between extendible sets
 * and SAF_EACH mode is not yet resolved.
 *
 * Return:      Returns 0 on success.
 *
 *--------------------------------------------------------------------------------------------------- */
int add_coords_to_unstructured_nodeset(  SAF_Set a_set,
					 const char *a_units, /*the distance units for the coordinates*/
					 int a_timestep,
					 int a_numNodes, 
					 MY_PRECISION *a_xyzCoords
					 )
{
   SAF_Field l_coordsField;/* Handle to the coordinate field. */
   SAF_Field l_coordsComponentField[3];  /* Handle to the 3 components of the coordinate field. */
   SAF_Unit *l_units;            /* Handle to the units for the coordinates. */
   char l_str[256];


   printinfo("warning add_coords_to_unstructured_nodeset needs work  a_numNodes=%d\n",a_numNodes);
#if 0
  /*XXX jake todo:
    THIS IS NOT RIGHT YET. SHOULDNT RE-DECLARE A COLLECTION ON THE SAME SET.
    COULD MAKE IT EXTENDIBLE, BUT THEN THERE IS THE PROBLEM WITH SAF_EACH,
    PLUS THE PROBLEM ABOUT THE NUMBER OF NODES DECREASING. COULD ALSO JUST
    CREATE NEW CATS, BUT THIS IS UGLY AND DEFEATS THE PURPOSE*/
  /*I THOUGHT I ALREADY FIXED THIS, DID I LOSE THE CODE SOMEWHERE?*/
   saf_declare_collection(SAF_EACH, &a_set, &g_nodes,  SAF_CELLTYPE_POINT, a_numNodes,
                          SAF_1DF(a_numNodes), SAF_DECOMP_TRUE);
#endif

    /* Get a handle to the units for this field. */
   l_units = saf_find_one_unit(g_db, a_units,NULL);

   /* Declare the fields. */
   sprintf(l_str,"X_coord_fld_ts%d",a_timestep);

   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), l_str,    &a_set,        l_units,
                     SAF_SELF(g_db),  SAF_ZONAL(&g_nodes), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField);

   sprintf(l_str,"Y_coord_fld_ts%d",a_timestep);

   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), l_str,   &a_set,         l_units,
                     SAF_SELF(g_db),  SAF_ZONAL(&g_nodes), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField+1);

   sprintf(l_str,"Z_coord_fld_ts%d",a_timestep);

   saf_declare_field(SAF_EACH, g_db,&(g_coordsIndivFieldTemplates[0]), l_str,    &a_set,       l_units,
                     SAF_SELF(g_db),  SAF_ZONAL(&g_nodes), MY_SAF_PRECISION,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     l_coordsComponentField+2);

   sprintf(l_str,"coord_fld_ts%d",a_timestep);

   saf_declare_field(SAF_EACH, g_db,&g_coordsFieldTemplate, l_str,  &a_set,  l_units,
                     SAF_SELF(g_db), SAF_ZONAL(&g_nodes), MY_SAF_PRECISION,
                     l_coordsComponentField, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                     &l_coordsField);


   /* Write the coordinate field. */
   saf_write_field(SAF_EACH, &l_coordsField, SAF_WHOLE_FIELD, 1,
                   0, (void**)&a_xyzCoords, g_safFile);

   /* Specify that it is a coordinate field */
   saf_declare_coords(SAF_EACH, &l_coordsField);

   /* Specify that they are all coordinate fields */
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[0]) );
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[1]) );
   saf_declare_coords(SAF_EACH, &(l_coordsComponentField[2]) );


   add_field_to_timestep_data( &l_coordsField, &g_coordsFieldTemplate, a_timestep );


   return(0);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Does a Given Field Reside On or Below the Given Set?
 *
 * Description: 
 * Given a set and a field, this function determines if the field is on or below (i.e. on 
 * a subset of the set) the field.
 *
 * Return:      Returns 1 for true, 0 otherwise.
 *
 *--------------------------------------------------------------------------------------------------- */
int is_field_on_or_below_set( SAF_Field a_field, SAF_Set a_set )
{
  int i,k;
  int l_numFields=0;
  SAF_Field *l_fields=0;
  SAF_Set *l_subSets=0;
  int l_numSubSets=0;
  /*first check if the field is on the set itself*/
  saf_find_fields(SAF_EACH, g_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT,
		  SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);

  for( i = 0 ; i < l_numFields; i++ )
    {
      if( SAF_EQUIV(&a_field,&(l_fields[i])) )
	{
	  return(1);/*a_field is on a_set*/
	}
    }
  /*next check if the field is on any subsets of the topsets*/
  saf_find_matching_sets(SAF_ALL, g_db, SAF_ANY_NAME, SAF_SPACE, SAF_ANY_TOPODIM,
			 SAF_EXTENDIBLE_TORF, SAF_TOP_FALSE, &l_numSubSets, &l_subSets);
  for( k=0; k<l_numSubSets; k++ )
    {
      int l_numRels=0;
      saf_find_subset_relations(SAF_ALL, g_db,&a_set, &(l_subSets[k]), SAF_ANY_CAT,
				SAF_ANY_CAT, SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE,
				&l_numRels, NULL );
      if( l_numRels > 1 )
	{
	  printinfo("is_field_on_or_below_set error: expect only 1 relation between set and subset, found %d\n",l_numRels);
	}
      if( l_numRels > 0 )
	{
	  int l_found = is_field_on_or_below_set( a_field, l_subSets[k] );
	  if( l_found ) return(1);
	}
    }
    
  if(l_fields) free(l_fields);
  if(l_subSets) free(l_subSets);

  return(0);
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Find Which of a List of Sets Does a Given Field Reside On or Below
 *
 * Description: 
 * Given a list of sets and a field, this function returns the corresponding
 * index for the set that the field is on or below (i.e. on a subset of the set). If the
 * field is not on or below any of the sets, then -1 is returned.
 *
 * Return:      Returns an index from 0 to a_numTopsets-1 if successful, -1 otherwise.
 * 
 *
 *--------------------------------------------------------------------------------------------------- */
int which_of_these_topsets_is_field_from( SAF_Field a_field, int a_numTopsets, SAF_Set *a_topsets )
{
  int j;
  for( j=0; j<a_numTopsets; j++ )
  {
    if( is_field_on_or_below_set(a_field,a_topsets[j]) ) return(j);
  }
  return(-1);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Create a Top Set for the Database
 *
 * Description: 
 * Creates a single top set (using parallel mode SAF_ALL: no matter how many processors 
 * are used, there is still only one top set), of which all of the structured blocks are subsets. This
 * top set is a dummy set that is used to unite all of the blocks under one suite.
 *
 * Return:      Returns the created top set.
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Set create_TOP_SET()
{
  SAF_Set l_topset=SS_SET_NULL;
  SAF_Cat l_cat=SS_CAT_NULL;
  int i, l_numBlocksAllProcs=0;

  /*create the suiteset: a top-topset above all others, a 1-d trivial set*/
  saf_declare_set(SAF_ALL, g_db, "TOP_SET", 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &l_topset);


  /*figure out how many blocks in the entire simulation*/
  l_numBlocksAllProcs = g_numBlocksAdded*g_nprocs;

  saf_declare_category(SAF_ALL,g_db, "blocks",  SAF_BLOCK, 0, &l_cat);
  saf_declare_collection(SAF_ALL, &l_topset, &l_cat, SAF_CELLTYPE_SET, l_numBlocksAllProcs,
			 _saf_indexspec(1, l_numBlocksAllProcs, 0,0), SAF_DECOMP_TRUE);

  /*relate each 'topset' to the suiteset as one block of many*/
  for(i=0;i<g_numBlocksAdded;i++)
    {
      int l_whichEntry = i+g_rank*g_numBlocksAdded;
      SAF_Rel l_rel;
      SAF_Set l_subset = g_blocksAndUnstrNodesetsAdded[i];

      saf_declare_collection(SAF_EACH, &l_subset, &l_cat, SAF_CELLTYPE_SET, 1,
			     _saf_indexspec(1,1,0,0), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_EACH, g_db, &l_topset, &l_subset, 
				  &l_cat, &l_cat, SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,
				  SAF_TUPLES, SAF_INT, NULL, 0, NULL, &l_rel);
      saf_write_subset_relation(SAF_EACH, &l_rel, SAF_INT, &l_whichEntry, 0, NULL, g_safFile);
    }



  /*create a coords field that contains handles pointing to the coord fields
    of each block*/
  {
    SAF_FieldTmpl l_coordsTmpl;
    SAF_Field l_coordsField;
    SAF_Field *l_indivCoordFields = (SAF_Field *)malloc( g_numBlocksAdded * sizeof(SAF_Field));
    SAF_Field *l_allFields = (SAF_Field *)malloc( l_numBlocksAllProcs * sizeof(SAF_Field));


    saf_declare_field_tmpl(SAF_ALL, g_db, "coord_fields_tmpl", SAF_ALGTYPE_FIELD,
			   NULL/*SAF_EMPTY_HANDLE*/, SAF_NOT_APPLICABLE_QUANTITY,
			   SAF_NOT_APPLICABLE_INT,/*This is the # of components.Set to 
						   SAF_NOT_APPLICABLE_INT because it is INhomogeneous?*/ 
			   NULL, &l_coordsTmpl );
    
    saf_declare_field(SAF_ALL, g_db,&l_coordsTmpl, "coord_fields",&l_topset,SAF_NOT_APPLICABLE_UNIT,
		      &l_cat,SAF_ZONAL(&l_cat),
		      0/*DSL_HANDLE*/,NULL,SAF_INTERLEAVE_VECTOR,SAF_IDENTITY,NULL,&l_coordsField);

    saf_declare_coords(SAF_ALL,&(l_coordsField));
    saf_declare_default_coords(SAF_ALL,&l_topset,&l_coordsField);    

    for(i=0;i<g_numBlocksAdded;i++)
      {
	int l_numReturned=0;
	SAF_Field *l_returnedHandles=0;
	SAF_Set l_subset = g_blocksAndUnstrNodesetsAdded[i];
	saf_find_default_coords(SAF_EACH, &l_subset, &l_indivCoordFields[i]);

	/*hooey is this right for sslib?*/
        l_returnedHandles = (SAF_Field *)saf_allgather_handles((ss_pers_t *)(l_indivCoordFields+i), &l_numReturned, NULL);

	/*an unnecessary check*/
	if( g_nprocs != l_numReturned )
	  {
	    printinfo("error saf_allgather_handles g_nprocs=%d l_numReturned=%d\n",g_nprocs,l_numReturned);
	    exit(-1);
	  }

	memcpy( &l_allFields[i*g_nprocs], l_returnedHandles, g_nprocs*sizeof(SAF_Field) );
	free(l_returnedHandles);
      }

    /*just a printout*/
    /*for(i=0;i<l_numBlocksAllProcs;i++)
      { 
      int j;
      char *name=0;
      saf_describe_field(SAF_ALL, l_allFields[i], NULL, &name, NULL, 
      NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL);
      for(j=0;j<g_rank;j++) printinfo("\t\t\t\t");
      printinfo("\t i=%d %s theRow=%d\n",i,name,l_allFields[i].theRow);
      if(name) free(name);
      }*/


    saf_write_field(SAF_ALL, &l_coordsField, SAF_WHOLE_FIELD, 1, SAF_HANDLE, (void **)(&l_allFields), g_safFile);


    free(l_indivCoordFields);
    free(l_allFields);
  }




  return( l_topset );

} /* create_TOP_SET */


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     Write Timestep Data to Database
 *
 * Description: 
 * This function creates the states and suite for the database from the 
 * global list of sets, fields, templates and timesteps. After creating the suite,
 * the global list is emptied.
 *
 * Although it is desirable to have only one suite in a database, it is allowed to 
 * call this function numerous times (creating a new suite each time with whatever
 * blocks and variables are in the global list). 
 *
 *--------------------------------------------------------------------------------------------------- */
void write_timestep_data( MY_PRECISION *a_timeValues )
{
  int j;
  int l_whichStateGroup;
  int l_startIndex=0;
  char l_str[128];
  SAF_Field *l_allProcFields=0;
  SAF_FieldTmpl *l_allProcFieldTemplates=0;
  int l_allProcNumFields=0;
  int *l_allProcTimestepPerField=0;
  int l_lastNumStates=0;

  if(!g_rank) printinfo("\n--------------STARTING STATES&SUITES--------------\n\n");

  g_suiteset = create_TOP_SET();

 
#ifdef HAVE_PARALLEL

  if(!g_rank) printinfo("\nStarting gathering handles: num fields goes from %d to %d\n\n",
			g_currentNumFields,g_currentNumFields*g_nprocs);

  l_allProcNumFields=g_currentNumFields*g_nprocs;  
  l_allProcFields=(SAF_Field *)malloc(l_allProcNumFields*sizeof(SAF_Field));
  l_allProcFieldTemplates=(SAF_FieldTmpl *)malloc(l_allProcNumFields*sizeof(SAF_FieldTmpl));
  l_allProcTimestepPerField=(int *)malloc(l_allProcNumFields*sizeof(int));

  for(j=0;j<g_currentNumFields;j++)
    {
      int i,l_numReturned=0;
      SAF_Field *l_fptr = 0;
      SAF_FieldTmpl *l_tptr = 0;

      l_fptr = saf_allgather_handles((ss_pers_t*)(&g_fields[j]), &l_numReturned, NULL);

      /*dont need to exchange templates, because they are created with SAF_ALL*/
#define DONT_EXCHANGE_TEMPLATES

#ifndef DONT_EXCHANGE_TEMPLATES
      l_tptr = saf_allgather_handles((ss_pers_t*)(&g_fieldTemplates[j]), &l_numReturned, NULL);
#endif

      
      for(i=0;i<g_nprocs;i++) 
	{
	  /*note: assuming here that each proc is writing at the same timestep*/
	  l_allProcTimestepPerField[j*g_nprocs+i] = g_timestepsPerAddedField[j];
	  l_allProcFields[j*g_nprocs+i] = l_fptr[i];
#ifndef DONT_EXCHANGE_TEMPLATES
	  l_allProcFieldTemplates[j*g_nprocs+i] = l_tptr[i];
#else
	  l_allProcFieldTemplates[j*g_nprocs+i] = g_fieldTemplates[j];
#endif
	}
    }

  if(!g_rank) printinfo("\nFinished gathering handles.\n\n");

#else
  l_allProcNumFields=g_currentNumFields;
  l_allProcFields=g_fields;
  l_allProcFieldTemplates=g_fieldTemplates;
  l_allProcTimestepPerField = g_timestepsPerAddedField;
#endif


  sort_timestep_data( l_allProcNumFields, l_allProcFields, l_allProcFieldTemplates,
		      l_allProcTimestepPerField );

  if(!g_rank ) printinfo("write_timestep_data l_allProcNumFields=%d  g_numTimes=%d\n",
			 l_allProcNumFields,g_numTimes);


printf("create_str_mesh getting a sslib message here in saf_declare_suite:\n");/*hooey*/

  /*create the one and only suite*/
  saf_declare_suite(SAF_ALL, g_db, "SUITE", &g_suiteset, NULL, &g_suite);



  /*In the worst case (a different template set for each timestep) there will be
    as many state groups as timesteps*/
  for( l_whichStateGroup=0; l_whichStateGroup<g_numTimes; l_whichStateGroup++ )
    {
      SAF_StateGrp l_stateGroup;
      SAF_StateTmpl l_stateTemplate;
      SAF_FieldTmpl *l_fieldTemplates=0;
      int l_numFieldsPerTime=1;
      int l_numTimesInThisGroup=1;
      int l_done=0;
      SAF_Unit *l_units;

       
      l_units = saf_find_one_unit(g_db, "second",NULL);
	

      /*count the number of fields that will be in this state group*/
      for(j=l_startIndex+1; j<l_allProcNumFields; j++ )
	{
	  if( l_allProcTimestepPerField[j]==l_allProcTimestepPerField[l_startIndex] )
	    {
	      l_numFieldsPerTime++;
	    }
	  else break;
	}

      /*count the number of different times in this group. (i.e. how many times, for
	this block, the series of field templates is the same for a certain timestep
	as for this first timestep)*/
      l_done=0;
      while(!l_done)
	{
	  for(j=l_startIndex; j<l_startIndex+l_numFieldsPerTime; j++ )
	    {
	      int l_index = j+l_numFieldsPerTime*l_numTimesInThisGroup;
	      if( l_index >= l_allProcNumFields ||
		  !SAF_EQUIV(&(l_allProcFieldTemplates[j]), &(l_allProcFieldTemplates[l_index])) )
		{
		  l_done=1;
		  break;
		}
	    }
	  if( !l_done )
	    {
	      l_numTimesInThisGroup++;
	    }
	}

      if(!g_rank) printinfo("     for state group %d: found %d fields per state, and %d states\n",
			    l_whichStateGroup,l_numFieldsPerTime,l_numTimesInThisGroup );


      if(l_lastNumStates > l_numTimesInThisGroup)
	{
	  if(!g_rank) printinfo("\n\nwarning: 12-10-02 THERE WILL BE A READING BUG BECAUSE # STATES HAS DECREASED SINCE LAST GROUP\n\n\n");
	}
      l_lastNumStates = l_numTimesInThisGroup;

		
      /*create the list of field templates for this state group*/
      l_fieldTemplates = (SAF_FieldTmpl *)malloc(l_numFieldsPerTime*sizeof(SAF_FieldTmpl));
      for(j=0; j<l_numFieldsPerTime; j++ )
	{
	  l_fieldTemplates[j] = l_allProcFieldTemplates[l_startIndex+j];
	}

      snprintf(l_str,127,"state_tmpl_grp%d",l_whichStateGroup);
      saf_declare_state_tmpl(SAF_ALL,g_db,  l_str, l_numFieldsPerTime, l_fieldTemplates, &l_stateTemplate);
	  

      snprintf(l_str,127,"state_grp%d",l_whichStateGroup);
      saf_declare_state_group(SAF_ALL, g_db, l_str, &g_suite, &g_suiteset, &l_stateTemplate, SAF_QTIME, 
			      l_units, MY_SAF_PRECISION, &l_stateGroup);


      /* write the states */
      for(j=0; j<l_numTimesInThisGroup; j++ )
	{
	  int l_startIndexForThisGroup = l_startIndex + j*l_numFieldsPerTime;
	  int l_timestepForThisGroup = l_allProcTimestepPerField[l_startIndexForThisGroup];
	  MY_PRECISION l_timeValueForThisGroup = a_timeValues[l_timestepForThisGroup];

	  if(!g_rank) printinfo("           grp %d, writing state %d of %d, timestep %d, time %f\n",
				l_whichStateGroup, j,l_numTimesInThisGroup,
				l_timestepForThisGroup, l_timeValueForThisGroup );

	  saf_write_state(SAF_ALL,&l_stateGroup,j,&g_suiteset,MY_SAF_PRECISION,
			  &l_timeValueForThisGroup, l_allProcFields+l_startIndexForThisGroup );

	}

      if(l_fieldTemplates) free(l_fieldTemplates);
      l_startIndex += l_numFieldsPerTime*l_numTimesInThisGroup;

      if( l_startIndex>=l_allProcNumFields )
	{
	  if(!g_rank) printinfo("     ****finished making state groups for this suite\n\n");
	  break;
	}
    }
 


#ifdef HAVE_PARALLEL
  free( l_allProcFields );
  free( l_allProcFieldTemplates );
  free( l_allProcTimestepPerField );
#endif

}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Create an Example Structured Mesh File (create_str_mesh)
 * Purpose:     main Function for create_str_mesh
 *
 * Description: 
 * This function parses the command line options and then creates a 
 * test database that exercises the various structured mesh routines defined here.
 *
 * Usage   
 *
 *   ./create_str_mesh [-h] [-v] [-t NUM] [-n] [FILENAME]
 *
 * Command Line Options
 *
 *   -h, --help  print this help message and exit
 *
 *   -t NUM   NUM is an int from 0-4 to specify one of 5 different tweaks
 *            to the created file, for testing safdiff. Default is NUM=0 (no tweaks)
 *
 *   -v, --verbose  output messages to stdout, otherwise output to file saf_reader_stdout.txt
 *            (Note that this only works if compiled with str_mesh_reader.c, which is otherwise not necessary)
 *
 *   -n, --norand   dont use random numbers in data
 *
 *   FILENAME  create/overwrite this saf file, otherwise use file test_str_mesh.saf
 *
 *--------------------------------------------------------------------------------------------------- */
int main(int argc, char **argv)
{
#define MY_MAX_DBNAME_SIZE 1024
  int l_short=0;
  char l_dbname[MY_MAX_DBNAME_SIZE];
  char l_dbnameDefault[]="test_str_mesh.saf";        /* Name of the SAF database file to be created. */
  SAF_DbProps *l_dbprops;      /* Handle to the SAF database properties. */
  char l_str[1024];
  MY_PRECISION *l_fakeData=0;
  MY_PRECISION *l_fakeXCoords=0,*l_fakeYCoords=0,*l_fakeZCoords=0;
  int l_printToFile=1;

#ifdef HAVE_PARALLEL
  /* the MPI_init comes first because on some platforms MPICH's mpirun
   * doesn't pass the same argc, argv to all processors. However, the MPI
   * spec says nothing about what it does or might do to argc or argv. In
   * fact, there is no "const" in the function prototypes for either the
   * pointers or the things they're pointing too.  I would rather pass NULL
   * here and the spec says this is perfectly acceptable.  However, that too
   * has caused MPICH to core on certain platforms. */
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &g_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &g_nprocs);
#endif



  #define OUT_FILENAME "saf_reader_stdout.txt"
  initialize_str_globals();/*must do this before printinfo_set_filename, or the latter will be overwritten*/
  printinfo_set_filename(OUT_FILENAME);
  snprintf(l_dbname,MY_MAX_DBNAME_SIZE-1,"%s",l_dbnameDefault);

  /*PARSE COMMAND LINE*/
  {
    int l_whichArg;
    int l_gotFilename=0,l_printHelpAndExit=0;
    if(!g_rank)
      {
	for(l_whichArg=1; l_whichArg<argc; l_whichArg++)
	  {
	    if( !strcmp(argv[l_whichArg],"-h") ||  !strcmp(argv[l_whichArg],"-vh") ||
		!strcmp(argv[l_whichArg],"-hv") ||
		!strcmp(argv[l_whichArg],"--help") || argc>=6 )
	      {
		l_printHelpAndExit=1;
	      }
	    else if( !strcmp(argv[l_whichArg],"-v") || !strcmp(argv[l_whichArg],"--verbose") )
	      {

		/*set the environment variable: otherwise, printinfo wont print*/
		char *l_envVar = (char *)malloc(256*sizeof(char));
		sprintf(l_envVar,"SAF_READER_DEBUG=true");
#ifndef JANUS
		putenv(l_envVar); 
#endif
		free(l_envVar);

		l_printToFile=0;
	      }
	    else if( !strcmp(argv[l_whichArg],"-sh") || !strcmp(argv[l_whichArg],"--short") )
	      {
		l_short=1;
	      }
	    else if( !strcmp(argv[l_whichArg],"-n") || !strcmp(argv[l_whichArg],"--norand") )
	      {
		g_useRandom=0;
		printinfo("setting g_useRandom=0   RAND_MAX=%d\n",RAND_MAX);
	      }
	    else if( !strcmp(argv[l_whichArg],"-t") )
	      {
		/*OPTION FOR CREATING A TEST FILE WITH A SLIGHT DIFFERENCE
		  VALID VALUES ARE INTEGERS 0-4*/
		l_whichArg++;
		g_diffTestOption=atoi(argv[l_whichArg]);
		printinfo("\nGOT g_diffTestOption=%d\n\n",g_diffTestOption);
	      }
	    else if(!l_gotFilename)
	      {
		l_gotFilename=1;
		snprintf(l_dbname,MY_MAX_DBNAME_SIZE-1,argv[l_whichArg]);
	      }
	    else
	      {
		l_printHelpAndExit=1;
	      }
	    if(l_printHelpAndExit) break;
	  }
      }

#ifdef HAVE_PARALLEL    
    MPI_Bcast(&l_printHelpAndExit, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&l_printToFile, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&l_gotFilename, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&g_useRandom, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&l_short, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(l_gotFilename)
      {
	MPI_Bcast(l_dbname, MY_MAX_DBNAME_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
      }
#endif

    /*printinfo("AFTER COMMAND LINE: pr=%d help=%d tofile=%d gotfile=%d %s\n",
      g_rank,l_printHelpAndExit,l_printToFile,l_gotFilename,l_dbname);*/


    if(l_printHelpAndExit)
      {
	printf("Usage: %s [-h] [-v] [-t NUM] [-n] [FILENAME]\n",argv[0]);
	printf("Create a test SAF structured mesh file.\n\n");
	printf("   -h, --help  print this help message and exit\n");
	printf("   -t NUM   NUM is an int from 0-4 to specify one of 5 different tweaks\n");
	printf("            to the created file, for testing safdiff. Default is NUM=0 (no tweaks)\n");
	printf("   -v, --verbose  output messages to stdout, otherwise output to file %s\n",OUT_FILENAME);
	printf("   -n, --norand   dont use random numbers in data\n");
	printf("   FILENAME  create/overwrite this saf file, otherwise use file %s\n\n",l_dbnameDefault);
	printf("report bugs to saf-help@sourceforge.sandia.gov\n");

	return(-1);
      }
  }

 
  if(!l_printToFile) printinfo_set_filename(NULL);

  if(!g_rank) printinfo("\n%s writing to file %s\n",argv[0],l_dbname);

  if(l_short) g_numTimestepsToWrite = g_numTimestepsToWriteShort;


  setbuf(stdout, NULL);
  setbuf(stderr, NULL);


  /*test safdiff*/
#ifdef HAVE_PARALLEL
  MPI_Bcast(&g_diffTestOption, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
  if(g_diffTestOption==2) g_numTimestepsToWrite++;
  if(g_diffTestOption==3) g_dataBlockAProc0YDim++;
  if(g_diffTestOption==4) g_dataBlockAProc0NodeSubset2.y_count--;
  if(g_diffTestOption==5) g_dataMultiplier=10;

#ifdef TEST_FILE_PATH
  chdir(TEST_FILE_PATH);
#endif

#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif


  /* Initialize the library. */
  saf_init(SAF_DEFAULT_LIBPROPS);


  /* Create the database properties. */
  l_dbprops = saf_createProps_database();

  /* Set the clobber database property so any existing file
   * will be overwritten. */
  saf_setProps_Clobber(l_dbprops);

  /* Open the SAF database. Give it name l_dbname, properties l_dbprops and
   * set g_db to be a handle to this database. */
  g_db = saf_open_database(l_dbname,l_dbprops);

  g_safFile = g_db;



  /****************************************************************************
   ****BEGIN THE STRUCTURED MESH SPECIFIC PART OF MAIN************************
   ***************************************************************************/


  /*
  ** parallel test: make the unst nodes a dummy for one of the procs
  ** note: this is not tested with read_str_mesh, because it is actually
  ** an unstructured mesh thing
  */
  if(g_rank==1) g_dataUnstrNodeCloudNumNodes = 0;



  /*
  ** DECLARE ALL OF THE CATEGORIES THAT MIGHT HAVE VARIABLES MAPPED TO THEM.
  */
  declare_all_structured_categories();


  /*
  ** CREATE THE COORDINATE TEMPLATES THAT WE WILL BE USING.
  */
  saf_declare_field_tmpl(SAF_ALL, g_db, "coords_1D_comp_fld_tmpl", 
			 SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QLENGTH, 1,
			 NULL, g_coordsIndivFieldTemplates );
  g_coordsIndivFieldTemplates[1] = g_coordsIndivFieldTemplates[0];
  g_coordsIndivFieldTemplates[2] = g_coordsIndivFieldTemplates[0];
  saf_declare_field_tmpl(SAF_ALL, g_db, "coords_fld_tmpl",  
			 SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
			 g_coordsIndivFieldTemplates, &g_coordsFieldTemplate);

  /*
  ** CREATE BLOCK A
  */
#if 1
  {
    SAF_Set *l_blockSet=NULL, *l_subSet=NULL;
    SAF_FieldTmpl *l_varFieldTemplate=NULL;
    int l_timestep=0;
    int l_blockXDim,l_blockYDim,l_blockZDim;
    struct_hyperslab l_nodeSubset1,l_nodeSubset2;

  
    if(g_rank==0)
      {
	l_blockXDim = g_dataBlockAProc0XDim;
	l_blockYDim = g_dataBlockAProc0YDim;
	l_blockZDim = g_dataBlockAProc0ZDim;
	l_nodeSubset1 = g_dataBlockAProc0NodeSubset1;
	l_nodeSubset2 = g_dataBlockAProc0NodeSubset2;
      }
    else if(g_rank==1)
      {
	l_blockXDim = g_dataBlockAProc1XDim;
	l_blockYDim = g_dataBlockAProc1YDim;
	l_blockZDim = g_dataBlockAProc1ZDim;
	l_nodeSubset1 = g_dataBlockAProc1NodeSubset1;
	l_nodeSubset2 = g_dataBlockAProc1NodeSubset2;
      }
    else
      {
	l_blockXDim = g_dataBlockAProc2XDim;
	l_blockYDim = g_dataBlockAProc2YDim;
	l_blockZDim = g_dataBlockAProc2ZDim;
	l_nodeSubset1 = g_dataBlockAProc2NodeSubset1;
	l_nodeSubset2 = g_dataBlockAProc2NodeSubset2;
      }






    /*write the block */
    create_fake_data_coords( l_blockXDim,l_blockYDim,l_blockZDim,
			    &l_fakeXCoords,&l_fakeYCoords,&l_fakeZCoords );
    sprintf(l_str,"block_A_pr%d",g_rank);


    l_blockSet = add_separable_block( l_str,"meter",
				      l_blockXDim,l_blockYDim,l_blockZDim, 
				      l_fakeXCoords,l_fakeYCoords,l_fakeZCoords );



    destroy_fake_data(&l_fakeXCoords);
    destroy_fake_data(&l_fakeYCoords);
    destroy_fake_data(&l_fakeZCoords);


#if 1
    /*write a per-node var on the entire block*/
    /*THIS VAR SHOULD APPEAR ON ALL BLOCKS, ALL PROCS*/
    for( l_timestep=0; l_timestep<g_numTimestepsToWrite; l_timestep++ )
      {
	create_fake_data_var( l_blockXDim,l_blockYDim,l_blockZDim, &l_fakeData, l_timestep,0 );
	if(!l_timestep)
	  {
	    l_varFieldTemplate = write_node_variable(l_blockSet, "mass", "kilogram", l_fakeData, l_timestep );
	  }
	else
	  {
	    write_node_variable_using_existing_template(l_blockSet, l_varFieldTemplate, "mass", "kilogram", l_fakeData, l_timestep );
	  }
      }
    destroy_fake_data(&l_fakeData);/*can have many create_'s for 1 destroy_, iff all same size*/
    if(l_varFieldTemplate) 
      {
	free(l_varFieldTemplate);
	l_varFieldTemplate=0;
      }
#endif


#if 1
    /*write a per-elem var on the entire block*/
    for( l_timestep=0; l_timestep<g_numTimestepsToWrite; l_timestep++ )
      {
	create_fake_data_var( l_blockXDim-1,l_blockYDim-1,l_blockZDim-1, &l_fakeData, l_timestep,1 );
	if(!l_timestep)
	  {
	    l_varFieldTemplate = write_elem_variable(l_blockSet, "electric current", "ampere", l_fakeData, l_timestep );
	  }
	else
	  {
	    write_elem_variable_using_existing_template(l_blockSet,l_varFieldTemplate,"electric current","ampere",l_fakeData,l_timestep);
	  }
      }
    destroy_fake_data(&l_fakeData);/*can have many create_'s for 1 destroy_, iff all same size*/
    if(l_varFieldTemplate) 
      {
	free(l_varFieldTemplate);
	l_varFieldTemplate=0;
      }
#endif


#if 1
    /*create subsets of the x,y,&z edges & faces with a large do-list, and write vars*/
    {
      SAF_Set *l_xEdgeSet=0,*l_yEdgeSet=0,*l_zEdgeSet=0;
      SAF_Set *l_xFaceSet=0,*l_yFaceSet=0,*l_zFaceSet=0;
      SAF_FieldTmpl *l_xedgeTemplate=0,*l_yedgeTemplate=0,*l_zedgeTemplate=0;
      SAF_FieldTmpl *l_xfaceTemplate=0,*l_yfaceTemplate=0,*l_zfaceTemplate=0;
      for( l_timestep=0; l_timestep<g_numTimestepsToWrite; l_timestep++ )
	{
	  create_fake_data_var( l_nodeSubset2.x_count,l_nodeSubset2.y_count,
			     l_nodeSubset2.z_count, &l_fakeData, l_timestep,0 );
	  if(!l_timestep )
	    {
	      l_xEdgeSet = create_x_edge_subset(l_blockSet, l_nodeSubset2 );
	      l_xedgeTemplate = write_x_edge_variable(l_xEdgeSet, "force", "newton", l_fakeData, l_timestep );
	      l_yEdgeSet = create_y_edge_subset(l_blockSet, l_nodeSubset2 );
	      l_yedgeTemplate = write_y_edge_variable(l_yEdgeSet, "force", "newton", l_fakeData, l_timestep );
	      l_zEdgeSet = create_z_edge_subset(l_blockSet, l_nodeSubset2 );
	      l_zedgeTemplate = write_z_edge_variable(l_zEdgeSet, "force", "newton", l_fakeData, l_timestep );

	      l_xFaceSet = create_x_face_subset(l_blockSet, l_nodeSubset2 );
	      l_xfaceTemplate = write_x_face_variable(l_xFaceSet, "luminous intensity", "candela", l_fakeData, l_timestep );
	      l_yFaceSet = create_y_face_subset(l_blockSet, l_nodeSubset2 );
	      l_yfaceTemplate = write_y_face_variable(l_yFaceSet, "luminous intensity", "candela", l_fakeData, l_timestep );
	      l_zFaceSet = create_z_face_subset(l_blockSet, l_nodeSubset2 );
	      l_zfaceTemplate = write_z_face_variable(l_zFaceSet, "luminous intensity", "candela", l_fakeData, l_timestep );
	    }
	  else
	    {
	      write_x_edge_variable_using_existing_template(l_xEdgeSet, l_xedgeTemplate, "force", "newton", l_fakeData, l_timestep );
	      write_y_edge_variable_using_existing_template(l_yEdgeSet, l_yedgeTemplate, "force", "newton", l_fakeData, l_timestep );
	      write_z_edge_variable_using_existing_template(l_zEdgeSet, l_zedgeTemplate, "force", "newton", l_fakeData, l_timestep );

	      write_x_face_variable_using_existing_template(l_xFaceSet,l_xfaceTemplate,"luminous intensity","candela",l_fakeData,l_timestep);
	      write_y_face_variable_using_existing_template(l_yFaceSet,l_yfaceTemplate,"luminous intensity","candela",l_fakeData,l_timestep);
	      write_z_face_variable_using_existing_template(l_zFaceSet,l_zfaceTemplate,"luminous intensity","candela",l_fakeData,l_timestep);
	    }
	  destroy_fake_data(&l_fakeData);
	}
      if(l_xEdgeSet) free(l_xEdgeSet);
      if(l_yEdgeSet) free(l_yEdgeSet);
      if(l_zEdgeSet) free(l_zEdgeSet);
      if(l_xFaceSet) free(l_xFaceSet);
      if(l_yFaceSet) free(l_yFaceSet);
      if(l_zFaceSet) free(l_zFaceSet);
      if(l_xedgeTemplate) free(l_xedgeTemplate);
      if(l_yedgeTemplate) free(l_yedgeTemplate);
      if(l_zedgeTemplate) free(l_zedgeTemplate);
      if(l_xfaceTemplate) free(l_xfaceTemplate);
      if(l_yfaceTemplate) free(l_yfaceTemplate);
      if(l_zfaceTemplate) free(l_zfaceTemplate);
    }
#endif


#if 1
    l_timestep=0;

    /*write a var for every x,y,&z pointing edge*/
    create_fake_data_var( l_blockXDim-1, l_blockYDim, l_blockZDim, &l_fakeData, l_timestep,0 );
    l_varFieldTemplate = write_x_edge_variable(l_blockSet,"energy","joule",l_fakeData,l_timestep);
    free(l_varFieldTemplate);
    destroy_fake_data(&l_fakeData);

    create_fake_data_var( l_blockXDim, l_blockYDim-1, l_blockZDim, &l_fakeData, l_timestep,0 );
    l_varFieldTemplate = write_y_edge_variable(l_blockSet,"energy","joule",l_fakeData,l_timestep);
    free(l_varFieldTemplate);
    destroy_fake_data(&l_fakeData);

    create_fake_data_var(  l_blockXDim, l_blockYDim, l_blockZDim-1, &l_fakeData, l_timestep,0 );
    l_varFieldTemplate = write_z_edge_variable(l_blockSet,"energy","joule",l_fakeData,l_timestep);
    free(l_varFieldTemplate);
    destroy_fake_data(&l_fakeData);


    /*write a var for every x,y,&z normaled face*/
    create_fake_data_var( l_blockXDim, l_blockYDim-1, l_blockZDim-1, &l_fakeData, l_timestep,0 );
    l_varFieldTemplate = write_x_face_variable(l_blockSet,"frequency","hertz",l_fakeData,l_timestep);
    free(l_varFieldTemplate);
    destroy_fake_data(&l_fakeData);

    create_fake_data_var( l_blockXDim-1, l_blockYDim, l_blockZDim-1, &l_fakeData, l_timestep,0 );
    l_varFieldTemplate = write_y_face_variable(l_blockSet,"frequency","hertz",l_fakeData,l_timestep);
    free(l_varFieldTemplate);
    destroy_fake_data(&l_fakeData);

    create_fake_data_var( l_blockXDim-1, l_blockYDim-1, l_blockZDim, &l_fakeData, l_timestep,0 );
    l_varFieldTemplate = write_z_face_variable(l_blockSet,"frequency","hertz",l_fakeData,l_timestep);
    free(l_varFieldTemplate);
    destroy_fake_data(&l_fakeData);
#endif


#if 1
    /*create a subset and write two per-node vars on it*/
    {
      l_timestep=0;
      l_subSet = create_node_subset(l_blockSet, l_nodeSubset1 );
       
      create_fake_data_var( l_nodeSubset1.x_count,l_nodeSubset1.y_count,
			 l_nodeSubset1.z_count, &l_fakeData, l_timestep,0 );
      l_varFieldTemplate = write_node_variable(l_subSet, "temperature", "kelvin", l_fakeData, l_timestep );
      free(l_varFieldTemplate);
      l_varFieldTemplate = write_node_variable(l_subSet, "pressure", "pascal", 
					       l_fakeData, l_timestep );
      destroy_fake_data(&l_fakeData);

      /*add new timesteps of a scalar variable */
      for( l_timestep=1; l_timestep<((g_numTimestepsToWrite-2>5) ? 5:g_numTimestepsToWrite-2); l_timestep++ )
	{
	  create_fake_data_var( l_nodeSubset1.x_count,l_nodeSubset1.y_count,
			     l_nodeSubset1.z_count, &l_fakeData, l_timestep,0 );
	  write_node_variable_using_existing_template(l_subSet, l_varFieldTemplate, "pressure", "pascal",  
						      l_fakeData, l_timestep );
	  destroy_fake_data(&l_fakeData);
	}

      if(l_varFieldTemplate) 
	{
	  free(l_varFieldTemplate);
	  l_varFieldTemplate=0;
	}
      if(l_subSet) 
	{
	  free(l_subSet);
	  l_subSet=0;
	}
    }
#endif


    if(l_blockSet) free(l_blockSet);
  }
#endif








  /*
  ** CREATE BLOCK B
  */
#if 1
  {
    SAF_Set *l_blockSet=NULL,*l_hexSet=NULL;
    int l_timestep=0;
    SAF_FieldTmpl *l_varFieldTemplate=NULL;
    int l_blockXDim,l_blockYDim,l_blockZDim;
    struct_hyperslab l_nodeSubset,l_elemSubset;


    if(g_rank==0)
      {
	l_blockXDim = g_dataBlockBProc0XDim;
	l_blockYDim = g_dataBlockBProc0YDim;
	l_blockZDim = g_dataBlockBProc0ZDim;
	l_elemSubset = g_dataBlockBProc0ElemSubset;
	l_nodeSubset = g_dataBlockBProc0NodeSubset;
      }
    else if(g_rank==1)
      {
	l_blockXDim = g_dataBlockBProc1XDim;
	l_blockYDim = g_dataBlockBProc1YDim;
	l_blockZDim = g_dataBlockBProc1ZDim;
	l_elemSubset = g_dataBlockBProc1ElemSubset;
	l_nodeSubset = g_dataBlockBProc1NodeSubset;
      }
    else
      {
	l_blockXDim = g_dataBlockBProc2XDim;
	l_blockYDim = g_dataBlockBProc2YDim;
	l_blockZDim = g_dataBlockBProc2ZDim;
	l_elemSubset = g_dataBlockBProc2ElemSubset;
	l_nodeSubset = g_dataBlockBProc2NodeSubset;
      }



    /*write the block */
    create_fake_data_coords( l_blockXDim,l_blockYDim,l_blockZDim,
			    &l_fakeXCoords,&l_fakeYCoords,&l_fakeZCoords );
    sprintf(l_str,"block_B_pr%d",g_rank);

    {
      /*this will be a curvilinear block, but we currently have only separable data:
	create a curvilinear array, and add a random amount to each value*/
      MY_PRECISION *l_xBuf=0,*l_yBuf=0,*l_zBuf=0;
      int l_count,i,j,k;
      int l_numNodes = l_blockXDim*l_blockYDim*l_blockZDim;
      if(l_numNodes)
	{
	  l_xBuf = (MY_PRECISION *)malloc( l_numNodes*sizeof(MY_PRECISION) );
	  l_yBuf = (MY_PRECISION *)malloc( l_numNodes*sizeof(MY_PRECISION) );
	  l_zBuf = (MY_PRECISION *)malloc( l_numNodes*sizeof(MY_PRECISION) );
	  if(!l_xBuf || !l_yBuf || !l_zBuf)
	    {
	      printinfo("error: create_fake_data_coords malloc error creating curvilinear block 2\n");
	      exit(-1);
	    }
	}
      l_count=0;


  /*Use the same sequence of random numbers, so that each database created
    (with the same options) will be the same. The random numbers are helpful
    for testing the visualization tool.*/
  srand(123);

      for(k=0; k<l_blockZDim; k++)
	{
	  for(j=0; j<l_blockYDim; j++)
	    {
	      for(i=0; i<l_blockXDim; i++ )
		{
		  MY_PRECISION l_rand1=my_rand();
		  MY_PRECISION l_rand2=my_rand();
		  MY_PRECISION l_rand3=my_rand();
		  l_xBuf[l_count]=l_fakeXCoords[i] + .25 * ( l_rand1 - 0.5);
		  l_yBuf[l_count]=l_fakeYCoords[j] + .25 * ( l_rand2 - 0.5);
		  l_zBuf[l_count]=l_fakeZCoords[k] + .25 * ( l_rand3 - 0.5);
		  l_count++;
		}
	    }
	}

      l_blockSet = add_curvilinear_block( l_str,"meter",
					  l_blockXDim,l_blockYDim,l_blockZDim, 
					  l_xBuf, l_yBuf, l_zBuf );

      if(l_xBuf) free(l_xBuf);
      if(l_yBuf) free(l_yBuf);
      if(l_zBuf) free(l_zBuf);
    }


    destroy_fake_data(&l_fakeXCoords);
    destroy_fake_data(&l_fakeYCoords);
    destroy_fake_data(&l_fakeZCoords);



#if 1
    /*write a per-node var on the entire block*/
    /*THIS VAR SHOULD APPEAR ON ALL BLOCKS, ALL PROCS*/
    for( l_timestep=0; l_timestep<g_numTimestepsToWrite; l_timestep++ )
      {
	create_fake_data_var( l_blockXDim,l_blockYDim,l_blockZDim, &l_fakeData, l_timestep,0 );
	if(!l_timestep)
	  {
	    l_varFieldTemplate = write_node_variable(l_blockSet, "mass", "kilogram", l_fakeData, l_timestep );
	  }
	else
	  {
	    write_node_variable_using_existing_template(l_blockSet, l_varFieldTemplate, "mass", "kilogram", l_fakeData, l_timestep );
	  }
      }
    destroy_fake_data(&l_fakeData);/*can have many create_'s for 1 destroy_, iff all same size*/
    if(l_varFieldTemplate) 
      {
	free(l_varFieldTemplate);
	l_varFieldTemplate=0;
      }
#endif



#if 1
    /*write a per-hex var on the entire block*/
    for( l_timestep=0; l_timestep<g_numTimestepsToWrite; l_timestep++ )
      {
	create_fake_data_var( l_blockXDim-1,l_blockYDim-1,l_blockZDim-1, &l_fakeData, l_timestep,1 );
	if(!l_timestep)
	  {
	    l_varFieldTemplate = write_elem_variable(l_blockSet, "electric current", "ampere", l_fakeData, l_timestep );
	  }
	else
	  {
	    write_elem_variable_using_existing_template(l_blockSet, l_varFieldTemplate, 
							"electric current", "ampere", l_fakeData, l_timestep );
	  }
      }
    if(l_varFieldTemplate) 
      {
	free(l_varFieldTemplate);
	l_varFieldTemplate=0;
      }
    destroy_fake_data(&l_fakeData);/*can have many create_'s for 1 destroy_, iff all same size*/
#endif


#if 1
    /*Create a subset of the hexes with a do-list, and write a per-hex var on it*/
    {
      l_timestep=0;
      l_hexSet = create_elem_subset(l_blockSet, l_elemSubset );
      create_fake_data_var( l_elemSubset.x_count,l_elemSubset.y_count,
			 l_elemSubset.z_count, &l_fakeData, l_timestep,0 );
      l_varFieldTemplate = write_elem_variable(l_hexSet, "length", "mile", l_fakeData, l_timestep );

      l_timestep++;
      create_fake_data_var( l_elemSubset.x_count,l_elemSubset.y_count,
			 l_elemSubset.z_count, &l_fakeData, l_timestep,0 );
      write_elem_variable_using_existing_template(l_hexSet, l_varFieldTemplate, "length", "mile", l_fakeData, l_timestep );
      free(l_varFieldTemplate);
      free(l_hexSet);
      destroy_fake_data(&l_fakeData);
    }

#endif


#if 1
    /*create subsets of the x,y,&z edges & faces with a large do-list, and write vars*/
    {
      SAF_Set *l_xEdgeSet=0,*l_yEdgeSet=0,*l_zEdgeSet=0;
      SAF_Set *l_xFaceSet=0,*l_yFaceSet=0,*l_zFaceSet=0;
      SAF_FieldTmpl *l_xedgeTemplate=0,*l_yedgeTemplate=0,*l_zedgeTemplate=0;
      SAF_FieldTmpl *l_xfaceTemplate=0,*l_yfaceTemplate=0,*l_zfaceTemplate=0;
      for( l_timestep=0; l_timestep<((g_numTimestepsToWrite-2>5) ? 5:g_numTimestepsToWrite-2); l_timestep++ )
	{
	  create_fake_data_var( l_nodeSubset.x_count,l_nodeSubset.y_count,
			     l_nodeSubset.z_count, &l_fakeData, l_timestep,0 );


	  if(g_diffTestOption==1) { if(l_timestep==1 && g_rank==0) l_fakeData[0] += .001; }/*test safdiff*/



	  if(!l_timestep )
	    {
	      l_xEdgeSet = create_x_edge_subset(l_blockSet, l_nodeSubset );
	      l_xedgeTemplate = write_x_edge_variable(l_xEdgeSet, "energy", "joule", l_fakeData, l_timestep );
	      l_yEdgeSet = create_y_edge_subset(l_blockSet, l_nodeSubset );
	      l_yedgeTemplate = write_y_edge_variable(l_yEdgeSet, "energy", "joule", l_fakeData, l_timestep );
	      l_zEdgeSet = create_z_edge_subset(l_blockSet, l_nodeSubset );
	      l_zedgeTemplate = write_z_edge_variable(l_zEdgeSet, "energy", "joule", l_fakeData, l_timestep );

	      l_xFaceSet = create_x_face_subset(l_blockSet, l_nodeSubset );
	      l_xfaceTemplate = write_x_face_variable(l_xFaceSet, "frequency", "hertz", l_fakeData, l_timestep );
	      l_yFaceSet = create_y_face_subset(l_blockSet, l_nodeSubset );
	      l_yfaceTemplate = write_y_face_variable(l_yFaceSet, "frequency", "hertz", l_fakeData, l_timestep );
	      l_zFaceSet = create_z_face_subset(l_blockSet, l_nodeSubset );
	      l_zfaceTemplate = write_z_face_variable(l_zFaceSet, "frequency", "hertz", l_fakeData, l_timestep );
	    }
	  else
	    {
	      write_x_edge_variable_using_existing_template(l_xEdgeSet, l_xedgeTemplate, "energy", 
							    "joule", l_fakeData, l_timestep );
	      write_y_edge_variable_using_existing_template(l_yEdgeSet, l_yedgeTemplate, "energy", 
							    "joule", l_fakeData, l_timestep );
	      write_z_edge_variable_using_existing_template(l_zEdgeSet, l_zedgeTemplate, "energy", 
							    "joule", l_fakeData, l_timestep );

	      write_x_face_variable_using_existing_template(l_xFaceSet, l_xfaceTemplate, "frequency", 
							    "hertz", l_fakeData, l_timestep );
	      write_y_face_variable_using_existing_template(l_yFaceSet, l_yfaceTemplate, "frequency", 
							    "hertz", l_fakeData, l_timestep );
	      write_z_face_variable_using_existing_template(l_zFaceSet, l_zfaceTemplate, "frequency", 
							    "hertz", l_fakeData, l_timestep );
	    }
	  destroy_fake_data(&l_fakeData);
	}
      if(l_xEdgeSet) free(l_xEdgeSet);
      if(l_yEdgeSet) free(l_yEdgeSet);
      if(l_zEdgeSet) free(l_zEdgeSet);
      if(l_xFaceSet) free(l_xFaceSet);
      if(l_yFaceSet) free(l_yFaceSet);
      if(l_zFaceSet) free(l_zFaceSet);
      if(l_xedgeTemplate) free(l_xedgeTemplate);
      if(l_yedgeTemplate) free(l_yedgeTemplate);
      if(l_zedgeTemplate) free(l_zedgeTemplate);
      if(l_xfaceTemplate) free(l_xfaceTemplate);
      if(l_yfaceTemplate) free(l_yfaceTemplate);
      if(l_zfaceTemplate) free(l_zfaceTemplate);
    }
#endif

printf("Not putting string annotations yet in sslib upgrade.\n");/*hooey*/
#if 0
    /*Write some string annotations and attach them to this block*/
    {
      char l_testStr[]="String 1 on block B";
      char l_testStr2[]="String 2 on block B";
      printinfo("attaching strings to block B\n");

      saf_put_set_att(SAF_EACH, l_blockSet, "att_str1", H5T_STRING, 1, (void *)l_testStr);
      saf_put_set_att(SAF_EACH, l_blockSet, "att_str2", H5T_STRING, 1, (void *)l_testStr2);
    }
#endif


printf("Not putting float annotations yet in sslib upgrade.\n");/*hooey*/
#if 0
    /*Write some float annotations and attach them to this block*/
    {
      float l_test1=123.456;
      float l_test2[]={ 789.987, 654.3210 };

      printinfo("attaching floats to block B\n");

      saf_put_set_att(SAF_EACH, l_blockSet, "att_float1", SAF_FLOAT, 1, &l_test1);
      saf_put_set_att(SAF_EACH, l_blockSet, "att_float2", SAF_FLOAT, 2, l_test2);
    }
#endif



    if(l_blockSet) free(l_blockSet);

  }
#endif


printf("Not putting string annotations yet in sslib upgrade.\n");/*hooey*/
#if 0
  /* write a string annotation and attach it to the entire data set*/
  {
    const char l_testStr[]="String attached to entire db.";
    /*saf_putAttribute(g_db,g_db,SAF_ALL, "att_str_on_set", H5T_STRING, 1, l_testStr);*/
    saf_put_attribute(SAF_ALL,(ss_pers_t *)g_db,"att_str_on_set", H5T_STRING, 1, l_testStr);
  }
#endif


#if 1
  /* write integers annotations and attach them to the entire data set*/
  {
    int l_testInt[]={ 123456, 789 };
    /*saf_putAttribute(g_db,g_db,SAF_ALL, "att_int_on_set", SAF_INT, 2, l_testInt);*/
    saf_put_attribute(SAF_ALL,(ss_pers_t *)g_db,"att_int_on_set", SAF_INT, 2, l_testInt);
  }

  /* write float annotations and attach them to the entire data set*/
  {
    float l_testFloat[]={ 1.1, 2.2, 3.3 };
    /*saf_putAttribute(g_db,g_db,SAF_ALL, "att_float_on_set", SAF_FLOAT, 3, l_testFloat);*/
    saf_put_attribute(SAF_ALL,(ss_pers_t *)g_db,"att_float_on_set", SAF_FLOAT, 3, l_testFloat);
  }
#endif


  /* add sets of unattached nodes */
  {
    int l_timestep=0;
    SAF_Set *l_set=0;
    MY_PRECISION *l_fakeIt=0;
    int l_numNodes = g_dataUnstrNodeCloudNumNodes;

    if(!g_rank) printinfo("Adding initial node cloud, num nodes=%d\n",l_numNodes);

    create_fake_data_node_cloud( l_numNodes*3, &l_fakeIt, l_timestep );
    sprintf(l_str,"node_cloud_pr%d",g_rank);
    l_set = add_unstructured_nodeset( l_str, "meter",l_timestep,l_numNodes,l_fakeIt);
    destroy_fake_data(&l_fakeIt);

    for(l_timestep=1;l_timestep<g_numTimestepsToWrite;l_timestep++)
      {
#if 0
  /*XXX jake todo:
    THIS IS NOT RIGHT YET. SHOULDNT RE-DECLARE A COLLECTION ON THE SAME SET.
    COULD MAKE IT EXTENDIBLE, BUT THEN THERE IS THE PROBLEM WITH SAF_EACH,
    PLUS THE PROBLEM ABOUT THE NUMBER OF NODES DECREASING. COULD ALSO JUST
    CREATE NEW CATS, BUT THIS IS UGLY AND DEFEATS THE PURPOSE*/
  /*I THOUGHT I ALREADY FIXED THIS, DID I LOSE THE CODE SOMEWHERE?*/
	l_numNodes = g_dataUnstrNodeCloudNumNodes+2*l_timestep;
#endif

	if(!g_rank) printinfo("Adding node cloud for time %d of %d, num nodes=%d\n",l_timestep,g_numTimestepsToWrite,l_numNodes);

	create_fake_data_node_cloud( l_numNodes*3, &l_fakeIt, l_timestep );
	add_coords_to_unstructured_nodeset( *l_set, "meter",l_timestep,l_numNodes,l_fakeIt);
	destroy_fake_data(&l_fakeIt);
      }


    free(l_set);
  }



  /*
   *AFTER ALL SETS AND FIELDS ARE WRITTEN, CREATE THE TIME SUITES
   */
  if( g_numTimes > 0 )
    {
      int i;
      MY_PRECISION *l_timeValues = (MY_PRECISION *)malloc(g_numTimes*sizeof(MY_PRECISION));
      for(i=0;i<g_numTimes;i++) l_timeValues[i]=i*.01;/*making up time values*/
      write_timestep_data(l_timeValues);
      free(l_timeValues);
    }



  /****************************************************************************
   ****END OF THE STRUCTURED MESH SPECIFIC PART OF MAIN************************
   ***************************************************************************/






  /* Close the SAF database. */
  saf_close_database(g_db);


  /* Finalize access to the library. */
  saf_final();



#ifdef HAVE_PARALLEL
  MPI_Finalize();
#endif



  /*free mallocs*/

  if(g_blocksAndUnstrNodesetsAdded) 
    {
      free(g_blocksAndUnstrNodesetsAdded);
      g_blocksAndUnstrNodesetsAdded=0;
      g_numBlocksAdded=0;
    }
  if(g_fields) 
    {
      free(g_fields);
      g_fields=0;
    }
  if(g_fieldTemplates) 
    {
      free(g_fieldTemplates);
      g_fieldTemplates=0;
    }
  if(g_timestepsPerAddedField) 
    {
      free(g_timestepsPerAddedField);
      g_timestepsPerAddedField=0;
    }

  printinfo("proc %d of %d exiting with no errors\n",g_rank,g_nprocs);

  return(0);

}
