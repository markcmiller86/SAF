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

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests
 * Purpose:    	Test of read of a triangle mesh with an identified embedded
 *              boundary and fields defined on them.
 *
 * Description: This test demonstrates reading of a simple 2D triangular mesh
 *              immersed in 2D Euclidean vector space.  An internal boundary of
 *              the mesh is an identified subset consisting of the edges of
 *              some of these triangular elements.  The mesh has 2 fields
 *              defined on it--the global coordinate field and a rank 2
 *              symmetric tensor.  A scalar field is defined on only the
 *              embedded boundary.  The global coordinate and scalar fields
 *              have their dofs defined on nodes while the tensor field's dofs
 *              are defined on the elements.  The program accepts one optional
 *              argument, do_interactive.  If the program is invoked without
 *              this argument, then it will attempt to read a mesh with 1 edge
 *              in x and 2 in y.  If it is invoked with do_interactive, the
 *              user will be prompted for the number of x and y edges.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of
 *              SAF_ALL mode in all calls.
 *
 * Programmer:	William J. Arrighi, LLNL, Thursday Feb 1, 2001
 *
 *-----------------------------------------------------------------------------
 */
static SAF_Db *db=NULL;         /* Handle to the SAF database. */
static SAF_Cat *nodes = NULL;   /* Handles to the node, elements and edges */
static SAF_Cat *elems = NULL;   /* categories. */
static SAF_Cat *edges = NULL;
static SAF_Set *mesh = NULL;    /* Handles to the mesh and boundary sets. */
static SAF_Set *boundary = NULL;
static int edge_ct_x, edge_ct_y;/* The number of edges in the x and y directions. */
static int node_ct_x, node_ct_y;/* The number of nodes in the x and y directions. */
static int node_ct;             /* Total number of nodes in the mesh. */
static int ele_ct;              /* Total number of elements in the mesh. */
static int failCount = 0;

#define NODE_ID(i, j) ((i)*node_ct_y + (j))

/**/
static void
read_user_input(void)
{
    /* Query user for dimensions of mesh if do_interactive is selected. */

    /* Preconditions: */

    /* Body: */

    edge_ct_x = -1;

    while (edge_ct_x < 1) {
        printf("Enter the number of elements in the x direction: ");
        scanf("%i", &edge_ct_x);
        if (edge_ct_x < 1)
            printf("\n***There must be at least 1 element in the x direction.***\n\n");
    }

    edge_ct_y = -1;

    while (edge_ct_y < 1) {
        printf("Enter the number of elements in the y direction: ");
        scanf("%i", &edge_ct_y);
        if (edge_ct_y < 1)
            printf("\n***There must be at least 1 element in the y direction.***\n\n");
    }

    /* Postconditions: */

    /* Exit */

    return;
}

/**/
static int *
make_mesh_connectivity(void)
{ 
    /* Creates a rectangular array of triangles
     * edge_ct_x = number of edges in x-dir
     * edge_ct_y = number of edgs in y-dir
     * 
    * Number of triangles in x-dir = 2*edge_ct_x and similarly for y-dir. */

    int *node_ids,                        /* Node to element connectivity. */
        conn_idx = 0,                     /* Index into connectivity array. */
        i, j, shared_node1, shared_node2;

    /* Preconditions: */

    /* Body: */

    /* allocate node_ids as single index array */
    node_ids = (int*)malloc(ele_ct*3*sizeof(int));

    for(i = 0; i < edge_ct_x; i++) {
        for(j = 0; j < edge_ct_y; j++) {
            /* Each (i,j) pair is the lower-left-hand node in a quad containing two triangles. */

            /* Connectivity for lower triangle in quad, follow right-hand rule (ccw) starting in llh corner of quad. */ 

            shared_node1 = NODE_ID(i+1, j);
            shared_node2 = NODE_ID(i, j+1);
            node_ids[conn_idx++] = NODE_ID(i,j);
            node_ids[conn_idx++] = shared_node1;
            node_ids[conn_idx++] = shared_node2;

            /* Connectivity for upper triangle in quad, follow right-hand rule (ccw) starting in urh corner of quad. */

            node_ids[conn_idx++] = NODE_ID(i+1, j+1);
            node_ids[conn_idx++] = shared_node2;
            node_ids[conn_idx++] = shared_node1;
        }
    } 

    /* Postconditions: */

    /* Exit */

    return node_ids;
}

/**/
static void
fill_boundary_maps(int *bndy_conn, int *bndy_nodes,
                   int *edges_to_elems, int *which_edge)
{
    /* Fills the boundary connectivity, boundary node to global node, edge to element and edge to element edge maps. */

    int conn_idx = 0, /* Index into connectivity array. */
        edge_idx = 0, /* Index into other edge arrays. */
                   i, node;

    /* Preconditions: */

    /* Body: */

    for (i = 0; i < edge_ct_x; i++) {
        node = NODE_ID(i, 1);
        bndy_conn[conn_idx++] = node;
        bndy_conn[conn_idx++] = NODE_ID(i+1, 1);
        bndy_nodes[edge_idx] = node;
        edges_to_elems[edge_idx] = 2*i*edge_ct_y+1;
        which_edge[edge_idx++] = 0;
    }
    bndy_nodes[edge_idx] = NODE_ID(i, 1);

    /* Postconditions: */

    /* Exit */

    return;
}

