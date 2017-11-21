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
#ifndef SS_HAVE_SSBLOB_H
#define SS_HAVE_SSBLOB_H

#define SS_BLOB_GRPNAME         "Blob-Storage"  /* Group name inside top-level scope to store blob datasets */
#define SS_BLOB_COLLECTIVE      0x00000001U     /* Data I/O is collective; lack of flag implies independent. */
#define SS_BLOB_UNBIND          0x00000002U     /* Unlink memory from blob after I/O request, even on failure. */
#define SS_BLOB_ASYNC           0x00000004U     /* I/O can be performed asynchronously. */
#define SS_BLOB_FREE            0x00000008U     /* Free caller-supplied buffer when no longer needed */
#define SS_BLOB_EXTEND          0x00000010U     /* The blob is extendible */
#define SS_BLOB_EACH            0x00000020U     /* ss_blob_mkstorage() binds each blob to point into one common dataset */
#define SS_BLOB_RANK            0x00000040U     /* ss_blob_mkstorage() assumes all tasks bound to same amount of memory */
#define SS_BLOB_COPY            0x00000080U     /* Make temp copy to hold data during async ss_blob_write() */

#define SS_BLOB_FLUSH_SEND      0x00000100U     /* During a flush, block until send operations complete for data shipping */
#define SS_BLOB_FLUSH_RECV      0x00000200U     /* During a flush, block until receive operations complete for data shipping */
#define SS_BLOB_FLUSH_WRITE     0x00000400U     /* During a flush, block until H5Dwrite() has completed */
#define SS_BLOB_FLUSH_SHIP      (SS_BLOB_FLUSH_SEND|SS_BLOB_FLUSH_RECV) /*flush data shipping*/
#define SS_BLOB_REAP_SEND       0x00000800U     /* During a flush, reap completed send operations */
#define SS_BLOB_REAP_RECV       0x00001000U     /* During a flush, reap completed receive operations */
#define SS_BLOB_REAP_WRITE      0x00002000U     /* During a flush, reap completed H5Dwrite() operations */
#define SS_BLOB_REAP_SHIP       (SS_BLOB_REAP_SEND|SS_BLOB_REAP_RECV) /*reap data shipping*/

#define SS_BLOB_2PIO_TAG        44              /* MPI message tag for two-phase I/O */


typedef struct ss_blobobj_tm {
    ss_persobj_t        pers;                   /* This must be first! */
    size_t              d_idx;                  /* Cached index into global blob table for the bound dataset, initialized
                                                 * usually by calling ss_blob_didx(). A value of zero could mean either that
                                                 * the d_idx value is not yet cached or the cached value is zero. A
                                                 * cached value is only valid if the `dsetaddr' member of the ss_blobobj_tf
                                                 * struct is also non-zero. */
    void                *mem;                   /* Ptr to memory if dataset is bound to memory. */
    hid_t               mtype;                  /* Duplicated memory datatype */
    hid_t               mspace;                 /* Duplicated memory data space */
} ss_blobobj_tm;

typedef struct ss_blobobj_tf {
    haddr_t             dsetaddr;               /* Address of the dataset in the same file as the blob */
    hsize_t             start[SS_MAXDIMS];      /* offset to start of hyperslab; dataset dimensionality; zero padded */
    hsize_t             count[SS_MAXDIMS];      /* number of blocks included in hyperslab; dataset dimensionality; zero padded */
} ss_blobobj_tf;

typedef struct ss_blob_2pio_t {
    size_t              minbufsize;             /* Minimum buffer size (bytes) per aggregation task */
    size_t              maxaggtasks;            /* Max number of aggregation tasks per dataset; 0 implies no 2-phase I/O */
    size_t              aggbuflimit;            /* Max bytes allocated for all aggregation buffers on this task */
    size_t              sendqueue;              /* Max number of outstanding buffers held by senders for 2-phase I/O */
    size_t              alignment;              /* Aggregator alignment (1 implies no alignment; zero is invalid) */
    htri_t              asynchdf5;              /* If true then try to use asynchronous H5Dwrite() */
    int                 aggbase;                /* If non-negative, defines rank of base aggregator */
    int                 tpn;                    /* If positive, defines tasks per node */
} ss_blob_2pio_t;

/* Certain blob operations are file collective even though one would expect them to be collective over the blob (i.e.,
 * collective over the scope in which the blob is defined):
 *   (1) creating storage -- because H5Dcreate() is collective.
 *   (2) binding a dataset -- for symmetry with creating storage and for consistency with internal data structures.
 *   (3) read and write -- when collectivity is requested because datasets can be shared by blobs
 *   (4) flush -- because 2-phase I/O may have moved data off the original task
 *
 * Each GFile record will maintain a list of all blob-related datasets within said file. */
