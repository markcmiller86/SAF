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
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif
#include <math.h>
#include <safP.h>
#include <testutil.h>

#ifdef M_PI
#   define SAF_M_PI     M_PI
#else
#   define SAF_M_PI     3.14159265358979323846  /* pi */
#endif


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Pipe Use Case 
 *
 * Description: This is testing code that exercises the pipe use case. This code declares SAF objects and, optionally,
 *		*writes* the raw data. There are two command line arguments; DO_WRITES and DO_MULTIFLE.
 *
 *		If DO_WRITES is *not* present, this test will only issue saf_declare_xxx() calls but not
 *              saf_write_xxx() calls.
 *		If DO_WRITES is present, this test will also issue the saf_write_xxx() calls to write raw data for
 *              fields and relations.
 *
 *		If DO_MULTIFILE is present, the raw data will be written to different files. Otherwise, it will be
 *              written to the master file.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Bill Arrighi, LLNL, Tues Oct 10, 2000 
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
/* Types of elements that may be used. */
enum elmts {LINEAR_QUAD, LINEAR_HEX, QUADRATIC_QUAD, QUADRATIC_HEX};

/* Type of element to be used. */
enum elmts elmt_type;

/* The SAF type of the element to be used. */
SAF_CellType saf_elmt_type;

/* Number of elements in the theta, Z, and R directions; the total
 * number of elements in the mesh and the number of elements in cell_1,
 * cell_2 and cell_3.  r_ele_ct is not relevant for pipes made of
 * elements whose base dimension is not 3 */
int theta_ele_ct = 0, z_ele_ct = 0, r_ele_ct = 0, ele_ct,
    num_cell_1_elmts, num_cell_2_elmts, num_cell_3_elmts;


/* The inner radius of the pipe and, for pipes made of hexes, the thickness
 * of the wall of the pipe. */
float inner_radius = 0.0, thickness = 0.0;

/* The location in z of the lower end of the pipe and the height of pipe. */
float z_min, height = 0.0;

/* Offset of the axis of the pipe from Z axis in X and Y directions. */
float x_offset, y_offset;

/* The size of the connectivity array for each cell (part). */
int cell_1_conn_sz, cell_2_conn_sz, cell_3_conn_sz;

/* The number of nodes in each element; the number of nodes in the theta, r
 * and z directions; the total number of nodes in the mesh; and the number
 * of nodes in the 3 cells. */
int nodes_per_ele, theta_node_ct, r_node_ct, z_node_ct, node_ct,
    cell_1_node_ct = 0, cell_2_node_ct = 0, cell_3_node_ct = 0;

/* The topological dimension of the elements from which the pipe is made. */
int topo_dim;

/* element id and connectivity arrays for each cell (part) */
int *cell_1_eids, *cell_1_conn,
    *cell_2_eids, *cell_2_conn,
    *cell_3_eids, *cell_3_conn;

/* Arrays indicating nodal inclusion in each cell. */
int *cell_1_nodes, *cell_2_nodes, *cell_3_nodes;

/* The array of global coordinate dofs. */
double *lcoord_dof_tuple;

static const float TWOPI = 2 * SAF_M_PI;

static void
read_user_input(void)
{
  int in_val = -1;

  /* Get the type of element from which the pipe is made. */
/*  while (in_val < 0 || in_val > 3) */
  while (in_val < 0 || in_val > 1)
  {
/*    printf("Select the type of element to be used.\nEnter 0 for linear quads, "
           "1 for linear hexahedrons,\n2 for quadratic quads or 3 for "
           "quadratic hexahedrons: "); */
    printf("Select the type of element to be used.\nEnter 0 for linear quads or "
           "1 for linear hexahedrons: ");
    scanf("%i", &in_val);
/*    if (in_val < 0 || in_val > 3) */
    if (in_val < 0 || in_val > 1)
/*      printf("\n***Invalid element type.  Valid values are 0-3.***\n\n"); */
      printf("\n***Invalid element type.  Valid values are 0 or 1.***\n\n");
  }
  switch (in_val)
  {
    case 0:
      elmt_type = LINEAR_QUAD;
      break;
    case 1:
      elmt_type = LINEAR_HEX;
      break;
    case 2:
      elmt_type = QUADRATIC_QUAD;
      break;
    case 3:
      elmt_type = QUADRATIC_HEX;
      break;
  }

  /* Get the number of elements in the theta direction. */
  while (theta_ele_ct < 6)
  {
    printf("\nEnter how many elements are in the theta direction--\n"
           "value must be 6 or greater: ");
    scanf("%i", &theta_ele_ct);
    if (theta_ele_ct < 6)
      printf("\n***The number of elements in the theta direction must be 6 or greater.***\n");
  }

  /* Get the number of elements in the z direction. */
  while (z_ele_ct <= 0)
  {
    printf("\nEnter how many elements are in the z direction: ");
    scanf("%i", &z_ele_ct);
    if (z_ele_ct <= 0)
      printf("\n***The number of elements in the z direction must greater than 0.***\n");
  }

  /* Get the number of elements in the r direction for 3D elements. */
  if (elmt_type == QUADRATIC_HEX || elmt_type == LINEAR_HEX)
  {
    while (r_ele_ct <= 0)
    {
      printf("\nEnter how many elements are in the radial direction: ");
      scanf("%i", &r_ele_ct);
      if (r_ele_ct <=0)
        printf("\n***The number of elements in the radial direction must be greater than 0.***\n");
    }
  }
  else
    r_ele_ct = 1;

  /* Get the inner radius of the pipe. */
  while (inner_radius <= 0)
  {
    printf("\nEnter the inner radius of the pipe: ");
    scanf("%f", &inner_radius);
    if (inner_radius <= 0)
      printf("\n***The inner radius must be greater than 0.***\n");
  }

  /* Get the thickness of the wall of the pipe for 3D elements */
  if (elmt_type == QUADRATIC_HEX || elmt_type == LINEAR_HEX)
  {
    while (thickness <= 0)
    {
      printf("\nEnter the thickness of the wall of the pipe: ");
      scanf("%f", &thickness);
      if (thickness <= 0)
        printf("\n***The thickness must be greater than 0.***\n");
    }
  }

  /* Get the location in z of the lower end of the pipe. */
  printf("\nEnter the location in z of the lower end of the pipe: ");
  scanf("%f", &z_min);

  /* Get the height of the pipe. */
  while (height <= 0)
  {
    printf("\nEnter the height of the pipe: ");
    scanf("%f", &height);
    if (height <= 0)
      printf("\n***The height must be greater than 0.***\n");
  }

  /* Get the offset in the x direction of the axis of the pipe. */
  printf("\nEnter the offset in the x direction of the axis of the pipe: ");
  scanf("%f", &x_offset);

  /* Get the offset in the x direction of the axis of the pipe. */
  printf("\nEnter the offset in the y direction of the axis of the pipe: ");
  scanf("%f", &y_offset);

  return;
}

