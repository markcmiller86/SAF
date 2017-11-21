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

#include <saf.h>
#define READER_DECLARE
#include <readutils.h>


int not_in_name( char *name, char **dict, int size ) {
  int i;

  for( i = 0; i < size; i++ ) {
    if( !strcmp(name, dict[i]) )
      return 0;
  }
  return 1;

}




/*
find_fields_on_set

Given a linked list of Set_Info structures and an index into the
list, find all fields whose base_space is this set and create
a linked list of Field_Info structures to be stored in the 
Set_Info structure for this set.

This function calls get_field_info() for each field as a way
of obtaining Field_Info parameters for each field.

*/
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
	/* SAF_Cat saf_self; */
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



	tmp_set->fields = calloc(num_fields, sizeof(Field_Info));

	saf_find_fields(SAF_ALL, db, &saf_set, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, \
			SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &num_fields, &fields);



	for( i = 0; i < num_fields; i++ ) {

		name = NULL;
		tmp_set->fields[i].the_field = fields + i;
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
	

		
		if( ! SAF_EQUIV(storage_decomp,SAF_SELF(db)) ) {
			name = NULL;
			saf_describe_category(SAF_ALL, storage_decomp,&name,NULL,NULL);
		}
		else {
			field_info->storage_decomp=NULL;
		}
	
	}

}

/*
get_field_info

Given a Field_Info structure, obtain information about the
field and its field_template.  Uses SAFAPI functions saf_describe_field,
get_count_and_type_for_field, etc  and the local function get_fieldtmpl_info
to obtain field parameters and properites.

*/
void get_field_info( Field_Info *field_info )
{

	int i;
	static size_t count = 0;
	static hid_t Ptype;
	SAF_Field *field;
	SAF_FieldTarget *ftarget;
	SAF_Set *base_space;
	SAF_FieldTmpl *field_template;
	FieldTmpl_Info *template_info;
	SAF_Cat *storage_decomp;
	SAF_Cat *coeff_assoc;
	SAF_Cat *eval_coll;
	char *name;
	hbool_t is_coord;
	int assoc_ratio;
	SAF_Eval eval_func;
	int num_comps;
	SAF_Field *components=NULL;
	Field_Info *component_fields;
	SAF_Interleave interleave;
	int *comp_order;

	

	field = field_info->the_field;
	ftarget = field_info->ftarget;

        base_space = (SAF_Set *)calloc(1, sizeof(SAF_Set));
	
                template_info = (FieldTmpl_Info *)calloc(1, sizeof(FieldTmpl_Info));
                field_template = (SAF_FieldTmpl *)calloc(1, sizeof(SAF_FieldTmpl));

		name = NULL;
		comp_order = NULL;

		coeff_assoc = (SAF_Cat *)calloc(1, sizeof(SAF_Cat));
		storage_decomp = (SAF_Cat *)calloc(1, sizeof(SAF_Cat));
		eval_coll = (SAF_Cat *)calloc(1, sizeof(SAF_Cat));



		saf_describe_field(SAF_ALL, field, field_template, &name, base_space, NULL, &is_coord, storage_decomp, \
					coeff_assoc, &assoc_ratio, eval_coll, &eval_func, NULL, \
					&num_comps, NULL, &interleave, NULL);

		for( i = 0 ; i < num_all_sets; i++ ) {
		  if( SAF_EQUIV(base_space, &(allsets[i].the_set)) )
		    field_info->base_space = (void *)(allsets + i);
		}
		field_info->base_space_name = saf_set_name(base_space);


		saf_get_count_and_type_for_field( SAF_ALL, field, ftarget, &count, &Ptype);



		template_info->fieldtmpl = field_template;


		get_fieldtmpl_info( template_info );


		/* if( strcmp(template_info->algtype,"SAF_FIELD") )  { */


		if( strcmp(template_info->algtype,"field") )  { 


			name = NULL;
			comp_order = NULL;
			saf_describe_field(SAF_ALL, field, field_template, &name, NULL, NULL, &is_coord, NULL, \
					NULL, &assoc_ratio, NULL, &eval_func, NULL, \
					&num_comps, &components, &interleave, NULL);

		}

		/* a bug with Ptype in saf_describe_field for some state fields requires that we
		   use this call to get Ptype instead 
		*/
		saf_get_count_and_type_for_field(SAF_ALL, field,ftarget, NULL, &Ptype);

		field_info->template_info = template_info;
		field_info->coeff_assoc = coeff_assoc;
		field_info->assoc_ratio = assoc_ratio;
		field_info->eval_coll = eval_coll;
		field_info->storage_decomp = storage_decomp;
		field_info->field_name = name;
		/*		field_info->eval_func = evalfunc_string(&eval_func); */
		field_info->is_coord = (unsigned char)is_coord;
		field_info->data_type = Ptype;
		field_info->data_size = count;
		field_info->field_data = NULL;

		if( components == NULL ) {
			field_info->component_fields = NULL;
			field_info->num_comps = 0;
		}
		else {
		        field_info->num_comps = num_comps;
			component_fields = (Field_Info *)calloc(num_comps, sizeof(Field_Info));
			field_info->component_fields = component_fields;
			for( i = 0; i < num_comps; i++ ) {
				component_fields[i].the_field = components + i;
				get_field_info( component_fields + i );
				
			}
		}

}

