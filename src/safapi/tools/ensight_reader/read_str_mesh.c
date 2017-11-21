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
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Description:
 *
 *
 * The functions here use a global variables defined in this file (global vars are 
 * prepended by g_, local vars are prepended by l_, and argument vars are prepended by a_).
 *
 *--------------------------------------------------------------------------------------------------- */


#include <math.h>
#include <str_mesh_reader.h>
#include <str_mesh_test.h>


 

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Global Variables
 *
 * Description: 
 *
 * These are global variables, most of which will be retrieved from str_mesh_reader.c.
 *
 *--------------------------------------------------------------------------------------------------- */
SAF_Db *g_db;
int g_numSets,g_numSubsets;
SAF_Set *g_sets,*g_subsets,*g_edgeSubsets,*g_faceSubsets,*g_elemSubsets;
int g_numEdgeSubsets,g_numFaceSubsets,g_numElemSubsets;
int g_nprocs=1;/*NOT from str_mesh_reader.c*/
int g_rank=0;/*NOT from str_mesh_reader.c*/




/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Global Variables
 *
 * Description: 
 *
 * These are prototypes.
 *
 *--------------------------------------------------------------------------------------------------- */


void read_and_print_all_vars_on_set( SAF_Set a_set );
void read_and_print_subsets_on_set( SAF_Set a_set, SAF_Cat a_coll, int a_numSubsets, SAF_Set *a_subsets  );
void read_and_print_set_attributes( SAF_Set a_set );
void read_and_print_attributes( void );
void add_to_test_list(const char *a_name,int a_x,int a_y,int a_z,
		      int *a_which, char **a_destStrings, int *a_destSizes);
int exit_with_error(void);
char *get_size_of_variable_field_on_set( SAF_Set a_set, int a_whichVar, int *a_num, char **a_varType );
void read_variable_field_on_set( SAF_Set a_set, int a_whichVar, MY_PRECISION *a_varData );



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Get the Size of a Variable Field on a Set
 * Description:
 *
 * Get the number of entries and the variable type of a given field on a given set.
 * The field is specified by an index referring to the list returned by saf_find_fields,
 * omitting any coordinate fields.
 * 
 * Note that the term "variable" here is only local to the set, so variable N on set A is
 * not necessarily related to variable N on set B. This is different from the way "variable" 
 * is used in the Structured Mesh Reader code: there the variables are arranged so that they
 * refer to the same thing on different sets.
 *
 * The valid range for a_whichField is 0 to N-1, where N is returned by get_num_variable_fields_on_set.
 *
 * This function uses no global variables.
 *
 * Return: a character string describing the variable type, see get_var_type_name
 *
 *--------------------------------------------------------------------------------------------------- */
