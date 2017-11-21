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
#ifndef SS_HAVE_SSFILE_H
#define SS_HAVE_SSFILE_H

/* The transient information about a file. Information about the open file is stored in the GFile array in sslib_g since multiple
 * `File' objects might share a common underlying HDF5 file (for instance, when two SSlib files both make references to a
 * third file the third file will have a corresponding `File' object in each of the two files and also as the first entry in
 * its own `File' table. */
typedef struct ss_fileobj_tm {
    ss_persobj_t        pers;           /* Inherits from this. This member must be first! */
    size_t              gfileidx;       /* Index into the global file table */
    hbool_t             explicit_open;  /* True if and only if this ss_file_t object was used to explicitly open the file */
} ss_fileobj_tm;

typedef struct ss_fileobj_tf {          /* Memory-resident persistent information */
    ss_string_t         name;           /* Name of the file (absolute or relative w.r.t. file containing this table), or NULL */
} ss_fileobj_tf;

typedef struct {
    ss_file_t           file;                   /* The file to be opened */
    char                *newname;               /* Optional name to override the one stored in the file object. This is
                                                 * allocated by the library so the caller should reallocate it when extending
                                                 * the name. */
} ss_file_ref_t;

/* Types generated from above declarations */
#include "ssfiletab.h"  /*GENERATED*/

#ifdef __cplusplus
extern "C" {
#endif

herr_t ss_filetab_init(void); /*to initialize machine generated stuff*/
herr_t ss_file_init(void);
ss_file_t *ss_file_open(ss_file_t *file, const char *name, unsigned flags, ss_prop_t *props);
ss_file_t *ss_file_create(const char *name, unsigned flags, ss_prop_t *props);
hid_t ss_file_isopen(ss_file_t *file, const char *name);
htri_t ss_file_istransient(ss_file_t *file);
htri_t ss_file_iswritable(ss_file_t *file);
herr_t ss_file_readonly(ss_file_t *file);
herr_t ss_file_synchronize(ss_file_t *file, ss_prop_t *props);
htri_t ss_file_synchronized(ss_file_t *file);
herr_t ss_file_flush(ss_file_t *file, ss_prop_t *props);
herr_t ss_file_close(ss_file_t *file);
htri_t ss_file_close1_cb(size_t itemidx, ss_pers_t *pers, ss_persobj_t *persobj, void *udata);
htri_t ss_file_close2_cb(size_t itemidx, ss_pers_t *pers, ss_persobj_t *persobj, void *udata);
herr_t ss_file_closeall(FILE *verbose);
herr_t ss_file_registry(ss_file_t *file, ss_scope_t *registry);
char *ss_file_fixname(const char *name, const char *wrt, const char *relative, size_t len, char *buffer);
ss_scope_t *ss_file_topscope(ss_file_t *file, ss_scope_t *buf);
ss_file_ref_t *ss_file_references(ss_file_t *master, size_t *nfiles, ss_file_ref_t *fileref, ss_prop_t *props);
herr_t ss_file_openall(size_t nfiles, ss_file_ref_t *fileref, unsigned flags, ss_prop_t *props);

#ifdef __cplusplus
}
#endif

#endif /* !SS_HAVE_SSFILE_H */