typedef struct ss_gblob_t {
    ss_obj_t            obj;                    /* This must be first! A gblob is a kind of SSlib object */
    hid_t               storage;                /* Blob storage group */
    ss_blob_2pio_t      agg;                    /* Aggregation related info; properties are constant unless `a' is null */

    /* A list of all known blob datasets across all tasks of a file. */
    struct {
        hid_t           dset;                   /* A handle to the open dataset */
        H5G_stat_t      stat;                   /* Dataset information from H5Gget_objinfo() */
        hid_t           dtype;                  /* Needed because H5Dget_type() is file-collective */
        hid_t           dspace;                 /* Needed because H5Dget_space() is file-collective */
        hbool_t         is_extendible;          /* Is the dataset extendible? */
        struct {
            size_t      n;                      /* Number of aggregator tasks allowed for this dataset; nelmts in tasks[] */
            int         *tasks;                 /* Array of `n' aggregator task numbers w.r.t. the file communicator */
            size_t      elmts_per_agg;          /* Elements per aggregator */
            int         i_am_aggregator;        /* Non-negative index into `aggtask' if current task is listed in that array */
            void        *aggbuf;                /* Aggregation buffer for this dataset, will point somewhere into aggbuf_freeable */
            void        *aggbuf_freeable;       /* Memory allocated for `aggbuf' pointer; free this, not `aggbuf' */
            ss_aio_t    *aiocb;                 /* Control block for async H5Dwrite(). See ss_blob_async_flush() */
            size_t      recv_nused;             /* Number of outstanding async receives */
            size_t      recv_nalloc;            /* Number of elements allocated for `recv' array */
            MPI_Request *recv;                  /* Outstanding async receive requests */
            size_t      send_nused;             /* Number of outstanding async sends */
            size_t      send_nalloc;            /* Number of elements allocated for `send' array */
            MPI_Request *send;                  /* Outstanding async send requests */
            void        **sendbufs;             /* Send buffers that need to be freed */
            size_t      sendbufs_nused;         /* Number of buffers to be freed */
            size_t      sendbufs_nalloc;        /* Number of elements allocated for `sendbufs' array */
            hid_t       iom;                    /* Data space describing elements of the aggregation buffer for I/O */
            hid_t       iof;                    /* Data space describing where aggregation elements will land in the dataset */
        } agg;
    } *d;                                       /* Malloc'd array of dataset records */
    size_t              d_nused;                /* Number of initial elements of `d' that are used. */
    size_t              d_nalloc;               /* Number of elements allocated for `d' array */

    /* A list of I/O requests to be started at the next file-collective call -- data is local to a task. */
    struct ss_blob_async_t {
        size_t          d_idx;                  /* Index into `d' array for dataset on which to operate */
        hid_t           mtype;                  /* Memory data type */
        hid_t           iom;                    /* Memory data space extent and selection */
        hid_t           iof;                    /* File data space extent and selection */
        void            *buffer;                /* Memory buffer for the data, described by `iom' */
        unsigned        flags;                  /* Various SS_BLOB flags, most importantly SS_BLOB_FREE for the buffer */
    } *a;                                       /* Malloc'd array of async I/O requests not yet started */
    size_t              a_nused;                /* Number of initial elements of `a' that are used */
    size_t              a_nalloc;               /* Number of elements allocated for `a' array */
} ss_gblob_t;

typedef struct {
    int                 ndims;                  /* Stride dimensionality */
    hsize_t             offset;                 /* Byte offset to first element */
    hsize_t             duplicity;              /* Number of items each stride element refers to */
    hsize_t             stride[H5S_MAX_RANK];   /* Strides - i.e., amount to advance in each iteration of each dimension */
    hsize_t             count[H5S_MAX_RANK];    /* Counters for each stride dimension */
} ss_blob_stride_t;