/*

get_fieldtmpl_info

Given and FieldTmpl_Info structure, obtain field_template
parameters and properties for the field_template.

Uses the SAFAPI function saf_describe_field_tmpl and other
SAFAPI saf_describe_* calls

*/
void get_fieldtmpl_info(  FieldTmpl_Info *template_info )
{

	int i, num_comps=0;
	SAF_FieldTmpl *field_template;
	/* SAF_Set *base_space; */
	/* SAF_AlgType algtype; */
	SAF_Algebraic algtype;
	SAF_Basis basis;
	FieldTmpl_Info *component_tmpls=NULL;
	SAF_FieldTmpl *components=NULL;
	char *name;


		name = NULL;
		/* base_space = (SAF_Set *)malloc(sizeof(SAF_Set)); */

		field_template = template_info->fieldtmpl;



		if( field_template == NULL ) {
			printf("field_template is NULL\n");
			return;
		}




		saf_describe_field_tmpl(SAF_ALL,field_template,&name, /* base_space, */ &algtype,&basis,NULL,&num_comps,NULL);


		if( num_comps > 0 ) {
			saf_describe_field_tmpl(SAF_ALL,field_template,NULL,NULL,NULL,NULL,&num_comps,&components);
		}



		if( components == NULL ) {
			template_info->component_tmpls = NULL;
		}
		else {


                        component_tmpls = (FieldTmpl_Info *)calloc(num_comps, sizeof(FieldTmpl_Info));
			template_info->component_tmpls = component_tmpls;
			for( i = 0; i < num_comps; i++ ) {
				component_tmpls[i].fieldtmpl = components + i;

				get_fieldtmpl_info( component_tmpls + i );
				
			}
		}

		template_info->fieldtmpl_name = name;
		template_info->num_comps = num_comps;

        /*
		for( i = 0 ; i < num_all_sets; i++ ) {
		  if( SAF_EQUIV(*base_space, allsets[i].the_set) )
		    template_info->base_space = (void *)(allsets + i);
		}
		template_info->base_space_name = saf_set_name(base_space);
        */

		template_info->algtype =  algtype_string(&algtype);
		template_info->basis = basis_string(&basis);
}


/*

saf_set_name

a utility function that obtains and returns the name for 
a given SAF_Set

*/
char *saf_set_name( SAF_Set *SAF_subset) 
{


	char *set_name=NULL;

	saf_describe_set(SAF_ALL, SAF_subset, &set_name, NULL, NULL, NULL, NULL, NULL, NULL);

	return set_name;

}


/* 
algtype_string

a utility function that returns the description name
for a given SAF_Algebraic

*/
  char * algtype_string(SAF_Algebraic *algtype) {

    char *name = NULL;

    saf_describe_algebraic(SAF_ALL, algtype, &name, NULL,NULL);

    return name;

  }