/**/
static void
read_base_space(void)
{
    /* Reads the triangle mesh from the saf_file associated with db. */

    SAF_Rel *srel1 = NULL;     /* Handles to the subset and topological */
    SAF_Rel *srel2 = NULL;     /* relations. */
    SAF_Rel trel1, *p_trel1 = &trel1, trel2, *p_trel2 = &trel2;
    SAF_Cat *coll_cats = NULL; /* Handle to the collection categories found. */
    int num,                   /* Number of entities allocated by client and/or found by library in a find call. */
        tdim,                  /* Topological dimension. */
        i;
    char *p_name = NULL;       /* Name of entity being described. */
    SAF_Role role;
    SAF_SilRole srole;
    SAF_ExtendMode extmode;
    SAF_TopMode topmode;
    SAF_CellType cell_type;
    SAF_DecompMode is_decomp;
    SAF_Set sup_set, sub_set,
        containing_set, range_s;
    SAF_Cat sup_cat, sub_cat,
        the_pieces, range_c, storage_decomp;
    SAF_BoundMode sbmode, cbmode;
    SAF_RelRep srtype;
    SAF_RelRep trtype;
    hid_t data_type, a_type, b_type;
    int *abuf = NULL, *bbuf = NULL;
    size_t a_count, b_count;
    int *mesh_conn, *bndy_conn,         /* Node to element and node to edge connectivity data. */
        *bndy_nodes,                    /* Boundary to global node map. */
        *edges_to_elems,                /* Mapping from edges to elements. */
        *which_edge;                    /* Mapping from edge to edge of an element. */

    /* Preconditions: */

    /* Body: */

    /* Set the number of nodes and elements in the mesh. */
    node_ct_x = edge_ct_x + 1;
    node_ct_y = edge_ct_y + 1;
    node_ct = node_ct_x*node_ct_y;
    ele_ct  = edge_ct_x*edge_ct_y*2;

    /* Make the mesh connectivity. */
    mesh_conn = make_mesh_connectivity();

    /* Create and fill the boundary connectivity, global to boundary node map, boundary edge to element map and boundary edge
     * to element edge maps. */
    bndy_conn = (int*)malloc(edge_ct_x*2*sizeof(int));
    bndy_nodes = (int*)malloc(node_ct_x*sizeof(int));
    edges_to_elems = (int*)malloc(edge_ct_x*sizeof(int));
    which_edge = (int*)malloc(edge_ct_x*sizeof(int));
    fill_boundary_maps(bndy_conn, bndy_nodes, edges_to_elems, which_edge);

    /*--------------------------------------------------------------------------- 
     *                           FIND CATEGORIES
     *--------------------------------------------------------------------------- */
    TESTING("finding node category");
    num = 0;
    saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "nodes", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num, &nodes);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("finding element category");
    num = 0;
    saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "elems", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num, &elems);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("finding edge category");
    num = 0;
    saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "edges", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num, &edges);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    /*--------------------------------------------------------------------------- 
     *                         DESCRIBE CATEGORIES
     *--------------------------------------------------------------------------- */
    TESTING("describing node category");
    saf_describe_category(SAF_ALL, nodes, &p_name, &role, &tdim);
    if (strcmp(p_name, "nodes") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=0) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("describing element category");
    p_name = NULL;
    saf_describe_category(SAF_ALL, elems, &p_name, &role, &tdim);
    if (strcmp(p_name, "elems") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=2) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    TESTING("describing node category");
    p_name = NULL;
    saf_describe_category(SAF_ALL, edges, &p_name, &role, &tdim);
    if (strcmp(p_name, "edges") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    /*--------------------------------------------------------------------------- 
     *                              FIND SETS
     *--------------------------------------------------------------------------- */
    TESTING("finding matching set for mesh");
    num = 0;
    saf_find_matching_sets(SAF_ALL, db, "TOP_CELL", SAF_SPACE, 2, SAF_EXTENDIBLE_FALSE, SAF_TOP_TRUE,  &num, &mesh);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("finding matching set for boundary");
    num = 0;
    saf_find_matching_sets(SAF_ALL, db, "EMBEDDED_BOUNDARY", SAF_SPACE, 1, SAF_EXTENDIBLE_FALSE, SAF_TOP_FALSE, &num, &boundary);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    /*--------------------------------------------------------------------------- 
     *                            DESCRIBE SETS
     *--------------------------------------------------------------------------- */
    TESTING("describing mesh set");
    p_name = NULL;
    num = 0;
    saf_describe_set(SAF_ALL, mesh, &p_name, &tdim, &srole, &extmode, &topmode, &num, &coll_cats);
    if (strcmp(p_name, "TOP_CELL") || tdim != 2 || srole != SAF_SPACE || extmode != SAF_EXTENDIBLE_FALSE ||
        topmode != SAF_TOP_TRUE || num != 2 ||
        !(SAF_EQUIV(coll_cats+0, nodes) || SAF_EQUIV(coll_cats+1, nodes)) ||
        !(SAF_EQUIV(coll_cats+0, elems) || SAF_EQUIV(coll_cats+1, elems))) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    free(coll_cats);

    TESTING("describing boundary set");
    p_name = NULL;
    num = 0;
    coll_cats = NULL;
    saf_describe_set(SAF_ALL, boundary, &p_name, &tdim, &srole, &extmode, &topmode, &num, &coll_cats);
    if (strcmp(p_name, "EMBEDDED_BOUNDARY") || tdim != 1 || srole != SAF_SPACE || extmode != SAF_EXTENDIBLE_FALSE ||
        topmode != SAF_TOP_FALSE || num != 2 ||
        !(SAF_EQUIV(coll_cats+0, nodes) || SAF_EQUIV(coll_cats+1, nodes)) ||
        !(SAF_EQUIV(coll_cats+0, edges) || SAF_EQUIV(coll_cats+1, edges))) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    free(coll_cats);

    /*--------------------------------------------------------------------------- 
     *                          FIND COLLECTIONS
     *--------------------------------------------------------------------------- */
    TESTING("finding collection of nodes in mesh");
    num = 0;
    coll_cats = NULL;
    saf_find_collections(SAF_ALL, mesh, SAF_TOPOLOGY, SAF_CELLTYPE_POINT, 0, SAF_DECOMP_FALSE, &num, &coll_cats);
    if (num != 1 || !SAF_EQUIV(coll_cats+0, nodes)) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    free(coll_cats);

    TESTING("finding collections of elements in mesh");
    num = 0;
    coll_cats = NULL;
    saf_find_collections(SAF_ALL, mesh, SAF_TOPOLOGY, SAF_CELLTYPE_TRI, 2, SAF_DECOMP_TRUE, &num, &coll_cats);
    if (num != 1 || !SAF_EQUIV(coll_cats+0, elems)) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    free(coll_cats);

    TESTING("finding collection of nodes in boundary");
    num = 0;
    coll_cats = NULL;
    saf_find_collections(SAF_ALL, boundary, SAF_TOPOLOGY, SAF_CELLTYPE_POINT, 0, SAF_DECOMP_FALSE, &num, &coll_cats);
    if (num != 1 || !SAF_EQUIV(coll_cats+0, nodes)) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    free(coll_cats);

    TESTING("finding collections of edges in boundary");
    num = 0;
    coll_cats = NULL;
    saf_find_collections(SAF_ALL, boundary, SAF_TOPOLOGY, SAF_CELLTYPE_LINE, 1, SAF_DECOMP_TRUE, &num, &coll_cats);
    if (num != 1 || !SAF_EQUIV(coll_cats+0, edges)) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    free(coll_cats);

    /*--------------------------------------------------------------------------- 
     *                        DESCRIBE COLLECTIONS
     *--------------------------------------------------------------------------- */
    TESTING("describing collection of nodes in mesh");
    saf_describe_collection(SAF_ALL, mesh, nodes, &cell_type, &num, NULL, &is_decomp, NULL);
    if (cell_type != SAF_CELLTYPE_POINT || num != node_ct ||
        is_decomp != SAF_DECOMP_FALSE) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("describing collection of elements in mesh");
    saf_describe_collection(SAF_ALL, mesh, elems, &cell_type, &num, NULL, &is_decomp, NULL);
    if (cell_type != SAF_CELLTYPE_TRI || num != ele_ct || is_decomp != SAF_DECOMP_TRUE) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("describing collection of nodes in boundary");
    saf_describe_collection(SAF_ALL, boundary, nodes, &cell_type, &num, NULL, &is_decomp, NULL);
    if (cell_type != SAF_CELLTYPE_POINT || num != node_ct_x ||
        is_decomp != SAF_DECOMP_FALSE) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    TESTING("describing collection of edges in boundary");
    saf_describe_collection(SAF_ALL, boundary, edges, &cell_type, &num, NULL, &is_decomp, NULL);
    if (cell_type != SAF_CELLTYPE_LINE || num != edge_ct_x ||
        is_decomp != SAF_DECOMP_TRUE) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    /*--------------------------------------------------------------------------- 
     *                         FIND SUBSET RELATION
     *--------------------------------------------------------------------------- */
    TESTING("finding subset relation of nodes of the boundary in nodes of the whole");
    num = 0;
    saf_find_subset_relations(SAF_ALL, db, mesh, boundary, SAF_COMMON(nodes), &num, &srel1);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("finding subset relation of edges of the boundary in elements of the whole");
    num = 0;
    saf_find_subset_relations(SAF_ALL, db, mesh, boundary, SAF_EMBEDBND(elems, edges), &num, &srel2);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    /*--------------------------------------------------------------------------- 
     *                       DESCRIBE SUBSET RELATION
     *--------------------------------------------------------------------------- */
    TESTING("describing subset relation of nodes of the boundary in nodes of the whole");
    saf_describe_subset_relation(SAF_ALL, srel1, &sup_set, &sub_set, &sup_cat, &sub_cat, &sbmode, &cbmode, &srtype, &data_type);
    if (!SAF_EQUIV(&sup_set, mesh) || !SAF_EQUIV(&sub_set, boundary) ||
        !SAF_EQUIV(&sup_cat, nodes) || !SAF_EQUIV(&sub_cat, nodes) ||
        sbmode != SAF_BOUNDARY_FALSE || cbmode != SAF_BOUNDARY_FALSE ||
        !SAF_EQUIV(&srtype, SAF_TUPLES) || !H5Tequal(data_type, H5T_NATIVE_INT)) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("describing subset relation of edges of the boundary in elements of the whole");
    /* In the case of a boundary subset relation, two buffers of data are needed to describe the relation.  However, only one
     * data_type is returned here.  It is not clear from examining the code which buffer's data_type is being described here.
     * There is no requirement that the types of these buffers be the same and both buffers are written to the dataset.
     * Therefore, it is essential that the types be identified properly prior to reading the the data from the database.  This
     * describe is insufficient.  It and its companion for the topological relations must be changed so that they describe both
     * buffers' types.  Note that saf_get_... does provide the proper interface but it, too, does the wrong thing and assumes
     * that both buffers must be of the same type and therefore may not give the proper result. */
    saf_describe_subset_relation(SAF_ALL, srel2, &sup_set, &sub_set, &sup_cat, &sub_cat, &sbmode, &cbmode, &srtype, &data_type);
    if (!SAF_EQUIV(&sup_set, mesh) || !SAF_EQUIV(&sub_set, boundary) ||
        !SAF_EQUIV(&sup_cat, elems) || !SAF_EQUIV(&sub_cat, edges) ||
        sbmode != SAF_BOUNDARY_FALSE || cbmode != SAF_BOUNDARY_TRUE ||
        !SAF_EQUIV(&srtype, SAF_TUPLES) || !H5Tequal(data_type, H5T_NATIVE_INT)) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    /*--------------------------------------------------------------------------- 
     *                       READING SUBSET RELATIONS
     *--------------------------------------------------------------------------- */
    TESTING("reading subset relation of nodes of the boundary in nodes of the whole");
    saf_get_count_and_type_for_subset_relation(SAF_ALL, srel1, NULL, &a_count, &a_type, &b_count, &b_type);
    if (!H5Tequal(a_type, H5T_NATIVE_INT) || b_type>0 || a_count != (size_t)node_ct_x || b_count != 0) {
        FAILED;
        failCount++;
    } else {
        /* The types and counts match so try to read the actual buffers of subset
         * relation data. */
        abuf = (int *)malloc(a_count*sizeof(int));
        saf_read_subset_relation(SAF_ALL, srel1, NULL, (void**)&abuf, NULL);
        /* Verify that the contents are as expected. */
        for (i = 0; i < (int)a_count; i++)
            if (bndy_nodes[i] != abuf[i])
                break;
        if (i < (int)a_count) {
            FAILED;
            failCount++;
        } else {
            PASSED;
        }
        free(abuf);
        abuf = NULL;
    }

    TESTING("reading subset relation of edges of the boundary in elements of the whole");
    /* In the case of boundary subset relations, the results from this may be misleading.  There is no requirement that the two
     * buffers have the same type.  However, the type of one of the buffers (its not clear which one) is returned as the type
     * of both by this function.  See comment above for saf_describe_subset_relation.  Use with caution. */
    saf_get_count_and_type_for_subset_relation(SAF_ALL, srel2, NULL, &a_count, &a_type, &b_count, &b_type);
    if (!H5Tequal(a_type, H5T_NATIVE_INT) || !H5Tequal(b_type, H5T_NATIVE_INT) || a_count != (size_t)edge_ct_x
        || b_count != (size_t)edge_ct_x) {
        FAILED;
        failCount++;
    } else {
        /* The types and counts match so try to read the actual buffers of subset
         * relation data. */
        saf_read_subset_relation(SAF_ALL, srel2, NULL, (void**)&abuf, (void**)&bbuf);
        if (abuf == NULL || bbuf == NULL) {
            FAILED;
            failCount++;
        } else {
            /* We got the buffers--verify that the contents are as expected. */
            for (i = 0; i < (int)a_count; i++)
                if (edges_to_elems[i] != abuf[i])
                    break;
            if (i < (int)a_count) {
                FAILED;
                failCount++;
            } else {
                /* The abuf is good so now check the bbuf. */
                for (i = 0; i < (int)b_count; i++)
                    if (which_edge[i] != bbuf[i])
                        break;
                if (i < (int)b_count) {
                    FAILED;
                    failCount++;
                } else {
                    PASSED;
                }
            }
        }
        if (abuf != NULL) {
            free(abuf);
            abuf = NULL;
        }
        if (bbuf != NULL) {
            free(bbuf);
            bbuf = NULL;
        }
    }
    if (srel1 == NULL)
        free(srel1);
    if (srel2 == NULL)
        free(srel2);

    /*--------------------------------------------------------------------------- 
     *                       FIND TOPOLOGY RELATIONS
     *--------------------------------------------------------------------------- */
    TESTING("finding topology relation of nodes in mesh in elements in mesh");
    num = 1;
    /* It would be good if this routine had filters that matched the declare
     * parameters.  As written this isn't good for much and will require more
     * work by a client to find the topo relation that they are interested in. */
    saf_find_topo_relations(SAF_ALL, db, mesh, NULL, &num, &p_trel1);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("finding topology relation of nodes in mesh in edges in boundary");
    saf_find_topo_relations(SAF_ALL, db, boundary, NULL, &num, &p_trel2);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    /*--------------------------------------------------------------------------- 
     *                     DESCRIBE TOPOLOGY RELATIONS
     *--------------------------------------------------------------------------- */
    TESTING("describing topology relation of nodes in mesh in elements in mesh");
    saf_describe_topo_relation(SAF_ALL, &trel1, &containing_set, &the_pieces, &range_s, &range_c, &storage_decomp, &trtype,
                               &data_type);
    if (!SAF_EQUIV(&containing_set, mesh) || !SAF_EQUIV(&the_pieces, elems) || !SAF_EQUIV(&range_s, mesh) ||
        !SAF_EQUIV(&range_c, nodes) || !_saf_is_self_decomp(&storage_decomp) || !SAF_EQUIV(&trtype, SAF_UNSTRUCTURED) ||
        !H5Tequal(data_type, H5T_NATIVE_INT)) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    TESTING("describing topology relation of nodes in mesh in edges in boundary");
    saf_describe_topo_relation(SAF_ALL, &trel2, &containing_set, &the_pieces, &range_s, &range_c, &storage_decomp, &trtype,
                               &data_type);
    if (!SAF_EQUIV(&containing_set, boundary) || !SAF_EQUIV(&the_pieces, edges) || !SAF_EQUIV(&range_s, mesh) ||
        !SAF_EQUIV(&range_c, nodes) || !_saf_is_self_decomp(&storage_decomp) || !SAF_EQUIV(&trtype, SAF_UNSTRUCTURED) ||
        !H5Tequal(data_type, H5T_NATIVE_INT)) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    /*--------------------------------------------------------------------------- 
     *                      READING TOPOLOGY RELATION
     *--------------------------------------------------------------------------- */
    TESTING("reading topology relation of nodes in mesh in elements in mesh");
    saf_get_count_and_type_for_topo_relation(SAF_ALL, &trel1, NULL, NULL, &a_count, &a_type, &b_count, &b_type);
    if (!H5Tequal(a_type, H5T_NATIVE_INT) || !H5Tequal(b_type, H5T_NATIVE_INT) ||
        a_count != 1 || b_count != (size_t)ele_ct*3) {
        FAILED;
        failCount++;
    } else {
        /* The types and counts match so try to read the actual buffers of topo
         * relation data. */
        saf_read_topo_relation(SAF_ALL, &trel1, NULL, (void**)&abuf, (void**)&bbuf);
        if (abuf == NULL || bbuf == NULL) {
            FAILED;
            failCount++;
        } else {
            /* We got the buffers--verify that the contents are as expected. */
            if (*abuf != 3) {
                FAILED;
                failCount++;
            } else {
                /* The abuf is good so now check the bbuf. */
                for (i = 0; i < (int)b_count; i++)
                    if (mesh_conn[i] != bbuf[i])
                        break;
                if (i < (int)b_count) {
                    FAILED;
                    failCount++;
                } else {
                    PASSED;
                }
            }
        }
        if (abuf != NULL) {
            free(abuf);
            abuf = NULL;
        }
        if (bbuf != NULL) {
            free(bbuf);
            bbuf = NULL;
        }
    }

    TESTING("reading topology relation of nodes in mesh in edges in boundary");
    saf_get_count_and_type_for_topo_relation(SAF_ALL, &trel2, NULL, NULL, &a_count, &a_type, &b_count, &b_type);
    if (!H5Tequal(a_type, H5T_NATIVE_INT) || !H5Tequal(b_type, H5T_NATIVE_INT) ||
        a_count != 1 || b_count != (size_t)edge_ct_x*2) {
        FAILED;
        failCount++;
    } else {
        /* The types and counts match so try to read the actual buffers of topo
         * relation data. */
        saf_read_topo_relation(SAF_ALL, &trel2, NULL, (void**)&abuf, (void**)&bbuf);
        if (abuf == NULL || bbuf == NULL) {
            FAILED;
            failCount++;
        } else {
            /* We got the buffers--verify that the contents are as expected. */
            if (*abuf != 2) {
                FAILED;
                failCount++;
            } else {
                /* The abuf is good so now check the bbuf. */
                for (i = 0; i < (int)b_count; i++)
                    if (bndy_conn[i] != bbuf[i])
                        break;
                if (i < (int)b_count) {
                    FAILED;
                    failCount++;
                } else {
                    PASSED;
                }
            }
        }
        if (abuf != NULL) {
            free(abuf);
            abuf = NULL;
        }
        if (bbuf != NULL) {
            free(bbuf);
            bbuf = NULL;
        }
    }
    free(mesh_conn);
    free(bndy_conn);
    free(bndy_nodes);
    free(edges_to_elems);
    free(which_edge);

    return;
}

