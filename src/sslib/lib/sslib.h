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
#ifndef SS_HAVE_SSLIB_H
#define SS_HAVE_SSLIB_H

/* Results from feature tests */
#ifdef WIN32
#   include "SAFconfig-WIN32.h"
#elif defined(JANUS)
#   include "SAFconfig-JANUS.h"
#else
#   include "SAFconfig.h"
#endif

/* System includes (alphabetical) */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#define H5_USE_16_API
#include <hdf5.h>
#ifdef HAVE_REGEX_H
#   include <regex.h>
#endif
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif
#ifdef HAVE_ZLIB_H
#   include <zlib.h>
#endif
#ifdef HAVE_WINDOWS_H
#   include <windows.h>      /* Necessary for WIN32 */
#endif
#ifdef HAVE_IO_H
#   include <io.h>      /* Necessary for WIN32 */
#endif
#ifdef HAVE_DIRECT_H
#   include <direct.h>  /* Necessary for WIN32 */
#endif
#ifdef HAVE_PROCESS_H
#   include <process.h> /* Necessary for WIN32 */
#endif

/* Although `long long' is part of the revised ANSI-C some compilers don't support it yet.  We define `long_long' as the
 * longest integral integer type supported by the compiler, usually 64 bits. It must be legal to qualify `long_long' with
 * `unsigned'. */
#if SIZEOF_LONG_LONG>0
#   define long_long	long long
#elif SIZEOF___INT64>0
#   define long_long	__int64	/*Win32*/
#   undef SIZEOF_LONG_LONG
#   define SIZEOF_LONG_LONG SIZEOF___INT64
#else
#   define long_long	long int
#   undef SIZEOF_LONG_LONG
#   define SIZEOF_LONG_LONG SIZEOF_LONG
#endif

/* If MPI isn't available then define some MPI datatypes that are used elsewhere. This shouldn't
 * interfere with a client because they'll never see these definitions and none of them will affect linking. */
#ifndef HAVE_PARALLEL
typedef int MPI_Datatype;
typedef int MPI_Comm;
typedef int MPI_Request;
typedef int MPI_Errhandler;
typedef ssize_t MPI_Aint;
typedef int MPI_Info;
#define MPI_UNDEFINED (-32766)
#define MPI_INFO_NULL 0
#define MPI_INT 0
#define MPI_CHAR 0
#define MPI_UNSIGNED_LONG 0
#define MPI_BYTE 0
#define MPI_DATATYPE_NULL 0
#define MPI_DOUBLE 0
#endif /*HAVE_PARALLEL*/

/* Misc. */
#ifndef __cplusplus
extern const hbool_t    false;
extern const hbool_t    true;
#endif

#define FALSE           0
#define TRUE            1
#define SS_MAXDIMS      4                       /* Maximum dataset dimensionality for SSlib-created datasets */
#define SS_NOSIZE       ((size_t)(-1))

/* These type qualifiers can be used to suppress warnings from GCC for function formal arguments that are unused. They should
 * not be used to suppress warnings due to the function being partially unimplemented since the warning serves as a reminder
 * of that fact.
 *   UNUSED        -- marks arguments as intentionally unused in both serial and parallel versions
 *   UNUSED_SERIAL -- marks arguments as intentionally unused in serial code but used in parallel versions */
#ifndef UNUSED
#   ifdef HAVE_ATTRIBUTE
#       define UNUSED __attribute__((__unused__))
#   else
#       define UNUSED /*ARGSUSED*/
#   endif
#endif
#ifdef HAVE_PARALLEL
#   define UNUSED_SERIAL /*void*/
#else
#   define UNUSED_SERIAL UNUSED
#endif

/* Forward declarations */
struct ss_table_t;

/* Local includes (by layer) */
#include "ssvers.h"                             /* generated file containing version numbers */
#include "sshdf5.h"                             /* extra HDF5ish stuff */
#include "sserr.h"                              /* error handling */
#include "ssobj.h"                              /* general object stuff */
#include "ssaio.h"                              /* asynchronous I/O */
#include "ssprop.h"                             /* property lists */
#include "ssval.h"                              /* generic data value stuff */
#include "ssstring.h"                           /* variable length character strings */
#include "ssarray.h"                            /* variable length arrays */
#include "ssobj.h"                              /* object base class */
#include "sspers.h"                             /* persistent objects */
#include "sstable.h"                            /* object tables: they hold persistent objects */
#include "ssscope.h"                            /* file scopes */
#include "ssblob.h"                             /* blobs that point to raw data */
#include "ssgfile.h"                            /* shared (global) file data */
#include "ssfile.h"                             /* files and file objects */
#include "ssattr.h"                             /* persistent object attributes */
#include "ssmpi.h"                              /* MPI support */
#include "ssdebug.h"                            /* Interactive debugging functions */

