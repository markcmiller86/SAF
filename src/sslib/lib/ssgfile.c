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
SS_IF(gfile);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Global File Information
 *
 * Description: This chapter describes data structures and functions related to files that are or were opened by SSlib.  The
 *              main purpose of this layer is to provide a mechanism that allows different /File/ persistent objects to refer
 *              to a common underlying HDF5 file with a name possibly different than that specified in the /File/ object.
 *
 *              The main data structure is an unsorted, task-global array (sslib_g.gfile.ent) that maintains information that
 *              might be shared between /File/ objects. Persistent object links (ss_pers_t) have an index into the so called
 *              "GFile" array and thus in order to determine when a link dangles it is necessary that once an entry is added
 *              to the GFile array that it never be removed, but rather just marked as closed.
 *
 *              Each task has its own version of the GFile array with entries that might be different than entries on other
 *              tasks. This is due to the fact that file create, open, and close operations are collective across a file
 *              communicator which might be a subset of the library communicator. This means, however, that a GFile array
 *              index cannot simply be passed from task to task when transferring object links between tasks as happens during
 *              many synchronization algorithms employed by SSlib. Therefore each GFile entry is also given a unique serial
 *              number that is shared between the tasks of that file's communicator and it is this serial number that is
 *              transmitted.
 *
 *              A secondary purpose of this interface is to allow /File/ objects to be implicitly opened if their name matches
 *              that of some currently open file. Whenever SSlib opens a file (call it the /master/ file) it looks
 *              at the /File/ tables of the master file to discover the names of all subordinate files that might
 *              be referenced by the master file (i.e., all ss_file_t objects except the first one in each table,
 *              which refers to the master file itself). If any of the subordinate names are relative, it temporarily converts
 *              them to absolute names using the master file's directory as the current working directory and uses these
 *              converted names when searching for matching entries in the GFile array. If the subordinate file under
 *              consideration matches a name in the GFile array then the subordinate file will point to that entry, thus
 *              sharing the entry with some other file; otherwise the subordinate file will point to a brand new entry. In any
 *              case, the subordinate file's explicit_open flag will be set to false and the GFile entry's cur_opens
 *              counter will not be incremented.  If the GFile entry is marked as currently open then the subordinate
 *              file is also implicitly open, sharing the same underlying HDF5 file and MPI communicator.  Any implicitly 
 *              opened file can be reopened at any time, either with the same flags as it already shares or with a brand new
 *              name, and once that happens the file is considered to be explicitly opened.  Care should be taken to ensure
 *              that object links weren't already dereferenced through the implicitly opened file, or two or more links that
 *              look identical might not be so (this could actually be checked by SSlib with an appropriate counter).
 *              Only files that are explicitly opened can be explicitly closed. Any implicitly opened files will implicitly
 *              close once all explicitly opened files in common are closed.
 *
 *              The SS_GFILE_IDX() macro converts an array index into an ss_gfile_t pointer. Since the GFile array is
 *              dynamically allocated it might also be reallocated and it's base address change during certain function calls
 *              (any function that adds a record might change the base address). Therefore care should be taken when storing a
 *              GFile pointer in a local variable. In general the library passes around GFile array indices instead of
 *              pointers for this very reason.
 *
 *              The endorsed method of looping over all GFile elements is the following:
 *
 *                  size_t i;
 *                  ss_gfile_t *gfile;
 *                  for (i=0; NULL!=(gfile=SS_GFILE_IDX(i)); i++) {
 *                      // body of loop operating on gfile, but be careful about the
 *                      // GFile array being reallocated since that would make gfile
 *                      // an invalid pointer.
 *                  }
 *
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Global File Information
 * Purpose:     Interface initializer
 *
 * Description: Initializes the global file information interface.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_gfile_init(void)
{
    SS_ENTER_INIT;
    /* nothing to do */
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Global File Information
 * Purpose:     Find GFile entry by name
 *
 * Description: Scans the entire GFile array and returns the first entry having the specified name.
 *
 * Return:      Returns an index into the GFile array on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, January  9, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_gfile_find_name(const char *name)
{
    SS_ENTER(ss_gfile_find_name, size_t);
    size_t              gfileidx=0;

    for (gfileidx=0; gfileidx<sslib_g.gfile.nused; gfileidx++) {
        ss_gfile_t *gfile = sslib_g.gfile.ent + gfileidx;
        if (gfile->name && !strcmp(gfile->name, name)) goto done;
    }
    SS_ERROR_FMT(NOTFOUND, ("name=\"%s\"", name));

done:
SS_CLEANUP:
    SS_LEAVE(gfileidx);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Global File Information
 * Purpose:     Find GFile entry by serial number
 *
 * Description: Scans the entire GFile array and returns the first (only) entry having the specified serial number.
 *
 * Return:      Returns an index into the GFile array on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, February  2, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_gfile_find_serial(size_t serial)
{
    SS_ENTER(ss_gfile_find_serial, size_t);
    size_t              gfileidx=0;

    for (gfileidx=0; gfileidx<sslib_g.gfile.nused; gfileidx++) {
        ss_gfile_t *gfile = sslib_g.gfile.ent + gfileidx;
        if (gfile->serial == serial) goto done;
    }
    SS_ERROR_FMT(NOTFOUND, ("serial=0x%08lx", (unsigned long)serial));

done:
SS_CLEANUP:
    SS_LEAVE(gfileidx);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Global File Information
 * Purpose:     Create a new GFile entry
 *
 * Description: Adds a new entry to the end of the GFile array.
 *
 * Return:      Returns the index into the GFile array for the new entry on success; SS_NOSIZE on failure.
 *
 * Parallel:    Collective over COMM, the MPI communicator that defines which tasks are calling this function to collectively
 *              create a new shared entry in the GFile array.  The only purpose of the collectivity is to be able to assign
 *              a unique serial number for the new GFile array member.
 *
 * Programmer:  Robb Matzke
 *              Friday, January  9, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_gfile_new(MPI_Comm comm)
{
    SS_ENTER(ss_gfile_new, size_t);
    size_t      retval=SS_NOSIZE;

    SS_EXTEND_ZERO(sslib_g.gfile.ent, MAX(64,sslib_g.gfile.nused+1), sslib_g.gfile.nalloc);
    retval = sslib_g.gfile.nused++;
    sslib_g.gfile.ent[retval].serial = ss_mpi_serial(comm);
    SS_ASSERT(SS_NOSIZE!=sslib_g.gfile.ent[retval].serial);

SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Global File Information
 * Purpose:     Print global file table
 *
 * Description: Displays information about all global files
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, April 18, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_gfile_debug_all(FILE *out)
{
    SS_ENTER(ss_gfile_debug_all, herr_t);
    size_t      gfileidx;
    char        prefix[32];

    for (gfileidx=0; SS_GFILE_IDX(gfileidx); gfileidx++) {
        sprintf(prefix, "\001#%-4lu ", (unsigned long)gfileidx);
        ss_gfile_debug_one(gfileidx, out, prefix);
    }

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Global File Information
 * Purpose:     Print information about a global file entry
 *
 * Description: Prints information about a single entry in the global file array.
 *
 * Return:      Returns non-negative on success; negative on failure
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, April 18, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_gfile_debug_one(size_t idx, FILE *out, const char *prefix)
{
    SS_ENTER(ss_gfile_debug_one, herr_t);
    int         self, fldsize=16;
    unsigned    flags;
    char        intro[32], white_space[128];
    ss_gfile_t  *gfile=NULL;
    hbool_t     first_only=FALSE;

    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    sprintf(intro, "SSlib-%d: ", self);
    if (!prefix) prefix = "";
    if ('\001'==*prefix) {
        first_only = TRUE;
        prefix++;
    }
    
    /* The gfile general info */
    gfile = SS_GFILE_IDX(idx);
    fprintf(out, "%s%s%-*s", intro, prefix, fldsize, "global file:");
    fprintf(out, " direct(%lu) 0x%08lx ", (unsigned long)idx, (unsigned long)gfile);
    if (!gfile) {
        fprintf(out, "invalid\n");
        goto done;
    }
    fprintf(out, "serial(0x%08lx)\n", (unsigned long)(gfile->serial));

    /* Replace prefix with all white space if appropriate now that we displayed the first line */
    if (first_only) {
        assert(strlen(prefix)<sizeof white_space);
        memset(white_space, ' ', strlen(prefix));
        white_space[strlen(prefix)] = '\0';
        prefix = white_space;
    }
        
    /* The destination file */
    fprintf(out, "%s%s%-*s \"%s\"\n", intro, prefix, fldsize, "file name:", gfile->name);
    fprintf(out, "%s%s%-*s", intro, prefix, fldsize, "file status:");
    if (gfile->cur_open<=0) {
        fprintf(out, " closed\n");
        goto done;
    }
    fprintf(out, " open(%lu)", (unsigned long)(gfile->cur_open));
    fprintf(out, " flags(");
    flags = gfile->flags;
    fprintf(out, "%s", (flags & H5F_ACC_RDWR) ? "RDWR" : "RDONLY");
    flags &= ~H5F_ACC_RDWR;
    if (flags & H5F_ACC_TRANSIENT) {
        fprintf(out, "|TRANSIENT");
        flags &= ~H5F_ACC_TRANSIENT;
    }
    if (flags & H5F_ACC_TRUNC) {
        fprintf(out, "|TRUNC");
        flags &= ~H5F_ACC_TRUNC;
    }
    if (flags & H5F_ACC_EXCL) {
        fprintf(out, "|EXCL");
        flags &= ~H5F_ACC_EXCL;
    }
    if (flags & H5F_ACC_CREAT) {
        fprintf(out, "|CREAT");
        flags &= ~H5F_ACC_CREAT;
    }
    if (flags & H5F_ACC_DEBUG) {
        fprintf(out, "|DEBUG");
        flags &= ~H5F_ACC_DEBUG;
    }
    if (flags) {
        fprintf(out, "|0x%08lx", (unsigned long)flags);
    }
    fprintf(out, ")");
    if (0==gfile->fid) fprintf(out, " hdf5(closed)\n");
    else if (1==gfile->fid) fprintf(out, " hdf5(transient)\n");
    else if (gfile->fid<0) fprintf(out, " hdf5(error)\n");
    else fprintf(out, " hdf5(%lu)\n", (unsigned long)(gfile->fid));

done:
    SS_LEAVE(0);
}