char *get_size_of_variable_field_on_set( SAF_Set a_set, int a_whichField, int *a_num, char **a_varType )
{
  int l_numFields=0,i,l_count=0;
  SAF_Field *l_fields=0;
  saf_find_fields(SAF_ALL, g_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
		  SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);
  for( i = 0 ; i < l_numFields; i++ )
    {
      hbool_t l_isCoordField;
      hid_t l_varType;
      SAF_Cat l_cat;
      char *l_name=0;

      if( !my_saf_is_valid_field_handle( &(l_fields[i])) ) continue;

      saf_describe_field(SAF_ALL, &(l_fields[i]), NULL,  &l_name, NULL, NULL,
			 &l_isCoordField, NULL, &l_cat, NULL, NULL, 
			 NULL, &l_varType, NULL, NULL, NULL, NULL);

      if( !l_isCoordField ) 
	{
	  if( l_count < a_whichField )
	    {
	      l_count++;
	    }
	  else
	    {
	      SAF_CellType l_cellType;
	      SAF_IndexSpec l_indexSpec;
	      a_num[0]=0;
	      saf_describe_collection(SAF_ALL,&a_set,&l_cat,&l_cellType,a_num,&l_indexSpec,NULL,NULL);	

	      if( a_varType )
		{
		  int l_xDim=0,l_yDim=0,l_zDim=0;
		  int l_ix,l_iy,l_iz;

		  l_ix = l_indexSpec.sizes[0];
		  l_iy = l_indexSpec.sizes[1];
		  l_iz = l_indexSpec.sizes[2];

		  get_block_size_from_set(&a_set,&l_xDim,&l_yDim,&l_zDim);
		  if( l_cellType==SAF_CELLTYPE_POINT )
		    {
		      a_varType[0] = ens_strdup("nodes");
		    }
		  else if( l_cellType==SAF_CELLTYPE_LINE )
		    {
		      if( l_xDim-1 == l_ix ) a_varType[0] = ens_strdup("xEdges");
		      else if( l_yDim-1 == l_iy ) a_varType[0] = ens_strdup("yEdges");
		      else if( l_zDim-1 == l_iz ) a_varType[0] = ens_strdup("zEdges");
		      else a_varType[0]=ens_strdup("Edges");
		    }
		  else if( l_cellType == SAF_CELLTYPE_QUAD )
		    {
		      if( l_xDim == l_ix ) a_varType[0] = ens_strdup("xFaces");
		      else if( l_yDim == l_iy ) a_varType[0] = ens_strdup("yFaces");
		      else if( l_zDim == l_iz ) a_varType[0] = ens_strdup("zFaces");
		      else a_varType[0] = ens_strdup("Faces");
		    }
		  else if( l_cellType == SAF_CELLTYPE_HEX )
		    {
		      a_varType[0] = ens_strdup("elems");
		    }
		  else 
		    a_varType[0] = ens_strdup("(unknown cell type?)");
		}

	      {
		char *l_returned = ens_strdup(l_name);
		if(l_fields) free(l_fields);
		if(l_name) free(l_name);
		return( l_returned );
	      }
	    }
	}
    }
  if(l_fields) free(l_fields);
  return(0);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Read a Variable Field from a Set
 * Description:
 *
 * Read a given field on a given set. The calling function must preallocate a_varData to
 * the size specified by get_size_of_variable_field_on_set. The field is specified by an index 
 * referring to the list returned by saf_find_fields, omitting any coordinate fields.
 *
 * Note that the term "variable" here is only local to the set, so variable N on set A is
 * not necessarily related to variable N on set B. This is different from the way "variable" 
 * is used in the Structured Mesh Reader code: there the variables are arranged so that they
 * refer to the same thing on different sets.
 *
 * The valid range for a_whichField is 0 to N-1, where N is returned by get_num_variable_fields_on_set.
 *
 * This function uses no global variables.
 *
 *--------------------------------------------------------------------------------------------------- */
void read_variable_field_on_set( SAF_Set a_set, int a_whichField, MY_PRECISION *a_varData )
{
  int l_numFields=0,i,l_count=0;
  SAF_Field *l_fields=0;
  saf_find_fields(SAF_ALL, g_db, &a_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
		  SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_numFields, &l_fields);
  for( i = 0 ; i < l_numFields; i++ )
    {
      hbool_t l_isCoordField;
      hid_t l_varType;
      SAF_Cat l_cat;

      if( !my_saf_is_valid_field_handle( &(l_fields[i])) ) continue;

      saf_describe_field(SAF_ALL, &(l_fields[i]), NULL, NULL, NULL, NULL,
			 &l_isCoordField, NULL, &l_cat, NULL, NULL, 
			 NULL, &l_varType, NULL, NULL, NULL, NULL);
      if( !l_isCoordField ) 
	{
	  if( l_count < a_whichField )
	    {
	      l_count++;
	    }
	  else
	    {
	      int l_numEntries=0;
	      int l_timestep = get_timestep_for_field( l_fields[i] );
	      saf_describe_collection(SAF_ALL,&a_set,&l_cat,NULL,&l_numEntries,NULL,NULL,NULL);

	      printinfo("Timestep for var = %d\n",l_timestep);

	      if( read_whole_field_to_my_precision(&(l_fields[i]),l_varType,(size_t)l_numEntries,a_varData) )
		{
		  free(l_fields);
		  return;
		}
	      break;
	    }
	}
    }
  if(l_fields) free(l_fields);
}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Read and Print a Portion of all Variable Fields on a Set
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void read_and_print_all_vars_on_set( SAF_Set a_set )
{
  int l_numVars,j,k;
  l_numVars = get_num_variable_fields_on_set( a_set );
  if(!g_rank ) printinfo("Found %d vars on set\n",l_numVars);

  if( l_numVars )
    {

      for( j=0; j<l_numVars; j++ )
	{
	  int l_numEntries=0;
	  MY_PRECISION *l_var;
	  char *l_collName=0;
	  char *l_name = get_size_of_variable_field_on_set( a_set, j, &l_numEntries, &l_collName );

	  if(!l_numEntries ) continue;

	  l_var = (MY_PRECISION *)malloc(l_numEntries*sizeof(MY_PRECISION));
	  read_variable_field_on_set( a_set, j, l_var );

	  if(!g_rank )
	    {
	      float l_min=l_var[0];
	      float l_max=l_var[0];

	      /*print a few at the beginning and print the last one*/
	      printinfo("%s var %s[%d]: ",l_collName,l_name,l_numEntries);
	      for(k=0;k<l_numEntries;k++)
		{
		  if( k==4 && k<l_numEntries-1 )
		    {
		      printinfo("...");
		    }
		  else if( k<4 )
		    {
		      printinfo("%e ",l_var[k]);
		    }
		  if( l_var[k] > l_max ) l_max = l_var[k];
		  else if( l_var[k] < l_min ) l_min = l_var[k];
		}
	      if(k<l_numEntries) printinfo("%e",l_var[l_numEntries-1]);
	      printinfo("\n");
	      printinfo("\t\t min=%e  max=%e\n",l_min,l_max);
	    }

	  free(l_var);
	  if(l_name) free(l_name);
	}
    }
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Print Information from all Subsets of a Set
 *
 * Description: 
 *
 * The argument a_coll must be one of the eight collections on a block (nodes, elems,
 * x-edges, y-edges, z-edges, x-faces, y-faces, z-faces)
 *
 * The argument a_subsets must be the list of subsets returned by get_num_node_subsets,
 * get_num_edge_subsets, get_num_face_subsets, or get_num_elem_subsets
 *
 * The argument a_numSubsets is the length of the array in argument a_subsets.
 *
 *--------------------------------------------------------------------------------------------------- */
void read_and_print_subsets_on_set( SAF_Set a_set, SAF_Cat a_coll, int a_numSubsets, SAF_Set *a_subsets  )
{
  int j;
  for( j=0; j<a_numSubsets; j++ )
    {
      int l_numRels=0;
      SAF_Rel *l_rels=0;

      saf_find_subset_relations(SAF_ALL, g_db, &a_set, &(a_subsets[j]), &a_coll, 
				&a_coll, SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE,
				&l_numRels,  &l_rels );



#if 1
  {
    char *l_supersetname=0, *l_subsetname=0, *l_catname=0;
    saf_describe_set(SAF_ALL, &a_set, &l_supersetname, NULL, NULL, NULL, NULL, NULL, NULL );
    saf_describe_set(SAF_ALL, &(a_subsets[j]), &l_subsetname, NULL, NULL, NULL, NULL, NULL, NULL );
    saf_describe_category(SAF_ALL,&a_coll,&l_catname,NULL,NULL);
    if(!g_rank) printinfo("%d    read_and_print_subsets_on_set superset=\"%s\" subset=\"%s\"   cat=\"%s\"  \n",
	   l_numRels,l_supersetname,l_subsetname,l_catname);

    if(l_supersetname) free(l_supersetname);
    if(l_subsetname) free(l_subsetname);
    if(l_catname) free(l_catname);
  }
#endif



      if( l_numRels>1 )
	{
	  if(!g_rank) printinfo("read_and_print_subsets_on_set error: found %d subset relations\n",l_numRels);
	}
      else if( l_numRels==1 )
	{
	  size_t l_abuf_sz,l_bbuf_sz;
	  int *l_abuf=NULL,*l_bbuf=NULL;
	  hid_t   l_abuf_type,l_bbuf_type;
	  struct_hyperslab *l_hslab;
	  
	  saf_get_count_and_type_for_subset_relation(SAF_ALL,&(l_rels[0]),NULL,&l_abuf_sz,&l_abuf_type,&l_bbuf_sz,&l_bbuf_type );
	  
	  if(l_abuf_sz!=9 || !H5Tequal(l_abuf_type,SAF_INT) || l_bbuf_sz!=0)
	    {
	      if(!g_rank) printinfo("read_and_print_subsets_on_set error: subset relation is not a 3d hslab\n");
	    }
	  else
	    {
	      saf_read_subset_relation(SAF_ALL,&(l_rels[0]),NULL,(void **)&l_abuf, (void **)&l_bbuf );
	      l_hslab = (struct_hyperslab *)l_abuf;
	      if(!g_rank ) printinfo("read hyperslab: %d,%d,%d   %d,%d,%d   %d,%d,%d\n",
		     l_hslab->x_start,l_hslab->x_count,l_hslab->x_skip,
		     l_hslab->y_start,l_hslab->y_count,l_hslab->y_skip,
		     l_hslab->z_start,l_hslab->z_count,l_hslab->z_skip );
	    }
	  read_and_print_all_vars_on_set( a_subsets[j] );		
	}

      if(l_rels) free(l_rels);
    }
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Read and Print All Attributes on a Set
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void read_and_print_set_attributes( SAF_Set a_set )
{
  int l_count=0,i;
  char **l_names=NULL;
  hid_t l_type = 0;

  saf_get_set_att(SAF_ALL, &a_set, SAF_ATT_NAMES, &l_type, &l_count, (void **)&l_names );

  if(!g_rank ) printinfo("Found %d attributes on this set:\n",l_count);

  for(i=0;i<l_count;i++)
    {
      char *l_strings=NULL;
      int l_countAgain=0,j;
      hid_t l_typeAgain = H5T_STRING;
      saf_get_set_att(SAF_ALL, &a_set, l_names[i], &l_typeAgain, &l_countAgain, (void **)&l_strings );

      for( j=0; j<l_countAgain; j++ )
	{

	  if(H5Tequal(l_typeAgain,H5T_STRING))
	    {
	      if(l_countAgain!=1)
		{
		  /*do nothing. This is broken? see getAtt.c*/
		  if(!g_rank) printinfo("    attribute %d of %d, type:string, #components=%d ....saf is broken for >1 ?\n",
		       i, l_count, l_countAgain );
		}
	      else
		if(!g_rank ) printinfo("    attribute %d of %d, type:string, name:%s, component %d of %d=%s\n",
		       i, l_count, l_names[i], j, l_countAgain, l_strings );
	    }
	  else if(H5Tequal(l_typeAgain,SAF_FLOAT)) {
	    if(!g_rank ) printinfo("    attribute %d of %d, type:float, name:%s, component %d of %d=%f\n",
			       i, l_count, l_names[i], j, l_countAgain, ((float *)l_strings)[j] );
	  } else if(H5Tequal(l_typeAgain,SAF_DOUBLE)) {
	    if(!g_rank ) printinfo("    attribute %d of %d, type:double, name:%s, component %d of %d=%f\n",
			       i, l_count, l_names[i], j, l_countAgain, ((float *)l_strings)[j] );
	  } else if(H5Tequal(l_typeAgain,SAF_INT)) {
	    if(!g_rank ) printinfo("    attribute %d of %d, type:integer, name:%s, component %d of %d=%d\n",
			       i, l_count, l_names[i], j, l_countAgain, ((int *)l_strings)[j] );
	  } else {
	    if(!g_rank) printinfo("  attribute %d of %d, unknown type\n",i,l_count);
	  }

	}
    }

}


/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Read and Print all Database Attributes
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void read_and_print_attributes(  )
{
  int l_count=0,i;
  char **l_names=NULL;

  saf_get_attribute(SAF_ALL,(ss_pers_t*)g_db,SAF_ATT_NAMES, NULL, &l_count, (void **)&l_names );

  for(i=0;i<l_count;i++)
    {
      char *l_strings=NULL;
      int l_countAgain=0,j;
      hid_t l_typeAgain = H5T_STRING;
      saf_get_attribute(SAF_ALL,(ss_pers_t*)g_db, l_names[i], &l_typeAgain, &l_countAgain, (void **)&l_strings );

      for( j=0; j<l_countAgain; j++ )
	{
	  if(H5Tequal(l_typeAgain,H5T_STRING))
	    {
	      if(l_countAgain!=1)
		{
		  /*do nothing. This is broken? see getAtt.c*/
		  if(!g_rank) printinfo("    attribute %d of %d, type:string, #components=%d ....saf is broken for >1 ?\n",
		       i, l_count, l_countAgain );
		}
	      else
		if(!g_rank ) printinfo("    attribute %d of %d, type:string, name:%s, component %d of %d=%s\n",
		       i, l_count, l_names[i], j, l_countAgain, l_strings );
	    }
	  else if(H5Tequal(l_typeAgain,SAF_FLOAT)) {
	    if(!g_rank ) printinfo("    attribute %d of %d, type:float, name:%s, component %d of %d=%f\n",
		   i, l_count, l_names[i], j, l_countAgain, ((float *)l_strings)[j] );
	  } else if(H5Tequal(l_typeAgain,SAF_DOUBLE)) {
	    if(!g_rank ) printinfo("    attribute %d of %d, type:double, name:%s, component %d of %d=%f\n",
		   i, l_count, l_names[i], j, l_countAgain, ((float *)l_strings)[j] );
	  } else if(H5Tequal(l_typeAgain,SAF_INT)) {
	    if(!g_rank ) printinfo("    attribute %d of %d, type:integer, name:%s, component %d of %d=%d\n",
		   i, l_count, l_names[i], j, l_countAgain, ((int *)l_strings)[j] );
	  } else {
	    /*XXX why do I find one of these?*/
	    if(!g_rank) printinfo("\n    attribute %d of %d, unknown type\n",i,l_count);
	  }
	}
    }
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Add to the List that will be Tested with Make Test
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void add_to_test_list(const char *a_name,int a_x,int a_y,int a_z,
		      int *a_which, char **a_destStrings, int *a_destSizes)
{
  if(a_x && a_y && a_z)
    {
      a_destStrings[a_which[0]] = ens_strdup(a_name);
      a_destSizes[a_which[0]*3+0] = a_x;
      a_destSizes[a_which[0]*3+1] = a_y;
      a_destSizes[a_which[0]*3+2] = a_z;
      a_which[0]++;
    }
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     Exit With an Error
 *
 * Description: 
 * Returns -1 after closing the database, calling saf_final, an calling MPI_Finalize if needed.
 *
 *--------------------------------------------------------------------------------------------------- */
int exit_with_error()
{
  if(!g_rank) printinfo("read_str_mesh: about to saf_close_database and saf_final in exit_with_error\n");
  saf_close_database(g_db);
  saf_final();

#ifdef HAVE_PARALLEL
   MPI_Finalize();
#endif

  return(-1);
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Simple Structured Mesh Read and Print Program (read_str_mesh)
 * Purpose:     main Function of read_str_mesh
 *
 * Description: 
 *
 *
 *--------------------------------------------------------------------------------------------------- */
int main(int argc, char **argv)
{/* Note that I call this function "Private" so that mkdoc doesnt generate an "Overloaded Definitions" chapter*/
  int i;
  int l_short=0;

#define MY_MAX_DBNAME_SIZE 1024

  char l_dbname[MY_MAX_DBNAME_SIZE];
  char l_dbnameDefault[]="test_str_mesh.saf";        /* Name of the SAF database file to be created. */
  int l_doTest=0, l_testFileNumProcs=1;

  char **l_testDesiredBlockNames=0;
  int *l_testDesiredBlockDims=0;
  int l_printToFile=0;
 

  /*set the environment variable: otherwise, printinfo wont print*/
  char *l_envVar = (char *)malloc(256*sizeof(char));
  sprintf(l_envVar,"SAF_READER_DEBUG=true");
#ifndef JANUS
  putenv(l_envVar); 
#endif
  free(l_envVar);


#ifdef WIN32
  /*g_rerouteStdinStdout=0;*/
#endif


#ifdef HAVE_PARALLEL
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &g_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &g_nprocs);
#endif

  l_testFileNumProcs = g_nprocs;

  initialize_str_globals();/*must do this before printinfo_set_filename, or the latter will be overwritten*/
  printinfo_set_filename(NULL);
  snprintf(l_dbname,MY_MAX_DBNAME_SIZE-1,"%s",l_dbnameDefault);


  /*PARSE COMMAND LINE*/
  {
    int l_whichArg;
    int l_gotFilename=0,l_printHelpAndExit=0;
    if(!g_rank)
      {
	for(l_whichArg=1; l_whichArg<argc; l_whichArg++)
	  {
	    if( !strcmp(argv[l_whichArg],"-h") ||  !strcmp(argv[l_whichArg],"-qh") ||
		!strcmp(argv[l_whichArg],"-hq") ||
		!strcmp(argv[l_whichArg],"--help") || argc>=6 ) /*xxx improve this: not all combinations covered*/
	      {
		l_printHelpAndExit=1;
	      }
	    else if( !strcmp(argv[l_whichArg],"-q") || !strcmp(argv[l_whichArg],"--quiet") )
	      {
		l_printToFile=1;
	      }
	    else if( !strcmp(argv[l_whichArg],"-sh") || !strcmp(argv[l_whichArg],"--short") )
	      {
		l_short=1;
	      }
	    else if( !strcmp(argv[l_whichArg],"-t") || !strcmp(argv[l_whichArg],"--test") )
	      {
		l_doTest=1;
		/*get an integer (# procs that the test file was created with) that must follow -t*/
		l_whichArg++;
		if(l_whichArg>=argc)
		  {
		    l_printHelpAndExit=1;
		  }
		else
		  {
		    l_testFileNumProcs = atoi(argv[l_whichArg]);
		  }
#ifndef HAVE_PARALLEL
		/*Note: for the moment, if run in serial, this parameter doesnt matter,
		  overwrite it now*/
		l_testFileNumProcs=1;
#endif
	      }
	    else if(!l_gotFilename)
	      {
		l_gotFilename=1;
		snprintf(l_dbname,MY_MAX_DBNAME_SIZE,argv[l_whichArg]);
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
    MPI_Bcast(&l_doTest, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&l_gotFilename, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&l_testFileNumProcs, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&l_short, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if(l_gotFilename)
      {
	MPI_Bcast(l_dbname, MY_MAX_DBNAME_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
      }
#endif

#define OUTPUT_FILE_NAME "saf_reader_stdout.txt"
    if( l_printToFile )
      {
	printinfo_set_filename(OUTPUT_FILE_NAME);
      }

    /*printf("AFTER COMMAND LINE: pr=%d help=%d tofile=%d test=%d gotfile=%d %s\n",
      g_rank,l_printHelpAndExit,l_printToFile,l_doTest,l_gotFilename,l_dbname);*/

    if(l_printHelpAndExit)
      {
	printf("Usage: %s [-h] [-q] [-t NPROCS] [FILENAME]\n",argv[0]);
	printf("Read structured mesh data from a SAF file.\n\n");
	printf("   -h, --help  print this help message and exit\n");
	printf("   -q, --quiet  output messages to file %s, otherwise output to stdout\n",OUTPUT_FILE_NAME);
	printf("   -t NPROCS, --test NPROCS  test the saf file against str_mesh_test.h, return 0 for pass\n");
	printf("      1 for fail, NPROCS is a positive integer that equals the # of processors the data file\n");
	printf("      was written with (if run in serial, NPROCS is currently ignored)\n");
	printf("   FILENAME  read this saf file, otherwise use file %s\n\n",l_dbnameDefault);
	printf("report bugs to saf-help@sourceforge.sandia.gov\n");

	if( l_doTest ) return(-1);

	return(0);
      }

  }

  if(!g_rank) printinfo("Beginning read_str_mesh on file %s\n",l_dbname);

  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  if( init_str_mesh_reader( l_dbname ) ) return(-1);



  /*Temporary - get the global vars we are borrowing from str_mesh_reader.c*/
  g_db = getDatabase();
  g_numSets = getNumStrSets();
  g_numSubsets = getNumStrSubsets();
  g_numEdgeSubsets = getNumStrEdgeSubsets();
  g_numFaceSubsets = getNumStrFaceSubsets();
  g_numElemSubsets = getNumStrElemSubsets();
  g_sets = getStrSets();
  g_subsets = getStrSubsets();
  g_edgeSubsets = getStrEdgeSubsets();
  g_faceSubsets = getStrFaceSubsets();
  g_elemSubsets = getStrElemSubsets();


  if(l_short) g_numTimestepsToWrite = g_numTimestepsToWriteShort;

  if(l_doTest)
    {
      /*compare # of timesteps*/
      if(g_numTimestepsToWrite!=get_num_timesteps())
	{
	  printf("%d read_str_mesh TEST FAILED: found %d timesteps, expected %d\n",g_rank,g_numTimestepsToWrite,get_num_timesteps());
	  return( exit_with_error() );
	}      
      printf("%d read_str_mesh TEST PASSED: found %d timesteps, expected %d\n",g_rank,g_numTimestepsToWrite,get_num_timesteps());
      
      /*compare # of blocks. note: dummy blocks(0x0x0) wont be found, so must count this way*/
      {
	int l_numExpected=0;

	if(g_dataBlockAProc0XDim && g_dataBlockAProc0YDim && g_dataBlockAProc0ZDim) l_numExpected++;
	if(g_dataBlockBProc0XDim && g_dataBlockBProc0YDim && g_dataBlockBProc0ZDim) l_numExpected++;

	if( l_testFileNumProcs > 1 )
	  {
	    if(g_dataBlockAProc1XDim && g_dataBlockAProc1YDim && g_dataBlockAProc1ZDim) l_numExpected++;
	    if(g_dataBlockBProc1XDim && g_dataBlockBProc1YDim && g_dataBlockBProc1ZDim) l_numExpected++;
	  }
	if( l_testFileNumProcs > 2 )
	  {
	    if(g_dataBlockAProc2XDim && g_dataBlockAProc2YDim && g_dataBlockAProc2ZDim) l_numExpected += (l_testFileNumProcs-2);
	    if(g_dataBlockBProc2XDim && g_dataBlockBProc2YDim && g_dataBlockBProc2ZDim) l_numExpected += (l_testFileNumProcs-2);
	  }

	if( l_numExpected != g_numSets )
	  {
	    printf("%d read_str_mesh TEST FAILED: found %d blocks, expected %d\n",g_rank,g_numSets,l_numExpected);
	    return( exit_with_error() );
	  }      
	printf("%d read_str_mesh TEST PASSED: found %d blocks, expected %d\n",g_rank,g_numSets,l_numExpected);
      }

      /*create the list of expected block names and expected block sizes*/
      if(g_numSets)
	{
	  int l_which=0,j;
	  l_testDesiredBlockNames=(char **)malloc(g_numSets*sizeof(char *));
	  l_testDesiredBlockDims=(int *)malloc(g_numSets*3*sizeof(int));
	  for(j=0;j<g_numSets;j++) l_testDesiredBlockNames[j]=0;

	  add_to_test_list("block_A_pr0",g_dataBlockAProc0XDim,g_dataBlockAProc0YDim,g_dataBlockAProc0ZDim,
			   &l_which,l_testDesiredBlockNames,l_testDesiredBlockDims);
	  add_to_test_list("block_B_pr0",g_dataBlockBProc0XDim,g_dataBlockBProc0YDim,g_dataBlockBProc0ZDim,
			   &l_which,l_testDesiredBlockNames,l_testDesiredBlockDims);
	  if( l_testFileNumProcs > 1 )
	    {
	      add_to_test_list("block_A_pr1",g_dataBlockAProc1XDim,g_dataBlockAProc1YDim,g_dataBlockAProc1ZDim,
			       &l_which,l_testDesiredBlockNames,l_testDesiredBlockDims);
	      add_to_test_list("block_B_pr1",g_dataBlockBProc1XDim,g_dataBlockBProc1YDim,g_dataBlockBProc1ZDim,
			       &l_which,l_testDesiredBlockNames,l_testDesiredBlockDims);
	    }
	  for(j=2;j<l_testFileNumProcs;j++)
	    {
	      char l_str[256];
	      sprintf(l_str,"block_A_pr%d",j);
	      add_to_test_list(l_str,g_dataBlockAProc2XDim,g_dataBlockAProc2YDim,g_dataBlockAProc2ZDim,
			       &l_which,l_testDesiredBlockNames,l_testDesiredBlockDims);
	      sprintf(l_str,"block_B_pr%d",j);
	      add_to_test_list(l_str,g_dataBlockBProc2XDim,g_dataBlockBProc2YDim,g_dataBlockBProc2ZDim,
			       &l_which,l_testDesiredBlockNames,l_testDesiredBlockDims);
	    }
	}
    }


  {
    char l_str[256];
    get_str_mesh_reader_version(l_str,256);
    if(!g_rank) printinfo("%s\n\n",l_str);
  }

#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif


  /*READ ALL BLOCKS: note: there is no example here of doing this
    recursively, but it is certainly possible*/
  for( i = 0 ; i < g_numSets; i++ ) 
    {
      int l_numCollections=0;
      char *l_name=0;
      SAF_SilRole l_role;
      SAF_ExtendMode l_extendMode;
      SAF_TopMode l_topMode;
      SAF_Cat *l_cats=0;
      int l_xDim=0,l_yDim=0,l_zDim=0,l_topoDim=0;
      MY_PRECISION *l_xCoords=0, *l_yCoords=0, *l_zCoords=0;
      SAF_Cat *l_nodes=0,*l_xEdges=0,*l_yEdges=0,*l_zEdges=0,*l_xFaces=0,*l_yFaces=0,*l_zFaces=0,*l_elems=0; 

      /*GET INFO ABOUT THIS BLOCK*/
      saf_describe_set(SAF_ALL, &(g_sets[i]), &l_name, &l_topoDim, &l_role, &l_extendMode, 
		       &l_topMode, &l_numCollections, &l_cats);

      /*READ THIS BLOCK*/
      get_block_size(i,&l_xDim,&l_yDim,&l_zDim);



      if(l_doTest)
	{
	  int j,l_found=0;
	  for(j=0;j<g_numSets;j++)
	    {
	      if( l_testDesiredBlockNames[j]
		  && !strcmp( l_testDesiredBlockNames[j], l_name ) 
		  && l_xDim==l_testDesiredBlockDims[j*3+0]
		  && l_yDim==l_testDesiredBlockDims[j*3+1]
		  && l_zDim==l_testDesiredBlockDims[j*3+2] )
		{
		  l_found=1;
		  break;
		}
	    }
	  if(!l_found)
	    {
	      printf("%d read_str_mesh TEST FAILED: did not find block %s (%dx%dx%d)\n",
		     g_rank,l_name,l_xDim,l_yDim,l_zDim);
	      if(!g_rank)
		{
		  printf("   In the following list:\n");
		  for(j=0;j<g_numSets;j++)
		    {
		      printf("      %s %dx%dx%d\n",l_testDesiredBlockNames[j],l_testDesiredBlockDims[j*3+0],
			     l_testDesiredBlockDims[j*3+1],l_testDesiredBlockDims[j*3+2] );
		    }
		}
	      return( exit_with_error() );
	    }   
	  printf("%d read_str_mesh TEST PASSED: found block %s (%dx%dx%d)\n",   
		 g_rank,l_name,l_xDim,l_yDim,l_zDim);
	}




      l_xCoords = (MY_PRECISION *)malloc(l_xDim*sizeof(MY_PRECISION));
      l_yCoords = (MY_PRECISION *)malloc(l_yDim*sizeof(MY_PRECISION));
      l_zCoords = (MY_PRECISION *)malloc(l_zDim*sizeof(MY_PRECISION));

      /*treat the coords as separable, whether they are or not*/
      read_block_coords_as_separable(i,l_xCoords,l_yCoords,l_zCoords);
 
      read_and_print_set_attributes(g_sets[i]);

      get_str_mesh_collections( g_sets[i], &l_nodes, &l_xEdges, &l_yEdges, &l_zEdges, &l_xFaces, 
				&l_yFaces, &l_zFaces, &l_elems );

      /*read all vars mapped to collections on this block*/
      read_and_print_all_vars_on_set( g_sets[i] );

      if(!g_rank ) printinfo("Reading node subsets on this block:\n");
      read_and_print_subsets_on_set( g_sets[i], *l_nodes, g_numSubsets, g_subsets  );

      if(!g_rank ) printinfo("Reading x edge subsets on this block:\n");
      read_and_print_subsets_on_set( g_sets[i], *l_xEdges, g_numEdgeSubsets, g_edgeSubsets  );

      if(!g_rank ) printinfo("Reading y edge subsets on this block:\n");
      read_and_print_subsets_on_set( g_sets[i], *l_yEdges, g_numEdgeSubsets, g_edgeSubsets  );

      if(!g_rank ) printinfo("Reading z edge subsets on this block:\n");
      read_and_print_subsets_on_set( g_sets[i], *l_zEdges, g_numEdgeSubsets, g_edgeSubsets  );


      if(!g_rank ) printinfo("Reading x face subsets on this block:\n");
      read_and_print_subsets_on_set( g_sets[i], *l_xFaces, g_numFaceSubsets, g_faceSubsets  );

      if(!g_rank ) printinfo("Reading y face subsets on this block:\n");
      read_and_print_subsets_on_set( g_sets[i], *l_yFaces, g_numFaceSubsets, g_faceSubsets  );

      if(!g_rank ) printinfo("Reading z face subsets on this block:\n");
      read_and_print_subsets_on_set( g_sets[i], *l_zFaces, g_numFaceSubsets, g_faceSubsets  );


      if(!g_rank ) printinfo("Reading elem subsets on this block:\n");
      read_and_print_subsets_on_set( g_sets[i], *l_elems, g_numElemSubsets, g_elemSubsets  );

      if(l_xCoords) free(l_xCoords);
      if(l_yCoords) free(l_yCoords);
      if(l_zCoords) free(l_zCoords);

      if(l_name) free(l_name);
    }

  
  if(!g_rank) printinfo("read_str_mesh: about to finalize_str_mesh_reader\n");
  finalize_str_mesh_reader( 1 );


  /*jake_malloc_summary();  */

 
#ifdef HAVE_PARALLEL
  MPI_Finalize();
#endif


  return(0);
}

