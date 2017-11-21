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
 * Chapter:     Collection Categories
 * Description:
 *
 * Collection categories are used to categorize collections of sets or cells. Each collection on a set is one of a particular
 * category. There is only ever one collection of a particular category on a set. Typically, collection categories are used
 * to categorize, for example collections of nodes, elements, processors, blocks, domains, etc. However, collection categories
 * may be used however the client wishes to categorize different collections of sets or cells.
 * 
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Categories
 * Purpose:     Declare a collection category
 *
 * Description: This function declares a collection category.
 *
 * Parallel:    This call must be collective across the database communicator.
 *
 * Return:      The new category handle is returned on success; NULL on failure. If CAT is non-null then it will be initialized
 *              and used as the return value.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Cat *
saf_declare_category(SAF_ParMode pmode,
                     SAF_Db *db,        /* The database handle. */
                     const char *name,  /* The collection category name. */
                     SAF_Role *role,    /* Role of collections of this category (see Collection Roles). */
                     int tdim,          /* The maximum topological dimension of the members of collections of this category. */
                     SAF_Cat *cat       /* OUT: The returned collection category handle. */
                     )
{
   SAF_ENTER(saf_declare_category, NULL);
   ss_scope_t           scope;
   
   SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("PMODE must be valid"));
   if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);
   SAF_REQUIRE(ss_file_isopen(db, NULL)>0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("DB must be a valid database"));
   SAF_REQUIRE(ss_file_iswritable(db)>0, SAF_NO_CHK_COST, NULL,
               _saf_errmsg("the database must not be open for read-only access"));
   SAF_REQUIRE(name != NULL, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("a NAME must be supplied for the category"));
   SAF_REQUIRE(SS_ROLE(role), SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("ROLE must be a valid collection role"));
   SAF_REQUIRE(tdim >= 0, SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("topological dimension, TDIM, must be positive"));

   ss_file_topscope(db, &scope);
   cat = (ss_cat_t*)ss_pers_new(&scope, SS_MAGIC(ss_cat_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)cat, NULL);

   if (SAF_EACH==pmode) SS_PERS_UNIQUE(cat);
   ss_string_set(SS_CAT_P(cat,name), name);
   SS_CAT(cat)->role = *role;
   SS_CAT(cat)->tdim = tdim;

   SAF_LEAVE(cat);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Categories
 * Purpose:     Get a description of a collection category
 *
 * Description: This call describes a collection category.
 *
 * Parallel:    This call must be collective across the database communicator in which the category is defined.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_category(SAF_ParMode pmode,
                      SAF_Cat *cat,     /* A collection category handle. */
                      char **name,      /* If non-NULL, the returned name of the collection category (see Returned Strings). */
                      SAF_Role *role,   /* If non-NULL, the returned role of the collection category (see Collection Roles). */
                      int *tdim         /* If non-NULL, the returned maximum topological dimension of members of collections
                                         * of this category. */
                      )
{
  SAF_ENTER(saf_describe_category, SAF_PRECONDITION_ERROR);

  SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
              _saf_errmsg("PMODE must be valid"));
  if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
  SAF_REQUIRE(SS_CAT(cat), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
              _saf_errmsg("CAT must be a valid category handle"));

  /* return the desired values */
  if (_saf_setupReturned_string(name, ss_string_ptr(SS_CAT_P(cat,name))) != SAF_SUCCESS)
    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to process returned string"));
  if (role) {
      *role = SS_CAT(cat)->role;
  }
  if (tdim != NULL)
      *tdim = SS_CAT(cat)->tdim;

  SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Categories
 * Purpose:     Find collection categories
 *
 * Description:	This function will find collection categories matching the specified NAME, ROLE and TDIM. It searches
 *		collection categories defined on the CONTAINING_SET, which can be set to SAF_UNIVERSE(db), implying that all
 *		collection categories in the *entire* database should be searched. Since the number of collection categories is
 *		relatively small, *and*global*, such a search should not take much time.
 *
 * Parallel:    This function must be called collectively across the database communicator of the CONTAINING_SET.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_categories(SAF_ParMode pmode,
                    SAF_Db *db,                 /* Database on which to restrict the search. */
                    SAF_Set *containing_set,    /* The set upon which to restrict the search. The special macro SAF_UNIVERSE(db)
                                                 * (which takes a database handle as an argument) allows the search to span
                                                 * all categories of the specified database. */
                    const char *name,           /* The name of the categories upon which to restrict the search. The constant
                                                 * SAF_ANY_NAME allows the search to span categories with any name. */
                    SAF_Role *role,             /* The role of the categories upon which to restrict the search. A null pointer
                                                 * allows the search to span categories with any role (see
                                                 * Collection Roles). */
                    int tdim,                   /* The topological dimension of the categories upon which to restrict the
                                                 * search. The constant SAF_ANY_TOPODIM allows the search to span categories
                                                 * with any topological dimension. */
                    int *num,                   /* For this and the succeeding argument [see Returned Handles]. */ 
                    SAF_Cat **found             /* For this and the preceding  argument [see Returned Handles]. */
                    )
{
  SAF_ENTER(saf_find_categories, SAF_PRECONDITION_ERROR);
  SAF_KEYMASK(SAF_Cat, key, mask);
  size_t                nfound=SS_NOSIZE;
  ss_scope_t            scope=SS_SCOPE_NULL;
  size_t                i, j;
  ss_collection_t       coll;
  SAF_Cat               *cats=NULL;

  SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
              _saf_errmsg("PMODE must be valid"));
  if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
  SAF_REQUIRE(ss_file_isopen(db, NULL)>0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
              _saf_errmsg("DB must be a valid database"));
  SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
	      _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));
  SAF_REQUIRE(!role || SS_ROLE(role), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
	      _saf_errmsg("ROLE must be a valid role handle or NULL"));
  SAF_REQUIRE(!containing_set || SS_SET(containing_set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
              _saf_errmsg("CONTAINING_SET must be a valid set handle or NULL"));

  /* fill in appropriate members for the find call */
  if (name)
      SAF_SEARCH_S(SAF_Cat, key, mask, name, name);
  if (tdim != SAF_ANY_TOPODIM)
      SAF_SEARCH(SAF_Cat, key, mask, tdim, tdim);
  if (role)
      SAF_SEARCH_LINK(SAF_Cat, key, mask, role, *role); /*search with `equal' not `eq', thus SAF_SEARCH_LINK() not SAF_SEARCH() */

  /* Get all categories that match (do not limit the results) because we'll need to prune that list against the CONTAINING_SET
   * if there was one. */
  ss_file_topscope(db, &scope);
  cats = (ss_cat_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, NULL, NULL);
  if (!cats) SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("find failed"));

  /*If containing_set is an actual set (and not SAF_UNIVERSE(db)), then remove all
    categories from the list that are not in collections on the set*/
  if (containing_set && !_saf_is_universe(containing_set)) {
      for (i=j=0; i<nfound; i++) {
          if (_saf_getCollection_set(containing_set, cats+i, &coll)) {
              if (i!=j) cats[j] = cats[i];
              j++;
          }
      }
      nfound = j;
  }

  /*   Return only what the caller asked for...
   */
  if (!found) {
      /* Count the matches */
      assert(num);
      *num = nfound;
  } else if (!*found) {
      /* Library allocates results */
      *found = cats;
      cats = NULL;
      if (num) *num = nfound;
  } else {
      /* Find limited matches; client allocates result buffer */
      assert(num);
      if (nfound>(size_t)*num)
          SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matching objects"));
      memcpy(*found, cats, nfound*sizeof(*cats));
      *num = nfound;
  }
  
  /*  Cleanup...
   *  ...file handles if they wern't passed back
   */
  cats = SS_FREE(cats);
  SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Categories   
 * Purpose:     Put an attribute with a cat
 *
 * Description: This function is identical to the generic saf_put_attribute() function except that it is specific to
 *              SAF_Cat objects to provide the client with compile time type checking. For a description,
 *              see saf_put_attribute().
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_put_cat_att(SAF_ParMode pmode,              /* Parallel mode for adding the new attribute. */
                SAF_Cat *cat,                   /* Collection category for which the new attribute is added. */
                const char *name,               /* The name of the attribute. */
                hid_t datatype,                 /* The datatype of each element of the VALUE for the attribute. */
                int count,                      /* The number of elements pointed to by VALUE, each of type DATATYPE. */
                const void *value               /* The array of COUNT elements each of type DATATYPE to use for the
                                                 * attribute's value. */
                )
{
  SAF_ENTER(saf_put_cat_att, SAF_PRECONDITION_ERROR);
  int retval = saf_put_attribute(pmode, (ss_pers_t*)cat, name, datatype, count, value);
  SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Categories   
 * Purpose:     Get an attribute with a cat
 *
 * Description: This function is identical to the generic saf_get_attribute() function except that it is specific to
 *              SAF_Cat objects to provide the client with compile time type checking. For a description,
 *              see saf_get_attribute().
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_cat_att(SAF_ParMode pmode,
                SAF_Cat *cat,                   /* Collection category owning the attribute for which we're searching. */
                const char *name,               /* Name of the attribute. */
                hid_t *datatype,                /* OUT: Datatype of the attribute as it is stored. */
                int *count,                     /* OUT: Number of elements contained in the attribute. */
                void **value                    /* OUT: On successful return this will point to an allocated array containing
                                                 * COUNT elements each of type DATATYPE. */
                )
{
  SAF_ENTER(saf_get_cat_att, SAF_PRECONDITION_ERROR);
  int retval = saf_get_attribute(pmode, (ss_pers_t*)cat, name, datatype, count, value);
  SAF_LEAVE(retval);
}
