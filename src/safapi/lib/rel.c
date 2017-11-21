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
 * Chapter:	Topology Relations
 *
 * Description: Topology relations are used to define the inter-relationships between the members of a collection
 *		and how those members are knitted together to form a /mesh/. For more information, see [Relation Notes] and 
 *		saf_declare_topo_relation().
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Note:	Relation Notes
 * Description:
 *	In theory, every kind of relationship we might wish to define between sets is simply a mathematical relation. In fact,
 *	even fields are relations between sets representing the base space of the field and sets representing the set of possible
 *	values the field can attain over its base space.
 *
 *	In practice, we have need to distinguish between different kinds of relations. Of course, fields are characterized
 *	as fields. However, for relationships between sets, we define two special cases; /subset/relations/ and
 *	/topology/relations/. A subset relation identifies that two sets are related to each other, one the subset of the
 *	other. Furthermore that subset is identified by enumerating those members of a collection in the superset that are 
 *	also in the subset. For example, to specify a processor subset of a whole, we might identify all those elements
 *	on the whole that are on the processor.
 *
 *	The other kind of relation we define is a topology relation. Another good name might have been /mesh/ relation. 
 *	However, we have tried to avoid words, like mesh, that might have specific and overloaded meanings across the various
 *	application domains SAF is designed to support. A topology relation defines how different members of a collection
 *	are knitted together to form some larger piece. For example, a topology relation is used to define how different
 *	elements are knitted together (at the nodes) to form a finite element mesh.
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Subset Relations
 * Description: Subset relations are used to define a relationship between two sets in which one set, the intended 
 *		/subset/, is the subset of the other set, the intended /superset/. In order to define a subset relation
 *		both sets require a common, decomposing collection. That is, there must exist a collection of the same
 *		category on both sets and that collection must be a decomposition of its containing set. For more information,
 *		see [Relation Notes] and saf_declare_subset_relation().
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Subset Relations
 * Purpose:	Declare a subset relation
 *
 * Description: This call is used to declare a subset relation between two sets. The relation is specified in terms of
 *              collections on both sets. The subset, SUB, can be either a boundary of SUP or not. Which case is indicated
 *            	by the SBMODE argument which can be either SAF_BOUNDARY_TRUE or SAF_BOUNDARY_FALSE.
 *
 *		In addition, The *members*of*the*collection* on the SUB set are either *on*the*boundary*of* the
 *		members of the collection on the SUP set or not (the only other acceptable case is one in which the members
 *		of the collection on the SUB are *equal*to* the members of the collection on SUP). Which case is indicated by
 *		the value of the CBMODE argument, can be either SAF_BOUNDARY_TRUE or SAF_BOUNDARY_FALSE.
 *
 *		Thus, there are two statements made about boundary information. One about the sets, SUP and SUB and one
 *		about the members of the collections on SUP and SUB. Furthermore, the statement about the sets, indicated
 *		by SBMODE, is that SUB is *the* boundary of SUP or it is not. The statement about the collections,
 *		indicated by CBMODE, is that the members of the SUB collection are *on*the*boundary*of* the members of 
 *		the SUP collection or not. 
 *
 *		The values in ABUF enumerate the members of the collection on SUB that are either on the boundaries of or
 *		equal to the members of the collection on SUP. In the *on*the*boundary*of* case (e.g. CBMODE==SAF_BOUNDARY_TRUE)
 *		the values in BBUF, if non-NULL, indicate "which" piece of the SUP collection member's boundary each member
 *		of SUB collection is. For example, if the SUB collection is faces and the SUP collection is a bunch of hexes,
 *		BBUF can be used to identify which of the 6 faces each member of SUB collection is. This information is
 *		optional.
 *
 *		The group of four formal arguments SUP_CAT, SUB_CAT, SBMODE, CBMODE select from the various cases described
 *		above. For convenience, we provide a number of macros for these four arguments for the common cases...
 *
 *              SAF_COMMON(C) : a subset relationship in which the subset is specified by enumerating those members of
 *              the superset that are *in* the subset. This is the most common case. Argument C is the collection
 *              category both sets have in common.
 *
 *              SAF_BOUNDARY(P,B) : a subset relationship in which SUB is *the* boundary of SUP and the
 *              members of B on SUB are on the boundaries of the members of P on SUP.
 *
 *		SAF_EMBEDBND(P,B) : a subset relationship in which SUB is some embedded boundary in SUP and
 *		members of the collection B on SUB are on the boundaries of the members of collection P on SUP.
 *
 *              SAF_GENERAL(BND) : a subset relationship in which all that is known is that SUB is indeed a subset of SUP
 *              The details of the relationships are not known. In this case, the BND is a boolean indicating
 *		if SUB is *the* boundary of SUP.
 *
 *		Finally, there is the subset relation representation type, SRTYPE...
 *
 *		By and large, the details of the relation data can be derived from knowledge of the indexing schemes used in
 *		the domain and range collections of the relation. For example, if the range is indexed using some N
 *		dimensional indexing scheme, then the relation will either be an N dimensional hyperslab or a list of N-tuples.
 *
 *		In the case of SAF_HSLAB, it is assumed the memory pointed to by ABUF contains 3 N-tuples of the form (starts,
 *		counts, strides) where starts, counts, and strides indicate the starting point of the hyperslab in N
 *		dimensions, the number of items in each dimension and the stride (through the range collection) in each
 *		dimension respectively. The order of dimensional axes in each of these arrays is assumed to match the terms in
 *		which the range collection's indexing is specified.
 *
 *		In the case of SAF_TUPLES, it is assumed the memory pointed to by ABUF contains a list of N-tuples where each N
 *		tuple identifies one member of the range collection. The offsets argument to SAF_TUPLES, if present, indicates
 *		a fixed N-tuple offset to be associated with each N-tuple in ABUF.
 *
 *		There are two ways the client may pass the data buffer holding the relation data; either here as the ABUF
 *		argument of the declare call or later as the ABUF argument of the write call. The client cannot do both. It
 *		must choose. This flexibility was provided to aim the API for in-memory communications as well as persistent
 *		file writes. By and large, the client should pass NULL for the ABUF arg here and pass the buffer in the write
 *		call whenever it is writing persistent data to the file. However, whenever the client is planning to do
 *		in-memory communication, it should specify ABUF here.
 *
 * Issue:	We may want to have separate datatypes for ABUF and BBUF as BBUF's values are likely to always fit in
 *		a byte though I don't know any clients that actually store them that way.
 *
 *		At present, we assert that for any boundary case, the range collection (on SUP) must be of a primitive
 *		type (e.g. not SAF_CELLTYPE_SET). However, this *really* need not be the case. Any set which has a boundary set
 *		can then have "pieces" of that boundary that could be referred to use the local address space of that
 *		boundary. It just so happens that the most common case for this is when we are referring to cell-types.
 *
 * Return:	On success returns either the supplied REL argument or a pointer to a newly allocated relation link. Returns
 *              the null pointer on failure.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Rel *
