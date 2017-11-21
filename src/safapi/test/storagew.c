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
#include <math.h>
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#include <saf.h>
#include <testutil.h>


static int numberOfDomains;
static int numberOfGhostNodes;
static int numberOfGhostZones;
static int numberOfNodes;
static int numberOfNodesPerDomain;
static int numberOfNodesPerZone;
static int numberOfZones;
static int numberOfZonesPerDomain;
static int thisDomain;

/*-------------------------------------------------------------------------
|
|   Audience:    Private
|   Chapter:     Testing
|   Purpose:     Compute global node number.
|
|   Description: This function is used to compute the global number of
|                a node number which is relative to the given domain.
|                Note that this function makes particular use of the
|                particular numbering found in this problem.
|
|   Return:      A non-negative integer is returned when the function is
|                successful, otherwise a negative integer is returned.
|
|   Parallel:    This function may be called independently.
|
|   Issues:      None.
|
|   Bugs:        No known.
|
|   Modifications:
|
+------------------------------------------------------------------------*/


static int globalNodeNumber ( int localNodeNumber
                            )
{
   return thisDomain*(numberOfNodesPerDomain - numberOfNodesPerZone*numberOfGhostZones) +
      localNodeNumber;
}


/*-------------------------------------------------------------------------
|
|   Audience:    Private
|   Chapter:     Testing
|   Purpose:     Compute global zone number.
|
|   Description: This function is used to compute the global number
|                of a zone number which is relative to the given
|                domain. Note that this function makes particular
|                use of the particular numbering found in this problem.
|
|   Return:      A non-negative integer is returned when the function is
|                successful, otherwise a negative integer is returned.
|
|   Parallel:    This function may be called independently.
|
|   Issues:      None.
|
|   Bugs:        No known.
|
|   Modifications:
|
+------------------------------------------------------------------------*/

static int globalZoneNumber ( int localZoneNumber
                            )
{
  return thisDomain*(numberOfZonesPerDomain - numberOfGhostZones) + localZoneNumber;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Tests
 * Purpose:     Write fields stored on a decomposition
 *
 * Description: This is testing code that exercises indirect fields used to represent field stored on a domain
 *              decomposition.  An indirect files is a field whose stored coeff are handles to fields. Typically
 *              such a field is on a superset and the handles are to the "same" field on the subsets.  This test
 *              generates an SAF database containing an unstructured mesh with two domains.  Indirect fields are
 *              specified on the mesh which refer to fields actually stored in fields on the domains.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Jim Reus, 13Jul2000
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
    char                dbname[1024];
    hbool_t             do_multifile = FALSE;
    int                 i, P=0, Np=1;
    int                 errorCount=0;
    SAF_Db              *db=NULL;
    SAF_DbProps         *dbprops=NULL;
    char                domainName[256];
    SAF_Cat             nodes, zones, domains;
    SAF_Set             mesh, domain;
    SAF_FieldTmpl       coord_mesh_ftmpl, coord_d_ftmpl;
    SAF_FieldTmpl       coord_d_ctmpl;
    SAF_FieldTmpl       interleaved_mesh_ftmpl, interleaved_d_ftmpl;
    SAF_FieldTmpl       interleaved_d_ctmpl;
    SAF_FieldTmpl       press_mesh_ftmpl, press_d_ftmpl;
    SAF_FieldTmpl       temp_mesh_ftmpl, temp_d_ftmpl;
    SAF_FieldTmpl       tmp_ftmpl[6];
    SAF_Field           coords_mesh;
    SAF_Field           press_mesh;
    SAF_Field           temp_mesh;
    SAF_Field           interleaved_mesh;
    SAF_Field           coords_d;
    SAF_Field           coord_d_compon[3];
    SAF_Field           interleaved_d;
    SAF_Field           interleaved_d_compon[3];
    SAF_Field           press_d;
    SAF_Field           temp_d;
    SAF_Db              *topofile=NULL;
    SAF_Db              *seqfiles[5]= {0, 0, 0, 0, 0};
    SAF_Rel             R_mesh_domain_nodes;
    SAF_Rel             R_mesh_domain_zones;
    SAF_Rel             R_mesh_domain_domains;
    SAF_Rel             T_zones_node_domain;
    SAF_Rel             T_zones_node_mesh;

#ifdef HAVE_PARALLEL
    /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass the same argc, argv to all
     * processors. However, the MPI spec says nothing about what it does or might do to argc or argv. In fact, there is no
     * "const" in the function prototypes for either the pointers or the things they're pointing too.  I would rather pass NULL
     * here and the spec says this is perfectly acceptable.  However, that too has caused MPICH to core on certain
     * platforms. */
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &P);
    MPI_Comm_size(MPI_COMM_WORLD, &Np);
#endif

    STU_ProcessCommandLine(1, argc, argv,
                           "do_multifile",
                           "if present, write data to different SAF files",
                           &do_multifile,
                           STU_END_OF_ARGS);
#ifdef WIN32
	/*This doesnt work in WIN32 for now. 12jan2005*/
		do_multifile=0;
#endif

