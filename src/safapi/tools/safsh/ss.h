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
#include "Python.h"
#include "Numeric/arrayobject.h"

typedef struct Collection_Info Collection_Info;
typedef struct Set_Info Set_Info;
typedef struct FieldTmpl_Info FieldTmpl_Info;
typedef struct Field_Info Field_Info;
typedef struct DB_Info DB_Info;
typedef Set_Info *Set;
typedef Collection_Info *Collections;
typedef Field_Info *Fields;
typedef FieldTmpl_Info *FieldTmpls;

typedef struct FieldTableOfValues_type{
       SAF_Field key;
       Fields value;
} FieldTableOfValues_t;

typedef struct FieldTmplTableOfValues_type{
       SAF_FieldTmpl key;
       FieldTmpls value;
} FieldtmplTableOfValues_t;


struct Collection_Info {
	SAF_Cat the_cat;
	SAF_Set the_set;
	char *cat_name;
  int count;
	Set_Info *child_setlink;   
	Set_Info *parent_setlink;
	void *topo_link;
	void *topo_domain_id;
	int parent_colllink;
	int num_subset_relations;
	SAF_Rel *subset_relation;
};	

struct FieldTmpl_Info {

	SAF_FieldTmpl *fieldtmpl;
	char *fieldtmpl_name;
	int num_comps;
  /* SAF_Set *base_space; */
  /* char *base_space_name; */
	char *algtype;
	char *basis;
	SAF_Quantity *quant;
	FieldTmpl_Info *component_tmpls;
  FieldTmpl_Info *next;
};	



struct Field_Info {
	SAF_Field *the_field;
        SAF_FieldTarget *ftarget;
	FieldTmpl_Info *template_info;
	char *field_name;
  Set_Info *base_space;
  char *base_space_name;
	unsigned char is_coord;
	int my_set_coll_num;
	SAF_Cat *eval_coll;
	SAF_Cat *coeff_assoc;
	int assoc_ratio;
	SAF_Cat *storage_decomp;
	SAF_Eval *eval_func;
	char *data_type;
        int data_size;
	char *interleave;
	int num_comps;
	int comp_order;
        void **field_data; /*only used if this is an indirect field and we put the handles here */
        Field_Info *component_fields; /* Field_Info * component_fields */
};

	
struct Set_Info {
	SAF_Set the_set;
	char *set_name; 
        SAF_SilRole silrole;
        int row_id;
  Set_Info *duplicate_set; /* (Set_Info *) */
	int num_sets;
	int num_colls;
	int num_fields;
	Collections collections;
	int num_topo_relations;
	SAF_Rel *topo_relation;
	Fields fields;
};


struct DB_Info {
	SAF_Db *the_db;
	char *db_name;
  SAF_Cat *saf_self;
	Set tops;
        int num_top_sets;
        Set allsets;
        int num_all_sets;
        Set suitesets;
        int num_suite_sets;
  FieldTmpl_Info *allfieldtmpls;
  FieldtmplTableOfValues_t *fieldtmpl_lookup;
  Field_Info *allfields;
  FieldTableOfValues_t *field_lookup;
  int num_all_fields;
  int num_field_tmpls;
  Set suitesubsets;
  int number_suite_subsets;
};	



typedef struct {
        PyObject_HEAD
        /* SAF_Db *dbhandle; */
	DB_Info *dbhandle;
	int num_top_sets;
	Set_Info *tops;
        Set_Info *suites;
        int num_suites;
} dbobject;



