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

ss_collection_t coll_g[40];             /* some collections */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Create some collections
 *
 * Description: Creates some collections in the specified scope.
 *
 * Return:      Number of errors.
 *
 * Parallel:    Collective across the SCOPE communicator.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 10, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tarray1_colls(ss_scope_t *scope, size_t *ncolls)
{
    int         i, n=0, self;

    if (ss_scope_comm(scope, NULL, &self, NULL)<0) return 1;
    
    /* Independently created (but common across all tasks) collections */
    for (i=0; i<10; i++, n++) {
        if (NULL==ss_pers_new(scope, SS_MAGIC(ss_collection_t), NULL, 0, (ss_pers_t*)(coll_g+n), NULL)) return 1;
        SS_COLLECTION(coll_g+n)->count = n;
    }

    /* Independently created collections at each task */
    for (i=0; i<10; i++, n++) {
        if (NULL==ss_pers_new(scope, SS_MAGIC(ss_collection_t), NULL, 0, (ss_pers_t*)(coll_g+n), NULL)) return 1;
        SS_COLLECTION(coll_g+n)->count = (self+1)*1000000 + n;
    }

    /* Collectively created collections */
    for (i=0; i<10; i++, n++) {
        if (NULL==ss_pers_new(scope, SS_MAGIC(ss_collection_t), NULL, SS_ALLSAME, (ss_pers_t*)(coll_g+n), NULL)) return 1;
        SS_COLLECTION(coll_g+n)->count = n;
    }

    /* Independently created collections on just one task */
    if (0==self) {
        for (i=0; i<10; i++, n++) {
            if (NULL==ss_pers_new(scope, SS_MAGIC(ss_collection_t), NULL, 0, (ss_pers_t*)(coll_g+n), NULL)) return 1;
            SS_COLLECTION(coll_g+n)->count = n;
        }
    }

    *ncolls = n;
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Collectively adding elements
 *
 * Description: Create a `set' and collectively add some collection objects to the `colls' array.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 10, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tarray1a(void)
{
    int         nerr=0, self;
    size_t      ncolls;
    FILE        *_print=NULL;
    ss_file_t   *file1=NULL;
    ss_scope_t  *tscope1=NULL;
    ss_set_t    *set1=NULL, *set2=NULL;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("collectively creating array") {
        /* Create new files for this test */
        file1 = ss_file_create("tarray1_a1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");

        /* Create collections */
        if (tarray1_colls(tscope1, &ncolls)) SS_FAILED_WHEN("creating collections");

        /* Create a set and make it point to a collection */
        set1 = SS_PERS_NEW(tscope1, ss_set_t, 0); /*omit SS_ALLSAME just to test sync*/
        ss_string_set(SS_SET_P(set1,name), "set 1");
        if (ss_array_resize(SS_SET_P(set1,colls), 1)<0) SS_FAILED_WHEN("extending set1 colls array");
        if (ss_array_put(SS_SET_P(set1,colls), ss_pers_tm, 0, 1, coll_g)<0) SS_FAILED_WHEN("ss_array_put set1 failed");

        /* Create another set and make it point to lots of collections */
        assert(ncolls>=10);
        set2 = SS_PERS_NEW(tscope1, ss_set_t, 0); /*omit SS_ALLSAME to force a sync*/
        ss_string_set(SS_SET_P(set2,name), "set 2");
        if (ss_array_resize(SS_SET_P(set2,colls), 10)<0) SS_FAILED_WHEN("extending set2 colls array");
        if (ss_array_put(SS_SET_P(set2,colls), 911/*unused*/, 0, 10, coll_g)<0) SS_FAILED_WHEN("ss_array_put set2 failed");

        /* Close the file */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Read an array
 *
 * Description: Reads an array from the file and verifies that tarray1a() functioned correctly.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Monday, March 15, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tarray1a_read(void)
{
    int         nerr=0, self;
    size_t      nsets;
    FILE        *_print=NULL;
    ss_file_t   *file1=NULL, *file2=NULL;
    ss_scope_t  *tscope1=NULL, *tscope2=NULL;
    ss_set_t    *sets=NULL, *key=NULL;
    ss_collection_t coll;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("reading an array") {
        /* Open existing files for this test */
        file1 = ss_file_open(NULL, "tarray1_a1.saf", H5F_ACC_RDONLY, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");

        file2 = ss_file_create("tarray1_a2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!file2) SS_FAILED_WHEN("creating file 2");
        tscope2 = ss_file_topscope(file2, NULL);
        if (!tscope2) SS_FAILED_WHEN("obtaining top scope 2");

        /* Find the sets */
        key = SS_PERS_NEW(tscope2, ss_set_t, 0);
        sets = SS_PERS_FIND(tscope1, key, NULL, SS_NOSIZE, nsets);
        if (2!=nsets) SS_FAILED_WHEN("expected two sets");

        /* The size of the array of the first set must be 1 */
        if (1!=ss_array_nelmts(SS_SET_P(sets+0,colls))) SS_FAILED_WHEN("set1 array nelmts != 1");
        if (NULL==ss_array_get(SS_SET_P(sets+0,colls), -1, 0, 1, &coll)) SS_FAILED_WHEN("set1 collection");
        if (_print) ss_pers_dump((ss_pers_t*)(coll_g+0), _print, "  ", "coll_g[0]");
        if (_print) ss_pers_dump((ss_pers_t*)&coll, _print, "  ", "set1.colls[0]");
        if (SS_PERS_EQ(&coll, coll_g+0)<=0) SS_FAILED_WHEN("comparing set1 collections");
        
        /* The size of the array of the second set must be 10 */
        if (10!=ss_array_nelmts(SS_SET_P(sets+1,colls))) SS_FAILED_WHEN("set2 array nelmts != 10");

        /* Close files */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        if (ss_file_close(file2)<0) SS_FAILED_WHEN("closing file 2");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*ARGSUSED*/
int
main(int UNUSED_SERIAL argc, char UNUSED_SERIAL *argv[])
{
    int         nerr=0;

    /* Initialization */
#ifdef HAVE_PARALLEL
    MPI_Init(&argc, &argv);
#endif
    ss_init(SS_COMM_WORLD);

    /* Valid Tests */
    nerr += tarray1a();
    nerr += tarray1a_read();

    /* Finalization */
    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return nerr?1:0;
}
