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
 * Chapter:     Quantities
 *
 * Description: A quantity in the general sense is a property ascribed to phenomena, bodies, or substances that can
 *              be quantified for, or assigned to, a particular phenomenon, body, or substance. The library defines seven
 *              basic quantities (length, mass, time, electric current, thermodynamic temperature, amount of a substance, and
 *              luminous intensity) and additional quantities can be derived as products of powers of the seven basic
 *              quantities (e.g., "volume" and "acceleration"). All quantities are unitless -- they describe what can be
 *              measured but not how to measure it.
 *
 *              Unlike many other quantity implementations, this one is able to distinguish between dimensionless things like
 *              mass fractions (mass/mass) and length fractions (length/length). It does so by canceling numerators with
 *              denominators except when the numerator and denominator are equal. That is, mass/mass is considered a different
 *              quantity than length/length.
 *
 *              The library defines the seven basic quantities whose names follow the format "SAF_QX" where "X" is
 *              replaced by one of the words LENGTH, MASS, TIME, CURRENT, TEMP, AMOUNT, or LIGHT. Additional quantities can be
 *              derived from these by first creating an empty quantity and then multiplying powers of other quantities. For
 *              instance, volume per unit time would be defined as
 *
 *                  SAF_Quantity *q_vpt = saf_declare_quantity(SAF_ALL, db, "volume per time", "vol/time", NULL);
 *                  saf_multiply_quantity(SAF_ALL, q_vpt, SAF_QLENGTH, 3);
 *                  saf_multiply_quantity(SAF_ALL, q_vpt, SAF_QTIME, -1);
 *
 *              The reader is encouraged to visit http://physics.nist.gov/cuu/Units/units.html to get more information
 *		about quantities and units.
 *--------------------------------------------------------------------------------------------------------------------------------
 */


