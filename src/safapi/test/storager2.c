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
#include <safP.h>
#include <testutil.h>

static int numberOfDomains;
static int numberOfZonesPerDomain;
static int numberOfGhostZones;
static int numberOfZones;
static int numberOfNodesPerZone;
static int numberOfGhostNodes;
static int numberOfNodesPerDomain;
static int numberOfNodes;

/*-------------------------------------------------------------------------
| Audience:    Private
| Chapter:     Testing
| Purpose:     Compute global node number.
|
| Description: This function is used to compute the global number of
|              a node number which is relative to the given domain.
|              Note that this function makes particular use of the
|              particular numbering found in this problem.
|
| Return:      A non-negative integer is returned when the function is
|              successful, otherwise a negative integer is returned.
|
| Parallel:    This function may be called independently.
|
| Issues:      There is clearly to be a more efficient way of computing
|              this however efficiency is not particularly important in
|              this case.
+------------------------------------------------------------------------*/
static int
globalNodeNumber(int domainNumber, int localNodeNumber)
{
    int d, n, nn;

    nn = 0;
    for (d=0; d<numberOfDomains; ++d) {
        for (n=0; n<numberOfNodesPerDomain; ++n) {
            if (n == localNodeNumber && d == domainNumber)
                goto foundIt;
            nn += 1;
        }
        nn -= numberOfNodesPerZone * numberOfGhostZones;
    }
    nn = -1;
foundIt:
    return nn;
}

/*-------------------------------------------------------------------------
| Audience:    Private
| Chapter:     Testing
| Purpose:     Compute global zone number.
|
| Description: This function is used to compute the global number
|              of a zone number which is relative to the given
|              domain. Note that this function makes particular
|              use of the particular numbering found in this problem.
|
| Return:      A non-negative integer is returned when the function is
|              successful, otherwise a negative integer is returned.
|
| Parallel:    This function may be called independently.
|
| Issues:      There is clearly to be a more efficient way of computing
|              this however efficiency is not particularly important in
|              this case.
+------------------------------------------------------------------------*/
static int
globalZoneNumber(int domainNumber, int localZoneNumber)
{
    int d, z, zz;

    zz = 0;
    for (d=0; d<numberOfDomains; ++d) {
        for (z=0; z<numberOfZonesPerDomain; ++z) {
            if (z == localZoneNumber && d == domainNumber)
                goto foundIt;
            zz += 1;
        }
        zz -= numberOfGhostZones;
    }
    zz = -1;
foundIt:
    return zz;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Tests
 * Purpose:     Read fields stored on decomposition
 *
 * Description: This is testing code that exercises indirect fields used to represent fields stored on a domain
 *              decomposition.  An indirect files is a field whose stored coeff are handles to fields. Typically such
 *              a field is on a superset and the handles are to the "same" field on the subsets.  This particular
 *              test attempts to read and analyze the file(s) generated by the "write fields on a decomposition"
 *              test (storagew).
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Jim Reus, 13Jul2000
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
    char          dbname[1024];
    int           do_multifile = false;
    int           errorCount=0;
    int           P, Np, NpWriters;
    SAF_Db        *db=NULL;
    SAF_DbProps   *dbprops=NULL;
    SAF_Cat       nodes, zones, domains;
    SAF_Set       mesh, domain;
    SAF_Field     coords_mesh, coords_d;
    SAF_Field     coord_x_mesh;
    SAF_Field     coord_y_mesh;
    SAF_Field     coord_z_mesh;
    SAF_Field     interleaved_mesh, interleaved_d;
    SAF_Field     interleaved_x_mesh;
    SAF_Field     interleaved_y_mesh;
    SAF_Field     interleaved_z_mesh;
    SAF_Field     press_mesh, press_d;
    SAF_Field     temp_mesh, temp_d;

#ifdef HAVE_PARALLEL
    /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass the same argc, argv to all
     * processors. However, the MPI spec says nothing about what it does or might do to argc or argv. In fact, there is no
     * "const" in the function prototypes for either the pointers or the things they're pointing too.  I would rather pass NULL
     * here and the spec says this is perfectly acceptable.  However, that too has caused MPICH to core on certain
     * platforms. */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &P);
    MPI_Comm_size(MPI_COMM_WORLD, &Np);
#else
    P  = 0;
    Np = 1;
#endif

    /* process command line args */

    /* since we want to see whats happening make sure stdout and stderr are unbuffered */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    STU_ProcessCommandLine(1, argc, argv, 
                           "do_multifile", 
                           "if present, expect data to have been written to different SAF files", 
                           &do_multifile, 
                           STU_END_OF_ARGS);
#ifdef WIN32
	/*This doesnt work in WIN32 for now. 12jan2005*/
		do_multifile=0;
#endif

