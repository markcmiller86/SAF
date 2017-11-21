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
#ifndef SS_HAVE_SSSCOPE_H
#define SS_HAVE_SSSCOPE_H

typedef struct ss_scopeobj_tm {         /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */

    /* The following are set after booting the scope */
    hid_t               gid;            /* HDF5 group where this scope lives. 0 means never booted; negative means error
                                         * encountered when booting; 1 means transient scope booted; other is an HDF5 group
                                         * handle. */
    ss_string_table_t   *strings;       /* Variable length strings for objects in this scope */
    ss_table_t          *table[SS_PERS_NCLASSES];

    /* The following are set after opening the scope and reset when closing */
    MPI_Comm            comm;           /* Communicator for this scope. MPI_COMM_NULL (more portably, SS_COMM_NULL) means the
                                         * scope is not opened at this task. */                                  
    hbool_t             comm_duped;     /* True if the scope owns the communicator and will free it when closing. */
    unsigned            flags;          /* Various H5F_ACC flags, a subset of those specified for the whole file */
} ss_scopeobj_tm;

typedef struct ss_scopeobj_tf {         /* Memory-resident persistent information */
    ss_string_t         name;           /* Name of the group in the HDF5 file that holds the scope */
} ss_scopeobj_tf;

/* Information to pass through ss_table_scan() for ss_scope_synchronize_cb() */
typedef struct ss_scope_sync_t {
    unsigned            tableidx;       /* Table to be synchronized */
    ss_prop_t           *props;         /* Optional properties */
    int                 transient_errors;/* Number of errors that might be fixed by an additional synchronization operation */
} ss_scope_sync_t;
    
/* Types generated from above declarations */
#include "ssscopetab.h"

#ifdef __cplusplus
extern "C" {
#endif

herr_t ss_scopetab_init(void); /*to initialize machine generated stuff*/
herr_t ss_scope_init(void);
herr_t ss_scope_dest(ss_scope_t *scope);
herr_t ss_scope_dest_(ss_scopeobj_t *scopeobj, unsigned flags);
htri_t ss_scope_boot_top(size_t gfileidx, MPI_Comm filecomm, hbool_t comm_duped);
herr_t ss_scope_boot(ss_scope_t *scope, hid_t gid, MPI_Comm filecomm, hbool_t create);
herr_t ss_scope_open(ss_scope_t *scope, unsigned flags, ss_prop_t *props);
herr_t ss_scope_close(ss_scope_t *scope);
hid_t ss_scope_isopen(ss_scope_t *scope);
htri_t ss_scope_isopentop(ss_scope_t *scope);
htri_t ss_scope_istransient(ss_scope_t *scope);
htri_t ss_scope_iswritable(ss_scope_t *scope);
herr_t ss_scope_synchronize(ss_scope_t *scope, unsigned table, ss_prop_t *props);
htri_t ss_scope_synchronize_cb(size_t itemidx, ss_pers_t *_scope, ss_persobj_t *_scopeobj, void *_sync_data);
htri_t ss_scope_synchronized(ss_scope_t *scope, unsigned table);
herr_t ss_scope_flush(ss_scope_t *scope, unsigned table, ss_prop_t *props);
ss_table_t *ss_scope_table(ss_scope_t *scope, unsigned tableidx, ss_table_t *table);
herr_t ss_scope_comm(ss_scope_t *scope, MPI_Comm *comm/*out*/, int *self/*out*/, int *ntasks/*out*/);
htri_t ss_scope_dest_cb(size_t itemidx, ss_pers_t *_scope, ss_persobj_t *_scopeobj, void *_udata);
htri_t ss_scope_flush_cb(size_t itemidx, ss_pers_t *_scope, ss_persobj_t *_scopeobj, void *_udata);
herr_t ss_scope_setversion(hid_t grp);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSSCOPE_H*/
