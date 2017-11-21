/*
 *       Copyright(C) 1999-2005 The Regents of the University of California.
 *           This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *           Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *           U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *           the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *           dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *           Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *           Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *           University.
 *       
 *       Copyright(C) 1999-2005 Sandia Corporation.  
 *           Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
 *           on behalf of the U.S. Government.  Export  of this program may require a license from the United
 *           States Government.
 *       
 *       Disclaimer:
 *           This document was  prepared as an account of  work sponsored by an agency  of  the United States
 *           Government. Neither the United States  Government nor the United States Department of Energy nor
 *           the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any
 *           warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the
 *           accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process
 *           disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference
 *           herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark,
 *           manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement,
 *           recommendation, or favoring by the  United States Government   or the University of  California.
 *           The views and opinions of authors expressed herein do not necessarily state  or reflect those of
 *           the  United  States Government or  the   University of California   and shall  not be  used  for
 *           advertising or product endorsement purposes.
 *       
 *       
 *       Active Developers:
 *           Peter K. Espen              SNL
 *           Eric A. Illescas            SNL
 *           Jake S. Jones               SNL
 *           Robb P. Matzke              LLNL
 *           Greg Sjaardema              SNL
 *       
 *       Inactive Developers:
 *           William J. Arrighi          LLNL
 *           Ray T. Hitt                 SNL
 *           Mark C. Miller              LLNL
 *           Matthew O'Brien             LLNL
 *           James F. Reus               LLNL
 *           Larry A. Schoof             SNL
 *       
 *       Acknowledgements:
 *           Marty L. Barnaby            SNL - Red parallel perf. study/tuning
 *           David M. Butler             LPS - Data model design/implementation Spec.
 *           Albert K. Cheng             NCSA - Parallel HDF5 support
 *           Nancy Collins               IBM - Alpha/Beta user
 *           Linnea M. Cook              LLNL - Management advocate
 *           Michael J. Folk             NCSA - Management advocate 
 *           Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
 *           Wilbur R. Johnson           SNL - Early developer
 *           Quincey Koziol              NCSA - Serial HDF5 Support 
 *           Celeste M. Matarazzo        LLNL - Management advocate
 *           Tyce T. McLarty             LLNL - parallel perf. study/tuning
 *           Tom H. Robey                SNL - Early developer
 *           Reinhard W. Stotzer         SNL - Early developer
 *           Judy Sturtevant             SNL - Red parallel perf. study/tuning 
 *           Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
 *       
 */
#include <saf.h>
#include <testutil.h>