/**/
static double *
make_coord_field_dofs(void)
{
    /* Creates the coordinate field dofs for a rectangular triangle mesh.
     * 
     * Number of nodes in x-dir = edge_ct_x+1 and
    * similarly for y-dir. */

    int i, j, node_id;
    double delx = 1.0/edge_ct_x;         /* X increment between nodes in x. */
    double dely = 1.0/edge_ct_y;         /* Y increment between nodes in y. */
    double *dofs;                        /* The array of dofs. */
    double x, y;                         /* x and y coordinates. */

    /* Preconditions: */

    /* Body: */

    /* allocate dofs as single index array,
    * but treat as dofs[nx+1][ny+1][2]; */
    dofs = (double*)malloc(node_ct*2*sizeof(double));

    for(i = 0; i < node_ct_x; i++) {
        x = i*delx;
        for(j = 0; j < node_ct_y; j++) {
            y = j*dely;
            node_id = NODE_ID(i,j);
            dofs[2*node_id] = x;
            dofs[2*node_id+1] = y;
        }
    }

    /* Postconditions: */

    /* Exit */

    return dofs;
}

/**/
static void
read_coord_field(void)
{
    /* Read the coordinate field for the mesh. */

    SAF_FieldTmpl *coords_ftmpl = NULL; /* Handle to the coordinate field field template. */
    SAF_FieldTmpl *coords_ctmpl = NULL; /* Handle to the coordinate field's components' field templates. */
    SAF_Field *coords = NULL;           /* Handle to the coordinate field. */
    double *lcoord_dof_tuple;           /* The coordinate field dofs. */
    double *dofs_read = NULL;           /* Dofs read from the file. */
    int num, i;
    char *p_name = NULL;
    SAF_Algebraic atype;
    SAF_Basis basis;
    hid_t dof_type;
    size_t count;

    /* Create the coordinate field dofs. */
    lcoord_dof_tuple = make_coord_field_dofs();

    /*--------------------------------------------------------------------------- 
     *                        FIND FIELD TEMPLATES 
     *--------------------------------------------------------------------------- */
    TESTING("finding global coordinate component field template");
    num = 0;
    saf_find_field_tmpls(SAF_ALL, db, "e2_on_triangle_mesh_ctmpl",
                         SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_ANY_QUANTITY, &num,
                         &coords_ctmpl);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("finding global coordinate composite field template");
    num = 0;
    saf_find_field_tmpls(SAF_ALL, db, "e2_on_triangle_mesh_tmpl", SAF_ALGTYPE_VECTOR,
                         SAF_CARTESIAN, SAF_ANY_QUANTITY, &num, &coords_ftmpl);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    /*--------------------------------------------------------------------------- 
     *                      DESCRIBE FIELD TEMPLATES
     *--------------------------------------------------------------------------- */
    TESTING("describing global coordinate component field template");
    num = 0;
    saf_describe_field_tmpl(SAF_ALL, coords_ctmpl, &p_name, &atype, &basis, NULL, &num, NULL);
    if (strcmp(p_name, "e2_on_triangle_mesh_ctmpl") || !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) ||
        !SAF_EQUIV(&basis, SAF_UNITY) || num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    TESTING("describing global coordinate composite field template");
    p_name = NULL;
    num = 0;
    saf_describe_field_tmpl(SAF_ALL, coords_ftmpl, &p_name, &atype, &basis, NULL, &num, NULL);
    if (strcmp(p_name, "e2_on_triangle_mesh_tmpl") || !SAF_EQUIV(&atype, SAF_ALGTYPE_VECTOR) ||
        !SAF_EQUIV(&basis, SAF_CARTESIAN) || num != 2) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    if (coords_ctmpl != NULL)
        free(coords_ctmpl);
    if (coords_ftmpl != NULL)
        free(coords_ftmpl);

    /*--------------------------------------------------------------------------- 
     *                             FIND FIELD
     *--------------------------------------------------------------------------- */
    TESTING("finding global coordinate field");
    num = 0;
    saf_find_fields(SAF_ALL, db, mesh, "coord field", SAF_ANY_QUANTITY, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_ANY_UNIT,
                    SAF_NODAL(nodes, elems), &num, &coords);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    /*--------------------------------------------------------------------------- 
     *                             READ FIELD
     *--------------------------------------------------------------------------- */
    TESTING("reading coordinate field");
    saf_get_count_and_type_for_field(SAF_ALL, coords, NULL, &count, &dof_type);
    if (count != (size_t)node_ct*2 || !H5Tequal(dof_type, H5T_NATIVE_DOUBLE)) {
        FAILED;
        failCount++;
    } else {
        saf_read_field(SAF_ALL, coords, NULL, SAF_WHOLE_FIELD, (void**)&dofs_read);
        for (i = 0; i < (int)count; i++)
            if (lcoord_dof_tuple[i] != dofs_read[i])
                break;
        if (i < (int)count) {
            FAILED;
            failCount++;
        } else {
            PASSED;
        }
    }
    if (coords != NULL)
        free(coords);

    /* Free the dofs now that we are done with them. */
    free(lcoord_dof_tuple);
    free(dofs_read);

    return;
}

