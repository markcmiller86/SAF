/*
 * Copyright © 2004 
 *                  All rights reserved.
 *
 * Programmer:  Robb Matzke <matzke@inspiron.spizella.com>
 *              Friday, June 11, 2004
 */

/*
 * Copyright(C) 1999-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 1999-2005 Sandia Corporation.  
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
 * 
 * Active Developers:
 *     Peter K. Espen              SNL
 *     Eric A. Illescas            SNL
 *     Jake S. Jones               SNL
 *     Robb P. Matzke              LLNL
 *     Greg Sjaardema              SNL
 * 
 * Inactive Developers:
 *     William J. Arrighi          LLNL
 *     Ray T. Hitt                 SNL
 *     Mark C. Miller              LLNL
 *     Matthew O'Brien             LLNL
 *     James F. Reus               LLNL
 *     Larry A. Schoof             SNL
 * 
 * Acknowledgements:
 *     Marty L. Barnaby            SNL - Red parallel perf. study/tuning
 *     David M. Butler             LPS - Data model design/implementation Spec.
 *     Albert K. Cheng             NCSA - Parallel HDF5 support
 *     Nancy Collins               IBM - Alpha/Beta user
 *     Linnea M. Cook              LLNL - Management advocate
 *     Michael J. Folk             NCSA - Management advocate 
 *     Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
 *     Wilbur R. Johnson           SNL - Early developer
 *     Quincey Koziol              NCSA - Serial HDF5 Support 
 *     Celeste M. Matarazzo        LLNL - Management advocate
 *     Tyce T. McLarty             LLNL - parallel perf. study/tuning
 *     Tom H. Robey                SNL - Early developer
 *     Reinhard W. Stotzer         SNL - Early developer
 *     Judy Sturtevant             SNL - Red parallel perf. study/tuning 
 *     Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
 * 
 */
