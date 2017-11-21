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

#include <sys/types.h>
#include <sys/stat.h>
#include <saf.h>
#include <testutil.h>


/* Functions defined here to be available from elsewhere. */
int larry1w_main(int argc, char **argv, int skip_termination);
int larry1r_main(int argc, char **argv, int skip_initialization);

/* this function is needed for testing, here but is otherwise private to SAF and we don't want to include all of safP.h here */
extern hbool_t _saf_is_self_decomp(SAF_Cat *cat);

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Larry's Use Case Mains
 *
 * Description: The code for Larry's use case has been adapted from two separate clients; one for the writer and one for the
 * 		reader. In order to support testing of SAF's memory-resident mode (HDF5's core file driver), we needed a way
 *		for the writer to pass an open, in memory database to the reader so the reader could attempt to read it
 *		it back. So, both the original writer and reader clients' main()'s were moved to this file and renmamed.
 *		Now, the separate writer and reader clients are just wrappers to their respective subroutines, here.
 *
 *		Both /mains/ have an additional argument indicating whether or not to skip initialization and termination.
 *		This permits both the writer main and reader mean to execute in the same executable.
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/* we make this global to both writer and reader mains so that we can test core driver across both writer and reader */
static SAF_Db *db=NULL;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Larry's Use Case Writer
 *
 * Description: This is testing code that exercises larry's first use case. This code declares SAF objects and, optionally,
 *		*writes* the raw data. There are two command line arguments; DO_WRITES and DO_MULTIFLE.
 *
 *		Both tests larry1w.c and larry1.r are interdependent in that larry1w.c writes data that larry1r.c further
 *		reads and confirms is what is expected.
 *
 *		If DO_WRITES is *not* present, this test will only issue saf_declare_xxx() calls but not saf_write_xxx() calls
 *		If DO_WRITES is present, this test will also issue the saf_write_xxx() calls to write raw data for fields
 *		and relations.
 *
 *		If DO_MULTIFILE is present, the raw data will be written to different files. Otherwise, it will be written
 *		to the master file.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Mark Miller, LLNL, Wed Mar 15, 2000 
 *
 * Modifications: Matthew O'Brien 9/4/2001.  Added a new boundary set,
 *                ss1_boundary, to test more of the functionality of saf_find_sets
 *                in traverse.c
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
larry1w_main(int argc, char **argv, int skip_termination)
{
    char dbname[1024]="", *dbname_p=dbname;
    char tmpName[1024];
    hbool_t do_writes = false;
    hbool_t do_multifile = false;
    int i, self=0, ntasks=1;
    SAF_DbProps *dbprops=NULL;
    SAF_Suite cpbs;
    SAF_Cat nodes, elems, blocks, side_sets, node_sets, edges;

    SAF_Set top, cell_1, cell_2, cell_3, ss1, time_base, emptySet;
    SAF_Set myProcSet, myProcSub1, myProcSub2;
    SAF_Set ss2, ns1, cell_2_tri, cell_2_quad;
    /*a new boundary set, ss1_boundary, was added to test the more of the functionality of saf_find_sets*/
    SAF_Set ss1_boundary;
    SAF_Rel rel, trel;
    SAF_FieldTmpl coords_ftmpl, coords_ctmpl, distfac_ftmpl, temp1_ftmpl, time_ftmpl;
    SAF_FieldTmpl stress_ftmpl, stress_ctmpl, temp2_ftmpl, press_ftmpl, tmp_ftmpl[6];
    SAF_Field coords, coord_compon[2], distfac, temps1, temps2, disps, disp_compons[2];
    SAF_Field stress, ovw_stress, stress_compons[3], pressure, time_fld;
    SAF_Db *topofile=NULL;
    SAF_Db *seqfiles[5];
    SAF_Role SAF_USERD;
    SAF_Unit umeter;
    SAF_AltIndexSpec aspec, explicit_aspec, aspecwrite, index_cell_1_nodes;
    SAF_IndexSpec indxspec;
    int          passCount,failCount;
    int l_perturb=0, l_mem_inc=0, l_mem_save=0;
    hbool_t l_quiet = false;

    /* DONT PUT ANY CODE HERE SO WE CAN CONTINUE WITH VARIABLE DECLARATIONS UNIQUE TO PARALLEL */

#ifdef HAVE_PARALLEL
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &self);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
#endif
    ntasks = ntasks; /* quiet the compiler */

    STU_ProcessCommandLine(1, argc, argv,
                         
                           "do_multifile",
                           "if present, write data to different SAF files",
                           &do_multifile,
                         
                           "do_writes",
                           "if present, issue the saf_write_xxx() calls",
                           &do_writes,
                         
                           "-perturb %d",
                           "specify an integer from 0-6 specifying which perturbation (0 for none)",
                           &l_perturb,
                         
                           "-file %s",
                           "specify a filename instead of using the default",
                           &dbname_p,
                         
                           "-quiet",
                           "if present, print no commentary",
                           &l_quiet,
                         
                           "-mem_res %d %d",
                           "Use memory-resident mode. Specify reallocation increment in Kbytes and whether to save on close",
                           &l_mem_inc, &l_mem_save,
                         
                           STU_END_OF_ARGS);

#ifdef WIN32
	/*This doesnt work in WIN32 for now. 12jan2005*/
		do_multifile=0;l_mem_inc=0; l_mem_save=0;
