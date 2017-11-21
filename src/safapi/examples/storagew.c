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

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Storagew
 *
 * Description: This is code that demonstrates indirect fields used to
 *              represent fields stored on a domain decomposition.  An
 *              indirect field is a field whose stored coefficients are
 *              handles to fields.  Typically such a field is on a superset
 *              and the handles are to the "same" field on the subsets.  This
 *              example generates a SAF database containing an unstructured
 *              mesh with two domains.  Indirect fields are specified on the
 *              mesh which refer to fields actually stored in fields on the
 *              domains.
 *              
 *              Parallel and serial behavior are identical due to use of
 *              SAF_ALL mode in all calls.
 *-----------------------------------------------------------------------------
 */
SAF_Db *db=NULL;                    /* Handle to the SAF database. */
SAF_Field coord_d0_compon[3], /* Handle to the coordinate component fields
                               * on domain0. */
          coords_d0,          /* Handle to the coordinate field on domain0. */
          coord_d1_compon[3], /* Handle to the coordinate component fields
                               * on domain1. */
          coords_d1,          /* Handle to the coordinate field on domain1. */
          temp_d0,            /* Handle to the temperature field on domain0. */
          temp_d1;            /* Handle to the temperature field on domain1. */
SAF_Cat nodes,                /* Handle to the node category. */
        zones,                /* Handle to the zones category. */
        domains;              /* Handle to the domain category. */
SAF_Set mesh,                 /* Handle to the whole set. */
        domain0,              /* Handle to the domain0 subset of the mesh. */
        domain1;              /* Handle to the domain1 subset of the mesh. */


/*
 * Prototypes
 */