#include <safP.h>


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:	Alternative Index Specification
 *              
 * Description: The indexing specification of a collection is a characterization, more generally, of the name space used to
 *              identify members of the collection. For example, we might choose to refer to the members of a collection of 4
 *              quads, [Q,Q,Q,Q], using any of the following schemes:
 *
 *              * a: 0,1,2,3 
 *              * b: "Larry", "Mark", "Peter", "Ray" 
 *              * c: 27, 13, 102, 77 
 *              * d: 14, 36, 37, 92 
 *              * e: (0,0), (0,1), (1,0), (1,1) 
 *              * f: 0x00000000, 0x00000001, 0x00010000, 0x00010001
 *
 *              The /a/ scheme might be considered the "default" or "natural" naming scheme. /b/ is a naming scheme based upon
 *              strings. /c/ is a naming scheme based upon some arbitrary integer enumeration. Likewise for /d/. /e/ is a
 *              naming scheme based upon rectangular indexing. /f/ is a naming scheme that might be used in a pyramid of
 *              resolution of quads with 16 or fewer layers in which a 32 bit quantity is broken into two 16-bit pieces, one
 *              for the row and column of each layer in the pyramid. 
 *
 *              Some observations about these naming schemes. In some, /a/, /e/ and /f/ there is an easily specified rule for
 *              generating the names. In the others, the names must be explicitly enumerated. In some, /a/, /b/, /d/, /e/ and
 *              /f/ the names are sorted. In some, /a/, /e/ and /f/, the names are "compact" meaning that given the names of
 *              any two succesive members, there is no name that can be drawn from the same pool from which the other names
 *              come that falls between them.
 *
 *              From these observations, we conclude that an indexing spec can be either implicit or explicit. An implicit
 *              spec is one in which there is a simple rule for constructing each id in the name space. An explicit indexing
 *              spec is one in which each id in the name space must be explicitly specified. In addition, for an explicit spec,
 *              we also need to know if the names are sorted (and maybe even how to sort them by virtue of a call-back
 *              function to compare names), and if the names are compact.
 *
 *              SAF's notion of an /indexing/specification/ should be evolved to include these notions. Nonetheless, immediate
 *              support for user-defined IDs is essential. Therefore, we have provided functions in SAF for a client to
 *              specify /alternative/indexing/ specifications for a given collection. These functions will permit a SAF client
 *              to declare/describe and write/read alternative IDs. However, all relations involving the collection must still
 *              be specified in terms of the default indexing. Later, we can enhance the relations interface for SAF to
 *              support a client that specifies its relations in terms of these alternative IDs. 
 *
 *              *Implementation*Details*
 *
 *              These are details that are probably of no concern to the general user.  This info is for someone who cares
 *              about the lower levels of SAF and how Alternative Indexing was implemented.  The two SAF object data types
 *              SAF_IndexSpec and SAF_AltIndexSpec both map to the same SSlib object, namely ss_indexspec_t.  Every collection
 *              record has a variable length array of links to ss_indexspec_t objects. The first item in that array is the
 *              default index spec for that collection. If there are any alternate index specs for a collection, typically
 *              there would be only one, since these would be the one way that the client refers to their node ids (or elem
 *              ids or face ids, etc).
 *---------------------------------------------------------------------------------------------------------------------------------
 */


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Alternative Index Specification
 * Purpose:	Declare an Alternative Index Specification
 *
 * Description: 
 *              There is already a default SAF_IndexSpec associated with the collection defined by containing_set and cat.
 *              This call registers another, alternate index specification.  
 *              The default index spec associated with the collection is something that allows you to describe the
 *              collection IDs very easily by specifying the start index and how many you have (typically the start
 *              index is 0).  If you have some other, arbitrary way to identify the members of the collection,
 *              then you need to write out a problem sized array describing the names you give to the members of that
 *              collection. This is an explicit alternate indexing scheme, since you need to explicitly list
 *              the id's for each member of the collection.
 *              An implicit index spec is something that can be captured by stating the start index and how many you
 *              have, so you don't need to explicitly list the collection ids.
 *              
 *
 * Issue:       The data_type is just stored as the DSL_Type data_type member of the SAF_AltIndexSpec. This
 *              is transient, in memory data, it is not written to the saf database until the saf_write_alternate_indexspec
 *              call.  This means that if you do something like: saf_declare_alternate_indexspec, then
 *              saf_find_alternate_index_spec, then saf_describe_alternate_indexspec, (with no write call yet)
 *              you won't be able to recover the data_type.  
 *
 * Return:	On success, returns either the ASPEC argument or a newly allocated index specification. Returns the null
 *              pointer on failure.
 *
 * Programmer:
 *		Matthew O'Brien, LLNL, 2002-04-22, Initial implementation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_AltIndexSpec *