#define HASH_QUANTITIES
#ifdef HASH_QUANTITIES /*RPM DEBUGGING 2004-09-26*/
static SAF_HashTable *QHash;
#endif

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     Declare a new quantity
 * Concepts:    Quantities, declaration of; Declaring quantities
 *
 * Description: This function declares a new quantity whose product of powers is unity. The client is expected to
 *              multiply powers of other quantities into this new quantity (via saf_multiply_quantity()) in order to complete
 *              its definition.
 *
 * Return:      A new quantity handle is returned on success. Otherwise a SAF_ERROR_HANDLE value is returned or an exception is
 *              raised, depending on the error handling property of the library.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 21, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_declare_quantity(SAF_ParMode pmode,
                     SAF_Db *db,
                     const char *description,   /* A short description of the new quantity (e.g., "volume per time"). */
                     const char *abbreviation,  /* An optional abbreviation or symbol name for the quantity. */
                     const char *url,           /* An optional URL to the quantity documentation. */
                     SAF_Quantity *quant        /* OUT: Optional quantity handle to initialize (and return). */
                     )
{
    SAF_ENTER(saf_declare_quantity, NULL);
    ss_scope_t          scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);
    ss_file_topscope(db, &scope);
    quant = (ss_quantity_t*)ss_pers_new(&scope, SS_MAGIC(ss_quantity_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U,
                                        (ss_pers_t*)quant, NULL);

    if (SAF_EACH==pmode) SS_PERS_UNIQUE(quant);
    ss_string_set(SS_QUANTITY_P(quant,name), description);
    ss_string_set(SS_QUANTITY_P(quant,abbr), abbreviation);
    ss_string_set(SS_QUANTITY_P(quant,url), url);

#ifdef HASH_QUANTITIES /*RPM DEBUGGING 2004-09-26*/
    /* Store the new quantity in the hash table by both description and abbreviation */
    if (!QHash) QHash = _saf_htab_new();
    _saf_htab_insert(QHash, _saf_hkey_str(description), (ss_pers_t*)quant);
    _saf_htab_insert(QHash, _saf_hkey_str(abbreviation), (ss_pers_t*)quant);
#endif
    
    SAF_LEAVE(quant);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     Multiply a quantity into a quantity definition
 * Concepts:    Quantities, multiplication of; Multiplying quantities
 *
 * Description: After creating a new quantity with saf_declare_quantity(), the QUANTITY is defined by multiplying powers
 *              of other quantities into it, one per call to this function. A division can be accomplished by supplying a
 *              negative POWER (a POWER of zero has no effect).
 *
 * Return:      This function returns some non-negative value on success; otherwise, it either returns a negative value or
 *              raises an exception, depending on the error handling property of the library.
 *
 * Parallel:	Depends on the PMODE argument.
 *
 * Also:        saf_divide_quantity() -- a macro which calls this function with a negated power.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 21, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_multiply_quantity(SAF_ParMode pmode,
                      SAF_Quantity *quantity,                   /* INOUT: The quantity which is affected by this operation */
                      SAF_Quantity *multiplier,                 /* What to multiply into QUANTITY */
                      int power                                 /* Number of times to multiply MULTIPLIER into QUANTITY */
                      )
{
    SAF_ENTER(saf_multiply_quantity, SAF_PRECONDITION_ERROR);
    int         i, recip;
    unsigned    dim_q=(SAF_DIMENSIONLESS_QUANTITY & SS_QUANTITY(quantity)->flags);
    unsigned    dim_m=(SAF_DIMENSIONLESS_QUANTITY & SS_QUANTITY(multiplier)->flags);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_QUANTITY(quantity), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("QUANTITY must be a valid quantity handle"));
    SAF_REQUIRE(SS_QUANTITY(multiplier), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("MULTIPLIER must be a valid quantity handle"));
                

    /* Determine if Q is the reciprocal of M^POWER */
    recip = (dim_q == dim_m);
    for (i=0; i<SS_MAX_BASEQS && recip; i++)
        if (SS_QUANTITY(quantity)->power[i] != -power*SS_QUANTITY(multiplier)->power[i]) recip=0;
    
    SAF_DIRTY(quantity, pmode);
    if (recip) {
        /* If Q and M are reciprocals then turn U into a dimensionless quntity. */
        SS_QUANTITY(quantity)->flags |= SAF_DIMENSIONLESS_QUANTITY;

    } else if (dim_q==dim_m) {
        /* If Q and M are both dimensioned quantities (the default) then multiply them in the normal cancelling fasion. If Q
         * and M are both dimensionless quantities then they should have only positive powers and they can be added in the
         * normal fasion (the powers refer to cancelling powers in the numerator and denominator). */
        for (i=0; i<SS_MAX_BASEQS; i++)
            SS_QUANTITY(quantity)->power[i] += power * SS_QUANTITY(multiplier)->power[i];

    } else if (dim_q) {
        /* If Q is dimensionless but M is dimensioned, then set Q to be M and turn off the dimensionless flag. In other words,
         * the powers originally in Q cancel themselves. */
        SS_QUANTITY(quantity)->flags &= ~SAF_DIMENSIONLESS_QUANTITY;
        for (i=0; i<SS_MAX_BASEQS; i++)
            SS_QUANTITY(quantity)->power[i] = power * SS_QUANTITY(multiplier)->power[i];
        
    } else {
        /* Q is dimensioned but M isn't. Just discard M since it would cancel itself anyway. */
    }
     
    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     Query quantity characteristics
 * Concepts:    Quantities, description of; Describing quantities
 *
 * Description: Given a QUANTITY this function returns any information which is known about that quantity.
 *
 * Return:      A non-negative value is returned on success. Failure is indicated by a negative return value or the raising of
 *              an exception, depending on the error handling property of the library.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 22, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_quantity(SAF_ParMode pmode,
                      SAF_Quantity *quantity,   /* Quantity about which to retrieve information. */
                      char **description,       /* If non-null then upon return this will point to an allocated copy of the
                                                 * quantity description. */
                      char **abbreviation,      /* If non-null then upon return this will point to an allocated copy of the
                                                 * quantity abbreviation if one is defined. */
                      char **url,               /* If non-null then upon return this will point to an allocated copy of the
                                                 * quantity documentation URL if one is defined. */
                      unsigned *flags,          /* If non-null then the special quantity flags are written into the location
                                                 * indicated by this pointer. */                                      
                      unsigned *power           /* If non-null then upon return this seven-element array will be filled in
                                                 * with the powers of the seven basic quantities. */
                      )
{
    SAF_ENTER(saf_describe_quantity, SAF_PRECONDITION_ERROR);
    int                 i;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_QUANTITY(quantity), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("QUANTITY must be a valid quantity handle"));

    _saf_setupReturned_string(description, ss_string_ptr(SS_QUANTITY_P(quantity,name)));
    _saf_setupReturned_string(url, ss_string_ptr(SS_QUANTITY_P(quantity,url)));
    _saf_setupReturned_string(abbreviation,ss_string_ptr(SS_QUANTITY_P(quantity,abbr)));

    if (flags) *flags = SS_QUANTITY(quantity)->flags;
    if (power)
        for (i=0; i<SS_MAX_BASEQS; i++) power[i] = SS_QUANTITY(quantity)->power[i];
    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     Find quantities
 *
 * Description: This function allows a client to search for quantities in the database. The search may be limited by one or
 *              more criteria such as the name of the quantity, etc.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Parallel:    Depends on PMODE
 *
 * Programmer:  Robb Matzke
 *              Friday, April  2, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_quantities(SAF_ParMode pmode,
                    SAF_Db *db,                 /* Database in which to limit the search. */
                    const char *desc,           /* Optional quantity description for which to search. */
                    const char *abbr,           /* Optional abbreviation for which to search. */
                    const char *url,            /* Optional URL for which to search. */
                    unsigned flags,             /* Optional flags for which to search, or SAF_ANY_INT. */
                    int *power,                 /* Optional base quantity powers for which to search. If the pointer is
                                                 * non-null then the elements can be SAF_ANY_INT for the ones in which the
                                                 * caller is not interested. */
                    int *num,                   /* For this and the succeeding argument [see Returned Handles]. */
                    SAF_Quantity **found        /* For this and the preceding argument [see Returned Handles]. */
                    )
{
    SAF_ENTER(saf_find_quantities, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Quantity, key, mask);
    size_t      nfound;
    ss_scope_t  scope;
    int         i;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(ss_file_isopen(db, NULL)>0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DB must be a valid database"));
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));

    ss_file_topscope(db, &scope);
    if (desc) SAF_SEARCH_S(SAF_Quantity, key, mask, name, desc);
    if (abbr) SAF_SEARCH_S(SAF_Quantity, key, mask, abbr, abbr);
    if (url)  SAF_SEARCH_S(SAF_Quantity, key, mask, url, url);
    if (SAF_ANY_INT!=flags) SAF_SEARCH(SAF_Quantity, key, mask, flags, flags);
    if (power) {
        for (i=0; i<SS_MAX_BASEQS; i++) {
            if (SAF_ANY_INT!=power[i]) SAF_SEARCH(SAF_Quantity, key, mask, power[i], power[i]);
        }
    }
    
    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_quantity_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound,
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
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     Convenience function for finding a quantity
 *
 * Description: This is a simple version of saf_find_quantity() that takes fewer arguments.
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April  4, 2000
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_one_quantity(SAF_Db *database,         /* The database in which to find the specified quantity. */
                      const char *desc,         /* Quantity description to find. */
                      SAF_Quantity *buf         /* OUT: Optional quantity handle to initialize and return. */
                      )
{
    SAF_ENTER(saf_find_one_quantity, NULL);
#ifdef HASH_QUANTITIES /*RPM DEBUGGING 2004-09-26*/
    size_t limit=1;
    buf = (SAF_Quantity*)_saf_htab_find(QHash, _saf_hkey_str(desc), &limit, (ss_pers_t*)buf);
    assert(1==limit);
#else
    int n;

    /* Look for quantity by name */
    n = 1;
    saf_find_quantities(SAF_EACH, database, desc, NULL, NULL, SAF_ANY_INT, NULL, &n, &buf);

    /* Look for quantity by abbreviation */
    if (0==n) {
        n = 1;
        saf_find_quantities(SAF_EACH, database, NULL, desc, NULL, SAF_ANY_INT, NULL, &n, &buf);
    }
#endif

    SAF_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Find quantity not applicable
 *
 * Description: Find and return quantity not applicable
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April  19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_quantity_not_applicable(void)
{
  SAF_ENTER(saf_find_quantity_not_applicable, NULL);
  SAF_LEAVE(&SAF_NOT_APPLICABLE_QUANTITY_g);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Find quantity time
 *
 * Description: Find and return quantity time
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April  19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_quantity_time(void)
{
  SAF_ENTER(saf_find_quantity_time, NULL);
  SAF_LEAVE(_SAF_GLOBALS.quant[SAF_BASEQ_TIME]);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Find quantity mass
 *
 * Description: Find and return quantity mass
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April  19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_quantity_mass(void)
{
  SAF_ENTER(saf_find_quantity_mass, NULL);
  SAF_LEAVE(_SAF_GLOBALS.quant[SAF_BASEQ_MASS]);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Find quantity electric current
 *
 * Description: Find and return quantity electric current
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April  19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_quantity_electric_current(void)
{
  SAF_ENTER(saf_find_quantity_electric_current, NULL);
  SAF_LEAVE(_SAF_GLOBALS.quant[SAF_BASEQ_CURRENT]);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Find quantity length
 *
 * Description: Find and return quantity length
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April  19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_quantity_length(void)
{
  SAF_ENTER(saf_find_quantity_length, NULL);
  SAF_LEAVE(_SAF_GLOBALS.quant[SAF_BASEQ_LENGTH]);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Find quantity luminous intensity
 *
 * Description: Find and return quantity luminous intensity
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April  19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_quantity_luminous_intensity(void)
{
  SAF_ENTER(saf_find_quantity_luminous_intensity, NULL);
  SAF_LEAVE(_SAF_GLOBALS.quant[SAF_BASEQ_LIGHT]);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Find quantity thermodynamic temperature
 *
 * Description: Find and return quantity thermodynamic temperature
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April  19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_quantity_thermodynamic_temperature(void)
{
  SAF_ENTER(saf_find_quantity_thermodynamic_temperature, NULL);
  SAF_LEAVE(_SAF_GLOBALS.quant[SAF_BASEQ_TEMP]);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Quantities
 * Purpose:     Find quantity amount of a substance
 *
 * Description: Find and return quantity amount of a substance
 *
 * Return:      On success, a handle for the first quantity found which has description DESC in database DATABASE is returned.
 *              Otherwise a SAF_ERROR_HANDLE is returned.
 *
 * Parallel:	This function must be called collectively in the database communicator.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April  19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Quantity *
saf_find_quantity_amount_of_a_substance(void)
{
  SAF_ENTER(saf_find_quantity_amount_of_a_substance, NULL);
  SAF_LEAVE(_SAF_GLOBALS.quant[SAF_BASEQ_AMOUNT]);
}
