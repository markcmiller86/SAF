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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <safP.h>
#include <testutil.h>

int
main(int argc,
     char **argv)
{
   char dbname[1024];
   struct stat statbuf1, statbuf2;
   int fd, nerrors=0;
   SAF_Db *db=NULL;
   SAF_DbProps *dbprops;

#ifdef HAVE_PARALLEL
   MPI_Init(&argc,&argv);
#endif



   saf_init(SAF_DEFAULT_LIBPROPS);

   /* create the props we'll throughout use for clobbering */
   dbprops = saf_createProps_database();
   saf_setProps_Clobber(dbprops);

   /* since these are tests, we want to see everything as it happens */
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   /* create the database name */
   sprintf(dbname,"%s/%s",TEST_FILE_PATH,TEST_FILE_NAME);

   /* make sure a file by this name does not exist already */
   UNLINK(dbname);

   /* try to create first */
   TESTING("open/close");
   SAF_TRY_BEGIN
   {
      db = saf_open_database(dbname,SAF_DEFAULT_DBPROPS);
      saf_close_database(db);
      PASSED;
   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
         nerrors++;
      }
   }
   SAF_TRY_END

   /* now, test open for read-only */
   TESTING("open for read-only");
   SAF_TRY_BEGIN
   {

      /* by chmod'ing the file to 400, we can test whether the read-only property
         is indeed working. We should not be able to succeed in opening a 400 file
	 without opening it for read-only */

#ifndef WIN32
      CHMOD(dbname, S_IRUSR);
#else
      CHMOD(dbname, S_IREAD);
#endif
      dbprops = saf_createProps_database();
      saf_setProps_ReadOnly(dbprops);
      db = saf_open_database(dbname,dbprops);
      saf_close_database(db);
      PASSED;
#ifndef WIN32
      CHMOD(dbname, S_IRUSR|S_IWUSR); /* so the unlink will succeed later */
#else
      CHMOD(dbname, S_IREAD|S_IWRITE);
#endif

   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
         nerrors++;
      }
   }
   SAF_TRY_END

   /* remove the test file */
   UNLINK(dbname);

   /* now, test open for clobber */
   TESTING("clobber");



#ifdef WIN32
   /*There is a bug with WIN32 when the same filename is used,
     related to being unable to truly unlink the file because
	 it is still being held onto by saf or hdf5. For now, do 
	 this test by using a new filename*/
      sprintf(dbname,"%s/%s",TEST_FILE_PATH,"test_saf_win32.saf");
#endif


   SAF_TRY_BEGIN
   {
      /* make a temporary file of 1 byte to attempt open for clobbering */
      if (SAF_RANK(NULL) == 0)
      {  char buf = '\0';

#ifndef WIN32
         fd = open(dbname,O_CREAT|O_TRUNC|O_WRONLY,0644);
#else
         fd = open(dbname,O_CREAT|O_TRUNC|O_WRONLY,S_IWRITE);/*note: 3rd arg is ignored if file exists*/
#endif

         write(fd,&buf,1);
         close(fd);
      }
      STAT(dbname, &statbuf1);

#ifndef WIN32
      sleep(1);
#else
	  Sleep(2500);/*(in milliseconds) Using 2.5 seconds instead of 1 sec
					because stat sometimes returns the same time for a 1
					sec difference?*/
#endif

      /* ok, now open with clobber turned on (should create a new file) */
      dbprops = saf_createProps_database();
      saf_setProps_Clobber(dbprops);
      db = saf_open_database(dbname,dbprops);
      saf_close_database(db);
      STAT(dbname, &statbuf2);

      /* now, check the file size and modification time to confirm it was clobbered */
      if (SAF_RANK(NULL) == 0 && ((statbuf2.st_mtime > statbuf1.st_mtime &&
						 statbuf2.st_size > statbuf1.st_size)))
         PASSED;
      else
      {
         FAILED;
         nerrors++;
      }
   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
         nerrors++;
      }
   }
   SAF_TRY_END

   /* remove the test file */
   UNLINK(dbname);



#ifdef WIN32
   /*Return to the original filename*/
   sprintf(dbname,"%s/%s",TEST_FILE_PATH,TEST_FILE_NAME);
#endif






   /* test open for no clobber */
   TESTING("no clobber");
   SAF_TRY_BEGIN
   {
      /* first, create an emtpy database */
      db = saf_open_database(dbname,SAF_DEFAULT_DBPROPS);
      saf_close_database(db);
      STAT(dbname, &statbuf1);

      /* now, re-open it */
      db = saf_open_database(dbname,SAF_DEFAULT_DBPROPS);
      /* we do the stat here because we know the close will re-write VBT's tables. We just
	 want to make sure the open didn't blow it away */
      STAT(dbname, &statbuf2);
      saf_close_database(db);

      /* confirm file did not change */
      if ((SAF_RANK(NULL) == 0) && (statbuf1.st_mtime != statbuf2.st_mtime))
      {
          FAILED;
          nerrors++;
      }
      else
         PASSED;

   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
         nerrors++;
      }
   }
   SAF_TRY_END

   /* remove the test file */
   UNLINK(dbname);


   /* now, test the global db list */
   TESTING("saf_final() cleanup");