#ifdef HAVE_PARALLEL
    MPI_Bcast(&do_multifile, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

    /* for convenience, set working directory to the test file directory */
    chdir(TEST_FILE_PATH);

#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    saf_init(SAF_DEFAULT_LIBPROPS);

    strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {
        char domainName[256];
        char tmp[256];

        /*  Note: because we are in a try block here, all failures will send us to the one and only
         *        catch block at the end of this test. */

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                    OPEN FILE(S)
         *--------------------------------------------------------------------------------------------------------------------- */
        TESTING("open database for reading only");

        dbprops = saf_createProps_database();
        saf_setProps_ReadOnly(dbprops);
        db      = saf_open_database(dbname, dbprops);

        /* For multifile mode, we expect one file for topology (relations) and one file for
                 * each time step.  Determine if the database was generated in a multifile mode */
        PASSED;

        TESTING("finding suplemental files");
        SKIPPED;

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                   PREPARE
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            int *NpWritersP = &NpWriters;
            hid_t type=H5T_NATIVE_INT;
            int count=1;
            saf_get_attribute(SAF_ALL, (ss_pers_t*)db, "ntasks", &type, &count, (void**)&NpWritersP);
        }

        numberOfDomains        = NpWriters;
        numberOfZonesPerDomain = 3;
        numberOfGhostZones     = 1;
        numberOfZones          = numberOfZonesPerDomain + ((numberOfDomains-1)*(numberOfZonesPerDomain-numberOfGhostZones));
        numberOfNodesPerZone   = 8;
        numberOfGhostNodes     = 4;
        numberOfNodesPerDomain = numberOfNodesPerZone + ((numberOfZonesPerDomain-1)*(numberOfNodesPerZone-numberOfGhostNodes));
        numberOfNodes          = numberOfNodesPerZone + ((numberOfZones-1)*(numberOfNodesPerZone-numberOfGhostNodes));

        sprintf(domainName, "Domain_%05d", P);

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                             GET/CHECK CATEGORIES
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            int      count;
            SAF_Cat *list;

            /* We expect to find exactly 3 categories: nodes, zones, domains. */            
            TESTING("finding all categories");
            count = 0;
            list  = NULL;
            saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_ANY_ROLE, SAF_ANY_TOPODIM, &count, &list);
            if (count != 3) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr, "...expected 3 categories, found %d\n", count);
                goto theExit;
            }
            PASSED;
        }

        {
            int         count;
            SAF_Cat    *list;
            SAF_Role    role;
            int         tdim;

            /* In addition to finding the right categories, we also check that
             * each has the proper role and topological dimension. */
            TESTING("finding individual categories");

            count = 0;
            list  = NULL;
            saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "nodes", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &count, &list);
            if (count != 1) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr, "...expected 1 \"nodes\" category, found %d\n", count);
                goto theExit;
            }
            nodes = list[0];
            saf_describe_category(SAF_ALL, &nodes, NULL, &role, &tdim);
            if (!SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim != 0) {
                FAILED;
                errorCount += 1;
                if (P == 0) {
                    if (!SAF_EQUIV(&role, SAF_TOPOLOGY))
                        fprintf(stderr, "...\"nodes\" category to have a topological role\n");
                    if (tdim != 0)
                        fprintf(stderr, "...\"nodes\" category to have a topological dimension of 0, found %d\n", tdim);
                }
                goto theExit;
            }

            count = 0;
            list  = NULL;
            saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "zones", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &count, &list);
            if (count != 1) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr, "...expected 1 \"zones\" category, found %d\n", count);
                goto theExit;
            }
            zones = list[0];
            saf_describe_category(SAF_ALL, &zones, NULL, &role, &tdim);
            if (!SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim != 3) {
                FAILED;
                errorCount += 1;
                if (P == 0) {
                    if (!SAF_EQUIV(&role, SAF_TOPOLOGY))
                        fprintf(stderr, "...\"zones\" category to have a topological role\n");
                    if (tdim != 3)
                        fprintf(stderr, "...\"zones\" category to have a topological dimension of 3, found %d\n", tdim);
                }
                goto theExit;
            }

            count = 0;
            list  = NULL;
            saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "domains", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &count, &list);
            if (count != 1) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr, "...expected 1 \"domains\" category, found %d\n", count);
                goto theExit;
            }
            domains = list[0];
            saf_describe_category(SAF_ALL, &domains, NULL, &role, &tdim);
            if (!SAF_EQUIV(&role, SAF_DOMAIN) || tdim != 3) {
                FAILED;
                errorCount += 1;
                if (P == 0) {
                    if (!SAF_EQUIV(&role, SAF_TOPOLOGY))
                        fprintf(stderr, "...\"domains\" category to have a topological role\n");
                    if (tdim != 3)
                        fprintf(stderr, "...\"domains\" category to have a topological dimension of 3, found %d\n", tdim);
                }
                goto theExit;
            }

            PASSED;
        }

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                ANALYZE SETS
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            int             count;
            SAF_ExtendMode  extmode;
            SAF_Set        *list;
            SAF_SilRole     role;
            int             tdim;
            SAF_TopMode     topmode;

            /* We expect to find one overall set representing the mesh... */            
            TESTING("finding the 1 top (all)");
            {
                int      numberOfTops;
                SAF_Set *topHandles;

                numberOfTops = 0;
                topHandles   = NULL;
                saf_find_matching_sets(SAF_ALL, db, 
                                       SAF_ANY_NAME, 
                                       SAF_SPACE, 
                                       SAF_ANY_TOPODIM, 
                                       SAF_EXTENDIBLE_TORF, 
                                       SAF_TOP_TRUE, 
                                       &numberOfTops, 
                                       &topHandles
                                       );
                if (numberOfTops != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 top set, found %d\n", numberOfTops);
                    goto theExit;
                }
            }
            PASSED;

            /* We expect to find one or more subsets, each representing a domain. The
             * number of these should match the number of processors... */
            sprintf(tmp, "finding the %d subset%s (all)", NpWriters, (NpWriters==1)?"":"s");
            TESTING(tmp);
            {
                int      numberOfSubsets;
                SAF_Set *subsetHandles;

                numberOfSubsets = 0;
                subsetHandles   = NULL;
                saf_find_matching_sets(SAF_ALL, db, 
                                       SAF_ANY_NAME, 
                                       SAF_SPACE, 
                                       SAF_ANY_TOPODIM, 
                                       SAF_EXTENDIBLE_TORF, 
                                       SAF_TOP_FALSE, 
                                       &numberOfSubsets, 
                                       &subsetHandles
                                       );
                if (numberOfSubsets != NpWriters) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected %d subset%s, found %d\n", NpWriters, (NpWriters==1)?"":"s", numberOfSubsets);
                    goto theExit;
                }
            }
            PASSED;

            /* This time we provide a carefully generated name which should match exactly one
             * subset, not that this assumes that the file was written by the same number of
             * processors as we're using to read. */
            TESTING("finding the local subset (each)");
            {
                int      numberOfSubsets;
                SAF_Set *subsetHandles;

                numberOfSubsets = 0;
                subsetHandles   = NULL;
                saf_find_matching_sets(SAF_EACH, db, 
                                       domainName, 
                                       SAF_SPACE, 
                                       SAF_ANY_TOPODIM, 
                                       SAF_EXTENDIBLE_TORF, 
                                       SAF_TOP_FALSE, 
                                       &numberOfSubsets, 
                                       &subsetHandles
                                       );
                if (numberOfSubsets != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 subset, found %d\n", numberOfSubsets);
                    goto theExit;
                }
            }
            PASSED;

            TESTING("finding \"TheMesh\" (all)");

            count = 0;
            list  = NULL;
            saf_find_matching_sets(SAF_ALL, db, 
                                   "TheMesh", 
                                   SAF_ANY_SILROLE, 
                                   SAF_ANY_TOPODIM, 
                                   SAF_EXTENDIBLE_TORF, 
                                   SAF_TOP_TORF, 
                                   &count, 
                                   &list
                                   );
            if (count != 1) {
                FAILED;
                errorCount += 1;
                goto theExit;
            }
            PASSED;

            TESTING("analyzing \"TheMesh\" (all)");
            mesh = list[0];
            saf_describe_set(SAF_ALL, &mesh, NULL, &tdim, &role, &extmode, &topmode, NULL, NULL);
            if (role != SAF_SPACE || tdim != 3 || extmode != SAF_EXTENDIBLE_FALSE || topmode != SAF_TOP_TRUE) {
                FAILED;
                errorCount += 1;
                if (P == 0) {
                    if (role != SAF_SPACE)
                        fprintf(stderr, "...expected \"TheMesh\" to have a \"spatial\" SIL role.\n");
                    if (tdim != 3)
                        fprintf(stderr, "...expected \"TheMesh\" to have a maximum basespace dimension of 3.\n");
                    if (extmode != SAF_EXTENDIBLE_FALSE)
                        fprintf(stderr, "...didn't expect \"TheMesh\" to be extendable.\n");
                    if (topmode != SAF_TOP_TRUE)
                        fprintf(stderr, "...expected \"TheMesh\" to be a \"top set\".\n");
                }
                goto theExit;
            }
            PASSED;

            TESTING("analyzing the domain sets (each)");
            {
                /* We also expect to find a set for each domain, first look for it by name
                 * and check it out. */
                {
                    count = 0;
                    list  = NULL;
                    saf_find_matching_sets(SAF_EACH, db, 
                                           domainName, 
                                           SAF_ANY_SILROLE, 
                                           SAF_ANY_TOPODIM, 
                                           SAF_EXTENDIBLE_TORF, 
                                           SAF_TOP_TORF, 
                                           &count, 
                                           &list
                                           );
                    if (count != 1) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected 1 set named \"%s\", found %d\n", domainName, count);
                        goto theExit;
                    }
                    domain = list[0];
                    saf_describe_set(SAF_EACH, &domain, NULL, &tdim, &role, &extmode, &topmode, NULL, NULL);
                    if (role != SAF_SPACE || tdim != 3 || extmode != SAF_EXTENDIBLE_FALSE || topmode != SAF_TOP_FALSE) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            if (role != SAF_SPACE)
                                fprintf(stderr, "...expected \"%s\" to have a \"spatial\" SIL role.\n", domainName);
                            if (tdim != 3)
                                fprintf(stderr, "...expected \"%s\" to have a maximum basespace dimension of 3.\n", domainName);
                            if (extmode != SAF_EXTENDIBLE_FALSE)
                                fprintf(stderr, "...didn't expect \"%s\" to be extendable.\n", domainName);
                            if (topmode != SAF_TOP_FALSE)
                                fprintf(stderr, "...didn't expect \"%s\" to be a \"top set\".\n", domainName);
                        }
                        goto theExit;
                    }
                }
            }
            PASSED;
        }

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                 ANALYZE COLLECTIONS
         *--------------------------------------------------------------------------------------------------------------------- */

        {
            SAF_CellType  cellType;
            int           count;
            SAF_IndexSpec ispec;
            SAF_DecompMode isDecomp;

            TESTING("analyzing \"nodes\" collection on \"TheMesh\" (all)");
            {
                saf_describe_collection(SAF_ALL, &mesh, &nodes, &cellType, &count, &ispec, &isDecomp, NULL);
                if (cellType != SAF_CELLTYPE_POINT || count != numberOfNodes || isDecomp!=SAF_DECOMP_FALSE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        if (cellType != SAF_CELLTYPE_POINT)
                            fprintf(stderr, "...expected \"nodes on TheMesh\" to be \"points\"\n");
                        if (count != numberOfNodes)
                            fprintf(stderr, "...expected %d \"nodes on TheMesh\", found %d\n", numberOfNodes, count);
                        if (isDecomp!=SAF_DECOMP_FALSE)
                            fprintf(stderr, "...didn't expect \"nodes on TheMesh\" to be a decomposition\n");
                    }
                    goto theExit;
                }
            }
            PASSED;

            TESTING("analyzing \"zones\" collection on \"TheMesh\" (all)");
            {
                saf_describe_collection(SAF_ALL, &mesh, &zones, &cellType, &count, &ispec, &isDecomp, NULL);
                if (cellType != SAF_CELLTYPE_HEX || count != numberOfZones || isDecomp!=SAF_DECOMP_TRUE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        if (cellType != SAF_CELLTYPE_HEX)
                            fprintf(stderr, "...expected \"zones on TheMesh\" to be \"hexes\"\n");
                        if (count != numberOfZones)
                            fprintf(stderr, "...expected %d \"zones on TheMesh\", found %d\n", numberOfZones, count);
                        if (isDecomp!=SAF_DECOMP_TRUE)
                            fprintf(stderr, "...expected \"zones on TheMesh\" to be a decomposition\n");
                    }
                    goto theExit;
                }
            }
            PASSED;

            TESTING("analyzing \"domains\" collection on \"TheMesh\" (all)");
            {
                saf_describe_collection(SAF_ALL, &mesh, &domains, &cellType, &count, &ispec, &isDecomp, NULL);
                if (cellType != SAF_CELLTYPE_SET || count != numberOfDomains || isDecomp!=SAF_DECOMP_TRUE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        if (cellType != SAF_CELLTYPE_SET)
                            fprintf(stderr, "...expected \"domains on TheMesh\" to be \"sets\"\n");
                        if (count != numberOfDomains)
                            fprintf(stderr, "...expected %d \"domains on TheMesh\", found %d\n", numberOfDomains, count);
                        if (isDecomp!=SAF_DECOMP_TRUE)
                            fprintf(stderr, "...expected \"domains on TheMesh\" to be a decomposition\n");
                    }
                    goto theExit;
                }
            }
            PASSED;

            {
                TESTING("analyzing \"nodes\" collection on \"Domain\" (each)");
                {
                    saf_describe_collection(SAF_EACH, &domain, &nodes, &cellType, &count, &ispec, &isDecomp, NULL);
                    if (cellType != SAF_CELLTYPE_POINT || count != numberOfNodesPerDomain || isDecomp!=SAF_DECOMP_FALSE) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            if (cellType != SAF_CELLTYPE_POINT)
                                fprintf(stderr, "...expected \"nodes on %s\" to be \"points\"\n", domainName);
                            if (count != numberOfNodesPerDomain)
                                fprintf(stderr, "...expected %d \"nodes on %s\", found %d\n", numberOfNodesPerDomain, domainName,
                                        count);
                            if (isDecomp!=SAF_DECOMP_FALSE)
                                fprintf(stderr, "...didn't expect \"nodes on %s\" to be a decomposition\n", domainName);
                        }
                        goto theExit;
                    }
                }
                PASSED;

                TESTING("analyzing \"zones\" collection on \"Domain\" (each)");
                {
                    saf_describe_collection(SAF_EACH, &domain, &zones, &cellType, &count, &ispec, &isDecomp, NULL);
                    if (cellType != SAF_CELLTYPE_HEX || count != numberOfZonesPerDomain || isDecomp!=SAF_DECOMP_TRUE) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            if (cellType != SAF_CELLTYPE_HEX)
                                fprintf(stderr, "...expected \"zones on %s\" to be \"hexes\"\n", domainName);
                            if (count != numberOfZonesPerDomain)
                                fprintf(stderr, "...expected %d \"zone on %s\", found %d\n", numberOfZonesPerDomain, domainName,
                                        count);
                            if (isDecomp!=SAF_DECOMP_TRUE)
                                fprintf(stderr, "...expected \"zones on %s\" to be a decomposition\n", domainName);
                        }
                        goto theExit;
                    }
                }
                PASSED;

                TESTING("analyzing \"domains\" collection on \"Domain\" (each)");
                {
                    saf_describe_collection(SAF_EACH, &domain, &domains, &cellType, &count, &ispec, &isDecomp, NULL);
                    if (cellType != SAF_CELLTYPE_SET || count != 1 || isDecomp!=SAF_DECOMP_TRUE) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            if (cellType != SAF_CELLTYPE_SET)
                                fprintf(stderr, "...expected \"domains on %s\" to be \"sets\"\n", domainName);
                            if (count != 1)
                                fprintf(stderr, "...expected 1 \"domain on %s\", found %d\n", domainName, count);
                            if (isDecomp!=SAF_DECOMP_TRUE)
                                fprintf(stderr, "...expected \"domains on %s\" to be a decomposition\n", domainName);
                        }
                        goto theExit;
                    }
                }
                PASSED;
            }
        }

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                ANALYZE SUBSET RELATIONS
         *--------------------------------------------------------------------------------------------------------------------- */


        {
            int            count;
            hid_t          theDatatype;
            SAF_Rel       *list;
            SAF_Cat        supCat, subCat;
            SAF_BoundMode  sbmode, cbmode;
            SAF_RelRep     relRep;

            /* Initialize the datatypes of each of the relations.  These will be filled in
             * using the saf_find_subset_relations() and saf_describe_subset_relation()
             * functions.  It would probably be nicer to find a more general procedure
             * but that is for the future... */

            TESTING("finding \"nodes\" subset relations (each)");
            {
                count = 0;
                list  = NULL;
                saf_find_subset_relations(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&nodes), &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 subset relation between \"TheMesh\" and \"%s\" through \"nodes\", found %d\n", domainName, count);
                    goto theExit;
                }
            }
            PASSED;

            TESTING("analyzing \"nodes\" subset relations (each)");
            {
                theDatatype = H5I_INVALID_HID;
                saf_describe_subset_relation(SAF_EACH, list+0, NULL, NULL, &supCat, &subCat, &sbmode, &cbmode, &relRep,
                                             &theDatatype);
                if ((!SAF_EQUIV(&supCat, &nodes)) || (!SAF_EQUIV(&subCat, &nodes))) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected both categories on subset relation between \"TheMesh\" and \"%s\" to be on \"nodes\"\n", domainName);
                    goto theExit;
                }
                if (sbmode != SAF_BOUNDARY_FALSE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...didn't expect subset relation between \"TheMesh\" and \"%s\" through \"nodes\" to be \"boundary\"\n", domainName);
                    goto theExit;
                }
                if (cbmode != SAF_BOUNDARY_FALSE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...didn't expect subset relation between \"TheMesh\" and \"%s\" through \"nodes\" to have \"boundary\" members\n", domainName);
                    goto theExit;
                }
                if (!SAF_EQUIV(&relRep, SAF_TUPLES)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected subset relation between \"TheMesh\" and \"%s\" through \"nodes\" to be described as \"tuples\"\n", domainName);
                    goto theExit;
                }
                if (H5T_INTEGER!=H5Tget_class(theDatatype)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr,
                                "...wrong datatype for subset relation between \"TheMesh\" and \"%s\" through \"nodes\"\n",
                                domainName);
                    goto theExit;
                }
            }
            PASSED;

            TESTING("finding \"zones\" subset relations (each)");
            {
                count = 0;
                list  = NULL;
                saf_find_subset_relations(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&zones), &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 subset relation between \"TheMesh\" and \"%s\" through \"zones\", found %d\n", domainName, count);
                    goto theExit;
                }
            }
            PASSED;

            TESTING("analyzing \"zones\" subset relations (each)");
            {
                theDatatype = H5I_INVALID_HID;
                saf_describe_subset_relation(SAF_EACH, list+0, NULL, NULL, &supCat, &subCat, &sbmode, &cbmode, &relRep, 
                                             &theDatatype);
                if ((!SAF_EQUIV(&supCat, &zones)) || (!SAF_EQUIV(&subCat, &zones))) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected both categories on subset relation between \"TheMesh\" and \"%s\" to be on \"zones\"\n", domainName);
                    goto theExit;
                }
                if (sbmode != SAF_BOUNDARY_FALSE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...didn't expect subset relation between \"TheMesh\" and \"%s\" through \"zones\" to be \"boundary\"\n", domainName);
                    goto theExit;
                }
                if (cbmode != SAF_BOUNDARY_FALSE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...didn't expect subset relation between \"TheMesh\" and \"%s\" through \"zones\" to have \"boundary\" members\n", domainName);
                    goto theExit;
                }
                if (!SAF_EQUIV(&relRep, SAF_TUPLES)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected subset relation between \"TheMesh\" and \"%s\" through \"zones\" to be described by \"tuples\"\n", domainName);
                    goto theExit;
                }
                if (H5T_INTEGER!=H5Tget_class(theDatatype)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr,
                                "...wrong datatype for subset relation between \"TheMesh\" and \"%s\" through \"zones\"\n",
                                domainName);
                    goto theExit;
                }
            }
            PASSED;

            TESTING("finding \"domains\" subset relations (each)");
            {
                count = 0;
                list  = NULL;
                saf_find_subset_relations(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&domains), &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 subset relation between \"TheMesh\" and \"%s\" through \"domains\", found %d\n", domainName, count);
                    goto theExit;
                }
            }
            PASSED;

            TESTING("analyzing \"domains\" subset relations (each)");
            {
                theDatatype = H5I_INVALID_HID;
                saf_describe_subset_relation(SAF_EACH, list+0, NULL, NULL, &supCat, &subCat, &sbmode, &cbmode, &relRep, 
                                             &theDatatype);
                if ((!SAF_EQUIV(&supCat, &domains)) || (!SAF_EQUIV(&subCat, &domains))) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected both categories on subset relation between \"TheMesh\" and \"%s\" to be on \"domains\"\n", domainName);
                    goto theExit;
                }
                if (sbmode != SAF_BOUNDARY_FALSE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...didn't expect subset relation between \"TheMesh\" and \"%s\" through \"domains\" to be \"boundary\"\n", domainName);
                    goto theExit;
                }
                if (cbmode != SAF_BOUNDARY_FALSE) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...didn't expect subset relation between \"TheMesh\" and \"%s\" through \"domains\" to have \"boundary\" members\n", domainName);
                    goto theExit;
                }
                if (!SAF_EQUIV(&relRep, SAF_TUPLES)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected subset relation between \"TheMesh\" and \"%s\" through \"domains\" to be described by \"tuples\"\n", domainName);
                    goto theExit;
                }
                if (!H5Tequal(theDatatype, SAF_HANDLE)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr,
                                "...wrong datatype for subset relation between \"TheMesh\" and \"%s\" through \"domains\"\n",
                                domainName);
                    }
                    goto theExit;
                }
            }
            PASSED;
        }

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                      READ SUBSET RELATIONS
         *--------------------------------------------------------------------------------------------------------------------- */

        {
            int       count;
            SAF_Rel  *list;
            void     *Abuf, *Bbuf;
            size_t    Acount, Bcount;
            hid_t      Atype, Btype;

            TESTING("reading \"nodes\" subset relations (each)");
            {
                count = 0;
                list  = NULL;
                saf_find_subset_relations(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&nodes), &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 subset relation between \"TheMesh\" and \"%s\" through \"nodes\"\n", domainName);
                    goto theExit;
                }

                Abuf = Bbuf = NULL;
                saf_get_count_and_type_for_subset_relation(SAF_EACH, list+0, NULL, &Acount, &Atype, &Bcount, &Btype);
                saf_read_subset_relation(SAF_EACH, list+0, NULL, &Abuf, &Bbuf);

                if (Abuf == NULL) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected non-nil A-buffer between \"TheMesh\" and \"%s\" through \"nodes\"\n", domainName);
                    goto theExit;
                }
                if (Acount != (size_t)numberOfNodesPerDomain) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr, "...expected %d items in A-buffer between \"TheMesh\" and \"%s\" through \"nodes\"\n", numberOfNodesPerDomain, domainName);
                        fprintf(stderr, "   found %ld item%s\n", (long)Acount, (Acount==1)?"":"s");
                    }
                    goto theExit;
                }
                {
                    int *expect;
                    int  n;

                    expect = (int *)malloc(numberOfNodesPerDomain*sizeof(int));
                    for (n=0; n<numberOfNodesPerDomain; ++n)
                        expect[n] = globalNodeNumber(P, n);

                    if (H5Tequal(Atype, H5T_NATIVE_INT)) {
                        size_t  i;
                        int    *p;

                        p = (int *)Abuf;
                        for (i=0; i<Acount; ++i) {
                            if (p[i] != (int)(expect[i])) {
                                FAILED;
                                errorCount += 1;
                                if (P == 0) {
                                    fprintf(stderr, "...wrong value for item %lu in A-buffer between \"TheMesh\" and \"%s\" through \"nodes\"\n", (unsigned long)i, domainName);
                                    fprintf(stderr, "   expected %d, found %d\n", expect[i], p[i]);
                                }
                                goto theExit;
                            }
                        }
                    } else {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            fprintf(stderr,
                                    "...wrong datatype for items in A-buffer between \"TheMesh\" and \"%s\" through \"nodes\"\n",
                                    domainName);
                        }
                        goto theExit;
                    }
                    free(expect);
                }
                if (Bbuf != NULL) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected nil B-buffer between \"TheMesh\" and \"%s\" through \"nodes\"\n", domainName);
                    goto theExit;
                }
                if (Bcount != 0) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr, "...expected 0 items in B-buffer between \"TheMesh\" and \"%s\" through \"nodes\"\n", domainName);
                        fprintf(stderr, "   found %ld item%s\n", (long)Bcount, (Bcount==1)?"":"s");
                    }
                    goto theExit;
                }
            }
            PASSED;

            TESTING("reading \"zones\" subset relations (each)");
            {
                count = 0;
                list  = NULL;
                saf_find_subset_relations(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&zones), &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 subset relation between \"TheMesh\" and \"%s\" through \"zones\"\n", domainName);
                    goto theExit;
                }

                Abuf = Bbuf = NULL;
                saf_get_count_and_type_for_subset_relation(SAF_EACH, list+0, NULL, &Acount, &Atype, &Bcount, &Btype);
                saf_read_subset_relation(SAF_EACH, list+0, NULL, &Abuf, &Bbuf);
                if (Abuf == NULL) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected non-nil A-buffer between \"TheMesh\" and \"%s\" through \"zones\"\n", domainName);
                    goto theExit;
                }
                if (Acount != (size_t)numberOfZonesPerDomain) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr, "...expected %d item in A-buffer between \"TheMesh\" and \"%s\" through \"zones\"\n", 
                                numberOfZonesPerDomain, 
                                domainName
                                );
                        fprintf(stderr, "   found %ld item%s\n", (long)Acount, (Acount==1)?"":"s");
                    }
                    goto theExit;
                }
                {
                    int *expect;
                    int  z;

                    expect = (int*)malloc(numberOfZonesPerDomain*sizeof(int));
                    for (z=0; z<numberOfZonesPerDomain; ++z)
                        expect[z] = globalZoneNumber(P, z);

                    if (H5Tequal(Atype, H5T_NATIVE_INT)) {
                        size_t  i;
                        int    *p;

                        p = (int *)Abuf;
                        for (i=0; i<Acount; ++i)
                            if (p[i] != (int)(expect[i])) {
                                FAILED;
                                errorCount += 1;
                                if (P == 0) {
                                    fprintf(stderr, "...wrong value for item %lu in A-buffer between \"TheMesh\" and \"%s\" through \"zones\"\n", (unsigned long)i, domainName);
                                    fprintf(stderr, "   expected %d, found %d\n", expect[i], p[i]);
                                }
                                goto theExit;
                            }
                    } else {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            fprintf(stderr,
                                    "...wrong datatype for items in A-buffer between \"TheMesh\" and \"%s\" through \"zones\"\n",
                                    domainName);
                        }
                        goto theExit;
                    }
                    free(expect);
                }
                if (Bbuf != NULL) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected nil B-buffer between \"TheMesh\" and \"%s\" through \"zones\"\n", domainName);
                    goto theExit;
                }
                if (Bcount != 0) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr, "...expected 0 items in B-buffer between \"TheMesh\" and \"%s\" through \"zones\"\n", domainName);
                        fprintf(stderr, "   found %ld item%s\n", (long)Bcount, (Bcount==1)?"":"s");
                    }
                    goto theExit;
                }
            }
            PASSED;

            TESTING("reading \"domains\" subset relations (each)");
            {
                count = 0;
                list  = NULL;
                saf_find_subset_relations(SAF_EACH, db, &mesh, &domain, SAF_COMMON(&domains), &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 subset relation between \"TheMesh\" and \"%s\" through \"domains\"\n", domainName);
                    goto theExit;
                }

                Abuf = Bbuf = NULL;
                saf_get_count_and_type_for_subset_relation(SAF_EACH, list+0, NULL, &Acount, &Atype, &Bcount, &Btype);
                saf_read_subset_relation(SAF_EACH, list+0, NULL, &Abuf, &Bbuf);
                if (Abuf == NULL) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected non-nil A-buffer between \"TheMesh\" and \"%s\" through \"domains\"\n", domainName);
                    goto theExit;
                }
                if (Acount != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr, "...expected 1 item in A-buffer between \"TheMesh\" and \"%s\" through \"domains\"\n", domainName);
                        fprintf(stderr, "   found %ld item%s\n", (long)Acount, (Acount==1)?"":"s");
                    }
                    goto theExit;
                }
                {
                    int expect[1];

                    expect[0] = P;

                    if (H5Tequal(Atype, H5T_NATIVE_SIZE)) {
                        size_t i;
                        size_t *p;

                        p = (size_t*)Abuf;
                        for (i=0; i<Acount; ++i)
                            if (p[i] != (size_t)expect[i]) {
                                FAILED;
                                errorCount += 1;
                                if (P == 0) {
                                    fprintf(stderr, "...wrong value for item %lu in A-buffer between \"TheMesh\" and \"%s\" through \"domains\"\n", (unsigned long)i, domainName);
                                    fprintf(stderr, "   expected %d, found %lu\n", expect[i], (unsigned long)(p[i]));
                                }
                                goto theExit;
                            }
                    } else {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            fprintf(stderr,
                                    "...wrong datatype for items in A-buffer between \"TheMesh\" and \"%s\" through \"domains\"\n",
                                    domainName);
                        }
                        goto theExit;
                    }
                }
                if (Bbuf != NULL) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected nil B-buffer between \"TheMesh\" and \"%s\" through \"domains\"\n", domainName);
                    goto theExit;
                }
                if (Bcount != 0) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr, "...expected 0 items in B-buffer between \"TheMesh\" and \"%s\" through \"domains\"\n", domainName);
                        fprintf(stderr, "   found %ld item%s\n", (long)Bcount, (Bcount==1)?"":"s");
                    }
                    goto theExit;
                }
            }
            PASSED;
        }

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                      ANALYZE TOPOLOGY RELATIONS
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            int      count;
            SAF_Rel *list;

            TESTING("finding global topology relations (all)");
            {
                count = 0;
                list  = NULL;
                saf_find_topo_relations(SAF_ALL, db, &mesh, NULL, &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr, "...expected 1 topological relation on \"TheMesh\"\n");
                        fprintf(stderr, "   found %d relation%s\n", count, (count==1)?"":"s");
                    }
                    goto theExit;
                }
            }
            PASSED;

            TESTING("finding local topology relations (each)");
            {
                count = 0;
                list  = NULL;
                saf_find_topo_relations(SAF_EACH, db, &domain, NULL, &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr, "...expected 1 topological relation on \"%s\"\n", domainName);
                        fprintf(stderr, "   found %d relation%s\n", count, (count==1)?"":"s");
                    }
                    goto theExit;
                }
            }
            PASSED;
        }

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                      READ TOPOLOGY RELATIONS
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            void          *Abuf, *Bbuf;
            size_t         Acount, Bcount;
            hid_t          Atype, Btype;
            int            count;
            SAF_Cat        glueCat;
            SAF_Set        glueSet;
            hbool_t        isSelfStored;
            SAF_Rel       *list;
            SAF_Cat        partCat;
            SAF_Set        partSet;
            SAF_Rel        TonDomain;
            SAF_RelRep     relRep;

            TESTING("reading \"direct\" topology relations (each)");
            {
                count = 0;
                list  = NULL;
                saf_find_topo_relations(SAF_EACH, db, &domain, NULL, &count, &list);
                if (count != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 topological relation on \"%s\", found %ld\n", domainName, (long)count);
                    goto theExit;
                }
                TonDomain = list[0];
            }
            PASSED;

            TESTING("analyzing \"direct\" topology relations (each)");
            {
                saf_is_self_stored_topo_relation(SAF_EACH, &TonDomain, &isSelfStored);
                if (!isSelfStored) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected topological relation on \"%s\" to be stored on self\n", domainName);
                    goto theExit;
                }
                saf_describe_topo_relation(SAF_EACH, &TonDomain, &partSet, &partCat, &glueSet, &glueCat, NULL, &relRep, NULL);
                if (!SAF_EQUIV(&partCat, &zones)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected topological relation on \"%s\" to be binding \"zones\"\n", domainName);
                    goto theExit;
                }
                if (!SAF_EQUIV(&partSet, &domain)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected topological relation on \"%s\" to be binding zones on \"%s\"\n", domainName, domainName);
                    goto theExit;
                }
                if (!SAF_EQUIV(&glueCat, &nodes)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected topological relation on \"%s\" to be binding zones on \"%s\" using \"nodes\"\n", domainName, domainName);
                    goto theExit;
                }
                if (!SAF_EQUIV(&glueSet, &domain)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected topological relation on \"%s\" to be binding zones on \"%s\" using \"nodes\" on \"%s\"\n", domainName, domainName, domainName);
                    goto theExit;
                }
            }
            PASSED;

            TESTING("reading \"direct\" topology relations (each)");
            {
                int  Aexpect[1];
                int *Bexpect;
                int  N;

                Abuf   = NULL;
                Acount = 0;
                Atype  = H5I_INVALID_HID;
                Bbuf   = NULL;
                Bcount = 0;
                Btype  = H5I_INVALID_HID;
                saf_get_count_and_type_for_topo_relation(SAF_EACH, &TonDomain, NULL, NULL, &Acount, &Atype, &Bcount, &Btype);
                saf_read_topo_relation(SAF_EACH, &TonDomain, NULL, &Abuf, &Bbuf);

                Aexpect[0]   = numberOfNodesPerZone;
                N            = numberOfZonesPerDomain * numberOfNodesPerZone;
                if ((Bexpect = (int*)malloc(N*sizeof(int))) != NULL) {
                    int i, n, nn, z;

                    nn         = 0;
                    i          = 0;
                    for (z=0; z<numberOfZonesPerDomain; ++z) {
                        for (n=0; n<numberOfNodesPerZone; ++n) {
                            Bexpect[i] = nn;
                            i         += 1;
                            nn        += 1;
                        }
                        nn    -= numberOfGhostNodes;
                    }
                }

                if (Acount != 1) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 1 A-buffer item in topological relation on \"%s\", found %ld\n", domainName, (long)Acount);
                    goto theExit;
                }
                if (H5Tequal(Atype, H5T_NATIVE_INT)) {
                    size_t  i;
                    int    *p;

                    p = (int *)Abuf;
                    for (i=0; i<Acount; ++i)
                        if (p[i] != (int)(Aexpect[i])) {
                            FAILED;
                            errorCount += 1;
                            if (P == 0) {
                                fprintf(stderr, "...wrong value for A-buffer item %lu in topological relation on \"%s\"\n", (unsigned long)i, domainName);
                                fprintf(stderr, "   expected %d, found %d\n", Aexpect[i], p[i]);
                            }
                            goto theExit;
                        }
                } else {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr,
                                "...wrong datatype for items in A-buffer for topological relation on \"%s\"\n", domainName);
                    }
                    goto theExit;
                }
                if (Bcount != (size_t)N) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected %d B-buffer items in topological relation on \"%s\", found %ld\n", N, domainName, (long)Bcount);
                    goto theExit;
                }
                if (H5Tequal(Btype, H5T_NATIVE_INT)) {
                    size_t  i;
                    int    *p;

                    p = (int *)Bbuf;
                    for (i=0; i<Bcount; ++i) {
                        if (p[i] != (int)(Bexpect[i])) {
                            FAILED;
                            errorCount += 1;
                            if (P == 0) {
                                fprintf(stderr, "...wrong value for B-buffer item %lu in topological relation on \"%s\"\n", (unsigned long)i, domainName);
                                fprintf(stderr, "   expected %d, found %d\n", Bexpect[i], p[i]);
                            }
                            goto theExit;
                        }
                    }
                } else {
                    FAILED;
                    errorCount += 1;
                    if (P == 0) {
                        fprintf(stderr,
                                "...wrong datatype for items in B-buffer for topological relation on \"%s\"\n", domainName);
                    }
                    goto theExit;
                }
                free(Bexpect);
            }
            PASSED;

            {
                void     *Abuf, *Bbuf;
                size_t    Acount, Bcount;
                hid_t     Atype, Btype;
                SAF_Rel  *theRels;

                TESTING("finding \"indirect\" topology relations (all)");
                {
                    count = 0;
                    list  = NULL;
                    saf_find_topo_relations(SAF_ALL, db, &mesh, NULL, &count, &list);
                    if (count != 1) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected 1 topological relation on \"TheMesh\", found %d\n", count);
                        goto theExit;
                    }
                }
                PASSED;

                TESTING("analyzing \"indirect\" topology relations (all)");
                {
                    saf_is_self_stored_topo_relation(SAF_ALL, list+0, &isSelfStored);
                    if (isSelfStored) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected topological relation on \"TheMesh\" to be stored on a decomposition\n");
                        goto theExit;
                    }
                }
                PASSED;

                TESTING("reading \"indirect\" topology relations (all)");
                {
                    theRels = NULL;
                    Abuf    = NULL;
                    Acount  = 0;
                    Atype   = H5I_INVALID_HID;
                    Bbuf    = NULL;
                    Bcount  = 0;
                    Btype   = H5I_INVALID_HID;
                    saf_get_count_and_type_for_topo_relation(SAF_ALL, list+0, NULL, NULL, &Acount, &Atype, &Bcount, &Btype);
                    saf_read_topo_relation(SAF_ALL, list+0, NULL, &Abuf, &Bbuf);
                    if (Abuf == NULL) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected A-buffer from topological relation on \"TheMesh\"\n");
                        goto theExit;
                    }
                    if (Acount != (size_t)numberOfDomains) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected A-buffer from topological relation on \"TheMesh\" to contain %d item%s\n", numberOfDomains, (numberOfDomains==1)?"":"s");
                        goto theExit;
                    }
                    if (!H5Tequal(Atype, SAF_HANDLE)) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected A-buffer from topological relation on \"TheMesh\" to contain handles\n");
                        goto theExit;
                    }
                    theRels = (SAF_Rel *)Abuf;
                    if (!SAF_EQUIV(theRels+P, &TonDomain)) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected item %d in A-buffer from topological relation on \"TheMesh\" to contain topological relation on \"%s\"\n", P, domainName);
                        goto theExit;
                    }
                    if (Bbuf != NULL) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected no B-buffer from topological relation on \"TheMesh\"\n");
                        goto theExit;
                    }
                    if (Bcount != 0) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected B-buffer from topological relation on \"TheMesh\" to contain no items\n");
                        goto theExit;
                    }
                    if (Btype>0) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected B-buffer from topological relation on \"TheMesh\" to have no type\n");
                        goto theExit;
                    }
                }
                PASSED;
            }

            {
                size_t      *Abuf, *Bbuf;
                size_t      Acount, Bcount;
                hid_t       Atype, Btype;
                int         count;
                int         i, nn;
                SAF_Rel    *list;
                int         z;
                SAF_RelTarget listTarget;

                TESTING("finding \"direct\" topology relations on the mesh (all)");
                {
                    count = 0;
                    list  = NULL;
                    saf_find_topo_relations(SAF_ALL, db, &mesh, NULL, &count, &list);
                    if (count != 1) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected 1 topological relation on \"TheMesh\", found %d\n", count);
                        goto theExit;
                    }
                }
                PASSED;

                TESTING("analyzing \"direct\" topology relations on the mesh (all)");
                {
                    saf_is_self_stored_topo_relation(SAF_ALL, list+0, &isSelfStored);
                    if (isSelfStored) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected topological relation on \"TheMesh\" to be stored on a decomposition\n");
                        goto theExit;
                    }
                }
                PASSED;

                TESTING("reading \"direct\" topology relations on the mesh (all)");
                {
                    Abuf   = NULL;
                    Acount = 0;
                    Atype  = H5I_INVALID_HID;
                    Bbuf   = NULL;
                    Bcount = 0;
                    Btype  = H5I_INVALID_HID;
                    /* We want to remap */
                    saf_target_topo_relation(&listTarget, NULL, NULL, SAF_SELF(db), NULL, H5I_INVALID_HID);
                    saf_get_count_and_type_for_topo_relation(SAF_ALL, list+0, &listTarget, NULL, &Acount, &Atype, &Bcount, &Btype);
                    if (Acount != 1) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected topological relation on \"TheMesh\" to to have an A-count of 1, found %ld\n"
                                    , (long)Acount
                                    );
                        goto theExit;
                    }
                    if (!H5Tequal(Atype, H5T_NATIVE_SIZE)) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...wrong datatype for topological relation on \"TheMesh\"\n");
                        goto theExit;
                    }
                    if (((long)Bcount) != (long)(numberOfZones*numberOfNodesPerZone)) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected topological relation on \"TheMesh\" to to have a B-count of %ld, found %ld\n"
                                    , (long)(numberOfZones*numberOfNodesPerZone)
                                    , (long)Bcount
                                    );
                        goto theExit;
                    }
                    if (!H5Tequal(Btype, H5T_NATIVE_SIZE)) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...wrong datatype for topological relation on \"TheMesh\"\n");
                        goto theExit;
                    }
                    saf_read_topo_relation(SAF_ALL, list+0, &listTarget, (void**)&Abuf, (void**)&Bbuf);
                    if (Abuf == NULL) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected A-buffer from topological relation on \"TheMesh\"\n");
                        goto theExit;
                    }
                    if (Abuf[0] != numberOfNodesPerZone) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected A-buffer to contain {%d}, found {%ld}\n"
                                    , numberOfNodesPerZone
                                    , (long)Abuf[0]
                                    );
                        goto theExit;
                    }
                    if (Bbuf == NULL) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected B-buffer from topological relation on \"TheMesh\"\n");
                        goto theExit;
                    }
                    i  = 0;
                    nn = 0;
                    for (z=0; z<numberOfZones; ++z) {
                        int n;

                        for (n=0; n<numberOfNodesPerZone; ++n) {
                            if (Bbuf[i] != nn) {
                                FAILED;
                                errorCount += 1;
                                if (P == 0)
                                    fprintf(stderr, "...expected B-buffer[%ld] to contain %ld, found %ld\n"
                                            , (long)i
                                            , (long)nn
                                            , (long)(Bbuf[i])
                                            );
                                goto theExit;
                            }
                            i  += 1;
                            nn += 1;
                        }
                        nn -= numberOfGhostNodes;
                    }
                }
                PASSED;
            }
        }

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                           FIND FIELDS
         *--------------------------------------------------------------------------------------------------------------------- */

        coords_mesh = SS_FIELD_NULL;
        coord_x_mesh = SS_FIELD_NULL;
        coord_y_mesh = SS_FIELD_NULL;
        coord_z_mesh = SS_FIELD_NULL;
        interleaved_mesh = SS_FIELD_NULL;
        interleaved_x_mesh = SS_FIELD_NULL;
        interleaved_y_mesh = SS_FIELD_NULL;
        interleaved_z_mesh = SS_FIELD_NULL;
        temp_mesh = SS_FIELD_NULL;
        press_mesh = SS_FIELD_NULL;

        {
            SAF_Field *listOfFieldsOnMesh;
            int        numberOfFieldsOnMesh;
            SAF_Cat    storage_decomp;
            int        whichFieldOnMesh;

            TESTING("finding fields on \"TheMesh\" (all)");
            {
                numberOfFieldsOnMesh = 0;
                listOfFieldsOnMesh   = NULL;
                saf_find_fields(SAF_ALL, db, &mesh, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT, 
                                SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &numberOfFieldsOnMesh,
                                &listOfFieldsOnMesh);
                if (numberOfFieldsOnMesh != 4) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected 4 fields on \"TheMesh\", found %d\n", numberOfFieldsOnMesh);
                    goto theExit;
                }
            }
            PASSED;

            TESTING("analyzing fields on \"TheMesh\" (all)");
            {
                for (whichFieldOnMesh=0; whichFieldOnMesh<numberOfFieldsOnMesh; ++whichFieldOnMesh) {
                    size_t          expectCount;
                    hid_t           expectType;
                    hbool_t         isSelfStored;
                    char           *name;
                    size_t          theCount;
                    hid_t           theType;

                    name = NULL;
                    saf_describe_field(SAF_ALL, listOfFieldsOnMesh+whichFieldOnMesh, NULL, &name, NULL, NULL, NULL, NULL, NULL, 
                                       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
                    if (!strcmp(name, "coords")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        coords_mesh         = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "X")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        coord_x_mesh        = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "Y")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        coord_y_mesh        = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "Z")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        coord_z_mesh        = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "interleaved")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        interleaved_mesh    = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "iX")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        interleaved_x_mesh  = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "iY")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        interleaved_y_mesh  = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "iZ")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        interleaved_z_mesh  = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "temperature")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        temp_mesh           = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else if (!strcmp(name, "pressure")) {
                        expectCount         = numberOfDomains;
                        expectType          = SAF_HANDLE;
                        press_mesh          = listOfFieldsOnMesh[whichFieldOnMesh];
                    } else {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            fprintf(stderr, "...expected field on \"TheMesh\" to have name in \"coords\", \"X\", \"Y\", \"Z\", \"interleaved\", \"iX\", \"iY\", \"iZ\", \"temperature\", \"pressure\"\n");
                            fprintf(stderr, "   found %s\n", name);
                        }
                        goto theExit;
                    }
                    saf_is_self_stored_field(SAF_ALL, listOfFieldsOnMesh+whichFieldOnMesh, &isSelfStored);
                    if (isSelfStored) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected field %s on \"TheMesh\" to be stored on a decomposition\n", name);
                        goto theExit;
                    }
                    saf_get_count_and_type_for_field(SAF_ALL, listOfFieldsOnMesh+whichFieldOnMesh, NULL, &theCount, &theType);
                    if (theCount != expectCount) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0) {
                            fprintf(stderr, "...expected field %s on \"TheMesh\" to have %ld DOF%s\n", name,
                                    (long)expectCount, (expectCount==1)?"":"s");
                            fprintf(stderr, "   found %ld DOF%s\n", (long)theCount, (theCount==1)?"":"s");
                        }
                        goto theExit;
                    }
                    if (0 < theCount) {
                        int             assoc_ratio;
                        SAF_Cat         coeff_assoc;
                        SAF_Field(comp_flds, p_comp_flds, 3);
                        SAF_Interleave  comp_interleave;
                        int             comp_order[3], *p_comp_order = comp_order;
                        hid_t           data_type;
                        SAF_Cat         eval_coll;
                        SAF_Eval        eval_func;
                        SAF_FieldTmpl   ftmpl;
                        hbool_t         is_coord;
                        int             num_comps;

                        if (!H5Tequal(theType, expectType)) {
                            FAILED;
                            errorCount += 1;
                            if (P == 0)
                                fprintf(stderr, "...wrong datatype for field %s on \"TheMesh\"\n", name);
                            goto theExit;
                        }
                        num_comps = 3;
                        saf_describe_field(SAF_ALL, listOfFieldsOnMesh+whichFieldOnMesh, &ftmpl, NULL, NULL, NULL, &is_coord,
                                           &storage_decomp, &coeff_assoc, &assoc_ratio, &eval_coll, &eval_func, &data_type,
                                           &num_comps, &p_comp_flds, &comp_interleave, &p_comp_order);
                        if (!SAF_EQUIV(&storage_decomp, &domains)) {
                            FAILED;
                            errorCount += 1;
                            if (P == 0)
                                fprintf(stderr, "...expected field %s on \"TheMesh\" to be stored on a decomposition\n", name);
                            goto theExit;
                        }
                    }
                }
                free(listOfFieldsOnMesh);
            }
            PASSED;

            {
                SAF_Field *listOfFieldsOnThisDomain;
                int        numberOfFieldsOnThisDomain;

                TESTING("finding fields on this \"Domain\" (each)");
                {
                    numberOfFieldsOnThisDomain = 0;
                    listOfFieldsOnThisDomain   = NULL;
                    saf_find_fields(SAF_EACH, db, &domain, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, 
                                    SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, 
                                    &numberOfFieldsOnThisDomain, &listOfFieldsOnThisDomain);
                    if (numberOfFieldsOnThisDomain != 10) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected 10 fields on \"%s\", found %d\n", domainName, numberOfFieldsOnThisDomain);
                        goto theExit;
                    }
                    if (listOfFieldsOnThisDomain == NULL) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...missing the %d fields on \"%s\"\n", numberOfFieldsOnThisDomain, domainName);
                        goto theExit;
                    }
                }
                PASSED;

                TESTING("analyzing fields on this \"Domain\" (each)");
                {
                    int whichFieldOnThisDomain;

                    for (whichFieldOnThisDomain=0; whichFieldOnThisDomain<numberOfFieldsOnThisDomain; ++whichFieldOnThisDomain) {
                        size_t          expectCount;
                        hid_t           expectType;
                        hbool_t         isSelfStored;
                        char           *name;
                        size_t          theCount;
                        hid_t           theType;

                        name = NULL;
                        saf_describe_field(SAF_EACH, listOfFieldsOnThisDomain+whichFieldOnThisDomain, NULL, &name, NULL, NULL, 
                                           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
                        if (!strcmp(name, "coords")) {
                            expectCount        = 3 * numberOfNodesPerDomain;
                            expectType         = H5T_NATIVE_FLOAT;
                            coords_d           = listOfFieldsOnThisDomain[whichFieldOnThisDomain];
                        } else if (!strcmp(name, "X")) {
                            expectCount        = 0;
                            expectType         = H5I_INVALID_HID;
                        } else if (!strcmp(name, "Y")) {
                            expectCount        = 0;
                            expectType         = H5I_INVALID_HID;
                        } else if (!strcmp(name, "Z")) {
                            expectCount        = 0;
                            expectType         = H5I_INVALID_HID;
                        } else if (!strcmp(name, "interleaved")) {
                            expectCount        = 3 * numberOfNodesPerDomain;
                            expectType         = H5T_NATIVE_FLOAT;
                            interleaved_d      = listOfFieldsOnThisDomain[whichFieldOnThisDomain];
                        } else if (!strcmp(name, "iX")) {
                            expectCount        = 0;
                            expectType         = H5I_INVALID_HID;
                        } else if (!strcmp(name, "iY")) {
                            expectCount        = 0;
                            expectType         = H5I_INVALID_HID;
                        } else if (!strcmp(name, "iZ")) {
                            expectCount        = 0;
                            expectType         = H5I_INVALID_HID;
                        } else if (!strcmp(name, "temperature")) {
                            expectCount        = numberOfNodesPerDomain;
                            expectType         = H5T_NATIVE_FLOAT;
                            temp_d             = listOfFieldsOnThisDomain[whichFieldOnThisDomain];
                        } else if (!strcmp(name, "pressure")) {
                            expectCount        = numberOfZonesPerDomain;
                            expectType         = H5T_NATIVE_FLOAT;
                            press_d            = listOfFieldsOnThisDomain[whichFieldOnThisDomain];
                        } else {
                            FAILED;
                            errorCount += 1;
                            if (P == 0) {
                                fprintf(stderr, "...expected field on \"%s\" to have name in \"coords\", \"X\", \"Y\", \"Z\", \"interleaved\", \"iX\", \"iY\", \"iZ\", \"temperature\", \"pressure\"\n", domainName);
                                fprintf(stderr, "   found %s\n", name);
                            }
                            goto theExit;
                        }
                        saf_is_self_stored_field(SAF_EACH, listOfFieldsOnThisDomain+whichFieldOnThisDomain, &isSelfStored);
                        if (!isSelfStored) {
                            FAILED;
                            errorCount += 1;
                            if (P == 0)
                                fprintf(stderr, "...expected field %s on \"%s\" to be stored on \"self\"\n", name, domainName);
                            goto theExit;
                        }
                        saf_get_count_and_type_for_field(SAF_EACH, listOfFieldsOnThisDomain+whichFieldOnThisDomain, NULL,
                                                         &theCount, &theType);
                        if (theCount != expectCount) {
                            FAILED;
                            errorCount += 1;
                            if (P == 0) {
                                fprintf(stderr, "...expected field %s on \"%s\" to have %ld DOF%s\n",
                                        name, domainName, (long)expectCount, (expectCount==1)?"":"s");
                                fprintf(stderr, "   found %ld DOF%s\n", (long)theCount, (theCount==1)?"":"s");
                            }
                            goto theExit;
                        }
                        if (0 < theCount) {
                            int             assoc_ratio;
                            SAF_Cat         coeff_assoc;
                            SAF_Field(comp_flds, p_comp_flds, 3);
                            SAF_Interleave  comp_interleave;
                            int             comp_order[3], *p_comp_order = comp_order;
                            hid_t           data_type;
                            SAF_Cat         eval_coll;
                            SAF_Eval        eval_func;
                            SAF_FieldTmpl   ftmpl;
                            hbool_t         is_coord;
                            int             num_comps;

                            if (!H5Tequal(theType, expectType)) {
                                FAILED;
                                errorCount += 1;
                                if (P == 0) {
                                    fprintf(stderr, "...wrong datatype for field %s on \"%s\"\n", name, domainName);
                                }
                                goto theExit;
                            }
                            num_comps = 3;
                            saf_describe_field(SAF_EACH, listOfFieldsOnThisDomain+whichFieldOnThisDomain, &ftmpl, NULL, NULL, 
                                               NULL, &is_coord, &storage_decomp, &coeff_assoc, &assoc_ratio, &eval_coll, 
                                               &eval_func, &data_type, &num_comps, &p_comp_flds, &comp_interleave, 
                                               &p_comp_order);
                            if (!_saf_is_self_decomp(&storage_decomp)) {
                                FAILED;
                                errorCount += 1;
                                if (P == 0)
                                    fprintf(stderr, "...expected field %s on \"%s\" to be stored on a decomposition\n", 
                                            name, domainName);
                                goto theExit;
                            }
                        }
                    }
                    free(listOfFieldsOnThisDomain);
                }
                PASSED;
            }
        }

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                       DEFAULT COORD FIELDS
         *--------------------------------------------------------------------------------------------------------------------- */
        TESTING("finding default coordinate fields on \"TheMesh\" (all)");
        {
            SAF_Field coords_on_mesh=SS_FIELD_NULL;

            saf_find_default_coords (SAF_ALL, &mesh, &coords_on_mesh);
            if (!SAF_EQUIV(&coords_on_mesh, &coords_mesh)) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr, "...wrong default coordinate field on \"TheMesh\"\n");
                goto theExit;
            }
        }
        PASSED;

        TESTING("finding default coordinate fields on this \"Domain\" (each)");
        {
            SAF_Field coords_on_domain;

            saf_find_default_coords(SAF_EACH, &domain, &coords_on_domain);
            if (!SAF_EQUIV(&coords_on_domain, &coords_d)) {
                FAILED;
                errorCount += 1;
                if (P == 0)
                    fprintf(stderr, "...wrong default coordinate field on \"%s\"\n", domainName);
            }
        }
        PASSED;

        /*------------------------------------------------------------------------------------------------------------------------ 
         *                                                           READ FIELDS
         *--------------------------------------------------------------------------------------------------------------------- */

        TESTING("reading \"direct\" fields (each)");
        {
            float *Cexpect;
            int    k, n, z;
            float *Iexpect;
            float *Pexpect;
            float *Texpect;
            float *Xexpect;
            float *Yexpect;
            float *Zexpect;

            Cexpect = (float*)malloc(3*numberOfNodesPerDomain*sizeof(float));
            Xexpect = Cexpect;
            Yexpect = Xexpect + numberOfNodesPerDomain;
            Zexpect = Yexpect + numberOfNodesPerDomain;
            Iexpect = (float*)malloc(3*numberOfNodesPerDomain*sizeof(float));
            Texpect = (float*)malloc(numberOfNodesPerDomain*sizeof(float));
            Pexpect = (float*)malloc(numberOfZonesPerDomain*sizeof(float));
            k = 0;
            for (n=0; n<numberOfNodesPerDomain; ++n) {
                int          i, j;
                int          nn;
                static float XX[4] = {0, 1, 1, 0};
                static float YY[4] = {0, 0, 1, 1};
                float        X, Y, Z;

                nn            = globalNodeNumber(P, n);
                i             = nn % 4;
                j             = nn / 4;
                X             = XX[i];
                Y             = YY[i];
                Z             = (float)(-j);
                Xexpect[n]    = X;
                Yexpect[n]    = Y;
                Zexpect[n]    = Z;
                Texpect[n]    = (float)sqrt(X*X+Y*Y+Z*Z);
                Iexpect[k+0]  = Xexpect[n];
                Iexpect[k+1]  = Yexpect[n];
                Iexpect[k+2]  = Zexpect[n];
                k            += 3;
            }
            for (z=0; z<numberOfZonesPerDomain; ++z)
                Pexpect[z] = globalZoneNumber(P, z);

            if (SS_FIELD(&coords_d)) {
                void   *buffer;
                int     i;
                double *p;
                SAF_FieldTarget coordsTarget;

                buffer = NULL;
                /* this is the only parameter used */
                saf_target_field(&coordsTarget, NULL, NULL, NULL, SAF_ANY_RATIO, NULL, NULL, H5T_NATIVE_DOUBLE,
                                 SAF_INTERLEAVE_NONE, NULL);
                saf_read_field(SAF_EACH, &coords_d, &coordsTarget, SAF_WHOLE_FIELD, &buffer);
                p = (double *)buffer;
                for (i=0; i<3*numberOfNodesPerDomain; ++i)
                    if (Cexpect[i] != (float)(p[i])) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected item %d of field \"coords\" on \"%s\" to be %f, found %f\n", i, domainName, Cexpect[i], p[i]);
                        goto theExit;
                    }
            }
            if (SS_FIELD(&interleaved_d)) {
                void   *buffer;
                int     i;
                double *p;
                SAF_FieldTarget interleavedTarget;

                buffer = NULL;
                /* this is the only parameter used */
                saf_target_field(&interleavedTarget, NULL, NULL, NULL, SAF_ANY_RATIO, NULL, NULL, H5T_NATIVE_DOUBLE,
                                 SAF_INTERLEAVE_NONE, NULL);
                saf_read_field(SAF_EACH, &interleaved_d, &interleavedTarget, SAF_WHOLE_FIELD, &buffer);
                p = (double *)buffer;
                for (i=0; i<3*numberOfNodesPerDomain; ++i)
                    if (Iexpect[i] != (float)(p[i])) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected item %d of field \"interleaved\" on \"%s\" to be %f, found %f\n", i, domainName, Cexpect[i], p[i]);
                        goto theExit;
                    }
            }
            if (SS_FIELD(&temp_d)) {
                void  *buffer;
                int    i;
                float *p;

                buffer = NULL;
                saf_read_field(SAF_EACH, &temp_d, NULL, SAF_WHOLE_FIELD, &buffer);
                p = (float *)buffer;
                for (i=0; i<numberOfNodesPerDomain; ++i)
                    if (p[i] != Texpect[i]) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected item %d of field \"temperature\" on \"%s\" to be %f, found %f\n", i, domainName, Texpect[i], p[i]);
                        goto theExit;
                    }
            }
            if (SS_FIELD(&press_d)) {
                void  *buffer;
                int    i;
                float *p;

                buffer = NULL;
                saf_read_field(SAF_EACH, &press_d, NULL, SAF_WHOLE_FIELD, &buffer);
                p = (float *)buffer;
                for (i=0; i<numberOfZonesPerDomain; ++i)
                    if (p[i] != Pexpect[i]) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected item %d of field \"pressure\" on \"%s\" to be %f, found %f\n", i, domainName, Pexpect[i], p[i]);
                        goto theExit;
                    }
            }
            free(Pexpect);
            free(Texpect);
            free(Iexpect);
            free(Cexpect);
        }
        PASSED;

        TESTING("reading \"indirect\" fields (all)");
        {
            if (SS_FIELD(&coords_mesh)) {
                void      *buffer;
                SAF_Field *f;

                buffer = NULL;
                saf_read_field(SAF_ALL, &coords_mesh, NULL, SAF_WHOLE_FIELD, &buffer);
                f = (SAF_Field *)buffer;
                if (!SAF_EQUIV(f+P, &coords_d)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected item %d of field \"coords\" on \"TheMesh\" to be field \"coords\" on \"%s\"\n", P, domainName);
                    goto theExit;
                }
            }
            if (SS_FIELD(&temp_mesh)) {
                void      *buffer;
                SAF_Field *f;

                buffer = NULL;
                saf_read_field(SAF_ALL, &temp_mesh, NULL, SAF_WHOLE_FIELD, &buffer);
                f = (SAF_Field *)buffer;
                /* we test for NOT_APPLICABLE_FIELD in the zeroth entry here */
                if ((P==0) && !SAF_EQUIV(f+0, SAF_NOT_APPLICABLE_FIELD)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected item 0 of field \"temperature\" on \"TheMesh\" to be SAF_NOT_APPLICABLE_FIELD\n");
                    goto theExit;
                }
                if ((P>0) && !SAF_EQUIV(f+P, &temp_d)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected item %d of field \"temperature\" on \"TheMesh\" to be field \"temperature\" on \"%s\"\n", P, domainName);
                    goto theExit;
                }
            }
            if (SS_FIELD(&press_mesh)) {
                void      *buffer;
                SAF_Field *f;

                buffer = NULL;
                saf_read_field(SAF_ALL, &press_mesh, NULL, SAF_WHOLE_FIELD, &buffer);
                f = (SAF_Field *)buffer;
                if (!SAF_EQUIV(f+P, &press_d)) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected item %d of field \"pressure\" on \"TheMesh\" to be field \"pressure\" on \"%s\"\n", P, domainName);
                    goto theExit;
                }
            }
        }
        PASSED;

        TESTING("reading \"direct\" fields on the mesh (all)");
