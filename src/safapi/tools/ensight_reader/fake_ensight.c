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
 * Chapter:     Ensight Reader Tester (fake_ensight)
 * Description:
 *
 * This program calls the same reader functions (in the same order) that Ensight would
 * call when using the SAF reader. The ordering is APPROXIMATELY as follows:
 *
 * get_reader_name
 *
 * get_str_mesh_reader_version
 *
 * init_str_mesh_reader
 *
 * init_unstr_mesh_reader
 *
 * get_num_timesteps
 *
 * (for each timestep) get_time_value_for_timestep
 *
 * get_num_unstr_parts
 *
 * get_num_blocks
 *
 * (for each timestep)
 *
 *    (for each structured block)
 *
 *        get_block_name
 *
 *        get_block_size
 *
 *    (for each unstructured part)
 *
 *        get_unstr_part_name
 *
 *        get_unstr_point_coords_used_by_part
 *
 *        get_num_*_elements_in_unstr_part
 *
 *    get_num_str_vars
 *
 *    get_num_unstr_vars
 *
 *    (for each structured variable)
 *
 *        get_str_var_name
 *
 *        get_variable_type
 *
 *        get_str_var_unit_name
 *
 *        get_str_var_quant_name
 *
 *    (for each unstructured variable)
 *
 *        get_unstr_var_name
 *
 *        is_a_nodal_unstr_var
 *
 *        is_this_unstr_var_*
 *
 *    (for each structured block)
 *
 *        read_block_coords_as_curvilinear
 *
 *    (for each unstructured part)
 *
 *        (in various orders) 
 *
 *            get_unstr_point_coords_used_by_part
 *
 *            get_num_*_elements_in_unstr_part
 *
 *            get_*_element_connectivity
 *
 *    (for each structured variable)
 *
 *        get_variable_type
 *
 *        read_variable
 *
 *        get_str_var_name 
 * 
 *    (for each unstructured variable)
 *
 *        is_this_unstr_var_*
 *
 *        is_there_an_unstr_var_instance
 *
 *        is_this_unstr_var_mapped_to_*
 *
 *        get_unstr_point_coords_used_by_part
 *
 *        get_num_*_elements_in_unstr_part
 *
 *        read_unstr_*_variable
 *
 * finalize_unstr_mesh_reader
 *
 * finalize_str_mesh_reader
 *
 *--------------------------------------------------------------------------------------------------- */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "global_extern.h" 
                           
#include "str_mesh_reader.h"
#include "unstr_mesh_reader.h"

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif



/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Ensight Reader Tester (fake_ensight)
 * Purpose:     Global Variables
 * Description:
 *
 * These are global variables.
 *
 *--------------------------------------------------------------------------------------------------- */

#define LARRY1W_TEMP_FIX
/*  #define FAKE_A_VAR_IF_THERE_ARE_NONE*/ /*to fix an unexplained ensight bug?*/







/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     
 * Purpose:     Prototypes
 * Description: Ensight Reader Tester (fake_ensight)
 *
 * These are prototypes.
 *
 *--------------------------------------------------------------------------------------------------- */
const char *get_name_of_ensight_celltype(int which_type);


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Ensight Reader Tester (fake_ensight)
 * Purpose:     Get the Name of an Ensight Cell Type
 * Description: Returns a string corresponding to the given Ensight cell type, e.g. "Z_POINT" or "Z_TRI03".
 *--------------------------------------------------------------------------------------------------- */
