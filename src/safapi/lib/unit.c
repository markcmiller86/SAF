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
#include <math.h>

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 *
 * Description: A unit is a particular physical quantity, defined and adopted by convention, with which other particular
 *              quantities of the same kind are compared to express their value. The library has two classes of units:
 *              /basic/units/ and /derived/units/. Basic units measure some arbitrary amount of a specific quantity while
 *              derived units are created by multiplying, scaling, and translating powers of other units (basic and/or
 *              derived).  All units are associated with a specific quantity of the database either explicitly or implicitly.
 *              Implicit association is allowed if the appropriate quantity is not ambiguous. The library is able to convert
 *              an array of measurements from one unit to another if the source and destination unit measure the same
 *              specific quantity.
 *
 *              The definition of a basic unit is a two step process.  First an empty definition is created with
 *              saf_declare_unit(), then the unit is associated with a quantity with saf_quantify_unit(). Example: define
 *              meters as a basic unit of length. That is, meters measures some arbitrary amount of length and will be the
 *              basis for deriving all compatible units.
 *
 *                  SAF_Unit *m = saf_declare_unit(SAF_ALL,db,"meter","m",NULL);
 *                  saf_quantify_unit(SAF_ALL,m,SAF_QLENGTH,1);
 *                  saf_commit(m,SAF_ALL,database);
 *
 *              The definition of derived units is similar when the new unit measures the same quantity. Example: define
 *              kilometers as 1000 meters (km and m both measure the same quantity).
 *              
 *                  SAF_Unit *km = saf_declare_unit(SAF_ALL,db,"kilometer","km",NULL);
 *                  saf_multiply_unit(SAF_ALL,km,1000,m,1);
 *
 *              Another way to define a unit is to multiply other units together. When this happens the new unit measures a
 *              different quantity than its unit divisors.  In most cases the library can figure out what specific quantity to
 *              use for the unit, but this is not possible when the library contains multiple quantity definitions for similar
 *              quantities (e.g., `molecular amount' and `monetary amount' are both amount-of-a-substance quantities, but the
 *              library has two separate quantity definitions because it should should not be possible to convert between
 *              moles and dollars).  Example: define coulomb as an ampere second instead of some arbitrary amount of charge:
 *
 *                  SAF_Unit *C = saf_declare_unit(SAF_ALL,db,"coulomb","C",NULL);
 *                  saf_multiply_unit(SAF_ALL,C,1,A,1); // ampere
 *                  saf_multiply_unit(SAF_ALL,C,1,s,1); // second
 *                  SAF_Quantity *charge = saf_find_one_quantity(db,"electric charge",NULL);
 *                  saf_quantify_unit(SAF_ALL,C,charge,1);
 *
 *              In the previous example the saf_quantify_unit() could have been omitted since the library only defines one
 *              electric charge quantity and there is no ambiguity.
 *
 *              Two notable units are thermodynamic temperature measured in absolute Celsius and Fahrenheit. Both of these
 *              are the same amount as a degree Kelvin or a degree Rankine, but are offset by some amount. These units can be
 *              declared with saf_offset_unit():
 *
 *                  SAF_Unit *absC = saf_declare_unit(SAF_ALL,db,"absolute Celceus","absC",NULL);
 *                  saf_multiply_unit(SAF_ALL,absC,1,k,1); // degree Kelvin
 *                  saf_offset_unit(SAF_ALL,absC,273.5);   // 0 deg C is 273.15 k
 *
 *              Another special type of unit is one which uses a logarithmic scale instead of a linear scale. For example,
 *              a decibel is a dimensionless measure of the ratio of two powers, equal to ten times the logarithm to the base
 *              ten of the ratio of two powers.  In acoustics the decibel is 20 times the common log of the ratio of sound
 *              pressures, with the denominator usually being 2e-5 pascal.  The saf_log_unit() can be used to define such a
 *              unit:
 *
 *                  SAF_Unit *dB = saf_declare_unit(SAF_ALL,db,"decibel","dB",NULL);
 *                  SAF_Quantity *spr = saf_find_one_quantity(db,"sound pressure ratio",NULL);
 *                  saf_quantify_unit(SAF_ALL,dB,spr,1);
 *                  saf_log_unit(SAF_ALL,dB,10,20);
 *
 *              The saf_offset_unit() and saf_log_unit() can only be applied to a unit after all multiplications have been
 *              performed, and such a unit cannot be used to derive other units.
 *--------------------------------------------------------------------------------------------------------------------------------
 */

