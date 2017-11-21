/*
 * Copyright(C) 1999 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 1999 Sandia Corporation.
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
 * 
 * Acknowledgements:
 *     Mark C. Miller              LLNL - Design input
 */
#include "sslib.h"
SS_IF(scope);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 *
 * Description: A scope is a collection of persistent object tables that belong to a single file. A file always has one
 *              top-level scope called "SAF" which is returned when the file is opened (see ss_file_open()), but may have any
 *              number of additional scopes. Each scope is associated with a communicator which is a subset of the
 *              communicator for file containing the scope, and that communicator defines what tasks "own" the scope.
 *              Operations that open, close, or create scopes are generally collective over the file communicator; operations
 *              that modify the contents of a scope are generally collective over the scope communicator; operations that
 *              simply access the scope are generally independent.
 *
 *              Scopes satisfy a number of design goals:
 *              
 *              * Scopes minimize communication by isolating certain objects to a subset of MPI_COMM_WORLD.
 *              * Scopes provide a mechanism for controlled partial reads of the SAF metadata.
 *              * Scopes provide a framework for transient objects.
 *              * Scopes will allow for a crude form of object deletion but at a finer granularity than entire files.
 *              * Scopes turn SAF's auxiliary files into standalone SAF databases.
 *
 *              A scope is a type of persistent object pointed to by a variable of type ss_scope_t (see Persistent Objects).
 *              As such, a scope is simply an entry in a table that gets written to a file. Each file has only one scope table,
 *              called FILE:/SAF/Scopes. That is, only the top-level scope contains a /Scopes/ table and the first entry in
 *              that table is always the top-level scope itself, FILE:/SAF.
 *
 *              Since a scope is a persistent object, scopes are created, modified, destroyed, and queried just like any other
 *              persistent object. However, since a scope is also part of the file infrastructure, additional operations are
 *              defined and documented in this chapter.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Note:        Scope Properties
 * Description: * /comm/: The MPI communicator for the scope.  The default is to use the communicator for the file in which the
 *              scope is defined.
 *
 *              * /duped/: If set then SSlib assumes that the communicator in the /comm/ property has been duplicated and will
 *              free that communicator when the scope is closed. Otherwise the specified communicator is used but not
 *              duplicated or freed.
 *
 *              * /careful/: Normally a scope synchronization operation only considers table entries marked as dirty. If a scope
 *              is opened with this property set to a nonzero value then checksums will be recomputed for all entries in the
 *              tables, which allows the client to forego setting the dirty bit when modifying table entries. No warning will
 *              normally be issued for entries which are found to be dirty by comparing checksums but which have the dirty bit
 *              not set.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Note:        Synchronization Algorithm
 *
 * Description: Each persistent object belongs to exactly one scope, which is "owned" by some subset of the tasks in
 *              the file's communicator and these tasks are allowed to append new objects to the tables of the scope and to
 *              modify existing objects in those tables. Since it is of paramount importance that all tasks owning a scope
 *              have the same information to write to the underlying dataset, we must insure that objects are maintained
 *              consistently across the scope's communicator. There are at least three approaches:
 *
 *              * Ensure that any object update is collective and that all tasks make identical changes.
 *              * Ensure that all updates are collective but use communication to broadcast from one task to the others.
 *              * Allow objects to become out of sync across the tasks and synchronize them before writing to disk.
 *
 *              The problem with the first two approaches is that object updates are collective. The original VBT layer used
 *              this approach, which was made even worse by the fact that VBT updates used the file communicator. The second
 *              approach is worse than the first because not only is it collective, but it also requires communication. SSlib
 *              uses the third approach, which is substantially more complicated but results in an API where each task can
 *              independently create and/or modify objects, or subsets of the scope communicator can cooperate to define a
 *              single object. The process of getting an object to disk becomes a two part problem: (1) synchronize the object
 *              across the scope's communicator, then (2) flush the object if dirty to the file. It is hoped that a
 *              synchronization approach can perform better because it will result in fewer, larger inter-process messages.
 *
 *              First, some definitions:
 *
 *              * A /dirty/ object is any object whose data has changed subsequent to being stored in a file. This is a
 *                local characteristic--each task may have its own idea of whether an object is dirty. Normally only
 *                one task ever writes a particular object to the file, so only that one task ever sets dirty bits back
 *                to zero (but that's fine because the dirty bits are not used for any other purpose).
 *
 *              * A /clean/ object is any object that is not dirty.
 *
 *              * Whereas dirty and clean refer to whether an object was changed after being committed to storage,
 *                /synchronized/ and /unsynchronized/ refer to whether an object was changed after some point in time
 *                when all tasks agreed on its value. These characteristics are also local to a task, although the act of
 *                marking an object synchronized (i.e., synchronizing the object) is a collective operation.
 *
 *              * A /new/ object is any object that was created without the collective cooperation of all tasks of the
 *                object's scope. Such an object is born dirty and unsynchronized and is assigned a temporary slot in the
 *                table that holds the object. The act of synchronizing the object assigns the object to a permanent table
 *                slot. Objects can also be created with the SS_ALLSAME bit, but since those objects are immediately given a
 *                permanent slot assignement and marked as synchronized (but dirty) they are not referred to as /new/ objects.
 *
 *              * An /unresolved/link/ is a persistent object link that points to a new object. Since a new object must exist
 *                in memory, an unresolved link must be in the SS_PERS_LINK_MEMORY state and point to an object that uses
 *                only indirect indexing (i.e., the link itself contains an indirect index and the object's !mapidx field also
 *                contains an indirect index). Any link that points to a non-new object is a /resolved/link/.
 *
 *              * An /unresolved/object/ is any persistent object that contains at least one unresolved link.  All other
 *                objects are said to be /resolved/.
 *
 *              * To /synchronize/ an unsynchronized object means to communicate among the scope's tasks so that all tasks
 *                have the same information.
 *
 *              * To /clean/ an object means to write its data to the file.
 *
 *              * To /resolve/ a link means to examine the object to which it points and adjust the link contents if the
 *                object is found to be non-new. Otherwise the link remains unresolved.
 *
 *              * To /resolve/ an object means to resolve all links emanating from that object.
 *
 *              The synchronization algorithm operates on one table at a time via ss_table_synchronize() and is usually
 *              invoked in a particular order to minimize table dependencies since only resolved objects can be synchronized.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Note:        Synchronization Properties
 *
 * Description: * /test/: If true then the synchronization functions only test whether things are synchronized and return
 *              success if so, or fail with SS_MINOR_SKIPPED.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Interface initializer
 *
 * Description: Initializes the scope interface.
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
ss_scope_init(void)
{
    SS_ENTER_INIT;
    if (ss_scopetab_init()<0) SS_ERROR(INIT);
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Creates/reads the top-level scope in a file
 *
 * Description: This function either creates a new top-level scope in the specified file by creating the group that holds it
 *              and populating that group with the required tables, or by opening the group and its datasets.  Booting the top
 *              scope is essentially the same as booting any other scope except we have to read the ss_scope_t and ss_file_t
 *              tables. We also have to do some fancy footwork because ss_scope_boot() needs a scope object, which we don't
 *              have until we've at least partially booted the top scope.
 *
 * Return:      On success, returns true (positive) if the top-level scope had to be created and false otherwise; returns
 *              negative on failure.
 *
 * Parallel:    Collective across the file's communicator since we're doing file-collective HDF5 operations.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, August  5, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_scope_boot_top(size_t gfileidx,      /* Eventual index into the GFile array for FID */
                  MPI_Comm filecomm,    /* MPI communicator for the top-level scope, the file communicator */
                  hbool_t comm_duped    /* True if SSlib has duplicated the communicator for this scope */
                  )
{
    SS_ENTER(ss_scope_boot_top, htri_t);
    ss_scope_t          *topscope=NULL;
    ss_scopeobj_t       *topscopeobj=NULL;
    ss_table_t          *scopetab=NULL;
    hid_t               gid=-1, versgrp=-1;
    ss_gfile_t          *gfile=NULL;
    size_t              idx;
    const char          *topname = "SAF";
    char                version[256];
    int                 i;
    ss_pers_class_t     *pc=NULL;
    hbool_t             creating=FALSE;

    /* This could be a transient file, in which case we *must* be creating the top scope. */
    if (NULL==(gfile=SS_GFILE_IDX(gfileidx))) SS_ERROR(FAILED);

    /* Attempt to open the top scope. If it isn't present then we must be creating the file. */
    if (gfile->flags & H5F_ACC_TRANSIENT) {
        gid = 1;
        creating = TRUE;
    } else if ((gid=H5Gopen(gfile->fid, topname))<0) {
        SS_STATUS_OK;
        if ((gid=H5Gcreate(gfile->fid, topname, 0))<0) SS_ERROR(HDF5);
        if (ss_scope_setversion(gid)<0) SS_ERROR(HDF5);
        creating = TRUE;
    }

    /* Create the group that holds the shared datatypes for the various tables and populate it with those types. Since each
     * scope has the same tables and the HDF5 object header messages to describe those types are potentially thousands of
     * bytes, we can save substantial space in the file by using shared datatypes. However, since every version of SAF might
     * have different datatypes and since every file might have scopes created by different versions of the library, we make
     * one group for each library version. */
    if (gid>1) {
        sprintf(version, "%d.%d.%d", SS_VERS_MAJOR, SS_VERS_MINOR, SS_VERS_RELEASE);
        if (SS_VERS_ANNOT && *SS_VERS_ANNOT) {
            SS_ASSERT(strlen(version)+1+strlen(SS_VERS_ANNOT)+1<sizeof(version));
            strcat(version, "-");
            strcat(version, SS_VERS_ANNOT);
        }
        if ((versgrp=H5Gopen(gid, version))<0) {
            /* Create datatypes, but only if the file is open for writing. If the file was created by a different version of
             * SSlib and open for read-only then we won't be creating datasets that need to share these types anyway. */      
            SS_STATUS_OK;
            if (creating) {
                if ((versgrp=H5Gcreate(gid, version, 0))<0) SS_ERROR(HDF5);
                for (i=0; i<SS_PERS_NCLASSES; i++) {
                    if (NULL==(pc=SS_PERS_CLASS(i))) continue;
                    gfile->tff[i] = H5Tcopy(pc->tff); /*because pc->tff is immutable*/
                    if (H5Tcommit(versgrp, pc->name, gfile->tff[i])<0) SS_ERROR(HDF5);
                }
            }
        } else {
            /* Open the datatypes */
            for (i=0; i<SS_PERS_NCLASSES; i++) {
                if (NULL==(pc=SS_PERS_CLASS(i))) continue;
                if ((gfile->tff[i]=H5Topen(versgrp, pc->name))<0) SS_ERROR(HDF5);
                SS_ASSERT(H5Tequal(gfile->tff[i], pc->tff));
            }
        }
    }

    /* Create a scope table in memory (or use an existing one). We have to do this before we create the scope itself because
     * this table will serve as the storage for the scope object. */
    if ((topscope=gfile->topscope)) {
        topscopeobj = SS_SCOPE(topscope);
        scopetab = topscopeobj->m.table[SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))];
    }
    scopetab = ss_table_new(SS_MAGIC(ss_scope_t), gid, creating, gfile->tff, scopetab, NULL);

    /* Create the top-scope object. We don't have to create the strings table until we perform table I/O operations, which
     * allows us to set the scope name without needing the strings table in memory (strings table is read by ss_scope_boot()).
     * A new scope is created if the table is empty, which might be the case even if the gfile->topscope exists (consider what
     * happens when a file is created, then closed, then recreated).  Even when we're opening an existing file for read-only
     * we create the topscope object because it's needed by ss_scope_boot(), but we'll later overwrite that temporary
     * top-scope when we actually read the scope table. */
    if (scopetab->nperm>0) {
        SS_ASSERT(gfile->topscope);
        SS_ASSERT(topscopeobj==(ss_scopeobj_t*)ss_table_lookup(scopetab, idx=0, SS_STRICT));
    } else {
        if (NULL==(topscopeobj=(ss_scopeobj_t*)ss_table_newobj(scopetab, SS_TABLE_DIRECT, NULL, &idx))) SS_ERROR(FAILED);
        SS_ASSERT(0==idx); /*entry zero of the top scope's scope table always points to the top scope itself*/

        /* We had to create the object with ss_table_newobj() instead of ss_pers_new() since the later takes a containing
         * scope argument which we don't have yet (that's what we're in the process of creating). Therefore we must manually
         * set the `dirty' and `sync' members of the object. */
        topscopeobj->m.pers.dirty = TRUE;
        topscopeobj->m.pers.synced = SS_ALLSAME;

        /* Set the scope name */
        if (ss_string_set(&(topscopeobj->name), topname)<0) SS_ERROR(FAILED);

        /* The top scope has a scope table */
        topscopeobj->m.table[SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))] = scopetab;

        /* Make sure the GFile entry points to the top scope. */
        if (NULL==(topscope=(ss_scope_t*)ss_pers_refer_topscope(gfileidx, (ss_persobj_t*)topscopeobj))) SS_ERROR(FAILED);
        gfile->topscope = topscope;
    }

    /* Boot the scope by opening the group and datasets and reading the variable length string data */
    if (ss_scope_boot(topscope, gid, filecomm, creating)<0) SS_ERROR(FAILED);

    /* Read the scope, which only top scopes have, if the file exists. We've already created the table above and initialized
     * the first entry in the table in order to call ss_scope_boot() on that entry. Now we'll read the actual file data, which
     * clobbers the persistent part that we initialized and clears the dirty bit and sets the synchronization bit. */
    if (!creating) {
        if (ss_table_read(scopetab, topscope)<0) SS_ERROR(FAILED);
        if (NULL==ss_table_lookup(scopetab, (size_t)0, SS_STRICT))
            SS_ERROR_FMT(CORRUPT, ("scope table appears to be corrupt"));
        SS_ASSERT(topscopeobj==(ss_scopeobj_t*)ss_table_lookup(scopetab, (size_t)0, SS_STRICT));
        SS_ASSERT(topscopeobj==SS_SCOPE(topscope));
        SS_ASSERT(topscopeobj==SS_SCOPE(gfile->topscope));
        if (ss_string_cmp_s(&(topscopeobj->name), topname))
            SS_ERROR_FMT(CORRUPT, ("incorrect top scope name in scope table: %s", ss_string_ptr(&(topscopeobj->name))));
    }
    
    /* Successful cleanup */
    if (versgrp>0 && H5Gclose(versgrp)<0) SS_ERROR(HDF5);
    versgrp = -1;

 SS_CLEANUP:
    if (topscope) {
        if (scopetab && NULL==topscopeobj->m.table[SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))]) ss_table_dest(scopetab, SS_STRICT);
        gfile = SS_GFILE_LINK(topscope);
        ss_scope_dest(topscope);
        if (gfile) gfile->topscope = topscope = SS_FREE(topscope);
    } else if (topscopeobj) {
        ss_scope_dest_(topscopeobj, SS_STRICT);
    }
    if (versgrp>0) H5Gclose(versgrp);

    SS_LEAVE(creating);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Creates/reads a scope
 *
 * Description: This function creates or reads a scope during the boot phase but does not open the scope. It just opens the
 *              scope's group and the datasets therein.  The ss_scope_boot_top() function should be used to open the top scope
 *              of a file.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the file communicator since we're doing file-collective HDF5 operations.
 *
 * Programmer:  Robb Matzke
 *              Saturday, December 13, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_boot(ss_scope_t *scope,                /* The scope to be booted */
              hid_t gid,                        /* Optional group that already exists */
              MPI_Comm filecomm,                /* The file communicator */
              hbool_t create                    /* Create the scope or open an existing scope? */
              )
{
    
    SS_ENTER(ss_scope_boot, herr_t);
    ss_gfile_t          *gfile=NULL;
    const char          *scopename=NULL;
    ss_string_table_t   *strings=NULL;
    unsigned            tableidx;
    ss_table_t          *table=NULL;

    SS_ASSERT_MEM(scope, ss_scope_t);
    if (NULL==(gfile=SS_GFILE_LINK(scope))) SS_ERROR(NOTFOUND);
    if (NULL==(scopename=ss_string_ptr(SS_SCOPE_P(scope,name)))) SS_ERROR(FAILED);

    /* Open or create the scope group unless the caller supplied an open group */
    if (gid<=0) {
        if (!create) {
            if ((gid=H5Gopen(gfile->fid, scopename))<0) SS_ERROR_FMT(HDF5, ("scope=\"%s\"", scopename));
        } else if (gfile->flags & H5F_ACC_TRANSIENT) {
            gid = 1;
        } else {
            if ((gid=H5Gcreate(gfile->fid, scopename, 0))<0) SS_ERROR(HDF5);
            if (ss_scope_setversion(gid)<0) SS_ERROR_FMT(HDF5, ("scope=\"%s\"", scopename));
        }
    }
    
    /* Create or read the variable length strings for this scope. */
    strings = SS_SCOPE(scope)->m.strings;
    if (NULL==(strings=ss_string_boot(gid, filecomm, create, strings)))
        SS_ERROR_FMT(FAILED, ("scope=\"%s\"", scopename));

    /* Create or open tables (except ss_scope_t which belongs only to top scopes) */
    for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
        if (!ss_pers_class_g[tableidx].name) continue;                          /*skip unused slot*/
        if (tableidx==SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))) continue;        /*skip scope table*/
        table = SS_SCOPE(scope)->m.table[tableidx];
        if (NULL==(table=ss_table_new(tableidx, gid, create, gfile->tff, table, NULL)))
            SS_ERROR_FMT(FAILED, ("scope=\"%s\"", scopename));
        SS_SCOPE(scope)->m.table[tableidx] = table;
    }

    /* Update scope data */
    SS_SCOPE(scope)->m.gid = gid;
    SS_SCOPE(scope)->m.strings = strings;
    SS_SCOPE(scope)->m.comm = SS_COMM_NULL; /*scope is not open yet*/
    SS_SCOPE(scope)->m.comm_duped = FALSE;

