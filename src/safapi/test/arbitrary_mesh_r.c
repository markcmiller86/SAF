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
 * Purpose:    	Test of read of an arbitrary mesh and fields defined on it.
 *
 * Description: This test demonstrates reading of a simple 2D mesh made up of
 *              quads and triangles immersed in 2D Euclidean vector space.  The
 *              lower half of the mesh is triangles and the upper half is
 *              quads.  The mesh has 3 fields defined on it--the global
 *              coordinate field, a scalar field and a rank 2 symmetric tensor.
 *              The global coordinate and scalar fields have their dofs defined
 *              on nodes while the tensor field's dofs are defined on the
 *              elements.  The program accepts one optional argument,
 *              do_interactive.  If the program is invoked without this
 *              argument, then the generated mesh will have 1 edge in x and 2
 *              in y.  If it is invoked with do_interactive, the user will be
 *              prompted for the number of x and y edges.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of
 *              SAF_ALL mode in all calls.
 *
 * Programmer:	William J. Arrighi, LLNL, Monday Jan 29, 2001
 *
 * Modifications:
 *
 *              Jim Reus, 22Feb2001
 *              - Modified the saf_find_files test to change the wildcard
 *                style pattern "*.saf" into a regular expression "^.+\.saf$"
 *                which is one-or-more of "anything" followed by an actual
 *                period followed by the string "saf".
 *
 *-----------------------------------------------------------------------------
 */
SAF_Db *db=NULL;                /* Handle to the SAF database. */
SAF_Cat *nodes = NULL,    /* Handles to the node and elements categories. */
        *elems = NULL;
SAF_Set *mesh = NULL;     /* Handle to the mesh set. */
int edge_ct_x, edge_ct_y, /* The number of edges in the x and y directions. */
    node_ct_x, node_ct_y, /* The number of nodes in the x and y directions. */
    tri_edge_ct_y,        /* Number of triangle edges in the y direction. */
    quad_edge_ct_y,       /* Number of quad edges in the y direction. */
    node_ct,              /* Total number of nodes in the mesh. */
    ele_ct,               /* Total number of elements in the mesh. */
    failCount = 0;

#define NODE_ID(i, j) ((i)*node_ct_y + (j))

static void
read_user_input(void)
{
   edge_ct_x = -1;

   while (edge_ct_x < 1)
   {
      printf("Enter the number of elements in the x direction: ");
      scanf("%i", &edge_ct_x);
      if (edge_ct_x < 1)
         printf("\n***There must be at least 1 element in the x direction.***\n\n");
   }

   edge_ct_y = -1;

   while (edge_ct_y < 1)
   {
      printf("Enter the number of elements in the y direction: ");
      scanf("%i", &edge_ct_y);
      if (edge_ct_y < 1)
         printf("\n***There must be at least 1 element in the y direction.***\n\n");
   }

   return;
}

static int *
make_ele_node_ct_array(void)
{
   /* Create an array containing the node count for each element in the mesh.
    * This forms the abuf arg for the topo relation.
    */
   int *abuf,
       i, j, ele_idx = 0;

   abuf = (int*)malloc(ele_ct*sizeof(int));
   for(i=0; i<edge_ct_x; i++)
   {
      for(j=0; j<tri_edge_ct_y; j++)
      {
         abuf[ele_idx++] = 3;
         abuf[ele_idx++] = 3;
      }
      for(j=0; j<quad_edge_ct_y; j++)
         abuf[ele_idx++] = 4;
   }

   return abuf;
}

static int *
make_mesh_connectivity(void)
{ 
   /* Creates a rectangular array of triangles and quads
    *
    * Number of triangles in x-dir = edge_ct_x*edge_ct_y/2*2.
    * Number of quads in y-dir = edge_ct_x*(edge_ct_y+1)/2. */


   int *node_ids,                      /* Node to element connectivity. */
       conn_ub,                        /* Connectivity array dimension. */
       conn_idx = 0,                   /* Index into connectivity array. */
       i, j;

   /* Preconditions: */

   /* Body: */

   /* allocate node_ids as single index array */
   conn_ub = edge_ct_x*(tri_edge_ct_y*6+quad_edge_ct_y*4);
   node_ids = (int*)malloc(conn_ub*sizeof(int));

   for(i = 0; i < edge_ct_x; i++)
   {
      for(j = 0; j < tri_edge_ct_y; j++)
      {
         /* Each (i,j) pair is the lower-left-hand node in
          * a quad containing two triangles. */

         /* Connectivity for lower triangle in quad,
          * follow right-hand rule (ccw) starting
          * in llh corner of quad. */ 

         node_ids[conn_idx++] = NODE_ID(i,j);
         node_ids[conn_idx++] = NODE_ID(i+1, j);
         node_ids[conn_idx++] = NODE_ID(i, j+1);

         /* Connectivity for upper triangle in quad,
          * follow right-hand rule (ccw) starting
          * in urh corner of quad. */

         node_ids[conn_idx++] = NODE_ID(i+1,j);
         node_ids[conn_idx++] = NODE_ID(i+1, j+1);
         node_ids[conn_idx++] = NODE_ID(i, j+1);
      }
      for(; j < edge_ct_y; j++)
      {
         /* Each (i,j) pair is the lower-left-hand node in a quad. */ 

         node_ids[conn_idx++] = NODE_ID(i,j);
         node_ids[conn_idx++] = NODE_ID(i+1, j);
         node_ids[conn_idx++] = NODE_ID(i+1, j+1);
         node_ids[conn_idx++] = NODE_ID(i, j+1);
      }
   } 

   /* Postconditions: */

   /* Exit */

  return node_ids;
}

