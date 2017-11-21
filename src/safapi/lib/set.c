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
 * Audience:	Public
 * Chapter:	Sets
 * Description: Sets in SAF represent infinite point sets. As discussed in the chapter on collections (see Collections),
 *		in theory all nodes, edges, faces, volumes, etc. are sets.
 *
 *		However, in SAF, set objects (e.g. something created with a call to saf_declare_set()) are instantiated
 *		only to represent infinite point sets that are decomposed into other, more primitive entities. Examples
 *		are materials, processor pieces, domains, parts in an assembly, blocks, nodesets, etc.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Sets
 * Purpose:     Declare a set
 *
 * Description: Every set has a /maximum/topological/dimension/ indicating how the infinity of points that are the set
 *		are organized. Are they organized along some curve (1D), surface (2D), volume (3D), etc.?  More formally,
 *		the /maximum/ /topological/ /dimension/ of a set indicates the maximum rank of local coordinate systems over
 *		all neighborhoods of the infinite point set.
 *
 *		Note that a /maximum/topological/dimension/ of 0 does *not* mean that the set contains a single point or 
 *		no points. It means that the set contains only a finite number of points. That is the set is *not* an infinite
 *		point set but a finite one.
 *
 * Return:      Returns a pointer to a set link on success; null on failure. The SET argument is the successful return value,
 *              or if SET is null, a new set link is allocated for the return.
 *
 * Issue:	Eventually roles specific to the creation of algebraic types and cell types will be added.
 *
 *		I think we can eliminate the ROLE argument here and instead deduce it from the SAF_Quantity associated
 *		with the default coordinates for the set. For example, if the default coordinates represent a length quantity,
 *		then the ROLE must be SAF_SPACE. If they represent a time quantity, then the ROLE must be SAF_TIME.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Set *
saf_declare_set(SAF_ParMode pmode,	/* The parallel mode. */
		SAF_Db *db,             /* The database handle in which to create the set. */
		const char *name,	/* The name of the set being declared. */
		int max_topo_dim,	/* The topological dimension of the set. If the set will contain sets of different
                                         * topological dimensions then this must be the maximum topological dimension of any
                                         * set in the subset inclusion lattice rooted below SET. */
		SAF_SilRole role,	/* The role of the set. Possible values are SAF_SPACE for a spatial set, SAF_TIME for
                                         * a time-base set, SAF_PARAM for a parameter space set, or SAF_USERD for a user-defined
					 * role. */
		SAF_ExtendMode extmode, /* Indicates whether or not the base-space represented by the set is extendible. Possible
					 * values are SAF_EXTENDIBLE_TRUE or SAF_EXTENDIBLE_FALSE */
		SAF_Set *set		/* OUT: Optional memory for link to the newly declared set. */
		)
{
    SAF_ENTER(saf_declare_set, NULL);
    ss_scope_t  scope=SS_SCOPE_NULL;    /* Scope where the new set will be created. */

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);

    SAF_REQUIRE(SS_FILE(db), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("DATABASE must be a valid handle"));
    SAF_REQUIRE(name, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("NAME cannot be NULL"));
    SAF_REQUIRE(name[0] != '@', SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("NAME must not begin with a leading '@'"));
    SAF_REQUIRE(max_topo_dim >= 0, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("MAX_TOPO_DIM must be positive"));
    SAF_REQUIRE(role == SAF_TIME || role == SAF_SPACE || role == SAF_PARAM || role == SAF_SUITE,
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("ROLE must be SAF_TIME, SAF_SPACE, or SAF_PARAM"));
#ifdef SSLIB_SUPPORT_PENDING
    /* So which is it: MAX_TOPO_DIM==1 or 0<=MAX_TOPO_DIM<=1? */
