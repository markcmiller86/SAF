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
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */

#include "subset_transforms.h"



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void init_subset_transform( struct_subset_transform *a_subsetList )
{
  int i;
  a_subsetList->m_maxLength=1000;/*XXX*/
  a_subsetList->m_length=0;
  a_subsetList->m_cat=SS_CAT_NULL;
  a_subsetList->m_sets = (SAF_Set *)malloc(a_subsetList->m_maxLength * sizeof(SAF_Set));
  a_subsetList->m_transform = (int **)malloc(a_subsetList->m_maxLength * sizeof(int *));
  for(i=0;i<a_subsetList->m_maxLength;i++) 
    {
      a_subsetList->m_transform[i]=NULL;
    }
}

/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void delete_subset_transform( struct_subset_transform *a_subsetList )
{
  int i;
  if(a_subsetList->m_transform)
    {
      for(i=0;i<a_subsetList->m_maxLength;i++) 
	{
	  if(a_subsetList->m_transform[i])
	    {
	      free(a_subsetList->m_transform[i]);
	    }
	}
      free(a_subsetList->m_transform);
      a_subsetList->m_transform=NULL;
    }
  if(a_subsetList->m_sets)
    {
      free(a_subsetList->m_sets);
      a_subsetList->m_sets=NULL;
    }
}
/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
void resize_subset_transform( struct_subset_transform *a_subsetList )
{
  int i;
  int l_newLen = a_subsetList->m_maxLength*2;
  int **l_newTransform = (int **)malloc(l_newLen * sizeof(int *));
  SAF_Set *l_newSets = (SAF_Set *)malloc(l_newLen * sizeof(SAF_Set));
  memcpy(l_newSets,a_subsetList->m_sets,a_subsetList->m_maxLength*sizeof(SAF_Set));
  free(a_subsetList->m_sets);
  a_subsetList->m_sets = l_newSets;

  for(i=0;i<a_subsetList->m_maxLength;i++) l_newTransform[i]=NULL;
  memcpy(l_newTransform,a_subsetList->m_transform,a_subsetList->m_maxLength*sizeof(int *));
  free(a_subsetList->m_transform);
  a_subsetList->m_transform = l_newTransform;

  a_subsetList->m_maxLength=l_newLen;
}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 * THESE ARE JUST TEMPORARY DEBUG FUNCTIONS. THE STRINGS ARE NEVER FREED
 *--------------------------------------------------------------------------------------------------- */
/*const char *
get_rel_type_string( SAF_RelRep a_type )
{
  if( SAF_EQUIV(&a_type,SAF_TUPLES) ) return("SAF_TUPLES");
  if( SAF_EQUIV(&a_type,SAF_HSLAB) ) return("SAF_HSLAB");
  return("unknown?");
}
*/

char *get_set_name(SAF_Set a_set);
char *get_field_name(SAF_Field a_field);
char *get_cat_name(SAF_Cat a_cat);

char *get_set_name(SAF_Set a_set)
{
  char *l_name=NULL;
  saf_describe_set(SAF_ALL, &a_set, &l_name, NULL, NULL, NULL, NULL, NULL, NULL );
  return(l_name);
}

