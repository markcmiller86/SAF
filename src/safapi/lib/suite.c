/*
 *       Copyright(C) 1999-2005 The Regents of the University of California.
 *           This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *           Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *           U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *           the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *           dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *           Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *           Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *           University.
 *       
 *       Copyright(C) 1999-2005 Sandia Corporation.  
 *           Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
 *           on behalf of the U.S. Government.  Export  of this program may require a license from the United
 *           States Government.
 *       
 *       Disclaimer:
 *           This document was  prepared as an account of  work sponsored by an agency  of  the United States
 *           Government. Neither the United States  Government nor the United States Department of Energy nor
 *           the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any
 *           warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the
 *           accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process
 *           disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference
 *           herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark,
 *           manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement,
 *           recommendation, or favoring by the  United States Government   or the University of  California.
 *           The views and opinions of authors expressed herein do not necessarily state  or reflect those of
 *           the  United  States Government or  the   University of California   and shall  not be  used  for
 *           advertising or product endorsement purposes.
 *       
 *       
 *       Active Developers:
 *           Peter K. Espen              SNL
 *           Eric A. Illescas            SNL
 *           Jake S. Jones               SNL
 *           Robb P. Matzke              LLNL
 *           Greg Sjaardema              SNL
 *       
 *       Inactive Developers:
 *           William J. Arrighi          LLNL
 *           Ray T. Hitt                 SNL
 *           Mark C. Miller              LLNL
 *           Matthew O'Brien             LLNL
 *           James F. Reus               LLNL
 *           Larry A. Schoof             SNL
 *       
 *       Acknowledgements:
 *           Marty L. Barnaby            SNL - Red parallel perf. study/tuning
 *           David M. Butler             LPS - Data model design/implementation Spec.
 *           Albert K. Cheng             NCSA - Parallel HDF5 support
 *           Nancy Collins               IBM - Alpha/Beta user
 *           Linnea M. Cook              LLNL - Management advocate
 *           Michael J. Folk             NCSA - Management advocate 
 *           Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
 *           Wilbur R. Johnson           SNL - Early developer
 *           Quincey Koziol              NCSA - Serial HDF5 Support 
 *           Celeste M. Matarazzo        LLNL - Management advocate
 *           Tyce T. McLarty             LLNL - parallel perf. study/tuning
 *           Tom H. Robey                SNL - Early developer
 *           Reinhard W. Stotzer         SNL - Early developer
 *           Judy Sturtevant             SNL - Red parallel perf. study/tuning 
 *           Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
 *       
 */
