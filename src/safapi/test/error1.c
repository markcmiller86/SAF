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

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#include <stdio.h>
#include <safP.h>
#include <testutil.h>

/*returns number of errors*/
static int
test_none_mode(void)
{
    int         retval=0;
    SAF_LibProps *plib = saf_createProps_lib();
    saf_setProps_ErrorLogging(plib, "none");
    
    saf_init(plib);
    SAF_ENTER(test_none_mode, 1);
    _SAF_GLOBALS.p.DoAbort = false;
    
    /* test the none mode */
#ifdef WIN32
	/*XXX This is a strange microsoft visual c++ build feature: simply
	sending the word "error" to stdout seems to cause the post-build batch
	steps to signal failure. I have not yet found documentation for this
	feature, so I dont yet know how to turn it off. So, for now, dont send
	the word "error" to stdout*/
    TESTING("\"none\" err logging mode");
#else
    TESTING("\"none\" error logging mode");
#endif
    SAF_TRY_BEGIN {
        SAF_NR_ERROR(SAF_MISC_ERROR, _saf_errmsg("testing the \"none\" error logging mode"));
    } SAF_TRY_END;
    SAF_BARRIER(NULL);
    
    /* how do we confirm that no output was generated anywhere? */
    if (_SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_NONE) {
        FAILED;
        retval = 1;
    } else {
        PASSED;
    }
    SAF_LEAVE(retval);
}

/**/
static int
test_stderr_mode(void)
{
    int         retval=0;
    SAF_LibProps *plib = saf_createProps_lib();
    saf_setProps_ErrorLogging(plib, "stderr");

    saf_init(plib);
    SAF_ENTER(test_stderr_mode, 1);
    _SAF_GLOBALS.p.DoAbort = false;
    
    /* you *must* use "a" mode or parallel output won't work */
    freopen("saf_stderr.log", "a", stderr);
#ifdef WIN32
	/*XXX This is a strange microsoft visual c++ build feature: simply
	sending the word "error" to stdout seems to cause the post-build batch
	steps to signal failure. I have not yet found documentation for this
	feature, so I dont yet know how to turn it off. So, for now, dont send
	the word "error" to stdout*/
    TESTING("\"stderr\" err logging mode");
#else
    TESTING("\"stderr\" error logging mode");
#endif
    SAF_TRY_BEGIN {
        SAF_NR_ERROR(SAF_MISC_ERROR, _saf_errmsg("testing the \"stderr\" error logging mode"));
    } SAF_TRY_END;
    SAF_BARRIER(NULL);
    UNLINK("saf_stderr.log");
   
    /* how do we confirm the correct error message appeared on stderr? */
    if (_SAF_GLOBALS.p.ErrorLogMode == SAF_ERRLOG_STDERR) {
        PASSED;
    } else {
        FAILED;
        retval = 1;
    }

    SAF_LEAVE(retval);
}

/**/
static int
test_file_mode(void)
{
    int         retval=0;
    SAF_LibProps *plib = saf_createProps_lib();
    saf_setProps_ErrorLogging(plib, "file:saf_errors.log");
    
    saf_init(plib);
    SAF_ENTER(test_file_mode, 1);
    _SAF_GLOBALS.p.DoAbort = false;
    
#ifdef WIN32
	/*XXX This is a strange microsoft visual c++ build feature: simply
	sending the word "error" to stdout seems to cause the post-build batch
	steps to signal failure. I have not yet found documentation for this
	feature, so I dont yet know how to turn it off. So, for now, dont send
	the word "error" to stdout*/
    TESTING("\"file\" err logging mode");
#else
    TESTING("\"file\" error logging mode");
#endif
    SAF_TRY_BEGIN {
        SAF_NR_ERROR(SAF_MISC_ERROR, _saf_errmsg("testing the \"file\" error logging mode [10]"));
    } SAF_TRY_END;
    SAF_BARRIER(NULL);
    UNLINK(_SAF_GLOBALS.p.ErrorLogName);
    if (_SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_FILE) {
        FAILED;
        retval = 1;
    } else {
        PASSED;
    }
    SAF_LEAVE(retval);
}

