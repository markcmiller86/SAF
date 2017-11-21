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
#include <../test/testutil.h>

#define MAX_NUM_NODES_PER_ELEM  8
#define MAX_NUM_NODES  15
#define MAX_NUM_ELEMS  12

/*
 *  Prototypes
 */

void create_elem_map (int loc_num_elems, int start, int *elem_map);

void extract_connect (int num_elem, int *elem_map, int *connect, int *domain_connect);

void create_node_map (int len_connect, int *domain_connect, int *node_map, int *loc_num_nodes);

void create_local_connect (int *node_map, int len_node_map, int len_connect, int *domain_connect, int *loc_connect);

void sort_int(int n, int ra[]);

int bin_search2 (int value, int num, int List[]);

void calc_in_or_out(int first_elem_in_blk, int last_elem_in_blk, int first_elem_in_domain, int last_elem_in_domain, 
                    int *first_elem_in_dom_blk, int *num_elem_in_dom_blk);

void extract_dom_block_connect(int num_elem_in_blk, int *loc_connect, int *dom_blk_connect);

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:     This program demonstrates the functions needed by a parallel EXODUS application.
 *
 * Description: Creates a SAF database in parallel, emulating a parallel EXODUS writing client.
 *
 *              The code may be somewhat confusing but the subset relation graph (SRG) is straightforward.
 *              The SRG looks like this:
 *
 *              TOP_SET
 *              |
 *              |
 *              |---DOMAIN_0
 *              |   |
 *              |   |
 *              |   |---BLOCK_0_DOMAIN_0
 *              |   |   .
 *              |   |   .
 *              |   |---BLOCK_num_blocks_DOMAIN_0
 *              |
 *              |---DOMAIN_1
 *              |   .
 *              |   .
 *              |   .
 *              |---DOMAIN_num_domains
 *              |   |
 *              |   |
 *              |   |---BLOCK_0_DOMAIN_num_domains
 *              |   |   .
 *              |   |   .
 *              |   |---BLOCK_num_blocks_DOMAIN_num_domains
 *              |
 *              |
 *              |
 *              |
 *              |---BLOCK_0
 *              |   |
 *              |   |
 *              |   |---BLOCK_0_DOMAIN_0  (same set as subset of DOMAIN_0)
 *              |   |   .
 *              |   |   .
 *              |   |---BLOCK_0_DOMAIN_num_domains
 *              |
 *              |---BLOCK_1
 *              |   .
 *              |   .
 *              |   .
 *              |---BLOCK_num_blocks
 *                  |
 *                  |
 *                  |---BLOCK_num_blocks_DOMAIN_0
 *                  |   .
 *                  |   .
 *                  |---BLOCK_num_blocks_DOMAIN_num_domains
 *
 *
 * Parallel:    This is a parallel client.
 *
 * Programmer:	Larry Schoof, 12/18/2001
 *
 * Modifications: 
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
    SAF_Db *db=NULL;
    char dbname[1024];
    SAF_DbProps *dbprops=NULL;
    SAF_Role saf_ss_role, saf_ns_role, saf_node_comm_role, saf_side_comm_role;
    SAF_Cat nodes, elems, blocks, side_sets, node_sets, domain_cat, node_comm, side_comm;
    SAF_Set top, block_1, block_2, block_3, block_4, block_5;
    SAF_Set domain_set, domain_block_1, domain_block_2, domain_block_3, domain_block_4, domain_block_5; 
    SAF_Set block_set[5], dom_block_set[5];
    SAF_Set side_set_a, side_set_b, node_set_a;
    SAF_Set side_set_a_q, side_set_a_t;
    SAF_Set node_comm_set, side_comm_set;
    SAF_Set domain_ssa, domain_ssa_q, domain_ssa_t, domain_ssb, domain_nsa;

    SAF_Rel rel, dom_blk_trel;

    SAF_FieldTmpl coords_ctmpl, coords_ftmpl, tmp_ftmpl[6], coords_on_top_ftmpl;
    SAF_FieldTmpl dom_ssa_elem_ctmpl, dom_ssa_elem_ftmpl;
    SAF_FieldTmpl blk_1_elem_ftmpl, blk_3_elem_ftmpl, ssaq_elem_ftmpl, ssat_elem_ftmpl;
    SAF_FieldTmpl nsa_node_ftmpl;
    SAF_FieldTmpl top_elem_ftmpl, top_constant_ftmpl;
    SAF_FieldTmpl domain_node_ctmpl, domain_node_ftmpl;
    SAF_FieldTmpl domain_elem_ftmpl;
    SAF_FieldTmpl disp_on_top_ftmpl;
    SAF_FieldTmpl vel_on_top_ftmpl;
    SAF_FieldTmpl dom_block_2_elem_ctmpl, dom_block_2_elem_ftmpl;
    SAF_FieldTmpl stress_on_blk2_ftmpl, stress_on_blk5_ftmpl;
    SAF_FieldTmpl strain_on_blk1_ftmpl, strain_on_blk3_ftmpl;
    SAF_FieldTmpl press_on_blk1_ftmpl, press_on_blk2_ftmpl;
    SAF_FieldTmpl dom_block_elem_ctmpl, dom_block_elem_ftmpl;
    SAF_FieldTmpl dom_block_node_ctmpl, dom_block_node_ftmpl;
    SAF_FieldTmpl cent_on_top_ftmpl;
    SAF_FieldTmpl nodes_on_blk_ftmpl;
    SAF_FieldTmpl node_comm_ftmpl, side_comm_ftmpl;

    SAF_Field x_coords, y_coords, z_coords, coords, coords_on_top;
    SAF_Field thickness, dist_fact;
    SAF_Field x_disp, y_disp, z_disp, disp, disp_on_top;
    SAF_Field x_vel, y_vel, z_vel, vel, vel_on_top;
    SAF_Field sigxx, sigyy, sigzz, sigxy, sigyz, sigzx;
    SAF_Field stress, stress_on_blk2, stress_on_blk5;
    SAF_Field strain, strain_on_blk1, strain_on_blk3;
    SAF_Field epsxx, epsyy, epszz, epsxy, epsyz, epszx;
    SAF_Field pressure, press_on_blk1, press_on_blk2;
    SAF_Field cent_x, cent_y, cent_z, centroid, cent_on_top;
    SAF_Field centroid_fld_list[5];
    SAF_Field ke, te;
    SAF_Field x_rot, y_rot, z_rot, rot, nodal_rot_in_blk;
    SAF_Field area;
    SAF_Field x_norm, y_norm, z_norm, normal;
    SAF_Field elem_ids_fld, elem_ids_on_top;
    SAF_Field node_comm_procs_fld, side_comm_procs_fld;
  
    SAF_Field tmp_fields[6], *coord_components;

    SAF_Field field_list[40];
    SAF_FieldTmpl field_tmpl_list[40];

    SAF_Suite suite;
    SAF_StateTmpl stmpl, init_stmpl;
    SAF_StateGrp state_grp, init_state_grp;

    SAF_Quantity *vel_q, *stress_q, *strain_q, *pressure_q, *energy_q, *angle_q, *area_q;
    SAF_Quantity *tmp_q;

    SAF_Unit *meter=NULL, *m_per_s=NULL, *pascal=NULL;

    char tmp_name[255];

    int i, block_index, offset;
    int dom_blk_node_map[12], num_nodes_in_dom_blk;
    int rank, num_domain, num_blocks, num_side_sets, num_node_sets, len_connect, num_nodes, num_sides;
    int glob_num_nodes, glob_num_elems, loc_num_nodes, loc_num_elems;
    int begin_elem_index_in_domain, end_elem_index_in_domain, *begin_elem_index_in_blk, *end_elem_index_in_blk;
    int begin_elem_index_in_dom_blk;
    int num_elem_in_dom_blk;
    int accum_num_elem;
    int *node_map, *elem_map, *domain_connect, *loc_connect;
    int block_num[2];

    float time[10];
 
    int failed=0;
    int cnt = 0, state;

    /* fill in the global connectivity array; this is padded with -1 so there is a constant stride thru the array; this padding
     * is just for convenience in this client and has nothing to do with SAF */
    int glob_connect[] = { 3,4,7,6,-1,-1,-1,-1, 0,1,4,3,-1,-1,-1,-1,    /* BLOCK 1 */
                           3,4,14,13,6,7,10,9, 0,1,12,11,3,4,14,13,     /* BLOCK 2 */
                           7,5,8,-1,-1,-1,-1,-1, 4,5,7,-1,-1,-1,-1,-1, 4,1,5,-1,-1,-1,-1,-1, 1,2,5,-1,-1,-1,-1,-1, /* BLOCK 3 */
                           4,14,10,7,5,-1,-1,-1, 1,12,14,4,5,-1,-1,-1,  /* BLOCK 4 */
                           7,5,10,8,-1,-1,-1,-1, 1,2,12,5,-1,-1,-1,-1}; /* BLOCK 5 */

    int num_elem_in_blk[] = {2,2,4,2,2};   /* number of elements in each block */

    /* global element IDs */    
    int elem_ids[] = {100, 101, 102, 103, 104, 105, 106, 207, 208, 209, 210, 211};

    /* global coordinates */    
    /* node ID:  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  */
    float x[] = {0., 1., 2., 0., 1., 2., 0., 1., 2., 0., 1., 0., 1., 0., 1.};
    float y[] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 1., 1., 1., 1., 1.};
    float z[] = {0., 0., 0., 1., 1., 1., 2., 2., 2., 2., 2., 0., 0., 1., 1.};

    /* global nodal displacements */    
    /* node ID:    0    1    2    3    4    5    6    7    8    9    10    11    12    13    14  */
    float dx[] = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 1.10, 1.11, 1.12, 1.13, 1.14};
    float dy[] = {2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 2.10, 2.11, 2.12, 2.13, 2.14};
    float dz[] = {3.0, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 3.10, 3.11, 3.12, 3.13, 3.14};

    /* global nodal velocities */    
    float xvel[] = {10.0, 10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8, 10.9, 10.10, 10.11, 10.12, 10.13, 10.14};
    float yvel[] = {20.0, 20.1, 20.2, 20.3, 20.4, 20.5, 20.6, 20.7, 20.8, 20.9, 20.10, 20.11, 20.12, 20.13, 20.14};
    float zvel[] = {30.0, 30.1, 30.2, 30.3, 30.4, 30.5, 30.6, 30.7, 30.8, 30.9, 30.10, 30.11, 30.12, 30.13, 30.14};

    /* global element centroids */    
    /* element ID     0     1     2     3    4     5     6     7     8     9    10    11 */
    float xcent[] = { .5,   .5,   .5,   .5, 1.67, 1.33, 1.33, 1.67, 1.33, 1.33, 1.5 , 1.5};
    float ycent[] = {0. ,  0. ,   .5,   .5, 0.  , 0.  , 0.  , 0.  ,  .33,  .33,  .25,  .25};
    float zcent[] = {1.5,   .5,  1.5,   .5, 1.67, 1.33,  .67,  .33,  .67, 1.33, 1.75, 1.25};

    /* global stresses
     * only elements in blocks 2 (elements 2 and 3) and 5 (elements 10 and 11) should have valid values */
    float sigxx_vals[] = {-1, -1, 100.2, 100.3, -1, -1, -1, -1, -1, -1, 100.10, 100.11};
    float sigyy_vals[] = {-1, -1, 200.2, 200.3, -1, -1, -1, -1, -1, -1, 200.10, 200.11};
    float sigzz_vals[] = {-1, -1, 300.2, 300.3, -1, -1, -1, -1, -1, -1, 300.10, 300.11};
    float sigxy_vals[] = {-1, -1, 400.2, 400.3, -1, -1, -1, -1, -1, -1, 400.10, 400.11};
    float sigyz_vals[] = {-1, -1, 500.2, 500.3, -1, -1, -1, -1, -1, -1, 500.10, 500.11};
    float sigzx_vals[] = {-1, -1, 600.2, 600.3, -1, -1, -1, -1, -1, -1, 600.10, 600.11};

    /* global strains
     * only elements in blocks 1 (elements 0 and 1) and 3 (elements 4,5,6,7) should have valid values */
    float epsxx_vals[] = {10.0, 10.1, -1, -1, 10.4, 10.5, 10.6, 10.7, -1, -1, -1, -1};
    float epsyy_vals[] = {20.0, 20.1, -1, -1, 20.4, 20.5, 20.6, 20.7, -1, -1, -1, -1};
    float epszz_vals[] = {30.0, 30.1, -1, -1, 30.4, 30.5, 30.6, 30.7, -1, -1, -1, -1};
    float epsxy_vals[] = {40.0, 40.1, -1, -1, 40.4, 40.5, 40.6, 40.7, -1, -1, -1, -1};
    float epsyz_vals[] = {50.0, 50.1, -1, -1, 50.4, 50.5, 50.6, 50.7, -1, -1, -1, -1};
    float epszx_vals[] = {60.0, 60.1, -1, -1, 60.4, 60.5, 60.6, 60.7, -1, -1, -1, -1};

    /* global pressures
     * only elements in blocks 1 (elements 0 and 1) and 2 (elements 2 and 3) should have valid values */
    float press_vals[] = {1000., 2000., 3000., 4000., -1, -1, -1, -1, -1, -1, -1, -1};

    void *pbuf;

    /* initialize MPI */    
#ifdef HAVE_PARALLEL
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &num_domain);
#else
    rank=0;
    num_domain=1;
#endif

