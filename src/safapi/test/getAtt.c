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
#include <saf.h>
#include <testutil.h>

#define FREE_ARRAY(p,count)  {int looper; for(looper=0; looper<count; looper++){ free((p)[looper]);}}


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

static void
err_here(const char *s)
{
    printf("%s",s);
}


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
 *              - added more saf objects for more saf_get_xxx_att testing.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc,
     char **argv)
{
    char dbname[1024];
    hbool_t failed;
    hid_t str20;
    SAF_Db *db=NULL;
    SAF_Cat nodes;
    SAF_Set top, cell_1;
    SAF_Cat(cats, p_cats, 2);
    SAF_Set(sets, p_sets, 1);
    SAF_Rel(rels, p_rels, 1);
    SAF_StateTmpl(state_tmpls, p_state_tmpls,1);

    SAF_FieldTmpl(field_tmpls, p_field_tmpls, 1);
    SAF_Suite(suites, p_suites, 1);
    SAF_Field(fields, p_fields, 1);


    SAF_RelRep relrep;
    SAF_Rel rel1;
    SAF_Suite suite;
    SAF_StateTmpl st_tmpl;
    SAF_FieldTmpl field_tmpl;

    SAF_Field coord_compon[2];
    int failCount, num_cats, num_sets, num_rels, num_field_tmpls;
    int num_suites, num_state_tmpls, num_fields;
    const char strAttr1[20] = "Vader Lives";
    const char strAttr2[3][20] = {"Vader Lives", "Star Wars", "Obiwan Kenobi"};
    /* setAttr_t setAtt1 = {99, "Darth", 3.1415926};
     * setAttr_t setAtt2 = {100, "Vader", 3.1415926};
     * setAttr_t setAtt3 = {101, "Lives", 3.1415926};
     * relAttr_t relAtt1 = {-1, {0xAA, 1.01, 'R'}, "Robb", 123.456};
     * relAttr_t relAtt2 = {-2, {0xBB, 1.02, 'S'}, "Matzke", 123.456}; */

#ifdef HAVE_PARALLEL
    /* the MPI_init comes first because on some platforms MPICH's mpirun doesn't pass
     * the same argc, argv to all processors. However, the MPI spec says nothing about
     * what it does or might do to argc or argv. In fact, there is no "const" in the
     * function prototypes for either the pointers or the things they're pointing too.
     * I would rather pass NULL here and the spec says this is perfectly acceptable.
     * However, that too has caused MPICH to core on certain platforms. */
    MPI_Init(&argc,&argv);
#endif

    build_h5_types();
    str20 = H5Tcopy(H5T_C_S1);
    H5Tset_size(str20, 20);
    H5Tlock(str20);

    /* for convenience, set working directory to the test file directory */
    chdir(TEST_FILE_PATH);

    saf_init(SAF_DEFAULT_LIBPROPS);

    strcpy(dbname, TEST_FILE_NAME);

    SAF_TRY_BEGIN {

        /* note: because we are in a try block here, all failures will send us to the one and only catch block at the end of
         *       this test */

        db = saf_open_database(dbname,SAF_DEFAULT_DBPROPS);

        failCount = 0;

        /* get some handles to some objects */
        num_cats = 1;
        saf_find_categories(SAF_ALL, db, SAF_UNIVERSE(db), "nodes", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_cats, &p_cats);
        nodes = cats[0];


        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "TOP_CELL", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets); 
        top = sets[0];

        num_sets = 1;
        saf_find_matching_sets(SAF_ALL, db, "CELL_1", SAF_ANY_SILROLE, SAF_ANY_TOPODIM, SAF_EXTENDIBLE_TORF,
                               SAF_TOP_TORF, &num_sets, &p_sets); 
        cell_1 = sets[0];

        num_rels = 1;
        saf_find_subset_relations(SAF_ALL, db, &top, &cell_1, SAF_COMMON(&nodes), &num_rels, &p_rels);
        if (num_rels != 1)
            failed = TRUE;
        rel1 = rels[0];

        num_field_tmpls=1;
        saf_find_field_tmpls(SAF_ALL,db,"field_tmpl",SAF_ALGTYPE_ANY,NULL,NULL,&num_field_tmpls, &p_field_tmpls);
        if(num_field_tmpls != 1)
            failed=TRUE;
        field_tmpl=field_tmpls[0];


        num_suites=1;
        saf_find_suites (SAF_ALL, db, "INIT_SUITE", &num_suites, &p_suites);
        if(num_suites != 1)
            failed=TRUE;
        suite=suites[0];

        num_fields=1;
        saf_find_fields(SAF_ALL, db, SAF_UNIVERSE(db), "X", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
                        SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &num_fields, &p_fields);
        coord_compon[0] = fields[0];

        num_state_tmpls=1;
        saf_find_state_tmpl(SAF_ALL, db, "INIT_SUITE_STATE_TMPL", &num_state_tmpls, &p_state_tmpls);
        if(num_state_tmpls != 1)
            failed=TRUE;
        st_tmpl=state_tmpls[0];

        saf_find_one_relrep(db, "relrep", &relrep);

        TESTING("getting attributes known to exist");
        {
            hid_t type; 
            int count;
            setAttr_t readSetAtt, *PreadSetAtt = &readSetAtt;
            relAttr_t readRelAtt, *PreadRelAtt = &readRelAtt;

            failed = FALSE;

            count = 1;
            type = H5I_INVALID_HID;
            saf_get_set_att(SAF_ALL, &top, "testAtt1", &type, &count, (void**) &PreadSetAtt);
            if (count!=1 || !H5Tequal(type, setAttr_h5) || readSetAtt.gorfo!=99 ||
                strcmp(readSetAtt.foo,"Darth") || readSetAtt.bar!=3.1415926)
                failed = TRUE;

            count = 1;
            type = H5I_INVALID_HID;
            saf_get_set_att(SAF_ALL, &cell_1, "testAtt", &type, &count, (void**) &PreadSetAtt);
            if (count!=1 || !H5Tequal(type, setAttr_h5) || readSetAtt.gorfo!=99 || 
                strcmp(readSetAtt.foo,"Darth") || readSetAtt.bar!=3.1415926)
                failed = TRUE;
       
            /* the generic interface is used here because we didn't implement the SAF_Rel specific functions */
            count = 1;
            type = H5I_INVALID_HID;
            saf_get_attribute(SAF_ALL, (ss_pers_t*)&rel1, "testAtt", &type, &count, (void**) &PreadRelAtt);

            if (count!=1 || !H5Tequal(type, relAttr_h5) || readRelAtt.gorfo!=-1 ||
                readRelAtt.stuff.i!=0xAA || readRelAtt.stuff.foo!=1.01 || readRelAtt.stuff.bar!='R' ||
                strcmp(readRelAtt.foo,"Robb") || readRelAtt.bar!=123.456)
                failed = TRUE;

            if (failed) {
                failCount++;
                FAILED;
            } else {
                PASSED;
            }
	
            TESTING("saf_get_attribute with type CHAR");
            /* SSlib gets the type back as the same thing as the saf_put_attribute() without any of this nonsense about turning an
             * 1-byte integer array into a null terminated ASCII string. */
            {
                char *mychar=NULL;

                count = 1;
                type = H5T_NATIVE_CHAR;
                saf_get_attribute(SAF_ALL, (ss_pers_t*)&rel1, "charM", &type, &count, (void**) &mychar);

                if (count!=5 || !H5Tequal(type, H5T_NATIVE_CHAR) || strcmp(mychar, "MATT")) {
                    FAILED;
                } else {
                    PASSED;
                }
                free(mychar);

            }

            TESTING("saf_get_attribute on database"); 
            count = 1; 
            type = relAttr_h5; 

            saf_get_attribute(SAF_ALL, (ss_pers_t*)db, "putAttribute", &type, &count, (void**) &PreadRelAtt);

            if (count!=1 || !H5Tequal(type, relAttr_h5) || readRelAtt.gorfo!=-1 || 
                readRelAtt.stuff.i!=0xAA || readRelAtt.stuff.foo!=1.01 || readRelAtt.stuff.bar!='R' ||
                strcmp(readRelAtt.foo,"Robb") || readRelAtt.bar!=123.456)
                failed = TRUE;

            if (failed) {
                failCount++;
                FAILED;
            } else {
                PASSED;
            }

            TESTING("saf_get_attribute SAF_ATT_NAMES"); 
            {
                char **names=NULL;
                char *chars=NULL;
                int i;
                failed=FALSE;

                saf_get_attribute(SAF_ALL, (ss_pers_t*)db, SAF_ATT_NAMES, &type, &count, (void**) &names);
                for(i=0; i<count; i++){
                    if(strcmp(names[i], "putAttribute") &&
                       strcmp(names[i], "char1") &&
                       strcmp(names[i], "string2")) {
                        failed=TRUE;
                    }
                }
                FREE_ARRAY(names,count);
                if (count!=3 || failed) {
                    FAILED;
                } else {
                    PASSED;
                }

                TESTING("saf_get_attribute, char");
                type=H5T_NATIVE_CHAR;
                chars=NULL;
                count=0;

                saf_get_attribute(SAF_ALL, (ss_pers_t*)db, "char1", &type, &count, (void**) &chars);
                if (count==5 && H5Tequal(type, H5T_NATIVE_CHAR) && !strcmp(chars, "MATT")) {
                    PASSED;
                } else {
                    FAILED;
                }
                free(chars);

            }


        }

        /* It is no longer an error to try to get an attribute that doesn't exist. See HYPer03337. */
        TESTING("getting attributes known NOT to exist");
        {
            int n=0;
            saf_get_set_att(SAF_ALL, &cell_1, "gorfo", NULL, &n, NULL);
            if (n) {
                failCount++;
                FAILED;
            } else {
                PASSED;
            }
        }

        TESTING("getting string valued attributes");
        {
            int myfailCount = failCount;
            int i,count;
            char *p1 = NULL;
            char *p2 = NULL;
            hid_t type = H5I_INVALID_HID;

            saf_get_set_att(SAF_ALL, &top, "string1", &type, &count, (void**) &p1);
            if ((count != 1) || !H5Tequal(type, str20) || strcmp(p1, strAttr1)) {
                failCount++;
                err_here("1\n");
            }
            free(p1);
            H5Tclose(type);

            count=0;
            type=H5I_INVALID_HID;
            p2 = NULL;
            saf_get_set_att(SAF_ALL, &top, "string2", &type, &count, (void**) &p2);
            if (count==3 && H5Tequal(type, str20)) {
                for (i = 0; i < count; i++) {
                    if (strcmp(p2+i*20,strAttr2[i])) {
                        failCount++;
                        err_here("1.5\n");
                    }
                }
            } else {
                failCount++;
                err_here("2\n");
            }
            p2 = SS_FREE(p2);
            H5Tclose(type);

            count=0;
            type=H5I_INVALID_HID;
            p2 = NULL;
            saf_get_cat_att(SAF_ALL, &nodes, "string2", &type, &count, (void **)&p2);
            if (count==3 && H5Tequal(type, str20)) {
                for (i = 0; i < count; i++) {
                    if (strcmp(p2+i*20,strAttr2[i])) {
                        failCount++;
                        err_here("2.5\n");
                    }
                }
            } else {
                failCount++;
                err_here("3\n");
            }
            p2 = SS_FREE(p2);
            H5Tclose(type);

            count=0;
            type=H5I_INVALID_HID;
            p2 = NULL;
            saf_get_field_tmpl_att(SAF_ALL, &field_tmpl, "string2", &type, &count, (void **)&p2);
            if (count==3 && H5Tequal(type, str20)) {
                for (i = 0; i < count; i++){
                    if (strcmp(p2+i*20,strAttr2[i])) {
                        failCount++;
                        err_here("4\n");
                    }
                }
            } else {
                failCount++;
                err_here("5\n");
            }
            p2 = SS_FREE(p2);
            H5Tclose(type);

            count=0;
            type=H5I_INVALID_HID;
            p2 = NULL;
            saf_get_state_tmpl_att(SAF_ALL, &st_tmpl, "string2", &type, &count, (void **)&p2);
            if (count==3 && H5Tequal(type, str20)) {
                for (i = 0; i < count; i++) {
                    if (strcmp(p2+i*20,strAttr2[i])) {
                        failCount++;
                        err_here("6\n");
                    }
                }
            } else {
                failCount++;
                err_here("7\n");

            }
            p2 = SS_FREE(p2);
            H5Tclose(type);

            count=0;
            type=H5I_INVALID_HID;
            p2 = NULL;
            saf_get_suite_att(SAF_ALL, &suite, "string2", &type, &count, (void**) &p2);
            if (count==3 && H5Tequal(type, str20)) {
                for (i = 0; i < count; i++) {
                    if (strcmp(p2+i*20,strAttr2[i])){
                        failCount++;
                        err_here("8\n");
                    }
                }
            } else {
                failCount++;
                err_here("9\n");
            }
            p2 = SS_FREE(p2);
            H5Tclose(type);

            count=0;
            type=H5I_INVALID_HID;
            p2 = NULL;
            saf_get_field_att(SAF_ALL, coord_compon+0, "string2", &type, &count, (void**) &p2);
            if (count==3 && H5Tequal(type, str20)) {
                for (i = 0; i < count; i++)
                    if (strcmp(p2+i*20,strAttr2[i])){
                        failCount++;
                        err_here("10\n");
                    }
            } else {
                failCount++;
                err_here("11\n");
            }
            p2 = SS_FREE(p2);
            H5Tclose(type);

            if (myfailCount < failCount) {
                FAILED;
            } else {
                PASSED;
            }
        }

        TESTING("saf_get_attribute getting strings");
        {
            int count=0, i, myfailCount=failCount;
            hid_t type=H5I_INVALID_HID;
            char *p2 = NULL;
            saf_get_attribute(SAF_ALL, (ss_pers_t*)db, "string2", &type, &count, (void**) &p2);
            if (count==3 && H5Tequal(type, str20)) {
                for (i = 0; i < count; i++) {
                    if (strcmp(p2+i*20,strAttr2[i])){
                        failCount++;
                        err_here("10\n");
                    }
                }
            } else {
                failCount++;
                err_here("11\n");
            }
            p2 = SS_FREE(p2);
            if (myfailCount < failCount) {
                FAILED;
            } else {
                PASSED;
            }
        }

        TESTING("getting the count of the number of attributes, SAF_ATT_COUNT query");
        {
            int count;

            saf_get_set_att(SAF_ALL, &top, SAF_ATT_COUNT, NULL, &count, NULL);
            if (count != 5) {
                failCount++;
                FAILED;
            } else {
                PASSED;
            }
        }

        TESTING("getting the names of the attributes, SAF_ATT_NAMES query");
        {
            int count,i;
            char **names = NULL;
            hid_t type = H5I_INVALID_HID;
            hbool_t testFailed = FALSE;

            saf_get_set_att(SAF_ALL, &top, SAF_ATT_NAMES, &type, &count, (void**) &names);
            if (count != 5) {
                testFailed = TRUE;
            } else {
                for (i = 0; i < count; i++) {
                    /* we can't be assured of the order in which names are returned */
                    if (strcmp(names[i], "testAtt1") && strcmp(names[i], "testAtt2") && strcmp(names[i], "testAtt3") &&
                        strcmp(names[i], "string1") && strcmp(names[i], "string2")) {
                        testFailed = TRUE;
                        break;
                    }
                }
            }
            if (testFailed) {
                failCount++;
                FAILED;
            } else {
                PASSED;
            }
            FREE_ARRAY(names,count);
        }


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
 
