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

#define DSET_DIM_0      3               /* The number of blobs */
#if 1
#define DSET_DIM_1      20              /* Must be at least 4 */
#define DSET_DIM_2      21              /* Must be at least 8 */
#define DSET_DIM_3      22              /* Must be at least 11 */
#else
#define DSET_DIM_1      5               /* Must be at least 4 */
#define DSET_DIM_2      9               /* Must be at least 8 */
#define DSET_DIM_3      12              /* Must be at least 11 */
#endif

#define DSET_DECL(X)    int X[DSET_DIM_0][DSET_DIM_1][DSET_DIM_2][DSET_DIM_3]
#define DSET_SIZE       {DSET_DIM_0, DSET_DIM_1, DSET_DIM_2, DSET_DIM_3}


DSET_DECL(data_g);                              /* The expected dataset value */
ss_file_t       *file_g = NULL;                 /* The file opened by this test */
ss_scope_t      *tscope_g = NULL;               /* File top scope */
ss_blob_t       *blob_g[DSET_DIM_0];            /* The blobs all pointing into a single dataset */
hid_t           dset_g;                         /* HDF5 dataset for the blobs */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Multidimensional Asynchronous Blob Operations
 * Purpose:     Verify dataset contents
 *
 * Description: Verifies that the dataset's current contents match what's stored in data_g as the expected contents.
 *
 * Return:      Returns number of mismatches or negative on failure; zero on success.
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Wednesday, October 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob2_verify(void)
{
    hid_t               dspace=-1;
    int                 i, j, k, l, nerr=0, self;
    DSET_DECL(whole_dataset);

#if 1 /* Use this method to ensure that non-written data is still zero. */
    memset(whole_dataset, 1, sizeof whole_dataset);
#else /* Unfortunately the mpio driver before about 1.7.15 doesn't correctly handle short reads, so this is required */
    memset(whole_dataset, 0, sizeof whole_dataset);
#endif

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    if ((dspace=H5Dget_space(dset_g))<0) return -1;
    if (H5Dread(dset_g, H5T_NATIVE_INT, dspace, dspace, H5P_DEFAULT, whole_dataset)<0) return -1;
    for (i=0; i<DSET_DIM_0; i++) {
        for (j=0; j<DSET_DIM_1; j++) {
            for (k=0; k<DSET_DIM_2; k++) {
                for (l=0; l<DSET_DIM_3; l++) {
                    if (whole_dataset[i][j][k][l]!=data_g[i][j][k][l]) {
                        fprintf(stderr, "  T%d: read[%d][%d][%d][%d] = %d (expected %d)\n",
                                self, i, j, k, l, whole_dataset[i][j][k][l], data_g[i][j][k][l]);
                        nerr++;
                    }
                }
            }
        }
    }
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Multidimensional Asynchronous Blob Operations
 * Purpose:     Create file and blobs
 *
 * Description: This function creates a four dimensional dataset that will hold three dimensional fields where the slowest
 *              varying dimension is the field index.  It then creates three blobs and binds them to the new dataset.
 *
 * Return:      Returns the number of errors encountered.
 *
 * Parallel:    Collective across MPI_COMM_WORLD.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, October 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob2_create(void)
{
    hsize_t     dset_size[4] = DSET_SIZE;
    hsize_t     blob_start[4] = {0, 0, 0, 0};
    hsize_t     blob_size[4] = {1, DSET_DIM_1, DSET_DIM_2, DSET_DIM_3};
    FILE        *_print=NULL;
    int         self, i, nerr=0;
    hid_t       h5file=-1, dspace=-1;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("dataset creation and binding") {
        /* Create a new file for this test */
        file_g = ss_file_create("tblob2.saf", H5F_ACC_RDWR, NULL);
        if (!file_g) SS_FAILED_WHEN("creating file");
        tscope_g = ss_file_topscope(file_g, NULL);
        if (!tscope_g) SS_FAILED_WHEN("obtaining top scope");

        /* Create the dataset to hold the three blobs. */
        h5file = ss_file_isopen(file_g, NULL);
        if (h5file<0) SS_FAILED_WHEN("obtaining HDF5 file handle");
        if ((dspace=H5Screate_simple(4, dset_size, dset_size))<0) SS_FAILED_WHEN("defining data space");
        if ((dset_g=H5Dcreate(h5file, "the_main_dataset", H5T_NATIVE_INT, dspace, H5P_DEFAULT))<0)
            SS_FAILED_WHEN("creating dataset");

        /* Collectively create blobs */
        for (i=0; i<DSET_DIM_0; i++) {
            blob_g[i] = SS_PERS_NEW(tscope_g, ss_blob_t, SS_ALLSAME);
            if (!blob_g[i]) SS_FAILED_WHEN("creating blob");
        }
        
        /* Bind the blobs to the dataset */
        for (i=0; i<DSET_DIM_0; i++) {
            blob_start[0] = i;
            if (H5Sselect_slab(dspace, H5S_SELECT_SET, (hsize_t)0, blob_start, blob_size)<0)
                SS_FAILED_WHEN("defining dataset selection");
            if (ss_blob_bind_f(blob_g[i], dset_g, dspace, 0U)<0) SS_FAILED_WHEN("binding blob to dataset");
        }
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Multidimensional Asynchronous Blob Operations
 * Purpose:     Inititalizes blob data
 *
 * Description: Given the size of a blob and the location of the blob in memory and storage, this function will initialize a
 *              buffer with some data that is to be written to the file and adjust the expected values stored in the global
 *              dataset array.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, October 31, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static void
tblob2_init_data(int blob_num,                  /* Which blob are we talking about? */
                 const hsize_t *slab_size,      /* How big is the slab for I/O.  This is always three dimensional */
                 const hsize_t *slab_dstart,    /* Where does the slab land in the blob's file storage space? (3d) */
                 const hsize_t *slab_mstart,    /* Where is the slab data in the memory buffer? (3d) */
                 const hsize_t *buf_size,       /* How big is the memory buffer? (3d) */
                 int *buffer                    /* Optional memory buffer */
                 )
{
    hsize_t     i, j, k;
    static int  dval=1;
    static int  ival=1000000000;

    /* Make sure the requested slab fits in the dataset. The mismatch of dimensions is because the dataset is four dimensions,
     * the first of which is the blob index while the I/O request is only three dimensions (matching the blob dimensionality. */
    assert(blob_num>=0 && blob_num<DSET_DIM_0);
    assert(slab_dstart[0]+slab_size[0]<=DSET_DIM_1);
    assert(slab_dstart[1]+slab_size[1]<=DSET_DIM_2);
    assert(slab_dstart[2]+slab_size[2]<=DSET_DIM_3);

    /* Make sure the slab fits in the buffer */
    assert(slab_mstart[0]+slab_size[0]<=buf_size[0]);
    assert(slab_mstart[1]+slab_size[1]<=buf_size[1]);
    assert(slab_mstart[2]+slab_size[2]<=buf_size[2]);

    /* Initialize the buffer and update expected dataset contents. Watch out because the buffer, the I/O slab, and the
     * dataset need not all be the same size. */
    if (buffer) memset(buffer, 0, (size_t)(sizeof(*buffer)*buf_size[0]*buf_size[1]*buf_size[2]));
    for (i=0; i<buf_size[0]; i++) {
        for (j=0; j<buf_size[1]; j++) {
            for (k=0; k<buf_size[2]; k++) {
                if (i<slab_mstart[0] || i>=slab_mstart[0]+slab_size[0] ||
                    j<slab_mstart[1] || j>=slab_mstart[1]+slab_size[1] ||
                    k<slab_mstart[2] || k>=slab_mstart[2]+slab_size[2]) {
                    /* Buffer element is not part of the I/O request. */
                    if (buffer) buffer[i*buf_size[1]*buf_size[2] + j*buf_size[2] + k] = ival;
                } else {
                    if (buffer) buffer[i*buf_size[1]*buf_size[2] + j*buf_size[2] + k] = dval;
                    data_g
                        [blob_num]
                        [slab_dstart[0]+i-slab_mstart[0]]
                        [slab_dstart[1]+j-slab_mstart[1]]
                        [slab_dstart[2]+k-slab_mstart[2]] = dval;
                    dval++;
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Multidimensional Asynchronous Blob Operations
 * Purpose:     Blob writing
 *
 * Description: Writes asynchronously from one task to part of the first blob.  The source memory is a three dimensional
 *              buffer whose middle dimension is slightly larger than the ultimate I/O request, which makes the data transfer
 *              partial in both the source and destination aspects.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Issue:       HDF5-1.7.9 is necessary for partial I/O from memory to work properly.
 *              
 * Programmer:  Robb Matzke
 *              Wednesday, October 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob2_write1(void)
{
    const hsize_t       req_size[3]      = {DSET_DIM_1-3, DSET_DIM_2-7, DSET_DIM_3-10}; /* size of the I/O request */
    const hsize_t       req_offset[3]    = {1,            4,            5            }; /* where it lands in blob's part of dset */
    const hsize_t       buffer_size[3]   = {DSET_DIM_1-3, DSET_DIM_2-3, DSET_DIM_3-10}; /* total size of the buffer */
    int                 buffer             [DSET_DIM_1-3][DSET_DIM_2-3][DSET_DIM_3-10]; /* buffer, larger than request */
    const hsize_t       buffer_start[3]  = {0,            3,            0            }; /* where request starts in buffer */

    hid_t               mspace=-1, iospace=-1;
    int                 self, nerr=0, blob_idx=0;
    FILE                *_print=NULL;
    unsigned            write_flags = SS_BLOB_UNBIND; /*and augmented below*/

    /* Choose one type or another. The datatype of the file is H5T_NATIVE_INT and choosing H5T_NATIVE_UINT for the memory type
     * will cause a slightly different code path to be taken in SSlib. */
#if 0
    hid_t               mtype = H5T_NATIVE_INT;
#else
    hid_t               mtype = H5T_NATIVE_UINT;
#endif
                                        
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("single-task write") {
        /* Initialize the buffer */
        tblob2_init_data(blob_idx, req_size, req_offset, buffer_start, buffer_size, (int*)buffer);

        if (_print) {
            fprintf(_print, "    total mem buffer size [%lu,%lu,%lu]\n",
                    (unsigned long)(buffer_size[0]), (unsigned long)(buffer_size[1]), (unsigned long)(buffer_size[2]));
            fprintf(_print, "    mem data starts at [%lu,%lu,%lu]\n",
                    (unsigned long)(buffer_start[0]), (unsigned long)(buffer_start[1]), (unsigned long)(buffer_start[2]));
            fprintf(_print, "    number of elements to write [%lu,%lu,%lu]\n",
                    (unsigned long)(req_size[0]), (unsigned long)(req_size[1]), (unsigned long)(req_size[2]));
            fprintf(_print, "    where data lands in dataset [%lu,%lu,%lu]\n",
                    (unsigned long)(req_offset[0]), (unsigned long)(req_offset[1]), (unsigned long)(req_offset[2]));
            fprintf(_print, "    first integer to write: %d\n", buffer[buffer_start[0]][buffer_start[1]][buffer_start[2]]);
            fprintf(_print, "    last integer to write: %d\n",
                    buffer[buffer_start[0]+req_size[0]-1][buffer_start[1]+req_size[1]-1][buffer_start[2]+req_size[2]-1]);
        }
        
        /* Task zero will write some stuff */
        if (0==self) {
            /* Bind buffer to the blob */
            if ((mspace=H5Screate_simple(3, buffer_size, buffer_size))<0) SS_FAILED_WHEN("creating memory space");
            if (H5Sselect_slab(mspace, H5S_SELECT_SET, (hsize_t)0, buffer_start, req_size)<0)
                SS_FAILED_WHEN("creating memory selection");
            if (ss_blob_bind_m(blob_g[blob_idx], buffer, mtype, mspace)<0) SS_FAILED_WHEN("binding to memory");
            if (H5Sclose(mspace)<0) SS_FAILED_WHEN("closing data space");
        
            /* Describe what part of the dataset we're writing to. This is 3d because a blob is 3d (even though the dataspace
             * where the blob is stored is 4d). */
            if (ss_blob_space(blob_g[blob_idx], NULL, &iospace)<0) SS_FAILED_WHEN("obtaining blob data space");
            if (H5Sselect_slab(iospace, H5S_SELECT_SET, (hsize_t)0, req_offset, req_size)<0)
                SS_FAILED_WHEN("defining io selection");
        
            /* Write the data -- you may comment out the SS_BLOB_ASYNC to cause a different and much simpler code path through
             * SSlib to be used in order to check that the logic here is correct. */
#if 1
            write_flags |= SS_BLOB_ASYNC;
#endif
            if (ss_blob_write(blob_g[blob_idx], iospace, write_flags, NULL)<0) SS_FAILED_WHEN("writing blob");
        }

        /* All tasks synchronize, flush data, and read. We need the barrier because ss_blob_flush() doesn't have a barrier. */
        if (ss_blob_synchronize(tscope_g, NULL)<0) SS_FAILED_WHEN("synchronizing independent writes");
        if (ss_blob_flush(tscope_g, blob_g[blob_idx], SS_STRICT, NULL)<0) SS_FAILED_WHEN("flushing independent writes");
        ss_mpi_barrier(SS_COMM_WORLD);
        if (tblob2_verify()) SS_FAILED_WHEN("verifying results");
        
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Multidimensional Asynchronous Blob Operations
 * Purpose:     Blob writing
 *
 * Description: Writes asynchronously and repeatedly from one task to part of the second blob.  The source memory is a three
 *              dimensional buffer whose middle dimension is slightly larger than the ultimate I/O request, which makes the
 *              data transfer partial in both the source and destination aspects.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Issue:       HDF5-1.7.9 is necessary for partial I/O from memory to work properly.
 *              
 * Programmer:  Robb Matzke
 *              Wednesday, October 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob2_write2(void)
{
    const hsize_t       req_size[3]      = {DSET_DIM_1-3, DSET_DIM_2-7, DSET_DIM_3-10}; /* size of the I/O request */
    const hsize_t       req_offset[3]    = {1,            4,            5            }; /* where it lands in blob's part of dset */
    const hsize_t       buffer_size[3]   = {DSET_DIM_1-3, DSET_DIM_2-3, DSET_DIM_3-10}; /* total size of the buffer */
    const hsize_t       buffer_start[3]  = {0,            3,            0            }; /* where request starts in buffer */

    int                 *buffer[3]= {NULL, NULL, NULL};
    hid_t               mspace=-1, iospace=-1;
    int                 self, nerr=0, blob_idx=1, i;
    FILE                *_print=NULL;
    unsigned            write_flags = SS_BLOB_UNBIND; /*and augmented below*/

    /* Choose one type or another. The datatype of the file is H5T_NATIVE_INT and choosing H5T_NATIVE_UINT for the memory type
     * will cause a slightly different code path to be taken in SSlib. */
#if 0
    hid_t               mtype = H5T_NATIVE_INT;
#else
    hid_t               mtype = H5T_NATIVE_UINT;
#endif
                                        
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("single-task repeated write") {

        /* Task zero will write some stuff */
        for (i=0; i<3; i++) {
            /* Initialize the buffer */
            if (0==self) buffer[i] = malloc((size_t)(buffer_size[0]*buffer_size[1]*buffer_size[2]*sizeof(*buffer)));
            tblob2_init_data(blob_idx, req_size, req_offset, buffer_start, buffer_size, 0==self?buffer[i]:NULL);
            
            if (0==self) {
                /* Bind buffer to the blob */
                if ((mspace=H5Screate_simple(3, buffer_size, buffer_size))<0) SS_FAILED_WHEN("creating memory space");
                if (H5Sselect_slab(mspace, H5S_SELECT_SET, (hsize_t)0, buffer_start, req_size)<0)
                    SS_FAILED_WHEN("creating memory selection");
                if (ss_blob_bind_m(blob_g[blob_idx], buffer[i], mtype, mspace)<0) SS_FAILED_WHEN("binding to memory");
                if (H5Sclose(mspace)<0) SS_FAILED_WHEN("closing data space");
        
                /* Describe what part of the dataset we're writing to. This is 3d because a blob is 3d (even though the dataspace
                 * where the blob is stored is 4d). */
                if (ss_blob_space(blob_g[blob_idx], NULL, &iospace)<0) SS_FAILED_WHEN("obtaining blob data space");
                if (H5Sselect_slab(iospace, H5S_SELECT_SET, (hsize_t)0, req_offset, req_size)<0)
                    SS_FAILED_WHEN("defining io selection");
        
                /* Write the data -- you may comment out the SS_BLOB_ASYNC to cause a different and much simpler code path through
                 * SSlib to be used in order to check that the logic here is correct. */
#if 1
                write_flags |= SS_BLOB_ASYNC;
#endif
                if (ss_blob_write(blob_g[blob_idx], iospace, write_flags, NULL)<0) SS_FAILED_WHEN("writing blob");
            }
        }

        /* All tasks synchronize, flush data, and read. We need the barrier because ss_blob_flush() doesn't have a barrier. */
        if (ss_blob_synchronize(tscope_g, NULL)<0) SS_FAILED_WHEN("synchronizing independent writes");
        if (ss_blob_flush(tscope_g, blob_g[blob_idx], SS_STRICT, NULL)<0) SS_FAILED_WHEN("flushing independent writes");
        ss_mpi_barrier(SS_COMM_WORLD);
        if (tblob2_verify()) SS_FAILED_WHEN("verifying results");

        if (0==self) {
            free(buffer[0]);
            free(buffer[1]);
            free(buffer[2]);
        }
        
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Multidimensional Asynchronous Blob Operations
 * Purpose:     Blob writing
 *
 * Description: Writes asynchronously and collectively to part of the third blob. Each of up to DSET_DIM_1 tasks will write
 *              a single slab to the dataset. None of the tasks' data will overlap with any other task.
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Issue:       HDF5-1.7.9 is necessary for partial I/O from memory to work properly.
 *              
 * Programmer:  Robb Matzke
 *              Wednesday, October 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob2_write3(void)
{
    const hsize_t       req_size[3]      = {1,            DSET_DIM_2,   DSET_DIM_3   }; /* size of the I/O request */
    hsize_t             req_offset[3]    = {999999,       0,            0            }; /* where it lands in blob's part of dset */
    const hsize_t       buffer_size[3]   = {1,            DSET_DIM_2,   DSET_DIM_3   }; /* total size of the buffer */
    const hsize_t       buffer_start[3]  = {0,            0,            0            }; /* where request starts in buffer */

    int                 *buffer=NULL;
    hid_t               mspace=-1, iospace=-1;
    int                 self, ntasks, task, nerr=0, blob_idx=2;
    FILE                *_print=NULL;
    unsigned            write_flags = SS_BLOB_UNBIND; /*and augmented below*/

    /* Choose one type or another. The datatype of the file is H5T_NATIVE_INT and choosing H5T_NATIVE_UINT for the memory type
     * will cause a slightly different code path to be taken in SSlib. */
#if 0
    hid_t               mtype = H5T_NATIVE_INT;
#else
    hid_t               mtype = H5T_NATIVE_UINT;
#endif
                                        
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("collective write") {

        /* Initialize the buffer */
        buffer = malloc((size_t)(buffer_size[0]*buffer_size[1]*buffer_size[2]*sizeof(*buffer)));
        for (task=0; task<ntasks && task<DSET_DIM_1; task++) {
            req_offset[0] = task;
            tblob2_init_data(blob_idx, req_size, req_offset, buffer_start, buffer_size, task==self?buffer:NULL);
        }
        req_offset[0] = self;

        if (self<DSET_DIM_1) {
            /* Bind buffer to the blob */
            if ((mspace=H5Screate_simple(3, buffer_size, buffer_size))<0) SS_FAILED_WHEN("creating memory space");
            if (H5Sselect_slab(mspace, H5S_SELECT_SET, (hsize_t)0, buffer_start, req_size)<0)
                SS_FAILED_WHEN("creating memory selection");
            if (ss_blob_bind_m(blob_g[blob_idx], buffer, mtype, mspace)<0) SS_FAILED_WHEN("binding to memory");
            if (H5Sclose(mspace)<0) SS_FAILED_WHEN("closing data space");
        
            /* Describe what part of the dataset we're writing to. This is 3d because a blob is 3d (even though the dataspace
             * where the blob is stored is 4d). */
            if (ss_blob_space(blob_g[blob_idx], NULL, &iospace)<0) SS_FAILED_WHEN("obtaining blob data space");
            if (H5Sselect_slab(iospace, H5S_SELECT_SET, (hsize_t)0, req_offset, req_size)<0)
                SS_FAILED_WHEN("defining io selection");
        
            /* Write the data -- you may comment out the SS_BLOB_ASYNC to cause a different and much simpler code path through
             * SSlib to be used in order to check that the logic here is correct. */
#if 1
            write_flags |= SS_BLOB_ASYNC;
#endif
            if (ss_blob_write(blob_g[blob_idx], iospace, write_flags, NULL)<0) SS_FAILED_WHEN("writing blob");
        }

        /* All tasks synchronize, flush data, and read. We need the barrier because ss_blob_flush() doesn't have a barrier. */
        if (ss_blob_synchronize(tscope_g, NULL)<0) SS_FAILED_WHEN("synchronizing independent writes");
        if (ss_blob_flush(tscope_g, blob_g[blob_idx], SS_STRICT, NULL)<0) SS_FAILED_WHEN("flushing independent writes");
        ss_mpi_barrier(SS_COMM_WORLD);
        if (tblob2_verify()) SS_FAILED_WHEN("verifying results");

        if (0==self) free(buffer);
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Multidimensional Asynchronous Blob Operations
 * Purpose:     Close the file
 *
 * Description: Closes the file
 *
 * Return:      Number of errors
 *
 * Parallel:    Collective across MPI_COMM_WORLD
 *
 * Programmer:  Robb Matzke
 *              Wednesday, October 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
tblob2_close(void)
{
    int         self, nerr=0;
    FILE        *_print=NULL;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    _print = 0==self ? stderr : NULL;

    SS_CHECKING("file close") {
        if (ss_file_close(file_g)<0) SS_FAILED_WHEN("closing file");
    } SS_END_CHECKING_WITH(nerr++);
    return nerr;
}

/*ARGSUSED*/
int
main(int UNUSED_SERIAL argc, char UNUSED_SERIAL *argv[])
{
    int         nerr=0, self, ntasks;
    int         minbufsize = 1024;
    int         maxaggtasks;
    ss_prop_t   *aggprops=NULL;

    /* Initialization */
#ifdef HAVE_PARALLEL
    MPI_Init(&argc, &argv);
#endif
    ss_init(SS_COMM_WORLD);
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    ntasks = ss_mpi_comm_size(SS_COMM_WORLD);

    /* Set the minimum aggregation buffer size as low as possible and the maximum number of aggregators as high as possible
     * (but leaving one non-aggregator) in order to stress test. */
    aggprops = ss_prop_new("two-phase I/O properties");
    ss_prop_add(aggprops, "minbufsize", H5T_NATIVE_INT, &minbufsize);
    maxaggtasks = MAX(1, ntasks-1);
    ss_prop_add(aggprops, "maxaggtasks", H5T_NATIVE_INT, &maxaggtasks);
    ss_blob_set_2pio(NULL, aggprops);
    ss_prop_dest(aggprops);
    if (0==self) {
        printf("info: set min aggregation buffer size to %s.\n", ss_bytes((hsize_t)minbufsize, NULL));
        printf("info: set max aggregators to %d of %d tasks.\n", maxaggtasks, ntasks);
    }
    
    /* Tests */
    nerr += tblob2_create();            /* create file, create and bind blobs */
    nerr += tblob2_write1();            /* write from one task */
    nerr += tblob2_write2();            /* write from one task repeatedly */
    nerr += tblob2_write3();            /* collective async write */
    nerr += tblob2_close();             /* close file */

    /* Finalization */
    ss_finalize();
    H5close();
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return nerr?1:0;
}
