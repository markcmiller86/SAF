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
/*==================================*/
/* (API 2.0) USERD DSO Routines     */
/*                                  */
/* 2.01  has MAXTYPE changes,       */
/*       and part number references */
/*       have been made consistent  */
/*==================================*/
/*-------------------------------------------------------------------------
Routine Index:

--------------------------------------
Generally Needed for UNSTRUCTURED data
(Can be dummys if only block data)
--------------------------------------
USERD_get_part_coords                         part's node coordinates
USERD_get_part_node_ids                       part's node ids
USERD_get_part_elements_by_type               part's element connectivites
USERD_get_part_element_ids_by_type            part's element ids

-----------------------------------------
Generally Needed for BLOCK data
(Can be dummys if only unstructured data)
-----------------------------------------
USERD_get_block_coords_by_component           block coordinates
USERD_get_block_iblanking                     block iblanking values
USERD_get_ghosts_in_block_flag                block ghost cell existence?
USERD_get_block_ghost_flags                   block ghost cell flags

---------------------------
Generally needed for either
or both kinds of data
---------------------------
USERD_get_name_of_reader                      name of reader for GUI
USERD_get_reader_release                      release string of reader
USERD_get_reader_descrip                      description of reader for GUI
USERD_get_reader_version                      provide reader API version number

USERD_set_filenames                           filenames entered in GUI
USERD_set_server_number                       server which of how many

USERD_get_number_of_timesets                  number of timesets
USERD_get_timeset_description                 description of timeset
USERD_get_geom_timeset_number                 timeset # to use for geom

USERD_get_num_of_time_steps                   number of time steps
USERD_get_sol_times                           solution time values
USERD_set_time_set_and_step                   current timeset and time step

USERD_get_changing_geometry_status            changing geometry?
USERD_get_node_label_status                   node labels?
USERD_get_element_label_status                element labels?
USERD_get_model_extents                       provide model bounding extents
USERD_get_number_of_files_in_dataset          number of files in model
USERD_get_dataset_query_file_info             info about each model file
USERD_get_descrip_lines                       file associated descrip lines
USERD_get_number_of_model_parts               number of model parts
USERD_get_gold_part_build_info                part or block type/descrip etc.
USERD_get_maxsize_info                        part/block allocation maximums
USERD_get_ghosts_in_model_flag                model contains ghost cells?

USERD_get_border_availability                 part border provided?
USERD_get_border_elements_by_type             part border conn and parent info

USERD_get_number_of_variables                 number of variables
USERD_get_gold_variable_info                  variable type/descrip etc.
USERD_get_var_by_component                    part or block variable values
USERD_get_constant_val                        constant variable's value
USERD_get_var_value_at_specific               node's or element's variable
                                                 value over time
USERD_stop_part_building                      cleanup after part build routine

USERD_bkup                                    archive routine

USERD_exit_routine                            cleanup upon exit routine

*---------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/*---------------------------------------------------------------------
 * NOTE: Unless explicitly stated otherwise, all arrays are zero based
 *       In true C fashion.
 *---------------------------------------------------------------------*/

#include "global_extern.h"          /* Any files containing these routines */
                                    /* should include this header file     */


#include "str_mesh_reader.h"
#include "unstr_mesh_reader.h"

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif


#define LARRY1W_TEMP_FIX

#if 0
  #define FAKE_A_VAR_IF_THERE_ARE_NONE /*to fix an unexplained ensight bug?*/
#endif


#define USE_SOS_FOR_SAF
#ifdef USE_SOS_FOR_SAF
static int *g_objectIsValidInSOS=NULL;
#endif


/* Typical Global Variables */      /* You will likely have several more */
/*--------------------------*/

static int Num_dataset_files           = 0;
static int Numparts_available          = 0;
static int Num_variables               = 0;

static int Num_timesets                = 1;/*jake: look into these later*/
static int Current_timeset             = 1;
static int Geom_timeset_number         = 1;

static int Num_time_steps[Z_MAX_SETS]  = {0,0,0,0,0,0,0,0,0};/*on and on Z_MAX_SETS=300*/
static int Current_time_step           = 0;

static int Server_Number               = 1;
static int Tot_Servers                 = 1;







/* 
   Ensight doesnt seem to keep any data around, so I should do it.
   Note: jun 19 2003: currently have this commented out because, for
   example, the dataset jew05_blast_083002dyn.saf requires at least
   (it dies on my linux box) 817,634,320 bytes memory 
*/
/*#define RETAIN_COORDS_IN_MEMORY*/
#ifdef RETAIN_COORDS_IN_MEMORY
  float ***g_xCoordsPerPartPerTimestep=0;
  float ***g_yCoordsPerPartPerTimestep=0;
  float ***g_zCoordsPerPartPerTimestep=0;
  int **g_nodesPerPartPerTimestep=0;
  int g_bytesCostToRetainInfo=0;
#endif

const char *get_z_celltype_desc( int a_type );
const char *get_z_celltype_desc( int a_type )
{
	if(a_type==Z_POINT) return("Z_POINT");
	else if(a_type==Z_BAR02) return("Z_BAR02");
	else if(a_type==Z_BAR03) return("Z_BAR03");
	else if(a_type==Z_TRI03) return("Z_TRI03");
	else if(a_type==Z_TRI06) return("Z_TRI06");
	else if(a_type==Z_QUA04) return("Z_QUA04");
	else if(a_type==Z_QUA08) return("Z_QUA08");
	else if(a_type==Z_TET04) return("Z_TET04");
	else if(a_type==Z_TET10) return("Z_TET10");
	else if(a_type==Z_PYR05) return("Z_PYR05");
	else if(a_type==Z_PYR13) return("Z_PYR13");
	else if(a_type==Z_PEN06) return("Z_PEN06");
	else if(a_type==Z_PEN15) return("Z_PEN15");
	else if(a_type==Z_HEX08) return("Z_HEX08");
	else if(a_type==Z_HEX20) return("Z_HEX20");
	return("(unknown?)");
}


/*-----------------------------
 * UNSTRUCTURED DATA ROUTINES: 
 *-----------------------------*/


/*--------------------------------------------------------------------
 * USERD_get_part_coords -
 *--------------------------------------------------------------------
 *
 *   Get the coordinates for an unstructured part.
 *
 *  (IN)  part_number             = The part number
 *
 *                                  (1-based index of part table, namely:
 * 
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *  (OUT) coord_array             = 2D float array which contains
 *                                  x,y,z coordinates of each node.
 *
 *       (IMPORTANT: the second dimension of of this array is 1-based!!!)
 *
 *                                  (Array will have been allocated
 *                                   3 by (number_of_nodes + 1) for the part
 *                                   long - see USERD_get_gold_part_build_info)
 *
 *                       ex) If number_of_nodes = 100
 *                           as obtained in:
 *                             USERD_get_gold_part_build_info
 *
 *                           Then the allocated dimensions of the
 *                           pointer sent to this routine will be:
 *                             coord_array[3][101]
 *
 *                           Ignore the coord_array[0][0]
 *                                      coord_array[1][0]
 *                                      coord_array[2][0] locations and start
 *                           the node coordinates at:
 *                             coord_array[0][1]
 *                             coord_array[1][1]
 *                             coord_array[2][1]
 *
 *                             coord_array[0][2]
 *                             coord_array[1][2]
 *                             coord_array[2][2]
 *
 *                                   etc.
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * This will be based on Current_time_step
 *
 *  * Not called unless number_of_nodes for the part > 0
 *--------------------------------------------------------------------*/

int
USERD_get_part_coords(int part_number,
                      float **coord_array)
{
  float *l_origArray=0,*l_ptr;
  int l_numNodes=0,i;
  int l_whichUnstrPart=part_number-1-get_num_blocks();




#ifdef USE_SOS_FOR_SAF
  /*This function is definitely called, even if the object was given no nodes and no
   elems on this processor.*/
  if(g_objectIsValidInSOS && !g_objectIsValidInSOS[part_number-1])
    {
      /*printinfo("SKIPPING USERD_get_part_coords FOR SERVER # %d  part_number=%d\n",
	     Server_Number, part_number);*/
      return(Z_OK);
    }
#endif





  printinfo("saf reader entering USERD_get_part_coords\n");

  if(l_whichUnstrPart<0 || l_whichUnstrPart>=get_num_unstr_parts())
    {
      printinfo("error in USERD_get_part_coords, l_whichUnstrPart out of range\n");
      return(Z_ERR);
    }

#ifdef RETAIN_COORDS_IN_MEMORY
  if( g_xCoordsPerPartPerTimestep )
    {
      if( g_xCoordsPerPartPerTimestep[Current_time_step] )
	{
	  if( g_nodesPerPartPerTimestep[Current_time_step][part_number-1]>=0 )
	    {
	      printinfo("USERD_get_part_coords restoring old values for Current_time_step=%d part_number=%d\n",
			Current_time_step, part_number );
	      if( g_xCoordsPerPartPerTimestep[Current_time_step][part_number-1] )
		{
		  l_numNodes = g_nodesPerPartPerTimestep[Current_time_step][part_number-1];
		  memcpy( &coord_array[0][1], g_xCoordsPerPartPerTimestep[Current_time_step][part_number-1], 
			  l_numNodes*sizeof(float) );
		  memcpy( &coord_array[1][1], g_yCoordsPerPartPerTimestep[Current_time_step][part_number-1], 
			  l_numNodes*sizeof(float) );
		  memcpy( &coord_array[2][1], g_zCoordsPerPartPerTimestep[Current_time_step][part_number-1], 
			  l_numNodes*sizeof(float) );
		}
	      return(Z_OK);
	    }
	}
    }
#endif


  l_numNodes = get_unstr_point_coords_used_by_part(l_whichUnstrPart,Current_time_step,0);


#ifdef LARRY1W_TEMP_FIX
  if( !l_numNodes && Current_time_step  )
    {
      /*we have no points at this timestep: check if we have any points at all, maybe
	the points dont change with time*/
      l_numNodes = get_unstr_point_coords_used_by_part(l_whichUnstrPart,0,0);
      if( l_numNodes  ) 
	{
	  /*I think this is wrong in larry1w: if pts are not time dependent, then
	    they shouldn't be in the suites at all, and should get time -1, not time 0*/
	  printinfo("   LARRY1W_TEMP_FIX: found no pts for this timestep, but found %d using time 0\n\n",l_numNodes);
	}
    }
#endif




  printinfo("\nsaf reader in USERD_get_part_coords, part_number=%d, l_whichUnstrPart=%d, Current_time_step=%d, l_numNodes=%d\n",
	    part_number,l_whichUnstrPart,Current_time_step,l_numNodes);

  if( l_numNodes )
    {
      l_origArray = (float *)malloc(3*l_numNodes*sizeof(float));
      if(!l_origArray)
	{
	  printinfo("USERD_get_part_coords failed to alloc l_origArray\n");
	  return(Z_ERR);
	}
      l_numNodes = get_unstr_point_coords_used_by_part(l_whichUnstrPart,Current_time_step,l_origArray);



#ifdef LARRY1W_TEMP_FIX
      if( !l_numNodes && Current_time_step  )
	{
	  /*we have no points at this timestep: check if we have any points at all, maybe
	    the points dont change with time*/
	  l_numNodes = get_unstr_point_coords_used_by_part(l_whichUnstrPart,0,l_origArray);
	  if( l_numNodes  ) 
	    {
	      /*I think this is wrong in larry1w: if pts are not time dependent, then
		they shouldn't be in the suites at all, and should get time -1, not time 0*/
	      printinfo("   LARRY1W_TEMP_FIX: found no pts for this timestep, but found %d using time 0\n\n",l_numNodes);
	    }
	}
#endif


      l_ptr=l_origArray;
      for(i=0; i<l_numNodes; i++)
	{
	  coord_array[0][i+1]=l_ptr[0];
	  coord_array[1][i+1]=l_ptr[1];
	  coord_array[2][i+1]=l_ptr[2];
	  l_ptr += 3;
	}
      free(l_origArray);

#if 1
      printinfo("\ncoord_array [%d]:\n",l_numNodes);
      for(i=0; i<l_numNodes; i++)
	{
	  printinfo("     [%d]= \t%e,\t%e,\t%e\n", i,
		    coord_array[0][i+1], coord_array[1][i+1], coord_array[2][i+1] );

	  if( i==5 && l_numNodes>10 ) i= l_numNodes-6;
	} 
      printinfo("\n");
#endif

    }




#ifdef RETAIN_COORDS_IN_MEMORY
  if( !g_xCoordsPerPartPerTimestep && get_num_timesteps() )
    {
      g_xCoordsPerPartPerTimestep = (float ***)malloc(get_num_timesteps()*sizeof(float **));
      g_yCoordsPerPartPerTimestep = (float ***)malloc(get_num_timesteps()*sizeof(float **));
      g_zCoordsPerPartPerTimestep = (float ***)malloc(get_num_timesteps()*sizeof(float **));
      g_nodesPerPartPerTimestep = (int **)malloc(get_num_timesteps()*sizeof(int *));

      g_bytesCostToRetainInfo += 3*get_num_timesteps()*sizeof(float **) + get_num_timesteps()*sizeof(int *);

      if(!g_xCoordsPerPartPerTimestep || !g_yCoordsPerPartPerTimestep || !g_zCoordsPerPartPerTimestep
	 || !g_nodesPerPartPerTimestep)
	{
	  printinfo("error allocating g_xCoordsPerPartPerTimestep\n");
	  exit(-1);
	}
      for(i=0;i<get_num_timesteps();i++)
	{
	  g_xCoordsPerPartPerTimestep[i]=0;
	  g_yCoordsPerPartPerTimestep[i]=0;
	  g_zCoordsPerPartPerTimestep[i]=0;
	  g_nodesPerPartPerTimestep[i]=0;
	}
    }

  if( !g_xCoordsPerPartPerTimestep[Current_time_step] )
    {
      g_xCoordsPerPartPerTimestep[Current_time_step] = (float **)malloc(Numparts_available*sizeof(float *));
      g_yCoordsPerPartPerTimestep[Current_time_step] = (float **)malloc(Numparts_available*sizeof(float *));
      g_zCoordsPerPartPerTimestep[Current_time_step] = (float **)malloc(Numparts_available*sizeof(float *));
      g_nodesPerPartPerTimestep[Current_time_step] = (int *)malloc(Numparts_available*sizeof(int));

      g_bytesCostToRetainInfo += 3*Numparts_available*sizeof(float *) + Numparts_available*sizeof(int);

      if(!g_xCoordsPerPartPerTimestep[Current_time_step] || !g_yCoordsPerPartPerTimestep[Current_time_step]
	 || !g_zCoordsPerPartPerTimestep[Current_time_step] || !g_nodesPerPartPerTimestep[Current_time_step])
	{
	  printinfo("error allocating g_xCoordsPerPartPerTimestep[Current_time_step]\n");
	  exit(-1);
	}
      for(i=0;i<Numparts_available;i++)
	{
	  g_xCoordsPerPartPerTimestep[Current_time_step][i]=0;
	  g_yCoordsPerPartPerTimestep[Current_time_step][i]=0;
	  g_zCoordsPerPartPerTimestep[Current_time_step][i]=0;
	  g_nodesPerPartPerTimestep[Current_time_step][i]=-1;
	}
    }


  if( g_nodesPerPartPerTimestep[Current_time_step][part_number-1]<0 ) 
    { 
      g_nodesPerPartPerTimestep[Current_time_step][part_number-1]=l_numNodes;
      printinfo("USERD_get_part_coords storing values for Current_time_step=%d part_number=%d numNodes=%d\n",
		Current_time_step, part_number, l_numNodes );
    
      if(l_numNodes)
	{
	  g_xCoordsPerPartPerTimestep[Current_time_step][part_number-1] = (float *)malloc(l_numNodes*sizeof(float));
	  g_yCoordsPerPartPerTimestep[Current_time_step][part_number-1] = (float *)malloc(l_numNodes*sizeof(float));
	  g_zCoordsPerPartPerTimestep[Current_time_step][part_number-1] = (float *)malloc(l_numNodes*sizeof(float));

	  g_bytesCostToRetainInfo += 3*l_numNodes*sizeof(float);
	  printinfo("saf reader RETAIN_COORDS_IN_MEMORY new cost is %d bytes\n",g_bytesCostToRetainInfo);
	  
	  if(!g_xCoordsPerPartPerTimestep[Current_time_step][part_number-1] 
	     || !g_yCoordsPerPartPerTimestep[Current_time_step][part_number-1]
	     || !g_zCoordsPerPartPerTimestep[Current_time_step][part_number-1] )
	    {
	      printinfo("error allocating g_xCoordsPerPartPerTimestep[Current_time_step][part_number-1]\n");
	      exit(-1);
	    }

	  memcpy( g_xCoordsPerPartPerTimestep[Current_time_step][part_number-1], &coord_array[0][1], 
		  l_numNodes*sizeof(float) );
	  memcpy( g_yCoordsPerPartPerTimestep[Current_time_step][part_number-1], &coord_array[1][1], 
		  l_numNodes*sizeof(float) );
	  memcpy( g_zCoordsPerPartPerTimestep[Current_time_step][part_number-1], &coord_array[2][1], 
		  l_numNodes*sizeof(float) );
	}
    }

#endif






  return(Z_OK);
}


