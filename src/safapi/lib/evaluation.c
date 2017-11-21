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
 * Chapter:     Evaluation Types
 * Purpose:     Declare a new evaluation type
 *
 * Description: This function declares a new evaluation type with a unique identification number.
 *
 * Return:      A handle to the new evaluation type.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Eval *
saf_declare_evaluation(SAF_ParMode pmode,
                       SAF_Db *db,
                       const char *name,                /* Name of the evaluation type */
                       const char *url,                 /* An optional URL to the evaluation documentation */
                       SAF_Eval *buf                    /* OUT: Optional buffer to fill in and return */
                       )
{
    SAF_ENTER(saf_declare_evaluation, NULL);
    ss_scope_t          scope;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(NULL);
    ss_file_topscope(db, &scope);
    buf = (ss_evaluation_t*)ss_pers_new(&scope, SS_MAGIC(ss_evaluation_t), NULL, SAF_ALL==pmode?SS_ALLSAME:0U,
                                        (ss_pers_t*)buf, NULL);

    if (SAF_EACH==pmode) SS_PERS_UNIQUE(buf);
    ss_string_set(SS_EVALUATION_P(buf,name), name);
    ss_string_set(SS_EVALUATION_P(buf,url), url);

    SAF_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Evaluation Types
 * Purpose:     Describe an evaluation type
 *
 * Description: Breaks EVALUATION into its parts and returns them through pointers.
 *
 * Return:      A non-negative value indicates success, and a negative value indicates failure. On return, the output
 *              arguments NAME, URL, and ID will be initialized.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_evaluation(SAF_ParMode pmode,
                        SAF_Eval *evaluation,   /* Evaluation to describe */
                        char **name,            /* If non-null, on return points to malloc'd evaluation name if any */
                        char **url              /* If non-null, on return points to malloc'd URL if any */
                        )
{
    SAF_ENTER(saf_describe_evaluation, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
    SAF_REQUIRE(SS_EVALUATION(evaluation), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("EVALUATION must be a valid evaluation handle"));

    _saf_setupReturned_string(name, ss_string_ptr(SS_EVALUATION_P(evaluation,name)));
    _saf_setupReturned_string(url, ss_string_ptr(SS_EVALUATION_P(evaluation,url)));

    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Evaluation Types
 * Purpose:     Find evaluation types
 *
 * Description: The function allows the client to search for evaluation types in the database. The search may be limited by
 *              one or more criteria such as the name of the unit, etc.
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
saf_find_evaluations(SAF_ParMode pmode,
                     SAF_Db *db,                /* Database in which to limit the search. */
                     const char *name,          /* Optional name for which to search. */
                     const char *url,           /* Optional URL for which to search. */
                     int *num,                  /* For this and the succeeding argument [see Returned Handles]. */
                     SAF_Eval **found           /* For this and the preceding argument [see Returned Handles]. */
                     )
{
    SAF_ENTER(saf_find_evaluations, SAF_PRECONDITION_ERROR);
    SAF_KEYMASK(SAF_Eval, key, mask);
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
    if (name) SAF_SEARCH_S(SAF_Eval, key, mask, name, name);
    if (url)  SAF_SEARCH_S(SAF_Eval, key, mask, url, url);
    
    if (!found) {
        /* Count the matches */
        assert(num);
        nfound = SS_NOSIZE;
        ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound, SS_PERS_TEST, NULL);
        *num = nfound;
    } else if (!*found) {
        /* Find all matches; library allocates results */
        nfound = SS_NOSIZE;
        *found = (ss_evaluation_t*)ss_pers_find(&scope, (ss_pers_t*)key, mask_count?(ss_persobj_t*)&mask:NULL, 0, &nfound,
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
 * Chapter:     Evaluation Types
 * Purpose:     Find one evaluation type
 *
 * Description: This is a convenience version of saf_find_evaluation() that returns the first evaluation type it finds whose
 *              name matches that which is specified.
 *
 * Return:      A handle to a matching evaluation type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Robb Matzke
 *              Thursday, April 19, 2001
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Eval *
saf_find_one_evaluation(SAF_Db *database,               /* The database in which to search */
                        const char *name,               /* The name for which to search */
                        SAF_Eval *buf                   /* OUT: Optional buffer to fill in and return */
                        )
{
    SAF_ENTER(saf_find_one_evaluation,NULL);
    int         n=1;

    saf_find_evaluations(SAF_EACH, database, name, NULL, &n, &buf);
    SAF_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Evaluation Types
 * Purpose:     Find evaluation type constant
 *
 * Description: Find and return evaluation type constant
 *
 * Return:      A handle to a matching evaluation type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Eval *
saf_find_evaluation_constant(void)
{
  SAF_ENTER(saf_find_evaluation_constant, NULL);
  if(_SAF_GLOBALS.evaluation_constant == NULL){
      _SAF_GLOBALS.evaluation_constant = saf_find_one_evaluation(_SAF_GLOBALS.reg.std_file, "constant", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.evaluation_constant);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Evaluation Types
 * Purpose:     Find evaluation type piecewise constant
 *
 * Description: Find and return evaluation type piecewise constant
 *
 * Return:      A handle to a matching evaluation type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Eval *
saf_find_evaluation_piecewise_constant(void)
{
  SAF_ENTER(saf_find_evaluation_piecewise_constant, NULL);
  if(_SAF_GLOBALS.evaluation_piecewise_constant == NULL){
      _SAF_GLOBALS.evaluation_piecewise_constant = saf_find_one_evaluation(_SAF_GLOBALS.reg.std_file, "piecewise constant", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.evaluation_piecewise_constant);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Evaluation Types
 * Purpose:     Find evaluation type piecewise linear
 *
 * Description: Find and return evaluation type piecewise linear
 *
 * Return:      A handle to a matching evaluation type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Eval *
saf_find_evaluation_piecewise_linear(void)
{
  SAF_ENTER(saf_find_evaluation_piecewise_linear, NULL);
  if(_SAF_GLOBALS.evaluation_piecewise_linear == NULL){
      _SAF_GLOBALS.evaluation_piecewise_linear = saf_find_one_evaluation(_SAF_GLOBALS.reg.std_file, "piecewise linear", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.evaluation_piecewise_linear);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Evaluation Types
 * Purpose:     Find evaluation type uniform
 *
 * Description: Find and return evaluation type uniform
 *
 * Return:      A handle to a matching evaluation type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Eval *
saf_find_evaluation_uniform(void)
{
  SAF_ENTER(saf_find_evaluation_uniform, NULL);
  if(_SAF_GLOBALS.evaluation_uniform == NULL){
      _SAF_GLOBALS.evaluation_uniform = saf_find_one_evaluation(_SAF_GLOBALS.reg.std_file, "uniform", NULL);
  }
  SAF_LEAVE(_SAF_GLOBALS.evaluation_uniform);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Evaluation Types
 * Purpose:     Find evaluation type not applicable
 *
 * Description: Find and return evaluation type not applicable
 *
 * Return:      A handle to a matching evaluation type on success; SAF_ERROR_HANDLE on failure.
 *
 * Programmer:  Matthew O'Brien
 *              Friday, April 19, 2002
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Eval *
saf_find_evaluation_not_applicable(void)
{
  SAF_ENTER(saf_find_evaluation_not_applicable, NULL);
  SAF_LEAVE(&SAF_NOT_APPLICABLE_EVALUATION_g);
}