#define HASH_UNITS
#ifdef HASH_UNITS /*RPM DEBUGGING 2004-09-26*/
static SAF_HashTable *UHash;
#endif

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Declare a new unit
 * Concepts:    Units, declaration of; Declaring units
 *
 * Description: This function declares a new unit whose product of quantity powers is unity. The client is expected to
 *              multiply powers of other quantities or units into this new unit via saf_multiply_unit().
 *
 * Return:      A new unit handle is returned on success. Otherwise a SAF_ERROR_HANDLE is returned or an exception
 *              is raised, depending on the error handling property of the library.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 22, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Unit *
saf_declare_unit(SAF_ParMode pmode,
                 SAF_Db *db,                    /* The database in which to create the new unit. */
                 const char *name,              /* Optional singular unit name. */
                 const char *abbr,              /* Optional singular abbreviation */
                 const char *url,               /* Optional documentation URL. */
                 SAF_Unit *unit                 /* OUT: Optional unit handle to initialize and return. */
                 )
{
    SAF_ENTER(saf_declare_unit, NULL);
    ss_scope_t          scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);
    ss_file_topscope(db, &scope);
    unit = (ss_unit_t*)ss_pers_new(&scope, SS_MAGIC(ss_unit_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U, (ss_pers_t*)unit, NULL);

    if (SAF_EACH==pmode) SS_PERS_UNIQUE(unit);
    ss_string_set(SS_UNIT_P(unit,name), name);
    ss_string_set(SS_UNIT_P(unit,abbr), abbr);
    ss_string_set(SS_UNIT_P(unit,url), url);
    SS_UNIT(unit)->scale = 1.0;

#ifdef HASH_UNITS /*RPM DEBUGGING 2004-09-26*/
    /* Store the new unit in the hash table by both description and abbreviation */
    if (!UHash) UHash = _saf_htab_new();
    _saf_htab_insert(UHash, _saf_hkey_str(name), (ss_pers_t*)unit);
    _saf_htab_insert(UHash, _saf_hkey_str(abbr), (ss_pers_t*)unit);
#endif

    SAF_LEAVE(unit);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Associates a unit of measure with a specific quantity
 *
 * Description: A basic unit is a unit which measures an arbitrary amount of some quantity, and is defined simply by
 *              associating the unit with its quantity by calling this function. (no multiplications by other units are
 *              necessary).
 *
 *              Derived units are built by multiplying together powers of one or more other units. If just one unit is
 *              multiplied into the new definition then the new definition will refer to the same specific quantity as the
 *              unit on which it is based (if the power is one). Otherwise, when units are multiplied together the quantity
 *              measured by the product is different than the quantity measured by any of the multiplicands. When this happens
 *              it may be necessary for the client to call this function to associate a specific quantity with this new unit
 *              (it is not necessary if the library can deduce the specific quantity unambiguously from the unit's database).
 *
 * Return:      A non-negative value is returned on success; otherwise either a negative value is returned or an exception is
 *              raised, depending on the error handling property of the library.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 20, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_quantify_unit(SAF_ParMode pmode,
                  SAF_Unit *unit,        /* The unit whose quantity information is being set. */
                  SAF_Quantity *quantity,/* The quantity which this unit measures.            */
                  double scale           /* This argument can be used to defined a new unit as some scale of the base unit for
                                          * the quantity without requiring the unit definition to include a multiplication by
                                          * the base unit. The SCALE is multiplied into any scale which is already present. */
                  )
{
    SAF_ENTER(saf_quantify_unit, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_UNIT(unit), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must be a valid unit handle"));
    SAF_REQUIRE(SS_QUANTITY(quantity), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("QUANTITY must be a valid quantity handle"));
    SAF_REQUIRE(scale>0.0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SCALE must be positive"));

    /* If UNIT already has a quantity then the new quantity must be structurally equivalent. */
    if (!SS_PERS_ISNULL(SS_UNIT_P(unit,quant))) {
        assert(SS_MAX_BASEQS==7);
        if (SS_QUANTITY(SS_UNIT_P(unit,quant))->flags    != SS_QUANTITY(quantity)->flags ||
            SS_QUANTITY(SS_UNIT_P(unit,quant))->power[0] != SS_QUANTITY(quantity)->power[0] ||
            SS_QUANTITY(SS_UNIT_P(unit,quant))->power[1] != SS_QUANTITY(quantity)->power[1] ||
            SS_QUANTITY(SS_UNIT_P(unit,quant))->power[2] != SS_QUANTITY(quantity)->power[2] ||
            SS_QUANTITY(SS_UNIT_P(unit,quant))->power[3] != SS_QUANTITY(quantity)->power[3] ||
            SS_QUANTITY(SS_UNIT_P(unit,quant))->power[4] != SS_QUANTITY(quantity)->power[4] ||
            SS_QUANTITY(SS_UNIT_P(unit,quant))->power[5] != SS_QUANTITY(quantity)->power[5] ||
            SS_QUANTITY(SS_UNIT_P(unit,quant))->power[6] != SS_QUANTITY(quantity)->power[6]) {
            SAF_ERROR(SAF_CONTEXT_ERROR, _saf_errmsg("UNIT and QUANTITY are incompatible"));
        }
    }

    /* Associate QUANTITY with UNIT */
    SAF_DIRTY(unit, pmode);
    SS_UNIT(unit)->quant = *quantity;
    SS_UNIT(unit)->scale *= scale;
    
    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Multiply a unit into a unit definition
 * Concepts:    Units, multiplication of; Multiplying units
 *
 * Description: After creating a new unit with saf_declare_unit(), the UNIT is defined by multiplying scaled powers of
 *              other units into it, one per call to this function. A division by MULTIPLIER can be accomplished by supplying a
 *              negative POWER, although COEF is always multiplied into U.  Essentially, the result is:
 *
 *                  UNIT' = UNIT * COEF * (MULTIPLIER ^ POWER)
 *
 *              If MULTIPLIER is NULL then it is assumed to be unity. In other words, the scale factor can be
 *              adjusted for the unit by calling this function with only a COEF value.
 *
 * Return:      This function returns some non-negative value on success; otherwise, it either returns a negative value or
 *              raises an exception, depending on the error handling property of the library.
 *
 * Also:        saf_divide_unit()
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 22, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_multiply_unit(SAF_ParMode pmode,
                  SAF_Unit *unit,                  /* The unit which is being modified by multiplying MULTIPLIER into it. */
                  double coef,                     /* A real coefficient multiplied into UNIT */
                  SAF_Unit *multiplier,            /* The optional multiplicand unit */
                  int power                        /* The power to which MULTIPLIER is raised before multiplying it into UNIT */
                  )
{
    SAF_ENTER(saf_multiply_unit, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_UNIT(unit), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must be a valid unit handle"));
    SAF_REQUIRE(0==SS_UNIT(unit)->offset, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must have a zero offset (the default)"));
    SAF_REQUIRE(0==SS_UNIT(unit)->logbase, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must not have a logarithm base assigned (the default)"));
    SAF_REQUIRE(coef>0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("COEF must be positive"));
    SAF_REQUIRE(!multiplier || SS_UNIT(multiplier),
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("MULTIPLIER must be a valid unit handle if supplied"));
    SAF_REQUIRE(!multiplier || 0==SS_UNIT(multiplier)->offset, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("MULTIPLIER must have a zero offset if supplied"));
    SAF_REQUIRE(!multiplier || 0==SS_UNIT(multiplier)->logbase, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("MULTIPLIER must not use a logarithmic scale if supplied"));

    /* Mark unit as having been modified. */
    SAF_DIRTY(unit, pmode);

    /* Fix affected wildcards */
    if (0==SS_UNIT(unit)->scale)
        SS_UNIT(unit)->scale = 1.0;

    /* Initial quantity */
    if (SS_PERS_ISNULL(SS_UNIT_P(unit,quant)) && multiplier && power) {
        SS_UNIT(unit)->quant = SS_UNIT(multiplier)->quant;
        SS_UNIT(unit)->scale *= SS_UNIT(multiplier)->scale;
        --power;
    }

    /* If the unit has a quantity already assigned then create a new quantity to assign to the unit. */
    if (!SS_PERS_ISNULL(SS_UNIT_P(unit,quant)) && multiplier && power) {
        ss_scope_t u_scope;
        ss_quantity_t *q;
        ss_pers_scope((ss_pers_t*)unit, &u_scope);
        q = SS_PERS_COPY(SS_UNIT_P(unit,quant), &u_scope, SAF_ALL==pmode?SS_ALLSAME:0U);
        SS_UNIT(unit)->quant = *q;
        ss_string_reset(SS_QUANTITY_P(q,name));
        ss_string_reset(SS_QUANTITY_P(q,abbr));
        ss_string_reset(SS_QUANTITY_P(q,url));
        saf_multiply_quantity(pmode, q, SS_UNIT_P(multiplier,quant), power);
        SS_FREE(q);
    }

    /* Adjust scale */
    if (multiplier) SS_UNIT(unit)->scale *= pow(SS_UNIT(multiplier)->scale, (double)power);
    SS_UNIT(unit)->scale *= coef;

    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Translate unit by an offset
 *
 * Description: Some units of measure have a scale which is translated from the origin by some amount. The most notable
 *              examples are absolute degrees Celsius and Fahrenheit.
 *
 *                  SAF_Unit absC = saf_declare_unit("Celsius","absC");
 *                  SAF_multiply_unit(absC, 1, kelvin, 1);
 *                  SAF_offset_unit(absC, 273.15);
 *
 * Return:      This function returns some non-negative value on success; otherwise, it either returns a negative value or
 *              raises an exception, depending on the error handling property of the library.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 20, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_offset_unit(SAF_ParMode pmode,
                SAF_Unit *unit,                 /* The unit which is being translated by OFFSET. */
                double offset                   /* The amount by which to translate the unit. */
                )
{
    SAF_ENTER(saf_offset_unit, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_UNIT(unit), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must be a valid unit handle"));
    SAF_REQUIRE(0==SS_UNIT(unit)->logbase, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must not have a logarithm base assigned (the default)"));

    SAF_DIRTY(unit, pmode);
    SS_UNIT(unit)->offset += offset;
    SAF_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Apply a logarithmic scale to a unit
 *
 * Description: Some units of measure use a logarithmic scale. An example is decibels. This function sets the base for the
 *              logarithm. A LOGBASE of zero implies a linear scale and is the default for all units. This function should
 *              only be called after any calls to saf_multiply_unit() and saf_offset_unit() for UNIT.
 *
 *                  U' = LOGCOEF *log* UNIT
 *
 *              where *log* is to the base LOGBASE.
 *
 * Return:      This function returns some non-negative value on success; otherwise, it either returns a negative value or
 *              raises an exception, depending on the error handling property of the library.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 20, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_log_unit(SAF_ParMode pmode,
             SAF_Unit *unit,                 /* The unit which is being translated by OFFSET. */
             double logbase,                 /* The base of the logarithm */
             double logcoef                  /* The amount by which to multiply the unit after taking the log. */
             )
{
    SAF_ENTER(saf_log_unit, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_UNIT(unit), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must be a valid unit handle"));
    SAF_REQUIRE(logbase>=0.0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("LOGBASE must be non-negative"));
    SAF_REQUIRE(logcoef || !logbase, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("LOGCOEF must be non-zero if a logarithmic scale is used"));

    SAF_DIRTY(unit, pmode);
    SS_UNIT(unit)->logcoef = logbase ? logcoef : 0.0;
    SS_UNIT(unit)->logbase = logbase;

    SAF_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Query unit characteristics
 * Concepts:    Units, querying characteristics; Characteristics of units
 *
 * Description: Given a UNIT, this function returns any information which is known about that unit.
 *
 * Return:      A non-negative value is returned on success. Failure is indicated by a negative return value or the raising of
 *              an exception, depending on the error handling property of the library. Some returned values may be DSL
 *              wildcards, depending on how UNIT was initialized.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 22, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_unit(SAF_ParMode pmode,
                  SAF_Unit *unit,               /* Unit about which to retrieve information. */
                  char **name,                  /* If non-null then upon return this will point to an allocated copy of the
                                                 * unit singular name. */
                  char **abbr,                  /* If non-null then upon return this will point to an allocated copy of the
                                                 * unit singular abbreviation. */
                  char **url,                   /* If non-null then upon return this will point to an allocated copy of the
                                                 * URL for the unit's documentation. */
                  double *scale,                /* If non-null then upon return *SCALE will be the scale factor for the unit. */
                  double *offset,               /* If non-null then upon return *OFFSET will be the offset for the unit. */
                  double *logbase,              /* If non-null then upon return *LOGBASE will be the logarithm base for the
                                                 * unit. The returned value zero indicates no logarithm is applied. */
                  double *logcoef,              /* If non-null then upon return *LOGCOEF will be the multiplier of the
                                                 * logarithmic scale. */
                  SAF_Quantity *quantity        /* If non-null then upon return this will point to the handle of the quantity
                                                 * on which this unit is based. If the UNIT has not been defined yet (such as
                                                 * calling this function immediately after saf_declare_unit()) then the
                                                 * quantity handle will be initialized to a null link. */
                  )
{
    SAF_ENTER(saf_describe_unit, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_UNIT(unit), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("UNIT must be a valid unit handle"));

    /* fill in return values */
    _saf_setupReturned_string(name, ss_string_ptr(SS_UNIT_P(unit,name)));
    _saf_setupReturned_string(abbr, ss_string_ptr(SS_UNIT_P(unit,abbr)));
    _saf_setupReturned_string(url, ss_string_ptr(SS_UNIT_P(unit,url)));
    if (scale)    *scale    = SS_UNIT(unit)->scale;
    if (offset)   *offset   = SS_UNIT(unit)->offset;
    if (logbase)  *logbase  = SS_UNIT(unit)->logbase;
    if (logcoef)  *logcoef  = SS_UNIT(unit)->logcoef;
    if (quantity) *quantity = SS_UNIT(unit)->quant;
        
    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Find units
 *
 * Description: This function allows a client to search for units in the database. The search may be limited by one or more
 *              criteria such as the name of the unit, etc.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Parallel:    Depends on PMODE
 *
 * Programmer:  Robb Matzke
 *              Monday, April  5, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_units(SAF_ParMode pmode,
               SAF_Db *db,                      /* Database in which to limit the search. */
               const char *name,                /* Optional unit description for which to search. */
               const char *abbr,                /* Optional abbreviation for which to search. */
               const char *url,                 /* Optional URL for which to search. */
               double scale,                    /* Optional scale for which to search (or pass SAF_ANY_DOUBLE). */
               double offset,                   /* Optional offset for which to search (or pass SAF_ANY_DOUBLE). */
               double logbase,                  /* Optional logorithm base for which to search (or pass SAF_ANY_DOUBLE). */
               double logcoef,                  /* Optional logorithm coefficient for which to search (or pass
                                                 * SAF_ANY_DOUBLE). */
               SAF_Quantity *quant,             /* Optional quantity for which to search. */
               int *num,                        /* For this and the succeeding argument [see Returned Handles]. */
               SAF_Unit **found                 /* For this and the preceding argument [see Returned Handles]. */
               )
{
    SAF_ENTER(saf_find_units, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Unit, key, mask);
    size_t      nfound;
    ss_scope_t  scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(ss_file_isopen(db, NULL)>0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DB must be a valid database"));
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));

    ss_file_topscope(db, &scope);
    if (name) SAF_SEARCH_S(SAF_Unit, key, mask, name, name);
    if (abbr) SAF_SEARCH_S(SAF_Unit, key, mask, abbr, abbr);
    if (url)  SAF_SEARCH_S(SAF_Unit, key, mask, url, url);
    if (SAF_ANY_DOUBLE!=scale) SAF_SEARCH(SAF_Unit, key, mask, scale, scale);
    if (SAF_ANY_DOUBLE!=offset) SAF_SEARCH(SAF_Unit, key, mask, offset, offset);
    if (SAF_ANY_DOUBLE!=logbase) SAF_SEARCH(SAF_Unit, key, mask, logbase, logbase);
    if (SAF_ANY_DOUBLE!=logcoef) SAF_SEARCH(SAF_Unit, key, mask, logcoef, logcoef);
    if (!SS_PERS_ISNULL(quant)) SAF_SEARCH(SAF_Unit, key, mask, quant, *quant);
    
    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_unit_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, NULL, NULL);
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
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Convenience function for finding a unit
 *
 * Description: This is a simple version of saf_find_unit() that takes fewer arguments.
 *
 * Return:      On success, a handle for the first unit found which has name NAME or abbreviation NAME in database DATABASE.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 20, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Unit *
saf_find_one_unit(SAF_Db *database,             /* The database in which to find the specified unit. */
                  const char *name,             /* The singular name of the unit to find, e.g., "meter". */
                  SAF_Unit *buf                 /* OUT: Optional unit handle to initialize and return. */
                  )
{
    SAF_ENTER(saf_find_one_unit, NULL);
#ifdef HASH_UNITS /*RPM DEBUGGING 2004-09-26*/
    size_t limit=1;
    buf = (SAF_Unit*)_saf_htab_find(UHash, _saf_hkey_str(name), &limit, (ss_pers_t*)buf);
    assert(1==limit);
#else
    int n;

    /* Look for unit by name */
    n = 1;
    saf_find_units(SAF_EACH, database, name, NULL, NULL, SAF_ANY_DOUBLE, SAF_ANY_DOUBLE, SAF_ANY_DOUBLE, SAF_ANY_DOUBLE,
                   NULL, &n, &buf);

    /* Look for unit by abbreviation */
    if (0==n) {
        n = 1;
        saf_find_units(SAF_EACH, database, NULL, name, NULL, SAF_ANY_DOUBLE, SAF_ANY_DOUBLE, SAF_ANY_DOUBLE, SAF_ANY_DOUBLE,
                       NULL, &n, &buf);
    }
#endif
    SAF_LEAVE(buf);
}


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Find the not applicable unit
 *
 * Description: Find and return the not applicable unit.
 *
 * Return:      On success, a handle for the first unit found.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Unit *
saf_find_unit_not_applicable(void)
{
  SAF_ENTER(saf_find_unit_not_applicable, NULL);
  SAF_LEAVE(&SAF_NOT_APPLICABLE_UNIT_g);
}
