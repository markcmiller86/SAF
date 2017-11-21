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


#include <jni.h>
#include "saf_0005fwrap.h"
#include <saf.h>
#include <stdio.h>
#include <stdlib.h>

SAF_DbProps *p=NULL;
SAF_Db *db=NULL;
char *dbname;
char *mypwd;

SAF_Cat *p_cats;
SAF_Set *top_sets, *all_sets, *suite_sets, *suite_subsets;
SAF_CellType celltype;


typedef struct {
	SAF_Cat the_cat;
	char *cat_name;
	int count;
	char *celltype;
	char *ispec;
	unsigned char is_decomp;
	void *child_setlink;   
	void *parent_setlink;
	void *topo_link;
	void *topo_domain_id;
	int parent_colllink;
	int num_subset_relations;
	SAF_Rel *subset_relation;
} Collection_Info;	

typedef Collection_Info *Collections;


typedef struct {

	SAF_FieldTmpl *fieldtmpl;
	char *fieldtmpl_name;
	int num_comps;
	char *algtype;
	char *basis;
	SAF_Quantity *quant;
	void *component_tmpls; /* FieldTmpl_Info * component_tmpls */
} FieldTmpl_Info;	


typedef struct {
	SAF_Field *the_field;
	FieldTmpl_Info *template_info;
	char *field_name;
	void *base_space; /* (Set_Info *) */
	char *base_space_name;
	jboolean is_coord;
	int my_set_id;
	int my_set_coll_num;
	SAF_Cat *eval_coll;
	SAF_Cat *coeff_assoc;
	int assoc_ratio;
	SAF_Cat *storage_decomp;
	char *eval_func;
	char *data_type;
	int data_size;
	char *interleave;
	int num_comps;
	int comp_order;
	void *component_fields; /* Field_Info * component_fields */
} Field_Info;


typedef Field_Info *Fields;
	

typedef struct {
	SAF_Set the_set;
	char *set_name;
	char *silrole;
  SAF_SilRole srole;
	char *extmode;
	char *topmode;
	int max_topo_dim;
	int num_sets;
	int num_colls;
	int num_fields;
	Collections collections;
	int num_topo_relations;
	SAF_Rel *topo_relation;
	Fields fields;
} Set_Info;

typedef Set_Info *Set;

Set tops;
Set allsets;
Set suitesets;
Set suitesubsets;


void *found_handle;
size_t nfound;

int num_top_sets, num_all_sets, num_suite_sets, number_suite_subsets, num_time_sets, num_param_sets;

int get_topo_range_set( int );
void find_collections( Set_Info *, int );
void find_fields_on_set( Set_Info *, int );
char * saf_set_name( SAF_Set * );
char * role_string( int );
char * silrole_string( SAF_SilRole );
char * extmode_string( int );
char * celltype_string( int );
char * objecttype_string( int );
/* char * algtype_string( int ); */
char *algtype_string( SAF_Algebraic * );
char * StringDataType( hid_t );
char * interleave_string( int );
/* char * basis_string( int ); */
char * basis_string( SAF_Basis * ); 
/* char * evalfunc_string( int ); */
char * evalfunc_string( SAF_Eval *); 
void get_set_info( Set_Info * );
void get_fieldtmpl_info( FieldTmpl_Info * );
void get_field_info( Field_Info * );
char *StringDataTypeValue(hid_t , ss_pers_t *);
void find_topo_relations( Set_Info *);

char *saf_cat_name( SAF_Cat *SAF_cat)
{
        char *cat_name=NULL;
                                                                                                                             
        saf_describe_category(SAF_ALL, SAF_cat, &cat_name, NULL, NULL);
                                                                                                                             
        return cat_name;
                                                                                                                             
}


