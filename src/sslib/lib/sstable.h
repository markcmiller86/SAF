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
#ifndef SS_HAVE_SSTABLE_H
#define SS_HAVE_SSTABLE_H

#define SS_NSUBTABLES           (32-5)          /* 32 bit indices with the first table reponsible for 5 bits of index */

typedef enum ss_table_state_t {
    SS_TABLE_STATE_NONE=0,
    SS_TABLE_STATE_BOOT,                        /* table has been booted but not yet read */
    SS_TABLE_STATE_READ,                        /* table data has been read from the file (or file is empty) */
    SS_TABLE_STATE_CLOSED                       /* objects have been released but `maptab' still exists. In this case the
                                                 * `subtab' pointers still exist as do the `maptab' pointers, but the memory
                                                 * into which they point has been freed -- the pointers are only used to
                                                 * calculate direct indices via pointer arithmetic. */
} ss_table_state_t;

typedef struct ss_table_t {
    ss_obj_t            obj;                    /* inherits from object */
    size_t              objsize;                /* size of each object entry in the table */
    unsigned            objmagic;               /* magic number for all objects in this table */
    ss_table_state_t    state;                  /* one of the SS_TABLE_STATE_* constants */
    size_t              nperm;                  /* number of permanent (direct, non-relocatable, initial) entries */
    size_t              nentries;               /* total number of active entries (total number of objects) */
    size_t              nmap;                   /* number of entries used in map array */
    void                *subtab[SS_NSUBTABLES]; /* subtables allocated on demand */
    void                **maptab[SS_NSUBTABLES];/* map subtables for temporary items, points into subtab, allocated on demand */
    hid_t               dset;                   /* hdf5 dataset for this table (zero for transient table) */
} ss_table_t;

#define SS_TABLE_DIRECT         0               /* just a place holder for 2nd arg of ss_table_newobj() */
#define SS_TABLE_INDIRECT       ((size_t)1<<(8*sizeof(size_t)-1))

#define SS_TABLE_ALL            SS_PERS_NCLASSES /* signifies all tables for certain function calls */

/* This is the information exchanged between tasks during a synchronization operation. WARNING: THERE IS A CORRESPONDING MPI
 * DATATYPE DEFINED IN ss_table_init() THAT MUST ALSO BE ADJUSTED!!!!! */
typedef struct ss_table_cksum_t {
    size_t              itemidx;                /* Index into table, indirect if and only if object is new */
    ss_val_cksum_t      cksum;                  /* Checksum for the object */
} ss_table_cksum_t;

/* This struct keeps track of objects by checksum and is used during synchronization. */
typedef struct ss_table_seen_t {
    size_t              nused;                  /* Number of slots used in the malloc'd `obj' array herein */
    size_t              nalloc;                 /* Total number of slots allocated for the `obj' array */
    struct {
        ss_val_cksum_t  cksum;                  /* Checksums that was seen (sort key) */
        size_t          perm_idx;               /* The new, not yet initialized, permanent location for the object */
    } *obj;
} ss_table_seen_t;

/* This struct keeps track of objects that need to be exchanged between tasks during a synchronization operation. */
typedef struct ss_table_desire_t {
    size_t              nused;                  /* Number of slots used in the malloc'd `obj' array herein */
    size_t              nalloc;                 /* Total number of slots allocated for the `obj' array */
    struct {
        ss_persobj_t    *storage;               /* Where the object is stored on the `root' task; destination on others */
        size_t          itemidx;                /* Index (direct) for the `storage' in the specified `scope' (for debugging) */
        int             root;                   /* The MPI task w.r.t. the scope communicator to serve as the object source */
    } *obj;
} ss_table_desire_t;

typedef htri_t (*ss_table_scan_t)(size_t itemidx, ss_pers_t *pers, ss_persobj_t *persobj, void *udata);
extern ss_scope_t ss_table_scope_g;             /* Scope where I/O is currently occuring */

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
herr_t ss_table_init(void);
ss_table_t *ss_table_new(unsigned tableidx, hid_t scopegrp, hbool_t create, const hid_t *tff_ary, ss_table_t *exists,
                         ss_prop_t *props);
herr_t ss_table_dest(ss_table_t *table, unsigned flags);
ss_persobj_t *ss_table_lookup(ss_table_t *table, size_t idx, unsigned flags);
size_t ss_table_direct(ss_table_t *table, size_t idx);
size_t ss_table_indirect(ss_table_t *table, size_t idx, size_t beyond);
ss_persobj_t *ss_table_newobj(ss_table_t *table, size_t idxtype, const ss_persobj_t *init, size_t *idx);
ss_persobj_t *ss_table_mkdirect(ss_table_t *table, size_t old_idx, size_t *new_idx);
herr_t ss_table_broadcast(ss_table_t *table, ss_scope_t *scope, int root);
ss_persobj_t *ss_table_redirect(ss_table_t *table, size_t old_idx, size_t new_idx);
size_t ss_table_localidx(size_t *elmtnum);
size_t ss_table_globalidx(size_t subtableidx, size_t itemidx);
htri_t ss_table_owns(ss_table_t *table, ss_persobj_t *ptr);
size_t ss_table_subsize(size_t subtableidx);
herr_t ss_table_read(ss_table_t *table, ss_scope_t *scope);
herr_t ss_table_adjustmap(ss_table_t *table, void *old_subtab[]);
herr_t ss_table_write(ss_table_t *table, ss_scope_t *scope);
herr_t ss_table_synchronize(ss_table_t *table, ss_scope_t *scope, ss_prop_t *props);
ss_table_cksum_t *ss_table_sync_cksum(ss_persobj_t *persobj, ss_table_cksum_t *cksum_ary, size_t *cksum_n, size_t *cksum_a,
                                      const ss_val_cksum_t *cksum);
htri_t ss_table_sync_seen(ss_table_seen_t *seen, ss_val_cksum_t *cksum, size_t *seen_idx);
herr_t ss_table_sync_desire(ss_table_desire_t *desire, ss_persobj_t *storage, size_t objidx, int root);
herr_t ss_table_sync_bcast(ss_table_desire_t *desire, ss_scope_t *scope, unsigned objmagic);
htri_t ss_table_scan(ss_table_t *table, ss_scope_t *scope, size_t start, ss_table_scan_t func, void *udata);
htri_t ss_table_cleanup_cb(size_t itemidx, ss_pers_t *pers, ss_persobj_t *persobj, void *udata);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSTABLE_H*/
