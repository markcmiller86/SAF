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
#ifndef safP_h_included
/*DOCUMENTED*/
#define safP_h_included

#ifdef WIN32
#pragma warning(disable:4003) /*"not enough actual parameters for macro"*/
#endif

/* The public and private include files. */
#include <saf.h>

/* System include files needed by most source files */
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <sys/resource.h>
#endif

#ifdef HAVE_SYS_TIME_H
#   include <sys/time.h>
#endif

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#ifdef HAVE_REGEX_H
#   include <regex.h>
#else
#   ifdef HAVE_LIBGEN_H
#      include <libgen.h>
#   endif
#endif

#include "genreg.h"
#include "libprops.h"
#include "dbprops.h"
#include "db.h"
#include "quant.h"
#include "hash.h"

#ifdef HAVE_PARALLEL
#include <mpi.h>
#endif

/* All SAF global variables are gathered into this struct which is used to define _SAF_GLOBALS. */
typedef struct SAF_Library {
    SAF_LibProps        p;                      /* library properties */
    hbool_t	        DontReportErrors;       /* used only for testing */
    FILE		*ErrorLogFile;
    int                 AssertDisableCost;
    int                 PreCondDisableCost;
    int                 PostCondDisableCost;
    double		CummWriteTime;
    double		CummReadTime;
    double		WallClockTime;
    hbool_t	        Grabbed;
    hbool_t	        AtExiting;		/* used to distinguish whether exit called saf_final or the client did */
    hbool_t	        RaiseSigStopOnError;	/* used to help attache debuggers in error conditions */
    hbool_t	        DoMPIFinalize;	        /* used only for testing */
    int                 Rank;
    int                 Size;
    ss_prop_t           *find_detect_overflow;
    ss_file_t           *local_file;            /* a transient file opened locally on each MPI task */
    ss_scope_t          *local_scope;           /* the top scope associated with local_file */
    ss_pers_t           *key[SS_PERS_NCLASSES]; /* "key" objects for searching. These point into the local_file. */
    struct {
        char            *std_name;              /* name of the built-in object registry */
        ss_file_t       *std_file;              /* file holding all the built-in standard objects */
        ss_scope_t      *std_scope;             /* the top-scope of the std_file */
    } reg;                                      /* object registries */
    struct {
        size_t          next;                   /* index to next slot to use */
        char            **pool;                 /* array of SAF_Library.p.StrPoolSize string pointers. */
    } str;
    
    /* New global variable to cache the macro find calls.  This way the find only happens once, after that, we use these cached
     * values. */
    SAF_Algebraic *algebraic_scalar;
    SAF_Algebraic *algebraic_vector;
    SAF_Algebraic *algebraic_component;
    SAF_Algebraic *algebraic_tensor;
    SAF_Algebraic *algebraic_symmetric_tensor;
    SAF_Algebraic *algebraic_tuple;
    SAF_Algebraic *algebraic_field;

    SAF_Basis *basis_unity;
    SAF_Basis *basis_cartesian;
    SAF_Basis *basis_spherical;
    SAF_Basis *basis_cylindrical;
    SAF_Basis *basis_uppertri;
    SAF_Basis *basis_variable;

    SAF_Eval *evaluation_constant;
    SAF_Eval *evaluation_piecewise_constant;
    SAF_Eval *evaluation_piecewise_linear;
    SAF_Eval *evaluation_uniform;

    SAF_Quantity *quant[SS_MAX_BASEQS];         /* The seven basic quantities */

    SAF_RelRep *relrep_hslab;
    SAF_RelRep *relrep_tuples;
    SAF_RelRep *relrep_totality;
    SAF_RelRep *relrep_structured;
    SAF_RelRep *relrep_unstructured;
    SAF_RelRep *relrep_arbitrary;

    SAF_Role *role_topology;
    SAF_Role *role_processor;
    SAF_Role *role_block;
    SAF_Role *role_domain;
    SAF_Role *role_assembly;
    SAF_Role *role_material;
    SAF_Role *role_space_slice;
    SAF_Role *role_param_slice;

    SAF_IndexSpec indexspec_not_applicable;
} SAF_Library;
extern SAF_Library _SAF_GLOBALS;

#ifndef __UNUSED__
#  ifdef HAVE_ATTRIBUTE
/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Suppress unused argument warnings from GCC
 *
 * Description: This macro is used to denote a deliberately unused functional formal argument, and suppress the associated
 *              compiler warning from GCC.  Example usage:
 *
 *                  //ARGSUSED
 *                  int
 *                  func(int a, int __UNUSED__ b, int c)
 *
 *              For SGI compilers functions with unused arguments should have the word ARGSUSED inside a C comment immediately
 *              preceding the function declaration.
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#     define __UNUSED__ __attribute__((__unused__))
#  else
/*DOCUMENTED*/
#     define __UNUSED__ 
#  endif
#endif


