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
 * Chapter:     Fields
 * Description:
 * A field is some physical phenomenon known or expected to have /value/ at every point in the set over which the field
 * is defined. In other words, a field represents some continuous (as opposed to discrete) function which is defined over
 * the infinite point set specified as the /base/space/ for the field.
 *
 * In SAF, we divide the notion of a field into two pieces; a wholly abstract piece free of the details of how a field is
 * is /implemented/, and an implementation specific piece. The abstract piece is called a /field/template/. See
 * saf_declare_field_tmpl() for more information. Essentially, a field template defines a /class/ of fields. The implementation
 * specific piece of a field is called, simply, a /field/.
 *
 * Presently, SAF requires the client to create a new scientific modeling primitive (e.g. a field object) for each instance
 * of a field's data. For example, if you have a pressure field that is evolving with time, each time the field's data is 
 * written to the database, the client needs to declare a new field object. The client is *not* simply writing more data for
 * the same field object. As more experience is gained with the data model and implementation, this behavior will be modified
 * to be more natural. It is ok for a client to simply create a field with the same name, etc. This will not cause any problems
 * in SAF. For example, if a client creates several pressure fields, all of which are instances of the same field at different
 * points in time, that is ok. However, the client will probably want to organize those fields into a more aggregate
 * /field/of/fields/ (e.g. a field whose "values" are other fields on other base spaces). We call such a field an /indirect/ field.
 * In fact, the /states/and/suites/ interface is provided as a conventient way to construct an indirect field representing
 * various states of the problem output by the client. See [States] or saf_declare_state_group(). However, there are a variety of
 * situations in which a client may want to define an indirect field. The remaining portions of this chapter introduction
 * discuss these situations in some detail. We'll begin with some definitions.
 *
 * *Degrees*Of*Freedom*(*dofs*): The /degrees/of/freedom/ or /dofs/ of a field is the name we give to the data associated with
 * the field. Typically, the dofs are the problem sized arrays of floats or doubles representing some independent (or dependent)
 * variables of a simulation. We call these datums /degrees/of/freedom/ because, within the context of SAF, they are the
 * degrees of freedom *in*the*representation* of the field. It is important to recognize this context of
 * /the/representation/of/ the field. That is what SAF is solely concerned with: representing fields so that other clients
 * can read and interpret them. In this context, every datum represents a degree of freedom. This sense of degree of freedom
 * should *not* be confused with, for example, similar terminology in the linear system of equations a client might be solving.
 * That is an entirely different context in which similar terminology is used to describe those datums that effect the solution
 * of the system of equations being solved. SAF is concerned with data that effects the representation of the field.
 * Why don't we call these /values/? Because the word "values" implies that the field is, in fact, equal to these numbers for
 * some (maybe many) points in the base-space. And, this is only true when the field's evaluation function is /interpolating/.
 * That is, the interpolation functions *pass*through* the dofs controlling the interpolation. This is most certainly not true for
 * a field represented by, for example, a Fourier series.
 *
 * *Indirect*Field*: An /indirect/field/ is any field whose algebraic type is SAF_ALGTYPE_FIELD. Equivalently, this means that
 * if you were to call saf_read_field() for such a field, you would obtain a bunch of SAF_Field field handles. Likewise, when
 * the algebraic type of a field is *not* SAF_ALGTYPE_FIELD, the field is not an indirect field and we, instead, call it a
 * /direct/field/. Note that indirection is, in general, recursive. An indirect field can refer to fields that are themselves
 * indirect fields. An example of an indirect field is the pressure over a mesh as a function of time for 9 time instances.
 * There would be 9 instances of pressure fields on the mesh, one for each time instant. Each of these fields is just one
 * of the instances of the pressure on the mesh. To characterize the pressure field's variation over time, we would define
 * another field on the time base space having 9 dofs. Each dof would be a different one of the pressure fields over the
 * mesh as illustrated in figure "indirect field-1.gif".
 *
 * [figure indirect_field-1.gif]
 *
 * *Homogeneous*Field*: A /homogeneous/field/ is any field whose /defining/characteristics/ *do*not*vary* over the base space upon
 * which the field is declared. We include in "/defining/characteristics/" all those parameters used to declare a field and its
 * field template such as algebraic type, number of components, quantity, units, component interleave, component order, evaluation
 * function and even its storage.
 *
 * Any field that is not homogeneous is /inhomogeneous/. An example of an inhomogeneous field is
 * a stress tensor defined over a 3D rocket body and its 2D fins. Over the 3D body, the field is a 3D symmetric tensor and over
 * the 2D fins it is a 2D symmetric tensor. This is illustrated in "indirect field-2.gif".
 *
 * [figure indirect_field-2.gif]
 *
 * Another example is a coordinate field of a mesh whose storage is decomposed into separate chunks, one for each processor in a
 * parallel decomposition. This is illustrated in "indirect field-3.gif".
 *
 * [figure indirect_field-3.gif]
 *
 * SAF deals with inhomogeneous fields by breaking them up, recursively in general, into homogeneous pieces. Thus, the data
 * for an inhomogeneous field is the handles to these field pieces. An inhomogeneous field is, therefore, also an indirect field.
 * Furthermore, if a field is inhomogeneous, all bets are off about *any* of the field's /defining/characteristics/. All that can
 * be said, for sure, about an inhomogeneous field is that there is some decomposing collection of the field's base-space upon
 * which it is /presumably/ piecewise homogeneous. We say /presumably/ here because any piece of an inhomogeneous field can itself
 * be inhomogeneous so that, in general, its decomposition into homogeneous pieces is recursive.
 *
 * With all of this information, we can construct a /pseudo/class-hierarchy/ for these various kinds of fields.
 *
 *                                            Field
 *                                              |
 *                             ---------------------------------
 *                             |                               |
 *                         INdirect                          Direct
 *                             |                               |
 *                   -------------------                       |
 *                   |                 |                       | 
 *            INhomogeneous       Homogeneous              Homogeneous
 *                                     |
 *                                     |
 *                                     |
 *                                 State Field
 *
 * Because an inhomogeneous field is also an indirect field, it is often convenient when talking about both inhomogeneous indirect
 * fields and homogeneous indirect fields to simply refer to the two as inhomogeneous and indirect fields, respectively. There are
 * some important conceptual differences between inhomogeneous and indirect fields worth mentioning here.
 *
 * First, one requirement of the various fields comprising an inhomogeneous field is that the union of the base-spaces of all
 * the homogeneous pieces *must* form a decomposition of the base-space of the inhomogeneous aggregate. SAF enforces
 * this condition and will not permit a client to construct an inhomogeneous field for which this is not true.
 *
 * Second, it does not make sense to conceive of interpolating between the pieces of an inhomogeneous field in the same way we
 * might want to interpolate between the pieces of an indirect field. For example, it doesn't make sense to try to interpolate
 * between the stress tensor on the fins and the stress tensor on the rocket body of the inhomogeneous field in
 * "indirect field-2.gif" while it does make sense to try to interpolate between the 4th and 5th instances of the pressure field
 * in "indirect field-1.gif".
 *
 * Third, for all homogeneous fields, the number of dofs read from and written to a field is the product of the number
 * of components in the field, the size of the collection the field's dofs are associated with and the association ratio
 * (see saf_declare_field()). This is true for homogeneous, direct fields where the dofs might be floats or doubles as well as
 * homogeneous, indirect fields where the dofs are handles to other fields. However, for inhomogenous fields, the number of
 * field handles to be read and written is determined by the size of the decomposing collection upon which the field is presumably
 * piecewise homogeneous. That collection is what determines the number of pieces the field is decomposed into.
 *
 * *FIELD*TARGETING*: SAF now offers some limited ability to /transform/ a field during read. Currently, this capability is
 * available *only*during*read*. Transformations during write will be made available later. Currently, on read, a client
 * can invoke the following transformations:
 *    a. changes in precision (single<-->double)
 *    b. changes in interleave (vector<-->component)
 *    c. changes in storage decomposition (self<-->other immediate). By /immediate/ we mean a decomposition which is
 *       immediately below the self set in the subset relation graph.
 * The targeting function, saf_target_field(), is used to tell SAF to invoke such transforms during read. The intent is that a
 * reader will call saf_target_field() before making a call to read the field. The target call will indicate the intended form
 * of the field in the destination. Once targeting has been setup with a call to saf_target_field(), a call to
 * saf_read_field() will /do/the/right/thing/ resulting in the altered field in the destination's buffers.
 * This is an experimental capability and interface. Field targeting will only work on serial SAF clients or single processor
 * parallel SAF clients (i.e. SAF clients that have opened the database on just one processor).
 *
 * Soon, SAF will offer some limited ability to /transform/ a field during read or write.
 * The intent is that a reader or writer will call saf_target_field() before making a call to
 * read or write the field. The target call will indicate the intended form of the field in the destination (the database during
 * write or the client's memory during read). Once targeting has been setup with a call to saf_target_field(), a call to
 * saf_read_field() or saf_write_field() will /do/the/right/thing/ resulting in the altered field in the destination's buffers.
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Declare a field
 *
 * Description:	This function is used to declare a field. A field is some physical quantity known to have /value/ at every point
 *		in the infinite set of points that serves as the field's base space. That is, a field is some continuous (as
 *		opposed to discrete) quantity that exists and has value /everywhere/ over the base space the field is defined
 *		on - that is, at every point in the infinite set of points that is the field's base space.
 *
 *		We apologize for the large number of arguments in this function call. We have developed prototype interfaces
 *		that reduce this complexity significantly but introduce other issues. As more experience is gained with this
 *		software and data model, we'll have a better idea how to proceed.
 *
 *		In SAF, the description of a field is done in two parts; a field template (see saf_declare_field_tmpl()) and
 *		an instance of a field. The field template object describes all the abstract information about a field. The field
 *		object itself describes the implementation details of an instance of a field. For example, the field template
 *		object describes the abstract quantity of measure the field represents, such as length (see Quantities) while
 *		the field object describes specific units of measure for that quantity such as meters (see Units).
 *
 *		You will notice that the base space upon which the field is defined is *not* part of the field object. Instead
 *		it is part of the field template object. This allows the field template object to classify fields according
 *		to which pieces of the base space they are defined on.
 *
 *		In the hierarchy of sets that serve as candidate base spaces for fields, the idea is to declare a field on
 *		the top-most set in the hierarchy which contains all points the field is defined on but contains no points
 *		the field is *not* defined on. Such a set is also called the /maximal/ set of the field. It could also be
 *		thought of as the region of support of the field.
 *
 *		There is a *big* difference between /declaring/
 *		a field that is identically zero over portions of a set and /declaring/ the field only over the
 *		subset(s) for which it is non-zero. The former indicates that the field is known everywhere on the set and
 *		is zero in some places. The latter indicates that the field is known on the subset(s) and /undefined/ (e.g.
 *		does not exist) anywhere else.
 *
 *		At present, SAF really does not do much to interpret the data or descriptive information for a field. Currently,
 *		SAF simply allows a writer client to describe the salient features of a field and a reader client to discover
 *		them. As SAF evolves, SAF will be able to interpret more and more about the field itself.
 *
 * Return:      Returns a handle to the set on success (either the one passed in by the FLD argument or one allocated herein);
 *              returns NULL on failure.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Field *
