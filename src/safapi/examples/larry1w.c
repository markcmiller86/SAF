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

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 *
 * Description: This is testing code that exercises Larry's first use case. This code declares SAF objects and writes the raw
 *              data.
 *--------------------------------------------------------------------------------------------------------------------------------
 */

/* Globally used handles. */
SAF_Db *db=NULL;                  /* Handle to the SAF database. */
SAF_Db *saf_file=NULL;          /* Handle to the SAF file. */
SAF_Cat nodes,              /* Handle to the node category. */
        elems,              /* Handle to the element category. */
        edges;              /* Handle to the edges category. */
SAF_Set top,                /* Handle to the top set--the mesh. */
        cell_1,             /* Handle to the cell_1 subset of top. */
        cell_2,             /* Handle to the cell_2 subset of top. */
        cell_2_tri,         /* Handle to the cell_2_tri subset of top. */
        cell_2_quad,        /* Handle to the cell_2_quad subset of top. */
        cell_3,             /* Handle to the cell_3 subset of top. */
        ss1,                /* Handle to the side set 1 subset of top. */
        ss2,                /* Handle to the side set 2 subset of top. */
        ns1,                /* Handle to the node set 1 subset of top. */
         time_base;          /* Handle to the time base set. */

SAF_Suite suite;

SAF_FieldTmpl coords_ftmpl, /* Handle to the field template for the
                             * global coordinates and diplacements. */
              coords_ctmpl, /* Handle to the field template for the components
                             * of the global coordinates and diplacements. */
              tmp_ftmpl[3]; /* temporary field template handle for
                             * component field templates. */
SAF_FieldTmpl stress_ftmpl, /* Handle to the stress field template. */
              stress_ctmpl; /* Handle to the stress field's components'
                             * field templates. */
SAF_FieldTmpl temp2_ftmpl;  /* Handle to the temperature field template. */
SAF_FieldTmpl press_ftmpl;  /* Handle to the pressure field template. */
SAF_Field disps,            /* Handle to the displacement field. */
          disp_compons[2];  /* Handle to the 2 components of the
                             * displacement field. */
SAF_Field stress,           /* Handle to the stress field. */
          stress_compon[3]; /* Handle to the 3 components of the
                             * stress field. */
SAF_Field temps2;           /* Handle to the temperature field. */
SAF_Field press;            /* Handle to the pressure field. */
SAF_FieldTmpl distfac_ftmpl;/* Handle to the distribution factors field
                             * template. */
SAF_Field distfac;          /* Handle to the distribution factors field. */
SAF_FieldTmpl temp1_ftmpl;  /* Handle to the temperature field template. */
SAF_Field temps1;           /* Handle to the temperature field. */
SAF_Field coords,           /* Handle to the coordinate field. */
          coord_compon[2];  /* Handle to the 2 components of the
                             * coordinate field. */




/*
 * Prototypes
 */