saf_declare_subset_relation(SAF_ParMode pmode,		/* The parallel mode. */
                            SAF_Db *db,                 /* The database in which to place the new relation. */
			    SAF_Set *sup,		/* The superset. In SAF_ONE parallel mode, all processors except the
                                                         * one identified by the SAF_ONE argument should pass the null set of
							 * the database by using the SAF_NULL macro. */
			    SAF_Set *sub,		/* The subset. In SAF_ONE parallel mode, all processors except the one
                                                         * identified by the SAF_ONE argument should pass the null set of the
							 * database by using the SAF_NULL macro. */
			    SAF_Cat *sup_cat,	        /* The collection category on the SUP set upon which
                                                         * the subset relation is being defined. Note that collections of this
                                                         * category must have already been defined on SUP. Otherwise, an
                                                         * error is generated. Note, the four args, SUP_CAT, SUB_CAT, SBMODE,
                                                         * CBMODE, are typically passed using one of the macros described above, 
                                                         * SAF_COMMON(C), SAF_BOUNDARY(P,B), SAF_EMBEDBND(P,B) or
                                                         * SAF_GENERAL(BND) */
			    SAF_Cat *sub_cat,           /* The collection category on the SUB set upon which the subset relation
                                                         * is being defined. Note that collections of this category must have
							 * already been defined on SUB. Otherwise an error is generated. */
			    SAF_BoundMode sbmode,       /* Indicates whether SUB is the boundary of SUP. Pass either
							 * SAF_BOUNDARY_TRUE or SAF_BOUNDARY_FALSE */
			    SAF_BoundMode cbmode,       /* Indicates whether *members* of collection on SUB are *on* the
                                                         * boundary of members of the collection on SUP. Pass either
                                                         * SAF_BOUNDARY_TRUE or SAF_BOUNDARY_FALSE */
			    SAF_RelRep *srtype,		/* Subset relation types. This argument describes how the data in ABUF
                                                         * represents the subset. Valid values are SAF_HSLAB meaning that ABUF
                                                         * points to a hyperslab specification and SAF_TUPLES meaning that ABUF
							 * points to a list of N-tuples. */
			    hid_t A_type,		/* The type of the data in A_BUF */
			    void *A_buf,		/* This buffer contains references, one for each member of the domain
                                                         * collection (on SUB), to members of the range collection (on SUP).
                                                         * The client may pass NULL here meaning that the raw data will be bound
							 * to the object during write, rather than declaration. */
			    hid_t B_type,		/* The type of the data in B_BUF */
			    void *B_buf,		/* This buffer is valid *only* when the members of the domain collection
                                                         * (on SUB) are on the boundaries of the members of the range collection
                                                         * (on SUP). In this case, the data contained in this buffer identifies
                                                         * "which piece" of the boundary each member of the domain collection is.
                                                         * Otherwise, the client should pass NULL here.
                                                         * As with ABUF, the client may pass also NULL here meaning the raw data
							 * will be bound to the object during write, rather than declaration. */
			    SAF_Rel *rel		/* OUT: Optional returned relation handle. */
			    )
{
    SAF_ENTER(saf_declare_subset_relation, NULL);
    ss_scope_t          scope;                          /* The scope where the new relation will be created. */
    ss_collection_t     sub_coll=SS_COLLECTION_NULL, sup_coll=SS_COLLECTION_NULL;
    int                 sub_count, sup_ndims;
    ss_indexspec_t      idx;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);
    ss_file_topscope(db, &scope);

    SAF_REQUIRE(SS_SET(sup), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("SUP must be a valid set handle"));
    SAF_REQUIRE(SS_SET(sub), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("SUB must be a valid set handle"));
    SAF_REQUIRE(sbmode==SAF_BOUNDARY_TRUE || sbmode==SAF_BOUNDARY_FALSE, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("SBMODE must be either SAF_BOUNDARY_TRUE or SAF_BOUNDARY_FALSE"));
    SAF_REQUIRE(cbmode==SAF_BOUNDARY_TRUE || cbmode==SAF_BOUNDARY_FALSE, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("CBMODE must be either SAF_BOUNDARY_TRUE or SAF_BOUNDARY_FALSE"));
    SAF_REQUIRE((sbmode==SAF_BOUNDARY_TRUE && cbmode==SAF_BOUNDARY_TRUE) || sbmode==SAF_BOUNDARY_FALSE,
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("CBMODE must be SAF_BOUNDARY_TRUE if SBMODE is SAF_BOUNDARY_TRUE for all participating processes"));
    SAF_REQUIRE((!SS_CAT(sup_cat) && !SS_CAT(sub_cat) && !A_buf) || (SS_CAT(sup_cat) && SS_CAT(sub_cat)),
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("Either A_BUF is null and both SUP_CAT and SUB_CAT are not valid cat handles or"
                            "SUP_CAT and SUB_CAT are both valid cat handles"));
    SAF_REQUIRE((B_buf && cbmode==SAF_BOUNDARY_TRUE) || !B_buf, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("B_BUF can be non-NULL only when CBMODE is SAF_BOUNDARY_TRUE"));
    SAF_REQUIRE((_saf_is_self_decomp(sup_cat) && _saf_is_self_decomp(sub_cat) &&
                 cbmode!=SAF_BOUNDARY_TRUE && sbmode!=SAF_BOUNDARY_TRUE) ||
                (!_saf_is_self_decomp(sup_cat) && !_saf_is_self_decomp(sub_cat)),
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("on the reserved, \"self\" collection, CBMODE and SBMODE must be SAF_BOUNDARY_FALSE"));
    SAF_REQUIRE(SS_RELREP(srtype), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("SRTYPE must be a valid relation representation handle"));
    SAF_REQUIRE(SAF_HSLAB_ID==SS_RELREP(srtype)->id || SAF_TUPLES_ID==SS_RELREP(srtype)->id,
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("SRTYPE must be either SAF_HSLAB or SAF_TUPLES"));
    SAF_REQUIRE(rel, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("REL must be non-NULL"));
    SAF_REQUIRE(A_type<=0 || H5T_INTEGER==H5Tget_class(A_type), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("A_TYPE must be an integer type if supplied"));
    SAF_REQUIRE(B_type<=0 || H5T_INTEGER==H5Tget_class(B_type), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("B_TYPE must be an integer type if supplied"));

    if (SS_CAT(sup_cat) && SS_CAT(sub_cat)) {
        _saf_getCollection_set(sub, sub_cat, &sub_coll);
        _saf_getCollection_set(sup, sup_cat, &sup_coll);

        /* confirm sup collecton has its respective category defined on it */
        if (SS_PERS_ISNULL(&sup_coll)) {
	    /* instantiate the self collection on the sup set, if necessary */
	    if (_saf_is_self_decomp(sup_cat)) {
                if (saf_declare_collection(pmode, sup, sup_cat, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE) != SAF_SUCCESS) {
                    SAF_ERROR(NULL, _saf_errmsg("unable to instantiate self collection on set \"%s\"",
                                                ss_string_ptr(SS_CAT_P(sup_cat,name))));
                }
	    } else {
                SAF_ERROR(NULL,_saf_errmsg("set \"%s\" does not have a collection of category \"%s\"",
                                           ss_string_ptr(SS_SET_P(sup,name)), ss_string_ptr(SS_CAT_P(sup_cat,name))));
            }
        }

        /* confirm sub collecton has its respective category defined on it */
        if (SS_PERS_ISNULL(&sub_coll)) {
	    /* instantiate the self collection on the sub set, if necessary */
	    if (_saf_is_self_decomp(sub_cat)) {
                if (saf_declare_collection(pmode, sub, sub_cat, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE)!=SAF_SUCCESS) {
                    SAF_ERROR(NULL, _saf_errmsg("unable to instantiate self collection on set \"%s\"",
                                                ss_string_ptr(SS_CAT_P(sub_cat,name))));
                }
                sub_count = 1;
                sup_ndims = 1;
	    } else {
                SAF_ERROR(NULL, _saf_errmsg("set \"%s\" does not have a collection of category \"%s\"",
                                            ss_string_ptr(SS_SET_P(sub,name)), ss_string_ptr(SS_CAT_P(sub_cat,name))));
            }
        } else {
            /* obtain the count of the domain */
            sub_count = SS_COLLECTION(&sub_coll)->count;

            /* obtain the default indexing scheme of the range */
            ss_array_get(SS_COLLECTION_P(&sup_coll,indexing), ss_pers_tm, (size_t)0, (size_t)1, &idx);
            sup_ndims = SS_INDEXSPEC(&idx)->ndims;
        }
    } else {
        /* if both sup_cat and sub_cat are NULL, it is the general case */
        sub_count = 0;
        sup_ndims = 0;
    }

    /* Initialize the relation record */
    rel = (ss_rel_t*)ss_pers_new(&scope, SS_MAGIC(ss_rel_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)rel, NULL);
    if (!rel) SAF_ERROR(NULL, _saf_errmsg("unable to create or initialize the new relation object and/or handle"));
    if (SAF_EACH==pmode) SS_PERS_UNIQUE(rel);
    SS_REL(rel)->sub = *sub;
    if (sub_cat) SS_REL(rel)->sub_cat = *sub_cat;
    SS_REL(rel)->sup = *sup;
    if (sup_cat) SS_REL(rel)->sup_cat = *sup_cat;
    SS_REL(rel)->kind = cbmode==SAF_BOUNDARY_TRUE ? SAF_RELKIND_BOUND : SAF_RELKIND_EQUAL;
    SS_REL(rel)->rep_type = *srtype;

    /* If the subset is the boundary of the superset, then set the bnd_set_id member of superset */
    if (sbmode == SAF_BOUNDARY_TRUE) {
        SAF_DIRTY(sup, pmode);
        SS_SET(sup)->bnd_set = *sub;
    }

    /* Issue: If we could guarantee all processors' is_top member were identical, we could wrap this call so that we don't try
     *        to put the set record if its already NOT a top set. */

    /* Set the "is_top" member of the subset to false */
    
    /* The test whether parent and child sroles are the same has been added as a result of the new 'cross-product base space'.
     * The 'mesh space' topset now becomes a subset of the new Suite set, but we want the conceptual topset of the mesh space
     * to still be flagged as a topset.  The srole of the suite is SAF_SUITE and the srole of the mesh space topset is
     * SAF_SPACE.  By checking to make sure the sroles are equal before changing is_top to false, we can ensure that the mesh
     * space stays flagged with is_top true.
     * 
     * The is_top in should probably be changed from hbool_t to another type that allows for us to have multiple topsets for
     * different sroles.  SPACE topsets, SUITE topsets, TIME topsets, etc. */
    if (SS_SET(sup)->srole == SS_SET(sub)->srole) {
        SAF_DIRTY(sub, pmode);
        SS_SET(sub)->is_top = FALSE;
    }

    /* Fill in the handle and return it. */
    SS_REL(rel)->m.abuf = A_buf;
    SS_REL(rel)->m.abuf_type = A_type;
    SS_REL(rel)->m.bbuf = B_buf;
    SS_REL(rel)->m.bbuf_type = B_type;

    /* Calculate size stuff. In general case, the bufs are zero-sized. */
    if (!SS_CAT(sup_cat) && !SS_CAT(sub_cat)) {
        SS_REL(rel)->m.abuf_size = 0;
        SS_REL(rel)->m.bbuf_size = 0;
    } else {
        if (SAF_HSLAB_ID==SS_RELREP(srtype)->id)
            SS_REL(rel)->m.abuf_size = 3 * sup_ndims;           /* start, length and stride for each dim of sup */
        if (SAF_TUPLES_ID==SS_RELREP(srtype)->id)
            SS_REL(rel)->m.abuf_size = sub_count * sup_ndims;   /* one ndim-tuple for each member of sub */
        SS_REL(rel)->m.bbuf_size = sub_count;
    }

    SAF_LEAVE(rel);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Subset Relations
 * Purpose:	Find subset relations
 *
 * Description: This function finds any subset relations that might exist between two sets or a subset relation
 *		on a specific collection category.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_subset_relations(SAF_ParMode pmode,    /* The parallel mode. */
                          SAF_Db *db,           /* Database in which to limit the search. */
			  SAF_Set *sup,         /* The superset to limit search to. */
			  SAF_Set *sub,         /* The subset to limit search to. */
			  SAF_Cat *sup_cat,     /* The collection category on the superset to limit search to. Pass
                                                 * SAF_ANY_CAT if you do not want to limit the search to any particular
                                                 * category. */
                          SAF_Cat *sub_cat,     /* The collection category on the subset to limit search to. Pass SAF_ANY_CAT
                                                 * if you do not want to limit the search to any particular category. */
                          SAF_BoundMode sbmode, /* If SAF_BOUNDARY_TRUE, limit search to relations in which the subset is the
                                                 * boundary of the superset. */
                          SAF_BoundMode cbmode, /* If SAF_BOUNDARY_TRUE, limit search to relations in which the members of the
                                                 * subset are on the boundaries of the members of the superset. */
			  int *num,             /* For this and the succeeding argument, (see Returned Handles). */
			  SAF_Rel **found       /* For this and the preceding argument, (see Returned Handles). */
			  )
{
    SAF_ENTER(saf_find_subset_relations, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Rel, key, mask);
    size_t      nfound;
    ss_scope_t  scope=SS_SCOPE_NULL;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(ss_file_isopen(db, NULL)>0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DB must be a valid database"));
    SAF_REQUIRE(!sup_cat || SS_CAT(sup_cat), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SUP_CAT must either be a valid category handle or SAF_ANY_CAT"));
    SAF_REQUIRE(!sub_cat || SS_CAT(sub_cat), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SUB_CAT must either be a valid category handle or SAF_ANY_CAT"));
    SAF_REQUIRE(SS_SET(sub), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SUB must be a valid set handle"));
    SAF_REQUIRE(SS_SET(sup), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SUP must be a valid set handle"));
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM and FOUND must be compatible for the return value allocation"));

    ss_file_topscope(db, &scope);

    /* See if the superset's bnd_set matches the sbmode search criteria. */
    if ((sbmode==SAF_BOUNDARY_TRUE && SAF_EQUIV(SS_SET_P(sup,bnd_set),sub)) || sbmode==SAF_BOUNDARY_FALSE) {
        /* The sbmode search criteria matches.  Continue to find the subset relations matching the other criteria. */
        SAF_SEARCH(SAF_Rel, key, mask, sub, *sub);
        SAF_SEARCH(SAF_Rel, key, mask, sup, *sup);
        if (sup_cat) SAF_SEARCH(SAF_Rel, key, mask, sup_cat, *sup_cat);
        if (sub_cat) SAF_SEARCH(SAF_Rel, key, mask, sub_cat, *sub_cat);
        SAF_SEARCH(SAF_Rel, key, mask, kind, cbmode==SAF_BOUNDARY_TRUE?SAF_RELKIND_BOUND:SAF_RELKIND_EQUAL);

        if (!found) {
            /* Count the matches */
            assert(num);
            nfound = SS_NOSIZE;
            ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
            *num = nfound;
        } else if (!*found) {
            /* Find all matches; library allocates results */
            nfound = SS_NOSIZE;
            *found = (ss_rel_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound,
                                             NULL, NULL);
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
    } else {
        if (!found || *found) {
            /* Count the matches or client allocates result buffer */
            assert(num);
            *num = 0;
        } else {
            /* library allocates results */
            if (num) *num = nfound;
        }
    }
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Subset Relations
 * Purpose:	Get a description of a subset relation
 *
 * Description: Returns information about a subset relation.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_subset_relation(SAF_ParMode pmode,         /* The parallel mode. */
                             SAF_Rel *rel,              /* The relation handle. */
                             SAF_Set *sup,              /* OUT: The superset. Pass NULL if you do not want this value returned. */
                             SAF_Set *sub,              /* OUT: The subset. Pass NULL if you do not want this value returned. */
                             SAF_Cat *sup_cat,          /* OUT: The collection category on the SUP set upon which the subset
                                                         * relation is defined. Note that collections of this category must
                                                         * have already been defined on SUP. Otherwise, an error is generated.
                                                         * Note that the four args SUP_CAT, SUB_CAT, SBMODE, and CBMODE are
                                                         * typically passed using one of the macros described in the
                                                         * saf_declare_subset_relation() call, SAF_COMMON(), SAF_BOUNDARY(),
                                                         * SAF_EMBEDBND() or SAF_GENERAL(). Pass NULL if you do not want this
                                                         * value returned. */
                             SAF_Cat *sub_cat,          /* OUT: The collection category on the SUB set upon which the subset
                                                         * relation is defined. Again, pass NULL if you do not want this value
                                                         * returned. */
                             SAF_BoundMode *sbmode,     /* OUT: Indicates whether SUB is the boundary of SUP. A value of
                                                         * SAF_BOUNDARY_TRUE, indicates that the SUB is a boundary of SUP. A
                                                         * value of SAF_BOUNDARY_FALSE indicates SUB is *not* a boundary of
                                                         * SUP. Pass NULL if you do not want this value returned. */
                             SAF_BoundMode *cbmode,     /* OUT: Indicates whether *members* of collection on SUB are *on* the
                                                         * boundaries of members of the collection on SUP. A value of
                                                         * SAF_BOUNDARY_TRUE indicates they are. A value of SAF_BOUNDARY_FALSE
                                                         * indicates they are not. Pass NULL if you do not want this value
                                                         * returned. */
                             SAF_RelRep *srtype,        /* OUT: The representation specification. Pass NULL if you do not want
                                                         * this handle returned. See saf_declare_subset_relation() for the
                                                         * meaning of values of this argument. */
                             hid_t *data_type	        /* OUT: The data-type of the data stored with the relation. Pass NULL
                                                         * if you do not want this value returned. */
			     )
{
    SAF_ENTER(saf_describe_subset_relation, SAF_PRECONDITION_ERROR);
    hid_t               ftype=-1;               /* File datatype */
    ss_collection_t     sub_coll;               /* Collection on relation's subset */

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("the REL argument must be a valid handle"));

    /* Return desired params. */
    if (sup) *sup = SS_REL(rel)->sup;
    if (sub) *sub = SS_REL(rel)->sub;
    if (sup_cat) *sup_cat = SS_REL(rel)->sup_cat;
    if (sub_cat) *sub_cat = SS_REL(rel)->sub_cat;
    if (cbmode) *cbmode = SS_REL(rel)->kind==SAF_RELKIND_BOUND ? SAF_BOUNDARY_TRUE : SAF_BOUNDARY_FALSE;
    if (sbmode) {
        if (SS_REL(rel)->kind == SAF_RELKIND_BOUND) {
            /* This subset relation is SOME kind of boundary relation.  The subset MAY be the boundary of the superset.  Look
             * at the superset record to find out if it is. */
            *sbmode = SAF_EQUIV(SS_SET_P(SS_REL_P(rel,sup),bnd_set), SS_REL_P(rel,sub)) ? SAF_BOUNDARY_TRUE : SAF_BOUNDARY_FALSE;
        } else {
            *sbmode = SAF_BOUNDARY_FALSE;
        }
    }
    if (srtype) *srtype = SS_REL(rel)->rep_type;
    if (data_type) {
        if (!SS_PERS_ISNULL(SS_REL_P(rel,r_blob))) {
            ss_blob_bound_f(SS_REL_P(rel,r_blob), NULL, NULL, NULL, NULL, &ftype);
            *data_type = H5Tget_native_type(ftype, H5T_DIR_DEFAULT);
            H5Tclose(ftype); ftype=-1;
        } else {
            /* Checking for the special case, where the subset collection has a count of 1, is of type SET and is a
             * decomposition. */
            _saf_getCollection_set(SS_REL_P(rel,sub), SS_REL_P(rel,sub_cat), &sub_coll);
            if (SS_COLLECTION(&sub_coll)->count<=1 &&
                SS_COLLECTION(&sub_coll)->cell_type==SAF_CELLTYPE_SET &&
                SS_COLLECTION(&sub_coll)->is_decomp) {
                if (data_type) *data_type = H5Tcopy(ss_pers_tm);
	    } else {
                SAF_ERROR(SAF_CONSTRAINT_ERROR,_saf_errmsg("cannot return data_type and/or file prior to writing"));
            }
        }
    }
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Subset Relations
 * Purpose:	Set the destination form of a subset relation
 *
 * Description:	This function establishes the target (destination) form of subset relation data during either read or write.
 *		When used prior to a write call, it establishes the form of data in the file. When used prior to a read call,
 *		it establishes the form of data as desired in memory.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issues:	Not implemented yet.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_target_subset_relation(SAF_RelTarget *target,       /* OUT: Relation targeting information to be initialize herein. */
			   SAF_RelRep *srtype,          /* Target subset relation types. */
			   hid_t type         	        /* Target data types. */
			   )
{
   SAF_ENTER(saf_target_subset_relation, SAF_PRECONDITION_ERROR);

   memset(target, 0, sizeof *target);
   target->is_set = TRUE;
   target->data_type = type;

   SAF_ERROR(SAF_NOTIMPL_ERROR, _saf_errmsg("not implemented yet"));
   SAF_LEAVE(SAF_NOTIMPL_ERROR);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Subset Relations
 * Purpose:	Reuse data in a subset relation	
 *
 * Description:	This call binds data for an existing relation to a new relation. This call can be used in place of a
 *		saf_write_subset_relation() call if the data that would have been written in the subset relation is
 *		identical to some other relation data already written to the database.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Mark Miller LLNL, 2002-07-23
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_use_written_subset_relation(SAF_ParMode pmode,      /* the parallel mode. */
                                SAF_Rel *rel,		/* The handle for the relation to be updated. */
                                SAF_Rel *oldrel,        /* The handle for the relation pointing to the data to be re-used. */
                                hid_t A_buf_type,	/* The type of data that would be written for the A buffer (see
							 * saf_write_subset_relation()) if this call was actually doing any
							 * writing. */
                                hid_t B_buf_type,	/* The type of data that would be written for the B buffer (see
							 * saf_write_subset_relation()) if this call was actually doing any
							 * writing. */
                                SAF_Db *file		/* The file the data would be written to if this call was actually
                                                         * doing any writing. */                                                  
)
{
    SAF_ENTER(saf_use_written_subset_relation, SAF_PRECONDITION_ERROR);
    int         retval;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("REL must be a valid relation handle"));
    SAF_REQUIRE(SS_REL(oldrel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("OLDREL must be a valid relation handle"));

    /* Confirm relevant parts of relation records are identical */
    SAF_REQUIRE(SAF_EQUIV(SS_REL_P(rel,sup_cat),SS_REL_P(oldrel,sup_cat)), SAF_NO_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("OLDREL must be same as REL to re-use data written to it"));
    SAF_REQUIRE(SS_REL(rel)->kind==SS_REL(oldrel)->kind, SAF_NO_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("OLDREL must be same as REL to re-use data written to it"));

    /* Make the actual call to write/update the relation data */
    retval = _saf_write_subset_relation(pmode, rel, oldrel, A_buf_type, NULL, B_buf_type, NULL, file);
    SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Subset Relations
 * Purpose:	Write a subset relation	
 *
 * Description:	This call writes relation data to the specified file.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Parallel:    SAF_EACH mode is a collective call where each of the N tasks provides a unique relation. SAF will create a
 *              single HDF5 dataset to hold all the data and will create N blobs to point into nonoverlapping regions in that
 *              dataset.
 *
 * Issue:	Overwrite is not currently allowed.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_write_subset_relation(SAF_ParMode pmode,    /* The parallel mode. */
			  SAF_Rel *rel,		/* The relation whose data is to be written. */
			  hid_t A_type,         /* The type of A_BUF (if not already supplied through the
                                                 * saf_declare_subset_relation() call). */
			  void *A_buf,          /* The data (if not already supplied through the
						 * saf_declare_subset_relation() call). */
			  hid_t B_type,         /* The type of B_BUF (if not already supplied through the
						 * saf_declare_subset_relation() call. */
			  void *B_buf,          /* The data (if not already supplied through the
						 * saf_declare_subset_relation() call). */
			  SAF_Db *file		/* The optional destination file to write the data to. A null pointer for this
                                                 * argument indicates that the data is to be written to the same file as REL. */
			  )
{
    SAF_ENTER(saf_write_subset_relation, SAF_PRECONDITION_ERROR);
    double              timer_start=0.0;        /* Start time for calculating total write time. */
    int                 retval;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("REL must be a valid relation handle"));
    SAF_REQUIRE(SAF_XOR(SS_REL(rel)->m.abuf, A_buf), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("A_BUF should be specified either here or in the saf_declare_subset_relation() "
                            "call but not both"));
    SAF_REQUIRE(!B_buf || !SS_REL(rel)->m.bbuf, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("B_BUF, if present, should be specified either here or in the "
                            "saf_declare_subset_relation() call but not both"));

    if (_SAF_GLOBALS.p.TraceTimes)
        timer_start = _saf_wall_clock(FALSE);

    /* ok now make the actual call to write/update the relation data */
    retval = _saf_write_subset_relation(pmode, rel, NULL, A_type, A_buf, B_type, B_buf, file);

    if (_SAF_GLOBALS.p.TraceTimes)
        _SAF_GLOBALS.CummWriteTime += _saf_wall_clock(false) - timer_start;

    SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private	
 * Chapter:	Subset Relations
 * Purpose:	Do the actual work of subset relation writing	
 *
 * Description:	This private function does all the real work of writing relation data to the specified file.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issue:	Overwrite is not currently allowed.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_write_subset_relation(SAF_ParMode pmode,	/* The parallel mode. */
                           SAF_Rel *rel,        /* the relation being updated. */
			   SAF_Rel *oldrel,     /* Optional: The old relation record being re-used */
                           hid_t A_type,        /* The type of A_BUF if not already supplied through
                                                 * saf_declare_subset_relation(). */
                           void *A_buf,         /* The data if not already supplied through saf_declare_subset_relation() or
                                                 * via the OLDREL relation. */                                            
                           hid_t B_type,        /* The type of B_BUF if not already supplied through
						 * saf_declare_subset_relation(). */
                           void *B_buf,         /* The data if not already supplied through saf_declare_subset_relation() or
                                                 * via the OLDREL relation. */                                            
                           SAF_Db *file		/* The optional destination file to which to write the data. If this is the
                                                 * null pointer then the data will be written to the file to which REL belongs. */
                           )
{
    SAF_ENTER(_saf_write_subset_relation, SAF_PRECONDITION_ERROR);
    ss_scope_t          scope;                  /* Scope where relation dofs are to be written */
    SAF_Set             sub, sup;               /* Sub- and super-sets cached from REL for convenience */
    ss_collection_t     sub_coll=SS_COLLECTION_NULL; /* Subset collection or null if REL->sub_cat is the self category */
    ss_collection_t     sup_coll=SS_COLLECTION_NULL; /* Superset collections or null if REL->sup_cat is the self category */
    ss_pers_t           val;                    /* For error checking */
    size_t              *offsets=NULL;          /* Array of task offsets into a VL array */
    ss_pers_t           *all_subsets=NULL;      /* Array of subset handles from all the participating tasks */
    ss_pers_t           *all_supcols=NULL;      /* Array of all superset collections from the participating tasks */
    MPI_Comm            rel_comm=SS_COMM_NULL;  /* Communicator associated with the relation */
    int                 self, ntasks;           /* Rank and size of rel_comm */
    ss_scope_t          rel_scope=SS_SCOPE_NULL;/* Scope in which REL exists */
    int                 i;
    hsize_t             abuf_size;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    /* Where sould a new blob be created if we have to do that? */
    ss_pers_scope(file?(ss_pers_t*)file:(ss_pers_t*)rel, &scope);
    
    /* Prefer type and data pointers set earlier */
    if (SS_REL(rel)->m.abuf_type>0)
        A_type = SS_REL(rel)->m.abuf_type;
    if (SS_REL(rel)->m.bbuf_type>0)
        B_type = SS_REL(rel)->m.bbuf_type;
    if (SS_REL(rel)->m.abuf)
        A_buf = SS_REL(rel)->m.abuf;
    if (SS_REL(rel)->m.bbuf)
        B_buf = SS_REL(rel)->m.bbuf;

    /* B_buf cannot be present if the relation is of kind EQUAL */
    SAF_ASSERT((!B_buf && SS_REL(rel)->kind==SAF_RELKIND_EQUAL) || SS_REL(rel)->kind!=SAF_RELKIND_EQUAL,
               SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("subset must be some kind of boundary for optional B_BUF data to be present"));

    /* Cache some things for convenience */
    sub = SS_REL(rel)->sub;
    if (NULL==_saf_getCollection_set(&sub, SS_REL_P(rel,sub_cat), &sub_coll))
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("required subset collection was not available"));

    /* obtain the collection record for the range set */
    sup = SS_REL(rel)->sup;
    if (NULL==_saf_getCollection_set(&sup, SS_REL_P(rel,sup_cat), &sup_coll))
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("required superset collection was not available."));

#ifdef SSLIB_SUPPORT_PENDING
    /* Why is the following SAF_ASSERT_BEGIN commented out? [rpm 2004-05-24] */
#endif /*SSLIB_SUPPORT_PENDING*/
#if 0
    /* the following is error checking code only and is purely to catch the second case noted above. */
    SAF_ASSERT_BEGIN(SAF_HIGH_CHK_COST) {
        DSL_Boolean_t rangeOK = true, domainOK = true;

        if ((sub_collr.cell_type == SAF_CELLTYPE_SET) and (sub_collr.count > 1)) {
	    int i;
	    VBT_Id_t *member_ids;

	    /* obtain the range collection's metablob of member ids */
            member_ids = (VBT_Id_t *) malloc(sup_collr.count*sizeof(VBT_Id_t));
            if (member_ids == NULL)
                SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable to allocate temporary memory for member ids"));

            if (VBT_readDataFrom_metablob(vbtfile,pmode,DSL_OFFSET,
				          sup_collr.members_blob_id,0,sup_collr.count,member_ids)!=VBT_STATUS_OK)
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("unable to read members metablob %d",sup_collr.members_blob_id));

	    /* scan the range collection metablob */
	    for (i = 0; i < sup_collr.count; i++) {
                if (member_ids[i] == SAF_INVALID_ID)
                    rangeOK = false;
	    }
            _saf_free(member_ids);
	    
	    /* obtain the domain collection's metablob of member ids */
            member_ids = (VBT_Id_t *) malloc(sub_collr.count*sizeof(VBT_Id_t));
            if (member_ids == NULL)
                SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable to allocate temporary memory for member ids"));

            if (VBT_readDataFrom_metablob(vbtfile, pmode, DSL_OFFSET, sub_collr.members_blob_id,
				          0, sub_collr.count, member_ids)!=VBT_STATUS_OK)
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("unable to read members metablob %d",sub_collr.members_blob_id));

	    /* scan the domain collection metablob */
	    for (i = 0; i < sub_collr.count; i++) {
                if (member_ids[i] == SAF_INVALID_ID)
                    domainOK = false;
	    }
            _saf_free(member_ids);
        }

    } SAF_ASSERT_END(rangeOK && domainOK, SAF_CONSTRAINT_ERROR,
                     _saf_errmsg("attempted to define subset relation between incompletely defined non-primitive collections"));
#endif

    /* For the special case in which the collection on the containing set of the domain is of size one AND is also a
     * decomposition, we put the set links of the domain into that collection on the range, at the location specified by the
     * single value in rel.abuf.
     *   
     * Actually, there is another special case here too. That is the case where a subset relation is defined on a non-primitive
     * collection. This means that the collection contains sets, not cells. In this case, those set ids that are in the
     * collection on the range must also already exist in the collection on the domain. Otherwise, we throw an error. */
    if (SS_COLLECTION(&sub_coll)->count<=1 &&
        SS_COLLECTION(&sub_coll)->cell_type==SAF_CELLTYPE_SET &&
        SS_COLLECTION(&sub_coll)->is_decomp) {
        int A_buf_int;
        if (NULL==_saf_convert(A_type, A_buf, H5T_NATIVE_INT, &A_buf_int))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to convert A_buf to an integer"));

        /* For subset relations defined on the reserved "self" collection, extend the collection, if necessary, so that it is
	 * large enough for the currently specified member index */
        if (_saf_is_self_decomp(SS_REL_P(rel,sup_cat))) {
            if (A_buf_int >= SS_COLLECTION(&sup_coll)->count) {
                int addCount = A_buf_int - SS_COLLECTION(&sup_coll)->count + 1;
                if (saf_extend_collection(pmode, &sup, SS_REL_P(rel,sup_cat), addCount, SAF_1DC(addCount)) != SAF_SUCCESS)
                    SAF_ERROR(SAF_WRITE_ERROR,_saf_errmsg("unable to extend the self collection on set \"%s\"",
                                                          ss_string_ptr(SS_SET_P(&sup,name))));
            }
        } else {
            if (A_buf_int!=0 && SS_COLLECTION(&sup_coll)->count!=0 && A_buf_int>=SS_COLLECTION(&sup_coll)->count)
                SAF_ERROR(SAF_CONSTRAINT_ERROR,_saf_errmsg("specified member index is larger than collection"););
        }
         
        /* As an extra check, we read the metablob value at this point and, if it is NOT set, allow the update or if it is set
         * to the value being added, allow it, else, abort with error. This is a totally optional errorcheck */
        if (NULL==ss_array_get(SS_COLLECTION_P(&sup_coll,members), ss_pers_tm, (size_t)A_buf_int, 1, &val))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to read member link sup_coll.members[%d]", A_buf_int));
        if (!SS_PERS_ISNULL(&val) && !SS_PERS_EQ(&val, SS_REL_P(rel,sub)))
            SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("overwriting sup_coll.members[%d] with a different set", A_buf_int));
        if (B_buf)
            SAF_ERROR(SAF_CONSTRAINT_ERROR,_saf_errmsg("for a \"simple\" subset relation, B-buf must be null"));

        /* Now, add the set to the members array at the correct position.  If each task added its own data to the
         * sup_coll->members array we would end up with a situation where the synchronization algorithms think that the tasks
         * made incompatible changes to the array.  In order to avoid that we must ensure that every task modifies the array
         * in the same manner. We're making use of the fact that a SAF_EACH mode call is still a collective call. [rpm
         * 2004-08-23] */
        SAF_DIRTY(&sup_coll, pmode);
        ss_pers_scope((ss_pers_t*)rel, &rel_scope);
        ss_scope_comm(&rel_scope, &rel_comm, &self, &ntasks);
        offsets = malloc(ntasks*sizeof(*offsets));
        offsets[self] = A_buf_int;
        ss_mpi_allgather(offsets, 1, MPI_SIZE_T, rel_comm);
        all_subsets = saf_allgather_handles((ss_pers_t*)SS_REL_P(rel, sub), NULL, NULL);
        all_supcols = saf_allgather_handles((ss_pers_t*)&sup_coll, NULL, NULL);
        for (i=0; i<ntasks; i++) {
            if (ss_array_put(SS_COLLECTION_P(all_supcols+i,members), ss_pers_tm, offsets[i], (size_t)1, all_subsets+i)<0)
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("unable to write to sup_coll.members[%d]", A_buf_int));
        }
        offsets = SS_FREE(offsets);
        all_subsets = SS_FREE(all_subsets);
        all_supcols = SS_FREE(all_supcols);
    } else {
        /* Just use regular 'ole blobs */

        /* Disallow overwrites to subset relations */
        if (!SS_PERS_ISNULL(SS_REL_P(rel,r_blob)))
            SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("cannot overwrite a subset relation"));

        SAF_DIRTY(rel,pmode);

        /* Create a blob and write data to it. If some tasks are reusing data from an existing relation we still need to
         * participate in these calls for collectivity's sake. */
        abuf_size = oldrel ? 0 : SS_REL(rel)->m.abuf_size;
        if (oldrel) {
            assert(NULL==A_buf);
            A_buf = (void*)1; /*so ss_blob_bind_m1() binds instead of unbinds*/
        }
        if (NULL==ss_blob_new(&scope, SAF_ALL==pmode?SS_ALLSAME:0U, SS_REL_P(rel,r_blob)))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create range blob"));
        if (ss_blob_bind_m1(SS_REL_P(rel,r_blob), A_buf, A_type, abuf_size)<0)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot bind memory buffer to range blob"));
        if (ss_blob_mkstorage(SS_REL_P(rel,r_blob), NULL, SAF_ALL==pmode?SS_ALLSAME:SS_BLOB_EACH, NULL)<0)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create range blob dataset"));
        if (ss_blob_write1(SS_REL_P(rel,r_blob), (hsize_t)0, abuf_size,
                           SS_BLOB_UNBIND|SS_BLOB_COLLECTIVE|(SAF_ALL==pmode?SS_ALLSAME:0U), NULL)<0)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot write to range blob"));
        
        if (oldrel)
            SS_REL(rel)->r_blob = SS_REL(oldrel)->r_blob;

        /* now, write the domain blob (d_blob) for the B_buf data, if present */
        if (B_buf) {
            assert(!oldrel);

            /* disallow overwrites to subset relations */
            if (!SS_PERS_ISNULL(SS_REL_P(rel,d_blob)))
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("cannot overwrite a subset relation"));
            SAF_DIRTY(rel,pmode);

            /* Create a blob and write data to it. */
            if (NULL==ss_blob_new(&scope, SAF_ALL==pmode?SS_ALLSAME:0U, SS_REL_P(rel,d_blob)))
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create domain blob"));
            if (ss_blob_bind_m1(SS_REL_P(rel,d_blob), B_buf, B_type, (hsize_t)SS_REL(rel)->m.bbuf_size)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot bind memory to domain blob"));
            if (ss_blob_mkstorage(SS_REL_P(rel,d_blob), NULL, SAF_ALL==pmode?SS_ALLSAME:SS_BLOB_EACH, NULL)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create domain blob dataset"));
            if (ss_blob_write1(SS_REL_P(rel,d_blob), (hsize_t)0, (hsize_t)SS_REL(rel)->m.bbuf_size,
                               SS_BLOB_UNBIND|SS_BLOB_COLLECTIVE|(SAF_ALL==pmode?SS_ALLSAME:0U), NULL)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot write to domain blob"));
        }
    }
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Subset Relations
 * Purpose:     Get datatype and size for a subset relation
 *
 * Description: This function is used to retrieve the number and type of A-buffer and B-buffer data items that would be retrieved
 *              by a call to the saf_read_subset_relation() function.  This function may be used by the caller to determine
 *              the sizes of the buffers needed when pre-allocation is desired or to determine how to traverse the buffer(s)
 *              returned by the saf_read_subset_relation() function.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Jim Reus, LLNL, 2000-09-26
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_count_and_type_for_subset_relation(SAF_ParMode pmode,           /* The parallel mode. */
					   SAF_Rel *rel,                /* The relation handle. */
                                           SAF_RelTarget *target,       /* Optional relation targeting information. */
					   size_t *abuf_sz,             /* OUT: The number of items that would be placed in
                                                                         * the A-buffer by a call to the
                                                                         * saf_read_subset_relation() function.  The caller
                                                                         * may pass value of NULL for this parameter if this
                                                                         * value is not desired. */
					   hid_t *abuf_type,            /* OUT: The type of the items that would be placed in
                                                                         * the A-buffer by a call to the
                                                                         * saf_read_subset_relation() function.  The caller
                                                                         * may pass value of NULL for this parameter if this
                                                                         * value is not desired. */
					   size_t *bbuf_sz,             /* OUT: The number of items that would be placed in
                                                                         * the B-buffer by a call to the
                                                                         * saf_read_subset_relation() function.  The caller
                                                                         * may pass value of NULL for this parameter if this
                                                                         * value is not desired. */
					   hid_t *bbuf_type             /* OUT: The type of the items that would be placed in
                                                                         * the B-buffer by a call to the
                                                                         * saf_read_subset_relation() function.  The caller
                                                                         * may pass value of NULL for this parameter if this
                                                                         * value is not desired. */
                                           )
{
    SAF_ENTER(saf_get_count_and_type_for_subset_relation, SAF_PRECONDITION_ERROR);
    ss_set_t            sub;                    /* Cached subset from the relation */
    hid_t               ftype=-1;               /* File datatype for blobs */
    hsize_t             size;                   /* Size of a blob */
    ss_collection_t     sub_coll;               /* Collection on relation's subset */

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("REL must be a valid relation handle for all participating processes"));

    /* ISSUE: Relation targeting is not yet implemented. */
    SAF_ASSERT(!target, SAF_LOW_CHK_COST, SAF_NOTIMPL_ERROR, _saf_errmsg("Relation targeting is not yet implemented"));

    /* Cache some stuff for convenience */
    sub = SS_REL(rel)->sub;
    if (NULL==_saf_getCollection_set(&sub, SS_REL_P(rel,sub_cat), &sub_coll))
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("required subset collection was not available"));

    /* If the collection is of size 1 and the type is SET and represents part of a decomposition then this is the special case
     * where we store the relation data in the collection. */
    if (SS_COLLECTION(&sub_coll)->count==1 &&
        SS_COLLECTION(&sub_coll)->cell_type==SAF_CELLTYPE_SET &&
        SS_COLLECTION(&sub_coll)->is_decomp) {
        if (abuf_sz) *abuf_sz   = 1;
        if (abuf_type) *abuf_type = H5Tcopy(H5T_NATIVE_SIZE);
    } else {
	/*  First, dealing with the range to be stored in abuf (indirect_rels or r_blob). There is always an A-buffer... */
        if (ss_array_nelmts(SS_REL_P(rel,indirect_rels))>0) {
            if (abuf_sz) *abuf_sz = ss_array_nelmts(SS_REL_P(rel,indirect_rels));
            if (abuf_type) *abuf_type = H5Tcopy(ss_pers_tm);
        } else if (!SS_PERS_ISNULL(SS_REL_P(rel,r_blob))) {
            ss_blob_bound_f1(SS_REL_P(rel,r_blob), NULL, NULL, &size, &ftype);
            if (abuf_type) *abuf_type = H5Tget_native_type(ftype, H5T_DIR_DEFAULT);
            H5Tclose(ftype); ftype=-1;
            if (abuf_sz) *abuf_sz = size;
        } else {
            if (abuf_sz) *abuf_sz = 0;
            if (bbuf_sz) *bbuf_sz = 0;
            if (abuf_type) *abuf_type = H5I_INVALID_HID;
            if (bbuf_type) *bbuf_type = H5I_INVALID_HID;
            return SAF_SUCCESS;
        }

	/* Now to get the info for the B-buffer (domain blob).  Note that there may have been no B-buffer. */
        if (SS_PERS_ISNULL(SS_REL_P(rel,d_blob))) {
            if (bbuf_sz) *bbuf_sz = 0;
	    if (bbuf_type) *bbuf_type = H5I_INVALID_HID;
        } else {
            ss_blob_bound_f1(SS_REL_P(rel,d_blob), NULL, NULL, &size, &ftype);
            if (bbuf_type) *bbuf_type = H5Tget_native_type(ftype, H5T_DIR_DEFAULT);
            H5Tclose(ftype); ftype=-1;
            if (bbuf_sz) *bbuf_sz = size;
        }
    }
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Subset Relations
 * Purpose:	Read the data for a subset relation
 *
 * Description: Read the data associated with a subset relation. Note that there is no information about the buffers passed as
 *		formal arguments to this call. Why? Because any information about the "native" buffers is known via the
 *		saf_describe_subset_relation() call. The client may "target" the data read in this call for a particular
 *		data-type, etc. by using the saf_target_subset_relation() call.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Modifications:
 * 		Robb Matzke, LLNL, 2000-02-17
 *		Added documentation.
 *
 *		Mark Miller, LLNL, Tue Mar 14, 2000
 *		Added BBUF to deal with "which" piece of boundary information (see saf_declare_subset_relation())
 *
 * 		Jim Reus, 31Jan2001
 *		- Changed some callocs to mallocs to avoid problem size init.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_read_subset_relation(SAF_ParMode pmode,	/* The parallel mode. */
			 SAF_Rel *rel,	        /* The relation whose data is to be read. */
                         SAF_RelTarget *target, /* Relation targeting information. */
			 void **abuf,	        /* The data representing those members in the range collection (on the superset)
						 * that are related to the members in the domain collection (on the subset). */
			 void **bbuf	        /* Optional data for boundary subsets indicating which local piece of boundary
                                                 * each member in the domain collection represents in each member of the
						 * range collection (see saf_declare_subset_relation()) */
                         )
{
    SAF_ENTER(saf_read_subset_relation, SAF_PRECONDITION_ERROR);
    double              timer_start=0;          /* Timer for accumulating time spent reading data. */
    ss_set_t            sub,sup;                /* Relation's subset and superset */
    ss_collection_t     sub_coll, sup_coll;     /* Relation's sub and super collections */
    size_t              width;                  /* Number of elements in a collection `members' array */
    ss_pers_t           val;                    /* Element from a collection `members' array; link to a set hopefully. */
    hsize_t             size;                   /* Number of elements in a blob */
    hid_t               ftype=-1;               /* File datatype for a blob */
    hid_t               mtype=-1;               /* Memory datatype corresponding to `ftype' */
    int                 i;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("REL must be a valid relation handle"));
    SAF_REQUIRE(abuf, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ABUF cannot be null for all participating processes"));
    SAF_ASSERT(!target, SAF_LOW_CHK_COST, SAF_NOTIMPL_ERROR,
               _saf_errmsg("Relation targeting is not implemented"));

    /* Start timer */
    if (_SAF_GLOBALS.p.TraceTimes)
        timer_start = _saf_wall_clock(false);

    /* Cache some stuff */
    sub = SS_REL(rel)->sub;
    if (NULL==_saf_getCollection_set(&sub, SS_REL_P(rel,sub_cat), &sub_coll))
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("required subset collection was not available"));
    sup = SS_REL(rel)->sup;

    /* Just as in the write case, we need to do something special for the case in which the domain collection contains just 1
     * member AND is a decomposition of its containing set.  */
    if (SS_COLLECTION(&sub_coll)->count<=1 &&
        SS_COLLECTION(&sub_coll)->cell_type==SAF_CELLTYPE_SET &&
        SS_COLLECTION(&sub_coll)->is_decomp) {
        if (NULL==_saf_getCollection_set(&sup, SS_REL_P(rel,sup_cat), &sup_coll))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("attempt to read non-existent collection members"));
        if (0==(width=ss_array_nelmts(SS_COLLECTION_P(&sup_coll,members))))
            SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("attempt to read non-existent collection members"));
        if (SS_COLLECTION(&sup_coll)->count!=(int)width)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("apparent attempt to read members array before it's been completey filled"));

        /* Scan list of members for the one pointing to the subset and return a link to it in the array */
        for (i=0; i<SS_COLLECTION(&sup_coll)->count; i++) {
            ss_array_get(SS_COLLECTION_P(&sup_coll,members), ss_pers_tm, (size_t)i, 1, &val);
            if (SS_PERS_EQ(&val,SS_REL_P(rel,sub))) {
                if (!*abuf && NULL==(*abuf=malloc(sizeof(size_t))))
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate space to return collection members"));
                *(size_t*)(*abuf) = i;
                break;
            }
        }
        if (i>=SS_COLLECTION(&sup_coll)->count)
            SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to find subset in the superset collection's member list"));
    } else {
        /*  First, read the abuf (range) data; it is *always* present either indirectly or in a blob. */
        if (ss_array_nelmts(SS_REL_P(rel,indirect_rels))>0) {
            *abuf = ss_array_get(SS_REL_P(rel,indirect_rels), ss_pers_tm, (size_t)0, SS_NOSIZE, *abuf);
            if (!*abuf) SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("problems reading indirect relation pointers"));
            mtype = H5I_INVALID_HID;
        } else if (!SS_PERS_ISNULL(SS_REL_P(rel,r_blob))) {
            /* obtain the data set type. Note, it will be same for bbuf too, needed below. */
            ss_blob_bound_f1(SS_REL_P(rel,r_blob), NULL, NULL, &size, &ftype);
            mtype = H5Tget_native_type(ftype, H5T_DIR_DEFAULT);
            H5Tclose(ftype); ftype=-1;

            /* allocate space for the abuf data or check size */
            if (!*abuf && NULL==(*abuf = malloc((size_t)size*H5Tget_size(mtype))))
                SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate space to read abuf of relation"));

            /* read the data */
            ss_blob_bind_m1(SS_REL_P(rel,r_blob), *abuf, mtype, size);
            ss_blob_read1(SS_REL_P(rel,r_blob), (hsize_t)0, size, SS_BLOB_UNBIND, NULL);
        } else {
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("attempt to read non-existent range data"));
        }

        /* Read the bbuf (domain) data */
        if (bbuf) {
            /* Issue: If the client requests BBUF but none was written, is that an error? Unfortunately, the only answer that
             *        works in all cases is to declare this an error. This is so because it is not possible to notify the client
             *        that none was written except by returning *BBUF==NULL and that is *not* possible in the case that the client
             *        has pre-allocated BBUF (except if we opt to free the pre-allocated BBUF, and then set it to NULL which I
             *        don't think would be a good idea). We limit returning error to *only* this case. The other case returns
             *        BBUF==NULL */
	    if (SS_PERS_ISNULL(SS_REL_P(rel,d_blob))) {
                if (*bbuf)
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("attempt to read non-existent bbuf blob into pre-allocated memory"));
	    } else {
                /* allocate space for the bbuf data or check size */
                ss_blob_bound_f1(SS_REL_P(rel,d_blob), NULL, NULL, &size, NULL);
                if (!*bbuf && NULL==(*bbuf=malloc((size_t)size*H5Tget_size(mtype))))
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate space to read bbuf of relation"));

                /* read the data */
                ss_blob_bind_m1(SS_REL_P(rel,d_blob), *bbuf, mtype, size);
                ss_blob_read1(SS_REL_P(rel,d_blob), (hsize_t)0, size, SS_BLOB_UNBIND, NULL);
	    }
        }
    }

    if (mtype>0) H5Tclose(mtype);
    if (_SAF_GLOBALS.p.TraceTimes)
        _SAF_GLOBALS.CummReadTime += _saf_wall_clock(FALSE) - timer_start;

    SAF_LEAVE(SAF_SUCCESS);

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Topology Relations
 * Purpose:	Declare a topological relation
 *
 * Description:	A topology relation describes how the individual members of a collection are sewn together to form a mesh. A
 *		topology relation is composed of one or more steps. Each step in the relation represents a portion of the
 *		dimensional cascade in representing an N dimensional set in terms of a bunch of N-1 dimensional sets that form
 *		its boundary, which are, in turn, represented by a bunch of N-2 dimensional sets, etc. The last step is
 *		always on zero dimensional sets (e.g., SAF_CELLTYPE_POINT cells). Typically, there is only ever one step from a
 *              primitive decomposition to SAF_CELLTYPE_POINT cells (nodes). In this case, the topology relation is a list
 *              describing the nodal connectivity for each element in the decomposition. 
 *
 *		In the case of SAF_STRUCTURED, all other arguments are currently ignored and rectangular structure is assumed.
 *		Later, different types of structure will be supported. In the case of SAF_UNSTRUCTURED, ABUF is a pointer to
 *		one value of type DATA_TYPE representing the number of range references for each member of the domain
 *		collection and BBUF is an array of type DATA_TYPE containing that number of range references for each member
 *              of the domain. In the case of SAF_ARBITRARY, ABUF is a pointer to an array of values of type DATA_TYPE equal
 *              to the size of the domain collection. Each value in the ABUF array represents the number of range references
 *              for the corresponding number of the domain collection. BBUF is a pointer to the range references.
 *
 *		By convention, a topology relation should be declared on the maximal set in the subset inclusion lattice (e.g.,
 *		the top-most set in the subset inclusion lattice) for which it makes sense to define the topology. If the
 *		topology relation is, in fact, stored in non-contiguous chunks, then the client should use the STORAGE_DECOMP
 *		argument of the topology relation to declare that the relation data is stored in pieces on the given
 *		decomposition.
 *
 * Return:      On success, returns a pointer to the new relation: either the REL argument or an allocated relation. Returns
 *              the null pointer on failure.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Rel *
