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

#include <safP.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h> /*for getpid() on WIN32*/
#endif

#define SAF_BUILTIN_REGISTRY    "/SAF:std_types"

SAF_Library     _SAF_GLOBALS;
hbool_t         _SAF_InitDone;
int             SAF_VERSION_VAR;                /*global variable depending on SAF version number*/
int             SAF_PARALLEL_VAR;               /*global variable depending on parallel or serial compilation*/
int             _saf_skip_init_g;
int             _saf_check_grabbed_g;

/* These global variables point into the built-in object registry and are initialized by _saf_gen_stdtypes() */
SAF_Set         SAF_UNIVERSE_SET_g;            /* See SAF_UNIVERSE macro */
SAF_Cat         SAF_SELF_CAT_g;                /* See SAF_SELF macro */
SAF_Field       SAF_NOT_APPLICABLE_FIELD_g;
SAF_Role        SAF_NOT_APPLICABLE_ROLE_g;
SAF_Basis       SAF_NOT_APPLICABLE_BASIS_g;
SAF_Algebraic   SAF_NOT_APPLICABLE_ALGEBRAIC_g;
SAF_Eval        SAF_NOT_APPLICABLE_EVALUATION_g;
SAF_RelRep      SAF_NOT_APPLICABLE_RELREP_g;
SAF_Quantity    SAF_NOT_APPLICABLE_QUANTITY_g;
SAF_Unit        SAF_NOT_APPLICABLE_UNIT_g;
SAF_Cat         SAF_NOT_APPLICABLE_CAT_g;
SAF_Set         SAF_NOT_APPLICABLE_SET_g;
ss_collection_t SAF_NOT_APPLICABLE_COLLECTION_g;
SAF_Rel         SAF_NOT_APPLICABLE_REL_g;
ss_tops_t       SAF_NOT_APPLICABLE_TOPS_g;
ss_blob_t       SAF_NOT_APPLICABLE_BLOB_g;
ss_indexspec_t  SAF_NOT_APPLICABLE_INDEXSPEC_g;

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Environment
 * Description: A number of environment variables affect the operation of SAF such as error detections and reporting as
 *		well as where predefined types are obtained.
 *--------------------------------------------------------------------------------------------------------------------------------
 */

/* all of the following macros are really dummys for MkDoc's sake and are otherwise, unnecessary.
   They are surrounded by '#if 0 ... #endif' to avoid polluting the compiled code. */

#if 0
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Environment	
 * Purpose: 	Trace SAF API calls and times	
 * Concepts:	Environment; Tracing	
 * Description: This is a string valued environment variable used to control API call tracing in SAF. It may be set to
 *		any one of the values described below. Note that API call tracing is logged to the same file specified
 *		by SAF_ERROR_REPORTING. However, if SAF_ERROR_REPORTING is set to "none" and SAF_TRACING is not also
 *		"none", SAF will log its API tracing to stderr. Currently, SAF only logs entrances to SAF API calls, not
 *		exits.
 *
 *		Note: Since the bulk of SAF's API is collective, only processor 0 actually prints any trace information.
 *
 *		   none
 *
 *		This is the default. It means that no API tracing will be generated.
 *
 *		   times
 *
 *		This setting will record the cumulative amount of time spent in saf_read_xxx() calls and saf_write_xxx() calls
 *		as compared to the total time between calls to saf_init() and saf_final(). The times recorded are wall clock
 *		seconds. Entrances to functions WILL NOT be reported. However, during saf_final(), the cumulative timers
 *		for time spent in reads and writes will be reported.
 *
 *		   public
 *
 *		Public API calls will be logged to whatever file SAF is also reporting errors to.
 *
 *		   public,times
 *
 *		Same as "public" but SAF will also output wall clock times since the last API call was entered. SAF
 *		will report the delta since the last call and the absolute time, starting from 0. The times reported
 *		are WALL CLOCK seconds, not CPU seconds. Thus, if there are other activities causing SAF to run more slowly
 *		then it will be reflected in the times SAF reports.
 *
 *		   public,private
 *
 *		Both public and private API calls are logged.
 *
 *		   public,private,times
 *
 *		Both public and private API calls are logged along with timing information.
 *
 *		Finally, if SAF_TRACING is set to a valid value other than "none", SAF will also invoke DSL's tracing
 *		facilities. However, DSL's tracing facilities WILL NOT take effect unless the environment var DSL_TRACING is
 *		also defined in the environment. Thus, DSL's tracing can be turned on/off separately by setting or unsetting
 *		the DSL_TRACING environment variable.
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_TRACING	0	/* environment variable */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Environment	
 * Purpose: 	Control Reporting of Error Messages
 * Concepts:	Environment; Error Reporting
 * Description: This is a string valued environment variable that may be set to one of the following values...
 *
 *		  none
 *
 *              Means no error reporting. SAF does not print error messages. This is the default
 *		for a production compile but may be overridden at anytime by use of this environment variable.
 *
 *		  stderr
 *
 *              Means SAF sends its error messages to the stderr stream. This is the default for
 *		a serial, development compile. See below for the default for a parallel, development compile.
 *		If this mode is selected in parallel, SAF will prepend each message with the rank of the
 *		MPI task in the communicator used to initialize SAF (see saf_init()) to each line of output
 *		in this file.
 *
 *		  file: /name/                               // no white space
 *
 *              Where /name/ is a file name, this means SAF will open a stream by
 *		this name and send its error messages to this stream. In parallel, SAF will prepend the rank
 *		of the task in the communicator used to initialize SAF (see saf_init()) to each line
 *		of output to this file. However, the order of task's output to this file is indeterminate.
 *
 *		  procfile: /prefix/,/fmt/,/suffix/          // no white space
 *
 *              where /prefix/ and /suffix/ are parts of a name and /fmt/ is a printf
 *              style integer format designation for including the task number in the name. For example, in
 *              'procfile:saf_,%03d,.log", the prefix is 'saf_', task number format designation is '%03d' and the
 *              suffix is '.log'. If this mode is selected in a serial run, the task number format designation will
 *              be ignored. A minor issue with this form of error logging is that it generates one file for each
 *              task. If you have a 1,000 task run, you get 1000 files. However, it does keep each task's
 *              outputs separate, unlike the preceding mode. However, the following mode gets around this problem by
 *              generating only a single log file and forcing each proc to write to only a given segment of the file.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ERROR_REPORTING 0	/* environment variable */
