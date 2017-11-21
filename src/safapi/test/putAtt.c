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
#include <sys/types.h>
#include <sys/stat.h>
#include <safP.h>
#include <testutil.h>

/* A couple of more exotic datatypes to use as attributes. The corresponding HDF5 datatypes are also generated in the following
 * two functions, so don't change one without the other! */
typedef struct {
    int         gorfo;
    char        foo[10];
    double      bar;
} setAttr_t;
hid_t setAttr_h5 = -1;

typedef struct {
    int         gorfo;
    struct {
        short   i;
        double  foo;
        char    bar;
    } stuff;
    char        foo[10];
    double      bar;
} relAttr_t;
hid_t relAttr_h5 = -1;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Tests
 * Purpose:     Build hdf5 datatypes
 *
 * Description: Builds HDF5 datatypes corresponding to setAttr_t and relAttr_t.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, August  2, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static void
build_h5_types(void)
{
    hid_t       str10 = H5Tcopy(H5T_C_S1);

    H5Tset_size(str10, 10);
    
    /* setAttr_t */
    setAttr_h5 = H5Tcreate(H5T_COMPOUND, sizeof(setAttr_t));
    H5Tinsert(setAttr_h5, "gorfo", HOFFSET(setAttr_t, gorfo), H5T_NATIVE_INT);
    H5Tinsert(setAttr_h5, "foo",   HOFFSET(setAttr_t, foo),   str10);
    H5Tinsert(setAttr_h5, "bar",   HOFFSET(setAttr_t, bar),   H5T_NATIVE_DOUBLE);
    H5Tlock(setAttr_h5);

    /* relAttr_t */
    relAttr_h5 = H5Tcreate(H5T_COMPOUND, sizeof(relAttr_t));
    H5Tinsert(relAttr_h5, "gorfo",     HOFFSET(relAttr_t, gorfo),     H5T_NATIVE_INT);
    H5Tinsert(relAttr_h5, "stuff.i",   HOFFSET(relAttr_t, stuff.i),   H5T_NATIVE_SHORT);
    H5Tinsert(relAttr_h5, "stuff.foo", HOFFSET(relAttr_t, stuff.foo), H5T_NATIVE_DOUBLE);
    H5Tinsert(relAttr_h5, "stuff.bar", HOFFSET(relAttr_t, stuff.bar), H5T_NATIVE_CHAR);
    H5Tinsert(relAttr_h5, "foo",       HOFFSET(relAttr_t, foo),       str10);
    H5Tinsert(relAttr_h5, "bar",       HOFFSET(relAttr_t, bar),       H5T_NATIVE_DOUBLE);
}
    
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Tests 
 * Purpose:    	Attribute tests	
 *
 * Description: This is testing code that exercises SAF's attribute interface. It creates a few different kinds of
 *		objects and then puts and gets attributes for those objects closing the database between.
 *		It does compare the contents of the attributes for consistency.
 *
 *		Also, it exercises the special attribute queries, SAF_ATT_NAMES, SAF_ATT_COUNT. 
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Mark Miller, LLNL, Wed Mar 15, 2001
 *		- Initial implementation
 *
 *		MCM, LLNL, Sat Jun 30, 2001 
 *		- added test for string valued attribute
 *            
 *              Matthew O'Brien, LLNL, Oct 23, 2001
 *              - added more saf objects for more saf_put_xxx_att tests
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
    char dbname[1024];
    int failCount=0, self=0, dummy=37;
    int someInts[2] = {1138,1139};
    SAF_Db *db=NULL;
    SAF_DbProps *p=NULL;
    SAF_Cat nodes, elems, edges;
    SAF_Set top, cell_1, cell_2, cell_3, ss2, ns1, localCell;
    SAF_Rel rel1, rel2, rel3, rel4;
    SAF_FieldTmpl field_tmpl;
    SAF_StateGrp state_grp;
    SAF_Unit usec, umeter;
    SAF_Suite suite;
    SAF_StateTmpl st_tmpl;
    SAF_Field xfield;
    SAF_FieldTmpl fld_tmpls[4];
    SAF_FieldTmpl coords_ctmpl;
    hid_t str20;

    char mychar[]={'M','A','T', 'T', '\0'};
    char strAttr1[20] = "Vader Lives";
    char strAttr2[3][20] = {"Vader Lives", "Star Wars", "Obiwan Kenobi"};
    setAttr_t setAtt1 = {99, "Darth", 3.1415926};
    setAttr_t setAtt2 = {100, "Vader", 3.1415926};
    setAttr_t setAtt3 = {101, "Lives", 3.1415926};
    static setAttr_t dummyAttr[2]; /*initialized to zero*/
    relAttr_t relAtt1 = {-1, {0xAA, 1.01, 'R'}, "Robb", 123.456};
    relAttr_t relAtt2 = {-2, {0xBB, 1.02, 'S'}, "Matzke", 123.456};
    /*int intAtt[3] = {1414, 2718, 3141};*/

