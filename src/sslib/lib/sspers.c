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
SS_IF(pers);

ss_pers_class_t  ss_pers_class_g[SS_PERS_NCLASSES];
hid_t ss_pers_tm;                                       /* HDF5 datatype corresponding to ss_pers_t */
hid_t ss_pers_tf;                                       /* HDF5 datatype for an ss_pers_t in a file */
const ss_pers_t SS_PERS_NULL;                           /* A null link, sort of like a C null pointer */

static htri_t ss_pers_find_cb(size_t itemidx, ss_pers_t *pers, ss_persobj_t *persobj, void *_find_data);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Note:        Persistent Object Properties
 *
 * Description:
 *              * /noregistry/: If set to a non-zero value then a call to ss_pers_find() will search only in the specified
 *                              scope and not any registry scopes.
 *
 *              * /detect_overflow/: If true then a find operation will fail if the number of objects found is greater than
 *                                   the limit imposed by the caller of ss_pers_find().
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Interface initializer
 *
 * Description: Initializes the persistent object interface.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, July 31, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_init(void)
{
    unsigned char       uc;
    hid_t               table_enum=-1;

    SS_ENTER_INIT;

    /* The HDF5 datatype corresponding to ss_pers_t in memory. This is an opaque datatype so that we can register a custom
     * conversion function to/from the file representation of an object link. */
    if ((ss_pers_tm = H5Tcreate(H5T_OPAQUE, sizeof(ss_pers_t)))<0) SS_ERROR_FMT(HDF5, ("ss_pers_tm"));
    if (H5Tset_tag(ss_pers_tm, "SSlib::pers_tm")<0) SS_ERROR_FMT(HDF5, ("ss_pers_tm"));
    if (H5Tlock(ss_pers_tm)<0) SS_ERROR_FMT(HDF5, ("ss_pers_tm"));

    /* A 1-byte enumeration type for the various kinds of tables. This isn't really necessary (we could have just used
     * a one-byte integer) and in fact we never actually ever try to convert these based on member names when reading links
     * from a file. But the output from HDF5 tools looks a lot better because they'll print the table name instead of some
     * small integer. */
    if ((table_enum = H5Tenum_create(H5T_STD_U8LE))<0) SS_ERROR(HDF5);
    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_scope_t));
    if (H5Tenum_insert(table_enum, "scope", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.scope"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_field_t));
    if (H5Tenum_insert(table_enum, "field", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.field"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_role_t));
    if (H5Tenum_insert(table_enum, "role", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.role"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_basis_t));
    if (H5Tenum_insert(table_enum, "basis", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.basis"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_algebraic_t));
    if (H5Tenum_insert(table_enum, "algebraic", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.algebraic"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_evaluation_t));
    if (H5Tenum_insert(table_enum, "evaluation", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.evaluation"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_relrep_t));
    if (H5Tenum_insert(table_enum, "relrep", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.relrep"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_quantity_t));
    if (H5Tenum_insert(table_enum, "quantity", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.quantity"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_unit_t));
    if (H5Tenum_insert(table_enum, "unit", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.unit"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_cat_t));
    if (H5Tenum_insert(table_enum, "cat", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.cat"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_collection_t));
    if (H5Tenum_insert(table_enum, "collection", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.collection"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_set_t));
    if (H5Tenum_insert(table_enum, "set", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.set"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_rel_t));
    if (H5Tenum_insert(table_enum, "rel", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.rel"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_fieldtmpl_t));
    if (H5Tenum_insert(table_enum, "fieldtmpl", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.fieldtmpl"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_tops_t));
    if (H5Tenum_insert(table_enum, "tops", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.tops"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_blob_t));
    if (H5Tenum_insert(table_enum, "blob", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.blob"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_indexspec_t));
    if (H5Tenum_insert(table_enum, "indexspec", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.indexspec"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_file_t));
    if (H5Tenum_insert(table_enum, "file", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.file"));

    uc = SS_MAGIC_SEQUENCE(SS_MAGIC(ss_attr_t));
    if (H5Tenum_insert(table_enum, "attr", &uc)<0) SS_ERROR_FMT(HDF5, ("table_enum.attr"));

    /* The HDF5 datatype for a persistent object link in a file. Do not change this type description without also changing the
     * conversion functions in ss_pers_convert_mf() and ss_pers_convert_fm(). The file representation of an object link is:
     *
     *   file_idx:  Object links are stored in a SAF file which contains a "File" table in its top-level scope.  An object
     *              link contains an index into that table which can be used to describe where the object to which the link
     *              points actually lives.  The first item of the "File" table refers to the file containing that table.
     *
     *   scope_idx: The destination file (i.e., the file containing the object to which the link points) contains a top-level
     *              scope with a "Scope" table that describes all the scopes existing in that file.  This scope_idx field
     *              is an index into that "Scope" table. The first item of the "Scope" table refers to the top-level scope.
     *   
     *   table:     Every scope contains a table for each class of object that can be stored in that scope and the table_idx
     *              field specifies which table, and thus which object class.  This is just the SS_MAGIC_SEQUENCE() part of a
     *              persistent object or persistent link magic value (e.g., SS_MAGIC_SEQUENCE(SS_MAGIC_OF(ss_field_t))).
     *
     *   item_idx:  This is an index into the selected table where the object is stored and will always be a direct
     *              index (i.e., the SS_TABLE_INDIRECT bit is clear). */
    if ((ss_pers_tf = H5Tcreate(H5T_COMPOUND, 11))<0) SS_ERROR_FMT(HDF5, ("ss_pers_tf"));
    if (H5Tinsert(ss_pers_tf, "file_idx",  0, H5T_STD_U16LE)<0) SS_ERROR_FMT(HDF5, ("ss_pers_tf.file_idx"));
    if (H5Tinsert(ss_pers_tf, "scope_idx", 2, H5T_STD_U32LE)<0) SS_ERROR_FMT(HDF5, ("ss_pers_tf.scope_idx"));
    if (H5Tinsert(ss_pers_tf, "table",     6, table_enum   )<0) SS_ERROR_FMT(HDF5, ("ss_pers_tf.table_idx"));
    if (H5Tinsert(ss_pers_tf, "item_idx",  7, H5T_STD_U32LE)<0) SS_ERROR_FMT(HDF5, ("ss_pers_tf.item_idx"));
    if (H5Tlock(ss_pers_tf)<0) SS_ERROR_FMT(HDF5, ("ss_pers_tf"));

    /* Datatypes from these interfaces are needed by ss_perstab_init */
    if (ss_string_init()<0) SS_ERROR(INIT);
    if (ss_array_init()<0) SS_ERROR(INIT);
    if (ss_blob_init()<0) SS_ERROR(INIT);

    /* Conversion functions */
    if (H5Tregister(H5T_PERS_HARD, "ss_pers_t(mf)", ss_pers_tm, ss_pers_tf, ss_pers_convert_mf)<0) SS_ERROR(INIT);
    if (H5Tregister(H5T_PERS_HARD, "ss_pers_t(fm)", ss_pers_tf, ss_pers_tm, ss_pers_convert_fm)<0) SS_ERROR(INIT);
    
    /* Initialize table datatypes */
    if (ss_perstab_init()) SS_ERROR(INIT);

    /* Successful cleanup */
    if (table_enum>0 && H5Tclose(table_enum)<0) SS_ERROR(HDF5);
    table_enum = -1;

 SS_CLEANUP:
    if (table_enum>0) H5Tclose(table_enum);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Create a new persistent object
 *
 * Description: Creates a new persistent object of the specified object type in the specified scope.  Normally this function
 *              assumes that every caller could be creating its own object and the table synchronization algorithm will
 *              determine later how many objects were actually created by comparing their contents. However, synchronization
 *              can be an expensive operation which can be avoided when the caller knows that all tasks of the scope's
 *              communicator are participating to create a single object. This is situation is noted by passing the
 *              SS_ALLSAME bit in the FLAGS argument.
 *
 * Return:      Returns a link to the new object on success; the null pointer on failure. If BUF is supplied then that will be
 *              the successful return value, otherwise a persistent object link will be allocated.
 *
 * Parallel:    Independent or collective. This function must be collective across the scope's communicator (although
 *              communication-free) if the SS_ALLSAME bit is passed in the FLAGS argument. In other words, if all
 *              tasks are participating to create one single object, then the call must be collective if we wish to avoid the
 *              synchronization costs later. However, it is still possible for all tasks to create one single object
 *              independently (i.e., creation order doesn't matter) if they don't pass SS_ALLSAME and they don't
 *              mind paying the synchronization cost later.
 *
 * Also:        ss_pers_copy()
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
ss_pers_new(ss_scope_t *scope,          /* The scope that will own this new object. */
            unsigned tableid,           /* A magic number whose sequence part defines a table */
            const ss_persobj_t *init,   /* Optional initial data of type ss_persobj_t or a type derived therefrom. The type must
                                         * be appropriate for the class of object being created. This argument can be used to
                                         * copy a persistent object. ISSUE: Should this be a link instead? */
            unsigned flags,             /* Creation flags, like SS_ALLSAME */
            ss_pers_t *buf,             /* Optional buffer for return value */
            ss_prop_t UNUSED *props     /* Additional properties (none defined yet) */
            )
{
    SS_ENTER(ss_pers_new, ss_pers_tP);
    ss_persobj_t *persobj = NULL;
    ss_pers_t *objlink = NULL;
    size_t idxtype;
    ss_table_t *table=NULL;
    ss_pers_class_t *pc=NULL;

    tableid = SS_MAGIC_SEQUENCE(tableid);
    if (NULL==(pc=SS_PERS_CLASS(tableid))) SS_ERROR(NOTFOUND);

    /* Obtain a pointer to the table for this object. */
    if (NULL==(table = ss_scope_table(scope, tableid, NULL))) SS_ERROR(FAILED);

    /* Obtain memory for the object and initialize it.  When declaring a single new object with SS_ALLSAME we can
     * immediately give it a permanent home, evicting any temporary object that might be there, because we know all tasks have
     * the same number of permanent objects. */
    idxtype = flags & SS_ALLSAME ? 0 : SS_TABLE_INDIRECT;
    if (NULL==(persobj=ss_table_newobj(table, idxtype, init, NULL))) SS_ERROR(FAILED);
    persobj->dirty = TRUE;

    /* If an initial value was supplied then we may have to reallocate some of the resources in the new object so that they're
     * not shared between the new object and the initial object. We do this in place (hence the NULL first argument). */
    if (init && ss_val_copy(NULL, persobj, pc->valinfo_nused, pc->valinfo)<0) SS_ERROR(FAILED);

    /* If all tasks are supplying the same data then mark the object as synchronized because it will save us some work when we
     * actually do attempt to synchronize later.  We mark it with SS_ALLSAME (which is true but distinct from the constant
     * `TRUE') to indicate that we never actually synchronized but rather we just "know" that the object is in a synchronized
     * state.  It's not possible to avoid this little complication because the rule is that all synchronized objects have a
     * last-synchronized checksum stored with them but we can't compute the checksum yet because we're just now creating an
     * empty object that the user will fill in shortly.  See also ss_table_synchronize(). */
    if (flags & SS_ALLSAME) {
        persobj->synced = SS_ALLSAME;
    }

    /* Create a link to the object. This will be our return value */
    if (NULL==(objlink=ss_pers_refer(scope, persobj, buf))) SS_ERROR(FAILED);
    
 SS_CLEANUP:
    SS_LEAVE(objlink);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Copy an object
 *
 * Description: Copy the given object and return a link to it.  If the object contains memory that needs to be copied (like
 *              character strings or variable length arrays) then those are copied also.  Other objects to which the original
 *              pointed are not copied -- the new object has links to the same ones.
 *
 * Return:      Returns a link to the new object on success; the null pointer on failure.  If BUF is supplied then that will
 *              be the successful return value, otherwise a persistent object link will be allocated.
 *
 * Parallel:    Independent or collective. This function must be collective across the scope's communicator (althrough
 *              communication-free) if the SS_ALLSAME bit is passed in the FLAGS argument. In other words, if all tasks are
 *              participating to create one single new object then the call must be collecitve if we wish to avoid the
 *              synchronization costs later. However, it is still possible for all tasks to create one single object
 *              independently (i.e., creation order doesn't matter) if they don't pass SS_ALLSAME and they don't mind paying
 *              the synchronization cost later.
 *
 * Also:        ss_pers_new()
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
ss_pers_copy(ss_pers_t *pers,           /* The object to be copied. */
             ss_scope_t *scope,         /* The destination scope that will own the new object. */
             unsigned flags,            /* Creation flags like SS_ALLSAME (see ss_pers_new()). */
             ss_pers_t *buf,            /* Optional buffer for return value. */
             ss_prop_t *props           /* Additional properties (none defined yet) */
             )
{
    SS_ENTER(ss_pers_copy, ss_pers_tP);
    ss_pers_t           *retval=NULL;
    unsigned            tableid;
    ss_persobj_t        *persobj=NULL;

    SS_ASSERT_CLASS(pers, ss_pers_t);

    tableid = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(pers));
    if (NULL==(persobj=ss_pers_deref(pers))) SS_ERROR(FAILED);
    if (NULL==(retval=ss_pers_new(scope, tableid, persobj, flags, buf, props))) SS_ERROR(FAILED);

 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Sets persistent object to initial state
 *
 * Description: This function sets a persistent object to an initial state of an all-zero bit pattern. The ss_foo/obj/_tm and
 *              ss_foo/obj/_tf parts of the C struct are set to zero but the other stuff is left unmodified (except the dirty
 *              bit is set).
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. However, if the SS_ALLSAME bit flag is set then this function should be called collectively
 *              across the communicator of the scope that owns the object.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, January 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_reset(ss_pers_t *pers,          /* The object to be reset */
              unsigned flags            /* Bit flags such as SS_ALLSAME */
              )
{
    SS_ENTER(ss_pers_reset, herr_t);
    ss_persobj_t        *persobj=NULL;

    SS_ASSERT_CLASS(pers, ss_pers_t);
    if (NULL==(persobj=ss_pers_deref(pers))) SS_ERROR(FAILED);
    if (ss_pers_reset_(persobj, flags)<0) SS_ERROR(FAILED);
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Sets persistent object to initial state
 *
 * Description: Internal version of ss_pers_reset() that takes an object pointer instead of a link.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. However, if the SS_ALLSAME bit flag is set then this function should be called collectively
 *              across the communicator of the scope that owns the object.
 *
 * Programmer:  Robb Matzke
 *              Friday, February 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_reset_(ss_persobj_t *persobj,   /* The object to be reset */
               unsigned flags           /* Bit flags such as SS_ALLSAME */
               )
{
    SS_ENTER(ss_pers_reset_, herr_t);
    ss_pers_class_t     *pc=NULL;
    size_t              start, size;

    SS_ASSERT_CLASS(persobj, ss_persobj_t);
    if (NULL==(pc=SS_PERS_CLASS(SS_MAGIC_SEQUENCE(SS_MAGIC_OF(persobj))))) SS_ERROR(NOTFOUND);

    /* All persistent objects in memory start with an `m' member containing all the transient memory values which is followed
     * by the file-persistent stuff to be reset.  The `m' member always starts with a `pers' member which must not be reset
     * and which is followed by object-specific transient stuff that should be cleared. */
    ss_val_reset(persobj, pc->valinfo_nused, pc->valinfo);
    start = sizeof(ss_persobj_t);
    size = pc->t_size - start;
    memset((char*)persobj+start, 0, size);

    /* Mark object as modified and unsynchronized */
    persobj->dirty = TRUE;
    persobj->synced = ((flags & SS_ALLSAME) && persobj->synced) ? SS_ALLSAME : FALSE;

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Destructor
 *
 * Description: Destroys a persistent object link by releasing those resources used by a persistent object link. If the link
 *              was allocated on the heap then the caller should free it.
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
ss_pers_dest(ss_pers_t *pers)
{
    SS_ENTER(ss_pers_dest, herr_t);
    SS_ASSERT_CLASS(pers, ss_pers_t);
    SS_OBJ_DEST(pers);
 SS_CLEANUP:
    SS_LEAVE(0);
}


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Dereference an object link
 *
 * Description: Given a link to a persistent object, dereference that link and return a pointer to the object itself. This may
 *              involve reading a table into memory if this is the first dereference into that table.
 *
 *              This function is almost never invoked directly by client code. Instead, the client will use macros appropriate
 *              for each object class which will check the link class and cast the return value to the appropriate type.  For
 *              instance, SS_FIELD() is a macro that takes a field object link as an argument, compile-time checks that the
 *              argument is an ss_field_t pointer, run-time check that the pointer is valid, and return an ss_fieldobj_t
 *              pointer.
 *
 * Return:      Returns an object pointer on success; the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Issue:       We should probably accumulate some sort of statistics here to make sure that the object caching is performing
 *              as expected.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, June 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_persobj_t *
ss_pers_deref(ss_pers_t *pers)
{
    SS_ENTER(ss_pers_deref, ss_persobj_tP);
    ss_persobj_t        *persobj=NULL;                  /* A pointer to the persistent object to be returned */

    SS_ASSERT(pers);
    SS_ASSERT_CLASS(pers, ss_pers_t);

    /* Make sure link is current */
    if (ss_pers_update(pers)<0) SS_ERROR(FAILED);

    /* Error conditions */
    if (SS_PERS_LINK_NULL==ss_pers_link_state(pers)) SS_ERROR_FMT(NOTFOUND, ("dereferencing a null link"));
    if (SS_PERS_LINK_RESERVED==ss_pers_link_state(pers)) SS_ERROR_FMT(CORRUPT, ("mangled object link"));
    if (SS_PERS_LINK_CLOSED==ss_pers_link_state(pers)) SS_ERROR_FMT(NOTFOUND, ("object not found"));

    /* Result */
    persobj = ss_pers_link_objptr(pers);
    SS_ASSERT(persobj);
    SS_ASSERT_CLASS(persobj, ss_persobj_t);
    SS_ASSERT(SS_MAGIC_SEQUENCE(SS_MAGIC_OF(persobj))==SS_MAGIC_SEQUENCE(SS_MAGIC_OF(pers)));

SS_CLEANUP:
    SS_LEAVE(persobj);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Updates an object link
 *
 * Description: This function makes information in the object link as current as possible. If the object has a permanent home
 *              in the table but the object index stored in the link is indirect then it is converted to a direct index.  If
 *              the object is in memory then the link is moved to the `memory' state.  If the `mapidx' value stored in the
 *              object is different than the object index in the link then the object index is updated in the link.
 *
 *              This function is essentially a weaker version of ss_pers_deref().
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, January 14, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_update(ss_pers_t *pers)
{
    SS_ENTER(ss_pers_update, herr_t);
    ss_gfile_t          *gfile=NULL;                    /* The GFile array entry for the file that owns the object */
    ss_table_t          *table=NULL;                    /* The scope table or the resulting object's table */
    ss_scopeobj_t       *scopeobj=NULL;                 /* The scope object owning the desired object */
    ss_persobj_t        *persobj=NULL;                  /* A pointer to the persistent object to be returned */
    size_t              objidx;                         /* An index into a table for the object in question */
    unsigned            tableidx;                       /* Object magic serial number for table */

    SS_ASSERT(pers);
    SS_ASSERT_CLASS(pers, ss_pers_t);

    if (SS_PERS_LINK_NULL==ss_pers_link_state(pers)) goto done;
    if (SS_PERS_LINK_RESERVED==ss_pers_link_state(pers)) goto done;
    if (NULL==(gfile=SS_GFILE_LINK(pers))) goto done;

    if (gfile->cur_open>0 ||
        (gfile->topscope && SS_MAGIC(ss_scope_t)==SS_MAGIC_OF(pers))) {
        /* The file is currently open (or the file was previously open and we're updating a scope link) but the object in
         * question may not yet be in memory if its table has never been read by this task. If the object is in memory then
         * make sure the link has the current address and object index, otherwise make the link a `closed' link. */

        /* Scope tables are never deleted when a scope is closed, therefore there is no need to consult the `open_serial'
         * number of such a link to determine if the cached object pointer is out of date.  In fact, doing so would cause
         * infinite recursion when the ss_scope_table() just below tries to dereference gfile->topscope. */
        if (SS_MAGIC_OF(pers)==SS_MAGIC(ss_scope_t)) ss_pers_link_setopenserial(pers, gfile->open_serial);

        /* If the link is in the memory state and the object to which it points agrees has a `mapidx' that agrees with the
         * object index stored in the link itself, then short circuit in order to prevent infinite recursion between this
         * function and ss_pers_deref(). */
        if (SS_PERS_LINK_MEMORY==ss_pers_link_state(pers) &&
            gfile->open_serial==ss_pers_link_openserial(pers) &&
            NULL!=(persobj=ss_pers_link_objptr(pers)) &&
            persobj->mapidx==ss_pers_link_objidx(pers)) goto done;

        /* Get the object's scope */
        if (NULL==(table=ss_scope_table(gfile->topscope, SS_MAGIC(ss_scope_t), NULL))) SS_ERROR(FAILED);
        if (NULL==(scopeobj=(ss_scopeobj_t*)ss_table_lookup(table, ss_pers_link_scopeidx(pers), SS_STRICT))) SS_ERROR(FAILED);

        /* Get the object's table */
        tableidx = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(pers));
        if (NULL==(table=scopeobj->m.table[tableidx])) SS_ERROR(NOTFOUND);

        /* See if the object is in memory already. This has the side effect of allocating memory for the object although it
         * won't actually read the object from the file. If the object isn't in memory yet then the link must necessarily have
         * a direct object index. */
        if (NULL==(persobj=ss_table_lookup(table, ss_pers_link_objidx(pers), 0))) SS_ERROR(FAILED);
        if (SS_MAGIC_CLASS(SS_MAGIC_OF(persobj))!=SS_MAGIC(ss_persobj_t)) {
            /* Not in memory */
            SS_ASSERT(0==(ss_pers_link_objidx(pers) & SS_TABLE_INDIRECT));
            ss_pers_link_setstate(pers, SS_PERS_LINK_CLOSED);
            ss_pers_link_setobjptr(pers, NULL);
            ss_pers_link_setopenserial(pers, gfile->open_serial);
        } else {
            /* In memory */
            ss_pers_link_setstate(pers, SS_PERS_LINK_MEMORY);
            ss_pers_link_setobjidx(pers, persobj->mapidx);
            ss_pers_link_setobjptr(pers, persobj);
            ss_pers_link_setopenserial(pers, gfile->open_serial);
        }
        
    } else if (gfile->topscope) {
        /* File was open before but now is closed. It's scopes are still in memory however and we can use that to make sure
         * that the link has a direct object index. */

        /* Get the object's scope */
        if (NULL==(table=ss_scope_table(gfile->topscope, SS_MAGIC(ss_scope_t), NULL))) SS_ERROR(FAILED);
        if (NULL==(scopeobj=(ss_scopeobj_t*)ss_table_lookup(table, ss_pers_link_scopeidx(pers), SS_STRICT))) SS_ERROR(FAILED);

        /* Get the object's table. */
        tableidx = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(pers));
        if (NULL==(table=scopeobj->m.table[tableidx])) SS_ERROR(NOTFOUND);

        /* Make the link a `closed' link and use the direct index */
        ss_pers_link_setstate(pers, SS_PERS_LINK_CLOSED);
        ss_pers_link_setobjptr(pers, NULL);
        if (ss_pers_link_objidx(pers) & SS_TABLE_INDIRECT) {
            if (SS_NOSIZE==(objidx=ss_table_direct(table, ss_pers_link_objidx(pers)))) SS_ERROR(FAILED);
            SS_ASSERT(0==(objidx & SS_TABLE_INDIRECT)); /*if a table was closed it must have been synchronized*/
            ss_pers_link_setobjidx(pers, objidx);
        }
        
    } else {
        /* The destination file has never been opened. Therefore the link must already be in a closed state with a direct
         * object index. */
        SS_ASSERT(SS_PERS_LINK_CLOSED==ss_pers_link_state(pers));
        SS_ASSERT(0==(ss_pers_link_objidx(pers) & SS_TABLE_INDIRECT));
    }

done:
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Create an object link
 *
 * Description: This function creates (or fills in) a link to a new object that exists in memory.
 *
 * Return:      Returns a pointer to a persisent object link (either the supplied PERS or a newly allocated one) on success;
 *              the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, June 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
ss_pers_refer(ss_scope_t *scope,              /* The scope to which PERSOBJ belongs. */
              ss_persobj_t *persobj,          /* The object to which the new link will point. */
              ss_pers_t *pers                 /* Optional memory for the link. */
              )
{
    SS_ENTER(ss_pers_refer, ss_pers_tP);
    unsigned            tableidx;
    ss_pers_t           *pers_in=pers;
    ss_pers_class_t     *pc=NULL;
    ss_table_t          *table=NULL;
    ss_gfile_t          *gfile=NULL;

    SS_ASSERT_MEM(scope, ss_scope_t);
    SS_ASSERT_CLASS(persobj, ss_persobj_t);
    tableidx = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(persobj));
    pc = SS_PERS_CLASS(tableidx);
    SS_ASSERT(pc);
    gfile = SS_GFILE_LINK(scope);
    SS_ASSERT(gfile);

    /* Create and/or initialize the link */
    pers=(ss_pers_t*)ss_obj_new((ss_obj_t*)pers, SS_MAGIC_CONS(SS_MAGIC(ss_pers_t), tableidx), sizeof(ss_pers_t), NULL);
    if (!pers) SS_ERROR(FAILED);

    /* The PERS object must be in the same file as the SCOPE -- in fact, in the specified scope */
    if (NULL==(table = ss_scope_table(scope, SS_MAGIC_OF(pers), NULL))) SS_ERROR(FAILED);
    if (ss_table_owns(table, persobj)<=0) SS_ERROR(NOTFOUND);

    /* Initialize the link */
    ss_pers_link_setobjptr(pers, persobj);
    ss_pers_link_setobjidx(pers, persobj->mapidx);
    ss_pers_link_setopenserial(pers, gfile->open_serial);
    ss_pers_link_setscopeidx(pers, SS_SCOPE(scope)->m.pers.mapidx);
    ss_pers_link_setgfileidx(pers, ss_pers_link_gfileidx(scope));
    ss_pers_link_setstate(pers, SS_PERS_LINK_MEMORY);

 SS_CLEANUP:
    if (pers && !pers_in) {
        SS_OBJ_DEST(pers);
        SS_FREE(pers);
    }
    SS_LEAVE(pers);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Create an object link
 *
 * Description: This function creates (or fills in) a link to a new object. An object link can be in one of two states: Closed
 *              and Memory (there is a Filed state also, but that never appears anywhere but in a file).  This function creates
 *              a link in the Closed state.
 *
 * Return:      Returns a pointer to a persistent object link (either the supplied PERS or a newly allocated one) on success;
 *              the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, August 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
ss_pers_refer_c(ss_scope_t *scope,              /* The scope containing the object to be referenced */
                unsigned tableidx,              /* The magic number for the type of object being referenced. Only the sequence
                                                 * part is actually used, so one may pass magic numbers of the ss_pers_t or
                                                 * ss_persobj_t variety. */
                size_t itemidx,                 /* The index of the item being referenced. This can be a direct or indirect
                                                 * index. */
                ss_pers_t *pers                 /* Optional memory for the link. If none is supplied then memory is allocated.
                                                 * This is accomplished simply by passing this argument down to ss_obj_new(). */
                )
{
    SS_ENTER(ss_pers_refer_c, ss_pers_tP);
    ss_pers_t           *pers_in=pers;
    ss_gfile_t          *gfile=NULL;

    SS_ASSERT_MEM(scope, ss_scope_t);
    tableidx = SS_MAGIC_SEQUENCE(tableidx);
    SS_ASSERT(SS_PERS_CLASS(tableidx));

    /* Create and/or initialize the link */
    if (NULL==(gfile=SS_GFILE_LINK(scope))) SS_ERROR_FMT(NOTFOUND, ("gfile of scope"));
    pers=(ss_pers_t*)ss_obj_new((ss_obj_t*)pers, SS_MAGIC_CONS(SS_MAGIC(ss_pers_t), tableidx), sizeof(ss_pers_t), NULL);
    if (!pers) SS_ERROR(FAILED);
    ss_pers_link_setobjptr(pers, NULL);
    ss_pers_link_setobjidx(pers, itemidx);
    ss_pers_link_setopenserial(pers, gfile->open_serial);
    ss_pers_link_setscopeidx(pers, SS_SCOPE(scope)->m.pers.mapidx);
    ss_pers_link_setgfileidx(pers, ss_pers_link_gfileidx(scope));
    ss_pers_link_setstate(pers, SS_PERS_LINK_CLOSED);

 SS_CLEANUP:
    if (pers && !pers_in) {
        SS_OBJ_DEST(pers);
        SS_FREE(pers);
    }
    SS_LEAVE(pers);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Bootstrap topscope handle
 *
 * Description: This functions sole purpose is to create a handle to a top-level scope when a file is opened or created. We
 *              can't just use ss_pers_refer() because that function takes a scope handle as an argument.
 *
 * Return:      Returns a pointer to a new scope handle on success; the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August  8, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
ss_pers_refer_topscope(size_t gfileidx, ss_persobj_t *scopeobj)
{
    SS_ENTER(ss_pers_refer_topscope, ss_pers_tP);
    ss_scope_t *scope=NULL;
    ss_gfile_t  *gfile;
    
    if (NULL==(gfile=SS_GFILE_IDX(gfileidx))) SS_ERROR_FMT(NOTFOUND, ("gfile index %lu", (unsigned long)gfileidx));
    if (NULL==(scope=SS_OBJ_NEW(ss_scope_t))) SS_ERROR(FAILED);
    ss_pers_link_setobjptr(scope, scopeobj);
    ss_pers_link_setobjidx(scope, scopeobj->mapidx);
    ss_pers_link_setscopeidx(scope, 0);
    ss_pers_link_setgfileidx(scope, gfileidx);
    ss_pers_link_setopenserial(scope, gfile->open_serial);
    ss_pers_link_setstate(scope, SS_PERS_LINK_MEMORY);
    
 SS_CLEANUP:
    SS_LEAVE((ss_pers_t*)scope);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Obtain scope for an object
 *
 * Description: Every persistent object belongs to a scope and this function returns a link to that scope.
 *
 * Return:      On success, returns a link to the scope containing PERS. If the caller supplied a buffer for the result in the
 *              BUF argument then that's the success pointer returned. Returns the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, November 17, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_scope_t *
ss_pers_scope(ss_pers_t *pers,               /* Persistent object link to query */
              ss_scope_t *buf                /* OUT: Optional buffer for the result scope link */
              )
{
    SS_ENTER(ss_pers_scope, ss_scope_tP);
    ss_scope_t          *buf_here=NULL;
    ss_scope_t          topscope, *retval=NULL;
    ss_persobj_t        *scopeobj=NULL;
    ss_table_t          *table=NULL;

    SS_ASSERT_CLASS(pers, ss_pers_t);
    if (NULL==ss_pers_topscope(pers, &topscope)) SS_ERROR(FAILED);
    if (NULL==(table = ss_scope_table(&topscope, SS_MAGIC(ss_scope_t), NULL))) SS_ERROR(FAILED);
    if (NULL==(scopeobj=ss_table_lookup(table, ss_pers_link_scopeidx(pers), SS_STRICT))) SS_ERROR(FAILED);
    if (NULL==(retval = (ss_scope_t*)ss_pers_refer(&topscope, scopeobj, (ss_pers_t*)buf))) SS_ERROR(FAILED);

 SS_CLEANUP:
    SS_FREE(buf_here);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Obtain file for an object
 *
 * Description: Every persistent object belongs to a file and this function returns a link to that file.
 *
 * Return:      On success, returns a link to the file containing PERS. If the caller supplied a buffer for the result in the
 *              FILE argument then that's the success pointer returned. Returns the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, May 21, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_file_t *
ss_pers_file(ss_pers_t *pers,           /* Persistent object link to query */
             ss_file_t *file            /* OUT: Optional buffer for the result file link */
             )
{
    SS_ENTER(ss_pers_file, ss_file_tP);
    ss_scope_t          topscope;

    if (NULL==ss_pers_topscope(pers, &topscope)) SS_ERROR(FAILED);
    if (NULL==(file=(ss_file_t*)ss_pers_refer_c(&topscope, SS_MAGIC(ss_file_t), 0, (ss_pers_t*)file))) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(file);
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Test whether an object can be modified
 *
 * Description: An object can be modified if it belongs to a scope that is modifiable. This function tests whether that is
 *              true.
 *
 * Issue:       Since the client can always obtain a pointer into memory for the object by dereferencing the link, there is
 *              nothing stopping the client from modifying that memory and setting the object's dirty bit and since all of
 *              that can be done with straight C code (without SSlib assistance) it is impossible for SSlib to warn about that
 *              situation.  However, a synchronization should be able to detect and report it.
 *
 * Return:      Returns true (positive) if PERS points to a persistent object that exists in a writable scope and false
 *              otherwise. If PERS points to a persistent object that is not in memory (e.g., it's file is not open or its
 *              file has been subsequently closed) then this function returns false. Errors are indicated with a negative
 *              return value.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, January 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_pers_iswritable(ss_pers_t *pers)
{
    SS_ENTER(ss_pers_iswritable, htri_t);
    htri_t              retval=FALSE;
    ss_gfile_t          *gfile=NULL;
    ss_scope_t          scope;

    if (NULL==(gfile=SS_GFILE_LINK(pers))) SS_ERROR(FAILED);
    if (0==gfile->cur_open) goto done; /*file is not open*/
    if (NULL==ss_pers_scope(pers, &scope)) SS_ERROR(FAILED);
    if (ss_scope_isopen(&scope)<=0) {
        SS_STATUS_OK;
        goto done; /*scope is not open*/
    }
    if ((retval=ss_scope_iswritable(&scope))<0) SS_ERROR(FAILED);

done:
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Obtain top scope for an object
 *
 * Description: Every persistent object belongs to a scope, every scope belongs to a file, and every file has one top-scope.
 *              This function returns a link to that top-scope.
 *
 * Return:      On success, returns a link to the top-scope for PERS. If the caller supplied a buffer for the result in the
 *              BUF argument then that's the success pointer returned. Returns the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, September  8, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_scope_t *
ss_pers_topscope(ss_pers_t *pers,               /* Persistent object link to query */
                 ss_scope_t *buf                /* OUT: Optional buffer for the result scope link */
                 )
{
    SS_ENTER(ss_pers_topscope, ss_scope_tP);
    ss_scope_t          *buf_here=NULL;
    ss_gfile_t          *gfile=NULL;

    SS_ASSERT_CLASS(pers, ss_pers_t);
    if (SS_PERS_LINK_NULL==ss_pers_link_state(pers)) SS_ERROR_FMT(NOTFOUND, ("null persistent link"));
    if (SS_PERS_LINK_RESERVED==ss_pers_link_state(pers)) SS_ERROR_FMT(CORRUPT, ("mangled persistent link"));

    if (NULL==(gfile = SS_GFILE_LINK(pers))) SS_ERROR(FAILED);
    if (gfile->cur_open<=0) SS_ERROR_FMT(NOTFOUND, ("file is closed: %s", gfile->name));
    SS_ASSERT(gfile->topscope);

    if (!buf && NULL==(buf=buf_here=malloc(sizeof(*buf)))) SS_ERROR(RESOURCE);
    *buf = *(gfile->topscope);

 SS_CLEANUP:
    SS_FREE(buf_here);
    SS_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Determine link equality
 *
 * Description: This function determines if two links point to the same object.
 *
 * Return:      Returns true (positive) if PERS1 and PERS2 refer to the same object without actually dereferencing the link,
 *              false if not, and negative on error. The names ss_pers_eq() and ss_pers_equal() come from LISP where the !eq
 *              function tests whether its operands refer to the same object and !equal that recursively compares its operands
 *              to determine if they have the same value.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, September 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_pers_eq(ss_pers_t *pers1, ss_pers_t *pers2)
{
    SS_ENTER(ss_pers_eq, htri_t);
    int                 cmp=-1;

    cmp = ss_pers_cmp(pers1, pers2, NULL);
    if (-2==cmp) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(0==cmp);
}

/*---------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Determine object equality
 *
 * Description: This function determines if two links point to objects that could be consistered to be the same object
 *              even if they don't point to the same memory. In other words, a "meter" in one database is almost certainly
 *              the same to a "meter" in some other database even though the two object handles point to distinct objects
 *              in memory (i.e., ss_pers_equal() is true but ss_pers_eq() is false).
 *
 * Return:      Returns true (positive) if PERS1 and PERS2 refer to objects whose internals are equal or if PERS1 and
 *              PERS2 both point to the same object or both are null pointers.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  8, 2004
 *---------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_pers_equal(ss_pers_t *pers1,
              ss_pers_t *pers2,
              ss_prop_t UNUSED *props   /* A property list to indicate how the comparison should proceed. No properties
                                         * are currently defined. If an object contains persistent object links then the
                                         * pointed-to objects will be compared with ss_pers_eq() instead of recursively
                                         * calling ss_pers_equal(). */
              )
{
    SS_ENTER(ss_pers_equal, htri_t);
    int                 cmp=-1;
    static char         mask[1024];
    static int          ncalls;

    /* Build the mask used to compare the two objects. */
    if (0==ncalls++) {
#ifndef NDEBUG
        int i;
        for (i=0; i<SS_PERS_NCLASSES; i++) {
            SS_ASSERT(ss_pers_class_g[i].t_size<=sizeof mask);
        }
#endif
        memset(mask, SS_VAL_CMP_DFLT, sizeof mask);
    }

    if (-2==(cmp=ss_pers_cmp(pers1, pers2, (ss_persobj_t*)mask))) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(0==cmp);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Change link state
 *
 * Description: A persistent object link can be in either a Closed or Memory state (not including the Filed state of links as
 *              they appear in a file).  This function moves a link from state to state and also makes sure all the information
 *              in the link is up to date.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, September 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_state(ss_pers_t *pers,                  /* The persistent object link whose state is to be changed. */
              ss_pers_link_state_t state        /* Desired state for the link, one of the SS_PERS_LINK constants. */
              )
{
    SS_ENTER(ss_pers_state, herr_t);

    SS_ASSERT_CLASS(pers, ss_pers_t);

    switch (state) {
    case SS_PERS_LINK_NULL:
        ss_pers_link_setobjptr(pers, NULL);
        ss_pers_link_setobjidx(pers, 0);
        ss_pers_link_setscopeidx(pers, 0);
        ss_pers_link_setgfileidx(pers, 0);
        ss_pers_link_setopenserial(pers, 0);
        break;
    case SS_PERS_LINK_RESERVED:
        /* What does this really mean? */
        break;
    case SS_PERS_LINK_CLOSED:
        if (SS_PERS_LINK_NULL==ss_pers_link_state(pers)) SS_ERROR_FMT(NOTFOUND, ("link is null"));
        if (SS_PERS_LINK_RESERVED==ss_pers_link_state(pers)) SS_ERROR_FMT(CORRUPT, ("mangled link"));

        /* We might be able to be more efficient here, especially if the link is in a closed state already. The problem here
         * is that if the link is in a closed state but the file is open then it will be updated to the memory state and the
         * object will be read from the file. */
        if (ss_pers_update(pers)<0) SS_ERROR(FAILED);
        ss_pers_link_setobjptr(pers, NULL);
        break;
    case SS_PERS_LINK_MEMORY:
        /* If the link is already in a memory state we probably still want to make sure the `objidx' is updated to be a direct
         * index if one exists. */  
        if (ss_pers_update(pers)<0) SS_ERROR(FAILED);
        break;
    }
    ss_pers_link_setstate(pers, state);

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Compares two persistent objects
 *
 * Description: Compares two objects, P1 and P2, and returns a value similar to memcmp() or strcmp().
 *
 *              If MASK is the null pointer then only the contents of the links P1 and P2 are consulted and the underlying
 *              objects are never referenced, a so called /shallow/ comparison. The return value will be one of the following:
 *
 *                                         |  1 if A > B by some well defined global ordering
 *                 ss_pers_cmp(A,B,NULL) = |  0 if A and B point to the same object
 *                                         | -1 if A < B
 *                                         | -2 on error
 *
 *              On the other hand, if MASK is not null then a /deep/ comparison is performed and MASK should be a block of
 *              memory the same size as the objects to which P1 and P2 point (MASK is not referenced if P1 and P2 point to
 *              objects of different types). The block of memory should be initialized to zero except where a comparison in
 *              the two underlying objects is desired.  For instance, when comparing the roles and topological dimensions of
 *              two categories (SAF_Cat) you would do the following:
 *
 *                  SAF_Cat a=...; b=...;
 *                  ss_catobj_t mask;
 *                  memset(&mask, 0, sizeof mask);
 *                  memset(&mask.role, SS_VAL_CMP_DFLT, 1); // default comparison mode for roles
 *                  memset(&mask.tdim, SS_VAL_CMP_DFLT, 1); // default comparison mode for topological dimensions
 *                  ss_pers_cmp((ss_pers_t*)&a, (ss_pers_t*)&b, (ss_persobj_t*)&mask);
 *
 *              Note two things: (1) only the first non-zero byte in the mask corresponding to any particular member is
 *              consulted to determine the kind of comparison, and (2) ss_pers_t and ss_persobj_t are the types from which all
 *              persistent object links and objects are derived and are binary compatible with all links and objects.
 *
 *              To compare all fields of two categories you could just set the whole mask to the desired comparison because
 *              ss_pers_cmp() will skip over parts of the mask that don't actually correspond to things that can be compared
 *              (e.g., padding bytes inserted by the compiler between members of the object and parts of the objects that
 *              are SSlib's private bookkeeping records):
 *
 *                  memset(&mask, SS_VAL_CMP_DFLT, sizeof mask);
 *
 *              The various flags defining comparisons are defined by the ss_val_cmp_t type.
 *
 * Issue:       Deep comparisons are not yet fully recursive. I.e., if P1 and P2 are being deeply compared and the objects to
 *              which P1 and P2 point contain object links which are being compared because they correspond to non-zero bits
 *              in MASK, then only a shallow comparison is performed on those links.  We plan to add a property list argument
 *              to this function that would allow finer-grained control of the deep comparison recursion.
 *
 * Also:        ss_pers_eq(), ss_pers_equal(), SS_PERS_EQ(), SS_PERS_EQUAL()
 *
 * Return:      Similar to memcmp() except successful return value is one of: -1, 0, or 1 instead of arbitrary negative and
 *              positive values. This allows -2 (or less) to indicate failure, which is standard practice in SSlib for
 *              comparison functions.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_pers_cmp(ss_pers_t *p1,                      /* First of two objects to compare. */
            ss_pers_t *p2,                      /* Second of two objects to cmpare. */
            const ss_persobj_t *mask            /* Optional mask to use for deep comparisons. A null value implies a shallow
                                                 * comparison, which means that the comparison should only look at the object
                                                 * handles and not the objects themselves. */
            )
{
    SS_ENTER(ss_pers_cmp, int);
    int         retval=0;

    SS_RETVAL(-2);                              /* Comparison functions return -2 for failure since -1 means P1<P2 */

    /* Check basic stuff that we need to do for both shallow and deep comparisons. */
    if (SS_PERS_ISNULL(p1) && SS_PERS_ISNULL(p2)) {
        goto done;
    } else if (SS_PERS_LINK_NULL==ss_pers_link_state(p1)) {
        retval = -1;
        goto done;
    } else if (SS_PERS_LINK_NULL==ss_pers_link_state(p2)) {
        retval = 1;
        goto done;
    }

    /* Some error checks */
    if (SS_PERS_LINK_RESERVED==ss_pers_link_state(p1) || SS_PERS_LINK_RESERVED==ss_pers_link_state(p2))
        SS_ERROR_FMT(USAGE, ("P1 and/or P2 have reserved state"));
    SS_ASSERT_CLASS(p1, ss_pers_t);
    SS_ASSERT_CLASS(p2, ss_pers_t);

    /* More stuff for both shallow and deep */
    if (SS_MAGIC_SEQUENCE(SS_MAGIC_OF(p1)) < SS_MAGIC_SEQUENCE(SS_MAGIC_OF(p2))) {
        retval = -1;
        goto done;
    } else if (SS_MAGIC_SEQUENCE(SS_MAGIC_OF(p1)) > SS_MAGIC_SEQUENCE(SS_MAGIC_OF(p2))) {
        retval = 1;
        goto done;
    }

    /* This stuff is only applicable to a shallow comparison. Object handles are unequal if:
     *   1. They point to different files, or
     *   2. They point to different scopes, or
     *   3. They point to different table entries */
    if (!mask) {
        if (ss_pers_link_gfileidx(p1) < ss_pers_link_gfileidx(p2)) {
            retval = -1;
            goto done;
        } else if (ss_pers_link_gfileidx(p1) > ss_pers_link_gfileidx(p2)) {
            retval = 1;
            goto done;
        } else if (ss_pers_link_scopeidx(p1) < ss_pers_link_scopeidx(p2)) {
            retval = -1;
            goto done;
        } else if (ss_pers_link_scopeidx(p1) > ss_pers_link_scopeidx(p2)) {
            retval = 1;
            goto done;
        }
        if (ss_pers_update(p1)<0) SS_ERROR(FAILED);
        if (ss_pers_update(p2)<0) SS_ERROR(FAILED);
        if (ss_pers_link_objidx(p1) < ss_pers_link_objidx(p2)) {
            retval = -1;
            goto done;
        } else if (ss_pers_link_objidx(p1) > ss_pers_link_objidx(p2)) {
            retval = 1;
            goto done;
        }
    }

    /* This stuff is only applicable to a deep comparison. */
    if (mask) {
        ss_persobj_t *po1, *po2;
        if (NULL==(po1=ss_pers_deref(p1))) SS_ERROR(FAILED);
        if (NULL==(po2=ss_pers_deref(p2))) SS_ERROR(FAILED);
        if (-2==(retval=ss_pers_cmp_(po1, po2, mask))) SS_ERROR(FAILED);
    }

done:
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Compares two persistent objects
 *
 * Description: This is an internal version of ss_pers_cmp() and does only a deep comparison of the two objects.
 *
 * Return:      On success returns -1, 0, or 1 depending on whether P1 is less than, equal, or greater than P2 by some
 *              arbitrary but consistent comparison algorithm. Returns -2 on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, October 18, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_pers_cmp_(ss_persobj_t *p1,                  /* First of two objects to compare. This is normally considered to be the
                                                 * "haystack". */                                      
             ss_persobj_t *p2,                  /* Second of two objects to compare. This is normally considered to be the
                                                 * "needle" and might contain special things like regular expressions, etc.
                                                 * depending on the values contained in the MASK. */
             const ss_persobj_t *mask           /* Which elements of P1 and P2 to compare. This isn't really a true object but
                                                 * rather a chunk of memory the same size as the objects that is filled with
                                                 * bytes that say which members of P1 and P2 to compare and how to compare
                                                 * them. */
             )
{
    
    SS_ENTER(ss_pers_cmp_, int);
    unsigned            persseq;                /* Persistent object type sequence number */
    int                 retval=0;               /* Return value */
    ss_pers_class_t     *pc=NULL;

    SS_RETVAL(-2);                              /* Failure return value */
    SS_ASSERT_CLASS(p1, ss_persobj_t);
    SS_ASSERT_CLASS(p2, ss_persobj_t);
    persseq = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(p1));
    if (SS_MAGIC_SEQUENCE(SS_MAGIC_OF(p2))!=persseq) SS_ERROR_FMT(USAGE, ("P1 and P2 must be same type"));
    SS_ASSERT(mask); /*must be present, but not necessarily a true ss_persobj_t object*/

    if (NULL==(pc=SS_PERS_CLASS(SS_MAGIC_SEQUENCE(SS_MAGIC_OF(p1))))) SS_ERROR(FAILED);
    if (-2==(retval=ss_val_cmp(p1, p2, mask, pc->valinfo_nused, pc->valinfo))) SS_ERROR(FAILED);
    
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Compute a checksum for a persistent object
 *
 * Description: Computes a checksum for the persistent part of a persistent object in memory.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_cksum(ss_persobj_t *persobj,            /* Persistent object whose checksum will be computed. */
              ss_val_cksum_t *cksum             /* OUT: The computed checksum. */
              )
{
    SS_ENTER(ss_pers_cksum, herr_t);
    ss_pers_class_t             *pc=NULL;

    SS_ASSERT_CLASS(persobj, ss_persobj_t);
    if (NULL==(pc=SS_PERS_CLASS(SS_MAGIC_SEQUENCE(SS_MAGIC_OF(persobj))))) SS_ERROR(NOTFOUND);
    if (ss_val_cksum(persobj, pc->valinfo_nused, pc->valinfo, NULL, cksum)<0) SS_ERROR(FAILED);
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Find objects in a scope
 *
 * Description: This function will find all objects in a particular SCOPE that match certain fields of a specified KEY object.
 *              The KEY and MASK must refer to persistent objects of the same type where KEY contains the values to compare
 *              against and MASK specifies which part of KEY to consider and how to compare. However, the MASK is not a true
 *              object in that it doesn't need to be created in some table with ss_pers_new(); it can just be allocated on the
 *              stack. Any atomic element of MASK that has at least one bit set indicates that the corresponding element of
 *              KEY is to be considered during the comparison.  If no bits of MASK are set then an error is raised, but if
 *              MASK is the null pointer then we treat the KEY as matching every object in the scope.
 *
 *              If NFOUND is non-null then its incoming value will be used to limit the search to the specified number of
 *              returned matches.  If more items match than what was specified then the additional items are simply ignored as
 *              if they didn't even exist (unless the "detect_overflow" property is true, in which case an error is raised).
 *              The caller can pass in SS_NOSIZE if no limit is desired.  If NFOUND is the null pointer (it can only be so if
 *              BUFFER is also null) then it is treated as if it had pointed to SS_NOSIZE.
 *
 *              The caller can supply a BUFFER for the result or, by passing a null pointer, request that the library allocate
 *              the buffer. If BUFFER is supplied then it must contain at least NFOUND (as set on entry to this function)
 *              elements to hold the result. But if BUFFER is the special constant SS_PERS_TEST then the function behaves as
 *              if a valid BUFFER was supplied except that it does not attempt to initialize that buffer in any way.  This can
 *              be used to count how many matches would be found and even limit the counting by supplying an initial value for
 *              NFOUND.
 *
 *              A positive value for an NSKIP argument causes this function to act as if the first NSKIP matched objects
 *              didn't, in fact, match.
 *
 *
 * Return:      On success this function returns an array of matching persistent object links into the specified SCOPE (the
 *              caller supplied BUFFER or one allocated by the library) or the constant SS_PERS_TEST and NFOUND (if supplied)
 *              will point to the number of matches found limited by the incoming value of NFOUND (or SS_NOSIZE).  If space
 *              permits, the last element of the return value will be followed by a null persistent link, which makes it
 *              possible to loop over the return value even if NFOUND was the null pointer.
 *
 *              In order to distinguish the case where no item is found from the case where an error occurs, the former
 *              results in a non-null return value (the library will allocate an array of size one if the caller didn't supply
 *              a BUFFER and initialize it to SS_PERS_NULL). The NFOUND returned value is zero in either case.
 *
 *              If no objects match in the specified scope and the object type is not ss_scope_t or ss_file_t and the
 *              `noregistries' property is false or not set then each registry scope associated with the file containing SCOPE
 *              will be searched until matches are found in some scope or all registries are processed.
 *
 *              This function returns the null pointer for failure. It is not considered a failure when the KEY simply doesn't
 *              match any of the available objects.
 *
 * Example:     Example 1: Find all fields with an association ratio of 1 in the /main/ scope:
 *
 *                  // Obtain key from a transient scope; allocate the mask on the stack
 *                  ss_field_t *key = SS_PERS_NEW(transient, ss_field_t, SS_ALLSAME);
 *                  ss_fieldobj_t mask;
 *                  // Set key value for which to search and indicate such in the mask
 *                  SS_FIELD(key)->assoc_ratio = 1;
 *                  memset(&mask, 0, sizeof mask);
 *                  mask.assoc_ratio = SS_VAL_CMP_DFLT; //default comparison
 *                  // Search for matches
 *                  size_t nfound = SS_NOSIZE;
 *                  ss_field_t *found = ss_pers_find(main, key, mask, 0, &nfound, NULL, NULL);
 *                  // Print names of all matches
 *                  for (i=0; i<nfound; i++)
 *                      printf("match %d name=\"%s\"\n",i,ss_string_ptr(SS_FIELD_P(found+i,name)));
 *
 *              Example 2: Find first 10 fields with a name consisting of the word "field" in any combination of upper and
 *              lower case letters, followed by one or more digits:
 *              
 *                  // Obtain key from a transient scope; allocate the mask on the stack
 *                  ss_field_t *key = SS_PERS_NEW(transient, ss_field_t, SS_ALLSAME);
 *                  ss_fieldobj_t mask;
 *                  // Set key value for which to search and indicate such in the mask
 *                  ss_string_set(SS_FIELD_P(key,name), "^field[0-9]+$");
 *                  memset(&mask, 0, sizeof mask);
 *                  memset(&(mask.name), SS_VAL_CMP_RE_ICASE, 1);
 *                  // Search for matches
 *                  size_t nfound = 10;
 *                  ss_field_t found[10];
 *                  ss_pers_find(main, key, &mask, 0, &nfound, found, NULL);
 *
 *              Example 3: Count how many fields are in the scope /proc1/:
 *
 *                  ss_field_t *key = ....; // any field object
 *                  size_t nfound = SS_NOSIZE; // do not limit the search
 *                  ss_pers_find(proc1, key, NULL, 0, &nfound, SS_PERS_TEST, NULL);
 *                  printf("found %lu item(s)\n", (unsigned long)nfound);
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
ss_pers_find(ss_scope_t *scope,                 /* Scope to be searched */
             ss_pers_t *key,                    /* Value for which to search. This is required even if MASK is null because
                                                 * the KEY determines the type of objects for which to search. */
             ss_persobj_t *mask,                /* Which elements of KEY to consider when searching. It is an error if no bits
                                                 * of MASK are set, but if MASK is the null pointer then KEY is assumed to
                                                 * match every object. If non-null then MASK and KEY must be of the same
                                                 * type. The reason MASK is an object pointer rather than an object link is
                                                 * that the memory is really only used to store one-byte flags that control
                                                 * how the matching is performed. In other words, MASK isn't truly an
                                                 * object--it just has to be the same size as an object. */
             size_t nskip,                      /* Number of initial matched results that should be skipped. */
             size_t *nfound,                    /* INOUT:  The input value limits the matching to the specified number of
                                                 * objects, and on successful return this points to the number of objects
                                                 * actually found to match. This can be a null pointer as long as BUFFER is
                                                 * a null pointer, but if BUFFER is supplied then the incoming value of NFOUND
                                                 * indicates the number of elements in BUFFER. An incoming value of SS_NOSIZE
                                                 * indicates that the result is not to be truncated. */
             ss_pers_t *buffer,                 /* Optional buffer to fill in with handles to items that were found. If this
                                                 * is the constant SS_PERS_TEST then this function behaves exactly as if the
                                                 * caller had supplied a buffer but does not attempt to return links to the
                                                 * matching objects. */
             ss_prop_t *props                   /* Optional properties (See Persistent Object Properties) */
             )
{
    SS_ENTER(ss_pers_find, ss_pers_tP);
    unsigned tableid;
    ss_table_t *table=NULL;
    ss_pers_find_t find_data;
    ss_scope_t *reg_scope=NULL;
    size_t regidx;
    int noregistries=0;
    ss_gfile_t *gfile=NULL;

    memset(&find_data, 0, sizeof find_data);    /* Necessary for error cleanup */

    SS_ASSERT_MEM(scope, ss_scope_t);
    SS_ASSERT_CLASS(key, ss_pers_t);
    if (buffer && NULL==nfound)
        SS_ERROR_FMT(USAGE, ("if BUFFER is specified then NFOUND must be non-null"));
    
    tableid = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(key));
    if (NULL==(table = ss_scope_table(scope, tableid, NULL))) SS_ERROR(FAILED);

    /* Initialize data to pass through ss_table_scan() */
    if (NULL==(find_data.key = ss_pers_deref(key))) SS_ERROR(NOTFOUND);
    find_data.mask = mask;
    find_data.buffer = buffer;
    find_data.nalloc = buffer ? *nfound : 0;
    find_data.limit = nfound ? *nfound : SS_NOSIZE;
    find_data.nskip = nskip;
    find_data.overflowed = FALSE;
    find_data.scope = scope;
    if (NULL==ss_prop_get(props, "detect_overflow", H5T_NATIVE_INT, &(find_data.detect_overflow))) {
        SS_STATUS_OK;
        find_data.detect_overflow = FALSE;
    }
    
    /* Scan the entire table for matches. We could also check whether ss_table_scan() returned positive, which indicates that
     * we've found more matches than the find_data.limit value. */
    if (ss_table_scan(table, NULL, 0, ss_pers_find_cb, &find_data)<0) SS_ERROR(FAILED);
    if (find_data.detect_overflow && find_data.overflowed) SS_ERROR(OVERFLOW);

    /* Search registries until we find something. The tables that describe the file infrastructure do not need to search
     * registries because doing so doesn't really make any sense: Scope, File, and Blob tables. */
    if (0==find_data.nused &&
        SS_MAGIC_OF(key)!=SS_MAGIC(ss_scope_t) &&
        SS_MAGIC_OF(key)!=SS_MAGIC(ss_file_t) &&
        SS_MAGIC_OF(key)!=SS_MAGIC(ss_blob_t) &&
        (NULL==ss_prop_get(props, "noregistries", H5T_NATIVE_INT, &noregistries) || !noregistries)) {
        SS_STATUS_OK; /*clean up from possible failed ss_prop_get()*/
        if (NULL==(gfile = SS_GFILE_LINK(scope))) SS_ERROR(FAILED);
        for (regidx=0; 0==find_data.nused && regidx<gfile->reg_nused; regidx++) {
            reg_scope = gfile->reg + regidx;
            if (NULL==(table = ss_scope_table(reg_scope, tableid, NULL))) SS_ERROR(FAILED);
            find_data.scope = reg_scope;
            if (ss_table_scan(table, reg_scope, 0, ss_pers_find_cb, &find_data)<0) SS_ERROR(FAILED);
            if (find_data.detect_overflow && find_data.overflowed) SS_ERROR(OVERFLOW);
        }
    }
    
    /* If no matches were found but we were otherwise successful then make sure we return a non-null value */
    if (nfound) *nfound = find_data.nused;
    if (0==find_data.nused && NULL==find_data.buffer) {
        find_data.buffer = calloc(1, sizeof(ss_pers_t));
        find_data.nalloc = 1;
    }
    
    /* If there is room then set the link after the last one to null */
    if (!buffer) SS_EXTEND(find_data.buffer, find_data.nused+1, find_data.nalloc);
    if (find_data.nused<find_data.nalloc && SS_PERS_TEST!=find_data.buffer)
        memset(find_data.buffer + find_data.nused, 0, sizeof(find_data.buffer[0]));
        

SS_CLEANUP:
    if (!buffer) SS_FREE(find_data.buffer);
    if (nfound) *nfound = 0;
    SS_LEAVE(find_data.buffer);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Callback for ss_pers_find()
 *
 * Description: This function is invoked on every object of a table, comparing that object to the key and mask supplied in
 *              _FIND_DATA and if the object matches, adding a link to that object to the array supplied in _FIND_DATA.
 *
 * Return:      Returns false on success; negative on failure.  Returns true (positive) if more than the maximum number of
 *              matches is found (the check only happens if find_data.detect_overflow is true).
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static htri_t
ss_pers_find_cb(size_t UNUSED itemidx, ss_pers_t UNUSED *_pers, ss_persobj_t *persobj, void *_find_data)
{
    SS_ENTER(ss_pers_find_cb, htri_t);
    ss_pers_find_t      *find_data = (ss_pers_find_t*)_find_data;
    int                 cmp;
    htri_t              retval=FALSE;

    /* We calculate this here if needed instead of passing in through the argument list because most of the time we don't need
     * it. Calculating `pers' for every call to this function (via ss_pers_refer()) turns out to be quite expensive. */
    ss_pers_t           pers=SS_PERS_NULL;

    SS_ASSERT(0==(itemidx & SS_TABLE_INDIRECT));

    if (find_data->mask) {
        cmp = ss_pers_cmp_(persobj, find_data->key, find_data->mask);
        if (-2 == cmp) SS_ERROR(FAILED); /* special error return value for comparison functions */
    } else {
        cmp = 0;
    }
    
    if (0==cmp) {
        if (find_data->nskip>0) {
            --(find_data->nskip);
            goto done;
        }
        if (find_data->nused >= find_data->limit) {
            /* We've now found more items that what the caller specified was the limit.  Immediately abort the scan by
             * returning a positive value. */
            find_data->overflowed = TRUE;
            retval = TRUE;
            goto done;
        }

        /* Extend the buffer if necessary. We have to be careful not to extend if the caller supplied the buffer. */
        SS_EXTEND(find_data->buffer, MINMAX(find_data->nused+1, 32, find_data->limit), find_data->nalloc);

        /* Add this item to the result buffer unless we're only counting matches */
        if (SS_PERS_TEST!=find_data->buffer) {
            if (NULL==ss_pers_refer(find_data->scope, persobj, &pers)) SS_ERROR(FAILED);
            find_data->buffer[find_data->nused] = pers;
        }
        find_data->nused++;

        /* Did we find enough matches? If the caller wants us to detect overflow then we have to keep going. */
        if (find_data->nused>=find_data->limit && !find_data->detect_overflow) {
            retval = TRUE;
            goto done;
        }
    }

done:
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Mark object as modified
 *
 * Description: If a persistent object is modified then it should also be marked as such by invoking this function. If all
 *              tasks modify the persistent object in the same manner then the second argument can be SS_ALLSAME, otherwise it
 *              should be zero.  The SS_PERS_MODIFIED() macro is a convenience for this function since the client is often
 *              passing a subclass of ss_pers_t and may get compiler warnings.
 *
 *              The client can call this function either before or after making a modification to the object, but it's
 *              generally safer to make this call first so that the object is marked as modified even if the modification is
 *              interrupted by an error. It doesn't hurt to mark an object as modified and then not actually modify it -- it
 *              just causes the synchronization algorithm to take longer to discover that there weren't any changes.
 *
 *              The `dirty' flag is always set to true to indicate that the object's new value differs (or is about to differ)
 *              from what is stored in the file.
 *
 *              If FLAGS has the SS_ALLSAME bit set then the client is indicating that all tasks belonging to the scope have
 *              (or will) make identical modifications. In this case, if the object's `synced' flag is set we promote it to
 *              SS_ALLSAME to indicate that the object is synchronized but its sync_cksum and sync_serial values are outdated.
 *              Otherwise the object's `synced' flag is set to false.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Issue:       We should really check whether the scope owning the object is read-only, otherwise we won't get any indication
 *              of an error until we try to synchronize.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Sunday, November 23, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_modified(ss_pers_t *pers,               /* Persistent object to mark as modified */
                 unsigned flags                 /* Bitflags such as SS_ALLSAME */
                 )
{
    SS_ENTER(ss_pers_modified, herr_t);
    ss_persobj_t *persobj = ss_pers_deref(pers);

    if (!persobj) SS_ERROR(NOTFOUND);
    persobj->dirty = TRUE;
    persobj->synced = ((flags & SS_ALLSAME) && persobj->synced) ? SS_ALLSAME : FALSE;

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Make a new object unique
 *
 * Description: If N MPI tasks each create an object that is identical across all the tasks (such as happens when
 *              saf_declare_set() is called with all arguments the same) then SSlib will merge those N new objects into a
 *              single permanent object. A similar thing happens when a single tasks creates multiple new identical objects.
 *              The merging happens during a synchronization operation and only for objects that are new (i.e., objects that
 *              were not created with the SS_ALLSAME bit flag).
 *
 *              By calling this function on a persistent object, the persistent object is modified in a unique manner which
 *              causes it to be different than any other object on this MPI task or any other MPI task.  The uniqueness should
 *              only be set for new objects.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, May 27, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_unique(ss_pers_t *pers                  /* Persistent object to make unique */
               )
{
    SS_ENTER(ss_pers_unique, herr_t);
    ss_persobj_t *persobj = ss_pers_deref(pers);

    if (!persobj) SS_ERROR(NOTFOUND);
    if (0==(persobj->mapidx & SS_TABLE_INDIRECT)) SS_ERROR_FMT(USAGE, ("only new objects can be marked as unique"));
    persobj->saf_each = sslib_g.serial++;
    persobj->dirty = TRUE;
    persobj->synced = FALSE;

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Display object link details
 *
 * Description: This is a debugging routine for displaying all known details about a persistent object link, PERS. The output
 *              is to the OUT file and all lines of output begin with `SSlib-%d:' followed by a space where `%d' is replaced
 *              by the callers rank in MPI_COMM_WORLD.  The first line of output will include a call to printf() with FMT and
 *              the variable arguments (FMT should not end with a line feed or colon as this function will add them). Each of
 *              the following lines will include information about the link preceded by PREFIX, which is usually used just to
 *              supply some extra white space.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, December  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_dump(ss_pers_t *pers,                   /* Link being displayed */
             FILE *out,                         /* Output stream */
             const char *prefix,                /* Optional prefix string for each line of output (except header line) */
             const char *fmt,                   /* Optional printf() format string for first line of output */
             ...                                /* Arguments to pass to printf() for the FMT string */
             )
{
    SS_ENTER(ss_pers_dump, herr_t);
    va_list ap;
    herr_t retval=-1;

    va_start(ap, fmt);
    retval = ss_pers_dumpv(pers, out, prefix, fmt, ap);
    va_end(ap);
    if (retval<0) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Debugging aid
 *
 * Description: Prints all known information about an object to the standard output stream.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August 27, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_debug(ss_pers_t *pers)
{
    SS_ENTER(ss_pers_debug, herr_t);
    if (ss_pers_dump(pers, stdout, NULL, NULL)<0) SS_ERROR(FAILED);
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Display object link details
 *
 * Description: This is the va_list version of ss_pers_dump(). You'll probably want to use ss_pers_dump() instead.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December  5, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_dumpv(ss_pers_t *pers,                  /* Link being displayed */
              FILE *out,                        /* Output stream */
              const char *prefix,               /* Optional prefix string for each line of output. If this string begins with
                                                 * a `\001' then the prefix is printed only on the first line of output and
                                                 * all other lines will use an equivalent amount of white space instead. */
              const char *fmt,                  /* Part of header line output */
              va_list ap                        /* Extra arguments for header line */
              )
{
    SS_ENTER(ss_pers_dumpv, herr_t);
    ss_persobj_t        *persobj = NULL;
    const int           fldsize = 16;
    ss_scope_t          scope=SS_SCOPE_NULL;
    size_t              idx, indirect_idx;
    ss_pers_class_t     *pc=NULL;
    ss_val_cksum_t      cksum;
    char                intro[32], white_space[128];
    int                 self;
    unsigned            seq, nmembs, membno;
    char                *memb_name=NULL;
    hid_t               memb_type=-1;
    size_t              memb_off;
    hbool_t             first_only=FALSE;
    ss_table_t          *table=NULL;

    if (!out) out = stderr;
    if (!prefix) prefix = "";
    if ('\001'==*prefix) {
        first_only = TRUE;
        prefix++;
    }
    self = ss_mpi_comm_rank(SS_COMM_WORLD);
    sprintf(intro, "SSlib-%d: ", self);

    /* Optional header line */
    if (fmt) {
        fputs(intro, out);
        vfprintf(out, fmt, ap);
        fputs(":\n", out);
    }

    /* Link state and address */
    seq = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(pers));
    if (pers) pc = SS_PERS_CLASS(seq);
    if (!pers) {
        fprintf(out, "%s%s%-*s null pointer\n", intro, prefix, fldsize, "link:");
        goto done;
    } else if (SS_PERS_LINK_NULL==ss_pers_link_state(pers)) {
        fprintf(out, "%s%s%-*s 0x%08lx null %s(%u)\n", intro, prefix, fldsize, "link:", (unsigned long)pers, pc->name, seq);
        goto done;
    } else if (SS_PERS_LINK_RESERVED==ss_pers_link_state(pers)) {
        fprintf(out, "%s%s%-*s 0x%08lx mangled %s(%u)\n", intro, prefix, fldsize, "link:", (unsigned long)pers, pc->name, seq);
        goto done;
    } else if (SS_PERS_LINK_CLOSED==ss_pers_link_state(pers)) {
        fprintf(out, "%s%s%-*s 0x%08lx closed %s(%u)\n", intro, prefix, fldsize, "link:", (unsigned long)pers, pc->name, seq);
    } else {
        fprintf(out, "%s%s%-*s 0x%08lx memory %s(%u)\n", intro, prefix, fldsize, "link:", (unsigned long)pers, pc->name, seq);
    }

    /* Replace the prefix with white space now that we printed the first line */
    if (first_only) {
        assert(strlen(prefix)<sizeof white_space);
        memset(white_space, ' ', strlen(prefix));
        white_space[strlen(prefix)] = '\0';
        prefix = white_space;
    }
    
    /* File information */
    idx = ss_pers_link_gfileidx(pers);
    ss_gfile_debug_one(idx, out, prefix);
    if (!SS_GFILE_IDX(idx)) goto done;

    /* The scope */
    fprintf(out, "%s%s%-*s", intro, prefix, fldsize, "scope:");
    ss_pers_scope(pers, &scope);
    idx = ss_pers_link_scopeidx(pers);
    fprintf(out, " %s(%lu) 0x%08lx",
            (idx & SS_TABLE_INDIRECT)?"indirect":"direct", (unsigned long)(idx & ~SS_TABLE_INDIRECT),
            (unsigned long)SS_SCOPE(&scope));
    if (!SS_SCOPE(&scope)) {
        fprintf(out, " [cannot dereference]\n");
        goto done;
    }
    if (SS_COMM_NULL==SS_SCOPE(&scope)->m.comm) {
        fprintf(out, " closed\n");
        goto done;
    }
    if (0==SS_SCOPE(&scope)->m.gid) fprintf(out, " hdf5(closed)");
    else if (1==SS_SCOPE(&scope)->m.gid) fprintf(out, " hdf5(transient)");
    else if (SS_SCOPE(&scope)->m.gid<0) fprintf(out, " hdf5(error)");
    else fprintf(out," hdf5(%lu)", (unsigned long)(SS_SCOPE(&scope)->m.gid));
    fprintf(out, " \"%s\"\n", ss_string_ptr(SS_SCOPE_P(&scope,name)));

    /* The object index */
    fprintf(out, "%s%s%-*s", intro, prefix, fldsize, "object:");
    idx = ss_pers_link_objidx(pers);
    fprintf(out, " %s(%lu)", (idx & SS_TABLE_INDIRECT)?"indirect":"direct", (unsigned long)(idx & ~SS_TABLE_INDIRECT));

    /* The indices that point to the same object */
    if (NULL==(table=ss_scope_table(&scope, seq, NULL))) {
        fprintf(out, " <table-error>");
    } else {
        indirect_idx = 0;
        while (SS_NOSIZE!=(indirect_idx=ss_table_indirect(table, idx, indirect_idx))) {
            if (indirect_idx==idx) continue;
            fprintf(out, " indirect(%lu)", (unsigned long)(indirect_idx & ~SS_TABLE_INDIRECT));
        }
    }
    
    /* The object memory */
    if (SS_PERS_LINK_MEMORY!=ss_pers_link_state(pers)) {
        fprintf(out, "\n");
        goto done;
    }
    persobj = ss_pers_deref(pers);
    if (!persobj) {
        fprintf(out, " [deref error]\n");
        goto done;
    }
    fprintf(out, " 0x%08lx %ssynchronized%s %sdirty\n", (unsigned long)persobj, persobj->synced?"":"!",
            SS_ALLSAME==persobj->synced?"(ALLSAME)":"", persobj->dirty?"":"!");

    /* Checksums */
    fprintf(out, "%s%s%-*s", intro, prefix, fldsize, "checksums:");
    fprintf(out, " sync%u=0x%08lx-%02x", persobj->sync_serial, persobj->sync_cksum.adler, persobj->sync_cksum.flags);
    if (ss_pers_cksum(persobj, &cksum)<0) SS_ERROR(FAILED);
    fprintf(out, " current=0x%08lx-%02x", cksum.adler, cksum.flags);
    fprintf(out, "\n");

    /* The object contents */
    nmembs = H5Tget_nmembers(pc->tfm);
    for (membno=0; membno<nmembs; membno++) {
        memb_name = H5Tget_member_name(pc->tfm, membno);
        memb_type = H5Tget_member_type(pc->tfm, membno);
        memb_off = H5Tget_member_offset(pc->tfm, membno);
        fprintf(out, "%s%s  %-*s", intro, prefix, fldsize-1, memb_name);
        ss_val_dump((char*)persobj+memb_off, memb_type, pers, out, NULL);
        fprintf(out, "\n");
        memb_name = SS_FREE(memb_name);
        if (H5Tclose(memb_type)<0) SS_ERROR(HDF5);
        memb_type=-1;
    }
    
done:
    SS_STATUS_OK;
SS_CLEANUP:
    SS_FREE(memb_name);
    if (memb_type>0) H5Tclose(memb_type);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Display object details
 *
 * Description: Identical to ss_pers_dump() except it takes a scope and object pointer instead of a link.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, December  3, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_dump_(ss_persobj_t *persobj, ss_scope_t *scope, FILE *out, const char *prefix, const char *fmt, ...)
{
    SS_ENTER(ss_pers_dump_, herr_t);
    ss_pers_t           pers;
    va_list             ap;
    herr_t              retval;

    if (NULL==ss_pers_refer(scope, persobj, &pers)) SS_ERROR(FAILED);
    va_start(ap, fmt);
    retval = ss_pers_dumpv(&pers, out, prefix, fmt, ap);
    va_end(ap);
    if (retval<0) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Convert object link to file type
 *
 * Description: This function converts a single persistent object link from memory format SRC to file format _DST. The
 *              pointers must not point to overlapping areas of memory.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. However, this function may create permanent File objects in the current scope from scope task
 *              zero and the caller must therefore collectively call ss_table_broadcast() on the File table to make sure the
 *              other tasks of the scope get the same data.
 *
 * Programmer:  Robb Matzke
 *              Monday, March 15, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_convert_mf2(ss_pers_t *src,             /* Persistent object link in memory format. */
                    void *_dst,                 /* Persistent object link in file format. */
                    ss_table_t *filetab         /* Optional File table for the scope to which the persistent object link
                                                 * belongs. */
                    )
{
    SS_ENTER(ss_pers_convert_mf2, herr_t);
    char                *dst=_dst;
    char                *name=NULL;             /* Name to associate with the new file object */
    size_t              file_idx=SS_NOSIZE, scope_idx=SS_NOSIZE, item_idx=SS_NOSIZE;
    unsigned            table_idx=0x100;        /* Invalid since larger than 0xff */
    hbool_t             found;
    ss_fileobj_t        *fileobj=NULL;          /* An object from the FILETAB table. */
    ss_file_t           file;                   /* A temporary link to `fileobj' */
    ss_gfile_t          *gfile=NULL, *gfile_dst=NULL;
    ss_table_t          *table=NULL;
    ss_scopeobj_t       *scopeobj=NULL;
    ss_scope_t          scope=SS_SCOPE_NULL;

    if (!filetab && NULL==(filetab=ss_scope_table(&ss_table_scope_g, SS_MAGIC(ss_file_t), NULL))) SS_ERROR(FAILED);

    if (SS_PERS_LINK_NULL==ss_pers_link_state(src)) {
        file_idx = 0xffff;              /* bytes 0, 1 */
        scope_idx = 0xffffffff;         /* bytes 2, 3, 4, 5 */
        table_idx = 0xff;               /* byte 6 (this is really the only important one) */
        item_idx = 0xffffffff;          /* bytes 7, 8, 9, 10 */
    } else if (SS_PERS_LINK_RESERVED==ss_pers_link_state(src)) {
        SS_ERROR_FMT(CORRUPT, ("mangled persistent link"));
    } else {
        /* File index. We look in the File table of the current scope to see if it has an entry for the destination file (i.e.,
         * the file that owns the object). */
        for (file_idx=0, found=FALSE; !found; file_idx++) {
            if (NULL==(fileobj=(ss_fileobj_t*)ss_table_lookup(filetab, file_idx, 0))) SS_ERROR(FAILED);
            if (SS_MAGIC_OF(fileobj)!=SS_MAGIC(ss_fileobj_t) || (fileobj->m.pers.mapidx & SS_TABLE_INDIRECT)) break;
            if (fileobj->m.gfileidx==ss_pers_link_gfileidx(src)) {
                found = TRUE;
                break;
            }
        }

        /* If we didn't find a valid entry in the File table of the current scope then we must add one now. We are currently in
         * an independent mode but need to get a permanent table index for the new file object. Our only choice is to create an
         * object with SS_ALLSAME but delay the collectivity point until we return back up into ss_table_write(), which is the
         * *only* function from which this function is called (indirectly via H5Dwrite()). The ss_table_write() should then
         * call ss_table_broadcast() for the File table. */
        if (!found) {
            if (NULL==ss_pers_new(&ss_table_scope_g, SS_MAGIC(ss_file_t), NULL, SS_ALLSAME, (ss_pers_t*)&file, NULL))
                SS_ERROR(FAILED);
            if (NULL==(gfile_dst=SS_GFILE_LINK(src))) SS_ERROR(NOTFOUND);
            if (NULL==(gfile=SS_GFILE_LINK(&ss_table_scope_g))) SS_ERROR(NOTFOUND); /*the scope being written*/
            if (gfile_dst->flags & H5F_ACC_TRANSIENT) {
                /* Store the absolute name */
                ss_string_set(SS_FILE_P(&file,name), gfile_dst->name);
            } else {
                /* Use a destination file name that is relative to the current file name. */
                if (NULL==(name=ss_file_fixname(gfile_dst->name, NULL, gfile->name, 0, NULL))) SS_ERROR(FAILED);
                if (ss_string_set(SS_FILE_P(&file,name), name)<0) SS_ERROR(FAILED);
                name = SS_FREE(name);
            }
            SS_FILE(&file)->m.gfileidx = ss_pers_link_gfileidx(src);
            file_idx = ss_pers_link_objidx(&file);
        }

        /* scope index */
        scope_idx = ss_pers_link_scopeidx(src);

        /* table index */
        table_idx = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(src));

        /* item index */
        item_idx = ss_pers_link_objidx(src);

        /* If the item index is indirect then we need to convert it to a direct index. There are two possibilities:
         *   1. If the table is for a scope that is currently open then dereferencing the object will update the object index
         *      to a direct index if and only if the object can be dereferenced and is not a new, never-synchronized object. 
         *   2. If the table is for a scope that's been closed then the table no longer has a reference to the object but it
         *      still contains enough information to convert the indirect index into a direct index. */
        if (item_idx & SS_TABLE_INDIRECT) {
            gfile_dst = SS_GFILE_LINK(src);
            SS_ASSERT(gfile_dst);
            SS_ASSERT(gfile_dst->topscope);
            if (gfile_dst->cur_open>0) {
                (void)ss_pers_deref(src);
                item_idx = ss_pers_link_objidx(src);
                if (item_idx & SS_TABLE_INDIRECT) SS_ERROR(NOTIMP); /*object is still new, not much to write*/
            } else {
                if (NULL==(table=ss_scope_table(gfile_dst->topscope, SS_MAGIC(ss_scope_t), NULL))) SS_ERROR(FAILED);
                if (NULL==(scopeobj=(ss_scopeobj_t*)ss_table_lookup(table, scope_idx, SS_STRICT))) SS_ERROR(FAILED);
                if (NULL==ss_pers_refer(gfile_dst->topscope, (ss_persobj_t*)scopeobj, (ss_pers_t*)&scope)) SS_ERROR(FAILED);
                if (NULL==(table=ss_scope_table(&scope, table_idx, NULL))) SS_ERROR(FAILED);
                if (SS_NOSIZE==(item_idx=ss_table_direct(table, item_idx))) SS_ERROR(FAILED);
            }
        }
    }

    /* Encode. Truncation of r.h.s. values is intended. */
    SS_ASSERT(file_idx<=0xffff);
    SS_ASSERT(scope_idx<=0xffffffff);
    SS_ASSERT(table_idx<=0xff);
    SS_ASSERT(item_idx<=0xffffffff);

    dst[0] = (unsigned char)(file_idx & 0xff);
    dst[1] = (unsigned char)((file_idx>>8) & 0xff);

    dst[2] = (unsigned char)(scope_idx & 0xff);
    dst[3] = (unsigned char)((scope_idx>>8) & 0xff);
    dst[4] = (unsigned char)((scope_idx>>16) & 0xff);
    dst[5] = (unsigned char)((scope_idx>>24) & 0xff);

    dst[6] = (unsigned char)(table_idx & 0xff);
            
    dst[7] = (unsigned char)(item_idx & 0xff);
    dst[8] = (unsigned char)((item_idx>>8) & 0xff);
    dst[9] = (unsigned char)((item_idx>>16) & 0xff);
    dst[10]= (unsigned char)((item_idx>>24) & 0xff);

SS_CLEANUP:
    SS_FREE(name);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Convert object link to file type
 *
 * Description: This is an H5Tconvert() callback to convert ss_pers_t object links in memory into the file representation.
 *
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. However, this function may create permanent File objects in the current scope from scope task
 *              zero and the caller must therefore collectively call ss_table_broadcast() on the File table to make sure the
 *              other tasks of the scope get the same data.
 *
 * Programmer:  Robb Matzke
 *              Monday, December  8, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_pers_convert_mf(hid_t UNUSED src_type,       /* Source data type (should be ss_pers_tm) */
                   hid_t UNUSED dst_type,       /* Destination data type (should be ss_pers_tf) */
                   H5T_cdata_t *cdata,          /* Data type conversion control data */
                   size_t nelmts,               /* Number of elements to convert */
                   size_t buf_stride,           /* Distance between elements in bytes */
                   size_t UNUSED bkg_stride,    /* Distance between elemenents in the background buffer */
                   void *buf,                   /* The buffer which is to be converted in place */
                   void UNUSED *bkg,            /* The background buffer. */
                   hid_t UNUSED dxpl            /* Dataset transfer property list. */
                   )
{
    SS_ENTER(ss_pers_convert_mf, herr_t);
    hsize_t             elmtno;                 /* Counter through the elements to be converted */
    char                *src = buf;             /* Source values */
    char                *dst = buf;             /* Destination pointer */
    size_t              dst_size=11;            /* Size in bytes of the destination type */
    ss_pers_t           tmp;                    /* Temporary for non-overlapping conversion */
    ss_table_t          *filetab=NULL;

    SS_ASSERT(cdata);

    switch (cdata->command) {
    case H5T_CONV_INIT:
        /* See ss_pers_init() for definitions of these types */
        SS_ASSERT(H5Tequal(src_type, ss_pers_tm));
        SS_ASSERT(H5Tequal(dst_type, ss_pers_tf));
        SS_ASSERT(H5Tget_size(src_type)>H5Tget_size(dst_type)); /* determines traversal direction */
        SS_ASSERT(dst_size==H5Tget_size(dst_type));
        break;

    case H5T_CONV_FREE:
        break;

    case H5T_CONV_CONV:
        if (NULL==(filetab=ss_scope_table(&ss_table_scope_g, SS_MAGIC(ss_file_t), NULL))) SS_ERROR(FAILED);
        for (elmtno=0; elmtno<nelmts; elmtno++) {
            memcpy(&tmp, src, sizeof tmp);
            if (ss_pers_convert_mf2(&tmp, dst, filetab)<0) SS_ERROR(FAILED);
            src += buf_stride ? buf_stride : sizeof tmp;
            dst += buf_stride ? buf_stride : dst_size;
        }
        break;

    default:
        SS_ERROR_FMT(HDF5, ("unexpected value for cdata->command"));
    }
    
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Convert link to memory type
 *
 * Description: This function converts a single persistent object link from the file representation in _SRC to the memory
 *              representation in DST. The _SRC and DST must not overlap.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, March 15, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_convert_fm2(void *_src,                 /* File representation of a persistent object link. */
                    ss_pers_t *dst,             /* OUT: Memory representation of the persistent object link */
                    ss_table_t *filetab         /* Optional pointer to the file table in the scope to which the persistent
                                                 * object link that we're converting belongs. The only purpose of this
                                                 * argument is to prevent this function from having to look it up each time
                                                 * it's called. */
                    )
{
    SS_ENTER(ss_pers_convert_fm2, herr_t);
    const char          *src = _src;
    size_t              file_idx, scope_idx, item_idx;
    unsigned            table_idx;
    ss_fileobj_t        *fileobj=NULL;

    if (!filetab && NULL==(filetab=ss_scope_table(&ss_table_scope_g, SS_MAGIC(ss_file_t), NULL))) SS_ERROR(FAILED);

    SS_H5_DECODE(src, 2, file_idx);
    SS_H5_DECODE(src, 4, scope_idx);
    SS_H5_DECODE(src, 1, table_idx);
    SS_H5_DECODE(src, 4, item_idx);

    memset(dst, 0, sizeof(*dst));
    if (table_idx != 0xff) {
        if (NULL==(fileobj=(ss_fileobj_t*)ss_table_lookup(filetab, file_idx, 0))) SS_ERROR(FAILED);
        if (NULL==ss_obj_new((ss_obj_t*)dst, SS_MAGIC_CONS(SS_MAGIC(ss_pers_t), table_idx), sizeof(ss_pers_t), dst))
            SS_ERROR(FAILED);
        ss_pers_link_setstate(dst, SS_PERS_LINK_CLOSED);
        ss_pers_link_setgfileidx(dst, fileobj->m.gfileidx);
        ss_pers_link_setscopeidx(dst, scope_idx);
        ss_pers_link_setobjidx(dst, item_idx);
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Convert object link to memory type
 *
 * Description: This is an H5Tconvert() callback to convert ss_pers_t object links from a file into the memory representation.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, December  8, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_pers_convert_fm(hid_t UNUSED src_type,       /* Source data type (should be ss_pers_tm) */
                   hid_t UNUSED dst_type,       /* Destination data type (should be ss_pers_tf) */
                   H5T_cdata_t *cdata,          /* Data type conversion control data */
                   size_t nelmts,               /* Number of elements to convert */
                   size_t buf_stride,           /* Distance between elements in bytes */
                   size_t UNUSED bkg_stride,    /* Distance between elemenents in the background buffer */
                   void *buf,                   /* The buffer which is to be converted in place */
                   void UNUSED *bkg,            /* The background buffer. */
                   hid_t UNUSED dxpl            /* Dataset transfer property list. */
                   )
{
    SS_ENTER(ss_pers_convert_fm, herr_t);
    size_t              src_size=11;            /* Size in bytes of the source (file) datatype */
    char                *src=NULL;              /* Source data buffer */
    char                *dst=NULL;              /* Destination data buffer */
    ss_pers_t           tmp;                    /* Temporary for non-overlapping conversions */
    ss_table_t          *filetab=NULL;          /* File table from the scope being read */
    size_t              elmtno;

    SS_ASSERT(cdata);

    switch (cdata->command) {
    case H5T_CONV_INIT:
        /* See ss_pers_init() for definitions of these types */
        SS_ASSERT(H5Tequal(src_type, ss_pers_tf));
        SS_ASSERT(H5Tequal(dst_type, ss_pers_tm));
        SS_ASSERT(H5Tget_size(src_type)<H5Tget_size(dst_type)); /*determines traversal direction*/
        SS_ASSERT(src_size==H5Tget_size(src_type));
        break;

    case H5T_CONV_FREE:
        break;

    case H5T_CONV_CONV:
        if (NULL==(filetab=ss_scope_table(&ss_table_scope_g, SS_MAGIC(ss_file_t), NULL))) SS_ERROR(FAILED);
        src = (char*)buf + (nelmts-1) * (buf_stride ? buf_stride : src_size);
        dst = (char*)buf + (nelmts-1) * (buf_stride ? buf_stride : sizeof tmp);
        for (elmtno=0; elmtno<nelmts; elmtno++) {
            if (ss_pers_convert_fm2(src, &tmp, filetab)<0) SS_ERROR(FAILED);
            memcpy(dst, &tmp, sizeof tmp);
            src -= buf_stride ? buf_stride : src_size;
            dst -= buf_stride ? buf_stride : sizeof tmp;
        }
        break;

    default:
        SS_ERROR_FMT(HDF5, ("unexpected value for cdata->command"));
    }
    
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Checksum persistent object links
 *
 * Description: Compute a checksumm for zero or more ss_pers_t objects.  The links are converted to the closed state first and
 *              the GFile's serial number is used in place of the GFile index (since the former is task independent).
 *
 * Return:      Returns non-negative on success; negative on failure. On success, CKSUM is updated by folding in the new
 *              checksum value.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, November 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_pers_sum_cb(void *buffer,                    /* Address of first object to checksum. */
               size_t UNUSED size,              /* Size in bytes of each element of BUFFER. */
               size_t nelmts,                   /* Number of objects to checksum. */
               ss_val_cksum_t *cksum            /* INOUT: Checksum value to be updated. */
               )
{
    SS_ENTER(ss_pers_sum_cb, herr_t);
    ss_pers_t           *pers=(ss_pers_t*)buffer;
    ss_gfile_t          *gfile=NULL;
    ss_pers_t           tmp;
    size_t              i, sumsize;
    void                *sumstart=NULL;
    ss_persobj_t        *persobj=NULL;
    ss_pers_class_t     *pc=NULL;
    htri_t             oldmark=-1;

    SS_ASSERT(size==sizeof tmp);
    for (i=0; i<nelmts; i++) {
        if (!SS_PERS_ISNULL(pers+i)) {
            /* If the link points to a new (local) object then instead of computing a checksum for the link we'll follow the
             * link and compute a checksum for the object to which it points. This is necessary because checksums should be
             * the same across tasks but there's no global information for a new (local) link. By computing a checksum on the
             * object to which the link points the tasks can at least determine if the object's values are the same. However,
             * in following the links we need to be careful to avoid cycles in the graph. So each object has a boolean
             * `marked' bit that we can use for that purpose. Be careful to reset the bit to false! */
            if (NULL==(persobj=ss_pers_deref(pers+i))) SS_ERROR(FAILED);
            oldmark = persobj->marked;

            /* Copy link into temporary memory and make it a `closed' link with up-to-date object index. It might be better to
             * just update the BUFFER itself, but we're going to swap out some parts of it when checksuming anyway and will
             * need to copy the memory sooner or later. */
            tmp = pers[i];
            if (ss_pers_state(&tmp, SS_PERS_LINK_CLOSED)<0) SS_ERROR(FAILED);
            
            if (ss_pers_link_objidx(&tmp) & SS_TABLE_INDIRECT) {
                if (!persobj->marked) {
                    persobj->marked = TRUE;
                    /* If the linked-to object is new then set the `new' bit in the checksum. The call to ss_pers_state() above has
                     * already guaranteed that the object index will be direct if possible. */
                    cksum->flags |= SS_VAL_CKSUM_NEW;
                    pc = SS_PERS_CLASS(SS_MAGIC_SEQUENCE(SS_MAGIC_OF(persobj)));
                    SS_ASSERT(pc);
                    if (ss_val_cksum(persobj, pc->valinfo_nused, pc->valinfo, cksum, cksum)<0) SS_ERROR(FAILED);
                }
            } else {
                /* Swap out the GFile index with the GFile serial number before computing the checksum. At this point TMP is
                 * no longer a valid persistent object link!!! */
                gfile = SS_GFILE_LINK(&tmp);
                SS_ASSERT(gfile);
                ss_pers_link_setgfileidx(&tmp, gfile->serial);

                /* Checksum the TMP memory, excluding the tmp.obj member */
                sumstart = (char*)&tmp + sizeof(tmp.obj);
                sumsize = sizeof(tmp) - sizeof(tmp.obj);
                cksum->adler = ss_adler32(cksum->adler, sumstart, sumsize);
            }
            
            persobj->marked = oldmark;
        }
    }

SS_CLEANUP:
    if (persobj) {
        assert(oldmark>=0);
        persobj->marked = oldmark;
    }
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Compare persistent object links
 *
 * Description: Pairwise comparison of zero or more objects of the _BUF array with the _KEY array. The _KEY describes the value
 *              for which we're searching and the FLAGS says how to search.
 *
 * Return:      On success returns negative if _BUF is less than _KEY, zero if they are equal, and one if _BUF is greater than
 *              _KEY. Returns -2 on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, November 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_pers_cmp_cb(void *_buf,                      /* Haystack array. */
               void *_key,                      /* Array of values for which we are searching. */
               unsigned flags,                  /* How to search (see SS_VAL_CMP). */
               size_t UNUSED size,              /* Size of each object in the BUF and KEY arrays. */
               size_t nelmts                    /* Number of elements in the BUF and KEY arrays. */
               )
{
    SS_ENTER(ss_pers_cmp_cb, int);
    int                 retval = 0;
    size_t              i;

    SS_RETVAL(-2);
    SS_ASSERT(size==sizeof(ss_pers_t));

    for (i=0, retval=0; i<nelmts && 0==retval; i++) {
        ss_pers_t *buf = (ss_pers_t*)_buf+i;
        ss_pers_t *key = (ss_pers_t*)_key+i;

        if (SS_VAL_CMP_EQUAL == (flags & 0xF0)) {
            static int ncalls=0;
            if (0==ncalls++) {
                fprintf(sslib_g.warnings, "In %s() at %s:%u: falling back to shallow comparison (equal not implemented)\n",
                        _func, __FILE__, __LINE__);
            }
        }

        if (-2==(retval=ss_pers_cmp(buf, key, NULL))) SS_ERROR(FAILED);
    }
    
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Persistent Objects
 * Purpose:     Encode persistent object link
 *
 * Description: Encode a link so it can be transmitted to another task.  The following things are encoded for transmission:
 *
 *              * The sequence part of the magic number, which defines the table to which the link points (1 byte)
 *              * The file serial number, which is used to determine which GFile entry is referenced (sizeof(size_t))
 *              * The scope index (four bytes)
 *              * The item index (four bytes)
 *
 * Return:      Returns the (possibly reallocated) SERBUF on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, November 21, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
ss_pers_encode_cb(void *buffer,                         /* Array of objects to be encoded. */
                  char *serbuf,                         /* Buffer into which to place results. */
                  size_t *serbuf_nused,                 /* INOUT: Number of bytes actually used in SERBUF. */
                  size_t *serbuf_nalloc,                /* INOUT: Number of bytes allocated for SERBUF. */
                  size_t UNUSED size,                   /* Size of each object in BUFFER. */
                  size_t nelmts                         /* Number of elements in BUFFER. */
                )
{
    SS_ENTER(ss_pers_encode_cb, charP);
    const ss_pers_t     *pers = (const ss_pers_t*)buffer;
    ss_gfile_t          *gfile=NULL;                            /* GFile array entry for the object link */
    size_t              nbytes=9+sizeof(gfile->serial);         /* Max number of bytes encoded herein */
    size_t              i, j;
    size_t              scope_idx, item_idx;
    unsigned            table_idx;

    SS_ASSERT(sizeof(ss_pers_t)==size);

    for (i=0; i<nelmts; i++, pers++) {
        SS_EXTEND(serbuf, MAX(64, *serbuf_nused+nbytes), *serbuf_nalloc);
        if (SS_PERS_ISNULL(pers)) {
            serbuf[(*serbuf_nused)++] = 0xff; /*nothing more to encode*/
        } else {
            /* Table index */
            SS_ASSERT_CLASS(pers, ss_pers_t);
            table_idx = SS_MAGIC_SEQUENCE(SS_MAGIC_OF(pers));
            serbuf[(*serbuf_nused)++] = table_idx & 0xff;
            
            /* GFile serial number */
            if (NULL==(gfile=SS_GFILE_LINK(pers))) SS_ERROR(NOTFOUND);
            for (j=0; j<sizeof(gfile->serial); j++) {
                serbuf[(*serbuf_nused)++] = (gfile->serial >> (8*j)) & 0xff;
            }

            /* Scope index */
            scope_idx = ss_pers_link_scopeidx(pers);
            serbuf[(*serbuf_nused)++] = scope_idx & 0xff;
            serbuf[(*serbuf_nused)++] = (scope_idx>>8) & 0xff;
            serbuf[(*serbuf_nused)++] = (scope_idx>>16) & 0xff;
            serbuf[(*serbuf_nused)++] = (scope_idx>>24) & 0xff;

            /* Item index, direct */
            item_idx = ss_pers_link_objidx(pers);
            if (item_idx & SS_TABLE_INDIRECT) {
                ss_pers_t tmp = *pers;
                if (ss_pers_update(&tmp)<0) SS_ERROR(FAILED);
                item_idx = ss_pers_link_objidx(&tmp);
                SS_ASSERT(0==(item_idx & SS_TABLE_INDIRECT));
            }
            serbuf[(*serbuf_nused)++] = item_idx & 0xff;
            serbuf[(*serbuf_nused)++] = (item_idx>>8) & 0xff;
            serbuf[(*serbuf_nused)++] = (item_idx>>16) & 0xff;
            serbuf[(*serbuf_nused)++] = (item_idx>>24) & 0xff;
        }
    }

SS_CLEANUP:
    SS_LEAVE(serbuf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Persistent Objects
 * Purpose:     Decode persistent object links
 *
 * Description: Decodes the stuff encoded by ss_pers_encode_cb().
 *
 * Return:      Returns total number of bytes consumed from SERBUF on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, February  2, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_pers_decode_cb(void *buffer,                         /* Array of objects into which to decode SERBUF. */
                  const char *serbuf,                   /* Encoded information to be decoded. */
                  size_t size,                          /* Size of each element in BUFFER array. */
                  size_t nelmts                         /* Number of elements in BUFFER array. */
                  )
{
    SS_ENTER(ss_pers_decode_cb, size_t);
    size_t              i, serial, scope_idx, item_idx, gfile_idx;
    ss_pers_t           *pers = (ss_pers_t*)buffer;
    ss_gfile_t          *gfile=NULL; /*used only for sizeof*/
    unsigned            table_idx;
    const char          *serbuf_orig=serbuf;

    SS_ASSERT(sizeof(ss_pers_t)==size);
    for (i=0; i<nelmts; i++, pers++) {
        memset(pers, 0, sizeof(*pers));
        SS_H5_DECODE(serbuf, 1, table_idx);
        if (0xff!=table_idx) {
            SS_H5_DECODE(serbuf, sizeof(gfile->serial), serial);
            SS_H5_DECODE(serbuf, 4, scope_idx);
            SS_H5_DECODE(serbuf, 4, item_idx);


            if (SS_NOSIZE==(gfile_idx=ss_gfile_find_serial(serial))) SS_ERROR(NOTFOUND);
            if (NULL==ss_obj_new((ss_obj_t*)pers, SS_MAGIC_CONS(SS_MAGIC(ss_pers_t), table_idx), sizeof(*pers), pers))
                SS_ERROR(FAILED);
            ss_pers_link_setstate(pers, SS_PERS_LINK_CLOSED);
            ss_pers_link_setgfileidx(pers, gfile_idx);
            ss_pers_link_setscopeidx(pers, scope_idx);
            ss_pers_link_setobjidx(pers, item_idx);
        }
    }
    
SS_CLEANUP:
    memset(buffer, 0, size*nelmts); /*make them all null links on failure*/
    SS_LEAVE((size_t)(serbuf-serbuf_orig));
}