/*-------------------------------------------------------------------------------------------------------------------------------
 *		   segfile:<prefix>,<rows>,<cols>,<suffix> TEMPORARLILY DISABLED. DO NOT USE.
 *
 *		where <prefix> and <suffix> are parts of a name and <rows> and <cols> specify the size, in rows
 *		and columns of characters, of each processor's writable segment of a single file. For example,
 *		in the string "segfile:saf_errors,10,80,.log", <prefix>='saf_errors', <suffix>='.log',
 *		<rows>='10' and <cols>='80'. This tells SAF to create
 *		a single file which all processors write to. Processor with rank /i/ in the communicator used
 *		to initialize SAF (see saf_init()), writes to lines i*10...(i+1)*10-1, where each line is
 *		of width 80 characters. This method is offered as an alternative to the 1 file per processor
 *		method, but still keeps each processor's stream relatively separate. The trick is that each
 *		processor gets a fixed number of lines (characters) to write to in the log file. Since it is
 *		likely that SAF will have to abort if it detects an error in parallel, the amount of error output
 *		for each processor is relatively small; just a few lines. This is the default error logging mode
 *		(with <a>='saf_errors_', <b>='.log', <r>='10' and <c>='80') for a parallel, development compile.
 *		Finally, if a processor's output exceeds the number of lines allocated to a processor, its output
 *		will wrap around to the beginning of its output block. This way, you may always see the last
 *		<r> lines of output. Error messages are prepended with the processor's rank and error message call count.
 *--------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Environment	
 * Purpose:  	Control Assertion Checking	
 * Concepts:	Environment; Error Checking 
 * Description: There are three environment variables that control, independently, the level or pre-, post- and assert-
 *		condition checking done by SAF. They are...
 *
 *
 *                 SAF_ASSERT_DISABLE
 *
 *                 SAF_PRECOND_DISABLE
 *
 *                 SAF_POSTCOND_DISABLE
 *
 *		These three environment variables control the level of assertion, pre-condition and post-condition checking,
 *		respectively, that SAF does during execution. Each is a string valued environment variable with possible values
 *		"none", "high", "medium", and "all". For example, a value of "none" for SAF_ASSERT_DISABLE means that none
 *		of the assertion checking is disabled. A value of "high" means that all high cost assertions are disabled but
 *		medium and low cost assertions are still enabled. A value "medium" means that all high and medium cost
 *		assertions are disabled but low cost assertions are still enabled. A value of "all" means that all assertions
 *		are disabled. Likewise, SAF_PRECOND_DISABLE controls pre-condition checking and SAF_POSTCOND_DISABLE
 *		controls post-condition checking.
 *		
 *		The cost of an assertion, pre-condition or post-condition is specified in terms relative to the SAF function
 *		in which the condition is checked. This means that a simple test for a null pointer in a very simple SAF
 *		function, such as saf_setProps_LibComm(), is considered high cost while in a saf_declare_field() it is considered
 *		low cost. This is so because the test for a null pointer in saf_setProps_LibComm() relative to the other work
 *		saf_setProps_LibComm() does is high cost while that same test in saf_declare_field() is relatively low cost.
 *
 *		In addition to controlling SAF's assertion, pre-condition and post-condition checking, these environment
 *		variables also control similar checks that go on in the lower layers of SAF. These lower layers do not
 *		have high, medium and low check costs. Instead, they can either be turned on or off. The checks are performed
 *              if SAF_ASSERT_DISABLE is "none", and not performed otherwise.
 *
 *		Assertion, pre-condition and post-condition checking has a marked effect on performance. To obtain maximum
 *		performance, all checks should be turned off using
 *
 *
 *                 setenv SAF_ASSERT_DISABLE all
 *                 setenv SAF_PRECOND_DISABLE all
 *                 setenv SAF_POSTCOND_DISABLE all
 *
 *              or
 *
 *                 env SAF_ASSERT_DISABLE=all \
 *                     SAF_PRECOND_DISABLE=all \
 *                     SAF_POSTCOND_DISABLE=all   a.out ...
 *
 *		For each of these environment variables, if it does not exist, SAF will set the default values depending
 *              on whether the library was compiled for production or development. For a production compile, the default
 *              values for all three environment settings are "all" meaning that the error checking for assertion,
 *              pre-condition and post-condition checking is set for maximal performance. For a development compile, the
 *              default setting for all three is "none" meaning it is set for maximal error checking.
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ASSERT_DISABLE 0	/* environment variable */


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Environment	
 * Purpose:  	Control Precondition Checking	
 * Concepts:	Environment; Error Checking 
 * Description:	See SAF_ASSERT_DISABLE
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_PRECOND_DISABLE 0	/* environment variable */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Environment	
 * Purpose:  	Control Postcondition Checking	
 * Concepts:	Environment; Error Checking 
 * Description:	See SAF_ASSERT_DISABLE
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_POSTCOND_DISABLE 0	/* environment variable */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Environment	
 * Purpose:  	Specify location of SAF's standard types database	
 * Concepts:	Environment; Standard types 
 * Description: This is a string valued environment variable that holds colon (':') separated list of pathnames files from where
 *		SAF will obtain predefined type definitions. If this variable is not set, SAF will build an use a transient
 *	 	database containing a minimal set of pre-defined types. In typical usage, this variable need not be set.
 *
 *		By default, SAF will generate a minimal, memory resident registry that is destroyed when saf is finalized. This
 * 		permits SAF to operate in such a way that it does not need to access some registry file on disk somewhere to
 *		properly initialize. However, other, disk resident registry files can be opened if either the env. variable,
 *		SAF_REGISTRIES, is set and/or the client has specified a specific registry with the initialization properties.
 *
 *		Files specified by the SAF_REGISTRIES env. variable are first in the list followed by those specified by the
 *		initialization properties. In this way, if SAF_REGISTRIES is specified, the definitions of symbols there take
 *		precedence over those that may also exist in the file(s) specified by the initialization properties.
 * 
 *		If SAF_REGISTRIES is set to the string "implicit", it will look for a file named `Registry.saf` in the
 *		/usual/places/, namely in the current working directory, then in the user's home directory and finally in the
 *		SAF installation directory.
 *
 *		Errors will be reported for registries specified explicitly by environment variables
 *		and/or calls to saf_SetProps_Registry(), but not for the implicit locations.  A warning will be issued if no
 *		registry can be found at all.
 *
 *		Finally, if SAF_REGISTRIES is set to the string "none", then no registries will be
 *		opened, not even the minimal, memory resident one. If SAF_REGISTRIES is set to the string "default" then only the
 *		minimal registry will be opened. That is, you can force SAF to ignore all registries specified by a client through
 *		the initialization properties by setting SAF_REGISTRIES to "default".
 *
 *              Note that the order in which filenames are specified is important. When SAF needs to look up a pre-defined
 *              datatype, it searches its known registries in the order in which they were specified in SAF_REGISTRIES,
 *              then those specified with saf_setProps_Registry. SAF returns the first matching referenced type.
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_REGISTRIES 0	/* environment variable */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Environment
 * Purpose:     Built-in registry name
 * Concepts:    Environment; Standard types; registry; object registry
 *
 * Description: Normally SAF will create a minimal object registry that exists only in memory. However, if this environment
 *              variable is set to the name of a file, then SAF will save the built-in object registry in that file. This is
 *              intended only to be used for debugging to make sure that the object registry file's contents are what is
 *              expected.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_REGISTRY_SAVE       /* environment variable */
