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

#include <safP.h>
#include <testutil.h>

/* this file gets included in another .c file to create another version of this test with altered behavior.
   We use the value of DO_FINALIZE to specify the behavior. One tests that SAF correctly does at_exit and the
   other tests that SAF correctly detects that MPI_Finalized was called prior to exiting. */
#ifndef DO_FINALIZE
#define DO_FINALIZE	false
#endif

int
main(int argc,
     char **argv)
{
   int do_finalize = DO_FINALIZE;	/* used to test SAF's response to at_exit after MPI_Finalize has been called */
#ifdef HAVE_PARALLEL
   int rank = 0;
#endif


#ifdef HAVE_PARALLEL
   /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass
      the same argc, argv to all processors. However, the MPI spec says nothing about
      what it does or might do to argc or argv. In fact, there is no "const" in the
      function prototypes for either the pointers or the things they're pointing too.
      I would rather pass NULL here and the spec says this is perfectly acceptable.
      However, that too has caused MPICH to core on certain platforms.  */
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

   saf_init(SAF_DEFAULT_LIBPROPS);

   /* since these are tests, we want to see everything as it happens */
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   /* now, test the global db list */
   if (do_finalize)
      TESTING("at_exit after MPI_Finalize (***ERROR OUTPUT IS NORMAL***)\n");
   else
   {
      TESTING("setting up at_exit test");
      if (SAF_RANK(NULL)==0)
         puts("      see below");
   }

   SAF_TRY_BEGIN
   {
      int testval = 1138; /* for THX 1138. I'm a George Lucas fan */
      SAF_Db *db1, *db2, *db3, *db4, *db5, *db6;

      /* clean up potential garbage before we begin */
      UNLINK("test_db_1.saf");
      UNLINK("test_db_2.saf");
      UNLINK("test_db_3.saf");
      UNLINK("test_db_4.saf");
      UNLINK("test_db_5.saf");
      UNLINK("test_db_6.saf");

      /* create several dbs */
      db1 = saf_open_database("test_db_1.saf",SAF_DEFAULT_DBPROPS);
      db2 = saf_open_database("test_db_2.saf",SAF_DEFAULT_DBPROPS);
      db3 = saf_open_database("test_db_3.saf",SAF_DEFAULT_DBPROPS);
      db4 = saf_open_database("test_db_4.saf",SAF_DEFAULT_DBPROPS);
      db5 = saf_open_database("test_db_5.saf",SAF_DEFAULT_DBPROPS);
      db6 = saf_open_database("test_db_6.saf",SAF_DEFAULT_DBPROPS);

      /* now put some attributes in the database which we will inspect
         later for validity */
      saf_put_attribute(SAF_ALL, (ss_pers_t*)db1, "atexit-test", H5T_NATIVE_INT, 1, &testval);
      saf_put_attribute(SAF_ALL, (ss_pers_t*)db2, "atexit-test", H5T_NATIVE_INT, 1, &testval);
      saf_put_attribute(SAF_ALL, (ss_pers_t*)db4, "atexit-test", H5T_NATIVE_INT, 1, &testval);
      saf_put_attribute(SAF_ALL, (ss_pers_t*)db6, "atexit-test", H5T_NATIVE_INT, 1, &testval);

      /* now close two in the middle of this list */
      saf_close_database(db5);
      saf_close_database(db3);

      /* turn on the call to MPIFinalize */
#ifdef HAVE_PARALLEL
      _SAF_GLOBALS.DoMPIFinalize = true;
#endif

      /* ok, we confirm the test passed in atexit_confirm.c */
      
   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
      }
   }
   SAF_TRY_END


#ifdef HAVE_PARALLEL
   if (do_finalize)
   {
      MPI_Finalize();
      _SAF_GLOBALS.DoMPIFinalize = false;
   }
#endif

   exit(0);
/* NOTREACHED */
   return 0;
}