/**/
static int
test_proc_file(void)
{
    int         retval=0;
    SAF_LibProps *plib = saf_createProps_lib();
    saf_setProps_ErrorLogging(plib, "procfile:saf_errors_,%03d,.log");

    saf_init(plib);
    SAF_ENTER(test_proc_file, 1);
    _SAF_GLOBALS.p.DoAbort = false;
    
#ifdef WIN32
	/*XXX This is a strange microsoft visual c++ build feature: simply
	sending the word "error" to stdout seems to cause the post-build batch
	steps to signal failure. I have not yet found documentation for this
	feature, so I dont yet know how to turn it off. So, for now, dont send
	the word "error" to stdout*/
    TESTING("\"procfile\" err logging mode");
#else
    TESTING("\"procfile\" error logging mode");
#endif
    SAF_TRY_BEGIN {
        SAF_NR_ERROR(SAF_MISC_ERROR, _saf_errmsg("testing the \"procfile\" error logging mode [10]"));
    } SAF_TRY_END;
    SAF_BARRIER(NULL);
    unlink(_SAF_GLOBALS.p.ErrorLogName); /* we use lower-case version (e.g. all procs do it) because each proc has a logfile */
#ifdef HAVE_PARALLEL
    if (_SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_PROCFILE) {
        FAILED;
        retval = 1;
    }
#else
    if (_SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_FILE) {
        FAILED;
        retval = 1;
    }
#endif
    if (!retval) PASSED;
    SAF_LEAVE(retval);
}

/**/
static int
test_seg_file(void)
{
    int         retval=0;
    SAF_LibProps *plib = saf_createProps_lib();
    saf_setProps_ErrorLogging(plib, "segfile:saf_errors_,10,80,.log");

    saf_init(plib);
    SAF_ENTER(test_seq_file, 1);
    _SAF_GLOBALS.p.DoAbort = false;

#ifdef WIN32
	/*XXX This is a strange microsoft visual c++ build feature: simply
	sending the word "error" to stdout seems to cause the post-build batch
	steps to signal failure. I have not yet found documentation for this
	feature, so I dont yet know how to turn it off. So, for now, dont send
	the word "error" to stdout*/
    TESTING("\"segfile\" err logging mode");
#else
    TESTING("\"segfile\" error logging mode");
#endif
    SAF_TRY_BEGIN {
        SAF_NR_ERROR(SAF_MISC_ERROR, _saf_errmsg("testing the \"segfile\" error logging mode [10]"));
    } SAF_TRY_END;
    SAF_BARRIER(NULL);
    UNLINK(_SAF_GLOBALS.p.ErrorLogName);
#ifdef HAVE_PARALLEL
    if (_SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_SEGFILE) {
        FAILED;
        retval = 1;
    }
#else
    if (_SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_FILE) {
        FAILED;
        retval = 1;
    }
#endif
    if (!retval) PASSED;
    SAF_LEAVE(retval);
}

/* Many of DSL's internal functions are written with the assumption that they are initialized only once, which causes problems
 * when the software stack is "finalized" and then reinitialized as when calling saf_init() after saf_final(). Therefore this
 * test now only does one thing at a time. --rpm 2002-09-23 */
int
main(int argc, char *argv[])
{
    int self=0, nerrors;
    static const char *usage_fmt = "usage: %s [none|stderr|file|procfile|segfile]\n";

#ifdef HAVE_PARALLEL
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &self);
#endif

    if (2!=argc) {
        if (0==self) fprintf(stderr, usage_fmt, argv[0]);
        exit(1);
    } else if (!strcmp(argv[1], "none")) {
        nerrors = test_none_mode();
    } else if (!strcmp(argv[1], "stderr")) {
        nerrors = test_stderr_mode();
    } else if (!strcmp(argv[1], "file")) {
        nerrors = test_file_mode();
    } else if (!strcmp(argv[1], "procfile")) {
        nerrors = test_proc_file();
    } else if (!strcmp(argv[1], "segfile")) {
        nerrors = test_seg_file();
    } else {
        if (0==self) fprintf(stderr, usage_fmt, argv[0]);
        exit(1);
    }

   saf_final();

#ifdef HAVE_PARALLEL
   /* make sure everyone returns the same error status */
   MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Finalize();
#endif
   return nerrors;
}