#endif
    SAF_REQUIRE(role != SAF_TIME || (0 <= max_topo_dim && max_topo_dim <= 1),
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("if ROLE is SAF_TIME then MAX_TOPO_DIM must be 1"));
    SAF_REQUIRE(extmode != SAF_EXTENDIBLE_TORF, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("EXTMODE must be either SAF_EXTENDIBLE_TRUE or SAF_EXTENDIBLE_FALSE"));

                     
    /* Build a set record. */
    ss_file_topscope(db, &scope);
    set = (ss_set_t*)ss_pers_new(&scope, SS_MAGIC(ss_set_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)set, NULL);
    if (!set) 
        SAF_ERROR(NULL,_saf_errmsg("unable to init set record"));

    /* Fill in the set's modeling data. */
    if (SAF_EACH==pmode) SS_PERS_UNIQUE(set);
    ss_string_set(SS_SET_P(set,name), name);
    SS_SET(set)->user_id = 0;
    SS_SET(set)->tdim = max_topo_dim;
    SS_SET(set)->srole = role;
    SS_SET(set)->is_top = TRUE;	/* a set is "always" considered a top at first */
    SS_SET(set)->is_extendible = (hbool_t) extmode;

    SAF_LEAVE(set);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Sets
 * Purpose:     Obtain a set description
 *
 * Description: This function returns information about a set. 
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Modifications:
 *		Robb Matzke, LLNL, 2000-02-03
 *		Added documentation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_set(SAF_ParMode pmode,	/* The parallel mode. */
		 SAF_Set *set,		/* The set to be described. */
		 char **name,		/* OUT: The returned name of the set. Pass NULL if you do not want this information
                                         * returned (see Returned Strings). */
		 int *max_topo_dim,	/* OUT: The topological dimension of the set. A NULL pointer can be passed if the caller is
					 * not interested in obtaining this information. */
		 SAF_SilRole *role,	/* OUT: The subset inclusion lattice role of the set. A NULL pointer can be passed if the
					 * caller is not interested in obtaining this information. */
		 SAF_ExtendMode *extmode,/* OUT: Whether the set is extendible or not. A NULL pointer can be passed if the
                                          * caller is not interested in obtaining this information. */
		 SAF_TopMode *topmode,	/* OUT: Whether the set is a top-level set in the SIL or not */
		 int *num_colls,	/* OUT: The number of collections currently defined on the set. A NULL pointer can be
					 * passed if the caller is not interested in obtaining this information. */
		 SAF_Cat **cats		/* OUT: The list of collection categories of the collections defined on the set. A NULL
                                         * pointer can be passed if the caller is not interested in obtaining this
                                         * information. CATS should point to the NULL pointer if the client wants the library
                                         * to allocate space, otherwise CATS should point to something allocated by the
                                         * caller. In the latter case, the input value of NUM_COLLS indicates the number of
					 * handles the CATS argument can hold. */
		 )
{
    SAF_ENTER(saf_describe_set, SAF_PRECONDITION_ERROR);

    size_t n=0, i;
    ss_collection_t *colls=NULL;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SET must be a valid set handle"));
    SAF_REQUIRE(!cats || num_colls, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM_COLLS must be returned if CATS is requested"));

    /* Return what was requested by the client. */
    n = ss_array_nelmts(SS_SET_P(set,colls));
    if (_saf_setupReturned_string(name, ss_string_ptr(SS_SET_P(set,name))) != SAF_SUCCESS)
        SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable to return set name for set %s\n", ss_string_ptr(SS_SET_P(set,name))));
    if (max_topo_dim) *max_topo_dim = SS_SET(set)->tdim;
    if (role) *role = SS_SET(set)->srole;
    if (topmode) *topmode = (SAF_TopMode) SS_SET(set)->is_top;
    if (extmode) *extmode = (SAF_ExtendMode) SS_SET(set)->is_extendible;
    if (cats && n>0) {
        /* If the collections categories are to be returned and any were found either allocated storage for them or verify that
         * any client supplied storage is sufficient and fill the returned cat handles. */
        if (!*cats) {
            *cats = calloc(n, sizeof(**cats));
            if (!*cats)
                SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable allocate memory for categories"));
        } else if (*num_colls<(int)n) {
            SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("client allocated mem, %i, too small for returned cats, %i", *num_colls, n));
        }
        colls = malloc(n*sizeof(*colls));
        ss_array_get(SS_SET_P(set,colls), ss_pers_tm, (size_t)0, n, colls);
        for (i=0; i<n; i++) {
            (*cats)[i] = SS_COLLECTION(colls+i)->cat;
        }
        colls = SS_FREE(colls);
    }
    if (num_colls) *num_colls = n;

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Sets
 * Purpose:     Find set by matching criteria
 *
 * Description: This function will find sets by searching the *entire* database and matching certain criteria.
 *		Because it finds sets by matching criteria, this function *does*not* exploit the subset inclusion lattice to
 *		improve performance.
 *
 *		If the NAME_GREP argument begins with a leading "at sign" character, '@', the remaining characters will be
 *		treated as a limited form of a regular expression akin to that supported in 'ed'. Otherwise, it will
 *		be treated as a specific name for a set. If the name does not matter, pass SAF_ANY_NAME.
 *
 *		If the library was not compiled with -lgen support library, then if regular expressions are used,
 *		the library will behave as though SAF_ANY_NAME was specified. 
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Modifications:
 *		Robb Matzke, LLNL, 2000-02-03
 *		Added documentation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_matching_sets(SAF_ParMode pmode,	/* The parallel mode. */
		       SAF_Db *db,              /* The database in which to search */
		       const char *name_grep,	/* The name of the desired set(s) or a limited regular expression that the
                                                 * set names must match. If this argument begins with a leading "at sign", '@',
                                                 * character, the remaining characters will be treated as a limited form of
                                                 * a regular expression akin to that supported by 'ed.' The constant SAF_ANY_NAME
						 * can be passed if the client does not want to limit the search by name. */
		       SAF_SilRole srole,	/* The subset inclusion lattice role of the desired set(s). The SAF_ANY_SILROLE
                                                 * constant can be passed if the client is not interested in restricting the
						 * search on this criteria. */
		       int tdim,		/* The topological dimension of the desired set(s). The SAF_ANY_TOPODIM constant
                                                 * can be passed if the client is not interested in restricting the search on this
						 * criteria. */
		       SAF_ExtendMode extmode,  /* User to specify if the set is extendible or not (whether it can grow or not).
						 * Pass SAF_EXTENDIBLE_TRUE, SAF_EXTENDIBLE_FALSE, or SAF_EXTENDIBLE_TORF */
		       SAF_TopMode topmode,	/* whether the matching sets should be top sets. Pass SAF_TOP_TRUE, SAF_TOP_FALSE,
						 * or SAF_TOP_TORF */
		       int *num,		/* For this and the succeeding argument [see Returned Handles]. */
		       SAF_Set **found		/* For this and the preceding argument [see Returned Handles]. */
		       )
{
    SAF_ENTER(saf_find_matching_sets, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Set, key, mask);
    size_t      nfound;
    ss_scope_t  scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_FILE(db), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DATABASE must be a database handle"));
    SAF_REQUIRE(srole == SAF_SUITE || srole == SAF_TIME || srole == SAF_SPACE || srole == SAF_PARAM || srole == SAF_ANY_SILROLE, 
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("the SROLE must be one of SAF_TIME, SAF_SPACE, SAF_PARAM, or SAF_ANY_SILROLE"));
    SAF_REQUIRE(srole != SAF_TIME || tdim == 1, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if SROLE is TIME then TDIM must be 1"));
    SAF_REQUIRE(tdim >= 0 || tdim == SAF_ANY_TOPODIM, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("TDIM must be SAF_ANY_TOPODIM or positive"));
    SAF_REQUIRE(extmode == SAF_EXTENDIBLE_TRUE || extmode == SAF_EXTENDIBLE_FALSE || extmode == SAF_EXTENDIBLE_TORF,
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("EXTMODE cannot be arbitrarily non-zero for truth"));
    SAF_REQUIRE(topmode == SAF_TOP_TRUE || topmode == SAF_TOP_FALSE || topmode == SAF_TOP_TORF,
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("TOPMODE cannot be arbitrarily non-zero for truth"));
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));

    /* fill in record with stuff to look for */ 
    ss_file_topscope(db, &scope);
    if (name_grep) {
        if (name_grep[0] == '@') {
            SAF_SEARCH_RE(SAF_Set, key, mask, name, name_grep+1);
        } else {
            SAF_SEARCH_S(SAF_Set, key, mask, name, name_grep);
        }
    }
    if (srole != SAF_ANY_SILROLE)
        SAF_SEARCH(SAF_Set, key, mask, srole, srole);
    if (tdim != SAF_ANY_TOPODIM)
        SAF_SEARCH(SAF_Set, key, mask, tdim, tdim);
    if (topmode != SAF_TOP_TORF)
        SAF_SEARCH(SAF_Set, key, mask, is_top, topmode);
    if (extmode != SAF_EXTENDIBLE_TORF)
        SAF_SEARCH(SAF_Set, key, mask, is_extendible, extmode);

    /*  Now, find ids of matching records... */
    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_set_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, NULL, NULL);
        if (num) *num = nfound;
    } else {
        /* Find limited matches; client allocates result buffer */
        assert(num);
        nfound = *num;
        if (NULL==ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, (ss_pers_t*)*found,
                               _SAF_GLOBALS.find_detect_overflow)) {
            SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matching objects"));
        }
        *num = nfound;
    }
    SAF_LEAVE(SAF_SUCCESS);
}


