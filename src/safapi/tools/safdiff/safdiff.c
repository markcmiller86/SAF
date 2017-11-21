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
#include <safdiff.h>

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	Global Variables
 *
 * Description: These are global variables.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/

t_safdiffPrintLevel g_printLevel=PRINT_LEVEL_1;
int g_expectFileDifferences=0;
char g_missingdbname[1024]="";		/*missing db file name*/
char g_print_some_strings_buffer[1024]="";
int g_rank=0;                             /*processor rank for MPI*/
double g_time_value_abs_tol = 0.0;			/*the coordinate tolerance*/
int g_lefttimeindex = -1;		 /*if g_lefttimeindex >= 0 then only diff that time index*/
int g_righttimeindex = -1;		/*if g_righttimeindex >= 0 then only diff that time index*/
double g_lefttimevalue = 0;
double g_righttimevalue = 0;
int g_gotLeftTimeValue=0;
int g_gotRightTimeValue=0;
int g_strictorder=0;			/*1 force strict order, 0 allow any order*/
int g_columnwidth=0;			/*the number of characters in each column*/
int g_namewidth=32, g_valwidth=32, g_diffwidth=21, g_perwidth=19, g_precision=-1;
int g_totalwidth=80;				/*the total width of the output*/
char g_configfile[1024];			/*the name of the configuration file*/
int g_matched_arg;			/*a flag used to catch command line arguments that don't get matched*/
char g_coord_field_string[1024];		/*ave, left or right*/
SAF_Db *g_newdb, *g_leftdb, *g_rightdb;		/*the left database, right database, and the new database, which contains the diffs*/
SAF_Db *g_missingdb;			/*to store all of the "missing" objects*/
char g_leftdbname[1024] = "\0", g_rightdbname[1024] = "\0", g_newdbname[1024]="diffs.saf"; /*the file names of the databases*/

int g_overwritenewdb=0;			/*1: overwrite the newdb if it already exists. 0: increment the suffix of g_newdbname
					  and do not overwrite g_newdb, write it to the incremented file name*/
int g_debugging=0;			/*debugging level*/
int g_drawtree=0;				/*1 draw tree, 0 don't draw tree*/
int g_terminal_width=80;			/*the width of the output terminal*/
int g_terminal_height=-1;			/*the height of the output terminal*/

/*I dont think this is a good idea. Commented it out.
#define USE_BANNER
*/
#ifdef USE_BANNER
char g_banner[1024]="";			/*the g_banner that is repeated every screen full of data so you know what the data is 
					  that you are looking at*/ 
#endif


size_t g_printed_lines=0;			/*the number of printed lines, so the g_banner can be printed when we've got a screenful,
						  i.e. g_printed_lines % g_terminal_height == 0 */
char g_leftobjname[1024]="";			/*specify a single object to consider in the left database*/
char g_rightobjname[1024]="";			/*specify a single object to consider in the right database*/

#define MAX_CONTEXT_BUFFER_LENGTH 256
int g_show_context=1;			/*0/1 flag on whether or not to show context (the output queue) (THE BANNERS & BREAKS?)*/
int g_context_length=0;			/*the length of the output queue for context*/
char g_context_buffer[MAX_CONTEXT_BUFFER_LENGTH][256];		/*The buffer that holds the lines of context*/


/*These two vars are part of a hack to show a bit more than just
  the line with the error on it. When an error is printed, the
  heading preceding the error will be printed (0th string in the
  array) and the first string (e.g. 'set name') after the error
  is printed (1th string in the array). Note: if they have already
  been printed, because the verbosity level is set high enough,
  then they will not be printed again.*/
char g_hidden_context_buffer[2][1024]={ "", "" };
int g_just_reset_hidden_context_buffer=0;


int g_exit_value=0;
int g_sets_first=0;	/*no more option here: hardwired to suites first because of new states&suites*/


SAF_HashTable *g_field_tmpls_written; /*a bitvector that knows which field templates have already been written to the new database
				   the key into the bit vector is the row id of the field template*/
SAF_HashTable *g_fields_written;	  /*a bitvector that knows which fields have already been written to the new database*/
SAF_HashTable *g_leftfields_seen;	  /*a bitvector that stores which left fields we have already diffed (i.e. called compare_field_data on)*/
SAF_HashTable *g_rightfields_seen;	  /*a bitvector that stores which right fields we have already diffed (i.e. called compare_field_data on)*/

FieldTableOfValues_t *g_leftWhichTimestepPerFieldRow=NULL;
int g_nleft=0;
int g_nright=0;
FieldTableOfValues_t *g_rightWhichTimestepPerFieldRow=NULL;	/*an array such that g_leftWhichTimestepPerFieldRow[key]== 
					  the state index that the field is in, or -1 if
					  the field is not in any state*/


SAF_HashTable *g_seen_sets;                    /*A hash table to keep track of which sets we've already seen*/
SAF_HashTable *g_fieldOptionsHash;

fieldOptions g_globalfo, g_save_globalfo;

int g_leftNumTimesteps=0;
double *g_leftTimeValues=0;
int g_rightNumTimesteps=0;
double *g_rightTimeValues=0;

#define FORMAT_STRING_LENGTH 128	/*the max length we allocate for the format strings*/

typedef struct formatString_type{
	char m_floatf[FORMAT_STRING_LENGTH];		/*the format string to printf floats*/
	char m_float_leftmissing[FORMAT_STRING_LENGTH];	/*printf a float, left missing*/
	char m_float_rightmissing[FORMAT_STRING_LENGTH];/*printf a float, right missing*/

	char m_expf[FORMAT_STRING_LENGTH];		/*the format string to printf exps*/
	char m_exp_leftmissing[FORMAT_STRING_LENGTH];	/*printf a exp, left missing*/
	char m_exp_rightmissing[FORMAT_STRING_LENGTH];  /*printf a exp, right missing*/

	char m_intf[FORMAT_STRING_LENGTH];		/*the format string to printf ints*/
	char m_int_leftmissing[FORMAT_STRING_LENGTH];	/*printf an int, left missing*/
	char m_int_rightmissing[FORMAT_STRING_LENGTH];	/*printf an int, right missing*/

	char m_string[FORMAT_STRING_LENGTH];		/*printf strings*/
	char m_heading[FORMAT_STRING_LENGTH];		/*printf the heading*/
} formatString_t;

/*global vairable to hold all of the format strings for ASCII output*/
formatString_t g_formatString;

/*so we can convert a float or double to binary*/
typedef union number_type{
	long 		longt;
	int  		intt[2];

#ifndef WIN32
	long long 	longlongt;
#endif

	float 		floatt;
	double 		doublet;
} number_t;


SAF_Set         g_missing_set;
SAF_Cat         g_missing_cat;
SAF_Field       g_missing_field;
SAF_FieldTmpl   g_missing_field_tmpl;
SAF_StateTmpl   g_missing_state_tmpl;
SAF_StateGrp    g_missing_state_group;
SAF_Suite       g_missing_suite;

#define compare_sets_missing 		g_missing_set
#define	compare_cats_missing 		g_missing_cat
#define compare_field_data_missing 	g_missing_field
#define compare_fields_missing 		g_missing_field
#define compare_field_templates_missing	g_missing_field_tmpl
#define compare_state_tmpls_missing	g_missing_state_tmpl
#define compare_suites_missing		g_missing_suite

SAF_Role 	g_missing_role;
SAF_Quantity	*g_missing_quantity;
SAF_Unit	g_missing_unit;
SAF_Algebraic	g_missing_algebraic;
SAF_Basis	g_missing_basis;
SAF_Eval	g_missing_eval;



int g_indent=0;		/*global variable to keep track of how many function calls deep the call stack is, used for
			  printing the tree*/
int g_level[128];		/*whether or not where should draw tree branch at g_level[g_indent]*/
int g_last[128];		/*g_last[i] == where or not we have reached the last branch at g_level[i]*/

char g_value_string[1024];	/*global variable that is set to the value of a comman line argument, i.e. --arg=value or -a value*/



#define DUMMY_FLOAT 5.859874 	/*pi + e, a place marker for a float that doesnt matter? Used to be a "missing" float*/
#define DUMMY_INT 5859874 	/* 1 million *(pi + e), see above*/
#define DUMMY_STRING ""       /*print nothing if the string is missing*/

/*free each element of an array of pointers*/
#define FREE_ARRAY(p,count)  {int looper; for(looper=0; looper<count; looper++){ free((p)[looper]);}} 
#define SWAP(a,b,type) {type t; t=a; a=b; b=t;} /*swap elements of a certain type*/


#if 1
  /*the percent change from left to right.  if left is 0, then its 100 times the differences (the old way)*/
  #define PER_CHANGE(left, right) ( (left) ? (left)==(right) ? 0.0 : (100.0*((right)-(left)))/((left)*1.0) : 100.0*((right)-(left)) )
#else
  /*the percent change from left to right, if one value is zero then the difference is either 0% or 100%*/
  #define PER_CHANGE(left, right)    \
      (!left&&right)||(left&&!right) ? 100 : ( (left)==(right) ? 0.0 : (100.0*((right)-(left)))/((left)*1.0) )
#endif


/*a convience macro for saf_target_field, with the unused arguments filled in*/
#define  saf_target_field(field, hid_t) 								\
            saf_target_field(field, SAF_NOT_SET_UNIT,SAF_NOT_SET_CAT,SAF_NOT_SET_CAT,SAF_ANY_RATIO,	\
		SAF_NOT_SET_CAT,SAF_ANY_EFUNC,hid_t,SAF_INTERLEAVE_NONE,NULL)			

/*returns x and y if a is true, returns x or y if a is false, this is used when deciding when a difference exists,
  whether both the relative and absolute tolerances are exceeded, or 
  whether either of the relative or the absolute tolerances are exceeded.*/
#define and_OR_or(a,x,y) (    (a) ? (x) && (y) : (x) || (y)     )

/*given all these parameters, set outside_threshold to be whether or not the parameters are outside the threshold*/
#define OUTSIDE_THRESHOLD(absrel_andor, abs_tol, rel_tol, floor_tol, per_change, left, right, outside_threshold)           \
  {                                                                                                            \
    outside_threshold = and_OR_or(absrel_andor, fabs(per_change) > rel_tol , fabs((double)(right-left)) > abs_tol);      \
    if(  (fabs((double)left) < (floor_tol)) && (fabs((double)right) < (floor_tol)) ) outside_threshold=0;                                \
  }

/*safdiff has 3 possible return values:
  0:  everything was the same, within the given thresholds
  1:  encountered a difference, outside of the given thresholds
  2:  encountered an error
*/
#define SAFDIFF_ERROR     -1  /*return this as the exit value of safdiff upon encountering an error*/
#define SAFDIFF_SAME      0   /*return this as the exit value of safdiff if everything was the same*/
#define SAFDIFF_DIFFERENT 1   /*return this as the exit value of safdiff upon encountering a difference*/


/*note replacing strcpy with equivalent strncpy only to avoid __strcpy_small warnings on linux*/
#define strcpy(TOSTR,FROMSTR) strncpy(TOSTR,FROMSTR,strlen(FROMSTR)+1)



/*
** Tried USE_EXTRA_SORT 021803. Sorts the fields and templates by other parameters,
** if their names are equal. Doesnt seem to be necessary anymore, after qsort
** was replaced with safdiff_sort

#define USE_EXTRA_SORT
*/ 


int Ftabcompare(const void  *i, const void *j)
{
  int value;
  FieldTableOfValues_t *left;
  FieldTableOfValues_t *right;
  ss_fieldobj_t mask;

  left = (FieldTableOfValues_t *)i;
  right = (FieldTableOfValues_t *)j;

  /* ss_set_t s1, s2; */
  memset(&mask, SS_VAL_CMP_DFLT, sizeof mask);

  value = ss_pers_cmp((ss_pers_t*)&(left->key), (ss_pers_t*)&(right->key), (ss_persobj_t*)&mask); 

  return(value);
}

int *Ftab_find(FieldTableOfValues_t *table, SAF_Field *rlfield, int n)
{
  FieldTableOfValues_t *ret_field;
  ret_field = (FieldTableOfValues_t *)bsearch((void *)rlfield, (const void *)table, (size_t) n, sizeof (FieldTableOfValues_t),Ftabcompare);
  if (ret_field)
     return (&(ret_field->value));
  else
     return (NULL);

}



/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	Prototypes
 *
 * Description: These are prototypes.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
char *safdiff_strdup(const char *a_str);
#ifdef SSLIB_SUPPORT_PENDING
#ifdef HAVE_PARALLEL
  int printf(const char *format, ...);
#endif
#endif
int get_closest_index( double a_value, int a_numEntries, double *a_entries );
void exit_safdiff( int a_val );
void safdiff_sort(ss_pers_t *base, size_t nmemb, size_t size,
              int (*compar)(ss_pers_t *, ss_pers_t *));
int sort_compare_field_tmpls(ss_pers_t *a_left, ss_pers_t *a_right);
int sort_compare_fields(ss_pers_t *a_left, ss_pers_t *a_right);
int is_number_outside_precision( double a_num, int a_width, int a_prec, double *a_uppermult, double *a_lowermult );
int is_number_outside_value_precision( double a_num );
void print_difference( double a_val );
void print_percentage( double a_val );
int print_version_info(const char *a_leftname, const char *a_rightname);
void print_int_internal( t_safdiffPrintLevel a_printLevel, const char *label/*used by macros*/, int a_left, int a_right, 
			 int a_leftIsMissing, int a_rightIsMissing,
			 int a_alwaysPass,
			 int a_absrel_andor, double a_abs_tol, double a_rel_tol, double a_floor_tol );
void print_float_internal( t_safdiffPrintLevel a_printLevel, const char *label/*used by macros*/, double a_left, double a_right, 
			 int a_leftIsMissing, int a_rightIsMissing,
			 int a_alwaysPass,
			 int a_absrel_andor, double a_abs_tol, double a_rel_tol, double a_floor_tol );
/*  void testBitVector(void); */
void print_string_temp( t_safdiffPrintLevel a_printLevel,const char *label, const char *left, const char *right,
			int a_dummyLeftIsMissing, int a_dummyRightIsMissing);
int arePermutations(int *a, int *b, size_t count);
void test_arePermutations(void);
void parseFieldTolerance(int second_time, char *value_string);
int white_space(char *line);
void compare_time_values( void );
int find_objs_and_compare(char *leftobjname, char *rightobjname);
void compare_individual_attributes_new_handles(SAF_Db *left, SAF_Db *right,
					       const char *a_leftName, const char *a_rightName);
void compare_individual_attributes_old_handles(ss_pers_t *left, ss_pers_t *right,
					       const char *a_leftName, const char *a_rightName);
void find_non_suite_top_sets(SAF_ParMode a_pmode, SAF_Db *a_db, int *a_numSets, SAF_Set **a_sets );



/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: Added this temporary replacement for strdup 02/26/03 because strdup doesnt 
 * seem to work on sass3276
 *
 *----------------------------------------------------------------------------------------------------------------*/