char *get_field_name(SAF_Field a_field)
{
  char *l_name=NULL;
  saf_describe_field(SAF_ALL, &a_field, NULL, &l_name, NULL, NULL, NULL, 
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

  return(l_name);
}
char *get_cat_name(SAF_Cat a_cat)
{
  char *l_name=NULL;
  saf_describe_category(SAF_ALL,&a_cat,&l_name,NULL,NULL);
  return(l_name);
}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
int get_subset_transforms( SAF_Db a_db, SAF_Set a_set, SAF_Cat a_cat, struct_subset_transform *a_subsetList )
{
  static SAF_Db *l_db=SAF_NOT_SET_DB;
  static int l_numSets=0;
  static SAF_Set *l_sets=NULL;

  int i,l_callingSet=-1,l_callingCount=-1;
  SAF_IndexSpec l_callingIspec;

  /*If this is the first set, add it to the list. If it is not the first set, then the calling
    set will have added it to the list already.*/
  if( !a_subsetList->m_length )
    {
      if(a_subsetList->m_length+1>=a_subsetList->m_maxLength) resize_subset_transform(a_subsetList);
      a_subsetList->m_sets[ a_subsetList->m_length ] = a_set;
      a_subsetList->m_transform[ a_subsetList->m_length ] = NULL;/*NULL means the default self xform (0,1,2,...)*/
      a_subsetList->m_length++;
      a_subsetList->m_cat=a_cat;
    }

  /*Figure out the index of the calling set in the set history*/
  for(i=a_subsetList->m_length-1;i>=0;i--)
    {
      if(SAF_EQUIV(&a_set,&(a_subsetList->m_sets[i])))
	{
	  l_callingSet=i;
	  break;
	}
    }
  if(l_callingSet<0)
    {
      printf("error l_callingSet<0  m_length=%d\n",a_subsetList->m_length);
      return(-1);
    }

  saf_describe_collection(SAF_ALL,&a_set,&a_cat,NULL,&l_callingCount,&l_callingIspec,NULL,NULL);
  if(l_callingCount<=0)
    {
      return(0);
    }

  /*First find all sets, to use later when finding subset relations.*/
  if( !SAF_EQUIV(l_db,&a_db) )
    {
      if( l_sets) 
	{
	  free(l_sets);
	  l_sets=NULL;
	  l_numSets=0;
	}
      l_db = &a_db;
      /*Note l_sets will leak once here*/
      saf_find_matching_sets(SAF_ALL, &a_db, SAF_ANY_NAME, SAF_SPACE, SAF_ANY_TOPODIM,
			     SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &l_numSets, &l_sets);
    }
      

            
  /*Find any subsets of this set*/
  for( i=0; i<l_numSets; i++ )
    {
      int j,l_found=0,l_numRels=0;
      SAF_Rel *l_rels=NULL;

      /*verify that we havent already examined this set*/
      for(j=0;j<a_subsetList->m_length;j++)
	{
	  if( SAF_EQUIV(&(l_sets[i]),&(a_subsetList->m_sets[j])) )
	    {
	      l_found=1;
	      break;
	    }
	}
      if(l_found) continue;
      

      /*XXX SAF_BOUNDARY_TORF doesnt seem to work?*/
      saf_find_subset_relations(SAF_ALL, &a_db, &a_set, &(l_sets[i]), &a_cat, &a_cat,
				SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,&l_numRels,&l_rels);

      /*XXX note assuming all 0-based for now*/

      if(l_numRels==1)
	{
	  SAF_RelRep l_relType;
	  size_t l_abuf_size=0,l_bbuf_size=0;
	  int j;
	  hid_t l_abuf_type,l_bbuf_type;
	  SAF_IndexSpec l_ispec;
	  int *l_callingSetXform = a_subsetList->m_transform[l_callingSet];
	  int *l_abuf=NULL;
	  int *l_thisXform=NULL;
	  int l_thisXformLength=0;
	  int l_success=1;

	  /*Read the subset relation*/
	  saf_describe_subset_relation(SAF_ALL,&(l_rels[0]),NULL,NULL,NULL,NULL,NULL,NULL,&l_relType,NULL);

	  /*use the IndexSpec of the superset*/
	  saf_describe_collection(SAF_ALL,&a_set,&a_cat,NULL,NULL,&l_ispec,NULL,NULL);

	  saf_get_count_and_type_for_subset_relation(SAF_ALL,&(l_rels[0]),NULL,&l_abuf_size,&l_abuf_type,&l_bbuf_size,&l_bbuf_type );

	  if( l_abuf_size>0 && H5Tequal(l_abuf_type,SAF_INT) && l_bbuf_size==0 && l_bbuf_type<0)/*H5Tequal(l_bbuf_type,H5I_INVALID_HID) )*/
	    {

	      saf_read_subset_relation(SAF_ALL,&(l_rels[0]),NULL,(void**)&l_abuf,NULL);

	      /*XXX need to check if actually was written*/

	      /*Note that with SAF_TUPLES we dont care (?) what the ndims of the subset is. Whether it is 
		1D or 3D does not change what we do here, which is, for each entry in the collection, get a tuple
		referring to the superset. But if the superset is 3D there is a question: do we refer to
		it with a 3D tuple, or with a 1D tuple (in which we assume we know the 1D representation
		int the superset collection?
	      */
	
	      if( SAF_EQUIV(&l_relType,SAF_TUPLES) && l_callingIspec.ndims==1 ) 
		{	 
		  if( l_ispec.origins[0] )
		    {

		      /*
		      printf("get_subset_transforms abuf: ");
		      for(j=0;j<(int)l_abuf_size;j++) printf("%d ", l_abuf[j]);
		      printf("\n");
		      */


		      for(j=0;j<(int)l_abuf_size;j++) 
			{
			  l_abuf[j] -= l_ispec.origins[0];
			  if(l_abuf[j]<0)
			    {
			      printf("error in get_subset_transforms: got index<0 after adjusting ispec.origin of %d\n",
				     l_ispec.origins[0]);
			      l_success=0;
			      break;
			    }
			}
		    }
		  if(l_success)
		    {
		      l_thisXform = l_abuf;
		      l_thisXformLength = l_abuf_size;
		    }
		}
	      else if( SAF_EQUIV(&l_relType,SAF_HSLAB) ) 
		{
		  int l_dim,l_numTuples;

		  /*note that a SAF hslab is start,start,start,count,count,count, stride,stride,stride (for e.g. 3D)*/

		  if( (int)l_abuf_size != l_callingIspec.ndims*3 )
		    {
		      printf("HSLAB error: l_abuf_size=%d l_callingIspec.ndims=%d\n",l_abuf_size,l_callingIspec.ndims);
		      if(l_abuf) free(l_abuf);
		      l_abuf=NULL;
		      continue;
		    }

		  l_numTuples=1;
		  for(l_dim=0;l_dim<l_callingIspec.ndims;l_dim++)
		    {
		      l_numTuples*=l_abuf[1*l_callingIspec.ndims+l_dim];
		    }
		  l_thisXformLength= l_numTuples*l_callingIspec.ndims;
		  l_thisXform = (int *)malloc(l_thisXformLength*sizeof(int));

		  /*
		    printf("SAF_HSLAB starts: ");
		    for(l_dim=0;l_dim<l_callingIspec.ndims;l_dim++) printf("%d ",l_abuf[l_dim]-l_callingIspec.origins[l_dim]);
		    printf("\n");
		    printf("SAF_HSLAB counts: ");
		    for(l_dim=0;l_dim<l_callingIspec.ndims;l_dim++) printf("%d ",l_abuf[1*l_callingIspec.ndims+l_dim]);
		    printf("     coll dim: ");
		    for(l_dim=0;l_dim<l_callingIspec.ndims;l_dim++) printf("%d ",l_callingIspec.sizes[l_dim]);
		    printf("\n");
		    printf("SAF_HSLAB strides: ");
		    for(l_dim=0;l_dim<l_callingIspec.ndims;l_dim++) printf("%d ",l_abuf[2*l_callingIspec.ndims+l_dim]);
		    printf("\n");
		  */

	


		  /*XXX we are writing the output in NDF? or NDC? ...INTERLEAVE? VECTOR?*/
		  for(l_dim=0;l_dim<l_callingIspec.ndims;l_dim++)
		    {
		      int l_start=l_abuf[l_dim]-l_callingIspec.origins[l_dim];
		      int l_count=l_abuf[1*l_callingIspec.ndims+l_dim];
		      int l_stride=l_abuf[2*l_callingIspec.ndims+l_dim];
		      int l_current=l_start;
		      int *l_ptr = &l_thisXform[l_dim];
		      int l_periodCounter=0;
		      int l_end = l_start+l_count*l_stride;

		      int l_period=1;
		      for(j=0;j<l_dim;j++) l_period*=l_abuf[1*l_callingIspec.ndims+j];

		      /*printf("   l_dim=%d l_period=%d\n",l_dim,l_period);*/

		      for(j=0;j<l_numTuples;j++)
			{		  
			  l_ptr[0] = l_current;
			  l_ptr += l_callingIspec.ndims;
			  l_periodCounter++;
			  if(l_periodCounter>=l_period)
			    {
			      l_current+=l_stride;
			      l_periodCounter=0;
			    }		 
			  if( l_current>=l_end )
			    {
			      l_current=l_start;
			      l_periodCounter=0;
			    }		      
			}
		    }


	    
		}
	      else
		{
		  /* Note the abuf_type SAF_OFFSET relations all seem to be from TOP_SET to other_set SAF_INT & SAF_CELLTYPE_SET rels*/
		  l_success=0;
		}


	      if(l_success)
		{

		  /*
		    printf("%d -> %d subset rel(%p): ",l_callingSet,i,l_thisXform);
		    if( l_callingIspec.ndims==3 )
		    for(j=0;j<l_thisXformLength;j+=3) 
		    {
		    printf("%d %d %d, ",l_thisXform[j],l_thisXform[j+1],l_thisXform[j+2]);
		    if(!((j+1)%10)) printf("\n");
		    }
		    else
		    for(j=0;j<l_thisXformLength;j++) 
		    {
		    printf("%d ",l_thisXform[j]);
		    if(!((j+1)%10)) printf("\n");
		    }
		    printf("\n");
		  */


		  if(l_callingSetXform && l_thisXform)
		    {
		      for(j=0;j<l_thisXformLength;j++) 
			{
			  if(l_thisXform[j]<0 || l_thisXform[j]>=l_callingCount)
			    {
			      printf("error get_subset_transforms: subset rel index (%d) is out of range (%d)\n",
				     l_thisXform[j], l_callingCount );
			      l_success=0;
			      break;
			    }
			  else
			    {
			      l_thisXform[j] = l_callingSetXform[ l_thisXform[j] ];
			    }
			}

		      /*
			printf("     xformed to: ");
			for(j=0;j<l_thisXformLength;j++)
			{
			printf("%d ",l_thisXform[j]);
			if(j>10) { printf("..."); break; }
			}
			printf("\n");
		      */
		    }


		  /*printf("just set a_subsetList->m_transform=%p   entry=%d   to=%p\n",
		    a_subsetList->m_transform, a_subsetList->m_length, l_thisXform);*/

		  if(l_success)
		    {
		      /*add this set and xform to the set list, and recursively call this function on this set*/
		      if(a_subsetList->m_length+1>=a_subsetList->m_maxLength) resize_subset_transform(a_subsetList);
		      a_subsetList->m_sets[ a_subsetList->m_length ] = l_sets[i];
		      a_subsetList->m_transform[ a_subsetList->m_length ] = l_thisXform;      
		      a_subsetList->m_length++;

		      if( get_subset_transforms(a_db,l_sets[i],a_cat,a_subsetList) ) 
			{
			  return(-1);
			}
		    }
		}
	      if(l_abuf && l_abuf!=l_thisXform) free(l_abuf);

	    }
	}
      else if(l_numRels>1)
	{
	  printf("error l_numRels=%d between two sets on same cat!\n",l_numRels);
	  return(-1);
	}

      if(l_rels) free(l_rels);
    }

  return(0);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
int read_field_from_superset(
#ifdef __cplusplus
			   SAFStrMeshReader *a_strReader,
#endif
			   SAF_Field a_field, SAF_Set a_subset, struct_subset_transform a_subsetList, MY_PRECISION **a_data )
{
  int i,j;
  int l_subsetIndex=-1;
  int l_numEntriesInColl=0,l_numEntriesInSubsetColl=0,l_numFieldComponents=0;
  hid_t l_fieldDsltype;
  size_t l_fieldCount=0;
  int l_numFloatsInInput=0,l_numFloatsInOutput=0;
  SAF_Interleave l_interleave;

  /*Assume this is done before
    if(read_field_from_superset_check( a_field, a_subset, a_subsetList )) return(-1);
  */
  for(i=0;i<a_subsetList.m_length;i++)
    {
      if( SAF_EQUIV(&a_subset,&(a_subsetList.m_sets[i])) )
	{
	  l_subsetIndex=i;
	  break;
	}
    }

  saf_describe_field(SAF_ALL, &a_field, NULL, NULL, NULL, NULL, NULL, 
		     NULL,NULL, NULL, NULL, NULL, NULL, &l_numFieldComponents, NULL, &l_interleave, NULL);

  saf_describe_collection(SAF_ALL,&(a_subsetList.m_sets[0]),&(a_subsetList.m_cat),NULL,&l_numEntriesInColl,
			  NULL,NULL,NULL);

  saf_describe_collection(SAF_ALL,&(a_subsetList.m_sets[l_subsetIndex]),&(a_subsetList.m_cat),NULL,&l_numEntriesInSubsetColl,
			  NULL,NULL,NULL);

  saf_get_count_and_type_for_field(SAF_ALL, &a_field, NULL, &l_fieldCount, &l_fieldDsltype);


      
  /*
    printf("read_field_from_superset: l_numFieldComponents=%d l_fieldCount=%d l_numEntriesInColl=%d l_numEntriesInSubsetColl=%d\n",
    l_numFieldComponents,l_fieldCount,l_numEntriesInColl,l_numEntriesInSubsetColl);
  */



  /*
  ** Read and transform the field
  */
  l_numFloatsInInput = l_numEntriesInColl*l_numFieldComponents;
  l_numFloatsInOutput = l_numEntriesInSubsetColl*l_numFieldComponents;

  if(!l_subsetIndex)/*i.e. l_numFloatsInInput==l_numFloatsInOutput*/
    {
      /*This is a trivial case: the field's set is the same as the subset*/

      if(!a_data[0]) a_data[0] = (MY_PRECISION *)malloc(l_numFloatsInInput*sizeof(MY_PRECISION));

#ifdef __cplusplus
      a_strReader->read_whole_field_to_my_precision( &a_field, l_fieldDsltype, (size_t)l_numFloatsInInput, a_data[0]);
#else
      read_whole_field_to_my_precision( &a_field, l_fieldDsltype, (size_t)l_numFloatsInInput, a_data[0]);
#endif
    }
  else
    {
      /*Read the field into a temp buffer, then transform the field into the argument buffer*/
      int *l_xform = a_subsetList.m_transform[l_subsetIndex];
      MY_PRECISION *l_inData= (MY_PRECISION *)malloc(l_numFloatsInInput*sizeof(MY_PRECISION));
      int l_min = l_numFloatsInOutput;
      if(l_min>l_numFloatsInInput) l_min=l_numFloatsInInput;


      if(!a_data[0]) a_data[0] = (MY_PRECISION *)malloc(l_numFloatsInOutput*sizeof(MY_PRECISION));
      


#ifdef __cplusplus
      a_strReader->read_whole_field_to_my_precision(&a_field, l_fieldDsltype, (size_t)l_numFloatsInInput, l_inData);
#else
      read_whole_field_to_my_precision( &a_field, l_fieldDsltype, (size_t)l_numFloatsInInput, l_inData);	
#endif



#if 0
      printf("superset indata:");
      for(j=0;j<l_numFloatsInInput;j++) 
	{
	  printf(" %.1f",l_inData[j]);
	  if(l_numFieldComponents>1 && l_interleave==SAF_INTERLEAVE_VECTOR && !((j+1)%l_numFieldComponents)) 
	    printf(",");
	  else if(l_numFieldComponents>1 && l_interleave==SAF_INTERLEAVE_COMPONENT && !((j+1)%l_numEntriesInColl))
	    printf(",");
	}
      printf("\n");
#endif




      if(!l_xform)
	{
	  /*There is no transform, so it is the default: 0,1,2,3,...*/
	  memcpy(a_data[0],l_inData,l_min*sizeof(MY_PRECISION));
	}
      else
	{
	  MY_PRECISION *l_to = a_data[0];
	  /* printf("read_field_from_superset about to transform %d to %d, num components=%d  topset=%s subset=%s field=%s\n",
	     l_numEntriesInColl,l_numEntriesInSubsetColl,l_numFieldComponents,
	     get_set_name(a_subsetList.m_sets[0]),get_set_name(a_subset),get_field_name(a_field) );*/
	  

	  if(l_numFieldComponents==1 || l_interleave==SAF_INTERLEAVE_VECTOR )
	    {
	      for(i=0;i<l_numEntriesInSubsetColl;i++)
		{
		  for(j=0;j<l_numFieldComponents;j++)
		    {
		      l_to[0] = l_inData[ l_numFieldComponents*l_xform[i]+j ];
		      l_to++;
		    }
		}
	    }
	  else if( l_interleave==SAF_INTERLEAVE_COMPONENT )
	    {
	      for(j=0;j<l_numFieldComponents;j++)	      
		{
		  int l_position = l_numEntriesInColl*j;
		  for(i=0;i<l_numEntriesInSubsetColl;i++)
		    {
		      l_to[0] = l_inData[l_position+l_xform[i]];
		      l_to++;
		    }
		}
	    }
	  else
	    {
	      printf("\nerror read_field_from_superset invalid interleave\n\n");
	      exit(-1);
	    }





	}
      if(l_inData) free(l_inData);



    }
  

  return(0);
}





/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
int read_field_from_superset_check( SAF_Field a_field, SAF_Set a_subset, struct_subset_transform a_subsetList )
{
  int i,l_subsetIndex=-1;
  hid_t l_fieldDsltype;
  size_t l_fieldCount=0;
  int l_numFieldComponents=0;

  if( !my_saf_is_valid_field_handle(&a_field) ) 
    {
      printf("error read_field_from_superset_check: invalid field\n");
      return(-1);
    }
  if( !my_saf_is_valid_set_handle(&a_subset) ) 
    {
      printf("error read_field_from_superset_check: invalid set\n");
      return(-1);
    }
  for(i=0;i<a_subsetList.m_length;i++)
    {
      if( SAF_EQUIV(&a_subset,&(a_subsetList.m_sets[i])) )
	{
	  l_subsetIndex=i;
	  break;
	}
    }
  if( l_subsetIndex<0 ) 
    {
      printf("error read_field_from_superset_check: set is not in set history\n");
      return(-1);
    }
  {
    SAF_Set l_set;
    SAF_Cat l_cat;
    SAF_Interleave l_interleave;
    saf_describe_field(SAF_ALL, &a_field, NULL, NULL, &l_set, NULL, NULL, 
		       NULL, &l_cat, NULL, NULL, NULL, NULL, &l_numFieldComponents, NULL, &l_interleave, NULL);
    if( !SAF_EQUIV(&l_set,&(a_subsetList.m_sets[0])) )
      {
	printf("error read_field_from_superset_check: set and set history dont match\n");
	return(-1);
      }
    if( !SAF_EQUIV(&l_cat,&(a_subsetList.m_cat)) )
      {
	printf("error read_field_from_superset_check: cat and set history dont match\n");
	return(-1);
      }
    if(l_numFieldComponents<1)
      {
	/*Note this usually happens because the field is INhomogeneous.*/
	/*printf("error read_field_from_superset_check: num field components=%d   INhomogeneous?\n",l_numFieldComponents);*/
	return(-1);
      }
    if(l_numFieldComponents!=1 && l_interleave!=SAF_INTERLEAVE_VECTOR && l_interleave!=SAF_INTERLEAVE_COMPONENT )
      {
	printf("error read_field_from_superset_check: bad interleave\n");
	return(-1);
      }
  }

  saf_get_count_and_type_for_field(SAF_ALL, &a_field, NULL, &l_fieldCount, &l_fieldDsltype);
  if(!l_fieldCount)
    {
      /*printf("error read_field_from_superset_check: saf_get_count_and_type_for_field gets count 0(field=%s)\n",get_field_name(a_field));*/
      return(-1);
    }
      
  /*read_whole_field_to_my_precision handles only these 3 types*/
  if( H5Tequal(l_fieldDsltype,SAF_FLOAT) || H5Tequal(l_fieldDsltype,SAF_DOUBLE) || 
      H5Tequal(l_fieldDsltype,SAF_INT) || H5Tequal(l_fieldDsltype,SAF_LONG) )
    {
      return(0); /*THIS IS THE ONLY READABLE CONDITION*/
    }
  else
    {
#if 1
      /*doing it this way because saf_data_has_been_written_to_comp_field can cause core dump?*/
      printf("error read_field_from_superset_check: cant handle dsl type yet AND/OR data is not written to comp field\n");
      return(-1);
#else
  
      hbool_t l_result;
      saf_data_has_been_written_to_comp_field(SAF_ALL,a_field,&l_result);
      if( l_result )
	{
	  printf("read_field_from_superset_check: cant handle dsl type yet, written to composite field, num comps=%d (should be 1) field=%s\n",
		  l_numFieldComponents, get_field_name(a_field) );
	  return(-1);
	}
      else
	{
	  printf("error read_field_from_superset_check: cant handle dsl type yet, and data is not written to comp field\n" );
	  return(-1);
	}
#endif	   
    }

}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
int read_field_from_subset_check( SAF_Field a_field, SAF_Set a_subset, struct_subset_transform a_subsetList )
{
  int i,l_subsetIndex=-1;
  hid_t l_fieldDsltype;
  size_t l_fieldCount=0;
  int l_numFieldComponents=0;

  if( !my_saf_is_valid_field_handle(&a_field) ) 
    {
      printf("error read_field_from_subset_check: invalid field\n");
      return(-1);
    }
  for(i=0;i<a_subsetList.m_length;i++)
    {
      if( SAF_EQUIV(&a_subset,&(a_subsetList.m_sets[i])) )
	{
	  l_subsetIndex=i;
	  break;
	}
    }
  if( l_subsetIndex<0 ) 
    {
      printf("error read_field_from_subset_check: set is not in set history\n");
      return(-1);
    }
  {
    SAF_Set l_set;
    SAF_Cat l_cat;
    SAF_Interleave l_interleave;
    saf_describe_field(SAF_ALL, &a_field, NULL, NULL, &l_set, NULL, NULL, 
		       NULL, &l_cat, NULL, NULL, NULL, NULL, &l_numFieldComponents, NULL, &l_interleave, NULL);
    if( !SAF_EQUIV(&l_set,&(a_subsetList.m_sets[l_subsetIndex])) || !SAF_EQUIV(&l_set,&a_subset) )
      {
	printf("error read_field_from_subset_check: set and set history dont match, l_subsetIndex=%d\n",l_subsetIndex);
	return(-1);
      }
    if( !SAF_EQUIV(&l_cat,&(a_subsetList.m_cat)) )
      {
	printf("error read_field_from_subset_check: cat and set history dont match\n");
	return(-1);
      }
    if(l_numFieldComponents<1)
      {
	/*Note this usually happens because the field is INhomogeneous.*/
	/*printf("error read_field_from_subset_check: num field components=%d   INhomogeneous?\n",l_numFieldComponents);*/
	return(-1);
      }
    if(l_numFieldComponents!=1 && l_interleave!=SAF_INTERLEAVE_VECTOR && l_interleave!=SAF_INTERLEAVE_COMPONENT )
      {
	printf("error read_field_from_subset_check: bad interleave\n");
	return(-1);
      }
  }

  saf_get_count_and_type_for_field(SAF_ALL, &a_field, NULL, &l_fieldCount, &l_fieldDsltype);
  if(!l_fieldCount)
    {
      printf("error read_field_from_subset_check: saf_get_count_and_type_for_field gets count 0 (field=%s)\n",get_field_name(a_field));
      return(-1);
    }
      
  /*read_whole_field_to_my_precision handles only these 3 types*/
  if( H5Tequal(l_fieldDsltype,SAF_FLOAT) || H5Tequal(l_fieldDsltype,SAF_DOUBLE) || 
      H5Tequal(l_fieldDsltype,SAF_INT) || H5Tequal(l_fieldDsltype,SAF_LONG) )
    {
      return(0); /*THIS IS THE ONLY READABLE CONDITION*/
    }
  else
    {
#if 1
      /*doing it this way because saf_data_has_been_written_to_comp_field can cause core dump?*/
      printf("error read_field_from_subset_check: cant handle dsl type yet AND/OR data is not written to comp field\n");
      return(-1);
#else
  
      hbool_t l_result;
      saf_data_has_been_written_to_comp_field(SAF_ALL,a_field,&l_result);
      if( l_result )
	{
	  printf("read_field_from_subset_check: cant handle dsl type yet, written to composite field, num comps=%d (should be 1) field=%s\n",
		  l_numFieldComponents, get_field_name(a_field) );
	  return(-1);
	}
      else
	{
	  printf("error read_field_from_superset_check: cant handle dsl type yet, and data is not written to comp field\n" );
	  return(-1);
	}
#endif	   
    }

}




/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
int read_field_from_subset(
#ifdef __cplusplus
			   SAFStrMeshReader *a_strReader,
#endif
 SAF_Field a_field, SAF_Set a_subset, struct_subset_transform a_subsetList, MY_PRECISION **a_data,
			     int **a_truth )
{
  int i,j;
  int l_subsetIndex=-1;
  int l_numEntriesInColl=0,l_numEntriesInSubsetColl=0,l_numFieldComponents=0;
  hid_t l_fieldDsltype;
  size_t l_fieldCount=0;
  int l_numFloatsInInput=0,l_numFloatsInOutput=0;
  SAF_Interleave l_interleave;

  /*Assume this is done before
    if(read_field_from_subset_check( a_field, a_subset, a_subsetList )) return(-1);
  */
  for(i=0;i<a_subsetList.m_length;i++)
    {
      if( SAF_EQUIV(&a_subset,&(a_subsetList.m_sets[i])) )
	{
	  l_subsetIndex=i;
	  break;
	}
    }

  saf_describe_field(SAF_ALL, &a_field, NULL, NULL, NULL, NULL, NULL, 
		     NULL,NULL, NULL, NULL, NULL, NULL, &l_numFieldComponents, NULL, &l_interleave, NULL);

  saf_describe_collection(SAF_ALL,&(a_subsetList.m_sets[0]),&(a_subsetList.m_cat),NULL,&l_numEntriesInColl,
			  NULL,NULL,NULL);

  saf_describe_collection(SAF_ALL,&(a_subsetList.m_sets[l_subsetIndex]),&(a_subsetList.m_cat),NULL,&l_numEntriesInSubsetColl,
			  NULL,NULL,NULL);

  saf_get_count_and_type_for_field(SAF_ALL, &a_field, NULL, &l_fieldCount, &l_fieldDsltype);


      
  /* printf("read_field_from_subset: l_numFieldComponents=%d l_fieldCount=%d l_numEntriesInColl=%d l_numEntriesInSubsetColl=%d\n",
     l_numFieldComponents,l_fieldCount,l_numEntriesInColl,l_numEntriesInSubsetColl);*/
  



  /*
  ** Read and transform the field
  */
  l_numFloatsInOutput = l_numEntriesInColl*l_numFieldComponents;
  l_numFloatsInInput = l_numEntriesInSubsetColl*l_numFieldComponents;

  if(!l_subsetIndex)/*i.e. l_numFloatsInInput==l_numFloatsInOutput*/
    {
      /*This is a trivial case: the field's set is the same as the subset*/

      /*printf("read_field_from_subset trivial transform\n");*/


      if(!a_data[0]) a_data[0] = (MY_PRECISION *)malloc(l_numFloatsInInput*sizeof(MY_PRECISION));

#ifdef __cplusplus
      a_strReader->read_whole_field_to_my_precision( &a_field, l_fieldDsltype, (size_t)l_numFloatsInInput, a_data[0]);
#else
      read_whole_field_to_my_precision( &a_field, l_fieldDsltype, (size_t)l_numFloatsInInput, a_data[0]);
#endif

      if(!a_truth[0]) a_truth[0] = (int *)malloc(l_numEntriesInColl*sizeof(int));
      for(i=0;i<l_numEntriesInColl;i++) a_truth[0][i]=1;/*all entries are valid*/
    }
  else
    {
      /*Read the field into a temp buffer, then transform the field into the argument buffer*/
      int *l_xform = a_subsetList.m_transform[l_subsetIndex];
      MY_PRECISION *l_inData= (MY_PRECISION *)malloc(l_numFloatsInInput*sizeof(MY_PRECISION));
      int l_min = l_numFloatsInOutput;
      if(l_min>l_numFloatsInInput) l_min=l_numFloatsInInput;

      if(!a_data[0]) a_data[0] = (MY_PRECISION *)malloc(l_numFloatsInOutput*sizeof(MY_PRECISION));
      if(!a_truth[0]) 
	{
	  a_truth[0] = (int *)malloc(l_numEntriesInColl*sizeof(int));
	  for(i=0;i<l_numEntriesInColl;i++) a_truth[0][i]=0;/*start with all entries invalid*/
	}
      /*Note that if we didnt allocate the truth data here, then we are trusting it was already
	set by the calling function!*/

#ifdef __cplusplus
      a_strReader->read_whole_field_to_my_precision( &a_field, l_fieldDsltype, (size_t)l_numFloatsInInput, l_inData);
#else
      read_whole_field_to_my_precision( &a_field, l_fieldDsltype, (size_t)l_numFloatsInInput, l_inData);	
#endif

      if(!l_xform)
	{
	  /*There is no transform, so it is the default: 0,1,2,3,...*/
	  memcpy(a_data[0],l_inData,l_min*sizeof(MY_PRECISION));
	  for(i=0;i<l_min;i++) a_truth[0][i]=1;
	}
      else
	{
	  MY_PRECISION *l_from = l_inData;
	  /*printf("read_field_from_subset about to transform(expand) %d to %d, num components=%d\n",
	    l_numEntriesInSubsetColl,l_numEntriesInColl,l_numFieldComponents );*/
	  

	  if(l_numFieldComponents==1 || l_interleave==SAF_INTERLEAVE_VECTOR )
	    {
	      for(i=0;i<l_numEntriesInSubsetColl;i++)
		{
		  int l_position = l_numFieldComponents*l_xform[i];
		  a_truth[0][ l_xform[i] ]=1;
		  for(j=0;j<l_numFieldComponents;j++)
		    {
		      a_data[0][l_position+j] = l_from[0];
		      l_from++;
		    }
		}
	    }
	  else if( l_interleave==SAF_INTERLEAVE_COMPONENT )
	    {
	      for(j=0;j<l_numFieldComponents;j++)	      
		{
		  int l_position = l_numEntriesInColl*j;
		  for(i=0;i<l_numEntriesInSubsetColl;i++)
		    {
		      if(!j) a_truth[0][ l_xform[i] ]=1;
		      a_data[0][l_position+l_xform[i]] = l_from[0];
		      l_from++;
		    }
		}
	    }
	  else
	    {
	      printf("\nerror read_field_from_subset invalid interleave\n\n");
	      exit(-1);
	    }


	  /*XXX zero any unused slots, just in case the calling function doesnt handle them*/
	  if(l_numFieldComponents==1 || l_interleave==SAF_INTERLEAVE_VECTOR )
	    {
	      for(i=0;i<l_numEntriesInColl;i++)
		{
		  if( !a_truth[0][i] )
		    {
		      for(j=0;j<l_numFieldComponents;j++)	      
			{
			  a_data[0][l_numFieldComponents*i+j]=0.0;
			}
		    }
		}
	    }
	  else
	    {
	      for(i=0;i<l_numEntriesInColl;i++)
		{
		  if( !a_truth[0][i] )
		    {
		      for(j=0;j<l_numFieldComponents;j++)	      
			{
			  a_data[0][j*l_numEntriesInColl+i]=0.0;
			}
		    }
		}
	    }




	  /*print temporary*/
	  /*
	    for(i=0;i<l_numEntriesInColl;i++)
	    {
	    if( a_truth[0][i] )
	    {
	    printf("DATA %d     %d     ",i,a_truth[0][i]);
	    for(j=0;j<l_numFieldComponents;j++)
	    {
	    printf("%f ",a_data[0][i*l_numFieldComponents+j] );
	    }
	    printf("\n");
	    }
	    }
	  */



	}
      if(l_inData) free(l_inData);
    }
  

  return(0);
}



/*----------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     
 * Description:
 *
 *
 *--------------------------------------------------------------------------------------------------- */
int transform_connectivity_list( SAF_Set a_subset, struct_subset_transform a_subsetList, int a_length, int *a_data )
{
  int i;
  int l_subsetIndex=-1;
  int *l_xform=NULL;

  for(i=0;i<a_subsetList.m_length;i++)
    {
      if( SAF_EQUIV(&a_subset,&(a_subsetList.m_sets[i])) )
	{
	  l_subsetIndex=i;
	  break;
	}
    }
  if( l_subsetIndex<0 ) 
    {
      printf("error transform_connectivity_list: set is not in set history\n");
      return(-1);
    }

  l_xform = a_subsetList.m_transform[l_subsetIndex];
  if(l_xform) /*if NULL, then it is the default xform: 0,1,2,3... do nothing*/
    {
      for(i=0;i<a_length;i++)
	{
	  a_data[i] = l_xform[ a_data[i] ];
	}
    }
  return(0);
}