/**/
static double *
make_scalar_field_dofs(void)
{
    /* Creates the scalar field dofs on the embedded boundary of a rectangular triangle mesh.
     * edge_ct_x = number of edges in x-dir
     * edge_ct_y = number of edgs in y-dir
     * 
    * Number of nodes in x-dir = edge_ct_x+1 and similarly for y-dir. */

    int i;
    double delx = 1.0/edge_ct_x,       /* X increment between nodes in x. */
                  *dofs,                      /* The array of dofs. */
                  x;                          /* Used to compute dofs. */

    /* Preconditions: */

    /* Body: */

    /* allocate dofs as single index array, but treat as dofs[nx+1][ny+1][2]; */
    dofs = (double*)malloc(node_ct_x*sizeof(double));

    for(i = 0; i < node_ct_x; i++) {
        x = i*delx;
        dofs[i] = x*x+1;
    }

    /* Postconditions: */

    /* Exit */

    return dofs;
}

/**/
static void
read_scalar_field(void)
{
    /* Read the scalar field defined on the mesh. */

    SAF_FieldTmpl *scalar_ftmpl = NULL; /* Handle to the scalar field field
                                        * template. */
    SAF_Field *scalar = NULL;           /* Handle to the scalar field. */
    double *lscalar_dof_tuple,          /* The scalar field dofs. */
        *dofs_read = NULL;           /* Dofs read from the file. */
    int num, i;
    char *p_name = NULL;
    SAF_Algebraic atype;
    SAF_Basis basis;
    hid_t dof_type;
    size_t count;

    /* Create the scalar field dofs. */
    lscalar_dof_tuple = make_scalar_field_dofs();

    /*--------------------------------------------------------------------------- 
     *                        FIND FIELD TEMPLATES 
     *--------------------------------------------------------------------------- */
    TESTING("finding scalar field template");
    num = 0;
    saf_find_field_tmpls(SAF_ALL, db,
                         "at0_on_triangle_mesh_embed_boundary_tmpl", SAF_ALGTYPE_SCALAR,
                         SAF_UNITY, SAF_ANY_QUANTITY, &num, &scalar_ftmpl);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    /*--------------------------------------------------------------------------- 
     *                      DESCRIBE FIELD TEMPLATES
     *--------------------------------------------------------------------------- */
    TESTING("describing scalar field template");
    num = 0;
    saf_describe_field_tmpl(SAF_ALL, scalar_ftmpl, &p_name, &atype, &basis, NULL, &num, NULL);
    if (strcmp(p_name, "at0_on_triangle_mesh_embed_boundary_tmpl") || !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) ||
        !SAF_EQUIV(&basis, SAF_UNITY) || num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    if (scalar_ftmpl != NULL)
        free(scalar_ftmpl);

    /*--------------------------------------------------------------------------- 
     *                             FIND FIELD
     *--------------------------------------------------------------------------- */
    TESTING("finding scalar field");
    num = 0;
    saf_find_fields(SAF_ALL, db, boundary, "scalar field", SAF_ANY_QUANTITY, SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_ANY_UNIT,
                    SAF_NODAL(nodes, edges), &num, &scalar);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    /*--------------------------------------------------------------------------- 
     *                             READ FIELD
     *--------------------------------------------------------------------------- */
    TESTING("reading scalar field");
    saf_get_count_and_type_for_field(SAF_ALL, scalar, NULL, &count, &dof_type);
    if (count != (size_t)node_ct_x || !H5Tequal(dof_type, H5T_NATIVE_DOUBLE)) {
        FAILED;
        failCount++;
    } else {
        saf_read_field(SAF_ALL, scalar, NULL, SAF_WHOLE_FIELD, (void**)&dofs_read);
        for (i = 0; i < (int)count; i++)
            if (lscalar_dof_tuple[i] != dofs_read[i])
                break;
        if (i < (int)count) {
            FAILED;
            failCount++;
        } else {
            PASSED;
        }
    }
    if (scalar != NULL)
        free(scalar);

    /* Free the dofs now that we are done with them. */
    free(lscalar_dof_tuple);
    free(dofs_read);

    return;
}