/* Audience:    Public
 * Chapter:     Library
 * Purpose:     Initialize the library
 * Description: This is a macro around ss_init_func() that also checks header/library compatibility.
 * Return:      Same as ss_init_func()
 * Parallel:    Same as ss_init_func() */
#define ss_init(COMM) (ss_check_version(COMM,                                                                                  \
                                        SS_VERS_MAJOR, SS_VERS_MINOR, SS_VERS_RELEASE, SS_VERS_ANNOT,                          \
                                        SS_INSTALL_INCLUDEDIR),                                                                \
                       ss_init_func(COMM))

/* Audience:    Private
 * Chapter:     Miscellaneous
 * Purpose:     Deallocate memory
 * Description: This is just a convenience macro for the free() library function. It only calls free() if the pointer,
 *              X, is non-null.
 * Return:      Always returns the null pointer. */
#define SS_FREE(X)      (void*)((long)((X) && (free(X),NULL)))

#define MIN(X,Y)        ((X)<(Y)?(X):(Y))
#define MIN3(X,Y,Z)     MIN(MIN(X,Y),Z)
#define MAX(X,Y)        ((X)>(Y)?(X):(Y))
#define MAX3(X,Y,Z)     MAX(MAX(X,Y),Z)
#define MINMAX(X,Y,Z)   MAX(X, MIN(Y, Z))       /* Returns Y bounded between X and Z */
#define NELMTS(X)       (sizeof(X)/sizeof((X)[0]))
#define ALIGN(X,A)      (A*((X+A-1)/A))

/* Audience:    Public
 * Chapter:     Miscellaneous
 * Purpose:     Global bit flags
 * Description: Various bit flags that are useful to different interfaces. These global flags use the high-order bits of
 *              a word while the interface-specific flags use the low-order bits. */
#define SS_ALLSAME              0x00010000              /* All applicable tasks are supplying identical data or performing the
                                                         * same operation.  For instance, in a call to ss_pers_new() this
                                                         * indicates that the new object can be "born synchronized" because
                                                         * all tasks will have the same value for the object without needing
                                                         * to communicate. Absence of this bit simply means that different
                                                         * tasks might be supplying different data but doesn't guarantee that
                                                         * the data is different. (Note: this value must be distinct from TRUE,
                                                         * but don't worry, it's checked at runtime.) */
#define SS_STRICT               0x00020000              /* Flag that causes certain functions to "try harder" to do something */

#if SS_ALLSAME==TRUE
#  error "SS_ALLSAME and TRUE must be different values"
#endif

/* Audience:    Private
 * Chapter:     Miscellaneous
 * Purpose:     Per-task transient file
 * Description: The SAF library occassionally needs a scratch file and this constant is its name. The file will be
 *              created as a task-local transient file by each MPI task. */
#define SS_PERTASK_FILENAME "/SAF:SSlib:temp"

/* Audience:    Private
 * Chapter:     Miscellaneous
 * Purpose:     Extend allocated array
 * Description: This macro extends a dynamically allocated array so it contains at least _MINSIZE_ elements and stores the new
 *              total size of the array in _NALLOC_.  If the realloc() call fails then a resource error is raised and the text
 *              of that error message is the _PTR_ expression. */