/* 
basis_string

a utility function that returns the description name
for a given SAF_Basis

*/
char *
basis_string(SAF_Basis *basis)
{
    char *name = NULL;

    if (SS_PERS_ISNULL(basis)) {
        name = calloc(1, 1);
    } else {
        saf_describe_basis(SAF_ALL, basis, &name, NULL);
    }
    return name;
}


/* 

saftype_string

a utility function that returns the description 
namd for a given hid_t

*/
  const char * saftype_string( hid_t data_type ) {


		if( H5Tequal(data_type, SAF_CHAR) ) return("SAF_CHAR");
		if( H5Tequal(data_type, SAF_FLOAT)) return("SAF_FLOAT");
		if( H5Tequal(data_type, SAF_HANDLE)) return("SAF_HANDLE");
		if( H5Tequal(data_type, SAF_INT)) return("SAF_INT");
		if( H5Tequal(data_type, SAF_LONG)) return("SAF_LONG");
		if( H5Tequal(data_type, SAF_DOUBLE)) return("SAF_DOUBLE");
		if( H5Tequal(data_type, H5T_C_S1)) return("H5T_C_S1");
		return("Unknown");
  }



/*
find_collections

Given and linked list of Set_Info structures and an index
into the list, find all the collection categories that are
defined "on" this set.

Initializes a linked list of Collection_Info structures which
will later be used for storing subset relation parent and
child links for representing the "Subset Inclusion Lattice tree"

For display and reporting purposes, all SAF_CELLTYPE_SET categories
are stored in the list first.
*/
void find_collections(  Set_Info *the_set, int set_num )
{

	int i, j, num_colls, dont_add_saf_self;
	SAF_Set saf_set;
	SAF_Cat *saf_cats;
	SAF_Cat *saf_self;
	Set_Info *tmp_set;
	char *name;
        int count;
	SAF_Role role;
	int tdim;
	SAF_CellType celltype;




	tmp_set = the_set + set_num;

	if (tmp_set->collections != NULL)
		return;



	saf_cats = NULL;
	saf_set = tmp_set->the_set;

	/*
	saf_find_collections(SAF_ALL, &saf_set, SAF_ANY_ROLE, SAF_ANY_CELL_TYPE, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
		&num_colls, NULL);
	*/
	saf_find_collections(SAF_ALL, &saf_set, SAF_ANY_ROLE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
		&num_colls, &saf_cats);



/*	saf_self = SAF_SELF(db);   # <<<< use to be */
	saf_self = (SAF_Cat *)SAF_SELF(db);
	dont_add_saf_self = 0;


	for( i = 0 ; i < num_colls; i++ ) {
		if( SAF_EQUIV(saf_self,saf_cats+i) ) {
			dont_add_saf_self = 1;
		}
	}

	if( dont_add_saf_self == 0 ) {
		free(saf_cats);
		saf_cats = (SAF_Cat *)calloc( (num_colls+1), sizeof(SAF_Cat) );
		saf_find_collections(SAF_ALL, &saf_set, SAF_ANY_ROLE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
			&num_colls, &saf_cats);
                /* saf_cats[num_colls] = SAF_SELF(*db); # <<<< use to be */
		saf_cats[num_colls] = *saf_self;
		num_colls++;
	}




	tmp_set->num_colls = num_colls;

	if( num_colls == 0 ) {
		tmp_set->collections = NULL;
		return;
	}

	tmp_set->collections = (Collection_Info *)(calloc(num_colls, sizeof(Collection_Info)));



	j = 0;
	for( i = 0 ; i < num_colls; i++ ) {
	  name = NULL;
	  saf_describe_category(SAF_ALL, &(saf_cats[i]), &name, &role, &tdim);
	  saf_describe_collection(SAF_ALL,&saf_set,&(saf_cats[i]),&celltype,&count,NULL,NULL,NULL);

	  if( celltype == SAF_CELLTYPE_SET ) {
	    tmp_set->collections[j].the_cat = saf_cats[i];
	    tmp_set->collections[j].cat_name = name;
	    tmp_set->collections[j].count = count;
	    tmp_set->collections[j].celltype = celltype;
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
	    tmp_set->collections[j].celltype = celltype;
	    j++;
	  }

		
	  tmp_set->collections[i].child_setlink = NULL;
	  tmp_set->collections[i].scl = NULL;
	  tmp_set->collections[i].scn = NULL;
	  tmp_set->collections[i].num_children = 0;
	  tmp_set->collections[i].subset_relation = NULL;
	  tmp_set->collections[i].relation_data = NULL;
	  tmp_set->collections[i].num_subset_relations = -1;
	  tmp_set->collections[i].topo_link = NULL;
	  tmp_set->collections[i].topo_domain_id = NULL;
	  tmp_set->collections[i].parent_setlink = NULL;
	  tmp_set->collections[i].parent_colllink = 0;


	}
}