/*--------------------------------------------------------------------
 * USERD_get_part_node_ids -
 *--------------------------------------------------------------------
 *
 *   Prior to API 2.01:
 *   ==================
 *   Get the node ids of an unstructured part.
 *
 *   Starting at API 2.01:
 *   ====================
 *   Get the node ids of an unstructured or structured part.
 *
 *
 *  (IN)  part_number             = The part number
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *  (OUT) nodeid_array            = 1D array containing node ids of
 *                                   each node in the part.
 *
 *       (IMPORTANT: this array is 1-based!!!)
 *
 *                                  (Array will have been allocated
 *                                   (number_of_nodes + 1) for the part long
 *                                   see USERD_get_gold_part_build_info)
 *
 *                       ex) If number_of_nodes = 100
 *                           as obtained in:
 *                             USERD_get_gold_part_build_info
 *
 *                           Then the allocated dimensions of the
 *                           pointer sent to this routine will be:
 *                             nodeid_array[101]
 *
 *                           Ignore the nodeid_array[0] location and start
 *                           the node ids at:
 *                             nodeid_array[1]
 *
 *                             nodeid_array[2]
 *
 *                                   etc.
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * This will be based on Current_time_step
 *
 *  * Not called unless number_of_nodes for the part is > 0  and
 *    node label status is TRUE
 *
 *  * The ids are purely labels.  However, any node id < 0 will never be
 *    displayed.
 *--------------------------------------------------------------------*/
int
USERD_get_part_node_ids(int part_number,
                        int *nodeid_array)
{
  int l_numNodes=0,i;
  int l_whichUnstrPart=part_number-1-get_num_blocks();

  printinfo("saf reader in USERD_get_part_node_ids, part_number=%d, l_whichUnstrPart=%d, Current_time_step=%d\n",
	 part_number,l_whichUnstrPart,Current_time_step);

  l_numNodes = get_unstr_point_coords_used_by_part(l_whichUnstrPart,Current_time_step,0);



#ifdef LARRY1W_TEMP_FIX
  if( !l_numNodes && Current_time_step  )
    {
      /*we have no points at this timestep: check if we have any points at all, maybe
	the points dont change with time*/
      l_numNodes = get_unstr_point_coords_used_by_part(l_whichUnstrPart,0,0);
      if( l_numNodes  ) 
	{
	  /*I think this is wrong in larry1w: if pts are not time dependent, then
	    they shouldn't be in the suites at all, and should get time -1, not time 0*/
	  printinfo("   LARRY1W_TEMP_FIX: found no pts for this timestep, but found %d using time 0\n\n",l_numNodes);
	}
    }
#endif


  printinfo("\n      INDEXING %d NODES\n\n",l_numNodes);
      
  for(i=0;i<l_numNodes;i++)
    {
      nodeid_array[i+1] = i+1;
    }

  return(Z_OK);
}

/*--------------------------------------------------------------------
 * USERD_get_part_elements_by_type -
 *--------------------------------------------------------------------
 *
 *   Gets the connectivities for the elements of a particular type
 *   in an unstructured part
 *
 *  (IN)  part_number             = The part number
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *  (IN)  element_type            = One of the following (See global_extern.h)
 *                                  Z_POINT    node point element
 *                                  Z_BAR02    2 node bar
 *                                  Z_BAR03    3 node bar
 *                                  Z_TRI03    3 node triangle
 *                                  Z_TRI06    6 node triangle
 *                                  Z_QUA04    4 node quad
 *                                  Z_QUA08    8 node quad
 *                                  Z_TET04    4 node tetrahedron
 *                                  Z_TET10   10 node tetrahedron
 *                                  Z_PYR05    5 node pyramid
 *                                  Z_PYR13   13 node pyramid
 *                                  Z_PEN06    6 node pentahedron
 *                                  Z_PEN15   15 node pentahedron
 *                                  Z_HEX08    8 node hexahedron
 *                                  Z_HEX20   20 node hexahedron
 *
 *   Starting at API 2.01:
 *   ====================
 *                                  Z_G_POINT    ghost node point element
 *                                  Z_G_BAR02    2 node ghost bar
 *                                  Z_G_BAR03    3 node ghost bar
 *                                  Z_G_TRI03    3 node ghost triangle
 *                                  Z_G_TRI06    6 node ghost triangle
 *                                  Z_G_QUA04    4 node ghost quad
 *                                  Z_G_QUA08    8 node ghost quad
 *                                  Z_G_TET04    4 node ghost tetrahedron
 *                                  Z_G_TET10   10 node ghost tetrahedron
 *                                  Z_G_PYR05    5 node ghost pyramid
 *                                  Z_G_PYR13   13 node ghost pyramid
 *                                  Z_G_PEN06    6 node ghost pentahedron
 *                                  Z_G_PEN15   15 node ghost pentahedron
 *                                  Z_G_HEX08    8 node ghost hexahedron
 *                                  Z_G_HEX20   20 node ghost hexahedron
 *                                  Z_NSIDED     n node ghost nsided polygon
 *                                  Z_NFACED     n face ghost nfaced polyhedron
 *
 *   Starting at API 2.02:
 *   ====================
 *                                  Z_NSIDED     n node nsided polygon
 *                                  Z_NFACED     n face nfaced polyhedron
 *                                  Z_G_NSIDED   n node ghost nsided polygon
 *                                  Z_G_NFACED   n face ghost nfaced polyhedron
 *
 *  (OUT) conn_array              = 2D array containing connectivity
 *                                   of each element of the type.
 *
 *                                  (Array will have been allocated
 *                                   number_of_elements of
 *                                   the type by connectivity length
 *                                   of the type)
 *
 *                       ex) If number_of_elements[Z_TRI03] = 25
 *                              number_of_elements[Z_QUA04] = 100
 *                              number_of_elements[Z_HEX08] = 30
 *                           as obtained in:
 *                            USERD_get_gold_part_build_info
 *
 *                           Then the allocated dimensions available
 *                           for this routine will be:
 *                              conn_array[25][3]   when called with Z_TRI03
 *
 *                              conn_array[100][4]  when called with Z_QUA04
 *
 *                              conn_array[30][8]   when called with Z_HEX08
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * This will be based on Current_time_step
 *--------------------------------------------------------------------*/
int
USERD_get_part_elements_by_type(int part_number,
                                int element_type,
                                int **conn_array)
{
  int l_whichUnstrPart=part_number-1-get_num_blocks();
  int l_numAvailPoints=0;

  printinfo("\nsaf reader in USERD_get_part_elements_by_type, part_number=%d  element_type=%d  ts=%d\n",
	    part_number,element_type,Current_time_step);

  l_numAvailPoints = get_unstr_point_coords_used_by_part(l_whichUnstrPart,Current_time_step,0);


#ifdef LARRY1W_TEMP_FIX
  if( !l_numAvailPoints && Current_time_step  )
    {
      /*we have no points at this timestep: check if we have any points at all, maybe
	the points dont change with time*/
      l_numAvailPoints = get_unstr_point_coords_used_by_part(l_whichUnstrPart,0,0);
      if( l_numAvailPoints ) 
	{
	  /*I think this is wrong in larry1w: if pts are not time dependent, then
	    they shouldn't be in the suites at all, and should get time -1, not time 0*/
	  printinfo("   LARRY1W_TEMP_FIX: found no pts for this timestep, but found %d using time 0\n\n",l_numAvailPoints);
	}
    }
#endif

  
  if( element_type == Z_POINT )
    {
      int l_numNodes=0,l_isball=0;

      l_numNodes = get_num_3ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      if(l_numNodes) l_isball=3;
      
      if(!l_numNodes)
	{
	  l_numNodes = get_num_2ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	  if(l_numNodes) l_isball=2;
	}
      if(!l_numNodes)
	{
	  l_numNodes = get_num_1ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	  if(l_numNodes) l_isball=1;
	}

      if(!l_numNodes) l_numNodes = get_num_point_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);

      printinfo(".....  Z_POINT, num elements=%d   num available pts=%d\n",l_numNodes,l_numAvailPoints);
      if(l_numNodes)
	{
	  int i;
	  int *l_origArray = (int *)malloc(l_numNodes*sizeof(int));
	  if(!l_origArray)
	    {
	      printinfo("USERD_get_part_elements_by_type failed to alloc l_origArray\n");
	      return(Z_ERR);
	    }
	  if(l_isball==3) 
	    get_3ball_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
	  if(l_isball==2) 
	    get_2ball_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
	  if(l_isball==1) 
	    get_1ball_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
	  else
	    get_point_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
	  for(i=0;i<l_numNodes;i++)
	    {
	      conn_array[i][0] = l_origArray[i]+1;/*note: this ensight array is 1-based?*/
	    }
	  free(l_origArray);
	}
    }
  else 
    {
      int l_numElem = 0;
      int l_numPtsInElem = 0;

      if(element_type==Z_BAR02)
	l_numElem = get_num_line_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_BAR03)
	l_numElem = get_num_quadraticLine_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_TRI03)
	l_numElem = get_num_tri_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_TRI06)
	l_numElem = get_num_quadraticTri_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_QUA04)
	l_numElem = get_num_quad_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_QUA08)
	l_numElem = get_num_quadraticQuad_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_TET04)
	l_numElem = get_num_tet_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_TET10)
	l_numElem = get_num_quadraticTet_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_PYR05)
	l_numElem = get_num_pyramid_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_PYR13)
	l_numElem = get_num_quadraticPyramid_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_PEN06)
	l_numElem = get_num_prism_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_PEN15)
	l_numElem = get_num_quadraticPrism_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_HEX08)
	l_numElem = get_num_hex_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if(element_type==Z_HEX20)
	l_numElem = get_num_quadraticHex_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);


      if(element_type==Z_BAR02) l_numPtsInElem=2;
      else if(element_type==Z_BAR03) l_numPtsInElem=3;
      else if(element_type==Z_TRI03) l_numPtsInElem=3;
      else if(element_type==Z_TRI06) l_numPtsInElem=6;
      else if(element_type==Z_QUA04) l_numPtsInElem=4;
      else if(element_type==Z_QUA08) l_numPtsInElem=8;
      else if(element_type==Z_TET04) l_numPtsInElem=4;
      else if(element_type==Z_TET10) l_numPtsInElem=10;
      else if(element_type==Z_PYR05) l_numPtsInElem=5;
      else if(element_type==Z_PYR13) l_numPtsInElem=13;
      else if(element_type==Z_PEN06) l_numPtsInElem=6;
      else if(element_type==Z_PEN15) l_numPtsInElem=15;
      else if(element_type==Z_HEX08) l_numPtsInElem=8;
      else if(element_type==Z_HEX20) l_numPtsInElem=20;


      printinfo(".....  %s num elements=%d   num available pts=%d\n",get_z_celltype_desc(element_type),
		l_numElem,l_numAvailPoints );

      if(l_numElem && l_numAvailPoints)
	{
          int i,j,l_numErrors=0,l_maxErrIndex=0;
	  int *l_origArray = (int *)malloc(l_numElem*l_numPtsInElem*sizeof(int));
	  if(!l_origArray)
	    {
	      printinfo("USERD_get_part_elements_by_type failed to alloc l_origArray\n");
	      return(Z_ERR);
	    }


          if(element_type==Z_BAR02) 
			get_line_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_BAR03) 
			get_quadraticLine_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_TRI03) 
			get_tri_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_TRI06) 
			get_quadraticTri_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_QUA04) 
			get_quad_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_QUA08) 
			get_quadraticQuad_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_TET04) 
			get_tet_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_TET10) 
			get_quadraticTet_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_PYR05) 
			get_pyramid_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_PYR13) 
			get_quadraticPyramid_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_PEN06) 
			get_prism_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_PEN15) 
			get_quadraticPrism_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_HEX08) 
			get_hex_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);
          else if(element_type==Z_HEX20) 
			get_quadraticHex_element_connectivity(l_whichUnstrPart,Current_time_step,l_origArray);


  	if( element_type == Z_PEN06 )
	{
	  for(i=0;i<l_numElem;i++)
	    {
	      /*by observation, this is the ordering used in the sierra data file gen5a-out.saf*/
	      int l_base = 6*i;
	      conn_array[i][0] = l_origArray[l_base+2]+1;
	      conn_array[i][1] = l_origArray[l_base+1]+1;/*one tip of the wedge*/
	      conn_array[i][2] = l_origArray[l_base+0]+1;
	      conn_array[i][3] = l_origArray[l_base+5]+1;
	      conn_array[i][4] = l_origArray[l_base+4]+1;/*the other tip of the wedge*/
	      conn_array[i][5] = l_origArray[l_base+3]+1;
	    }
	}
	else
	{
	  for(i=0;i<l_numElem;i++)
	    {
	      for(j=0;j<l_numPtsInElem;j++)
	        {
	          conn_array[i][j] = l_origArray[l_numPtsInElem*i+j]+1;/*note: this ensight array is 1-based?*/
		}
	    }
	}

	  /*test*/
	  for(i=0;i<l_numElem;i++)
	    {
	      for( j=0; j<l_numPtsInElem; j++ )
		{
		  if( conn_array[i][j]<1 || conn_array[i][j]>l_numAvailPoints )
		    {
		      if(!l_numErrors) 
			printinfo("%s INDEX ERROR: i=%d j=%d    %d  lim=%d\n",get_z_celltype_desc(element_type),
				i,j,conn_array[i][j],l_numAvailPoints );
		      l_numErrors++;

		      if( l_maxErrIndex < conn_array[i][j] ) l_maxErrIndex = conn_array[i][j];
		    }
		}
	    }
	  free(l_origArray);

	  if( l_numErrors ) 
	    {
	      printinfo("error: FOUND %d INDEX ERRORS! (max bad index was %d)(num pts avail is %d)\n\n\n",
			l_numErrors,l_maxErrIndex,l_numAvailPoints);
	      return(Z_ERR);
	    }
	}
      else if( l_numElem && !l_numAvailPoints )
	{
	  printinfo(".....  error: have polys but have no points\n\n");
	}
    }


  return(Z_OK);
}

/*--------------------------------------------------------------------
 * USERD_get_part_element_ids_by_type -
 *--------------------------------------------------------------------
 *
 *   Prior to API 2.01:
 *   ==================
 *   Gets the ids for the elements of a particular type for an
 *   unstructured part.
 *
 *   Starting at API 2.01:
 *   ====================
 *   Gets the ids for the elements of a particular type for an
 *   unstructured or structured part.
 *
 *
 *  (IN)  part_number             = The part number
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *  (IN)  element_type            = One of the following (See global_extern.h)
 *                                  Z_POINT    node point element
 *                                  Z_BAR02    2 node bar
 *                                  Z_BAR03    3 node bar
 *                                  Z_TRI03    3 node triangle
 *                                  Z_TRI06    6 node triangle
 *                                  Z_QUA04    4 node quad
 *                                  Z_QUA08    8 node quad
 *                                  Z_TET04    4 node tetrahedron
 *                                  Z_TET10   10 node tetrahedron
 *                                  Z_PYR05    5 node pyramid
 *                                  Z_PYR13   13 node pyramid
 *                                  Z_PEN06    6 node pentahedron
 *                                  Z_PEN15   15 node pentahedron
 *                                  Z_HEX08    8 node hexahedron
 *                                  Z_HEX20   20 node hexahedron
 *
 *   Starting at API 2.01:
 *   ====================
 *                                  Z_G_POINT    ghost node point element
 *                                  Z_G_BAR02    2 node ghost bar
 *                                  Z_G_BAR03    3 node ghost bar
 *                                  Z_G_TRI03    3 node ghost triangle
 *                                  Z_G_TRI06    6 node ghost triangle
 *                                  Z_G_QUA04    4 node ghost quad
 *                                  Z_G_QUA08    8 node ghost quad
 *                                  Z_G_TET04    4 node ghost tetrahedron
 *                                  Z_G_TET10   10 node ghost tetrahedron
 *                                  Z_G_PYR05    5 node ghost pyramid
 *                                  Z_G_PYR13   13 node ghost pyramid
 *                                  Z_G_PEN06    6 node ghost pentahedron
 *                                  Z_G_PEN15   15 node ghost pentahedron
 *                                  Z_G_HEX08    8 node ghost hexahedron
 *                                  Z_G_HEX20   20 node ghost hexahedron
 *
 *   Starting at API 2.02:
 *   ====================
 *                                  Z_NSIDED     n node nsided polygon
 *                                  Z_NFACED     n face nfaced polyhedron
 *                                  Z_G_NSIDED   n node ghost nsided polygon
 *                                  Z_G_NFACED   n face ghost nfaced polyhedron
 *
 *  (OUT) elemid_array            = 1D array containing id of each
 *                                   element of the type.
 *
 *                                  (Array will have been allocated
 *                                   number_of_elements of type long)
 *
 *                       ex) If number_of_elements[Z_TRI03] = 25
 *                              number_of_elements[Z_QUA04] = 100
 *                              number_of_elements[Z_HEX08] = 30
 *                           as obtained in:
 *                            USERD_get_gold_part_build_info
 *
 *                           Then the allocated dimensions available
 *                           for this routine will be:
 *                              elemid_array[25]    when called with Z_TRI03
 *
 *                              elemid_array[100]   when called with Z_QUA04
 *
 *                              elemif_array[30]    when called with Z_HEX08
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *
 *  * This will be based on Current_time_step
 *
 *  Prior to API 2.01:
 *  =================
 *  * Not called unless Num_unstructured_parts is > 0  and element
 *    label status is TRUE
 *
 *  Starting at API 2.01:
 *  ====================
 *  * Not called unless element label status is TRUE
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_get_part_element_ids_by_type(int part_number,
                                   int element_type,
                                   int __UNUSED__ *elemid_array)
{
  printinfo("saf reader in USERD_get_part_element_ids_by_type, part_number=%d  element_type=%d\n",part_number,element_type);
  return(Z_OK);
}


/*---------------------------
 * STRUCTURED DATA ROUTINES: 
 *---------------------------*/

