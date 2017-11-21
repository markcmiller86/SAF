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
SS_IF(blob);

static int ss_eq_nos(unsigned long const a[2], unsigned long const b[2])
{
    return (a[0]==b[0] && a[1]==b[1]);
}

static hsize_t ss_blob_unlim_g[H5S_MAX_RANK];
static ss_gblob_t *ss_blob_async_sort_g;                /* passes a gblob to the qsort() callback */
static ss_blob_2pio_t ss_blob_2pio_g;                   /* library-wide two-phase I/O properties initialized by ss_init() */
size_t ss_blob_aggbuf_total_g;                          /* Total *bytes* used for aggregation buffers for this task */
size_t ss_blob_sendbuf_total_g;                         /* Total freeable *buffers* allocated for sending */

/* Private two-phase I/O functions */
#ifdef HAVE_PARALLEL
static herr_t ss_blob_async_receives(ss_gblob_t *gblob, size_t d_idx, int srctask, MPI_Comm filecomm, int nstrides,
                                     const ss_blob_stride_t *s);
static herr_t ss_blob_async_sends(ss_gblob_t *gblob, size_t d_idx, int aggtask, MPI_Comm filecomm, int nstrides,
                                  const ss_blob_stride_t *s, char *buffer, unsigned flags);
#endif
static herr_t ss_blob_async_aggregators(MPI_Comm filecomm, ss_gblob_t *gblob, size_t d_idx, int ndims, hsize_t *dset_size);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Note:        Aggregation Properties
 *
 * Description: These are properties that have to do with two-phase I/O performed in SSlib for raw data when I/O operations
 *              are collective. See ss_blob_set_2pio() for details.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Note:        Blob Properties
 *
 * Description: These properties control how certain blob operations work.
 *
 *              * !dcpl:        An HDF5 dataset creation property list to be used by ss_blob_mkstorage() when creating
 *                              a new dataset.
 *
 *              * !dset:        If specified and positive then only blobs stored on this dataset
 *                              are synchronized in a call to ss_blob_synchronize().
 *
 *              * !name:        If supplied, then ss_blob_mkstorage() will create the dataset with the specified name
 *                              in addition to linking it into the "Blob-Storage" group as usual.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
                        
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 *
 * Description: The functions of this programming interface deal with I/O to and from an HDF5 file. These are mostly for
 *              non-table data such as application data for fields and relations. SSlib needs to support the following goals:
 *
 *              * *Targeting*: Data can be targeted for a specific architecture during write operations that create a new
 *                dataset. This is useful when data is created on one architecture and will be read repeatedly on another
 *                architecture, allowing the data conversion price to be paid just once.
 *
 *              * *Precision*: The data precision can be changed during read/write operations by requesting that the size
 *                of datatypes in the file are different than those in memory. For example, when writing a plot file the
 *                caller might supply !double values but desire them to be written as !float values.
 *
 *              * *Task-Aggregation*: When many tasks are contributing non-overlapping data for a single HDF5 dataset
 *                then it may be advantageous to do some message passing in order to aggregate the data to a smaller subset of
 *                tasks where each one can contribute a larger aligned block of data to the file system.
 *
 *              * *Field-Aggregation*: Distinct fields and/or relations might want to share a single dataset in a
 *                non-overlapping manner in order to improve I/O performance. It should be up to the application how to
 *                organized the data in the dataset.
 *
 *              * *Sharing*: Two or more fields or relations should be able to point to a common dataset if those fields or
 *                relations truly reference common data. Changing the data for one field or relation will also change the data
 *                for the other fields or relations.
 *
 *              * *Cross-file*: We should be able to store raw data in an SSlib database other than the one holding the
 *                relation or field.
 *
 *              * *Prewritten*: It should be possible to point to field or relation data that was already written to a file by
 *                the client.
 *
 *              In order to get all this to work, SSlib relies heavily on HDF5 support and therefore exposes the HDF5 API to
 *              the SSlib client.  This allows the client to make full use of HDF5 capabilities, but in many cases the client
 *              would rather just let SSlib take care of all the storage details. These two competing design goals are handled
 *              by SSlib /blob/ persistent objects (not to be confused with the old VBT blobs which served a similar but much
 *              simpler purpose).
 *
 *              A blob points to either a buffer in memory or part of a dataset in a file or both. When pointing to a dataset,
 *              the dataset must always be in the same file as the blob itself.  An object such as a field in one SSlib file
 *              can store raw data in some other SSlib file by linking to a blob defined in that second file.  All blob
 *              datasets have names in the blob storage group of the top-level scope, and the names are the decimal
 *              representation of the dataset object header address.  This accomplishes three goals: (1) any blob dataset can
 *              be referred to with a single haddr_t value, (2) unique dataset names can be created with no communication, and
 *              (3) all blob datasets can be discovered with just a couple HDF5 calls.
 *
 *              SSlib allows blobs to share datasets and the shared dataset. The dimensionality of a blob may be less than the
 *              dimensionality of the dataset in which it lives allowing, for instance, one-dimensional blobs to be overlayed
 *              as rows of a two-dimensional dataset. See ss_blob_bind_f() and ss_blob_space() for details.
 * 
 *              Since dataset creation and opening in HDF5 is an operation that is collective across the file communicator,
 *              many blob operations are also collective across that communicator.
 *
 *              Blobs cannot be associated with a transient scope since there is no underlying HDF5 file in which to store the
 *              raw data.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/* Global blob table -- see ssblob.h */
ss_gblob_t      *ss_gblob_g=NULL;
size_t          ss_gblob_nused_g=0;
size_t          ss_gblob_nalloc_g=0;

