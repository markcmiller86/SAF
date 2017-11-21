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
#include <../test/testutil.h>


typedef struct {
  SAF_Cat the_cat;
  char *cat_name;
  int count;
  SAF_CellType celltype;
  char *ispec;
  unsigned char is_decomp;
  void *child_setlink;
  void *parent_setlink;
  void *topo_link;
  void *topo_domain_id;
  int parent_colllink;
  int num_subset_relations;
  void *scl;
  int *scn;
  int num_children;
  void *relation_data;
  SAF_Rel *subset_relation;
} Collection_Info;	

typedef Collection_Info *Collections;

typedef struct {

  SAF_FieldTmpl *fieldtmpl;
  char *fieldtmpl_name;
  int num_comps;
  /* void *base_space; */  /* a pointer to the Set_Info for the base space */
  /* char *base_space_name; */
  char *algtype;
  char *basis;
  SAF_Quantity *quant;
  void *component_tmpls; /* FieldTmpl_Info * component_tmpls */
} FieldTmpl_Info;	


typedef struct {
  SAF_Field *the_field;
  SAF_FieldTarget *ftarget;
  FieldTmpl_Info *template_info;
  char *field_name;
  void *base_space;   /* a pointer to the Set_Info for the base space */
  char *base_space_name; 
  unsigned char is_coord;
  int my_set_coll_num;
  SAF_Cat *eval_coll;
  SAF_Cat *coeff_assoc;
  int assoc_ratio;
  SAF_Cat *storage_decomp;
  char *eval_func;
  hid_t data_type;
  int data_size;
  char interleave;
  int num_comps;
  int comp_order;
  void **field_data; /*only used if this is an indirect field and we put the handles here */
  void *component_fields; /* Field_Info * component_fields */
} Field_Info;


typedef Field_Info *Fields;

typedef struct {
  SAF_Set the_set;
  char *set_name; 
  SAF_SilRole silrole;
  SAF_ExtendMode extmode;
  int max_topo_dim;
  int num_sets;
  int my_num_in_set_list;
  int num_colls;
  int num_fields;
  Collections collections;
  int num_topo_relations;
  SAF_Rel *topo_relation;
  Fields fields;
} Set_Info;

typedef Set_Info *Sets, *Set;

typedef struct {

  SAF_Field *saf_fields;
  Field_Info *the_fields;
  int num_fields;
  SAF_Set the_set;
} Field_List;



int not_in_name( char *, char **, int );
void find_collections( Set_Info *, int );
Set_Info * get_subsets( Set_Info *, int );
void find_fields_on_set( Set_Info *, int);
void find_topo_relations_on_set(Set_Info *);
void get_fieldtmpl_info( FieldTmpl_Info * );
void get_field_info( Field_Info * );
void get_set_info( Set_Info *);
char * saf_set_name( SAF_Set * );
char *algtype_string( SAF_Algebraic * );
char * basis_string( SAF_Basis * ); 
void recurse( Set_Info *, Collection_Info * );
void read_indirect_field( Field_Info *);
int check_indirect_field( Field_Info *);
void read_field( Field_Info *);
void read_state_field( Field_Info *);
const char *saftype_string( hid_t );


#ifdef READER_DECLARE
#  define R_EXTERN 
#else
#  define R_EXTERN extern
#endif

R_EXTERN SAF_Cat saf_self;
R_EXTERN SAF_Db *db;
R_EXTERN int num_all_sets;
R_EXTERN Set_Info *allsets;

R_EXTERN SAF_Set *top_sets, *all_sets, *suite_sets, *time_sets, *param_sets;

R_EXTERN SAF_Rel *topo_rels;
R_EXTERN int num_top_sets, num_suites, num_topos;

R_EXTERN int tdim;
R_EXTERN SAF_TopMode t_mode;
R_EXTERN SAF_ExtendMode e_mode;
R_EXTERN SAF_SilRole srole;
R_EXTERN char *set_name;


R_EXTERN Field_List set_fields;
R_EXTERN Sets *tops, *suites;
R_EXTERN int *top_nums, *suite_nums;

R_EXTERN Set_Info *tmp_set;
R_EXTERN Collection_Info *tmp_coll;
R_EXTERN Field_Info *tmp_field;

R_EXTERN int tmp_coll_num;


#undef R_EXTERN
