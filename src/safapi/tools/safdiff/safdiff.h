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
#ifndef SAFDIFF_H
/*DOCUMENTED*/
#define SAFDIFF_H


/*#include <saftest.h>   */
#include <safP.h>
#include <math.h>
#include <ctype.h>

#ifndef WIN32
#if 0
#include <stropts.h>
#include <termio.h>
#endif
#include <termios.h>
#endif

#include <sys/types.h>  /*to stat a file*/
#include <sys/stat.h>	/*to stat a file*/
#include <string.h>
#include <stdarg.h>
#include <newsuite.h>
#include <names.h>
/*#include <hash.h>*/
#include <bitvector.h>



typedef enum {
   PRINT_LEVEL_neg = -1,
   PRINT_LEVEL_0 = 0,
   PRINT_LEVEL_1 = 1,
   PRINT_LEVEL_2 = 2,
   PRINT_LEVEL_3 = 3,
   PRINT_LEVEL_4 = 4,
   PRINT_LEVEL_5 = 5
} t_safdiffPrintLevel;


typedef struct FiedTableOfValues_type{
       SAF_Field key;
       int value;
} FieldTableOfValues_t;

/*the structure for what to store in the 3rd database that contains the differences*/
typedef struct store_type{
	int diffs;		/*store the difference, Right-Left*/
	int rel_diff;		/*store the relative difference, 100*(Right-Left)/Left */
	int map;		/*store a 0/1 map, 0 means inside the threshold, 1 means outside the threshold*/
	int left;		/*copy left field to new db*/
	int right;		/*copy right field to new db*/
	int ave;		/*store ave of left and right fields in new db*/
	int exclusive;		/*store exclusive objects*/
	int anything;		/*are we storing anything? ie. diff or rel_diff or map or exclusive*/

} store_t;


typedef struct fieldOptions_type{
  char fieldname[128];            /*the name of the field*/
  double rel_tol;		  /*the relative tolerance*/
  double abs_tol;		  /*the absolute tolerance*/
  double floor_tol;               /*the noise floor tolerance*/
  int absrel_andor;		/*0 --> or, 1-->and.  Specify when a difference exists, 
				  1:  when both the abs_tol *and* the rel_tol are exceeded -OR-
				  0:  when either the abs_tol *or* the rel_tol are exceeded */
  int obase;			/*the output base, one of 2,8,10 or 16*/

  t_safdiffPrintLevel verbosity;          /*store information about the verbosity of output for this field*/
  int exclusive;


  store_t store;                  /*what to write to the output database*/
} fieldOptions;

/*function prototypes*/

/*the following functions contain for loops [i.e. for(i=0; i<MIN(left_count, right_count); i++) ],
these need to be replaced with more general list comparison functions
that try to find matching items to compare, and deal with exclusive objects


compare_cat_lists_and_find_subsets

compare_attributes
compare_topo_relations
                    
*/
                    
/*composed of other saf objects (hence make further calls to saf_compare_xxx)*/

int compare_attributes_new_handles(SAF_Db *left, SAF_Db *right);
int compare_attributes_old_handles(ss_pers_t *left, ss_pers_t *right, int a_leftMissing, int a_rightMissing);


int compare_sets(SAF_Set *left, SAF_Set *right);                          /*compare_attributes*/
int compare_cats(SAF_Cat *left_cat, SAF_Cat *right_cat);                  /*compare_attributes*/
int compare_fields(SAF_Field *leftfield, SAF_Field *rightfield);          
int compare_field_data(SAF_Field *leftfield, SAF_Field *rightfield);	/*compare_attributes*/
int compare_field_templates(SAF_FieldTmpl *left, SAF_FieldTmpl *right);   /*compare_attributes*/

int compare_state_tmpls(SAF_StateTmpl *left, SAF_StateTmpl *right);	/*compare_attributes*/
int compare_suites(SAF_Suite *left, SAF_Suite *right);                    /*compare_attributes*/
int compare_topo_relations(SAF_Set *left, SAF_Set *right, SAF_Cat *leftcat, SAF_Cat *rightcat);
int compare_units(SAF_Unit *left_unit, SAF_Unit *right_unit, int a_leftMissing, int a_rightMissing);
        