/**/
static double *
make_stress_field_dofs(void)
{
    /* Creates a "stress" field on a rectangular array of triangles.
     * Used to test instantiation of st2 field; values of field are meaningless.
     * edge_ct_x = number of edges in x-dir
     * edge_ct_y = number of edgs in y-dir
     * 
     * Number of nodes in x-dir = edge_ct_x+1 and
    * similarly for y-dir. */

    int tri_ct = 0,                        /* Triangle counter. */
                 i, j;
    double *dofs;                          /* The array of dofs. */

    /* Preconditions: */

    /* Body: */

    /* allocate dofs as single index array,
    * but treat as dofs[edge_ct_x][edge_ct_y][3]; */
    dofs = (double*)malloc(ele_ct*3*sizeof(double));

    for(i=0; i<edge_ct_x; i++) {
        for(j=0; j<edge_ct_y; j++) {
            /* Each (i,j) pair is the lower-left-hand node in a quad containing two triangles. */

            /* Dofs for lower triangle in quad. */

            dofs[3*tri_ct]   = tri_ct+0.0; /* xx component */
            dofs[3*tri_ct+1] = tri_ct+0.1; /* xy component */
            dofs[3*tri_ct+2] = tri_ct+0.2; /* yy component */

            tri_ct++;

            /* Dofs for upper triangle in quad. */

            dofs[3*tri_ct]   = tri_ct+0.0; /* xx component */
            dofs[3*tri_ct+1] = tri_ct+0.1; /* xy component */
            dofs[3*tri_ct+2] = tri_ct+0.2; /* yy component */

            tri_ct++;
        }
    }

    /* Postconditions: */

    /* Exit */

    return dofs;
}

