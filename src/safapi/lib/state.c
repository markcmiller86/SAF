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

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     States
 *
 * Description: A state is a "slice" through a suite at a fixed parameter value (typically time).  For example, a state
 *              contains all the following that is associated with a specific time step of a simulation:  
 *
 *              - pointer to the computational mesh (i.e., a set); 
 *              - pointer to the default coordinate field (an independent variable) of the mesh; 
 *              - the time value (also an independent variable) of the state; 
 *              - pointers to all the fields (the dependent variables) attached to the mesh at the specific time step.
 *
 *              What if the desired output changes from state to state?.  For example, suppose a client writes various 
 *              combintations of Coordinates (C), Pressure (P), Temperature (T), Velocity (V) and Stress (S) fields according 
 *              to the following sequence...
 *
 *                                                 C   C
 *                         C   C   C   C           V   V
 *                 C   C   V   V   V   V   C   C   T   T
 *                 V   V   T   T   T   T   V   V   P   P
 *                 S   S   P   P   P   P   S   S   S   S
 *                 +---+---+---+---+---+---+---+---+---+
 *                 0   1   2   3   4   5   6   7   8   9   <-- indices
 *
 *                 0  0.5  1  1.5  2  2.5  3  3.5  4  4.5  <-- times
 *
 *              The client should declare a state template that contains field templates for all the fields that will be 
 *              referenced at any state.  In the example above, the state template should consist of field templates for 
 *              C, V, T, P, and S.  For the states that don't contain all the fields, the client should pass a 
 *              SAF_NOT_APPLICABLE_FIELD for those fields that aren't applicable for the state being written.
 *
 *--------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	States
 * Purpose:	Declare a state group
 *
 * Description:	A state group contains all of the states associated with a suite.  It contains:
 *
 *                - a name
 *                - pointer to the suite that these states are attached to
 *                - array of sets that represent the computational meshes associated with each state
 *                - a coordinate field containing two components
 *                    -- a scalar field whose values are the parametric values (e.g., time values) associated with each state
 *                    -- a field whose values are IDs of the default coordinate fields (the independent variable) of the 
 *                       computational mesh associated with each state 
 *                - a field containing IDs of all the fields (dependent variables) attached to the computational mesh at each state
 *
 * Return:      Returns a pointer to the new state group on success; null on failure.  If the caller supplies a STATE_GRP
 *              argument then that becomes the return value, otherwise a new state group link is allocated herein.
 *
 * Issues:	The new implementation of state group supercedes the current concept of a "state field".  A "state field", 
 *              as currently implemented, is just one component of a state group.  Thus, we can delete all references to 
 *              SAF_StateFld and add the new type SAF_StateGrp that contains:
 *
 *                - the name of the state group
 *                - pointer to the SAF_Suite that the state group is attached to
 *                - array of pointers to the SAF_Sets that represent the computational meshes associated with each state
 *                - pointer to a SAF_Field coordinate field containing two components (this field may have to be an indirect field
 *                  if we don't support composite fields with heterogeneous components): 
 *                    -- a scalar field whose values are the parametric values (e.g., time values) associated with each state
 *                    -- an indirect field whose values are IDs of the default coordinate fields of the computational mesh
 *                       associated with each state 
 *                - pointer to an indirect SAF_Field containing IDs of all the fields (dependent variables) attached to the
 *                  computational mesh at each state (this indirect field is what is currently a "state field")
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_StateGrp *
saf_declare_state_group(SAF_ParMode   pmode,		/* The parallel mode. */
                        SAF_Db        *db,              /* The database in which to declare the new state group. */
		        const char    *name,		/* The name of this state group. */
                        SAF_Suite     *suite,           /* The suite that these states are associated with. */
			SAF_Set       *mesh_space,      /* The set representing the computational mesh */
                        SAF_StateTmpl *stmpl,		/* A state template that defines the pattern (via a list of field 
                                                         * templates) of fields that can be stored in each state. */
                        SAF_Quantity  *quantity,        /* The quantity associated with the axis of the parametric space.
                                                         * For example, SAF_TIME_QUANTITY. */
                        SAF_Unit      *unit,            /* The units associated with the axis of the parametric space. */ 
                        hid_t         coord_data_type,  /* The data type of the coordinates of the parametric space. */ 
		        SAF_StateGrp  *state_grp	/* The returned handle for a state group. */
		        )

{
    SAF_ENTER(saf_declare_state_group,0);

    SAF_Cat stategrp_cat;
    int num_space_cats=0, num_param_cats=0, num_ftmpls=0;
    char tmp_name[1024];
    int index[1];
    SAF_FieldTmpl comp_ftmpl[2]; 
    SAF_Cat *space_cats = NULL;
    SAF_Cat *param_cats = NULL;
    SAF_FieldTmpl *coords_ftmpl;
    SAF_Field     *coord_fields;
    SAF_Field     *coords, *dep_var_fld;

    SAF_Field     mesh_defcoord_field;
    SAF_FieldTmpl mesh_defcoord_tmpl;

    SAF_FieldTmpl stategrp_comp_tmpls[2];
    SAF_FieldTmpl stategrp_tmpl;
    SAF_Field     *stategrp_contents;
    SAF_Db        suite_db=SS_FILE_NULL;

    int l_numFound = 0;
    SAF_Cat *l_catsFound = NULL;

    ss_pers_file((ss_pers_t*)suite, &suite_db);

    /* create a coordinate field for the suite, consisting of two components, a coordinate field of the parametric space, and
     * an indirect field containing IDs of the coordinate fields of the computational mesh; these component fields must also
     * be created */

    /* saf_find_categories (suite, "space_slice_cat", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_space_cats, &space_cats); */
    saf_find_collections(pmode, suite, SAF_SPACE_SLICE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM,
                         SAF_DECOMP_TORF, &num_space_cats, &space_cats);
    /* saf_find_categories (suite, "param_slice_cat", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_param_cats, &param_cats); */
    saf_find_collections(pmode, suite, SAF_PARAM_SLICE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM,
                         SAF_DECOMP_TORF, &num_param_cats, &param_cats);

    if( param_cats == NULL || space_cats == NULL ) {
        SAF_ERROR(NULL, _saf_errmsg("param_cats or space_cats collections returned NULL on suite"));
    }

    saf_find_collections(pmode, suite, SAF_TOPOLOGY,
                         SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM,
                         SAF_DECOMP_TORF, &l_numFound, &l_catsFound);

    if( l_numFound <= 0 ) {
        saf_declare_category(SAF_ALL, db, "stategroups",
                             SAF_TOPOLOGY, 0, &stategrp_cat);
        saf_declare_collection(pmode, suite, &stategrp_cat,
                               SAF_CELLTYPE_POINT, 1, SAF_1DC(1),
                               SAF_DECOMP_FALSE);
    } else if( l_numFound >= 1 ) {
        stategrp_cat = l_catsFound[0];
        /* free(l_catsFound); */
    }

    coord_fields = (SAF_Field *)malloc( 2 * sizeof(SAF_Field));

    /* create the coordinate field associated with the parametric space; */
    sprintf (tmp_name, "%s_PARAM_COORDS_TMPL", name);
    saf_declare_field_tmpl (pmode, db, tmp_name, SAF_ALGTYPE_SCALAR, SAF_UNITY, quantity, 1, NULL, &(comp_ftmpl[0]));
    sprintf (tmp_name, "%s_PARAM_COORDS", name);
    saf_declare_field (pmode, db, comp_ftmpl+0, tmp_name, suite, unit, SAF_SELF(XXX), SAF_NODAL(param_cats+0, param_cats+0),
                       coord_data_type, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &(coord_fields[0]));
    saf_declare_coords (pmode, coord_fields+0);

    /* create the indirect field that will contain the coord fields of each of the sets in the state group */
    saf_find_default_coords(pmode, mesh_space, &mesh_defcoord_field);
    if (SS_PERS_ISNULL(&mesh_defcoord_field))
        SAF_ERROR(NULL, _saf_errmsg("default coords are not defined on the mesh set"));

    saf_describe_field(pmode, &mesh_defcoord_field, &mesh_defcoord_tmpl,
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                       NULL, NULL, NULL, NULL, NULL);

    sprintf (tmp_name, "%s_SPACE_COORDS_TMPL", name);
    saf_declare_field_tmpl (pmode, db, tmp_name, SAF_ALGTYPE_FIELD, SAF_ANY_BASIS, SAF_NOT_APPLICABLE_QUANTITY, 1,
                            &mesh_defcoord_tmpl, &(comp_ftmpl[1]));
    sprintf (tmp_name, "%s_SPACE_COORDS", name);

    saf_declare_field (pmode, db, comp_ftmpl+1, tmp_name, suite, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(XXX),
                       SAF_NODAL(space_cats+0, space_cats+0), SAF_HANDLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                       &(coord_fields[1]));
 
    saf_declare_coords (pmode, coord_fields+1);

    /* create the coord field made up of the param coord field and the space coord field */
    /* make this an indirect field until we can determine the status of composite fields containing components with different
     * algebraic types */
    coords = (SAF_Field *)malloc(sizeof(SAF_Field));
    sprintf( tmp_name, "%s_SUITE_COORDS_TMPL",name);
 
    coords_ftmpl = &(stategrp_comp_tmpls[0]);

    saf_declare_field_tmpl (pmode, db, tmp_name, SAF_ALGTYPE_FIELD, SAF_ANY_BASIS, SAF_NOT_APPLICABLE_QUANTITY, 2, comp_ftmpl,
                            coords_ftmpl);
 
    sprintf( tmp_name, "%s_SUITE_COORDS", name);

    saf_declare_field (pmode, db, coords_ftmpl, tmp_name, suite, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(XXX),
                       SAF_NODAL(space_cats+0, space_cats+0), SAF_HANDLE, NULL, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, coords);

    index[0] = 0;

    saf_write_field (pmode, coords, 1, SAF_TUPLES, index, 1, SAF_HANDLE, (void **)&coord_fields, &suite_db);
    _saf_free(coord_fields);

    /* create the composite coord field made up of the param coord field and the space coord field */
#if 0
    /* this is commented out in favor of using the indirect field above */
    sprintf (tmp_name, "%s_SUITE_COORDS_TMPL", name);
    saf_declare_field_tmpl(pmode, tmp_name, database, SAF_ALGTYPE_TUPLE, SAF_CARTESIAN, SAF_QLENGTH, 2,
                           comp_ftmpl, &coords_ftmpl);
    sprintf (tmp_name, "%s_SUITE_COORDS", name);
    saf_declare_field(pmode, coords_ftmpl, tmp_name, suite, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(db),
                      SAF_NODAL(space_cats[0], space_cats[0]),
                      NULL, coord_fields, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coords);
#endif

    /* create an indirect field to contain IDs of the fields for dependent variables; this is the current "state field"; use
     * the state template for this */

    /* find how many fields are in each state; this is the number of field templates in the state template */
    stategrp_comp_tmpls[1] = *stmpl;
    saf_describe_state_tmpl (pmode, stmpl, NULL,  &num_ftmpls, NULL);

    sprintf (tmp_name, "%s_DEP_VAR_FIELD", name);

    dep_var_fld = (SAF_Field *)malloc(sizeof(SAF_Field));
    if (num_ftmpls > 1) {   /* interleave isn't applicable if there's only 1 field per state */
        saf_declare_field (pmode, db, stmpl, tmp_name, suite, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(XXX), 
                           SAF_NODAL(space_cats+0, space_cats+0), SAF_HANDLE, NULL, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                           dep_var_fld);
    } else {
        saf_declare_field (pmode, db, stmpl, tmp_name, suite, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(XXX), 
                           SAF_NODAL(space_cats+0, space_cats+0), SAF_HANDLE, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL,
                           dep_var_fld);
    }

    /* create the StateGrp indirect field that will contain the "composite" coords and dependent variable indirect fields */
    /* associate this stategrp field with the stategrp_cat category on suite */
    sprintf(tmp_name, "%s_TMPL", name);
    
    saf_declare_field_tmpl(pmode, db, tmp_name, SAF_ALGTYPE_FIELD, SAF_ANY_BASIS, SAF_NOT_APPLICABLE_QUANTITY,
                           2, stategrp_comp_tmpls, &stategrp_tmpl);
    saf_declare_field( pmode, db, &stategrp_tmpl, name, suite, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(db), 
                       SAF_NODAL(&stategrp_cat, &stategrp_cat), SAF_HANDLE, NULL, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL,
                       state_grp);

    stategrp_contents = (SAF_Field *)malloc( 2 * sizeof(SAF_Field));
    stategrp_contents[0] = *coords;
    stategrp_contents[1] = *dep_var_fld;

    index[0] = 0;
    saf_write_field (pmode, state_grp, 1, SAF_TUPLES, index, 1, SAF_HANDLE, (void **)&stategrp_contents, &suite_db);

    _saf_free(coords);
    _saf_free(dep_var_fld);
    _saf_free(param_cats);
    _saf_free(space_cats);
    _saf_free(stategrp_contents);
    _saf_free(l_catsFound);
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	States
 * Purpose:	Get a description of a state group
 *
 * Description:	Returns the description of a state group
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_state_group(SAF_ParMode   pmode,		/* The parallel mode. */
                         SAF_StateGrp  *state_grp,	/* The state group to be described. */
                         char          **name,		/* OUT: Returned name of the state group. Pass NULL if you do not want this
                                                         * value returned. */
                         SAF_Suite     *suite,          /* OUT: Returned suite the state group is associated with. */
                         SAF_StateTmpl *stmpl,		/* OUT: Returned state template. Pass NULL if you do not want this value
                                                         * returned. */
                         SAF_Quantity  *quantity,       /* OUT: The returned quantity associated with the axis of the
                                                         * parametric space. For example, SAF_TIME_QUANTITY. */
                         SAF_Unit      *unit,           /* OUT: The returned units associated with the axis of the parametric
                                                         * space. */
                         hid_t         *coord_data_type,/* OUT: The returned data type of the coordinates of the parametric
                                                         * space. */
                         int           *num_states	/* OUT: Returned number of states that have been written to this state
                                                         * group. Pass NULL if you do not want this value returned. */
                         )
{
    SAF_ENTER(saf_describe_state_group, SAF_PRECONDITION_ERROR);

    int num_stategrp_state_comp;
    int index[1];

    SAF_Field *coord;
    SAF_Field *coords;
    SAF_Field *param_coord;
    SAF_Field *stategrp_state;
    SAF_Field *stategrp_contents;

    SAF_FieldTmpl param_coord_tmpl;
    SAF_FieldTmpl tmp_field_tmpl[1];

    hid_t stategrp_state_type;
    size_t stategrp_state_size;

    saf_describe_field(pmode, state_grp, NULL, name, suite, 
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                       NULL, NULL, NULL, NULL, NULL);

    /* the stategrp blob should contain two field handles:
     * the first is the indirect coord field containing the mesh_coords, and param_coord
     * the second is the state field containing the fields stored at this suite_index */
    stategrp_contents = NULL;
    index[0] = 0;
    saf_read_field (pmode, state_grp, NULL, 1, SAF_TUPLES, index, (void **)(&stategrp_contents));
    coord = &(stategrp_contents[0]);
    stategrp_state = &(stategrp_contents[1]);
   
    /* now read the coord field and get the mesh coord and the param coord (dump times) */
    coords = NULL;
    saf_read_field (pmode, coord, NULL, 1, SAF_TUPLES, index, (void **)(&coords));
    param_coord = &(coords[0]);

    /* get information from the param_coord field */
    saf_describe_field(pmode, param_coord, &param_coord_tmpl, NULL, NULL, unit,
                       NULL, NULL, NULL, NULL, NULL, NULL, 
                       coord_data_type, NULL, NULL, NULL, NULL);

    saf_describe_field_tmpl( pmode, &param_coord_tmpl, NULL, NULL, NULL,
                             quantity, NULL, NULL);

    /* get the state template for the stategrp_state  */        
    saf_describe_field(pmode, stategrp_state, tmp_field_tmpl, 
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	
    saf_describe_field_tmpl(pmode, tmp_field_tmpl+0, NULL, NULL, NULL, NULL,
                            &num_stategrp_state_comp, NULL);

    if( stmpl != NULL ) {
        *stmpl = tmp_field_tmpl[0];
    }

    if( num_states != NULL ) {
        /* get the count for the stategrp_state field */
        saf_get_count_and_type_for_field(pmode, stategrp_state, NULL, &stategrp_state_size, &stategrp_state_type);

        *num_states = stategrp_state_size / num_stategrp_state_comp;
    }
    _saf_free(coords); 
    _saf_free(stategrp_contents); 
    SAF_LEAVE(0);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	States
 * Purpose:	Write out a state
 *
 * Description:	Write out all the elements of a state.  This includes the following:
 *
 *              * ID of the computational mesh (i.e., a set ID) associated with this state; 
 *              * ID of the default coordinate field of the mesh; 
 *              * the parametric value (e.g., the time value) associated with this state; 
 *              * IDs of all the fields (the dependent variables) attached to the mesh at this state.
 *
 *              The state is referenced by an index which is an index into each of the arrays that compose the state group.
 *              See the description of a state group.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issues:	This function does the following actions under the covers to implement the cross-product base space.
 *
 *              * increments the "SAF_SPACE_SLICE" collection
 *              * adds a subset relation between the set identified by the MESH argument and the suite set
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_write_state(SAF_ParMode  pmode,		/* The parallel mode. */
                SAF_StateGrp *state_grp,	/* The state group into which this state will be inserted. */
                int          state_index,	/* The index within the state group at which this state will be written.  
                                                 * This index is 0-based. */
                SAF_Set      *mesh_space,       /* The ID of the mesh associated with this state. */
                hid_t        coord_data_type,   /* The data type of COORD */
                void         *coord_data,	/* The coordinate of STATE_INDEX within the state group.  For instance, this
                                                 * is typically the time value of the state. */
		SAF_Field    *fields		/* The fields (the dependent variables) to be written to this state. */
		)
{
  SAF_ENTER(saf_write_state, SAF_PRECONDITION_ERROR);

  SAF_Db db=SS_FILE_NULL;
  SAF_Cat *param_cats = NULL, *space_cats = NULL, *stategrp_cats = NULL, *mesh_space_cats = NULL;
  int num_param_cats = 0, num_space_cats = 0, /*num_stategrp_cats = 0,*/ num_mesh_space_cats = 0;
  int coll_count = 0, param_count = 0;
  int i, add_count;
  int rel_buf[1];
  int index[1];
  SAF_Set suite; /* the suite "associated" with this stategroup */
  SAF_FieldTmpl stategrp_tmpl;

  SAF_Rel /* *space_rels = NULL,*/ *suite_space_rels = NULL;

  SAF_Field *stategrp_state = NULL;
  hid_t group_type;
  size_t group_size;
  SAF_Field *stategrp_contents = NULL;
  SAF_Field *coords = NULL;
  SAF_Field *coord = NULL;
  SAF_Field *mesh_coord = NULL, *param_coord = NULL;
  SAF_Field *mesh_default_coord = NULL;


  /* find the suite associated with this stategroup */
  saf_describe_field(pmode, state_grp, &stategrp_tmpl, NULL, &suite, 
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL);

  saf_get_count_and_type_for_field(pmode, state_grp, NULL, &group_size, &group_type );


  /* get and set the database and db handles */
  ss_pers_file((ss_pers_t*)&suite, &db);


  {
    /* If necessary, extend the SAF_SPACE_SLICE collection with which the states are associated */

    /* Find the SAF_SPACE_SLICE category defined on this set (suite) */
    space_cats = NULL;
    /* saf_find_categories (suite, "space_slice_cat" , SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_space_cats, (SAF_Cat **)&space_cats); */
    saf_find_collections(pmode, &suite, SAF_SPACE_SLICE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM,
                         SAF_DECOMP_TORF, &num_space_cats, &space_cats);
    param_cats = NULL;
    /* saf_find_categories( suite, "param_slice_cat", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_param_cats, (SAF_Cat **)&param_cats); */
    saf_find_collections(pmode, &suite, SAF_PARAM_SLICE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM,
                         SAF_DECOMP_TORF, &num_param_cats, &param_cats);


    saf_find_collections(pmode, mesh_space, SAF_SPACE_SLICE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
			 &num_mesh_space_cats, NULL);



    if( num_mesh_space_cats < 1 ) {
      saf_declare_collection(pmode, mesh_space, space_cats+0, SAF_CELLTYPE_POINT, 1, SAF_1DC(1), SAF_DECOMP_FALSE);
    }
    mesh_space_cats = NULL;
    saf_find_collections(pmode, mesh_space, SAF_SPACE_SLICE, SAF_CELLTYPE_ANY, SAF_ANY_TOPODIM, SAF_DECOMP_TORF, \
			 &num_mesh_space_cats, &mesh_space_cats); 



    /* stategrp_role = saf_find_one_role( database, "stategroup_role"); */
/*
    stategrp_cats = NULL;
    saf_find_categories( suite, "stategroups", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_stategrp_cats, (SAF_Cat **)&stategrp_cats);
*/
    

    saf_describe_collection (pmode, &suite, space_cats+0, NULL, &coll_count, NULL, NULL, NULL);
    saf_describe_collection (pmode, &suite, param_cats+0, NULL, &param_count, NULL, NULL, NULL);

    add_count = 0;
    if (coll_count < state_index+1) {  /* assume just 1-d suite for this first implementation */
      add_count = state_index - coll_count + 1;
      saf_extend_collection (pmode, &suite, space_cats+0, add_count, SAF_1DC(add_count));

      if ( add_count > 1 )
	printf("WARNING: saf_write_state: extending space collection by more than 1\n");

    }

/*
    space_rels = NULL;
    saf_find_subset_relations(pmode, suite,  mesh_space, SAF_COMMON(space_cats[0]), &num_space_rels, &space_rels);
*/


/*
    if( state_index > num_space_rels ) {
      printf("state_index is greater than number of subset relations: %i,%i",state_index, num_space_rels);
    }
*/


    if( add_count > 0 ){
      suite_space_rels = (SAF_Rel *)malloc( add_count * sizeof(SAF_Rel));   
      for( i = 0 ; i < add_count ; i++ ) {

          saf_declare_subset_relation(pmode, &db, &suite, mesh_space, SAF_COMMON(space_cats+0), SAF_TUPLES, SAF_INT, NULL,
                                      H5I_INVALID_HID, NULL, &(suite_space_rels[i]));

	rel_buf[0] = coll_count + i; /* write the state_index to the subset relation */
	saf_write_subset_relation(pmode, suite_space_rels+i, H5T_NATIVE_INT, rel_buf, H5I_INVALID_HID, NULL, &db);
      }
    }
 

  }

  {

    /* the stategrp blob should contain two field handles:
         the first is the indirect coord field containing the mesh_coords, and param_coord
         the second is the state field containing the fields stored at this suite_index
    */
    index[0] = 0;
    stategrp_contents = NULL;
    saf_read_field (pmode, state_grp, NULL, 1, SAF_TUPLES, index, (void **)(&stategrp_contents));
    coord = &(stategrp_contents[0]);
    stategrp_state = &(stategrp_contents[1]);

    /* now read the coord field and get the mesh coord and the param coord (dump times) */
    coords = NULL;
    saf_read_field (pmode, coord, NULL, 1, SAF_TUPLES, index, (void **)(&coords));
    mesh_coord = &(coords[1]);
    param_coord = &(coords[0]);

    /* write the handle from the mesh_space default coord field to the mesh_coord indirect field */
    mesh_default_coord = (SAF_Field *)malloc(sizeof(SAF_Field));
    saf_find_default_coords(pmode, mesh_space, mesh_default_coord);
    if (SS_PERS_ISNULL(mesh_default_coord))
	SAF_ERROR(-1,_saf_errmsg("default coords are not defined on the mesh set"));
    index[0] = state_index;
    saf_write_field( pmode, mesh_coord, 1, SAF_TUPLES, index, 1, SAF_HANDLE, (void **)(&mesh_default_coord), &db);
    _saf_free(mesh_default_coord);

    /* write the param coord value (typically a dump time) to the param_coord field */
    /*
      coord contains the value of type coord_data_type.  coord and coord_data_type are supplied by the user 
       in this functions parameters
    */
    while( param_count < (state_index+1) ) {
      add_count = (state_index+1) - param_count;
      saf_extend_collection (pmode, &suite, param_cats+0, add_count, SAF_1DC(add_count));
      saf_describe_collection (pmode, &suite, param_cats+0, NULL, &param_count, NULL, NULL, NULL);
    }
    index[0] = state_index;
    saf_write_field( pmode, param_coord, 1, SAF_TUPLES, index, 1, coord_data_type, (void **)&coord_data, &db);

    /* write the field IDs for the dependent variables to the state */
    saf_write_field( pmode, stategrp_state, 1, SAF_TUPLES, index, 1, SAF_HANDLE, (void **)&fields, &db);

    /*
      that should be it, no need to write anything to the stategrp field itself since it simply contains
       the ids of the fields we're writing to.
    */
  }


	if( param_cats != NULL )
		free(param_cats);

	if( space_cats != NULL )
		free(space_cats);

	if( mesh_space_cats != NULL )
		free(mesh_space_cats);

	if( stategrp_cats != NULL )
		free(stategrp_cats);

	if( suite_space_rels != NULL )
		free( suite_space_rels );

	if( stategrp_contents != NULL )
		free( stategrp_contents);

	if( coords != NULL )
		free(coords);

	SAF_LEAVE(0);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	States
 * Purpose:	Retrieve a state
 *
 * Description:	Read all the elements of a state.  This includes the following:
 *
 *              * ID of the computational mesh (i.e., a set ID) associated with this state; 
 *              * ID of the default coordinate field of the mesh; 
 *              * the parametric value (e.g., the time value) associated with this state; 
 *              * IDs of all the fields (the dependent variables) attached to the mesh at this state.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_read_state(SAF_ParMode  pmode,		/* The parallel mode */
               SAF_StateGrp *state_grp,		/* The state group from which this state will be read. */
               int          state_index,	/* An index that specifies which state within the state group will be read.
                                                 * This index is 0-based. */
               SAF_Set      *mesh,              /* OUT: Returned ID of the mesh associated with this state. */
               SAF_Field    *deflt_coords,      /* OUT: Returned ID of the default coordinate field of MESH; we may want to
                                                 * delete this argument since the client can call saf_find_default_coords()
                                                 * for MESH. */
               void         *coord_data,	/* OUT: Returned coordinate of STATE_INDEX within the state group.  For
                                                 * instance, this is typically the time value of the state. */
	       SAF_Field    **fields		/* The IDs of the fields (the dependent variables) to be read from this state.
                                                 * The caller may supply a pointer to a value of NULL if this function is to 
                                                 * allocate a buffer.  If the caller supplies a pointer to a non-nil pointer, 
                                                 * then it is the responsibility of the caller to ensure that the buffer is of 
                                                 * sufficient size to contain the coordinates.  This size (NUM_FIELDS) is the 
                                                 * number of field templates (NUM_FTMPLS) obtained by a call to 
                                                 * saf_describe_state_tmpl().*/
               )
{
  SAF_ENTER(saf_read_state, SAF_PRECONDITION_ERROR);
  
  SAF_Field *coord;
  SAF_Set suite;
  int index[1];
  SAF_Db db=SS_FILE_NULL;
  SAF_Field *coords,*tmp_deflt_coords;
  SAF_Field *tmp_fields;
  SAF_Field *mesh_coord, *param_coord;
  SAF_Field *stategrp_state;
  SAF_FieldTmpl stategrp_tmpl;
  size_t group_size;
  hid_t group_type;
  SAF_Field *stategrp_contents;

  /* find the suite associated with this stategroup */
  saf_describe_field(pmode, state_grp, &stategrp_tmpl, NULL, &suite, 
		     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		     NULL, NULL, NULL, NULL);

  saf_get_count_and_type_for_field(pmode, state_grp, NULL, &group_size, &group_type );

  if( group_size != 2 ) {
    printf("size of stategrp field != 2\n");
  }


  /* get and set the database and db handles */
  ss_pers_file((ss_pers_t*)&suite, &db);

  {

 
    /* the stategrp blob should contain two field handles:
         the first is the indirect coord field containing the mesh_coords, and param_coord
         the second is the state field containing the fields stored at this suite_index
    */
    stategrp_contents = NULL;
    index[0] = 0;
    saf_read_field (pmode, state_grp, NULL, 1, SAF_TUPLES, index, (void **)(&stategrp_contents));
    coord = &(stategrp_contents[0]);
    stategrp_state = &(stategrp_contents[1]);
   
    /* now read the coord field and get the mesh coord and the param coord (dump times) */
    coords = NULL;
    saf_read_field (pmode, coord, NULL, 1, SAF_TUPLES, index, (void **)(&coords));
    mesh_coord = &(coords[1]);
    param_coord = &(coords[0]);

    if( deflt_coords != NULL ) {
      	tmp_deflt_coords = NULL;
      	saf_read_field( pmode, mesh_coord, NULL, 1, SAF_TUPLES, &state_index, (void **)&tmp_deflt_coords); 
      	*deflt_coords = *tmp_deflt_coords;
    }

    /* get the mesh set that the mesh_default_coord lives on */
    if( mesh != NULL ) {
      saf_describe_field( pmode, deflt_coords, NULL, NULL, mesh, 
			NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL);
    }
    
    if( coord_data != NULL ) {
        saf_read_field(pmode, param_coord, NULL, 1, SAF_TUPLES, &state_index, (void **)&coord_data);
    }

    if( fields != NULL ) {
	if( *fields == NULL ) {
    		tmp_fields = NULL;
    		saf_read_field( pmode, stategrp_state, NULL, 1, SAF_TUPLES, &state_index, (void **)&tmp_fields);
		*fields = tmp_fields;
	}
	else {
		tmp_fields = *fields;
    		saf_read_field( pmode, stategrp_state, NULL, 1, SAF_TUPLES, &state_index, (void **)&tmp_fields);
	}
    }


  }
  
    free(coords);
    free(stategrp_contents);

    SAF_LEAVE(0);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	States
 * Purpose:	Find state groups
 *
 * Description:	This finds the state groups that match specified criteria.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_state_groups(SAF_ParMode  pmode,		/* The parallel mode. */
                      SAF_Suite    *suite,		/* The suite within which to search. */
		      const char   *name,		/* The name of the state group for which to search.  Pass SAF_ANY_NAME 
                                                         * if you do not want to limit your search. */
		      int          *num_state_grps,	/* OUT: Returned number of state groups found. */
		      SAF_StateGrp **state_grps 	/* OUT: Returned state groups found. */
		      )
{
  SAF_ENTER(saf_find_state_groups, SAF_PRECONDITION_ERROR);
  SAF_Db db=SS_FILE_NULL;
  /*SAF_Role stategrp_role;*/

  int found = 0;
  SAF_Cat stategrp_cat;

  /* get and set the database and db handles */
  ss_pers_file((ss_pers_t*)suite, &db);

  /* stategrp_role = saf_find_one_role( database, "stategroup_role" ); */

  /* stategrp_cats = NULL; */
  /* saf_find_categories( suite, "stategroups", SAF_ANY_ROLE, SAF_ANY_TOPODIM, &num_cats, &stategrp_cats); */
  {
    int l_numFound = 0, i;
    SAF_Cat *l_collsFound = NULL;
    saf_find_collections(pmode, suite, SAF_TOPOLOGY, SAF_CELLTYPE_ANY,
                         SAF_ANY_TOPODIM, SAF_DECOMP_TORF,
                         &l_numFound, &l_collsFound);

    for(i=0;i<l_numFound;i++) {
        char *l_name=0;
        saf_describe_category(SAF_ALL, l_collsFound+i, &l_name, NULL, NULL );
        if( !strcmp(l_name,"stategroups") ) {
            found=1;
            stategrp_cat = l_collsFound[i];
            _saf_free(l_name);
            break;
        }
        _saf_free(l_name);
    }
    if( l_collsFound != NULL )
	free(l_collsFound);

  }


  if( found == 0 ) {
      if (num_state_grps != NULL)
          *num_state_grps = 0;
      if (state_grps != NULL)
          *state_grps = NULL;
      return SAF_SUCCESS;
  }
  else {

      saf_find_fields( pmode, &db, suite, name, NULL, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_UNIT,
                       &stategrp_cat, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, num_state_grps, state_grps);
  }

  SAF_LEAVE(SAF_SUCCESS);
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	States	
 * Purpose:    	Attach an attribute to a state group
 *
 * Description: This function is identical to the generic saf_put_attribute() function except that it is specific to
 *    		SAF_StateGrp objects to provide the client with compile time type checking. For a description,
 *		see saf_put_attribute().
 *
 * Programmer:  Mark Miller, LLNL, 2000-08-30
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_put_state_grp_att(SAF_ParMode pmode, SAF_StateGrp *state_grp, const char *key, hid_t type, int count, const void *value)
{
  SAF_ENTER(saf_put_state_grp_att, SAF_PRECONDITION_ERROR);
  int retval = saf_put_attribute(pmode, (ss_pers_t*)state_grp, key, type, count, value);
  SAF_LEAVE(retval);
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	States	
 * Purpose:    	Get an attribute attached to a state group
 *
 * Description: This function is identical to the generic saf_get_attribute() function except that it is specific to
 *    		SAF_StateGrp objects to provide the client with compile time type checking. For a description,
 *		see saf_get_attribute().
 *
 * Modifications:
 *		Mark Miller, LLNL, 2000-08-30 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_state_grp_att(SAF_ParMode pmode, SAF_StateGrp *state_grp, const char *key, hid_t *type, int *count, void **value)
{
  SAF_ENTER(saf_get_state_grp_att, SAF_PRECONDITION_ERROR);
  int retval = saf_get_attribute(pmode, (ss_pers_t*)&state_grp, key, type, count, value);
  SAF_LEAVE(retval);
}
