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
 * Chapter:     File Operations
 * Purpose:     Basic file creation
 *
 * Description: Tests whether SSlib can create an empty file, close it, and then open it again, then finally close it.
 *
 * Return:      Number of errors.
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Friday, August 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tfile1a(void)
{
    int                 nerr=0;
    ss_file_t           *file1=NULL;
    int                 self;
    FILE                *_print=NULL;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("basic file creation/close/open") {
        /* Create a new file */
        file1 = ss_file_create("tfile1_a1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file");

        /* Close the file without having added anything to it */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing created file");
        file1 = NULL;

        /* Open the same file again */
        file1 = ss_file_open(NULL, "tfile1_a1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("opening file");

        /* Close the opened file. */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing opened file");
        file1 = NULL;
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     File Operations
 * Purpose:     File predicates
 *
 * Description: After creating a file, test whether the various file predicates operate as expected.
 *
 * Return:      Number of errors.
 *
 * Parallel:    Collective across MPI_COMM_WORLD.
 *
 * Programmer:  Robb Matzke
 *              Friday, August 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tfile1b(void)
{
    int                 nerr=0;
    ss_file_t           *file1=NULL;
    int                 self;
    FILE                *_print=NULL;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("permanent file predicates") {
        /* Create a new file */
        file1 = ss_file_create("tfile1_b1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file");

        /* Test that file is reported as open */
        if (ss_file_isopen(file1, NULL)<=0) SS_FAILED_WHEN("ss_file_isopen() on file object");
        if (ss_file_isopen(NULL, "tfile1_b1.saf")<=0) SS_FAILED_WHEN("ss_file_isopen() on file name");
        if (ss_file_isopen(NULL, "./tfile1_b1.saf")<=0) SS_FAILED_WHEN("ss_file_isopen() on related name");
        if (ss_file_isopen(file1, "tfile1_b1.saf")<=0) SS_FAILED_WHEN("ss_file_isopen() file and name");
        if (ss_file_isopen(file1, "./tfile1_b1.saf")<=0) SS_FAILED_WHEN("ss_file_isopen() file and related name");

        /* Test that additional files are not reported as open */
        if (ss_file_isopen(NULL, "tfile1_b2.saf")!=0) SS_FAILED_WHEN("ss_file_isopen() on wrong name");
        if (ss_file_isopen(file1, "tfile1_b2.saf")!=0) SS_FAILED_WHEN("ss_file_isopen() on file and wrong name");
        
        /* The created file is not transient. */
        if (ss_file_istransient(file1)) SS_FAILED_WHEN("ss_file_istransient() on permanent file");

        /* Close the file */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     File Operations
 * Purpose:     Conflicting file access
 *
 * Description: Test that SSlib prevents the caller from opening the same file multiple times concurrently.
 *
 * Issue:       We should also test ss_file_open() with a non-null first argument (a file object) with no name, same name,
 *              related name, and non-equivalent name.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD.
 *
 * Programmer:  Robb Matzke
 *              Friday, August 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tfile1c(void)
{
    int                 nerr=0;
    ss_file_t           *file1=NULL;
    ss_file_t           *file2=NULL;
    int                 self;
    FILE                *_print=NULL;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("conflicting file access") {
        /* Create an initial file */
        file1 = ss_file_create("tfile1_c1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file");

        /* Try to create a file with the same name (not allowed) */
        H5E_BEGIN_TRY {
            file2 = ss_file_create("tfile1_c1.saf", H5F_ACC_RDWR, NULL);
        } H5E_END_TRY;
        if (file2) SS_FAILED_WHEN("creating same name");

        /* Try to create a file with a related name (not allowed) */
        H5E_BEGIN_TRY {
            file2 = ss_file_create("./tfile1_c1.saf", H5F_ACC_RDWR, NULL);
        } H5E_END_TRY;
        if (file2) SS_FAILED_WHEN("creating related name");

        /* Try to open a file with the same name (not allowed) */
        H5E_BEGIN_TRY {
            file2 = ss_file_open(NULL, "tfile1_c1.saf", H5F_ACC_RDWR, NULL);
        } H5E_END_TRY;
        if (file2) SS_FAILED_WHEN("opening same name");
        
        /* Try to open a file with a related name (not allowed) */
        H5E_BEGIN_TRY {
            file2 = ss_file_open(NULL, "./tfile1_c1.saf", H5F_ACC_RDWR, NULL);
        } H5E_END_TRY;
        if (file2) SS_FAILED_WHEN("opening related name");

        /* Try to open the file for read-only access with the same name (not allowed) */
        H5E_BEGIN_TRY {
            file2 = ss_file_open(NULL, "tfile1_c1.saf", H5F_ACC_RDONLY, NULL);
        } H5E_END_TRY;
        if (file2) SS_FAILED_WHEN("opening related name rdonly");

        /* Close the file */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file");
        file1 = NULL;
        
    } SS_END_CHECKING_WITH(nerr++);
    return nerr++;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     File Operations
 * Purpose:     Transient files
 *
 * Description: Tests various basic operations (open, close, predicates) on transient files.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD.
 *
 * Programmer:  Robb Matzke
 *              Friday, August 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tfile1d(void)
{
    int                 nerr=0;
    ss_file_t           *file1=NULL;
    ss_file_t           *file2=NULL;
    int                 self;
    FILE                *_print=NULL;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("basic transient file operations") {
        /* Create a transient file */
        file1 = ss_file_create("tfile1_d1.saf", H5F_ACC_TRANSIENT, NULL);
        if (!file1) SS_FAILED_WHEN("creating file");

        /* A transient file cannot be opened twice with the same name or related name */
        H5E_BEGIN_TRY {
            file2 = ss_file_open(NULL, "./tfile1_d1.saf", H5F_ACC_TRANSIENT, NULL);
        } H5E_END_TRY;
        if (file2) SS_FAILED_WHEN("opening related transient name");

        /* A normal file cannot be opened if it has the same name as a transient file */
        H5E_BEGIN_TRY {
            file2 = ss_file_open(NULL, "././tfile1_d1.saf", H5F_ACC_RDONLY, NULL);
        } H5E_END_TRY;
        if (file2) SS_FAILED_WHEN("opening related file");

        /* The ss_file_isopen() predicate should return 1 for a transient file */
        if (1!=ss_file_isopen(file1, NULL)) SS_FAILED_WHEN("ss_file_isopen() on file object");
        if (1!=ss_file_isopen(NULL, "./tfile1_d1.saf")) SS_FAILED_WHEN("ss_file_isopen() on related name");

        /* The file should be reported as being transient */
        if (ss_file_istransient(file1)<=0) SS_FAILED_WHEN("ss_file_istransient");
        
        /* Close the transient file */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file");
        file1 = NULL;
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     File Operations
 * Purpose:     Recreating a file
 *
 * Description: What happens if SSlib opens a file, reads and/or modifies part of it, closes it, then clobbers it by creating
 *              a new file with the same name?
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Monday, April 12, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tfile1e(void)
{
    int                 nerr=0;
    ss_file_t           *file1=NULL;
    ss_scope_t          *tscope1=NULL;
    int                 self, i;
    FILE                *_print=NULL;
    ss_role_t           *r[10];
    ss_roleobj_t        *roleobj;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
#if 1
    _print = 0==self ? stderr : NULL;
#else
    _print = stderr;
#endif

    SS_CHECKING("recreating a file") {
        /* Create a file */
        file1 = ss_file_create("tfile1_e1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope");

        /* Create some objects */
        for (i=0; i<10; i++) {
            r[i] = SS_PERS_NEW(tscope1, ss_role_t, SS_ALLSAME);
            if (!r[i]) SS_FAILED_WHEN("creating role");
        }

        /* Close the file */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file");
        file1 = NULL;

        /* Recreate the file */
        file1 = ss_file_create("tfile1_e1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("recreating file");

        /* Try to dereference the previous objects */
        for (i=0; i<10; i++) {
            H5E_BEGIN_TRY {
                roleobj = SS_ROLE(r[i]);
            } H5E_END_TRY;
            if (roleobj) SS_FAILED_WHEN("role deref should have failed");
        }
        
        /* Close the file again */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("reclosing file");
        file1 = NULL;
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

    /* Tests */
    nerr += tfile1a();          /* basic file creaton */
    nerr += tfile1b();          /* file predicates */
    nerr += tfile1c();          /* conflicting file access */
    nerr += tfile1d();          /* transient files */
    nerr += tfile1e();          /* recreating a file */
    
    /* Finalization */
    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return nerr?1:0;
}