/*
get_subsets

Given a collection defined by the Set_Info  and an index into
the Collection_Info linked list stored on the set, find all sets
that are subset relation children of the given collection.

Initialize a linked list of Set_Info structures that represent
the list of subsets of the given collection and fill in links
that point these subset to their parent (the given collection).

For each of the subsets found, recursively call get_subsets
thereby recursively building the subset relation "tree"
*/

Set_Info * get_subsets( Set_Info *set_info, int coll_id )
{
	int i, j, as;
	int number_of_subsets=0;
	int num_rels;
	int num_children;

	SAF_Rel *SAF_rels;
	SAF_Set *SAF_subsets=NULL;
	
	Set_Info *subsets;
	Collection_Info cat_info;

	cat_info = set_info->collections[coll_id];


	if( set_info->collections[coll_id].child_setlink != NULL ) 
		return (set_info->collections[coll_id].child_setlink);


	/*

	if( cat_info.celltype == SAF_CELLTYPE_SET ) {
	   int coll_count;

	  saf_describe_collection(SAF_ALL, set_info->the_set, cat_info.the_cat, NULL, &coll_count, NULL, NULL, &SAF_subsets);

	  saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,set_info->the_set, cat_info.the_cat, &number_of_subsets, NULL);
	}
	else {
	  saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,set_info->the_set, cat_info.the_cat, &number_of_subsets, &SAF_subsets);
	}
	*/
	 saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,&(set_info->the_set), &(cat_info.the_cat), &number_of_subsets, &SAF_subsets);

	if( number_of_subsets > 0 ) {

          subsets = (Set_Info *)(calloc(number_of_subsets, sizeof(Set_Info)));
	  set_info->collections[coll_id].num_subset_relations = number_of_subsets;
	  (set_info->collections[coll_id]).scn = (int *)calloc(number_of_subsets, sizeof(int));
	}
	else {
		subsets = NULL;
		set_info->collections[coll_id].child_setlink=NULL;
	}


	if( subsets == NULL ) {
		return 0;
	}




	for( i = 0; i < number_of_subsets; i++ ) {


		subsets[i].the_set = SAF_subsets[i];
		subsets[i].set_name = saf_set_name(SAF_subsets + i);
		get_set_info(subsets + i);
		subsets[i].num_sets = number_of_subsets;
		subsets[i].my_num_in_set_list = i;
		subsets[i].topo_relation = NULL;
		subsets[i].fields = NULL;
		subsets[i].num_fields=0;
		subsets[i].collections = NULL;



		for( as = 0 ; as < num_all_sets; as++ ) {

			if( SAF_EQUIV( &(allsets[as].the_set), &(SAF_subsets[i])) ) {

				subsets[i].fields = allsets[as].fields;
				subsets[i].collections = allsets[as].collections;
				subsets[i].num_fields = allsets[as].num_fields;
				subsets[i].num_colls = allsets[as].num_colls;

			}
		}
	}



	for( i = 0 ; i < number_of_subsets; i++ ) {

		/* find_collections(subsets, i); */
		/*
		find_fields_on_set(subsets,i);
		*/

			
		for( j = 0 ; j < (subsets+i)->num_colls; j++ )
			get_subsets( (subsets+i), j );
		


		if( subsets[i].collections != NULL ) {
			for( j = 0 ; j < subsets[i].num_colls; j++ ) {

				SAF_rels = NULL ;

				saf_find_subset_relations(SAF_ALL, db,&(set_info->the_set), 
					&(subsets[i].the_set), &(subsets[i].collections[j].the_cat),
					&(set_info->collections[coll_id].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,
					&num_rels,NULL);

				if( num_rels > 0 ) {
					saf_find_subset_relations(SAF_ALL, db, &(set_info->the_set), 
						&(subsets[i].the_set), &(subsets[i].collections[j].the_cat),
						&(set_info->collections[coll_id].the_cat),SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,
						&num_rels,&SAF_rels);


					/* the subset gets the list of subset relations between it */
					/* and the superset. */

					subsets[i].collections[j].num_subset_relations = num_rels;
					subsets[i].collections[j].subset_relation = SAF_rels;

					num_children = (set_info->collections[coll_id]).num_children;
					(set_info->collections[coll_id]).scn[num_children] = j;
					(set_info->collections[coll_id]).num_children++; 
					(set_info->collections[coll_id]).scl = (void *)(subsets[i].collections + j);
					
				}
				
				subsets[i].collections[j].parent_setlink = (void *)set_info;
				subsets[i].collections[j].parent_colllink = coll_id;
				
			}
		}

		


	}
	(set_info->collections[coll_id]).child_setlink = (void *)subsets;
	return subsets;
}

