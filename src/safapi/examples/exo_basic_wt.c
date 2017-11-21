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

SAF_Db *db=NULL;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Basic EXODUS test
 *
 * Description: The exo_basic_wt and exo_basic_rd test the SAF library to ensure that all serial EXODUS
 *              functionality is supported.
 *
 * Parallel:    This tests SAF in serial.  There are corresponding tests (exo_par_wt and exo_par_rd) to
 *              test SAF in parallel.
 *
 * Programmer:	Larry Schoof, 11/13/2001
 *
 * Modifications: 
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */

int
main(int argc, char **argv)
{

  /*SAF_Db db;*/
  /* SAF_Handle db; */
  char dbname[1024];
  SAF_DbProps *dbprops=NULL;
  SAF_Role saf_ss_role, saf_ns_role;
  SAF_Cat nodes, elems, blocks, side_sets, node_sets;
  SAF_Set top, block_1, block_2, block_3, block_4, block_5;
  SAF_Set block_set[5];
  SAF_Set side_set_a, side_set_b, node_set_a;
  SAF_Set side_set_a_q, side_set_a_t;
  SAF_Rel rel, trel;

  SAF_FieldTmpl coords_ctmpl, coords_ftmpl, tmp_ftmpl[6];
  SAF_FieldTmpl blk_1_elem_ftmpl, blk_3_elem_ftmpl, ssaq_elem_ftmpl, ssat_elem_ftmpl;
  SAF_FieldTmpl nsa_node_ftmpl, top_node_ctmpl, top_node_ftmpl;
  SAF_FieldTmpl block_5_elem_ctmpl, block_5_elem_ftmpl;
  SAF_FieldTmpl block_1_elem_ctmpl, block_1_elem_ftmpl;
  SAF_FieldTmpl block_2_elem_ctmpl, block_2_elem_ftmpl;
  SAF_FieldTmpl block_3_elem_ctmpl, block_3_elem_ftmpl;
  SAF_FieldTmpl block_elem_ctmpl, block_elem_ftmpl;
  SAF_FieldTmpl top_elem_ftmpl, top_constant_ftmpl;
  SAF_FieldTmpl block_node_ctmpl, block_node_ftmpl;
  SAF_FieldTmpl ssa_elem_ctmpl, ssa_elem_ftmpl;

  SAF_Field x_coords, y_coords, z_coords, coords;
  SAF_Field thickness, dist_fact;
  SAF_Field x_disp, y_disp, z_disp, disp;
  SAF_Field x_vel, y_vel, z_vel, vel;
  SAF_Field sigxx, sigyy, sigzz, sigxy, sigyz, sigzx;
  SAF_Field stress;
  SAF_Field epsxx, epsyy, epszz, epsxy, epsyz, epszx;
  SAF_Field strain, pressure;
  SAF_Field cent_x, cent_y, cent_z, centroid;
  SAF_Field centroid_fld_list[5];
  SAF_Field ke, te;
  SAF_Field x_rot, y_rot, z_rot, rot;
  SAF_Field area;
  SAF_Field x_norm, y_norm, z_norm, normal;
  SAF_Field elem_ids;
  
  SAF_Field tmp_fields[6], *coord_components;

  SAF_Field field_list[40];
  SAF_FieldTmpl field_tmpl_list[40];

  SAF_Suite suite;
  SAF_StateTmpl stmpl;
  SAF_StateGrp state_grp;

  SAF_Quantity *vel_q, *stress_q, *strain_q, *pressure_q, *energy_q, *angle_q, *area_q;
  SAF_Quantity *tmp_q;


  char tmp_name[255];
  int i, index[3];
  float time[10]={1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0};
  
  int failCount = 0;
  int cnt = 0, state;

  void *pbuf;

#ifdef HAVE_PARALLEL
  MPI_Init(&argc,&argv);

  SKIPPED;
  MPI_Finalize();

  exit(0);
#endif

  /*
   * initialize the library
   */

  saf_init(SAF_DEFAULT_LIBPROPS);

  SAF_TRY_BEGIN
    {
      /*
       * open (create) a database 
       */

      strcpy(dbname, TEST_FILE_NAME);

      dbprops = saf_createProps_database();
      saf_setProps_Clobber(dbprops);
      db = saf_open_database(dbname,dbprops);

      /*
       * create categories that will be used in creating collections on sets
       */

      saf_declare_role(SAF_ALL, db, "side sets", NULL, &saf_ss_role); 
      saf_declare_role(SAF_ALL, db, "node sets", NULL, &saf_ns_role);

      saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, 0, &nodes);
      saf_declare_category(SAF_ALL, db, "elems", SAF_TOPOLOGY, 3, &elems);
      saf_declare_category(SAF_ALL, db, "blocks", SAF_BLOCK, 3, &blocks);
      saf_declare_category(SAF_ALL, db, "side_sets", &saf_ss_role, 2, &side_sets);
      saf_declare_category(SAF_ALL, db, "node_sets", &saf_ns_role, 0, &node_sets);




      /* get quantities that will be used in various field templates */ 

      vel_q = saf_find_one_quantity(db,"velocity",NULL); 
    
      tmp_q = saf_find_one_quantity( db, "force",NULL);
      stress_q = saf_declare_quantity(SAF_ALL,db,"stress","stress",NULL,NULL);
      saf_multiply_quantity(SAF_ALL,stress_q, tmp_q, 1);
      saf_multiply_quantity(SAF_ALL,stress_q,SAF_QLENGTH, -2);
    
    
      strain_q = saf_declare_quantity(SAF_ALL,db,"strain", "strain", NULL, NULL); 
      tmp_q = SAF_QLENGTH;
      saf_multiply_quantity(SAF_ALL,strain_q, tmp_q, 1);
      saf_multiply_quantity(SAF_ALL,strain_q, tmp_q, -1);
    
      pressure_q = saf_find_one_quantity (db, "pressure",NULL);
      energy_q = saf_find_one_quantity (db, "energy",NULL);
      angle_q = saf_find_one_quantity(db,"plane angle",NULL);
      area_q = saf_find_one_quantity(db,"area",NULL); 

      /*
       * write out some (3) QA records
       */
      { 
          const char *qa_record[3][64] = {{"FASTQ; FASTQ; 11/16/1989; 12:00:00"}, 
                                          {"GEN3D; GEN3D; 11/16/1989; 17:00:00"}, 
                                          {"JAC3D 05; JAC3D 05; 11/17/1989; 11:26:18"}};
          hid_t str64 = H5Tcopy(H5T_C_S1);
          H5Tset_size(str64, 64);
          saf_put_attribute(SAF_ALL, (ss_pers_t*)db, "QA_RECORDS", str64, 3, qa_record);
          H5Tclose(str64);
      }

      /*
       * write out some INFO records
       */
      {
          const char *info[5][16] = {{"info record 1"},
                                      {"info record 2"},
                                     {"info record 3"},
                                     {"info record 4"},
                                     {"info record 5"}};
          hid_t str16 = H5Tcopy(H5T_C_S1);
          H5Tset_size(str16, 16);
          saf_put_attribute(SAF_ALL, (ss_pers_t*)db, "INFO_RECORDS", str16, 5, info);
      }

      /*
       * create a top set called "TOP_SET"
       */

      saf_declare_set(SAF_ALL, db, "TOP_SET", 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &top);


      /*
       * create collections in "TOP_SET" for:
       *   elements;
       *   nodes;
       *   element blocks;
       *   side sets;
       *   node sets
       */

      saf_declare_collection(SAF_ALL, &top, &nodes, SAF_CELLTYPE_POINT, 15, SAF_1DC(15), SAF_DECOMP_FALSE);
      saf_declare_collection(SAF_ALL, &top, &elems, SAF_CELLTYPE_MIXED, 12, SAF_1DC(12), SAF_DECOMP_TRUE);
      saf_declare_collection(SAF_ALL, &top, &blocks, SAF_CELLTYPE_SET, 5, SAF_1DC(5), SAF_DECOMP_TRUE);
      saf_declare_collection(SAF_ALL, &top, &side_sets, SAF_CELLTYPE_SET, 2, SAF_1DC(2), SAF_DECOMP_FALSE);
      saf_declare_collection(SAF_ALL, &top, &node_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_FALSE);

      /*
       * create "BLOCK_1" with quad shells
       */

      saf_declare_set(SAF_ALL, db, "BLOCK_1", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_1);

      /*
       * optional attribute that EXODUS clients may want to know
       */

      saf_put_set_att(SAF_ALL, &block_1, "EXO_ELEM_TYPE", H5T_C_S1, 1, "SHELL");

      saf_declare_collection(SAF_ALL, &block_1, &elems, SAF_CELLTYPE_QUAD, 2, SAF_1DC(2), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &block_1, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {0,2,1};   /* start, count, stride */
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_collection(SAF_ALL, &block_1, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &block_1, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {0};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      /* NOTE: this node subset relation is necessary because we are going to define a field on the nodes of BLOCK_1 */

      saf_declare_collection(SAF_ALL, &block_1, &nodes, SAF_CELLTYPE_POINT, 6, SAF_1DC(6), SAF_DECOMP_FALSE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &block_1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {0,1,3,4,6,7};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_topo_relation(SAF_ALL, db, &block_1, &elems, &top, &nodes, SAF_SELF(db), &block_1,
				SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
      { int abuf[] = {4};  /* stride (number of nodes per element) */
      int bbuf[] = {3,4,7,6, 0,1,4,3};  /* node list */
      saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, abuf, SAF_INT, bbuf, db);
      }

      /*
       * create "BLOCK_2" with hexes
       */

      saf_declare_set(SAF_ALL, db, "BLOCK_2", 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_2);

      /*
       * optional attribute that EXODUS clients may want to know
       */

      saf_put_set_att(SAF_ALL, &block_2, "EXO_ELEM_TYPE", H5T_C_S1, 1, "HEX");

      saf_declare_collection(SAF_ALL, &block_2, &elems, SAF_CELLTYPE_HEX, 2, SAF_1DC(2), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &top, &block_2, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {2,2,1};   /* start, count, stride */
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_collection(SAF_ALL, &block_2, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &block_2, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {1};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_topo_relation(SAF_ALL, db, &block_2, &elems, &top, &nodes, SAF_SELF(db), &block_2,
				SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
      { int abuf[] = {8};
      int bbuf[] = {3,4,14,13,6,7,10,9, 0,1,12,11,3,4,14,13};
      saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, abuf, SAF_INT, bbuf, db);
      }

      /*
       * create "BLOCK_3" with tri shells
       */

      saf_declare_set(SAF_ALL, db, "BLOCK_3", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_3);

      /*
       * optional attribute that EXODUS clients may want to know
       */

      saf_put_set_att(SAF_ALL, &block_3, "EXO_ELEM_TYPE", H5T_C_S1, 1, "SHELL");

      saf_declare_collection(SAF_ALL, &block_3, &elems, SAF_CELLTYPE_TRI, 4, SAF_1DC(4), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &top, &block_3, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {4,4,1};   /* start, count, stride */
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_collection(SAF_ALL, &block_3, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &block_3, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {2};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      /* NOTE: this node subset relation is necessary because we are going to define a field on the nodes of BLOCK_3 */

      saf_declare_collection(SAF_ALL, &block_3, &nodes, SAF_CELLTYPE_POINT, 6, SAF_1DC(6), SAF_DECOMP_FALSE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &block_3, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {1,2,4,5,7,8};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_topo_relation(SAF_ALL,db, &block_3, &elems, &top, &nodes, SAF_SELF(db), &block_3,
				SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
      { int abuf[] = {3};
      int bbuf[] = {7,5,8, 4,5,7, 4,1,5, 1,2,5};
      saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, abuf, SAF_INT, bbuf, db);
      }

      /*
       * create "BLOCK_4" with pyramids
       */

      saf_declare_set(SAF_ALL, db, "BLOCK_4", 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_4);

      /*
       * optional attribute that EXODUS clients may want to know
       */

      saf_put_set_att(SAF_ALL, &block_4, "EXO_ELEM_TYPE", H5T_C_S1, 1, "PYRAMID");

      saf_declare_collection(SAF_ALL, &block_4, &elems, SAF_CELLTYPE_PYRAMID, 2, SAF_1DC(2), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &top, &block_4, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {8,2,1};   /* start, count, stride */
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_collection(SAF_ALL, &block_4, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &block_4, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {3};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_topo_relation(SAF_ALL, db, &block_4, &elems, &top, &nodes, SAF_SELF(db), &block_4,
				SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
      { int abuf[] = {5};
      int bbuf[] = {4,14,10,7,5, 1,12,14,4,5};
      saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, abuf, SAF_INT, bbuf, db);
      }

      /*
       * create "BLOCK_5" with tets
       */

      saf_declare_set(SAF_ALL, db, "BLOCK_5", 3, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &block_5);

      /*
       * optional attribute that EXODUS clients may want to know
       */

      saf_put_set_att(SAF_ALL, &block_5, "EXO_ELEM_TYPE", H5T_C_S1, 1, "TET");

      saf_declare_collection(SAF_ALL, &block_5, &elems, SAF_CELLTYPE_PYRAMID, 2, SAF_1DC(2), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &top, &block_5, SAF_COMMON(&elems), SAF_HSLAB, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {10,2,1};   /* start, count, stride */
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_collection(SAF_ALL, &block_5, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &block_5, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {4};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_topo_relation(SAF_ALL, db, &block_5, &elems, &top, &nodes, SAF_SELF(db), &block_5,
				SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
      { int abuf[] = {4};
      int bbuf[] = {7,5,10,8, 1,2,12,5};
      saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, abuf, SAF_INT, bbuf, db);
      }

      /*
       * create "SIDE_SET_A" 
       */

      saf_declare_set(SAF_ALL, db, "SIDE_SET_A", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_set_a);

      saf_declare_collection(SAF_ALL, &side_set_a, &elems, SAF_CELLTYPE_MIXED, 2, SAF_1DC(2), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &top, &side_set_a, SAF_EMBEDBND(&elems,&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int abuf[] = {2,10};   /* side set element list */
      int bbuf[] = {5,2};   /* side set side list  */
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, abuf, SAF_INT, bbuf, db);
      }

      saf_declare_collection(SAF_ALL, &side_set_a, &side_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &side_set_a, SAF_COMMON(&side_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {0};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      /*
       * create subsets "SIDE_SET_A_QUADS" and "SIDE_SET_A_TRIS" which are sets of homogeneous primitives
       */

      saf_declare_set(SAF_ALL, db, "SIDE_SET_A_QUADS", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_set_a_q);

      saf_declare_collection(SAF_ALL, &side_set_a_q, &elems, SAF_CELLTYPE_QUAD, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &side_set_a, &side_set_a_q, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {0};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_set(SAF_ALL, db, "SIDE_SET_A_TRIS", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_set_a_t);

      saf_declare_collection(SAF_ALL, &side_set_a_t, &elems, SAF_CELLTYPE_TRI, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &side_set_a, &side_set_a_t, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {1};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      /*
       * create "SIDE_SET_B"
       */

      saf_declare_set(SAF_ALL, db, "SIDE_SET_B", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &side_set_b);

      saf_declare_collection(SAF_ALL, &side_set_b, &elems, SAF_CELLTYPE_MIXED, 4, SAF_1DC(4), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &top, &side_set_b, SAF_EMBEDBND(&elems,&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int abuf[] = {2,5,3,6};   /* side set element list */
      int bbuf[] = {0,0,0,0};   /* side set side list; yes, all of these are the 0th face of the associated elements  */
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, abuf, SAF_INT, bbuf, db);
      }

      saf_declare_collection(SAF_ALL, &side_set_b, &side_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &side_set_b, SAF_COMMON(&side_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {1};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      /*
       * create "NODE_SET_A"
       */

      saf_declare_set(SAF_ALL, db, "NODE_SET_A", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &node_set_a);

      saf_declare_collection(SAF_ALL, &node_set_a, &nodes, SAF_CELLTYPE_POINT, 9, SAF_1DC(9), SAF_DECOMP_TRUE);

      saf_declare_subset_relation(SAF_ALL, db, &top, &node_set_a, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {0,1,2,3,4,5,6,7,8};   /* node set node list */
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      saf_declare_collection(SAF_ALL, &node_set_a, &node_sets, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &top, &node_set_a, SAF_COMMON(&node_sets), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
      { int buf[] = {0};
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, buf, H5I_INVALID_HID, NULL, db);
      }

      /*
       * OK, all the sets are written out
       * now write out the fields
       */


      cnt = 0;

      /******************************************************************************************************
       * field of coordinates of nodes on TOP_SET
       ******************************************************************************************************/

      /*
       * first, declare field templates for the component (X, Y, Z) fields;
       * then declare field templates for the composite (XYZ) field
       */

      saf_declare_field_tmpl(SAF_ALL, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
			     NULL, &coords_ctmpl);

      tmp_ftmpl[0] = coords_ctmpl;
      tmp_ftmpl[1] = coords_ctmpl;
      tmp_ftmpl[2] = coords_ctmpl;


      saf_declare_field_tmpl(SAF_ALL, db, "coordinate_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
			     tmp_ftmpl, &coords_ftmpl);

      /* 
       * declare the component and composite coordinate fields
       */

      saf_declare_field(SAF_ALL, db, &coords_ctmpl, "X", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_coords);
      saf_declare_field(SAF_ALL, db, &coords_ctmpl, "Y", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_coords);
      saf_declare_field(SAF_ALL, db, &coords_ctmpl, "Z", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_coords);

      tmp_fields[0] = x_coords;
      tmp_fields[1] = y_coords;
      tmp_fields[2] = z_coords;
      coord_components = tmp_fields;

      saf_declare_field(SAF_ALL, db, &coords_ftmpl, "coords", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			SAF_FLOAT, coord_components, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &coords);

      {  /* X coordinates */
	/* node ID:    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  */
	float buf[] = {0., 1., 2., 0., 1., 2., 0., 1., 2., 0., 1., 0., 1., 0., 1.};
	void *pbuf = &buf[0];
	saf_write_field(SAF_ALL, &x_coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
      }

      {  /* Y coordinates */
	/* node ID:    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  */
	float buf[] = {0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 1., 1., 1., 1., 1.};
	void *pbuf = &buf[0];
	saf_write_field(SAF_ALL, &y_coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
      }

      {  /* Z coordinates */
	/* node ID:    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  */
	float buf[] = {0., 0., 0., 1., 1., 1., 2., 2., 2., 2., 2., 0., 0., 1., 1.};
	void *pbuf = &buf[0];
	saf_write_field(SAF_ALL, &z_coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
      }

      /* specify that is a coordinate field */
      saf_declare_coords(SAF_ALL, &coords);
      saf_declare_default_coords(SAF_ALL, &top, &coords);

      field_list[cnt] = coords;
      field_tmpl_list[cnt] = coords_ftmpl;
      cnt++;




      /******************************************************************************************************
       * field of constant shell thicknesses of elements in BLOCK_1
       * 1 thickness will be specified per element (constant thru element)
       ******************************************************************************************************/

      saf_declare_field_tmpl(SAF_ALL, db, "blk_1_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
			     NULL, &blk_1_elem_ftmpl);

      saf_declare_field(SAF_ALL, db, &blk_1_elem_ftmpl, "elem_thickness", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &thickness);

      {  /* shell thicknesses for BLOCK_1 (1 per element) */
	float buf[] = {0.01, 0.02};
	void *pbuf = &buf[0];
	saf_write_field(SAF_ALL, &thickness, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
      }

      field_list[cnt] = thickness;
      field_tmpl_list[cnt] = blk_1_elem_ftmpl;
      cnt++;



      /******************************************************************************************************
       * field of varying shell thicknesses of elements in BLOCK_3
       *
       * 3 thicknesses will be specified per element (1 per node)
       *
       * NOTE: this field is not specified to be associated with a node collection the block, which would
       *       enforce thickness continuity at the nodes; 
       *
       *       rather the thicknesses are specified associated with the elements (3 per element) which allows 
       *       for discontinuity at the nodes
       *
       *       the locations at which the thicknesses are associated, as well as the ordering, must currently be assumed 
       *       (at the nodes of the tris, in the order of the nodes in the topo relations)
       ******************************************************************************************************/

      saf_declare_field_tmpl(SAF_ALL, db, "blk_3_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
			     NULL, &blk_3_elem_ftmpl);

      saf_declare_field(SAF_ALL, db, &blk_3_elem_ftmpl, "elem_thickness", &block_3, NULL, SAF_SELF(db),
			&elems, 3, &elems, SAF_SPACE_PWLINEAR,
			SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &thickness);

      {  /* shell thicknesses for BLOCK_3 (3 per element) */
	float buf[] = {.01, .02, .03,  .01, .02, .03,  .01, .02, .03,  .01, .02, .03};
	void *pbuf = &buf[0];
	saf_write_field(SAF_ALL, &thickness, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
      }

      field_list[cnt] = thickness;
      field_tmpl_list[cnt] = blk_3_elem_ftmpl;
      cnt++;



      /******************************************************************************************************
       * field of distribution factors at nodes of SIDE_SET_A
       * these will be fields on the homogeneous subsets of SIDE_SET_A (i.e., SIDE_SET_A_TRIS and SIDE_SET_A_QUADS)
       ******************************************************************************************************/

      saf_declare_field_tmpl(SAF_ALL, db, "ssaq_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QAMOUNT, 1,
			     NULL, &ssaq_elem_ftmpl);

      saf_declare_field(SAF_ALL, db, &ssaq_elem_ftmpl, "dist_factor", &side_set_a_q, NULL, SAF_SELF(db), 
			&elems, 4, &elems, SAF_SPACE_PWLINEAR, SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &dist_fact);

      {  /* distribution factors (4 per element) */
	float buf[] = {.1, .2, .3, .4};
	void *pbuf = &buf[0];
	saf_write_field(SAF_ALL, &dist_fact, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
      }

      field_list[cnt] = dist_fact;
      field_tmpl_list[cnt] = ssaq_elem_ftmpl;
      cnt++;

      saf_declare_field_tmpl(SAF_ALL, db, "ssat_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QAMOUNT, 1,
			     NULL, &ssat_elem_ftmpl);

      saf_declare_field(SAF_ALL, db, &ssat_elem_ftmpl, "dist_factor", &side_set_a_t, NULL, SAF_SELF(db), 
			&elems, 3, &elems, SAF_SPACE_PWLINEAR, SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &dist_fact);

      {  /* distribution factors (3 per element) */
	float buf[] = {.1, .2, .3};
	void *pbuf = &buf[0];
	saf_write_field(SAF_ALL, &dist_fact, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
      }

      field_list[cnt] = dist_fact;
      field_tmpl_list[cnt] = ssat_elem_ftmpl;
      cnt++;



      /******************************************************************************************************
       * field of distribution factors at nodes of NODE_SET_A
       ******************************************************************************************************/

      saf_declare_field_tmpl(SAF_ALL, db, "nsa_node_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QAMOUNT, 1,
			     NULL, &nsa_node_ftmpl);

      /*
	saf_declare_field(SAF_ALL, db, &nsa_node_ftmpl, "dist_factor", &node_set_a, NULL, SAF_SELF(db), 
	SAF_NODAL(&nodes,&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &dist_fact);
      */

      saf_declare_field(SAF_ALL, db, &nsa_node_ftmpl, "dist_factor", &node_set_a, NULL, SAF_SELF(db), 
			&nodes, 1, &nodes, SAF_SPACE_PWLINEAR, SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &dist_fact);

      {  /* distribution factors (1 per node) */
	float buf[] = {.1, .2, .3, .4, .5, .6, .7, .8, .9};
	void *pbuf = &buf[0];
	saf_write_field(SAF_ALL, &dist_fact, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
      }

      field_list[cnt] = dist_fact;
      field_tmpl_list[cnt] = nsa_node_ftmpl;
      cnt++;




      for (state=0, cnt=0; state<2; state++) {

	/******************************************************************************************************
	 * displacement vector field associated with nodes of TOP_SET
	 ******************************************************************************************************/

	/*
	 * first, declare field templates for the component (DX, DY, DZ) fields;
	 * then declare field templates for the composite (DISP) field
	 */

	saf_declare_field_tmpl(SAF_ALL, db, "top_node_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
			       NULL, &top_node_ctmpl);

	tmp_ftmpl[0] = top_node_ctmpl;
	tmp_ftmpl[1] = top_node_ctmpl;
	tmp_ftmpl[2] = top_node_ctmpl;

	saf_declare_field_tmpl(SAF_ALL, db, "top_node_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
			       tmp_ftmpl, &top_node_ftmpl);
  
	/* 
	 * declare the component and composite displacement fields
	 */

	saf_declare_field(SAF_ALL, db, &top_node_ctmpl, "DISP_X", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_disp);
	saf_declare_field(SAF_ALL, db, &top_node_ctmpl, "DISP_Y", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_disp);
	saf_declare_field(SAF_ALL, db, &top_node_ctmpl, "DISP_Z", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_disp);

	tmp_fields[0] = x_disp;
	tmp_fields[1] = y_disp;
	tmp_fields[2] = z_disp;

	saf_declare_field(SAF_ALL, db, &top_node_ftmpl, "displacement", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			  SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &disp);

	{  /* X displacements */
	  /* node ID:     0     1     2     3     4     5     6     7     8     9    10    11    12    13    14  */
	  float buf[] = {1.00, 1.01, 1.02, 1.03, 1.04, 1.05, 1.06, 1.07, 1.08, 1.09, 1.10, 1.11, 1.12, 1.13, 1.14};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &x_disp, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* Y displacements */
	  /* node ID:     0     1     2     3     4     5     6     7     8     9    10    11    12    13    14  */
	  float buf[] = {2.00, 2.01, 2.02, 2.03, 2.04, 2.05, 2.06, 2.07, 2.08, 2.09, 2.10, 2.11, 2.12, 2.13, 2.14};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &y_disp, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* Z displacements */
	  /* node ID:     0     1     2     3     4     5     6     7     8     9    10    11    12    13    14  */
	  float buf[] = {3.00, 3.01, 3.02, 3.03, 3.04, 3.05, 3.06, 3.07, 3.08, 3.09, 3.10, 3.11, 3.12, 3.13, 3.14};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &z_disp, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = disp;
	field_tmpl_list[cnt] = top_node_ftmpl;
	cnt++;

	/******************************************************************************************************
	 * velocity vector field associated with nodes of TOP_SET
	 ******************************************************************************************************/

	/*
	 * first, declare field templates for the component (VX, VY, VZ) fields;
	 * then declare field templates for the composite (VEL) field
	 */

 
	saf_declare_field_tmpl(SAF_ALL, db, "top_node_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, 
			       vel_q, 1, NULL, &top_node_ctmpl);

	tmp_ftmpl[0] = top_node_ctmpl;
	tmp_ftmpl[1] = top_node_ctmpl;
	tmp_ftmpl[2] = top_node_ctmpl;

	saf_declare_field_tmpl(SAF_ALL, db, "top_node_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, vel_q, 3,
			       tmp_ftmpl, &top_node_ftmpl);

	/*
	 * declare the component and composite velocity fields
	 */

	saf_declare_field(SAF_ALL, db, &top_node_ctmpl, "VEL_X", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_vel);
	saf_declare_field(SAF_ALL, db, &top_node_ctmpl, "VEL_Y", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_vel);
	saf_declare_field(SAF_ALL, db, &top_node_ctmpl, "VEL_Z", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_vel);

	tmp_fields[0] = x_vel;
	tmp_fields[1] = y_vel;
	tmp_fields[2] = z_vel;

	saf_declare_field(SAF_ALL, db, &top_node_ftmpl, "velocity", &top, NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			  SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &vel);

	{  /* X velocities */
	  float buf[] = {10.00, 10.01, 10.02, 10.03, 10.04, 10.05, 10.06, 10.07, 10.08, 10.09, 10.10, 10.11, 10.12, 10.13, 10.14};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &x_vel, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* Y velocities */
	  float buf[] = {20.00, 20.01, 20.02, 20.03, 20.04, 20.05, 20.06, 20.07, 20.08, 20.09, 20.10, 20.11, 20.12, 20.13, 20.14};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &y_vel, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* Z velocities */
	  float buf[] = {30.00, 30.01, 30.02, 30.03, 30.04, 30.05, 30.06, 30.07, 30.08, 30.09, 30.10, 30.11, 30.12, 30.13, 30.14};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &z_vel, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = vel;
	field_tmpl_list[cnt] = top_node_ftmpl;
	cnt++;

	/******************************************************************************************************
	 * stress symmetric tensor field associated with elements of BLOCK_2 and BLOCK_5
	 ******************************************************************************************************/

	/*
	 * first, declare field templates for the component (SIGXX, SIGYY, SIGZZ, SIGXY, SIGYZ, SIGZX) fields;
	 * then declare field templates for the composite (SIG) field
	 */


	/*
	 * stress on BLOCK_2
	 */

	saf_declare_field_tmpl(SAF_ALL, db, "block_2_elem_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, 
			       stress_q, 1, NULL, &block_2_elem_ctmpl);

	tmp_ftmpl[0] = block_2_elem_ctmpl;
	tmp_ftmpl[1] = block_2_elem_ctmpl;
	tmp_ftmpl[2] = block_2_elem_ctmpl;
	tmp_ftmpl[3] = block_2_elem_ctmpl;
	tmp_ftmpl[4] = block_2_elem_ctmpl;
	tmp_ftmpl[5] = block_2_elem_ctmpl;

	saf_declare_field_tmpl(SAF_ALL, db, "block_2_elem_ftmpl", SAF_ALGTYPE_SYMTENSOR, SAF_CARTESIAN, stress_q, 6,
			       tmp_ftmpl, &block_2_elem_ftmpl);

	/*
	 * declare the component and composite stress fields
	 */

	saf_declare_field(SAF_ALL, db, &block_2_elem_ctmpl, "SIGXX", &block_2, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigxx);
	saf_declare_field(SAF_ALL, db, &block_2_elem_ctmpl, "SIGYY", &block_2, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigyy);
	saf_declare_field(SAF_ALL, db, &block_2_elem_ctmpl, "SIGZZ", &block_2, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigzz);
	saf_declare_field(SAF_ALL, db, &block_2_elem_ctmpl, "SIGXY", &block_2, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigxy);
	saf_declare_field(SAF_ALL, db, &block_2_elem_ctmpl, "SIGYZ", &block_2, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigyz);
	saf_declare_field(SAF_ALL, db, &block_2_elem_ctmpl, "SIGZX", &block_2, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigzx);

	tmp_fields[0] = sigxx;
	tmp_fields[1] = sigyy;
	tmp_fields[2] = sigzz;
	tmp_fields[3] = sigxy;
	tmp_fields[4] = sigyz;
	tmp_fields[5] = sigzx;

	saf_declare_field(SAF_ALL, db, &block_2_elem_ftmpl, "stress", &block_2, NULL, SAF_SELF(db),  SAF_ZONAL(&elems),
			  SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &stress);

	{  /* SIGXX */
	  float buf[] = {100., 101.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigxx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGYY */
	  float buf[] = {200., 201.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigyy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGZZ */
	  float buf[] = {300., 301.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigzz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGXY */
	  float buf[] = {400., 401.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigxy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGYZ */
	  float buf[] = {500., 501.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigyz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGZX */
	  float buf[] = {600., 601.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigzx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = stress;
	field_tmpl_list[cnt] = block_2_elem_ftmpl;
	cnt++;

	/*
	 * stress on BLOCK_5
	 */

	saf_declare_field_tmpl(SAF_ALL, db, "block_5_elem_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, 
			       stress_q, 1, NULL, &block_5_elem_ctmpl);

	tmp_ftmpl[0] = block_5_elem_ctmpl;
	tmp_ftmpl[1] = block_5_elem_ctmpl;
	tmp_ftmpl[2] = block_5_elem_ctmpl;
	tmp_ftmpl[3] = block_5_elem_ctmpl;
	tmp_ftmpl[4] = block_5_elem_ctmpl;
	tmp_ftmpl[5] = block_5_elem_ctmpl;

	saf_declare_field_tmpl(SAF_ALL, db, "block_5_elem_ftmpl", SAF_ALGTYPE_SYMTENSOR, SAF_CARTESIAN, stress_q, 6,
			       tmp_ftmpl, &block_5_elem_ftmpl);

	/*
	 * declare the component and composite stress fields
	 */

	saf_declare_field(SAF_ALL, db, &block_5_elem_ctmpl, "SIGXX", &block_5, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigxx);
	saf_declare_field(SAF_ALL, db, &block_5_elem_ctmpl, "SIGYY", &block_5, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigyy);
	saf_declare_field(SAF_ALL, db, &block_5_elem_ctmpl, "SIGZZ", &block_5, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigzz);
	saf_declare_field(SAF_ALL, db, &block_5_elem_ctmpl, "SIGXY", &block_5, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigxy);
	saf_declare_field(SAF_ALL, db, &block_5_elem_ctmpl, "SIGYZ", &block_5, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigyz);
	saf_declare_field(SAF_ALL, db, &block_5_elem_ctmpl, "SIGZX", &block_5, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &sigzx);
  
	tmp_fields[0] = sigxx;
	tmp_fields[1] = sigyy;
	tmp_fields[2] = sigzz;
	tmp_fields[3] = sigxy;
	tmp_fields[4] = sigyz;
	tmp_fields[5] = sigzx;

	saf_declare_field(SAF_ALL, db, &block_5_elem_ftmpl, "stress", &block_5, NULL, SAF_SELF(db),  SAF_ZONAL(&elems),
			  SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &stress);

	{  /* SIGXX */
	  float buf[] = {100., 101.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigxx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGYY */
	  float buf[] = {200., 201.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigyy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGZZ */
	  float buf[] = {300., 301.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigzz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGXY */
	  float buf[] = {400., 401.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigxy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGYZ */
	  float buf[] = {500., 501.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigyz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* SIGZX */
	  float buf[] = {600., 601.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &sigzx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = stress;
	field_tmpl_list[cnt] = block_5_elem_ftmpl;
	cnt++;

	/******************************************************************************************************
	 * strain symmetric tensor field associated with elements of BLOCK_1 and BLOCK_3
	 ******************************************************************************************************/

	/*
	 * first, declare field templates for the component (EPSXX, EPSYY, EPSZZ, EPSXY, EPSYZ, EPSZX) fields;
	 * then declare field templates for the composite (EPS) field
	 */
	/*
	 * strain on BLOCK_1
	 */

	saf_declare_field_tmpl(SAF_ALL, db, "block_1_elem_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, 
			       strain_q, 1, NULL, &block_1_elem_ctmpl);

	tmp_ftmpl[0] = block_1_elem_ctmpl;
	tmp_ftmpl[1] = block_1_elem_ctmpl;
	tmp_ftmpl[2] = block_1_elem_ctmpl;
	tmp_ftmpl[3] = block_1_elem_ctmpl;
	tmp_ftmpl[4] = block_1_elem_ctmpl;
	tmp_ftmpl[5] = block_1_elem_ctmpl;

	saf_declare_field_tmpl(SAF_ALL, db, "block_1_elem_ftmpl", SAF_ALGTYPE_SYMTENSOR, SAF_CARTESIAN, strain_q, 6,
			       tmp_ftmpl, &block_1_elem_ftmpl);

	/*
	 * declare the component and composite strain fields
	 */

	saf_declare_field(SAF_ALL, db, &block_1_elem_ctmpl, "EPSXX", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsxx);
	saf_declare_field(SAF_ALL, db, &block_1_elem_ctmpl, "EPSYY", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsyy);
	saf_declare_field(SAF_ALL, db, &block_1_elem_ctmpl, "EPSZZ", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epszz);
	saf_declare_field(SAF_ALL, db, &block_1_elem_ctmpl, "EPSXY", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsxy);
	saf_declare_field(SAF_ALL, db, &block_1_elem_ctmpl, "EPSYZ", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsyz);
	saf_declare_field(SAF_ALL, db, &block_1_elem_ctmpl, "EPSZX", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epszx);

	tmp_fields[0] = epsxx;
	tmp_fields[1] = epsyy;
	tmp_fields[2] = epszz;
	tmp_fields[3] = epsxy;
	tmp_fields[4] = epsyz;
	tmp_fields[5] = epszx;
  
	saf_declare_field(SAF_ALL, db, &block_1_elem_ftmpl, "strain", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &strain);
    
	{  /* EPSXX */
	  float buf[] = {.1, .11};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epsxx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSYY */
	  float buf[] = {.2, .21};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epsyy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSZZ */
	  float buf[] = {.3, .31};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epszz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSXY */
	  float buf[] = {.4, .41};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epsxy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSYZ */
	  float buf[] = {.5, .51};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epsyz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSZX */
	  float buf[] = {.6, .61};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epszx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = strain;
	field_tmpl_list[cnt] = block_1_elem_ftmpl;
	cnt++;

	/*
	 * strain on BLOCK_3
	 */
  
	saf_declare_field_tmpl(SAF_ALL, db, "block_3_elem_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, 
			       strain_q, 1, NULL, &block_3_elem_ctmpl);

	tmp_ftmpl[0] = block_3_elem_ctmpl;
	tmp_ftmpl[1] = block_3_elem_ctmpl;
	tmp_ftmpl[2] = block_3_elem_ctmpl;
	tmp_ftmpl[3] = block_3_elem_ctmpl;
	tmp_ftmpl[4] = block_3_elem_ctmpl;
	tmp_ftmpl[5] = block_3_elem_ctmpl;

	saf_declare_field_tmpl(SAF_ALL, db, "block_3_elem_ftmpl", SAF_ALGTYPE_SYMTENSOR, SAF_CARTESIAN, strain_q, 6,
			       tmp_ftmpl, &block_3_elem_ftmpl);

	/*
	 * declare the component and composite strain fields
	 */
  
	saf_declare_field(SAF_ALL, db, &block_3_elem_ctmpl, "EPSXX", &block_3, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsxx);
	saf_declare_field(SAF_ALL, db, &block_3_elem_ctmpl, "EPSYY", &block_3, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsyy);
	saf_declare_field(SAF_ALL, db, &block_3_elem_ctmpl, "EPSZZ", &block_3, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epszz);
	saf_declare_field(SAF_ALL, db, &block_3_elem_ctmpl, "EPSXY", &block_3, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsxy);
	saf_declare_field(SAF_ALL, db, &block_3_elem_ctmpl, "EPSYZ", &block_3, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epsyz);
	saf_declare_field(SAF_ALL, db, &block_3_elem_ctmpl, "EPSZX", &block_3, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &epszx);

	tmp_fields[0] = epsxx;
	tmp_fields[1] = epsyy;
	tmp_fields[2] = epszz;
	tmp_fields[3] = epsxy;
	tmp_fields[4] = epsyz;
	tmp_fields[5] = epszx;

	saf_declare_field(SAF_ALL, db, &block_3_elem_ftmpl, "strain", &block_3, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &strain);

	{  /* EPSXX */
	  float buf[] = {.1, .11};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epsxx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSYY */
	  float buf[] = {.2, .21};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epsyy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSZZ */
	  float buf[] = {.3, .31};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epszz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSXY */
	  float buf[] = {.4, .41};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epsxy, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSYZ */
	  float buf[] = {.5, .51};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epsyz, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* EPSZX */
	  float buf[] = {.6, .61};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &epszx, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = strain;
	field_tmpl_list[cnt] = block_3_elem_ftmpl;
	cnt++;

	/******************************************************************************************************
	 * pressure scalar field associated with elements of BLOCK_1 and BLOCK_2
	 ******************************************************************************************************/

	/*
	 * pressure on BLOCK_1
	 */

	/*
	 * first, declare field template
	 */

  
	saf_declare_field_tmpl(SAF_ALL, db, "block_1_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, pressure_q, 1,
			       NULL, &block_1_elem_ftmpl);

	/* 
	 * declare the field
	 */

	saf_declare_field(SAF_ALL, db, &block_1_elem_ftmpl, "PRESSURE", &block_1, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &pressure);
  
	{  /* pressures */
	  float buf[] = {1000., 2000.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &pressure, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = pressure;
	field_tmpl_list[cnt] = block_1_elem_ftmpl;
	cnt++;

	/*
	 * pressure on BLOCK_2
	 */

	/*
	 * first, declare field template
	 */

	saf_declare_field_tmpl(SAF_ALL, db, "block_2_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, pressure_q, 1,
			       NULL, &block_2_elem_ftmpl);

	/* 
	 * declare the field
	 */

	saf_declare_field(SAF_ALL, db, &block_2_elem_ftmpl, "PRESSURE", &block_2, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &pressure);

	{  /* pressures */
	  float buf[] = {3000., 4000.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &pressure, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = pressure;
	field_tmpl_list[cnt] = block_2_elem_ftmpl;
	cnt++;

	/******************************************************************************************************
	 * centroid vector field associated with elements of BLOCK_1, BLOCK_2, BLOCK_3, BLOCK_4, and BLOCK_5
	 ******************************************************************************************************/

	{
	  /* element ID    0     1     2     3    4     5     6     7     8     9    10    11 */
	  float xbuf[] = { .5,   .5,   .5,   .5, 1.67, 1.33, 1.33, 1.67, 1.33, 1.33, 1.5 , 1.5};
	  float ybuf[] = {0. ,  0. ,   .5,   .5, 0.  , 0.  , 0.  , 0.  ,  .33,  .33,  .25,  .25};
	  float zbuf[] = {1.5,   .5,  1.5,   .5, 1.67, 1.33,  .67,  .33,  .67, 1.33, 1.75, 1.25};
  
	  int index[]  = {0, 2, 4, 8, 10};   /* index of beginning element for each block */

	  block_set[0] = block_1;
	  block_set[1] = block_2;
	  block_set[2] = block_3;
	  block_set[3] = block_4;
	  block_set[4] = block_5;

	  for (i=0; i<5; i++) {

	    /*
	     * first, declare field templates for the component (CX, CY, CZ) fields;
	     * then declare field templates for the composite (CENTROID) field
	     */

	    sprintf (tmp_name, "block_%d_elem_ctmpl", i+1);

	    saf_declare_field_tmpl(SAF_ALL, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
				   NULL, &block_elem_ctmpl);

	    tmp_ftmpl[0] = block_elem_ctmpl;
	    tmp_ftmpl[1] = block_elem_ctmpl;
	    tmp_ftmpl[2] = block_elem_ctmpl;

	    sprintf (tmp_name, "block_%d_elem_tmpl", i+1);

	    saf_declare_field_tmpl(SAF_ALL, db, tmp_name, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
				   tmp_ftmpl, &block_elem_ftmpl);

	    /* 
	     * declare the component and composite centroid fields
	     */

	    saf_declare_field(SAF_ALL, db, &block_elem_ctmpl, "CENT_X", &(block_set[i]), NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &cent_x);

	    pbuf = (void *)(&(xbuf[index[i]]));

	    saf_write_field(SAF_ALL, &cent_x, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
    
	    saf_declare_field(SAF_ALL, db, &block_elem_ctmpl, "CENT_Y", &(block_set[i]), NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &cent_y);

	    pbuf = (void *)(&(ybuf[index[i]]));

	    saf_write_field(SAF_ALL, &cent_y, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);

	    saf_declare_field(SAF_ALL, db, &block_elem_ctmpl, "CENT_Z", &(block_set[i]), NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			      SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &cent_z);

	    pbuf = (void *)(&(zbuf[index[i]]));

	    saf_write_field(SAF_ALL, &cent_z, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);

	    tmp_fields[0] = cent_x;
	    tmp_fields[1] = cent_y;
	    tmp_fields[2] = cent_z;

	    saf_declare_field(SAF_ALL, db, &block_elem_ftmpl, "centroid", &(block_set[i]), NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			      SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &centroid);

	    centroid_fld_list[i] = centroid;

	  }
	}



	/******************************************************************************************************
	 * centroids for TOP
	 * this will be an indirect field that references the centroid fields of each of the blocks
	 ******************************************************************************************************/

	saf_declare_field_tmpl(SAF_ALL, db, "top_elem_tmpl", SAF_ALGTYPE_FIELD, SAF_ANY_BASIS, 
			       SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &top_elem_ftmpl);

	saf_declare_field(SAF_ALL, db, &top_elem_ftmpl, "centroid", &top, NULL, &blocks, SAF_ZONAL(&elems),
			  SAF_HANDLE, NULL, SAF_INTERLEAVE_INDEPENDENT, NULL, NULL, &centroid);

	pbuf = (void *)(&(centroid_fld_list[0]));
	saf_write_field(SAF_ALL, &centroid, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);

	field_list[cnt] = centroid;
	field_tmpl_list[cnt] = top_elem_ftmpl;
	cnt++;


	/******************************************************************************************************
	 * kinetic energy constant field associated with TOP_SET
	 ******************************************************************************************************/
    
	saf_declare_field_tmpl(SAF_ALL, db, "top_constant", SAF_ALGTYPE_SCALAR, SAF_ANY_BASIS, 
			       energy_q, 1, NULL, &top_constant_ftmpl);


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

 

	/******************************************************************************************************
	 * total energy constant field associated with TOP_SET
	 ******************************************************************************************************/
      

	saf_declare_field(SAF_ALL, db, &top_constant_ftmpl, "TOTAL_ENERGY", &top, NULL, SAF_SELF(db), SAF_CONSTANT(db),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, NULL, NULL, &te);

	{  
	  float buf[] = {20000.};
	  pbuf = (void *)(&buf[0]);
	  saf_write_field(SAF_ALL, &te, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = te;
	field_tmpl_list[cnt] = top_constant_ftmpl;
	cnt++;
      
      
	/******************************************************************************************************
	 * nodal rotation field with components ROTX, ROTY, and ROTZ associated with nodes of BLOCK_1 and BLOCK_3
	 ******************************************************************************************************/
  

	block_set[0] = block_1;
	block_set[1] = block_3;

	for (i=0; i<2; i++) {

	  /*
	   * first, declare field templates for the component (ROTX, ROTY, ROTZ) fields;
	   * then declare field templates for the composite (ROTATION) field
	   */

	  sprintf (tmp_name, "block_%d_node_ctmpl", i+1);
	  saf_declare_field_tmpl(SAF_ALL, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, 
				 angle_q, 1, NULL, &block_node_ctmpl);

	  tmp_ftmpl[0] = block_node_ctmpl;
	  tmp_ftmpl[1] = block_node_ctmpl;
	  tmp_ftmpl[2] = block_node_ctmpl;

	  saf_declare_field_tmpl(SAF_ALL, db, "block_node_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, angle_q, 3,
				 tmp_ftmpl, &block_node_ftmpl);

	  /*
	   * declare the component and composite rotation fields
	   */

	  saf_declare_field(SAF_ALL, db, &block_node_ctmpl, "ROT_X", &(block_set[i]), NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_rot);
	  saf_declare_field(SAF_ALL, db, &block_node_ctmpl, "ROT_Y", &(block_set[i]), NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_rot);
	  saf_declare_field(SAF_ALL, db, &block_node_ctmpl, "ROT_Z", &(block_set[i]), NULL, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
			    SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_rot);

	  tmp_fields[0] = x_rot;
	  tmp_fields[1] = y_rot;
	  tmp_fields[2] = z_rot;

	  saf_declare_field(SAF_ALL, db, &block_node_ftmpl, "ROTATION", &(block_set[i]), NULL, SAF_SELF(db), 
			    SAF_NODAL(&nodes, &elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &rot);

	  {  /* X rotation */
	    float buf[] = {.010, .011, .011, .011, .011, .011};
	    void *pbuf = &buf[0];
	    saf_write_field(SAF_ALL, &x_rot, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	  }

	  {  /* Y rotation */
	    float buf[] = {.020, .021, .022, .023, .024, .025};
	    void *pbuf = &buf[0];
	    saf_write_field(SAF_ALL, &y_rot, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	  }

	  {  /* Z rotation */
	    float buf[] = {.030, .031, .032, .033, .034, .035};
	    void *pbuf = &buf[0];
	    saf_write_field(SAF_ALL, &z_rot, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	  }

	  field_list[cnt] = rot;
	  field_tmpl_list[cnt] = block_node_ftmpl;
	  cnt++;

	}

	/******************************************************************************************************
	 * area field associated with elements of SIDE_SET_A
	 ******************************************************************************************************/


	/*
	 * first, declare field template
	 */


	saf_declare_field_tmpl(SAF_ALL, db, "ssa_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, area_q, 1,
			       NULL, &ssa_elem_ftmpl);

	/* 
	 * declare the field
	 */

	saf_declare_field(SAF_ALL, db, &ssa_elem_ftmpl, "AREA", &side_set_a, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &area);

	{  /* areas */
	  float buf[] = {1., .5};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &area, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = area;
	field_tmpl_list[cnt] = ssa_elem_ftmpl;
	cnt++;

	/******************************************************************************************************
	 * field of vector normals with components NX, NY, and NZ associated with elements of SIDE_SET_A
	 ******************************************************************************************************/

	/*
	 * first, declare field templates for the component (NX, NY, NZ) fields;
	 * then declare field templates for the composite (NORMALS) field
	 */


	saf_declare_field_tmpl(SAF_ALL, db, "ssa_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, 
			       SAF_QLENGTH, 1, NULL, &ssa_elem_ctmpl);
  
	tmp_ftmpl[0] = ssa_elem_ctmpl;
	tmp_ftmpl[1] = ssa_elem_ctmpl;
	tmp_ftmpl[2] = ssa_elem_ctmpl;

	saf_declare_field_tmpl(SAF_ALL, db, "ssa_elem_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
			       tmp_ftmpl, &ssa_elem_ftmpl);

	/*
	 * declare the component and composite normal fields
	 */

	saf_declare_field(SAF_ALL, db, &ssa_elem_ctmpl, "NX", &side_set_a, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &x_norm);
	saf_declare_field(SAF_ALL, db, &ssa_elem_ctmpl, "NY", &side_set_a, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &y_norm);
	saf_declare_field(SAF_ALL, db, &ssa_elem_ctmpl, "NZ", &side_set_a, NULL, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &z_norm);

	tmp_fields[0] = x_norm;
	tmp_fields[1] = y_norm;
	tmp_fields[2] = z_norm;

	saf_declare_field(SAF_ALL, db, &ssa_elem_ftmpl, "NORMAL", &side_set_a, NULL, SAF_SELF(db), 
			  SAF_ZONAL(&elems), SAF_FLOAT, tmp_fields, SAF_INTERLEAVE_COMPONENT, SAF_IDENTITY, NULL, &normal);

	{  /* X normal */
	  float buf[] = {0., 0.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &x_norm, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* Y normal */
	  float buf[] = {0., 0.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &y_norm, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	{  /* Z normal */
	  float buf[] = {1., 1.};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &z_norm, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = normal;
	field_tmpl_list[cnt] = ssa_elem_ftmpl;
	cnt++;

	/******************************************************************************************************
	 * ID integer field associated with elements of TOP_SET
	 ******************************************************************************************************/

	/*
	 * first, declare field template
	 */


	saf_declare_field_tmpl(SAF_ALL, db, "top_elem_ftmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_NOT_APPLICABLE_QUANTITY, 1,
			       NULL, &top_elem_ftmpl);

	/* 
	 * declare the field
	 */

	saf_declare_field(SAF_ALL, db, &top_elem_ftmpl, "ELEM_IDS", &top, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(db), SAF_ZONAL(&elems),
			  SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &elem_ids);


	{  /* element IDs */
	  int buf[] = {100, 101, 102, 103, 104, 105, 106, 207, 208, 209, 210, 211};
	  void *pbuf = &buf[0];
	  saf_write_field(SAF_ALL, &elem_ids, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, db);
	}

	field_list[cnt] = elem_ids;
	field_tmpl_list[cnt] = top_elem_ftmpl;
	cnt++; 

	if (state==0) {  /* first time thru, declare suite and state field */
	  saf_declare_suite(SAF_ALL,db,"TIME_SUITE",&top,NULL, &suite);
	  saf_declare_state_tmpl(SAF_ALL, db, "TIME_SUITE_TMPL", cnt, field_tmpl_list, &stmpl);
	  saf_declare_state_group(SAF_ALL,db, "TIME_STATE_GROUP",&suite,&top,&stmpl,SAF_QTIME,SAF_ANY_UNIT,
				  SAF_FLOAT,&state_grp);
	}

	index[0] = state;
	saf_write_state(SAF_ALL, &state_grp, index[0], &top, SAF_FLOAT, &(time[state]), field_list);

      }

      printf("closing database\n");
      saf_close_database(db);

    }
  SAF_CATCH
    {
      SAF_CATCH_ALL
	{
	  FAILED;
	  failCount += 1;
	}
    }
  SAF_TRY_END


  saf_final();


  return (failCount==0)? 0 : 1;

}
