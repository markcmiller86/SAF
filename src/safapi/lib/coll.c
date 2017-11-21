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
 * Chapter:     Collections
 * Description:
 *
 * In theory, all nodes, edges, faces, elements, blocks, materials, processor pieces etc. are just sets. See saf_declare_set()
 * for more of a description of sets. In practice, we have a need to distinguish between two kinds of sets: /primitive/ ones
 * such as the elements of a mesh, and /aggregate/ ones, such as material or block sets. We call these two classes of sets,
 * /Cells/ and /Sets/ respectively. Cells are primitive sets such as the nodes or elements of a mesh. What makes them primitive?
 * They are not decomposed into any other sets, whose union can form them. In lattice theory terms, cells are
 * /Join/Irreducible/Members/ (or /JIMS/) of the subset inclusion lattice.
 *
 * Sets are aggregate sets such as a processor piece or a block. In lattice theory terms, sets are /Join/Reducible/Members/ (or
 * /JRMS/ pronounced "germs"). The key point here is that cells are *never* instantiated as first class sets in this API
 * (e.g. using the saf_declare_set() call). Instead cells only ever exist as members of collections.
 *
 * On the other hand, collections themselves may be composed of either cells or sets. When a collection is declared, the
 * client either specifies a cell-type for the members, implying the collection is composed of cells, or not, implying the
 * collection is composed of sets.
 *
 * Since collections are defined by their containing set and a collection category, this pair serves to define a collection
 * and there is no specific SAF_Xxxx handle explicitly for collections.
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collections
 * Purpose:     Compare two collections
 *
 * Description: Compare two given collections for equality.  Note that each collection is specified as a set-category pair.
 *              These pairs are equal if they have "the same" sets and "the same" categories.
 *
 * Parallel:    Independent
 *---------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
