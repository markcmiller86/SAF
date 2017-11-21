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
 * Chapter:     Copying
 * Purpose:     Copy object from one file to another.
 *
 * Description: Create objects in one file and copy them to the other file.
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
tcopy1a(void)
{
    int                 nerr=0, self;
    ss_file_t           *file1=NULL, *file2=NULL, *kmfile=NULL;
    ss_scope_t          *tscope1=NULL, *tscope2=NULL, *keymask=NULL;
    FILE                *_print=NULL;
    int                 i, attrout=500;
    ss_quantity_t       *q1=NULL;
    ss_unit_t           *u1=NULL, *u2=NULL;
    ss_attr_t           *a1=NULL, *a2=NULL;
    size_t              nfound;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("inter-file object copy") {
        /* Create new files for this test */
        file1 = ss_file_create("tcopy1_a1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");
        
        file2 = ss_file_create("tcopy1_a2.saf", H5F_ACC_RDWR, NULL);
        if (!file2) SS_FAILED_WHEN("creating file 2");
        tscope2 = ss_file_topscope(file2, NULL);
        if (!tscope2) SS_FAILED_WHEN("obtaining top scope 2");
        
        kmfile = ss_file_create("tcopy1_a3.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* All tasks create a quantity in file1 */
        q1 = SS_PERS_NEW(tscope1, ss_quantity_t, SS_ALLSAME);
        if (!q1) SS_FAILED_WHEN("creating quantity");
        ss_string_set(SS_QUANTITY_P(q1,name), "quantity-1");
        ss_string_set(SS_QUANTITY_P(q1,url), "http://saf.llnl.gov/SAF/Registry/Testing/tcopy1/quantity-1.html");
        ss_string_set(SS_QUANTITY_P(q1,abbr), "q1");
        SS_QUANTITY(q1)->flags = 911;
        for (i=0; i<7; i++) SS_QUANTITY(q1)->power[i] = i;

        /* All tasks create a unit in file1 */
        u1 = SS_PERS_NEW(tscope1, ss_unit_t, SS_ALLSAME);
        if (!u1) SS_FAILED_WHEN("creating unit");
        ss_string_set(SS_UNIT_P(u1,name), "unit-1");
        ss_string_set(SS_UNIT_P(u1,url), "http://saf.llnl.gov/SAF/Registry/Testing/tcopy1/unit-1.html");
        ss_string_set(SS_UNIT_P(u1,abbr), "u1");
        SS_UNIT(u1)->scale = 1.1;
        SS_UNIT(u1)->offset = 2.2;
        SS_UNIT(u1)->logbase = 3.3;
        SS_UNIT(u1)->logcoef = 4.4;
        SS_UNIT(u1)->quant = *q1;

        /* All tasks add an attribute to the unit */
        a1 = ss_attr_new((ss_pers_t*)u1, "a1", H5T_NATIVE_INT, 1, &attrout, SS_ALLSAME, NULL, NULL);
        if (!a1) SS_FAILED_WHEN("creating attribute");

        /* All tasks copy the original unit to the second file */
        u2 = (ss_unit_t*)ss_pers_copy((ss_pers_t*)u1, tscope2, SS_ALLSAME, NULL, NULL);
        if (!u2) SS_FAILED_WHEN("copying unit");

        /* The attribute should *NOT* have been copied */
        nfound = ss_attr_count((ss_pers_t*)u2, NULL);
        if (SS_NOSIZE==nfound) SS_FAILED_WHEN("ss_attr_count failed");
        if (nfound!=0) SS_FAILED_WHEN("attribute should not have been copied");

        /* Now explicitly copy the attribute. It only makes sense to copy it into the same scope as it was. */
        a2 = (ss_attr_t*)ss_pers_copy((ss_pers_t*)a1, tscope1, SS_ALLSAME, NULL, NULL);
        if (!a2) SS_FAILED_WHEN("copying attribute");

        /* Close the files */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        if (ss_file_close(file2)<0) SS_FAILED_WHEN("closing file 2");
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
    nerr += tcopy1a();

    /* Finalization */
    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return nerr?1:0;
}
