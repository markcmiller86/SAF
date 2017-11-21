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
SS_IF(debug);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Debugging
 * Purpose:     Interface initializer
 *
 * Description: Initializes the debug interface.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, April 18, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_debug_init(void)
{
    SS_ENTER_INIT;
    /* nothing to do */
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Debugging
 * Purpose:     Enter an interactive debugging loop
 *
 * Description: Read and execute debugging commands if the `commands' word was present in the SSLIB_DEBUG environment
 *              variable. The file containing the commands is also specified in SSLIB_DEBUG. See ss_debug_env() for complete
 *              documentation for that variable. If `commands' is not specified in SSLIB_DEBUG for the calling task or if the
 *              command input file is empty then this function is a no-op.
 *
 *              The commands accepted by this function are defined in ss_debug_s(). In addition, the command `detach' causes
 *              this function to immediately return. The ss_debug() function may be called more than once in any given
 *              executable.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, April 18, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_debug(void)
{
    SS_ENTER(ss_debug, herr_t);
    char        line[1024], c[1];
    int         i, self;

    if (sslib_g.command_fd<0) goto done;
    self = ss_mpi_comm_rank(SS_COMM_WORLD);

    while (1) {
        /* Prompt */
        if (isatty(sslib_g.command_fd))
            fprintf(stderr, "SSlib-%d> ", self);
        
        /* Read a line of input */
        for (i=0, line[0]='\0'; (size_t)i+1<sizeof line; i++) {
            ssize_t nread = read(sslib_g.command_fd, c, 1);
            if (0==nread) {
                if (0==i) strcpy(line, "detach");
                break;
            }
            if (nread<0) SS_ERROR_FMT(CORRUPT, ("read failed from fd %d: %s", sslib_g.command_fd, strerror(errno)));
            if ('\n'==*c) break;
            line[i] = *c;
            line[i+1] = '\0';
        }
        if (!strcmp(line, "detach")) break;

        /* Process the command, but if an error occurs print it and continue as normal */
        if (ss_debug_s(line)<0) SS_ERROR_NOW(FAILED, (""));
    }

done:
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Debugging
 * Purpose:     Parse debug setup statements
 *
 * Description: This function looks at the contents of the SSLIB_DEBUG environment variable. It is a semicolon separated list
 *              of terms which control various things. Valid terms are:
 *
 *              * task=/n/:     Controls which tasks will be affected by subsequent debugging terms. A value of /n/ with an
 *                              initial plus sign will add task /n/ to the list of selected tasks; a leading minus sign removes
 *                              the task from the list; lack of a plus or minus makes task /n/ the only selected task. The
 *                              value !all or !none can also be supplied which selects all tasks or no tasks, respectively.
 *                              The value can also be a comma-separated list of task ranks which acts the same as if multiple
 *                              !task terms had been specified (the plus or minus sign should be at the beginning of the list
 *                              and applies to all values of the list).
 *
 *              * error=/n/:    When errors are pushed onto the error stack they are each given a unique (within a task)
 *                              identification number. When this !error debugging term is specified then the debugger of choice
 *                              is invoked when error number /n/ is pushed onto the stack.  If the equal sign and error number
 *                              are omitted then the debugger is never started for an error, but the error stack will display
 *                              the error identification numbers.  Only one error number can be specified per task--if more
 *                              flexibility is needed then the application can be run under a debugger with a breakpoing set
 *                              in ss_error().
 *
 *              * file=/name/:  Selects the output file to use for subsequent debugging terms for the selected tasks. The
 *                              file will be created if it doesn't exist or truncated if it does exist. The name is actually
 *                              a printf() format string and the first format specifier (if present) should be for an integer
 *                              task number.  If /name/ is the word !none then output is disabled; if /name/ is a positive
 *                              integer then the specified file descriptor is used without attempting to open it (this is
 *                              useful if the descriptor was opened with the shell).  If a number and name are both specified
 *                              separated by a comma then the name is opened and dup'd to the desired file descriptor. If the
 *                              name begins with a `<' character then the file is opened for read-only.
 *
 *              * stop:         The specified MPI task(s) will print their MPI rank and process ID and then suspend themselves,
 *                              giving an opportunity for a debugger to attach.
 *
 *              * pause=N:      The specified MPI task(s) will immediately pause for N seconds. This is useful
 *                              when a task needs to give a debugger (such as strace) to automatically attach to child
 *                              processes.
 *
 *              * debugger=/name/: Specifies which debugger should be used. The default is `ddd'.
 *
 *              * debug:        The specified debugger (or ddd) is started for the affected task or tasks.
 *                              This probably only works on systems that have a FILE:/proc/self/exe link to the executable and
 *                              the DISPLAY environment variable set properly for the affected task.  If the non-default
 *                              debugger is desired then the `debugger' keyword must appear before this `debug' keyword.
 *
 *              * signal:       Start the debugger when a task is about to die from certain signals (those that signify
 *                              a program error). The task is suspended (although other signal handlers might still be
 *                              executed) and must be explicitly killed. The `debug' keyword takes precedence over `signal'.
 *
 *              * stack:        Turn automatic error reporting on or off for selected tasks depending on the current setting
 *                              for the file descriptor. When off, errors are reported by return values as usual and the error
 *                              stack contains information about the error, but the stack is not automatically printed.
 *                              The default is that errors are printed to stderr.
 *
 *              * pid:          Print the process ID for all selected tasks. This is useful when various tools (such as
 *                              valgrind) print PIDs but have no way of knowing the MPI task number.
 *
 *              * mpi:          Do not register an MPI error handler in the ss_init() call.
 *
 *              * banner=STR:   Display the specified string value on stderr when ss_init() is about to return. This is
 *                              normally used in conjuction with the config file to notify users that they should recompile
 *                              their application with a newer version of sslib.
 *
 *              * commands:     Enables the ss_debug() calls that might appear in applications. The `file' term should be used
 *                              before this term in order to specify from where the debug commands should be read (don't
 *                              forget to use the `<' in front of the file name in order to open it for read-only). If no file
 *                              is specified then SSlib attempts to read the commands from the stderr stream, which may cause
 *                              the commands to be read from the controlling terminal in certain situations (but it's usually
 *                              better to be explicit by providing the `file=</dev/tty' term). Specifying an empty file such
 *                              as /dev/null has essentially the same effect as if the `commands' term was not given.
 *
 *              * warnings:     For the selected MPI tasks, send all miscellaneous SSlib warning messages to the selected
 *                              file.
 *
 *              * check=/what/: Turns on or off various categories of internal consistency checking, some of which incur
 *                              considerable runtime expense.  The /what/ is a comma-separated list of category names where
 *                              that category of checking is turned off if introduced with a minus sign and on otherwise. Only
 *                              selected tasks are affected. See the table below for a list of categories.
 *
 *              The following internal consistency checking categories are defined. Some categories can take a comma-separated
 *              list of attributes separated from the category name by an equal sign. When a category is followed by an equal
 *              sign then it must be the last category listed for that !check term, but additional categories can be specified
 *              with additional !check terms.
 *
 *              * sync:         When turned on, SSlib will check for many situations where a call to ss_pers_modified() (or
 *                              the macro SS_PERS_MODIFIED()) was accidently omitted by computing and caching checksums. If
 *                              the !error attribute is specified then such situations will be considered errors instead of
 *                              just generating debugging information on the warning stream. If the !bcast attribute is
 *                              specified then information about which objects are transmitted will be displayed to the
 *                              warning stream.
 *
 *              * 2pio:         SSlib will display certain information about 2-phase I/O if this is turned on. For instance,
 *                              when aggregation tasks are chosen for a blob the mapping from dataset addresses to aggregators
 *                              is displayed. The !task setting doesn't affect this flag since it's always task zero that
 *                              displays this collective information.
 *
 * Example:     Example 1: To start the DDD debugger on task 17:
 *
 *                  SSLIB_DEBUG='task=17;debug' ...
 *
 *              Example 2: To stop all tasks but task 17:
 *
 *                  SSLIB_DEBUG='task=-17;stop' ...
 *
 *              Example 3: To cause task 17 to report errors to a file named "task17.err" and no other task to report errors:
 *
 *                  SSLIB_DEBUG='file=none;stack;task=17;file=task17.err;stack' ...
 *
 *              Example 4: Cause HDF5 to emit tracing information to files like FILE:task001.trace, FILE:task002.trace, etc.
 *              The thing to watch out for here is that HDF5 gets initialized before SSlib and if file descriptor 99 is not
 *              open then tracing is disabled. So we rely on the shell to supply an initial file for descriptor 99 which SSlib
 *              will swap out from under HDF5. Until the swap occurs, all tasks will emit tracing to the shell-supplied file:
 *
 *                  SSLIB_DEBUG="file=99,task%03d.trace" HDF5_DEBUG=99,trace 99>tasks.trace ...
 *
 *              Example 5: Invoke a debugger on any task that fails an assertion or receives certain other normally fatal
 *              signals.  Use !gdb instead of the default !ddd.
 *
 *                  SSLIB_DEBUG='debugger=gdb;signal' ...
 *
 *              Example 6: To cause each task to redirect its standard error output to its own file:
 *
 *                  SSLIB_DEBUG='file=2,stderr.%04d' ...
 *
 *              Example 7: To type commands interactively to SSlib one makes a call to ss_debug() in the application
 *              and then uses SSLIB_DEBUG as follows:
 *
 *                  SSLIB_DEBUG='task=0;file=<commands.txt;commands' ...
 *
 *              Example 8: To turn off the warning/debug messages that are normally emitted from SSlib on the stderr stream
 *              one would do the following:
 *
 *                  SSLIB_DEBUG='file=/dev/null;warnings' ...
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the library communicator. We do this because environment variables are sometimes only
 *              available at certain tasks (task zero of the library communicator must have the environment variable).
 *
 * Programmer:  Robb Matzke
 *              Friday, September 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_debug_env(MPI_Comm UNUSED_SERIAL comm,       /* The library communicator. Pass any integer value when using a version of
                                                 * SSlib compiled without MPI support. */
         const char *s_const                    /* Optional string to use instead of looking at the SSLIB_DEBUG environment
                                                 * variable. Pass null to use SSLIB_DEBUG instead. Passing an empty string
                                                 * (or all white space) accomplishes nothing. Task zero broadcasts this string
                                                 * to all the other tasks. */
         )
{
    SS_ENTER(ss_debug_env, herr_t);
    const char  *s_hdf5=NULL;                   /* value of the HDF5_DEBUG environment variable */
    int         s_len[2];                       /* length of SSLIB_DEBUG and HDF5_DEBUG environment variable values */
    char        *s=NULL, *s_here=NULL;          /* malloc'd version of SSLIB_DEBUG's value; `s' is for strtok() 1st arg */
    char        *rest=NULL;                     /* ptr into `s' for first char after parsing a number with strtol() */
    int         fd=2;                           /* file number for output, defaults to stderr */
    int         self, ntasks;                   /* MPI rank and size of COMM */
    char        buf1[1024], buf2[1024], buf3[1024];/* temporary buffers */
    char        *filename=NULL;                 /* Name of file to be opened */
    hbool_t     *task_enabled=NULL;             /* array specifying which tasks are affected by the following terms */
    hbool_t     debugger_started=FALSE;         /* has the `debugger' keyword been processed already? */
    long        nl;                             /* return value from strtol() */
    size_t      nchars;
    int         i, n, sign, nterms;
    const char  *source=NULL;

    /* Initializations */
    if ((self=ss_mpi_comm_rank(comm))<0) SS_ERROR(FAILED);
    if ((ntasks=ss_mpi_comm_size(comm))<0) SS_ERROR(FAILED);
    if (NULL==(task_enabled=malloc(ntasks*sizeof(*task_enabled)))) SS_ERROR(RESOURCE);
    for (i=0; i<ntasks; i++) task_enabled[i] = TRUE;
    
    /* Task zero gets the values of SSLIB_DEBUG and HDF5_DEBUG and broadcasts their lengths to the other tasks. We use
     * `int' for the string lengths because the MPI_SIZE_T type isn't initialized by ss_mpi_init() until after we've
     * parsed the SSLIB_DEBUG environment variable. */
    if (0==self) {
        if (s_const) {
            source = "supplied string";
        } else {
            source = "$SSLIB_DEBUG";
            s_const = getenv("SSLIB_DEBUG");
        }
        s_len[0] = s_const ? (int)strlen(s_const)+1 : 0;
        s_hdf5 = getenv("HDF5_DEBUG");
        s_len[1] = s_hdf5 ? (int)strlen(s_hdf5)+1 : 0;
    }
    if (ss_mpi_bcast(&s_len, 2, MPI_INT, 0, comm)<0) SS_ERROR(FAILED);

    if (s_len[0]>0) {
        /* Broadcast the SSLIB_ERROR value */
        if (NULL==(s=s_here=malloc((size_t)MAX(s_len[0], s_len[1])))) SS_ERROR(RESOURCE);
        if (0==self) strcpy(s, s_const);
        if (ss_mpi_bcast(s, (size_t)(s_len[0]), MPI_CHAR, 0, comm)<0) SS_ERROR(FAILED);

        /* Parse the SSLIB_ERROR value */
        while (s && *s) {
            if (s[0]==';') {
                s++;
            } else if (!strncmp(s, "task=", 5)) {
                /* Select certain tasks */
                s += 5;
                sign = 0;
                nterms = 0;

                while (*s && ';'!=*s) {

                    /* Get the leading `+' or `-' sign. It applies to all subsequent task numbers in the list */
                    while (*s && isspace(*s)) s++;
                    if ('-'==*s) {
                        sign = -1;
                        s++;
                    } else if ('+'==*s) {
                        sign = 1;
                        s++;
                    }
                    
                    while (*s && isspace(*s)) s++;
                    if (!strncmp(s, "all", 3)) {
                        for (i=0; i<ntasks; i++) task_enabled[i] = TRUE;
                        s += 3;
                    } else if (!strncmp(s, "none", 4)) {
                        for (i=0; i<ntasks; i++) task_enabled[i] = FALSE;
                        s += 4;
                    } else if ((nl=strtol(s, &rest, 0)) || rest!=s) {
                        if (nl<0 || nl>=ntasks) SS_ERROR_FMT(DOMAIN, ("task rank %ld is not valid in %s", nl, source));
                        if (sign>0) {
                            task_enabled[nl] = TRUE;
                        } else if (sign<0) {
                            task_enabled[nl] = FALSE;
                        } else {
                            if (0==nterms) {
                                for (i=0; i<ntasks; i++) task_enabled[i] = FALSE;
                            }
                            task_enabled[nl] = TRUE;
                        }
                        s = rest;
                    } else {
                        SS_ERROR_FMT(USAGE, ("malformed task specification in %s at: %s", source, s));
                    }
                    if (','==*s) s++;
                    nterms++;
                }
                
            } else if (!strncmp(s, "error=", 6)) {
                if ((0==(nl=strtol(s+6, &rest, 0)) && rest==s) || (*rest!=';' && *rest!='\0'))
                    SS_ERROR_FMT(USAGE, ("malformed error number in %s at: %s", source, s));
                if (task_enabled[self]) {
                    sslib_g.show_error_ids = TRUE;
                    sslib_g.debug_error = nl;
                }
                s = rest;
                
            } else if (!strncmp(s, "error", 5) && (';'==s[5] || !s[5])) {
                if (task_enabled[self]) sslib_g.show_error_ids = TRUE;
                s += 5;

            } else if (!strncmp(s, "file=", 5)) {
                /* Select file descriptor and/or open a file */
                s += 5;

                /* Look for a leading integer optionally followed by a comma */
                if ((0==(nl=strtol(s, &rest, 0)) && rest==s) ||                 /* if no leading integer, or ... */
                    (*rest!=',' && *rest!=';' && *rest!='\0')) {                /* something other than ',' or ';' follows... */
                    n = -1;                                                     /* then there is no leading file descriptor. */
                } else {
                    SS_ASSERT(nl<=INT_MAX);
                    n = (int)nl;
                    s = ','==*rest ? rest+1 : rest;                             /* otherwise skip the descriptor and ',' */
                }

                /* Look for a file name. It is an error if there's no name and the destination fd is negative */
                if ((!s || !*s || ';'==*s) && n<0) {
                    SS_ERROR_FMT(USAGE, ("malformed file term in %s", source));
                } else if (!strncmp(s, "none", 4) && (';'==s[4] || !s[4])) {
                    if (task_enabled[self]) {
                        s += 4;
                        if (n>=0) SS_ERROR_FMT(USAGE, ("file descriptor and `none' both specified in %s", source));
                        fd = -1;
                    }
                } else if (*s && ';'!=*s) {
                    if ((rest=strchr(s, ';'))) {
                        SS_ASSERT((size_t)(rest-s) < sizeof buf1);
                        strncpy(buf1, s, (size_t)(rest-s));
                        buf1[rest-s] = '\0';
                        sprintf(buf2, buf1, self);
                        sprintf(buf3, buf1, -1); /*for comparison later*/
                        s = rest;
                    } else {
                        sprintf(buf2, s, self);
                        sprintf(buf3, s, -1); /*for comparison later*/
                        s = NULL;
                    }
                    if ('<'==buf2[0]) {
                        /* The file is being opened for reading. */
                        if (task_enabled[self]) {
                            filename = buf2+1;
                            fd = open(filename, O_RDONLY);
                            if (fd<0) SS_ERROR_FMT(FAILED, ("cannot open file `%s': %s", filename, strerror(errno)));
                        }
                    } else if (!strcmp(buf2, buf3)) {
                        /* All tasks are opening the same file. The lowest numbered task will create or truncate the file and
                         * then everyone else will open it for appending. This prevents tasks from clobbering each other's
                         * output. */
                        for (i=0; i<ntasks; i++) if (task_enabled[i]) break;
                        if (i==self) {
                            /* The first task should create/truncate the file */
                            fd = open(buf2, O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0666);
                            if (fd<0) SS_ERROR_FMT(FAILED, ("cannot create file `%s': %s", buf2, strerror(errno)));
                        }
                        ss_mpi_barrier(comm);
                        if (task_enabled[self] && i!=self) {
                            /* All other tasks just open the file */
                            fd = open(buf2, O_RDWR|O_APPEND, 0666);
                            if (fd<0) SS_ERROR_FMT(FAILED, ("cannot open file `%s': %s", buf2, strerror(errno)));
                        }
                    } else if (task_enabled[self]) {
                        /* All tasks are opening different files */
                        fd = open(buf2, O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0666);
                        if (fd<0) SS_ERROR_FMT(FAILED, ("cannot create file `%s': %s", buf2, strerror(errno)));
                    }
                    if (task_enabled[self] && n>=0) {
                        if (dup2(fd, n)<0) SS_ERROR_FMT(FAILED, ("dup2 failed: %s", strerror(errno)));
                        close(fd);
                        fd = n;
                    }
                } else if (n>0) {
                    if (task_enabled[self]) fd = n;
                }
                
            } else if (!strncmp(s, "stop", 4) && (';'==s[4] || !s[4])) {
                /* Stop all affected tasks immediately */
                s += 4;
                if (task_enabled[self]) {
#ifdef HAVE_KILL
                    sprintf(buf1, "SSLIB: MPI task %d, PID %d is stopping with SIGSTOP\n", self, getpid());
                    if (strlen(buf1)!=(size_t)write(fd, buf1, strlen(buf1))) write(2, buf1, strlen(buf1));
                    kill(getpid(), SIGSTOP);
#else
                    SS_ERROR_FMT(NOTIMP, ("keyword `stop' is not supported in %s on this platform", source));
#endif
                }
                
            } else if (!strncmp(s, "pause=", 6)) {
                if ((0==(nl=strtol(s+6, &rest, 0)) && rest==s) || nl<0 || (*rest!=';' && *rest!='\0'))
                    SS_ERROR_FMT(USAGE, ("malformed pause value in %s at: %s", source, s));
                if (task_enabled[self]) {
                    fprintf(stderr, "SSLIB: MPI task %d, PID %d is pausing for %ld second%s\n", self, getpid(), nl, 1==nl?"":"s");
#ifdef WIN32
                    if (nl) Sleep((unsigned)nl*1000);
#else
                    if (nl) sleep((unsigned)nl);
#endif
                }
                s = rest;
                
            } else if (!strncmp(s, "banner=", 7)) {
                s += 7;
                for (nchars=0; s[nchars] && ';'!=s[nchars]; nchars++) /*void*/;
                sslib_g.banner = SS_FREE(sslib_g.banner);
                if (nchars) {
                    if (NULL==(sslib_g.banner=malloc(nchars+1))) SS_ERROR_FMT(RESOURCE, ("banner string"));
                    strncpy(sslib_g.banner, s, nchars);
                    sslib_g.banner[nchars] = '\0';
                }
                s += nchars;

            } else if (!strncmp(s, "debugger=", 9)) {
                s += 9;
                if ((rest = strchr(s, ';'))) {
                    nchars = MIN(sizeof(sslib_g.debugger)-1, (size_t)(rest-s));
                    strncpy(sslib_g.debugger, s, nchars);
                    sslib_g.debugger[nchars] = '\0';
                    s = rest;
                } else {
                    nchars = MIN(sizeof(sslib_g.debugger)-1, strlen(s));
                    strncpy(sslib_g.debugger, s, nchars);
                    sslib_g.debugger[nchars] = '\0';
                    s = NULL;
                }
                    
            } else if (!strncmp(s, "debug", 5) && (';'==s[5] || !s[5])) {

                /* Spawn a debugger and attach it to the first affected task */
                s += 5;
                if (task_enabled[self]) {
                    if (ss_debug_start(sslib_g.debugger)<0) SS_ERROR(FAILED);
                    debugger_started = TRUE;
                    sslib_g.debug_signal = FALSE;
                }
                
            } else if (!strncmp(s, "signal", 6) && (';'==s[6] || !s[6])) {
                /* Cause certain program error signals to start a debugger */
#ifdef HAVE_SIGACTION
                struct sigaction newaction;
                s += 6;
                if (task_enabled[self] && !debugger_started) {
                    sslib_g.debug_signal = TRUE;
                    newaction.sa_handler = ss_debug_signal;
                    sigemptyset(&(newaction.sa_mask));
                    newaction.sa_flags = 0;
                    sigaction(SIGABRT, &newaction, NULL);
                    sigaction(SIGSEGV, &newaction, NULL);
                    sigaction(SIGILL,  &newaction, NULL);
                    sigaction(SIGBUS,  &newaction, NULL);
                    sigaction(SIGFPE,  &newaction, NULL);
                }
#else
                SS_ERROR_FMT(NOTIMP, ("keyword `signal' not supported in %s on this platform", source));
#endif
                
            } else if (!strncmp(s, "stack", 5) && (';'==s[5] || !s[5])) {
                /* Set file descriptor for stack traces */
                s += 5;
                if (task_enabled[self]) ss_err_cntl_g.fd = fd;
                
            } else if (!strncmp(s, "pid", 3) && (';'==s[3] || !s[3])) {
#ifdef HAVE_GETPID
                s += 3;
                if (task_enabled[self]) {
                    sprintf(buf1, "SSLIB: MPI task %d has PID %d\n", self, getpid());
                    if (strlen(buf1)!=(size_t)write(fd, buf1, strlen(buf1))) write(2, buf1, strlen(buf1));
                }
#else
                SS_ERROR_FMT(NOTIMP, ("keyword `pid' not supported in %s on this platform", source));
#endif
                
            } else if (!strncmp(s, "commands", 8) && (';'==s[8] || !s[8])) {
                s += 8;
                if (task_enabled[self]) sslib_g.command_fd = fd;
                
            } else if (!strncmp(s, "mpi", 3) && (';'==s[3] || !s[3])) {
                s += 3;
                if (task_enabled[self]) sslib_g.ignore_mpierror = TRUE;
                
            } else if (!strncmp(s, "warnings", 8) && (';'==s[8] || !s[8])) {
                s += 8;
                if (task_enabled[self]) sslib_g.warnings = fdopen(fd, "w"); /*do not close old one*/
                
            } else if (!strncmp(s, "check=", 6)) {
                /* check=sync        -- turn on default synchronization checking/debugging options
                 * check=!sync       -- turn off all synchronization checking/debugging options
                 * check=sync=bcast  -- turn on sync bcast debugging
                 * check=!sync=bcast -- turn off sync bcast debugging
                 * etc. */
                s += 6;
                while (*s && ';'!=*s) {
                    hbool_t turn_off=FALSE;
                    if ('-'==*s) {
                        turn_off = TRUE;
                        s++;
                    }
                    if (!strncmp(s, "sync", 4) && (';'==s[4] || ','==s[4] || '='==s[4] || !s[4])) {
                        s += 4;
                        if ('='==*s) {
                            s++;
                            while (*s && ';'!=*s) {
                                if (!strncmp(s, "error", 5) && (';'==s[5] || ','==s[5] || !s[5])) {
                                    s += 5;
                                    if (task_enabled[self]) sslib_g.sync_check = turn_off ? FALSE : SS_STRICT;
                                } else if (!strncmp(s, "bcast", 5) && (';'==s[5] || ','==s[5] || !s[5])) {
                                    s += 5;
                                    if (task_enabled[self]) sslib_g.sync_bcast = turn_off ? FALSE : TRUE;
                                } else {
                                    SS_ERROR_FMT(USAGE, ("unknown attribute for check=%ssync in %s at: %s",
                                                         turn_off?"-":"", source, s));
                                }
                                while (*s && ','==*s) s++;
                            }
                        } else if (task_enabled[self]) {
                            if (turn_off) {
                                sslib_g.sync_check = FALSE;
                                sslib_g.sync_bcast = FALSE;
                            } else {
                                /* Default values */
                                sslib_g.sync_check = TRUE;
                                sslib_g.sync_bcast = FALSE;
                            }
                        }
                    } else if (!strncmp(s, "2pio", 4) && (';'==s[4] || ','==s[4] || !s[4])) {
                        s += 4;
                        sslib_g.tpio_alloc = turn_off ? FALSE : TRUE; /*regardless of task_enabled[]*/
                    } else {
                        SS_ERROR_FMT(USAGE, ("unknown category for check in %s at: %s%s",
                                             source, turn_off?"-":"", s));
                    }
                    while (*s && ','==*s) s++;
                }
                
            } else {
                SS_ERROR_FMT(USAGE, ("unknown term in %s at: %s", source, s));
            }

            while (s && ';'==*s) s++;
        }

#if 0 /* disable because there is no H5debug_mask() yet. --rpm 2003-09-12 */
        /* Broadcast the HDF5_DEBUG value and reinitialize HDF5's debug settings */
        if (H5debug_mask("-all")<0) SS_ERROR(HDF5);
        if (s_hdf5) {
            if (0==self) strcpy(s_here, s_hdf5);
            if (MPI_Bcast(s_here, s_len[1], MPI_CHAR, 0, comm)) SS_ERROR(MPI);
            if (H5debug_mask(s_here)<0) SS_ERROR(HDF5);
        }
#endif

        /* Cleanup */
        s_here = SS_FREE(s_here);
    }
 task_enabled = SS_FREE(task_enabled);
    
 SS_CLEANUP:
    SS_FREE(s_here);
    SS_FREE(task_enabled);

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Debugging
 * Purpose:     Start a debugger
 *
 * Description: Starts a debugger by forking with the child exec'ing the specified debugger whose first argument will be the
 *              name of the executable and whose second argument is the PID of the parent process.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, October 23, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_debug_start(const char *debugger)
{
    SS_ENTER(ss_debug_start, herr_t);
#ifdef HAVE_FORK
    int         self;
    pid_t       pid, child;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    pid = getpid();
    if (!debugger || !debugger[0]) debugger = "ddd";

    /* Emit a message to stderr saying that we're starting the debugger */
    fprintf(stderr, "SSLIB: MPI task %d, PID %d is starting debugger `%s'\n", self, pid, debugger);

    /* Fork, with child starting the debugger and parent stopping to wait */
    if (0==(child=fork())) {
        /* child execs the debugger */
        char arg1[64], arg2[64], buf[1024];
        sprintf(arg1, "/proc/%d/exe", getppid());
        sprintf(arg2, "%d", getppid());
        execlp(debugger, debugger, arg1, arg2, NULL);

        /* Fall back to shell command if unable to start it directly */
        sprintf(buf, "%s %s %s", debugger, arg1, arg2);
        execlp("/bin/sh", "/bin/sh", "-c", buf, NULL);

        fprintf(stderr, "SSLIB: unable to start debugger: %s %s %s\n", debugger, arg1, arg2);
        exit(1);
    } else if (child<0) {
        SS_ERROR_FMT(RESOURCE, ("fork() failed"));
    } else {
        /* parent stops now to wait for the debugger to attach */
        /* ISSUE: Sending SIGSTOP to the parent (the process to be debugged) works fine when the process is an MPICH task but
         *        causes the task to be suspended when it's run as a serial task from a shell. */ 
        kill(pid, SIGSTOP);
    }
#else
    SS_ERROR_FMT(NOTIMP, ("unable to start a debugger on this platform: no fork()"));
#endif

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Debugging
 * Purpose:     Start debugger for a signal
 *
 * Description: This is a signal handler that starts a debugger on the process that got the signal.
 *
 * Return:      Never returns
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, October 23, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void
ss_debug_signal(int signo)
{
    /* SS_ENTER -- not necessary since we never even return */
    char buf[256];
    int self;
    
    /* We should really set handlers back to their default values now and unmask them */
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    sprintf(buf, "SSLIB: MPI task %d, PID %d", self, getpid());
#ifdef HAVE_PSIGNAL
    psignal(signo, buf);
#else
    fprintf(stderr, "%s: %d\n", buf, signo);
#endif
    ss_debug_start(sslib_g.debugger);
    fprintf(stderr, "SSLIB: MPI task %d, PID %d is paused pending debug\n", self, getpid());
    
    while (true) {
#ifdef HAVE_PAUSE
        pause();
#endif
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Debugging
 * Purpose:     Evaluate a debug command
 *
 * Description: This function parses a debugging command in CMD and executes it. The first word in the string is the name
 *              of the command and the rest of the string contains the arguments for that command.
 *
 *              * files:
 *               Display information about all known files.
 *
 *              * classes:
 *               Show names of all object classes.
 *
 *              * /class/ /spec/:
 *               Display information about the specified persistent object. /Class/ is one of the class words such as
 *               `set' or `field', etc. Use the command `classes' for a complete list.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, April 18, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_debug_s(const char *cmd)
{
    SS_ENTER(ss_debug_s, herr_t);
    static size_t       old_begin[3] = {3, 0, 0};       /* beginning file, scope, object indices from previous command */
    static size_t       old_final[3] = {3, 0, 0};       /* ending (inclusive) file, scope, and object indices from previous */
    size_t              cur_begin[3], cur_final[3];     /* beginning and ending indices for the current command */
    hbool_t             indirect;                       /* whether the `i' or `I' is seen before a number */
    int                 self=0;                         /* MPI task rank */
    unsigned            seq;                            /* Magic sequence number for object classes */
    char                *rest;                          /* for strtol(), actually points to const char* */
    char                command[64];                    /* The first word of the command string */
    size_t              gfile_idx, scope_idx, obj_idx;  /* Indices for GFile, scope, and object */
    ss_gfile_t          *gfile=NULL;                    /* The GFile struct for the object being dumped */
    ss_scope_t          scope=SS_SCOPE_NULL;            /* The scope for the object being dumped */
    ss_pers_t           pers=SS_PERS_NULL;              /* The handle for the object being dumped */
    ss_persobj_t        *persobj=NULL;                  /* Pointer to the persistent object */
    char                intro[32];                      /* Prefix for each line of output */
    const char          *cs;                            /* Temporary pointer into CMD */
    ss_table_t          *table=NULL;                    /* Table holding the object to be dumped */
    
    int                 i;                              /* Counters */
    size_t              at;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    sprintf(intro, "SSlib-%d: ", self);

    /* Extract the command word from the beginning of the string and advance CMD to the beginning of the arguments */
    at = 0;
    while (*cmd && isspace(*cmd)) cmd++;
    while (*cmd && !isspace(*cmd)) {
        if (at+1>=sizeof command) SS_ERROR_FMT(USAGE, ("command name is too long"));
        command[at++] = *cmd++;
    }
    command[at] = '\0';
    while (*cmd && isspace(*cmd)) cmd++;

    /* Check for commands that are class names. Such commands are followed by up to three comma-separated indices or index
     * ranges, one for the file, one for the scope within the file, and one for the object within the table (the table that is
     * used is implied by the command, e.g, the "set" table for the "set" command). An index range is two indices separated by
     * a dash (such as `4-8') or a single `*'. An index is either a nonnegative integer preceded by an optional `i' or `I' to
     * indicate an indirect index. */
    for (seq=0; seq<SS_PERS_NCLASSES; seq++) {
        if (NULL==SS_PERS_CLASS(seq) || strcmp(command, SS_PERS_CLASS(seq)->name)) continue;

        /* Count the comma-separated indices and/or index ranges. There should be one, two, or three */
        for (cs=cmd, i=0; strchr(cs, ','); cs=strchr(cs, ',')+1, i++) /*void*/;
        if (i>2) SS_ERROR_FMT(USAGE, ("malformed arguments for `%s' command: %s", command, cmd));
        
        /* Parse each argument, skipping over the leading elements of cur_begin and cur_final that are not specified with
         * arguments. Use old_begin[] and old_final[] as defaults for missing index ranges. */
        memcpy(cur_begin, old_begin, sizeof cur_begin);
        memcpy(cur_final, old_final, sizeof cur_final);
        for (i=2-i; i<3 && *cmd; i++) {
            if ('*'==*cmd) {
                cur_begin[i] = 0;
                cur_final[i] = SS_NOSIZE;
                for (cmd++; *cmd && isspace(*cmd); cmd++) /*void*/;
            } else {
                /* First index in the range */
                if ('i'==*cmd || 'I'==*cmd) {
                    indirect = TRUE;
                    cmd++;
                } else {
                    indirect = FALSE;
                }
                errno = 0;
                cur_begin[i] = strtol(cmd, &rest, 0);
                if (rest==cmd || errno)
                    SS_ERROR_FMT(USAGE, ("bad arg list for `%s' command beginning at: %s", command, cmd));
                if (indirect) cur_begin[i] |= SS_TABLE_INDIRECT;
                for (cmd=rest; *cmd && isspace(*cmd); cmd++) /*void*/;

                /* Second index in the range */
                if ('-'==*cmd) {
                    for (cmd++; *cmd && isspace(*cmd); cmd++) /*void*/;
                    if ('i'==*cmd || 'I'==*cmd) {
                        indirect = TRUE;
                        cmd++;
                    } else {
                        indirect = FALSE;
                    }
                    errno = 0;
                    cur_final[i] = strtol(cmd, &rest, 0);
                    if (rest==cmd || errno)
                        SS_ERROR_FMT(USAGE, ("bad arg list for `%s' command beginning at: %s", command, cmd));
                    if (indirect) cur_final[i] |= SS_TABLE_INDIRECT;
                    for (cmd=rest; *cmd && isspace(*cmd); cmd++) /*void*/;
                    if (cur_final[i]<cur_begin[i])
                        SS_ERROR_FMT(USAGE, ("%s index range for `%s' is inverted: %lu-%lu",
                                             (0==i?"file":(1==i?"scope":"object")), command,
                                             (unsigned long)(cur_begin[i]), (unsigned long)(cur_final[i])));
                } else {
                    cur_final[i] = cur_begin[i];
                }
                
                if (','==*cmd)
                    for (cmd++; *cmd && isspace(*cmd); cmd++) /*void*/;
            }
        }
        if (cmd && *cmd)
            SS_ERROR_FMT(USAGE, ("malformed arguments for `%s' command beginning at: %s", command, cmd));

        /* Save values as defaults for next command */
        memcpy(old_begin, cur_begin, sizeof cur_begin);
        memcpy(old_final, cur_final, sizeof cur_final);

        /* Dump everything */
        for (gfile_idx=cur_begin[0]; gfile_idx<=cur_final[0]; gfile_idx++) {
            gfile = SS_GFILE_IDX(gfile_idx);
            if (!gfile) {
                if (SS_NOSIZE==cur_final[0]) break;
                SS_ERROR_FMT(NOTFOUND, ("GFile entry %lu does not exist", (unsigned long)gfile_idx));
            }

            for (scope_idx=cur_begin[1]; scope_idx<=cur_final[1]; scope_idx++) {
                if (NULL==ss_pers_refer_c(gfile->topscope, SS_MAGIC(ss_scope_t), scope_idx, (ss_pers_t*)&scope) ||
                    NULL==SS_SCOPE(&scope)) {
                    if (SS_NOSIZE==cur_final[1]) break;
                    SS_ERROR_FMT(FAILED, ("failed to create handle for scope %lu", (unsigned long)scope_idx));
                }

                if (cur_begin[2]==cur_final[2]) {
                    obj_idx = cur_begin[2];
                    if (NULL==ss_pers_refer_c(&scope, seq, obj_idx, &pers))
                        SS_ERROR_FMT(FAILED, ("failed to create handle for object %lu", (unsigned long)obj_idx));
                    if (ss_pers_update(&pers)<0) SS_ERROR(FAILED);
                    ss_pers_dump(&pers, stdout, "  ", "dumping %s %lu,%lu,%s%lu",
                                 command, (unsigned long)gfile_idx, (unsigned long)scope_idx,
                                 (obj_idx & SS_TABLE_INDIRECT) ? "I" : "",
                                 (unsigned long)(obj_idx & ~SS_TABLE_INDIRECT));
                } else {
                    /* We could just loop over the specified objects like above with ss_pers_refer_c() and call ss_pers_dump()
                     * for each one, but those two functions are happy to create links or to dump objects that don't even
                     * exist. Therefore we will query the table to see what objects exist and only dump those. */
                    if (NULL==(table=ss_scope_table(&scope, seq, NULL))) SS_ERROR(FAILED);
                    for (obj_idx=cur_begin[2]; obj_idx<=cur_final[2]; obj_idx++) {
                        persobj = ss_table_lookup(table, obj_idx, 0u);
                        if (!SS_MAGIC_OK(SS_MAGIC_OF(persobj))) break;
                        if (NULL==ss_pers_refer_c(&scope, seq, obj_idx, &pers))
                            SS_ERROR_FMT(FAILED, ("failed to create handle for object %lu", (unsigned long)obj_idx));
                        if (ss_pers_update(&pers)<0) SS_ERROR(FAILED);
                        ss_pers_dump(&pers, stdout, "  ", "dumping %s %lu,%lu,%s%lu",
                                     command, (unsigned long)gfile_idx, (unsigned long)scope_idx,
                                     (obj_idx & SS_TABLE_INDIRECT) ? "I" : "",
                                     (unsigned long)(obj_idx & ~SS_TABLE_INDIRECT));
                    }
                }
            }
        }
        goto done;
    }
    
    /* Other commands */
    if (!strcmp(command, "classes")) {
        for (i=0; i<SS_PERS_NCLASSES; i++) {
            ss_pers_class_t *cls = SS_PERS_CLASS(i);
            if (!cls) continue;
            printf("%s#%-4d %s\n", intro, i, cls->name);
            printf("%s      tfm(%lu) tff(%lu) tm(%lu) tf(%lu) t_size(%lu)\n",
                   intro, (unsigned long)(cls->tfm), (unsigned long)(cls->tff), (unsigned long)(cls->tm),
                   (unsigned long)(cls->tf), (unsigned long)(cls->t_size));
        }
    } else if (!strcmp(command, "files")) {
        ss_gfile_debug_all(stdout);
    } else {
        SS_ERROR_FMT(USAGE, ("unknown debugging command: %s", command));
    }

done:
    SS_STATUS_OK;
SS_CLEANUP:
    SS_LEAVE(0);
}

