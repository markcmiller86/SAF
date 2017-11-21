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
#include <testutil.h>

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Test new/renew functions to increase lines of coverage tested.  Also tests describe functions to increase 
 *			loc tested.
 *
 * Description: Tests new/rewnew functions that were previously untested.  Also tests describe functions to incrase
 * 			loc tested.  Note that we are not doing any sanity checking on the calls to saf_new/renew
 * 			we are simply calling saf_new/renew.  But we are checking for correctness on all of the calls
 *			to saf_describe.
 *
 * These tests came out of props.c into their own file, renew.c for better organization of the test clients.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:        Matthew O'Brien, LLNL October 22, 2001 
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
    char dbname[1024];
    SAF_Db *db=NULL;
    SAF_DbProps *dbprops=NULL;
    SAF_LibProps *libprops=NULL;
    int failCount=0;

#ifdef HAVE_PARALLEL
    /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass the same argc, argv to all
     * processors. However, the MPI spec says nothing about what it does or might do to argc or argv. In fact, there is no
     * "const" in the function prototypes for either the pointers or the things they're pointing too.  I would rather pass NULL
     * here and the spec says this is perfectly acceptable.  However, that too has caused MPICH to core on certain platforms. */
    MPI_Init(&argc,&argv);
#endif

    /* process command line args */
    /* since we want to see whats happening make sure stdout and stderr are unbuffered */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    /* for convenience, set working directory to the test file directory */
    chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    libprops = saf_createProps_lib();
    saf_setProps_ErrorMode(libprops,SAF_ERRMODE_THROW);
    saf_init(libprops);
    saf_freeProps_lib(libprops);

    strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {
        /* note: because we are in a try block here, all failures will send us to the one and only
         *       catch block at the end of this test */
        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);

        db = saf_open_database(dbname,dbprops);

        /*-------------------------------------------------------------------------------------------------------*/
        /*------------------------------Describe stuff  ---------------------------------------------------------*/
        /*-------------------------------------------------------------------------------------------------------*/

        /*new, previously untested stuff from algebraic.c--------------------*/
        TESTING("saf_describe_algebraic");
        {
            char *name=NULL, *url=NULL;
            hbool_t indirect;

            saf_describe_algebraic(SAF_ALL, SAF_ALGTYPE_VECTOR, &name, &url, &indirect);
            if (!strcmp(name, "vector")) {
		PASSED;
            } else {
		FAILED;
            }
            free(name);
            free(url);
        }
        /*END new, previously untested stuff from algebraic.c--------------------*/

        /*new, previously untested stuff from basis.c--------------------*/
        {
            char *name=NULL, *url=NULL;
            SAF_Basis ortho;

            TESTING("saf_describe_basis");
            saf_declare_basis(SAF_ALL, db, "ortho", "www.orthoman.org", &ortho);

            saf_describe_basis(SAF_ALL, &ortho, &name, &url);
       
            if (strcmp(name,"ortho") || strcmp(url,"www.orthoman.org")) {
                FAILED;
            } else {
                PASSED;
            }
	
            free(name);
            free(url);
	      
        }
        /*END new, previously untested stuff from basis.c--------------------*/

        /*evaluation.c-------------------------------*/
        {
            SAF_Eval shme;
            char *name=NULL, *url=NULL;

            TESTING("saf_describe_evaluation");
            saf_declare_evaluation(SAF_ALL, db, "shme", "www.bla.com", &shme);

            saf_describe_evaluation(SAF_ALL, &shme, &name, &url);
     

            if (strcmp(name,"shme") || strcmp(url, "www.bla.com")) {
                FAILED;
            } else {
                PASSED;
            }
            free(name);
            free(url);
        }
        /*end evaluation.c---------------------------*/


        /*relrep.c---------------------------*/
        {
            char *name=NULL, *url=NULL;
            int id;
            SAF_RelRep relrep;

            TESTING("saf_describe_relrep");
            saf_declare_relrep(SAF_ALL, db, "relrep", "www.cheesefries.org", 135, &relrep);

            saf_describe_relrep(SAF_ALL, &relrep, &name, &url, &id);
       
            if (strcmp(name,"relrep") || strcmp(url, "www.cheesefries.org")) {
                FAILED;
            } else {
                PASSED;
            }

            free(name);
            free(url);
	      
        }
        /*END relrep.c---------------------------*/

        /*role.c---------------------------*/     
        {
            char *name=NULL, *url=NULL;
            SAF_Role role;

            TESTING("saf_describe_role");
            saf_declare_role(SAF_ALL, db, "role", "www.KnowYourRole.org", &role);

            saf_describe_role(SAF_ALL, &role, &name, &url);
       
            if (strcmp(name,"role") || strcmp(url, "www.KnowYourRole.org")) {
                FAILED;
            } else {
                PASSED;
            }

            free(name);
            free(url);
        }
        /*END role.c---------------------------*/

        /*cat.c---------------------------*/
	{
            char *name=NULL;
            SAF_Role role;
            SAF_Cat cate;
            int dim;

            TESTING("saf_describe_category");

            saf_declare_category(SAF_ALL, db, "my_cat", SAF_TOPOLOGY, 0, &cate);

            saf_describe_category(SAF_ALL, &cate, &name, &role, &dim);

            if (strcmp(name, "my_cat")==0 && dim==0) {
		PASSED;	
            } else {
		FAILED;
            }
        }
        /*END cat.c---------------------------*/

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            failCount += 1;
        }
    } SAF_TRY_END;

    saf_close_database(db);
    saf_final();

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&failCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return (failCount==0)? 0 : 1;

}
