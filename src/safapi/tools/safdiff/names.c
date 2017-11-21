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
#include <names.h>

#define NUM_RETURNED_STRINGS 128                        /*we have this many global strings for returning*/
#define RETURNED_STRINGS_MAX_LEN 129
char g_returned_string[NUM_RETURNED_STRINGS][RETURNED_STRINGS_MAX_LEN];        /*the table of global strings for returning*/
int g_which_string=0;                                     /*the index in the table of global strings*/

/*note replacing strcpy with equivalent strncpy only to avoid __strcpy_small warnings on linux*/
#define strcpy(TOSTR,FROMSTR) strncpy(TOSTR,FROMSTR,strlen(FROMSTR)+1)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * 
 * Description: These functions provide a convinient way to get the string that is the name of a saf object.  It is much
 *		easier to say set_name(myset)  and get "top_set", than to have to do saf_describe_set(...), with tons
 *		of NULL arguments.  Another example is field_name(myfield) to get the name of a field.
 *		or cat_name(mycat) returns the name of the category.
 *
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/





/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     A macro to help get the name of any saf object.
 * 
 * Description: A general macro for doing saf_describe_xxx will all parameters NULL except the name.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *------------------------------------------------------------------------------------------------------------------------------*/
#define SAF_OBJECT_NAME(saf_describe_xxx)                          \
{                                                                  \
        char *name=NULL;                                           \
        /*P(g_which_string_sem)*/                                    \
        g_which_string=(g_which_string+1)%NUM_RETURNED_STRINGS;        \
        /*V(g_which_string_sem)*/                                    \
        saf_describe_xxx;                                          \
		if(strlen(name)>=RETURNED_STRINGS_MAX_LEN) name[RETURNED_STRINGS_MAX_LEN-1]='\0'; \
        strcpy(g_returned_string[g_which_string],name);             \
        free(name);                                                \
        return g_returned_string[g_which_string];                      \
} 

 
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     No Purpose Yet
 * 
 * Description: No Description Yet
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 *------------------------------------------------------------------------------------------------------------------------------*/
char *saf_next_string()
{
        g_which_string=(g_which_string+1)%NUM_RETURNED_STRINGS;
	return g_returned_string[g_which_string];
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of a category.  Return "SAF_SELF" if the cat is SAF_SELF.
 * 
 * Description: Return a string that is the name of a category.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
char *cat_name(SAF_Cat *cat)
{
        if( !SS_CAT(cat) )
        {
                printf("error cat_name: not a valid category\n");
                exit(-1);
        }

        SAF_OBJECT_NAME( saf_describe_category(SAF_ALL, cat, &name, NULL,NULL));
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of a set.  
 * 
 * Description: Return a string that is the name of a set.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
char *set_name(SAF_Set *set)
{
	if( !SS_SET(set) )
	{
		printf("error set_name: not a valid set\n");
		exit(-1);
	}

        SAF_OBJECT_NAME(saf_describe_set(SAF_ALL,set, &name, NULL,NULL,NULL,NULL,NULL,NULL));
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of a collection, given a set and a category.  
 * 
 * Description: Return a string that is the name of a collection, given a set and a category, concatenate their names.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
char *collection_name(SAF_Set *set, SAF_Cat *cat)
{
        char *setname=NULL, *catname=NULL;

        setname=set_name(set);
        catname=cat_name(cat);
        strcat(setname, " ");
        strcat(setname, catname);

        return setname;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of a field.
 * 
 * Description: Return a string that is the name of a field.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
char *field_name(SAF_Field *field)
{
        SAF_OBJECT_NAME(saf_describe_field(SAF_ALL,field, NULL,&name,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL));
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of a state template.
 * 
 * Description: Return a string that is the name of a state template.
 *-------------------------------------------------------------------------------------------------------------------------------*/
char *state_tmpl_name(SAF_StateTmpl *statetmpl)
{
  SAF_OBJECT_NAME(saf_describe_state_tmpl(SAF_ALL, statetmpl, &name,NULL,NULL));
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of an algebraic
 * 
 * Description: Return a string that is the name of an algebraic.
 *-------------------------------------------------------------------------------------------------------------------------------*/
char *algebraic_name(SAF_Algebraic *algebraic)
{
  SAF_OBJECT_NAME(saf_describe_algebraic(SAF_ALL,algebraic, &name,NULL,NULL));
}
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of a basis
 * 
 * Description: Return a string that is the name of a basis.
 *-------------------------------------------------------------------------------------------------------------------------------*/
char *basis_name(SAF_Basis *basis)
{
  SAF_OBJECT_NAME(saf_describe_basis(SAF_ALL,basis, &name,NULL));
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of a field template.
 * 
 * Description: Return a string that is the name of a field tempate.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
char *field_tmpl_name(SAF_FieldTmpl *fieldtmpl)
{
        SAF_OBJECT_NAME(saf_describe_field_tmpl(SAF_ALL,fieldtmpl,&name,NULL,NULL,NULL,NULL,NULL));
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Getting Object Names
 * Purpose:     Return a string that is the name of a suite.
 * 
 * Description: Return a string that is the name of a suite.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
char *suite_name(SAF_Suite *suite)
{
        SAF_OBJECT_NAME( saf_describe_suite(SAF_ALL,suite,&name,NULL,NULL,NULL) ); 
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sorting SAF Objects
 *
 * Description: We can sort an array of saf objects based on the name of the object.  This is what the first implementation
 *		of safdiff does.  It tries to match up elements in two lists of saf objects by same name.
 *		Later this can be relaxed to try to match objects up by more fundamental properties.
 *		Most saf objects have a function saf_compare_xxx_names, which is a function suitable for use with qsort.
 *
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sorting SAF Objects
 * Purpose:     Compare two set names, this function can be used with qsort.
 * 
 * Description: A function that can be used in a call to qsort, to sort two arrays of sets
 *		returns an integer less than, equal to, or greater than zero
 *		to indicate if the first argument is to be considered less than,
 *		equal to, or greater than the second argument.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_set_names(ss_pers_t *left, ss_pers_t *right)
{               
        return strcmp(set_name((SAF_Set *)left), set_name((SAF_Set *)right));
}               

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sorting SAF Objects
 * Purpose:     Compare two cat names, this function can be used with qsort.
 * 
 * Description: A function that can be used in a call to qsort, to sort two arrays of cats
 *		returns an integer less than, equal to, or greater than zero
 *		to indicate if the first argument is to be considered less than,
 *		equal to, or greater than the second argument.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_cat_names(ss_pers_t *left, ss_pers_t *right)
{
        return strcmp(cat_name((SAF_Cat *)left), cat_name((SAF_Cat *)right));
}
   
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sorting SAF Objects
 * Purpose:     Compare two field names, this function can be used with qsort.
 * 
 * Description: A function that can be used in a call to qsort, to sort two arrays of fields
 *		returns an integer less than, equal to, or greater than zero
 *		to indicate if the first argument is to be considered less than,
 *		equal to, or greater than the second argument.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_field_names(ss_pers_t *left, ss_pers_t *right)
{        
        return strcmp(field_name((SAF_Field*)left), field_name((SAF_Field*)right));
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sorting SAF Objects
 * Purpose:     Compare two field template names, this function can be used with qsort.
 * 
 * Description: A function that can be used in a call to qsort, to sort two arrays of field templates
 *		returns an integer less than, equal to, or greater than zero
 *		to indicate if the first argument is to be considered less than,
 *		equal to, or greater than the second argument.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_field_tmpl_names(ss_pers_t *left, ss_pers_t *right)
{        
        return strcmp(field_tmpl_name((SAF_FieldTmpl*)left), field_tmpl_name((SAF_FieldTmpl*)right));
}
 



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sorting SAF Objects
 * Purpose:     Compare two state template names, this function can be used with qsort.
 * 
 * Description: A function that can be used in a call to qsort, to sort two arrays of state templates
 *		returns an integer less than, equal to, or greater than zero
 *		to indicate if the first argument is to be considered less than,
 *		equal to, or greater than the second argument.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_state_tmpl_names(ss_pers_t *left, ss_pers_t *right)
{        
        return strcmp(state_tmpl_name((SAF_StateTmpl*)left), state_tmpl_name((SAF_StateTmpl*)right));
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sorting SAF Objects
 * Purpose:     Compare two suite names, this function can be used with qsort.
 * 
 * Description: A function that can be used in a call to qsort, to sort two arrays of suites
 *		returns an integer less than, equal to, or greater than zero
 *		to indicate if the first argument is to be considered less than,
 *		equal to, or greater than the second argument.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
int compare_suite_names(ss_pers_t *left, ss_pers_t *right)
{        
        return strcmp(suite_name((SAF_Suite*)left), suite_name((SAF_Suite*)right));
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 *
 * Description: There are many #define and enums in saf to represent special values and types.  These functions
 *		take variables of those special types and return strings that represent the value.
 *
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/





/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_ExtendMode
 * 
 * Description: Return a string that represents the value of a variable of type SAF_ExtendMode.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
const char *str_SAF_ExtendMode(SAF_ExtendMode mode){

        if(mode==SAF_EXTENDIBLE_FALSE) return "SAF_EXTENDIBLE_FALSE";
        if(mode==SAF_EXTENDIBLE_TRUE) return "SAF_EXTENDIBLE_TRUE";
        if(mode==SAF_EXTENDIBLE_TORF) return "SAF_EXTENDIBLE_TORF";

        return "UNKNOWN ExtendMode";
 
} 
 
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_TopMode
 * 
 * Description: Return a string that represents the value of a variable of type SAF_TopMode.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_SAF_TopMode(SAF_TopMode mode)
{

        if(mode==SAF_TOP_FALSE) return "SAF_TOP_FALSE";
        if(mode==SAF_TOP_TRUE) return "SAF_TOP_TRUE";  
        if(mode==SAF_TOP_TORF) return "SAF_TOP_TORF";  

        return "UNKNOWN TopMode";

}
 
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_SilRole
 * 
 * Description: Return a string that represents the value of a variable of type SAF_SilRole.
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 * 
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_SAF_SilRole(SAF_SilRole role)
{
        if(role==SAF_TIME) return "SAF_TIME";
        if(role==SAF_SPACE) return "SAF_SPACE";
        if(role==SAF_PARAM) return "SAF_PARAM";
        
        return "UNKNOWN ROLE";
}
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type hid_t
 *
 * Description: Return a string that represents the value of a variable of type hid_t.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 * Issues:      This function only handles a few native types and none of the non-native types. HDF5 will soon have
 *              an hid-to-text function that could replace this one. [rpm 2004-10-20]
 *-------------------------------------------------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type hid_t
 * 
 * Description: Return a string that represents the value of a variable of type hid_t.
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 *
 * Issues:      This function only handles a few native types and none of the non-native types. HDF5 will soon have
 *              an hid-to-text function that could replace this one. [rpm 2004-10-20]
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *StringDataType(hid_t type)
{
  if (type < 0)return "SAF_TYPE_UNKNOWN<0";
  if(H5Tequal(type,SAF_INT)) return "SAF_INT";
  if(H5Tequal(type,SAF_LONG)) return "SAF_LONG";
  if(H5Tequal(type,SAF_HANDLE)) return "SAF_HANDLE";
  if(H5Tequal(type,SAF_FLOAT)) return "SAF_FLOAT";  
  if(H5Tequal(type,SAF_DOUBLE)) return "SAF_DOUBLE";
  if(H5Tequal(type,SAF_CHAR)) return "SAF_CHAR";
  if(H5T_STRING==H5Tget_class(type)) return "SAF_STRING";
  if(H5Tequal(type,H5T_NATIVE_SIZE)) return "SAF_SIZE";
/*
  if(type==DSL_OFFSET) return "DSL_OFFSET";

  if(type== DSL_BOOLEAN) return "DSL_BOOLEAN";
  if(type== DSL_REFERENCE) return "DSL_REFERENCE";         
  if(type== DSL_SHORT) return "DSL_SHORT"; 
  if(type== DSL_UNSIGNED_CHAR) return "DSL_UNSIGNED_CHAR";     
  if(type== DSL_UNSIGNED_INT) return "DSL_UNSIGNED_INT";      
  if(type== DSL_UNSIGNED_INT8) return "DSL_UNSIGNED_INT8";     
  if(type== DSL_UNSIGNED_INT16) return "DSL_UNSIGNED_INT16";    
  if(type== DSL_UNSIGNED_INT32) return "DSL_UNSIGNED_INT32";    
  if(type== DSL_UNSIGNED_INT64) return "DSL_UNSIGNED_LONG";    
  if(type== DSL_UNSIGNED_LONG) return "DSL_UNSIGNED_LONG";     
  if(type== DSL_UNSIGNED_LONG_LONG) return "DSL_UNSIGNED_LONG_LONG";
  if(type== DSL_UNSIGNED_SHORT) return "DSL_UNSIGNED_SHORT";    
  if(H5Tequal(type, SAF_INT8)) return "SAF_INT8"; 
  if(H5Tequal(type, SAF_INT16)) return "SAF_INT16";   
  if(H5Tequal(type, SAF_INT32)) return "SAF_INT32";   
  if(H5Tequal(type, SAF_INT64)) return "SAF_INT64";
  if(H5Tequal(type, SAF_LONG_LONG)) return "SAF_LONG_LONG";         
*/

  return "Unknown data type.... ";
/*
  return DSL_nameOf_type(type);
*/

  /*return H5T_C_S1Of_type("",type);*/
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_BoundMode
 * 
 * Description: Return a string that represents the value of a variable of type SAF_BoundMode
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 * 
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_SAF_BoundMode(SAF_BoundMode mode)
{
        if(mode==SAF_BOUNDARY_FALSE) return "SAF_BOUNDARY_FALSE";
        if(mode==SAF_BOUNDARY_TRUE) return "SAF_BOUNDARY_TRUE";  
        if(mode==SAF_BOUNDARY_TORF) return "SAF_BOUNDARY_TORF";  
        
        return "UNKNOWN BNDRY MODE";
}
 
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type hbool_t
 * 
 * Description: Return a string that represents the value of a variable of type hbool_t
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL November 1, 2001
 * 
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_hbool_t(hbool_t t)
{
        if(t==1) return "true";
        if(t==0) return "false";
        
        return "unknown";
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_TriState
 * 
 * Description: Return a string that represents the value of a variable of type SAF_TriState
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL November 15, 2001
 * 
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_SAF_TriState(SAF_TriState t)
{
        if(t==SAF_TRISTATE_TRUE) return "true";
        if(t==SAF_TRISTATE_FALSE) return "false";
        if(t==SAF_TRISTATE_TORF) return "true or false";
        
        return "unknown";
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_Interleave
 * 
 * Description: Return a string that represents the value of a variable of type SAF_Interleave
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL November 19, 2001
 * 
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_SAF_Interleave(SAF_Interleave t)
{

	if(t==SAF_INTERLEAVE_VECTOR) return "SAF_INTERLEAVE_VECTOR";
	if(t==SAF_INTERLEAVE_COMPONENT) return "SAF_INTERLEAVE_COMPONENT";
	if(t==SAF_INTERLEAVE_INDEPENDENT) return "SAF_INTERLEAVE_INDEPENDENT";
	if(t==SAF_INTERLEAVE_NONE) return "SAF_INTERLEAVE_NONE";
	if(t==SAF_BLOCKED) return "SAF_BLOCKED";
	if(t==SAF_INTERLEAVED) return "SAF_INTERLEAVED";
        
        return "unknown";
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_RelRep
 * 
 * Description: Return a string that represents the value of a variable of type SAF_RelRep
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL November 26, 2001
 * 
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_SAF_RelRep(SAF_RelRep *t)
{

        char *name=NULL;

        saf_describe_relrep(SAF_ALL,t, &name,  NULL,NULL);

        
	if(!strcmp("tuples", 		name)) return "SAF_TUPLES";
        if(!strcmp("totality", 		name)) return "SAF_TOTALITY"; 
        if(!strcmp("hslab", 		name)) return "SAF_HSLAB";
        if(!strcmp("unstructured", 	name)) return "SAF_UNSTRUCTURED";
        if(!strcmp("structured", 	name)) return "SAF_STRUCTURED";
        if(!strcmp("arbitrary", 	name)) return "SAF_ARBITRARY";

	return name;

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_Algebraic
 * 
 * Description: Return a string that represents the value of a variable of type SAF_Algebraic
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL December 11, 2001
 * 
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_SAF_Algebraic(SAF_Algebraic *t)
{

        char *name=NULL;

        saf_describe_algebraic(SAF_ALL,t, &name,  NULL,NULL);

        
	if(!strcmp("scalar", 		name)) return "SAF_ALGTYPE_SCALAR";
        if(!strcmp("vector", 		name)) return "SAF_ALGTYPE_VECTOR"; 
        if(!strcmp("component", 	name)) return "SAF_ALGTYPE_COMPONENT";
        if(!strcmp("tensor", 		name)) return "SAF_ALGTYPE_TENSOR";
        if(!strcmp("symmetric tensor", 	name)) return "SAF_ALGTYPE_SYMTENSOR";
        if(!strcmp("tuple", 		name)) return "SAF_ALGTYPE_TUPLE";
        if(!strcmp("field", 		name)) return "SAF_ALGTYPE_FIELD";

	return name;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Printing a Special Type
 * Purpose:     Return a string representing the value of variables of type SAF_ObjectType
 * 
 * Description: Return a string that represents the value of a variable of type SAF_ObjectType
 * 
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 * 
 * Programmer:  Matthew O'Brien, LLNL January 11, 2002
 * 
 *-------------------------------------------------------------------------------------------------------------------------------*/
const char *str_SAF_ObjectType(ss_pers_t *object)
{
       if (SS_SET(object)) 
          return "SAF_Set";
       else if (SS_CAT(object))
          return "SAF_Cat";
       else if (SS_FIELD(object))
          return "SAF_Field";
       else if (SS_FIELDTMPL(object))
          return "SAF_FieldTmpl";
       else if (SS_REL(object))
           return "SAF_Rel";
       else if (SS_BASIS(object))
           return "SAF_BASIS";
       else if (SS_ALGEBRAIC(object))
           return "SAF_ALGEBRAIC";
       else if (SS_COLLECTION(object))
           return "SAF_COLLECTION";
       else if (SS_REL(object))
           return "SAF_REL";
       else if (SS_RELREP(object))
           return "SAF_RELREP";
       else if (SS_ROLE(object))
           return "SAF_ROLE";
       else if (SS_EVALUATION(object))
           return "SAF_EVALUATION";
       else if (SS_QUANTITY(object))
           return "SAF_QUANTITY";
       else if (SS_UNIT(object))
           return "SAF_UNIT";
       else
           return "Unknown type";
/*
        switch(t){
                case   _SAF_OBJTYPE_CAT:	return "SAF_Cat";
                case   _SAF_OBJTYPE_FIELD:	return "SAF_Field";
                case   _SAF_OBJTYPE_FTMPL: 	return "SAF_FieldTmpl";
                case   _SAF_OBJTYPE_SUITE: 	return "SAF_Suite";
                case   _SAF_OBJTYPE_SET:  	return "SAF_Set";

                case   _SAF_OBJTYPE_STMPL:   	return "SAF_StateTmpl";
                case   _SAF_OBJTYPE_FILE:	return "SAF_File";
                case   _SAF_OBJTYPE_REL: 	return "SAF_Rel";
                case   _SAF_OBJTYPE_BASE:  	return "SAF_Base";
                case   _SAF_OBJTYPE_NTYPES:	return "SAF_Ntypes";
                default:			return "Unknown type";
        }
*/


}


