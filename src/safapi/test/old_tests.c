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
#include <saf.h>
#include <testutil.h>

/**/
int
main(int argc, char **argv)
{
    char dbname[1024];
    char *name;
    hbool_t failed, read_failed;
    hbool_t do_writes=FALSE, do_reads=FALSE;
    int tdim, rank=0, nerrors=0;
    SAF_Role role, SAF_USERD;
    SAF_SilRole srole;
    SAF_Db *db=NULL;
    SAF_DbProps *dbprops=NULL;
    SAF_Cat nodes, elems, blocks, side_sets, edges, procs;
    SAF_Set top, cell_1, cell_2, cell_3, ss1, ss2, ns1, cell_2_tri, cell_2_quad;
    SAF_Rel rel1, rel, rel2, trel, trel1, trel2;
    SAF_FieldTmpl coords_ftmpl, coords_ctmpl, distfac_ftmpl, temp1_ftmpl, stress_ftmpl, stress_ctmpl, temp2_ftmpl, press_ftmpl;
    SAF_FieldTmpl tmp_ftmpl[6];
    SAF_Field coords, coords_compons[2], distfac, temps1, temps2, disps, disp_compons[2], stress, stress_compons[3], pressure;

#ifdef HAVE_PARALLEL
    /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass the same argc, argv to all
     * processors. However, the MPI spec says nothing about what it does or might do to argc or argv. In fact, there is no
     * "const" in the function prototypes for either the pointers or the things they're pointing too.  I would rather pass NULL
     * here and the spec says this is perfectly acceptable.  However, that too has caused MPICH to core on certain
    * platforms. */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

    /* since we want to see whats happening make sure stdout and stderr are unbuffered */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    STU_ProcessCommandLine(1, argc, argv,
                           "do_writes",
                           "if present, issue the saf_write_xxx() calls",
                           &do_writes,
                           "do_reads",
                           "if present, issue the saf_read_xxx() calls and compare data",
                           &do_reads,
                           STU_END_OF_ARGS);

#ifdef HAVE_PARALLEL
    MPI_Bcast(&do_writes, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&do_reads, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

    saf_init(SAF_DEFAULT_LIBPROPS);

    /* create the database name */
    sprintf(dbname, "%s/%s", TEST_FILE_PATH, TEST_FILE_NAME);

    SAF_TRY_BEGIN {

        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);
        db = saf_open_database(dbname, dbprops);

        /* declare a user-defined role with ID arbitrarily chosen as 911 */
        saf_declare_role(SAF_ALL, db, "testing", "old_tests", &SAF_USERD);

        /* declare some categories */
        TESTING("declare category");
        saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, 0, &nodes);
        saf_declare_category(SAF_ALL, db, "elems", SAF_TOPOLOGY, 2, &elems);
        saf_declare_category(SAF_ALL, db, "edges", &SAF_USERD, 1, &edges);
        saf_declare_category(SAF_ALL, db, "blocks", SAF_BLOCK, 2, &blocks);
        saf_declare_category(SAF_ALL, db, "side_sets", &SAF_USERD, 1, &side_sets);
        saf_declare_category(SAF_ALL, db, "procs", SAF_PROCESSOR, 2, &procs);

        PASSED;

        failed = FALSE;
        TESTING("describe category");

        /* describe collection categories */
        name = NULL;
        saf_describe_category(SAF_ALL, &nodes, &name, &role, &tdim);
        if (strcmp(name, "nodes") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=0)
            failed = TRUE;
        free(name);
        name = NULL;
        saf_describe_category(SAF_ALL, &elems, &name, &role, &tdim);
        if (strcmp(name, "elems") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=2)
            failed = TRUE;
        free(name);
        name = NULL;
        saf_describe_category(SAF_ALL, &procs, &name, &role, &tdim);
        if (strcmp(name, "procs") || !SAF_EQUIV(&role, SAF_PROCESSOR) || tdim!=2)
            failed = TRUE;
        free(name);

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* test find calls */
    SAF_TRY_BEGIN {
        SAF_Cat(cats, p_cats, 2);
        int num_cats = 2;

        TESTING("find category");
        failed = FALSE;

        /* find by a role */
        saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), NULL, SAF_TOPOLOGY, SAF_ANY_TOPODIM, &num_cats, &p_cats);
        if (num_cats != 2)
            failed = TRUE;
        if (!(SAF_EQUIV(cats+0, &nodes) || SAF_EQUIV(cats+1, &nodes)))
            failed = TRUE;
        if (!(SAF_EQUIV(cats+0, &elems) || SAF_EQUIV(cats+1, &elems)))
            failed = TRUE;

        /* find by a name */
        saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "side_sets", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_cats, &p_cats);
        if (!SAF_EQUIV(cats+0, &side_sets))
            failed = TRUE;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;
  
    SAF_TRY_BEGIN {
        TESTING("declare set");
        failed = FALSE;

        /* declare sets */
        saf_declare_set(SAF_ALL, db, "TOP_CELL", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &top);
        saf_declare_set(SAF_ALL, db, "CELL_1", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_1);
        saf_declare_set(SAF_ALL, db, "CELL_2", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_2);
        saf_declare_set(SAF_ALL, db, "CELL_3", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_3);
        saf_declare_set(SAF_ALL, db, "SIDE_SET_1", 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ss1);
        saf_declare_set(SAF_ALL, db, "SIDE_SET_2", 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ss2);
        saf_declare_set(SAF_ALL, db, "NODE_SET_1", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ns1);

        /* we'll need the following sets to deal with inhomogeneous cell type
       on CELL_2 */
        saf_declare_set(SAF_ALL, db, "CELL_2_TRIS", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_2_tri);
        saf_declare_set(SAF_ALL, db, "CELL_2_QUADS", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_2_quad);

        PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    SAF_TRY_BEGIN {
        char *name;
        int topo_dim;
        SAF_ExtendMode extendible;

        failed = FALSE;
        TESTING("describe set");

        name = NULL;
        saf_describe_set(SAF_ALL, &top, &name, NULL, &srole, &extendible, NULL, NULL, NULL);
        if ((strcmp(name, "TOP_CELL") != 0) || (srole != SAF_SPACE) || (extendible != SAF_EXTENDIBLE_FALSE))
            failed = TRUE;
        free(name);
        saf_describe_set(SAF_ALL, &cell_3, NULL, &topo_dim, NULL, NULL, NULL, NULL, NULL);
        if (topo_dim != 2)
            failed = TRUE;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* test declare calls */
    SAF_TRY_BEGIN {

        TESTING("declare collection");

        /* declare collections on each of the previously defined sets */
        saf_declare_collection(SAF_ALL, &top, &nodes, SAF_CELLTYPE_POINT, 18, SAF_1DC(18), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &top, &elems, SAF_CELLTYPE_MIXED, 12, SAF_1DC(12), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &top, &blocks, SAF_CELLTYPE_SET, 4, SAF_1DC(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &top, &side_sets, SAF_CELLTYPE_SET, 2, SAF_1DC(2), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_1, &nodes, SAF_CELLTYPE_POINT, 9, SAF_1DC(9), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_1, &elems, SAF_CELLTYPE_QUAD, 4, SAF_1DC(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_1, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2, &nodes, SAF_CELLTYPE_POINT, 7, SAF_1DC(7), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_2, &elems, SAF_CELLTYPE_MIXED, 4, SAF_1DC(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2, &blocks, SAF_CELLTYPE_SET, 2, SAF_1DC(2), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_3, &nodes, SAF_CELLTYPE_POINT, 10, SAF_1DC(10), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_3, &elems, SAF_CELLTYPE_QUAD, 4, SAF_1DC(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_3, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &ss1, &nodes, SAF_CELLTYPE_POINT, 3, SAF_1DC(3), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &ss1, &edges, SAF_CELLTYPE_LINE, 2, SAF_1DC(2), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &ss2, &nodes, SAF_CELLTYPE_POINT, 5, SAF_1DC(5), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &ss2, &edges, SAF_CELLTYPE_LINE, 4, SAF_1DC(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &ns1, &nodes, SAF_CELLTYPE_POINT, 5, SAF_1DC(5), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2_tri, &nodes, SAF_CELLTYPE_POINT, 5, SAF_1DC(5), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_2_tri, &elems, SAF_CELLTYPE_TRI, 3, SAF_1DC(3), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2_tri, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2_quad, &nodes, SAF_CELLTYPE_POINT, 4, SAF_1DC(4), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_2_quad, &elems, SAF_CELLTYPE_QUAD, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2_quad, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

        PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* test the describe calls */
    SAF_TRY_BEGIN {
        SAF_CellType cell_type;
        int count;
        SAF_DecompMode is_decomp;
        SAF_IndexSpec ispec;

        failed = FALSE;
        TESTING("describe collection");

        saf_describe_collection(SAF_ALL, &top, &elems, &cell_type, &count, NULL, &is_decomp, NULL);
        if ((cell_type != SAF_CELLTYPE_MIXED) || (count != 12) || (is_decomp!=SAF_DECOMP_TRUE))
            failed = TRUE;
        saf_describe_collection(SAF_ALL, &cell_1, &elems, NULL, &count, NULL, NULL, NULL);
        if (count != 4)
            failed = TRUE;
        saf_describe_collection(SAF_ALL, &cell_2, &nodes, &cell_type, NULL, &ispec, NULL, NULL);
        if ((cell_type != SAF_CELLTYPE_POINT) || (ispec.ndims != 1) || (ispec.sizes[0] != 7))
            failed = TRUE;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* test the find calls */
    SAF_TRY_BEGIN {
        SAF_Cat(cats, p_cats, 2);
        int num_colls;

        failed = FALSE;
        TESTING("find collections");

        num_colls = 2;
        saf_find_collections(SAF_ALL, &top, SAF_ANY_ROLE, SAF_CELLTYPE_POINT, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, &num_colls,
                             &p_cats);
        if ((num_colls != 1) || (!SAF_EQUIV(cats+0, &nodes)))
            failed = TRUE;

        num_colls = 2;
        saf_find_collections(SAF_ALL, &top, SAF_TOPOLOGY, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, &num_colls, &p_cats);
        if (num_colls != 2)
            failed = TRUE;
        if (!(SAF_EQUIV(cats+0, &nodes) || SAF_EQUIV(cats+1, &nodes)))
            failed = TRUE;
        if (!(SAF_EQUIV(cats+0, &elems) || SAF_EQUIV(cats+1, &elems)))
            failed = TRUE;

        num_colls = 2;
        saf_find_collections(SAF_ALL, &top, SAF_ANY_ROLE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TRUE, &num_colls, &p_cats);
        if (num_colls != 2)
            failed = TRUE;
        if (!(SAF_EQUIV(cats+0, &blocks) || SAF_EQUIV(cats+1, &blocks)))
            failed = TRUE;
        if (!(SAF_EQUIV(cats+0, &elems) || SAF_EQUIV(cats+1, &elems)))
            failed = TRUE;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    SAF_TRY_BEGIN {

        TESTING(do_writes?"declare & write subset relation":"declare subset relation");

        /* nodes and elems of cell_1 and top */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&nodes), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel1);
        if (do_writes) {
            int buf[] = {1, 2, 3, 5, 6, 7, 9, 10, 11};
            saf_write_subset_relation(SAF_ALL, &rel1, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }

        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&elems), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel2);
        if (do_writes) {
            int buf[] = {1, 2, 4, 5};
            saf_write_subset_relation(SAF_ALL, &rel2, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }

        /* nodes and elems of cell_2 and top */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2, SAF_COMMON(&nodes), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {9, 10, 11, 13, 14, 16, 17};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2, SAF_COMMON(&elems), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {7, 8, 9, 11};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }

        /* nodes and elems of cell_2_tri & cell_2_quad */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri, SAF_COMMON(&nodes), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {1, 2, 3, 5, 7};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri, SAF_COMMON(&elems), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {7, 8, 9};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&nodes), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {13, 14, 16, 17};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&elems), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {11};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }

        /* nodes and elems of cell_3 */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3, SAF_COMMON(&nodes), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {3, 4, 7, 8, 11, 12, 14, 15, 17, 18};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3, SAF_COMMON(&elems), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {3, 6, 10, 12};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }

        saf_declare_subset_relation(SAF_ALL, db, &top, &ss1, SAF_COMMON(&nodes), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {9, 10, 11};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }

        /* nodes of side set 2 */
        saf_declare_subset_relation(SAF_ALL, db, &top, &ss2, SAF_COMMON(&nodes), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {1, 5, 9, 13, 16};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }

        /* nodes of node set 1 */
        saf_declare_subset_relation(SAF_ALL, db, &top, &ns1, SAF_COMMON(&nodes), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {1, 5, 9, 13, 16};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
 
        /* now, declare the subset relations for blocks */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&blocks), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {0};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri, SAF_COMMON(&blocks), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {1};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&blocks), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {2};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3, SAF_COMMON(&blocks), SAF_TUPLES, H5T_NATIVE_INT, NULL,
                                    H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {3};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, db);
        }

        PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
            goto theExit;
        }
    } SAF_TRY_END;

    /* now, try to find subset relations */
    SAF_TRY_BEGIN {
        int num_rels;
        SAF_Rel(rels, p_rels, 1);

        failed = FALSE;
        TESTING("find subset relation");

        saf_find_subset_relations(SAF_ALL, db, &top, &cell_1, SAF_COMMON(SAF_ANY_CAT), &num_rels, NULL);
        if (num_rels != 3)
            failed = TRUE;

        num_rels = 1;
        saf_find_subset_relations(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&nodes), &num_rels, &p_rels);
        if (num_rels != 1)
            failed = TRUE;
        if (!SAF_EQUIV(rels+0, &rel1))
            failed = TRUE;

        saf_find_subset_relations(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&elems), &num_rels, &p_rels);
        if (num_rels != 1)
            failed = TRUE;
        if (!SAF_EQUIV(rels+0, &rel2))
            failed = TRUE;

        saf_find_subset_relations(SAF_ALL, db, &cell_2, &cell_1, SAF_COMMON(SAF_ANY_CAT), &num_rels, &p_rels);
        if (num_rels != 0)
            failed = TRUE;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    SAF_TRY_BEGIN {
        TESTING(do_writes?"declare & write topological relation":"declare topological relation");

        /* for cell_1 */
        saf_declare_topo_relation(SAF_ALL, db, &cell_1, &elems, &top, &nodes, SAF_SELF(db), &cell_1,
                                  SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel1);
        if (do_writes) {
            int abuf[] = {4};
            int bbuf[] = {1, 2, 6, 5, 2, 3, 7, 6, 5, 6, 10, 9, 6, 7, 11, 10};
            saf_write_topo_relation(SAF_ALL, &trel1, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, db);
        }

        /* for cell_2_tri */
        saf_declare_topo_relation(SAF_ALL, db, &cell_2_tri, &elems, &top, &nodes, SAF_SELF(db), &cell_2_tri,
                                  SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel2);
        if (do_writes) {
            int abuf[] = {3};
            int bbuf[] = {1, 2, 4, 4, 2, 5, 2, 3, 5};
            saf_write_topo_relation(SAF_ALL, &trel2, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, db);
        }

        /* for cell_2_quad */
        saf_declare_topo_relation(SAF_ALL, db, &cell_2_quad, &elems, &top, &nodes, SAF_SELF(db), &cell_2_quad,
                                  SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
        if (do_writes) {
            int abuf[] = {4};
            int bbuf[] = {4, 5, 7, 6};
            saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, db);
        }

        /* for cell_3 */
        saf_declare_topo_relation(SAF_ALL, db, &cell_3, &elems, &top, &nodes, SAF_SELF(db), &cell_3,
                                  SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
        if (do_writes) {
            int abuf[] = {4};
            int bbuf[] = {1, 2, 4, 3, 3, 4, 6, 5, 5, 6, 8, 7, 7, 8, 10, 9};
            saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, db);
        }

        PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    read_failed = FALSE;
    SAF_TRY_BEGIN {
        if (do_reads) {
            TESTING("read subset relation");

            /* try to read subset relation 1 */
            {
                int buf[] = {1, 2, 3, 5, 6, 7, 9, 10, 11};
                int i, *rbuf = NULL;
                saf_read_subset_relation(SAF_ALL, &rel1, NULL, (void**)&rbuf, NULL);
                for (i = 0; i < (int)sizeof(buf) / (int)sizeof(int); i++) {
                    if (buf[i] != rbuf[i])
                        read_failed = TRUE;
                }
                free(rbuf);
            }

            /* try to read subset relation 2 */
            {
                int buf[] = {1, 2, 4, 5};
                int i, *rbuf = NULL;
                saf_read_subset_relation(SAF_ALL, &rel2, NULL, (void**)&rbuf, NULL);
                for (i = 0; i < (int)sizeof(buf) / (int)sizeof(int); i++) {
                    if (buf[i] != rbuf[i])
                        read_failed = TRUE;
                }
                free(rbuf);
            }

            if (read_failed) {
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

    /* now, test reading topological relations */
    read_failed = FALSE;
    SAF_TRY_BEGIN {
        if (do_reads) {
            TESTING("read topological relations");

            /* try to read topological relation 1 */
            {
                int abuf[] = {4};
                int bbuf[] = {1, 2, 6, 5, 2, 3, 7, 6, 5, 6, 10, 9, 6, 7, 11, 10};
                int i, *rabuf = NULL, *rbbuf = NULL;
                saf_read_topo_relation(SAF_ALL, &trel1, NULL, (void**)&rabuf, (void**) &rbbuf);
                if (abuf[0] != rabuf[0])
                    read_failed = TRUE;
                for (i = 0; i < (int)sizeof(bbuf) / (int)sizeof(int); i++) {
                    if (bbuf[i] != rbbuf[i])
                        read_failed = TRUE;
                }
                free(rabuf);
                free(rbbuf);
            }

            /* try to read topological relation 2 */
            {
                int abuf[] = {3};
                int bbuf[] = {1, 2, 4, 4, 2, 5, 2, 3, 5};
                int i, *rabuf = NULL, *rbbuf = NULL;
                saf_read_topo_relation(SAF_ALL, &trel2, NULL, (void**)&rabuf, (void**) &rbbuf);
                if (abuf[0] != rabuf[0])
                    read_failed = TRUE;
                for (i = 0; i < (int)sizeof(bbuf) / (int)sizeof(int); i++) {
                    if (bbuf[i] != rbbuf[i])
                        read_failed = TRUE;
                }
                free(rabuf);
                free(rbbuf);
            }

            if (read_failed) {
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

        /* now, lets declare some field templates */
        TESTING("declare field templates");

        /* for coordinate components */
        saf_declare_field_tmpl(SAF_ALL, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
                               NULL, &coords_ctmpl);
    
        tmp_ftmpl[0] = coords_ctmpl;
        tmp_ftmpl[1] = coords_ctmpl;

        /* for coordinate fields */
        saf_declare_field_tmpl(SAF_ALL, db, "coordinate_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 2,
                               tmp_ftmpl, &coords_ftmpl);
    
        /* for distribution factors */
        saf_declare_field_tmpl(SAF_ALL, db, "distrib_factors_tmpl", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_NOT_APPLICABLE_QUANTITY, 1,
                               NULL, &distfac_ftmpl);

        /* for temperature on ns1 */
        saf_declare_field_tmpl(SAF_ALL, db, "temp_on_ns1_tmpl", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QTEMP, 1,
                               NULL, &temp1_ftmpl);

        /* for stress components */
        saf_declare_field_tmpl(SAF_ALL, db, "stress_on_cell_1_ctmpl", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QNAME(db, "pressure"), 1,
                               NULL, &stress_ctmpl);

        tmp_ftmpl[0] = stress_ctmpl;
        tmp_ftmpl[1] = stress_ctmpl;
        tmp_ftmpl[2] = stress_ctmpl;

        /* for stress */
        saf_declare_field_tmpl(SAF_ALL, db, "stress_on_cell_1_tmpl", SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI,
                               SAF_QNAME(db, "pressure"), 3, tmp_ftmpl, &stress_ftmpl);

        /* for temperature on nodes of cell_2 */
        saf_declare_field_tmpl(SAF_ALL, db, "temp_on_cell_2_tmpl", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QTEMP, 1,
                               NULL, &temp2_ftmpl);

        /* for pressure on elemenst of side set 1 */
        saf_declare_field_tmpl(SAF_ALL, db, "pressure_on_ss1", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QNAME(db, "pressure"), 1,
                               NULL, &press_ftmpl);

        PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* now, try some describes */
    SAF_TRY_BEGIN {
        SAF_Algebraic atype;
        int n;

        failed = FALSE;
        TESTING("describe field templates");

        name = NULL;
        saf_describe_field_tmpl(SAF_ALL, &coords_ftmpl, &name, &atype, NULL, NULL, &n, NULL);
        if (strcmp(name, "coordinate_tmpl") || !SAF_EQUIV(&atype, SAF_ALGTYPE_VECTOR) || n!=2)
            failed = TRUE;
        free(name);
        name = NULL;
        saf_describe_field_tmpl(SAF_ALL, &distfac_ftmpl, &name, &atype, NULL, NULL, &n, NULL);
        if ((strcmp(name, "distrib_factors_tmpl"))  || !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) || (n != 1))
            failed = TRUE;
        free(name);
        saf_describe_field_tmpl(SAF_ALL, &coords_ctmpl, NULL, &atype, NULL, NULL, &n, NULL);
        if (!SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) || (n != 1))
            failed = TRUE;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* now, try some finds */
    SAF_TRY_BEGIN {
        int num_ftmpls;
        SAF_FieldTmpl(ftmpls, p_ftmpls, 1);

        failed = FALSE;
        TESTING("find field templates");

        saf_find_field_tmpls(SAF_ALL, db, NULL, SAF_ALGTYPE_SCALAR, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, NULL);
        if (num_ftmpls != 6)
            failed = TRUE;

        saf_find_field_tmpls(SAF_ALL, db, NULL, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, NULL);
        if (num_ftmpls != 8)
            failed = TRUE;

        saf_find_field_tmpls(SAF_ALL, db, NULL, SAF_ALGTYPE_SYMTENSOR, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, NULL);
        if (num_ftmpls != 1)
            failed = TRUE;

        num_ftmpls = 1;
        saf_find_field_tmpls(SAF_ALL, db, NULL, SAF_ALGTYPE_SYMTENSOR, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, &p_ftmpls);
        if (num_ftmpls != 1)
            failed = TRUE;
        if (!SAF_EQUIV(ftmpls+0, &stress_ftmpl))
            failed = TRUE;

        num_ftmpls = 1;
        saf_find_field_tmpls(SAF_ALL, db, "distrib_factors_tmpl", SAF_ALGTYPE_ANY,
                             SAF_ANY_BASIS, SAF_ANY_QUANTITY, &num_ftmpls, &p_ftmpls);
        if (num_ftmpls != 1)
            failed = TRUE;
        if (!SAF_EQUIV(ftmpls+0, &distfac_ftmpl))
            failed = TRUE;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;
    

    SAF_TRY_BEGIN {
        SAF_Unit umeter;
        SAF_Unit ukelvin;
        SAF_Unit upsi;
        SAF_Unit upascal;

        saf_find_one_unit(db, "meter", &umeter);
        saf_find_one_unit(db, "kelvin", &ukelvin);
        saf_find_one_unit(db, "psi", &upsi);
        saf_find_one_unit(db, "pascal", &upascal);

        TESTING(do_writes?"declare & write field":"declare field");

        /* now for some fields */
        saf_declare_field(SAF_ALL, db, &coords_ctmpl, "X", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coords_compons[0]));
        saf_declare_field(SAF_ALL, db, &coords_ctmpl, "Y", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coords_compons[1]));
        saf_declare_field(SAF_ALL, db, &coords_ftmpl, "coords", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          H5T_NATIVE_FLOAT, coords_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coords);
        if (do_writes) {
            float buf[] = {0., 4., 1., 4., 2., 4., 2.5, 4.,
                           0., 3., 1., 3., 2., 3., 2.5, 3.,
                           0., 2., 1., 2., 2., 2., 2.5, 2.,
                           0., 1., 2., 1., 2.5, 1.,
                           0., 0., 2., 0., 2.5, 0.};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db); 
        }

        /* specify that is a coordinate field */
        saf_declare_coords(SAF_ALL, &coords);

        /* made up distribution factors on ss2 */
        saf_declare_field(SAF_ALL, db, &distfac_ftmpl, "distribution factors", &ss2, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(db),
                          SAF_NODAL(&nodes, &edges), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &distfac);
        if (do_writes) {
            float buf[] = {4., 3., 2., 1., 0.};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &distfac, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db); 
        }

        /* made up temperatures */
        saf_declare_field(SAF_ALL, db, &temp1_ftmpl, "temperature", &ns1, &ukelvin, SAF_SELF(db), SAF_NODAL(&nodes, &nodes),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &temps1);
        if (do_writes) {
            float buf[] = {100., 150., 150., 100., 75.};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &temps1, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db); 
        }

        /* made up displacements */
        saf_declare_field(SAF_ALL, db, &coords_ctmpl, "dX", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[0]));
        saf_declare_field(SAF_ALL, db, &coords_ctmpl, "dY", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[1]));
        saf_declare_field(SAF_ALL, db, &coords_ftmpl, "displacements", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          H5T_NATIVE_FLOAT, disp_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &disps);
        if (do_writes) {
            float buf[] = {.25, .25, .25, .25, .25, .25, .25, .25,
                           .25, .25, .25, .25, .25, .25, .25, .25,
                           .25, .25, .25, .25, .25, .25, .25, .25,
                           .25, .25, .25, .25, .25, .25,
                           .25, .25, .25, .25, .25, .25};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &disps, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db); 
        }

        /* made up stresses */
        saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sx", &cell_1, &upsi, SAF_SELF(db), SAF_ZONAL(&elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[0]));
        saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sy", &cell_1, &upsi, SAF_SELF(db), SAF_ZONAL(&elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[1]));
        saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sxy", &cell_1, &upsi, SAF_SELF(db), SAF_ZONAL(&elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[2]));
        saf_declare_field(SAF_ALL, db, &stress_ftmpl, "stress", &cell_1, &upsi, SAF_SELF(db), SAF_ZONAL(&elems),
                          H5T_NATIVE_FLOAT, stress_compons, SAF_INTERLEAVE_VECTOR, NULL, NULL, &stress);
        if (do_writes) {
            float buf[] = {0.5, 0.25, 0.5, 0.5, 0.25, 0.5,
                           0.5, 0.25, 0.5, 0.5, 0.25, 0.5};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &stress, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db); 
        }

        /* made up temperatures */
        saf_declare_field(SAF_ALL, db, &temp2_ftmpl, "temperature", &cell_2, &ukelvin, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &temps2);
        if (do_writes) {
            float buf[] = {75., 95., 120., 80., 115., 85., 110.};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &temps2, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db); 
        }

        /* made up pressures */
        saf_declare_field(SAF_ALL, db, &press_ftmpl, "pressure", &ss1, &upascal, SAF_SELF(db), SAF_ZONAL(&edges),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &pressure);
        if (do_writes) {
            float buf[] = {45., 55.};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &pressure, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db); 
        }

        PASSED;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* now, test reading of fields */
    read_failed = FALSE;
    SAF_TRY_BEGIN {
        if (do_reads) {
            TESTING("read field");

            {
                float buf[] = {0., 4., 1., 4., 2., 4., 2.5, 4.,
                               0., 3., 1., 3., 2., 3., 2.5, 3.,
                               0., 2., 1., 2., 2., 2., 2.5, 2.,
                               0., 1., 2., 1., 2.5, 1.,
                               0., 0., 2., 0., 2.5, 0.};
                float *rbuf = NULL;
                int i;
                saf_read_field(SAF_ALL, &coords, NULL, SAF_WHOLE_FIELD, (void**)&rbuf);
                for (i = 0; i < (int)sizeof(buf) / (int)sizeof(float); i++)
                    if (buf[i] != rbuf[i])
                        read_failed = TRUE;
                free(rbuf);
            }

            {
                float buf[] = {4., 3., 2., 1., 0.};
                float *rbuf = NULL;
                int i;
                saf_read_field(SAF_ALL, &distfac, NULL, SAF_WHOLE_FIELD, (void**)&rbuf);
                for (i = 0; i < (int)sizeof(buf) / (int)sizeof(float); i++)
                    if (buf[i] != rbuf[i])
                        read_failed = TRUE;
                free(rbuf);
            }

            if (read_failed) {
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

    /* test finds */
    SAF_TRY_BEGIN {
        SAF_Field(fields, p_fields, 2);
        int num_fields = 2;

        failed = FALSE;
        TESTING("find field");

        saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), "coords", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY,
                        SAF_ANY_BASIS, SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC,
                        &num_fields, NULL);
        if (num_fields != 1)
            failed = TRUE;

        num_fields = 2;
        saf_find_fields(SAF_ALL, db, &top, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_VECTOR,
                        SAF_ANY_BASIS, SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC,
                        &num_fields, &p_fields);
        if (num_fields != 2)
            failed = TRUE;
        if (!(SAF_EQUIV(&coords, fields+0) || SAF_EQUIV(&coords, fields+1)))
            failed = TRUE;
        if (!(SAF_EQUIV(&disps, fields+0) || SAF_EQUIV(&disps, fields+1)))
            failed = TRUE;

        saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY,
                        SAF_ANY_BASIS, SAF_ANY_UNIT, &nodes, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC,
                        &num_fields, NULL);
        if (num_fields != 9)
            failed = TRUE;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

theExit:
    saf_close_database(db);
    saf_final();

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return nerrors?1:0;
}