#endif

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Library Initialization
 * Description: To interact with SAF, the client must call saf_init(). To end interaction with SAF, the client must call
 *		saf_final().
 *
 *		In parallel, SAF will not call MPI_Init or MPI_Finalize on behalf of the client. It is the client's
 *		responsibility to initialize and finalize MPI. MPI should be initialized before calling saf_init() and
 *		finalized after calling saf_final().
 *
 *		The only SAF functions that can be called outside of an enclosing saf_init()/saf_final() pair are functions
 *		to create and set library properties (see Library Properties).
 *
 *		SAF provides a link-time library and include file consistency check that will generate an undefined reference
 *		link-time error for a symbol whose name is of the form "SAF_version_X_Y_Z", if the library and include files are
 *		not consistent. 
 *--------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Library Initialization
 * Purpose:     Extend registry list
 *
 * Description: Extends the registry-related global arrays to be able to hold N more registry names.
 *
 * Return:      Zero on success, negative otherwise.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, July 25, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
_saf_grow_registries(size_t n)
{
    SAF_ENTER(_saf_grow_registries,-1);

    if (_SAF_GLOBALS.p.reg.nused + n > _SAF_GLOBALS.p.reg.nalloc) {
        size_t total = MAX3(16, _SAF_GLOBALS.p.reg.nalloc+n, 2*_SAF_GLOBALS.p.reg.nalloc);
        size_t i;
        void *x;

        /* Extend the arrays if possible */
        if (NULL==(x=realloc(_SAF_GLOBALS.p.reg.name, total*sizeof(_SAF_GLOBALS.p.reg.name[0])))) SAF_RETURN(-1);
        _SAF_GLOBALS.p.reg.name = x;
        
        if (NULL==(x=realloc(_SAF_GLOBALS.p.reg.nowarn, total*sizeof(_SAF_GLOBALS.p.reg.nowarn[0])))) SAF_RETURN(-1);
        _SAF_GLOBALS.p.reg.nowarn = x;

        if (NULL==(x=realloc(_SAF_GLOBALS.p.reg.db, total*sizeof(_SAF_GLOBALS.p.reg.db[0])))) SAF_RETURN(-1);
        _SAF_GLOBALS.p.reg.db = x;
        

        /* Initialize the new array values */
        for (i=_SAF_GLOBALS.p.reg.nalloc; i<total; i++) {
            _SAF_GLOBALS.p.reg.name[i] = NULL;
            _SAF_GLOBALS.p.reg.nowarn[i] = SAF_TRISTATE_FALSE;
            _SAF_GLOBALS.p.reg.db[i] = NULL;
        }

        /* Finalize the extension */
        _SAF_GLOBALS.p.reg.nalloc = total;
    }
    SAF_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Initialization
 * Purpose:     Initialize the built-in object registry
 *
 * Description: The built-in object registry is a SAF file that contains definitions for objects that are frequently used.
 *              This function also caches some of those objects in global variables.
 *
 * Issue:       Since other files probably point into the built-in registry and they do so by specifying a table row number,
 *              we have to be sure that we always create the built-in registry the same way. It is important that we don't
 *              move objects around in the tables over the life of the file--only add new objects to the end of the table.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the library communicator.
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static herr_t
_saf_gen_stdtypes(ss_file_t *stdtypes)
{
    SAF_ENTER(_saf_gen_stdtypes, -1);
    ss_scope_t  stdscope;
    int         i;

    /* WARNING: SEE COMMENTS IN PROLOGUE. DO NOT CHANGE THE ORDER OF, OR DELETE OBJECTS DEFINED HERE BECAUSE EXISTING FILES
     *          MAY BE POINTING INTO THE BUILT-IN REGISTRY AND DO NOT EXPECT IT TO CHANGE OVER TIME. ONLY ADD NEW OBJECTS. */

    if (NULL==ss_file_topscope(stdtypes, &stdscope))
        SAF_ERROR(-1, _saf_errmsg("cannot find top scope for built-in registry file"));

    /* Create "not applicable" objects and cache them in global variables. By convention, a "not applicable" object is simply
     * what gets created by SS_PERS_NEW() by default. */
    ss_pers_new(&stdscope, SS_MAGIC(ss_field_t),      NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_FIELD_g,      NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_role_t),       NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_ROLE_g,       NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_basis_t),      NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_BASIS_g,      NULL); 
    ss_pers_new(&stdscope, SS_MAGIC(ss_algebraic_t),  NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_ALGEBRAIC_g,  NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_evaluation_t), NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_EVALUATION_g, NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_relrep_t),     NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_RELREP_g,     NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_quantity_t),   NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_QUANTITY_g,   NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_unit_t),       NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_UNIT_g,       NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_cat_t),        NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_CAT_g,        NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_set_t),        NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_SET_g,        NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_collection_t), NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_COLLECTION_g, NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_rel_t),        NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_REL_g,        NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_tops_t),       NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_TOPS_g,       NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_blob_t),       NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_BLOB_g,       NULL);
    ss_pers_new(&stdscope, SS_MAGIC(ss_indexspec_t),  NULL, SS_ALLSAME, (ss_pers_t*)&SAF_NOT_APPLICABLE_INDEXSPEC_g,  NULL);

    /* The seven basic quantities */
    _SAF_GLOBALS.quant[SAF_BASEQ_TIME]    = saf_declare_quantity(SAF_ALL, stdtypes,
                                                                 "time",                      NULL,          "builtin", NULL);
    _SAF_GLOBALS.quant[SAF_BASEQ_MASS]    = saf_declare_quantity(SAF_ALL, stdtypes,
                                                                 "mass",                      NULL,          "builtin", NULL);
    _SAF_GLOBALS.quant[SAF_BASEQ_CURRENT] = saf_declare_quantity(SAF_ALL, stdtypes,
                                                                 "electric current",          "current",     "builtin", NULL);
    _SAF_GLOBALS.quant[SAF_BASEQ_LENGTH]  = saf_declare_quantity(SAF_ALL, stdtypes,
                                                                 "length",                    NULL,          "builtin", NULL);
    _SAF_GLOBALS.quant[SAF_BASEQ_LIGHT]   = saf_declare_quantity(SAF_ALL, stdtypes,
                                                                 "luminous intensity",        "light",       "builtin", NULL);
    _SAF_GLOBALS.quant[SAF_BASEQ_TEMP]    = saf_declare_quantity(SAF_ALL, stdtypes,
                                                                 "thermodynamic temperature", "temperature", "builtin", NULL);
    _SAF_GLOBALS.quant[SAF_BASEQ_AMOUNT]  = saf_declare_quantity(SAF_ALL, stdtypes,
                                                                 "amount of a substance",     "amount",      "builtin", NULL);
    for (i=0; i<SS_MAX_BASEQS; i++) {
        SAF_DIRTY(_SAF_GLOBALS.quant[i], SAF_ALL);
        SS_QUANTITY(_SAF_GLOBALS.quant[i])->power[i] = 1;
    }
    
    /* Additional things */
    ss_pers_new(&stdscope, SS_MAGIC(ss_set_t), NULL, SS_ALLSAME, (ss_pers_t*)&SAF_UNIVERSE_SET_g, NULL);
    ss_string_set(SS_SET_P(&SAF_UNIVERSE_SET_g,name), "universe");
    
    ss_pers_new(&stdscope, SS_MAGIC(ss_cat_t), NULL, SS_ALLSAME, (ss_pers_t*)&SAF_SELF_CAT_g,     NULL);
    ss_string_set(SS_CAT_P(&SAF_SELF_CAT_g,name), "self");

    SAF_LEAVE(0);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Library Initialization
 * Purpose:	Initialize the library
 * Concepts:	Initialization
 *
 * Description:	This is the real implementation of saf_init() (all documentation is contained there).
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_init(SAF_LibProps *properties      /* This argument, if not null, provides library property
					 * values which will override the default properties. */
          )
{
    SAF_ENTER_NOINIT(_saf_init, SAF_PRECONDITION_ERROR);
    
    SAF_LibProps        *p = properties;
    int                 i;
    int                 mpi_initialized=TRUE;
    ss_prop_t           *local_props=NULL;
    MPI_Comm            comm_self = SS_COMM_SELF;
    char                *save_registry=NULL;
    unsigned            builtin_flags;

    if(_SAF_InitDone)
        SAF_RETURN(SAF_SUCCESS);

    /* create default properties if none were specified and use them to initialize the _SAF_GLOBAL stuff, which we might adjust
     * below. */
    if (!p)
        p = saf_createProps_lib();
    _SAF_GLOBALS.p = *p;
    memset(&(_SAF_GLOBALS.p.reg), 0, sizeof(_SAF_GLOBALS.p.reg));

    /* We set the default error checking properties differently depending on whether this is a production compile or not. For a
     * production compile, error checking is by default maximally disabled. Otherwise, it is by default, maximally enabled. */
    _saf_set_disable_threshold("SAF_ASSERT_DISABLE",&(_SAF_GLOBALS.AssertDisableCost));
    _saf_set_disable_threshold("SAF_PRECOND_DISABLE",&(_SAF_GLOBALS.PreCondDisableCost));
    _saf_set_disable_threshold("SAF_POSTCOND_DISABLE",&(_SAF_GLOBALS.PostCondDisableCost));
    _SAF_GLOBALS.AtExiting = TRUE;	/* a client call to saf_final sets this to false */

    /* confirm MPI is initialized */
#ifdef HAVE_PARALLEL
    if ((MPI_Initialized(&mpi_initialized) != MPI_SUCCESS)) mpi_initialized=FALSE;
#endif
    if (!mpi_initialized) {
        _SAF_GLOBALS.p.DoAbort = false; /* we have to turn off call to abort here */
        SAF_ERROR(SAF_FATAL_ERROR, _saf_errmsg("MPI is not running"));
    }

    /* duplicate the communicator and cache its rank and size */
    if (ss_mpi_comm_dup(p->LibComm, &(_SAF_GLOBALS.p.LibComm))<0)
	SAF_ERROR(SAF_FATAL_ERROR, _saf_errmsg("unable to duplicate MPI communicator"));
    _SAF_GLOBALS.Rank = ss_mpi_comm_rank(_SAF_GLOBALS.p.LibComm);
    _SAF_GLOBALS.Size = ss_mpi_comm_size(_SAF_GLOBALS.p.LibComm);

    /* set wall clock timer start value on all processors */
    _saf_wall_clock(true);
    ss_mpi_bcast(&_SAF_GLOBALS.WallClockTime, 1, MPI_DOUBLE, 0, _SAF_GLOBALS.p.LibComm);

    /* Initialize other subsystems from bottom up */
    ss_init(_SAF_GLOBALS.p.LibComm);

    /* The cached "not applicable" index spec */
    _SAF_GLOBALS.indexspec_not_applicable.ndims=0;
    for (i=0; i<SAF_MAX_NDIMS; i++) {
        _SAF_GLOBALS.indexspec_not_applicable.sizes[i]   = 0;
        _SAF_GLOBALS.indexspec_not_applicable.origins[i] = 0;
        _SAF_GLOBALS.indexspec_not_applicable.order[i]   = 0;
    }

    /* If client asked for PROCFILE mode, we need to do a little more work on the file name */
    if (p->ErrorLogMode == SAF_ERRLOG_PROCFILE)
        sprintf(_SAF_GLOBALS.p.ErrorLogName, p->ErrorLogName, _SAF_GLOBALS.Rank); 

    /* Open the error log stream. */
    switch (p->ErrorLogMode) {
    case SAF_ERRLOG_FILE:
    case SAF_ERRLOG_PROCFILE:
        if ((_SAF_GLOBALS.ErrorLogFile = fopen(_SAF_GLOBALS.p.ErrorLogName, "a")) == NULL) /*"a" necessary for parallel*/
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to open logfile stream \"%s\"",_SAF_GLOBALS.p.ErrorLogName));
        break;
    case SAF_ERRLOG_SEGFILE:
        if ((_SAF_GLOBALS.ErrorLogFile = fopen(_SAF_GLOBALS.p.ErrorLogName, "w")) == NULL)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to open logfile stream \"%s\"",_SAF_GLOBALS.p.ErrorLogName));
        break;
    case SAF_ERRLOG_STDERR:
        _SAF_GLOBALS.ErrorLogFile = stderr;
        break;
    case SAF_ERRLOG_NONE:
        if (p->TraceLogMode!=SAF_TRACELOG_NONE)
            _SAF_GLOBALS.ErrorLogFile = stderr;
        break;
    }
   
    /* make sure error logging output is unbuffered */
    if (_SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_NONE)
        setbuf(_SAF_GLOBALS.ErrorLogFile, NULL);

    /* Create the string pool if so requested */
    if (SAF_STRMODE_POOL==_SAF_GLOBALS.p.StrMode) {
        _SAF_GLOBALS.p.StrPoolSize = MAX(_SAF_GLOBALS.p.StrPoolSize, 16);
        _SAF_GLOBALS.str.pool = calloc(_SAF_GLOBALS.p.StrPoolSize, sizeof(_SAF_GLOBALS.str.pool[0]));
    }

    /* Various propery lists used throughout the library */
    _SAF_GLOBALS.find_detect_overflow = ss_prop_new("find detects overflows");
    ss_prop_add(_SAF_GLOBALS.find_detect_overflow, "detect_overflow", H5T_NATIVE_HBOOL, &true);
    
    /* we're done with the init */
    _SAF_InitDone = true;

    /* Create a transient file locally on each MPI task. This file will be used for things such as the various "key" objects
     * for searching. All tasks open a file with the same name but all such files are independent. We do it this way because
     * there isn't any need for "key" objects to be synchronized between tasks. */
    local_props = ss_prop_new("local properties");
    ss_prop_add(local_props, "comm", H5T_NATIVE_MPI_COMM, &comm_self);
    _SAF_GLOBALS.local_file = ss_file_create("/SAF:local", H5F_ACC_TRANSIENT, local_props);
    _SAF_GLOBALS.local_scope = ss_file_topscope(_SAF_GLOBALS.local_file, NULL);
    ss_prop_dest(local_props);
    local_props = NULL;

    /* Create the default object registry file. This file is a library-wide transient file which is populated with frequently
     * used objects and then marked as read-only. However, if the SAF_REGISTRY_SAVE environment variable is set then use its
     * value as the name of the registry file that should be saved.  This is currently (as of 2004-08-01) used only for
     * debugging because a SAF file created in this manner is not read properly later, probably because object `equal'
     * comparison is not implemented and falls back to `eq' comparison. */
    SAF_GETENV_LIB(save_registry, "SAF_REGISTRY_SAVE");
    if (save_registry && *save_registry) {
        _SAF_GLOBALS.reg.std_name = save_registry; /*do not free save_registry at end of this function*/
        builtin_flags = 0;
    } else {
        _SAF_GLOBALS.reg.std_name = malloc(strlen(SAF_BUILTIN_REGISTRY)+1);
        strcpy(_SAF_GLOBALS.reg.std_name, SAF_BUILTIN_REGISTRY);
        builtin_flags = H5F_ACC_TRANSIENT;
    }
    if (NULL==(_SAF_GLOBALS.reg.std_file=ss_file_create(_SAF_GLOBALS.reg.std_name, builtin_flags, NULL)))
        SAF_ERROR(-1, _saf_errmsg("cannot create built-in registry file \"%s\"", _SAF_GLOBALS.reg.std_name));
    if (NULL==(_SAF_GLOBALS.reg.std_scope=ss_file_topscope(_SAF_GLOBALS.reg.std_file, NULL)))
        SAF_ERROR(-1, _saf_errmsg("cannot find top scope for built-in registry file"));

    _saf_gen_stdtypes(_SAF_GLOBALS.reg.std_file); /* Very basic stuff required for proper library functioning */
    _saf_gen_registry(_SAF_GLOBALS.reg.std_file); /* Additional commonly used objects */

    /* Mark the file read-only since we won't make any more modifications. Doing so also makes things more efficient because
     * SSlib will be able to tell that no synchronization will ever be necessary.  No synchronization is necessary now because
     * everything created so far was created collectively with SS_ALLSAME. */
    if (0==(builtin_flags & H5F_ACC_TRANSIENT) && ss_file_flush(_SAF_GLOBALS.reg.std_file, NULL)<0)
        SAF_ERROR(-1, _saf_errmsg("cannot flush saved built-in registry file \"%s\"", _SAF_GLOBALS.reg.std_name));
    ss_file_readonly(_SAF_GLOBALS.reg.std_file);

    /* Issue: By default, SAF will generate a minimal, memory resident registry that is destroyed when saf is finalized. This
     *        permits SAF to operate in such a way that it does NOT need to access some registry file on disk somewhere to
     *        properly initialize. However, other, disk resident registry files can be opened if either the env. variable,
     *        SAF_REGISTRIES, is set and/or the client has specified a specific registry with the initialization
     *        properties. Files specified by the SAF_REGISTRIES env. variable are first in the list followed by those specified
     *        by the initialization properties.  In this way, if SAF_REGISTRIES is specified, the definitions of symbols there
     *        take precedence over those that may also exist in the file(s) specified by the initialization properties. If
     *        SAF_REGISTRIES is set to the string "implicit", it will look for a file named `Registry.saf` in the /usual
     *        places/, namely in the current working directory, then in the user's home directory and finally in the SAF
     *        installation directory.  Errors will be reported for registries specified explicitly by environment variables
     *        and/or calls to saf_SetProps_Registry(), but not for the implicit locations.  A warning will be issued if no
     *        registry can be found at all. Finally, if SAF_REGISTRIES is set to the string "none", then no registries will be
     *        opened, not even the minimal, memory resident one. If SAF_REGISTRIES is set to the string "default" then only the
     *        minimal registry will be opened. That is, you can force SAF to ignore all registries specified by a client through
     *        the initialization properties by setting SAF_REGISTRIES to "default" */
    {
        size_t i, n, nopened=0;
        char *s = NULL;
        char *file_name = NULL;
        SAF_DbProps *reg_props=NULL;

        SAF_GETENV_LIB(s, "SAF_REGISTRIES");

        /* If the environment variable has the value "none" then don't use any registries. */
        if (!s || strcmp(s, "none")) {
            /* Always put the default, minimal, memory registry in the list of reg's to open */
            if (_saf_grow_registries(1)<0)
                SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("Memory allocation error for registry list"));
            n = _SAF_GLOBALS.p.reg.nused++;
            _SAF_GLOBALS.p.reg.name[n] = _saf_strdup(_SAF_GLOBALS.reg.std_name);
            _SAF_GLOBALS.p.reg.db[n] = _SAF_GLOBALS.reg.std_file;

            if (!s || strcmp(s, "default")) {
                if (s) {
                    if (strcmp(s,"implicit")==0) {
                        /* Implicit names, "./Registry.saf", "$HOME/Registry.saf", "$SAF_INSTALL_DATADIR/Registry.saf" */
                        if (_saf_grow_registries(3)<0)
                            SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("Memory allocation error for registry list"));

                        n = _SAF_GLOBALS.p.reg.nused++;
                        _SAF_GLOBALS.p.reg.name[n] = _saf_strdup("./Registry.saf");
                        _SAF_GLOBALS.p.reg.nowarn[n] = SAF_TRISTATE_TRUE;

                        _saf_free(s);

                        SAF_GETENV_LIB(s, "HOME");

                        if (s) {
                            /* this is more portable than getpwuid() */
                            file_name = malloc(strlen(s)+strlen("/Registry.saf")+1);
                            strcpy(file_name, s);
                            strcat(file_name, "/Registry.saf");
                            n = _SAF_GLOBALS.p.reg.nused++;
                            _SAF_GLOBALS.p.reg.name[n] = file_name;
                            _SAF_GLOBALS.p.reg.nowarn[n] = SAF_TRISTATE_TRUE;
                        }
                        _saf_free(s);

                        n = _SAF_GLOBALS.p.reg.nused++;

#ifdef WIN32
                        {
                            char *l_env = getenv("SAF_INSTALL_PATH");
                            if(!l_env) l_env = getenv("SAF_INSTALL_DATADIR");
                            if(l_env) {
                                char *l_wholestring = (char *)malloc(strlen(l_env)+strlen("\\share\\Registry.saf")+1);
                                sprintf(l_wholestring,"%s\\share\\Registry.saf",l_env);
                                _SAF_GLOBALS.p.reg.name[n] = _saf_strdup(l_wholestring);
                                free(l_wholestring);
                            } else {
                                _SAF_GLOBALS.p.reg.name[n] = _saf_strdup("Registry.saf");
                            }
                        }
#else
                        _SAF_GLOBALS.p.reg.name[n] = _saf_strdup(SAF_INSTALL_DATADIR "/Registry.saf");
#endif
                        _SAF_GLOBALS.p.reg.nowarn[n] = SAF_TRISTATE_TRUE;

                    } else {
                        /* The names specified with the environment variable SAF_REGISTRIES */
                        char *saf_registry = _saf_strdup(s);
                        char *tok_start = saf_registry;
                        while ((file_name=strtok(tok_start, ":"))) {
                            tok_start = NULL;
                            if (*file_name) {
                                if (_saf_grow_registries(1)<0)
                                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("Memory allocation error for registry list"));
                                n = _SAF_GLOBALS.p.reg.nused++;
                                _SAF_GLOBALS.p.reg.name[n] = _saf_strdup(file_name);
                            }
                        }
                        _saf_free(saf_registry);
                    }

                }

                if (p->reg.nused>0) {
                    /* registries specified with saf_setProps_Registry() */
                    if (_saf_grow_registries(p->reg.nused)<0)
                        SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("Memory allocation error for registry list"));
                    for (i=0; i<p->reg.nused; i++) {
                        n = _SAF_GLOBALS.p.reg.nused++;
                        _SAF_GLOBALS.p.reg.name[n] = _saf_strdup(p->reg.name[i]);
                        _SAF_GLOBALS.p.reg.nowarn[n] = p->reg.nowarn[i];
                    }
                }
            }

            /* Open the registry files */
            reg_props = saf_createProps_database();
            saf_setProps_DbComm(reg_props, p->LibComm);
            saf_setProps_ReadOnly(reg_props);
            reg_props->NoRegistries = TRUE;

            for (i=0; i<_SAF_GLOBALS.p.reg.nused; i++) {
                SAF_Db *db = _SAF_GLOBALS.p.reg.db[i];
                if (!db) {
                    H5E_BEGIN_TRY {
                        _SAF_GLOBALS.p.DoAbort = FALSE;
                        _SAF_GLOBALS.DontReportErrors = TRUE;
                        db = saf_open_database(_SAF_GLOBALS.p.reg.name[i], reg_props);
                        _SAF_GLOBALS.DontReportErrors = FALSE;
                        _SAF_GLOBALS.p.DoAbort = p->DoAbort;
                    } H5E_END_TRY;
                }
                if (!db && !_SAF_GLOBALS.p.reg.nowarn[i])
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to open SAF registry file \"%s\"", _SAF_GLOBALS.p.reg.name[i]));
                _SAF_GLOBALS.p.reg.db[i] = db;
                if (db) nopened++;
            }
            reg_props = _saf_free(reg_props);

        }

        /* Warn if we didn't open any registries. This is not an error because the applications that produce
         * registries probably don't use any registries themselves. */
        if (!nopened)
            fprintf(stderr, "SAF: no  registries were opened.\n");
    }

    if (!properties) free(p);

    /* register the private version of saf_final with atexit */
    atexit(_saf_final);


    SAF_LEAVE(SAF_SUCCESS);

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience: 	Public
 * Chapter:	Library Initialization
 * Purpose:	Finalize access to the library
 * Concepts:	Initialization
 *
 * Description:	A call to saf_final() terminates the client's interaction with the SAF library. Any open databases and
 *		supplemental files are closed and all memory allocated by the library is freed. Calling this function when the
 *		library is already in a finalized state has no effect. This function should not be called before the library
 *              has been initialized.
 *
 *		This call is mainly just a wrapper for a call to _saf_final() so that we can distinguish between a situation
 *		in which saf_final is called by exit and one in which the client made the call explicitly.
 *
 * Parallel:	This function must be called collectively across all processes in the library's communicator, which was set in
 *		the saf_init() call. Furthermore, the client should not call MPI_Finalize prior to calling saf_final().
 *		SAF does try to detect this condition and report its occurrence before aborting. However, on some platforms,
 *		this is simply not possible and the client might silently hang with no indication as to the cause.
 *
 * Modifications:
 * 		Robb Matzke, 2000-01-24
 *		Added documentation
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
saf_final(void)
{
    SAF_ENTER(saf_final,/*void*/);

   _SAF_GLOBALS.AtExiting = false;
   _saf_final();
   SAF_LEAVE(/*void*/);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience: 	Private	
 * Chapter:	Library Initialization
 * Purpose:	Finalize access to the library
 * Concepts:	Initialization
 *
 * Description:	See description of saf_final(). In addition, _saf_final() makes a determination if it is being called explicitly
 *		by the client or via at_exit. An error message will be generated if SAF is able to determine that MPI_Finalize()
 *		has been called prior to calling _saf_final(). However, it will not attempt to abort in this case as MPI's abort
 *		is probably not available.
 *
 *		Note that if you are using the DSL interface apart from SAF, this call with finalize DSL also. You will need
 *		to re-init DSL's interface.
 *
 * Issue:	We should probably have an argument or property to control whether DSL_finalize is called.
 *
 *
 * Modifications:
 * 		Mark Miller, LLNL, 2000-10-06
 *		Initial implementation
 *
 *              Robb Matzke, LLNL, 2001-04-03
 *              Fixed memory error caused by freeing a null pointer.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
_saf_final(void)
{
   SAF_ENTER(_saf_final, /*void*/);
    
   size_t i;

   if (!_SAF_InitDone)
       SAF_RETURN(/*void*/);

#ifdef HAVE_PARALLEL
   {
      if (_saf_is_mpi_finalized() == SAF_MPI_FINALIZED_TRUE)
      {
	 /* we need to turn off the call to MPI_Abort here becuase MPI is not initialized */
	 _SAF_GLOBALS.p.DoAbort = false;
	 if (_SAF_GLOBALS.AtExiting)
	    SAF_NR_ERROR(SAF_MISC_ERROR,
                         _saf_errmsg("saf_final() called via at_exit after MPI_Finalize has been called")) /* no ; */
	 else
            SAF_NR_ERROR(SAF_MISC_ERROR,
                         _saf_errmsg("saf_final has been called after MPI_Finalize"));
	 _exit(0);
      }
   }
#endif

   /* Close all parts of the registry */

   for (i=0; i<_SAF_GLOBALS.p.reg.nused; i++) {
       if (_SAF_GLOBALS.p.reg.db[i]) {
           saf_close_database(_SAF_GLOBALS.p.reg.db[i]);
           _SAF_GLOBALS.p.reg.db[i] = NULL;
        
       _saf_free(_SAF_GLOBALS.p.reg.name[i]);
       }
   }

   ss_file_close(_SAF_GLOBALS.local_file);

   for (i=0; i<SS_PERS_NCLASSES; i++) {
      _saf_free(_SAF_GLOBALS.key[i]);
   }
   for (i=0; i<SS_MAX_BASEQS; i++) {
      _saf_free(_SAF_GLOBALS.quant[i]);
   }

   _saf_free(_SAF_GLOBALS.p.reg.name);
   _saf_free(_SAF_GLOBALS.p.reg.nowarn);
   _saf_free(_SAF_GLOBALS.p.reg.db);

   _saf_free(_SAF_GLOBALS.algebraic_scalar);
   _saf_free(_SAF_GLOBALS.algebraic_vector);
   _saf_free(_SAF_GLOBALS.algebraic_component);
   _saf_free(_SAF_GLOBALS.algebraic_tensor);
   _saf_free(_SAF_GLOBALS.algebraic_symmetric_tensor);
   _saf_free(_SAF_GLOBALS.algebraic_tuple);
   _saf_free(_SAF_GLOBALS.algebraic_field);

   _saf_free(_SAF_GLOBALS.basis_unity);
   _saf_free(_SAF_GLOBALS.basis_cartesian);
   _saf_free(_SAF_GLOBALS.basis_spherical);
   _saf_free(_SAF_GLOBALS.basis_cylindrical);
   _saf_free(_SAF_GLOBALS.basis_uppertri);
   _saf_free(_SAF_GLOBALS.basis_variable);

   _saf_free(_SAF_GLOBALS.evaluation_constant);
   _saf_free(_SAF_GLOBALS.evaluation_piecewise_constant);
   _saf_free(_SAF_GLOBALS.evaluation_piecewise_linear);
   _saf_free(_SAF_GLOBALS.evaluation_uniform);

   _saf_free(_SAF_GLOBALS.relrep_hslab);
   _saf_free(_SAF_GLOBALS.relrep_tuples);
   _saf_free(_SAF_GLOBALS.relrep_totality);
   _saf_free(_SAF_GLOBALS.relrep_structured);
   _saf_free(_SAF_GLOBALS.relrep_unstructured);
   _saf_free(_SAF_GLOBALS.relrep_arbitrary);

   _saf_free(_SAF_GLOBALS.role_topology);
   _saf_free(_SAF_GLOBALS.role_processor);
   _saf_free(_SAF_GLOBALS.role_block);
   _saf_free(_SAF_GLOBALS.role_domain);
   _saf_free(_SAF_GLOBALS.role_assembly);
   _saf_free(_SAF_GLOBALS.role_material);
   _saf_free(_SAF_GLOBALS.role_space_slice);
   _saf_free(_SAF_GLOBALS.role_param_slice);

   _saf_free(_SAF_GLOBALS.local_file);
   _saf_free(_SAF_GLOBALS.local_scope);
   _saf_free(_SAF_GLOBALS.reg.std_scope);
   _saf_free(_SAF_GLOBALS.reg.std_file);
   _saf_free(_SAF_GLOBALS.reg.std_name);
   _saf_free(_SAF_GLOBALS.find_detect_overflow->name);
   _saf_free(_SAF_GLOBALS.find_detect_overflow->values);
   _saf_free(_SAF_GLOBALS.find_detect_overflow);
   
   /* make a final trace entry */
   if ((_SAF_GLOBALS.p.ErrorLogMode == SAF_ERRLOG_PROCFILE) ||
       ((_SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_PROCFILE) && (_SAF_GLOBALS.Rank == 0)))
   {
      if (_SAF_GLOBALS.p.TraceTimes)
      {
	 double r = _SAF_GLOBALS.CummReadTime;
	 double w = _SAF_GLOBALS.CummWriteTime;
	 double rw = r + w;
	 double tot = _saf_wall_clock(false);
         fprintf(_SAF_GLOBALS.ErrorLogFile,"leaving saf_final [%6.2f wc-secs]\n", tot);
         fprintf(_SAF_GLOBALS.ErrorLogFile,"cumulative time in r/w/r+w = %6.2f/%6.2f/%6.2f wc-secs, %2d/%2d/%2d %% "
	    "of total\n", r, w, rw, (int) (100*r/tot), (int) (100*w/tot), (int) (100*rw/tot));
      }
      else
      {
	 if (_SAF_GLOBALS.p.TraceLogMode != SAF_TRACELOG_NONE)
            fprintf(_SAF_GLOBALS.ErrorLogFile,"leaving saf_final\n");
      }
   }

   /* close any open error logging streams */
   if ((_SAF_GLOBALS.p.ErrorLogMode!=SAF_ERRLOG_NONE && _SAF_GLOBALS.p.ErrorLogMode!=SAF_ERRLOG_STDERR) ||
       (_SAF_GLOBALS.p.ErrorLogMode==SAF_ERRLOG_NONE && _SAF_GLOBALS.p.TraceLogMode!=SAF_TRACELOG_NONE)) {
       fclose(_SAF_GLOBALS.ErrorLogFile);
       _SAF_GLOBALS.ErrorLogFile = NULL;
   }

   /* Shut down lower layers. */
   ss_finalize();
   /* ISSUE: HDF5-1.7.25 doesn't yet have reference counted library initialization and therefore this call to H5close() will
    *        wipe out the HDF5 library even if the SAF client intents to continue using it in some way.  However, if we don't
    *        call H5close() then we also cannot call MPI_Finalize() here. */
   H5close();

#ifdef HAVE_PARALLEL
   MPI_Barrier(_SAF_GLOBALS.p.LibComm);
   MPI_Comm_free(&_SAF_GLOBALS.p.LibComm);
   _SAF_GLOBALS.p.LibComm = MPI_COMM_NULL;
   if (_SAF_GLOBALS.DoMPIFinalize)
   {
      MPI_Finalize();
   }
#endif

   memset(&_SAF_GLOBALS, 0, sizeof _SAF_GLOBALS);
   _SAF_InitDone = false;
   SAF_LEAVE(/*void*/);
}
