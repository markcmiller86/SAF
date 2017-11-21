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
/*
**
**
**
*/


#include "str_mesh_reader.h"
#include "idl_reader.h"

int g_numDatasets=0;
int *g_datasetVar=0;
int *g_datasetTimestep=0;


/*
**
*/
void count_datasets()
{
  int i, j, l_numVars = get_num_str_vars();
  int l_numTimesteps = get_num_timesteps();
  int l_count=0;

  for(j=0; j<l_numTimesteps; j++)
    {
      for(i=0; i<l_numVars; i++)
	{
	  if( get_field_name_for_var( i, j, 0, 0 ) )
	    {
	      l_count++;
	    }
	}
    }

  g_datasetVar = (int *)malloc(l_count*sizeof(int));
  g_datasetTimestep = (int *)malloc(l_count*sizeof(int));

  l_count=0;
  for(j=0; j<l_numTimesteps; j++)
    {
      for(i=0; i<l_numVars; i++)
	{
	  if( get_field_name_for_var( i, j, 0, 0 ) )
	    {
	      g_datasetVar[l_count] = i;
	      g_datasetTimestep[l_count] = j;
	      l_count++;
	    }
	}
    }
  g_numDatasets = l_count;

  printinfo("count_datasets found %d\n",g_numDatasets);
}


/*
**
*/
void open_SAF( char* a_filename, int* a_error )
{
  printinfo("open_SAF trying file %s\n",a_filename);

  a_error[0]= init_str_mesh_reader( a_filename );

  count_datasets();
}

/*
**
*/
void close_SAF(  int* a_error )
{
  printinfo("close_SAF\n");

  a_error[0]= finalize_str_mesh_reader( 1 );

  g_numDatasets=0;
  if( g_datasetVar ) 
    {
      free(g_datasetVar);
      g_datasetVar=0;
    }
  if( g_datasetTimestep )
    {
      free(g_datasetTimestep);
      g_datasetTimestep=0;
    }
}

/*
**
*/
void dir_SAF( char* a_filename )
{
  char l_str[512];
  int j;
  FILE *fp = fopen(a_filename,"w");
  if( !fp ) 
    {
      return;
    }
  for(j=0; j<g_numDatasets; j++)
    {
      l_str[0]='\0';
      get_field_name_for_var( g_datasetVar[j], g_datasetTimestep[j], l_str, 511 );

      fprintf(fp,"%s\n",l_str);

      printinfo("dir_SAF: dataset %d:   var %d time %d name \"%s\"\n",j,
	     g_datasetVar[j], g_datasetTimestep[j],l_str);
    }
  fclose(fp);
}

/*
**
*/
void read_SAF_header( int a_dataset_number, 
		      int* a_nblock, /*# of blocks: could/should remove unused blocks for this var*/
		      int* a_nxyz, /* sum of all block x sizes, sum of y...etc*/
		      int* a_nvect ) /*sum of all block's x*y*z*/
{
  int l_numBlocks = get_num_blocks();
  int k,l_whichTimestep, l_whichVar;

  l_whichTimestep = g_datasetTimestep[ a_dataset_number ];
  l_whichVar = g_datasetVar[ a_dataset_number ];

  a_nblock[0]=0;
  a_nxyz[0]=0;
  a_nxyz[1]=0;
  a_nxyz[2]=0;
  a_nvect[0]=0;

  for(k=0; k<l_numBlocks; k++)
    {
      if(does_var_exist_on_block(k,l_whichVar,l_whichTimestep))
	{
	  int x,y,z;

	  a_nblock[0]++;

	  get_variable_size( k, l_whichVar, l_whichTimestep, &x, &y, &z );

	  a_nxyz[0] += x;
	  a_nxyz[1] += y;
	  a_nxyz[2] += z;
	  a_nvect[0] += x*y*z;
	}
    }
}



/*
Read dataset 
 where: 
REAL  - you tell me 
dataset_number The dataset number, n, corresponding to the nth 
dataset_name 

SAF_type    Type of data: 
            1=node, 2=X-edge, 3=Y-edge, 4=Z-edge, 
            5=X-normal face, 6=Y-normal face, 7=Z-normal face, 
8=Elements 
len_x  - length of each X-array for each block  (e.g.    8,4,2,3...)
len_y  - length of each Y-array for each block 
len_z  - length of each Z-array for each block 

x     - X values packed 
y     - Y values packed 
z     - Z values packed 
vector- Vector values packed 
*/
void read_SAF_dataset( int a_dataset_number, int *a_SAF_type,
		       int *a_len_x, int *a_len_y, int *a_len_z,
		       MY_PRECISION *a_x, MY_PRECISION *a_y, MY_PRECISION *a_z, 
		       MY_PRECISION *a_vector )
{
  int l_numBlocks = get_num_blocks();
  int k,l_whichTimestep, l_whichVar;
  int l_whichBlockInOutput=0;

  MY_PRECISION *l_xPtr = a_x;
  MY_PRECISION *l_yPtr = a_y;
  MY_PRECISION *l_zPtr = a_z;
  MY_PRECISION *l_vecPtr = a_vector;

  l_whichTimestep = g_datasetTimestep[ a_dataset_number ];
  l_whichVar = g_datasetVar[ a_dataset_number ];

  for(k=0; k<l_numBlocks; k++)
    {
      if(does_var_exist_on_block(k,l_whichVar,l_whichTimestep))
	{
	  int x,y,z,l_xyz;

	  get_variable_size( k, l_whichVar, l_whichTimestep, &x, &y, &z );
	  l_xyz= x*y*z;

	  /*just write over it each time: should always be the same*/
	  a_SAF_type[0]= get_variable_type( k, l_whichVar, l_whichTimestep );


	  a_len_x[l_whichBlockInOutput] = x;
	  a_len_y[l_whichBlockInOutput] = y;
	  a_len_z[l_whichBlockInOutput] = z;
	  
	  read_block_coords_for_selected_variable_as_separable( k, l_whichVar, l_whichTimestep,  l_xPtr, l_yPtr, l_zPtr );
	  
	  read_simple_variable( k, l_whichVar, l_whichTimestep, l_vecPtr );

	  l_xPtr += x;
	  l_yPtr += y;
	  l_zPtr += z;
	  l_vecPtr += l_xyz;
	  l_whichBlockInOutput++;
	}
    }
}

