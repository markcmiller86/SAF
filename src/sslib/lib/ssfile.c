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
SS_IF(file);

static htri_t ss_file_boot1_cb(size_t itemidx, ss_pers_t *pers, ss_persobj_t *persobj, void *udata);
static htri_t ss_file_boot2_cb(size_t itemidx, ss_pers_t *pers, ss_persobj_t *persobj, void *udata);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 *
 * Description: An SSlib file is an HDF5 file with a certain minimum internal structure: all SSlib files contain a group named
 *              "/SAF" which serves as the top-level scope for the file (there may be additional groups that also implement
 *              other scopes). The top-level scope is always opened with the file's communicator and has a /Scope/ table in
 *              addition to the usual tables. The /Scope/ table is a list of all scopes contained in the file and the first
 *              element of that table is the top-level scope itself.
 *
 *              Every scope also has a /File/ table that lists all files referred to by objects stored in that scope. The first
 *              element of every /File/ table is understood to be the file containing that table.
 *
 *              Files are always opened with the ss_file_open() function (or the convenience ss_file_create()) and closed with
 *              ss_file_close(). The file-related functions operate on or return a link of type ss_file_t, which points to
 *              some entry in a top-scope's /File/ table.
 *
 *              In addition to the collection of /File/ tables in all scopes of all files that are currently
 *              open, the library maintains a per-task list of files and a mapping from the /File/ table members to
 *              this global list. The mapping from a /File/ table member to the gfile array is accomplished with the
 *              ss_file_open() function. This allows the following:
 *
 *              * Members from multiple /File/ tables can point to a common underlying HDF5 file,
 *              * The file whose name was originally recorded in the /File/ table can be temporarily renamed,
 *              * Newly discovered file objects can be implicitly opened if they match a previous open file.
 *
 *              The implicit opening needs more discussion: whenever SSlib opens a file (call it the /master/ file) it looks
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
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Note:        File Properties
 * Description:
 *              * /comm/: The MPI communicator for the file.  The default is to use the library's communicator.
 *
 *              * /info/: The MPI info object for the file.  The default is to pass MPI_INFO_NULL.
 *
 *              * /fapl/: The HDF5 file access property list to use when creating or opening a file. At a minimum, the caller
 *              should specify the HDF5 virtual file driver that should be used.  The default is to use either the !sec2
 *              driver or the !mpiposix driver, depending on whether the file communicator has one MPI task or more than one.
 *              However, if the use_mpiio property is set then the !mpiio virtual file driver is used by default when the file
 *              communicator has more than one MPI task.
 *
 *              * /fcpl/: The HDF5 file creation property list to be used when creating a new file.
 *
 *              * /hid/: The HDF5 file handle to use instead of calling H5Fopen() or H5Fcreate().  The file handle is not
 *              duplicated by ss_file_open() and therefore the caller should not call H5Fclose() but rather relinquish control
 *              of that file to SSlib, eventually closing the file with ss_file_close(). The !fapl and !fcpl values are not
 *              used if the file is already opened.
 *
 *              * /use_mpiio/: If no file access property is supplied and SSlib was compiled with parallel support
 *              and the file communicator has more than one MPI task, then the !mpiio virtual file driver (if supported)
 *              will be used to access the file.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Files
 * Purpose:     Interface initializer
 *
 * Description: Initializes the file interface.
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
ss_file_init(void)
{
    SS_ENTER_INIT;

    if (ss_pers_init()<0) SS_ERROR(INIT);       /* ss_file_t et al need object link types */
    if (ss_filetab_init()<0) SS_ERROR(INIT);    /* initialize ss_file_t related datatypes */

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Open or create a file
 *
 * Description: Explititly opens an SSlib file and returns a link to the ss_file_t object for that file. Either FILE or NAME
 *              or both may be specified. If one or the other (but not both) is specified then the file is simply opened with
 *              the name contained in the file object or the specified name.  If both are specified then a mapping from the
 *              FILE object to the specified NAME is established, which is necessary if the name originally recorded in the
 *              /Files/ table is no longer valid due to the file being moved in the file system.
 *
 *              Depending on FLAGS, the file might be created if it doesn't exist (H5_ACC_CREATE) or truncated if it does exist
 *              (H5_ACC_TRUNC). If the file is truncated, files which were mentioned in its /File/ table are not
 *              automatically deleted or truncated and other files which link to this truncated file will subsequently contain
 *              dangling or invalid links.
 *
 *              SSlib supports transient objects by placing them in transient files. A transient file is simply a special
 *              SSlib file that doesn't correspond to any underlying storage (not even an HDF5 file using the !core virtual
 *              file driver).  Such files support more or less the same SSlib operations as real files although some
 *              operations may be tuned for this special case (e.g., ss_file_flush()).  Transient files are always created for
 *              read and write access as are the scopes they contain, and are denoted as such by the the bit H5F_ACC_TRANSIENT
 *              in the FLAGS argument. They share the same name space as their permanent cousins, and thus it is not possible
 *              to have a transient and permanent file both named "foo.saf" although creating a transient file doesn't affect
 *              any file that might already exist by that name.
 *              
 *              All /File/ tables of the file that is newly opened are scanned and all of their members are added to the
 *              GFile array. If that array already had the name marked as open then the corresponding entry of the /File/
 *              table will be implicitly opened. Files opened implicitly need not be closed and can be opened explicitly at
 *              any time (although if one is going to open it explicitly it's best to do so early on).
 *
 *              The ss_file_create() function is a convenience for creating a new file.
 *
 * Return:      Returns a link to the ss_file_t object for the newly opened file on success; null on failure.  If the FILE
 *              argument is supplied then this will be the successful return value.
 *
 * Parallel:    Collective across the file's communicator as specified by PROPS.comm, defaulting to the same thing as
 *              the library's communicator.
 *
 * Issues:      HDF5 doesn't yet (1.6.0) support the H5F_ACC_TRANSIENT bit, which would essentially make all operations on the
 *              file no-ops. Therefore this functionality must be supported in SSlib.
 *
 * Programmer:  Robb Matzke
 *              Monday, May 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_file_t *
ss_file_open(ss_file_t *file,           /* Optional handle to a persistent file object from a /File/ table. */
             const char *name,          /* Optional name of file to be opened. */
             unsigned flags,            /* HDF5-style file access flags. */
             ss_prop_t *props           /* Optional file property list (see File Properties). */
             )
{
    SS_ENTER(ss_file_open, ss_file_tP);
    size_t              gfileidx;               /* Index into the GFile array */
    ss_gfile_t          *gfile=NULL;            /* An unused slot from the GFile array */
    char                *fixedname=NULL;        /* Normalized absolute name */
    hid_t               fapl_created=-1, fcpl_created=-1, fapl=-1, fcpl=-1;
    MPI_Comm            comm, comm_temp;
    hbool_t             comm_duped=FALSE;
    MPI_Info            info=MPI_INFO_NULL;
    htri_t              creating=FALSE;         /* Did we create a new file? */
    ss_table_t          *scopetab=NULL;
    ss_table_t          *filetab=NULL;
    ss_file_t           *retval=NULL;
    ss_fileobj_t        *fileobj;
    ss_gfile_t          orig_gfile;             /* Original value of the gfile slot to restore during error recovery*/
    size_t              udata[2];               /* Info to pass through to ss_file_boot2_cb() */
    ss_prop_t           *scope_props=NULL;      /* Properties for opening the top scope */
    ss_file_t           *file_here=NULL;        /* File object to destroy on error */

    memset(&orig_gfile, 0, sizeof orig_gfile);

    if (file && name) {
        /* Bind the name to the file. In other words, the file's entry in the file table contains one name but the caller
         * wants to use some other name instead. Perhaps the original file has been moved. */
        if (ss_file_isopen(file, NULL)<0) SS_ERROR(FAILED);
        if (NULL==(fixedname=ss_file_fixname(name, NULL, NULL, 0, NULL))) SS_ERROR(FAILED);
        if (SS_NOSIZE!=(gfileidx=ss_gfile_find_name(fixedname))) {
            gfile = SS_GFILE_IDX(gfileidx);
            if (gfile->cur_open>0) {
                /* The global file by this name is already open. We can just make the FILE point to this gfile entry as
                 * an implicitly opened file. */
                SS_FILE(file)->m.gfileidx = gfileidx;
                if (NULL==(retval = ss_pers_file((ss_pers_t*)(gfile->topscope), NULL))) SS_ERROR(FAILED);
                goto done;
            }
        }
        SS_STATUS_OK; /*clean up ss_gfile_find_name() failure*/
        if (NULL==(retval=ss_file_open(NULL, name, flags, props))) SS_ERROR(FAILED);
        
    } else if (file) {
        /* Open file with the name specified in SS_FILE(file) */
        if (NULL==(name=ss_string_ptr(SS_FILE_P(file,name)))) SS_ERROR(FAILED);
        if (NULL==(retval=ss_file_open(file, name, flags, props))) SS_ERROR(FAILED);
        
    } else {
        /* Obtain the MPI communicator.  If the caller is passing in a communicator then we should dup it so operations
         * performed by SSlib on that communicator don't interfere with the caller. HDF5 will also always dup the communicator
         * when the file is opened/created. */
        if (props && ss_prop_get(props, "comm", H5T_NATIVE_MPI_COMM, &comm_temp)) {
            if (ss_mpi_comm_dup(comm_temp, &comm)<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
            comm_duped = TRUE;
        } else {
            SS_STATUS_OK; /*for possible failed ss_prop_get()*/
            comm = sslib_g.comm;
        }
        if (!props || NULL==ss_prop_get(props, "info", H5T_NATIVE_MPI_INFO, &info)) {
            SS_STATUS_OK; /*we don't care if ss_prop_get() failed*/
            info = MPI_INFO_NULL;
        }
        
        /* We want to open a file with the specified name. First scan through the gfile array to see if there's already an
         * entry for this name. If so, the named file is either open already or the name is referenced from the /File/ table
         * of one or more open files. Create an entry if we don't find one existing. */
        if (NULL==(fixedname = ss_file_fixname(name, NULL, NULL, 0, NULL))) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        if (SS_NOSIZE==(gfileidx=ss_gfile_find_name(fixedname))) {
            SS_STATUS_OK;
            if (SS_NOSIZE==(gfileidx = ss_gfile_new(comm))) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
            gfile = SS_GFILE_IDX(gfileidx);
            orig_gfile = *gfile;
            if (NULL==(gfile->name=malloc(strlen(fixedname)+1))) SS_ERROR_FMT(RESOURCE, ("file=\"%s\"", name));
            strcpy(gfile->name, fixedname);
            if (NULL==(gfile->dirname=malloc(strlen(fixedname)+1))) SS_ERROR_FMT(RESOURCE, ("file=\"%s\"", name));
            strcpy(gfile->dirname, gfile->name);

#ifdef WIN32
            {
                char *fwd_slash = strrchr(gfile->dirname, '/');
                char *back_slash = strrchr(gfile->dirname, '\\');
                SS_ASSERT(fwd_slash || back_slash);
                if(fwd_slash && !back_slash) *fwd_slash='\0';
                else if(!fwd_slash && back_slash) *back_slash='\0';
                else if(fwd_slash > back_slash) *fwd_slash='\0';
                else *back_slash='\0';
            }
#else
            SS_ASSERT(strrchr(gfile->dirname, '/'));
            *(strrchr(gfile->dirname, '/')) = '\0';
#endif

        } else {
            gfile = SS_GFILE_IDX(gfileidx);
            orig_gfile = *gfile;
        }
        
        /* If the gfile entry is an open file then we have an error. It should not be possible to open the same file more than
         * once concurrently. Increment the file-open counter early because some functions below (e.g., ss_pers_deref()) will
         * need to see that the file is at least in the process of being opened. */
        if (gfile->cur_open>0) SS_ERROR_FMT(PERM, ("file already open: %s", fixedname));
        gfile->cur_open++;
        gfile->open_serial++;

        /* Create file access and creation properties or use the ones supplied in PROPS. If we create them here we'll have to
         * close them below. */
        if (!props || NULL==ss_prop_get(props, "fapl", H5T_NATIVE_HID, &fapl) || fapl<=0) {
            SS_STATUS_OK; /*possible failed ss_prop_get()*/
            if ((fapl=fapl_created=H5Pcreate(H5P_FILE_ACCESS))<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
        }
        if (!props || NULL==ss_prop_get(props, "fcpl", H5T_NATIVE_HID, &fcpl) || fcpl<=0) {
            SS_STATUS_OK; /*possible failed ss_prop_get()*/
            if ((fcpl=fcpl_created=H5Pcreate(H5P_FILE_CREATE))<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
        }

        /* Create data transfer properties -- we modify them below if necessary */
        if (gfile->dxpl_independent<=0) gfile->dxpl_independent = H5Pcreate(H5P_DATASET_XFER);
        if (gfile->dxpl_collective<=0) gfile->dxpl_collective = H5Pcreate(H5P_DATASET_XFER);
        gfile->flags = flags;
        
        /* If neither fcpl nor fapl were supplied then set various things now in the ones we created. */
        if (fapl_created>=0 && fcpl_created>=0) {
#if H5_VERS_NUMBER>=1005000
            /* The following property causes HDF5 to close all objects related to the file when the file is closed. This is
             * important for the MPI file driver to prevent HDF5 from making MPI calls after MPI_Finalize(), but it doesn't
             * hurt to do the same for the other drivers. In fact, doing this for the serial driver will help us detect errors
             * in SSlib if it tries to use an object after the object's file has been closed because HDF5 will report an invalid
             * object ID. */
            H5Pset_fclose_degree(fapl, H5F_CLOSE_STRONG);
#endif
#ifdef HAVE_PARALLEL
            if (1==ss_mpi_comm_size(comm)) {
                /* Set the VFD to sec2 */
                if (H5Pset_fapl_sec2(fapl)<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
            } else {
                /* Set the VFD to mpio. If we choose to use the mpiposix driver instead we'll reset it below. */
                if (H5Pset_fapl_mpio(fapl, comm, MPI_INFO_NULL)<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
                if (H5Pset_dxpl_mpio(gfile->dxpl_independent, H5FD_MPIO_INDEPENDENT)<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
                if (H5Pset_dxpl_mpio(gfile->dxpl_collective, H5FD_MPIO_COLLECTIVE)<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
            
                /* GPFS on LLNL's SP systems grants all file tokens to the first process that writes at file offset zero,
                 * resulting in extra overhead when other processes need to write to parts of the file. Therefore, we tell HDF5
                 * to avoid ever writing into the first GPFS block of the file. Also, by using HDF5's MPI/POSIX driver we can
                 * bypass MPI-IO, but be warned that this is only expected to work if all tasks are on a single node or the
                 * files are on a parallel file system. */
#if 0
                if (!props || ss_prop_get_i(props, "use_mpiio")<=0) {
                    SS_STATUS_OK; /*possible failed ss_prop_get_i() has same meaning as use_mpiio==false*/
                    if (H5Pset_fapl_mpiposix(fapl, comm, TRUE)<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
                    if (H5Pset_userblock(fcpl, (hsize_t)512*1024)<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
                    if (H5Pset_alignment(fapl, (hsize_t)64*1024, (hsize_t)512*1024)<0) SS_ERROR_FMT(HDF5, ("file=\"%s\"", name));
                }
#endif
            }
#endif /*HAVE_PARALLEL*/
        }
        
        /* Create or open the file. Use the supplied name just in case ss_file_fixname() has some problem. This would be much
         * easier if H5Fopen() worked more like unix open(), being able to create new files, truncate existing files, etc. */
        if (flags & H5F_ACC_TRANSIENT) {
            gfile->fid = 1; /* non-negative indicates no error, but not a valid hdf5 file handle*/
        } else if (flags & (H5F_ACC_TRUNC | H5F_ACC_EXCL)) {
            unsigned tempflags = flags & ~(H5F_ACC_RDWR|H5F_ACC_CREAT); /* H5Fcreate() is picky about these: they must be off */
            if ((gfile->fid=H5Fcreate(name, tempflags, fcpl, fapl))<0)
                SS_ERROR_FMT(HDF5, ("unable to create file: %s", fixedname));
        } else {
            if ((gfile->fid=H5Fopen(name, flags, fapl))<0) SS_ERROR_FMT(HDF5, ("unable to open file: %s", fixedname));
        }

        /* Create or read the basic file structure. */
        if ((creating=ss_scope_boot_top(gfileidx, comm, comm_duped))<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        gfile = SS_GFILE_IDX(gfileidx); /*might have been clobbered by previous call*/

        /* Create or read the basic blob storage information */
        if (NULL==(gfile->gblob = ss_blob_boot(gfile->topscope, (hbool_t)creating))) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));

        /* If we're creating the file then the first entry of the top-scope's File table must be the file itself. This is an
         * appropriate place to call this function because the communicator for the top scope is the same as for the file,
         * which is the communicator over which ss_file_open() was called. */
        if (NULL==(filetab=ss_scope_table(gfile->topscope, SS_MAGIC(ss_file_t), NULL))) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        if (creating) {
            if (NULL==(file = file_here = SS_PERS_NEW(gfile->topscope, ss_file_t, SS_ALLSAME)))
                SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
            if (ss_string_set(SS_FILE_P(file,name), name)<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        } else {
            if (ss_table_read(filetab, gfile->topscope)<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
            if (NULL==(fileobj=(ss_fileobj_t*)ss_table_lookup(filetab, 0, SS_STRICT)))
                SS_ERROR_FMT(CORRUPT, ("top scope's file table is empty"));
            if (NULL==(file=file_here=(ss_file_t*)ss_pers_refer(gfile->topscope, (ss_persobj_t*)fileobj, NULL)))
                SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        }

        /* Process all entries of all File tables by adding records to GFile array. Table slot zero is already in
         * the GFile array at location gfileidx. */
        udata[0] = gfileidx;
        udata[1] = comm;
        if (NULL==(scopetab=ss_scope_table(gfile->topscope, SS_MAGIC(ss_scope_t), NULL)))
            SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        if (ss_table_scan(scopetab, gfile->topscope, 0, ss_file_boot1_cb, udata)<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        gfile = SS_GFILE_IDX(gfileidx); /*may have been clobbered by previous call*/

        /* Open this scope */
        if (NULL==(scope_props=ss_prop_new("scope open props"))) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        if (ss_prop_add(scope_props, "comm", H5T_NATIVE_MPI_COMM, &comm)<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        if (ss_prop_add(scope_props, "duped", H5T_NATIVE_HBOOL, &comm_duped)<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        if (ss_scope_open(gfile->topscope, gfile->flags, scope_props)<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        if (ss_prop_dest(scope_props)<0) SS_ERROR_FMT(FAILED, ("file=\"%s\"", name));
        scope_props = NULL;

        /* Mark file as explicitly opened (gfile.cur_open was incremented above) */
        SS_FILE(file)->m.explicit_open = TRUE;
        retval = file;
        file = NULL;
    }

done:
    /* successful cleanup */
    SS_FREE(fixedname);
    if (fapl_created>0) H5Pclose(fapl_created);
    if (fcpl_created>0) H5Pclose(fcpl_created);
    
SS_CLEANUP:
    SS_FREE(fixedname);

    /* Destroy the `file' return value */
    if (file_here) {
        ss_pers_dest((ss_pers_t*)file_here);
        file_here = SS_FREE(file_here);
    }
    
    /* Restore the `gfile' entry to the original value */
    if (gfile) {
        if (gfile->name!=orig_gfile.name) SS_FREE(gfile->name);
        if (gfile->fid>0 && gfile->fid!=orig_gfile.fid) H5Fclose(gfile->fid);
        if (gfile->topscope && gfile->topscope!=orig_gfile.topscope) ss_scope_dest(gfile->topscope);
        SS_ASSERT(NULL==gfile->gblob || NULL==orig_gfile.gblob || gfile->gblob==orig_gfile.gblob);
        if (gfile->dxpl_independent>0 && gfile->dxpl_independent!=orig_gfile.dxpl_independent) H5Pclose(gfile->dxpl_independent);
        if (gfile->dxpl_collective>0 && gfile->dxpl_collective!=orig_gfile.dxpl_collective) H5Pclose(gfile->dxpl_collective);
        SS_ASSERT(NULL==gfile->reg || gfile->reg==orig_gfile.reg);
        *gfile = orig_gfile;
    }

    if (scope_props) ss_prop_dest(scope_props);
    if (fapl_created>0) H5Pclose(fapl_created);
    if (fcpl_created>0) H5Pclose(fcpl_created);
    if (comm_duped) ss_mpi_comm_free(&comm);

    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Obtain information about referenced files
 *
 * Description: A SAF database can consist of many files which reference each other.  This function will return information
 *              about all such files that might be referenced by the MASTER file. The caller is expected to fill in certain
 *              members of the returned array and then use that array to call ss_file_openall().
 *
 * Return:      On success, returns either FILEREF (if non-null) or an allocated array and the NFILES argument indicates how
 *              many elements of the return value have been initialized.  Returns the null pointer on failure.
 *
 * Parallel:    Independent.
 *
 * Programmer:  Robb Matzke
 *              Monday, July 19, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_file_ref_t *
ss_file_references(ss_file_t *master,           /* The file in question */
                   size_t *nfiles,              /* INOUT: Upon return this argument will point to the number of valid entries
                                                 * in the return value.  If FILEREF is non-null then the input value of this
                                                 * pointer specifies the maximum number of file reference entries to
                                                 * initialize in FILEREF and if there are more than that many that are found
                                                 * in the MASTER file then an error is raised. */
                   ss_file_ref_t *fileref,      /* Optional pointer to an array of file reference information that will be
                                                 * initialized by this function and returned (if non-null) as the successful
                                                 * return value of this function. */
                   ss_prop_t UNUSED *props      /* File properties (none defined yet) */
                   )
{
    SS_ENTER(ss_file_references, ss_file_ref_tP);
    ss_scope_t          topscope;
    ss_scope_t          scope;
    unsigned            scope_idx, file_idx;
    size_t              nused=0, nalloc=0;
    ss_gfile_t          *gfile=NULL;
    ss_file_ref_t       *retval=fileref;
    ss_file_t           file;
    char                *pathname=NULL;
    const char          *filename=NULL;

    SS_ASSERT(nfiles);
    if (!ss_file_isopen(master, NULL)) SS_ERROR_FMT(PERM, ("file is not open"));
    if (NULL==ss_file_topscope(master, &topscope)) SS_ERROR(FAILED);
    if (NULL==(gfile=SS_GFILE_LINK(master))) SS_ERROR(FAILED);

    /* The "pathname" is the directory part of the normalized, absolute master name. */
#ifdef WIN32
    /*handle windows case where absolute path is like "c:\" or "d://"*/
    SS_ASSERT((gfile->name[0]=='/' && gfile->name[1]) ||
              (strlen(gfile->name)>=3 && gfile->name[1]==':' && (gfile->name[2]=='/' || gfile->name[2]=='\\')));
#else
    SS_ASSERT(gfile->name[0]=='/' && gfile->name[1]);
#endif
    if (NULL==(pathname=malloc(strlen(gfile->name)+1))) SS_ERROR(RESOURCE);
    strcpy(pathname, gfile->name);
#ifdef WIN32
    {
        char *fwd_slash = strrchr(pathname, '/');
        char *back_slash = strrchr(pathname, '\\');
        SS_ASSERT(fwd_slash || back_slash);
        if(fwd_slash && !back_slash) *fwd_slash='\0';
        else if(!fwd_slash && back_slash) *back_slash='\0';
        else if(fwd_slash > back_slash) *fwd_slash='\0';
        else *back_slash='\0';
    }
#else
    SS_ASSERT(strrchr(pathname, '/'));
    *(strrchr(pathname, '/')) = '\0';
#endif
    
    /* For each scope in the file */
    for (scope_idx=0; /*void*/; scope_idx++) {
        if (NULL==ss_pers_refer_c(&topscope, SS_MAGIC(ss_scope_t), scope_idx, (ss_pers_t*)&scope)) SS_ERROR(FAILED);
        if (NULL==SS_SCOPE(&scope)) break;

        /* For each file in that scope except the first one (which is the MASTER) */
        for (file_idx=1; /*void*/; file_idx++) {
            if (NULL==ss_pers_refer_c(&scope, SS_MAGIC(ss_file_t), file_idx, (ss_pers_t*)&file)) SS_ERROR(FAILED);
            if (NULL==SS_FILE(&file)) break;

            /* Extend the return value array if necessary. */
            if (!fileref) {
                SS_EXTEND(retval, MAX(32,nused+1), nalloc);
            } else if (nused>=*nfiles) {
                SS_ERROR(OVERFLOW);
            }
            
            /* The name of the file is either absolute or relative to the master file. If relative then we need to make it
             * relative to the current working directory. */
            filename = ss_string_ptr(SS_FILE_P(&file,name));
            if ('/'==filename[0]) {
                if (NULL==(retval[nused].newname=malloc(strlen(filename)+1))) SS_ERROR(RESOURCE);
                strcpy(retval[nused].newname, filename);
            } else {
                if (NULL==(retval[nused].newname=ss_file_fixname(ss_string_ptr(SS_FILE_P(&file,name)), pathname, ".", 0, NULL)))
                    SS_ERROR(RESOURCE);
            }
            
            /* Save other stuff */
            retval[nused].file = file;
            nused++;
        }
    }
    SS_FREE(pathname);

    *nfiles=nused;
    if (0==nused && !retval && NULL==(retval=calloc(1, sizeof(*retval)))) SS_ERROR(RESOURCE);
    
SS_CLEANUP:
    if (!fileref) SS_FREE(retval);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Open many files
 *
 * Description: This function opens all files specified in the arguments.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the union of communicators specified in the FILEREF array.
 *
 * Programmer:  Robb Matzke
 *              Monday, July 19, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_file_openall(size_t nfiles,                  /* Number of entries in the FILEREF array. */
                ss_file_ref_t *fileref,         /* Array of information for files to be opened. */
                unsigned flags,                 /* Flags to control how files are opened. */
                ss_prop_t *props                /* Additional file opening properties (see File Properties). */
                )
{
    SS_ENTER(ss_file_openall, herr_t);
    size_t              i;
    ss_file_t *file;

    for (i=0; i<nfiles; i++) {
        file = ss_file_open(&(fileref[i].file), fileref[i].newname, flags, props);
        if (NULL==file)
            SS_ERROR_FMT(FAILED, ("fileref %lu with name `%s'", (unsigned long)i, fileref[i].newname));
        SS_FREE(file);
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Files
 * Purpose:     Scan the File table of every scope
 *
 * Description: This function is called once per scope in a newly opened file. It traverses the /File/ table of each scope
 *              in order to perform boot-related work.
 *
 * Return:      Returns zero on success; negative on failure.
 *
 * Parallel:    Collective over the communicator for the file being opened up in ss_file_open(), the same file which contains
 *              the Scope object PERSOBJ.
 *
 * Also:        ss_file_boot2_cb()
 *
 * Programmer:  Robb Matzke
 *              Monday, January  5, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
static htri_t
ss_file_boot1_cb(size_t UNUSED itemidx, ss_pers_t *pers, ss_persobj_t UNUSED *persobj, void *udata)
{
    SS_ENTER(ss_file_boot1_cb, htri_t);
    ss_scope_t          *scope = (ss_scope_t*)pers;
    ss_table_t          *filetab = ss_scope_table(scope, SS_MAGIC(ss_file_t), NULL);

    if (!filetab) SS_ERROR(FAILED);
    if (ss_table_scan(filetab, scope, 0, ss_file_boot2_cb, udata)<0) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Files
 * Purpose:     Adds a file to the GFile array
 *
 * Description: This function is called once per file in the ss_file_t table of a file that has just recently been opened. It
 *              adds records to the GFile array for each file in the list based on the fully qualified name stored in the
 *              file record.
 *
 * Return:      Returns zero on success; negative on failure.
 *
 * Parallel:    Collective over the communicator for the file being opened up in ss_file_open(), the same file which contains
 *              the File object PERSOBJ.
 *
 * Programmer:  Robb Matzke
 *              Sunday, December 14, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
static htri_t
ss_file_boot2_cb(size_t itemidx,
                 ss_pers_t UNUSED *pers,
                 ss_persobj_t *persobj,
                 void *udata                    /* Array of size_t elements where the first is the GFile index for the file
                                                 * being opened and the second is the MPI communicator for that file. */
                 )
{
    SS_ENTER(ss_file_boot2_cb, htri_t);
    ss_fileobj_t        *fileobj = (ss_fileobj_t*)persobj;
    size_t              top_gfileidx = ((size_t*)udata)[0]; /* the gfile index for the file containing PERSOBJ */
    size_t              gfileidx;                       /* the gfile index of PERSOBJ */
    MPI_Comm            comm = ((size_t*)udata)[1];     /* the communicator for the file containing PERSOBJ */
    char                *fixedname=NULL;
    ss_gfile_t          *gfile=NULL, *top_gfile=NULL;

    if (0==itemidx) {
        /* The first item in the table must point to the file just opened. There should already be an entry in the
         * GFile array for this file. */
        SS_ASSERT(SS_NOSIZE!=top_gfileidx);
        fileobj->m.gfileidx = top_gfileidx;
    } else {
        /* The name in a file object is either absolute or relative w.r.t. the file containing that file object. */
        top_gfile = SS_GFILE_IDX(top_gfileidx);
        if (NULL==(fixedname=ss_file_fixname(ss_string_ptr(&(fileobj->name)), top_gfile->dirname, NULL, 0, NULL)))
            SS_ERROR(FAILED);

        /* Make this item point into the GFile array. */
        if (SS_NOSIZE==(gfileidx=ss_gfile_find_name(fixedname))) {
            if (SS_NOSIZE==(gfileidx=ss_gfile_new(comm))) SS_ERROR(FAILED);
            gfile = SS_GFILE_IDX(gfileidx);
            gfile->name = fixedname;
            if (NULL==(gfile->dirname = malloc(strlen(fixedname)+1))) SS_ERROR(RESOURCE);
            strcpy(gfile->dirname, fixedname);
            SS_ASSERT(strrchr(gfile->dirname, '/'));
            *(strrchr(gfile->dirname, '/')) = '\0';
            fixedname = NULL;
        }
        fileobj->m.gfileidx = gfileidx;
    }

    fixedname = SS_FREE(fixedname);
SS_CLEANUP:
    SS_FREE(fixedname);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Create a new file
 *
 * Description: Creates and initializes an SSlib file. This is actually just a convenience function for calling ss_file_open()
 *              with a FLAGS argument of H5F_ACC_RDWR | H5F_ACC_TRUNC | H5F_ACC_CREAT in addition to those bit flags passed
 *              into this function.
 *
 * Return:      Same as ss_file_open().
 *
 * Parallel:    Same as ss_file_open().
 *
 * Programmer:  Robb Matzke
 *              Monday, May 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_file_t *
ss_file_create(const char *name,        /* Name of file to be created. */
               unsigned flags,          /* HDF5-style file access flags (see ss_file_open()). */
               ss_prop_t *props         /* Optional file property list (see File Properties). */
               )
{
    SS_ENTER(ss_file_create, ss_file_tP);
    ss_file_t *file = NULL;
    
    flags |= H5F_ACC_RDWR | H5F_ACC_TRUNC | H5F_ACC_CREAT;
    if (NULL==(file=ss_file_open(NULL, name, flags, props))) SS_ERROR(FAILED);

 SS_CLEANUP:
    SS_LEAVE(file);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Test file open status
 *
 * Description: Determines whether a file named NAME (names normalized with ss_file_fixname() before being compared) is open,
 *              or whether the persistent file object FILE corresponds to an open file, or whether FILE is currently mapped to
 *              NAME depending on whether only NAME is specified, only FILE is specified, or both NAME and FILE are specified,
 *              respectively.
 *
 * Return:      Returns true (a positive HDF5 file handle) if the file is currently open (explicitly or implicitly); false
 *              if the file is not currently open; negative otherwise. The HDF5 file handle is not duplicated and the client
 *              should not invoke H5Fclose() on the return value.
 *
 * Parallel:    Independent. However, since the underlying HDF5 file was opened collectively, many operations on that file
 *              must necessarily be collective and therefore if the return value is to be used as a file (instead of a logic
 *              value) then this function will most likely be called collectively across the file's communicator.
 *
 * Issues:      The returned HDF5 file handle is not duplicated before being returned for three reasons: (1) the H5Freopen()
 *              function returns a handle which does not participate in the same file mount structure as the original and thus
 *              we cannot guarantee that SSlib's file view would be consistent with that of the returned handle, (2) the
 *              H5Freopen() function is collective which would preclude this function from being usable as an independent test
 *              of file availability, and (3) requiring the caller to H5Fclose() the return value gets in the way of using
 *              this function as a predicate.
 *
 *              Since transient files are not supported by HDF5 there can be no HDF5 file handle for a file created with the
 *              H5F_ACC_TRANSIENT bit set. This function returns the integer 1 for such files, which is a positive true value
 *              but which is not a valid HDF5 file handle (or any valid handle for that matter).
 *
 * Also:        ss_file_hid()
 *
 * Programmer:  Robb Matzke
 *              Tuesday, May 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
ss_file_isopen(ss_file_t *file,         /* Optional handle to a persistent File object. */
               const char *name         /* Optional real name of file to test for open status. */
               )
{
    SS_ENTER(ss_file_isopen, hid_t);
    ss_gfile_t          *gfile=NULL;
    hid_t               retval=0;       /* zero is false and not a valid HDF5 file handle */
    char                *fixedname=NULL;
    size_t              i;

    if (file) SS_ASSERT_MEM(file, ss_file_t);

    if (file && name) {
        /* Check that FILE is opened with NAME */
        if (NULL==(gfile=SS_GFILE_LINK(file))) SS_ERROR(NOTFOUND);
        if (gfile->cur_open>0) {
            if (NULL==(fixedname=ss_file_fixname(name, NULL, NULL, 0, NULL))) SS_ERROR(FAILED);
            if (!strcmp(fixedname, gfile->name)) retval = gfile->fid;
            fixedname = SS_FREE(fixedname);
        }

    } else if (file) {
        /* Check that FILE is open */
        if (NULL==(gfile=SS_GFILE_LINK(file))) SS_ERROR(NOTFOUND);
        if (gfile->cur_open>0) retval = gfile->fid;
    } else if (name) {
        /* Check that NAME is open */
        if (NULL==(fixedname=ss_file_fixname(name, NULL, NULL, 0, NULL))) SS_ERROR(FAILED);
        if (SS_NOSIZE!=(i=ss_gfile_find_name(fixedname))) {
            gfile = SS_GFILE_IDX(i);
            if (gfile->cur_open>0) retval = gfile->fid;
        }
        fixedname = SS_FREE(fixedname);

    } else {
        SS_ERROR(USAGE);
    }
    
 SS_CLEANUP:
    SS_FREE(fixedname);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Tests transient state of a file
 *
 * Description: This function tests whether FILE is a transient file. Transient files don't correspond to any underlying
 *              permanent storage (not even to an HDF5 file with a !core driver).
 *
 * Return:      Returns true (positive) if FILE is a transient file; false if FILE is a permanent file; negative on error. It
 *              is an error to query a file which isn't in memory yet and therefore doesn't correspond to an open file.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_file_istransient(ss_file_t *file     /* A link to some File object */
                    )
{
    SS_ENTER(ss_file_istransient, htri_t);
    ss_gfile_t          *gfile=NULL;

    if (ss_file_isopen(file, NULL)<=0) SS_ERROR_FMT(PERM, ("FILE is not open"));
    if (NULL==(gfile=SS_GFILE_LINK(file))) SS_ERROR(FAILED);

 SS_CLEANUP:
    SS_LEAVE(gfile->flags & H5F_ACC_TRANSIENT);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Test file writability
 *
 * Description: Files can be open for read-only access or for read and write access. This function tests the writing
 *              capability of the file in question.
 *
 * Return:      Returns true (positive) if FILE was opened with the H5F_ACC_RDWR flag and false otherwise; returns negative on
 *              failure, including when FILE is not open.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, January 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_file_iswritable(ss_file_t *file      /* A link to some File object */
                   )
{
    SS_ENTER(ss_file_iswritable, htri_t);
    htri_t              retval=-1;
    ss_gfile_t          *gfile=NULL;

    if (ss_file_isopen(file, NULL)<=0) SS_ERROR(NOTOPEN);
    if (NULL==(gfile=SS_GFILE_LINK(file))) SS_ERROR(NOTFOUND);
    retval = (gfile->flags & H5F_ACC_RDWR) ? TRUE : FALSE;
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Mark file as read-only
 *
 * Description: A file can be marked as read-only even after it is opened for read-write. This is often useful when a file is
 *              created since a read-only file results in certain optimizations (such as knowing that such a file is always in
 *              a synchronized state). Marking a file as read-only can be substantially faster than closing the file and then
 *              reopening it since no I/O needs to happen.
 *
 *              The file should be in a synchronized state before this function is called.
 *
 * Issue:       It would be nice if HDF5 had a similar function, which would add an extra level of error checking to prevent
 *              SSlib from accidently writing to the HDF5 file after it was marked as read-only.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the file's communicator.
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_file_readonly(ss_file_t *file)
{
    SS_ENTER(ss_file_readonly, herr_t);
    ss_gfile_t  *gfile=NULL;

    if (ss_file_isopen(file, NULL)<=0) SS_ERROR(NOTOPEN);
    if (NULL==(gfile=SS_GFILE_LINK(file))) SS_ERROR(NOTFOUND);
    gfile->flags &= ~H5F_ACC_RDWR;
    gfile->flags |= H5F_ACC_RDONLY;
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Synchronize all scopes of a file
 *
 * Description: As mentioned in ss_scope_synchronize(), persistent object tables may become unsynchronized across the MPI
 *              tasks that own them. This function is simply a convenience function to synchronize all tables of all scopes
 *              that belong to the specified open file. See [Synchronization Algorithm] for more details.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the file's communicator.
 *
 * Issues:      If the caller supplies a property list then the `err_newptrs' integer property should be a member of that list
 *              since it is used internally by this function.
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_file_synchronize(ss_file_t *file,    /* The file to synchronize. */
                    ss_prop_t *props    /* Optional synchronization properties. */
                    )
{
    SS_ENTER(ss_file_synchronize, herr_t);
    ss_scope_sync_t     sync_data;      /* Stuff to pass through ss_table_scan() */
    ss_table_t          *scopetab=NULL; /* The top-scope's scope table */
    unsigned            tableidx;       /* Counter through valid tables of each scope */
    ss_scope_t          topscope;       /* Top scope of FILE */
    static ss_prop_t    *syncprops=NULL;
    int                 pass=0, old_transient_errors=0;

    if (!syncprops) {
        if (NULL==(syncprops=ss_prop_new(_func))) SS_ERROR(FAILED);
        if (ss_prop_add(syncprops, "err_newptrs", H5T_NATIVE_INT, NULL)<0) SS_ERROR(FAILED);
    }
    if (!props) props = syncprops;
    
    if (ss_file_isopen(file, NULL)<=0) SS_ERROR_FMT(PERM, ("FILE is not open"));

    /* If the file is read-only there should be nothing to do. However, it is still possible that the client modified the
     * object anyway and the only possible way to detect that is to attempt to synchronize. So if the library is compiled for
     * debugging then we'll descend, otherwise we'll return now. */
#ifdef NDEBUG
    if (!ss_file_iswritable(file)) goto done;
#endif

    /* Synchronize the top scope's scope table. This is file-collective because the top scope's communicator is the file
     * communicator. */
    if (NULL==ss_file_topscope(file, &topscope)) SS_ERROR(FAILED);
    if (ss_scope_synchronize(&topscope, SS_MAGIC(ss_scope_t), props)<0) SS_ERROR(FAILED);
    if (NULL==(scopetab=ss_scope_table(&topscope, SS_MAGIC(ss_scope_t), NULL))) SS_ERROR(FAILED);

    /* Now synchronize the rest of the tables. The outer loop is per table while the inner loop is per scope. This will
     * minimize the number of references to unsynchronized objects when one scope points to objects in another scope. */
    memset(&sync_data, 0, sizeof sync_data);
    sync_data.props = props;
    while (pass++<10) {
        sync_data.transient_errors = 0;
        for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
            if (!SS_PERS_CLASS(tableidx)) continue;
            if (tableidx==SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scopeobj_t))) continue; /*already synced*/
            sync_data.tableidx = tableidx;
            if (ss_table_scan(scopetab, &topscope, 0, ss_scope_synchronize_cb, &sync_data)<0) SS_ERROR(FAILED);
        }
        if (0==sync_data.transient_errors) break;
        if (pass>0 && sync_data.transient_errors==old_transient_errors)
            SS_ERROR_FMT(FAILED, ("unable to resolve inter-file cycles in the object graph"));
        old_transient_errors = sync_data.transient_errors;
    }
    
#ifdef NDEBUG
done:
#endif
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Query file synchronization state
 *
 * Description: As detailed in ss_scope_synchronize(), scopes may become out of sync across the MPI tasks that own them. This
 *              function queries all open scopes of the specified file to determine if any of them need to be synchronized,
 *              and may be faster than calling ss_file_synchronize() even when all scopes are in a synchronized state.
 *
 * Return:      Returns true (positive) if all scopes of FILE are currently synchronized; false if any scope needs to be
 *              synchronized; negative on failure.
 *
 * Parallel:    Collective across the file's communicator.
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_file_synchronized(ss_file_t *file    /* An open file */
                     )
{
    SS_ENTER(ss_file_synchronized, htri_t);
    ss_prop_t           *props=NULL;
    htri_t              retval=TRUE;

    /* Build a synchronization property that says to only test synchronization */
    if (NULL==(props=ss_prop_new("test synchronization"))) SS_ERROR(FAILED);
    if (ss_prop_add(props, "test", H5T_NATIVE_HBOOL, &true)<0) SS_ERROR(FAILED);

    /* Test synchronization */
    if (ss_file_synchronize(file, props)<0) {
        const H5E_error2_t *einfo;
        SS_STATUS(0, einfo);
        if (einfo->min_num!=SS_MINOR_SKIPPED) {
            SS_REFAIL;
            SS_ERROR(FAILED);
        } else {
            SS_STATUS_OK;
            retval = FALSE;
            goto done;
        }
    }

 done:
    /* Clean up the properties */
    ss_prop_dest(props);
    props = NULL;
    
 SS_CLEANUP:
    if (props) ss_prop_dest(props);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Write pending data to file
 *
 * Description: As objects are created or modified the library caches changes in memory to prevent repeatedly writing to disk.
 *              This function writes that data to disk. However, to reduce the amount of communication necessary in cases where
 *              the caller knows the file is synchronized, the various data flushing functions do not synchronize first, so
 *              the caller should invoke ss_file_synchronize().  The flushing functions also do not generally guarantee that
 *              the data is flushed from HDF5 to the underlying file.
 *
 *              Flushing a transient file is a no-op.
 *
 *              The ss_file_close() function both synchronizes and flushes.
 *
 * Example:     The following code flushes data to HDF5 and then tells HDF5 to flush its data to the file:
 *
 *                  ss_file_t file = ss_file_open(....);
 *                  ....
 *                  ss_file_synchronize(file);
 *                  ss_file_flush(file, NULL);
 *                  H5Fflush(ss_file_isopen(file), H5F_SCOPE_GLOBAL);
 *
 * Return:      Returns non-negative on success, negative on failure.  It is an error to attempt to flush a file which is not
 *              open.
 *
 * Parallel:    Collective across the file's communicator (see ss_file_open()).
 *
 * Programmer:  Robb Matzke
 *              Tuesday, May 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_file_flush(ss_file_t *file,          /* The file to be flushed. */
              ss_prop_t *props          /* Flushing properties. See ss_scope_flush(). */
              )
{
    SS_ENTER(ss_file_flush, herr_t);
    ss_table_t          *table=NULL;
    ss_scope_t          topscope;

    if (ss_file_isopen(file, NULL)<=0) SS_ERROR_FMT(PERM, ("FILE is not open"));
    if (NULL==ss_file_topscope(file, &topscope)) SS_ERROR(FAILED);

    /* Flush all scopes of this file */
    if (NULL==(table=ss_scope_table(&topscope, SS_MAGIC(ss_scope_t), NULL))) SS_ERROR(FAILED);
    if (ss_table_scan(table, &topscope, 0, ss_scope_flush_cb, props)<0)
        SS_ERROR_FMT(FAILED, ("\"%s\"", SS_FILE(file)->name));

    /* Flush all pending asynchronous writes */
    if (ss_blob_flush(&topscope, NULL, SS_STRICT, NULL)<0) SS_ERROR(FAILED);

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Close a file
 *
 * Description: Closes a file and all scopes belonging to that file.  All scopes belonging to the file are synchronized
 *              and flushed to the file first, and then all such scopes are closed.  If the file contains scopes that were
 *              serving as registries for other files, those scopes will be removed from those files' registry stacks.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the file's communicator (see ss_file_open()).
 *
 * Issues:      Closing a file simply causes the cur_open reference counter to be decremented in the GFile array. When
 *              this counter reaches zero the file is considered to be closed and we call H5Fclose(), but we don't entirely
 *              reset the GFile array entry to zero just in case there are still things pointing into this file.
 *
 * Also:        ss_file_flush(), ss_file_synchronize()
 *
 * Programmer:  Robb Matzke
 *              Monday, May 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_file_close(ss_file_t *file           /* The file to be closed */
              )
{
    SS_ENTER(ss_file_close, herr_t);
    ss_gfile_t          *gfile=NULL;                    /* GFile array entry for the file owning the FILE object */
    ss_table_t          *table=NULL;                    /* A persistent object table */
    ss_scope_t          topscope=SS_SCOPE_NULL;         /* The top scope of FILE */
    ss_scope_t          *reg=NULL;                      /* Object registry entry */
    int                 nopen=0;                        /* Number of File objects explicitly open in this file */
    size_t              gfile_idx, nreg, reg_idx;
    htri_t              is_same;

    /* File must be explicitly open in order to be closed */
    if (ss_file_isopen(file, NULL)<=0) SS_ERROR_FMT(PERM, ("FILE is not an open file"));
    if (NULL==(gfile = SS_GFILE_LINK(file))) SS_ERROR(NOTFOUND);
    if (gfile->cur_open<=0) SS_ERROR_FMT(PERM, ("FILE is not explicitly open"));
    if (NULL==ss_file_topscope(file, &topscope)) SS_ERROR(FAILED);

    /* Flush the file before we even try anything else. We can skip this for a transient file. */
    if (!ss_file_istransient(file)) {
        if (ss_file_synchronize(file, NULL)<0) SS_ERROR(FAILED);
        if (ss_file_flush(file, NULL)<0) SS_ERROR(FAILED);
    }
    
    /* Look some things up before we start destroying data structures */
    if (NULL==(gfile=SS_GFILE_LINK(&topscope))) SS_ERROR(FAILED);

    /* Close the file for real if this is the last file that explicitly references the underlying HDF5 file. Do not
     * decrement the gfile->cur_open yet because some functions below will need to know that the file isn't completely
     * closed yet (e.g., ss_pers_deref()). */
    if (1==gfile->cur_open) {
        /* Find all top-level scopes that use this as a registry and remove that association */
        for (gfile_idx=0; NULL!=(gfile=SS_GFILE_IDX(gfile_idx)); gfile_idx++) {
            if (gfile->cur_open>0) {
                nreg = gfile->reg_nused;
                reg =  gfile->reg;
                reg_idx = 0;
                while (reg_idx++<nreg) {
                    if ((is_same=SS_PERS_EQ(&topscope, reg))<0) SS_ERROR(FAILED);
                    if (is_same) {
                        --nreg;
                        memmove(reg, reg+1, nreg*sizeof(*reg));
                        reg[nreg] = SS_SCOPE_NULL;
                        gfile->reg_nused -= 1;
                    } else {
                        reg++;
                    }
                }
            }
        }

        /* Remove all this file's registries */
        gfile = SS_GFILE_LINK(&topscope);
        if (gfile->reg_nused) {
            gfile->reg_nused = 0;
            gfile->reg_nalloc = 0;
            gfile->reg = SS_FREE(gfile->reg);
        }

        /* Close all open scopes in the closing file. This removes the objects from memory (some of which may occupy a
         * substantial amount of memory) but leaves the relatively small indirect index mapping information so that any
         * persistent object that still points into the closing file will be able to convert indirect object links into direct
         * object links if necessary. It would be nice to be able to also close any File objects that might have been
         * explicitly opened, but alas, our current collectivity might not match that by which the contained File object was
         * opened and hence must be closed; but we can warn about that with some extra work. */
        if (NULL==(table=ss_scope_table(&topscope, SS_MAGIC(ss_scope_t), NULL))) SS_ERROR(FAILED);
        if (ss_table_scan(table, &topscope, 0, ss_file_close1_cb, &nopen)<0) SS_ERROR(FAILED);
        SS_ASSERT(nopen>0); /* because this file, which is File zero of the top scope, is explicitly open yet */
        if (nopen>1) SS_ERROR_FMT(USAGE, ("%d file%s still open in %s", nopen-1, 2==nopen?"":"s", gfile->name));

        /* Destroy the global blob table for this file */
        if (ss_blob_desttab(gfile->gblob)<0) SS_ERROR(FAILED);
        gfile->gblob = NULL;

        /* Close the HDF5 file (fid==1 implies transient file). This is where all those scope groups and table datasets get
         * closed since in the interest of less collectivity those functions just dropped the handles instead of closing them.
         * See the H5F_CLOSE_STRONG property in ss_file_open(). */
        if (gfile->fid>1 && H5Fclose(gfile->fid)<0) SS_ERROR(HDF5);
        gfile->fid = 0;

        /* Release other resources */
        if (gfile->dxpl_independent>0 && H5Pclose(gfile->dxpl_independent)<0) SS_ERROR(HDF5);
        gfile->dxpl_independent = 0;
        if (gfile->dxpl_collective>0 && H5Pclose(gfile->dxpl_collective)<0) SS_ERROR(HDF5);
        gfile->dxpl_collective = 0;
    }

    /* Decrement file open counter */
    --gfile->cur_open;
    
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Files
 * Purpose:     Callback from ss_file_close()
 *
 * Description: This function gets invoked for every scope of the closing file from ss_file_close(). If the library is so
 *              configured, it traverses the File table in the scope and counts the files that are explicitly open. In any case,
 *              the scope is partially destroyed by partially destroying each of the scope's tables, leaving only enough
 *              information that persistent object links that have indirect indices into the closing file can convert them to
 *              direct indices (this is called "closing the scope").
 *
 * Return:      Returns zero on success; negative on failure.
 *
 * Parallel:    Collective across the closing file's communicator.
 *
 * Also:        ss_table_dest()
 *
 * Programmer:  Robb Matzke
 *              Monday, January 12, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
htri_t
ss_file_close1_cb(size_t UNUSED itemidx, ss_pers_t *pers, ss_persobj_t UNUSED *persobj, void *udata)
{
    SS_ENTER(ss_file_close1_cb, htri_t);
    ss_scope_t          *scope=(ss_scope_t*)pers;       /* scope to be closed when closing a file */
    ss_table_t          *filetab=NULL;
    hid_t               scope_grp;
    
    /* Count explicitly open files */
    if (NULL==(filetab=ss_scope_table(scope, SS_MAGIC(ss_file_t), NULL))) SS_ERROR(FAILED);
    if (ss_table_scan(filetab, scope, 0, ss_file_close2_cb, udata)<0) SS_ERROR(FAILED);

    /* Partially destroy the scope by closing it where opened. The ss_scope_close() is scope-collective, which is a subset of
     * file-collective, and the ss_scope_isopen() makes sure that only those tasks in the scope's communicator call
     * ss_scope_close(). */
    if ((scope_grp=ss_scope_isopen(scope))<0) SS_ERROR(FAILED);
    if (scope_grp && ss_scope_close(scope)<0) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Files
 * Purpose:     Callback from ss_file_close()
 *
 * Description: This function gets called from ss_file_close1_cb() for all entries in a File table in order to make sure that
 *              all explicitly opened files are closed.  It simply counts the number of explicitly opened files because the
 *              collectivity of ss_file_close1_cb() might be different than the collectivity required to close the files.
 *
 * Return:      Returns false on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, December 17, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
htri_t
ss_file_close2_cb(size_t UNUSED itemidx, ss_pers_t *pers, ss_persobj_t UNUSED *persobj, void UNUSED *udata)
{
    SS_ENTER(ss_file_close2_cb, htri_t);
    ss_file_t           *file = (ss_file_t*)pers;
    int                 *nopen=(int*)udata;             /* counter for number of open files in this scope */
    hid_t               hdf5file;

    if ((hdf5file=ss_file_isopen(file, NULL))<0) SS_ERROR(FAILED);
    if (hdf5file && SS_FILE(file)->m.explicit_open) (*nopen)++;

SS_CLEANUP:
    SS_LEAVE(FALSE);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Files
 * Purpose:     Closes all open files
 *
 * Description: A call to this function flushes and closes all open files. Normally one would only do this when finalizing the
 *              library.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the library communicator.
 *
 * Programmer:  Robb Matzke
 *              Monday, June  7, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_file_closeall(FILE *verbose          /* If non-null then information about open files will be printed to this stream. */
                 )
{
    SS_ENTER(ss_file_closeall, herr_t);
    ss_gfile_t          *gfile;
    size_t              i, gfile_idx, *sernum=NULL, nsernums, sernum_nalloc=0;
    int                 root, self, ntasks, filetask, nfiletasks, my_need_header=0, need_header, task, libtask;
    int                 range_start, range_end=-1;
    ss_file_t           file;
    unsigned long       flags;
    MPI_Comm            filecomm;

    self = ss_mpi_comm_rank(sslib_g.comm);
    ntasks = ss_mpi_comm_size(sslib_g.comm);

    /* If verbose we need to decide whether there are any file names to report in order to print the header */
    if (verbose) {
        for (gfile_idx=0; !my_need_header && (gfile=SS_GFILE_IDX(gfile_idx)); gfile_idx++) {
            if (gfile->cur_open && (gfile->flags & H5F_ACC_RDWR) && 0==(gfile->flags & H5F_ACC_TRANSIENT))
                my_need_header++;
        }
#ifdef HAVE_PARALLEL
        if (MPI_Allreduce(&my_need_header, &need_header, 1, MPI_INT, MPI_SUM, sslib_g.comm)) SS_ERROR(MPI);
#else
        need_header = my_need_header;
#endif
        if (need_header) {
            if (0==self) fprintf(verbose, "SAF file(s) still open at library termination:\n");
            ss_mpi_barrier(sslib_g.comm);
        }
    }

    /* Now close one task's files at a time with other tasks jumping into the collective calls as necessary. */
    for (root=0; root<ntasks; root++) {
        /* Get a list of serial numbers for files still open on the `root' task. */
        if (root==self) {
            nsernums=0;
            for (gfile_idx=0; (gfile=SS_GFILE_IDX(gfile_idx)); gfile_idx++) {
                if (gfile->cur_open) {
                    SS_EXTEND(sernum, MAX(nsernums+1, 64), sernum_nalloc);
                    sernum[nsernums++] = gfile->serial;
                }
            }
        }
        if (ss_mpi_bcast(&nsernums, 1, MPI_SIZE_T, root, sslib_g.comm)<0) SS_ERROR(FAILED);
        SS_EXTEND(sernum, nsernums, sernum_nalloc);
        if (nsernums && ss_mpi_bcast(sernum, nsernums, MPI_SIZE_T, root, sslib_g.comm)<0) SS_ERROR(FAILED);
        
        /* Process each file by serial number. Only some of the tasks might have a file open */
        for (i=0; i<nsernums; i++) {
            gfile_idx = ss_gfile_find_serial(sernum[i]); SS_STATUS_OK;
            gfile = SS_GFILE_IDX(gfile_idx);

            if (gfile && gfile->cur_open) {
                /* Print information about the file if it is open for writing and not transient */
                if (verbose && (gfile->flags & H5F_ACC_RDWR) && 0==(gfile->flags & H5F_ACC_TRANSIENT)) {
                    if (ss_scope_comm(gfile->topscope, &filecomm, &filetask, &nfiletasks)<0) SS_ERROR(FAILED);
                    if (0==filetask) {
                        fprintf(verbose, "    %s\n", gfile->name);
                        fprintf(verbose, "        serial=%lu, hdf5=%lu, cur_open=%lu, flags=",
                                (unsigned long)(gfile->serial), (unsigned long)(gfile->fid),
                                (unsigned long)(gfile->cur_open));

                        flags = gfile->flags;
                        fprintf(verbose, "%s", (flags & H5F_ACC_RDWR) ? "RDWR" : "RDONLY");
                        flags &= ~H5F_ACC_RDWR;
                        if (flags & H5F_ACC_TRANSIENT) {
                            fprintf(verbose, "|TRANSIENT");
                            flags &= ~H5F_ACC_TRANSIENT;
                        }
                        if (flags & H5F_ACC_TRUNC) {
                            fprintf(verbose, "|TRUNC");
                            flags &= ~H5F_ACC_TRUNC;
                        }
                        if (flags & H5F_ACC_EXCL) {
                            fprintf(verbose, "|EXCL");
                            flags &= ~H5F_ACC_EXCL;
                        }
                        if (flags & H5F_ACC_CREAT) {
                            fprintf(verbose, "|CREAT");
                            flags &= ~H5F_ACC_CREAT;
                        }
                        if (flags & H5F_ACC_DEBUG) {
                            fprintf(verbose, "|DEBUG");
                            flags &= ~H5F_ACC_DEBUG;
                        }
                        if (flags) {
                            fprintf(verbose, "|0x%08lx", (unsigned long)flags);
                        }
                        fprintf(verbose, "\n");
                        
                        if (ntasks>1) {
                            fprintf(verbose, "        open on task%s ", 1==nfiletasks?"":"s");
                            for (task=0, range_start=-1; task<nfiletasks; task++) {
                                libtask = ss_mpi_maptask(task, filecomm, SS_COMM_WORLD);
                                /* Try to reduce clutter by printing task ranges */
                                if (range_start<0) {
                                    fprintf(verbose, "%s%d", task?",":"", libtask);
                                    range_start = range_end = libtask;
                                } else if (libtask==range_end+1) {
                                    range_end = libtask;
                                } else if (range_start==range_end) {
                                    fprintf(verbose, ",%d", libtask);
                                    range_start = range_end = libtask;
                                } else if (range_start+1==range_end) {
                                    fprintf(verbose, ",%d,%d", range_end, libtask);
                                    range_start = range_end = libtask;
                                } else {
                                    fprintf(verbose, "-%d,%d", range_end, libtask);
                                    range_start = range_end = libtask;
                                }
                            }
                            if (range_start>=0) {
                                if (range_start+1==range_end) {
                                    fprintf(verbose, ",%d", range_end);
                                } else if (range_start<range_end) {
                                    fprintf(verbose, "-%d", range_end);
                                }
                            }
                            if (nfiletasks>2) fprintf(verbose, " (%d tasks)", nfiletasks);
                            fprintf(verbose, "\n");
                        }
                    }
                }

                /* Build a reference to the file object in the top-scope of that file and use it to close the file */
                if (NULL==ss_pers_refer_c(gfile->topscope, SS_MAGIC(ss_file_t), (size_t)0, (ss_pers_t*)&file)) SS_ERROR(FAILED);
                while (gfile->cur_open) {
                    if (ss_file_close(&file)<0) SS_ERROR(FAILED);
                }
            }
        }
    }
    sernum = SS_FREE(sernum);

SS_CLEANUP:
    SS_FREE(sernum);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Attach an object registry scope
 *
 * Description: A /find/ operation searches a specific scope for objects that match some partially initialized key value.
 *              However, sometimes object definitions are in a separate object registry instead and should be "sucked into"
 *              the main file as necessary.  An object registry is simply a stack of additional scopes to search when a /find/
 *              operation for the specified scope fails to locate any matching objects.
 *
 *              If a scope of FILE is searched during a /find/ operation and results in no matches, then the REGISTRY scope is
 *              searched (registries are searched in the order defined with this function) and the object handle that gets
 *              returned is marked as coming from a registry. The current version of the library simply makes links to the
 *              registry scope, but a future version might copy the object from the registry into the specified FILE along
 *              with all prerequisites.
 *
 *              Registry lists are associated with the shared file information in the GFile array. That is, if two
 *              ss_file_t objects are opened and refer to the same underlying HDF5 file, then adding a registry to one of
 *              those ss_file_t links will cause the other link to also see the registry.  This allows files that are opened
 *              implicitly to automatically use the same registry as their explicitly opened counterpart.
 *
 *              *Note*: The /File/, /Scope/, and /Blob/ tables, which describe infrastructure, do not use object
 *               registries during /find/ operations. If the REGISTRY scope is closed then it is automatically removed from
 *               all the files for which it's serving as a registry.
 *
 * Example:     Here's how this function might be used:
 *
 *                ss_file_t file = ss_file_open("registry.saf",H5F_ACC_RDONLY,NULL);
 *                ss_scope_t registry = ss_file_topscope(file);
 *                ss_file_t myfile = ss_file_create("myfile.saf",H5F_ACC_RDWR,NULL);
 *                ss_file_registry(myfile,registry);
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent, although it's typically used in such a way that all tasks of the FILE communicator make identical
 *              calls to this function to define a common set of object registries.
 *
 * Programmer:  Robb Matzke
 *              Monday, May 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_file_registry(ss_file_t *file,       /* The file that is getting the new registry. */
                 ss_scope_t *registry   /* The open scope to serve as the registry. This need not be a top-level scope though
                                         * it usually is. It could even be some scope within FILE in an extreme case. */
                 )
{
    SS_ENTER(ss_file_registry, herr_t);
    ss_gfile_t          *gfile=NULL;

    if (ss_file_isopen(file, NULL)<=0) SS_ERROR_FMT(PERM, ("FILE is not an open file"));
    if (ss_scope_isopen(registry)<=0) SS_ERROR_FMT(USAGE, ("REGISTRY is not an open scope"));
    if (NULL==(gfile=SS_GFILE_LINK(file))) SS_ERROR(NOTFOUND);

    /* Add this registry to the end of the list of registries for the file */
    SS_EXTEND(gfile->reg, gfile->reg_nused+1, gfile->reg_nalloc);
    gfile->reg[gfile->reg_nused] = *registry;
    gfile->reg_nused++;

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Files
 * Purpose:     Normalize file name
 *
 * Description: Normalize the given file NAME by making it an absolute name using the current working directory, or relative
 *              to RELATIVE, removing multiple consecutive slashes and trailing slashes, and removing "." and ".." components
 *              (except where necessary to make the name relative). This function does not attempt to normalize names by
 *              reading symbolic link information from the file system, therefore names that traverse a symbolic link are not
 *              unique but names of non-existing files can be normalized.
 *
 * Return:      Returns the normalized, null-terminated name stored in BUFFER (or an allocated buffer if BUFFER is the null
 *              pointer).  Returns the null pointer on error, including errors due to the supplied BUFFER not being large
 *              enough to hold the result.  In an error condition, the contents of BUFFER (if supplied by the caller) may have
 *              been partially modified.  The allocated return value should eventually be released by calling free().
 *
 * Parallel:    Independent
 *
 * Issues:      The behavior of not resolving symbolic links to their pointee names is required in order to make transient
 *              files work properly.
 *
 *              When creating a relative name (i.e., RELATIVE is non-null) then the supplied buffer, if any, should be large
 *              enough to hold both the normlized absolute name and the generated relative name.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, May 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
ss_file_fixname(const char *name,       /* Incoming file name. */
                const char *wrt,        /* Optional starting point for relative names, or cwd if NULL. Must be absolute and
                                         * normalized, done by calling this function first with WRT as the NAME. */
                const char *relative,   /* Optional. If specified this should be the absolute name of some file and the
                                         * returned normalized string will be relative to this file. If the caller supplies a
                                         * directory name for this argument then it should end with a "/." component so the
                                         * correct number of ".." components are prepended to the returned value. If the
                                         * string "." is given then return a name relative to the current working directory. */
                size_t len,             /* Length of result buffer if BUFFER is supplied, otherwise ignored. */
                char *buffer            /* Optional result buffer. */
                )
{
    SS_ENTER(ss_file_fixname, charP);
    char        *buffer_allocated=NULL; /* An allocated BUFFER value, allocated in this function */
    size_t      cur=0;                  /* Current offset into result buffer */
    int         pass;                   /* We make two passes through this code: one for the WRT and one for NAME */
    const char  *s=NULL;                /* The string on which we're working for any given pass through the code */
    const char  *end=NULL;              /* Points to end of next component of source */
    const char  *r, *r_end;             /* Pointers into RELATIVE and the end of a component in RELATIVE */
    char        *x=NULL;                /* Very temporary pointer to allocated memory */
    char        *wrt_allocated=NULL;    /* An allocated WRT value, allocated in this function */
    size_t      wrt_size=0;             /* Size of wrt_allocated */
    int         ncomps, i;              /* Number of components remaining in a file name */

#ifdef WIN32
    /*handle windows case where absolute path is like "c:\" or "d://"*/
    if (wrt && '/'!=wrt[0] && !(strlen(wrt)>=3 && wrt[1]==':' && (wrt[2]=='/' || wrt[2]=='\\')))
        SS_ERROR_FMT(USAGE, ("WRT must be absolute if supplied"));
#else
    if (wrt && '/'!=wrt[0]) SS_ERROR_FMT(USAGE, ("WRT must be absolute if supplied"));
#endif
    if (!name) SS_ERROR_FMT(USAGE, ("NAME must be non-null"));

    /* Allocate buffer for the result. We will reallocate this below if it needs to grow. */
    if (!buffer) {
        len = 256;
        if (NULL==(buffer=buffer_allocated=malloc(len))) SS_ERROR(RESOURCE);
    }

    /* Get the current working directory name if necessary. We can't assume GNU allocation semantics for getcwd(). */
    if (
#ifdef WIN32
        /* Handle windows case where absolute path is like "c:\" or "d://".  This is split this way so as not to confuse automatic indentation engines. */
        ('/'!=*name && !wrt && !(strlen(name)>=3 && name[1]==':' && (name[2]=='/' || name[2]=='\\')) ) || (relative && !strcmp(relative, "."))
#else
        ('/'!=*name && !wrt) || (relative && !strcmp(relative, "."))
#endif
        ) {
        wrt_size = 256;
        while (true) {
            if (NULL==(wrt_allocated=malloc(wrt_size))) SS_ERROR(RESOURCE);
            if (wrt_allocated==getcwd(wrt_allocated, wrt_size)) {
                if (strlen(wrt_allocated)+strlen("/.")<wrt_size) break;
                errno=ERANGE;
            }
            wrt_allocated = SS_FREE(wrt_allocated);
            if (ERANGE!=errno) SS_ERROR_FMT(FAILED, ("getcwd()"));
            wrt_size *= 2;
        }
    }

    /* Do two passes: first process the WRT then the NAME. If no WRT was specified and NAME is a relative name then use the
     * current working directory. */
    for (pass=0; pass<2; pass++) {
        if (0==pass) {
#ifdef WIN32
            /*handle windows case where absolute path is like "c:\" or "d://"*/
            if (strlen(name)>=3 && name[1]==':' && (name[2]=='/' || name[2]=='\\')) continue;
#endif
            if ('/'==*name) continue;
            if (!wrt) wrt = wrt_allocated;
            s = wrt;
        } else {
            s = name;
        }
        
        while (s) {
            /* Skip slashes in the source and bail out if it's empty now. */
            while ('/'==*s) s++;
            if (!*s) break;

            /* Get the extent of the next component of the source */
            for (end=s; *end && '/'!=*end; end++) /*void*/;
            SS_ASSERT(end>s);
            
            /* Process special components. The `.' component is simply ignored and the `..' component causes us to pop all
             * characters from the buffer including the previous slash. */
            if (!strncmp(s, ".", (size_t)(end-s))) {
                s = end;
                continue;
            } else if (!strncmp(s, "..", (size_t)(end-s))) {
                while (cur>0 && '/'!=buffer[cur-1]) --cur; /*everything up to the previous slash*/
                if (cur>0) --cur; /*and then the previous slash*/
                s = end;
                continue;
            }

            /* Append a slash to the result followed by the next component of the source */
            if (cur+1+(end-s)>len) {
                if (buffer_allocated) {
                    len = MAX(len*2, cur+1+(end-s));
                    if (NULL==(x=realloc(buffer, len))) SS_ERROR(RESOURCE);
                    buffer = buffer_allocated = x;
                } else {
                    SS_ERROR(OVERFLOW);
                }
            }

#ifdef WIN32
            /*handle windows case where absolute path is like "c:\" or "d://"*/
            if(!cur && strlen(s)>=3 && s[1]==':' && (s[2]=='/' || s[2]=='\\')) {
                /*do not prepend with '/' in this case*/
            }
            else
#endif
                buffer[cur++] = '/';
            memcpy(buffer+cur, s, (size_t)(end-s));
            cur += end-s;
            s = end;
        }
    }
        
    /* If the result is empty (which it could be if we've seen enough `..' components, then set it to `/' */
    if (0==cur) {
        if (!len) SS_ERROR(OVERFLOW);
        buffer[cur++] = '/';
    }

    /* Null terminate */
    if (cur>=len) {
        if (buffer_allocated) {
            len++;
            if (NULL==(x=realloc(buffer, len))) SS_ERROR(RESOURCE);
            buffer = buffer_allocated = x;
        } else {
            SS_ERROR(OVERFLOW);
        }
    }
    buffer[cur++] = '\0';

    /* Should we make the name relative to some other name? */
    if (relative) {
        /* Special case when RELATIVE is "." */
        if (!strcmp(relative, ".")) {
            strcat(wrt_allocated, "/."); /*because wrt_allocated is a directory, not a file (space allocated above) */
            relative = wrt_allocated;
        }
        
        /* Strip off components that are common to both names. Each component begins with a slash. */
        for (s=buffer, r=relative; '/'==*s && '/'==*relative; s=end, r=r_end) {
            for (end=s+1; *end && '/'!=*end; end++) /*void*/;
            for (r_end=r+1; *r_end && '/'!=*r_end; r_end++) /*void*/;
            if ((end-s)!=(r_end-r) || memcmp(s, r, (size_t)(end-s))) break;
        }

        /* For each component left in "relative" except the last one (the first of which is pointed to by "r") prepend a "../"
         * component to what's left of "buffer" (pointed to by "s"). Advance "s" past the initial slash first if necessary. */
        if (*r) r++;
        if ('/'==*s) s++;
        for (ncomps=0; *r; r++)
            if ('/'==*r) ncomps++;
        if (ncomps*3+strlen(s)>=len) {
            if (buffer_allocated) {
                len = MAX(len*2, ncomps*3+strlen(s)+1);
                if (NULL==(x=realloc(buffer, len))) SS_ERROR(RESOURCE);
                s = x + (s-buffer);
                buffer = buffer_allocated = x;
            } else {
                SS_ERROR(OVERFLOW);
            }
        }
        memmove(buffer+ncomps*3, s, strlen(s)+1);
        for (i=0; i<ncomps; i++)
            memcpy(buffer+i*3, "../", 3);
        if (!*buffer) strcpy(buffer, ".");
    }
    
    /* Successful cleanup */
    if (wrt_allocated) wrt = wrt_allocated = SS_FREE(wrt_allocated);

 SS_CLEANUP:
    SS_FREE(buffer_allocated);
    SS_FREE(wrt_allocated);
    
    SS_LEAVE(buffer);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Files
 * Purpose:     Obtain top scope
 *
 * Description: Given a file, return a link to the top scope of that file.  This is really just a convenience function for
 *              ss_pers_topscope() that exists mostly for compile-time type checking since this is an exceedingly common
 *              operation.
 *
 * Return:      Returns a pointer to a top-scope link on success (BUF if that was non-null); null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, December 17, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_scope_t *
ss_file_topscope(ss_file_t *file,               /* File for which to obtain a link to a top scope. */
                 ss_scope_t *buf                /* Optional buffer in which to store the resulting link. */
                 )
{
    SS_ENTER(ss_file_topscope, ss_scope_tP);
    ss_scope_t  *retval=NULL;

    SS_ASSERT_TYPE(file, ss_file_t);
    if (NULL==(retval=ss_pers_topscope((ss_pers_t*)file, buf))) SS_ERROR(FAILED);
SS_CLEANUP:
    SS_LEAVE(retval);
}