/*--------------------------------------------------------------------
 * USERD_get_block_coords_by_component -
 *--------------------------------------------------------------------
 *
 *   Get the coordinates of a given block, component at a time
 *
 *  (IN)  block_number            = The block number
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *  (IN)  which_component         = Z_COMPX if x component wanted
 *                                = Z_COMPY if y component wanted
 *                                = Z_COMPZ if z component wanted
 *
 *  (OUT) coord_array             = 1D array containing x,y, or z
 *                                   coordinate component of each node
 *
 *                                  (Array will have been allocated
 *                                   i*j*k for the block long)
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * This will be based on Current_time_step
 *
 *  * Only called for structured "block" parts
 *--------------------------------------------------------------------*/
int
USERD_get_block_coords_by_component(int block_number,
                                    int which_component,
                                    float *coord_array)
{
  float *l_x=0,*l_y=0,*l_z=0;

  printinfo("saf reader in USERD_get_block_coords_by_component, block_number=%d\n",block_number);

  if( block_number-1<0 || block_number-1>=get_num_blocks() )
    {
      printinfo("error in USERD_get_block_coords_by_component, block_number out of range\n");
      return(Z_ERR);
    }

  if( which_component == Z_COMPX ) l_x=coord_array;
  else if( which_component == Z_COMPY ) l_y=coord_array;
  else if( which_component == Z_COMPZ ) l_z=coord_array;

  if( read_block_coords_as_curvilinear(block_number-1,l_x,l_y,l_z) ) 
    {
      printinfo(".....saf reader read_block_coords_as_curvilinear returned an error?\n");
      return(Z_ERR);
    }

  return(Z_OK);
}

/*--------------------------------------------------------------------
 * USERD_get_block_iblanking -
 *--------------------------------------------------------------------
 *
 *   Get the iblanking value at each node of a block - If Z_IBLANKED
 *
 *  (IN)  block_number            = The block number
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *  (OUT) iblank_array            = 1D array containing iblank value
 *                                   for each node.
 *
 *                                  (Array will have been allocated
 *                                   i*j*k for the block long)
 *
 *          possible values are:   Z_EXT     = exterior (outside)
 *                                 Z_INT     = interior (inside)
 *                                 Z_BND     = boundary
 *                                 Z_INTBND  = internal boundary
 *                                 Z_SYM     = symmetry plane
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * This will be based on Current_time_step
 *
 *  * Only called for structured "block" parts
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_get_block_iblanking(int __UNUSED__ block_number,
                          int __UNUSED__ *iblank_array)
{
  printinfo("saf reader in USERD_get_block_iblanking\n");
  /*jake: what is this?*/
  return(Z_OK);
}

/*--------------------------------------------------------------------
 * USERD_get_block_ghost_flags -
 *--------------------------------------------------------------------
 *
 *   Get the ghost_flags value at each element of a block.
 *
 *  (IN)  block_number            = The block number
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *  (OUT) ghost_flags             = 1D array containing ghost flag value
 *                                   for each block cell.
 *
 *                                  (Array will have been allocated
 *                                   (i-1)*(j-1)*(k-1) for the block long)
 *
 *          possible values are:    0  = non-ghost cell  (normal cell)
 *                                 >0  = ghost cell
 *
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * This will be based on Current_time_step
 *
 *  * Only called for structured "block" parts
 *
 *  * It is sufficient to set the value to be 1 to flag as a ghost cell,
 *    but the value can be any non-zero value, so you could use it to
 *    indicate which block or which server (for Server-of-server use) the
 *    cell is actually in.
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_get_block_ghost_flags(int __UNUSED__ block_number,
                            int __UNUSED__ *ghost_flags)
{
  printinfo("saf reader in USERD_get_block_ghost_flags\n");
  return(Z_OK);
}

/*-------------------
 * GENERAL ROUTINES: 
 *-------------------*/


/*--------------------------------------------------------------------
 * USERD_get_name_of_reader -
 *--------------------------------------------------------------------
 *
 *   Gets the name of the reader, so gui can list as a reader
 *
 *  (OUT) reader_name          = the name of the reader (data format)
 *                             (max length is Z_MAX_USERD_NAME, which 
 *                              is 20)
 *
 *  (OUT) *two_fields          = FALSE if only one data field is required
 *                                     in the data dialog of EnSight.
 *                               TRUE if two data fields required
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * Always called.  Provide a name for your custom reader format
 *
 *  * If you don't want a custom reader to show up in the data dialog
 *    choices, return a name of "No_Custom"
 *--------------------------------------------------------------------*/


int
USERD_get_name_of_reader(char reader_name[Z_MAX_USERD_NAME],
                         int *two_fields)

{
  /*printinfo("saf reader in USERD_get_name_of_reader\n");*/
 
  /*jake: using Z_MAX_USERD_NAME-1 because it seems Ensight has
    a different idea about string length....also doing this in
    many other parts of this file*/




#ifdef HARDWIRE_SAF_READER_VERSION_FOR_ENSIGHT
  strncpy(reader_name,"SAF_SSLIB",Z_MAX_USERD_NAME-1);
#else
  get_reader_name( reader_name, Z_MAX_USERD_NAME-1 );
#endif
 

 
  *two_fields = FALSE;

  return(Z_OK);
}

/*--------------------------------------------------------------------
 * USERD_get_reader_release -
 *--------------------------------------------------------------------
 *
 *   Gets the release string for the reader.
 *
 *   This release string is a free-format string which is for
 *   informational purposes only.  It is often useful to increment
 *   the release number/letter to indicate a change in the reader.
 *   The given string will simply be output by the EnSight server
 *   when the reader is selected.
 *
 *  (OUT) release_number       = the release number of the reader
 *                             (max length is Z_MAX_USERD_NAME, which
 *                              is 20)
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * Called when the reader is selected for use.
 *--------------------------------------------------------------------*/
int
USERD_get_reader_release(char version_number[Z_MAX_USERD_NAME])
{

  printinfo("saf reader in USERD_get_reader_release, Z_MAX_USERD_NAME=%d\n",Z_MAX_USERD_NAME);

  strncpy(version_number,"1.0",Z_MAX_USERD_NAME-1);
  return(Z_OK);
}



/*--------------------------------------------------------------------
 * USERD_get_reader_descrip -
 *--------------------------------------------------------------------
 *
 *  Gets the description of the reader, so gui can give more info
 *
 *  (OUT) reader_descrip       = the description of the reader
 *                             (max length is MAXFILENP, which
 *                              is 255)
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *--------------------------------------------------------------------*/
int
USERD_get_reader_descrip(char descrip[Z_MAXFILENP])
{

#if 1
  /*Dont get the real reader version by calling get_str_mesh_reader_version,
    because that will ensure that ensight core dumps on exit even if no saf
    file is opened.*/

  snprintf(descrip,Z_MAXFILENP-1,"SAF SSLIB Reader");
#else
  printinfo("saf reader in USERD_get_reader_descrip\n");

  get_str_mesh_reader_version(descrip,Z_MAXFILENP-1);
#endif

  return(Z_OK);

} /*End of USERD_get_reader_descrip()*/


/*--------------------------------------------------------------------
 * USERD_get_reader_version -
 *--------------------------------------------------------------------
 *
 *   Gets the API version number of the user defined reader
 *
 *   The functions that EnSight will call depends on this API
 *   version.  See the README files for more information.
 *
 *  (OUT) version_number       = the version number of the reader
 *                             (max length is Z_MAX_USERD_NAME, which
 *                              is 20)
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * Always called.
 *
 *  * This needs to be "2.000" or greater. Otherwise EnSight will assume
 *    this reader is API 1.0 instead of 2.0
 *--------------------------------------------------------------------*/
int
USERD_get_reader_version(char version_number[Z_MAX_USERD_NAME])
{

  /* 
   * This is one of the first functions ensight will call. So, initialize
   * the globals here so that all output goes to saf_reader_stdout.txt
   *
   * I think there are a couple fns called before this, but from a different proc?
   */
  if(!are_str_mesh_reader_globals_initialized()) 
    {
      initialize_str_globals();
    }


  printinfo("saf reader in USERD_get_reader_version\n");

  strncpy(version_number,"2.020",Z_MAX_USERD_NAME-1);
  return(Z_OK);
}


/*--------------------------------------------------------------------
 * USERD_set_filenames -
 *--------------------------------------------------------------------
 *
 *   Receives the geometry and result filenames entered in the data
 *   dialog.  The user written code will have to store and use these
 *   as needed.  The user written code must manage its own files!!
 *
 *  (IN) filename_1   = the filename entered into the geometry
 *                         field of the data dialog.
 *  (IN) filename_2   = the filename entered into the result
 *                         field of the data dialog.
 *                         (If the two_fields flag in USERD_get_name_of_reader
 *                          is FALSE, this will be null string)
 *  (IN) the_path     = the path info from the data dialog.
 *                      Note: filename_1 and filename_2 have already
 *                            had the path prepended to them.  This
 *                            is provided in case it is needed for
 *                            filenames contained in one of the files
 *  (IN) swapbytes    = TRUE if should swap bytes when reading data.
 *                    = FALSE normally
 *
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * Since you must manage everything from the input that is entered in
 *    these data dialog fields, this is an important routine!
 *
 *  * Since you manage these files, they can be whatever.  Perhaps
 *    you will use only one, and have references to everything else
 *    you need within it, like EnSight's case file does.
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_set_filenames(char filename_1[],
                    char filename_2[],
                    char __UNUSED__ the_path[],
                    int swapbytes)
{
  printinfo("saf reader in USERD_set_filenames\n");

  if(filename_2 && strlen(filename_2)) 
    {
      printinfo(" warning: saf reader only uses 1 filename, but a 2nd file was given (%s)\n",filename_2);
    }

  /*if(swapbytes==TRUE) printinfo("not an error: USERD_set_filenames swapbytes==TRUE ... IGNORING THIS\n");*/

  /*look_for_registry_in_ensight_tree();*/

  if( init_str_mesh_reader(filename_1) ) return(Z_ERR);

  if( init_unstr_mesh_reader(1/*1 for "use quadratic elems"*/) ) return(Z_ERR);


#ifdef USE_SOS_FOR_SAF
  {
    int i;
    int l_numObjects = get_num_blocks()+get_num_unstr_parts();
    if(l_numObjects)
      {
	g_objectIsValidInSOS=(int *)malloc( l_numObjects * sizeof(int) );
	for(i=0;i<l_numObjects;i++) g_objectIsValidInSOS[i]=1;
      }


#if 0
    /*simplest method: deal out a block to each process, in order, until they are gone*/
    if(Tot_Servers>1)
      {
	for(i=0;i<l_numObjects;i++)
	  {
	    if( (i%Tot_Servers) != Server_Number-1 ) g_objectIsValidInSOS[i]=0;
	  }
      }       
#else
    /*More complicated method: deal out blocks to processes based on the number of nodes
      at the first time step. Note that this method assumes that the user will load all
      of the structured and unstructured blocks! If some are not loaded, the balance 
      will likely be off.*/
    if(Tot_Servers>1)
      {
	int *l_numNodesPerServer = (int *)malloc(Tot_Servers*sizeof(int));
	for(i=0;i<Tot_Servers;i++) l_numNodesPerServer[i]=0;

	for(i=0;i<l_numObjects;i++)
	  {
	    int j, l_numNodes, l_lowestNumNodes, l_whichServer;

	    /*how many nodes does this object have?*/
	    if( i<get_num_blocks() )
	      {
		int l_x,l_y,l_z;
		get_block_size( i, &l_x, &l_y, &l_z );		
		l_numNodes=l_x*l_y*l_z;
	      }
	    else
	      {
		l_numNodes=get_unstr_point_coords_used_by_part( i-get_num_blocks(), 0, 0 );
	      }

	    /*which server should get this object?*/
	    l_lowestNumNodes = l_numNodesPerServer[0];
	    l_whichServer = 0;
	    for(j=0;j<Tot_Servers;j++)
	      {
		if( !l_numNodesPerServer[j] )
		  {
		    l_whichServer=j;
		    break;
		  }
		if( l_numNodesPerServer[j] < l_lowestNumNodes )
		  {
		    l_lowestNumNodes = l_numNodesPerServer[j];
		    l_whichServer = j;
		  }
	      }

	    /*set the corresponding flag so that only the selected server loads the object*/
	    l_numNodesPerServer[l_whichServer] += l_numNodes;
	    if( Server_Number-1 != l_whichServer ) g_objectIsValidInSOS[i]=0;

	  }

	/*print results*/
	for(i=0;i<l_numObjects;i++)
	  {
	    if( g_objectIsValidInSOS[i] ) printf("server %d got object %d  (server has %d total nodes)\n",
						 Server_Number-1,i,l_numNodesPerServer[Server_Number-1] );
	  }

	free(l_numNodesPerServer);
      }   
#endif

  }
#endif

  return(Z_OK);
}

/*--------------------------------------------------------------------
 * USERD_set_server_number -
 *--------------------------------------------------------------------
 *
 *   Receives the server number of how many total servers.
 *
 *  (IN) cur_serv    = the current server.
 *  (IN) tot_servs   = the total number of servers.
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * Only useful if your user defined reader is being used with EnSight's
 *    Server-of-Server capability.  And even then, it may or may not be
 *    something that you can take advantage of.  If your data is already 
 *    partitioned in some manner, such that you can access the proper
 *    portions using this information.
 *
 *    For all non-SOS uses, this will simply be 1 of 1
 *--------------------------------------------------------------------*/
void
USERD_set_server_number(int cur_serv,
                        int tot_servs)
{

  /*1-based, e.g. if tot_servs=2, then cur_serv's=1,2*/
  if(tot_servs>1) printinfo("saf reader in USERD_set_server_number: cur_serv=%d tot_servs=%d\n",cur_serv,tot_servs);

  Server_Number = cur_serv;
  Tot_Servers = tot_servs;

  if(Tot_Servers>1)
    {
      char l_str[256];
      sprintf(l_str,"saf_reader_stdout.%d.txt",Server_Number-1);
      printinfo_set_filename(l_str);
    }
}


/*--------------------------------------------------------------------
 * USERD_get_number_of_timesets -
 *--------------------------------------------------------------------
 *
 *  Gets the number of timesets used in the model.
 *
 *  If you have a static model, both geometry and variables, you should
 *  return a value of zero.
 *
 *  If you have a transient model, then you should return one or more.
 *
 *  For example:
 *
 *     Geometry    Variables                                 No. of timesets
 *     ---------   ------------------------------            ---------------
 *     static      static                                      0
 *     static      transient, all using same timeset           1
 *
 *     transient   transient, all using same timeset as geom   1
 *
 *     static      transient, using 3 different timesets       3
 *
 *     transient   transient, using 3 different timesets and
 *                            none of them the same as the
 *                            geometry timeset                 4
 *         etc.
 *
 *  NOTE: ALL GEOMETRY MUST USE THE SAME TIMESET!!! You will have to provide
 *                                                  the timeset number to use
 *                                                  for geometry in:
 *                                              USERD_get_geom_timeset_number
 *
 *        Variables can use the same timeset as the geometry, or can use
 *        other timesets. More than one variable can use the same timeset.
 *
 *  example:  changing geometry at 5 steps, 0.0, 1.0, 2.0, 3.0, 4.0
 *            variable 1 provided at these same five steps
 *            variable 2 provided at 3 steps, 0.5, 1.25, 3.33
 *
 *       This routine should return a value of 2, because only
 *       two different timesets are needed. Timeset 1 would be for the
 *       geometry and variable 1 (they both use it). Timeset 2 would
 *       be for variable 2, which needs its own in this case.
 *
 *  Notes:
 *--------------------------------------------------------------------*/