#include "ssblobtab.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
herr_t ss_blobtab_init(void); /*to initialize machine generated stuff*/
herr_t ss_blob_init(void);
ss_blob_t *ss_blob_new(ss_scope_t *scope, unsigned flags, ss_blob_t *buf);
herr_t ss_blob_bind_m(ss_blob_t *blob, void *mem, hid_t mtype, hid_t mspace);
herr_t ss_blob_bind_m1(ss_blob_t *blob, void *mem, hid_t mtype, hsize_t nelmts);
herr_t ss_blob_bind_f(ss_blob_t *blob, hid_t dset, hid_t dspace, unsigned flags);
herr_t ss_blob_bind_f1(ss_blob_t *blob, hid_t dset, hsize_t offset, hsize_t size, unsigned flags);
void *ss_blob_bound_m(ss_blob_t *blob, hid_t *mtype, hid_t *mspace);
void *ss_blob_bound_m1(ss_blob_t *blob, hid_t *mtype, hsize_t *size);
int ss_blob_bound_f(ss_blob_t *blob, hid_t *dset, hsize_t *offsets, hsize_t *sizes, hid_t *fspace, hid_t *ftype);
herr_t ss_blob_bound_f1(ss_blob_t *blob, hid_t *dset, hsize_t *offset, hsize_t *size, hid_t *ftype);
int ss_blob_space(ss_blob_t *blob, hsize_t *size, hid_t *space);
herr_t ss_blob_mkstorage(ss_blob_t *blob, hsize_t *size, unsigned flags, ss_prop_t *props);
herr_t ss_blob_extend(ss_blob_t *blob, const hsize_t *size, unsigned flags, ss_prop_t *props);
herr_t ss_blob_extend1(ss_blob_t *blob, hsize_t size, unsigned flags, ss_prop_t *props);
void *ss_blob_read(ss_blob_t *blob, hid_t iospace, unsigned flags, ss_prop_t *props);
void *ss_blob_read1(ss_blob_t *blob, hsize_t offset, hsize_t nelmts, unsigned flags, ss_prop_t *props);
herr_t ss_blob_write(ss_blob_t *blob, hid_t iospace, unsigned flags, ss_prop_t *props);
herr_t ss_blob_write1(ss_blob_t *blob, hsize_t offset, hsize_t nelmts, unsigned flags, ss_prop_t *props);
herr_t ss_blob_synchronize(ss_scope_t *topscope, ss_prop_t *props);
herr_t ss_blob_flush(ss_scope_t *topscope, ss_blob_t *blob, unsigned flags, ss_prop_t *props);
herr_t ss_blob_set_2pio(ss_blob_t *blob, ss_prop_t *props);
ss_prop_t *ss_blob_get_2pio(ss_blob_t *blob, ss_prop_t *props);

/* The somewhat more private stuff */
ss_gblob_t *ss_blob_boot(ss_scope_t *topscope, hbool_t create);
herr_t ss_blob_desttab(ss_gblob_t *gblob);
herr_t ss_blob_boot_cb(hid_t group, const char *name, void *op_data);
int ss_blob_ckspace(hid_t space, int maxdims, hsize_t *size, hsize_t *start, hsize_t *count, hsize_t *nelmts);
herr_t ss_blob_ckspaces(ss_blob_t *blob, hid_t iospace, hid_t *iom, hid_t *iof);
hid_t ss_blob_normalize(hid_t space);
herr_t ss_blob_stride(int ndims, const hsize_t *dset_size, const hsize_t *slab_start, const hsize_t *slab_count,
                      ss_blob_stride_t *stride);
herr_t ss_blob_stride_1(hsize_t start, hsize_t count, ss_blob_stride_t *s);
herr_t ss_blob_stride_copy(void *dst, const ss_blob_stride_t *dst_stride, const void *src, const ss_blob_stride_t *src_stride,
                           size_t elmt_size);
hsize_t ss_blob_array_linear(int ndims, const hsize_t *size, const hsize_t *start, const hsize_t *count);
herr_t ss_blob_array_multidim(int ndims, const hsize_t *size, hsize_t linear, hsize_t *multidim);
int ss_blob_async_intersect(ss_gblob_t *gblob, size_t d_idx, int aggtask, const hsize_t *dset_size,
                            const hsize_t *req_dstart, const hsize_t *mem_size, const hsize_t *req_mstart,
                            const hsize_t *req_count, ss_blob_stride_t *req_strides, ss_blob_stride_t *snd_strides);
int ss_blob_async_sort_cb(const void *_a, const void *_b);
herr_t ss_blob_async_append(ss_gblob_t *gblob, size_t d_idx, hid_t mtype, hid_t iom, hid_t iof, void *buffer, unsigned flags);
void *ss_blob_async_buffer(ss_gblob_t *gblob, size_t d_idx);
herr_t ss_blob_async_prune(size_t aggbufsize, size_t sendbufsize);
herr_t ss_blob_async_flush_all(ss_gblob_t *gblob, size_t d_idx, hid_t dxpl, unsigned flags);
herr_t ss_blob_async_flush(ss_gblob_t *gblob, size_t d_idx, hid_t dxpl, unsigned flags);
size_t ss_blob_didx(ss_blob_t *blob);
ss_prop_t *ss_blob_init_2pio(ss_prop_t *props, const char *s);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSBLOB_H*/