SS_CLEANUP:
    /* If there's an error we haven't actually modified the scope object (except possibly the m.table array), therefore all we
     * need to clean up are some local variables. */
    if (gid>1) H5Gclose(gid);
    ss_string_desttab(strings);
    for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
        if (!ss_pers_class_g[tableidx].name) continue; /*unused slot*/
        if (tableidx!=SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t)) && SS_SCOPE(scope)->m.table[tableidx]) {
            ss_table_dest(SS_SCOPE(scope)->m.table[tableidx], SS_STRICT);
            SS_SCOPE(scope)->m.table[tableidx] = NULL;
        }
    }

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Scope destructor
 *
 * Description: This function destroys all memory associated with a scope.  This function is usually called for error cleanup
 *              or when the library is exiting. It is much more destructive than ss_scope_close().
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, August  5, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_dest(ss_scope_t *scope)
{
    SS_ENTER(ss_scope_dest, herr_t);
    ss_scopeobj_t *scopeobj;

    SS_ASSERT_TYPE(scope, ss_scope_t);
    scopeobj = SS_SCOPE(scope);
    SS_ASSERT(scopeobj);

    /* Destroy the scope and the link */
    if (ss_scope_dest_(scopeobj, SS_STRICT)<0) SS_ERROR(FAILED);
    if (ss_pers_dest((ss_pers_t*)scope)) SS_ERROR(FAILED);

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Scope destructor
 *
 * Description: This is the internal version of ss_scope_dest(). It takes a pointer to the scope object instead of a scope link.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent.
 *
 * Issue:       In order for this function to be independent it foregoes closing the HDF5 group associated with the scope.
 *              This isn't really a problem though because HDF5 will close the group when the file is closed, which is a
 *              collective operation across the file's communicator.
 *
 * Programmer:  Robb Matzke
 *              Thursday, August 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_dest_(ss_scopeobj_t *scopeobj,
               unsigned flags                   /* If the SS_STRICT bit is set then the same is passed down to ss_table_dest()
                                                 * and all scope and table related memory is freed. Otherwise the tables are
                                                 * only partially destroyed and the scope object continues to exist. */
               )
{
    SS_ENTER(ss_scope_dest_, herr_t);
    unsigned            tableidx;
    ss_table_t          *scopetab=NULL;
    ss_table_t          *table=NULL;

    SS_ASSERT_TYPE(scopeobj, ss_scopeobj_t);

    /* Destroy things that were set when the scope was opened */
    if (scopeobj->m.comm != SS_COMM_NULL) {
        if (scopeobj->m.comm_duped) ss_mpi_comm_free(&(scopeobj->m.comm));
        scopeobj->m.comm = SS_COMM_NULL;
    }
    scopeobj->m.comm_duped = FALSE;

    /* Destroy things that were set when the scope was booted */
    if (flags & SS_STRICT) {
        if (ss_string_desttab(scopeobj->m.strings)<0) SS_ERROR(FAILED);
        scopeobj->m.strings = NULL;
    }
    for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
        if (tableidx!=SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))) {
            table = scopeobj->m.table[tableidx];
            if (table && ss_table_dest(table, flags)<0) SS_ERROR(FAILED);
            if (flags & SS_STRICT) scopeobj->m.table[tableidx] = NULL;
        }
    }
    scopeobj->m.gid = 0; /* we just discard it; HDF5 will close it when the file is closed */

    /* Destroy persistent things in the scope */
    if (flags & SS_STRICT) ss_string_reset(&(scopeobj->name));

    /* Now we can finally destroy the scope table itself */
    /* ISSUE: This function makes no attempt to destroy scopes in this scope's scope table. */
    if (flags & SS_STRICT) {
        scopetab = scopeobj->m.table[SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))];
        scopeobj->m.table[SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))] = NULL;
        if (scopetab && ss_table_dest(scopetab, SS_STRICT)<0) SS_ERROR(FAILED);
    }
    
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Opens a scope
 *
 * Description: Given a link to a scope (i.e., a link to an entry in the top-level /Scopes/ table of some file that is
 *              currently open), open the scope. The FLAGS argument determines the mode for opening the scope.  The following
 *              bits are supported at this time:
 *              
 *              * H5F_ACC_RDONLY: The scope is opened for read-only access.
 *              * H5F_ACC_RDWR:   The scope is opened for both read and write access.
 *              * H5F_ACC_DEBUG:  Turn on scope debugging.
 *
 *              The H5F_ACC_EXCL, H5F_ACC_TRUNC, and H5F_ACC_CREAT bits are not supported by this function because they
 *              require participation of all tasks in the file's communicator, and therefore SSlib separates scope creation
 *              from scope opening.
 *
 *              The H5F_ACC_RDWR flag can only be used if the containing file is also H5F_ACC_RDWR.
 *
 *              The scope will be a transient scope if and only if the file was opened with H5F_ACC_TRANSIENT. Therefore this
 *              function simply ignores that bit in the FLAGS vector.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 *              It is an error to open a scope that is already open.  However, since the original open might have been
 *              performed on a disjoint subset of tasks, the current operation might not be able to detect a duplicate open.
 *              If disjoint sets of tasks open the same scope for read-only access and no task has the scope open for writing
 *              then things will most likely work properly.
 *
 * Parallel:    Collective across the scope's communicator, PROPS.comm, defaulting to the same communicator as the file in
 *              which the scope exists.
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_open(ss_scope_t *scope,        /* A link to a scope object, probably the result of a /find/ operation. */
              unsigned flags,           /* Various bit flags to control common scope open switches. */
              ss_prop_t *props          /* Scope opening properties (see Scope Properties). */
              )
{
    SS_ENTER(ss_scope_open, herr_t);
    MPI_Comm            comm=SS_COMM_NULL;
    hbool_t             comm_duped=FALSE;
    ss_gfile_t          *gfile=NULL;
    unsigned            tableidx;
    ss_table_t          *table=NULL;
    const char          *scopename=NULL;

    SS_ASSERT_MEM(scope, ss_scope_t);
    if (NULL==(scopename = ss_string_ptr(SS_SCOPE_P(scope,name)))) scopename="NONAME";


    /* Check that the scope is not already open. This will only catch an error if this task had the scope open, but
     * quite often it is possible that the scope was opened only on some other task(s). The usual mistake is that all tasks
     * open the file with MPI_COMM_SELF, in which case synchronizations don't work properly. */
    if (SS_SCOPE(scope)->m.comm != SS_COMM_NULL) SS_ERROR_FMT(PERM, ("already open: %s", scopename));

#ifndef NDEBUG
    {
        /* These things should have been set when the scope was booted. */
        SS_ASSERT(SS_SCOPE(scope)->m.gid>0);
        SS_ASSERT(SS_SCOPE(scope)->m.strings);
        for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
            if (tableidx!=SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t)) && NULL!=SS_PERS_CLASS(tableidx)) {
                SS_ASSERT(SS_SCOPE(scope)->m.table[tableidx]);
            }
        }
    }
