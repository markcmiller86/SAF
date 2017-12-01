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
#include "sslib.h"
SS_IF(sslib);

/* Boolean values with storage so we can pass pointers. This makes calls like the following possible:
 *    ss_prop_add(propertylist, "some-property", H5T_NATIVE_HBOOL, &true);
 * which adds a property to a list and gives it an initial value of TRUE */
const hbool_t   false=0;
const hbool_t   true=1;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Introduction
 *
 * Description: The SAF Support Library (SSlib) grew out of experience the Sets and Fields (SAF) team had with the former
 *              Vector Bundle Tables (VBT) layer and Data Sharability Layer (DSL) and to some extent with the Hierarchical
 *              Data Format version 5 (HDF5, see https://support.hdfgroup.org/HDF5/doc/index.html) library from NCSA. It was
 *              decided that in order to increase performance, generalize some underlying functionality, and improve code
 *              engineering that we would embark on an effort to rewrite most of VBT and DSL with these goals in mind:
 *
 *              * *Reduced*Communication*: We learned by experience that designing an API that requires underlying communication
 *                makes it extremely difficult to optimize for performance at a later time, and that algorithms that require
 *                communication can be substantially slower than those that don't. So algorithms will be used to reduce
 *                communication and the API will be designed so that cases of repeated communication in the old VBT/DSL API
 *                can be performed just once, and cases of related communication can be combined into single messages.
 *
 *              * *Variable*Length*Datatypes*: The VBT design set aside a fixed size character array for every string, which
 *                resulted in substantial wasted file space and lower bandwidth and precluded the client from using arbitrary
 *                length strings. The SSlib will employ HDF5 variable length datatypes to avoid these problems.
 *
 *              * *Transient*Objects*: The original VBT specification had no provision for creating objects that exist only in
 *                memory, although eventually this was patched in using HDF5's !core virtual file driver. Transient objects
 *                are designed into SSlib.
 *
 *              * *Object*Deletion*: VBT did not allow for easy deletion of objects from the database. Although SSlib probably
 *                won't allow individual objects to be deleted, it will allow entire scopes to be deleted, freeing up memory
 *                in the HDF5 file as provided by the HDF5 library and file format.
 *
 *              * *Every*File*a*Database*: SAF had a notion of supplemental data files that were pointed to by a single
 *                master file, collectively called the database. It was not possible to open just a supplemental file, but
 *                one always had to open the master file. SSlib will make no distinction between master and supplemental
 *                files, rather every file will be a self-contained database. SAF allowed supplemental files to be missing;
 *                SSlib allows databases to be missing.
 *
 *              * *Partial*Metadata*Reads*: VBT always read all the object definitions from the database whenever a database
 *                was opened. SSlib will only read subsets of a file called "scopes" and only when those scopes are accessed
 *                and only by the tasks accessing those scopes.
 *
 *              * *Interfile*Object*References*: A VBT file could only refer to objects that were also in the same file. SSlib
 *                files will have the capability to refer to objects that are in some other file.
 *
 *              * *Multiple*References*: In SSlib, two or more objects may make references to a common third object or to
 *                common raw data, thus reducing the required storage.
 *
 *              * *Object*Copying*: Tools such as !safdiff formerly needed extensive coding in order to copy an object (e.g.,
 *                a field) from one database to another. SSlib will provide that functionality at a much lower layer.  This
 *                also simplifies the implementation of Object Registries in SAF by moving much of that functionality downward
 *                in the software stack.
 *
 *              * *Common*Error*Handling*: A code engineering aspect of SSlib is to generalize the HDF5 error handling
 *                subsystem, turn it into a public programming interface, and use it for SSlib and eventually higher
 *                software layers. This unifies the error recording and reporting features of all layers involved.
 *
 *              * *Flexible*File*Decomposition*: As mentioned already, SAF required all object metadata to be stored in a
 *                single master file with optional supplemental files to hold raw field data.  SSlib relaxes that constraint
 *                so that operational environments like SILO's multi-file output are possible, where the MPI job is
 *                partitioned into smaller subsets of tasks with each subset responsible for a single database, the databases
 *                being "sewed" together later.
 *
 *              * *Reduced*Code*Generation*: SSlib replaces the more than 12,000 lines of !vbtgen (a table parser and C code
 *                generator) with a few hundred lines of perl that does something very similar.  In addition, the perl script
 *                parses standard C typedefs instead of a custom language.
 *
 *              * *Better*HDF5*Coupling*: The DSL datatype interface (more than 12,000 lines of library code) will be replaced
 *                with the HDF5 datatype interface plus a few additional functions that may migrate into the HDF5 library.
 *
 *              The plots below show the before and after scalability and performance improvements achieved.
 *
 *              Pre-optimized raw data I/O aggregate bandwidth scalability
 *
 *              [figure plot01.jpg]
 *
 *              Pre-optimized overall I/O aggregate bandwidth scalability
 *
 *              [figure plot02.jpg]
 *
 *              Optimized raw data I/O aggregate bandwidth scalability
 *
 *              [figure plot03.jpg]
 *
 *              Optimized overall I/O aggregate bandwidth scalability
 *
 *              [figure plot04.jpg]
 *
 *              Comparison of SAF and Silo Ale3d restart file dump times
 *
 *              [figure plot05.jpg]
 *
 *              Comparison of SAF Ale3d restart file dump times by functionality
 *
 *              [figure plot06.jpg]
 *
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library
 *
 * Description: The SSlib library must be explicitly initialized before being used and should be finalized when the client
 *              is finished using it.  In addition, this chapter contains some additional functions that operate on the
 *              library as a whole.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

ss_lib_props_t sslib_g;                         /* Global library properties */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Library
 * Purpose:     Interface initializer
 *
 * Description: Initializes the "interface". However, since this source file isn't really an interface, this function doesn't
 *              need to do much.
 *
 * Return:      Returns non-negative on success, negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, July 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_sslib_init(void)
{
    SS_ENTER_INIT;
    /* Nothing to do. See ss_init(), which is for explicit SSlib initialization. */
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library
 * Purpose:     Initialize the library
 *
 * Description: Call this function to initialize SSlib. It's use is entirely optional since SSlib generally initializes each
 *              layer of its software stack as it becomes necessary.  However, if only a subset of the MPI tasks will be
 *              making calls to SSlib then this function can be invoked to define what tasks own the library.
 *
 *              This function initializes the collective parts of some other layers as well when those other layers are
 *              largely independent and might not have an opportunity to do their own collective initialization.
 *
 *              Normally the client initializes the library with the ss_init() macro.
 *
 * Issue:       We cannot pass things to this function with property lists since those property lists rely on SSlib having
 *              been initialized first.
 *
 * Return:      Returns non-negative on success; negative on failure.  It is an error to call this function more than one time
 *              or to call it after the library has been implicitly initialized.
 *
 * Parallel:    Collective across the specified communicator.
 *
 * Also:        ss_initialized() ss_init()
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 17, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_init_func(MPI_Comm communicator      /* Library communicator defining the maximal set of MPI tasks that can be involved in
                                         * various collective SSlib function calls. However, many collective SSlib functions
                                         * can operate on a subset of this communicator. If SSlib is implicitly initialized
                                         * then MPI_COMM_WORLD is assumed. When SSlib is compiled without MPI support then the
                                         * COMMUNICATOR argument is just an integer that's ignored by this function. */
        )
{
    SS_ENTER(ss_init_func, herr_t);
    ss_prop_t           *tfile_props=NULL;
    MPI_Comm            selfcomm=SS_COMM_SELF;
    size_t              nbits;
    int                 self=0, ntasks=1;

    /* This is `static' so that we can edit the default path directly in the library binary if necessary. */
    static char         cfile_name[2048] = SS_INSTALL_SYSCONFDIR "/include/sslib.conf\0"
                                           "*** PRECEDING STRING IS 2048 BYTES TOTAL ***";

    if (sslib_g.initialized) SS_ERROR_FMT(INIT, ("the library is already initialized"));
    sslib_g.command_fd = -1;
    sslib_g.warnings = fdopen(2, "w");
    
#ifdef HAVE_PARALLEL
    MPI_Comm_rank(communicator, &self);
    MPI_Comm_size(communicator, &ntasks);
    if (MPI_Comm_dup(communicator, &(sslib_g.comm))) SS_ERROR(MPI);
#endif

    /* Set the HDF5 automatic error reporting. We can't do this directly with H5Eset_auto() because the SS_LEAVE() macro will
     * clobber that. */
    SS_ASSERT(1==sslib_g.call_depth); /*not really necessary, but will catch any potential problems from changing SS_LEAVE() */
    _efunc = ss_err_print;
    _edata = &ss_err_cntl_g;

    /* Initialize the job-wide serial numbers. The high order bits are the MPI rank number of this task within the library
     * communicator and the low order bits are initialized to zero. Whenever you need a unique number, just increment the
     * serial number. */
    for (nbits=0; nbits<8*sizeof(sslib_g.serial) && (size_t)(ntasks-1)>>nbits!=0; nbits++) /*void*/;
    sslib_g.serial = (size_t)self << (8*sizeof(sslib_g.serial) - nbits);

    /* Configure the library first by reading the configuration file and then by processing the SSLIB_DEBUG environment
     * variable. If the library isn't installed yet then the configuration file will be located in the source directory,
     * so look in the installation directory first, then the source directory. If the file can't be found then complain but
     * don't fail. */
    if (access(cfile_name, F_OK)<0) {
        strncpy(cfile_name, SS_INSTALL_SRCDIR "/sslib/lib/sslib.conf", sizeof cfile_name);
        if (access(cfile_name, F_OK)<0) {
#ifndef PRODUCTION_COMPILE
            fprintf(sslib_g.warnings, "SSlib-%d: unable to find sslib.conf file in %s or %s.\n",
                    self, SS_INSTALL_SYSCONFDIR "/include/", cfile_name);
            fprintf(sslib_g.warnings, "SSlib-%d: The libss binary can be edited to fix the search path.\n", self);
#endif
            cfile_name[0] = '\0';
        }
    }
    if (cfile_name[0] && ss_config(communicator, cfile_name)<0) SS_ERROR(FAILED);
    if (ss_debug_env(communicator, NULL)<0) SS_ERROR(FAILED);

#ifdef HAVE_PARALLEL
    /* Create the MPI error handler and set it for this communicator. We save the error handler handle because we'll also use
     * it when setting the error handler for other communicators, such as scope communicators. */
    if (!sslib_g.ignore_mpierror) {
#if 1 /*from MPICH header file*/
        MPI_Errhandler_create(ss_err_mpierror, &(sslib_g.ehandler));
        MPI_Errhandler_set(sslib_g.comm, sslib_g.ehandler);
#else /*from "MPI: The Complete Reference: Vol 1" by Mark Snir et al*/
        MPI_Comm_create_errhandler(ss_err_mpierror, &(sslib_g.ehandler));
        MPI_Comm_set_errhandler(sslib_g.comm, sslib_g.ehandler);
#endif
    }
#else
    sslib_g.comm = SS_COMM_SELF;
#endif /*HAVE_PARALLEL*/

    /* Initialize subsystems -- we can be pretty lazy here because most of them auto initialize properly. The only time they
     * don't get initialized as they should is when a subsystem exports a global variable and that global variable is used by
     * some other subsystem before any function of that subsystem is called. */
    if (ss_mpi_init()<0) SS_ERROR(FAILED);
    if (ss_pers_init()<0) SS_ERROR(FAILED);
    if (ss_attr_init()<0) SS_ERROR(FAILED);

    /* Initialize Library-Wide 2-Phase I/O Defaults and process the SSLIB_2PIO environment variable. */
    if (ss_blob_set_2pio(NULL, NULL)<0) SS_ERROR(FAILED);

    /* Create a transient file on each task */
    if (NULL==(tfile_props=ss_prop_new(SS_PERTASK_FILENAME ".props"))) SS_ERROR(FAILED);
    if (ss_prop_add(tfile_props, "comm", H5T_NATIVE_MPI_COMM, &selfcomm)<0) SS_ERROR(FAILED);
    if (NULL==(sslib_g.temp.file=ss_file_create(SS_PERTASK_FILENAME, H5F_ACC_TRANSIENT, tfile_props))) SS_ERROR(FAILED);
    if (NULL==(sslib_g.temp.tscope=ss_file_topscope(sslib_g.temp.file, NULL))) SS_ERROR(FAILED);
    if (ss_prop_dest(tfile_props)<0) SS_ERROR(FAILED);
    tfile_props = NULL;

    /* Print the welcome banner */
    if (sslib_g.banner && 0==ss_mpi_comm_rank(communicator)) fprintf(stderr, "SSlib-0: %s\n", sslib_g.banner);
                                             
    /* All was successful. Consider the library initialized. Stuff after here uses the SSlib API. */
    sslib_g.initialized = TRUE;

 SS_CLEANUP:
    if (tfile_props) ss_prop_dest(tfile_props);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library
 * Purpose:     Test library initialization state
 *
 * Description: Tests whether the library has been successfully initialized but not yet finalized.
 *
 * Return:      Returns true (positive) if the library has been initialized but not yet finalized and false otherwise. This
 *              function never fails and does not implicitly initialize the library.
 *
 * Parallel:    Independent, although typically called collectively.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, May 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_initialized(void)
{
    SS_ENTER(ss_initialized, htri_t);
    SS_LEAVE(sslib_g.initialized && !sslib_g.finalized);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library
 * Purpose:     Terminate the library
 *
 * Description: A call to this function will flush all pending data to the layers below SSlib, and then release as many
 *              resources as possible. This function is a no-op if called after a previous successful call or before the
 *              library is initialized (explicitly or implicitly). Obviously this function does not implicitly initialize the
 *              library.
 *
 *              Calling this function near the end of execution is strongly encouraged though not strictly necessary if all
 *              files have been explicitly flushed and/or closed.  This function is suitable for registration with atexit()
 *              provided care is taken to ensure that layers below SSlib are not finalized first.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the library's communicator.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 17, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_finalize(void)
{
    SS_ENTER(ss_finalize, herr_t);
    size_t          tableidx;
    ss_pers_class_t *pcls;
    
    if (sslib_g.initialized && !sslib_g.finalized) {
        /* Issue warnings about files that are still open with write access and close all files. */
        ss_file_closeall(sslib_g.warnings);

        /* Shut down asynchronous I/O threads */
        ss_aio_finalize();
        
        /* Free error reporting buffer */
        ss_err_cntl_g.ptr = SS_FREE(ss_err_cntl_g.ptr);
        ss_err_cntl_g.ptrlen = 0;
        
        /* Library has been finalized */
        sslib_g.finalized = TRUE;

#ifdef HAVE_PARALLEL

        /* Free MPI structure for each type of class */
        for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
            pcls = SS_PERS_CLASS(tableidx);
            if (!pcls)continue;
            if (pcls->serialized)MPI_Type_free(&(pcls->serialized));
         }
#endif

    }
    SS_LEAVE(0);
}

/*---------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library
 * Purpose:     Mark library as finalized
 *
 * Description: Sometimes it's necessary to mark the library as finalized but without actually finalizing it. For instance,
 *              a call to MPI_Abort() may eventually call exit() (e.g., from MPID_SHMEM_Abort()) which would cause SSlib's
 *              atexit() handler to be invoked. But we really don't want that because the handler tries to do some MPI
 *              communication. So this function is supplied to make the atexit() handler think ss_finalize() has already
 *              been called.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the library's communicator, although this function does no MPI communication.
 *
 * Issue:       This function is here only because some libraries (e.g., MPICH's MPI_Abort()) incorrectly call exit()
 *              instead of calling _exit().
 *
 * Programmer:  Robb Matzke
 *              Wednesday, September 15, 2004
 *---------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_zap(void)
{
    SS_ENTER(ss_zap, herr_t);
    
    if (sslib_g.initialized && !sslib_g.finalized)
        sslib_g.finalized = TRUE;

#if defined WIN32 || defined JANUS
#else
    /* This function is only called by _saf_error() when it's about to call MPI_Abort(), and the buggy MPICH calls exit()
     * instead of raising a SIGABRT. Therefore if signal debugging is enabled with the SSLIB_DEBUG environment variable we'll
     * start the debugger here before its too late. */
    if (sslib_g.debug_signal) {
        /* ISSUE: We should really set handlers back to their default values now and unmask them */
        int self = ss_mpi_comm_rank(SS_COMM_WORLD);
        char buf[256];
        sprintf(buf, "SSLIB: MPI task %d, PID %d", self, getpid());
        psignal(SIGABRT, buf);
        ss_debug_start(sslib_g.debugger);
        fprintf(stderr, "SSLIB: MPI task %d, PID %d is paused pending debug\n", self, getpid());
        while (true) pause();
    }
#endif /*WIN32*/

    SS_LEAVE(0);
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Library
 * Purpose:     Terminate processes
 *
 * Description: Terminates all MPI processes with a core dump if enabled. This function is called when a major error is
 *              detected by SSlib and there is little or no hope of being able to report that error in a meaningful way. This
 *              can potentially happen when SSlib experiences certain errors in the error reporting interface.
 *
 * Return:      Never returns
 *
 * Parallel:    Independent, although it doesn't really matter because there's no hope of continuing anyway.
 *
 * Programmer:  Robb Matzke
 *              Monday, August  4, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void
ss_abort(void)
{
    /* SS_ENTER -- no can do */
    abort();
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Library
 * Purpose:     Read configuration file
 *
 * Description: When SSlib is initialized it reads a configuration file located at a known location. The file consists of
 *              blank lines, comment lines (any line starting with a '#'), and configuration lines. All configuration lines
 *              are passed to ss_debug_env().
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the library communicator
 *
 * Programmer:  Robb Matzke
 *              Monday, January 10, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_config(MPI_Comm UNUSED_SERIAL comm,          /* The library communicator. */
          const char *filename                  /* The full name of the configuration file. */
          )
{
    SS_ENTER(ss_config, int);
    FILE *cfile=NULL;
    char buf[1024];
    int self, i;
    size_t size;

    if ((self=ss_mpi_comm_rank(comm))<0) SS_ERROR(FAILED);
    if (0==self) cfile = fopen(filename, "r"); /*might fail; handled later*/
    while (1) {
        /* Task zero reads a line from the file */
        if (0==self) {
            if (!cfile) {
                strcpy(buf, "__ERR__");
            } else if (NULL==fgets(buf, sizeof buf, cfile)) {
                strcpy(buf, "__END__");
            }
            if (buf[0] && '\n'==buf[strlen(buf)-1]) buf[strlen(buf)-1] = '\0'; /*chomp line feed*/
            for (i=0; buf[i] && isspace(buf[i]); i++) /*void*/;
            memmove(buf, buf+i, strlen(buf+i)+1);
            if (!buf[0] || '#'==buf[0]) continue;
            size = strlen(buf)+1;
        }

        /* Broadcast the line of text */
        ss_mpi_bcast(&size, 1, MPI_SIZE_T, 0, comm);
        ss_mpi_bcast(buf, size, MPI_BYTE, 0, comm);

        /* All tasks handle the line */
        if (!strcmp(buf, "__ERR__")) {
            if (0==self && !cfile) SS_ERROR_FMT(FAILED, ("fopen(\"%s\",\"r\") failed", filename));
            SS_ERROR_FMT(FAILED, ("configuration file parsing failed"));
        } else if (!strcmp(buf, "__END__")) {
            break;
        } else {
            if (ss_debug_env(comm, buf)<0) SS_ERROR(FAILED);
        }
    }

    /* Successful cleanup */
    if (cfile) {
        fclose(cfile);
        cfile = NULL;
    }

SS_CLEANUP:
    if (cfile) fclose(cfile);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library
 * Purpose:     Start debugger for error
 *
 * Description: This function gets called whenever SSlib pushes an error onto the error stack. If the error's identification
 *              number matches what the user wishes to debug as set with the SSLIB_DEBUG environment variable then the
 *              debugger is started.  This function is also a useful place to set debugger breakpoints.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 11, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_error(void)
{
    /* SS_ENTER -- skipped because this is error handling for the error stack */
    int self;

    if (ss_err_cntl_g.nerrors==sslib_g.debug_error) {
        self = ss_mpi_comm_rank(SS_COMM_WORLD);
        fprintf(stderr, "SSLIB: MPI task %d, PID %d encountered error %lu\n",
                self, getpid(), (unsigned long)(ss_err_cntl_g.nerrors));
        ss_debug_start(sslib_g.debugger);
    }
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library
 * Purpose:     Renders human readable numbers
 *
 * Description: Often when printing a very large decimal number it's not obvious whether that number is some multiple of a
 *              power of 1024. This function breaks down the NBYTES argument into GB, MB, and kB and stores it in a buffer
 *              supplied by the caller or a buffer allocated in this function.  The output format will be something along the
 *              lines of:
 *
 *                4,197,386 (4M+3k+10)
 *
 *              If the NBYTES has all bits set then it will be printed in hexadecimal.  If it is less than 1024 then the
 *              parenthesised part will be omitted.
 *
 *              This function has no provision for limiting the size of the result string. The maximum string length on a
 *              64-bit machine would be 62 bytes counting the NUL terminator:
 *
 *                ##,###,###,###,###,###,### (##,###,###,###G+####M+####k+####)
 *
 * Return:      Returns a pointer to the NUL-terminated string containing the number on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December  3, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
ss_bytes(hsize_t nbytes,
         char *buf              /* Optional buffer to hold the results. If the user supplies the buffer then it should be
                                 * large enough to hold the result. On a 64-bit machine that would be at least 62 bytes. If
                                 * the caller passes the null pointer then one of six static buffers will be used (don't
                                 * make more than six calls to this function in a single printf() argument list). */
         )
{
    SS_ENTER(ss_bytes, charP);
    static char buffers[6][64];
    static int ncalls=0;
    char tmp[32], join[2];
    size_t ng = nbytes >> 30;
    size_t nm = (nbytes >> 20) & 0x3ff;
    size_t nk = (nbytes >> 10) & 0x3ff;
    size_t nb = nbytes & 0x3ff;

    if (!buf)
        buf = buffers[ncalls++ % 4];
    
    /* The decimal part with commas */
    sprintf(tmp, "%llu", (unsigned long_long)nbytes);
    if (NULL==ss_insert_commas(tmp)) SS_ERROR(FAILED);
    strcpy(buf, tmp);

    /* The parenthesised part */
    join[0] = join[1] = '\0';
    if (nbytes>=1024) {
        strcat(buf, " (");
        if (ng) {
            sprintf(tmp, "%lu", (unsigned long)ng);
            if (NULL==ss_insert_commas(tmp)) SS_ERROR(FAILED);
            sprintf(buf+strlen(buf), "%sG", tmp);
            *join = '+';
        }
        if (nm) {
            sprintf(buf+strlen(buf), "%s%luM", join, (unsigned long)nm);
            *join = '+';
        }
        if (nk)
            sprintf(buf+strlen(buf), "%s%luk", join, (unsigned long)nk);
        if (nb)
            sprintf(buf+strlen(buf), "+%lu", (unsigned long)nb);
        strcat(buf, ")");
    }
SS_CLEANUP:
    SS_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library
 * Purpose:     Insert commas into an integer
 *
 * Description: Given a string containing a decimal integer, this function will insert commas between every third digit in
 *              order to make it more human readable.
 *
 * Return:      Returns BUF after having modified it in place. Returns null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December  3, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
ss_insert_commas(char *buf)
{
    SS_ENTER(ss_insert_commas, charP);
    char        sign[2];
    int         i;
    size_t      ndigits;

    /* Peel off the sign character and verify input value */
    sign[0] = sign[1] = '\0';
    if (!buf || !*buf) SS_ERROR_FMT(USAGE, ("empty or null input value"));
    if (!isdigit(*buf)) {
        *sign = *buf;
        memmove(buf, buf+1, strlen(buf));
    }
    if (!*buf || strspn(buf, "0123456789")!=strlen(buf)) SS_ERROR_FMT(USAGE, ("malformed input value: %s", sign, buf));

    /* Insert commas */
    if ((ndigits=strlen(buf))>4) {
        for (i=ndigits-3; i>0; i-=3) {
            memmove(buf+i+1, buf+i, strlen(buf+i)+1);
            buf[i] = ',';
        }
    }

    /* Prepend the sign */
    if (*sign) {
        memmove(buf+1, buf, strlen(buf)+1);
        *buf = *sign;
    }
    
SS_CLEANUP:
    SS_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Library
 * Purpose:     Compute an Adler32 checksum
 *
 * Description: The following source code is borrowed heavily from the zlib project and modified by the SAF development team
 *              so it can be compiled without relying on the entire zlib source code and without conflicting with that code
 *              when that code is linked into the same executable as SSlib.  The zlib source is subject to the following
 *              copyright and license:
 *              
 *                Copyright (C) 1995-2004 Jean-loup Gailly and Mark Adler
 * 
 *                This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be
 *                held liable for any damages arising from the use of this software.
 * 
 *                Permission is granted to anyone to use this software for any purpose, including commercial applications, and
 *                to alter it and redistribute it freely, subject to the following restrictions:
 * 
 *                1. The origin of this software must not be misrepresented; you must not claim that you wrote the original
 *                   software. If you use this software in a product, an acknowledgment in the product documentation would be
 *                   appreciated but is not required. 
 *                2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the
 *                   original software.
 *                3. This notice may not be removed or altered from any source distribution.
 * 
 *                Jean-loup Gailly        Mark Adler
 *                jloup@gzip.org          madler@alumni.caltech.edu
 * 
 *                The data format used by the zlib library is described by RFCs (Request for Comments) 1950 to 1952 in the
 *                files http://www.ietf.org/rfc/rfc1950.txt (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip
 *                format).
 *
 * Return:      Returns the new Adler32 checksum computed by considering LEN bytes starting at BUF and using ADLER as an
 *              initial value. Never fails.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, December 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
unsigned long
ss_adler32(unsigned long adler, const char *buf, size_t len)
{
    unsigned long       s1 = adler & 0xffff;
    unsigned long       s2 = (adler >> 16) & 0xffff;
    size_t              k;
    const unsigned long base = 65521;   /* largest prime smaller than 65536 */
    const size_t        nmax = 5552;    /* largest n such that 255n(n+1)/2 + (n+1)(base-1) <= 2^32-1 */

    if (!buf) return 1L;

    while (len > 0) {
        k = MIN(len, nmax);
        len -= k;

        /* Just the unrolled loop from below */
        while (k >= 16) {
            s1 += buf[ 0]; s2 += s1;
            s1 += buf[ 1]; s2 += s1;
            s1 += buf[ 2]; s2 += s1;
            s1 += buf[ 3]; s2 += s1;
            s1 += buf[ 4]; s2 += s1;
            s1 += buf[ 5]; s2 += s1;
            s1 += buf[ 6]; s2 += s1;
            s1 += buf[ 7]; s2 += s1;
            s1 += buf[ 8]; s2 += s1;
            s1 += buf[ 9]; s2 += s1;
            s1 += buf[10]; s2 += s1;
            s1 += buf[11]; s2 += s1;
            s1 += buf[12]; s2 += s1;
            s1 += buf[13]; s2 += s1;
            s1 += buf[14]; s2 += s1;
            s1 += buf[15]; s2 += s1;
            buf += 16;
            k -= 16;
        }

        /* Take care of the rest */
        while (k--) {
            s1 += *buf++;
            s2 += s1;
        }

        s1 %= base;
        s2 %= base;
    }
    return (s2 << 16) | s1;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Library
 * Purpose:     Checks header/library compatibility
 *
 * Description: When called with the constants defined in ssvers.h this function will compare them with values compiled into
 *              the library and fail with an error message if the versions are not compatible.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, January 10, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_check_version(MPI_Comm UNUSED_SERIAL comm, /*library communicator*/
                 unsigned major, unsigned minor, unsigned rel, const char *annot,
                 const char *incdir)
{
    /* SS_ENTER() -- not used because this can be invoked before ss_init */
    int         self=0;

#ifdef HAVE_PARALLEL
    MPI_Comm_rank(comm, &self);
#endif

    if (major!=SS_VERS_MAJOR || minor!=SS_VERS_MINOR) {
        if (0==self) {
            fprintf(stderr, "SSlib-0: The header files compiled by this client are not compatible with the library.\n");
            
            fprintf(stderr, "         Header files %u.%u.%u", major, minor, rel);
            if (annot && *annot) fprintf(stderr, "-%s", annot);
            if (incdir && *incdir) fprintf(stderr, " possibly from %s", incdir);
            fprintf(stderr, "\n");

            fprintf(stderr, "         Library %u.%u.%u", SS_VERS_MAJOR, SS_VERS_MINOR, SS_VERS_RELEASE);
            if (SS_VERS_ANNOT && *SS_VERS_ANNOT) fprintf(stderr, "-%s", SS_VERS_ANNOT);
            fprintf(stderr, " possibly from %s\n", SS_INSTALL_LIBDIR);
        }
        return -1;
    }
    return 0;
}