static void
compute_derived_parameters(void)
{
  size_t int_sz;

  if (elmt_type == QUADRATIC_HEX)
  {
    topo_dim      = 3;
    nodes_per_ele = 20;
    theta_node_ct = 2 * theta_ele_ct;
    r_node_ct     = 2 * r_ele_ct + 1;
    z_node_ct     = 2 * z_ele_ct + 1;
    node_ct       = ((r_ele_ct + 1) * (3 * z_ele_ct + 2) +
                     r_ele_ct * (z_ele_ct + 1)) * theta_ele_ct;
  }
  else if (elmt_type == LINEAR_HEX)
  {
    saf_elmt_type = SAF_CELLTYPE_HEX;
    topo_dim      = 3;
    nodes_per_ele = 8;
    theta_node_ct = theta_ele_ct;
    r_node_ct     = r_ele_ct + 1;
    z_node_ct     = z_ele_ct + 1;
    node_ct       = r_node_ct * theta_node_ct * z_node_ct;
  }
  else if (elmt_type == QUADRATIC_QUAD)
  {
    topo_dim      = 2;
    nodes_per_ele = 8;
    theta_node_ct = 2 * theta_ele_ct;
    z_node_ct     = 2 * z_ele_ct + 1;
    node_ct       = (2 * (z_ele_ct + 1) + z_ele_ct) * theta_ele_ct;
  }
  else
  {
    saf_elmt_type = SAF_CELLTYPE_QUAD;
    topo_dim      = 2;
    nodes_per_ele = 4;
    theta_node_ct = theta_ele_ct;
    z_node_ct     = z_ele_ct + 1;
    node_ct       = theta_node_ct * z_node_ct;
  }
  ele_ct           = r_ele_ct * theta_ele_ct * z_ele_ct;
  num_cell_1_elmts = (2 * (z_ele_ct + 1) / 3 - (z_ele_ct + 1) / 3) * r_ele_ct *
                     (theta_ele_ct - theta_ele_ct / 3 - 3);
  num_cell_2_elmts = (2 * (z_ele_ct + 1) / 3 - (z_ele_ct + 1) / 3) * r_ele_ct *
                     (theta_ele_ct / 3 - 1);
  num_cell_3_elmts = ele_ct - num_cell_1_elmts - num_cell_2_elmts;
  cell_1_conn_sz   = num_cell_1_elmts * nodes_per_ele;
  cell_2_conn_sz   = num_cell_2_elmts * nodes_per_ele;
  cell_3_conn_sz   = num_cell_3_elmts * nodes_per_ele;
  int_sz           = sizeof(int);
  cell_1_eids      = (int*)malloc(num_cell_1_elmts * int_sz);
  cell_2_eids      = (int*)malloc(num_cell_2_elmts * int_sz);
  cell_3_eids      = (int*)malloc(num_cell_3_elmts * int_sz);
  cell_1_conn      = (int*)malloc(cell_1_conn_sz * int_sz);
  cell_2_conn      = (int*)malloc(cell_2_conn_sz * int_sz);
  cell_3_conn      = (int*)malloc(cell_3_conn_sz * int_sz);
  cell_1_nodes     = (int*)calloc((unsigned int)node_ct, int_sz);
  cell_2_nodes     = (int*)calloc((unsigned int)node_ct, int_sz);
  cell_3_nodes     = (int*)calloc((unsigned int)node_ct, int_sz);
  lcoord_dof_tuple = (double*)malloc(node_ct * 3 * sizeof(double));

  return;
}