#endif
    
    /* Check FLAGS compatibility with the file */
    if (NULL==(gfile=SS_GFILE_LINK(scope)) || !gfile->topscope) SS_ERROR(NOTFOUND);
    SS_ASSERT(gfile && gfile->topscope);
    flags &= (H5F_ACC_RDWR|H5F_ACC_DEBUG); /*weed out flags except those we support*/
    flags |= (gfile->flags & H5F_ACC_TRANSIENT);
    if ((flags & H5F_ACC_RDWR) && 0==(gfile->flags & H5F_ACC_RDWR))
        SS_ERROR_FMT(PERM, ("scope cannot be writable in a read-only file: %s", scopename));
        
    /* Obtain the communicator */
    if (!props || NULL==ss_prop_get(props, "comm", H5T_NATIVE_MPI_COMM, &comm)) {
        SS_STATUS_OK;
        comm = SS_SCOPE(gfile->topscope)->m.comm;
        comm_duped = FALSE;
    } else if (NULL==ss_prop_get(props, "duped", H5T_NATIVE_HBOOL, &comm_duped)) {
        SS_STATUS_OK;
        comm_duped = FALSE;
    }

    /* Read all the table data for this scope. We could have delayed this but all tasks will need to have the data anyway when
     * they synchronize, which will almost certainly happen before the file is closed.  We also have the problem that MPI-IO
     * doesn't guarantee that a reader task will see data that was recently written by some other task (some file systems such
     * as POSIX do in fact garantee this, while others such as PVFS don't).  If the file is read only, then we could easily
     * delay the reading of the table until it's actually needed, but even then how often really is it that a scope will be
     * opened on multiple tasks but some table not ever accessed? */
    for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
        if (NULL==SS_PERS_CLASS(tableidx)) continue;
        if (NULL==(table=ss_scope_table(scope, tableidx, NULL))) SS_ERROR_FMT(FAILED, ("scope=\"%s\"", scopename));
        if (ss_table_read(table, scope)<0) SS_ERROR_FMT(FAILED, ("scope=\"%s\"", scopename));
    }
    
    /* Assign the communicator to the scope to mark it as being opened */
    SS_SCOPE(scope)->m.comm = comm;
    SS_SCOPE(scope)->m.comm_duped = comm_duped;
    SS_SCOPE(scope)->m.flags = flags;

 SS_CLEANUP:
    if (comm_duped && comm!=SS_COMM_NULL) ss_mpi_comm_free(&comm);

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Closes a scope
 *
 * Description: Closes the specified scope without totally destroying the memory representation. Specifically, the top-scope's
 *              Scope table is left intact as are the indirect map arrays in all scopes that have them. This is required for
 *              the following common scenario:
 *
 *              The application has two files called FILE:File-A and FILE:File-B. The application creates a new object (e.g., a
 *              quantity) in FILE:File-A without using the SS_ALLSAME flag and then creates another object (e.g., a unit) in
 *              FILE:File-B that refers to the object in FILE:File-A. The application closes FILE:File-A which closes all the
 *              scopes in that file. It then attempts to close FILE:File-B, which includes a synchronization and a flush.
 *              However, when flushing, SSlib will need to convert a persistent object link from the Memory state to the
 *              Closed state and convert its indirect object index to a direct object index. The only way this can be done is
 *              by having the indirect mappings for the table that contained the object in FILE:File-A.
 *
 *              The scope is assumed to already be synchronized and flushed. In fact, it would not even be possible to flush
 *              the scope from this function because doing so may require a call to H5Dextend(), which is file collective.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the scope's communicator.  This is the same set of tasks that originally opened the scope.
 *
 * Also:        ss_scope_dest()
 *              ss_scope_dest_()
 *
 * Programmer:  Robb Matzke
 *              Wednesday, January  7, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_close(ss_scope_t *scope)
{
    SS_ENTER(ss_scope_close, herr_t);
    hid_t               scope_grp;
    unsigned            tableidx;
    ss_table_t          *table=NULL;
    const char          *scopename=NULL;

    SS_ASSERT_MEM(scope, ss_scope_t);
    if (NULL==(scopename=ss_string_ptr(SS_SCOPE_P(scope,name)))) scopename="NONAME";
    if ((scope_grp = ss_scope_isopen(scope))<0) SS_ERROR_FMT(FAILED, ("scope=\"%s\"", scopename));
    if (!scope_grp) SS_ERROR_FMT(NOTFOUND, ("scope is not open: %s", scopename));

    /* Should it be possible to close the top scope? Probably only from ss_file_close() and similar! */

    /* Close down the tables (except scope table if any) */
    for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
        table = SS_SCOPE(scope)->m.table[tableidx];
        if (!table || tableidx==SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t))) continue;
        if (ss_table_dest(table, 0)<0) SS_ERROR_FMT(FAILED, ("scope=\"%s\"", scopename)); /* only partially destroyed */
    }

    /* Reset the communicator */
    if (SS_SCOPE(scope)->m.comm_duped && ss_mpi_comm_free(SS_SCOPE_P(scope,m.comm))<0)
        SS_ERROR_FMT(MPI, ("scope=\"%s\"", scopename));
    SS_SCOPE(scope)->m.comm = SS_COMM_NULL;
    SS_SCOPE(scope)->m.comm_duped = FALSE;
    SS_SCOPE(scope)->m.flags = 0;

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Query scope open status
 *
 * Description: A scope is either opened or closed at any given time on any given task. This function returns that status.
 *
 * Return:      Returns true (a positive HDF5 group handle) if SCOPE is currently opened by the calling task; false if SCOPE
 *              is currently closed; negative on error.  The group handle is not duplicated and the caller should not invoke
 *              H5Gclose() on the return value.  This function returns an error (instead of false) if the scope is not even
 *              booted. To be "booted" means the scope object corresponds to some HDF5 group which is open.
 *
 * Parallel:    Independent
 *
 * Issues:      The group handle return value is not duplicated by this function because (1) not doing so is consistent with
 *              ss_file_isopen() and (2) doing so would require this function to be collective.
 *
 *              Since transient files are not supported by HDF5 there can be no HDF5 file handle for a scope created in a
 *              transient file. This function returns the integer 1 for such files, which is a positive true value
 *              but which is not a valid HDF5 group handle (or any valid handle for that matter).
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
ss_scope_isopen(ss_scope_t *scope       /* A link to a scope object. */
                )
{
    SS_ENTER(ss_scope_isopen, hid_t);
    hid_t       retval=0;

    SS_ASSERT_MEM(scope, ss_scope_t);
    if (SS_SCOPE(scope)->m.gid<=0) SS_ERROR_FMT(CORRUPT, ("scope does not correspond to an open HDF5 group"));
    if (SS_SCOPE(scope)->m.comm!=SS_COMM_NULL) retval = SS_SCOPE(scope)->m.gid;
    
 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Determines if scope is an open top-scope
 *
 * Description: Determines if SCOPE is open and a top-scope. A scope can be in an opened or closed state. Every file has
 *              exactly one top-level scope which is the first entry in that scope's Scope table.
 *
 * Return:      Returns true (positive) if SCOPE is open and a top-scope or false if not. Returns negative on failure.
 *
 * Parallel:    Independent
 *
 * Issue:       It might be better to just look to see if the specified scope has a communicator other than MPI_COMM_NULL and
 *              has a non-null pointer for a Scope table since only top-level scopes have a Scope table.
 *
 * Programmer:  Robb Matzke
 *              Thursday, August 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_scope_isopentop(ss_scope_t *scope)
{
    SS_ENTER(ss_scope_isopentop, htri_t);
    ss_gfile_t          *gfile=NULL;
    htri_t              retval=FALSE;

    SS_ASSERT_TYPE(scope, ss_scope_t);

    if (NULL==SS_SCOPE(scope)) goto done;
    if (NULL==(gfile=SS_GFILE_LINK(scope))) goto done;
    if (gfile->cur_open<=0) goto done; /*file is closed*/
    if (!SS_SCOPE(scope)) goto done;
    if ((retval = SS_PERS_EQ(scope, gfile->topscope))<0) SS_ERROR(FAILED);

done:
    SS_STATUS_OK;
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Tests transient state of a scope
 *
 * Description: This function tests whether SCOPE is a transient scope. A scope is transient if it belongs to a transient file.
 *
 * Return:      Returns true (positive) if SCOPE is a transient scope; false if SCOPE is a permanent scope; negative on error. It
 *              is an error to query a scope which isn't open.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_scope_istransient(ss_scope_t *scope  /* Any open scope. */
                     )
{
    SS_ENTER(ss_scope_istransient, htri_t);
    hid_t       scopegrp = ss_scope_isopen(scope);
    htri_t      retval=-1;

    if (scopegrp<0) SS_ERROR(FAILED);
    retval = (1==scopegrp);
 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Tests whether scope can be modified
 *
 * Description: If a scope is opened for read-only access then the objects in that scope cannot be modified. This function
 *              tests for that condition.
 *
 * Return:      Returns true (positive) if SCOPE is open with write access (that is, the ss_scope_open() call was passed the
 *              H5F_ACC_RDWR flag); returns false if the scope is opened for read-only access; returns negative on failures.
 *              It is considered a failure if SCOPE is not open on the calling task.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, January 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_scope_iswritable(ss_scope_t *scope   /* Any open scope. */
                    )
{
    SS_ENTER(ss_scope_iswritable, htri_t);
    htri_t      retval=FALSE;
    ss_gfile_t  *gfile=NULL;

    /* Check whether the H5F_ACC_RDWR bit is set on the file. If the file is read-only then all scopes in the file are also
     * read-only. This allows the client to create a transient scope (e.g., to serve as a standard registry) and then mark the
     * entire file as read-only in order to prevent modification of it's objects and to make the library more efficient (there
     * are certain time-saving assumptions that can be made if a file is read-only). */
    if (NULL==(gfile=SS_GFILE_LINK(scope))) SS_ERROR(NOTFOUND);
    if (gfile->flags & H5F_ACC_RDWR) {
        /* If the file is writable then check the scope's flags. */
        if (ss_scope_isopen(scope)<=0) SS_ERROR(NOTOPEN);
        if (SS_SCOPE(scope)->m.flags & H5F_ACC_RDWR) retval=TRUE;
    }

SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Synchronize a scope
 *
 * Description: Various scope modifying operations that would normally be collective across the scope's communicator can be
 *              carried out locally. When this happens the various tasks of the scope's communicator may store differing
 *              information for the scope.  This function is intended to synchronize the various tables of a particular scope
 *              across all the MPI tasks that "own" that scope. See [Synchronization Algorithm] for more details.
 *
 * Return:      Returns non-negative on success; negative on failure. It is an error to attempt to synchronize a scope that is
 *              not open.
 *
 * Parallel:    Collective across the scope's communicator.  Substantial communication may be required.
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_synchronize(ss_scope_t *scope,         /* A link to an open scope that should be synchronized. */
                     unsigned tableidx,         /* Magic number to define which table to synchronize. If TABLEIDX is
                                                 * SS_TABLE_ALL then all tables of the scope will be synchronized. */
                     ss_prop_t *props           /* See [Synchronization Properties] */
                     )
{
    SS_ENTER(ss_scope_synchronize, herr_t);
    ss_table_t          *table=NULL;
    const char          *scopename=NULL;

    SS_ASSERT_MEM(scope, ss_scope_t);
    tableidx = SS_MAGIC_SEQUENCE(tableidx);

    /* If the scope is open for read-only access then there should be nothing to do. However, if SSlib was compiled with
     * debugging support then we should descend into the table synchronization anyway in order to check that the client didn't
     * accidently modify an object. */
#ifdef NDEBUG
    if (!ss_scope_iswritable(scope)) goto done;
#endif

    if (NULL==(scopename = ss_string_ptr(SS_SCOPE_P(scope,name)))) SS_ERROR(FAILED);
    if (SS_TABLE_ALL==tableidx) {
        for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
            if (!SS_PERS_CLASS(tableidx)) continue;
            if (NULL==(table=ss_scope_table(scope, tableidx, NULL))) SS_ERROR(FAILED);
            if (ss_table_synchronize(table, scope, props)<0)
                SS_ERROR_FMT(FAILED, ("table %u of scope \"%s\"", tableidx, scopename));
        }
    } else {
        if (!SS_PERS_CLASS(tableidx)) SS_ERROR(NOTFOUND);
        if (NULL==(table=ss_scope_table(scope, tableidx, NULL))) SS_ERROR(FAILED);
        if (ss_table_synchronize(table, scope, props)<0)
            SS_ERROR_FMT(FAILED, ("%s table(%u) of scope \"%s\"", SS_PERS_CLASS(tableidx)->name, tableidx, scopename));
    }

#ifdef NDEBUG
done:
#endif
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Callback to synchronize scopes
 *
 * Description: This function is a callback for ss_table_scan() to flush each scope of a scope table.
 *
 * Return:      Returns FALSE on success; negative on failure.
 *
 * Parallel:    Collective across the communicator of the file to which the scope belongs.  Since only the top scope has a
 *              scope table and the top scope is opened collectively across the file communicator, all file tasks have the
 *              same information about the scopes.
 *
 * Programmer:  Robb Matzke
 *              Monday, November 17, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
htri_t
ss_scope_synchronize_cb(size_t UNUSED itemidx,          /* Item index for _SCOPE in the top-scope scope table */
                        ss_pers_t *_scope,              /* Link to the scope being synchronized */
                        ss_persobj_t UNUSED *_scopeobj, /* The scope object being synchronized; _SCOPE points to this. */
                        void *_sync_data                /* Stuff passed through ss_table_scan() */
                        )
{
    SS_ENTER(ss_scope_synchronize_cb, htri_t);
    ss_scope_t *scope = (ss_scope_t*)_scope;
    ss_scope_sync_t *sync_data = (ss_scope_sync_t*)_sync_data;
    int err_newptrs;

    /* This function is file-collective but ss_scope_synchronize() is scope collective. */
    if (ss_scope_isopen(scope)) {
        if (ss_scope_synchronize(scope, sync_data->tableidx, sync_data->props)<0) {
            SS_SAVE;
            if ((err_newptrs=ss_prop_get_i(sync_data->props, "err_newptrs"))>0) {
                SS_STATUS_OK;
                sync_data->transient_errors += err_newptrs;
            } else {
                SS_REFAIL;
                SS_ERROR(FAILED);
            }
        }
    }
    
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Query scope synchronization state
 *
 * Description: As detailed in ss_scope_synchronize(), scopes may become out of sync when tasks independently modify table
 *              entries.  This function will query whether a scope (or some table of the scope) is out of sync without
 *              synchronizing the scope. In fact, even when the scope is in a synchronized state, calling this function may be
 *              faster than calling ss_scope_synchronize().
 *
 * Return:      Returns true (positive) if the scope is in a synchronized state; false if synchronization is necessary;
 *              negative on error.  It is an error to make this query about a scope which is not open.
 *
 * Parallel:    Collective across a superset of the scope's communicator. Communication is required within the scope
 *              communicator and other tasks will return true (positive) because the scope is not open there.
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_scope_synchronized(ss_scope_t *scope,        /* A link to the scope whose synchronization state is to be queried. */
                      unsigned tableidx         /* Magic number to define which table to query. If TABLEIDX is greater than or
                                                 * equal to SS_NPERSL_CLASSES then all tables of the specified scope must be
                                                 * in a synchronized state before this function returns true. */
                      )
{
    SS_ENTER(ss_scope_synchronized, htri_t);
    ss_prop_t           *props=NULL;
    htri_t              retval=TRUE;

    /* Build a synchronization property that says to only test synchronization */
    if (NULL==(props=ss_prop_new("test synchronization"))) SS_ERROR(FAILED);
    if (ss_prop_add(props, "test", H5T_NATIVE_HBOOL, &true)<0) SS_ERROR(FAILED);

    /* Test synchronization */
    if (ss_scope_synchronize(scope, tableidx, props)<0) {
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
 * Chapter:     Scopes
 * Purpose:     Write pending data to file
 *
 * Description: Flushing a scope causes all of its tables to be written to HDF5. It does not include synchronizing the tables
 *              or telling HDF5 to flush its cached data to the file or telling the operating system to flush dirty blocks to
 *              disk. That can be done with code similar to the following:
 *
 *                  ss_scope_flush(scope, SS_MAGIC(ss_field_t), properties);
 *                  H5Fflush(ss_scope_isopen(scope), H5F_SCOPE_GLOBAL); // flushes the whole hdf5 file and all mounts
 *
 * Return:      Returns non-negative on success, negative on failure.
 *
 * Parallel:    Conceptually this function is collective across the scope's communicator, however because ss_table_write() and
 *              ss_string_flush() are file-collective due to HDF5 API restrictions this function must also be file collective.
 *              Fortunately the SCOPE argument is available on all tasks of the file which makes this restriction easy to
 *              program around.
 *
 * Issue:       When flushing a specific table the variable length string values are not written to the file.
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_flush(ss_scope_t *scope,               /* A link to the open scope to be flushed. */
               unsigned tableidx,               /* Magic number to define which table to flush, or SS_TABLE_ALL */
               ss_prop_t UNUSED *props          /* Scope flushing properties (none defined yet) */
               )
{
    SS_ENTER(ss_scope_flush, herr_t);
    ss_table_t          *table=NULL;
    ss_scope_t          topscope = SS_SCOPE_NULL;
    const char          *scopename=NULL;

    SS_ASSERT_MEM(scope, ss_scope_t);
    tableidx = SS_MAGIC_SEQUENCE(tableidx);

    if (NULL==(scopename=ss_string_ptr(SS_SCOPE_P(scope,name)))) SS_ERROR(FAILED);
    if (SS_TABLE_ALL==tableidx) {
        /* Flush each of the object tables */
        for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
            if (!SS_PERS_CLASS(tableidx)) continue;
            if (NULL==(table=ss_scope_table(scope, tableidx, NULL))) SS_ERROR(FAILED);
            if (ss_table_write(table, scope)<0) SS_ERROR_FMT(FAILED, ("table %u of scope \"%s\"", tableidx, scopename));
        }
        /* Now write the variable length strings */
        if (NULL==ss_pers_topscope((ss_pers_t*)scope, &topscope)) SS_ERROR(FAILED);
        if (ss_string_flush(SS_SCOPE(scope)->m.strings, SS_SCOPE(scope)->m.comm, SS_SCOPE(&topscope)->m.comm)<0)
            SS_ERROR_FMT(FAILED, ("strings table of scope \"%s\"", scopename));
    } else {
        if (!SS_PERS_CLASS(tableidx)) SS_ERROR(NOTFOUND);
        if (NULL==(table=ss_scope_table(scope, tableidx, NULL))) SS_ERROR(FAILED);
        if (ss_table_write(table, scope)<0) SS_ERROR_FMT(FAILED, ("table %u of scope \"%s\"", tableidx, scopename));
    }
    
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Obtain a table from a scope
 *
 * Description: Given an open scope, return one of the tables associated with that scope. If the TABLE pointer is non-null
 *              then this function also verifies that TABLE is a specific member of the scope (the one indicated by TABLEIDX)
 *              or any member of the scope (if TABLEIDX is equal to SS_TABLE_ALL).
 *
 * Return:      A pointer to a table on success; the null pointer on failure
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, July 31, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_table_t *
ss_scope_table(ss_scope_t *scope,               /* A link to the open scope for which we desire a table */
               unsigned tableidx,               /* A table number or persistent object or link magic number */
               ss_table_t *table                /* The null pointer or a table for which to verify existence */
               )
{
    SS_ENTER(ss_scope_table, ss_table_tP);
    ss_table_t          *retval=NULL;
    ss_table_t          *found=NULL;

    SS_ASSERT_MEM(scope, ss_scope_t);

    tableidx = SS_MAGIC_SEQUENCE(tableidx);
    if (table) {
        if (tableidx==SS_TABLE_ALL) {
            /* Check whether TABLE exists as any table of the scope */
            for (tableidx=0; tableidx<SS_PERS_NCLASSES; tableidx++) {
                if (SS_SCOPE(scope)->m.table[tableidx]==table) {
                    retval = table;
                    break;
                }
            }
            if (!retval) SS_ERROR(NOTFOUND);
        } else {
            /* Check whehter TABLE is a particular table of the scope */
            tableidx = SS_MAGIC_SEQUENCE(tableidx);
            if (NULL==(found=SS_SCOPE(scope)->m.table[tableidx]) || found!=table) SS_ERROR(NOTFOUND);
            retval = found;
        }
    } else {
        /* Return the specified table of the scope */
        SS_ASSERT(tableidx<SS_PERS_NCLASSES);
        if (NULL==(retval=SS_SCOPE(scope)->m.table[tableidx])) SS_ERROR(NOTFOUND);
    }

 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Scopes
 * Purpose:     Query the scope communicator
 *
 * Description: Given a scope, return the scope's communicator without dup'ing it.  This is either the scope's communicator or
 *              the communicator of the file to which the scope belongs.
 *
 * Return:      Returns non-negative on success, negative on failure.  The communicator is returned through the COMM argument
 *              when the function is successful.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, August 21, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_comm(ss_scope_t *scope,
              MPI_Comm *comm,           /* Optional returned communicator, not duplicated. */
              int *self,                /* Optional returned calling task's rank within communicator */
              int *ntasks               /* Optional returned size of communicator */
              )
{
    SS_ENTER(ss_scope_comm, herr_t);
    SS_ASSERT_MEM(scope, ss_scope_t);

    if (comm) *comm = SS_SCOPE(scope)->m.comm;
    if (self && (*self=ss_mpi_comm_rank(SS_SCOPE(scope)->m.comm))<0) SS_ERROR(FAILED);
    if (ntasks && (*ntasks=ss_mpi_comm_size(SS_SCOPE(scope)->m.comm))<0) SS_ERROR(FAILED);

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Callback to destroy scopes
 *
 * Description: This function is a callback for ss_table_scan() to partially destroy each scope (the SS_STRICT bit is not
 *              passed down to ss_scope_dest_() and therefore the /Scope/ table (if any) is not affected and the tables will
 *              maintain their `maptab' entries.
 *
 * Return:      Returns FALSE on success; negative on failure.
 *
 * Parallel:    Independent as long as ss_scope_dest_() is also independent.
 *
 * Programmer:  Robb Matzke
 *              Thursday, August 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
htri_t
ss_scope_dest_cb(size_t UNUSED itemidx, ss_pers_t UNUSED *_scope, ss_persobj_t *_scopeobj, void UNUSED *udata)
{
    SS_ENTER(ss_scope_dest_cb, htri_t);
    ss_scopeobj_t  *scopeobj = (ss_scopeobj_t*)_scopeobj;
    if (ss_scope_dest_(scopeobj, 0/*!SS_STRICT*/)<0) SS_ERROR(FAILED);
 SS_CLEANUP:
    SS_LEAVE(FALSE);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Callback to flush scopes
 *
 * Description: This function is a callback for ss_table_scan() to flush each scope of a scope table.
 *
 * Return:      Returns FALSE on success; negative on failure
 *
 * Parallel:    Collective across the communicator of the file to which the scope belongs.
 *
 * Programmer:  Robb Matzke
 *              Thursday, August 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
htri_t
ss_scope_flush_cb(size_t UNUSED itemidx, ss_pers_t *_scope, ss_persobj_t UNUSED *_scopeobj, void *_props)
{
    SS_ENTER(ss_scope_flush_cb, htri_t);
    ss_scope_t          *scope = (ss_scope_t*)_scope;

    if (ss_scope_flush(scope, SS_TABLE_ALL, _props)<0) SS_ERROR(FAILED);
 SS_CLEANUP:
    SS_LEAVE(FALSE);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Scopes
 * Purpose:     Set version information
 *
 * Description: Whenever a scope group is created we also create an attribute containing version information about the various
 *              software layers we know about. We allow each software layer to have a name (e.g., `saf', `hdf5', `mpi'), up to
 *              three integer version parts, and an annotation string.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across the file communicator to which GRP belongs.
 *
 * Programmer:  Robb Matzke
 *              Saturday, November 15, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_scope_setversion(hid_t grp           /* The HDF5 group representing the new scope */
                    )
{
    SS_ENTER(ss_scope_setversion, herr_t);
    hid_t       attr=-1;                /* Attribute to hold version info */
    hid_t       vinfo=-1;               /* The top level struct containing one member for each software layer */
    hid_t       saf_type=-1;            /* The SAF version info */
    hid_t       hdf_type=-1;            /* The HDF5 version info */
    hid_t       mpi_type=-1;            /* The MPI version info */
    hid_t       annot=-1;               /* Temporary type to hold a string */
    hid_t       scalar=-1;              /* Scalar data space */
    int         vno;                    /* Buffer for version number part */
    size_t      offset=0;               /* Running offset into `buffer' */
    size_t      saf_start;              /* Offset in `buffer' for start of SAF version info */
    size_t      hdf_start;              /* Offset in `buffer' for start of HDF5 version info */
    size_t      mpi_start;              /* Offset in `buffer' for start of MPI version info */
    char        buffer[1024];         /* Packed version info to be written to the attribute */
    hsize_t     one=1;

    /* SAF Version number */
    saf_start = offset;

    vno = SS_VERS_MAJOR;
    memcpy(buffer+offset, &vno, sizeof vno);
    offset += sizeof vno;
    
    vno = SS_VERS_MINOR;
    memcpy(buffer+offset, &vno, sizeof vno);
    offset += sizeof vno;

    vno = SS_VERS_RELEASE;
    memcpy(buffer+offset, &vno, sizeof vno);
    offset += sizeof vno;

    strcpy(buffer+offset, SS_VERS_ANNOT);
    offset += strlen(SS_VERS_ANNOT)+1;

    if ((saf_type = H5Tcreate(H5T_COMPOUND, offset-saf_start))<0) SS_ERROR(HDF5);
    if (H5Tinsert(saf_type, "major", 0, H5T_NATIVE_INT)<0) SS_ERROR(HDF5);
    if (H5Tinsert(saf_type, "minor", sizeof(vno), H5T_NATIVE_INT)<0) SS_ERROR(HDF5);
    if (H5Tinsert(saf_type, "release", 2*sizeof(vno), H5T_NATIVE_INT)<0) SS_ERROR(HDF5);
    if ((annot = H5Tcopy(H5T_C_S1))<0) SS_ERROR(HDF5);
    if (H5Tset_size(annot, strlen(SS_VERS_ANNOT)+1)<0) SS_ERROR(HDF5);
    if (H5Tinsert(saf_type, "annot", 3*sizeof(vno), annot)<0) SS_ERROR(HDF5);
    if (H5Tclose(annot)<0) SS_ERROR(HDF5);
    annot = -1;
    
    /* HDF5 Version number */
    hdf_start = offset;
    
    vno = H5_VERS_MAJOR;
    memcpy(buffer+offset, &vno, sizeof vno);
    offset += sizeof vno;
    
    vno = H5_VERS_MINOR;
    memcpy(buffer+offset, &vno, sizeof vno);
    offset += sizeof vno;

    vno = H5_VERS_RELEASE;
    memcpy(buffer+offset, &vno, sizeof vno);
    offset += sizeof vno;

    strcpy(buffer+offset, H5_VERS_SUBRELEASE);
    offset += strlen(H5_VERS_SUBRELEASE)+1;

    if ((hdf_type = H5Tcreate(H5T_COMPOUND, offset-hdf_start))<0) SS_ERROR(HDF5);
    if (H5Tinsert(hdf_type, "major", 0, H5T_NATIVE_INT)<0) SS_ERROR(HDF5);
    if (H5Tinsert(hdf_type, "minor", sizeof(vno), H5T_NATIVE_INT)<0) SS_ERROR(HDF5);
    if (H5Tinsert(hdf_type, "release", 2*sizeof(vno), H5T_NATIVE_INT)<0) SS_ERROR(HDF5);
    if ((annot = H5Tcopy(H5T_C_S1))<0) SS_ERROR(HDF5);
    if (H5Tset_size(annot, strlen(H5_VERS_SUBRELEASE)+1)<0) SS_ERROR(HDF5);
    if (H5Tinsert(hdf_type, "annot", 3*sizeof(vno), annot)<0) SS_ERROR(HDF5);
    if (H5Tclose(annot)<0) SS_ERROR(HDF5);
    annot = -1;

    /* MPI Version number */
    mpi_start = offset;
#ifdef HAVE_PARALLEL
    vno = MPI_VERSION;
    memcpy(buffer+offset, &vno, sizeof vno);
    offset += sizeof vno;
    
    vno = MPI_SUBVERSION;
    memcpy(buffer+offset, &vno, sizeof vno);
    offset += sizeof vno;

    if ((mpi_type = H5Tcreate(H5T_COMPOUND, offset-mpi_start))<0) SS_ERROR(HDF5);
    if (H5Tinsert(mpi_type, "major", 0, H5T_NATIVE_INT)<0) SS_ERROR(HDF5);
    if (H5Tinsert(mpi_type, "minor", sizeof(vno), H5T_NATIVE_INT)<0) SS_ERROR(HDF5);
#endif /*HAVE_PARALLEL*/

    /* Now put them all together */
    if ((vinfo = H5Tcreate(H5T_COMPOUND, offset))<0) SS_ERROR(HDF5);
    if (hdf_start-saf_start>0 && H5Tinsert(vinfo, "saf", saf_start, saf_type)<0) SS_ERROR(HDF5);
    if (mpi_start-hdf_start>0 && H5Tinsert(vinfo, "hdf5", hdf_start, hdf_type)<0) SS_ERROR(HDF5);
    if (offset-mpi_start>0 && H5Tinsert(vinfo, "mpi", mpi_start, mpi_type)<0) SS_ERROR(HDF5);

    /* Create and write the attribute */
    if ((scalar=H5Screate_simple(1, &one, &one))<0) SS_ERROR(HDF5);
    if ((attr=H5Acreate(grp, "version", vinfo, scalar, H5P_DEFAULT))<0) SS_ERROR(HDF5);
    if (H5Awrite(attr, vinfo, buffer)<0) SS_ERROR(HDF5);

    /* Free resources */
    if (H5Aclose(attr)<0) SS_ERROR(HDF5);
    attr = -1;
    if (H5Tclose(vinfo)<0) SS_ERROR(HDF5);
    vinfo = -1;
    if (saf_type>0 && H5Tclose(saf_type)<0) SS_ERROR(HDF5);
    saf_type = -1;
    if (hdf_type>0 && H5Tclose(hdf_type)<0) SS_ERROR(HDF5);
    hdf_type = -1;
    if (mpi_type>0 && H5Tclose(mpi_type)<0) SS_ERROR(HDF5);
    mpi_type = -1;
    if (H5Sclose(scalar)<0) SS_ERROR(HDF5);
    scalar = -1;
    
SS_CLEANUP:
    if (attr>0) H5Aclose(attr);
    if (vinfo>0) H5Tclose(vinfo);
    if (saf_type>0) H5Tclose(saf_type);
    if (hdf_type>0) H5Tclose(hdf_type);
    if (mpi_type>0) H5Tclose(mpi_type);
    if (annot>0) H5Tclose(annot);
    if (scalar>0) H5Sclose(scalar);
    SS_LEAVE(0);
}