/*
 *
 *
 */
#ifdef DO_IDL_READER_TEST

int main( int argc, char **argv )
{

  int l_error=0;
#if 0
  open_SAF( "/home/jsjones/alegra_data/5ptflop.saf", &l_error );
#else
  open_SAF( "/home/jsjones/ensight_reader/test_str_mesh.saf", &l_error );
#endif
  if( l_error )
    {
      printinfo("open_SAF error in idl_reader_test. exiting\n");
      return(-1);
    }

  if(!g_numDatasets) return(0);

  dir_SAF( "idl_dir_SAF.txt" );

  {
    int i, l_numBlocks, l_nxyz[3], l_nvect, l_safType;
    int *l_len_x,*l_len_y,*l_len_z;
    MY_PRECISION *l_x,*l_y,*l_z,*l_data;

    for(i=0;i<g_numDatasets;i++)
      {

	read_SAF_header( i, &l_numBlocks, l_nxyz, &l_nvect );

	printinfo("read_SAF_header: dataset %d: blocks=%d, xyz=%d %d %d, nvect=%d\n",
	       i,l_numBlocks,l_nxyz[0],l_nxyz[1],l_nxyz[2],l_nvect );


	if( l_numBlocks && l_nxyz[0] && l_nxyz[1] && l_nxyz[2] && l_nvect )
	  {
	    l_len_x = (int *)malloc(l_numBlocks*sizeof(int));
	    l_len_y = (int *)malloc(l_numBlocks*sizeof(int));
	    l_len_z = (int *)malloc(l_numBlocks*sizeof(int));
	    l_x = (MY_PRECISION *)malloc(l_nxyz[0]*sizeof(MY_PRECISION));
	    l_y = (MY_PRECISION *)malloc(l_nxyz[1]*sizeof(MY_PRECISION));
	    l_z = (MY_PRECISION *)malloc(l_nxyz[2]*sizeof(MY_PRECISION));
	    l_data = (MY_PRECISION *)malloc(l_nvect*sizeof(MY_PRECISION));
	    
	    if( !l_len_x || !l_len_y || !l_len_z || !l_x || !l_y || !l_z || !l_data )
	      {
		printinfo("error allocating for read_SAF_dataset, exiting\n");
		exit(-1);
	      }
	    printinfo("\nread_SAF_dataset:\n");

	    read_SAF_dataset( i, &l_safType,
			      l_len_x, l_len_y, l_len_z,
			      l_x, l_y, l_z, 
			      l_data );

#if 0    
	    {
	      int j;
	      
	      printinfo("l_len_x[%d]=",l_numBlocks);
	      for(j=0;j<l_numBlocks;j++) printinfo(" %d",l_len_x[j]);
	      printinfo("\n");

	      printinfo("l_len_y[%d]=",l_numBlocks);
	      for(j=0;j<l_numBlocks;j++) printinfo(" %d",l_len_y[j]);
	      printinfo("\n");

	      printinfo("l_len_z[%d]=",l_numBlocks);
	      for(j=0;j<l_numBlocks;j++) printinfo(" %d",l_len_z[j]);
	      printinfo("\n");


	      printinfo("l_x[%d]=",l_nxyz[0]);
	      for(j=0;j<l_nxyz[0];j++) printinfo(" %f",l_x[j]);
	      printinfo("\n");

	      printinfo("l_y[%d]=",l_nxyz[1]);
	      for(j=0;j<l_nxyz[1];j++) printinfo(" %f",l_y[j]);
	      printinfo("\n");

	      printinfo("l_z[%d]=",l_nxyz[2]);
	      for(j=0;j<l_nxyz[2];j++) printinfo(" %f",l_z[j]);
	      printinfo("\n");

	      printinfo("l_data[%d]=",l_nvect);
	      for(j=0;j<l_nvect;j++) printinfo(" %f",l_data[j]);
	      printinfo("\n\n");
	    }

#endif

	    free(l_len_x);
	    free(l_len_y);
	    free(l_len_z);
	    free(l_x);
	    free(l_y);
	    free(l_z);
	    free(l_data);
	  }
	else
	  {
	    printinfo("read_SAF_dataset: trivial case, not calling it\n");
	  }
      }
  }

  close_SAF( &l_error );
  if( l_error )
    {
      printinfo("close_SAF error in idl_reader_test. exiting\n");
      return(-1);
    }




  return(0);
}
#endif