saf_declare_alternate_indexspec(SAF_ParMode pmode,              /*The parallel mode*/
                                SAF_Db *db,                     /* Database to contain the new index spec. */
                                SAF_Set *containing_set,        /* The containing set of the collection.*/
                                SAF_Cat *cat,                   /* The collection category. */
                                const char *name,               /* The name you wish to assign to this alt index spec*/
                                hid_t data_type,                /* The data type used to identify members of the collection*/
                                hbool_t is_explicit,            /* Whether the indexing specification is explicit or implicit*/
                                SAF_IndexSpec implicit_ispec,   /* The alternate indexing scheme of the collection. Ignored
                                                                 * for explicit specs. Pass SAF_NA_INDEXSPEC for explicit
                                                                 * alternative index specs. */
                                hbool_t is_compact,             /* Whether the indexing specification is compact or not.
                                                                 * Ignored for implicit specs. */
                                hbool_t is_sorted,              /* Whether the indexing specification is sorted or not.
                                                                 * Ignored for implicit specs. */
                                SAF_AltIndexSpec *aspec         /* OUT: The optional returned alternate index spec handle. If
                                                                 * the null pointer is passed for this argument then new
                                                                 * memory is allocated and returned, otherwise this argument
                                                                 * serves as the successful return value. */
                                )
{
    SAF_ENTER(saf_declare_alternate_indexspec, NULL);
    ss_collection_t             coll;
    ss_indexspec_t              defaultidx;
    ss_indexspecobj_t           *init=NULL;
    ss_scope_t                  scope=SS_SCOPE_NULL;    /* Scope in which to create new index spec. */
    size_t                      i;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);

    SAF_REQUIRE(SS_SET(containing_set),SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("CONTAINING_SET must be a valid set handle"));
    SAF_REQUIRE(SS_CAT(cat), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("CAT must be a valid cat handle"));

    /* The scope in which to declare the new indexspec */
    ss_file_topscope(db, &scope);

    if (SS_CAT(cat)) {
        /* Confirm the containing set has the category defined on it. */
        if (NULL==_saf_getCollection_set(containing_set, cat, &coll))
            SAF_ERROR(NULL, _saf_errmsg("set \"%s\" does not have a collection of category \"%s\"",
                                        ss_string_ptr(SS_SET_P(containing_set,name)),
                                        ss_string_ptr(SS_CAT_P(cat,name))));
        
        /* Obtain the default indexing scheme. */
        if (NULL==ss_array_get(SS_COLLECTION_P(&coll,indexing), ss_pers_tm, (size_t)0, (size_t)1, &defaultidx) ||
            !SS_INDEXSPEC(&defaultidx))
            SAF_ERROR(NULL, _saf_errmsg("unable to obtain default indexing specification for set \"%s\"",
                                        ss_string_ptr(SS_SET_P(containing_set,name))));
    }

    /* Fill in an index spec record */
    init = is_explicit ? SS_INDEXSPEC(&defaultidx) : NULL;
    aspec = (ss_indexspec_t*)ss_pers_new(&scope, SS_MAGIC(ss_indexspec_t), (ss_persobj_t*)init, SAF_ALL==pmode?SS_ALLSAME:0U,
                                         (ss_pers_t*)aspec, NULL);
    if (!aspec)
	SAF_ERROR(NULL, _saf_errmsg("unable to initialize index spec record"));
    if (SAF_EACH==pmode) SS_PERS_UNIQUE(aspec);
    if (!is_explicit){
        /* It is an implicit alt indexing scheme; use the IMPLICIT_ISPEC */
        SS_INDEXSPEC(aspec)->ndims = implicit_ispec.ndims;
        for (i=0; i<SAF_MAX_NDIMS; i++){
            SS_INDEXSPEC(aspec)->sizes[i] = implicit_ispec.sizes[i];
            SS_INDEXSPEC(aspec)->origins[i] = implicit_ispec.origins[i];
            SS_INDEXSPEC(aspec)->order[i]   = implicit_ispec.order[i];
        }
        SS_INDEXSPEC(aspec)->index_type  = SS_INDEXSPEC(&defaultidx)->index_type; /*implicit_ispec.index_type; does not exist*/
    }
    ss_string_set(SS_INDEXSPEC_P(aspec,name), name);
    SS_INDEXSPEC(aspec)->coll = coll;
    SS_INDEXSPEC(aspec)->is_explicit = is_explicit;
    SS_INDEXSPEC(aspec)->is_sorted = is_sorted;
    SS_INDEXSPEC(aspec)->is_compact = is_compact;

    /* This new index spec should be added to the end of the array of index specs. */
    i = ss_array_nelmts(SS_COLLECTION_P(&coll,indexing));
    ss_array_resize(SS_COLLECTION_P(&coll,indexing), i+1);
    ss_array_put(SS_COLLECTION_P(&coll,indexing), ss_pers_tm, i, (size_t)1, aspec);

    /* Save the datatype */
    SS_INDEXSPEC(aspec)->m.data_type = data_type;
    SAF_LEAVE(aspec);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Alternative Index Specification
 * Purpose:	Get a description of an alternate indexing spec
 *
 * Description: Get a description of an alternate indexing spec
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:
 * 		Matthew O'Brien, April 23, 2002.  Initial implementation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_alternate_indexspec(SAF_ParMode pmode,             /* The parallel mode*/
                                 SAF_AltIndexSpec *aspec,       /* The alternate index spec you want the description of. */
                                 SAF_Set *containing_set,       /* OUT: The containing set of the collection.  Pass NULL if you do
                                                                 * not want this returned. */
                                 SAF_Cat *cat,                  /* OUT: The collection category. Pass NULL if you do not want this
                                                                 * returned. */
                                 char **name,                   /* OUT: The name of this alt index spec. */
                                 hid_t *data_type,              /* OUT: The data type used to identify members of the collection.
                                                                 * Pass NULL if you do not want this returned. */
                                 hbool_t *is_explicit,          /* OUT: Whether the indexing specification is explicit or
                                                                 * implicit. */
                                 SAF_IndexSpec *implicit_ispec, /* OUT: The alternate indexing scheme of the collection. If the
                                                                 * index spec is explicit, then SAF_NA_INDEXSPEC will be
                                                                 * returned.  If the index spec is implicit, the implicit
                                                                 * index spec will be returned here. */
                                 hbool_t *is_compact,           /* OUT: Whether the indexing specification is compact or not.
                                                                 * Ignored for implicit specs. */
                                 hbool_t *is_sorted             /* OUT: Whether the indexing specification is sorted or not.
                                                                 * Ignored for implicit specs. */
                                 )
{
    SAF_ENTER(saf_describe_alternate_indexspec, SAF_PRECONDITION_ERROR);
    ss_collection_t     coll=SS_COLLECTION_NULL;
    size_t              i;
    hid_t               ftype;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_INDEXSPEC(aspec), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("the ASPEC argument must be a valid handle"));


    /* Get the collection associated with the alt indexing spec. */
    coll = SS_INDEXSPEC(aspec)->coll;

    /* Now the we have the collection record, get the containing set. */
    if (containing_set)
        *containing_set = SS_COLLECTION(&coll)->containing_set;

    /* Also get the cat. */
    if (cat)
        *cat = SS_COLLECTION(&coll)->cat;

    /* Populate the data_type */
    if (data_type) {
        if (!SS_PERS_ISNULL(SS_INDEXSPEC_P(aspec,blob))) {
            ss_blob_bound_f1(SS_INDEXSPEC_P(aspec,blob), NULL, NULL, NULL, &ftype);
            *data_type = H5Tget_native_type(ftype, H5T_DIR_DEFAULT);
            H5Tclose(ftype);
        } else {
            *data_type = H5I_INVALID_HID;
        }
    }

    if (is_explicit)
        *is_explicit = SS_INDEXSPEC(aspec)->is_explicit;
    if (is_sorted)
        *is_sorted = SS_INDEXSPEC(aspec)->is_sorted;
    if (is_compact)
        *is_compact = SS_INDEXSPEC(aspec)->is_compact;
 
    if (implicit_ispec) { 
        if (SS_INDEXSPEC(aspec)->is_explicit) {
            *implicit_ispec = SAF_NA_INDEXSPEC;
        } else {
            /* It is implicit, so return the regular implicit_ispec. */
            implicit_ispec->ndims = SS_INDEXSPEC(aspec)->ndims;
            for (i=0; i<SAF_MAX_NDIMS; i++) {
                implicit_ispec->sizes[i]   = SS_INDEXSPEC(aspec)->sizes[i];
                implicit_ispec->origins[i] = SS_INDEXSPEC(aspec)->origins[i];
                implicit_ispec->order[i]   = SS_INDEXSPEC(aspec)->order[i];
            }
        }
    }

    /* return the name of the alt index spec */
    if (_saf_setupReturned_string(name, ss_string_ptr(SS_INDEXSPEC_P(aspec,name))) != SAF_SUCCESS)
        SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to return alt index spec name for alt index spec %s\n",
                                                ss_string_ptr(SS_INDEXSPEC_P(aspec,name))));

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Alternative Index Specification
 * Purpose:     Find alternate index specs by matching criteria
 *
 * Description: Find  alternate index specs by matching criteria.
 *
 *		If the NAME_GREP argument begins with a leading "at sign" character, '@', the remaining characters will be
 *		treated as a limited form of a regular expression akin to that supported in 'ed'. Otherwise, it will
 *		be treated as a specific name for a set. If the name does not matter, pass SAF_ANY_NAME.
 *
 *		If the library was not compiled with -lgen support library, then if regular expressions are used,
 *		the library will behave as though SAF_ANY_NAME was specified.
 *
 * Issue:       This function does not follow the usual semantics of a /find/ operation. Instead of searching through a
 *              database (or scope) and looking for index specifications that match a certain pattern, it instead looks at a
 *              collection (specified with the CONTAINING_SET and CAT arguments) and returns any index specifications of that
 *              collection that have the requested name or name pattern.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:
 *		Matthew O'Brien, April 24, 2002, Initial implementation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_alternate_indexspecs(SAF_ParMode pmode,        /* The parallel mode */
                              SAF_Set *containing_set,  /* The containing set of the collection.*/
                              SAF_Cat *cat,             /* The collection category. */
                              const char *name_grep,    /* The name of the alt index spec you wish to search for. Pass NULL if
                                                         * you do not wish to limit the search via a name. */
                              int *num,                 /* For this and the succeeding argument [see Returned Handles]. */
                              SAF_AltIndexSpec **found  /* For this and the preceding argument [see Returned Handles]. */
                              )
{
    SAF_ENTER(saf_find_alternate_indexspecs, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_AltIndexSpec, key, mask);
    ss_collection_t     coll=SS_COLLECTION_NULL;
    ss_indexspec_t      *ispec=NULL;
    size_t              nispecs=0, i;
    
    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));

    if (!_saf_is_self_decomp(cat)) {
        /* Get the array of index specifications for the collection. Skip the first one, which is the default index spec. */
        if (NULL==_saf_getCollection_set(containing_set, cat, &coll))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to get collection"));
        nispecs = ss_array_nelmts(SS_COLLECTION_P(&coll,indexing));
        nispecs -= 1;
        ispec = ss_array_get(SS_COLLECTION_P(&coll,indexing), ss_pers_tm, (size_t)1, nispecs, NULL);

        /* If a name is supplied then prune the return value according to that name (or regular expression). Prune in such a
         * way as to keep the index specs in their definition order. */    
        if (name_grep) {
            if ('@'==*name_grep) {
                SAF_SEARCH_RE(SAF_AltIndexSpec, key, mask, name, name_grep+1);
            } else {
                SAF_SEARCH_S(SAF_AltIndexSpec, key, mask, name, name_grep);
            }
            i=0;
            while (i<nispecs) {
                if (0==ss_pers_cmp((ss_pers_t*)ispec+i, (ss_pers_t*)key, (ss_persobj_t*)&mask)) {
                    i++;
                } else {
                    --nispecs;
                    memmove(ispec+i, ispec+i+1, (nispecs-i)*sizeof(*ispec));
                }
            }
        }
    }

    /* Return only what the caller asked for... */
    if (!found) {
        /* Count the matches */
        assert(num);
        *num = nispecs;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        *found = ispec;
        ispec = NULL;
        if (num) *num = nispecs;
    } else {
        /* Find limited matches; client allocates result buffer */
        assert(num);
        if (nispecs>(size_t)*num)
            SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matching objects"));
        *found = ispec;
        ispec = NULL;
        *num = nispecs;
    }

    SS_FREE(ispec);
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Alternative Index Specification
 * Purpose:     Write an alternate index specs to disk
 *
 * Description: Write an alternate index specs to disk, involves actual I/O
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:
 *		Matthew O'Brien, April 24, 2002, Initial implementation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_write_alternate_indexspec(SAF_ParMode pmode,        /* The parallel mode. */
                              SAF_AltIndexSpec *aspec,  /* The alternate index spec to write. */
                              hid_t data_type,          /* The datatype used to identify members of the collection, if not
                                                         * already supplied with saf_declare_alternate_indexspec(). */
                              void *buf,                /* The buffer of data to write. */
                              SAF_Db *file              /* The optional destination file to which to write the data. If this
                                                         * is a null pointer then the data is written to the same file as
                                                         * ASPEC. */
                              )
{
    SAF_ENTER(saf_write_alternate_indexspec, SAF_PRECONDITION_ERROR);
    double              timer_start=0;
    hsize_t             offset;
    ss_collection_t     coll=SS_COLLECTION_NULL;
    ss_scope_t          scope=SS_SCOPE_NULL;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);


    SAF_REQUIRE(SS_INDEXSPEC(aspec), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ASPEC must be a valid alternate index spec handle"));
    SAF_REQUIRE(buf, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("BUF must not be null"));
    SAF_REQUIRE(SS_INDEXSPEC(aspec)->m.data_type>0 || data_type>0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("You must pass a datatype either in the call to saf_declare_alternate_indexspec() or here"));

    /* Start the timer */
    if (_SAF_GLOBALS.p.TraceTimes)
        timer_start = _saf_wall_clock(false);
   
    /* Get the collection record */
    ss_pers_scope(file?(ss_pers_t*)file:(ss_pers_t*)aspec, &scope);
    if (data_type<=0) data_type = SS_INDEXSPEC(aspec)->m.data_type;
    coll = SS_INDEXSPEC(aspec)->coll;

    /* Disallow overwrites to alt index specs */
    if (!SS_PERS_ISNULL(SS_INDEXSPEC_P(aspec,blob)))
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot overwrite an alt index spec"));

    /* Write the BUF data */
    SAF_DIRTY(aspec, pmode);
    if (NULL==ss_blob_new(&scope, SAF_ALL==pmode?SS_ALLSAME:0U, SS_INDEXSPEC_P(aspec,blob)) ||
        ss_blob_bind_m1(SS_INDEXSPEC_P(aspec,blob), buf, data_type, (hsize_t)SS_COLLECTION(&coll)->count)<0 ||
        ss_blob_mkstorage(SS_INDEXSPEC_P(aspec,blob), &offset, SAF_ALL==pmode?SS_ALLSAME:0U, NULL)<0)
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot allocate file space for the data"));
    if (ss_blob_write1(SS_INDEXSPEC_P(aspec,blob), offset, (hsize_t)SS_COLLECTION(&coll)->count,
                       SS_BLOB_COLLECTIVE|SS_BLOB_UNBIND|(SAF_ALL==pmode?SS_ALLSAME:0U), NULL)<0)
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot write data"));

    /* Accumulate times */
    if (_SAF_GLOBALS.p.TraceTimes)
        _SAF_GLOBALS.CummWriteTime += (_saf_wall_clock(false) - timer_start);

    SAF_LEAVE(SAF_SUCCESS);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Alternative Index Specification
 * Purpose:     Read an alternate index specs from disk
 *
 * Description: Read an alternate index specs from disk, involves actual I/O
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issue:       There is no way for the caller to find out what datatype is being returned.
 *
 * Programmer:
 *		Matthew O'Brien, April 24, 2002, Initial implementation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_read_alternate_indexspec(SAF_ParMode pmode,         /* The parallel mode. */
                             SAF_AltIndexSpec *aspec,   /* The alternate index spec handle to read. */
                             void **buf                 /* The buffer to be filled in with the data. */
                             )
{
    SAF_ENTER(saf_read_alternate_indexspec, SAF_PRECONDITION_ERROR);
    double              timer_start=0;
    ss_collection_t     coll=SS_COLLECTION_NULL;
   
    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_INDEXSPEC(aspec), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ASPEC must be a valid alt index spec handle"));
    SAF_REQUIRE(buf, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("BUF cannot be null"));

    /* Start the timer. */
    if (_SAF_GLOBALS.p.TraceTimes)
        timer_start = _saf_wall_clock(false);

    /* Obtain the collection record. */
    coll = SS_INDEXSPEC(aspec)->coll;
    if (!SS_COLLECTION(&coll))
	SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to obtain collection record"));

    /* Read in the buf data. */
    if (SS_PERS_ISNULL(SS_INDEXSPEC_P(aspec,blob)))
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("attempt to read non-existent blob; there is no data for the alternate index spec"));
    if (NULL==(*buf=ss_blob_read1(SS_INDEXSPEC_P(aspec,blob), (hsize_t)0, (hsize_t)SS_COLLECTION(&coll)->count, 0U, NULL)))
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to read buf alt index spec data"));

    /* Accumulate times */
    if (_SAF_GLOBALS.p.TraceTimes)
        _SAF_GLOBALS.CummReadTime += (_saf_wall_clock(false) - timer_start);

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Alternative Index Specification
 * Purpose:     Return the not applicable index spec
 *
 * Description: Return the not applicable index spec, this is used with explicit indexing specs
 *
 * Return:	The not applicable index spec
 *
 * Programmer:
 *		Matthew O'Brien, April 24, 2002, Initial implementation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_IndexSpec
_saf_indexspec_not_applicable(void)
{
    SAF_ENTER(_saf_indexspec_not_applicable, _SAF_GLOBALS.indexspec_not_applicable);
    SAF_LEAVE(_SAF_GLOBALS.indexspec_not_applicable);
}