#endif

    /* sanity checks */
    if(l_perturb<0) l_perturb=0; else if(l_perturb>6) l_perturb=6;
    if(l_mem_inc<0) l_mem_inc=0;
    if(l_mem_save!=0) l_mem_save=1;

    /* for convenience, set working directory to the test file directory */
    chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    saf_init(SAF_DEFAULT_LIBPROPS);

    /*only use TEST_FILE_NAME if the command-line didnt contain a filename*/
    if(!strlen(dbname)) strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {

        /* note: because we are in a try block here, all failures will send us to the one and only
	 catch block at the end of this test */

        dbprops = saf_createProps_database();
        saf_setProps_Clobber(dbprops);
        if (l_mem_inc && ntasks==1)
            saf_setProps_MemoryResident(dbprops);
        db = saf_open_database(dbname,dbprops);

        /* for multifile mode, we declare one file for topology (relations) and one file for
	 each time step. At present, this use case code writes only one time step */

        /* declare some supplemental files if we're in multifile mode */
        if (do_multifile) {
#ifdef HAVE_PARALLEL
            if (0==self) mkdir("testdata", 0777); /*might already exist*/
            MPI_Barrier(MPI_COMM_WORLD);
#else
            mkdir("testdata", 0777); /*might already exist*/
#endif

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
            for (i = 0; i < 5; i++)
                seqfiles[i] = db;
	}

        passCount = 0;
        failCount = 0;

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE ROLES
         *--------------------------------------------------------------------------------------------------------------------- */
        saf_declare_role(SAF_ALL, db, "larry1w user defined", "LUC", &SAF_USERD); /*LUC=Larry's Use Case*/
    
        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE CATEGORIES
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING("declaring categories");
        saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, 0, &nodes);
        saf_declare_category(SAF_ALL, db, "elems", SAF_TOPOLOGY, 2, &elems);
        saf_declare_category(SAF_ALL, db, "edges", &SAF_USERD, 1, &edges);
        saf_declare_category(SAF_ALL, db, "blocks", SAF_BLOCK, 2, &blocks);
        saf_declare_category(SAF_ALL, db, "side_sets", &SAF_USERD, 1, &side_sets);
        saf_declare_category(SAF_ALL, db, "node_sets", &SAF_USERD, 0, &node_sets);

        if(l_perturb==5) {
            /*perturb 5 is an extra category*/
            SAF_Cat l_extra;
            saf_declare_category(SAF_ALL, db, "extra_cat", SAF_TOPOLOGY, 3, &l_extra);
	}

        if(!l_quiet) PASSED;
        passCount += 1;

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE SETS 
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING("declaring sets");
        saf_declare_set(SAF_ALL, db, "empty set", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &emptySet);
        saf_declare_set(SAF_ALL, db, "TOP_CELL", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &top);
        saf_declare_set(SAF_ALL, db, "CELL_1", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_1);
        saf_declare_set(SAF_ALL, db, "CELL_2", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_2);
        saf_declare_set(SAF_ALL, db, "CELL_3", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_3);
        saf_declare_set(SAF_ALL, db, "SIDE_SET_1", 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ss1);
        saf_declare_set(SAF_ALL, db, "SIDE_SET_1_BOUNDARY", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ss1_boundary);
        saf_declare_set(SAF_ALL, db, "SIDE_SET_2", 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ss2);
        saf_declare_set(SAF_ALL, db, "NODE_SET_1", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ns1);
        saf_declare_set(SAF_ALL, db, "TIME", 1, SAF_TIME, SAF_EXTENDIBLE_TRUE, &time_base);

        /* we'll need the following sets to deal with inhomogeneous cell type on CELL_2 */
        saf_declare_set(SAF_ALL, db, "CELL_2_TRIS", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_2_tri);
        saf_declare_set(SAF_ALL, db, "CELL_2_QUADS", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_2_quad);
        if(!l_quiet) PASSED;
        passCount += 1;


        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE SAF_EACH SETS 
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING("declaring some SAF_EACH mode sets");
        /* some each mode tests */
        sprintf(tmpName,"proc %02d", self);
        saf_declare_set(SAF_EACH, db, tmpName, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &myProcSet);
        sprintf(tmpName,"proc %02d - sub1", self);
        saf_declare_set(SAF_EACH, db, tmpName, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &myProcSub1);
        sprintf(tmpName,"proc %02d - sub2", self);
        saf_declare_set(SAF_EACH, db, tmpName, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &myProcSub2);
        if(!l_quiet) PASSED;
        passCount += 1;

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE COLLECTIONS
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING("declaring collections");
        saf_declare_collection(SAF_ALL, &top, &nodes, SAF_CELLTYPE_POINT, 18, SAF_1DF(18), SAF_DECOMP_FALSE); 
        saf_declare_collection(SAF_ALL, &top, &elems, SAF_CELLTYPE_MIXED, 12, SAF_1DF(12), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &top, &blocks, SAF_CELLTYPE_SET, 4, SAF_1DF(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &top, &side_sets, SAF_CELLTYPE_SET, 2, SAF_1DF(2), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &top, &node_sets, SAF_CELLTYPE_SET, 1, SAF_1DF(1), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_1, &nodes, SAF_CELLTYPE_POINT, 9, SAF_1DF(9), SAF_DECOMP_FALSE);

        if(l_perturb==1) /*perturb 1 is one less quad on a collection*/
            saf_declare_collection(SAF_ALL, &cell_1, &elems, SAF_CELLTYPE_QUAD, 3, SAF_1DF(3), SAF_DECOMP_TRUE);
        else
            saf_declare_collection(SAF_ALL, &cell_1, &elems, SAF_CELLTYPE_QUAD, 4, SAF_1DF(4), SAF_DECOMP_TRUE);

        saf_declare_collection(SAF_ALL, &cell_1, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DF(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2, &nodes, SAF_CELLTYPE_POINT, 7, SAF_1DF(7), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_2, &elems, SAF_CELLTYPE_MIXED, 4, SAF_1DF(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2, &blocks, SAF_CELLTYPE_SET, 2, SAF_1DF(2), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_3, &nodes, SAF_CELLTYPE_POINT, 10, SAF_1DF(10), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_3, &elems, SAF_CELLTYPE_QUAD, 4, SAF_1DF(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_3, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DF(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &ss1, &nodes, SAF_CELLTYPE_POINT, 3, SAF_1DF(3), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &ss1, &edges, SAF_CELLTYPE_LINE, 2, SAF_1DF(2), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &ss1_boundary, &nodes, SAF_CELLTYPE_POINT, 2, SAF_1DF(2), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &ss2, &nodes, SAF_CELLTYPE_POINT, 5, SAF_1DF(5), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &ss2, &edges, SAF_CELLTYPE_LINE, 4, SAF_1DF(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &ns1, &nodes, SAF_CELLTYPE_POINT, 5, SAF_1DF(5), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2_tri, &nodes, SAF_CELLTYPE_POINT, 5, SAF_1DF(5), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_2_tri, &elems, SAF_CELLTYPE_TRI, 3, SAF_1DF(3), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2_tri, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DF(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2_quad, &nodes, SAF_CELLTYPE_POINT, 4, SAF_1DF(4), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_2_quad, &elems, SAF_CELLTYPE_QUAD, 1, SAF_1DF(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2_quad, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DF(1), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &time_base, &nodes, SAF_CELLTYPE_POINT, 0, SAF_1DC(0), SAF_DECOMP_FALSE);


        if(l_perturb==6) {
            /*perturb 6 is an extra collection*/
            saf_declare_collection(SAF_ALL, &ss1, &elems, SAF_CELLTYPE_QUAD, 3, SAF_1DF(3), SAF_DECOMP_FALSE);
	}


        if(!l_quiet) PASSED;
        passCount += 1;

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE SAF_EACH COLLECTIONS 
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING("declaring some SAF_EACH mode collections");
        saf_declare_collection(SAF_EACH, &myProcSet,  &nodes, SAF_CELLTYPE_POINT, (self+1)*3, SAF_1DF((self+1)*3),
                               SAF_DECOMP_FALSE); 
        saf_declare_collection(SAF_EACH, &myProcSub1, &nodes, SAF_CELLTYPE_POINT, (self+1)*2, SAF_1DF((self+1)*2),
                               SAF_DECOMP_FALSE); 
        saf_declare_collection(SAF_EACH, &myProcSub2, &nodes, SAF_CELLTYPE_POINT, (self+1)*1, SAF_1DF((self+1)*1),
                               SAF_DECOMP_FALSE); 
        if(!l_quiet) PASSED;
        passCount += 1;

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE ALTERNATE INDEX SPECIFICATIONS
         *--------------------------------------------------------------------------------------------------------------------- */


        if(!l_quiet) TESTING("declaring  alternate index specifications");
        {
            indxspec = SAF_1DF(12);
            saf_declare_alternate_indexspec(SAF_ALL, db, &top, &nodes, "first", H5T_NATIVE_INT, true,
                                            SAF_NA_INDEXSPEC,true,false, &aspec);

            saf_declare_alternate_indexspec(SAF_ALL, db, &top, &elems, "first", H5T_NATIVE_INT, true,
                                            SAF_NA_INDEXSPEC,true,false, &aspec);
            saf_declare_alternate_indexspec(SAF_ALL, db, &top, &elems, "second", H5T_NATIVE_INT, true,
                                            SAF_NA_INDEXSPEC,false,false, &aspecwrite);
            saf_declare_alternate_indexspec(SAF_ALL, db, &top, &elems, "third", H5T_NATIVE_INT, true,
                                            SAF_NA_INDEXSPEC,true,true, &aspec);
            saf_declare_alternate_indexspec(SAF_ALL, db, &top, &elems, "fourth", H5T_NATIVE_INT, true,
                                            SAF_NA_INDEXSPEC,false,true, &explicit_aspec);

            saf_declare_alternate_indexspec(SAF_ALL, db, &cell_1, &nodes, "cell_1 nodes", H5I_INVALID_HID, true,
                                            SAF_NA_INDEXSPEC,false,false, &index_cell_1_nodes);

            saf_declare_alternate_indexspec(SAF_ALL, db, &top, &elems,"fifth implicit", H5T_NATIVE_INT, false,
                                            indxspec,true,true, &aspec);


        }
        if(!l_quiet) PASSED; 

        if(do_writes){
            if(!l_quiet) TESTING("writing    alternate index specifications");
            {
                /*the collection count is 12*/
                int buf[] = {2,3,5,7,11,13,17,19,23,29,31,37};
                /*the collection count is 9*/
                int buf2[] = {8,6,4,2,0,7,5,3,1};
                saf_write_alternate_indexspec(SAF_ALL, &aspecwrite, H5I_INVALID_HID, buf, topofile);
                saf_write_alternate_indexspec(SAF_ALL, &index_cell_1_nodes, H5T_NATIVE_INT,buf2, topofile);
	
                if(!l_quiet) PASSED;
            }
        }

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE SAF_GENERAL SUBSET RELATION
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING("declaring a SAF_GENERAL subset relation");

        saf_declare_subset_relation(SAF_ALL, db, &top, &emptySet, SAF_GENERAL(SAF_BOUNDARY_FALSE), SAF_TUPLES, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);

        if(!l_quiet) PASSED;
        passCount += 1;


        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE AND WRITE SAF_EACH SUBSET RELATIONS 
         *                                                   (buf in write call)
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING(do_multifile?
                             do_writes?
                             "writing SAF_EACH subset relations [buf in write] [multifile]"
                             :
                             "declaring SAF_EACH subset relations [buf in write]"
                             :
                             do_writes?
                             "writing SAF_EACH subset relations [buf in write]"
                             :
                             "declaring SAF_EACH subset relations [buf in write]");


        /* each mode tests */
        saf_declare_subset_relation(SAF_EACH, db, &top, &myProcSet, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int i, buf[128];
            for (i = 0; i < (self+1)*3; i++)
                buf[i] = i;
            saf_write_subset_relation(SAF_EACH, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_EACH, db, &myProcSet, &myProcSub1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int i, buf[128];
            for (i = 0; i < (self+1)*2; i++)
                buf[i] = i*2;
            saf_write_subset_relation(SAF_EACH, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_EACH, db, &myProcSet, &myProcSub2, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int i, buf[128];
            for (i = 0; i < (self+1)*1; i++)
                buf[i] = i;
            saf_write_subset_relation(SAF_EACH, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        if(!l_quiet) PASSED;
        passCount += 1;


        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE AND WRITE SUBSET RELATIONS 
         *                                                   (buf in write call)
         *--------------------------------------------------------------------------------------------------------------------- */
        /* Note: The declare/write calls are written in the manner they are so that one can easily
         *       associate the data being written with the object (field or relation) it is for. If all the
         *       saf_write_xxx() calls are moved to the end, after the declare calls, this association would
	 be harder to see */


        if(!l_quiet) TESTING(do_multifile?
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
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) { 
            int buf[] = {1,2,3,5,6,7,9,10,11};

            if(l_perturb==2) {
                /*perturb 2 is a change in subset rel values*/
                buf[0]=2;
                buf[1]=1;
	    }
	
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {1,2,4,5};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}

        /* nodes and elems of cell_2 and top */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {9,10,11,13,14,16,17};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {7,8,9,11}; 
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}

        /* nodes and elems of cell_2_tri & cell_2_quad */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {9,10,11,13,14}; 
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {7,8,9}; 
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {13,14,16,17}; 
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {11}; 
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}

        /*cell_2_tri and cell_2_quad are subsets of cell_2,
	this was added to test saf_find_sets, so cell_2_tri has multiple super-sets */
        saf_declare_subset_relation(SAF_ALL, db, &cell_2, &cell_2_tri, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {9,10,11,13,14}; 
            /*this numbering scheme is relative to top, should it be relative to cell_2?
	  i.e. should it be {1,2,3,4,5} ? */
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &cell_2, &cell_2_tri, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {7,8,9}; 
            /*this numbering scheme is relative to top, should it be relative to cell_2?
	  i.e. should it be {1,2,3} ? */
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &cell_2, &cell_2_quad, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {13,14,16,17};
            /*this numbering scheme is relative to top, should it be relative to cell_2?
	  i.e. should it be {4,5,6,7} ? */
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &cell_2, &cell_2_quad, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {11}; 
            /*this numbering scheme is relative to top, should it be relative to cell_2?
	  i.e. should it be {4} ? */
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
    

        /* nodes and elems of cell_3 */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {3,4,7,8,11,12,14,15,17,18};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {3,6,10,12}; 
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        if(!l_quiet) PASSED;
        passCount += 1;

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE AND WRITE SUBSET RELATIONS 
         *                                                   (buf in declare call)
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING(do_multifile?
                             do_writes?
                             "writing subset relations [buf in declare] [multifile]"
                             :
                             "declaring subset relations [buf in declare]"
                             :
                             do_writes?
                             "writing subset relations [buf in declare]"
                             :
                             "declaring subset relations [buf in declare]");


        /* nodes of side set 1 */
        {
            int buf[] = {9,10,11}; 

            saf_declare_subset_relation(SAF_ALL, db, &top, &ss1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, buf, H5I_INVALID_HID, NULL, &rel);
            if (do_writes)
                saf_write_subset_relation(SAF_ALL, &rel, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, topofile);
        }

        /*declaring ss1_boundary as the boundary of ss1 */
        {
            int buf[] = {9,11}; 

            saf_declare_subset_relation(SAF_ALL, db, &ss1, &ss1_boundary, SAF_BOUNDARY(&nodes,&nodes), SAF_TUPLES, SAF_INT, buf, H5I_INVALID_HID, NULL, &rel);
            if (do_writes)
                saf_write_subset_relation(SAF_ALL, &rel, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, topofile);

        }


        /* nodes of side set 2 */
        {
            int buf[] = {1,5,9,13,16}; 

            saf_declare_subset_relation(SAF_ALL, db, &top, &ss2, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, buf, H5I_INVALID_HID, NULL, &rel);
            if (do_writes)
                saf_write_subset_relation(SAF_ALL, &rel, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, topofile);
        }

        /* nodes of node set 1 */
        {
            int buf[] = {4,8,12,15,18}; 

            saf_declare_subset_relation(SAF_ALL, db, &top, &ns1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, buf, H5I_INVALID_HID, NULL, &rel);
            if (do_writes)
                saf_write_subset_relation(SAF_ALL, &rel, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, topofile);
        }
        if(!l_quiet) PASSED;
        passCount += 1;

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE AND WRITE SUBSET RELATIONS FOR BLOCKS
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING(do_multifile?
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
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {0};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_tri, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {1};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2_quad, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {2};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_3, SAF_COMMON(&blocks), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID, NULL, &rel);
        if (do_writes) {
            int buf[] = {3};
            saf_write_subset_relation(SAF_ALL, &rel, H5T_NATIVE_INT, buf, H5I_INVALID_HID, NULL, topofile);
	}
        if(!l_quiet) PASSED;
        passCount += 1;




        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE AND WRITE TOPOLOGY RELATIONS 
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING(do_multifile?
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
        saf_declare_topo_relation(SAF_ALL, db, &cell_1, &elems, &top, &nodes, SAF_SELF(db), &cell_1,
                                  SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);

        if (do_writes) {
            int abuf[] = {4};
            int bbuf[] = {1,2,6,5,2,3,7,6,5,6,10,9,6,7,11,10};
            saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, topofile);
	}

        /* for cell_2_tri */
        saf_declare_topo_relation(SAF_ALL, db, &cell_2_tri, &elems, &top, &nodes, SAF_SELF(db), &cell_2_tri,
                                  SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
        if (do_writes) {
            int abuf[] = {3};
            int bbuf[] = {9,10,13,10,14,13,10,11,14};
            saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, topofile);
	}

        /* for cell_2_quad */
        saf_declare_topo_relation(SAF_ALL, db, &cell_2_quad, &elems, &top, &nodes, SAF_SELF(db), &cell_2_quad,
                                  SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
        if (do_writes) {
            int abuf[] = {4};
            int bbuf[] = {13,14,17,16};
            saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, topofile);
	}
        if(!l_quiet) PASSED;
        passCount += 1;

        /* for cell_3... we do cell_3 a little differently so that we can also test another function,
	 saf_get_count_and_type_for_topo_relation */
        if(!l_quiet) TESTING("saf_get_count_and_type_for_topo_relation before and after writing");
        {
            size_t a_count, b_count;
            hid_t a_type, b_type;
            hbool_t failed = false;
            int abuf[] = {4};
            int bbuf[] = {3,4,8,7,  7,8,12,11,  11,12,15,14,  14,15,18,17};

            /* for cell_3 */
            saf_declare_topo_relation(SAF_ALL, db, &cell_3, &elems, &top, &nodes, SAF_SELF(db), &cell_3,
                                      SAF_UNSTRUCTURED, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, &trel);

            a_count = b_count = 0;
            a_type = b_type = H5I_INVALID_HID;
            saf_get_count_and_type_for_topo_relation(SAF_ALL, &trel, NULL, NULL, &a_count, &a_type, &b_count, &b_type);
            if (a_count != 1 || !H5Tequal(a_type, H5T_NATIVE_INT) || b_count != 16 || !H5Tequal(b_type, H5T_NATIVE_INT))
                failed = true;

            if (do_writes)
                saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, abuf, H5T_NATIVE_INT, bbuf, topofile);

            a_count = b_count = 0;
            a_type = b_type = H5I_INVALID_HID;
            saf_get_count_and_type_for_topo_relation(SAF_ALL, &trel, NULL, NULL, &a_count, &a_type, &b_count, &b_type);
            if (a_count != 1 || !H5Tequal(a_type, H5T_NATIVE_INT) || b_count != 16 || !H5Tequal(b_type, H5T_NATIVE_INT))
                failed = true;

            if (failed) {
                FAILED;
                failCount++;
            } else {
                if(!l_quiet) PASSED;
                passCount++;
            }
        }

        {
            /*do topo rels for side sets*/
            int side_set_edge_node_ct[] = {2};
            int ss1_connectivity[] = {1,2,2,3};
            int ss2_connectivity[] = {1,2,2,3,3,4,4,5};
	
            /* connectivity of the nodes of the top in the edges of side set 1 */
            saf_declare_topo_relation(SAF_ALL, db, &ss1, &edges, &top, &nodes,
                                      SAF_SELF(db), &ss1, SAF_UNSTRUCTURED, H5I_INVALID_HID,
                                      NULL, H5I_INVALID_HID, NULL, &trel);
            saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, side_set_edge_node_ct,
                                    H5T_NATIVE_INT, ss1_connectivity,    topofile);
	
            /* connectivity of the nodes of the top in the edges of side set 2 */
            saf_declare_topo_relation(SAF_ALL, db, &ss2, &edges, &top, &nodes,
                                      SAF_SELF(db), &ss2, SAF_UNSTRUCTURED, H5I_INVALID_HID,
                                      NULL, H5I_INVALID_HID, NULL, &trel);
            saf_write_topo_relation(SAF_ALL, &trel, H5T_NATIVE_INT, side_set_edge_node_ct,
                                    H5T_NATIVE_INT, ss2_connectivity,    topofile);
        }



        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE FIELD TEMPLATES 
         *--------------------------------------------------------------------------------------------------------------------- */
        if(!l_quiet) TESTING("declaring field templates");

        /* for coordinate field components */ /* top */
        saf_declare_field_tmpl(SAF_ALL, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
                               NULL, &coords_ctmpl);
    
        /* fill array of component field templates for the composite field template */
        tmp_ftmpl[0] = coords_ctmpl;
        tmp_ftmpl[1] = coords_ctmpl;

        /* for coordinate fields */ /* top */
        saf_declare_field_tmpl(SAF_ALL, db, "coordinate_tmpl", SAF_ALGTYPE_VECTOR, SAF_CARTESIAN, SAF_QLENGTH, 2,
                               tmp_ftmpl, &coords_ftmpl);
    
        /* for distribution factors */ /* ss2 */
        saf_declare_field_tmpl(SAF_ALL, db, "distrib_factors_tmpl", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_NOT_APPLICABLE_QUANTITY, 1,
                               NULL, &distfac_ftmpl);

        /* for temperature on ns1 */ /* ns1 */
        saf_declare_field_tmpl(SAF_ALL, db, "temp_on_ns1_tmpl", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QTEMP, 1,
                               NULL, &temp1_ftmpl);

        /* for stress components */ /* cell_1 */
        saf_declare_field_tmpl(SAF_ALL, db, "scalar on cell_1", SAF_ALGTYPE_SCALAR, SAF_UNITY,
                               SAF_QNAME(db,"pressure"), 1, NULL, &stress_ctmpl);

        /* fill array of component field templates for the composite field template */
        tmp_ftmpl[0] = stress_ctmpl;
        tmp_ftmpl[1] = stress_ctmpl;
        tmp_ftmpl[2] = stress_ctmpl;

        /* for stress */ /* cell_1 */
        saf_declare_field_tmpl(SAF_ALL, db, "upper-tri on cell_1", SAF_ALGTYPE_SYMTENSOR, SAF_UPPERTRI,
                               SAF_QNAME(db,"pressure"), 3, tmp_ftmpl, &stress_ftmpl);

        /* for temperature on nodes of cell_2 */ /* cell_2 */
        saf_declare_field_tmpl(SAF_ALL, db, "temp_on_cell_2_tmpl", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QTEMP, 1,
                               NULL, &temp2_ftmpl);

        /* for pressure on elemenst of side set 1 */ /* ss1 */
        saf_declare_field_tmpl(SAF_ALL, db, "pressure_on_ss1", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QNAME(db,"pressure"),
                               1, NULL, &press_ftmpl);

        /* for time on time_base */ /* time_base */
        saf_declare_field_tmpl(SAF_ALL, db, "time_on_time_base", SAF_ALGTYPE_SCALAR, SAF_UNITY, SAF_QTIME,
                               1, NULL, &time_ftmpl);

        if(!l_quiet) PASSED;
        passCount += 1;

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE AND WRITE FIELDS
         *                                                   (buf specified in write call)
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            SAF_Unit ukelvin;

            saf_find_one_unit(db, "meter", &umeter);
            saf_find_one_unit(db, "kelvin", &ukelvin);

            if(!l_quiet) TESTING(do_multifile?
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
            saf_declare_field(SAF_ALL, db, &coords_ctmpl, "X", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                              H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coord_compon[0]));
            saf_declare_field(SAF_ALL, db, &coords_ctmpl, "Y", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                              H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coord_compon[1]));
            saf_declare_field(SAF_ALL, db, &coords_ftmpl, "coords", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                              H5T_NATIVE_FLOAT, coord_compon, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coords); 
            if (do_writes) {
                float buf[] = {0.,4., 1.,4., 2.,4., 2.5,4.,
                               0.,3., 1.,3., 2.,3., 2.5,3.,
                               0.,2., 1.,2., 2.,2., 2.5,2.,
                               0.,1.,        2.,1., 2.5,1.,
                               0.,0.,        2.,0., 2.5,0.};
                void *pbuf = &buf[0];

                if(l_perturb==3) {
                    /*perturb 3 is a 5% changed field value*/
                    buf[10] *= 1.05;
                }

                saf_write_field(SAF_ALL, &coords, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]); 
            }

            /* specify that is a coordinate field */
            saf_declare_coords(SAF_ALL, &coords);
            saf_declare_default_coords(SAF_ALL, &top, &coords);

            /* made up distribution factors on ss2 */
            saf_declare_field(SAF_ALL, db, &distfac_ftmpl, "distribution factors", &ss2, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(db), 
                              SAF_NODAL(&nodes, &edges), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &distfac);
            if (do_writes) {
                float buf[] = {4., 3., 2., 1., 0.};
                void *pbuf = &buf[0];
                saf_write_field(SAF_ALL, &distfac, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
            }

            /* made up temperatures */
            saf_declare_field(SAF_ALL, db, &temp1_ftmpl, "temperature", &ns1, &ukelvin, SAF_SELF(db), SAF_NODAL(&nodes, &nodes),
                              H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &temps1);
            if (do_writes) {
                float buf[] = {100., 150., 150., 100., 75.};
                void *pbuf = &buf[0];
                saf_write_field(SAF_ALL, &temps1, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
            }

            /* made up displacements */
            saf_declare_field(SAF_ALL, db, &coords_ctmpl, "dX", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                              H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[0]));
            saf_declare_field(SAF_ALL, db, &coords_ctmpl, "dY", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                              H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(disp_compons[1]));
            saf_declare_field(SAF_ALL, db, &coords_ftmpl, "displacements", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                              H5T_NATIVE_FLOAT, disp_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &disps);
            if (do_writes) {
                float buf[] = {.25,.25,   .25,.25,   .25,.25,   .25,.25,
                               .25,.25,   .25,.25,   .25,.25,   .25,.25,
                               .25,.25,   .25,.25,   .25,.25,   .25,.25,
                               .25,.25,              .25,.25,   .25,.25,
                               .25,.25,              .25,.25,   .25,.25};
                void *pbuf = &buf[0];
                saf_write_field(SAF_ALL, &disps, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
            }
            if(!l_quiet) PASSED;
            passCount += 1;
        }


        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   DECLARE AND WRITE FIELDS
         *                                                   (buf specified in declare call)
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            SAF_Unit upascal;
            SAF_Unit upsi;
            SAF_Unit ukelvin;

            saf_find_one_unit(db, "pascal", &upascal);
            saf_find_one_unit(db, "psi", &upsi);
            saf_find_one_unit(db, "kelvin", &ukelvin);

            if(!l_quiet) TESTING(do_multifile?
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
            {

                /*float buf[] = {0.5, 0.25, 0.5,0.5, 0.25, 0.5,
                 * 0.5, 0.25, 0.5,0.5, 0.25, 0.5};
	    void *pbuf = &buf[0]; */

                saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sx", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[0]));
                saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sy", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[1]));
                saf_declare_field(SAF_ALL, db, &stress_ctmpl, "Sxy", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[2]));

#if 0
                /*the original way: write to the composite field*/
                saf_declare_field(SAF_ALL, db, &stress_ftmpl, "stress", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, stress_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, &pbuf, &stress);

                if (do_writes)
                    saf_write_field(SAF_ALL, &stress, SAF_WHOLE_FIELD, 0, H5I_INVALID_HID, NULL, seqfiles[0]);
#else
                /*the new way: write to each individual component, to test the ability of saf_read_field()
	   *to get the composite field from the components */
                saf_declare_field(SAF_ALL, db, &stress_ftmpl, "stress", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, stress_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &stress);
	  
                if (do_writes) {
                    float l_buf0[] = { 0.5, 0.5, 0.5, 0.5 };
                    float l_buf1[] = { 0.25, 0.25, 0.25, 0.25 };
                    float l_buf2[] = { 0.5, 0.5, 0.5, 0.5 };
                    void *l_pbuf = &l_buf0[0];
                    saf_write_field(SAF_ALL, stress_compons+0, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &l_pbuf, seqfiles[0]);
                    l_pbuf = &l_buf1[0];
                    saf_write_field(SAF_ALL, stress_compons+1, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &l_pbuf, seqfiles[0]);
                    l_pbuf = &l_buf2[0];
                    saf_write_field(SAF_ALL, stress_compons+2, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &l_pbuf, seqfiles[0]);
                }
#endif
            }

            /* made up temperatures */
            {
                float buf[] = {75., 95., 120., 80., 115., 85., 110.};
                void *pbuf = &buf[0];

                saf_declare_field(SAF_ALL, db, &temp2_ftmpl, "temperature", &cell_2, &ukelvin, SAF_SELF(db), 
                                  SAF_NODAL(&nodes, &elems), H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, &pbuf, &temps2);
                if (do_writes)
                    saf_write_field(SAF_ALL, &temps2, SAF_WHOLE_FIELD, 0, H5I_INVALID_HID, NULL, seqfiles[0]);
            }

            /* made up pressures */
            {
                float buf[] = {45., 55.};
                void *pbuf = &buf[0];

                saf_declare_field(SAF_ALL, db, &press_ftmpl, "pressure", &ss1, &upsi, SAF_SELF(db), SAF_ZONAL(&edges),
                                  H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, &pbuf, &pressure);
                if (do_writes)
                    saf_write_field(SAF_ALL, &pressure, SAF_WHOLE_FIELD, 0, H5I_INVALID_HID, NULL, seqfiles[0]);
            }

            if(!l_quiet) PASSED;
            passCount += 1;

        }

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   A CONSTANT FIELD
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            SAF_Field cg_comps[2], centerOfGravity;

            if(!l_quiet) TESTING("declaring and writing a constant field");
            saf_declare_field(SAF_ALL, db, &coords_ctmpl, "Xcg", &top, &umeter, SAF_SELF(db), SAF_CONSTANT(db),
                              H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(cg_comps[0]));
            saf_declare_field(SAF_ALL, db, &coords_ctmpl, "Ycg", &top, &umeter, SAF_SELF(db), SAF_CONSTANT(db),
                              H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(cg_comps[1]));
            saf_declare_field(SAF_ALL, db, &coords_ftmpl, "CG", &top, &umeter, SAF_SELF(db), SAF_CONSTANT(db),
                              H5T_NATIVE_FLOAT, cg_comps, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &centerOfGravity);
            if (do_writes) {
                float buf[] = {2.,2.};
                void *pbuf = &buf[0];
                saf_write_field(SAF_ALL, &centerOfGravity, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
            }
            if(!l_quiet) PASSED;
            passCount += 1;
        }

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   EXTENDING A COLLECTION 
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            if(!l_quiet) TESTING("extending a collection and getting count and type on a field");
            saf_extend_collection(SAF_ALL, &time_base, &nodes, 10, SAF_1DC(10));
            if(!l_quiet) PASSED;
        }

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   PARTIAL FIELD WRITE
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            SAF_Unit usec;
       
            saf_find_one_unit(db, "second", &usec);
            saf_extend_collection(SAF_ALL, &time_base, &nodes, 1, SAF_1DC(1));

            if(!l_quiet) TESTING(do_multifile?
                                 do_writes?
                                 "partial field write [buf in write] [multifile]"
                                 :
                                 "partial field write [buf in write]"
                                 :
                                 do_writes?
                                 "partial field write [buf in write]"
                                 :
                                 "partial field write [buf in write]");

            /* made up times */
            {
                float buf[]  = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0};
                float buf1[] = {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5, 13.5};
                int members[3] = {0,0,1};
                void *pbuf = NULL; 
                hid_t type;
                size_t cnt;

                saf_declare_field(SAF_ALL, db, &time_ftmpl, "times", &time_base, &usec, SAF_SELF(db), SAF_NODAL(&nodes, &nodes),
                                  H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &time_fld);

                saf_get_count_and_type_for_field(SAF_ALL, &time_fld, NULL, &cnt, &type);
                if ((cnt != 0) || (type>0)) {
                    FAILED;
                    goto overWriteTest;
                }

                /* NOTE: we declared the nodes on time with size of 1 and then extended by 10. So total nodes on time
 	         * at this point is 11. */

                /* indicate this is a coordinate field for the time base */
                saf_declare_coords(SAF_ALL, &time_fld);
                saf_declare_default_coords(SAF_ALL, &time_base, &time_fld);

                /* first, lets try to write the whole field as a partial request and see what happens */
                pbuf = &buf[0];
                members[0] = 0; /* start at 0 */
                members[1] = 11; /* count of 11 */
                if (do_writes) {
                    saf_write_field(SAF_ALL, &time_fld, 11, SAF_HSLAB, members, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);

                    saf_get_count_and_type_for_field(SAF_ALL, &time_fld, NULL, &cnt, &type);
                    if ((cnt != 11) || !H5Tequal(type, H5T_NATIVE_FLOAT)) {
                        FAILED;
                        goto overWriteTest;
                    }
                }

                /* ok, now lets try to partial overwrite the first three with contents of buf1 */
                pbuf = &buf1[0];
                members[0] = 0; /* start at 0 */
                members[1] = 3; /* count of 3 */
                if (do_writes)
                    saf_write_field(SAF_ALL, &time_fld, 3, SAF_HSLAB, members, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);

                /* ok, now lets overwrite member 4 with tuple mode with contents of buf1 */
                pbuf = &buf1[4];
                members[0] = 4; /* write member 4 */
                if (do_writes)
                    saf_write_field(SAF_ALL, &time_fld, 1, SAF_TUPLES, members, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);

                /* now, lets extend the collection again and do another write on the very last member */
                saf_extend_collection(SAF_ALL, &time_base, &nodes, 3, SAF_1DC(3));

                pbuf = &buf1[13];
                members[0] = 13; /* write member 13 */
                if (do_writes) {
                    saf_write_field(SAF_ALL, &time_fld, 1, SAF_TUPLES, members, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
	      
                    saf_get_count_and_type_for_field(SAF_ALL, &time_fld, NULL, &cnt, &type);
                    if ((cnt != 14) || !H5Tequal(type, H5T_NATIVE_FLOAT)) {
                        FAILED;
                        goto overWriteTest;
                    }
                }
            }

            if(!l_quiet) PASSED;
        }

    overWriteTest:
        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   OVER- WRITE FIELDS
         *                                                   (buf specified in declare call)
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            SAF_Unit upascal;

            saf_find_one_unit(db, "pascal", &upascal);

            if(!l_quiet) TESTING(do_multifile?
                                 do_writes?
                                 "OVERwriting fields [buf in write] [multifile]"
                                 :
                                 "OVERwriting fields [buf in write]"
                                 :
                                 do_writes?
                                 "OVERwriting fields [buf in write]"
                                 :
                                 "OVERwritieg fields [buf in write]");

            /* made up stresses */
            {
                float buf[] = {3.5, 3.25, 3.5,3.5, 3.25, 3.5,
                               3.5, 3.25, 3.5,3.5, 3.25, 3.5};
                void *pbuf = &buf[0];

                saf_declare_field(SAF_ALL, db, &stress_ctmpl, "OvwPx", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[0]));
                saf_declare_field(SAF_ALL, db, &stress_ctmpl, "OvwPy", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[1]));
                saf_declare_field(SAF_ALL, db, &stress_ctmpl, "OvwPxy", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(stress_compons[2]));
                saf_declare_field(SAF_ALL, db, &stress_ftmpl, "OvwP", &cell_1, &upascal, SAF_SELF(db), SAF_ZONAL(&elems),
                                  H5T_NATIVE_FLOAT, stress_compons, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &ovw_stress);
                if (do_writes)
                    saf_write_field(SAF_ALL, &ovw_stress, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
            }

            /* made new stresses to overwrite old ones */
            {
                float buf[] = {9.5, 9.25, 9.5,9.5, 9.25, 9.5,
                               9.5, 9.25, 9.5,9.5, 9.25, 9.5};
                void *pbuf = &buf[0];

                if (do_writes)
                    saf_write_field(SAF_ALL, &ovw_stress, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, &pbuf, seqfiles[0]);
            }
            if(!l_quiet) PASSED;
        }

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                         OVER-WRITE FIELDS OBTAINED FROM FIND CALL
         *                                         (buf specified in declare call)
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            int n=0;
            SAF_Field *fields=NULL;

            if(!l_quiet) TESTING(do_multifile?
                                 do_writes?
                                 "OVERwriting fields obtained from saf_find_field [buf in write] [multifile]"
                                 :
                                 "OVERwriting fields obtained from saf_find_field [buf in write]"
                                 :
                                 do_writes?
                                 "OVERwriting fields obtained from saf_find_field [buf in write]"
                                 :
                                 "OVERwritieg fields obtained from saf_find_field [buf in write]");



            /* find the fields on cell_2 */
            saf_find_fields(SAF_ALL, db, &cell_2, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT,
                            SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &n, &fields);
#if 1 /*rpm 2004-06-28*/
            fprintf(stderr, "n=%d\n", n);
#endif

            /* made up temperatures */
            {
                float buf[] = {750., 950., 1200., 800., 1150., 850., 1100.};
                void *pbuf = &buf[0];

                if (do_writes)
                    saf_write_field(SAF_ALL, fields+0, SAF_WHOLE_FIELD, 1, H5T_NATIVE_FLOAT, &pbuf, seqfiles[0]);
            }
            if(!l_quiet) PASSED;
        }

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                   STATES AND SUITES      
         *--------------------------------------------------------------------------------------------------------------------- */
        {
            int index[1];
            float time[1];

            SAF_StateTmpl st_tmpl;
            SAF_FieldTmpl fld_tmpls[4];
            SAF_StateGrp state_grp;
            SAF_Field field_list[4]; 
            SAF_Unit l_units_seconds;

            if(!l_quiet) TESTING("states and suites");


            /* create the suite */
            saf_declare_suite(SAF_ALL, db, "CPBS", &top, NULL, &cpbs);

            fld_tmpls[0] = coords_ftmpl;
            fld_tmpls[1] = stress_ftmpl;
            fld_tmpls[2] = temp2_ftmpl;
            fld_tmpls[3] = press_ftmpl;

            saf_declare_state_tmpl (SAF_ALL, db, "STATE_TMPL", 4, fld_tmpls, &st_tmpl);

            saf_find_one_unit(db, "second", &l_units_seconds);

            saf_declare_state_group(SAF_ALL, db, "STATE_GROUP", &cpbs, &top, &st_tmpl, SAF_QTIME, &l_units_seconds, 
                                    SAF_FLOAT, &state_grp);


            /* insert the following fields into the states for time steps 0, 1, and 2:
             * displacement vector on nodes of whole
             * stress tensor on elements of cell_1
             * temperature on nodes of cell_2
             * pressure on elements of side_set_1 */

            index[0] = 0;

            time[0] = 0.0;

            field_list[0] = disps;
            field_list[1] = stress;
            field_list[2] = temps2;
            field_list[3] = pressure;

            saf_write_state(SAF_ALL, &state_grp, index[0], &top, SAF_FLOAT, time, field_list);

            /* for this test, just write out the same fields (IDs) to the successive states;
             * in actual simulations, new fields will be created for each time step */

            index[0] = 1;

            time[0] = 0.001;

            field_list[0] = disps;
            field_list[1] = stress;
            field_list[2] = temps2;
            field_list[3] = pressure;

            saf_write_state(SAF_ALL, &state_grp, index[0], &top, SAF_FLOAT, time, field_list);

            index[0] = 2;

            time[0] = 0.002;

            if(l_perturb==4) {
                /*perturb 4 is a 5% changed time value*/
                time[0] *= 1.05;
            }

            field_list[0] = disps;
            field_list[1] = stress;
            field_list[2] = temps2;
            field_list[3] = pressure;

            saf_write_state(SAF_ALL, &state_grp, index[0], &top, SAF_FLOAT, time, field_list);


            if(!l_quiet) PASSED;
            passCount += 1;

        }

        if(!l_quiet) TESTING("database close");
        if (!skip_termination) saf_close_database(db);
        if (do_multifile) {
            saf_close_database(topofile);
            for (i = 0; i < 5; i++) {
                saf_close_database(seqfiles[i]);
	    }
        }
        if(!l_quiet) {
            if (!skip_termination)
                PASSED;
            else
                SKIPPED;
        }
        passCount += 1;

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            failCount += 1;
	}
    } SAF_TRY_END;

    if (!skip_termination) {
        saf_final();
#ifdef HAVE_PARALLEL
        /* make sure everyone returns the same error status */
        MPI_Bcast(&failCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Finalize();
#endif
    }

    return (failCount==0)? 0 : 1;

}