#include "wrapper.h" /*SAF function wrappers*/


/* ----------------------------------

             CONSTANTS

   ---------------------------------- */

#define SAF_INSTALL_DATADIR SS_INSTALL_DATADIR

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Assertion costs
 * Description: Values to indicate costs of logical assertions or pre/post conditions.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
typedef int SAF_AssertionCost;
#define SAF_NO_CHK_COST    0
#define SAF_LOW_CHK_COST   1
#define SAF_MED_CHK_COST   2
#define SAF_HIGH_CHK_COST  3

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Array size
 * Description: Return number of elements in array.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_NELMTS(X)       (sizeof(X)/sizeof(*(X)))

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Exclusive OR operator
 * Description: Returns A XOR B
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_XOR(A,B)    (((A) && !(B)) || (!(A) && (B)))

#ifdef HAVE_PARALLEL
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Miscellaneous Utilities 
 * Purpose:	Get an environment variable 
 *
 * Description:	MPI-1 doesn't guarentee environment is passed to all processors. This macro 
 *		should be used to get environment variables during initialization of lib. It
 *		should only be used during initialization in _saf_init() or _saf_setProps_lib().
 *		This particular macro gets the env. in MPI_COMM_WORLD.
 *
 *		If the quiered environment variable exists, this macro will allocate space for it
 *		that the caller will have to free later
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_GETENV_WORLD(resultStr,envStr)							\
        do                                                    					\
        {  char *bcastStr;									\
	   int rank=0, strLen;									\
	   MPI_Comm_rank(MPI_COMM_WORLD, &rank);						\
	   if (rank == 0)									\
	   {											\
              resultStr = getenv(envStr);							\
	      strLen = resultStr?(int)strlen(resultStr):0;					\
	   }											\
           MPI_Bcast(&strLen, 1, MPI_INT, 0, MPI_COMM_WORLD);					\
	   if (strLen)										\
	   {											\
	      bcastStr = malloc((size_t)strLen+1);					        \
	      if (rank == 0)									\
	         strncpy(bcastStr, resultStr, (size_t)strLen+1);				\
              MPI_Bcast(bcastStr, strLen+1, MPI_CHAR, 0, MPI_COMM_WORLD);			\
	      resultStr = bcastStr;								\
	   }											\
	   else											\
	      resultStr = NULL;									\
	}while(false)
#else
/*DOCUMENTED*/
#define SAF_GETENV_WORLD(resultStr,envStr)							\
	do											\
	{  char *bcastStr;									\
	   int strLen;										\
	   resultStr = getenv(envStr);								\
	   strLen = resultStr?(int)strlen(resultStr):0;						\
	   if (strLen)										\
	   {											\
               bcastStr = malloc((size_t)strLen+1);                                             \
               strncpy(bcastStr, resultStr, (size_t)strLen+1);                                  \
               resultStr = bcastStr;								\
	   }											\
	   else											\
	      resultStr = NULL;									\
	}while(false)
	
#endif

#ifdef HAVE_PARALLEL
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Miscellaneous Utilities 
 * Purpose:	Get an environment variable 
 *
 * Description:	MPI-1 doesn't guarentee environment is passed to all processors. This macro 
 *		should be used to get environment variables during initialization of lib. It
 *		should only be used during initialization in _saf_init() or _saf_setProps_lib().
 *		This particular macro gets the env. in MPI_COMM_WORLD. 
 *
 *		If the quiered environment variable exists, this macro will allocate space for it
 *		that the caller will have to free later
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_GETENV_LIB(resultStr,envStr)                                                                                       \
        do                                                                                                                     \
        {  char *bcastStr;                                                                                                     \
	   int rank=0, strLen;                                                                                                 \
	   MPI_Comm_rank(_SAF_GLOBALS.p.LibComm, &rank);                                                                       \
	   if (rank == 0)                                                                                                      \
	   {                                                                                                                   \
              resultStr = getenv(envStr);                                                                                      \
	      strLen = resultStr?(int)strlen(resultStr):0;                                                                     \
	   }                                                                                                                   \
           MPI_Bcast(&strLen, 1, MPI_INT, 0, _SAF_GLOBALS.p.LibComm);                                                          \
	   if (strLen)                                                                                                         \
	   {                                                                                                                   \
	      bcastStr = malloc((size_t)strLen+1);                                                                             \
	      if (rank == 0)                                                                                                   \
	         strncpy(bcastStr, resultStr, (size_t)strLen+1);                                                               \
              MPI_Bcast(bcastStr, strLen+1, MPI_CHAR, 0, _SAF_GLOBALS.p.LibComm);                                              \
	      resultStr = bcastStr;                                                                                            \
	   }                                                                                                                   \
	   else                                                                                                                \
	      resultStr = NULL;                                                                                                \
	}while(false)