#if 0   /* this is commented out until field remapping is fully implemented */
	/*  First check the nodal vector field "coords"... */        
        {
            {
                void     *buffer;
                size_t    expectCount;
                float    *p;
                size_t    theCount;
                hid_t     theType;

                expectCount = 3 * numberOfNodes;
                saf_target_field(&coords_mesh, /* When reading this field... */
                                 SAF_NOT_SET_UNIT, 
                                 SAF_SELF(db), /* ...we want to remap           */
                                 SAF_NOT_SET_CAT, 
                                 -1, 
                                 SAF_NOT_SET_CAT, 
                                 SAF_NOT_SET_EFUNC, 
                                 NULL, 
                                 SAF_INTERLEAVE_NONE, 
                                 NULL
                                 );
                saf_get_count_and_type_for_field(SAF_ALL, coords_mesh, 
                                                 &theCount, 
                                                 &theType
                                                 );
                if (theCount != expectCount) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected %lu DOF%s for field \"coords\" on \"TheMesh\", found %lu\n"
			        , (unsigned long)expectCount, (expectCount==1)?"":"s"
			        , (unsigned long)theCount
                                );
                    goto theExit;
                }
            }

            /*  Next check the nodal scalar field "temperature"... */            
            {
                size_t    expectCount;
                size_t    theCount;
                hid_t     theType;

                expectCount = numberOfNodes;
                saf_target_field(&temp_mesh, /* When reading this field... */
                                 SAF_NOT_SET_UNIT, 
                                 SAF_SELF(db), /* ...we want to remap           */
                                 SAF_NOT_SET_CAT, 
                                 -1, 
                                 SAF_NOT_SET_CAT, 
                                 SAF_NOT_SET_EFUNC, 
                                 NULL, 
                                 SAF_INTERLEAVE_NONE, 
                                 NULL
                                 );
                saf_get_count_and_type_for_field(SAF_ALL, temp_mesh, 
                                                 &theCount, 
                                                 &theType
                                                 );
                if (theCount != expectCount) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected %lu DOF%s for field \"temperature\" on \"TheMesh\", found %lu\n"
			        , (unsigned long)expectCount, (expectCount==1)?"":"s"
			        , (unsigned long)theCount
                                );
                    goto theExit;
                }
            }

            /*  Then check the zonal scalar field "pressure"... */            
            {  
                size_t    expectCount;
                size_t    theCount;
                hid_t     theType;

                expectCount = numberOfZones;
                saf_target_field(&press_mesh, /* When reading this field... */
                                 SAF_NOT_SET_UNIT, 
                                 SAF_SELF(db), /* ...we want to remap           */
                                 SAF_NOT_SET_CAT, 
                                 -1, 
                                 SAF_NOT_SET_CAT, 
                                 SAF_NOT_SET_EFUNC, 
                                 NULL, 
                                 SAF_INTERLEAVE_NONE, 
                                 NULL
                                 );
                saf_get_count_and_type_for_field(SAF_ALL, press_mesh, 
                                                 &theCount, 
                                                 &theType
                                                 );
                if (theCount != expectCount) {
                    FAILED;
                    errorCount += 1;
                    if (P == 0)
                        fprintf(stderr, "...expected %lu DOF%s for field \"pressure\" on \"TheMesh\", found %lu\n"
			        , (unsigned long)expectCount, (expectCount==1)?"":"s"
			        , (unsigned long)theCount
                                );
                    goto theExit;
                }

                {
                    void     *buffer;
                    buffer = NULL;
                    saf_read_field(SAF_ALL, press_mesh, SAF_WHOLE_FIELD, &buffer);
                    if (buffer == NULL) {
                        FAILED;
                        errorCount += 1;
                        if (P == 0)
                            fprintf(stderr, "...expected a buffer of %lu DOF%s for field \"pressure\" on \"TheMesh\"\n"
                                    , (unsigned long)expectCount, (expectCount==1)?"":"s"
                                    );
                        goto theExit;
                    }
                }
            }
        }
        PASSED;
#else
        SKIPPED;
#endif

    theExit:

        TESTING("database close");
        saf_close_database(db);
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
    /* make sure everyone returns the same error status */
    MPI_Bcast(&errorCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return (errorCount==0)? 0 : 1;
}