#ifdef HAVE_PARALLEL
    /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass
     * the same argc, argv to all processors. However, the MPI spec says nothing about
     * what it does or might do to argc or argv. In fact, there is no "const" in the
     * function prototypes for either the pointers or the things they're pointing too.
     * I would rather pass NULL here and the spec says this is perfectly acceptable.
      However, that too has caused MPICH to core on certain platforms. */
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &self);
#endif

    build_h5_types();
    str20 = H5Tcopy(H5T_C_S1);
    H5Tset_size(str20, 20);
    H5Tlock(str20);

    /* for convenience, set working directory to the test file directory */
    chdir(TEST_FILE_PATH);

    saf_init(SAF_DEFAULT_LIBPROPS);

    /*Some of the following tests are designed to fail, and require high pre-condition
     * check costs. This normally requires that the environment variable SAF_PRECOND_DISABLE
     * be set to "none". HOWEVER, mpi does not require environment variables to be passed
     * to programs. SAF handles the case when the env var is passed only to the root process.
     * MPICH passes all env vars to the root process, but LAM does not. So, in order for
     * these tests to work with LAM, we set this flag here. (JSJ 19aug03) */
    _SAF_GLOBALS.PreCondDisableCost = SAF_HIGH_CHK_COST;


    strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {

        /* note: because we are in a try block here, all failures will send us to the one and only
       catch block at the end of this test */

        p = saf_createProps_database();
        saf_setProps_Clobber(p);
        db = saf_open_database(dbname,p);

        /*We need to declare enough saf objects so we can test all the type specific saf_put_xxx_att functions*/

        /* declare some categories */
        saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, 0, &nodes);
        saf_declare_category(SAF_ALL, db, "elems", SAF_TOPOLOGY, 2, &elems);
        saf_declare_category(SAF_ALL, db, "edges", SAF_TOPOLOGY, 1, &edges);

        /* declare some sets */
        saf_declare_set(SAF_ALL, db, "TOP_CELL", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &top);
        saf_declare_set(SAF_ALL, db, "CELL_1", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_1);
        saf_declare_set(SAF_ALL, db, "CELL_2", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_2);
        saf_declare_set(SAF_ALL, db, "CELL_3", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &cell_3);
        saf_declare_set(SAF_EACH, db, "local cell", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &localCell);


        saf_declare_set(SAF_ALL, db, "SIDE_SET_2", 1, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ss2);
        saf_declare_set(SAF_ALL, db, "NODE_SET_1", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &ns1);

        /* declare some collections */
        saf_declare_collection(SAF_ALL, &top, &nodes, SAF_CELLTYPE_POINT, 18, SAF_1DC(18), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &top, &elems, SAF_CELLTYPE_MIXED, 12, SAF_1DC(12), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_1, &nodes, SAF_CELLTYPE_POINT, 9, SAF_1DC(9), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_1, &elems, SAF_CELLTYPE_QUAD, 4, SAF_1DC(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_2, &nodes, SAF_CELLTYPE_POINT, 7, SAF_1DC(7), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_2, &elems, SAF_CELLTYPE_MIXED, 4, SAF_1DC(4), SAF_DECOMP_TRUE);
        saf_declare_collection(SAF_ALL, &cell_3, &nodes, SAF_CELLTYPE_POINT, 10, SAF_1DC(10), SAF_DECOMP_FALSE);
        saf_declare_collection(SAF_ALL, &cell_3, &elems, SAF_CELLTYPE_QUAD, 4, SAF_1DC(4), SAF_DECOMP_TRUE);

        /* declare some relations */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                    NULL, &rel1);
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                    NULL, &rel2);


        /* nodes and elems of cell_2 and top */
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2, SAF_COMMON(&nodes), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                    NULL, &rel3);
        saf_declare_subset_relation(SAF_ALL, db, &top, &cell_2, SAF_COMMON(&elems), SAF_TUPLES, SAF_INT, NULL, H5I_INVALID_HID,
                                    NULL, &rel4);

        /* declare some field templates*/
        saf_declare_field_tmpl(SAF_ALL, db, "field_tmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,NULL, &field_tmpl);

        saf_declare_field_tmpl(SAF_ALL, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
                               NULL, &coords_ctmpl);



        /* create a suite*/
        saf_declare_suite(SAF_ALL, db, "INIT_SUITE", &top, NULL, &suite);

        saf_find_one_unit(db, "meter", &umeter);

        /* declare a field template*/
        saf_declare_field_tmpl(SAF_ALL, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR, SAF_CARTESIAN, SAF_QLENGTH, 1,
                               NULL, &coords_ctmpl);

 
        /*declare a field*/
        saf_declare_field(SAF_ALL, db, &coords_ctmpl, "X", &top, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems),
                          H5T_NATIVE_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &xfield);


        saf_declare_coords(SAF_ALL, &xfield);
        saf_declare_default_coords(SAF_ALL, &top, &xfield);
   
 
        fld_tmpls[0] = coords_ctmpl;
        fld_tmpls[1] = coords_ctmpl;

        /*declare a state template*/
        saf_declare_state_tmpl(SAF_ALL, db, "INIT_SUITE_STATE_TMPL", 2, fld_tmpls, &st_tmpl);

 
        /* create a state group for this suite */
        saf_find_one_unit(db, "second", &usec);
        saf_declare_state_group(SAF_ALL, db, "STATE_GROUP", &suite, &top, &st_tmpl, SAF_QTIME, &usec, SAF_FLOAT, &state_grp);


        /* ok, now we have several different kinds of objects to put/get some attributes for */
        TESTING("putting attributes (SAF_ALL mode)");
        saf_put_set_att(SAF_ALL, &top, "testAtt1", setAttr_h5, 1, &setAtt1);
        saf_put_set_att(SAF_ALL, &top, "testAtt2", setAttr_h5, 1, &setAtt2);
        saf_put_set_att(SAF_ALL, &top, "testAtt3", setAttr_h5, 1, &setAtt3);
        saf_put_set_att(SAF_ALL, &cell_1, "testAtt", setAttr_h5, 1, &setAtt1);

        /* the generic attribute interface is used here because we didn't implement the SAF_Rel specific versions */
        saf_put_attribute(SAF_ALL, (ss_pers_t*)&rel1, "testAtt", relAttr_h5, 1, &relAtt1);
        saf_put_attribute(SAF_ALL, (ss_pers_t*)&rel2, "testAtt", relAttr_h5, 1, &relAtt2);

        /* Issue: the following two cases have identical behavior.  so even if you write 5 DSL_CHAR'S, when
      you "get" the attribute, you'll get 1 DSL_STRING.  so you must null terminate your char array */

        saf_put_attribute(SAF_ALL, (ss_pers_t*)&rel1, "charM", H5T_NATIVE_CHAR, 5, mychar);
        saf_put_attribute(SAF_ALL, (ss_pers_t*)db, "putAttribute", relAttr_h5, 1, &relAtt1);
        saf_put_attribute(SAF_ALL, (ss_pers_t*)db, "char1", H5T_NATIVE_CHAR, 5, mychar);
        PASSED;

        /* DSL used to be able to handle something called SAF_STRING, which was a pointer to a variable length, NUL terminated
         * array of one-byte ASCII characters.  HDF5 handles strings slightly differently: it has variable length (VL) strings
         * and fixed length strings, both of which can describe non-ASCII character sets, multi-byte strings, NUL padded vs.
         * SPC padded (Fortran) vs. NUL terminated (C), etc.  However, the VL strings don't (as of 2004-08-02) work properly
         * for parallel applications and therefore we're stuck with fixed length strings. We could create a different fixed
         * length string type for each size string we have, but we'll just use arrays of 20 bytes for everything to make it
         * easier. */
        TESTING("putting string valued attributes");
        saf_put_set_att(SAF_ALL, &top, "string1", str20, 1, strAttr1);
        saf_put_set_att(SAF_ALL, &top, "string2", str20, 3, strAttr2);
        saf_put_cat_att(SAF_ALL, &nodes, "string2", str20, 3, strAttr2);
        saf_put_field_tmpl_att(SAF_ALL, &field_tmpl, "string2", str20, 3, strAttr2);
        saf_put_suite_att(SAF_ALL, &suite, "string2", str20, 3, strAttr2);
        saf_put_field_att(SAF_ALL, &xfield, "string2", str20, 3, strAttr2);
        saf_put_state_tmpl_att(SAF_ALL, &st_tmpl, "string2", str20, 3, strAttr2);    
        saf_put_attribute(SAF_ALL, (ss_pers_t*)db, "string2", str20, 3, strAttr2);

        /*new stuff to exercise more code of _saf_putAttribute_handle it looks like SAF_RelRep was the only way to make *_
         * saf_isaPersistent_handle return true */
        { 
            SAF_RelRep relrep;
    
            saf_declare_relrep(SAF_ALL, db, "relrep", "www.relrep.com", 3333, &relrep);
            saf_put_attribute(SAF_ALL, (ss_pers_t*)&relrep, "string2", str20, 3, strAttr2);
        }
    
        PASSED;

        TESTING("putting attributes (SAF_EACH mode)");
        /* should support different attribute names, types and sizes from each processor */
        saf_put_set_att(SAF_EACH, &localCell,
                        self%2 ? "testAttEachMode" : "someInts", 
                        self%2 ? setAttr_h5 : H5T_NATIVE_INT,
                        self%2 ? 1 : 2,
                        self%2 ? (void*)&setAtt1 : (void*)someInts);
        PASSED;

        TESTING("putting attributes (SAF_EACH_SPECIAL mode)");
        saf_put_set_att(SAF_EACH, &localCell, "testAttEachSpecialMode", setAttr_h5, 1, &setAtt1);
        PASSED;

        /* The purpose of the following tests (although buggy in the case of a single-task MPI job) was to check that SAF
         * could correctly detect cases where the callers supplied SAF_ALL as the parallel mode but then didn't all supply the
         * same arguments.  Detecting such cases required additional communication which the SSlib version does not perform.
         * However, SSlib does allows the arguments to vary across the calling tasks as long as the SAF_ALL mode is not used.
         * In other words, SSlib fully supports independent calls to saf_put_set_att() and related functions. */
        TESTING("independent attribute operations");
        saf_put_set_att(SAF_EACH, &localCell, "gorfoA", setAttr_h5, 1, &setAtt1);
        saf_put_set_att(SAF_EACH, &cell_1, self?"foobar":"gorfoB", setAttr_h5, 1, &setAtt1);
        saf_put_set_att(SAF_EACH, &cell_1, "gorfoC", self?SAF_INT:setAttr_h5, 1, self?(void*)&dummy:(void*)&setAtt1);
        saf_put_set_att(SAF_EACH, &cell_1, "gorfoD", setAttr_h5, self?1:2, self?&setAtt1:dummyAttr);
        saf_put_set_att(SAF_EACH, &cell_1, "gorfoE", setAttr_h5, 1, &setAtt1);
        saf_put_set_att(SAF_EACH, &cell_1, "gorfoF", setAttr_h5, 1, &setAtt1);
        saf_put_set_att(SAF_EACH, &localCell, self?"foobar":"gorfoG", setAttr_h5, 1, &setAtt1);
        saf_put_set_att(SAF_EACH, &localCell, "gorfoH", self?SAF_INT:setAttr_h5, 1, self?(void*)&dummy:(void*)&setAtt1);
        saf_put_set_att(SAF_EACH, &localCell, "gorfoI", setAttr_h5, self?1:2, self?&setAtt1:dummyAttr);
        PASSED;

        saf_close_database(db);

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            failCount += 1;
        }
    } SAF_TRY_END;

    saf_final();

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&failCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif

    return (failCount==0)? 0 : 1;

}
