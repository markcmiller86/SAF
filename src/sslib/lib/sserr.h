/*
 * Copyright(C) 2004-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 2004-2005 Sandia Corporation.
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
 * Authors:
 *     Robb P. Matzke              LLNL
 *     Eric A. Illescas            SNL
 * 
 * Acknowledgements:
 *     Mark C. Miller              LLNL - Design input
 * 
 */
#ifndef SS_HAVE_SSERR_H
#define SS_HAVE_SSERR_H

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 *
 * Description: SSlib functions and the functions that SSlib calls all indicate errors by returning special values. SSlib code
 *              inspects return values and enters an error recovery mode when an error is detected. In order to make the SSlib
 *              code base cleaner and to facilitate changes in error handling policies, we use a number of macros. The goal is
 *              to have a system that is easy to program, easy to optimize, and easy to read (lean, mean, and clean).
 *
 *              Almost every function will begin and end with SS_ENTER() and SS_LEAVE() calls.  Somewhere in between them will
 *              be an SS_CLEANUP label that marks the boundary between normal flow of control and error recovery code.  Inside
 *              the normal flow of control will be calls to SS_ERROR() or SS_ERROR_FMT() when an error is detected, or various
 *              calls to SS_STATUS() macros to get information about an error.
 *
 *              Sometimes, particularly during a parallel run, error recovery is impossible, prohibitively expensive, or
 *              unusually complex. In such cases SSlib may call MPI_Abort(). (A version of SSlib compiled with MPI support but
 *              run with a single MPI task as the library communicator is considered a serial run.)
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/* All SSlib functions return similar values for errors. The error value is based on the return type of the function and
 * we define that with these macros. The lower-case return type is an artifact of how these symbols are generated: they are
 * just the return type of the function with asterisks replaced by `P' */

/* Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Error value by datatype
 * Description: All SSlib functions return similar values for errors. The error value is based on the return type of the
 *              function and is defined by these macros. The lower-case return type is an artifact of how these symbols are
 *              generated: they are just the return type of the function with asterisks replaced by the letter `P'. */
#define SS_RETURNS_charP                char              *_retval = NULL
#define SS_RETURNS_const_charP          const char        *_retval = NULL
#define SS_RETURNS_const_voidP          const void        *_retval = NULL
#define SS_RETURNS_double               double            _retval = -1
#define SS_RETURNS_H5E_error2_tP        const H5E_error2_t *_retval = NULL
#define SS_RETURNS_herr_t               herr_t            _retval = -1
#define SS_RETURNS_hid_t                hid_t             _retval = -1
#define SS_RETURNS_hsize_t              hsize_t           _retval = (hsize_t)(-1)
#define SS_RETURNS_htri_t               htri_t            _retval = -1
#define SS_RETURNS_int                  int               _retval = -1
#define SS_RETURNS_size_t               size_t            _retval = SS_NOSIZE
#define SS_RETURNS_ss_attr_tP           ss_attr_t         *_retval = NULL
#define SS_RETURNS_ss_blob_tP           ss_blob_t         *_retval = NULL
#define SS_RETURNS_ss_file_tP           ss_file_t         *_retval = NULL
#define SS_RETURNS_ss_file_ref_tP       ss_file_ref_t     *_retval = NULL
#define SS_RETURNS_ss_gblob_tP          ss_gblob_t        *_retval = NULL
#define SS_RETURNS_ss_obj_tP            ss_obj_t          *_retval = NULL
#define SS_RETURNS_ss_pers_tP           ss_pers_t         *_retval = NULL
#define SS_RETURNS_ss_persobj_tP        ss_persobj_t      *_retval = NULL
#define SS_RETURNS_ss_prop_tP           ss_prop_t         *_retval = NULL
#define SS_RETURNS_ss_scope_tP          ss_scope_t        *_retval = NULL
#define SS_RETURNS_ss_string_table_tP   ss_string_table_t *_retval = NULL
#define SS_RETURNS_ss_table_tP          ss_table_t        *_retval = NULL
#define SS_RETURNS_ss_table_cksum_tP    ss_table_cksum_t  *_retval = NULL
#define SS_RETURNS_ss_val_info_tP       ss_val_info_t     *_retval = NULL
#define SS_RETURNS_unsigned_charP       unsigned char     *_retval = NULL
#define SS_RETURNS_voidP                void              *_retval = NULL

/* Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Minor error numbers
 * Issues:      These C preprocessor symbols would normally just be defined as the corresponding global variable, however
 *              error class numbers are generated at runtime with HDF5 calls and thus they must be initialized as a side
 *              effect of referencing them. The initialization is done by calling ss_err_init1() and passing the address of
 *              the global minor error class variable.  That function does the initialization and then returns the contents of
 *              that global variable. */
#define SS_MINOR_ASSERT   ss_err_init1(&ss_minor_assert_g,   "assertion failed")                /*assertion failed*/
#define SS_MINOR_CONS     ss_err_init1(&ss_minor_cons_g,     "constructor failed")              /*constructor failed*/
#define SS_MINOR_CORRUPT  ss_err_init1(&ss_minor_corrupt_g,  "possible file corruption")        /*file file corruption*/
#define SS_MINOR_DOMAIN   ss_err_init1(&ss_minor_domain_g,   "value outside valid domain")      /*value outside valid domain*/
#define SS_MINOR_EXISTS   ss_err_init1(&ss_minor_exists_g,   "already exists")                  /*already exists*/
#define SS_MINOR_FAILED   ss_err_init1(&ss_minor_failed_g,   "operation failed")                /*a catch-all*/
#define SS_MINOR_HDF5     ss_err_init1(&ss_minor_hdf5_g,     "HDF5 call failed")                /*HDF5 call failed*/
#define SS_MINOR_INIT     ss_err_init1(&ss_minor_init_g,     "not initialized")                 /*not initialized*/
#define SS_MINOR_MPI      ss_err_init1(&ss_minor_mpi_g,      "MPI call failed")                 /*MPI call failed*/
#define SS_MINOR_NOTFOUND ss_err_init1(&ss_minor_notfound_g, "not found")                       /*not found*/
#define SS_MINOR_NOTIMP   ss_err_init1(&ss_minor_notimp_g,   "not implemented")                 /*not implemented*/
#define SS_MINOR_NOTOPEN  ss_err_init1(&ss_minor_notopen_g,  "not open")                        /*not open*/
#define SS_MINOR_OVERFLOW ss_err_init1(&ss_minor_overflow_g, "arithmetic or buffer overflow")   /*arithmetic or buffer overflow*/
#define SS_MINOR_PERM     ss_err_init1(&ss_minor_perm_g,     "not permitted")                   /*not permitted*/
#define SS_MINOR_RESOURCE ss_err_init1(&ss_minor_resource_g, "insufficient resources")          /*insufficient resources*/
#define SS_MINOR_SKIPPED  ss_err_init1(&ss_minor_skipped_g,  "operation skipped by request")    /*operation skipped by request*/
#define SS_MINOR_TYPE     ss_err_init1(&ss_minor_type_g,     "bad datatype")                    /*bad datatype*/
#define SS_MINOR_USAGE    ss_err_init1(&ss_minor_usage_g,    "incorrect usage or bad arguments")/*incorrect usage*/

/* Error global variables defined at the top of sserr.c */
extern hid_t ss_minor_assert_g;
extern hid_t ss_minor_cons_g;
extern hid_t ss_minor_corrupt_g;
extern hid_t ss_minor_domain_g;
extern hid_t ss_minor_exists_g;
extern hid_t ss_minor_failed_g;
extern hid_t ss_minor_hdf5_g;
extern hid_t ss_minor_init_g;
extern hid_t ss_minor_mpi_g;
extern hid_t ss_minor_notfound_g;
extern hid_t ss_minor_notimp_g;
extern hid_t ss_minor_notopen_g;
extern hid_t ss_minor_overflow_g;
extern hid_t ss_minor_perm_g;
extern hid_t ss_minor_resource_g;
extern hid_t ss_minor_skipped_g;
extern hid_t ss_minor_type_g;
extern hid_t ss_minor_usage_g;

/* Error class for entire library */
extern hid_t ss_eclass_g;

/* The ss_err_info() and ss_err_walk() functions use this data structure to pass information back and forth through HDF5 */
typedef struct ss_err_info_t {
    unsigned            position;               /* non-negative stack position for which information is returned */
    const H5E_error2_t   *info;                  /* information about the error, points directly into error stack */
} ss_err_info_t;