char *safdiff_strdup(const char *a_str)
{
        int l_len;
        char *l_ptr=0;
        l_len = strlen(a_str);
        /*if(l_len<=0) return(0);*//*if length 0, then allocate a length 1 string, containing only '\0'*/
        l_ptr = (char *)malloc( (l_len+1)*sizeof(char) );/*+1 for the ending '\0'*/
        memcpy(l_ptr,a_str,l_len*sizeof(char));
        l_ptr[l_len]='\0';/*ok because of the +1 above*/
 
        return(l_ptr);
}
 
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	Exit
 *
 * Description: Handles exiting safdiff. Calls saf_final, cleans up temporary files, and calls
 * MPI_Finalize if needed.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void exit_safdiff( int a_val )
{
  int l_returnVal = a_val;

  /*this is a trick to get safdiff to return a good value (0) when
    it finds differences and differences were expected*/
  if(g_expectFileDifferences )
    {
      printf("Note: by request, safdiff is returning an inverted return value!\n");
      if(a_val==SAFDIFF_DIFFERENT) l_returnVal=SAFDIFF_SAME;
      if(a_val==SAFDIFF_SAME) l_returnVal=SAFDIFF_DIFFERENT;
    }

  saf_final();

  /*
  ** Try to remove the missing file, dont leave garbage behind.
  ** Note that I moved this to after saf_final, because on windows we
  ** get an error "The process cannot access the file because it
  ** is being used by another process."
  */
  if(strlen(g_missingdbname))
    {
      char l_str[256];

#ifndef JANUS
#ifndef WIN32
      sprintf(l_str,"rm -f %s\n",g_missingdbname);/*we know we created it, so we can remove it*/
#else
      sprintf(l_str,"del /f %s\n",g_missingdbname);/*we know we created it, so we can remove it*/
#endif

      system(l_str);
#endif
    }


#ifdef HAVE_PARALLEL
  /* make sure everyone returns the same error status */
  MPI_Bcast(&l_returnVal, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Finalize();
#endif


  exit(l_returnVal);
}



/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: Sorts a list of objects using a given compare function. We cannot use
 * qsort for this because, for qsort, "If two members compare as equal, their order in the sorted 
 * array is undefined." We need predictability in the ordering scheme, so that we can compare safdiff
 * output from different platforms.
 *
 * Issue: This is currently just a simple bubble sort, should make it faster.
 *        THIS IS NOT MAKING IT INTO THE DOCUMENTATION????????????????????????????????????
 *----------------------------------------------------------------------------------------------------------------*/
void safdiff_sort( ss_pers_t *base,  /*Starting address of the object list*/
		   size_t nmemb,/*Number of objects in the list*/ 
		   size_t size, /*Number of bytes per object*/
		   int (*compar)(ss_pers_t *, ss_pers_t *) /*Comparison function. If the return value is > 0, then
							       the two arguments' positions in the list will be switched.*/
		   )
{
  int l_didSomething=1;
  size_t i;
  void *l_tmp;
  
  if(!nmemb || !size) return;

  l_tmp = malloc(size);

  while(l_didSomething)
    { 
      l_didSomething=0;
      for(i=0;i<nmemb-1;i++)
	{
	  int l_val = (*compar)( (void *)((size_t)base+i*size), (void *)((size_t)base+(i+1)*size) );
	  if(l_val>0)
	    {
	      memcpy(l_tmp,(void *)((size_t)base+i*size),size);
	      memcpy((void *)((size_t)base+i*size),(void *)((size_t)base+(i+1)*size),size);
	      memcpy((void *)((size_t)base+(i+1)*size),l_tmp,size);
	      l_didSomething=1;
	    }
	}
    }
  free(l_tmp);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing Lists
 * Purpose:    	Given two lists of saf objects, match up elements in the two lists for comparison.
 *
 * Description: If order=strict, then compare objects in two lists based on their position in the list.  
 *		If order=any then Given two lists of saf objects, 
 *		instead of comparing the items in the list based on their position in the list,
 *		try to find an appropriate match in the other list, for each item.
 *
 *		This algorithm is "fast" because instead of scaning the second list for an appropriate match, for each object
 *		in the first list (this algorithm requires time Theta(leftnum * rightnum)), what this algorithm does is
 *		sort the lists, then you can compare objects almost by position in the list.  Thus it requires time
 *		Theta(leftnum * log(leftfnum) + rightnum * log(rightnum)).
 *
 * Issues:	If g_globalfo.exclusive (i.e. the user wants exclusive objects displayed) then in order to re-use the 
 *		exsisting compare_xxx functions, I created a bunch of SAF_Xxx g_missing_xxx objects that can be
 *		used to compare real saf objects with, but have special values so that the g_missing_xxx object doesn't 
 *		get printed.  Problem: not all of the g_missing_xxx objects are constructed correctly.  
 *		Also note we have #define compare_xxx_missing g_missing_xxx to make this macro work.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 17, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
#define COMPARE_XXX_LISTS(left, right, leftnum, rightnum, size, sort_compare, compare_xxx)                       	\
{	size_t l=0,r=0,i,minnum;                                                                                       	\
	if(g_strictorder){												\
		minnum=MIN(leftnum,rightnum);									\
		for(i=0; i<minnum; i++){										\
			compare_xxx(left+i, right+i);									\
		}													\
		if(g_globalfo.exclusive){									\
			for(i=rightnum; i<leftnum; i++){								\
				compare_xxx(left+i, &compare_xxx ## _missing);						\
			}												\
			for(i=leftnum; i<rightnum; i++){								\
				compare_xxx( &compare_xxx ## _missing, right+i);					\
			}												\
		}													\
	}else{														\
		safdiff_sort((ss_pers_t *)(left), leftnum, size, sort_compare);                                            	\
		safdiff_sort((ss_pers_t *)(right), rightnum, size, sort_compare);                                           	\
	 	                                                                                                     	\
		/*after the lists are sorted, we can go through the list only once to find matching names (linear time)*/\
		while(l<leftnum && r<rightnum){                                                                     	\
			while( l<leftnum && r<rightnum && sort_compare((ss_pers_t *)(left+l),(ss_pers_t *)(right+r)) == 0 ){                 	\
				compare_xxx(left+l,right+r);                                                       	\
				l++; r++;                                                                            	\
			}                                                                                            	\
			while( l<leftnum && r<rightnum && sort_compare((ss_pers_t *)(left+l),(ss_pers_t *)(right+r)) < 0 ){                  	\
				if(g_globalfo.exclusive){								\
					compare_xxx(left+l, &compare_xxx ## _missing);					\
				}											\
				l++;                                                                                 	\
			}                                                                                            	\
			while( l<leftnum && r<rightnum && sort_compare((ss_pers_t *)(left+l),(ss_pers_t *)(right+r)) > 0 ){                  	\
				if(g_globalfo.exclusive){								\
					compare_xxx(&compare_xxx ## _missing, right+r);					\
				}											\
				r++;                                                                                 	\
			}                                                                                            	\
		}                                                                                                    	\
		if(g_globalfo.exclusive){										\
			while(l<leftnum){										\
				compare_xxx(left+l, &compare_xxx ## _missing);						\
				l++;											\
			}												\
			while(r<rightnum){										\
				compare_xxx(&compare_xxx ## _missing, right+r);						\
				r++;											\
			}												\
		}													\
	}														\
	return 0;                                                                                            		\
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing Lists
 * Purpose:    	Given two lists of saf objects, canonicalize the two lists so that corresponding elements are at the
 *		same index in the list.
 *
 * Description: This function takes two lists that can be in any order and re-orders the elements so that corresponding
 *		elements are at the same index in the list.  Elements in one list that do not have a corresponding element
 * 		in the other list are after the return value of this function, elements before the return value of this
 *		function can be paired 1 to 1 with each other for comparision.  So the prefix of the two lists, of 
 *		length the return value of this function can be compared with each other, elements whose index
 *		is >= the return value have no corresponding match in the other list, so you can call
 *		compare_xxx(list[>= return value], missing object) on those elements.  and you can call
 *		compare_xxx(left[i], right[i]) for 0<= i < return value.
 *
 *
 * Issues:	
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL January 7, 2002
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/ 
#define CANONICALIZE_XXX_LISTS(left, right, leftnum, rightnum, size, sort_compare, type)		                \
{	size_t l=0,r=0, prefix=0;											\
        int leftend = (int)leftnum-1;   /*index to the end of the left list, to put elements that don't have a match*/  \
        int rightend = (int)rightnum-1; /*index to the end of the right list, to put elements that don't have a match*/ \
        type *leftgoal, *rightgoal;                                                                                     \
        leftgoal = malloc(size * leftnum);                                                                              \
	rightgoal = malloc(size * rightnum);                                                                            \
	if(!g_strictorder){												\
		safdiff_sort((ss_pers_t *)(left), leftnum, size, sort_compare);                                             	\
		safdiff_sort((ss_pers_t *)(right), rightnum, size, sort_compare);                                           	\
		/*after the lists are sorted, we can go through the list only once to find matching names (linear time)*/\
		while(l<leftnum && r<rightnum){                                                                     	\
			while( l<leftnum && r<rightnum && sort_compare((ss_pers_t *)(left+l),(ss_pers_t *)(right+r)) == 0 ){                 	\
				leftgoal[prefix]=left[l];                                                               \
			        rightgoal[prefix]=right[r];                                                             \
				prefix++;										\
				l++; r++;                                                                            	\
			}                                                                                            	\
			while( l<leftnum && r<rightnum && sort_compare((ss_pers_t *)(left+l),(ss_pers_t *)(right+r)) < 0 ){                  	\
				leftgoal[leftend]=left[l];                                                              \
			        leftend--;                                                                              \
				l++;                                                                                 	\
			}                                                                                            	\
			while( l<leftnum && r<rightnum && sort_compare((ss_pers_t *)(left+l),(ss_pers_t *)(right+r)) > 0 ){                  	\
			        rightgoal[rightend]=right[r];                                                           \
			        rightend--;                                                                             \
				r++;                                                                                 	\
			}                                                                                            	\
		}                                                                                                    	\
	        while(l<leftnum){                                                                                       \
	               leftgoal[leftend]=left[l];                                                                       \
		       leftend--;                                                                                       \
		       l++;                                                                                             \
                }                                                                                                       \
                while(r<rightnum){                                                                                      \
		       rightgoal[rightend]=right[r];                                                                    \
		       rightend--;                                                                                      \
		       r++;                                                                                             \
	        }                                                                                                       \
	}														\
        memcpy(left, leftgoal, size*leftnum);                                                                           \
        memcpy(right, rightgoal, size*rightnum);                                                                        \
	free(leftgoal);                                                                                                 \
	free(rightgoal);                                                                                                \
	return( (int)prefix );                                                                                        	\
}


#if defined WIN32 || defined JANUS
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: For some reason, the multi-threaded library doesnt seem
 * to contain snprintf? XXX Figure out the correct library
 * later, but for now just create our own dumb snprintf
 * here. There could easily be a memory problem with this!
 *
 *----------------------------------------------------------------------------------------------------------------*/
int snprintf(char *buf, size_t count, const char *fmt, ... )
{
  va_list args;
  va_start(args,fmt);
  vsprintf(buf, fmt, args);

  if(strlen(buf)>=count)
  {
	  printf("WIN32 snprintf error here!! %d %d\n",strlen(buf),count);
	  exit(-1);
  }
}
#endif



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing Lists
 * Purpose:    	Do something to two lists of objects.
 *
 * Description: Do something to two lists of objects, where the two lists can have different lengths.
 *		Let mincount=MIN(leftcount,rightcount), this funciton does something to the part the two lists
 *		have in common, from 0 to mincount-1, and then another thing to the remaining part of the longer list.
 *		This function is mostly used for printing two arrays of different length.
 *
 * Parallel:    Same as serial.
 *
 * Programmer:	Matthew O'Brien, LLNL November 21, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
#define PRINT_SOME_STRINGS(a_printLevel,leftcount, rightcount, i, label,leftstring, rightstring)	\
  {									\
	int mincount;							\
	mincount = MIN(leftcount, rightcount);				\
	for(i=0; i<mincount; i++){					\
	    snprintf(g_print_some_strings_buffer,1023,"%s[%d]",label,i); \
	    print_string(a_printLevel,g_print_some_strings_buffer, leftstring, rightstring);	\
	}								\
	for(i=rightcount; i<leftcount; i++){				\
	    snprintf(g_print_some_strings_buffer,1023,"%s[%d]",label,i); \
	    print_string(a_printLevel,g_print_some_strings_buffer, leftstring, "");		\
	}								\
	for(i=leftcount; i<rightcount; i++){				\
	    snprintf(g_print_some_strings_buffer,1023,"%s[%d]",label,i); \
	    print_string(a_printLevel,g_print_some_strings_buffer, "", rightstring);		\
	}								\
  }





/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print Macros
 *
 * Description: Some print macros.
 *
 * Programmer:	Matthew O'Brien, LLNL November 21, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
#define dprint(x) printf(#x " = %g\n", x)
#define dprint2(x,y) printf(#x " = %g " #y " = %g\n", x, y)
#define iprint(x) printf(#x " = %i\n", x)
#define iprint2(x,y) printf(#x " = %i " #y " = %i\n", x, y)
#define sprint(x) printf(#x " = %s\n", x)
#define sprint2(x,y) printf(#x " = %s " #y " = %s\n", x, y)



/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	Compare Two Field Templates
 *
 * Description: Compares two field templates by using strcmp to compare first the names,
 *              then the basis names, then the algebraic names. It returns a number less than
 *              zero if a_left is "less than" a_right, greater than zero if a_left is "greater
 *              than" a_right, and zero otherwise.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int sort_compare_field_tmpls(ss_pers_t *a_left, ss_pers_t *a_right)
{
  SAF_FieldTmpl *l_lefttmpl;
  SAF_FieldTmpl *l_righttmpl;
  int l_val;

  l_lefttmpl = (SAF_FieldTmpl *)a_left;
  l_righttmpl = (SAF_FieldTmpl *)a_right;
  if( (l_val=compare_field_tmpl_names((ss_pers_t *)l_lefttmpl,(ss_pers_t *)l_righttmpl)) ) return(l_val);

  {
    SAF_Algebraic leftalg,rightalg;
    SAF_Basis leftbasis, rightbasis;
    saf_describe_field_tmpl(SAF_ALL, l_lefttmpl, NULL, &leftalg, &leftbasis, NULL, NULL, NULL );
    saf_describe_field_tmpl(SAF_ALL, l_righttmpl, NULL, &rightalg, &rightbasis, NULL, NULL, NULL ); 

    if( (l_val=strcmp(basis_name(&leftbasis), basis_name(&rightbasis))) ) return(l_val);
    if( (l_val=strcmp(algebraic_name(&leftalg), algebraic_name(&rightalg))) ) return(l_val);
  }

  return(0);
}


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: Compare two fields from the same database by first comparing the names,
 *              then the timestep, and finally the row number. Returns < 0 for less-than,
 *              0 for equal, and > 0 for greater-than.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *----------------------------------------------------------------------------------------------------------------*/
int sort_compare_fields(ss_pers_t *a_left, ss_pers_t *a_right)
{
  SAF_Field *l_leftfield;
  SAF_Field *l_rightfield;
  int l_val;

  l_leftfield = (SAF_Field *)a_left;
  l_rightfield = (SAF_Field *)a_right;

  /*compare field name*/
  if( (l_val=compare_field_names((ss_pers_t *)l_leftfield,(ss_pers_t *)l_rightfield)) ) return(l_val);


  /*compare timestep (if we know the timesteps) after finding out which 
    database these fields are from*/
  {
    SAF_Db *l_leftdb;
    SAF_Db *l_rightdb;
    SAF_Db l_leftfielddb;
    SAF_Db l_rightfielddb;

    int *leftbuffer=NULL;
    int *rightbuffer=NULL;

    l_leftdb = g_leftdb;
    l_rightdb = g_rightdb;

    ss_pers_file((ss_pers_t *)l_leftfield,&l_leftfielddb);
    ss_pers_file((ss_pers_t *)l_rightfield,&l_rightfielddb);
    
    if( !SAF_EQUIV(&l_leftfielddb,&l_rightfielddb) )
      {
	printf("sort_compare_fields: fields must be from same database\n");
	exit_safdiff(SAFDIFF_ERROR);
      }

    if( SAF_EQUIV(&l_leftfielddb,l_leftdb) )
      {
	if(g_leftWhichTimestepPerFieldRow)
	  {
           leftbuffer = Ftab_find(g_leftWhichTimestepPerFieldRow,  l_leftfield, g_nleft);
           rightbuffer = Ftab_find(g_rightWhichTimestepPerFieldRow,  l_rightfield, g_nright);
           if (rightbuffer && leftbuffer)  
              { l_val = leftbuffer[0] - rightbuffer[0];
	        if(l_val) return(l_val);
               }
	  }
      }
    else if( SAF_EQUIV(l_leftfield,l_rightdb) ) 
      {
	if(g_rightWhichTimestepPerFieldRow)
	  {
           leftbuffer = Ftab_find(g_rightWhichTimestepPerFieldRow,  l_leftfield, g_nleft);
           rightbuffer =Ftab_find(g_rightWhichTimestepPerFieldRow,  l_rightfield, g_nright);
           if (rightbuffer && leftbuffer)  
              { l_val = leftbuffer[0] - rightbuffer[0];
	        if(l_val) return(l_val);
               }
	  }      
      }
    else
      {
	printf("sort_compare_fields: fields must be from left or right database\n");
	exit_safdiff(SAFDIFF_ERROR);
      }
  }


  /*compare row number*/
  /* Row numbers no longer exist
  l_val = (int)(l_leftfield->theRow - l_rightfield->theRow);
  if(l_val) return(l_val);
  */

  return(0);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Get the height and width of the output terminal.
 *
 * Description: Get the height and width of the output terminal.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  6, 2000
 *
 * Modifications:	Matthew O'Brien, LLNL Nov 19,2001, Robb wrote this for silodiff, I changed it to work with safdiff.
 *--------------------------------------------------------------------------------------------------------------------------------*/
void out_init_size(void)
{
  int         width = 80;             /*the default                   */
  int         height = 0;             /*the default is no paging info */
  const char  *s;

  /*
   * Try to get it from the COLUMNS environment variable first since its
   * value is sometimes wrong.
   */
  if ((s=getenv("COLUMNS")) && isdigit(*s)) {
    width = (int)strtol(s, NULL, 0);
  }
  if ((s=getenv("LINES")) && isdigit(*s)) {
    height = (int)strtol(s, NULL, 0);
  }
#if defined(TIOCGWINSZ)
  {
    /* Unix with ioctl(TIOCGWINSZ) */
    struct winsize w;
    if (ioctl(2, TIOCGWINSZ, &w)>=0 && w.ws_col>0) {
      width = w.ws_col;
      height = w.ws_row;
    }
  }
#elif defined(TIOCGETD)
  {
    /* Unix with ioctl(TIOCGETD) */
    struct uwdata w;
    if (ioctl(2, WIOCGETD, &w)>=0 && w.uw_width>0) {
      width = w.uw_width / w.uw_hs;
      height = w.uw_height / w.uw_vs; /*is this right? -rpm*/
    }
  }
#endif

#ifdef WIN32
	{
		CONSOLE_SCREEN_BUFFER_INFO l_info;
		HANDLE l_handle = GetStdHandle( STD_OUTPUT_HANDLE );

		if( l_handle == INVALID_HANDLE_VALUE )
		{
			/*printf("\nWIN32 GetStdHandle error=%d\n\n",GetLastError());*/
		}
		else
		{
			if( GetConsoleScreenBufferInfo( l_handle, &l_info ) )
			{
				width = l_info.dwSize.X - 1;/*note: could also use dwMaximumWindowSize*/
				height = l_info.dwSize.Y - 1;
			}
			else
			{
				/*printf("\nWIN32 GetConsoleScreenBufferInfo error=%d\n\n",GetLastError());*/
			}
		}
	}
#endif

  /* Set width to at least 1, height to at least 1 */
  if (width<1) width = 1;
  if (height<0) height = 1;

  /* Set the global values */
  if(g_terminal_height<0){
    /*g_terminal_height is initialized to -1, and can be set on the command line, we don't want
      to clobber what was set on the command line*/
    g_terminal_height=height;
  }
  g_terminal_width=width;

  /*        OUT_NCOLS = width;
	    OUT_NROWS = height;
	    OUT_LTMAR = 0.25 * OUT_NCOLS;
	    OUT_COL2 = (OUT_LTMAR+OUT_NCOLS)/2;*/
}


/*this structure is to organize the various ASCII output possiblities for different datatype {float, int, string}
  and whethere or not the left or right value is missing
*/

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Build a format string.
 *
 * Description: A macro to build a format string.  5 columns of output, it allows you to control the width and precision.
 *              This structure allows the organization of the various ASCII output possiblities for different datatype 
 *              {float, int, string} and whether or not the left or right value is missing.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 16, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------- */


#define BUILD_FORMAT_STRING(formatstring, g_valwidth, valprecision, leftformatchar, rightformatchar )	\
  {													\
	sprintf(formatstring, "%%-*.*s %%%i.%i%c %%%i.%i%c",						\
		g_valwidth, 	valprecision, 	leftformatchar,		/*2nd column: left value */	\
		g_valwidth, 	valprecision, 	rightformatchar );	/*3rd column: right value*/	\
  }


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Can a Floating Point Number be Effectively Printed in a Given Space?
 *
 * Description: Checks if the absolute value of a number is less than a given threshold
 * but still greater than zero, or greater than a given threshold. If either of these conditions is true, 
 * then the number will either take up too many characters when printed (e.g. 10000000000) or will be rounded
 * beyond recognition (e.g. .000000021 printed as .0000000). This function is used to automatically
 * determine if a number should be printed in exponential form instead of regular form. 
 *
 * If the arguments *a_uppermult and *a_lowermult are zero, then this function will calculate what the upper
 * and lower thresholds should be, based on the arguments a_width and a_prec.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int is_number_outside_precision( double a_num, /*the number in question*/
				 int a_width, /*the max char width for printing*/
				 int a_prec, /*the precision of the number*/
				 double *a_uppermult, /*the upper threshold if already known, otherwise zero*/
				 double *a_lowermult /*the lower threshold if already known, otherwise zero*/
				 )
{
  double l_abs=fabs(a_num);
  if(!*a_lowermult)
    {
      int i;
      *a_lowermult=1;
      for(i=0;i<a_prec;i++) *a_lowermult *= 10;
      *a_uppermult=1;
      for(i=0;i<a_width-a_prec-2;i++) *a_uppermult *= 10;
    }
  if( a_num!=0.0  && ( l_abs>*a_uppermult || l_abs**a_lowermult<1.0 ) ) return(1);
  return(0);
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Can a Floating Point Number be Effectively Printed?
 *
 * Description: This function is a wrapper for is_number_outside_precision so that the upper
 * and lower thresholds only have to be calculated once.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int is_number_outside_value_precision( double a_num )
{
  static double l_lowermult=0;
  static double l_uppermult=0;
  return(is_number_outside_precision( a_num,g_valwidth,g_precision,&l_uppermult,&l_lowermult));
}
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print a Difference
 *
 * Description: Prints a number in the difference column.
 *
 *----------------------------------------------------------------------------------------------------------------*/
void print_difference( double a_val )
{
  static char l_expstr[1024]="";
  static char l_regstr[1024]="";
  static double l_lowermult=0;
  static double l_uppermult=0;
  int l_useExp = is_number_outside_precision(a_val,g_diffwidth,g_precision,&l_uppermult,&l_lowermult);
  if( !strlen(l_expstr) )
  { 
    /*create the format strings one time only*/
    sprintf(l_expstr," %%%i.%i%c%%",g_diffwidth, g_precision -2/*make room for e-10?*/, 'e');
    sprintf(l_regstr," %%%i.%i%c%%",g_diffwidth, g_precision, 'f');
  }
  if(l_useExp) printf(l_expstr,a_val);
  else printf(l_regstr,a_val);
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print a Percentage
 *
 * Description: Prints a number in the percentage column.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void print_percentage( double a_val )
{
  static char l_str[1024]="";
  double l_val=1000;
  double l_abs = fabs(a_val);
  int l_maxPrecision = g_precision;
  int l_precision = g_precision;
  int l_width = g_perwidth;

  while(l_abs>=l_val)
    {
      l_precision--;
      l_val*=10;
      if(!l_precision) break;
    }

  if(!l_precision)
    sprintf(l_str," %%%i.%i%c%%%%\n",l_width, 	l_maxPrecision-2,	'e');
  else
    sprintf(l_str," %%%i.%i%c%%%%\n",l_width, 	l_precision,	'f');
  printf(l_str,a_val);

}




/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Build all of the format strings one time, at initialization, so they can be used in the printing routines.
 *
 * Description: Fill in the global variable g_formatString with all the possiblities for ASCII output.  It builds
 *		printf format strings to handle different data types {string, float, int} and whether or not
 *		the left or right argument is missing.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 26, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void build_formatStrings()
{
  char formatchar;
  char perchangechar='f', stringformatchar='s', floatformatchar='f', expformatchar='e';
  int orig_precision;

  out_init_size();/*this sets the global variables g_terminal_height and g_terminal_width */

  orig_precision=g_precision;

  if(g_terminal_width>1)
    {
      int l_usableWidth = g_terminal_width-1-4;/*4 spaces, 1 space between each column*/
      if(l_usableWidth>0)
	{
	  /*these match the orig proportions
	  double l_valFrac = .215;
	  double l_diffFrac = .14;
	  double l_perFrac = .14;
	  */

	  double l_valFrac =  .26;
	  double l_diffFrac = .09;
	  double l_perFrac =  .09;

	  int l_min = 10;

	  g_valwidth  = (int)(l_valFrac * (double)l_usableWidth);
	  g_diffwidth  = (int)(l_diffFrac * (double)l_usableWidth);
	  g_perwidth  = (int)(l_perFrac * (double)l_usableWidth);

	  if(g_valwidth<l_min) g_valwidth=l_min;
	  if(g_diffwidth<l_min) g_diffwidth=l_min;
	  if(g_perwidth<l_min) g_perwidth=l_min;

	  g_namewidth = l_usableWidth - g_valwidth*2 - g_diffwidth - g_perwidth;

	  if(g_namewidth<l_min) g_namewidth=l_min;
	}
      else
	{
	  printf("Error: your window is only %d chars wide.\n",g_terminal_width);
	  exit(-1);
	}

      /*
      printf("g_terminal_width=%d      g_namewidth=%d  g_valwidth=%d  g_diffwidth=%d  g_perwidth=%d    sum=%d\n",
	     g_terminal_width,g_namewidth,g_valwidth,g_diffwidth,g_perwidth,
	     g_namewidth+g_valwidth*2+g_diffwidth+g_perwidth);
      */


      if(orig_precision < 0) g_precision = MAX(2,g_perwidth-5);    /*g_precision is only overwritten if the user didn't manually set it*/
      /*now we have g_namewidth + 1 + g_valwidth + 1 + g_valwidth + 1 + g_diffwidth + 1 + g_perwidth + 1 == g_terminal_width */
    }
  if(g_columnwidth){
    g_namewidth = g_columnwidth;
    g_valwidth  = g_columnwidth;
    g_diffwidth = g_columnwidth;
    g_perwidth  = g_columnwidth;
    if(orig_precision < 0) g_precision = MAX(2,g_columnwidth-6); /*g_precision is only overwritten if the user didn't manually set it*/
  }

  /*default value of 141*/
  g_totalwidth = g_namewidth + 1 + g_valwidth + 1 + g_valwidth + 1 + g_diffwidth + 1 + g_perwidth + 1;

  switch(g_globalfo.obase)
    {
    case  2: formatchar='s'; break;
    case  8: formatchar='o'; break;
    case 10: formatchar='i'; break;
    case 16: formatchar='x'; break;
    default: printf("output base error, the base must be one of 2,8,10 or 16\n"); exit_safdiff(SAFDIFF_ERROR);
    }

  /*integers*/
  if(g_globalfo.obase==2)
    {
      char lformatchar;
      int lprecision;
      int lvalwidth;

      lformatchar='s';
      lprecision=32+3;
      lvalwidth=lprecision;
      sprintf(g_formatString.m_intf, 	   "left    %s-%i%c %s%i.%i%c\n"
	      "right   %s%i.%i%c\n"
	      "diff    %s%i.%i%c\n"
	      "rel. diff    %s%i.%i%c%%%%\n",
	      "%",g_namewidth, 's', 					/*1st column: label      */
	      "%",lvalwidth, lprecision, lformatchar,			/*2nd column: left value */
	      "%",lvalwidth+g_namewidth+1, lprecision, lformatchar, 	/*3rd column: right value*/
	      "%",lvalwidth+g_namewidth+1, lprecision, lformatchar,	/*4th column: difference */
	      "%",lvalwidth+g_namewidth+1-6, g_precision, perchangechar);	/*5th column: %Change    */

      /*with base two, the percentage IS included in the format string, so dont print percentage*/

    }else{
      BUILD_FORMAT_STRING(g_formatString.m_intf, g_valwidth,1,formatchar,formatchar);

    }
  BUILD_FORMAT_STRING(g_formatString.m_int_leftmissing,  g_valwidth, 1,'s',formatchar);
  BUILD_FORMAT_STRING(g_formatString.m_int_rightmissing,  g_valwidth, 1,formatchar,'s');




  /*floats*/
  formatchar=floatformatchar;
  if(g_globalfo.obase==2)
    {
      char lformatchar;
      int lprecision;
      int lvalwidth;

      lformatchar='s';
      lprecision=64+3;
      lvalwidth=lprecision;
      sprintf(g_formatString.m_floatf, 	   "left    %s-%i%c %s%i.%i%c\n"
	      "right   %s%i.%i%c\n"
	      "diff    %s%i.%i%c\n"
	      "rel. diff    %s%i.%i%c%%%%\n",
	      "%",g_namewidth, 's', 					/*1st column: label      */
	      "%",lvalwidth, lprecision, lformatchar,			/*2nd column: left value */
	      "%",lvalwidth+g_namewidth+1, lprecision, lformatchar, 	/*3rd column: right value*/
	      "%",lvalwidth+g_namewidth+1, lprecision, lformatchar,	/*4th column: difference */
	      "%",lvalwidth+g_namewidth+1-6, g_precision, perchangechar);/*5th column: %Change    */

      /*with base two, the percentage IS included in the format string, so dont print percentage*/

    }else{
      BUILD_FORMAT_STRING(g_formatString.m_floatf, g_valwidth,g_precision,formatchar,formatchar);
    }
  BUILD_FORMAT_STRING(g_formatString.m_float_leftmissing, g_valwidth,g_precision,'s',formatchar);
  BUILD_FORMAT_STRING(g_formatString.m_float_rightmissing, g_valwidth,g_precision,formatchar,'s');



  /*exponential - used for very small numbers, only valid for base 10*/
  formatchar=expformatchar;
  if(g_globalfo.obase!=10)
    {
      /*just use the float formats*/
      strcpy(g_formatString.m_expf,g_formatString.m_floatf);
      strcpy(g_formatString.m_exp_leftmissing,g_formatString.m_float_leftmissing);
      strcpy(g_formatString.m_exp_rightmissing,g_formatString.m_float_rightmissing);
    }
  else
    {
      /*could try either expanding the precision, or reducing the valwidth by l_extraBits*/
      /*int l_extraBits = g_valwidth-g_precision-4; */

      sprintf(g_formatString.m_expf, "%%-*.*s %%%i.%i%c %%%i.%i%c",
	      g_valwidth,g_precision, 	'e',	
	      g_valwidth/*-l_extraBits*/, 	g_precision, 	'e');
 
      sprintf(g_formatString.m_exp_leftmissing, "%%-*.*s %%%i.%i%c %%%i.%i%c",
	      g_valwidth,g_precision, 	's',
	      g_valwidth, 	g_precision, 	'e');

      sprintf(g_formatString.m_exp_rightmissing, "%%-*.*s %%%i.%i%c %%%i.%i%c",
	      g_valwidth,g_precision, 	'e',
	      g_valwidth, 	g_precision, 	's');
    }


  /*strings*/
  formatchar=stringformatchar;

  sprintf(g_formatString.m_string, "%%-*.*s %%%i%c %%%i%c %%%i%c %%%i%c\n", 
	  g_valwidth,formatchar,	/*2nd column: left value */
	  g_valwidth, formatchar, 	/*3rd column: right value*/
	  g_diffwidth, formatchar,	/*4th column: difference */
	  g_perwidth+1, 's');	/*5th column: %Change    */


  sprintf(g_formatString.m_heading, "%%-*.*s %%%i%c %%%i%c %%%i%c %%%i%c\n",
	  g_valwidth,formatchar,	/*2nd column: left value */
	  g_valwidth, formatchar, 	/*3rd column: right value*/
	  g_diffwidth, formatchar,	/*4th column: difference */
	  g_perwidth+1, 's');		/*5th column: %Change    */


}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Initialization
 * Purpose:    	Initialize some saf objects to "missing" values.
 *
 * Description: Initialize some saf objects to "missing" values, so that we can compare exclusive objects to these
 *		missing objects and the missing object will have blank values, and the exclusive object will have
 *		its regular values appear.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void initializeMissingObjects()
{
  SAF_Unit meter;
  saf_find_one_unit(g_missingdb,"meter",&meter);
  /*saf_find_one_unit(g_missingdb,"second",&usec);*/
  saf_declare_role(SAF_ALL, g_missingdb, "", "", &g_missing_role); 
  saf_declare_category(SAF_ALL, g_missingdb, "", &g_missing_role, DUMMY_INT, &g_missing_cat);
  saf_declare_set(SAF_ALL, g_missingdb, "", DUMMY_INT, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &g_missing_set);
  saf_declare_collection(SAF_ALL, &g_missing_set, &g_missing_cat, SAF_CELLTYPE_POINT, 
			 DUMMY_INT, SAF_1DC(DUMMY_INT), SAF_DECOMP_FALSE);
  g_missing_quantity = saf_declare_quantity(SAF_ALL,g_missingdb,"","",NULL,NULL);
  /****
       {
       SAF_Quantity infopermol;
       infopermol = saf_declare_quantity("","","");
       sprint("after declare");
       compare_quantity(infopermol, infopermol);
       saf_multiply_quantity(infopermol, saf_find_one_quantity(g_leftdb, "information"),1);
       sprint("after multiply by information");
       compare_quantity(infopermol, infopermol);
       saf_multiply_quantity(infopermol, saf_find_one_quantity(g_leftdb, "amount"),-1);
       sprint("after mult by amount");
       compare_quantity(infopermol, infopermol);

       saf_multiply_quantity(infopermol, saf_find_one_quantity(g_leftdb, "amount"),-1);
       sprint("after mult by amount");
       compare_quantity(infopermol, infopermol);

       }
  *****/
  saf_multiply_quantity(SAF_ALL, g_missing_quantity, SAF_QTIME,DUMMY_INT);
  saf_multiply_quantity(SAF_ALL, g_missing_quantity, SAF_QMASS,DUMMY_INT);
  saf_multiply_quantity(SAF_ALL, g_missing_quantity, SAF_QCURRENT,DUMMY_INT);
  saf_multiply_quantity(SAF_ALL, g_missing_quantity, SAF_QLENGTH,DUMMY_INT);
  saf_multiply_quantity(SAF_ALL, g_missing_quantity, SAF_QLIGHT,DUMMY_INT);
  saf_multiply_quantity(SAF_ALL, g_missing_quantity, SAF_QTEMP,DUMMY_INT);
  saf_multiply_quantity(SAF_ALL, g_missing_quantity, SAF_QAMOUNT,DUMMY_INT);

  saf_declare_unit(SAF_ALL, g_missingdb, "","","",&g_missing_unit);
  saf_quantify_unit(SAF_ALL, &g_missing_unit,g_missing_quantity,DUMMY_FLOAT);
  saf_multiply_unit(SAF_ALL, &g_missing_unit,DUMMY_FLOAT,&meter,0);
  saf_offset_unit(SAF_ALL, &g_missing_unit, DUMMY_FLOAT);
  saf_log_unit(SAF_ALL, &g_missing_unit,DUMMY_FLOAT, DUMMY_FLOAT);

  saf_declare_algebraic(SAF_ALL, g_missingdb, "","",SAF_TRISTATE_FALSE,&g_missing_algebraic);
  saf_declare_basis(SAF_ALL, g_missingdb, "","",&g_missing_basis);
  saf_declare_evaluation(SAF_ALL, g_missingdb, "","",&g_missing_eval);

  saf_declare_field_tmpl(SAF_ALL,g_missingdb,"",&g_missing_algebraic,&g_missing_basis,
			 g_missing_quantity,1,NULL,&g_missing_field_tmpl);
  saf_declare_field(SAF_ALL,g_missingdb,&g_missing_field_tmpl,"",&g_missing_set,&g_missing_unit,SAF_SELF(g_missingdb), 
		    &g_missing_cat, DUMMY_INT, &g_missing_cat, &g_missing_eval, /*SAF_SPACE_PWCONST*/
		    SAF_DOUBLE,NULL,SAF_INTERLEAVE_NONE, SAF_IDENTITY,NULL,&g_missing_field);


  /*Note we must have default coords on the top mesh set, or
    saf_declare_state_group will fail. Perhaps this should be
    changed in saf?*/
  saf_declare_coords(SAF_ALL,&g_missing_field);
  saf_declare_default_coords(SAF_ALL,&g_missing_set,&g_missing_field);


  saf_declare_suite(SAF_ALL,g_missingdb,"",&g_missing_set,NULL,&g_missing_suite);



  {
    int index[1];
    double time[1];
    SAF_Field field_list[3];
    SAF_FieldTmpl field_tmpl[3];

    field_tmpl[0]=g_missing_field_tmpl;
    field_tmpl[1]=g_missing_field_tmpl;
    field_tmpl[2]=g_missing_field_tmpl;

    saf_declare_state_tmpl(SAF_ALL,g_missingdb ,"", 3,field_tmpl, &g_missing_state_tmpl);

    saf_declare_state_group(SAF_ALL,g_missingdb , "", &g_missing_suite, &g_missing_set, &g_missing_state_tmpl, 
			    SAF_TIME, SAF_ANY_UNIT, SAF_DOUBLE, &g_missing_state_group);
			 /* SAF_TIME, SAF_NOT_APPLICABLE_UNIT, SAF_DOUBLE, &g_missing_state_group); */
 

    index[0]=0;
    time[0]=DUMMY_FLOAT;
    field_list[0]=g_missing_field;
    field_list[1]=g_missing_field;
    field_list[2]=g_missing_field;

    saf_write_state(SAF_ALL,&g_missing_state_group,index[0],&g_missing_set,SAF_DOUBLE, time, field_list);
  }
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print SAF Version Info
 *
 * Description: Prints the SAF and HDF5 version numbers for the reader and the two compared databases.
 *
 *----------------------------------------------------------------------------------------------------------------*/
int print_version_info(const char *a_leftname, const char *a_rightname)
{
  char l_safLabelStr[64],l_leftSafStr[64],l_rightSafStr[64];
  char l_hdfLabelStr[64],l_leftHdfStr[64],l_rightHdfStr[64];
  char l_leftMpiStr[64],l_rightMpiStr[64];
  char l_annot[16];
  SAF_PathInfo l_pathinfo;
  int l_major=0,l_minor=0,l_patch=0;

  /*left*/
  l_pathinfo = saf_readInfo_path(a_leftname,0);
  if(saf_getInfo_isSAFdatabase(l_pathinfo))
    {
      saf_getInfo_libversion(l_pathinfo,&l_major,&l_minor,&l_patch,l_annot);
      sprintf(l_leftSafStr,"SAF-%d.%d.%d%s%s",l_major,l_minor,l_patch, (strlen(l_annot) ? "-":""), l_annot);

      if( l_major!=SAF_VERSION_MAJOR || l_minor!=SAF_VERSION_MINOR || l_patch!=SAF_VERSION_RELEASE )
	{
	  /*should this be an error?*/
	}

      saf_getInfo_hdfversion(l_pathinfo,&l_major,&l_minor,&l_patch,l_annot);
      sprintf(l_leftHdfStr,"HDF5-%d.%d.%d%s%s",l_major,l_minor,l_patch, (strlen(l_annot) ? "-":""),l_annot);

      if( l_major!=H5_VERS_MAJOR || l_minor!=H5_VERS_MINOR || l_patch!=H5_VERS_RELEASE )
	{
	  /*should this be an error?*/
	}

#ifdef HAVE_PARALLEL
      saf_getInfo_mpiversion(l_pathinfo,&l_major,&l_minor,&l_patch,l_annot);
#else
	  l_major=0;l_minor=0;l_patch=0;l_annot[0]='\0';
#endif
      sprintf(l_leftMpiStr,"MPI-%d.%d.%d%s%s",l_major,l_minor,l_patch, (strlen(l_annot) ? "-":""),l_annot);
    }
  else
    {
      printf("Error: left file %s is NOT a saf database\n", a_leftname);
      return(-1);
    }


  /*right*/
  l_pathinfo = saf_readInfo_path(a_rightname,0);
  if(saf_getInfo_isSAFdatabase(l_pathinfo))
    {
      saf_getInfo_libversion(l_pathinfo,&l_major,&l_minor,&l_patch,l_annot);
      sprintf(l_rightSafStr,"SAF-%d.%d.%d%s%s",l_major,l_minor,l_patch, (strlen(l_annot) ? "-":""),l_annot);

      if( l_major!=SAF_VERSION_MAJOR || l_minor!=SAF_VERSION_MINOR || l_patch!=SAF_VERSION_RELEASE )
	{
	  /*should this be an error?*/
	}

      saf_getInfo_hdfversion(l_pathinfo,&l_major,&l_minor,&l_patch,l_annot);
      sprintf(l_rightHdfStr,"HDF5-%d.%d.%d%s%s",l_major,l_minor,l_patch, (strlen(l_annot) ? "-":""),l_annot);

      if( l_major!=H5_VERS_MAJOR || l_minor!=H5_VERS_MINOR || l_patch!=H5_VERS_RELEASE )
	{
	  /*should this be an error?*/
	}

#ifdef HAVE_PARALLEL
      saf_getInfo_mpiversion(l_pathinfo,&l_major,&l_minor,&l_patch,l_annot);
#else
	  l_major=0;l_minor=0;l_patch=0;l_annot[0]='\0';
#endif
      sprintf(l_rightMpiStr,"MPI-%d.%d.%d%s%s",l_major,l_minor,l_patch, (strlen(l_annot) ? "-":""),l_annot);
    }
  else
    {
      printf("Error: right file %s is NOT a saf database\n", a_rightname);
      return(-1);
    }


  saf_freeInfo_path(l_pathinfo);
 

  sprintf(l_safLabelStr,"(reader is SAF-%d.%d.%d%s%s)",SAF_VERSION_MAJOR,SAF_VERSION_MINOR,
	  SAF_VERSION_RELEASE, (strlen(SAF_VERSION_ANNOT) ? "-":""),SAF_VERSION_ANNOT);
  sprintf(l_hdfLabelStr,"(reader is HDF5-%d.%d.%d%s%s)",H5_VERS_MAJOR,H5_VERS_MINOR,
	  H5_VERS_RELEASE, (strlen(H5_VERS_SUBRELEASE) ? "-":""),H5_VERS_SUBRELEASE);


  print_string(PRINT_LEVEL_1,l_safLabelStr,l_leftSafStr,l_rightSafStr);
  print_string(PRINT_LEVEL_1,l_hdfLabelStr,l_leftHdfStr,l_rightHdfStr);
  print_string_ignore(PRINT_LEVEL_1,"MPI version",l_leftMpiStr,l_rightMpiStr);



  return(0);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Initialization
 * Purpose:    	Initialize bit vectors to 0, when we see an object set its bit to 1.
 *
 * Description: We keep several bit vectors, for field templates and fields, and use these bit vectors to avoid repeating work.
 *		There is more than one way to encounter a field (via finding the fields of a set, or via reading the fields
 *		from a state field), the key into the bit vector is the _saf_rowOf_xxx number.  This uniquely identifies
 *		a saf object from other saf objects of the same type.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL December 12, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
void initializeBitVectorsForNewDatabase()
{
  g_field_tmpls_written = _saf_htab_new();
  g_fields_written = _saf_htab_new();
  g_leftfields_seen = _saf_htab_new();
  g_rightfields_seen = _saf_htab_new();

}


#define IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(different)				       \
      g_printed_lines++;								       \
      g_exit_value |= different;                                                               \
      if(g_printLevel<=PRINT_LEVEL_1 && different){			                       \
            print_break_no_buffer(PRINT_LEVEL_1,"Encountered differences, exiting early.");    \
            if( g_globalfo.store.anything )  {  \
               printf("Note: because of early exit, the differences file %s might be incomplete.\n",g_newdbname); \
               printf("      To generate the complete differences file, use option \"-v 2\" or greater.\n");  \
            }                                   \
            printf("%s AND %s ARE DIFFERENT\n", g_leftdbname, g_rightdbname);                  \
            exit_safdiff(SAFDIFF_DIFFERENT);                                                           \
      }  



/*This Macro subtracts the width of the characters needed to draw the tree in front of the label of
  a row.   This is so that the column still line up*/
#define DYNAMIC_NAME_WIDTH (g_namewidth - 2*g_indent )


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Context buffer
 * Purpose:    	Add a line to the context buffer
 *
 * Description: Add a line to the context buffer.  The given string is copied to the context buffer, so you can free
 *		your own copy if you want.  The context buffer is emptied when a difference is encountered.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 20, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
#define ADD_STRING_TO_CONTEXT_BUFFER(string)									\
{														\
/*	g_context_buffer[g_context_length]=(char *)malloc(strlen(string)*sizeof(char));	*/			\
        if(g_context_length >= MAX_CONTEXT_BUFFER_LENGTH){                                                      \
            /*we need to drop these lines if the buffer fills up*/                                              \
            g_context_length = MAX_CONTEXT_BUFFER_LENGTH-1;                                                     \
        }                                                                                                       \
	strcpy(g_context_buffer[g_context_length], string);							\
	g_context_length++;                                                                                     \
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print the necessary tree structure to the left of the label.
 *
 * Description: This macro uses the global variable "g_indent" to print the correct tree structure to the left
 *		of the label.
 *
 * Parallel:    Not parallel.
 *
 * Programmer:	Matthew O'Brien, LLNL November 26, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
#define PRE_PRINT										\
{												\
	if(g_show_context){									\
		int i;										\
		for(i=0; i<g_context_length; i++){						\
			printf("%s", g_context_buffer[i]);					\
			/*free(g_context_buffer[i]);*/						\
			g_context_buffer[i][0]='\0';						\
		}										\
		g_context_length=0;								\
	}											\
                                                                                                \
        if( g_printLevel>0 && strlen(g_hidden_context_buffer[0]) ) { \
              printf("%s",g_hidden_context_buffer[0]);       \
              if( strlen(g_hidden_context_buffer[1]) ) printf("%s",g_hidden_context_buffer[1]);       \
              g_hidden_context_buffer[0][0]='\0'; \
              g_hidden_context_buffer[1][0]='\0'; \
        } \
                                                                                                \
	if(g_drawtree){										\
		char spaces[128]="| | | | | | | | | | | | | | | | | | | | | | | | | | ";	\
	/*	if(g_indent > 1){	spaces[2*g_indent-3]='-'; spaces[2*g_indent-2]='\0';}else{spaces[0]='\0';}	*/ \
		if(g_indent>0){ spaces[2*g_indent-1]='-';						\
			spaces[2*g_indent]='\0';							\
		}else{spaces[0]='\0';}								\
		printf("%s",spaces);								\
	}else{											\
		g_indent=0;									\
	}											\
}

/*if we want to keep track of the last branch at a given level
#define PRE_PRINT_someday							\
{									\
int i;								\
\
for(i=0; i<g_indent-1; i++){					\
if(g_level[i]) 	printf("|-");				\
else		printf("  ");				\
}								\
if(g_indent!=0){							\
if(g_last[g_indent])	printf("+-");			\
else			printf("|-");			\
}else{								\
\
}								\
}	
*/


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print and Compare Integers
 *
 * Description: Prints and compares two integers.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void print_int_internal( t_safdiffPrintLevel a_printLevel, /*priority for determining whether anything is printed*/
			 const char *label, /*A descriptive label. Note that this must be named "label" because the
					      function uses legacy macros that require it.*/
			 int a_left, /*The left value*/
			 int a_right,/*The right value*/
			 int a_leftIsMissing, /*Flag signalling that the left value is missing, or meaningless*/
			 int a_rightIsMissing,/*Flag signalling that the right value is missing, or meaningless*/
			 int a_alwaysPass, /*Flag signalling that the test should pass, even if the comparision fails*/
			 int a_absrel_andor, /*See fieldOptions struct*/
			 double a_abs_tol, /*See fieldOptions struct*/
			 double a_rel_tol, /*See fieldOptions struct*/
			 double a_floor_tol /*See fieldOptions struct*/
			 )
{
  double per_change;
  int outside_threshold=0;

  if( a_leftIsMissing && a_rightIsMissing ) 
    {
      PRE_PRINT;
      if( g_printLevel>=a_printLevel ) printf("%s\n",label);
      return;
    }
  if( a_leftIsMissing )
    {
      PRE_PRINT;

      if(g_globalfo.obase==2)
	{	
	  printf(g_formatString.m_intf, label, "", binary(a_right), "",100.0);
	}
      else
	{	
	  printf(g_formatString.m_int_leftmissing, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH,label,
		 "", a_right );	
	  print_difference( (double)a_right);
	  print_percentage(100.0);
	}
      if(!a_alwaysPass) outside_threshold=1;
      IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(outside_threshold);
      return;
    }
  if( a_rightIsMissing )
    {
      PRE_PRINT;

      if(g_globalfo.obase==2)
	{	
	  printf(g_formatString.m_intf, label, binary(a_left),"",  "",-100.0);
	}
      else
	{	
	  printf(g_formatString.m_int_rightmissing, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH,label,
		 a_left, ""); 
	  print_difference(-1.0* (double)a_left);
	  print_percentage(-100.0);
	}
      if(!a_alwaysPass) outside_threshold=1;
      IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(outside_threshold);
      return;
    }

  per_change=PER_CHANGE(a_left,a_right);

  if(!a_alwaysPass) 
    {
      OUTSIDE_THRESHOLD(a_absrel_andor, a_abs_tol,
			a_rel_tol, a_floor_tol, per_change, a_left,
			a_right, outside_threshold);
    }

  if( g_printLevel>=a_printLevel || outside_threshold  )
    {	
      PRE_PRINT;
      if(g_globalfo.obase==2)
	{	
	  printf(g_formatString.m_intf, label, binary(a_left),
		 binary(a_right), binary(a_right-a_left),per_change);
	}
      else
	{	
	  printf(g_formatString.m_intf, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH, label,
		 a_left,a_right);
	  print_difference((double)(a_right-a_left));
	  print_percentage(per_change);
	}	
      IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(outside_threshold);	
    }	      
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print and Compare Integers
 *
 * Description: Prints and compares two integers using the preset global options.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void print_int(  t_safdiffPrintLevel a_printLevel, /*priority for determining whether anything is printed*/
		 const char *label, /*A descriptive label. Note that this must be named "label" because the
				      function uses legacy macros that require it.*/
		 int a_left, /*The left value*/
		 int a_right,/*The right value*/
		 int a_leftIsMissing, /*Flag signalling that the left value is missing, or meaningless*/
		 int a_rightIsMissing /*Flag signalling that the right value is missing, or meaningless*/		       
		 )
{
  print_int_internal( a_printLevel, label, a_left, a_right, 
		      a_leftIsMissing, a_rightIsMissing,
		      0,
		      g_globalfo.absrel_andor, g_globalfo.abs_tol,  g_globalfo.rel_tol,  g_globalfo.floor_tol );
}



/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print and Compare Floats
 *
 * Description: Prints and compares two floats.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void print_float_internal( t_safdiffPrintLevel a_printLevel, /*priority for determining whether anything is printed*/
			 const char *label, /*A descriptive label. Note that this must be named "label" because the
					      function uses legacy macros that require it.*/
			 double a_left, /*The left value*/
			 double a_right,/*The right value*/
			 int a_leftIsMissing, /*Flag signalling that the left value is missing, or meaningless*/
			 int a_rightIsMissing,/*Flag signalling that the right value is missing, or meaningless*/
			 int a_alwaysPass, /*Flag signalling that the test should pass, even if the comparision fails*/
			 int a_absrel_andor, /*See fieldOptions struct*/
			 double a_abs_tol, /*See fieldOptions struct*/
			 double a_rel_tol, /*See fieldOptions struct*/
			 double a_floor_tol /*See fieldOptions struct*/
			 )
{
  double per_change;
  int outside_threshold=0;

  if( a_leftIsMissing && a_rightIsMissing ) 
    {
      PRE_PRINT;
      if( g_printLevel>=a_printLevel ) printf("%s\n",label);
      return;
    }
  if( a_leftIsMissing )
    {
      PRE_PRINT;
      if(g_globalfo.obase==2)
	{	
	  printf(g_formatString.m_floatf, label, "", binaryd(a_right), "",100);
	}
      else
	{	
	  if( is_number_outside_value_precision(a_right) )
	    {
	      printf(g_formatString.m_exp_leftmissing, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH, label,
		     "",a_right);
	      print_difference(a_right);
	      print_percentage( 100.0 );
	    }
	  else
	    {
	      printf(g_formatString.m_float_leftmissing, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH,label,
		     "", a_right );
	      print_difference(a_right);
	      print_percentage(100.0);
	    }
	}
      if(!a_alwaysPass) outside_threshold=1;
      IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(outside_threshold);
      return;
    }

  if( a_rightIsMissing )
    {
      PRE_PRINT;

      if(g_globalfo.obase==2)
	{	
	  printf(g_formatString.m_floatf, label, binaryd(a_left),"", "",-100);
	}
      else
	{	
	  if( is_number_outside_value_precision(a_left) )
	    {
	      printf(g_formatString.m_exp_rightmissing, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH, label,
		     a_left,"");
	      print_difference(-1.0*a_left);
	      print_percentage( -100.0 );
	    }
	  else
	    {
	      printf(g_formatString.m_float_rightmissing, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH,label,
		     a_left, "");
	      print_difference(-1.0*a_left);
	      print_percentage(-100.0);
	    }
	}

      if(!a_alwaysPass) outside_threshold=1;
      IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(outside_threshold);
      return;
    }

  per_change=PER_CHANGE(a_left,a_right);

  if(!a_alwaysPass) 
    {
      OUTSIDE_THRESHOLD(a_absrel_andor, a_abs_tol,
			a_rel_tol, a_floor_tol, per_change, a_left,
			a_right, outside_threshold);
    }

  if( g_printLevel>=a_printLevel || outside_threshold  )
    {	
      if(g_globalfo.obase==2)
	{	
	  printf(g_formatString.m_floatf, label, binaryd(a_left),
		 binaryd(a_right), binaryd(a_right-a_left),per_change);
	  /*with base two, the percentage IS included in the format string, so dont print percentage*/
	}
      else
	{	
	  PRE_PRINT;
	  if( is_number_outside_value_precision(a_left) || is_number_outside_value_precision(a_right) )
	    {
	      /*numbers are small (or large) enough, we should use exp format*/
	      printf(g_formatString.m_expf, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH, label,
		     a_left,a_right);
	      print_difference(a_right-a_left);
	      print_percentage( per_change );
	    }
	  else
	    {
	      printf(g_formatString.m_floatf, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH, label,
		     a_left,a_right);
	      print_difference(a_right-a_left);
	      print_percentage( per_change );
	    }
	}	
      IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(outside_threshold);	
    }	      
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print and Compare Floats
 *
 * Description: Prints and compares two floats using the preset global options.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void print_float(  t_safdiffPrintLevel a_printLevel, /*priority for determining whether anything is printed*/
		 const char *label, /*A descriptive label. Note that this must be named "label" because the
				      function uses legacy macros that require it.*/
		 double a_left, /*The left value*/
		 double a_right,/*The right value*/
		 int a_leftIsMissing, /*Flag signalling that the left value is missing, or meaningless*/
		 int a_rightIsMissing /*Flag signalling that the right value is missing, or meaningless*/		       
		 )
{
  print_float_internal( a_printLevel, label, a_left, a_right, 
			a_leftIsMissing, a_rightIsMissing,
			0,
			g_globalfo.absrel_andor, g_globalfo.abs_tol,  g_globalfo.rel_tol,  g_globalfo.floor_tol );
}




/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print in multiple colums for easy comparison
 *
 * Description: Print integers, 5 columns of output, 1. lable (name)  2. left value  3. right value
 *		4. difference (right-left)  5. percent change 100*(right-left)/left
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL Septemper 24, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/

void print_long( t_safdiffPrintLevel a_printLevel, const char *label, long int a_left, long int a_right, 
		 int a_leftIsMissing, int a_rightIsMissing )
{
  print_int(a_printLevel,label,(int)a_left,(int)a_right,a_leftIsMissing,a_rightIsMissing);
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Prints and Compares Unsigned ints
 *
 * Description: Prints and compares two unsigned ints.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *----------------------------------------------------------------------------------------------------------------*/
void print_unsigned_int( t_safdiffPrintLevel a_printLevel, const char *label, unsigned int a_left, unsigned int a_right, 
		 int a_leftIsMissing, int a_rightIsMissing )
{
  print_int(a_printLevel,label,(int)a_left,(int)a_right,a_leftIsMissing,a_rightIsMissing);
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Return a string of 1s and 0s of the binary representation of an integer.
 *
 * Description: Return a string of 1s and 0s which is the base 2 representation of an integer.  This uses the safdiff
 *		returned string policy, a table of strings that gets cyclically recycled.  Copy this string if you want
 *		want it to stay around, and not be recycled by safdiff.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL Septemper 24, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
char *binary(int n)
{

  char octal[129];
  char *binary_return;
  int i, length, sizeof_int = sizeof(int);


  binary_return = saf_next_string();

  if(sizeof_int==4){
    /*it takes 11 octal digits to represent 32 bits*/
    sprintf(octal, "%11o", n);
  }else if(sizeof_int==8){
    /*it takes 22 octal digits to represent 64 bits*/
    sprintf(octal, "%22o", n);
  }else{
    iprint((int)sizeof(int));
    printf("sizeof(int) is not 4 or 8, error\n"); exit_safdiff(SAFDIFF_ERROR);
  }
	
  length=(int)strlen(octal);


  binary_return[0]='\0';

  if(sizeof_int==8){

    switch(octal[0]){

    case ' ':
    case '0': strcat(binary_return,"00"); break;
    case '1': strcat(binary_return,"01"); break;

    default: printf("a 64 bit int requires 22 octal dits to represent, but the first octal digit "
		    "must be a 0 or a 1, no greater\n"); exit_safdiff(SAFDIFF_ERROR);
    }
  }else{

    switch(octal[0]){
			
    case ' ':
    case '0': strcat(binary_return,"00"); break;
    case '1': strcat(binary_return,"01"); break;
    case '2': strcat(binary_return,"10"); break;
    case '3': strcat(binary_return,"11"); break;
	
    default: printf("a 32 bit int requires 11 octal dits to represent, but the first octal digit "
		    "must be 0, 1, 2 or 3, no greater\n"); exit_safdiff(SAFDIFF_ERROR);
    }
  }


  for(i=1; i<length; i++){
    switch(octal[i]){
			
    case ' ':
    case '0': strcat(binary_return,"000"); break;
    case '1': strcat(binary_return,"001"); break;
    case '2': strcat(binary_return,"010"); break;
    case '3': strcat(binary_return,"011"); break;
    case '4': strcat(binary_return,"100"); break;
    case '5': strcat(binary_return,"101"); break;
    case '6': strcat(binary_return,"110"); break;
    case '7': strcat(binary_return,"111"); break;

    }
  }

  return binary_return;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Return a string of 1s and 0s of a floating point number.
 *
 * Description: Return a string of 1s and 0s which is the base 2 representation of a floating point numer.
 *		It will deal with 4 and 8 byte floating point numbers.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
#define BINARY_FLOATING_POINT(f, type, set_r_to_binary_string)					\
{												\
	char *b, *r;										\
	int mantissa=0, exponent=0;									\
	int i, sizeof_type;									\
	number_t n;										\
												\
        sizeof_type=sizeof(type);                                                               \
	if(sizeof_type==8){									\
		mantissa=52;									\
		exponent=11;									\
	}else if(sizeof_type==4){								\
		mantissa=23;									\
		exponent= 8;									\
	}else{											\
		printf("the function only works on 4 or 8 byte floating point numbers\n");	\
		exit_safdiff(SAFDIFF_ERROR);								\
	}											\
	b=saf_next_string();									\
												\
	/*if the bit layout of floats are sign, then exponent, then mantissa*/			\
		SWAP(mantissa,exponent,int);							\
	/*otherwise, don't swap if the bit layout is sign, mantissa then exponent*/		\
                                                                                                \
	set_r_to_binary_string									\
												\
	b[0]=r[0];										\
	b[1]='-';										\
	for(i=1; i<=mantissa; i++){								\
		b[i+1]=r[i];									\
	}											\
	b[mantissa+2]='-';									\
												\
	for(i=mantissa+1; i<=mantissa+exponent; i++){						\
		b[i+2]=r[i];									\
	}											\
												\
	return b;										\
}												

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Return a string of 1s and 0s of a float.
 *
 * Description: Return a string of 1s and 0s which is the base 2 representation of a float.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
char *binaryf(float d)
{
	BINARY_FLOATING_POINT(d, float, { n.floatt=d; r=binary(n.intt[0]); } );
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Return a string of 1s and 0s of a double.
 *
 * Description: Return a string of 1s and 0s which is the base 2 representation of a double.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
char *binaryd(double d)
{

#ifndef WIN32
	BINARY_FLOATING_POINT(d, double, { n.doublet=d; r=binaryll(n.longlongt); } );
#else
	BINARY_FLOATING_POINT(d, double, { n.doublet=d; r=binary(n.longt); } );
#endif
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Return a string of 1s and 0s of a long long.
 *
 * Description: Return a string of 1s and 0s which is the base 2 representation of a long long.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/

#ifndef WIN32
char *binaryll(long long ll)
{
	number_t n;
	char *a,*b;

	n.longlongt=ll;
	a=binary(n.intt[0]);
	b=binary(n.intt[1]);

	strcat(a,b);

	return a;

}
#endif
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Test binary floats and doubles.
 *
 * Description: Test the routines that print floats and doubles in binary.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
void testBinaryFloats()
{ 
  float f=1.2345f;
  double d=1.2345;

#ifndef WIN32
  long long ll;
#endif

  int i;

  for(i=0; i<10; i++){
    f*=(-2);
    printf("binary f float %s %f\n", binaryf(f), f);
  }
  for(i=0; i<10; i++){
    d*=(-2); 
    printf("binary double %s %e\n", binaryd(d), d);
  }

#ifndef WIN32
  ll=1;
  ll= ll<<60;
  ll += 1LL<<50;
  printf("long long i %lli\n", ll);
  printf("long long hex   %llx\n", ll);
  printf("long long octal %llo\n", ll);
  printf("long long binary %s\n", binaryll(ll));
#endif
}

#ifdef SSLIB_SUPPORT_PENDING
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Bit Vector
 * Purpose:    	Test the bit vector implementation.
 *
 * Description: Test the bit vector implementation.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL December 12, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------
void testBitVector()
{
  BitVector odd, even, squares;
  int n=65,i;

  initializeBitVector(&odd, n);
  initializeBitVector(&even, n);
  for(i=0; i<n; i++){
    if(i%2==0){
      setBit(&even, i, 1);
    }else{
      setBit(&odd, i, 1);
    }
  }

  for(i=0; i<n; i++){
    if(getBit(even, i) == getBit(odd,i)){
      printf("%i is both even and odd\n", i);
    }
    if(i%2==0){
      if(!getBit(even,i)){
	printf("%i is not even\n",i);
      }
    }
    if(i%2==1){
      if(!getBit(odd,i)){
	printf("%i is not odd\n",i);
      }
    }
    printf("%i odd: %i, even %i\n", i, getBit(odd,i), getBit(even,i));

  }

  initializeBitVector(&squares, n);
  for(i=0; i<sqrt((double)n); i++){
    setBit(&squares, i*i, 1);
  }
  for(i=0; i<n; i++){
    if(getBit(squares, i)){
      printf("%i is a perfect square\n", i);
    }
  }

  iprint2((int)sizeof(long), (int)sizeof(int));

#ifndef WIN32
  iprint((int)sizeof(long long));
#endif
}
 *--------------------------------------------------------------------------------------------------------------------------------*/

#endif /* SSLIB_SUPPORT_PENDING */


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print a message to the screen that something is different.
 *
 * Description: Print a message to the screen that something is different.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL Septemper 24, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
int different(char *message)
{
	printf("******%s\n", message);
	return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print an error message to stdout.
 *
 * Description: Print an error message to stdout, precfixed by @@@@@.
 *		Does not print the message if we are in summary mode.  Only prints the message if we are in summary mode.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 20, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
void print_error(const char *message)
{
  if( g_printLevel>=PRINT_LEVEL_1){
		printf("@@@@@ %s\n", message);
	}
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print a banner every screenfull of information.
 *
 * Description: This macro uses the global variables g_printed_lines and g_terminal_height.  Every time
 *		we've printed a screenful of array data, print the banner again.  Make sure to set
 *		g_printed_lines=0; at the begining of printing an array.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 14, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
#ifdef USE_BANNER
  #define IF_SCREENFULL_PRINT_BANNER						\
  if(g_printed_lines % g_terminal_height == 0 ){					\
	if( g_printLevel>=PRINT_LEVEL_1){					\
		ADD_STRING_TO_CONTEXT_BUFFER(g_banner);				\
	}									\
	g_printed_lines++;							\
  }	
#else
  #define IF_SCREENFULL_PRINT_BANNER
#endif		

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print in multiple colums for easy comparison, with extra args
 *
 * Description: This is the same as print_string, except that there are additional unused
 * arguments to match the format of other print_ functions.
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
/*ARGSUSED*/
void print_string_temp( t_safdiffPrintLevel a_printLevel,const char *label, const char *left, const char *right,
			int  __UNUSED__ a_dummyLeftIsMissing, int  __UNUSED__ a_dummyRightIsMissing)
{
  print_string( a_printLevel,label, left, right);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	A macro to print an array of arbitrary type to stdout.
 *
 * Description: Prints an array of arbitrary type, by calling the appropriate type-specific print function.
 *		 5 columns of output, 1. lable (name) 2. left value 3. right value
 *		4. difference (right-left)  5. percent change 100*(right-left)/left
 *		Note that this function prints the index in the array in column 1 with the label.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Issues: 	requires int i; and char newname[128]; to be declared prior to calling.
 *
 * Programmer:	Matthew O'Brien, LLNL October 24, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
#define PRINT_TYPE_ARRAY(name, left, right, left_count, right_count, print_type, DUMMY_VALUE)  \
{                                                        \
        int min_count=0;									\
	min_count= (int) MIN(left_count, right_count);						\
	g_printed_lines=0;									\
	for(i=0; i < min_count; i++){                                               		\
		IF_SCREENFULL_PRINT_BANNER							\
		sprintf(newname,"%s[%i]",name,i);                                       	\
		print_type(a_printLevel,newname, left[i], right[i], 0,0);                                 	\
	}                                                                               	\
	for(i=(int)right_count; i < (int)left_count; i++){                                      \
		IF_SCREENFULL_PRINT_BANNER							\
		print_type(a_printLevel,newname, left[i], DUMMY_VALUE, 0, 1);                             	\
	}                                                                               	\
	for(i=(int)left_count; i < (int)right_count; i++){                                      \
		IF_SCREENFULL_PRINT_BANNER							\
		sprintf(newname,"%s[%i]",name,i);                                       	\
		print_type(a_printLevel,newname, DUMMY_VALUE, right[i], 1, 0);                            	\
	}                                                                               	\
}    



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	A macro to print an array of arbitrary numeric type to stdout.
 *
 * Description: Prints an array of arbitrary type, by calling the appropriate type-specific print function.
 *		 5 columns of output, 1. lable (name) 2. left value 3. right value
 *		4. difference (right-left)  5. percent change 100*(right-left)/left.
 *		If g_globalfo.verbosity.brief, (i.e. brief mode), then find the maximum difference and print
 *		only the maximum difference instead of the entire array.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 24, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
#define PRINT_TYPE_ARRAY_numeric(name, left, right, left_count, right_count, print_type, DUMMY_VALUE)  \
{                                                                                                       \
	int i;                                                                                          \
	char newname[128];                                                                              \
	size_t min_count;                                                                               \
	double diff, maxdiff=0, maxreldiff=0, reldiff;                                                  \
                                                                                                        \
	if( g_printLevel>=a_printLevelForAll )                                                                 \
	{                                                                                               \
		PRINT_TYPE_ARRAY(name, left, right, left_count, right_count, print_type, DUMMY_VALUE)  \
	}                                                                                               \
	else {                                 \
		min_count=MIN(left_count, right_count);                                                 \
		for(i=0; i<(int)min_count; i++){                                                        \
			diff = fabs( (double)(right[i]-left[i]) );                                      \
			reldiff=fabs(PER_CHANGE((double)(left[i]),(double)(right[i])));                 \
			if(diff > maxdiff) 		maxdiff=diff;                                   \
			if(reldiff > maxreldiff) 	maxreldiff=reldiff;                             \
		}                                                                                       \
		for(i=0; i<(int)min_count; i++){                                                        \
			diff = fabs( (double)(right[i]-left[i]) );                                      \
			reldiff=fabs(PER_CHANGE((double)(left[i]),(double)(right[i])));                 \
			if(diff == maxdiff){                                                            \
				sprintf(newname,"max diff %s[%i]",name,i);                              \
				print_type(a_printLevel,newname, left[i], right[i], 0,0);                       \
				 break; 					\
			}                                                                               \
			if(reldiff == maxreldiff){                                                      \
				sprintf(newname,"max rel diff %s[%i]",name,i);                          \
				print_type(a_printLevel,newname, left[i], right[i], 0,0);                                 \
				 break; 					\
			}                                                                               \
		}                                                                                       \
	}                                                                                               \
} 


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	A macro to print an array of arbitrary numeric type to stdout.
 *
 * Description: Prints an array of arbitrary type, by calling the appropriate type-specific print function.
 *		 5 columns of output, 1. lable (name) 2. left value 3. right value
 *		4. difference (right-left)  5. percent change 100*(right-left)/left.
 *		You need to know the hid_t of the data so the array indexing can be done correctly.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *
 * Programmer:	Matthew O'Brien, LLNL October 24, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
void print_type_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
		       const char *name, void *leftcoords_void, void *rightcoords_void, size_t leftdim, size_t rightdim, 
		       hid_t lefttype, hid_t righttype)
{
  if (lefttype && righttype) {
    if(H5Tequal(lefttype, SAF_DOUBLE) && H5Tequal(righttype, SAF_DOUBLE)){
      print_double_array(a_printLevel,a_printLevelForAll,name, (double *)leftcoords_void, (double *)rightcoords_void, leftdim, rightdim);
    }
    else if(H5Tequal(lefttype, SAF_FLOAT) && H5Tequal(righttype, SAF_FLOAT)){
      print_float_array(a_printLevel,a_printLevelForAll,name, (float *)leftcoords_void, (float *)rightcoords_void, leftdim, rightdim);
    }
    else if(H5Tequal(lefttype, SAF_INT) && H5Tequal(righttype, SAF_INT)){
      print_int_array(a_printLevel,a_printLevelForAll,name, (int *)leftcoords_void, (int *)rightcoords_void, leftdim, rightdim);
    }
    else if(H5Tequal(lefttype, SAF_LONG) && H5Tequal(righttype, SAF_LONG)){
      print_long_array(a_printLevel,a_printLevelForAll,name, (long *)leftcoords_void, (long *)rightcoords_void, leftdim, rightdim);
    }
    else {
      size_t leftsize, rightsize;
      int k;

      leftsize =  H5Tget_size(lefttype);
      rightsize =  H5Tget_size(righttype);

      PRINT_SOME_STRINGS(a_printLevel, (int)leftdim, (int)rightdim, k, name,
			 StringDataTypeValue(lefttype,  (ss_pers_t*)( (char *)leftcoords_void + k*leftsize )  ),
			 StringDataTypeValue(righttype, (ss_pers_t*)( (char *)rightcoords_void + k*rightsize ) ) 
			 );

     }
   }
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print an array of floats to stdout.
 *
 * Description: Print an array of floats to stdout.  A type specific function that wraps the macro PRINT_TYPE_ARRAY
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 24, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
void print_float_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
			const char *name, float *left, float *right, size_t left_count, size_t right_count)
{
	PRINT_TYPE_ARRAY_numeric(name, left, right, left_count, right_count, print_float, DUMMY_FLOAT);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print an array of doubles to stdout.
 *
 * Description: Print an array of doubles to stdout.  A type specific function that wraps the macro PRINT_TYPE_ARRAY
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 24, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_double_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
			const char *name, double *left, double *right, size_t left_count, size_t right_count)
{
	PRINT_TYPE_ARRAY_numeric(name, left, right, left_count, right_count, print_float, DUMMY_FLOAT);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print an array of ints to stdout.
 *
 * Description: Print an array of ints to stdout.  A type specific function that wraps the macro PRINT_TYPE_ARRAY
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 24, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_int_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
			const char *name, int *left, int *right, size_t left_count, size_t right_count)
{
	PRINT_TYPE_ARRAY_numeric(name, left, right, left_count, right_count, print_int, DUMMY_INT);
}
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print an array of longs to stdout.
 *
 * Description: Print an array of longs to stdout.  A type specific function that wraps the macro PRINT_TYPE_ARRAY
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 24, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_long_array( t_safdiffPrintLevel a_printLevel,  t_safdiffPrintLevel a_printLevelForAll,
			const char *name, long *left, long *right, size_t left_count, size_t right_count)
{
	PRINT_TYPE_ARRAY_numeric(name, left, right, left_count, right_count, print_long, DUMMY_INT);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print an array of strings to stdout.
 *
 * Description: Print an array of strings to stdout.  A type specific function that wraps the macro PRINT_TYPE_ARRAY
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 24, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_string_array( t_safdiffPrintLevel a_printLevel,const char *name, char **left, char **right, size_t left_count, size_t right_count)
{
	int i;                                                                                  
	char newname[128];                                                                      

	PRINT_TYPE_ARRAY(name, left, right, left_count, right_count, print_string_temp, DUMMY_STRING);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print in multiple colums for easy comparison
 *
 * Description: Print stings, 5 columns of output, 1. lable (name)  2. left value  3. right value
 *		4. same or different  5. same or different
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL Septemper 24, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_string( t_safdiffPrintLevel a_printLevel,const char *label, const char *left, const char *right)
{
  char differ[16];
  int stringcmp;

  if( left!=NULL && right!=NULL )
    stringcmp = ( strcmp(left,right) ? SAFDIFF_DIFFERENT : SAFDIFF_SAME );
  else if( left==NULL && right==NULL )
    stringcmp=SAFDIFF_SAME;
  else
    stringcmp=SAFDIFF_DIFFERENT;

  strcpy(differ, stringcmp ? "different" : "same" );

  if(g_diffwidth<10 || g_perwidth<10) differ[4]='\0';/*change "different" to diff if the column is too narrow*/

  /*XXX todo: make this neat*/
  {
    int l_leftLen = (int)strlen(left);
    int l_rightLen = (int)strlen(right);

    if( l_leftLen<=g_valwidth && l_rightLen<=g_valwidth )
      { /*plenty of room to print as is*/
	if( g_printLevel>=a_printLevel || stringcmp)
	  {
	    PRE_PRINT;
	    printf(g_formatString.m_string, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH, label, left, right, differ, differ);
	    IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(stringcmp);
	  }
	else if(g_hidden_context_buffer[1][0]=='\0' && g_just_reset_hidden_context_buffer ) 
	  {
	    sprintf(g_hidden_context_buffer[1],g_formatString.m_string, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH, 
		    label, left, right, differ, differ );  
	  }
      } 
    else
      {
	/*HACK TO SQUEEZE A BIT MORE OUT OF THE LINE: print just one "same" or "diff",
	 and squeeze it to the end of the line*/
	static char l_newfmt[256]="";
	int l_extraAvailable = g_diffwidth + g_perwidth - 4 +2;
	int l_extraNeededLeft = l_leftLen - g_valwidth;
	int l_extraLeft = MIN(l_extraAvailable,l_extraNeededLeft);
	l_extraAvailable -= l_extraLeft;
	differ[4]='\0';/*change "different" to "diff"*/

	sprintf(l_newfmt, "%%-%i.%is %%%is %%%is %%%is\n", DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH,
		g_valwidth+l_extraLeft,g_valwidth+l_extraAvailable, 4 );

	if( g_printLevel>=a_printLevel || stringcmp)
	  {
	    PRE_PRINT;
	    printf(l_newfmt, label, left, right, differ);
	    IF_SUMMARY_MODE_AND_DIFFERENT_THEN_EXIT(stringcmp);
	  }
	else if(g_hidden_context_buffer[1][0]=='\0' && g_just_reset_hidden_context_buffer ) 
	  {
	    sprintf(g_hidden_context_buffer[1],l_newfmt, label, left, right, differ );  
	  }
      }
  }

  g_just_reset_hidden_context_buffer=0;    
}


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print Two Strings Without Comparing
 *
 * Description: Prints two strings, noting if they are different, but without causing safdiff to return
 *              a difference. This is used for comparing items like the MPI version, which  do not affect 
 *              whether the databases are fundamentally different.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void print_string_ignore( t_safdiffPrintLevel a_printLevel,const char *label, const char *left, const char *right)
{
  char differ[16];
  int stringcmp;

  if( left!=NULL && right!=NULL )
    {
      stringcmp = ( strcmp(left,right) ? SAFDIFF_DIFFERENT : SAFDIFF_SAME );
    }
  else if( left==NULL && right==NULL )
    {
      stringcmp=SAFDIFF_SAME;
    }
  else
    {
      stringcmp=SAFDIFF_DIFFERENT;
    }

  strcpy(differ, stringcmp ? "ignored" : "same" );

  if( g_printLevel>=a_printLevel || stringcmp){
    PRE_PRINT
      printf(g_formatString.m_string, DYNAMIC_NAME_WIDTH,DYNAMIC_NAME_WIDTH, label, left, right, differ, differ);
  }
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print 5 column headings.
 *
 * Description: Print 5 column headings, Object, Left saf database, Right saf database, difference, %Change
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL Septemper 24, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_heading( t_safdiffPrintLevel a_printLevel, const char *leftname, const char *rightname)
{
  if( g_printLevel>=a_printLevel)
    {
      char heading[1024];
      sprintf(heading,g_formatString.m_heading, g_namewidth,g_namewidth, "Object",  
	      leftname, rightname, "abs. diff", "rel. diff");
      ADD_STRING_TO_CONTEXT_BUFFER(heading);
      print_break(a_printLevel,"");
    }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print a label followed by a dashed line.
 *
 * Description: Print a label followed by a dashed line, so that the total length of the line is constant.
 *		We start with a really long string of dashed and control the length by setting one dash to '\0'
 *		to terminate the string. Also adds the string to the context buffer, see ADD_STRING_TO_CONTEXT_BUFFER.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL November 20, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_break( t_safdiffPrintLevel a_printLevel,const char *message)
{
  char dashes[]=	
    "------------------------------------------------------------------------------------------------------"
    "------------------------------------------------------------------------------------------------------"
    "------------------------------------------------------------------------------------------------------"
    "------------------------------------------------------------------------------------------------------";
  int len;
  char heading[1024];
  len = g_totalwidth - (int)strlen(message);
  len=MAX(len,0);
  dashes[len]='\0';
  sprintf(heading, "%s%s\n", message, dashes);
  if( g_printLevel>=a_printLevel )
    {
      ADD_STRING_TO_CONTEXT_BUFFER(heading);
      g_hidden_context_buffer[0][0]='\0';
      g_hidden_context_buffer[1][0]='\0';
      g_just_reset_hidden_context_buffer=0; 
    }
  else
    {
      strcpy(g_hidden_context_buffer[0],heading);
      g_hidden_context_buffer[1][0]='\0';
      g_just_reset_hidden_context_buffer=1;
    }
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Print a Break Without Adding to the Buffer
 *
 * Description: See print_break.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void print_break_no_buffer( t_safdiffPrintLevel a_printLevel,const char *message)
{
  if( g_printLevel>=a_printLevel )
    {
      char dashes[]=	
	"------------------------------------------------------------------------------------------------------"
	"------------------------------------------------------------------------------------------------------"
	"------------------------------------------------------------------------------------------------------"
	"------------------------------------------------------------------------------------------------------";
      int len;
      len = g_totalwidth - (int)strlen(message);
      len=MAX(len,0);
      dashes[len]='\0';
      printf("%s%s\n", message, dashes);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Utilities
 * Purpose:    	Given two arrays of integers, return 1 if they are permutations of each other, 0 if not.
 *
 * Description: Given two arrays of integers, return 1 or 0 if the two lists are or are not permutations of each other, repectively.
 *		This algorithm runs in time Theta(Max{count,max-min+1}) and requires space Theta(max-min+1).
 *
 * Issues:	The space requirement could be dangerous if the range of the numbers is too wide.
 *
 * Parallel:    Parallel and serial same behavior.
 *
 * Programmer:	Matthew O'Brien, LLNL October 29, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int arePermutations(int *a, int *b, size_t count)
{
  size_t i;
  int mina, maxa, minb, maxb;
  int *presenta, *presentb, ret;

  mina=a[0];	maxa=a[0];
  minb=b[0];	maxb=b[0];

  for(i=0; i<count; i++){
    if(a[i]<mina) mina=a[i];
    if(b[i]<minb) minb=b[i];
    if(a[i]>maxa) maxa=a[i];
    if(b[i]>maxb) maxb=b[i];
  }

  if(mina!=minb||maxa!=maxb) return 0;

  presenta=(int *)calloc( (unsigned int)(maxa-mina+1), sizeof(int) );
  presentb=(int *)calloc( (unsigned int)(maxa-mina+1), sizeof(int) );

  for(i=0; i<count; i++){
    presenta[ a[i]-mina ]++;
    presentb[ b[i]-minb ]++;
  }
	
  ret = memcmp(presenta, presentb, (maxa-mina+1)*sizeof(int));

  free(presenta);
  free(presentb);
	
  return ret ? 0 : 1;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Utilities
 * Purpose:    	Test the function arePermutations
 *
 * Description: Test the function arePermutations
 *
 * Issues:	The space requirement could be dangerous if the range of the numbers is too wide.
 *
 * Parallel:    Parallel and serial same behavior.
 *
 * Programmer:	Matthew O'Brien, LLNL October 29, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void test_arePermutations()
{
  int *a, *b,r;
  size_t i, n;

  n=8;
  a=(int *)malloc(n*sizeof(int));
  b=(int *)malloc(n*sizeof(int));

  for(i=0; i<n; i++){

#ifdef WIN32 
    a[i]=(int)rand()&2047;
#else
    a[i]=(int)random()&2047;
#endif

    b[i]= a[i];
    /*b[i]= (int)random()&3;*/
  }

  for(i=0; i<n; i++){

#ifdef WIN32 
    r=(int)rand()&(n-1);
#else
    r=(int)random()&(n-1);
#endif

    SWAP( b[i], b[r], int );
  }

  print_int_array(PRINT_LEVEL_3,PRINT_LEVEL_3,"perm", a,b,n,n);
  printf("arePermutations %i\n", arePermutations(a,b,n));
}






/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Command Line Arguments
 * Purpose:    	Convient macro for command line arguements.
 *
 * Description: You pass the macro the short form (ie -A), the long form (ie --absolute=), what to do with the g_value_string
 *		and whether or not the argument is a flag only, ie (-h, --help don't take arguments).
 *
 * Parallel:    
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
#define ARG_MATCH(shortform,longform, what_to_do,flag_only)                          		\
	if (!strcmp(argv[i], shortform)||strstr(argv[i],longform) ) {    			\
                int flag_only_variable=flag_only;                                               \
		if(!strcmp(argv[i], shortform)){                           			\
			i++;                                               			\
			if(i<argc && !flag_only_variable) strcpy(g_value_string,argv[i]);	\
		}else{                                                     			\
			char *p;                                           			\
			p=strchr(argv[i],'=');                             			\
			if(p!=NULL){                                       			\
				p++;                                       			\
				strcpy(g_value_string,p); 					\
			}                                                  			\
			else{									\
				if(!flag_only_variable){					\
					printf("You must not use any spaces in the long argument form, --longform=value\n");	\
					exit_safdiff(SAFDIFF_ERROR);					\
				}								\
			}									\
		}                                                          			\
		what_to_do;                                                			\
		g_matched_arg=1;                                             			\
	}                                                                  

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Help
 * Purpose:    	Helper function to print command line flags followed by the description.
 *
 * Description: Helper function to print command line flags followed by the description.
 *
 * Parallel:    
 *
 * Programmer:	Matthew O'Brien, LLNL November 28, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_help(const char *switches, const char *description)
{
  printf("%s\n", switches);
  printf("%s\n\n", description);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Help
 * Purpose:    	Print usage message, synopsis.
 *
 * Description: Print the usage message, the synopsis.  This function is called if you type in safdiff - h or safdiff --help.
 *
 * Parallel:    
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void print_usageMessage()
{

  out_init_size();

  printf("safdiff - SAF file differencing tool\n\n");
  printf("VERSION:   SAF-%d.%d.%d%s%s, HDF5-%d.%d.%d%s%s\n\n",
	 SAF_VERSION_MAJOR,SAF_VERSION_MINOR,SAF_VERSION_RELEASE,
	 (strlen(SAF_VERSION_ANNOT) ? "-":""),SAF_VERSION_ANNOT,
	 H5_VERS_MAJOR,H5_VERS_MINOR,H5_VERS_RELEASE, 
	 (strlen(H5_VERS_SUBRELEASE) ? "-":""),H5_VERS_SUBRELEASE );


  printf("USAGE: BASIC OPTIONS\n"
	 "\n"
	 "safdiff [-v 0|1|2|3|4|5] FILE1 FILE2\n"
	 "\n"
	 );

  printf("   -v V, --verbosity V    Control the verbosity of the output\n"
	 "         0: quiet mode, print one line only\n"
	 "         1: DEFAULT - print general info and the first difference\n"
	 "         2: print general info and all differences\n"
	 "         3: print more detail\n"
	 "         4: print much more detail\n"
	 "         5: print everything\n" 
	 "\n"
	 );


  printf("USAGE: ALL OPTIONS\n"
	 "\n"
	 "safdiff [-A TOL] [-R TOL] [-p strict|any]\n"
	 "\t[-F TOL]\n"
	 "\t[-ao and|or] [-t] [-w width]\n"
	 "\t[-m PRECISION]\n"
#ifdef USE_BANNER
	 "\t[-y HEIGHT]\n"
#endif
	 "\t[-tt TOL] [-o 16|10|8|2]\n"
	 "\t[-v 0|1|2|3|4|5]\n"
	 "\t[-s {diffs,rel_diff,map,left,right,ave,exclusive}]\n"
	 "\t[-i ave|left|right]\n"
	 "\t[-f FILE] [-h]\n"
	 "\t[-l OBJ -r OBJ]\n"
	 "\t[-lt INDEX | -lv TIME]\n"
	 "\t[-rt INDEX | -rv TIME]\n"
	 "\t[-e field_name:absolute=A:relative=R:floor=F:absrel_andor=and:store=LIST:verbosity=LIST]\n"
	 "\t[-nd FILE3] [-od]\n"
	 "\t[-x]\n"
	 "\t-ld FILE1 -rd FILE2 | FILE1 FILE2\n"
	 "\n"
	 "Or the equivalent long form:\n"
	 "\n"
	 "safdiff [--absolute=TOL] [--relative=TOL] [--order=strict|any]\n"
	 "\t[--floor=TOL]\n"
	 "\t[--absrel_andor=and|or] [--drawtree] [--width=N]\n"
	 "\t[--precision=N]\n"
#ifdef USE_BANNER
	 "\t[--height=N]\n"
#endif
	 "\t[--timetol=TOL] [--obase=16|10|8|2]\n"
	 "\t[--verbosity=0|1|2|3|4|5]\n"
	 "\t[--store={diffs,rel_diff,map,left,right,ave,exclusive}]\n"
	 "\t[--coord_field=ave|left|right]\n"
	 "\t[--help]\n"
	 "\t[--leftobj=OBJ --righobj=OBJ]\n"
	 "\t[--lefttimeindex=INDEX | --lefttimevalue=TIME]\n"
	 "\t[--righttimeindex=INDEX | --righttimevalue=TIME]\n"
	 "\t[--field=field_name:absolute=A:relative=R:floor=F:absrel_andor=and:store=LIST:verbosity=V]\n"
	 "\t[--newdb=FILE3] [--overwritenewdb]\n"
	 "\t[--exclusive]\n"
	 "\t--leftdb=FILE1 --rightdb=FILE2 | FILE1 FILE2\n"
	 "\n"	);

  printf("DESCRIPTION:\n\n");
  /*printf("safdiff prints the differences between two saf databases to stdout.\n"
    "It also has the ability to create a third saf database that contains\n"
    "selected differences or similarities between the first two.\n"
    "The command line arguments are classified as follows:\n"
    "\n" "\n"
    "Controlling what you are differencing: \n"
    "\n"
    "This is where you specify the file names of what to difference, \n"
    "or if you want to difference one time step against another, or if\n"
    "you want to only difference a particular SAF object within the database\n"
    "(the default action is to difference everything in the database).\n"
    "\n" "\n"
    "Controlling what is considered a difference:\n"
    "\n"
    "This is where you can set absolute, relative and noise floor tolerances.  \n"
    "You can also specify whether to compare objects based on the position \n"
    "they are returned in a list, or you can tell safdiff to try to match \n"
    "up corresponding objects, comparing objects that have the same name.\n"
    "\n" "\n"
    "Controlling ASCII output:\n"
    "\n"
    "You can specify the output base, hexidecimal, decimal, octal or binary.  \n"
    "You can also override the detected terminal height and width and set the \n"
    "floating point output precision.  You can also turn on and off the \"tree style\" \n"
    "output.  You can also specify the degree of verbosity of the output \n"
    "as an integer from 0 through 5, all explained below.\n"
    "\n" "\n"
    "Controlling the SAF database output:\n"
    "\n"
    "A third database can be created, with the same structure as the left \n"
    "database, but with additional fields written to this database: \n"
    "diffs, rel_diffs, map, left, right, ave.  These options are explained below.\n"
    "\n" "\n"
    "safdiff has a configuration file that contains your default options, these options\n"
    "can be overwritten on the command line.  safdiff first looks for ./safdiff.config,\n"
    "then for ~/safdiff.config.  You can specify any configuration file by\n"
    "safdiff -f my_config_file\n"
    "\n");*/



  printf("Control what is being compared:\n\n");

  printf("-ld FILE, --leftdb=FILE       Specify the filename for the left database\n");
  printf("-rd FILE, --rightdb=FILE      Specify the filename for the right database\n");
  printf("    Note: you can specify the left and right databases with no flags as the last\n");
  printf("          two arguments, for instance: \"safdiff -v 3 -t leftname.saf rightname.saf\"\n\n");

  printf("-lt N, --lefttimeindex=N       Only consider this left time index\n");
  printf("-rt N, --righttimeindex=N      Only consider this right time index\n");
  printf("-lv V, --lefttimevalue=N       Only consider this left time value\n");
  printf("-rv V, --righttimevalue=N      Only consider this right time value\n");
  printf("    Note: you cannot specify both a lefttimeindex and a lefttimevalue, or both\n");
  printf("          a righttimeindex and a righttimevalue\n\n");

  printf("-l obj, --leftobj=obj    Limit the comparison to a single left object\n");
  printf("-r obj, --rightobj=obj   Limit the comparison to a single right object\n");
  printf("    Note: currently the only type of saf objects that can be specified are suites\n");
  printf("          fields, field templates, sets and categories.\n");
  printf("    Note: specifying a set will cause the subset and topology relations defined on\n");
  printf("          that set to be output.\n");

  printf("-f FILE   Specify the configuration file for safdiff. If this file is\n");
  printf("          found, then safdiff will parse the contents for command-line options,\n");
  printf("          and add them to any options that are on the actual command-line\n");
  printf("          Note: the default is ./safdiff.config, then ~/safdiff.config\n\n");

  printf("Control what is considered a difference:\n\n");

  printf("-tt TOL, --timetol=TOL       Time value coordinate absolute tolerance\n\n");

  printf("-A TOL, --absolute=TOL    Absolute tolerance: values are different if |right-left| > TOL\n");
  printf("-R TOL, --relative=TOL    Relative tolerance, values are different if |right-left|/left > TOL\n");
  printf("-ao and|or, --absrel_andor=and|or     Control whether to use abs or rel tolerance, or both\n");
  printf("     and: different if outside absolute threshold *and* relative threshold\n");
  printf("     or: different if outside absolute threshold *or* relative threshold\n\n");

  printf("-F TOL, --floor=TOL       Noise floor tolerance: if both the left and the right values\n");
  printf("                          have magnitude less than TOL, they will be considered the same\n\n");

  printf("-p any|strict, --order=any|strict    Allow any order or force same order\n");
  printf("     any: when comparing two lists of saf objects, match objects by name\n");
  printf("     strict: when comparing two lists of saf objects, match objects by their position in the list\n\n");

  printf("Control ASCII output:\n\n");
	
  printf("-o N, --obase=N   Set the output base to N, N must be one of 2,8,10 or 16\n\n");

  printf("-v V, --verbosity=V    Control the verbosity of the output\n"
	 "      0: quiet mode, print one line only\n"
	 "      1: DEFAULT - print general info and the first difference\n"
	 "      2: print general info and all differences\n"
	 "      3: print more detail\n"
	 "      4: print much more detail\n"
	 "      5: print everything\n\n" );

  printf("-w N, --width=N    set the column width of the output\n");

#ifdef USE_BANNER
  printf("-y N, --height=N   Set the terminal height, in number of lines.\n");
  printf("     Note: when printing arrays that span serveral screenfulls, safdiff prints\n");
  printf("           a heading every N lines, where N is your actual terminal height in lines,\n");
  printf("           or the value that is set with this parameter\n\n");
#endif

  printf("-m N, --precision=N   Set the output precision, or the # of digits after the decimal\n\n");

  printf("-t, --drawtree        Tree style output\n\n");



  printf("-x, --exclusive      Refrain from printing data that is found in one\n");
  printf("                     dataset but not the other\n\n");



  printf("What is written to a new saf database that contains the differences:\n\n");
	
  printf("-nd FILE3, --newdb=FILE3   Set the name of the new database\n");

  printf("-od, --overwritenewdb      If this flag is set, then the output file specified by --newdb=FILE3\n");
  printf("                           be overwritten if it already exists. Otherwise, FILE3 will not be\n");
  printf("                           overwritten. Instead the differences will be written to FILE3_n\n\n");

  printf("-s LIST, --store=LIST    What to store in the output database.\n"
	 "         LIST must be a comma separated list (no spaces) of the following:\n"
	 "              diffs:      Store the differences (Right-Left) for each field\n"
	 "              rel_diff:   Store the relative differences 100*(Right-Left)/Left for each field\n"
	 "              map:        Store a 0/1 threshold map that is 1 if the differences of the\n"
	 "                          Left and Right field values are outside of the threshold\n"
	 "                          and 0 if the difference is within the threshold.\n"
	 "              left:       Copy the left field values to the new database.\n"
	 "              right:      Copy the right field values to the new database.\n"
	 "              ave:        Store the average of the left and right field values to the new database.\n\n"
	 );

  printf("-i ave|left|right, --coord_field=ave|left|right       Choose default coordinates for output database.\n" 
	 "                          left:  use the left values as coordinates\n"
	 "                          right: use the right values as coordinates\n"
	 "                          ave:   take the average of the left and right values as coordinates\n\n");




  printf("Field-wise Options:\n\n");

  printf("-e FIELD_OPTIONS, --field=FIELD_OPTIONS    Override global parameters with field-specific values\n"
	 "             Specify FIELD_OPTIONS using a colon (:) to separate each parameter=value pair.\n"
	 "             FIELD_OPTIONS should look something like this:\n"
	 "             field_name:absolute=A:relative=R:floor=F:absrel_andor=and:store=LIST:verbosity=V\n"
	 "             Everything before the first colon (:) is taken to be the field name. For fields not\n"
	 "             specified like this, the global values for the parameters will be used.\n\n");



  printf("EXIT VALUE:\n\n");
  printf(    "   0: No difference within the given thresholds\n"
	     "   1: There exists a difference withing the given thresholds\n"
	     "  -1: Encountered an error\n\n");

  printf("EXAMPLES:\n\n");

  printf("Show output in all cases where the relative difference exceeds 1%%\n"
	 "   safdiff -R 1 -v 2 Left.saf Right.saf\n");
  printf("Show only the maximum difference for fields, relative difference must exceed 1%%\n"
	 "   safdiff -R 1 -v 4 Left.saf Right.saf\n");
  printf("Show all values, even if they are the same\n"
	 "   safdiff -v 5 Left.saf Right.saf\n");
  printf("Display the contents of a single database\n"
	 "   safdiff -v 5 Left.saf Left.saf\n");
  printf("Difference only the field \"temperature\", show output upon any differences\n"
	 "   safdiff --leftobj=temperature --rightobj=temperature Left.saf Right.saf\n");
  printf("Compare time step 0 with time step 1, in the same database\n"
	 "   safdiff --lefttimeindex=0 --righttimeindex=1 Left.saf Left.saf\n");
  printf("Store the field differences in a new SAF database, differences.saf\n"
	 "   safdiff --store=diffs --newdb=differences.saf Left.saf Right.saf\n");
  printf("Store the relative field differences and the 0/1 threshold map in a new database\n"
	 "   safdiff --store=rel_diffs,map --newdb=differences.saf Left.saf Right.saf\n\n");


  exit(0);
}




/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Command Line Arguments
 * Purpose:    	Allow each field to have its own tolerances and parameters for what to store.
 *
 * Description: Build arg and argv from the value_string, argc and argv can then be passed
 *              to processCommandArguments so that the fieldOptions stucture field_options
 *              can be populated with the users requests for that field.  Then those
 *              field_options are inserted into the global hash table g_fieldOptionsHash
 *              so that whenever we encounter that field, we can set the tolerances
 *              to that fields specific tolerances.
 *
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL January 28, 2002
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void parseFieldTolerance(int second_time, char *value_string)
{

  fieldOptions *field_options;
  char field_name[128];
  int buildargc=1;
  char *buildargv[64], *p;
  const char DELIMITER=':';
	
  if(second_time){
    field_options = (fieldOptions *)malloc(sizeof(fieldOptions));
	
    *field_options=g_globalfo; /*The second time through we know all the global parameters have been set*/
	
    if(g_debugging){sprint(value_string); }
    buildargv[0]=safdiff_strdup("safdiff");
	
    /*turn value_string that looks like "fieldname;absolute=A;relative=B;floor=C" into
      field_name="fieldname",
      argv[1]="--absolute=A", argv[2]="--relative=B", argv[3]="--floor=C"*/
    /*everything before the first semi-colon is the field name*/

    p=strchr(value_string, DELIMITER);
    if(p){
      *p='\0';
		
      strcpy(field_name, value_string); 
      p++;
      value_string=p;
      while( (p=strchr(value_string, DELIMITER)) || ( strlen(value_string)>0 )   ){
	if(p) *p='\0';
	buildargv[buildargc]=(char *)malloc( (strlen(value_string)+2)*sizeof(char) );
	strcpy(buildargv[buildargc],"--");
	strcat(buildargv[buildargc], value_string);
	buildargc++;
	if(p){
	  p++;
	  value_string=p;
	}else{
	  value_string[0]='\0';
	}
			
      }


      /*populate field_options with the info in buildargc and buildargv*/
      processCommandArguments(buildargc, buildargv, field_options,0);
      if(g_debugging){ 
	int i;
	for(i=0; i<buildargc; i++){
	  sprint(buildargv[i]);
	}
	iprint(g_globalfo.store.anything); 
      }
      g_globalfo.store.anything = (  (g_globalfo.store.anything)||(field_options->store.anything)  );
      _saf_htab_insert(g_fieldOptionsHash, _saf_hkey_str(safdiff_strdup(field_name)), (ss_pers_t*)field_options);
    }
  }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Command Line Arguments
 * Purpose:    	Process the command line arguments.
 *
 * Description: An array char **argv, and int argc are assembled after reading the configuration file, and then passed to this
 *		function.  Then the real command arguments are passed to this function so they can overwrite what's in the file.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void processCommandArguments(int argc, char **argv, fieldOptions *field_options, int second_time)
{
  int i=1;

  while (i < argc)
    {
      g_matched_arg=0;


      ARG_MATCH("-f","--configfile=", g_matched_arg=1,0);
      ARG_MATCH("-A","--absolute=", field_options->abs_tol=atof(g_value_string),0);
      ARG_MATCH("-R","--relative=", field_options->rel_tol=atof(g_value_string),0);
      ARG_MATCH("-ao","--absrel_andor=", field_options->absrel_andor = strcmp(g_value_string,"and") ? 0 : 1,0 );
      ARG_MATCH("-F","--floor=", field_options->floor_tol=atof(g_value_string),0);

      ARG_MATCH("-p","--order=", g_strictorder= strcmp(g_value_string,"strict") ? 0 : 1,0 );
      ARG_MATCH("-tt","--timetol=", g_time_value_abs_tol=atof(g_value_string),0);

      ARG_MATCH("-d","--debugging=", g_debugging=atoi(g_value_string),0);/*undocumented debug option*/
      ARG_MATCH("-t","--drawtree", g_drawtree=1,1);

      ARG_MATCH("-ld","--leftdb=", strcpy(g_leftdbname,g_value_string),0);
      ARG_MATCH("-rd","--rightdb=", strcpy(g_rightdbname,g_value_string),0);

      ARG_MATCH("-nd","--newdb=", strcpy(g_newdbname,g_value_string),0);
      ARG_MATCH("-od","--overwritenewdb", g_overwritenewdb=1,1);

      ARG_MATCH("-lt","--lefttimeindex=", g_lefttimeindex=atoi(g_value_string),0);
      ARG_MATCH("-rt","--righttimeindex=", g_righttimeindex=atoi(g_value_string),0);



      ARG_MATCH("-lv","--lefttimevalue=", g_gotLeftTimeValue=1;g_lefttimevalue=atof(g_value_string) ,0); 
      ARG_MATCH("-rv","--righttimevalue=", g_gotRightTimeValue=1;g_righttimevalue=atof(g_value_string) ,0); 




      ARG_MATCH("-l","--leftobj", strcpy(g_leftobjname,g_value_string),0);
      ARG_MATCH("-r","--rightobj", strcpy(g_rightobjname,g_value_string),0);

      ARG_MATCH("-o","--obase=", field_options->obase=atoi(g_value_string),0);
      ARG_MATCH("-i","--coord_field=", strcpy(g_coord_field_string,g_value_string),0);
      ARG_MATCH("-h","--help", print_usageMessage(),1);
      ARG_MATCH("-w","--width=", g_columnwidth=atoi(g_value_string),0);
      ARG_MATCH("-m","--precision=", g_precision=atoi(g_value_string),0);

#ifdef USE_BANNER
      ARG_MATCH("-y","--height=", g_terminal_height=atoi(g_value_string),0);
#endif

      ARG_MATCH("-e","--field=", 
		parseFieldTolerance(second_time, g_value_string),0);
			

      ARG_MATCH("-v","--verbosity=", g_printLevel= atoi(g_value_string),0); 
      if(g_printLevel<=PRINT_LEVEL_0) g_printLevel=PRINT_LEVEL_0;
      else if(g_printLevel>PRINT_LEVEL_5) g_printLevel=PRINT_LEVEL_5;


      ARG_MATCH("-x","--exclusive", g_globalfo.exclusive=0,1); 


      ARG_MATCH("-expectFileDifferences","--expectFileDifferences", g_expectFileDifferences=1,1); /*undocumented option for testing*/


      ARG_MATCH("-s","--store=", 
      {
	field_options->store.diffs     = strstr(g_value_string,"diffs") ? 1 : 0;
	field_options->store.rel_diff  = strstr(g_value_string,"rel_diff") ? 1 : 0;
	field_options->store.map       = strstr(g_value_string,"map") ? 1 : 0;
	field_options->store.left      = strstr(g_value_string,"left") ? 1 : 0;
	field_options->store.right     = strstr(g_value_string,"right") ? 1 : 0;
	field_options->store.ave       = strstr(g_value_string,"ave") ? 1 : 0;
	field_options->store.exclusive = strstr(g_value_string,"exclusive") ? 1 : 0;
      }
		,0);

      if(g_matched_arg==0 && i < argc - 2){
	printf("unknown command line or configuration file parameter %s. Use %s --help for options.\n", argv[i], argv[0]);
      }

      i++;
    }/*while(i < argc)*/




  if(g_gotLeftTimeValue && g_lefttimeindex >= 0)
    {
      printf("You cannot specify both a lefttimeindex and a lefttimevalue, you must specify one or the other\n");
      printf("lefttimeindex %i\n", g_lefttimeindex);
      printf("lefttimealue %f\n", g_lefttimevalue);
      exit_safdiff(SAFDIFF_ERROR);
    }
  if(g_gotRightTimeValue && g_righttimeindex >= 0)
    {
      printf("You cannot specify both a righttimeindex and a righttimevalue, you must specify one or the other\n");
      printf("righttimeindex %i\n", g_righttimeindex);
      printf("righttimealue %f\n", g_righttimevalue);
      exit_safdiff(SAFDIFF_ERROR);
    }



  field_options->store.anything=(field_options->store.anything || field_options->store.diffs || 
				 field_options->store.rel_diff || field_options->store.map || 
				 field_options->store.left || field_options->store.right || 
				 field_options->store.ave || field_options->store.exclusive);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Utilities
 * Purpose:    	Return 1 if a string is entirely white space, 0 if not.
 *
 * Description: Return 1 if a string is entirely white space, 0 if not.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL December 12, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int white_space(char *line)
{
  int i, len;
  len=(int)strlen(line);
  for(i=0; i<len; i++){
    switch(line[i]){
    case ' ':
    case '\t':
      {
	break;
      }
    default:
      {
	return 0;
      }
    }
  }
  return 1;

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Command Line Arguments
 * Purpose:    	Read the configuration file.
 *
 * Description: An array char **argv, and int argc are assembled after reading the configuration file (in this function), 
 *		and then passed to processCommandLineArguments.  This is so the the options in the config file can be
 *		overwritten by the command line arguments.  The file must be of the form
 *		commandlineoption=value, where commandlineoption is a command line option of the form
 *		--commandlineoption=value.  # is the comment character for the config file.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 12, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void  readConfigurationFile()
{
  int maxline,len;
  FILE *fp;
  char *lval, *rval, *line;
  char *lp, *rp;
  struct stat buf1;

  char *argv[1024];
  int argc=1;
  int line_number=0;


  if(stat(g_configfile,&buf1) >= 0){
    fp = fopen(g_configfile, "r");
  }else if( stat("~/safdiff.config",&buf1) >= 0 ){
    strcpy(g_configfile, "~/safdiff.config");
    fp = fopen(g_configfile, "r");
  }else{
    /*
    printf("ERROR: Could not find a configuration file\n"
	   "You can specify a configuration file by ``safdiff -f my_config_file ...''\n"
	   "By default, safdiff looks for ./safdiff.config and then ~/safdiff.config to use as configuration files\n");
    */
    return;
  }
  if(!fp) return;

  printf("safdiff is using config file %s\n",g_configfile);

  maxline=1024;
  lval = (char *)malloc(maxline*sizeof(char));
  rval = (char *)malloc(maxline*sizeof(char));
  line = (char *)malloc(maxline*sizeof(char));

  while( fgets(line, maxline, fp) != NULL){

    line_number++;
    line[strlen(line)-1]='\0';
    lp=strchr(line,'#');
    if(lp!=NULL){
      *lp='\0';
    }
    if(strlen(line)>0 && !white_space(line)){


#if 1
      rp=strchr(line, '=');

      if(rp==NULL){
	printf("Missing an equal sign (=), in configuration file.  Line %i:  %s\n",line_number,line);
	exit_safdiff(SAFDIFF_ERROR);
      }
      *rp='\0';
      sscanf(line, "%s", lval);
      rp++;
      sscanf(rp, "%s", rval);
      /*		sprint2(lval, rval);*/

#else
      sscanf(line, "%s = %s\n", lval, rval);
      printf("%s\n",line);
      printf("lval = _%s_, rval= _%s_\n",lval, rval);
	
      if(rp=strchr(lval, '=') ){
	*rp='\0';
	rp++;
	sscanf(rp, "%s", rval);
	printf("rval _%s_\n", rval);
      }
#endif

      if( strlen(rval) > 0 ){

	sprintf(line,"--%s=%s", lval, rval);
	len=(int)strlen(line)+1;
	argv[argc]=(char *)malloc(len*sizeof(char));
	strcpy(argv[argc],line);
	/*			printf("%s\n",argv[argc]);*/
	argc++;
      }
    }

  }/*while*/


  argv[argc]=(char *)malloc(1024*sizeof(char));
  strcpy(argv[argc], strlen(g_leftdbname) ? g_leftdbname : "Left.saf");
  argc++;

  argv[argc]=(char *)malloc(1024*sizeof(char));
  strcpy(argv[argc], strlen(g_rightdbname) ? g_rightdbname : "Right.saf");
  argc++;


  argv[0]=(char *)malloc(32*sizeof(char));
  strcpy(argv[0], "safdiff");
  /*we call this twice because we need *all* of the global variables to be set correctly
   *before* we deal with the per field parameters that overwrite the global variables*/
  processCommandArguments(argc, argv, &g_globalfo,0);
  processCommandArguments(argc, argv, &g_globalfo,1);

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 *
 * Description: The general structure of a function compare_xxx is that it calls saf_describe_xxx
 *		on the left and right argument, and then prints any of the primitive datatypes
 *		that the describe returned.  If the describe returns other saf objects of type SAF_yyy, then
 *		compare_yyy will be called on those objects.
 *
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/




/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Print out the value of two hid_ts.
 *
 * Description: Print out strings that represent hid_ts.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 19, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_hid_ts(hid_t lefttype, hid_t righttype, int a_leftMissing, int a_rightMissing)
{
  print_string((t_safdiffPrintLevel) PRINT_LEVEL_3,"data type", 
	       (a_leftMissing || lefttype <= 0) ? "":StringDataType(lefttype), 
	       (a_rightMissing || righttype <= 0) ? "":StringDataType(righttype) );
  return 0;	
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Print out the value of two SAF_Interleaves.
 *
 * Description: Print out strings that represent SAF_Interleave.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 19, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_interleave(SAF_Interleave *leftcomp_intlv, SAF_Interleave *rightcomp_intlv, int a_leftMissing, int a_rightMissing)
{
  print_string((t_safdiffPrintLevel) PRINT_LEVEL_3,"interleave",  
	       a_leftMissing ? "":str_SAF_Interleave(*leftcomp_intlv),  
	       a_rightMissing? "":str_SAF_Interleave(*rightcomp_intlv));
  return 0;
}


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void compare_time_values( ) 
{
  int i;

  g_leftNumTimesteps = get_number_of_timesteps(g_leftdb,&g_leftTimeValues);
  g_rightNumTimesteps = get_number_of_timesteps(g_rightdb,&g_rightTimeValues);


  if( g_gotLeftTimeValue )
    { /*find the corresponding time index for a time value given on the command line*/
      g_lefttimeindex = get_closest_index(g_lefttimevalue,g_leftNumTimesteps,g_leftTimeValues);
      if(g_lefttimeindex<0)
	{
	  printf("Error: cant find a left timestep with value=%f\n",g_lefttimevalue);
	  exit_safdiff(SAFDIFF_ERROR);
	}
    }

  if( g_gotRightTimeValue )
    { /*find the corresponding time index for a time value given on the command line*/
      g_righttimeindex = get_closest_index(g_righttimevalue,g_rightNumTimesteps,g_rightTimeValues);
      if(g_righttimeindex<0)
	{
	  printf("Error: cant find a right timestep with value=%f\n",g_righttimevalue);
	  exit_safdiff(SAFDIFF_ERROR);
	}
    }

  if(g_lefttimeindex >= 0 && g_righttimeindex >= 0)
    {
      /*if we are requesting to look at only a certain pair of timesteps,
	then we dont want call the files different because they have a 
	different # of timesteps*/
      char label[64];

      print_int_internal( PRINT_LEVEL_1, "number of time steps",g_leftNumTimesteps,g_rightNumTimesteps,
			  0,0,/*neither is missing*/
			  1,/*ignore any differences*/
			  0,0.0,0.0,0.0 );/*no tolerances*/

      if( g_lefttimeindex == g_righttimeindex )
	sprintf(label,"  time at timestep %d", g_lefttimeindex);
      else
	sprintf(label,"  time at timesteps left=%d right=%d", g_lefttimeindex,g_righttimeindex);

      print_float_internal( PRINT_LEVEL_1,label,g_leftTimeValues[g_lefttimeindex],g_rightTimeValues[g_righttimeindex],
			    0,0,
			    0,
			    1, g_time_value_abs_tol,0.0,0.0 );
    }
  else
    {
      int l_max=(g_leftNumTimesteps>g_rightNumTimesteps)?g_leftNumTimesteps:g_rightNumTimesteps;

      print_int(PRINT_LEVEL_1,"number of time steps",g_leftNumTimesteps,g_rightNumTimesteps,0,0);

      for(i=0;i<l_max;i++)
	{
	  char label[64];
	  sprintf(label,"  time at timestep %d", i);

	  print_float_internal( PRINT_LEVEL_1,label,
				i<g_leftNumTimesteps ? g_leftTimeValues[i] : DUMMY_FLOAT,
				i<g_rightNumTimesteps ? g_rightTimeValues[i] : DUMMY_FLOAT,
				i>=g_leftNumTimesteps, i>=g_rightNumTimesteps,
				0,
				1, g_time_value_abs_tol,0.0,0.0 );
	}
    }
}
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Find the saf objects with the given names and compare them.
 *
 * Description: Given two strings representing any saf objects, search through the saf database and if a saf object
 *		exists with the name, compare the objects.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 30, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int find_objs_and_compare(char *leftobjname, char *rightobjname)
{
  int leftnum=0, rightnum=0;
  SAF_Set *leftsets=NULL, *rightsets=NULL;
  SAF_Cat *leftcats=NULL, *rightcats=NULL;
  SAF_Field *leftfields=NULL, *rightfields=NULL;
  SAF_FieldTmpl *leftfield_tmpls=NULL, *rightfield_tmpls=NULL;
  SAF_Suite *leftsuites=NULL, *rightsuites=NULL;
 
  /*compare suites*/
  leftnum=0; rightnum=0;
  saf_find_suites(SAF_ALL, g_leftdb, leftobjname, &leftnum, &leftsuites);
  saf_find_suites(SAF_ALL, g_rightdb, rightobjname, &rightnum, &rightsuites);
  if(leftnum>0 && rightnum>0 && leftsuites!=NULL && rightsuites!=NULL)
    {
      compare_time_values();
      compare_suites(leftsuites, rightsuites);
      return 0;
    }
  
  /*compare sets*/
  leftnum=0; rightnum=0;
  saf_find_matching_sets(SAF_ALL, g_leftdb, leftobjname, SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF, SAF_TOP_TORF,
			 &leftnum, &leftsets);
  saf_find_matching_sets(SAF_ALL, g_rightdb, rightobjname, SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,SAF_TOP_TORF,
			 &rightnum, &rightsets);
  if(leftnum>0 && rightnum>0 && leftsets!=NULL && rightsets!=NULL)
    {
      compare_sets(leftsets+0, rightsets+0);
      return 0;
    }

  /*compare cats*/
  leftnum=0; rightnum=0;
  saf_find_categories(SAF_ALL, g_leftdb, SAF_UNIVERSE(g_leftdb), leftobjname,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&leftnum,&leftcats);
  saf_find_categories(SAF_ALL, g_rightdb, SAF_UNIVERSE(g_rightdb), rightobjname,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&rightnum,&rightcats);
  if(leftnum>0 && rightnum>0 && leftcats!=NULL && rightcats!=NULL){
    compare_cats( leftcats+0, rightcats+0 );
    return 0;
  }

  /*compare fields*/
  leftnum=0; rightnum=0;
  saf_find_fields(SAF_ALL, g_leftdb, SAF_UNIVERSE(g_leftdb), leftobjname, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
		  SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &leftnum, &leftfields);
  saf_find_fields(SAF_ALL, g_rightdb, SAF_UNIVERSE(g_rightdb), rightobjname, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
		  SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &rightnum, &rightfields);
  if(leftnum>0 && rightnum>0 && leftfields!=NULL && rightfields!=NULL){
    compare_fields(leftfields, rightfields);
    return 0;
  }

  /*compare field templates*/
  leftnum=0; rightnum=0;
  saf_find_field_tmpls(SAF_ALL,g_leftdb,leftobjname,SAF_ALGTYPE_ANY,SAF_ANY_BASIS,NULL,
		       &leftnum, &leftfield_tmpls);
  saf_find_field_tmpls(SAF_ALL,g_rightdb,rightobjname,SAF_ALGTYPE_ANY,SAF_ANY_BASIS,NULL,
		       &rightnum, &rightfield_tmpls);
  if(leftnum>0 && rightnum>0 && leftfield_tmpls!=NULL && rightfield_tmpls!=NULL){
    compare_field_templates(leftfield_tmpls, rightfield_tmpls);
    return 0;
  }



  /*can't find units or quantities because saf will throw an error if it can't find them*/
#if 0
  SAF_Quantity leftquantity, rightquantity;
  SAF_Unit leftunit, rightunit;

  leftquantity=saf_find_one_quantity(g_leftdb, leftobjname);
  rightquantity=saf_find_one_quantity(g_rightdb, rightobjname);
  if(leftquantity!=SAF_ERROR_HANDLE && rightquantity!=SAF_ERROR_HANDLE){
    compare_quantity(leftquantity, rightquantity);
    return 0;
  }

  saf_find_one_unit(g_leftdb,leftobjname,&leftunit);
  saf_find_one_unit(g_rightdb,rightobjname,&rightunit);
  if(leftunit!=SAF_ERROR_HANDLE && rightunit!=SAF_ERROR_HANDLE){
    compare_units(leftunit,rightunit);
    return 0;
  }
#endif

  return 1;
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Copying SAF Objects
 * Purpose:    	Given an existing set in a database, create the same set in a new database.
 *
 * Description: Given an existing set in a database, create the same set in a new database.
 *		If the set is already in the new database, return the handle to the new set.
 *
 *
 * Parallel:    
 *
 * Programmer:	Matthew O'Brien, LLNL January 18, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int saf_copy_set(SAF_ParMode pmode, SAF_Set *oldset, SAF_Set *newset, SAF_Db *newdatabase)
{
  int leftnumfound_newsets=0;
  SAF_Set *leftfound_newsets=NULL;
  char *leftsetname = NULL;
  int left_topo_dim=0;
  SAF_SilRole left_srole;
  SAF_ExtendMode left_extendible;
  SAF_TopMode left_topmode;
  /*Maybe  we also want to copy the categories defined on the set*/
  /*int leftnum_colls=0;
    SAF_Cat *left_cats=NULL;*/

  if( !oldset )
  {
	printf("error saf_copy_set: not a valid set handle\n");
	exit(-1);
  }


  saf_describe_set(pmode, oldset, &leftsetname, &left_topo_dim, &left_srole, &left_extendible,
		   &left_topmode, NULL,NULL /*&leftnum_colls, &left_cats*/);
  saf_find_matching_sets(pmode, newdatabase, leftsetname, left_srole,
			 left_topo_dim,left_extendible, SAF_TOP_TORF, &leftnumfound_newsets, &leftfound_newsets);
  if(leftnumfound_newsets>0 && leftfound_newsets!=NULL){
    (*newset)=leftfound_newsets[0];
    free(leftfound_newsets);
  }else{
    saf_declare_set(pmode,newdatabase,leftsetname,left_topo_dim,left_srole,left_extendible,newset);
  }
  free(leftsetname);

  return SAF_SUCCESS;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Writing a New Database
 * Purpose:    	Given an existing cat in a database, create the same cat in a new database.
 *
 * Description:  Given an existing cat in a database, create the same cat in a new database.
 *
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL December 6, 2001
 *
 *--------------------------------------------------------------------------------------------------------------------------------*/
int saf_copy_category(SAF_Cat *oldcat, SAF_Cat *newcat, SAF_Db *newdatabase)
{
	int numold_cats=0;
	SAF_Cat *old_cats=NULL;
	/*saf_describe_category*/
	char *oldcatname=NULL;
	SAF_Role oldcat_role;
	int oldcat_dim;


	if(SS_CAT(oldcat)){
		newcat=SAF_SELF(newdatabase);
	}else{
	        saf_describe_category(SAF_ALL, oldcat, &oldcatname, &oldcat_role,&oldcat_dim);
	        saf_find_categories(SAF_ALL, newdatabase, SAF_UNIVERSE(newdatabase), oldcatname, &oldcat_role, oldcat_dim,
			&numold_cats, &old_cats);
		if(numold_cats>0 && old_cats!=NULL){
			newcat=old_cats;
			free(old_cats);
		}else{
			saf_declare_category(SAF_ALL, newdatabase, oldcatname, &oldcat_role, oldcat_dim, newcat);
		}
		free(oldcatname);
	}
	return SAF_SUCCESS;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Writing a New Database
 * Purpose:    	Given an existing collection in the left database, create the same collection in the new database.
 *
 * Description:  Given an existing collection in a database, create the same collection in a new database.
 *              newset and newcat must already be declared in the new database.
 *
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL December 6, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int saf_copy_collection(SAF_ParMode pmode, SAF_Set *oldset, SAF_Cat *oldcat, SAF_Set *newset, SAF_Cat *newcat)
{
	/*saf_describe_collection*/
	SAF_CellType leftcell_type;
	int leftcollection_count;
	SAF_IndexSpec leftispec;
	SAF_DecompMode leftis_decomp;
	/*SAF_Set *leftmember_sets=NULL;*/
	int found_it=0;
	int t;
	/*saf_find_collections*/
	int num_colls;
	SAF_Cat *cats=NULL;
	if(!SS_CAT(oldcat)){
	  saf_find_collections(pmode, newset, NULL, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM,
			       SAF_DECOMP_TORF, &num_colls, &cats);
	  if(num_colls>0 && cats!=NULL){
	    for(t=0; t<num_colls; t++){
	      if(!strcmp(cat_name(oldcat), cat_name(cats+t))){
		*newcat=cats[t];
		found_it=1;
		break;
	      }
	    }
	  }
	  if(!found_it){
	    saf_describe_collection(pmode, oldset, oldcat, &leftcell_type, &leftcollection_count,
				    &leftispec, &leftis_decomp, NULL );
	    saf_declare_collection(pmode,newset,newcat,leftcell_type,leftcollection_count,
				   leftispec,leftis_decomp);
	  }
	  free(cats);
	} 
	return SAF_SUCCESS;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Writing a New Database
 * Purpose:    	Translate a suite in one database to a suite in the new database.
 *
 * Description:  MACRO.  This macro assumes the existence of a suite of the same name as the left suite
 *		in the new database.  It does not declare a suite, it only finds the one that is supposed to already
 *		exist in the new database, by the same name as the suite in the left database.
 *
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL December 15, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int saf_copy_suite(SAF_ParMode pmode, SAF_Suite *oldsuite, SAF_Suite *newsuite, SAF_Db *newdatabase)
{
  int numfound_suites=0;
  SAF_Suite *found_suites=NULL;
  char *oldsuitename=NULL;



  printf("jake in saf_copy_suite....not ready yet, exiting\n");
  exit_safdiff(SAFDIFF_ERROR);




  saf_describe_suite(pmode, oldsuite, &oldsuitename, NULL, NULL, NULL);
  saf_find_suites(pmode,newdatabase,oldsuitename,&numfound_suites,&found_suites);

  if(numfound_suites>0 && found_suites!=NULL){
    (*newsuite)=found_suites[0];
  }else{

    /*jake XXX: need to copy the mesh set as well, to create this suite*/

    saf_declare_suite(pmode,newdatabase,oldsuitename,NULL,NULL,newsuite);


  }
  if(found_suites) free(found_suites);
  if(oldsuitename) free(oldsuitename);
  return SAF_SUCCESS;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Clean up after a call to compare_xxx
 *
 * Description: Decrement the indentation, free the context array and set the context length to zero.
 *		This needs to be done any time a compare_xxx function exists, so we make a macro for it.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 29, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
#define END_OF_COMPARE_FUNCTION					\
{								\
	int looper;						\
	if(g_indent>0) g_indent--;						\
	/*FREE_ARRAY(g_context_buffer, g_context_length);*/		\
	for(looper=0; looper<g_context_length; looper++){		\
		g_context_buffer[looper][0]='\0';			\
	}							\
	g_context_length=0;					\
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two saf sets.
 *
 * Description: This function compares two saf sets.  It follows the general structure of comparing two saf objects:
 *		That is, it calls saf_describe_set on both the left and right sets, then it prints out all of the 
 *		primitive datatypes, and calls compare_xxx on all of the other saf objects that compose a SAF_Set.
 *		This function also finds its subsets and recursively compares the subsets.
 *		This function also finds the fields declared on this sets and compares them.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL October 10, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_sets(SAF_Set *left, SAF_Set *right)
{
  if(g_globalfo.store.anything){
    return safdiff_compare_sets(left, right, g_newdb);
  }else{
    return safdiff_compare_sets(left, right, NULL);
  }
}


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int safdiff_compare_sets(SAF_Set *left, SAF_Set *right, SAF_Db *newdatabase)
{
  int l_leftMissing=1;
  int l_rightMissing=1;
  SAF_Set newset;
  char *leftsetname = NULL, *rightsetname = NULL;
  int left_topo_dim=0, right_topo_dim=0;
  SAF_SilRole left_srole, right_srole;
  SAF_ExtendMode left_extendible, right_extendible;
  SAF_TopMode left_topmode, right_topmode;
  int leftnum_colls=0, rightnum_colls=0;
  SAF_Cat *left_cats=NULL, *right_cats=NULL;
  int i;
  size_t limit=1;
  ss_pers_t *options=NULL;
  options = _saf_htab_find(g_seen_sets, _saf_hkey_str(safdiff_strdup(set_name(left))), &limit, options);
  if (limit > 0) return 0;

  _saf_htab_insert(g_seen_sets, _saf_hkey_str(safdiff_strdup(set_name(left))), (ss_pers_t *)&limit);

  if( !SS_SET(left) || !SS_SET(right) )
  {
	printf("error saf_compare_sets: not a valid set handle\n");
	exit(-1);
  }


  saf_describe_set(SAF_ALL, left, &leftsetname, &left_topo_dim, &left_srole, &left_extendible, 
		   &left_topmode, &leftnum_colls, &left_cats);
  saf_describe_set(SAF_ALL, right, &rightsetname, &right_topo_dim, &right_srole, &right_extendible, 
		   &right_topmode, &rightnum_colls, &right_cats);

  /*write out set to new database*/
  if(newdatabase)
    {
      SAF_Cat newcat;

      saf_copy_set(SAF_ALL,left, &newset, newdatabase);
      for(i=0; i<leftnum_colls; i++){
	saf_copy_category(left_cats+i, &newcat,g_newdb);
	saf_copy_collection(SAF_ALL, left, left_cats+i, &newset, &newcat);
      }
    }


  print_break(PRINT_LEVEL_3,"Next Set ");
  print_string(PRINT_LEVEL_3,"set name", leftsetname, rightsetname);
  g_indent++;
  print_int(PRINT_LEVEL_3,"max topo dim", left_topo_dim, right_topo_dim,0,0);
  print_string(PRINT_LEVEL_3,"sil role", str_SAF_SilRole(left_srole), str_SAF_SilRole(right_srole));

  print_string(PRINT_LEVEL_3,"extendible", str_SAF_ExtendMode(left_extendible), str_SAF_ExtendMode(right_extendible));
  print_string(PRINT_LEVEL_4,"top mode", str_SAF_TopMode(left_topmode), str_SAF_TopMode(right_topmode));



  print_int(PRINT_LEVEL_3,"num collections", leftnum_colls, rightnum_colls,0,0);


  canonicalize_cat_lists(left_cats, right_cats, (size_t)leftnum_colls, (size_t)rightnum_colls);


  compare_coll_lists(left,right, (size_t)leftnum_colls, (size_t)rightnum_colls,left_cats, right_cats);

  {/*new stuff for saf_find_fields*/
    /*saf_find_fields*/
    int leftnum_fields=0, rightnum_fields=0;
    SAF_Field *leftfields=NULL, *rightfields=NULL;
    int i;
    saf_find_fields(SAF_ALL, g_leftdb, left, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
		    SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &leftnum_fields, &leftfields);

    saf_find_fields(SAF_ALL, g_rightdb, right, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
		    SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &rightnum_fields, &rightfields);

 
    /*jake: what about fields that are not in the suites? compare them or no?*/

    /*filter according to fields that are in a give state (g_lefttimeindex or g_righttimeindex)*/
    if(g_lefttimeindex >= 0 && g_righttimeindex >= 0)
      {
	SAF_Field *filteredleftfields, *filteredrightfields;
	int count=0;
        int *buffer=NULL;

	filteredleftfields=(SAF_Field *)malloc(leftnum_fields * sizeof(SAF_Field));
	filteredrightfields=(SAF_Field *)malloc(rightnum_fields * sizeof(SAF_Field));
	for(i=0; i<leftnum_fields; i++){
           buffer = Ftab_find(g_leftWhichTimestepPerFieldRow,  leftfields+i, g_nleft);
           if (buffer)
	      if(buffer[0]==g_lefttimeindex)
	           filteredleftfields[count++]=leftfields[i];
	}
	memcpy(leftfields, filteredleftfields, count * sizeof(SAF_Field));

	leftnum_fields = count;
	free(filteredleftfields);
	count=0;
	for(i=0; i<rightnum_fields; i++){
           buffer = Ftab_find(g_rightWhichTimestepPerFieldRow,  rightfields+i, g_nright);
           if (buffer)
	      if(buffer[0]==g_righttimeindex)
	            filteredrightfields[count++]=rightfields[i];
	}
	memcpy(rightfields, filteredrightfields, count * sizeof(SAF_Field));

	leftnum_fields = count;
	rightnum_fields = count;
	free(filteredrightfields);
      }




#ifdef USE_EXTRA_SORT
    /*jake test 021803 pre-sort the fields*/
    safdiff_sort((ss_pers_t *)(leftfields), (size_t)leftnum_fields, sizeof(SAF_Field), sort_compare_fields);
    safdiff_sort((ss_pers_t *)(rightfields), (size_t)rightnum_fields, sizeof(SAF_Field), sort_compare_fields);
#endif



    print_int(PRINT_LEVEL_3,"num fields", leftnum_fields, rightnum_fields,0,0);
    canonicalize_field_lists(leftfields, rightfields, (size_t)leftnum_fields, (size_t)rightnum_fields);

    compare_field_lists(leftfields, rightfields, (size_t)leftnum_fields, (size_t)rightnum_fields);


    if(leftfields) free(leftfields);
    if(rightfields) free(rightfields);
  }

#if 0 
  compare_attributes_old_handles((ss_pers_t*)left, (ss_pers_t*)right, 0,0);
#endif

  if (SS_SET(left))l_leftMissing=0;
  if (SS_SET(right))l_rightMissing=0;
  compare_attributes_old_handles((ss_pers_t*)left, (ss_pers_t*)right, l_leftMissing,l_rightMissing);



  /*left and right are two sets that we are comparing*/
  compare_cat_lists_and_find_subsets(left_cats, right_cats, (size_t)leftnum_colls, (size_t)rightnum_colls, left, right);


  if(left_cats) free(left_cats);
  if(right_cats) free(right_cats);
  if(leftsetname) free(leftsetname);
  if(rightsetname) free(rightsetname);

  END_OF_COMPARE_FUNCTION;

  return 0;
}/*safdiff_compare_sets*/


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two topological relations.
 *
 * Description: Given two sets and two categories, compare the topological relations defined by them.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_topo_relations(SAF_Set *left, SAF_Set *right, SAF_Cat *leftcat, SAF_Cat *rightcat)
{
  int i;
  SAF_Set leftset,rightset;
  SAF_Cat leftthe_pieces, rightthe_pieces;
  SAF_Set leftrange_s, rightrange_s;
  SAF_Cat leftrange_c, rightrange_c;
  SAF_Cat leftstorage_decomp, rightstorage_decomp;
  SAF_RelRep lefttrtype, righttrtype;
  char *leftthe_piecesname=NULL, *rightthe_piecesname=NULL;
  char *leftrange_cname=NULL, *rightrange_cname=NULL;
  int leftnum_rels=0, rightnum_rels=0;
  SAF_Rel *lefttoporels=NULL, *righttoporels=NULL;
  SAF_RelRep leftrelrep, rightrelrep;
  size_t leftabuf_sz=0, rightabuf_sz=0;
  hid_t leftabuf_type, rightabuf_type;
  size_t leftbbuf_sz=0, rightbbuf_sz=0;
  hid_t leftbbuf_type, rightbbuf_type;
  int *leftabuf=0, *leftbbuf=0;
  int *rightabuf=0, *rightbbuf=0;

  saf_find_topo_relations(SAF_ALL,g_leftdb,left,NULL, &leftnum_rels, &lefttoporels);
  saf_find_topo_relations(SAF_ALL,g_rightdb,right,NULL, &rightnum_rels, &righttoporels);


  /*not needed?
    print_string(PRINT_LEVEL_3,"find topo rel on set", set_name(left), set_name(right));
    g_indent++;*/


  print_int(PRINT_LEVEL_3,"number topo rels", leftnum_rels, rightnum_rels,0,0);


  if( leftnum_rels > 0 && rightnum_rels > 0 )
    {
      int minnum_rels = MIN(leftnum_rels, rightnum_rels);

      for(i=0; i<minnum_rels; i++)
	{
	  saf_describe_topo_relation(SAF_ALL,lefttoporels+i, &leftset, &leftthe_pieces, &leftrange_s,
				     &leftrange_c, &leftstorage_decomp, &lefttrtype, NULL);
	  saf_describe_topo_relation(SAF_ALL,righttoporels+i, &rightset, &rightthe_pieces, &rightrange_s,
				     &rightrange_c, &rightstorage_decomp, &righttrtype,NULL);

	  leftthe_piecesname=cat_name(&leftthe_pieces);
	  rightthe_piecesname=cat_name(&rightthe_pieces);
	  leftrange_cname=cat_name(&leftrange_c);
	  rightrange_cname=cat_name(&rightrange_c);


	  if(!strcmp(leftthe_piecesname,cat_name(leftcat)) && 
	     !strcmp(rightthe_piecesname,cat_name(rightcat)) )
	    { 

	      print_break(PRINT_LEVEL_3,"Topology Relations ");
	      print_string(PRINT_LEVEL_3,"set name", set_name(&leftset), set_name(&rightset));
	      print_string(PRINT_LEVEL_3,"the pieces", leftthe_piecesname, rightthe_piecesname);
	      print_string(PRINT_LEVEL_3,"collection", collection_name(&leftrange_s,&leftrange_c),collection_name(&rightrange_s,&rightrange_c));
	      print_string(PRINT_LEVEL_3,"storage decomp", cat_name(&leftstorage_decomp), cat_name(&rightstorage_decomp));

	      compare_relreps(&lefttrtype, &righttrtype,0,0);

	      saf_get_count_and_type_for_topo_relation(SAF_ALL,lefttoporels+i,NULL,&leftrelrep,
						       &leftabuf_sz,&leftabuf_type,&leftbbuf_sz,&leftbbuf_type);
	      saf_get_count_and_type_for_topo_relation(SAF_ALL,righttoporels+i,NULL,&rightrelrep,
						       &rightabuf_sz,&rightabuf_type,&rightbbuf_sz,&rightbbuf_type);

	      print_int(PRINT_LEVEL_3,"abuf size", (int)leftabuf_sz, (int)rightabuf_sz,0,0);
	      print_string(PRINT_LEVEL_3,"abuf type",(leftabuf_sz <= 0) ? "":StringDataType(leftabuf_type),
                                                     (rightabuf_sz <=0) ? "":StringDataType(rightabuf_type));
	      print_int(PRINT_LEVEL_3,"bbuf size", (int)leftbbuf_sz, (int)rightbbuf_sz,0,0);
	      print_string(PRINT_LEVEL_3,"bbuf type",(leftbbuf_sz <= 0) ? "":StringDataType(leftbbuf_type),
                                                     (rightbbuf_sz<= 0) ? "":StringDataType(rightbbuf_type));

	      leftabuf=NULL;  leftbbuf=NULL;
	      rightabuf=NULL; rightbbuf=NULL;
	    
	      if(leftabuf_sz || leftbbuf_sz)
		saf_read_topo_relation(SAF_ALL, lefttoporels+i, NULL, (void **)&leftabuf, (void **)&leftbbuf);
	      if(rightabuf_sz || rightbbuf_sz)
		saf_read_topo_relation(SAF_ALL, righttoporels+i, NULL, (void **)&rightabuf, (void **)&rightbbuf);

	      /*write topology relations to new database*/
	      if(g_globalfo.store.anything)
		{
		  SAF_Rel newrel;
		  SAF_Set leftmy_piece;
		  SAF_Db *newfile;
		  SAF_Set newset, newrange_s, newmy_piece;
		  SAF_Cat newthe_pieces, newrange_c, newstorage_decomp;
		  newfile=g_newdb;

		  leftmy_piece=leftset;
		  saf_copy_set(SAF_ALL,&leftset, &newset, g_newdb);
		  saf_copy_category(&leftthe_pieces, &newthe_pieces,g_newdb);
		  saf_copy_set(SAF_ALL, &leftrange_s, &newrange_s, g_newdb);
		  saf_copy_category(&leftrange_c, &newrange_c, g_newdb);
		  saf_copy_category(&leftstorage_decomp,&newstorage_decomp,g_newdb);
		  saf_copy_set(SAF_ALL, &leftmy_piece, &newmy_piece, g_newdb);
		  saf_declare_topo_relation(SAF_ALL, g_newdb, &newset, &newthe_pieces, &newrange_s, &newrange_c, &newstorage_decomp,
					    &newmy_piece, &lefttrtype, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &newrel);
		  /*leftabuf_type, NULL, leftbbuf_type, NULL, &newrel);*/

		  if(leftabuf_sz || leftbbuf_sz)
		    saf_write_topo_relation(SAF_ALL, &newrel,leftabuf_type, (void *)leftabuf, leftbbuf_type, (void *)leftbbuf, newfile);
		}

#ifdef USE_BANNER
	      if(leftbbuf_sz > rightbbuf_sz){
		sprintf(g_banner, "Topology relation defined on %s, sewing together %s to form %s\n", 
			set_name(&leftset), leftrange_cname,leftthe_piecesname);
	      }else{
		sprintf(g_banner, "Topology relation defined on %s, sewing together %s to form %s\n", 
			set_name(&rightset), rightrange_cname,rightthe_piecesname);
	      }
#endif

	      /*
	      ** note: not prepared here to print out anything but ints! (especially not SAF_HANDLES) XXX
	      */
	      if((leftbbuf_sz <= 0) && (rightbbuf_sz <= 0))return 0;

	      if( H5Tequal(leftabuf_type,SAF_INT) && H5Tequal(rightabuf_type,SAF_INT) )
		{
		  print_int_array(PRINT_LEVEL_4,PRINT_LEVEL_5,"abuf", leftabuf, rightabuf, leftabuf_sz, rightabuf_sz);
		}
	      else print_string_ignore(PRINT_LEVEL_4,"abuf","not SAF_INT!","");

	      if( H5Tequal(leftbbuf_type,SAF_INT) && H5Tequal(rightbbuf_type,SAF_INT) )
		{
		  /*all the stuff in between the stars is a much more informative way of printing bbuf*/

		  /*print_int_array(PRINT_LEVEL_4,PRINT_LEVEL_5,"bbuf", leftbbuf, rightbbuf, leftbbuf_sz, rightbbuf_sz);*/


		  /**********************************************************************************************************************/
		  /*Very informative label for printing topological relations: "elements i, nodes j"*/
		  {
		    char label[256];
		    size_t abuf_ptr=0;
		    int nextblock=0;
		    int node=0;
		    size_t j;
		    size_t minbbuf_sz=MIN(leftbbuf_sz, rightbbuf_sz);

		    if(leftabuf) nextblock=leftabuf[0];

		    g_printed_lines=0;
		    if(!strcmp(str_SAF_RelRep(&lefttrtype),"SAF_UNSTRUCTURED") && 
		       !strcmp(str_SAF_RelRep(&righttrtype),"SAF_UNSTRUCTURED"))
		      {

#define CREATE_UNSTRUCTED_LABEL(the_piecesname, abuf, range_cname, l_index)	\
sprintf(label, "%s %i, %s %i", the_piecesname, (int)(l_index/abuf[0]), range_cname, (int)(l_index%abuf[0]))



			if(g_printLevel==PRINT_LEVEL_3)
			  {
			    for(j=0; j<minbbuf_sz; j++)
			      {
				IF_SCREENFULL_PRINT_BANNER
				  CREATE_UNSTRUCTED_LABEL(leftthe_piecesname, leftabuf, leftrange_cname, j);
				if(leftbbuf[j]!=rightbbuf[j]){
				  print_int(PRINT_LEVEL_3,label,leftbbuf[j], rightbbuf[j],0,0);
				  break;
				}
			      }
			    if(j==minbbuf_sz && j>0)
			      {
				j--;
				IF_SCREENFULL_PRINT_BANNER
				  strcat(label, ", max diff");
				print_int(PRINT_LEVEL_3,label,leftbbuf[j], rightbbuf[j],0,0);

			      }
			  }
			else if(g_printLevel>=PRINT_LEVEL_4)
			  {
			    for(j=0; j<minbbuf_sz; j++)
			      {
				IF_SCREENFULL_PRINT_BANNER
				  CREATE_UNSTRUCTED_LABEL(leftthe_piecesname, leftabuf, leftrange_cname, j);
				print_int(PRINT_LEVEL_4,label,leftbbuf[j], rightbbuf[j],0,0);
			      }
			    for(j=rightbbuf_sz; j<leftbbuf_sz; j++)
			      {
				IF_SCREENFULL_PRINT_BANNER
				  CREATE_UNSTRUCTED_LABEL(leftthe_piecesname, leftabuf, leftrange_cname, j);
				print_int(PRINT_LEVEL_4,label,leftbbuf[j], DUMMY_INT,0,1);
			      }
			    for(j=leftbbuf_sz; j<rightbbuf_sz; j++)
			      {
				IF_SCREENFULL_PRINT_BANNER
				  CREATE_UNSTRUCTED_LABEL(rightthe_piecesname, rightabuf, rightrange_cname, j);
				print_int(PRINT_LEVEL_4,label,DUMMY_INT, rightbbuf[j],1,0);
			      }
			  }
		      }
		    else if(!strcmp(str_SAF_RelRep(&lefttrtype),"SAF_ARBITRARY") &&
			    !strcmp(str_SAF_RelRep(&righttrtype),"SAF_ARBITRARY"))
		      {


#define CREATE_ARBITRARY_LABEL(the_piecesname, abuf, range_cname,j,abuf_sz)			       	\
{													\
	if((int)j==nextblock){										\
		node=0;											\
		abuf_ptr++;										\
		if(abuf_ptr>=abuf_sz){									\
			printf("Error: this topology relation of type SAF_ARBITRARY is messed up "	\
				"there are more elements in bbuf than abuf indicates\n");		\
		}											\
		nextblock+=abuf[abuf_ptr];								\
	}												\
	sprintf(label, "%s %i, %s %i", the_piecesname, (int)abuf_ptr, range_cname, (int)node);		\
	node++;												\
}



			if(g_printLevel==PRINT_LEVEL_3)
			  {
			    for(j=0; j<minbbuf_sz; j++){
			      CREATE_ARBITRARY_LABEL(leftthe_piecesname, leftabuf, leftrange_cname,j,leftabuf_sz);
			      if(leftbbuf[j]!=rightbbuf[j]){
				print_int(PRINT_LEVEL_3,label,leftbbuf[j], rightbbuf[j],0,0);
				break;
			      }
			    }
			    if(j==minbbuf_sz && j>0){
			      j--;
			      strcat(label, ", max diff");
			      print_int(PRINT_LEVEL_3,label,leftbbuf[j], rightbbuf[j],0,0);

			    }
			  }
			else if(g_printLevel>=PRINT_LEVEL_4)
			  {
			    for(j=0; j<minbbuf_sz; j++){
			      IF_SCREENFULL_PRINT_BANNER
				CREATE_ARBITRARY_LABEL(leftthe_piecesname, leftabuf, leftrange_cname,j,leftabuf_sz);
			      print_int(PRINT_LEVEL_4,label,leftbbuf[j], rightbbuf[j],0,0);
			    }
			    for(j=(int)rightbbuf_sz; j<leftbbuf_sz; j++){
			      IF_SCREENFULL_PRINT_BANNER
				CREATE_ARBITRARY_LABEL(leftthe_piecesname, leftabuf, leftrange_cname,j,leftabuf_sz);
			      print_int(PRINT_LEVEL_4,label,leftbbuf[j], DUMMY_INT,0,1);
			    }
			    for(j=(int)leftbbuf_sz; j<rightbbuf_sz; j++){
			      IF_SCREENFULL_PRINT_BANNER
				CREATE_ARBITRARY_LABEL(leftthe_piecesname, leftabuf, leftrange_cname,j,leftabuf_sz);
			      print_int(PRINT_LEVEL_4,label,DUMMY_INT, rightbbuf[j],1,0);
			    }
			  }

		      }

		  }
		  /**********************************************************************************************************************/
		}
	      else print_string_ignore(PRINT_LEVEL_4,"bbuf","not SAF_INT!","");





	      if(leftabuf) free(leftabuf);  
	      if(leftbbuf) free(leftbbuf);
	      if(rightabuf) free(rightabuf); 
	      if(rightbbuf) free(rightbbuf);

	    }/*if cat names are the same*/
	}/*for i*/
    }/*if EQUIV*/


  if(lefttoporels) free(lefttoporels);
  if(righttoporels) free(righttoporels);
  END_OF_COMPARE_FUNCTION;
  return 0;

}/*compare_topo_relations*/



/* Changed this from referring to a field's "state" to "timestep",
** because a state index is not necessarily the same as the time index.
*/
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int get_timestep_per_field(SAF_Db *db, int a_numTimesteps, double *a_timeValues, FieldTableOfValues_t **a_timestepPerField, int *nfields)
{
  SAF_Field *fields=NULL;
  int nfound;
  int num_fields;
  int num_suites, i, j, k;
  SAF_Suite *suites=NULL;
  SAF_Field *fields_instate;

  /*alloc and init the returned array*/
  saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
		  SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &num_fields, &fields);

  if( num_fields <= 0 ) return(0);

  nfound=0;
  /* a_timestepPerField = (FieldTableOfValues_t *)calloc((unsigned int)num_fields , sizeof(FieldTableOfValues_t)); 
  */
  if ((*a_timestepPerField))
    for(i=0; i<num_fields; i++)
       (*a_timestepPerField)[i].key=SS_FIELD_NULL;   /*this is the signal that the field is not in any timestep*/


  saf_find_suites(SAF_ALL, db, SAF_ANY_NAME, &num_suites, &suites);

  for(i=0; i<num_suites; i++)
    {
      int l_numGroups=0;
      SAF_StateGrp *l_stateGroups=0;
      saf_find_state_groups(SAF_ALL, suites+i, NULL, &l_numGroups, &l_stateGroups);

      for(j=0; j<l_numGroups; j++)
	{
	  int l_numStates=0;
	  SAF_StateTmpl l_stateTemplate;
	  int l_numFieldsPerState=0;
	  hid_t l_varType;

	  saf_describe_state_group(SAF_ALL,l_stateGroups+j,NULL,NULL,&l_stateTemplate,NULL,NULL,&l_varType,&l_numStates);
	  saf_describe_state_tmpl(SAF_ALL,&l_stateTemplate,NULL,&l_numFieldsPerState, NULL );

	  /*if(g_debugging) printf("   State Group %d of %d: %d states, %d fields per state--------------------\n",
	    j+1,l_numGroups,l_numStates,l_numFieldsPerState);*/

	  for(k=0; k<l_numStates; k++)
	    {
	      int kk,l_thisTimestep=-1;
	      float l_thisTimeValueFloat=-1;
	      double l_thisTimeValueDouble=-1;
	      int l_thisTimeValueInt=-1;
	      void *l_timePtr=0;

              if (l_varType < 0){
                  printf("get_number_of_timesteps: error getting time value, not ready for l_varType=%ld\n",(long)l_varType);
		  exit_safdiff(SAFDIFF_ERROR);
                 }
	      if( H5Tequal(l_varType, SAF_FLOAT) ) l_timePtr=&l_thisTimeValueFloat;
	      else if( H5Tequal(l_varType, SAF_DOUBLE) ) l_timePtr=&l_thisTimeValueDouble;
	      else if( H5Tequal(l_varType, SAF_INT) ) l_timePtr=&l_thisTimeValueInt;
	      else 
		{
		  printf("get_number_of_timesteps: error getting time value, not ready for l_varType=%s\n",
			 StringDataType(l_varType));
		  exit_safdiff(SAFDIFF_ERROR);
		}

	      fields_instate=NULL;

	      saf_read_state(SAF_ALL, l_stateGroups+j, k, NULL, NULL, l_timePtr,&fields_instate);

	      if(H5Tequal(l_varType, SAF_FLOAT) ) l_thisTimeValueDouble = (double)l_thisTimeValueFloat;
	      /*else if( l_varType == SAF_DOUBLE ) l_thisTimeValueDouble = (double)l_thisTimeValueDouble;*/
	      else if(H5Tequal(l_varType, SAF_INT) ) l_thisTimeValueDouble = (double)l_thisTimeValueInt;

	      /*find what timestep this time value refers to*/
	      for(kk=0;kk<a_numTimesteps;kk++)
		{
		  if( l_thisTimeValueDouble == a_timeValues[kk] )
		    {
		      l_thisTimestep=kk;
		      break;
		    }
		}
	
	      for(kk=0; kk<l_numFieldsPerState; kk++)
		{	
                  if (SS_FIELD(fields_instate+kk)) 
		  {
		    /*
		    printf("%d of %d: Timestep %d, time %f, row of field %3i, field name %s\n", kk,l_numFieldsPerState,
		     l_thisTimestep, l_thisTimeValueDouble,  fields_instate[kk].theRow,field_name(fields_instate[kk]));
		    */
    
		    /* use to be.... (*a_timestepPerField)[fields_instate[kk].theRow]=l_thisTimestep; */
                   (*a_timestepPerField) = (FieldTableOfValues_t *)realloc((*a_timestepPerField), (size_t) ((nfound+1)*sizeof(FieldTableOfValues_t)));
                    (*a_timestepPerField)[nfound].key=fields_instate[kk];
                    (*a_timestepPerField)[nfound].value=l_thisTimestep;
                    nfound++;
		  }
		  else
		  {
		     /*XXX something is wrong with the field*/
		  }
		}
	      free(fields_instate);
	    }

	}	

    }
  if(fields) free(fields);
  if(suites) free(suites);

  qsort((*a_timestepPerField), (size_t) nfound, sizeof(FieldTableOfValues_t), Ftabcompare); 
  *nfields = nfound;

  return 0;
} /*get_timestep_per_field*/

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: Get the associated time value for each state in each suite,
 * sort and remove duplicates, and return the number of timesteps.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int get_number_of_timesteps(SAF_Db *a_db, double **a_timeValuesPerTimeStep)
{
  int l_numTimesteps=0;
  int i,j,l_whichTimestep;
  int l_numSuites=0;
  SAF_Set *l_suites=0;
  double *l_allTimeValues=0;

  saf_find_suites( SAF_ALL, a_db, SAF_ANY_NAME, &l_numSuites, &l_suites );

  /*count the maximum # of timesteps possible*/
  for( i=0; i<l_numSuites; i++ )
    {
      SAF_StateGrp *l_state_grps=0;
      int l_whichGroup, l_numGroups=0;
      saf_find_state_groups(SAF_ALL, l_suites+i, NULL, &l_numGroups, &l_state_grps);
      for( l_whichGroup=0; l_whichGroup<l_numGroups; l_whichGroup++ )
	{
	  int l_numStatesPerGroup=0;
	  saf_describe_state_group(SAF_ALL, l_state_grps+l_whichGroup, NULL, NULL, NULL,
				   NULL, NULL, NULL, &l_numStatesPerGroup );
	  l_numTimesteps += l_numStatesPerGroup;
	}
    }
  /*printf("l_numTimesteps=%d\n",l_numTimesteps);*/
  if( !l_numTimesteps ) return(0); /*no timesteps, so return 0*/

  /*allocate and read the time values for all the possible timesteps*/
  l_allTimeValues = (double *)malloc(l_numTimesteps*sizeof(double));
  l_whichTimestep=0;
  for( i=0; i<l_numSuites; i++ )
    {
      hid_t l_varType;
      SAF_StateGrp *l_state_grps=0;
      int l_whichGroup, l_numGroups=0;

      saf_find_state_groups(SAF_ALL, l_suites+i, NULL, &l_numGroups, &l_state_grps);

      for( l_whichGroup=0; l_whichGroup<l_numGroups; l_whichGroup++ )
	{
	  int l_numStatesPerGroup=0;
	  saf_describe_state_group(SAF_ALL, l_state_grps+l_whichGroup, NULL, NULL, NULL,
				   NULL, NULL, &l_varType, &l_numStatesPerGroup );

	  for(j=0;j<l_numStatesPerGroup;j++)
	    {
	      float l_thisTimeValueFloat=-1;
	      double l_thisTimeValueDouble=-1;
	      int l_thisTimeValueInt=-1;
	      void *l_timePtr=0;

              if (l_varType < 0){
                  printf("get_number_of_timesteps: error getting time value, not ready for l_varType=%ld\n",(long)l_varType);
		  exit_safdiff(SAFDIFF_ERROR);
                 }
	      if( H5Tequal(l_varType, SAF_FLOAT) ) l_timePtr=&l_thisTimeValueFloat;
	      else if( H5Tequal(l_varType, SAF_DOUBLE) ) l_timePtr=&l_thisTimeValueDouble;
	      else if( H5Tequal(l_varType, SAF_INT) ) l_timePtr=&l_thisTimeValueInt;
	      else 
		{
		  printf("get_number_of_timesteps: error getting time value, not ready for l_varType=%s\n",StringDataType(l_varType));
		  exit_safdiff(SAFDIFF_ERROR);
		}
		  
	      saf_read_state(SAF_ALL, l_state_grps+l_whichGroup, j, NULL, NULL, l_timePtr,  NULL ); 

	      if( H5Tequal(l_varType, SAF_FLOAT) ) l_allTimeValues[l_whichTimestep] = (double)l_thisTimeValueFloat;
	      else if( H5Tequal(l_varType, SAF_DOUBLE) ) l_allTimeValues[l_whichTimestep] = (double)l_thisTimeValueDouble;
	      else if( H5Tequal(l_varType, SAF_INT) ) l_allTimeValues[l_whichTimestep] = (double)l_thisTimeValueInt;

	      /*printf("     l_allTimeValues[%d]=%f\n",l_whichTimestep,l_allTimeValues[l_whichTimestep]);*/

	      l_whichTimestep++;
	    }
	}
    }  



  /*go thru the list of time values, remove duplicates, and bubble sort*/
  {
    int l_didSomething;
    do {
      int l_duplicate=-1;
      l_didSomething=0;	  
      for( i=0; i<l_numTimesteps-1; i++ ) 
	{
	  if( l_allTimeValues[i] == l_allTimeValues[i+1] )
	    {
	      l_duplicate = i+1;
	      l_didSomething = 1;
	      break;
	    }
	  else if( l_allTimeValues[i] > l_allTimeValues[i+1] )
	    {
	      double l_temp = l_allTimeValues[i+1];
	      l_allTimeValues[i+1] = l_allTimeValues[i];
	      l_allTimeValues[i] = l_temp;
	      l_didSomething = 1;
	    }
	}
      if( l_duplicate >= 0 )
	{ /*shuffle down the remaining entries in the list*/
	  for( i=l_duplicate; i<l_numTimesteps; i++ ) 
	    {
	      l_allTimeValues[i] = l_allTimeValues[i+1];
	    }
	  l_numTimesteps--;
	}
    } while( l_didSomething );
  } 
  /*printf("After sorting:\n");
    for(i=0;i<l_numTimesteps;i++) printf("\t\t time[%d]=%f\n",i,l_allTimeValues[i]);*/

  /*allocate and fill the returned argument, if requested*/
  if(a_timeValuesPerTimeStep)
    {
      a_timeValuesPerTimeStep[0]=(double *)malloc(l_numTimesteps*sizeof(double));
      memcpy(a_timeValuesPerTimeStep[0],l_allTimeValues,l_numTimesteps*sizeof(double));
    }

  free(l_allTimeValues);

  return(l_numTimesteps);
}





/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two suites.
 *
 * Description: Given two suites, compare them.  It compares their coordinate fields, their state templates and 
 *		the state fields associated with the two suites.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_suites(SAF_Suite *left, SAF_Suite *right)
{
  char *left_suitename=NULL, *right_suitename=NULL;
  int leftdim=0, rightdim=0;
  SAF_Quantity leftquantity, rightquantity;
  SAF_Unit leftunit, rightunit;
  hid_t leftcoord_data_type, rightcoord_data_type;
  SAF_Field leftcoords=SS_FIELD_NULL, rightcoords=SS_FIELD_NULL; 
  int i;
  int l_leftNumSpaceSets=0,l_rightNumSpaceSets=0;
  SAF_Set *l_leftMeshSpace=0, *l_rightMeshSpace=0, *l_leftParamSpace=0, *l_rightParamSpace=0;


  /*note: according to suites.c l_*ParamSpace is not implemented yet*/
  saf_describe_suite(SAF_ALL,left,&left_suitename,&l_leftNumSpaceSets,&l_leftMeshSpace,&l_leftParamSpace);
  saf_describe_suite(SAF_ALL,right,&right_suitename,&l_rightNumSpaceSets,&l_rightMeshSpace,&l_rightParamSpace);

  if(l_leftNumSpaceSets<=0 || l_rightNumSpaceSets<=0)
    {
      printf("Skipping compare_suites because l_leftNumSpaceSets=%d l_rightNumSpaceSets=%d\n",
	     l_leftNumSpaceSets,l_rightNumSpaceSets );
      return(0);
    }

  /*Get the default coords for just the first mesh set, for now. Later
    might repeat this (and all of the following) for all mesh sets? */
  saf_find_default_coords(SAF_ALL, l_leftMeshSpace, &leftcoords);
  saf_find_default_coords(SAF_ALL, l_rightMeshSpace, &rightcoords);

  /*get coords unit,datatype,and quantity*/
  {
    SAF_FieldTmpl l_tmpl=SS_FIELDTMPL_NULL;
    saf_describe_field(SAF_ALL, &leftcoords, &l_tmpl, NULL, NULL, &leftunit, 
		       NULL,NULL,NULL,NULL,NULL,NULL,&leftcoord_data_type,NULL,NULL,NULL,NULL);
    saf_describe_field_tmpl(SAF_ALL, &l_tmpl, NULL,NULL,NULL, &leftquantity, NULL,NULL);
  }
  {
    SAF_FieldTmpl l_tmpl=SS_FIELDTMPL_NULL;
    saf_describe_field(SAF_ALL, &rightcoords, &l_tmpl, NULL, NULL, &rightunit, 
		       NULL,NULL,NULL,NULL,NULL,NULL,&rightcoord_data_type,NULL,NULL,NULL,NULL);
    saf_describe_field_tmpl(SAF_ALL, &l_tmpl, NULL,NULL,NULL, &rightquantity, NULL,NULL);
  }

  /*jake note: XXX havent ever checked these sections that write to a third file*/
  if(g_globalfo.store.anything)
    {
      SAF_Suite newsuite;
      SAF_FieldTmpl leftfieldtmpl;
      SAF_Set l_newSet = SS_SET_NULL;

      saf_describe_unit(SAF_ALL, &leftunit, NULL,NULL,NULL,NULL,NULL,NULL,NULL,&leftquantity);

      if(!g_sets_first)
	{
	  int num_cats;
	  SAF_Cat *cats=NULL, newcat;
	  /* !g_sets_first means suites must be first.  So we need to declare the categories before
	     the call to saf_declare_suites because it will declare a category if it can't find one*/
	  saf_find_categories(SAF_ALL, g_leftdb, SAF_UNIVERSE(g_leftdb), SAF_ANY_NAME, SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_cats, &cats);
	  for(i=0; i<num_cats; i++){
	    saf_copy_category(cats+i, &newcat, g_newdb);
	  }
	}

      /*find the mesh set in the new database to go with this suite*/
      {
	char *l_leftMeshSetName=0;
	int l_numFound=0,l_maxTopoDim=0;
	SAF_Set *l_sets=0;
	SAF_SilRole l_role;
	SAF_ExtendMode l_extend;
	SAF_TopMode l_topmode;

        if( !SS_SET(l_leftMeshSpace) )
        {
	      printf("error compare_suites: not a valid set handle\n");
	      exit(-1);
        }

	saf_describe_set(SAF_ALL,l_leftMeshSpace,&l_leftMeshSetName,&l_maxTopoDim,&l_role,
			 &l_extend,&l_topmode,NULL,NULL);
	saf_find_matching_sets(SAF_ALL, g_newdb, l_leftMeshSetName, l_role, l_maxTopoDim,
			       l_extend,l_topmode, &l_numFound, &l_sets);
	if(l_numFound==1)
	  {
	    l_newSet = l_sets[0];
	  }
      }

      if( !SAF_EQUIV(&l_newSet,NULL) )
	{	
          int set_value=1;
	  saf_declare_suite(SAF_ALL,g_newdb,left_suitename,&l_newSet,NULL,&newsuite);


	  /*the above call to saf_declare_suite has the side effect of declaring a field template and a field*/
	  saf_describe_field(SAF_ALL, &leftcoords, &leftfieldtmpl, NULL, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
			     NULL,NULL,NULL,NULL,NULL);	

          _saf_htab_insert(g_field_tmpls_written, _saf_hkey_str(safdiff_strdup(field_tmpl_name(&leftfieldtmpl))), (ss_pers_t *)&set_value);
          _saf_htab_insert(g_fields_written,      _saf_hkey_str(safdiff_strdup(field_name(&leftcoords))),    (ss_pers_t *)&set_value);

	}
    }



  print_break(PRINT_LEVEL_3,"Next suite");
  print_string(PRINT_LEVEL_3,"suite name", left_suitename, right_suitename);
  g_indent++;
  print_int(PRINT_LEVEL_3,"suite dim", leftdim, rightdim,0,0);


  print_break(PRINT_LEVEL_3,"Default coordinate field");
  compare_fields(&leftcoords, &rightcoords);


  compare_attributes_old_handles((ss_pers_t*)left, (ss_pers_t*)right, 0,0);


  free(left_suitename);
  free(right_suitename);

  END_OF_COMPARE_FUNCTION
    return 0;
}/*compare_suites*/




/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two field templates.
 *
 * Description: Compare two field templates.  
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_field_templates(SAF_FieldTmpl *left, SAF_FieldTmpl *right)
{
  char *leftname=NULL, *rightname=NULL;
  SAF_Algebraic leftalg_type, rightalg_type;
  SAF_Basis leftbasis, rightbasis;
  SAF_Quantity leftquantity, rightquantity;
  int leftnum_comp=0, rightnum_comp=0;
  SAF_FieldTmpl *leftctmpl=NULL, *rightctmpl=NULL;
  int l_leftMissing = SAF_EQUIV(left,&g_missing_field_tmpl);
  int l_rightMissing = SAF_EQUIV(right,&g_missing_field_tmpl);

  saf_describe_field_tmpl(SAF_ALL, left, &leftname, &leftalg_type, &leftbasis,
			  &leftquantity, &leftnum_comp, &leftctmpl);
  saf_describe_field_tmpl(SAF_ALL, right, &rightname, &rightalg_type, &rightbasis, 
			  &rightquantity, &rightnum_comp, &rightctmpl);

  print_string(PRINT_LEVEL_3,"field template name", l_leftMissing ? "":leftname, l_rightMissing ? "":rightname);
  g_indent++;

  l_leftMissing=1;
  l_rightMissing=1;
  if (SS_ALGEBRAIC(&leftalg_type))l_leftMissing=0;
  if (SS_ALGEBRAIC(&rightalg_type))l_rightMissing=0;
  compare_algebraic(&leftalg_type, &rightalg_type,l_leftMissing,l_rightMissing);
  
  l_leftMissing=1;
  l_rightMissing=1;
  if (SS_BASIS(&leftbasis))l_leftMissing=0;
  if (SS_BASIS(&rightbasis))l_rightMissing=0;
  compare_basis(&leftbasis, &rightbasis,l_leftMissing,l_rightMissing);

  l_leftMissing=1;
  l_rightMissing=1;
  if (SS_QUANTITY(&leftquantity))l_leftMissing=0;
  if (SS_QUANTITY(&rightquantity))l_rightMissing=0;
  compare_quantity(&leftquantity, &rightquantity,l_leftMissing,l_rightMissing);

  print_int(PRINT_LEVEL_3,"num components", leftnum_comp, rightnum_comp,l_leftMissing,l_rightMissing);
  if(leftctmpl!=NULL && rightctmpl!=NULL && leftnum_comp>0 && rightnum_comp>0)
    {
      int i;
      canonicalize_field_tmpl_lists(leftctmpl, rightctmpl, (size_t)leftnum_comp, (size_t)rightnum_comp);
      PRINT_SOME_STRINGS(PRINT_LEVEL_3,leftnum_comp, rightnum_comp, i,
			 "component field tmpl name", field_tmpl_name(leftctmpl+i), field_tmpl_name(rightctmpl+i));
    }

  compare_attributes_old_handles((ss_pers_t*)left, (ss_pers_t*)right,l_leftMissing,l_rightMissing);

  if(leftname) free(leftname);	if(rightname) free(rightname);
  if(leftctmpl) free(leftctmpl);	if(rightctmpl) free(rightctmpl);
  END_OF_COMPARE_FUNCTION;
  return 0;
}/*compare_field_templates*/


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two fields.
 *
 * Description: Compare two fields.  Compares the field data along with field properties.  Recursively compares component fields.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_fields(SAF_Field *leftfield, SAF_Field *rightfield)
{
  char *leftfield_name=NULL, *rightfield_name=NULL;
  SAF_FieldTmpl leftfield_tmpl, rightfield_tmpl;
  SAF_Unit left_unit, right_unit;
  hbool_t leftis_coord, rightis_coord;
  SAF_Cat leftstorage_decomp, rightstorage_decomp;
  SAF_Cat leftcoeff_assoc, rightcoeff_assoc;
  int leftassoc_ratio, rightassoc_ratio;
  SAF_Cat lefteval_coll, righteval_coll;
  SAF_Eval lefteval_func, righteval_func;
  hid_t leftdata_type, rightdata_type;
  int leftnum_comps=0, rightnum_comps=0;
  SAF_Field *leftcomp_flds, *rightcomp_flds;
  SAF_Interleave leftcomp_intlv, rightcomp_intlv;
  int *leftcomp_order=NULL, *rightcomp_order=NULL;
  fieldOptions *field_options=NULL;
  int l_leftMissing;
  int l_rightMissing;
  size_t limit=1;

  leftcomp_flds=NULL; rightcomp_flds=NULL;

  l_leftMissing = SAF_EQUIV(leftfield,&g_missing_field);
   l_rightMissing = SAF_EQUIV(rightfield,&g_missing_field);

  saf_describe_field(SAF_ALL, leftfield,&leftfield_tmpl, &leftfield_name, NULL,
		     &left_unit,&leftis_coord,&leftstorage_decomp,&leftcoeff_assoc,
		     &leftassoc_ratio,&lefteval_coll,&lefteval_func,
		     &leftdata_type, &leftnum_comps, NULL,&leftcomp_intlv,&leftcomp_order);
  saf_describe_field(SAF_ALL, rightfield,&rightfield_tmpl, &rightfield_name, NULL,
		     &right_unit,&rightis_coord,&rightstorage_decomp,&rightcoeff_assoc,
		     &rightassoc_ratio,&righteval_coll,&righteval_func,
		     &rightdata_type, &rightnum_comps, NULL, &rightcomp_intlv, &rightcomp_order);

  field_options = (fieldOptions *)_saf_htab_find(g_fieldOptionsHash, _saf_hkey_str(safdiff_strdup(leftfield_name)), &limit, (ss_pers_t *)field_options);
  if((int) limit > 0){
    g_save_globalfo = g_globalfo;
    g_globalfo = *field_options;
  }



  print_string(PRINT_LEVEL_3,"field name", l_leftMissing ? "":leftfield_name, 
	       l_rightMissing ? "":rightfield_name);
  g_indent++;



  print_string(PRINT_LEVEL_4,"is coord fld", l_leftMissing ? "":str_hbool_t(leftis_coord), 
	       l_rightMissing ? "":str_hbool_t(rightis_coord));	
  print_string(PRINT_LEVEL_3,"storage decomp cat name", l_leftMissing ? "":cat_name(&leftstorage_decomp), 
	       l_rightMissing ? "":cat_name(&rightstorage_decomp));
  print_int(PRINT_LEVEL_3,"association ratio", leftassoc_ratio, rightassoc_ratio,0,0);

  l_leftMissing=1;
  l_rightMissing=1;
  if (SS_UNIT(&left_unit))l_leftMissing=0;
  if (SS_UNIT(&right_unit))l_rightMissing=0;
  compare_units(&left_unit, &right_unit, l_leftMissing, l_rightMissing);

  print_string(PRINT_LEVEL_3,"eval cat name", l_leftMissing ? "":cat_name(&lefteval_coll),
	       l_rightMissing ? "":cat_name(&righteval_coll));

  /* compare_eval(&lefteval_func, &righteval_func, l_leftMissing, l_rightMissing); */
  compare_hid_ts(leftdata_type, rightdata_type, l_leftMissing, l_rightMissing);
  compare_interleave(&leftcomp_intlv, &rightcomp_intlv, l_leftMissing, l_rightMissing);

  if(leftcomp_order!=NULL && rightcomp_order!=NULL){
    print_int_array(PRINT_LEVEL_3,PRINT_LEVEL_3,"component order", leftcomp_order, rightcomp_order, 
		    (size_t)leftnum_comps, (size_t)rightnum_comps);
  }
  else if(leftcomp_order!=NULL && rightcomp_order==NULL){
    print_int_array(PRINT_LEVEL_3,PRINT_LEVEL_3,"component order", leftcomp_order, leftcomp_order, 
		    (size_t)leftnum_comps, 0);
  }
  else if(leftcomp_order==NULL && rightcomp_order!=NULL){
    print_int_array(PRINT_LEVEL_3,PRINT_LEVEL_3,"component order", rightcomp_order, rightcomp_order, 
		    0, (size_t)rightnum_comps);
  }
  compare_field_templates(&leftfield_tmpl, &rightfield_tmpl);
  compare_field_data(leftfield, rightfield);/*note: this causes the field name to be repeated! need to fix*/

  if(leftcomp_flds!=NULL && rightcomp_flds!=NULL){
    int i;
    /* recursively compare component fields */

#ifdef USE_EXTRA_SORT
    /*jake test 021803 pre-sort the fields*/
    safdiff_sort((ss_pers_t *)(leftcomp_flds), (size_t)leftnum_comps, sizeof(SAF_Field), sort_compare_fields);
    safdiff_sort((ss_pers_t *)(rightcomp_flds), (size_t)rightnum_comps, sizeof(SAF_Field), sort_compare_fields);
#endif


    canonicalize_field_lists(leftcomp_flds, rightcomp_flds, (size_t)leftnum_comps, (size_t)rightnum_comps);
    PRINT_SOME_STRINGS(PRINT_LEVEL_3,leftnum_comps, rightnum_comps, i, "component fields",  
		       field_name(leftcomp_flds+i),field_name(rightcomp_flds+i));
    compare_field_lists(leftcomp_flds, rightcomp_flds, (size_t)leftnum_comps, (size_t)rightnum_comps);
  }

  if(leftfield_name) free(leftfield_name);	if(rightfield_name) free(rightfield_name);
  if(leftcomp_flds) free(leftcomp_flds); 	if(rightcomp_flds) free(rightcomp_flds);
  if(leftcomp_order) free(leftcomp_order);	if(rightcomp_order) free(rightcomp_order);

  g_globalfo = g_save_globalfo; /*restore global field options*/
  END_OF_COMPARE_FUNCTION;
  return 0;
}/*compare_fields*/


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Writing a New Database
 * Purpose:    	Write out new field templates
 *
 * Description: the idea of this function is to declare the component field templates *before* 
 * 		the actual field template.
 *		the component field templatess need to be already built before the call to saf_declare_field_tmpl
 *
 * Issues:	this function is a mess because saf_describe_field_templates does not return the component field templates
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL December 7, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int build_field_tmpl(SAF_Field *leftfield, SAF_FieldTmpl *leftfield_tmpl, SAF_FieldTmpl *newfield_tmpl)
{
  SAF_ENTER(build_field_tmpl,SAF_PRECONDITION_ERROR);

    int set_value=1;
  SAF_FieldTmpl *newfield_tmpls=NULL;
  /*originally parameters*/
  SAF_Set *newbase_space;
  int i;
  /*saf_describe_field_tmpl*/
  char *leftname=NULL;
  SAF_Set leftbase_space;
  SAF_Algebraic leftalg_type;
  SAF_Basis leftbasis;
  SAF_Quantity leftquantity;
  int leftnum_comp=0;
  SAF_FieldTmpl *leftctmpl=NULL;
	
  /*saf_find_field_tmpls*/
  int numfoundfield_tmpls;
  SAF_FieldTmpl *foundfield_tmpls=NULL;

  /*here since saf_describe_field_tmpl is broken*/
  /*saf_describe_field*/
  SAF_Field *leftcomp_flds=NULL;
  int leftnum_compfields;
  SAF_Unit leftunit;
  ss_pers_t *options=NULL;
  size_t limit;

  /*uniquely determine the field template, so we don't add it again*/

  describe_field_tmpl_new(SAF_ALL, leftfield, &leftname, &leftbase_space, &leftalg_type, &leftbasis,
			      &leftquantity, &leftnum_comp, leftctmpl);

  newbase_space=(SAF_Set *)malloc(1*sizeof(SAF_Set));
  saf_copy_set(SAF_ALL, &leftbase_space, newbase_space, g_newdb);

  options = _saf_htab_find(g_field_tmpls_written, _saf_hkey_str(safdiff_strdup(field_tmpl_name(leftfield_tmpl))), &limit, options);

  if(limit > 0){
    /*Fidld Template already exists in g_newdb, translate left field tmpl handle into new field tmpl handle*/


    find_field_tmpls_new(SAF_ALL,g_newdb,leftname, &leftalg_type,
			     &leftbasis, NULL, leftnum_comp,&numfoundfield_tmpls, foundfield_tmpls);
    if(numfoundfield_tmpls>0 )
      {
	if(numfoundfield_tmpls>1 && g_printLevel>=PRINT_LEVEL_3)
	  {
	    printf("(Note: found %d equivalent field templates in new database - waste of space)\n",numfoundfield_tmpls);
	    for(i=0; i<numfoundfield_tmpls; i++){
	      printf("   (name %s)\n", field_tmpl_name(foundfield_tmpls+i));
	    }
	  }
	(*newfield_tmpl)=foundfield_tmpls[0];
      }else{
	int count;
	SAF_FieldTmpl *ftmpls=NULL;

	saf_find_field_tmpls(SAF_ALL,g_newdb,SAF_ANY_NAME,SAF_ALGTYPE_ANY,SAF_ANY_BASIS,NULL,
			     &count, &ftmpls);

	printf("Here are all of the field templates in the new database:\n");
	for(i=0; i<count; i++){
	  sprint(field_tmpl_name(ftmpls+i));
	}

	printf("We expected to find field template %s already declared in the new database\n", leftname);
	exit_safdiff(SAFDIFF_ERROR);
      }
  }else{
    /*Field Template does not exist in new db, declare new field template in g_newdb*/

    saf_describe_field(SAF_ALL, leftfield, leftfield_tmpl, NULL, NULL, &leftunit,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
		       &leftnum_compfields,&leftcomp_flds,NULL,NULL);

	
    if(leftnum_compfields==leftnum_comp && leftnum_comp>0 && leftctmpl!=NULL && leftcomp_flds!=NULL){
      newfield_tmpls=(SAF_FieldTmpl *)malloc(leftnum_comp*sizeof(SAF_FieldTmpl));
      for(i=0; i<leftnum_comp; i++){
	build_field_tmpl(leftcomp_flds+i,leftctmpl+i, newfield_tmpls+i);
      }
    }
		
    /*should I be doing this?, i'm overwriting the quantity defined on the field template, with
      the quantity of the unit defined on the field*/
    saf_describe_unit(SAF_ALL, &leftunit, NULL,NULL,NULL,NULL,NULL,NULL,NULL,&leftquantity);

    saf_declare_field_tmpl(SAF_ALL,g_newdb, leftname,/* *newbase_space, */&leftalg_type, &leftbasis,&leftquantity,leftnum_comp,
			   newfield_tmpls, newfield_tmpl);
    _saf_htab_insert(g_field_tmpls_written,      _saf_hkey_str(safdiff_strdup(field_tmpl_name(leftfield_tmpl))),    (ss_pers_t *)&set_value);


  }
  free(leftname);	
  free(leftctmpl);
  free(foundfield_tmpls);
  free(leftcomp_flds);

  SAF_LEAVE(0);
}
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/

#define SAF_DECLARE_FIELD_HERE(label)											\
{															\
                int set_value = 1;                                                                                      \
		sprintf(newfield_name, "%s%s", leftfield_name, label);							\
		saf_declare_field(SAF_ALL,newfile, newfield_tmpl, newfield_name, &newbase_space, &left_unit, &newstorage_decomp, \
			&newcoeff_assoc, leftassoc_ratio, &neweval_coll, &lefteval_func, left_type, newcomp_fields,	\
			leftcomp_intlv, leftcomp_order, NULL, newfield);						\
                _saf_htab_insert(g_fields_written,      _saf_hkey_str(safdiff_strdup(newfield_name)),    (ss_pers_t *)&set_value); \
}

#define DEFINE_NEW_FIELD(type, new_def)				\
{								\
	type *lefttyped_buf=(type *)left_buf;			\
	type *righttyped_buf=(type *)right_buf;			\
	type *newtyped_buf=(type *)new_buf;			\
	int min_count=MIN((int)left_count, (int)right_count);	\
	for(i=0; i<min_count; i++){				\
		new_def						\
	}							\
}

#define	FIND_THE_RIGHT_TYPE(new_def)				\
{								\
        if (left_type > 0) {                                    \
	  if(H5Tequal(left_type,SAF_DOUBLE)){				\
		DEFINE_NEW_FIELD(double, new_def);		\
	  }							\
	  else if(H5Tequal(left_type,SAF_FLOAT)){				\
		DEFINE_NEW_FIELD(float, new_def);		\
	  }							\
	  else if(H5Tequal(left_type,SAF_LONG)){				\
		DEFINE_NEW_FIELD(long, new_def);		\
	  }							\
	  else if(H5Tequal(left_type,SAF_INT)){				\
		DEFINE_NEW_FIELD(int, new_def);			\
	  }							\
	}							\
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Writing a New Database
 * Purpose:    	Write out new fields.
 *
 * Description: the idea of this function is to declare the component fields *before* 
 * 		the actual field.
 *		the component fields need to be already built before the call to saf_declare_field
 *		Once the component fields are taken care of, write out the new fields.
 *		Deals with coordinate fields differently than non-coordinate fields.
 *		Can write out 1. diffs, 2. relative difffs, 3. 0/1 threshold map, 4. left 5. right 6. ave
 *
 * Issues:	
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL December 7, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int build_fields(SAF_Field *leftfield, SAF_Field *rightfield, SAF_Field *newfield)
{

  SAF_ENTER(build_fields,SAF_PRECONDITION_ERROR);

  /*saf_describe_field*/
  SAF_FieldTmpl leftfield_tmpl;
  char *leftfield_name=NULL;
  SAF_Unit left_unit;
  hbool_t leftis_coord;
  SAF_Cat leftstorage_decomp;
  SAF_Cat leftcoeff_assoc;
  int leftassoc_ratio;
  SAF_Cat lefteval_coll;
  SAF_Eval lefteval_func;
  hid_t leftdata_type;
  int leftnum_comps;
  SAF_Field *leftcomp_fields=NULL, *newcomp_fields=NULL;
  SAF_Interleave leftcomp_intlv;
  int *leftcomp_order=NULL;

  /*saf_declare_field*/
  SAF_Db *newfile;
  SAF_Cat newstorage_decomp, newcoeff_assoc, neweval_coll;
  int i;

  /*saf_describe_field_tmpl*/
  SAF_FieldTmpl *newfield_tmpls=NULL;
  SAF_FieldTmpl *newfield_tmpl;
  int newnumfield_tmpls;
  char *leftfield_tmplname=NULL;
  SAF_Set leftbase_space,newbase_space;
  SAF_Algebraic leftalg_type;
  SAF_Basis leftbasis;
  SAF_Quantity left_quantity;
  int leftnumcompfield_tmpls=0;
  int newnumcompfield_tmpls=0;

  /*get_count_and_type_for_field*/
  size_t left_count=0, right_count=0;
  hid_t left_type, right_type;

  /*saf_read_field*/
  void *left_buf=NULL, *right_buf=NULL, *new_buf=NULL;
  char newfield_name[256];
  size_t limit;
	
  int newnumfound_fields=0;
  SAF_Field *newfound_fields=NULL;
  fieldOptions *field_options=NULL;
  ss_pers_t *options=NULL;

  saf_describe_field(SAF_ALL, leftfield,&leftfield_tmpl, &leftfield_name,&leftbase_space, &left_unit,&leftis_coord,
		     &leftstorage_decomp,&leftcoeff_assoc,&leftassoc_ratio,&lefteval_coll,&lefteval_func,
		     &leftdata_type,&leftnum_comps,&leftcomp_fields,&leftcomp_intlv,&leftcomp_order);

  saf_describe_field_tmpl(SAF_ALL, &leftfield_tmpl, &leftfield_tmplname, /* &leftbase_space, */
			  &leftalg_type,&leftbasis,&left_quantity,&leftnumcompfield_tmpls,NULL);

  /*if this field has its own special tolerances and options, use those */
  limit=1; 
  field_options =(fieldOptions *)_saf_htab_find(g_fieldOptionsHash, _saf_hkey_str(safdiff_strdup(leftfield_name)), &limit, (ss_pers_t *)field_options);
  if(limit > 0){
    g_save_globalfo = g_globalfo;
    g_globalfo = *field_options;
  }


  /*Translates leftbase_space into newbase_space*/
  saf_copy_set(SAF_ALL, &leftbase_space, &newbase_space, g_newdb);

  options = _saf_htab_find(g_fields_written, _saf_hkey_str(safdiff_strdup(leftfield_name)), &limit, (ss_pers_t *)options);
  if(limit > 0){
    int foundit=0;


#if 1
    saf_find_fields(SAF_ALL, g_newdb, SAF_UNIVERSE(g_newdb), SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
		    SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &newnumfound_fields, &newfound_fields);
#else
    saf_find_fields(SAF_ALL, g_newdb, SAF_UNIVERSE(g_newdb), SAF_ANY_NAME, left_quantity, leftalg_type, leftbasis,
		    left_unit, newcoeff_assoc, leftassoc_ratio, neweval_coll, lefteval_func, &newnumfound_fields, &newfound_fields);
#endif

    if(newnumfound_fields > 0 && newfound_fields!=NULL){
      for(i=0; i<newnumfound_fields; i++){
	char *fldname = field_name(newfound_fields+i);
	char *p=strrchr(fldname, '*');

	/*this takes a fieldname of the form "velocity *diffs*" and chops off the last " *....*", to give "velocity"*/
	if(p!=NULL){
	  *p='\0';
	  p=strrchr(fldname, '*');
	  if(p!=NULL){
	    p--;
	    *p='\0';
	  }
	}
	if(!strcmp(fldname, leftfield_name)){
	  (*newfield)=newfound_fields[i];
	  foundit=1;
	  break;
	}
      }
    }
    if(!foundit){
      iprint(newnumfound_fields);
      printf("Expected to find a field by the name of %s in the new database\n", leftfield_name);
      exit_safdiff(SAFDIFF_ERROR);
    }

    free(newfound_fields);
  }else{/*we have not written this field yet*/


    if(leftnum_comps>0 && leftcomp_fields!=NULL){
      newcomp_fields=(SAF_Field *)malloc(leftnum_comps*sizeof(SAF_Field));
      for(i=0; i<leftnum_comps; i++){
	build_fields(leftcomp_fields+i, rightfield, &newcomp_fields[i]);
      }
		
    }

    /*********************************************************************************/
    /*Translates leftfield_tmpl into newfield_tmpl*/

    find_field_tmpls_new(SAF_ALL,g_newdb,leftfield_tmplname,&leftalg_type,&leftbasis, 
			     NULL, leftnumcompfield_tmpls, &newnumfield_tmpls, newfield_tmpls);
    if(newnumfield_tmpls>0){
      /*It found the field template already in the new database, we don't need to build it*/
    }else{ 
      /*The field template was not yet in the new db, so create it there*/
      build_field_tmpl(leftfield, &leftfield_tmpl, newfield_tmpl);

      find_field_tmpls_new(SAF_ALL,g_newdb,leftfield_tmplname,&leftalg_type,&leftbasis, 
			       NULL, leftnumcompfield_tmpls, &newnumfield_tmpls, newfield_tmpls);

    }
    if(newnumfield_tmpls>0)
      {
	newfield_tmpl=newfield_tmpls;

	saf_describe_field_tmpl(SAF_ALL, newfield_tmpl, NULL, NULL,NULL,&left_quantity,&newnumcompfield_tmpls,NULL);
	if(newnumfield_tmpls>1 && g_printLevel>=PRINT_LEVEL_3)
	  {
	    printf("(Note: found %d equivalent field templates in new database - waste of space)\n",newnumfield_tmpls);
	    for(i=0; i<newnumfield_tmpls; i++){
	      printf("   (name %s)\n", field_tmpl_name(newfield_tmpls+i));
	    }
	  }
	
      }else{ 
	printf("error, expected to find a valid field template named %s\n", field_tmpl_name(&leftfield_tmpl));
	exit_safdiff(SAFDIFF_ERROR);
      }
    /*********************************************************************************/
	
    saf_get_count_and_type_for_field(SAF_ALL, leftfield, NULL, &left_count, &left_type);
    saf_get_count_and_type_for_field(SAF_ALL, rightfield, NULL, &right_count, &right_type);

    if(right_count>0 && right_type>0){
      saf_read_field(SAF_ALL, rightfield, NULL, SAF_WHOLE_FIELD, (void **)&right_buf);
    }


    /*make sure these "new" objects are associated with the "new" database*/
    newfile=g_newdb;
    saf_copy_category(&leftstorage_decomp, &newstorage_decomp, g_newdb);
    saf_copy_category(&leftcoeff_assoc, &newcoeff_assoc, g_newdb);
    saf_copy_category(&lefteval_coll, &neweval_coll, g_newdb);
    saf_copy_collection(SAF_ALL, &leftbase_space, &lefteval_coll, &newbase_space, &neweval_coll);
    saf_copy_collection(SAF_ALL, &leftbase_space, &leftcoeff_assoc, &newbase_space, &newcoeff_assoc);
    saf_copy_collection(SAF_ALL, &leftbase_space, &leftstorage_decomp, &newbase_space, &newstorage_decomp);


    if(left_type == SAF_HANDLE && left_count>0)
      {
	SAF_DECLARE_FIELD_HERE(""); 

	if(g_debugging){
	  sprint("here we are at the first time we get to this handle stufff");
	  sprint(cat_name(&leftstorage_decomp));
	  sprint(cat_name(&newstorage_decomp));
	  sprint(StringDataType(leftdata_type));
	  sprint(StringDataType(left_type));
	  sprint(str_SAF_Algebraic(&leftalg_type));
	}



	/*we have an indirect field, a field of fields, so write it out*/
	/*we can't just read it from the left db and write it to the new db, we need to translate the left fields into new fields*/
	saf_read_field(SAF_ALL, leftfield, NULL, SAF_WHOLE_FIELD, (void **)&left_buf);
	{
	  size_t k;
	  SAF_Field *newfields, *leftreadfields;
	  newfields=(SAF_Field *)malloc(left_count * sizeof(SAF_Field));
	  leftreadfields=(SAF_Field*)left_buf;

	  for(k=0; k<left_count; k++){
	    /*NOTE: THIS IS WRONG, I AM PASSING leftreadfields[k] when I should be passing the corresponding RIGHT field*/
	    build_fields(leftreadfields+k, leftreadfields+k, newfields+k);
	  }
	  saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&newfields, newfile);
	}
      }else{/*the left_type is not SAF_HANDLE, so it is not an indirect field*/


	if(left_count>0 && left_type>0 ){
	  size_t memsize;
	  saf_read_field(SAF_ALL, leftfield, NULL, SAF_WHOLE_FIELD, (void **)&left_buf);
	  memsize=left_count *  H5Tget_size(left_type);
	  new_buf=(void *)malloc(memsize);
	  memcpy(new_buf,left_buf, memsize);

	  if(leftis_coord){

	    SAF_DECLARE_FIELD_HERE("");

	    if(left_type>0 && left_type==right_type && left_count > 0){
	      if(!strcmp(g_coord_field_string,"ave")){
		FIND_THE_RIGHT_TYPE({newtyped_buf[i] = (righttyped_buf[i] + lefttyped_buf[i])/2;} );
		saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&new_buf, newfile);
	      }else if(!strcmp(g_coord_field_string,"right")){
				/*FIND_THE_RIGHT_TYPE({newtyped_buf[i] = righttyped_buf[i];} );*/
		memsize= H5Tget_size(left_type) *MIN(left_count, right_count);
		memcpy(new_buf, right_buf, memsize);
		saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&new_buf, newfile);
	      }else{/*use left coords if above 2 don't match*/
		saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&left_buf, newfile);
	      }
	      saf_declare_coords(SAF_ALL,newfield);

	    }		

	  }
	  /*else*/{  /*we want to write out diffs and others for coordinate fields too*/
	    if(g_globalfo.store.diffs)
	      {
		/*problems here, monday night  XXX???????????????? who wrote this message and when?*/
		SAF_DECLARE_FIELD_HERE(" *diffs*");
		if(left_type>0 && left_type==right_type && left_count > 0){
		  FIND_THE_RIGHT_TYPE({newtyped_buf[i] = righttyped_buf[i] - lefttyped_buf[i];} );
		  saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&new_buf, newfile);
		}
	      }
	    if(g_globalfo.store.rel_diff){
	      SAF_DECLARE_FIELD_HERE(" *rel_diff*");
	      if(left_type>0 && left_type==right_type && left_count > 0){
		FIND_THE_RIGHT_TYPE({newtyped_buf[i] = PER_CHANGE((double)(lefttyped_buf[i]),
								  (double)(righttyped_buf[i])); } );
		saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&new_buf, newfile);
	      }
	    }
	    if(g_globalfo.store.map){
	      SAF_DECLARE_FIELD_HERE(" *map*");
	      if(left_type>0 && left_type==right_type && left_count > 0){      
		FIND_THE_RIGHT_TYPE(
		{
		  int outside_threshold;
		  double per_change=PER_CHANGE((double)(lefttyped_buf[i]),(double)(righttyped_buf[i]));
		  OUTSIDE_THRESHOLD(g_globalfo.absrel_andor, g_globalfo.abs_tol, g_globalfo.rel_tol, 
				    g_globalfo.floor_tol, per_change, 
				    lefttyped_buf[i], righttyped_buf[i], outside_threshold);
		  if(outside_threshold){ newtyped_buf[i] = 1;	}else{	newtyped_buf[i] = 0;	}

		});
		saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&new_buf, newfile);
	      }
	    }
	    if(g_globalfo.store.left){
	      SAF_DECLARE_FIELD_HERE(" *left*");
	      if(left_type>0 && left_type==right_type && left_count > 0){
		saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&left_buf, newfile);
	      }
	    }
	    if(g_globalfo.store.right){
	      SAF_DECLARE_FIELD_HERE(" *right*");
	      if(left_type>0 && left_type==right_type && left_count > 0){
				/*FIND_THE_RIGHT_TYPE({newtyped_buf[i] = righttyped_buf[i];} );*/
		memsize= H5Tget_size(left_type) *MIN(left_count, right_count);
		memcpy(new_buf, right_buf, memsize);
		saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&new_buf, newfile);
	      }

	    }
	    if(g_globalfo.store.ave){
	      SAF_DECLARE_FIELD_HERE(" *ave*");
	      if(left_type>0 && left_type==right_type && left_count > 0){
		FIND_THE_RIGHT_TYPE({newtyped_buf[i] = (righttyped_buf[i] + lefttyped_buf[i])/2;} );
		saf_write_field(SAF_ALL, newfield, SAF_WHOLE_FIELD,1,H5I_INVALID_HID,(void **)&new_buf, newfile);
	      }
	    }
	    if(g_globalfo.store.exclusive){
	      SAF_DECLARE_FIELD_HERE(" *exclusive*");
	    }
	  }/*!leftis_coord, i.e. end of non-coordinate field block*/

	}else{
	  /*we have a component field that does not have any data, do not modify its name*/
	  /*not if(left_count>0 && left_type!=NULL){*/

	  SAF_DECLARE_FIELD_HERE("");
	}

      }/*end of block that deals with "direct" fields, i.e. left_type!=SAF_HANDLE */
 
    /******************************************************************/
    if(g_debugging){
      printf("New field name %s\n", field_name(newfield));
      if(leftcomp_fields!=NULL){
	for(i=0; i<leftnum_comps; i++){
	  sprint(field_name(leftcomp_fields+i));
	}
      }
    }
    /******************************************************************/

  }/*end of block that deals with previously unwritten field*/

  free(newfield_tmpls);
  free(newcomp_fields);
  free(left_buf);		free(right_buf);	free(new_buf);
  g_globalfo = g_save_globalfo;

  SAF_LEAVE(0);
}/*build_fields*/


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/


/*write out fields to new database*/

#define CREATE_LABEL(label,prefix,fieldname,left_count,leftcoeff_assocname,						\
		indexwidth,i,leftassoc_ratio,leftnum_comps,leftcomp_fieldname)						\
{															\
  size_t jj;  														\
  if( leftcomp_intlv == SAF_INTERLEAVE_VECTOR && rightcomp_intlv == SAF_INTERLEAVE_VECTOR){				\
	jj = (i/leftassoc_ratio) % leftnum_comps;									\
	sprintf(label, "%s%s[%*i] %s", prefix,leftcoeff_assocname, 							\
		(int)indexwidth,(int)((i/leftassoc_ratio)/leftnum_comps), leftcomp_fieldname[jj]);			\
  }else if( leftcomp_intlv == SAF_INTERLEAVE_COMPONENT && rightcomp_intlv == SAF_INTERLEAVE_COMPONENT){                	\
	alternating_width = (int)left_count / leftnum_comps;								\
	jj = (i/leftassoc_ratio) / alternating_width;									\
	sprintf(label, "%s%s[%*i] %s", prefix,leftcoeff_assocname, 							\
		(int)indexwidth,(int)((i/leftassoc_ratio) % alternating_width), leftcomp_fieldname[jj]);		\
  }else if (leftcomp_intlv != rightcomp_intlv ){                                                                        \
	sprintf(label, "%s%s[%*i] %s", prefix,leftcoeff_assocname, 							\
		(int)indexwidth,(int)i, fieldname);									\
  }else{ /*SAF_INTERLEAVE_NONE, scalar field, both have the same type*/							\
	sprintf(label, "%s%s[%*i] %s", prefix,leftcoeff_assocname, 							\
		(int)indexwidth,(int)i, fieldname);									\
  }															\
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/

#define PRINT_FIELD(print_type, DUMMY_VALUE,left_count,right_count)							\
{															\
	size_t min_count, k;                                                                               		\
	double diff, maxdiff=0, maxreldiff=0, reldiff;                                                			\
	char **leftcomp_fieldname, **rightcomp_fieldname;								\
	char label[128];												\
	int j,indexwidth;												\
	int alternating_width;												\
	indexwidth = (int)(log10((double)MAX(left_count, right_count)) + 1);						\
	leftcomp_fieldname = (char **)malloc(leftnum_comps * sizeof(char*));						\
	rightcomp_fieldname = (char **)malloc(rightnum_comps * sizeof(char*));						\
	alternating_width = (int)left_count/leftnum_comps;								\
					       										\
	if(leftcomp_fields!=NULL)											\
	for(j=0; j<leftnum_comps; j++){											\
                /*The reason we need to strdup these strings is because the functions xxx_name                          \
		  all cyclically recyle the returned strings, so the value can change over time.                        \
		  We want to make sure we have our own copy of the strings                                              \
		*/                                                                                                      \
		leftcomp_fieldname[j]=safdiff_strdup(field_name(leftcomp_fields+j));						\
	}														\
	if(rightcomp_fields!=NULL)											\
	for(j=0; j<rightnum_comps; j++){										\
		rightcomp_fieldname[j]=safdiff_strdup(field_name(rightcomp_fields+j));						\
	}														\
	min_count=MIN(left_count, right_count);                                                 			\
	g_printed_lines=0;												\
	if( g_printLevel<PRINT_LEVEL_5 ){                                       		                        \
                /*JUST PRINT OUT MAX DIFF FOR FIELD*/ 									\
		indexwidth = 1;												\
		for(k=0; k<min_count; k++){                                                             		\
			diff = fabs((double)(right_buftype[k]-left_buftype[k]));                                        \
			reldiff=fabs(PER_CHANGE((double)(left_buftype[k]),(double)(right_buftype[k])));                 \
			if(diff > maxdiff) 		maxdiff=diff;                                   		\
			if(reldiff > maxreldiff) 	maxreldiff=reldiff;                             		\
		}                                                                                       		\
		for(k=0; k<min_count; k++){                                                             		\
			diff = fabs((double)(right_buftype[k]-left_buftype[k]));                                        \
			reldiff=fabs(PER_CHANGE((double)(left_buftype[k]),(double)(right_buftype[k])));                 \
			if(diff == maxdiff){                                                            		\
				/*don't need to print banner for only 1 line of data, this info is right above it anyway*/ \
				/*IF_SCREENFULL_PRINT_BANNER*/								\
				CREATE_LABEL(label,"max diff ",leftfield_name,left_count,leftcoeff_assocname,		\
					indexwidth,k,leftassoc_ratio,leftnum_comps,leftcomp_fieldname); 		\
				print_type(PRINT_LEVEL_4,label, left_buftype[k], right_buftype[k],0,0);       	\
				break;/*in case there are multiple entries with this value*/ \
			}                                                                               		\
			if(reldiff == maxreldiff){                                                      		\
				/*don't need to print banner for only 1 line of data, this info is right above it anyway*/ \
				/*IF_SCREENFULL_PRINT_BANNER*/								\
				CREATE_LABEL(label,"max rel diff ",leftfield_name,left_count,leftcoeff_assocname,	\
					indexwidth,k,leftassoc_ratio,leftnum_comps,leftcomp_fieldname);			\
				print_type(PRINT_LEVEL_4,label, left_buftype[k], right_buftype[k],0,0);        \
				break;/*in case there are multiple entries with this value*/ \
			}                                                                               		\
		}                                                                                       		\
	}else 	{			        									\
	           /*PRINT OUT EVERY VALUE IN FIELD*/                                                              	\
		for(k=0; k<min_count; k++){										\
			IF_SCREENFULL_PRINT_BANNER									\
			CREATE_LABEL(label,"",leftfield_name,left_count,leftcoeff_assocname,				\
				indexwidth,k,leftassoc_ratio,leftnum_comps,leftcomp_fieldname); 			\
			print_type(PRINT_LEVEL_0/*no effect*/,label, left_buftype[k], right_buftype[k],0,0);	      	\
		}													\
		for(k=right_count; k<left_count; k++){								        \
			IF_SCREENFULL_PRINT_BANNER									\
			CREATE_LABEL(label,"",leftfield_name,left_count,leftcoeff_assocname,				\
				indexwidth,k,leftassoc_ratio,leftnum_comps,leftcomp_fieldname); 			\
			print_type(PRINT_LEVEL_0/*no effect*/,label, left_buftype[k], DUMMY_VALUE,0,1);			\
		}													\
		alternating_width = (int)right_count/rightnum_comps;							\
		for(k=left_count; k<right_count; k++){													\
			IF_SCREENFULL_PRINT_BANNER									\
			CREATE_LABEL(label,"",rightfield_name,right_count,rightcoeff_assocname,				\
				indexwidth,k,rightassoc_ratio,rightnum_comps,rightcomp_fieldname);			\
			print_type(PRINT_LEVEL_0/*no effect*/,label, DUMMY_VALUE, right_buftype[k], 1,0);		\
		}													\
															\
	}														\
	if(leftcomp_fields!=NULL)											\
		FREE_ARRAY(leftcomp_fieldname, leftnum_comps);								\
	if(rightcomp_fields!=NULL)											\
		FREE_ARRAY(rightcomp_fieldname, rightnum_comps);							\
	free(leftcomp_fieldname);											\
	free(rightcomp_fieldname);											\
}/*end PRINT_FIELD*/				



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare the *data* associated with two fields.
 *
 * Description: Compare two the *data* associated with two fields.  Does not compare other properties of the fields.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *------------------------------------------------------------------------------------------------------------------------------*/
int compare_field_data(SAF_Field *leftfield, SAF_Field *rightfield)
{
  size_t zero=0;
  SAF_Db leftfielddb;
  SAF_Db g_missing_fielddb;
  size_t limit = 1;
  size_t left_count=0, right_count=0;
  hid_t left_type=0, right_type=0;
  void *left_buf=NULL, *right_buf=NULL;
  char *leftbasespace_name=NULL, *rightbasespace_name=NULL;
  char *lefteval_collname=NULL, *righteval_collname=NULL;
  char *leftcoeff_assocname=NULL, *rightcoeff_assocname=NULL;
  SAF_Set leftbase_space, rightbase_space;
  char *leftfield_tmplname=NULL, *rightfield_tmplname=NULL;

#ifdef USE_BANNER
  int len;
#endif

  SAF_FieldTmpl leftfield_tmpl, rightfield_tmpl;
  char *leftfield_name=NULL, *rightfield_name=NULL;
  SAF_Unit left_unit, right_unit;
  hbool_t leftis_coord, rightis_coord;
  SAF_Cat leftstorage_decomp, rightstorage_decomp;
  SAF_Cat leftcoeff_assoc, rightcoeff_assoc;
  int leftassoc_ratio, rightassoc_ratio;
  SAF_Cat lefteval_coll, righteval_coll;
  SAF_Eval lefteval_func, righteval_func;
  hid_t leftdata_type=H5I_INVALID_HID, rightdata_type=H5I_INVALID_HID;
  int leftnum_comps, rightnum_comps;
  SAF_Field *leftcomp_fields=NULL, *rightcomp_fields=NULL;
  SAF_Interleave leftcomp_intlv=SAF_INTERLEAVE_NONE, rightcomp_intlv=SAF_INTERLEAVE_NONE;
  int *leftcomp_order=NULL, *rightcomp_order=NULL;
  fieldOptions *field_options=NULL;

  int l_leftMissing = SAF_EQUIV(leftfield,&g_missing_field);
  int l_rightMissing = SAF_EQUIV(rightfield,&g_missing_field);

  ss_pers_t *options=NULL;
  size_t left_limit=SS_NOSIZE;
  size_t right_limit=SS_NOSIZE;
  int set_value=1;
  options=NULL;
  options = _saf_htab_find(g_leftfields_seen,  _saf_hkey_str(safdiff_strdup(field_name(leftfield))), &left_limit, options);
  free(options);options=NULL;
  options = _saf_htab_find(g_rightfields_seen,  _saf_hkey_str(safdiff_strdup(field_name(rightfield))), &right_limit, options);
  free(options);


  if((left_limit > 0) && (right_limit > 0)){
    return 0;
  }
  _saf_htab_insert(g_leftfields_seen,      _saf_hkey_str(safdiff_strdup(field_name(leftfield))),    (ss_pers_t *)&set_value);
  _saf_htab_insert(g_rightfields_seen,      _saf_hkey_str(safdiff_strdup(field_name(rightfield))),    (ss_pers_t *)&set_value);
	
  saf_get_count_and_type_for_field(SAF_ALL, leftfield, NULL, &left_count, &left_type);
  saf_get_count_and_type_for_field(SAF_ALL, rightfield, NULL, &right_count, &right_type);


  /* Note we may not be able to get leftdata_type,leftcomp_fields if is a SAF_HANDLE */
  if( left_type == SAF_HANDLE || right_type == SAF_HANDLE )
    {
      saf_describe_field(SAF_ALL, leftfield,&leftfield_tmpl, &leftfield_name,&leftbase_space, &left_unit,&leftis_coord,
			 &leftstorage_decomp,&leftcoeff_assoc,&leftassoc_ratio,&lefteval_coll,&lefteval_func,
			 NULL,&leftnum_comps,NULL,&leftcomp_intlv,&leftcomp_order);
      saf_describe_field(SAF_ALL, rightfield,&rightfield_tmpl, &rightfield_name,&rightbase_space, &right_unit,&rightis_coord,
			 &rightstorage_decomp,&rightcoeff_assoc,&rightassoc_ratio,&righteval_coll,&righteval_func,
			 NULL,&rightnum_comps,NULL,&rightcomp_intlv,&rightcomp_order);

      /*printf("jake warning: in compare_field_data got SAF_HANDLE\n");*/
    }
  else
    {
      saf_describe_field(SAF_ALL, leftfield,&leftfield_tmpl, &leftfield_name,&leftbase_space, &left_unit,&leftis_coord,
			 &leftstorage_decomp,&leftcoeff_assoc,&leftassoc_ratio,&lefteval_coll,&lefteval_func,
			 &leftdata_type,&leftnum_comps,&leftcomp_fields,&leftcomp_intlv,&leftcomp_order);
      saf_describe_field(SAF_ALL, rightfield,&rightfield_tmpl, &rightfield_name,&rightbase_space, &right_unit,&rightis_coord,
			 &rightstorage_decomp,&rightcoeff_assoc,&rightassoc_ratio,&righteval_coll,&righteval_func,
			 &rightdata_type,&rightnum_comps,&rightcomp_fields,&rightcomp_intlv,&rightcomp_order);
    }


  /*if this field has its own special tolerances and options, use those*/
  field_options = (fieldOptions *)_saf_htab_find(g_fieldOptionsHash, _saf_hkey_str(safdiff_strdup(leftfield_name)), &limit, (ss_pers_t *)field_options);
  if(limit > 0) {
      g_save_globalfo = g_globalfo;
      g_globalfo = *field_options;
    }

  lefteval_collname=cat_name(&lefteval_coll);
  righteval_collname=cat_name(&righteval_coll);

  leftcoeff_assocname=safdiff_strdup(cat_name(&leftcoeff_assoc));
  rightcoeff_assocname=safdiff_strdup(cat_name(&rightcoeff_assoc));

  saf_describe_field_tmpl(SAF_ALL, &leftfield_tmpl, &leftfield_tmplname, NULL,NULL,NULL,NULL,NULL);
  saf_describe_field_tmpl(SAF_ALL, &rightfield_tmpl, &rightfield_tmplname, NULL,NULL,NULL,NULL,NULL);

  leftbasespace_name=set_name(&leftbase_space);
  rightbasespace_name=set_name(&rightbase_space);

#ifdef USE_BANNER
  if(left_count > right_count){
    sprintf(g_banner, "defined on %s, evaluated on %s", leftbasespace_name, lefteval_collname);
  }else{
    sprintf(g_banner, "defined on %s, evaluated on %s", rightbasespace_name, righteval_collname);
  }
  len=(int)strlen(g_banner);
  sprintf(g_banner, g_formatString.m_heading, len,len,g_banner, leftfield_name, rightfield_name,
	  "abs. diff", "rel. diff");
#endif

  if(leftfield_tmplname) free(leftfield_tmplname);
  if(rightfield_tmplname) free(rightfield_tmplname);


  print_string(PRINT_LEVEL_3,"field name", l_leftMissing ? "":leftfield_name, l_rightMissing ? "":rightfield_name);
  g_indent++;



  print_string(PRINT_LEVEL_4,"is coord fld", l_leftMissing ? "":str_hbool_t(leftis_coord), 
	       l_rightMissing ? "":str_hbool_t(rightis_coord));	

  print_string(PRINT_LEVEL_4,"base space", l_leftMissing ? "":leftbasespace_name, l_rightMissing ? "":rightbasespace_name);
  print_string(PRINT_LEVEL_4,"evaluated on", l_leftMissing ? "":lefteval_collname, l_rightMissing ? "":righteval_collname);

  compare_interleave(&leftcomp_intlv, &rightcomp_intlv, l_leftMissing, l_rightMissing);   		

  print_int(PRINT_LEVEL_4,"num components", leftnum_comps, rightnum_comps,l_leftMissing, l_rightMissing);
  if(leftcomp_fields!=NULL && rightcomp_fields!=NULL)
    {
      int i;

#ifdef USE_EXTRA_SORT
      /*jake test 021803 pre-sort the fields*/
      safdiff_sort((ss_pers_t *)(leftcomp_fields), (size_t)leftnum_comps, sizeof(SAF_Field), sort_compare_fields);
      safdiff_sort((ss_pers_t *)(rightcomp_fields), (size_t)rightnum_comps, sizeof(SAF_Field), sort_compare_fields);
#endif

      canonicalize_field_lists(leftcomp_fields, rightcomp_fields, (size_t)leftnum_comps, (size_t)rightnum_comps);
      PRINT_SOME_STRINGS(PRINT_LEVEL_4,leftnum_comps, rightnum_comps, i, "component fields", 
			 field_name(leftcomp_fields+i), field_name(rightcomp_fields+i));
    }
  print_int(PRINT_LEVEL_4,"dof count", (int)left_count, (int)right_count,l_leftMissing, l_rightMissing);
  print_string(PRINT_LEVEL_4,"type", (left_count<=0) ? "":StringDataType(left_type), (right_count<=0) ? "":StringDataType(right_type));



  left_buf=NULL;
  right_buf=NULL;


  if( left_count>0 && left_type>0 && left_type != SAF_HANDLE){
    saf_read_field(SAF_ALL, leftfield, NULL, SAF_WHOLE_FIELD, (void **)&left_buf);
  }
  if( right_count>0 && right_type>0 && right_type != SAF_HANDLE ){
    saf_read_field(SAF_ALL, rightfield, NULL, SAF_WHOLE_FIELD, (void **)&right_buf);
  }



  /*write out fields to new database*/
  ss_pers_file((ss_pers_t *)leftfield,&leftfielddb);
  ss_pers_file((ss_pers_t *)&g_missing_field,&g_missing_fielddb);

  if(g_globalfo.store.anything && !SAF_EQUIV(&leftfielddb,&g_missing_fielddb)){
    SAF_FieldTmpl newfield_tmpl;
    SAF_Field newfield;

    build_field_tmpl(leftfield, &leftfield_tmpl, &newfield_tmpl);
    build_fields(leftfield, rightfield, &newfield);
  }

  
  if (left_count > 0 && right_count > 0 && left_type>0 && right_type>0 ) {
     if(H5Tequal(left_type,SAF_FLOAT) && H5Tequal(right_type,SAF_FLOAT)){
       float *left_buftype = (float *)left_buf, *right_buftype = (float *)right_buf;
       PRINT_FIELD(print_float, DUMMY_FLOAT,left_count, right_count);
       }
     else if(H5Tequal(left_type,SAF_DOUBLE) && H5Tequal(right_type,SAF_DOUBLE)){
       double *left_buftype = (double *)left_buf, *right_buftype = (double *)right_buf;
       PRINT_FIELD(print_float, DUMMY_FLOAT,left_count, right_count);
       }
     else if(H5Tequal(left_type,SAF_INT) && H5Tequal(right_type,SAF_INT)){
       int *left_buftype = (int *)left_buf, *right_buftype = (int *)right_buf;
       PRINT_FIELD(print_int, DUMMY_INT,left_count, right_count);
       }
     else if(H5Tequal(left_type, SAF_LONG) && H5Tequal(right_type,SAF_LONG)){
       long *left_buftype = (long *)left_buf, *right_buftype = (long *)right_buf;
       PRINT_FIELD(print_long, DUMMY_INT,left_count,right_count);
       }
     else if(left_type == SAF_HANDLE && right_type == SAF_HANDLE){

       /*XXX note: SAF_HANDLE not working here....need to revisit*/
   
       /*
         SAF_Field *leftinfield=left_buf, *rightinfield=right_buf;
         compare_field_lists(leftinfield, rightinfield, left_count, right_count);
       */

       }
    }
  else if(left_count> 0 && left_type>0){
         /*print out the left array first*/
      printf("safdiff did not recognize the right type: %ld Count was %ld; Data could not be printed ", (long)right_type, (long)right_count);
      if(H5Tequal(left_type,SAF_FLOAT)){
        float *left_buftype = (float *)left_buf, *right_buftype = (float *)right_buf;
        PRINT_FIELD(print_float, DUMMY_FLOAT,left_count,zero);
       }
      else if(H5Tequal(left_type,SAF_DOUBLE)){
        double *left_buftype = (double *)left_buf, *right_buftype = (double *)right_buf;
        PRINT_FIELD(print_float, DUMMY_FLOAT,left_count, zero);
      }
      else if(H5Tequal(left_type,SAF_INT)){
        int *left_buftype = (int *)left_buf, *right_buftype = (int *)right_buf;
        PRINT_FIELD(print_int, DUMMY_INT,left_count, zero);
      }
      else if(H5Tequal(left_type, SAF_LONG)){
        long *left_buftype = (long *)left_buf, *right_buftype = (long *)right_buf;
        PRINT_FIELD(print_long, DUMMY_INT,left_count, zero);
      }
  }
  else if(right_count> 0 && right_type>0){

       /*now print out the right array*/
      printf("safdiff did not recognize the left type: %ld Count was %ld; Data could not be printed ", (long)left_type, (long)left_count);
       if(H5Tequal(right_type,SAF_FLOAT)){
         float *left_buftype = (float *)left_buf, *right_buftype = (float *)right_buf;
         PRINT_FIELD(print_float, DUMMY_FLOAT,zero,right_count);
       }
       else if(H5Tequal(right_type,SAF_DOUBLE)){
         double *left_buftype = (double *)left_buf, *right_buftype = (double *)right_buf;
         PRINT_FIELD(print_float, DUMMY_FLOAT,zero, right_count);
       }
       else if(H5Tequal(right_type,SAF_INT)){
         int *left_buftype = (int *)left_buf, *right_buftype = (int *)right_buf;
         PRINT_FIELD(print_int, DUMMY_INT,zero, right_count);
          }
/*
       else if(H5Tequal(right_type,SAF_UNSIGNED_INT)){
         unsigned int *left_buftype = (unsigned int *)left_buf, *right_buftype = (unsigned int *)right_buf;
         PRINT_FIELD(print_unsigned_int, DUMMY_INT,0, right_count);
       }
*/
       else if(H5Tequal(right_type,SAF_LONG)){
         long *left_buftype = (long *)left_buf, *right_buftype = (long *)right_buf;
         PRINT_FIELD(print_long, DUMMY_INT,zero, right_count);
       }
    }
   else if (left_count > 0){/*the datatype are the same, but unrecognized*/
       printf("safdiff did not recognize the types and cannot print data. Type: %ld Nvalues: %ld \n ", (long)left_type, (long)left_count);
    }
   else if (right_count > 0){/*the datatype are the same, but unrecognized*/
       printf("safdiff did not recognize the types and cannot print data. Types: %ld and %ld \n ", (long)right_type, (long)right_count);
    }


  compare_attributes_old_handles((ss_pers_t*)leftfield, (ss_pers_t*)rightfield,l_leftMissing,l_rightMissing);

  if(left_buf) free(left_buf);		
  if(right_buf) free(right_buf);
  if(leftfield_name) free(leftfield_name);	
  if(rightfield_name) free(rightfield_name);
  if(leftcoeff_assocname) free(leftcoeff_assocname);
  if(rightcoeff_assocname) free(rightcoeff_assocname);

  g_globalfo = g_save_globalfo;


  END_OF_COMPARE_FUNCTION;
  return 0;
}/*compare_field_data*/

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two roles.
 *
 * Description: Compare two roles.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_roles(SAF_Role *left, SAF_Role *right, int a_leftMissing, int a_rightMissing)
{
  char *leftname=NULL, *rightname=NULL;
  char *left_url=NULL, *right_url=NULL;

  if(!a_leftMissing) saf_describe_role(SAF_ALL, left, &leftname,  &left_url );
  if(!a_rightMissing) saf_describe_role(SAF_ALL, right, &rightname,  &right_url);

  print_string(PRINT_LEVEL_3,"role name", 
	       a_leftMissing ? "":leftname, a_rightMissing ? "":rightname);
  g_indent++;
  if( (left_url&&strcmp(left_url,"standard")) || (right_url&&strcmp(right_url, "standard")) ){
    print_string(PRINT_LEVEL_4,"role url",  
		 a_leftMissing ? "":left_url, a_rightMissing ? "":right_url);
  }
  print_int(PRINT_LEVEL_4,"role id", 0, 0,a_leftMissing,a_rightMissing);
	

  if(leftname) free(leftname);
  if(rightname) free(rightname);
  if(left_url) free(left_url);
  if(right_url) free(right_url);

  END_OF_COMPARE_FUNCTION;
  return 0;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two relation representations.
 *
 * Description: Compare two relation representations.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *------------------------------------------------------------------------------------------------------------------------------*/
int compare_relreps(SAF_RelRep *left, SAF_RelRep *right, int a_leftMissing, int a_rightMissing)
{
  char *leftname=NULL, *rightname=NULL;
  char *left_url=NULL, *right_url=NULL;
  int leftid, rightid;

  if(!a_leftMissing) saf_describe_relrep(SAF_ALL, left, &leftname,  &left_url, &leftid);
  if(!a_rightMissing) saf_describe_relrep(SAF_ALL, right, &rightname,  &right_url, &rightid);


  print_string(PRINT_LEVEL_3,"relrep name", 
	       a_leftMissing ? "":str_SAF_RelRep(left), a_rightMissing ? "":str_SAF_RelRep(right));
  g_indent++;

  /*	
	print_string(PRINT_LEVEL_3,"relrep name", leftname, rightname);
	print_string(PRINT_LEVEL_3,"relrep url", left_url, right_url);
	print_int(PRINT_LEVEL_3,"relrep id", leftid, rightid,0,0);
  */


  if(leftname) free(leftname);
  if(rightname) free(rightname);
  if(left_url) free(left_url);
  if(right_url) free(right_url);

  END_OF_COMPARE_FUNCTION;
  return 0;

}
#ifdef SSLIB_SUPPORT_PENDING

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two SAF_Files.
 *
 * Description: Compare two SAF_Files.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_files(SAF_Db *left, SAF_Db *right)
{

  char *leftpath=NULL, *rightpath=NULL;

  g_indent++;
  if(_saf_is_valid_file_handle(left) && _saf_is_valid_file_handle(right)){

    saf_describe_file(left, &leftpath);
    saf_describe_file(right, &rightpath);

    print_string(PRINT_LEVEL_1," file", leftpath, rightpath);

    free(leftpath);
    free(rightpath);
  }

  END_OF_COMPARE_FUNCTION
    return 0;
}
#endif /*SSLIB_SUPPORT_PENDING*/


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two quantities.
 *
 * Description: Compare two quantities.  Assumes that the quantity power array returned always returns with
 *		the quanties in the same order: time, mass, current, length, light, temperature, amount.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_quantity(SAF_Quantity *left_quan, SAF_Quantity *right_quan, int a_leftMissing, int a_rightMissing)
{

  char *left_desc=NULL, *right_desc=NULL;
  char *left_qabbr=NULL, *right_qabbr=NULL;
  char *left_qurl=NULL, *right_qurl=NULL;
  unsigned left_flags, right_flags, left_power[7], right_power[7];


  memset(left_power, 0, sizeof left_power);
  memset(right_power, 0, sizeof right_power);
  if(!a_leftMissing) saf_describe_quantity(SAF_ALL, left_quan, &left_desc, &left_qabbr, &left_qurl, &left_flags, left_power);
  if(!a_rightMissing) saf_describe_quantity(SAF_ALL, right_quan, &right_desc, &right_qabbr, &right_qurl, &right_flags, right_power);

  print_string(PRINT_LEVEL_3,"quantity desc", 
	       a_leftMissing ? "":left_desc, a_rightMissing ? "":right_desc);
  g_indent++;
  print_string(PRINT_LEVEL_4,"quantity abbr",
	       a_leftMissing ? "":left_qabbr, a_rightMissing ? "":right_qabbr);
  print_string(PRINT_LEVEL_4,"quantity url", 
	       a_leftMissing ? "":left_qurl, a_rightMissing ? "":right_qurl);

  print_unsigned_int(PRINT_LEVEL_4,"quantity flags", left_flags, right_flags,a_leftMissing,a_rightMissing);

  if(left_power[0] || right_power[0]) 	
    print_unsigned_int(PRINT_LEVEL_4,"time power", 	left_power[0], right_power[0],a_leftMissing,a_rightMissing);
  if(left_power[1] || right_power[1]) 	
    print_unsigned_int(PRINT_LEVEL_4,"mass power", 	left_power[1], right_power[1],a_leftMissing,a_rightMissing);
  if(left_power[2] || right_power[2]) 	
    print_unsigned_int(PRINT_LEVEL_4,"current power", 	left_power[2], right_power[2],a_leftMissing,a_rightMissing);
  if(left_power[3] || right_power[3]) 	
    print_unsigned_int(PRINT_LEVEL_4,"length power", 	left_power[3], right_power[3],a_leftMissing,a_rightMissing);
  if(left_power[4] || right_power[4]) 	
    print_unsigned_int(PRINT_LEVEL_4,"light power", 	left_power[4], right_power[4],a_leftMissing,a_rightMissing);
  if(left_power[5] || right_power[5]) 	
    print_unsigned_int(PRINT_LEVEL_4,"temperature power",left_power[5], right_power[5],a_leftMissing,a_rightMissing);
  if(left_power[6] || right_power[6]) 	
    print_unsigned_int(PRINT_LEVEL_4,"amount power", 	left_power[6], right_power[6],a_leftMissing,a_rightMissing);

  if(left_desc) free(left_desc);  if(right_desc) free(right_desc);
  if(left_qabbr) free(left_qabbr); if(right_qabbr) free(right_qabbr);
  if(left_qurl) free(left_qurl);  if(right_qurl) free(right_qurl);

  END_OF_COMPARE_FUNCTION;
  return 1;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two units.
 *
 * Description: Compare two units.  
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_units(SAF_Unit *left_unit, SAF_Unit *right_unit, int a_leftMissing, int a_rightMissing)
{
  int l_leftMissing=1;
  int l_rightMissing=1;
  char *left_name=NULL, *right_name=NULL;
  char *left_abbr=NULL, *right_abbr=NULL;
  char *left_url=NULL, *right_url=NULL;
  double left_scale=0, right_scale=0, left_offset=0, right_offset=0;
  double left_logbase=0, right_logbase=0, left_logcoef=0, right_logcoef=0;
  SAF_Quantity left_quan=SS_QUANTITY_NULL, right_quan=SS_QUANTITY_NULL;

  if(!a_leftMissing) saf_describe_unit(SAF_ALL, left_unit, &left_name, &left_abbr, &left_url, 
				       &left_scale, &left_offset, &left_logbase, &left_logcoef, &left_quan);

  if(!a_rightMissing) saf_describe_unit(SAF_ALL, right_unit, &right_name, &right_abbr, &right_url, 
					&right_scale, &right_offset, &right_logbase, &right_logcoef, &right_quan);

  print_string(PRINT_LEVEL_3,"unit name", 
	       a_leftMissing ? "":left_name, a_rightMissing ? "":right_name);
  g_indent++;
  print_string(PRINT_LEVEL_4,"unit abbr",  
	       a_leftMissing ? "":left_abbr, a_rightMissing ? "":right_abbr);
  print_string(PRINT_LEVEL_4,"unit url",  
	       a_leftMissing ? "":left_url, a_rightMissing ? "":right_url);

  print_float(PRINT_LEVEL_4,"scale", left_scale, right_scale, a_leftMissing, a_rightMissing );
  print_float(PRINT_LEVEL_4,"offset", left_offset, right_offset, a_leftMissing, a_rightMissing );
  print_float(PRINT_LEVEL_4,"logbase", left_logbase, right_logbase, a_leftMissing, a_rightMissing );
  print_float(PRINT_LEVEL_4,"log coef", left_logcoef, right_logcoef, a_leftMissing, a_rightMissing );


  if (SS_QUANTITY(&left_quan))l_leftMissing=0;
  if (SS_QUANTITY(&right_quan))l_rightMissing=0;
  compare_quantity(&left_quan, &right_quan,l_leftMissing,l_rightMissing);

  if(left_name) free(left_name); if(right_name) free(right_name);
  if(left_abbr) free(left_abbr); if(right_abbr) free(right_abbr);
  if(left_url) free(left_url);  if(right_url) free(right_url);

  END_OF_COMPARE_FUNCTION;
  return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two algebraic types.
 *
 * Description: Compare two algebraic types.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_algebraic(SAF_Algebraic *left, SAF_Algebraic *right, int a_leftMissing, int a_rightMissing)
{
  char *leftname=NULL, *rightname=NULL;
  char *lefturl=NULL, *righturl=NULL;
  SAF_TriState leftindr, rightindr;

  if(!a_leftMissing) saf_describe_algebraic(SAF_ALL, left, &leftname, &lefturl, &leftindr);
  if(!a_rightMissing) saf_describe_algebraic(SAF_ALL, right, &rightname, &righturl, &rightindr);

  print_string(PRINT_LEVEL_3,"algebraic name", 
	       a_leftMissing ? "":leftname, a_rightMissing ? "":rightname);
  g_indent++;
  print_string(PRINT_LEVEL_4,"url", 
	       a_leftMissing ? "":lefturl, a_rightMissing ? "":righturl);
  print_string(PRINT_LEVEL_4,"is indirect", 
	       a_leftMissing ? "":str_SAF_TriState(leftindr), 
	       a_rightMissing ? "":str_SAF_TriState(rightindr));
  print_int(PRINT_LEVEL_4,"id", 0, 0,a_leftMissing,a_rightMissing);

  if(leftname) free(leftname); if(rightname) free(rightname);
  if(lefturl) free(lefturl);  if(righturl) free(righturl);

  END_OF_COMPARE_FUNCTION;
  return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two bases.
 *
 * Description: Compare two bases.  This is a leaf procedure.  
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_basis(SAF_Basis *left, SAF_Basis *right, int a_leftMissing, int a_rightMissing)
{
  char *leftname=NULL, *rightname=NULL;
  char *lefturl=NULL, *righturl=NULL;

  if(!a_leftMissing) saf_describe_basis(SAF_ALL, left, &leftname, &lefturl);
  if(!a_rightMissing) saf_describe_basis(SAF_ALL, right, &rightname, &righturl);

  print_string(PRINT_LEVEL_4,"Basis name",  
	       a_leftMissing ? "":leftname,a_rightMissing ? "":rightname);
  g_indent++;
  print_string(PRINT_LEVEL_4,"url",  
	       a_leftMissing ? "":lefturl, a_rightMissing ? "":righturl);
  print_int(PRINT_LEVEL_4,"id", 0, 0,a_leftMissing,a_rightMissing);

  if(leftname) free(leftname); if(rightname) free(rightname);
  if(lefturl) free(lefturl);  if(righturl) free(righturl);

  END_OF_COMPARE_FUNCTION;
  return 0;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two categories.
 *
 * Description: Compare two categories.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/


int compare_cats(SAF_Cat *left_cat, SAF_Cat *right_cat)
{
  int l_leftMissing=1;
  int l_rightMissing=1;
  char *left_name=NULL, *right_name=NULL;
  SAF_Role left_role, right_role;
  int left_dim, right_dim;
  int lr, rr;

  if(!SS_CAT(left_cat) || !SS_CAT(right_cat)){
    print_error("cats are not valid");
    return 1;
  }

  lr=saf_describe_category(SAF_ALL, left_cat, &left_name, &left_role, &left_dim);
  rr=saf_describe_category(SAF_ALL, right_cat, &right_name, &right_role, &right_dim);

  if(lr!=0 || rr!=0){
    printf("non zero return for saf_describe_category\n");
    END_OF_COMPARE_FUNCTION
      return 1;
  }

  print_string(PRINT_LEVEL_3,"cat name", left_name, right_name);

  g_indent++;
  print_int(PRINT_LEVEL_4,"cat dim", left_dim, right_dim,0,0);

  if (SS_ROLE(&left_role))l_leftMissing=0;
  if (SS_ROLE(&right_role))l_rightMissing=0;
  compare_roles(&left_role,&right_role,l_leftMissing,l_rightMissing);

  l_leftMissing=1;
  l_rightMissing=1;
  if (SS_CAT(left_cat))l_leftMissing=0;
  if (SS_CAT(right_cat))l_rightMissing=0;

  compare_attributes_old_handles((ss_pers_t*)left_cat, (ss_pers_t*)right_cat, l_leftMissing,l_rightMissing);

  if(left_name) free(left_name);     
  if(right_name) free(right_name);

  END_OF_COMPARE_FUNCTION;
  return 0;
}/*compare_cats*/

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *--------------------------------------------------------------------------------------------------------------- */
const char *get_celltype_desc( SAF_CellType a_type )
{
  if( a_type == SAF_CELLTYPE_POINT ) return("SAF_CELLTYPE_POINT");
  if( a_type == SAF_CELLTYPE_HEX ) return("SAF_CELLTYPE_HEX");
  if( a_type == SAF_CELLTYPE_LINE ) return("SAF_CELLTYPE_LINE");
  if( a_type == SAF_CELLTYPE_TRI ) return("SAF_CELLTYPE_TRI");
  if( a_type == SAF_CELLTYPE_QUAD ) return("SAF_CELLTYPE_QUAD");
  if( a_type == SAF_CELLTYPE_TET ) return("SAF_CELLTYPE_TET");
  if( a_type == SAF_CELLTYPE_PYRAMID ) return("SAF_CELLTYPE_PYRAMID");
  if( a_type == SAF_CELLTYPE_PRISM ) return("SAF_CELLTYPE_PRISM");
  /* if( a_type == SAF_CELLTYPE_0BALL ) return("SAF_CELLTYPE_0BALL"); */
  if( a_type == SAF_CELLTYPE_1BALL ) return("SAF_CELLTYPE_1BALL");
  if( a_type == SAF_CELLTYPE_2BALL ) return("SAF_CELLTYPE_2BALL");
  if( a_type == SAF_CELLTYPE_3BALL ) return("SAF_CELLTYPE_3BALL");
  if( a_type == SAF_CELLTYPE_MIXED ) return("SAF_CELLTYPE_MIXED");
  if( a_type == SAF_CELLTYPE_ARB ) return("SAF_CELLTYPE_ARB");
  if( a_type == SAF_CELLTYPE_SET ) return("SAF_CELLTYPE_SET");
  if( a_type == SAF_CELLTYPE_ANY ) return("SAF_CELLTYPE_ANY");
  return("SAF_CELLTYPE_unknown");
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int compare_collections(SAF_Set *a_left_set, SAF_Cat *a_left_cat, SAF_Set *a_right_set, SAF_Cat *a_right_cat)
{
  SAF_CellType l_left_celltype,l_right_celltype;
  int l_left_count=0,l_right_count=0;
  SAF_IndexSpec l_left_ispec,l_right_ispec;
  SAF_DecompMode l_left_decomp,l_right_decomp;
  SAF_Set *l_left_member_sets=0,*l_right_member_sets=0;
  int l_leftValid = !SAF_EQUIV(a_left_cat,NULL);
  int l_rightValid = !SAF_EQUIV(a_right_cat,NULL);

  l_left_ispec.ndims=0;
  l_right_ispec.ndims=0;

  if(!l_leftValid && !l_rightValid) return(0);

  if(l_leftValid)
    {
      saf_describe_collection(SAF_ALL,a_left_set,a_left_cat,&l_left_celltype,&l_left_count,
			      &l_left_ispec,&l_left_decomp,NULL);
    }
  if(l_rightValid)
    {
      saf_describe_collection(SAF_ALL,a_right_set,a_right_cat,&l_right_celltype,&l_right_count,
			      &l_right_ispec,&l_right_decomp,NULL);
    }

  if(l_leftValid && l_rightValid)
    {
      print_string(PRINT_LEVEL_3,"collection cat name", (char *) cat_name(a_left_cat),cat_name(a_right_cat));
      print_string(PRINT_LEVEL_3,"collection cell type",get_celltype_desc(l_left_celltype),
		   get_celltype_desc(l_right_celltype));
      print_int(PRINT_LEVEL_3,"collection count", l_left_count, l_right_count,0,0);
      print_int(PRINT_LEVEL_3,"collection ispec dim", l_left_ispec.ndims, l_right_ispec.ndims,0,0 );
    }
  else if(l_leftValid)
    {
      char l_str[64];
      print_string(PRINT_LEVEL_3,"collection cat name", cat_name(a_left_cat),"");
      print_string(PRINT_LEVEL_3,"collection cell type",get_celltype_desc(l_left_celltype),"");
      sprintf(l_str,"%d",l_left_count);
      print_string(PRINT_LEVEL_3,"collection count", l_str,"");
      sprintf(l_str,"%d",l_left_ispec.ndims);
      print_string(PRINT_LEVEL_3,"collection ispec dim", l_str, "");
    }
  else
    {
      char l_str[64];
      print_string(PRINT_LEVEL_3,"collection cat name", "",cat_name(a_right_cat));
      print_string(PRINT_LEVEL_3,"collection cell type","",get_celltype_desc(l_right_celltype));
      sprintf(l_str,"%d",l_right_count);
      print_string(PRINT_LEVEL_3,"collection count","", l_str);
      sprintf(l_str,"%d",l_right_ispec.ndims);
      print_string(PRINT_LEVEL_3,"collection ispec dim","", l_str);
    }


  {
    int i;
    int l_max = MAX(l_left_ispec.ndims, l_right_ispec.ndims);

    for(i=0;i<l_max;i++)
      {
	char l_str[64];
	sprintf(l_str,"collection ispec order[%d]",i);
	if( i<l_left_ispec.ndims && i<l_right_ispec.ndims )
	  {
	    print_int(PRINT_LEVEL_3,l_str,l_left_ispec.order[i],l_right_ispec.order[i],0,0);
	  }
	else if( i<l_left_ispec.ndims )
	  {
	    char l_leftstr[64];
	    sprintf(l_leftstr,"%d",l_left_ispec.order[i]);
	    print_string(PRINT_LEVEL_3,l_str,l_leftstr,"");
	  }
	else 
	  {
	    char l_rightstr[64];
	    sprintf(l_rightstr,"%d",l_right_ispec.order[i]);
	    print_string(PRINT_LEVEL_3,l_str,"",l_rightstr);
	  }
      }
    for(i=0;i<l_max;i++)
      {
	char l_str[64];
	sprintf(l_str,"collection ispec origins[%d]",i);
	if( i<l_left_ispec.ndims && i<l_right_ispec.ndims )
	  {
	    print_int(PRINT_LEVEL_3,l_str,l_left_ispec.origins[i],l_right_ispec.origins[i],0,0);
	  }
	else if( i<l_left_ispec.ndims )
	  {
	    char l_leftstr[64];
	    sprintf(l_leftstr,"%d",l_left_ispec.origins[i]);
	    print_string(PRINT_LEVEL_3,l_str,l_leftstr,"");
	  }
	else 
	  {
	    char l_rightstr[64];
	    sprintf(l_rightstr,"%d",l_right_ispec.origins[i]);
	    print_string(PRINT_LEVEL_3,l_str,"",l_rightstr);
	  }
      }
  }


  if(l_left_member_sets) free(l_left_member_sets);     
  if(l_right_member_sets) free(l_right_member_sets);

  END_OF_COMPARE_FUNCTION;

  return 0;
}/*compare_collections*/

/*------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare two evaluations.
 *
 * Description: Compare two evaluations.  This is a leaf procedure.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *----------------------------------------------------------------------------------------------------------------------------- */
/*  
 * saf_describe_evaluation  no longer exists in sslib!!!
int compare_eval(SAF_Eval *lefteval_func, SAF_Eval *righteval_func, int a_leftMissing, int a_rightMissing)
{
  char *left_name=NULL, *right_name=NULL;
  char *left_url=NULL, *right_url=NULL;
  int left_id, right_id;

  if( (!a_leftMissing && SAF_EVALUATION!=saf_classOf_handle(lefteval_func))
      || (!a_rightMissing && SAF_EVALUATION!=saf_classOf_handle(righteval_func)) ){
    print_error("not valid eval");
    return -1;
  }

  if(!a_leftMissing) saf_describe_evaluation(lefteval_func, &left_name, &left_url, &left_id);
  if(!a_rightMissing) saf_describe_evaluation(righteval_func, &right_name, &right_url, &right_id);

  print_string(PRINT_LEVEL_3,"eval name",   
	       a_leftMissing ? "":left_name, a_rightMissing ? "":right_name);
  g_indent++;
  print_string(PRINT_LEVEL_4,"eval url",   
	       a_leftMissing ? "":left_url, a_rightMissing ? "":right_url);
  print_int(PRINT_LEVEL_4,"eval id", left_id, right_id,a_leftMissing,a_rightMissing);
	
  if(left_name) free(left_name); if(right_name) free(right_name);
  if(left_url) free(left_url);  if(right_url) free(right_url);

  END_OF_COMPARE_FUNCTION;
  return 0;
}
*/

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Copying SAF Objects
 * Purpose:     Translate a field handle in one database, to a field handle in another database.
 *
 * Description: If you have the field handle to a field in one database, and you know that same field
 *              exists in another database and you want a handle to it, call this function
 *              to get a handle to the field in the new database.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL January 22, 2002.
 *
 *-----------------------------------------------------------------------------------------------------------------------------*/
int saf_translate_field(SAF_ParMode pmode, SAF_Field *oldfield, SAF_Field *newfield, SAF_Db *newdatabase)
{


  /*saf_describe_field*/
  SAF_FieldTmpl ftmpl;
  char *name=NULL;
  SAF_Unit unit;
  hbool_t is_coord;
  SAF_Cat storage_decomp;
  SAF_Cat coeff_assoc, newcoeff_assoc;
  int assoc_ratio;
  SAF_Cat eval_coll, neweval_coll;
  SAF_Eval eval_func;
  hid_t data_type;
  int num_comps;
  SAF_Field *comp_flds=NULL;
  SAF_Interleave comp_intlv;
  int *comp_order=NULL;

  /*saf_describe_field_tmpl*/                             
  SAF_Set base, newbase;
  SAF_Algebraic atype;
  SAF_Basis basis;
  SAF_Quantity quantity;

  /*saf_find_field*/
  int numfoundfields;
  SAF_Field *foundfields=NULL;

  saf_describe_field(pmode, oldfield, &ftmpl, &name, &base, &unit, &is_coord, &storage_decomp,
		     &coeff_assoc, &assoc_ratio, &eval_coll, &eval_func, &data_type, &num_comps,
		     &comp_flds, &comp_intlv, &comp_order);
  saf_describe_field_tmpl(pmode, &ftmpl, NULL, /* &base, */  &atype, &basis, &quantity, NULL, NULL);  

  saf_copy_set(pmode, &base, &newbase, newdatabase);
  saf_copy_category(&coeff_assoc, &newcoeff_assoc, newdatabase);
  saf_copy_category(&eval_coll, &neweval_coll, newdatabase);

  saf_find_fields(pmode,newdatabase, &newbase, name, &quantity, &atype, &basis, &unit, &newcoeff_assoc, assoc_ratio, &neweval_coll,
		  &eval_func, &numfoundfields, &foundfields);

  if(numfoundfields > 0 && foundfields != NULL){
    (*newfield) = foundfields[0];
  }else{
    printf("Expected to find a field by the name of %s\n", name);
    exit_safdiff(SAFDIFF_ERROR);
  }

  free(foundfields);
  return SAF_SUCCESS;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Copying SAF Objects
 * Purpose:     Translate a field tmpl handle in one database, to a field tmpl handle in another database.
 *
 * Description: If you have the field tmpl handle to a field tmpl in one database, and you know that same field
 *              tmpl exists in another database and you want a handle to it, call this function
 *              to get a handle to the field tmpl in the new database.
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL January 22, 2002.
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int saf_translate_field_tmpl(SAF_ParMode pmode, SAF_FieldTmpl *oldfieldtmpl, SAF_FieldTmpl *newfieldtmpl, SAF_Db *newdatabase)
{

  /*saf_describe_field_tmpl*/                             
  char *name=NULL;
  SAF_Set base=SS_SET_NULL, newbase=SS_SET_NULL;
  SAF_Algebraic atype;
  SAF_Basis basis;
  SAF_Quantity quantity;

  /*saf_find_field_tmpls*/
  int num_ftmpls;
  SAF_FieldTmpl *ftmpls=NULL;

  saf_describe_field_tmpl(pmode, oldfieldtmpl, &name, /* &base, */ &atype, &basis, &quantity, NULL, NULL);  

  saf_copy_set(pmode, &base, &newbase, newdatabase);
  saf_find_field_tmpls(pmode, newdatabase, name, &atype, &basis, &quantity, &num_ftmpls, &ftmpls);


  if(num_ftmpls > 0 && ftmpls != NULL){
    (*newfieldtmpl) = ftmpls[0];
  }else{
    printf("Expected to find a field template by the name of %s\n", name);
    exit_safdiff(SAFDIFF_ERROR);
  }

  free(ftmpls);
  return SAF_SUCCESS;
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: this appears to only be called from compare_individual_attributes_old_handles ?
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *--------------------------------------------------------------------------------------------------------------*/

int saf_translate_basehandle(ss_pers_t *oldbasehandle, ss_pers_t *newbasehandle, SAF_Db *newdatabase)
{
  if (SS_CAT(oldbasehandle)) 
    saf_copy_category((SAF_Cat *)oldbasehandle, (SAF_Cat *)newbasehandle, newdatabase);
  else if (SS_FIELD(oldbasehandle))
    saf_translate_field(SAF_ALL, (SAF_Field *)oldbasehandle, (SAF_Field *)newbasehandle, newdatabase);
 else if (SS_FIELDTMPL(oldbasehandle))
    saf_translate_field_tmpl(SAF_ALL, (SAF_FieldTmpl *)oldbasehandle, (SAF_FieldTmpl *)newbasehandle, newdatabase);
 else if (SS_SET(oldbasehandle))
    saf_copy_set(SAF_ALL, (SAF_Set *)oldbasehandle, (SAF_Set *)newbasehandle, newdatabase);
 else
    {
    printf("Cannot write out attributes defined on objects of type %s\n", str_SAF_ObjectType(oldbasehandle));
    }

/*
  case   _SAF_OBJTYPE_SUITE:{
    saf_copy_suite(SAF_ALL, (SAF_Suite *)oldbasehandle, (SAF_Suite *)newbasehandle, newdatabase);
  }
  case   _SAF_OBJTYPE_FILE:
  case   _SAF_OBJTYPE_REL:
  case   _SAF_OBJTYPE_BASE:
  case   _SAF_OBJTYPE_NTYPES:
*/
  return SAF_SUCCESS;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    
 * Chapter:    	
 * Purpose:    	
 *
 * Description: 
 *
 * Parallel:    
 *
 * Programmer:	
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
  /*
   *            from SAFhandles.h

   new handles: use saf_getAttribute and saf_putAttribute
   SAF_Algebraic   SAF_Basis       SAF_Db
   SAF_DbProps     SAF_Eval        SAF_FileProps
   SAF_LibProps    SAF_Quantity    SAF_RelRep
   SAF_Role        SAF_Unit

   all other "old" handles use saf_get_attribute and saf_put_attribute

   typedef enum {
   _SAF_OBJTYPE_CAT,
   _SAF_OBJTYPE_FIELD,
   _SAF_OBJTYPE_FILE,
   _SAF_OBJTYPE_FTMPL,
   _SAF_OBJTYPE_REL,
   _SAF_OBJTYPE_SUITE,
   _SAF_OBJTYPE_SET,
   _SAF_OBJTYPE_STMPL,
   _SAF_OBJTYPE_BASE,
   _SAF_OBJTYPE_NTYPES             Must be last
   } SAF_ObjectType;


   ((ss_pers_t *)&saf_object_here)->theType == one of the above

   why not

   saf_object_here.theType ???

   *              *NOTICE* At this time (saf-1.2.0) not all of SAF has been modified to use this improved object handle
   *              interface. Only those object classes that can appear in an object registry have been upgraded so far.
   *              Therefore, operations on the "new" objects may be slightly different than operations on the "old" objects.
   *              For instance, getting and setting attributes of an object is done with the saf_getAttribute() and
   *              saf_putAttribute() functions for "new" objects and saf_get_attribute() and saf_put_attribute() functions for
   *              "old" objects.  If the programmer accidently uses an "old" method on a "new" class or vice versa the C
   *              compiler will complain with an error.
   */


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare New Style SAF Attributes
 *
 * Description: Compares new SAF attributes, i.e. accessed by saf_getAttribute and saf_putAttribute.
 *              See SAFhandles.h.
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
void compare_individual_attributes_new_handles(SAF_Db *left, SAF_Db *right,
					       const char *a_leftName, const char *a_rightName)
{ 
  void *left_void=NULL, *right_void=NULL;	
  int l_leftSubCount=0, l_rightSubCount=0;
  hid_t l_leftSubType=0, l_rightSubType=0;

  if( a_leftName && !strcmp(a_leftName,".SAF_DbProps") && a_rightName && !strcmp(a_rightName,".SAF_DbProps") )
    {
      /*dont compare .SAF_DbProps because it is compared elsewhere*/
      return;
    }

  print_string(PRINT_LEVEL_1,"attribute name", !a_leftName ? "":a_leftName, !a_rightName ? "":a_rightName);	

  if(a_leftName){	
    /* saf_getAttribute(left,g_leftdb,SAF_ALL,a_leftName,&l_leftSubType,&l_leftSubCount,(void **)&left_void); */
    saf_get_attribute(SAF_ALL, (ss_pers_t*)left, a_leftName,&l_leftSubType,&l_leftSubCount,(void **)&left_void);
    if(g_globalfo.store.anything){	
      /*XXX jake need to change first entry to an appropriate SAF_Handle*/

      if( strcmp(a_leftName,".SAF_DbProps") )
          saf_put_attribute(SAF_ALL,(ss_pers_t*)g_newdb,a_leftName,l_leftSubType,l_leftSubCount,(void *)left_void);
	  /* saf_putAttribute(g_newdb,g_newdb,SAF_ALL,a_leftName,l_leftSubType,l_leftSubCount,(void *)left_void); */
    }
  }	
  if(a_rightName){
    /* saf_getAttribute(right,g_rightdb,SAF_ALL,a_rightName,&l_rightSubType,&l_rightSubCount,(void **)&right_void); */
    saf_get_attribute(SAF_ALL, (ss_pers_t*)right,a_rightName,&l_rightSubType,&l_rightSubCount,(void **)&right_void);
    if(g_globalfo.store.anything){
      /*XXX jake need to change first entry to an appropriate SAF_Handle*/

      if( strcmp(a_rightName,".SAF_DbProps") )
        saf_put_attribute(SAF_ALL,(ss_pers_t*)g_newdb,a_rightName,l_rightSubType,l_rightSubCount,(void *)right_void);
	/* saf_putAttribute(g_newdb,g_newdb,SAF_ALL,a_rightName,l_rightSubType,l_rightSubCount,(void *)right_void); */
    } 
  }

  if (l_leftSubCount || l_rightSubCount)
    {	
     print_string(PRINT_LEVEL_1,"attribute type", StringDataType(l_leftSubType), StringDataType(l_rightSubType) );
     print_int(PRINT_LEVEL_1,"attribute count", l_leftSubCount, l_rightSubCount,0,0);
     if(H5Tequal(l_leftSubType,H5T_C_S1) && H5Tequal(l_rightSubType,H5T_C_S1)){	
       /*limit the check to one string. There is a saf error with getAttribute, see test/getAtt.c*/
       if( l_leftSubCount<=1 && l_rightSubCount<=1 )
         print_string_array(PRINT_LEVEL_1,"att val",(char **)&left_void,(char **)&right_void,(size_t)l_leftSubCount,(size_t)l_rightSubCount);
     }	
     else if(H5Tequal(l_leftSubType,SAF_INT) && H5Tequal(l_rightSubType,SAF_INT)){
       print_int_array(PRINT_LEVEL_1,PRINT_LEVEL_1,"att val", (int *)left_void, (int *)right_void, 
		       (size_t)l_leftSubCount, (size_t)l_rightSubCount);
     }	
     else if(H5Tequal(l_leftSubType,SAF_FLOAT) && H5Tequal(l_rightSubType,SAF_FLOAT)){
       print_float_array(PRINT_LEVEL_1,PRINT_LEVEL_1,"att val", (float *)left_void, (float *)right_void, 
		         (size_t)l_leftSubCount, (size_t)l_rightSubCount);
     }	
     else{	
       int *left_pt=left_void, *right_pt=right_void, j=0;
       PRINT_SOME_STRINGS(PRINT_LEVEL_1,l_leftSubCount, l_rightSubCount,j, "att val",
		          (a_leftName!=NULL ? StringDataTypeValue(l_leftSubType,(ss_pers_t*)left_pt+j) : ""),
		          (a_rightName!=NULL ? StringDataTypeValue(l_rightSubType,(ss_pers_t*)right_pt+j) : "") );
     }	
  }
  free(left_void);	
  free(right_void);	
}


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: Note that this is set to PRINT_LEVEL_1 because it is currently only
 * called for the global attributes.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 
 *---------------------------------------------------------------------------------------------------------------*/
int compare_attributes_new_handles(SAF_Db *left, SAF_Db *right)
{
  int  i, minnum_atts;
  int left_count=0,right_count=0;
  char **left_value=0, **right_value=0;

  saf_get_attribute(SAF_ALL,(ss_pers_t *)left,SAF_ATT_NAMES, NULL, &left_count, (void **)&left_value );
  saf_get_attribute(SAF_ALL,(ss_pers_t *)right,SAF_ATT_NAMES, NULL, &right_count, (void **)&right_value );

  if(left_count==0 && right_count==0) return 0;

  print_int(PRINT_LEVEL_1,"number of attribs",left_count,right_count,0,0);
  g_indent++;
	
  /*dont print this: enough info will be printed in compare_individual_attributes_new_handles
    print_string_array(PRINT_LEVEL_1,"attribute name", left_value, right_value, (size_t)left_count, (size_t)right_count);*/

  /*should not compare these lists in order, should find compatible elements for comparison*/
  minnum_atts=MIN(left_count, right_count);

  for(i=0; i<minnum_atts; i++){
    compare_individual_attributes_new_handles(left,right,left_value[i], right_value[i]);
  }
  for(i=minnum_atts; i<left_count; i++){
    compare_individual_attributes_new_handles(left,right,left_value[i], NULL);
  }
  for(i=minnum_atts; i<right_count; i++){
    compare_individual_attributes_new_handles(left,right,NULL, right_value[i]);
  }

  FREE_ARRAY(left_value, left_count);
  FREE_ARRAY(right_value, right_count);
  END_OF_COMPARE_FUNCTION

    return 0;
}/*compare_attributes_new_handles*/



/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing SAF Objects
 * Purpose:    	Compare Old Style SAF Attributes
 *
 * Description: Compares old SAF attributes, i.e. accessed by saf_get_attribute and saf_put_attribute.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void compare_individual_attributes_old_handles(ss_pers_t *left, ss_pers_t *right,
				   const char *a_leftName, const char *a_rightName)
{ 
  void *left_void=NULL, *right_void=NULL;	
  int l_leftSubCount=0, l_rightSubCount=0;
  hid_t l_leftSubType=H5I_INVALID_HID, l_rightSubType=H5I_INVALID_HID;

  print_string(PRINT_LEVEL_3,"attribute name", !a_leftName ? "":a_leftName, !a_rightName ? "":a_rightName);	

  if(a_leftName){
    saf_get_attribute(SAF_ALL,left,a_leftName,&l_leftSubType,&l_leftSubCount,(void **)&left_void);
    if(g_globalfo.store.anything){	
      ss_pers_t *newbasehandle=(ss_pers_t *)malloc(1024);
      saf_translate_basehandle(left, newbasehandle, g_newdb); 
      saf_put_attribute(SAF_ALL, newbasehandle, a_leftName, l_leftSubType,l_leftSubCount,(void *)left_void);
    }
  }	
  if(a_rightName){
    saf_get_attribute(SAF_ALL,right,a_rightName,&l_rightSubType,&l_rightSubCount,(void **)&right_void);
    if(g_globalfo.store.anything){	
      ss_pers_t *newbasehandle=(ss_pers_t *)malloc(1024);
      saf_translate_basehandle(right, newbasehandle, g_newdb); 
      saf_put_attribute(SAF_ALL, newbasehandle, a_rightName, l_rightSubType,l_rightSubCount,(void *)right_void);
    }
  }
  if (l_leftSubCount || l_rightSubCount) {	
     print_string(PRINT_LEVEL_3,"attribute type", !a_leftName ? "":StringDataType(l_leftSubType), 
	       !a_rightName ? "":StringDataType(l_rightSubType));

     print_int(PRINT_LEVEL_3,"attribute count", l_leftSubCount, l_rightSubCount,(int)!a_leftName,(int)!a_rightName);

     if (H5T_STRING==H5Tget_class(l_leftSubType) && H5T_STRING==H5Tget_class(l_rightSubType)) {
       /*limit the check to one string. There is a saf error with getAttribute, see test/getAtt.c*/
       if( l_leftSubCount<=1 && l_rightSubCount<=1 )
         print_string_array(PRINT_LEVEL_3,"att val",(char **)&left_void,(char **)&right_void,(size_t)l_leftSubCount,(size_t)l_rightSubCount);
     }	
     else if(H5Tequal(l_leftSubType,SAF_INT) && H5Tequal(l_rightSubType,SAF_INT)){
       print_int_array(PRINT_LEVEL_3,PRINT_LEVEL_3,"att val", (int *)left_void, (int *)right_void, 
		       (size_t)l_leftSubCount, (size_t)l_rightSubCount);
     }	
     else if(H5Tequal(l_leftSubType,SAF_FLOAT) && H5Tequal(l_rightSubType,SAF_FLOAT)){
       print_float_array(PRINT_LEVEL_3,PRINT_LEVEL_3,"att val", (float *)left_void, (float *)right_void, 
		         (size_t)l_leftSubCount, (size_t)l_rightSubCount);
     }	
     else{	
       int *left_pt=left_void, *right_pt=right_void, j=0;
       PRINT_SOME_STRINGS(PRINT_LEVEL_3,l_leftSubCount, l_rightSubCount, j, "att val",
		          (a_leftName!=NULL ? StringDataTypeValue(l_leftSubType,(ss_pers_t*)(left_pt+j)) : ""),
		          (a_rightName!=NULL ? StringDataTypeValue(l_rightSubType,(ss_pers_t*)(right_pt+j)) : "") );
     }	
   }
  free(left_void);	
  free(right_void);	
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: 
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int compare_attributes_old_handles(ss_pers_t *left, ss_pers_t *right, int a_leftMissing, int a_rightMissing)
{
  int  i, minnum_atts;
  int left_count=0,right_count=0;
  char **left_value=0, **right_value=0;

  if(!a_leftMissing) saf_get_attribute(SAF_ALL, left, SAF_ATT_NAMES, NULL, &left_count, (void **)&left_value);
  if(!a_rightMissing) saf_get_attribute(SAF_ALL, right, SAF_ATT_NAMES, NULL, &right_count, (void **)&right_value);
   
  if(left_count==0 && right_count==0) return 0;

  print_int(PRINT_LEVEL_3,"number of attribs",left_count,right_count,a_leftMissing,a_rightMissing);
  g_indent++;
	

  /*should not compare these lists in order, should find compatible elements for comparison*/
  minnum_atts=MIN(left_count, right_count);

  for(i=0; i<minnum_atts; i++){
    compare_individual_attributes_old_handles(left,right,left_value[i], right_value[i]);
  }
  for(i=minnum_atts; i<left_count; i++){
    compare_individual_attributes_old_handles(left,right,left_value[i], NULL);
  }
  for(i=minnum_atts; i<right_count; i++){
    compare_individual_attributes_old_handles(left,right,NULL, right_value[i]);
  }

  FREE_ARRAY(left_value, left_count);
  FREE_ARRAY(right_value, right_count);
  END_OF_COMPARE_FUNCTION;

  return 0;
}/*compare_attributes_old_handles*/


                                                                          

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tools
 * Purpose:    	Saf database differencer.
 *
 * Description: We assume that leftset and rightset are comparable sets, we now search through the list
 *		of categories to find similar catagories to call compare_cats and also to use
 *		the cats and sets in a call to saf_find_sets.
 *			compare_sets is the function that calls this function.  We could just call saf_find_sets
 *		with the given sets and with categories with corresponding indexes from the call to saf_describe_set,
 *		but this function allows us to be fancy and find similar categories to use for the call to saf_find_sets.
 *
 *		saf_describe_set returns a list of categories, so we could call 
 * 		saf_find_sets(...SUBS, leftset,left_category[i],...)
 * 		saf_find_sets(...SUBS, rightset,right_category[i],...)
 *
 *		The above approach does not make sure that the categories are similar, but this function does.
 * 		Here we find similar categories, then we call saf_find_sets.
 *
 *
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL October 23, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_cat_lists_and_find_subsets(
	SAF_Cat *left, SAF_Cat *right, size_t leftnum, size_t rightnum,
	SAF_Set *leftset, SAF_Set *rightset
)
{
  size_t l=0,r=0;
  int i,j;                       
  size_t size;                                  
  int leftnum_sets, rightnum_sets;
  SAF_Set *leftsets=NULL, *rightsets=NULL;
	         
  int (*sort_compare)(ss_pers_t *, ss_pers_t *);

  print_break(PRINT_LEVEL_3,"Finding Relations");

  g_indent++;

  print_string(PRINT_LEVEL_3,"set name", set_name(leftset),set_name(rightset));


  sort_compare=compare_cat_names;


  size=sizeof(SAF_Cat);                       
  safdiff_sort((ss_pers_t *)(left), leftnum, size, sort_compare);                                                  
  safdiff_sort((ss_pers_t *)(right), rightnum, size, sort_compare);                                                
                                                                                                             
  /*after the lists are sorted, we can go through the list only once to find matching names (linear time)*/  
  while(l<leftnum && r<rightnum)
    {                                                                     
      while( l<leftnum && r<rightnum && sort_compare((ss_pers_t *)(left+l),(ss_pers_t *)(right+r)) == 0 )
	{        
	  print_string(PRINT_LEVEL_3,"collection cat name", cat_name(left+l),cat_name(right+r));
                         
	  compare_topo_relations(leftset, rightset, left+l, right+r);

	  leftsets=NULL; rightsets=NULL;
	  /*we knew the sets were ok to compare, now we know the cats are ok to compare too ????*/
	  saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,leftset, left+l, &leftnum_sets, &leftsets);
	  saf_find_sets(SAF_ALL,SAF_FSETS_SUBS,rightset, right+r, &rightnum_sets, &rightsets);

	  print_int(PRINT_LEVEL_3,"number of subsets", leftnum_sets,rightnum_sets,0,0);

	  if(leftnum_sets>0 || rightnum_sets>0)
	  {
	      canonicalize_set_lists(leftsets, rightsets, leftnum_sets, rightnum_sets);
	      PRINT_SOME_STRINGS(PRINT_LEVEL_3,leftnum_sets, rightnum_sets, i, "subset name",
				set_name(leftsets+i),set_name(rightsets+i) );

	      compare_set_lists(leftsets, rightsets, (size_t)leftnum_sets, (size_t)rightnum_sets);
	  }

	  if(leftnum_sets>0 && rightnum_sets>0)
	    {
	      int leftnum_rels, rightnum_rels;
	      SAF_Rel *leftrels=NULL, *rightrels=NULL;

	      SAF_Cat leftsub_cat, rightsub_cat;
	      SAF_BoundMode leftsbmode, rightsbmode;
	      SAF_BoundMode leftcbmode, rightcbmode;
	      SAF_RelRep leftsrtype, rightsrtype;
	      /*hid_t leftdata_type, rightdata_type;*/

	      size_t leftabuf_sz=0, rightabuf_sz=0;
	      hid_t leftabuf_type, rightabuf_type;
	      size_t leftbbuf_sz=0, rightbbuf_sz=0;
	      hid_t leftbbuf_type, rightbbuf_type;

	      int *leftabuf=NULL, *rightabuf=NULL;
	      int *leftbbuf=NULL, *rightbbuf=NULL;

	      int minnum_sets=MIN(leftnum_sets, rightnum_sets);
	      int minnum_rels;

				/*canonicalize_lists(leftsets,rightsets, leftnum_sets, rightnum_sets);*/
				/*this makes leftsets[i] correspond to rightsets[i]*/
	      for(i=0; i<minnum_sets; i++)
		{
		  leftrels=NULL; rightrels=NULL;

		  /*we know the superset, leftset, the subset is leftsets[i], the category on the superset
		    is left[l], we don't know the category on the subset*/
		  saf_find_subset_relations(SAF_ALL, g_leftdb, leftset, leftsets+i, left+l, SAF_ANY_CAT, 
					    SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE, &leftnum_rels, &leftrels);
		  saf_find_subset_relations(SAF_ALL, g_rightdb, rightset, rightsets+i, right+r, SAF_ANY_CAT, 
					    SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE, &rightnum_rels, &rightrels);

		  print_int(PRINT_LEVEL_3,"number of subset rels", leftnum_rels, rightnum_rels,0,0);

		  minnum_rels=MIN(leftnum_rels, rightnum_rels);


		  /*compare_subset_relation_lists(leftrels,rightrels,leftnum_rels,rightnum_rels);*/
		  for(j=0; j<minnum_rels; j++)
		    {
		      print_break(PRINT_LEVEL_3,"Subset Relations ");

		      /*note: if subset is not written, cannot request datatype or file*/
		      saf_describe_subset_relation(SAF_ALL,leftrels+j,NULL,NULL,NULL,
						   &leftsub_cat, &leftsbmode, &leftcbmode, &leftsrtype, 
						   NULL);
		      saf_describe_subset_relation(SAF_ALL,rightrels+j,NULL,NULL,NULL,
						   &rightsub_cat, &rightsbmode, &rightcbmode,&rightsrtype,
						   NULL);

		      print_string(PRINT_LEVEL_3,"subset rel cat name", cat_name(&leftsub_cat),cat_name(&rightsub_cat));

		      print_string(PRINT_LEVEL_3,"set boundary",str_SAF_BoundMode(leftsbmode),str_SAF_BoundMode(rightsbmode));
		      print_string(PRINT_LEVEL_3,"cat boundary",str_SAF_BoundMode(leftcbmode),str_SAF_BoundMode(rightcbmode));
		      compare_relreps(&leftsrtype, &rightsrtype, 0,0);
		
		      saf_get_count_and_type_for_subset_relation(SAF_ALL, leftrels+j, NULL, &leftabuf_sz, 
								 &leftabuf_type, &leftbbuf_sz, &leftbbuf_type);
		      saf_get_count_and_type_for_subset_relation(SAF_ALL, rightrels+j, NULL, &rightabuf_sz, 
								 &rightabuf_type, &rightbbuf_sz, &rightbbuf_type);

		      print_int(PRINT_LEVEL_3,"abuf size", (int)leftabuf_sz, (int)rightabuf_sz,0,0);
		      print_string(PRINT_LEVEL_3,"abuf type", (leftabuf_sz <= 0) ? "":StringDataType(leftabuf_type),
                                                              (rightabuf_sz<= 0) ? "":StringDataType(rightabuf_type));
		      print_int(PRINT_LEVEL_3,"bbuf size", (int)leftbbuf_sz, (int)rightbbuf_sz,0,0);
		      print_string(PRINT_LEVEL_3,"bbuf type",(leftbbuf_sz <= 0) ? "": StringDataType(leftbbuf_type),
                                                                 (rightbbuf_sz<= 0) ? "": StringDataType(rightbbuf_type));
		
		      if( leftabuf_sz || leftbbuf_sz )
			{
			  int l_skipped_read=1;
			  leftabuf=NULL; leftbbuf=NULL; rightabuf=NULL; rightbbuf=NULL;
			
/* 
			  if( (leftabuf_sz&&leftbbuf_sz) || 
			      (leftabuf_type!=DSL_OFFSET && leftbbuf_type!=DSL_OFFSET) )
*/
			  if (leftabuf_sz&&leftbbuf_sz)
			    {
			      l_skipped_read=0;
			      saf_read_subset_relation(SAF_ALL, leftrels+j, NULL, (void **)&leftabuf,(void**)&leftbbuf);
			    }
/*
			  if( (rightabuf_sz&&rightbbuf_sz) || 
			      (rightabuf_type!=DSL_OFFSET && rightbbuf_type!=DSL_OFFSET) )
*/
			  if (rightabuf_sz&&rightbbuf_sz)  
			    {
			      l_skipped_read=0;
			      saf_read_subset_relation(SAF_ALL, rightrels+j, NULL, (void **)&rightabuf, (void**)&rightbbuf);
			    }


			  if( l_skipped_read )
			    {
			      /*jake note XXX: this seems to be sufficient to keep saf from dying in 
				saf_read_subset_relation, but can it be relaxed? should it fail? should
				saf be fixed?*/
			    }



			  /*store the subset relations*/
			  if(g_globalfo.store.anything)
			    {
			      SAF_Rel newsubset_rel;
			      SAF_Db *newfile;
			      SAF_Set newsupset, newsubset;
			      SAF_Cat newsupcat, newsubcat;

			      saf_copy_set(SAF_ALL, leftset, &newsupset, g_newdb);
			      saf_copy_set(SAF_ALL, leftsets+i, &newsubset, g_newdb);
			      saf_copy_category(left+l, &newsupcat, g_newdb);
			      saf_copy_category(&leftsub_cat, &newsubcat, g_newdb);
			      saf_copy_collection(SAF_ALL, leftsets+i, &leftsub_cat, &newsubset, &newsubcat);
			      saf_copy_collection(SAF_ALL, leftset, left+l, &newsupset, &newsupcat);


			      saf_declare_subset_relation(SAF_ALL, g_newdb, &newsupset, &newsubset, &newsupcat, &newsubcat, leftsbmode, 
							  leftcbmode, &leftsrtype, leftabuf_type, NULL, leftbbuf_type, NULL, &newsubset_rel);

			      /*someday we might want to have the file structure of the output database 
				be the same as the input database. Right now, all the output is being 
				written to one file*/
			      newfile=g_newdb;

			      if( leftabuf && leftbbuf )/*should this be || ?*/
				{
				  saf_write_subset_relation(SAF_ALL, &newsubset_rel, leftabuf_type, (void *)leftabuf, 
							    leftbbuf_type, (void *)leftbbuf,newfile);
				}
			    }

#ifdef USE_BANNER
			  if(leftabuf_sz > rightabuf_sz){
			    sprintf(g_banner,"%s is a subset of %s, on category %s\n",
				    set_name(leftsets+i), set_name(leftset),cat_name(leftsub_cat));
			  }else{
			    sprintf(g_banner,"%s is a subset of %s, on category %s\n",
				    set_name(rightsets+i), set_name(rightset),cat_name(rightsub_cat));
			  }
#endif

			  print_int_array(PRINT_LEVEL_4,PRINT_LEVEL_5,"bbuf", leftbbuf, rightbbuf, 
					  leftbbuf_sz, rightbbuf_sz);

			  if(leftabuf && rightabuf && (leftabuf_sz || rightabuf_sz))
			    {
			      if(H5Tequal(leftabuf_type,SAF_INT)){
				print_int_array(PRINT_LEVEL_4,PRINT_LEVEL_5,"abuf", leftabuf, rightabuf, 
						leftabuf_sz, rightabuf_sz);
			      }
/* else if(leftabuf_type==SAF_LONG || leftabuf_type==DSL_OFFSET){ */
			      else if(H5Tequal(leftabuf_type,SAF_LONG)){
				print_long_array(PRINT_LEVEL_4,PRINT_LEVEL_5,"abuf", (long *)leftabuf, 
						 (long *)rightabuf, leftabuf_sz, rightabuf_sz);
			      }
			    }
		
			  if(leftabuf) free(leftabuf);  
			  if(leftbbuf) free(leftbbuf);
			  if(rightabuf) free(rightabuf); 
			  if(rightbbuf) free(rightbbuf);
			}

		    }/*for j*/


		  if(leftrels) free(leftrels);
		  if(rightrels) free(rightrels);
		}/*for i*/



	    }/*if(leftnum_sets>0 && rightnum_sets>0)*/

	  if(leftsets) free(leftsets);
	  if(rightsets) free(rightsets);

	  l++; r++;                                                                            
	}                                                                                            
      while( l<leftnum && r<rightnum && sort_compare(((ss_pers_t *)(left+l)),(ss_pers_t *)(right+r)) < 0 ){                  
	l++;                                                                                 
      }                                                                                            
      while( l<leftnum && r<rightnum && sort_compare((ss_pers_t *)(left+l),(ss_pers_t *)(right+r)) > 0 ){                  
	r++;                                                                                 
      }                                                                                            
    }        
  END_OF_COMPARE_FUNCTION
    return 0;                                                                                            
}
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/

int canonicalize_set_lists(SAF_Set *left, SAF_Set *right, int leftnum, int rightnum){
	CANONICALIZE_XXX_LISTS(left, right, (size_t)leftnum, (size_t)rightnum, sizeof(SAF_Set),compare_set_names, SAF_Set);
}
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int canonicalize_cat_lists(SAF_Cat *left, SAF_Cat *right, size_t leftnum, size_t rightnum){
	CANONICALIZE_XXX_LISTS(left, right, leftnum, rightnum, sizeof(SAF_Cat),compare_cat_names, SAF_Cat);
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int canonicalize_field_lists(SAF_Field *left, SAF_Field *right, size_t leftnum, size_t rightnum)
{
  CANONICALIZE_XXX_LISTS(left, right, leftnum, rightnum,sizeof(SAF_Field), compare_field_names, SAF_Field);
}
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/

int canonicalize_field_tmpl_lists(SAF_FieldTmpl *left, SAF_FieldTmpl *right, size_t leftnum, size_t rightnum){
	CANONICALIZE_XXX_LISTS(left, right, leftnum, rightnum,sizeof(SAF_FieldTmpl),compare_field_tmpl_names, SAF_FieldTmpl);
}
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int canonicalize_suite_lists(SAF_Suite *left, SAF_Suite *right, size_t leftnum, size_t rightnum){
	CANONICALIZE_XXX_LISTS(left, right, leftnum, rightnum,sizeof(SAF_Suite), compare_suite_names, SAF_Suite);
}
/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int canonicalize_state_tmpl_lists(SAF_StateTmpl *left, SAF_StateTmpl *right, size_t leftnum, size_t rightnum){
	CANONICALIZE_XXX_LISTS(left, right, leftnum, rightnum,sizeof(SAF_StateTmpl),compare_state_tmpl_names, SAF_StateTmpl);
}







/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing Lists
 * Purpose:    	Compare two lists of categories.
 *
 * Description: Compare two lists of categories.  A type specific wrapper for the macro COMPARE_XXX_LISTS
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_cat_lists(SAF_Cat *left, SAF_Cat *right, size_t leftnum, size_t rightnum)
{
  COMPARE_XXX_LISTS(left, right, leftnum, rightnum, sizeof(SAF_Cat), compare_cat_names, compare_cats);
}


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int compare_coll_lists(SAF_Set *a_leftset, SAF_Set *a_rightset, size_t a_leftnum, size_t a_rightnum,
		       SAF_Cat *a_leftcats, SAF_Cat *a_rightcats )
{
  size_t l=0,r=0,i,minnum; 

  if(g_strictorder)
    {	
      minnum=MIN(a_leftnum,a_rightnum);		
      for(i=0; i<minnum; i++){	
	compare_collections(a_leftset,a_leftcats+i,a_rightset,a_rightcats+i);		
      }			
      if(g_globalfo.exclusive){	
	for(i=a_rightnum; i<a_leftnum; i++){	
	  compare_collections(a_leftset,a_leftcats+i,a_rightset,NULL);
	}		
	for(i=a_leftnum; i<a_rightnum; i++){	
	  compare_collections(a_leftset,NULL,a_rightset,a_rightcats+i);
	}	
      }	
    }
  else
    {		
      safdiff_sort((ss_pers_t *)(a_leftcats), a_leftnum, sizeof(SAF_Cat), compare_cat_names);   
      safdiff_sort((ss_pers_t *)(a_rightcats), a_rightnum, sizeof(SAF_Cat), compare_cat_names);  
	 	                                                       
      /*after the lists are sorted, we can go through the list only once to find matching names (linear time)*/
      while(l<a_leftnum && r<a_rightnum)
	{   
	  while( l<a_leftnum && r<a_rightnum && compare_cat_names((ss_pers_t *)(a_leftcats+l),(ss_pers_t *)(a_rightcats+r)) == 0 ){  
	    compare_collections(a_leftset,a_leftcats+l,a_rightset,a_rightcats+r);                                      
	    l++; r++;                                                           
	  }                                                                           
	  while( l<a_leftnum && r<a_rightnum && compare_cat_names((ss_pers_t *)(a_leftcats+l),(ss_pers_t *)(a_rightcats+r)) < 0 ){   
	    if(g_globalfo.exclusive){			
	      compare_collections(a_leftset,a_leftcats+l,a_rightset,NULL);
	    }									
	    l++;                                                                    
	  }                                                                               
	  while( l<a_leftnum && r<a_rightnum && compare_cat_names((ss_pers_t *)(a_leftcats+l),(ss_pers_t *)(a_rightcats+r)) > 0 ){       
	    if(g_globalfo.exclusive){	
	      compare_collections(a_leftset,NULL,a_rightset,a_rightcats+r);					
	    }									
	    r++;                                                                    
	  }                                                                               
	}                                                                                       
      if(g_globalfo.exclusive)
	{								
	  while(l<a_leftnum){			
	    compare_collections(a_leftset,a_leftcats+l,a_rightset,NULL);		
	    l++;							
	  }								
	  while(r<a_rightnum){	
	    compare_collections(a_leftset,NULL,a_rightset,a_rightcats+r);		
	    r++;							
	  }								
	}									
    }										

  return(0);
}




/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing Lists
 * Purpose:    	Compare two lists of sets.
 *
 * Description: Compare two lists of sets.  A type specific wrapper for the macro COMPARE_XXX_LISTS
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *------------------------------------------------------------------------------------------------------------------------------*/
int compare_set_lists(SAF_Set *left, SAF_Set *right, size_t leftnum, size_t rightnum)
{
	COMPARE_XXX_LISTS(left, right, leftnum, rightnum, sizeof(SAF_Set), compare_set_names, compare_sets);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing Lists
 * Purpose:    	Compare two lists of fields
 *
 * Description: Compare two lists of fields.  A type specific wrapper for the macro COMPARE_XXX_LISTS
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_field_lists(SAF_Field *left, SAF_Field *right, size_t leftnum, size_t rightnum)
{
	COMPARE_XXX_LISTS(left, right, leftnum, rightnum, sizeof(SAF_Field), compare_field_names, compare_field_data);
}




/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing Lists
 * Purpose:    	Compare two lists of field templates.
 *
 * Description: Compare two lists of field templates.  A type specific wrapper for the macro COMPARE_XXX_LISTS
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_field_tmpl_lists(SAF_FieldTmpl *left, SAF_FieldTmpl *right, size_t leftnum, size_t rightnum)
{

#ifdef USE_EXTRA_SORT
  /*jake test 021803: sort by more than just the template name*/
  COMPARE_XXX_LISTS(left, right, leftnum, rightnum, sizeof(SAF_FieldTmpl), 
		    sort_compare_field_tmpls, compare_field_templates);
#else
  COMPARE_XXX_LISTS(left, right, leftnum, rightnum, sizeof(SAF_FieldTmpl), 
		    compare_field_tmpl_names, compare_field_templates);
#endif
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Comparing Lists
 * Purpose:    	Compare two lists of suites.
 *
 * Description: Compare two lists of suites.  A type specific wrapper for the macro COMPARE_XXX_LISTS
 *
 * Parallel:    Same.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_suite_lists(SAF_Suite *left, SAF_Suite *right, size_t leftnum, size_t rightnum)
{
	COMPARE_XXX_LISTS(left, right, leftnum, rightnum, sizeof(SAF_Suite), compare_suite_names, compare_suites);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Formatted Printing
 * Purpose:    	Override printf in parallel
 *
 * Description: Overrides printf in parallel so that only the 0th processor prints.
 *
 * Programmer:	Matthew O'Brien, LLNL November 8, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
#ifdef HAVE_PARALLEL
int printf(const char *format, ...)
{
  va_list args;
  if(g_rank == 0){
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
  }
  return 0;
}
#endif


/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: Find all non-suite sets that are top sets, or that would
 * be top sets if all suites were removed. XXX need to speed up the search
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
void find_non_suite_top_sets(SAF_ParMode a_pmode, SAF_Db *a_db, int *a_numSets, SAF_Set **a_sets )
{
  int i,j,l_numTopsets=0,l_numSubsets=0;
  SAF_Set *l_topsets=0,*l_subsets=0;
  int *l_keepTopsets=0,*l_keepSubsets=0;
  int l_numTopsetsToKeep,l_numSubsetsToKeep=0;
  a_numSets[0]=0;

  saf_find_matching_sets(a_pmode, a_db, SAF_ANY_NAME, SAF_ANY_SILROLE, SAF_ANY_TOPODIM,
			 SAF_EXTENDIBLE_TORF, SAF_TOP_TRUE, &l_numTopsets, &l_topsets);

  if(!l_topsets) return;/*there are no suites and no topsets: do nothing*/

  saf_find_matching_sets(a_pmode, a_db, SAF_ANY_NAME, SAF_ANY_SILROLE, SAF_ANY_TOPODIM,
			 SAF_EXTENDIBLE_TORF, SAF_TOP_FALSE, &l_numSubsets, &l_subsets);

  l_numTopsetsToKeep=l_numTopsets;

  l_keepTopsets = (int *)malloc(l_numTopsets*sizeof(int));
  for(i=0;i<l_numTopsets;i++) l_keepTopsets[i]=1;

  if(l_numSubsets) l_keepSubsets = (int *)malloc(l_numSubsets*sizeof(int));
  for(i=0;i<l_numSubsets;i++) l_keepSubsets[i]=0;

  for(i=0;i<l_numTopsets;i++)
    {
      SAF_SilRole l_role;

        if( !(l_topsets+i) )
        {
	      printf("error find_non_suite_top_sets: not a valid set handle\n");
	      exit(-1);
        }

      saf_describe_set(a_pmode,l_topsets+i,NULL,NULL,&l_role,NULL,NULL,NULL,NULL);
      if( l_role==SAF_SUITE )
	{
	  l_keepTopsets[i]=0;/*this is a suite: dont keep it*/
	  l_numTopsetsToKeep--;

	  /*find and keep any first level subsets of this suite, because they would
	    be topsets if this suite were not here*/
	  for(j=0;j<l_numSubsets;j++)
	    {
	      /* XXX todo: should not rely on the name to exclude the suite_param_set,
		 need to identify it with its collection instead*/
	      if(!l_keepSubsets[j] && strcmp("suite_param_set",set_name(l_subsets+j)) )
		{
		  int l_numRels=0;
		  saf_find_subset_relations(a_pmode,a_db,l_topsets+i,l_subsets+j,SAF_ANY_CAT,SAF_ANY_CAT,
					    SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE,&l_numRels,NULL);
		  if(l_numRels) 
		    {
		      l_numSubsetsToKeep++;
		      l_keepSubsets[j]=1; /*keep it*/
		    }
		}
	    }
	}
    }

  /*allocate the argument array and fill it*/
  {
    int l_which=0;
    a_numSets[0]=l_numTopsetsToKeep+l_numSubsetsToKeep;

    if(a_numSets[0])
      {
	a_sets[0] = (SAF_Set *)malloc( a_numSets[0] * sizeof(SAF_Set) );
	for(i=0;i<l_numTopsets;i++)
	  {
	    if(l_keepTopsets[i])
	      {
		a_sets[0][l_which++] = l_topsets[i];
	      }
	  }
	for(i=0;i<l_numSubsets;i++)
	  {
	    if(l_keepSubsets[i])
	      {
		a_sets[0][l_which++] = l_subsets[i];
	      }
	  }
      }
  }    

  /*printf("\nfind_non_suite_top_sets kept %d topsets and %d subsets  out of %d and %d\n\n",
    l_numTopsetsToKeep,l_numSubsetsToKeep, l_numTopsets, l_numSubsets );*/
 
  if(l_keepTopsets) free(l_keepTopsets);
  if(l_keepSubsets) free(l_keepSubsets);
  if(l_topsets) free(l_topsets);
  if(l_subsets) free(l_subsets);
}

/*-----------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	No Chapter Yet
 * Purpose:    	No Purpose Yet
 *
 * Description: Using this to figure out which timestep is intended when the user
 * specifies the time "value" at the command line.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *---------------------------------------------------------------------------------------------------------------*/
int get_closest_index( double a_value, int a_numEntries, double *a_entries )
{
  int i;
  if(!a_numEntries) return(-1);

  for(i=0;i<a_numEntries;i++) 
    {
      if( a_value<=a_entries[i] ) return(i);

      if( i<a_numEntries-1 )
	{
	  double l_ratio = (a_value-a_entries[i]) / (a_entries[i+1] - a_entries[i]);
	  if( l_ratio < .5 ) return(i);
	}
    }

  return(a_numEntries-1);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tools
 * Purpose:    	Saf database differencer.
 *
 * Description: invoked on the command line:
 *		safdiff [options] L R
 * 		tells you the difference between two saf databases, L and R.
 *
 *
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Matthew O'Brien, LLNL Septemper 24, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
  SAF_LibProps *libprops;
  int i;
  SAF_DbProps *leftdbprops, *rightdbprops, *newdbprops, *missingdbprops;

  int num_left_top_sets=0, num_right_top_sets=0;
  SAF_Set *left_top_sets=NULL, *right_top_sets=NULL;

  int num_left_suites=0, num_right_suites=0;
  SAF_Suite *left_suites=NULL, *right_suites=NULL;
  struct stat buf1;


#ifdef HAVE_PARALLEL
  /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass
     the same argc, argv to all processors. However, the MPI spec says nothing about
     what it does or might do to argc or argv. In fact, there is no "const" in the
     function-prototypes for either the pointers or the things they're pointing too.
     I would rather pass NULL here and the spec says this is perfectly acceptable.
     However, that too has caused MPICH to core on certain platforms.  */
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &g_rank);
#endif

  for(i=0; i<MAX_CONTEXT_BUFFER_LENGTH; i++){
    g_context_buffer[i][0]='\0';
  }


  /* process command line args */
  {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    /*must set these default values before we process the command line, so they can be overwritten*/
    for(i=0;i<128;i++) g_globalfo.fieldname[i]='\0';
    g_globalfo.rel_tol=0.0;
    g_globalfo.abs_tol=0.0;
    g_globalfo.floor_tol=0.0;
    g_globalfo.absrel_andor=1;
    g_globalfo.obase=10;


    g_globalfo.verbosity=PRINT_LEVEL_1;
    g_globalfo.exclusive=1;


    g_globalfo.store.diffs=0;
    g_globalfo.store.rel_diff=0;
    g_globalfo.store.map=0;
    g_globalfo.store.left=0;
    g_globalfo.store.right=0;
    g_globalfo.store.ave=0;
    g_globalfo.store.exclusive=0;
    g_globalfo.store.anything=0;

    g_indent=0;
    for(i=0; i<128; i++){ g_level[i]=0; g_last[i]=0;}


    /*we must check the command line argument to see if the user specifies a differenct config file,
     *before* we read the config file */
    strcpy(g_configfile, "./safdiff.config");
    for(i=0; i<argc; i++){
      if(!strcmp(argv[i],"-f")){
	if(++i == argc){ printf("you must specify a config file after -f\n"); exit_safdiff(SAFDIFF_ERROR); 
	} else {strcpy(g_configfile, argv[i]); }
      }
    }

    g_fieldOptionsHash = _saf_htab_new();

    /*must read the configuration file before we process command line, so options can be overwritten
      on the command line*/
    readConfigurationFile();

    /*we call this twice because we need *all* of the global variables to be set correctly
     *before* we deal with the per field parameters that overwrite the global variables*/
    processCommandArguments(argc,argv, &g_globalfo, 0);		
    processCommandArguments(argc,argv, &g_globalfo, 1);
    if(argc<3)
      {
		if( !strlen(g_leftdbname) || !strlen(g_rightdbname) )
		  {
			printf("You must specify at least two saf database file names\n");
			exit_safdiff(SAFDIFF_ERROR);
		  }
      }
    else
      {
		struct stat buf1, buf2;

		/* neither of the last two arguments begin with a '-', so they are assumed to be filenames,
		   if we have not already found the filenames*/
		if( argv[argc-1][0]!='-' && !strlen(g_rightdbname) )
		  {
			if(stat(argv[argc-1],&buf2) >= 0 ){
			  strcpy(g_rightdbname,argv[argc-1]);
			}else{
			  printf("Could not stat file %s\n", argv[argc-1]);	exit_safdiff(SAFDIFF_ERROR);
			}
		  }
		if( argv[argc-2][0]!='-' && !strlen(g_leftdbname) )
		  {
			if(stat(argv[argc-2],&buf1) >= 0 ){
			  strcpy(g_leftdbname,argv[argc-2]);
			}else{
			  printf("Could not stat file %s\n", argv[argc-2]);	exit_safdiff(SAFDIFF_ERROR);
			}
		  }
      }
  }

  if(g_debugging) printf("finished processing command line\n");


#ifdef TEST_FILE_PATH
  chdir(TEST_FILE_PATH);
#endif

#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  libprops=saf_createProps_lib();
  saf_setProps_ErrorMode(libprops,SAF_ERRMODE_THROW);

  saf_init(libprops);

  if(g_debugging) printf("finished calling saf_init\n");

  /*this builds all of the format strings for various calls to printf, with different datatypes, spacing, base of a number
    and whethere the left or right value is missing*/
  build_formatStrings();

  /*testBinaryFloats();*/

  g_seen_sets = _saf_htab_new();

  print_heading(PRINT_LEVEL_1,g_leftdbname, g_rightdbname);


  print_break(PRINT_LEVEL_1,"General Info ");


  if( print_version_info(g_leftdbname,g_rightdbname) ) return(SAFDIFF_ERROR); /*files were not saf files*/


  leftdbprops = saf_createProps_database();
  rightdbprops = saf_createProps_database();
  newdbprops = saf_createProps_database();
  missingdbprops = saf_createProps_database();
  saf_setProps_Clobber(missingdbprops);
  saf_setProps_ReadOnly(rightdbprops);
  saf_setProps_ReadOnly(leftdbprops);

  if(g_debugging) printf("about to call saf_open_database's on names %s and %s\n",g_leftdbname,g_rightdbname);

  g_leftdb = saf_open_database(g_leftdbname,leftdbprops);
  g_rightdb = saf_open_database(g_rightdbname,rightdbprops);

  if(g_debugging) printf("finished saf_open_database's on names %s and %s\n",g_leftdbname,g_rightdbname);

  /*create a new file name for the new database that contains the differences*/
  if(g_globalfo.store.anything)
    {
      if(!g_overwritenewdb){
	while(stat(g_newdbname,&buf1) >= 0){
	  char *p;
	  int filenum=1;
	  print_error(g_newdbname);
	  print_error("Output file already exists, incrementing suffix");
	  p=strrchr(g_newdbname,'_');
	  if( p==NULL){
	    sprintf(g_newdbname,"%s_1",g_newdbname);
	  }else{
	    *p='\0';
	    p++;
	    filenum=atoi(p);
	    filenum++;
	    sprintf(g_newdbname, "%s_%i", g_newdbname, filenum);
	  }
	}
      }

      saf_setProps_Clobber(newdbprops);

      print_error("output will be written to file name:");
      print_error(g_newdbname);

      g_newdb = saf_open_database(g_newdbname,newdbprops);
    }


  strcpy(g_missingdbname, "missing.saf");

  /*Check that there will not be an error when we try to open the missing
    database for writing. If there will be an error, try some alternate
    filenames. If we still cant open a file for writing, exit with error. */
  {
    char l_altName[1024]="";
    int l_altCount=0;
    FILE *fp = fopen(g_missingdbname,"w");
    while(!fp)
      {
	sprintf(l_altName,"%s_%d",g_missingdbname,l_altCount++);

	fp = fopen(l_altName,"w");
	if(l_altCount>9) break;
      }
    if(!fp)
      {
	printf("Error: cant open any of files %s through %s for writing. Check permissions.\n",
	       g_missingdbname, l_altName );
	g_missingdbname[0]='\0';/*clear the name, so the exit routine wont delete it*/
	exit_safdiff(SAFDIFF_ERROR);
      }
    if( strlen(l_altName) ) strcpy(g_missingdbname,l_altName);
    fclose(fp);
  }



  g_missingdb = saf_open_database(g_missingdbname,missingdbprops);

  initializeBitVectorsForNewDatabase();

  initializeMissingObjects();

  /*testBitVector(); exit_safdiff(SAFDIFF_ERROR);*/



  if(g_leftobjname!=NULL && g_rightobjname!=NULL && strlen(g_leftobjname)>0 && strlen(g_rightobjname)>0)
    {
      if(find_objs_and_compare(g_leftobjname, g_rightobjname)!=0)
	{
	  printf("could not find a saf object named %s in %s and/or a saf object named %s in %s\n", 
		 g_leftobjname,g_leftdbname, g_rightobjname, g_rightdbname);
	}
      goto theEnd;
    }

  saf_find_suites(SAF_ALL, g_leftdb, SAF_ANY_NAME, &num_left_suites, &left_suites);
  saf_find_suites(SAF_ALL, g_rightdb, SAF_ANY_NAME, &num_right_suites, &right_suites);
  print_int(PRINT_LEVEL_1,"number of suites", num_left_suites, num_right_suites,0,0);


  find_non_suite_top_sets(SAF_ALL, g_leftdb, &num_left_top_sets, &left_top_sets );
  find_non_suite_top_sets(SAF_ALL, g_rightdb, &num_right_top_sets, &right_top_sets );
  print_int(PRINT_LEVEL_1,"number of \"top\" space sets",num_left_top_sets, num_right_top_sets,0,0);	


  {
    int l_numOtherLeft=0,l_numOtherRight=0;
    saf_find_matching_sets(SAF_ALL, g_leftdb, SAF_ANY_NAME, SAF_SPACE, SAF_ANY_TOPODIM,
			   SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &l_numOtherLeft, NULL);
    saf_find_matching_sets(SAF_ALL, g_rightdb, SAF_ANY_NAME, SAF_SPACE, SAF_ANY_TOPODIM,
			   SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &l_numOtherRight, NULL);
    l_numOtherLeft -= num_left_top_sets;
    l_numOtherRight -= num_right_top_sets;
    print_int(PRINT_LEVEL_1,"number of other space sets",l_numOtherLeft,l_numOtherRight,0,0);
  }


  compare_time_values();


  get_timestep_per_field(g_leftdb, g_leftNumTimesteps, g_leftTimeValues, &g_leftWhichTimestepPerFieldRow, &g_nleft);
  get_timestep_per_field(g_rightdb, g_rightNumTimesteps, g_rightTimeValues, &g_rightWhichTimestepPerFieldRow, &g_nright);



  /*compare global attributes on each db*/
  print_break(PRINT_LEVEL_1,"Global Attributes ");

  compare_attributes_new_handles(g_leftdb,g_rightdb);

  g_save_globalfo = g_globalfo;


  /*compare categories*/
  {
    int l_leftNumCats=0,l_rightNumCats=0;
    SAF_Cat *l_leftCats=0,*l_rightCats=0;
    print_break(PRINT_LEVEL_1,"Categories ");

    saf_find_categories(SAF_ALL, g_leftdb, SAF_UNIVERSE(g_leftdb),SAF_ANY_NAME,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&l_leftNumCats,&l_leftCats);
    saf_find_categories(SAF_ALL, g_rightdb, SAF_UNIVERSE(g_rightdb),SAF_ANY_NAME,SAF_ANY_ROLE,SAF_ANY_TOPODIM,&l_rightNumCats,&l_rightCats);

    print_int(PRINT_LEVEL_2,"num categories",l_leftNumCats,l_rightNumCats,0,0);
    compare_cat_lists(l_leftCats,l_rightCats, (size_t)l_leftNumCats, (size_t)l_rightNumCats);
    if(l_leftCats) free(l_leftCats);
    if(l_rightCats) free(l_rightCats);
  }

  /*compare field templates*/
  {
    int l_numLeft=0,l_numRight=0;
    SAF_FieldTmpl *l_leftTemplates=0,*l_rightTemplates=0;
    print_break(PRINT_LEVEL_1,"Field Templates ");

    saf_find_field_tmpls(SAF_ALL,g_leftdb,SAF_ANY_NAME,SAF_ALGTYPE_ANY,NULL,
			 NULL,&l_numLeft,&l_leftTemplates);
    saf_find_field_tmpls(SAF_ALL,g_rightdb,SAF_ANY_NAME,SAF_ALGTYPE_ANY,NULL,
			 NULL,&l_numRight,&l_rightTemplates);

    print_int(PRINT_LEVEL_2,"number of field templates",l_numLeft,l_numRight,0,0);

    compare_field_tmpl_lists(l_leftTemplates,l_rightTemplates,(unsigned int)l_numLeft,(unsigned int)l_numRight);

    if(l_leftTemplates) free(l_leftTemplates);
    if(l_rightTemplates) free(l_rightTemplates);
  }




  /*SUITE CODE*/
  canonicalize_suite_lists(left_suites, right_suites, (size_t)num_left_suites, (size_t)num_right_suites);
  compare_suite_lists(left_suites, right_suites, (size_t)num_left_suites, (size_t)num_right_suites);
  free(left_suites);
  free(right_suites);

  /*TOP SET CODE*/
  canonicalize_set_lists(left_top_sets, right_top_sets, num_left_top_sets, num_right_top_sets);
  compare_set_lists(left_top_sets, right_top_sets, (size_t)num_left_top_sets, (size_t)num_right_top_sets);
  free(left_top_sets);
  free(right_top_sets);


  /*test_arePermutations();*/


 theEnd:

  if(g_exit_value == SAFDIFF_ERROR)
    {
      print_break_no_buffer(PRINT_LEVEL_0,"Exiting safdiff with an error");
    }else{
      print_break_no_buffer(PRINT_LEVEL_1,"Finished safdiff");
    }


  if(g_globalfo.store.anything){
    saf_close_database(g_newdb);
  }
  saf_close_database(g_missingdb);

  if(strcmp(g_leftdbname, g_rightdbname)){
    saf_close_database(g_rightdb);
  }

  if(g_printLevel>=PRINT_LEVEL_0) printf("%s AND %s ARE %s.\n", g_leftdbname, g_rightdbname, 
					 (g_exit_value ? "DIFFERENT" : "THE SAME") );

  exit_safdiff(g_exit_value);

  return g_exit_value;
}
/*
** END OF FILE: safdiff.c
*/

/* 
  const char * saftype_string( hid_t data_type ) {
                                                                                                                             
                                                                                                                             
                if( H5Tequal(data_type, SAF_CHAR) ) return("SAF_CHAR");
                if( H5Tequal(data_type, SAF_FLOAT)) return("SAF_FLOAT");
                if( data_type ==  SAF_HANDLE) return("SAF_HANDLE");
                if( H5Tequal(data_type, SAF_INT)) return("SAF_INT");
                if( H5Tequal(data_type, SAF_LONG)) return("SAF_LONG");
                if( H5Tequal(data_type, SAF_DOUBLE)) return("SAF_DOUBLE");
                if( H5Tequal(data_type, H5T_C_S1)) return("H5T_C_S1");
                return("Unknown");
  }
*/
