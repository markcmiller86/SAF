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
SS_IF(table);

static hid_t            ss_table_space_g        = -1;           /* HDF5 initial data space for new tables */
static MPI_Datatype UNUSED_SERIAL ss_table_cksum_mpi=MPI_DATATYPE_NULL;           /* MPI datatype for ss_table_t */
static unsigned         ss_table_sync_serial_g;                 /* Serial number for ss_table_synchronize() */
ss_scope_t              ss_table_scope_g;                       /* Scope where I/O is currently occuring */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Note:        Table Properties
 *
 * Description: * !chunksize: This is the target chunk size in bytes. The actual chunk size may differ slightly due to the fact
 *              that the specified size might not be a multiple of the datatype size when the dataset is created. The default
 *              chunk size is defined in ss_table_new().
 *
 *              * !test:        If this boolean is set in a call to ss_table_synchronize() then that function will fail with a
 *                              SKIPPED error if the table is not synchronized.
 *
 *              * err_newptrs:  If this integer property is defined then the ss_table_synchronize() function will set it
 *                              to the number of objects that could not be completely synchronized because they point to
 *                              new objects.
 *
 *              * err_incompat: If this integer property is defined then the ss_table_synchronize() function will set it
 *                              to the number of objects that could not be synchronized because two or more tasks made
 *                              incompatible changes to those objects.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 *
 * Description: All objects of a particular class belonging to a particular scope are stored in a persistent object table. The
 *              table data structure is such that an object lookup (i.e., dereferencing a persistent object link to get a
 *              pointer to a persistent object) is a constant-time operation. The table is actually a fixed-length list of
 *              nodes where the first and second nodes have 64 entries and each subsequent node has twice as many as the
 *              previous node. Once a node is allocated it isn't freed or moved until the table is completely destroyed, a
 *              property that allows objects to be referenced by pointer and yet allows the table to grow over time without
 *              placing too much strain on malloc().
 *
 *              A table stores two types of objects: permanent and temporary. A permanent object has the property that once
 *              it's added to the table it's never deleted or moved. Like permanent objects, temporary objects have a fixed
 *              index in the table, but the memory for the temporary object can move around. Both types are objects are stored
 *              in the same array of objects, but temporary objects are looked up from an index by consulting a separate "map"
 *              array. The "map" array is an array of pointers into the object array. Object indices are marked as temporary
 *              by looking at the highest-order bit: relocatable objects have the bit set.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Interface initializer
 *
 * Description: Initializes the persistent object table interface
 *
 * Return:      Returns non-negative on success; negative on failure
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, July 31, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_init(void)
{
    hsize_t             curdim=0, maxdim=H5S_UNLIMITED;

#ifdef HAVE_PARALLEL
    int                 size[5];
    MPI_Aint            offset[5];
    MPI_Datatype        type[5];
#endif

    SS_ENTER_INIT;
    
    /* The HDF5 data space to use when creating datasets for new tables. */
    if ((ss_table_space_g = H5Screate_simple(1, &curdim, &maxdim))<0) SS_ERROR(INIT);

#ifdef HAVE_PARALLEL
    /* The MPI datatype for ss_table_cksum_t. Perhaps we should construct the HDF5 datatype first, and then call H5Tmpi() to
     * get the corresponding MPI datatype. */
    offset[0]=0;                                        size[0]=1;                      type[0]=MPI_LB;
    offset[1]=HOFFSET(ss_table_cksum_t, itemidx);       size[1]=1;                      type[1]=ss_mpi_size_mt;
    offset[2]=HOFFSET(ss_table_cksum_t, cksum.adler);   size[2]=1;                      type[2]=MPI_UNSIGNED_LONG;
    offset[3]=HOFFSET(ss_table_cksum_t, cksum.flags);   size[3]=1;                      type[3]=MPI_BYTE;
    offset[4]=sizeof(ss_table_cksum_t);                 size[4]=1;                      type[4]=MPI_UB;
    if (ss_table_cksum_mpi == MPI_DATATYPE_NULL) {
        if (MPI_Type_struct(5, size, offset, type, &ss_table_cksum_mpi)) SS_ERROR(MPI);
       }
    if (MPI_Type_commit(&ss_table_cksum_mpi)) SS_ERROR(MPI);
#ifndef NDEBUG
    {
        MPI_Aint extent;
        MPI_Type_extent(ss_table_cksum_mpi, &extent);
        SS_ASSERT(extent == sizeof(ss_table_cksum_t));
    }
#endif
#endif /*HAVE_PARALLEL*/
    
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Create a new empty object table
 *
 * Description: Each scope contains a table corresponding to each class of persistent object; each table contains all the
 *              persistent objects of a specified class belonging to a certain scope. The objects in a table may be in various
 *              states.
 *
 * Return:      Returns a pointer to the new table on success; the null pointer on failure.
 *
 * Parallel:    Creation of a scope in memory is an independent operation, but if the SCOPEGRP is supplied then a dataset is
 *              created in the file as well, making this function collective across the file's communicator.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_table_t *
ss_table_new(unsigned tableidx,         /* Table index number or magic number of object or link thereto */
             hid_t scopegrp,            /* Optional HDF5 group containing the scope to which this table belongs. A value
                                         * of one implies a transient file and such scopes are born read. */
             hbool_t create,            /* Should the HDF5 dataset be created or should it already exist? */
             const hid_t *tff_ary,      /* This is an optional array of table file datatypes, generally the !tff array from an
                                         * ss_gfile_t struct which points to shared versions of the !tff datatypes found in
                                         * the ss_pers_class_t array. */
             ss_table_t *exists,        /* Optional pointer to a table that already exists. If supplied then this function only
                                         * reinitializes the HDF5-level objects. */
             ss_prop_t *props           /* See Table Properties */
             )
{
    SS_ENTER(ss_table_new, ss_table_tP);
    hid_t               dcpl=-1;
    hsize_t             chunk_size;             /* Actual chunk size in terms of how many dataset elements */
    hsize_t             target_chunk_size;      /* Desired chunk size in terms of bytes */
    ss_table_t          *retval=NULL;
    ss_pers_class_t     *pc = ss_pers_class_g + SS_MAGIC_SEQUENCE(tableidx);
    size_t              tff_size;               /* Size of file datatype */
    hid_t               tff=-1;
    size_t              i;

    /* Create the table in memory */
    tableidx = SS_MAGIC_SEQUENCE(tableidx);
    if (NULL==(retval=exists) && NULL==(retval=SS_OBJ_NEW(ss_table_t))) SS_ERROR(CONS);
    retval->objsize = pc->t_size;
    retval->objmagic = SS_MAGIC_CONS(SS_MAGIC(ss_persobj_t), tableidx);
    SS_ASSERT(retval->objsize);

    if (tff_ary && tff_ary[tableidx]>0) {
        tff = tff_ary[tableidx];
        SS_ASSERT(H5Tequal(tff, pc->tff));
    } else {
        tff = pc->tff;
    }
    
    /* Create the dataset in the file */
    if (1==scopegrp) {
        /* This is a transient scope and therefore the table is empty */
        retval->state = SS_TABLE_STATE_READ;
        retval->nperm = retval->nentries = retval->nmap = 0;
        retval->dset = 0;
        for (i=0; i<SS_NSUBTABLES; i++)
            retval->subtab[i] = retval->maptab[i] = NULL;
    } else if (scopegrp>1) {
        if (create) {
            /* The table must necessarily be empty. */
            if (!props || SS_NOSIZE==(target_chunk_size=ss_prop_get_u(props, "chunksize")) || 0==target_chunk_size)
                target_chunk_size = 4096;
            if (0==(tff_size = H5Tget_size(tff))) SS_ERROR(HDF5);
            chunk_size = MAX(1, target_chunk_size/tff_size);
            if ((dcpl = H5Pcreate(H5P_DATASET_CREATE))<0) SS_ERROR(HDF5);
            if (H5Pset_chunk(dcpl, 1, &chunk_size)<0) SS_ERROR(HDF5);
            if ((retval->dset=H5Dcreate(scopegrp, pc->name, tff, ss_table_space_g, dcpl))<0) SS_ERROR(HDF5);
            if (H5Pclose(dcpl)<0) SS_ERROR(HDF5);
            dcpl = -1; /*closed*/

            /* If subtables are allocated then leave them allocated but zero out all the objects contained therein. This
             * normally only happens for Scope tables. Otherwise make sure the subtable pointers (which are invalid and used
             * only for recalculating indirect map pointers) should be reset to null pointers. */
            for (i=0; i<SS_NSUBTABLES; i++) {
                retval->maptab[i] = SS_FREE(retval->maptab[i]);
                if (retval->subtab[i]) {
                    if (SS_TABLE_STATE_READ==retval->state) {
                        memset(retval->subtab[i], 0, ss_table_subsize(i)*retval->objsize);
                    } else {
                        retval->subtab[i] = NULL;
                    }
                }
            }

            /* Set the table size to zero and mark it as having been read since we know the table is empty. */
            retval->nperm = retval->nentries = retval->nmap = 0;
            retval->state = SS_TABLE_STATE_READ;
        } else {
            if ((retval->dset=H5Dopen(scopegrp, pc->name))<0) SS_ERROR(HDF5);
            /* Issue: this would be the perfect time to check that the datatype of the dataset is compatible with the file
             *        datatype for the table.  This is how SSlib should implement foreward/backward compatibility. However,
             *        we'll delay the implementation for the time being and assume either the file format doesn't change or
             *        we'll get HDF5 datatype conversion errors reported when we try to read or modify an older file.
             *        --rpm 2003-08-08 */

            if (!exists) retval->state = SS_TABLE_STATE_BOOT;
        }
    } else {
        SS_ERROR_FMT(USAGE, ("SCOPEGRP is negative"));
    }
    
 SS_CLEANUP:
    if (dcpl>0) H5Pclose(dcpl);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Destroy an object table
 *
 * Description: Destroys a table, releasing all resources held by the table. However, if the SS_STRICT flag is not specified
 *              and the table is in the SS_TABLE_STATE_READ state (i.e., data was read from a file or there was no data to
 *              read) then the table is only partly destroyed, as described below.
 *
 *              The partial destruction of a table is necessary to free the potentially large array of persistent objects but
 *              yet maintain enough information so that links to those objects that might still exist in other parts of the
 *              library (in a `closed' state) and which have indirect object indices can have those indices converted to
 *              direct indices.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Issue:       In order to be independent this function does not close the HDF5 dataset (if any) associated with this table.
 *              That's not technically a problem though because when the HDF5 file is closed (which is collective across the
 *              file communicator) HDF5 will also close this dataset.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_dest(ss_table_t *table,        /* Table to be destroyed */
              unsigned flags            /* Zero for partial destruction or SS_STRICT for complete destruction */
              )
{
    SS_ENTER(ss_table_dest, herr_t);
    size_t               i,j;
    ss_persobj_t        *persobj;

    SS_ASSERT_TYPE(table, ss_table_t);

    if (flags & SS_STRICT) {
        for (i=0; i<SS_NSUBTABLES; i++) {
            if (SS_TABLE_STATE_CLOSED!=table->state) SS_FREE(table->subtab[i]);
            table->subtab[i] = NULL;
            table->maptab[i] = SS_FREE(table->maptab[i]);
        }
        table->state = SS_TABLE_STATE_NONE;
        SS_OBJ_DEST(table);
        table = SS_FREE(table);
    } else if (SS_TABLE_STATE_READ==table->state) {
        /* Free the subtables but don't null out the pointers because we'll need them in order to recompute the pointers in
         * the `maptab' sub tables if the table is ever reread from disk. See ss_table_redirect() for details. We _can_ null
         * out the subtable pointer if there are no map tables (there are no map tables iff maptab[0] is null). */
        for (i=0; i<SS_NSUBTABLES; i++) {
            if (table->subtab[i]) {
                size_t subsize = ss_table_subsize(i);
                for (j=0; j<ss_table_subsize(i); j++) {
                    persobj = (ss_persobj_t*)((char*)(table->subtab[i]) + j*table->objsize);
                    if (SS_MAGIC_OK(SS_MAGIC_OF(persobj)) && ss_pers_reset_(persobj, 0)<0) SS_ERROR(FAILED);
                }
                memset(table->subtab[i], 0, subsize*table->objsize);
                SS_FREE(table->subtab[i]);
            }
            if (NULL==table->maptab[0]) table->subtab[i] = NULL;
        }
        table->state = SS_TABLE_STATE_CLOSED;
        table->dset = -1; /* consider dataset to be closed */
    } else {
        SS_ERROR(NOTIMP);
    }

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Lookup entry by index
 *
 * Description: Given a global index to some object in the table's address space, return a pointer to that slot.  If the
 *              subtable isn't allocated yet then it will be (assuming the SS_STRICT bit isn't set in the FLAGS argument).
 *              Subtables are initialized to all zero when they are allocated. If the high-order bit of IDX is set then the
 *              lookup uses one level of indirection: the table map array.
 *
 * Return:      Returns a pointer to some entry in a table on success; the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, June 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_persobj_t *
ss_table_lookup(ss_table_t *table,      /* Table in which to look up an object */
                size_t idx,             /* The global index of the object within the table (direct or indirect) */
                unsigned flags          /* If the SS_STRICT bit is set then fail if the object is not allocated or if the
                                         * magic number is not in the ss_persobj_t class. Otherwise allocate memory in the
                                         * table but don't complain if no object is there yet. */
                )
{
    SS_ENTER(ss_table_lookup, voidP);
    size_t subtabidx;
    ss_persobj_t *retval=NULL;
    hbool_t is_indirect = idx & SS_TABLE_INDIRECT ? TRUE : FALSE;
    size_t locidx=idx;

    SS_ASSERT_TYPE(table, ss_table_t);
    if (SS_NOSIZE==(subtabidx=ss_table_localidx(&locidx))) SS_ERROR(FAILED);

    if (is_indirect) {
        /* Use map table. This should point to something in the same table (but no guarantee). */
        if (!table->maptab[subtabidx]) {
            if (flags & SS_STRICT) {
                SS_ERROR_FMT(NOTFOUND, ("bad indirect index %lu subtab=%lu locidx=%lu (null subtable)",
                                        (unsigned long)(idx & ~SS_TABLE_INDIRECT),
                                        (unsigned long)subtabidx, (unsigned long)locidx));
            }
            if (NULL==(table->maptab[subtabidx]=calloc(ss_table_subsize(subtabidx), sizeof(void*)))) SS_ERROR(RESOURCE);
        }
        retval = table->maptab[subtabidx][locidx];
        if (!retval) {
            SS_ERROR_FMT(NOTFOUND, ("bad indirect index %lu subtab=%lu locidx=%lu (null map entry)",
                                    (unsigned long)(idx & ~SS_TABLE_INDIRECT),
                                    (unsigned long)subtabidx, (unsigned long)locidx));
        }
        if (flags & SS_STRICT) {
            if (SS_TABLE_STATE_READ!=table->state) {
                SS_ERROR_FMT(NOTFOUND, ("bad indirect index %lu subtab=%lu locidx=%lu (table not in READ state)",
                                        (unsigned long)(idx & ~SS_TABLE_INDIRECT),
                                        (unsigned long)subtabidx, (unsigned long)locidx, SS_MAGIC_OF(retval)));
            }
            if (SS_MAGIC_CLASS(SS_MAGIC_OF(retval))!=SS_MAGIC(ss_persobj_t)) {
                SS_ERROR_FMT(NOTFOUND, ("bad indirect index %lu subtab=%lu locidx=%lu (magic=0x%08x)",
                                        (unsigned long)(idx & ~SS_TABLE_INDIRECT),
                                        (unsigned long)subtabidx, (unsigned long)locidx, SS_MAGIC_OF(retval)));
            }
        }
    } else {
        /* Direct index into object table. */
        if (!table->subtab[subtabidx]) {
            if (flags & SS_STRICT) {
                SS_ERROR_FMT(NOTFOUND, ("bad direct index %lu subtab=%lu locidx=%lu (null subtable)",
                                        (unsigned long)idx, (unsigned long)subtabidx, (unsigned long)locidx));
            }
            if (NULL==(table->subtab[subtabidx]=calloc(ss_table_subsize(subtabidx), table->objsize))) SS_ERROR(RESOURCE);
        }
        retval = (ss_persobj_t*)((char*)(table->subtab[subtabidx]) + (locidx * table->objsize));
        if (flags & SS_STRICT) {
            if (SS_TABLE_STATE_READ!=table->state) {
                SS_ERROR_FMT(NOTFOUND, ("bad direct index %lu subtab=%lu locidx=%lu (table not in READ state)",
                                        (unsigned long)idx, (unsigned long)subtabidx, (unsigned long)locidx));
            }
            if (SS_MAGIC_CLASS(SS_MAGIC_OF(retval))!=SS_MAGIC(ss_persobj_t)) {
                SS_ERROR_FMT(NOTFOUND, ("bad direct index %lu subtab=%lu locidx=%lu (magic=0x%08lx)",
                                        (unsigned long)idx, (unsigned long)subtabidx, (unsigned long)locidx,
                                        SS_MAGIC_OF(retval)));
            }
        }
    }

 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Convert indirect to direct index
 *
 * Description: Given a table return the direct index for the object to which IDX points. IDX may already be a direct index.
 *
 * Return:      Returns the direct index of the object to which IDX points.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, January  8, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_table_direct(ss_table_t *table,      /* Table in which IDX exists */
                size_t idx              /* Indirect index into the table */
                )
{
    SS_ENTER(ss_table_direct, size_t);
    size_t              orig_idx=idx;   /* Incoming value of IDX since we change it herein */
    size_t              subtabidx;      /* Which sub table */
    size_t              nelmts;         /* Number of objects stored in a particular subtable */
    size_t              retval=SS_NOSIZE;
    size_t              diff;
    char                *addr=NULL;

    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(SS_TABLE_STATE_READ==table->state || SS_TABLE_STATE_CLOSED==table->state);
    if (idx & SS_TABLE_INDIRECT) {
        /* Find the address corresponding to the indirect index */
        if (SS_NOSIZE==(subtabidx=ss_table_localidx(&idx))) SS_ERROR(FAILED);
        if (!table->maptab[subtabidx] || NULL==(addr=table->maptab[subtabidx][idx]))
            SS_ERROR_FMT(NOTFOUND, ("bad indirect %lu:%lu", (unsigned long)subtabidx, (unsigned long)idx));

        /* Convert the address into a direct index by looking at the subtab addresses */
        for (subtabidx=0; subtabidx<SS_NSUBTABLES; subtabidx++) {
            if (NULL==table->subtab[subtabidx]) break;
            nelmts = ss_table_subsize(subtabidx);
            if (addr>=(char*)(table->subtab[subtabidx]) && addr<(char*)(table->subtab[subtabidx])+(nelmts*table->objsize)) {
                diff = addr - (char*)(table->subtab[subtabidx]);
                SS_ASSERT(0==diff % table->objsize);
                retval = diff / table->objsize;

                /* If the address is larger than the stored number of permanent objects then use the indirect index instead.
                 * In other words, only return a direct index if we can guarantee that it's permanent. */
                if (retval>=table->nperm) retval = orig_idx;
                goto done;
            }
        }

        SS_ERROR(NOTFOUND);
    }

done:
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Object Tables
 * Purpose:     Find indirect indices for an object
 *
 * Description: Given a direct index for a table object, return the first indirect index which also points to the object and
 *              which is larger than BEYOND.
 *
 * Return:      Returns a matching indirect index on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 27, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_table_indirect(ss_table_t *table,            /* Table in which IDX exists */
                  size_t idx,                   /* Index for the object for which we are searching for an indirect index. This
                                                 * is usually a direct index but doesn't necessarily have to be such. */
                  size_t beyond                 /* Return an indirect index greater than this value. A value of zero means to
                                                 * return the first indirect index. This argument can be used to scan through
                                                 * a table looking for all matching indirect indices for a particular direct
                                                 * index. If this is a direct index (such as zero) then the first matching
                                                 * indirect index is returned. */
                  )
{
    SS_ENTER(ss_table_indirect, size_t);
    ss_persobj_t        *persobj=NULL;
    size_t              retval = SS_NOSIZE;
    size_t              first_subtab, subtabidx, nelmts, i;

    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(SS_TABLE_STATE_READ==table->state || SS_TABLE_STATE_CLOSED==table->state);
    if (SS_NOSIZE==beyond) SS_ERROR_FMT(USAGE, ("BEYOND is out of range"));

    /* Get the object address and minimum return value */
    if (NULL==(persobj=ss_table_lookup(table, idx, SS_STRICT))) SS_ERROR(FAILED);
    if (beyond & SS_TABLE_INDIRECT) {
        beyond++;
        first_subtab = ss_table_localidx(&beyond);
    } else {
        first_subtab = beyond = 0;
    }

    /* Scan through the indirect pointers to find the first one that points to persobj */
    for (subtabidx=first_subtab; subtabidx<SS_NSUBTABLES; subtabidx++) {
        if (NULL==table->maptab[subtabidx]) continue;
        nelmts = ss_table_subsize(subtabidx);
        for (i=beyond; i<nelmts; i++) {
            if (table->maptab[subtabidx][i]==persobj) {
                retval = SS_TABLE_INDIRECT | ss_table_globalidx(subtabidx, i);
                goto done;
            }
        }
        beyond = 0; /*start at zero in the next subtable*/
    }

done:
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Allocate a new object
 *
 * Description: When storage is desired for a new object this function can be called to allocate it.  When allocating an
 *              indirect object a new entry is added to the end of the map array that points to an object storage at the end
 *              of the object array.  When allocating a direct object this function will return a pointer to the first table
 *              slot that doesn't have a direct object already stored there, possibly relocating a temporary object that was
 *              there.
 *
 * Return:      A pointer to a new object on success; null on failure. The index of the new object is returned through the
 *              optional IDX argument.
 *
 * Parallel:    Independent when creating an indirect object; collective over the table's scope when creating a direct object.
 *              However, the collectivity can be relaxed a bit as long as all tasks create an object. For instance, some tasks
 *              might convert an indirect object to a direct object (see ss_table_mkdirect()) while others call this function.
 *
 * Programmer:  Robb Matzke
 *              Monday, June 30, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_persobj_t *
ss_table_newobj(ss_table_t *table,              /* Table in which to allocate a new object */
                size_t idxtype,                 /* Direct or indirect object (only high-order bit is consulted) */
                const ss_persobj_t *init,       /* Optional initial value of the new object */
                size_t *idx                     /* OUT: Optional returned direct or indirect index */
                )
{
    SS_ENTER(ss_table_newobj, ss_persobj_tP);
    ss_persobj_t *persobj = NULL;               /* returned value */
    size_t myidx;                               /* return value for IDX */
    hbool_t extended_map_array=FALSE;           /* did we extend the map array? */
    hbool_t is_indirect = idxtype & SS_TABLE_INDIRECT ? TRUE : FALSE;

    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(SS_TABLE_STATE_BOOT==table->state || SS_TABLE_STATE_READ==table->state);

    /* Find slot for the object and allocate storage if necessary. As noted in ss_table_redirect(), it should be possible to
     * reuse "holes" in the table, but we don't do that here because we lack the data structures that would make finding the
     * holes better than O(n) time complexity. */
    myidx = is_indirect ? table->nentries : table->nperm;
    if (NULL==(persobj=ss_table_lookup(table, myidx, 0))) SS_ERROR(FAILED);

    /* If we're asking for a direct object and an indirect object is already stored here then move the indirect object to the
     * end of the table. Be careful that we know where the end of the table really is since we haven't updated counters yet.
     * Once we've moved the object we *must* increase the table size even if we fail. */
    if (SS_MAGIC_OK(SS_MAGIC_OF(persobj))) {
        size_t locidx = persobj->mapidx;
        size_t maptab = ss_table_localidx(&locidx);
        ss_persobj_t *newstorage=NULL;

        SS_ASSERT(!is_indirect);                           /* because the destination slot is occupied, and... */
        SS_ASSERT(table->nperm<table->nentries);           /* ...because there must be at least one indirect object... */
        SS_ASSERT(persobj->mapidx & SS_TABLE_INDIRECT);    /* ...and the destination slot is that indirect object. */
        
        if (NULL==(newstorage=ss_table_lookup(table, table->nentries, 0))) SS_ERROR(FAILED);
        if (ss_obj_move((ss_obj_t*)persobj, (ss_obj_t*)newstorage, table->objsize)<0) SS_ERROR(FAILED);
        table->maptab[maptab][locidx] = newstorage;
        table->nentries++;
    }
    
    /* If an indirect index is desired then allocate one. We have to be sure to clean this up if we fail. */
    if (is_indirect) {
        void *tmp;
        size_t maptab, locidx;
        
        myidx = SS_TABLE_INDIRECT | table->nmap;
        tmp = ss_table_lookup(table, myidx, 0);
        SS_STATUS_OK; /* ss_table_lookup() fails because there isn't an indirect object with that index yet */
        SS_ASSERT(!tmp); /*there better not already be a pointer stored at that map entry!*/

        locidx = myidx;
        maptab = ss_table_localidx(&locidx);
        table->maptab[maptab][locidx] = persobj;
        table->nmap++;
        extended_map_array = TRUE;
    }
    
    /* Initialize the new object. If this fails the object's magic number is guaranteed to be zero. If we were creating the
     * object at the end of the table then all is okay, otherwise we might end up with an unused slot, but that's okay too
     * because the next permanent object will occupy it. */
    if (NULL==ss_obj_new((ss_obj_t*)persobj, table->objmagic, table->objsize, init)) SS_ERROR(FAILED);
    persobj->mapidx = myidx;
    
    /* Update table counters now that we know we'll succeed. */
    if (is_indirect) {
        table->nentries++;
    } else {
        table->nperm++;
        if (table->nperm > table->nentries) table->nentries = table->nperm;
    }
    
    /* Successful return by reference */
    if (idx) *idx = myidx;
    
 SS_CLEANUP:
    if (extended_map_array) {
        /* Reset the last map entry if we allocated one */
        size_t locidx = table->nmap-1;
        size_t tabidx = ss_table_localidx(&locidx);
        SS_ASSERT(table->nmap>0);
        table->maptab[tabidx][locidx] = NULL;
        --table->nmap;
    }
    
    SS_LEAVE(persobj);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Convert indirect to a direct object
 *
 * Description: Converts an indirect object to a direct object.
 *
 * Return:      Returns a pointer to the object on success; null on failure. The index of the new object is returned through
 *              the optional NEW_IDX argument.
 *
 * Parallel:    Collective across the table's scope. However, this can be relaxed a bit as long as all required tasks create
 *              an object. For instance, some tasks might call this function while others call ss_table_newobj() to create the
 *              direct object.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_persobj_t *
ss_table_mkdirect(ss_table_t *table,            /* Table on which to operate. */
                  size_t old_idx,               /* An indirect index for the object in question. */
                  size_t *new_idx               /* OUT: Optional returned direct index. */
                  )
{
    SS_ENTER(ss_table_mkdirect, ss_persobj_tP);
    ss_persobj_t        *curstorage=NULL;       /* current storage for the object in question */
    ss_persobj_t        *newstorage=NULL;       /* destination storage for new permanent object */
    unsigned char       temp[512];              /* temporary storage for swapping objects */
    size_t              idx;                    /* object index */
    size_t              maptab;                 /* index of map table */

    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(SS_TABLE_STATE_READ==table->state);

    /* Get pointer to current storage and the destination storage */
    if (0==(SS_TABLE_INDIRECT & old_idx)) SS_ERROR_FMT(USAGE, ("OLD_IDX must be indirect"));
    SS_ASSERT(table->nperm<table->nentries); /* because there must be at least one indirect object... OLD_IDX */
    if (NULL==(curstorage = ss_table_lookup(table, old_idx, SS_STRICT))) SS_ERROR(FAILED);
    SS_ASSERT(curstorage->mapidx==old_idx);
    if (NULL==(newstorage = ss_table_lookup(table, table->nperm, 0))) SS_ERROR(FAILED);

    if (curstorage!=newstorage) {
        /* The object in question must be moved to the permanent location in the table. That destination location is either
         * unoccupied (a hole created by ss_table_redirect() and having a zero magic number) or is occupied by some other
         * indirect indexed object. In either case we'll just swap the memory and make sure we update the map subtable (if
         * appropriate) to point to the new location for the object that was moved out of the way. */
        SS_ASSERT(table->objsize<=sizeof temp);
        memcpy(temp, newstorage, table->objsize);
        memcpy(newstorage, curstorage, table->objsize);
        memcpy(curstorage, temp, table->objsize);

        /* Change the indirect pointer for the object being converted to point to the new location */
        idx = newstorage->mapidx;
        SS_ASSERT(idx & SS_TABLE_INDIRECT);
        maptab = ss_table_localidx(&idx);
        table->maptab[maptab][idx] = newstorage;

        /* Change indirect pointer to point to the new location for the object (if any) that was in the way. */
        if (SS_MAGIC_OK(SS_MAGIC_OF(curstorage))) {
            idx = curstorage->mapidx;
            SS_ASSERT(idx & SS_TABLE_INDIRECT);
            maptab = ss_table_localidx(&idx);
            table->maptab[maptab][idx] = curstorage;
        }
    }
    
    /* Change the `mapidx' value to a direct index and update counters */
    if (new_idx) *new_idx = table->nperm;
    newstorage->mapidx = table->nperm;
    table->nperm++;

SS_CLEANUP:
    SS_LEAVE(newstorage);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Broadcast direct objects
 *
 * Description: Creation of a synchronized object requires collectivity across all tasks that own the object. That is, across
 *              all tasks that belong to the communicator for the scope to which the table that holds the object belongs.
 *              However, sometimes it's more convenient for one task to create these new "synchronized" objects and then all
 *              tasks call this function in order to get the same data.
 *
 *              Only one task (ROOT) can independently create the SS_ALLSAME objects, although any of the tasks may have
 *              outstanding, unsynchronized permanent or new objects. In other words, at the time of this call all non-root
 *              tasks have the same number of permanent objects registered in their tables, while the ROOT task has zero or
 *              more additional permanent objects.
 *
 *              This function is currently used when flushing objects that point into other files. The problem is that when
 *              such an object is written to the HDF5 file the File table in that scope must contain an entry for the
 *              pointed-to file, or such an entry is added as an SS_ALLSAME object (see ss_pers_convert_mf()). However, only
 *              scope task zero is writing the data and therefore the collectivity of the SS_ALLSAME object creation must be
 *              delayed. After the H5Dwrite() call has completed, ss_scope_flush() collectively calls this function with scope
 *              task zero serving as the root, and then flushes the File table.
 *
 * Example:     Task 15 makes these calls
 *
 *                  for (i=0; i<100; i++) {
 *                      char url[256];
 *                      ss_role_t *role = SS_PERS_NEW(scope, ss_role_t, SS_ALLSAME);
 *                      sprintf(url, "http://saf.llnl.gov/role/%d", i);
 *                      ss_string_set(SS_ROLE_P(role,url), url);
 *                  }
 *
 *              All tasks do the following:
 *
 *                  table = ss_scope_table(scope, SS_MAGIC(ss_role_t), NULL);
 *                  ss_table_broadcast(table, scope, 15);
 *
 * Issue:       The delaying of collectivity for SS_ALLSAME object creation can be a dangerous operation because an error
 *              between the ss_pers_new() call and the ss_table_broadcast() could leave tables in an inconsistent state. The
 *              ss_table_synchronize() function will therefore check (when debugging is enabled) that all tasks possess the
 *              same number of permanent objects.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the communicator of SCOPE. This function has serious problems during error recovery because
 *              the task with the error will bail out early, leaving the subsequent collective MPI functions hanging.
 *
 * Programmer:  Robb Matzke
 *              Thursday, January 22, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_broadcast(ss_table_t *table,           /* The table which is affected. */
                   ss_scope_t *scope,           /* The scope to which the table belongs. */
                   int root                     /* The root task which has the additional SS_ALLSAME objects. */
                   )
{
    SS_ENTER(ss_table_broadcast, herr_t);
    MPI_Comm    comm;                           /* Scope communicator */
    size_t      lo_water, hi_water;             /* Low (non-root) and high (root) values for table->nperm */
    int         self, ntasks;                   /* My task ID and number of MPI tasks in COMM */
    size_t      subtabidx;                      /* Which subtable are we talking about? */
    size_t      first_loc_idx;                  /* Local subtab index for first affected slot in subtable */
    size_t      subtabsize;                     /* Total number of elements in a particular subtable */
    size_t      nsubelmts;                      /* Number of elements affected in the subtable */
    void        *first_obj=NULL;                /* The first affected object in each subtable */
    void        *aside=NULL;                    /* Additional data for serialized objects */
    size_t      aside_nbytes;                   /* Size of the ASIDE buffer */
    ss_pers_class_t *pc=NULL;
    size_t      i, idx;

    /* Check various things first */
    SS_ASSERT_TYPE(table, ss_table_t);
    if (!ss_scope_table(scope, table->objmagic, table)) SS_ERROR_FMT(USAGE, ("TABLE is not owned by SCOPE"));
    if (ss_scope_comm(scope, &comm, &self, &ntasks)<0) SS_ERROR(FAILED);
    if (root>=ntasks) SS_ERROR_FMT(DOMAIN, ("ROOT is not a valid scope task"));
    if (1==ntasks) goto done;
    if (NULL==(pc=SS_PERS_CLASS(SS_MAGIC_SEQUENCE(table->objmagic)))) SS_ERROR(FAILED);
    
    /* Find out how many permanent objects everyone else has so the root task knows how many to broadcast. We know now that
     * there are at least two tasks, so we'll use task 0 or 1 (whichever is not the ROOT) as the low water mark and ROOT as
     * the high water mark. */
    lo_water = hi_water = table->nperm;
    if (ss_mpi_bcast(&lo_water, 1, MPI_SIZE_T, 0==root?1:0, comm)<0) SS_ERROR(FAILED);
    if (ss_mpi_bcast(&hi_water, 1, MPI_SIZE_T, root, comm)<0) SS_ERROR(FAILED);
    SS_ASSERT(root==self || lo_water==table->nperm);

    /* Process one subtable at a time */
    while (lo_water<hi_water) {
        /* Figure out which elements of the subtable are affected */
        first_loc_idx = lo_water;                                       
        subtabidx = ss_table_localidx(&first_loc_idx);
        subtabsize = ss_table_subsize(subtabidx);
        nsubelmts = MIN(subtabsize-first_loc_idx, hi_water-lo_water);

        /* Non-root tasks must make room for the new permanent objects. The easiest way to do that is to just create the
         * required number of permanent objects which we'll overwrite shortly. */
        if (self!=root) {
            for (i=0; i<nsubelmts; i++) {
                if (NULL==ss_table_newobj(table, 0, NULL, &idx)) SS_ERROR(FAILED);
                SS_ASSERT(idx==lo_water+i);
            }
        }

        /* All tasks find the address of the first object affected in this subtable */
        if (NULL==(first_obj=ss_table_lookup(table, lo_water, SS_STRICT))) SS_ERROR(FAILED);

        /* The root task will create an aside buffer by serializing the object and broadcast that buffer to all other tasks. */
        if (root==self &&
            NULL==(aside=ss_val_ser_encode(first_obj, NULL, nsubelmts, &aside_nbytes, pc->valinfo_nused, pc->valinfo)))
            SS_ERROR(FAILED);
        if (ss_mpi_bcast(&aside_nbytes, 1, MPI_SIZE_T, root, comm)<0) SS_ERROR(FAILED);
        if (root!=self && NULL==(aside=malloc(MAX(1, aside_nbytes)))) SS_ERROR(RESOURCE);
        if (aside_nbytes && ss_mpi_bcast(aside, aside_nbytes, MPI_BYTE, root, comm)<0) SS_ERROR(FAILED);

        /* The root task broadcasts the objects themselves, then everyone else decodes them with the aside buffer. */
        if (ss_mpi_bcast(first_obj, nsubelmts, pc->serialized, root, comm)<0) SS_ERROR(FAILED);
        if (root!=self && ss_val_ser_decode(first_obj, NULL, nsubelmts, aside, pc->valinfo_nused, pc->valinfo)<0) SS_ERROR(FAILED);
        aside = SS_FREE(aside);

        /* Update counters */
        lo_water += nsubelmts;
        SS_ASSERT(lo_water<=hi_water);
    }

done:
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Point an indirect at a direct object
 *
 * Description: A call to this function will cause the object at OLD_IDX to be discarded and any use of OLD_IDX will instead
 *              point to the same object as NEW_IDX.  OLD_IDX must be an indirect object and NEW_IDX must be a direct object.
 *              This function is almost identical to ss_table_mkdirect() except the old object is discarded.  The `mapidx' of
 *              the old object is set to SS_NOSIZE to indicate that it is no longer in use. If there are persistent object
 *              links that are caching this pointer they will notice that the `mapidx' value is incorrect and therefore
 *              recaclulate the pointer from the various indices also strored in the link.  The magic number is also cleared
 *              to indicate that the object is no longer valid.
 *
 * Return:      Returns the storage for the object with index NEW_IDX.
 *
 * Issue:       This function leaves a hole in the table because an object is removed. It would be safe to reuse the hole to
 *              store some other object since the other object must necessarily have a direct index different than NEW_IDX and
 *              an indirect index other than OLD_IDX and therefore ss_pers_deref() would never mistake the new data for the
 *              old object.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_persobj_t *
ss_table_redirect(ss_table_t *table,            /* Table being affected */
                  size_t old_idx,               /* Original object */
                  size_t new_idx                /* New object */
                  )
{
    SS_ENTER(ss_table_redirect, ss_persobj_tP);
    ss_persobj_t        *curstorage=NULL;       /* storage for the indirect object in question */
    ss_persobj_t        *newstorage=NULL;       /* storage for direct object destination */
    size_t              idx;                    /* object index */
    size_t              maptab;                 /* index of map table */

    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(SS_TABLE_STATE_READ==table->state);

    /* Get pointer to current storage and the direct storage */
    if (0==(SS_TABLE_INDIRECT & old_idx)) SS_ERROR_FMT(USAGE, ("OLD_IDX must be indirect"));
    if (new_idx>=table->nperm) SS_ERROR_FMT(USAGE, ("NEW_IDX must be direct"));
    if (NULL==(curstorage = ss_table_lookup(table, old_idx, SS_STRICT))) SS_ERROR(FAILED);
    if (NULL==(newstorage = ss_table_lookup(table, new_idx, SS_STRICT))) SS_ERROR(FAILED);
    SS_ASSERT(table->nperm<table->nentries); /* because there must be at least one indirect object... */
    SS_ASSERT(curstorage->mapidx==old_idx);
    SS_ASSERT(newstorage->mapidx==new_idx);
    SS_ASSERT(curstorage!=newstorage);

    /* The indirect index points to the new direct object */
    idx = old_idx;
    maptab = ss_table_localidx(&idx);
    table->maptab[maptab][idx] = newstorage;

    /* Release the old indirect object */
    ss_pers_reset_(curstorage, 0u);
    curstorage->mapidx = SS_NOSIZE;
    SS_OBJ_DEST(curstorage);

SS_CLEANUP:
    SS_LEAVE(newstorage);
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Convert global index to local indices
 *
 * Description: Given a table global element index, compute which subtable owns that object and what the subtable-local index
 *              of that object is.  This operation is contant time, requiring at most five comparisons.  The ELMTNUM can be a
 *              direct or indirect index since the high-order bit is ignored.
 *
 * Return:      Returns the index of the subtable by value, and the index within that subtable through ELMTNUM. If the
 *              incoming value of ELMTNUM is SS_NOSIZE (i.e., the maximum possible size_t value) then this function returns
 *              SS_NOSIZE and does not modify ELMTNUM.  The high-order bit of ELMTNUM is always zero on successful return.
 *
 * Parallel:    Independent.
 *
 * Programmer:  Robb Matzke
 *              Friday, June 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_table_localidx(size_t *elmtnum               /* On entry, the table-global index for a particular element. On successful
                                                 * return, the subtable-local index for that element. */
                  )
{
    SS_ENTER(ss_table_localidx, size_t);
    size_t      retval=SS_NOSIZE;

    if (SS_NOSIZE==*elmtnum) SS_ERROR(OVERFLOW);
    *elmtnum &= ~SS_TABLE_INDIRECT; /* turn off indirect bit */
    
    /* This is simply a binary search unrolled for performance reasons. Many apologies for the unreadable code. */
    if (*elmtnum < (size_t)1<<18) {
        if (*elmtnum < (size_t)1<<11) {
            if (*elmtnum < (size_t)1<<8) {
                if (*elmtnum < (size_t)1<<6) {
                    retval = 0;
                } else if (*elmtnum < (size_t)1<<7) {
                    retval = 1;
                    *elmtnum -= (size_t)1<<6;
                } else {
                    retval = 2;
                    *elmtnum -= (size_t)1<<7;
                }
            } else {
                if (*elmtnum < (size_t)1<<9) {
                    retval = 3;
                    *elmtnum -= (size_t)1<<8;
                } else if (*elmtnum < (size_t)1<<10) {
                    retval = 4;
                    *elmtnum -= (size_t)1<<9;
                } else {
                    retval = 5;
                    *elmtnum -= (size_t)1<<10;
                }
            }
        } else {
            if (*elmtnum < (size_t)1<<15) {
                if (*elmtnum < (size_t)1<<13) {
                    if (*elmtnum < (size_t)1<<12) {
                        retval = 6;
                        *elmtnum -= (size_t)1<<11;
                    } else {
                        retval = 7;
                        *elmtnum -= (size_t)1<<12;
                    }
                } else {
                    if (*elmtnum < (size_t)1<<14) {
                        retval = 8;
                        *elmtnum -= (size_t)1<<13;
                    } else {
                        retval = 9;
                        *elmtnum -= (size_t)1<<14;
                    }
                }
            } else {
                if (*elmtnum < (size_t)1<<16) {
                    retval = 10;
                    *elmtnum -= (size_t)1<<15;
                } else if (*elmtnum < (size_t)1<<17) {
                    retval = 11;
                    *elmtnum -= (size_t)1<<16;
                } else {
                    retval = 12;
                    *elmtnum -= (size_t)1<<17;
                }
            }
        }
    } else {
        if (*elmtnum < (size_t)1<<25) {
            if (*elmtnum < (size_t)1<<21) {
                if (*elmtnum < (size_t)1<<19) {
                    retval = 13;
                    *elmtnum -= (size_t)1<<18;
                } else if (*elmtnum < (size_t)1<<20) {
                    retval = 14;
                    *elmtnum -= (size_t)1<<19;
                } else {
                    retval = 15;
                    *elmtnum -= (size_t)1<<20;
                }
            } else {
                if (*elmtnum < (size_t)1<<23) {
                    if (*elmtnum < (size_t)1<<22) {
                        retval = 16;
                        *elmtnum -= (size_t)1<<21;
                    } else {
                        retval = 17;
                        *elmtnum -= (size_t)1<<22;
                    }
                } else {
                    if (*elmtnum < (size_t)1<<24) {
                        retval = 18;
                        *elmtnum -= (size_t)1<<23;
                    } else {
                        retval = 19;
                        *elmtnum -= (size_t)1<<24;
                    }
                }
            }
        } else {
            if (*elmtnum < (size_t)1<<28) {
                if (*elmtnum < (size_t)1<<26) {
                    retval = 20;
                    *elmtnum -= (size_t)1<<25;
                } else if (*elmtnum < (size_t)1<<27) {
                    retval = 21;
                    *elmtnum -= (size_t)1<<26;
                } else {
                    retval = 22;
                    *elmtnum -= (size_t)1<<27;
                }
            } else {
                if (*elmtnum < (size_t)1<<30) {
                    if (*elmtnum < (size_t)1<<29) {
                        retval = 23;
                        *elmtnum -= (size_t)1<<28;
                    } else {
                        retval = 24;
                        *elmtnum -= (size_t)1<<29;
                    }
                } else {
                    if (*elmtnum < (size_t)1<<31) {
                        retval = 25;
                        *elmtnum -= (size_t)1<<30;
                    } else {
                        /* This case is here because if size_t is wide enough then the SS_TABLE_INDIRECT bit won't interfere
                         * with the low-order 32 bits. Otherwise this case might never be reached. */                   
                        retval = 26;
                        *elmtnum -= (size_t)1<<31;
                    }
                }
            }
        }
    }
 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Convert local indices to global index
 *
 * Description: Given a subtable index and an item index relative to the start of that subtable, return the global index
 *              for that item.  Algthough SUBTABLEIDX must be a valid subtable index (i.e., zero through 26, inclusive) the
 *              ITEMIDX may be past the end of the subtable (as long as it's not past the whole table!). Therefore it's
 *              possible to normalize a local index with the following code:
 *
 *                      subtable = ss_table_localidx(ss_table_globalidx(subtable,itemidx), &itemidx);
 *
 * Return:      Returns the global item index on success, or SS_NOSIZE on failure. The only way this function can fail
 *              is if the SUBTABLEIDX combined with ITEMIDX makes a reference to an object which cannot be in the table
 *              because its global index would be larger than 32 bits or interferes with the SS_TABLE_INDIRECT bit.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, June 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_table_globalidx(size_t subtableidx,  /* index of the subtable, returned from ss_table_localidx() */
                   size_t itemidx       /* index within that subtable (or even past that subtable) */
                   )
{
    SS_ENTER(ss_table_globalidx, size_t);
    size_t tablebase=0, retval=SS_NOSIZE;

    if (subtableidx>=SS_NSUBTABLES) SS_ERROR(OVERFLOW);
    if (subtableidx) tablebase = (size_t)1 << (subtableidx+5);
    retval = tablebase + itemidx;
    if (retval<tablebase || SS_NOSIZE==retval || SS_TABLE_INDIRECT & retval) SS_ERROR(OVERFLOW);

 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Number of elements in a subtable
 *
 * Description: Computes the number of possible elements within a specified subtable. Subtables have the property that all but
 *              subtable zero has a size that is equal to the global index of the first item in that table. The zeroeth
 *              subtable has a special-case size of 64 elements.
 *
 * Return:      Number of elements within the subtable on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, June 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_table_subsize(size_t subtableidx)
{
    SS_ENTER(ss_table_subsize, size_t);
    size_t      retval = 64;

    if (subtableidx>=SS_NSUBTABLES) SS_ERROR(OVERFLOW);
    if (subtableidx) retval = (size_t)1 << (subtableidx+5);
 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Determine if object belongs to table
 *
 * Description: Given a table and a pointer, determine whether PTR is a pointer to a persistent object within the table. PTR must
 *              point to a valid persistent object -- one that has been initialized and has a valid mapidx backreference.
 *
 * Return:      Returns true (positive) if PTR points to a valid persistent object inside TABLE and false if not; returns
 *              negative on failure.
 *
 * Parallel:    Independent.
 *
 * Programmer:  Robb Matzke
 *              Friday, August  8, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_table_owns(ss_table_t *table,        /* A table to query */
              ss_persobj_t *ptr         /* Pointer to suspected object within the table */
              )
{
    SS_ENTER(ss_table_owns, htri_t);
    ss_persobj_t        *found=NULL;
    htri_t              retval=-1;
    size_t              locidx, subtabidx;

    /* Check args */
    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(SS_TABLE_STATE_BOOT==table->state || SS_TABLE_STATE_READ==table->state);
    SS_ASSERT_CLASS(ptr, ss_persobj_t);

    /* Convert object's back pointer into subtable and local item indices */
    locidx = ptr->mapidx;
    subtabidx = ss_table_localidx(&locidx);
    if (locidx==SS_NOSIZE) SS_ERROR(FAILED);

    /* Now we essentially do an ss_table_lookup() but without allocating subtables */
    if (ptr->mapidx & SS_TABLE_INDIRECT) {
        if (table->maptab[subtabidx]) found = table->maptab[subtabidx][locidx];
    } else {
        if (table->subtab[subtabidx]) found = (ss_persobj_t*)((char*)(table->subtab[subtabidx]) + (locidx * table->objsize));
    }

    /* See if what we looked up matches the pointer that was passed in */
    retval = (found==ptr);

 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Read a table from a file
 *
 * Description: This function reads a table from disk if it hasn't been already read (it is a no-op to attempt to read a table
 *              that's already in memory). In general, table datasets are opened collectively when a scope is opened, but are
 *              read independently the first time an object is dereferenced from in the table.
 *
 *              If the table already contains objects that are initialized then only the persistent part of those objects are
 *              affected. This is used by ss_scope_boot() in order to read a scope table from the file but where the first
 *              element of that table was already initialized and is being passed as the target of the SCOPE link.
 *
 * Return:      Returns a pointer to a new, initialized table on success; the null pointer on failure.  
 *
 * Parallel:    Independent.
 *
 * Issue:       It may be more efficient for this function to be collective across the scope's communicator and called when a
 *              scope is opened. Each task would be responsible for reading part of the dataset (aligned on GPFS page
 *              boundaries, etc) and broadcasting its information to the other tasks.  But in the end, every task of the
 *              scope's communicator would need the complete table.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_read(ss_table_t *table,                /* Table to be read. */
              ss_scope_t *scope                 /* Scope whose table we are about to read. */
              )
{
    SS_ENTER(ss_table_read, herr_t);
    hsize_t     dsize=0;                        /* table dataset elements remaining to be read */
    hsize_t     nperm=0;                        /* number of permanent table entries */
    hid_t       dspace=-1;                      /* the dataset data space */
    hid_t       mspace=-1;                      /* the memory data space */
    hsize_t     next_idx=0;                     /* next table index to be filled in */
    size_t      loc_idx=0;                      /* index into subtable */
    size_t      subtable_idx=0;                 /* which subtable of a table */
    hsize_t     subtab_size;                    /* number of elements in a particular subtable */
    ss_pers_class_t *pc=NULL;                   /* persistent object class for this table */
    hid_t       dxpl=-1;                        /* dataset transfer property list */
    void        *subtab_mem=NULL;               /* pointer into subtable memory for objects */
    void        *old_subtab[SS_NSUBTABLES];     /* old deallocated pointers when re-reading a table */

    SS_ASSERT_TYPE(table, ss_table_t);
    if (SS_TABLE_STATE_READ==table->state) goto done;

    /* Save old subtab pointers */
    SS_ASSERT(sizeof(old_subtab)==sizeof(table->subtab));
    memcpy(old_subtab, table->subtab, sizeof old_subtab);
    memset(table->subtab, 0, sizeof(table->subtab));

    /* Obtain information about the size of the dataset */
    pc = SS_PERS_CLASS(SS_MAGIC_SEQUENCE(table->objmagic));
    SS_ASSERT(pc);
    if ((dspace=H5Dget_space(table->dset))<0) SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
    if (1!=H5Sget_simple_extent_ndims(dspace)) SS_ERROR_FMT(DOMAIN, ("not a 1d dataset"));
    if (H5Sget_simple_extent_dims(dspace, &dsize, NULL)<0) SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
    if (dsize >= SS_TABLE_INDIRECT) SS_ERROR_FMT(OVERFLOW, ("table=\"%s\"", pc->name));
    nperm = dsize;

    /* Create the dataset transfer property list for independent I/O */
    if ((dxpl=H5Pcreate(H5P_DATASET_XFER))<0) SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
#ifdef HAVE_PARALLEL
    if (H5Pset_dxpl_mpio(dxpl, H5FD_MPIO_INDEPENDENT)<0) SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
#endif

    /* Tell datatype conversion functions what scope is being used for I/O so they can pick up things such as the string
     * table that can't be passed down through HDF5. */
    ss_table_scope_g = *scope;
            
    /* Read each subtable independently */
    while (dsize>0) {

        /* Allocate the next subtable. If the table was in the CLOSED state then any non-nil subtable pointers didn't really
         * point to allocated memory (well, it was once allocated but freed since then and the pointers were kept around only
         * for the ss_table_adjustmap() call below.  If the state was BOOT then there probably were no non-nil subtables
         * except for one case: when ss_scope_boot_top() creates the initial entry for the scope table, and in that case we
         * simply reuse the memory that was already allocated (the read will clobber the persistent part of those objects but
         * not the transient parts). */
        loc_idx = next_idx;
        if (SS_NOSIZE==(subtable_idx = ss_table_localidx(&loc_idx))) SS_ERROR_FMT(FAILED, ("table=\"%s\"", pc->name));
        if (SS_TABLE_STATE_BOOT==table->state && old_subtab[subtable_idx]) {
            subtab_mem = table->subtab[subtable_idx] = old_subtab[subtable_idx];
        } else {
            if (NULL==(subtab_mem = ss_table_lookup(table, (size_t)next_idx, 0))) SS_ERROR_FMT(FAILED, ("table=\"%s\"", pc->name));
        }
        if (SS_NOSIZE==(subtab_size = ss_table_subsize(subtable_idx))) SS_ERROR_FMT(FAILED, ("table=\"%s\"", pc->name));
        subtab_size = MIN(subtab_size, dsize);

        /* Create the memory and file spaces and selections */
        if ((mspace = H5Screate_simple(1, &subtab_size, NULL))<0) SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
        if (H5Sselect_slab(dspace, H5S_SELECT_SET, (hsize_t)0, &next_idx, &subtab_size)<0)
            SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
            
        /* Read data and convert it to the memory representation. The H5Dread() reads the persistent data into
         * memory, but we have to manually initialize the non-persistent part. If the non-persistent part looks like it's
         * already initialized (i.e., has the correct magic number) then don't bother resetting it to zero. This allows
         * the ss_scope_boot_top() function to initialize a scope table with the "self" entry at item index zero and then call
         * ss_table_read() to overwrite that object with the real data from the file without clobbering the `m' member
         * that's already been initialized. */
        if (H5Dread(table->dset, pc->tfm, mspace, dspace, dxpl, subtab_mem)<0) SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
        for (loc_idx=0; loc_idx<subtab_size; loc_idx++) {
            ss_persobj_t *persobj = (ss_persobj_t*)((char*)subtab_mem + loc_idx*table->objsize);
            if (NULL==ss_obj_new((ss_obj_t*)persobj, table->objmagic, table->objsize, persobj))
                SS_ERROR_FMT(FAILED, ("table=\"%s\"", pc->name));
            persobj->mapidx = next_idx + loc_idx;
            persobj->dirty = FALSE;
            persobj->synced = TRUE;

            /* If the client wishes for SSlib to check whether objects are modified regardless of the settings
             * for persobj->synced then we must compute the initial checksum. */
            if (sslib_g.sync_check && ss_pers_cksum(persobj, &(persobj->sync_cksum))<0)
                SS_ERROR_FMT(FAILED, ("table=\"%s\"", pc->name));
        }
            
        /* Update counters */
        dsize -= subtab_size;
        next_idx += subtab_size;
    }

    /* Close HDF5 objects */
    if (H5Sclose(dspace)<0) SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
    dspace = -1;
    if (H5Pclose(dxpl)<0) SS_ERROR_FMT(HDF5, ("table=\"%s\"", pc->name));
    dxpl = -1;

    /* Adjust any existing map pointers to point into the new subtables instead of the old ones. */
    if (SS_TABLE_STATE_CLOSED==table->state && ss_table_adjustmap(table, old_subtab)<0)
        SS_ERROR_FMT(FAILED, ("table=\"%s\"", pc->name));

    /* Finalize table */
    table->state = SS_TABLE_STATE_READ;
    table->nperm = table->nentries = nperm;
    ss_table_scope_g = SS_SCOPE_NULL;

done:
SS_CLEANUP:
    /* Release HDF5 resources */
    if (dspace>0) H5Sclose(dspace);
    if (mspace>0) H5Sclose(mspace);
    if (dxpl>0) H5Pclose(dxpl);
    memcpy(table->subtab, old_subtab, sizeof old_subtab);
    ss_table_scope_g = SS_SCOPE_NULL;
    
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Adjust indirect maps
 *
 * Description: When a file (or scope) is closed it's tables are partially destroyed by deallocating the subtables. However
 *              the indirect map entries and the subtable pointers are maintained for the sole purpose of converting dangling
 *              links from indirect links to direct links. When a file (or scope) is reopened the subtables are reread from
 *              disk requiring new subtables to be allocated. Once this happens the old (deallocated) subtable pointers are
 *              clobbered and the indirect map indices are no longer valid.
 *
 *              This function takes an array of old, deallocated subtable pointers and uses them to adjust the indirect map
 *              indices to point into the correct new subtables.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, March 22, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_adjustmap(ss_table_t *table,                   /* The table that was recently read from disk */
                   void *old_subtab[]                   /* The array of previous, deallocated subtables */
                   )
{
    SS_ENTER(ss_table_adjustmap, herr_t);
    size_t              subtable_idx;                   /* Index through the table's new subtables */
    size_t              old_subtable_idx;               /* Index through the table's old subtables */
    size_t              loc_idx;                        /* Index through the table's new subtable objects */
    size_t              old_loc_idx;                    /* Index through the table's old subtable objects */
    size_t              subtable_size;                  /* Size of the table's new subtables */
    size_t              old_subtable_size;              /* Size of the table's old subtables */
    char                *addr;                          /* Temporary pointer to an object */
    size_t              diff;                           /* Byte offset into an old subtable */
    hbool_t             found;                          /* Did we find the old memory for an indirect object? */

    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(old_subtab);

    for (subtable_idx=0; subtable_idx<SS_NSUBTABLES && table->maptab[subtable_idx]; subtable_idx++) {
        if (SS_NOSIZE==(subtable_size=ss_table_subsize(subtable_idx))) SS_ERROR(FAILED);
        for (loc_idx=0; loc_idx<subtable_size; loc_idx++) {
            if (NULL==(addr=table->maptab[subtable_idx][loc_idx])) continue;

            /* Find the old subtable that would have held this object. This is O(n), but since we're doing this repeatedly it
             * might be advantageous to use an index to get O(log N) here. [rpm 2004-03-22] */
            for (old_subtable_idx=0, found=FALSE; old_subtable_idx<SS_NSUBTABLES && !found; old_subtable_idx++) {
                if (NULL==old_subtab[old_subtable_idx]) continue;
                if (SS_NOSIZE==(old_subtable_size=ss_table_subsize(old_subtable_idx))) SS_ERROR(FAILED);
                if (addr>=(char*)(old_subtab[old_subtable_idx]) &&
                    addr<(char*)(old_subtab[old_subtable_idx])+(old_subtable_size*table->objsize)) {
                    diff = addr - (char*)(old_subtab[old_subtable_idx]);
                    SS_ASSERT(0==diff % table->objsize);
                    SS_ASSERT(NULL!=table->subtab[old_subtable_idx]);
                    old_loc_idx = diff / table->objsize;
                    SS_ASSERT(old_loc_idx<old_subtable_size);
                    table->maptab[subtable_idx][loc_idx] = (char*)(table->subtab[old_subtable_idx])+(old_loc_idx*table->objsize);
                    found = TRUE;
                }
            }
            SS_ASSERT(found);
        }
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Marks all objects as clean
 *
 * Description: This is a callback for ss_table_scan() which marks all direct-indexed objects as clean regardless of their
 *              current setting. This is used as the no-op table write for transient tables.
 *
 * Return:      Returns true (positive) if PERSOBJ is an indirect object, which causes the table scan operation to return
 *              early (all direct objects occur before the indirect objects in the table).  Returns false for indirect
 *              objects. Returns negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
htri_t
ss_table_cleanup_cb(size_t UNUSED itemidx, ss_pers_t UNUSED *pers, ss_persobj_t *persobj, void UNUSED *udata)
{
    SS_ENTER(ss_table_cleanup_cb, herr_t);
    herr_t      retval=FALSE;
    
    if (persobj->mapidx & SS_TABLE_INDIRECT) {
        retval = TRUE;
    } else {
        SS_ASSERT(persobj->synced);
        persobj->dirty = FALSE;
    }

SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Write table data to a file
 *
 * Description: A call to this function will result in "cleaning" the table by writing all dirty objects to the
 *              table's dataset.  If the underlying scope is a transient scope then no output is actually performed. Indexed
 *              (i.e., new objects) will not be written to the file.  It is the caller's responsibility to synchronize the
 *              table first.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Although this function is conceptually scope collective, it must be file collective because of the call to
 *              H5Dextend().  Fortunately the SCOPE exists on all tasks because it's stored in the Scope table of the
 *              top-level scope, which is open across the entire file.
 *
 * Issues:      The H5Dwrite() operations are currently independent from MPI task zero. Also, we use extendible datasets and
 *              data type conversions that will almost certainly incur some execution overhead in HDF5. However, we do use low
 *              and high water marks on each subtable in order to try to reduce the number of records that need to be written.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_write(ss_table_t *table,               /* table to write to disk */
               ss_scope_t *scope                /* scope that owns TABLE */
               )
{
    SS_ENTER(ss_table_write, herr_t);
    static const int    root=0;                 /* scope task that actually does the H5Dwrite() */
    int                 file_root=-1;           /* MPI file task corresponding to scope task ROOT */
    hid_t               dspace=-1;              /* data space for the table dataset */
    hid_t               mspace=-1;              /* memory data space */
    hid_t               dxpl=-1;                /* data transfer property list */
    hsize_t             dsize;                  /* dataset size */
    size_t              itemidx=0;              /* global item index */
    size_t              loc_idx;                /* local item index (w.r.t. subtable) */
    size_t              subtable_idx;           /* which subtable of the whole */
    hsize_t             subtable_size;          /* number of objects potentially stored in the subtable */
    char                *subtable_mem;          /* points to first persistent object in a subtable */
    hsize_t             dirty_min, dirty_max;   /* low and high water marks within a subtable */
    int                 self=-1;                /* MPI task rank of the communicator of the scope owning the table, or -1 */
    ss_pers_class_t     *pc=NULL;               /* class information for this table */
    ss_table_t          *filetab=NULL;          /* File table that might need broadcasting */
    htri_t              isopen;                 /* True if SCOPE is open; else calling task is an extra */
    ss_scope_t          topscope;               /* The top scope of SCOPE */
    MPI_Comm            scope_comm=SS_COMM_NULL;/* The scope communicator, only available if SCOPE is open on calling task */
    MPI_Comm            file_comm=SS_COMM_NULL; /* The file communicator obtained from the top-scope of SCOPE */
    struct exchange {
        htri_t          writable;               /* Is the SCOPE writable? */
        size_t          nperm;                  /* Size of the table */
    } exchange;                                 /* Data broadcast from scope task zero to all tasks of the file communicator */
    static MPI_Datatype exchange_type=MPI_DATATYPE_NULL;
    
    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT_TYPE(scope, ss_scope_t);
    SS_ASSERT(table==ss_scope_table(scope, SS_TABLE_ALL, table)); /*SCOPE must own TABLE*/
    if ((isopen=ss_scope_isopen(scope))<0) SS_ERROR(FAILED); /*extra tasks have a closed scope*/
    if (isopen && ss_scope_comm(scope, &scope_comm, &self, NULL)<0) SS_ERROR(FAILED);
    pc = SS_PERS_CLASS(SS_MAGIC_SEQUENCE(table->objmagic));
    SS_ASSERT(pc);

    /* If the table is transient then just mark all objects as clean and bail out all tasks */
    if (0==table->dset) {
        if (isopen && ss_table_scan(table, NULL, 0, ss_table_cleanup_cb, NULL)<0) SS_ERROR(FAILED);
        goto done;
    }

    /* Obtain the file communicator and file task number corresponding to the ROOT scope task. The problem here is that in
     * order to map the `root' task of the scope communicator to a task number (file_root) of the file communicator we need to
     * have both communicators, and the only way to have the scope communicator is if the scope is open on the calling task.
     * Therefore what we do is calculate the file_root on the tasks that can, and then max-reduce the value to get it to the
     * tasks where the scope is not open. */
    if (NULL==ss_pers_topscope((ss_pers_t*)scope, &topscope)) SS_ERROR(FAILED);
    if (ss_scope_comm(&topscope, &file_comm, NULL, NULL)<0) SS_ERROR(FAILED);
    if (isopen) {
        if ((file_root = ss_mpi_maptask(root, scope_comm, file_comm))<0) SS_ERROR(FAILED);
    } else {
        file_root = -1;
    }
#ifdef HAVE_PARALLEL
    {
        int tmp = file_root;
        if (MPI_Allreduce(&tmp, &file_root, 1, MPI_INT, MPI_MAX, file_comm)) SS_ERROR(MPI);
    }
#endif /*HAVE_PARALLEL*/
    
    /* If the scope is not writable then we've got problems. However, only the tasks in the scope communicator can make that
     * determination, so we'll have to broadcast it from one of those tasks. We could avoid the broadcast by depending on
     * H5Dextend() or H5Dwrite() to fail on a read-only file, but then the failure would not occur if the file was opened for
     * read-write access and then changed to read-only access. */
    if (isopen && (exchange.writable=ss_scope_iswritable(scope))<0) SS_ERROR(FAILED);

    /* Exchange the scope writability bit and the current table size by broadcasting from scope task zero. */
    if (root==self) exchange.nperm = table->nperm;
#ifdef HAVE_PARALLEL
    if (MPI_DATATYPE_NULL==exchange_type) {
        static int len[4] = {1, 1, 1, 1};
        static MPI_Aint disp[4] = {0, offsetof(struct exchange,writable), offsetof(struct exchange,nperm),
                                   sizeof(struct exchange)};
        MPI_Datatype types[4];
        types[0] = MPI_LB;
        types[1] = MPI_HTRI_T;
        types[2] = MPI_SIZE_T;
        types[3] = MPI_UB;
        if (MPI_Type_struct(4, len, disp, types, &exchange_type)) SS_ERROR_FMT(MPI, ("MPI_Type_struct"));
        if (MPI_Type_commit(&exchange_type)) SS_ERROR_FMT(MPI, ("MPI_Type_commit"));
    }
#endif /*HAVE_PARALLEL*/
    if (ss_mpi_bcast(&exchange, 1, exchange_type, file_root, file_comm)<0) SS_ERROR(FAILED);
    
    /* Extend the underlying dataset if necessary. The extra tasks don't know the current table size, so they must get it from
     * one of the scope tasks. For simplicity, we just have the ROOT scope task broadcast the table size. */
    if ((dspace = H5Dget_space(table->dset))<0) SS_ERROR(HDF5);
    if (H5Sget_simple_extent_dims(dspace, &dsize, NULL)<0) SS_ERROR(HDF5);
    if (exchange.nperm > dsize) {
        if (!exchange.writable) SS_ERROR_FMT(PERM, ("scope is read-only"));
        dsize = exchange.nperm; /*for type conversion*/
        if (H5Dextend(table->dset, &dsize)<0) SS_ERROR(HDF5);
        if (H5Sclose(dspace)<0) SS_ERROR(HDF5);
        if ((dspace = H5Dget_space(table->dset))<0) SS_ERROR(HDF5);
    }

    /* Tell datatype conversion functions what scope is being used for I/O so they can pick up things such as the string
     * table that can't be passed down through HDF5. */
    SS_ASSERT(SS_PERS_ISNULL(&ss_table_scope_g)); /*no recursion allowed*/
    ss_table_scope_g = *scope;
            
    /* Create the dataset transfer property list for independent I/O */
    if (self==root) {
        if ((dxpl=H5Pcreate(H5P_DATASET_XFER))<0) SS_ERROR(HDF5);
#ifdef HAVE_PARALLEL
        if (H5Pset_dxpl_mpio(dxpl, H5FD_MPIO_INDEPENDENT)<0) SS_ERROR(HDF5);
#endif /*HAVE_PARALLEL*/
    }

    /* Scan each subtable for dirty objects. Extra tasks do not participate. */
    if (isopen) {
        for (itemidx=0; itemidx<dsize; itemidx+=subtable_size) {
            loc_idx = itemidx;
            if (SS_NOSIZE==(subtable_idx = ss_table_localidx(&loc_idx))) SS_ERROR(FAILED);
            SS_ASSERT(0==loc_idx);
            if (SS_NOSIZE==(subtable_size = ss_table_subsize(subtable_idx))) SS_ERROR(FAILED);
            subtable_size = MIN(subtable_size, dsize-itemidx);
            if (NULL==(subtable_mem = (char*)ss_table_lookup(table, itemidx, 0))) SS_ERROR(FAILED);

            /* Find minimum and maximum dirty entries for this subtable */
            for (dirty_min=dirty_max=SS_NOSIZE; loc_idx<subtable_size; loc_idx++) {
                ss_persobj_t *persobj = (ss_persobj_t*)(subtable_mem + loc_idx * table->objsize);
                if (persobj->dirty) {
                    if (SS_NOSIZE==dirty_min) {
                        dirty_min = dirty_max = loc_idx;
                    } else {
                        dirty_max = loc_idx;
                    }
                }
            }

            /* If dirty objects are found then write them to the dataset and mark them as clean. */
            /* ISSUE: Because only task zero actually ever calls H5Dwrite(), only task zero needs to maintain the dirty bits on
             *        the objects. This makes life easier since we don't have to worry about transmitting the dirty bit when we
             *        transmit objects from one task to another, although doing so wouldn't be difficult with the serialization
             *        functions we have in place. */
            if (SS_NOSIZE!=dirty_min) {
                if (self==root) {
                    hsize_t nelmts = (dirty_max - dirty_min) + 1;
                    hsize_t dset_start = itemidx + dirty_min;
                    if (!exchange.writable) SS_ERROR_FMT(PERM, ("scope is read-only"));
                    if ((mspace = H5Screate_simple(1, &subtable_size, NULL))<0) SS_ERROR(HDF5);
                    if (H5Sselect_slab(mspace, H5S_SELECT_SET, (hsize_t)0, &dirty_min, &nelmts)<0) SS_ERROR(HDF5);
                    if (H5Sselect_slab(dspace, H5S_SELECT_SET, (hsize_t)0, &dset_start, &nelmts)<0) SS_ERROR(HDF5);
                    if (H5Dwrite(table->dset, pc->tfm, mspace, dspace, dxpl, subtable_mem)<0) SS_ERROR(HDF5);
                    if (H5Sclose(mspace)<0) SS_ERROR(HDF5);
                    mspace = -1;
                }

                for (loc_idx=dirty_min; loc_idx<=dirty_max; loc_idx++) {
                    ss_persobj_t *persobj = (ss_persobj_t*)(subtable_mem + loc_idx * table->objsize);
                    persobj->dirty = FALSE;
                }
            }

            /* ISSUE: The H5Dwrite() call will invoke ss_pers_convert_mf() to convert persistent object links from memory to
             * file representation. That call may need to independently create permanent File objects in the scope and
             * therefore at some point we'll need to collectively call ss_table_broadcast() to make sure the other tasks have
             * the same File objects. We'll call the ss_table_broadcast() here although it might be better to do it higher up
             * in order to aggregate some communication. For instance, ss_scope_flush() could flush each table except the File
             * table, then ss_table_broadcast() the File table, then flush the File table. */
            if (NULL==(filetab=ss_scope_table(scope, SS_MAGIC(ss_file_t), NULL))) SS_ERROR(FAILED);
            if (ss_table_broadcast(filetab, scope, 0)<0) SS_ERROR(FAILED);
        }
    }
    
  done:
    if (dspace>=0 && H5Sclose(dspace)<0) SS_ERROR(HDF5);
    dspace = -1;
    if (dxpl>=0 && H5Pclose(dxpl)<0) SS_ERROR(HDF5);
    dxpl = -1;
    ss_table_scope_g = SS_SCOPE_NULL;

#ifdef HAVE_PARALLEL
    if (MPI_Type_free(&exchange_type)) SS_ERROR(MPI);
#endif
    exchange_type = MPI_DATATYPE_NULL;


  SS_CLEANUP:
    if (dspace>=0) H5Sclose(dspace);
    if (mspace>=0) H5Sclose(mspace);
    if (dxpl>=0) H5Pclose(dxpl);
    ss_table_scope_g = SS_SCOPE_NULL;
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Synchronize a table across tasks
 *
 * Description: As MPI tasks independently modify objects they become out of sync with other MPI tasks. The purpose of this
 *              function is to collectively examine the objects and make sure all pertinent tasks have identical information
 *              about each object. See [Synchronization Algorithm] for more details.
 *
 * Return:      Returns non-negative if all objects were successfully synchronized; negative on failure.
 *
 * Parallel:    Collective across the communicator of the scope to which this table belongs.
 *
 * Issue:       This function should have some way to return a list of objects that could not be synchronized. This would
 *              allow all other objects to be synchronized and a list of exceptional objects (and the reasons for no
 *              synchronization) to be returned so we could try to synchronize again later. For instance, if the table
 *              contains objects that have links to other new objects then we can't synchronize them yet but we should still
 *              be able to synchronize the other stuff. This behavior could be controlled through the property list.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, August 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_synchronize(ss_table_t *table,         /* Table which is to be synchronized */
                     ss_scope_t *scope,         /* Scope to which TABLE belongs */
                     ss_prop_t *props           /* See [Synchronization Properties] */
                     )
{
    SS_ENTER(ss_table_synchronize, herr_t);
    size_t              item_idx;               /* index into the table */
    ss_persobj_t        *persobj=NULL;          /* storage for a persistent object */
    MPI_Comm            comm;                   /* communicator for the scope */
    int                 ntasks, root, self;     /* mpi task numbers w.r.t. the scope communicator */
    hbool_t             test_only=FALSE;        /* should we only test whether the table is synchronized? */
    ss_table_seen_t     seen;                   /* list of checksums that have been encountered */
    htri_t              have_seen;              /* have we encountered this checksum before? */
    size_t              seen_idx;               /* where is object in `seen' list */
    ss_val_cksum_t      cur_cksum;              /* current checksum of an object */
    int                 cmp;                    /* comparison result */
    ss_table_desire_t   desire;                 /* list of objects whose data needs to be exchanged */
    int                 err_incompat_changes=0; /* number of errors for incompatible changes to an object */
    int                 err_points_to_new=0;    /* number of objects that point to new objects and thus are not synchronized */

    /* Variable length array of this task's checksums */
    ss_table_cksum_t    *cksum=NULL;
    size_t              cksum_n=0, cksum_a=0;

    /* Variable length array of some other task's checksums */
    ss_table_cksum_t    *exchanged=NULL;
    size_t              *exchanged_n=NULL, exchanged_a=0;
    size_t              ex_idx;                 /* Index into `exchanged' array */

    ss_table_sync_serial_g++;
    memset(&seen, 0, sizeof seen);
    memset(&desire, 0, sizeof desire);

    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(SS_TABLE_STATE_READ==table->state);
    if (!ss_scope_table(scope, table->objmagic, table)) SS_ERROR_FMT(USAGE, ("TABLE is not owned by SCOPE"));
    if (ss_scope_comm(scope, &comm, &self, &ntasks)<0) SS_ERROR(FAILED);

#ifndef NDEBUG
    {
        /* We can check that ss_table_newobj() was always called collectively because all tasks should have the same number of
         * permanent objects in this table. See ss_table_broadcast() */
        size_t nperm = table->nperm;
        if (ss_mpi_bcast(&nperm, 1, MPI_SIZE_T, 0, comm)) SS_ERROR(FAILED);
        SS_ASSERT(nperm==table->nperm);
    }
#endif

    /* Get a list of all objects that are unsynchronized so that we can broadcast that list our peers when it's our turn.
     * Under normal operation an object is considered to be unsynchronized when the persobj->synced flag is false, but if a
     * previous sync_cksum was computed (i.e., non-zero) and it matches the current checksum then set the synced_flag back to
     * true.  We do this because sometimes the client thinks it made a modification but in actuality the value wasn't changed
     * or was changed back to the previous setting and it's cheaper to figure that out now locally. */
    for (item_idx=0; item_idx<table->nentries; item_idx++) {
        if (NULL==(persobj=ss_table_lookup(table, item_idx, 0))) {
            SS_ERROR(NOTFOUND);
            
        } else if (SS_MAGIC_OF(persobj)!=table->objmagic) {
            /* The table might have holes in it (see ss_table_redirect()) that are marked by a zero magic number. However, we
             * test this in such a way as to also detect coding problems where a table might contain an invalid object. */       
            SS_ASSERT(0==SS_MAGIC_OF(persobj));

        } else if (sslib_g.sync_check) {
            /* Since the global sync_check flag is set then we ignore the persobj->synced flag and go strictly by checksums.
             * In this case, the sync_cksum is initialized when the object is loaded from the file because we know it's
             * synchronized at that time.  So if the current checksum differs from the last-synchronized checksum then assume
             * the object needs to be synchronized.  But...
             *
             * If this object was "born synchronized" then compute checksums now because we couldn't do it before. See
             * ss_pers_new() for details.  In general we are unable to detect the accidental omission of line 7 in the
             * following code:
             *
             *   1  q = SS_PERS_NEW(tscope, ss_quantity_t, SS_ALLSAME);
             *   2  if (!q) SS_FAILED_WHEN("creating collective quantity");
             *   3  SS_QUANTITY(q)->flags = 100;
             *   4
             *   5  if (0==self) {
             *   6      SS_QUANTITY(q)->flags = 101;
             *   7      // SS_PERS_MODIFIED(q, 0); -- omitted by accident
             *   8  }
             *
             * If the object was marked as synchronized on all but one task then we must assume it truly was, otherwise task
             * zero will advertise one checksum while the other tasks advertise a different checksum and it will look like
             * tasks made inconsistent modifications. */
            if (ss_pers_cksum(persobj, &cur_cksum)<0) SS_ERROR(FAILED);
            if (persobj->synced==SS_ALLSAME) {
                /* Assume synchronized at previous call */
                persobj->sync_serial = ss_table_sync_serial_g-1;
                persobj->sync_cksum = cur_cksum;
                persobj->synced = TRUE;
            } else if (ss_val_cksum_cmp(&(persobj->sync_cksum), &cur_cksum)) {
                /* Not synchronized according to checksums */
                if (persobj->synced) {
                    if (ss_pers_dump_(persobj, scope, sslib_g.warnings, "  ",
                                      "ss_table_synchronize-%u: unsynchronized object marked as synchronized (fixed)",
                                      ss_table_sync_serial_g)<0) SS_ERROR(FAILED);
                    if (SS_STRICT==sslib_g.sync_check) SS_ERROR_FMT(FAILED, ("unsynchronized object marked as synchronized"));
                }
                persobj->synced = FALSE;
                if (NULL==(cksum = ss_table_sync_cksum(persobj, cksum, &cksum_n, &cksum_a, &cur_cksum))) SS_ERROR(FAILED);
            } else {
                /* Synchronized according to checksums. We don't warn here because it might have been due to a client
                 * modifying an object, marking it as unsynchronized, and then reverting to the original value. */
                persobj->synced = TRUE;
            }
            
        } else if (!persobj->synced) {
            /* Object has explicitly been marked as not synchronized. */
            if (ss_pers_cksum(persobj, &cur_cksum)<0) SS_ERROR(FAILED);
            if (!ss_val_cksum_isset(&(persobj->sync_cksum)) || ss_val_cksum_cmp(&(persobj->sync_cksum), &cur_cksum)) {
                if (NULL==(cksum = ss_table_sync_cksum(persobj, cksum, &cksum_n, &cksum_a, &cur_cksum))) SS_ERROR(FAILED);
            } else {
                persobj->synced = TRUE;
            }
        }
    }
    if (NULL==(exchanged_n=malloc(ntasks*sizeof(*exchanged_n)))) SS_ERROR(RESOURCE);
#ifdef HAVE_PARALLEL
    if (MPI_Allgather(&cksum_n, 1, ss_mpi_size_mt, exchanged_n, 1, ss_mpi_size_mt, comm)) SS_ERROR(MPI);
#else
    exchanged_n[0] = cksum_n;
#endif

    /* If this is only a test, then check to see if there's anything we need to do. */
    if (props && NULL==ss_prop_get(props, "test", H5T_NATIVE_HBOOL, &test_only)) SS_STATUS_OK;
    if (test_only) {
        for (root=0; root<ntasks; root++) {
            if (exchanged_n[root]) SS_ERROR(SKIPPED);
        }
        goto done;
    }

    /* If the scope is read-only and there's something to do synchronize then we've got a major problem: someone modified an
     * object that shouldn't have been modified! We intentionally do this _after_ the testing phase above so that a client can
     * query whether there are modified objects without getting an error for a read-only scope. */
    if (ss_scope_iswritable(scope)<=0) {
        for (root=0; root<ntasks; root++) {
            if (exchanged_n[root]) SS_ERROR_FMT(PERM, ("modified objects in a read-only scope"));
        }
    }
    
    /* Process persistent objects one task at a time in task rank order.  Each task will broadcast the checksums computed
     * above and then all tasks process the list. We might be able to save some time by hoisting the MPI_Bcast() out of the
     * loop and using MPI_Allgatherv() instead, but that could use a *lot* more memory. But since we already know the global
     * size of this array (sum of all exchanged_n[] from above) we could hoist if the memory consumption wouldn't be too bad.
     * This would probably give us a good benefit when running on lots of tasks but with very few unsynchronized objects (such
     * as when only one task is creating/modifying objects). */
    for (root=0; root<ntasks; root++) {
        if (!exchanged_n[root]) continue;

        /* Broadcast root's checksums -- including to root itself */
        SS_EXTEND(exchanged, exchanged_n[root], exchanged_a);
        if (self==root) memcpy(exchanged, cksum, exchanged_n[root]*sizeof(*exchanged));
        if (ss_mpi_bcast(exchanged, exchanged_n[root], ss_table_cksum_mpi, root, comm)<0) SS_ERROR(FAILED);

        for (ex_idx=0; ex_idx<exchanged_n[root]; ex_idx++) {
            if (SS_TABLE_INDIRECT & exchanged[ex_idx].itemidx) {
                if ((have_seen=ss_table_sync_seen(&seen, &(exchanged[ex_idx].cksum), &seen_idx))<0) SS_ERROR(FAILED);
                if (!have_seen) {
                    /* We haven't seen this checksum yet for a new object during this synchronization.  If an identical
                     * permanent object already exists we have to still create a new object (this allows a client to create
                     * multiple identical objects and then modify them later, but only if there's a synchronization between
                     * each creation).  So, in any case, convert the new object to a permanent object. The non-root tasks might
                     * have this data already (we could check their checksum lists), but we assume they don't and arrange to
                     * obtain the object from the ROOT task.  This is most likely the Right Thing to do because if the client
                     * had in fact created the same object on every task it would have probably used the SS_ALLSAME bit flag in
                     * order to avoid the synchronization altogether. The ss_table_newobj() and ss_table_mkdirect() are
                     * co-collective. */
                    persobj = root == self ?
                              ss_table_mkdirect(table, exchanged[ex_idx].itemidx, &item_idx) :
                              ss_table_newobj(table, 0, NULL, &item_idx);
                    if (!persobj) SS_ERROR(FAILED);
                    seen.obj[seen_idx].perm_idx = item_idx; /*save the object's new permanent home*/
                    if (exchanged[ex_idx].cksum.flags & SS_VAL_CKSUM_NEW) {
                        /* If the new object contains links to other new objects then the owner can't broadcast the object to
                         * the other tasks and therefore we can't completely synchronize it. The best we can do is give the
                         * object a permanent index in the table. The non-owner tasks mark the new object as synchronized but
                         * the owner doesn't--this is to prevent a later synchronize from thinking that all those tasks reset
                         * the object to zero and thus are in conflict with the changes made by the true owner. */
                        err_points_to_new++;
                        if (self!=root) {
                            persobj->synced = SS_ALLSAME; /*see above*/
                            persobj->sync_serial = ss_table_sync_serial_g;
                            ss_val_cksum_reset(&(persobj->sync_cksum));
                        }
                    } else {
                        if (ss_table_sync_desire(&desire, persobj, item_idx, root)<0) SS_ERROR(FAILED);
                        persobj->sync_serial = ss_table_sync_serial_g;
                        persobj->sync_cksum = exchanged[ex_idx].cksum;
                    }
                } else {
                    /* We already saw this checksum as a new object during this synchronization and have converted it to a
                     * permanent object so we can discard this object. This accomplishes two things:
                     *   1. When N tasks each create 1 object and that object is identical across all N tasks then we end up
                     *      creating a single permanent object as a result of the synchronization.  This makes the use of
                     *      SS_ALLSAME in the ss_pers_new() call less critical.
                     *   2. When one task creates N identical objects, a synchronization will result in a single permanent
                     *      object for that task.  This is simply a storage optimization (perhaps it's even the wrong way
                     *      to do business.
                     * It is safe to discard the object because it will be discarded by all tasks. The root task actually
                     * points the indirect indices all at the same object, but no data is exchanged for any task. */
                    if (root==self &&
                        NULL==ss_table_redirect(table, exchanged[ex_idx].itemidx, seen.obj[seen_idx].perm_idx))
                        SS_ERROR(FAILED);
                }
            } else {
                /* Item is modified but not new */
                if (NULL==(persobj = ss_table_lookup(table, exchanged[ex_idx].itemidx, SS_STRICT))) SS_ERROR(FAILED);
                if (ss_pers_cksum(persobj, &cur_cksum)<0) SS_ERROR(FAILED);
                cmp = ss_val_cksum_cmp(&cur_cksum, &(exchanged[ex_idx].cksum));

                if (exchanged[ex_idx].cksum.flags & SS_VAL_CKSUM_NEW) {
                    /* The object in question contains references to other objects that have never been synchronized (i.e., new
                     * objects) and which don't yet have a permanent entry in a table. Links to new objects are not inter-task
                     * portable and therefore it is impossible to synchronize objects that contain such links. */
                    err_points_to_new++;
#if 1 /*DEBUGGING [rpm 2004-08-25]*/
                    ss_pers_dump_(persobj, scope, sslib_g.warnings, "  ", "this object points to new indirect objects (warning)");
#endif
                } else if (persobj->sync_serial==ss_table_sync_serial_g &&
                    ss_val_cksum_cmp(&(persobj->sync_cksum), &(exchanged[ex_idx].cksum))) {
                    /* We've already encountered this object from a lower-rank task, but this time the checksum is different
                     * than what that original task advertised. This means that two tasks have modified the object in
                     * incompatible ways, which is an error. However, we can somewhat recover from that error by using the
                     * value that will be supplied by the lower-rank task. */
                    if (self==root) {
                        size_t search_idx;
                        for (search_idx=0; search_idx<desire.nused; search_idx++)
                            if (desire.obj[search_idx].itemidx==exchanged[ex_idx].itemidx) break;
                        SS_ASSERT(search_idx<desire.nused);
                        ss_pers_dump_(persobj, scope, sslib_g.warnings, "  ", "incompatible changes w.r.t. task %d (mine clobbered)", 
                                      ss_mpi_maptask(desire.obj[search_idx].root, comm, SS_COMM_WORLD));
                        SS_STATUS_OK;
                    }
                    err_incompat_changes++; /*delay actual error reporting until the end*/
                    
                } else if (persobj->sync_serial==ss_table_sync_serial_g) {
                    /* We've already encountered this object from a lower-rank task. Two or more tasks are making identical
                     * changes to this object. We only need to get the object from the lower-rank task, so there's nothing to
                     * do here. */
                    
                } else if (cmp) {
                    /* We haven't encountered this object yet and our current checksum is not the same as the one being
                     * advertised from the `root' task (and thus self!=root). We will arrange to receive the object from the
                     * root task and update the checksum in expectation of getting the object. */
                    persobj->sync_serial = ss_table_sync_serial_g;
                    persobj->sync_cksum = exchanged[ex_idx].cksum;
                    if (ss_table_sync_desire(&desire, persobj, exchanged[ex_idx].itemidx, root)<0) SS_ERROR(FAILED);
                    
                } else if (self==root) {
                    /* We own the object and no other lower-rank task has advertised a checksum for this object. Therefore it
                     * is our responsibility to broadcast this object to the other tasks. We will assume, for simplicity, that
                     * all other tasks will need it, which is the expected common case. */
                    persobj->sync_serial = ss_table_sync_serial_g;
                    persobj->sync_cksum = exchanged[ex_idx].cksum;
                    if (ss_table_sync_desire(&desire, persobj, exchanged[ex_idx].itemidx, root)<0) SS_ERROR(FAILED);
                    
                } else {
                    /* We haven't encountered this object by any lower-ranked task, nor are we the root task for this object,
                     * nor do we need to obtain this object from the root (because our checksum matches the advertised
                     * checksum). However, since the root task will be broadcasting the object shortly, we must be prepared to
                     * receive it. */
                    persobj->sync_serial = ss_table_sync_serial_g;
                    persobj->sync_cksum = exchanged[ex_idx].cksum;
                    if (ss_table_sync_desire(&desire, persobj, exchanged[ex_idx].itemidx, root)<0) SS_ERROR(FAILED);
                }
            }
        }
    }

    /* Exchange object data as necessary and update all tables. Mark exchanged objects as synchronized. */
    if (ss_table_sync_bcast(&desire, scope, table->objmagic)<0) SS_ERROR(FAILED);

 done:
    cksum = SS_FREE(cksum);
    exchanged = SS_FREE(exchanged);
    exchanged_n = SS_FREE(exchanged_n);
    seen.obj = SS_FREE(seen.obj);
    desire.obj = SS_FREE(desire.obj);

    /* If the caller passed in a property list with the appropriate properties then return information about error conditions
     * through that property list. */
    if (props) {
        if (ss_prop_has(props, "err_newptrs") &&
            ss_prop_set_i(props, "err_newptrs", err_points_to_new)<0)
            SS_ERROR(FAILED);
        if (ss_prop_has(props, "err_incompat") &&
            ss_prop_set_i(props, "err_incompat", err_incompat_changes)<0)
            SS_ERROR(FAILED);
    }
    
    /* Delayed error reporting */
    if (err_incompat_changes)
        SS_ERROR_FMT(FAILED, ("two or more tasks made incompatible changes to %d object%s",
                              err_incompat_changes, 1==err_incompat_changes?"":"s"));
    if (err_points_to_new)
        SS_ERROR_FMT(FAILED, ("object%s that point%s to new objects could not be synchronized",
                              1==err_points_to_new?"":"s", 1==err_points_to_new?"s":""));

 SS_CLEANUP:
    cksum = SS_FREE(cksum);
    exchanged = SS_FREE(exchanged);
    seen.obj = SS_FREE(seen.obj);
    desire.obj = SS_FREE(desire.obj);
    exchanged_n = SS_FREE(exchanged_n);
    
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Add checksum information to an array
 *
 * Description: This function adds all necessary information about checksums to the end of an array that will be later
 *              exchanged with other MPI tasks.
 *
 * Return:      Returns a pointer to the possibly relocated array on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_table_cksum_t *
ss_table_sync_cksum(ss_persobj_t *persobj,              /* Persistent object whose checksum will be computed */
                    ss_table_cksum_t *cksum_ary,        /* Array to which to append the checksum information */
                    size_t *cksum_n,                    /* Current number of elements in the array */
                    size_t *cksum_a,                    /* Allocated size of the array in elements */
                    const ss_val_cksum_t *cksum         /* Optional checksum to store instead of computing one */
                    )
{
    SS_ENTER(ss_table_sync_cksum, ss_table_cksum_tP);
    ss_table_cksum_t    *slot=NULL;
    ss_val_cksum_t      cksum_here;
    
    SS_ASSERT_CLASS(persobj, ss_persobj_t);
    SS_ASSERT(cksum_n && cksum_a && *cksum_n<=*cksum_a);
    SS_ASSERT(!cksum_ary || *cksum_n>0);

    /* Extend the array if necessary. */
    SS_EXTEND(cksum_ary, MAX(32,*cksum_n+1), *cksum_a);

    /* Compute current checksum if none supplied */
    if (!cksum) {
        ss_pers_cksum(persobj, &cksum_here);
        cksum = &cksum_here;
    }
    
    /* Initialize new slot of the array */
    slot = cksum_ary + *cksum_n;
    slot->cksum = *cksum;
    slot->itemidx = persobj->mapidx;

    /* Success */
    (*cksum_n)++;

 SS_CLEANUP:
    SS_LEAVE(cksum_ary);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Keep track of encountered objects
 *
 * Description: This function keeps track of what persistent objects have been encountered by keeping a variable length list
 *              of object checksums. Each time it's called it searches for the specified checksum and adds it to the sorted
 *              list.
 *
 * Return:      Returns true (positive) if the checksum CKSUM was previously stored in the SEEN list, false otherwise. Returns
 *              negative on error.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_table_sync_seen(ss_table_seen_t *seen,               /* Information about object's we've encountered */
                   ss_val_cksum_t *cksum,               /* Object checksum */
                   size_t *seen_idx                     /* OUT: optional index where info is stored. */
                   )
{
    SS_ENTER(ss_table_sync_seen, htri_t);
    size_t      lt, md, rt;
    htri_t      retval=FALSE;
    int         cmp;

    SS_ASSERT(seen);

    /* Binary search */
    lt = md = 0;
    rt = seen->nused;
    cmp = -1;
    while (lt<rt) {
        md = (lt+rt)/2;
        cmp = ss_val_cksum_cmp(cksum, &(seen->obj[md].cksum));
        if (cmp<0) {
            rt = md;
        } else if (cmp>0) {
            lt = md+1;
        } else {
            break;
        }
    }
    if (cmp>0) md++;
    if (0==cmp) {
        retval = TRUE;
    } else {
        SS_EXTEND(seen->obj, MAX(128,seen->nused+1), seen->nalloc);
        memmove(seen->obj+md+1, seen->obj+md, (seen->nused-md)*sizeof(seen->obj[0]));
        seen->nused++;
        memset(seen->obj+md, 0, sizeof(seen->obj[0]));
        seen->obj[md].cksum = *cksum;
    }
    if (seen_idx) *seen_idx = md;

SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Records objects to broadcast
 *
 * Description: Records which objects need to be broadcast from the ROOT task to the other tasks.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the scope communicator because eventually (in the ss_table_sync_bcast() call) we'll be
 *              operating collectively on the DESIRE data structure.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_sync_desire(ss_table_desire_t *desire,         /* Information about desired objects */
                     ss_persobj_t *storage,             /* Source or destination for object inter-task transfer. If the caller
                                                         * is the ROOT task then this should be the source object; otherwise
                                                         * this is the destination. */
                     size_t storage_idx,                /* Where the object is (or will be) stored on current task */
                     int root                           /* MPI task that owns the object */
                     )
{
    SS_ENTER(ss_table_sync_desire, herr_t);
    size_t              n;

    SS_EXTEND(desire->obj, MAX(256,desire->nused+1), desire->nalloc);
    n = desire->nused;

    desire->obj[n].storage = storage;
    desire->obj[n].itemidx = storage_idx;
    desire->obj[n].root = root;
    desire->nused++;

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Broadcast object info
 *
 * Description: Sends objects from tasks that have correct data to those that need it. Currently this is just a broadcast of
 *              each object from the root tasks.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the scope communicator. All tasks must pass an identical list of objects for the DESIRE
 *              argument, although the storage locations will all be local pointers.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_table_sync_bcast(ss_table_desire_t *desire,  /* Object exchange information (identical on all tasks) */
                    ss_scope_t *scope,          /* Scope being synchronized */
                    unsigned objmagic           /* Magic number for the objects being synchronized */
                    )
{
    SS_ENTER(ss_table_sync_bcast, herr_t);
    MPI_Comm            scopecomm;
    int                 self;
    size_t              i, nbytes;
    char                *aside=NULL;
    ss_pers_class_t     *pc=NULL;

    if (0==desire->nused) goto done;
    if (ss_scope_comm(scope, &scopecomm, &self, NULL)<0) SS_ERROR(FAILED);
    if (NULL==(pc=SS_PERS_CLASS(SS_MAGIC_SEQUENCE(objmagic)))) SS_ERROR(FAILED);
    

    for (i=0; i<desire->nused; i++) {
        
        /* Some debugging output for now */
        if (sslib_g.sync_bcast && self==desire->obj[i].root) {
            ss_pers_dump_(desire->obj[i].storage, scope, sslib_g.warnings, "  ",
                          "ss_table_sync_bcast: broadcasting the following object", self);
            SS_STATUS_OK;
        }

        /* This certainly isn't the final implementation! We just broadcast the object from the root task to all other tasks.
         * The object is zeroed out on all receivers before being broadcasted. This is because the object has local memory
         * that is probably caching the persistent stuff (e.g., like the hid_t `type' member of an ss_array_t caching the
         * serialized type description `enctype'). */
        if (self==desire->obj[i].root &&
            NULL==(aside = ss_val_ser_encode(desire->obj[i].storage, NULL, 1, &nbytes, pc->valinfo_nused, pc->valinfo)))
            SS_ERROR(FAILED);
        if (ss_mpi_bcast(&nbytes, 1, MPI_SIZE_T, desire->obj[i].root, scopecomm)<0) SS_ERROR(FAILED);
        if (!aside && nbytes>0 && NULL==(aside=malloc(nbytes))) SS_ERROR(RESOURCE);
        if (ss_mpi_bcast(aside, nbytes, MPI_BYTE, desire->obj[i].root, scopecomm)<0) SS_ERROR(FAILED);
        if (self!=desire->obj[i].root && ss_pers_reset_(desire->obj[i].storage, 0)<0) SS_ERROR(FAILED);
        if (ss_mpi_bcast(desire->obj[i].storage, 1, pc->serialized, desire->obj[i].root, scopecomm)<0) SS_ERROR(FAILED);
        if (self!=desire->obj[i].root) {
            if (ss_val_ser_decode(desire->obj[i].storage, NULL, 1, aside, pc->valinfo_nused, pc->valinfo)<0) SS_ERROR(FAILED);
        }
        aside = SS_FREE(aside);

        /* Mark the object as synchronized */
        if (self!=desire->obj[i].root) desire->obj[i].storage->dirty = TRUE;
        desire->obj[i].storage->synced = TRUE;
        desire->obj[i].storage->mapidx = desire->obj[i].itemidx;

#ifndef NDEBUG
        {
            /* Make sure that the new object value has the correct checksum */
            ss_val_cksum_t cksum;
            if (ss_pers_cksum(desire->obj[i].storage, &cksum)<0) SS_ERROR(FAILED);
            if (ss_val_cksum_cmp(&(desire->obj[i].storage->sync_cksum), &cksum))
                SS_ERROR_FMT(FAILED, ("checksum failure"));
        }
#endif
    }

done:
SS_CLEANUP:
    SS_FREE(aside);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Object Tables
 * Purpose:     Visit each member of a table
 *
 * Description: Invoke the FUNC callback for each defined member of the desired table of the specified scope until that
 *              function returns non-zero.  The starting index can be specified to initiate the scan at a position other than
 *              zero. If the starting index is a direct index then all items are scanned in the order stored in the table (the
 *              callback will know when the indirect objects begin because the object->m.mapidx value will be an indirect
 *              index.  When the starting index is an indirect index then all indirect indices are scanned, which might result
 *              in certain underlying objects being scanned more than one time if they have multiple indirect indices (the
 *              callback can prevent this by processing only indirect objects whose object->m.mapidx is equal to the indirect
 *              object passed into the callback). Also note that all indirect objects also have a direct index, but the
 *              inverse is not true -- some direct objects don't have an indirect index.
 *
 * Return:      A positive return value of the callback cancels the scan and causes this function to return the same value. A
 *              negative return of the callback results in a FAILED error by this function and immediate termination of the
 *              scan. This function returns false if the scan reached the end of the table without the callback ever returning
 *              non-zero. This function returns negative for any errors detected herein.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, August 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_table_scan(ss_table_t *table,                /* table of which we will scan through all members */
              ss_scope_t *scope,                /* Optional scope that owns TABLE. If the scope is provided then the FUNC
                                                 * will be invoked with object links in addition to the object pointers
                                                 * themselves. This is useful, for example, when FUNC needs to invoke other
                                                 * functions that take object links instead of object pointers. */
              size_t start,                     /* starting index, direct or indirect */
              ss_table_scan_t func,             /* scanning callback function */
              void *udata                       /* data to pass through to FUNC */
              )
{
    SS_ENTER(ss_table_scan, htri_t);
    htri_t              retval=FALSE;
    size_t              loc_idx;                /* object index with respect to beginning of subtable */
    size_t              subtab_idx;             /* index of a subtable */
    size_t              subtab_size;            /* size of a subtable */
    size_t              subtab_end;             /* subtable holding first object past end of current table */
    size_t              locidx_end;             /* index into subtab_end for first object past end of table */
    size_t              nitems;                 /* number of items through a loop */
    size_t              itemidx;                /* global index for item in question */
    ss_pers_t           pers=SS_PERS_NULL;      /* link to pass to callback if SCOPE is defined */
    ss_persobj_t        *persobj;               /* persistent object to pass down to callback function */
    htri_t              status;                 /* callback return status */

    SS_ASSERT_TYPE(table, ss_table_t);
    SS_ASSERT(SS_TABLE_STATE_READ==table->state);

    if (start & SS_TABLE_INDIRECT) {
        /* Scan through the indirect map */
        start &= ~SS_TABLE_INDIRECT;
        locidx_end = table->nmap;
        subtab_end = ss_table_localidx(&locidx_end);
        for (/*void*/; start<table->nmap; start+=nitems) {
            loc_idx = start;
            subtab_idx = ss_table_localidx(&loc_idx);
            SS_ASSERT(subtab_idx!=SS_NOSIZE);
            subtab_size = ss_table_subsize(subtab_idx);
            SS_ASSERT(subtab_size!=SS_NOSIZE);
            nitems = subtab_size - loc_idx;
            if (table->maptab[subtab_idx]) {
                for (/*void*/; loc_idx<(subtab_idx==subtab_end ? locidx_end : subtab_size); loc_idx++) {
                    persobj = table->maptab[subtab_idx][loc_idx];
                    if (SS_MAGIC_OK(SS_MAGIC_OF(persobj))) {
                        itemidx = ss_table_globalidx(subtab_idx, loc_idx);
                        if (scope && NULL==ss_pers_refer(scope, persobj, &pers)) SS_ERROR(FAILED);
                        status = (func)(itemidx|SS_TABLE_INDIRECT, scope?&pers:NULL, persobj, udata);
                        if (status<0) SS_ERROR(FAILED);
                        if (status>0) {
                            retval = status;
                            goto done;
                        }
                    }
                }
            }
        }
    } else {
        /* Scan directly */
        locidx_end = table->nentries;
        subtab_end = ss_table_localidx(&locidx_end);
        for (/*void*/; start<table->nentries; start+=nitems) {
            loc_idx = start;
            subtab_idx = ss_table_localidx(&loc_idx);
            SS_ASSERT(subtab_idx!=SS_NOSIZE);
            subtab_size = ss_table_subsize(subtab_idx);
            SS_ASSERT(subtab_size!=SS_NOSIZE);
            nitems = subtab_size - loc_idx;
            if (table->subtab[subtab_idx]) {
                for (/*void*/; loc_idx<(subtab_idx==subtab_end ? locidx_end : subtab_size); loc_idx++) {
                    persobj = (ss_persobj_t*)((char*)(table->subtab[subtab_idx]) + (loc_idx * table->objsize));
                    if (SS_MAGIC_OK(SS_MAGIC_OF(persobj))) {
                        itemidx = ss_table_globalidx(subtab_idx, loc_idx);
                        if (scope && NULL==ss_pers_refer(scope, persobj, &pers)) SS_ERROR(FAILED);
                        status = (func)(itemidx, scope?&pers:NULL, persobj, udata);
                        if (status<0) SS_ERROR(FAILED);
                        if (status>0) {
                            retval = status;
                            goto done;
                        }
                    }
                }
            }
        }
    }

 done:
 SS_CLEANUP:
    SS_LEAVE(retval);
}