/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Larry's Use Case Reader
 *
 * Description: This is testing code that exercises larry's first use case. This tests finds and describes of SAF objects 
 *		and, optionally, *reads* the raw data. There are two command line arguments; DO_DESCIBES and DO_READS. 
 *		The program will *always* test the finds. The describes and reads are optional.
 *
 *		Both tests larry1w.c and larry1r.c are interdependent in that larry1w.c writes data that larry1r.c further
 *		reads and confirms is what is expected.
 *
 *		This test begins by finding various objects in the file. These objects are known to be in the file
 *		by virtue of the interdependence on larry1w.c	
 *
 *		If DO_DESCRIBES is present, this test performs saf_describe_xxx() calls.
 *
 *		If DO_READS is present, this test also performs saf_read_xxx() calls and compares data.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Mark Miller, LLNL, Thu Mar 15, 2000 
 *
 *--------------------------------------------------------------------------------------------------------------------------------
 */
static int
SAF_IndexSpecEqual(SAF_IndexSpec a, SAF_IndexSpec b){
  int i;
  
  if(a.ndims!=b.ndims) 
    return 0;
  for(i=0; i<SAF_MAX_NDIMS; i++){
    if(a.sizes[i]   != b.sizes[i] ||
       a.origins[i] != b.origins[i] ||
       a.order[i]   != b.order[i]){
      return 0;
    }
  }
  
  return 1;
}