/**/
static void
make_arrays_quadratic_hex(void)
{
  SAF_ENTER(make_arrays_quadratic_hex, /*void*/);
    
  int cell_1_elmt_ct = 0, cell_2_elmt_ct = 0, cell_3_elmt_ct = 0,
      i = 0, j, k, e = 0, p = 0, pt_num, offset, in_cell_1_2_row,
      *conn_ptr, *first_elmt_in_row_ptr=0, *nodes_ptr;
  double r, r_plus, r_plus_plus, z_plus, z_plus_plus, theta, theta_plus,
         theta_plus_plus, cos_theta, sin_theta, cos_theta_plus, sin_theta_plus,
         cos_theta_plus_plus, sin_theta_plus_plus, x, y, z, x_rplus, y_rplus,
         x_rplusplus, y_rplusplus, x_thetaplus, y_thetaplus,
         x_rplusplus_thetaplus, y_rplusplus_thetaplus, *lcoord_dof_tuple_ptr;

  /* Visit the same node (node 0) of each element in the pipe.  For this
   * quadratic element the next element in each direction is 2 nodes away.
   * Fill in the connectivity and global coordinates array for each element. */
  r_plus_plus = inner_radius;
  for (i = 0; i < r_ele_ct; i++)
  {
    /* Loop over each element in the radial direction. */

    /* Determine r, the radius at nodes 0, 3, 4, 7, 11, 15, 16 and 19; r_plus,
     * the radius at nodes 8, 10, 12 and 14; and r_plus_plus, the radius at
     * nodes 1, 2, 5, 6, 9, 13, 17 and 18. */
    r           = r_plus_plus;
    r_plus      = inner_radius + thickness * ((double)(i)+.5)/((double)(r_ele_ct));
    r_plus_plus = inner_radius + thickness * ((double)(i+1))/((double)(r_ele_ct));

    z_plus_plus = z_min;
    for (j = 0; j < z_ele_ct; j++)
    {
      /* Loop over each element in the z direction. */

      /* Determine z, the location in z of nodes 0, 1, 2, 3, 8, 9, 10 and 11;
       * z_plus, the location in z of nodes 16, 17, 18 and 19; and z_plus_plus,
       * the location in z of nodes 4, 5, 6, 7, 12, 13, 14 and 15. */
      z           = z_plus_plus;
      z_plus      = z_min + height * ((double)(j)+.5)/((double)(z_ele_ct));
      z_plus_plus = z_min + height * ((double)(j+1))/((double)(z_ele_ct));

      /* Determine if any elements in this row are in cell 1 or 2. */
      in_cell_1_2_row = j>(z_ele_ct+1)/3-1 && j<2*(z_ele_ct+1)/3;

      theta_plus_plus = 0.0;
      for (k = 0; k < theta_ele_ct; k++)
      {
        /* Loop over each element in the theta direction. */

        /* Determine theta, the angle of nodes 0, 1, 4, 5, 8, 12, 16 and 17;
         * and theta_plus, the angle of nodes 9, 11, 13 and 15. */
        theta      = theta_plus_plus;
        theta_plus = TWOPI * ((double)(k)+.5)/((double)theta_ele_ct);

        /* Set element connectivity, element id and global coordinate values. */

        /* Determine which cell (part) this element belongs to.  Set
         * conn_ptr to the current end of that connectivity array, set
         * the element id for that cell and increment the count of elements
         * in that cell. */
        if (in_cell_1_2_row && (k>theta_ele_ct/3 && k<theta_ele_ct-2))
	{
	  conn_ptr = cell_1_conn + cell_1_elmt_ct * nodes_per_ele;
          nodes_ptr = cell_1_nodes;
          cell_1_eids[cell_1_elmt_ct] = e;
          cell_1_elmt_ct++;
	}
	else if (in_cell_1_2_row && (k<theta_ele_ct/3-1))
	{
	  conn_ptr = cell_2_conn + cell_2_elmt_ct * nodes_per_ele;
          nodes_ptr = cell_2_nodes;
          cell_2_eids[cell_2_elmt_ct] = e;
          cell_2_elmt_ct++;
          /* We must know where the nodes at theta==0 are since the
           * nodes at theta==2pi are coincident with these. */
	  if (k == 0)
	    first_elmt_in_row_ptr = conn_ptr;
	}
        else
	{
	  conn_ptr = cell_3_conn + cell_3_elmt_ct * nodes_per_ele;
          nodes_ptr = cell_3_nodes;
          cell_3_eids[cell_3_elmt_ct] = e;
          cell_3_elmt_ct++;
          /* We must know where the nodes at theta==0 are since the
           * nodes at theta==2pi are coincident with these. */
	  if (k == 0)
	    first_elmt_in_row_ptr = conn_ptr;
	}
        /* Fill in the connectivity data and global coordinate values for
         * this element.
         * First determine x, y, x_rplus, y_rplus,  x_rplusplus, y_rplusplus,
         * x_thetaplus, y_thetaplus, x_rplusplus_thetaplus,
         * y_rplusplus_thetaplus. */
        cos_theta             = cos(theta);
        sin_theta             = sin(theta);
        cos_theta_plus        = cos(theta_plus);
        sin_theta_plus        = sin(theta_plus);
        x                     = r           * cos_theta      + x_offset;
        y                     = r           * sin_theta      + y_offset;
        x_rplus               = r_plus      * cos_theta      + x_offset;
        y_rplus               = r_plus      * sin_theta      + y_offset;
        x_rplusplus           = r_plus_plus * cos_theta      + x_offset;
        y_rplusplus           = r_plus_plus * sin_theta      + y_offset;
        x_thetaplus           = r           * cos_theta_plus + x_offset;
        y_thetaplus           = r           * sin_theta_plus + y_offset;
        x_rplusplus_thetaplus = r_plus_plus * cos_theta_plus + x_offset;
        y_rplusplus_thetaplus = r_plus_plus * sin_theta_plus + y_offset;
        /* Node 0. */
        offset = 3 * p;
	conn_ptr[0]  = p;
        nodes_ptr[p] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x;
        *lcoord_dof_tuple_ptr++ = y;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 1. */
        pt_num = p + (z_ele_ct + 1) * theta_node_ct +
                 (z_ele_ct + 1) * (theta_node_ct / 2) +
                 z_ele_ct * theta_node_ct / 2;
        offset = 3 * pt_num;
	conn_ptr[1]  = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplusplus;
        *lcoord_dof_tuple_ptr++ = y_rplusplus;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 4. */
        pt_num = p + theta_node_ct + theta_node_ct / 2;
        offset = 3 * pt_num;
	conn_ptr[4]  = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x;
        *lcoord_dof_tuple_ptr++ = y;
        *lcoord_dof_tuple_ptr   = z_plus_plus;
        /* Node 5. */
        pt_num = conn_ptr[1] + theta_node_ct + theta_node_ct / 2;
        offset = 3 * pt_num;
	conn_ptr[5]  = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplusplus;
        *lcoord_dof_tuple_ptr++ = y_rplusplus;
        *lcoord_dof_tuple_ptr   = z_plus_plus;
        /* Node 8. */
        pt_num = p + (z_ele_ct + 1 - j) * theta_node_ct +
                 z_ele_ct * theta_node_ct / 2 - k;
        offset = 3 * pt_num;
	conn_ptr[8]  = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplus;
        *lcoord_dof_tuple_ptr++ = y_rplus;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 9. */
        pt_num = conn_ptr[1] + 1;
        offset = 3 * pt_num;
	conn_ptr[9]  = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplusplus_thetaplus;
        *lcoord_dof_tuple_ptr++ = y_rplusplus_thetaplus;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 11. */
        pt_num = p + 1;
        offset = 3 * pt_num;
	conn_ptr[11] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_thetaplus;
        *lcoord_dof_tuple_ptr++ = y_thetaplus;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 12. */
        pt_num = conn_ptr[8] + theta_node_ct / 2;
        offset = 3 * pt_num;
	conn_ptr[12] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplus;
        *lcoord_dof_tuple_ptr++ = y_rplus;
        *lcoord_dof_tuple_ptr   = z_plus_plus;
        /* Node 13. */
        pt_num = conn_ptr[5] + 1;
        offset = 3 * pt_num;
	conn_ptr[13] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplusplus_thetaplus;
        *lcoord_dof_tuple_ptr++ = y_rplusplus_thetaplus;
        *lcoord_dof_tuple_ptr   = z_plus_plus;
        /* Node 15. */
        pt_num = conn_ptr[4] + 1;
        offset = 3 * pt_num;
	conn_ptr[15] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_thetaplus;
        *lcoord_dof_tuple_ptr++ = y_thetaplus;
        *lcoord_dof_tuple_ptr   = z_plus_plus;
        /* Node 16. */
        pt_num = p + theta_node_ct - k;
        offset = 3 * pt_num;
	conn_ptr[16] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x;
        *lcoord_dof_tuple_ptr++ = y;
        *lcoord_dof_tuple_ptr   = z_plus;
        /* Node 17. */
        pt_num = conn_ptr[1] + theta_node_ct - k;
        offset = 3 * pt_num;
	conn_ptr[17] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplusplus;
        *lcoord_dof_tuple_ptr++ = y_rplusplus;
        *lcoord_dof_tuple_ptr   = z_plus;

        if (k == (theta_ele_ct - 1))
        {
          /* Nodes in the faces at theta_plus_plus == 2pi are coincident with
           * nodes in the faces at theta == 0. */
          conn_ptr[2]  = first_elmt_in_row_ptr[1];
          conn_ptr[3]  = first_elmt_in_row_ptr[0];
          conn_ptr[6]  = first_elmt_in_row_ptr[5];
          conn_ptr[7]  = first_elmt_in_row_ptr[4];
          conn_ptr[10] = first_elmt_in_row_ptr[8];
          conn_ptr[14] = first_elmt_in_row_ptr[12];
          conn_ptr[18] = first_elmt_in_row_ptr[17];
          conn_ptr[19] = first_elmt_in_row_ptr[16];
          nodes_ptr[first_elmt_in_row_ptr[1]]  = 1;
          nodes_ptr[first_elmt_in_row_ptr[0]]  = 1;
          nodes_ptr[first_elmt_in_row_ptr[5]]  = 1;
          nodes_ptr[first_elmt_in_row_ptr[4]]  = 1;
          nodes_ptr[first_elmt_in_row_ptr[8]]  = 1;
          nodes_ptr[first_elmt_in_row_ptr[12]] = 1;
          nodes_ptr[first_elmt_in_row_ptr[17]] = 1;
          nodes_ptr[first_elmt_in_row_ptr[16]] = 1;
        }
        else
        {
          /* Nodes in the faces not at theta_plus_plus == 2pi are unique. */

          /* Determine theta_plus_plus, the angle of nodes 2, 3, 6, 7, 10, 14,
           * 18 and 19. */
          theta_plus_plus = TWOPI * ((double)(k+1))/((double)theta_ele_ct);
          cos_theta_plus_plus = cos(theta_plus_plus);
          sin_theta_plus_plus = sin(theta_plus_plus);
          x                   = r           * cos_theta_plus_plus + x_offset;
          y                   = r           * sin_theta_plus_plus + y_offset;
          x_rplus             = r_plus      * cos_theta_plus_plus + x_offset;
          y_rplus             = r_plus      * sin_theta_plus_plus + y_offset;
          x_rplusplus         = r_plus_plus * cos_theta_plus_plus + x_offset;
          y_rplusplus         = r_plus_plus * sin_theta_plus_plus + y_offset;
          /* Node 2. */
          pt_num = conn_ptr[1] + 2;
          offset = 3 * pt_num;
          conn_ptr[2]  = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_rplusplus;
          *lcoord_dof_tuple_ptr++ = y_rplusplus;
          *lcoord_dof_tuple_ptr   = z;
          /* Node 3. */
          pt_num = p + 2;
          offset = 3 * pt_num;
          conn_ptr[3]  = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x;
          *lcoord_dof_tuple_ptr++ = y;
          *lcoord_dof_tuple_ptr   = z;
          /* Node 6. */
          pt_num = conn_ptr[5] + 2;
          offset = 3 * pt_num;
          conn_ptr[6]  = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_rplusplus;
          *lcoord_dof_tuple_ptr++ = y_rplusplus;
          *lcoord_dof_tuple_ptr   = z_plus_plus;
          /* Node 7. */
          pt_num = conn_ptr[4] + 2;
          offset = 3 * pt_num;
          conn_ptr[7]  = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x;
          *lcoord_dof_tuple_ptr++ = y;
          *lcoord_dof_tuple_ptr   = z_plus_plus;
          /* Node 10. */
          pt_num = conn_ptr[8] + 1;
          offset = 3 * pt_num;
          conn_ptr[10] = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_rplus;
          *lcoord_dof_tuple_ptr++ = y_rplus;
          *lcoord_dof_tuple_ptr   = z;
          /* Node 14. */
          pt_num = conn_ptr[12] + 1;
          offset = 3 * pt_num;
          conn_ptr[14] = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_rplus;
          *lcoord_dof_tuple_ptr++ = y_rplus;
          *lcoord_dof_tuple_ptr   = z_plus_plus;
          /* Node 18. */
          pt_num = conn_ptr[17] + 1;
          offset = 3 * pt_num;
          conn_ptr[18] = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_rplusplus;
          *lcoord_dof_tuple_ptr++ = y_rplusplus;
          *lcoord_dof_tuple_ptr   = z_plus;
          /* Node 19. */
          pt_num = conn_ptr[16] + 1;
          offset = 3 * pt_num;
          conn_ptr[19] = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x;
          *lcoord_dof_tuple_ptr++ = y;
          *lcoord_dof_tuple_ptr   = z_plus;
	}
        /* Move to the next element and set the global point number for
         * node 0 of that element. */
        e++;
        p += 2;
      }
      /* Set the global point number for node 0 of the first element in the
       * next row. */
      p += theta_node_ct/2;
    }
    /* Set the global point number for node 0 of the first element at the
     * next radius. */
    p += theta_node_ct + (z_ele_ct + 1) * (theta_node_ct / 2);
  }
  SAF_ENSURE(cell_1_elmt_ct == cell_1_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_1_elmt_ct incorrect"));
  SAF_ENSURE(cell_2_elmt_ct == cell_2_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_2_elmt_ct incorrect"));
  SAF_ENSURE(cell_3_elmt_ct == cell_3_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_3_elmt_ct incorrect"));
  SAF_LEAVE(/*void*/);
}

/**/
static void
make_arrays_linear_hex(void)
{
  SAF_ENTER(make_arrays_linear_hex, /*void*/);
    
  int cell_1_elmt_ct = 0, cell_2_elmt_ct = 0, cell_3_elmt_ct = 0,
      i, j, k, e = 0, p = 0, pt_num, offset, in_cell_1_2_row,
      *conn_ptr, *first_elmt_in_row_ptr=0, *nodes_ptr;
  double cos_theta, sin_theta, cos_theta_plus, sin_theta_plus,
         x, y, x_rplus, y_rplus, x_thetaplus, y_thetaplus,
         x_rplusthetaplus, y_rplusthetaplus,
         r, r_plus, z, z_plus, theta, theta_plus, *lcoord_dof_tuple_ptr;

  /* Visit the same node (node 0) of each element in the pipe.  For this
   * linear element the next element in each direction is 1 node away.
   * Fill in the connectivity and global coordinates array for each element. */
  r_plus = inner_radius;
  for (i = 0; i < r_ele_ct; i++)
  {
    /* Loop over each element in the radial direction. */

    /* Determine r, the radius at nodes 0, 1, 2 and 3; and r_plus the radius
     * at nodes 4, 5, 6 and 7. */
    r      = r_plus;
    r_plus = inner_radius + thickness * ((double)(i+1))/((double)(r_ele_ct));

    z_plus = z_min;
    for (j = 0; j < z_ele_ct; j++)
    {
      /* Loop over each element in the z direction. */

      /* Determine z, the location in z of nodes 0, 1, 4 and 5; and z_plus,
       * the location in z of nodes 2, 3, 6 and 7. */
      z      = z_plus;
      z_plus = z_min + height * ((double)(j+1))/((double)(z_ele_ct));

      /* Determine if any elements in this row are in cells 1 or 2. */
      in_cell_1_2_row = j>(z_ele_ct+1)/3-1 && j<2*(z_ele_ct+1)/3;

      theta_plus = 0.0;
      for (k = 0; k < theta_ele_ct; k++)
      {
        /* Loop over each element in the theta direction. */

        /* Determine theta, the angle of nodes 0, 3, 4 and 7. */
        theta = theta_plus;

        /* Set element connectivity, element id and global coordinate values. */

        /* Determine which cell (part) this element belongs to.  Set
         * conn_ptr to the current end of that connectivity array, set
         * the element id for that cell and increment the count of elements
         * in that cell. */
        if (in_cell_1_2_row && (k>theta_ele_ct/3 && k<theta_ele_ct-2))
	{
	  conn_ptr = cell_1_conn + cell_1_elmt_ct * nodes_per_ele;
          nodes_ptr = cell_1_nodes;
          cell_1_eids[cell_1_elmt_ct] = e;
          cell_1_elmt_ct++;
	}
        else if (in_cell_1_2_row && (k<theta_ele_ct/3-1))
	{
	  conn_ptr = cell_2_conn + cell_2_elmt_ct * nodes_per_ele;
          nodes_ptr = cell_2_nodes;
          cell_2_eids[cell_2_elmt_ct] = e;
          cell_2_elmt_ct++;
          /* We must know where the nodes at theta==0 are since the
           * nodes at theta==2pi are coincident with these. */
	  if (k == 0)
	    first_elmt_in_row_ptr = conn_ptr;
	}
        else
	{
	  conn_ptr = cell_3_conn + cell_3_elmt_ct * nodes_per_ele;
          nodes_ptr = cell_3_nodes;
          cell_3_eids[cell_3_elmt_ct] = e;
          cell_3_elmt_ct++;
          /* We must know where the nodes at theta==0 are since the
           * nodes at theta==2pi are coincident with these. */
	  if (k == 0)
	    first_elmt_in_row_ptr = conn_ptr;
	}
        /* Fill in the connectivity data and global coordinate values for
         * this element.
         * First determine x, y, x_plus and y_plus--the x and y coordinates at
         * theta and r_plus. */
        cos_theta = cos(theta);
        sin_theta = sin(theta);
        x = r * cos_theta + x_offset;
        y = r * sin_theta + y_offset;
        x_rplus = r_plus * cos_theta + x_offset;
        y_rplus = r_plus * sin_theta + y_offset;
        /* Node 0. */
        offset = 3 * p;
        conn_ptr[0] = p;
        nodes_ptr[p] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x;
        *lcoord_dof_tuple_ptr++ = y;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 3. */
        pt_num = p + theta_node_ct;
        offset = 3 * pt_num;
        conn_ptr[3] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x;
        *lcoord_dof_tuple_ptr++ = y;
        *lcoord_dof_tuple_ptr   = z_plus;
        /* Node 4. */
        pt_num = p + theta_node_ct * z_node_ct;
        offset = 3 * pt_num;
        conn_ptr[4] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplus;
        *lcoord_dof_tuple_ptr++ = y_rplus;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 7. */
        pt_num = conn_ptr[4] + theta_node_ct;
        offset = 3 * pt_num;
        conn_ptr[7] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_rplus;
        *lcoord_dof_tuple_ptr++ = y_rplus;
        *lcoord_dof_tuple_ptr   = z_plus;

        if (k == (theta_ele_ct - 1))
        {
          /* Nodes in the faces at theta_plus == 2pi are coincident with nodes
           * in the face at theta == 0. */
          conn_ptr[1] = first_elmt_in_row_ptr[0];
          conn_ptr[2] = first_elmt_in_row_ptr[3];
          conn_ptr[5] = first_elmt_in_row_ptr[4];
          conn_ptr[6] = first_elmt_in_row_ptr[7];
          nodes_ptr[first_elmt_in_row_ptr[0]] = 1;
          nodes_ptr[first_elmt_in_row_ptr[3]] = 1;
          nodes_ptr[first_elmt_in_row_ptr[4]] = 1;
          nodes_ptr[first_elmt_in_row_ptr[7]] = 1;
        }
        else
        {
          /* Nodes in the faces not at theta_plus == 2pi are unique. */

          /* Determine theta_plus, the angle of nodes 1, 2, 5 and 6 and
           * and x_thetaplus, y_thetaplus, x_rplusthetaplus, y_rplusthetaplus. */
          theta_plus = TWOPI * ((double)(k+1))/((double)theta_ele_ct);
          cos_theta_plus = cos(theta_plus);
          sin_theta_plus = sin(theta_plus);
          x_thetaplus = r * cos_theta_plus + x_offset;
          y_thetaplus = r * sin_theta_plus + y_offset;
          x_rplusthetaplus = r_plus * cos_theta_plus + x_offset;
          y_rplusthetaplus = r_plus * sin_theta_plus + y_offset;
          /* Node 1. */
          pt_num = p + 1;
          offset = 3 * pt_num;
          conn_ptr[1] = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_thetaplus;
          *lcoord_dof_tuple_ptr++ = y_thetaplus;
          *lcoord_dof_tuple_ptr   = z;
          /* Node 2. */
          pt_num = conn_ptr[3] + 1;
          offset = 3 * pt_num;
          conn_ptr[2] = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_thetaplus;
          *lcoord_dof_tuple_ptr++ = y_thetaplus;
          *lcoord_dof_tuple_ptr   = z_plus;
          /* Node 5. */
          pt_num = conn_ptr[4] + 1;
          offset = 3 * pt_num;
          conn_ptr[5] = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_rplusthetaplus;
          *lcoord_dof_tuple_ptr++ = y_rplusthetaplus;
          *lcoord_dof_tuple_ptr   = z;
          /* Node 6. */
          pt_num = conn_ptr[7] + 1;
          offset = 3 * pt_num;
          conn_ptr[6] = pt_num;
          nodes_ptr[pt_num] = 1;
          lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
          *lcoord_dof_tuple_ptr++ = x_rplusthetaplus;
          *lcoord_dof_tuple_ptr++ = y_rplusthetaplus;
          *lcoord_dof_tuple_ptr   = z_plus;
        }
        /* Move to the next element and set the global point number for
         * node 0 of that element. */
        e++;
        p++;
      }
    }
    /* Set the global point number for node 0 of the first element at the
     * next radius. */
    p += theta_node_ct;
  }
  SAF_ENSURE(cell_1_elmt_ct == cell_1_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_1_elmt_ct incorrect"));
  SAF_ENSURE(cell_2_elmt_ct == cell_2_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_2_elmt_ct incorrect"));
  SAF_ENSURE(cell_3_elmt_ct == cell_3_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_3_elmt_ct incorrect"));
  SAF_LEAVE(/*void*/);
}

/**/
static void
make_arrays_quadratic_quad(void)
{
  SAF_ENTER(make_arrays_quadratic_quad, /*void*/);
    
  int cell_1_elmt_ct = 0, cell_2_elmt_ct = 0, cell_3_elmt_ct = 0,
      j, k, e = 0, p = 0, pt_num, offset, in_cell_1_2_row,
      *conn_ptr, *first_elmt_in_row_ptr=0, *nodes_ptr;
  double x, y, z, x_plus, y_plus, z_plus, z_plus_plus,
         theta, theta_plus, theta_plus_plus, *lcoord_dof_tuple_ptr;

  /* Visit the same node (node 0) of each element in the pipe.  For this
   * quadratic element the next element in each direction is 2 nodes away.
   * Fill in the connectivity and global coordinates array for each element. */
  z_plus_plus = z_min;
  for (j = 0; j < z_ele_ct; j++)
  {
    /* Loop over each element in the z direction. */

    /* Determine z, the location in z of nodes 0, 1 and 4; z_plus, the location
     * in z of nodes 5 and 7; and z_plus_plus, the location in z of nodes 2, 3
     * and 6. */
    z           = z_plus_plus;
    z_plus      = z_min + height * ((double)(j)+.5)/((double)(z_ele_ct));
    z_plus_plus = z_min + height * ((double)(j+1))/((double)(z_ele_ct));

    /* Determine if any elements in this row are in cells 1 or 2. */
    in_cell_1_2_row =  j>(z_ele_ct+1)/3-1 && j<2*(z_ele_ct+1)/3;

    theta_plus_plus = 0.0;
    for (k = 0; k < theta_ele_ct; k++)
    {
      /* Loop over each element in the theta direction. */

      /* Determine theta, the angle of nodes 0, 3 and 7; and theta_plus, the
       * angle of nodes 4 and 6. */
      theta      = theta_plus_plus;
      theta_plus = TWOPI * ((double)(k)+.5)/((double)theta_ele_ct);

      /* Set element connectivity, element id and global coordinate values. */

      /* Determine which cell (part) this element belongs to.  Set
       * conn_ptr to the current end of that connectivity array, set
       * the element id for that cell and increment the count of elements
       * in that cell. */
      if (in_cell_1_2_row && (k>theta_ele_ct/3 && k<theta_ele_ct-2))
      {
        conn_ptr = cell_1_conn + cell_1_elmt_ct * nodes_per_ele;
        nodes_ptr = cell_1_nodes;
        cell_1_eids[cell_1_elmt_ct] = e;
        cell_1_elmt_ct++;
      }
      else if (in_cell_1_2_row && (k<theta_ele_ct/3-1))
      {
        conn_ptr = cell_2_conn + cell_2_elmt_ct * nodes_per_ele;
        nodes_ptr = cell_2_nodes;
        cell_2_eids[cell_2_elmt_ct] = e;
        cell_2_elmt_ct++;
        /* We must know where the nodes at theta==0 are since the nodes at
         * theta==2pi are coincident with these. */
        if (k == 0)
          first_elmt_in_row_ptr = conn_ptr;
      }
      else
      {
        conn_ptr = cell_3_conn + cell_3_elmt_ct * nodes_per_ele;
        nodes_ptr = cell_3_nodes;
        cell_3_eids[cell_3_elmt_ct] = e;
        cell_3_elmt_ct++;
        /* We must know where the nodes at theta==0 are since the nodes at
         * theta==2pi are coincident with these. */
        if (k == 0)
          first_elmt_in_row_ptr = conn_ptr;
      }
      /* Fill in the connectivity data and global coordinate values for
       * this element.
       * First determine x, y, x_plus and y_plus the x and y coordinates at
       * theta and theta plus. */
      x = inner_radius * cos(theta) + x_offset;
      y = inner_radius * sin(theta) + y_offset;
      x_plus = inner_radius * cos(theta_plus) + x_offset;
      y_plus = inner_radius * sin(theta_plus) + y_offset;
      /* Node 0. */
      offset = 3 * p;
      conn_ptr[0] = p;
      nodes_ptr[p] = 1;
      lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
      *lcoord_dof_tuple_ptr++ = x;
      *lcoord_dof_tuple_ptr++ = y;
      *lcoord_dof_tuple_ptr   = z;
      /* Node 3. */
      pt_num = p + theta_node_ct + theta_ele_ct;
      offset = 3 * pt_num;
      conn_ptr[3] = pt_num;
      nodes_ptr[pt_num] = 1;
      lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
      *lcoord_dof_tuple_ptr++ = x;
      *lcoord_dof_tuple_ptr++ = y;
      *lcoord_dof_tuple_ptr   = z_plus_plus;
      /* Node 4. */
      pt_num = p + 1;
      offset = 3 * pt_num;
      conn_ptr[4] = pt_num;
      nodes_ptr[pt_num] = 1;
      lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
      *lcoord_dof_tuple_ptr++ = x_plus;
      *lcoord_dof_tuple_ptr++ = y_plus;
      *lcoord_dof_tuple_ptr   = z;
      /* Node 6. */
      pt_num = conn_ptr[3] + 1;
      offset = 3 * pt_num;
      conn_ptr[6] = pt_num;
      nodes_ptr[pt_num] = 1;
      lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
      *lcoord_dof_tuple_ptr++ = x_plus;
      *lcoord_dof_tuple_ptr++ = y_plus;
      *lcoord_dof_tuple_ptr   = z_plus_plus;
      /* Node 7. */
      pt_num = p + theta_node_ct - k;
      offset = 3 * pt_num;
      conn_ptr[7] = pt_num;
      nodes_ptr[pt_num] = 1;
      lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
      *lcoord_dof_tuple_ptr++ = x;
      *lcoord_dof_tuple_ptr++ = y;
      *lcoord_dof_tuple_ptr   = z_plus;

      if (k == (theta_ele_ct - 1))
      {
        /* Nodes in the edges at theta_plus_plus == 2pi are coincident with
         * nodes in the edges at theta == 0. */
        conn_ptr[1] = first_elmt_in_row_ptr[0];
        conn_ptr[2] = first_elmt_in_row_ptr[3];
        conn_ptr[5] = first_elmt_in_row_ptr[7];
        nodes_ptr[first_elmt_in_row_ptr[0]] = 1;
        nodes_ptr[first_elmt_in_row_ptr[3]] = 1;
        nodes_ptr[first_elmt_in_row_ptr[7]] = 1;
      }
      else
      {
        /* Nodes in the faces not at theta_plus_plus == 2pi are unique. */

        /* Determine theta_plus_plus, the angle of nodes 1, 2 and 5; and
         * x_plus and y_plus, the x and y coordinate at theta_plus_plus. */
        theta_plus_plus = TWOPI * ((double)(k+1))/((double)theta_ele_ct);
        x_plus = inner_radius * cos(theta_plus_plus) + x_offset;
        y_plus = inner_radius * sin(theta_plus_plus) + y_offset;
        /* Node 1. */
        pt_num = p + 2;
        offset = 3 * pt_num;
        conn_ptr[1] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_plus;
        *lcoord_dof_tuple_ptr++ = y_plus;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 2. */
        pt_num = conn_ptr[3] + 2;
        offset = 3 * pt_num;
        conn_ptr[2] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_plus;
        *lcoord_dof_tuple_ptr++ = y_plus;
        *lcoord_dof_tuple_ptr   = z_plus_plus;
        /* Node 5. */
        pt_num = conn_ptr[7] + 1;
        offset = 3 * pt_num;
        conn_ptr[5] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x_plus;
        *lcoord_dof_tuple_ptr++ = y_plus;
        *lcoord_dof_tuple_ptr   = z_plus;
      }
      /* Move to the next element and set the global point number for
       * node 0 of that element. */
      e++;
      p += 2;
    }
    /* Set the global point number for node 0 of the first element at the
     * next row. */
    p += theta_ele_ct;
  }
  SAF_ENSURE(cell_1_elmt_ct == cell_1_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_1_elmt_ct incorrect"));
  SAF_ENSURE(cell_2_elmt_ct == cell_2_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_2_elmt_ct incorrect"));
  SAF_ENSURE(cell_3_elmt_ct == cell_3_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_3_elmt_ct incorrect"));
  SAF_LEAVE(/*void*/);
}

static void
make_arrays_linear_quad(void)
{
  SAF_ENTER_NOINIT(make_arrays_linear_quad, /*void*/);
  int cell_1_elmt_ct = 0, cell_2_elmt_ct = 0, cell_3_elmt_ct = 0,
      j, k, e = 0, pt_num, offset, in_cell_1_2_row,
      *conn_ptr, *first_elmt_in_row_ptr=0, *nodes_ptr;
  double x, y, z, z_plus, theta, theta_plus, *lcoord_dof_tuple_ptr;

  /* Visit the same node (node 0) of each element in the pipe.  For this
   * linear element the next element in each direction is 1 node away.
   * Fill in the connectivity and global coordinates array for each element. */
  z_plus = z_min;
  for (j = 0; j < z_ele_ct; j++)
  {
    /* Loop over each element in the z direction. */

    /* Determine z, the z location of nodes 0 and 1; and z_plus, the z
     * location of nodes 2 and 3. */
    z      = z_plus;
    z_plus = z_min + height * ((double)(j+1))/((double)(z_ele_ct));

    /* Determine if any elements in this row are in cells 1 or 2. */
    in_cell_1_2_row = j>(z_ele_ct+1)/3-1 && j<2*(z_ele_ct+1)/3;

    theta_plus = 0.0;
    for (k = 0; k < theta_ele_ct; k++)
    {
      /* Loop over each element in the theta direction. */

      /* Determine theta, the angle of nodes 0 and 3. */
      theta = theta_plus;

      /* Set element connectivity, element id and global coordinate values. */

      /* Determine which cell (part) this element belongs to.  Set
       * conn_ptr to the current end of that connectivity array, set
       * the element id for that cell and increment the count of elements
       * in that cell. */
      if (in_cell_1_2_row && (k>theta_ele_ct/3 && k<theta_ele_ct-2))
      {
        conn_ptr = cell_1_conn + cell_1_elmt_ct * nodes_per_ele;
        nodes_ptr = cell_1_nodes;
        cell_1_eids[cell_1_elmt_ct] = e;
        cell_1_elmt_ct++;
      }
      else if (in_cell_1_2_row && (k<theta_ele_ct/3-1))
      {
        conn_ptr = cell_2_conn + cell_2_elmt_ct * nodes_per_ele;
        nodes_ptr = cell_2_nodes;
        cell_2_eids[cell_2_elmt_ct] = e;
        cell_2_elmt_ct++;
        /* We must know where the nodes at theta==0 are since the
         * nodes at theta==2pi are coincident with these. */
        if (k == 0)
          first_elmt_in_row_ptr = conn_ptr;
      }
      else
      {
        conn_ptr = cell_3_conn + cell_3_elmt_ct * nodes_per_ele;
        nodes_ptr = cell_3_nodes;
        cell_3_eids[cell_3_elmt_ct] = e;
        cell_3_elmt_ct++;
        /* We must know where the nodes at theta==0 are since the
         * nodes at theta==2pi are coincident with these. */
        if (k == 0)
          first_elmt_in_row_ptr = conn_ptr;
      }
      /* Fill in the connectivity data and global coordinate values for
       * this element.
       * First determine x and y, the x and y coordinates at theta. */
      x = inner_radius * cos(theta) + x_offset;
      y = inner_radius * sin(theta) + y_offset;
      /* Node 0. */
      offset = 3 * e;
      conn_ptr[0] = e;
      nodes_ptr[e] = 1;
      lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
      *lcoord_dof_tuple_ptr++ = x;
      *lcoord_dof_tuple_ptr++ = y;
      *lcoord_dof_tuple_ptr   = z;
      /* Node 3. */
      pt_num = e + theta_node_ct;
      offset = 3 * pt_num;
      conn_ptr[3] = pt_num;
      nodes_ptr[pt_num] = 1;
      lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
      *lcoord_dof_tuple_ptr++ = x;
      *lcoord_dof_tuple_ptr++ = y;
      *lcoord_dof_tuple_ptr   = z_plus;

      if (k == (theta_ele_ct - 1))
      {
        /* Nodes in the faces at theta_plus == 2pi are coincident with nodes
         * in the faces at theta == 0. */
        conn_ptr[1] = first_elmt_in_row_ptr[0];
        conn_ptr[2] = first_elmt_in_row_ptr[3];
        nodes_ptr[first_elmt_in_row_ptr[0]] = 1;
        nodes_ptr[first_elmt_in_row_ptr[3]] = 1;
      }
      else
      {
        /* Nodes in the faces not at theta_plus == 2pi are unique. */

        /* Determine theta_plus, the angle of nodes 1 and 2 and x and y, the
         * x and y coordinates at those nodes. */
        theta_plus = TWOPI * ((double)(k+1))/((double)theta_ele_ct);
        x = inner_radius * cos(theta_plus) + x_offset;
        y = inner_radius * sin(theta_plus) + y_offset;
        /* Node 1. */
        pt_num = e + 1;
        offset = 3 * pt_num;
        conn_ptr[1] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x;
        *lcoord_dof_tuple_ptr++ = y;
        *lcoord_dof_tuple_ptr   = z;
        /* Node 2. */
        pt_num = conn_ptr[3] + 1;
        offset = 3 * pt_num;
        conn_ptr[2] = pt_num;
        nodes_ptr[pt_num] = 1;
        lcoord_dof_tuple_ptr    = lcoord_dof_tuple + offset;
        *lcoord_dof_tuple_ptr++ = x;
        *lcoord_dof_tuple_ptr++ = y;
        *lcoord_dof_tuple_ptr++ = z_plus;
      }
      /* Move to the next element. */
      e++;
    }
  }
  SAF_ENSURE(cell_1_elmt_ct == cell_1_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_1_elmt_ct incorrect"));
  SAF_ENSURE(cell_2_elmt_ct == cell_2_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_2_elmt_ct incorrect"));
  SAF_ENSURE(cell_3_elmt_ct == cell_3_conn_sz/nodes_per_ele, SAF_LOW_CHK_COST, ;, _saf_errmsg("cell_3_elmt_ct incorrect"));
  SAF_LEAVE(/*void*/);
}

static void
make_arrays(void)
{
  int i;

  /* Call the function that is appropriate for the type of elements
   * that the pipe is to be made from. */
  if (elmt_type == QUADRATIC_HEX)
    make_arrays_quadratic_hex();
  else if (elmt_type == LINEAR_HEX)
    make_arrays_linear_hex();
  else if (elmt_type == QUADRATIC_QUAD)
    make_arrays_quadratic_quad();
  else
    make_arrays_linear_quad();

  /* Compact the node arrays to create arrays of unique global nodes. */
  for (i = 0; i < node_ct; i++)
  {
    if (cell_1_nodes[i])
      cell_1_nodes[cell_1_node_ct++] = i;
    if (cell_2_nodes[i])
      cell_2_nodes[cell_2_node_ct++] = i;
    if (cell_3_nodes[i])
      cell_3_nodes[cell_3_node_ct++] = i;
  }

  return;
}

int
main(int argc,
     char **argv)
{
   char dbname[1024];
   hbool_t do_writes = false;
   hbool_t do_multifile = false;
   hbool_t do_interactive = false;
   int i, rank=0;
   SAF_Db *db=NULL;
   SAF_DbProps *dbprops=NULL;
   SAF_Cat nodes, elems, blocks;
   SAF_Set top, cell_1, cell_2, cell_3;
   SAF_Rel rel, trel;
   SAF_FieldTmpl coords_ftmpl, coords_ctmpl;
   SAF_FieldTmpl stress_ftmpl, stress_ctmpl;
   SAF_FieldTmpl tmp_ftmpl[6];
   SAF_Field coords, coord_compon[3], disps, disp_compons[3];
   SAF_Field stress, stress_compons[6];
   SAF_Db *topofile;
   SAF_Db *seqfiles[5];
   int          passCount, failCount;

#ifdef HAVE_PARALLEL
   /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass
      the same argc, argv to all processors. However, the MPI spec says nothing about
      what it does or might do to argc or argv. In fact, there is no "const" in the
      function prototypes for either the pointers or the things they're pointing too.
      I would rather pass NULL here and the spec says this is perfectly acceptable.
      However, that too has caused MPICH to core on certain platforms.  */
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif


      /* since we want to see whats happening make sure stdout and stderr are unbuffered */
      setbuf(stdout, NULL);
      setbuf(stderr, NULL);

      STU_ProcessCommandLine(1, argc, argv,
          "do_writes",
             "if present, issue the saf_write_xxx() calls",
             &do_writes,
          "do_multifile",
             "if present, write data to different SAF files",
             &do_multifile,
          "do_interactive",
             "if present, prompt for input parameters otherwise use defaults",
             &do_interactive,
          STU_END_OF_ARGS);
#ifdef WIN32
	/*This doesnt work in WIN32 for now. 12jan2005*/
		do_multifile=0;
#endif

      if (rank==0 && do_interactive)
      {
        /* Prompt user for parameters about the pipe. */
        read_user_input();
      }
      else
      {
        /* Assume that we're invoked as part of the test suite.
         * Set some default values. */
        elmt_type = LINEAR_QUAD;
        theta_ele_ct = 16;
        z_ele_ct = 4;
        r_ele_ct = 1;
        inner_radius = 2.0;
        z_min = -2.0;
        height = 4.0;
        x_offset = 0.0;
        y_offset = 0.0;
      }

#ifdef HAVE_PARALLEL
  MPI_Bcast(&do_writes, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&do_multifile, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&elmt_type, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&theta_ele_ct, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&z_ele_ct, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&r_ele_ct, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&inner_radius, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&thickness, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&z_min, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&height, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&x_offset, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&y_offset, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
#endif

  /* Now that all the fundamental parameters of the problem are known the
   * derived parameters, connectivity arrays, element id arrays and arrays
   * of field values may all be determined. */

  /* Compute the derived parameters about the pipe. */
  compute_derived_parameters();

  /* Create the arrays of mesh data and field values. */
  make_arrays();

  /* for convenience, set working directory to the test file directory */
  chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  saf_init(SAF_DEFAULT_LIBPROPS);

  strcpy(dbname, TEST_FILE_NAME);

  SAF_TRY_BEGIN
  {
    int abuf[] = {4};

    /* note: because we are in a try block here, all failures will send us to the one and only
       catch block at the end of this test */

    dbprops = saf_createProps_database();
    saf_setProps_Clobber(dbprops);
    db = saf_open_database(dbname,dbprops);

    /* for multifile mode, we declare one file for topology (relations) and one file for
       each time step. At present, this use case code writes only one time step */

    /* declare some supplemental files if we're in multifile mode */
    if (do_multifile)
    {
       UNLINK("testdata/topology.dsl");
       topofile = saf_open_database("testdata/topology.dsl", dbprops);
       for (i = 0; i < 5; i++)
       {  char filename[80];

	  sprintf(filename, "testdata/step_%04d.dsl", i);
          UNLINK(filename);
          seqfiles[i] = saf_open_database(filename, dbprops);
       }
    }
    else
    {
      topofile = db;
      for (i = 0; i < 5; i++)
         seqfiles[i] = db;
    }

    passCount = 0;
    failCount = 0;

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE CATEGORIES
     -----------------------------------------------------------------------------------------------------------------------------
     */
    TESTING("declaring categories");
    saf_declare_category(SAF_ALL,db, "nodes",  SAF_TOPOLOGY, 0,        &nodes);
    saf_declare_category(SAF_ALL,db, "elems",  SAF_TOPOLOGY, topo_dim, &elems);
    saf_declare_category(SAF_ALL,db, "blocks", SAF_BLOCK,    topo_dim, &blocks);
    PASSED;
    passCount += 1;

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE SETS 
     -----------------------------------------------------------------------------------------------------------------------------
     */
    TESTING("declaring sets");
    saf_declare_set(SAF_ALL, db, "TOP_CELL", topo_dim, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &top);
    saf_declare_set(SAF_ALL, db, "CELL_1",   topo_dim, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_1);
    saf_declare_set(SAF_ALL, db, "CELL_2",   topo_dim, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_2);
    saf_declare_set(SAF_ALL, db, "CELL_3",   topo_dim, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_3);
    PASSED;
    passCount += 1;

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE COLLECTIONS
     -----------------------------------------------------------------------------------------------------------------------------
     */
    TESTING("declaring collections");
    saf_declare_collection(SAF_ALL, &top,   &nodes, SAF_CELLTYPE_POINT,node_ct,         SAF_1DC(node_ct),         SAF_DECOMP_FALSE);
    saf_declare_collection(SAF_ALL, &top,   &elems, saf_elmt_type,     ele_ct,          SAF_1DC(ele_ct),          SAF_DECOMP_TRUE);
    saf_declare_collection(SAF_ALL, &top,   &blocks,SAF_CELLTYPE_SET,  3,               SAF_1DC(3),               SAF_DECOMP_TRUE);
    saf_declare_collection(SAF_ALL, &cell_1,&nodes, SAF_CELLTYPE_POINT,cell_1_node_ct,  SAF_1DC(cell_1_node_ct),  SAF_DECOMP_FALSE);
    saf_declare_collection(SAF_ALL, &cell_1,&elems, saf_elmt_type,     num_cell_1_elmts,SAF_1DC(num_cell_1_elmts),SAF_DECOMP_TRUE);
    saf_declare_collection(SAF_ALL, &cell_1,&blocks,SAF_CELLTYPE_SET,  1,               SAF_1DC(1),               SAF_DECOMP_TRUE);
    saf_declare_collection(SAF_ALL, &cell_2,&nodes, SAF_CELLTYPE_POINT,cell_2_node_ct,  SAF_1DC(cell_2_node_ct),  SAF_DECOMP_FALSE);
    saf_declare_collection(SAF_ALL, &cell_2,&elems, saf_elmt_type,     num_cell_2_elmts,SAF_1DC(num_cell_2_elmts),SAF_DECOMP_TRUE);
    saf_declare_collection(SAF_ALL, &cell_2,&blocks,SAF_CELLTYPE_SET,  1,               SAF_1DC(1),               SAF_DECOMP_TRUE);
    saf_declare_collection(SAF_ALL, &cell_3,&nodes, SAF_CELLTYPE_POINT,cell_3_node_ct,  SAF_1DC(cell_3_node_ct),  SAF_DECOMP_FALSE);
    saf_declare_collection(SAF_ALL, &cell_3,&elems, saf_elmt_type,     num_cell_3_elmts,SAF_1DC(num_cell_3_elmts),SAF_DECOMP_TRUE);
    saf_declare_collection(SAF_ALL, &cell_3,&blocks,SAF_CELLTYPE_SET,  1,               SAF_1DC(1),               SAF_DECOMP_TRUE);
    PASSED;
    passCount += 1;

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE AND WRITE SUBSET RELATIONS 
     *						                (buf in write call)
     -----------------------------------------------------------------------------------------------------------------------------
     */
    /* Note: The declare/write calls are written in the manner they are so that one can easily
       associate the data being written with the object (field or relation) it is for. If all the
       saf_write_xxx() calls are moved to the end, after the declare calls, this association would
       be harder to see */

    TESTING(do_multifile?
	       do_writes?
                  "writing subset relations [buf in write] [multifile]"
               :
                  "declaring subset relations [buf in write]"
            :
	       do_writes?
		  "writing subset relations [buf in write]"
               :
                  "declaring subset relations [buf in write]");

    /* nodes and elems of cell_1 and top */
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
      saf_write_subset_relation(SAF_ALL,&rel, H5T_NATIVE_INT, cell_1_nodes, H5I_INVALID_HID, NULL, topofile);
    free(cell_1_nodes);
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_1, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
      saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, cell_1_eids, H5I_INVALID_HID, NULL, topofile);
    free(cell_1_eids);

    /* nodes and elems of cell_2 and top */
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_2, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
      saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, cell_2_nodes, H5I_INVALID_HID, NULL, topofile);
    free(cell_2_nodes);
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_2, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
      saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, cell_2_eids, H5T_NATIVE_INT, NULL, topofile);
    free(cell_2_eids);

    /* nodes and elems of cell_3 and top */
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_3, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
      saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, cell_3_nodes, H5T_NATIVE_INT, NULL, topofile);
    free(cell_3_nodes);
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_3, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
      saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, cell_3_eids, H5T_NATIVE_INT, NULL, topofile);
    free(cell_3_eids);
    PASSED;
    passCount += 1;

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE AND WRITE SUBSET RELATIONS FOR BLOCKS
     -----------------------------------------------------------------------------------------------------------------------------
     */
    TESTING(do_multifile?
	       do_writes?
                  "writing subset relations for blocks [multifile]"
               :
                  "declaring subset relations for blocks"
            :
	       do_writes?
		  "writing subset relations for blocks"
               :
                  "declaring subset relations for blocks");

    /* now, declare the subset relations for blocks */
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_1, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
    { int buf[] = {0};
       saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5T_NATIVE_INT, NULL, topofile);
    }
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_2, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
    { int buf[] = {1};
       saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5T_NATIVE_INT, NULL, topofile);
    }
    saf_declare_subset_relation(SAF_ALL,db, &top, &cell_3, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
    if (do_writes)
    { int buf[] = {2};
       saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5T_NATIVE_INT, NULL, topofile);
    }
    PASSED;
    passCount += 1;

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE AND WRITE TOPOLOGY RELATIONS 
     -----------------------------------------------------------------------------------------------------------------------------
     */
    TESTING(do_multifile?
	       do_writes?
                  "writing topology relations [multifile]"
               :
                  "declaring topology relations"
            :
	       do_writes?
		  "writing topology relations"
               :
                  "declaring topology relations");

    /* for cell_1 */
    saf_declare_topo_relation(SAF_ALL,db, &cell_1, &elems, &top, &nodes, SAF_SELF(db), &cell_1,
       SAF_UNSTRUCTURED,  H5T_NATIVE_INT, NULL,  H5T_NATIVE_INT, NULL, &trel);
    if (do_writes)
      saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, cell_1_conn, topofile);
    free(cell_1_conn);

    /* for cell_2 */
    saf_declare_topo_relation(SAF_ALL,db, &cell_2, &elems, &top, &nodes, SAF_SELF(db), &cell_2,
       SAF_UNSTRUCTURED, H5T_NATIVE_INT, NULL,  H5T_NATIVE_INT, NULL, &trel);
    if (do_writes)
      saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, cell_2_conn, topofile);
    free(cell_2_conn);

    /* for cell_3 */
    saf_declare_topo_relation(SAF_ALL,db, &cell_3, &elems, &top, &nodes, SAF_SELF(db), &cell_3,
       SAF_UNSTRUCTURED, H5T_NATIVE_INT, NULL,  H5T_NATIVE_INT, NULL, &trel);
    if (do_writes)
      saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, cell_3_conn, topofile);
    free(cell_3_conn);
    PASSED;
    passCount += 1;

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE FIELD TEMPLATES 
     -----------------------------------------------------------------------------------------------------------------------------
     */
    TESTING("declaring field templates");

    /* for coordinate components */
    saf_declare_field_tmpl(SAF_ALL,db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
       NULL, &coords_ctmpl);

    tmp_ftmpl[0] = coords_ctmpl;
    tmp_ftmpl[1] = coords_ctmpl;
    tmp_ftmpl[2] = coords_ctmpl;

    /* for coordinate fields */
    saf_declare_field_tmpl(SAF_ALL,db, "coordinate_tmpl",  SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3,
       tmp_ftmpl, &coords_ftmpl);

    /* for stress components */
    saf_declare_field_tmpl(SAF_ALL,db, "stress_on_cell_1_ctmpl", SAF_ALGTYPE_SCALAR, SAF_UNITY,
       SAF_QNAME(db,"pressure"), 1, NULL, &stress_ctmpl);

    tmp_ftmpl[0] = stress_ctmpl;
    tmp_ftmpl[1] = stress_ctmpl;
    tmp_ftmpl[2] = stress_ctmpl;
    tmp_ftmpl[3] = stress_ctmpl;
    tmp_ftmpl[4] = stress_ctmpl;
    tmp_ftmpl[5] = stress_ctmpl;

    /* for stress */
    saf_declare_field_tmpl(SAF_ALL,db, "stress_on_cell_1_tmpl", SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI,
       SAF_QNAME(db,"pressure"), 6, tmp_ftmpl, &stress_ftmpl);

    PASSED;
    passCount += 1;

    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE AND WRITE FIELDS
     *				                        (buf specified in write call)
     -----------------------------------------------------------------------------------------------------------------------------
     */
    {
       SAF_Unit umeter;
       int num_disp_vals = node_ct * 3;
       void *pbuf = &lcoord_dof_tuple[0];

       saf_find_one_unit(db, "meter",&umeter);

       TESTING(do_multifile?
	          do_writes?
                     "writing fields [buf in write] [multifile]"
                  :
                     "declaring fields [buf in write]"
               :
	          do_writes?
		     "writing fields [buf in write]"
                  :
                     "declaring fields [buf in write]");

       /* now for some fields */
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "X", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coord_compon[0]));
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "Y", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coord_compon[1]));
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "Z", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coord_compon[2]));
       saf_declare_field(SAF_ALL,db, &coords_ftmpl, "coords at t0", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, coord_compon, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coords); 
       if (do_writes)
         saf_write_field(SAF_ALL, &coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);

       /* specify that is a coordinate field */
       saf_declare_coords(SAF_ALL, &coords);

       /* made up displacements
        * be cheap and reuse the array of coordinate values for the displacements */
       /* for time step 0 */
       for (i = 0; i < num_disp_vals; i++)
         lcoord_dof_tuple[i] = i * .001;
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dX", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[0]));
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dY", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[1]));
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dZ", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[2]));
       saf_declare_field(SAF_ALL,db, &coords_ftmpl, "displacements at t0", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, disp_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &disps);
       if (do_writes)
         saf_write_field(SAF_ALL, &disps, SAF_WHOLE_FIELD, 1,  H5I_INVALID_HID, &pbuf, seqfiles[0]);
       /* for time step 1 */
       for (i = 0; i < num_disp_vals; i++)
         lcoord_dof_tuple[i] = i * .002;
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dX", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[0]));
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dY", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[1]));
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dZ", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[2]));
       saf_declare_field(SAF_ALL,db, &coords_ftmpl, "displacements at t1", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, disp_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &disps);
       if (do_writes)
         saf_write_field(SAF_ALL, &disps, SAF_WHOLE_FIELD, 1,  H5I_INVALID_HID, &pbuf, seqfiles[0]);
       /* for time step 2 */
       for (i = 0; i < num_disp_vals; i++)
         lcoord_dof_tuple[i] = i * .003;
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dX", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[0]));
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dY", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[1]));
       saf_declare_field(SAF_ALL,db, &coords_ctmpl, "dZ", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[2]));
       saf_declare_field(SAF_ALL,db, &coords_ftmpl, "displacements at t2", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
          H5T_NATIVE_DOUBLE, disp_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &disps);
       if (do_writes)
         saf_write_field(SAF_ALL, &disps, SAF_WHOLE_FIELD, 1,  H5I_INVALID_HID, &pbuf, seqfiles[0]);
       free(lcoord_dof_tuple);
       PASSED;
       passCount += 1;
    }


    /*
     -----------------------------------------------------------------------------------------------------------------------------
     *                                                    DECLARE AND WRITE FIELDS
     *				                        (buf specified in declare call)
     -----------------------------------------------------------------------------------------------------------------------------
     */
    {
       SAF_Unit upascal;
       int num_stress_vals = ele_ct * 6;
       double *stress_dof_tuple;
       void *pbuf;

       saf_find_one_unit(db, "pascal",&upascal);

       TESTING(do_multifile?
	          do_writes?
                     "writing fields [buf in declare] [multifile]"
                  :
                     "declaring fields [buf in declare]"
               :
	          do_writes?
		     "writing fields [buf in declare]"
                  :
                     "declaring fields [buf in declare]");

       /* made up stresses */
       stress_dof_tuple = (double*)malloc(num_stress_vals * sizeof(double));
       pbuf = &stress_dof_tuple[0];
       /* at time step 0 */
       for (i = 0; i < num_stress_vals; i++)
         stress_dof_tuple[i] = i * .01;

       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxx",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[0]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Syy",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[1]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Szz",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[2]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxy",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[3]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxz",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[4]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Syz",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[5]));
       saf_declare_field(SAF_ALL,db, &stress_ftmpl, "stress at t0", &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, stress_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, &pbuf, &stress);
       if (do_writes)
          saf_write_field(SAF_ALL, &stress, SAF_WHOLE_FIELD, 0,  H5I_INVALID_HID, NULL, seqfiles[0]);
       /* at time step 1 */
       for (i = 0; i < num_stress_vals; i++)
         stress_dof_tuple[i] = i * .02;

       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxx",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[0]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Syy",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[1]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Szz",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[2]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxy",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[3]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxz",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[4]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Syz",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[5]));
       saf_declare_field(SAF_ALL,db, &stress_ftmpl, "stress at t1", &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, stress_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, &pbuf, &stress);
       if (do_writes)
          saf_write_field(SAF_ALL, &stress, SAF_WHOLE_FIELD, 0,  H5I_INVALID_HID, NULL, seqfiles[0]);
       /* at time step 2 */
       for (i = 0; i < num_stress_vals; i++)
         stress_dof_tuple[i] = i * .03;

       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxx",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[0]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Syy",   &cell_1,  &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[1]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Szz",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[2]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxy",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[3]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Sxz",    &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[4]));
       saf_declare_field(SAF_ALL,db, &stress_ctmpl, "Syz",   &cell_1,  &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, NULL,           SAF_INTERLEAVE_NONE,   SAF_IDENTITY, NULL,  &(stress_compons[5]));
       saf_declare_field(SAF_ALL,db, &stress_ftmpl, "stress at t2", &cell_1, &upascal, SAF_SELF(db),  SAF_ZONAL(&elems),
          H5T_NATIVE_DOUBLE, stress_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, &pbuf, &stress);
       if (do_writes)
          saf_write_field(SAF_ALL, &stress, SAF_WHOLE_FIELD, 0,  H5I_INVALID_HID, NULL, seqfiles[0]);

       PASSED;
       passCount += 1;

    }

    TESTING("database close");
    saf_close_database(db);
        if (do_multifile) {
            saf_close_database(topofile);
            for (i = 0; i < 5; i++) {
                saf_close_database(seqfiles[i]);
            }
        }

    PASSED;
    passCount += 1;

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

#ifdef HAVE_PARALLEL
  MPI_Finalize();
#endif

  return (failCount==0)? 0 : 1;
}