saf_declare_field(SAF_ParMode pmode,            /* The parallel mode. */
                  SAF_Db *db,                   /* The database where the new field will be created. */
                  SAF_FieldTmpl *ftmpl,         /* The field template handle for this field. Recall that the field template
                                                 * describes the abstract features of the field, including the quantity the
                                                 * field represents, and the algebraic type. The field being created in this
                                                 * saf_declare_field() call is simply an instance of the abstract field
                                                 * characterized by the field template passed as this argument. */
                  const char *name,             /* The name of this field. If a writer client declares different fields with
                                                 * the same name, a reader client that searches for fields by name will find
                                                 * multiple matches. However, it is ok to declare different fields with the
                                                 * same name. */
                  SAF_Set *base_space,          /* The base_space of this field */
                  SAF_Unit *unit,               /* The specific units of measure. If in the field template, the quantity was
                                                 * not specified, then the only valid value that can be passed for units is
                                                 * SAF_NOT_APPLICABLE_UNIT. Otherwise, pass SAF_NOT_SET_UNIT if you do not want
                                                 * to specify units for the field or pass one of the valid units of
                                                 * measure. */
                  SAF_Cat *homog_decomp,        /* If the field is homogeneous, enter SAF_SELF() here. Otherwise, the field is
                                                 * inhomogenous and this argument must indicate a decomposing collection of the
                                                 * field's base-space upon which it is /presumably/ homogeneous. We say
                                                 * /presumably/ because it is not a *requirement* that the field be
                                                 * homogeneous on each of the members of the collection indentified here. The
                                                 * field pieces defined on any one or all of those members can, in turn, also
                                                 * be inhomogeneous. The only requirement is that the collection identified
                                                 * here be a decomposition of the associated set and that, ultimately, the
                                                 * recursion of defining inhomogeneous fields in terms of other inhomogeneous
                                                 * fields terminates on a bunch of homogeneous pieces. A common use of this
                                                 * argument is to indicate that the field is broken into independent chunks of
                                                 * storage (either within a single processor or distributed across other
                                                 * processors). In fact, prior to SAF-1.2.1, that was all this argument was
                                                 * used for and documented as supporting. Any collections contained in the
                                                 * base space set for which the IS_DECOMP argument in the
                                                 * saf_declare_collection() call was SAF_DECOMP_TRUE, can be passed
                                                 * here. See the chapter introduction for fields for further information (see
                                                 * Fields). */
                  SAF_Cat *coeff_assoc,         /* This argument identifies the category of a collection in the base space set
                                                 * which the field's coefficients are n:1 associated with.  For example, for a
                                                 * field whose coefficients are 1:1 with a collection of a category
                                                 * representing the nodes, you would identify that collection category with
                                                 * this argument. Likewise, for a field whose coefficients are 4:1 with a
                                                 * collection of a category representing the elements in the problem, you
                                                 * would identify that collection with this argument. Note, if the
                                                 * coefficients are associated with the base space itself, and not the members
                                                 * of a collection in the base-space set, you would pass SAF_SELF() for
                                                 * this argument. */
                  int assoc_ratio,              /* This argument specifies the /n/ in the n:1 association described above. For
                                                 * example, if for every member of the collection representing the elements,
                                                 * you have 1 coefficient, then this value would be 1. This value is always
                                                 * non-negative. */
                  SAF_Cat *eval_cat,            /* This argument specifies the collection whose members represent the pieces in
                                                 * the piecewise evaluation of the field.  If there is only a single piece
                                                 * (e.g. the whole base space), then pass SAF_SELF().  For example, a
                                                 * collection category identifying the nodes for the COEFF_ASSOC argument and
                                                 * an ASSOC_RATIO of 1 indicates only that we have 1 coefficient for each
                                                 * member of the collection of nodes. It does not indicate which collection
                                                 * in the base space (for example the elements), the field is actually
                                                 * piecewise evaluated on. */
                  SAF_Eval *eval_func,          /* This argument identifies one of several evaluation functions currently known
                                                 * to SAF. Again, SAF does not yet actually evaluate a field. It only stores
                                                 * the descriptive information to support its evaluation. See definition of
                                                 * SAF_EvalFunc enum for the possible values. Also, we have provided some
                                                 * convenience macros for this and COEFF_ASSOC, ASSOC_RATIO, and EVAL_CAT
                                                 * arguments for common cases; /node/ /centered/ and /zone/ /centered/
                                                 * fields. Pass SAF_NODAL() for a node centered field, SAF_ZONAL() for a zone
                                                 * centered field, SAF_DECOMP for a field that is piecewise constant over
                                                 * some /decomposing/ collection (e.g. domains) or SAF_CONSTANT() for a
                                                 * constant field. */
                  hid_t data_type,              /* The type of data in BUFS if BUFS are provided. */
                  SAF_Field *comp_flds,         /* Array of component field handles.  Pass null only if there are no
                                                 * components to this field (the field is a scalar field). */           
                  SAF_Interleave comp_intlv,    /* The particular fashion in which components are interleaved.  Currently there
                                                 * are really only two: SAF_INTERLEAVE_VECTOR and SAF_INTERLEAVE_COMPONENT.
                                                 * These represent the XYZXYZ...XYZ and the XXX...XYYY...YZZZ...Z cases.  Note
                                                 * that interleave really only deals within a single blob of storage.  In the
                                                 * case of a composite field whose coefficients are stored independently on
                                                 * the component fields, interleave really has no meaning (use
                                                 * SAF_INTERLEAVE_INDEPENDENT).  Interleave only has meaning on fields with
                                                 * storage.  In the case of a scalar field interleave is also meaningless,
                                                 * both cases degenerate to the same layout: XXX...X (use
                                                 * SAF_INTERLEAVE_NONE). */
                  int *comp_order,              /* Only relevant for fields with component fields.  This value indicates the
                                                 * order of the fields in the COMP_FLDS relative to the registered
                                                 * order. Pass NULL if the permutation is the identity. */
                  void **bufs,                  /* The field data buffers. Pass NULL if you would rather provide this on the
                                                 * write call.  Note that the number and size of buffers (if any) is specified
                                                 * by the interleave and number of components.  If the field has vector
                                                 * interleave then there may only be 1 buffer, if the field has component
                                                 * interleave then there must be num_components buffers.  The number of
                                                 * components is defined in the field template specified by FTMPL. */
                  SAF_Field *fld                /* OUT: The optional returned field handle. If NULL is passed here then this
                                                 * function allocates the field handle before returning it. */
                  )
{
    SAF_ENTER(saf_declare_field, NULL);
    ss_scope_t          scope=SS_SCOPE_NULL;
    SAF_Algebraic       algebraic=SS_ALGEBRAIC_NULL;
    int                 i, count, nbufs, buf_size=0;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);

    SAF_REQUIRE(SS_SET(base_space), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("BASE_SPACE must be a valid set handle"));
    SAF_REQUIRE(name, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("NAME must be non-null"));
    SAF_REQUIRE(SS_FIELDTMPL(ftmpl), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("FTMPL must be a valid field template"));
    SAF_REQUIRE(_saf_is_self_decomp(homog_decomp) || SS_CAT(homog_decomp), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("STORAGE_DECOMP must be either SELF_DECOMP or a valid cat handle"));
    SAF_REQUIRE(_saf_is_self_decomp(coeff_assoc) || SS_CAT(coeff_assoc), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("COEFF_ASSOC must be a valid cat handle"));
    SAF_REQUIRE(_saf_is_self_decomp(eval_cat) || SS_CAT(eval_cat), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("EVAL_CAT must be a valid cat handle"));
    SAF_REQUIRE(!unit || SS_UNIT(unit), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("UNIT must be a valid unit handle if supplied"));
    SAF_REQUIRE(_saf_is_valid_units(unit, ftmpl), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("UNIT must agree with quantity defined on field template"));
    SAF_REQUIRE(assoc_ratio >= 0, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("ASSOC_RATIO must be non-negative"));
    SAF_REQUIRE(!eval_func || SS_EVALUATION(eval_func), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("EVAL_FUNC must be a valid evaluation type handle if supplied"));
    SAF_REQUIRE(true, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("units of component fields must match units of composite field"));

    /* Get the algebraic type for the SAF_ASSERT() below */
    algebraic = SS_FIELDTMPL(ftmpl)->algebraic;

    /* Error checking */
    SAF_ASSERT(data_type<=0 ||
               (SS_ALGEBRAIC(&algebraic)->indirect && H5Tequal(data_type,SAF_HANDLE)) ||
               SAF_XOR(_saf_is_self_decomp(homog_decomp), H5Tequal(data_type,SAF_HANDLE)),
               SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("STORAGE_DECOMP=(SAF_SELF,!SAF_SELF) ==> DATA_TYPE=(!SAF_HANDLE,SAF_HANDLE) or "
                           "ALG_TYPE is SAF_ALGTYPE_FIELD"));
    SAF_ASSERT(_saf_is_self_decomp(homog_decomp) || SS_ALGEBRAIC(&algebraic)->indirect, 
               SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("algebraic type must be SAF_ALGTYPE_FIELD if field is inhomogeneous"));
    SAF_ASSERT((!(SS_ALGEBRAIC(&algebraic)->indirect) || !comp_flds), SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("component fields cannot be supplied if algebraic type is SAF_ALGTYPE_FIELD"));
    SAF_ASSERT((SS_FIELDTMPL(ftmpl)->num_comps == 1 && comp_intlv == SAF_INTERLEAVE_NONE) ||
               (1 < SS_FIELDTMPL(ftmpl)->num_comps && (comp_intlv == SAF_INTERLEAVE_VECTOR ||
                                                        comp_intlv == SAF_INTERLEAVE_COMPONENT ||
                                                        comp_intlv == SAF_INTERLEAVE_INDEPENDENT)) ||
               SS_FIELDTMPL(ftmpl)->num_comps == SAF_NOT_APPLICABLE_INT,
               SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("component interleave must be appropriate for number of component fields"));
    SAF_ASSERT(SS_ALGEBRAIC(&algebraic)->indirect ||
               (SS_FIELDTMPL(ftmpl)->num_comps==1 && !comp_flds) ||
               (SS_FIELDTMPL(ftmpl)->num_comps>1  &&  comp_flds),
               SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("must supply component fields when there are 2 or more component expected"));
    SAF_ASSERT(comp_order==NULL || SS_FIELDTMPL(ftmpl)->num_comps>1,
               SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("COMP_ORDER is relevent only when there is more than 1 component"));
    SAF_ASSERT(comp_order==SAF_IDENTITY || _saf_is_permutation(SS_FIELDTMPL(ftmpl)->num_comps,comp_order),
               SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("if specified, COMP_ORDER must a valid permutation vector"));

    /* check buffer pointers */
    SAF_ASSERT_BEGIN(SAF_LOW_CHK_COST) {
        ok = TRUE;
        if (bufs && comp_intlv == SAF_INTERLEAVE_INDEPENDENT) {
	    for (i=0; i<SS_FIELDTMPL(ftmpl)->num_comps && ok; i++) {
                if (!bufs[i]) ok = FALSE;
            }
        }
    } SAF_ASSERT_END(NULL, _saf_errmsg("BUFS must point to NCOMPS valid (i.e., non-null) pointers"));

    /* Verify that comp_intlv==SAF_INTERLEAVE_VECTOR if the field is not a state field with more than one component on an
     * extendible base space. */
    SAF_ASSERT(!SS_SET(base_space)->is_extendible || SS_ALGEBRAIC(&algebraic)->indirect ||
               SS_FIELDTMPL(ftmpl)->num_comps==1 || comp_intlv==SAF_INTERLEAVE_VECTOR,
               SAF_LOW_CHK_COST, NULL,
               _saf_errmsg("only VECTOR interleaving is allowed for fields (not state fields) "
                           "with more than one component defined on extendible base spaces"));

    /* Create the new field object */
    ss_file_topscope(db, &scope);
    fld = (ss_field_t*)ss_pers_new(&scope, SS_MAGIC(ss_field_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)fld, NULL);
    if (SAF_EACH==pmode) SS_PERS_UNIQUE(fld);
    
    /* if the eval collection category (eval_cat) is not SAF_SELF and non-primitive, then make sure eval_func is PWCONSTANT */
    /* This looks like a library limitation to me, so I'm coding it to look for an evaluation function whose name is
     * "piecewise constant". If this is some fundamental limitation of the model then we should really add some property to
     * the Evaluation table to describe this. --rpm 2001-04-26 */
    /* If we allow SAF_SPACE_PWCONST then we must certainly allow SAF_SPACE_CONSTANT.  I think we should remove
     * this restriction alltogether.  mjo 2002-04-03 */
    if (!_saf_is_self_decomp(eval_cat)) {
        ss_collection_t eval_coll;
        SAF_Eval *pwconst = SAF_SPACE_PWCONST;
        SAF_Eval *constant = SAF_SPACE_CONSTANT;

        if (NULL==_saf_getCollection_set(base_space, eval_cat, &eval_coll))
            SAF_ERROR(NULL, _saf_errmsg("unable to obtain collection for category\n"));
        SAF_ASSERT(SS_COLLECTION(&eval_coll)->cell_type!=SAF_CELLTYPE_SET ||
                   (SS_COLLECTION(&eval_coll)->cell_type==SAF_CELLTYPE_SET && 
                    (SAF_EQUIV(eval_func, pwconst) ||  SAF_EQUIV(eval_func, constant))),
                   SAF_LOW_CHK_COST, NULL,
                   _saf_errmsg("only SAF_SPACE_PWCONST is valid on non-primitive collections for a participating process"));
    }

    /*  If this is a multi-component (non-scalar) field, but NOT a field of fields (i.e., ALG_TYPE!=SAF_ALGTYPE_FIELD), 
     *  then we must set-up the association from this field to the components and then deal with any ordering... */
    if (SS_FIELDTMPL(ftmpl)->num_comps>1 && !SS_ALGEBRAIC(&algebraic)->indirect) {
        /* Save the supplied field links */
        ss_array_resize(SS_FIELD_P(fld,comp_fields), (size_t)SS_FIELDTMPL_M(ftmpl,num_comps));
        ss_array_put(SS_FIELD_P(fld,comp_fields), ss_pers_tm, 0, SS_NOSIZE, comp_flds);
        
        /* Now if the caller supplied an ordering permutation vector the we'll save it. */
        if (comp_order) {
            ss_array_target(SS_FIELD_P(fld,comp_order), H5T_NATIVE_INT);
            ss_array_resize(SS_FIELD_P(fld,comp_order), (size_t)SS_FIELDTMPL_M(ftmpl,num_comps));
            ss_array_put(SS_FIELD_P(fld,comp_order), H5T_NATIVE_INT, 0, SS_NOSIZE, comp_order);
        }
    }

    /* Additional initialization of the new field object */
    ss_string_set(SS_FIELD_P(fld,name), name);
    SS_FIELD(fld)->base_space = *base_space;
    SS_FIELD(fld)->ftmpl = *ftmpl;
    if (unit) SS_FIELD(fld)->units = *unit;
    SS_FIELD(fld)->storage_decomp_cat = *homog_decomp;
    SS_FIELD(fld)->comp_intlv = comp_intlv;
    SS_FIELD(fld)->dof_assoc_cat = *coeff_assoc;
    SS_FIELD(fld)->assoc_ratio = assoc_ratio;
    SS_FIELD(fld)->eval_decomp_cat = *eval_cat;
    if (eval_func) SS_FIELD(fld)->evaluation = *eval_func;
    SS_FIELD(fld)->is_homogeneous = _saf_is_self_decomp(homog_decomp);

    /* Compute the number of and size of each buffer (if any).
     *   
     * Note that the buffer size depends on the number of size of the collection category and the association ratio which gives
     * the number of coefficients.  Multiplying this by the number of components gives the number of "numbers" of the given
     * datatype (float, double, ... ) Fields with interleave NONE have no components (are scalar) and fields with interleave
     * INDEPENDENT often have components but often have no buffers since the storage is on the components, we'll treat this as
     * vector. */
    if (_saf_is_self_decomp(homog_decomp)) {
        /* if coeff_assoc is SAF_SELF, then we have only 1 member of the collection. That is the set itself.
	    Otherwise, we need to get the collection to get its count */
        if (!_saf_is_self_decomp(coeff_assoc)) {
            ss_collection_t assoc_coll;
            _saf_getCollection_set(base_space, coeff_assoc, &assoc_coll);
            count = SS_COLLECTION(&assoc_coll)->count;
        } else {
	    count = 1;
        }
        switch (comp_intlv) {
        case SAF_INTERLEAVE_NONE:
            nbufs    = 1;
            buf_size = count * assoc_ratio;
            break;
        case SAF_INTERLEAVE_VECTOR:
            nbufs    = 1;
            assert(SS_FIELDTMPL(ftmpl)->num_comps>=0);
            buf_size = count * assoc_ratio * SS_FIELDTMPL(ftmpl)->num_comps;
            break;
        case SAF_INTERLEAVE_COMPONENT:
            assert(SS_FIELDTMPL(ftmpl)->num_comps>=0);
            nbufs    = SS_FIELDTMPL(ftmpl)->num_comps;
            buf_size = count * assoc_ratio;
            break;
        case SAF_INTERLEAVE_INDEPENDENT:
            nbufs    = 1;
            buf_size = count * assoc_ratio;
            break;
        default: 
            /* VBT_INTERLEAVE_ANY, VBT_INTERLEAVE_INVALID, VBT_INTERLEAVE_NA, VBT_INTERLEAVE_UNKNOWN are handled here: by doing
	     * nothing. */
            nbufs = 1;
            buf_size = count * assoc_ratio;
            break;
        }
    } else {
        ss_collection_t storage_coll;
        _saf_getCollection_set(base_space, homog_decomp, &storage_coll);
        nbufs = 1;
        buf_size = SS_COLLECTION(&storage_coll)->count;
    }

    /* stick buf, buf size and type onto field handle */
    SS_FIELD(fld)->m.nbufs     = nbufs;
    SS_FIELD(fld)->m.bufs      = bufs;
    SS_FIELD(fld)->m.buf_size  = buf_size;
    SS_FIELD(fld)->m.data_type = data_type;

    SAF_LEAVE(fld);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Declare default coordinates of a given set
 *
 * Description: Many fields might be suitable to serve as a coordinate field. Absolute coordinates and displacements are
 *		just two examples. This reference manual is not the appropriate place to go into the specific mathematical
 *		requirements for a field to serve as a coordinate field. However, recognizing that more than one field
 *		can serve as a coordinate field raises the issue, which field should be used as the coordinate field if
 *		nothing else is specified. This function declares which field ought to be treated as the default coordinates. 
 *
 *		Note that in order for a field to be declared as the default coordinate field for a set, the field must
 *		first be declared as a coordinate field.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_declare_default_coords(SAF_ParMode pmode, 	/* the parallel mode */
                           SAF_Set *base, 	/* the base space set whose default coordinates are being declared */
                           SAF_Field *field	/* the field to serve as the default coordinates */
                           )
{
    SAF_ENTER(saf_declare_default_coords, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(base), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("BASE must be a valid set handle"));
    SAF_REQUIRE(SS_FIELD(field), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FIELD must be a valid field handle"));

    SAF_ASSERT(SS_FIELD(field)->is_coord_field, SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
               _saf_errmsg("the specified field must be a coordinate field"));

    /* The field's base space must be `base' */
    SAF_ASSERT(SS_PERS_EQ(SS_FIELD_P(field,base_space), base), SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
               _saf_errmsg("the field to be used as default coord must be defined on BASE"));

    /* Set the default coordinate field on the set */
    SAF_DIRTY(base, pmode);
    SS_SET(base)->dflt_coordfld = *field;

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Declare a field as a candidate coordinate field
 *
 * Description: Use the function to indicate that a particular field is a coordinate field. This merely identifies a field
 *              as a candidate coordinate field. More than one field may serve as the coordinate field for a set. For example, in
 *              engineering codes, there are the deformed and un-deformed coordinates.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_declare_coords(SAF_ParMode pmode,   /* The parallel mode. */
                   SAF_Field *field     /* The field to be characterized as a coordinate field. */
                   )
{
    SAF_ENTER(saf_declare_coords, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    /* Set the is coord field for this field record to true */
    SAF_DIRTY(field, pmode);
    SS_FIELD(field)->is_coord_field = TRUE;

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Find coordinate fields
 *
 * Description: Use this function to find the coordinate fields of a set. In general, we allow for more than one coordinate
 *              field to be defined. For example, in engineering codes, there are the deformed and undeformed coordinates.
 *              Thus, this function can return multiple fields. Even so, there is only ever one field known as the *default*
 *              coordinate field for a set. This field is found with a call to saf_find_default_coords().
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_coords(SAF_ParMode pmode,              /* The parallel mode. */
                SAF_Db *db,                     /* Database in which to limit the search. */
                SAF_Set *base,                  /* The base space for which coordinate fields are desired. */
                int *num,                       /* For this and the succeeding argument [see Returned Handles]. */
                SAF_Field **found               /* For this and the preceding argument [see Returned Handles]. */
                )
{
    SAF_ENTER(saf_find_coords, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Field, key, mask);
    size_t              nfound;
    ss_scope_t          scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(base) || _saf_is_universe(base), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("BASE must be either a valid set handle or the universe set for all participating processes"));
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));

    ss_file_topscope(db, &scope);
    if (base && !_saf_is_universe(base)) SAF_SEARCH(SAF_Field, key, mask, base_space, *base);
    SAF_SEARCH(SAF_Field, key, mask, is_coord_field, TRUE);

    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_field_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, NULL, NULL);
        if (num) *num = nfound;
    } else {
        /* Find limited matches; client allocates result buffer */
        assert(num);
        nfound = *num;
        if (NULL==ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, (ss_pers_t*)*found,
                               _SAF_GLOBALS.find_detect_overflow)) {
            SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matchine objects"));
        }
        *num = nfound;
    }
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Find default coordinate fields
 *
 * Description: Use this function to find the default coordinate fields of a set. There is only ever one default coordinate field
 *              for a set.
 *
 * Return:      On success, returns a pointer to the default coordinate field. If the FIELD argument was supplied then it is
 *              filled in and becomes the return value, otherwise a new field link is allocated and returned.  Returns NULL on
 *              failure. If no default coordinate field has been assigned to the BASE set then a valid object link is returned
 *              but that link is nil (i.e., a call to SS_PERS_ISNULL() on the return value is true but the return value is not
 *              a NULL C pointer).
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Field *
saf_find_default_coords(SAF_ParMode pmode,      /* The parallel mode */
                        SAF_Set *base,          /* The set for which the default coordinate field is returned */
                        SAF_Field *field        /* OUT: The returned field handle, if found, otherwise SAF_NOT_SET_FIELD */
                        )
{
    SAF_ENTER(saf_find_default_coords, NULL);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);

    SAF_REQUIRE(SS_SET(base), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("BASE must be a valid set handle for participating processes"));

    if (!field && NULL==(field=malloc(sizeof *field)))
        SAF_ERROR(NULL, _saf_errmsg("unable to allocate return value"));
    *field = SS_SET(base)->dflt_coordfld;
    
    SAF_LEAVE(field);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Get a description of a field
 *
 * Description: NOT WRITTEN YET.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Modifications:
 *              Robb Matzke, LLNL, 2000-02-17
 *              Added documentation.
 *
 *              Jim Reus, 13Jul2000
 *              Added component interleave parameter.
 *              Removed number of buffers parameter (refer to number of
 *                 components and component interleave parameters).
 *              Re-ordered parameters to go somewhat with saf_declare_field
 *              Changed "component ordering" parameter from int* to int**
 *                 since it is now a permutation vector.
 *
 *              Jim Reus, 28Jul2000
 *              Changed interleave to an output parameter as it should be.
 *
 * 		Jim Reus, 31Jan2001
 *		- Changed some callocs to mallocs to avoid problem size init.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