/*-------------------------------------------------------------------------------------------------------------------------------
 * Description: Test basic predefined quantities.
 *
 * Return:      Number of errors.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  4, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
test_pre_quant(void)
{
    int                 nerrors=0;
    SAF_Quantity        *q;
    int                 bq;
    unsigned            flags;
    unsigned int        power[SS_MAX_BASEQS];

    /* time */
    TESTING("predefined time quantity");
    SAF_TRY_BEGIN {
        q = SAF_QTIME;
        if (!q) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS!=saf_describe_quantity(SAF_ALL, q, NULL, NULL, NULL, &flags, power)) {
            FAILED;
            nerrors++;
        } else if (flags) {
            FAILED;
            nerrors++;
        } else {
            for (bq=0; bq<SS_MAX_BASEQS; bq++) {
                if (power[bq]!=(SAF_BASEQ_TIME==bq?1:0)) {
                    FAILED;
                    nerrors++;
                    break;
                }
            }
            if (bq>=SS_MAX_BASEQS) PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* mass */
    TESTING("predefined mass quantity");
    SAF_TRY_BEGIN {
        q =  SAF_QMASS;
        if (!q) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS!=saf_describe_quantity(SAF_ALL, q, NULL, NULL, NULL, &flags, power)) {
            FAILED;
            nerrors++;
        } else if (flags) {
            FAILED;
            nerrors++;
        } else {
            for (bq=0; bq<SS_MAX_BASEQS; bq++) {
                if (power[bq]!=(SAF_BASEQ_MASS==bq?1:0)) {
                    FAILED;
                    nerrors++;
                    break;
                }
            }
            if (bq>=SS_MAX_BASEQS) PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* current */
    TESTING("predefined current quantity");
    SAF_TRY_BEGIN {
        q = SAF_QCURRENT;
        if (!q) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS!=saf_describe_quantity(SAF_ALL, q, NULL, NULL, NULL, &flags, power)) {
            FAILED;
            nerrors++;
        } else if (flags) {
            FAILED;
            nerrors++;
        } else {
            for (bq=0; bq<SS_MAX_BASEQS; bq++) {
                if (power[bq]!=(SAF_BASEQ_CURRENT==bq?1:0)) {
                    FAILED;
                    nerrors++;
                    break;
                }
            }
            if (bq>=SS_MAX_BASEQS) PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* length */
    TESTING("predefined length quantity");
    SAF_TRY_BEGIN {
        q = SAF_QLENGTH;
        if (!q) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS!=saf_describe_quantity(SAF_ALL, q, NULL, NULL, NULL, &flags, power)) {
            FAILED;
            nerrors++;
        } else if (flags) {
            FAILED;
            nerrors++;
        } else {
            for (bq=0; bq<SS_MAX_BASEQS; bq++) {
                if (power[bq]!=(SAF_BASEQ_LENGTH==bq?1:0)) {
                    FAILED;
                    nerrors++;
                    break;
                }
            }
            if (bq>=SS_MAX_BASEQS) PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* light */
    TESTING("predefined light quantity");
    SAF_TRY_BEGIN {
        q = SAF_QLIGHT;
        if (!q) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS!=saf_describe_quantity(SAF_ALL, q, NULL, NULL, NULL, &flags, power)) {
            FAILED;
            nerrors++;
        } else if (flags) {
            FAILED;
            nerrors++;
        } else {
            for (bq=0; bq<SS_MAX_BASEQS; bq++) {
                if (power[bq]!=(SAF_BASEQ_LIGHT==bq?1:0)) {
                    FAILED;
                    nerrors++;
                    break;
                }
            }
            if (bq>=SS_MAX_BASEQS) PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* temp */
    TESTING("predefined temp quantity");
    SAF_TRY_BEGIN {
        q = SAF_QTEMP;
        if (!q) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS!=saf_describe_quantity(SAF_ALL, q, NULL, NULL, NULL, &flags, power)) {
            FAILED;
            nerrors++;
        } else if (flags) {
            FAILED;
            nerrors++;
        } else {
            for (bq=0; bq<SS_MAX_BASEQS; bq++) {
                if (power[bq]!=(SAF_BASEQ_TEMP==bq?1:0)) {
                    FAILED;
                    nerrors++;
                    break;
                }
            }
            if (bq>=SS_MAX_BASEQS) PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    /* amount */
    TESTING("predefined amount quantity");
    SAF_TRY_BEGIN {
        q = SAF_QAMOUNT;
        if (!q) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS!=saf_describe_quantity(SAF_ALL, q, NULL, NULL, NULL, &flags, power)) {
            FAILED;
            nerrors++;
        } else if (flags) {
            FAILED;
            nerrors++;
        } else {
            for (bq=0; bq<SS_MAX_BASEQS; bq++) {
                if (power[bq]!=(SAF_BASEQ_AMOUNT==bq?1:0)) {
                    FAILED;
                    nerrors++;
                    break;
                }
            }
            if (bq>=SS_MAX_BASEQS) PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;

    return nerrors;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Description: Test that new quantities can be defined.
 *
 * Return:      Number of errors
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  4, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
test_new_quant(SAF_Db *db)
{
    SAF_Quantity        *qn;
    int                 nerrors=0;

    TESTING("new quantity definition");
    SAF_TRY_BEGIN {
        qn = saf_declare_quantity(SAF_ALL, db, "new quantity 1", "nq1", NULL, NULL);
        if (!qn) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS != saf_multiply_quantity(SAF_ALL, qn, SAF_QLENGTH, 1)) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS != saf_multiply_quantity(SAF_ALL, qn, SAF_QMASS, 1)) {
            FAILED;
            nerrors++;
        } else if (SAF_SUCCESS != saf_divide_quantity(SAF_ALL, qn, SAF_QTIME, 2)) {
            FAILED;
            nerrors++;
        } else {
            PASSED;
        }
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            nerrors++;
        }
    } SAF_TRY_END;
    return nerrors;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Description: Tests the saf_find_quantities() function with various arguments.
 *
 * Return:      Number of errors
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  4, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
test_find_quant(SAF_Db *db)
{
    size_t              nelmts;
    int                 nerrors=0;
    SAF_Quantity        *key, *found=NULL;
    ss_quantityobj_t    mask;
    ss_file_t           *tempfile;
    ss_scope_t          tempscope, topscope;

    tempfile = ss_file_create("/saf:test_find_quant", H5F_ACC_TRANSIENT, NULL);
    ss_file_topscope(tempfile, &tempscope);
    

    key = SS_PERS_NEW(&tempscope, ss_quantity_t, SS_ALLSAME);
    memset(&mask, 0, sizeof mask);
    ss_file_topscope(db, &topscope);

    SAF_TRY_BEGIN {
        TESTING("find all quantities");
        nelmts = SS_NOSIZE;
        found = SS_PERS_FIND(&topscope, key, NULL, SS_NOSIZE, nelmts);
        if (!found) {
            FAILED;
            nerrors++;
        } else if (nelmts!=1) {
            FAILED;
            printf("  found %lu elements (expected one)\n", (unsigned long)nelmts);
            nerrors++;
        } else {
            PASSED;
        }
        found = SS_FREE(found);
        
        TESTING("find quantity by abbreviation");
        ss_string_set(SS_QUANTITY_P(key,abbr), "amount");
        memset(&mask, 0, sizeof mask);
        memset(&mask.abbr, SS_VAL_CMP_DFLT, 1);
        nelmts = SS_NOSIZE;
        found = SS_PERS_FIND(&topscope, key, &mask, SS_NOSIZE, nelmts);
        if (!found) {
            FAILED;
            nerrors++;
        } else if (1!=nelmts) {
            FAILED;
            printf("  found %lu elements\n", (unsigned long)nelmts);
            nerrors++;
        } else {
            PASSED;
        }
        

        TESTING("find quantity by power");
        saf_multiply_quantity(SAF_ALL, key, SAF_QTIME, 1);
        memset(&mask, 0, sizeof mask);
        memset(&mask.power, SS_VAL_CMP_DFLT, sizeof mask.power);
        nelmts = SS_NOSIZE;
        found = SS_PERS_FIND(&topscope, key, &mask, SS_NOSIZE, nelmts);
        if (!found) {
            FAILED;
            nerrors++;
        } else if (1!=nelmts) {
            FAILED;
            printf("  found %lu elements\n", (unsigned long)nelmts);
            nerrors++;
        } else {
            PASSED;
        }

    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            printf("  raised an exception\n");
            nerrors++;
        }
    } SAF_TRY_END;

    return nerrors;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Description: Test predefined units.
 *
 * Return:      Number of errors.
 *
 * Programmer:  Robb Matzke
 *              Friday, June 23, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
test_pre_unit(SAF_Db *db)
{
    SAF_Unit    *u;
    int         nerrors=0;
    char        *name=NULL, *abbr=NULL;
    double      scale=9999.9999, offset=9999.9999, logbase=9999.9999, logcoef=9999.9999;
    
    
    TESTING("predefined unit: meter");
    SAF_TRY_BEGIN {
        u = saf_find_one_unit(db, "meter", NULL);
        if (!u) {
            FAILED;
            printf("  saf_find_one_unit() failed to find first occurrence of \"meter\"\n");
            nerrors++;
        } else if (SAF_SUCCESS!=saf_describe_unit(SAF_ALL, u, &name, &abbr, NULL, &scale, &offset, &logbase, &logcoef, NULL)) {
            FAILED;
            printf("  saf_describe_unit() failed for \"meter\"\n");
            nerrors++;
        } else if (strcmp(name, "meter")) {
            FAILED;
            printf("  saf_describe_unit() returned name=\"%s\" instead of \"meter\"\n", name);
            nerrors++;
        } else if (strcmp(abbr, "m")) {
            FAILED;
            printf("  saf_describe_unit() returned abbr=\"%s\" instead of \"m\"\n", abbr);
            nerrors++;
        } else if (scale!=1.0) {
            FAILED;
            printf("  saf_describe_unit() return scale=%g instead of 1 (\"meter\" is a basic unit)\n", scale);
            nerrors++;
        } else if (offset) {
            FAILED;
            printf("  saf_describe_unit() returned offset=%g instead of zero\n", offset);
            nerrors++;
        } else if (logbase) {
            FAILED;
            printf("  saf_describe_unit() returned logbase=%g instead of zero\n", logbase);
            nerrors++;
        } else if (logcoef) {
            FAILED;
            printf("  saf_describe_unit() returned logcoef=%g instead of zero\n", logcoef);
            nerrors++;
        }
        PASSED;
    } SAF_CATCH {
        SAF_CATCH_ALL {
            FAILED;
            printf("  raised an exception\n");
            nerrors++;
        }
    } SAF_TRY_END;
    return nerrors;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Description: Various quantity and unit tests
 *
 * Return:      Zero on success, positive on failure.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  4, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
    SAF_Db              *db=NULL;
    SAF_DbProps         *db_props=NULL;
    int                 nerrors=0;

#ifdef HAVE_PARALLEL
    MPI_Init(&argc,&argv);
#endif
    
    /* Open the database */
    if (SAF_SUCCESS!=saf_init(SAF_DEFAULT_LIBPROPS)) {
        fprintf(stderr, "saf_init() failed\n");
        exit(1);
    }
    if (NULL==(db_props=saf_createProps_database())) {
        fprintf(stderr, "saf_createProps_database() failed\n");
        exit(1);
    }
    if (SAF_SUCCESS!=saf_setProps_Clobber(db_props)) {
        fprintf(stderr, "saf_setProps_Clobber() failed\n");
        exit(1);
    }
    if (NULL==(db=saf_open_database("quantities.saf", db_props))) {
        fprintf(stderr, "saf_open_database() failed\n");
        exit(1);
    }

    /* Check that we can open the 7 basic quantities and that their definitions are correct */
    nerrors += test_pre_quant();

    /* Check that we can define new quantities */
    nerrors += test_new_quant(db);

    /* Check that we can find quantities */
    nerrors += test_find_quant(db);

    /* Test saf_find_unit() and saf_describe_unit() on some predefined units. The remainder of the units API is tested by the
     * saf_open_database() call above, which calls _saf_init_unit() which makes extensive calls to the units API. */
    nerrors += test_pre_unit(db);

    /* Close the database */
    if (SAF_SUCCESS!=saf_close_database(db)) {
        fprintf(stderr, "saf_close_database() failed\n");
        exit(1);
    }
    saf_final();

#ifdef HAVE_PARALLEL
    /* make sure everyone returns the same error status */
    MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Finalize();
#endif
    return nerrors?1:0;
}