/*
get_set_info

a utility function that get parameters for this given SAF_Set
contained in the given Set_Info structure
*/
void get_set_info( Set_Info *set_info)
{
	int tdim;
	int num_colls;
	SAF_TopMode t_mode;
	SAF_ExtendMode e_mode;
	SAF_SilRole srole;


  saf_describe_set(SAF_ALL, &(set_info->the_set), NULL, &tdim, &srole, &e_mode, &t_mode, &num_colls, NULL);
  set_info->silrole = srole;

}


/* 
find_topo_relations_on_set

Given a Set_Info structure, find all topo_relations defined
on the given set
*/
void find_topo_relations_on_set( Set_Info *set_info)
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

/*
A debugging utility that recursively explores the subset relation
tree rooted at the collection defined by the given Set_Info
and Collection_Info structures.
*/
void recurse( Set_Info *the_set, Collection_Info *the_cat )
{

  int i,j;
  Set_Info *subsets;
  int scn;
  subsets = (Set_Info *)the_cat->child_setlink;

  if( subsets == NULL ) {
    return;
  }
  for( i = 0 ; i < the_cat->num_children; i++ ) {
    scn = the_cat->scn[i];
    printf("%s, %s has subset on %s,%s:%d,%d\n",the_set->set_name, the_cat->cat_name, subsets[i].set_name, \
	   subsets[i].collections[scn].cat_name, subsets[i].collections[scn].num_subset_relations,\
	   (int)(subsets[i].collections[scn].subset_relation)  );
    for( j = 0 ; j < subsets[i].num_colls ; j++ ) 
      recurse( subsets + i, subsets[i].collections + j );
  }


}

/* 
read_indirect_field

Given a Field_Info structure that is an "indirect field",
read the field_handles stored on this field, build Field_Info
structures for each stored field_handle, and store a linked list
of the Field_Info structures on the given Field_Info structure.

i.e. created a linked list of FIeld_Info structures that describe
the Fields "pointed to" by this "indirect field"

Note: is the given Field_Info does not contain an "indirect field"
then print a message and return, doing nothing

*/
void read_indirect_field( Field_Info *field_info )
{

  int i;
  void *handles;
  Field_Info **field_infos;


  if(!H5Tequal(field_info->data_type, SAF_HANDLE)) {
    printf("field data_type must be SAF_HANDLE for indirect field read\n");
    return;
  }


  if( field_info->field_data != NULL ) {
    return;
    
  }


  handles = NULL;

  saf_read_field(SAF_ALL,  field_info->the_field, NULL, SAF_WHOLE_FIELD, &handles);

  field_infos = (Field_Info **)calloc(field_info->data_size, sizeof(Field_Info *));

  for( i = 0 ; i < field_info->data_size; i++ ) {

    field_infos[i] = (Field_Info *)calloc(1, sizeof(Field_Info));
    field_infos[i]->the_field = ((SAF_Field *)handles) + i;
    get_field_info( field_infos[i] );

  }
  
  field_info->field_data = (void **)field_infos;

}


