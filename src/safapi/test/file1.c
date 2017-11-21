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
#ifndef WIN32
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <safP.h> 
#include <testutil.h>

#ifdef SSLIB_SUPPORT_PENDING /* Commented out because SSlib doesn't distinguish between a database and a supplemental file. */

#undef TEST_FILE_NAME
#define TEST_FILE_NAME "file1.saf"

/* lets build an exotic type to use a supplemental file attribute */
DSL_TYPE_DEFINE(sfileAttr_t,
                struct {
                    int gorfo;
                    char foo[10];
                    double bar;
                }
                );


/* define the number of files we will declare in the test */
#ifdef HAVE_PARALLEL
#define NUM_TEST_FILES 23
#else
#define NUM_TEST_FILES SAF_MAX_SFILEBLKS * SAF_SFILE_BLKSIZE / 20
#endif

/* Test declare calls */
static int
test_1(void)
{
    SAF_Db              db;
    SAF_File            f1, f2, f3;
    sfileAttr_t         Att1 = {99, "Darth", 3.1415926};
    sfileAttr_t         Att2 = {100, "Vader", 3.1415926};
    sfileAttr_t         Att3 = {101, "Lives", 3.1415926};
    DSL_Boolean_t       failed=false;
    char                *name;
    int                 nerrors=0;
    
    SAF_TRY_BEGIN {
        TESTING("declare files");
        UNLINK(TEST_FILE_NAME);
        UNLINK("file_000.dsl");
        UNLINK("file_001.dsl");
        UNLINK("file_002.dsl");

        db = saf_open_database(TEST_FILE_NAME,SAF_DEFAULT_DBPROPS);

        saf_declare_file(db, "file_000.dsl", &f1);
        saf_declare_file(db, "file_001.dsl", &f2);
        saf_declare_file(db, "file_002.dsl", &f3);
        PASSED;


        TESTING("writing supplemental file attributes");
        saf_put_file_att(f1, "testAtt", sfileAttr_t_(), 1, &Att1);
        saf_put_file_att(f2, "testAtt", sfileAttr_t_(), 1, &Att2);
        saf_put_file_att(f3, "testAtt", sfileAttr_t_(), 1, &Att3);
        PASSED;

        TESTING("reading supplemental file attributes");
        {
            DSL_Type type; 
            int count;
            sfileAttr_t readAtt, *PreadAtt = &readAtt;

            failed = false;

            count = 1;
            type = NULL;
            saf_get_file_att(f1, "testAtt", &type, &count, (void**) &PreadAtt);
            if ((count != 1) || (type != sfileAttr_t_()) || (readAtt.gorfo != 99) || 
                (strcmp(readAtt.foo,"Darth")) || (readAtt.bar != 3.1415926))
                failed = true;

            count = 1;
            type = NULL;
            saf_get_file_att(f2, "testAtt", &type, &count, (void**) &PreadAtt);
            if ((count != 1) || (type != sfileAttr_t_()) || (readAtt.gorfo != 100) || 
                (strcmp(readAtt.foo,"Vader")) || (readAtt.bar != 3.1415926))
                failed = true;

            count = 1;
            type = NULL;
            saf_get_file_att(f3, "testAtt", &type, &count, (void**) &PreadAtt);
            if ((count != 1) || (type != sfileAttr_t_()) || (readAtt.gorfo != 101) || 
                (strcmp(readAtt.foo,"Lives")) || (readAtt.bar != 3.1415926))
                failed = true;

            if (failed)
                FAILED;
            else
                PASSED;
        }

        saf_close_database(db);

        /* cleanup */
        UNLINK(TEST_FILE_NAME);
        UNLINK("file_000.dsl");
        UNLINK("file_001.dsl");
        UNLINK("file_002.dsl");

#ifdef HAVE_PARALLEL
        MPI_Barrier(MPI_COMM_WORLD);
#endif

        TESTING("describe files");
        db = saf_open_database(TEST_FILE_NAME,SAF_DEFAULT_DBPROPS);

        saf_declare_file(db, "file_000.dsl", &f1);
        saf_declare_file(db, "file_001.dsl", &f2);
        saf_declare_file(db, "file_002.dsl", &f3);

        failed = false;
        name = NULL;
        saf_describe_file(f1, &name);
        if (strcmp(name,"file_000.dsl"))
            failed = true;
        free(name); name = NULL;
        saf_describe_file(f2, &name);
        if (strcmp(name,"file_001.dsl"))
            failed = true;
        free(name); name = NULL;
        saf_describe_file(f3, &name);
        if (strcmp(name,"file_002.dsl"))
            failed = true;
        free(name);

        saf_close_database(db);

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        /* cleanup */
        UNLINK(TEST_FILE_NAME);
        UNLINK("file_000.dsl");
        UNLINK("file_001.dsl");
        UNLINK("file_002.dsl");

#ifdef HAVE_PARALLEL
        MPI_Barrier(MPI_COMM_WORLD);
#endif

        /* test flush and suggest close */
        TESTING("file flush");
        db = saf_open_database(TEST_FILE_NAME,SAF_DEFAULT_DBPROPS);

        saf_declare_file(db, "file_000.dsl", &f1);
        saf_declare_file(db, "file_001.dsl", &f2);
        saf_declare_file(db, "file_002.dsl", &f3);

        saf_flush_file(f1);
        saf_flush_file(f3);
        PASSED;

        TESTING("suggest close");
        saf_suggest_close_file(f1);
        saf_suggest_close_file(f2);
        PASSED;

        saf_close_database(db);

        /* cleanup */
        UNLINK(TEST_FILE_NAME);
        UNLINK("file_000.dsl");
        UNLINK("file_001.dsl");
        UNLINK("file_002.dsl");

#ifdef HAVE_PARALLEL
        MPI_Barrier(MPI_COMM_WORLD);
#endif
        
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;
    return nerrors;
}

/**/
static int
test_2(int self, int ntasks)
{
    SAF_Db              db;
    int                 i, j, n=NUM_TEST_FILES, nerrors=0;
    SAF_File            files[NUM_TEST_FILES];
    DSL_File            dslFile;
    const char          *dslFileName=NULL;
    hid_t               id;
    SAF_File            f1, f2;
    SAF_DbProps         dbprops;
    sfileAttr_t         Att1 = {99, "Darth", 3.1415926};
    
#ifdef HAVE_PARALEL
    int                 limit = 50;
#else
    int                 limit = 1000;
#endif
    
    TESTING("Testing lru mechanism");
    UNLINK("file_004.dsl");
    
    SAF_TRY_BEGIN {
        db = saf_open_database(TEST_FILE_NAME,SAF_DEFAULT_DBPROPS);

        /* declare a whole bunch of files */
        for (i = 0; i < n; i++) {
            char theName[1024];

            sprintf(theName,"file_%04d.dsl",i);
            saf_declare_file(db, theName, files + i);
        }

        /* now, try to randomly obtain a file handle */
        for (i=0; i<limit; i++) {

            j = rand() % n;
            _saf_file_handle((SAF_DatabaseObj*)_saf_dereference_handle(db), files[j], &dslFile);
            if (dslFile == NULL) {
                saf_close_database(db);
                FAILED;
                nerrors++;
                fprintf(stderr,"..._saf_file_handle returned NULL DSL_File\n");
                goto jail;
            }
            if ((dslFileName=DSL_nameOf_file(dslFile)) == NULL) {
                saf_close_database(db);
                FAILED;
                nerrors++;
                fprintf(stderr,"..._saf_file_handle returned DSL_File with NULL name\n");
                goto jail;
            }
            if (DSL_lengthOf_string(dslFileName) == 0) {
                saf_close_database(db);
                FAILED;
                nerrors++;
                fprintf(stderr,"..._saf_file_handle returned DSL_File with zero-length name\n");
                goto jail;
            }
            if (not DSL_hdfFileIdOf_file(dslFile,&id)) {
                saf_close_database(db);
                FAILED;
                nerrors++;
                fprintf(stderr,"...can't get HDF id of DSL file returned by _saf_file_handle\n");
                goto jail;
            }
            if (id < 0) {
                saf_close_database(db);
                FAILED;
                nerrors++;
                fprintf(stderr,"..._saf_file_handle returned DSL_File with invalid HDF id\n");
                goto jail;
            }
        }
        PASSED;

        saf_close_database(db);

    jail:
        ;
    }
    SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    UNLINK(TEST_FILE_NAME);
    for (i = 0; i < n; i++) {
        char theName[1024];

        sprintf(theName,"file_%04d.dsl",i);
        UNLINK(theName);
    }

#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

#ifdef WIN32
		/*Skip these until I figure out what is wrong. It is likely
		  the same problem that is in db1.c, involving unlinking open files*/
		TESTING("WARNING: SKIPPING SEVERAL TESTS FOR WIN32");
		SKIPPED;
#else

    SAF_TRY_BEGIN {
        UNLINK(TEST_FILE_NAME);
        TESTING("clobber with supplemental files");

        db = saf_open_database(TEST_FILE_NAME,SAF_DEFAULT_DBPROPS);

        /* create some supplemental files */
        saf_declare_file(db, "file_000.dsl", &f1);
        saf_declare_file(db, "file_001.dsl", &f2);
        saf_declare_file(db, "file_002.dsl", &f2);

        /* close the database */
        saf_close_database(db);

        /* create properties to open for clobber */
        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);

        /* open and close the database */
        db = saf_open_database(TEST_FILE_NAME,dbprops);
        saf_close_database(db);

#ifdef HAVE_PARALLEL
        MPI_Barrier(MPI_COMM_WORLD);
#endif

        /* confirm the supplemental files were clobbered */
        if (SAF_RANK(SAF_EMPTY_HANDLE) == 0) {
        
            if ( (access("file_000.dsl", F_OK) == 0) &&
                 (access("file_001.dsl", F_OK) == 0) &&
                 (access("file_002.dsl", F_OK) == 0) ) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    SAF_TRY_BEGIN {
        UNLINK(TEST_FILE_NAME);
        TESTING("declaring supplemental over existing file (clobber inherited from database)");

        /* set up clobber property for database */
        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);

        /* open database for clobber */
        db = saf_open_database(TEST_FILE_NAME,dbprops);

        /* create an existing file in the filesystem (only proc 0 does this) */
        if (SAF_RANK(SAF_EMPTY_HANDLE) == 0) {
            int fd;
            fd = open("file_000.dsl", O_RDWR|O_TRUNC|O_CREAT,0664);
            close(fd);
        }
        SAF_BARRIER(SAF_EMPTY_HANDLE);

        /* now try to declare a supplement file by the same name */
        saf_declare_file(db, "file_000.dsl", &f1);

        /* close the database */
        saf_close_database(db);
        PASSED;

        /* cleanup */
        UNLINK("file_000.dsl");

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;


    SAF_TRY_BEGIN {
        int numFiles = 0;

        UNLINK(TEST_FILE_NAME);
        TESTING("saf_update_database");

        db = saf_open_database(TEST_FILE_NAME,SAF_DEFAULT_DBPROPS);
        saf_declare_file(db, "file_000.dsl", &f1);
        saf_update_database(db);
        saf_declare_file(db, "file_001.dsl", &f2);
        saf_update_database(db);
        saf_declare_file(db, "file_002.dsl", &f2);
        saf_close_database(db);
        db = saf_open_database(TEST_FILE_NAME,SAF_DEFAULT_DBPROPS);
        saf_find_files(db, SAF_ANY_NAME, &numFiles, NULL);
        if (numFiles != 3) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        TESTING("dsl grab and write");
        {
            DSL_Boolean_t testFailed = false;
            DSL_Dataset d;
            DSL_Offset start, end;

            saf_grab_dsl(SAF_MASTER(db), &dslFile);
            if (dslFile == NULL)
                goto dslGrabFailed;
            d = DSL_create1D_dataset(dslFile, "saf_grabbed_dsl", NULL, DSL_INT, (hsize_t)ntasks); 
            if (d == NULL)
                goto dslGrabFailed;
            if (DSL_writeSingleBuffer1D_dataset(d, (hssize_t)self, 1, DSL_INT, &self, &start, &end)<0)
                goto dslGrabFailed;
            if (start != self || end != self)
                goto dslGrabFailed;
            DSL_close_dataset(d);
            goto dslGrabOk;

        dslGrabFailed:
            testFailed = true;
        dslGrabOk:
            saf_ungrab_dsl(dslFile);
            if (testFailed)
                FAILED;
            else
                PASSED;
        }

        TESTING("attempt SAF calls while dsl grab is in effect");
        {

            /* turn off error reporting and abort */
            _SAF_GLOBALS.DontReportErrors = TRUE;
            _SAF_GLOBALS.DoAbort = false;
            saf_grab_dsl(SAF_MASTER(db), &dslFile);

            SAF_TRY_BEGIN {
                /* some arbitrary SAF calls that better not succeed */
                saf_declare_file(db, "file_004.dsl", &f1);
                saf_put_file_att(SAF_MASTER(db), "testAtt", sfileAttr_t_(), 1, &Att1);
                FAILED;
            } SAF_CATCH {
                SAF_CATCH_ALL {
                    saf_ungrab_dsl(dslFile);
                    PASSED;
                }
            } SAF_TRY_END;

            /* turn it all back on */
            _SAF_GLOBALS.DontReportErrors = FALSE;
            _SAF_GLOBALS.DoAbort = true;
        }

        TESTING("ungrab dsl");
        saf_grab_dsl(SAF_MASTER(db), &dslFile);
        saf_ungrab_dsl(dslFile);

        /* some arbitrary SAF calls that now better succeed */
        saf_declare_file(db, "file_004.dsl", &f1);
        saf_put_file_att(SAF_MASTER(db), "testAtt", sfileAttr_t_(), 1, &Att1);
        PASSED;

        TESTING("hdf5 grab and read");
        {
            /* This test used to consist of HDF5 calls to open and read the dataset that was created and written with DSL calls
             * above. However, DSL now performs various types of caching in order to try to overlap writes from different tasks,
             * which means that we can't assume the data is even available for HDF5.  Therefore, all we will do is grab the HDF5
         * handle, do something with it, and ungrab. */
            DSL_Boolean_t testFailed = false;
            hid_t dsid;
            H5G_stat_t sb;

            saf_grab_hdf5(SAF_MASTER(db), &id);
            if ((dsid=H5Dopen(id, "saf_grabbed_dsl"))<0) goto hdf5GrabFailed;
            if (H5Gget_objinfo(dsid, ".", true, &sb)<0) goto hdf5GrabFailed;
            if (H5Dclose(dsid)<0) goto hdf5GrabFailed;
            goto hdf5GrabOk;

        hdf5GrabFailed:
            testFailed = true;
        hdf5GrabOk:
            saf_ungrab_hdf5(id);
            if (testFailed)
                FAILED;
            else
                PASSED;

        }
        saf_close_database(db);
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

#endif /*WIN32*/


    return nerrors;
}
#endif /*SSLIB_SUPPORT_PENDING*/

    


/**/
int
main(int argc, char *argv[])
{
    int         self=0, ntasks=1, nerrors=0;
    
#ifdef HAVE_PARALLEL
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &self);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
#endif

    /* since these are tests, we want to see everything as it happens */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

#ifdef SSLIB_SUPPORT_PENDING
    if (2==argc && !strcmp(argv[1], "default")) {
        /* Tests using default library properties */
        saf_init(SAF_DEFAULT_LIBPROPS);
        nerrors = test_1();
    } else if (2==argc && !strcmp(argv[1], "limited")) {
        /* Test the lru mechanism */
        SAF_LibProps *libprops = saf_createProps_lib();
        saf_setProps_MaxOpenFiles(libprops,10);
        saf_init(libprops);
        saf_freeProps_lib(libprops);
        nerrors = test_2(self, ntasks);
    } else {
        if (0==self) fprintf(stderr, "usage: %s default|limited\n", argv[0]);
#ifdef HAVE_PARALLEL
        MPI_Finalize();
#endif
        exit(1);
    }
#else
    saf_init(SAF_DEFAULT_LIBPROPS);
    TESTING("Supplemental files");
    SKIPPED;
#endif /*SSLIB_SUPPORT_PENDING*/

#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    saf_final();

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif
    return nerrors?1:0;
}