static void
read_base_space(void)
{
   /* Reads the mesh from the saf_file associated with db. */

   SAF_Rel *trels = NULL;   /* Handle to the topological relations found. */
   SAF_Cat (coll_cats, p_coll_cats, 2); /* Handle to the collection categories found. */
   int num,                 /* Number of entities allocated by client and/or
                             * found by library in a find call. */
       tdim,                /* Topological dimension. */
       i;
   char name[1024],         /* Name of entity being described. */
        *p_name = &name[0];
   SAF_Role role;
   SAF_SilRole srole;
   SAF_ExtendMode extmode;
   SAF_TopMode topmode;
   SAF_CellType cell_type;
   SAF_DecompMode is_decomp;
   SAF_Set containing_set, range_s;
   SAF_Cat the_pieces, range_c, storage_decomp;
   SAF_RelRep trtype;
   hid_t data_type;
   size_t a_count, b_count;
   hid_t a_type, b_type;
   int *abuf = NULL,
       *bbuf = NULL,
       *nodes_per_ele,
       *conn;

   /* Set the number of nodes and elements in the mesh. */
   tri_edge_ct_y = edge_ct_y/2;
   quad_edge_ct_y = (edge_ct_y+1)/2;
   node_ct_x = edge_ct_x+1;
   node_ct_y = edge_ct_y+1;
   node_ct = node_ct_x*node_ct_y;
   ele_ct  = edge_ct_x*(tri_edge_ct_y * 2 + quad_edge_ct_y);

   /* Make the abuf. */
   nodes_per_ele = make_ele_node_ct_array();

   /* Make the mesh connectivity. */
   conn = make_mesh_connectivity();

   /*
    ---------------------------------------------------------------------------
    *                            FIND CATEGORIES
    ---------------------------------------------------------------------------
    */
   TESTING("finding node category");
   num = 0;
   saf_find_categories(SAF_ALL,db,SAF_UNIVERSE(db), "nodes", SAF_ANY_ROLE,
                       SAF_ANY_TOPODIM, &num, &nodes);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   TESTING("finding element category");
   num = 0;
   saf_find_categories(SAF_ALL,db,SAF_UNIVERSE(db), "elems", SAF_ANY_ROLE,
                       SAF_ANY_TOPODIM, &num, &elems);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                          DESCRIBE CATEGORIES
    ---------------------------------------------------------------------------
    */
   TESTING("describing node category");
   saf_describe_category(SAF_ALL,nodes, &p_name, &role, &tdim);
   if (strcmp(name, "nodes") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=0)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   TESTING("describing element category");
   saf_describe_category(SAF_ALL,elems, &p_name, &role, &tdim);
   if (strcmp(name, "elems") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=2)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                               FIND SETS
    ---------------------------------------------------------------------------
    */
   TESTING("finding matching sets");
   num = 0;
   saf_find_matching_sets(SAF_ALL, db, "TOP_CELL", SAF_SPACE, 2,
                          SAF_EXTENDIBLE_FALSE, SAF_TOP_TRUE, &num, &mesh);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                             DESCRIBE SETS
    ---------------------------------------------------------------------------
    */
   TESTING("describing sets");
   num = 2;
   saf_describe_set(SAF_ALL, mesh, &p_name, &tdim, &srole, &extmode, &topmode,
                    &num, &p_coll_cats);
   if (strcmp(name, "TOP_CELL") || tdim != 2 ||
       srole != SAF_SPACE || extmode != SAF_EXTENDIBLE_FALSE ||
       topmode != SAF_TOP_TRUE || num != 2 ||
       ! (SAF_EQUIV(&coll_cats[0], nodes) ||
            SAF_EQUIV(&coll_cats[1], nodes)) ||
       ! (SAF_EQUIV(&coll_cats[0], elems) ||
            SAF_EQUIV(&coll_cats[1], elems)))
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                           FIND COLLECTIONS
    ---------------------------------------------------------------------------
    */
   TESTING("finding collection of nodes in mesh");
   num = 2;
   saf_find_collections(SAF_ALL, mesh, SAF_TOPOLOGY, SAF_CELLTYPE_POINT, 0,
                        SAF_DECOMP_FALSE, &num, &p_coll_cats);
   if (num != 1 || ! SAF_EQUIV(&coll_cats[0], nodes))
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   TESTING("finding collections of elements in mesh");
   num = 2;
   saf_find_collections(SAF_ALL, mesh, SAF_TOPOLOGY, SAF_CELLTYPE_ARB,   2,
                        SAF_DECOMP_TRUE, &num, &p_coll_cats);
   if (num != 1 || ! SAF_EQUIV(&coll_cats[0], elems))
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                         DESCRIBE COLLECTIONS
    ---------------------------------------------------------------------------
    */
   TESTING("describing collection of nodes in mesh");
   saf_describe_collection(SAF_ALL, mesh, nodes, &cell_type, &num, NULL,
                           &is_decomp, NULL);
   if (cell_type != SAF_CELLTYPE_POINT || num != node_ct ||
       is_decomp != SAF_DECOMP_FALSE)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   TESTING("describing collection of elements in mesh");
   saf_describe_collection(SAF_ALL, mesh, elems, &cell_type, &num, NULL,
                           &is_decomp, NULL);
   if (cell_type != SAF_CELLTYPE_ARB || num != ele_ct || is_decomp != SAF_DECOMP_TRUE)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                        FIND TOPOLOGY RELATION
    ---------------------------------------------------------------------------
    */
   TESTING("finding topology relation");
   num = 0;
   saf_find_topo_relations(SAF_ALL,db, mesh, NULL, &num, &trels);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                       DESCRIBE TOPOLOGY RELATION
    ---------------------------------------------------------------------------
    */
   TESTING("describing topology relation");
   saf_describe_topo_relation(SAF_ALL, trels, &containing_set, &the_pieces,
                              &range_s, &range_c, &storage_decomp, &trtype,
                              &data_type);
   if (!SAF_EQUIV(&containing_set, mesh) ||
       !SAF_EQUIV(&the_pieces, elems) || !SAF_EQUIV(&range_s, mesh) ||
       !SAF_EQUIV(&range_c, nodes) ||
       !_saf_is_self_decomp(&storage_decomp) || !SAF_EQUIV(&trtype, SAF_ARBITRARY) ||
       !H5Tequal(data_type, H5T_NATIVE_INT))
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                       READING TOPOLOGY RELATION
    ---------------------------------------------------------------------------
    */
   TESTING("reading topology relation");
   saf_get_count_and_type_for_topo_relation(SAF_ALL, trels, NULL, NULL, &a_count, &a_type,
                                            &b_count, &b_type);
   if (!H5Tequal(a_type, H5T_NATIVE_INT) || !H5Tequal(b_type, H5T_NATIVE_INT) || a_count != (size_t)ele_ct ||
       b_count !=  (size_t)edge_ct_x*(tri_edge_ct_y*6+quad_edge_ct_y*4))
   {
      FAILED;
      failCount++;
   }
   else
   {
      /* The types and counts match so try to read the actual buffers of topo
       * relation data.
       */
      saf_read_topo_relation(SAF_ALL, trels, NULL, (void**)&abuf, (void**)&bbuf);
      if (abuf == NULL || bbuf == NULL)
      {
         FAILED;
         failCount++;
      }
      else
      {
         /* We got the buffers--verify that the contents are as expected. */
         for (i = 0; i < (int)a_count; i++)
            if (nodes_per_ele[i] != abuf[i])
               break;
         if (i < (int)a_count)
         {
            FAILED;
            failCount++;
         }
         else
         {
            /* The abuf is good so now check the bbuf. */
            for (i = 0; i < (int)b_count; i++)
               if (conn[i] != bbuf[i])
                  break;
            if (i < (int)b_count)
            {
               FAILED;
               failCount++;
            }
            else
               PASSED;
         }
      }
      if (abuf != NULL)
         free(abuf);
      if (bbuf != NULL)
         free(bbuf);
   }
   if (trels != NULL)
      free(trels);
   free(conn);
   free(nodes_per_ele);

   return;
}