const char *get_name_of_ensight_celltype(int which_type)
{
  if( which_type==Z_POINT ) return("Z_POINT");
  else if( which_type==Z_TRI03 ) return("Z_TRI03");
  else if( which_type==Z_QUA04 ) return("Z_QUA04");
  else if( which_type==Z_TET04 ) return("Z_TET04");
  else if( which_type==Z_PYR05 ) return("Z_PYR05");
  else if( which_type==Z_HEX08 ) return("Z_HEX08");
  else if( which_type==Z_PEN06 ) return("Z_PEN06");
  else if( which_type==Z_BAR02 ) return("Z_BAR02");

        else if(which_type==Z_BAR03) return("Z_BAR03");
        else if(which_type==Z_TRI06) return("Z_TRI06");
        else if(which_type==Z_QUA08) return("Z_QUA08");
        else if(which_type==Z_TET10) return("Z_TET10");
        else if(which_type==Z_PYR13) return("Z_PYR13");
        else if(which_type==Z_PEN15) return("Z_PEN15");
        else if(which_type==Z_HEX20) return("Z_HEX20");

  else return("Z_unknown?");
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Ensight Reader Tester (fake_ensight)
 * Purpose:     The main function.
 * Description: The main function.
 *--------------------------------------------------------------------------------------------------- */

int main( int argc, char **argv )
{
  int l_short=0;
  char *l_filename=0;
  int l_numTimesteps = 0, l_numVariables=0;
  int l_Numparts_available=0,which_variable=0;
  int i,j,k,ret;
  int **l_number_of_elements=0;
  int *number_of_nodes=0;
  int **ijk_dimensions=0;
  int l_timestep=0;
  int rank=-1;
  int nproc=-1;


  /*set the environment variable: otherwise, printinfo wont print*/
  char *l_envVar = (char *)malloc(256*sizeof(char));
  sprintf(l_envVar,"SAF_READER_DEBUG=true");
#ifndef JANUS
  putenv(l_envVar); 
#endif
  free(l_envVar);


#if 0
  init_str_mesh_reader("../create_unstr_mesh/test_unstr_mesh.saf");
  init_unstr_mesh_reader(0);
  finalize_unstr_mesh_reader();

  printf("\njake finished test in fake_ensight\n\n");
  exit(0);
#endif


#ifdef HAVE_PARALLEL
  MPI_Init(&argc,&argv);
#endif

  initialize_str_globals();/*must do this before printinfo_set_filename, or the latter will be overwritten*/
  printinfo_set_filename(NULL);

  if( argc == 3 )
    { /*XXX todo: make this command-line hack better*/
      if(!strcmp(argv[1],"-sh")) l_short=1;
      l_filename = argv[2];
    }
  else if( argc == 2 )
    {
      l_filename = argv[1];
    }
  else
    {
      l_filename= ens_strdup("../create_unstr_mesh/test_unstr_mesh.saf");
    }


  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  printinfo("\nfake_ensight opening file: %s\n",l_filename);



#ifdef HAVE_PARALLEL
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nproc);
  printinfo("Hello from proc %d of %d\n",rank,nproc);
#endif

  {
    char reader_name[Z_MAX_USERD_NAME];
    int two_fields;
    ret = USERD_get_name_of_reader(reader_name,&two_fields);
    if(ret!=Z_OK)
      {
	printinfo("fake_ensight fails\n");
	return(-1);
      }
    printinfo("USERD_get_name_of_reader: %s\n",reader_name);
  }

  {
    char descrip[Z_MAXFILENP];
    ret = USERD_get_reader_descrip(descrip);
    if(ret!=Z_OK)
      {
	printinfo("fake_ensight fails\n");
	return(-1);
      }
  }


  {
    char version_number[Z_MAX_USERD_NAME];
    ret = USERD_get_reader_version(version_number);
    if(ret!=Z_OK)
      {
	printinfo("fake_ensight fails\n");
	return(-1);
      }
  }

  {
    char version_number[Z_MAX_USERD_NAME];
    ret = USERD_get_reader_release(version_number);
    if(ret!=Z_OK)
      {
	printinfo("fake_ensight fails\n");
	return(-1);
      }
  }


  {
    int swapbytes=1;
    char *the_path=ens_strdup("");
    char *filename_2=0;
    ret = USERD_set_filenames(l_filename, filename_2, the_path, swapbytes);
    if(ret!=Z_OK)
      {
	printinfo("fake_ensight fails\n");
	return(-1);
      } 
    free(the_path); 
  }


  USERD_get_number_of_timesets( );
  USERD_get_geom_timeset_number( );

  {
    char timeset_description[Z_BUFL];
    int timeset_number=1;
    ret = USERD_get_timeset_description(timeset_number,timeset_description);
    if(ret!=Z_OK)
      {
	printinfo("fake_ensight fails\n");
	return(-1);
      }
  }

  {
    int timeset_number=1;
    l_numTimesteps = USERD_get_num_of_time_steps( timeset_number );
  }

  {
    int timeset_number=1;
    float *solution_times = (float *)malloc( l_numTimesteps * sizeof(float) );
    ret = USERD_get_sol_times( timeset_number, solution_times );
    if(ret!=Z_OK)
      {
	printinfo("fake_ensight fails\n");
	return(-1);
      }
    free(solution_times);
  }

  USERD_set_time_set_and_step(1,0);
  USERD_get_changing_geometry_status( );


  {
    char version_number[Z_MAX_USERD_NAME];
    ret = USERD_get_reader_version(version_number);
    if(ret!=Z_OK)
      {
	printinfo("fake_ensight fails\n");
	return(-1);
      }
  }

  USERD_get_node_label_status();
  USERD_get_element_label_status();
  USERD_get_number_of_files_in_dataset();
  

  l_Numparts_available = USERD_get_number_of_model_parts();/*i.e. num unstr parts + num str blocks,
							     not dependent on Current_time_step*/




#if 0
  printinfo("warning: only doing initialization, leaving fake_ensight early\n");
  if(0)
#endif


    /* 
    ** from here on is dependent on Current_time_step
    */
    for( l_timestep=0; l_timestep<l_numTimesteps; l_timestep++ )
      {
	char **var_description=0;



	if(l_short && l_timestep)
	  {
	    printinfo("warning: only doing first timestep, leaving fake_ensight early\n");
	    break;
	  }



	printinfo("\n\n===========================================================================\n");
	printinfo("============STARTING TIME %d of %d   =============================\n",l_timestep,l_numTimesteps);
	printinfo("===========================================================================\n");


	/*
	** Note: ensight seems to call USERD_set_time_set_and_step over and over.
	** Since it does little, I only call it once per time change.
	*/
	USERD_set_time_set_and_step(1,l_timestep);

	if( l_Numparts_available )
	  {
	    int *part_id = (int *)malloc(l_Numparts_available*sizeof(int));
	    int *part_types = (int *)malloc(l_Numparts_available*sizeof(int));
	    char **part_description = (char **)malloc(l_Numparts_available*sizeof(char *));
	    int **iblanking_options = (int **)malloc(l_Numparts_available*sizeof(int *));
	    l_number_of_elements = (int **)malloc(l_Numparts_available*sizeof(int *));
	    number_of_nodes = (int *)malloc(l_Numparts_available*sizeof(int));
	    ijk_dimensions = (int **)malloc(l_Numparts_available*sizeof(int *));

	    if( !part_id || !part_types || !part_description || !ijk_dimensions || !iblanking_options
		|| !l_number_of_elements || !number_of_nodes )
	      {
		printinfo("fake_ensight FAILED TO ALLOC 1\n");
		return(-1);
	      }

	    for(i=0;i<l_Numparts_available;i++) 
	      {

		part_description[i]=(char *)malloc(Z_BUFL*sizeof(char));
		l_number_of_elements[i]=(int *)malloc(Z_MAXTYPE*sizeof(int));
		ijk_dimensions[i]=(int *)malloc(3*sizeof(int));
		ijk_dimensions[i][0]=ijk_dimensions[i][1]=ijk_dimensions[i][2]=0;
		iblanking_options[i]=(int *)malloc(6*sizeof(int));

		if( !part_description[i] || !l_number_of_elements[i] || !ijk_dimensions[i]
		    || !iblanking_options[i] )
		  {
		    printinfo("fake_ensight FAILED TO ALLOC 2\n");
		    return(-1);
		  }
	      }

	    ret = USERD_get_gold_part_build_info(part_id,
						 part_types,
						 part_description,
						 number_of_nodes,
						 l_number_of_elements,
						 ijk_dimensions,
						 iblanking_options );


	    /*free memory*/
	    for(i=0;i<l_Numparts_available;i++) 
	      {
		free( part_description[i] );
		free( iblanking_options[i] );
	      }
	    free( part_id  );
	    free( part_types  );
	    free( part_description  );
	    free( iblanking_options  );
	  }

	USERD_get_ghosts_in_model_flag();


	USERD_get_maxsize_info(0,0,0);/*note: will cause error if actually tries to give info*/
	USERD_get_model_extents( 0 );/*note: will cause error if actually tries to give info*/


	l_numVariables = USERD_get_number_of_variables( );




 




	if(l_numVariables)
	  {
	    char **var_filename = (char **)malloc(l_numVariables*sizeof(char *));
	    char **var_ifilename = (char **)malloc(l_numVariables*sizeof(char *));
	    int *var_type = (int *)malloc(l_numVariables*sizeof(int));
	    int *var_classify = (int *)malloc(l_numVariables*sizeof(int));
	    int *var_complex = (int *)malloc(l_numVariables*sizeof(int));
	    int *var_contran = (int *)malloc(l_numVariables*sizeof(int));
	    int *var_timeset = (int *)malloc(l_numVariables*sizeof(int));
	    float *var_freq = (float *)malloc(l_numVariables*sizeof(int));
	    var_description = (char **)malloc(l_numVariables*sizeof(char *));


	    if( !var_description || !var_filename || !var_ifilename || !var_type ||
		!var_classify || !var_complex || !var_contran || !var_timeset || !var_freq )
	      {
		printinfo("fake_ensight FAILED TO ALLOC 3\n");
		return(-1);
	      }

	    for(i=0;i<l_numVariables;i++)
	      {
		var_description[i] = (char *)malloc(Z_BUFL*sizeof(char));
		var_filename[i] = (char *)malloc(Z_BUFL*sizeof(char));
		var_ifilename[i] = (char *)malloc(Z_BUFL*sizeof(char));
		var_description[i][0] = '\0';
		var_filename[i][0] = '\0';
		var_ifilename[i][0] = '\0';


		if( !var_description[i] || !var_filename[i] || !var_ifilename[i] )
		  {
		    printinfo("fake_ensight FAILED TO ALLOC 4\n");
		    return(-1);
		  }

	      }

	    ret = USERD_get_gold_variable_info(var_description,
					       var_filename,
					       var_type,
					       var_classify,
					       var_complex,
					       var_ifilename,
					       var_freq,
					       var_contran,
					       var_timeset);
	    if(ret!=Z_OK)
	      {
		printinfo("fake_ensight fails\n");
		return(-1);
	      }

	    /*free memory*/
	    for(i=0;i<l_numVariables;i++)
	      {
		free( var_filename[i] );
		free( var_ifilename[i] );
	      }


	    free(var_filename );
	    free(var_ifilename );
	    free(var_type );
	    free(var_classify );
	    free(var_complex );
	    free(var_contran );
	    free(var_timeset );
	    free(var_freq );


	  }






	printinfo("===================================================\n");

	for(i=1;i<=l_Numparts_available;i++)
	  {
	    if( i<=get_num_blocks() )
	      { /*Handle reading structured block coords*/
		/* Note that I had overlooked reading in the structured coords for a long
		   time, until 7aug2003. I put them here, but this may not be exactly the 
		   order that Ensight would place them. It most likely doesnt matter. */

		int l_x,l_y,l_z;
		float *l_data=0;

		get_block_size( i-1, &l_x, &l_y, &l_z );
		    
		printinfo("==================================================================================\n");
		printinfo("HANDLING structured block=%d of %d  size=%dx%dx%d  time=%d\n",
			  i-1,l_Numparts_available,l_x,l_y,l_z,l_timestep);
		printinfo("==================================================================================\n");

		l_data = (float *)malloc(l_x*l_y*l_z*sizeof(float));	      
		if(!l_data)
		  {
		    printinfo("fake_ensight fails: failed to alloc l_data for structured coords\n");
		    return(-1);
		  }

		read_block_coords_as_curvilinear(i-1,l_data,NULL,NULL);

		printinfo("X Coords:");
		for(j=0;j<l_x*l_y*l_z;j++)
		  {
		    printinfo(" %f",l_data[j]);
		    if(j>5) break;
		  }
		printinfo(" ...\n");

		read_block_coords_as_curvilinear(i-1,NULL,l_data,NULL);

		printinfo("Y Coords:");
		for(j=0;j<l_x*l_y*l_z;j++)
		  {
		    printinfo(" %f",l_data[j]);
		    if(j>5) break;
		  }
		printinfo(" ...\n");

		read_block_coords_as_curvilinear(i-1,NULL,NULL,l_data);

		printinfo("Z Coords:");
		for(j=0;j<l_x*l_y*l_z;j++)
		  {
		    printinfo(" %f",l_data[j]);
		    if(j>5) break;
		  }
		printinfo(" ...\n");


		free(l_data);
	      }
	    else for(j=0;j<Z_MAXTYPE;j++)
	      { /*Handle reading unstructured part coords*/
		if( l_number_of_elements[i-1][j] )
		  {
		    int l_numInCell=0;
		    int **conn_array = (int **)malloc( l_number_of_elements[i-1][j] * sizeof(int *) );
		    char l_partName[256];
		    int l_whichUnstrPart = i-get_num_blocks()-1;

		    get_unstr_part_name( l_whichUnstrPart, l_partName, 255 );


		    if(!conn_array)
		      {
			printinfo("fake_ensight fails: failed to alloc conn_array\n");
			return(-1);
		      }
	      
		    /*there should be a fn to do this?*/
		    if( j==Z_POINT ) l_numInCell=1;
		    else if( j==Z_TRI03 ) l_numInCell=3;
		    else if( j==Z_PYR05 ) l_numInCell=5;
		    else if( j==Z_QUA04 ) l_numInCell=4;
		    else if( j==Z_TET04 ) l_numInCell=4;
		    else if( j==Z_HEX08 ) l_numInCell=8;
		    else if( j==Z_PEN06 ) l_numInCell=6;
		    else if( j==Z_BAR02 ) l_numInCell=2;

		    else if( j==Z_TRI06 ) l_numInCell=6;
		    else if( j==Z_PYR13 ) l_numInCell=13;
		    else if( j==Z_QUA08 ) l_numInCell=8;
		    else if( j==Z_TET10 ) l_numInCell=10;
		    else if( j==Z_HEX20 ) l_numInCell=20;
		    else if( j==Z_PEN15 ) l_numInCell=15;
		    else if( j==Z_BAR03 ) l_numInCell=3;
		    else
		      {
			printinfo("fake_ensight fails: got an unknown celltype=%d\n",j);
			return(-1);
		      }

		    printinfo("==================================================================================\n");
		    printinfo("HANDLING unstructured part=%s(%d of %d)  Z_TYPE=%d(%s) of %d  num_elems=%d l_numInCell=%d  num_nodes=%d  time=%d\n",
			      l_partName,i-1,l_Numparts_available,j,get_name_of_ensight_celltype(j),Z_MAXTYPE,
			      l_number_of_elements[i-1][j],l_numInCell,number_of_nodes[i-1],l_timestep);
		    printinfo("==================================================================================\n");


		    for(k=0;k<l_number_of_elements[i-1][j];k++) 
		      {
			conn_array[k]=(int *)malloc(l_numInCell*sizeof(int));
			if(!conn_array[k])
			  {
			    printinfo("fake_ensight fails: failed to alloc conn_array[%d] to %d bytes\n",k,(int)(l_numInCell*sizeof(int)));
			    return(-1);
			  }
		      }

 
		    ret = USERD_get_part_elements_by_type(i,j,conn_array);
		    if(ret!=Z_OK)
		      {
			printinfo("fake_ensight fails, celltype=%d, num=%d\n",j,l_number_of_elements[i-1][j]);
			return(-1);
		      }

		    if( number_of_nodes[i-1] )
		      {
			float **coord_array = NULL;
			coord_array = (float **)malloc(3*sizeof(float *));

			if(!coord_array)
			  {
			    printinfo("error fake_ensight failed to alloc coord_array\n");
			    return(-1);
			  }
			for(k=0;k<3;k++) 
			  {
			    coord_array[k] = (float *)malloc( (number_of_nodes[i-1]+1)*sizeof(float) );
			    if(!coord_array[k])
			      {
				printinfo("error fake_ensight fails: failed to alloc coord_array[%d] to %d bytes\n",
					  k,(int)( (number_of_nodes[i-1]+1) *sizeof(float)) );
				return(-1);
			      }
			  }



			/*XXX this is called here more often than ensight calls it, why?*/
			ret = USERD_get_part_coords(i, coord_array);
			if(ret!=Z_OK)
			  {
			    printinfo("error fake_ensight fails in USERD_get_part_coords, celltype=%d, num=%d\n",j,l_number_of_elements[i-1][j]);
			    return(-1);
			  }


#if 1
			/*calc and print the min-max of the values*/
			{
			  float l_min,l_max;
			  for( k=0; k<3; k++ )
			    {
			      int kk;
			      l_min = l_max = coord_array[k][1];
			      for( kk=2; kk<number_of_nodes[i-1]+1; kk++ )
				{
				  if(l_min>coord_array[k][kk]) l_min = coord_array[k][kk];
				  if(l_max<coord_array[k][kk]) l_max = coord_array[k][kk];
				}
			      printinfo("      coord_array[%d] min=%f max=%f\n",k,l_min,l_max);
			    }
			}
#endif

			for(k=0;k<3;k++) free(coord_array[k]);
			free(coord_array);
		      }




		    for(k=0;k<l_number_of_elements[i-1][j];k++) free(conn_array[k]);
		    free(conn_array);
		  }
	      }
	  }


	USERD_stop_part_building( );


	USERD_get_descrip_lines(0,0,0,0,0);/*will be an error if fn does anything*/



	printinfo("===================================================\n");








	for( which_variable=1; which_variable<=l_numVariables; which_variable++ )
	  {
	    int imag_data = FALSE;
	    int component = 0;
	    int which_type=0;
	    int var_type = Z_SCALAR;
	    int l_whichUnstrVar = which_variable-1-getMaxNumStrVars();
	    int l_numEntries=0;

	    if( l_whichUnstrVar < 0 )
	      {
		/*this is a structured var*/
		printinfo("fake_ensight only partially written for structured vars, here goes:\n");

		for(i=1;i<=l_Numparts_available;i++)
		  {
		    saf_str_mesh_var_type l_strVarType;

		    if( i-1 >= get_num_blocks() )
		      {
			/*the structured blocks are listed first. This must be unstructured, so quit*/
			break;
		      }

		    l_strVarType = get_variable_type(i-1,which_variable-1,l_timestep);
		    if( l_strVarType == SAF_STR_MESH_ELEM )
		      {
			if( ijk_dimensions[i-1][0]>0 &&
			    ijk_dimensions[i-1][1]>0 &&
			    ijk_dimensions[i-1][2]>0 )
			  {
			    l_numEntries = (ijk_dimensions[i-1][0]-1)*(ijk_dimensions[i-1][1]-1)*(ijk_dimensions[i-1][2]-1);
			  }
			else l_numEntries=0;
		      }
		    else if( l_strVarType == SAF_STR_MESH_UNKNOWN )
		      {
			l_numEntries=0;
		      }
		    else
		      {
			/*all other types are mapped to points*/
			l_numEntries = ijk_dimensions[i-1][0]*ijk_dimensions[i-1][1]*ijk_dimensions[i-1][2];
		      }

		    if( l_numEntries<0 )
		      {
			printinfo("fake_ensight error in str mesh area: l_numEntries=%d\n",l_numEntries);
			return(-1);
		      }

		    if( l_numEntries )
		      {
			/*note the +1: var_array ignores the first entry*/
			float *var_array = (float *)malloc((l_numEntries+1)*sizeof(float));
			if(!var_array)
			  {
			    printinfo("fake_ensight fails: failed to alloc var_array for reading str var\n");
			    return(-1);
			  }


			ret = USERD_get_var_by_component( which_variable,
							  i,
							  var_type,
							  which_type,
							  imag_data,
							  component,
							  var_array );		    
			if(ret==Z_ERR)
			  {
			    printinfo("fake_ensight fails reading str var\n");
			    return(-1);
			  }

#if 1
			/*calc and print the min-max of the values*/
			{
			  float l_min,l_max;
			  l_min = l_max = var_array[1];
			  for( k=2; k<l_numEntries+1; k++ )
			    {
			      if(l_min>var_array[k]) l_min = var_array[k];
			      if(l_max<var_array[k]) l_max = var_array[k];
			    }
			  printinfo("      structured var var_array (%d entries)(%d %d %d) min=%f max=%f\n",
				    l_numEntries,ijk_dimensions[i-1][0],ijk_dimensions[i-1][1],ijk_dimensions[i-1][2],l_min,l_max);
			}
#endif
	
			free(var_array);
		      }
		  }	    
		continue;/*finished reading this str var*/
	      }


	    if( is_this_unstr_var_scalar(l_whichUnstrVar) )
	      var_type=Z_SCALAR;
	    else if( is_this_unstr_var_vector(l_whichUnstrVar) )
	      var_type=Z_VECTOR;
	    else if( is_this_unstr_var_symtensor(l_whichUnstrVar) )
	      var_type=Z_TENSOR;
	    else if( is_this_unstr_var_tensor(l_whichUnstrVar) )
	      var_type=Z_TENSOR9;
	    else
	      {
		printinfo("got unknown var_type, using Z_SCALAR for now\n");
		var_type=Z_SCALAR;
	      }


	    printinfo("\n\n===================================================\n");
	    printinfo("============STARTING VAR %d of %d  \"%s\" time=%d ",
		      which_variable-1,l_numVariables,var_description[l_whichUnstrVar],l_timestep);
	    if( var_type==Z_SCALAR ) printinfo("Z_SCALAR\n");
	    else if( var_type==Z_VECTOR ) printinfo("Z_VECTOR\n");
	    else if( var_type==Z_TENSOR ) printinfo("Z_TENSOR\n");
	    else if( var_type==Z_TENSOR9 ) printinfo("Z_TENSOR9\n");
	    else  printinfo("Z_unknown?\n");
	    printinfo("===================================================\n");




	    for(i=1;i<=l_Numparts_available;i++)
	      {
		int l_numPointsInColl=0;
		int l_whichUnstrPart = i-get_num_blocks()-1;

		if( !is_there_an_unstr_var_instance( l_whichUnstrPart, l_whichUnstrVar, l_timestep ) )
		  {
		    printinfo("\nxxxxxx Doing ensight model part %d of %d:  NO INSTANCE OF UNSTR PART %d VAR %d TIME %d\n",
			      i-1,l_Numparts_available,
			      l_whichUnstrPart,l_whichUnstrVar, l_timestep );
		    continue;
		  }


		if( is_this_unstr_var_mapped_to_point_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=1;
		    which_type=Z_POINT;
		  }




		else if( is_this_unstr_var_mapped_to_tri_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=3;
		    which_type=Z_TRI03;
		  }
		else if( is_this_unstr_var_mapped_to_quad_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=4;
		    which_type=Z_QUA04;
		  }
		else if( is_this_unstr_var_mapped_to_tet_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=4;
		    which_type=Z_TET04;
		  }
		else if( is_this_unstr_var_mapped_to_hex_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=8;
		    which_type=Z_HEX08;
		  }
		else if( is_this_unstr_var_mapped_to_pyramid_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=5;
		    which_type=Z_PYR05;
		  }
		else if( is_this_unstr_var_mapped_to_prism_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=6;
		    which_type=Z_PEN06;
		  }
		else if( is_this_unstr_var_mapped_to_line_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=2;
		    which_type=Z_BAR02;
		  }


		else if( is_this_unstr_var_mapped_to_quadraticTri_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=6;
		    which_type=Z_TRI06;
		  }
		else if( is_this_unstr_var_mapped_to_quadraticQuad_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=8;
		    which_type=Z_QUA08;
		  }
		else if( is_this_unstr_var_mapped_to_quadraticTet_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=10;
		    which_type=Z_TET10;
		  }
		else if( is_this_unstr_var_mapped_to_quadraticHex_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=20;
		    which_type=Z_HEX20;
		  }
		else if( is_this_unstr_var_mapped_to_quadraticPyramid_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=13;
		    which_type=Z_PYR13;
		  }
		else if( is_this_unstr_var_mapped_to_quadraticPrism_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=15;
		    which_type=Z_PEN15;
		  }
		else if( is_this_unstr_var_mapped_to_quadraticLine_on_this_part(l_whichUnstrVar,l_whichUnstrPart) )
		  {
		    l_numPointsInColl=3;
		    which_type=Z_BAR03;
		  }




		else if( is_this_unstr_var_mapped_to_point(l_whichUnstrVar) )
		  {
		    /*This will handle the case when the variable is mapped to the node set, when
		      the node set is different than the part set*/
		    l_numPointsInColl=1;
		    which_type=Z_POINT;
		  }
		else
		  {
		    printinfo("\nxxxxxx Doing ensight model part %d of %d: cant do it, l_numPointsInColl=%d  mapped_to_mixed=%d\n",
			      i-1,l_Numparts_available, l_numPointsInColl, 
			      is_this_unstr_var_mapped_to_mixed_on_this_part(l_whichUnstrVar,l_whichUnstrPart) );
		    continue;
		  }





		if( !is_this_unstr_var_mapped_to_point(l_whichUnstrVar) )
		  {
		    l_numEntries = l_number_of_elements[i-1][which_type];
		    printinfo("\nxxxxxx Doing ensight model part %d of %d, with %d elements of type %s xxxxxxx\n",
			      i-1,l_Numparts_available,l_numEntries,get_name_of_ensight_celltype(which_type) );
		  }
		else
		  {
		    l_numEntries = number_of_nodes[i-1];
		    printinfo("\nxxxxxx Doing ensight model part %d of %d, with %d nodes xxxxxxx\n",
			      i-1,l_Numparts_available,l_numEntries );
		  }


		if( l_numEntries )
		  {
		    int l_numComponents=0;
		    /*note the +1: var_array ignores the first entry*/
		    float *var_array = (float *)malloc((l_numEntries+1)*sizeof(float));
		    if(!var_array)
		      {
			printinfo("fake_ensight fails: failed to alloc var_array for unstr var\n");
			return(-1);
		      }

		    if( var_type==Z_SCALAR ) l_numComponents = 1;
		    else if( var_type==Z_VECTOR )  l_numComponents = 3;
		    else if( var_type==Z_TENSOR )  l_numComponents = 6;
		    else if( var_type==Z_TENSOR9 )  l_numComponents = 9;
		    else { printinfo("\n\n UNKNOWN VAR TYPE, YOU SHOULDNT BE HERE\n\n"); exit(-1); }

		    /*
		      printinfo("var_array=%p l_numEntries=%d  numensightcomps=%d for ensight type %s\n",
		      var_array,l_numEntries,l_numComponents,get_name_of_ensight_celltype(which_type) );
		    */
		    for( component=0; component<l_numComponents; component++ )
		      {

			if(l_numComponents>1) printinfo("\n   xxxxxxxxxxxxx Doing component %d of %d xxxxxxx\n",component,l_numComponents);

			ret = USERD_get_var_by_component( which_variable,
							  i,
							  var_type,
							  which_type,
							  imag_data,
							  component,
							  var_array );

			if(ret==Z_ERR)
			  {
			    printinfo("fake_ensight fails\n");
			    return(-1);
			  }

			if(ret==Z_UNDEF)
			  {
			    printinfo("     component %d Z_UNDEF\n",component);
			  }
			else
			  {
#if 1
			    /*calc and print the min-max of the values*/
			    float l_min,l_max;
			    l_min = l_max = var_array[1];
			    for( k=2; k<l_numEntries+1; k++ )
			      {
				if(l_min>var_array[k]) l_min = var_array[k];
				if(l_max<var_array[k]) l_max = var_array[k];
			      }

			    if(l_min<-1000000000 || l_max>1000000000)
			      printinfo("      var_array min=%f max=%f    **warning large possibly bad values**\n",l_min,l_max);
			    else
			      printinfo("      var_array min=%f max=%f\n",l_min,l_max);
#endif
			  }


		      }

		    free(var_array);
		  }
	      }
	  }


	/*
	** free up
	*/
	if( var_description )
	  {
	    for(i=0;i<l_numVariables;i++)
	      {
		free( var_description[i]);
	      }
	    free(var_description );
	  }
	if( l_number_of_elements )
	  {
	    for(i=0;i<l_Numparts_available;i++) 
	      {
		free( l_number_of_elements[i] );
	      }
	    free( l_number_of_elements );
	    l_number_of_elements=0;
	  }
	if( ijk_dimensions )
	  {
	    for(i=0;i<l_Numparts_available;i++) 
	      {
		free( ijk_dimensions[i] );
	      }
	    free( ijk_dimensions );
	    ijk_dimensions=0;
	  }

	if( number_of_nodes )
	  {
	    free( number_of_nodes );
	    number_of_nodes=0;
	  }




      } /*l_timestep*/

  printinfo("\nSuccessfully finished fake_ensight. Now calling USERD_exit_routine\n\n");



  USERD_exit_routine( );



 

#ifdef HAVE_PARALLEL
  MPI_Finalize();
#endif


  return(0);
}