/**/
static void
read_stress_field(void)
{
    /* Read the stress field defined on the mesh. */

    SAF_FieldTmpl *stress_ftmpl = NULL, /* Handle to the stress field field
                                        * template. */
                  *stress_ctmpl = NULL; /* Handle to the stress field's
                                        * components' field templates. */
    SAF_Field *stress = NULL;           /* Handle to the stress field. */
    double *lstress_dof_tuple,          /* The stress field dofs. */
        *dofs_read = NULL;           /* Dofs read from the file. */
    int num, i;
    char *p_name = NULL;
    SAF_Algebraic atype;
    SAF_Basis basis;
    hid_t dof_type;
    size_t count;

    /* Create the stress field dofs. */
    lstress_dof_tuple = make_stress_field_dofs();

    /*--------------------------------------------------------------------------- 
     *                        FIND FIELD TEMPLATES 
     *---------------------------------------------------------------------------  */
    TESTING("finding stress component field template");
    num = 0;
    saf_find_field_tmpls(SAF_ALL, db, "st2_e2_on_triangle_mesh_ctmpl",
                         SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_ANY_QUANTITY, &num,
                         &stress_ctmpl);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("finding stress composite field template");
    num = 0;
    saf_find_field_tmpls(SAF_ALL, db, "st2_e2_on_triangle_mesh_tmpl",
                         SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI, SAF_ANY_QUANTITY,
                         &num, &stress_ftmpl);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    /*--------------------------------------------------------------------------- 
     *                      DESCRIBE FIELD TEMPLATES
     *---------------------------------------------------------------------------  */
    TESTING("describing stress component field template");
    num = 0;
    saf_describe_field_tmpl(SAF_ALL, stress_ctmpl, &p_name, &atype, &basis, NULL, &num, NULL);
    if (strcmp(p_name, "st2_e2_on_triangle_mesh_ctmpl") || !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) ||
        !SAF_EQUIV(&basis, SAF_UNITY) || num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }

    TESTING("describing stress composite field template");
    p_name = NULL;
    num = 0;
    saf_describe_field_tmpl(SAF_ALL, stress_ftmpl, &p_name, &atype, &basis, NULL, &num, NULL);
    if (strcmp(p_name, "st2_e2_on_triangle_mesh_tmpl") || !SAF_EQUIV(&atype, SAF_ALGTYPE_SYMTENSOR) ||
        !SAF_EQUIV(&basis, SAF_UPPERTRI) || num != 3) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
    if (stress_ctmpl != NULL)
        free(stress_ctmpl);
    if (stress_ftmpl != NULL)
        free(stress_ftmpl);

    /*--------------------------------------------------------------------------- 
     *                             FIND FIELD
     *---------------------------------------------------------------------------  */
    TESTING("finding stress field");
    num = 0;
    saf_find_fields(SAF_ALL, db, mesh, "stress tensor", SAF_ANY_QUANTITY, SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI, SAF_ANY_UNIT,
                    SAF_ZONAL(elems), &num, &stress);
    if (num != 1) {
        FAILED;
        failCount++;
    } else {
        PASSED;
    }
   
    /*--------------------------------------------------------------------------- 
     *                             READ FIELD
     *---------------------------------------------------------------------------  */
    TESTING("reading stress field");
    saf_get_count_and_type_for_field(SAF_ALL, stress, NULL, &count, &dof_type);
    if (count != (size_t)ele_ct*3 || !H5Tequal(dof_type, H5T_NATIVE_DOUBLE)) {
        FAILED;
        failCount++;
    } else {
        saf_read_field(SAF_ALL, stress, NULL, SAF_WHOLE_FIELD, (void**)&dofs_read);
        for (i = 0; i < (int)count; i++)
            if (lstress_dof_tuple[i] != dofs_read[i])
                break;
        if (i < (int)count) {
            FAILED;
            failCount++;
        } else {
            PASSED;
        }
    }
    if (stress != NULL)
        free(stress);

    /* Free the dofs now that we are done with them. */
    free(lstress_dof_tuple);
    free(dofs_read);

    return;
}

