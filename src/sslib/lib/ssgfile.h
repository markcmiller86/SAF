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
#ifndef SS_HAVE_SSGFILE_H
#define SS_HAVE_SSGFILE_H

/* The data structure that holds information about all files known to the library. It's essentially a merged version of all
 * known `File' tables that have been read into memory.  Each member of a `File' table has an index into the global file table
 * which holds runtime information that might be shared between `File' objects. Object links also index into the global file
 * table. */
typedef struct ss_gfile_t {
    /* The following are set when a file is booted */
    char        *name;                  /* The normalized, absolute file name. This is either the normalized version of some `File'
                                         * object or the normalized version of the name actually used to open that file. */
    char        *dirname;               /* The directory part of "name" */
    hid_t       fid;                    /* The HDF5 file ID for this file: zero if the file has never been opened, negative if
                                         * there was an error opening the file; one if the file is transient; positive if the
                                         * file is currently open. */
    size_t      serial;                 /* Serial number unique to this entry but shared by all tasks owning this entry */

    /* The following are set when a file is opened */
    size_t      cur_open;               /* Number of explicitly opened ss_file_t objects pointing to this slot (0=file closed). */
    unsigned    flags;                  /* Flags used to open the file */
    size_t      open_serial;            /* Serial number incremented each time this file is reopened. */
    ss_scope_t  *topscope;              /* Top-level scope for this file, null if the file was never opened. This is not freed
                                         * when the file is closed in order that existing persistent links into this file can
                                         * still convert indirect indices into direct indices. */
    ss_gblob_t  *gblob;                 /* Information about all blob datasets in this file */
    hid_t       dxpl_independent;       /* Dataset transfer property list for independent I/O */
    hid_t       dxpl_collective;        /* Dataset transfer property list for collective I/O */
    hid_t       tff[SS_PERS_NCLASSES];  /* The shared versions of ss_pers_class_t->tff */

    /* The following are set by assigning registry scopes */
    ss_scope_t  *reg;                   /* Array of registry scopes */
    size_t      reg_nused;              /* Number of slots used in the `reg' array */
    size_t      reg_nalloc;             /* Number of slots allocated in the `reg' array */
} ss_gfile_t;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Global File Information
 * Purpose:     Obtain global file record by index
 * Description: Given an index return the global file record or a null pointer.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_GFILE_IDX(N) ((N)<sslib_g.gfile.nused ? sslib_g.gfile.ent+(N) : NULL)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Global File Information
 * Purpose:     Obtain global file record for a file
 * Description: Given a file object return the global file record or a null pointer.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_GFILE_FILE(F) ((F) ? SS_GFILE_IDX((F)->m.gfileidx) : NULL)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Global File Information
 * Purpose:     Obtain global file record for a link
 * Description: Given a link to a persistent object return the global file record for the file to which the object belongs.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_GFILE_LINK(LNK) ((LNK) ? SS_GFILE_IDX(ss_pers_link_gfileidx(LNK)) : NULL)

#ifdef __cplusplus
extern "C" {
#endif

herr_t ss_gfile_init(void);
size_t ss_gfile_find_name(const char *name);
size_t ss_gfile_find_serial(size_t serial);
size_t ss_gfile_new(MPI_Comm comm);
herr_t ss_gfile_debug_all(FILE *out);
herr_t ss_gfile_debug_one(size_t idx, FILE *out, const char *prefix);

#ifdef __cplusplus
}
#endif

#endif /* !SS_HAVE_SSGFILE_H */
