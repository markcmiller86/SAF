/*
 * Copyright(C) 2004-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 2004-2005 Sandia Corporation.
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
 * Authors:
 *     Robb P. Matzke              LLNL
 *     Eric A. Illescas            SNL
 * 
 * Acknowledgements:
 *     Mark C. Miller              LLNL - Design input
 * 
 */
#include "sslib.h"

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Searching
 * Purpose:     Print quantities
 *
 * Description: Prints information about specified quantities to stdout.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static void
tfind1_print(FILE *stream, size_t n, ss_pers_t *q)
{
    size_t              i, j;

    if (!stream) return;
    
    if (n && !q) {
        fprintf(stream, "    !!! trying to print %lu item%s but quantity link array is null !!!\n",
               (unsigned long)n, 1==n?"":"s");
        return;
    }

    fprintf(stream, "  found %lu items%s\n", (unsigned long)n, n?":":".");
    for (i=0; i<n; i++) {
        fprintf(stream, "  ");
        fprintf(stream, "  name=\"%s\"", ss_string_ptr(SS_QUANTITY_P(q+i,name)));
        fprintf(stream, ", url=\"%s\"", ss_string_ptr(SS_QUANTITY_P(q+i,url)));
        fprintf(stream, ", abbr=\"%s\"", ss_string_ptr(SS_QUANTITY_P(q+i,abbr)));
        fprintf(stream, ", flags=0x%08x", SS_QUANTITY(q+i)->flags);
        fprintf(stream, ", power=(");
        for (j=0; j<7; j++)
            fprintf(stream, "%s%d", j?",":"", SS_QUANTITY(q+i)->power[j]);
        fprintf(stream, ")\n");
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Searching
 * Purpose:     
 *
 * Description: 
 *
 * Return:      
 *
 * Parallel:    
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tfind1a(hbool_t rdonly)
{
    int                 nerr=0, self, i;
    ss_file_t           *file=NULL, *tempfile=NULL, *regfile=NULL;
    ss_scope_t          *tscope=NULL, *temp=NULL, *registry=NULL;
    ss_quantity_t       *q=NULL, *key=NULL;
    ss_quantityobj_t    mask;
    ss_pers_t           *found=NULL; /*actually quantity links*/
    ss_prop_t           *no_registries=NULL, *detect_overflow=NULL;
    FILE                *_print=NULL;
    char                name[256], url[256], abbr[256];
    size_t              nfound;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("file initialization") {
        /* Create or open the file for this test */
        if (!rdonly) {
            file = ss_file_create("tfind1_a1.saf", H5F_ACC_RDWR, NULL);
            if (!file) SS_FAILED_WHEN("creating file");
        } else {
            file = ss_file_open(NULL, "tfind1_a1.saf", H5F_ACC_RDONLY, NULL);
            if (!file) SS_FAILED_WHEN("opening file");
        }
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");

        /* Create a transient file for holding the key and mask objects */
        tempfile = ss_file_create("tfind1_a2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!tempfile) SS_FAILED_WHEN("creating transient file");
        temp = ss_file_topscope(tempfile, NULL);
        if (!temp) SS_FAILED_WHEN("obtaining temp scope");

        /* Create a simple transient registry file containing a few quantities */
        regfile = ss_file_create("tfind1_a3.saf", H5F_ACC_TRANSIENT, NULL);
        if (!regfile) SS_FAILED_WHEN("creating registry");
        registry = ss_file_topscope(regfile, NULL);
        if (!registry) SS_FAILED_WHEN("obtaining registry scope");
        q = SS_PERS_NEW(registry, ss_quantity_t, SS_ALLSAME);
        if (!q) SS_FAILED_WHEN("creating registry quantity");
        ss_string_set(SS_QUANTITY_P(q,name), "reg name");
        ss_string_set(SS_QUANTITY_P(q,url), "reg url");
        ss_string_set(SS_QUANTITY_P(q,abbr), "reg abbr");
        SS_QUANTITY(q)->flags = 1;
        SS_QUANTITY_M(q,power)[0] = 2;
        SS_QUANTITY_M(q,power)[1] = 3;
        SS_QUANTITY_M(q,power)[2] = 4;
        SS_QUANTITY_M(q,power)[3] = 5;
        SS_QUANTITY_M(q,power)[4] = 6;
        SS_QUANTITY_M(q,power)[5] = 7;
        SS_QUANTITY_M(q,power)[6] = 8;

        /* Bind registry to the main file */
        if (ss_file_registry(file, registry)<0) SS_FAILED_WHEN("attaching registry");
        
        /* Create a ton of quantity objects with various values */
        if (!rdonly) {
            for (i=0; i<1000; i++) {
                sprintf(name, "q%04d", i);
                sprintf(url, "div-100=%03d", i/100);
                sprintf(abbr, "mod-50=%02d", i%50);
                q = SS_PERS_NEW(tscope, ss_quantity_t, SS_ALLSAME);
                if (!q) SS_FAILED_WHEN("creating quantity");
                ss_string_set(SS_QUANTITY_P(q,name), name);
                ss_string_set(SS_QUANTITY_P(q,url), url);
                ss_string_set(SS_QUANTITY_P(q,abbr), abbr);
                SS_QUANTITY(q)->flags = i;
                SS_QUANTITY_M(q,power)[0] = i % 128;
                SS_QUANTITY_M(q,power)[1] = i % 64;
                SS_QUANTITY_M(q,power)[2] = i % 32;
                SS_QUANTITY_M(q,power)[3] = i % 16;
                SS_QUANTITY_M(q,power)[4] = i % 8;
                SS_QUANTITY_M(q,power)[5] = i % 4;
                SS_QUANTITY_M(q,power)[6] = i % 2;
            }
        }
    } SS_END_CHECKING_WITH(return 1);

    no_registries = ss_prop_new("no registries");
    ss_prop_add(no_registries, "noregistries", H5T_NATIVE_HBOOL, &true);
    detect_overflow = ss_prop_new("detect overflow");
    ss_prop_add(detect_overflow, "detect_overflow", H5T_NATIVE_HBOOL, &true);

    SS_CHECKING("key setup") {
        key = SS_PERS_NEW(temp, ss_quantity_t, SS_ALLSAME);
        if (!key) SS_FAILED_WHEN("creating key");
        ss_string_set(SS_QUANTITY_P(key,name), "q0200");
        ss_string_set(SS_QUANTITY_P(key,abbr), "XXXXX");
        SS_QUANTITY(key)->flags = 1;
        SS_QUANTITY(key)->power[0] = 104;
        SS_QUANTITY_M(key,power)[1] = 40;
    } SS_END_CHECKING_WITH(return 1);

    /* Searching with an empty (but non-null) mask should fail even when a buffer is supplied */
    SS_CHECKING("search with empty mask") {
        ss_pers_t supplied_buffer=SS_PERS_NULL;
        nfound = 1; /*size of supplied buffer*/
        memset(&mask, 0, sizeof mask);
        H5E_BEGIN_TRY {
            found = ss_pers_find(tscope, (ss_pers_t*)key, (ss_persobj_t*)&mask, 0, &nfound, &supplied_buffer, NULL);
        } H5E_END_TRY;
        tfind1_print(_print, nfound, found);
        if (found) SS_FAILED_WHEN("expected null return");
        if (nfound!=0) SS_FAILED_WHEN("expected zero matches");
    } SS_END_CHECKING_WITH(nerr++);

    /* Count how many items total. */
    SS_CHECKING("count with empty mask") {
        nfound = SS_NOSIZE; /* do not limit matches */
        found = ss_pers_find(tscope, (ss_pers_t*)key, NULL, 100, &nfound, SS_PERS_TEST, NULL);
        if (_print) fprintf(_print, "  counted %lu matches\n", (unsigned long)nfound);
        if (SS_PERS_TEST!=found) SS_FAILED_WHEN("expected SS_PERS_TEST return");
        if (900!=nfound) SS_FAILED_WHEN("expected 900 matches");
    } SS_END_CHECKING_WITH(nerr++);

    /* Count how many items but with a limit that is too low. */
    SS_CHECKING("limited count") {
        nfound = 100;
        found = ss_pers_find(tscope, (ss_pers_t*)key, NULL, 100, &nfound, SS_PERS_TEST, NULL);
        if (_print) fprintf(_print, "  counted %lu matches\n", (unsigned long)nfound);
        if (SS_PERS_TEST!=found) SS_FAILED_WHEN("expected SS_PERS_TEST return");
        if (100!=nfound) SS_FAILED_WHEN("expected 100 matches");
    } SS_END_CHECKING_WITH(nerr++);

    /* Count how many items but with a limit that is too low, and request failure. */
    SS_CHECKING("count with overflow detection") {
        nfound = 100;
        H5E_BEGIN_TRY {
            found = ss_pers_find(tscope, (ss_pers_t*)key, NULL, 100, &nfound, SS_PERS_TEST, detect_overflow);
        } H5E_END_TRY;
        if (found) SS_FAILED_WHEN("expected an overflow error");
    } SS_END_CHECKING_WITH(nerr++);
    
    /* Look for something that doesn't exist */
    SS_CHECKING("nonexistent value") {
        if (_print) fprintf(_print, "  looking for abbr=\"%s\"\n", ss_string_ptr(SS_QUANTITY_P(key,abbr)));
        memset(&mask, 0, sizeof mask);
        memset(&(mask.abbr), SS_VAL_CMP_DFLT, 1);
        found = SS_PERS_FIND(tscope, key, &mask, SS_NOSIZE, nfound);
        tfind1_print(_print, nfound, found);
        if (0!=nfound) SS_FAILED_WHEN("expected no matches");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);
    
    /* Find quantity with specified flags */
    SS_CHECKING("search by flags") {
        if (_print) fprintf(_print, "  looking for flags=0x%08x\n", SS_QUANTITY(key)->flags);
        memset(&mask, 0, sizeof mask);
        mask.flags = SS_VAL_CMP_DFLT;
        found = SS_PERS_FIND(tscope, key, &mask, SS_NOSIZE, nfound);
        tfind1_print(_print, nfound, found);
        if (1!=nfound) SS_FAILED_WHEN("expected one match");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);

    /* Find quantity with name */
    SS_CHECKING("search by exact name") {
        if (_print) fprintf(_print, "  looking for name=\"%s\"\n", ss_string_ptr(SS_QUANTITY_P(key,name)));
        memset(&mask, 0, sizeof mask);
        memset(&(mask.name), SS_VAL_CMP_DFLT, 1);
        found = SS_PERS_FIND(tscope, key, &mask, SS_NOSIZE, nfound);
        tfind1_print(_print, nfound, found);
        if (1!=nfound) SS_FAILED_WHEN("expected one match");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);
    
    /* Find quantities with power[1] */
    SS_CHECKING("search by power[1]") {
        if (_print) fprintf(_print, "  looking for power[1]=%d\n", SS_QUANTITY(key)->power[1]);
        memset(&mask, 0, sizeof mask);
        memset(mask.power+1, 0xff, 1);
        found = SS_PERS_FIND(tscope, key, &mask, 14, nfound);
        tfind1_print(_print, nfound, found);
        if (14!=nfound) SS_FAILED_WHEN("expected 14 matches");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);

    /* Find quantities with power[0,1] */
    SS_CHECKING("search by power[0,1]") {
        if (_print) fprintf(_print, "  looking for power[0,1]={%d,%d}\n", SS_QUANTITY(key)->power[0], SS_QUANTITY(key)->power[1]);
        memset(&mask, 0, sizeof mask);
        memset(mask.power+0, 0xff, sizeof(mask.power[0]));
        memset(mask.power+1, 0xff, sizeof(mask.power[1]));
        found = SS_PERS_FIND(tscope, key, &mask, SS_NOSIZE, nfound);
        tfind1_print(_print, nfound, found);
        if (7!=nfound) SS_FAILED_WHEN("expected 7 matches");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);

    /* Search for a name containing the substring "021" */
    SS_CHECKING("search name substring") {
        ss_string_set(SS_QUANTITY_P(key,name), "021");
        if (_print) fprintf(_print, "  looking for names containing \"%s\"\n", ss_string_ptr(SS_QUANTITY_P(key,name)));
        memset(&mask, 0, sizeof mask);
        memset(&(mask.name), SS_VAL_CMP_SUBSTR, 1);
        found = SS_PERS_FIND(tscope, key, &mask, SS_NOSIZE, nfound);
        tfind1_print(_print, nfound, found);
        if (11!=nfound) SS_FAILED_WHEN("expected 11 matches");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);

#if defined(HAVE_REGCOMP) && defined(HAVE_REGEXEC) && defined(REG_EXTENDED)
    /* Search for names that match a certain regular expression */
    SS_CHECKING("search name reg-exp") {
        ss_string_set(SS_QUANTITY_P(key,name), "[qQ].21[5-9]");
        if (_print) fprintf(_print, "  looking for names matching /%s/\n", ss_string_ptr(SS_QUANTITY_P(key,name)));
        memset(&mask, 0, sizeof mask);
        memset(&(mask.name), SS_VAL_CMP_RE, 1);
        found = SS_PERS_FIND(tscope, key, &mask, SS_NOSIZE, nfound);
        tfind1_print(_print, nfound, found);
        if (5!=nfound) SS_FAILED_WHEN("expected 5 matches");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);
#endif

    /* Search for something that will be found in the registry instead of the requested scope, but
     * turn off registry searching temporarily so that we don't find it. */
    SS_CHECKING("no registry search") {
        SS_QUANTITY(key)->power[6] = 8;
        if (_print) fprintf(_print, "  looking for power[6]=%d\n", SS_QUANTITY(key)->power[6]);
        memset(&mask, 0, sizeof mask);
        mask.power[6] = SS_VAL_CMP_DFLT;
        nfound = SS_NOSIZE;
        found = ss_pers_find(tscope, (ss_pers_t*)key, (ss_persobj_t*)&mask, 0, &nfound, NULL, no_registries);
        tfind1_print(_print, nfound, found);
        if (0!=nfound) SS_FAILED_WHEN("expected 0 matches");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);
    
    /* Search for something that will be found in the registry instead of the requested scope */
    SS_CHECKING("registry search") {
        SS_QUANTITY(key)->power[6] = 8;
        if (_print) fprintf(_print, "  looking for power[6]=%d\n", SS_QUANTITY(key)->power[6]);
        memset(&mask, 0, sizeof mask);
        mask.power[6] = SS_VAL_CMP_DFLT;
        found = SS_PERS_FIND(tscope, key, &mask, SS_NOSIZE, nfound);
        tfind1_print(_print, nfound, found);
        if (1!=nfound) SS_FAILED_WHEN("expected 1 match");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);

    /* Close the registry file and then search the same again -- the registry should have been detached
     * from `tscope' and thus the search will fail to find anything. */
    SS_CHECKING("registry closed search") {
        if (ss_file_close(regfile)<0) SS_FAILED_WHEN("closing registry");
        SS_QUANTITY(key)->power[6] = 8;
        if (_print) fprintf(_print, "  looking for power[6]=%d\n", SS_QUANTITY(key)->power[6]);
        memset(&mask, 0, sizeof mask);
        mask.power[6] = SS_VAL_CMP_DFLT;
        found = SS_PERS_FIND(tscope, key, &mask, SS_NOSIZE, nfound);
        tfind1_print(_print, nfound, found);
        if (0!=nfound) SS_FAILED_WHEN("expected 0 matches");
        if (!found) SS_FAILED_WHEN("expected non-null return");
        free(found);
        found = NULL;
    } SS_END_CHECKING_WITH(nerr++);

    ss_prop_dest(no_registries);
    no_registries = NULL;
    ss_prop_dest(detect_overflow);
    detect_overflow = NULL;
    
    /* Close the files */
    SS_CHECKING("closing files") {
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(tempfile)<0) SS_FAILED_WHEN("closing transient file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}


/*ARGSUSED*/
int
main(int UNUSED_SERIAL argc, char UNUSED_SERIAL *argv[])
{
    int         nerr=0, self;
    hbool_t     rdonly=FALSE;

    /* Initialization */
#ifdef HAVE_PARALLEL
    MPI_Init(&argc, &argv);
#endif
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ss_init(SS_COMM_WORLD);

    /* Parse command-line arguments */
    if (argc>1) {
        if (!strcmp(argv[1], "rdonly")) {
            rdonly = TRUE;
        } else {
            if (0==self)
                fprintf(stderr, "unknown argument: %s\n", argv[1]);
            nerr++;
            goto done;
        }
    }

    /* Tests */
    nerr += tfind1a(rdonly);
    
    /* Finalization */
done:
    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return nerr?1:0;
}
