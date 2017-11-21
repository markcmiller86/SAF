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
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Algebraic Types
 * Purpose:     Declare a new algebraic type
 *
 * Description: This function declares a new algebraic type with a unique identification number.
 *
 * Return:      A handle to the new algebraic type.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_declare_algebraic(SAF_ParMode pmode,
                      SAF_Db *db,                       /* The database in which to create the new algebraic type */
                      const char *name,                 /* Name of the algebraic type */
                      const char *url,                  /* An optional URL to the algebraic documentation */
                      hbool_t indirect,                 /* If true then field is indirection to another field */
                      SAF_Algebraic *alg                /* OUT: Optional handle to initialize (and return) */
                      )
{
    SAF_ENTER(saf_declare_algebraic, NULL);
    ss_scope_t          topscope;

    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);
    ss_file_topscope(db, &topscope);
    alg = (ss_algebraic_t*)ss_pers_new(&topscope, SS_MAGIC(ss_algebraic_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U,
                                       (ss_pers_t*)alg, NULL);

    if (SAF_EACH==pmode) SS_PERS_UNIQUE(alg);
    ss_string_set(SS_ALGEBRAIC_P(alg,name), name);
    ss_string_set(SS_ALGEBRAIC_P(alg,url), url);
    SS_ALGEBRAIC(alg)->indirect = indirect;

    SAF_LEAVE(alg);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Algebraic Types
 * Purpose:     Describe an algebraic type
 *
 * Description: Breaks ALGEBRAIC into its parts and returns them through pointers.
 *
 * Return:      A non-negative value indicates success, and a negative value indicates failure. On return, the output
 *              arguments NAME, URL, INDIRECT, and ID will be initialized.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_algebraic(SAF_ParMode pmode,
                       SAF_Algebraic *alg,      /* Algebraic to describe */
                       char **name,             /* If non-null, on return points to malloc'd algebraic name if any */
                       char **url,              /* If non-null, on return points to malloc'd URL if any */
                       hbool_t *indirect        /* If non-null, on return points to non-zero if type is indirect */
                       )
{
    SAF_ENTER(saf_describe_algebraic, SAF_PRECONDITION_ERROR);

    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_ALGEBRAIC(alg), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("ALG must be a valid algebraic handle"));

    _saf_setupReturned_string(name, ss_string_ptr(SS_ALGEBRAIC_P(alg,name)));
    _saf_setupReturned_string(url, ss_string_ptr(SS_ALGEBRAIC_P(alg,url)));
    if (indirect) *indirect = SS_ALGEBRAIC(alg)->indirect;

    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Algebraic Types
 * Purpose:     Find algebraic types
 *
 * Description: This function allows a client to search for algebraic types in the database. The search may be limited by one
 *              or more criteria such as the name fo the algebraic type, etc.
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
saf_find_algebraics(SAF_ParMode pmode,
                    SAF_Db *db,                 /* Database in which to limit the search. */
                    const char *name,           /* Optional name for which to search. */
                    const char *url,            /* Optional URL for which to search. */
                    htri_t indirect,            /* Optional indirect flag for which to search. The caller should pass a
                                                 * negative value if it is not interested in restricting the search. */
                    int *num,                   /* For this and the succeeding argument [see Returned Handles]. */
                    SAF_Algebraic **found       /* For this and the preceding argument [see Returned Handles]. */
                    )
{
    SAF_ENTER(saf_find_algebraics, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Algebraic, key, mask);
    size_t              nfound;
    ss_scope_t          scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(ss_file_isopen(db, NULL)>0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DB must be a valid database"));
    SAF_REQUIRE(_saf_valid_memhints(num, (void**)found), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("NUM and FOUND must be compatible for return value allocation"));

    ss_file_topscope(db, &scope);
    if (name) SAF_SEARCH_S(SAF_Algebraic, key, mask, name, name);
    if (url)  SAF_SEARCH_S(SAF_Algebraic, key, mask, url, url);
    if (indirect>=0) SAF_SEARCH(SAF_Algebraic, key, mask, indirect, indirect?TRUE:FALSE);

    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_algebraic_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound,
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
 * Chapter:     Algebraic Types
 * Purpose:     Find one algebraic type
 *
 * Description: This is a convenience version of saf_find_algebraic() that returns the first algebraic type it finds whose
 *              name matches that which is specified.
 *
 * Return:      A handle to a matching algebraic type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_find_one_algebraic(SAF_Db *database,                /* The database in which to search */
                       const char *name,                /* The name for which to search */
                       SAF_Algebraic *buf               /* OUT: Optional algebraic handle to initialize and return */
                       )
{
    SAF_ENTER(saf_find_one_algebraic, NULL);
    int                 n=1;

    saf_find_algebraics(SAF_EACH, database, name, NULL, SAF_TRISTATE_TORF, &n, &buf);
    SAF_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Algebraic Types
 * Purpose:     Find algebraic type scalar
 *
 * Description: This finds and returns the algebraic type scalar
 *
 * Return:      A handle to a matching algebraic type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_find_algebraic_scalar(void)
{
  SAF_ENTER(saf_find_algebraic_scalar, NULL);
  if(_SAF_GLOBALS.algebraic_scalar == NULL){
      _SAF_GLOBALS.algebraic_scalar = saf_find_one_algebraic(_SAF_GLOBALS.reg.std_file, "scalar", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.algebraic_scalar);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Algebraic Types
 * Purpose:     Find algebraic type vector
 *
 * Description: This finds and returns the algebraic type vector
 *
 * Return:      A handle to a matching algebraic type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_find_algebraic_vector(void)
{
  SAF_ENTER(saf_find_algebraic_vector, NULL);
  if(_SAF_GLOBALS.algebraic_vector == NULL){
      _SAF_GLOBALS.algebraic_vector = saf_find_one_algebraic(_SAF_GLOBALS.reg.std_file, "vector", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.algebraic_vector);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Algebraic Types
 * Purpose:     Find algebraic type component
 *
 * Description: This finds and returns the algebraic type component
 *
 * Return:      A handle to a matching algebraic type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_find_algebraic_component(void)
{
  SAF_ENTER(saf_find_algebraic_component, NULL);
  if(_SAF_GLOBALS.algebraic_component == NULL){
      _SAF_GLOBALS.algebraic_component = saf_find_one_algebraic(_SAF_GLOBALS.reg.std_file, "component", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.algebraic_component);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Algebraic Types
 * Purpose:     Find algebraic type tensor
 *
 * Description: This finds and returns the algebraic type tensor
 *
 * Return:      A handle to a matching algebraic type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_find_algebraic_tensor(void)
{
  SAF_ENTER(saf_find_algebraic_tensor, NULL);
  if(_SAF_GLOBALS.algebraic_tensor == NULL){
      _SAF_GLOBALS.algebraic_tensor = saf_find_one_algebraic(_SAF_GLOBALS.reg.std_file, "tensor", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.algebraic_tensor);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Algebraic Types
 * Purpose:     Find algebraic type symmetric tensor
 *
 * Description: This finds and returns the algebraic type symmetric tensor
 *
 * Return:      A handle to a matching algebraic type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_find_algebraic_symmetric_tensor(void)
{
  SAF_ENTER(saf_find_algebraic_symmetric_tensor, NULL);
  if(_SAF_GLOBALS.algebraic_symmetric_tensor == NULL){
      _SAF_GLOBALS.algebraic_symmetric_tensor = saf_find_one_algebraic(_SAF_GLOBALS.reg.std_file, "symmetric tensor", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.algebraic_symmetric_tensor);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Algebraic Types
 * Purpose:     Find algebraic type tuple
 *
 * Description: This finds and returns the algebraic type tuple
 *
 * Return:      A handle to a matching algebraic type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_find_algebraic_tuple(void)
{
  SAF_ENTER(saf_find_algebraic_tuple, NULL);
  if(_SAF_GLOBALS.algebraic_tuple == NULL){
      _SAF_GLOBALS.algebraic_tuple = saf_find_one_algebraic(_SAF_GLOBALS.reg.std_file, "tuple", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.algebraic_tuple);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Algebraic Types
 * Purpose:     Find algebraic type field
 *
 * Description: This finds and returns the algebraic type field
 *
 * Return:      A handle to a matching algebraic type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Algebraic *
saf_find_algebraic_field(void)
{
  SAF_ENTER(saf_find_algebraic_field, NULL);
  if(_SAF_GLOBALS.algebraic_field == NULL){
      _SAF_GLOBALS.algebraic_field = saf_find_one_algebraic(_SAF_GLOBALS.reg.std_file, "field", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.algebraic_field);
}
