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
#define TASK_BASE 100000000

typedef struct {
    double real, imag;
} tblob1_complex_t;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Linear Synchronous Blob Operations
 * Purpose:     Create a file name
 *
 * Description: Creates a file name for these tests
 *
 * Return:      Ptr to a static file name.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, June 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static char *
tblob1_name(const char *base, hsize_t size1)
{
    static char         name[1024];

    sprintf(name, "%s-%lu.saf", base, (unsigned long)size1);
    return name;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Linear Synchronous Blob Operations
 * Purpose:     Blob creation
 *
 * Description: Tests blob creation functions.
 *
 * Return:      Returns the number of errors encountered.
 *
 * Parallel:    Collective across MPI_COMM_WORLD.
 *
 * Programmer:  Robb Matzke
 *              Thursday, September 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob1a(hsize_t size1)
{
    int         nerr=0;
    ss_file_t   *file=NULL;
    ss_scope_t  *tscope=NULL;
    ss_blob_t   *blob=NULL;
    int         self;
    FILE        *_print=NULL;
    char        *filename=tblob1_name("tblob1_a1", size1);

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("blob creation") {
        /* Create a new file for this test */
        file = ss_file_create(filename, H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("optaining top scope");

        /* Collectively create a new blob in the top scope */
        blob = ss_blob_new(tscope, SS_ALLSAME, NULL);
        if (!blob) SS_FAILED_WHEN("creating blob");

        blob = SS_PERS_NEW(tscope, ss_blob_t, SS_ALLSAME);
        if (!blob) SS_FAILED_WHEN("creating second blob");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing the file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Linear Synchronous Blob Operations
 * Purpose:     Blob memory binding
 *
 * Description: Tests blob memory binding operations
 *
 * Return:      Returns the number of errors encountered.
 *
 * Parallel:    Collective across MPI_COMM_WORLD.
 *
 * Programmer:  Robb Matzke
 *              Thursday, September 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob1b(hsize_t size1)
{
    int         nerr=0;
    int         *buffer=(int*)1, *b=NULL;
    ss_file_t   *file=NULL;
    ss_scope_t  *tscope=NULL;
    ss_blob_t   *blob=NULL;
    hsize_t     size2;
    hid_t       mtype=-1;
    int         self;
    FILE        *_print=NULL;
    char        *filename=tblob1_name("tblob1_b1", size1);

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("blob memory binding") {
        /* Create a new file for this test */
        file = ss_file_create(filename, H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");

        /* Collectively create a new blob in the top scope */
        blob = SS_PERS_NEW(tscope, ss_blob_t, SS_ALLSAME);
        if (!blob) SS_FAILED_WHEN("creating blob");

        /* Bind and unbind a couple of times with 1d interface */
        if (ss_blob_bind_m1(blob, buffer, H5T_NATIVE_INT, size1)<0) SS_FAILED_WHEN("binding");
        if (ss_blob_bind_m1(blob, NULL, 0, (hsize_t)0)<0) SS_FAILED_WHEN("unbinding");
        if (ss_blob_bind_m1(blob, buffer, H5T_NATIVE_INT, size1)<0) SS_FAILED_WHEN("rebinding");
        if (ss_blob_bind_m1(blob, buffer, H5T_NATIVE_INT, size1)<0) SS_FAILED_WHEN("re-rebinding");

        /* Query the binding */
        b = ss_blob_bound_m1(blob, NULL, NULL);
        if (b!=buffer) SS_FAILED_WHEN("querying buffer");
        b = ss_blob_bound_m1(blob, &mtype, NULL);
        if (b!=buffer) SS_FAILED_WHEN("querying type");
        if (H5Tequal(H5T_NATIVE_INT, mtype)<=0) SS_FAILED_WHEN("returned wrong datatype");
        b = ss_blob_bound_m1(blob, &mtype, &size2);
        if (b!=buffer) SS_FAILED_WHEN("querying size");
        if (size2!=size1) SS_FAILED_WHEN("returned wrong size");

        /* Query when unbound -- should be an error */
        if (ss_blob_bind_m1(blob, NULL, 0, (hsize_t)0)<0) SS_FAILED_WHEN("unbinding again");
        H5E_BEGIN_TRY {
            b = ss_blob_bound_m1(blob, NULL, NULL);
        } H5E_END_TRY;
        if (b) SS_FAILED_WHEN("querying when not bound");

        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Linear Synchronous Blob Operations
 * Purpose:     Blob dataset binding
 *
 * Description: Tests blob dataset binding operations
 *
 * Return:      Returns the number of errors encountered.
 *
 * Parallel:    Collective across MPI_COMM_WORLD.
 *
 * Programmer:  Robb Matzke
 *              Thursday, September 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob1c(const char *name, hsize_t size1, htri_t extendible)
{
    int         nerr=0;
    ss_file_t   *file=NULL;
    ss_scope_t  *tscope=NULL;
    ss_blob_t   *blob=NULL;
    hid_t       fid=-1, dset=-1, space=-1, dset2=-1, dcpl=-1;
    hsize_t     size2, offset, dsize, dmaxsize, chunksize=5;
    herr_t      status;
    int         self;
    FILE        *_print=NULL;
    char        *filename=tblob1_name(name, size1);

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("blob dataset binding") {
        if (_print) {
            fprintf(_print, "  filename:   %s\n", filename);
            fprintf(_print, "  elements:   %lu %s %s\n",
                    (unsigned long)size1, "SAF_ALL", extendible?"extendible":"fixed-size");
        }

        /* Create a new file for this test */
        file = ss_file_create(filename, H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");

        /* Collectively create a new blob in the top scope */
        blob = SS_PERS_NEW(tscope, ss_blob_t, SS_ALLSAME);
        if (!blob) SS_FAILED_WHEN("creating blob");

        /* Query when unbound -- should be an error */
        H5E_BEGIN_TRY {
            status = ss_blob_bound_f1(blob, NULL, NULL, NULL, NULL);
        } H5E_END_TRY;
        if (status>=0) SS_FAILED_WHEN("querying unbound");
        
        /* Create a dataset */
        dcpl = H5Pcreate(H5P_DATASET_CREATE);
        if (extendible) H5Pset_chunk(dcpl, 1, &chunksize);
        dsize = MAX(1, size1); /*difficult to create a zero-sized dataset*/
        fid = ss_file_isopen(file, NULL);
        if (fid<0) SS_FAILED_WHEN("obtaining HDF5 file handle");
        dmaxsize = extendible ? H5S_UNLIMITED : dsize;
        space = H5Screate_simple(1, &dsize, &dmaxsize);
        if (space<0) SS_FAILED_WHEN("creating data space");
        dset = H5Dcreate(fid, "dset_c1", H5T_NATIVE_INT, space, dcpl);
        if (dset<0) SS_FAILED_WHEN("creating dataset");

        /* Bind to dataset with 1d interface and query */
        if (ss_blob_bind_f1(blob, dset, (hsize_t)0, size1, extendible?SS_BLOB_EXTEND:0U)<0) SS_FAILED_WHEN("binding");
        if (ss_blob_bound_f1(blob, NULL, NULL, NULL, NULL)<0) SS_FAILED_WHEN("querying whether bound");
        if (ss_blob_bound_f1(blob, &dset2, NULL, NULL, NULL)<0) SS_FAILED_WHEN("querying dataset");
        /* if (H5Dclose(dset2)<0) SS_FAILED_WHEN("closing returned dataset"); // DO NOT CLOSE. SEE ss_blob_bound_f() */
        if (ss_blob_bound_f1(blob, NULL, &offset, &size2, NULL)<0) SS_FAILED_WHEN("querying selection");
        if (offset!=0) SS_FAILED_WHEN("wrong offset");
        if (size1!=size2) SS_FAILED_WHEN("wrong size");

        /* Extend the blob and query again */
        if (extendible) {
            if (ss_blob_extend1(blob, 2*size1, 0U, NULL)<0) SS_FAILED_WHEN("extending blob");
            if (ss_blob_bound_f1(blob, NULL, &offset, &size2, NULL)<0) SS_FAILED_WHEN("querying extended blob");
            if (offset!=0) SS_FAILED_WHEN("wrong extended offset");
            if (size2!=2*size1) SS_FAILED_WHEN("wrong extended size");
        }
        
        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Linear Synchronous Blob Operations
 * Purpose:     Blob dataset allocation
 *
 * Description: Tests blob dataset allocation by binding to memory and then calling ss_blob_mkstorage().
 *
 * Return:      Returns the number of errors encountered.
 *
 * Parallel:    Collective across MPI_COMM_WORLD.
 *
 * Programmer:  Robb Matzke
 *              Thursday, September 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob1d(const char *name, hsize_t size1, htri_t extendible,
        hbool_t allsame)
{
    int         nerr=0, *buffer=(int*)1;
    ss_file_t   *file=NULL;
    ss_scope_t  *tscope=NULL;
    ss_blob_t   *blob=NULL;
    hsize_t     start;
    int         self;
    FILE        *_print=NULL;
    char        *filename=tblob1_name(name, size1);
    hid_t       complex=-1;
    unsigned    flags=0;
    
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("dataset creation") {
        if (_print) {
            fprintf(_print, "  filename:   %s\n", filename);
            fprintf(_print, "  elements:   %lu %s %s\n",
                    (unsigned long)size1, allsame?"SAF_ALL":"SAF_EACH", extendible?"extendible":"fixed-size");
        }
        
        /* Create a new file for this test */
        file = ss_file_create(filename, H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");

        /* Collectively create a new blob in the top scope */
        blob = SS_PERS_NEW(tscope, ss_blob_t, SS_ALLSAME);
        if (!blob) SS_FAILED_WHEN("creating blob");

        /* Bind blob to memory */
        if (ss_blob_bind_m1(blob, buffer, H5T_NATIVE_INT, size1)<0) SS_FAILED_WHEN("binding memory");

        /* Create and bind the dataset */
        flags = (extendible?SS_BLOB_EXTEND:0U) | (allsame?SS_ALLSAME:0U);
        if (ss_blob_mkstorage(blob, &start, flags, NULL)<0) SS_FAILED_WHEN("creating dataset");
        if (allsame) {
            if (start!=0) SS_FAILED_WHEN("starting offset");
        } else {
            if (start!=(size_t)(size1*self)) SS_FAILED_WHEN("starting offset");
        }
        
        /* Do the same thing again but use a more complicated datatype */
        complex = H5Tcreate(H5T_COMPOUND, sizeof(tblob1_complex_t));
        H5Tinsert(complex, "real", HOFFSET(tblob1_complex_t,real), H5T_NATIVE_DOUBLE);
        H5Tinsert(complex, "imag", HOFFSET(tblob1_complex_t,imag), H5T_NATIVE_DOUBLE);
        blob = SS_PERS_NEW(tscope, ss_blob_t, SS_ALLSAME);
        if (!blob) SS_FAILED_WHEN("creating second blob");
        if (ss_blob_bind_m1(blob, buffer, complex, size1)<0) SS_FAILED_WHEN("binding second memory");
        flags = (extendible?SS_BLOB_EXTEND:0U) | (allsame?SS_ALLSAME:0U);
        if (ss_blob_mkstorage(blob, &start, flags, NULL)<0) SS_FAILED_WHEN("creating second dataset");
        if (allsame) {
            if (start!=0) SS_FAILED_WHEN("second starting offset");
        } else {
            if (start!=(size_t)(size1*self)) SS_FAILED_WHEN("second starting offset");
        }
        
        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");

        /* Open the file and then close it again to test that blobs can be booted */
        file = ss_file_open(NULL, filename, H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("opening file");
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing second file");

        /* Open the file and then close it a third time to test that blobs can be booted */
        file = ss_file_open(NULL, filename, H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("opening file again");
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing third file");
        
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Linear Synchronous Blob Operations
 * Purpose:     Blob I/O
 *
 * Description: Tries to write and read a blob
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Monday, June 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob1f(const char *name, hsize_t size1, htri_t extendible,
        hbool_t allsame, hbool_t collective)
{
    int         nerr=0, nwrong=0, *buffer=NULL, *rdata=NULL;
    hsize_t     my_offset, i, newsize;
    ss_file_t   *file=NULL;
    ss_scope_t  *tscope=NULL;
    ss_blob_t   *blob=NULL;
    int         self, ntasks;
    FILE        *_print=NULL;
    char        *filename=tblob1_name(name, size1);
    unsigned    flags;

    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("blob write and read") {
        if (_print) {
            fprintf(_print, "  filename:   %s\n", filename);
            fprintf(_print, "  elements:   %lu %s %s\n",
                    (unsigned long)size1, allsame?"SAF_ALL":"SAF_EACH", extendible?"extendible":"fixed-size");
            fprintf(_print, "  I/O mode:   %s\n", collective?"collective":"independent");
        }

#if H5_VERS_NUMBER <= 1007031
        /* A bug in HDF5-1.7.31 and probably earlier versions causes a failed assertion in H5TB_end() under the H5Dwrite()
         * call for extendible but currently empty 1-dimensional datasets. */
        if (0==size1 && extendible) SS_SKIPPED_WHEN("skipped for HDF5-1.7.31 H5TB_end() bug");
#endif

#if H5_VERS_NUMBER <= 1007037 && !defined(HAVE_PARALLEL)
        /* A bug in HDF5-1.7.37 and probably earlier causes a failed assersion in H5D_chunk_read() */
        if (0==size1 && extendible) SS_SKIPPED_WHEN("skipped for HDF5-1.7.37 H5D_chunk_read() bug");
#endif

#if H5_VERS_NUMBER <= 1007037
        /* A bug in HDF5-1.7.37 and probably earlier versions causes a failed assertion in H5D_contig_read() under the
         * H5Dread() call when size1 is zero for serial versions of the library. */
        if (0==size1 && !extendible)
            SS_SKIPPED_WHEN("skipped for HDF5-1.7.37 H5D_contig_read() bug");
#endif

        /* Create a new file for this test */
        file = ss_file_create(filename, H5F_ACC_RDWR, NULL);
        if (!file) SS_FAILED_WHEN("creating file");
        tscope = ss_file_topscope(file, NULL);
        if (!tscope) SS_FAILED_WHEN("obtaining top scope");

        /* Collectively create a new blob in the top scope */
        blob = SS_PERS_NEW(tscope, ss_blob_t, SS_ALLSAME);
        if (!blob) SS_FAILED_WHEN("creating blob");

        /* Initialize memory and bind it to the blob */
        buffer = malloc((size_t)MAX(size1,1) * sizeof(*buffer));
        for (i=0; i<size1; i++) buffer[i] = allsame?TASK_BASE+i:(self+1)*TASK_BASE+i;
        if (ss_blob_bind_m1(blob, buffer, H5T_NATIVE_INT, size1)<0) SS_FAILED_WHEN("binding memory");

        /* Create and bind the dataset */
        flags = (extendible?SS_BLOB_EXTEND:0U) | (allsame?SS_ALLSAME:0U);
        if (ss_blob_mkstorage(blob, &my_offset, flags, NULL)<0) SS_FAILED_WHEN("creating dataset");

        /* Perform the I/O operation. Every task will write its buffer in task rank order. */
        flags = SS_BLOB_UNBIND | (collective?SS_BLOB_COLLECTIVE:0U) | (allsame?SS_ALLSAME:0U);
        if (ss_blob_write1(blob, my_offset, size1, flags, NULL)<0)
            SS_FAILED_WHEN("writing data");

        /* Now attempt to read the same data */
        flags = SS_BLOB_UNBIND | (collective?SS_BLOB_COLLECTIVE:0U) | (allsame?SS_ALLSAME:0U);
        rdata = ss_blob_read1(blob, my_offset, size1, flags, NULL);
        if (!rdata) SS_FAILED_WHEN("reading data");

        /* Compare the data */
        for (i=0, nwrong=0; i<size1; i++) {
            if (buffer[i]!=rdata[i]) {
                if (nwrong++<10)
                    fprintf(stderr, "  task %d index %lu: wrote %d but read %d\n", self, (unsigned long)i, buffer[i], rdata[i]);
            }
        }
#ifdef HAVE_PARALLEL
        MPI_Allreduce(&nwrong, &nerr, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
#else
        nerr = nwrong;
#endif
        if (nerr>0) SS_FAILED_WHEN("comparing data");

        /* If the blob is extendible then extend it and write more data */
        if (extendible) {
            for (i=0; i<size1; i++) buffer[i] = allsame?TASK_BASE+i:(self+1)*TASK_BASE+size1+i;
            if (ss_blob_bind_m1(blob, buffer, H5T_NATIVE_INT, size1)<0) SS_FAILED_WHEN("binding memory for extension");
            if (allsame) {
                my_offset = size1;
            } else {
                my_offset = ntasks*size1 + self*size1;
            }
            newsize = my_offset + size1;
            flags = allsame?SS_ALLSAME:0U;
            if (ss_blob_extend1(blob, newsize, flags, NULL)<0) SS_FAILED_WHEN("extending the blob");
            flags = SS_BLOB_UNBIND | (collective?SS_BLOB_COLLECTIVE:0U) | (allsame?SS_ALLSAME:0U);
            if (ss_blob_write1(blob, my_offset, size1, flags, NULL)<0) SS_FAILED_WHEN("writing extended data");
            rdata = ss_blob_read1(blob, my_offset, size1, flags, NULL);
            if (!rdata) SS_FAILED_WHEN("reading extended data");
#if 0
            if (_print) fprintf(_print, "  THIS TEST IS PARTIALLY DISABLED DUE TO BUGS IN HDF5-1.7.15 THROUGH 1.7.30\n");
#else
            for (i=0, nwrong=0; i<size1; i++) {
                if (buffer[i]!=rdata[i]) {
                    if (nwrong++<10)
                        fprintf(stderr, "  task %d index %lu: wrote %d but read %d (dset offset %lu)\n",
                                self, (unsigned long)i, buffer[i], rdata[i], (unsigned long)(my_offset+i));
                }
            }
            if (nwrong>0) SS_FAILED_WHEN("comparing extended data");
#endif
        }
        
        /* Close the file */
        if (ss_file_close(file)<0) SS_FAILED_WHEN("closing file");
        file = NULL;
    } SS_END_CHECKING_WITH(nerr++);
    if (buffer) free(buffer);

    return nerr;
}
        
/**/
int
main(int argc, char *argv[])
{
    int         nerr=0, argno;
    hsize_t     nelmts=10;

    /* Initialization */
#ifdef HAVE_PARALLEL
    MPI_Init(&argc, &argv);
#endif
    ss_init(SS_COMM_WORLD);

    /* Parse arguments */
    for (argno=1; argno<argc; argno++) {
        if (!strncmp(argv[argno], "--nelmts=", 9)) {
            nelmts = strtol(argv[argno]+9, NULL, 0);
        } else {
            fprintf(stderr, "%s: unknown argument: %s\n", argv[0], argv[argno]);
            goto done;
        }
    }

    /* Tests */
    nerr += tblob1a(nelmts);
    nerr += tblob1b(nelmts);

    /* Arguments:
     *   Collective I/O operations? ---------------------+
     *   Allsame blob creation size? -------------+      |
     *   Extendible blob? -----------------V      V      V   */
    nerr += tblob1c("tblob1_c1", nelmts, FALSE);
    nerr += tblob1c("tblob1_c2", nelmts, TRUE);

    nerr += tblob1d("tblob1_d1", nelmts, FALSE, FALSE);
    nerr += tblob1d("tblob1_d2", nelmts, FALSE, TRUE);
    nerr += tblob1d("tblob1_d3", nelmts, TRUE,  FALSE);
    nerr += tblob1d("tblob1_d4", nelmts, TRUE,  TRUE);

    nerr += tblob1f("tblob1_f1", nelmts, FALSE, FALSE, FALSE);
    nerr += tblob1f("tblob1_f2", nelmts, FALSE, FALSE, TRUE);
    nerr += tblob1f("tblob1_f3", nelmts, FALSE, TRUE,  FALSE);
    nerr += tblob1f("tblob1_f4", nelmts, FALSE, TRUE,  TRUE);
    nerr += tblob1f("tblob1_f5", nelmts, TRUE,  FALSE, FALSE);
    nerr += tblob1f("tblob1_f6", nelmts, TRUE,  FALSE, TRUE);
    nerr += tblob1f("tblob1_f7", nelmts, TRUE,  TRUE,  FALSE);
    nerr += tblob1f("tblob1_f8", nelmts, TRUE,  TRUE,  TRUE);

    /* Finalization */
done:
    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return nerr?1:0;
}