int
USERD_get_number_of_timesets( void )
{
  printinfo("saf reader in USERD_get_number_of_timesets FOUND %d TIMESETS\n",Num_timesets);
  return(Num_timesets);
}


/*--------------------------------------------------------------------
 * USERD_get_timeset_description -
 *--------------------------------------------------------------------
 *
 *  Get the description to associate with the desired timeset.
 *
 *  (IN)  timeset_number      = the timeset number
 *
 *                              For example: If USERD_get_number_of_timesets
 *                                           returns 2, the valid
 *                                           timeset_number's would be 1 and 2.
 *
 *  (OUT) timeset_description  = timeset description string
 *
 *
 *  returns: Z_OK  if successful
 *           Z_ERR if not successful
 *
 *  Notes:
 *  * A string of NULLs is valid for timeset_description
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_get_timeset_description(int __UNUSED__ timeset_number,
                              char timeset_description[Z_BUFL])
{
  printinfo("saf reader in USERD_get_timeset_description\n");

  strncpy(timeset_description,"timeset desc",Z_BUFL);

  return(Z_OK);
}



/*--------------------------------------------------------------------
 * USERD_get_geom_timeset_number -
 *--------------------------------------------------------------------
 *
 *  Gets the timeset number to be used for geometry
 *
 *  It must be in the valid range of timeset numbers
 *       For example,  If USERD_get_number_of_timesets
 *                     returns 2, the valid timeset_number's
 *                     would be 1 and 2.
 *
 *  Notes:
 *  * If your model is static, which you indicated by returning a zero
 *    in USERD_get_number_of_timesets, you can return a zero here as well.
 *
 *  * Geom_timeset_number is often set previously, such as in
 *    USERD_set_filenames.
 *--------------------------------------------------------------------*/
int
USERD_get_geom_timeset_number( void )
{
  printinfo("saf reader in USERD_get_geom_timeset_number\n");

  return(Geom_timeset_number);
}



/*--------------------------------------------------------------------
 * USERD_get_num_of_time_steps - 
 *-------------------------------------------------------------------- 
 * 
 *   Get the number of time steps of data available for desired timeset. 
 * 
 *  (IN)  timeset_number     = the timeset number
 *
 *                            For example: If USERD_get_number_of_timesets
 *                                         returns 2, the valid
 *                                         timeset_number's would be 1 and 2
 *
 *  returns: number of time steps in timeset (>0 if okay, <=0 if problems). 
 * 
 *  Notes: 
 *  * This should be >= 1       1 indicates a static problem 
 *                             >1 indicates a transient problem 
 * 
 *  * Num_time_steps[timeset_number] would be set here 
 *--------------------------------------------------------------------*/ 
int
USERD_get_num_of_time_steps( int timeset_number )
{
  printinfo("saf reader in USERD_get_num_of_time_steps, timeset_number=%d  get_num_timesteps()=%d\n",timeset_number,get_num_timesteps());



  Num_time_steps[timeset_number] = get_num_timesteps();
  return(Num_time_steps[timeset_number]);
}


/*--------------------------------------------------------------------
 * USERD_get_sol_times - 
 *-------------------------------------------------------------------- 
 * 
 *   Get the solution times associated with each time step
 *   for desired timeset. 
 * 
 *  (IN)  timeset_number     = the timeset number
 *
 *                             For example: If USERD_get_number_of_timesets
 *                                          returns 2, the valid
 *                                          timeset_number's would be 1 and 2
 *
 *  (OUT) solution_times     = 1D array of solution times per time step 
 * 
 *                                  (Array will have been allocated 
 *                                   Num_time_steps[timeset_number] long) 
 * 
 *  returns: Z_OK  if successful 
 *           Z_ERR if not successful 
 * 
 *  Notes: 
 *  * These must be non-negative and increasing. 
 *--------------------------------------------------------------------*/ 
int 
USERD_get_sol_times(int timeset_number,
                    float *solution_times) 
{ 
  int i;

  printinfo("saf reader in USERD_get_sol_times, timeset_number=%d, solution_times=%s\n",
		timeset_number, solution_times?"non-zero":"zero" );

  for(i=0;i<get_num_timesteps();i++)
    {
      solution_times[i]=get_time_value_for_timestep(i);
      /*printinfo("......set solution_times[%d]=%e\n",i,solution_times[i]);*/

      if( i && (solution_times[i]<=solution_times[i-1]) )
	{
	  float l_itWas = solution_times[i];
	  solution_times[i] = solution_times[i-1] + 1;
	  printinfo("\nwarning: ensight wont like time=%f, changing it to %f\n\n",l_itWas,solution_times[i]);
	}
    }

  return(Z_OK);
}



/*--------------------------------------------------------------------
 * USERD_set_time_set_and_step -
 *--------------------------------------------------------------------
 *
 *   Set the current time step in the desired timeset.  All functions that
 *   need time, and that do not explicitly pass it in, will use the timeset
 *   and step set by this routine, if needed.
 *
 *  (IN) timeset_number  = the timeset number (1 based).
 *
 *                         For example:  If USERD_get_number_of_timesets
 *                                       returns 2, the valid timeset_number's
 *                                       would be 1 and 2.
 *
 *  (IN) time_step - The current time step
 *
 *  Notes:
 *  * Current_time_step and Current_timeset would be set here
 *--------------------------------------------------------------------*/
void
USERD_set_time_set_and_step(int timeset_number,
                            int time_step)
{

  if( Current_time_step != time_step ||  Current_timeset != timeset_number )
    {
      printinfo("\n\nsaf reader CHANGING TIMESTEP in USERD_set_time_set_and_step: timeset_number=%d time_step=%d\n",
		timeset_number,time_step);
    }

  Current_time_step = time_step;
  Current_timeset = timeset_number;
  return;
}


/*--------------------------------------------------------------------
 * USERD_get_changing_geometry_status - 
 *-------------------------------------------------------------------- 
 * 
 *   Gets the changing geometry status 
 * 
 *  returns:  Z_STATIC        if geometry does not change 
 *            Z_CHANGE_COORDS if changing coordinates only 
 *            Z_CHANGE_CONN   if changing connectivity 
 * 
 *  Notes: 
 *  * EnSight does not support changing number of parts.  But the 
 *    coords and/or the connectivity of the parts can change. Note that 
 *    a part is allowed to be empty (number of nodes and elements equal 
 *    to zero). 
 *--------------------------------------------------------------------*/ 
int
USERD_get_changing_geometry_status( void )
{
  /*If there is only 1 (or 0) timesteps, and you dont use Z_STATIC, ensight
    will crash with no explanation!*/
  if( !get_num_unstr_parts() || get_num_timesteps()<2  )
    {
      printinfo("saf reader in USERD_get_changing_geometry_status, returning Z_STATIC\n");
      return(Z_STATIC);
    }
  else
    {
   #if 1
      /*I think this one is right*/
      printinfo("saf reader USERD_get_changing_geometry_status, picking Z_CHANGE_CONN because get_num_unstr_parts()=%d\n",
	     get_num_unstr_parts());
      return(Z_CHANGE_CONN);
   #else
      printinfo("saf reader USERD_get_changing_geometry_status, picking Z_CHANGE_COORDS cause get_num_unstr_parts()=%d\n",
	     get_num_unstr_parts());
      return(Z_CHANGE_COORDS);
   #endif
    }

}


/*--------------------------------------------------------------------
 * USERD_get_node_label_status - 
 *-------------------------------------------------------------------- 
 * 
 *  Answers the question as to whether node labels will be provided. 
 * 
 *  returns:  TRUE        if node labels will be provided 
 *            FALSE       if node labels will NOT be provided 
 * 
 *  Notes: 
 *  * These are needed in order to do any node querying, or node 
 *    labeling on-screen                               . 
 * 
 *  * Prior to API 2.01:
 *    =================
 *      For unstructured parts, you can read them from your file if 
 *      available, or can assign them, etc. They need to be unique 
 *      per part, and are often unique per model (especially
 *      if you are dealing with a decomposed dataset). They must also be 
 *      positive numbers greater than zero. 
 * 
 *        USERD_get_part_node_ids is used to obtain the ids, if the 
 *        status returned here is TRUE. 
 * 
 *        (Unlike API 1.0, where the connectivity of elements had to be 
 *         according to the node ids - API 2.0's element connectivities 
 *         are not affected either way by the status here.) 
 * 
 *        For structured parts, EnSight will assign ids if you return a 
 *        status of TRUE here.  You cannot assign them yourself!! 
 *
 *  * Starting at API 2.01:
 *    ====================
 *      For both unstructured and structured parts, you can read them
 *      from your file if available, or can assign them, etc. They need
 *      to be unique per part, and are often unique per model. They must
 *      also be positive numbers greater than zero. 
 * 
 *        USERD_get_part_node_ids is used to obtain the ids, if the 
 *        status returned here is TRUE. 
 *--------------------------------------------------------------------*/ 
int
USERD_get_node_label_status( void )
{
  printinfo("saf reader in USERD_get_node_label_status ....DOING NOTHING\n");
  return(FALSE);
}


/*--------------------------------------------------------------------
 * USERD_get_element_label_status - 
 *-------------------------------------------------------------------- 
 * 
 *  Answers the question as to whether element labels will be provided. 
 * 
 *  returns:  TRUE        if element labels will be provided 
 *            FALSE       if element labels will NOT be provided 
 * 
 *  Notes: 
 *  * These are needed in order to do any element querying, or 
 *    element labeling on-screen. 
 * 
 *  * Prior to API 2.01:
 *    =================
 *      For unstructured parts, you can read them from your file if 
 *      available, or can assign them, etc. They need to be unique 
 *      per part, and are often unique per model. 
 * 
 *      API 1.0:
 *        USERD_get_element_ids_for_part is used to obtain the ids, 
 *        on a part by part basis, if TRUE status is returned here. 
 * 
 *      API 2.0:
 *        USERD_get_part_element_ids_by_type is used to obtain the ids, 
 *        on an element type by part basis, if TRUE status is returned here. 
 *
 *      For structured parts, EnSight will assign ids if you return a 
 *        status of TRUE here.  You cannot assign them youself!! 
 *
 *   * Starting at API 2.01:
 *     ====================
 *      For both unstructured and structured parts, you can read them
 *      from your file if available, or can assign them, etc. They need
 *      to be unique per part, and are often unique per model (especially
 *      if you are dealing with a decomposed dataset). 
 * 
 *      USERD_get_part_element_ids_by_type is used to obtain the ids, 
 *      on an element type by part basis, if TRUE status is returned here. 
 *--------------------------------------------------------------------*/
int
USERD_get_element_label_status( void )
{
  printinfo("saf reader in USERD_get_element_label_status ....DOING NOTHING\n");
  return(FALSE);
}



/*-------------------------------------------------------------------
 * USERD_get_model_extents - 
 *------------------------------------------------------------------- 
 * 
 *   Gets the model bounding box extents.  If this routine supplys them 
 *   EnSight will not have to spend time doing so.  If this routine 
 *   returns Z_ERR, EnSight will have to take the time to touch all the 
 *   nodes and gather the extent info. 
 * 
 *  (OUT) extents[0] = min x 
 *               [1] = max x 
 *               [2] = min y 
 *               [3] = max y 
 *               [4] = min z 
 *               [5] = max z 
 * 
 *  returns: Z_ERR if no extents given (EnSight will need to calculate) 
 *           Z_OK if extents given 
 * 
 *  Notes: 
 *  * This will be based on Current_time_step 
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_get_model_extents( float __UNUSED__ extents[6] )
{
  printinfo("saf reader in USERD_get_model_extents: NOT SUPPLYING THEM....ok\n");
  return(Z_ERR);
}


/*--------------------------------------------------------------------
 * USERD_get_number_of_files_in_dataset -
 *--------------------------------------------------------------------
 *
 *   Get the total number of files in the dataset.  Used for the      
 *   dataset query option. 
 * 
 *  returns: the total number of files in the dataset 
 * 
 *  Notes: 
 *  * You can be as complete as you want about this.  If you don't 
 *    care about the dataset query option, return a value of 0 
 *    If you only want certain files, you can just include them. But, 
 *    you will need to supply the info in USERD_get_dataset_query_file_info 
 *    for each file you include here. 
 * 
 *  * Num_dataset_files would be set here 
 *--------------------------------------------------------------------*/ 
int 
USERD_get_number_of_files_in_dataset( void ) 
{
  printinfo("saf reader in USERD_get_number_of_files_in_dataset\n");
  Num_dataset_files = 1;
  return(Num_dataset_files);
}


/*--------------------------------------------------------------------
 * USERD_get_dataset_query_file_info - 
 *-------------------------------------------------------------------- 
 * 
 *   Get the information about files in the dataset.  Used for the 
 *   dataset query option. 
 * 
 *  (OUT) qfiles   = Structure containing information about each file 
 *                   of the dataset. The Z_QFILES structure is defined 
 *                   in the global_extern.h file 
 * 
 *                   (The structure will have been allocated 
 *                    Num_dataset_files long, with 10 description 
 *                    lines per file). 
 *                    (See USERD_get_number_of_files_in_dataset) 
 * 
 *      qfiles[].name        = The name of the file 
 *                             (Z_MAXFILENP is the dimensioned length 
 *                              of the name) 
 * 
 *      qfiles[].sizeb       = The number of bytes in the file 
 *                             (Typically obtained with a call to the 
 *                              "stat" system routine) 
 * 
 *      qfiles[].timemod     = The time the file was last modified 
 *                             (Z_MAXTIMLEN is the dimesioned length 
 *                              of this string) 
 *                             (Typically obtained with a call to the 
 *                              "stat" system routine) 
 * 
 *      qfiles[].num_d_lines = The number of description lines you 
 *                              are providing from the file. Max = 10 
 * 
 *      qfiles[].f_desc[]    = The description line(s) per file, 
 *                              qfiles[].num_d_lines of them 
 *                              (Z_MAXFILENP is the allocated length of 
 *                               each line) 
 * 
 *  returns: Z_OK  if successful 
 *           Z_ERR if not successful 
 * 
 *  Notes: 
 *  * If Num_dataset_files is 0, this routine will not be called. 
 *    (See USERD_get_number_of_files_in_dataset) 
 *--------------------------------------------------------------------*/ 
/*ARGSUSED*/
int
USERD_get_dataset_query_file_info(Z_QFILES __UNUSED__ *qfiles)
{
  printinfo("saf reader in USERD_get_dataset_query_file_info\n");
  return(Z_OK);
}



/*--------------------------------------------------------------------
 * USERD_get_descrip_lines - 
 *-------------------------------------------------------------------- 
 * 
 *   Get two description lines associated with geometry per time step, 
 *   or one description line associated with a variable per time step. 
 * 
 *  (IN)  which_type           = Z_GEOM for geometry 
 *                             = Z_VARI for variable 
 * 
 *  (IN)  which_var            = If it is a variable, which one. 
 *                               Ignored if geometry type. 
 * 
 *  (IN)  imag_data            = TRUE if want imaginary data file. 
 *                               FALSE if want real data file. 
 * 
 *  (OUT) line1                = The 1st geometry description line, 
 *                               or the variable description line. 
 * 
 *  (OUT) line2                = The 2nd geometry description line 
 *                               Not used if variable type. 
 * 
 *  returns: Z_OK  if successful 
 *           Z_ERR if not successful 
 * 
 *  Notes: 
 *  * This will be based on Current_time_step 
 * 
 *  * These are the lines EnSight can echo to the screen in 
 *    annotation mode. 
 *--------------------------------------------------------------------*/ 
/*ARGSUSED*/
int 
USERD_get_descrip_lines(int __UNUSED__ which_type,
                        int __UNUSED__ which_var,
                        int __UNUSED__ imag_data,
                        char __UNUSED__ line1[Z_BUFL],
                        char __UNUSED__ line2[Z_BUFL])
{
  /*jake: this happens before changing a timestep IF a var is mapped,
    or about to be mapped?*/

  printinfo("saf reader in USERD_get_descrip_lines\n");

  /*
  if( which_type == Z_GEOM )
    {
      snprintf(line1,Z_BUFL,"jake nothing for Z_GEOM yet");
    }
  else if( which_type == Z_VARI )
    {
    }
  */
  return(Z_OK);
}