/* We need to define these variables so that they are accessible from within a recursively called find function but we don't
 * want to pass this down as an argument because that changes the SAF API. */
static struct {
    int         depth;                  /* Depth in callgraph: 0==top function call; 1==first recursive call; etc. */
    ss_set_t    *sets;                  /* List of found sets, possibly allocated by the find function */
    size_t      nused;                  /* Number of sets currently in the list of sets */
    size_t      nalloc;                 /* Number of elements allocated in the `sets' array. */
} saf_find_sets_g;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Sets
 * Purpose:     Find sets by traversing the subset inclusion lattice
 *
 * Description: There are two ways to search for sets. One is to simply search the whole database looking for sets that
 *		match a particular search criteria such as a name, base dimension, etc. which is handled by
 *		saf_find_matching_sets(). The other is to search for sets by traversing the subset inclusion lattice which is
 *		handled by this function. This latter approach is typically faster as it involves only a portion of all sets in
 *		the database.
 *
 *		The possible modes to the find call are described below.
 *
 *		   FMODE==SAF_FSETS_TOP
 *
 *		This mode of the find will find the top-most ancestor of a given set.
 *
 *		   FMODE==SAF_FSETS_BOUNDARY
 *
 *		This mode of the find will find the boundary set of a given set. Note, currently this mode will return
 *		a boundary only if one exists in the file. It will *not* attempt to compute a boundary.
 *
 *		   FMODE==SAF_FSETS_SUBS
 *
 *		This mode will find all sets that are immediate subsets of the given set by the specified collection category,
 *		if any is specified. If the specified collection category is SAF_ANY_CAT, then all immediate subsets will
 *              be returned, regardless of category.
 *
 *		   FMODE=SAF_FSETS_SUPS
 *
 *		This mode will find all sets that are immediate supersets of the given set by the specified collection category,
 *		if any is specified. If the specified collection category is SAF_ANY_CAT, then all immediate supersets will
 *              be returned, regardless of category. Note, in typical cases, there is often only one superset of a given set 
 *              by a given collection category.
 *
 *                 FMODE=SAF_FSETS_LEAVES 
 *              
 *              This mode finds all leaf sets in the subset inclusion lattice rooted at SET (a leaf set is a set that is a
 *              descendent of SET by the specified collection category and which has no sets below it. SAF_ANY_CAT is allowed
 *              to be the specified collection category.
 *
 *
 * Issue:	If FMODE is SAF_FSETS_TOP, the memory allocation rules for SAF become irrelevant. If we confine ourselves to
 *		topological information (e.g. statements about base-space sets without respect to fields, particularly coordinate
 *		fields), then there is only ever one top (maximal ancestor) for any set. Of course, if we have two totally
 *		independent objects, say a hammer and a wall, such that the hammer has interpenetrated the wall, then the
 *		set that represents the intersection between the hammer and the wall is a subset of both. However, this
 *		intersection is a result of the fact that the hammer's coordinate field places it inside the wall. In the purely
 *		topological setting, the sets that represent the hammer and the wall are each top sets. Thus, in theory
 *		the set that represents their intersection has two maximal ancestors. We do *not* worry about this case here.
 *		Thus, a query for SAF_FSETS_TOP is always a single set and so either the client asks for the single set handle
 *		to be allocated, or it does not.
 *
 *		If FMODE is SAF_FSETS_BOUNDARY, some similar arguments apply. There is only ever one boundary of another set.
 *		Thus, if the client queries for the boundary, it is assumed the client has either allocated a single set handle
 *		or the library will and simply fill it in.
 *
 * 		For the SAF_FSETS_TOP and SAF_FSETS_BOUNDARY cases, cat must be SAF_NOT_APPLICABLE_CAT.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_sets(SAF_ParMode pmode,	/* The parallel mode. */
	      SAF_FindSetMode fmode,    /* The find mode. Possible values are SAF_FSETS_TOP to find the top-level set in the
                                         * subset inclusion lattice in which SET is a member; SAF_FSETS_BOUNDARY to find the
                                         * boundary of set SET; SAF_FSETS_SUBS to find all sets which are immediate subsets of SET
                                         * by the specified collection category; SAF_FSETS_SUPS to find all sets which are
                                         * immediate supersets of SET by the specified collection category; and
                                         * SAF_FSETS_LEAVES to find all leaf sets in the subset inclusion lattice rooted at
                                         * SET (a leaf set is a set that is a descendent of SET by the specified collection
                                         * category and which has no sets below it). */
	      SAF_Set *set,		/* The set in the subset inclusion lattice at which to begin searching. */
	      SAF_Cat *cat,		/* The collection category upon which to search for subsets, supersets, or leaf sets. */
	      int *num,		        /* For this and the succeeding argument [see Returned Handles]. */
	      SAF_Set **found		/* For this and the preceding argument [see Returned Handles]. */	
	      )
{
    SAF_ENTER(saf_find_sets, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Rel, relkey, relmask);
    size_t      nrelsfound;             /* Number of relations found */
    ss_rel_t    *allrels=NULL;          /* Relations that were found */
    ss_scope_t  scope=SS_SCOPE_NULL;    /* The scope of SET used to search for relations defined on SET */
    ss_rel_t    frel=SS_REL_NULL;       /* Found relation */
    ss_set_t    fset=SS_SET_NULL;       /* Found set */
    htri_t      duplicate;              /* True if Set is a duplicate or error occurs */
    ss_set_t    *subs=NULL;             /* Allocated array of subsets */
    int         nsubs=0;                /* Number of subsets in the `subs' array */
    size_t      i, j;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SET must be a valid set handle"));
    SAF_REQUIRE(fmode==SAF_FSETS_TOP || fmode==SAF_FSETS_SUBS || fmode==SAF_FSETS_SUPS || fmode==SAF_FSETS_LEAVES ||
                fmode==SAF_FSETS_BOUNDARY,
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FMODE must be SAF_FSETS... _TOP, _SUBS, _SUPS, _LEAVES or _BOUNDARY"));
    SAF_REQUIRE(((fmode==SAF_FSETS_SUPS || fmode==SAF_FSETS_SUBS || fmode==SAF_FSETS_LEAVES) && (SS_CAT(cat) || !cat)) ||
                ((fmode==SAF_FSETS_TOP || fmode==SAF_FSETS_BOUNDARY) && (!cat || SAF_EQUIV(cat, SAF_NOT_APPLICABLE_CAT))),
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("CAT arg applicable only for SAF_FSETS_SUPS, SAF_FSETS_SUBS and SAF_FSETS_LEAVES modes"));

    /* For recursive SAF_FSETS_LEAVES calls to this function the _saf_valid_memhints() doesn't apply as both are null ptrs. */
    SAF_REQUIRE((SAF_FSETS_LEAVES==fmode && saf_find_sets_g.depth>0) ||
                _saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));
    SAF_REQUIRE(fmode!=SAF_FSETS_LEAVES || saf_find_sets_g.depth>0 || num, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM is required in a top-level SAF_FSETS_LEAVES mode call"));

    /* ISSUE: This function looks for Relations only in the same scope that stores SET and thus cannot traverse a subset
     *        inclusion lattice that extends outside that scope. [rpm 2004-06-21] */
    ss_pers_scope((ss_pers_t*)set, &scope);

    switch (fmode) {
    case SAF_FSETS_BOUNDARY:
        if (found && !*found && NULL==(*found=calloc(1, sizeof(**found))))
            SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("cannot allocate Set return value"));
        if (!SS_PERS_ISNULL(SS_SET_P(set,bnd_set))) {
            if (num) *num = 1;
            if (found) (*found)[0] = SS_SET(set)->bnd_set;
        } else {
            if (num) *num = 0;
        }
        break;

    case SAF_FSETS_TOP:
        /* fill in search criteria */
        SAF_SEARCH(SAF_Rel, relkey, relmask, kind, SAF_RELKIND_EQUAL);
        if (!SS_PERS_ISNULL(cat)) {
            SAF_SEARCH(SAF_Rel, relkey, relmask, sub_cat, *cat);
            SAF_SEARCH(SAF_Rel, relkey, relmask, sup_cat, *cat);
        }

        /* loop until found a top */
        fset = *set;
        while (1) {
           /* Did we found the top? */
            if (SS_SET(&fset)->is_top && SS_SET(&fset)->srole==SS_SET(set)->srole) {
                if (num) *num = 1;
                if (found && !*found && NULL==(*found=calloc(1, sizeof(**found))))
                    SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("cannot allocate Set return value"));
                (*found)[0] = fset;
                break;
            }

            /* Find the next set up */
            SAF_SEARCH(SAF_Rel, relkey, relmask, sub, fset);
            nrelsfound=1;
            if (NULL==ss_pers_find(&scope, (ss_pers_t*)relkey, (ss_persobj_t*)&relmask, (size_t)0, &nrelsfound,
                                   (ss_pers_t*)&frel, NULL))
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("find failed"));
            if (nrelsfound!=1)
                SAF_ERROR(SAF_CONSTRAINT_ERROR,_saf_errmsg("unable to find a top for set \"%s\"",
                                                           ss_string_ptr(SS_SET_P(set,name))));
            fset = SS_REL(&frel)->sup;
        }
        break;

    case SAF_FSETS_SUBS:
    case SAF_FSETS_SUPS:
        /* Fill in search criteria. */
        if (fmode == SAF_FSETS_SUBS) {
            SAF_SEARCH(SAF_Rel, relkey, relmask, sup, *set);
        } else {
            SAF_SEARCH(SAF_Rel, relkey, relmask, sub, *set);
        }
        SAF_SEARCH(SAF_Rel, relkey, relmask, kind, SAF_RELKIND_EQUAL);
        if (!SS_PERS_ISNULL(cat)) {
            SAF_SEARCH(SAF_Rel, relkey, relmask, sub_cat, *cat);
            SAF_SEARCH(SAF_Rel, relkey, relmask, sup_cat, *cat);
        }

        /* Find matching relations */
        nrelsfound = SS_NOSIZE;
        allrels = (ss_rel_t*)ss_pers_find(&scope, (ss_pers_t*)relkey, (ss_persobj_t*)&relmask, (size_t)0, &nrelsfound, NULL, NULL);
        if (!allrels)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("failed when finding relations associated with the set"));

        /* If the category is SAF_ANY_CAT, then allrels[] will hold a link to each relation of the scope, regardless of
         * category. This means we might end up with sets listed multiple times. We must prune the list of sets so that
         * each set is mentioned at most one time. This fixes HYPer02781: saf_find_sets() won't accept SAF_ANY CAT. */
        for (i=1; i<nrelsfound; i++) {
            if (SAF_FSETS_SUPS==fmode) {
                for (j=0, duplicate=FALSE; j<i && !duplicate; j++)
                    duplicate = SS_PERS_EQ(SS_REL_P(allrels+j,sup), SS_REL_P(allrels+i,sup));
            } else {
                for (j=0, duplicate=FALSE; j<i && !duplicate; j++)
                    duplicate = SS_PERS_EQ(SS_REL_P(allrels+j,sub), SS_REL_P(allrels+i,sub));
            }
            if (duplicate) {
                allrels[i] = allrels[nrelsfound-1]; /*replace current rel with the last rel*/
                --nrelsfound; /*we discarded one relation*/
                --i; /*repeat loop at current location*/
            }
        }
        
        /* Return what the caller asked for. */
        if (!found) {
            /* Count the matches */
            assert(num);
            *num = nrelsfound;
        } else if (!*found) {
            /* Find all matches; library allocates results */
            if (NULL==(*found=malloc(MAX(nrelsfound,1)*sizeof(**found))))
                SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate return value"));
            if (num) *num = nrelsfound;
        } else {
            /* Find limited matches; client allocates result buffer */
            assert(num);
            if (nrelsfound>(size_t)*num)
                SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matching objects"));
            *num = nrelsfound;
        }
        if (found) {
            for (i=0; i<nrelsfound; i++) {
                if (SAF_FSETS_SUBS==fmode) {
                    (*found)[i] = SS_REL(allrels+i)->sub;
                } else {
                    (*found)[i] = SS_REL(allrels+i)->sup;
                }
            }
        }
        break;
        
    case SAF_FSETS_LEAVES:
        /* Finding the leaves is a recursive operation: For the given set, find the leaves of all its subsets and merge those
         * lists, removing duplicates. */

        /* ISSUE: For a SAF_FSETS_LEAVES search with a null collection category (SAF_ANY_CAT) this function will return a list
         *        of unique sets by pruning out the duplicates. However, the pruning occurs down at the leaves and not in the
         *        internal nodes of the graph, and therefore we may end up traversing portions of the graph repeatedly.
         *        [rpm 2004-06-21] */

        if (0==saf_find_sets_g.depth)
            memset(&saf_find_sets_g, 0, sizeof saf_find_sets_g);

        /* Make sure this set isn't already in the list of sets to be returned. */
        for (i=0, duplicate=FALSE; i<saf_find_sets_g.nused && !duplicate; i++) {
            duplicate = SS_PERS_EQ(set, saf_find_sets_g.sets+i);
        }
        if (duplicate) {
            if (num) *num = 0;
            goto done;
        }

        /* Find this set's subsets */
        subs = NULL;
        saf_find_sets(pmode, SAF_FSETS_SUBS, set, cat, &nsubs, &subs);

        if (0==nsubs) {
            /* This set has no subsets: it must be a leaf. Add it to the list of sets since we already know it's not a
             * duplicate of anything in that list. */
            if (saf_find_sets_g.nused>=saf_find_sets_g.nalloc) {
                saf_find_sets_g.nalloc = MAX(64, 2*saf_find_sets_g.nalloc);
                if (NULL==(saf_find_sets_g.sets=realloc(saf_find_sets_g.sets,
                                                        saf_find_sets_g.nalloc*sizeof(saf_find_sets_g.sets[0]))))
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate list of matching sets"));
            }
            saf_find_sets_g.sets[saf_find_sets_g.nused++] = *set;
        } else {
            /* Recursively add leaves of the subsets */
            assert(nsubs>0); /*for cast*/
            for (i=0; i<(size_t)nsubs; i++) {
                saf_find_sets_g.depth++;
                saf_find_sets(pmode, fmode, subs+i, cat, NULL, NULL); /*return values added to saf_find_sets_g*/
                saf_find_sets_g.depth--;
            }
        }

        /* Free up subset list */
        subs = SS_FREE(subs);
        nsubs = 0;

        /* When the top-level call is about to return, saf_find_sets_g contains the array of sets to be returned. Either
         * return that array, copy the sets into a caller-supplied array, or just return the count. */
        if (0==saf_find_sets_g.depth) {
            if (!found) {
                /* Count the matches */
                assert(num);
                *num = saf_find_sets_g.nused;
                saf_find_sets_g.sets = SS_FREE(saf_find_sets_g.sets);
            } else if (!*found) {
                /* Find all matches; library allocates results */
                *found = saf_find_sets_g.sets;
                if (num) *num = saf_find_sets_g.nused;
                saf_find_sets_g.sets = NULL;
            } else {
                /* Find limited matches; client allocates result buffer */
                assert(num);
                if ((int)saf_find_sets_g.nused>*num)
                    SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matching objects"));
                memcpy(*found, saf_find_sets_g.sets, saf_find_sets_g.nused*sizeof(*found));
                *num = saf_find_sets_g.nused;
                saf_find_sets_g.sets = SS_FREE(saf_find_sets_g.sets);
            }
            memset(&saf_find_sets_g, 0, sizeof saf_find_sets_g); /*cleanup for next call*/
        }
        break;
    }

    SS_FREE(allrels);