#if 1
   /* Does not work because DSL cannot reinitialize properly due to a design issue --rpm 2002-09-23 */
   /* Does not work because SSlib prohibits calling ss_init() more than once per run --rpm 2004-04-14 */
   SKIPPED;
#else
   SAF_TRY_BEGIN
   {
      SAF_Db *db1, *db2, *db3;

      /* get rid of any garbage before we begin */
      UNLINK("test_db_1.saf");
      UNLINK("test_db_2.saf");
      UNLINK("test_db_3.saf");

      /* create several dbs */
      db1 = saf_open_database("test_db_1.saf",SAF_DEFAULT_DBPROPS);
      db2 = saf_open_database("test_db_2.saf",SAF_DEFAULT_DBPROPS);
      db3 = saf_open_database("test_db_3.saf",SAF_DEFAULT_DBPROPS);

      /* now close one in the middle of this list */
      saf_close_database(db2);

      /* now call final */
      saf_final();

      /* now, re-init the library and try to open some of those that should
	 have been properly closed by saf_final. Any errors will send us to catch block. */
      saf_init(SAF_DEFAULT_LIBPROPS);
      db1 = saf_open_database("test_db_1.saf",SAF_DEFAULT_DBPROPS);
      db3 = saf_open_database("test_db_3.saf",SAF_DEFAULT_DBPROPS);
      saf_close_database(db1);
      saf_close_database(db3);

      PASSED;

   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
         nerrors++;
      }
   }
   SAF_TRY_END

   /* cleanup the test files */
   if (SAF_RANK(NULL)==0)
   {
      unlink("test_db_1.saf");
      unlink("test_db_2.saf");
      unlink("test_db_3.saf");
      unlink("test_db_4.saf");
      unlink("test_db_5.saf");
      unlink("test_db_6.saf");
   }
#ifdef HAVE_PARALLEL
   MPI_Barrier(MPI_COMM_WORLD);
#endif
#endif


   TESTING("put attribute");
   SAF_TRY_BEGIN
   {
      int val = 1138;

      db = saf_open_database(dbname,SAF_DEFAULT_DBPROPS);
      saf_put_attribute(SAF_ALL, (ss_pers_t*)db, "attribute1", H5T_NATIVE_INT, 1, &val);
      saf_put_attribute(SAF_ALL, (ss_pers_t*)db, ".read-only-attr", H5T_NATIVE_INT, 1, &val);
      saf_close_database(db);

      PASSED;
   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
         nerrors++;
      }
   }
   SAF_TRY_END

   /* test some attribute put/gets */
   TESTING("get attribute");
   SAF_TRY_BEGIN
   {
      int count=1, *val = NULL;
      hid_t type = H5T_NATIVE_INT;

      db = saf_open_database(dbname,SAF_DEFAULT_DBPROPS);
      saf_get_attribute(SAF_ALL, (ss_pers_t*)db, "attribute1", &type, &count, (void**)&val);

      /* keep the db open for next test of put attributes */

      if (!H5Tequal(type, H5T_NATIVE_INT) || (count != 1) || (*val != 1138))
      {
         FAILED;
         nerrors++;
      }
      else
         PASSED;
      free(val);
   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
         FAILED;
         nerrors++;
      }
   }
   SAF_TRY_END

   /* note: put attributes happen in saf_open_database() call. So, if open succeeds,
      saf_put_attributes must have succeeded. All we test here is the put into a read-only. */

   /* Turn off error reporting for this test.  We expect to get an error if the
    * test passes.  Seeing the error message when the test actually passed may
    * be confusing. */
   _SAF_GLOBALS.DontReportErrors = TRUE;
   _SAF_GLOBALS.p.DoAbort = false;

   /* test some attribute put/gets */
   TESTING("overwrite read-only attribute");
#if 1 /* SSlib doesn't support read-only attributes except when the attribute's scope is read-only */
   saf_close_database(db);
   SKIPPED;
#else
   SAF_TRY_BEGIN
   {
      int temp = 1138+10;

      saf_put_attribute(SAF_ALL, (ss_pers_t*)db, ".read-only-attr", H5T_NATIVE_INT, 1, &temp);
      saf_close_database(db);

      /* if we don't get an error, it actually fails this test */
      FAILED;
      nerrors++;
   }
   SAF_CATCH
   {
      SAF_CATCH_ALL
      {
	 /* if we get an error, it actually passes this test */
         PASSED;
      }
   }
   SAF_TRY_END
#endif

   /* Turn error reporting back on. */
   _SAF_GLOBALS.DontReportErrors = FALSE;
   _SAF_GLOBALS.p.DoAbort = true;

   /* remove the test file .... DONT REMOVE IT: THE NEXT TEST USES IT*/
   /*UNLINK(dbname);*/

   /* finalize saf */
   saf_final();

#ifdef HAVE_PARALLEL
   /* make sure everyone returns the same error status */
   MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Finalize();
#endif

   return nerrors?1:0;
}
