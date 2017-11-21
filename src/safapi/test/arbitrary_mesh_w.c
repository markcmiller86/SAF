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

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests
 * Purpose:    	Test of creation of an arbitrary mesh and fields defined
 *              on it.
 *
 * Description: This test demonstrates the creation of a 2D mesh made up of
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
 * Programmer:	William J. Arrighi, LLNL, Tuesday Jan 23, 2001
 *
 *-----------------------------------------------------------------------------
 */
SAF_Db *db=NULL;                /* Handle to the SAF database. */
SAF_Cat nodes, elems;     /* Handles to the node and elements categories. */
SAF_Set mesh;             /* Handle to the mesh set. */
SAF_Db *saf_file=NULL;    /* Handle to the saf file. */
int edge_ct_x, edge_ct_y, /* The number of edges in the x and y directions. */
    node_ct_x, node_ct_y, /* The number of nodes in the x and y directions. */
    tri_edge_ct_y,        /* Number of triangle edges in the x direction. */
    quad_edge_ct_y,       /* Number of quad edges in the y direction. */
    node_ct,              /* Total number of nodes in the mesh. */
    ele_ct;               /* Total number of elements in the mesh. */

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

   while (edge_ct_y < 2)
   {
      printf("Enter the number of elements in the y direction: ");
      scanf("%i", &edge_ct_y);
      if (edge_ct_y < 2)
         printf("\n***There must be at least 2 elements in the y direction.***\n\n");
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
make_base_space(void)
{
   /* Constructs the mesh and writes it to saf_file. */

   SAF_Rel trel;  /* Handle to the topological relation. */
   int *abuf,     /* Number of nodes for each element in the mesh. */
       *conn;     /* Node to element connectivity data. */

   /* Set the number of nodes and elements in the mesh. */
   tri_edge_ct_y = edge_ct_y/2;
   quad_edge_ct_y = (edge_ct_y+1)/2;
   node_ct_x = edge_ct_x+1;
   node_ct_y = edge_ct_y+1;
   node_ct = node_ct_x*node_ct_y;
   ele_ct  = edge_ct_x*(tri_edge_ct_y * 2 + quad_edge_ct_y);

   /* Make the abuf. */
   abuf = make_ele_node_ct_array();

   /* Make the connectivity. */
   conn = make_mesh_connectivity();

   /*
    ---------------------------------------------------------------------------
    *                            DECLARE CATEGORIES
    ---------------------------------------------------------------------------
    */
   TESTING("declaring categories");
   saf_declare_category(SAF_ALL,db, "nodes",  SAF_TOPOLOGY, 0, &nodes);
   saf_declare_category(SAF_ALL,db, "elems",  SAF_TOPOLOGY, 2, &elems);
   PASSED;

   /*
    ---------------------------------------------------------------------------
    *                               DECLARE SET
    ---------------------------------------------------------------------------
    */
   TESTING("declaring sets");
   saf_declare_set(SAF_ALL, db, "TOP_CELL", 2, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &mesh);
   PASSED;

   /*
    ---------------------------------------------------------------------------
    *                           DECLARE COLLECTIONS
    ---------------------------------------------------------------------------
    */
   TESTING("declaring collections");
   saf_declare_collection(SAF_ALL, &mesh, &nodes,  SAF_CELLTYPE_POINT, node_ct,
                          SAF_1DC(node_ct), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &mesh, &elems,  SAF_CELLTYPE_ARB,   ele_ct,
                          SAF_1DC(ele_ct),  SAF_DECOMP_TRUE);
   PASSED;

   /*
    ---------------------------------------------------------------------------
    *                   DECLARE AND WRITE TOPOLOGY RELATIONS
    ---------------------------------------------------------------------------
    */
   TESTING("declaring and writing topology relations");
   saf_declare_topo_relation(SAF_ALL,db, &mesh, &elems, &mesh, &nodes,
                             SAF_SELF(db), &mesh, SAF_ARBITRARY, H5I_INVALID_HID,
                             NULL, H5I_INVALID_HID,NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT,
                           conn, saf_file);
   PASSED;

   /* Free the connectivity data now that we are done with it. */
   free(abuf);
   free(conn);

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
   double delx = 1.0/edge_ct_x, /* X increment between nodes in x. */
          dely = 1.0/edge_ct_y, /* Y increment between nodes in y. */
          *dofs,                /* The array of dofs. */
          x, y;                 /* x and y coordinates. */

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
make_coord_field(void)
{
   /* Construct the coordinate field on the mesh. */

   SAF_FieldTmpl coords_ftmpl, /* Handle to the coordinate field field
                                * template. */
                 coords_ctmpl, /* Handle to the coordinate field's components'
                                * field templates. */
		 tmp_ftmpl[3]; /* temporary field template handle for
				* component field templates. */
   SAF_Field coords,           /* Handle to the coordinate field. */
             coord_compon[2];  /* Handle to the 2 components of the
                                * coordinate field. */
   SAF_Unit umeter;            /* Handle to the units for the coordinates. */
   double *lcoord_dof_tuple;   /* The coordinate field dofs. */
   int comp_order[2] = {1, 0}; /* Array to unscramble the ordering of the
                                * components of the global coordinates. */

   /* Create the coordinate field dofs. */
   lcoord_dof_tuple = make_coord_field_dofs();

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   TESTING("declaring global coordinate field template");
   saf_declare_field_tmpl(SAF_ALL,db, "e2_on_mesh_ctmpl",
                          SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QLENGTH, 1,
                          NULL, &coords_ctmpl);

   tmp_ftmpl[0] = coords_ctmpl;
   tmp_ftmpl[1] = coords_ctmpl;
   saf_declare_field_tmpl(SAF_ALL,db, "e2_on_mesh_tmpl",
                          SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 2,
                          tmp_ftmpl, &coords_ftmpl);
   PASSED;

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                       (buf specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "meter",&umeter);

   /* Declare the fields. */
   TESTING("declaring and writing global coordinate field");
   /*
    * Test the comp_order parameter by declaring the 1st component to be "Y"
    * and the 2nd component to be "X" and using {1, 0} as the comp_order
    * argument in the declaration of the composite coordinate field.
    */
   saf_declare_field(SAF_ALL,db, &coords_ctmpl, "X",           &mesh, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_DOUBLE,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     coord_compon+1);
   saf_declare_field(SAF_ALL,db, &coords_ctmpl, "Y",           &mesh, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_DOUBLE,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     coord_compon);
   saf_declare_field(SAF_ALL,db, &coords_ftmpl, "coord field", &mesh, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_DOUBLE,
                     coord_compon, SAF_INTERLEAVE_VECTOR, comp_order, NULL,
                     &coords);

   /* Write the coordinate field. */
   saf_write_field(SAF_ALL, &coords, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID, (void**)&lcoord_dof_tuple, saf_file);

   /* Specify that is a coordinate field */
   saf_declare_coords(SAF_ALL, &coords);
   PASSED;

   /* Free the dofs now that we are done with them. */
   free(lcoord_dof_tuple);

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
make_scalar_field(void)
{
   /* Construct the scalar field on the mesh. */

   SAF_FieldTmpl scalar_ftmpl; /* Handle to the field template. */
   SAF_Field scalar;           /* Handle to the field. */
   SAF_Unit umeter;            /* Handle to the units of the field. */
   double *lscalar_dof_tuple;  /* The scalar field dofs. */

   /* Create the scalar field dofs. */
   lscalar_dof_tuple = make_scalar_field_dofs();

   /*
    ---------------------------------------------------------------------------
    *                          DECLARE FIELD TEMPLATE
    ---------------------------------------------------------------------------
    */
   TESTING("declaring scalar field template");
   saf_declare_field_tmpl(SAF_ALL,db, "at0_on_mesh_tmpl",
                           SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QLENGTH, 1,
                           NULL, &scalar_ftmpl);
   PASSED;

   /*
    ---------------------------------------------------------------------------
    *                          DECLARE AND WRITE FIELD
    *                       (buf specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Get the units for the field. */
   saf_find_one_unit(db, "meter",&umeter);

   /* Declare the field. */
   TESTING("declaring and writing scalar field");
   saf_declare_field(SAF_ALL,db, &scalar_ftmpl, "scalar field", &mesh, &umeter,
                     SAF_SELF(db), SAF_NODAL(&nodes, &elems), H5T_NATIVE_DOUBLE,
                     NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &scalar);

   /* Write the field. */
   saf_write_field(SAF_ALL, &scalar, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID, (void**)&lscalar_dof_tuple, saf_file);
   PASSED;

   /* Free the dofs now that we are done with them. */
   free(lscalar_dof_tuple);

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

   int elmt_idx = 0, /* Element counter. */
       i, j;
   double *dofs;     /* The array of dofs. */

   /* Preconditions: */

   /* Body: */

   /* allocate dofs as single index array,
    * but treat as dofs[edge_ct_x][edge_ct_y][3]; */
   dofs = (double*)malloc(ele_ct*3*sizeof(double));

   for(i=0; i<edge_ct_x; i++)
   {
      for(j=0; j<tri_edge_ct_y; j++)
      {
         /* Each (i, j) pair is the lower-left-hand node in a quad
          * containing two triangles.
          */

         /* Dofs for lower triangle in quad. */
         dofs[3*elmt_idx]   = elmt_idx+0.0; /* xx component */
         dofs[3*elmt_idx+1] = elmt_idx+0.1; /* xy component */
         dofs[3*elmt_idx+2] = elmt_idx+0.2; /* yy component */

         elmt_idx++;

         /* Dofs for upper triangle in quad. */
         dofs[3*elmt_idx]   = elmt_idx+0.0; /* xx component */
         dofs[3*elmt_idx+1] = elmt_idx+0.1; /* xy component */
         dofs[3*elmt_idx+2] = elmt_idx+0.2; /* yy component */

         elmt_idx++;
      }
      for(j=0; j<quad_edge_ct_y; j++)
      {
         /* Each (i,j) pair is the lower-left-hand node in a quad */
         dofs[3*elmt_idx]   = elmt_idx+0.0; /* xx component */
         dofs[3*elmt_idx+1] = elmt_idx+0.1; /* xy component */
         dofs[3*elmt_idx+2] = elmt_idx+0.2; /* yy component */

         elmt_idx++;
      }
   }

   /* Postconditions: */

   /* Exit */

   return dofs;
}

static void
make_stress_field(void)
{
   /* Construct the stress field on the mesh. */

   SAF_FieldTmpl stress_ftmpl, /* Handle to the stress field template. */
                 stress_ctmpl, /* Handle to the stress field's components'
                                * field templates. */
		 tmp_ftmpl[3]; /* temporary field template handles for
				* component field templates. */
   SAF_Field stress,           /* Handle to the stress field. */
             stress_compon[3]; /* Handle to the stress field's components. */
   SAF_Unit upascal;           /* Handle to the units for this field. */
   double *lstress_dof_tuple;  /* The stress field dofs. */

   /* Create the stress field dofs. */
   lstress_dof_tuple = make_stress_field_dofs();

   /*
    ---------------------------------------------------------------------------
    *                          DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   TESTING("declaring stress field template");
   saf_declare_field_tmpl(SAF_ALL,db, "st2_e2_on_mesh_ctmpl",
                          SAF_ALGTYPE_SCALAR, SAF_UNITY,
                          SAF_QNAME(db,"pressure"), 1, NULL,
			  &stress_ctmpl);

   tmp_ftmpl[0] = stress_ctmpl;
   tmp_ftmpl[1] = stress_ctmpl;
   tmp_ftmpl[2] = stress_ctmpl;
   saf_declare_field_tmpl(SAF_ALL,db, "st2_e2_on_mesh_tmpl",
                          SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI,
                          SAF_QNAME(db,"pressure"), 3, tmp_ftmpl,
                          &stress_ftmpl);
   PASSED;

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *			   (buf specified in declare call)
    ---------------------------------------------------------------------------
    */
   /* Get the units for the field. */
   saf_find_one_unit(db, "pascal",&upascal);

   /* Declare the fields. */
   TESTING("declaring and writing stress field");
   saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxx",           &mesh, &upascal,
                     SAF_SELF(db), SAF_ZONAL(&elems), H5T_NATIVE_DOUBLE,
                     NULL,          SAF_INTERLEAVE_NONE,   SAF_IDENTITY,
                     NULL,                        stress_compon);
   saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Syy",           &mesh, &upascal,
                     SAF_SELF(db), SAF_ZONAL(&elems), H5T_NATIVE_DOUBLE,
                     NULL,          SAF_INTERLEAVE_NONE,   SAF_IDENTITY,
                     NULL,                        stress_compon+1);
   saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxy",           &mesh, &upascal,
                     SAF_SELF(db), SAF_ZONAL(&elems), H5T_NATIVE_DOUBLE,
                     NULL,          SAF_INTERLEAVE_NONE,   SAF_IDENTITY,
                     NULL,                        stress_compon+2);
   saf_declare_field(SAF_ALL,db, &stress_ftmpl, "stress tensor", &mesh, &upascal,
                     SAF_SELF(db), SAF_ZONAL(&elems), H5T_NATIVE_DOUBLE,
                     stress_compon, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY,
                     (void**)&lstress_dof_tuple, &stress);

   /* Write the field. */
   saf_write_field(SAF_ALL, &stress, SAF_WHOLE_FIELD, 0, H5I_INVALID_HID, NULL, saf_file);
   PASSED;

   /* Free the dofs now that we are done with them. */
   free(lstress_dof_tuple);

   return;
}

int main(int argc, char **argv)
{
   char dbname[1024];        /* Name of the SAF database file to be created. */
   int rank=0;               /* Rank of this process for parallel. */
   SAF_DbProps *dbprops=NULL;      /* Handle to the SAF databsae properties. */
   int failCount = 0;
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

   /* Initialize the library. */
   saf_init(SAF_DEFAULT_LIBPROPS);

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
      saf_setProps_Clobber(dbprops);

      /* Open the SAF database. Give it name dbname, properties p and
       * set db to be a handle to this database. */
      TESTING("database open");
      db = saf_open_database(dbname,dbprops);
      PASSED;

      /* Get the handle to the master file. */
      saf_file = db;

      /* Construct the base space with edge_ct_x edges in the x direction and
       * edge_ct_y edges in the y  direction in database db in SAF file
       * saf_file.  Set mesh to mesh set, nodes to the category of nodes in the
       * mesh and elems to the category of elements in the mesh. */
      make_base_space();

      /* Construct the coordinate field on the mesh. */
      make_coord_field();

      /* Construct the scalar field on the mesh. */
      make_scalar_field();

      /* Construct the stress tensor on the mesh. */
      make_stress_field();

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
