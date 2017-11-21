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
#include <safP.h> /* for SAF_REQUIRE macro */
#include <../test/testutil.h>

SAF_Db *db;                /* Handle to the SAF database. */

/*-----------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Triangle Mesh
 *
 * Description: This tutorial demonstrates the creation of a simple 2D
 *              triangular mesh immersed in 2D Euclidean vector space.  The
 *              mesh has 3 fields defined on it--the global coordinate field,
 *              a scalar field and a rank 2 symmetric tensor.  The global
 *              coordinate and scalar fields have their dofs defined on nodes
 *              while the tensor field's dofs are defined on the elements.  If
 *              the program is invoked without any arguments, then the
 *              generated mesh will have 1 triangle edge in x and 2 in y.  If
 *              it is given any args the first is interpreted as the number of
 *              x triangle edges and the second as the number of y triangle
 *              edges.
 *
 *              Parallel and serial behavior is identical due to use of
 *              SAF_ALL mode in all calls.
 *-----------------------------------------------------------------------------
 */
#define NODE_ID(i, j) ((i)*node_ct_y + (j))


/*
 * Prototypes
 */
int *make_mesh_connectivity(int edge_ct_x, int edge_ct_y );
void make_base_space(SAF_Db *db, SAF_Set *mesh, SAF_Cat *nodes,
                     SAF_Cat *elems,
                     int edge_ct_x, int edge_ct_y);
double *make_coord_field_dofs(int edge_ct_x, int edge_ct_y );
void make_coord_field(int edge_ct_x, int edge_ct_y, SAF_Db *db, SAF_Set *mesh,
                      SAF_Cat *nodes, SAF_Cat *elems, SAF_Db *saf_file);
double *make_scalar_field_dofs(int edge_ct_x,int edge_ct_y);
void make_scalar_field(int edge_ct_x, int edge_ct_y, SAF_Db *db, SAF_Set *mesh,
                       SAF_Cat *nodes, SAF_Cat *elems, SAF_Db *saf_file);
double *make_stress_field_dofs(int edge_ct_x,int edge_ct_y );
void make_stress_field(int edge_ct_x, int edge_ct_y, SAF_Db *db,
                       SAF_Set *mesh, SAF_Cat *elems, SAF_Db *saf_file);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Create rectangular array of triangles
 *
 * Description: Creates a rectangular array of triangles. The number of triangles in the X direction is 2*EDGE_CT_X and
 *              similarly for the Y direction.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int *