/*composed of primitive types*/

int compare_roles(SAF_Role *left, SAF_Role *right, int a_leftMissing, int a_rightMissing);
/* int compare_eval(SAF_Eval *lefteval_func, SAF_Eval *righteval_func, int a_leftMissing, int a_rightMissing); */
int compare_quantity(SAF_Quantity *left_quan, SAF_Quantity *right_quan, int a_leftMissing, int a_rightMissing);
int compare_relreps(SAF_RelRep *left, SAF_RelRep *right, int a_leftMissing, int a_rightMissing);
int compare_basis(SAF_Basis *left, SAF_Basis *right, int a_leftMissing, int a_rightMissing);
int compare_algebraic(SAF_Algebraic *left, SAF_Algebraic *right, int a_leftMissing, int a_rightMissing);
int compare_hid_ts(hid_t lefttype, hid_t righttype, int a_leftMissing, int a_rightMissing);
int compare_interleave(SAF_Interleave *leftcomp_intlv, SAF_Interleave *rightcomp_intlv, int a_leftMissing, int a_rightMissing);

/*compare lists of saf objects*/
int compare_set_lists(SAF_Set *left, SAF_Set *right, size_t leftnum, size_t rightnum);
int compare_cat_lists(SAF_Cat *left, SAF_Cat *right, size_t leftnum, size_t rightnum);
int compare_field_lists(SAF_Field *left, SAF_Field *right, size_t leftnum, size_t rightnum);
int compare_field_tmpl_lists(SAF_FieldTmpl *left, SAF_FieldTmpl *right, size_t leftnum, size_t rightnum);

/* int compare_state_tmpl_lists(SAF_StateTmpl left, SAF_StateTmpl right, size_t leftnum, size_t rightnum); */
int compare_suite_lists(SAF_Suite *left, SAF_Suite *right, size_t leftnum, size_t rightnum);

int compare_cat_lists_and_find_subsets(SAF_Cat *left, SAF_Cat *right, size_t leftnum, size_t rightnum,
        SAF_Set *leftset, SAF_Set *rightset);



/*printing routines*/
char *binary(int n); 

#ifndef WIN32
char *binaryll(long long ll);
#endif

char *binaryf(float f);
char *binaryd(double d);
int different(char *message);
void print_error(const char *message);

void print_string( t_safdiffPrintLevel a_printLevel,const char *label, const char *left, const char *right);
void print_string_ignore( t_safdiffPrintLevel a_printLevel,const char *label, const char *left, const char *right);



void print_int( t_safdiffPrintLevel a_printLevel, const char *label, int a_left, int a_right, 
		   int a_leftIsMissing, int a_rightIsMissing );


void print_long( t_safdiffPrintLevel a_printLevel, const char *label, long int a_left, long int a_right, 
		   int a_leftIsMissing, int a_rightIsMissing );


void print_unsigned_int( t_safdiffPrintLevel a_printLevel, const char *label, unsigned int a_left, unsigned int a_right, 
		   int a_leftIsMissing, int a_rightIsMissing ); 




void print_float( t_safdiffPrintLevel a_printLevel, const char *label, double a_left, double a_right, 
		  int a_leftIsMissing, int a_rightIsMissing );



void print_string_array( t_safdiffPrintLevel a_printLevel,
			 const char *name, char **left, char **right, size_t left_count, size_t right_count);


void print_int_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
			const char *name, int *left, int *right, size_t left_count, size_t right_count);
void print_long_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
			const char *name, long *left, long *right, size_t left_count, size_t right_count);
void print_float_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
			const char *name, float *left_buf, float *right_buf, size_t left_count, size_t right_count);
void print_double_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
			const char *name, double *left, double *right, size_t left_count, size_t right_count);



void print_type_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
		       const char *name, void *leftcoords_void, void *rightcoords_void, size_t leftdim, size_t rightdim,
		       hid_t lefttype, hid_t righttype);