/*-------------------------------------------------------------------
 * USERD_get_number_of_model_parts - 
 *------------------------------------------------------------------- 
 * 
 *   Gets the total number of unstructured and structured parts 
 *   in the model, for which you can supply information. 
 *
 *   This value is typically called:  Numparts_available
 * 
 *  returns:  Numparts_available  (>0 if okay, <=0 if probs) 
 * 
 *  Notes: 
 *  * IMPORTANT!!  The part or block numbers that get passed to various
 *                 routines in this API, will be the one-based table index
 *                 of these parts.
 *
 *                 For example, if you have three parts, the part or block
 *                 numbers of these parts will be:  1,2,3
 *
 *  * If going to have to read down through the parts in order to 
 *    know how many, you may want to build a table of pointers to 
 *    the various parts, so can easily get to particular parts in 
 *    later processes.  If you can simply read the number of parts 
 *    at the head of the file, then you would probably not build the 
 *    table at this time. 
 *-------------------------------------------------------------------*/
int
USERD_get_number_of_model_parts( void )
{
  printinfo("saf reader in USERD_get_number_of_model_parts, Current_timeset=%d\n",Current_timeset);

  Numparts_available = get_num_blocks() + get_num_unstr_parts();

  printinfo("saf reader LEAVING USERD_get_number_of_model_parts=%d (get_num_blocks()=%d get_num_unstr_parts()=%d) get_num_timesteps()=%d\n",
	 Numparts_available, get_num_blocks(), get_num_unstr_parts(), get_num_timesteps());

  return(Numparts_available);
}


/*--------------------------------------------------------------------
 * USERD_get_gold_part_build_info - 
 *-------------------------------------------------------------------- 
 * 
 *   Gets the info needed for part building process 
 * 
 *  (OUT) part_id                = Array containing the external part 
 *                                 ids for each of the model parts. 
 * 
 *                                 IMPORTANT: 
 *                                  Parts numbers must be >= 1, because
 *                                  of the way they are used in the GUI
 * 
 *             ******************************************* 
 *              The ids provided here are the numbers by
 *              which the parts will be referred to in the
 *              GUI (if possible). They are basically
 *              labels as far as you are concerned.
 *
 *              Note: The part numbers you pass to routines
 *              which receive a part_number or block_number
 *              or which_part as an argument are the 1-based
 *              table index of the parts!
 *
 *              example:  If Numparts_available = 3
 *
 *                        Table index        part_id
 *                        -----------        -------
 *                         1                  13
 *                         2                  57
 *                         3                  125
 *
 *                         ^                   ^
 *                         |                   |
 *                         |                    These are placed in:
 *                         |                      part_id[0] = 13
 *                         |                      part_id[1] = 57
 *                         |                      part_id[2] = 125
 *                         |                    for GUI labeling purposes.
 *                         |
 *                          These implied table indices are the part_number,
 *                          block_number, or which_part numbers that you would
 *                          pass to routines like:
 *
 *                         USERD_get_part_coords(int part_number,...
 *                         USERD_get_part_node_ids(int part_number,...
 *                         USERD_get_part_elements_by_type(int part_number,...
 *                         USERD_get_part_element_ids_by_type(int part_number,...
 *                         USERD_get_block_coords_by_component(int block_number,...
 *                         USERD_get_block_iblanking(int block_number,...
 *                         USERD_get_block_ghost_flags(int block_number,...
 *                         USERD_get_ghosts_in_block_flag(int block_number)
 *                         USERD_get_border_availability( int part_number,...
 *                         USERD_get_border_elements_by_type( int part_number,...
 *                         USERD_get_var_by_component(int which_variable,
 *                                                    int which_part,...
 *                         USERD_get_var_value_at_specific(int which_var,
 *                                                         int which_node_or_elem,
 *                                                         int which_part,...
 *             ******************************************** 
 * 
 *                                  (Array will have been allocated 
 *                                   Numparts_available long) 
 * 
 *  (OUT) part_types             = Array containing one of the 
 *                                 following for each model part: 
 * 
 *                                       Z_UNSTRUCTURED or 
 *                                       Z_STRUCTURED  or 
 *                                       Z_IBLANKED 
 * 
 *                                  (Array will have been allocated 
 *                                   Numparts_available long) 
 * 
 *  (OUT) part_description       = Array containing a description 
 *                                 for each of the model parts 
 * 
 *                                  (Array will have been allocated 
 *                                   Numparts_available by Z_BUFL 
 *                                   long) 
 * 
 *  (OUT) number_of_nodes        = Number of unstructured nodes in the part 
 * 
 *                                  (Array will have been allocated 
 *                                   Numparts_available long) 
 * 
 *  (OUT) number_of_elements     = 2D array containing number of 
 *                                 each type of element for each 
 *                                 unstructured model part. 
 *                                 ------------ 
 *                                 Possible types are: 
 * 
 *                                Z_POINT   =  point 
 *                                Z_BAR02   =  2-noded bar 
 *                                Z_BAR03   =  3-noded bar 
 *                                Z_TRI03   =  3-noded triangle 
 *                                Z_TRI06   =  6-noded triangle 
 *                                Z_QUA04   =  4-noded quadrilateral 
 *                                Z_QUA08   =  8-noded quadrilateral 
 *                                Z_TET04   =  4-noded tetrahedron 
 *                                Z_TET10   = 10-noded tetrahedron 
 *                                Z_PYR05   =  5-noded pyramid 
 *                                Z_PYR13   = 13-noded pyramid 
 *                                Z_PEN06   =  6-noded pentahedron 
 *                                Z_PEN15   = 15-noded pentahedron 
 *                                Z_HEX08   =  8-noded hexahedron 
 *                                Z_HEX20   = 20-noded hexahedron 
 * 
 *   Starting at API 2.01:
 *   ====================
 *                                Z_G_POINT    ghost node point element
 *                                Z_G_BAR02    2 node ghost bar
 *                                Z_G_BAR03    3 node ghost bar
 *                                Z_G_TRI03    3 node ghost triangle
 *                                Z_G_TRI06    6 node ghost triangle
 *                                Z_G_QUA04    4 node ghost quad
 *                                Z_G_QUA08    8 node ghost quad
 *                                Z_G_TET04    4 node ghost tetrahedron
 *                                Z_G_TET10   10 node ghost tetrahedron
 *                                Z_G_PYR05    5 node ghost pyramid
 *                                Z_G_PYR13   13 node ghost pyramid
 *                                Z_G_PEN06    6 node ghost pentahedron
 *                                Z_G_PEN15   15 node ghost pentahedron
 *                                Z_G_HEX08    8 node ghost hexahedron
 *                                Z_G_HEX20   20 node ghost hexahedron
 *
 *   Starting at API 2.02:
 *   ====================
 *                                Z_NSIDED     n node nsided polygon
 *                                Z_NFACED     n face nfaced polyhedron
 *                                Z_G_NSIDED   n node ghost nsided polygon
 *                                Z_G_NFACED   n face ghost nfaced polyhedron
 *
 *                               (Ignored unless Z_UNSTRUCTURED type) 
 * 
 *                                  (Array will have been allocated 
 *                                   Numparts_available by 
 *                                   Z_MAXTYPE long) 
 * 
 *  (OUT) ijk_dimensions         = 2D array containing ijk dimensions 
 *                                 for each structured model part. 
 *                                           ---------- 
 *                                  (Ignored if Z_UNSTRUCTURED type) 
 * 
 *                                  (Array will have been allocated 
 *                                   Numparts_available by 3 long) 
 * 
 *                             ijk_dimensions[][0] = I dimension 
 *                             ijk_dimensions[][1] = J dimension 
 *                             ijk_dimensions[][2] = K dimension 
 * 
 *  (OUT) iblanking_options      = 2D array containing iblanking 
 *                                 options possible for each 
 *                                 structured model part. 
 *                                 ---------- 
 *                                 (Ignored unless Z_IBLANKED type) 
 * 
 *                                 (Array will have been allocated 
 *                                  Numparts_available by 6 long) 
 * 
 *      iblanking_options[][Z_EXT]     = TRUE if external (outside) 
 *                       [][Z_INT]     = TRUE if internal (inside) 
 *                       [][Z_BND]     = TRUE if boundary 
 *                       [][Z_INTBND]  = TRUE if internal boundary 
 *                       [][Z_SYM]     = TRUE if symmetry surface 
 * 
 *  returns: Z_OK  if successful 
 *           Z_ERR if not successful 
 * 

 *  Notes: 
 *  * If you haven't built a table of pointers to the different parts, 
 *    you might want to do so here as you gather the needed info. 
 * 
 *  * This will be based on Current_time_step 
 *--------------------------------------------------------------------*/ 
/*ARGSUSED*/
int 
USERD_get_gold_part_build_info(int *part_id,
                               int *part_types,
                               char *part_description[Z_BUFL],
                               int *number_of_nodes,
                               int *number_of_elements[Z_MAXTYPE],
                               int *ijk_dimensions[3],
                               int __UNUSED__ *iblanking_options[6])
{
  int i,j,l_x,l_y,l_z;

  printinfo("\n*******BEGIN   USERD_get_gold_part_build_info, Current_time_step=%d ***************\n",Current_time_step);



  if( Numparts_available != get_num_blocks() + get_num_unstr_parts() )
    {
      printinfo("error in USERD_get_gold_part_build_info  Numparts_available=%d (get_num_blocks()=%d get_num_unstr_parts()=%d)\n",
		Numparts_available, get_num_blocks(), get_num_unstr_parts() );
      return(Z_ERR);
    }



  /*handle structured meshes (blocks)*/
  for(i=0;i<get_num_blocks();i++)
    {
      part_id[i]=i+1;
      part_types[i]=Z_STRUCTURED;

      get_block_name( i, part_description[i], Z_BUFL-1 );
      printinfo("......saf reader got part_description[%d]=%s, Z_BUFL=%d\n",i,part_description[i],Z_BUFL);
      number_of_nodes[i]=0;/*number_of_nodes ignored for structured?*/   
      for(j=0;j<Z_MAXTYPE;j++) number_of_elements[i][j]=0;/*number_of_elements ignored for structured?*/

#ifdef USE_SOS_FOR_SAF
      if(g_objectIsValidInSOS && !g_objectIsValidInSOS[i])
	{
	  /*in server-of-servers mode, this process does not contain this object*/
	  ijk_dimensions[i][0]=0;
	  ijk_dimensions[i][1]=0;
	  ijk_dimensions[i][2]=0;
	  continue;
	}
#endif

      get_block_size( i, &l_x, &l_y, &l_z );
      ijk_dimensions[i][0]=l_x;
      ijk_dimensions[i][1]=l_y;
      ijk_dimensions[i][2]=l_z;
      printinfo("......saf reader got  ijk_dimensions[%d]= %d %d %d\n",i,ijk_dimensions[i][0],ijk_dimensions[i][1],ijk_dimensions[i][2] );
    }

  for(i=get_num_blocks();i<get_num_blocks()+get_num_unstr_parts();i++)
    {
      int l_whichUnstrPart = i-get_num_blocks();

      part_id[i]=i+1;
      part_types[i]=Z_UNSTRUCTURED;
      get_unstr_part_name( l_whichUnstrPart, part_description[i], Z_BUFL-1 );
      for(j=0;j<Z_MAXTYPE;j++) number_of_elements[i][j]=0;/*set all to 0 to start with*/


#ifdef USE_SOS_FOR_SAF
      if(g_objectIsValidInSOS && !g_objectIsValidInSOS[i])
	{
	  /*in server-of-servers mode, this process does not contain this object*/
	  number_of_nodes[i]=0;
	  continue;
	}
#endif


      number_of_nodes[i] = get_unstr_point_coords_used_by_part( l_whichUnstrPart, Current_time_step, 0 );



#ifdef LARRY1W_TEMP_FIX
      if( !number_of_nodes[i] && Current_time_step  )
	{
	  /*we have no points at this timestep: check if we have any points at all, maybe
	    the points dont change with time*/
	  number_of_nodes[i] = get_unstr_point_coords_used_by_part(l_whichUnstrPart,0,0);
	  if( number_of_nodes[i]  ) 
	    {
	      /*I think this is wrong in larry1w: if pts are not time dependent, then
		they shouldn't be in the suites at all, and should get time -1, not time 0*/
	      printinfo("   LARRY1W_TEMP_FIX: found no pts for this timestep, but found %d using time 0\n\n",number_of_nodes[i]);
	    }
	}
#endif


      number_of_elements[i][Z_POINT]=0;

      number_of_elements[i][Z_TRI03]=get_num_tri_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_QUA04]=get_num_quad_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_TET04]=get_num_tet_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_HEX08]=get_num_hex_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_PYR05]=get_num_pyramid_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_PEN06]=get_num_prism_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_BAR02]=get_num_line_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);

      number_of_elements[i][Z_TRI06]=get_num_quadraticTri_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_QUA08]=get_num_quadraticQuad_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_TET10]=get_num_quadraticTet_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_HEX20]=get_num_quadraticHex_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_PYR13]=get_num_quadraticPyramid_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_PEN15]=get_num_quadraticPrism_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      number_of_elements[i][Z_BAR03]=get_num_quadraticLine_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);


      {
	int l_num1Balls= get_num_1ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	int l_num2Balls= get_num_2ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	int l_num3Balls= get_num_3ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);

	if(l_num3Balls) number_of_elements[i][Z_POINT]=l_num3Balls;
	else if(l_num2Balls) number_of_elements[i][Z_POINT]=l_num2Balls;
	else if(l_num1Balls) number_of_elements[i][Z_POINT]=l_num1Balls;

	if(l_num1Balls) printinfo("warning saf reader USERD_get_gold_part_build_info got SAF_CELLTYPE_1BALL's\n");
	if(l_num2Balls) printinfo("warning saf reader USERD_get_gold_part_build_info got SAF_CELLTYPE_2BALL's\n");
	if(l_num3Balls) printinfo("warning saf reader USERD_get_gold_part_build_info got SAF_CELLTYPE_3BALL's\n");

	if( l_num3Balls && (l_num1Balls||l_num2Balls) )
	  printinfo("warning got more than one kind of SAF_CELLTYPE_*BALL: using only SAF_CELLTYPE_3BALL\n");
	else if( l_num2Balls && l_num1Balls )
	  printinfo("warning got both SAF_CELLTYPE_2BALL and SAF_CELLTYPE_1BALL: using only SAF_CELLTYPE_2BALL\n");
      }

      if( number_of_elements[i][Z_TRI03] || 
	  number_of_elements[i][Z_QUA04] ||
	  number_of_elements[i][Z_TET04] ||
	  number_of_elements[i][Z_HEX08] ||
	  number_of_elements[i][Z_PYR05] ||
	  number_of_elements[i][Z_PEN06] ||
	  number_of_elements[i][Z_BAR02] ||
          number_of_elements[i][Z_TRI06] || 
	  number_of_elements[i][Z_QUA08] ||
	  number_of_elements[i][Z_TET10] ||
	  number_of_elements[i][Z_HEX20] ||
	  number_of_elements[i][Z_PYR13] ||
	  number_of_elements[i][Z_PEN15] ||
	  number_of_elements[i][Z_BAR03] ||
	  number_of_elements[i][Z_POINT] )
	{
	  /*If there are other elements in this part, then we will assume that the 
	    points are for connectivity only, and dont need to be made into elements themselves*/
	}
      else
	{
	  number_of_elements[i][Z_POINT]=get_num_point_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	}


      printinfo("saf reader USERD_get_gold_part_build_info,end of: l_whichUnstrPart=%d (i=%d) numnodes=%d ts=%d\n",
		l_whichUnstrPart,i,number_of_nodes[i], Current_time_step);
      if(number_of_elements[i][Z_POINT]) printinfo("     Z_POINT=%d\n", number_of_elements[i][Z_POINT] );
      if(number_of_elements[i][Z_TRI03]) printinfo("     Z_TRI03=%d\n", number_of_elements[i][Z_TRI03] );
      if(number_of_elements[i][Z_QUA04]) printinfo("     Z_QUA04=%d\n", number_of_elements[i][Z_QUA04] );
      if(number_of_elements[i][Z_TET04]) printinfo("     Z_TET04=%d\n", number_of_elements[i][Z_TET04] );
      if(number_of_elements[i][Z_HEX08]) printinfo("     Z_HEX08=%d\n", number_of_elements[i][Z_HEX08] );
      if(number_of_elements[i][Z_PYR05]) printinfo("     Z_PYR05=%d\n", number_of_elements[i][Z_PYR05] );
      if(number_of_elements[i][Z_PEN06]) printinfo("     Z_PEN06=%d\n", number_of_elements[i][Z_PEN06] );
      if(number_of_elements[i][Z_BAR02]) printinfo("     Z_BAR02=%d\n", number_of_elements[i][Z_BAR02] );

      if(number_of_elements[i][Z_TRI06]) printinfo("     Z_TRI06=%d\n", number_of_elements[i][Z_TRI06] );
      if(number_of_elements[i][Z_QUA08]) printinfo("     Z_QUA08=%d\n", number_of_elements[i][Z_QUA08] );
      if(number_of_elements[i][Z_TET10]) printinfo("     Z_TET10=%d\n", number_of_elements[i][Z_TET10] );
      if(number_of_elements[i][Z_HEX20]) printinfo("     Z_HEX20=%d\n", number_of_elements[i][Z_HEX20] );
      if(number_of_elements[i][Z_PYR13]) printinfo("     Z_PYR13=%d\n", number_of_elements[i][Z_PYR13] );
      if(number_of_elements[i][Z_PEN15]) printinfo("     Z_PEN15=%d\n", number_of_elements[i][Z_PEN15] );
      if(number_of_elements[i][Z_BAR03]) printinfo("     Z_BAR03=%d\n", number_of_elements[i][Z_BAR03] );

      if(get_num_unknown_elements_in_unstr_part(l_whichUnstrPart,Current_time_step))
	printinfo("     Z_unknown?=%d\n", get_num_unknown_elements_in_unstr_part(l_whichUnstrPart,Current_time_step) );