done:
    SAF_LEAVE(SAF_SUCCESS);

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Sets
 * Purpose:    	Put an attribute to a set
 *
 * Description: This function is identical to the generic saf_put_attribute() function except that it is specific to
 *    		SAF_Set objects to provide the client with compile time type checking. For a description,
 *		see saf_put_attribute().
 *
 * Programmer:
 *		Mark Miller, LLNL, 03/10/00
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_put_set_att(SAF_ParMode pmode, SAF_Set *set, const char *key, hid_t type, int count, const void *value)
{
  SAF_ENTER(saf_put_set_att, SAF_PRECONDITION_ERROR);
  int retval = saf_put_attribute(pmode, (ss_pers_t*)set, key, type, count, value);
  SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Sets
 * Purpose:    	Get an attribute from a set
 *
 * Description: This function is identical to the generic saf_get_attribute() function except that it is specific to
 *    		SAF_Set objects to provide the client with compile time type checking. For a description,
 *		see saf_get_attribute().
 *
 * Programmer:
 *		Mark Miller, LLNL, 03/10/00
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_set_att(SAF_ParMode pmode, SAF_Set *set, const char *key, hid_t *type, int *count, void **value)
{
  SAF_ENTER(saf_get_set_att, SAF_PRECONDITION_ERROR);
  int retval = saf_get_attribute(pmode, (ss_pers_t*)set, key, type, count, value);
  SAF_LEAVE(retval);
}