saf_declare_topo_relation(SAF_ParMode pmode,		/* The parallel mode. */
                          SAF_Db *db,                   /* The dataset where the new relation will be created. */
			  SAF_Set *set,		        /* The containing set of the collection whose members are being sewn
							 * together by the relation. */
			  SAF_Cat *pieces,	        /* The collection of members that are being sewn together. */
			  SAF_Set *range_set,
			  SAF_Cat *range_cat, 	        /* Together, RANGE_SET and RANGE_CAT identify the range of the relation
                                                         * (e.g., collection used to glue the pieces together). There are
                                                         * really only two valid values for RANGES_S: the set SET or the set
							 * MY_PIECE. */
			  SAF_Cat *storage_decomp,      /* The decomposition of SET upon which the relation is stored. */
			  SAF_Set *my_piece,	        /* The piece of the decomposition being declared here. */
			  SAF_RelRep *trtype,           /* The relation types. One of SAF_STRUCTURED, SAF_UNSTRUCTURED, or
							 * SAF_ARBITRARY. */
                          hid_t A_type,                 /* The type of the data in A_BUF. */
                          void *A_buf,                  /* The buffer. Pass NULL if you would rather provide this in
                                                         * the write call. */
                          hid_t B_type,                 /* The type of the data in B_BUF. */
                          void *B_buf,                  /* The buffer. Pass NULL if you would rather provide this in
                                                         * the write call. */
			  SAF_Rel *rel		        /* OUT: Optional memory that will be initialized (and returned) to
                                                         * point to the new relation. */
			  )
{
    SAF_ENTER(saf_declare_topo_relation, NULL);
    ss_scope_t          scope;                  /* The scope where the new relation will be created. */
    int                 i, sum=0, tmp;
    ss_collection_t     pieces_coll;            /* Collection for PIECES category on SET */
    ss_collection_t     range_coll;             /* Collection for RANGE_CAT on RANGE_SET */
    ss_collection_t     storage_decomp_coll;    /* Collection for STORAGE_DECOMP on SET */

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);
    ss_file_topscope(db, &scope);

    SAF_REQUIRE(SS_SET(set), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("SET must be a valid set handle"));
    SAF_REQUIRE(SS_CAT(pieces), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PIECES must be a valid category"));
    SAF_REQUIRE(SS_SET(range_set), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("RANGE_SET must be a valid set"));
    SAF_REQUIRE(SS_CAT(range_cat), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("RANGE_CAT must be a valid category"));
    SAF_REQUIRE(SS_SET(my_piece), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("MY_PIECES must be a valid handle"));
    SAF_REQUIRE(SS_CAT(storage_decomp), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("STORAGE_DECOMP must be either the self decomposition or a valid cat handle"));
    SAF_REQUIRE(SS_RELREP(trtype), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("TRTYPE must be a consistent relation representation handle"));
    SAF_REQUIRE(SAF_STRUCTURED_ID==SS_RELREP(trtype)->id || SAF_UNSTRUCTURED_ID==SS_RELREP(trtype)->id ||
                SAF_ARBITRARY_ID==SS_RELREP(trtype)->id, 
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("TRTYPE must be a valid topology representation"));
    SAF_REQUIRE(A_type<=0 || H5T_INTEGER==H5Tget_class(A_type) ||
                (H5Tequal(A_type,ss_pers_tm) && _saf_is_self_decomp(storage_decomp)),
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("If supplied, A_TYPE must be an integer type (or SAF_HANDLE if decomposed)"));
    SAF_REQUIRE(A_type>0 || !A_buf, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("A_TYPE must be supplied if A_BUF is supplied"));
    SAF_REQUIRE(B_type<=0 || H5T_INTEGER==H5Tget_class(B_type) || H5Tequal(B_type, ss_pers_tm),
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("B_TYPE must be an integer type or handle type"));
    SAF_REQUIRE(B_type || !B_buf, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("B_TYPE must be supplied if B_BUF if supplied"));

    /* The my_piece parameter */
    SAF_REQUIRE(A_type<=0 || SAF_XOR(_saf_is_self_decomp(storage_decomp), H5Tequal(A_type, ss_pers_tm)), 
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("A_TYPE must be handle if storage decomposition is not self "
                            "but must not be handle if storage decomposition is self"));

    /* confirm pieces cat exists on set */
    if (NULL==_saf_getCollection_set(set, pieces, &pieces_coll))
	SAF_ERROR(NULL, _saf_errmsg("collection \"%s\" not found on set \"%s\"",
                                    ss_string_ptr(SS_CAT_P(pieces,name)), ss_string_ptr(SS_SET_P(set,name))));

    /* Confirm pieces is a primitive collection */
    if (SS_COLLECTION(&pieces_coll)->cell_type == SAF_CELLTYPE_SET)
	SAF_ERROR(NULL, _saf_errmsg("SAF currently only supports topology relations on primitive collections"));

    switch(SS_SET(set)->tdim) {
    case 0:
        if (SS_COLLECTION(&pieces_coll)->cell_type != SAF_CELLTYPE_POINT)
            SAF_ERROR(NULL, _saf_errmsg("domain set dimension not compatible with collection cell"));
        break;
    case 1:
        if (SS_COLLECTION(&pieces_coll)->cell_type != SAF_CELLTYPE_LINE)
            SAF_ERROR(NULL, _saf_errmsg("domain set dimension not compatible with collection cell"));
        break;
    case 2:
        if (SAF_STRUCTURED_ID==SS_RELREP(trtype)->id && SS_COLLECTION(&pieces_coll)->cell_type != SAF_CELLTYPE_QUAD)
            SAF_ERROR(NULL, _saf_errmsg("structured meshes declared on domain sets of dimension "
                                        "2 may only use SAF_CELLTYPE_QUAD collection cells"));
        break;
    case 3:
        if (SAF_STRUCTURED_ID==SS_RELREP(trtype)->id && SS_COLLECTION(&pieces_coll)->cell_type != SAF_CELLTYPE_HEX)
            SAF_ERROR(NULL, _saf_errmsg("structured meshes declared on domain sets of dimension "
                                        "3 may only use SAF_CELLTYPE_HEX collection cells"));
        break;
    default:
        SAF_ERROR(NULL, _saf_errmsg("domain sets of dimension > 3 not currently supported"));
    }

    /* Confirm range_cat exists on range_set set */
    if (NULL==_saf_getCollection_set(range_set, range_cat, &range_coll))
	SAF_ERROR(NULL, _saf_errmsg("collection \"%s\" not found on set \"%s\"",
                                    ss_string_ptr(SS_CAT_P(range_cat,name)), ss_string_ptr(SS_SET_P(range_set,name))));

    /* Build a relation record */
    rel = (ss_rel_t*)ss_pers_new(&scope, SS_MAGIC(ss_rel_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)rel, NULL);
    if (!rel) SAF_ERROR(NULL, _saf_errmsg("unable to create or initialize the new relation object and/or handle."));
    if (SAF_EACH==pmode) SS_PERS_UNIQUE(rel);
    SS_REL(rel)->sub = *set;
    SS_REL(rel)->sub_cat = *pieces;
    SS_REL(rel)->sub_decomp_cat = *storage_decomp;
    SS_REL(rel)->sup = *range_set;
    SS_REL(rel)->sup_cat = *range_cat;
    SS_REL(rel)->sup_decomp_cat = *storage_decomp; 
    SS_REL(rel)->kind = SAF_RELKIND_SUBSET;
    SS_REL(rel)->rep_type = *trtype;

    /* Compute the size of each buffer (if any). */    
    SS_REL(rel)->m.abuf_type = A_type;
    SS_REL(rel)->m.bbuf_type = B_type;
    if (_saf_is_self_decomp(storage_decomp)) {
        /* A topological relation which is stored on self has buffers whose sizes are based on the size of the collection being
         * sewn together (given "set" and "pieces" category).
         * 
         * Note that the buffer size depends on the number of size of the collection category and the nature of the relation
         * (structured, unstructured, ...). */
        switch (SS_RELREP(trtype)->id) {
        case SAF_STRUCTURED_ID:
            SS_REL(rel)->m.abuf = NULL;
            SS_REL(rel)->m.bbuf = NULL;
            SS_REL(rel)->m.abuf_size = 0;
            SS_REL(rel)->m.bbuf_size = 0;
            break;
        case SAF_UNSTRUCTURED_ID:
            /* If the user gives a value for A_buf and A_type, it is the number of range refs per member of the pieces,
             * otherwise, we assume its 1 and catch it on the write call */
            SS_REL(rel)->m.abuf = A_buf;
            SS_REL(rel)->m.bbuf = B_buf;
            SS_REL(rel)->m.abuf_size = 1;
            SS_REL(rel)->m.bbuf_size = SS_COLLECTION(&pieces_coll)->count; 
            if (A_type>0 && A_buf) {
                _saf_convert(A_type, A_buf, H5T_NATIVE_INT, &tmp);
                SS_REL(rel)->m.bbuf_size *= tmp;
            }
            break;
        case SAF_ARBITRARY_ID:
            /* If the user gives a value for A_buf, it is an array of length equal to size of pieces from which can can
	     * calculate the size of B_buf. Otherwise, we set bbuf_size to -1 and catch it on the write call */
            SS_REL(rel)->m.abuf = A_buf;
            SS_REL(rel)->m.bbuf = B_buf;
            SS_REL(rel)->m.abuf_size = SS_COLLECTION(&pieces_coll)->count;
            if (A_type>0 && A_buf) {
                for (i=0, sum=0; i<SS_COLLECTION(&pieces_coll)->count; i++) {
                    _saf_convert(A_type, A_buf, H5T_NATIVE_INT, &tmp);
                    sum += tmp;
                }
                SS_REL(rel)->m.bbuf_size = sum; 
            } else {
                SS_REL(rel)->m.bbuf_size = SS_NOSIZE; 
            }
            break;
        default:
            SAF_ERROR(NULL,_saf_errmsg("invalide topology relation type"));
        }
    } else {
        /* A topological relation which is stored on a decomposition has only a single buffer of type SAF_HANDLE whose size is
         * based on the size of the storage collection.  At this point the issue of structured vs. unstructured vs. arbitrary
         * is not important, the buffer is simply a vector of handles. */
        if (NULL==_saf_getCollection_set(set, storage_decomp, &storage_decomp_coll))
	    SAF_ERROR(NULL, _saf_errmsg("collection \"%s\" not found on set \"%s\"",
                                        ss_string_ptr(SS_CAT_P(storage_decomp,name)), ss_string_ptr(SS_SET_P(set,name))));
    }

    SAF_LEAVE(rel);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Topology Relations
 * Purpose:	Get description of topological relation	
 *
 * Description: This function returns information about a topological relation.	
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_topo_relation(SAF_ParMode pmode,		/* The parallel mode. */
			   SAF_Rel *rel,		/* The relation to be described. */
			   SAF_Set *set,		/* OUT: The containing set of the collection that is sewn together by the
							 * relation. */
			   SAF_Cat *pieces,	        /* OUT: The collection of members that are sewn together. */
			   SAF_Set *range_set,
			   SAF_Cat *range_cat, 		/* OUT: Together the RANGE_S and RANGE_C pair identifies the collection
							 * used to glue the pieces together. */
			   SAF_Cat *storage_decomp,	/* OUT: The decomposition of SET upon which the relation is actually
                                                         * stored. */
			   SAF_RelRep *trtype,		/* OUT: The topology relation type. */
			   hid_t *data_type		/* OUT: The type of the data. */
			   )
{
    SAF_ENTER(saf_describe_topo_relation, SAF_PRECONDITION_ERROR);
    hid_t       ftype=-1;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    
    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("REL must be a valid relation handle for all participating processes"));

    /* Return each of the desired values, a desired parameter is indicated by a non-null pointer. */
    if (set) *set = SS_REL(rel)->sub;
    if (pieces) *pieces = SS_REL(rel)->sub_cat;
    if (range_set) *range_set = SS_REL(rel)->sup;
    if (range_cat) *range_cat = SS_REL(rel)->sup_cat;
    if (storage_decomp) {
        if (!_saf_is_self_decomp(SS_REL_P(rel,sup_decomp_cat)) &&
            SAF_EQUIV(SS_REL_P(rel,sup_decomp_cat), SS_REL_P(rel,sub_decomp_cat))) {
            *storage_decomp = SS_REL(rel)->sup_decomp_cat;
        } else {
            *storage_decomp = *SAF_SELF(XXX);
        }
    }
    if (trtype) *trtype = SS_REL(rel)->rep_type;
    if (data_type) {
        if (SAF_STRUCTURED_ID==SS_RELREP(SS_REL_P(rel,rep_type))->id) {
            /* In the case of structured meshes, no topo relation data is written so we just return the following values to
             * indicate this. */
            *data_type = H5I_INVALID_HID;
        } else {
            /* The mesh is not structured so the topo relation data is written to the file and we must find the information
             * about the data_type. */
            if (SS_PERS_ISNULL(SS_REL_P(rel,r_blob)))
                SAF_ERROR(SAF_CONSTRAINT_ERROR,_saf_errmsg("cannot return data_type and/or file prior to writing"));
            ss_blob_bound_f1(SS_REL_P(rel,r_blob), NULL, NULL, NULL, &ftype);
            *data_type = H5Tget_native_type(ftype, H5T_DIR_DEFAULT);
            H5Tclose(ftype); ftype=-1;
        }
    }

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Topology Relations
 * Purpose:	Find topological relations
 *
 * Description: This function will find the topological relations governing a given set. Note that if the given set
 *		is one that is the subset of where the topological relations are actually declared, this call will return
 *		that set and the topological relation(s) defined on that set.
 *
 * Issue:	What if there are multiple topological relations governing a given set which are declared on different sets?
 *		The TOPO_ANCESTOR argument needs to be of length *Pnum_rels.
 *
 *              The TOPO_ANCESTOR argument is not actually referenced or returned by this function.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_topo_relations(SAF_ParMode pmode,		/* The parallel mode. */
                        SAF_Db *db,                     /* The database in which to search for topology relations. */
			SAF_Set *set,			/* The set whose topology is sought. */
			SAF_Set *topo_ancestor,		/* OUT: In many cases, the topology for a given set is known only on some
                                                         * ancestor of the set. This return value indicates that ancestor.  If
                                                         * SAF_EQUIV() for SET and TOPO_ANCESTOR is true, then the topology
                                                         * relations found by this call are indeed those defined on the
							 * specified set. Otherwise, they are defined on the TOPO_ANCESTOR. */
			int *num,			/* For this and the succeeding argument, (see Returned Handles). */
			SAF_Rel **found			/* For this and the preceding argument, (see Returned Handles). */
			)
{
    SAF_ENTER(saf_find_topo_relations, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Rel, key, mask);
    size_t               nfound;
    ss_scope_t           scope;
   
    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SET must be a valid handle"));
#if 0 /*not currently used*/
    SAF_REQUIRE(topo_ancestor, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("TOPO_ANCESTOR must be non-null"));
#else
    if (topo_ancestor) *topo_ancestor = SS_SET_NULL;
#endif
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));

    /* fill in search criteria */
    ss_file_topscope(db, &scope);
    SAF_SEARCH(SAF_Rel, key, mask, sub, *set);
    SAF_SEARCH(SAF_Rel, key, mask, kind, SAF_RELKIND_SUBSET);

    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_rel_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, NULL, NULL);
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

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Topology Relations
 * Purpose:	Is topological relation stored on self
 *
 * Description:	This function is used by a client to test if a topology relation is stored on self.  The boolean
 *              result is returned by reference.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_is_self_stored_topo_relation(SAF_ParMode pmode,     /* The parallel mode. */
			         SAF_Rel *rel,          /* The handle of the topological relation which is to be examined. */
				 hbool_t *Presult       /* OUT: A pointer to caller supplied memory which is to receive the
                                                         * result of the test: true if the relation is self stored or false if
                                                         * it is stored on a decomposition.  Note that it is permitted for the
                                                         * caller to pass a value of NULL for this parameter. */
				 )
{
    SAF_ENTER(saf_is_self_stored_topo_relation, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    if (Presult)
        *Presult = _saf_is_self_decomp(SS_REL_P(rel,sub_decomp_cat));

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Topology Relations
 * Purpose:	Set the destination form of a topological relation
 *
 * Description:	This function establishes the target (destination) form of topo relation data during either read or write.
 *		When used prior to a write call, it establishes the form of data in the file. When used prior to a read call,
 *		it establishes the form of data as desired in memory.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issues:	Not all features have been implemented yet.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_target_topo_relation(SAF_RelTarget *target,         /* OUT: Relation targeting information to be initialized by this
                                                         * function. */                                              
			 SAF_Set *range_set,            /* Optional set. */
			 SAF_Cat *range_cat,            /* Together the RANGE_SET this identifies the target collection to be
                                                         * used to glue the pieces together. Currently both of these
                                                         * parameters are ignored.  */
			 SAF_Cat *decomp,		/* The optional target decomposition. */
			 SAF_RelRep *trtype,	        /* The optional target relation types. Currently this parameter is
                                                         * ignored. */                                               
			 hid_t data_type	        /* The optional target data type. */
			)
{
   SAF_ENTER(saf_target_topo_relation, SAF_PRECONDITION_ERROR);
   SAF_REQUIRE(target, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("must pass non-null target information"));
   SAF_REQUIRE(SS_CAT(decomp), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
	       _saf_errmsg("DECOMP must be either NOT_SET, SELF_DECOMP or a valid cat handle"));

   target->is_set = TRUE;
/*    
   Sun compiler had a problem with the following three lines. noconst: vs. const
   target->range_set = range_set ? *range_set : SS_SET_NULL;
   target->range_cat = range_cat ? *range_cat : SS_CAT_NULL;
   target->decomp = decomp ? *decomp : SS_CAT_NULL;
*/
   target->range_set    = SS_SET_NULL;
   if (range_set)
      target->range_set = SS_SET_NULL;
   target->range_cat    = SS_CAT_NULL;
   if (range_cat)
     target->range_cat  = *range_cat;
   target->decomp       = SS_CAT_NULL;
   if (decomp)
     target->decomp     =  *decomp;
      
   target->data_type = data_type;
#ifdef SSLIB_SUPPORT_PENDING /* This shouldn't be necessary since we have the type in the SAF_RelTarget struct*/
   Ptrel->abuf_type = targ_data_type;
   Ptrel->bbuf_type = targ_data_type;
#endif /*SSLIB_SUPPORT_PENDING*/

   SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Topology Relations
 * Purpose:	Check existence and type of topo buffers
 *
 * Description:	This function is called by the saf_write_topo_relation() function
 *              in order to verify the validity of the types and existence of the
 *              A- and B-buffers.  The requirements:
 *
 *              - A the handle rel must be a valid relation handle.
 *              - If the topological relation is stored on self then...
 *                   - If the topological relation is structured then
 *                        - A_type and A_buf must both be NULL
 *                        - B_type and B_buf must both be NULL
 *                   - If the topological relation is unstructured then
 *
 *              - If the topological relation is stored on a decomposition then...
 *
 * Return:	A value of true is returned when this function succeeds,
 *		otherwise a value of false is returned.
 *
 * Programmer:  Jim Reus, LLNL, 2000-09-28
 *---------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_valid_topo_write_buffers(SAF_ParMode pmode, SAF_Rel *rel, hid_t A_type, void *A_buf, hid_t B_type, void *B_buf)
{
    SAF_ENTER(_saf_valid_topo_write_buffers, false);
    hbool_t     result=FALSE;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(FALSE);

    /* Is this topology relation stored on self? */
    if (_saf_is_self_decomp(SS_REL_P(rel,sub_decomp_cat))) {
        /* This is topology stored on self, look at the representation type to decide on which buffers are even
         * appropriate... */
        switch (SS_RELREP(SS_REL_P(rel,rep_type))->id) {
        case SAF_STRUCTURED_ID:
            /* No buffers are applicable. */
            if (A_type<=0 && !A_buf && B_type<=0 && !B_buf)
                result = TRUE;
            break;
        case SAF_UNSTRUCTURED_ID:
        case SAF_ARBITRARY_ID:
            /* In the case of unstructured or arbitrary relations both buffers are needed.  Both the A- and B- buffers must be
             * provided in the saf_declare_topo_relation() call or here to the saf_write_topo_relation(). */
            if (!SAF_XOR(SS_REL(rel)->m.abuf,A_buf) && (!SS_REL(rel)->m.abuf || !A_buf || SS_REL(rel)->m.abuf!=A_buf))
                goto done;
            if (!SAF_XOR(SS_REL(rel)->m.bbuf,B_buf) && (!SS_REL(rel)->m.bbuf || !B_buf || SS_REL(rel)->m.bbuf!=B_buf))
                goto done;

            /* Similarly both the A- and B-buffer types must be provided in the saf_declare_topo_relation() call or here to the
             * saf_write_topo_relation() but not in both places. */
            if (!SAF_XOR(SS_REL(rel)->m.abuf_type>0, A_type>0) &&
                (SS_REL(rel)->m.abuf_type<=0 || A_type<=0 || !H5Tequal(SS_REL(rel)->m.abuf_type,A_type)))
                goto done;
            if (!SAF_XOR(SS_REL(rel)->m.bbuf_type>0, B_type>0) &&
                (SS_REL(rel)->m.bbuf_type<=0 || B_type<=0 || !H5Tequal(SS_REL(rel)->m.bbuf_type,B_type)))
                goto done;

            /* If A-buffer is provided here then the type must be provided here. Likewise for B-buffer */
            if ((A_buf && A_type<=0) || (B_buf && B_type<=0))
                goto done;
            
            /* The A-buffer and B-buffer must be of an integral types */
            if (H5T_INTEGER!=H5Tget_class(A_type>0?A_type:SS_REL(rel)->m.abuf_type) ||
                H5T_INTEGER!=H5Tget_class(B_type>0?B_type:SS_REL(rel)->m.bbuf_type))
                goto done;

            /* Everything is OK */
            result = TRUE;
            break;
        }
    } else {
        /* This is topology stored on a decomposition: there must be an A-buffer of type SAF_HANDLE but no B-buffer.  The
         * A-buffer must be provided in the saf_declare_topo_relation() call or here to the saf_write_topo_relation() but not
         * in both places. */
        if (!SAF_XOR(SS_REL(rel)->m.abuf,A_buf))
            goto done;
        
        /* Similarly the A-buffer type must be provided in the saf_declare_topo_relation() call or here to the
         * saf_write_topo_relation() but not in both places. */
        if (!SAF_XOR(SS_REL(rel)->m.abuf_type>0, A_type>0))
            goto done;
        
        /*  No B-buffer or B-buffer type is permitted... */
        if (SS_REL(rel)->m.bbuf || B_buf || SS_REL(rel)->m.bbuf_type>0 || B_type>0)
            goto done;
        
        /* The A buffer must be of type SAF_HANDLE... */
        if (!H5Tequal(A_type>0?A_type:SS_REL(rel)->m.abuf_type, ss_pers_tm))
            goto done;
        
        /* Everything is OK */
        result = TRUE;
    }

done:
    SAF_LEAVE(result);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Topology Relations
 * Purpose:	Write topological relation data
 *
 * Description: This function writes topological relation data to the given file.	
 *
 * Parallel:    SAF_EACH mode is a collective call where each of the N tasks provides a unique relation. SAF will create a
 *              single HDF5 dataset to hold all the data and will create N blobs to point into nonoverlapping regions in that
 *              dataset.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_write_topo_relation(SAF_ParMode pmode,      /* The parallel mode. */
			SAF_Rel *rel,	        /* The relation handle. */
			hid_t A_type,           /* See saf_declare_topo_relation(). */
			void *A_buf,	        /* See saf_declare_topo_relation(). */
			hid_t B_type,           /* See saf_declare_topo_relation(). */
			void *B_buf,	        /* See saf_declare_topo_relation(). */
			SAF_Db *file	        /* The optional destination file. By default (if null) the data is written to
                                                 * the same file to which REL belongs. */
			)
{
    SAF_ENTER(saf_write_topo_relation, SAF_PRECONDITION_ERROR);
    double              timer_start=0;          /* Start time for keeping track of how long it takes to write data. */
    ss_scope_t          scope;                  /* Scope where relation's blob will be created. */
    ss_cat_t            storage_decomp;         /* The sub_decomp_cat or storage_decomp cached from REL for convenience. */
    ss_collection_t     storage_coll;           /* The collection associated with the storage_decomp category of REL. */
    size_t              bufferSize;             /* Number of elements in a buffer. */
    ss_set_t            sup;                    /* Cached superset from REL. */

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("REL must be a valid rel handle"));
    SAF_REQUIRE(A_type || !A_buf, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("A_TYPE must be supplied if A_BUF is supplied"));
    SAF_REQUIRE(_saf_valid_topo_write_buffers(pmode, rel, A_type, A_buf, B_type, B_buf),
                SAF_HIGH_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("A- and B-buffers and types must be set appropriately"));

    if (_SAF_GLOBALS.p.TraceTimes)
        timer_start = _saf_wall_clock(false);

    /* Cache some stuff for convenience */
    sup = SS_REL(rel)->sup;
    storage_decomp = SS_REL(rel)->sub_decomp_cat;

    /* Where sould a new blob be created if we have to do that? */
    ss_pers_scope(file?(ss_pers_t*)file:(ss_pers_t*)rel, &scope);

    /* Pick up the A- and B-buffer pointers and types, note that these may be found either in the relation handle (when they
     * were supplied to the saf_declare_topo_relation() function) or as parameters passed to this function. */
    if (!A_buf) A_buf = SS_REL(rel)->m.abuf;
    if (A_type<=0) A_type = SS_REL(rel)->m.abuf_type;
    if (!B_buf) B_buf = SS_REL(rel)->m.bbuf;
    if (B_type<=0) B_type = SS_REL(rel)->m.bbuf_type;

    /* If the relation is stored on a decomposition, then the category is stored as the "sub_decomp_cat", we'll use this cat */
    if (_saf_is_self_decomp(&storage_decomp)) {
	/* Compute the B-buffer size... */        
        switch (SS_RELREP(SS_REL_P(rel,rep_type))->id) {
        case SAF_STRUCTURED_ID:
            /* We assume rectangular structure and, thus, there is no data to write. */
            goto theExit;
        case SAF_UNSTRUCTURED_ID:
            /* In this case the size of the B-buffer depends on the contents of the A-buffer. If an A-buffer was provided in
             * the declare then the B-buffer size would already be known.  But if the A-buffer was deferred by the declare and
             * provided here then the B-buffer buffer size only refkects the size of the collection and must be computed with
             * the number provided as the single element of the A-buffer. */
            if (SS_REL(rel)->m.abuf) {
                bufferSize = SS_REL(rel)->m.bbuf_size;
            } else {
                int A_buf_int;
                _saf_convert(A_type, A_buf, H5T_NATIVE_INT, &A_buf_int);
                bufferSize = SS_REL(rel)->m.bbuf_size * A_buf_int;
            }

            /* Now it is time to write the B-buffer (range) contents out and record it as a blob.  First write out the B-buffer
             * data. Disallow overwrite of topology relation */
            if (!SS_PERS_ISNULL(SS_REL_P(rel,r_blob)))
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot overwrite topology relation"));

            /* Meaning of pmode...
             *   SAF_ALL:  Collective call where all tasks have identical data and therefore only one of them needs to write
             *             to the file.
             *   SAF_EACH: Collective call where the N tasks create N blobs but those blobs all point into a single common
             *             dataset where the data is stored in task rank order. */
            SAF_DIRTY(rel,pmode);
            if (NULL==ss_blob_new(&scope, SAF_ALL==pmode?SS_ALLSAME:0U, SS_REL_P(rel,r_blob)) ||
                ss_blob_bind_m1(SS_REL_P(rel,r_blob), B_buf, B_type, (hsize_t)bufferSize)<0 ||
                ss_blob_mkstorage(SS_REL_P(rel,r_blob), NULL, SAF_ALL==pmode?SS_ALLSAME:SS_BLOB_EACH, NULL)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create range blob"));
            if (ss_blob_write1(SS_REL_P(rel,r_blob), (hsize_t)0, (hsize_t)bufferSize,
                               SS_BLOB_UNBIND|SS_BLOB_COLLECTIVE|(SAF_ALL==pmode?SS_ALLSAME:0U), NULL)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot write to range blob"));
            break;
        case SAF_ARBITRARY_ID:
            /* In this case the size of the B-buffer depends on the contents of the A-buffer.  If an A-buffer was provided in
             * the declare then the B-buffer size would already be known.  But the A-buffer was not provided in the declare so
             * we'll now compute the size of the B-buffer by summing the contents of the A-buffer.  Note that the A-buffer
             * "size" was known (and is the collection size). */
            if (SS_REL(rel)->m.bbuf_size!=SS_NOSIZE) {
                bufferSize = SS_REL(rel)->m.bbuf_size;
            } else {
                int A_buf_int;
                size_t i, A_type_size=H5Tget_size(A_type);
                for (i=0, bufferSize=0; i<SS_REL(rel)->m.abuf_size; i++) {
                    _saf_convert(A_type, (char*)A_buf+i*A_type_size, H5T_NATIVE_INT, &A_buf_int);
                    bufferSize += A_buf_int;
                }
            }

            /* First write out the A-buffer data. Disallow overwrite of topology relation. */
            if (!SS_PERS_ISNULL(SS_REL_P(rel,d_blob)))
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("cannot overwrite topology relation"));
            /* Meaning of pmode...
             *   SAF_ALL:  Collective call where all tasks have identical data and therefore only one of them needs to write
             *             to the file.
             *   SAF_EACH: Collective call where the N tasks create N blobs but those blobs all point into a single common
             *             dataset where the data is stored in task rank order. */
            SAF_DIRTY(rel,pmode);
            if (NULL==ss_blob_new(&scope, SAF_ALL==pmode?SS_ALLSAME:0U, SS_REL_P(rel,d_blob)))
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create domain blob"));
            if (ss_blob_bind_m1(SS_REL_P(rel,d_blob), A_buf, A_type, (hsize_t)SS_REL(rel)->m.abuf_size)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot bind memory to domain blob"));
            if (ss_blob_mkstorage(SS_REL_P(rel,d_blob), NULL, SAF_ALL==pmode?SS_ALLSAME:SS_BLOB_EACH, NULL)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create domain blob dataset"));
            if (ss_blob_write1(SS_REL_P(rel,d_blob), (hsize_t)0, (hsize_t)SS_REL(rel)->m.abuf_size,
                               SS_BLOB_UNBIND|SS_BLOB_COLLECTIVE|(SAF_ALL==pmode?SS_ALLSAME:0U), NULL)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot write to domain blob"));

            /* Now write out the B-buffer data. Disallow overwrite of topology relation. */
            if (!SS_PERS_ISNULL(SS_REL_P(rel,r_blob)))
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("cannot overwrite topology relation"));
            /* Meaning of pmode...
             *   SAF_ALL:  Collective call where all tasks have identical data and therefore only one of them needs to write
             *             to the file.
             *   SAF_EACH: Collective call where the N tasks create N blobs but those blobs all point into a single common
             *             dataset where the data is stored in task rank order. */
            SAF_DIRTY(rel,pmode);
            if (NULL==ss_blob_new(&scope, SAF_ALL==pmode?SS_ALLSAME:0U, SS_REL_P(rel,r_blob)))
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create range blob"));
            if (ss_blob_bind_m1(SS_REL_P(rel,r_blob), B_buf, B_type, (hsize_t)bufferSize)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot bind memory to range blob"));
            if (ss_blob_mkstorage(SS_REL_P(rel,r_blob), NULL, SAF_ALL==pmode?SS_ALLSAME:SS_BLOB_EACH, NULL)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create range blob dataset"));
            if (ss_blob_write1(SS_REL_P(rel,r_blob), (hsize_t)0, (hsize_t)bufferSize,
                               SS_BLOB_UNBIND|SS_BLOB_COLLECTIVE|(SAF_ALL==pmode?SS_ALLSAME:0U), NULL)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot write to range blob"));
            break;
        default:
            SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("invalid topological relation rep-type in database"));
        }
    } else {
	/* The relation is stored on a decomposition. The caller should have passed a single buffer (A-buffer) of handles. */
        if (NULL==_saf_getCollection_set(&sup, &storage_decomp, &storage_coll))
	    SAF_ERROR(SAF_CONSTRAINT_ERROR,
                      _saf_errmsg("collection \"%s\" not found on set \"%s\"",
                                  ss_string_ptr(SS_CAT_P(&storage_decomp,name)), ss_string_ptr(SS_SET_P(&sup,name))));
        bufferSize = SS_COLLECTION(&storage_coll)->count;
        
        /* Disallow overwrites of topology */
        if (ss_array_nelmts(SS_REL_P(rel,indirect_rels))>0 || !SS_PERS_ISNULL(SS_REL_P(rel,d_blob)))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot overwrite topology relation"));

        /* Copy relation links from A_buf into the indirect_rels variable length array. */
#ifdef SSLIB_SUPPORT_PENDING
        /* Does SAF_EACH mode mean that each task is providing data to be stored in task rank order? */
#endif /*SSLIB_SUPPORT_PENDING*/
        SAF_DIRTY(rel, pmode);
        ss_array_resize(SS_REL_P(rel,indirect_rels), bufferSize);
        ss_array_put(SS_REL_P(rel,indirect_rels), ss_pers_tm, (size_t)0, bufferSize, A_buf);
    }

theExit:

    if (_SAF_GLOBALS.p.TraceTimes)
        _SAF_GLOBALS.CummWriteTime += (_saf_wall_clock(false) - timer_start);

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Topology Relations
 * Purpose:	Get datatype and size for a topological relation
 *
 * Description: This function is used to retrieve the number and type of A-buffer and B-buffer data items that would be retrieved
 *              by a call to the saf_read_topo_relation() function.  This function may be used by the caller to determine
 *              the sizes of the buffers needed when pre-allocation is desired or to determine how to traverse the buffer(s)
 *              returned by the saf_read_topo_relation() function.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_count_and_type_for_topo_relation(SAF_ParMode pmode,     /* The parallel mode. */
					 SAF_Rel *rel,          /* The relation handle. */
                                         SAF_RelTarget *target, /* Targeting information. */
					 SAF_RelRep *PrepType,  /* OUT: The mapping representation type (arbitrary, structured, or
                                                                 * unstructured). The caller may pass value of NULL for this
                                                                 * parameter if this value is not desired. */
					 size_t *abuf_sz,       /* OUT: The number of items that would be placed in the A-buffer by
                                                                 * a call to the saf_read_topo_relation() function.  The caller
                                                                 * may pass value of NULL for this parameter if this value is not
                                                                 * desired. */
					 hid_t *abuf_type,      /* OUT: The type of the items that would be placed in the
                                                                 * A-buffer by a call to the saf_read_topo_relation()
                                                                 * function.  The caller may pass value of NULL for this
                                                                 * parameter if this value is not desired. */
					 size_t *bbuf_sz,       /* OUT: The number of items that would be placed in the B-buffer by
                                                                 * a call to the saf_read_topo_relation() function.  The caller
                                                                 * may pass value of NULL for this parameter if this value is not
                                                                 * desired. */
					 hid_t *bbuf_type       /* OUT: The type of the items that would be placed in the
                                                                 * B-buffer by a call to the saf_read_topo_relation()
                                                                 * function.  The caller may pass value of NULL for this
                                                                 * parameter if this value is not desired. */
                                         )
{
    SAF_ENTER(saf_get_count_and_type_for_topo_relation, SAF_PRECONDITION_ERROR);
    static SAF_RelTarget        rt_zero;                /* Default targeting */
    ss_scope_t                  scope=SS_SCOPE_NULL;    /* Scope containing REL */
    int                         scope_size;             /* Size of the communicator for `scope' */
    size_t                      maxFactor=0;
    size_t                      minFactor=0;
    ss_set_t                    sup=SS_SET_NULL;
    ss_cat_t                    storage_decomp=SS_CAT_NULL;
    ss_relrep_t                 relrep=SS_RELREP_NULL;
    ss_blob_t                   rblob=SS_BLOB_NULL;
    ss_blob_t                   dblob=SS_BLOB_NULL;
    hsize_t                     r_size, d_size;         /* Sizes of range and domain blobs in elements */
    hid_t                       mtype=-1, ftype=-1;     /* Memory and file datatypes */
    hbool_t                     desireHandles, haveFactor;
    ss_collection_t             storage_coll=SS_COLLECTION_NULL;
    size_t                      IAcount, IBcount, d;
    hid_t                       IAtype=-1, IBtype=-1;
    void                        *IAbuf=NULL, *IBbuf=NULL;
    ss_relrep_t                 haveRelRep=SS_RELREP_NULL;
    ss_rel_t                    *theRels=NULL;
    int                         collectionSize;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    ss_pers_scope((ss_pers_t*)rel, &scope);
    ss_scope_comm(&scope, NULL, NULL, &scope_size);
    if (!target) target = &rt_zero;

    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("REL must be a valid relation handle"));
    SAF_REQUIRE((target->is_set && (SS_PERS_ISNULL(&target->decomp) || pmode==SAF_ALL || 1==scope_size)) || !target->is_set,
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if targeting of storage decomposition is used, the read must be a SAF_ALL mode read "
                            "or the database must be opened on only a single processor"));

    /* Check that data has actually been written to this relation and if not, return either what is known about it in the
     * relation handle itself or no information. */
    if (SS_PERS_ISNULL(SS_REL_P(rel,r_blob)) && SS_PERS_ISNULL(SS_REL_P(rel,d_blob)) &&
        0==ss_array_nelmts(SS_REL_P(rel,indirect_rels))) {
        if (abuf_sz)
            *abuf_sz = SS_NOSIZE==SS_REL(rel)->m.abuf_size ? 0 : SS_REL(rel)->m.abuf_size;
        if (bbuf_sz)
            *bbuf_sz = SS_NOSIZE==SS_REL(rel)->m.bbuf_size ? 0 : SS_REL(rel)->m.bbuf_size;
        if (abuf_type)
            *abuf_type = SS_REL(rel)->m.abuf_type>0 ? H5Tcopy(SS_REL(rel)->m.abuf_type) : H5I_INVALID_HID;
        if (bbuf_type)
            *bbuf_type = SS_REL(rel)->m.bbuf_type>0 ? H5Tcopy(SS_REL(rel)->m.bbuf_type) : H5I_INVALID_HID;
        goto theExit;
    }

    /* Cache some things for convenience */
    sup = SS_REL(rel)->sup;
    storage_decomp = SS_REL(rel)->sub_decomp_cat;
    relrep = SS_REL(rel)->rep_type;
    rblob = SS_REL(rel)->r_blob;
    dblob = SS_REL(rel)->d_blob;


    /* A topological relation that is stored on self is treated quite a bit differently that one that is stored on a
     * decomposition... */
    if (_saf_is_self_decomp(&storage_decomp)) {
        /*  Quick check for structured topology (nothing to read)... */            
        if (SAF_STRUCTURED_ID==SS_RELREP(&relrep)->id) {
            if (PrepType) *PrepType = *SAF_STRUCTURED;
            if (abuf_sz) *abuf_sz = 0;
            if (abuf_type) *abuf_type = H5I_INVALID_HID;;
            if (bbuf_sz) *bbuf_sz = 0;
            if (bbuf_type) *bbuf_type = H5I_INVALID_HID;
            goto theExit;
        }

        ss_blob_bound_f1(&rblob, NULL, NULL, &r_size, &ftype);
        mtype = H5Tget_native_type(ftype, H5T_DIR_DEFAULT);
        H5Tclose(ftype); ftype=-1;
        
        /* The nature of the A- and B-buffers depends on the rep type. */            
        switch (SS_RELREP(&relrep)->id) {
        case SAF_UNSTRUCTURED_ID:
            if (PrepType) *PrepType = *SAF_UNSTRUCTURED;
            if (abuf_sz) *abuf_sz = 1;
            if (abuf_type) *abuf_type = H5Tcopy(mtype);
            if (bbuf_sz) *bbuf_sz = r_size;
            if (bbuf_type) *bbuf_type = H5Tcopy(mtype);
            break;
        case SAF_ARBITRARY_ID:
            /* In this case, we need to get the blob record for the domain of the relation too */
            ss_blob_bound_f1(&dblob, NULL, NULL, &d_size, NULL);

            if (PrepType) *PrepType = *SAF_ARBITRARY;
            if (abuf_sz) *abuf_sz = d_size;
            if (abuf_type) *abuf_type = H5Tcopy(mtype);
            if (bbuf_sz) *bbuf_sz = r_size;
            if (bbuf_type) *bbuf_type = H5Tcopy(mtype);
            break;
        default:
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unknown topology relation type %s (%d)",
                                                  ss_string_ptr(SS_RELREP_P(&relrep,name)), SS_RELREP(&relrep)->id));
        }
    } else {
        /* The relation is stored on a decomposition. There are two cases here: the caller wishes to receive info about the map
         * on self decomposition (remapping the values to "global") or the caller wishes to recieve info on the (indirect)
         * handles to the relations which actually have the data. The caller informs SAF of which of these two cases is desired
         * by using the saf_target_topo_relation function. */
        if (SS_PERS_ISNULL(&target->decomp)) {
            desireHandles = TRUE;
        } else if (_saf_is_self_decomp(&target->decomp)) {
            desireHandles = FALSE;
        } else {
            desireHandles = TRUE;
        }
        if (desireHandles) {
            if (NULL==_saf_getCollection_set(&sup, &storage_decomp, &storage_coll))
                SAF_ERROR(SAF_CONSTRAINT_ERROR,
                          _saf_errmsg("collection \"%s\" not found on set \"%s\"",
                                      ss_string_ptr(SS_CAT_P(&storage_decomp,name)),
                                      ss_string_ptr(SS_SET_P(&sup,name))));
            if (PrepType) *PrepType = *SAF_NOT_APPLICABLE_RELREP;
            if (abuf_sz) *abuf_sz = (size_t)SS_COLLECTION(&storage_coll)->count;
            if (abuf_type) *abuf_type = H5Tcopy(ss_pers_tm);
            if (bbuf_sz) *bbuf_sz = 0;
            if (bbuf_type) *bbuf_type = H5I_INVALID_HID;
        } else {
            /* Case 2: the caller wishes to receive info about the maps as though they had been stored on self rather than on a
             * decomposition. The caller should expect the info on the number and types of items stored at A- and/or B-buffers.
             * The appropriate number of buffers data depends in the nature of the relation: structured, unstructured, or
             * arbitrary. */

            /* First we'll get the indirect handles using a recursive call. */
            IAcount = 0;
            IAtype = H5I_INVALID_HID;
            IBcount = 0;
            IBtype = H5I_INVALID_HID;
            saf_get_count_and_type_for_topo_relation(pmode, rel, NULL, NULL, &IAcount, &IAtype, &IBcount, &IBtype);
            if (IAcount < 1)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("no indirect topo handles"));
            IAbuf = NULL;
            IBbuf = NULL;
            if (saf_read_topo_relation(pmode, rel, NULL, &IAbuf, &IBbuf)!=SAF_SUCCESS || !IAbuf)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't retrieve indirect topo handles"));

            /* We'll read each of the topo relations on the decomposition to determine the "mapping factor". The mapping factor
             * is the number of maps for each member of the "stitched" collection (example: nodes per zone). We'll be using the
             * "mapping factor" to compute the returned (predicted) B-buffer size. */
            haveFactor = FALSE;
            haveRelRep = SS_RELREP_NULL;
            theRels = IAbuf;
            for (d=0; d<IAcount; d++) {
                void *Abuf, *Bbuf;
                size_t Acount=0, Bcount=0;
                hid_t Atype=H5I_INVALID_HID, Btype=H5I_INVALID_HID;
                SAF_RelRep thisRelRep;

                saf_get_count_and_type_for_topo_relation(pmode, theRels+d, NULL, &thisRelRep, &Acount, &Atype, &Bcount, &Btype);
                if (SS_PERS_ISNULL(&haveRelRep)) {
                    haveRelRep = thisRelRep;
                } else if (!SAF_EQUIV(&haveRelRep, SAF_ERROR_RELREP) && !SAF_EQUIV(&haveRelRep,&thisRelRep)) {
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("multiple relation representations found"));
                }
                Abuf = Bbuf = NULL;
                if (saf_read_topo_relation(pmode, theRels+d, NULL, &Abuf, &Bbuf)!=SAF_SUCCESS || !Abuf)
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't retrieve indirect topo handles"));

                /* Look at the size of the A-buffer and the contents if the size is exactly 1. This is the "mapping factor"
                 * indicating how many of the "stitching" category for each of the "stitched" category.  Update both the max an
                 * min encountered... */
                if (Acount==1 && Bcount>0) {
                    size_t tmp;
                    _saf_convert(Atype, Abuf, H5T_NATIVE_SIZE, &tmp);
                    if (haveFactor) {
                        if (tmp<minFactor) minFactor = tmp;
#ifdef SSLIB_SUPPORT_PENDING
                        /* This is the old code. It looks wrong. [rpm 2004-05-26] */
                        if (minFactor<tmp) maxFactor = tmp;
#else
                        if (tmp>maxFactor) maxFactor = tmp;
#endif /*SSLIB_SUPPORT_PENDING*/
                    } else {
                        minFactor = maxFactor = tmp;
                        haveFactor = TRUE;
                    }
                }
                Abuf = SS_FREE(Abuf);
                Bbuf = SS_FREE(Bbuf);
            }

            if (SAF_EQUIV(&haveRelRep, SAF_ARBITRARY)) {
                if (PrepType) *PrepType = *SAF_ARBITRARY;
                SAF_ERROR(SAF_NOTIMPL_ERROR, _saf_errmsg("\"arbitrary\" topo remapping not implemented yet"));
            } else if (SAF_EQUIV(&haveRelRep, SAF_STRUCTURED)) {
                if (PrepType) *PrepType = *SAF_STRUCTURED;
                if (abuf_sz) *abuf_sz = 0;
                if (abuf_type) *abuf_type = H5I_INVALID_HID;
                if (bbuf_sz) *bbuf_sz = 0;
                if (bbuf_type) *bbuf_type = H5I_INVALID_HID;
            } else if (SAF_EQUIV(&haveRelRep, SAF_UNSTRUCTURED)) {
                if (haveFactor && minFactor==maxFactor) {
                    saf_describe_collection(pmode, SS_REL_P(rel,sub), SS_REL_P(rel,sub_cat), NULL, &collectionSize, NULL, NULL,
                                            NULL);
                    if (PrepType) *PrepType = *SAF_UNSTRUCTURED;
                    if (abuf_sz) *abuf_sz = 1;
                    if (abuf_type) *abuf_type = H5Tcopy(H5T_NATIVE_SIZE);
                    if (bbuf_sz) *bbuf_sz = collectionSize * minFactor;
                    if (bbuf_type) *bbuf_type = H5Tcopy(H5T_NATIVE_SIZE);
                } else {
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("mixed factors not supported in topo remapping"));
                }
            }

            IAbuf = SS_FREE(IAbuf);
            IBbuf = SS_FREE(IBbuf);
        }
    }

theExit:

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Topology Relations
 * Purpose:	Read topological relation data
 *
 * Description: This function reads topological relation data from the database.	
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_read_topo_relation(SAF_ParMode pmode,	/* The parallel mode. */
		       SAF_Rel *rel,		/* The topology relation to be read. */
                       SAF_RelTarget *target,   /* Relation targeting information. */
		       void **abuf,     	/* The returned data. See saf_declare_topo_relation(). */
		       void **bbuf		/* The returned data. See saf_declare_topo_relation(). */
		       )
{
    SAF_ENTER(saf_read_topo_relation, SAF_PRECONDITION_ERROR);
    static SAF_RelTarget        rt_zero;                /* Default targeting */
    ss_scope_t                  scope=SS_SCOPE_NULL;    /* Scope containing REL */
    int                         scope_size;             /* Size of the communicator for `scope' */
    hid_t                       ftype=-1, mtype=-1;     /* File and memory datatypes */
    size_t                      mtype_size;             /* Size in bytes of the memory datatype */
    double                      timer_start=0;
    ss_set_t                    sub=SS_SET_NULL, sup=SS_SET_NULL, Dstitching_set=SS_SET_NULL, Dusing_set=SS_SET_NULL;
    ss_cat_t                    storage_decomp=SS_CAT_NULL, Dstitching_cat=SS_CAT_NULL, Dusing_cat=SS_CAT_NULL;
    ss_collection_t             storage_coll=SS_COLLECTION_NULL;
    ss_relrep_t                 relrep=SS_RELREP_NULL, thisRelRep=SS_RELREP_NULL;
    ss_blob_t                   rblob=SS_BLOB_NULL;     /* range blob */
    ss_blob_t                   dblob=SS_BLOB_NULL;     /* domain blob */
    hsize_t                     d_nelmts, r_nelmts;     /* number of elements in domain and range blobs */
    ss_cat_t                    sub_cat=SS_CAT_NULL;
    ss_cat_t                    sup_cat=SS_CAT_NULL;
    ss_collection_t             sub_coll=SS_COLLECTION_NULL;
    hbool_t                     desireHandles;
    size_t                      bufferSize, IAcount, Acount, *Abuffer=NULL, *Bbuffer=NULL, Bcount, Nstitched, mapped_u;
    size_t                      SS_stitching_Acount, NusingPerStitched, mapped_s;
    size_t                      d, i, j, k, s, u;
    int                         tmp_i, numberOfSubsetRelations, jj, *p, start, count, stride;
    SAF_RelRep                  theRep, subsetRelationType;
    SAF_Rel                     *IAbuf=NULL, *subsetRelationList=NULL;
    hid_t                       IAtype=-1, Atype=-1, Btype=-1, SS_stitching_Atype=-1, SS_using_Atype=-1;
    ss_file_t                   db=SS_FILE_NULL;        /* File to which REL belongs and to which searches are limited. */
    void                        *SS_stitching_Abuf=NULL, *SS_using_Abuf=NULL, *Abuf=NULL, *Bbuf=NULL;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    ss_pers_scope((ss_pers_t*)rel, &scope);
    ss_scope_comm(&scope, NULL, NULL, &scope_size);
    if (!target) target = &rt_zero;

    SAF_REQUIRE(SS_REL(rel), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("REL must be a valid relation handle"));
    SAF_REQUIRE(abuf, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ABUF must be non-null"));
    SAF_REQUIRE(!bbuf || !SAF_XOR(*abuf,*bbuf), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("either both ABUF and BBUF point to NULL or both ABUF and BBUF do not point to NULL"));
    SAF_REQUIRE((target->is_set && (SS_PERS_ISNULL(&target->decomp) || pmode==SAF_ALL || 1==scope_size)) || !target->is_set,
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if targeting of storage decomposition is used, the read must be a SAF_ALL mode read "
                            "or the database must be opened on only a single processor"));

    if (_SAF_GLOBALS.p.TraceTimes)
        timer_start = _saf_wall_clock(false);
    
    /* Cache some things for convenience */
    ss_pers_file((ss_pers_t*)rel, &db);
    sub = SS_REL(rel)->sub;
    sup = SS_REL(rel)->sup;
    sub_cat = SS_REL(rel)->sub_cat;
    sup_cat = SS_REL(rel)->sup_cat;
    storage_decomp = SS_REL(rel)->sub_decomp_cat;
    relrep = SS_REL(rel)->rep_type;
    rblob = SS_REL(rel)->r_blob;
    dblob = SS_REL(rel)->d_blob;

    /* Reading a topological relation that is stored on self is quite a bit different that one that is stored on a
     * decomposition... */
    if (_saf_is_self_decomp(&storage_decomp)) {
	/* In this case the topological relation is stored on self.  The number of items stored at A- and B-buffer data depends
         * in the nature of the relation: structured, unstructured, or arbitrary. First of all structured topology has no
         * buffers to read at all. */
        if (SAF_STRUCTURED_ID==SS_RELREP(&relrep)->id)
	    goto theExit;

        /* Range blob */
        ss_blob_bound_f1(&rblob, NULL, NULL, &r_nelmts, &ftype);
        mtype = H5Tget_native_type(ftype, H5T_DIR_DEFAULT);
        mtype_size = H5Tget_size(mtype);
        H5Tclose(ftype); ftype=-1;

	/* As previosly noted, depending if the topological relation is unstructured or arbitrary we'll be needing either 1 or
         * 2 buffers to receive the data.  Which we then fill with data of the datatype found above. */
        switch (SS_RELREP(&relrep)->id) {
        case SAF_UNSTRUCTURED_ID:
            /* Allocate space if necessary */
            if (!*abuf && NULL==(*abuf=malloc(mtype_size)))
                SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable to allocate space to read topological relation"));
            if (bbuf && !*bbuf && r_nelmts && NULL==(*bbuf=malloc((size_t)r_nelmts*mtype_size)))
                SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable to allocate space to read topological relation"));

            /* Read the range (bbuf) data */
            if (bbuf) {
                if (ss_blob_bind_m1(&rblob, *bbuf, mtype, r_nelmts)<0 ||
                    NULL==ss_blob_read1(&rblob, (hsize_t)0, r_nelmts, SS_BLOB_UNBIND, NULL))
                    SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("unable to read relation data"));
            }

            /* Compute the # of range refs per member of collection */
            if (NULL==_saf_getCollection_set(&sub, &sub_cat, &sub_coll))
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to obtain collection for subset"));
            tmp_i = SS_COLLECTION(&sub_coll)->count ? r_nelmts / SS_COLLECTION(&sub_coll)->count : 0;
            if (NULL==_saf_convert(H5T_NATIVE_INT, &tmp_i, mtype, *abuf))
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("data type is not appropriate for a relation"));
            break;
        case SAF_ARBITRARY_ID:
            /* In this case, we need to the domain of the relation too */
            ss_blob_bound_f1(&dblob, NULL, NULL, &d_nelmts, NULL);
            
            /* Allocate space if necessary */
            if (!*abuf && NULL==(*abuf=malloc((size_t)d_nelmts*mtype_size)))
                SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable to allocate space to read topological relation"));
            if (bbuf && !*bbuf && NULL==(*bbuf=malloc((size_t)r_nelmts*mtype_size)))
                SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable to allocate space to read topological relation"));

            /* Read the range data */
            if (bbuf) {
                if (ss_blob_bind_m1(&rblob, *bbuf, mtype, r_nelmts)<0 ||
                    NULL==ss_blob_read1(&rblob, (hsize_t)0, r_nelmts, SS_BLOB_UNBIND, NULL))
                    SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("unable to read relation data"));
            }
            
            /* Read the domain data */
            if (ss_blob_bind_m1(&dblob, *abuf, mtype, d_nelmts)<0 ||
                NULL==ss_blob_read1(&dblob, (hsize_t)0, d_nelmts, SS_BLOB_UNBIND, NULL))
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("unable to read relation data"));

            break;
        default:
            SAF_ERROR(SAF_FILE_ERROR,
                      _saf_errmsg("unknown topology relation type %s (%d)",
                                  ss_string_ptr(SS_RELREP_P(&relrep,name)), SS_RELREP(&relrep)->id));
		
        }
    } else {
	/* The relation is stored on a decomposition. There are two cases here: the caller wishes to receive the map on self
         * decomposition (remapping the values to "global") or the caller wishes to recieve the (indirect) handles to the
         * relations which actually have the data. The caller informs SAF of which of these two cases is desired by using the
         * saf_target_topo_relation function. */
        if (SS_PERS_ISNULL(&target->decomp)) {
	    desireHandles = TRUE;
        } else if (_saf_is_self_decomp(&target->decomp)) {
            desireHandles = FALSE;
        } else {
            desireHandles = TRUE;
        }
        if (desireHandles) {
            /* Case 1: the caller wishes to receive the indirect handles. This is the common case: the caller should expect a
             * single buffer (the A-buffer) of handles, we'll construct a buffer to hold the row numbers picked up from the
             * blob... */
            if (NULL==_saf_getCollection_set(&sup, &storage_decomp, &storage_coll))
                SAF_ERROR(SAF_CONSTRAINT_ERROR,
                          _saf_errmsg("collection \"%s\" not found on set \"%s\"",
                                      ss_string_ptr(SS_CAT_P(&storage_decomp,name)),
                                      ss_string_ptr(SS_SET_P(&sup,name))));
	    bufferSize = SS_COLLECTION(&storage_coll)->count;

            /* Read the relation links */
            if (NULL==(*abuf=ss_array_get(SS_REL_P(rel,indirect_rels), ss_pers_tm, (size_t)0, bufferSize, *abuf)))
                SAF_ERROR(SAF_MEMORY_ERROR,_saf_errmsg("unable to obtain topological relation data"));
        } else {
            /* Case 2: the caller wishes to receive the maps as though they had been stored on self rather than on a
             * decomposition. The caller should expect the appropriate number of buffers, the number of items stored at A- and
             * B-buffer data depends in the nature of the relation: structured, unstructured, or arbitrary. */

            /* First we'll get the indirect handles using a recursive call. */
	    saf_get_count_and_type_for_topo_relation(pmode, rel, target, &theRep, &Acount, &Atype, &Bcount, &Btype);

            switch (SS_RELREP(&theRep)->id) {
            case SAF_ARBITRARY_ID:
                SAF_ERROR(SAF_NOTIMPL_ERROR, _saf_errmsg("remapping of arbitrary topology not implemented yet"));
            case SAF_STRUCTURED_ID:
                SAF_ERROR(SAF_NOTIMPL_ERROR, _saf_errmsg("remapping of structured topology not implemented yet"));
            case SAF_UNSTRUCTURED_ID:
                Abuffer = malloc(sizeof(*Abuffer));
                Bbuffer = malloc(Bcount*sizeof(*Bbuffer));
                if (!Abuffer || !Bbuffer)
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("out of memory"));
                Abuffer[0] = SS_NOSIZE;
                for (i=0; i<Bcount; i++) Bbuffer[i] = SS_NOSIZE;
                
                saf_get_count_and_type_for_topo_relation(pmode, rel, NULL, NULL, &IAcount, &IAtype, NULL, NULL);
                if (IAcount<1)
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("no indirect topo handles"));
                if (saf_read_topo_relation(pmode, rel, NULL, (void**)&IAbuf, NULL)!=SAF_SUCCESS || !IAbuf)
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't retrieve indirect topo handles"));

                for (d=0; d<IAcount; ++d) {
                    SS_stitching_Abuf = SS_using_Abuf = Abuf = Bbuf = NULL;
                    saf_describe_topo_relation(pmode, IAbuf+d, &Dstitching_set, &Dstitching_cat, &Dusing_set, &Dusing_cat,
                                               NULL, NULL, NULL);
                    if (!SAF_EQUIV(&sub_cat,&Dstitching_cat))
                        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("remaping of boundary relations not supported yet"));
                    if (!SAF_EQUIV(&sup_cat,&Dusing_cat))
                        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("remaping of boundary relations not supported yet"));

                    if (!SAF_EQUIV(&sub,&Dstitching_set)) {
                        subsetRelationList=NULL;
                        saf_find_subset_relations(pmode, &db, &sub, &Dstitching_set, &sub_cat, &Dstitching_cat,
                                                  SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE, &numberOfSubsetRelations,
                                                  &subsetRelationList);
                        saf_describe_subset_relation(pmode, subsetRelationList+0, NULL, NULL, NULL, NULL, NULL, NULL,
                                                     &subsetRelationType, NULL);
                        if (numberOfSubsetRelations!=1 || !subsetRelationList)
                            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't retrieve subset relations"));
                        saf_get_count_and_type_for_subset_relation(pmode, subsetRelationList+0, NULL, &SS_stitching_Acount,
                                                                   &SS_stitching_Atype, NULL, NULL);
                        saf_read_subset_relation(pmode, subsetRelationList+0, NULL, &SS_stitching_Abuf, NULL);
                        if (SAF_HSLAB_ID==SS_RELREP(&subsetRelationType)->id) {
                            if (SS_stitching_Acount!=3)
                                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid hyper slab relation"));
                            if (H5Tequal(SS_stitching_Atype,H5T_NATIVE_INT)) {
                                start  = ((int *)SS_stitching_Abuf)[0];
                                count  = ((int *)SS_stitching_Abuf)[1];
                                stride = ((int *)SS_stitching_Abuf)[2];
                                SS_stitching_Abuf = SS_FREE(SS_stitching_Abuf);
                                p = malloc(count*sizeof(int));
                                for (jj=0; jj<count; jj++)
                                    p[jj] = start + jj * stride;
                                SS_stitching_Acount = count;
                                SS_stitching_Abuf = p;
                            } else {
                                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid hyper slab type"));
                            }
                        }
                        subsetRelationList = SS_FREE(subsetRelationList);
                    }
                    if (!SAF_EQUIV(&sup, &Dusing_set)) {
                        subsetRelationList=NULL;
                        saf_find_subset_relations(pmode, &db, &sup, &Dusing_set, &sup_cat, &Dusing_cat, SAF_BOUNDARY_FALSE,
                                                  SAF_BOUNDARY_FALSE, &numberOfSubsetRelations, &subsetRelationList);
                        if (numberOfSubsetRelations!=1 || !subsetRelationList)
                            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't retrieve subset relations"));
                        saf_get_count_and_type_for_subset_relation(pmode, subsetRelationList+0, NULL, NULL, &SS_using_Atype,
                                                                   NULL, NULL);
                        saf_read_subset_relation(pmode, subsetRelationList+0, NULL, &SS_using_Abuf, NULL);
                        subsetRelationList = SS_FREE(subsetRelationList);
                    }
                    saf_get_count_and_type_for_topo_relation(pmode, IAbuf+d, NULL, &thisRelRep, &Acount, &Atype, &Bcount, &Btype);
                    if (Acount > 0 && Bcount > 0) {
                        switch (SS_RELREP(&thisRelRep)->id) {
                        case SAF_ARBITRARY_ID:
                            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't remap arbitrary topo relation"));
                        case SAF_STRUCTURED_ID:
                            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't remap structured topo relation"));
                        case SAF_UNSTRUCTURED_ID:
                            saf_read_topo_relation(pmode, IAbuf+d, NULL, &Abuf, &Bbuf);
                            /* At this point we have every thing we need to add the contribution of this "domain" to the global
                             * map: one or more subset maps which show how a local member of a collection (such as a node or a
                             * zone), and the "local" topo maps. And all we must do is the actual composition and insertion
                             * into the map being assembled. */
                            
                            /* Retrieve the number of "usings" per "stitched" (Abuf[0]). In a typical case "using" is "nodes"
                             * and "stitched" is "zones" (but it is not limited to this)... */
                            _saf_convert(Atype, Abuf, H5T_NATIVE_SIZE, &NusingPerStitched);
                            if (SS_NOSIZE==Abuffer[0]) {
                                Abuffer[0] = NusingPerStitched;
                            } else if (Abuffer[0]!=NusingPerStitched) {
                                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("inconsistant mapping factor"));
                            }
                            Nstitched = Bcount / NusingPerStitched;

                            /* Now we'll run through each of the members being "stitched" and for each of these we'll run
                             * though the N relations associated where N is number of "usings" per "stitched" (ie. nodes per
                             * zone). */
                            for (s=0; s<Nstitched; s++) {
                                if (SS_stitching_Abuf) {
                                    _saf_convert(SS_stitching_Atype, (char*)SS_stitching_Abuf + s*H5Tget_size(SS_stitching_Atype),
                                                 H5T_NATIVE_SIZE, &mapped_s);
                                } else {
                                    mapped_s = s;
                                }
                                for (i=0; i<NusingPerStitched; i++) {
                                    /* Within each "stitched" (ie. zone) we'll run through its list of "usings" (ie. nodes). */
                                    j  = s * NusingPerStitched + i;
                                    _saf_convert(Btype, (char*)Bbuf + j*H5Tget_size(Btype), H5T_NATIVE_SIZE, &u);
                                    if (SS_using_Abuf) {
                                        _saf_convert(SS_using_Atype, (char*)SS_using_Abuf + u*H5Tget_size(SS_using_Atype),
                                                     H5T_NATIVE_SIZE, &mapped_u);
                                    } else {
                                        mapped_u = u;
                                    }
                                    k = mapped_s * NusingPerStitched + i;
                                    Bbuffer[k] = mapped_u;
                                }
                            }
                            break;
                        default:
                            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't remap topo relation of unsupported type"));
                        }

                        Abuf = SS_FREE(Abuf);
                        Bbuf = SS_FREE(Bbuf);
                        SS_stitching_Abuf = SS_FREE(SS_stitching_Abuf);
                        SS_using_Abuf = SS_FREE(SS_using_Abuf);
                    }
                }
                if (abuf) {
                    *abuf = Abuffer;
                    Abuffer = NULL;
                }
                if (bbuf) {
                    *bbuf = Bbuffer;
                    Bbuffer = NULL;
                }
                break;
            default:
                SAF_ERROR(SAF_FILE_ERROR,_saf_errmsg("unsupported relation representation type"));
            }

            Abuffer = SS_FREE(IAbuf);
            Abuffer = SS_FREE(Abuffer);
            Bbuffer = SS_FREE(Bbuffer);
        }
    }

theExit:

    if (_SAF_GLOBALS.p.TraceTimes)
        _SAF_GLOBALS.CummReadTime += (_saf_wall_clock(false) - timer_start);

    SAF_LEAVE(SAF_SUCCESS);
}