#if 1 /*print some info*/
      {
	int l_numDifferentTypes=0;
	if( number_of_elements[i][Z_POINT] ) l_numDifferentTypes++;
	if( number_of_elements[i][Z_TRI03] ) l_numDifferentTypes++;
	if( number_of_elements[i][Z_QUA04] ) l_numDifferentTypes++;
	if( number_of_elements[i][Z_TET04] ) l_numDifferentTypes++;
	if( number_of_elements[i][Z_HEX08] ) l_numDifferentTypes++;
	if( number_of_elements[i][Z_PYR05] ) l_numDifferentTypes++;
	if( number_of_elements[i][Z_PEN06] ) l_numDifferentTypes++;
	if( number_of_elements[i][Z_BAR02] ) l_numDifferentTypes++;
        if(number_of_elements[i][Z_TRI06]) l_numDifferentTypes++;
        if(number_of_elements[i][Z_QUA08]) l_numDifferentTypes++;
        if(number_of_elements[i][Z_TET10]) l_numDifferentTypes++;
        if(number_of_elements[i][Z_HEX20]) l_numDifferentTypes++;
        if(number_of_elements[i][Z_PYR13]) l_numDifferentTypes++;
        if(number_of_elements[i][Z_PEN15]) l_numDifferentTypes++;
        if(number_of_elements[i][Z_BAR03]) l_numDifferentTypes++;
	if( l_numDifferentTypes>1 ) printinfo("     warning: there are %d different types of element in this part\n",
					      l_numDifferentTypes);
      }
#endif


      printinfo("\n");
    }



  printinfo("*******END     USERD_get_gold_part_build_info***************************\n\n");

  return(Z_OK);
}


/*--------------------------------------------------------------------
 * USERD_get_ghosts_in_block_flag - 
 *-------------------------------------------------------------------- 
 * 
 *   Gets whether ghost cells present in block or not
 * 
 *  (IN) block_number      = The block part number
 *
 *                          (1-based index of part table, namely:
 *
 *                             1 ... Numparts_available.
 *
 *                           It is NOT the part_id that
 *                           is loaded in USERD_get_gold_part_build_info)
 * 
 *  returns: TRUE  if any ghost cells in this strucutred part
 *           FALSE if no ghost cells in this strucutred part
 * 
 *  Notes: 
 *  * This will be based on Current_time_step 
 *
 *  * Intended for structured parts only, value will be ignored for
 *    unstructured parts
 *--------------------------------------------------------------------*/ 
int 
USERD_get_ghosts_in_block_flag(int block_number)
{
  printinfo("saf reader in USERD_get_ghosts_in_block_flag for block %d, NO GHOST CELLS\n",block_number);
  return(FALSE);
}

/*--------------------------------------------------------------------
 * USERD_get_maxsize_info -
 *--------------------------------------------------------------------
 *
 *   Gets maximum part sizes for efficient memory allocation.
 *
 *   Transient models (especially those that increase in size) can cause
 *   reallocations, at time step changes, to keep chewing up more and
 *   more memory.   The way to avoid this is to know what the maximum
 *   size of such memory will be, and allocate for this maximum initially.
 *
 *   Accordingly, if you choose to provide this information (it is optional),
 *   EnSight will take advantage of it.
 *
 *
 *  (OUT) max_number_of_nodes    = Maximum number of unstructured nodes
 *                                 that will be in the part (over all time).
 *
 *                                  (Array will have been allocated
 *                                   Numparts_available long)
 *
 *  (OUT) max_number_of_elements = 2D array containing maximum number of
 *                                 each type of element for each
 *                                 unstructured model part (over all time).
 *                                 ------------
 *                                 Possible types are:
 *
 *                                Z_POINT   =  point
 *                                Z_BAR02   =  2-noded bar
 *                                Z_BAR03   =  3-noded bar
 *                                Z_TRI03   =  3-noded triangle
 *                                Z_TRI06   =  6-noded triangle
 *                                Z_QUA04   =  4-noded quadrilateral
 *                                Z_QUA08   =  8-noded quadrilateral
 *                                Z_TET04   =  4-noded tetrahedron
 *                                Z_TET10   = 10-noded tetrahedron
 *                                Z_PYR05   =  5-noded pyramid
 *                                Z_PYR13   = 13-noded pyramid
 *                                Z_PEN06   =  6-noded pentahedron
 *                                Z_PEN15   = 15-noded pentahedron
 *                                Z_HEX08   =  8-noded hexahedron
 *                                Z_HEX20   = 20-noded hexahedron
 *
 *   Starting at API 2.01:
 *   ====================
 *                                Z_G_POINT    ghost node point element
 *                                Z_G_BAR02    2 node ghost bar
 *                                Z_G_BAR03    3 node ghost bar
 *                                Z_G_TRI03    3 node ghost triangle
 *                                Z_G_TRI06    6 node ghost triangle
 *                                Z_G_QUA04    4 node ghost quad
 *                                Z_G_QUA08    8 node ghost quad
 *                                Z_G_TET04    4 node ghost tetrahedron
 *                                Z_G_TET10   10 node ghost tetrahedron
 *                                Z_G_PYR05    5 node ghost pyramid
 *                                Z_G_PYR13   13 node ghost pyramid
 *                                Z_G_PEN06    6 node ghost pentahedron
 *                                Z_G_PEN15   15 node ghost pentahedron
 *                                Z_G_HEX08    8 node ghost hexahedron
 *                                Z_G_HEX20   20 node ghost hexahedron
 *
 *   Starting at API 2.02:
 *   ====================
 *                                Z_NSIDED     n node nsided polygon
 *                                Z_NFACED     n face nfaced polyhedron
 *                                Z_G_NSIDED   n node ghost nsided polygon
 *                                Z_G_NFACED   n face ghost nfaced polyhedron
 *
 *                               (Ignored unless Z_UNSTRUCTURED type)
 *
 *                                  (Array will have been allocated
 *                                   Numparts_available by
 *                                   Z_MAXTYPE long)
 *
 *  (OUT) max_ijk_dimensions  = 2D array containing maximum ijk dimensions
 *                              for each structured model part (over all time).
 *                                           ----------
 *                                (Ignored if Z_UNSTRUCTURED type)
 *
 *                                (Array will have been allocated
 *                                 Numparts_available by 3 long)
 *
 *                             max_ijk_dimensions[][0] = maximum I dimension
 *                             max_ijk_dimensions[][1] = maximum J dimension
 *                             max_ijk_dimensions[][2] = maximum K dimension
 *
 *  returns: Z_OK  if supplying maximum data
 *           Z_ERR if not supplying maximum data, or some error occurred
 *                   while trying to obtain it.
 *
 *  Notes:
 *  * You need to have first called USERD_get_number_of_model_parts and
 *    USERD_get_gold_part_build_info, so Numparts_available is known and
 *    so EnSight will know what the type is (Z_UNSTRUCTURED, Z_STRUCTURED,
 *    or Z_IBLANKED) of each part.
 *
 *  * This will NOT be based on Current_time_step - it is to be the maximum
 *    values over all time!!
 *
 *  * This information is optional.  If you return Z_ERR, Ensight will still
 *    process things fine, reallocating as needed, etc.  However, for
 *    large transient models you will likely use considerably more memory
 *    and take more processing time for the memory reallocations. So, if it
 *    is possible to provide this information "up front", it is recommended
 *    to do so.
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_get_maxsize_info(int __UNUSED__ *max_number_of_nodes,
                       int __UNUSED__ *max_number_of_elements[Z_MAXTYPE],
                       int __UNUSED__ *max_ijk_dimensions[3])
{
  printinfo("saf reader in USERD_get_maxsize_info: NOT PROVIDING MAXSIZE INFO...OK\n");
  return(Z_ERR);
}


/*--------------------------------------------------------------------
 * USERD_get_ghosts_in_model_flag - 
 *-------------------------------------------------------------------- 
 * 
 *  Answers the question as to whether any ghost cells in the model. 
 * 
 *  returns:  TRUE        if any ghost cells in the model 
 *            FALSE       if no ghost cells in the model
 * 
 *  Notes: 
 *  * This routine is new in the 2.01 API
 *--------------------------------------------------------------------*/ 
int
USERD_get_ghosts_in_model_flag( void )
{
  printinfo("saf reader in USERD_get_ghosts_in_model_flag  NO GHOST CELLS\n");
  return(FALSE);
}



/*--------------------------------------------------------------------
 * USERD_get_border_availability - 
 *-------------------------------------------------------------------- 
 * 
 *   Finds out if border elements are provided by the reader for the
 *   desired part, or will need to be computed internally by EnSight.
 * 
 *  (IN)  part_number             = The part number
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *   (OUT) number_of_elements     = 2D array containing number of
 *                                  each type of border element in
 *                                  the part.
 *                                  ------------
 *                                  Possible types are:
 *
 *                                  Z_POINT   =  point
 *                                  Z_BAR02   =  2-noded bar
 *                                  Z_BAR03   =  3-noded bar
 *                                  Z_TRI03   =  3-noded triangle
 *                                  Z_TRI06   =  6-noded triangle
 *                                  Z_QUA04   =  4-noded quadrilateral
 *                                  Z_QUA08   =  8-noded quadrilateral
 *
 *  Returns:
 *  -------
 *  Z_OK  if border elements will be provided by the reader.
 *         (number_of_elements array will be loaded and
 *          USERD_get_border_elements_by_type will be called)
 *
 *  Z_ERR if border elements are not available - thus EnSight must compute.
 *         (USERD_get_border_elements_by_type will not be called)
 *
 *
 *  Notes:
 *  -----
 *  * Only called if border representation is used.
 *
 *  * Will be based on Current_time_step
 *
 *--------------------------------------------------------------------*/ 
/*ARGSUSED*/
int
USERD_get_border_availability( int __UNUSED__ part_number,
                               int __UNUSED__ number_of_elements[Z_MAXTYPE])
{
  printinfo("saf reader in USERD_get_border_availability BORDER ELEMS NOT AVAILABLE\n");
  return(Z_ERR);
}



/*--------------------------------------------------------------------
 * USERD_get_border_elements_by_type - 
 *-------------------------------------------------------------------- 
 * 
 *   Provides border element connectivity and parent information.
 *
 *  (IN)  part_number             = The part number
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 *
 *  (IN)  element_type            = One of the following (See global_extern.h)
 *                                  Z_POINT    node point element
 *                                  Z_BAR02    2 node bar
 *                                  Z_BAR03    3 node bar
 *                                  Z_TRI03    3 node triangle
 *                                  Z_TRI06    6 node triangle
 *                                  Z_QUA04    4 node quad
 *                                  Z_QUA08    8 node quad
 *
 *  (OUT) conn_array              = 2D array containing connectivity
 *                                  of each border element of the type.
 *
 *                                 (Array will have been allocated
 *                                  num_of_elements of the type by
 *                                  connectivity length of the type)
 *
 *                      ex) If number_of_elements[Z_TRI03] = 25
 *                             number_of_elements[Z_QUA04] = 100
 *                             number_of_elements[Z_QUA08] = 30
 *                          as obtained in:
 *                           USERD_get_border_availability
 *
 *                          Then the allocated dimensions available
 *                          for this routine will be:
 *                             conn_array[25][3]   when called with Z_TRI03
 *
 *                             conn_array[100][4]  when called with Z_QUA04
 *
 *                             conn_array[30][8]   when called with Z_QUA08
 *
 *  (OUT) parent_element_type   = 1D array containing element type of the
 *                                parent element (the one that the border
 *                                element is a face/edge of).
 *
 *                               (Array will have been allocated
 *                                num_of_elements of the type long)
 *
 *  (OUT) parent_element_num   = 1D array containing element number of the
 *                                parent element (the one that the border
 *                                element is a face/edge of).
 *
 *                               (Array will have been allocated
 *                                num_of_elements of the type long)
 *
 *  Returns:
 *  -------
 *  Z_OK  if successful
 *  Z_ERR if not successful
 *
 *  Notes:
 *  -----
 *  * Only called if USERD_get_border_availability returned Z_OK
 *
 *  * Will be based on Current_time_step
 *
 *--------------------------------------------------------------------*/ 
/*ARGSUSED*/
int
USERD_get_border_elements_by_type( int __UNUSED__ part_number,
                                   int __UNUSED__ element_type,
                                   int __UNUSED__ **conn_array,
                                   short __UNUSED__ *parent_element_type,
                                   int __UNUSED__ *parent_element_num)
{
  printinfo("saf reader in USERD_get_border_elements_by_type\n");
  return(Z_ERR);
}



/*--------------------------------------------------------------------
 * USERD_get_number_of_variables - 
 *-------------------------------------------------------------------- 
 * 
 *   Get the number of variables (includes constant, scalar, and 
 *   vector types), for which you will be providing info. 
 * 
 *  returns: number of variables (includes constant, scalar, vector, 
 *            and tensor types) 
 *            >=0 if okay 
 *            <0 if problem 
 * 
 *  Notes: 
 *   ***************************************************************** 
 *  * Variable numbers, by which references will be made, are implied 
 *    here. If you say there are 3 variables, the variable numbers 
 *    will be 1, 2, and 3. 
 *   ***************************************************************** 
 * 
 *  * Num_variables would be set here 
 *--------------------------------------------------------------------*/ 
int
USERD_get_number_of_variables( void )
{

#ifdef FAKE_A_VAR_IF_THERE_ARE_NONE
  if(!get_num_str_vars() && !get_num_unstr_vars())
    {
      printinfo("\nFAKING A SINGLE VARIABLE in USERD_get_number_of_variables!!!!!\n\n");
      Num_variables=1;
      return(Num_variables);
    }
#endif



  Num_variables = get_num_str_vars()+get_num_unstr_vars();
  printinfo("saf reader in USERD_get_number_of_variables=%d (get_num_str_vars()=%d get_num_unstr_vars()=%d)\n",
	 Num_variables,get_num_str_vars(),get_num_unstr_vars());
  return(Num_variables);
}