int
saf_describe_field(SAF_ParMode pmode,           /* The parallel mode. */
                   SAF_Field *field,            /* The field handle. */
                   SAF_FieldTmpl *ftmpl,        /* OUT: The returned field template handle. Pass NULL if you do not want this
                                                 * value returned. */
                   char **name,                 /* OUT: The returned name of the field. Pass NULL if you do not want this
                                                 * value returned. (see Returned Strings). */
		   SAF_Set *base_space,         /* OUT: The returned base space of the field. Pass NULL if you do not want
                                                 * this value returned. */
                   SAF_Unit *unit,              /* OUT: The returned unit of measure. */
                   hbool_t *is_coord,           /* OUT: A returned boolean indicating if the field is a coordinate field. Pass
                                                 * NULL if you do not want this value returned. */
                   SAF_Cat *homog_decomp,       /* NULL: If the field is homogeneous, the value returned here, if requested,
                                                 * is always SAF_SELF(). That is, SAF_EQUIV(SAF_SELF(db), homog_decomp) will
                                                 * return true. Otherwise, it will return false, the field is inhomogeneous
                                                 * and this argument is the decomposition on which the field is presumably
						 * piecewise homogeneous. Pass NULL if you do not want this value returned. */
                   SAF_Cat *coeff_assoc,        /* OUT: The collection with which the field coefficients are associated in an
                                                 * n:1 relationship. Pass NULL if you do not want this value returned. */
                   int *assoc_ratio,            /* OUT: The `n' in the n:1 relationship described for the COEFF_ASSOC
                                                 * argument. Pass NULL if you do not want this value returned. */
                   SAF_Cat *eval_coll,          /* OUT: The collection whose sets decompose the base space set and over which
                                                 * the field is actually evaluated. Pass NULL if you do not want this value
                                                 * returned. */
                   SAF_Eval *eval_func,         /* OUT: The evaluation function. Pass NULL if you do not want this value
                                                 * returned. */
                   hid_t *data_type,            /* OUT: The file datatype of the field. Pass NULL if you do not want this value
                                                 * returned. The caller is responsible for invoking H5Tclose() when the
                                                 * datatype is no longer needed. A negative returned value indicates no
                                                 * known file datatype. */
                   int *num_comps,              /* OUT: The number of components in the field. Pass NULL if you do not want
                                                 * this value returned. */
                   SAF_Field **comp_flds,       /* OUT: The component fields. Pass NULL if you do not want this value returned. */
                   SAF_Interleave *comp_intlv,  /* OUT: The particular fashion in which components are interleaved.  Currently
                                                 * there are really only two: SAF_INTERLEAVE_VECTOR and SAF_INTERLEAVE_COMPONENT.
                                                 * These represent the XYZXYZ...XYZ and the XXX...XYYY...YZZZ...Z cases.  Note that
                                                 * interleave really only deals within a single blob of storage.  In the case of a
                                                 * composite field whose coefficients are stored independently on the component
                                                 * fields then interleave really has no meaning (use SAF_INTERLEAVE_INDEPENDENT).
                                                 * Interleave only has meaning on fields with storage.  In the case of a scalar
                                                 * field interleave is also meaningless, both cases degenerate to the same layout:
                                                 * XXX...X (use SAF_INTERLEAVE_NONE). */
                   int **comp_order             /* OUT: The component ordering in the field. Pass NULL if you do not want this
                                                 * value returned. */
                   )
{
    SAF_ENTER(saf_describe_field, SAF_PRECONDITION_ERROR);
    ss_blob_t           blob=SS_BLOB_NULL;
    ss_fieldtmpl_t      ftmpl_storage=SS_FIELDTMPL_NULL;
    int                 i, ncomps;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_FIELD(field), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FIELD must be a valid field handle for all participating processes"));

    /* fill in all the stuff we get from the field record itself */
    if (base_space) *base_space = SS_FIELD(field)->base_space;
    if (!ftmpl) ftmpl = &ftmpl_storage; /*because we need it below*/
    *ftmpl = SS_FIELD(field)->ftmpl;
    if (_saf_setupReturned_string(name, ss_string_ptr(SS_FIELD_P(field,name))))
        SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to return NAME for field"));
    if (unit) *unit = SS_FIELD(field)->units;
    if (is_coord) *is_coord = SS_FIELD(field)->is_coord_field;
    if (homog_decomp) *homog_decomp = SS_FIELD(field)->storage_decomp_cat;
    if (coeff_assoc) *coeff_assoc = SS_FIELD(field)->dof_assoc_cat;
    if (assoc_ratio) *assoc_ratio = SS_FIELD(field)->assoc_ratio;
    if (eval_coll) *eval_coll = SS_FIELD(field)->eval_decomp_cat;
    if (eval_func) *eval_func = SS_FIELD(field)->evaluation;
    ncomps = SS_FIELDTMPL(ftmpl)->num_comps;
    if (num_comps) *num_comps = ncomps;
    if (data_type) {
        blob = SS_FIELD(field)->dof_blob;
        if (SS_PERS_ISNULL(&blob)) {
            *data_type = H5I_INVALID_HID;
        } else if (ss_array_nelmts(SS_FIELD_P(field,indirect_fields))>0) {
            *data_type = H5Tcopy(ss_pers_tm);
        } else {
            if (ss_blob_bound_f(&blob, NULL, NULL, NULL, NULL, data_type)<0)
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to obtain file datatype for field DOFs"));
        }
    }
    if (comp_intlv) *comp_intlv = SS_FIELD(field)->comp_intlv;
    if (ncomps > 1) {
        /*  Ah, we have component fields, did the caller request any component fields? */
        if (comp_flds) {
            /*  Yes, then the caller must pass a pointer to a variable which either already holds or will hold the
             *  number of components. */
            if (num_comps == NULL)
                SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("you must provide NUM_COMPS if you ask for the"
                                                        " component fields"));
            /*  Is there already a buffer to hold the component field handles? */
            if (*comp_flds != NULL) {
                /* If the client has allocated storage for the component fields, make sure that there is enough. */
                if (*num_comps < (int)ncomps)
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("NUM_COMPS is too small for component field handles %i %i",
                                                            *num_comps, ncomps));
            } else {
                /*  The caller passed a pointer to a nil pointer so we'll allocate the buffer... */
                *comp_flds = calloc((size_t)ncomps, sizeof(**comp_flds));
                if (*comp_flds == NULL)
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate space for component field handles COMP_FLDS"));
            }

            if (NULL==ss_array_get(SS_FIELD_P(field,comp_fields), ss_pers_tm, 0, (size_t)ncomps, *comp_flds)) {
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to read component field array"));
            }
        }

        /*  If the client wants the component ordering then return it. */        
        if (comp_order) {
            if (num_comps == NULL)
                SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("you must provide NUM_COMPS if you ask for"
                                                        " the component order"));
            if (*comp_order != NULL) {
                /* If the client has allocated storage for the component order, make sure that there is enough. */
                if (*num_comps < (int)ncomps)
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("NUM_COMPS is too small for component order %i %i",
                                                            *num_comps, ncomps));
            } else {
                /* Attempt to allocate storage if the client hasn't already. */
                if (NULL==(*comp_order=malloc(ncomps*sizeof(**comp_order))))
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate comp_order array"));
            }

            /* If the component order was stored in a metablob then read it into the comp_order array */
            if (0==ss_array_nelmts(SS_FIELD_P(field,comp_order))) {
                for (i=0; i<ncomps; i++) (*comp_order)[i] = i;
            } else {
                if (NULL==ss_array_get(SS_FIELD_P(field,comp_order), H5T_NATIVE_INT, 0, (size_t)ncomps, *comp_order))
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to read component field order"));
            }
        }
    }
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Find fields
 *
 * Description: This function allows a client to search for fields in the database. The search may be limited by one or
 *		more criteria such as the name of the field, the quantity the field represents, the base space the field is
 *		defined on, etc., etc.
 *
 * Issue:	Should SAF traverse up the SIL to find all fields that are actually defined for the given set?
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_fields(SAF_ParMode pmode,              /* The parallel mode. */
                SAF_Db *db,                     /* Database in which to limit the search. */
                SAF_Set *base,                  /* The base space to limit the search to. Pass SAF_UNIVERSE() or NULL if you
                                                 * do not want to limit the search to any particular base space. */
                const char *name,               /* Limit search to fields with this name. Pass SAF_ANY_NAME if you do not want
                                                 * to limit the search. */
                SAF_Quantity *quantity,		/* Limit search to fields of specified quantity. Pass NULL to not limit search. */
                SAF_Algebraic *atype,           /* Limit the search to this algebraic type. Pass SAF_ALGTYPE_ANY if you do
                                                 * not want to limit the search. */
                SAF_Basis *basis,               /* Limit the search to this basis. Pass SAF_ANY_BASIS if you do not want to
                                                 * limit the search. */
                SAF_Unit *unit,                 /* Limit search to fields with these units. Pass SAF_ANY_UNIT to not limit
						 * search. */
                SAF_Cat *coeff_assoc,           /* Limit search. Pass SAF_ANY_CAT to not limit the search. */
                int assoc_ratio,                /* Limit search. Pass SAF_ANY_RATIO to not limit the search. */
                SAF_Cat *eval_decomp,           /* Limit search. Pass SAF_ANY_CAT to not limit the search. */
                SAF_Eval *eval_func,            /* Limit search. Pass SAF_ANY_EFUNC to not limit the search. */
                int *nfound,                    /* For this and the succeeding argument, (see Returned Handles). */
                SAF_Field **found            	/* For this and the preceding argument, (see Returned Handles). */
                )
{
    SAF_ENTER(saf_find_fields, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Field, key, mask);
    int                 nftmpls=0, n;
    SAF_FieldTmpl       *ftmpls=NULL;
    size_t              thisListN, nret=0, i;
    SAF_Field           *thisList=NULL, *retval=found?*found:NULL;
    ss_scope_t          scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    
    SAF_REQUIRE(!base || SS_SET(base), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("BASE must either be a valid set handle or the universe set if supplied"));
    SAF_REQUIRE(!quantity || SS_QUANTITY(quantity), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("QUANTITY must either be a valid quantity handle or SAF_ANY_QUANTITY"));
    SAF_REQUIRE(!atype || SS_ALGEBRAIC(atype), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ATYPE must be a valid algebraic type handle or SAF_ANY_ALGEBRAIC"));
    SAF_REQUIRE(!basis || SS_BASIS(basis), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("BASIS must be a valid basis handle or SAF_ANY_BASIS"));
    SAF_REQUIRE(!unit || SS_UNIT(unit), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must either be a valid unit handle or SAF_ANY_UNIT"));
    SAF_REQUIRE(!eval_func || SS_EVALUATION(eval_func), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("EVAL_FUNC must be a valid evaluation function handle or SAF_ANY_EVALUATION"));
    SAF_REQUIRE(!coeff_assoc || SS_CAT(coeff_assoc), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("COEFF_ASSOC must either be a valid cat handle or SAF_ANY_CAT"));
    SAF_REQUIRE(!eval_decomp || SS_CAT(eval_decomp), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("EVAL_DECOMP must either be a valid cat handle or SAF_ANY_CAT"));
    SAF_REQUIRE(_saf_valid_memhints(nfound, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NFOUND and FOUND must be compatible for return value allocation"));

    /* Where do we search? */
    ss_file_topscope(db, &scope);

    /* First find all the templates that match the search criteria, then loop over those templates to find the matching
     * fields. */
    if (atype || basis || quantity) {
        saf_find_field_tmpls(pmode, db, NULL, atype, basis, quantity, &nftmpls, &ftmpls);
        /* We're done early if the search criteria yielded no matching field template. */
        if (0==nftmpls) {
            if (found) *nfound = 0;
            SAF_RETURN(SAF_SUCCESS);
        }
    }

    /* Initialize the key/mask for the field search */
    if (name) SAF_SEARCH_S(SAF_Field, key, mask, name, name);
    if (coeff_assoc) SAF_SEARCH(SAF_Field, key, mask, dof_assoc_cat, *coeff_assoc);
    if (SAF_ANY_INT!=assoc_ratio) SAF_SEARCH(SAF_Field, key, mask, assoc_ratio, assoc_ratio);
    if (eval_decomp) SAF_SEARCH(SAF_Field, key, mask, eval_decomp_cat, *eval_decomp);
    if (base && !_saf_is_universe(base)) SAF_SEARCH(SAF_Field, key, mask, base_space, *base);
    if (unit) SAF_SEARCH(SAF_Field, key, mask, units, *unit);
    if (eval_func) SAF_SEARCH(SAF_Field, key, mask, evaluation, *eval_func);

    /*  Now, loop over the number of field templates doing finds on the fields */
    for (n=0; n<MAX(1, nftmpls); n++) {
        /* Fill in the field template id for this pass if we have any */
        if (ftmpls) SAF_SEARCH(SAF_Field, key, mask, ftmpl, ftmpls[n]);

        /* Find fields matching key/mask */
        thisList = SS_PERS_FIND(&scope, key, mask_count?&mask:NULL, SS_NOSIZE, thisListN);
        
        /* Append these fields to the return value */
        if (!found) {
            /* Count the matches */
            assert(nfound);
            nret += thisListN;
        } else if (!*found) {
            /* Find all matches; library allocates results */
            if (NULL==(retval=realloc(retval, MAX(nret+thisListN,1)*sizeof(*retval))))
                SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate space for fields"));
            for (i=0; i<thisListN; i++)
                retval[nret++] = thisList[i];
        } else {
            /* Find limited matches; client allocates result buffer */
            assert(nfound);
            if ((int)(nret+thisListN)>*nfound) 
                SAF_ERROR(SAF_CONSTRAINT_ERROR, _saf_errmsg("found too many matching objects"));
            for (i=0; i<thisListN; i++)
                retval[nret++] = thisList[i];
        }

        thisList = SS_FREE(thisList);
    }

    /* Return values */
    if (nfound) *nfound = nret;
    if (found) *found = retval;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Is field stored on self
 *
 * Description: This function is used by a client to test if a field is stored on self or on a decomposition.  The boolean
 *              result is returned by reference.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_is_self_stored_field(SAF_ParMode pmode,     /* The parallel mode. */
                         SAF_Field *field,      /* The handle of the field which is to be examined. */
                         hbool_t *result        /* OUT: Optional pointer to memory which is to receive the result of the test:
                                                 * true if the field is self stored or false if it is stored on a
                                                 * decomposition. */
                         )
{
    SAF_ENTER(saf_is_self_stored_field, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_FIELD(field), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FIELD must be a valid field handle"));

    /* When a field is stored on "self" then we store actual field coefficient values.  When a field is not stored on "self"
     * then DOFs are stored on the subsets specified by the decomposition.  In this case the field values of this "parent"
     * field are the handles to the "actual" fields found on each of the subsets. */
    if (_saf_is_self_decomp(SS_FIELD_P(field,storage_decomp_cat))) {
        if (result) *result = TRUE;
    } else {
        if (result) *result = FALSE;
    }

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Write the data for a field
 *
 * Description:	This function is used to write a field's data. If the field is *not* an indirect reference to other fields,
 * 		this call involves *real* disk I/O. All functions in SAF with either "read" or "write" in the name potentially
 *		involve real disk I/O.
 *
 *		This function allows a client to write either the entire field's data or a portion of the field's data. Recall
 *		that the /degrees/of/freedom/ (dofs) of a field are n:1 associated with the members of some collection in
 *		the set upon which the field is defined. We call this collection the /associated/collection/.
 * 
 *		In order to specify a partial request, the client is required to
 *		specify which members of the associated collection it is writing the dofs for. Ultimately, those members may be
 *		specified using a N dimensional hyperslab (or hypersample) or an arbitrary list of N-tuples. In either case,
 *		the number of dimensions, 'N', is the number of indexing dimensions in the associated collection.
 *
 *		At present, there are several limitations. First, the collection must be 1 dimensionally indexed. Next,
 *		only the hyperslab mode or a single member in tuple-mode are supported, not hypersamples and not an
 *		arbitrary list. Finally, if the field is a multi-component field, then the only supported interleave mode is
 *		SAF_INTERLEAVE_VECTOR.
 *
 *		For indirect fields, the notion of writing on the composite or component fields is lost. On an indirect,
 *		composite field, the values written must be handles to other composite fields. Likewise for its component
 *		fields. The values written must be handles for other component fields.
 *
 *		Finally, we provide as a convenience the macro SAF_WHOLE_FIELD which expands to a comma separated list of
 *		values, 0, SAF_TOTALITY, NULL, for the three arguments MEMBER_COUNT, REQ_TYPE, MEMBER_IDS for the case in
 *		which the client is writing the whole field in this call.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issues:	A partial I/O request looks a lot like a subset relation. In fact, we even use the same data type, SAF_SRType,
 *		to identify the type of partial I/O request. It may be difficult for a client to distinguish between making
 *		a partial I/O request and making real subsets. In theory, there *really* should be no difference. The act
 *		of writing a portion of a field *is* the act of defining a subset of the base space the field is defined on
 *		and then restricting the field to that subset. In the current implementation, this requires, at a minimum, the
 *		ability to create transient objects such as the subset representing the piece of the field being written in
 *		this call. In addition, it *really* requires decoupling the storage containers into which field's data goes
 *		from declaring and writing fields.
 *
 *		For a compound data-type on a composite field, we probably ought to confirm a) the DSL_rankOf the compound
 *		type is equal to the number of components, b) the type of each member of the compound type is equal to the
 *		type of each of the component fields (assuming both are ordered the same), and c) the names of the member
 *		types are the same as the component fields. Currently we are only checking a).
 *
 *		Computing the actual size of the I/O request here is NO SMALL TASK. It depends on a combination of factors
 *		including the number of buffers, the number of members whose dofs are being written, the association ratio,
 *		the data-type and whether the field is direct or indirect.
 *
 * Parallel:    SAF_EACH mode is a collective call where each of the N tasks provides a unique relation. SAF will create a
 *              single HDF5 dataset to hold all the data and will create N blobs to point into nonoverlapping regions in that
 *              dataset. In SAF_EACH mode the call must still be collective across the FILE communicator (or the communicator
 *              of the dataset to which FIELD belongs if FILE is null). This requirement is due to the fact that an HDF5
 *              dataset may need to be created and such an operation is collective.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_write_field(SAF_ParMode pmode,      /* The parallel mode. */
                SAF_Field *field,       /* The field to write. */
                int member_count,	/* A count of the number of members of the collection in which the field's dofs are
                                         * n:1 associated that are actually being written in this call. This value is
                                         * ignored if you are writing the entire field's dofs in this call (i.e., REQ_TYPE is
                                         * SAF_TOTALITY). Also note that as a convenience, we provide the macro
                                         * SAF_WHOLE_FIELD which expands to a comma separated list of appropriate values for
                                         * this argument and the next two, for the case in which the whole field is being
                                         * written in this call. */
                SAF_RelRep *req_type,	/* The type of I/O request. We use a relation representation type here to specify the
                                         * type of the partial request because it captures the necessary information. Pass
                                         * SAF_HSLAB if you are writing the dofs of a partial hyperslab of the members of the
                                         * associated collection. In this case, MEMBER_IDS points to 3 N-tuples of starts,
                                         * counts and strides of the hyperslab (hypersample) request. Pass SAF_TUPLES, if you
                                         * are writing the dofs for an arbitrary list of members of the associated collection.
                                         * In this case, the MEMBER_IDS points to a list of N-tuples. In both cases, 'N' is
                                         * the number of indexing dimensions in the associated collection. Finally, pass
                                         * SAF_TOTALITY if you are writing the entire field's set of dofs. */
                int *member_ids,	/* Depending on the value of REQ_TYPE, this argument points to 3 N-tuples storing,
                                         * respectively, the starts, counts and strides *in*each*dimension* of the associated
                                         * collection or to a list of MEMBER_COUNT N-tuples, each one identifying a single
                                         * member of the associated collection or to NULL in the case of a SAF_TOTALITY
                                         * request. */
                int nbufs,		/* The number of buffers. Valid values are either 1 or a value equal to the number of
                                         * components of the field. A value greater than 1 indicates that the field is stored
                                         * component by component, one buffer for each component. Note, however, that current
                                         * limitations of partial requests support only fields that are interleaved by
                                         * SAF_INTERLEAVE_VECTOR. This, in turn, means that in a partial I/O request, NBUFS
                                         * can only ever be one. */
                hid_t buf_type,         /* The type of the objects in the buffer(s). If the buffer datatype was provided in
                                         * the saf_declare_field() call that produced the field handle then this parameter
                                         * should have a negative value. If however the datatype was not provided in the
                                         * saf_declare_field() or if the handle was the result of a find operation then the
                                         * datatype must be provided in this call. */
                void **bufs,		/* The buffers. */
                SAF_Db *file            /* Optional file into which the data is written. If none is supplied then the data is
                                         * written to the same file as the FIELD. */
                )
{
    SAF_ENTER(saf_write_field, SAF_PRECONDITION_ERROR);
    double              timer_start=0;
    SAF_FieldTmpl       ftmpl=SS_FIELDTMPL_NULL;
    SAF_Set             base=SS_SET_NULL;
    SAF_Algebraic       algebraic=SS_ALGEBRAIC_NULL;
    int                 buf_size;
    ss_collection_t     coll=SS_COLLECTION_NULL;
    ss_blob_t           dof_blob;       /* The blob that holds the dofs being written. */
    hsize_t             ndofs=1;        /* Total number of dofs to be written. Start at one and multiply it up. */
    hsize_t             my_blob_offset; /* Offset of first item of this task's data in the dof blob. */
    hsize_t             offset;         /* Offset of this task's data in the dof blob for each buffer in turn. */
    SAF_Field           *fields=NULL;   /* One-dimensional array of fields representing all fields from BUFS arrays. */
    hsize_t             my_blob_size;   /* Current size of blob based on what elements tasks are writing (used for creation). */
    hbool_t             should_write;   /* True if this task should call ss_blob_write(). */
    int                 bufno;          /* Counter over the BUFS. */
    ss_scope_t          scope;          /* The scope in which to create the new blob. */
    MPI_Comm            scope_comm;     /* The communicator for `scope' */
    int                 scope_self;     /* MPI task rank of calling task in scope_comm. */
    unsigned            flags;          /* Bit flags for blob operations */
    size_t              cur_ndofs;      /* Number of dofs currently in the indirect_fields array */
    
    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    
    SAF_REQUIRE(SS_FIELD(field), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FIELD must be a valid field handle"));
    SAF_REQUIRE(SAF_XOR(SS_FIELD(field)->m.bufs, bufs), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("BUFS must be specified here or in the saf_declare_field() call (not both)"));
    SAF_REQUIRE(!SAF_XOR(nbufs, bufs), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("pass either valid BUFS and NBUFS>0 or NULL and NBUFS==0"));
    SAF_REQUIRE(_saf_is_valid_io_request(pmode, field, member_count, req_type, member_ids, nbufs),
                SAF_HIGH_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if partial I/O request, collection must be 1D indexed, REQ_TYPE must be SAF_HSLAB "
                            "or a single (e.g. MEMBER_COUNT=1) SAF_TUPLE and field's interleave, if multi-component, "
                            "must be SAF_INTERLEAVE_VECTOR"));
    SAF_REQUIRE((SS_FIELD(field)->m.data_type>0 || buf_type>0), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("buffer datatype must be specified in field declaration or write"));
    SAF_REQUIRE(SS_FIELD(field)->m.data_type<=0 || buf_type<=0 || H5Tequal(SS_FIELD(field)->m.data_type, buf_type), 
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("buffer datatype must be consistent between field declaration and write"));

    /* Start the timer */
    if (_SAF_GLOBALS.p.TraceTimes)
        timer_start = _saf_wall_clock(FALSE);

    /* Where sould a new blob be created if we have to do that? */
    ss_pers_scope(file?(ss_pers_t*)file:(ss_pers_t*)field, &scope);
    ss_scope_comm(&scope, &scope_comm, &scope_self, NULL);

    /* Copy links to local variables for convenience. */
    ftmpl = SS_FIELD(field)->ftmpl;
    base = SS_FIELD(field)->base_space;
    algebraic = SS_FIELDTMPL(&ftmpl)->algebraic;

    /* If data was supplied in the saf_declare_field() call then use that data instead */
    if (SS_FIELD(field)->m.bufs) {
        bufs = SS_FIELD(field)->m.bufs;
        nbufs = SS_FIELD(field)->m.nbufs;
    }
    if (SS_FIELD(field)->m.data_type>0)
        buf_type = SS_FIELD(field)->m.data_type;
    SAF_ASSERT(nbufs==1 || nbufs==SS_FIELDTMPL(&ftmpl)->num_comps, SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
               _saf_errmsg("NBUFS %i and ftmpl.num_comps %i not consistent", nbufs, SS_FIELDTMPL(&ftmpl)->num_comps));

    /* Check that all supplied buffer pointers in BUF are non-null */
    SAF_ASSERT_BEGIN(SAF_LOW_CHK_COST) {
        int i;
        ok = TRUE;
        if (bufs && nbufs>1) {
            for (i=0; i<nbufs && ok; i++) {
                if (!bufs[i]) ok = FALSE;
            }
        }
    } SAF_ASSERT_END(SAF_ASSERTION_ERROR, _saf_errmsg("BUFS must point to NBUFS valid (e.g. non-NULL) pointers"));

    /* The only cases in which the data can be links to other fields is when the storage decomposition is not self or when the
     * algebraic type is indirect (i.e., SAF_FIELD) */
    SAF_ASSERT_BEGIN(SAF_LOW_CHK_COST) {
        if (H5Tequal(buf_type, ss_pers_tm)) {
            ok = (SS_ALGEBRAIC(&algebraic)->indirect || !_saf_is_self_decomp(SS_FIELD_P(field,storage_decomp_cat)));
        } else {
            ok = (!SS_ALGEBRAIC(&algebraic)->indirect && _saf_is_self_decomp(SS_FIELD_P(field,storage_decomp_cat)));
        }
    } SAF_ASSERT_END(SAF_ASSERTION_ERROR,
                     _saf_errmsg("Data can be field links only if the algebraic type is indirect (SAF_FIELD) or the storage "
                                 "decomponsition is not SAF_SELF."));

    /* If we're writing the whole field then ignore what the user passed in for MEMBER_COUNT and instead look at either the
     * field's dof_assoc_cat or, for an indirect field, its storage_decomp_cat. */
    if (SS_RELREP(req_type)->id==SAF_TOTALITY_ID) {
        if (!_saf_is_self_decomp(SS_FIELD_P(field,dof_assoc_cat))) {
            if (_saf_is_self_decomp(SS_FIELD_P(field,storage_decomp_cat))) {
                _saf_getCollection_set(&base, SS_FIELD_P(field,dof_assoc_cat), &coll);
            } else {
                _saf_getCollection_set(&base, SS_FIELD_P(field,storage_decomp_cat), &coll);
            }
            SAF_ASSERT(!SS_PERS_ISNULL(&coll), SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
                       _saf_errmsg("_saf_getCollection_set failed"));
            member_count = SS_COLLECTION(&coll)->count;
        } else {
            member_count = 1;
        }
    }
    ndofs *= member_count;

    /* Multiply by the field's association ratio */
    ndofs *= SS_FIELD(field)->assoc_ratio;

    /* Now multiply by the field template's number of components. */
    if (_saf_is_primitive_type(buf_type)) {
        assert(SS_FIELDTMPL(&ftmpl)->num_comps>=0);
        ndofs *= SS_FIELDTMPL(&ftmpl)->num_comps;
    } else if (H5Tequal(buf_type, ss_pers_tm)) {
        if (_saf_is_self_decomp(SS_FIELD_P(field,storage_decomp_cat))) {
            assert(SS_FIELDTMPL(&ftmpl)->num_comps>=0);
            ndofs *= SS_FIELDTMPL(&ftmpl)->num_comps;
        }
    } else {
#ifdef SSLIB_SUPPORT_PENDING
        SAF_ASSERT((ftmplr.num_comps == DSL_rankOf_type(buf_type)), SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
                   _saf_errmsg("for a non-primitive datatype, the type must have a rank equal to the number of components "
                               "of the field"));
#endif /*SSLIB_SUPPORT_PENDING*/
        SAF_ASSERT(nbufs==1, SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
                   _saf_errmsg("for a non-primitive datatype, their must be only one buffer"));
        SAF_ASSERT(SS_FIELD(field)->comp_intlv == SAF_INTERLEAVE_VECTOR, SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
                   _saf_errmsg("for a non-primitive datatype, the interleave must be SAF_INTERLEAVE_VECTOR"));
    }

    /* Number of dofs per buffer */
    SAF_ASSERT(0==ndofs % nbufs, SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
               _saf_errmsg("NBUFS, %d, must evenly divide into size, %d", nbufs, ndofs));
    buf_size = (int)ndofs / nbufs;



    /* If the data is field handles (i.e., the algebric type is indirect (SAF_FIELD) or the storage decomponsition is not
     * SAF_SELF), then store those field handles in a variable length array in the field. We use a variable length array
     * because it's able to handle the conversion from memory to file representation of persistent object links and is well
     * suited for the small array of field links. */
    if (H5Tequal(buf_type, ss_pers_tm)) {
        /* Make sure that we haven't already stored data that isn't field links. */
        SAF_ASSERT(SS_PERS_ISNULL(SS_FIELD_P(field,dof_blob)), SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
                   _saf_errmsg("field already has data written in a DOF blob"));
        
        /* Convert the separate field arrays into a single array */
        if (NULL==(fields=_saf_field_handles_1d(nbufs, (SAF_Field**)bufs, buf_size)))
            SAF_ERROR(-1, _saf_errmsg("unable to convert handles to 1d array"));

        /* Where does the data land in the variable length array? */
        offset = SAF_TOTALITY_ID==SS_RELREP(req_type)->id ? 0 : member_ids[0];

        /* We adjust the offset for the case of a state field because all component handles are compressed into one buf, not
         * NBUFS blobs. */
        assert(0==offset || SS_FIELDTMPL(&ftmpl)->num_comps>=0);
        offset *= SS_FIELDTMPL(&ftmpl)->num_comps;

        /* ISSUE: Is it possible that a SAF_EACH call will have a different offset and data for each task? If so we'll have to
         *        do some communicating first otherwise ss_file_synchronize() will see that each task made incompatible
         *        modifications to this object. This code just checks that for now. [rpm 2004-06-07] */
        {
            int taskno, ntasks=ss_mpi_comm_size(scope_comm);
            unsigned long *all_offsets = malloc(ntasks*sizeof(*all_offsets));
            all_offsets[scope_self] = offset;
            ss_mpi_allgather(all_offsets, 1, MPI_UNSIGNED_LONG, scope_comm);
            for (taskno=0; taskno<ntasks; taskno++) {
                if (all_offsets[taskno]!=all_offsets[scope_self]) {
                    SAF_ERROR(-1, _saf_errmsg("offset[task=%d]=%lu; offset[task=%d]=%lu\n",
                                              taskno, all_offsets[taskno], scope_self, all_offsets[scope_self]));
                }
            }
            SS_FREE(all_offsets);
        }

        /* Insert the field handles into the array and free the buffer, extending the array if necessary */
        SAF_DIRTY(field, pmode);
        cur_ndofs = ss_array_nelmts(SS_FIELD_P(field,indirect_fields));
        if (cur_ndofs<offset+ndofs)
            ss_array_resize(SS_FIELD_P(field,indirect_fields), (size_t)(offset+ndofs));
        ss_array_put(SS_FIELD_P(field,indirect_fields), ss_pers_tm, (size_t)offset, (size_t)ndofs, fields);
        fields = SS_FREE(fields);

        goto done;
    }

    /* We can't get here without passing the valid_io_request pre-condition and all the limitations it currently imposes. So,
     * we know member_ids is either an array of 3 ints {start, count, stride} where stride is constrained to 1 for SAF_HSLAB or
     * an array of 1 int {index} for SAF_TUPLES. Regardless, member_ids[0] is the starting position and member_count is the
     * size of the request. */

    /* Where will each task's contribution land in the blob? We call this `my_blob_offset'. */
    if (SAF_ALL==pmode) {
        if (SAF_TOTALITY_ID==SS_RELREP(req_type)->id) {
            /* Every task is providing all the data. member_ids is probably null. */
            my_blob_size = ndofs;
            my_blob_offset = 0;
            should_write = (0==scope_self);
        } else {
            /* All tasks are providing identical data destined for identical locations in the blob. */
            my_blob_size = member_ids[0] + ndofs;
            my_blob_offset = member_ids[0];
            should_write = (0==scope_self);
        }
    } else {
        if (SAF_TOTALITY_ID==SS_RELREP(req_type)->id) {
            my_blob_size = ndofs;
            my_blob_offset = 0;
            should_write = TRUE;
        } else {
            /* Each task is providing some data (possibly none) at it's own base offset. */
            my_blob_size = member_ids[0] + ndofs;
            my_blob_offset = member_ids[0];
            should_write = TRUE;
        }
    }

    /* Create or extend the blob(s) and underlying dataset. */
    dof_blob = SS_FIELD(field)->dof_blob;
    if (SS_PERS_ISNULL(&dof_blob)) {
        /* Create the blobs if they don't exist yet. */
        if (NULL==ss_blob_new(&scope, SAF_ALL==pmode?SS_ALLSAME:0U, &dof_blob))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create field dof blob"));
        
        /* Temporarily bind some memory just so we can create the dataset */
        if (ss_blob_bind_m1(&dof_blob, (void*)1, buf_type, my_blob_size)<0)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot bind memory to field dof blob"));
        flags = (SAF_ALL==pmode?SS_ALLSAME:SS_BLOB_EACH) |
                (SS_SET(&base)->is_extendible?SS_BLOB_EXTEND:0U);
        if (ss_blob_mkstorage(&dof_blob, NULL, flags, NULL)<0)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot create field dof blob dataset"));
        if (ss_blob_bind_m1(&dof_blob, NULL, -1, (hsize_t)0)<0)
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot unbind memory from field dof blob"));

        /* Save the new blob pointer in the field */
        SAF_DIRTY(field,pmode);
        SS_FIELD(field)->dof_blob = dof_blob;
    } else if (SS_SET(SS_FIELD_P(field,base_space))->is_extendible) {
        /* The ss_blob_write1() below is independent so we need to extend the dataset here if necessary */
        if (SAF_ALL!=pmode)
            SAF_ERROR(SAF_NOTIMPL_ERROR, _saf_errmsg("extending in SAF_EACH mode is not implemented yet"));
        ss_blob_extend1(&dof_blob, my_blob_size, SS_ALLSAME, NULL);
    }
    
    /* Write all buffers to the blob. Since each task may have a different number of buffers we have to use independent I/O */
    if (should_write) {
        for (bufno=0; bufno<nbufs; bufno++) {
            ss_blob_bind_m1(&dof_blob, bufs[bufno], buf_type, (hsize_t)buf_size);
            offset = my_blob_offset + bufno*buf_size;
            ss_blob_write1(&dof_blob, offset, (hsize_t)buf_size, SS_BLOB_UNBIND, NULL);
        }
    }

done:
    SS_FIELD(field)->m.bufs = NULL;
    SS_FIELD(field)->m.nbufs = 0;
    SS_FIELD(field)->m.buf_size = 0;
#if 0 /* Do not clear this one: the declared datatype should stick around */
    SS_FIELD(field)->m.data_type = 0;
#endif

    /* Time accounting */
    if (_SAF_GLOBALS.p.TraceTimes)
        _SAF_GLOBALS.CummWriteTime += (_saf_wall_clock(false) - timer_start);

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Does field have data
 *
 * Description: This function is used to check if a given field has a valid blob id (which it would if it has had data
 *              written to it and doesn't if it has not).
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_data_has_been_written_to_field(SAF_ParMode pmode,   /* The parallel mode. */
				   SAF_Field *field,    /* The field handle. */
				   hbool_t *Presult     /* OUT: A pointer to caller supplied memory which is to receive the answer
                                                         * to the question.  A value of true is saved at this location if the
                                                         * field has had data written to it, false if not. */
                                   )
{
    SAF_ENTER(saf_data_has_been_written_to_field, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_FIELD(field), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FIELD must be a valid field handle for all participating processes"));

    if (SS_PERS_ISNULL(SS_FIELD_P(field,dof_blob)) && 0==ss_array_nelmts(SS_FIELD_P(field,indirect_fields))) {
        if (Presult) *Presult = FALSE;
    } else {
        if (Presult) *Presult = TRUE;
    }

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Queries whether data has been written
 *
 * Description: Does a composite or component field have written data corresponding to this field.
 *              
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Modifications:
 *              Jake S Jones, 20may2002
 *              Initial version.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_data_has_been_written_to_comp_field(SAF_ParMode pmode,      /* The parallel mode. */
					SAF_Field *field,       /* The field handle. */
					hbool_t *Presult        /* OUT: A pointer to caller supplied memory which is to receive the
                                                                 * answer to the question.  A value of true is saved at this
                                                                 * location if the field has had data written to it, false if
								 * not. */
					)
{
    SAF_ENTER(saf_data_has_been_written_to_comp_field, SAF_PRECONDITION_ERROR);
    hbool_t l_succeeded = FALSE;
    SAF_Field *l_componentFields=NULL, parentField;
    int l_numComponents=0;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_FIELD(field), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FIELD must be a valid field handle for all participating processes"));


    saf_describe_field(pmode, field, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, 
                       &l_numComponents, &l_componentFields, NULL, NULL);

    _saf_find_parent_field(pmode, field, &parentField);

    if (l_componentFields && l_numComponents) {
        int i;
        for (i=0; i<l_numComponents; i++) {
            /* There wont be an endless loop as long as saf_data_has_been_written_to_field doesnt call
	     * saf_data_has_been_written_to_comp_field */
            saf_data_has_been_written_to_field(pmode, l_componentFields+i, &l_succeeded);
            if (!l_succeeded) break;
        }
    } else if (!SAF_EQUIV(field,&parentField)) {
        saf_data_has_been_written_to_field( pmode, &parentField, &l_succeeded);
    }

    if (Presult) *Presult = l_succeeded;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Fields
 * Purpose:     Obtain the number of components of a field
 *
 * Description: This function is used to retrieve the number of components of a field.  This routine assumes that the
 *              field is an indirect field.  It gets the number of components by recursively traversing the first
 *              indirect field handle of the field.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_numberOfComponentsOf_field(SAF_ParMode pmode,      /* The parallel mode. */
				SAF_Field *field,       /* The field handle. */
				int *num_comps          /* OUT: The number of components. */
                                )
{
    SAF_ENTER(_saf_numberOfComponentsOf_field, SAF_PRECONDITION_ERROR);
    SAF_Field           *Ibuf=NULL;
    size_t              Icount=0;
    SAF_FieldTmpl       ftmpl;

    saf_get_count_and_type_for_field(pmode, field, NULL, &Icount, NULL);
    if (Icount == 0)
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("no indirect field handles"));

    Ibuf = malloc(Icount*sizeof(*Ibuf));
    if (Ibuf == NULL)
        SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate buffer to recieve field row numbers"));
    saf_read_field(pmode, field, NULL, SAF_WHOLE_FIELD, (void**)&Ibuf);

    /* We'll now get the number of components of the first field on the decomposition to determine the number to return. */
    ftmpl = SS_FIELD(Ibuf)->ftmpl;
    if (SS_FIELDTMPL(&ftmpl)->num_comps == -1) {
        _saf_numberOfComponentsOf_field(pmode, Ibuf, num_comps);
    } else {
        *num_comps = SS_FIELDTMPL(&ftmpl)->num_comps;
    }

    free(Ibuf);
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Get datatype and size for a field
 *
 * Description: This function is used to retrieve the number and type of items that would be retrieved by a call to the
 *              saf_read_field() function.  This function may be used by the caller to determine the size of the buffer
 *              needed when pre-allocation is desired or to determine how to traverse the buffer returned by the
 *              saf_read_field() function.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issues:      Fields stored on a decomposition must have same datatype. It may be possible to relax this a bit.
 *              Also what if the field has been decomposed into blocks? say triangles and quads, field remapping
 *              may be possible but makes no sense as the DOFs would be all mixed-up some for triangles, some for
 *              quads.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int

saf_get_count_and_type_for_field(SAF_ParMode pmode,     /* The parallel mode. */
                                 SAF_Field *field,      /* The field handle. */
                                 SAF_FieldTarget *target, /* Optional field targeting information. */
                                 size_t *Pcount,        /* OUT: The number of items that would be placed in the buffer by a
                                                         * call to the saf_read_field() function.  The caller may pass a value
                                                         * of NULL for this parameter if this value is not desired. */
                                 hid_t *Ptype           /* OUT: The type of the items that would be placed in the buffer by a
                                                         * call to the saf_read_field() function.  The caller may pass a value
                                                         * of NULL for this parameter if this value is not desired. The
                                                         * returned HDF5 datatype can be closed by the caller when no longer
                                                         * needed. */
                                 )
{
    SAF_ENTER(saf_get_count_and_type_for_field, SAF_PRECONDITION_ERROR);
    ss_scope_t          scope=SS_SCOPE_NULL;            /* Scope containing FIELD */
    ss_fieldtmpl_t      ftmpl=SS_FIELDTMPL_NULL;        /* Field template for FIELD */
    ss_set_t            basespace=SS_SET_NULL;          /* Base space of FIELD */
    ss_cat_t            dof_assoc_cat=SS_CAT_NULL;      /* Cached from FIELD */
    int                 scope_size;                     /* Size of the communicator for `scope' */
    hsize_t             count;                          /* Needed because PCOUNT is not the right type */
    hid_t               ftype;                          /* File datatype */
    hbool_t             desireHandles;                  /* Should we read field links or the pointed-to-field's dofs? */
    ss_field_t          *ifields=NULL;                  /* Indirect fields */
    size_t              icount=0;                       /* Number of indirect fields */
    ss_field_t          *comps=NULL;                    /* Component fields */
    int                 num_comps;                      /* Number of component fields */
    static SAF_FieldTarget ft_zero;                     /* Default field targeting */
    int                 numberOfComponents;
    int                 collSize;
    
    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    ss_pers_scope((ss_pers_t*)field, &scope);
    ss_scope_comm(&scope, NULL, NULL, &scope_size);
    if (!target) target = &ft_zero;

    SAF_REQUIRE(SS_FIELD(field), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FIELD must be a valid field handle"));
    SAF_REQUIRE(SS_PERS_ISNULL(&target->decomp) || pmode==SAF_ALL || 1==scope_size, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if targeting of storage decomposition is used, the read must be a SAF_ALL mode read or the "
                            "database must be opened on only a single processor"));

    if (Pcount)
        *Pcount = 1234567;

    /* Cache some stuff for convenience */
    ftmpl = SS_FIELD(field)->ftmpl;
    dof_assoc_cat = SS_FIELD(field)->dof_assoc_cat;
    basespace = SS_FIELD(field)->base_space;

    /* When a field is stored on "self" then we store actual field coefficient values.  When a field is not stored on "self"
     * then DOFs are stored on the subsets specified by the decomposition.  In this case the field values of this "parent"
     * field are the handles to the "actual" fields found on each of the subsets. Dofs are stored in the dof_blob while field
     * handles are stored in a variable length array. */
    if (!SS_PERS_ISNULL(SS_FIELD_P(field,dof_blob))) {
        /*  In this case the field appears to be stored on self so the stored values are real field DOF values. */
        SAF_ASSERT(_saf_is_self_decomp(SS_FIELD_P(field,storage_decomp_cat)), SAF_LOW_CHK_COST, SAF_ASSERTION_ERROR,
                   _saf_errmsg("if a dof_blob exists then the field must not be storing pointers to other fields"));
        ss_blob_bound_f1(SS_FIELD_P(field,dof_blob), NULL, NULL, &count, &ftype);
        if (Ptype) *Ptype = H5Tget_native_type(target->data_type>0?target->data_type:ftype, H5T_DIR_DEFAULT);
        H5Tclose(ftype); ftype=-1;
        if (Pcount) *Pcount = count;
    } else if (ss_array_nelmts(SS_FIELD_P(field,indirect_fields))>0) {
        /* In this case the field is stored on a decomposition.  The stored values are handles to the fields on the parts *
	 * forming the decomposition.  The datatype is known to be handles (to fields) and the VL-array tells how
	 * many. However the caller may have used the saf_target_field() function to request the field to be remapped. */
        if (SS_PERS_ISNULL(&target->decomp)) {
            desireHandles = TRUE;
        } else if (_saf_is_self_decomp(&target->decomp)) {
            desireHandles = FALSE;
        } else {
            desireHandles = TRUE;
        }
        if (desireHandles) {
            /* The caller has not requested the field to be remapped, thus we can assume that the caller desires handles... */
            if (Pcount) *Pcount = ss_array_nelmts(SS_FIELD_P(field,indirect_fields));
            if (Ptype) *Ptype = H5Tcopy(ss_pers_tm); /*target datatype is ignored in this case*/
        } else {  
            /* The caller has requested the field to be remapped, we must track down the proper count by examining the
             * collection that the field is defined on (rather than the one that it is decomposed on) and the proper type
             * by following the indirection. */
            if (Pcount) {
                /* First we'll figure out how many DOFs the remapped field has. We'll use the category that field is defined on
                 * and the basespace to identify the collection that the field DOFS are associated with. Note that the
                 * basespace is actually kept with the field template. If the number of components is negative then the
                 * component count must be gotten by recursing on the indirect fields until a valid component count is
                 * encountered. */
                numberOfComponents = SS_FIELDTMPL(&ftmpl)->num_comps;
                if (numberOfComponents <= 0)
                    _saf_numberOfComponentsOf_field(pmode, field, &numberOfComponents);
                saf_describe_collection(pmode, &basespace, &dof_assoc_cat, NULL, &collSize, NULL, NULL, NULL);
                *Pcount = collSize * SS_FIELD(field)->assoc_ratio * numberOfComponents;
            }
            if (Ptype) {
                if (target->data_type<=0) {
                    /*  Then we'll get the indirect handles using a recursive call with no field targeting. */
                    saf_get_count_and_type_for_field(pmode, field, NULL, &icount, NULL);
                    if (!icount)
                        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("no indirect field handles"));
                    if (NULL==(ifields=malloc(icount*sizeof(*ifields))))
                        SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate buffer to recieve indirect fields"));
                    saf_read_field(pmode, field, NULL, SAF_WHOLE_FIELD, (void**)&ifields);

                    /* We'll now get the type of the first field on the decomposition to determine the type to return.  Note
                     * that we are not requiring the data types of the fields to match since saf_read_field will do the
                     * appropriate data type conversions. Use the same targeting as for FIELD. */
                    saf_get_count_and_type_for_field(pmode, ifields, target, NULL, Ptype);
                    ifields = SS_FREE(ifields);
                } else {
                    *Ptype  = H5Tget_native_type(target->data_type, H5T_DIR_DEFAULT);
                }
            }
        }
    } else {
        /* In this case the field is a component of another field or is made up of components.  The datatype and count are
         * unknown.  However the caller may have used the saf_target_field() function to request the field to be remapped. */
        if (SS_PERS_ISNULL(&target->decomp)) {
            desireHandles = TRUE;
        } else if (_saf_is_self_decomp(&target->decomp)) {
            desireHandles = FALSE;
        } else {
            desireHandles = TRUE;
        }
        if (desireHandles) {
            if (Pcount) *Pcount = 0;
            if (Ptype) *Ptype = H5I_INVALID_HID;
        } else {
            /* Lets find out if the field is made up of components.  If it is then get the size and count of the first
             * component, otherwise punt since it is a component of another field. */
            saf_describe_field(pmode, field, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &num_comps,
                               &comps, NULL, NULL);
            if (0 < num_comps) {
                /* In this case the field is made up of components.  We can use the count and type of the first component. */
                saf_get_count_and_type_for_field(pmode, comps+0, NULL, Pcount, Ptype);
                if (Pcount) *Pcount *= num_comps;
                comps = SS_FREE(comps);
            } else {
                /* In this case the field is a component of another field and has no storage of its own.  It really should have
                 * a blob record which provides the offset/skip info for traversing the blob of the parent field but that is
                 * not currently the case... */
                if (Pcount) *Pcount = 0;
                if (Ptype) *Ptype  = H5I_INVALID_HID;
            }
        }
    }

    assert(!Pcount || *Pcount!=1234567);
    SAF_LEAVE(SAF_SUCCESS);
}
     

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Fields
 * Purpose:    	Find the parent of a component field
 *
 * Description: Find the parent of a component field.  Find the field who has the input field as a component.
 *              This function caches the parent field in the component field in order to keep the performance good.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Field *
_saf_find_parent_field(SAF_ParMode pmode,
                       SAF_Field *component_field,      /* Field for which we are searching for a parent. */
                       SAF_Field *retval                /* OUT: Optional buffer in which to store the result. If this is NULL
                                                         * then a buffer will be allocated for the return value. */
                       )
{
    SAF_ENTER(_saf_find_parent_field, SAF_ERROR_FIELD);
    int         nfound=0;
    SAF_Field   *fields=NULL, *component_fields;
    int         i, j, num_comps;
    SAF_Db      db;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);

    /* Get the file to which the component_field belongs */
    ss_pers_file((ss_pers_t*)component_field, &db);


    if (SS_PERS_ISNULL(SS_FIELD_P(component_field,m.parent))) {
        saf_find_fields(pmode, &db, NULL, NULL, NULL, NULL, NULL, NULL, NULL, SAF_ANY_RATIO, NULL, NULL, &nfound, &fields);
        for(i=0; i<nfound; i++) {
            component_fields=NULL;
            num_comps=0;

            {
                /* JSJ - Check if this field is a STATE, and if so, skip the saf_describe_field.  Calling saf_describe_field
                 * for a STATE field causes an error. This section should be removed when either saf_describe_field is fixed
                 * (if this is indeed an error) or when you can use saf_find_fields to find all-alg-types-except-fields, or ? */
                SAF_FieldTmpl l_ftmpl;
                SAF_Algebraic l_alg_type;
                saf_describe_field(pmode, fields+i, &l_ftmpl, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                   NULL, NULL, NULL, NULL);
                saf_describe_field_tmpl(pmode, &l_ftmpl, NULL, &l_alg_type, NULL, NULL, NULL, NULL);
                if (SAF_EQUIV(&l_alg_type,SAF_ALGTYPE_FIELD)) continue;
            }
      
            saf_describe_field(pmode, fields+i, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               &num_comps, &component_fields, NULL, NULL);	
            if (num_comps>1) {
                SS_FIELD(fields+i)->m.parent = fields[i];
                for (j=0; j<num_comps; j++) {
                    SS_FIELD(component_fields+j)->m.parent = fields[i];
                }
            }
            component_fields = SS_FREE(component_fields);
        }
    }

    if (!retval && NULL==(retval=malloc(sizeof *retval)))
        SAF_ERROR(NULL, _saf_errmsg("unable to allocate return value"));
    *retval = SS_FIELD(component_field)->m.parent;
    SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:    	Fields
 * Purpose:     Read data for an indirect field
 *
 * Description: This function is called by saf_read_field() when the desired field was not directly
 *              written. It will try to find the data in components of the desired field or in a 
 *              parent field. 
 *
 * Issue:       Tested only with 1 level of indirection, but it should work with more.
 *
 *              Assuming SAF_WHOLE_FIELD for now.
 *
 *              Apparently doesn't take field target datatype into account when reading the data. [rpm 2004-05-21]
 *
 * Programmer:	Jake S. Jones, May 14, 2002.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_read_comp_field(SAF_ParMode pmode,
                     SAF_Field *field,
                     int member_count,
                     SAF_RelRep *req_type,
                     int *member_ids,
                     void **Pbuf
                     )
{
    SAF_ENTER(_saf_read_comp_field, SAF_PRECONDITION_ERROR);

    SAF_Field *l_componentFields=NULL;
    int l_numComponents=0;
    SAF_Field l_parentField;
    SAF_Interleave l_compInterleave;

    SAF_REQUIRE(member_count<=0 && SS_RELREP(req_type)->id==SAF_TOTALITY_ID && member_ids==NULL,
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("Currently, if reading a component-composite field, the request must be for SAF_WHOLE_FIELD"));

    saf_describe_field(pmode, field, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
                       &l_numComponents, &l_componentFields, &l_compInterleave, NULL);

    _saf_find_parent_field(pmode, field, &l_parentField);

    if (l_componentFields && l_numComponents && !SAF_EQUIV(field,&l_parentField)) {
        printf("WARNING: FIELD HAS BOTH PARENT AND CHILDREN....MIGHT NOT FIND DATA AS CURRENTLY WRITTEN.\n");
    }

    if (l_componentFields && l_numComponents) {
        /* Handle case where this is a composite field, but the data was written to the indiv components. */ 
        char **l_compBuf = malloc(l_numComponents*sizeof(*l_compBuf));
        size_t i, l_count=0, l_PtypeSize;
        hid_t l_Ptype;

        for (i=0; i<(size_t)l_numComponents; i++) {
            l_compBuf[i]=0;
            saf_read_field(pmode, l_componentFields+i, NULL, member_count, req_type, member_ids, (void**)(l_compBuf+i));
	}

        /* ISSUE: Assuming all components have the same count and type. */
        saf_get_count_and_type_for_field(SAF_ALL, l_componentFields, NULL, &l_count, &l_Ptype );/*check only 1st component*/
        l_PtypeSize = H5Tget_size(l_Ptype);
	
        /* Allocate a return buffer if the caller did not provide one... */
        if (*Pbuf == NULL) {
            *Pbuf = malloc(l_count * l_PtypeSize * l_numComponents);
            if (!*Pbuf) SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate space to read field"));
	}

        if (l_compInterleave == SAF_INTERLEAVE_VECTOR) {
            char *l_ptr = Pbuf[0];
            for (i=0; i<l_count; i++) {
                int j;
                for (j=0; j<l_numComponents; j++, l_ptr+=l_PtypeSize)
                    memcpy(l_ptr , l_compBuf[j]+i*l_PtypeSize, l_PtypeSize);
	    }
	} else if (l_compInterleave == SAF_INTERLEAVE_COMPONENT || l_compInterleave == SAF_INTERLEAVE_INDEPENDENT) {
            /*ISSUE: As of 2004may13, treating INDEPENDENT as COMPONENT, rather than failing*/
            int j;
            char *l_ptr = Pbuf[0];
            for (j=0; j<l_numComponents; j++, l_ptr+=l_count*l_PtypeSize) {
                memcpy(l_ptr, l_compBuf[j], l_count*l_PtypeSize);
	    }
	} else {
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unknown component interleave (not SAF_INTERLEAVE_VECTOR or _COMPONENT)"));
	}
        l_compBuf = SS_FREE(l_compBuf);
    } else if (!SAF_EQUIV(field,&l_parentField)) {
        char **l_parentBuf;
        SAF_FieldTmpl l_parentFieldTemplate;
        SAF_Field *l_parentComponentFields=NULL;
        char *l_parentFieldName=NULL;
        int l_numParentComponents=0;
        hid_t l_Ptype;
        size_t l_PtypeSize;
        int l_whichComponentIsIt=-1;
        int i;
        size_t l_count=0;
        SAF_Interleave l_parentCompInterleave;

        saf_describe_field(pmode, &l_parentField, &l_parentFieldTemplate, &l_parentFieldName, NULL,NULL, 
                           NULL, NULL, NULL, NULL, NULL, NULL, NULL, &l_numParentComponents, 
                           &l_parentComponentFields, &l_parentCompInterleave, NULL);

        saf_get_count_and_type_for_field(SAF_ALL, &l_parentField, NULL, &l_count, &l_Ptype);
        l_PtypeSize = H5Tget_size(l_Ptype);

        l_count = l_count / l_numParentComponents;

        /*XXX is a waste to read entire field: should use a partial io request*/
        l_parentBuf = malloc(sizeof(*l_parentBuf)); 
        *l_parentBuf = NULL;
      
        saf_read_field(pmode, &l_parentField, NULL, member_count, req_type, member_ids, (void**)l_parentBuf);
      
        if (!l_parentComponentFields || !l_numParentComponents) {
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("parent field seems to have disowned all child fields"));
	}

        for (i=0; i<l_numParentComponents; i++) {
            if (SAF_EQUIV(field,l_parentComponentFields+i)) {
                l_whichComponentIsIt = i;
                break;
            }
	}

        if (l_whichComponentIsIt < 0) {
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("parent field seems to have disowned child field"));
	}

        /*ISSUE: Assuming all components have the same count and type. */

        /* Allocate a return buffer if the caller did not provide one... */
        if (*Pbuf == NULL) {
            *Pbuf = malloc(l_count * l_PtypeSize);
            if (!*Pbuf) SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate space to read field"));
	}

        if (l_parentCompInterleave == SAF_INTERLEAVE_VECTOR) {
            /*XXX this is slow*/
            size_t k;
            char *l_ptr = Pbuf[0];

            for (k=0; k<l_count; k++, l_ptr+=l_PtypeSize) {
                size_t l_bytes=k*l_numParentComponents*l_PtypeSize + l_whichComponentIsIt*l_PtypeSize;
                memcpy(l_ptr, l_parentBuf[0]+l_bytes, l_PtypeSize);
	    }
	} else if (l_parentCompInterleave == SAF_INTERLEAVE_COMPONENT) {
            memcpy(Pbuf[0], l_parentBuf[0]+l_whichComponentIsIt*l_count*l_PtypeSize, l_count*l_PtypeSize);
	} else {
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unknown component interleave (not SAF_INTERLEAVE_VECTOR or _COMPONENT)"));
	}
    } else {
        /*this field was apparently never written*/
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to read unwritten field"));
    }

    SAF_LEAVE(SAF_SUCCESS);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Read the data for a field
 *
 * Description:	This is function is used to read a field's data. If the field is *not* an indirect reference to other fields,
 * 		this call involves *real* disk I/O. All functions in SAF with either "read" or "write" in the name potentially
 *		involve real disk I/O.
 *
 *		This function allows a client to read either the entire field's data or a portion of the field's data. Recall
 *		that the /degrees/of/freedom/ (dofs) of a field are n:1 associated with the members of some collection in
 *		the set upon which the field is defined. We call this collection the /associated/collection/.
 * 
 *		In order to specify a partial request, the client is required to
 *		specify which members of the associated collection it is reading the dofs for. Ultimately, those members may be
 *		specified using a N dimensional hyperslab (or hypersample) or an arbitrary list of N-tuples. In either case,
 *		the number of dimensions, 'N', is the number of indexing dimensions in the associated collection.
 *
 *		At present, there are several limitations. First, the collection must be 1 dimensionally indexed. Next,
 *		only the hyperslab mode or a single member in tuple-mode are supported; not hypersamples and not an
 *		arbitrary list. Also, if the field is a multi-component field, then the only supported interleave mode is
 *		SAF_INTERLEAVE_VECTOR.
 *
 *		Finally, we provide as a convenience the macro SAF_WHOLE_FIELD which expands to a comma separated list of
 *		values, 0, SAF_TOTALITY, NULL, for the three arguments MEMBER_COUNT, REQ_TYPE, MEMBER_IDS for the case in
 *		which the client is reading the whole field in this call.
 *
 * Issues:	A partial I/O request looks a lot like a subset relation. In fact, we even use the same data type, SAF_SRType,
 *		to identify the type of partial I/O request. It may be difficult for a client to distinguish between making
 *		a partial I/O request and making real subsets. In theory, there *really* should be no difference. The act
 *		of reading/writing a portion of a field *is* the act of defining a subset of the base space the field is defined
 *		on and then restricting the field to that subset. In the current implementation, this requires, at a minimum, the
 *		ability to create transient objects such as the subset representing the piece of the field being read/written in
 *		this call. In addition, it *really* requires decoupling the storage containers into which field's data goes
 *		from declaring and reading/writing fields.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_read_field(SAF_ParMode pmode,       /* The parallel mode. */
               SAF_Field *field,        /* The field which is to be read. */
               SAF_FieldTarget *target, /* Field targeting information. */
               int member_count,	/* A count of the number of members of the collection in which the field's dofs are n:1
                                         * associated with that are actually being written in this call. This value is ignored
                                         * if you are reading the entire field's dofs in this call (i.e., REQ_TYPE =
                                         * SAF_TOTALITY). Also note that as a convenience, we provide the macro
                                         * SAF_WHOLE_FIELD which expands to a comma separated list of appropriate values for
                                         * this argument and the next two, for the case in which the whole field is being read
                                         * in this call. */
               SAF_RelRep *req_type,	/* The type of I/O request. We use a relation representation type here to specify the
                                         * type of the partial request because it captures the necessary information. Pass
                                         * SAF_HSLAB if you are reading the dofs of a partial hyperslab of the members of the
                                         * associated collection. In this case, MEMBER_IDS points to 3 N-tuples of starts,
                                         * counts and strides of the hyperslab (hypersample) request. Pass SAF_TUPLES, if you
                                         * are reading the dofs for an arbitrary list of members of the associated collection.
                                         * In this case, the MEMBER_IDS points to a list of N-tuples. In both cases, 'N' is
                                         * the number of indexing dimensions in the associated collection. Finally, pass
                                         * SAF_TOTALITY if you are reading the entire field's set of dofs. */
               int *member_ids,	        /* Depending on the value of REQ_TYPE, this argument points to 3 N-tuples storing,
                                         * respectively, the starts, counts and strides *in*each*dimension* of the associated
                                         * collection or to a list of MEMBER_COUNT N-tuples, each one identifying a single
                                         * member of the associated collection or to NULL in the case of a SAF_TOTALITY request. */
               void **Pbuf              /* INOUT: A pointer to a buffer pointer which is to receive the values read.  The
                                         * caller may supply a pointer to a value of NULL if this function is to allocate a
                                         * buffer.  If the caller supplies a pointer to a non-NULL pointer (to a buffer) then
                                         * it is up to the caller to ensure that the buffer is of sufficient size to hold all
                                         * of the data retrieved.  The caller should use saf_describe_field() or
                                         * saf_get_count_and_type_for_field() to determine the datatype of the values read. */
               )
{
    SAF_ENTER(saf_read_field, SAF_PRECONDITION_ERROR);
    double              timer_start=0;                  /* Start time for accumulating total field read times */
    ss_scope_t          scope=SS_SCOPE_NULL;            /* Scope containing FIELD */
    int                 scope_size;                     /* Size of the cummunicator for `scope' */
    static SAF_FieldTarget ft_zero;                     /* Default field targeting */
    hbool_t             has_been_written;               /* True if data has been written to the field */
    int                 retval;                         /* Return value for this function */
    ss_fieldtmpl_t      ftmpl=SS_FIELDTMPL_NULL;        /* Cached field template link from FIELD */
    ss_algebraic_t      algebraic=SS_ALGEBRAIC_NULL;    /* Cached algebraic type link from FIELD */
    ss_blob_t           dof_blob=SS_BLOB_NULL;          /* Cashed DOF blob from FIELD */
    hbool_t             desireHandles;                  /* Should we read field handles instead of dofs? */
    size_t              size;                           /* Number of elements to read */
    hsize_t             hsize;                          /* Size to bass to blob functions */
    size_t              offset;                         /* Index of first element to read */
    hid_t               memDatatype=-1;                 /* Type of data to store in returned PBUF array */
    hid_t               fileDatatype=-1;                /* Type of data stored in the file */
    SAF_Db              base_space_db;                  /* The database holding the base space of FIELD */
    size_t              numberOfGlobalDOFs;
    hid_t               theGlobalDOFType;
    SAF_FieldTmpl       theGlobalTemplate;
    SAF_Interleave      theGlobalComponentInterleave;
    size_t              theGlobalDOFSize;
    void                *theGlobalBuffer=NULL;
    char                *theGlobalPointer;
    size_t              numberOfLocalFields;
    int		        theGlobalComponentCount;
    SAF_Set             theGlobalSet;
    void                *buffer;
    SAF_Field           *theLocalFields;
    size_t              f;
    hid_t               theHandleType;
    size_t	        numberOfGlobalValues;
    
    if (_SAF_GLOBALS.p.TraceTimes)
        timer_start = _saf_wall_clock(false);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    ss_pers_scope((ss_pers_t*)field, &scope);
    ss_scope_comm(&scope, NULL, NULL, &scope_size);
    if (!target) target = &ft_zero;

    SAF_REQUIRE(SS_FIELD(field), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FIELD must be a valid field handle"));
    SAF_REQUIRE(Pbuf, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PBUF must be non-null"));
    SAF_REQUIRE(_saf_is_valid_io_request(pmode, field, member_count, req_type, member_ids, 1),
                SAF_HIGH_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if partial I/O request, associated collection must be 1D indexed, REQ_TYPE must be SAF_HSLAB "
                            "or a single SAF_TUPLE and field's interleave, if multi-component, must be SAF_INTERLEAVE_VECTOR"));
    SAF_REQUIRE(SS_PERS_ISNULL(&target->decomp) || pmode==SAF_ALL || 1==scope_size, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if field targeting of storage decomposition is used, the read must be a SAF_ALL mode read or the "
                            "database must be opened on only a single processor"));

    /* If there is no data written to this field, try using _saf_read_comp_field instead. */
    saf_data_has_been_written_to_field(pmode, field, &has_been_written);
    if(!has_been_written) {
        retval = _saf_read_comp_field(pmode, field, member_count, req_type, member_ids, Pbuf);
        SAF_RETURN(retval);
    }

    /* Cache some stuff for convenience */
    ftmpl = SS_FIELD(field)->ftmpl;
    algebraic = SS_FIELDTMPL(&ftmpl)->algebraic;
    dof_blob = SS_FIELD(field)->dof_blob;

    /* We can't get here without passing the valid_io_request pre-condition and all the limitations it currently
     * imposes. So, we know member_ids is either an array of 3 ints {start, count, stride} where stride is constrained
     * to 1 for SAF_HSLAB or an array of 1 int {index} for SAF_TUPLES. Regardless, member_ids[0] is the starting position
     * and member_count is the size of the request. */
    if (_saf_is_self_decomp(SS_FIELD_P(field,storage_decomp_cat)) && !SS_ALGEBRAIC(&algebraic)->indirect) {
        /* The stored values in the field are real field DOF values. */
        ss_blob_bound_f1(&dof_blob, NULL, NULL, &hsize, &fileDatatype);
        if (SAF_TOTALITY_ID==SS_RELREP(req_type)->id) {
            size = (size_t)hsize;
            offset = 0;
        } else {
            offset = member_ids[0];
            size = member_count;
        }
        memDatatype = H5Tget_native_type(target->data_type>0?target->data_type:fileDatatype, H5T_DIR_DEFAULT);

        /*  Allocate a return buffer if the caller did not provide one... */            
        if (!*Pbuf && NULL==(*Pbuf=malloc(size*H5Tget_size(memDatatype))))
            SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate space to read field"));

        /* Read field DOFs from blob dataset filling the return buffer... */
        if (ss_blob_bind_m1(&dof_blob, *Pbuf, memDatatype, (hsize_t)size)<0 ||
            NULL==ss_blob_read1(&dof_blob, (hsize_t)offset, (hsize_t)size, SS_BLOB_UNBIND, NULL))
            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to read field dofs"));
    } else {
        /* The field is stored on a decomposition.  The stored values are handles to the fields on the parts forming the
         * decomposition.  The datatype is known to be handles (to fields) and the blob record tells how many. However the
         * caller may have used the saf_target_field() function to request the field to be remapped. */
        if (SAF_TOTALITY_ID==SS_RELREP(req_type)->id) {
            offset = 0;
            size = ss_array_nelmts(SS_FIELD_P(field,indirect_fields));
        } else {
            offset = member_ids[0];
            size = member_count;
        }
        if (SS_PERS_ISNULL(&target->decomp)) {
            desireHandles = TRUE;
        } else if (_saf_is_self_decomp(&target->decomp)) {
            desireHandles = FALSE;
        } else {
            desireHandles = TRUE;
        }
        if (desireHandles) {
            /* We need to adjust the offset for the case of a homogeneous field of fields (probably a state field) because all
             * the data is compressed into a single blob. For an inhomogeneous field the size and count need no adjustment. */
            if (_saf_is_self_decomp(SS_FIELD_P(field,storage_decomp_cat)) && SS_ALGEBRAIC(&algebraic)->indirect) {
                assert(SS_FIELDTMPL(&ftmpl)->num_comps>=0);
                size *= SS_FIELDTMPL(&ftmpl)->num_comps;
                offset *= SS_FIELDTMPL(&ftmpl)->num_comps;
            }
            if (NULL==(*Pbuf = ss_array_get(SS_FIELD_P(field,indirect_fields), ss_pers_tm, offset, size, *Pbuf)))
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unable to read field links"));
        } else {
            /* The caller wishes to receive the DOFs as though they had been stored on self rather than on a
             * decomposition. What we need to do is to allocate a buffer big enough for the mapped field. We then read the each
             * local field one at a time and move the DOFs from the local field buffer to the global buffer. */

            /* ISSUE: For an indirect field the the local fields are all "similar". That is, they have the same algebraic type,
             *        association category, units and such. This function should check for this but doesn't. In the future
             *        some differences can be smoothed-over (such as units) but some probably can not (such as algebraic
             *        type). */
            /* ISSUE: The proper use of PMODE is not fully worked out. */
            /* ISSUE: Multiple indirection may actually fall out of this solution but that is not at all clear. */
            
            saf_get_count_and_type_for_field(pmode, field, target, &numberOfGlobalDOFs, &theGlobalDOFType);
            saf_describe_field(pmode, field, &theGlobalTemplate, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               &theGlobalComponentCount, NULL, &theGlobalComponentInterleave, NULL);
            if (target->is_set) theGlobalComponentInterleave = target->comp_intlv;
#if 0
            /*  This needs to be fixed.  The problem is if theGlobalComponentCount == -1,
             *  then we don't know if we need to check the interleave.  We will only
             *  find out once we have read in the first block if we have multiple components,
                 *  so the test needs to be moved, not sure where at this point. */
            if (theGlobalComponentCount > 1) {
                if (theGlobalComponentInterleave != SAF_INTERLEAVE_VECTOR &&
                    theGlobalComponentInterleave != SAF_INTERLEAVE_COMPONENT) {
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid component interleave"));
                }
            }
#endif

            /*  Allocate storage for the global buffer. */                
            theGlobalDOFSize = H5Tget_size(theGlobalDOFType);
            if (*Pbuf) {
                theGlobalBuffer = *Pbuf;
            } else {
                if (numberOfGlobalDOFs == 0) {
                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("field has no global DOFs"));
                }
                theGlobalBuffer = malloc(numberOfGlobalDOFs * H5Tget_size(theGlobalDOFType));
                if (theGlobalBuffer == NULL) {
                    SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("can't allocate global DOF buffer"));
                }
            }
            theGlobalPointer = theGlobalBuffer;

            saf_get_count_and_type_for_field(pmode, field, NULL, &numberOfLocalFields, &theHandleType);
            if (numberOfLocalFields < 1) {
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("no indirect field handles"));
            }
            saf_describe_field(pmode, field, &theGlobalTemplate, NULL, &theGlobalSet, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               NULL, NULL, NULL, NULL, NULL);
            saf_describe_field_tmpl(pmode, &theGlobalTemplate, NULL, NULL, NULL, NULL, NULL, NULL);
            buffer = NULL;
            saf_read_field(pmode, field, NULL, SAF_WHOLE_FIELD, &buffer);
            if (buffer == NULL) {
                SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't read field handles"));
            }
            theLocalFields = buffer;
            for (f=0; f<numberOfLocalFields; f++) {
                size_t   numberOfLocalDOFs;
                hid_t theLocalDOFType;

                saf_get_count_and_type_for_field(pmode, theLocalFields+f, target, &numberOfLocalDOFs, &theLocalDOFType);
                if (target->data_type>0)
                    theLocalDOFType = target->data_type;
                if (0 < numberOfLocalDOFs) {
                    void          *Abuffer;
                    size_t         AbufSize;
                    hid_t          AbufType;
                    void          *Bbuffer;
                    size_t         BbufSize;
                    hid_t          BbufType;
                    int            C;
                    size_t         G,L;
                    size_t         CGstride, Gstride;
                    size_t         CLstride, Lstride;
                    int            numberOfSubsetRels;
                    SAF_Cat        theCat;
                    int	           theLocalComponentCount;
                    SAF_Field     *theLocalComponentFields;
                    SAF_Interleave theLocalComponentInterleave;
                    size_t         numberOfLocalValues;
                    void          *theLocalBuffer;
                    size_t         theLocalDOFSize;
                    char          *theLocalPointer;
                    SAF_Set        theLocalSet;
                    SAF_FieldTmpl  theLocalTemplate;
                    SAF_Rel       *theSubsetRels;
                    SAF_IndexSpec  ispec;
                    size_t         origin;
                    SAF_RelRep     srtype;

                    theLocalComponentFields = NULL;
                    saf_describe_field(pmode, theLocalFields+f, &theLocalTemplate, NULL, &theLocalSet, NULL, NULL, NULL, &theCat,
                                       NULL, NULL, NULL, NULL, &theLocalComponentCount, &theLocalComponentFields, NULL, NULL);

                    /* If the component count is negative then it is an indirect field and the component count must be gotten
                     * by recursing on the indirect fields until a valid component count is encountered. */
                    if (theLocalComponentCount == -1)
                        _saf_numberOfComponentsOf_field(pmode, theLocalFields+f, &theLocalComponentCount);

                    numberOfGlobalValues = numberOfGlobalDOFs / theLocalComponentCount;
                    numberOfLocalValues = numberOfLocalDOFs / theLocalComponentCount;

                    if (theLocalComponentFields != NULL) {
                        int c;
                        char *thePointer;

                        theLocalDOFSize = H5Tget_size(theLocalDOFType);
                        theLocalBuffer  = malloc(numberOfLocalDOFs*theLocalDOFSize);
                        if (theLocalBuffer == NULL)
                            SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("can't allocate local DOF buffer"));

                        thePointer = theLocalBuffer;
                        for(c=0; c<theLocalComponentCount; c++) {
                            saf_read_field(pmode, theLocalComponentFields+c, NULL, SAF_WHOLE_FIELD, (void **)&thePointer);
                            thePointer += theLocalDOFSize * numberOfLocalValues;
                        }

                        free(theLocalComponentFields);

                        theLocalComponentInterleave = SAF_INTERLEAVE_COMPONENT;
                    } else {
                        theLocalDOFSize = H5Tget_size(theLocalDOFType);
                        theLocalBuffer  = malloc(numberOfLocalDOFs*theLocalDOFSize);
                        if (theLocalBuffer == NULL)
                            SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("can't allocate local DOF buffer"));
                        saf_read_field(pmode, theLocalFields+f, target, SAF_WHOLE_FIELD, &theLocalBuffer);
                        theLocalComponentInterleave = target->comp_intlv;
                    }
                    theLocalPointer = theLocalBuffer;
                    if (theLocalComponentCount > 1) {
                        if (theLocalComponentInterleave != SAF_INTERLEAVE_VECTOR &&
                            theLocalComponentInterleave != SAF_INTERLEAVE_COMPONENT) {
                            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid component interleave"));
                        }
                    }
                    saf_describe_field_tmpl(pmode, &theLocalTemplate, NULL, NULL, NULL, NULL, NULL, NULL);
                    saf_describe_collection(pmode, &theLocalSet, &theCat, NULL, NULL, &ispec, NULL, NULL);
                    origin = ispec.origins[0];

                    numberOfSubsetRels = 0;
                    theSubsetRels      = NULL;
#ifdef SSLIB_SUPPORT_PENDING /* Fix me in Phase-II */
                    /* ISSUE: When remapping an indirect field we only look in the top-scope of the file containing the
                     *        field's base space when searching for the subset relations. [rpm 2004-05-24] */            
#endif /*SSLIB_SUPPORT_PENDING*/
                    ss_pers_file((ss_pers_t*)&theGlobalSet, &base_space_db);
                    saf_find_subset_relations(pmode, &base_space_db, &theGlobalSet, &theLocalSet, &theCat, &theCat,
                                              SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE, &numberOfSubsetRels, &theSubsetRels);
                    if (numberOfSubsetRels!=1 || !theSubsetRels)
                        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't locate subset relation for field stored on domain"));
                    saf_describe_subset_relation(pmode, theSubsetRels, NULL, NULL, NULL, NULL, NULL, NULL, &srtype, NULL);
                    saf_get_count_and_type_for_subset_relation(pmode, theSubsetRels, NULL, &AbufSize, &AbufType, &BbufSize,
                                                               &BbufType);
                    Abuffer = NULL;
                    Bbuffer = NULL;
                    saf_read_subset_relation(pmode, theSubsetRels, NULL, &Abuffer, &Bbuffer);
                    if (!Abuffer)
                        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("can't read subset for domain"));
                    if (Bbuffer)
                        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unexpected B-buffer for subset for domain"));

                    /* Handle hyperslabs, by creating a tuple out of the hyperslab.  This isn't very efficient but it is the
                     * most economical to program. */
                    if (SAF_EQUIV(&srtype, SAF_HSLAB)) {
                        if (AbufSize != 3)
                            SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid hyper slab relation"));
                        if (H5Tequal(AbufType,H5T_NATIVE_INT)) {
                            int i;
                            int *p;
                            int start;
                            int count;
                            int stride;

                            start  = ((int *)Abuffer)[0];
                            count  = ((int *)Abuffer)[1];
                            stride = ((int *)Abuffer)[2];
                            free(Abuffer);

                            p = malloc(count*sizeof(int));
                            for (i=0; i<count; i++) {
                                p[i] = start + i * stride;
                            }
                            AbufSize = count;
                            Abuffer = p;
                        }
                    }

                    if (AbufSize != numberOfLocalValues)
                        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("subset size != number of local DOFs"));

                    if (theGlobalComponentInterleave == SAF_INTERLEAVE_VECTOR) {
                        CGstride = 1;
                        Gstride = theGlobalComponentCount;
                    } else {
                        CGstride = numberOfGlobalValues;
                        Gstride = 1;
                    }
                    if (theLocalComponentInterleave == SAF_INTERLEAVE_VECTOR) {
                        CLstride = 1;
                        Lstride = theLocalComponentCount;
                    } else {
                        CLstride = numberOfLocalValues;
                        Lstride = 1;
                    }

                    if (H5Tequal(AbufType,H5T_NATIVE_INT)) {
                        int *p = (int*)Abuffer;
                        void *buf = malloc(MAX(theLocalDOFSize, theGlobalDOFSize));
                        for (C=0; C<theLocalComponentCount; ++C) {
                            for (L=0; L<numberOfLocalValues; ++L) {
                                G = (size_t)(p[L]) - origin;
                                if (numberOfGlobalValues <= G) {
                                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid index found in A-buffer"));
                                }
                                memcpy(buf, theLocalPointer+(L*Lstride+C*CLstride)*theLocalDOFSize, theLocalDOFSize);
                                H5Tconvert(theLocalDOFType, theGlobalDOFType, 1, buf, NULL, H5P_DEFAULT);
                                memcpy(theGlobalPointer+(G*Gstride+C*CGstride)*theGlobalDOFSize, buf, theGlobalDOFSize);
                            }
                        }
                        SS_FREE(buf);
                    } else if (H5Tequal(AbufType,H5T_NATIVE_UINT)) {
                        unsigned int *p = (unsigned int *)Abuffer;
                        void *buf = malloc(MAX(theLocalDOFSize, theGlobalDOFSize));
                        for (C=0; C<theLocalComponentCount; ++C) {
                            for (L=0; L<numberOfLocalValues; ++L) {
                                G = (size_t)(p[L]) - origin;
                                if (numberOfGlobalValues <= G) {
                                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid index found in A-buffer"));
                                }
                                memcpy(buf, theLocalPointer+(L*Lstride+C*CLstride)*theLocalDOFSize, theLocalDOFSize);
                                H5Tconvert(theLocalDOFType, theGlobalDOFType, 1, buf, NULL, H5P_DEFAULT);
                                memcpy(theGlobalPointer+(G*Gstride+C*CGstride)*theGlobalDOFSize, buf, theGlobalDOFSize);
                            }
                        }
                        SS_FREE(buf);
                    } else if (H5Tequal(AbufType,H5T_NATIVE_LONG)) {
                        long *p = (long *)Abuffer;
                        void *buf = malloc(MAX(theLocalDOFSize, theGlobalDOFSize));
                        for (C=0; C<theLocalComponentCount; ++C) {
                            for (L=0; L<numberOfLocalValues; ++L) {
                                G = (size_t)(p[L]) - origin;
                                if (numberOfGlobalValues <= G) {
                                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid index found in A-buffer"));
                                }
                                memcpy(buf, theLocalPointer+(L*Lstride+C*CLstride)*theLocalDOFSize, theLocalDOFSize);
                                H5Tconvert(theLocalDOFType, theGlobalDOFType, 1, buf, NULL, H5P_DEFAULT);
                                memcpy(theGlobalPointer+(G*Gstride+C*CGstride)*theGlobalDOFSize, buf, theGlobalDOFSize);
                            }
                        }
                        SS_FREE(buf);
                    } else if (H5Tequal(AbufType,H5T_NATIVE_ULONG)) {
                        unsigned long *p = (unsigned long *)Abuffer;
                        void *buf = malloc(MAX(theLocalDOFSize, theGlobalDOFSize));
                        for (C=0; C<theLocalComponentCount; ++C) {
                            for (L=0; L<numberOfLocalValues; ++L) {
                                G = (size_t)(p[L]) - origin;
                                if (numberOfGlobalValues <= G) {
                                    SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("invalid index found in A-buffer"));
                                }
                                memcpy(buf, theLocalPointer+(L*Lstride+C*CLstride)*theLocalDOFSize, theLocalDOFSize);
                                H5Tconvert(theLocalDOFType, theGlobalDOFType, 1, buf, NULL, H5P_DEFAULT);
                                memcpy(theGlobalPointer+(G*Gstride+C*CGstride)*theGlobalDOFSize, buf, theGlobalDOFSize);
                            }
                        }
                        SS_FREE(buf);
                    } else {
                        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("unsupported type for subset for domain"));
                    }
                    free(Abuffer);
                    free(theSubsetRels);
                    free(theLocalBuffer);
                }
            }
            free(buffer);
            if (*Pbuf == NULL)
                *Pbuf = theGlobalBuffer;
        }
    }

    if (_SAF_GLOBALS.p.TraceTimes)
        _SAF_GLOBALS.CummReadTime += (_saf_wall_clock(false) - timer_start);

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Set the destination form of a field
 *
 * Description: Setup targeting information for a field during read or write. Please see the introductory note in the Field's
 *		chapter for some information on field targeting. 
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_target_field(SAF_FieldTarget *target,       /* OUT: The target information that will be initialized by this call. */
                 SAF_Unit *targ_units,          /* The new units. This parameter is ignored at this time. */
                 SAF_Cat *targ_storage_decomp,  /* The new storage decomposition. */
                 SAF_Cat *targ_coeff_assoc,     /* This parameter is ignored at this time. */
                 int targ_assoc_ratio,          /* This parameter is ignored at this time. */
                 SAF_Cat *targ_eval_coll,       /* This parameter is ignored at this time. */
                 SAF_Eval *targ_func,           /* This parameter is ignored at this time. */
                 hid_t targ_data_type,          /* The new destination data type. When the saf_write_field() function is called
						 * the datatype of the dataset produced is determined by this parameter.  When
						 * the saf_read_field() function is called, the datatype of the values placed in
                                                 * the caller's memory is determined by this parameter. If a value of
                                                 * H5I_INVALID_HID is passed for this parameter then datatype targeting is
                                                 * turned off and the default mechanism for determining the destination
                                                 * datatype is used. */
                 SAF_Interleave comp_intlv,     /* The particular fashion in which components are interleaved.  Currently
                                                 * there are really only two: SAF_INTERLEAVE_VECTOR and SAF_INTERLEAVE_COMPONENT.
                                                 * These represent the XYZXYZ...XYZ and the XXX...XYYY...YZZZ...Z cases.  Note that
                                                 * interleave really only deals with a single blob of storage.  In the case of a
                                                 * composite field whose coefficients are stored independently on the component
                                                 * fields then interleave really has no meaning (use SAF_INTERLEAVE_INDEPENDENT).
                                                 * Interleave only has meaning on fields with storage.  In the case of a scalar
                                                 * field interleave is also meaningless, both cases degenerate to the same layout:
                                                 * XXX...X (use SAF_INTERLEAVE_NONE). This parameter is ignored at this time. */
                 int *comp_order                /* Only relevant for fields with component fields.  This value indicates the order
                                                 * of the field IDs in the COMP_FLDS relative to the registered order. Pass NULL
                                                 * if the permutation is the identity. This parameter is ignored at this time. */

                 )
{
   SAF_ENTER(saf_target_field, SAF_PRECONDITION_ERROR);

   SAF_REQUIRE(!targ_storage_decomp || SS_CAT(targ_storage_decomp), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("STORAGE_DECOMP must be either NOT_SET, SELF_DECOMP or a valid cat handle"));
   SAF_REQUIRE(target, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("TARGET must be non-null"));

   memset(target, 0, sizeof *target);
   target->is_set = TRUE;
   if (targ_units) target->units = *targ_units;
   if (targ_storage_decomp) target->decomp = *targ_storage_decomp;
   if (targ_coeff_assoc) target->coeff_assoc = *targ_coeff_assoc;
   target->assoc_ratio = targ_assoc_ratio;
   if (targ_eval_coll) target->eval_coll = *targ_eval_coll;
   if (targ_func) target->func = *targ_func;  
   target->data_type = targ_data_type;
   target->comp_intlv = comp_intlv;
   target->comp_order = comp_order;

   SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Put an attribute with a field
 *
 * Description: This function is identical to the generic saf_put_attribute() function except that it is specific to
 *              SAF_Field objects to provide the client with compile time type checking. For a description,
 *              see saf_put_attribute().
 *
 * Modifications:
 *              Mark Miller, LLNL, 2000-04-20
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_put_field_att(SAF_ParMode pmode, SAF_Field *field, const char *name, hid_t type, int count, const void *value)
{
   SAF_ENTER(saf_put_field_att, SAF_PRECONDITION_ERROR);
   int retval = saf_put_attribute(pmode, (ss_pers_t*)field, name, type, count, value);
   SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     Get an attribute from a field
 *
 * Description: This function is identical to the generic saf_get_attribute() function except that it is specific to
 *              SAF_Field objects to provide the client with compile time type checking. For a description,
 *              see saf_get_attribute().
 *
 * Modifications:
 *              Mark Miller, LLNL, 2000-04-20
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_field_att(SAF_ParMode pmode, SAF_Field *fld, const char *name, hid_t *type, int *count, void **value)
{
   SAF_ENTER(saf_get_field_att, SAF_PRECONDITION_ERROR);
   int retval = saf_get_attribute(pmode, (ss_pers_t*)fld, name, type, count, value);
   SAF_LEAVE(retval);
}
