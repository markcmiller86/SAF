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
SS_IF(err);

/* Error class for entire SSlib */
hid_t ss_eclass_g               = -1;

/* Error global variables */
hid_t ss_minor_assert_g         = -1;
hid_t ss_minor_cons_g           = -1;
hid_t ss_minor_corrupt_g        = -1;
hid_t ss_minor_domain_g         = -1;
hid_t ss_minor_exists_g         = -1;
hid_t ss_minor_failed_g         = -1;
hid_t ss_minor_hdf5_g           = -1;
hid_t ss_minor_init_g           = -1;
hid_t ss_minor_mpi_g            = -1;
hid_t ss_minor_notfound_g       = -1;
hid_t ss_minor_notimp_g         = -1;
hid_t ss_minor_notopen_g        = -1;
hid_t ss_minor_overflow_g       = -1;
hid_t ss_minor_perm_g           = -1;
hid_t ss_minor_resource_g       = -1;
hid_t ss_minor_skipped_g        = -1;
hid_t ss_minor_type_g           = -1;
hid_t ss_minor_usage_g          = -1;

/* Error printing control information */
ss_err_cntl_t ss_err_cntl_g = {2, NULL, 0, 0};

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Interface initializer
 *
 * Description: This is the initialization function for the error handling interface.
 *
 * Return:      Returns non-negative on success, negative on failure.
 *
 * Parallel:    Indepedent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, July 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_err_init(void)
{
    static char         version_string[128];
    static char         classname[128];
    
    /* This must come before SS_ENTER_INIT because that macro uses ss_eclass_g */
    sprintf(version_string, "%d.%d.%d%s%s", SS_VERS_MAJOR, SS_VERS_MINOR, SS_VERS_RELEASE,
            SS_VERS_ANNOT && SS_VERS_ANNOT[0]?"-":"", SS_VERS_ANNOT?SS_VERS_ANNOT:"");
    sprintf(classname, "SAF-%s", version_string);
    if (ss_eclass_g<=0) ss_eclass_g = H5Eregister_class(classname, "SSlib", version_string);

    SS_ENTER_INIT;
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Initialize and return minor error object
 *
 * Description: Minor error numbers are initialized at runtime but are used by nearly every SSlib interface. This function is
 *              intended to both initialize the minor error numbers and return the error object requested, and is normally
 *              invoked through a macro:
 *
 *                  #define SS_MINOR_INIT ss_err_init1(&ss_minor_init_g,"not initialized")
 *
 * Return:      Returns the minor error number that was requested after making sure it was initialized.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 17, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
ss_err_init1(hid_t *to_return,          /* Pointer to the (global) variable that holds the error message handle. */
             const char *mesg           /* The error string to assign to this minor error handle. */
             )
{
    SS_ENTER(ss_err_init1, hid_t);

    if (*to_return<0) {
        if ((*to_return = H5Ecreate_msg(ss_eclass_g, H5E_MINOR, mesg))<0) {
            fprintf(stderr, "SSlib (SAF) major problems during initialization for minor error `%s'. Aborting...", mesg);
            H5Eprint1(stderr);
            ss_abort();
        }
    }
    
    SS_LEAVE(*to_return);
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Error string formatter
 *
 * Description: Due to the way the SS_ERROR_FMT() macro works, this function is necessary in order to change the variable
 *              length arguments of that macro into a single string to be passed to H5Epush().  Since SS_ERROR_FMT() is the
 *              only place from which this function is called, we can guarantee that it's safe to just vsnprintf() the string
 *              into a static array.
 *
 * Return:      Returns a pointer to a static array that holds the result.
 *
 * Parallel:    Independent
 *
 * Issue:       The vsnprintf() function is not portable, so we use the more dangerous sprintf() instead.  This should be
 *              fixed once C99 is available in more places. --rpm 2003-07-29
 *
 * Programmer:  Robb Matzke
 *              Tuesday, July 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
const char *
ss_err_fmt(const char *fmt, ...)
{
    va_list             ap;
#ifdef HAVE_VSNPRINTF
    static char         *buffer=NULL;
    static size_t       buf_nalloc=0;
    size_t              nchars;

    va_start(ap, fmt);
    nchars = vsnprintf(buffer, buf_nalloc, fmt, ap);
    va_end(ap);
    
    if (nchars >= buf_nalloc) {
        buf_nalloc = nchars+1;
        buffer = realloc(buffer, buf_nalloc);
        assert(buffer);
        va_start(ap, fmt);
        vsnprintf(buffer, buf_nalloc, fmt, ap);
        va_end(ap);
    }
#else
#define MYBUFSIZ 2048
    static char buffer[MYBUFSIZ];
    size_t nchars;
    
    va_start(ap, fmt);
    nchars = vsprintf(buffer, fmt, ap);
    va_end(ap);
    if (nchars > MYBUFSIZ) {
      fprintf(stderr, "SSlib (SAF) Internal error in ss_err_fmt; memory may be corrupted.\n");
      ss_abort();
    }
    buffer[nchars] = '\0';
#endif
    return buffer;
}

#ifdef HAVE_PARALLEL
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Callback helper for MPI errors
 *
 * Description: This function gets invoked for any MPI errors associated with a communicator being used by SSlib. Since SSlib
 *              requires that all functions return a value but MPI requires that an error handler return void, this function
 *              is invoked by the MPI error handler ss_err_mpierror() which in turn returns void.
 *
 *              It's usually pointless to resume MPI operations after an error is detected because the standard allows the MPI
 *              library to enter into an unknown state after an error. However, most SSlib users would like to be able to
 *              obtain a stack trace for MPI-related errors, and doing so requires that we return control all the way back up
 *              to the top of the SSlib call stack.  However, we'll set sslib_g.had_mpierror so SSlib aborts after reporting
 *              the error.
 *
 * Return:      Always fails, returning negative.
 *
 * Parallel:    Invoked by the MPI error handler.  Do not make MPI calls after this returns!
 *
 * Programmer:  Robb Matzke
 *              Monday, August 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
static herr_t
ss_err_mpierror1(MPI_Comm UNUSED *comm, int ecode)
{
    SS_ENTER(ss_err_mpierror1, herr_t);
    static char eclass_str[MPI_MAX_ERROR_STRING];
    static char error_str[MPI_MAX_ERROR_STRING];
    int         eclass_len, error_len;
    int         eclass;
    
    MPI_Error_string(ecode, error_str, &error_len);
    MPI_Error_class(ecode, &eclass);
    if (ecode == eclass) {
        SS_ERROR_FMT(MPI, ("%s", error_str));
    } else {
        MPI_Error_string(eclass, eclass_str, &eclass_len);
        SS_ERROR_FMT(MPI, ("%s: %s", eclass_str, error_str));
    }
    
 SS_CLEANUP:
    SS_LEAVE(-1);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Callback for MPI errors
 *
 * Description: This is the callback function for MPI errors for all communicators used by SSlib. It's mainly here so that we
 *              have a convenient place to set a breakpoint while debugging (set it here for MPI-related errors that would
 *              normally cause the job to terminate, and set one in H5Epush() for errors detected and handled by SSlib.
 *
 * Parallel:    Inovked in response to an error detected inside the MPI library.
 *
 * Programmer:  Robb Matzke
 *              Monday, August 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void
ss_err_mpierror(MPI_Comm *comm, int *ecode, ...)
{
    /* SS_ENTER() doesn't work with functions that return void, but MPI requires error handlers to return void. Therefore,
     * this function serves as the error handler for MPI but it just invokes ss_err_mpierror1() that abides by the SSlib way
     * of writing functions. */
    ss_err_mpierror1(comm, *ecode);
}
#endif /*HAVE_PARALLEL*/

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Examine error stack
 *
 * Description: This function obtains information about a specific entry on an error stack. The POSITION should be an integer
 *              where zero is the first entry pushed onto the stack, one is the second, etc., and -1 is the last entry, -2 is
 *              the second to last, etc.
 *
 * Return:      Returns a pointer directly into the error stack on success; the null pointer on failure. The pointer is only
 *              guaranteed to be valid until the stack is modified in some way.
 *
 * Issue:       It doesn't work to call this function with H5E_DEFAULT as the error stack since (1) additional HDF5 functions
 *              will be called that will probably clear the default stack at some point, and (2) it is awkward to recover from
 *              an error while examining the error stack used during recovery.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, August 26, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
const H5E_error2_t *
ss_err_info(hid_t estack, int position)
{
    SS_ENTER(ss_err_info, H5E_error2_tP);
    int         stksize;
    ss_err_info_t walker;

    if ((stksize=H5Eget_num(estack))<0) SS_ERROR(HDF5);
    if (position<0) position = stksize + position;
    if (position<0 || position>=stksize) SS_ERROR(DOMAIN);

    memset(&walker, 0, sizeof walker);
    walker.position = position;
    if (H5Ewalk2(estack, H5E_WALK_UPWARD, ss_err_info_cb, &walker)<0) SS_ERROR(HDF5);
    if (!walker.info) SS_ERROR(NOTFOUND);
    
 SS_CLEANUP:
    SS_LEAVE(walker.info);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Walk error stack and save entry
 *
 * Description: This is a callback function for H5Ewalk() that scans an error stack for the requested stack position and
 *              caches the error information for that position in _WALKER, which is actually of type ss_err_info_t.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, August 26, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_err_info_cb(unsigned n, const H5E_error2_t *info, void *_walker)
{
    SS_ENTER(ss_err_info_cb, herr_t);
    ss_err_info_t *walker = (ss_err_info_t*)_walker;

    if (n==walker->position) walker->info = info;
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Print error stack
 *
 * Description: Prints the specified error stack. See ss_err_print_cb() for details.  This is the handler that is installed by
 *              SSlib by default, and therefore must be capable of handling errors in both SSlib and when the client directly
 *              calls HDF5.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, August 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_err_print(hid_t estack, void *_econtrol)
{
    hid_t               tmpstack = H5Eget_current_stack();
    SS_ENTER(ss_err_print, herr_t);
    ss_err_cntl_t       *econtrol=(ss_err_cntl_t*)_econtrol;
    char                buf[64];
    int                 self;

    if (estack==H5E_DEFAULT) {
        /* We've been called in reponse to an automatic error report from outside SSlib. For instance, the client may have
         * made a direct call to an HDF5 function that failed. We should fall back to the default HDF5 error reporting. */
        H5Eprint2(tmpstack, NULL);
        H5Eclose_stack(tmpstack);
    } else {
        /* This stack is being printed in response to a problem within SSlib. */
        if ((self=ss_mpi_comm_rank(SS_COMM_WORLD))<0) SS_ERROR(FAILED);

        /* We buffer up the whole error message and then issue it with one write() call. This method has less probability that the
         * error messages from different tasks will be interleaved than if we issued little bitty writes. */
        if (econtrol->fd>=0) {
            if (econtrol->ptr) econtrol->ptr[0] = '\0';
            if (H5Ewalk2(estack, H5E_WALK_DOWNWARD, ss_err_print_cb, _econtrol)<0) SS_ERROR(HDF5);
            sprintf(buf, "SAF: Error detected in MPI task %d.\n", self);
            write(econtrol->fd, buf, strlen(buf));
            if (econtrol->ptr) write(econtrol->fd, econtrol->ptr, strlen(econtrol->ptr));
        }
    }
    
SS_CLEANUP:
    /* If an error occurred above then we probably can't (and therefore shouldn't) print that error stack. So we'll
     * just print some message and abort. */
    fprintf(stderr, "SSlib: problems printing error stack. Aborting...\n");
    ss_abort();

    SS_LEAVE(0);
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Print error stack
 *
 * Description: The default HDF5 error stack printing is horrendously verbose.  This function attempts to remedy that by
 *              condensing some information about the error.  Here's an example error message printed by the default error
 *              handler, H5E_walk_cb():
 *
 *                  SSlib-DIAG: Error detected in 0.1 MPI-process 0.
 *                    #000: /home/matzke/saf-devel/src/sslib/lib/ssfile.c line 284 in ss_file_open(): unable to open file: x.saf
 *                      class: SSlib
 *                      major: file
 *                      minor: HDF5 call failed
 *                  HDF5-DIAG: Error detected in  1.7.1 MPI-process 0.
 *                    #001: /home/matzke/hdf5-devel/src/H5F.c line 2413 in H5Fopen(): unable to open file
 *                      class: HDF5
 *                      major: File interface
 *                      minor: Unable to open file
 *
 *              And here's what this function would display instead
 *
 *                  SSlib: Error detected in MPI task 0.
 *                    #000: unable to open file: x.saf
 *                      SSlib-0.0.1::file::HDF5 call failed
 *                      In ss_file_open() at /home/matzke/saf-devel/src/sslib/lib/ssfile.c line 284
 *                    #001: unable to open file
 *                      HDF5-1.7.2::File interface::Unable to open file
 *                      In H5Fopen() at /home/matzke/hdf5-devel/src/H5F.c line 2413
 *
 *              The things that change are:
 *
 *              * The header line is only printed once per error stack instead of once per library. The reasoning is that
 *                the user is probably interested in the fact that an error was detected in his call SSlib but not interested
 *                that SSlib is blaming HDF5 for the error. And if he really is interested he can look at the error class,
 *                which by convention will be information about the library.
 *
 *              * The library version number has been moved from the header line to the error class because it's often useful
 *                to know the version number (e.g., to make sure things were linked as expected) but we now have only one
 *                header line per stack trace.
 *
 *              * The header line uses the term "task" instead of "process" since this is the conventional MPI nomenclature.
 *                An MPI task may consist of multiple operating system processes and/or threads which are scheduled on a
 *                processing node that has one or more processors.  The MPI information is omitted for the serial version of
 *                the library.
 *
 *              * The error text is moved to the beginning of the first line of output for each stack entry since the user
 *                is probably much more interested in the text of the error message than the source location where it was
 *                detected. The source location was moved to the second line.
 *
 *              * The source location is described first by function, then by file and line number since function name is
 *                probably more meaningful to a casual user.
 *
 *              * The class, major, and minor information was combined into one line since those fields are usually very
 *                short. Double colons were used as separators since many object oriented languages use them to qualify
 *                symbolic names.
 *
 *              It's also possible to get XEmacs to parse the SSlib error stack so that with the click of a mouse button
 *              you can be taken directly to the line in the file where the SS_ERROR() macro appears. Evaluate the following
 *              expressions to get this to work (i.e., put them in FILE:~/.xemacs/init.el):
 *
 *                  (load "compile")
 *                  (setq compilation-error-regexp-alist-alist ; run-time errors from SAF/SSlib
 *                    (append compilation-error-regexp-alist-alist
 *                      '((sslib ("    In [a-z_A-Z][a-z_A-Z0-9]*() at \\([a-zA-Z]?:?[^:( \t\n]+\\) line \\([0-9]+\\)$" 1 2)))))
 *                  (compilation-build-compilation-error-regexp-alist)
 * 
 * Return:      Returns non-negative on success; negative on failure
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, August 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_err_print_cb(unsigned n, const H5E_error2_t *einfo, void *_econtrol)
{
    SS_ENTER(ss_err_print_cb, herr_t);
    ss_err_cntl_t       *econtrol = (ss_err_cntl_t*)_econtrol;
    char                class_name[256], minor[256], major[256];
    char                temp[256], *rest=NULL;
    const char          *desc=NULL;
    size_t              curlen, linelen;
    size_t              error_id=0;
    unsigned            h5maj=0, h5min=0, h5patch=0;

    assert(einfo);
    assert(econtrol);

    /* Current length of error string */
    curlen = econtrol->ptr ? strlen(econtrol->ptr) : 0;

    /* Get strings */
    if (H5Eget_class_name(einfo->cls_id, class_name, sizeof(class_name))<0) SS_ERROR(HDF5);
    if (H5Eget_msg(einfo->maj_num, NULL, major, sizeof major)<0) SS_ERROR(HDF5);
    if (H5Eget_msg(einfo->min_num, NULL, minor, sizeof minor)<0) SS_ERROR(HDF5);

    /* If the class name is "HDF5" then append the version number from the library */
    if (!strcmp(class_name, "HDF5") && H5get_libversion(&h5maj, &h5min, &h5patch)>=0) {
        sprintf(class_name, "HDF5-%u.%u.%u", h5maj, h5min, h5patch);
        if (H5_VERS_SUBRELEASE[0]) strcat(class_name, H5_VERS_SUBRELEASE); /*no way to get this from the library; use headers*/
    }
    
    /* Get the error description. It may begin with an integer error identification number followed by a colon. */
    if (einfo->desc && einfo->desc[0]) {
        if ((error_id=(size_t)strtol(einfo->desc, &rest, 0))>0 && ':'==*rest) {
            rest++; /*skip over the `:'*/
            if (!*rest) rest = NULL;
            desc = rest;
        } else {
            error_id = 0;
            desc = einfo->desc;
        }
    }

    /* Line 1: level, minor and description (if given and different than minor) */
    sprintf(temp, "  #%02u: ", n);
    linelen = strlen(temp) + strlen(minor) + strlen(": ") + (desc?strlen(desc):0) + strlen("\n");
    SS_EXTEND(econtrol->ptr, curlen+linelen+2, econtrol->ptrlen);
    sprintf(econtrol->ptr+curlen, "%s%s", temp, minor);
    curlen += strlen(econtrol->ptr+curlen);
    if (desc && strcmp(minor, desc)) {
        sprintf(econtrol->ptr+curlen, ": %s", desc);
        curlen += strlen(econtrol->ptr+curlen);
    }
    strcpy(econtrol->ptr+curlen, "\n");
    curlen += strlen(econtrol->ptr+curlen);

    /* Line 2: class :: major and error ID number */
    if (sslib_g.show_error_ids && error_id>0) {
        sprintf(temp, " (error=%lu)", (unsigned long)error_id);
    } else {
        *temp = '\0';
    }
    linelen = 4 + strlen(class_name) + 2 + strlen(major) + strlen(temp) + 1;
    SS_EXTEND(econtrol->ptr, curlen+linelen+2, econtrol->ptrlen);
    sprintf(econtrol->ptr+curlen, "    %s::%s%s\n", class_name, major, temp);
    curlen += strlen(econtrol->ptr+curlen);

    /* Line 3: location */
    sprintf(temp, "%u", einfo->line);
    linelen = 7 + strlen(einfo->func_name) + 6 + strlen(einfo->file_name) + 6 + strlen(temp) + 1;
    SS_EXTEND(econtrol->ptr, curlen+linelen+2, econtrol->ptrlen);
    sprintf(econtrol->ptr+curlen, "    In %s() at %s line %s\n", einfo->func_name, einfo->file_name, temp);

 SS_CLEANUP:
    /* If an error occurred above then we probably can't (and therefore shouldn't) print that error stack. So we'll
     * just print some message and abort. */
    fprintf(stderr, "SSlib: problems printing error stack. Aborting...\n");
    ss_abort();

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Error handler test code
 *
 * Description: This is simply a test case for the error handling code. It calls itself recursively, decrementing DEPTH and
 *              failing when DEPTH reaches zero.
 *
 * Return:      Always returns negative to indicate failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, June 26, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_err_check(int depth)
{
    SS_ENTER(ss_err_check, herr_t);
    if (depth>0) {
        ss_err_check(depth-1);
        SS_ERROR(FAILED);
    } else {
        SS_ERROR_FMT(FAILED, ("this is a test"));
    }
SS_CLEANUP:
    SS_LEAVE(0);
}