static double *
make_coord_field_dofs(void)
{
   /* Creates the coordinate field dofs for a rectangular mesh of quads and
    * triangles.
    *
    * Number of nodes in x-dir = edge_ct_x+1 and
    * similarly for y-dir. */

   int i, j, node_id;
   double delx = 1.0/edge_ct_x,         /* X increment between nodes in x. */
          dely = 1.0/edge_ct_y,         /* Y increment between nodes in y. */
          *dofs,                        /* The array of dofs. */
          x, y;                         /* x and y coordinates. */

   /* Preconditions: */

   /* Body: */

   /* allocate dofs as single index array,
    * but treat as dofs[nx+1][ny+1][2]; */
   dofs = (double*)malloc(node_ct*2*sizeof(double));

   for(i = 0; i < node_ct_x; i++)
   {
      x = i*delx; 
      for(j = 0; j < node_ct_y; j++)
      {
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

static void
read_coord_field(void)
{
   /* Read the coordinate field for the mesh. */

   SAF_FieldTmpl *coords_ftmpl = NULL, /* Handle to the coordinate field field
                                        * template. */
                 *coords_ctmpl = NULL, /* Handle to the coordinate field's */
                                       /* components' field templates. */
                 described_ftmpl;
   SAF_Field *coords = NULL,           /* Handle to the coordinate field. */
             *coord_x_comp = NULL,     /* Handle to the coordinate field's */
             *coord_y_comp = NULL,     /* x and y components. */
             *described_coords_comps = NULL;
   double *lcoord_dof_tuple,           /* The coordinate field dofs. */
          *dofs_read = NULL;           /* Dofs read from the file. */
   int num, i;
   char name[1024],
        *p_name = &name[0];
   SAF_Set base;
   SAF_Algebraic atype;
   SAF_Basis basis;
   SAF_Quantity quant;
   SAF_Unit units;
   hbool_t is_coord;
   SAF_Cat storage_decomp,
           coeff_assoc,
           eval_coll;
   SAF_Eval eval_func;
   hid_t dof_type;
   int num_comps;
   SAF_Interleave comp_intlv;
   int comp_order[2],
       *p_comp_order = &comp_order[0];
   size_t count;

   /* Create the coordinate field dofs. */
   lcoord_dof_tuple = make_coord_field_dofs();

   /*
    ---------------------------------------------------------------------------
    *                         FIND FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   TESTING("finding global coordinate component field template");
   num = 0;
   saf_find_field_tmpls(SAF_ALL, db, "e2_on_mesh_ctmpl", SAF_ALGTYPE_SCALAR,
                        SAF_UNITY, SAF_ANY_QUANTITY, &num, &coords_ctmpl);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   TESTING("finding global coordinate composite field template");
   num = 0;
   saf_find_field_tmpls(SAF_ALL, db, "e2_on_mesh_tmpl", SAF_ALGTYPE_VECTOR,
                        SAF_CARTESIAN, SAF_ANY_QUANTITY, &num, &coords_ftmpl);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                       DESCRIBE FIELD TEMPLATES
    ---------------------------------------------------------------------------
    */
   TESTING("describing global coordinate component field template");
   num = 0;
   saf_describe_field_tmpl(SAF_ALL, coords_ctmpl, &p_name, /* &base,*/ &atype,
                           &basis, &quant, &num, NULL);
   /*
    *  Quant should reference one of the standard quantity definitions.
    *  However, the standard definitions are not saved with a database when it
    *  is written.  Therefore, it is not possible to SAF_EQUIV(quant, proper
    *  standard quantity) here to verify correctness.  When the problem with
    *  saving the standard definitions is resolved, check that the quantity
    *  returned by this function is as expected.
    */
   if (strcmp(name, "e2_on_mesh_ctmpl") ||
       !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) || !SAF_EQUIV(&basis, SAF_UNITY) || num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   TESTING("describing global coordinate composite field template");
   num = 0;
   saf_describe_field_tmpl(SAF_ALL, coords_ftmpl, &p_name, /* &base, */ &atype,
                           &basis, &quant, &num, NULL);
   /*
    *  Quant should reference one of the standard quantity definitions.
    *  However, the standard definitions are not saved with a database when it
    *  is written.  Therefore, it is not possible to SAF_EQUIV(quant, proper
    *  standard quantity) here to verify correctness.  When the problem with
    *  saving the standard definitions is resolved, check that the quantity
    *  returned by this function is as expected.
    */
   if (strcmp(name, "e2_on_mesh_tmpl") || 
       !SAF_EQUIV(&atype, SAF_ALGTYPE_VECTOR) || !SAF_EQUIV(&basis, SAF_CARTESIAN) || num != 2)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                              FIND FIELD
    ---------------------------------------------------------------------------
    */
   TESTING("finding global coordinate component fields");
   num = 0;
   saf_find_fields(SAF_ALL,db, mesh, "X", SAF_ANY_QUANTITY, SAF_ALGTYPE_SCALAR,
                   SAF_UNITY, SAF_ANY_UNIT, SAF_NODAL(nodes, elems),
                   &num, &coord_x_comp);

   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
   {
      num = 0;
      saf_find_fields(SAF_ALL,db, mesh, "Y", SAF_ANY_QUANTITY, SAF_ALGTYPE_SCALAR,
                      SAF_UNITY, SAF_ANY_UNIT, SAF_NODAL(nodes, elems),
                      &num, &coord_y_comp);
      if (num != 1)
      {
         FAILED;
         failCount++;
      }
      else
         PASSED;
   }

   TESTING("finding global coordinate field");
   num = 0;
   saf_find_fields(SAF_ALL,db, mesh, "coord field", SAF_ANY_QUANTITY, SAF_ALGTYPE_VECTOR,
                   SAF_CARTESIAN, SAF_ANY_UNIT, SAF_NODAL(nodes, elems),
                   &num, &coords);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                            DESCRIBE FIELD
    ---------------------------------------------------------------------------
    */
   TESTING("describing global coordinate X component field");
   saf_describe_field(SAF_ALL, coord_x_comp, &described_ftmpl, &p_name, &base, 
                      &units, &is_coord, &storage_decomp, &coeff_assoc, &num,
                      &eval_coll, &eval_func, &dof_type, &num_comps,
                      &described_coords_comps, &comp_intlv, NULL);
   if (!SAF_EQUIV(&described_ftmpl, coords_ctmpl) ||
       strcmp(p_name, "X") || is_coord ||
       !_saf_is_self_decomp(&storage_decomp) ||
       !SAF_EQUIV(&coeff_assoc, nodes) || num != 1 ||
       !SAF_EQUIV(&eval_coll, elems) || !SAF_EQUIV(&eval_func, SAF_SPACE_PWLINEAR) ||
       dof_type != H5I_INVALID_HID || num_comps != 1 ||
       described_coords_comps != NULL || comp_intlv != SAF_INTERLEAVE_NONE
       )
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;
   if (described_coords_comps != NULL)
      free(described_coords_comps);
   if (coords_ctmpl != NULL)
      free(coords_ctmpl);

   TESTING("describing global coordinate field");
   num_comps = 2; /* Needed to be set to tell how many entries are in the
                   * pre-allocated comp_order array. */
   saf_describe_field(SAF_ALL, coords,       &described_ftmpl, &p_name,&base,
                      &units, &is_coord, &storage_decomp, &coeff_assoc, &num,
                      &eval_coll, &eval_func, &dof_type, &num_comps,
                      &described_coords_comps, &comp_intlv, &p_comp_order);
   if (!SAF_EQUIV(&described_ftmpl, coords_ftmpl) ||
       strcmp(p_name, "coord field") || !is_coord ||
       !_saf_is_self_decomp(&storage_decomp) ||
       !SAF_EQUIV(&coeff_assoc, nodes) || num != 1 ||
       !SAF_EQUIV(&eval_coll, elems) || !SAF_EQUIV(&eval_func, SAF_SPACE_PWLINEAR) ||
       !H5Tequal(dof_type, H5T_NATIVE_DOUBLE) || num_comps != 2 ||
       !SAF_EQUIV(&described_coords_comps[comp_order[0]], coord_x_comp) ||
       !SAF_EQUIV(&described_coords_comps[comp_order[1]], coord_y_comp) ||
       comp_intlv != SAF_INTERLEAVE_VECTOR )
   {
   if (!SAF_EQUIV(&described_ftmpl, coords_ftmpl) ) printf("check1 ftmpl\n");
   if ( !SAF_EQUIV(&described_coords_comps[p_comp_order[0]], coord_x_comp))printf("check2 coord_x_comp \n");
   if ( !SAF_EQUIV(&described_coords_comps[p_comp_order[1]], coord_y_comp) )printf("check3 coord_y_comp\n" );

      FAILED;
      failCount++;
   }
   else
      PASSED;
   if (coords_ftmpl != NULL)
      free(coords_ftmpl);
   if (coord_x_comp != NULL)
      free(coord_x_comp);
   if (coord_y_comp != NULL)
      free(coord_y_comp);
   if (described_coords_comps != NULL)
      free(described_coords_comps);

   /*
    ---------------------------------------------------------------------------
    *                              READ FIELD
    ---------------------------------------------------------------------------
    */
   TESTING("reading coordinate field");
   saf_get_count_and_type_for_field(SAF_ALL, coords, NULL, &count, &dof_type);
   if (count != (size_t)node_ct*2 || !H5Tequal(dof_type, H5T_NATIVE_DOUBLE))
   {
      FAILED;
      failCount++;
   }
   else
   {
      saf_read_field(SAF_ALL, coords, NULL, SAF_WHOLE_FIELD, (void**)&dofs_read);
      for (i = 0; i < (int)count; i++)
	if (lcoord_dof_tuple[i] != dofs_read[i])
           break;
      if (i < (int)count)
      {
         FAILED;
         failCount++;
      }
      else
        PASSED;
   }
   if (coords != NULL)
      free(coords);

   /* Free the dofs now that we are done with them. */
   free(lcoord_dof_tuple);
   free(dofs_read);

   return;
}

static double *
make_scalar_field_dofs(void)
{
   /* Creates a scalar field dofs on a rectangular mesh of quads and triangles.
    *
    * Number of nodes in x-dir = edge_ct_x+1 and
    * similarly for y-dir. */

   int i, j, node_id;
   double delx = 1.0/edge_ct_x,       /* X increment between nodes in x. */
          dely = 1.0/edge_ct_y,       /* Y increment between nodes in y. */
          *dofs,                      /* The array of dofs. */
          x, y;                       /* Used to compute dofs. */

   /* Preconditions: */

   /* Body: */

   /* allocate dofs as single index array,
    * but treat as dofs[nx+1][ny+1][2]; */
   dofs = (double*)malloc(node_ct*sizeof(double));

   for(i = 0; i < node_ct_x; i++)
   {
      x = i*delx; 
      for(j = 0; j < node_ct_y; j++)
      {
         y = j*dely; 
         node_id = NODE_ID(i,j);
         dofs[node_id] = x*x + y*y;
      }
   }

   /* Postconditions: */

   /* Exit */

   return dofs;
}

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
   char name[1024],
        *p_name = &name[0];
   SAF_Algebraic atype;
   SAF_Basis basis;
   SAF_Quantity quant;
   hid_t dof_type;
   size_t count;

   /* Create the scalar field dofs. */
   lscalar_dof_tuple = make_scalar_field_dofs();

   /*
    ---------------------------------------------------------------------------
    *                         FIND FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   TESTING("finding scalar field template");
   num = 0;
   saf_find_field_tmpls(SAF_ALL, db, "at0_on_mesh_tmpl", SAF_ALGTYPE_SCALAR,
                        SAF_UNITY, SAF_ANY_QUANTITY, &num, &scalar_ftmpl);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                       DESCRIBE FIELD TEMPLATES
    ---------------------------------------------------------------------------
    */
   TESTING("describing scalar field template");
   num = 0;
   saf_describe_field_tmpl(SAF_ALL, scalar_ftmpl, &p_name, &atype,
                           &basis, &quant, &num, NULL);
   /*
    *  Quant should reference one of the standard quantity definitions.
    *  However, the standard definitions are not saved with a database when it
    *  is written.  Therefore, it is not possible to SAF_EQUIV(quant, proper
    *  standard quantity) here to verify correctness.  When the problem with
    *  saving the standard definitions is resolved, check that the quantity
    *  returned by this function is as expected.
    */
   if (strcmp(name, "at0_on_mesh_tmpl") || 
       !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) || !SAF_EQUIV(&basis, SAF_UNITY) || num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;
   if (scalar_ftmpl != NULL)
      free(scalar_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                              FIND FIELD
    ---------------------------------------------------------------------------
    */
   TESTING("finding scalar field");
   num = 0;
   saf_find_fields(SAF_ALL,db, mesh, "scalar field", SAF_ANY_QUANTITY,
                   SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_ANY_UNIT,
                   SAF_NODAL(nodes, elems), &num, &scalar);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                              READ FIELD
    ---------------------------------------------------------------------------
    */
   TESTING("reading scalar field");
   saf_get_count_and_type_for_field(SAF_ALL, scalar, NULL, &count, &dof_type);
   if (count != (size_t)node_ct || !H5Tequal(dof_type, H5T_NATIVE_DOUBLE)) 
   {
      FAILED;
      failCount++;
   }
   else
   {
      saf_read_field(SAF_ALL, scalar, NULL, SAF_WHOLE_FIELD, (void**)&dofs_read);
      for (i = 0; i < (int)count; i++)
	if (lscalar_dof_tuple[i] != dofs_read[i])
           break;
      if (i < (int)count)
      {
         FAILED;
         failCount++;
      }
      else
        PASSED;
   }
   if (scalar != NULL)
      free(scalar);

   /* Free the dofs now that we are done with them. */
   free(lscalar_dof_tuple);
   free(dofs_read);

   return;
}

static double *
make_stress_field_dofs(void)
{
   /* Creates a "stress" field on a rectangular array of triangles and quads.
    * Used to test instantiation of st2 field; values of field are meaningless.
    *
    * Number of nodes in x-dir = edge_ct_x+1 and
    * similarly for y-dir. */

   int ele_idx = 0,                     /* Element counter. */
       i, j;
   double *dofs;                        /* The array of dofs. */

   /* Preconditions: */

   /* Body: */

   /* allocate dofs as single index array,
    * but treat as dofs[edge_ct_x][edge_ct_y][3]; */
   dofs = (double*)malloc(ele_ct*3*sizeof(double));

   for(i=0; i<edge_ct_x; i++)
   {
      for(j=0; j<tri_edge_ct_y; j++)
      {
         /* Each (i,j) pair is the lower-left-hand node in a quad
          * containing two triangles.
          */

         /* Dofs for lower triangle in quad. */
         dofs[3*ele_idx]   = ele_idx+0.0; /* xx component */
         dofs[3*ele_idx+1] = ele_idx+0.1; /* xy component */
         dofs[3*ele_idx+2] = ele_idx+0.2; /* yy component */

         ele_idx++;

         /* Dofs for upper triangle in quad. */
         dofs[3*ele_idx]   = ele_idx+0.0; /* xx component */
         dofs[3*ele_idx+1] = ele_idx+0.1; /* xy component */
         dofs[3*ele_idx+2] = ele_idx+0.2; /* yy component */

         ele_idx++;
      }
      for(j=0; j<quad_edge_ct_y; j++)
      {
         /* Each (i,j) pair is the lower-left-hand node in a quad */
         dofs[3*ele_idx]   = ele_idx+0.0; /* xx component */
         dofs[3*ele_idx+1] = ele_idx+0.1; /* xy component */
         dofs[3*ele_idx+2] = ele_idx+0.2; /* yy component */

         ele_idx++;
      }
   }

   /* Postconditions: */

   /* Exit */

   return dofs;
}

static void
read_stress_field(void)
{
   /* Read the stress field defined on the mesh. */

   SAF_FieldTmpl *stress_ftmpl = NULL, /* Handle to the stress field field
                                        * template. */
                 *stress_ctmpl = NULL, /* Handle to the stress field's
                                        * components' field templates. */
                 described_ftmpl;
   SAF_Field *stress = NULL;           /* Handle to the stress field. */
   double *lstress_dof_tuple,          /* The stress field dofs. */
          *dofs_read = NULL;           /* Dofs read from the file. */
   int num, i;
   char name[1024],
        *p_name = &name[0];
   SAF_Set base;
   SAF_Algebraic atype;
   SAF_Basis basis;
   SAF_Quantity quant;
   SAF_Unit units;
   hbool_t is_coord;
   SAF_Cat storage_decomp,
           coeff_assoc,
           eval_coll;
   SAF_Eval eval_func;
   hid_t dof_type;
   int num_comps;
   SAF_Interleave comp_intlv;
   int comp_order[3],
       *p_comp_order = &comp_order[0];
   size_t count;

   /* Create the stress field dofs. */
   lstress_dof_tuple = make_stress_field_dofs();

   /*
    ---------------------------------------------------------------------------
    *                         FIND FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   TESTING("finding stress component field template");
   num = 0;
   saf_find_field_tmpls(SAF_ALL, db, "st2_e2_on_mesh_ctmpl",
                        SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_ANY_QUANTITY, &num,
                        &stress_ctmpl);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   TESTING("finding stress composite field template");
   num = 0;
   saf_find_field_tmpls(SAF_ALL, db, "st2_e2_on_mesh_tmpl",
                        SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI, SAF_ANY_QUANTITY,
                        &num, &stress_ftmpl);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                       DESCRIBE FIELD TEMPLATES
    ---------------------------------------------------------------------------
    */
   TESTING("describing stress component field template");
   num = 0;
   saf_describe_field_tmpl(SAF_ALL, stress_ctmpl, &p_name, &atype,
                           &basis, &quant, &num, NULL);
   /*
    *  Quant should reference one of the standard quantity definitions.
    *  However, the standard definitions are not saved with a database when it
    *  is written.  Therefore, it is not possible to SAF_EQUIV(quant, proper
    *  standard quantity) here to verify correctness.  When the problem with
    *  saving the standard definitions is resolved, check that the quantity
    *  returned by this function is as expected.
    */
   if (strcmp(name, "st2_e2_on_mesh_ctmpl") ||
       !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) ||
       !SAF_EQUIV(&basis, SAF_UNITY) || num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   TESTING("describing stress composite field template");
   num = 0;
   saf_describe_field_tmpl(SAF_ALL, stress_ftmpl, &p_name, &atype,
                           &basis, &quant, &num, NULL);
   /*
    *  Quant should reference one of the standard quantity definitions.
    *  However, the standard definitions are not saved with a database when it
    *  is written.  Therefore, it is not possible to SAF_EQUIV(quant, proper
    *  standard quantity) here to verify correctness.  When the problem with
    *  saving the standard definitions is resolved, check that the quantity
    *  returned by this function is as expected.
    */
   if (strcmp(name, "st2_e2_on_mesh_tmpl") ||
       !SAF_EQUIV(&atype, SAF_ALGTYPE_SYMTENSOR) ||
       !SAF_EQUIV(&basis, SAF_UPPERTRI) || num != 3)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;
   if (stress_ctmpl != NULL)
      free(stress_ctmpl);

   /*
    ---------------------------------------------------------------------------
    *                              FIND FIELD
    ---------------------------------------------------------------------------
    */
   TESTING("finding stress field");
   num = 0;
   saf_find_fields(SAF_ALL,db, mesh, "stress tensor", SAF_ANY_QUANTITY,
                   SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI, SAF_ANY_UNIT,
                   SAF_ZONAL(elems), &num, &stress);
   if (num != 1)
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;

   /*
    ---------------------------------------------------------------------------
    *                            DESCRIBE FIELD
    ---------------------------------------------------------------------------
    */
   TESTING("describing stress field");
   num_comps = 3; /* Needed to be set to tell how many entries are in the
                   * pre-allocated comp_order array. */
   saf_describe_field(SAF_ALL, stress,       &described_ftmpl, &p_name, &base, 
                      &units, &is_coord, &storage_decomp, &coeff_assoc, &num,
                      &eval_coll, &eval_func, &dof_type, &num_comps,
                      NULL, &comp_intlv, &p_comp_order);
   if (!SAF_EQUIV(&described_ftmpl, stress_ftmpl) ||
       strcmp(p_name, "stress tensor") || is_coord ||
       !_saf_is_self_decomp(&storage_decomp) ||
       !SAF_EQUIV(&coeff_assoc, elems) || num != 1 ||
       !SAF_EQUIV(&eval_coll, elems) || !SAF_EQUIV(&eval_func, SAF_SPACE_PWCONST) ||
       !H5Tequal(dof_type, H5T_NATIVE_DOUBLE) || num_comps != 3 ||
       comp_order[0] != 0 || comp_order[1] != 1 || comp_order[2] != 2 ||
       comp_intlv != SAF_INTERLEAVE_VECTOR )
   {
      FAILED;
      failCount++;
   }
   else
      PASSED;
   if (stress_ftmpl != NULL)
      free(stress_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                              READ FIELD
    ---------------------------------------------------------------------------
    */
   TESTING("reading stress field");
   saf_get_count_and_type_for_field(SAF_ALL, stress, NULL, &count, &dof_type);
   if (count != (size_t)ele_ct*3 || !H5Tequal(dof_type, H5T_NATIVE_DOUBLE))
   {
      FAILED;
      failCount++;
   }
   else
   {
      saf_read_field(SAF_ALL, stress, NULL, SAF_WHOLE_FIELD, (void**)&dofs_read);
      for (i = 0; i < (int)count; i++)
	if (lstress_dof_tuple[i] != dofs_read[i])
           break;
      if (i < (int)count)
      {
         FAILED;
         failCount++;
      }
      else
        PASSED;
   }
   if (stress != NULL)
      free(stress);

   /* Free the dofs now that we are done with them. */
   free(lstress_dof_tuple);
   free(dofs_read);

   return;
}

int main(int argc, char **argv)
{
   char dbname[1024];        /* Name of the SAF database file to be created. */
   int rank=0;               /* Rank of this process for parallel. */
   SAF_DbProps *dbprops=NULL;      /* Handle to the SAF databsae properties. */
   SAF_LibProps *libprops;    /* Handle to the SAF library properties. */

      hbool_t do_interactive = false;

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

      if (rank==0 && do_interactive)
         /* Prompt user for number of edges in x and y. */
         read_user_input();
      else
      {
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

   /* Set the string mode library property to SAF_STRMODE_CLIENT. */
   libprops = saf_createProps_lib();
   saf_setProps_StrMode(libprops, SAF_STRMODE_CLIENT);

   /* Initialize the library. */
   saf_init(libprops);

   /* Get the name of the SAF database. */
   strcpy(dbname, TEST_FILE_NAME);

   SAF_TRY_BEGIN
   {
      /* NOTE: because we are in a try block here, all failures will send us to
       * the one and only catch block at the end of this test */

      /* Create the database properties. */
      dbprops = saf_createProps_database();

      /* Set the clobber database property so any existing file
       * will be overwritten. */
      saf_setProps_ReadOnly(dbprops);

      /* Open the SAF database. Give it name dbname, properties p and
       * set db to be a handle to this database. */
      TESTING("database open");
      db = saf_open_database(dbname,dbprops);
      PASSED;

      /* SAF on SSlib no longer has a notion of supplemental files. However, each database (actually each scope) does keep
       * track of which other files are referenced by interfile object links in that database (or scope). In order to get to
       * that information we have to talk to SSlib directly since SAF has no interface yet by which that information can be
       * obtained. [rpm 2004-08-18] */
      TESTING("saf_find_files");
      {
          size_t nfound=SS_NOSIZE;
          ss_fileobj_t file_mask;
          ss_file_t *file_key=(ss_file_t*)ss_pers_new(_SAF_GLOBALS.local_scope, SS_MAGIC(ss_file_t), NULL, SS_ALLSAME, NULL, NULL);
          ss_scope_t *topscope=ss_pers_topscope((ss_pers_t*)db, NULL);
          memset(&file_mask, 0, sizeof file_mask);

          /* Search for a specific nonexistent name */
          ss_string_set(SS_FILE_P(file_key,name), "foobar");
          memset(&file_mask.name, SS_VAL_CMP_DFLT, 1);
          ss_pers_find(topscope, (ss_pers_t*)file_key, (ss_persobj_t*)&file_mask, 0, &nfound, SS_PERS_TEST, NULL);
          if (0!=nfound) {
              FAILED;
              failCount++;
          } else {

#ifdef WIN32 
                  PASSED;
#else
              /* Search for a pattern of names. The pattern is such that the database itself is matched. */
              ss_string_set(SS_FILE_P(file_key,name), "saf$");
              memset(&file_mask.name, SS_VAL_CMP_RE, 1);
              nfound = SS_NOSIZE; /*do not limit the search*/
              ss_pers_find(topscope, (ss_pers_t*)file_key, (ss_persobj_t*)&file_mask, 0, &nfound, SS_PERS_TEST, NULL);
              if (1!=nfound) {
                  FAILED;
                  failCount++;
              } else {
                  PASSED;
              }
#endif

          }
      }

      /* Read the base space with edge_ct_x edges in the x direction and
       * edge_ct_y edges in the y  direction in file associated with
       * database db.  Set mesh to mesh set, nodes to the category of nodes in
       * the mesh and elems to the category of elements in the mesh. */
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

   /* Finalize access to the library. */
   saf_final();

#ifdef HAVE_PARALLEL
   /* make sure everyone returns the same error status */
   MPI_Bcast(&failCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Finalize();
#endif

   return failCount == 0 ? 0 : 1;
}