/*--------------------------------------------------------------------
 * USERD_get_gold_variable_info - 
 *-------------------------------------------------------------------- 
 * 
 *   Get the variable descriptions, types and filenames 
 * 
 *  (OUT) var_description      = Variable descriptions 
 * 
 *                               (Array will have been allocated 
 *                                Num_variables by Z_BUFL long) 
 * 
 *  (OUT) var_filename         = Variable real filenames 
 * 
 *                               (Array will have been allocated 
 *                                Num_variables by Z_BUFL long) 
 * 
 *  (OUT) var_type             = Variable type 
 * 
 *                               (Array will have been allocated 
 *                                Num_variables long) 
 * 
 *                               types are:  Z_CONSTANT 
 *                                           Z_SCALAR 
 *                                           Z_VECTOR 
 *                                           Z_TENSOR 
 *                                           Z_TENSOR9 
 * 
 *  (OUT) var_classify         = Variable classification 
 * 
 *                               (Array will have been allocated 
 *                                Num_variables long) 
 * 
 *                               types are:  Z_PER_NODE 
 *                                           Z_PER_ELEM 
 * 
 *  (OUT) var_complex          = TRUE if complex, FALSE otherwise 
 * 
 *                               (Array will have been allocated 
 *                                Num_variables long) 
 * 
 *  (OUT) var_ifilename        = Variable imaginary filenames (if complex) 
 * 
 *                               (Array will have been allocated 
 *                                Num_variables by Z_BUFL long) 
 * 
 *  (OUT) var_freq             = complex frequency  (if complex) 
 * 
 *                               (Array will have been allocated 
 *                                Num_variables long) 
 * 
 *  (OUT) var_contran          = TRUE if constant changes per time step 
 *                               FALSE if constant truly same at all time steps
 * 
 *                               (Array will have been allocated 
 *                                Num_variables long) 
 *
 *  (OUT) var_timeset          = Timeset the variable will use (1 based).
 *                               (For static models, set it to 1)
 *
 *                               (Array will have been allocated
 *                                Num_variables long)
 *
 *                              For example:  If USERD_get_number_of_timesets
 *                                            returns 2, the valid
 *                                            timeset_number's would be 1 or 2.
 *
 *  returns: Z_OK  if successful 
 *           Z_ERR if not successful 
 * 
 *  Notes: 
 *  * The implied variable numbers apply, but be aware that the 
 *    arrays are zero based. 
 *    So for variable 1, will need to provide   var_description[0] 
 *                                              var_filename[0] 
 *                                              var_type[0] 
 *                                              var_classify[0] 
 *                                              var_complex[0] 
 *                                              var_ifilename[0] 
 *                                              var_freq[0] 
 *                                              var_contran[0] 
 *                                              var_timeset[0]
 * 
 *       for variable 2, will need to provide   var_description[1] 
 *                                              var_filename[1] 
 *                                              var_type[1] 
 *                                              var_classify[1] 
 *                                              var_complex[1] 
 *                                              var_ifilename[1] 
 *                                              var_freq[1] 
 *                                              var_contran[1]
 *                                              var_timeset[1] 
 *             etc. 
 *--------------------------------------------------------------------*/ 
int
USERD_get_gold_variable_info(char **var_description,
                             char **var_filename,
                             int *var_type,
                             int *var_classify,
                             int *var_complex,
                             char **var_ifilename,
                             float *var_freq,
                             int *var_contran,
                             int *var_timeset)
{
  int i;




#ifdef USE_SOS_FOR_SAF
  /*dont need to do or prevent anything here. This function needs to be called whether the object
   is on this server or not.*/
#endif



  printinfo("\n*******BEGIN   USERD_get_gold_variable_info, Current_time_step=%d ***************\n",Current_time_step);


#ifdef FAKE_A_VAR_IF_THERE_ARE_NONE
  if(!get_num_str_vars() && !get_num_unstr_vars())
    {
      printinfo("\nFAKING A SINGLE VARIABLE in USERD_get_gold_variable_info (making a nodal var named \"fake_var\")!!!!!\n\n");
      snprintf(var_description[0],Z_BUFL-1,"fake_var");
      var_classify[0]=Z_PER_NODE;
      sprintf(var_filename[0],"none");
      var_type[0]=Z_SCALAR;
      var_complex[0]=FALSE;
      sprintf(var_ifilename[0],"none");
      var_freq[0]=0;
      var_contran[0]=TRUE;/* ?? */
      var_timeset[0]=1;

      return(Z_OK);
    }
#endif

  /*handle the structured vars*/
  for(i=0;i<get_num_str_vars();i++)
    {
      saf_str_mesh_var_type l_strMeshVarType = SAF_STR_MESH_UNKNOWN;

      get_str_var_name( i, var_description[i], Z_BUFL-1 );

      /*Get the var type, and check that all blocks have the same type
	or have no type at all for this variable index*/
      {
	int j,k,l_numValidTs=0;
	char *l_unitName=0,*l_quantName=0;
	for(j=0;j<get_num_blocks()/*Numparts_available*/;j++) 
	  {
	    for(k=0;k<Num_time_steps[Current_timeset];k++)
	      {
		const saf_str_mesh_var_type l_thisVarType = get_variable_type(j,i,k);
		if( l_strMeshVarType==SAF_STR_MESH_UNKNOWN ) 
		  {
		    l_strMeshVarType = l_thisVarType;
		  }
		else if( l_thisVarType!=SAF_STR_MESH_UNKNOWN && !are_similar_str_mesh_var_types(l_thisVarType,l_strMeshVarType) )
		  {
		    printinfo("error IN USERD_get_gold_variable_info: types not consistent for var %d of %d********\n",i,get_num_str_vars());
		    printinfo("  j=%d get_num_blocks()=%d Numparts_available=%d l_thisVarType=%d l_strMeshVarType=%d SAF_STR_MESH_UNKNOWN=%d\n",
			      j,get_num_blocks(),Numparts_available,l_thisVarType, l_strMeshVarType, SAF_STR_MESH_UNKNOWN );
		    printinfo("  k=%d Num_time_steps[%d]=%d\n",k,Current_timeset,Num_time_steps[Current_timeset]);

		    return( Z_ERR );
		  }
		if( l_thisVarType != SAF_STR_MESH_UNKNOWN ) l_numValidTs++;
	      }
	  }
	l_unitName = get_str_var_unit_name(i);
	l_quantName = get_str_var_quant_name(i);

	printinfo(".....var %d, units %s, quant %s, instances=%d of %d, \tdesc=%s\n",i, 
		  l_unitName ? l_unitName:"", 
		  l_quantName ? l_quantName:"", 
		  l_numValidTs, Numparts_available*Num_time_steps[Current_timeset], 
		  var_description[i] ? var_description[i]:"" );

	if(l_unitName) free(l_unitName);
	if(l_quantName) free(l_quantName);
      }

      if( l_strMeshVarType == SAF_STR_MESH_NODE )
	{
	  var_classify[i]=Z_PER_NODE;
	}
      else if( l_strMeshVarType == SAF_STR_MESH_UNKNOWN )
	{
	  var_classify[i]=Z_PER_NODE;/*no var to read....continuing*/
	}
      else if( l_strMeshVarType == SAF_STR_MESH_ELEM )
	{
	  var_classify[i]=Z_PER_ELEM;/*elems*/
	}
      else 
	{
	  var_classify[i]=Z_PER_NODE;/*edges,faces**************/
	}

      sprintf(var_filename[i],"none");
      var_type[i]=Z_SCALAR;
      var_complex[i]=FALSE;
      sprintf(var_ifilename[i],"none");
      var_freq[i]=0;
      var_contran[i]=TRUE;/* ?? */
      var_timeset[i]=1;
    }


  /*handle the UNstructured vars*/
  for(i=get_num_str_vars();i<Num_variables;i++)
    {
      int l_whichUnstrVar = i-get_num_str_vars();

      get_unstr_var_name( l_whichUnstrVar, var_description[i], Z_BUFL-1 );

      if( is_a_nodal_unstr_var(l_whichUnstrVar)  )
	var_classify[i]=Z_PER_NODE;
      else 
	var_classify[i]=Z_PER_ELEM;

      sprintf(var_filename[i],"none");


      if( is_this_unstr_var_scalar(l_whichUnstrVar) ) var_type[i]=Z_SCALAR;
      else if( is_this_unstr_var_vector(l_whichUnstrVar) ) var_type[i]=Z_VECTOR;
      else if( is_this_unstr_var_symtensor(l_whichUnstrVar) ) var_type[i]=Z_TENSOR;
      else if( is_this_unstr_var_tensor(l_whichUnstrVar) ) var_type[i]=Z_TENSOR9;
      else
	{
	  printinfo("got unknown var_type, using Z_SCALAR for now\n");
	  var_type[i]=Z_SCALAR;
	}

      /*
	if( var_type[i]==Z_SCALAR ) printinfo("Z_SCALAR\n");
	else if( var_type[i]==Z_VECTOR ) printinfo("Z_VECTOR\n");
	else if( var_type[i]==Z_TENSOR ) printinfo("Z_TENSOR\n");
	else if( var_type[i]==Z_TENSOR9 ) printinfo("Z_TENSOR9\n");
	else  printinfo("Z_unknown?\n");
      */

      var_complex[i]=FALSE;
      sprintf(var_ifilename[i],"none");
      var_freq[i]=0;
      var_contran[i]=TRUE;/* ?? */
      var_timeset[i]=1;
    }

  printinfo("*******END     USERD_get_gold_variable_info***************************\n\n");

  return(Z_OK);
}     



/*--------------------------------------------------------------------
 * USERD_get_var_by_component -  used by both unstructured parts and 
 *                               structured block parts. 
 *-------------------------------------------------------------------- 
 * 
 *  if Z_PER_NODE: 
 *    Get the component value at each node for a given variable in the part. 
 * 
 *  or if Z_PER_ELEM: 
 *    Get the component value at each element of a specific part and type for
 *    a given variable. 
 * 
 *  (IN)  which_variable          = The variable number 
 * 
 *  (IN)  which_part                Since EnSight Version 7.4
 *                                  -------------------------
 *                                = The part number 
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 * 
 *                                  Prior to EnSight Version 7.4
 *                                  ----------------------------
 *                                = The part id   This is the part_id label loaded
 *                                                in USERD_get_gold_part_build_info.
 *                                                It is NOT the part table index.
 *
 *  (IN)  var_type                = Z_SCALAR 
 *                                  Z_VECTOR 
 *                                  Z_TENSOR     ( symmetric tensor) 
 *                                  Z_TENSOR9    (asymmetric tensor) 
 * 
 *  (IN)  which_type 
 * 
 *           if Z_PER_NODE:         Not used 
 * 
 *           if Z_PER_ELEM:       = The element type 
 *                                  Z_POINT    node point element 
 *                                  Z_BAR02    2 node bar 
 *                                  Z_BAR03    3 node bar 
 *                                  Z_TRI03    3 node triangle 
 *                                  Z_TRI06    6 node triangle 
 *                                  Z_QUA04    4 node quad 
 *                                  Z_QUA08    8 node quad 
 *                                  Z_TET04    4 node tetrahedron 
 *                                  Z_TET10   10 node tetrahedron 
 *                                  Z_PYR05    5 node pyramid 
 *                                  Z_PYR13   13 node pyramid 
 *                                  Z_PEN06    6 node pentahedron 
 *                                  Z_PEN15   15 node pentahedron 
 *                                  Z_HEX08    8 node hexahedron 
 *                                  Z_HEX20   20 node hexahedron 
 * 
 *   Starting at API 2.01:
 *   ====================
 *                                  Z_G_POINT    ghost node point element
 *                                  Z_G_BAR02    2 node ghost bar
 *                                  Z_G_BAR03    3 node ghost bar
 *                                  Z_G_TRI03    3 node ghost triangle
 *                                  Z_G_TRI06    6 node ghost triangle
 *                                  Z_G_QUA04    4 node ghost quad
 *                                  Z_G_QUA08    8 node ghost quad
 *                                  Z_G_TET04    4 node ghost tetrahedron
 *                                  Z_G_TET10   10 node ghost tetrahedron
 *                                  Z_G_PYR05    5 node ghost pyramid
 *                                  Z_G_PYR13   13 node ghost pyramid
 *                                  Z_G_PEN06    6 node ghost pentahedron
 *                                  Z_G_PEN15   15 node ghost pentahedron
 *                                  Z_G_HEX08    8 node ghost hexahedron
 *                                  Z_G_HEX20   20 node ghost hexahedron
 *   Starting at API 2.02:
 *   ====================
 *                                  Z_NSIDED     n node nsided polygon
 *                                  Z_NFACED     n face nfaced polyhedron
 *                                  Z_G_NSIDED   n node ghost nsided polygon
 *                                  Z_G_NFACED   n face ghost nfaced polyhedron
 *
 *
 * 
 *  (IN)  imag_data               = TRUE if imag component 
 *                                  FALSE if real component 
 * 
 *  (IN)  component               = The component: (0       if Z_SCALAR) 
 *                                                 (0 - 2   if Z_VECTOR) 
 *                                                 (0 - 5   if Z_TENSOR) 
 *                                                 (0 - 8   if Z_TENSOR9) 
 * 
 *                                * 6 Symmetric Indicies, 0:5    * 
 *                                * ---------------------------- * 
 *                                *     | 11 12 13 |   | 0 3 4 | * 
 *                                *     |          |   |       | * 
 *                                * T = |    22 23 | = |   1 5 | * 
 *                                *     |          |   |       | * 
 *                                *     |       33 |   |     2 | * 
 * 
 *                                * 9 General   Indicies, 0:8    * 
 *                                * ---------------------------- * 
 *                                *     | 11 12 13 |   | 0 1 2 | * 
 *                                *     |          |   |       | * 
 *                                * T = | 21 22 23 | = | 3 4 5 | * 
 *                                *     |          |   |       | * 
 *                                *     | 31 32 33 |   | 6 7 8 | * 
 * 
 * (OUT) var_array 
 * 
 *     ----------------------------------------------------------------------- 
 *     (IMPORTANT: this array is 1-based for both Z_PER_NODE and Z_PER_ELEM!!!
 *     ----------------------------------------------------------------------- 
 * 
 *          if Z_PER_NODE:       = 1D array containing variable component value
 *                                  for each node.
 *
 *                                 (Array will have been allocated
 *                                  (number_of_nodes+1) long)
 *
 *              Info stored in this fashion:
 *                   var_array[0] = not used
 *                   var_array[1] = var component for node 1 of part
 *                   var_array[2] = var component for node 2 of part
 *                   var_array[3] = var component for node 3 of part
 *                   etc.
 *
 *           if Z_PER_ELEM:      = 1d array containing variable component value
 *                                 for each element of particular part & type.
 *
 *                            (Array will have been allocated
 *                             (number_of_elements[which_part][which_type] + 1)
 *                              long.  See USERD_get_gold_part_build_info)
 *
 *              Info stored in this fashion:
 *                   var_array[1] = var component for elem 1 (of part and type)
 *                   var_array[2] = var component for elem 2         "
 *                   var_array[3] = var conponent for elem 3         "
 *                   etc.
 *
 *  returns: Z_OK  if successful                                      
 *           Z_ERR if not successful                                  
 *
 *       or: Z_UNDEF if this variable is not defined on this part. In which
 *                   case you need not load anything into the var_array.
 *
 *  Notes:
 *  * This will be based on Current_time_step
 *
 *  * Not called unless Num_variables is > 0
 *
 *  * The per_node or per_elem classification must be obtainable from the
 *    variable number (a var_classify array needs to be retained)
 *
 *  * If the variable is not defined for this part, simply return with a
 *    value of Z_UNDEF.  EnSight will treat the variable as undefined for
 *    this part.
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_get_var_by_component(int which_variable,
                           int which_part,
                           int var_type,
                           int which_type,
                           int __UNUSED__ imag_data,
                           int component,
                           float *var_array)
{
  int l_isProbablyAPointVar=0;

  printinfo("   USERD_get_var_by_component bl=%d, var=%d, ts=%d \n", which_part-1, which_variable-1, Current_time_step );

#ifdef FAKE_A_VAR_IF_THERE_ARE_NONE
  if(!get_num_str_vars() && !get_num_unstr_vars())
    {
      printinfo("\nFAKING A SINGLE VARIABLE in USERD_get_var_by_component (not writing anything to var_array)!!!!!\n\n");
      return(Z_OK);
    }
#endif

  /*note: took out a bunch of checks on 14aug02, because am pretty sure ensight does them*/

  /*check that the saf str mesh type agrees with 'which_type'*/

  if( which_part-1 < get_num_blocks() )
    {
      /*THIS IS A STRUCTURED PART*/
      if( read_variable(which_part-1,which_variable-1,Current_time_step,&var_array[1])  )
	{
	  printinfo("\n   saf reader USERD_get_var_by_component, read_variable failed!!!!!!!!!!!!\n");
	  return(Z_UNDEF);
	}

      {
	char l_str[256];
	get_str_var_name( which_variable-1,l_str, 255 );
	printinfo("   ....saf reader successfully leaving USERD_get_var_by_component: %s\n",l_str);
      }

    }
  else
    {
      /*THIS IS AN UNSTRUCTURED PART*/
      int l_howManyEntriesIExpect=0;
      int l_whichUnstrPart=which_part-1-get_num_blocks();

      if( !is_there_an_unstr_var_instance(l_whichUnstrPart,which_variable-1,Current_time_step) )
	{
	  return(Z_UNDEF);
	}


      if( is_this_unstr_var_mapped_to_point_on_this_part(which_variable-1,l_whichUnstrPart) ||
	  (which_type==Z_TRI03 && is_this_unstr_var_mapped_to_tri_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_QUA04 && is_this_unstr_var_mapped_to_quad_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_TET04 && is_this_unstr_var_mapped_to_tet_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_HEX08 && is_this_unstr_var_mapped_to_hex_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_PYR05 && is_this_unstr_var_mapped_to_pyramid_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_PEN06 && is_this_unstr_var_mapped_to_prism_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_BAR02 && is_this_unstr_var_mapped_to_line_on_this_part(which_variable-1,l_whichUnstrPart)) ||

	  (which_type==Z_TRI06 && is_this_unstr_var_mapped_to_quadraticTri_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_QUA08 && is_this_unstr_var_mapped_to_quadraticQuad_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_TET10 && is_this_unstr_var_mapped_to_quadraticTet_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_HEX20 && is_this_unstr_var_mapped_to_quadraticHex_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_PYR13 && is_this_unstr_var_mapped_to_quadraticPyramid_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_PEN15 && is_this_unstr_var_mapped_to_quadraticPrism_on_this_part(which_variable-1,l_whichUnstrPart)) ||
	  (which_type==Z_BAR03 && is_this_unstr_var_mapped_to_quadraticLine_on_this_part(which_variable-1,l_whichUnstrPart)) ) 
	{
	  /*ok*/
	}
      else
	{
	  if(is_this_unstr_var_mapped_to_point(which_variable-1))
	    {
	      /*ok: this is the case when the variable is mapped to the node set, when
		the node set is different than the part set*/
	      l_isProbablyAPointVar=1;
	    }
	  else
	    {
	      return(Z_UNDEF);
	    }
	}



      if( l_isProbablyAPointVar ||
	  is_this_unstr_var_mapped_to_point_on_this_part(which_variable-1,l_whichUnstrPart) )/*need to change this to Z_PER_NODE somehow*/
	{
	  l_howManyEntriesIExpect = get_unstr_point_coords_used_by_part( l_whichUnstrPart, Current_time_step, 0 );

#ifdef LARRY1W_TEMP_FIX
	  if( !l_howManyEntriesIExpect && Current_time_step )
	    {
	      /*we have no points at this timestep: check if we have any points at all, maybe
		the points dont change with time*/
	      l_howManyEntriesIExpect = get_unstr_point_coords_used_by_part(l_whichUnstrPart,0,0);
	      if( l_howManyEntriesIExpect ) 
		{
		  /*I think this is wrong in larry1w: if pts are not time dependent, then
		    they shouldn't be in the suites at all, and should get time -1, not time 0*/
		  printinfo("   LARRY1W_TEMP_FIX: found no pts for this timestep, but found %d using time 0\n\n",l_howManyEntriesIExpect);
		}
	    }
#endif

	}
      else if( which_type==Z_POINT )
	{
	  l_howManyEntriesIExpect = get_num_3ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	  if(!l_howManyEntriesIExpect)
	    l_howManyEntriesIExpect = get_num_2ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	  if(!l_howManyEntriesIExpect)
	    l_howManyEntriesIExpect = get_num_1ball_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	  if(!l_howManyEntriesIExpect)
	    l_howManyEntriesIExpect = get_num_point_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
	}
      else if( which_type == Z_TRI03 ) 
	l_howManyEntriesIExpect = get_num_tri_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_QUA04 ) 
	l_howManyEntriesIExpect = get_num_quad_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_TET04 ) 
	l_howManyEntriesIExpect = get_num_tet_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_HEX08 ) 
	l_howManyEntriesIExpect = get_num_hex_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_PYR05 ) 
	l_howManyEntriesIExpect = get_num_pyramid_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_PEN06 ) 
	l_howManyEntriesIExpect = get_num_prism_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_BAR02 ) 
	l_howManyEntriesIExpect = get_num_line_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);

      else if( which_type == Z_TRI06 ) 
	l_howManyEntriesIExpect = get_num_quadraticTri_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_QUA08 ) 
	l_howManyEntriesIExpect = get_num_quadraticQuad_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_TET10 ) 
	l_howManyEntriesIExpect = get_num_quadraticTet_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_HEX20 ) 
	l_howManyEntriesIExpect = get_num_quadraticHex_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_PYR13 ) 
	l_howManyEntriesIExpect = get_num_quadraticPyramid_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_PEN15 ) 
	l_howManyEntriesIExpect = get_num_quadraticPrism_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);
      else if( which_type == Z_BAR03 ) 
	l_howManyEntriesIExpect = get_num_quadraticLine_elements_in_unstr_part(l_whichUnstrPart,Current_time_step);

      /*
	if( var_type==Z_SCALAR ) printinfo("Z_SCALAR\n");
	else if( var_type==Z_VECTOR ) printinfo("Z_VECTOR\n");
	else if( var_type==Z_TENSOR ) printinfo("Z_TENSOR\n");
	else if( var_type==Z_TENSOR9 ) printinfo("Z_TENSOR9\n");
	else  printinfo("Z_unknown?\n");
      */

      if( var_type==Z_SCALAR )
	{
	  if( read_unstr_scalar_variable(l_whichUnstrPart,which_variable-1,Current_time_step,&var_array[1],
					 l_howManyEntriesIExpect)  )
	    {
	      printinfo("   USERD_get_var_by_component, read_unstr_scalar_variable failed! which_type=%d Part=%d var=%d ts=%d\n",
			which_type,l_whichUnstrPart, which_variable-1, Current_time_step );
	      return(Z_UNDEF);
	    }

	  printinfo("   USERD_get_var_by_component, read_unstr_scalar_variable SUCCESS Part=%d var=%d ts=%d\n",
		    l_whichUnstrPart, which_variable-1, Current_time_step );
	}
      else if( var_type==Z_VECTOR )
	{
	  if( read_unstr_vector_variable(l_whichUnstrPart, which_variable-1, Current_time_step,
					 component, &var_array[1],
					 l_howManyEntriesIExpect)  )
	    {
	      printinfo("   USERD_get_var_by_component, read_unstr_vector_variable failed! which_type=%d Part=%d var=%d ts=%d\n",
			which_type,l_whichUnstrPart, which_variable-1, Current_time_step);
	      return(Z_UNDEF);
	    }

	  printinfo("   USERD_get_var_by_component, read_unstr_vector_variable SUCCESS Part=%d var=%d ts=%d\n",
		    l_whichUnstrPart, which_variable-1, Current_time_step);
	}
      else if( var_type==Z_TENSOR )
	{
	  if( read_unstr_symtensor_variable(l_whichUnstrPart, which_variable-1, Current_time_step,
					    component, &var_array[1],
					    l_howManyEntriesIExpect)  )
	    {
	      printinfo("   USERD_get_var_by_component, read_unstr_symtensor_variable failed! which_type=%d Part=%d var=%d ts=%d comp=%d\n",
			which_type,l_whichUnstrPart, which_variable-1, Current_time_step, component);
	      return(Z_UNDEF);
	    }

	  printinfo("   USERD_get_var_by_component, read_unstr_symtensor_variable SUCCESS Part=%d var=%d ts=%d comp=%d\n",
		    l_whichUnstrPart, which_variable-1, Current_time_step, component);
	}
      else if( var_type==Z_TENSOR9 )
	{
	  if( read_unstr_tensor_variable(l_whichUnstrPart, which_variable-1, Current_time_step,
					 component, &var_array[1],
					 l_howManyEntriesIExpect)  )
	    {
	      printinfo("   USERD_get_var_by_component, read_unstr_tensor_variable failed! which_type=%d Part=%d var=%d ts=%d comp=%d\n",
			which_type,l_whichUnstrPart, which_variable-1, Current_time_step, component);
	      return(Z_UNDEF);
	    }

	  printinfo("   USERD_get_var_by_component, read_unstr_tensor_variable SUCCESS Part=%d var=%d ts=%d comp=%d\n",
		    l_whichUnstrPart, which_variable-1, Current_time_step, component);
	}
    }


  return(Z_OK);
}