/**/
int
larry1r_main(int argc, char **argv, int skip_initialization)
{
    char *dbname=0;
    char *sfdirname=0;
    char *name;
    int command_line_result=0;
    hbool_t do_describes = false;
    hbool_t do_reads = false;
    hbool_t multifile=false;
    int i, rank=0, nerrors=0;
    int tdim, topo_dim, count, num_cats, num_colls, num_rels, num_sets;
    int num_ftmpls;
    SAF_Role role;
    SAF_SilRole srole;
    SAF_DbProps *dbprops=NULL;
    hbool_t failed;
    SAF_ExtendMode extendible;
    SAF_CellType cell_type;
    SAF_DecompMode is_decomp;
    SAF_IndexSpec ispec;
    SAF_Cat(cats, p_cats, 3);
    SAF_Set top;
    SAF_Set(sets, p_sets, 1);
    SAF_Rel(rels, p_rels, 1);
    SAF_FieldTmpl(ftmpls, p_ftmpls, 1);
    SAF_FieldTmpl coords_ftmpl, coords_ctmpl, distfac_ftmpl;
    SAF_Field(fields, p_fields, 10);
    SAF_Field distfac, temps2, time_coords;
    SAF_Cat nodes, elems;
    SAF_Set cell_1, cell_2, cell_2_tri, cell_2_quad, cell_3, time;
    SAF_Rel rel1, rel2, trel1, trel2_tri, trel2_quad, trel3;

    int failCount=0;
    SAF_AltIndexSpec explicit_aspec, aspecwrite, index_cell_1_nodes;
    SAF_AltIndexSpec *aspecs=NULL, imp_aspec;
    SAF_IndexSpec indxspec;

    /* DONT PUT ANY CODE HERE SO WE CAN CONTINUE WITH VARIABLE DECLARATIONS UNIQUE TO PARALLEL */

#ifdef HAVE_PARALLEL
    if (!skip_initialization) {
        MPI_Init(&argc,&argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    }
#endif

    /* since we want to see whats happening make sure stdout and stderr are unbuffered */
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    /* process command line args */
    command_line_result = STU_ProcessCommandLine(1, argc, argv,
                                                 "do_reads",
                                                 "if present, test the saf_read_xxx() calls",
                                                 &do_reads,
                                                 "do_describes",
                                                 "if present, test saf_describe_xxx() calls",
                                                 &do_describes,
                                                 "-sfdir %s",
                                                 "-sfdir %%s    : specify a supplemental file directory",
                                                 &sfdirname,
                                                 "-db %s",
                                                 "-db %%s    : specify a database name",
                                                 &dbname,
                                                 STU_END_OF_ARGS);

    if( command_line_result < 0 ) {
        goto theExit;
    }
   
    if( !dbname ) {
        /*did not get dbname in command line reader: allocate here*/
        dbname = (char *)malloc(1024*sizeof(char));
        dbname[0]='\0';
    }
    if( !sfdirname ) {
        /*did not get sfdirname in command line reader: allocate here*/
        sfdirname = (char *)malloc(1024*sizeof(char));
        sfdirname[0]='\0';
    }

    /* for convenience, set working directory to the test file directory unless file was specified on command line */
    if (!strlen(dbname))
        chdir(TEST_FILE_PATH);
#ifdef HAVE_PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    if (!skip_initialization)
        saf_init(SAF_DEFAULT_LIBPROPS);

    if (!strlen(dbname))
        strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {

        /* note: because we are in a try block here, all failures will send us to the one and only
       catch block at the end of this test */

        if (!skip_initialization) {
            dbprops = saf_createProps_database();
            saf_setProps_ReadOnly(dbprops);
#ifdef SSLIB_SUPPORT_PENDING
            if (strlen(sfdirname))
                saf_setProps_SFileDir(dbprops, sfdirname);
#endif /*SSLIB_SUPPORT_PENDING*/
            db = saf_open_database(dbname,dbprops);
        }

        /* determine if the database was generated in a multifile mode */
#ifdef SSLIB_SUPPORT_PENDING
        saf_find_files(db, SAF_ANY_NAME, &i, NULL);
        if (i > 0)
            multifile = true;
        else
            multifile = false;
#endif /*SSLIB_SUPPORT_PENDING*/



        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                FIND CATEGORIES
         *---------------------------------------------------------------------------------------------------------------------*/

        /* find number of topological categories */
        TESTING("finding categories [name=any,role=topology,tdim=any]");
        saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_TOPOLOGY, SAF_ANY_TOPODIM, &num_cats, NULL);
        if (num_cats != 3) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        /* find the topological category whose name is "nodes" */
        TESTING("finding categories [name=\"nodes\",{role,tdim}=any]");
        num_cats = 2;
        saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "nodes", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_cats, &p_cats);
        if (num_cats != 1) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
        nodes = cats[0];

        /* find the topological category whose name is "elems" */
        TESTING("finding categories [name=\"elems\",{role,tdim}=any]");
        num_cats = 2;
        saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "elems", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_cats, &p_cats);
        if (num_cats != 1) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
        elems = cats[0];

        /* test find all topological categories */
        TESTING("finding categories [{name,tdim}=any,role=topology]");
        num_cats = 3;
        saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_TOPOLOGY, SAF_ANY_TOPODIM, &num_cats, &p_cats);
        /* if ((num_cats != 2) or
         * (not (SAF_EQUIV(cats[0], nodes) or SAF_EQUIV(cats[1], nodes))) or
         * (not (SAF_EQUIV(cats[0], elems) or SAF_EQUIV(cats[1], elems)))) */
        if( num_cats != 3 ) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        /*--------------------------------------------------------------------------------------------------------------------- 
         *                                                DESCRIBE CATEGORIES
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_describes) {
            failed = false;
            TESTING("describe categories");

            /* describe collection categories */
            name = NULL;
            saf_describe_category(SAF_ALL, &nodes,&name,&role,&tdim);
            if (strcmp(name,"nodes") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=0)
                failed = true;
            free(name);
            name = NULL;
            saf_describe_category(SAF_ALL, &elems,&name,&role,&tdim);
            if (strcmp(name,"elems") || !SAF_EQUIV(&role, SAF_TOPOLOGY) || tdim!=2)
                failed = true;
            free(name);

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                FIND MATCHING SETS 
         *---------------------------------------------------------------------------------------------------------------------*/
        TESTING("finding matching sets [tdim=1]");
        saf_find_matching_sets(SAF_ALL, db, SAF_ANY_NAME, SAF_ANY_SILROLE, 1, SAF_EXTENDIBLE_TORF, SAF_TOP_TORF, &num_sets, NULL); 
        if (num_sets != 5) {
            /* 3 declared sets plus 2 suite sets (CPBS and suite_param_set) */
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        TESTING("finding matching sets [name_grep=\"CELL_2\"]");
        saf_find_matching_sets(SAF_ALL, db, "CELL_2", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, NULL); 
        if (num_sets != 1) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    
#if defined WIN32 || defined janus /*WIN32 does not have regcomp,regcmp,regex (@)*/
        TESTING("finding matching sets [name_grep=\"@^CELL_2$\"]");
	SKIPPED;
        TESTING("finding matching sets [name_grep=\"@CELL\"]");
	SKIPPED;
        TESTING("finding matching sets [name_grep=\"@^CELL_[123]$\"]");
	SKIPPED;
#else
        TESTING("finding matching sets [name_grep=\"@^CELL_2$\"]");
        saf_find_matching_sets(SAF_ALL, db, "@^CELL_2$", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, NULL); 
        if (num_sets != 1) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        TESTING("finding matching sets [name_grep=\"@CELL\"]");
        saf_find_matching_sets(SAF_ALL, db, "@CELL", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, NULL); 
        if (num_sets != 6) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        TESTING("finding matching sets [name_grep=\"@^CELL_[123]$\"]");
        saf_find_matching_sets(SAF_ALL, db, "@^CELL_[123]$", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, NULL); 
        if (num_sets != 3) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
#endif

        TESTING("finding matching sets [name_grep=\"TOP_CELL\"]");
        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "TOP_CELL", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets); 
        top = p_sets[0];

        if (num_sets != 1) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        /* find some other sets needed for tests below */
        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "CELL_1", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets); 
        cell_1 = p_sets[0];
        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "CELL_2", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets);
        cell_2 = p_sets[0];
        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "CELL_2_TRIS", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets);
        cell_2_tri = p_sets[0];
        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "CELL_2_QUADS", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets);
        cell_2_quad = p_sets[0];
        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "CELL_3", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets); 
        cell_3 = p_sets[0];
        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "TIME", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets); 
        time = p_sets[0];

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                FIND SETS (SAF_EACH mode)
         *---------------------------------------------------------------------------------------------------------------------*/
        TESTING("saf_find_sets in SAF_EACH mode");
        {
            char tmpName[1024];
            int i, j, numSets = 0;
            SAF_Set myProcSet, *theSets=NULL;
            int n;

            failed = false;
            /* first, find the proc sets using a matching find */
            sprintf(tmpName,"proc %02d", rank);
            saf_find_matching_sets(SAF_EACH, db, tmpName, SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF, SAF_TOP_TORF,
                                   &n, &theSets);
            myProcSet=theSets[0];
            free(theSets);

            n=0;
            theSets=NULL;
            saf_find_sets(SAF_EACH, SAF_FSETS_SUBS, &myProcSet, &nodes, &numSets, &theSets);
            if (numSets != 2) {
                failed = true;
            } else {
                /* SSlib makes no guarantee about the order that things are returned unless they were declared with SAF_ALL
                 * mode. The reason is that anything declared in SAF_EACH mode is local to a task until a table
                 * synchronization occurs. During the synchronization permanent homes are found in the table for the objects
                 * and depending on certain circumstances (available space, whether two or more tasks declared similar objects,
                 * whether certain errors were encountered, etc) those new homes could be in a different order than the
                 * objects were stored locally. */
                char **tmpName = malloc(numSets*sizeof(*tmpName));
                hbool_t *seenName = calloc((size_t)numSets, sizeof(*seenName));
                for (i=0; i<numSets; i++) {
                    tmpName[i] = malloc(20);
                    sprintf(tmpName[i], "proc %02d - sub%1d", rank, i+1);
                }
                for (i=0; i<numSets; i++) {
                    char *theName=NULL;
                    saf_describe_set(SAF_EACH, theSets+i, &theName, NULL, NULL, NULL, NULL, NULL, NULL);
                    for (j=0; j<numSets; j++) {
                        if (!strcmp(theName, tmpName[j])) {
                            if (seenName[j])
                                failed = TRUE;
                            seenName[j] = TRUE;
                            break;
                        }
                    }
                    if (j>numSets)
                        failed = TRUE;
                    free(theName);
                }
                for (i=0; i<numSets; i++)
                    free(tmpName[i]);
                free(tmpName);
                free(seenName);
            }
            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                DESCRIBE SETS 
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_describes) {
            failed = false;
            TESTING("describing sets");
            name = NULL;
            saf_describe_set(SAF_ALL, &top, &name, NULL, &srole, &extendible, NULL, NULL, NULL);
            if ((strcmp(name, "TOP_CELL") != 0) || (srole != SAF_SPACE) || (extendible != SAF_EXTENDIBLE_FALSE))
                failed = true;
            free(name);
            saf_describe_set(SAF_ALL, &cell_3, NULL, &topo_dim, NULL, NULL, NULL, NULL, NULL);
            if (topo_dim != 2)
                failed = true;

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                FIND COLLECTIONS
         *---------------------------------------------------------------------------------------------------------------------*/
        failed = false;
        TESTING("finding collections");


        num_colls = 2;
        saf_find_collections(SAF_ALL, &top, SAF_ANY_ROLE, SAF_CELLTYPE_POINT, SAF_ANY_TOPODIM,
                             SAF_DECOMP_TORF, &num_colls, &p_cats);
        if ((num_colls != 2) || (!SAF_EQUIV(cats+0, &nodes)))
            failed = true;


        num_colls = 2;
        saf_find_collections(SAF_ALL, &top, SAF_TOPOLOGY, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM,
                             SAF_DECOMP_TORF, &num_colls, &p_cats);
        if (num_colls != 2)
            failed = true;
        if (!(SAF_EQUIV(cats+0, &nodes) || SAF_EQUIV(cats+1, &nodes)))
            failed = true;
        if (!(SAF_EQUIV(cats+0, &elems) || SAF_EQUIV(cats+1, &elems)))
            failed = true;

        num_colls = 2;
        saf_find_collections(SAF_ALL, &top, SAF_ANY_ROLE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM,
                             SAF_DECOMP_TRUE, &num_colls, &p_cats);
        if (num_colls != 2)
            failed = true;
        if (!(SAF_EQUIV(cats+0, &elems) || SAF_EQUIV(cats+1, &elems)))
            failed = true;


        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                DESCRIBE COLLECTIONS
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_describes) {
            failed = false;
            TESTING("describing collections");

            saf_describe_collection(SAF_ALL, &top, &elems, &cell_type, &count, NULL, &is_decomp, NULL);
            if ((cell_type != SAF_CELLTYPE_MIXED) || (count != 12) || (is_decomp != SAF_DECOMP_TRUE))
                failed = true;
            saf_describe_collection(SAF_ALL, &cell_1, &elems, NULL, &count, NULL, NULL, NULL);
            if (count != 4)
                failed = true;
            saf_describe_collection(SAF_ALL, &cell_2, &nodes, &cell_type, NULL, &ispec, NULL, NULL);
            if ((cell_type != SAF_CELLTYPE_POINT) || (ispec.ndims != 1) || (ispec.sizes[0] != 7))
                failed = true;

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }


        /*---------------------------------------------------------------------------------------------------------------------
         *                                                FIND ALTERNATE INDEX SPECIFICATIONS
         *---------------------------------------------------------------------------------------------------------------------*/
        TESTING("finding alternate index specifications");
        {
            char *name=NULL;
            int failAlt=0, n;
            saf_find_alternate_indexspecs(SAF_ALL, &top, &elems, SAF_ANY_NAME, &n, &aspecs);
            for(i=0; i<n; i++){
                saf_describe_alternate_indexspec(SAF_ALL, aspecs+i, NULL,NULL,&name,NULL,NULL,NULL,NULL,NULL);

                if(!strcmp(name, "first") && !strcmp(name, "second") && !strcmp(name, "third") && !strcmp(name, "fourth") &&
                   !strcmp(name, "fifth implicit")){
                    failCount++;
                    failAlt++;
                }
                if(!strcmp(name, "fifth implicit")){
                    imp_aspec = aspecs[i];
                }
                if(!strcmp(name, "fourth")){
                    explicit_aspec = aspecs[i];
                }

                free(name); name=NULL;
            }

            free(aspecs); aspecs=NULL;
            saf_find_alternate_indexspecs(SAF_ALL, &top, &elems, "second", &n, &aspecs);
            if(n!=1) failAlt++;

            saf_describe_alternate_indexspec(SAF_ALL, aspecs+0, NULL,NULL,&name,NULL,NULL,NULL,NULL,NULL);
            if(strcmp(name, "second")) failAlt++;
            aspecwrite = aspecs[0];

            free(name); name=NULL;

            free(aspecs); aspecs=NULL;

#if defined WIN32 || defined JANUS /*WIN32 does not have regcomp,regcmp,regex (@)*/
#else
            saf_find_alternate_indexspecs(SAF_ALL, &top, &elems, "@third", &n, &aspecs);
            if(n!=1) failAlt++;

            saf_describe_alternate_indexspec(SAF_ALL, aspecs+0, NULL,NULL,&name,NULL,NULL,NULL,NULL,NULL);
            if(strcmp(name, "third")) failAlt++;

            free(name); name=NULL;
            free(aspecs); aspecs=NULL;
#endif

            /*finding the alt index spec for cell_1 and nodes*/
            saf_find_alternate_indexspecs(SAF_ALL, &cell_1, &nodes, SAF_ANY_NAME, &n, &aspecs);
            if(n!=1) failAlt++;

            saf_describe_alternate_indexspec(SAF_ALL, aspecs+0, NULL,NULL,&name,NULL,NULL,NULL,NULL,NULL);
            if(strcmp(name, "cell_1 nodes")) failAlt++;
            index_cell_1_nodes = aspecs[0];

            free(name); name=NULL;

            free(aspecs); aspecs=NULL;

            if(failAlt){
                FAILED;
            }else{
                PASSED;
            }
      
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                DESCRIBE ALTERNATE INDEX SPECIFICATIONS
         *---------------------------------------------------------------------------------------------------------------------*/
        if(do_describes){
            TESTING("describing alternate index specifications");
            {
                char *name=NULL;
                SAF_Set containing_set;
                SAF_Cat altcat;
                hid_t data_type;
                hbool_t is_explicit, is_compact, is_sorted;
                SAF_IndexSpec implicit_ispec;
                int failAlt=0;

                indxspec = SAF_1DF(12);

                /*describe an implicit alt index spec*/
                saf_describe_alternate_indexspec(SAF_ALL, &imp_aspec, &containing_set, &altcat, &name, &data_type,
                                                 &is_explicit, &implicit_ispec,&is_compact, &is_sorted);


                if(SAF_IndexSpecEqual(implicit_ispec, indxspec)
                   && SAF_EQUIV(&containing_set, &top) && SAF_EQUIV(&altcat, &elems) 
                   && !strcmp(name, "fifth implicit") && 
                   data_type<=0 /*note the data_type is invalid because no data was written out for this alt index spec*/
                   && is_explicit == false && implicit_ispec.ndims == indxspec.ndims ){
                    /*PASSED*/
                }else{
                    failCount++;
                    failAlt++;
                }


                free(name); name=NULL;
                /*describe an explicit alt index spec*/
                saf_describe_alternate_indexspec(SAF_ALL, &explicit_aspec, &containing_set, &altcat, &name, &data_type,
                                                 &is_explicit, &implicit_ispec,&is_compact, &is_sorted);


                if(SAF_IndexSpecEqual(implicit_ispec, SAF_NA_INDEXSPEC) && 
                   SAF_EQUIV(&containing_set, &top) && SAF_EQUIV(&altcat, &elems) && !strcmp(name, "fourth") && 
                   data_type<=0 /*note the data type is invalid since no data was written*/
                   && is_explicit == true &&
                   is_compact == false && is_sorted == true){
                    /*PASSED*/
                }else{
                    failCount++;
                    failAlt++;
                }

                free(name); name=NULL;
                /*describe an explicit alt index spec*/
                saf_describe_alternate_indexspec(SAF_ALL, &index_cell_1_nodes, &containing_set, &altcat, &name, &data_type,
                                                 &is_explicit, &implicit_ispec,&is_compact, &is_sorted);

                if(SAF_IndexSpecEqual(implicit_ispec, SAF_NA_INDEXSPEC) && 
                   SAF_EQUIV(&containing_set, &cell_1) && SAF_EQUIV(&altcat, &nodes) && !strcmp(name, "cell_1 nodes") && 
                   H5Tequal(data_type, H5T_NATIVE_INT) && is_explicit == true && 
                   is_compact == false && is_sorted == false){
                    /*PASSED*/
                }else{
                    failCount++;
                    failAlt++;
                }


                if(failAlt){
                    FAILED;
                }else{
                    PASSED;
                }

            }
        }/*if(do_describes)*/


        /*---------------------------------------------------------------------------------------------------------------------
         *                                                READ ALTERNATE INDEX SPECIFICATIONS
         *---------------------------------------------------------------------------------------------------------------------*/
        if(do_reads){
            TESTING("reading    alternate index specifications");
            {
                /*the collection count is 12*/
                int buf[] = {2,3,5,7,11,13,17,19,23,29,31,37};
                int buf2[] = {8,6,4,2,0,7,5,3,1};
                int *readbuf=NULL;

                saf_read_alternate_indexspec(SAF_ALL, &aspecwrite, (void **)&readbuf);
                for(i=0; i<12; i++){
                    if(buf[i]!=readbuf[i]){
                        FAILED;
                        break;
                    }
                }

                free(readbuf); readbuf=NULL;
                saf_read_alternate_indexspec(SAF_ALL, &index_cell_1_nodes, (void **)&readbuf);
                for(i=0; i<9; i++){
                    if(buf2[i]!=readbuf[i]){
                        FAILED;
                        break;
                    }
                }

                PASSED;
            }
        }/*if(do_reads)*/





        /*---------------------------------------------------------------------------------------------------------------------
         *                                                FIND SUBSET RELATIONS 
         *---------------------------------------------------------------------------------------------------------------------*/
        failed = false;
        TESTING("finding subset relations");

        saf_find_subset_relations(SAF_ALL, db, &top, &cell_1, SAF_COMMON(SAF_ANY_CAT), &num_rels, NULL);
        if (num_rels != 3)
            failed = true;

        num_rels = 1;
        saf_find_subset_relations(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&nodes), &num_rels, &p_rels);
        if (num_rels != 1)
            failed = true;
        rel1 = rels[0];

        num_rels = 1;
        saf_find_subset_relations(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&elems), &num_rels, &p_rels);
        if (num_rels != 1)
            failed = true;
        rel2 = rels[0];

        num_rels = 1;
        saf_find_subset_relations(SAF_ALL, db, &cell_2, &cell_1, SAF_COMMON(SAF_ANY_CAT), &num_rels, &p_rels);
        if (num_rels != 0)
            failed = true;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                DESCRIBE SUBSET RELATIONS 
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_describes) {
            SAF_Set sup, sub;
            SAF_Cat sup_cat, sub_cat;
            SAF_BoundMode sbmode, cbmode;

            failed = false;
            TESTING("describing subset relations");

            saf_describe_subset_relation(SAF_ALL, &rel1, &sup, &sub, &sup_cat, &sub_cat,
                                         &sbmode, &cbmode, NULL, NULL);
            if (!SAF_EQUIV(&sup, &top) || !SAF_EQUIV(&sub, &cell_1) || !SAF_EQUIV(&sup_cat, &nodes) ||
                !SAF_EQUIV(&sub_cat, &nodes) || (sbmode != SAF_BOUNDARY_FALSE) || (cbmode != SAF_BOUNDARY_FALSE))
                failed = true;

            saf_describe_subset_relation(SAF_ALL, &rel2, &sup, &sub, &sup_cat, &sub_cat,
                                         &sbmode, &cbmode, NULL, NULL);
            if (!SAF_EQUIV(&sup, &top) || !SAF_EQUIV(&sub, &cell_1) || !SAF_EQUIV(&sup_cat, &elems) ||
                !SAF_EQUIV(&sub_cat, &elems) || (sbmode != SAF_BOUNDARY_FALSE) || (cbmode != SAF_BOUNDARY_FALSE))
                failed = true;

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                READ SUBSET RELATIONS 
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_reads) {
            failed = false;
            TESTING(multifile?"reading subset relations [multifile]":"reading subset relations");

            /* try to read subset relation 1 */
            {
                int buf[] = {1,2,3,5,6,7,9,10,11};
                int *rbuf = NULL;
                saf_read_subset_relation(SAF_ALL, &rel1, NULL, (void**) &rbuf, NULL);
                for (i = 0; i < (int) (sizeof(buf) / sizeof(int)); i++)
                    if (buf[i] != rbuf[i])
                        failed = true;
                free(rbuf);
            }

            /* try to read subset relation 2 */
            {
                int buf[] = {1,2,4,5};
                int *rbuf = NULL;
                saf_read_subset_relation(SAF_ALL, &rel2, NULL, (void**) &rbuf, NULL);
                for (i = 0; i < (int) (sizeof(buf) / sizeof(int)); i++)
                    if (buf[i] != rbuf[i])
                        failed = true;
                free(rbuf);
            }

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }

        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                FIND TOPOLOGY RELATIONS 
         *---------------------------------------------------------------------------------------------------------------------*/
        {
            int num_topo_rels;
            SAF_Rel *topo_rels;

            TESTING("find topology relations");

            topo_rels = NULL;
            failed = false;
            num_topo_rels = 0;
            saf_find_topo_relations(SAF_ALL, db, &cell_1, NULL, &num_topo_rels, &topo_rels);
            if (num_topo_rels != 1) {
                FAILED;
                failed = true;
                fprintf(stderr, "...expected 1 topological relation on \"cell_1\"\n");
                fprintf(stderr, "   found %d relations\n", num_topo_rels);
            }
            trel1 = topo_rels[0];

            topo_rels = NULL;
            num_topo_rels = 0;
            saf_find_topo_relations(SAF_ALL, db, &cell_2_tri, NULL, &num_topo_rels, &topo_rels);
            if (num_topo_rels != 1) {
                if (!failed) {
                    FAILED;
                    failed = true;
                }
                fprintf(stderr, "...expected 1 topological relation on \"cell_2_tri\"\n");
                fprintf(stderr, "   found %d relations\n", num_topo_rels);
            }
            trel2_tri = topo_rels[0];

            topo_rels = NULL;
            num_topo_rels = 0;
            saf_find_topo_relations(SAF_ALL, db, &cell_2_quad, NULL, &num_topo_rels, &topo_rels);
            if (num_topo_rels != 1) {
                if (!failed) {
                    FAILED;
                    failed = true;
                }
                fprintf(stderr, "...expected 1 topological relation on \"cell_2_quad\"\n");
                fprintf(stderr, "   found %d relations\n", num_topo_rels);
            }
            trel2_quad = topo_rels[0];

            topo_rels = NULL;
            num_topo_rels = 0;
            saf_find_topo_relations(SAF_ALL, db, &cell_3, NULL, &num_topo_rels, &topo_rels);
            if (num_topo_rels != 1) {
                if (!failed) {
                    FAILED;
                    failed = true;
                }
                fprintf(stderr, "...expected 1 topological relation on \"cell_3\"\n");
                fprintf(stderr, "   found %d relations\n", num_topo_rels);
            }
            trel3 = topo_rels[0];

            if (!failed)
                PASSED;
            else
                nerrors++;
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                DESCRIBE TOPOLGY RELATIONS
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_describes) {
            SAF_Set containing_set, range_s;
            SAF_Cat the_pieces, range_c, storage_decomp;
            SAF_RelRep trtype;
            hid_t data_type;

            TESTING("describe topology relations");

            failed = false;
            /* get description of topology relation 1 */
            saf_describe_topo_relation(SAF_ALL, &trel1, &containing_set, &the_pieces, &range_s, &range_c,
                                       &storage_decomp, &trtype, &data_type);
            if (!SAF_EQUIV(&containing_set, &cell_1) || !SAF_EQUIV(&the_pieces, &elems) || !SAF_EQUIV(&range_s, &top) ||
                !SAF_EQUIV(&range_c, &nodes) || !_saf_is_self_decomp(&storage_decomp) || !SAF_EQUIV(&trtype, SAF_UNSTRUCTURED) ||
                !H5Tequal(data_type, H5T_NATIVE_INT)) {
                FAILED;
                failed = true;
            }

            /* get description of topology relation 2 on triangles */
            saf_describe_topo_relation(SAF_ALL, &trel2_tri, &containing_set, &the_pieces, &range_s, &range_c,
                                       &storage_decomp, &trtype, &data_type);
            if (!SAF_EQUIV(&containing_set, &cell_2_tri) || !SAF_EQUIV(&the_pieces, &elems) || !SAF_EQUIV(&range_s, &top) ||
                !SAF_EQUIV(&range_c, &nodes) || !_saf_is_self_decomp(&storage_decomp) || !SAF_EQUIV(&trtype, SAF_UNSTRUCTURED) ||
                !H5Tequal(data_type, H5T_NATIVE_INT)) {
                if (!failed) {
                    FAILED;
                    failed = true;
                }
            }

            /* get description of topology relation 2 on quads */
            saf_describe_topo_relation(SAF_ALL, &trel2_quad, &containing_set, &the_pieces, &range_s, &range_c,
                                       &storage_decomp, &trtype, &data_type);
            if (!SAF_EQUIV(&containing_set, &cell_2_quad) || !SAF_EQUIV(&the_pieces, &elems) || !SAF_EQUIV(&range_s, &top) ||
                !SAF_EQUIV(&range_c, &nodes) || !_saf_is_self_decomp(&storage_decomp) || !SAF_EQUIV(&trtype, SAF_UNSTRUCTURED) ||
                !H5Tequal(data_type, H5T_NATIVE_INT)) {
                if (!failed) {
                    FAILED;
                    failed = true;
                }
            }

            /* get description of topology relation 3 */
            saf_describe_topo_relation(SAF_ALL, &trel3, &containing_set, &the_pieces, &range_s, &range_c,
                                       &storage_decomp, &trtype, &data_type);
            if (!SAF_EQUIV(&containing_set, &cell_3) || !SAF_EQUIV(&the_pieces, &elems) || !SAF_EQUIV(&range_s, &top) ||
                !SAF_EQUIV(&range_c, &nodes) || !_saf_is_self_decomp(&storage_decomp) || !SAF_EQUIV(&trtype, SAF_UNSTRUCTURED) ||
                !H5Tequal(data_type, H5T_NATIVE_INT)) {
                if (!failed) {
                    FAILED;
                    failed = true;
                }
            }

            if (!failed)
                PASSED;
            else
                nerrors++;
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                READ TOPOLGY RELATIONS
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_reads) {
            int i, *rabuf, *rbbuf;

            TESTING("read topology relations");

            failed = false;
            /* try to read topological relation 1 */
            {
                int abuf[] = {4};
                int bbuf[] = {1,2,6,5,2,3,7,6,5,6,10,9,6,7,11,10};

                rabuf = NULL;
                rbbuf = NULL;
                saf_read_topo_relation(SAF_ALL, &trel1, NULL, (void**) &rabuf, (void**) &rbbuf);
                if (abuf[0] != rabuf[0]) {
                    FAILED;
                    failed = true;
                    fprintf(stderr, "abuf on topological relation 1 incorrect");
                    fprintf(stderr, "...expected 4 got %i\n", rabuf[0]);
                }
                for (i = 0; i < 16; i++) {
                    if (bbuf[i] != rbbuf[i]) {
                        if (!failed) {
                            FAILED;
                            failed = true;
                        }
                        fprintf(stderr, "bbuf[%i] on topological relation 1 incorrect", i);
                        fprintf(stderr, "...expected %i got %i\n", bbuf[i], rbbuf[i]);
                    }
                }
                free(rabuf);
                free(rbbuf);
            }

            /* try to read topological relation 2 tri */
            {
                int abuf[] = {3};
                int bbuf[] = {9,10,13,10,14,13,10,11,14};

                rabuf = NULL;
                rbbuf = NULL;
                saf_read_topo_relation(SAF_ALL, &trel2_tri, NULL, (void**) &rabuf, (void**) &rbbuf);
                if (abuf[0] != rabuf[0]) {
                    FAILED;
                    failed = true;
                    fprintf(stderr, "abuf on topological relation 2 tri incorrect");
                    fprintf(stderr, "...expected 3 got %i\n", rabuf[0]);
                }
                for (i = 0; i < 9; i++) {
                    if (bbuf[i] != rbbuf[i]) {
                        if (!failed) {
                            FAILED;
                            failed = true;
                        }
                        fprintf(stderr, "bbuf[%i] on topological relation 2 tri incorrect", i);
                        fprintf(stderr, "...expected %i got %i\n", bbuf[i], rbbuf[i]);
                    }
                }
                free(rabuf);
                free(rbbuf);
            }

            /* try to read topological relation 2 quad */
            {
                int abuf[] = {4};
                int bbuf[] = {13,14,17,16};

                rabuf = NULL;
                rbbuf = NULL;
                saf_read_topo_relation(SAF_ALL, &trel2_quad, NULL, (void**) &rabuf, (void**) &rbbuf);
                if (abuf[0] != rabuf[0]) {
                    FAILED;
                    failed = true;
                    fprintf(stderr, "abuf on topological relation 2 quad incorrect");
                    fprintf(stderr, "...expected 4 got %i\n", rabuf[0]);
                }
                for (i = 0; i < 4; i++) {
                    if (bbuf[i] != rbbuf[i]) {
                        if (!failed) {
                            FAILED;
                            failed = true;
                        }
                        fprintf(stderr, "bbuf[%i] on topological relation 2 quad incorrect", i);
                        fprintf(stderr, "...expected %i got %i\n", bbuf[i], rbbuf[i]);
                    }
                }
                free(rabuf);
                free(rbbuf);
            }

            /* try to read topological relation 3 */
            {
                int abuf[] = {4};
                int bbuf[] = {3,4,8,7,  7,8,12,11,  11,12,15,14,  14,15,18,17};

                rabuf = NULL;
                rbbuf = NULL;
                saf_read_topo_relation(SAF_ALL, &trel3, NULL, (void**) &rabuf, (void**) &rbbuf);
                if (abuf[0] != rabuf[0]) { 
                    FAILED;
                    failed = true;
                    fprintf(stderr, "abuf on topological relation 3 incorrect");
                    fprintf(stderr, "...expected 4 got %i\n", rabuf[0]);
                }
                for (i = 0; i < 16; i++) {
                    if (bbuf[i] != rbbuf[i]) {
                        if (!failed) {
                            FAILED;
                            failed = true;
                        }  
                        fprintf(stderr, "bbuf[%i] on topological relation 3 incorrect", i);
                        fprintf(stderr, "...expected %i got %i\n", bbuf[i], rbbuf[i]);
                    }
                }
                free(rabuf);
                free(rbbuf);
            }

            if (!failed)
                PASSED;
            else
                nerrors++;
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                                FIND FIELD TEMPLATES 
         *---------------------------------------------------------------------------------------------------------------------*/
        failed = false;
        TESTING("finding field templates");
        saf_find_field_tmpls(SAF_ALL, db, NULL, SAF_ALGTYPE_SCALAR, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, NULL);
        if (num_ftmpls != 8)  /* includes 2 field templates for the coordinates for the suites */
            failed = true;

        num_ftmpls=1;
        saf_find_field_tmpls(SAF_ALL, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, &p_ftmpls);
        coords_ctmpl = ftmpls[0];
        if (num_ftmpls != 1)
            failed = true;

        num_ftmpls = 1;
        saf_find_field_tmpls(SAF_ALL, db, "coordinate_tmpl", SAF_ALGTYPE_VECTOR, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, &p_ftmpls);
        coords_ftmpl = ftmpls[0];
        if (num_ftmpls != 1)
            failed = true;

        saf_find_field_tmpls(SAF_ALL, db, "temp_on_ns1_tmpl", SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, NULL);
        if (num_ftmpls != 1)
            failed = true;

        saf_find_field_tmpls(SAF_ALL, db, "scalar on cell_1", SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, NULL);
        if (num_ftmpls != 1)
            failed = true;

        saf_find_field_tmpls(SAF_ALL, db, NULL, SAF_ALGTYPE_SYMTENSOR, SAF_ANY_BASIS, SAF_ANY_QUANTITY,
                             &num_ftmpls, NULL);
        if (num_ftmpls != 1)
            failed = true;

        num_ftmpls = 1;
        saf_find_field_tmpls(SAF_ALL, db, "distrib_factors_tmpl", SAF_ALGTYPE_ANY,
                             SAF_ANY_BASIS, SAF_ANY_QUANTITY, &num_ftmpls, &p_ftmpls);
        distfac_ftmpl = ftmpls[0];
        if (num_ftmpls != 1)
            failed = true;

        if (failed) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }


        /*---------------------------------------------------------------------------------------------------------------------
         *                                               DESCIBE FIELD TEMPLATES 
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_describes) {
            /* SAF_Set base; */
            SAF_Algebraic atype;
            SAF_FieldTmpl *componentTmpls;
            int n;

            failed = false;
            TESTING("describe field templates");

            name = NULL;
            n = 0;
            componentTmpls = NULL;
            saf_describe_field_tmpl(SAF_ALL, &coords_ftmpl, &name, &atype, NULL, NULL, &n, &componentTmpls);
            if ((strcmp(name,"coordinate_tmpl")) || !SAF_EQUIV(&atype, SAF_ALGTYPE_VECTOR) || (n != 2) ||
                !SAF_EQUIV(componentTmpls+0,&coords_ctmpl) || !SAF_EQUIV(componentTmpls+1,&coords_ctmpl))
                failed = true;
            free(componentTmpls);
            free(name);

            name = NULL;
            saf_describe_field_tmpl(SAF_ALL, &distfac_ftmpl, &name, /* &base,*/ &atype, NULL, NULL, &n, NULL);
            if ((strcmp(name,"distrib_factors_tmpl")) || !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) || (n != 1))
                failed = true;
            free(name);

            saf_describe_field_tmpl(SAF_ALL, &coords_ctmpl, NULL, &atype, NULL, NULL, &n, NULL);
            if ( !SAF_EQUIV(&atype, SAF_ALGTYPE_SCALAR) || (n != 1))
                failed = true;


            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                               FIND FIELDS
         *---------------------------------------------------------------------------------------------------------------------*/
        {
            int n,num_coord_fields;
            SAF_Field *Pcoord_fields;

            failed = false;
            TESTING("find fields");

            /* find a field by name */
            n = 10;
            saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), "distribution factors", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
                            SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &n, &p_fields);
            distfac = p_fields[0];
            if (n != 1)
                failed = true;

            /* find all fields on cell_2 */
            n = 10;
            saf_find_fields(SAF_ALL, db, &cell_2, SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT,
                            SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &n, &p_fields);
            temps2 = p_fields[0];
            if (n != 1)
                failed = true;

            /* find the default coordinate fields for the time base */
            saf_find_default_coords(SAF_ALL, &time, &time_coords);

            /* find any coordinates on the top */
            num_coord_fields=4;
            Pcoord_fields=calloc(4, sizeof(*Pcoord_fields));
            saf_find_coords(SAF_ALL,db, &top,&num_coord_fields,&Pcoord_fields);
            if (num_coord_fields != 1)
                failed = true;

            /* find any coordinates in the database */
            num_coord_fields=4;
            saf_find_coords(SAF_ALL,db, SAF_UNIVERSE(db),&num_coord_fields,&Pcoord_fields);
            if (num_coord_fields != 4)
                failed = true;

            free(Pcoord_fields);

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }


        /*---------------------------------------------------------------------------------------------------------------------
         *                                               READ FIELDS
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_reads) {
            size_t i;

            failed = false;
            TESTING("read fields");

            if (do_reads) {
                size_t count;
                float old_buf[] = {4., 3., 2., 1., 0.};
                float new_buf[] = {0., 0., 0., 0., 0.};
                void *pbuf = &new_buf[0];

                saf_read_field(SAF_ALL, &distfac, NULL, SAF_WHOLE_FIELD, &pbuf);
                saf_get_count_and_type_for_field(SAF_ALL, &distfac, NULL, &count, NULL);
                for (i = 0; i < count; i++)
                    if (new_buf[i] != old_buf[i]) {
                        failed = true;
                        nerrors++;
                        break;
                    }
            }

            if (do_reads) {
                size_t count;
                float old_buf[] = {750., 950., 1200., 800., 1150., 850., 1100.};
                float new_buf[] = { 0.,  0.,   0.,  0.,   0.,  0.,   0.};
                void *pbuf = &new_buf[0];

                saf_read_field(SAF_ALL, &temps2, NULL, SAF_WHOLE_FIELD, &pbuf);
                saf_get_count_and_type_for_field(SAF_ALL, &temps2, NULL, &count, NULL);
                for (i = 0; i < count; i++)
                    if (new_buf[i] != old_buf[i]) {
                        failed = true;
                        nerrors++;
                        break;
                    }
            }

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                       DESCRIBE AND READ CONSTANT FIELDS
         *---------------------------------------------------------------------------------------------------------------------

         * This code also demonstrates a huge problem with the state of the API as of 1/3/02. There are at least three different
         * ways a client has to test for equality of a datatype depending on the class of type it is.
         *   SAF_EQUIV for objects like fields, relations, etc
         *   saf_equiv for objects like units, quantities, etc.
         *   `=='  for things like interleave modes
         *   DSL_same_type for numeric datatypes
         *
         * Update, as of 7/30/04:
         *   SAF_EQUIV() and saf_equiv() are interchangeable and are defined in terms of SS_PERS_EQ()
         *   `==' is still used for C enumeration constants
         *   H5Tequal() is used to compare HDF5 datatypes
         */
        {
            int i,n;
            SAF_Field cgField;
            SAF_Unit umeter;
            SAF_Eval *evalConstant = SAF_SPACE_CONSTANT;

            saf_find_one_unit(db, "meter", &umeter);

            failed = false;
            TESTING("describing and reading constant fields");

            /* find the constant field */
            n = 10;
            saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), SAF_ANY_NAME, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
                            SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_SPACE_CONSTANT, &n, &p_fields);
            if (n != 3)
                failed = true;

            /* describe the fields */
            for (i = 0; i < n; i++) {
                SAF_FieldTmpl ftmpl;
                SAF_Set base;
                char *name=NULL;
                SAF_Unit units;
                hbool_t is_coord;
                SAF_Cat storage_decomp;
                SAF_Cat coeff_assoc;
                int assoc_ratio;
                SAF_Cat eval_coll;
                SAF_Eval eval_func;
                hid_t data_type;
                int num_comps = 10;
                SAF_Field *comp_flds=NULL;
                SAF_Interleave intlv;

                saf_describe_field(SAF_ALL, fields+i, &ftmpl, &name, &base, &units, &is_coord, &storage_decomp, &coeff_assoc,
                                   &assoc_ratio, &eval_coll, &eval_func, &data_type, &num_comps, (SAF_Field**) &comp_flds, &intlv, NULL);

                if (!SAF_EQUIV(&units, &umeter))
                    {failed = true; printf("units\n");}
                if (is_coord)
                    {failed = true; printf("is_coord\n");}
                if (!SAF_EQUIV(&storage_decomp, SAF_SELF(XXX)))
                    {failed = true; printf("storage_decomp\n");}
                if (!SAF_EQUIV(&coeff_assoc, SAF_SELF(XXX)))
                    {failed = true; printf("coeff_assoc\n");}
                if (assoc_ratio != 1)
                    {failed = true; printf("assoc_ratio\n");}
                if (!SAF_EQUIV(&eval_coll, SAF_SELF(XXX)))
                    {failed = true; printf("eval_coll\n");}
                if (!SAF_EQUIV(&eval_func, evalConstant))
                    {failed = true; printf("eval_func\n");}

                if (!strcmp(name, "Xcg") || !strcmp(name, "Ycg")) {
                    if ((num_comps != 1) || (intlv != SAF_INTERLEAVE_NONE)) {
                        failed  = true;
                        break;
                    } 
                } else if (!strcmp(name, "CG")) {
                    cgField = fields[i];
                    /* if no data was written by writer, data_type will be invalid here */ 
                    if (((data_type>0) && !H5Tequal(data_type, H5T_NATIVE_FLOAT)) || (num_comps != 2) || (intlv != SAF_INTERLEAVE_VECTOR)) {
                        failed  = true;
                        break;
                    } 
                } else {
                    failed  = true;
                    break;
                }
                free(name);
                free(comp_flds);
            }

            /* ok, now read the field */
            if (do_reads) {
                float *pbuf = NULL;
                saf_read_field(SAF_ALL, &cgField, NULL, SAF_WHOLE_FIELD, (void**) &pbuf);
                if ((pbuf[0] != 2.0) || (pbuf[1] != 2.0))
                    failed = true;
                free(pbuf);
            }

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }





        /*---------------------------------------------------------------------------------------------------------------------
         *                                               PARTIAL READ FIELDS
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_reads) {
            int i;

            failed = false;
            TESTING("partial read fields");

            if (do_reads) {
                /* Here's how the data was in larry1w...
                
                 * note: |=overwritten, *=never written
                 * float old_buf[]  = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0}
                 * 
                 * float old_buf1[] = {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5, 10.5, 11.5, 12.5, 13.5} */

                int count, offset;
                float old_buf[]  = {0.5, 1.5, 2.5, 3.0, 4.5, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,  0.0,  0.0, 13.5};
                float new_buf[]  = {-1., -1., -1., -1., -1., -1., -1., -1., -1., -1., -1.,   -1.,  -1., -1.};
                int members[3] = {0,0,1};
                void *pbuf = (void *) &new_buf[0]; 

                /* read 3 values starting at index 2 */
                offset = 2;
                count = 3;
                members[0] = offset;
                members[1] = count;
                saf_read_field(SAF_ALL, &time_coords, NULL, count, SAF_HSLAB, members, &pbuf);
                for (i = 0; i < count; i++)
                    if (new_buf[i] != old_buf[i+offset]) {
                        failed = true;
                        nerrors++;
                        break;
                    }

                /* read 1 tuple */
                members[0] = 5;
                saf_read_field(SAF_ALL, &time_coords, NULL, 1, SAF_TUPLES, members, &pbuf);
                if (new_buf[0] != old_buf[5]) {
                    failed = true;
                    nerrors++;
                }

                /* read the whole thing as a partial */
                offset = 0;
                count = 11;
                members[0] = offset;
                members[1] = count;
                saf_read_field(SAF_ALL, &time_coords, NULL, count, SAF_HSLAB, members, &pbuf);
                for (i = 0; i < count; i++)
                    if (new_buf[i] != old_buf[i+offset]) {
                        failed = true;
                        nerrors++;
                        break;
                    }
            }

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }

        }

        /*---------------------------------------------------------------------------------------------------------------------
         *                                       COMPONENT/COMPOSITE FIELD READS
         *---------------------------------------------------------------------------------------------------------------------*/
        if (do_reads) {
            void *l_compositeBuffer=0,*l_xBuffer=0,*l_yBuffer=0;
            int l_num=0;
            size_t l_numEntries=0;
            SAF_Field *l_compositeCoordField=0,*l_xCoordField=0,*l_yCoordField=0;

            failed = false;
            TESTING("read component/composite field");

            /*find the composite field and its components*/
            saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), "coords", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
                            SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_num, &l_compositeCoordField);
            if (l_num != 1)
                failed = true;
     
            l_num=0;
            saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), "X", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
                            SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_num, &l_xCoordField);
            if (l_num != 1) 
                failed = true;
     
            l_num=0;
            saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), "Y", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
                            SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &l_num, &l_yCoordField);
            if (l_num != 1) 
                failed = true;
      
            /*read the composite field and the individual component fields*/
            if( !failed ) {
                saf_get_count_and_type_for_field(SAF_ALL, l_compositeCoordField+0, NULL, &l_numEntries, NULL);
                saf_read_field(SAF_ALL, l_compositeCoordField+0, NULL, SAF_WHOLE_FIELD, &l_compositeBuffer);
                saf_read_field(SAF_ALL, l_xCoordField+0, NULL, SAF_WHOLE_FIELD, &l_xBuffer);
                saf_read_field(SAF_ALL, l_yCoordField+0, NULL, SAF_WHOLE_FIELD, &l_yBuffer);
            }

            /*compare the results*/
            if( !failed ) {
                size_t k;
                float *l_compPtr = (float *)l_compositeBuffer;
                float *l_xPtr = (float *)l_xBuffer;
                float *l_yPtr = (float *)l_yBuffer;
                for(k=0;k<l_numEntries; k+=2 /*2 components, x&y*/ ) {
                    if( l_compPtr[0]!=l_xPtr[0] || l_compPtr[1]!=l_yPtr[0] ) {
                        failed = true;
                        break;
                    }
                    l_compPtr += 2;
                    l_xPtr++;
                    l_yPtr++;
                }
            }
     
            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }
        }



        /*---------------------------------------------------------------------------------------------------------------------
         *                                               STATES AND SUITES
         *---------------------------------------------------------------------------------------------------------------------*/

        {
            int number, num_state_grps, num_states;
            char *name;

            hid_t data_type;

            SAF_Set base;
            SAF_Set *param_set;
            SAF_Suite tmp_suite[1];
            SAF_Suite *suites;
            SAF_StateTmpl stmpl[1];
            SAF_StateGrp *state_grps;
            SAF_Field    *the_fields;
            SAF_Field coord_fld;
            SAF_Quantity tmp_quantity[1];
            SAF_Unit tmp_unit[1];
            hid_t tmp_coord_type[1];
	


            failed = false;
            TESTING("states and suites");

            if(do_reads) {
                /* find all the suites in the database; should be 1 */
                suites = NULL;
                number = 0;
                saf_find_suites (SAF_ALL, db, SAF_ANY_NAME, &number, &suites);
                if (number != 1) {
                    FAILED;
                    failed = true;
                    fprintf (stderr, "...expected 1 suite; found %d\n", number);
                }

                /* find the one named "CPBS" */
                suites = NULL;
                number = 0;
                saf_find_suites (SAF_ALL, db, "CPBS", &number, &suites);
                if (number != 1) {
                    FAILED;
                    failed = true;
                    fprintf (stderr, "...expected 1 suite named \"CPBS\"; found %d\n", number);
                }

                param_set = NULL;
                saf_describe_suite (SAF_ALL, suites+0, NULL, NULL, NULL, &param_set);
                saf_find_state_groups(SAF_ALL, suites+0, NULL, &num_state_grps, NULL);
                state_grps = NULL;
                saf_find_state_groups(SAF_ALL, suites+0, NULL, &num_state_grps, &state_grps);
	  
                if(num_state_grps!=1) {
                    FAILED;
                    failed=true;
                    fprintf(stderr,"...expected 1 state group, found %d\n",num_state_grps);
                }
	  
                name = NULL;
                saf_describe_state_group(SAF_ALL, state_grps+0, &name,tmp_suite, stmpl, tmp_quantity, tmp_unit, tmp_coord_type, &num_states);
                if( num_states != 3 ) {
                    FAILED;
                    failed = true;
                    fprintf(stderr,"...expected 3 states in state_group, found %d\n",num_states);
	      
                }
                if( !H5Tequal(tmp_coord_type[0], SAF_FLOAT) ) {
                    FAILED;
                    failed = true;
                    fprintf(stderr, "...expected state_group coord_data_type to be SAF_FLOAT\n");
                }
                if( !SAF_EQUIV(tmp_quantity+0,SAF_QTIME) ) {
                    FAILED;
                    failed = true;
                    fprintf(stderr, "...expected state_group quantity to be SAF_QTIME\n");
                }
                if( !SAF_EQUIV(tmp_unit+0,saf_find_one_unit(db, "second", NULL)) ) {
                    FAILED;
                    failed = true;
                    fprintf(stderr, "...expected state_group unit to be \"second\"\n");
                }
	  
                the_fields = NULL;
                saf_read_state(SAF_ALL, state_grps+0, 1, NULL, &coord_fld, NULL, &the_fields ); 


                /* ok, now do the real describe we want this suite */
                saf_describe_field (SAF_ALL, &coord_fld, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
                                    &data_type, NULL, NULL, NULL, NULL);
                if (!H5Tequal(data_type, H5T_NATIVE_FLOAT)) {
                    FAILED;
                    failed = true;
                    fprintf (stderr, "...wrong data type for coordinate field of suite \"CPBS\"\n");
                }
	  
                /* describe the first field in this state; should be "displacements" */
	  
                name = NULL;
                saf_describe_field (SAF_ALL, the_fields+0, NULL, &name, &base, 
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	  
                if (strcmp(name,"displacements")) {
                    FAILED;
                    failed = true;
                    fprintf (stderr, "...expected field to be named \"displacements\" in suite \"CPBS\"; found %s\n", name);
                }

	  
                /* read the displacement field; compare it to the original */	  
                {
                    size_t i;
                    size_t count;
                    float old_buf[] = {.25,.25,   .25,.25,   .25,.25,   .25,.25,
                                       .25,.25,   .25,.25,   .25,.25,   .25,.25,
                                       .25,.25,   .25,.25,   .25,.25,   .25,.25,
                                       .25,.25,              .25,.25,   .25,.25,
                                       .25,.25,              .25,.25,   .25,.25};
                    float new_buf[] = {.00,.00,   .00,.00,   .00,.00,   .00,.00,
                                       .00,.00,   .00,.00,   .00,.00,   .00,.00,
                                       .00,.00,   .00,.00,   .00,.00,   .00,.00,
                                       .00,.00,              .00,.00,   .00,.00,
                                       .00,.00,              .00,.00,   .00,.00};
                    void *pbuf = &new_buf[0];
	    
                    saf_read_field(SAF_ALL, the_fields+0, NULL, SAF_WHOLE_FIELD, &pbuf);
                    saf_get_count_and_type_for_field(SAF_ALL, the_fields+0, NULL, &count, NULL);
                    for (i = 0; i < count; i++)
                        if (new_buf[i] != old_buf[i]) {
                            failed = true;
                            nerrors++;
                            break;
                        }
                }	  
            }

            if (failed) {
                FAILED;
                nerrors++;
            } else {
                PASSED;
            }

        }

        saf_close_database(db);

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    saf_final();

theExit:

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return nerrors?1:0;
}