make_mesh_connectivity(int edge_ct_x,   /*number of edges in X direction*/
                       int edge_ct_y    /*number of edges in Y direction*/
                       )
{
   SAF_ENTER(make_mesh_connectivity, NULL);
    
   int *node_ids;                 /* The node to element connectivity. */
   int node_ct_y = edge_ct_y + 1; /* Number of nodes in y. */
   int tri_ct = 0,                /* Number of triangles in mesh. */
       conn_idx = 0;              /* Index into connectivity array. */
   int i, j;

   /* Preconditions */

   SAF_REQUIRE(edge_ct_x > 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("there must be at least 1 edge in the x direction"));
   SAF_REQUIRE(edge_ct_y > 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("there must be at least 1 edge in the y direction"));

   /* Body */

   /* allocate node_ids as single index array,
    * but treat as node_ids[edge_ct_x][edge_ct_y][3]; */
   node_ids = (int*)malloc(edge_ct_x*edge_ct_y*2*3*sizeof(int));

   for(i = 0; i < edge_ct_x; i++)
   {
      for(j = 0; j < edge_ct_y; j++)
      {
         /* Each (i,j) pair is the lower-left-hand node in
          * a quad containing two triangles. */

         /* Connectivity for lower triangle in quad,
          * follow right-hand rule (ccw) starting
          * in llh corner of quad. */

         node_ids[conn_idx++] = NODE_ID(i,j);
         node_ids[conn_idx++] = NODE_ID(i+1, j);
         node_ids[conn_idx++] = NODE_ID(i, j+1);

         tri_ct++;


         /* Connectivity for upper triangle in quad,
          * follow right-hand rule (ccw) starting
          * in urh corner of quad. */

         node_ids[conn_idx++] = NODE_ID(i+1,j);
         node_ids[conn_idx++] = NODE_ID(i+1, j+1);
         node_ids[conn_idx++] = NODE_ID(i, j+1);

         tri_ct++;
      }
   }

   /* Postconditions */

   /* Exit */
  SAF_LEAVE(node_ids);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Construct triangle mesh
 *
 * Description: Constructs the triangle mesh and writes it to SAF_FILE.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_base_space(SAF_Db *db, SAF_Set *mesh, SAF_Cat *nodes,
                     SAF_Cat *elems,
                     int edge_ct_x, int edge_ct_y)
{
   SAF_Rel trel;        /* Handle to the topological relation. */
   int abuf[] = {3};    /* Number of nodes for each element type in mesh. */
   int node_ct, ele_ct; /* Numbers of nodes and elements in the mesh. */
   int *conn;           /* Node to element connectivity data. */

   /* Make the connectivity. */
   conn = make_mesh_connectivity(edge_ct_x, edge_ct_y);

   /* Set the number of nodes and elements in the mesh. */
   node_ct = (edge_ct_x+1)*(edge_ct_y+1);
   ele_ct  = edge_ct_x*edge_ct_y*2;

   /*
    ---------------------------------------------------------------------------
    *                            DECLARE CATEGORIES
    ---------------------------------------------------------------------------
    */
   saf_declare_category(SAF_ALL, db, "nodes",  SAF_TOPOLOGY, 0, nodes);
   saf_declare_category(SAF_ALL, db, "elems",  SAF_TOPOLOGY, 2, elems);

   /*
    ---------------------------------------------------------------------------
    *                               DECLARE SET
    ---------------------------------------------------------------------------
    */
   saf_declare_set(SAF_ALL, db, "TOP_CELL", 2, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, mesh);

   /*
    ---------------------------------------------------------------------------
    *                           DECLARE COLLECTIONS
    ---------------------------------------------------------------------------
    */
   saf_declare_collection(SAF_ALL, mesh, nodes,  SAF_CELLTYPE_POINT, node_ct,
                          SAF_1DC(node_ct), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, mesh, elems,  SAF_CELLTYPE_TRI,   ele_ct,
                          SAF_1DC(ele_ct),  SAF_DECOMP_TRUE);

   /*
    ---------------------------------------------------------------------------
    *                   DECLARE AND WRITE TOPOLOGY RELATIONS
    ---------------------------------------------------------------------------
    */
   saf_declare_topo_relation(SAF_ALL,db, mesh, elems, mesh, nodes,
                             SAF_SELF(db), mesh, SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL,
                             H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, abuf, SAF_INT,
                           conn, db);
   if (conn)free(conn);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Create coordinate field
 *
 * Description: Creates the coordinate field for a rectangular triangle mesh. Number of nodes in X direction is EDGE_CT_X+1
 *              and similarly for Y direction.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
double *
make_coord_field_dofs(int edge_ct_x,    /*number of edges in X direction*/
                      int edge_ct_y     /*number of edges in Y direction*/
                      )
{
   SAF_ENTER(make_coord_field_dofs, NULL);
    
   int node_ct_x = edge_ct_x+1,         /* Number of nodes in x. */
       node_ct_y = edge_ct_y+1,         /* Number of nodes in y. */
       dofs_ub = node_ct_x*node_ct_y*2, /* Number of dofs. */
       i, j, node_id;
   double delx = 1.0/edge_ct_x,         /* X increment between nodes in x. */
          dely = 1.0/edge_ct_y,         /* Y increment between nodes in y. */
          *dofs,                        /* The array of dofs. */
          x, y;                         /* x and y coordinates. */

   /* Preconditions */

   SAF_REQUIRE(edge_ct_x > 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("there must be at least 1 edge in the x direction"));
   SAF_REQUIRE(edge_ct_y > 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("there must be at least 1 edge in the y direction"));

   /* Body */


   /* allocate dofs as single index array,
    * but treat as dofs[nx+1][ny+1][2]; */
   dofs = (double*)malloc(dofs_ub*sizeof(double));

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

   /* Postconditions */

   /* Exit */

   SAF_LEAVE(dofs);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Construct coordinate field
 *
 * Description: Constructs the coordinate field on the mesh.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_coord_field(int edge_ct_x, int edge_ct_y, SAF_Db *db, SAF_Set *mesh,
                      SAF_Cat *nodes, SAF_Cat *elems, SAF_Db *saf_file)
{
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

   /* Create the coordinate field dofs. */
   lcoord_dof_tuple = make_coord_field_dofs(edge_ct_x, edge_ct_y);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL, db, "e2_on_triangle_mesh_ctmpl",
                           SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 1,
                           NULL, &coords_ctmpl);

   tmp_ftmpl[0] = coords_ctmpl;
   tmp_ftmpl[1] = coords_ctmpl;
   saf_declare_field_tmpl(SAF_ALL, db, "e2_on_triangle_mesh_tmpl",
                           SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 2,
                           tmp_ftmpl, &coords_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *                       (buf specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "meter", &umeter);

   /* Declare the fields. */
   saf_declare_field(SAF_ALL, db, &coords_ctmpl, "X",           mesh, &umeter,
                     SAF_SELF(db), SAF_NODAL(nodes, elems), SAF_DOUBLE,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     coord_compon);
   saf_declare_field(SAF_ALL, db, &coords_ctmpl, "Y",           mesh, &umeter,
                     SAF_SELF(db), SAF_NODAL(nodes, elems), SAF_DOUBLE,
                     NULL,         SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,
                     coord_compon+1);
   saf_declare_field(SAF_ALL, db, &coords_ftmpl, "coord field", mesh, &umeter,
                     SAF_SELF(db), SAF_NODAL(nodes, elems), SAF_DOUBLE,
                     coord_compon, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                     &coords);

   /* Write the coordinate field. */
   saf_write_field(SAF_ALL, &coords, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID,(void**)&lcoord_dof_tuple, saf_file);

   /* Specify that is a coordinate field */
   saf_declare_coords(SAF_ALL, &coords);

   /* Free the dofs now that we are done with them. */
   free(lcoord_dof_tuple);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Create scalar on mesh
 *
 * Description: Creates a scalar on a rectangular triangle mesh. Number of nodes in X direction is EDGE_CT_X+1 and similarly
 *              for Y direction.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
double *
make_scalar_field_dofs(int edge_ct_x,   /*number of edges in X direction*/
                       int edge_ct_y    /*number of edges in Y direction*/
                       )
{
   SAF_ENTER(make_scalar_field_dofs, NULL);

   int node_ct_x = edge_ct_x+1,       /* Number of nodes in x direction. */
       node_ct_y = edge_ct_y+1,       /* Number of nodes in y direction. */
       dofs_ub = node_ct_x*node_ct_y, /* Number of dofs. */
       i, j, node_id;
   double delx = 1.0,                 /* X increment between nodes in x. */
          dely = 1.0,                 /* Y increment between nodes in y. */
          *dofs,                      /* The array of dofs. */
          x, y;                       /* Used to compute dofs. */

   /* Preconditions */

   SAF_REQUIRE(edge_ct_x > 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("there must be at least 1 edge in the x direction"));
   SAF_REQUIRE(edge_ct_y > 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("there must be at least 1 edge in the y direction"));

   /* Body */


   /* allocate dofs as single index array,
    * but treat as dofs[nx+1][ny+1][2]; */
   dofs = (double*)malloc(dofs_ub*sizeof(double));

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

   /* Postconditions */

   /* Exit */

   SAF_LEAVE(dofs);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Construct scalar field
 *
 * Description: Construct the scalar field on the mesh.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_scalar_field(int edge_ct_x, int edge_ct_y, SAF_Db *db, SAF_Set *mesh,
                       SAF_Cat *nodes, SAF_Cat *elems, SAF_Db *saf_file)
{
   SAF_FieldTmpl scalar_ftmpl; /* Handle to the field template. */
   SAF_Field scalar;           /* Handle to the field. */
   SAF_Unit umeter;            /* Handle to the units of the field. */
   double *lscalar_dof_tuple;  /* The scalar field dofs. */

   /* Create the scalar field dofs. */
   lscalar_dof_tuple = make_scalar_field_dofs(edge_ct_x, edge_ct_y);

   /*
    ---------------------------------------------------------------------------
    *                          DECLARE FIELD TEMPLATE
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL, db, "at0_on_triangle_mesh_tmpl",
                           SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QLENGTH, 1,
                           NULL, &scalar_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                          DECLARE AND WRITE FIELD
    *                       (buf specified in write call)
    ---------------------------------------------------------------------------
    */
   /* Get the units for the field. */
   saf_find_one_unit(db, "meter", &umeter);

   /* Declare the field. */
   saf_declare_field(SAF_ALL, db, &scalar_ftmpl, "scalar field", mesh, &umeter,
                     SAF_SELF(db), SAF_NODAL(nodes, elems), SAF_DOUBLE,
                     NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &scalar);

   /* Write the field. */
   saf_write_field(SAF_ALL, &scalar, SAF_WHOLE_FIELD, 1,
                   H5I_INVALID_HID,(void**)&lscalar_dof_tuple, saf_file);

   /* Free the dofs now that we are done with them. */
   free(lscalar_dof_tuple);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Create stress field
 *
 * Description: Creates a stress field on a rectangular array of triangles. Used to test instantiation of st2 field; values of
 *              field are meaningless. Number of nodes in X direction is EDGE_CT_X+1 and similarly for Y direction.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
double *
make_stress_field_dofs(int edge_ct_x,   /*number of edges in X direction*/
                       int edge_ct_y    /*number of edges in Y direction*/
                       )
{
    SAF_ENTER(make_stress_field_dofs, NULL);

    int dofs_ub = edge_ct_x*edge_ct_y*2*3, /* Number of dofs. */
       tri_ct = 0,                        /* Triangle counter. */
       i, j;
   double *dofs;                          /* The array of dofs. */

   /* Preconditions */

   SAF_REQUIRE(edge_ct_x > 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("there must be at least 1 edge in the x direction"));
   SAF_REQUIRE(edge_ct_y > 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("there must be at least 1 edge in the y direction"));

   /* Body */

   /* allocate dofs as single index array,
    * but treat as dofs[edge_ct_x][edge_ct_y][3]; */
   dofs = (double*)malloc(dofs_ub*sizeof(double));

   for(i=0; i<edge_ct_x; i++)
   {
      for(j=0; j<edge_ct_y; j++)
      {
         /* Each (i,j) pair is the lower-left-hand node in 
          * a quad containing two triangles. */

         /* Stress for lower triangle in quad, */

         dofs[3*tri_ct]   = tri_ct+0.0; /* xx component */
         dofs[3*tri_ct+1] = tri_ct+0.1; /* xy component */
         dofs[3*tri_ct+2] = tri_ct+0.2; /* yy component */

         tri_ct++;

         /* Stress for upper triangle in quad, */

         dofs[3*tri_ct]   = tri_ct+0.0; /* xx component */
         dofs[3*tri_ct+1] = tri_ct+0.1; /* xy component */
         dofs[3*tri_ct+2] = tri_ct+0.2; /* yy component */

         tri_ct++;
      }
   }

   /* Postconditions */

   /* Exit */

   SAF_LEAVE(dofs);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Construct stress field
 *
 * Description: Construct the stress field on the mesh.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_stress_field(int edge_ct_x, int edge_ct_y, SAF_Db *db,
                       SAF_Set *mesh, SAF_Cat *elems, SAF_Db *saf_file)
{
   SAF_FieldTmpl stress_ftmpl, /* Handle to the stress field template. */
                 stress_ctmpl, /* Handle to the stress field's components'
                                * field templates. */
		 tmp_ftmpl[3]; /* temporary field template handles for
				* component field templates. */
   SAF_Field stress,           /* Handle to the stress field. */
             stress_compon[3]; /* Handle to the stress field's components. */
   SAF_Unit upascal;           /* Handle to the units for this field. */
   double *lstress_dof_tuple;  /* The stress field dofs. */
   SAF_Quantity *qbuf=NULL;

   /* Create the stress field dofs. */
   lstress_dof_tuple = make_stress_field_dofs(edge_ct_x, edge_ct_y);

   /*
    ---------------------------------------------------------------------------
    *                          DECLARE FIELD TEMPLATES 
    ---------------------------------------------------------------------------
    */
   qbuf = SAF_QNAME(db,"pressure");
   saf_declare_field_tmpl(SAF_ALL, db, "st2_ei_on_triangle_mesh",
                          SAF_ALGTYPE_SCALAR, SAF_UNITY,
                          qbuf, 1, NULL,
			  &stress_ctmpl);

   tmp_ftmpl[0] = stress_ctmpl;
   tmp_ftmpl[1] = stress_ctmpl;
   tmp_ftmpl[2] = stress_ctmpl;
   saf_declare_field_tmpl(SAF_ALL, db, "st2_e2_on_triangle_mesh",
                          SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI,
                          qbuf, 3, tmp_ftmpl,
                          &stress_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    *			   (buf specified in declare call)
    ---------------------------------------------------------------------------
    */
   /* Get the units for the field. */
   saf_find_one_unit(db, "pascal", &upascal);

   /* Declare the fields. */
   saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sxx",           mesh, &upascal,
                     SAF_SELF(db), SAF_ZONAL(elems), SAF_DOUBLE,
                     NULL,          SAF_INTERLEAVE_NONE,   SAF_IDENTITY,
                     NULL,                        stress_compon);
   saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Syy",           mesh, &upascal,
                     SAF_SELF(db), SAF_ZONAL(elems), SAF_DOUBLE,
                     NULL,          SAF_INTERLEAVE_NONE,   SAF_IDENTITY,
                     NULL,                        stress_compon+1);
   saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sxy",           mesh, &upascal,
                     SAF_SELF(db), SAF_ZONAL(elems), SAF_DOUBLE,
                     NULL,          SAF_INTERLEAVE_NONE,   SAF_IDENTITY,
                     NULL,                        stress_compon+2);
   saf_declare_field(SAF_ALL, db, &stress_ftmpl, "stress tensor", mesh, &upascal,
                     SAF_SELF(db), SAF_ZONAL(elems), SAF_DOUBLE,
                     stress_compon, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY,
                     (void**)&lstress_dof_tuple, &stress);

   /* Write the field. */
   saf_write_field(SAF_ALL, &stress, SAF_WHOLE_FIELD, 0, H5I_INVALID_HID,NULL, saf_file);

   /* Free the dofs now that we are done with them. */
   free(lstress_dof_tuple);
   if (qbuf)free(qbuf);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Triangle Mesh
 * Purpose:     Main entry point
 *
 * Description: See [Triangle Mesh].
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc,
     char **argv)
{
  char dbname[1024];        /* Name of the SAF database file to be created. */
  int rank=0;               /* Rank of this process for parallel. */
  /* SAF_Db db; */                /* Handle to the SAF database. */
  SAF_DbProps *dbprops;      /* Handle to the SAF databsae properties. */
  SAF_Cat nodes, elems;     /* Handles to the node and elements categories. */
  SAF_Set mesh;             /* Handle to the mesh set. */
  SAF_Db *saf_file;        /* Handle to the saf file. */
  int edge_ct_x, edge_ct_y; /* The number of triangle edges in
			     * the x and y directions. */
  int failed = 0;

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

  /* root process */
  if (rank == 0)
    {
      /* since we want to see what's happening make sure stdout and stderr
       * are unbuffered */
      setbuf(stdout, NULL);
      setbuf(stderr, NULL);

      /* Process any command line args to get the number of triangle
       * edges in the x and y directions. */
      edge_ct_x = 1;
      if (argc > 1)
	edge_ct_x = atoi(argv[1]);
      edge_ct_y = 2;
      if (argc > 2)
	edge_ct_y = atoi(argv[2]);
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
  strcpy(dbname, "triangle_mesh.saf");

  SAF_TRY_BEGIN
    {
      /* Because we are in a try block here, all failures will send us to
         the one and only catch block at the end of this test */

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

      /* Construct the base space with edge_ct_x triangle edges in the x
       * direction and edge_ct_y triangle edges in the y  direction in
       * database db in SAF file saf_file.  Set mesh to mesh set, nodes to the
       * category of nodes in the mesh and elems to the category of elements in
       * the mesh. */
      make_base_space(db, &mesh, &nodes, &elems,
                      edge_ct_x, edge_ct_y);

      /* Construct the coordinate field on the mesh. */
      make_coord_field(edge_ct_x, edge_ct_y, db, &mesh, &nodes, &elems, saf_file);

      /* Construct the scalar field on the mesh. */
      make_scalar_field(edge_ct_x, edge_ct_y, db, &mesh, &nodes, &elems,
                        saf_file);

      /* Construct the stress tensor on the mesh. */
      make_stress_field(edge_ct_x, edge_ct_y, db, &mesh, &elems, saf_file);
 

      /* Close the SAF database. */
      saf_close_database(db);
      saf_freeProps_database(dbprops);

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
  MPI_Bcast(&failed, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Finalize();
#endif

  return failed;
}