/*--------------------------------------------------------------------
 * USERD_get_constant_val - 
 *-------------------------------------------------------------------- 
 * 
 *   Get the value of a constant at a time step 
 * 
 *  (IN)  which_var            = The variable number 
 * 
 *  (IN)  imag_data            = TRUE if want imaginary data value. 
 *                               FALSE if want real data value. 
 * 
 *  returns: value of the requested constant variable 
 * 
 *  Notes: 
 *  * This will be based on Current_time_step 
 *--------------------------------------------------------------------*/ 
/*ARGSUSED*/
float
USERD_get_constant_val(int which_var, int __UNUSED__ imag_data)
{
  float constant_val;
  printinfo("saf reader in USERD_get_constant_val, arg which_var=%d\n",which_var);

  constant_val = 0.0;
  return(constant_val);
}



/*--------------------------------------------------------------------
 * USERD_get_var_value_at_specific - 
 *-------------------------------------------------------------------- 
 * 
 *  if Z_PER_NODE: 
 *    Get the value of a particular variable at a particular node in a 
 *    particular part at a particular time. 
 * 
 *  or if Z_PER_ELEM: 
 *    Get the value of a particular variable at a particular element of 
 *    a particular type in a particular part at a particular time. 
 * 
 *  (IN)  which_var   = Which variable 
 * 
 *  (IN)  which_node_or_elem 
 * 
 *                     If Z_PER_NODE: 
 *                       = The node number.  This is not the id, but is 
 *                                           the index of the node 
 *                                           list (1 based), or the block's 
 *                                           node list (1 based). 
 * 
 *                           Thus,  coord_array[1] 
 *                                  coord_array[2] 
 *                                  coord_array[3] 
 *                                       .      | 
 *                                       .      |which_node_or_elem index 
 *                                       .             ---- 
 * 
 * 
 *                     If Z_PER_ELEM: 
 *                       = The element number. This is not the id, but is 
 *                                             the element number index 
 *                                             of the number_of_element array 
 *                                        (see USERD_get_gold_part_build_info),
 *                                             or the block's element list
 *                                             (1 based).
 *
 *                           Thus,  for which_part:
 *                                  conn_array[which_elem_type][0]
 *                                  conn_array[which_elem_type][1]
 *                                  conn_array[which_elem_type][2]
 *                                       .                      |
 *                                       .          which_node_or_elem index
 *                                       .                        ----
 *
 *
 *  (IN)  which_part                Since EnSight Version 7.4
 *                                  -------------------------
 *                                = The part number 
 *
 *                                  (1-based index of part table, namely:
 *
 *                                     1 ... Numparts_available.
 *
 *                                   It is NOT the part_id that
 *                                   is loaded in USERD_get_gold_part_build_info)
 * 
 *                                  Prior to EnSight Version 7.4
 *                                  ----------------------------
 *                                = The part id   This is the part_id label loaded
 *                                                in USERD_get_gold_part_build_info.
 *                                                It is NOT the part table index.
 *
 *  (IN)  which_elem_type
 *
 *                       If Z_PER_NODE, or block part:
 *                         = Not used
 *
 *                       If Z_PER_ELEM:
 *                         = The element type.  This is the element type index
 *                                              of the number_of_element array
 *                                         (see USERD_get_gold_part_build_info)
 *                                                                    
 *  (IN)  time_step   = Time step to use                              
 *                                                                    
 *  (IN)  imag_data   = TRUE if want imaginary data file.
 *                      FALSE if want real data file.     
 *                                                                    
 *  (OUT) values      = scalar or vector component value(s)           
 *                       values[0] = scalar or vector[0]              
 *                       values[1] = vector[1]                        
 *                       values[2] = vector[2]                        
 *                                                                    
 *  returns: Z_OK  if successful                                      
 *           Z_ERR if not successful                                  
 *           Z_NOT_IMPLEMENTED if not implemented and want to use the slower,
 *                             complete update method within EnSight.
 *                                                                    
 *  Notes:                                                            
 *  * This routine is used in node querys over time (or element querys over
 *    time for Z_PER_ELEM variables).  If these operations are not critical
 *    to you, this can be a dummy routine.
 *
 *  * The per_node or per_elem classification must be obtainable from the
 *    variable number (a var_classify array needs to be retained)  
 *
 *  * The time step given is for the proper variable timeset, so it must
 *    be obtainable from the variable number as well.
 *--------------------------------------------------------------------*/
/*ARGSUSED*/
int
USERD_get_var_value_at_specific(int __UNUSED__ which_var,
                                int __UNUSED__ which_node_or_elem,
                                int __UNUSED__ which_part,
                                int __UNUSED__ which_elem_type,
                                int __UNUSED__ time_step,
                                float __UNUSED__ values[3],
                                int __UNUSED__ imag_data)
{
  return(Z_NOT_IMPLEMENTED);
}


/*--------------------------------------------------------------------
 * USERD_stop_part_building - 
 *-------------------------------------------------------------------- 
 * 
 *   Called when part builder is closed for USERD, can be used to clean 
 *   up memory, etc that was only needed during the part building process. 
 *--------------------------------------------------------------------*/ 
void 
USERD_stop_part_building( void )
{
  printinfo("saf reader in USERD_stop_part_building\n");
  return;
}


/*--------------------------------------------------------------------
 * USERD_bkup - 
 *-------------------------------------------------------------------- 
 * 
 *   Used in the archive process.  Save or restore info relating to 
 *   your user defined reader. 
 * 
 *  (IN)  archive_file         = The archive file pointer 
 * 
 *  (IN)  backup_type          = Z_SAVE_ARCHIVE for saving archive 
 *                               Z_REST_ARCHIVE for restoring archive 
 * 
 *  returns: Z_OK  if successful 
 *           Z_ERR if not successful 
 * 
 *  Notes: 
 *  * Since EnSight's archive file is saved in binary form, it is 
 *    suggested that you also do any writing to it or reading from it 
 *    in binary. 
 * 
 *  * You should archive any variables, that will be needed for 
 *    future operations, that will not be read or computed again 
 *    before they will be needed.  These are typically global 
 *    variables. 
 * 
 *  * Make sure that the number of bytes that you write on a save and 
 *    the number of bytes that you read on a restore are identical!! 
 * 
 *  * And one last reminder.  If any of the variables you save are 
 *    allocated arrays, you must do the allocations before restoring 
 *    into them. 
 *--------------------------------------------------------------------*/ 
/*ARGSUSED*/
int
USERD_bkup(FILE __UNUSED__ *archive_file,
           int __UNUSED__ backup_type)
{
  printinfo("saf reader in USERD_bkup\n");
  return(Z_OK);
}

/*--------------------------------------------------------------------
 * USERD_exit_routine
 *--------------------------------------------------------------------
 *
 *   Called when EnSight is exited for USERD, can be used to
 *   clean up temporary files, etc. It is often simply a dummy.
 *--------------------------------------------------------------------*/
void
USERD_exit_routine( void )
{


  printinfo("\nsaf reader in USERD_exit_routine\n");
 


#ifdef RETAIN_COORDS_IN_MEMORY
  if( g_xCoordsPerPartPerTimestep )
    {
      int i,j;
      for(i=0;i<get_num_timesteps();i++)
	{
	  if( g_xCoordsPerPartPerTimestep[i] )
	    {
	      for(j=0;j<Numparts_available;j++)
		{
		  if( g_xCoordsPerPartPerTimestep[i][j] )
		    {
		      free(g_xCoordsPerPartPerTimestep[i][j]);
		      free(g_yCoordsPerPartPerTimestep[i][j]);
		      free(g_zCoordsPerPartPerTimestep[i][j]);
		      g_xCoordsPerPartPerTimestep[i][j]=0;
		      g_yCoordsPerPartPerTimestep[i][j]=0;
		      g_zCoordsPerPartPerTimestep[i][j]=0;
		    }
		}

	      free(g_xCoordsPerPartPerTimestep[i]);
	      free(g_yCoordsPerPartPerTimestep[i]);
	      free(g_zCoordsPerPartPerTimestep[i]);
	      free(g_nodesPerPartPerTimestep[i]);

	      g_xCoordsPerPartPerTimestep[i]=0;
	      g_yCoordsPerPartPerTimestep[i]=0;
	      g_zCoordsPerPartPerTimestep[i]=0;
	      g_nodesPerPartPerTimestep[i]=0;
	    }
	}
      free(g_xCoordsPerPartPerTimestep);
      free(g_yCoordsPerPartPerTimestep);
      free(g_zCoordsPerPartPerTimestep);
      free(g_nodesPerPartPerTimestep);
      g_xCoordsPerPartPerTimestep=0;
      g_yCoordsPerPartPerTimestep=0;
      g_zCoordsPerPartPerTimestep=0;
      g_nodesPerPartPerTimestep=0;

      g_bytesCostToRetainInfo=0;
    }
#endif


#ifdef USE_SOS_FOR_SAF
  if(g_objectIsValidInSOS)
    {
      free(g_objectIsValidInSOS);
      g_objectIsValidInSOS=NULL;
    }
#endif


  printinfo("saf reader done freeing memory in USERD_exit_routine\n"); 



  finalize_unstr_mesh_reader();


  finalize_str_mesh_reader(1);

 

  printinfo("saf reader leaving USERD_exit_routine\n"); 

  return;
}


/*-------------------- End of libuserd.c --------------------*/