#if 0 /*DEBUGGING rpm 2002-07-08 */
    fprintf(stderr, "pid=%d, task=%d stopping\n", getpid(), rank);
    raise(SIGSTOP);
#endif
   
    /* initialize the SAF library */    
    saf_init(SAF_DEFAULT_LIBPROPS);

    SAF_TRY_BEGIN {
        /* open (create) a database  */            
        strcpy(dbname, "exo_par_wt.saf");

        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);
        db = saf_open_database(dbname,dbprops);

        /* Find units */
        meter = saf_find_one_unit(db, "meter", NULL);
        m_per_s = saf_find_one_unit(db, "meter per second", NULL);
        pascal = saf_find_one_unit(db, "pascal", NULL);

        /* create categories that will be used in creating collections on sets */            
        saf_declare_role(SAF_ALL, db, "side sets", NULL, &saf_ss_role); 
        saf_declare_role(SAF_ALL, db, "node sets", NULL, &saf_ns_role);
        saf_declare_role(SAF_ALL, db, "node comm maps", NULL, &saf_node_comm_role); 
        saf_declare_role(SAF_ALL, db, "side comm maps", NULL, &saf_side_comm_role);

        saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, 0, &nodes);
        saf_declare_category(SAF_ALL, db, "elems", SAF_TOPOLOGY, 3, &elems);
        saf_declare_category(SAF_ALL, db, "blocks", SAF_BLOCK, 3, &blocks);
        saf_declare_category(SAF_ALL, db, "domains", SAF_PROCESSOR, 3, &domain_cat);
        saf_declare_category(SAF_ALL, db, "side_sets", &saf_ss_role, 2, &side_sets);
        saf_declare_category(SAF_ALL, db, "node_sets", &saf_ns_role, 0, &node_sets);
        saf_declare_category(SAF_ALL, db, "node_comm", &saf_node_comm_role, 0, &node_comm);
        saf_declare_category(SAF_ALL, db, "side_comm", &saf_side_comm_role, 2, &side_comm);

        /* get quantities that will be used in various field templates */ 
        vel_q = saf_find_one_quantity(db,"velocity", NULL);
        stress_q = saf_find_one_quantity(db, "pressure", NULL); /*a.k.a., stress*/
    
        strain_q = saf_declare_quantity(SAF_ALL,db,"strain", "strain", NULL,NULL); 
        tmp_q = SAF_QLENGTH;
        saf_multiply_quantity(SAF_ALL,strain_q, tmp_q, 1);
        saf_multiply_quantity(SAF_ALL,strain_q, tmp_q, -1);
    
        pressure_q = saf_find_one_quantity (db, "pressure",NULL);
        energy_q = saf_find_one_quantity (db, "energy",NULL);
        angle_q = saf_find_one_quantity(db,"plane angle",NULL);
        area_q = saf_find_one_quantity(db,"area",NULL); 

        /* global set information */            
        { 
            /* create a top set called "TOP_SET" */                
            saf_declare_set(SAF_ALL, db, "TOP_SET", 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &top);

            /* create collections in "TOP_SET" for:
             * elements;
             * nodes;
             * element blocks;
             * side sets;
             * node sets;
             * domains */
            glob_num_nodes = 15;
            glob_num_elems = 12;
            num_blocks = 5;
            num_side_sets = 2;
            num_node_sets = 1;

            saf_declare_collection(SAF_ALL, &top, &nodes, SAF_CELLTYPE_POINT, glob_num_nodes, SAF_1DC(glob_num_nodes),
                                   SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_ALL, &top, &elems, SAF_CELLTYPE_MIXED, glob_num_elems, SAF_1DC(glob_num_elems),
                                   SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &top, &blocks, SAF_CELLTYPE_SET, num_blocks, SAF_1DC(num_blocks), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &top, &side_sets, SAF_CELLTYPE_SET, num_side_sets, SAF_1DC(num_side_sets),
                                   SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_ALL, &top, &node_sets, SAF_CELLTYPE_SET, num_node_sets, SAF_1DC(num_node_sets),
                                   SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_ALL, &top, &domain_cat, SAF_CELLTYPE_SET, num_domain, SAF_1DC(num_domain), SAF_DECOMP_TRUE);

            /* create "BLOCK_1" with quad shells */                
            saf_declare_set(SAF_ALL, db, "BLOCK_1", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_1);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_ALL, &block_1, "EXO_ELEM_TYPE", H5T_C_S1, 1, "SHELL");

            saf_declare_collection(SAF_ALL, &block_1, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_1, &elems, SAF_CELLTYPE_QUAD, num_elem_in_blk[0], SAF_1DC(num_elem_in_blk[0]), 
                                   SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_1, &nodes, SAF_CELLTYPE_POINT, 6, SAF_1DC(6), SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_ALL, &block_1, &domain_cat, SAF_CELLTYPE_SET, num_domain, SAF_1DC(num_domain),
                                   SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_1, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {0,2,1};   /* start, count, stride */
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_1, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {0};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* NOTE: this node subset relation is necessary because we are going to define a field on the nodes of BLOCK_1 */
            saf_declare_subset_relation(SAF_ALL, db, &top, &block_1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {0,1,3,4,6,7};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "BLOCK_2" with hexes */                
            saf_declare_set(SAF_ALL, db, "BLOCK_2", 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_2);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_ALL, &block_2, "EXO_ELEM_TYPE", H5T_C_S1, 1, "HEX");

            saf_declare_collection(SAF_ALL, &block_2, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_2, &elems, SAF_CELLTYPE_HEX, num_elem_in_blk[1], SAF_1DC(num_elem_in_blk[1]), 
                                   SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_2, &domain_cat, SAF_CELLTYPE_SET, num_domain, SAF_1DC(num_domain),
                                   SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_2, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {2,2,1};   /* start, count, stride */
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_2, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {1};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "BLOCK_3" with tri shells */                
            saf_declare_set(SAF_ALL, db, "BLOCK_3", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_3);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_ALL, &block_3, "EXO_ELEM_TYPE", H5T_C_S1, 1, "SHELL");

            saf_declare_collection(SAF_ALL, &block_3, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_3, &elems, SAF_CELLTYPE_TRI, num_elem_in_blk[2], SAF_1DC(num_elem_in_blk[2]), 
                                   SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_3, &nodes, SAF_CELLTYPE_POINT, 6, SAF_1DC(6), SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_ALL, &block_3, &domain_cat, SAF_CELLTYPE_SET, num_domain, SAF_1DC(num_domain), SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_3, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {4,4,1};   /* start, count, stride */
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_3, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {2};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* NOTE: this node subset relation is necessary because we are going to define a field on the nodes of BLOCK_3 */
            saf_declare_subset_relation(SAF_ALL, db, &top, &block_3, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {1,2,4,5,7,8};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "BLOCK_4" with pyramids */                
            saf_declare_set(SAF_ALL, db, "BLOCK_4", 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_4);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_ALL, &block_4, "EXO_ELEM_TYPE", H5T_C_S1, 1, "PYRAMID");

            saf_declare_collection(SAF_ALL, &block_4, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_4, &elems, SAF_CELLTYPE_PYRAMID, num_elem_in_blk[3], SAF_1DC(num_elem_in_blk[3]), 
                                   SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_4, &domain_cat, SAF_CELLTYPE_SET, num_domain, SAF_1DC(num_domain),
                                   SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_4, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {8,2,1};   /* start, count, stride */
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_4, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {3};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "BLOCK_5" with tets */                
            saf_declare_set(SAF_ALL, db, "BLOCK_5", 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_5);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_ALL, &block_5, "EXO_ELEM_TYPE", H5T_C_S1, 1, "TET");

            saf_declare_collection(SAF_ALL, &block_5, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_5, &elems, SAF_CELLTYPE_TET, num_elem_in_blk[4], SAF_1DC(num_elem_in_blk[4]), 
                                   SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &block_5, &domain_cat, SAF_CELLTYPE_SET, num_domain, SAF_1DC(num_domain),
                                   SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_5, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {10,2,1};   /* start, count, stride */
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_ALL, db, &top, &block_5, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            {
                int buf[] = {4};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "SIDE_SET_A"  */                
            saf_declare_set(SAF_ALL, db, "SIDE_SET_A", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_set_a);

            saf_declare_collection(SAF_ALL, &side_set_a, &side_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_ALL, &side_set_a, &elems, SAF_CELLTYPE_MIXED, 2, SAF_1DC(2), SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &top, &side_set_a, SAF_COMMON(&side_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[] = {0};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create subsets "SIDE_SET_A_QUADS" and "SIDE_SET_A_TRIS" which are sets of homogeneous primitives */                
            saf_declare_set(SAF_ALL, db, "SIDE_SET_A_QUADS", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_set_a_q);

            saf_declare_collection(SAF_ALL, &side_set_a_q, &elems, SAF_CELLTYPE_QUAD, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &side_set_a, &side_set_a_q, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {0};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_set(SAF_ALL, db, "SIDE_SET_A_TRIS", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_set_a_t);

            saf_declare_collection(SAF_ALL, &side_set_a_t, &elems, SAF_CELLTYPE_TRI, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &side_set_a, &side_set_a_t, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {1};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "SIDE_SET_B" */                
            saf_declare_set(SAF_ALL, db, "SIDE_SET_B", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_set_b);

            saf_declare_collection(SAF_ALL, &side_set_b, &side_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_ALL, &side_set_b, &elems, SAF_CELLTYPE_MIXED, 4, SAF_1DC(4), SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &top, &side_set_b, SAF_COMMON(&side_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[] = {1};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "NODE_SET_A" */                
            saf_declare_set(SAF_ALL, db, "NODE_SET_A", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &node_set_a);

            saf_declare_collection(SAF_ALL, &node_set_a, &node_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_ALL, &node_set_a, &nodes, SAF_CELLTYPE_POINT, 9, SAF_1DC(9), SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_ALL, db, &top, &node_set_a, SAF_COMMON(&node_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[] = {0};
                saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }
        }

        /* Now put out domain-local info.  the "downset" of each domain set (the part of the SRG below a domain set) is similar
         * to the global sets (sets for element blocks, node sets, and side sets) below the top set; the main difference is the
         * values in the subset relations; so here we duplicate the calls that created the global sets and just fill in
         * different subset relations */
        {
            /* Determine local number of elements. Just divide evenly across all domains */
            if (glob_num_elems < num_domain) {
                printf ("global number of elements is less than number of domains.\n");
                if (rank < glob_num_elems) loc_num_elems = 1;
                else loc_num_elems = 0;
            } else {
                loc_num_elems = glob_num_elems / num_domain;
                if (rank < (glob_num_elems % num_domain)) loc_num_elems++;
            }

            /* determine start and ending global indices of local elements within this domain MPI_Scan can assist with this */
#ifdef HAVE_PARALLEL
            MPI_Scan (&loc_num_elems, &end_elem_index_in_domain, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
#else
            end_elem_index_in_domain = loc_num_elems;
#endif
            begin_elem_index_in_domain = end_elem_index_in_domain - loc_num_elems;
            end_elem_index_in_domain--;

            /* calculate start and ending global indices of elements within each block */                
            begin_elem_index_in_blk = (int *) malloc (num_blocks * sizeof(int));
            end_elem_index_in_blk = (int *) malloc (num_blocks * sizeof(int));

            for (i=0, accum_num_elem=0; i<num_blocks; i++) {
                begin_elem_index_in_blk[i] = accum_num_elem;
                end_elem_index_in_blk[i] = begin_elem_index_in_blk[i] + num_elem_in_blk[i] - 1;
                accum_num_elem += num_elem_in_blk[i];
            }

            /* malloc things we need */
            len_connect = MAX_NUM_NODES_PER_ELEM * loc_num_elems;

            elem_map = (int *) malloc (loc_num_elems * sizeof(int));
            domain_connect = (int *) malloc (len_connect * sizeof(int));
            loc_connect = (int *) malloc (len_connect * sizeof(int));
            node_map = (int *) malloc (len_connect * sizeof(int));

            /* create element local/global map */
            create_elem_map (loc_num_elems, begin_elem_index_in_domain, elem_map);

            /* extract current domain's connectivity, referencing global node ids */
            extract_connect (glob_num_elems, elem_map, glob_connect, domain_connect);

            /* The local/global node map is just the current domain's connectivity, sorted with duplicate entries removed */
            create_node_map (len_connect, domain_connect, node_map, &loc_num_nodes);

            /* Using local/global node map, convert the domain connectivity (referencing global node ids) to local connectivity
             * (referencing local node ids) */
            create_local_connect (node_map, loc_num_nodes, len_connect, domain_connect, loc_connect);

            /* create a set for each domain in the decomposition each processor creates a set in SAF_EACH mode */
            sprintf (tmp_name, "DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_set);

            /* create collections in "DOMAIN_XX" for:
             * elements;
             * nodes;
             * element blocks;
             * side sets;
             * node sets;
             * domains;
             * node comm maps;
             * side comm maps */
            saf_declare_collection(SAF_EACH, &domain_set, &nodes, SAF_CELLTYPE_POINT, loc_num_nodes, SAF_1DC(loc_num_nodes), 
                                   SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_EACH, &domain_set, &elems, SAF_CELLTYPE_MIXED, loc_num_elems, SAF_1DC(loc_num_elems), 
                                   SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_set, &blocks, SAF_CELLTYPE_SET, num_blocks, SAF_1DC(num_blocks),
                                   SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_set, &side_sets, SAF_CELLTYPE_SET, num_side_sets, SAF_1DC(num_side_sets), 
                                   SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_EACH, &domain_set, &node_sets, SAF_CELLTYPE_SET, num_node_sets, SAF_1DC(num_node_sets), 
                                   SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_EACH, &domain_set, &domain_cat, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_set, &node_comm, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);
            saf_declare_collection(SAF_EACH, &domain_set, &side_comm, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);

            /* Declare subset relations of elements, nodes, domains, side sets, and node sets between top set and domain set */
            saf_declare_subset_relation(SAF_EACH, db, &top, &domain_set, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, elem_map, H5I_INVALID_HID, NULL, db);

            saf_declare_subset_relation(SAF_EACH, db, &top, &domain_set, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, node_map, H5I_INVALID_HID, NULL, db);

            saf_declare_subset_relation(SAF_EACH, db, &top, &domain_set, SAF_COMMON(&domain_cat), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &rank, H5I_INVALID_HID, NULL, db);

            /* write out the inter-processor node communication map containing:
             *  - a set of nodes that are shared by another processor
             *  - a field associated with each node of that set specifying a processor ID with which it is shared */
            {
                sprintf (tmp_name, "NODE_COMM_SET_%d", rank);
                saf_declare_set(SAF_EACH, db, tmp_name, 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &node_comm_set);

                saf_declare_collection(SAF_EACH, &node_comm_set, &node_comm, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);
                saf_declare_collection(SAF_EACH, &node_comm_set, &nodes, SAF_CELLTYPE_POINT, 2, SAF_1DC(2), SAF_DECOMP_FALSE);

                saf_declare_subset_relation(SAF_EACH, db, &domain_set, &node_comm_set, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL,
                                            H5I_INVALID_HID, NULL, &rel);
                {
                    int buf[] = {0,1};   /* node comm map node list; these are just made up */
                    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
                }

                saf_declare_subset_relation(SAF_EACH, db, &domain_set, &node_comm_set, SAF_COMMON(&node_comm), SAF_TUPLES, SAF_INT, NULL,
                                            H5I_INVALID_HID, NULL, &rel);
                {
                    int buf[] = {0};
                    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
                }

                sprintf (tmp_name, "node_comm_%d_tmpl", rank);
                saf_declare_field_tmpl(SAF_EACH, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                                       &node_comm_ftmpl);

                sprintf (tmp_name, "node_comm_procs_%d", rank);
                saf_declare_field(SAF_EACH, db, &node_comm_ftmpl, tmp_name, &node_comm_set, meter, SAF_SELF(db), &nodes, 1,
                                  &nodes, SAF_SPACE_PWLINEAR, SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                                  &node_comm_procs_fld);
                {
                    int buf[] = {1,2};  /* make up some proc IDS */
                    void *pbuf = &buf[0];
                    saf_write_field(SAF_EACH, &node_comm_procs_fld, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
                }

            }

            /* write out the inter-processor side communication map containing:
             *  - a set of sides that are shared by another processor; the sides are defined via a boundary relation
             *  - a field associated with each side of that set specifying a processor ID with which it is shared */
            {
                sprintf (tmp_name, "SIDE_COMM_SET_%d", rank);
                saf_declare_set(SAF_EACH, db, tmp_name, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_comm_set);

                saf_declare_collection(SAF_EACH, &side_comm_set, &side_comm, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);
                saf_declare_collection(SAF_EACH, &side_comm_set, &elems, SAF_CELLTYPE_MIXED, 2, SAF_1DC(2), SAF_DECOMP_FALSE);

                saf_declare_subset_relation(SAF_EACH, db, &domain_set, &side_comm_set, SAF_EMBEDBND(&elems,&elems), SAF_TUPLES, SAF_INT, 
                                            NULL, H5I_INVALID_HID, NULL, &rel);
                {
                    int abuf[] = {2,5};   /* side comm map element list */
                    int bbuf[] = {0,0};   /* side comm map side list */
                    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, abuf, SAF_INT, bbuf, db);
                }

                saf_declare_subset_relation(SAF_EACH, db, &domain_set, &side_comm_set, SAF_COMMON(&side_comm), SAF_TUPLES, SAF_INT, NULL,
                                            H5I_INVALID_HID, NULL, &rel);
                {
                    int buf[] = {0};
                    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
                }

                sprintf (tmp_name, "side_comm_%d_tmpl", rank);
                saf_declare_field_tmpl(SAF_EACH, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                                       &side_comm_ftmpl);

                sprintf (tmp_name, "side_comm_procs_%d", rank);
                saf_declare_field(SAF_EACH, db, &side_comm_ftmpl, tmp_name, &side_comm_set, meter, SAF_SELF(db),
                                  SAF_ZONAL(&elems), SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &side_comm_procs_fld);
                {
                    int buf[] = {3,4};  /* make up some proc IDS */
                    void *pbuf = &buf[0];
                    saf_write_field(SAF_EACH, &side_comm_procs_fld, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
                }
            }

            /* For each block in the domain:
             * create a set for the block (a subset of the domain)
             * define collections of elements, blocks, and domains
             * define element maps between block subset and domain set
             * define the mesh topology on the blocks
             * declare subset relations to the domain set (via block collection) and to the block set (via domain collection) */

            /* create "BLOCK_1" with quad shells */                
            sprintf (tmp_name, "BLOCK_1_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_block_1);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_EACH, &domain_block_1, "EXO_ELEM_TYPE", H5T_C_S1, 1, "SHELL");

            /* determine whether this block is completely on this processor, not on this proc, or split */                
            calc_in_or_out(begin_elem_index_in_blk[0], end_elem_index_in_blk[0], 
                           begin_elem_index_in_domain, end_elem_index_in_domain, 
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);

            saf_declare_collection(SAF_EACH, &domain_block_1, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_1, &elems, SAF_CELLTYPE_QUAD, num_elem_in_dom_blk,
                                   SAF_1DC(num_elem_in_dom_blk), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_1, &domain_cat, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

#if 0
            if (num_elem_in_dom_blk <= 0)  printf ("WARNING: element block 1 in domain %d is empty\n", rank);
#endif
            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_1, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_domain;
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;

                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_1, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {0};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &block_1, &domain_block_1, SAF_COMMON(&domain_cat), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &rank, H5I_INVALID_HID, NULL, db);

            saf_declare_subset_relation(SAF_EACH, db, &block_1, &domain_block_1, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_blk[0];
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;

                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* write out the topo relations (the connectivities of the domain blocks) */                
            saf_declare_topo_relation(SAF_EACH, db, &domain_block_1, &elems, &domain_set, &nodes, SAF_SELF(db), &domain_block_1,
                                      SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &dom_blk_trel);
            {
                int stride[] = {4};  /* stride (number of nodes per element) */
                int dom_blk_connect[8];  /* node list */

                offset = (begin_elem_index_in_dom_blk - begin_elem_index_in_domain) * MAX_NUM_NODES_PER_ELEM;
                extract_dom_block_connect(num_elem_in_dom_blk, &(loc_connect[offset]), dom_blk_connect);
                saf_write_topo_relation(SAF_EACH, &dom_blk_trel, SAF_INT, stride, SAF_INT, dom_blk_connect, db);
                create_node_map (8, dom_blk_connect, dom_blk_node_map, &num_nodes_in_dom_blk);
            }

            /* create an indirect topo relation on the global block; gather the handles of the topo relations on the domain
             * blocks and put them in the indirect topo relation */
            {
                int num_handles;
                SAF_Rel blk_trel, *dom_block_rels;

                /* gather field handles of topo relations for all domains and write out to (indirect) topo relation on global
                 * block */
                if (NULL==(dom_block_rels=(SAF_Rel*)saf_allgather_handles((ss_pers_t*)&dom_blk_trel, &num_handles, NULL)))
                    printf ("failed to gather topo relations for block 1\n");

                saf_declare_topo_relation(SAF_ALL, db, &block_1, &elems, &top, &nodes, &domain_cat, &block_1, SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL,
                                          H5I_INVALID_HID, NULL, &blk_trel);

                saf_write_topo_relation(SAF_ALL, &blk_trel, SAF_HANDLE, dom_block_rels, H5I_INVALID_HID, NULL, db);
            }

            /* NOTE: this node subset relation is necessary because we are going to define a field on the nodes of BLOCK_1 */
            saf_declare_collection(SAF_EACH, &domain_block_1, &nodes, SAF_CELLTYPE_POINT, num_nodes_in_dom_blk, 
                                   SAF_1DC(num_nodes_in_dom_blk), SAF_DECOMP_FALSE);
            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, dom_blk_node_map, H5I_INVALID_HID, NULL, db);

            /* create "BLOCK_2" with hexes */                
            sprintf (tmp_name, "BLOCK_2_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_block_2);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_EACH, &domain_block_2, "EXO_ELEM_TYPE", H5T_C_S1, 1, "HEX");

            /* determine whether this block is completely on this processor, not on this proc, or split */                
            calc_in_or_out(begin_elem_index_in_blk[1], end_elem_index_in_blk[1],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);

            saf_declare_collection(SAF_EACH, &domain_block_2, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_2, &elems, SAF_CELLTYPE_HEX, num_elem_in_dom_blk,
                                   SAF_1DC(num_elem_in_dom_blk), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_2, &domain_cat, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

            /* if (num_elem_in_dom_blk <= 0) printf ("WARNING: element block 2 in domain %d is empty\n", rank); */
            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_2, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_domain;
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;
                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_2, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {1};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &block_2, &domain_block_2, SAF_COMMON(&domain_cat), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &rank, H5I_INVALID_HID, NULL, db);

            saf_declare_subset_relation(SAF_EACH, db, &block_2, &domain_block_2, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_blk[1];
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;

                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* write out the topo relations (the connectivities of the domain blocks) */                
            saf_declare_topo_relation(SAF_EACH, db, &domain_block_2, &elems, &domain_set, &nodes, SAF_SELF(db), &domain_block_2,
                                      SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &dom_blk_trel);
            {
                int stride[] = {8};  /* stride (number of nodes per element) */
                int dom_blk_connect[16];  /* node list */

                offset = (begin_elem_index_in_dom_blk - begin_elem_index_in_domain) * MAX_NUM_NODES_PER_ELEM;
                extract_dom_block_connect(num_elem_in_dom_blk, &(loc_connect[offset]), dom_blk_connect);
                saf_write_topo_relation(SAF_EACH, &dom_blk_trel, SAF_INT, stride, SAF_INT, dom_blk_connect, db);
            }

            /* create an indirect topo relation on the global block; gather the handles of the topo relations on the domain
             * blocks and put them in the indirect topo relation */
            {
                int num_handles;
                SAF_Rel blk_trel, *dom_block_rels;

                /* gather field handles of topo relations for all domains and write out to (indirect) topo relation on global
                 * block */
                if (NULL==(dom_block_rels=(SAF_Rel*)saf_allgather_handles((ss_pers_t*)&dom_blk_trel, &num_handles, NULL)))
                    printf ("failed to gather topo relations for block 2\n");

                saf_declare_topo_relation(SAF_ALL, db, &block_2, &elems, &top, &nodes, &domain_cat, &block_2, SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL,
                                          H5I_INVALID_HID, NULL, &blk_trel);

                saf_write_topo_relation(SAF_ALL, &blk_trel, SAF_HANDLE, dom_block_rels, H5I_INVALID_HID, NULL, db);
            }

            /* create "BLOCK_3" with tri shells */                
            sprintf (tmp_name, "BLOCK_3_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_block_3);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_EACH, &domain_block_3, "EXO_ELEM_TYPE", H5T_C_S1, 1, "SHELL");

            /* determine whether this block is completely on this processor, not on this proc, or split */                
            calc_in_or_out(begin_elem_index_in_blk[2], end_elem_index_in_blk[2],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);

            saf_declare_collection(SAF_EACH, &domain_block_3, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_3, &elems, SAF_CELLTYPE_TRI, num_elem_in_dom_blk,
                                   SAF_1DC(num_elem_in_dom_blk), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_3, &domain_cat, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

            /* if (num_elem_in_dom_blk <= 0)  printf ("WARNING: element block 3 in domain %d is empty\n", rank); */
            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_3, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_domain;
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;
                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_3, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {2};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &block_3, &domain_block_3, SAF_COMMON(&domain_cat), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &rank, H5I_INVALID_HID, NULL, db);

            saf_declare_subset_relation(SAF_EACH, db, &block_3, &domain_block_3, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_blk[2];
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;

                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* write out the topo relations (the connectivities of the domain blocks) */                
            saf_declare_topo_relation(SAF_EACH, db, &domain_block_3, &elems, &domain_set, &nodes, SAF_SELF(db), &domain_block_3,
                                      SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &dom_blk_trel);
            {
                int stride[] = {3};  /* stride (number of nodes per element) */
                int dom_blk_connect[12];  /* node list */

                offset = (begin_elem_index_in_dom_blk - begin_elem_index_in_domain) * MAX_NUM_NODES_PER_ELEM;
                extract_dom_block_connect(num_elem_in_dom_blk, &(loc_connect[offset]), dom_blk_connect);
                saf_write_topo_relation(SAF_EACH, &dom_blk_trel, SAF_INT, stride, SAF_INT, dom_blk_connect, db);
                create_node_map (12, dom_blk_connect, dom_blk_node_map, &num_nodes_in_dom_blk);
            }

            /* create an indirect topo relation on the global block; gather the handles of the topo relations on the domain
             * blocks and put them in the indirect topo relation */
            {
                int num_handles;
                SAF_Rel blk_trel, *dom_block_rels;

                /* gather field handles of topo relations for all domains and write out to (indirect) topo relation on global
                 * block */
                if (NULL==(dom_block_rels=(SAF_Rel*)saf_allgather_handles((ss_pers_t*)&dom_blk_trel, &num_handles, NULL)))
                    printf ("failed to gather topo relations for block 3\n");

                saf_declare_topo_relation(SAF_ALL, db, &block_3, &elems, &top, &nodes, &domain_cat, &block_3, SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL,
                                          H5I_INVALID_HID, NULL, &blk_trel);

                saf_write_topo_relation(SAF_ALL, &blk_trel, SAF_HANDLE, dom_block_rels, H5I_INVALID_HID, NULL, db);
            }

            /* NOTE: this node subset relation is necessary because we are going to define a field on the nodes of BLOCK_3 */
            saf_declare_collection(SAF_EACH, &domain_block_3, &nodes, SAF_CELLTYPE_POINT, num_nodes_in_dom_blk, 
                                   SAF_1DC(num_nodes_in_dom_blk), SAF_DECOMP_FALSE);
            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_3, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, dom_blk_node_map, H5I_INVALID_HID, NULL, db);

            /* create "BLOCK_4" with pyramids */                
            sprintf (tmp_name, "BLOCK_4_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_block_4);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_EACH, &domain_block_4, "EXO_ELEM_TYPE", H5T_C_S1, 1, "PYRAMID");

            /* determine whether this block is completely on this processor, not on this proc, or split */                
            calc_in_or_out(begin_elem_index_in_blk[3], end_elem_index_in_blk[3],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);

            saf_declare_collection(SAF_EACH, &domain_block_4, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_4, &elems, SAF_CELLTYPE_PYRAMID, num_elem_in_dom_blk, 
                                   SAF_1DC(num_elem_in_dom_blk), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_4, &domain_cat, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

#if 0
            if (num_elem_in_dom_blk <= 0) printf ("WARNING: element block 4 in domain %d is empty\n", rank);
#endif
            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_4, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_domain;
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;
                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_4, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {3};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &block_4, &domain_block_4, SAF_COMMON(&domain_cat), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &rank, H5I_INVALID_HID, NULL, db);

            saf_declare_subset_relation(SAF_EACH, db, &block_4, &domain_block_4, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_blk[3];
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;

                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* write out the topo relations (the connectivities of the domain blocks) */                
            saf_declare_topo_relation(SAF_EACH, db, &domain_block_4, &elems, &domain_set, &nodes, SAF_SELF(db), &domain_block_4,
                                      SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &dom_blk_trel);
            {
                int stride[] = {5};  /* stride (number of nodes per element) */
                int dom_blk_connect[10];  /* node list */

                offset = (begin_elem_index_in_dom_blk - begin_elem_index_in_domain) * MAX_NUM_NODES_PER_ELEM;
                extract_dom_block_connect(num_elem_in_dom_blk, &(loc_connect[offset]), dom_blk_connect);
                saf_write_topo_relation(SAF_EACH, &dom_blk_trel, SAF_INT, stride, SAF_INT, dom_blk_connect, db);
            }

            /* create an indirect topo relation on the global block; gather the handles of the topo relations on the domain
             * blocks and put them in the indirect topo relation */
            {
                int num_handles;
                SAF_Rel blk_trel, *dom_block_rels;

                /* gather field handles of topo relations for all domains and write out to (indirect) topo relation on global
                 * block */
                if (NULL==(dom_block_rels=(SAF_Rel*)saf_allgather_handles((ss_pers_t*)&dom_blk_trel, &num_handles, NULL)))
                    printf ("failed to gather topo relations for block 4\n");

                saf_declare_topo_relation(SAF_ALL, db, &block_4, &elems, &top, &nodes, &domain_cat, &block_4, SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL,
                                          H5I_INVALID_HID, NULL, &blk_trel);

                saf_write_topo_relation(SAF_ALL, &blk_trel, SAF_HANDLE, dom_block_rels, H5I_INVALID_HID, NULL, db);
            }

            /* create "BLOCK_5" with tets */                
            sprintf (tmp_name, "BLOCK_5_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_block_5);

            /* optional attribute that EXODUS clients may want to know */                
            saf_put_set_att(SAF_EACH, &domain_block_5, "EXO_ELEM_TYPE", H5T_C_S1, 1, "TET");

            /* determine whether this block is completely on this processor, not on this proc, or split */                
            calc_in_or_out(begin_elem_index_in_blk[4], end_elem_index_in_blk[4],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);

            saf_declare_collection(SAF_EACH, &domain_block_5, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_5, &elems, SAF_CELLTYPE_TET, num_elem_in_dom_blk,
                                   SAF_1DC(num_elem_in_dom_blk), SAF_DECOMP_TRUE);
            saf_declare_collection(SAF_EACH, &domain_block_5, &domain_cat, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

#if 0
            if (num_elem_in_dom_blk <= 0) printf ("WARNING: element block 5 in domain %d is empty\n", rank);
#endif
            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_5, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_domain;
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;
                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_block_5, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {4};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &block_5, &domain_block_5, SAF_COMMON(&domain_cat), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &rank, H5I_INVALID_HID, NULL, db);

            saf_declare_subset_relation(SAF_EACH, db, &block_5, &domain_block_5, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[3];   /* start, count, stride */
                buf[0] = begin_elem_index_in_dom_blk - begin_elem_index_in_blk[4];
                buf[1] = num_elem_in_dom_blk;
                buf[2] = 1;

                if (begin_elem_index_in_dom_blk < 0) buf[0] = 0;
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* write out the topo relations (the connectivities of the domain blocks) */                
            saf_declare_topo_relation(SAF_EACH, db, &domain_block_5, &elems, &domain_set, &nodes, SAF_SELF(db), &domain_block_5,
                                      SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &dom_blk_trel);
            {
                int stride[] = {4};  /* stride (number of nodes per element) */
                int dom_blk_connect[8];  /* node list */

                offset = (begin_elem_index_in_dom_blk - begin_elem_index_in_domain) * MAX_NUM_NODES_PER_ELEM;
                extract_dom_block_connect(num_elem_in_dom_blk, &(loc_connect[offset]), dom_blk_connect);
                saf_write_topo_relation(SAF_EACH, &dom_blk_trel, SAF_INT, stride, SAF_INT, dom_blk_connect, db);
            }

            /* create an indirect topo relation on the global block; gather the handles of the topo relations on the domain
             * blocks and put them in the indirect topo relation */
            {
                int num_handles;
                SAF_Rel blk_trel, *dom_block_rels;

                /* gather field handles of topo relations for all domains and write out to (indirect) topo relation on global
                 * block */         
                if (NULL==(dom_block_rels=(SAF_Rel*)saf_allgather_handles((ss_pers_t*)&dom_blk_trel, &num_handles, NULL)))
                    printf ("failed to gather topo relations for block 5\n");

                saf_declare_topo_relation(SAF_ALL, db, &block_5, &elems, &top, &nodes, &domain_cat, &block_5,
                                          SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &blk_trel);

                saf_write_topo_relation(SAF_ALL, &blk_trel, SAF_HANDLE, dom_block_rels, H5I_INVALID_HID, NULL, db);
            }

            /* create "SIDE_SET_A" */                
            sprintf (tmp_name, "SIDE_SET_A_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_ssa);

            saf_declare_collection(SAF_EACH, &domain_ssa, &side_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);

            /* declare SIDE_SET_A on all domains but all will be empty except the one on domain_0; the side set *could* be
             * distributed across processors but just for convenience for this client, we put the whole side set on proc 0;
             * because of this, the side set element list isn't valid because it references global element ID's, not
             * processor-local IDs; an actual client would have the side set decomposition (which may be different from the
             * primary element decomposition) readily available and would use it to determine the element list */
            if (rank == 0)
                num_sides = 2;
            else
                num_sides = 0;

            saf_declare_collection(SAF_EACH, &domain_ssa, &elems, SAF_CELLTYPE_MIXED, num_sides, SAF_1DC(num_sides),
                                   SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_ssa, SAF_EMBEDBND(&elems,&elems), SAF_TUPLES, SAF_INT, NULL,
                                        H5I_INVALID_HID, NULL, &rel);
            {
                int abuf[] = {2,10};   /* side set element list */
                int bbuf[] = {5,2};   /* side set side list  */

                /* even though all procs call this, only proc 0 will write anything because it's the only proc that has
                 * declared a collection with count > 0 */
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, abuf, SAF_INT, bbuf, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_ssa, SAF_COMMON(&side_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {0};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create subsets "SIDE_SET_A_QUADS" and "SIDE_SET_A_TRIS" which are sets of homogeneous primitives */                
            sprintf (tmp_name, "SIDE_SET_A_QUADS_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_ssa_q);

            if (rank == 0)
                num_sides = 1;
            else
                num_sides = 0;

            saf_declare_collection(SAF_EACH, &domain_ssa_q, &elems, SAF_CELLTYPE_QUAD, num_sides, SAF_1DC(num_sides),
                                   SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_EACH, db, &domain_ssa, &domain_ssa_q, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {0};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            sprintf (tmp_name, "SIDE_SET_A_TRIS_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_ssa_t);

            if (rank == 0)
                num_sides = 1;
            else
                num_sides = 0;

            saf_declare_collection(SAF_EACH, &domain_ssa_t, &elems, SAF_CELLTYPE_TRI, num_sides, SAF_1DC(num_sides),
                                   SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_EACH, db, &domain_ssa, &domain_ssa_t, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {1};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "SIDE_SET_B" */                
            sprintf (tmp_name, "SIDE_SET_B_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_ssb);

            saf_declare_collection(SAF_EACH, &domain_ssb, &side_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);

            if (rank == 0)
                num_sides = 4;
            else
                num_sides = 0;

            saf_declare_collection(SAF_EACH, &domain_ssb, &elems, SAF_CELLTYPE_MIXED, num_sides, SAF_1DC(num_sides),
                                   SAF_DECOMP_TRUE);

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_ssb, SAF_EMBEDBND(&elems,&elems), SAF_TUPLES, SAF_INT, NULL,
                                        H5I_INVALID_HID, NULL, &rel);
            {
                int abuf[] = {2,5,3,6};   /* side set element list */
                int bbuf[] = {0,0,0,0};   /* side set side list; yes, all of these are the 0th face of the associated elements  */
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, abuf, SAF_INT, bbuf, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_ssb, SAF_COMMON(&side_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {1};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            /* create "NODE_SET_A" */                
            sprintf (tmp_name, "NODE_SET_A_DOMAIN_%d", rank);
            saf_declare_set(SAF_EACH, db, tmp_name, 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain_nsa);

            saf_declare_collection(SAF_EACH, &domain_nsa, &node_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);

            if (rank == 0)
                num_nodes = 9;
            else
                num_nodes = 0;

            saf_declare_collection(SAF_EACH, &domain_nsa, &nodes, SAF_CELLTYPE_POINT, num_nodes, SAF_1DC(num_nodes),
                                   SAF_DECOMP_FALSE);

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_nsa, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL,
                                        &rel);
            {
                int buf[] = {0,1,2,3,4,5,6,7,8};   /* node set node list */
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }

            saf_declare_subset_relation(SAF_EACH, db, &domain_set, &domain_nsa, SAF_COMMON(&node_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                        NULL, &rel);
            {
                int buf[] = {0};
                saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
            }
        }

        /* OK, all the sets are written out now write out the fields */

        /* put all of the next fields into a single state of a suite named "INIT_SUITE" */            
        cnt = 0;

        /* field of coordinates of nodes on TOP_SET; this will be an indirect field containing the IDs of the coordinate fields
         * defined on the domain sets; must first define the fields on the domain sets */

        /* first, declare field templates for the component (X, Y, Z) fields; then declare field templates for the composite
         * (XYZ) field */
        saf_declare_field_tmpl(SAF_EACH, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                               &coords_ctmpl);

        tmp_ftmpl[0] = coords_ctmpl;
        tmp_ftmpl[1] = coords_ctmpl;
        tmp_ftmpl[2] = coords_ctmpl;

        saf_declare_field_tmpl(SAF_EACH, db, "coordinate_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3, tmp_ftmpl,
                               &coords_ftmpl);

        /* declare the component and composite coordinate fields */
        saf_declare_field(SAF_EACH, db, &coords_ctmpl, "X", &domain_set, meter, SAF_SELF(db),  SAF_NODAL(&nodes, &elems),
                          SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_coords);
        saf_declare_field(SAF_EACH, db, &coords_ctmpl, "Y", &domain_set, meter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_coords);
        saf_declare_field(SAF_EACH, db, &coords_ctmpl, "Z", &domain_set, meter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_coords);

        tmp_fields[0] = x_coords;
        tmp_fields[1] = y_coords;
        tmp_fields[2] = z_coords;
        coord_components = tmp_fields;

        saf_declare_field(SAF_EACH, db, &coords_ftmpl, "coords_on_domains", &domain_set, meter, SAF_SELF(db), 
                          SAF_NODAL(&nodes, &elems), SAF_FLOAT, coord_components, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL,
                          &coords);
        {
            /* X coordinates */
            float buf[MAX_NUM_NODES];  /* max dimension */
            void *pbuf = &buf[0];

            for (i=0; i<loc_num_nodes; i++) {
                /* extract from global coordinates */
                buf[i] = x[node_map[i]];
            }
            saf_write_field(SAF_EACH, &x_coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
        }
        {
            /* Y coordinates */
            float buf[MAX_NUM_NODES];  /* max dimension */
            void *pbuf = &buf[0];

            for (i=0; i<loc_num_nodes; i++) {
                /* extract from global coordinates */
                buf[i] = y[node_map[i]];
            }
            saf_write_field(SAF_EACH, &y_coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
        }
        {
            /* Z coordinates */
            float buf[MAX_NUM_NODES];  /* max dimension */
            void *pbuf = &buf[0];

            for (i=0; i<loc_num_nodes; i++) {
                /* extract from global coordinates */
                buf[i] = z[node_map[i]];
            }
            saf_write_field(SAF_EACH, &z_coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
        }

        /* specify that this is a coordinate field */
        saf_declare_coords(SAF_EACH, &coords);
        saf_declare_default_coords(SAF_EACH, &domain_set, &coords);

        /* create indirect field for coordinates on top set */
        {
            int num_handles;
            SAF_Field *dom_coord_fields;
            void *pbuf;

            saf_declare_field_tmpl(SAF_ALL, db, "coords_on_top_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                   SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &coords_on_top_ftmpl);
            saf_declare_field(SAF_ALL, db, &coords_on_top_ftmpl, "coords_on_top", &top, NULL, &domain_cat,
                              SAF_NODAL(&nodes, &elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &coords_on_top);

            /* gather field handles of coordinates for all domains and write out to (indirect) coordinate field on top */
            dom_coord_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&coords, &num_handles, NULL);

            pbuf = &(dom_coord_fields[0]);
            saf_write_field(SAF_ALL, &coords_on_top, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);

            saf_declare_coords(SAF_ALL, &coords_on_top);
            saf_declare_default_coords(SAF_ALL, &top, &coords_on_top);
        }

        field_list[cnt] = coords_on_top;
        field_tmpl_list[cnt] = coords_on_top_ftmpl;
        cnt++;

        /* put the next fields (element thicknesses, distribution factors, etc.) on the global element blocks, side sets, and
         * node sets; they could just as easily be put out on the "domain blocks" */

        /* field of constant shell thicknesses of elements in BLOCK_1 1 thickness will be specified per element (constant thru
         * element) */
        saf_declare_field_tmpl(SAF_ALL, db, "blk_1_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                               &blk_1_elem_ftmpl);

        saf_declare_field(SAF_ALL, db, &blk_1_elem_ftmpl, "elem_thickness", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
                          SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &thickness);
        {
            /* shell thicknesses for BLOCK_1 (1 per element) */
            float buf[] = {0.01, 0.02};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &thickness, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
        }

        field_list[cnt] = thickness;
        field_tmpl_list[cnt] = blk_1_elem_ftmpl;
        cnt++;

        /* field of varying shell thicknesses of elements in BLOCK_3
         *
         * 3 thicknesses will be specified per element (1 per node)
         *
         * NOTE: this field is not specified to be associated with a node collection the block, which would enforce thickness
         *       continuity at the nodes;
         *
         * rather the thicknesses are specified associated with the elements (3 per element) which allows for discontinuity at
         * the nodes
         *
         * the locations at which the thicknesses are associated, as well as the ordering, must currently be assumed (at the
         * nodes of the tris, in the order of the nodes in the topo relations) */
        saf_declare_field_tmpl(SAF_ALL, db, "blk_3_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                               &blk_3_elem_ftmpl);

        saf_declare_field(SAF_ALL, db, &blk_3_elem_ftmpl, "elem_thickness", &block_3, NULL, SAF_SELF(db), &elems, 3, &elems,
                          SAF_SPACE_PWLINEAR, SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &thickness);
        {
            /* shell thicknesses for BLOCK_3 (3 per element) */
            float buf[] = {.01, .02, .03,  .01, .02, .03,  .01, .02, .03,  .01, .02, .03};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &thickness, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
        }

        field_list[cnt] = thickness;
        field_tmpl_list[cnt] = blk_3_elem_ftmpl;
        cnt++;

        /* field of distribution factors at nodes of SIDE_SET_A these will be fields on the homogeneous subsets of SIDE_SET_A
         * (i.e., SIDE_SET_A_TRIS and SIDE_SET_A_QUADS) */
        saf_declare_field_tmpl(SAF_ALL, db, "ssaq_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                               &ssaq_elem_ftmpl);

        saf_declare_field(SAF_ALL, db, &ssaq_elem_ftmpl, "dist_factor", &side_set_a_q, NULL, SAF_SELF(db), &elems, 4, &elems,
                          SAF_SPACE_PWLINEAR, SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &dist_fact);
        {
            /* distribution factors (4 per element) */
            float buf[] = {.1, .2, .3, .4};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &dist_fact, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
        }

        field_list[cnt] = dist_fact;
        field_tmpl_list[cnt] = ssaq_elem_ftmpl;
        cnt++;

        saf_declare_field_tmpl(SAF_ALL, db, "ssat_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                               &ssat_elem_ftmpl);

        saf_declare_field(SAF_ALL, db, &ssat_elem_ftmpl, "dist_factor", &side_set_a_t, NULL, SAF_SELF(db), &elems, 3, &elems,
                          SAF_SPACE_PWLINEAR, SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &dist_fact);
        {
            /* distribution factors (3 per element) */
            float buf[] = {.1, .2, .3};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &dist_fact, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
        }

        field_list[cnt] = dist_fact;
        field_tmpl_list[cnt] = ssat_elem_ftmpl;
        cnt++;

        /* field of distribution factors at nodes of NODE_SET_A */
        saf_declare_field_tmpl(SAF_ALL, db, "nsa_node_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
                               NULL, &nsa_node_ftmpl);

        /* saf_declare_field(SAF_ALL, db, &nsa_node_ftmpl, "dist_factor", node_set_a, NULL, SAF_SELF(db),
         * SAF_NODAL(nodes,elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &dist_fact); */
        saf_declare_field(SAF_ALL, db, &nsa_node_ftmpl, "dist_factor", &node_set_a, NULL, SAF_SELF(db), &nodes, 1, &nodes,
                          SAF_SPACE_PWLINEAR, SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &dist_fact);
        {
            /* distribution factors (1 per node) */
            float buf[] = {.1, .2, .3, .4, .5, .6, .7, .8, .9};
            void *pbuf = &buf[0];
            saf_write_field(SAF_ALL, &dist_fact, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
        }

        field_list[cnt] = dist_fact;
        field_tmpl_list[cnt] = nsa_node_ftmpl;
        cnt++;

        /* create a suite and state field into which the accumulated field handles will be inserted */            
        time[0] = 0.0;
        state=0;
        saf_declare_suite(SAF_ALL,db,"TIME_SUITE",&top,NULL,&suite);
        saf_declare_state_tmpl(SAF_ALL,db, "TIME_SUITE_INIT_TMPL", cnt, field_tmpl_list, &init_stmpl);
        saf_declare_state_group(SAF_ALL,db,"TIME_STATE_INIT_GROUP",&suite,&top,&init_stmpl,SAF_QTIME,SAF_ANY_UNIT,
                                SAF_FLOAT,&init_state_grp);
        saf_write_state(SAF_ALL, &init_state_grp, state, &top, SAF_FLOAT, &time[0], field_list);

        /* put the next fields into the first state (time = 1.0) and second state (time = 2.0) of a suite named "TIME_SUITE" */
        time[0] = 1.0;
        time[1] = 2.0;

        for (state=0; state<2; state++) {
            cnt=0;

            /* displacement vector field associated with nodes of each domain set.  first, declare field templates for the
            * component (DX, DY, DZ) fields; then declare field templates for the composite (DISP) field */
            saf_declare_field_tmpl(SAF_EACH, db, "domain_node_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                                   &domain_node_ctmpl);

            tmp_ftmpl[0] = domain_node_ctmpl;
            tmp_ftmpl[1] = domain_node_ctmpl;
            tmp_ftmpl[2] = domain_node_ctmpl;

            saf_declare_field_tmpl(SAF_EACH, db, "domain_node_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3, tmp_ftmpl,
                                   &domain_node_ftmpl);
  
            /* declare the component and composite displacement fields */
            saf_declare_field(SAF_EACH, db, &domain_node_ctmpl, "DISP_X", &domain_set, meter, SAF_SELF(db),
                              SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_disp);
            saf_declare_field(SAF_EACH, db, &domain_node_ctmpl, "DISP_Y", &domain_set, meter, SAF_SELF(db),
                              SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_disp);
            saf_declare_field(SAF_EACH, db, &domain_node_ctmpl, "DISP_Z", &domain_set, meter, SAF_SELF(db),
                              SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_disp);

            tmp_fields[0] = x_disp;
            tmp_fields[1] = y_disp;
            tmp_fields[2] = z_disp;

            saf_declare_field(SAF_EACH, db, &domain_node_ftmpl, "displacement", &domain_set, meter, SAF_SELF(db), 
                              SAF_NODAL(&nodes, &elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &disp);
            {
                /* X displacements */
                float buf[MAX_NUM_NODES];  /* max dimension */
                void *pbuf = &buf[0];

                for (i=0; i<loc_num_nodes; i++) {
                    /* extract from global displacements; multiply by state to get different field values at each state */
                    buf[i] = dx[node_map[i]] * state;
                }

                saf_write_field(SAF_EACH, &x_disp, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* Y displacements */
                float buf[MAX_NUM_NODES];  /* max dimension */
                void *pbuf = &buf[0];

                for (i=0; i<loc_num_nodes; i++) {
                    /* extract from global displacements; multiply by state to get different field values at each state */
                    buf[i] = dy[node_map[i]] * state;
                }

                saf_write_field(SAF_EACH, &y_disp, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* Z displacements */
                float buf[MAX_NUM_NODES];  /* max dimension */
                void *pbuf = &buf[0];

                for (i=0; i<loc_num_nodes; i++) {
                    /* extract from global displacements; multiply by state to get different field values at each state */
                    buf[i] = dz[node_map[i]] * state;
                }

                saf_write_field(SAF_EACH, &z_disp, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for displacements on top set */
            {
                int num_handles;
                SAF_Field *dom_disp_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "disp_on_top_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &disp_on_top_ftmpl);
                saf_declare_field(SAF_ALL, db, &disp_on_top_ftmpl, "disp_on_top", &top, NULL, &domain_cat,
                                  SAF_NODAL(&nodes, &elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &disp_on_top);

                /* gather field handles of displacements for all domains and write out to (indirect) displacement field on top */
                dom_disp_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&disp, &num_handles, NULL);

                pbuf = &(dom_disp_fields[0]);
                saf_write_field(SAF_ALL, &disp_on_top, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = disp_on_top;
            field_tmpl_list[cnt] = disp_on_top_ftmpl;
            cnt++;

            /* velocity vector field associated with nodes of each domain set */

            /* first, declare field templates for the component (VX, VY, VZ) fields; then declare field templates for the
             * composite (VEL) field */
            saf_declare_field_tmpl(SAF_EACH, db, "domain_node_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, vel_q, 1, NULL,
                                   &domain_node_ctmpl);

            tmp_ftmpl[0] = domain_node_ctmpl;
            tmp_ftmpl[1] = domain_node_ctmpl;
            tmp_ftmpl[2] = domain_node_ctmpl;

            saf_declare_field_tmpl(SAF_EACH, db, "dom_node_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, vel_q, 3, tmp_ftmpl,
                                   &domain_node_ftmpl);

            /* declare the component and composite velocity fields */                    
            saf_declare_field(SAF_EACH, db, &domain_node_ctmpl, "VEL_X", &domain_set, m_per_s, SAF_SELF(db),
                              SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_vel);
            saf_declare_field(SAF_EACH, db, &domain_node_ctmpl, "VEL_Y", &domain_set, m_per_s, SAF_SELF(db),
                              SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_vel);
            saf_declare_field(SAF_EACH, db, &domain_node_ctmpl, "VEL_Z", &domain_set, m_per_s, SAF_SELF(db),
                              SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_vel);

            tmp_fields[0] = x_vel;
            tmp_fields[1] = y_vel;
            tmp_fields[2] = z_vel;

            saf_declare_field(SAF_EACH, db, &domain_node_ftmpl, "velocity", &domain_set, m_per_s, SAF_SELF(db), 
                              SAF_NODAL(&nodes, &elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &vel);
            {
                /* X velocities */
                float buf[MAX_NUM_NODES];
                void *pbuf = &buf[0];

                for (i=0; i<loc_num_nodes; i++) {
                    /* extract from global velocities; multiply by state to get different field values at each state */
                    buf[i] = xvel[node_map[i]] * state;
                }
                saf_write_field(SAF_EACH, &x_vel, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* Y velocities */
                float buf[MAX_NUM_NODES];
                void *pbuf = &buf[0];

                for (i=0; i<loc_num_nodes; i++) {
                    /* extract from global velocities; multiply by state to get different field values at each state */
                    buf[i] = yvel[node_map[i]] * state;
                }
                saf_write_field(SAF_EACH, &y_vel, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* Z velocities */
                float buf[MAX_NUM_NODES];
                void *pbuf = &buf[0];

                for (i=0; i<loc_num_nodes; i++) {
                    /* extract from global velocities; multiply by state to get different field values at each state */
                    buf[i] = zvel[node_map[i]] * state;
                }
                saf_write_field(SAF_EACH, &z_vel, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for velocities on top set */
            {
                int num_handles;
                SAF_Field *dom_vel_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "vel_on_top_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &vel_on_top_ftmpl);
                saf_declare_field(SAF_ALL, db, &vel_on_top_ftmpl, "vel_on_top", &top, NULL, &domain_cat,
                                  SAF_NODAL(&nodes, &elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &vel_on_top);

                /* gather field handles of velocities for all domains and write out to (indirect) velocity field on top */
                dom_vel_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&vel, &num_handles, NULL);

                pbuf = &(dom_vel_fields[0]);
                saf_write_field(SAF_ALL, &vel_on_top, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = vel_on_top;
            field_tmpl_list[cnt] = vel_on_top_ftmpl;
            cnt++;

            /*stress symmetric tensor field associated with elements of BLOCK_2 and BLOCK_5
             *     
             * even if BLOCK_2 and BLOCK_5 are NULL (have no elements) on a given processor, we will declare and write to a
             * stress field */

            /* first, declare field templates for the component (SIGXX, SIGYY, SIGZZ, SIGXY, SIGYZ, SIGZX) fields; then declare
             * field templates for the composite (SIG) field */

            /* stress on BLOCK_2 */                    
            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_2_elem_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, stress_q, 1, NULL,
                                   &dom_block_2_elem_ctmpl);

            tmp_ftmpl[0] = dom_block_2_elem_ctmpl;
            tmp_ftmpl[1] = dom_block_2_elem_ctmpl;
            tmp_ftmpl[2] = dom_block_2_elem_ctmpl;
            tmp_ftmpl[3] = dom_block_2_elem_ctmpl;
            tmp_ftmpl[4] = dom_block_2_elem_ctmpl;
            tmp_ftmpl[5] = dom_block_2_elem_ctmpl;

            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_2_elem_ftmpl", SAF_ALGTYPE_SYMTENSOR, SAF_CARTESIAN, stress_q, 6,
                                   tmp_ftmpl, &dom_block_2_elem_ftmpl);

            /* declare the component and composite stress fields */                    
            saf_declare_field(SAF_EACH, db, &dom_block_2_elem_ctmpl, "SIGXX", &domain_block_2, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigxx);
            saf_declare_field(SAF_EACH, db, &dom_block_2_elem_ctmpl, "SIGYY", &domain_block_2, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigyy);
            saf_declare_field(SAF_EACH, db, &dom_block_2_elem_ctmpl, "SIGZZ", &domain_block_2, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigzz);
            saf_declare_field(SAF_EACH, db, &dom_block_2_elem_ctmpl, "SIGXY", &domain_block_2, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigxy);
            saf_declare_field(SAF_EACH, db, &dom_block_2_elem_ctmpl, "SIGYZ", &domain_block_2, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigyz);
            saf_declare_field(SAF_EACH, db, &dom_block_2_elem_ctmpl, "SIGZX", &domain_block_2, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigzx);

            tmp_fields[0] = sigxx;
            tmp_fields[1] = sigyy;
            tmp_fields[2] = sigzz;
            tmp_fields[3] = sigxy;
            tmp_fields[4] = sigyz;
            tmp_fields[5] = sigzx;

            saf_declare_field(SAF_EACH, db, &dom_block_2_elem_ftmpl, "stress", &domain_block_2, pascal, SAF_SELF(db), 
                              SAF_ZONAL(&elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &stress);

            /* determine which elements in the domain are in element block 2 so we can extract values from the global stress
             * field */
            calc_in_or_out(begin_elem_index_in_blk[1], end_elem_index_in_blk[1],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);
            {
                /* SIGXX */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigxx_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &sigxx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGYY */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigyy_vals[begin_elem_index_in_dom_blk+i];
                saf_write_field(SAF_EACH, &sigyy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGZZ */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigzz_vals[begin_elem_index_in_dom_blk+i];
                saf_write_field(SAF_EACH, &sigzz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGXY */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigxy_vals[begin_elem_index_in_dom_blk+i];
                saf_write_field(SAF_EACH, &sigxy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGYZ */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigyz_vals[begin_elem_index_in_dom_blk+i];
                saf_write_field(SAF_EACH, &sigyz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGZX */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigzx_vals[begin_elem_index_in_dom_blk+i];
                saf_write_field(SAF_EACH, &sigzx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for stress on global block 2 */
            {
                int num_handles;
                SAF_Field *dom_stress_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "stress_on_blk2_tmpl", SAF_ALGTYPE_FIELD, NULL, 
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &stress_on_blk2_ftmpl);
                saf_declare_field(SAF_ALL, db, &stress_on_blk2_ftmpl, "stress_on_blk2", &block_2, NULL, &domain_cat,
                                  SAF_ZONAL(&elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &stress_on_blk2);

                /* gather field handles of stress for all block2 domains and write out to (indirect) stress field on block 2 */
                dom_stress_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&stress, &num_handles, NULL);

                pbuf = &(dom_stress_fields[0]);
                saf_write_field(SAF_ALL, &stress_on_blk2, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = stress_on_blk2;
            field_tmpl_list[cnt] = stress_on_blk2_ftmpl;
            cnt++;

            /* stress on BLOCK_5 */                    
            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_5_elem_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, stress_q, 1, NULL,
                                   &dom_block_elem_ctmpl);

            tmp_ftmpl[0] = dom_block_elem_ctmpl;
            tmp_ftmpl[1] = dom_block_elem_ctmpl;
            tmp_ftmpl[2] = dom_block_elem_ctmpl;
            tmp_ftmpl[3] = dom_block_elem_ctmpl;
            tmp_ftmpl[4] = dom_block_elem_ctmpl;
            tmp_ftmpl[5] = dom_block_elem_ctmpl;

            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_5_elem_ftmpl", SAF_ALGTYPE_SYMTENSOR, SAF_CARTESIAN, stress_q, 6,
                                   tmp_ftmpl, &dom_block_elem_ftmpl);

            /* declare the component and composite stress fields */                    
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "SIGXX", &domain_block_5, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigxx);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "SIGYY", &domain_block_5, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigyy);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "SIGZZ", &domain_block_5, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigzz);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "SIGXY", &domain_block_5, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigxy);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "SIGYZ", &domain_block_5, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigyz);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "SIGZX", &domain_block_5, pascal, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigzx);
  
            tmp_fields[0] = sigxx;
            tmp_fields[1] = sigyy;
            tmp_fields[2] = sigzz;
            tmp_fields[3] = sigxy;
            tmp_fields[4] = sigyz;
            tmp_fields[5] = sigzx;

            saf_declare_field(SAF_EACH, db, &dom_block_elem_ftmpl, "stress", &domain_block_5, NULL, SAF_SELF(db), 
                              SAF_ZONAL(&elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &stress);

            /* determine which elements in the domain are in element block 5 so we can extract values from the global stress
             * field */
            calc_in_or_out(begin_elem_index_in_blk[4], end_elem_index_in_blk[4],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);
            {
                /* SIGXX */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigxx_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &sigxx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGYY */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigyy_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &sigyy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGZZ */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigzz_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &sigzz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGXY */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigxy_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &sigxy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGYZ */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigyz_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &sigyz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* SIGZX */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = sigzx_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &sigzx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for stress on global block 5 */
            {
                int num_handles;
                SAF_Field *dom_stress_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "stress_on_blk5_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &stress_on_blk5_ftmpl);
                saf_declare_field(SAF_ALL, db, &stress_on_blk5_ftmpl, "stress_on_blk5", &block_5, NULL, &domain_cat,
                                  SAF_ZONAL(&elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &stress_on_blk5);

                /* gather field handles of stress for all block5 domains and write out to (indirect) stress field on block 5 */
                dom_stress_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&stress, &num_handles, NULL);

                pbuf = &(dom_stress_fields[0]);
                saf_write_field(SAF_ALL, &stress_on_blk5, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = stress_on_blk5;
            field_tmpl_list[cnt] = stress_on_blk5_ftmpl;
            cnt++;

            /* strain symmetric tensor field associated with elements of BLOCK_1 and BLOCK_3 */

            /* first, declare field templates for the component (EPSXX, EPSYY, EPSZZ, EPSXY, EPSYZ, EPSZX) fields; then declare
             * field templates for the composite (EPS) field */
            
            /* strain on BLOCK_1 */                    
            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_1_elem_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, strain_q, 1, NULL,
                                   &dom_block_elem_ctmpl);

            tmp_ftmpl[0] = dom_block_elem_ctmpl;
            tmp_ftmpl[1] = dom_block_elem_ctmpl;
            tmp_ftmpl[2] = dom_block_elem_ctmpl;
            tmp_ftmpl[3] = dom_block_elem_ctmpl;
            tmp_ftmpl[4] = dom_block_elem_ctmpl;
            tmp_ftmpl[5] = dom_block_elem_ctmpl;

            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_1_elem_ftmpl", SAF_ALGTYPE_SYMTENSOR, SAF_CARTESIAN, strain_q, 6,
                                   tmp_ftmpl, &dom_block_elem_ftmpl);

            /* declare the component and composite strain fields */                    
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSXX", &domain_block_1, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsxx);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSYY", &domain_block_1, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsyy);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSZZ", &domain_block_1, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epszz);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSXY", &domain_block_1, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsxy);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSYZ", &domain_block_1, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsyz);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSZX", &domain_block_1, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epszx);

            tmp_fields[0] = epsxx;
            tmp_fields[1] = epsyy;
            tmp_fields[2] = epszz;
            tmp_fields[3] = epsxy;
            tmp_fields[4] = epsyz;
            tmp_fields[5] = epszx;
  
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ftmpl, "strain", &domain_block_1, NULL, SAF_SELF(db), 
                              SAF_ZONAL(&elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &strain);
    
            /* determine which elements in the domain are in element block 1 so we can extract values from the global stress
             * field */
            calc_in_or_out(begin_elem_index_in_blk[0], end_elem_index_in_blk[0],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);
            {
                /* EPSXX */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epsxx_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epsxx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSYY */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epsyy_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epsyy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSZZ */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epszz_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epszz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSXY */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epsxy_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epsxy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSYZ */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epsyz_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epsyz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSZX */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epszx_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epszx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for strain on global block 1 */
            {
                int num_handles;
                SAF_Field *dom_strain_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "strain_on_blk1_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &strain_on_blk1_ftmpl);
                saf_declare_field(SAF_ALL, db, &strain_on_blk1_ftmpl, "strain_on_blk1", &block_1, NULL, &domain_cat, 
                                  SAF_ZONAL(&elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &strain_on_blk1);

                /* gather field handles of strain for all block1 domains and write out to (indirect) strain field on block 1 */
                dom_strain_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&strain, &num_handles, NULL);

                pbuf = &(dom_strain_fields[0]);
                saf_write_field(SAF_ALL, &strain_on_blk1, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = strain_on_blk1;
            field_tmpl_list[cnt] = strain_on_blk1_ftmpl;
            cnt++;

            /* strain on BLOCK_3 */                    
            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_3_elem_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, strain_q, 1, NULL,
                                   &dom_block_elem_ctmpl);

            tmp_ftmpl[0] = dom_block_elem_ctmpl;
            tmp_ftmpl[1] = dom_block_elem_ctmpl;
            tmp_ftmpl[2] = dom_block_elem_ctmpl;
            tmp_ftmpl[3] = dom_block_elem_ctmpl;
            tmp_ftmpl[4] = dom_block_elem_ctmpl;
            tmp_ftmpl[5] = dom_block_elem_ctmpl;

            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_3_elem_ftmpl", SAF_ALGTYPE_SYMTENSOR, SAF_CARTESIAN, strain_q, 6,
                                   tmp_ftmpl, &dom_block_elem_ftmpl);

            /* declare the component and composite strain fields */                    
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSXX", &domain_block_3, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsxx);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSYY", &domain_block_3, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsyy);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSZZ", &domain_block_3, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epszz);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSXY", &domain_block_3, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsxy);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSYZ", &domain_block_3, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsyz);
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "EPSZX", &domain_block_3, NULL, SAF_SELF(db),
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epszx);

            tmp_fields[0] = epsxx;
            tmp_fields[1] = epsyy;
            tmp_fields[2] = epszz;
            tmp_fields[3] = epsxy;
            tmp_fields[4] = epsyz;
            tmp_fields[5] = epszx;

            saf_declare_field(SAF_EACH, db, &dom_block_elem_ftmpl, "strain", &domain_block_3, NULL, SAF_SELF(db), 
                              SAF_ZONAL(&elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &strain);

            /* determine which elements in the domain are in element block 3 so we can extract values from the global stress
             * field */
            calc_in_or_out(begin_elem_index_in_blk[2], end_elem_index_in_blk[2],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);
            {
                /* EPSXX */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epsxx_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epsxx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSYY */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epsyy_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epsyy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSZZ */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epszz_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epszz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSXY */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epsxy_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epsxy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSYZ */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epsyz_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epsyz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* EPSZX */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = epszx_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &epszx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for strain on global block 3 */
            {
                int num_handles;
                SAF_Field *dom_strain_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "strain_on_blk3_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &strain_on_blk3_ftmpl);
                saf_declare_field(SAF_ALL, db, &strain_on_blk3_ftmpl, "strain_on_blk3", &block_3, NULL, &domain_cat, 
                                  SAF_ZONAL(&elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &strain_on_blk3);

                /* gather field handles of strain for all block3 domains and write out to (indirect) strain field on block 3 */
                dom_strain_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&strain, &num_handles, NULL);

                pbuf = &(dom_strain_fields[0]);
                saf_write_field(SAF_ALL, &strain_on_blk3, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = strain_on_blk3;
            field_tmpl_list[cnt] = strain_on_blk3_ftmpl;
            cnt++;

            /* pressure scalar field associated with elements of BLOCK_1 and BLOCK_2 */

            /* pressure on BLOCK_1 */                

            /* first, declare field template */                    
            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_1_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, pressure_q, 1, NULL,
                                   &dom_block_elem_ftmpl);

            /* declare the field */
            saf_declare_field(SAF_EACH, db, &dom_block_elem_ftmpl, "PRESSURE", &domain_block_1, NULL, SAF_SELF(db), 
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &pressure);
  
            /* determine which elements in the domain are in element block 1 so we can extract values from the global pressure
             * field */
            calc_in_or_out(begin_elem_index_in_blk[0], end_elem_index_in_blk[0],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);
            {
                /* PRESSURE */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = press_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &pressure, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for pressure on global block 1 */
            {
                int num_handles;
                SAF_Field *dom_press_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "press_on_blk1_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &press_on_blk1_ftmpl);
                saf_declare_field(SAF_ALL, db, &press_on_blk1_ftmpl, "press_on_blk1", &block_1, NULL, &domain_cat,
                                  SAF_ZONAL(&elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &press_on_blk1);

                /* gather field handles of pressure for all block1 domains and write out to (indirect) pressure field on block 1 */
                dom_press_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&pressure, &num_handles, NULL);

                pbuf = &(dom_press_fields[0]);
                saf_write_field(SAF_ALL, &press_on_blk1, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = press_on_blk1;
            field_tmpl_list[cnt] = press_on_blk1_ftmpl;
            cnt++;

            /* pressure on BLOCK_2 */                

            /* first, declare field template */                    
            saf_declare_field_tmpl(SAF_EACH, db, "dom_block_2_elem_ftmpl",  SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, pressure_q, 1, NULL,
                                   &dom_block_2_elem_ftmpl);

            /* declare the field */
            saf_declare_field(SAF_EACH, db, &dom_block_2_elem_ftmpl, "PRESSURE", &domain_block_2, NULL, SAF_SELF(db), 
                              SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &pressure);

            /* determine which elements in the domain are in element block 2 so we can extract values from the global pressure
             * field */
            calc_in_or_out(begin_elem_index_in_blk[1], end_elem_index_in_blk[1],
                           begin_elem_index_in_domain, end_elem_index_in_domain,
                           &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);
            {
                /* PRESSURE */
                float buf[MAX_NUM_ELEMS];
                void *pbuf = &buf[0];

                if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = press_vals[begin_elem_index_in_dom_blk+i];

                saf_write_field(SAF_EACH, &pressure, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for pressure on global block 2 */
            {
                int num_handles;
                SAF_Field *dom_press_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "press_on_blk2_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &press_on_blk2_ftmpl);
                saf_declare_field(SAF_ALL, db, &press_on_blk2_ftmpl, "press_on_blk2", &block_2, NULL, &domain_cat, 
                                  SAF_ZONAL(&elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &press_on_blk2);

                /* gather field handles of pressure for all block2 domains and write out to (indirect) pressure field on block 2 */
                dom_press_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&pressure, &num_handles, NULL);

                pbuf = &(dom_press_fields[0]);
                saf_write_field(SAF_ALL, &press_on_blk2, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = press_on_blk2;
            field_tmpl_list[cnt] = press_on_blk2_ftmpl;
            cnt++;

            /* centroid vector field associated with elements of BLOCK_1, BLOCK_2, BLOCK_3, BLOCK_4, and BLOCK_5 */
            {
                dom_block_set[0] = domain_block_1;
                dom_block_set[1] = domain_block_2;
                dom_block_set[2] = domain_block_3;
                dom_block_set[3] = domain_block_4;
                dom_block_set[4] = domain_block_5;

                for (block_index=0; block_index<5; block_index++) {
                    /* first, declare field templates for the component (CX, CY, CZ) fields; then declare field templates for
                     * the composite (CENTROID) field */
                    sprintf (tmp_name, "dom_block_%d_elem_ctmpl", block_index+1);

                    saf_declare_field_tmpl(SAF_EACH, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                                           &dom_block_elem_ctmpl);

                    tmp_ftmpl[0] = dom_block_elem_ctmpl;
                    tmp_ftmpl[1] = dom_block_elem_ctmpl;
                    tmp_ftmpl[2] = dom_block_elem_ctmpl;

                    sprintf (tmp_name, "dom_block_%d_elem_tmpl", block_index+1);

                    saf_declare_field_tmpl(SAF_EACH, db, tmp_name, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3, tmp_ftmpl,
                                           &dom_block_elem_ftmpl);

                    /* determine which elements in the domain are in this element block so we can extract values from the
                     * global centroid field */
                    calc_in_or_out(begin_elem_index_in_blk[block_index], end_elem_index_in_blk[block_index],
                                   begin_elem_index_in_domain, end_elem_index_in_domain,
                                   &begin_elem_index_in_dom_blk, &num_elem_in_dom_blk);

                    /* declare the component and composite centroid fields */
                    saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "CENT_X", &(dom_block_set[block_index]), NULL,
                                      SAF_SELF(db), SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                                      &cent_x);
                    {
                        /* CENT_X */
                        float buf[MAX_NUM_ELEMS];
                        void *pbuf = &buf[0];

                        if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                        for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = xcent[begin_elem_index_in_dom_blk+i];

                        saf_write_field(SAF_EACH, &cent_x, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
                    }
    
                    saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "CENT_Y", &(dom_block_set[block_index]), NULL,
                                      SAF_SELF(db), SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                                      &cent_y);
                    {
                        /* CENT_Y */
                        float buf[MAX_NUM_ELEMS];
                        void *pbuf = &buf[0];

                        if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                        for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = ycent[begin_elem_index_in_dom_blk+i];

                        saf_write_field(SAF_EACH, &cent_y, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
                    }
    
                    saf_declare_field(SAF_EACH, db, &dom_block_elem_ctmpl, "CENT_Z", &(dom_block_set[block_index]), NULL,
                                      SAF_SELF(db), SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                                      &cent_z);
                    {
                        /* CENT_Z */
                        float buf[MAX_NUM_ELEMS];
                        void *pbuf = &buf[0];

                        if (begin_elem_index_in_dom_blk < 0) begin_elem_index_in_dom_blk = 0;
                        for (i=0; i<num_elem_in_dom_blk; i++) buf[i] = zcent[begin_elem_index_in_dom_blk+i];

                        saf_write_field(SAF_EACH, &cent_z, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
                    }
    
                    tmp_fields[0] = cent_x;
                    tmp_fields[1] = cent_y;
                    tmp_fields[2] = cent_z;

                    saf_declare_field(SAF_EACH, db, &dom_block_elem_ftmpl, "centroid", &(dom_block_set[block_index]), NULL,
                                      SAF_SELF(db), SAF_ZONAL(&elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT,
                                      SAF_IDENTITY, NULL, &centroid);

                    centroid_fld_list[block_index] = centroid;
                }
            }

            /* centroids for domain set. This will be an indirect field that references the centroid fields of each of the
             * blocks */
            saf_declare_field_tmpl(SAF_EACH, db, "dom_elem_tmpl", SAF_ALGTYPE_FIELD, SAF_ANY_BASIS, SAF_NOT_APPLICABLE_QUANTITY,
                                   SAF_NOT_APPLICABLE_INT, NULL, &domain_elem_ftmpl);

            saf_declare_field(SAF_EACH, db, &domain_elem_ftmpl, "centroid", &domain_set, NULL, &blocks, SAF_ZONAL(&elems),
                              SAF_HANDLE, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &centroid);

            pbuf = (void *)(&(centroid_fld_list[0]));
            saf_write_field(SAF_EACH, &centroid, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);

            /* create indirect field for centroids on top set */
            {
                int num_handles;
                SAF_Field *dom_cent_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "cent_on_top_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &cent_on_top_ftmpl);
                saf_declare_field(SAF_ALL, db, &cent_on_top_ftmpl, "cent_on_top", &top, NULL, &domain_cat, SAF_ZONAL(&elems),
                                  H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &cent_on_top);

                /* gather field handles of centroids for all domains and write out to (indirect) centroid field on top */
                dom_cent_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&centroid, &num_handles, NULL);

                pbuf = &(dom_cent_fields[0]);
                saf_write_field(SAF_ALL, &cent_on_top, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = cent_on_top;
            field_tmpl_list[cnt] = cent_on_top_ftmpl;
            cnt++;

            /* kinetic energy constant field associated with TOP_SET */
            saf_declare_field_tmpl(SAF_ALL, db, "top_constant", SAF_ALGTYPE_SCALAR, SAF_ANY_BASIS, energy_q, 1, NULL,
                                   &top_constant_ftmpl);

            saf_declare_field(SAF_ALL, db, &top_constant_ftmpl, "KIN_ENERGY", &top, NULL, SAF_SELF(db), SAF_CONSTANT(db),
                              SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, NULL, NULL, &ke);
            { 
                float buf[] = {10000.};
                pbuf = (void *)(&buf[0]);
                saf_write_field(SAF_ALL, &ke, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
  
            field_list[cnt] = ke;
            field_tmpl_list[cnt] = top_constant_ftmpl;
            cnt++;

            /* total energy constant field associated with TOP_SET */
            saf_declare_field(SAF_ALL, db, &top_constant_ftmpl, "TOTAL_ENERGY", &top, NULL, SAF_SELF(db), SAF_CONSTANT(db),
                              SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, NULL, NULL, &te);
            {  
                float buf[] = {20000.};
                void *pbuf = &buf[0];
                saf_write_field(SAF_ALL, &te, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            field_list[cnt] = te;
            field_tmpl_list[cnt] = top_constant_ftmpl;
            cnt++;

            /* nodal rotation field with components ROTX, ROTY, and ROTZ associated with nodes of BLOCK_1 and BLOCK_3 */
            block_set[0] = block_1;
            block_set[1] = block_3;

            dom_block_set[0] = domain_block_1;
            dom_block_set[1] = domain_block_3;

            block_num[0] = 1;
            block_num[1] = 3;

            for (i=0; i<2; i++) {
                /* first, declare field templates for the component (ROTX, ROTY, ROTZ) fields; then declare field templates for
                 * the composite (ROTATION) field */
                sprintf (tmp_name, "dom_block_%d_node_ctmpl", i+1);
                saf_declare_field_tmpl(SAF_EACH, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, angle_q, 1, NULL,
                                       &dom_block_node_ctmpl);

                tmp_ftmpl[0] = dom_block_node_ctmpl;
                tmp_ftmpl[1] = dom_block_node_ctmpl;
                tmp_ftmpl[2] = dom_block_node_ctmpl;

                saf_declare_field_tmpl(SAF_EACH, db, "dom_block_node_tmpl",  SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, angle_q, 3,
                                       tmp_ftmpl, &dom_block_node_ftmpl);

                /* declare the component and composite rotation fields */                        
                saf_declare_field(SAF_EACH, db, &dom_block_node_ctmpl, "ROT_X", &(dom_block_set[i]), NULL, SAF_SELF(db), 
                                  SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_rot);
                saf_declare_field(SAF_EACH, db, &dom_block_node_ctmpl, "ROT_Y", &(dom_block_set[i]), NULL, SAF_SELF(db), 
                                  SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_rot);
                saf_declare_field(SAF_EACH, db, &dom_block_node_ctmpl, "ROT_Z", &(dom_block_set[i]), NULL, SAF_SELF(db), 
                                  SAF_NODAL(&nodes, &elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_rot);

                tmp_fields[0] = x_rot;
                tmp_fields[1] = y_rot;
                tmp_fields[2] = z_rot;

                saf_declare_field(SAF_EACH, db, &dom_block_node_ftmpl, "ROTATION", &(dom_block_set[i]), NULL, SAF_SELF(db), 
                                  SAF_NODAL(&nodes, &elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL,
                                  &rot);
                {
                    /* X rotation */
                    float buf[] = {.011, .012, .013, .014, .015, .016};
                    void *pbuf = &buf[0];
                    saf_write_field(SAF_EACH, &x_rot, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
                }
                {
                    /* Y rotation */
                    float buf[] = {.021, .022, .023, .024, .025, .026};
                    void *pbuf = &buf[0];
                    saf_write_field(SAF_EACH, &y_rot, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
                }
                {
                    /* Z rotation */
                    float buf[] = {.031, .032, .033, .034, .035, .036};
                    void *pbuf = &buf[0];
                    saf_write_field(SAF_EACH, &z_rot, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
                }

                /* create indirect field for rotation on global blocks */
                {
                    int num_handles;
                    SAF_Field *dom_rot_fields;
                    void *pbuf;
                    sprintf (tmp_name, "nodes_in_blk_%d_ftmpl", block_num[i]);
                    saf_declare_field_tmpl(SAF_ALL, db, tmp_name, SAF_ALGTYPE_FIELD, NULL, SAF_NOT_APPLICABLE_QUANTITY,
                                           SAF_NOT_APPLICABLE_INT, NULL, &nodes_on_blk_ftmpl);
                    sprintf (tmp_name, "nodal_rot_in_blk_%d", block_num[i]);
                    saf_declare_field(SAF_ALL, db, &nodes_on_blk_ftmpl, tmp_name, &(block_set[i]), NULL, &domain_cat,
                                      SAF_NODAL(&nodes, &elems), H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL,
                                      &nodal_rot_in_blk);

                    /* gather field handles of nodal rotations for all blockX domains and write out to (indirect) rotation
                     * field on global block */
                    dom_rot_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&rot, &num_handles, NULL);

                    pbuf = &(dom_rot_fields[0]);
                    saf_write_field(SAF_ALL, &nodal_rot_in_blk, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
                }

                field_list[cnt] = nodal_rot_in_blk;
                field_tmpl_list[cnt] = nodes_on_blk_ftmpl;
                cnt++;
            }

            /* area field associated with elements of SIDE_SET_A */

            /* first, declare field template */                
            sprintf (tmp_name, "dom_ssa_elem_ftmpl_%d", rank);
            saf_declare_field_tmpl(SAF_EACH, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, area_q, 1, NULL,
                                   &dom_ssa_elem_ftmpl);

            /* declare the field */
            saf_declare_field(SAF_EACH, db, &dom_ssa_elem_ftmpl, "AREA", &domain_ssa, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
                              SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &area);
            {
                /* areas */
                float buf[] = {1., .5};
                void *pbuf = &buf[0];
                saf_write_field(SAF_EACH, &area, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* field of vector normals with components NX, NY, and NZ associated with elements of SIDE_SET_A */

            /* first, declare field templates for the component (NX, NY, NZ) fields; then declare field templates for the
             * composite (NORMALS) field */
            sprintf (tmp_name, "dom_ssa_elem_ctmpl_%d", rank);
            saf_declare_field_tmpl(SAF_EACH, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                                   &dom_ssa_elem_ctmpl);
  
            tmp_ftmpl[0] = dom_ssa_elem_ctmpl;
            tmp_ftmpl[1] = dom_ssa_elem_ctmpl;
            tmp_ftmpl[2] = dom_ssa_elem_ctmpl;

            sprintf (tmp_name, "dom_ssa_elem_tmpl_%d", rank);
            saf_declare_field_tmpl(SAF_EACH, db, tmp_name, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3, tmp_ftmpl,
                                   &dom_ssa_elem_ftmpl);

            /* declare the component and composite normal fields */                    
            saf_declare_field(SAF_EACH, db, &dom_ssa_elem_ctmpl, "NX", &domain_ssa, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
                              SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_norm);
            saf_declare_field(SAF_EACH, db, &dom_ssa_elem_ctmpl, "NY", &domain_ssa, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
                              SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_norm);
            saf_declare_field(SAF_EACH, db, &dom_ssa_elem_ctmpl, "NZ", &domain_ssa, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
                              SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_norm);

            tmp_fields[0] = x_norm;
            tmp_fields[1] = y_norm;
            tmp_fields[2] = z_norm;

            saf_declare_field(SAF_EACH, db, &dom_ssa_elem_ftmpl, "NORMAL", &domain_ssa, NULL, SAF_SELF(db), 
                              SAF_ZONAL(&elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &normal);
            {
                /* X normal */
                float buf[] = {0., 0.};
                void *pbuf = &buf[0];
                saf_write_field(SAF_EACH, &x_norm, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* Y normal */
                float buf[] = {0., 0.};
                void *pbuf = &buf[0];
                saf_write_field(SAF_EACH, &y_norm, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }
            {
                /* Z normal */
                float buf[] = {1., 1.};
                void *pbuf = &buf[0];
                saf_write_field(SAF_EACH, &z_norm, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* ID integer field associated with elements of TOP_SET */

            /* first, declare field template */                    
            saf_declare_field_tmpl(SAF_EACH, db, "domain_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN,
                                   SAF_NOT_APPLICABLE_QUANTITY, 1, NULL, &domain_elem_ftmpl);

            /* declare the field */
            saf_declare_field(SAF_EACH, db, &domain_elem_ftmpl, "ELEM_IDS", &domain_set, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(db), 
                              SAF_ZONAL(&elems), SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &elem_ids_fld);
            {
                /* element IDs */
                int buf[12];
                void *pbuf = &buf[0];

                for (i=0; i<loc_num_elems; i++) {
                    /* extract from global element IDs */
                    buf[i] = elem_ids[elem_map[i]];
                }
                saf_write_field(SAF_EACH, &elem_ids_fld, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
            }

            /* create indirect field for element IDs on top set */
            {
                int num_handles;
                SAF_Field *dom_elem_ids_fields;
                void *pbuf;

                saf_declare_field_tmpl(SAF_ALL, db, "elems_on_top_tmpl", SAF_ALGTYPE_FIELD, NULL,
                                       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &top_elem_ftmpl);
                saf_declare_field(SAF_ALL, db, &top_elem_ftmpl, "elem_ids_on_top", &top, NULL, &domain_cat, SAF_ZONAL(&elems),
                                  H5I_INVALID_HID, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &elem_ids_on_top);

                /* gather field handles of element ids for all domains and write out to (indirect) element ids field on top */
                dom_elem_ids_fields = (SAF_Field*)saf_allgather_handles((ss_pers_t*)&elem_ids_fld, &num_handles, NULL);
  
                pbuf = &(dom_elem_ids_fields[0]);
                saf_write_field(SAF_ALL, &elem_ids_on_top, SAF_WHOLE_FIELD, 1, SAF_HANDLE, &pbuf, db);
            }

            field_list[cnt] = elem_ids_on_top;
            field_tmpl_list[cnt] = top_elem_ftmpl;
            cnt++;

            if(!state) {
                /*if this is the first time through, must create the state group*/
                saf_declare_state_tmpl(SAF_ALL,db, "TIME_SUITE_TMPL", cnt, field_tmpl_list, &stmpl);
                saf_declare_state_group(SAF_ALL,db,"TIME_STATE_GROUP",&suite,&top,&stmpl,SAF_QTIME,SAF_ANY_UNIT, SAF_FLOAT,
                                        &state_grp);
            }

            saf_write_state(SAF_ALL, &state_grp, state, &top, SAF_FLOAT, &(time[state]), field_list);
        }

        saf_close_database(db);
    } SAF_CATCH {
        SAF_CATCH_ALL {
            failed = 1;
        }
    } SAF_TRY_END;
    
    saf_final();

    if (failed)
        FAILED;
    else
        PASSED;

#ifdef HAVE_PARALLEL
    MPI_Bcast(&failed, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return failed;
}


/***********************************************************************
 *
 * Create element local/global map
 *
 * This puts contiguous groups of elements in each domain.  This is
 * a reasonable map for a realistic application.
 *
 ***********************************************************************/
void create_elem_map (int loc_num_elems, int elem_num, int *elem_map)
{
  int i;

  for (i=0; i<loc_num_elems; i++) {
    elem_map[i] = elem_num++;
  }

}

/***********************************************************************
 *
 * Extract current domain's connectivity, referencing global node ids
 *
 * This extracts the "domain connectivity," that is, the connectivity
 * of the elements in the current domain.  The node ids in the domain
 * connectivity reference global node ids.
 *
 ***********************************************************************/
void extract_connect (int num_elem, int *elem_map, int *connect,
                      int *domain_connect)
{
  int i, j, k, m, offset;

  for (i=0, j=0, m=0; i<num_elem; i++) {
    if (elem_map[j] == i) {  /* extract this element */
      offset = (i * MAX_NUM_NODES_PER_ELEM);
      for (k=offset; k < offset+MAX_NUM_NODES_PER_ELEM; k++) {
        domain_connect[m++] = connect[k];
      }
      j++;
    }
  }
}

/***********************************************************************
 *
 * The local/global node map is just the current domain's connectivity,
 * sorted, with duplicate entries removed.  This isn't obvious, but
 * trust me.
 *
 ***********************************************************************/
void create_node_map (int len_connect, int *domain_connect, int *node_map,
                      int *loc_num_nodes)
{
  int cnt, i, num_neg=0, *tmp_map;

  
  tmp_map = (int *) malloc (len_connect * sizeof(int));

  memcpy (tmp_map, domain_connect, len_connect * sizeof(int));

  /* sort the domain connectivity */
  sort_int (len_connect, tmp_map);

  /* get rid of negative values (padding) */
  while (tmp_map[num_neg] < 0) num_neg++;

  node_map[0] = tmp_map[num_neg];

  /* now remove duplicate entries */

  for (cnt=0, i=num_neg; i<len_connect; i++) {
    if (node_map[cnt] != tmp_map[i]) {
      node_map[++cnt] = tmp_map[i];
    }
  }

  *loc_num_nodes = cnt+1;

}

/***********************************************************************
 *
 * Using local/global node map, convert the domain connectivity
 * (referencing global node ids) to local connectivity (referencing
 * local node ids).
 *
 * This requires inverting the local/global map, a relatively expensive
 * operation.  The procedure is:
 *
 *   for every entry in the domain connectivity
 *     search the node map until found
 *     set the value of the entry in the local connectivity to
 *       the index of the located value in the node map
 *
 ***********************************************************************/
void create_local_connect (int *node_map, int len_node_map,
                           int len_connect, int *domain_connect,
                           int *loc_connect)
{
  int i, index;

  for (i=0; i<len_connect; i++) {
    if (domain_connect[i] == -1) {  /* keep padding character (-1) */
      loc_connect[i] = -1;
    } else {
      index = bin_search2 (domain_connect[i], len_node_map, node_map);
      if (index == -1) {  /* not found */
        fprintf (stderr, "error creating local connectivity; i = %d\n", i);
        exit (EXIT_FAILURE);
      } else {
        loc_connect[i] = index;
      }
    }
  }
}

/*****************************************************************************
 *
 *       Numerical Recipies in C source code
 *       modified to have first argument an integer array
 *
 *       Sorts the array ra[0,..,(n-1)] in ascending numerical order using
 *       heapsort algorithm.
 *
 *****************************************************************************/

void sort_int(int n, int ra[])

{
  int   l, j, ir, i;
  int   rra;

  /*
   *  No need to sort if one or fewer items.
   */
  if (n <= 1) return;

  l=n >> 1;
  ir=n-1;
  for (;;) {
    if (l > 0)
      rra=ra[--l];
    else {
      rra=ra[ir];
      ra[ir]=ra[0];
      if (--ir == 0) {
        ra[0]=rra;
        return;
      }
    }
    i=l;
    j=(l << 1)+1;
    while (j <= ir) {
      if (j < ir && ra[j] < ra[j+1]) ++j;
      if (rra < ra[j]) {
        ra[i]=ra[j];
        j += (i=j)+1;
      }
      else j=ir+1;
    }
    ra[i]=rra;
  }
}

/*****************************************************************************
 *
 * Searches a monotonic list of values for the value, value.
 * It returns the index (0-based) of the first position found, which
 *   matches value.
 * The list is assumed to be monotonic, and consist of elements
 *   list[0], ..., list[n-1].
 * If no position in list matches value, it returns the value -1.
 *
 *****************************************************************************/

int bin_search2 (int value, int num, int List[])

{

 register int top, bottom = 0, middle, g_mid;

 /***** execution begins *****/

 top = num - 1;
 while (bottom <= top) {
   middle = (bottom + top) >> 1;
   g_mid = List[middle];
   if (value < g_mid)
     top = middle - 1;
   else if (value > g_mid)
     bottom = middle + 1;
   else
     return middle;     /* found */
 }

 return -1;

} /* bin_search2 */


/*****************************************************************************
 *
 * Determines which elements (if any) in a block are in a domain
 *
 * returns:
 *   - the number of elements in the domain block that are in the specified domain
 *   - the index of the first element in the domain block (-1 if none)
 *
 *****************************************************************************/

void calc_in_or_out(int first_elem_in_blk, int last_elem_in_blk, int first_elem_in_domain, int last_elem_in_domain, 
                    int *first_elem_in_dom_blk, int *num_elem_in_dom_blk)

{
  if ( (first_elem_in_blk >= first_elem_in_domain) && (last_elem_in_blk <= last_elem_in_domain) ) 
  {  /* entire block is in domain */
    *num_elem_in_dom_blk = last_elem_in_blk - first_elem_in_blk + 1;
    *first_elem_in_dom_blk = first_elem_in_blk;
  } 
  else if ( (last_elem_in_blk < first_elem_in_domain) || (first_elem_in_blk > last_elem_in_domain) ) 
  {  /* entire block is out of this domain */
    *num_elem_in_dom_blk = 0;
    *first_elem_in_dom_blk = -1;
  } 
  else 
  {  /* the block is split in multiple domains */
    if (first_elem_in_blk >= first_elem_in_domain) 
    {
      *num_elem_in_dom_blk = last_elem_in_domain - first_elem_in_blk + 1;
      *first_elem_in_dom_blk = first_elem_in_blk;
    }
    else
    {
      *num_elem_in_dom_blk = last_elem_in_blk - first_elem_in_domain + 1;
      *first_elem_in_dom_blk = first_elem_in_domain;
    }
  }

  return;
}

/*****************************************************************************
 *
 * Extracts the connectivity of a domain block given the connectivity of the domain
 *
 *****************************************************************************/

void extract_dom_block_connect(int num_elem_in_blk, int *loc_connect, int *dom_blk_connect)
{
  int i, j, k, m;

  for (i=0, k=0, m=0; i<num_elem_in_blk; i++)
    for (j=0; j<MAX_NUM_NODES_PER_ELEM; j++, k++)
      if (loc_connect[k] != -1) dom_blk_connect[m++] = loc_connect[k];

  return;
}