char *StringDataTypeValue(hid_t type, ss_pers_t *value)
{
  char string_c[80];
                                                                                                                                                                                                                                                                               
  sprintf(string_c, "Unknown data type.... %ld",(long)type);
  if (type < 0)
     sprintf(string_c, "Unknown data type=%ld ",(long)type);
                                                                                                                                                                                                                                                                               
  if (0) {
     if(H5Tequal(type,SAF_INT)) {
     int *p_i;
     p_i = (int *)value;
     sprintf(string_c,"SAF_INT; value: %d",(int)(p_i[0]));
                                                                                                                                                                                                                                                                               
     return strdup(string_c);
    }
  else if(H5Tequal(type,SAF_LONG)) {
     long *p_l;
     p_l = (long *)value;
     sprintf(string_c,"SAF_LONG; value: %ld",(long)(p_l[0]));
     return strdup(string_c);
    }
  else if(H5Tequal(type,SAF_HANDLE)) sprintf(string_c, "SAF_HANDLE; value: xxx");
  else if(H5Tequal(type,SAF_FLOAT)) sprintf(string_c, "SAF_FLOAT; value: xxx");
  else if(H5Tequal(type,SAF_DOUBLE)) sprintf(string_c, "SAF_DOUBLE value: xxx");
  else if(H5Tequal(type,SAF_CHAR)) sprintf(string_c, "SAF_CHAR value: xxx");
  else if(H5T_STRING==H5Tget_class(type)) sprintf(string_c, "SAF_STRING value: xxx");
  else if(H5Tequal(type,H5T_NATIVE_SIZE)) sprintf(string_c, "SAF_SIZE value: xxx");
/*
  if(H5Tequal(type, SAF_INT8)) return "SAF_INT8 value: xxx";
  if(H5Tequal(type, SAF_INT16)) return "SAF_INT16 value: xxx";
  if(H5Tequal(type, SAF_INT32)) return "SAF_INT32 value: xxx";
  if(H5Tequal(type, SAF_INT64)) return "SAF_INT64 value: xxx";
  if(H5Tequal(type, SAF_LONG_LONG)) return "SAF_LONG_LONG value: xxx";
*/
  }

  return strdup(string_c);
}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1number_1of_1sets(
	JNIEnv *env,
	jobject obj,
	jint set_id)
{

	Set_Info *the_set;

	if( set_id == 0 ) 
		return 0;

	the_set = (Set_Info *)set_id;

	return (jint) (the_set->num_sets);
}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1coll_1info(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint coll_id)
{
	char name[100];
	char buffer[100];
	Set_Info *set_info;
	SAF_Set saf_set;
	char *setname;
	int max_topo_dim;
	SAF_Cat saf_cat;
	SAF_SilRole silrole;
	SAF_ExtendMode extendmode;
	SAF_TopMode topmode;

	set_info = (Set_Info *)set_id;
	saf_set = set_info->the_set;
	saf_cat = set_info->collections[coll_id].the_cat;
	setname = NULL;
	saf_describe_set(SAF_ALL,&saf_set,&setname,&max_topo_dim,&silrole,&extendmode,&topmode,NULL,NULL);
	saf_describe_collection(SAF_ALL,&saf_set,&saf_cat,&celltype,NULL,NULL,NULL,NULL);
	
	strcpy(name,setname);
	strcat(name,":\n\t");
	strcat(name,set_info->collections[coll_id].cat_name);
	strcat(name,":\n\t\t");
	strcat(name,silrole_string(silrole));
	strcat(name,"\n\t\t");
	sprintf(buffer,"max_topo_dim: %d, ",max_topo_dim);
	strcat(name,buffer);
        char *str = celltype_string((int) celltype);
	sprintf(buffer,"\n\t\tcelltype: %s, ",str);
	strcat(name,buffer);
	

	return (*env)->NewStringUTF(env,name);	

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1a_1set(
	JNIEnv *env, 
	jobject obj,
	jint set_id,
	jint set_num)
{

	int num_sets;

	Set_Info *set_info;
	Set_Info *the_set;

	set_info = (Set_Info *)set_id;
	
	num_sets = set_info->num_sets;

	if( set_num > (num_sets - 1) )
		return (jint)set_info;
	else
		return (jint)(set_info + set_num);
}

JNIEXPORT jboolean JNICALL Java_saf_1wrap_native_1is_1set_1equiv(
	JNIEnv *env, 
	jobject obj,
	jint set_1,
	jint set_2)
{

	Set_Info *set1;
	Set_Info *set2;

	set1 = (Set_Info *)set_1;
	set2 = (Set_Info *)set_2;

	if( SAF_EQUIV(&(set1->the_set), &(set2->the_set)) ) {
		return (jboolean)1;
	}

	return (jboolean)0;
}

void find_topo_relations( Set_Info *set_info)
{

	int num_rels;
	SAF_Rel *rels;


	if( set_info->topo_relation != NULL )
		return;


	rels = NULL;
	saf_find_topo_relations(SAF_ALL,db,&(set_info->the_set),NULL,&num_rels,NULL);
	saf_find_topo_relations(SAF_ALL,db,&(set_info->the_set),NULL,&num_rels,&rels);

	set_info->num_topo_relations = num_rels;

	if( num_rels > 0 )
		set_info->topo_relation = rels;
	else
		set_info->topo_relation = NULL;

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1range_1set(
	JNIEnv *env,
	jobject obj,
	jint rel_id)
{

	int i;

	i = get_topo_range_set( rel_id );
	return (jint)(allsets+i);
}


int get_topo_range_set( int rel_id )
{
	int i;
	
	SAF_Rel *topo_rel;
	SAF_Set partSet, glueSet;
	SAF_Cat partCat, glueCat;
	SAF_RelRep trtype;

	topo_rel = (SAF_Rel *)rel_id;
	saf_describe_topo_relation(SAF_ALL,topo_rel,&partSet, &partCat, &glueSet, &glueCat, NULL, &trtype, NULL);

	for( i = 0; i < num_all_sets; i++ ) {
		if( SAF_EQUIV( &(allsets[i].the_set), &glueSet ) ) {
			return i;
		}
	}

	return 0;

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1range_1coll(
	JNIEnv *env,
	jobject obj,
	jint rel_id)
{

	int i, num_colls;

	int the_set=0;
	Set_Info *set_info;
	SAF_Rel *topo_rel;
	SAF_Set partSet, glueSet;
	SAF_Cat partCat, glueCat;
	SAF_RelRep trtype;

	the_set = get_topo_range_set( rel_id ); 
	set_info = allsets+the_set;
	num_colls = set_info->num_colls;

	topo_rel = (SAF_Rel *)rel_id;
	saf_describe_topo_relation(SAF_ALL,topo_rel,&partSet, &partCat, &glueSet, &glueCat, NULL, &trtype, NULL);

	for( i = 0; i < num_colls; i++ ) {
		if( SAF_EQUIV( &(set_info->collections[i].the_cat), &glueCat) ) {
			set_info->collections[i].topo_domain_id = (void *)rel_id;
			return i;
		}
	}

	return 0;
}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1topo_1domain_1id(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint coll_id)
{

	Set_Info *set_info;
	void *domain_id;
	
	set_info = (Set_Info *)set_id;


	domain_id = set_info->collections[coll_id].topo_domain_id;	

	if( domain_id == NULL )
		return 0;
	else
		return (jint)(domain_id);

}



JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1topo_1relation(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint coll_id)
{

	int i,j;

	Set_Info *set_info;
	Set_Info *all_set;

	set_info = (Set_Info *)set_id;

	for( i = 0 ; i < num_all_sets; i++ ) {	

		if( SAF_EQUIV(&(allsets[i].the_set), &(set_info->the_set)) ) {
			for( j = 0; j < set_info->num_colls; j++ ) {

				if( SAF_EQUIV(&(allsets[i].collections[j].the_cat), &(set_info->collections[coll_id].the_cat)) ) {	
					/* printf("in Java_saf_1wrap_native_1get_1topo_1relation, got a set, coll match, returning collection topo_link\n"); */

					return (jint)(allsets[i].collections[j].topo_link);
				}

			}
		}
	}

	/* printf("in Java_saf_1wrap_native_1get_1topo_1relation, didn't get a set match!\n"); */

	return (jint)(0);


}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1topos (
	JNIEnv *env,
	jobject obj,
	jint a_set,
	jint coll_id)
{
	int i, j;
	int num_rels;
	SAF_Rel *rels;
	SAF_Set *SAF_topos = NULL;

	SAF_Set glueSet, partSet;
	SAF_Cat glueCat, partCat;
	SAF_RelRep     trtype;

	Set_Info *set_info;
	Set_Info *topos;
	Collection_Info cat_info;

	set_info = (Set_Info *)a_set;

	find_topo_relations( set_info );

	num_rels = set_info->num_topo_relations;

	/* printf("in 1get_1topos, set_name is %s, num_rels is %d\n",set_info->set_name,num_rels); */

	if( num_rels > 0 ) {

		rels = set_info->topo_relation;

		for( i = 0; i < num_rels; i++ ) {

			saf_describe_topo_relation(SAF_ALL,rels+i,&partSet, &partCat, &glueSet, &glueCat, NULL, &trtype, NULL);
			if( SAF_EQUIV(&partCat, &(set_info->collections[coll_id].the_cat)) ) {
				set_info->collections[coll_id].topo_link = (void *)(rels + i);
				return (jint)( set_info->collections[coll_id].topo_link );
			}
		}
	}
	else
		return 0;

	return 0;

}




JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1subsets(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint coll_id)
{
	int i, j, as;
	int number_of_subsets=0;
	int num_rels;
	SAF_Rel *SAF_rels;
	SAF_Set *SAF_subsets=NULL;
	char *set_name;
	char *cat_name;
	SAF_SilRole srole;
	SAF_ExtendMode e_mode;
	int tdim;
	int found_equiv_set = 0;
	
	Set_Info *set_info;
	Set_Info *subsets;
	Collection_Info cat_info;

	set_info = (Set_Info *)set_id;
	cat_info = set_info->collections[coll_id];


	if( set_info->collections[coll_id].child_setlink != NULL ) 
		return (jint)(set_info->collections[coll_id].child_setlink);

	saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&(set_info->the_set), &(cat_info.the_cat), &number_of_subsets, &SAF_subsets);

	if( number_of_subsets == 0 ) {
		subsets = NULL;
		set_info->collections[coll_id].child_setlink=NULL;
		return 0;
	}
	else {
		subsets = (Set_Info *)(calloc((size_t) number_of_subsets, sizeof(Set_Info)));
	}

	if( subsets == NULL ) {
		return 0;
	}



	for( i = 0; i < number_of_subsets; i++ ) {
		set_name = NULL;
                saf_describe_set(SAF_ALL, SAF_subsets+i, &set_name, &tdim, &srole, &e_mode, NULL, NULL, NULL);

		subsets[i].the_set = SAF_subsets[i];
		subsets[i].set_name = saf_set_name(SAF_subsets + i);
		subsets[i].max_topo_dim = tdim;
		subsets[i].silrole = silrole_string( srole );
		subsets[i].extmode = extmode_string( e_mode );
		subsets[i].num_sets = number_of_subsets;
		subsets[i].topo_relation = NULL;
		subsets[i].fields = NULL;
		subsets[i].num_fields=0;
		subsets[i].collections = NULL;
		
	       
		found_equiv_set = 0;
		for( as = 0; as < num_all_sets; as++ ) {

			if( SAF_EQUIV(&(allsets[as].the_set), &(subsets[i].the_set)) ) {
				
				subsets[i].fields = allsets[as].fields;
				subsets[i].num_fields = allsets[as].num_fields;

				/*
				subsets[i].collections = allsets[as].collections;
				subsets[i].num_colls = allsets[as].num_colls;
				*/
				
				found_equiv_set = 1;
			}

		}
		if( found_equiv_set != 1 )
		  printf("get_subsets: Could not find equiv set for: %s\n", subsets[i].set_name);
	 

		find_collections(subsets, i);
		/*
		find_fields_on_set(subsets,i);
		*/
		

		if( subsets[i].collections != NULL ) {
			for( j = 0 ; j < subsets[i].num_colls; j++ ) {

				SAF_rels = NULL ;

				saf_find_subset_relations(SAF_ALL, db, &(set_info->the_set), \
					&(subsets[i].the_set), &(subsets[i].collections[j].the_cat),\
					&(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,\
					&num_rels,NULL);

				if( num_rels > 0 ) {
				        cat_name = NULL;
				        saf_describe_category(SAF_ALL, &(subsets[i].collections[j].the_cat), &cat_name, NULL,NULL);
					saf_find_subset_relations(SAF_ALL, db, &(set_info->the_set), \
						&(subsets[i].the_set), &(subsets[i].collections[j].the_cat),\
						&(subsets[i].collections[j].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,\
						&num_rels,&SAF_rels);

					/* the subset gets the list of subset relations between it */
					/* and the superset. */
					subsets[i].collections[j].num_subset_relations = num_rels;
					subsets[i].collections[j].subset_relation = SAF_rels;

				}	
				else {
					subsets[i].collections[j].num_subset_relations = 0;
					subsets[i].collections[j].subset_relation = NULL;
				}
				
				subsets[i].collections[j].parent_setlink = (void *)set_info;
				subsets[i].collections[j].parent_colllink = coll_id;
				(set_info->collections[coll_id]).child_setlink = (void *)subsets;

			}
		}
	}

	return (jint)subsets;
}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1subset_1relation(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint coll_id)
{

	Set_Info *set_info;
	SAF_Rel *the_rel;

	set_info = (Set_Info *)set_id;

	
	the_rel = (set_info->collections[coll_id].subset_relation);

	return (jint)(set_info->collections[coll_id].subset_relation);

}



JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1topo_1rel_1size (
	JNIEnv *env,
	jobject obj,
	jint rel_id,
	jint a_or_b)
{


	size_t abuf_sz, bbuf_sz;
	hid_t abuf_type, bbuf_type;
	
	int *rbuf = NULL;
	int result;
	SAF_Rel *rel;

	rel = (SAF_Rel *)rel_id;

	abuf_sz = bbuf_sz = 0;
	/* printf("get topo size: rel_id is %d\n",rel_id); */
	saf_get_count_and_type_for_topo_relation(SAF_ALL,rel,NULL,NULL,&abuf_sz,&abuf_type,&bbuf_sz,&bbuf_type);
	/* printf("data size is %d,%d\n",abuf_sz, bbuf_sz); */
	/* saf_read_topo_relation(SAF_ALL,rel, NULL, (void **)&rbuf, NULL); */

	/* printf("rbuf is %d\n",(int)rbuf); */

	/* result = (sizeof(rbuf))/(sizeof(int)); */

	if( a_or_b == 0 )
		return (jint)abuf_sz;
	else
		return (jint)bbuf_sz;



}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1topo_1rel_1type (
	JNIEnv *env,
	jobject obj,
	jint rel_id)
{

  hid_t abuf_type, bbuf_type;
  SAF_Rel *rel;

  rel = (SAF_Rel *)rel_id;

  saf_get_count_and_type_for_topo_relation(SAF_ALL, rel,NULL, NULL, NULL, &abuf_type, NULL, &bbuf_type);

  if(H5Tequal(abuf_type, SAF_HANDLE) ) 
    return (*env)->NewStringUTF(env,"SAF_HANDLE");
  else if (H5Tequal(bbuf_type, SAF_INT) )
    return (*env)->NewStringUTF(env,"SAF_INT");
  else if (H5Tequal(bbuf_type, SAF_FLOAT) )
    return (*env)->NewStringUTF(env,"SAF_FLOAT");
  else if (H5Tequal(bbuf_type, SAF_DOUBLE) )
    return (*env)->NewStringUTF(env,"SAF_DOUBLE");
  else
    return (*env)->NewStringUTF(env,"UNKNOWN");
     
}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1topo_1rel_1data (
	JNIEnv *env,
	jobject obj,
	jint rel_id,
	jint num,
	jint a_or_b)
{


	int i;
	static size_t abuf_sz = 0;
	static size_t bbuf_sz = 0;
	static SAF_Rel *rel = NULL;
	hid_t abuf_type, bbuf_type;
	static void *abuf = NULL;
	static void *bbuf = NULL;

	int result;

	if( rel != (SAF_Rel *)rel_id ) {
		if( abuf != NULL )
			free(abuf);
		if( bbuf != NULL )
			free(bbuf);

		rel = (SAF_Rel *)rel_id;
		
		abuf = bbuf = NULL;
		abuf_sz = bbuf_sz = 0;
		saf_get_count_and_type_for_topo_relation(SAF_ALL,rel,NULL,NULL,&abuf_sz,&abuf_type,&bbuf_sz,&bbuf_type);
		saf_read_topo_relation(SAF_ALL, rel, NULL, &abuf, &bbuf); 
	}

	if( a_or_b == 0 ) {
		if( num <= (int) abuf_sz )
			result = (int)((int *)abuf)[num];
		else
			result = -1;

	}
	else {
		if( num <= (int) bbuf_sz )
			result = (int)((int *)bbuf)[num];
		else
			result = -1;
	}
	/*
	for( i = 0 ; i < bbuf_sz; i++ ) {
			if( i == num) {
				result = (int)((int *)bbuf)[i];
				break;
			}
	}
	*/

	return (jint)result;

}


JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1subset_1rel_1type(
        JNIEnv *env,
	jobject obj,
	jint rel_id)
{

  SAF_RelRep srtype;
  int srtype_id;
  SAF_Rel *rel;

  rel = (SAF_Rel *)rel_id;
  saf_describe_subset_relation(SAF_ALL, rel, NULL, NULL, NULL, NULL, NULL, NULL, &srtype, NULL);
  saf_describe_relrep(SAF_ALL, &srtype, NULL, NULL, &srtype_id);
  if( srtype_id == SAF_HSLAB_ID ) {
    return (*env)->NewStringUTF(env,"HSLAB");
  }
  else {
    return (*env)->NewStringUTF(env,"TOTALITY");
  }

}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1subset_1rel_1size(
	JNIEnv *env, 
	jobject obj,
	jint rel_id)
{


	size_t abuf_sz, bbuf_sz;
	hid_t abuf_type, bbuf_type;

	SAF_RelRep srtype;
	int srtype_id;
	
	int *rbuf = NULL;
	int result;
	SAF_Rel *rel;

	rel = (SAF_Rel *)rel_id;

	if( rel == NULL )
	  return 0;

	abuf_sz = bbuf_sz = 0;
	/* printf("get subset size: rel_id is %d\n",rel_id);  */


	saf_describe_subset_relation(SAF_ALL, rel, NULL, NULL, NULL, NULL, NULL, NULL, &srtype, NULL);	

	saf_describe_relrep(SAF_ALL, &srtype,NULL,NULL,&srtype_id);

	saf_get_count_and_type_for_subset_relation(SAF_ALL,rel,NULL,&abuf_sz,&abuf_type,&bbuf_sz,&bbuf_type);

	if( abuf_sz <= 0 )
	  return 0;

	if( srtype_id == SAF_HSLAB_ID )  {
		saf_read_subset_relation(SAF_ALL, rel, NULL, (void **)&rbuf, NULL);
		/* printf("HSLAB: %d, %d, %d\n",(int)rbuf[0], (int)rbuf[1], (int)rbuf[2]); */
		return( (jint)rbuf[1] );
	}
		
	/* printf("data size is %d\n",abuf_sz);  */
	/* saf_read_subset_relation(SAF_ALL,*rel, (void **)&rbuf, NULL); */

	/* printf("rbuf is %d\n",(int)rbuf); */

	/* result = (sizeof(rbuf))/(sizeof(int)); */

	return (jint)abuf_sz;

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1subset_1rel_1data(
	JNIEnv *env,
	jobject obj,
	jint rel_id,
	jint num)
{

	int i, j = 0;
	static size_t abuf_sz = 0;
	static size_t bbuf_sz = 0;
	hid_t abuf_type, bbuf_type;
	static void *abuf = NULL;
	static void *bbuf = NULL;
	int *tmpbuf = NULL;
	int *tmpbuf2 = NULL;
	static SAF_Rel *rel = NULL;
	static void *rbuf = NULL;
	int result;
	SAF_RelRep srtype;
	int srtype_id;

	if( rel != (SAF_Rel *)rel_id ) {
		if( rbuf != NULL )
			free(rbuf);

		rbuf = NULL;
		
		rel = (SAF_Rel *)rel_id;

		abuf_sz = bbuf_sz = 0;

		saf_describe_subset_relation(SAF_ALL, rel, NULL, NULL, NULL, NULL, NULL, NULL, &srtype,  NULL);

		saf_describe_relrep(SAF_ALL, &srtype,NULL,NULL,&srtype_id);
	
		saf_get_count_and_type_for_subset_relation(SAF_ALL,rel,NULL,&abuf_sz,&abuf_type,&bbuf_sz,&bbuf_type);
		result = saf_read_subset_relation(SAF_ALL, rel, NULL, (void **)&rbuf, NULL);

	        if( result != SAF_SUCCESS ) {
		  rel = NULL;
		  return -1;
		}


		if( srtype_id == SAF_HSLAB_ID ) {

			tmpbuf = (int *)rbuf;

			
			tmpbuf2 = (int * )calloc((size_t) tmpbuf[1], sizeof(int));
			abuf_sz = (size_t)tmpbuf[1];
			j = 0;
			for( i = tmpbuf[0]; j < tmpbuf[1] ; i += tmpbuf[2] ) {
				tmpbuf2[j] = i;
				/* printf("%d:%d\n",j,i); */
				j++;
			}
			free(rbuf);
			rbuf = (void *)tmpbuf2;
			
		}
	}



	if( num <= (int) abuf_sz )
		result = (int)((int *)rbuf)[num];
	else
		result = -1;

	/*
	for( i = 0 ; i < abuf_sz; i++ ) {
			if( i == num) {
				result = (int)rbuf[i];
				break;
			}
	}
	*/

	return (jint)result;
}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1state_1data(
JNIEnv *env, jobject obj, jint field_id, jint state_num, jint num)
{

	int i;
	int index[1];
	int num_comps;
	static int last_state_num_read = -1;
	size_t count;
	static SAF_Field *fbuf = NULL;
	int value;
	void *coords = NULL;
	SAF_Field *state_buf;
	static SAF_Field *saf_field = NULL;
	SAF_Field the_field;
	static Field_Info *field_info = NULL;
	static int last_num = -1;

	static Field_Info *field_on_state_info = NULL;
	static SAF_Field  *the_field_on_state;

	field_info = (Field_Info *)field_id;
	the_field = *(field_info->the_field);

	num_comps = field_info->num_comps;


	if( (saf_field != NULL) && (fbuf != NULL) && (state_num == last_state_num_read) ) { 

		if( SAF_EQUIV(saf_field, &the_field) ) {


			field_on_state_info = (Field_Info *)calloc((size_t) 1,sizeof(Field_Info));	

			the_field_on_state = fbuf + num;

			/* erik use to be: if( SAF_EQUIV( *(fbuf + num), the_field_on_state)  )  */
			if( SAF_EQUIV( fbuf + num, the_field_on_state)  )
			  return (int)(field_on_state_info);
			else {
			  the_field_on_state = fbuf + num;
			}

			field_on_state_info->the_field = the_field_on_state;

			get_field_info( field_on_state_info );

			value = (int)(field_on_state_info);

			return value;
		}
		else {
		  
			free(fbuf);
			fbuf = NULL;
			saf_field = NULL;
		}
	}

	last_state_num_read = state_num;

	saf_field = field_info->the_field;

	index[0] = state_num; 
	fbuf = NULL;
	saf_read_state( SAF_ALL, saf_field, index[0], NULL, NULL,&coords, &fbuf); 



	field_on_state_info = (Field_Info *)calloc((size_t) 1,sizeof(Field_Info));	
	the_field_on_state = (SAF_Field *)calloc((size_t) 1,sizeof(SAF_Field));

	memcpy(the_field_on_state, fbuf + num, sizeof(SAF_Field)); 
	field_on_state_info->the_field = the_field_on_state;

	get_field_info( field_on_state_info );

	value = (int)(field_on_state_info);

	return (int)(value);
}
JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1data_1int(
JNIEnv *env, jobject obj, jint field_id, jint num)
{

    int i;
    size_t count;
    static int *ibuf = NULL;
    int value, int_value;
    static int minibuf[10];
    void  *pbuf;
    static SAF_Field *saf_field = NULL;
    static SAF_Field *the_fields = NULL;
    static Fields field_infos = NULL;
    SAF_Field the_field;
    Field_Info *field_info;
    static hid_t Ptype;

    field_info = (Field_Info *)field_id;
    the_field = *(field_info->the_field);

    if( (saf_field != NULL) && (ibuf != NULL) ) {

        if( SAF_EQUIV(saf_field, &the_field) ) {

          if( H5Tequal(Ptype, SAF_INT) )
            value = (int)(ibuf[num]);
          else {
            printf("Error: saf_wrap: get_field_data_int found non SAF_INT\n");
            return -1;
          }

          return (jint)value;

        }
        else {
            if( ibuf != minibuf )
                free(ibuf);
            ibuf = NULL;
            saf_field = NULL;
       }
    }


    saf_field = field_info->the_field;
    saf_get_count_and_type_for_field( SAF_ALL, saf_field,  NULL, &count, &Ptype);

      if( count > 1 )
        ibuf = (void *)calloc((size_t) count, sizeof(int));
      else
        ibuf = (void *)minibuf;
      pbuf = (void *)(ibuf);

    /* pbuf = (void *)(ibuf); */
    saf_read_field(SAF_ALL, saf_field, NULL, SAF_WHOLE_FIELD, &pbuf);

    if( H5Tequal(Ptype, SAF_INT) )
        value = (int)(ibuf[num]);
    else {
        printf("Error: saf_wrap: get_field_data_int found non SAF_INT\n");
        return -1;
    }
    return (jint)(value);
}

JNIEXPORT jfloat JNICALL Java_saf_1wrap_native_1get_1field_1data(
JNIEnv *env, jobject obj, jint field_id, jint num)
{

	int i;
	size_t count;
	static double *fbuf = NULL;
	float value;
	int int_value;
	static double minibuf[10];
	void  *pbuf;
	static SAF_Field *saf_field = NULL;
	static SAF_Field *the_fields = NULL;
	static Fields field_infos = NULL;
	SAF_Field the_field;
	Field_Info *field_info;
	static hid_t Ptype;

	field_info = (Field_Info *)field_id;
	the_field = *(field_info->the_field);

	if( (saf_field != NULL) && (fbuf != NULL) ) {

		if( SAF_EQUIV(saf_field, &the_field) ) {

		  if(H5Tequal(Ptype, SAF_HANDLE) ) {
		    int_value = (int)(field_infos + num);
		    printf("saf_wrap: get_field_data0: got %s\n",(field_infos+num)->field_name);
		    value = (float)int_value;
		  }
		  else if(H5Tequal(Ptype, SAF_DOUBLE) )
		    value = (float)(fbuf[num]);
		  else
		    value = (float) ( ((float *)fbuf)[num] ); 

		  return (jfloat)value;

		}
		else {
			if( fbuf != minibuf )
				free(fbuf);
			fbuf = NULL;
			saf_field = NULL;
		}
	}


	saf_field = field_info->the_field;
	saf_get_count_and_type_for_field( SAF_ALL, saf_field, NULL, &count, &Ptype);

	if(H5Tequal(Ptype, SAF_HANDLE) ) {
	  printf("GOT a SAF_HANDLE!\n");
	  the_fields = (SAF_Field *)calloc((size_t) count, sizeof(SAF_Field));
	  field_infos = (Field_Info *)calloc((size_t) count, sizeof(Field_Info));
	  pbuf = (void *)the_fields;
	  fbuf = (double *)the_fields;
	}
	else {
	  if( count > 1 ) 
	    fbuf = (double *)calloc((size_t) count, sizeof(double));
	  else
	    fbuf = minibuf;
	  pbuf = (void *)(fbuf);
	}

	/* pbuf = (void *)(fbuf); */
	saf_read_field(SAF_ALL, saf_field, NULL, SAF_WHOLE_FIELD, &pbuf);

	if(H5Tequal(Ptype, SAF_HANDLE) ) {
	  printf("get_field_data: count is %d\n",count);
	  for( i = 0 ; i < (int) count; i++ ) {
	    (field_infos + i)->the_field = (SAF_Field *)calloc((size_t) 1,sizeof(SAF_Field));
	    memcpy( (field_infos + i)->the_field, the_fields + i, sizeof(SAF_Field) );
	    get_field_info( field_infos + i );
	/*
	    printf("field %d,%d: %s, %s\n",i,(int)(field_infos+i),((Set_Info *)((field_infos + i)->template_info->base_space))->set_name,(field_infos+i)->field_name);
	    
	*/
	  }

	  printf("saf_wrap: get_field_data1: got %s\n",(field_infos+num)->field_name);
	  int_value = (int)(field_infos + num);
	  printf("int_value is %d\n", int_value);
	  value = (float)int_value;
	  printf("value casted to int from float : %d\n",(int)value);
	}
	else if( H5Tequal(Ptype, SAF_DOUBLE) )
		value = (float)(fbuf[num]);
	else
		value =  (float)( ((float *)fbuf)[num] );

	
	return (jfloat)(value);
}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1remap_1field(
JNIEnv *env, jobject obj, jint field_id, jint dest_field_id)
{


  Field_Info *field_info;
  Field_Info *new_field_info, *dest_field_info, *tmp_field_info;

  SAF_Field the_field, *new_field;

  field_info = (Field_Info *)field_id;
  dest_field_info = (Field_Info *)dest_field_id;
  the_field = *(field_info->the_field);

  tmp_field_info = (Field_Info *)calloc((size_t) 1, sizeof(Field_Info) );
  tmp_field_info->the_field = dest_field_info->the_field;
  get_field_info( tmp_field_info );
  dest_field_info = tmp_field_info;

  new_field = (SAF_Field *)calloc((size_t) 1, sizeof(SAF_Field) );
  *new_field = the_field;
  SAF_FieldTarget ftarg;
  size_t buf_size;
  hid_t buf_type;


  /*
  printf("num_comps: %d, base_space_name: %s, data_size: %d, tmpl_num_comps: %d\n",
     dest_field_info->num_comps, dest_field_info->template_info->base_space_name,
     dest_field_info->data_size, dest_field_info->template_info->num_comps);
  */


  saf_target_field(&ftarg, NULL, SAF_SELF(db),
                          SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, H5I_INVALID_HID,
                          SAF_INTERLEAVE_COMPONENT, NULL);
  saf_get_count_and_type_for_field(SAF_ALL, new_field, &ftarg, &buf_size, &buf_type);

  new_field_info = (Field_Info *)calloc((size_t) 1, sizeof(Field_Info) );
  new_field_info->the_field = new_field;

  get_field_info(new_field_info);
  dest_field_info->base_space_name = new_field_info->base_space_name;
  dest_field_info->base_space = new_field_info->base_space;
  dest_field_info->field_name = new_field_info->field_name;
  dest_field_info->data_size = new_field_info->data_size;
  dest_field_info->data_type = new_field_info->data_type;
  dest_field_info->the_field = new_field_info->the_field;

  /*
  printf("num_comps: %d, base_space_name: %s, data_size: %d, tmpl_num_comps: %d\n",
     dest_field_info->num_comps, dest_field_info->template_info->base_space_name,
     dest_field_info->data_size, dest_field_info->template_info->num_comps);
  */

  return (jint)dest_field_info;

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1indfield_1data(
JNIEnv *env, jobject obj, jint field_id, jint num, jint clear)
{

	int i;
	size_t count;
	static double *fbuf = NULL;
	int value;
	int int_value;
	static double minibuf[10];
	void  *pbuf;
	static SAF_Field *saf_field = NULL;
	static SAF_Field *the_fields = NULL;
	static Fields field_infos = NULL;
	SAF_Field the_field;
	Field_Info *field_info;
	static hid_t Ptype;

	field_info = (Field_Info *)field_id;
	the_field = *(field_info->the_field);

	if( (saf_field != NULL) && (fbuf != NULL) ) {

		if( SAF_EQUIV(saf_field, &the_field) && (clear == 0) ) {

		  if( H5Tequal(Ptype, SAF_HANDLE) ) {
		    int_value = (int)(field_infos + num);

		    value = int_value;
		  }
		  else if(H5Tequal(Ptype, SAF_DOUBLE) )
		    value = (int)(fbuf[num]);
		  else
		    value = (int) ( ((float *)fbuf)[num] ); 

		  return (int)value;

		}
		else {
			if( fbuf != minibuf )
				free(fbuf);
			fbuf = NULL;
			saf_field = NULL;
		}
	}


	saf_field = field_info->the_field;
	saf_get_count_and_type_for_field( SAF_ALL, saf_field, NULL,  &count, &Ptype);

	if(H5Tequal(Ptype, SAF_HANDLE) ) {

	  the_fields = (SAF_Field *)calloc((size_t) count, sizeof(SAF_Field));
	  field_infos = (Field_Info *)calloc((size_t) count, sizeof(Field_Info));
	  pbuf = (void *)the_fields;
	  fbuf = (double *)the_fields;
	}
	else {
	  if( count > 1 ) 
	    fbuf = (double *)calloc((size_t) count, sizeof(double));
	  else
	    fbuf = minibuf;
	  pbuf = (void *)(fbuf);
	}

	/* pbuf = (void *)(fbuf); */
	saf_read_field(SAF_ALL, saf_field, NULL, SAF_WHOLE_FIELD, &pbuf);

	if(H5Tequal(Ptype, SAF_HANDLE) ) {

	  for( i = 0 ; i < (int) count; i++ ) {
	    (field_infos + i)->the_field = (SAF_Field *)calloc((size_t) 1,sizeof(SAF_Field));
	    memcpy( (field_infos + i)->the_field, the_fields + i, sizeof(SAF_Field) );
	    get_field_info( field_infos + i );
	    
	  }

	  int_value = (int)(field_infos + num);

	  value = (int)int_value;
	}
	else if(H5Tequal(Ptype, SAF_DOUBLE) )
		value = (int)(fbuf[num]);
	else
		value =  (int)( ((float *)fbuf)[num] );

	
	return (jint)(value);
}

/* field data
          size_t count;
          float old_buf[] = {4., 3., 2., 1., 0.};
          float new_buf[] = {0., 0., 0., 0., 0.};
          void *pbuf = &new_buf[0];

          saf_read_field(SAF_ALL, distfac, SAF_WHOLE_FIELD, &pbuf);
          saf_get_count_and_type_for_field(SAF_ALL, distfac, &count, NULL);
          for (i = 0; i < count; i++)
             if (new_buf[i] != old_buf[i])
             {
                failed = true;
                nerrors++;
                break;
             }
       }
*/


JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1subset_1relation_1info(
	JNIEnv *env,
	jobject obj,
	jint rel_id)
{

	SAF_Rel *rel;
	SAF_Cat sub_cat, sup_cat;
	char *name;

	rel = (SAF_Rel *)rel_id;

	name = NULL;
	/* printf("get subset info: rel_id is %d\n",rel_id); */
	saf_describe_subset_relation(SAF_ALL, rel,  NULL, NULL, &sup_cat, &sub_cat, NULL, NULL, NULL, NULL);
	saf_describe_category(SAF_ALL, &sub_cat, &name, NULL, NULL);

	return (*env)->NewStringUTF(env,name);


}



JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1num_1colls(
	JNIEnv *env,
	 jobject obj,
	jint set_id)
{

	Set_Info *set;

	set = (Set_Info *)set_id;

	return(set->num_colls);

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1num_1tops(
	JNIEnv *env,
	jobject obj)
{

	return num_top_sets;	

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1num_1suites(
	JNIEnv *env,
	jobject obj)
{

	return num_suite_sets;	

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1set_1name(
	JNIEnv *env,
	jobject obj,
	jint set_id)
{

	Set_Info *the_set;
	char *name;

	if( set_id == 0 )
		return((*env)->NewStringUTF(env,name));

	the_set = (Set_Info *)set_id;

	name = the_set->set_name;

	return (*env)->NewStringUTF(env,name);
}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1set_1silrole(
JNIEnv *env, jobject obj, jint set_id)
{

	Set_Info *set_info;

	set_info = (Set_Info *)set_id;

	return (*env)->NewStringUTF(env,set_info->silrole);
}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1set_1extmode(
JNIEnv *env, jobject obj, jint set_id)
{

	Set_Info *set_info;

	set_info = (Set_Info *)set_id;

	return (*env)->NewStringUTF(env,set_info->extmode);
}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1set_1topmode(
JNIEnv *env, jobject obj, jint set_id)
{

	Set_Info *set_info;

	set_info = (Set_Info *)set_id;

	return (*env)->NewStringUTF(env,set_info->topmode);
}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1set_1max_1topo_1dim(
JNIEnv *env, jobject obj, jint set_id)
{

	Set_Info *set_info;

	set_info = (Set_Info *)set_id;

	return (jint)(set_info->max_topo_dim);
}


JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1coll_1cat_1name(
	JNIEnv *env,
	jobject obj,
	jint tmptops,
	jint num)
{

	Set_Info *tops;
	char *name;

	tops = (Set_Info *)tmptops;

/* erik changed because it was aborting 
	name = tops->collections[num].cat_name; 
*/
        if (SS_CAT(&(tops->collections[num])))
           name = saf_cat_name(&(tops->collections[num]));
        else
           name = NULL;

	return (*env)->NewStringUTF(env,name);

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1coll_1count(
JNIEnv *env, jobject obj, jint set_id, jint coll_num)
{

	Set_Info *set_info;
	set_info = (Set_Info *)set_id;

	return (jint)(set_info->collections[coll_num].count);
}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1coll_1celltype(
JNIEnv *env, jobject obj, jint set_id, jint coll_id)
{

	Set_Info *set_info;
	set_info = (Set_Info *)set_id;

	return (*env)->NewStringUTF(env,set_info->collections[coll_id].celltype);

}

JNIEXPORT jboolean JNICALL Java_saf_1wrap_native_1get_1coll_1is_1decomp(
JNIEnv *env, jobject obj, jint set_id, jint coll_id)
{

	Set_Info *set_info;
	set_info = (Set_Info *)set_id;

	return (jboolean)(set_info->collections[coll_id].is_decomp);

}


JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1category_1name(
JNIEnv *env, jobject obj, jint cat_id)
{

	SAF_Cat *category;
	char buffer[100];
	char *cat_name=NULL; 

	category = (SAF_Cat *)cat_id;


	if( ! SAF_EQUIV(category,SAF_SELF(db)) ) {
		saf_describe_category(SAF_ALL, category, &cat_name, NULL, NULL ); 
		strcpy(buffer,cat_name);
		free(cat_name);
	}
	else
	{
		strcpy(buffer,"SAF_SELF");
	}

	return (*env)->NewStringUTF(env,buffer);


}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1lookup_1coll_1num(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jstring cat_name)
{
	Set_Info *set_info;
	int i;
	const char *tmp_cat_name;


	tmp_cat_name = (*env)->GetStringUTFChars(env,cat_name,0);
	set_info = (Set_Info *)set_id;

	for( i = 0; i < set_info->num_colls; i++ ) {

		if ( !strcmp(tmp_cat_name,set_info->collections[i].cat_name) )
			return i;
	}

	return -1;

}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1all_1sets_1id(
	JNIEnv *env,
	jobject obj)
{

	return (int)allsets;


}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1suite_1sets_1id(
	JNIEnv *env,
	jobject obj)
{

	return (int)suitesets;


}



JNIEXPORT jint JNICALL Java_saf_1wrap_native_1open(
  JNIEnv *env, 
  jobject obj, 
  jstring file_path,
  jstring file_name,
  jstring mode)
{
	int status = 0;

   char *name;
   char *set_name, *set_name2, *topo_cat_name, *topo_cat_name2;
   hbool_t do_describes = false;
   hbool_t do_reads = false;
   hbool_t multifile;
   int i, j, k, rank=0, as;
   int tdim, topo_dim, count, num_cats, num_colls, num_rels, num_subs;
   SAF_Role role;
   SAF_TopMode t_mode;
   SAF_ExtendMode e_mode;
   SAF_SilRole srole;
   hbool_t failed;
   SAF_ExtendMode extendible;
   SAF_CellType cell_type;
   hbool_t is_decomp;
   SAF_Cat nodes, elems;
   SAF_Set top, cell_1, cell_2, cell_3, ns1;
   SAF_Rel *rels;
   SAF_Field *saf_fields;

   SAF_Set partSet;
   SAF_Cat partCat;
   SAF_Set glueSet;
   SAF_Cat glueCat;
   SAF_RelRep     trtype;

   const char *str = (*env)->GetStringUTFChars(env,file_name,0); 
   dbname = strdup(str);

   /* dbname = ("test_saf.saf");  */

   /* process command line args */
   if (rank == 0)
   {
      /* since we want to see whats happening make sure stdout and stderr are unbuffered */
      setbuf(stdout, NULL);
      setbuf(stderr, NULL);
   }

  /* for convenience, set working directory to the test file directory */
  /* chdir("/home/pkespen/saf/src/safapi/test/"); */

  const char *str2 = (*env)->GetStringUTFChars(env,file_path,0); 
  mypwd = strdup(str2);
  chdir(mypwd);  
  /* 
  printf(" ************* working directory: %s  ********  \n",mypwd);
  printf(" ************* file to open: %s  ********  \n",dbname);
  */

  /* called by saf_init java class, see SAFGUI */
  /* saf_init(NULL);  */

  SAF_TRY_BEGIN
  {


    /* note: because we are in a try block here, all failures will send us to the one and only
       catch block at the end of this test */

    p = saf_createProps_database();
    saf_setProps_ReadOnly(p);
    db = saf_open_database(dbname,p);
    if (db) {
      status = 0;
    }
    else {
      status = -1;
      return (jint)status;
    }


    p_cats = NULL;
    all_sets = NULL;
    num_all_sets = -1;

		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_ANY_SILROLE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_all_sets, NULL);
		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_ANY_SILROLE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_all_sets, &all_sets);

	allsets = (Set_Info *)calloc((size_t) num_all_sets, sizeof(Set_Info));

	/* do an error check for AllSets here */
		

	for(i = 0; i < num_all_sets; i++ ) {
		set_name=NULL;
                saf_describe_set(SAF_ALL, all_sets+i, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);
		/* printf("%d: %s\n",i,set_name); */

		allsets[i].the_set = all_sets[i];
		allsets[i].num_sets = num_all_sets;
		allsets[i].set_name = set_name;
		allsets[i].max_topo_dim = tdim;
		allsets[i].silrole = silrole_string( srole );
		allsets[i].extmode = extmode_string( e_mode );
		allsets[i].topo_relation = NULL;
		allsets[i].collections = NULL;
		allsets[i].fields = NULL;
		allsets[i].num_fields = 0;
	}

	for( i = 0 ; i < num_all_sets; i++ ) {
		find_collections( allsets, i );
		find_fields_on_set( allsets, i);
	}


    p_cats = NULL;
    suite_sets = NULL;
    suite_subsets = NULL;
    number_suite_subsets = -1;
    num_suite_sets = -1;

		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SUITE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_suite_sets, NULL);
		saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_SUITE, SAF_ANY_TOPODIM, \
			SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_suite_sets, &suite_sets);

	if( num_suite_sets > 0 ) {
	  suitesets = (Set_Info *)calloc((size_t) num_suite_sets, sizeof(Set_Info));

		for(i = 0; i < num_suite_sets; i++ ) {

			set_name=NULL;
            
			saf_describe_set(SAF_ALL, suite_sets+i, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);
			suitesets[i].the_set = suite_sets[i];
			suitesets[i].num_sets = num_suite_sets;
			suitesets[i].set_name = set_name;
			suitesets[i].silrole =  silrole_string(srole);
			suitesets[i].topo_relation = NULL;
			suitesets[i].collections = NULL;
			suitesets[i].fields = NULL;

			suitesets[i].num_fields = 0;

			find_collections( suitesets, i );
			find_fields_on_set( suitesets, i);
	
			for( j = 0 ; j < (suitesets+i)->num_colls; j++ ) {
 
			  if( !strcmp(suitesets[i].collections[j].cat_name,"space_slice_cat") ) {

			    saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&(suitesets[i].the_set),&((suitesets[i].collections[j]).the_cat),\
                                          &number_suite_subsets,NULL);
			    saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&(suitesets[i].the_set),&((suitesets[i].collections[j]).the_cat),\
                                          &number_suite_subsets,&suite_subsets);

			    suitesubsets = (Set_Info *)calloc((size_t) number_suite_subsets, sizeof(Set_Info));
			    for( k = 0 ; k < number_suite_subsets; k++ ) {
			      set_name = NULL;
			      saf_describe_set(SAF_ALL, suite_subsets+k, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);
			      suitesubsets[k].the_set = suite_subsets[k];
			      suitesubsets[k].num_sets = number_suite_subsets;
			      suitesubsets[k].set_name = set_name;
			      suitesubsets[k].silrole = silrole_string(srole);
			      suitesubsets[k].srole = srole;
			      suitesubsets[k].topo_relation = NULL;
			      suitesubsets[k].collections = NULL;
			      suitesubsets[k].fields = NULL;
			      suitesubsets[k].num_fields = 0;
			      find_collections( suitesubsets, k);
			      find_fields_on_set( suitesubsets, k);
                                
                            }
			  }
                        }

		}

	}

    top_sets = NULL;
    p_cats  = NULL;
    num_top_sets = -1;

	        saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME,SAF_SPACE,SAF_ANY_TOPODIM,SAF_EXTENDIBLE_TORF, \
                SAF_TOP_TRUE, &num_top_sets, NULL);
        saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME,SAF_SPACE,SAF_ANY_TOPODIM,SAF_EXTENDIBLE_TORF, \
                SAF_TOP_TRUE, &num_top_sets, &top_sets);


	if( num_top_sets < 1 ) {

	  if( number_suite_subsets > 0 ) {
	    for( i = 0; i < number_suite_subsets ; i++ ) {
	      if ( suitesubsets[i].srole == SAF_SPACE ){
		num_top_sets = 1;
		top_sets = &(suitesubsets[i].the_set);
		break;		
	      }
	    }
	  }
	  else { /* number_suite_subset <= 0 */
	    num_top_sets = -1;
	  }
	}

	if( num_top_sets <= 0 ) 
		return -1;

	tops = (Set_Info *)calloc((size_t) num_top_sets, sizeof(Set_Info)); 


	for( i = 0; i < num_top_sets; i++ ) {

		set_name=NULL;
                saf_describe_set(SAF_ALL, top_sets+i, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);

		tops[i].the_set = top_sets[i];
		tops[i].num_sets = num_top_sets;
		tops[i].set_name = set_name;
		tops[i].max_topo_dim = tdim;
		tops[i].silrole = silrole_string( srole );
		tops[i].extmode = extmode_string( e_mode );
		tops[i].topo_relation = NULL;
		tops[i].collections = NULL;
		tops[i].fields = NULL;
		tops[i].num_fields = 0;

		for( as = 0 ; as < num_all_sets; as++ ) {
			if (  SAF_EQUIV(&(allsets[as].the_set),top_sets+i) ) {
				tops[i].fields = allsets[as].fields;
				tops[i].collections = allsets[as].collections;
				tops[i].num_fields = allsets[as].num_fields;
				tops[i].num_colls = allsets[as].num_colls;
			}
		}
		/*
		find_collections( tops, i );
		find_fields_on_set( tops, i );
		*/


	}

  }
	return (jint)tops;   
  
}


void find_fields_on_set( Set_Info *the_set, int set_num)
{

	int i, j, num_fields = 0;
	SAF_Field *fields;
	Field_Info *field_info;
	SAF_Set saf_set;
	Set_Info *tmp_set;
	SAF_Cat *storage_decomp;
	SAF_Cat *coeff_assoc;
	SAF_Cat *eval_coll;
	char *name;

	tmp_set = the_set + set_num;
	if( tmp_set->fields != NULL )
		return;
	
	if( tmp_set->fields != NULL )
		return;

	saf_set = tmp_set->the_set;



	fields = NULL;
	saf_find_fields(SAF_ALL, db, &saf_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, \
			SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &num_fields, NULL);

	tmp_set->num_fields = num_fields;

	if( num_fields <= 0 ) {
		tmp_set->fields = NULL;
		return;
	}


	/* tmp_set->fields = (Field_Info *)(calloc((size_t) num_fields, sizeof(Field_Info))); */
        tmp_set->fields = calloc((size_t) num_fields, sizeof(Field_Info));

	saf_find_fields(SAF_ALL, db, &saf_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, \
			SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &num_fields, &fields);


	for( i = 0; i < num_fields; i++ ) {

		name = NULL;
		tmp_set->fields[i].the_field = fields + i;
		tmp_set->fields[i].my_set_id = (int)(tmp_set);
		field_info = ((Field_Info *)(tmp_set->fields)) + i;

		get_field_info( field_info );


		coeff_assoc = field_info->coeff_assoc;
		storage_decomp = field_info->storage_decomp;
		eval_coll = field_info->eval_coll;

		tmp_set->fields[i].my_set_coll_num = -1;
		for( j = 0; j < tmp_set->num_colls; j++ ) {

			if( (coeff_assoc != NULL) && SAF_EQUIV( &(tmp_set->collections[j].the_cat), coeff_assoc) ) {
				tmp_set->fields[i].my_set_coll_num = j;
			}

		}

		/* I do the following in get_field_info now */
		/*
		if( ! SAF_EQUIV(*storage_decomp,saf_self) ) {
			name = NULL;
			saf_describe_category(SAF_ALL, storage_decomp,&name,NULL,NULL);
		}
		else {
			field_info->storage_decomp=NULL;
		}
		*/

/*	
		name = NULL;
		saf_describe_category(SAF_ALL, coeff_assoc,&name,NULL,NULL);

		name = NULL;
		saf_describe_category(SAF_ALL, eval_coll,&name,NULL,NULL);
*/
		
	}


}

void get_field_info( Field_Info *field_info )
{

	int i, j, num_fields = 0;
	SAF_Field *field;
	SAF_FieldTmpl *field_template;
	FieldTmpl_Info *template_info;
	SAF_Set *saf_set;
	SAF_Set *base_space;
	Set_Info *tmp_set;
	SAF_Cat *storage_decomp;
	SAF_Cat *coeff_assoc;
	SAF_Cat *eval_coll;
	char *name, *catname;
	char *sd_name;
	hbool_t is_coord;
	int assoc_ratio;
	SAF_Eval eval_func;
	hid_t data_type;
	size_t data_size;
	int num_comps;
	SAF_Field *components=NULL;
	Field_Info *component_fields;
	SAF_Interleave interleave;
	int *comp_order;

	
         base_space = (SAF_Set *)calloc((size_t) 1, sizeof(SAF_Set));

	field = field_info->the_field;

	
                template_info = (FieldTmpl_Info *)calloc((size_t) 1, sizeof(FieldTmpl_Info));
                field_template = (SAF_FieldTmpl *)calloc((size_t) 1, sizeof(SAF_FieldTmpl));

		name = NULL;
		catname = NULL;
		comp_order = NULL;

                coeff_assoc = (SAF_Cat *)calloc((size_t) 1, sizeof(SAF_Cat));
                storage_decomp = (SAF_Cat *)calloc((size_t) 1, sizeof(SAF_Cat));
                eval_coll = (SAF_Cat *)calloc((size_t) 1, sizeof(SAF_Cat));


		saf_describe_field(SAF_ALL, field, field_template, &name, base_space, NULL, &is_coord, storage_decomp, \
					coeff_assoc, &assoc_ratio, eval_coll, &eval_func, NULL, \
					&num_comps, NULL , &interleave, NULL);



		template_info->fieldtmpl = field_template;


		get_fieldtmpl_info( template_info );


		/* printf("In get_field_info: %s:%s\n",name,template_info->algtype);  */

		/* if( strcmp(template_info->algtype,"SAF_FIELD") )  { */
		if( strcmp(template_info->algtype,"field") )  {
			name = NULL;
			comp_order = NULL;

			saf_describe_field(SAF_ALL, field, field_template, &name, base_space, NULL, &is_coord, NULL, \
					NULL, &assoc_ratio, eval_coll, &eval_func, NULL, \
					&num_comps, &components , &interleave, NULL);

			/*
			saf_describe_field(SAF_ALL, *field, field_template, &name, NULL, &is_coord, NULL, \
					NULL, &assoc_ratio, NULL, &eval_func, NULL, \
					&num_comps, NULL, &interleave, NULL, NULL);
			num_comps = 1;
			*/

		}

		field_info->base_space_name = saf_set_name(base_space); 
		
		for( i = 0; i < num_all_sets ; i++ ) {
			saf_set = &(allsets[i].the_set);
			if( SAF_EQUIV(saf_set, base_space) ) {
				field_info->base_space = (void *)(allsets + i);
				/* free(base_space); causing an error */
			}
		}
		

		field_info->template_info = template_info;
		field_info->coeff_assoc = coeff_assoc;

		/*
		saf_describe_category(SAF_ALL, coeff_assoc, &catname,NULL,NULL);
		printf("get_field_info: Field name: %s, coeff_assoc: %s\n",name, catname); 
		*/
		field_info->assoc_ratio = assoc_ratio;
		field_info->eval_coll = eval_coll;


		field_info->storage_decomp = storage_decomp;


		field_info->field_name = name;
		field_info->interleave = interleave_string( (int)interleave );
		field_info->num_comps = num_comps;
		field_info->eval_func = evalfunc_string(&eval_func);
		field_info->is_coord = (jboolean)is_coord;

		saf_get_count_and_type_for_field(SAF_ALL, field, NULL,  &data_size, &data_type );

		/* printf("Field name: %s, hid_t: %s\n",name,StringDataType( data_type ) ); */

		field_info->data_size = (int)data_size;
		field_info->data_type =(char *) StringDataType( data_type ); 

		if( components == NULL ) {
			field_info->component_fields = NULL;
		}
		else {
                        component_fields = (Field_Info *)calloc((size_t) num_comps, sizeof(Field_Info));
			field_info->component_fields = component_fields;
			for( i = 0; i < num_comps; i++ ) {
				component_fields[i].the_field = components + i;
				component_fields[i].my_set_id = field_info->my_set_id;
				get_field_info( component_fields + i );
				
			}
		}
		if( ! SAF_EQUIV(storage_decomp,SAF_SELF(db)) ) {
			name = NULL;
			saf_describe_category(SAF_ALL, storage_decomp,&name,NULL,NULL);
		}
		else {
			field_info->storage_decomp=NULL;
		}

}


void get_fieldtmpl_info(  FieldTmpl_Info *template_info )
{

	int i, j, num_comps=0;
	SAF_FieldTmpl *field_template;
	SAF_Set *saf_set;
	SAF_Set *base_space;
	SAF_Algebraic algtype;
	SAF_Basis basis;
	FieldTmpl_Info *component_tmpls=NULL;
	SAF_FieldTmpl *components=NULL;
	char *name;


		name = NULL;
                base_space = (SAF_Set *)calloc((size_t) 1, sizeof(SAF_Set));

		field_template = template_info->fieldtmpl;

		saf_describe_field_tmpl(SAF_ALL,field_template,&name, /* base_space, */ &algtype,&basis,NULL,&num_comps,NULL);

		if( num_comps > 0 ) {
			saf_describe_field_tmpl(SAF_ALL,field_template,NULL,NULL,NULL,NULL,&num_comps,&components);
		}
			
		if( components == NULL ) {
			template_info->component_tmpls = NULL;
		}
		else {
                        component_tmpls = (FieldTmpl_Info *)calloc((size_t) num_comps, sizeof(FieldTmpl_Info));
			template_info->component_tmpls = component_tmpls;
			for( i = 0; i < num_comps; i++ ) {
				component_tmpls[i].fieldtmpl = components + i;
				get_fieldtmpl_info( component_tmpls + i );
				
			}
		}

		template_info->fieldtmpl_name = name;
		template_info->num_comps = num_comps;
		/* template_info->base_space_name = saf_set_name(base_space); */
		/*
		for( i = 0; i < num_all_sets ; i++ ) {
			saf_set = &(allsets[i].the_set);
			if( SAF_EQUIV(*saf_set, *base_space) ) {
				template_info->base_space = (void *)(allsets + i);
				free(base_space);
			}
		}
		*/

		/* template_info->algtype =  algtype_string(algtype); */
		template_info->algtype =  algtype_string(&algtype);
		template_info->basis = basis_string(&basis);
}

void get_set_info(  Set_Info *set_info )
{
   char *set_name ;
   int tdim, num_colls;
   SAF_TopMode t_mode;
   SAF_ExtendMode e_mode;
   SAF_SilRole srole;


		set_name=NULL;
                saf_describe_set(SAF_ALL, &(set_info->the_set), &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);

		set_info->num_sets = num_all_sets;
		set_info->set_name = set_name;
		set_info->max_topo_dim = tdim;
		set_info->silrole = silrole_string( srole );
		set_info->extmode = extmode_string( e_mode );
		set_info->topo_relation = NULL;
		set_info->collections = NULL;
		set_info->fields = NULL;
		set_info->num_fields = 0;
		/* find_collections( allsets, i ); */
		/* find_fields_on_set( allsets, i); */
}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1attribute_1string(
								   JNIEnv *env,
								   jobject obj,
								   jint handle_id)
{

  int att_count = 0, sub_count = 0 , na = 0 , sa = 0, num_names = 0, nn = 0;
  hid_t a_type, b_type;
  void *attribute_value=NULL, *sub_attribute=NULL;
  char tmp_buffer[1024];
  char tiny_buffer[255];
  int result;

  

  saf_get_attribute(SAF_ALL, (ss_pers_t*)db, SAF_ATT_COUNT, NULL, &att_count, NULL);

  if( att_count <= 0 )
    return (*env)->NewStringUTF(env,"no attributes");

  strcpy(tmp_buffer,  "\n\tAttributes:\n");

  if( att_count > 0 ) {

    attribute_value = NULL;
    num_names = 0;
    result = saf_get_attribute(SAF_ALL, (ss_pers_t*)db, SAF_ATT_NAMES, &a_type,  &num_names,
			       (void **)&attribute_value);

    for( na = 0 ; na < num_names ; na++ ) {
      /* sprintf(tiny_buffer,"\t    ",num_names); */
      sprintf(tiny_buffer,"\t    ");
      strcat(tmp_buffer, tiny_buffer);
      strcat(tmp_buffer,((char **)attribute_value)[na]);
      strcat(tmp_buffer," :\n");

      sub_count = 0;
      sub_attribute = NULL;
      result = saf_get_attribute(SAF_ALL, (ss_pers_t*)db, ((char **)attribute_value)[na], &b_type, 
				 &sub_count, (void **)&sub_attribute);
      /* printf("dsl_sub_type is %d:%s\n", sub_count, DSL_stringOf_type(NULL, b_type)); */
      for( sa = 0 ; sa < sub_count ; sa++ ) {
	if(H5T_STRING==H5Tget_class(b_type) )
	  if( sub_count > 1 )
	    sprintf(tiny_buffer, "\t\t%s\n",((char **)sub_attribute)[sa]);
	  else
	    sprintf(tiny_buffer, "\t\t%s\n",((char *)sub_attribute));

	else
	  sprintf(tiny_buffer,"\t\t%s\n",StringDataTypeValue(b_type,((ss_pers_t *)sub_attribute)+ sizeof(b_type)*sa ));

	strcat(tmp_buffer,tiny_buffer);

      }
    }
  }

  return  (*env)->NewStringUTF(env,tmp_buffer);

}



JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1attr_1data(
								 JNIEnv *env,
								 jobject obj,
								 jint handle_id,
								 jstring name,
								 jint num)
{

  int result;
  char *data_string = NULL;
  static int attr_count = 0;
  static hid_t a_type;
  static void *sub_attribute = NULL;
  void *attribute_value=NULL;
  static const jbyte *attr_name=NULL;
  const jbyte *tmp_name=NULL;

  tmp_name = (*env)->GetStringUTFChars(env,name,NULL);

  if( !(strcmp(attr_name,tmp_name)) ) {
    if( num > (attr_count-1) )
        return  (*env)->NewStringUTF(env,"");


    if(H5T_STRING==H5Tget_class(a_type) )
      if( attr_count > 1 )
	data_string = ((char **)sub_attribute)[num] ;
      else
	data_string = ((char *)sub_attribute) ;

    else
      data_string = (char *)StringDataTypeValue(a_type,((ss_pers_t *)sub_attribute)+ sizeof(a_type)*num );

    return  (*env)->NewStringUTF(env,data_string); 

  }

  attr_count = 0;
  sub_attribute = NULL;
  attribute_value = NULL;

  attr_name = tmp_name;

  saf_get_attribute(SAF_ALL, (ss_pers_t*)db, attr_name, NULL, &attr_count, NULL);

  if( num > (attr_count-1) )
    return  (*env)->NewStringUTF(env,"");

  result = saf_get_attribute(SAF_ALL, (ss_pers_t*)db, attr_name, &a_type, 
			     &attr_count, (void **)&sub_attribute);
  
  if( H5T_STRING==H5Tget_class(a_type))
    if( attr_count > 1 )
      data_string = ((char **)sub_attribute)[num] ;
    else
      data_string = ((char *)sub_attribute) ;

  else
    data_string = (char *)StringDataTypeValue(a_type,((ss_pers_t *)sub_attribute)+ sizeof(a_type)*num );

  return  (*env)->NewStringUTF(env,data_string); 
}


JNIEXPORT jboolean JNICALL Java_saf_1wrap_native_1is_1attribute_1primitive(
								      JNIEnv *env,
								      jobject obj,
								      jint handle_id,
								      jstring name)
{
  int attr_count = 0;
  int result;
  hid_t a_type;
  void *attribute_value = NULL;
  const jbyte *attr_name=NULL;

  attr_name = (*env)->GetStringUTFChars(env,name,NULL);

  attribute_value = NULL;
  attr_count = 0;

  result = saf_get_attribute(SAF_ALL, (ss_pers_t*)db, attr_name, &a_type, 
			     &attr_count,NULL);
  result = (!H5Tequal(a_type, H5T_NATIVE_INT));
  
  return (jboolean)result;

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1sub_1attribute_1count(
								    JNIEnv *env,
								    jobject obj,
								    jint handle_id,
								    jstring name)
{

  int att_count = 0, result;
  const jbyte *attr_name=NULL;


  attr_name = (*env)->GetStringUTFChars(env,name,NULL);

  result = saf_get_attribute(SAF_ALL, (ss_pers_t*)db, attr_name, NULL, &att_count, NULL);
  return att_count; 

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1attribute_1count(
							       JNIEnv *env,
							       jobject obj,
							       jint handle_id)
{

  int att_count = 0, result;


  result = saf_get_attribute(SAF_ALL, (ss_pers_t*)db, SAF_ATT_COUNT, NULL, &att_count, NULL);
  return att_count; 
}


JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1attr_1type(
								       JNIEnv *env,
								       jobject obj,
								       jint handle_id,
								       jstring name)
{
  int attr_count = 0, result;
  hid_t a_type;
  const jbyte *attr_name=NULL;

  attr_name = (*env)->GetStringUTFChars(env,name,NULL);

  attr_count = 0;

  result = saf_get_attribute(SAF_ALL, (ss_pers_t*)db, attr_name, &a_type, 
			     &attr_count,NULL);

  return (*env)->NewStringUTF(env,StringDataType(a_type));

}
JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1attr_1name(
							     JNIEnv *env,
							     jobject obj,
							     jint handle_id,
							     jint num)
{
  int num_names = 0, result;
  void *attribute_value = NULL;
  
  attribute_value = NULL;
  num_names = 0;
  result = saf_get_attribute(SAF_ALL, (ss_pers_t*)db, SAF_ATT_NAMES, NULL,  &num_names,
			       (void **)&attribute_value);
  if( num > (num_names-1) )
    return (*env)->NewStringUTF(env,"");

  return (*env)->NewStringUTF(env,((char **)attribute_value)[num]);

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1field_1info(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint coll_num)
{

	int i, j;

	char buffer[400];
	char tmp[400];
	Set_Info *set_info;
	char *cat_name;
	SAF_Cat storage_decomp;


	set_info = (Set_Info *)set_id;


	if( (set_info->fields) == NULL ||  (set_info->num_fields == 0) ) {
		strcpy(buffer,"");
		return (*env)->NewStringUTF(env,buffer);
	}

	strcpy(buffer,"");

		for( j = 0; j < set_info->num_fields; j++ ) {
			if( set_info->fields[j].my_set_coll_num == coll_num ) {
				sprintf(tmp,"\n\t%s ",set_info->fields[j].field_name);
				strcat(buffer,tmp);
				if( (set_info->fields[j].storage_decomp != NULL) && ! SAF_EQUIV( SAF_SELF(db), set_info->fields[j].storage_decomp) ) {
					storage_decomp = *(set_info->fields[j].storage_decomp);
					cat_name = NULL;
					saf_describe_category(SAF_ALL, &storage_decomp,&cat_name,NULL,NULL);
					sprintf(tmp,", storage decomposition category: %s", cat_name);
					strcat(buffer,tmp);
				}
			}

		}

	return (*env)->NewStringUTF(env,buffer);
	
}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1num_1fields_1on_1coll(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint coll_num)
{

	int i, num = 0;
	Set_Info *set_info;
	
	set_info = (Set_Info *)set_id;

	if( (set_info->fields) == NULL ||  (set_info->num_fields == 0) ) {
		return 0;
	}


	for( i = 0; i < set_info->num_fields; i++ ) {
		if( set_info->fields[i].my_set_coll_num == coll_num ) {
			num++;
		}
	}

	return num;

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1rel_1id(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint coll_num,
	jint field_num)
{

	int i, count = -1;
	Set_Info *set_info;

	set_info = (Set_Info *)set_id;

	for( i = 0; i < set_info->num_fields; i++ ) {
		if( set_info->fields[i].my_set_coll_num == coll_num ) {
			count++; 
			if( count == field_num  )
				return (jint)(i);
		}
	}
	return 0;
}
JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1id(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint field_num)
{

	int i;
	Set_Info *set_info;
	Field_Info *field_info;

	set_info = (Set_Info *)set_id;
	field_info = set_info->fields;
	field_info = field_info + field_num;
	return (int)(field_info);

}


JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1field_1menu_1string(
	JNIEnv *env,
	jobject obj,
	jint set_id,
	jint field_id)
{
	Set_Info *set_info;

	set_info = (Set_Info *)set_id;
	return (*env)->NewStringUTF(env,set_info->fields[field_id].field_name);

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1field_1name(
	JNIEnv *env,
	jobject obj,
	jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (*env)->NewStringUTF(env,field_info->field_name);

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1field_1data_1type(
JNIEnv *env, jobject obj,  jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (*env)->NewStringUTF(env,field_info->data_type);

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1field_1interleave(
JNIEnv *env, jobject obj, jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;
	
	return (*env)->NewStringUTF(env,field_info->interleave);
}



JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1data_1size(
JNIEnv *env, jobject obj, jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return field_info->data_size;
}


JNIEXPORT jboolean JNICALL Java_saf_1wrap_native_1get_1field_1is_1coord(
JNIEnv *env, jobject obj, jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (jboolean)field_info->is_coord;

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1coeff_1assoc_1id(
JNIEnv *env, jobject obj, jint field_id)
{

	char *catname;
	SAF_Cat *the_cat;

	Field_Info *field_info;

	
	field_info = (Field_Info *)field_id;

	the_cat = field_info->coeff_assoc;

	/*
	catname = NULL;
	saf_describe_category(SAF_ALL, the_cat,&catname,NULL,NULL);

	printf("get_1field_1coeff_1assoc_1id: field_name: %s, coeff_assoc: %s\n",field_info->field_name, catname); 
	*/

	return (int)(field_info->coeff_assoc);

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1assoc_1ratio(
JNIEnv *env, jobject obj, jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (int)(field_info->assoc_ratio);

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1storage_1decomp_1id(
JNIEnv *env, jobject obj, jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (int)(field_info->storage_decomp);

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1eval_1coll_1id(
JNIEnv *env, jobject obj, jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (int)(field_info->eval_coll);
}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1num_1comps(
JNIEnv *env, jobject obj, jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (int)field_info->num_comps;
}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1component(
JNIEnv *env, jobject obj, jint field_id, jint field_num)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	if( field_info->component_fields == NULL )
		return 0;

	field_info = (Field_Info *)(field_info->component_fields) + field_num;
	return (int)field_info;
}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1field_1eval_1func(
JNIEnv *env, jobject obj, jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (*env)->NewStringUTF(env,field_info->eval_func);

}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1fieldtmpl_1id(
	JNIEnv *env,
	jobject obj,
	jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (int)(field_info->template_info);

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1fieldtmpl_1name(
	JNIEnv *env,
	jobject obj,
	jint fieldtmpl_id)
{


	FieldTmpl_Info *fieldtmpl_info;

	fieldtmpl_info = (FieldTmpl_Info *)fieldtmpl_id;

	return (*env)->NewStringUTF(env,fieldtmpl_info->fieldtmpl_name);

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1fieldtmpl_1num_1comp(
	JNIEnv *env,
	jobject obj,
	jint fieldtmpl_id)
{

	int num_comps;
	FieldTmpl_Info *fieldtmpl_info;

	fieldtmpl_info = (FieldTmpl_Info *)fieldtmpl_id;

	num_comps = fieldtmpl_info->num_comps;
	return num_comps;

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1fieldtmpl_1component(
	JNIEnv *env,
	jobject obj,
	jint fieldtmpl_id,
	jint fieldtmpl_num)
{

	FieldTmpl_Info *fieldtmpl_info;
	FieldTmpl_Info *component;
	fieldtmpl_info = (FieldTmpl_Info *)fieldtmpl_id;

	if( fieldtmpl_info->component_tmpls == NULL )
		return 0;

	component = fieldtmpl_info->component_tmpls;
	component = component + fieldtmpl_num;
	return (jint)(component);

}


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1field_1base_1space_1id(
	JNIEnv *env,
	jobject obj,
	jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;
	return (jint)(field_info->base_space);

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1field_1base_1space_1name(
	JNIEnv *env,
	jobject obj,
	jint field_id)
{

	Field_Info *field_info;

	field_info = (Field_Info *)field_id;

	return (*env)->NewStringUTF(env,field_info->base_space_name);

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1fieldtmpl_1atype(
	JNIEnv *env,
	jobject obj,
	jint fieldtmpl_id)
{

	FieldTmpl_Info *fieldtmpl_info;

	fieldtmpl_info = (FieldTmpl_Info *)fieldtmpl_id;

	return (*env)->NewStringUTF(env,fieldtmpl_info->algtype);

}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1fieldtmpl_1basis(
	JNIEnv *env,
	jobject obj,
	jint fieldtmpl_id)
{
	FieldTmpl_Info *fieldtmpl_info;
	fieldtmpl_info = (FieldTmpl_Info *)fieldtmpl_id;

	return (*env)->NewStringUTF(env,fieldtmpl_info->basis);

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1state_1num_1states(
JNIEnv *env, jobject obj, jint state_id)
{
  Field_Info *field_info;
  SAF_Field *saf_field;
  int num_states = 0;

  field_info = (Field_Info *)state_id;

  if( field_info == NULL )
    return 0;

  saf_field = field_info->the_field;

  saf_describe_state_group( SAF_ALL, saf_field, NULL, NULL, NULL, NULL, NULL, NULL, &num_states);
  return num_states;
}

JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1get_1state_1name(
JNIEnv *env, jobject obj, jint state_id)
{

	Field_Info *field_info;
	char *name;
	SAF_StateTmpl stmpl;

	field_info = (Field_Info *)state_id;

	name = NULL;

	return (*env)->NewStringUTF(env,name);

}

JNIEXPORT jint JNICALL Java_saf_1wrap_native_1get_1state_1field_1on_1state(
JNIEnv *env, jobject obj, jint state_id, jint num)
{

	Field_Info *field_info;
	Field_Info *comp_field_info;
	SAF_StateTmpl state_tmpl;
	SAF_Field *fields;
	void *coords;
	int i, index[1];
	int num_tmpls;

	field_info = (Field_Info *)state_id;

	saf_describe_state_tmpl(SAF_ALL, &state_tmpl, NULL, &num_tmpls, NULL );

	/* remove this when the field count bug is fixed */
	num_tmpls = 1;

	index[0] = 1;
	coords = NULL;
	fields = NULL;
	if( field_info->component_fields == NULL ) {
		if( fields != NULL ) {
			comp_field_info = (Field_Info *)calloc((size_t) num_tmpls, sizeof(Field_Info));
			for( i = 0 ; i < num_tmpls; i++ ) {
				comp_field_info[i].the_field = fields + i;

				get_field_info( comp_field_info + i );
			}
		}
		
		field_info->component_fields = (void *)comp_field_info;
	}

	comp_field_info = (Field_Info *)(field_info->component_fields);

	if( (num > (num_tmpls - 1)) || comp_field_info == NULL )
		return 0;

	return (int)(comp_field_info + num);

}

void find_collections(  Set_Info *the_set, int set_num )
{

	int i,j, num_colls, dont_add_saf_self;
	SAF_Set saf_set;
	SAF_Cat *saf_cats;
	SAF_Cat *saf_self;
	Set_Info *tmp_set;
	char *name;
	int count;
	SAF_Role role;
	SAF_IndexSpec ispec;
	SAF_DecompMode is_decomp;
	int tdim;
	SAF_CellType celltype;

	tmp_set = the_set + set_num;

	if (tmp_set->collections != NULL)
		return;


	saf_cats = NULL;
	saf_set = tmp_set->the_set;

	saf_find_collections(SAF_ALL, &saf_set, SAF_ANY_ROLE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
		&num_colls, &saf_cats);

	dont_add_saf_self = 0;


        saf_self = (SAF_Cat *)SAF_SELF(db);
	for( i = 0 ; i < num_colls; i++ ) {
		if( SAF_EQUIV(saf_self,saf_cats+i) ) {
			dont_add_saf_self = 1;
		}
	}


	if( dont_add_saf_self == 0 ) {
		free(saf_cats);
		saf_cats = (SAF_Cat *)calloc( (size_t) (num_colls+1), sizeof(SAF_Cat) );
                /* erik  num_colls += 1; */
		saf_find_collections(SAF_ALL, &saf_set, SAF_ANY_ROLE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
			&num_colls, &saf_cats);
/* erik
		saf_cats[num_colls] = *saf_self;
		num_colls++;
*/
	}
	tmp_set->num_colls = num_colls;

/*
        if( num_colls == 0 ) {
                tmp_set->collections = NULL;
                return;
        }
*/


	tmp_set->collections = (Collection_Info *)(calloc((size_t) num_colls, sizeof(Collection_Info)));
        j = 0;
        for( i = 0 ; i < num_colls; i++ ) {
          name = NULL;
          saf_describe_category(SAF_ALL, &(saf_cats[i]), &name, &role, &tdim);
          saf_describe_collection(SAF_ALL,&saf_set,&(saf_cats[i]),&celltype,&count,NULL,NULL,NULL);
                                                                                                                             
          if( celltype == SAF_CELLTYPE_SET ) {
            tmp_set->collections[j].the_cat = saf_cats[i];
            tmp_set->collections[j].cat_name = name;
            tmp_set->collections[j].count = count;
            tmp_set->collections[j].celltype = celltype_string((int)  celltype );
            tmp_set->collections[j].is_decomp = (unsigned char)is_decomp;
            j++;
          }
        }
                                                                                                                             
        for( i = 0 ; i < num_colls; i++ ) {
          name = NULL;
          saf_describe_category(SAF_ALL, &(saf_cats[i]), &name, &role, &tdim);
          saf_describe_collection(SAF_ALL,&saf_set,&(saf_cats[i]),&celltype,&count,NULL,NULL,NULL);
                                                                                                                             
          if( celltype != SAF_CELLTYPE_SET ) {
            tmp_set->collections[j].the_cat = saf_cats[i];
            tmp_set->collections[j].cat_name = name;
            tmp_set->collections[j].count = count;
            tmp_set->collections[j].celltype = celltype_string((int)  celltype );
            tmp_set->collections[j].is_decomp = (unsigned char)is_decomp;
            j++;
          }
                                                                                                                             
                                                                                                                             
          tmp_set->collections[i].child_setlink = NULL;
          tmp_set->collections[i].subset_relation = NULL;
          tmp_set->collections[i].subset_relation = NULL;
          tmp_set->collections[i].num_subset_relations = -1;
          tmp_set->collections[i].topo_link = NULL;
          tmp_set->collections[i].topo_domain_id = NULL;
          tmp_set->collections[i].parent_setlink = NULL;
          tmp_set->collections[i].parent_colllink = 0;
                                                                                                                             
                                                                                                                             
        }


/*
	for( i = 0 ; i < num_colls; i++ ) {

		name = NULL;
		saf_describe_category(SAF_ALL, saf_cats+i, &name, &role, &tdim);
		saf_describe_collection(SAF_ALL,&saf_set,saf_cats+i,&celltype,&count,&ispec,&is_decomp,NULL);
		tmp_set->collections[i].the_cat = saf_cats[i];
		tmp_set->collections[i].cat_name = name;
		tmp_set->collections[i].count = count;
		tmp_set->collections[i].celltype = celltype_string((int)  celltype );
		tmp_set->collections[i].is_decomp = (unsigned char)is_decomp;
		
		tmp_set->collections[i].child_setlink = NULL;
		tmp_set->collections[i].subset_relation = NULL;
		tmp_set->collections[i].num_subset_relations = 0;
		tmp_set->collections[i].topo_link = NULL;
		tmp_set->collections[i].topo_domain_id = NULL;
		tmp_set->collections[i].parent_setlink = NULL;
		tmp_set->collections[i].parent_colllink = 0;

	}
*/
}



JNIEXPORT void JNICALL Java_saf_1wrap_native_1close(
  JNIEnv *env,
  jobject obj,
  jint file_id)
{

	int status;

	if ( (status = saf_close_database(db)) != SAF_SUCCESS ) {
		/* printf("saf_close_database: unsuccessful"); */
		return;
	}
	/*
	free(allsets);
	free(all_sets);
	free(top_sets);
	free(tops);
	*/
	/* saf_final(); */

	dbname = NULL;
	file_id = -1;
	return;
}




JNIEXPORT jstring JNICALL Java_saf_1wrap_native_1tree(
  JNIEnv *env,
  jobject obj){

  return (*env)->NewStringUTF(env,"empty string");
}



  char * basis_string( SAF_Basis *basis ) {

    char *name = NULL;

    if (SS_BASIS(basis))
        saf_describe_basis(SAF_ALL, basis, &name, NULL);


    return name;


  }

  char * evalfunc_string( SAF_Eval *eval ) {

    char *name = NULL;

    saf_describe_evaluation(SAF_ALL, eval, &name, NULL);

    return name;

  }



  char * interleave_string(int interleave ) {

	char string[40];
	strcpy(string, "Unknown");
	switch( interleave ) {

		case -4: strcpy(string, "UNKNOWN");
		case -3: strcpy(string, "NA");
		case -2: strcpy(string, "INVALID");
		case -1: strcpy(string, "ANY");
		case 0: strcpy(string, "COMPONENT");
		case 1: strcpy(string, "VECTOR");
		case 2: strcpy(string, "INDEPENDENT");
		case 3: strcpy(string, "NONE");
		default: strcpy(string, "Undefined");
		return(strdup(string));

	}
/*
enum Interleave
{
   VBT_INTERLEAVE_COMPONENT,
   VBT_INTERLEAVE_VECTOR,
   VBT_INTERLEAVE_INDEPENDENT,
   VBT_INTERLEAVE_NONE,
   VBT_INTERLEAVE_ANY = -1,
   VBT_INTERLEAVE_INVALID = -2,
   VBT_INTERLEAVE_NA = -3,
   VBT_INTERLEAVE_UNKNOWN = -4
};
*/

  }




  char * StringDataType( hid_t type ) {


		char string[40];
		strcpy(string, "Unknown");
                if (type && (type > 0)) {
		   if(H5Tequal(type, SAF_CHAR) ) strcpy(string,"SAF_CHAR");
		   if(H5Tequal(type, SAF_FLOAT)) strcpy(string,"SAF_FLOAT");
		   if(H5Tequal(type, SAF_HANDLE)) strcpy(string,"SAF_HANDLE");
		   if(H5Tequal(type, SAF_INT)) strcpy(string,"SAF_INT");
		   if(H5Tequal(type, SAF_LONG)) strcpy(string,"SAF_LONG");
		   if(H5Tequal(type, SAF_DOUBLE)) strcpy(string,"SAF_DOUBLE");
		   if(H5T_STRING==H5Tget_class(type)) strcpy(string,"SAF_STRING");
                 }
		return(strdup(string));
  }

  char * algtype_string(SAF_Algebraic *algtype) {

    char *name = NULL;

    saf_describe_algebraic(SAF_ALL, algtype, &name, NULL,NULL);

    return name;

  }

  char * role_string(int id) {

	char string[40];
	strcpy(string, "Undefined SAF_ROLE");
	switch(id) {
		case 0: strcpy(string, "SAF_TOPOLOGY");
		case 1: strcpy(string, "VBT_ROLE_BND");
		case 2: strcpy(string, "SAF_PROCESSOR");
		case 3: strcpy(string, "SAF_DOMAIN");
		case 4: strcpy(string, "SAF_BLOCK");
		case 5: strcpy(string, "SAF_ASSEMBLY");
		case 6: strcpy(string, "SAF_MATERIAL");
		case 7: strcpy(string, "VBT_ROLE_XPROD");
		case 8: strcpy(string, "SAF_USERD");
		default: strcpy(string, "Undefined SAF_ROLE");
	}
        return(strdup(string));
  }

  char * silrole_string(SAF_SilRole srole) {

    char string[40];
    strcpy(string, "Unknown");
    if( srole == SAF_SPACE )
      strcpy(string, "SAF_SPACE");
    if( srole == SAF_SUITE )
      strcpy(string, "SAF_SUITE");
    if( srole == SAF_TIME )
      strcpy(string, "SAF_TIME");
    if( srole == SAF_PARAM )
      strcpy(string, "SAF_PARAM");
    if( srole == SAF_ANY_SILROLE )
      strcpy(string, "SAF_ANY_SILROLE");
    
    return(strdup(string));

  }

  char * extmode_string(int id) {

        char string[40];
        strcpy(string, "Undefined");
	switch(id) {
		case 0: strcpy(string, "SAF_EXTENDIBLE_FALSE");
		case 1: strcpy(string, "SAF_EXTENDIBLE_TRUE");
		case 2: strcpy(string, "SAF_EXTENDIBLE_TORF");
		default: strcpy(string, "Undefined");
	}
    return(strdup(string));
  }

  char * celltype_string(int id) {

        char string[40];
        strcpy(string, "Undefined CELLTYPE");
	switch(id) {
		case 0: strcpy(string, "SAF_SET");
		case 1: strcpy(string, "SAF_POINT");
		case 2: strcpy(string, "SAF_LINE");
		case 3: strcpy(string, "SAF_TRI");
		case 4: strcpy(string, "SAF_QUAD");
		case 5: strcpy(string, "SAF_TET");
		case 6: strcpy(string, "SAF_PYRAMID");
		case 7: strcpy(string, "SAF_PRISM");
		case 8: strcpy(string, "SAF_HEX");
		case 9: strcpy(string, "SAF_MIXED");
		case 10: strcpy(string, "SAF_ARB");
		case 11: strcpy(string, "SAF_1BALL");
		case 12: strcpy(string, "SAF_2BALL");
		case 13: strcpy(string, "SAF_3BALL");
		case 14: strcpy(string, "SAF_1SHELL");
		case 15: strcpy(string, "SAF_2SHELL");
		case 16: strcpy(string, "SAF_ANY_CELL_TYPE");
		default: strcpy(string, "Undefined CELLTYPE");
	}
    return(strdup(string));
   }

  char * objecttype_string(int id) {

        char string[40];
        strcpy(string, "Undefined CELLTYPE");
	switch(id) {
		case 1: strcpy(string, "_SAF_OBJTYPE_DB");
		case 2: strcpy(string, "_SAF_OBJTYPE_CAT");
		case 3: strcpy(string, "_SAF_OBJTYPE_FIELD");
		case 4: strcpy(string, "_SAF_OBJTYPE_FILE");
		case 5: strcpy(string, "_SAF_OBJTYPE_FTMPL");
		case 6: strcpy(string, "_SAF_OBJTYPE_REL");
		case 7: strcpy(string, "_SAF_OBJTYPE_SEQ");
		case 8: strcpy(string, "_SAF_OBJTYPE_SET");
		case 9: strcpy(string, "_SAF_OBJTYPE_STATE");
		case 10: strcpy(string, "_SAF_OBJTYPE_STMPL");
		case 11: strcpy(string, "_SAF_OBJTYPE_QUANTITY");
		case 12: strcpy(string, "_SAF_OBJTYPE_UNIT");
		case 13: strcpy(string, "_SAF_OBJTYPE_BASE");
		default: strcpy(string, "Undefined SAF_OBJTYPE");
	}
    return(strdup(string));
   }


JNIEXPORT jint JNICALL Java_saf_1wrap_native_1find_1set_1field_1tmpls(
	JNIEnv *env,
	jobject obj,
	jint set_id)
{

	SAF_Set saf_set;
	Set_Info *set_info;

	set_info = (Set_Info *)set_id;
	saf_set = set_info->the_set;

}


char *saf_set_name( SAF_Set *SAF_subset) 
{


	char *set_name=NULL;
	int tdim;
	int num_colls;
	SAF_TopMode t_mode;
	SAF_ExtendMode e_mode;
	SAF_SilRole srole;

	saf_describe_set(SAF_ALL, SAF_subset, &set_name, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);

	return set_name;

}




