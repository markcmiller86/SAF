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
#include <sys/types.h>
#include <sys/stat.h>
#include <safP.h>
#include <testutil.h>

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Test miscellaneous functions to increase lines of coverage tested. 
 *
 * Description: Tests functions that were previously untested.
 *
 *
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
    int failCount=0;
    char dbname[1024];
    SAF_Db *db=NULL;
    SAF_DbProps *dbprops=NULL;
    SAF_LibProps *libprops=NULL;
    SAF_Set *null_set=NULL;
    SAF_Set top=SS_SET_NULL, cell_1=SS_SET_NULL;
    SAF_Cat nodes=SS_CAT_NULL, elems=SS_CAT_NULL, tris=SS_CAT_NULL;
    SAF_Rel rel=SS_REL_NULL, newtrel=SS_REL_NULL;
    SAF_Db *topofile=NULL;
    hbool_t failed=false;

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

    SAF_TRY_BEGIN  {
        /* note: because we are in a try block here, all failures will send us to the one and only
         *       catch block at the end of this test */
        failed=false;
        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);
        db = saf_open_database(dbname,dbprops);
        topofile = db;

        TESTING("SAF_NULL_SET");
        null_set=SAF_NULL_SET(db);
        if (_saf_is_null(null_set)) {
            PASSED;
        } else {
            FAILED;
        }

        TESTING("SAF_NULL_*");
        (void)SAF_NULL_SET(db);
        (void)SAF_NULL_REL(db);
        (void)SAF_NULL_FTMPL(db);
        (void)SAF_NULL_FIELD(db);
        (void)SAF_NULL_STMPL(db);
        (void)SAF_NULL_SUITE(db);

        /*we can't test these functions because there are no is_null_xxx tests*/
        PASSED;
     
        TESTING("_saf_is_mpi_finalized()");
        {
            SAF_MPIFinalizeMode ret;
            ret=_saf_is_mpi_finalized();
            if (ret==SAF_MPI_FINALIZED_FALSE) {
                PASSED;
            } else {
                FAILED;
            }
        }

        /*declare some sets-----------------*/
        saf_declare_set(SAF_ALL, db, "TOP_CELL", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &top);
        saf_declare_set(SAF_ALL, db, "CELL_1", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_1);

        /*declare some categories-----------*/
        saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, 0, &nodes);
        saf_declare_category(SAF_ALL, db, "elems", SAF_TOPOLOGY, 2, &elems);
        saf_declare_category(SAF_ALL, db, "tris", SAF_TOPOLOGY, 2, &tris);

        /* here we use SAF_CELLTYPE_SET because this is for a "non-primitive" collection which is what is required if you pass
         * a non-null argument for the last argument, member_sets.  This was done to exercise more of the code */
        saf_declare_collection(SAF_ALL, &top, &nodes, SAF_CELLTYPE_SET, 12, SAF_1DC(12), SAF_DECOMP_TRUE);
        /* here we are testing with Fortran indexing scheme */
        saf_declare_collection(SAF_ALL, &top, &elems, SAF_CELLTYPE_SET, 10, SAF_1DF(10), SAF_DECOMP_TRUE);
        /* here we are testing with a made up indexing scheme */
        saf_declare_collection(SAF_ALL, &top, &tris, SAF_CELLTYPE_TRI, 64, 
                               _saf_indexspec(3, 4,4,4, 0,0,0, 0,2,1), SAF_DECOMP_TRUE); 

        saf_declare_collection(SAF_ALL, &cell_1, &tris, SAF_CELLTYPE_POINT, 9, SAF_1DC(9), SAF_DECOMP_FALSE);

        TESTING("saf_write_subset_relation");
        {
            int buf[] = {0,0,0,  0,0,1,  0,0,2,
                         1,0,0,  1,0,1,  1,0,2,
                         2,0,0,  2,0,1,  2,0,2};
            int *readbuf=NULL;
            size_t count,i;
            hid_t atype;

            saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&tris), SAF_TUPLES, SAF_INT, NULL,
                                        H5I_INVALID_HID, NULL, &rel);
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
            saf_get_count_and_type_for_subset_relation(SAF_ALL,&rel, NULL, &count,&atype,NULL,NULL);
            readbuf=(int *)malloc(count*sizeof(int));

            if (!H5Tequal(atype, H5T_NATIVE_INT) || count !=3*9) {
                failed=true;
            } else {
                saf_read_subset_relation(SAF_ALL,&rel,NULL, (void **)&readbuf,NULL);
                failed=false;
                for (i=0; i<count; i++) {
                    if (buf[i]!=readbuf[i])
                        failed=true;
                }
            }

            if (failed)
                FAILED;
            else
                PASSED;
        }

        {
#define N 32
            int vec[N],i;
            for (i=0; i<N; i++) {
                vec[i]= ((~i) & (N-1));
            }
            TESTING("_saf_is_permutation");
            if (! _saf_is_permutation(N, vec)) {
                FAILED;
            } else {
                PASSED;
            }
#undef N
        }

        TESTING("_saf_set_disable_threshold");
        {
            int nat;
            _saf_set_disable_threshold("SAF_DISABLE_THRESHOLD", &nat);
            /*	printf("nat %i\n", nat);*/
        }
        PASSED;
     
        /*end testing previously untested functions from utils.c----------*/

        /*new, previously untested stuff from db.c--------------------*/
        {
            TESTING("db.c stuff");
            saf_update_database(db);
            PASSED;

        }
        /*END new, previously untested stuff from db.c--------------------*/

        /*error.c-------------------------------*/
        TESTING("saf_error_str()");
        if (strcmp("", saf_error_str())==0)
            PASSED;
        else
            FAILED;
        /*end error.c---------------------------*/

        {
            SAF_Field *Pcoord_fields=calloc(4, sizeof(SAF_Field)), coords;
            SAF_FieldTmpl ftmpl;
            SAF_Unit usec;
            char *name=NULL;
            int num_coord_fields;

            TESTING("saf_find_coords");
            num_coord_fields=4;

            saf_find_one_unit(db, "second", &usec);
            saf_declare_field_tmpl(SAF_ALL, db, "field_tmpl", SAF_ALGTYPE_SCALAR, NULL, SAF_QTIME, 1, NULL, &ftmpl);

            saf_declare_field(SAF_ALL, db, &ftmpl, "field", &top, &usec, SAF_SELF(db),
                              &nodes,1,&nodes, SAF_SPACE_PWCONST, 
                              H5T_NATIVE_FLOAT,NULL, SAF_INTERLEAVE_NONE,NULL,NULL,&coords);

            saf_declare_coords(SAF_ALL, &coords);
            saf_find_coords(SAF_ALL, db, &top, &num_coord_fields, &Pcoord_fields);

            failed=false;
            if(num_coord_fields!=1){ failed=true; }
	
            saf_describe_field(SAF_ALL, Pcoord_fields+0, NULL, &name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               NULL, NULL, NULL, NULL);

            if(strcmp(name, "field")){ failed=true; }

            free(name); name=NULL;
            free(Pcoord_fields);
            Pcoord_fields=calloc(4, sizeof(*Pcoord_fields));
            num_coord_fields=4;

            saf_find_coords(SAF_ALL, db, SAF_UNIVERSE(db), &num_coord_fields, &Pcoord_fields);
            if (num_coord_fields!=1) failed=true;
	
            saf_describe_field(SAF_ALL, Pcoord_fields+0, NULL, &name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               NULL, NULL, NULL, NULL);

            if(strcmp(name, "field")){ failed=true; }

            free(Pcoord_fields);
            free(name);
	
            if(failed)
                FAILED;
            else
                PASSED;
        }
        /*end testing field.c newly tested stuff-----------------------------*/

        /*new*******************************************/

        /*----------------------testing coll.c stuff------------*/
        TESTING("saf_same_collections");
        if(saf_same_collections(&top, &nodes, &top, &elems) ||
           saf_same_collections(&top, &nodes, &cell_1, &elems) ||
           !saf_same_collections(&top, &nodes, &top, &nodes)){
            FAILED;
        }else{
            PASSED;
        }
        /*------------------end testing coll.c stuff------------*/

        /*-------------------------------------------------------------------*/
        /*new testing, where abuf!=NULL and bbuf!=NULL withing saf_declare_topo_relation*/
        /*the point is just to exercise more of the library*/
        TESTING("saf_declare_topo_relation and saf_get_count_and_type_for_topo_relation");
        {
            long arbbuf[] = {4,4,4,4, 4,4,4,4, 4,4,4,4,  4,4,4,4};
            long bbuf[] =   {1,2,6,5, 2,3,7,6, 5,6,10,9, 6,7,11,10};
            long abuf[] = {4};
            hid_t a_type, b_type;
            size_t a_count, b_count;

            failed=false;
            saf_declare_collection(SAF_ALL, &cell_1, &elems, SAF_CELLTYPE_POINT, 12, SAF_1DC(12), SAF_DECOMP_FALSE);

            saf_declare_topo_relation(SAF_ALL, db, &cell_1, &elems, &top, &nodes, SAF_SELF(db), &cell_1,
                                      SAF_ARBITRARY, H5T_NATIVE_LONG, arbbuf, H5T_NATIVE_LONG, bbuf, &newtrel);

            a_count=0; b_count=0;
            a_type=H5I_INVALID_HID; b_type=H5I_INVALID_HID;
            saf_get_count_and_type_for_topo_relation(SAF_ALL, &newtrel, NULL, NULL, &a_count, &a_type,&b_count, &b_type); 	
            if(!H5Tequal(a_type, H5T_NATIVE_LONG) || !H5Tequal(b_type, H5T_NATIVE_LONG)) failed=true;      

            saf_declare_topo_relation(SAF_ALL, db, &cell_1, &elems, &top, &nodes, SAF_SELF(db), &cell_1,
                                      SAF_UNSTRUCTURED, H5T_NATIVE_LONG, abuf, H5T_NATIVE_LONG, bbuf, &newtrel);

        }
        {
            int arbbuf[] = {4,4,4,4};
            int abuf[] = {4};
            int bbuf[] = {1,2,6,5,2,3,7,6,5,6,10,9,6,7,11,10};
            saf_declare_topo_relation(SAF_ALL, db, &cell_1, &elems, &top, &nodes, SAF_SELF(db), &cell_1,
                                      SAF_ARBITRARY, H5T_NATIVE_INT, arbbuf, H5T_NATIVE_INT, bbuf, &newtrel);

            saf_declare_topo_relation(SAF_ALL, db, &cell_1, &elems, &top, &nodes, SAF_SELF(db), &cell_1,
                                      SAF_UNSTRUCTURED, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, &newtrel);


        }
        if(failed) FAILED;
        else PASSED;
        /* end of new testing------------------------------------------------------*/
        /*-------------------------------------------------------------------*/

        TESTING("saf_find_fields, with a unit");
        {
            SAF_Field *p_fields=NULL;
            int n;
            SAF_Unit umeter;
            SAF_Unit usec;

            failed=false;
            saf_find_one_unit(db, "second", &usec);
            saf_find_one_unit(db, "meter", &umeter);

            saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), "field", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
                            &usec, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &n, &p_fields);

            if (n!=1) failed=true;

            free(p_fields);
            p_fields=NULL;

            saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), "field", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
                            &umeter, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &n, &p_fields);

            if (n!=0) failed=true;

            if (p_fields) free(p_fields);
            p_fields=NULL;

            saf_find_fields(SAF_ALL, db, &top, "does not exist", SAF_ANY_QUANTITY, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
                            &usec, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &n, &p_fields);

            if (n>0) failed=true;

            if (p_fields) free(p_fields);
        }
        if (failed) FAILED;
        else PASSED;

        saf_close_database(db);

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            failCount += 1;
	}
    } SAF_TRY_END;

    saf_final();

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&failCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return (failCount==0)? 0 : 1;
}
