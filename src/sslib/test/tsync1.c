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
 * Chapter:     Synchronizing
 * Purpose:     Single new object
 *
 * Description: Task zero creates an object, the scope is synchronized, and some other task queries the object.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1a(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_unit_t           *obj=NULL, *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("single new object") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_a1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_a2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* Task zero creates an object */
        if (0==self) {
            obj = SS_PERS_NEW(tscope, ss_unit_t, 0);
            if (!obj) SS_FAILED_WHEN("creating object");
            SS_UNIT(obj)->offset = 100.0;
        }

        /* All tasks synchronize */
        if (ss_scope_synchronize(tscope, SS_TABLE_ALL, NULL)<0) SS_FAILED_WHEN("synchronizing");

        /* Task one (or zero) queries the object */
        key = SS_PERS_NEW(keymask, ss_unit_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? (int)(SS_UNIT(found)->offset+0.5) : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected nfound==1");
        if (100!=exchange[2]) SS_FAILED_WHEN("expected value==100");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Identical new objects
 *
 * Description: Task zero creates three quantity objects that are all identical and then synchronizes. The result is that the
 *              three objects are merged into a single object but all three links are still valid.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1b(void)
{
    int                 nerr=0, self, ntasks, root, i;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_quantity_t       *q[3], *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("identical new objects") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_b1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_b2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* Task zero creates three identical objects */
        if (0==self) {
            for (i=0; i<3; i++) {
                q[i] = SS_PERS_NEW(tscope, ss_quantity_t, 0);
                if (!q[i]) SS_FAILED_WHEN("creating quantity");
                SS_QUANTITY(q[i])->flags = 100;
            }
        }

        /* All tasks synchronize. If you turn on sync broadcast debugging you should see just one object transmitted.
         * You can do that with the following addition to the SSLIB_DEBUG environment variable: check=sync=bcast */
        if (ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL)<0) SS_FAILED_WHEN("synchronizing quantity scope");

        /* Task zero checks that the three links point to the same object now */
        if (0==self) {
            if (!SS_PERS_EQ(q[0], q[1])) SS_FAILED_WHEN("q[0] != q[1]");
            if (!SS_PERS_EQ(q[1], q[2])) SS_FAILED_WHEN("q[1] != q[2]");
        }

        /* Task one (or zero) queries the quantities. There should be just one */
        key = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? SS_QUANTITY(found)->flags : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected nfound==1");
        if (100!=exchange[2]) SS_FAILED_WHEN("expected flags==100")
        
        /* Close the files */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Single new object same as existing object
 *
 * Description: All tasks collectively create an object, then task zero creates a single new object with the same value as the
 *              existing object, the scope is synchronized, and some other task queries the object. The result should be two
 *              objects in the table both with the same value.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1c(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_quantity_t       *q[2], *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("existing new object") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_c1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_c2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* All tasks create an object */
        q[0] = SS_PERS_NEW(tscope, ss_quantity_t, SS_ALLSAME);
        if (!q[0]) SS_FAILED_WHEN("creating collective quantity");
        SS_QUANTITY(q[0])->flags = 100;

        /* Task zero creates an object */
        if (0==self) {
            q[1] = SS_PERS_NEW(tscope, ss_quantity_t, 0);
            if (!q[1]) SS_FAILED_WHEN("creating independent quantity");
            SS_QUANTITY(q[1])->flags = 100;
        } else {
            q[1] = NULL;
        }

        /* All tasks synchronize */
        if (ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL)<0) SS_FAILED_WHEN("synchronizing quantity scope");

        /* Task one (or zero) queries the object */
        key = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? SS_QUANTITY(found+1)->flags : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (2!=exchange[1]) SS_FAILED_WHEN("expected nfound==2");
        if (100!=exchange[2]) SS_FAILED_WHEN("expected flags==100");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Independent modify
 *
 * Description: All tasks collectively create an object, task zero modifies that object, the scope is synchronized, and some
 *              other task queries the object.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1d(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_quantity_t       *q=NULL, *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("single object modification") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_d1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_d2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* All tasks create an object */
        q = SS_PERS_NEW(tscope, ss_quantity_t, SS_ALLSAME);
        if (!q) SS_FAILED_WHEN("creating collective quantity");
        SS_QUANTITY(q)->flags = 100;

        /* Task zero modifies the object */
        if (0==self) {
            SS_QUANTITY(q)->flags = 101;
            SS_PERS_MODIFIED(q, 0);
        }

        /* All tasks synchronize */
        if (ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL)<0) SS_FAILED_WHEN("synchronizing quantity scope");

        /* Task one (or zero) queries the object */
        key = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? SS_QUANTITY(found)->flags : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected nfound==1");
        if (101!=exchange[2]) SS_FAILED_WHEN("expected flags==101");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Independent modify w/o marking
 *
 * Description: All tasks collectively create an object, task zero modifies that object but does not mark it as modified, the
 *              scope is synchronized, and some other task queries the object.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Issue:       This test is only expected to succeed if the global sync_check flag is set to TRUE, but we can temporarily
 *              fool the library into thinking it was.
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1e(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_quantity_t       *q=NULL, *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3];
    hbool_t             old_sync_check=sslib_g.sync_check;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    /* In order for this test to work we have to temporarily turn on sync checking. This could be a bad thing to do because
     * the setting is intended to be constant for the life of the executable. */
    sslib_g.sync_check = TRUE;

    SS_CHECKING("failure to mark modified") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_e1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_e2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* All tasks create an object */
        q = SS_PERS_NEW(tscope, ss_quantity_t, SS_ALLSAME);
        if (!q) SS_FAILED_WHEN("creating collective quantity");
        SS_QUANTITY(q)->flags = 100;

        /* We must synchronize or else SSlib has no chance of catching the fact that we forgot to call SS_PERS_MODIFIED(). If
         * we don't sync it will instead look like the tasks provided inconsistent data when they promised they provided the
         * same data. It would be nice to be able to catch this case without the extra synchronization.... */
        if (ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL)<0) SS_FAILED_WHEN("originally syncing");

        /* Task zero modifies the object */
        if (0==self) {
            SS_QUANTITY(q)->flags = 101;
            /* SS_PERS_MODIFIED(q, 0); -- do not call for this test */
        }

        /* All tasks synchronize */
        if (_print)
            fprintf(_print, "NOTE: an \"unsynchronized object marked as synchronized\" warning may follow this line. "
                    "It can be ignored.\n");
        if (ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL)<0) SS_FAILED_WHEN("synchronizing quantity scope");

        /* Task one (or zero) queries the object */
        key = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? SS_QUANTITY(found)->flags : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected nfound==1");
        if (101!=exchange[2]) SS_FAILED_WHEN("expected flags==101");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);

    sslib_g.sync_check = old_sync_check;
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Extraneous marking
 *
 * Description: All tasks collectively create an object, task zero marks the object as modified but does not actually modify
 *              it, the scope is synchronized, and some other task queries the object.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1f(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_quantity_t       *q=NULL, *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("extraneous marking") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_f1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_f2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* All tasks create an object */
        q = SS_PERS_NEW(tscope, ss_quantity_t, SS_ALLSAME);
        if (!q) SS_FAILED_WHEN("creating collective quantity");
        SS_QUANTITY(q)->flags = 100;

        /* Task marks the object as modified */
        if (0==self) SS_PERS_MODIFIED(q, 0);

        /* All tasks synchronize */
        if (ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL)<0) SS_FAILED_WHEN("synchronizing quantity scope");

        /* Task one (or zero) queries the object */
        key = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? SS_QUANTITY(found)->flags : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected nfound==1");
        if (100!=exchange[2]) SS_FAILED_WHEN("expected flags==100");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     All independent modify
 *
 * Description: All tasks collectively create an object, all tasks modify the object identically but independently, the scope
 *              is synchronized, and some other task queries the object.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1g(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_quantity_t       *q=NULL, *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("all-tasks independent modification") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_g1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_g2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* All tasks create an object */
        q = SS_PERS_NEW(tscope, ss_quantity_t, SS_ALLSAME);
        if (!q) SS_FAILED_WHEN("creating collective quantity");
        SS_QUANTITY(q)->flags = 100;

        /* All tasks modify the object */
        SS_QUANTITY(q)->flags = 101;
        SS_PERS_MODIFIED(q, 0);

        /* All tasks synchronize */
        if (ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL)<0) SS_FAILED_WHEN("synchronizing quantity scope");

        /* Task one (or zero) queries the object */
        key = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? SS_QUANTITY(found)->flags : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected nfound==1");
        if (101!=exchange[2]) SS_FAILED_WHEN("expected flags==101");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Inconsistent independent modify
 *
 * Description: All tasks collectively create an object, all tasks modify the object in different ways, the scope
 *              is synchronized, and some other task queries the object.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1h(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_quantity_t       *q=NULL, *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3], expected;
    herr_t              status;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("inconsistent modification") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_h1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_h2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* All tasks create an object */
        q = SS_PERS_NEW(tscope, ss_quantity_t, SS_ALLSAME);
        if (!q) SS_FAILED_WHEN("creating collective quantity");
        SS_QUANTITY(q)->flags = 100;

        /* All tasks modify the object */
        SS_QUANTITY(q)->flags = 101 + self;
        SS_PERS_MODIFIED(q, 0);

        /* All tasks synchronize */
        if (_print) {
            if (1==ntasks) fprintf(_print, "NOTE: this test is pretty much a no-op when run on only one task.\n");
            else fprintf(_print, "NOTE: an \"incompatible changes\" warning may follow this line. It can be ignored.\n");
        }
        H5E_BEGIN_TRY {
            status = ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL);
        } H5E_END_TRY;
        if (ntasks>1 && status>=0) SS_FAILED_WHEN("synchronizing should have failed");

        /* Task one (or zero) queries the object */
        key = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        expected = 101;
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? SS_QUANTITY(found)->flags : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected nfound==1");
        if (expected!=exchange[2]) {
            char expected_s[64];
            sprintf(expected_s, "expected flags==%d", expected);
            SS_FAILED_WHEN(expected_s);
        }
        
        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Object dependencies
 *
 * Description: Task zero creates two objects, one of which links to the other, then all tasks synchronize.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1i(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_unit_t           *u=NULL, *ukey=NULL;
    ss_quantity_t       *q=NULL, *qkey=NULL;
    ss_pers_t           *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[6];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("object dependencies") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_i1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_i2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* Task zero creates a quantity and a unit */
        if (0==self) {
            q = SS_PERS_NEW(tscope, ss_quantity_t, 0);
            if (!q) SS_FAILED_WHEN("creating quantity");
            SS_QUANTITY(q)->flags = 100;
            u = SS_PERS_NEW(tscope, ss_unit_t, 0);
            if (!u) SS_FAILED_WHEN("creating unit");
            SS_UNIT(u)->offset = 101.0;
            SS_UNIT(u)->quant = *q;
        }

        /* All tasks synchronize */
        if (ss_scope_synchronize(tscope, SS_TABLE_ALL, NULL)<0) SS_FAILED_WHEN("synchronizing");

        /* Task one (or zero) queries the objects */
        root = (1==ntasks?0:1);
        qkey = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        ukey = SS_PERS_NEW(keymask, ss_unit_t, SS_ALLSAME);
        if (self==root) {
            found = SS_PERS_FIND(tscope, qkey, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? (int)SS_QUANTITY(found)->flags : 0;

            found = SS_PERS_FIND(tscope, ukey, NULL, SS_NOSIZE, nfound);
            exchange[3] = found ? 1 : 0;
            exchange[4] = (int)nfound;
            exchange[5] = nfound>=1 ? (int)(SS_UNIT(found)->offset+0.5) : 0;
        }
        ss_mpi_bcast(exchange, 6, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check quantity results */
        if (!exchange[0]) SS_FAILED_WHEN("find quant op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected quant nfound==1");
        if (100!=exchange[2]) SS_FAILED_WHEN("expected quant flags==100");
        
        if (!exchange[3]) SS_FAILED_WHEN("find unit op returned null");
        if (1!=exchange[4]) SS_FAILED_WHEN("expected unit nfound==1");
        if (101!=exchange[5]) SS_FAILED_WHEN("expected unit offset==101");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Dangling object dependencies
 *
 * Description: Task zero creates two objects, one (a unit) of which links to the other (a quantity), then all tasks
 *              synchronize. However, they try to synchronize the unit before the quantity and therefore fail because they
 *              can't possibly figure out if all tasks' units are really pointing to the same quantity and therefore cannot
 *              determine if there is one unit or several units.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1j(void)
{
    int                 nerr=0, self, ntasks, i;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_unit_t           *u=NULL, *ukey=NULL;
    ss_quantity_t       *q=NULL, *qkey=NULL;
    ss_pers_t           *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 *exchange=NULL;
    herr_t              status;
    ss_prop_t           *syncprops=NULL;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("object dangling dependencies") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_j1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_j2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* Task zero creates a quantity and a unit */
        if (0==self) {
            q = SS_PERS_NEW(tscope, ss_quantity_t, 0);
            if (!q) SS_FAILED_WHEN("creating quantity");
            SS_QUANTITY(q)->flags = 100;
            u = SS_PERS_NEW(tscope, ss_unit_t, 0);
            if (!u) SS_FAILED_WHEN("creating unit");
            SS_UNIT(u)->offset = 101.0;
            SS_UNIT(u)->quant = *q;
        }

        /* Synchronization properties */
        syncprops = ss_prop_new("sync");
        ss_prop_add(syncprops, "err_newptrs", H5T_NATIVE_INT, NULL);

        /* All tasks attempt to synchronize the unit before the quantity */
        H5E_BEGIN_TRY {
            status = ss_scope_synchronize(tscope, SS_MAGIC(ss_unit_t), syncprops);
        } H5E_END_TRY;
        if (status>=0) SS_FAILED_WHEN("synchronize units should have failed");
        if (1!=ss_prop_get_i(syncprops, "err_newptrs")) SS_FAILED_WHEN("expected err_newptrs==1");

        /* The first synchronization should have given the unit a permanent home. This second synchronization should be
         * a no-op since nothing more has changed. */
        ss_prop_set_i(syncprops, "err_newptrs", 0);
        H5E_BEGIN_TRY {
            status = ss_scope_synchronize(tscope, SS_MAGIC(ss_unit_t), syncprops);
        } H5E_END_TRY;
        if (status>=0) SS_FAILED_WHEN("synchronize units #2 should have failed");
        if (1!=ss_prop_get_i(syncprops, "err_newptrs")) SS_FAILED_WHEN("expected err_newptrs==1");

        /* All tasks query the objects */
        exchange = calloc((size_t)6*ntasks, sizeof(*exchange));
        assert(exchange);
        qkey = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        ukey = SS_PERS_NEW(keymask, ss_unit_t, SS_ALLSAME);

        found = SS_PERS_FIND(tscope, qkey, NULL, SS_NOSIZE, nfound);
        exchange[self*6+0] = found ? 1 : 0;
        exchange[self*6+1] = (int)nfound;
        exchange[self*6+2] = nfound>=1 ? (int)SS_QUANTITY(found)->flags : 0;

        found = SS_PERS_FIND(tscope, ukey, NULL, SS_NOSIZE, nfound);
        exchange[self*6+3] = found ? 1 : 0;
        exchange[self*6+4] = (int)nfound;
        exchange[self*6+5] = nfound>=1 ? (int)(SS_UNIT(found)->offset+0.5) : 0;

        /* Task zero should be the only one able to query the quantity. All tasks will have the unit but only task zero will
         * have the current value for it. */   
        ss_mpi_allgather(exchange, 6, MPI_INT, SS_COMM_WORLD);
        if (!exchange[0]) SS_FAILED_WHEN("task 0 find quant op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("task 0 expected quant nfound==1");
        if (100!=exchange[2]) SS_FAILED_WHEN("task 0 expected quant flags==100");
        
        if (!exchange[3]) SS_FAILED_WHEN("task 0 find unit op returned null");
        if (1!=exchange[4]) SS_FAILED_WHEN("task 0 expected unit nfound==1");
        if (101!=exchange[5]) SS_FAILED_WHEN("task 0 expected unit offset==101");
        
        for (i=1; i<ntasks; i++) {
            if (!exchange[i*6+0]) SS_FAILED_WHEN("non-root task find quant op returned null");
            if (0!=exchange[i*6+1]) SS_FAILED_WHEN("non-root task expected quant nfound==0");
            if (!exchange[i*6+3]) SS_FAILED_WHEN("non-root task find unit op returned null");
            if (1!=exchange[i*6+4]) SS_FAILED_WHEN("non-root task expected unit nfound==1");
        }

        /* Synchronize the quantity, then the unit */
        status = ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL);
        if (status<0) SS_FAILED_WHEN("synchronize quantities");
        status = ss_scope_synchronize(tscope, SS_MAGIC(ss_unit_t), NULL);
        if (status<0) SS_FAILED_WHEN("synchronize units #3 failed");

        /* All tasks should have correct data for the quantity and unit now. */
        found = SS_PERS_FIND(tscope, qkey, NULL, SS_NOSIZE, nfound);
        exchange[self*6+0] = found ? 1 : 0;
        exchange[self*6+1] = (int)nfound;
        exchange[self*6+2] = nfound>=1 ? (int)SS_QUANTITY(found)->flags : 0;

        found = SS_PERS_FIND(tscope, ukey, NULL, SS_NOSIZE, nfound);
        exchange[self*6+3] = found ? 1 : 0;
        exchange[self*6+4] = (int)nfound;
        exchange[self*6+5] = nfound>=1 ? (int)(SS_UNIT(found)->offset+0.5) : 0;

        ss_mpi_allgather(exchange, 6, MPI_INT, SS_COMM_WORLD);
        for (i=1; i<ntasks; i++) {
            if (!exchange[i*6+0]) SS_FAILED_WHEN("find quant op returned null");
            if (1!=exchange[i*6+1]) SS_FAILED_WHEN("expected quant nfound==1");
            if (100!=exchange[i*6+2]) SS_FAILED_WHEN("expected quant flags==100");
            if (!exchange[i*6+3]) SS_FAILED_WHEN("find unit op returned null");
            if (1!=exchange[i*6+4]) SS_FAILED_WHEN("expected unit nfound==1");
            if (101!=exchange[i*6+5]) SS_FAILED_WHEN("expected unit offset==101");
        }

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Inter-file dependencies
 *
 * Description: Task zero creates a quantity in file A and a unit in file B and then all tasks synchronize. The net effect
 *              is that the unit in file B will point to the quantity in file A.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1k(void)
{
    int                 nerr=0, self, ntasks, root;
    ss_file_t           *file1=NULL, *file2=NULL, *kmfile=NULL;
    ss_scope_t          *tscope1=NULL, *tscope2=NULL, *keymask=NULL;
    ss_unit_t           *u=NULL, *ukey=NULL;
    ss_quantity_t       *q=NULL, *qkey=NULL;
    ss_pers_t           *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[6];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("object inter-file dependencies") {
        /* Create new files for this test */
        file1 = ss_file_create("tsync1_k1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");

        file2 = ss_file_create("tsync1_k2.saf", H5F_ACC_RDWR, NULL);
        if (!file2) SS_FAILED_WHEN("creating file 2");
        tscope2 = ss_file_topscope(file2, NULL);
        if (!tscope2) SS_FAILED_WHEN("obtaining top scope 2");

        kmfile = ss_file_create("tsync1_k3.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* Task zero creates a quantity and a unit */
        if (0==self) {
            q = SS_PERS_NEW(tscope1, ss_quantity_t, 0);
            if (!q) SS_FAILED_WHEN("creating quantity");
            SS_QUANTITY(q)->flags = 100;

            u = SS_PERS_NEW(tscope2, ss_unit_t, 0);
            if (!u) SS_FAILED_WHEN("creating unit");
            SS_UNIT(u)->offset = 101.0;
            SS_UNIT(u)->quant = *q;
        }

        /* All tasks synchronize */
        if (ss_scope_synchronize(tscope1, SS_TABLE_ALL, NULL)<0) SS_FAILED_WHEN("synchronizing 1");
        if (ss_scope_synchronize(tscope2, SS_TABLE_ALL, NULL)<0) SS_FAILED_WHEN("synchronizing 2");

        /* Task one (or zero) queries the objects */
        root = (1==ntasks?0:1);
        qkey = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        ukey = SS_PERS_NEW(keymask, ss_unit_t, SS_ALLSAME);
        if (self==root) {
            found = SS_PERS_FIND(tscope1, qkey, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? (int)SS_QUANTITY(found)->flags : 0;

            found = SS_PERS_FIND(tscope2, ukey, NULL, SS_NOSIZE, nfound);
            exchange[3] = found ? 1 : 0;
            exchange[4] = (int)nfound;
            exchange[5] = nfound>=1 ? (int)(SS_UNIT(found)->offset+0.5) : 0;
        }
        ss_mpi_bcast(exchange, 6, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check quantity results */
        if (!exchange[0]) SS_FAILED_WHEN("find quant op returned null");
        if (1!=exchange[1]) SS_FAILED_WHEN("expected quant nfound==1");
        if (100!=exchange[2]) SS_FAILED_WHEN("expected quant flags==100");
        
        if (!exchange[3]) SS_FAILED_WHEN("find unit op returned null");
        if (1!=exchange[4]) SS_FAILED_WHEN("expected unit nfound==1");
        if (101!=exchange[5]) SS_FAILED_WHEN("expected unit offset==101");

        /* This test also checks the following:
         *   The unit in file2 points to a new quantity in file1. When ss_file_close() is called for file1 the quantity is
         *   synchronized, giving the quantity a permanent home in the table and then blowing away most of the data structures
         *   for file1. When file2 is closed its unit must be synchronized, but the unit contains a link to a quantity and the
         *   link is with an indirect index. Indirect indices are local to a task and therefore in order to synchronize we
         *   must be able to convert the indirect index to a direct index way down in the guts of SSlib even though the
         *   quantity's file has been closed.
         * There are two ways to work around this dilemma: (1) explicitly synchronize all files, then close all files and
         * (2) maintain enough information in the library after each file is closed so that links in other files can be
         * converted from indirect to direct. SSlib takes the second approach, which is a little more wasteful of memory but
         * certainly makes programming easier. */
#if 0
        /* The old method: synchronize then flush then close. It isn't sufficient to just synchronize because even
         * synchronized objects (at least those "born synchronized" with the SS_ALLSAME flag to SS_PERS_NEW()) can point to
         * another object indirectly. */
        if (ss_file_synchronize(file1, NULL)<0) SS_FAILED_WHEN("flushing file 1");
        if (ss_file_synchronize(file2, NULL)<0) SS_FAILED_WHEN("flushing file 2");
        if (ss_file_synchronize(kmfile, NULL)<0) SS_FAILED_WHEN("flushing keymask file");

        if (ss_file_flush(file1, NULL)<0) SS_FAILED_WHEN("flushing file 1");
        if (ss_file_flush(file2, NULL)<0) SS_FAILED_WHEN("flushing file 2");
        if (ss_file_flush(kmfile, NULL)<0) SS_FAILED_WHEN("flushing keymask file");
#endif

        /* Close the file */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        if (ss_file_close(file2)<0) SS_FAILED_WHEN("closing file 2");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Synchronizing
 * Purpose:     Identical new objects
 *
 * Description: Task zero creates three quantity objects that are all identical, then uses ss_pers_unique() to make them unique,
 *              and then synchronizes. The result is that there will be three identical objects after synchronizing.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tsync1l(void)
{
    int                 nerr=0, self, ntasks, root, i;
    ss_file_t           *file=NULL, *kmfile=NULL;
    ss_scope_t          *tscope=NULL, *keymask=NULL;
    ss_quantity_t       *q[3], *key=NULL, *found=NULL;
    FILE                *_print=NULL;
    size_t              nfound;
    int                 exchange[3];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("identical new objects marked unique") {
        /* Create new files for this test */
        file = ss_file_create("tsync1_l1.saf", H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");
        kmfile = ss_file_create("tsync1_l2.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* Task zero creates three identical objects */
        if (0==self) {
            for (i=0; i<3; i++) {
                q[i] = SS_PERS_NEW(tscope, ss_quantity_t, 0);
                if (!q[i]) SS_FAILED_WHEN("creating quantity");
                SS_QUANTITY(q[i])->flags = 100;
                SS_PERS_UNIQUE(q[i]);
            }
        }

        /* All tasks synchronize. If you turn on sync broadcast debugging you should see three objects transmitted.
         * You can do that with the following addition to the SSLIB_DEBUG environment variable: check=sync=bcast */
        if (ss_scope_synchronize(tscope, SS_MAGIC(ss_quantity_t), NULL)<0) SS_FAILED_WHEN("synchronizing quantity scope");

        /* Task zero checks that the three links point to three objects now */
        if (0==self) {
            if (SS_PERS_EQ(q[0], q[1])) SS_FAILED_WHEN("q[0] == q[1]");
            if (SS_PERS_EQ(q[1], q[2])) SS_FAILED_WHEN("q[1] == q[2]");
            if (SS_PERS_EQ(q[0], q[2])) SS_FAILED_WHEN("q[0] == q[2]");
        }

        /* Task one (or zero) queries the quantities. There should be three */
        key = SS_PERS_NEW(keymask, ss_quantity_t, SS_ALLSAME);
        root = (1==ntasks?0:1);
        if (self==root) {
            found = SS_PERS_FIND(tscope, key, NULL, SS_NOSIZE, nfound);
            exchange[0] = found ? 1 : 0;
            exchange[1] = (int)nfound;
            exchange[2] = nfound>=1 ? SS_QUANTITY(found)->flags : 0;
        }
        ss_mpi_bcast(exchange, 3, MPI_INT, root, SS_COMM_WORLD);

        /* All tasks check results */
        if (!exchange[0]) SS_FAILED_WHEN("find op returned null");
        if (3!=exchange[1]) SS_FAILED_WHEN("expected nfound==1");
        if (100!=exchange[2]) SS_FAILED_WHEN("expected flags==100")
        
        /* Close the files */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
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
    nerr += tsync1a();          /* single new object */
    nerr += tsync1b();          /* identical new objects */
    nerr += tsync1l();          /* identical new objects marked as unique */
    nerr += tsync1c();          /* single new object same as existing object */
    nerr += tsync1d();          /* modify single object */
    nerr += tsync1f();          /* extraneous object-modified setting */
    nerr += tsync1g();          /* all-task independent but identical modifications */
    nerr += tsync1i();          /* dependencies from one object to another */
    nerr += tsync1k();          /* stored reference to second file */

    /* Problems detected at runtime */
    nerr += tsync1e();          /* modify but fail to mark single object */
    nerr += tsync1j();          /* object depends on some other unsynchronized object */
    /* ss_pers_new(SS_ALLSAME) with differing data -- detectable if we sync all dirty things(?) */
    /* ss_pers_modified(SS_ALLSAME) with differeing data --detectable if we sync all dirty things(?) */

    /* Exceptional cases that raise errors */
    nerr += tsync1h();          /* inconsistent modifications */

    /* Exceptional cases that lock things up -- uncomment the one you want to try */
#if 0
    -- none defined yet --;
#endif

    /* Finalization */
    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return nerr?1:0;
}
