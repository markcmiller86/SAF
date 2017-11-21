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

#ifndef SAF_EVALUATION_H
/*DOCUMENTED*/
#define SAF_EVALUATION_H

SAF_BIND_SSLIB(SAF_Eval, evaluation);
#define SAF_MAGIC_SAF_Eval SS_MAGIC(ss_evaluation_t)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Evaluation Types
 *
 * Description: SAF currently supports specification of a field's evaluation method by picking from a list of known methods
 *		Currently, that list is relatively short. SAF provides tags for specifying constant, piecewise linear and
 *		piecewise constant evaluations of a field.
 *
 *		Eventually, this list of evaluation methods will be expanded to include many of the common spline,
 *		and spectral evaluation schemes and they will also be user-definable. However, in this first implementation
 *		of SAF, we provide only an enumeration of the most commonly used evaluation methods.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef SAF_Eval SAF_EvalConstants;
#define SAF_SPACE_CONSTANT      saf_find_evaluation_constant()      /* identifies an evaluation method that is constant. This
                                                                     * is really just an alias for piecewise constant in which
                                                                     * there is only one piece. */
#define SAF_SPACE_PWCONST       saf_find_evaluation_piecewise_constant() /* identifies an evaluation method that is piecewise
                                                                     * constant. That is it is constant over each piece in the
                                                                     * EVAL_COLL argument of saf_declare_field(). */
#define SAF_SPACE_PWLINEAR      saf_find_evaluation_piecewise_linear() /* identifies an evaluation method that is piecewise linear. */
#define SAF_SPACE_UNIFORM       saf_find_evaluation_uniform()	    /* identifies an evaluation method that is a single piece
                                                                     * of linear evaluation such as is common for /uniform/
                                                                     * coordinate fields. */

#define SAF_ANY_EFUNC NULL

/* Prototypes for API functions */
#ifdef __cplusplus
extern "C" {
#endif

SAF_Eval *saf_declare_evaluation(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, SAF_Eval *buf);
int saf_describe_evaluation(SAF_ParMode pmode, SAF_Eval *evaluation, char **name, char **url);
int saf_find_evaluations(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, int *num, SAF_Eval **found);
SAF_Eval *saf_find_one_evaluation(SAF_Db *database, const char *name, SAF_Eval *buf);
SAF_Eval *saf_find_evaluation_constant(void);
SAF_Eval *saf_find_evaluation_piecewise_constant(void);
SAF_Eval *saf_find_evaluation_piecewise_linear(void);
SAF_Eval *saf_find_evaluation_uniform(void);
SAF_Eval *saf_find_evaluation_not_applicable(void);

#ifdef __cplusplus
}
#endif

#endif /* !SAF_EVALUATION_H */
