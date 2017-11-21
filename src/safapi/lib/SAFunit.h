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
#ifndef SAF_UNIT_H
/*DOCUMENTED*/
#define SAF_UNIT_H

SAF_BIND_SSLIB(SAF_Unit, unit);
#define SAF_MAGIC_SAF_Unit SS_MAGIC(ss_unit_t)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Units
 * Purpose:     Divide a unit into a unit definition
 * Concepts:    Units, division of; Dividing units
 *
 * Description: This macro simply calls saf_multiply_unit() with a negated POWER argument and the reciprocal of the SCALE
 *              argument.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 21, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define saf_divide_unit(U,SCALE,DIVISOR,POWER)      saf_multiply_unit((U),1.0/(SCALE),(DIVISOR),-(POWER))

#define SAF_ANY_UNIT NULL

/* Prototypes for public functions */
#ifdef __cplusplus
extern "C" {
#endif

SAF_Unit *saf_declare_unit(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *abbr, const char *url, SAF_Unit *unit);
int saf_quantify_unit(SAF_ParMode pmode, SAF_Unit *unit, SAF_Quantity *quantity, double scale);
int saf_multiply_unit(SAF_ParMode pmode, SAF_Unit *unit, double coef, SAF_Unit *multiplier, int power);
int saf_offset_unit(SAF_ParMode pmode, SAF_Unit *unit, double offset);
int saf_log_unit(SAF_ParMode pmode, SAF_Unit *unit, double logbase, double logcoef);
int saf_describe_unit(SAF_ParMode pmode, SAF_Unit *unit, char **name/*out*/, char **abbr/*out*/, char **url/*out*/,
                      double *scale/*out*/, double *offset/*out*/, double *logbase/*out*/, double *logcoef/*out*/,
                      SAF_Quantity *quantity/*out*/);
int saf_find_units(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *abbr, const char *url, double scale,
                   double offset, double logbase, double logcoef, SAF_Quantity *quant, int *num, SAF_Unit **found);
SAF_Unit *saf_find_one_unit(SAF_Db *db, const char *name, SAF_Unit *buf);
SAF_Unit *saf_find_unit_not_applicable(void);

#ifdef __cplusplus
}
#endif

#endif /* !SAF_UNIT_H */
