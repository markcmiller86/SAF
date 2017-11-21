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
 * Chapter:     Field Templates
 * Description: A field template represents all the abstract features of a field. That is, those features that are immutable
 *		as the data is exchanged between one scientific computing client and another. By contrast, a field (which
 *		is defined in terms of a field template) represents all the features of a field that might possibly change
 *		as the field is exchanged between scientific computing clients.
 *
 *		By and large, a field template can be viewed as defining a class of fields.
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Field Templates
 * Purpose:     Declare a field template
 *
 * Description:	This function declares a field template. A field template defines the implementation independent features
 *		of a field such as its algebraic type, the quantity it represents, etc.
 *
 * Return:      A pointer to the new field template handle is returned on success, either the FTMPL argument if non-null or a
 *              freshly allocated handle.  A null pointer is returned on failure.
 *
 * Issues:	It would be better if we could create new field types (templates) from old ones, or alternatively,
 *		construct new algebriac types from old ones. This is most apparent when the alg-type is SAF_FIELD. This is
 *		the C-language equivalent of a void *. It tells us only that it is a reference to a field (in fact, the lib
 *		doesn't care if you pass SAF_Rel objects here) but does not say what kind of fields (e.g. field templates)
 *		it should reference. One might be inclined to think that the component fields templates can serve to define
 *		the type of references of a SAF_FIELD entity. However, this is not so. The component field templates define
 *		the /component/fields/. We can illustrate by an example.
 *
 *		Suppose we have a time series of the coordinate field of an airplane. Each instant in time of the coordinate
 *		field is a field on SPACE. To create the coordinates as a function of time, we create a field on TIME whose
 *		alg-type is SAF_FIELD. What are its component fields? If we want somehow to use the component fields to define
 *		the kinds of field this SAF_FIELD entity refers to, we'd specify the field template for coordinate fields
 *		on SPACE as the component field template here. However, if we do that, how do we then specify the components
 *		of coordinates as a function of time, namely x(t), y(t), and z(t) whose field template is on TIME. We can't!
 *		In essence, we need to be able to say what kind of SAF_FIELD this entity is by passing a list of field
 *		templates as the algebraic type. Or, more specifically, we need to generalize the notion of algebraic type
 *		and allow the client to build new types from old ones. That will be deferred to a later release. For now,
 *		the best we can do with this is equivalent to a void* field reference.
 *
 *		We could extend the API and allow a list of field templates for the atype argument. Alternatively, would
 *		could allow the member field types to come in via the component fields of the component fields of a SAF_FIELD
 *		template. However, that is rather convoluted.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_FieldTmpl *
saf_declare_field_tmpl(SAF_ParMode pmode,       /* The parallel mode. */
                       SAF_Db *db,              /* The database handle in which to create the template. */
                       const char *name,        /* The name of the field template. */
                       SAF_Algebraic *atype,    /* The algebraic type: SAF_ALGTYPE_SCALAR, SAF_ALGTYPE_VECTOR,
                                                 * SAF_ALGTYPE_TENSOR, SAF_ALGTYPE_SYMTENSOR, SAF_ALGTYPE_FIELD.  If
                                                 * the algebraic type is SAF_ALGTYPE_FIELD, then all we know about the
                                                 * field is that it references other fields (i.e., an indirect field).
                                                 * Therefore, the next four arguments are not applicable.  More
                                                 * generalized user defined type definitions will be available in later
                                                 * implementations. */
                       SAF_Basis *basis,        /* The basis. Not implemented yet. Pass null */
                       SAF_Quantity *quantity,  /* The quantity. See saf_declare_quantity() for quantity definitions and how
                                                 * to define new quantities. */
                       int num_comp,            /* Number of components. Although this may often be inferred from ATYPE,
                                                 * SAF currently does no work to infer it. Pass SAF_NOT_APPLICABLE_INT if
                                                 * this template will be used in the declaration of an inhomogeneous field.
                                                 * Otherwise, pass the number of components. For a simple scalar field, the
                                                 * number of components is 1. See Fields for further discussion of
						 * inhomogeneous fields. */
                       SAF_FieldTmpl *ctmpl,    /* This is an array of NUM_COMPS field template handles that comprise the
                                                 * composite field template or NULL if there are no component field
                                                 * templates. Pass NULL if this field template will be used in the 
						 * declaration of an INhomogeneous field. */
                       SAF_FieldTmpl *ftmpl     /* Returned field template handle for composite fields. If the algebraic
                                                 * type (ATYPE) is SAF_ALGTYPE_FIELD, then the returned field template
                                                 * may be used as a state template (see State Templates). */
                       )
{
    SAF_ENTER(saf_declare_field_tmpl, NULL);
    ss_scope_t          scope=SS_SCOPE_NULL;    /* The scope in which to declare the new field template. */

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);

    SAF_REQUIRE(ftmpl, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("FTMPL must be non-null"));
    SAF_REQUIRE(name, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("NAME must be non-null"));
    SAF_REQUIRE(num_comp==SAF_NOT_APPLICABLE_INT || num_comp>=1, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("NUM_COMP >= 1"));
    SAF_REQUIRE(num_comp==SAF_NOT_APPLICABLE_INT || (num_comp>1 && ctmpl) || num_comp==1, SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("CTMPL must be non-NULL if NUM_COMP > 1"));
    SAF_REQUIRE(SS_ALGEBRAIC(atype), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("ATYPE must be a valid algebraic type handle"));
    SAF_REQUIRE(ctmpl || num_comp==SAF_NOT_APPLICABLE_INT || (!ctmpl && num_comp==1 && !SS_ALGEBRAIC(atype)->indirect),
                SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("CTMPL may be NULL only if NUM_COMP == 1 and ATYPE must be direct"));
    SAF_REQUIRE(!ctmpl || (ctmpl && num_comp!=SAF_NOT_APPLICABLE_INT), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("CTMPL must be NULL if components are not appropriate"));
    SAF_REQUIRE(!basis || SS_BASIS(basis), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("BASIS must be a valid basis handle or NULL"));
    SAF_REQUIRE(!quantity || SS_QUANTITY(quantity), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("QUANTITY must be a valid quantity handle if supplied"));

    /* The scope in which to declare the field template. */
    ss_file_topscope(db, &scope);
    
    /* Allocate and/or initialize the new field template object. */
    ftmpl = (ss_fieldtmpl_t*)ss_pers_new(&scope, SS_MAGIC(ss_fieldtmpl_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U,
                                         (ss_pers_t*)ftmpl, NULL);
    if (SAF_EACH==pmode) SS_PERS_UNIQUE(ftmpl);
    
    /* Save the component field templates */
    if (ctmpl) {
        if (ss_array_resize(SS_FIELDTMPL_P(ftmpl,ftmpls), (size_t)num_comp)<0 ||
            ss_array_put(SS_FIELDTMPL_P(ftmpl,ftmpls), ss_pers_tm, (size_t)0, (size_t)num_comp, ctmpl)<0)
            SAF_ERROR(NULL, _saf_errmsg("unable to link to component field templates"));
    }

    /* Initialize the other non-zero parts of the field template */
    ss_string_set(SS_FIELDTMPL_P(ftmpl,name), name);
    SS_FIELDTMPL(ftmpl)->algebraic = *atype;
    if (basis) SS_FIELDTMPL(ftmpl)->basis = *basis;
    if (quantity) SS_FIELDTMPL(ftmpl)->quantity = *quantity;
    SS_FIELDTMPL(ftmpl)->num_comps = num_comp;
 
    /* return stuff */
    SAF_LEAVE(ftmpl);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Field Templates
 * Purpose:     Get a description of a field template
 *
 * Description: This function returns information about a field template. 
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_field_tmpl(SAF_ParMode pmode,      /* The parallel mode. */
                        SAF_FieldTmpl *ftmpl,   /* The field template to be described. */
                        char **name,            /* OUT: The returned name. Pass NULL if you do not want the name returned.
						 * (see Returned Strings). */
                        SAF_Algebraic *alg_type,/* OUT: The returned algebraic type. Pass NULL if you do not want the type
                                                 * returned. */
                        SAF_Basis *basis,       /* OUT: The returned basis. Pass null if you do not want the basis returned. */
                        SAF_Quantity *quantity, /* OUT: The returned quantity. Pass null if you do not want the name returned. */
                        int *num_comp,          /* OUT: The returned number of components. Pass NULL if you do not want the name
                                                 * returned. Note that if the field template is assocaited with an INhomogeneous
						 * field, the returned value will always be SAF_NOT_APPLICABLE_INT. */
                        SAF_FieldTmpl **ctmpl   /* OUT: The returned array of component field template handles.  Pass NULL if you
                                                 * do not want the array returned. If the field template is associated with 
                                                 * an INhomogeneous field, the returned value, if requested, will always be
                                                 * NULL. (If the field template does not point to other field templates then
                                                 * this argument will be untouched by this function.) */
                        )
{
    SAF_ENTER(saf_describe_field_tmpl, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_FIELDTMPL(ftmpl), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("FTMPL must be a valid field template handle"));

    /* Fill in the returned values */
    if (_saf_setupReturned_string(name, ss_string_ptr(SS_FIELDTMPL_P(ftmpl,name))) != SAF_SUCCESS)
        SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to return name for field template %s\n",
                                                ss_string_ptr(SS_FIELDTMPL_P(ftmpl,name))));
    if (alg_type)
        *alg_type = SS_FIELDTMPL(ftmpl)->algebraic;
    if (basis)
        *basis = SS_FIELDTMPL(ftmpl)->basis;
    if (quantity)
        *quantity = SS_FIELDTMPL(ftmpl)->quantity;
    if (num_comp)
        *num_comp = SS_FIELDTMPL(ftmpl)->num_comps;
    if (ctmpl && ss_array_nelmts(SS_FIELDTMPL_P(ftmpl,ftmpls))>0) {
        /* Only initialize CTMPL if there are stored field template links in this field template. */
        if (NULL==(*ctmpl=ss_array_get(SS_FIELDTMPL_P(ftmpl,ftmpls), ss_pers_tm, (size_t)0, SS_NOSIZE, *ctmpl)))
            SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("cannot read field template's ftmpls array"));
    }
    
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Field Templates
 * Purpose:     Find field templates
 *
 * Description: This function finds field templates according to specific search criteria. 
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_field_tmpls(SAF_ParMode pmode,         /* The parallel mode. */
                     SAF_Db *db,                /* the database context for this search (previously retrieved from base_space) */
                     const char *name,          /* The name of the field template. */
                     SAF_Algebraic *atype,      /* The algebraic type to limit the search to. Pass NULL if you do
                                                 * not want to limit the search by this parameter. */
                     SAF_Basis *basis,          /* The basis to limit the search to. Pass NULL if you do not want to
                                                 * limit the search by this parameter. */
                     SAF_Quantity *quantity,    /* The quantity to search for. Pass NULL if you do not want to
                                                 * limit the search by this parameter. */
                     int *num,                  /* For this and the succeeding argument [see Returned Handles]. */
                     SAF_FieldTmpl **found      /* For this and the preceding argument [see Returned Handles]. */
                     )
{
    SAF_ENTER(saf_find_field_tmpls, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_FieldTmpl, key, mask);
    size_t      nfound;
    ss_scope_t  scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(!atype || SS_ALGEBRAIC(atype), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ATYPE must be a valid algebraic handle if supplied"));
    SAF_REQUIRE(!basis || SS_BASIS(basis), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("BASIS must be a valid basis handle if supplied"));
    SAF_REQUIRE(!quantity || SS_QUANTITY(quantity), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("QUANTITY must be a valid quantity handle if supplied"));

    ss_file_topscope(db, &scope);
    if (name) SAF_SEARCH_S(SAF_FieldTmpl, key, mask, name, name);
    if (quantity) SAF_SEARCH(SAF_FieldTmpl, key, mask, quantity, *quantity);
    if (basis) SAF_SEARCH(SAF_FieldTmpl, key, mask, basis, *basis);
    if (atype) SAF_SEARCH(SAF_FieldTmpl, key, mask, algebraic, *atype);
    
    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_fieldtmpl_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound,
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

    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Field Templates
 * Purpose:     Put an attribute with a field template
 *
 * Description: This function is identical to the generic saf_put_attribute() function except that it is specific to
 *              SAF_FieldTmpl objects to provide the client with compile time type checking. For a description,
 *              see saf_put_attribute().
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_put_field_tmpl_att(SAF_ParMode pmode, SAF_FieldTmpl *ftmpl, const char *name, hid_t type, int count, const void *value)
{
  SAF_ENTER(saf_put_field_tmpl_att, SAF_PRECONDITION_ERROR);
  int retval = saf_put_attribute(pmode, (ss_pers_t*)ftmpl, name, type, count, value);
  SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Field Templates
 * Purpose:     Get an attribute with a field template
 *
 * Description: This function is identical to the generic saf_get_attribute() function except that it is specific to
 *              SAF_FieldTmpl objects to provide the client with compile time type checking. For a description,
 *              see saf_get_attribute().
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_field_tmpl_att(SAF_ParMode pmode, SAF_FieldTmpl *ftmpl, const char *name, hid_t *type, int *count, void **value)
{
  SAF_ENTER(saf_get_field_tmpl_att, SAF_PRECONDITION_ERROR);
  int retval = saf_get_attribute(pmode, (ss_pers_t*)ftmpl, name, type, count, value);
  SAF_LEAVE(retval);
}