#include <safP.h>

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Suites
 *
 * Description: A suite is a cartesian product of two base spaces (i.e., sets): one is the mesh from a simulation (typically
 *              a set with a SIL role of SAF_SPACE) and the other is a base space representing time or some other parametric
 *              space.  Fields can be defined on a suite that are either slices through space (at constant times) or slices through
 *              time (at constant locations in space).  The former are referred to as states; the latter are histories.
 *
 *
 * Issues:	
 *
 *--------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Suites
 * Purpose:	Declare a suite
 *
 * Description:	This creates a suite with the given name.
 *
 * Return:      Returns a pointer to a new suite on success; null on failure. If the caller supplies a SUITE argument then
 *              this will be the pointer that is returned instead of allocating a new suite handle.
 *
 * Issues:	Currently, a SAF_Suite is #typedef'd to a SAF_Set.  This may still work if we can identify suites with a new
 *              SIL role ("SAF_SUITE").
 *
 *              The ability to declare "subsuites" may be needed but won't be available in this implementation.  This 
 *              implementation will handle just 1-dimensional parametric spaces, such as time.
 *
 *              This function will create an extendible SAF_Set with two collections:  one for associating states 
 *              (fields across space at a fixed value of the specified parameter, usually time) and one for associating histories 
 *              (fields across the specified parameter, usually time, at a fixed point in space).  We will declare two new
 *              categories, "SAF_SPACE_SLICE" and "SAF_PARAM_SLICE", respectively, that will be used in the declaration of
 *              these collections.  These new categories will be a minor enhancement to the current /self/ category.
 *
 *              If PARAM_SPACE is passed as NULL, this function will create a SAF_Set to represent the parametric space.
 *              
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Suite *
saf_declare_suite(SAF_ParMode  pmode,		/* The parallel mode. */
                  SAF_Db       *database,	/* The SAF database handle. */
		  const char   *name,		/* The name of the suite. */
		  SAF_Set      *mesh_space,     /* The set representing the computational mesh.
                                                 * this is currently only a single set, so assume that the
                                                 * user cannot supply a list of mesh_space sets when declaring a suite */
                  SAF_Set      *param_space,    /* The set representing the parametric space, such as time.  If this is NULL,
                                                 * a set will be created with a SIL role of type TYPE. */
		  SAF_Suite    *suite		/* OUT: Optional memory for the returned handle. If null then a new handle is
                                                 * allocated by this function. */
		  )
{

    SAF_ENTER(saf_declare_suite,0);

    int l_numFound = 0;
    int l_spacecatsFound = 0;
    int l_paramcatsFound = 0;

    SAF_Cat *space_cat=NULL, *param_cat=NULL;
    SAF_Set param_set=SS_SET_NULL;

    SAF_Rel suite_param_rel=SS_REL_NULL, suite_mesh_rel=SS_REL_NULL;

    /* create a set with SIL role "SAF_SUITE" to uniquely identify this as a cross-product base space */
    suite = saf_declare_set(pmode, database, name, 1, SAF_SUITE, SAF_EXTENDIBLE_TRUE, suite);


    /* find the SAF_SPACE_SLICE collection/category, else create it */
    saf_find_collections(pmode, suite, SAF_SPACE_SLICE,
                         SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, 
                         &l_numFound, &space_cat);


    if( l_numFound <= 0 ) {
        _saf_free(space_cat);
        space_cat = NULL;
        saf_find_categories (pmode, database, SAF_UNIVERSE(XXX), SAF_ANY_NAME, SAF_SPACE_SLICE,
                             SAF_ANY_TOPODIM, &l_spacecatsFound, &space_cat);
	if( l_spacecatsFound <= 0 ) {
            /* space_cat = (SAF_Cat *)malloc(sizeof(SAF_Cat)); */
            saf_declare_category(SAF_ALL, database, "space_slice_cat",
                                 SAF_SPACE_SLICE, 1, space_cat);
	}
    	if( space_cat == NULL ) {
            SAF_ERROR(0, _saf_errmsg("SAF_SPACE_SLICE category could not be created"));
	}
  	saf_declare_collection (pmode, suite, space_cat,
                                SAF_CELLTYPE_POINT, 1, SAF_1DC(1),
                                SAF_DECOMP_FALSE);
    }
    if( space_cat == NULL ) {
        SAF_ERROR(0, _saf_errmsg("SAF_SPACE_SLICE category could not be created"));
    }

    /* find the SAF_PARAM_SLICE collection/category, else create it */
    l_numFound = 0;
    saf_find_collections(pmode, suite, SAF_PARAM_SLICE,
                         SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, 
                         &l_numFound, &param_cat);

    if( l_numFound <= 0 ) {
        _saf_free(param_cat);
	param_cat = NULL;
	saf_find_categories (pmode, database, SAF_UNIVERSE(XXX), SAF_ANY_NAME, SAF_PARAM_SLICE,
                             SAF_ANY_TOPODIM, &l_paramcatsFound, &param_cat);
	if( l_paramcatsFound <= 0 ) {	
            /* param_cat = (SAF_Cat *)malloc(sizeof(SAF_Cat)); */
            saf_declare_category(SAF_ALL, database, "param_slice_cat",
                                 SAF_PARAM_SLICE, 1, param_cat);
	}
    	if( param_cat == NULL ) {
            SAF_ERROR(0, _saf_errmsg("SAF_PARAM_SLICE category could not be created"));
	}
  	saf_declare_collection (pmode, suite, param_cat,
                                SAF_CELLTYPE_POINT, 1, SAF_1DC(1),
                                SAF_DECOMP_FALSE);
    }
    if( param_cat == NULL ) {
        SAF_ERROR(0, _saf_errmsg("SAF_PARAM_SLICE category could not be created"));
    }


    /* if param_space is NULL, create a set for the parametric space */
    if ( param_space == NULL ) {
        saf_declare_set(pmode, database, "suite_param_set", 1, SAF_TIME, SAF_EXTENDIBLE_TRUE, &param_set);
    } else {
        param_set = *param_space;
    }
  

    /* check whether this "param_slice_cat" collection already exists on the param_set ??? */

    /* declare the collection on param_set for "slices" */
    saf_declare_collection(pmode, &param_set, param_cat, SAF_CELLTYPE_POINT, 1, SAF_1DC(1), SAF_DECOMP_FALSE);
  
    saf_declare_subset_relation(pmode, database, suite, &param_set, SAF_COMMON(param_cat), SAF_TUPLES, H5T_NATIVE_INT,
                                NULL, H5I_INVALID_HID, NULL, &suite_param_rel);



  
    /* find or declare the "slice" cat/collection on mesh_space and create subset_relation */
    saf_declare_collection(pmode, mesh_space, space_cat, SAF_CELLTYPE_POINT, 1, SAF_1DC(1),SAF_DECOMP_FALSE);

    saf_declare_subset_relation(SAF_ALL, database, suite, mesh_space, SAF_COMMON(space_cat), SAF_TUPLES, SAF_INT, NULL,
                                H5I_INVALID_HID, NULL, &suite_mesh_rel);

    free(space_cat);
    free(param_cat);

    SAF_LEAVE(suite);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Suites
 * Purpose:	Get a description of a suite
 *
 * Description:	Returns the description of a suite.  This includes the name of the suite and the sets associated with the
 *              "SAF_SPACE_SLICE" and "SAF_PARAM_SLICE" collections.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_suite(SAF_ParMode pmode,		/* The parallel mode. */
                   SAF_Suite   *suite,		/* A suite handle. */
		   char        **name,		/* OUT: The returned name of the suite. Pass NULL if you do not want this value 
                                                 * returned. */
                   int         *num_space_sets, /* OUT: The number of sets returned in MESH_SPACE. */
                   SAF_Set     **mesh_space,    /* OUT: The returned array of sets representing the computational meshes associated
                                                 * with each state of the suite. This is the list of sets in the "SAF_SPACE_SLICE"
                                                 * collection. */
                   SAF_Set     **param_space    /* OUT: The returned array of sets representing the parametric space, such as
                                                 * time. These are associated with the histories of the suite and are thus
                                                 * contained in the "SAF_PARAM_SLICE" collection.  This will not be
                                                 * implemented at this time. */
		   )
{
  SAF_ENTER(saf_describe_suite,0);

  int num_space_cats, num_param_cats, num_param_sets;
  SAF_Cat *space_cats, *param_cats;

  saf_describe_set (pmode, suite, name, NULL, NULL, NULL, NULL, NULL, NULL);

  space_cats = NULL;
  /* saf_find_categories (suite, SAF_ANY_NAME, SAF_SPACE_SLICE, SAF_ANY_TOPODIM, &num_space_cats, &space_cats); */
  saf_find_collections(pmode, suite, SAF_SPACE_SLICE,
                         SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, 
                         &num_space_cats, &space_cats);

  if( (num_space_sets != NULL) && (mesh_space != NULL) )
      /* find all sets that are immediate subsets of suite by the SAF_SPACE_SLICE category */
      saf_find_sets( pmode, SAF_FSETS_SUBS, suite, space_cats+0, num_space_sets, mesh_space);

  if( param_space != NULL ) {
      param_cats = NULL;
      /* saf_find_categories(suite, SAF_ANY_NAME, SAF_PARAM_SLICE, SAF_ANY_TOPODIM, &num_param_cats, &param_cats);  */
      saf_find_collections(pmode, suite, SAF_PARAM_SLICE,
                         SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, 
                         &num_param_cats, &param_cats);
      saf_find_sets(pmode, SAF_FSETS_SUBS, suite, param_cats+0, &num_param_sets, NULL);
      if( num_param_sets > 0 )
          saf_find_sets(pmode, SAF_FSETS_SUBS, suite, param_cats+0, &num_param_sets, param_space);
  }
  SAF_LEAVE(0);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Suites
 * Purpose:	Find suites
 *
 * Description:	Find all the suites in a SAF database.  Under the cover, this finds all sets with a SIL_ROLE of SAF_SUITE.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_suites(SAF_ParMode pmode,	/* The parallel mode. */
                SAF_Db  *database,	/* The database in which to search. */
		const char  *name,	/* The name to limit the search to.  The constant SAF_ANY_NAME can be passed if 
                                         * the client does not want to limit the search by name.  */
		int         *num_suites,/* OUT: The returned number of suites. */
		SAF_Suite   **suites	/* OUT: The returned suites. */
		)
{
  SAF_ENTER(saf_find_suites, SAF_PRECONDITION_ERROR);

  /* find suites of type SAF_SUITE */
  saf_find_matching_sets(pmode, database, name, SAF_SUITE, 1, SAF_EXTENDIBLE_TORF, SAF_TOP_TRUE,
                         num_suites, suites);

  SAF_LEAVE(SAF_SUCCESS);

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Suites
 * Purpose:     Attach an attribute to a suite
 *
 * Description: This function is identical to the generic saf_put_attribute() function except that it is specific to
 *              SAF_Suite objects to provide the client with compile time type checking. For a description,
 *              see saf_put_attribute().
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_put_suite_att(SAF_ParMode pmode, SAF_Suite *suite, const char *att_key, hid_t att_type, int count, const void *value)
{
  SAF_ENTER(saf_put_suite_att, SAF_PRECONDITION_ERROR);
  int retval = saf_put_attribute(pmode, (ss_pers_t*)suite, att_key, att_type, count, value);
  SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Suites
 * Purpose:     Get an attribute attached to a suite
 *
 * Description: This function is identical to the generic saf_get_attribute() function except that it is specific to
 *              SAF_Suite objects to provide the client with compile time type checking. For a description,
 *              see saf_get_attribute().
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_suite_att(SAF_ParMode pmode, SAF_Suite *suite, const char *att_key, hid_t *att_type, int *count, void **value)
{
  SAF_ENTER(saf_get_suite_att, SAF_PRECONDITION_ERROR);
  int retval = saf_get_attribute(pmode, (ss_pers_t*)suite, att_key, att_type, count, value);
  SAF_LEAVE(retval);
}
