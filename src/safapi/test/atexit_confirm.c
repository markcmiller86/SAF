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

/**/
int
main(int argc,
     char **argv)
{
   SAF_ENTER_NOINIT(main, 1);

#ifdef HAVE_PARALLEL
   MPI_Init(&argc,&argv);
#endif

   saf_init(SAF_DEFAULT_LIBPROPS);

   /* since these are tests, we want to see everything as it happens */
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   /* try to create first */
   TESTING("saf_final() cleanup upon at_exit");
   SAF_TRY_BEGIN
   {
      int count, testval[3], *p = &testval[0];
      hid_t type=-1;
      SAF_Db *db1, *db2, *db4, *db6;

      /* try to open the databases */
      db1 = saf_open_database("test_db_1.saf",SAF_DEFAULT_DBPROPS);
      db2 = saf_open_database("test_db_2.saf",SAF_DEFAULT_DBPROPS);
      db4 = saf_open_database("test_db_4.saf",SAF_DEFAULT_DBPROPS);
      db6 = saf_open_database("test_db_6.saf",SAF_DEFAULT_DBPROPS);

      testval[0] = -1; count = 3; type = H5T_NATIVE_INT;
      saf_get_attribute(SAF_ALL, (ss_pers_t*)db1, "atexit-test", &type, &count, (void**)&p);
      if (!H5Tequal(type, H5T_NATIVE_INT) || count!=1 || *p!=1138)
         SAF_ERROR(1,_saf_errmsg("corrupted attribute value"));
      testval[0] = -1; count = 3; type = H5T_NATIVE_INT;
      saf_get_attribute(SAF_ALL, (ss_pers_t*)db2, "atexit-test", &type, &count, (void**)&p);
      if (!H5Tequal(type, H5T_NATIVE_INT) || count!=1 || *p != 1138)
         SAF_ERROR(1,_saf_errmsg("corrupted attribute value"));
      testval[0] = -1; count = 3; type = H5T_NATIVE_INT;
      saf_get_attribute(SAF_ALL, (ss_pers_t*)db4, "atexit-test", &type, &count, (void**)&p);
      if (!H5Tequal(type, H5T_NATIVE_INT) || count!=1 || *p!=1138)
         SAF_ERROR(1,_saf_errmsg("corrupted attribute value"));
      testval[0] = -1; count = 3; type = H5T_NATIVE_INT;
      saf_get_attribute(SAF_ALL, (ss_pers_t*)db6, "atexit-test", &type, &count, (void**)&p);
      if (!H5Tequal(type, H5T_NATIVE_INT) || count!=1 || *p!=1138)
         SAF_ERROR(1,_saf_errmsg("corrupted attribute value"));

      saf_close_database(db1);
      saf_close_database(db2);
      saf_close_database(db4);
      saf_close_database(db6);
      UNLINK("test_db_1.saf");
      UNLINK("test_db_2.saf");
      UNLINK("test_db_3.saf");
      UNLINK("test_db_4.saf");
      UNLINK("test_db_5.saf");
      UNLINK("test_db_6.saf");
      UNLINK("file_000.dsl");
      UNLINK("file_001.dsl");
      UNLINK("file_002.dsl");
      PASSED;
   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
      }
   }
   SAF_TRY_END

   saf_final();

#ifdef HAVE_PARALLEL
   MPI_Finalize();
#endif

   SAF_LEAVE(0);
}