/**/
int main(int argc, char **argv)
{
    char dbname[1024];         /* Name of the SAF database file to be created. */
    int rank=0;                /* Rank of this process for parallel. */
    SAF_DbProps *dbprops=NULL; /* Handle to the SAF databsae properties. */
    SAF_LibProps *libprops;    /* Handle to the SAF library properties. */
    hbool_t do_interactive = FALSE;

#ifdef HAVE_PARALLEL
    /* the MPI_init comes first because on some platforms MPICH's mpirun
     * doesn't pass the same argc, argv to all processors. However, the MPI
     * spec says nothing about what it does or might do to argc or argv. In
     * fact, there is no "const" in the function prototypes for either the
     * pointers or the things they're pointing too.  I would rather pass NULL
     * here and the spec says this is perfectly acceptable.  However, that too
    * has caused MPICH to core on certain platforms. */
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif


    /* since we want to see what's happening make sure stdout and stderr
       * are unbuffered */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    STU_ProcessCommandLine(1, argc, argv,
                           "do_interactive",
                           "if present, prompt for input parameters otherwise use defaults",
                           &do_interactive,
                           STU_END_OF_ARGS);

    if (rank==0 && do_interactive) {
        /* Prompt user for number of edges in x and y. */
        read_user_input();
    } else {
        edge_ct_x = 1;
        edge_ct_y = 2;
    }

#ifdef HAVE_PARALLEL
    /* Broadcast edge_ct_x and edge_ct_y from the root process to the others. */
    MPI_Bcast(&edge_ct_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&edge_ct_y, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

    /* for convenience, set working directory to the test file directory */
    chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    /* Set the string mode library property to */
    libprops = saf_createProps_lib();
    saf_setProps_StrMode(libprops, SAF_STRMODE_POOL);
    saf_setProps_StrPoolSize(libprops, 1024);

    /* Initialize the library. */
    saf_init(libprops);

    /* Get the name of the SAF database. */
    strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {
        /* NOTE: because we are in a try block here, all failures will send us to the one and only catch block at the end of
         *       this test. */

        /* Create the database properties. */
        dbprops = saf_createProps_database();

        /* Set the clobber database property so any existing file will be overwritten. */
        saf_setProps_ReadOnly(dbprops);

        /* Open the SAF database. Give it name dbname, properties p and set db to be a handle to this database. */
        TESTING("database open");
        db = saf_open_database(dbname,dbprops);
        PASSED;

        /* Read the base space with edge_ct_x edges in the x direction and edge_ct_y edges in the y direction in file
         * associated with database db.  Set mesh to mesh set, nodes to the category of nodes in the mesh and elems to the
         * category of elements in the mesh. */
        read_base_space();

        /* Read the coordinate field for the mesh. */
        read_coord_field();

        /* Read the scalar field defined on the mesh. */
        read_scalar_field();

        /* Read the stress tensor defined on the mesh. */
        read_stress_field();

        /* Close the SAF database. */
        TESTING("database close");
        saf_close_database(db);
        PASSED;
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            failCount += 1;
        }
    } SAF_TRY_END;

    /* Finalize access to the library. */
    saf_final();

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&failCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return failCount == 0 ? 0 : 1;
}