#ifdef HAVE_PARALLEL
    MPI_Bcast(&do_multifile, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

    /*  for convenience, set working directory to the test file directory */    
    chdir(TEST_FILE_PATH);

#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    saf_init(SAF_DEFAULT_LIBPROPS);

    strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {

        /*  Note: because we are in a try block here, all failures will send us to the one and only
         *        catch block at the end of this test. */

        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);
        db = saf_open_database(dbname,dbprops);

        /*  For multifile mode, we declare one file for topology (relations) and one file for
         *  each time step. At present, this use case code writes only one time step */

        /*  Declare some supplemental files if we're in multifile mode */        

        if (do_multifile) {
            UNLINK("testdata/topology.dsl");
            topofile = saf_open_database("testdata/topology.dsl", dbprops);
            for (i = 0; i < 5; i++) {
                char filename[80];

                sprintf(filename, "testdata/step_%04d.dsl", i);
                UNLINK(filename);
                seqfiles[i] = saf_open_database(filename, dbprops);
            }
        } else {
            topofile = db;
            for (i = 0; i < 5; i++) {
                seqfiles[i] = db;
            }
        }

        /*-----------------------------------------------------------------------------------------------------------------------
         *                                                   PREPARE
         *--------------------------------------------------------------------------------------------------------------------- */
        numberOfDomains        = Np;
        numberOfZonesPerDomain = 3;
        numberOfGhostZones     = 1;
        numberOfZones          = numberOfZonesPerDomain + ((numberOfDomains-1)*(numberOfZonesPerDomain-numberOfGhostZones));
        numberOfNodesPerZone   = 8;
        numberOfGhostNodes     = 4;
        numberOfNodesPerDomain = numberOfNodesPerZone + ((numberOfZonesPerDomain-1)*(numberOfNodesPerZone-numberOfGhostNodes));
        numberOfNodes          = numberOfNodesPerZone + ((numberOfZones-1)*(numberOfNodesPerZone-numberOfGhostNodes));
        thisDomain             = P;

        /* Store the number of MPI tasks as an attribute of the database so that the reader tests know how many tasks created
         * the file. That way we can run the reader with fewer or more tasks than the writer. */
        saf_put_attribute(SAF_ALL, (ss_pers_t*)db, "ntasks", H5T_NATIVE_INT, 1, &Np);

        /*-----------------------------------------------------------------------------------------------------------------------
         *                                             DECLARE CATEGORIES
         *--------------------------------------------------------------------------------------------------------------------- */
        TESTING("declaration of categories");
        saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, SAF_TOPODIM_0D, &nodes);
        saf_declare_category(SAF_ALL, db, "zones", SAF_TOPOLOGY, SAF_TOPODIM_3D, &zones);
        saf_declare_category(SAF_ALL, db, "domains", SAF_DOMAIN, SAF_TOPODIM_3D, &domains);
        PASSED;

        /*-----------------------------------------------------------------------------------------------------------------------
         *                                                 DECLARE SETS
         *--------------------------------------------------------------------------------------------------------------------- */
        sprintf(domainName,"Domain_%05d",P);
        TESTING("declaration of sets");
        saf_declare_set(SAF_ALL, db, "TheMesh", SAF_TOPODIM_3D, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &mesh);
        saf_declare_set(SAF_EACH, db, domainName, SAF_TOPODIM_3D, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain);
        PASSED;

        /*-----------------------------------------------------------------------------------------------------------------------
         *                                                 DECLARE COLLECTIONS
         *--------------------------------------------------------------------------------------------------------------------- */
        TESTING("declaration of collections");
        saf_declare_collection(SAF_ALL, &mesh, &nodes, SAF_CELLTYPE_POINT, numberOfNodes, SAF_1DC(numberOfNodes), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &mesh, &zones, SAF_CELLTYPE_HEX, numberOfZones, SAF_1DC(numberOfZones), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &mesh, &domains, SAF_CELLTYPE_SET, numberOfDomains, SAF_1DC(numberOfDomains), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_EACH, &domain, &nodes, SAF_CELLTYPE_POINT, numberOfNodesPerDomain, SAF_1DC(numberOfNodesPerDomain), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_EACH, &domain, &zones, SAF_CELLTYPE_HEX, numberOfZonesPerDomain, SAF_1DC(numberOfZonesPerDomain), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_EACH, &domain, &domains, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);
        PASSED;

        /*-----------------------------------------------------------------------------------------------------------------------
         *                                           DECLARE SUBSET RELATIONS
         *--------------------------------------------------------------------------------------------------------------------- */
        TESTING(do_multifile?
                "declaration of the subset relations [multifile]" :
                "declaration of the subset relations");

        saf_declare_subset_relation(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &R_mesh_domain_nodes);
        saf_declare_subset_relation(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&zones), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &R_mesh_domain_zones);
        saf_declare_subset_relation(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&domains), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &R_mesh_domain_domains);

        PASSED;
      
        /*-----------------------------------------------------------------------------------------------------------------------
         *                                            WRITE SUBSET RELATIONS
         *                                            (buf in write call)
         *--------------------------------------------------------------------------------------------------------------------- */
        TESTING(do_multifile?
                "writing the subset relations [multifile]" :
                "writing the subset relations");

        {
            int *buf;
            int  n;

            buf = malloc(numberOfNodesPerDomain*sizeof(int));
            for (n=0; n<numberOfNodesPerDomain; ++n)
                buf[n] = globalNodeNumber(n);
            saf_write_subset_relation(SAF_EACH, &R_mesh_domain_nodes, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
            free(buf);
        }
        {
            int *buf;
            int  z;

            buf = malloc(numberOfZonesPerDomain*sizeof(int));
            for (z=0; z<numberOfZonesPerDomain; ++z)
                buf[z] = globalZoneNumber(z);
            saf_write_subset_relation(SAF_EACH, &R_mesh_domain_zones, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
            free(buf);
        }
        {
            int buf[1];

            buf[0] = thisDomain;
            saf_write_subset_relation(SAF_EACH, &R_mesh_domain_domains, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
        }

        PASSED;

        /*-----------------------------------------------------------------------------------------------------------------------
         *                                                 DECLARE AND WRITE TOPOLOGY RELATIONS
         *--------------------------------------------------------------------------------------------------------------------- */
        TESTING(do_multifile?
                "declaration of \"direct\" (local) topology relations [multifile]" :
                "declaration of \"direct\" (local) topology relations");

        saf_declare_topo_relation(SAF_EACH, db, &domain, &zones, &domain, &nodes, SAF_SELF(db), &domain, SAF_UNSTRUCTURED,
                                  H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &T_zones_node_domain);
        {
            SAF_Cat        glueCat;
            SAF_Set        glueSet;
            SAF_Cat        partCat;
            SAF_Set        partSet;
            SAF_RelRep     relRep;

            saf_describe_topo_relation(SAF_EACH, &T_zones_node_domain, &partSet, &partCat, &glueSet, &glueCat, NULL, &relRep,
                                       NULL);
            if (!SAF_EQUIV(&partCat, &zones)) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr,"...expected topological relation on \"%s\" to be binding \"zones\"\n",domainName);
                goto theExit;
            }
            if (!SAF_EQUIV(&partSet, &domain)) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr,"...expected topological relation on \"%s\" to be binding zones on \"%s\"\n",domainName,domainName);
                goto theExit;
            }
            if (!SAF_EQUIV(&glueCat, &nodes)) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr,"...expected topological relation on \"%s\" to be binding zones on \"%s\" using \"nodes\"\n",domainName,domainName);
                goto theExit;
            }
            if (!SAF_EQUIV(&glueSet, &domain)) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr,"...expected topological relation on \"%s\" to be binding zones on \"%s\" using \"nodes\" on \"%s\"\n",domainName,domainName,domainName);
                goto theExit;
            }
        }
        PASSED;

        TESTING(do_multifile?
                "writing of \"direct\" (local) topology relations [multifile]" :
                "writing of \"direct\" (local) topology relations");

        {
            int  abuf[1];
            int *bbuf;
            int  i,n,nn,z;

            abuf[0] = numberOfNodesPerZone;
            bbuf    = (int *)malloc(numberOfZonesPerDomain*numberOfNodesPerZone*sizeof(int));
            nn      = 0;
            i       = 0;
            for (z=0; z<numberOfZonesPerDomain; ++z) {
                for (n=0; n<numberOfNodesPerZone; ++n) {
                    bbuf[i] = nn;
                    i      += 1;
                    nn     += 1;
                }
                nn -= numberOfGhostNodes;
            }
            saf_write_topo_relation(SAF_EACH, &T_zones_node_domain, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, topofile);
            free(bbuf);
        }
        PASSED;


        {
            SAF_Rel        *allHandles;
            hbool_t        haveHandles;
            int            N;
            SAF_Rel        oneHandle;
            hbool_t        written;

            TESTING(do_multifile?
                    "saf_allgather_handles with topological relations [multifile]" :
                    "saf_allgather_handles with topological relations");

            haveHandles = FALSE;
            written     = FALSE;
            oneHandle   = T_zones_node_domain;
            if (NULL==(allHandles=(SAF_Rel*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                FAILED;
                goto giveUpTopo;
            }
            if (N != Np) {
                FAILED;
                if (P == 0)
                    fprintf(stderr,"expected %d handle%s, got %d handle%s\n"
                            ,Np,(Np==1)?"":"s"
                            ,N,(N==1)?"":"s"
                            );
                goto giveUpTopo;
            }
            if (allHandles == NULL) {
                FAILED;
                if (P == 0)
                    fprintf(stderr,"expected %d handle%s, but got a nil pointer\n"
                            ,N,(N==1)?"":"s");
                goto giveUpTopo;
            }
            if (!SAF_EQUIV(allHandles+P, &T_zones_node_domain)) {
                FAILED;
                if (P == 0) {
                    fprintf(stderr,"got wrong handle, expected handle for this domain\n");
                    goto giveUpTopo;
                }
            }
            haveHandles = TRUE;
            PASSED;

        giveUpTopo:

            TESTING(do_multifile?
                    "declaration of \"indirect\" (global) topology relations [multifile]" :
                    "declaration of \"indirect\" (global) topology relations");

            saf_declare_topo_relation(SAF_ALL, db, &mesh, &zones, &mesh, &nodes, &domains, &mesh, SAF_UNSTRUCTURED,
                                      H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &T_zones_node_mesh);

            PASSED;

            TESTING(do_multifile?
                    "writing of \"indirect\" (global) topology relations [multifile]" :
                    "writing of \"indirect\" (global) topology relations");

            if (haveHandles) {
                saf_write_topo_relation(SAF_ALL, &T_zones_node_mesh, SAF_HANDLE, allHandles, H5I_INVALID_HID, NULL, topofile);
                written = TRUE;
            }

            if (written)
                PASSED;
            else
                SKIPPED;
        }

        /*-----------------------------------------------------------------------------------------------------------------------
         *                                                 DECLARE FIELD TEMPLATES
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            char tmp[256];

            TESTING("declaring global field templates");

            /* for coordinate fields */
            saf_declare_field_tmpl(SAF_ALL, db, "coords_on_mesh_tmpl", SAF_ALGTYPE_FIELD, NULL, SAF_NOT_APPLICABLE_QUANTITY,
                                   SAF_NOT_APPLICABLE_INT, NULL, &coord_mesh_ftmpl);
            /* for interleaved version of the coordinate field */
            saf_declare_field_tmpl(SAF_ALL, db, "interleaved_on_mesh_tmpl", SAF_ALGTYPE_FIELD, NULL, SAF_NOT_APPLICABLE_QUANTITY,
                                   SAF_NOT_APPLICABLE_INT, NULL, &interleaved_mesh_ftmpl);
            /* for temperature field */
            saf_declare_field_tmpl(SAF_ALL, db, "temp_on_mesh", SAF_ALGTYPE_FIELD, NULL, SAF_NOT_APPLICABLE_QUANTITY,
                                   SAF_NOT_APPLICABLE_INT, NULL, &temp_mesh_ftmpl);
            /* for pressure field */
            saf_declare_field_tmpl(SAF_ALL, db, "press_on_mesh", SAF_ALGTYPE_FIELD, NULL, SAF_NOT_APPLICABLE_QUANTITY,
                                   SAF_NOT_APPLICABLE_INT, NULL, &press_mesh_ftmpl);
            PASSED;

            TESTING("declaring local field templates");

            /* for coordinates */
            {
                sprintf(tmp,"coords_on_d%05d_ctmpl",P);
                saf_declare_field_tmpl(SAF_EACH, db, tmp, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL, &coord_d_ctmpl);

                tmp_ftmpl[0] = coord_d_ctmpl;
                tmp_ftmpl[1] = coord_d_ctmpl;
                tmp_ftmpl[2] = coord_d_ctmpl;
                sprintf(tmp,"coords_on_d%05d_tmpl",P);
                saf_declare_field_tmpl(SAF_EACH, db, tmp, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3, tmp_ftmpl,
                                       &coord_d_ftmpl);
            }
            /* for interleaved field */
            {
                sprintf(tmp,"interleaved_on_d%05d_ctmpl",P);
                saf_declare_field_tmpl(SAF_EACH, db, tmp, SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1, NULL,
                                       &interleaved_d_ctmpl);
                tmp_ftmpl[0] = interleaved_d_ctmpl;
                tmp_ftmpl[1] = interleaved_d_ctmpl;
                tmp_ftmpl[2] = interleaved_d_ctmpl;
                sprintf(tmp,"interleaved_on_d%05d_tmpl",P);
                saf_declare_field_tmpl(SAF_EACH, db, tmp, SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 3, tmp_ftmpl,
                                       &interleaved_d_ftmpl);
            }
            /* for temperature */
            sprintf(tmp,"temp_on_domain%05d",P);
            saf_declare_field_tmpl(SAF_EACH, db, tmp, SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QTEMP, 1, NULL, &temp_d_ftmpl);
            /* for pressure */
            sprintf(tmp,"press_on_domain%05d",P);
            saf_declare_field_tmpl(SAF_EACH, db, tmp, SAF_ALGTYPE_SCALAR, SAF_UNITY, saf_find_one_quantity(db,"pressure", NULL),
                                   1, NULL, &press_d_ftmpl);
            PASSED;
        }

        /*-----------------------------------------------------------------------------------------------------------------------
         *                                                 DECLARE AND WRITE FIELDS
         *                                                 (buf specified in write call)
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            SAF_Unit ukelvin;
            SAF_Unit umeter;
            SAF_Unit upascal;

            saf_find_one_unit(db, "meter", &umeter);
            saf_find_one_unit(db, "kelvin", &ukelvin);
            saf_find_one_unit(db, "pascal", &upascal);

            /* First we'll do the (blocked) coordinate fields... */
            {
                TESTING(do_multifile?
                        "declaration of \"direct\" coord fields on each domain [multifile]" :
                        "declaration of \"direct\" coord fields on each domain");

                saf_declare_field(SAF_EACH, db, &coord_d_ctmpl, "X", &domain, &umeter, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                                  coord_d_compon+0);
                saf_declare_field(SAF_EACH, db, &coord_d_ctmpl, "Y", &domain, &umeter, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                                  coord_d_compon+1);
                saf_declare_field(SAF_EACH, db, &coord_d_ctmpl, "Z", &domain, &umeter, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                                  coord_d_compon+2);
                saf_declare_field(SAF_EACH, db, &coord_d_ftmpl, "coords", &domain, &umeter, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, coord_d_compon, SAF_BLOCKED, SAF_IDENTITY, NULL,
                                  &coords_d);
                saf_declare_coords(SAF_EACH, &coords_d);
                saf_declare_default_coords(SAF_EACH,&domain,&coords_d);
                PASSED;

                TESTING(do_multifile?
                        "writing of \"direct\" coord field on each domain [multifile]" :
                        "writing of \"direct\" coord field on each domain");
                /* Store coordinates on domain in a blocked fashion, XX...XYY...YZZ...Z */
                {
                    int    n;
                    float *Xbuf;
                    float *Ybuf;
                    float *Zbuf;
                    void  *pbuf[3];

                    Xbuf = (float *)malloc(numberOfNodesPerDomain*sizeof(float));
                    Ybuf = (float *)malloc(numberOfNodesPerDomain*sizeof(float));
                    Zbuf = (float *)malloc(numberOfNodesPerDomain*sizeof(float));
                    for (n=0; n<numberOfNodesPerDomain; ++n) {
                        int          i,j;
                        int          nn;
                        static float XX[4] = {0,1,1,0};
                        static float YY[4] = {0,0,1,1};

                        /* We can compute the coordinates of a global node
                         * in a nifty fashion due to the simple geometry of
                             * the problem: a bunch of hexes in a row. */
                        nn      = globalNodeNumber(n);
                        i       = nn % 4;
                        j       = nn / 4;
                        Xbuf[n] = XX[i];
                        Ybuf[n] = YY[i];
                        Zbuf[n] = (double)(-j);
                    }
                    pbuf[0] = Xbuf;
                    pbuf[1] = Ybuf;
                    pbuf[2] = Zbuf;
                    saf_write_field(SAF_EACH, &coords_d, SAF_WHOLE_FIELD, 3, H5I_INVALID_HID, pbuf, seqfiles[0]);
                    free(Zbuf);
                    free(Ybuf);
                    free(Xbuf);
                }
                PASSED;

                /* Store coord (handles) on the mesh.
                 * 
                 * Note that this can be done only for the composite field.  Indirect fields cannot have component fields. */
                {
                    hbool_t ok=TRUE;

                    TESTING(do_multifile?
                            "declaration of \"indirect\" coord fields on mesh [multifile]" :
                            "declaration of \"indirect\" coord fields on mesh");
                    saf_declare_field(SAF_ALL, db, &coord_mesh_ftmpl, "coords", &mesh, SAF_NOT_APPLICABLE_UNIT, &domains,
                                      SAF_NODAL(&nodes, &zones), SAF_HANDLE, NULL, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY,
                                      NULL, &coords_mesh);
                    saf_declare_coords(SAF_ALL, &coords_mesh);
                    saf_declare_default_coords(SAF_ALL, &mesh, &coords_mesh);
                    PASSED;

                    {
                        SAF_Field       *allHandles=NULL;
                        hbool_t         haveHandles=FALSE;
                        int             N;
                        SAF_Field       oneHandle=coords_d;
                        hbool_t         written=FALSE;

                        TESTING(do_multifile?
                                "saf_allgather_handles of composit coord field(s) on domain [multifile]" :
                                "saf_allgather_handles of composit coord field(s) on domain");
                        if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                            FAILED;
                            goto giveUpFieldXYZ;
                        }
                        if (N != Np) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, got %d handle%s\n"
                                        ,Np,(Np==1)?"":"s"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldXYZ;
                        }
                        if (allHandles == NULL) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, but got a nil pointer\n"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldXYZ;
                        }
                        if (!SAF_EQUIV(allHandles+thisDomain, &coords_d)) {
                            FAILED;
                            if (P == 0) {
                                fprintf(stderr,"got wrong handle for this domain\n");
                                goto giveUpFieldXYZ;
                            }
                        }
                        haveHandles = TRUE;
                        PASSED;
                    giveUpFieldXYZ:
                        TESTING(do_multifile?
                                "writing of \"indirect\" composit coord field on mesh [multifile]" :
                                "writing of \"indirect\" composit coord field on mesh");
                        if (haveHandles) {
                            void *pbuf;

                            pbuf = &(allHandles[0]);
                            saf_write_field(SAF_ALL, &coords_mesh, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
                            written = TRUE;
                        }
                        if (written)
                            PASSED;
                        else
                            SKIPPED;
                        if (allHandles != NULL)
                            free(allHandles);
                    }
                    if (ok) {
                        SAF_Field     *allHandles;
                        int            N;
                        SAF_Field      oneHandle;

                        TESTING(do_multifile?
                                "saf_allgather_handles of X coord field(s) on domain [multifile]" :
                                "saf_allgather_handles of X coord field(s) on domain");
                        oneHandle   = coord_d_compon[0];
                        allHandles  = NULL;
                        if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                            FAILED;
                            goto giveUpFieldX;
                        }
                        if (N != Np) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, got %d handle%s\n",Np,(Np==1)?"":"s",N,(N==1)?"":"s");
                            goto giveUpFieldX;
                        }
                        if (allHandles == NULL) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, but got a nil pointer\n"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldX;
                        }
                        if (!SAF_EQUIV(allHandles+thisDomain, coord_d_compon+0)) {
                            FAILED;
                            if (P == 0) {
                                fprintf(stderr,"got wrong handle for this domain\n");
                                goto giveUpFieldX;
                            }
                        }
                        PASSED;
                    giveUpFieldX:
                        if (allHandles != NULL)
                            free(allHandles);
                    }
                    if (ok) {
                        SAF_Field     *allHandles;
                        int            N;
                        SAF_Field      oneHandle;

                        TESTING(do_multifile?
                                "saf_allgather_handles of Y coord field(s) on domain [multifile]" :
                                "saf_allgather_handles of Y coord field(s) on domain");
                        oneHandle   = coord_d_compon[1];
                        allHandles  = NULL;
                        if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                            FAILED;
                            goto giveUpFieldY;
                        }
                        if (N != Np) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, got %d handle%s\n"
                                        ,Np,(Np==1)?"":"s"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldY;
                        }
                        if (allHandles == NULL) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, but got a nil pointer\n"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldY;
                        }
                        if (!SAF_EQUIV(allHandles+thisDomain, coord_d_compon+1)) {
                            FAILED;
                            if (P == 0) {
                                fprintf(stderr,"got wrong handle for this domain\n");
                                goto giveUpFieldY;
                            }
                        }
                        PASSED;
                    giveUpFieldY:
                        if (allHandles != NULL)
                            free(allHandles);
                    }
                    if (ok) {
                        SAF_Field     *allHandles;
                        int            N;
                        SAF_Field      oneHandle;

                        TESTING(do_multifile?
                                "saf_allgather_handles of Z coord field(s) on domain [multifile]" :
                                "saf_allgather_handles of Z coord field(s) on domain");
                        oneHandle   = coord_d_compon[2];
                        allHandles  = NULL;
                        if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                            FAILED;
                            goto giveUpFieldZ;
                        }
                        if (N != Np) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, got %d handle%s\n",Np,(Np==1)?"":"s",N,(N==1)?"":"s");
                            goto giveUpFieldZ;
                        }
                        if (allHandles == NULL) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, but got a nil pointer\n",N,(N==1)?"":"s");
                            goto giveUpFieldZ;
                        }
                        if (!SAF_EQUIV(allHandles+thisDomain, coord_d_compon+2)) {
                            FAILED;
                            if (P == 0) {
                                fprintf(stderr,"got wrong handle for this domain\n");
                                goto giveUpFieldZ;
                            }
                        }
                        PASSED;
                    giveUpFieldZ:
                        if (allHandles != NULL)
                            free(allHandles);
                    }
                }
            }

            /* Then we'll do the interleaved (coordinate) fields... */
            {
                TESTING(do_multifile?
                        "declaration of \"direct\" interleaved (coord) fields on each domain [multifile]" :
                        "declaration of \"direct\" interleaved (coord) fields on each domain");

                saf_declare_field(SAF_EACH, db, &interleaved_d_ctmpl, "iX", &domain, &umeter, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                                  NULL, interleaved_d_compon+0);
                saf_declare_field(SAF_EACH, db, &interleaved_d_ctmpl, "iY", &domain, &umeter, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                                  NULL, interleaved_d_compon+1);
                saf_declare_field(SAF_EACH, db, &interleaved_d_ctmpl, "iZ", &domain, &umeter, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                                  NULL, interleaved_d_compon+2);
                saf_declare_field(SAF_EACH, db, &interleaved_d_ftmpl, "interleaved", &domain, &umeter, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, interleaved_d_compon, SAF_INTERLEAVED,
                                  SAF_IDENTITY, NULL, &interleaved_d);
                saf_declare_coords(SAF_EACH, &interleaved_d);
                PASSED;

                TESTING(do_multifile?
                        "writing of \"direct\" interleaved (coord) field on each domain [multifile]" :
                        "writing of \"direct\" interleaved (coord) field on each domain");
                /* Store the coordinates on domain in a interleaved fashion, XYZXYZ...XYZ */
                {
                    int    i,n;
                    float *Xbuf;
                    float *Ybuf;
                    float *Zbuf;
                    float *XYZbuf;
                    void  *pbuf[1];

                    /*  First form blocked buffers (just like before)... */
                    Xbuf = (float *)malloc(numberOfNodesPerDomain*sizeof(float));
                    Ybuf = (float *)malloc(numberOfNodesPerDomain*sizeof(float));
                    Zbuf = (float *)malloc(numberOfNodesPerDomain*sizeof(float));
                    for (n=0; n<numberOfNodesPerDomain; ++n) {
                        int          i,j;
                        int          nn;
                        static float XX[4] = {0,1,1,0};
                        static float YY[4] = {0,0,1,1};

                        /* We can compute the coordinates of a global node in a nifty fashion due to the simple geometry of the
		      * problem: a bunch of hexes in a row. */
                        nn      = globalNodeNumber(n);
                        i       = nn % 4;
                        j       = nn / 4;
                        Xbuf[n] = XX[i];
                        Ybuf[n] = YY[i];
                        Zbuf[n] = (double)(-j);
                    }
                    /* Now re-arange to form a single interleaved buffer... */
                    XYZbuf = (float *)malloc(numberOfNodesPerDomain*3*sizeof(float));
                    i      = 0;
                    for (n=0; n<numberOfNodesPerDomain; ++n) {
                        XYZbuf[i+0] = Xbuf[n];
                        XYZbuf[i+1] = Ybuf[n];
                        XYZbuf[i+2] = Zbuf[n];
                        i          += 3;
                    }
                    pbuf[0] = XYZbuf;
                    saf_write_field(SAF_EACH, &interleaved_d, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, pbuf, seqfiles[0]);
                    free(XYZbuf);
                    free(Zbuf);
                    free(Ybuf);
                    free(Xbuf);
                }
                PASSED;

                /* Store interleaved coord (handles) on the mesh.
                 * 
	         * Note that this can be done only for the composite field. */
                {
                    hbool_t ok=TRUE;

                    TESTING(do_multifile?
                            "declaration of \"indirect\" interleaved (coord) fields on mesh [multifile]" :
                            "declaration of \"indirect\" interleaved (coord) fields on mesh");
                    saf_declare_field(SAF_ALL, db, &interleaved_mesh_ftmpl, "interleaved", &mesh, SAF_NOT_APPLICABLE_UNIT,
                                      &domains, SAF_NODAL(&nodes, &zones), SAF_HANDLE, NULL, SAF_INTERLEAVE_VECTOR,
                                      SAF_IDENTITY, NULL, &interleaved_mesh);
                    saf_declare_coords(SAF_ALL, &interleaved_mesh);
                    PASSED;

                    {
                        SAF_Field       *allHandles=NULL;
                        hbool_t         haveHandles=FALSE;
                        int             N;
                        SAF_Field       oneHandle=interleaved_d;
                        hbool_t         written;

                        TESTING(do_multifile?
                                "saf_allgather_handles of composit interleaved coord field(s) on domain [multifile]" :
                                "saf_allgather_handles of composit interleaved coord field(s) on domain");
                        if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                            FAILED;
                            goto giveUpFieldIntlvdXYZ;
                        }
                        if (N != Np) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, got %d handle%s\n"
                                        ,Np,(Np==1)?"":"s"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldIntlvdXYZ;
                        }
                        if (allHandles == NULL) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, but got a nil pointer\n"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldIntlvdXYZ;
                        }
                        if (!SAF_EQUIV(allHandles+thisDomain, &interleaved_d)) {
                            FAILED;
                            if (P == 0) {
                                fprintf(stderr,"got wrong handle for this domain\n");
                                goto giveUpFieldIntlvdXYZ;
                            }
                        }
                        haveHandles = TRUE;
                        PASSED;
                    giveUpFieldIntlvdXYZ:
                        TESTING(do_multifile?
                                "writing of \"indirect\" composit interleaved coord field on mesh [multifile]" :
                                "writing of \"indirect\" composit interleaved coord field on mesh");
                        written = FALSE;
                        if (haveHandles) {
                            void *pbuf;

                            pbuf = &(allHandles[0]);
                            saf_write_field(SAF_ALL, &interleaved_mesh, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
                            written = TRUE;
                        }
                        if (written)
                            PASSED;
                        else
                            SKIPPED;
                        if (allHandles != NULL)
                            free(allHandles);
                    }
                    if (ok) {
                        SAF_Field     *allHandles;
                        int            N;
                        SAF_Field      oneHandle;

                        TESTING(do_multifile?
                                "saf_allgather_handles of X interleaved coord field(s) on domain [multifile]" :
                                "saf_allgather_handles of X interleaved coord field(s) on domain");
                        oneHandle   = interleaved_d_compon[0];
                        allHandles  = NULL;
                        if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                            FAILED;
                            goto giveUpFieldIntlvdX;
                        }
                        if (N != Np) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, got %d handle%s\n"
                                        ,Np,(Np==1)?"":"s"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldIntlvdX;
                        }
                        if (allHandles == NULL) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, but got a nil pointer\n"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldIntlvdX;
                        }
                        if (!SAF_EQUIV(allHandles+thisDomain, interleaved_d_compon+0)) {
                            FAILED;
                            if (P == 0) {
                                fprintf(stderr,"got wrong handle for this domain\n");
                                goto giveUpFieldIntlvdX;
                            }
                        }
                        PASSED;
                    giveUpFieldIntlvdX:
                        if (allHandles != NULL)
                            free(allHandles);
                    }
                    if (ok) {
                        SAF_Field     *allHandles;
                        int            N;
                        SAF_Field      oneHandle;

                        TESTING(do_multifile?
                                "saf_allgather_handles of Y interleaved coord field(s) on domain [multifile]" :
                                "saf_allgather_handles of Y interleaved coord field(s) on domain");
                        oneHandle   = interleaved_d_compon[1];
                        allHandles  = NULL;
                        if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                            FAILED;
                            goto giveUpFieldIntlvdY;
                        }
                        if (N != Np) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, got %d handle%s\n",Np,(Np==1)?"":"s",N,(N==1)?"":"s");
                            goto giveUpFieldIntlvdY;
                        }
                        if (allHandles == NULL) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, but got a nil pointer\n"
                                        ,N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldIntlvdY;
                        }
                        if (!SAF_EQUIV(allHandles+thisDomain, interleaved_d_compon+1)) {
                            FAILED;
                            if (P == 0) {
                                fprintf(stderr,"got wrong handle for this domain\n");
                                goto giveUpFieldIntlvdY;
                            }
                        }
                        PASSED;
                    giveUpFieldIntlvdY:
                        if (allHandles != NULL)
                            free(allHandles);
                    }
                    if (ok) {
                        SAF_Field     *allHandles;
                        int            N;
                        SAF_Field      oneHandle;

                        TESTING(do_multifile?
                                "saf_allgather_handles of Z interleaved coord field(s) on domain [multifile]" :
                                "saf_allgather_handles of Z interleaved coord field(s) on domain");
                        oneHandle   = interleaved_d_compon[2];
                        allHandles  = NULL;
                        if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                            FAILED;
                            goto giveUpFieldIntlvdZ;
                        }
                        if (N != Np) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, got %d handle%s\n",Np,(Np==1)?"":"s",N,(N==1)?"":"s"
                                        );
                            goto giveUpFieldIntlvdZ;
                        }
                        if (allHandles == NULL) {
                            FAILED;
                            if (P == 0)
                                fprintf(stderr,"expected %d handle%s, but got a nil pointer\n",N,(N==1)?"":"s");
                            goto giveUpFieldIntlvdZ;
                        }
                        if (!SAF_EQUIV(allHandles+thisDomain, interleaved_d_compon+2)) {
                            FAILED;
                            if (P == 0) {
                                fprintf(stderr,"got wrong handle for this domain\n");
                                goto giveUpFieldIntlvdZ;
                            }
                        }
                        PASSED;
                    giveUpFieldIntlvdZ:
                        if (allHandles != NULL)
                            free(allHandles);
                    }
                }
            }

            /* Now for some made-up temperatures... */
            {
                SAF_Field       *allHandles;
                hbool_t         haveHandles;
                int             N;
                SAF_Field       oneHandle;
                hbool_t         written;

                TESTING(do_multifile?
                        "declaration of \"direct\" temperature field on each domain [multifile]" :
                        "declaration of \"direct\" temperature field on each domain");

                saf_declare_field(SAF_EACH, db, &temp_d_ftmpl, "temperature", &domain, &ukelvin, SAF_SELF(db),
                                  SAF_NODAL(&nodes, &zones), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                                  &temp_d);

                PASSED;

                TESTING(do_multifile?
                        "declaration of \"indirect\" temperature field on mesh [multifile]" :
                        "declaration of \"indirect\" temperature field on mesh");

                saf_declare_field(SAF_ALL, db, &temp_mesh_ftmpl, "temperature", &mesh, SAF_NOT_APPLICABLE_UNIT, &domains,
                                  SAF_NODAL(&nodes, &zones), SAF_HANDLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                                  NULL, &temp_mesh);

                PASSED;

                TESTING(do_multifile?
                        "writing of \"direct\" temperature field on each domain [multifile]" :
                        "writing of \"direct\" temperature field on each domain");
                {
                    float *buf;
                    int    n;
                    void  *pbuf;

                    buf = (float *)malloc(numberOfNodesPerDomain*sizeof(float));
                    for (n=0; n<numberOfNodesPerDomain; ++n) {
                        int          i,j;
                        int          nn;
                        static float XX[4] = {0,1,1,0};
                        static float YY[4] = {0,0,1,1};
                        float        X,Y,Z;

                        nn     = globalNodeNumber(n);
                        i      = nn % 4;
                        j      = nn / 4;
                        X      = XX[i];
                        Y      = YY[i];
                        Z      = (double)(-j);
                        buf[n] = (float)sqrt(X*X+Y*Y+Z*Z);
                    }
                    pbuf = &(buf[0]);
                    saf_write_field(SAF_EACH, &temp_d, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
                }
                PASSED;

                TESTING(do_multifile?
                        "saf_allgather_handles of temperature field(s) on domain [multifile]" :
                        "saf_allgather_handles of temperature field(s) on domain");
                haveHandles = FALSE;
                oneHandle   = temp_d;
                allHandles  = NULL;
                if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                    FAILED;
                    goto giveUpFieldT;
                }
                if (N != Np) {
                    FAILED;
                    if (P == 0)
                        fprintf(stderr,"expected %d handle%s, got %d handle%s\n",Np,(Np==1)?"":"s",N,(N==1)?"":"s"
                                );
                    goto giveUpFieldT;
                }
                if (allHandles == NULL) {
                    FAILED;
                    if (P == 0)
                        fprintf(stderr,"expected %d handle%s, but got a nil pointer\n",N,(N==1)?"":"s");
                    goto giveUpFieldT;
                }
                if (!SAF_EQUIV(allHandles+thisDomain, &temp_d)) {
                    FAILED;
                    if (P == 0) {
                        fprintf(stderr,"got wrong handle for this domain\n");
                        goto giveUpFieldT;
                    }
                }
                haveHandles = TRUE;
                PASSED;

            giveUpFieldT:

                TESTING(do_multifile?
                        "writing of \"indirect\" temperature field on mesh [multifile]"
                        :
                        "writing of \"indirect\" temperature field on mesh");
                written = FALSE;
                if (haveHandles) {
                    void *pbuf;

                    pbuf = &(allHandles[0]);

                    /* arbitrarily set one of the entries to SAF_NOT_APPLICABLE_FIELD to set case where some pieces of indirection
                    * are NOT defined */
                    allHandles[0] = *SAF_NOT_APPLICABLE_FIELD;
                    saf_write_field(SAF_ALL, &temp_mesh, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
                    written = TRUE;
                }
                if (written)
                    PASSED;
                else
                    SKIPPED;
                if (allHandles != NULL)
                    free(allHandles);
            }

            /* Then some made-up pressures... */
            {
                SAF_Field       *allHandles;
                hbool_t         haveHandles;
                int             N;
                SAF_Field       oneHandle;
                hbool_t         written;

                TESTING(do_multifile?
                        "declaration of \"direct\" pressure field on each domain [multifile]" :
                        "declaration of \"direct\" pressure field on each domain");

                saf_declare_field(SAF_EACH, db, &press_d_ftmpl, "pressure", &domain, &upascal, SAF_SELF(db),
                                  SAF_ZONAL(&zones), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                                  NULL, &press_d);

                PASSED;

                TESTING(do_multifile?
                        "declaration of \"indirect\" pressure field on mesh [multifile]" :
                        "declaration of \"indirect\" pressure field on mesh");

                saf_declare_field(SAF_ALL, db, &press_mesh_ftmpl, "pressure", &mesh, SAF_NOT_APPLICABLE_UNIT, &domains,
                                  SAF_ZONAL(&zones), SAF_HANDLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY,
                                  NULL, &press_mesh);

                PASSED;

                TESTING(do_multifile?
                        "writing of \"direct\" pressure field on each domain [multifile]" :
                        "writing of \"direct\" pressure field on each domain");
                {
                    float *buf;
                    void  *pbuf;
                    int    z;

                    buf = (float *)malloc(numberOfZonesPerDomain*sizeof(float));
                    for (z=0; z<numberOfZonesPerDomain; ++z)
                        buf[z] = globalZoneNumber(z);
                    pbuf = &(buf[0]);
                    saf_write_field(SAF_EACH, &press_d, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
                }
                PASSED;

                TESTING(do_multifile?
                        "saf_allgather_handles of pressure field(s) on domain [multifile]" :
                        "saf_allgather_handles of pressure field(s) on domain");
                haveHandles = FALSE;
                oneHandle   = press_d;
                allHandles  = NULL;
                if (NULL==(allHandles=(SAF_Field*)saf_allgather_handles((ss_pers_t*)&oneHandle, &N, NULL))) {
                    FAILED;
                    goto giveUpFieldP;
                }
                if (N != Np) {
                    FAILED;
                    if (P == 0)
                        fprintf(stderr,"expected %d handle%s, got %d handle%s\n",Np,(Np==1)?"":"s",N,(N==1)?"":"s");
                    goto giveUpFieldP;
                }
                if (allHandles == NULL) {
                    FAILED;
                    if (P == 0)
                        fprintf(stderr,"expected %d handle%s, but got a nil pointer\n",N,(N==1)?"":"s");
                    goto giveUpFieldP;
                }
                if (!SAF_EQUIV(allHandles+thisDomain, &press_d)) {
                    FAILED;
                    if (P == 0) {
                        fprintf(stderr,"got wrong handle for this domain\n");
                        goto giveUpFieldP;
                    }
                }
                haveHandles = TRUE;
                PASSED;

            giveUpFieldP:

                TESTING(do_multifile?
                        "writing of \"indirect\" pressure field on mesh [multifile]"
                        :
                        "writing of \"indirect\" pressure field on mesh");
                written = FALSE;
                if (haveHandles) {
                    void *pbuf;

                    pbuf = &(allHandles[0]);
                    saf_write_field(SAF_ALL, &press_mesh, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
                    written = TRUE;
                }
                if (written)
                    PASSED;
                else
                    SKIPPED;
                if (allHandles != NULL)
                    free(allHandles);
            }
        }

    theExit:

        TESTING("database close");
        saf_close_database(db);

        if (do_multifile) {
            saf_close_database(topofile);
            for (i=0; i<5; i++) {
                saf_close_database(seqfiles[i]);
            }
        }
        PASSED;
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            errorCount += 1;
        }
    } SAF_TRY_END;

    TESTING("saf_final");
    saf_final();
    PASSED;

#ifdef HAVE_PARALLEL
    if (P == 0)
        printf("%s [on %03d procs] %-93s", "Testing",Np,"MPI broadcast of error status");
    fflush(stdout);

    /* make sure everyone returns the same error status */
    MPI_Bcast(&errorCount, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (P == 0)
        puts("        PASSED");
    fflush(stdout);
    MPI_Finalize();
#endif

    return (0 < errorCount)? 1 : 0;
}