int check_indirect_field( Field_Info *field_info )
{

  void *handles;
  static size_t count = 0;
  static hid_t Ptype;


  handles = NULL;

  saf_read_field(SAF_ALL,  field_info->the_field, NULL, SAF_WHOLE_FIELD, &handles);

  saf_get_count_and_type_for_field( SAF_ALL, ((SAF_Field *)handles),NULL, &count, &Ptype);

  if( Ptype == SAF_HANDLE )
    return 1;
  else
    return 0;

}

/*
read_field

Given a Field, read the field data with saf_read_field and 
store the data in the given Field_Info structure.
*/
void read_field( Field_Info *field_info )
{

  void **handles;
  SAF_FieldTarget *ftarg=calloc(1, sizeof *ftarg); /*because address is saved for later*/

  if( field_info->data_size <= 0 )
    return;

  if( !strcmp("field", field_info->template_info->algtype) && \
      field_info->storage_decomp != NULL) {

    printf("\n*****skipping field remapping;  RMAP\n");
    /* if( ! check_indirect_field( field_info ) ) { */
    if( 1 ) {


      printf("RMAP");
/* 
First parameter has to be SAF_FieldTarget instead of SAF_Field
*/
      field_info->ftarget = ftarg;

      saf_target_field(ftarg, NULL, SAF_SELF(db),
		     SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, H5T_NATIVE_DOUBLE,
		     SAF_INTERLEAVE_COMPONENT, NULL);

      get_field_info( field_info );
    }
  }


   
  handles = (void **)calloc(1, sizeof(void *)); 
  *handles = NULL;
  saf_read_field(SAF_ALL, field_info->the_field, ftarg, SAF_WHOLE_FIELD, handles);
  field_info->field_data = handles;
  return;

}


/*
read_state_field

Given a Field_Info that contains a SAF "state" field, 
read the state field and obtain the handles for all the
fields stored on this state.  Call get_field_info for 
each of the fields and initialize a linked list of 
Field_Info structures which represents the fields stored
on this state.

Save this initialized linked list of Field_Info structures
on the Field_Info structure for this field
*/
void read_state_field( Field_Info *field_info )
{

  int i, j, sn, num_states;
  int num_fields_in_state;
  int index[1];
  void *handles;
  Field_Info **field_infos;

  if( !H5Tequal(field_info->data_type, SAF_HANDLE)) {
    printf("state data_type must be SAF_HANDLE for indirect field read\n");
    return;
  }

  if( field_info->field_data != NULL ) {
    return;    
  }

  num_fields_in_state = field_info->template_info->num_comps;
  num_states = field_info->data_size / num_fields_in_state;

  field_infos = (Field_Info **)calloc( (num_states * num_fields_in_state), sizeof(Field_Info *));
  

  i = 0;
  for( sn = 0; sn < num_states; sn++ ) {

    handles = NULL;
    index[0] = sn;


    saf_read_field(SAF_ALL, field_info->the_field, NULL, 1, SAF_TUPLES, index, &handles);

    for( j = 0 ; j < num_fields_in_state; j++ ) {
        field_infos[i] = (Field_Info *)calloc(1, sizeof(Field_Info));
      field_infos[i]->the_field = ((SAF_Field *)handles) + j;
      get_field_info( field_infos[i] );
      i++;
    }
  }

  field_info->field_data = (void **)field_infos;
}