void build_formatStrings(void);
void print_heading( t_safdiffPrintLevel a_printLevel,const char *leftname, const char *rightname);
void print_break( t_safdiffPrintLevel a_printLevel,const char *message);
void print_globals(void);




/*miscellaneous utilities*/
void readConfigurationFile(void);
void processCommandArguments(int argc, char **argv, fieldOptions *field_options, int second_time);
void print_usageMessage(void);
void print_help(const char *switches, const char *description);
void initializeMissingObjects(void);
void out_init_size(void);   

/*debugging*/
void testBinaryFloats(void);



/*not categorized yet*/


int saf_translate_basehandle(ss_pers_t *oldbasehandle, ss_pers_t *newbasehandle, SAF_Db *newdatabase);

int saf_translate_field(SAF_ParMode pmode, SAF_Field *oldfield, SAF_Field *newfield, SAF_Db *newdatabase);
int saf_translate_field_tmpl(SAF_ParMode pmode, SAF_FieldTmpl *oldfieldtmpl, SAF_FieldTmpl *newfieldtmpl, SAF_Db *newdatabase);
int saf_copy_suite(     SAF_ParMode pmode, SAF_Suite *oldsuite, SAF_Suite *newsuite, SAF_Db *newdatabase);
int saf_copy_category(                     SAF_Cat *oldcat,     SAF_Cat *newcat,     SAF_Db *newdatabase);
int saf_copy_set(       SAF_ParMode pmode, SAF_Set *oldset,     SAF_Set *newset,     SAF_Db *newdatabase);
int saf_copy_collection(SAF_ParMode pmode, SAF_Set *oldset,     SAF_Cat *oldcat, SAF_Set *newset, SAF_Cat *newcat);
int safdiff_compare_sets(SAF_Set *left, SAF_Set *right, SAF_Db *newdatabase);

int canonicalize_set_lists(SAF_Set *left, SAF_Set *right, int leftnum, int rightnum);
int canonicalize_cat_lists(SAF_Cat *left, SAF_Cat *right, size_t leftnum, size_t rightnum);
int canonicalize_field_lists(SAF_Field *left, SAF_Field *right, size_t leftnum, size_t rightnum);
int canonicalize_field_tmpl_lists(SAF_FieldTmpl *left, SAF_FieldTmpl *right, size_t leftnum, size_t rightnum);
int canonicalize_suite_lists(SAF_Suite *left, SAF_Suite *right, size_t leftnum, size_t rightnum);

int canonicalize_state_tmpl_lists(SAF_StateTmpl *left, SAF_StateTmpl *right, size_t leftnum, size_t rightnum);

int Ftabcompare(const void  *i, const void *j);
int *Ftab_find(FieldTableOfValues_t *table, SAF_Field *rlfield, int n);

int get_timestep_per_field(SAF_Db *a_db, int a_numTimesteps, double *a_timeValues, FieldTableOfValues_t **a_timestepPerField,int *nfields);
void initializeBitVectorsForNewDatabase(void);
int build_fields(SAF_Field *leftfield, SAF_Field *rightfield, SAF_Field *newfield);
int build_field_tmpl(SAF_Field *leftfield, SAF_FieldTmpl *leftfield_tmpl, SAF_FieldTmpl *newfield_tmpl);

int get_number_of_timesteps(SAF_Db *a_db, double **a_timeValuesPerTimeStep);



void print_break_no_buffer( t_safdiffPrintLevel a_printLevel,const char *message);

int compare_collections(SAF_Set *a_left_set, SAF_Cat *a_left_cat, SAF_Set *a_right_set, SAF_Cat *a_right_cat);

const char *get_celltype_desc( SAF_CellType a_type );

int compare_coll_lists(SAF_Set *a_leftset, SAF_Set *a_rightset, size_t a_leftnum, size_t a_rightnum,
		       SAF_Cat *a_leftcats, SAF_Cat *a_rightcats );
#endif /* !SAFDIFF_H */
