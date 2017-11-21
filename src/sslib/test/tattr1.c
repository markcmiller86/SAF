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
 * Chapter:     Attributes
 * Purpose:     Test datatype serialization
 *
 * Description: Serializes a bunch of different datatypes, then unserializes them, then calls H5Tequal() to see if the
 *              unserialized type is the same as the serialized version.
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
tattr1a(void)
{
    int                 nerr=0, self;
    FILE                *_print=NULL;
    hid_t               t1, t2;
    char                *buf=NULL;
    const char          *rest=NULL;
    size_t              buf_nused, buf_nalloc;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("HDF5 type serialization") {
        if (_print) fprintf(_print, "  H5T_NATIVE_INT\n");
        t1 = H5T_NATIVE_INT;
        buf = H5encode(t1, buf, &buf_nused, &buf_nalloc);
        if (!buf) SS_FAILED_WHEN("encoding");
        t2 = H5decode(buf, &rest);
        if (t2<0) SS_FAILED_WHEN("decoding");
        if ((size_t)(rest-buf)!=buf_nused) SS_FAILED_WHEN("decoded wrong number of bytes");
        if (!H5Tequal(t1, t2)) SS_FAILED_WHEN("decoded type doesn't match");
        buf = SS_FREE(buf);
        if (_print) fprintf(_print, "    %lu bytes\n", (unsigned long)buf_nused);

        if (_print) fprintf(_print, "  H5T_NATIVE_FLOAT\n");
        t1 = H5T_NATIVE_FLOAT;
        buf = H5encode(t1, buf, &buf_nused, &buf_nalloc);
        if (!buf) SS_FAILED_WHEN("encoding");
        t2 = H5decode(buf, &rest);
        if (t2<0) SS_FAILED_WHEN("decoding");
        if ((size_t)(rest-buf)!=buf_nused) SS_FAILED_WHEN("decoded wrong number of bytes");
        if (!H5Tequal(t1, t2)) SS_FAILED_WHEN("decoded type doesn't match");
        buf = SS_FREE(buf);
        if (_print) fprintf(_print, "    %lu bytes\n", (unsigned long)buf_nused);

        if (_print) fprintf(_print, "  ss_string_tm\n");
        t1 = ss_string_tm;
        buf = H5encode(t1, buf, &buf_nused, &buf_nalloc);
        if (!buf) SS_FAILED_WHEN("encoding");
        t2 = H5decode(buf, &rest);
        if (t2<0) SS_FAILED_WHEN("decoding");
        if ((size_t)(rest-buf)!=buf_nused) SS_FAILED_WHEN("decoded wrong number of bytes");
        if (!H5Tequal(t1, t2)) SS_FAILED_WHEN("decoded type doesn't match");
        buf = SS_FREE(buf);
        if (_print) fprintf(_print, "    %lu bytes\n", (unsigned long)buf_nused);

        if (_print) fprintf(_print, "  ss_qauntityobj_tff\n");
        t1 = ss_quantityobj_tff;
        buf = H5encode(t1, buf, &buf_nused, &buf_nalloc);
        if (!buf) SS_FAILED_WHEN("encoding");
        t2 = H5decode(buf, &rest);
        if (t2<0) SS_FAILED_WHEN("decoding");
        if ((size_t)(rest-buf)!=buf_nused) SS_FAILED_WHEN("decoded wrong number of bytes");
        if (!H5Tequal(t1, t2)) SS_FAILED_WHEN("decoded type doesn't match");
        buf = SS_FREE(buf);
        if (_print) fprintf(_print, "    %lu bytes\n", (unsigned long)buf_nused);
        
        if (_print) fprintf(_print, "  ss_unitobj_tfm\n");
        t1 = ss_unitobj_tfm;
        buf = H5encode(t1, buf, &buf_nused, &buf_nalloc);
        if (!buf) SS_FAILED_WHEN("encoding");
        t2 = H5decode(buf, &rest);
        if (t2<0) SS_FAILED_WHEN("decoding");
        if ((size_t)(rest-buf)!=buf_nused) SS_FAILED_WHEN("decoded wrong number of bytes");
        if (!H5Tequal(t1, t2)) SS_FAILED_WHEN("decoded type doesn't match");
        buf = SS_FREE(buf);
        if (_print) fprintf(_print, "    %lu bytes\n", (unsigned long)buf_nused);

    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Attributes
 * Purpose:     Test attribute objects
 *
 * Description: This test makes sure that an ss_attrobj_t works correctly as far as creation, synchronization and so on
 *              without actually using the ss_attr_* interface.
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
tattr1b(void)
{
    int         nerr=0, self, ntasks;
    FILE        *_print=NULL;
    ss_file_t   *file1=NULL, *file2=NULL, *kmfile=NULL;
    ss_scope_t  *tscope1=NULL, *tscope2=NULL, *keymask=NULL;
    ss_attr_t   *attr1=NULL, *attr2=NULL, *key=NULL, *found=NULL;
    ss_attrobj_t mask;
    size_t      nfound;
    int         val1[2] = {111, 222};
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("attribute table operations") {
        /* Create new files for this test */
        file1 = ss_file_create("tattr1_b1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");

        file2 = ss_file_create("tattr1_b2.saf", H5F_ACC_RDWR, NULL);
        if (!file2) SS_FAILED_WHEN("creating file 2");
        tscope2 = ss_file_topscope(file2, NULL);
        if (!tscope2) SS_FAILED_WHEN("obtaining top scope 2");

        kmfile = ss_file_create("tattr1_b3.saf", H5F_ACC_TRANSIENT, NULL);
        if (!kmfile) SS_FAILED_WHEN("creating keymask file");
        keymask = ss_file_topscope(kmfile, NULL);
        if (!keymask) SS_FAILED_WHEN("obtaining keymask scope");

        /* All tasks create two attributes independently: attr1 has the same value for all tasks while attr2 has a different
         * value for each task. */
        attr1 = SS_PERS_NEW(tscope1, ss_attr_t, 0);
        if (!attr1) SS_FAILED_WHEN("creating attr 1");
        ss_string_set(SS_ATTR_P(attr1,name), "attr1");
        SS_ATTR(attr1)->owner = *(ss_pers_t*)file1;
        ss_array_target(SS_ATTR_P(attr1,value), H5T_NATIVE_INT);
        ss_array_resize(SS_ATTR_P(attr1,value), 2);
        ss_array_put(SS_ATTR_P(attr1,value), H5T_NATIVE_INT, (size_t)0, (size_t)2, val1);

        attr2 = SS_PERS_NEW(tscope1, ss_attr_t, 0);
        if (!attr2) SS_FAILED_WHEN("creating attr 2");
        ss_string_set(SS_ATTR_P(attr2,name), "attr2");
        SS_ATTR(attr2)->owner = *(ss_pers_t*)file1;
        ss_array_target(SS_ATTR_P(attr2,value), H5T_NATIVE_INT);
        ss_array_resize(SS_ATTR_P(attr2,value), 1);
        ss_array_put(SS_ATTR_P(attr2,value), H5T_NATIVE_INT, (size_t)0, (size_t)1, &self);

        /* Close the file and then reopen it. I don't think any of the previous tests do the synchronization quite this way. */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        SS_FREE(file1);
        SS_FREE(tscope1);
        file1 = ss_file_open(NULL, "tattr1_b1.saf", H5F_ACC_RDONLY, NULL);
        if (!file1) SS_FAILED_WHEN("opening file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("reobtaining top scope 1");

        /* Obtain a key/mask pair */
        key = SS_PERS_NEW(keymask, ss_attr_t, SS_ALLSAME);
        if (!key) SS_FAILED_WHEN("creating key");
        memset(&mask, 0, sizeof mask);
        
        /* Check for the correct number of attributes */
        nfound = SS_NOSIZE;
        found = (ss_attr_t*)ss_pers_find(tscope1, (ss_pers_t*)key, NULL, 0, &nfound, SS_PERS_TEST, NULL);
        if (SS_PERS_TEST!=found) SS_FAILED_WHEN("find should have returned SS_PERS_TEST");
        if (nfound!=(size_t)1+ntasks) SS_FAILED_WHEN("expected 1+ntasks attributes");

        /* Check that there are the required number of per-task attributes */
        ss_string_set(SS_ATTR_P(key,name), "attr2");
        memset(&(mask.name), SS_VAL_CMP_DFLT, 1);
        nfound = SS_NOSIZE;
        found = (ss_attr_t*)ss_pers_find(tscope1, (ss_pers_t*)key, (ss_persobj_t*)&mask, 0, &nfound, SS_PERS_TEST, NULL);
        if (SS_PERS_TEST!=found) SS_FAILED_WHEN("find should have returned SS_PERS_TEST");
        if (nfound!=(size_t)ntasks) SS_FAILED_WHEN("expected one attribute per task");

        /* Close the files */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        if (ss_file_close(file2)<0) SS_FAILED_WHEN("closing file 2");
        if (ss_file_close(kmfile)<0) SS_FAILED_WHEN("closing keymask file");
        SS_FREE(tscope1);
        SS_FREE(tscope2);
        SS_FREE(attr1);
        SS_FREE(attr2);
        SS_FREE(file1);
        SS_FREE(file2);
        SS_FREE(kmfile);
        SS_FREE(keymask);
        SS_FREE(key);

    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Attributes
 * Purpose:     Test attribute interface
 *
 * Description: This test is for the object attribute interface.
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
tattr1c(void)
{
    int         nerr=0, self, ntasks;
    FILE        *_print=NULL;
    ss_file_t   *file1=NULL;
    ss_scope_t  *tscope1=NULL;
    ss_attr_t   *attr1=NULL, *attr2=NULL;
    size_t      nfound;
    int         val1[2] = {111, 222};
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("attribute API operations") {
        /* Create new files for this test */
        file1 = ss_file_create("tattr1_c1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");

        /* All tasks create two attributes independently: attr1 has the same value for all tasks while attr2 has a different
         * value for each task. */
        attr1 = ss_attr_new((ss_pers_t*)file1, "attr1", H5T_NATIVE_INT, 2, val1, 0, NULL, NULL);
        if (!attr1) SS_FAILED_WHEN("creating attr 1");
        attr2 = ss_attr_new((ss_pers_t*)file1, "attr2", H5T_NATIVE_INT, 1, &self, 0, NULL, NULL);
        if (!attr2) SS_FAILED_WHEN("creating attr 2");

        /* Close the file and then reopen it. I don't think any of the previous tests do the synchronization quite this way. */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        if (tscope1)SS_FREE(tscope1);
        SS_FREE(file1);

        file1 = ss_file_open(NULL, "tattr1_c1.saf", H5F_ACC_RDONLY, NULL);
        if (!file1) SS_FAILED_WHEN("opening file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("reobtaining top scope 1");

        /* Check for the correct number of attributes */
        nfound = ss_attr_count((ss_pers_t*)file1, NULL);
        if (SS_NOSIZE==nfound) SS_FAILED_WHEN("ss_attr_count() failed");
        if (nfound!=(size_t)1+ntasks) SS_FAILED_WHEN("expected 1+ntasks attributes");

        /* Check that there are the required number of per-task attributes */
        nfound = ss_attr_count((ss_pers_t*)file1, "attr2");
        if (SS_NOSIZE==nfound) SS_FAILED_WHEN("ss_attr_count() failed");
        if (nfound!=(size_t)ntasks) SS_FAILED_WHEN("expected one attribute per task");

        /* Close the files */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        SS_FREE(file1);
        SS_FREE(tscope1);
        SS_FREE(attr1);
        SS_FREE(attr2);

    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Attributes
 * Purpose:     Test attribute data reading
 *
 * Description: This test is for reading attribute values.
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
tattr1d(void)
{
    int                 nerr=0, self;
    FILE                *_print=NULL;

    ss_file_t           *file1=NULL;
    ss_scope_t          *tscope1=NULL;
    ss_attr_t           *attr1=NULL, *attr2=NULL;
    int                 attr1_data[2]={111, 222};
    int                 attr2_data[2];

    int                 *i_ret=NULL, i_buf[2];
    unsigned char       *c_ret=NULL, c_buf[2];
    long_long            *l_ret=NULL, l_buf[2];
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("attribute data read operations") {
        /* Create new files for this test */
        file1 = ss_file_create("tattr1_d1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");

        /* All tasks create two attributes independently: attr1 has the same value for all tasks while attr2 has a different
         * value for each task. */
        attr1 = ss_attr_new((ss_pers_t*)file1, "attr1", H5T_NATIVE_INT, 2, attr1_data, 0, NULL, NULL);
        if (!attr1) SS_FAILED_WHEN("creating attr 1");
        attr2_data[0] = self;
        attr2_data[1] = 911; /*unused*/
        attr2 = ss_attr_new((ss_pers_t*)file1, "attr2", H5T_NATIVE_INT, 1, attr2_data, 0, NULL, NULL);
        if (!attr2) SS_FAILED_WHEN("creating attr 2");

        /* Synchronize files */
        if (ss_file_synchronize(file1, NULL)<0) SS_FAILED_WHEN("synchronizing file");

        /*-------------------------
         * Tests for "attr1"....
         *------------------------- */


        /*---*/
        if (_print) fprintf(_print, "  attr1, no conversion, all values, supplied buffer\n");
        memset(i_buf, 0, sizeof i_buf);
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, SS_NOSIZE, i_buf);
        if (i_ret!=i_buf) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr1_data[0] || i_ret[1]!=attr1_data[1]) SS_FAILED_WHEN("obtained incorrect data");

        /*---*/
        if (_print) fprintf(_print, "  attr1, no conversion, all values, no buffer\n");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, SS_NOSIZE, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr1_data[0] || i_ret[1]!=attr1_data[1]) SS_FAILED_WHEN("obtained incorrect data");
        free(i_ret);

        /*---*/
        if (_print) fprintf(_print, "  attr1, no conversion, first value, supplied buffer\n");
        memset(i_buf, 0, sizeof i_buf);
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 1, i_buf);
        if (i_ret!=i_buf) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr1_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        if (i_ret[1]) SS_FAILED_WHEN("clobbered trailing data");

        /*---*/
        if (_print) fprintf(_print, "  attr1, no conversion, second value, no buffer\n");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 1, SS_NOSIZE, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr1_data[1]) SS_FAILED_WHEN("obtained incorrect data");
        free(i_ret);

        /*---*/
        if (_print) fprintf(_print, "  attr1, downsize, all values, supplied buffer\n");
        memset(c_buf, 0, sizeof c_buf);
        c_ret = ss_attr_get(attr1, H5T_NATIVE_UCHAR, 0, 2, c_buf);
        if (c_ret!=c_buf) SS_FAILED_WHEN("ss_attr_get failed");
        if (c_ret[0]!=attr1_data[0] || c_ret[1]!=attr1_data[1]) SS_FAILED_WHEN("obtained incorrect data");

        /*---*/
        if (_print) fprintf(_print, "  attr1, downsize, second value, supplied buffer\n");
        memset(c_buf, 0, sizeof c_buf);
        c_ret = ss_attr_get(attr1, H5T_NATIVE_UCHAR, 1, 1, c_buf);
        if (c_ret!=c_buf) SS_FAILED_WHEN("ss_attr_get failed");
        if (c_ret[0]!=attr1_data[1]) SS_FAILED_WHEN("obtained incorrect data");
        if (c_ret[1]) SS_FAILED_WHEN("clobbered trailing data");

        /*---*/
        if (_print) fprintf(_print, "  attr1, downsize, first value, no buffer\n");
        c_ret = ss_attr_get(attr1, H5T_NATIVE_UCHAR, 0, 1, NULL);
        if (!c_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (c_ret[0]!=attr1_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        free(c_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, upsize, all values, no buffer\n");
        l_ret = ss_attr_get(attr1, H5T_NATIVE_LLONG, 0, 2, NULL);
        if (!l_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (l_ret[0]!=attr1_data[0] || l_ret[1]!=attr1_data[1]) SS_FAILED_WHEN("obtained incorrect data");
        free(l_ret);

        /*---*/
        if (_print) fprintf(_print, "  attr1, upsize, first value, supplied buffer\n");
        memset(l_buf, 0, sizeof l_buf);
        l_ret = ss_attr_get(attr1, H5T_NATIVE_LLONG, 0, 1, l_buf);
        if (l_ret!=l_buf) SS_FAILED_WHEN("ss_attr_get failed");
        if (l_ret[0]!=attr1_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        if (l_ret[1]) SS_FAILED_WHEN("clobbered trailing data");

        /*-------------------------
         * Tests for "attr2"....
         *------------------------- */

        /*---*/
        if (_print) fprintf(_print, "  attr2, no conversion, supplied buffer\n");
        memset(i_buf, 0, sizeof i_buf);
        i_ret = ss_attr_get(attr2, H5T_NATIVE_INT, 0, SS_NOSIZE, i_buf);
        if (i_ret!=i_buf) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr2_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        if (i_buf[1]) SS_FAILED_WHEN("clobbered trailing data");

        /*---*/
        if (_print) fprintf(_print, "  attr2, no conversion, no buffer\n");
        i_ret = ss_attr_get(attr2, H5T_NATIVE_INT, 0, 1, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr2_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        free(i_ret);

        /*---*/
        if (_print) fprintf(_print, "  attr2, downsize, supplied buffer\n");
        memset(c_buf, 0, sizeof c_buf);
        c_ret = ss_attr_get(attr2, H5T_NATIVE_UCHAR, 0, 1, c_buf);
        if (c_ret!=c_buf) SS_FAILED_WHEN("ss_attr_get failed");
        if (c_ret[0]!=attr2_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        if (c_buf[1]) SS_FAILED_WHEN("clobbered trailing data");
        
        /*---*/
        if (_print) fprintf(_print, "  attr2, downsize, no buffer\n");
        c_ret = ss_attr_get(attr2, H5T_NATIVE_UCHAR, 0, SS_NOSIZE, NULL);
        if (!c_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (c_ret[0]!=attr2_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        free(c_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr2, upsize, supplied buffer\n");
        memset(l_buf, 0, sizeof l_buf);
        l_ret = ss_attr_get(attr2, H5T_NATIVE_LLONG, 0, SS_NOSIZE, l_buf);
        if (l_ret!=l_buf) SS_FAILED_WHEN("ss_attr_get failed");
        if (l_ret[0]!=attr2_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        if (l_buf[1]) SS_FAILED_WHEN("clobbered trailing data");
        
        /*---*/
        if (_print) fprintf(_print, "  attr2, upsize, no buffer\n");
        l_ret = ss_attr_get(attr2, H5T_NATIVE_LLONG, 0, 1, NULL);
        if (!l_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (l_ret[0]!=attr2_data[0]) SS_FAILED_WHEN("obtained incorrect data");
        free(l_ret);
        
        /* Close the files */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        SS_FREE(file1);
        SS_FREE(tscope1);
        SS_FREE(attr1);
        SS_FREE(attr2);

    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Attributes
 * Purpose:     Test attribute data reading
 *
 * Description: This test is for modifying attribute values
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
tattr1e(void)
{
    int                 nerr=0, self;
    FILE                *_print=NULL;

    ss_file_t           *file1=NULL;
    ss_scope_t          *tscope1=NULL;
    ss_attr_t           *attr1=NULL;
    int                 val=0;
    int                 attr1_data[2]={111, 222};
    int                 *i_ret=NULL, i_buf[2];
    long_long           l_buf[2];
    unsigned char       c_buf[2];
    herr_t              status;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("attribute data write operations") {
        /* Create new files for this test */
        file1 = ss_file_create("tattr1_e1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");

        /* All tasks create an attribute collectively */
        attr1 = ss_attr_new((ss_pers_t*)file1, "attr1", H5T_NATIVE_INT, 2, attr1_data, SS_ALLSAME, NULL, NULL);
        if (!attr1) SS_FAILED_WHEN("creating attr 1");

        /* Synchronize files */
        if (ss_file_synchronize(file1, NULL)<0) SS_FAILED_WHEN("synchronizing file");

        /*---*/
        if (_print) fprintf(_print, "  attr1, no conversion, all values\n");
        i_buf[0] = val++;
        i_buf[1] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_INT, 0, 2, i_buf, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=i_buf[0] || i_ret[1]!=i_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, no conversion, first value\n");
        i_buf[0] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_INT, 0, 1, i_buf, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=i_buf[0] || i_ret[1]!=i_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, no conversion, second value\n");
        i_buf[1] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_INT, 1, 1, i_buf+1, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=i_buf[0] || i_ret[1]!=i_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, downsize, all values\n");
        l_buf[0] = val++;
        l_buf[1] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_LLONG, 0, 2, l_buf, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=l_buf[0] || i_ret[1]!=l_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, downsize, first value\n");
        l_buf[0] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_LLONG, 0, 1, l_buf, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=l_buf[0] || i_ret[1]!=l_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, downsize, second value\n");
        l_buf[1] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_LLONG, 1, 1, l_buf+1, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=l_buf[0] || i_ret[1]!=l_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, upsize, all values\n");
        c_buf[0] = val++;
        c_buf[1] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_UCHAR, 0, 2, c_buf, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=c_buf[0] || i_ret[1]!=c_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, upsize, first value\n");
        c_buf[0] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_UCHAR, 0, 1, c_buf, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=c_buf[0] || i_ret[1]!=c_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /*---*/
        if (_print) fprintf(_print, "  attr1, upsize, second value\n");
        c_buf[1] = val++;
        status = ss_attr_put(attr1, H5T_NATIVE_UCHAR, 1, 1, c_buf+1, SS_ALLSAME);
        if (status<0) SS_FAILED_WHEN("ss_attr_put failed");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=c_buf[0] || i_ret[1]!=c_buf[1]) SS_FAILED_WHEN("incorrect value read back");
        free(i_ret);
        
        /* Close the files */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        SS_FREE(file1);
        SS_FREE(tscope1);
        SS_FREE(attr1);
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Attributes
 * Purpose:     Test attribute metadata changes
 *
 * Description: This test is for modifying attribute metadata such as datatype and number of elements.
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
tattr1f(void)
{
    int                 nerr=0, self;
    FILE                *_print=NULL;

    ss_file_t           *file1=NULL;
    ss_scope_t          *tscope1=NULL;
    ss_attr_t           *attr1=NULL;
    int                 attr1_data[2]={111, 222};
    int                 *i_ret=NULL;
    unsigned char       *c_ret=NULL;
    void                *v_ret=NULL, *v_zero=NULL;
    hid_t               otype=-1, type=-1;
    size_t              nelmts;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("attribute metadata operations") {
        /* Create new files for this test */
        file1 = ss_file_create("tattr1_f1.saf", H5F_ACC_RDWR, NULL);
        if (!file1) SS_FAILED_WHEN("creating file 1");
        tscope1 = ss_file_topscope(file1, NULL);
        if (!tscope1) SS_FAILED_WHEN("obtaining top scope 1");

        /* All tasks create an attribute collectively */
        attr1 = ss_attr_new((ss_pers_t*)file1, "attr1", H5T_NATIVE_INT, 2, attr1_data, SS_ALLSAME, NULL, NULL);
        if (!attr1) SS_FAILED_WHEN("creating attr 1");

        /* Synchronize files */
        if (ss_file_synchronize(file1, NULL)<0) SS_FAILED_WHEN("synchronize 1");

        /* Change the number of elements from 2 to 4 independently on all tasks and synchronize */
        if (_print) fprintf(_print, "  increasing size from 2 to 4 elements; all tasks independent\n");
        if (ss_attr_modify(attr1, H5I_INVALID_HID, 4, 0)<0) SS_FAILED_WHEN("ss_attr_modify failed");
        if (ss_file_synchronize(file1, NULL)<0) SS_FAILED_WHEN("synchronize 2");
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 4, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr1_data[0] || i_ret[1]!=attr1_data[1]) SS_FAILED_WHEN("original values clobbered");
        if (i_ret[2] || i_ret[3]) SS_FAILED_WHEN("new values are not zero");
        free(i_ret);

        /* Change the number of elements from 4 to 3 collectively */
        if (_print) fprintf(_print, "  decreasing size from 4 to 3 elements\n");
        if (ss_attr_modify(attr1, H5T_NATIVE_INT, 3, SS_ALLSAME)<0) SS_FAILED_WHEN("ss_attr_modify failed");
        if (!ss_attr_describe(attr1, NULL, &type, &nelmts)) SS_FAILED_WHEN("ss_attr_describe failed");
        if (!H5Tequal(type, H5T_NATIVE_INT)) SS_FAILED_WHEN("incorrect datatype read back");
        if (3!=nelmts) SS_FAILED_WHEN("incorrect number of elements read back");
        H5Tclose(type);
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 3, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr1_data[0] || i_ret[1]!=attr1_data[1] || i_ret[2]) SS_FAILED_WHEN("original values clobbered");
        free(i_ret);
        H5E_BEGIN_TRY {
            i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 3, 1, NULL);
        } H5E_END_TRY;
        if (i_ret) SS_FAILED_WHEN("fourth value still present");

        /* Change the datatype from int to short; read back as char */
        if (_print) fprintf(_print, "  changing type from int to short; task zero only\n");
        if (0==self) if (ss_attr_modify(attr1, H5T_NATIVE_SHORT, SS_NOSIZE, 0)<0) SS_FAILED_WHEN("ss_attr_modify failed");
        if (ss_file_synchronize(file1, NULL)<0) SS_FAILED_WHEN("synchronize 3");
        if (!ss_attr_describe(attr1, NULL, &type, &nelmts)) SS_FAILED_WHEN("ss_attr_describe failed");
        if (!H5Tequal(type, H5T_NATIVE_SHORT)) SS_FAILED_WHEN("incorrect datatype read back");
        if (3!=nelmts) SS_FAILED_WHEN("incorrect number of elements read back");
        H5Tclose(type);
        c_ret = ss_attr_get(attr1, H5T_NATIVE_UCHAR, 0, 3, NULL);
        if (!c_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (c_ret[0]!=attr1_data[0] || c_ret[1]!=attr1_data[1] || c_ret[2]) SS_FAILED_WHEN("original values clobbered");
        free(c_ret);

        /* Change the datatype from short to char and drop the third element */
        if (_print) fprintf(_print, "  changing type from short to char; truncating to 2 elements\n");
        if (ss_attr_modify(attr1, H5T_NATIVE_UCHAR, 2, SS_ALLSAME)<0) SS_FAILED_WHEN("ss_attr_modify failed");
        if (!ss_attr_describe(attr1, NULL, &type, &nelmts)) SS_FAILED_WHEN("ss_attr_describe failed");
        if (!H5Tequal(type, H5T_NATIVE_UCHAR)) SS_FAILED_WHEN("incorrect datatype read back");
        if (2!=nelmts) SS_FAILED_WHEN("incorrect number of elements read back");
        H5Tclose(type);
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 2, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr1_data[0] || i_ret[1]!=attr1_data[1]) SS_FAILED_WHEN("original values clobbered");
        free(i_ret);

        /* Change the datatype from char long long and add an element */
        if (_print) fprintf(_print, "  changing type from char to long long and adding an element\n");
        if (ss_attr_modify(attr1, H5T_NATIVE_LLONG, 3, SS_ALLSAME)<0) SS_FAILED_WHEN("ss_attr_modify failed");
        if (!ss_attr_describe(attr1, NULL, &type, &nelmts)) SS_FAILED_WHEN("ss_attr_describe failed");
        if (!H5Tequal(type, H5T_NATIVE_LLONG)) SS_FAILED_WHEN("incorrect datatype read back");
        if (3!=nelmts) SS_FAILED_WHEN("incorrect number of elements read back");
        H5Tclose(type);
        i_ret = ss_attr_get(attr1, H5T_NATIVE_INT, 0, 3, NULL);
        if (!i_ret) SS_FAILED_WHEN("ss_attr_get failed");
        if (i_ret[0]!=attr1_data[0] || i_ret[1]!=attr1_data[1]) SS_FAILED_WHEN("original values clobbered");
        if (i_ret[2]) SS_FAILED_WHEN("new value is garbage");
        free(i_ret);

        /* Change datatype to something incompatible with a long long */
        if (_print) fprintf(_print, "  changing type to 32-byte opaque; adding an element\n");
        if ((otype=H5Tcreate(H5T_OPAQUE, 32))<0) SS_FAILED_WHEN("H5Tcreate failed");
        if (H5Tset_tag(otype, "tattr1f:testing")<0) SS_FAILED_WHEN("H5Tset_tag failed");

/* *************************************************************/
/* eai removing ss_attr_modify stops this program from hanging */

        if (ss_attr_modify(attr1, otype, 4, SS_ALLSAME)<0) SS_FAILED_WHEN("ss_attr_modify failed");
        if (!ss_attr_describe(attr1, NULL, &type, &nelmts)) SS_FAILED_WHEN("ss_attr_describe failed");
        if (!H5Tequal(type, otype)) SS_FAILED_WHEN("incorrect datatype read back");
        if (4!=nelmts) SS_FAILED_WHEN("incorrect number of elements read back");
        H5Tclose(type);
        v_ret = ss_attr_get(attr1, otype, 0, 4, NULL);
        if (!v_ret) SS_FAILED_WHEN("ss_attr_get failed");
        v_zero = calloc(4, 32);
        if (memcmp(v_ret, v_zero, 4*32)) SS_FAILED_WHEN("expected all zero values");
        v_zero = SS_FREE(v_zero);
        free(v_ret);

        /* Close the files */
        if (ss_file_close(file1)<0) SS_FAILED_WHEN("closing file 1");
        SS_FREE(file1);
        SS_FREE(tscope1);
        SS_FREE(attr1);

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
    nerr += tattr1a();                  /* basic datatype serialization */
    nerr += tattr1b();                  /* basic attribute table objects */
    nerr += tattr1c();                  /* creation through the attr interface */
    nerr += tattr1d();                  /* data read operations */
    nerr += tattr1e();                  /* data modification operations */
    nerr += tattr1f();                  /* metadata modification operations */

    /* Finalization */
    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return nerr?1:0;
}