void make_base_space(void);
void make_global_coord_field(void);
void make_displacement_field(void);
void make_distribution_factors_on_ss2_field(void);
void make_temperature_on_ns1_field(void);
void make_temperature_on_cell_2_field(void);
void make_stress_on_cell_1_field(void);
void make_pressure_on_ss1_field(void);
void make_time_base_field(void);
void make_init_suite(void);
void make_time_suite(void);


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the mesh
 *
 * Description: Constructs the mesh for Larry's use case and writes it to a file.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_base_space(void)
{
   SAF_Rel rel, trel; /* Handles to the subset and topological relations. */
   SAF_Cat blocks,    /* Handle to the blocks category. */
           side_sets, /* Handle to the side sets category. */
           node_sets; /* Handle to the node sets category. */
   SAF_Role SAF_USERD; /* User defined role */
   /* Arrays of the inclusion mappings. */
   int cell_1_nodes[] = {1,2,3,5,6,7,9,10,11},
       cell_1_elems[] = {1,2,4,5},
       cell_2_nodes[] = {9,10,11,13,14,16,17},
       cell_2_elems[] = {7,8,9,11},
       cell_2_tri_nodes[] = {9,10,11,13,14},
       cell_2_tri_elems[] = {7,8,9},
       cell_2_quad_nodes[] = {13,14,16,17},
       cell_2_quad_elems[] = {11},
       cell_3_nodes[] = {3,4,7,8,11,12,14,15,17,18},
       cell_3_elems[] = {3,6,10,12},
       cell_1_blocks[] = {0},
       cell_2_tri_blocks[] = {1},
       cell_2_quad_blocks[] = {2},
       cell_3_blocks[] = {3},
       ss1_nodes[] = {9,10,11},
       ss2_nodes[] = {1,5,9,13,16},
       ns1_nodes[] = {4,8,12,15,18},
       cell_1_element_node_ct[] = {4},
       cell_1_connectivity[] = {1,2,6,5,2,3,7,6,5,6,10,9,6,7,11,10},
       cell_2_tri_element_node_ct[] = {3},
       cell_2_tri_connectivity[] = {9,10,13,10,14,13,10,11,14},
       cell_2_quad_element_node_ct[] = {4},
       cell_2_quad_connectivity[] = {13,14,17,16},
       cell_3_element_node_ct[] = {4},
       cell_3_connectivity[] = {3,4,8,7,  7,8,12,11,  11,12,15,14,  14,15,18,17},
       side_set_edge_node_ct[] = {2},
       ss1_connectivity[] = {1,2,2,3},
       ss2_connectivity[] = {1,2,2,3,3,4,4,5};

   /*
    ---------------------------------------------------------------------------
    *                            DECLARE ROLES
    ---------------------------------------------------------------------------
    */
   saf_declare_role(SAF_ALL, db, "testing", "larry1w", &SAF_USERD);
       
   /*
    ---------------------------------------------------------------------------
    *                            DECLARE CATEGORIES
    ---------------------------------------------------------------------------
    */
   saf_declare_category(SAF_ALL, db, "nodes",     SAF_TOPOLOGY, 0, &nodes);
   saf_declare_category(SAF_ALL, db, "elems",     SAF_TOPOLOGY, 2, &elems);
   saf_declare_category(SAF_ALL, db, "edges",     SAF_TOPOLOGY, 1, &edges);
   saf_declare_category(SAF_ALL, db, "blocks",    SAF_BLOCK,    2, &blocks);
   saf_declare_category(SAF_ALL, db, "side_sets", &SAF_USERD,    1, &side_sets);
   saf_declare_category(SAF_ALL, db, "node_sets", &SAF_USERD,    0, &node_sets);

   /*
    ---------------------------------------------------------------------------
    *                               DECLARE SETS
    ---------------------------------------------------------------------------
    */
   saf_declare_set(SAF_ALL, db, "TOP_CELL",   2, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &top);
   saf_declare_set(SAF_ALL, db, "CELL_1",     2, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &cell_1);
   saf_declare_set(SAF_ALL, db, "CELL_2",     2, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &cell_2);
   saf_declare_set(SAF_ALL, db, "CELL_3",     2, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &cell_3);
   saf_declare_set(SAF_ALL, db, "SIDE_SET_1", 1, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &ss1);
   saf_declare_set(SAF_ALL, db, "SIDE_SET_2", 1, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &ss2);
   saf_declare_set(SAF_ALL, db, "NODE_SET_1", 0, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &ns1);
   saf_declare_set(SAF_ALL, db, "TIME",       0, SAF_TIME,
                   SAF_EXTENDIBLE_TRUE,  &time_base);

   /* The following sets are needed to deal with the inhomogeneous
    * cell types on CELL_2. */
   saf_declare_set(SAF_ALL, db, "CELL_2_TRIS",  2, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &cell_2_tri);
   saf_declare_set(SAF_ALL, db, "CELL_2_QUADS", 2, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &cell_2_quad);

   /*
    ---------------------------------------------------------------------------
    *                           DECLARE COLLECTIONS
    ---------------------------------------------------------------------------
    */
   /* collections contained in the top set */
   saf_declare_collection(SAF_ALL, &top,         &nodes,     SAF_CELLTYPE_POINT, 18,
                          SAF_1DF(18), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &top,         &elems,     SAF_CELLTYPE_MIXED, 12,
                          SAF_1DF(12), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &top,         &blocks,    SAF_CELLTYPE_SET,   4,
                          SAF_1DF(4),  SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &top,         &side_sets, SAF_CELLTYPE_SET,   2,
                          SAF_1DF(2),  SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &top,         &node_sets, SAF_CELLTYPE_SET,   1,
                          SAF_1DF(1),  SAF_DECOMP_FALSE);

   /* collections contained in the cell 1 set */
   saf_declare_collection(SAF_ALL, &cell_1,      &nodes,     SAF_CELLTYPE_POINT, 9,
                          SAF_1DF(9), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &cell_1,      &elems,     SAF_CELLTYPE_QUAD,  4,
                          SAF_1DF(4), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &cell_1,      &blocks,    SAF_CELLTYPE_SET,   1,
                          SAF_1DF(1), SAF_DECOMP_TRUE);

   /* collections contained in the cell 2 set */
   saf_declare_collection(SAF_ALL, &cell_2,      &nodes,     SAF_CELLTYPE_POINT, 7,
                          SAF_1DF(7), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &cell_2,      &elems,     SAF_CELLTYPE_MIXED, 4,
                          SAF_1DF(4), SAF_DECOMP_TRUE);

   /* collections contained in the cell 3 set */
   saf_declare_collection(SAF_ALL, &cell_3,      &nodes,     SAF_CELLTYPE_POINT, 10,
                          SAF_1DF(10), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &cell_3,      &elems,     SAF_CELLTYPE_QUAD,  4,
                          SAF_1DF(4), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &cell_3,      &blocks,    SAF_CELLTYPE_SET,   1,
                          SAF_1DF(1), SAF_DECOMP_TRUE);

   /* collections contained in side set 1 */
   saf_declare_collection(SAF_ALL, &ss1,         &nodes,     SAF_CELLTYPE_POINT, 3,
                          SAF_1DF(3), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &ss1,         &edges,     SAF_CELLTYPE_LINE,  2,
                          SAF_1DF(2), SAF_DECOMP_TRUE);

   /* collections contained in side set 2 */
   saf_declare_collection(SAF_ALL, &ss2,         &nodes,     SAF_CELLTYPE_POINT, 5,
                          SAF_1DF(5), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &ss2,         &edges,     SAF_CELLTYPE_LINE,  4,
                          SAF_1DF(4), SAF_DECOMP_TRUE);

   /* collections contained in node set 1 */
   saf_declare_collection(SAF_ALL, &ns1,         &nodes,     SAF_CELLTYPE_POINT, 5,
                          SAF_1DF(5), SAF_DECOMP_TRUE);

   /* collections contained in the set of cell_2's triangles */
   saf_declare_collection(SAF_ALL, &cell_2_tri,  &nodes,     SAF_CELLTYPE_POINT, 5,
                          SAF_1DF(5), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &cell_2_tri,  &elems,     SAF_CELLTYPE_TRI,   3,
                          SAF_1DF(3), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &cell_2_tri,  &blocks,    SAF_CELLTYPE_SET,   1,
                          SAF_1DF(1), SAF_DECOMP_TRUE);

   /* collections contained in the set of cell_2's quads */
   saf_declare_collection(SAF_ALL, &cell_2_quad, &nodes,     SAF_CELLTYPE_POINT, 4,
                          SAF_1DF(4), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &cell_2_quad, &elems,     SAF_CELLTYPE_QUAD,  1,
                          SAF_1DF(1), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &cell_2_quad, &blocks,    SAF_CELLTYPE_SET,   1,
                          SAF_1DF(1), SAF_DECOMP_TRUE);

   /* collections contained in the time base set */
   saf_declare_collection(SAF_ALL, &time_base,   &nodes,     SAF_CELLTYPE_POINT, 1,
                          SAF_1DC(1), SAF_DECOMP_TRUE);

   /*
    ---------------------------------------------------------------------------
    *                    DECLARE AND WRITE SUBSET RELATIONS
    ---------------------------------------------------------------------------
    */
   /* For the first group of subset relations, the relation data will be
    * passed to the write call rather than the declare call.  To make the
    * association of writes with declares clear, the writes appear immediately
    * after the declare with which they are associated.  It is not required
    * that each write immediately follow the declare with which it is
    * associated, however. */

   /* nodes and elems of cell_1 in nodes and elems of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1,      SAF_COMMON(&nodes),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_1_nodes,       H5I_INVALID_HID,
                             NULL, saf_file);
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1,      SAF_COMMON(&elems),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_1_elems,       H5I_INVALID_HID,
                             NULL, saf_file);

   /* nodes and elems of cell_2 in nodes and elems of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2,      SAF_COMMON(&nodes),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_2_nodes,       H5I_INVALID_HID,
                             NULL, saf_file);
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2,      SAF_COMMON(&elems),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_2_elems,       H5I_INVALID_HID,
                             NULL, saf_file);

   /* nodes and elems of cell_2_tri in nodes and elems of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri,  SAF_COMMON(&nodes),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_2_tri_nodes,   H5I_INVALID_HID,
                             NULL, saf_file);
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri,  SAF_COMMON(&elems),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_2_tri_elems,   H5I_INVALID_HID,
                             NULL, saf_file);

   /* nodes and elems of cell_2_quad in nodes and elems of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&nodes),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_2_quad_nodes,  H5I_INVALID_HID,
                             NULL, saf_file);
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&elems),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_2_quad_elems,  H5I_INVALID_HID,
                             NULL, saf_file);

   /* nodes and elems of cell_3 in nodes and elems of the top*/
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3,      SAF_COMMON(&nodes),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_3_nodes,       H5I_INVALID_HID,
                             NULL, saf_file);
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3,      SAF_COMMON(&elems),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_3_elems,       H5I_INVALID_HID,
                             NULL, saf_file);

   /* blocks of cell_1 in blocks of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1,      SAF_COMMON(&blocks),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_1_blocks,      H5I_INVALID_HID,
                             NULL, saf_file);

   /* blocks of cell_2_tri in blocks of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri,  SAF_COMMON(&blocks),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_2_tri_blocks,  H5I_INVALID_HID,
                             NULL, saf_file);

   /* blocks of cell_2_quad in blocks of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&blocks),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_2_quad_blocks, H5I_INVALID_HID,
                             NULL, saf_file);

   /* blocks of cell_3 in blocks of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3,      SAF_COMMON(&blocks),
                               SAF_TUPLES, H5I_INVALID_HID,    NULL,      H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, cell_3_blocks,      H5I_INVALID_HID,
                             NULL, saf_file);

   /* For the next group of subset relations, the relation data will be
    * passed to the declare call rather than the write call. */

   /* nodes of side set 1 in nodes of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &ss1,         SAF_COMMON(&nodes),
                               SAF_TUPLES, SAF_INT, ss1_nodes, H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, saf_file);

   /* nodes of side set 2 in nodes of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &ss2,         SAF_COMMON(&nodes),
                               SAF_TUPLES, SAF_INT, ss2_nodes, H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, saf_file);

   /* nodes of node set 1 in nodes of the top */
   saf_declare_subset_relation(SAF_ALL, db, &top, &ns1,         SAF_COMMON(&nodes),
                               SAF_TUPLES, SAF_INT, ns1_nodes, H5I_INVALID_HID, NULL,
                               &rel);
   saf_write_subset_relation(SAF_ALL, &rel, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, saf_file);

   /*
    ---------------------------------------------------------------------------
    *                   DECLARE AND WRITE TOPOLOGY RELATIONS
    ---------------------------------------------------------------------------
    */

   /* The connectivity data will be passed in the write call for all
    * topological relations. */

   /* connectivity of the nodes of the top in the elements of cell_1 */
   saf_declare_topo_relation(SAF_ALL, db, &cell_1,      &elems, &top, &nodes,
                             SAF_SELF(db), &cell_1, SAF_UNSTRUCTURED, H5I_INVALID_HID,
                             NULL, H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, cell_1_element_node_ct,
                           SAF_INT, cell_1_connectivity, saf_file);

   /* connectivity of the nodes of the top in the elements of cell_2_tri */
   saf_declare_topo_relation(SAF_ALL, db, &cell_2_tri,  &elems, &top, &nodes,
                             SAF_SELF(db), &cell_2_tri, SAF_UNSTRUCTURED, H5I_INVALID_HID,
                             NULL, H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, cell_2_tri_element_node_ct,
                           SAF_INT, cell_2_tri_connectivity, saf_file);

   /* connectivity of the nodes of the top in the elements of cell_2_quad */
   saf_declare_topo_relation(SAF_ALL, db, &cell_2_quad, &elems, &top, &nodes,
                             SAF_SELF(db), &cell_2_quad, SAF_UNSTRUCTURED, H5I_INVALID_HID,
                             NULL, H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, cell_2_quad_element_node_ct,
                           SAF_INT, cell_2_quad_connectivity, saf_file);

   /* connectivity of the nodes of the top in the elements of cell_3 */
   saf_declare_topo_relation(SAF_ALL, db, &cell_3,      &elems, &top, &nodes,
                             SAF_SELF(db), &cell_3, SAF_UNSTRUCTURED, H5I_INVALID_HID,
                             NULL, H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, cell_3_element_node_ct,
                           SAF_INT, cell_3_connectivity, saf_file);

   /* connectivity of the nodes of the top in the edges of side set 1 */
   saf_declare_topo_relation(SAF_ALL, db, &ss1,         &edges, &top, &nodes,
                             SAF_SELF(db), &ss1,    SAF_UNSTRUCTURED, H5I_INVALID_HID,
                             NULL, H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, side_set_edge_node_ct,
                           SAF_INT, ss1_connectivity,    saf_file);

   /* connectivity of the nodes of the top in the edges of side set 2 */
   saf_declare_topo_relation(SAF_ALL, db, &ss2,         &edges, &top, &nodes,
                             SAF_SELF(db), &ss2,    SAF_UNSTRUCTURED, H5I_INVALID_HID,
                             NULL, H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, side_set_edge_node_ct,
                           SAF_INT, ss2_connectivity,    saf_file);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the coordinate field
 *
 * Description: Constructs the coordinate field on the mesh.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_global_coord_field(void)
{
   SAF_Unit umeter;            /* Handle to the units for the coordinates. */
   /* The coordinate field dofs. */
   float lcoord_dof_tuple[] = {0.,4., 1.,4., 2.,4., 2.5,4.,
                               0.,3., 1.,3., 2.,3., 2.5,3.,
                               0.,2., 1.,2., 2.,2., 2.5,2.,
                               0.,1.,        2.,1., 2.5,1.,
                               0.,0.,        2.,0., 2.5,0.};
   void *dofs = &lcoord_dof_tuple[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR,
                          SAF_CARTESIAN, SAF_QLENGTH, 1,
                          NULL, &coords_ctmpl);

   tmp_ftmpl[0] = coords_ctmpl;
   tmp_ftmpl[1] = coords_ctmpl;
   saf_declare_field_tmpl(SAF_ALL, db, "coordinate_tmpl", SAF_ALGTYPE_VECTOR,
                          SAF_CARTESIAN, SAF_QLENGTH, 2,
                          tmp_ftmpl, &coords_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                      (dofs specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "meter", &umeter);

   /* Declare the fields. */
   saf_declare_field(SAF_ALL, db, &coords_ctmpl, "X",      &top, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_FLOAT,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     &(coord_compon[0]));
   saf_declare_field(SAF_ALL, db, &coords_ctmpl, "Y",      &top, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_FLOAT,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     &(coord_compon[1]));
   saf_declare_field(SAF_ALL, db, &coords_ftmpl, "coords", &top, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_FLOAT,
                     coord_compon, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                     &coords);

   /* Write the coordinate field. */
   saf_write_field(SAF_ALL, &coords, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID, &dofs, saf_file);

   /* specify that is a coordinate field */
   saf_declare_coords(SAF_ALL, &coords);
   saf_declare_default_coords(SAF_ALL, &top, &coords);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the displacement field
 *
 * Description: Constructs the displacement field on the mesh. The field templates for the displacement field are the same as
 *              the templates for the global coordinate field.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_displacement_field(void)
{
   SAF_Unit umeter;            /* Handle to the units for the coordinates. */
   /* The displacement field dofs. */
   float displacement_dof_tuple[] = {.25,.25,   .25,.25,   .25,.25,   .25,.25,
                                     .25,.25,   .25,.25,   .25,.25,   .25,.25,
                                     .25,.25,   .25,.25,   .25,.25,   .25,.25,
                                     .25,.25,              .25,.25,   .25,.25,
                                     .25,.25,              .25,.25,   .25,.25};
   void *dofs = &displacement_dof_tuple[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                      (dofs specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "meter", &umeter);

   /* Declare the fields. */
   saf_declare_field(SAF_ALL, db, &coords_ctmpl, "dX",            &top, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_FLOAT,
                     NULL,         SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &(disp_compons[0]));
   saf_declare_field(SAF_ALL, db, &coords_ctmpl, "dY",            &top, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_FLOAT,
                     NULL,         SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &(disp_compons[1]));
   saf_declare_field(SAF_ALL, db, &coords_ftmpl, "displacements", &top, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_FLOAT,
                     disp_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                     &disps);

   /* Write the field. */
   saf_write_field(SAF_ALL, &disps, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID, &dofs, saf_file);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the distribution factors
 *
 * Description: Constructs the distribution factors field on side set 2.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_distribution_factors_on_ss2_field(void)
{
   /* Made up distribution factors. */
   float distfac_dof_tuple[]  = {4., 3., 2., 1., 0.};
   void *dofs = &distfac_dof_tuple[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL,  db,"distrib_factors_tmpl", SAF_ALGTYPE_SCALAR,
                          SAF_UNITY, SAF_NOT_APPLICABLE_QUANTITY, 1, NULL,
                          &distfac_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                      (dofs specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Declare the field. */
   saf_declare_field(SAF_ALL, db, &distfac_ftmpl, "distribution factors",
                     &ss2, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(db),
                     SAF_NODAL(&nodes, &edges), H5T_NATIVE_FLOAT, NULL,
                     SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &distfac);

   /* Write the field. */
   saf_write_field(SAF_ALL, &distfac, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID, &dofs, saf_file);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the temparature field
 *
 * Description: Constructs the temperature field on node set 1.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_temperature_on_ns1_field(void)
{
   SAF_Unit ukelvin;          /* Handle to the units for the temperature. */
   /* Made up temperatures. */
   float temp1_dof_tuple[] = {100., 150., 150., 100., 75.};
   void *dofs = &temp1_dof_tuple[0];
   /* More made up temperatures. */
   float new_temp1_dof_tuple[] = {375., 415., 225., 195., 150.};
   void *new_dofs = &new_temp1_dof_tuple[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL,  db,"temp_on_ns1_tmpl", SAF_ALGTYPE_SCALAR,
                          SAF_UNITY, SAF_QTEMP, 1, NULL, &temp1_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                      (dofs specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "kelvin", &ukelvin);

   /* Declare the field. */
   saf_declare_field(SAF_ALL, db, &temp1_ftmpl, "temperature", &ns1, &ukelvin,
                     SAF_SELF(db), SAF_NODAL(&nodes, &nodes), H5T_NATIVE_FLOAT,
                     NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &temps1);

   /* Write the field. */
   saf_write_field(SAF_ALL, &temps1, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID, &dofs, saf_file);

   /*
    ---------------------------------------------------------------------------
    *                             OVERWRITE FIELD
    ---------------------------------------------------------------------------
    */
   saf_write_field(SAF_ALL, &temps1, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID, &new_dofs, saf_file);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the temperature field
 *
 * Description: Constructs the temperature field on node set 1.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_temperature_on_cell_2_field(void)
{
   SAF_Unit ukelvin;          /* Handle to the units for the temperature. */
   /* Made up temperatures. */
   float temp2_dof_tuple[] = {75., 95., 120., 80., 115., 85., 110.};
   void *dofs = &temp2_dof_tuple[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL,  db,"temp_on_cell_2_tmpl", SAF_ALGTYPE_SCALAR,
                          SAF_UNITY, SAF_QTEMP, 1, NULL, &temp2_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                     (dofs specified in declare call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "kelvin",&ukelvin);

   /* Declare the field. */
   saf_declare_field(SAF_ALL, db, &temp2_ftmpl, "temperature", &cell_2, &ukelvin,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_FLOAT,
                     NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, &dofs, &temps2);

   /* Write the field. */
   saf_write_field(SAF_ALL, &temps2, SAF_WHOLE_FIELD, 0, H5I_INVALID_HID, NULL, saf_file);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the stress field
 *
 * Description: Construct the stress field on cell 1.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_stress_on_cell_1_field(void)
{
   SAF_Unit upascal;           /* Handle to the units for the stress. */
   /* Made up stresses. */
   float stress_dof_tuple[] = {0.5, 0.25, 0.5,0.5, 0.25, 0.5,
                               0.5, 0.25, 0.5,0.5, 0.25, 0.5};
   void *dofs = &stress_dof_tuple[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL,  db,"stress_on_cell_1_tmpl",
                          SAF_ALGTYPE_SCALAR, SAF_UNITY,
                          SAF_QNAME(db,"pressure"), 1, NULL,
                          &stress_ctmpl);

   tmp_ftmpl[0] = stress_ctmpl;
   tmp_ftmpl[1] = stress_ctmpl;
   tmp_ftmpl[2] = stress_ctmpl;
   saf_declare_field_tmpl(SAF_ALL,  db,"stress_on_cell_1_tmpl",
                          SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI,
                          SAF_QNAME(db,"pressure"), 3, tmp_ftmpl,
                          &stress_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                     (dofs specified in declare call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "pascal", &upascal);

   /* Declare the field. */
   saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sx",     &cell_1, &upascal, SAF_SELF(db),
                     SAF_ZONAL(&elems), H5T_NATIVE_FLOAT, NULL,
                     SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                     NULL, &(stress_compon[0]));
   saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sy",     &cell_1, &upascal, SAF_SELF(db),
                     SAF_ZONAL(&elems), H5T_NATIVE_FLOAT, NULL,
                     SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                     NULL, &(stress_compon[1]));
   saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sxy",    &cell_1, &upascal, SAF_SELF(db),
                     SAF_ZONAL(&elems), H5T_NATIVE_FLOAT, NULL,
                     SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                     NULL, &(stress_compon[2]));

   saf_declare_field(SAF_ALL, db, &stress_ftmpl, "stress", &cell_1, &upascal, SAF_SELF(db),
                     SAF_ZONAL(&elems), H5T_NATIVE_FLOAT, stress_compon,
                     SAF_INTERLEAVE_VECTOR, SAF_IDENTITY,
                     &dofs, &stress);


   /* Write the field. */
   saf_write_field(SAF_ALL, &stress, SAF_WHOLE_FIELD, 0, H5I_INVALID_HID, NULL, saf_file);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the stress field
 *
 * Description: Constructs the stress field on cell 1.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_pressure_on_ss1_field(void)
{
   SAF_Unit upsi;             /* Handle to the units for the pressure. */
   /* Made up pressure. */
   float pressure_dof_tuple[] = {45., 55.};
   void *dofs = &pressure_dof_tuple[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL,  db,"pressure_on_ss1", SAF_ALGTYPE_SCALAR,
                          SAF_UNITY, SAF_QNAME(db,"pressure"), 1, NULL,
                          &press_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                     (dofs specified in declare call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "psi", &upsi);

   /* Declare the field. */
   saf_declare_field(SAF_ALL, db, &press_ftmpl, "pressure", &ss1, &upsi, SAF_SELF(db),
                     SAF_ZONAL(&edges), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE,
                     SAF_IDENTITY, &dofs, &press);

   /* Write the field. */
   saf_write_field(SAF_ALL, &press, SAF_WHOLE_FIELD, 0, H5I_INVALID_HID, NULL, saf_file);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Construct the time field
 *
 * Description: Construct the time field on the time base.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_time_base_field(void)
{
   SAF_FieldTmpl time_ftmpl; /* Handle to the time field template. */
   SAF_Field time_fld;       /* Handle to the time field. */
   SAF_Unit usec;         
   /* Made up times. */
   float time_dof_tuple[] = {0.0,  1.4,  1.8,  2.35,
                             3.0,  3.01, 5.25, 6.1,
                             6.75, 8.0,  11.0};
   void *dofs = &time_dof_tuple[0];
   int members[3] = {0,0,1};

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL,  db,"time_on_time_base", SAF_ALGTYPE_SCALAR,
                          SAF_UNITY, SAF_QTIME, 1, NULL, &time_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                      (dofs specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "second", &usec);

   /* Declare the field. */
   saf_declare_field(SAF_ALL, db, &time_ftmpl, "times", &time_base, &usec, SAF_SELF(db),
                     SAF_NODAL(&nodes, &nodes), H5T_NATIVE_FLOAT, NULL,
                     SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &time_fld);

   /* indicate this is a coordinate field for the time base */
   saf_declare_coords(SAF_ALL, &time_fld);
   saf_declare_default_coords(SAF_ALL, &time_base, &time_fld);

   /* Write part of the field--dofs on nodes 0-4. */
   members[0] = 0;  /* start at 0 */
   members[1] = 5;  /* count of 5 */
   saf_write_field(SAF_ALL, &time_fld, 5, SAF_HSLAB, members, 1,
                   H5I_INVALID_HID, &dofs, saf_file);

   /* Write the remainder of the field--dofs on nodes 5-10. */
   members[0] = 5;  /* start at 5 */
   members[1] = 6;  /* count of 6 */
   saf_write_field(SAF_ALL, &time_fld, 6, SAF_HSLAB, members, 1,
                   H5I_INVALID_HID, &dofs, saf_file);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Create initial suite
 *
 * Description: Create a suite for initial state (time step zero).
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_init_suite(void)
{
   int index[1];
   float time[1];
   SAF_Unit usec;
   SAF_StateTmpl st_tmpl;
   SAF_FieldTmpl fld_tmpls[3];
   SAF_StateGrp state_grp;
   SAF_Field field_list[3];

   saf_find_one_unit(db, "second", &usec);

   /* create a suite for initial state (time step 0) */
   saf_declare_suite(SAF_ALL, db, "INIT_SUITE", &top, NULL, &suite);

   /* create a state template to define what types of fields will be stored at each state in this suite;
      this is defined by a list of field templates */

   fld_tmpls[0] = coords_ftmpl;
   fld_tmpls[1] = distfac_ftmpl;
   fld_tmpls[2] = temp1_ftmpl;

   saf_declare_state_tmpl (SAF_ALL, db, "INIT_SUITE_STATE_TMPL", 3, fld_tmpls, &st_tmpl);

   /* create a state group for this suite */
   saf_declare_state_group(SAF_ALL, db, "INIT_STATEGRP", &suite, &top, &st_tmpl, SAF_QTIME, &usec,
			   SAF_FLOAT, &state_grp);

   /* insert the following fields into the state for time step 0:
    *   coordinates on nodes of whole
    *   distribution factors on nodes of side_set_2
    *   temperature on nodes of node_set_1
    */

   index[0] = 0;

   time[0] = 0.0;

   field_list[0] = coords;
   field_list[1] = distfac;
   field_list[2] = temps1;

   saf_write_state (SAF_ALL, &state_grp, index[0], &top, SAF_FLOAT, time, field_list);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Create time suite
 *
 * Description: Create time suite.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_time_suite(void)
{
   int index[1];
   float time[1];

   SAF_Unit usec;
   /* SAF_Suite suite; */
   SAF_StateTmpl st_tmpl;
   SAF_FieldTmpl fld_tmpls[4];
   SAF_StateGrp state_grp;
   SAF_Field field_list[4];


   saf_find_one_unit(db, "second", &usec);

   /* create a suite for other states  */
   saf_declare_suite(SAF_ALL, db, "OTHER_SUITE", &top, NULL, &suite);

   /* create a state template to define what types of fields will be stored at each state in this suite;
      this is defined by a list of field templates */

   fld_tmpls[0] = coords_ftmpl;
   fld_tmpls[1] = stress_ftmpl;
   fld_tmpls[2] = temp2_ftmpl;
   fld_tmpls[3] = press_ftmpl;

   saf_declare_state_tmpl (SAF_ALL, db, "OTHER_SUITE_STATE_TMPL", 4, fld_tmpls, &st_tmpl);

   /* create a state field for this suite */
   saf_declare_state_group(SAF_ALL, db, "OTHER_STATEGRP", &suite, &top, &st_tmpl, SAF_QTIME, &usec, 
			   SAF_FLOAT, &state_grp);

   /* insert the following fields into the states for time steps 0, 1, and 2:
    *   displacement vector on nodes of whole
    *   stress tensor on elements of cell_1
    *   temperature on nodes of cell_2
    *   pressure on elements of side_set_1
    */

   index[0] = 0;

   time[0] = 0.0;

   field_list[0] = disps;
   field_list[1] = stress;
   field_list[2] = temps2;
   field_list[3] = press;

   saf_write_state (SAF_ALL, &state_grp, index[0], &top, SAF_FLOAT, time, field_list);

   /* for this test, just write out the same fields (IDs) to the successive states;
    * in actual simulations, new fields will be created for each time step
    */

   index[0] = 1;

   time[0] = 0.001;

   field_list[0] = disps;
   field_list[1] = stress;
   field_list[2] = temps2;
   field_list[3] = press;

   saf_write_state (SAF_ALL, &state_grp, index[0], &top, SAF_FLOAT, time, field_list);

   index[0] = 2;

   time[0] = 0.002;

   field_list[0] = disps;
   field_list[1] = stress;
   field_list[2] = temps2;
   field_list[3] = press;

   saf_write_state (SAF_ALL, &state_grp, index[0], &top, SAF_FLOAT, time, field_list);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Larry's Use Case
 * Purpose:     Main entry point
 *
 * Description: Implementation of Larry's use case.
 *
 * Return:      Exit status is the number of failures.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc,
     char **argv)
{
  char dbname[1024]; /* Name of the SAF database file to be created. */
  int rank=0;        /* Rank of this process for parallel. */
  SAF_DbProps *dbprops;/* Handle to the SAF databsae properties. */
  int failed = 0;

#ifdef HAVE_PARALLEL
  /* the MPI_init comes first because on some platforms MPICH's mpirun
   * doesn't pass the same argc, argv to all processors. However, the MPI
   * spec says nothing about what it does or might do to argc or argv. In
   * fact, there is no "const" in the function prototypes for either the
   * pointers or the things they're pointing too.  I would rather pass NULL
   * here and the spec says this is perfectly acceptable.  However, that too
   * has caused MPICH to core on certain platforms.  */
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

  if (rank == 0)
    {
      /* since we want to see whats happening make sure stdout and stderr
       * are unbuffered */
      setbuf(stdout, NULL);
      setbuf(stderr, NULL);
    }

  /* for convenience, set working directory to the test file directory */
  chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  /* Initialize the library. */
  saf_init(SAF_DEFAULT_LIBPROPS);

  /* Get the name of the SAF database. */
  strcpy(dbname, "larry1.saf");

  SAF_TRY_BEGIN
    {
      /* Because we are in a try block here, all failures will send us to
       * the one and only catch block at the end of this test */

      /* Create the database properties. */
      dbprops = saf_createProps_database();

      /* Set the clobber database property so any existing file
       * will be overwritten. */
      saf_setProps_Clobber(dbprops);

      /* Open the SAF database. Give it name dbname, properties p and
       * set db to be a handle to this database. */
      db = saf_open_database(dbname,dbprops);

      /* Get the handle to the master file. */
      saf_file = db;

      /* Construct the base space. */
      make_base_space();

      /* Extend the collection of nodes on the time_base set by 10 so there
       * now is a collection of 11 nodes in this set. */
      saf_extend_collection(SAF_ALL, &time_base, &nodes, 10, SAF_1DC(10));

      /* Construct the coordinate field on the mesh. */
      make_global_coord_field();

      /* Construct the displacement field on the mesh. */
      make_displacement_field();

      /* Construct the distribution factors field on side set 2. */
      make_distribution_factors_on_ss2_field();

      /* Construct the temperature field on node set 1. */
      make_temperature_on_ns1_field();

      /* Construct the temperature field on cell 2. */
      make_temperature_on_cell_2_field();

      /* Construct the stress field on cell 1. */
      make_stress_on_cell_1_field();

      /* Construct the pressure field on side set 1. */
      make_pressure_on_ss1_field();

      /* Construct the time base field. */
      make_time_base_field();

      /* Construct suite to store initial state; store (references to) fields in initial state */
      /*
	make_init_suite();
      */

      /* Construct suite to store (references to) fields through time; store the fields */
      make_time_suite();

      /* Close the SAF database. */
      saf_close_database(db);

    }
  SAF_CATCH
    {
      SAF_CATCH_ALL
	{
	  failed = 1;
	}
    }
  SAF_TRY_END

    /* Finalize access to the library. */
    saf_final();

  if (failed)
    FAILED;
  else
    PASSED;

#ifdef HAVE_PARALLEL
  /* make sure everyone returns the same error status */
  MPI_Bcast(&failed, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Finalize();
#endif

  return failed;
}