#define SS_EXTEND(_ptr_,_minsize_,_nalloc_) do {                                                                               \
    if ((_minsize_) > (_nalloc_)) {                                                                                            \
        size_t _ss_extend_n = MAX((_minsize_), 2*(_nalloc_));                                                                  \
        void *_ss_extend_x = realloc((_ptr_), _ss_extend_n*sizeof(*(_ptr_)));                                                  \
        if (!_ss_extend_x) SS_ERROR_FMT(RESOURCE, ("%s", #_ptr_));                                                             \
        (_ptr_) = _ss_extend_x;                                                                                                \
        (_nalloc_) = _ss_extend_n;                                                                                             \
    }                                                                                                                          \
} while(false)

/* Audience:    Private
 * Chapter:     Miscellaneous
 * Purpose:     Extend allocated array and zero
 * Description: This macro extends a dynamically allocated array so it contains at least _MINSIZE_ elements and stores the new
 *              total size of the array in _NALLOC_.  If the realloc() call fails then a resource error is raised and the text
 *              of that error message is the _PTR_ expression. The new elements are zero-filled. */
#define SS_EXTEND_ZERO(_ptr_,_minsize_,_nalloc_) do {                                                                          \
    if ((_minsize_) > (_nalloc_)) {                                                                                            \
        size_t _ss_extend_n = MAX((_minsize_), 2*(_nalloc_));                                                                  \
        void *_ss_extend_x = realloc((_ptr_), _ss_extend_n*sizeof(*(_ptr_)));                                                  \
        if (!_ss_extend_x) SS_ERROR_FMT(RESOURCE, ("%s", #_ptr_));                                                             \
        (_ptr_) = _ss_extend_x;                                                                                                \
        memset((_ptr_)+(_nalloc_), 0, (_ss_extend_n-(_nalloc_))*sizeof(*(_ptr_)));                                             \
        (_nalloc_) = _ss_extend_n;                                                                                             \
    }                                                                                                                          \
} while(false)

/* Library properties and various global variables */
typedef struct ss_lib_props_t {
    /* MPI-Related stuff */
    hbool_t             initialized;    /* Has the library been initialized? */
    hbool_t             finalized;      /* Has the library been finalized? */
    MPI_Comm            comm;           /* Library-wide communicator, defining what tasks initialized the library */
    MPI_Errhandler      ehandler;       /* MPI handle to the MPI error handler, ss_err_mpierror() */
    size_t              serial;         /* Job-wide unique integer; just increment each time you need one */

    /* Error handling and debugging */
    size_t              call_depth;     /* Current depth in function calls, maintained by SS_ENTER and SS_LEAVE */
    hbool_t             ignore_mpierror;/* If non-zero then do not register an error handler for MPI-related errors */
    hbool_t             had_mpierror;   /* True iff SSlib experienced an MPI-related error that is normally fatal */
    hbool_t             debug_signal;   /* True if SSlib should start a debugger on any process that dies from a signal */
    size_t              debug_error;    /* Non-zero if SSlib should start a debugger when error N is pushed onto the stack. */
    hbool_t             show_error_ids; /* True if SSlib should show error identification numbers in the error stack. */
    char                debugger[256];  /* The name (or empty) of the default debugger. See ss_debug_start(). */
    hbool_t             sync_check;     /* Verify persistent object synchronization state by computing checksums when true. If
                                         * set to SS_STRICT then raise an error in addition to printing debugging information
                                         * when an inconsistency is discovered. */
    hbool_t             sync_bcast;     /* If set then display synchronization object broadcasts to stderr. */
    hbool_t             tpio_alloc;     /* If set then display info about allocation of tasks for two-phase I/O */
    char                *banner;        /* If non-null then ss_init() will print this string to stderr upon return */
    int                 command_fd;     /* If non-negative then ss_debug() will read commands from this file descriptor */
    H5E_auto2_t         recent_efunc;   /* Error handler function seen on latest entrance to SSlib */
    void                *recent_edata;  /* Error handler data seen on latest entrance to SSlib */
    FILE                *warnings;      /* The optional stream where miscellaneous SSlib warnings are printed. */

    /* Global file data structures */
    struct {
        ss_gfile_t      *ent;           /* Dynamic array of global file information */
        size_t          nused;          /* Size of `gfile' array */
        size_t          nalloc;         /* Elements allocated for `gfile' array */
    } gfile;

    /* Each task will also have it's own transient file for objects that it might need to create on the fly */
    struct {
        ss_file_t       *file;          /* File handle for the temporary file */
        ss_scope_t      *tscope;        /* Top scope of the temporary file */
    } temp;
    
} ss_lib_props_t;
extern ss_lib_props_t sslib_g;

#ifdef __cplusplus
extern "C" {
#endif

/* misc prototypes */
void ss_abort(void);
herr_t ss_init_func(MPI_Comm communicator);
herr_t ss_sslib_init(void);
htri_t ss_initialized(void);
herr_t ss_finalize(void);
herr_t ss_zap(void);
int ss_config(MPI_Comm comm, const char *filename);
herr_t ss_error(void);
char *ss_bytes(hsize_t nbytes, char *buf);
char *ss_insert_commas(char *buf);
unsigned long ss_adler32(unsigned long adler, const char *buf, size_t len);
int ss_check_version(MPI_Comm comm, unsigned major, unsigned minor, unsigned rel, const char *annot, const char *incdir);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSLIB_H*/