saf_same_collections(SAF_Set *Sa,       /* The set component of the first or left operand of the equality comparison operator. */
                     SAF_Cat *Ca,       /* The category component of the first or left operand of the equality comparison
                                         * operator. */
                     SAF_Set *Sb,       /* The set component of the second or right operand of the equality comparison operator. */
                     SAF_Cat *Cb        /* The category component of the second or right operand of the equality comparison
                                         * operator. */
                     )
{
  SAF_ENTER(saf_same_collections,false);
  hbool_t retval = SAF_EQUIV(Sa, Sb) && SAF_EQUIV(Ca, Cb);
  SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collections
 * Purpose:     Declare a collection
 *
 * Description: Collections are contained in sets. Thus there is no explicit object handle for a collection. Instead, a
 *              collection is referenced by a pair: the containing set handle and the category handle.
 *
 *              If the set is extendible, then any collection declared on it is considered extendible.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issues:	As currently implemented, the COUNT and ISPEC args are redundant. However, in general, the indexing schema
 *		used to identify members of the collection is nearly totally independent of the count. A common example SAF 
 *		*does*not*yet* support is the case in which the indexing ids for the members of the collection is some other
 *		arbitrary list of ints (or character string names, etc). For example, all of the nodes on the top set is *not*
 *		necessarily indexed 0...num_nodes-1. Under these conditions, the indexing scheme is another, problem sized
 *		array of ints. However, to handle this, we probably need a saf_write_collection_indexing() function to actually
 *              write that data to a file. Writing it in this call would violate our current policy where problem-sized disk
 *              I/O occurs only on calls with "write" or "read" in their names.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_declare_collection(SAF_ParMode pmode,       /* The parallel mode. */
                       SAF_Set *containing_set, /* The containing set of the collection. In SAF_ONE() parallel mode, all
                                                 * processes except the process identified by the rank argument of the SAF_ONE()
                                                 * macro are free to pass SAF_NULL_SET with the set's database handle. */
                       SAF_Cat *cat,            /* The collection category. */
                       SAF_CellType ctype,      /* The cell type of the members of the collection.  If this is a non-primitive
                                                 * collection, pass SAF_CELLTYPE_SET. If this is a primitive collection of
                                                 * mixed cell type, pass SAF_CELLTYPE_MIXED. If this is a primitive collection
                                                 * of arbitrarily connected cells, pass SAF_CELLTYPE_ARB. Otherwise, it must
                                                 * be a primitive collection of homogeneous type and the caller should pass
                                                 * one of the cell types specified by SAF_CellType. */
                       int count,               /* The number of members of the collection. If the containing set is an
                                                 * extendible set, the count can be changed by a call to
                                                 * saf_extend_collection(). */
                       SAF_IndexSpec ispec,     /* The indexing scheme of the collection (e.g., how are members of the collection
                                                 * identified within the collection). We have predefined some macros for common
                                                 * cases: SAF_1DC(), SAF_2DC(), and SAF_3DC() for C-ordered and indexed arrays and
                                                 *        likewise for Fortran-ordered and indexed arrays (replace the "C" with
                                                 * an "F" in the macro name). */
                       SAF_DecompMode is_decomp /* Indicates if the specified collection is a decomposition of its containing set.
                                                 * That is, if we take the union of all the members of the collection do we form
                                                 * a set that is equal to the containing set? */
                       )
{
    
    SAF_ENTER(saf_declare_collection, SAF_PRECONDITION_ERROR);
    ss_scope_t          scope=SS_SCOPE_NULL;            /* Scope to own the new collection */
    ss_indexspec_t      idx=SS_INDEXSPEC_NULL;          /* The default index spec */
    ss_collection_t     coll=SS_COLLECTION_NULL;        /* The new collection */
    int                 i, n;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(containing_set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("CONTAINING_SET must be a valid set handle for participating processes"));
    SAF_REQUIRE(SS_CAT(cat), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("CAT must be a valid category handle for participating processes"));
    SAF_REQUIRE(is_decomp != SAF_DECOMP_TORF, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("IS_DECOMP must be either SAF_DECOMP_TRUE or SAF_DECOMP_FALSE for participating processes"));
    SAF_REQUIRE(_saf_valid_indexspec(ispec, count), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ISPEC rank and sizes must be valid for participating processes"));

    /* The scope that owns CONTAINING_SET will also be the owner of the new collection. */
    if (NULL==ss_pers_scope((ss_pers_t*)containing_set, &scope))
        SAF_RETURN(SAF_SSLIB_ERROR);
    
    /* Initialize an index record */
    if (NULL==ss_pers_new(&scope, SS_MAGIC(ss_indexspec_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)&idx, NULL))
        SAF_RETURN(SAF_SSLIB_ERROR);
    if (SAF_EACH==pmode) SS_PERS_UNIQUE(&idx);
    SS_INDEXSPEC(&idx)->ndims = ispec.ndims;
    for (i=0, n=1; i<ispec.ndims; i++) {
        n *= ispec.sizes[i];
        SS_INDEXSPEC(&idx)->sizes[i] = ispec.sizes[i];
        SS_INDEXSPEC(&idx)->origins[i] = ispec.origins[i];
        SS_INDEXSPEC(&idx)->order[i] = ispec.order[i];
    }

    /* Initialize the collection record */
    if (NULL==ss_pers_new(&scope, SS_MAGIC(ss_collection_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)&coll, NULL))
        SAF_RETURN(SAF_SSLIB_ERROR);

    /* Fill in the record */
    if (SAF_EACH==pmode) SS_PERS_UNIQUE(&coll);
    SS_COLLECTION(&coll)->containing_set = *containing_set;
    SS_COLLECTION(&coll)->cat = *cat;
    SS_COLLECTION(&coll)->cell_type = ctype;
    SS_COLLECTION(&coll)->count = n;
    ss_array_resize(SS_COLLECTION_P(&coll,indexing), 1);
    ss_array_put(SS_COLLECTION_P(&coll,indexing), ss_pers_tm, (size_t)0, (size_t)1, &idx);
    SS_COLLECTION(&coll)->is_decomp = is_decomp?TRUE:FALSE;

    /* If this is a non-primitive collection then extend the variable length array of Set links. We extend the array here
     * because we don't actually store the COUNT value anywhere in the collection. The initial values of the array are all
     * null Set pointers. */
    if (SAF_CELLTYPE_SET==ctype) {
        if (ss_array_resize(SS_COLLECTION_P(&coll,members), (size_t)count)<0)
            SAF_RETURN(SAF_SSLIB_ERROR);
    }
      
    /* update the set record */
    _saf_putCollection_set(pmode, containing_set, cat, &coll);
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collections
 * Purpose:     Find collections
 *
 * Description:	This function is used to search for collections on a given set. In addition, the client
 *		can limit the search to collections of a given role, with a given cell-type or those which are or are not a
 *		decomposition of the containing set.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issue:	The documentation for this function originally said it would return all collections in the whole database
 *		if the containing_set arg passed was SAF_UNIVERSE(db). However, it is clear from the implementation that
 *		it cannot do that.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_collections(SAF_ParMode pmode,         /* The parallel mode. */
                     SAF_Set *containing_set,   /* The containing set in which to search for collections. In SAF_ONE()
                                                 * parallel mode, all processes except the process identified by the
                                                 * rank argument of the SAF_ONE() macro are free to pass SAF_NULL_SET
                                                 * with the set's database handle. */
                     SAF_Role *role,            /* The role of the collection. Pass NULL if you do not wish to
                                                 * limit the search by this parameter. */
                     SAF_CellType cell_type,    /* The cell-type of the members of the collection.  Pass SAF_ANY_CELLTYPE if you
                                                 * do not wish to limit the search by this parameter. */
                     int topo_dim,              /* The topological dimension of the collection. Pass SAF_ANY_TOPODIM if you do not
                                                 * wish to limit the search by this parameter. */
                     SAF_DecompMode decomp_mode,/* Whether the found collections must be a decomposition of the containing set.
                                                 * Pass SAF_DECOMP_TORF if it does not matter. */
                     int *num,           	/* For this and the succeeding argument, (see Returned Handles). */ 
                     SAF_Cat **found            /* For this and the preceding argument, (see Returned Handles). */ 
                     )
{
    SAF_ENTER(saf_find_collections, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Cat, cat_key, cat_mask);
    SAF_KEYMASK(SAF_Collection, coll_key, coll_mask);
    ss_catobj_t         *cat_mask_p=NULL, cat_mask_z;
    ss_collectionobj_t  *coll_mask_p=NULL, coll_mask_z;
    size_t              ncolls;                 /* Number of collections in CONTAINING_SET */
    size_t              nfound=0;               /* Number of matches found */
    size_t              limit=SS_NOSIZE;        /* Limit number of matches returned */
    size_t              i;
    ss_collection_t     coll;
    ss_cat_t            cat;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(containing_set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("CONTAINING_SET must be a valid set handle for participating processes"));
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM_COLLS and CATS must be compatible for return value allocation"));
    SAF_REQUIRE(!role || SS_ROLE(role), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ROLE must be a valid role handle if supplied"));

    /* Fill in the collection and category keys with desired information */
    if (cell_type != SAF_CELLTYPE_ANY)
        SAF_SEARCH(SAF_Collection, coll_key, coll_mask, cell_type, cell_type);
    if (topo_dim != SAF_ANY_TOPODIM)
        SAF_SEARCH(SAF_Cat, cat_key, cat_mask, tdim, topo_dim);
    if (decomp_mode != SAF_DECOMP_TORF)
        SAF_SEARCH(SAF_Collection, coll_key, coll_mask, is_decomp, decomp_mode?TRUE:FALSE);

    /* Allocate return value. */
    ncolls = ss_array_nelmts(SS_SET_P(containing_set,colls));
    if (!found) {
        /* Only counting */
        assert(num);
        *num=0;
    } else if (!*found) {
        /* Library allocates results */
        *found = calloc(ncolls, sizeof **found);
        if (num) *num = 0;
    } else {
        /* Client allocated results */
        assert(num);
        assert(*num>=0);
        limit = *num;
        *num = 0;
    }

    /* If the collection or catetory masks are empty then we should pass null for the mask arguments in order to match
     * everything and avoid an error regarding an empty mask. */
    memset(&coll_mask_z, 0, sizeof coll_mask_z);
    memset(&cat_mask_z, 0, sizeof cat_mask_z);
    coll_mask_p = memcmp(&coll_mask, &coll_mask_z, sizeof coll_mask) ? &coll_mask : NULL;
    cat_mask_p = memcmp(&cat_mask, &cat_mask_z, sizeof cat_mask) ? &cat_mask : NULL;
    
    /* Scan through the collections associated with the containing_set */
    for (i=0; i<ncolls; i++) {
        ss_array_get(SS_SET_P(containing_set,colls), 0, i, 1, &coll);
        cat = SS_COLLECTION(&coll)->cat;

        /* If a role is specified then determine whether it is the same role as what is associated with the cat for this
         * collection for this set. Since  collections of the self category can be explicitly stored in the set's collection
         * list then we have to be prepared for the case when cat->role is a null object link (because the "self" collection
         * category isn't allowed to point to a particular role. */
        if (role) {
            if (SS_PERS_ISNULL(SS_CAT_P(&cat,role))) continue;
            if (!SS_PERS_EQUAL(role, SS_CAT_P(&cat,role))) continue;
        }
        
        /* Compare the collection and category with the keys we initialized above. */
        if (coll_mask_p && 0!=ss_pers_cmp((ss_pers_t*)&coll, (ss_pers_t*)coll_key, (ss_persobj_t*)coll_mask_p)) continue;
        if (cat_mask_p && 0!=ss_pers_cmp((ss_pers_t*)&cat, (ss_pers_t*)cat_key, (ss_persobj_t*)cat_mask_p)) continue;

        /* Found a match. */
        if (nfound>=limit)
            SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matching objects"));
        if (found)
            (*found)[nfound] = cat;
        nfound++;
    }

    if (num) *num = nfound;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collections
 * Purpose:     Describe a collection
 *
 * Description: Returns information about a collection.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_collection(SAF_ParMode  pmode,             /* The parallel mode. */
                        SAF_Set *containing_set,        /* The containing set of the desired collection. In SAF_ONE() parallel
                                                         * mode, all processes except the process identified by the rank
                                                         * argument of the SAF_ONE() macro are free to pass SAF_NULL with the
                                                         * set's database handle. */
                        SAF_Cat *cat,                   /* The collection category of the desired collection. */
                        SAF_CellType *t,                /* OUT: The cell-type of the members of the collection. Pass NULL if
                                                         * this return value is not desired. */
                        int *count,                     /* OUT: The returned count of the collection.  Pass NULL if this
                                                         * return value is not desired. */
                        SAF_IndexSpec *ispec,		/* OUT: The returned indexing specification for the collection. Pass
                                                         * NULL if this return value is not desired. */
                        SAF_DecompMode *is_decomp,      /* OUT: Whether the collection is a decomposition of the containing
                                                         * set. Pass NULL if this return value is not desired. */
                        SAF_Set **member_sets           /* If the collection is non-primitive, this argument is used to return
                                                         * the specific set handles for the sets that are in the collection.
                                                         * Pass NULL if this return value is not desired. Otherwise, if
                                                         * MEMBER_SETS points to NULL, the library will allocate space for the
                                                         * returned set handles. Otherwise the caller allocates the space and
                                                         * the input value of COUNT indicates the size of the space in 
							 * number of set handles. */
                        )
{
    SAF_ENTER(saf_describe_collection, SAF_PRECONDITION_ERROR);
    SAF_Collection      coll = SS_COLLECTION_NULL;
    ss_indexspec_t      idx  = SS_INDEXSPEC_NULL;
    int                 i;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(containing_set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("the CONTAINING_SET must be valid for all participating processes"));
    SAF_REQUIRE(SS_CAT(cat), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("CAT must be a valid category handle for participating processes"));

    /* Issue: having both arguments NULL will cause _saf_valid_memhints to return false, but in this particular case, having both
    *        arguments NULL is ok, so we dont call _saf_valid_memhints if both are 0. */
    SAF_REQUIRE((!count && !member_sets) || _saf_valid_memhints(count, (void**)member_sets),
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM_SETS and MEMBER_SETS must be compatible for return value allocation"));

    /* Special case for the "self" collection, which isn't always instantiated as a first-class collection. */
    if (_saf_is_self_decomp(cat)) {
        if (t) *t = SAF_CELLTYPE_SET;
        if (is_decomp) *is_decomp = SAF_DECOMP_TRUE;
        if (ispec) {
            memset(ispec, 0, sizeof *ispec);
            ispec->ndims = 1;
            ispec->sizes[0] = 1;
        }
        if (member_sets) {
            if (!*member_sets) *member_sets = malloc(sizeof(**member_sets));
            (*member_sets)[0] = *containing_set;
        }
        if (count) *count = 1;
        goto done;
    }
    
    /* Obtain the collection given the containing set and collection category. If a collection is not defined on the
     * containing set for the specified category then fill in return values appropriately. */
    if (NULL==_saf_getCollection_set(containing_set, cat, &coll)) {
        if (t) *t = SAF_CELLTYPE_SET;
        if (is_decomp) *is_decomp = SAF_DECOMP_FALSE;
        if (ispec) memset(ispec, 0, sizeof *ispec);
        if (count) *count = 0;
        goto done;
    }

    /* The default index spec */
    ss_array_get(SS_COLLECTION_P(&coll,indexing), ss_pers_tm, (size_t)0, (size_t)1, &idx);

    SAF_ASSERT(SS_COLLECTION(&coll)->cell_type==SAF_CELLTYPE_SET || !member_sets, SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
               _saf_errmsg("MEMBER_SETS must be NULL for a primitive collection for participating processes"));

    /* fill in return info as requested by the client */
    if (t) *t = SS_COLLECTION(&coll)->cell_type;
    if (is_decomp) *is_decomp = SS_COLLECTION(&coll)->is_decomp ? SAF_DECOMP_TRUE : SAF_DECOMP_FALSE;
    if (ispec) {
        memset(ispec, 0, sizeof *ispec);
        if (!SS_PERS_ISNULL(&idx)) {
            ispec->ndims = SS_INDEXSPEC(&idx)->ndims;
            for (i=0; i<SS_INDEXSPEC(&idx)->ndims; i++) {
                ispec->sizes[i] = SS_INDEXSPEC(&idx)->sizes[i];
                ispec->origins[i] = SS_INDEXSPEC(&idx)->origins[i];
                ispec->order[i] = SS_INDEXSPEC(&idx)->order[i];
            }
        }
    }

    /* fill in the member sets, if requested */
    if (member_sets) {
        if (ss_array_nelmts(SS_COLLECTION_P(&coll,members))<(size_t)(SS_COLLECTION(&coll)->count))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("attempt to read member list before it's been completely filled"));
        if (!*member_sets) {
            /* Library allocates results and returns all member sets */
            *member_sets = malloc(SS_COLLECTION(&coll)->count * sizeof(**member_sets));
        } else {
            /* Client allocated result buffer and COUNT incoming value is the array size */
            SAF_ASSERT(count && *count>=SS_COLLECTION(&coll)->count, SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
                       _saf_errmsg("client allocated mem is too small %i", count));
        }
        ss_array_get(SS_COLLECTION_P(&coll,members), 0, 0, SS_NOSIZE, *member_sets);
    }

    /* finally, the count */
    if (count) *count = SS_COLLECTION(&coll)->count;
    
done:
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collections
 * Purpose:     Add members to a collection
 *
 * Description: This function allows the client to add members to an existing collection. While you can extend a collection, you
 *              cannot change the number of dimensions in the indexing scheme. You can only change the size in each dimension
 *              and then you can only increase it. That is, if the collection was indexed using 2 dimensional indexing, it
 *              cannot be changed to 3 dimensional indexing.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_extend_collection(SAF_ParMode pmode,        /* The parallel mode. */
                      SAF_Set *containing_set,	/* The containing set of the collection. */
                      SAF_Cat *cat,		/* The collection category of the collection. */
                      int add_count,		/* The number of members to add to the collection. */
                      SAF_IndexSpec add_ispec	/* The new indexing scheme. */
                      )
{
    SAF_ENTER(saf_extend_collection, SAF_PRECONDITION_ERROR);
    ss_collection_t     coll=SS_COLLECTION_NULL;
    ss_indexspec_t      idx=SS_INDEXSPEC_NULL;
    int                 i;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(containing_set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("CONTAINING_SET must be a valid set handle for participating processes"));
    SAF_REQUIRE(SS_CAT(cat), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("CAT must be a valid category handle for participating processes"));
    SAF_REQUIRE(_saf_valid_add_indexspec(pmode, containing_set, cat, add_ispec, add_count),
                SAF_MED_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ADD_ISPEC sizes must be valid, total ADD_COUNT and be compatible with the existing indexing"
                            " for participating processes"));

    /* If this isn't the self collection, confirm this collection is, indeed, defined on an extendible set */
    if (!_saf_is_self_decomp(cat) && !SS_SET(containing_set)->is_extendible)
        SAF_RETURN(SAF_ASSERTION_ERROR);


    /* Get collection and update relevant portions */
    _saf_getCollection_set(containing_set, cat, &coll);
    SAF_DIRTY(&coll, pmode);
    SS_COLLECTION(&coll)->count += add_count;

    /* Get the default indexing spec and update relevant portions */
    ss_array_get(SS_COLLECTION_P(&coll,indexing), ss_pers_tm, (size_t)0, (size_t)1, &idx);
    SAF_DIRTY(&idx, pmode);
    for (i=0; i<add_ispec.ndims; i++)
        SS_INDEXSPEC(&idx)->sizes[i] += add_ispec.sizes[i];

    /* If this is a non-primitive collection, update the member list */
    if (SS_COLLECTION(&coll)->cell_type == SAF_CELLTYPE_SET)
        ss_array_resize(SS_COLLECTION_P(&coll,members), (size_t)(SS_COLLECTION(&coll)->count));

    SAF_LEAVE(SAF_SUCCESS);
}