/* This struct controls the fine points of error printing */
typedef struct ss_err_cntl_t {
    int                 fd;                     /* file descriptor on which to report error (-1 turns off reporting) */
    char                *ptr;                   /* dynamically allocated buffer to hold error messages being printed */
    size_t              ptrlen;                 /* current allocated length of the PTR array */
    size_t              nerrors;                /* Number of errors pushed onto the stack; last used error ID number. */
} ss_err_cntl_t;
extern ss_err_cntl_t    ss_err_cntl_g;


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Define an interface
 *
 * Description: SSlib is composed of various interfaces, each responsible for operations on a specific class of objects or
 *              each supplying a set of related functions.  Each interface has a short name (like `scope') which is also the
 *              second component in most of the symbol names for that interface (like ss_scope_open).
 *
 * Programmer:  Robb Matzke
 *              Tuesday, July 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_IF(_if_name_)                                                                                                        \
    static herr_t(*ss_if_init)(void) = ss_##_if_name_##_init;                                                                    \
    static const char *ss_if_name_g = #_if_name_;                                                                               \
    static hbool_t ss_if_inited_g;                                                                                              \
    static hid_t ss_if_major_g


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Marks beginning of function
 *
 * Description: Nearly every SSlib function should call this macro as the first thing it does.  The first argument is the
 *              function name and the second is the return type. If the return type is a pointer then suffix the main type
 *              name with a `P'. Type qualifiers `const' and `static' can be omitted. Types consisting of more than one word
 *              should have the words separated by underscores.  For instance:
 *
 *                      unsigned long foo(void) {
 *                          SS_ENTER(foo, unsigned_long);
 *
 *              The definitions for failure return type are near the top of FILE:sserr.h as the various SS_RETURNS macros.
 *
 *              Interface initialization functions should use SS_ENTER_INIT() instead of SS_ENTER() because that macro does
 *              some additional work that's not required for most other functions.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ENTER(_function_name_, _return_type_) {                                                                              \
    /* Variables for flow control */                                                                                            \
    static const char *_func=#_function_name_;                                                                                  \
    hid_t _estack=-1;                                                                                                           \
    H5E_auto2_t _efunc=NULL;                                                                                               \
    void *_edata=NULL;                                                                                                          \
    hbool_t _doing_cleanup=FALSE;                                                                                               \
    SS_RETURNS_##_return_type_;                                                                                                 \
    SS_CHECK_FUNCTION_NAME(_function_name_);                                                                                    \
                                                                                                                                \
    /* Save HDF5's error reporting status and then turn it off. We only need to do this at the top-most SSlib function. */      \
    if (0==sslib_g.call_depth++) {                                                                                              \
        H5Eget_auto2(H5E_DEFAULT, &_efunc,&_edata);                                                                        \
        H5Eset_auto2(H5E_DEFAULT, NULL, NULL);                                                                                                 \
        H5Eclear();                                                                                                             \
        sslib_g.recent_efunc = _efunc;                                                                                          \
        sslib_g.recent_edata = _edata;                                                                                          \
                                                                                                                                \
    }                                                                                                                           \
                                                                                                                                \
    /* Initialize this interface or fail. Failure does not invoke user-defined cleanups */                                      \
    if (!ss_if_inited_g && ss_if_init()<0) {                                                                                    \
        _estack = H5Eget_current_stack(); /*do asap or we're likely to loose it*/                                               \
        H5Epush2(_estack, __FILE__, _func, __LINE__, ss_eclass_g, ss_if_major_g, SS_MINOR_INIT, "");                       \
        goto gotolabel_leave;                                                                                                   \
    }                                                                                                                           \
                                                                                                                                \
    /* New scope closed at SS_LEAVE() */                                                                                        \
    {                                                                                                                           \
        hbool_t _dummy=false /*because Irix cc thinks an empty statement is an executable statement*/

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Marks beginning of initialization function
 *
 * Description: Every interface has an initialization function that will begin with this macro instead of the usual SS_ENTER.
 *              The only difference between the two is that this one knows that all initialization functions are called
 *              ss_if_init and that there is a interface-scope static variable called ss_if_inited that is true if and only if
 *              the interface has been initialized.  The _IF_NAME_ should be the short name of the interface, the same name as
 *              is used as the second component of almost all of the symbols for that interface.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, July 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ENTER_INIT                                                                                                          \
    if (ss_if_inited_g) return 0;                                                                                              \
    ss_if_inited_g=TRUE;                                                                                                       \
    SS_ENTER(ss_if_init, herr_t);                                                                                              \
    if (ss_err_init()<0) { /*we must initialize the error interface before we can handle errors*/                              \
        fprintf(stderr, "SSlib (SAF) major problems during initialization for interface `%s'. Aborting...", ss_if_name_g);     \
        H5Eprint1(stderr);                                                                                                      \
        ss_abort();                                                                                                            \
    }                                                                                                                          \
    if ((ss_if_major_g=H5Ecreate_msg(ss_eclass_g, H5E_MAJOR, ss_if_name_g))<0) {                                               \
        fprintf(stderr, "SSlib (SAF) major problems during initialization for interface `%s'. Aborting...", ss_if_name_g);     \
        H5Eprint1(stderr);                                                                                                      \
        ss_abort();                                                                                                            \
    }
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Raise an error
 *
 * Description: When a return value from a function call indicates that an error has occurred and the caller wishes to enter
 *              the error handling phase of execution, then either this macro or its cousin SS_ERROR() should be called with
 *              an error code. The _ECODE_ is the final component of an SS_MINOR name and _FMT_ARGS_ is a parenthesised list
 *              consiting of a printf-like format string and optional additional arguments to satisfy the format.
 *
 *              If this macro is called in during the normal flow of control then control branches to the SS_CLEANUP label,
 *              otherwise control continues with the next statement. This allows the cleanup code to optionally replace the
 *              original error stack (the reason we're in error recovery) with an error stack for an error that occurred
 *              during error recovery.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ERROR_FMT(_ecode_, _fmt_args_) do {                                                                                 \
    hid_t _old_estack = _estack;                                                                                               \
    _estack = H5Eget_current_stack(); /*do ASAP or we'll loose it -- most HDF5 functions clobber it*/                          \
    if (_old_estack>=0) H5Eclose_stack(_old_estack);                                                                           \
    H5Epush2(_estack, __FILE__, _func, __LINE__, ss_eclass_g, ss_if_major_g, SS_MINOR_##_ecode_,                          \
            "%lu:%s", ++ss_err_cntl_g.nerrors, ss_err_fmt _fmt_args_);                                                         \
    if (SS_MINOR_##_ecode_ == SS_MINOR_MPI) sslib_g.had_mpierror = TRUE;                                                       \
    ss_error(); /*start debugger?*/                                                                                            \
    if (!_doing_cleanup) {                                                                                                     \
        _doing_cleanup = TRUE;                                                                                                 \
        goto _error;                                                                                                           \
    }                                                                                                                          \
} while (false);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Raise an error
 *
 * Description: This is a simpler version of SS_ERROR_FMT() that doesn't take the variable length format string.  See that
 *              macro for details. See SS_MINOR for error descriptions.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ERROR(_ecode_) SS_ERROR_FMT(_ecode_,(""))

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Raise an error now
 *
 * Description: This macro is identical to SS_ERROR_FMT() except instead of branching directly to the SS_CLEANUP label it
 *              immediately prints the error stack, clears the stack, and falls through to the following statement (unless the
 *              error is an MPI error, in which case the program aborts as it normally does when an error is printed.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ERROR_NOW(_ecode_, _fmt_args_) do {                                                                                  \
    /* Do the SS_ERROR_FMT() part without branching to the SS_CLEANUP label */                                                  \
    {                                                                                                                           \
        assert(!_doing_cleanup);                                                                                                \
        _doing_cleanup = TRUE;                                                                                                  \
        SS_ERROR_FMT(_ecode_, _fmt_args_);                                                                                      \
        _doing_cleanup = FALSE;                                                                                                 \
    }                                                                                                                           \
                                                                                                                                \
    /*  Dump the error stack like HDF5 but using the error function defined on the last entrance to SSlib */                    \
    if (sslib_g.recent_efunc) {                                                                                                 \
        (sslib_g.recent_efunc)(_estack, sslib_g.recent_edata);                                                                  \
        if (sslib_g.had_mpierror) {                                                                                             \
            fprintf(stderr, "SSlib experienced an MPI error. Aborting...\n");                                                   \
            ss_abort();                                                                                                         \
        }                                                                                                                       \
    }                                                                                                                           \
                                                                                                                                \
    /* Clear the error stack and continue */                                                                                    \
    H5Eclose_stack(_estack);                                                                                                    \
    _estack = -1;                                                                                                               \
    H5Eclear();                                                                                                                 \
} while (false) 
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Inquire about a specific error
 *
 * Description: Sometimes instead of immediately failing when a function returns an error indication, we would like to obtain
 *              more information about the nature of the error and make a decision whether to gracefully recover from the error
 *              and continue with the normal error-free flow of control, or push our own information onto the error stack and
 *              enter the error recovery phase. The various SS_STATUS macros can be used for this purpose.
 *
 *              This macro squirrels away the current error stack to protect it from being cleared by various HDF5 functions
 *              that might be called (in the exact same manner as SS_ERROR() except without pushing another entry and without
 *              branching to the SS_CLEANUP label), and returns information about the particular error message that initiated
 *              the error recovery.  The first argument should be an integer to indicate which stack position should be
 *              returned and the second argument is a pointer to an H5E_error_t variable that points directly into the error
 *              stack and which is valid only until another call to one of the SSlib error handling macros.
 *
 *              Error stack positions are measured so that zero is the earliest entry on the stack, 1 is the next entry, and
 *              so on.  If the index is out of range then _ERR_INFO_ is set to the null pointer and a SS_MINOR_RANGE error is
 *              pushed onto the current error stack (and an assertion fails if they're turned on).
 *
 * Example:
 *              if (ss_table_synchronize(table, scope, test_only)<0) {
 *                  const H5E_error2_t *status;
 *                  SS_STATUS(0, status);
 *                  if (status->min_id == SS_MINOR_SKIPPED) {
 *                      if (...) SS_ERROR(FAILED); // replace saved status stack and clean up
 *                      // all is okay -- continue as normal
 *                      SS_STATUS_OK;
 *                  } else {
 *                      // something else is wrong -- raise an error
 *                      SS_REFAIL;
 *                      SS_ERROR(FAILED);
 *                  }
 *              }
 *
 * Programmer:  Robb Matzke
 *              Monday, August 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_STATUS(_position_,_err_info_) do {                                                                                   \
    hid_t _old_estack = _estack;                                                                                                \
    _estack = H5Eget_current_stack(); /*do ASAP or we'll loose it -- most HDF5 functions clobber it*/                           \
    if (_old_estack>=0) H5Eclose_stack(_old_estack);                                                                            \
    _err_info_ = ss_err_info(_estack, _position_);                                                                              \
    assert(_err_info_);                                                                                                         \
} while (false)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Save error status
 *
 * Description: This squirrels away the current error stack just like SS_STATUS() except it doesn't retrieve information about
 *              any specific error in the stack.
 *
 * Programmer:  Robb Matzke
 *              Thursday, September 18, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_SAVE do {                                                                                                            \
    hid_t _old_estack = _estack;                                                                                                \
    _estack = H5Eget_current_stack(); /*do ASAP or we'll loose it -- most HDF5 functions clobber it*/                           \
    if (_old_estack>=0) H5Eclose_stack(_old_estack);                                                                            \
} while (false) 

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Clears an error condition
 *
 * Description: Sometimes when a function fails and pushes an error onto the error stack, the caller decides that the error
 *              is not really an error and desires to continue with the normal flow of execution. When this happens it is
 *              important to clear the error stack so that if and when the next error occurs it isn't pushed onto a stack that
 *              is left over from some previous unrelated error.
 *
 * Example:
 *              Example 1: To completely ignore an error
 *
 *                  hbool_t test_only = FALSE;
 *                  if (NULL==ss_prop_get(props, "test", H5T_NATIVE_HBOOL, &test_only)) SS_STATUS_OK;
 *                  SS_ASSERT_TYPE(scope, ss_scope_t); // this will then start a new stack on failure
 *
 *              Example 2: To cancel an error after determining that it's non-fatal. See the SS_STATUS() example.
 *
 *              Example 3: To cancel an error during error recovery, returning the success value instead.
 *
 *                  SS_CLEANUP:
 *                      if (recovery_is_complete) SS_STATUS_OK; // cancel the error
 *                      SS_LEAVE(retval);                       // and return the success value
 *
 * Programmer:  Robb Matzke
 *              Monday, August 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_STATUS_OK do {                                                                                                      \
    if (_estack>=0) {                                                                                                          \
        H5Eclose_stack(_estack);                                                                                               \
        _estack=-1;                                                                                                            \
    }                                                                                                                          \
    H5Eclear();                                                                                                                \
} while (false)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Fail again in an identical manner
 *
 * Description: The SS_STATUS(), SS_ERROR(), and SS_ERROR_FMT() macros squirrel away the current error stack to protect it
 *              from being cleared by other HDF5 functions. Invoking this function makes the current error stack to be the
 *              same as it was before. The new error stack should be handled in the same manner as if SS_REFAIL was a real
 *              function that had just failed. See SS_STATUS() for an example.
 *
 * Programmer:  Robb Matzke
 *              Monday, August 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_REFAIL do {                                                                                                         \
    if (_estack>=0) H5Eset_current_stack(_estack);                                                                             \
    else H5Eclear();                                                                                                           \
    _estack = -1;                                                                                                              \
} while (false) 

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Marks error recovery section
 *
 * Description: Every function body is partitioned into the part that handles the normal flow of control, and the part that
 *              cleans up after an error. The SS_CLEANUP label marks the boundary between the two.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_CLEANUP goto gotolabel_leave; _error


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Marks end of function
 *
 * Description: Any function that begins with a call to SS_ENTER() should end with a call to this macro. The argument is the
 *              value that should be returned if an error did not occur (if an error occurred the argument is ignored).
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_LEAVE(_retval_)                                                                                                     \
        if (_dummy) abort(); /*constant zero, but here to turn off warnings about it being otherwise unused*/                  \
      gotolabel_leave:                                                                                                         \
        if (!_doing_cleanup) _retval=_retval_; /*save success return value only if no error occurred*/                         \
    }                                                                                                                          \
                                                                                                                               \
    /* this is the only way we're allowed to leave an SSlib function */                                                        \
    assert(sslib_g.call_depth>0);                                                                                              \
    --sslib_g.call_depth;                                                                                                      \
                                                                                                                               \
    /* Restore the error handler saved by SS_ENTER(). As long as we're inside SSlib the HDF5 automatic error reporting         \
     * will be turned off, so we only have to call H5Eset_auto() to turn it back on. */                                        \
    if (_efunc) H5Eset_auto2(H5E_DEFAULT, _efunc,_edata);                                                                 \
                                                                                                                               \
    if (_doing_cleanup) {                                                                                                      \
        /* An error was reported by SS_ERROR_FMT() or SS_ERROR(). SSlib must perform the same actions as HDF5, namely upon     \
         * returning from the top-level call to SSlib the current error stack should contain relevant information, and if      \
         * automatic error reporting is turned on then the appropriate error callback should be invoked.  We must be           \
         * careful of function call ordering here because most HDF5 functions will clear the current error stack as their      \
         * first operation. */                                                                                                 \
        int _estack_size;                                                                                                      \
        assert(_estack>=0);                                                                                                    \
        if (_efunc) {                                                                                                          \
            _estack_size = H5Eget_num(_estack);                                                                                \
            if (_estack_size>0) {                                                                                              \
                (_efunc)(_estack, _edata); /*simulate what HDF5 would do for reporting errors */                               \
                if (sslib_g.had_mpierror) {                                                                                    \
                    fprintf(stderr, "SSlib experienced an MPI error. Aborting...\n");                                          \
                    ss_abort();                                                                                                \
                }                                                                                                              \
            }                                                                                                                  \
        }                                                                                                                      \
        H5Eset_current_stack(_estack);                                                                                         \
        _estack = -1; /*closed by H5Eset_current_stack()*/                                                                     \
    } else {                                                                                                                   \
        /* If an error stack was saved (probably with SS_STATUS() or SS_SAVE) but the function is returning success            \
         * then we should dispose of that stack. */                                                                            \
        if (_estack>0) H5Eclose_stack(_estack);                                                                                \
        /* Without the following H5Eclear() it is possible for an SSlib function to return failure and have a current          \
         * error stack left over from some unrelated event. This probably can only happen for functions that somehow           \
         * violate the normal SSlib manner of writing functions (or perhaps one of these HDF5 calls in this macro              \
         * failed). */                                                                                                         \
        /* H5Eclear();  // temporarily turned off to study performance [rpm 2004-09-25] */                                     \
    }                                                                                                                          \
    return _retval;                                                                                                            \
}
 
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Sets non-default return value for error recovery
 *
 * Description: If the error recovery portion of a function is entered then the default failure value (as determined by the
 *              function return type specified in the SS_ENTER() macro) is returned. However, this macro can be called at any
 *              time to set a new failure return value. It does not alter flow control.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_RETVAL(_retval_) _retval = _retval_

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Asserts object runtime type
 *
 * Description: The first argument should be an object of the specified type, valid at runtime. If it isn't then a TYPE
 *              error is raised. The _TYPE_ argument is a C datatype like ss_fieldtmpl_t.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ASSERT_TYPE(_obj_,_type_) do {                                                                                       \
    if (!(_obj_) || SS_MAGIC_OF(_obj_)!=SS_MAGIC(_type_))                                                                       \
        SS_ERROR_FMT(TYPE, ("%s should be type %s", #_obj_, #_type_))                                                           \
} while (false)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Asserts object runtime type and existence
 *
 * Description: The first argument should be an object of the specified type, valid at runtime. If it isn't then a TYPE
 *              error is raised. The _TYPE_ argument is a C datatype like ss_fieldtmpl_t.  This function is like
 *              SS_ASSERT_TYPE() except it also checks that the _OBJ_ exists in memory (i.e., the object can be dereferenced).
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ASSERT_MEM(_obj_,_type_) do {                                                                                       \
    SS_ASSERT_TYPE(_obj_,_type_);                                                                                              \
    if (NULL==ss_pers_deref((ss_pers_t*)_obj_)) SS_ERROR_FMT(TYPE, ("%s should be loadable", #_obj_));                         \
} while (false)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Asserts object runtime class
 *
 * Description: The first argument should be an object of the specified class, valid at runtime. If it isn't then a TYPE
 *              error is raised. The _CLS_ argument should be a C class datatype like ss_pers_t.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ASSERT_CLASS(_obj_,_cls_) do {                                                                                      \
    if (!(_obj_) || SS_MAGIC_CLASS(SS_MAGIC_OF(_obj_))!=SS_MAGIC_CLASS(SS_MAGIC(_cls_)))                                       \
        SS_ERROR_FMT(TYPE, ("%s should be class %s", #_obj_, #_cls_))                                                          \
} while (false);

#ifndef NDEBUG
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Assertion checker
 *
 * Description: This macro is similar to the standard assert() macro except it uses the SSlib error interface to report the
 *              failed assertion.
 *
 * Issue:       Use this macro only where it is valid to use SS_ERROR().
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 5, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_ASSERT(_condition_) do {                                                                                            \
    if (!(_condition_)) SS_ERROR_FMT(ASSERT, ("%s", #_condition_));                                                            \
} while (false);
#else
/*DOCUMENTED*/
#define SS_ASSERT(_condition_) /*void*/
#endif
    
#ifdef HAVE_FUNCTION
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Runtime check the function name
 *
 * Description: The SS_ENTER() macro takes the function name as its first argument. On some compilers (i.e., GCC) the function
 *              name is also available from the __FUNCTION__ macro and we can use that to check at runtime whether the
 *              supplied name for SS_ENTER() is the same as what is known by the compiler. It is easy to mix up names,
 *              especially when code is copied and pasted or a function name is changed, and doing so makes error messages
 *              confusing.  We can also do a compile-time check by seeing if the function is defined.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_CHECK_FUNCTION_NAME(_function_) \
    assert(_function_ && (!strcmp(#_function_, __FUNCTION__) || !strcmp(#_function_, "ss_if_init")))
#else
/*DOCUMENTED*/
#define SS_CHECK_FUNCTION_NAME(_function_) assert(true && _function_) /*trying to avoid "constant controlling expression" warns*/
#endif

#ifdef HAVE_FUNCTION
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Begin a functionality test
 *
 * Description: This family of macros can be used in the SSlib test suite to perform a test of some functionality. The
 *              SS_CHECKING() and SS_END_CHECKING() macros should be paired with curly braces. Inside the body of that
 *              construct may be zero or more calls to SS_FAILED() or SS_FAILED_WHEN(). If either of the failure macros is
 *              executed flow control branches to the SS_END_CHECKING macro.
 *
 *              The SS_END_CHECKING_WITH() macro can be used in place of SS_END_CHECKING(). It takes a single argument which
 *              is arbitrary code to execute if an error was detected in the body of the SS_CHECKING() construct. Typically
 *              the argument will be something alone the lines of `return FAILURE' or `goto error'.
 *
 *              The argument for SS_CHECKING() should be a string that will be printed to stderr after the word "checking".
 *              The string is printed only if _print is non-zero (similarly for the output from related macros).
 *
 *                  FILE *_print = 0==self ? stderr : NULL;
 *                  int nerrors=0;
 *                  SS_CHECKING("file opening operations") {
 *                      file1 = ss_file_create(....);
 *                      if (!file1) SS_FAILED_WHEN("creating");
 *                      file2 = ss_file_open(....);
 *                      if (!file2) SS_FAILED_WHEN("opening");
 *                  } SS_END_CHECKING_WITH(nerrors++);
 *
 *                  SS_CHECKING("file close") {
 *                      if (ss_file_close(....)<0) SS_FAILED;
 *                  } SS_END_CHECKING;
 *
 * Programmer:  Robb Matzke
 *              Monday, August 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_CHECKING(_what_) {                                                                                                  \
    int         _failed_checks=0; /*positive implies failure; negative implies skips*/                                         \
    const char *_what = _what_;                                                                                                \
    if (_print) fprintf(_print, "\n%s checking %s...\n", __FUNCTION__,  _what_);                                               \
    do {
#else
/*DOCUMENTED*/
#define SS_CHECKING(_what_) {                                                                                                  \
    int         _failed_checks=0;                                                                                              \
    const char *_what = _what_;                                                                                                \
    if (_print) fprintf(_print, "\nchecking %s...\n", _what_);                                                                 \
    do {
#endif

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Indicate functionality test failure
 * Description: See SS_CHECKING()
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_FAILED                                                                                                              \
    {                                                                                                                          \
        _failed_checks++;                                                                                                      \
        if (_print) fprintf(_print, "failed:  %-97s ***FAILED***\n", _what);                                                  \
        break; /* out of SS_CHECKING `do' loop */                                                                              \
    }
 
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Indicate functionality test failure
 * Description: See SS_CHECKING()
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_FAILED_WHEN(_mesg_)                                                                                                 \
    {                                                                                                                          \
        _failed_checks++;                                                                                                      \
        if (_print)                                                                                                            \
            fprintf(_print, "failed:  %s [%s]%*s ***FAILED***\n", _what, _mesg_,                                               \
                    (int)(94-MIN(94, strlen(_what)+strlen(_mesg_))), "");                                                    \
        break; /* out of SS_CHECKING `do' loop */                                                                              \
    }

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Indicate functionality test skipped
 * Description: See SS_CHECKING()
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_SKIPPED                                                                                                             \
    {                                                                                                                          \
        _failed_checks=-1;                                                                                                     \
        if (_print) fprintf(_print, "skipped: %-107s  --SKIPPED--\n", _what);                                                  \
        break; /* out of SS_CHECKING `do' loop */                                                                              \
    }
 
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Indicate functionality test skipped
 * Description: See SS_CHECKING()
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_SKIPPED_WHEN(_mesg_)                                                                                                \
    {                                                                                                                          \
        _failed_checks=-1;                                                                                                     \
        if (_print)                                                                                                            \
            fprintf(_print, "skipped: %s [%s]%*s  --SKIPPED--\n", _what, _mesg_,                                               \
                    (int)(104-MIN(104, strlen(_what)+strlen(_mesg_))), "");                                                    \
        break; /* out of SS_CHECKING `do' loop */                                                                              \
    }

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     End functionality test
 * Description: See SS_CHECKING()
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_END_CHECKING                                                                                                        \
    } while (false);                                                                                                           \
    if (!_failed_checks && _print) {                                                                                           \
        fprintf(_print, "passed:  %-100s PASSED\n", _what);                                                                    \
    }                                                                                                                          \
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     End functionality test
 * Description: See SS_CHECKING()
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_END_CHECKING_WITH(_code_)                                                                                           \
    } while (false);                                                                                                           \
    if (!_failed_checks) {                                                                                                     \
        if (_print) fprintf(_print, "passed:  %-100s PASSED\n", _what);                                                        \
    } else if (_failed_checks<0) {                                                                                             \
        /* do nothing when skipping a test */                                                                                  \
    } else {                                                                                                                   \
        _code_;                                                                                                                \
    }                                                                                                                          \
}

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes */
herr_t ss_err_init(void);
hid_t ss_err_init1(hid_t *to_return, const char *mesg);
const char *ss_err_fmt(const char *fmt, ...);
void ss_err_mpierror(MPI_Comm *comm, int *ecode, ...);
const H5E_error2_t *ss_err_info(hid_t estack, int position);
herr_t ss_err_info_cb(unsigned n, const H5E_error2_t *info, void *_walker);
herr_t ss_err_print(hid_t estack, void *econtrol);
herr_t ss_err_print_cb(unsigned n, const H5E_error2_t *einfo, void *econtrol);
herr_t ss_err_check(int depth);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSERR_H*/