#else
/*DOCUMENTED*/
#define SAF_GETENV_LIB(resultStr,envStr)                                                                                       \
	do                                                                                                                     \
	{  char *bcastStr;                                                                                                     \
	   int strLen;                                                                                                         \
	   resultStr = getenv(envStr);                                                                                         \
	   strLen = resultStr?(int)strlen(resultStr):0;                                                                        \
	   if (strLen)                                                                                                         \
	   {                                                                                                                   \
               bcastStr = malloc((size_t)strLen+1);                                                                            \
               strncpy(bcastStr, resultStr, (size_t)strLen+1);                                                                 \
               resultStr = bcastStr;                                                                                           \
	   }                                                                                                                   \
	   else                                                                                                                \
	      resultStr = NULL;                                                                                                \
	}while(false)
#endif

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Initialize key/mask pair
 *
 * Description: This macro declares and initializes a key and mask pair of objects to be used for searching a table.  The KEY
 *              and MASK arguments are simply names of local variables that will hold the values for which to search (the KEY)
 *              and how to compare those values to the existing objects when searching (the MASK).  The TYPE argument should
 *              be one of the SAF object datatypes such as SAF_Role.
 *
 * Example:     To search for a role with a specific name and ID number you would do the following:
 *                SAF_KEYMASK(SAF_Role,key,mask);                         // Declare and initialize the key and mask roles
 *                SAF_Role *found=NULL;                                   // List of all roles that were found
 *                size_t nfound;                                          // Number of roles that were found
 *                SAF_SEARCH(SAF_Role,key,mask,id,30);                    // Search for roles with ID of 30
 *                SAF_SEARCH_S(SAF_Role,key,mask,name,"foo");             // Search for roles with the name "foo"
 *                found = SS_PERS_FIND(scope,key,&mask,SS_NOSIZE,nfound); // The actual search
 *
 * Parallel:    Independent
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_KEYMASK(type,key,mask)                                                                                             \
    type##Obj mask;                                                                                                            \
    type *key=(type*)_saf_keymask(SAF_MAGIC(type), (ss_persobj_t*)&mask);                                                      \
    int mask##_count=0

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Sets a key/mask value for which to search
 *
 * Description: Given KEY and MASK variables declared by SAF_KEYMASK() search for a MEMBER with a particular VALUE.  If MEMBER
 *              is a variable length string then the SAF_SEARCH_S() macro should be used instead.
 *
 * Parallel:    Independent.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_SEARCH(type,key,mask,member,value) do {                                                                            \
    ((type##Obj*)ss_pers_deref((ss_pers_t*)key))->member=(value);                                                              \
    memset(&(mask.member), SS_VAL_CMP_DFLT, 1);                                                                                \
    mask##_count++;                                                                                                            \
} while (false)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Sets a key/mask value for which to search
 * Description: This is just like the SAF_SEARCH() macro except the VALUE argument is a C NUL-terminated character string.
 * Parallel:    Independent
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_SEARCH_S(type,key,mask,member,string) do {                                                                         \
    ss_string_set(&(((type##Obj*)ss_pers_deref((ss_pers_t*)key))->member), string);                                            \
    memset(&(mask.member), SS_VAL_CMP_DFLT, 1);                                                                                \
    mask##_count++;                                                                                                            \
} while (0)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Sets a key/mask value for which to search
 * Description: This is just like the SAF_SEARCH_S() macro except the STRING argument is a C NUL-terminated character string
 *              that serves as a regular expression.
 * Parallel:    Independent
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_SEARCH_RE(type,key,mask,member,string) do {                                                                        \
    ss_string_set(&(((type##Obj*)ss_pers_deref((ss_pers_t*)key))->member), string);                                            \
    memset(&(mask.member), SS_VAL_CMP_RE, 1);                                                                                  \
    mask##_count++;                                                                                                            \
} while (0)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Sets a key/mask value for which to search
 * Description: If you want to search for an object link with ss_pers_equal() rather than the default ss_pers_eq() then use
 *              this macro instead of SAF_SEARCH().
 * Parallel:    Independent
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_SEARCH_LINK(type,key,mask,member,link) do {                                                                        \
    ((type##Obj*)ss_pers_deref((ss_pers_t*)key))->member = (link);                                                             \
    memset(&(mask.member), SS_VAL_CMP_EQUAL, 1);                                                                               \
    mask##_count++;                                                                                                            \
} while (0)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Obtain magic number for a SAF datatype
 *
 * Description: When given a SAF datatype such as SAF_Role, this macro returns the magic number for the associated SSlib
 *              datatype, ss_role_t.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_MAGIC(type) SAF_MAGIC_##type

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Mark an object as having been modified
 *
 * Description: Before modifying a persistent part of an object the caller should mark that object as dirty with this macro.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_DIRTY(obj,pmode)    ss_pers_modified((ss_pers_t*)obj, SAF_ALL==pmode?SS_ALLSAME:0U)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Datatypes
 * Purpose:    	The state of MPI
 *
 * Description: Values used in determining if MPI_Finalize() has been called 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum {
   SAF_MPI_FINALIZED_FALSE = SAF_TRISTATE_FALSE,	/* MPI_Finalize() has been called */
   SAF_MPI_FINALIZED_TRUE  = SAF_TRISTATE_TRUE,		/* MPI_Finalize() has *not* been called */
   SAF_MPI_FINALIZED_TORF  = SAF_TRISTATE_TORF		/* SAF cannot determine if MPI_Finalize() has been called */
} SAF_MPIFinalizeMode;


/* ----------------------------------

     Private Function Prorotypes

   ---------------------------------- */
/*
 * function prototypes below are typeset in two ways...
 *    a. one for functions with three or fewer args (short form) and
 *    b. one for functions with four or more args (long form)
 */



/*
 *********************************************************
 * utility functions
 *********************************************************
 */
ss_collection_t _saf_get_collection(SAF_Set *set, SAF_Cat *cat);
hbool_t _saf_is_null(SAF_Set *s);
hbool_t _saf_is_universe(SAF_Set *s);
hbool_t _saf_is_self_decomp(SAF_Cat *cat);
SAF_Db *_saf_master_file(SAF_Db *db);
void *_saf_free(void *ptr);
char *_saf_strncpy(char *dst, const char *src, size_t n);
hbool_t _saf_is_permutation(int N, int vec[]);
int _saf_setupReturned_string(char **result, const char *input);
hbool_t _saf_valid_pmode(SAF_ParMode pmode);
hbool_t _saf_valid_memhints(int *n, void **ptr);
hbool_t _saf_valid_indexspec(SAF_IndexSpec ispec, int count);
hbool_t _saf_valid_add_indexspec(SAF_ParMode pmode, SAF_Set *set, SAF_Cat *cat, SAF_IndexSpec add_ispec, int add_count);
hbool_t _saf_is_participating_proc(SAF_ParMode pmode);
hbool_t _saf_is_valid_io_request(SAF_ParMode pmode, SAF_Field *field, int member_count, SAF_RelRep *req_type,
                                 int *member_ids, int nbufs);
void _saf_set_disable_threshold(const char *env_var_name, int *property_value);
void _saf_final(void);
double _saf_wall_clock(hbool_t init);
SAF_Field *_saf_field_handles_1d(int nbufs, SAF_Field **bufs, int count);
hbool_t _saf_is_valid_units(SAF_Unit *unit, SAF_FieldTmpl *ftmpl);
SAF_MPIFinalizeMode _saf_is_mpi_finalized(void);
char *_saf_strdup(const char *s);
ss_pers_t *_saf_keymask(unsigned magic, ss_persobj_t *mask);
hbool_t _saf_is_primitive_type(hid_t type);
void *_saf_convert(hid_t srctype, const void *srcbuf, hid_t dsttype, void *dstbuf);

/*
 *********************************************************
 * functions for handling error conditions
 *********************************************************
 */
extern void _saf_error(int throwval);
extern void _saf_throw(int throwval);
extern void _saf_generic_errmsg(const char *file, const char *func, int line);
extern void _saf_errmsg(const char *format, ...);
extern void _saf_error_print(void);

/*
 *********************************************************
 * handle functions for various objects 
 *********************************************************
 */

/* set */
ss_collection_t *_saf_getCollection_set(SAF_Set *set, SAF_Cat *cat, ss_collection_t *buf);
int _saf_putCollection_set(SAF_ParMode pmode, SAF_Set *set, SAF_Cat *cat, ss_collection_t *coll);

/* relation */
hbool_t _saf_valid_topo_write_buffers(SAF_ParMode pmode, SAF_Rel *rel, hid_t A_type, void *A_buf, hid_t B_type, void *B_buf);
int _saf_write_subset_relation(SAF_ParMode pmode, SAF_Rel *rel, SAF_Rel *oldrel, hid_t A_type, void *A_buf, hid_t B_type,
                               void *B_buf, SAF_Db *file);

/* field */
int _saf_numberOfComponentsOf_field(SAF_ParMode pmode, SAF_Field *field, int *num_comps);
SAF_Field *_saf_find_parent_field(SAF_ParMode pmode, SAF_Field *component_field, SAF_Field *retval);
int _saf_read_comp_field(SAF_ParMode pmode, SAF_Field *field, int member_count, SAF_RelRep *req_type, int *member_ids,
                         void **Pbuf);

#endif