void make_base_space(void);
void make_direct_coord_field(void);
void make_indirect_coord_field(void);
void make_direct_temperature_field(void);
void make_indirect_temperature_field(void);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Storagew
 * Purpose:     Create mesh
 *
 * Description: Constructs the mesh for storagew and writes it to the file.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_base_space(void)
{
   SAF_Rel rel,                   /* Handle to the subset relations. */
           topoRelationOnDomain0, /* Handles to the topological relations. */
           topoRelationOnDomain1,
           topoRelationOnMesh,
           abuf[2];
   /* Arrays of the inclusion mappings. */
   int domain0_nodes[] = {0,1,4,3,6,7,10,9},
       domain0_zones[] = {0},
       domain0_domains[] = {0},
       domain1_nodes[] = {1,2,5,4,7,8,11,10},
       domain1_zones[] = {1},
       domain1_domains[] = {1},
       domain0_element_node_ct[] = {8},
       domain0_connectivity[] = {0,1,2,3,4,5,6,7},
       domain1_element_node_ct[] = {8},
       domain1_connectivity[] = {0,1,2,3,4,5,6,7};

   /*
    ---------------------------------------------------------------------------
    *                            DECLARE CATEGORIES
    ---------------------------------------------------------------------------
    */
   saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, SAF_TOPODIM_0D, &nodes);
   saf_declare_category(SAF_ALL, db, "zones", SAF_TOPOLOGY, SAF_TOPODIM_3D, &zones);
   saf_declare_category(SAF_ALL, db, "domains", SAF_DOMAIN, SAF_TOPODIM_3D, &domains);

   /*
    ---------------------------------------------------------------------------
    *                               DECLARE SETS
    ---------------------------------------------------------------------------
    */
   saf_declare_set(SAF_ALL, db, "TheMesh",    SAF_TOPODIM_3D, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &mesh);
   saf_declare_set(SAF_ALL, db, "Domain_000", SAF_TOPODIM_3D, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &domain0);
   saf_declare_set(SAF_ALL, db, "Domain_001", SAF_TOPODIM_3D, SAF_SPACE,
                   SAF_EXTENDIBLE_FALSE, &domain1);

   /*
    ---------------------------------------------------------------------------
    *                           DECLARE COLLECTIONS
    ---------------------------------------------------------------------------
    */
   /* Collections of nodes, zones and domains in the mesh. */
   saf_declare_collection(SAF_ALL, &mesh,    &nodes,   SAF_CELLTYPE_POINT, 12,
                          SAF_1DC(12), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &mesh,    &zones,   SAF_CELLTYPE_HEX,   2,
                          SAF_1DC(2),  SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &mesh,    &domains, SAF_CELLTYPE_SET,   2,
                          SAF_1DC(2),  SAF_DECOMP_TRUE);

   /* Collections of nodes, zones and domains in domain0. */
   saf_declare_collection(SAF_ALL, &domain0, &nodes,   SAF_CELLTYPE_POINT, 8,
                          SAF_1DC(8),  SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &domain0, &zones,   SAF_CELLTYPE_HEX,   1,
                          SAF_1DC(1),  SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &domain0, &domains, SAF_CELLTYPE_SET,   1,
                          SAF_1DC(1),  SAF_DECOMP_TRUE);

   /* Collections of nodes, zones and domains in domain1. */
   saf_declare_collection(SAF_ALL, &domain1, &nodes,   SAF_CELLTYPE_POINT, 8,
                          SAF_1DC(8),  SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &domain1, &zones,   SAF_CELLTYPE_HEX,   1,
                          SAF_1DC(1),  SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &domain1, &domains, SAF_CELLTYPE_SET,   1,
                          SAF_1DC(1),  SAF_DECOMP_TRUE);

   /*
    ---------------------------------------------------------------------------
    *                    DECLARE AND WRITE SUBSET RELATIONS
    ---------------------------------------------------------------------------
    */
   /* nodes, zones and domains of domain 0 in nodes and zones of the mesh */
   saf_declare_subset_relation(SAF_ALL, db, &mesh, &domain0, SAF_COMMON(&nodes),
                               SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &domain0_nodes,
                             H5I_INVALID_HID, NULL, db);
   saf_declare_subset_relation(SAF_ALL, db, &mesh, &domain0, SAF_COMMON(&zones),
                               SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &domain0_zones,
                             H5I_INVALID_HID, NULL, db);
   saf_declare_subset_relation(SAF_ALL, db, &mesh, &domain0, SAF_COMMON(&domains),
                               SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &domain0_domains,
                             H5I_INVALID_HID, NULL, db);

   /* nodes, zones and domains of domain 1 in nodes and zones of the mesh */
   saf_declare_subset_relation(SAF_ALL, db, &mesh, &domain1, SAF_COMMON(&nodes),
                               SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &domain1_nodes,
                             H5I_INVALID_HID, NULL, db);
   saf_declare_subset_relation(SAF_ALL, db, &mesh, &domain1, SAF_COMMON(&zones),
                               SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &domain1_zones,
                             H5I_INVALID_HID, NULL, db);
   saf_declare_subset_relation(SAF_ALL, db, &mesh, &domain1, SAF_COMMON(&domains),
                               SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
   saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &domain1_domains,
                             H5I_INVALID_HID, NULL, db);

   /*
    ---------------------------------------------------------------------------
    *                   DECLARE AND WRITE TOPOLOGY RELATIONS
    ---------------------------------------------------------------------------
    */

   /* The connectivity data will be passed in the write call for all
    * topological relations. */

   /* connectivity of the nodes of domain0 in the zones of domain0 */
   saf_declare_topo_relation(SAF_ALL, db, &domain0, &zones, &domain0, &nodes,
                             SAF_SELF(db), &domain0, SAF_UNSTRUCTURED,
                             H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &topoRelationOnDomain0);
   saf_write_topo_relation(SAF_ALL, &topoRelationOnDomain0, SAF_INT,
                           domain0_element_node_ct, SAF_INT,
                           domain0_connectivity, db);

   /* connectivity of the nodes of domain1 in the zones of domain0 */
   saf_declare_topo_relation(SAF_ALL, db, &domain1, &zones, &domain1, &nodes,
                             SAF_SELF(db), &domain1, SAF_UNSTRUCTURED,
                             H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &topoRelationOnDomain1);
   saf_write_topo_relation(SAF_ALL, &topoRelationOnDomain1, SAF_INT,
                           domain1_element_node_ct, SAF_INT,
                           domain1_connectivity, db);

   /* connectivity of the nodes of the mesh in the zones of the mesh declared
    * indirectly through the 2 topological relations just declared */
   saf_declare_topo_relation(SAF_ALL, db, &mesh,    &zones, &mesh,    &nodes,
                             &domains,      &mesh,    SAF_UNSTRUCTURED,
                             H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &topoRelationOnMesh);
   abuf[0] = topoRelationOnDomain0;
   abuf[1] = topoRelationOnDomain1;
   saf_write_topo_relation(SAF_ALL, &topoRelationOnMesh,    SAF_HANDLE,
                           abuf,                    H5I_INVALID_HID,
                           NULL,                 db);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Storagew
 * Purpose:     Create direct coordinate field
 *
 * Description: Creates the global coordinate field defined directly on the two domains.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_direct_coord_field(void)
{
   SAF_Unit umeter;              /* Handle to the units for the coordinates. */
   SAF_FieldTmpl coord_d0_ftmpl, /* Handle to the coordinate field field
                                  * template on domain0. */
                 coord_d0_ctmpl, /* Handle to the coordinate field component
                                  * template on domain0. */
                 coord_d1_ftmpl, /* Handle to the coordinate field field
                                  * template on domain1. */
                 coord_d1_ctmpl, /* Handle to the coordinate field component
                                  * template on domain1. */
		 tmp_ftmpl[3];   /* temporarly field template handles for
				  * component fields. */

   /* Coordinate values. */
   float domain0_Xcoords[] = { 0., 1., 1., 0., 0., 1., 1., 0. },
         domain0_Ycoords[] = { 0., 0., 0., 0., 1., 1., 1., 1. },
         domain0_Zcoords[] = { 0., 0., 1., 1., 0., 0., 1., 1. },
         domain1_Xcoords[] = { 1., 2., 2., 1., 1., 2., 2., 1. },
         domain1_Ycoords[] = { 0., 0., 0., 0., 1., 1., 1., 1. },
         domain1_Zcoords[] = { 0., 0., 1., 1., 0., 0., 1., 1. };
   void *domain0_coords[3],
        *domain1_coords[3];
   domain0_coords[0] = domain0_Xcoords;
   domain0_coords[1] = domain0_Ycoords;
   domain0_coords[2] = domain0_Zcoords;
   domain1_coords[0] = domain1_Xcoords;
   domain1_coords[1] = domain1_Ycoords;
   domain1_coords[2] = domain1_Zcoords;

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES
    ---------------------------------------------------------------------------
    */
   /* for domain0 */
   saf_declare_field_tmpl(SAF_ALL, db, "coords_on_d0_ctmpl",   SAF_ALGTYPE_SCALAR,
                          SAF_CARTESIAN, SAF_QLENGTH, 1,
                          NULL, &coord_d0_ctmpl);

   tmp_ftmpl[0] = coord_d0_ctmpl;
   tmp_ftmpl[1] = coord_d0_ctmpl;
   tmp_ftmpl[2] = coord_d0_ctmpl;
   saf_declare_field_tmpl(SAF_ALL, db, "coords_on_d0_ctmpl",   SAF_ALGTYPE_VECTOR,
                          SAF_CARTESIAN, SAF_QLENGTH, 3,
                          tmp_ftmpl, &coord_d0_ftmpl);

   /* for domain1 */
   saf_declare_field_tmpl(SAF_ALL, db, "coords_on_d1_ctmpl",   SAF_ALGTYPE_SCALAR,
                          SAF_CARTESIAN, SAF_QLENGTH, 1,
                          NULL, &coord_d1_ctmpl);

   tmp_ftmpl[0] = coord_d1_ctmpl;
   tmp_ftmpl[1] = coord_d1_ctmpl;
   tmp_ftmpl[2] = coord_d1_ctmpl;
   saf_declare_field_tmpl(SAF_ALL, db, "coords_on_d1_tmpl",   SAF_ALGTYPE_VECTOR,
                          SAF_CARTESIAN, SAF_QLENGTH, 3,
                          tmp_ftmpl,   &coord_d1_ftmpl);


   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    ---------------------------------------------------------------------------
    */
   /* Get a handle to the units for this field. */
   saf_find_one_unit(db, "meter", &umeter);

   /* Declare the fields on domain0. */
   saf_declare_field(SAF_ALL, db, &coord_d0_ctmpl, "X",      &domain0, &umeter, SAF_SELF(db),
                     SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     NULL,            SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &(coord_d0_compon[0]));
   saf_declare_field(SAF_ALL, db, &coord_d0_ctmpl, "Y",      &domain0, &umeter, SAF_SELF(db),
                     SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     NULL,            SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &(coord_d0_compon[1]));
   saf_declare_field(SAF_ALL, db, &coord_d0_ctmpl, "Z",      &domain0, &umeter, SAF_SELF(db),
                     SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     NULL,            SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &(coord_d0_compon[2]));
   saf_declare_field(SAF_ALL, db, &coord_d0_ftmpl, "coords", &domain0, &umeter, SAF_SELF(db),
                     SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     coord_d0_compon, SAF_BLOCKED,         SAF_IDENTITY, NULL,
                     &coords_d0);

   /* Write the field on domain0. */
   saf_write_field(SAF_ALL, &coords_d0, SAF_WHOLE_FIELD, 3, H5I_INVALID_HID,
                   domain0_coords, db);

   /* specify that it is a coordinate field */
   saf_declare_coords(SAF_ALL, &coords_d0);
   saf_declare_default_coords(SAF_ALL,&domain0,&coords_d0);

   /* Declare the fields on domain1. */
   saf_declare_field(SAF_ALL, db, &coord_d1_ctmpl, "X",      &domain1, &umeter, SAF_SELF(db),
                     SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     NULL,            SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &(coord_d1_compon[0]));
   saf_declare_field(SAF_ALL, db, &coord_d1_ctmpl, "Y",      &domain1, &umeter, SAF_SELF(db),
                     SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     NULL,            SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &(coord_d1_compon[1]));
   saf_declare_field(SAF_ALL, db, &coord_d1_ctmpl, "Z",      &domain1, &umeter, SAF_SELF(db),
                     SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     NULL,            SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                     &(coord_d1_compon[2]));
   saf_declare_field(SAF_ALL, db, &coord_d1_ftmpl, "coords", &domain1, &umeter, SAF_SELF(db),
                     SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     coord_d1_compon, SAF_BLOCKED,         SAF_IDENTITY, NULL,
                     &coords_d1);

   /* Write the fields on domain1. */
   saf_write_field(SAF_ALL, &coords_d1, SAF_WHOLE_FIELD, 3, H5I_INVALID_HID,
                   domain1_coords, db);

   /* specify that it is a coordinate field */
   saf_declare_coords(SAF_ALL, &coords_d1);
   saf_declare_default_coords(SAF_ALL,&domain1,&coords_d1);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Storagew
 * Purpose:     Create indirect coordinate field
 *
 * Description: Creates the global coordinate field on the mesh but defined indirectly on the two domains.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_indirect_coord_field(void)
{
   SAF_FieldTmpl coord_mesh_ftmpl; /* Handle to the coordinate field template
                                    * on the mesh. */
   SAF_Field coords_mesh,          /* Handle to the coordinate field on the
                                    * mesh. */
             buf[2];
   void *pbuf = &buf[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES
    ---------------------------------------------------------------------------
    */
    saf_declare_field_tmpl(SAF_ALL, db, "coords_on_mesh_tmpl", SAF_ALGTYPE_FIELD,
                           NULL, SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT,
                           NULL, &coord_mesh_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    ---------------------------------------------------------------------------
    */

   /* the coordinate field on the mesh */
   saf_declare_field(SAF_ALL, db, &coord_mesh_ftmpl, "coords", &mesh, SAF_NOT_APPLICABLE_UNIT, &domains,
                     SAF_NODAL(&nodes, &zones), SAF_HANDLE,
                     NULL, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY,
                     NULL, &coords_mesh);
   buf[0] = coords_d0;
   buf[1] = coords_d1;
   saf_write_field(SAF_ALL, &coords_mesh,          SAF_WHOLE_FIELD, 1, H5I_INVALID_HID,
                   &pbuf, db);

   /* specify that it is a coordinate field */
   saf_declare_coords(SAF_ALL, &coords_mesh);
   saf_declare_default_coords(SAF_ALL,&mesh,&coords_mesh);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Storagew
 * Purpose:     Create direct temperature field
 *
 * Description: Creates the temperature field defined directly on the two domains.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_direct_temperature_field(void)
{
   SAF_Unit ukelvin;            /* Handle to the units for the field. */
   SAF_FieldTmpl temp_d0_ftmpl, /* Handle to the field template on domain0. */
                 temp_d1_ftmpl; /* Handle to the field template on domain1. */
   /* Temperature values. */
   float domain0_temps[] = { 0., 1., 1.414213562373095,
                             1., 1., 1.414213562373095,
                             1.732050807568877,
                             1.414213562373095},
         domain1_temps[] = { 1., 2., 2.23606797749979,
                             1.414213562373095,
                             1.414213562373095,
                             2.23606797749979,
                             2.449489742783177,
                             1.732050807568877};
   void *pdomain0_temps = &domain0_temps[0],
        *pdomain1_temps = &domain1_temps[0];

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES
    ---------------------------------------------------------------------------
    */
   /* on domain0 */
   saf_declare_field_tmpl(SAF_ALL, db, "temp_on_domain0", SAF_ALGTYPE_SCALAR,
                          SAF_UNITY, SAF_QTEMP, 1, NULL, &temp_d0_ftmpl);

   /* on domain1 */
   saf_declare_field_tmpl(SAF_ALL, db, "temp_on_domain1",  SAF_ALGTYPE_SCALAR,
                          SAF_UNITY, SAF_QTEMP, 1, NULL, &temp_d1_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    ---------------------------------------------------------------------------
    */
   /* Get the handle to the units for the field. */
   saf_find_one_unit(db, "kelvin", &ukelvin);

   /* on domain0 */
   saf_declare_field(SAF_ALL, db, &temp_d0_ftmpl, "temperature", &domain0, &ukelvin,
                     SAF_SELF(db), SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &temp_d0);
   saf_write_field(SAF_ALL, &temp_d0, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID,
                   &pdomain0_temps, db);

   /* on domain1 */
   saf_declare_field(SAF_ALL, db, &temp_d1_ftmpl, "temperature", &domain1, &ukelvin,
                     SAF_SELF(db), SAF_NODAL(&nodes, &zones), SAF_FLOAT,
                     NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &temp_d1);
   saf_write_field(SAF_ALL, &temp_d1, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID,
                   &pdomain1_temps, db);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Storagew
 * Purpose:     Create indirect temperature field
 *
 * Description: Creates the temperature field on the mesh but defined indirectly on the two domains.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void make_indirect_temperature_field(void)
{
   SAF_FieldTmpl temp_mesh_ftmpl;  /* Handle to the field template on
                                    * the mesh. */
   SAF_Field temp_mesh,            /* Handle to the field on the mesh. */
             buf[2];
   void *pbuf = &buf[0];

   buf[0] = temp_d0;
   buf[1] = temp_d1;
   /*
    ---------------------------------------------------------------------------
    *                         DECLARE FIELD TEMPLATES
    ---------------------------------------------------------------------------
    */
   saf_declare_field_tmpl(SAF_ALL, db, "temp_on_mesh",    SAF_ALGTYPE_FIELD,
                          NULL, SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &temp_mesh_ftmpl);

   /*
    ---------------------------------------------------------------------------
    *                         DECLARE AND WRITE FIELDS
    ---------------------------------------------------------------------------
    */

   saf_declare_field(SAF_ALL, db, &temp_mesh_ftmpl, "temperature", &mesh, SAF_NOT_APPLICABLE_UNIT,
                     &domains, SAF_NODAL(&nodes, &zones), SAF_HANDLE, NULL,
                     SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &temp_mesh);
   saf_write_field(SAF_ALL, &temp_mesh, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID,
                   &pbuf, db);

   return;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Storagew
 * Purpose:     Main entry point
 *
 * Description: See [Storagew]
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc,
     char **argv)
{
  char dbname[1024] = "storagew.saf"; /* Name of the SAF database file to be created. */
  int rank=0;        /* Rank of this process for parallel. */
  SAF_DbProps *dbprops;/* Handle to the SAF databsae properties. */
  int  failed=0;

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

  /*  for convenience, set working directory to the test file directory */
  chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  /* Initialize the library. */
  saf_init(SAF_DEFAULT_LIBPROPS);

  SAF_TRY_BEGIN
    {

      /* Because we are in a try block here, all failures will send us to
       * the one and only catch block at the end of this test. */

      /* Create the database properties. */
      dbprops = saf_createProps_database();

      /* Set the clobber database property so any existing file
       * will be overwritten. */
      saf_setProps_Clobber(dbprops);

      /* Open the SAF database. Give it name dbname, properties p and
       * set db to be a handle to this database. */
      db = saf_open_database(dbname,dbprops);

      /* Construct the base space. */
      make_base_space();

      /* Construct the direct coordinate field on the domains. */
      make_direct_coord_field();

      /* Construct the indirect coordinate field on the mesh. */
      make_indirect_coord_field();

      /* Construct the direct temperature field on the domains. */
      make_direct_temperature_field();

      /* Construct the indirect temperature field on the mesh. */
      make_indirect_temperature_field();

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