#if 0 /*RPM DEBUGGING 2004-12-09*/
/**/
static void
ss_blob_dump_sh(int signo)
{
    size_t gfile_idx, d_idx, nsends=0, nrecvs=0, nwrites=0;
    ss_gfile_t *gfile;
    for (gfile_idx=0; (gfile=SS_GFILE_IDX(gfile_idx)); gfile_idx++) {
        if (!gfile->cur_open) continue;
        for (d_idx=0; d_idx<gfile->gblob->d_nused; d_idx++) {
            nsends += gfile->gblob->d[d_idx].agg.send_nused;
            nrecvs += gfile->gblob->d[d_idx].agg.recv_nused;
            nwrites += gfile->gblob->d[d_idx].agg.aiocb ? 1 : 0;
        }
    }

    fprintf(sslib_g.warnings, "SSlib-%d: 2pio nsends=%lu, nrecvs=%lu, nwrites=%lu\n",
            ss_mpi_comm_rank(SS_COMM_WORLD), (unsigned long)nsends, (unsigned long)nrecvs, (unsigned long)nwrites);
}
#endif

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Interface initializer
 *
 * Description: Initializes the SSlib blob I/O interface.
 *
 * Return:      Returns non-negative on success, negative on failure
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_init(void)
{
    int         i;
    
    SS_ENTER_INIT;

    /* Initialize ss_blob_unlim_g */
    for (i=0; i<H5S_MAX_RANK; i++)
        ss_blob_unlim_g[i] = H5S_UNLIMITED;

    /* Initialize table datatypes */
    if (ss_blobtab_init()<0) SS_ERROR(INIT);

#if 0 /*RPM DEBUGGING 2004-12-09*/
    signal(SIGINT, ss_blob_dump_sh);
#endif

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Create a new blob
 *
 * Description: This is mostly a convenience function for ss_pers_new() that creates a new blob object in the specified scope.
 *
 * Return:      Returns a non-null pointer on success, either BUF if supplied or a newly allocated blob link; returns null on
 *              failure.
 *
 * Parallel:    Same semantics as ss_pers_new().
 *
 * Programmer:  Robb Matzke
 *              Monday, June 21, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_blob_t *
ss_blob_new(ss_scope_t *scope,                  /* Scope where the new blob will be created */
            unsigned flags,                     /* Flags such as SS_ALLSAME */
            ss_blob_t *buf                      /* OUT: Optional buffer to receive blob link */
            )
{
    SS_ENTER(ss_blob_new, ss_blob_tP);
    ss_blob_t   *retval=NULL;

    if (NULL==(retval=(ss_blob_t*)ss_pers_new(scope, SS_MAGIC(ss_blob_t), NULL, flags, (ss_pers_t*)buf, NULL))) SS_ERROR(FAILED);
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Bind a blob to memory
 *
 * Description: This function binds a blob to a memory buffer that will be used for I/O.  This is used when data is output
 *              from memory to the file and MTYPE and MSPACE describe the elements of memory to be written to the file. The
 *              memory is not copied by this function and so must not be freed by the caller until it is unbound from the
 *              blob.
 *
 *              A blob can be disassociated from memory by supplying a null pointer for the MEM argument, in which case the
 *              following arguments are ignored.  It is the caller's responsibility to deallocate any memory if appropriate.
 *              The memory can also be disassociated automatically after an ss_blob_read(), ss_blob_read1(), ss_blob_write(),
 *              or ss_blob_write1() call if the SS_BLOB_UNBIND bit is passed in the !flags argument to those functions.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_bind_m(ss_blob_t *blob,                 /* The blob to which memory is associated */
               void *mem,                       /* The memory to which the blob will now point */
               hid_t mtype,                     /* The datatype of each element of the memory. This datatype will be copied
                                                 * into the blob so the caller is free to close the handle at any time. */
               hid_t mspace                     /* The extent of memory and a selection of elements in that memory. This data
                                                 * space will be copied into the blob, so the caller should  close its handle. */
               )
{
    SS_ENTER(ss_blob_bind_m, herr_t);
    hid_t       hid;
    
    SS_ASSERT_MEM(blob, ss_blob_t);

    /* Unbind */
    SS_BLOB(blob)->m.mem = NULL;
    if (SS_BLOB(blob)->m.mtype>0) {
        if (H5Tclose(SS_BLOB(blob)->m.mtype)<0) SS_ERROR(HDF5);
        SS_BLOB(blob)->m.mtype = 0;
    }
    if (SS_BLOB(blob)->m.mspace>0) {
        if (H5Sclose(SS_BLOB(blob)->m.mspace)<0) SS_ERROR(HDF5);
        SS_BLOB(blob)->m.mspace = 0;
    }

    /* Bind */
    if (mem) {
        if (ss_blob_ckspace(mspace, SS_MAXDIMS, NULL, NULL, NULL, NULL)<0) SS_ERROR(FAILED);
        SS_BLOB(blob)->m.mem = mem;
        if ((hid=H5Tcopy(mtype))<0) SS_ERROR(HDF5);
        SS_BLOB(blob)->m.mtype = hid;
        if ((hid=ss_blob_normalize(mspace))<0) SS_ERROR(FAILED);
        SS_BLOB(blob)->m.mspace = hid;
    }
    
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Bind a blob to memory
 *
 * Description: This is a special case of ss_blob_bind_m() that binds one-dimensional memory to a blob. It's slightly easier
 *              to use because the caller isn't required to build an HDF5 data space to describe the memory layout.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_bind_m1(ss_blob_t *blob,                /* The blob to which memory is associated. */
                void *mem,                      /* The memory to be associated with the blob. */
                hid_t mtype,                    /* The datatype of each element of the memory. This datatype will be copied
                                                 * into the blob so the caller is free to close the handle at any time. */
                hsize_t nelmts                  /* Number of elements pointed to by MEM. */
                )
{
    SS_ENTER(ss_blob_bind_m1, herr_t);
    hid_t       mspace=-1;

    /* We have to use unlimited dimensions here because nelmts might be zero and HDF5 won't allow that */
    if (mem && (mspace=H5Screate_simple(1, &nelmts, ss_blob_unlim_g))<0) SS_ERROR(HDF5);
    if (ss_blob_bind_m(blob, mem, mtype, mspace)<0) SS_ERROR(FAILED);
    if (mem && H5Sclose(mspace)<0) SS_ERROR(HDF5);
    mspace = -1;

 SS_CLEANUP:
    if (mspace>0) H5Sclose(mspace);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Bind a blob to a dataset
 *
 * Description: If a dataset already exists and a new blob is being created then an existing dataset can be bound to the blob.
 *              In effect, this causes the blob to own part of the dataset. Multiple blobs can point into a common dataset and
 *              those blobs can own overlapping regions of the dataset. This function duplicates the dataset handle so the
 *              caller can close its handle without affecting SSlib.
 *
 *              This function is only applicable when initializing or modifying a blob since all blobs that are read from a
 *              file are automatically bound to the same dataset and region as previously specified. That is, information
 *              about the dataset and region are stored as part of the blob object in the file.
 *
 *              A blob can be disassociated from a dataset by providing a non-positive handle for the DSET argument, in which
 *              case the remaining arguments are ignored.
 *
 *              The multi-dimensional size of the blob is deteremined from the selection defined on DSPACE. The dimensionality
 *              of the blob is determined by removing all the dimensions that have unit size in the blob. Thus a blob's
 *              dimensionality may be less than the dimensionality of the dataset on which it is defined.  This allows, for
 *              instance, one dimensional blobs to be stored as rows of a two dimensional dataset. See ss_blob_space() for
 *              additional information.
 *
 *              A blob's size can be changed simply by rebinding it to a dataset using a new data space. If the the
 *              SS_BLOB_EXTEND bit is turned on in the FLAGS argument then the underlying dataset is extended if necessary in
 *              order for the blob to fit within the dataset. All calling tasks (even those that don't have a blob) must pass
 *              the same value for the SS_BLOB_EXTEND bit.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Conceptually collective across the blob's scope communicator, however in practice the file communicator must
 *              be used. This is due to the fact that SSlib might need to make file-collective calls to HDF5. The restriction
 *              isn't so bad though because the caller would have had to file-collectively create the dataset anyway.
 *
 *              All tasks should pass the same blob and dataset, but any "extra" task that's part of the blob's file
 *              communicator but not the blob's scope communicator should pass the top scope for the blob's file instead of
 *              the blob.  The DSPACE argument is ignored on the "extra" tasks but should be consistent across the blob's tasks.
 *
 * Issue:       Currently only very simple selections are allowed: a contiguous multi-dimensional rectangular region of an
 *              extent specified by arrays of offsets and lengths, one per dimension.
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_bind_f(ss_blob_t *blob,                 /* The blob to which a dataset is associated. */
               hid_t dset,                      /* The dataset to associate with the blob. The handle is duplicated by SSlib,
                                                 * allowing the caller to close its handle at any time. */
               hid_t dspace,                    /* A selection that describes which elements of DSET are owned by the blob. If
                                                 * non-positive then the entire dataset is owned by the blob. */
               unsigned flags                   /* Various bit flags affecting the operation of this function. */
               )
{
    SS_ENTER(ss_blob_bind_f, herr_t);
    ss_scope_t          topscope=SS_SCOPE_NULL; /* The top scope for the blob (obtained from blob or passed as BLOB argument) */
    ss_gblob_t          *gblob=NULL;            /* The gfile->gblob pointer for convenience */
    size_t              d_idx=SS_NOSIZE;        /* Index into gblob->d table */
    H5G_stat_t          sb;                     /* Stat buffer for the supplied dataset handle */
    H5G_stat_t          sb_link;                /* Stat buffer for dataset in blob storage directory */
    hid_t               dset_duped=-1;          /* Dataset handle duplicated by this function to be closed on error*/
    hid_t               dtype_duped=-1;         /* Dataset type opened in this function to be closed on error */
    hid_t               dspace_duped=-1;        /* Dataset space opened in this function to be closed on error */
    char                dsetname[64];           /* Dataset name as it appears in blob storage group */
    hbool_t             new_entry=FALSE;        /* Are we appending a new entry to the gblob table? */
    hid_t               dcpl=-1;                /* Dataset creation property list */
    int                 ndims=-1;               /* Dataset dimensionality */
    hsize_t             dsize[H5S_MAX_RANK];    /* Current size of DSET */
    hsize_t             newsize[H5S_MAX_RANK];  /* New required size of the dataset */
    int                 root;                   /* Rank of blob task zero in the file communicator */
    int                 self;                   /* Rank of the calling task in the file communicator */
    MPI_Comm            filecomm=SS_COMM_NULL;  /* The file communicator */
    htri_t              extendible=-1;          /* Is the dataset extendible? */
    int                 i;

    /* Communicator related stuff */
    if (!ss_mpi_extras((ss_pers_t**)&blob, &topscope)) SS_ERROR(FAILED);
    if (blob) SS_ASSERT_MEM(blob, ss_blob_t);
    else dspace = -1;
    if (ss_scope_comm(&topscope, &filecomm, &self, NULL)<0) SS_ERROR(FAILED);

    if (dset<=0) SS_ERROR_FMT(USAGE, ("no dataset handle"));
    if (H5Gget_objinfo(dset, ".", FALSE, &sb)<0) SS_ERROR(FAILED);
    gblob = SS_GFILE_LINK(&topscope)->gblob;

    /* Mark the blob as dirty here near the beginning of the function so we don't have to worry about it anymore. This makes
     * it easy if an error occurs half way through the function. No need to mark as unsynchronized since a precondition is
     * that we're being called collectively with all tasks having compatible arguments. */
    if (blob) SS_BLOB(blob)->m.pers.dirty = TRUE;

    /* Unbind any old dataset from the blob. We do this fairly early because we'd like to have it unbound if there was an
     * error in binding. Do not unbind memory. */
    if (blob && SS_BLOB(blob)->dsetaddr) {
        if (SS_NOSIZE==(d_idx=ss_blob_didx(blob))) SS_ERROR(FAILED);
        SS_ASSERT(ss_eq_nos(gblob->d[d_idx].stat.objno,sb.objno));
        SS_BLOB(blob)->dsetaddr = 0;
        memset(SS_BLOB(blob)->start, 0, sizeof(SS_BLOB(blob)->start));
        memset(SS_BLOB(blob)->count, 0, sizeof(SS_BLOB(blob)->count));
    }

    /* Make sure that the data space is suitable. Save the selection starts and counts in the blob object. */
    memset(SS_BLOB(blob)->start, 0, sizeof(SS_BLOB(blob)->start));
    memset(SS_BLOB(blob)->count, 0, sizeof(SS_BLOB(blob)->count));
    if (blob && (ndims=ss_blob_ckspace(dspace, SS_MAXDIMS, NULL, SS_BLOB(blob)->start, SS_BLOB(blob)->count, NULL))<0)
        SS_ERROR(FAILED);

    /* Is the dataset extendible? */
    if ((dcpl=H5Dget_create_plist(dset))<0) SS_ERROR(HDF5);
    extendible = H5D_CHUNKED == H5Pget_layout(dcpl);
    if (H5Pclose(dcpl)<0) SS_ERROR(HDF5);
    dcpl = -1;

    /* If the SS_BLOB_EXTENDIBLE flag is set then make sure the dataset is extendible */
    if ((flags & SS_BLOB_EXTEND) && !extendible)
        SS_ERROR_FMT(HDF5, ("SS_BLOB_EXTENDIBLE set but dataset is not extendible"));
    
    /* Every task allocates a record to the gblob table for the file, or updates a record if the dataset is already there
     * because some other blob is (or was) bound to it. */
    for (d_idx=0; d_idx<gblob->d_nused; d_idx++) {
        if (ss_eq_nos(gblob->d[d_idx].stat.objno,sb.objno) && ss_eq_nos(gblob->d[d_idx].stat.fileno,sb.fileno)) {
            break;
        }
    }
    if (d_idx >= gblob->d_nused) {
        SS_EXTEND(gblob->d, MAX(64,gblob->d_nused+1), gblob->d_nalloc);
        d_idx = gblob->d_nused; /*increment on success*/
        memset(gblob->d + d_idx, 0, sizeof(gblob->d[0]));
        new_entry = TRUE;
    }

    /* If the dataset was not already in the gblob table then make sure the dataset has a name in the blob storage group and
     * duplicate the dataset handle. Also obtain the datatype (see ss_blob_boot_cb() for reason). */
    if (gblob->d[d_idx].dset <= 0) {
        gblob->d[d_idx].stat = sb;
        SS_ASSERT(gblob->storage>0);
        /*sprintf(dsetname, "%lu", (unsigned long)(sb.objno));*/
        sprintf(dsetname, "%lu", (unsigned long)sb.objno[0]);
        if (H5Gget_objinfo(gblob->storage, dsetname, FALSE, &sb_link)>=0) {
            if (!ss_eq_nos(sb.objno,sb_link.objno)) SS_ERROR_FMT(CORRUPT, ("mis-linked blob storage for `%s'", dsetname));
        } else {
            if (H5Glink2(dset, ".", H5G_LINK_HARD, gblob->storage, dsetname)<0) SS_ERROR(HDF5);
        }
        if ((gblob->d[d_idx].dset = dset_duped = H5Dopen(dset, "."))<0) SS_ERROR(HDF5);
        if ((gblob->d[d_idx].dtype = dtype_duped = H5Dget_type(dset_duped))<0) SS_ERROR(HDF5);
        if ((gblob->d[d_idx].dspace = dspace_duped = H5Dget_space(dset_duped))<0) SS_ERROR(HDF5);
        gblob->d[d_idx].is_extendible = extendible;
    }

    /* If the SS_BLOB_EXTEND bit is set then extend the dataset if necessary so that the blob fits inside it. We can only make
     * the determination on the blob tasks but all file tasks need to know the result in order to call H5Dextend(). To further
     * complicate matters, the non-blob tasks don't know which task is the blob's task zero. */
    if (flags & SS_BLOB_EXTEND) {
        if ((ndims=ss_blob_ckspace(gblob->d[d_idx].dspace, SS_MAXDIMS, dsize, NULL, NULL, NULL))<0) SS_ERROR(FAILED);
        if ((root=ss_mpi_elect(blob?(ss_pers_t*)blob:(ss_pers_t*)&topscope))<0) SS_ERROR(FAILED);
        if (blob && root==self) {
            for (i=0; i<ndims; i++) {
                newsize[i] = MAX(dsize[i], SS_BLOB(blob)->start[i]+SS_BLOB(blob)->count[i]);
            }
        }
        if (ss_mpi_bcast(newsize, (size_t)ndims, MPI_HSIZE_T, root, filecomm)<0) SS_ERROR(FAILED);
        for (i=0; i<ndims; i++) {
            if (newsize[i]>dsize[i]) {
                if (H5Dextend(gblob->d[d_idx].dset, newsize)<0) SS_ERROR(HDF5);
                if (H5Sclose(gblob->d[d_idx].dspace)<0) SS_ERROR(HDF5);
                if ((gblob->d[d_idx].dspace=H5Dget_space(gblob->d[d_idx].dset))<0) SS_ERROR(HDF5);
                if (dspace_duped>0) dspace_duped = gblob->d[d_idx].dspace;
                if (ss_blob_async_aggregators(filecomm, gblob, d_idx, ndims, newsize)<0) SS_ERROR(FAILED);
                break;
            }
        }
    }


    /* Final adjustments to the blob */
    if (blob) {
        SS_ASSERT(ss_eq_nos(sb.objno,gblob->d[d_idx].stat.objno));
        SS_BLOB(blob)->dsetaddr = *((haddr_t*)sb.objno);
        SS_BLOB(blob)->m.d_idx = d_idx;
    }

    /* Adjust counters just before successful return */
    if (new_entry) gblob->d_nused++;

 SS_CLEANUP:
    if (dset_duped>0) H5Dclose(dset_duped);
    if (dtype_duped>0) H5Tclose(dtype_duped);
    if (dspace_duped>0) H5Sclose(dspace_duped);
    if (dcpl>0) H5Pclose(dcpl);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Bind a blob to a dataset
 *
 * Description: This is a convenience function for ss_blob_bind_f() that can be used when the dataset is one dimensional and
 *              the region of the dataset owned by the blob is contiguous.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    See ss_blob_bind_f().
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_bind_f1(ss_blob_t *blob,                /* The blob to which a dataset is associated. */
                hid_t dset,                     /* The dataset to associate with the blob. The handle is duplicated by SSlib,
                                                 * allowing the caller to close its handle at any time. */
                hsize_t offset,                 /* Offset into the array for the region of the dataset owned by this blob. */
                hsize_t size,                   /* Number of elements owned by this blob. */
                unsigned flags                  /* Bit flags to pass down to ss_blob_bind_f() */
                )
{
    SS_ENTER(ss_blob_bind_f1, herr_t);
    hid_t               dspace=-1;
    int                 ndims;

    if ((dspace=H5Dget_space(dset))<0) SS_ERROR(HDF5);
    if ((ndims=H5Sget_simple_extent_ndims(dspace))<0) SS_ERROR(HDF5);
    if (ndims!=1 && ndims!=0) SS_ERROR_FMT(USAGE, ("dataset is not one-dimensional or scalar"));

    if (SS_MAGIC(ss_scope_t)==SS_MAGIC_OF(blob)) {
        /* This task is in the file communicator but not the scope communicator */
        if (H5Sselect_none(dspace)<0) SS_ERROR(HDF5);
    } else {
        if (H5Sselect_slab(dspace, H5S_SELECT_SET, (hsize_t)0, &offset, &size)<0) SS_ERROR(HDF5);
    }
    if (ss_blob_bind_f(blob, dset, dspace, flags)<0) SS_ERROR(FAILED);
    if (H5Sclose(dspace)<0) SS_ERROR(HDF5);
    dspace = -1;

 SS_CLEANUP:
    if (dspace>0) H5Sclose(dspace);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Query memory bound to a blob
 *
 * Description: This function queries information about the memory that is bound to the blob through the ss_blob_bind_m()
 *              function.
 *
 * Return:      Returns a pointer to memory (the same pointer as passed to ss_blob_bind_m()) bound to the blob, and also
 *              returns copies of the memory datatype and data space through the optional MTYPE and MSPACE pointer arguments.
 *              Returns the null pointer on failure.  It is considered a failure to ask for memory information for a blob that
 *              is not bound to memory.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_blob_bound_m(ss_blob_t *blob,                /* Blob to query. */
                hid_t *mtype,                   /* OUT: Optional pointer for memory datatype, duplicated from that stored in
                                                 * the blob. */
                hid_t *mspace                   /* OUT: Optional pointer for memory dataspace (extent and selection),
                                                 * duplicated from that stored in the blob. */
                )
{
    SS_ENTER(ss_blob_bound_m, voidP);
    void        *retval=NULL;
    hid_t       type=-1;

    SS_ASSERT_MEM(blob, ss_blob_t);

    if (NULL==(retval=SS_BLOB(blob)->m.mem)) SS_ERROR_FMT(USAGE, ("blob is not bound to memory"));
    if (mtype && (*mtype = type = H5Tcopy(SS_BLOB(blob)->m.mtype))<0) SS_ERROR(HDF5);
    if (mspace && (*mspace = H5Scopy(SS_BLOB(blob)->m.mspace))<0) SS_ERROR(HDF5);

 SS_CLEANUP:
    if (type>0) H5Tclose(type);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Query memory bound to a blob
 *
 * Description: This is a convenience function for ss_blob_bound_m() when the memory bound to the blob is one dimensional, has
 *              a zero starting offset, and is contiguous. It returns an offset instead of an HDF5 data space.
 *
 * Return:      Returns a pointer to memory (the same pointer as passed to ss_blob_bind_m()) bound to the blob, and also
 *              returns a copy of the memory datatype and the number of consecutive elements contained in that memory. A null
 *              pointer is returned on failure. It is considered a failure to ask for memory information for a blob that is
 *              not bound to memory, or to use this function to query bound memory that cannot be described with simply a size.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_blob_bound_m1(ss_blob_t *blob,               /* Blob to query. */
                 hid_t *mtype,                  /* OUT: Optional pointer for memory datatype, duplicated from that stored in
                                                 * the blob. */
                 hsize_t *size                  /* OUT: Optional pointer to receive the number of elements pointed to by the
                                                 * memory bound to the blob. The extent and selection must be identical. */
                 )
{
    SS_ENTER(ss_blob_bound_m1, voidP);
    void        *retval=NULL;
    hid_t       type=-1;
    int         ndims;
    hsize_t     start, count;

    SS_ASSERT_MEM(blob, ss_blob_t);

    if (NULL==(retval=SS_BLOB(blob)->m.mem)) SS_ERROR_FMT(USAGE, ("blob is not bound to memory"));
    if (mtype && (*mtype = type = H5Tcopy(SS_BLOB(blob)->m.mtype))<0) SS_ERROR(HDF5);

    if (size) {
        if ((ndims=ss_blob_ckspace(SS_BLOB(blob)->m.mspace, 1, size, &start, &count, NULL))<0) SS_ERROR(FAILED);
        if (0==ndims) {
            *size = 1;
        } else {
            if (start) SS_ERROR_FMT(USAGE, ("memory selection has non-zero offset"));
            if (count!=*size) SS_ERROR_FMT(USAGE, ("memory selection is not all elements of extent"));
        }
    }
    
 SS_CLEANUP:
    if (type>0) H5Tclose(type);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Query dataset bound to a blob
 *
 * Description: A blob can be bound to (part of) a dataset and a call to this function will return the dataset and description
 *              of elements to which it is bound.
 *
 * Return:      On success, returns the dimensionality of the blob's dataset. The returned value indicates the number of
 *              elements of the optional OFFSETS and SIZES arrays that were initialized. A return value of zero indicates a
 *              scalar dataset. On failure, a negative value is returned.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_blob_bound_f(ss_blob_t *blob,                /* Blob to query. */
                hid_t *dset,                    /* OUT: Optional returned dataset handle. The handle is not duplicated and the
                                                 * caller should not close it. This is due to the fact that the handle
                                                 * duplication function (H5Dopen()) in HDF5 is file collective but this SSlib
                                                 * function is not. */
                hsize_t *offsets,               /* OUT: Optional offset per dataset dimension for the starting position of the
                                                 * data owned by this blob. The array should be large enough to hold SS_MAXDIMS
                                                 * offsets. */
                hsize_t *sizes,                 /* OUT: Optional size per dataset dimension for the portion of data owned by this
                                                 * blob. The array should be large enough to hold SS_MAXDIMS offsets. */
                hid_t *fspace,                  /* OUT: Optional data space to be returned. This is the data space of the
                                                 * underlying dataset with a selection representing the blob's portion of that
                                                 * data space.  The caller should close this data space when no longer needed. */
                hid_t *ftype                    /* OUT: Optional file datatype to be returned. The caller should close this
                                                 * datatype handle when no longer needed. */                                       
                )
{
    SS_ENTER(ss_blob_bound_f, int);
    ss_gblob_t  *gblob=NULL;                    /* The gblob table associated with the file owning BLOB */
    size_t      d_idx=SS_NOSIZE;                /* An index into the gblob->d table */
    hid_t       fspace_duped=-1;                /* Duplicated data space to be closed on error */
    hid_t       ftype_duped=-1;                 /* Duplicated datatype to be closed on error */
    int         ndims=0;                        /* Number of dimensions; dimension counter; return value */
    int         i;
    
    SS_ASSERT_MEM(blob, ss_blob_t);
    if (!SS_BLOB(blob)->dsetaddr) SS_ERROR_FMT(USAGE, ("blob is not bound to a dataset"));
    if (SS_NOSIZE==(d_idx=ss_blob_didx(blob))) SS_ERROR(FAILED);
    gblob = SS_GFILE_LINK(blob)->gblob;
    SS_ASSERT(gblob);
    SS_ASSERT(SS_BLOB(blob)->dsetaddr==*((haddr_t*)&(gblob->d[d_idx].stat.objno)[0]));

    /* Dataset */
    if (dset) *dset = gblob->d[d_idx].dset;

    /* Offsets and/or sizes and the return value */
    if ((ndims=H5Sget_simple_extent_ndims(gblob->d[d_idx].dspace))<0) SS_ERROR(HDF5);
    for (i=0; i<ndims; i++) {
        if (offsets) offsets[i] = SS_BLOB(blob)->start[i];
        if (sizes) sizes[i] = SS_BLOB(blob)->count[i];
    }

    /* Data space and selection */
    if (fspace) {
        if ((*fspace = fspace_duped = H5Scopy(gblob->d[d_idx].dspace))<0) SS_ERROR(HDF5);
        if (H5Sselect_slab(*fspace, H5S_SELECT_SET, (hsize_t)0, SS_BLOB(blob)->start, SS_BLOB(blob)->count)<0)
            SS_ERROR(HDF5);
    }

    /* Datatype */
    if (ftype && (*ftype = ftype_duped = H5Tcopy(gblob->d[d_idx].dtype))<0) SS_ERROR(HDF5);

 SS_CLEANUP:
    if (fspace_duped>0) H5Sclose(fspace_duped);
    if (ftype_duped>0) H5Tclose(ftype_duped);
    SS_LEAVE(ndims);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Query dataset bound to a blob
 *
 * Description: This is a convenience function for ss_blob_bound_f() when the blob data is scalar or one dimensional and
 *              contiguous in the dataset. Unlike ss_blob_bound_f(), a scalar blob will be treated as a one-dimensional blob
 *              of size one in order that OFFSET and SIZE are always returned.
 *
 * Return:      Returns non-negative on success; negative on failure.  It is considered a failure to make this query when no
 *              dataset is bound to the blob or the blob's region of the dataset doesn't satisfy the constraints set out
 *              above.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_bound_f1(ss_blob_t *blob,               /* Blob to query. */
                 hid_t *dset,                   /* OUT: Optional returned dataset handle. The handle is not duplicated and the
                                                 * caller should not close it. */
                 hsize_t *offset,               /* OUT: Optional starting offset for the data owned by BLOB. */
                 hsize_t *size,                 /* OUT: Optional number of elements owned by BLOB. */
                 hid_t *ftype                   /* OUT: Optional datatype of the underlying dataset. The caller should close this
                                                 * datatype handle when no longer needed. */
                 )
{
    SS_ENTER(ss_blob_bound_f1, herr_t);
    hsize_t     offsets[SS_MAXDIMS], sizes[SS_MAXDIMS];
    int         ndims;

    if ((ndims=ss_blob_bound_f(blob, dset, offsets, sizes, NULL, ftype))<0) SS_ERROR(FAILED);
    if (ndims>1) SS_ERROR_FMT(USAGE, ("dataset is multi-dimensional"));
    if (0==ndims) {
        if (offset) *offset = 0;
        if (size) *size = 0;
    } else {
        if (offset) *offset = offsets[0];
        if (size) *size = sizes[0];
    }
    
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Query blob extent
 *
 * Description: Since storage for multiple blobs can be aggregated into a single HDF5 dataset, the dimensionality and size of
 *              a blob might not match that of the whole dataset. This function returns information about the blob's extent.
 *              The blob dimensionality is implied by the dataset-dimensional offset/count values for the blob: any dimension
 *              that has a count==1 is considered to be excluded from the blob's dimensionality.
 *
 * Example:     A blob of size [100,1,100] at offset [0,0,0] in a dataset whose size is [100,200,400] will be considered to
 *              be a two dimensional blob of size [100,100].
 *
 * Return:      On success, returns the blob's dimensionality (zero implies scalar) and the extent through the optional SIZE
 *              and SPACE arguments. Returns negative on failure.  A null blob (a blob that has no elements) will be returned
 *              as a one dimensional space whose size is zero.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, September  5, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_blob_space(ss_blob_t *blob,                  /* The blob to query */
              hsize_t *size,                    /* OUT: Optional returned size per blob dimension */
              hid_t *space                      /* OUT: Optional returned data space for the blob */
              )
{
    SS_ENTER(ss_blob_space, int);
    int         blob_ndims=0, dset_ndims, i;
    hsize_t     blobsize[SS_MAXDIMS];
    const hsize_t *maxdims=NULL;

    SS_ASSERT_MEM(blob, ss_blob_t);
    if ((dset_ndims=ss_blob_bound_f(blob, NULL, NULL, NULL, NULL, NULL))<0) SS_ERROR(FAILED);

    /* Copy the blob dimension sizes from the blob object. The `count' stored in the blob object has one element per dataset
     * dimension. To get the blob dimensions we just discard any sizes that are unity. */
    for (i=blob_ndims=0; i<dset_ndims; i++) {
        if (SS_BLOB(blob)->count[i]>1) {
            blobsize[blob_ndims] = SS_BLOB(blob)->count[i];
            if (size) size[blob_ndims] = blobsize[blob_ndims];
            blob_ndims++;
        } else if (0==SS_BLOB(blob)->count[i]) {
            blob_ndims = 1;
            blobsize[0] = 0;
            if (size) size[0] = 0;
            maxdims = ss_blob_unlim_g;
            break;
        }
    }

    /* Create the data space. Give it the dimensionality and extent of the blob (not the dataset). Data spaces are born with
     * the "all" selection. If the blob had any zero size dimensions then treat the blob as a one-dimensional, unlimited space
     * whose current size is zero (see also ss_blob_normalize()). */
    if (space && (*space=H5Screate_simple(blob_ndims, blobsize, maxdims))<0) SS_ERROR(HDF5);

 SS_CLEANUP:
    SS_LEAVE(blob_ndims);
}
              
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Create storage for a blob
 *
 * Description: If a blob is bound only to memory then a call to this function will create a dataset to hold that data and
 *              will bind the dataset to the blob.  The created dataset is large enough to hold the memory data to which the
 *              blob is currently bound.
 *
 *              *Bit*Flags*
 *
 *              The following bit flags are supported in the FLAGS argument:
 *
 *              * SS_ALLSAME:     All tasks must be bound to identical memory buffer sizes and only one of those tasks will be
 *                                used to compute the total size of the new dataset.  In this case the returned SIZE will be zero
 *                                for all tasks.
 *
 *              * SS_BLOB_RANK:   All tasks pass the same blob, which is bound to an identical amount of memory across all
 *                                tasks, making it unecessary to do any communication to calculate total dataset size or the
 *                                return values for the SIZE argument.  This is useful when T tasks are about to each write N
 *                                elements in task rank order, creating a dataset whose total size is T*N.
 *
 *              * SS_BLOB_EACH:   Each task is passing its own unique blob and all those blobs will point to the same HDF5
 *                                dataset with the blobs layed out onto that dataset in task rank order. All of the blobs must
 *                                belong to the same scope because that scope determines the communicator that is used for
 *                                collective message passing operations.
 *
 *              * SS_BLOB_EXTEND: The blob and its new dataset will be extendible in one or more dimensions. If the caller
 *                                supplies a dataset creation property list with the "dcpl" property of PROPS (see Blob
 *                                Properties) then that list should contain chunk size information. If no "dcpl" property is
 *                                supplied then SSlib will create a dataset with some default chunk size near 64kB.
 *
 *              All tasks should pass the same blob (except when SS_BLOB_EACH is specified) and the total size of the
 *              resulting dataset is the sum of sizes of the bound memory across all tasks (unless SS_ALLSAME is specified).
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Conceptually, this function is collective across the communicator of the scope to which the blob belongs.
 *              However, since HDF5 metadata operations are file-collective this function must also be collective across the
 *              blob's file communicator.  But since the blob doesn't exist on the "extra" tasks those tasks should pass the
 *              blob's top-level scope instead in order to describe the file in which the dataset is being created.
 *
 * Issue:       It might be nice if this function also took a datatype to use when creating the dataset. But it's no big deal
 *              because the caller can create the dataset outside SSlib if need be, and then bind it to the blob with
 *              ss_blob_bind_f().
 *
 *              This function currently only creates one-dimensional datasets even when the memory spaces are
 *              multi-dimensional. This should eventually be fixed. [2003-09-11]
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_mkstorage(ss_blob_t *blob,              /* The blob for which a dataset should be created. As a temporary work around
                                                 * for HDF5 limitations, tasks that are part of the blob's file communicator
                                                 * but not the blob's scope communicator should pass the top-level scope for
                                                 * the file that contains the blob. See the parallel notes for details. */
                  hsize_t *size,                /* OUT: Optional pointer which, upon successful return, will point to the
                                                 * cumulative number of elements from all lower-rank MPI tasks.  This can be
                                                 * useful in some cases for the caller to compute the starting offset of a
                                                 * task's contribution to the dataset when it is known that all tasks will
                                                 * output data contiguously in task rank order. The reason it's computed by
                                                 * this function is because this function must perform collective
                                                 * communication anyway in order to determine the dataset size. In SS_ALLSAME
                                                 * mode the return value will be zero because each tasks data starts at the
                                                 * beginning of the blob. Likewise, for SS_BLOB_EACH mode the return value is
                                                 * zero because each task has its own blob to describe where that tasks data
                                                 * starts in the dataset shared by all the tasks. */
                  unsigned flags,               /* Certain bit flags that affect the operation of this function. See the
                                                 * description for details. */
                  ss_prop_t *props              /* Optional properties (see Blob Properties). If a dataset creation property
                                                 * list and/or dataset name is supplied then they must be the same across all
                                                 * calling tasks. */
                  )
{
    SS_ENTER(ss_blob_mkstorage, herr_t);
    ss_scope_t          topscope=SS_SCOPE_NULL; /* Top scope for BLOB */
    ss_scope_t          blobscope=SS_SCOPE_NULL;/* Scope in which the blob lives if this is a blob task */
    ss_gblob_t          *gblob=NULL;            /* The gblob table from the top scope's gfile entry */
    hid_t               dcpl=H5P_DEFAULT;       /* Dataset creation property list */
    hid_t               dcpl_close=-1;          /* Dataset creation property list to close during cleanup */
    long                my_nelmts_l=0;          /* Should be hssize_t but long is the biggest MPI type supported by reduce */
    long                all_nelmts_l=0;         /* Sum of all my_nelmts across all tasks in the file communicator. */
    hsize_t             my_nelmts;              /* Number of dataset elements to be owned by the BLOB */
    hsize_t             all_nelmts;             /* Same as all_nelmts except type is hsize_t for passing ptr to HDF5 */
    hsize_t             dset_size;              /* Number of elements in the created dataset */
    hsize_t             dset_maxsize;           /* The maximum size in elements of the created dataset */
    hid_t               dset=-1, fspace=-1;     /* Dataset and space to be cleaned up on return */
    hid_t               fid=-1;                 /* File handle, not to be closed on return */
    MPI_Comm            filecomm;               /* File communicator */
    int                 root;                   /* Root task w.r.t. file communicator for broadcast */
    int                 blob_self=-1;           /* Rank of calling task in the blob communicator, or negative */
    int                 blob_ntasks=0;          /* Number of tasks in the blob communicator, or zero */
    int                 file_ntasks=0;          /* Number of tasks in the file communicator. */
    size_t              elmt_size;              /* Size of a single element of the dataset in the file */
    hsize_t             chunk_size=64*1024;     /* Size of chunk: 64 kB adjusted later to elements instead of bytes */
    const char          *dsetname=NULL;         /* Optional additional dataset name */
    ss_file_t           blobfile=SS_FILE_NULL;  /* The file in which BLOB appears */
    hsize_t             offset;                 /* Blob's offset into the dataset */

    if (!ss_mpi_extras((ss_pers_t**)&blob, &topscope)) SS_ERROR(FAILED);
    gblob = SS_GFILE_LINK(&topscope)->gblob;
    if (blob) SS_ASSERT_MEM(blob, ss_blob_t);

    SS_ASSERT(((flags & SS_ALLSAME)?1:0) + ((flags & SS_BLOB_EACH)?1:0) + ((flags & SS_BLOB_RANK)?1:0) <= 1);

    /* Blob and file communicator info */
    if (blob) {
        if (NULL==ss_pers_scope((ss_pers_t*)blob, &blobscope)) SS_ERROR(FAILED);
        if (ss_scope_comm(&blobscope, NULL, &blob_self, &blob_ntasks)<0) SS_ERROR(FAILED);
    }
    if (ss_scope_comm(&topscope, &filecomm, NULL, &file_ntasks)<0) SS_ERROR(FAILED);
    
    /* Dataset creation properties -- same for *all* tasks */
    if (!props || NULL==ss_prop_get(props, "dcpl", H5T_NATIVE_HID, &dcpl)) {
        SS_STATUS_OK;
        if (flags & SS_BLOB_EXTEND) {
            if ((dcpl=dcpl_close=H5Pcreate(H5P_DATASET_CREATE))<0) SS_ERROR(HDF5);
            if (0==(elmt_size=H5Tget_size(SS_BLOB(blob)->m.mtype))) SS_ERROR(HDF5);
            chunk_size = MAX(1, (chunk_size+elmt_size/2)/elmt_size);
            if (H5Pset_chunk(dcpl, 1, &chunk_size)<0) SS_ERROR(HDF5);
        }
    } else if (H5D_CHUNKED!=H5Pget_layout(dcpl)) {
        SS_ERROR_FMT(USAGE, ("SS_BLOB_EXTEND flag set but supplied dataset creation property list is not chunked"));
    }

    /* Optional dataset name */
    if (!props || NULL==ss_prop_get(props, "name", H5T_NATIVE_VOIDP, &dsetname)) {
        SS_STATUS_OK;
        dsetname = NULL;
    }
    
    /* Determine how much total data we have. We only have to sum this across the scope communicator but we need the result
     * across the file communicator. If the the SS_ALLSAME or SS_BLOB_RANK flag is set then the size will be calculated and
     * broadcast from blob task zero. If blob_ntasks==file_ntasks then all calling tasks must necessarily also be blob
     * tasks and we may be able to avoid some communication. */
    if (blob && SS_BLOB(blob)->m.mem)
        if ((my_nelmts_l = H5Sget_select_npoints(SS_BLOB(blob)->m.mspace))<0) SS_ERROR(HDF5);
    if (flags & SS_ALLSAME) {
        /* All blob tasks are bound to the same size memory, but only one will be used as the total dataset size. */
        if (blob_ntasks!=file_ntasks) {
            if ((root=ss_mpi_elect(blob?(ss_pers_t*)blob:(ss_pers_t*)&topscope))<0) SS_ERROR(FAILED);
            ss_mpi_bcast(&my_nelmts_l, (size_t)1, MPI_UNSIGNED_LONG, root, filecomm);
        }
        all_nelmts_l = my_nelmts_l;
    } else if (flags & SS_BLOB_RANK) {
        /* All blob tasks are bound to the same size memory; total size is the sum across all blob tasks */
        all_nelmts_l = my_nelmts_l * blob_ntasks;
        if (blob_ntasks!=file_ntasks) {
            if ((root=ss_mpi_elect(blob?(ss_pers_t*)blob:(ss_pers_t*)&topscope))<0) SS_ERROR(FAILED);
            ss_mpi_bcast(&all_nelmts_l, (size_t)1, MPI_UNSIGNED_LONG, root, filecomm);
        }
    } else {
        /* Every task may be bound to a different memory size; total size is the sum across all blob tasks */
#ifdef HAVE_PARALLEL
        if (MPI_Allreduce(&my_nelmts_l, &all_nelmts_l, 1, MPI_UNSIGNED_LONG, MPI_SUM, filecomm)) SS_ERROR(MPI);
#else
        all_nelmts_l = my_nelmts_l;
#endif
    }
    my_nelmts = my_nelmts_l;            /* How many elements belong to this task's blob? Zero on non-blob tasks. */
    all_nelmts = all_nelmts_l;          /* How big is the whole dataset? */

    /* If the user is asking for a scanned SIZE then return that now since we're doing communication anyway. We could have
     * done an allgather of the local size and done the reduction and scan in this function, but separate MPI_Allreduce() and
     * MPI_Scan() seem more readable and use less memory.. */
    if (flags & SS_ALLSAME) {
        offset = 0;
        if (size) *size = offset;
    } else if (flags & SS_BLOB_RANK) {
        offset = blob_self * my_nelmts_l;
        if (size) *size = offset;
    } else {
#ifdef HAVE_PARALLEL
        if (MPI_Scan(&my_nelmts_l, &all_nelmts_l, 1, MPI_UNSIGNED_LONG, MPI_SUM, filecomm)) SS_ERROR(MPI);
        offset = all_nelmts_l - my_nelmts_l; /* exclusive scan */
#else
        offset = 0;
#endif
        if (size) {
            *size = (flags & SS_BLOB_EACH) ? 0 : offset;
        }
    }

    /* It doesn't make much sense to create a zero-size dataset, but there might be some cases where all tasks just happened
     * to have no data. We have two choices with HDF5: create a zero size extendible dataset or create a very small contiguous
     * dataset. We will use a 1d, contiguous dataset of size one when possible because that occupies the least file space and the
     * dimensionality matches what ss_blob_normalize() returns for a null space. */
    if (flags & SS_BLOB_EXTEND) {
        dset_size = all_nelmts;
        dset_maxsize = H5S_UNLIMITED;
    } else {
        dset_size = MAX(1, all_nelmts);
        dset_maxsize = dset_size;
    }
    if ((fspace=H5Screate_simple(1, &dset_size, &dset_maxsize))<0) SS_ERROR(HDF5);

    /* Create the new dataset using either the name supplied in the property list or a temporary name */
    if (dsetname) {
        if (NULL==ss_pers_file((ss_pers_t*)blob, &blobfile)) SS_ERROR(FAILED);
        if ((fid=ss_file_isopen(&blobfile, NULL))<0) SS_ERROR(FAILED);
        if ((dset=H5Dcreate(fid, dsetname, SS_BLOB(blob)->m.mtype, fspace, dcpl))<0) SS_ERROR(HDF5);
    } else {
        if ((dset=H5Dcreate(gblob->storage, "TEMP", SS_BLOB(blob)->m.mtype, fspace, dcpl))<0) SS_ERROR(HDF5);
        if (H5Gunlink(gblob->storage, "TEMP")<0) SS_ERROR(HDF5);
    }

    /* Bind the dataset to the blob(s) */
    if ((flags & SS_ALLSAME) || (flags & SS_BLOB_EACH)) {
        if (H5Sselect_slab(fspace, H5S_SELECT_SET, (hsize_t)0, &offset, &my_nelmts)<0) SS_ERROR(HDF5);
    } else {
        if (H5Sselect_slab(fspace, H5S_SELECT_SET, (hsize_t)0, NULL, &all_nelmts)<0) SS_ERROR(HDF5);
    }
    if (ss_blob_bind_f(blob, dset, fspace, (flags & SS_BLOB_EXTEND))<0) SS_ERROR(FAILED);

    /* Successful cleanup */
    if (dcpl_close>0) H5Pclose(dcpl_close);
    dcpl_close = -1;

 SS_CLEANUP:
    if (dset>0) H5Dclose(dset);
    if (fspace>0) H5Sclose(fspace);
    if (dcpl_close>0) H5Pclose(dcpl_close);

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Extend a blob
 *
 * Description: Certain blobs are considered to be extendible (see ss_blob_bind_f()) and this function can be used to make
 *              such a blob larger. The new size of the blob is the maximum SIZE from across all collectively calling tasks
 *              but a blob is never made smaller than its current size.  Each dimension of the blob is resized independently
 *              of the others. If the SS_ALLSAME bit is set in the FLAGS argument then the MPI reduction to find the maximum
 *              is skipped and we assume that all tasks passed the same value for SIZE.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the file communicator to which the blob belongs. Tasks that are not part of the blob's scope
 *              should pass the top-scope of the blob's file in place of the BLOB argument. This restriction is due to HDF5's
 *              requirement that an H5Dextend() call is file collective.
 *
 * Programmer:  Robb Matzke
 *              Monday, June 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_extend(ss_blob_t *blob,                 /* Blob which is possibly to be extended. */
               const hsize_t *size,             /* New size of the blob on each calling task per dataset dimension */
               unsigned flags,                  /* Bit flags such as SS_ALLSAME. */
               ss_prop_t UNUSED *props          /* Optional property list (none defined yet). */
               )
{
    SS_ENTER(ss_blob_extend, herr_t);
    ss_scope_t          topscope=SS_SCOPE_NULL; /* Top scope for BLOB */
    ss_scope_t          blobscope=SS_SCOPE_NULL;
    hsize_t             newsize[SS_MAXDIMS], dsize[SS_MAXDIMS];
    ss_gblob_t          *gblob=NULL;
    size_t              d_idx=SS_NOSIZE;
    hid_t               dspace=-1;
    int                 i, ndims;
    MPI_Comm            blobcomm=SS_COMM_NULL;

    if (!ss_mpi_extras((ss_pers_t**)&blob, &topscope)) SS_ERROR(FAILED);
    gblob = SS_GFILE_LINK(&topscope)->gblob;
    if (blob) SS_ASSERT_MEM(blob, ss_blob_t);

    /* The blob must be bound to a dataset */
    if (!SS_BLOB(blob)->dsetaddr) SS_ERROR_FMT(USAGE, ("blob is not bound to a dataset"));
    if (SS_NOSIZE==(d_idx=ss_blob_didx(blob))) SS_ERROR(FAILED);
    SS_ASSERT(SS_BLOB(blob)->dsetaddr==*((haddr_t*)(&gblob->d[d_idx].stat.objno)[0]));
    SS_ASSERT(gblob->d[d_idx].dset>0);
    if ((ndims=H5Sget_simple_extent_ndims(gblob->d[d_idx].dspace))<0) SS_ERROR(HDF5);

    if (blob) {
        /* MAX-Reduce the size in each dimension */
        if (flags & SS_ALLSAME) {
            for (i=0; i<ndims; i++) newsize[i] = size[i];
        } else {
            if (NULL==ss_pers_scope((ss_pers_t*)blob, &blobscope)) SS_ERROR(FAILED);
            if (ss_scope_comm(&blobscope, &blobcomm, NULL, NULL)<0) SS_ERROR(FAILED);
#ifdef HAVE_PARALLEL
            {
                unsigned long lsize[SS_MAXDIMS], lsize2[SS_MAXDIMS];
                for (i=0; i<ndims; i++) lsize[i] = size[i];
                if (MPI_Allreduce(lsize, lsize2, ndims, MPI_UNSIGNED_LONG, MPI_MAX, blobcomm)) SS_ERROR(MPI);
                for (i=0; i<ndims; i++) newsize[i] = lsize2[i];
            }
#else
            for (i=0; i<ndims; i++) newsize[i] = size[i];
#endif
        }
        
        /* Make sure that the dataset is extendible or already large enough. We could actually delay this and let
         * ss_blob_bind_f() check it, but then we might unbind the blob if it fails.  */
        ss_blob_ckspace(gblob->d[d_idx].dspace, ndims, dsize, NULL, NULL, NULL);
        if (gblob->d[d_idx].is_extendible) {
            for (i=0; i<ndims; i++) {
                dsize[i] = MAX(dsize[i], SS_BLOB(blob)->start[i]+newsize[i]);
            }
        } else {
            for (i=0; i<ndims; i++) {
                if (SS_BLOB(blob)->start[i]+newsize[i]>dsize[i]) {
                    SS_ERROR_FMT(OVERFLOW,
                                 ("blob(start=%lu,count=%lu) extends past end of non-extendible dataset(%lu) in dimension %d",
                                  (unsigned long)(SS_BLOB(blob)->start[i]), (unsigned long)newsize[i],
                                  (unsigned long)(dsize[i]), i));
                }
            }
        }
    
        /* Create a new data space for the rebind operation */
        if ((dspace=H5Screate_simple(ndims, dsize, ss_blob_unlim_g))<0) SS_ERROR(HDF5);
        if (H5Sselect_slab(dspace, H5S_SELECT_SET, (hsize_t)0, SS_BLOB(blob)->start, newsize)<0) SS_ERROR(HDF5);
    }

    /* All tasks rebind the blob to a dataset. The `dspace' argument is ignored on the non-blob tasks */
    if (ss_blob_bind_f(blob?blob:(ss_blob_t*)&topscope, gblob->d[d_idx].dset, dspace, SS_BLOB_EXTEND)<0) SS_ERROR(FAILED);

    /* Successful cleanup */
    if (dspace>0 && H5Sclose(dspace)<0) SS_ERROR(HDF5);
    dspace = -1;

SS_CLEANUP:
    if (dspace>0) H5Sclose(dspace);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Extend a blob
 *
 * Description: This is a convenience function for ss_blob_extend() which only extends the first dimension of a blob.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    See ss_blob_extend()
 *
 * Programmer:  Robb Matzke
 *              Monday, June 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_extend1(ss_blob_t *blob,                /* Blob which is possibly to be extended. */
                hsize_t size,                   /* The new size of the blob in elements. */
                unsigned flags,                 /* Bit flags such as SS_ALLSAME. */
                ss_prop_t *props                /* Optional property list (see ss_blob_extend()). */
                )
{
    SS_ENTER(ss_blob_extend1, herr_t);
    hsize_t     msize[H5S_MAX_RANK];

    memset(msize, 0, sizeof msize);
    msize[0] = size;
    if (ss_blob_extend(blob, msize, flags, props)<0) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Read data from a file
 *
 * Description: Given a blob which is bound to a dataset, read the desired portion of the blob as described by the IOSPACE
 *              argument and return a pointer to the buffer into which the data has been placed (which is either the buffer
 *              bound to the dataset with ss_blob_bind_m() or a buffer allocated by this function).
 *
 *              The FLAGS argument determines specifics about the read operation and the following bits are defined:
 *
 *              * SS_BLOB_COLLECTIVE: The operation is to be considered collective across the blob's file communicator.
 *                                    SSlib can use two-phase I/O for this situation. If this bit is not set then the
 *                                    operation is considered independent of any other task.
 *
 *              * SS_BLOB_ASYNC:      The I/O for this call can be performed asynchronously, allowing SSlib to use two-phase
 *                                    I/O even when the call is independent. Asynchronous reads are guaranteed to be completed
 *                                    after a call to ss_blob_flush() on the affected dataset.
 *
 *              * SS_BLOB_UNBIND:     The memory and blob are disassociated from each other when this function returns,
 *                                    whether the return status indicates success or failure. However, if the failure
 *                                    occurs early enough (e.g., the blob is invalid) then no disassociation will occur.
 *
 *              If the blob is not bound to memory then a buffer is allocated by this function but is not bound to the blob.
 *              The returned datatype is a native type based on the dataset type and computed with H5Dget_native_type().
 *              Otherwise the memory and dataset datatypes must be conversion compatible.
 *
 *              The data spaces of the memory, the blob, the dataset, and the IOSPACE must all be compatible. SSlib allows the
 *              dataset dimensionality to be larger than the blob dimensionality, but the memory, blob, and IOSPACE data spaces
 *              must all be the same dimensionality.
 *
 * Example:     These examples are all one-dimensional for simplicity, and therefore a real application would probably use the
 *              one-dimensional versions of most of these functions. Their names are the same except a `1' is appended; their
 *              arguments are obviously different. See ss_blob_read1() for examples.
 *
 *              Example 1: A single task reads all of the blob's data into a static buffer. We assume that the dataset
 *              contains 100 elements of an integer datatype. SSlib will convert the data from the file datatype to an !int
 *              type in memory.
 *
 *                  ss_blob_t b = SS_RELATION(rel)->d_blob;
 *                  int data[100];
 *                  hsize_t size = 100;
 *                  hid_t mspace = H5Screate_simple(1, &size, NULL); // 100 contiguous elements in memory
 *                  ss_blob_bind_m(&b, data, H5T_NATIVE_INT, mspace); // bind buffer to the blob
 *                  ss_blob_read(&b, H5S_ALL, SS_BLOB_UNBIND, NULL); // read data into the buffer
 *
 *              Example 2: All tasks read all data collectively (they could also do it independently as when the above example
 *              is executed by every task, but that could be very inefficient since SSlib and lower layers cannot recognize
 *              that collective optimizations are possible).
 *
 *                  ss_blob_t b = SS_RELATION(rel)->d_blob;
 *                  int data[100];
 *                  hsize_t size = 100;
 *                  hid_t mspace = H5Screate_simple(1, &size, NULL); // 100 contiguous elements in memory
 *                  ss_blob_bind_m(&b, data, H5T_NATIVE_INT, mspace); // bind buffer to the blob
 *                  ss_blob_read(&b, H5S_ALL, SS_BLOB_COLLECTIVE|SS_BLOB_UNBIND, NULL); // read data into the buffer
 *               
 *              Example 3: Each task reads 50 non-overlapping task-rank-order elements from a blob that was associated with a
 *              relation.  Each task provides a buffer for the result. We assume that the blob's
 *              data is one dimensional, a floating-point datatype, and of sufficient size to satisfy the read request.
 *
 *                  int self = ...;                  // task rank in blob's file communicator
 *                  float buffer[50];                // the result buffer
 *                  hsize_t start = 50 * self;       // starting offset relative to blob's data
 *                  hsize_t size = 50;               // number of consecutive elements to read
 *                  hid_t bspace;                    // blob's data space
 *                  hid_t mspace = H5Screate_simple(1, &size, NULL); // describe memory to HDF5
 *                  ss_blob_t *blob = SS_RELATION_P(rel, d_blob); // beware: blob pointer is temporary
 *                  ss_blob_bind_m(blob, buffer, H5T_NATIVE_FLOAT, mspace); // bind buffer to the dataset
 *                  ss_blob_bound_f(blob, NULL, NULL, NULL, &bspace); // get the blob's data space
 *                  H5Sselect_hyperslab(bspace, H5S_SELECT_SET, &start, NULL, &size, NULL); // describe partial read
 *                  ss_blob_read(blob, bspace, SS_BLOB_COLLECTIVE, NULL);
 *                  ss_blob_bind_m(blob, NULL, 0, 0); // unbind memory from blob (could have used SS_BLOB_UNBIND)
 *                  H5Sclose(bspace);
 *                  H5Sclose(mspace);
 *
 * Return:      Returns a pointer to memory containing the result data. If the blob was bound to memory then this is the same
 *              pointer that would be returned with a call to ss_blob_bound_m(), otherwise this is memory that was allocated
 *              by SSlib and should be freed by the caller. Returns the null pointer on failure.
 *
 *              For asynchronous operations there is currently no good way to determine whether this particular read was
 *              successful, only whether the entire flush operation was successful.
 *
 * Parallel:    Independent unless the SS_BLOB_COLLECTIVE bit is turned on in the FLAGS argument, in which case the function
 *              should be called collectively across all tasks in the file communicator to which the blob belongs. The tasks
 *              that are part of the file communicator but not part of the blob's scope communicator should pass the top scope
 *              of the blob's file as the BLOB argument.
 *
 *              The order of reads and writes is indeterminate when SSlib is doing asynchronous I/O and it is up to the caller
 *              to issue the appropriate ss_blob_flush() calls to ensure an ordering.
 *
 * Issue:       For a collective call where all tasks read the same selection of the dataset and all desire the same datatype
 *              and all destinations are contiguous in memory, SSlib may perform an independent H5Dread() and then broadcast
 *              the data to the other tasks.  This optimization should eventually be moved into HDF5.
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_blob_read(ss_blob_t *blob,                   /* The blob from which data should be read. This is the blob's top scope for
                                                 * tasks that are participating for collectivity and are members of the blob's
                                                 * file communicator but not the blob's scope communicator. */
             hid_t iospace,                     /* This is an optional hyperslab describing the part of the blob that is to
                                                 * be read. The extent and selection are relative to the portion of the
                                                 * dataset owned by the blob and described in some previous call to
                                                 * ss_blob_bind_f() (perhaps in an earlier execution). If not specified then
                                                 * all of the blob's data is read. Such a selection is generally constructed
                                                 * by calling ss_blob_space() and applying the selection. */
             unsigned flags,                    /* Various bit flags commonly passed to this function. */
             ss_prop_t UNUSED *props            /* See [Blob Properties]. (Unused at this time.) */
             )
{
    SS_ENTER(ss_blob_read, voidP);
    ss_scope_t          topscope=SS_SCOPE_NULL; /* The top scope for BLOB */
    ss_gblob_t          *gblob=NULL;            /* Global blob record for the file */
    size_t              d_idx=SS_NOSIZE;        /* Index into gblob->d table */
    void                *buffer=NULL;           /* Result buffer */
    hid_t               iom=-1, iof=-1;         /* Data spaces for memory and file -- extent and selections */
    hid_t               dxpl=-1;                /* Dataset transfer property list -- not copied here so do not close */
    hid_t               mtype=-1;               /* Memory datatype saved after unbind */
    herr_t              status;                 /* Return status of called functions */
    size_t              tsize=SS_NOSIZE;        /* Datatype size in bytes */
    hssize_t            nelmts;                 /* Number of elements in the I/O operation */

    if (!ss_mpi_extras((ss_pers_t**)&blob, &topscope)) SS_ERROR(FAILED);
    gblob = SS_GFILE_LINK(&topscope)->gblob;

    if (blob) {
        /* Verify data space consistency to the extent possible without actually calling H5Dread(). This also generates the
         * dataset-dimension data spaces for memory and file. We have to do this before unbinding the memory but we still want
         * to unbind it if necessary. So the error return is delayed until after the unbinding. */
        if ((status = ss_blob_ckspaces(blob, iospace, &iom, &iof))<0) SS_SAVE;

        /* Get a buffer for the result and unbind memory if so requested. If not bound to memory we'll allocate a buffer after
         * we've checked that we're bound to a dataset. */
        if ((buffer = SS_BLOB(blob)->m.mem)) {
            if (flags & SS_BLOB_UNBIND) {
                SS_BLOB(blob)->m.mem = NULL;
                mtype = SS_BLOB(blob)->m.mtype; /*will be closed at end*/
                SS_BLOB(blob)->m.mtype = 0;
                if (H5Sclose(SS_BLOB(blob)->m.mspace)<0) SS_ERROR(HDF5);
                SS_BLOB(blob)->m.mspace = 0;
            } else {
                if ((mtype = H5Tcopy(SS_BLOB(blob)->m.mtype))<0) SS_ERROR(HDF5);
            }
        }

        /* Now that we've had an opportunity to unbind, raise the ss_blob_ckspaces() error if there was one. */
        if (status<0) {
            SS_REFAIL;
            SS_ERROR(FAILED);
        }
        
        /* Make sure the blob is bound to a dataset */
        if (!SS_BLOB(blob)->dsetaddr) SS_ERROR_FMT(USAGE, ("blob is not bound to a dataset"));
        if (SS_NOSIZE==(d_idx=ss_blob_didx(blob))) SS_ERROR(FAILED);
        SS_ASSERT(SS_BLOB(blob)->dsetaddr==*((haddr_t*)(&gblob->d[d_idx].stat.objno)[0]));
        SS_ASSERT(gblob->d[d_idx].dset>0);

        /* Allocate a buffer if we don't have one by now. */
        if (!buffer) {
            SS_ASSERT(mtype<0);
            if ((mtype = H5Tget_native_type(gblob->d[d_idx].dtype, H5T_DIR_DEFAULT))<0) SS_ERROR(HDF5);
            if (0==(tsize=H5Tget_size(mtype))) SS_ERROR(HDF5);
            if ((nelmts = H5Sget_select_npoints(iom))<0) SS_ERROR(HDF5);
            if (NULL==(buffer=malloc(MAX((size_t)1, (size_t)(nelmts*tsize))))) SS_ERROR(RESOURCE);
        }
        
        /* Get the dataset transfer property list. We do not allow this to be specified by the caller because SSlib does too
         * many things under the covers to make that reliable. */
        dxpl = flags & SS_BLOB_COLLECTIVE ? SS_GFILE_LINK(&topscope)->dxpl_collective : SS_GFILE_LINK(&topscope)->dxpl_independent;
        
        /* Read the data */
        /* ISSUE: The two-phase I/O optimization for reads is not implemented. */
        if (H5Dread(gblob->d[d_idx].dset, mtype, iom, iof, dxpl, buffer)<0) SS_ERROR(HDF5);

        /* Cleanup */
        if (H5Sclose(iom)<0) SS_ERROR(HDF5);
        iom = -1;
        if (H5Sclose(iof)<0) SS_ERROR(HDF5);
        iof = -1;
        if (H5Tclose(mtype)<0) SS_ERROR(HDF5);
        mtype = -1;
    }

 SS_CLEANUP:
    if (iom>0) H5Sclose(iom);
    if (iof>0) H5Sclose(iof);
    if (mtype>0) H5Tclose(mtype);
    
    SS_LEAVE(buffer);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Read data from a file
 *
 * Description: This is a convenience function for ss_blob_read() when the blob's data is scalar or one dimensional and
 *              contiguous in the dataset.  The OFFSET and SIZE describe the part of the blob's data to be read and are in
 *              terms of elements with respect to the blob's data (not necessarily the whole dataset).
 *
 * Example:     The examples here are similar to those given for ss_blob_read() and show that it's much easier to use the
 *              special case functions for single dimension blobs.
 *
 *              Example 1: A single task reads all of the blob's data into a static buffer. We assume that the dataset
 *              contains 100 elements of an integer datatype. SSlib will convert the data from the file datatype to an !int
 *              type in memory.
 *
 *                  ss_blob_t b = SS_RELATION(rel)->d_blob;
 *                  int data[100];
 *                  ss_blob_bind_m1(&b, data, H5T_NATIVE_INT, 100);       // bind memory to the blob
 *                  ss_blob_read1(&b, 0, 100, SS_BLOB_UNBIND, NULL);      // read data into the buffer
 *
 *              Example 2: All tasks read all data collectively (they could also do it independently as when the above example
 *              is executed by every task, but that could be very inefficient since SSlib and lower layers cannot recognize
 *              that collective optimizations are possible).
 *
 *                  ss_blob_t b = SS_RELATION(rel)->d_blob;
 *                  int data[100];
 *                  ss_blob_bind_m1(&b, data, H5T_NATIVE_INT, 100);       // bind memory to the blob
 *                  ss_blob_read1(&b, 0, 100, SS_BLOB_COLLECTIVE, NULL);  // read data into the buffer
 *                  ss_blob_bind_m1(&b, NULL, 0, 0);                      // unbind the buffer just to be safe
 *               
 *
 *              Example 3: Each task reads 50 non-overlapping task-rank-order elements from a blob that was associated with a
 *              relation.  Each task provides a buffer for the result. We assume that the blob's
 *              data is one dimensional, a floating-point datatype, and of sufficient size to satisfy the read request.
 *
 *                  int self = ...;                  // task rank in blob's file communicator
 *                  float *buffer=NULL;              // the result buffer to be allocated by ss_blob_read1()
 *                  buffer = ss_blob_read1(blob, 50*self, 50, SS_BLOB_COLLECTIVE, NULL); // read part of the blob data
 *
 * Return:      Same as ss_blob_read().
 *
 * Parallel:    Same as ss_blob_read().
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_blob_read1(ss_blob_t *blob,                  /* The blob from which data should be read. */
              hsize_t offset,                   /* Offset w.r.t. the blob's data for the first element to be read. */
              hsize_t nelmts,                   /* Number of elements to be read from the blob's data. */
              unsigned flags,                   /* See the FLAGS argument of ss_blob_read(). */
              ss_prop_t *props                  /* See [Blob Properties]. */
              )
{
    SS_ENTER(ss_blob_read1, voidP);
    hid_t       iospace=-1;
    void        *retval=NULL;
    int         ndims;
    
    SS_ASSERT_MEM(blob, ss_blob_t);
    if ((ndims=ss_blob_space(blob, NULL, &iospace))<0) SS_ERROR(FAILED);
    if (1!=ndims && 0!=ndims) SS_ERROR_FMT(USAGE, ("blob is not one-dimensional or scalar"));

    /* Select the elements of the `iospce'. Even if `nelmts' is zero we still have to call ss_blob_read() for some side
     * effects like for the SS_BLOB_UNBIND bit flag. */
    if (0==ndims) {
        SS_ASSERT(0==nelmts || 1==nelmts);
        SS_ASSERT(0==offset || 0==nelmts);
        if (0==nelmts) {
            if (H5Sselect_none(iospace)<0) SS_ERROR(HDF5);
        } else {
            if (H5Sselect_all(iospace)<0) SS_ERROR(HDF5);
        }
    } else {
        if (H5Sselect_slab(iospace, H5S_SELECT_SET, (hsize_t)0, &offset, &nelmts)<0) SS_ERROR(HDF5);
    }

    /* The actual write */
    if (NULL==(retval=ss_blob_read(blob, iospace, flags, props))) SS_ERROR(FAILED);

    /* Successful cleanup */
    if (H5Sclose(iospace)<0) SS_ERROR(HDF5);
    iospace = -1;

 SS_CLEANUP:
    if (iospace>0) H5Sclose(iospace);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Write data to a blob
 *
 * Description: Given a blob that has been bound to both memory and a dataset, transfer data from memory to the file. The
 *              portion of the blob to write is described by the IOSPACE argument.
 *
 *              The FLAGS argument determines specifics about the write operation and the following bits are defined:
 *
 *              * SS_BLOB_COLLECTIVE: The operation is to be considered collective across the blob's file communicator.
 *                                    SSlib can use two-phase I/O for this situation. If this bit is not set then the
 *                                    operation is considered independent of any other task.
 *
 *              * SS_BLOB_ASYNC:      The I/O for this call can be performed asynchronously, allowing SSlib to use two-phase
 *                                    I/O even when the call is independent. Asynchronous writes are guaranteed to be completed
 *                                    after a call to ss_blob_flush() on the affected dataset.
 *
 *              * SS_BLOB_COPY:       In the case of asynchronous I/O it is important that the caller doesn't modify the
 *                                    memory being written until the data has been shipped to the aggregator. However, if this
 *                                    bit is turned on then SSlib will make a copy of the data before this call returns. Only
 *                                    the data actually being written is copied, which could be substantially smaller than the
 *                                    entire memory array that was supplied in the case of partial I/O.
 *
 *              * SS_BLOB_UNBIND:     The memory and blob are disassociated from each other when this function returns,
 *                                    whether the return status indicates success or failure. However, if the failure
 *                                    occurs early enough (e.g., the blob is invalid) then no disassociation will occur.
 *
 *              * SS_BLOB_FREE:       The memory bound to this blob should be freed as soon as it is no longer needed. For
 *                                    synchronous writes it will be freed before this function returns, but asynchronous
 *                                    writes may retain the memory longer.  The SS_BLOB_UNBIND flag must also be set when
 *                                    SS_BLOB_FREE is set.
 *
 *              * SS_ALLSAME:         This bit flag indicates that all blob tasks are calling this function collectively and
 *                                    they are all providing the same data values to be written all at the same offset. Thus
 *                                    only one task needs to actually do any writing. There are three main reasons to use this
 *                                    flag: (1) for convenience so the caller doesn't need to choose a particular task, (2) so
 *                                    that if the caller doesn't choose a particular task that tasks aren't duplicating effort,
 *                                    and (3) so that the aggregation tasks know that they might be able to get data without
 *                                    doing any communication.
 *
 *              The data spaces of the memory, the blob, the dataset, and the IOSPACE must all be compatible. SSlib allows the
 *              dataset dimensionality to be larger than the blob dimensionality, but the memory, blob, and IOSPACE data spaces
 *              must all be the same dimensionality.
 *
 * Example:     See ss_blob_read() for examples since this function is almost identical in nature. 
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 *              For asynchronous operations there is currently no good way to determine whether this particular write was
 *              successful, only whether the entire flush was successful.
 *
 * Parallel:    Independent unless the SS_BLOB_COLLECTIVE or SS_ALLSAME bits are turned on in the FLAGS argument, in which
 *              case the function should be called collectively across all tasks in the file communicator to which the blob
 *              belongs.  The tasks that are part of the file communicator but not part of the blob's scope communicator
 *              should pass the top scope of the blob's file as the BLOB argument.
 *
 *              The order of reads and writes is indeterminate when SSlib is doing asynchronous I/O and it is up to the caller
 *              to issue the appropriate ss_blob_flush() calls to ensure an ordering.
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_write(ss_blob_t *blob,                  /* The blob for which data is written, which must be bound to both memory
                                                 * and a dataset. Any task that is part of the file communicator but not the
                                                 * scope communicator is participating soley for the sake of collectivity and
                                                 * should pass the blob's top scope here instead. */
              hid_t iospace,                    /* This is an optional hyperslab describing the part of the blob that is to
                                                 * be written. The extent and selection are relative to the portion of the
                                                 * dataset owned by the blob and described in some previous call to
                                                 * ss_blob_bind_f() (perhaps in an earlier execution). If not specified then
                                                 * all of the blob's data is written. */
              unsigned flags,                   /* Various bit flags commonly passed to this function. */
              ss_prop_t UNUSED *props           /* See [Blob Properties]. (Unused at this time.) */
              )
{
    SS_ENTER(ss_blob_write, herr_t);
    ss_scope_t          topscope=SS_SCOPE_NULL; /* The top scope for BLOB */
    ss_scope_t          blobscope=SS_SCOPE_NULL;/* Scope that contains BLOB */
    ss_gblob_t          *gblob=NULL;            /* Global blob table from the file */
    size_t              d_idx=SS_NOSIZE;        /* Index into gblob->d table */
    void                *buffer=NULL;           /* Source buffer that was bound to the blob */
    hid_t               iom=-1, iof=-1;         /* Data spaces for memory and file -- extent and selections */
    hid_t               dxpl=-1;                /* Dataset transfer property list -- not copied here so do not close */
    hid_t               mtype=-1;               /* Memory datatype saved after unbind */
    herr_t              status;                 /* Return status of called functions */
    void                *conv_buf=NULL;         /* Conversion buffer */
    int                 blobtask=-1;            /* The rank of the calling task in the blob communicator */
    ss_gfile_t          *gfile=NULL;            /* The file descriptor for the top scope */
    static ss_prop_t    *syncprops=NULL;        /* Synchronization properties */
    MPI_Comm            blobcomm=SS_COMM_NULL;  /* Communicator for the blob's scope */

#ifdef HAVE_PARALLEL
    int                 ndims;                  /* Dimensionality */
    ss_blob_stride_t    iom_stride;             /* Stride description of memory buffer */
    ss_blob_stride_t    conv_stride;            /* Stride description of conversion buffer */
    hsize_t             buf_size[H5S_MAX_RANK]; /* Size of memory buffer */
    hsize_t             slab_start[H5S_MAX_RANK];/* Starting address of slab in buffer */
    hsize_t             slab_count[H5S_MAX_RANK];/* Size of the slab for I/O */
    hsize_t             nelmts_h;               /* Bigger, temporary version of `nelmts' */
    size_t              nelmts;                 /* Total number of elements being written */
    size_t              max_tsize;              /* Maximum of memory and file datatype sizes */
#endif

    if (flags & SS_ALLSAME) flags |= SS_BLOB_COLLECTIVE;
    if (!ss_mpi_extras((ss_pers_t**)&blob, &topscope)) SS_ERROR(FAILED);
    gfile = SS_GFILE_LINK(&topscope);
    gblob = gfile->gblob;

    /* Create synchronization property list */
    if (!syncprops) {
        if (NULL==(syncprops=ss_prop_new("blob sync props"))) SS_ERROR(FAILED);
        if (ss_prop_add(syncprops, "dset", H5T_NATIVE_HID, NULL)<0) SS_ERROR(FAILED);
    }
    
    if (blob) {
        /* Get blob's scope, communicator, and MPI task number in that communicator */
        if (NULL==ss_pers_scope((ss_pers_t*)blob, &blobscope)) SS_ERROR(FAILED);
        if (ss_scope_comm(&blobscope, &blobcomm, &blobtask, NULL)<0) SS_ERROR(FAILED);

        /* Verify data space consistency to the extent possible without actually calling H5Dwrite(). This also generates the
         * dataset-dimension data spaces for memory and file. We have to do this before unbinding the memory but we still want
         * to unbind it if necessary. So the error return is delayed until after the unbinding. */
        if ((status = ss_blob_ckspaces(blob, iospace, &iom, &iof))<0) SS_SAVE;

        /* Get the source buffer at this early stage so we can unbind before most errors if so requested. */
        if (NULL==(buffer=SS_BLOB(blob)->m.mem)) SS_ERROR_FMT(USAGE, ("blob is not bound to memory"));

        /* Unbind memory */
        if (flags & SS_BLOB_UNBIND) {
            SS_BLOB(blob)->m.mem = NULL;
            mtype = SS_BLOB(blob)->m.mtype; /*will be closed at end*/
            SS_BLOB(blob)->m.mtype = 0;
            if (H5Sclose(SS_BLOB(blob)->m.mspace)<0) SS_ERROR(HDF5);
            SS_BLOB(blob)->m.mspace = 0;
        } else {
            if ((mtype = H5Tcopy(SS_BLOB(blob)->m.mtype))<0) SS_ERROR(HDF5);
        }

        /* Now that we've had an opportunity to unbind, raise the ss_blob_ckspaces() error if there was one. */
        if (status<0) {
            SS_REFAIL;
            SS_ERROR(FAILED);
        }

        if ((flags & SS_BLOB_FREE) && !(flags & SS_BLOB_UNBIND)) SS_ERROR_FMT(USAGE, ("SS_BLOB_FREE requires SS_BLOB_UNBIND"));
        
        /* Make sure the blob is bound to a dataset. Perhaps this can be relaxed in the future, making it symmetric with
         * ss_blob_read() which can operate when the blob is not bound to memory. */
        if (!SS_BLOB(blob)->dsetaddr) SS_ERROR_FMT(USAGE, ("blob is not bound to a dataset"));
        if (SS_NOSIZE==(d_idx=ss_blob_didx(blob))) SS_ERROR(FAILED);
        SS_ASSERT(SS_BLOB(blob)->dsetaddr==*((haddr_t*)(&gblob->d[d_idx].stat.objno)[0]));
        SS_ASSERT(gblob->d[d_idx].dset>0);

#ifdef HAVE_PARALLEL
        /* Synchronous or asynchronous two-phase I/O. We can use two-phase I/O if it is enabled and either the SS_BLOB_ASYNC
         * or SS_BLOB_COLLECTIVE flags are set. However, if the SS_ALLSAME flag is set then we fall back to the basic
         * H5Dwrite() because two-phase I/O has not been optimized for that case: the basic H5Dwrite() is probably faster. */
        if (gblob->agg.maxaggtasks>0 &&
            (flags & (SS_BLOB_ASYNC|SS_BLOB_COLLECTIVE)) &&
            0==(flags & SS_ALLSAME)) {
            if ((nelmts_h=H5Sget_select_npoints(iom))>0) {
                SS_ASSERT(nelmts_h<(hsize_t)SS_NOSIZE); /*because we cast it below*/
                nelmts = (size_t)nelmts_h;
                if ((flags & SS_BLOB_COPY) || !H5Tequal(mtype, gblob->d[d_idx].dtype)) {
                    max_tsize = MAX(H5Tget_size(mtype), H5Tget_size(gblob->d[d_idx].dtype));
                    if (NULL==(conv_buf=malloc(max_tsize*nelmts))) SS_ERROR(RESOURCE);
                    ss_blob_sendbuf_total_g++; /*total number of buffers allocated; not bytes!*/
                    if ((ndims = ss_blob_ckspace(iom, H5S_MAX_RANK, buf_size, slab_start, slab_count, NULL))<0) SS_ERROR(FAILED);
                    SS_ASSERT(ndims>0); /*probably doesn't work for scalar? [rpm 2004-06-23]*/
                    if (ss_blob_stride(ndims, buf_size, slab_start, slab_count, &iom_stride)<0) SS_ERROR(FAILED);
                    if (ss_blob_stride_1((hsize_t)0, (hsize_t)nelmts, &conv_stride)<0) SS_ERROR(FAILED);
                    if (ss_blob_stride_copy(conv_buf, &conv_stride, buffer, &iom_stride, H5Tget_size(mtype))<0) SS_ERROR(FAILED);
                    if (H5Tconvert(mtype, gblob->d[d_idx].dtype, nelmts, conv_buf, NULL, H5P_DEFAULT)<0) SS_ERROR(FAILED);
                    if (flags & SS_BLOB_FREE) SS_FREE(buffer);
                    buffer = conv_buf;
                    flags |= SS_BLOB_FREE;
                    if (H5Sclose(iom)<0) SS_ERROR(HDF5);
                    if ((iom = H5Screate_simple(ndims, slab_count, slab_count))<0) SS_ERROR(HDF5);
                }
                if (ss_blob_async_append(gblob, d_idx, mtype, iom, iof, buffer, flags)<0) SS_ERROR(FAILED);
                mtype = iom = iof = -1;
            }

            /* If the ASYNC flag is not set (then the COLLECTIVE flag _must_ be set) then we must block for the two-phase
             * I/O operation to complete: both data shipping and the H5Dwrite() call. */
            if (0==(flags & SS_BLOB_ASYNC)) {
                SS_ASSERT(flags & SS_BLOB_COLLECTIVE);
                if (ss_prop_set(syncprops, "dset", H5T_NATIVE_HID, &(gblob->d[d_idx].dset))<0) SS_ERROR(FAILED);
                if (ss_blob_synchronize(&topscope, syncprops)<0) SS_ERROR(FAILED); /*initiate data shipping*/
                if (ss_blob_async_flush(gblob, d_idx, gfile->dxpl_independent, SS_STRICT)<0) SS_ERROR(FAILED);
                /* ISSUE: I don't think an MPI_Barrier() is sufficient to make the aggregators' independent H5Dwrite()
                 *        data to become available to the other tasks for reading. We may need either an MPI_File_sync()
                 *        or we may need to have the tasks write collectively. [rpm 2004-07-26] */                
                if (ss_mpi_barrier(blobcomm)<0) SS_ERROR(FAILED); /*wait for aggregators' independent H5Dwrite() calls*/
            }
            goto done;
        }
#endif

        /* Write the data synchronously */
        if (flags & SS_ALLSAME) {
            /* Only one task needs to write the data. We use blob task zero by default. However, all tasks must synchronize
             * because the other tasks were expecting to not return until their data had been written. */    
            if (0==blobtask) {
                dxpl = SS_GFILE_LINK(&topscope)->dxpl_independent;
                if (H5Dwrite(gblob->d[d_idx].dset, mtype, iom, iof, dxpl, buffer)<0) SS_ERROR(HDF5);
            }
            /* ISSUE: I don't think an MPI_Barrier() is sufficient here. We may need either an MPI_File_sync() or we may need
             *        to have the tasks write collectively but only one supplying data. [rpm 2004-07-26] */
            if (ss_mpi_barrier(blobcomm)<0) SS_ERROR(FAILED);
        } else {
            /* All tasks have data to write */
            dxpl = flags & SS_BLOB_COLLECTIVE ?
                   SS_GFILE_LINK(&topscope)->dxpl_collective :
                   SS_GFILE_LINK(&topscope)->dxpl_independent;
            if (H5Dwrite(gblob->d[d_idx].dset, mtype, iom, iof, dxpl, buffer)<0) SS_ERROR(HDF5);
        }
        if (flags & SS_BLOB_FREE) SS_FREE(buffer);
    }

#ifdef HAVE_PARALLEL
 done:
#endif
    
    /* Successful cleanup */
    if (iom>0 && H5Sclose(iom)<0) SS_ERROR(HDF5);
    iom = -1;
    if (iof>0 && H5Sclose(iof)<0) SS_ERROR(HDF5);
    iof = -1;
    if (mtype>0 && H5Tclose(mtype)<0) SS_ERROR(HDF5);
    mtype = -1;

 SS_CLEANUP:
    if (iom>0) H5Sclose(iom);
    if (iof>0) H5Sclose(iof);
    if (mtype>0) H5Tclose(mtype);
    SS_FREE(conv_buf);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Write data to a blob
 *
 * Description: This is a convenience function for ss_blob_write() when the blob's data is scalar or one dimensional and
 *              contiguous in the dataset. The OFFSET and SIZE describe the part of the blob's data to be written and are in
 *              terms of elements with respect to the blob's data (not necessarily the whole dataset).
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    See ss_blob_write().
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_write1(ss_blob_t *blob,                 /* The blob for which data is written. This blob must be bound to both memory
                                                 * and a dataset */
               hsize_t offset,                  /* Offset into the blob data for the first element to be written. */
               hsize_t nelmts,                  /* Number of consecutive elements to be written. */
               unsigned flags,                  /* See ss_blob_write(). */
               ss_prop_t *props                 /* See [Blob Properties]. */
               )
{
    SS_ENTER(ss_blob_write1, herr_t);
    hid_t       iospace=-1;
    int         ndims;
    
    SS_ASSERT_MEM(blob, ss_blob_t);
    if ((ndims=ss_blob_space(blob, NULL, &iospace))<0) SS_ERROR(FAILED);
    if (1!=ndims && 0!=ndims) SS_ERROR_FMT(USAGE, ("blob is not one-dimensional or scalar"));

    /* Select the elements of the `iospace'. Even if `nelmts' is zero we still have to call ss_blob_write() for some side
     * effects like for the SS_BLOB_UNBIND bit flag. */
    if (0==ndims) {
        SS_ASSERT(0==nelmts || 1==nelmts);
        SS_ASSERT(0==offset || 0==nelmts);
        if (0==nelmts) {
            if (H5Sselect_none(iospace)<0) SS_ERROR(HDF5);
        } else {
            if (H5Sselect_all(iospace)<0) SS_ERROR(HDF5);
        }
    } else {
        if (H5Sselect_slab(iospace, H5S_SELECT_SET, (hsize_t)0, &offset, &nelmts)<0) SS_ERROR(HDF5);
    }

    /* The actual write */
    if (ss_blob_write(blob, iospace, flags, props)<0) SS_ERROR(FAILED);

    /* Successful cleanup */
    if (H5Sclose(iospace)<0) SS_ERROR(HDF5);
    iospace = -1;

 SS_CLEANUP:
    if (iospace>0) H5Sclose(iospace);

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Initiate 2-phase I/O
 *
 * Description: When a task has a buffer of data to be written to the file it can do so either synchronously or
 *              asynchronously. The benefits of asynchronous writes are: (1) separate write requests from a task destined for
 *              a single dataset can be combined into a single H5Dwrite() call, (2) data from many tasks can be aggregated
 *              into larger amounts before the H5Dwrite() calls occur, and (3) at least part of the process of getting the
 *              data to disk can be overlapped with other computation.
 *
 *              Any task can make independent requests for asynchronous writes, and a collective call to this function causes
 *              tasks to agree on how the data transfer will occur and to initiate the asynchronous transfers of data to
 *              chosen aggregation tasks.  After this function has been called all data will either be on disk or in the
 *              process of moving to disk. Therefore it is not safe yet for the client to free its buffers and will not be
 *              safe until a call to ss_blob_flush().
 *
 *              All blobs for the file of the specified top scope are synchronized unless the "dset" property of PROPS is
 *              specified, in which case it should be a dataset handle to indicate the one and only dataset to synchronize.
 *
 * Issues:      In order to assure correct order of I/O operations it is imperative that all I/O for a particular HDF5 dataset
 *              be either synchronous or asynchronous.  SSlib doesn't check for mixed mode requests but it probably should.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    File collective across the communicator for TOPSCOPE.  File collectivity is necessary in order to meet HDF5
 *              API requirements for H5Dextend() and because multiple blobs from different scopes using different
 *              communicators can share a single dataset (the one thing they all have in common is the file).
 *
 *              Serial behavior is a no-op.
 *
 * Programmer:  Robb Matzke,
 *              Monday, September 1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_blob_synchronize(ss_scope_t UNUSED_SERIAL *topscope,
                    ss_prop_t UNUSED_SERIAL *props      /* See [Blob Properties]. */
                    )
{
    SS_ENTER(ss_blob_synchronize, herr_t);
#ifdef HAVE_PARALLEL
    ss_gblob_t          *gblob=NULL;                    /* Blob related file-level information */
    size_t              a_idx;                          /* Index into gblob->a table */
    size_t              a_start=0;                      /* Virtual start of the gblob->a[] so we don't have to shift elmts */
    size_t              a_start_orig=0;                 /* Original starting value of a_start */
    size_t              d_idx;                          /* Index into gblob->d[] for dataset on which we're operating */
    size_t              *ndsets=NULL;                   /* Array of dataset counts for all tasks */
    size_t              ndsets_total;                   /* Sum-reduction of ndsets[] */
    size_t              req_idx;                        /* Index through the I/O requests */
    size_t              nreq_total;                     /* Total I/O requests across all tasks */
    MPI_Comm            filecomm;                       /* File communicator */
    int                 self, ntasks;                   /* Rank and size of file communicator */
    int                 task;                           /* MPI task counter */
    hsize_t             *sel=NULL;                      /* Selection metadata (starts and counts per dimension per request) */
    size_t              sel_nalloc=0;                   /* Number of elements allocated for `sel' array */
    size_t              *sel_start=NULL;                /* Exclusive sum scan for selection metadata counts */
    size_t              *sel_count=NULL;                /* Number of selection metadata values by task */
    size_t              sel_idx;                        /* Index into `sel' array */
    int                 ndims;                          /* Dataset dimensionality */
    haddr_t             smallest_objno;                 /* Object header address of current dataset */
    hsize_t             dset_size[H5S_MAX_RANK];        /* Current size of the dataset */
    hsize_t             bound_start[H5S_MAX_RANK];      /* Selection bounding box starting offsets */
    hsize_t             bound_end[H5S_MAX_RANK];        /* bound_start + bound_count */
    hid_t               one_dset=-1;                    /* Non-copied handle to the dataset being synchronized */
    haddr_t             one_dset_addr=0;                /* Nonzero if only synchronizing one dataset */
    H5G_stat_t          stat;                           /* Status info for the one_dset */
    int                 i, j;

    /* WARNING! DO NOT CHANGE THIS STRUCT W/O CHANGING THE dset_info_mt MPI DATATYPE! */
    struct dset_info_t {
        haddr_t         objno;                          /* HDF5 address for dataset object header */
        size_t          nreq;                           /* Number of I/O requests for this dataset */
    }                   *dset_info=NULL;                /* Information exchanged between MPI tasks */
    MPI_Datatype        dset_info_mt=MPI_DATATYPE_NULL; /* MPI datatype for struct dset_info_t elements */

    gblob = SS_GFILE_LINK(topscope)->gblob;
    if (ss_scope_comm(topscope, &filecomm, &self, &ntasks)<0) SS_ERROR(FAILED);

    /* Are the callers synchronizing blobs for a particular dataset or all blobs in the file? */
    if (!props || NULL==ss_prop_get(props, "dset", H5T_NATIVE_HID, &one_dset)) {
        SS_STATUS_OK;
        one_dset = -1;
        one_dset_addr = 0;
    } else {
        if (H5Gget_objinfo(one_dset, ".", FALSE, &stat)<0) SS_ERROR(HDF5);
        one_dset_addr = *((haddr_t*)stat.objno);
    }
    
    /* Sort async list by dataset address but keep ordering per dataset */
    SS_ASSERT(!ss_blob_async_sort_g);
    ss_blob_async_sort_g = gblob;
    qsort(gblob->a, gblob->a_nused, sizeof(gblob->a[0]), ss_blob_async_sort_cb);
    ss_blob_async_sort_g = NULL;

    /* Combine adjacent requests */
    /* ISSUE: This function makes no attempt to combine separate write requests from a single task into a single request. */

    /* Allocate arrays that we'll need later */
    if (NULL==(ndsets=malloc(ntasks*sizeof(*ndsets)))) SS_ERROR(RESOURCE);
    if (NULL==(dset_info=malloc(ntasks*sizeof(*dset_info)))) SS_ERROR(RESOURCE);
    if (NULL==(sel_start=malloc((ntasks+1)*sizeof(*sel_start)))) SS_ERROR(RESOURCE);
    if (NULL==(sel_count=malloc(ntasks*sizeof(*sel_count)))) SS_ERROR(RESOURCE);

    /* Create MPI datatype for dset_info elements */
    {
        static int counts[4] = {1, 1, 1, 1};
        static MPI_Aint starts[4] = {0, offsetof(struct dset_info_t, objno),
                                     offsetof(struct dset_info_t, nreq), sizeof(struct dset_info_t)};
        static MPI_Datatype types[4];
        if (!types[0]) {
            types[0] = MPI_LB;
            types[1] = MPI_HADDR_T;
            types[2] = MPI_SIZE_T;
            types[3] = MPI_UB;
        }
        if (MPI_Type_struct(4, counts, starts, types, &dset_info_mt)) SS_ERROR(MPI);
        if (MPI_Type_commit(&dset_info_mt)) SS_ERROR(MPI);
    }

    /* How many unique datasets does each task have to process? */
    for (a_idx=0, ndsets[self]=0; a_idx<gblob->a_nused; a_idx++) {
        if (one_dset_addr>0) {
            if (*((haddr_t*)(&gblob->d[gblob->a[a_idx].d_idx].stat.objno)[0])==one_dset_addr) {
                a_start = a_idx;
                ndsets[self] = 1;
                break;
            }
        } else if (0==a_idx) {
            ndsets[self]++;
        } else if (!ss_eq_nos(gblob->d[gblob->a[a_idx].d_idx].stat.objno,gblob->d[gblob->a[a_idx-1].d_idx].stat.objno)) {
            ndsets[self]++;
        }
    }
    if (ss_mpi_allgather(ndsets, 1, MPI_SIZE_T, filecomm)<0) SS_ERROR(FAILED);
    for (task=0, ndsets_total=0; task<ntasks; task++) ndsets_total += ndsets[task];

    /* Process each dataset collectively. We could have done one big MPI_Allgather() of all the information we need for every
     * dataset, but that could be an awful lot of dataset I/O records. So instead we locally sort each gblob->a[] and then
     * repeatedly collectively choose the lowest-numbered dataset. */
    a_start_orig = a_start;
    while (ndsets_total) {
        /* Choose the lowest numbered dataset. We can't exchange information about the elements in the data space selection
         * until we know how many selections each task has. Pass zero for the `nreq' field if we don't have any more datasets. */
        if (a_start<gblob->a_nused) {
            dset_info[self].objno = gblob->d[gblob->a[a_start].d_idx].stat.objno;
            for (a_idx=1; a_start+a_idx<gblob->a_nused; a_idx++) {
                if (*((haddr_t*)(&gblob->d[gblob->a[a_start+a_idx].d_idx].stat.objno)[0])!=dset_info[self].objno) break;
            }
            dset_info[self].nreq = a_idx;
        } else {
            memset(dset_info+self, 0, sizeof(*dset_info));
        }
        if (ss_mpi_allgather(dset_info, 1, dset_info_mt, filecomm)<0) SS_ERROR(FAILED);
                
        /* Choose the dataset with the lowest object header address or the one that matches one_dset_addr. */
        if (one_dset_addr) {
            smallest_objno = one_dset_addr;
        } else {
            for (task=0, smallest_objno=0; task<ntasks; task++) {
                if (dset_info[task].nreq>0) {
                    if (smallest_objno==0) {
                        smallest_objno = dset_info[task].objno;
                    } else if (dset_info[task].objno < smallest_objno) {
                        smallest_objno = dset_info[task].objno;
                    }
                }
            }
            SS_ASSERT(smallest_objno>0); /* someone must have a dataset since ndsets_total is nonzero */
        }
        
        /* Find information about the dataset by looking for the appropriate entry in the gblob->d array. That array should
         * probably be sorted, but we can't easily sort it here because the gblob->a array contains indices into the gblob->d
         * array. */
        for (d_idx=0; d_idx<gblob->d_nused; d_idx++) if (*((haddr_t*)(&gblob->d[d_idx].stat.objno)[0])==smallest_objno) break;
        SS_ASSERT(d_idx<gblob->d_nused);
        if ((ndims=H5Sget_simple_extent_dims(gblob->d[d_idx].dspace, dset_size, NULL))<0) SS_ERROR(HDF5);

        /* Clean up the exchanged dset_info() by pruning out all entries other than smallest_objno. At the same time compute
         * an exclusive sum scan of the number of integers that must be exchanged in order to fully describe the data space
         * selection of all of the I/O requests for this dataset (see below). */
        for (task=0; task<=ntasks; task++) {
            if (task<ntasks) {
                if (smallest_objno!=dset_info[task].objno) memset(dset_info+task, 0, sizeof(*dset_info));
                sel_count[task] = 2 * ndims * dset_info[task].nreq; /* start & count per dimension per request */
            }
            sel_start[task] = task ? sel_start[task-1] + sel_count[task-1] : 0;
        }

        /* Allocate the `sel' array. */
        SS_ASSERT(0 == sel_start[ntasks] % (2*ndims));
        nreq_total = sel_start[ntasks] / (2*ndims);
        SS_EXTEND(sel, sel_start[ntasks], sel_nalloc);
        
        /* Exchange information about the elements selected for the I/O operation for the smallest dataset.  Each task will
         * broadcast a list of starts (per dimension) and a list of counts (per dimension) repeated once for each I/O request.
         * For each I/O request we group the starts together and the counts together (instead of starts and counts
         * interleaved) because that's how the various HDF5 data space functions take the arguments. So we don't have to
         * shuffle things around as much. */
        for (a_idx=0; a_idx<dset_info[self].nreq; a_idx++) {
            if (ss_blob_ckspace(gblob->a[a_start+a_idx].iof, ndims, NULL,
                                sel+sel_start[self]+a_idx*ndims*2+0/*starts*/, 
                                sel+sel_start[self]+a_idx*ndims*2+ndims/*counts*/, NULL)<0) SS_ERROR(FAILED);
        }
        if (ss_mpi_allgatherv(sel, sel_count, sel_start, MPI_HSIZE_T, filecomm)) SS_ERROR(MPI);

        /* Find the minimum bounding contiguous hyperslab for the I/O request and represent that with a multi-dimensional
         * start and end stored in bound_start[] and bound_end[] */
        for (i=0; i<ndims; i++) {
            bound_start[i] = sel[i];
            bound_end[i] = bound_start[i] + sel[ndims+i];
        }
        for (req_idx=1; req_idx<nreq_total; req_idx++) {
            for (i=0; i<ndims; i++) {
                bound_start[i] = MIN(bound_start[i], sel[req_idx*ndims*2+i]);
                bound_end[i] = MAX(bound_end[i], sel[req_idx*ndims*2+i]+sel[req_idx*ndims*2+ndims+i]);
            }
        }

        /* Extend dataset and reassign aggregators if appropriate */
        for (i=0; i<ndims; i++) {
            if (bound_end[i] > dset_size[i]) {
                for (j=0; j<ndims; j++) dset_size[j] = MAX(dset_size[j], bound_end[j]);
                if (H5Dextend(gblob->d[d_idx].dset, dset_size)<0) SS_ERROR(HDF5);
                if (H5Sclose(gblob->d[d_idx].dspace)<0) SS_ERROR(HDF5);
                if ((gblob->d[d_idx].dspace=H5Dget_space(gblob->d[d_idx].dset))<0) SS_ERROR(HDF5);
                if (ss_blob_async_aggregators(filecomm, gblob, d_idx, ndims, dset_size)<0) SS_ERROR(FAILED);
                break;
            }
        }

        /* If no aggregators are assigned yet then do that now. This takes care of the case when this is the first time
         * 2-phase I/O is performed on this dataset. */
        if (!gblob->d[d_idx].agg.tasks &&
            ss_blob_async_aggregators(filecomm, gblob, d_idx, ndims, dset_size)<0) SS_ERROR(FAILED);

        /* ISSUE: The data shipping code uses MPI async p2p functions even when the source and destination are the same task. */

        /* ISSUE: This function does not attempt to optimize the case when ss_blob_write() was called with the SS_ALLSAME bit
         *        flag. When this bit is set all blob tasks will have called ss_blob_write() with identical data and offsets and
         *        it may therefore be the case that an aggregation task has the data already available locally. */

        /* If the current task is an aggregator then post all receives for data. Allocation of the aggregation buffer might
         * cause us to block until I/O for some other dataset completes, hence *all* sends on other tasks to this task must be
         * posted for all prior datasets for which this task is an aggregator. */
        for (task=0; task<ntasks; task++) {
            for (sel_idx=sel_start[task]; sel_idx<sel_start[task+1]; sel_idx+=ndims*2) {
                hsize_t *req_dstart = sel + sel_idx;            /* ultimate position of request in the dataset */
                hsize_t *req_count  = sel + sel_idx + ndims;    /* size of the request */
                ss_blob_stride_t agg_stride[H5S_MAX_RANK*2];    /* desc of intersection of request with aggregation buffer */
                int nstrides = ss_blob_async_intersect(gblob, d_idx, self, dset_size, req_dstart, NULL, NULL,
                                                       req_count, agg_stride, NULL);
                if (nstrides<0) SS_ERROR(FAILED);
                if (ss_blob_async_receives(gblob, d_idx, task, filecomm, nstrides, agg_stride)<0) SS_ERROR(FAILED);
            }
        }

        /* If the current task has data then post sends to the appropriate aggregators */
        for (a_idx=0; a_idx<dset_info[self].nreq; a_idx++) {
            hsize_t *req_dstart = sel + sel_start[self] + a_idx*ndims*2;          /* ultimate position of request in the dataset */
            hsize_t *req_count = sel + sel_start[self] + a_idx*ndims*2 + ndims;   /* size of the request */
            ss_blob_stride_t send_stride[H5S_MAX_RANK*2];       /* desc of send buffer / aggregation buffer intersection */
            hsize_t req_dfirst_1 = ss_blob_array_linear(ndims, dset_size, req_dstart, NULL);
            hsize_t req_dlast_1 = ss_blob_array_linear(ndims, dset_size, req_dstart, req_count);
            hsize_t min_aggseq = req_dfirst_1 / gblob->d[d_idx].agg.elmts_per_agg;
            hsize_t max_aggseq = req_dlast_1 / gblob->d[d_idx].agg.elmts_per_agg;
            hsize_t mem_size[H5S_MAX_RANK], req_mstart[H5S_MAX_RANK];
            hsize_t aggseq;
            int aggtask, nstrides;

            SS_ASSERT(max_aggseq<gblob->d[d_idx].agg.n);
            if (ss_blob_ckspace(gblob->a[a_start+a_idx].iom, ndims, mem_size, req_mstart, NULL, NULL)<0) SS_ERROR(FAILED);
            for (aggseq=min_aggseq; aggseq<=max_aggseq; aggseq++) {
                aggtask = gblob->d[d_idx].agg.tasks[aggseq];
                nstrides = ss_blob_async_intersect(gblob, d_idx, aggtask, dset_size, req_dstart, mem_size, req_mstart,
                                                   req_count, NULL, send_stride);
                if (nstrides<0) SS_ERROR(FAILED);
                if (ss_blob_async_sends(gblob, d_idx, aggtask, filecomm, nstrides, send_stride,
                                        gblob->a[a_start+a_idx].buffer, gblob->a[a_start+a_idx].flags)<0)
                    SS_ERROR(FAILED);
                gblob->a[a_start+a_idx].flags &= ~SS_BLOB_FREE; /* should only be marked for freeing once! */
            }
        }

        /* Update dataset counters based on which tasks have requests for this dataset */
        a_start += dset_info[self].nreq;
        for (task=0; task<ntasks; task++) {
            if (dset_info[task].objno) --ndsets_total;
        }
    }

    /* Delete all the async requests that were just processed */
    for (a_idx=a_start_orig; a_idx<a_start; a_idx++) {
        H5Tclose(gblob->a[a_idx].mtype);
        H5Sclose(gblob->a[a_idx].iom);
        H5Sclose(gblob->a[a_idx].iof);
        /* Do not free gblob->a[a_idx].buffer because we are sending that data to the aggregators. The caller is
         * responsible for freeing that if desired once they know the data shipping has been completed. */
    }
    H5Eclear(); /*we don't care about errors in the above loop*/
    memmove(gblob->a+a_start_orig, gblob->a+a_start, (gblob->a_nused-a_start)*sizeof(gblob->a[0]));
    gblob->a_nused -= a_start - a_start_orig;
    SS_FREE(ndsets);
    SS_FREE(dset_info);
    SS_FREE(sel_count);
    SS_FREE(sel_start);
    SS_FREE(sel);
    if (MPI_Type_free(&dset_info_mt)) SS_ERROR(MPI);
    dset_info_mt = MPI_DATATYPE_NULL;

 SS_CLEANUP:
    /* ISSUE: Lots of error-related stuff needs to go here! */
#endif /*HAVE_PARALLEL*/
    
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Flush pending data to HDF5
 *
 * Description: Because SSlib can perform asynchronous I/O, data transferred between memory and the HDF5 file can sit in SSlib
 *              queues after the call to ss_blob_read() or ss_blob_write() returns. This function causes those buffers to be
 *              flushed to HDF5 (but does not subsequently demand that HDF5 flush its buffers to the Unix file). The TOPSCOPE and
 *              BLOB arguments indicates what should be flushed, and the FLAGS and PROPS arguments say how to flush it. In any
 *              case, all blob metadata for the file is synchronized.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    File-collective.  If the flush is occuring because a task (or tasks) wish to read data which was written
 *              asynchronously then the caller will be wise to also invoke MPI_Barrier() to insure that this task's read does
 *              not occur before the aggregator has a chance to write the previous data.  The barrier call is not part of this
 *              routine because most uses of this routine are just to ensure that data has been (or very shortly will have
 *              been) written to the HDF5 file.
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_flush(ss_scope_t *topscope,     /* Determines the file being flushed, and thus the collectivity of the function. */
              ss_blob_t *blob,          /* Optional argument to restrict the flushing to a single dataset: the dataset to
                                         * which BLOB refers, which might also be used by other blobs.  If BLOB is not
                                         * specified then all blob datasets for the file are flushed.  When BLOB is defined on
                                         * a subset of the FILE communicator, the tasks that don't own the blob should pass
                                         * a non-null object (passing the FILE argument a second time is recommended) in order
                                         * to distinguish between this single-dataset case and the all-datasets case without
                                         * the need for collective communication. */
              unsigned flags,           /* Bit flags that describe how to flush the selected datasets. See
                                         * ss_blob_async_flush() for details. If none of the !FLUSH or !REAP bits are set then
                                         * async two-phase I/O is started but nothing is reaped. */
              ss_prop_t *props          /* Flushing properties (none defined at this time). */
              )              
{
    SS_ENTER(ss_blob_flush, herr_t);
    ss_gfile_t          *gfile=NULL;
    size_t              d_idx=SS_NOSIZE;
    ss_prop_t           *syncprops = NULL;

    gfile = SS_GFILE_LINK(topscope);
    SS_ASSERT(gfile);

    /* Initiate two-phase I/O for the specified dataset(s) */
    if (blob) {
        if (NULL==(syncprops=ss_prop_new("blob sync props"))) SS_ERROR(FAILED);
        d_idx = SS_BLOB(blob)->m.d_idx;
        if (ss_prop_add(syncprops, "dset", H5T_NATIVE_HID, &(gfile->gblob->d[d_idx].dset))<0) SS_ERROR(FAILED);
    }
    if (ss_blob_synchronize(topscope, syncprops)<0) SS_ERROR(FAILED);

    /* Wait for two-phase I/O to complete according to FLAGS. The ss_blob_async_flush_all() is independent. */
    if (flags & (SS_STRICT|SS_BLOB_FLUSH_SHIP|SS_BLOB_FLUSH_WRITE|SS_BLOB_REAP_SHIP|SS_BLOB_REAP_WRITE)) {
        SS_ASSERT(!blob || d_idx!=SS_NOSIZE)
        if (ss_blob_async_flush_all(gfile->gblob, d_idx, gfile->dxpl_independent, flags)<0) SS_ERROR(FAILED);
    }

    /* Successful cleanup */
    if (syncprops) ss_prop_dest(syncprops);
    syncprops = NULL;

    SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Initialize two-phase I/O properties from string
 *
 * Description: Given a string (such as the value of the SSLIB_2PIO environment variable) create a property list of two-phase
 *              I/O properties. See ss_blob_set_2pio() for details.
 *
 * Return:      Property list of two-phase I/O properties on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, December 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_prop_t *
ss_blob_init_2pio(ss_prop_t *props, const char *s)
{
    SS_ENTER(ss_blob_init_2pio, ss_prop_tP);
    const char  *end=NULL;
    char        *rest=NULL;
    char        key[64], value[128];
    int         i;
    long        nl;
    size_t      size;
    ss_prop_t   *props_here=NULL;

    if (!s) s = "defaults";
    if (!props && NULL==(props=props_here=ss_prop_new("2pio"))) SS_ERROR(FAILED);

    while (*s) {
        /* Get the key */
        while (*s && (';'==*s || isspace(*s))) s++;
        for (end=s; *end && *end!=';' && *end!='='; end++) /*void*/;
        SS_ASSERT((size_t)(end-s) < sizeof(key));
        strncpy(key, s, (size_t)(end-s));
        key[end-s] = '\0';
        s = end;

        /* Get the value */
        if ('='==*s) {
            for (end=++s; *end && ';'!=*end; end++) /*void*/;
            SS_ASSERT((size_t)(end-s) < sizeof(value));
            strncpy(value, s, (size_t)(end-s));
            value[end-s] = '\0';
            s = end;
        } else {
            value[0] = '\0';
        }

        /* Take action */
        if (!*key && !*value && !*s) {
            break;
        } else if (!strcmp(key, "off") || !strcmp(key, "no") || !strcmp(key, "false")) {
            i = 0;
            if (ss_prop_has(props, "maxaggtasks")) {
                if (ss_prop_set(props, "maxaggtasks", H5T_NATIVE_INT, &i)<0) SS_ERROR(FAILED);
            } else {
                if (ss_prop_add(props, "maxaggtasks", H5T_NATIVE_INT, &i)<0) SS_ERROR(FAILED);
            }
        } else if (!strcmp(key, "on") || !strcmp(key, "yes") || !strcmp(key, "true") || !strcmp(key, "defaults")) {
            char        tmp[128];
            int         ntasks = ss_mpi_comm_size(sslib_g.comm);
            int         maxaggtasks = (ntasks+31)/32;
            sprintf(tmp,
                    "minbufsize=512kB; alignment=512kB; maxaggtasks=%d; sendqueue=4; aggbuflimit=10MB; "
                    "asynchdf5=yes; aggbase=-1; tpn=-1", maxaggtasks);
            if (NULL==(props=ss_blob_init_2pio(props, tmp))) SS_ERROR(FAILED);
        } else if (!strcmp(key, "minbufsize") || !strcmp(key, "alignment") || !strcmp(key, "aggbuflimit")) {
            /* Keys whose value is a non-negative number of bytes */
            if (0==(nl=strtol(value, &rest, 0)) && rest==value)
                SS_ERROR_FMT(USAGE, ("value is not an integer for %s term of SSLIB_2PIO: \"%s\"", key, value));
            if (!strcmp(rest, "k") || !strcmp(rest, "kB") || !strcmp(rest, "kb")) {
                nl *= 1024;
            } else if (!strcmp(rest, "M") || !strcmp(rest, "MB") || !strcmp(rest, "m") || !strcmp(rest, "mb")) {
                nl *= 1024*1024;
            } else if (!strcmp(rest, "G") || !strcmp(rest, "GB") || !strcmp(rest, "G") || !strcmp(rest, "gb")) {
                nl *= 1024*1024*1024;
            } else if (*rest) {
                SS_ERROR_FMT(USAGE, ("bad multiplier for %s term of SSLIB_2PIO: \"%s\"", key, rest));
            }
            if (nl<0) SS_ERROR_FMT(USAGE, ("value is negative for %s term of SSLIB_2PIO: \"%s\"", key, value));
            size = (size_t)nl;
            if (ss_prop_has(props, key)) {
                if (ss_prop_set(props, key, H5T_NATIVE_SIZE, &size)<0) SS_ERROR(FAILED);
            } else {
                if (ss_prop_add(props, key, H5T_NATIVE_SIZE, &size)<0) SS_ERROR(FAILED);
            }
        } else if (!strcmp(key, "maxaggtasks") || !strcmp(key, "sendqueue") ||
                   !strcmp(key, "aggbase") || !strcmp(key, "tpn")) {
            /* Keys whose value is an integer */
            if ((0==(nl=strtol(value, &rest, 0)) && rest==value) || *rest)
                SS_ERROR_FMT(USAGE, ("value is not an integer for %s term of SSLIB_2PIO: \"%s\"", key, value));
            i = (int)nl;
            if (ss_prop_has(props, key)) {
                if (ss_prop_set(props, key, H5T_NATIVE_INT, &i)<0) SS_ERROR(FAILED);
            } else {
                if (ss_prop_add(props, key, H5T_NATIVE_INT, &i)<0) SS_ERROR(FAILED);
            }
        } else if (!strcmp(key, "asynchdf5")) {
            /* Keys whose value is Boolean */
            if (!strcmp(value, "on") || !strcmp(value, "yes") || !strcmp(value, "true")) {
                i = TRUE;
            } else if (!strcmp(value, "off") || !strcmp(value, "no") || !strcmp(value, "false")) {
                i = FALSE;
            } else if (!*value) {
                SS_ERROR_FMT(USAGE, ("value expected for %s term of SSLIB_2PIO", key));
            } else if (*value) {
                SS_ERROR_FMT(USAGE, ("invalid Boolean value for %s term of SSLIB_2PIO: \"%s\"", key, value));
            }
            if (ss_prop_has(props, key)) {
                if (ss_prop_set(props, key, H5T_NATIVE_INT, &i)<0) SS_ERROR(FAILED);
            } else {
                if (ss_prop_add(props, key, H5T_NATIVE_INT, &i)<0) SS_ERROR(FAILED);
            }
        } else {
            SS_ERROR_FMT(USAGE, ("unknown key in SSLIB_2PIO: \"%s\"", key));
        }
    }

SS_CLEANUP:
    if (props_here) ss_prop_dest(props_here);
    SS_LEAVE(props);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Set two-phase I/O properties
 *
 * Description: Sets properties for two-phase I/O for the HDF5 file associated with the specified blob. If no blob is supplied then
 *              the library-wide defaults are adjusted.  Only the parameters present in the property list are adjusted. If
 *              called with null pointers for both BLOB and PROPS then the library-wide defaults are initialized from the
 *              SSLIB_2PIO environment variable.
 *
 *              The SSLIB_2PIO environment variable should be the word `yes' (or `on') which causes SSlib to use default
 *              values for two-phase I/O, the word `off' (or `no') in order to turn off two-phase I/O, or a
 *              semicolon-separated list of terms of the form KEY=VALUE. The KEY names are as follows and are the same as the
 *              names that could appear in the property list.
 *
 *              * !minbufsize:     The value should be an integer that specifies the minimum size in bytes to use for
 *                                 each aggregation buffer.  The actual aggregation buffer size is approximated by dividing
 *                                 the dataset size by the number of aggregators, subject to the alignment specified below.
 *                                 The default is 512kB, which is the GPFS page size on LLNL's AIX systems.
 *
 *              * !alignment:      The aggregation tasks will each be responsible for a dataset part which is some multiple
 *                                 of the alignment size.  However, SSlib will ignore the alignment under certain conditions
 *                                 when the alignment isn't a multiple of the dataset element size. The default is 512kB,
 *                                 which causes aggregation buffers to align on GPFS page boundaries on LLNL systems.
 *
 *              * !maxaggtasks:    The maximum number of aggregation tasks to use for each dataset. The actual number is
 *                                 approximated by dividing the dataset size by the !minbufsize, and limiting that by
 *                                 the !maxaggtasks value.  The default is to use 1/32 of the total MPI tasks, rouned up.
 *
 *              * !sendqueue:      This is the maximum number of buffers that can be held by any task during the data
 *                                 shipping phase of two-phase I/O. The default is four.  If more than four buffers are
 *                                 requested then SSlib will block pending completion of one of the previous asynchronous
 *                                 MPI_Isend() operations.
 *
 *              * !aggbuflimit:    This is the maximum number of bytes that can be used for all aggregation buffers
 *                                 across all files on a particular MPI task. An operation that would result in more than this
 *                                 amount being allocated by SSlib will cause SSlib to block until some asynchronous
 *                                 MPI_Irecv() and H5Dwrite() calls complete.  The default is 10MB.
 *
 *              * !asynchdf5:      The value is a Boolean that specifies whether SSlib should attempt to use POSIX.1b
 *                                 asynchronous I/O (AIO) in HDF5's mpiposix virtual file driver. Doing so currently
 *                                 requires a small patch to HDF5. The default is to attempt AIO.
 *
 *              * !aggbase:        SSlib chooses aggregators by taking a base MPI task and adding multiples of some
 *                                 aggregator increment modulo the number of tasks. The !aggbase term specifies how to
 *                                 choose the base aggregator. It can be the rank of a particular MPI task or the value -1,
 *                                 which indicates that the base aggregator is chosen by hashing the dataset's HDF5 object
 *                                 header address.  This is normally used only for debugging.
 *
 *              * !tpn:            Tasks per node, used to determine what MPI tasks serve as aggregators for a particular
 *                                 dataset. If unspecified (or non-positive) then SSlib uses an algorithm that attempts to
 *                                 distribute aggregators for a particular MPP architecture assuming 4 or 16 tasks per node.
 *
 *              The keys that take a size argument can consist of an integer value followed by an optional multiplier suffix
 *              which may be any one of kB (or k, kb), MB (or M, m, mb), or GB (or G, g, gb) to indicate 2^10, 2^20, or 2^30.
 *              Suffixes can only be used with the environment variable; property lists always specify values in terms of
 *              bytes.
 *
 *              Keys which take a Boolean value can be set to "yes", "on", "true", "no", "off", or "false" for the environment
 *              variable; Boolean properties always have an integer value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    When setting values for a particular file the call must be collective across the blob's file communicator,
 *              with the non-blob-owning tasks passing the blob's top-level scope instead. When setting library-wide defaults
 *              (BLOB is the null pointer) then the call must be collective across the library communicator. In any case all
 *              tasks must pass identical properties.
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_set_2pio(ss_blob_t *blob,       /* Optional blob to which whose file these settings apply. If no blob is
                                         * specified then the settings apply to SSlib in general and can be overridden
                                         * by individual blobs. */
                 ss_prop_t *props       /* See [Aggregation Properties]. */
                 )
{
    SS_ENTER(ss_blob_set_2pio, herr_t);
    int                 self, i;
    size_t              size;
    ss_blob_2pio_t      *agg=NULL;
    ss_scope_t          topscope=SS_SCOPE_NULL;
    ss_gblob_t          *gblob=NULL;
    char                *s=NULL;
    
    if (!blob && !props) {
        /* Broadcast SSLIB_2PIO environment variable from library task zero to all other library tasks */
        if ((self=ss_mpi_comm_rank(sslib_g.comm))<0) SS_ERROR(FAILED);
        if (0==self) {
            const char *s_const = getenv("SSLIB_2PIO");
            size = s_const ? strlen(s_const)+1 : 0;
            if (size) {
                if (NULL==(s=malloc(size))) SS_ERROR(RESOURCE);
                strcpy(s, s_const);
            }
        }
        if (ss_mpi_bcast(&size, 1, MPI_SIZE_T, 0, sslib_g.comm)<0) SS_ERROR(FAILED);
        if (size) {
            if (!s && NULL==(s=malloc(size))) SS_ERROR(RESOURCE);
            if (ss_mpi_bcast(s, size, MPI_CHAR, 0, sslib_g.comm)<0) SS_ERROR(FAILED);
        }

        /* Parse the SSLIB_2PIO value to create a property list */
        if (NULL==(props=ss_blob_init_2pio(NULL, "defaults"))) SS_ERROR(FAILED);
        if (NULL==(props=ss_blob_init_2pio(props, s))) SS_ERROR(FAILED);

        /* Use the property list to set properties */
        if (ss_blob_set_2pio(blob, props)<0) SS_ERROR(FAILED);
        if (ss_prop_dest(props)<0) SS_ERROR(FAILED);
        props = NULL;
        s = SS_FREE(s);

    } else {
        SS_ASSERT_TYPE(props, ss_prop_t);
        if (!blob) {
            agg = &ss_blob_2pio_g;
        } else {
            if (!ss_mpi_extras((ss_pers_t**)&blob, &topscope)) SS_ERROR(FAILED);
            gblob = SS_GFILE_LINK(&topscope)->gblob;
            if (gblob->a) SS_ERROR_FMT(USAGE, ("cannot set file properties after I/O occurs"))
            agg = &(gblob->agg);
        }
        if (ss_prop_get(props, "minbufsize",  H5T_NATIVE_SIZE, &size)) agg->minbufsize  = size;
        if (ss_prop_get(props, "alignment",   H5T_NATIVE_SIZE, &size)) agg->alignment   = size;
        if (ss_prop_get(props, "maxaggtasks", H5T_NATIVE_SIZE, &size)) agg->maxaggtasks = size;
        if (ss_prop_get(props, "sendqueue",   H5T_NATIVE_SIZE, &size)) agg->sendqueue   = size;
        if (ss_prop_get(props, "aggbuflimit", H5T_NATIVE_SIZE, &size)) agg->aggbuflimit = size;
        if (ss_prop_get(props, "asynchdf5",   H5T_NATIVE_INT,  &i))    agg->asynchdf5   = i ? TRUE : FALSE;
        if (ss_prop_get(props, "aggbase",     H5T_NATIVE_INT,  &i))    agg->aggbase     = i;
        if (ss_prop_get(props, "tpn",         H5T_NATIVE_INT,  &i))    agg->tpn         = i;
        SS_STATUS_OK; /*ignore failures from ss_prop_get() calls*/
    }
    
 SS_CLEANUP:
    SS_FREE(s);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     I/O
 * Purpose:     Get two-phase I/O properties
 *
 * Description: This is the inverse of ss_blob_set_2pio().  The only purpose of the BLOB argument is to obtain the file
 *              information where the two-phase I/O properties are kept, and therefore a scope can be passed instead.
 *
 * Return:      Returns a property list on success (either PROPS or a new one); null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, September  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_prop_t *
ss_blob_get_2pio(ss_blob_t *blob,       /* Optional blob whose file settings are to be retreived. If no blob is specified then
                                         * the library-wide defaults are returned. */
                 ss_prop_t *props       /* Optional property list to hold the results. Only the properties defined in the list
                                         * will be returned. If no property list is specified then a new one is created and
                                         * all two-phase I/O properties are returned. See [Aggregation Properties]. */
                 )
{
    SS_ENTER(ss_blob_get_2pio, ss_prop_tP);
    ss_blob_2pio_t      *agg=NULL;
    ss_scope_t          topscope=SS_SCOPE_NULL;
    ss_gblob_t          *gblob=NULL;

    /* Get the property list */
    if (!props) {
        if (NULL==(props=ss_blob_init_2pio(NULL, "defaults"))) SS_ERROR(FAILED);
    } else {
        SS_ASSERT_TYPE(props, ss_prop_t);
    }

    /* Get the property values */
    if (!blob) {
        agg = &ss_blob_2pio_g;
    } else {
        if (!ss_mpi_extras((ss_pers_t**)&blob, &topscope)) SS_ERROR(FAILED);
        gblob = SS_GFILE_LINK(&topscope)->gblob;
        agg = &(gblob->agg);
    }

    /* Set the property values */
    if (ss_prop_has(props, "minbufsize")  && ss_prop_set_u(props, "minbufsize",  agg->minbufsize)<0)  SS_ERROR(FAILED);
    if (ss_prop_has(props, "alignment")   && ss_prop_set_u(props, "alignment",   agg->alignment)<0)   SS_ERROR(FAILED);
    if (ss_prop_has(props, "maxaggtasks") && ss_prop_set_u(props, "maxaggtasks", agg->maxaggtasks)<0) SS_ERROR(FAILED);
    if (ss_prop_has(props, "sendqueue")   && ss_prop_set_u(props, "sendqueue",   agg->sendqueue)<0)   SS_ERROR(FAILED);
    if (ss_prop_has(props, "aggbuflimit") && ss_prop_set_u(props, "aggbuflimit", agg->aggbuflimit)<0) SS_ERROR(FAILED);
    if (ss_prop_has(props, "asynchdf5")   && ss_prop_set_i(props, "asynchdf5",   agg->asynchdf5)<0)   SS_ERROR(FAILED);
    if (ss_prop_has(props, "aggbase")     && ss_prop_set_i(props, "aggbase",     agg->aggbase)<0)     SS_ERROR(FAILED);
    if (ss_prop_has(props, "tpn")         && ss_prop_set_i(props, "tpn",         agg->tpn)<0)         SS_ERROR(FAILED);
    
 SS_CLEANUP:
    SS_LEAVE(props);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Create or read blob group
 *
 * Description: All blob datasets have names in a blob group and those names are based on the HDF5 object header address for
 *              the dataset.  This function either creates a new empty group or opens all the blob datasets in that group,
 *              adding them to the global blob table (ss_gblob_g).
 *
 * Return:      Returns a pointer to a new blob dataset table (gblob) on success; negative on failure.
 *
 * Parallel:    Collective across the file communicator because we're making collective HDF5 calls in order to create or open
 *              the blob groups and/or datasets.  If HDF5 had a more independent API then opening blob datasets could be
 *              delayed until they were actually being read.
 *
 * Programmer:  Robb Matzke
 *              Monday, September  8, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_gblob_t *
ss_blob_boot(ss_scope_t *topscope, hbool_t create)
{
    SS_ENTER(ss_blob_boot, ss_gblob_tP);
    ss_gblob_t  *retval=NULL;
    int         idx=0;

    if (ss_scope_isopentop(topscope)<=0) SS_ERROR_FMT(USAGE, ("topscope argument is not an open top scope"));
    if (NULL==(retval=SS_OBJ_NEW(ss_gblob_t))) SS_ERROR(RESOURCE);

    /* Load information about all blob datasets because much of this is file-collective */
    if (SS_SCOPE(topscope)->m.gid>1) { /* one implies transient file, nothing to do */
        if (create) {
            if ((retval->storage=H5Gcreate(SS_SCOPE(topscope)->m.gid, SS_BLOB_GRPNAME, 0))<0) SS_ERROR(HDF5);
        } else {
            if ((retval->storage=H5Gopen(SS_SCOPE(topscope)->m.gid, SS_BLOB_GRPNAME))<0) SS_ERROR(HDF5);
            if (H5Giterate(retval->storage, ".", &idx, ss_blob_boot_cb, retval)<0) SS_ERROR(HDF5);
        }
    }

    /* Initialize 2-phase I/O properties from library defaults */
    retval->agg = ss_blob_2pio_g;

 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Destroy a gblob table
 *
 * Description: When a scope is destroyed we must also destroy the blob dataset table (gblob) and this function does that.
 *              This function does not attempt to flush data first -- that should have already happened.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. In order to be independent this function does not close the datasets associated with blobs.
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_desttab(ss_gblob_t *gblob               /* Gblob table to be destroyed. This memory is also freed since ss_blob_boot()
                                                 * allocated it. */                                      
                )
{
    SS_ENTER(ss_blob_desttab, herr_t);
    size_t              i, j;

    if (gblob) {
        SS_ASSERT_TYPE(gblob, ss_gblob_t);

        /* The `d' array */
        for (i=0; i<gblob->d_nused; i++) {
            if (gblob->d[i].dset>0) H5Dclose(gblob->d[i].dset);
            if (gblob->d[i].dtype>0) H5Tclose(gblob->d[i].dtype);
            if (gblob->d[i].dspace>0) H5Sclose(gblob->d[i].dspace);
            SS_FREE(gblob->d[i].agg.tasks);
            SS_FREE(gblob->d[i].agg.aggbuf_freeable);
            SS_FREE(gblob->d[i].agg.recv);
            SS_FREE(gblob->d[i].agg.send);
            for (j=0; j<gblob->d[i].agg.sendbufs_nused; j++) {
                SS_FREE(gblob->d[i].agg.sendbufs[j]);
            }
            if (gblob->d[i].agg.iom>0) H5Sclose(gblob->d[i].agg.iom);
            if (gblob->d[i].agg.iof>0) H5Sclose(gblob->d[i].agg.iof);
        }
        gblob->d = SS_FREE(gblob->d);
        gblob->d_nused = gblob->d_nalloc = 0;

        /* The `a' array. See ss_blob_async_append(). The data type and spaces here were not duplicated and therefore do not
         * need to be closed. */   
        gblob->a = SS_FREE(gblob->a);
        gblob->a_nused = gblob->a_nalloc = 0;

        /* The gblob itself */
        if (gblob->storage>0) H5Gclose(gblob->storage);

        SS_OBJ_DEST(gblob);
        gblob = SS_FREE(gblob);
    }
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Blob boot callback
 *
 * Description: This is an H5G iterator called from ss_blob_boot() in order to open all blob storage datasets.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the file communicator.
 *
 * Programmer:  Robb Matzke
 *              Monday, September  8, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_boot_cb(hid_t group, const char *name, void *_gblob)
{
    SS_ENTER(ss_blob_boot_cb, herr_t);
    ss_gblob_t          *gblob = (ss_gblob_t*)_gblob;
    const char          *s;
    size_t              n;
    hid_t               dset_duped=-1, dtype_duped=-1, dspace_duped=-1;
    haddr_t             objno;

    /* Extend the gblob dataset table */
    SS_EXTEND(gblob->d, MAX(64,gblob->d_nused+1), gblob->d_nalloc);
    n = gblob->d_nused; /* increment when we know we'll succeed */
    memset(gblob->d+n, 0, sizeof(gblob->d[0]));
    
    /* Verify that name matches a blob dataset name pattern: only digits */
    for (s=name; *s; s++) {
        if (!isdigit(*s)) SS_ERROR_FMT(CORRUPT, ("blob storage group has strange dataset name: %s", name));
    }

    /* Open the dataset and match up the object header address with the dataset name */
    if (H5Gget_objinfo(group, name, FALSE, &(gblob->d[n].stat))<0) SS_ERROR(HDF5);
    if (H5G_DATASET!=gblob->d[n].stat.type) SS_ERROR_FMT(CORRUPT, ("object `%s' in blob storage group is not a dataset", name));
    objno = strtol(name, NULL, 0);
    if (objno!=*((haddr_t*)(&gblob->d[n].stat.objno)[0])) SS_ERROR_FMT(CORRUPT, ("blob dataset `%s' name is not ohdr address", name));
    if ((gblob->d[n].dset = dset_duped = H5Dopen(group, name))<0) SS_ERROR(FAILED);

    /* Get the dataset type and space because we might need it in an independent situations later but H5Dget_type() and
     * H5Dget_space() are file-collective. */
    if ((gblob->d[n].dtype = dtype_duped = H5Dget_type(dset_duped))<0) SS_ERROR(HDF5);
    if ((gblob->d[n].dspace = dspace_duped = H5Dget_space(dset_duped))<0) SS_ERROR(HDF5);

    gblob->d_nused++;
 SS_CLEANUP:
    if (dset_duped>0) H5Dclose(dset_duped);
    if (dtype_duped>0) H5Tclose(dtype_duped);
    if (dspace_duped>0) H5Sclose(dspace_duped);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Check data space extent and selection
 *
 * Description: This function checks that a data space extent and selection are simple, not more than MAXDIMS in
 *              dimensionality, and that the current selection is contiguous.
 *
 * Return:      Returns actual dimensionality on success, with initial elements of the output arguments initialized. Returns
 *              negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, September  5, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_blob_ckspace(hid_t space,                    /* HDF5 dataspace to be checked */
                int maxdims,                    /* Maximum allowed dimensionality */
                hsize_t *size,                  /* OUT: Optional data space extent, one element per dimension */
                hsize_t *start,                 /* OUT: Optional data selection startinf offsets, one element per dimension */
                hsize_t *count,                 /* OUT: Optional data selection counts, one element per dimension */
                hsize_t *nelmts                 /* OUT: Optional number of selected elements */
                )
{
    SS_ENTER(ss_blob_ckspace, int);
    int         ndims=0, i;
    hsize_t     size_here[H5S_MAX_RANK];
    hsize_t     corners[2*H5S_MAX_RANK];

    if (H5Sis_simple(space)<=0) SS_ERROR_FMT(NOTIMP, ("not a simple data space"));
    if ((ndims = H5Sget_simple_extent_ndims(space))<0) SS_ERROR(HDF5);
    if (ndims > maxdims) SS_ERROR_FMT(OVERFLOW, ("data space dimension(%d) is too large", ndims));
    if (!size) size = size_here;
    if (H5Sget_simple_extent_dims(space, size, NULL)<0) SS_ERROR(HDF5);
        
    switch (H5Sget_select_type(space)) {
    case H5S_SEL_ERROR:
        SS_ERROR(HDF5);
    case H5S_SEL_NONE:
        if (start) memset(start, 0, ndims*sizeof(*start));
        if (count) memset(count, 0, ndims*sizeof(*count));
        break;
    case H5S_SEL_ALL:
        if (start) memset(start, 0, ndims*sizeof(*start));
        if (count) for (i=0; i<ndims; i++) count[i] = size[i];
        break;
    case H5S_SEL_POINTS:
        SS_ERROR_FMT(NOTIMP, ("point list selection"));
    case H5S_SEL_HYPERSLABS:
        if (1!=H5Sget_select_hyper_nblocks(space))
            SS_ERROR_FMT(NOTIMP, ("memory selection is not contiguous"));
        if (H5Sget_select_hyper_blocklist(space, (hsize_t)0, (hsize_t)1, corners)<0) SS_ERROR(HDF5);
        for (i=0; i<ndims; i++) {
            if (start) start[i] = corners[i];
            if (count) count[i] = (corners[ndims+i] - corners[i]) + 1;
        }
        break;
    default:
        SS_ERROR_FMT(NOTIMP, ("unexpected return value from HDF5"));
    }

    if (nelmts) {
        hssize_t n = H5Sget_select_npoints(space);
        if (n<0) SS_ERROR(HDF5);
        *nelmts = (hsize_t)n;
    }
    
 SS_CLEANUP:
    SS_LEAVE(ndims);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Check file/memory data space compatibility
 *
 * Description: Verify that the blob's dataset data space is compatible with its memory data space (if it's bound to memory)
 *              and that the blob's data space is compatible with the dataset data space.  On success, return the dataset data
 *              space with a selection based on the blob's selection.
 *
 *              Another way of saying this is: there are three data spaces involved with an I/O operation:
 *
 *              * The dataset data space describes the extent of the dataset bound to the blob. A selection on that data space
 *                can describe either the elements owned by the blob or the subset thereof describing the elements to be
 *                operated upon.
 *
 *              * The blob data space describes the extent of the blob and its dimensionality can be smaller than that of
 *                the dataset where the blob's data is stored.
 *
 *              * The memory data space describes the memory buffer that serves as the source or destination for the I/O
 *                operation.  It's dimensionality should match the blob's dimensionality.
 *
 * Return:      Returns non-negative on success, negative on failure.  The IOM and IOF return values will be invalid handles
 *              if there is insufficient information to compute the data space or an error occurs.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, September  5, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_ckspaces(ss_blob_t *blob,               /* Blob whose data spaces are being checked. */
                 hid_t iospace,                 /* Optional extent of blob and selection of elements on which to operate */
                 hid_t *iom,                    /* OUT: Optional returned data space that will describe memory elements on
                                                 * which to operate with a dataset data space suitable for passing to
                                                 * H5Dread() or H5Dwrite(). If the dataset is not bound to memory but is
                                                 * bound to a dataset then this space will describe contiguous memory to
                                                 * hold the result. */
                 hid_t *iof                     /* OUT: Optional returned data space that will describe dataset elements on
                                                 * which to operate with a dataset data space suitable for passing to
                                                 * H5Dread() or H5Dwrite(). */
                )
{
    SS_ENTER(ss_blob_ckspaces, herr_t);
    ss_gblob_t  *gblob=NULL;
    int         mem_ndims=-1, io_ndims=-1, blob_ndims=-1, dset_ndims=-1; /* dimensionalities */
    int         dset_dim, blob_dim;                             /* dimension counters */
    hsize_t     io_nsel, mem_nsel;                              /* elements selected in I/O and memory spaces */
    hsize_t     mem_size[SS_MAXDIMS];                           /* memory extent */
    hsize_t     mem_start[SS_MAXDIMS];                          /* memory selection starting offsets */
    hsize_t     mem_count[SS_MAXDIMS];                          /* memory selection counts */
    hsize_t     io_size[SS_MAXDIMS];                            /* I/O extent */
    hsize_t     io_start[SS_MAXDIMS];                           /* I/O selection starting offsets w.r.t. blob space */
    hsize_t     io_count[SS_MAXDIMS];                           /* I/O selection counts */
    hsize_t     dset_start[H5S_MAX_RANK];                       /* selection starting offsets w.r.t. dataset space */
    hsize_t     dset_count[H5S_MAX_RANK];                       /* selection counts w.r.t. dataset space */
    hsize_t     dset_size[H5S_MAX_RANK];                        /* extent per dataset dimension */
    hsize_t     blob_size[H5S_MAX_RANK];                        /* size of the blob per blob dimension */
    hid_t       iof_duped=-1, iom_duped=-1;                     /* return values for cleanup during error */
    
    SS_ASSERT_MEM(blob, ss_blob_t);
    gblob = SS_GFILE_LINK(blob)->gblob;

    /* Set IOM and IOF to invalid handles in case of error */
    if (iom) *iom = -1;
    if (iof) *iof = -1;

    /* No need to check the dataset data space/selection stored in the blob as offsets and sizes, but we might need some
     * information about the blob and dataset. */
    if (SS_BLOB(blob)->dsetaddr>0) {
        if ((blob_ndims = ss_blob_space(blob, blob_size, NULL))<0) SS_ERROR(FAILED);
        if ((dset_ndims = ss_blob_bound_f(blob, NULL, NULL, NULL, NULL, NULL))<0) SS_ERROR(FAILED);
    }
    
    /* If a memory data space is present then check that it is valid in and of itself. This should have already been checked
     * when the memory was bound to the blob, but we call it here also for the return values. */
    if (SS_BLOB(blob)->m.mem &&
        (mem_ndims = ss_blob_ckspace(SS_BLOB(blob)->m.mspace, SS_MAXDIMS, mem_size, mem_start, mem_count, &mem_nsel))<0)
        SS_ERROR_FMT(FAILED, ("memory data space"));

    /* If an I/O data space is present then check that it is valid in and of itself. */
    if (iospace>0 &&
        (io_ndims = ss_blob_ckspace(iospace, SS_MAXDIMS, io_size, io_start, io_count, &io_nsel))<0)
        SS_ERROR_FMT(FAILED, ("I/O data space"));

    /* If an I/O space is supplied and the blob is bound to a dataset then check that the I/O space and blob space are
     * compatible. Furthermore, if the caller requested it, return a data space that selects the I/O elements of the dataset. */
    if (iospace>0 && SS_BLOB(blob)->dsetaddr) {
        SS_ASSERT(io_ndims>=0 && blob_ndims>=0);
        if (io_ndims!=blob_ndims) SS_ERROR_FMT(NOTIMP, ("I/O(%d) and blob(%d) dimensionality differ", io_ndims, blob_ndims));
        for (dset_dim=blob_dim=0; dset_dim<dset_ndims; dset_dim++) {
            if (1==SS_BLOB(blob)->count[dset_dim]) {
                /* No corresponding blob dimension for this dataset dimension */
                dset_start[dset_dim] = SS_BLOB(blob)->start[dset_dim];
                dset_count[dset_dim] = 1;
            } else {
                /* Dataset dimension corresponds to a blob dimension */
                if (io_start[blob_dim] + io_count[blob_dim] > SS_BLOB(blob)->count[dset_dim])
                    SS_ERROR_FMT(OVERFLOW,
                                 ("I/O selection(dim=%d,start=%lu,count=%lu) extends past blob extent(dim=%d,size=%lu)",
                                  blob_dim, (unsigned long)(io_start[blob_dim]), (unsigned long)(io_count[blob_dim]),
                                  dset_dim, (unsigned long)(SS_BLOB(blob)->count[dset_dim])));
                dset_start[dset_dim] = SS_BLOB(blob)->start[dset_dim] + io_start[blob_dim];
                dset_count[dset_dim] = io_count[blob_dim];
                blob_dim++;
            }
        }

        if (iof) {
            if ((*iof = iof_duped = H5Scopy(gblob->d[SS_BLOB(blob)->m.d_idx].dspace))<0) SS_ERROR(HDF5);
            SS_ASSERT(H5Sget_simple_extent_ndims(iof_duped)==dset_ndims);
            if (H5Sselect_slab(iof_duped, H5S_SELECT_SET, (hsize_t)0, dset_start, dset_count)<0)
                SS_ERROR(HDF5);
        }
    } else if (iof) {
        SS_ERROR_FMT(USAGE, ("not possible to compute IOF without IOSPACE or dataset binding"));
    }
    
    /* If the blob is bound to both memory and a dataset then pad out the memory data space information so it has the same
     * dimensionality as the blob data space. This is necessary because the blob dimensionality is inferred from the dataset
     * selection describing the blob's position in the dataset and the memory data space is normalized before storing it. It
     * is valid to extend the memory data space by inserting dimensions of size one at any location in the sequence. It is
     * possible to extend the memory data space by inserting unit-size dimensions provided the memory data space selection was
     * originally compatible with the blob data space extent. */
    if (SS_BLOB(blob)->m.mem && SS_BLOB(blob)->dsetaddr) {
        SS_ASSERT(mem_ndims>=0 && blob_ndims>=0);
        if (1==mem_ndims && 0==mem_count[0]) {
            /* The memory space is the null space */
            mem_ndims = blob_ndims;
            memset(mem_size, 0, sizeof mem_size);
            memset(mem_start, 0, sizeof mem_start);
            memset(mem_count, 0, sizeof mem_count);
        } else {
            /* Pad the memory space out to the same dimensionality as the blob space */
            for (blob_dim=0; blob_dim<blob_ndims; blob_dim++) {
                if (blob_dim>=mem_ndims) {
                    mem_size[blob_dim] = 1;
                    mem_start[blob_dim] = 0;
                    mem_count[blob_dim] = 1;
                    mem_ndims++;
                } else if (mem_count[blob_dim]>blob_size[blob_dim]) {
                    if (mem_ndims>=SS_MAXDIMS)
                        SS_ERROR_FMT(OVERFLOW, ("memory selection is not compatible with blob size"));
                    memmove(mem_size+blob_dim+1,  mem_size+blob_dim,  (mem_ndims-blob_dim)*sizeof(mem_size[0]));
                    memmove(mem_start+blob_dim+1, mem_start+blob_dim, (mem_ndims-blob_dim)*sizeof(mem_start[0]));
                    memmove(mem_count+blob_dim+1, mem_count+blob_dim, (mem_ndims-blob_dim)*sizeof(mem_count[0]));
                    mem_ndims++;
                    mem_size[blob_dim] = 1;
                    mem_start[blob_dim] = 0;
                    mem_count[blob_dim] = 1;
                }
            }
        }
        if (mem_ndims!=blob_ndims) SS_ERROR_FMT(NOTIMP, ("memory(%d) and blob(%d) dimensionality differ", mem_ndims, blob_ndims));
    }
    
    /* If an I/O space is supplied and the blob is bound to memory then check that the I/O space and memory space are
     * compatible. Furthermore, if the client requested it, return a data space for memory that has been expanded to the
     * dataset dimensionality. */
    if (iospace>0 && SS_BLOB(blob)->m.mem) {
        SS_ASSERT(io_ndims>=0 && mem_ndims>=0);
        if (io_ndims!=mem_ndims)
            SS_ERROR_FMT(NOTIMP, ("I/O(%d) and memory(%d) dimensionality differ", io_ndims, mem_ndims));
        if (io_nsel!=mem_nsel)
            SS_ERROR_FMT(NOTIMP, ("I/O(%lu) and memory(%lu) selections contain different amounts of data",
                                  (unsigned long)io_nsel, (unsigned long)mem_nsel));
        if (iom) {
            for (dset_dim=blob_dim=0; dset_dim<dset_ndims; dset_dim++) {
                if (1==SS_BLOB(blob)->count[dset_dim]) {
                    /* No corresponding blob dimension for this dataset dimension */
                    dset_start[dset_dim] = 0;
                    dset_count[dset_dim] = 1;
                    dset_size[dset_dim] = 1;
                } else {
                    /* Dataset dimension corresponds to a blob dimension */
                    dset_start[dset_dim] = mem_start[blob_dim];
                    dset_count[dset_dim] = mem_count[blob_dim];
                    dset_size[dset_dim] = mem_size[blob_dim];
                    SS_ASSERT(mem_start[blob_dim] + mem_count[blob_dim] <= mem_size[blob_dim]); /*caught by HDF5 earlier?*/
                    blob_dim++;
                }
            }
            /* Must use ss_blob_unlim_g because dset_size might be zero */
            if ((*iom = iom_duped = H5Screate_simple(dset_ndims, dset_size, ss_blob_unlim_g))<0) SS_ERROR(HDF5);
            if (H5Sselect_slab(iom_duped, H5S_SELECT_SET, (hsize_t)0, dset_start, dset_count)<0)
                SS_ERROR(HDF5);
        }
    }

    /* If the client requested an I/O space for memory and we haven't computed one yet because the blob is not bound to
     * memory, then compute it now if the blob is bound to a dataset. */
    if (iom && *iom<=0 && iospace>0 && SS_BLOB(blob)->dsetaddr) {
        for (dset_dim=blob_dim=0; dset_dim<dset_ndims; dset_dim++) {
            if (1==SS_BLOB(blob)->count[dset_dim]) {
                dset_count[dset_dim] = 1;
            } else {
                dset_count[dset_dim] = io_count[blob_dim++];
                if (0==dset_count[dset_dim]) {
                    dset_ndims = 1;
                    dset_count[0] = 0;
                    break;
                }
            }
        }
        if ((*iom = iom_duped = H5Screate_simple(dset_ndims, dset_count, ss_blob_unlim_g))<0) SS_ERROR(HDF5);
    }
            
 SS_CLEANUP:
    if (iof_duped>0) H5Sclose(iof_duped);
    if (iom_duped>0) H5Sclose(iom_duped);
    if (iom) *iom = -1;
    if (iof) *iof = -1;
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Normalize a data space
 *
 * Description: Given a data space (and selection) create a new data space that is exactly the same as the original except
 *              with all unit-size dimensions removed.  If the SPACE contains any dimension with zero size then the normalized
 *              return value will be a one-dimensional space of size zero.
 *
 *              The reason this works is because it's always possible to remove or insert dimensions of size one without
 *              affecting the element ordering or the inter-element spacing. The selection is also not affected as long as the
 *              dimensions that are removed had their (single) index value selected.
 *
 * Return:      On success, returns a new, normalized data space; return negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June 23, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
ss_blob_normalize(hid_t space)
{
    SS_ENTER(ss_blob_normalize, hid_t);
    hid_t       retval=-1;                      /* The new, normalized data space */
    int         ndims;                          /* Data space dimensionality */
    hsize_t     size[H5S_MAX_RANK];             /* Extent of the data space */
    hsize_t     start[H5S_MAX_RANK];            /* Offset to start of selection in each dimension of the data space */
    hsize_t     count[H5S_MAX_RANK];            /* Number of selected elements in each dimension of the data space */
    hsize_t     nselected;                      /* Number of elements selected */
    int         i, j;
    hbool_t     had_zero=FALSE;

    /* Get non-normalized extent and selection info */
    if ((ndims=ss_blob_ckspace(space, H5S_MAX_RANK, size, start, count, &nselected))<0) SS_ERROR(FAILED);

    /* Normalize the extent and selection info */
    for (i=j=0; i<ndims && !had_zero; i++) {
        if (size[i]>1) {
            size[j] = size[i];
            start[j] = start[i];
            count[j] = count[i];
            j++;
        } else if (0==size[i]) {
            had_zero = TRUE;
        }
    }
    ndims = had_zero ? -1 : j;

    /* Create a new, normalized data space */
    if (ndims<0) {
        /* The null space, represented by a 1d space of size zero */
        SS_ASSERT(nselected==0);
        if ((retval=H5Screate_simple(1, &nselected, ss_blob_unlim_g))<0) SS_ERROR(HDF5);
        if (H5Sselect_none(retval)<0) SS_ERROR(HDF5);
    } else if (0==ndims) {
        /* A scalar space */
        if ((retval=H5Screate_simple(ndims, size, NULL))<0) SS_ERROR(HDF5);
        SS_ASSERT(0==nselected || 1==nselected);
        if (nselected) {
            if (H5Sselect_all(retval)<0) SS_ERROR(HDF5);
        } else {
            if (H5Sselect_none(retval)<0) SS_ERROR(HDF5);
        }
    } else {
        retval = H5Screate_simple(ndims, size, NULL);
        if (H5Sselect_slab(retval, H5S_SELECT_SET, (hsize_t)0, start, count)<0) SS_ERROR(HDF5);
    }

SS_CLEANUP:
    if (retval>0) H5Sclose(retval);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Convert slab to strides
 *
 * Description: Given a desription of a slab (starting offset and counts) in a dataset, return the same information in terms
 *              of a stride vector.  This idea comes from HDF5's H5V interface.
 *
 * Return:      Returns non-negative on success; negative on failure. Successful return also causes the stride to be returned
 *              through the caller supplied STRIDE argument.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, October 24, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_stride(int ndims,                       /* Number of dimensions for the slab and its containing dataset or array. */
               const hsize_t *dset_size,        /* The total size of the dataset or array. */
               const hsize_t *slab_start,       /* The offset into the dataset for the beginning of the slab. */
               const hsize_t *slab_count,       /* The size of the slab in each dataset dimension */
               ss_blob_stride_t *s              /* OUT: The returned stride. */
               )
{
    SS_ENTER(ss_blob_stride, herr_t);
    int         i;
    hsize_t     acc;

    SS_ASSERT(ndims>0);

    /* Initialize the stride object with the fastest varying dimension */
    memset(s, 0, sizeof *s);
    s->ndims = ndims;
    s->stride[ndims-1] = 1;
    s->offset = slab_start[ndims-1];
    s->count[ndims-1] = slab_count[ndims-1];
    s->duplicity = 1;

    /* Compute the stride based on the remaining dimensions */
    for (i=ndims-2, acc=1; i>=0; --i) {
        SS_ASSERT(slab_start[i+1]+slab_count[i+1] <= dset_size[i+1]);
        s->stride[i] = acc * (dset_size[i+1] - slab_count[i+1]);
        s->count[i] = slab_count[i];
        acc *= dset_size[i+1];
        s->offset += acc * slab_start[i];
    }

    /* Optimize the stride by combining adjacent memory accesses */
    while (s->ndims>0 && s->stride[s->ndims-1]>0 && s->stride[s->ndims-1]==s->duplicity) {
        s->duplicity *= s->count[s->ndims-1];
        if (--s->ndims) s->stride[s->ndims-1] += s->count[s->ndims] * s->stride[s->ndims];
        s->stride[s->ndims] = s->count[s->ndims] = 0; /*just to keep things tidy*/
    }

    /* Optimize further by removing leading unit counts */
    while (s->ndims>0 && 1==s->count[0]) {
        for (i=1; i<s->ndims; i++) {
            s->count[i-1] = s->count[i];
            s->stride[i-1] = s->stride[i];
        }
        s->ndims -= 1;
        s->stride[s->ndims] = s->count[s->ndims] = 0; /*just to keep things tidy*/
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Create a one dimensional stride
 *
 * Description: This is a special case of ss_blob_stride() in that it operates only in one dimensional space.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, October 30, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_stride_1(hsize_t start, hsize_t count, ss_blob_stride_t *s)
{
    SS_ENTER(ss_blob_stride_1, herr_t);
    SS_ASSERT(count>0);
    SS_ASSERT(s);

    memset(s, 0, sizeof *s);
    s->ndims = 0;
    s->offset = start;
    s->duplicity = count;

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Copy data described by strides
 *
 * Description: Copies data from the SRC buffer to the DST buffer. The elements to be copied from the SRC buffer are described
 *              by the SRC_STRIDE while the destination elements in DST are described by DST_STRIDE. Both strides must
 *              describe the same number of elements.
 *
 * Return:      
 *
 * Parallel:    
 *
 * Programmer:  Robb Matzke
 *              Thursday, October 30, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_stride_copy(void *dst,                          /* The destination buffer */
                    const ss_blob_stride_t *dst_stride, /* Where to copy things into the destination buffer */
                    const void *src,                    /* The source buffer */
                    const ss_blob_stride_t *src_stride, /* What elements to copy from the source buffer */
                    size_t elmt_size                    /* Size in bytes of a single element */
                    )
{
    SS_ENTER(ss_blob_stride_copy, herr_t);
    char                *dstb=(char*)dst;               /* Byte addressable version of destination */
    const char          *srcb=(const char*)src;         /* Byte addressable version of source */
    size_t              dst_nelmts, src_nelmts;         /* Number of elements in destination and source buffers */
    hsize_t             src_idx[H5S_MAX_RANK];          /* Counter for source stride */
    hsize_t             dst_idx[H5S_MAX_RANK];          /* Counter for destination stride */
    size_t              elmtno;                         /* Element counter */
    size_t              src_at, dst_at;                 /* Duplicity counters */
    size_t              nelmts;                         /* Number of elements to copy each time through loop */
    hbool_t             carry;
    int                 i, dim;

    /* Count elements in source and destination and initialize counters */
    dst_nelmts = dst_stride->duplicity;
    for (dim=0; dim<dst_stride->ndims; dim++) {
        dst_nelmts *= dst_stride->count[dim];
        dst_idx[dim] = dst_stride->count[dim];
    }
    src_nelmts = src_stride->duplicity;
    for (dim=0; dim<src_stride->ndims; dim++) {
        src_nelmts *= src_stride->count[dim];
        src_idx[dim] = src_stride->count[dim];
    }
    if (dst_nelmts!=src_nelmts) SS_ERROR_FMT(USAGE, ("source and destination sizes don't agree"));

    /* Copy */
    dstb += dst_stride->offset * elmt_size;
    srcb += src_stride->offset * elmt_size;
    for (elmtno=0, src_at=dst_at=0; elmtno<src_nelmts; elmtno+=nelmts) {
        nelmts = MIN(dst_stride->duplicity-dst_at, src_stride->duplicity-src_at);
        memcpy(dstb+dst_at*elmt_size, srcb+src_at*elmt_size, elmt_size*nelmts);
        dst_at += nelmts;
        src_at += nelmts;

        if (dst_at == dst_stride->duplicity) {
            dst_at = 0;
            for (i=dst_stride->ndims-1, carry=TRUE; i>=0 && carry; --i) {
                dstb += dst_stride->stride[i] * elmt_size;
                if (--dst_idx[i]) carry = FALSE;
                else dst_idx[i] = dst_stride->count[i];
            }
        }
        if (src_at == src_stride->duplicity) {
            src_at = 0;
            for (i=src_stride->ndims-1, carry=TRUE; i>=0 && carry; --i) {
                srcb += src_stride->stride[i] * elmt_size;
                if (--src_idx[i]) carry = FALSE;
                else src_idx[i] = src_stride->count[i];
            }
        }
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Convert multidimensional index to linear index
 *
 * Description: Given a multidimensional index of some element in an array, this function will compute the linear index of
 *              that element assuming C ordering (the first dimension of an index varies the slowest while the last varies the
 *              fastest when traversing elements in memory order).
 *
 *              The values in START and COUNT need not be normalized. That is, if the total SIZE is (10,20) then it is legal
 *              to pass in a START that is (0,40) which is equivalent to a START of (2,0).  The only restriction is that the
 *              linear address is within the extent of the whole dataset or array. That is, a START of (9,20) is illegal
 *              because it's past the end of the whole array.
 *
 * Return:      Linear index of the specified multidimensional element.
 *
 * Parallel:    Independent
 *
 * Also:        ss_blob_array_multidim()
 *
 * Programmer:  Robb Matzke
 *              Wednesday, October 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hsize_t
ss_blob_array_linear(int ndims,                 /* Number of elements in following array arguments. */
                     const hsize_t *size,       /* Total size of the dataset into which we are indexing */
                     const hsize_t *start,      /* Element for which we are requesting a linear address, possibly offset
                                                 * by the COUNT argument. If this is null then assume a vector of all zero. */
                     const hsize_t *count       /* If this is non-null then it specifies the size of a slab whose starting
                                                 * corner is specified by START and whose opposite corner is the element for
                                                 * which we'll return a linear index. A null value for COUNT is the same as
                                                 * if the caller had supplied a vector of all 1's as the slab size. */
                     )
{
    SS_ENTER(ss_blob_array_linear, hsize_t);
    hsize_t             acc, linear=0;
    int                 i;

    SS_ASSERT(ndims>0 && ndims<=H5S_MAX_RANK);
    for (i=ndims-1, acc=1, linear=0; i>=0; --i) {
        SS_ASSERT(!count || count[i]>0);
        linear += ((start?start[i]:0) + (count?count[i]-1:0)) * acc;
        acc *= size[i];
    }

SS_CLEANUP:
    SS_LEAVE(linear);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Convert linear index to multidimensional index
 *
 * Description: This function is the inverse of ss_blob_array_linear() in that it takes a linear index into a multidimensional
 *              array and returns the equivalent multidimensional index.
 *
 * Return:      Returns non-negative on success with the multidimensional index stored in the caller-supplied MULTIDIM
 *              argument; returns negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, October 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_array_multidim(int ndims,               /* Number of elements in following array arguments. */
                       const hsize_t *size,     /* Total size of the dataset into which we are indexing. */
                       hsize_t linear,          /* The linear index into the array. */
                       hsize_t *multidim        /* OUT: The multidimensional equivalent of LINEAR. */
                       )
{
    SS_ENTER(ss_blob_array_multidim, herr_t);
    int         i;
    hsize_t     acc, down[H5S_MAX_RANK];

    SS_ASSERT(ndims>0 && ndims<=H5S_MAX_RANK);
    for (i=ndims-1, acc=1; i>=0; --i) {
        down[i] = acc;
        acc *= size[i];
    }
    for (i=0; i<ndims; i++) {
        multidim[i] = linear / down[i];
        linear %= down[i];
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Determine request and aggregator intersection
 *
 * Description: Given information about the ultimate location of a write request in the destination dataset (as
 *              multidimensional offsets and counts) and a task number to serve as an aggregator, determine if there is any
 *              intersection between the portion of the dataset where the request will land and the portion of the dataset for
 *              which said task is the aggregator.  If there is intersection then return information about it in terms of a
 *              set of ss_blob_stride_t structures whose storage is supplied by the caller.
 *
 * Example:     Given a two-dimensional dataset the !o characters represent the 2-dimensional slab from a single I/O request
 *              on some task while the !+ characters represent the part of the dataset for which some aggregator task
 *              is responsible. The !# characters are the intersection of the 2d slab with the aggregator's area of the
 *              dataset.
 *
 *                   ____________________
 *                  |....................|
 *                  |....................|
 *                  |....oooooo..........|
 *                  |....oooooo..........|
 *                  |....oooo##++++++++++|
 *                  |++++######++++++++++|
 *                  |++++######++++++++++|
 *                  |++++######+++++.....|
 *                  |....oooooo..........|
 *                  |....................|
 *                  +--------------------+
 *
 * Return:      Returns the number of strides that describe the intersection, zero if there is no intersection. If the dataset
 *              is N dimensional then at most 2N-1 stride objects are required to describe the intersection.  The number of
 *              stride objects is minimized as are the number of actual stride values within each stride object.  Returns
 *              negative for failures.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, October 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_async_intersect(ss_gblob_t *gblob,              /* File-wide blob information */
                        size_t d_idx,                   /* Index into !gblob->d[] for dataset in question */
                        int aggtask,                    /* The aggregator. If this task does not serve as an aggregator for
                                                         * the specified dataset then the function immediately returns zero. */
                        const hsize_t *dset_size,       /* Current dataset total size. The number of elements in this array
                                                         * and the following array arguments should match the dataset
                                                         * dimensionality. */
                        const hsize_t *req_dstart,      /* I/O request start w.r.t. dataset */
                        const hsize_t *mem_size,        /* Optional size of the memory buffer holding the data to be sent. */
                        const hsize_t *req_mstart,      /* Optional position of request in memory buffer */
                        const hsize_t *req_count,       /* Number of elements in each dimension for request */
                        ss_blob_stride_t *req_stride,   /* OUT: The optional caller supplied array of stride objects that will
                                                         * be initialized to describe the intersection upon successful return.
                                                         * The number of such structs initialized herein is the return value
                                                         * of the function. The caller should supply an array of at least 2N-1
                                                         * members where N is the dataset dimensionality. */
                        ss_blob_stride_t *snd_stride    /* OUT: The optional caller supplied array of stride objects that will
                                                         * be initialized to describe the elements of the memory buffer that
                                                         * are part of the intersection. If present then MEM_SIZE and
                                                         * REQ_MSTART must also be specified. */
                      )
{
    SS_ENTER(ss_blob_async_intersect, herr_t);
    size_t              aggseq;                         /* Which in the sequence of aggregators is AGGTASK? */
    int                 ndims;                          /* Dataset dimensionality; nelmts in various array arguments */
    hsize_t             dset_down[H5S_MAX_RANK];        /* Down-size of the array */
    hsize_t             acc;                            /* Accumulator for various things */
    hsize_t             req_dstart_1=0, req_dlast_1=0;  /* Linear indices into dataset for request */
    hsize_t             agg_start_1=0, agg_last_1=0;    /* Linear indices into dataset for aggregator's part */
    hsize_t             intersect_start[H5S_MAX_RANK];  /* Counter for current position in intersected region w.r.t. dataset */
    hsize_t             intersect_count[H5S_MAX_RANK];  /* Size of the intersection in each dimension */
    int                 dim, dim2;                      /* Counters over dimensions */
    int                 nstrides=0;                     /* Number of intersection slabs we've processed. */
    hsize_t             Ri;                             /* Elements from one request row to the next */
    hsize_t             Rr;                             /* Elements in one row of the requested region */
    hsize_t             agglen;                         /* Remaining length of the aggregation buffer */
    hsize_t             row_size[H5S_MAX_RANK];         /* Size of a "row" of the request */
    hbool_t             done;                           /* Loop control */
    int                 i;

    SS_ASSERT(gblob);
    SS_ASSERT(d_idx<gblob->d_nused);
    
    /* Is AGGTASK an aggregator? Which aggregator? */
    for (aggseq=0; aggseq<gblob->d[d_idx].agg.n; aggseq++) if (gblob->d[d_idx].agg.tasks[aggseq]==aggtask) break;
    if (aggseq>=gblob->d[d_idx].agg.n) goto done; /* aggtask is not an aggregator, therefore intersection is empty */
    if ((ndims=H5Sget_simple_extent_ndims(gblob->d[d_idx].dspace))<0) SS_ERROR(FAILED);

    /* Calculate down-sizes of the array and request. The definition is that down[i] is the number of elements represented by
     * one "row" in dimension i so that the size of the entire array is size[0]*down[0] elements. */
    SS_ASSERT(ndims<=H5S_MAX_RANK);
    for (dim=ndims-1, acc=1; dim>=0; --dim) {
        dset_down[dim] = acc;
        acc *= dset_size[dim];
    }
    
    /* Calculate linear indices in the dataset for lowest and highest elements in the request.  Be aware that the request need
     * not fully occupy the range of elements between req_dstart_1 and request_end_1 due to its multidimensional nature. */
    req_dstart_1 = ss_blob_array_linear(ndims, dset_size, req_dstart, NULL);
    req_dlast_1 = ss_blob_array_linear(ndims, dset_size, req_dstart, req_count);

    /* Calculate linear indices in the dataset for the part of the dataset owned by this aggregator. Unlike the request boundary
     * calculated above, the aggregator owns all elements between the low and high linear indices (see
     * ss_blob_async_aggregators() for how aggregators are chosen). */
    agg_start_1 = aggseq * gblob->d[d_idx].agg.elmts_per_agg;
    agg_last_1 = agg_start_1 + gblob->d[d_idx].agg.elmts_per_agg - 1;

#if 0 /*DEBUGGING*/
    {
        int j;
        hsize_t req_down[H5S_MAX_RANK];         /* Down-size of the request hyperslab */
        for (dim=ndims-1, acc=1; dim>=0; --dim) {
            req_down[dim] = acc;
            acc *= req_count[dim];
        }

        fprintf(sslib_g.warnings, "ss_blob_async_intersect() debugging: ------------------------------------------------\n");
        fprintf(sslib_g.warnings, "    dataset:\n");
        fprintf(sslib_g.warnings, "        size:      (");
        for (i=0; i<ndims; i++) fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)dset_size[i]);
        fprintf(sslib_g.warnings, "), %lu elements\n", (unsigned long)(dset_size[0]*dset_down[0]));

        fprintf(sslib_g.warnings, "    memory:%s\n", mem_size?"":" not specified");
        if (mem_size) {
            fprintf(sslib_g.warnings, "        size:      (");
            for (i=0, acc=1; i<ndims; i++) {
                fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)mem_size[i]);
                acc *= mem_size[i];
            }
            fprintf(sslib_g.warnings, "), %lu elements\n", (unsigned long)acc);
            fprintf(sslib_g.warnings, "        request at: (");
            for (i=0; i<ndims; i++) fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)req_mstart[i]);
            fprintf(sslib_g.warnings, ")\n");
        }

        fprintf(sslib_g.warnings, "    request:\n");
        fprintf(sslib_g.warnings, "        starts at: (");
        for (i=0; i<ndims; i++) fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)req_dstart[i]);
        fprintf(sslib_g.warnings, ") in dataset, linear element %lu\n", (unsigned long)req_dstart_1);
        fprintf(sslib_g.warnings, "        size:      (");
        for (i=0; i<ndims; i++) fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)req_count[i]);
        fprintf(sslib_g.warnings, "), %lu elements\n", (unsigned long)(req_count[0]*req_down[0]));
        fprintf(sslib_g.warnings, "        ends on:   (");
        ss_blob_array_multidim(ndims, dset_size, req_dlast_1, intersect_start/*out*/);
        for (i=0; i<ndims; i++) fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)intersect_start[i]);
        fprintf(sslib_g.warnings, ") in dataset, linear element %lu\n", (unsigned long)req_dlast_1);

        fprintf(sslib_g.warnings, "    aggregation buffer for task %d:\n", aggtask);
        fprintf(sslib_g.warnings, "        aggregator is %d of %d\n", aggseq, gblob->d[d_idx].agg.n);
        fprintf(sslib_g.warnings, "        starts at: (");
        ss_blob_array_multidim(ndims, dset_size, agg_start_1, intersect_start/*out*/);
        for (i=0; i<ndims; i++) fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)intersect_start[i]);
        fprintf(sslib_g.warnings, "), linear element %lu\n", (unsigned long)agg_start_1);
        fprintf(sslib_g.warnings, "        size:      %lu elements\n", (unsigned long)(agg_last_1+1-agg_start_1));
        fprintf(sslib_g.warnings, "        ends on:   (");
        ss_blob_array_multidim(ndims, dset_size, agg_last_1, intersect_start/*out*/);
        for (i=0; i<ndims; i++) fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)intersect_start[i]);
        fprintf(sslib_g.warnings, "), linear element %lu\n", (unsigned long)agg_last_1);

        fprintf(sslib_g.warnings, "    slabs: %s\n", (req_dlast_1<agg_start_1 || req_dstart_1>agg_last_1)?"no overlap":"");
    }
#endif

    /* Does the request overlap the part of the dataset for which we're responsible? */
    if (req_dlast_1<agg_start_1 || req_dstart_1>agg_last_1) goto done; /* no overlap */

    /* The `intersect_start' will serve as a multi-dimensional counter into the aggregation buffer w.r.t. the underlying
     * dataset. Before we even begin the main loop, we will make sure that the counter isn't less than the lowest linearly
     * numbered element of the request.  Then make sure it corresponds to an element of the request. */
    if (ss_blob_array_multidim(ndims, dset_size, MAX(agg_start_1,req_dstart_1), intersect_start/*out*/)<0) SS_ERROR(FAILED);
    for (dim=ndims-1; dim>=0; --dim) {
        if (intersect_start[dim]>=req_dstart[dim]+req_count[dim]) {
            for (i=dim; i<ndims; i++) intersect_start[i] = req_dstart[i];
            SS_ASSERT(dim>0);
            intersect_start[dim-1]++;
        }
    }
    
    /* Describing the intersection of the request elements and the aggregation elements is a three part process. We want to
     * use the fewest number of hyperslabs for the description but yet have to handle cases (like in the example above) where
     * the aggregation elements don't align well with the request elements and thus can't be described with a single
     * hyperslab.  The three part process is to take care of the ragged edges at the beginning and end of the intersection,
     * and then treat the middle as a single hyperslab.
     *
     * We call the process of handling the ragged edges "pruning the intersection" and we do that by looking first at the
     * fastest varying dimension, then the next fastest, etc. Whenever the intersection in that dimension isn't aligned with
     * the edge of the request we align it, possibly merging a lower-dimensional hyperslab into the final MPI and HDF5
     * selection objects. Pruning the "end" of the intersection is done in the opposite order. The middle is treated as a
     * special case of pruning the end. */
    for (dim=ndims-1; dim>=0; --dim) {
        if (intersect_start[dim]>=req_dstart[dim]+req_count[dim]) {
            /* Aggregation starts after request in this dimension. Increment aggregation starting point to align this
             * dimension at the next "row" of the slower-varying dimension. Then continue with the next slower dimension. This
             * can't be the slowest dimension because we would have avoided this whole process if so. */
            if (0==dim) break; /* we're already advanced past the whole request */
            intersect_start[dim] = req_dstart[dim];
            intersect_start[dim-1]++;
            continue;
        } else if (intersect_start[dim]<=req_dstart[dim]) {
            /* Aggregation starts before (or at) request in this dimension. Just increment the aggregation starting point to align
             * it with the start of the request and fall through to next dimension if there is one. */
            intersect_start[dim] = req_dstart[dim];
            if (dim>0) continue;
        }

        /* Aggregation starts within (or at the edge of) the request in this dimension. All faster-varying dimensions have
         * been aligned so we can merge an M-dimensional hyperslab into the final MPI and HDF5 selection objects, where M is
         * NDIMS-DIM. We have to handle the case where the end of the intersection is ragged, which significantly complicates
         * this section of code. */
        for (dim2=dim, done=FALSE; dim2<ndims && !done; dim2++) {
            if (ss_blob_array_linear(ndims, dset_size, intersect_start, NULL)>MIN(req_dlast_1,agg_last_1)) break;

            /* Calculate size of intersection slab, intersection_count[] */
            for (i=0; i<dim2; i++) intersect_count[i] = 1;

            /*      _______________________
             *     |.......................|  If we're working on the "rows" (i.e., dim2 represents a row) then
             *     |......oooooooo.........|  we need to figure out how many rows we can represent with a single
             *     |......oooooooo.........|  hyperslab.  We can do this by looking at the linear distance in the
             *     |......########+++++++++|  aggregation buffer from the beginning of one row of the intersection
             *     |++++++########+++++++++|  to the beginning of the next row of the intersection (Ri) and the
             *     |++++++########+++++++++|  length of a single row of the request (Rr) as it appears in that buffer.
             *     |++++++###ooooo.........|
             *     |......oooooooo.........|  The number of complete rows in the intersection is the length of
             *     |......oooooooo.........|  the aggregation buffer (the `+' and `#' elements) plus the request
             *     |.......................|  inter-row spacing (Ri-Rr) the quantity divided by Ri (integer division)
             *     +-----------------------+  bounded by the actual remaining requested size.
             */
            Ri = dset_down[dim2];
            for (i=0; i<ndims; i++) row_size[i] = (i<=dim2)?1:req_count[i];
            Rr = ss_blob_array_linear(ndims, dset_size, req_dstart, row_size)+1 -
                 ss_blob_array_linear(ndims, dset_size, req_dstart, NULL);
            agglen = agg_last_1+1 - ss_blob_array_linear(ndims, dset_size, intersect_start, NULL);
            intersect_count[dim2] = MIN((agglen+(Ri-Rr)) / Ri,
                                        req_dstart[dim2]+req_count[dim2]-intersect_start[dim2]);

            /* Faster dimensions are filled up */
            for (i=dim2+1; i<ndims; i++) {
                SS_ASSERT(intersect_start[i]==req_dstart[i]);
                intersect_count[i] = req_count[i];
            }

            /* Did we use up all of this row? If so, don't bother going through this dim2 loop again */
            if (intersect_count[dim2]==req_dstart[dim2]+req_count[dim2]-intersect_start[dim2]) done = TRUE;
            if (0==intersect_count[dim2]) continue; /* no point in figuring out sizes in other dimensions */

#if 0 /*DEBUGGING*/
            fprintf(sslib_g.warnings, "        %dd at (", dim2);
            for (i=0; i<ndims; i++) fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)intersect_start[i]);
            fprintf(sslib_g.warnings, ") size (");
            for (i=0, acc=1; i<ndims; i++) {
                fprintf(sslib_g.warnings, "%s%lu", i?",":"", (unsigned long)intersect_count[i]);
                acc *= intersect_count[i];
            }
            fprintf(sslib_g.warnings, "), %lu elements.", (unsigned long)acc);
            if (ss_blob_array_linear(ndims, dset_size, intersect_start, intersect_count)>MIN(req_dlast_1,agg_last_1))
                fprintf(sslib_g.warnings, " <--------------------------- ERROR!");
            fprintf(sslib_g.warnings, "\n");
#endif

            /* Describe the intersection in terms of a stride object in order to construct MPI types later. */
            if (req_stride) {
                if (ss_blob_stride(ndims, dset_size, intersect_start, intersect_count, req_stride+nstrides)<0) SS_ERROR(FAILED);
                SS_ASSERT(req_stride[nstrides].offset>=agg_start_1);
                req_stride[nstrides].offset -= agg_start_1;
            }
            if (snd_stride) {
                hsize_t mstart[H5S_MAX_RANK];
                for (i=0; i<ndims; i++) mstart[i] = intersect_start[i] + req_mstart[i] - req_dstart[i];
                if (ss_blob_stride(ndims, mem_size, mstart, intersect_count, snd_stride+nstrides)<0) SS_ERROR(FAILED);
            }
            nstrides++;

            /* Merge the intersection description into the HDF5 data space selections for the aggregation buffer and the
             * eventual dataset for that buffer to be used when we H5Dwrite() the buffer to the file. We should do this only
             * if the current task is an aggregator. */
            if (req_stride) {
                if (gblob->d[d_idx].agg.iof<=0) {
                    if ((gblob->d[d_idx].agg.iof=H5Scopy(gblob->d[d_idx].dspace))<0 ||
                        H5Sselect_none(gblob->d[d_idx].agg.iof)<0 ||
                        (gblob->d[d_idx].agg.iom=H5Scopy(gblob->d[d_idx].agg.iof))<0)
                        SS_ERROR(HDF5);
                }
                if (H5Sselect_slab(gblob->d[d_idx].agg.iof, H5S_SELECT_OR, (hsize_t)0, intersect_start, intersect_count)<0 ||
                    H5Sselect_slab(gblob->d[d_idx].agg.iom, H5S_SELECT_OR, agg_start_1, intersect_start, intersect_count)<0)
                    SS_ERROR(HDF5);
            }
            
            /* Increment multi-dimensional intersection counter by intersection_count to account for the part of the
             * intersection which we just processed. */
            intersect_start[dim2] += intersect_count[dim2];
        }
        
        /* Outer loop intersection increment */
        for (i=dim; i<ndims; i++) intersect_start[i] = req_dstart[i];
        if (dim>0) intersect_start[dim-1]++;
    }

#if 0 /*DEBUGGING*/
    fprintf(sslib_g.warnings, "    request strides:%s\n", req_stride?"":" not computed");
    if (req_stride) {
        fprintf(sslib_g.warnings, "         #  D    Offset Duplicity Elmts Count/stride\n");
        fprintf(sslib_g.warnings, "        -- -- --------- --------- ----- -----------------------------------------\n");
        for (i=0, acc=0; i<nstrides; i++) {
            size_t stride_nelmts = req_stride[i].duplicity;
            for (j=0; j<req_stride[i].ndims; j++) stride_nelmts *= req_stride[i].count[j];
            fprintf(sslib_g.warnings, "        %2d %2d %9lu %9lu %5lu ",
                    i, req_stride[i].ndims, (unsigned long)(req_stride[i].offset), (unsigned long)(req_stride[i].duplicity),
                    (unsigned long)stride_nelmts);
            for (j=0; j<req_stride[i].ndims; j++) {
                fprintf(sslib_g.warnings, "%s%lu/%lu", j?", ":"",
                        (unsigned long)(req_stride[i].count[j]), (unsigned long)(req_stride[i].stride[j]));
            }
            fprintf(sslib_g.warnings, "\n");
            acc += stride_nelmts;
        }
        fprintf(sslib_g.warnings, "        total elements in request strides: %lu\n", (unsigned long)acc);
    }
    fprintf(sslib_g.warnings, "    send strides:%s\n", snd_stride?"":" not computed");
    if (snd_stride) {
        fprintf(sslib_g.warnings, "         #  D    Offset Duplicity Elmts Count/stride\n");
        fprintf(sslib_g.warnings, "        -- -- --------- --------- ----- -----------------------------------------\n");
        for (i=0, acc=0; i<nstrides; i++) {
            size_t stride_nelmts = snd_stride[i].duplicity;
            for (j=0; j<snd_stride[i].ndims; j++) stride_nelmts *= snd_stride[i].count[j];
            fprintf(sslib_g.warnings, "        %2d %2d %9lu %9lu %5lu ",
                    i, snd_stride[i].ndims, (unsigned long)(snd_stride[i].offset), (unsigned long)(snd_stride[i].duplicity),
                    (unsigned long)stride_nelmts);
            for (j=0; j<snd_stride[i].ndims; j++) {
                fprintf(sslib_g.warnings, "%s%lu/%lu", j?", ":"",
                        (unsigned long)(snd_stride[i].count[j]), (unsigned long)(snd_stride [i].stride[j]));
            }
            fprintf(sslib_g.warnings, "\n");
            acc += stride_nelmts;
        }
        fprintf(sslib_g.warnings, "        total elements in send strides: %lu\n", (unsigned long)acc);
    }
    
    
#endif

done:
SS_CLEANUP:
    SS_LEAVE(nstrides);
}

#ifdef HAVE_PARALLEL
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Post async receives for 2-phase I/O
 *
 * Description: This function posts MPI asynchronous receives for data from task SRCTASK. If an aggregation buffer has not
 *              been created for the specified dataset then that will be done so now, but if it causes more than some
 *              user-specified maximum amount of memory to be used for aggregation buffers then this function blocks until
 *              data for some other dataset(s) has been moved to the HDF5 file and the corresponding aggregation buffer(s)
 *              have been released.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, October 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static herr_t
ss_blob_async_receives(ss_gblob_t *gblob,               /* File-wide blob information */
                       size_t d_idx,                    /* Index into !gblob->d[] for dataset in question */
                       int srctask,                     /* MPI task from which we should receive data */
                       MPI_Comm filecomm,               /* The file communicator of which SRCTASK is a part */
                       int nstrides,                    /* Number of elements in stride array S */
                       const ss_blob_stride_t *s        /* Description of affected portion of aggregation buffer */
                       )
{
    SS_ENTER(ss_blob_async_receives, herr_t);
    MPI_Datatype        elmttype=MPI_DATATYPE_NULL;     /* MPI datatype for single element of the dataset */
    MPI_Datatype        recvtype=MPI_DATATYPE_NULL;     /* All mpistride[] types combined into a single MPI type */
    char                *aggbuf=NULL;                   /* Aggregation buffer */
    MPI_Request         *request=NULL;                  /* Request object for the MPI_Irecv() */

    SS_ASSERT(nstrides<=H5S_MAX_RANK*2);
    if (0==nstrides) goto done;

    /* Convert the set of ss_blob_stride_t objects into a single MPI datatype */
    if (H5Tmpi(gblob->d[d_idx].dtype, &elmttype)<0) SS_ERROR(FAILED);
    if (ss_mpi_type_create_stride(nstrides, s, elmttype, &recvtype)<0) SS_ERROR(FAILED);

    /* Allocate the aggregation buffer, possibly flushing old buffers to disk */
    if (NULL==(aggbuf=ss_blob_async_buffer(gblob, d_idx))) SS_ERROR(FAILED);

    /* Post receive */
    SS_EXTEND(gblob->d[d_idx].agg.recv, MAX(16,gblob->d[d_idx].agg.recv_nused+1), gblob->d[d_idx].agg.recv_nalloc);
    request = gblob->d[d_idx].agg.recv + gblob->d[d_idx].agg.recv_nused;
    if (MPI_Irecv(aggbuf, 1, recvtype, srctask, SS_BLOB_2PIO_TAG, filecomm, request)) SS_ERROR(MPI);
    gblob->d[d_idx].agg.recv_nused++;
    
    /* Free resources */
    if (MPI_Type_free(&recvtype)) SS_ERROR(MPI);
    recvtype = MPI_DATATYPE_NULL;
    if (MPI_Type_free(&elmttype)) SS_ERROR(MPI);
    elmttype = MPI_DATATYPE_NULL;

done:
SS_CLEANUP:
    if (recvtype!=MPI_DATATYPE_NULL) MPI_Type_free(&recvtype);
    if (elmttype!=MPI_DATATYPE_NULL) MPI_Type_free(&elmttype);
    SS_LEAVE(0);
}
#endif /*HAVE_PARALLEL*/

#ifdef HAVE_PARALLEL
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Post async sends for 2-phase I/O
 *
 * Description: This function posts MPI asynchronous sends for data to aggregator task AGGTASK.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, October 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static herr_t
ss_blob_async_sends(ss_gblob_t *gblob,                  /* File-wide blob information */
                    size_t d_idx,                       /* Index into !gblob->d[] for dataset in question */
                    int aggtask,                        /* MPI task to which the data is being sent */
                    MPI_Comm filecomm,                  /* The file communicator of which AGGTASK is a part */
                    int nstrides,                       /* Number of elements in stride array S */
                    const ss_blob_stride_t *s,          /* Description of affected portion of source buffer */
                    char *buffer,                       /* Source buffer */
                    unsigned flags                      /* Flags, most importantly SS_BLOB_FREE */
                    )
{
    SS_ENTER(ss_blob_async_sends, herr_t);
    MPI_Datatype        elmttype=MPI_DATATYPE_NULL;     /* Datatype of a single BUFFER element */
    MPI_Datatype        sendtype=MPI_DATATYPE_NULL;     /* Type of stuff to send */
    MPI_Request         *request=NULL;                  /* Request object for the MPI_Isend() */

    /* Convert the set of ss_blob_stride_t objects into a single MPI datatype */
    if (H5Tmpi(gblob->d[d_idx].dtype, &elmttype)<0) SS_ERROR(FAILED);
    if (ss_mpi_type_create_stride(nstrides, s, elmttype, &sendtype)) SS_ERROR(FAILED);

    /* Post sends */
    SS_EXTEND(gblob->d[d_idx].agg.send, MAX(16,gblob->d[d_idx].agg.send_nused+1), gblob->d[d_idx].agg.send_nalloc);
    request = gblob->d[d_idx].agg.send + gblob->d[d_idx].agg.send_nused;
    if (MPI_Isend(buffer, 1, sendtype, aggtask, SS_BLOB_2PIO_TAG, filecomm, request)) SS_ERROR(MPI);
    gblob->d[d_idx].agg.send_nused++;

    /* If the send buffer is to be freed later then save it. If there is a limit on the number of such buffers then we should
     * block until some send requests complete in order to satisfy the constraint. */
    if (flags & SS_BLOB_FREE) {
        if (gblob->agg.sendqueue>0 && ss_blob_async_prune(SS_NOSIZE, gblob->agg.sendqueue-1)<0)
            SS_ERROR(FAILED);
        SS_EXTEND(gblob->d[d_idx].agg.sendbufs, MAX(16,gblob->d[d_idx].agg.sendbufs_nused+1), gblob->d[d_idx].agg.sendbufs_nalloc);
        gblob->d[d_idx].agg.sendbufs[gblob->d[d_idx].agg.sendbufs_nused++] = buffer;
    }
    
    /* Free resources */
    if (MPI_Type_free(&elmttype)) SS_ERROR(MPI);
    elmttype = MPI_DATATYPE_NULL;
    if (MPI_Type_free(&sendtype)) SS_ERROR(MPI);
    sendtype = MPI_DATATYPE_NULL;

SS_CLEANUP:
    if (MPI_DATATYPE_NULL!=elmttype) MPI_Type_free(&elmttype);
    if (MPI_DATATYPE_NULL!=sendtype) MPI_Type_free(&sendtype);
    SS_LEAVE(0);
}
#endif /*HAVE_PARALLEL*/

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Compares gblob async records
 *
 * Description: This is the callback for qsort() to sort the gblob->a table by dataset object header address but keep relative
 *              ordering for entries that refer to the same dataset.
 *
 * Return:      Same as strcmp()
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, October  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_blob_async_sort_cb(const void *_a, const void *_b)
{
    /* SS_ENTER() -- skipped for efficiency. This function never fails anyway. */
    ss_gblob_t *gblob = ss_blob_async_sort_g;
    const struct ss_blob_async_t *a = _a;
    const struct ss_blob_async_t *b = _b;

    /* use assert() and not SS_ASSERT() since there is no SS_ENTER() */
    assert(gblob);
    assert(a->d_idx < gblob->d_nused);
    assert(b->d_idx < gblob->d_nused);

    if (gblob->d[a->d_idx].stat.objno < gblob->d[b->d_idx].stat.objno) return -1;
    if (gblob->d[a->d_idx].stat.objno > gblob->d[b->d_idx].stat.objno) return 1;
    if (_a < _b) return -1;
    if (_a > _b) return 1;
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Note asynchronous I/O request
 *
 * Description: HDF5 dataset open, create, and extend operations are file-collective but SSlib desires to provide a
 *              scope-collective (or independent) interface for blob I/O. We do that by making a note of all I/O requests and
 *              initiating the request at some future file-collective time.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_async_append(ss_gblob_t *gblob,         /* File wide blob information */
                     size_t d_idx,              /* Index into the gblob->d[] array */
                     hid_t mtype,               /* Datatype of BUFFER */
                     hid_t iom,                 /* Elements of BUFFER selected for I/O */
                     hid_t iof,                 /* Description of where data will land in the dataset */
                     void *buffer,              /* The buffer supplying the data */
                     unsigned flags             /* Various SS_BLOB flags */
                     )
{
    SS_ENTER(ss_blob_async_append, herr_t);
    size_t              a_idx;

    SS_ASSERT_TYPE(gblob, ss_gblob_t);
    SS_ASSERT(mtype>0);
    SS_ASSERT(iom>0);
    SS_ASSERT(iof>0);

    /* Append a new item to the end. We do not duplicate the datatype or space handles. */
    SS_EXTEND(gblob->a, MAX(64,gblob->a_nused+1), gblob->a_nalloc);
    a_idx = gblob->a_nused++;
    gblob->a[a_idx].d_idx = d_idx;
    gblob->a[a_idx].mtype = mtype;
    gblob->a[a_idx].iom = iom;
    gblob->a[a_idx].iof = iof;
    gblob->a[a_idx].buffer = buffer;
    gblob->a[a_idx].flags = flags;

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Assign aggregator tasks to a dataset
 *
 * Description: This function assigns (or reassigns) aggregators to a dataset.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the dataset's file communicator.  This is a no-op for a serial version of SSlib or if the
 *              two-phase I/O is turned off at runtime.
 *
 * Programmer:  Robb Matzke
 *              Saturday, October  4, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
static herr_t
ss_blob_async_aggregators(MPI_Comm UNUSED_SERIAL filecomm,      /* Dataset's file communicator */
                          ss_gblob_t UNUSED_SERIAL *gblob,      /* File-wide blob information */
                          size_t UNUSED_SERIAL d_idx,           /* Index into gblob->d table */
                          int UNUSED_SERIAL ndims,              /* Dataset dimensionality. We could obtain it from the gblob,
                                                                 * but the caller already did that anyway so we can avoid the
                                                                 * call here. */
                          hsize_t UNUSED_SERIAL *dset_size      /* Total dataset size. We could obtain it from the gblob, but
                                                                 * the caller already did that anyway so we can avoid the call
                                                                 * here. */
                           )
{
    SS_ENTER(ss_blob_async_aggregators, herr_t);
#ifdef HAVE_PARALLEL
    unsigned    ntasks;                                 /* Number of tasks in the file communicator */
    int         self;                                   /* Caller's rank in the file communicator */
    unsigned    i, base, ng1, g1, g2, g3, target;       /* Temps for choosing task numbers */
    int         *map_a2t=NULL;                          /* Map from aggregator number to file communicator task number */
    int         *map_t2a=NULL;                          /* Map from file communicator task number to aggregator number */
    hsize_t     bytes_per_agg;                          /* Actual number of bytes per aggregator */
    hsize_t     elmts_per_agg;                          /* Actual number of elements per aggregator */
    size_t      naggs;                                  /* Desired number of aggregators */
    hsize_t     nelmts;                                 /* Total elements currently in the dataset */
    size_t      tsize;                                  /* Number of bytes in each dataset element */
    hsize_t     dsize;                                  /* Number of bytes in the whole dataset (tsize*nelmts) */
    size_t      min_bytes_per_agg;                      /* Minimum aggregation size, adjusted for alignment and datatype size */
    hbool_t     debug=sslib_g.tpio_alloc;               /* Turned on for certain errors in order to get more info */

    if (0==gblob->agg.maxaggtasks) goto done;           /* Two-phase I/O is disabled. */
    if (MPI_Comm_size(filecomm, (int*)&ntasks)) SS_ERROR(MPI);
    if (MPI_Comm_rank(filecomm, &self)) SS_ERROR(MPI);
    for (i=0, nelmts=1; i<(unsigned)ndims; i++) nelmts *= dset_size[i];
    if (0==(tsize = H5Tget_size(gblob->d[d_idx].dtype))) SS_ERROR(HDF5);
    dsize = tsize * nelmts;

    /* Calculate the number of aggregation tasks (naggs) and the number of dataset elements allocated to each aggregator
     * (elmts_per_agg).  We do this by minimizing the aggregation buffer size (i.e., maximizing the number of aggregators)
     * subject to all of the following constraings:
     *    1. The buffer size cannot be smaller than the client-defined minimum for this file (gblob->agg.minbufsize).
     *    2. The number of aggregators cannot be larger than the user-defined max for this file (gblob->agg.maxaggtasks).
     *    3. The buffer will be a multiple of some alignment defined for this file (gblob->agg.alignment), provided it doesn't
     *       conflict with the datatype size. If it does conflict, then we just make the buffer a little bigger.
     *    4. The buffer will be a multiple of the datatype size. */
    min_bytes_per_agg = ALIGN(gblob->agg.minbufsize, tsize); /*adjust min agg size for datatype size */
    naggs = MIN3(ntasks, gblob->agg.maxaggtasks, (dsize+min_bytes_per_agg-1)/min_bytes_per_agg);
    bytes_per_agg = ALIGN(ALIGN(dsize/naggs, gblob->agg.alignment), tsize);
    elmts_per_agg = bytes_per_agg / tsize;
    naggs = MIN3(ntasks, gblob->agg.maxaggtasks, (nelmts+elmts_per_agg-1)/elmts_per_agg);

    /* If the dataset isn't extendible then there's no point in storing all possible aggregator ranks. Furthermore, if we're
     * only going to use one aggregator then it's pointless to worry about padding the aggregation buffer to a multiple of the
     * alignment size. */
    if (1==naggs && !gblob->d[d_idx].is_extendible) {
        elmts_per_agg = nelmts;
        bytes_per_agg = dsize;
    }

    /* If the newly computed elmts_per_agg is the same as what we already have assigned as aggregators then there's no point
     * in continuing because we'll just be computing the same set again. */
    if (elmts_per_agg == gblob->d[d_idx].agg.elmts_per_agg) goto done;
    SS_ASSERT(0==gblob->d[d_idx].agg.elmts_per_agg || gblob->d[d_idx].is_extendible);

    /* ISSUE: Sometimes we choose bad default values for two-phase I/O settings. For example, should we bother using 2-phase
     *        I/O if the total dataset size is 100MB and we only want aggregator buffers to use 10MB?  The right way to handle
     *        this is to disable two-phase I/O on a dataset-by-dataset case, but what we currently do is just go ahead and
     *        allocated a bigger aggregation buffer. */
    if (0==self && bytes_per_agg>gblob->agg.aggbuflimit) {
        fprintf(sslib_g.warnings,
                "SSlib-%d: aggregation buffer will be oversubscribed (warning)\n", ss_mpi_comm_rank(MPI_COMM_WORLD));
        fprintf(sslib_g.warnings,
                "         Limit is %s bytes; details follow\n", ss_bytes((hsize_t)(gblob->agg.aggbuflimit), NULL));
        fprintf(sslib_g.warnings,
                "         Recommend turning off two-phase I/O as a temporary fix.\n");
        fprintf(sslib_g.warnings,
                "         This should be fixed in a future version. See ssblob.c line %lu.\n", (unsigned long)__LINE__);
        debug = TRUE;
    }
    
    /* Now that we know how many aggregators, we can choose which tasks of the file communicator will serve as the
     * aggregators. The goal is to choose tasks that are mapped to different SMP nodes because that serves the best chance of
     * keeping I/O memory-bound (versus I/O bound). However, since MPI doesn't have any method for getting this information we
     * have to guess. We assume that the file communicator's tasks are mapped to SMP nodes with a bucket filling algorithm:
     * each node gets sequentially number tasks until the node is full, then tasks start being assigned to the next node. We
     * also assume that the client is mapping either 4 or 16 MPI tasks to each SMP node. If any of these assumptions are
     * invalid it just means that we stand less of a chance of getting aggregators spread across nodes.
     *
     * Since we want to try to avoid two dataset sharing the same aggregators, we use the dataset object header address to
     * seed the algorithm. The bit swizzling is so that we're immune from object header alignment issues in HDF5. */
    if (gblob->agg.aggbase<0) {
        base = (unsigned)(gblob->d[d_idx].stat.objno); /*don't worry about overflow -- we're hashing anyway*/
        base += (base << 12);
        base ^= (base >> 22);
        base += (base << 4);
        base ^= (base >> 9);
        base += (base << 10);
        base ^= (base >> 2);
        base += (base << 7);
        base ^= (base >> 12);
        base %= ntasks;
    } else {
        base = gblob->agg.aggbase % ntasks;
        if (0==self) fprintf(sslib_g.warnings, "SSlib-0: base aggregator forced to library task %d\n", base);
    }
    
    /* Once we've chosen the initial aggregator the others are chosen by offset (modulo the number of tasks in the file
     * communicator). The algorithm for computing the offsets is multi-phased:
     *    (1) assign segments round-robin to buckets of 16 tasks.
     *    (2) within each bucket of 16, assign segments round-robin to buckets of 4 tasks.
     *    (3) within each bucket of 4, assign segments round-robin to tasks.
     * The assignment is subject to the constraint that no task can be assigned multiple segments (if this were to ever happen
     * then there may be cases where the asynchronous sends/receives during data-shipping get mixed up. So when there's a
     * collision (which can only happen if the number of total tasks is not a multiple of 16) we just use the next available
     * task instead, which might result in more collisions down the road. */
    if (NULL==(map_a2t=malloc(ntasks*sizeof(*map_a2t)))) SS_ERROR(RESOURCE);
    if (NULL==(map_t2a=malloc(ntasks*sizeof(*map_t2a)))) SS_ERROR(RESOURCE);
    for (i=0; i<(unsigned)ntasks; i++) map_a2t[i] = map_t2a[i] = -1; /* mark as unused */
    if (gblob->agg.tpn<=0) {
        ng1 = (ntasks + 15) / 16; /* number of top-level groups */
        for (i=0; i<(unsigned)ntasks; i++) {
            g1 = (16 * i) % ntasks; /*top level group number: 0, 16, 32, 48, ... NTASKS-1*/
            g2 = (i / ng1) % 4;     /*g2 w.r.t. g1: 0, 1, 2, or 3 */
            g3 = (i / (4*ng1)) % 4; /*g3 w.r.t. g2: 0, 1, 2, or 3 */
            target = (g1 + 4*g2 + g3) % ntasks;
            while (map_t2a[target]>=0) target = (target+1) % ntasks;
            map_t2a[target] = i;
            map_a2t[i] = target;
        }
    } else {
        ng1 = (ntasks + gblob->agg.tpn - 1) / gblob->agg.tpn; /* number of top-level groups */
        for (i=0; i<(unsigned)ntasks; i++) {
            g1 = (gblob->agg.tpn * i) % ntasks; /*top level group number by task rank */
            g3 = i / ng1;
            target = (g1 + g3) % ntasks;
            while (map_t2a[target]>=0) target = (target+1) % ntasks;
            map_t2a[target] = i;
            map_a2t[i] = target;
        }
    }
    
    /* If tasks are already assigned then we must not have any pending I/O.  This is the only way we can be sure that writes
     * will still occur in the order specified by the SSlib client. */
    if (gblob->d[d_idx].agg.tasks && (gblob->d[d_idx].agg.recv_nused>0 || gblob->d[d_idx].agg.aiocb))
        SS_ERROR_FMT(CORRUPT, ("caller should flush blob before changing its size"));

    /* Store the list of aggregator tasks. Try to reuse the old array if possible. We'll also pad out the list to be the
     * maximum number of aggregators allowed so that smallish datasets (w.r.t. min elements per aggregator times max number of
     * aggregators) have some room to grow before we need to recompute all this. */
    gblob->d[d_idx].agg.n = gblob->d[d_idx].is_extendible ? gblob->agg.maxaggtasks : naggs;
    gblob->d[d_idx].agg.elmts_per_agg = elmts_per_agg;
    gblob->d[d_idx].agg.tasks = realloc(gblob->d[d_idx].agg.tasks, gblob->d[d_idx].agg.n*sizeof(gblob->d[d_idx].agg.tasks[0]));
    if (NULL==gblob->d[d_idx].agg.tasks) {
        gblob->d[d_idx].agg.n = 0;
        SS_ERROR(RESOURCE);
    }
    gblob->d[d_idx].agg.i_am_aggregator = -1;
    for (i=0; i<gblob->d[d_idx].agg.n; i++) {
        gblob->d[d_idx].agg.tasks[i] = (base + map_a2t[i]) % ntasks;
        if (gblob->d[d_idx].agg.tasks[i] == self) gblob->d[d_idx].agg.i_am_aggregator = i;
    }
    
    /* Emit some debugging info if client desires and we didn't short circuit */
    if (0==self && debug) {
        hsize_t growable = MIN(ntasks, gblob->agg.maxaggtasks)*elmts_per_agg - nelmts;
        fprintf(sslib_g.warnings, "SSlib-%d: Allocating aggregators for dataset %d at HDF5 address %lu:\n",
                ss_mpi_comm_rank(MPI_COMM_WORLD), d_idx, (unsigned long)(gblob->d[d_idx].stat.objno));
        fprintf(sslib_g.warnings, "         Dataset size:              %s elements, %s bytes\n",
                ss_bytes(nelmts, NULL), ss_bytes(dsize, NULL));
        fprintf(sslib_g.warnings, "         Min aggregation size:      %s elements, %s bytes\n",
                ss_bytes((hsize_t)min_bytes_per_agg/tsize, NULL), ss_bytes((hsize_t)min_bytes_per_agg, NULL));
        fprintf(sslib_g.warnings, "         Aggregators needed:        %lu (may grow to %lu)\n",
                (unsigned long)naggs, (unsigned long)(gblob->agg.maxaggtasks));
        fprintf(sslib_g.warnings, "         Actual aggregation size:   %s elements, %s bytes%s\n",
                ss_bytes(elmts_per_agg, NULL), ss_bytes(bytes_per_agg, NULL),
                bytes_per_agg>gblob->agg.aggbuflimit?" (OVERSUBSCRIBED)":"");
        fprintf(sslib_g.warnings, "         Dset may grow by:          %s elements, %s bytes%s\n",
                ss_bytes(growable, NULL), ss_bytes(tsize*growable, NULL),
                gblob->d[d_idx].is_extendible?"":" (dataset is fixed size)");
        fprintf(sslib_g.warnings, "         Aggregator ranks (WORLD):  ");
        for (i=0; i<gblob->d[d_idx].agg.n; i++) {
            int tn = ss_mpi_maptask(gblob->d[d_idx].agg.tasks[i], filecomm, MPI_COMM_WORLD);
            fprintf(sslib_g.warnings, "%s%d", i?", ":"", tn);
        }
        fprintf(sslib_g.warnings, "%s\n", gblob->d[d_idx].agg.n>1?" (in that order)":"");
    }


done:
    SS_FREE(map_a2t);
    SS_FREE(map_t2a);

SS_CLEANUP:
    SS_FREE(map_a2t);
    SS_FREE(map_t2a);
#endif /*HAVE_PARALLEL*/
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Create new aggregation buffer
 *
 * Description: Every aggregator is responsible for some contiguous part of a dataset (which is also contiguous in the HDF5
 *              file if the dataset is a contiguous, i.e. non extendible, dataset).  This function allocates a buffer to hold
 *              the aggregated data which is received from the other tasks with asynchronous point-to-point communication.
 *
 *              If the addition of this buffer would cause more than gblob->agg.aggbuflimit bytes to be allocated for
 *              aggregation buffers then some of the older buffers will be flushed to disk and released before this function
 *              returns.
 *
 * Issue:       For simplicity the aggregator task will allocate a single buffer for each dataset for which it is responsible
 *              as opposed to multiple smaller buffers for each dataset.  There are at least two drawbacks to this method:
 *
 *              * The buffer might be very large compared to the amount of I/O occuring. In fact, it might be prohibitively
 *                large.  If it is larger than the total amount of memory allowed for aggregation buffers then a warning can
 *                be issued to the standard error stream to remind us that this needs to eventually be fixed.
 *
 *              * Overlapping writes might not be correctly ordered even if a single task issued both of the overlapping
 *                writes.  This is because MPI doesn't make any guarantees about the order of asynchronous receives
 *                when their destinations overlap.
 *
 * Return:      Returns a pointer to an aggregation buffer on success; returns the null pointer on failure.  The aggregation
 *              buffer is also stored as the gblob->d[].agg.buffer pointer.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, October 14, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_blob_async_buffer(ss_gblob_t *gblob,                 /* File-level blob information */
                     size_t d_idx                       /* Dataset index into GBLOB */
                     )
{
    SS_ENTER(ss_blob_async_buffer, voidP);
    void                *buffer=NULL;
    size_t              need, tsize, prune_to;
    size_t              pagesize;

    SS_ASSERT(d_idx < gblob->d_nused);
    SS_ASSERT(gblob->d[d_idx].agg.i_am_aggregator>=0);

    if ((buffer=gblob->d[d_idx].agg.aggbuf)) goto done;

    /* How big is this aggregation buffer? */
    if (0==(tsize = H5Tget_size(gblob->d[d_idx].dtype))) SS_ERROR(HDF5);
    need = gblob->d[d_idx].agg.elmts_per_agg * tsize;
    
    /* Prune some aggregation buffers to make room for the new one */
    if (need > gblob->agg.aggbuflimit) {
        fprintf(sslib_g.warnings, "SSlib-%d: aggregation buffer limit exceeded: allocating %s bytes; limit is %s\n",
                ss_mpi_comm_rank(SS_COMM_WORLD), ss_bytes((hsize_t)need, NULL), ss_bytes((hsize_t)(gblob->agg.aggbuflimit), NULL));
        prune_to = 0;
    } else {
        prune_to = gblob->agg.aggbuflimit - need;
    }
    if (ss_blob_async_prune(prune_to, SS_NOSIZE)<0) SS_ERROR(FAILED);

    /* ISSUE: According to AIX documentation, buffers passed to aio_write() should be page aligned (both ends) so that the
     *        process doesn't access the same page during the write (which would cause the process to incur a page fault and
     *        block until the write completes). We use this on other systems as well because it might prevent the OS from
     *        needing to do additional copying in order to align the data. */
#ifdef WIN32
	{  
		SYSTEM_INFO SysInfo;      
		GetSystemInfo( &SysInfo );     
		pagesize = SysInfo.dwPageSize;   
	} 
#elif defined(JANUS)
    pagesize = 4096; /* This is what we were told to use in netcdf */
#else
    pagesize = (size_t)sysconf(_SC_PAGESIZE);
#endif
    gblob->d[d_idx].agg.aggbuf_freeable = malloc(need+pagesize-1);
    gblob->d[d_idx].agg.aggbuf = buffer = (void*)ALIGN((size_t)(gblob->d[d_idx].agg.aggbuf_freeable), pagesize);
    if (!buffer) SS_ERROR(RESOURCE);
    
#if 0 /* we don't really need to zero out the entire buffer: only part of it will ever be initialized and only that same part
       * will ever be written to the file.  But zeroing it sure makes debugging easier! --rpm 2003-10-28 */
    memset(buffer, 0, need);
#endif

    /* Keep track of the total number of bytes allocated for aggregation buffers. */
    ss_blob_aggbuf_total_g += need;

done:
SS_CLEANUP:
    SS_LEAVE(buffer);
}


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Prune pending async send/recv
 *
 * Description: This function prunes the aggregation buffer usage down to some specified amount by flushing various
 *              asynchronous data.  It is similar to ss_blob_async_flush_all() but with the following two significant
 *              differences:
 *
 *              * It always operates on all datasets of all files open by the calling task but might short circuit
 *                if the specified constraints are met before all files are processed.
 *
 *              * The caller does not pass bit flags to determine how things are flushed. Instead, this function looks at
 *                what constraints are not yet met and decides how to flush things itself.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. Serial behavior is a no-op.
 *
 * Programmer:  Robb Matzke
 *              Monday, October 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_async_prune(size_t aggbufsize,  /* Max allowed bytes for all aggregation buffers.  The global variable
                                         * ss_blob_aggbuf_total_g contains the number of bytes currently in use by aggregation
                                         * buffers. The SS_NOSIZE constant may be passed here to indicate that no aggregation
                                         * buffer constraint is desired. */
                    size_t sendbufs     /* Max allowed freeable send buffers (not bytes). The global variable
                                         * ss_blob_sendbuf_total_g holds the number of freeable send buffers currently
                                         * allocated. The SS_NOSIZE constant may be passed here to indicate that no send
                                         * buffer constraint is desired. */
                    )
{
    SS_ENTER(ss_blob_async_prune, herr_t);
    int         pass;
    ss_gfile_t  *gfile;
    size_t      gfile_idx, d_idx;
    unsigned    flags;

    for (pass=0; pass<3; pass++) {
        for (gfile_idx=0; (gfile=SS_GFILE_IDX(gfile_idx)); gfile_idx++) {
            if (!gfile->cur_open) continue;
            for (d_idx=0; d_idx<gfile->gblob->d_nused; d_idx++) {
                /* Are we done? */
                if (ss_blob_aggbuf_total_g<=aggbufsize && ss_blob_sendbuf_total_g<=sendbufs) break;

                /* Choose flushing flags */
                flags = SS_BLOB_REAP_SEND | SS_BLOB_REAP_RECV | SS_BLOB_REAP_WRITE;
#if 1 /*RPM DEBUGGING 2004-12-13. Failing to set these causes temporary hangs on Linux with MPICH-1.2.5*/
                flags |= SS_BLOB_FLUSH_SEND | SS_BLOB_FLUSH_RECV | SS_BLOB_FLUSH_WRITE;
#else
                if (ss_blob_aggbuf_total_g>aggbufsize && pass>0) {
                    flags |= SS_BLOB_FLUSH_RECV | SS_BLOB_FLUSH_WRITE;
                } else if (ss_blob_sendbuf_total_g>sendbufs && pass>0) {
                    flags |= SS_BLOB_FLUSH_SEND;
                }
#endif

                /* Flush */
                if (ss_blob_async_flush(gfile->gblob, d_idx, gfile->dxpl_independent, flags)<0) SS_ERROR(FAILED);
            }
        }
    }
    if (ss_blob_aggbuf_total_g>aggbufsize || ss_blob_sendbuf_total_g>sendbufs)
        SS_ERROR_FMT(FAILED, ("unable to prune down to requested level %lu/%lu",
                              (unsigned long)aggbufsize, (unsigned long)sendbufs));

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Flush all datasets
 *
 * Description: This function is a wrapper around ss_blob_async_flush() that applies that function to one or more datasets
 *              depending on whether the caller supplied a particular file and/or a particular dataset within that file.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, December  8, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_blob_async_flush_all(ss_gblob_t *gblob,      /* Optional blob info for the file over which to restrict the flushing. If
                                                 * this is the null pointer then all datasets of all files will be flushed. */
                        size_t d_idx,           /* Optional dataset within a particular file to be flushed. If this is the
                                                 * constant SS_NOSIZE then all datasets of the specified file are flushed.
                                                 * This argument is only consulted if GFILE is non-null (otherwise all
                                                 * datasets of all files are flushed). */
                        hid_t dxpl,             /* Dataset transfer property list for independent I/O. */
                        unsigned flags          /* These bit flags are passed directly to the ss_blob_async_flush() function
                                                 * for each dataset that is flushed, and control how the dataset is flushed. */
                        )
{
    SS_ENTER(ss_blob_async_flush_all, herr_t);
    size_t      gfile_idx;

    if (!gblob) {
        ss_gfile_t *gfile;
        for (gfile_idx=0; (gfile=SS_GFILE_IDX(gfile_idx)); gfile_idx++) {
            if (!gfile->cur_open) continue;
            for (d_idx=0; d_idx<gfile->gblob->d_nused; d_idx++) {
                if (ss_blob_async_flush(gfile->gblob, d_idx, dxpl, flags)<0) SS_ERROR(FAILED);
            }
        }
    } else if (SS_NOSIZE==d_idx) {
        for (d_idx=0; d_idx<gblob->d_nused; d_idx++) {
            if (ss_blob_async_flush(gblob, d_idx, dxpl, flags)<0) SS_ERROR(FAILED);
        }
    } else {
        if (ss_blob_async_flush(gblob, d_idx, dxpl, flags)<0) SS_ERROR(FAILED);
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Flush asynchronous data for a dataset
 *
 * Description: This function flushes certain kinds of asynchronous data for a particular dataset according to the bits in the
 *              FLAGS argument.
 *
 *              * SS_BLOB_FLUSH_SEND:  Block until all asynchronous send operations have completed for the specified dataset
 *                                     and then reap the associated resources.
 *
 *              * SS_BLOB_FLUSH_RECV:  Block until all asynchronous receive operations have completed for the specified
 *                                     dataset and then reap the associated resources. Reaping receive operations results in
 *                                     initiating (possibly asynchronous) write operations. When bits controlling receive and
 *                                     write are both turned on, the writes will be performed first (this allows an
 *                                     application to complete all writes and then initiate new writes with a single call).
 *
 *              * SS_BLOB_FLUSH_SHIP:  This is an alias for SS_BLOB_FLUSH_SEND together with SS_BLOB_FLUSH_RECV in order
 *                                     to flush all data shipping operations for two-phase I/O.
 *
 *              * SS_BLOB_FLUSH_WRITE: Block until asynchronous H5Dwrite() completes for this dataset. The write can't be
 *                                     initiated until the receives are all completed, and turning on this bit in and of
 *                                     itself won't cause the receives to be flushed.
 *
 *              The bits described above cause this function to block until the specified operation(s) is completed. The
 *              following bits only indicate that this function should check whether specified operations have already
 *              completed and reap their resources if so. It is not necessary to specify the !REAP bit for an operation if
 *              the !FLUSH bit is also set, since flushing includes reaping by definition.
 *
 *              * SS_BLOB_REAP_SEND:   Reap resources for send operations that have already completed.
 *
 *              * SS_BLOB_REAP_RECV:   Reap resources for receive operations that have already completed.
 *
 *              * SS_BLOB_REAP_SHIP:   This is an alias for SS_BLOB_REAP_SEND together with SS_BLOB_REAP_RECV in order to reap
 *                                     all resources for data shipping.
 *
 *              * SS_BLOB_REAP_WRITE:  Reap resources for H5Dwrite() calls that have already completed. It is not necessary
 *                                     to explicitly reap synchronous H5Dwrite() calls.
 *
 *              If the SS_STRICT bit is set then this function behaves as if all of the !FLUSH bits were turned on, but with
 *              the modification that receives are flushed before writes (this allows everything to be flushed with a single
 *              call to this function).
 *
 * Note:        This function does not call H5Fflush() or sync() or any other function that might ensure that data has
 *              actually made it all the way to the storage device.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. When MPI support is not present this function becomes a no-op since it's not possible to do data
 *              shipping or asynchronous I/O in that situation.
 *
 * Programmer:  Robb Matzke
 *              Monday, July 12, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_blob_async_flush(ss_gblob_t UNUSED_SERIAL *gblob,    /* The global blob list. */
                    size_t UNUSED_SERIAL d_idx,         /* The entry in the GBLOB list that should be flushed. */
                    hid_t UNUSED_SERIAL dxpl,           /* Data transfer property list for independent I/O. */
                    unsigned UNUSED_SERIAL flags        /* Specifies the kind of operations for which this function blocks. */
                    )
{
    SS_ENTER(ss_blob_async_flush, herr_t);
#ifdef HAVE_PARALLEL
    static MPI_Status   *statuses=NULL;                 /* Status results for MPI test and wait functions */
    static size_t       nstatuses=0;                    /* Number of entries allocated for statuses[] */
    int                 completed;
    size_t              i, tsize, bytes_used;
    ss_aio_t            *aiocb=NULL;
    hbool_t             reaped_recv=FALSE;

#if 0 /*RPM DEBUGGING 2004-12-06*/
#   define PR if (0==ss_mpi_comm_rank(SS_COMM_WORLD)) fprintf
    PR(sslib_g.warnings,
       "%d: flush gblob=0x%08lx, d_idx=%lu", ss_mpi_comm_rank(SS_COMM_WORLD), (unsigned long)gblob, (unsigned long)d_idx);
    fprintf(sslib_g.warnings, "[%d]", ss_mpi_comm_rank(SS_COMM_WORLD));
#endif

    /* Special cases for flags */
    if (flags & SS_STRICT) {
        flags |= SS_BLOB_FLUSH_SHIP | SS_BLOB_FLUSH_WRITE;
    } else if (0==(flags & (SS_BLOB_FLUSH_SHIP|SS_BLOB_FLUSH_WRITE|SS_BLOB_REAP_SHIP|SS_BLOB_REAP_WRITE))) {
        goto done;
    }
#if 0 /*RPM DEBUGGING 2004-12-06*/
    PR(sslib_g.warnings, " flags=0x%08x", flags & ~(SS_STRICT|
                                                    SS_BLOB_FLUSH_SEND|SS_BLOB_FLUSH_RECV|SS_BLOB_FLUSH_WRITE|
                                                    SS_BLOB_REAP_SEND|SS_BLOB_REAP_RECV|SS_BLOB_REAP_WRITE));
    if (flags & SS_STRICT) PR(sslib_g.warnings, "|STRICT");
    if (flags & SS_BLOB_FLUSH_SEND) PR(sslib_g.warnings, "|F_SEND");
    if (flags & SS_BLOB_FLUSH_RECV) PR(sslib_g.warnings, "|F_RECV");
    if (flags & SS_BLOB_FLUSH_WRITE) PR(sslib_g.warnings, "|F_WRITE");
    if (flags & SS_BLOB_REAP_SEND) PR(sslib_g.warnings, "|R_SEND");
    if (flags & SS_BLOB_REAP_RECV) PR(sslib_g.warnings, "|R_RECV");
    if (flags & SS_BLOB_REAP_WRITE) PR(sslib_g.warnings, "|R_WRITE");
    PR(sslib_g.warnings, ", %lu/%lu/%d",
       (unsigned long)(gblob->d[d_idx].agg.send_nused), (unsigned long)(gblob->d[d_idx].agg.recv_nused),
       gblob->d[d_idx].agg.aiocb?1:0);
#endif

    /* Check arguments */
    if (!gblob || d_idx>=gblob->d_nused)
        SS_ERROR_FMT(USAGE, ("gblob and/or d_idx is invalid"));

    /* Make sure we have room to store the status results */
    SS_EXTEND(statuses, MAX3(32,gblob->d[d_idx].agg.recv_nused,gblob->d[d_idx].agg.send_nused), nstatuses);

    /* Reap completed send requests or block until some complete. */
    if (gblob->d[d_idx].agg.send_nused && (flags & (SS_BLOB_FLUSH_SEND|SS_BLOB_REAP_SEND))) {
        if (flags & SS_BLOB_FLUSH_SEND) {
            if (MPI_Waitall((int)gblob->d[d_idx].agg.send_nused, gblob->d[d_idx].agg.send, statuses)) SS_ERROR(MPI);
            completed = TRUE;
        } else {
            if (MPI_Testall((int)gblob->d[d_idx].agg.send_nused, gblob->d[d_idx].agg.send, &completed, statuses))
                SS_ERROR(MPI);
        }
        if (completed) {
#if 0 /*RPM DEBUGGING 2004-12-06*/
            PR(sslib_g.warnings, "; SEND");
#endif
            gblob->d[d_idx].agg.send_nused = gblob->d[d_idx].agg.send_nalloc = 0;
            gblob->d[d_idx].agg.send = SS_FREE(gblob->d[d_idx].agg.send);
            for (i=0; i<gblob->d[d_idx].agg.sendbufs_nused; i++) {
                gblob->d[d_idx].agg.sendbufs[i] = SS_FREE(gblob->d[d_idx].agg.sendbufs[i]);
                --ss_blob_sendbuf_total_g;
            }
            gblob->d[d_idx].agg.sendbufs = SS_FREE(gblob->d[d_idx].agg.sendbufs);
            gblob->d[d_idx].agg.sendbufs_nused = gblob->d[d_idx].agg.sendbufs_nalloc = 0;
        }
    }

    /* Reap completed receives or block until some complete. */
    if (gblob->d[d_idx].agg.recv_nused && (flags & (SS_BLOB_FLUSH_RECV|SS_BLOB_REAP_RECV))) {
        if (flags & SS_BLOB_FLUSH_RECV) {
            if (MPI_Waitall((int)gblob->d[d_idx].agg.recv_nused, gblob->d[d_idx].agg.recv, statuses)) SS_ERROR(MPI);
            completed = TRUE;
        } else {
            if (MPI_Testall((int)gblob->d[d_idx].agg.recv_nused, gblob->d[d_idx].agg.recv, &completed, statuses))
                SS_ERROR(MPI);
        }
        if (completed) {
#if 0 /*RPM DEBUGGING 2004-12-06*/
            PR(sslib_g.warnings, "; RECV");
#endif
            /* Free receive requests */
            gblob->d[d_idx].agg.recv_nused = 0;
            gblob->d[d_idx].agg.recv_nalloc = 0;
            gblob->d[d_idx].agg.recv = SS_FREE(gblob->d[d_idx].agg.recv);

            /* Initiate H5Dwrite() calls, telling HDF5 to use asynchronous I/O if possible. We do this by using a specially
             * patched version of the mpiposix driver which looks for an "async" property in the data transfer property list
             * and calls the function pointed to by that property: ss_aio_hdf5_cb(). That function in turn looks for the
             * "async_req" property, uses it to initiate asynchronous I/O, and sets that property to null. */
            if (gblob->agg.asynchdf5) {
                aiocb = calloc(1, sizeof(*aiocb));
                aiocb->aio_buf = gblob->d[d_idx].agg.aggbuf;
                if (!H5Pexist(dxpl, "async")) {
                    int(*async_func)(int, hid_t, haddr_t, size_t, const void*) = ss_aio_hdf5_cb;
                    if (H5Pinsert(dxpl, "async", sizeof(void*), &async_func, NULL, NULL, NULL, NULL, NULL)<0)
                        SS_ERROR(HDF5);
                    if (H5Pinsert(dxpl, "async_req", sizeof(aiocb), &aiocb, NULL, NULL, NULL, NULL, NULL)<0)
                        SS_ERROR(HDF5);
                } else if (H5Pset(dxpl, "async_req", &aiocb)<0) {
                    SS_ERROR(HDF5);
                }
            } else {
                /* Make sure hdf5 doesn't do async I/O because of stale properties */
                void *null=NULL;
                if (H5Pexist(dxpl, "async_req") && H5Pset(dxpl, "async_req", &null)<0) SS_ERROR(HDF5);
            }

            /* HDF5 will write (or start to write) the data. */
            if (H5Dwrite(gblob->d[d_idx].dset, gblob->d[d_idx].dtype, gblob->d[d_idx].agg.iom, gblob->d[d_idx].agg.iof,
                         dxpl, gblob->d[d_idx].agg.aggbuf)<0)
                SS_ERROR(HDF5);
            
            /* Check whether HDF5 used asynchronous or synchronous I/O */
            if (aiocb) {
                void *pval;
                if (H5Pget(dxpl, "async_req", &pval)<0) SS_ERROR(FAILED);
                if (!pval) {
#if 0 /*RPM DEBUGGING 2004-12-06*/
                    fprintf(sslib_g.warnings, "SSlib-%d: started async H5Dwrite %lu bytes at %lu\n",
                            ss_mpi_comm_rank(SS_COMM_WORLD), (unsigned long)(aiocb->aio_nbytes),
                            (unsigned long)(aiocb->aio_offset));
#endif
                    gblob->d[d_idx].agg.aiocb = aiocb;
                } else {
                    void *null=NULL;
                    if (H5Pset(dxpl, "async_req", &null)<0) SS_ERROR(HDF5);
                    fprintf(sslib_g.warnings, "SSlib-%d: write was synchronous\n", ss_mpi_comm_rank(SS_COMM_WORLD));
                    SS_FREE(aiocb);
                }
            }

            /* Release data spaces */
            if (H5Sclose(gblob->d[d_idx].agg.iom)<0) SS_ERROR(HDF5);
            gblob->d[d_idx].agg.iom = 0;
            if (H5Sclose(gblob->d[d_idx].agg.iof)<0) SS_ERROR(HDF5);
            gblob->d[d_idx].agg.iof = 0;

            /* Release buffer unless H5Dwrite() was asynchronous */
            if (!gblob->d[d_idx].agg.aiocb) {
                gblob->d[d_idx].agg.aggbuf = gblob->d[d_idx].agg.aggbuf_freeable = SS_FREE(gblob->d[d_idx].agg.aggbuf_freeable);
                if (0==(tsize=H5Tget_size(gblob->d[d_idx].dtype))) SS_ERROR(HDF5);
                bytes_used = tsize * gblob->d[d_idx].agg.elmts_per_agg;
                SS_ASSERT(bytes_used>0);
                SS_ASSERT(ss_blob_aggbuf_total_g>=bytes_used);
                ss_blob_aggbuf_total_g -= bytes_used;
            }

            /* Remember that we started H5Dwrite */
            reaped_recv = TRUE;
        }
    }

    /* Reap completed H5Dwrite() calls or block until some complete */
    if (gblob->d[d_idx].agg.aiocb && (flags & (SS_BLOB_FLUSH_WRITE|SS_BLOB_REAP_WRITE)) &&
        (!reaped_recv || (flags & SS_STRICT))) {

        if (flags & SS_BLOB_FLUSH_WRITE) {
            if (ss_aio_suspend(&(gblob->d[d_idx].agg.aiocb), 1)<0)
                SS_ERROR_FMT(FAILED, ("aio_suspend: %s", strerror(errno)));
            SS_ASSERT(0==ss_aio_error(gblob->d[d_idx].agg.aiocb));
        }
        if (EINPROGRESS!=ss_aio_error(gblob->d[d_idx].agg.aiocb)) {
#if 0 /*RPM DEBUGGING 2004-12-06*/
            PR(sslib_g.warnings, "; WRITE");
#endif
            if (ss_aio_return(gblob->d[d_idx].agg.aiocb)<0)
                SS_ERROR_FMT(FAILED, ("aio_return failed: %s", strerror(errno)));
            /* Reap control block and release the aggregation buffer */
            gblob->d[d_idx].agg.aiocb = SS_FREE(gblob->d[d_idx].agg.aiocb);
            gblob->d[d_idx].agg.aggbuf = gblob->d[d_idx].agg.aggbuf_freeable = SS_FREE(gblob->d[d_idx].agg.aggbuf_freeable);
            if (0==(tsize=H5Tget_size(gblob->d[d_idx].dtype))) SS_ERROR(HDF5);
            bytes_used = tsize * gblob->d[d_idx].agg.elmts_per_agg;
            SS_ASSERT(bytes_used>0);
            SS_ASSERT(ss_blob_aggbuf_total_g>=bytes_used);
            ss_blob_aggbuf_total_g -= bytes_used;
        }
    }
#if 0 /*RPM DEBUGGING 2004-12-06*/
    PR(sslib_g.warnings, "; %lu/%lu/%d\n",
       (unsigned long)(gblob->d[d_idx].agg.send_nused), (unsigned long)(gblob->d[d_idx].agg.recv_nused),
       gblob->d[d_idx].agg.aiocb?1:0);
#endif

done:
SS_CLEANUP:
#endif /*HAVE_PARALLEL*/

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     I/O
 * Purpose:     Obtain and cache d_idx value for a blob
 *
 * Description: Each blob which is bound to a dataset (i.e., the !dsetaddr value is non-zero) also has an index into the Gblob
 *              array of the file to which the blob belongs.  This index can be calculated by doing a linear search of the
 *              Gblob table looking for an entry with the same dataset object ID as the !dsetaddr stored in the blob.  This
 *              function does that linear search and then caches the index so the search doesn't need to ever be repeated.
 *
 * Issue:       Since zero is a valid d_idx and since blob objects (like all other objects) are initialized to all zeros we
 *              have to be particularly careful when detecting whether a zero is really a cached d_idx or simply an indication
 *              that no d_idx value has yet been cached.
 *
 * Return:      Returns the d_idx value on success; returns SS_NOSIZE if the blob is not bound to a dataset or if some other
 *              error occurs.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, July 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_blob_didx(ss_blob_t *blob)
{
    SS_ENTER(ss_blob_didx, size_t);
    ss_gblob_t  *gblob=NULL;
    size_t      d_idx;

    if (!SS_BLOB(blob)) SS_ERROR_FMT(NOTFOUND, ("BLOB could not be dereferenced"));
    if (0==SS_BLOB(blob)->dsetaddr) SS_ERROR_FMT(USAGE, ("BLOB is not bound to a dataset"));
    SS_ASSERT(SS_GFILE_LINK(blob));
    gblob = SS_GFILE_LINK(blob)->gblob;
    SS_ASSERT(gblob);
    SS_ASSERT(gblob->d_nused>0);


    /* Do the linear search if nothing is cached yet. In the case of a cached zero (which is indistinguishable from the case
     * of nothing cached) the linear search will only look at the first item in the table before finding a match). */ 
    if (0==SS_BLOB(blob)->m.d_idx) {
        for (d_idx=0; d_idx<gblob->d_nused; d_idx++) {
            if (*((haddr_t*)(&gblob->d[d_idx].stat.objno)[0])==SS_BLOB(blob)->dsetaddr) {
                SS_BLOB(blob)->m.d_idx = d_idx;
                break;
            }
        }
        SS_ASSERT(d_idx<gblob->d_nused);
    }

SS_CLEANUP:
    SS_LEAVE(SS_BLOB(blob)->m.d_idx);
}

