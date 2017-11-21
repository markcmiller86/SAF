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
#ifndef NAMES_H
/*DOCUMENTED*/
#define NAMES_H

#include <safP.h>

/*mechanism for returning strings*/

char *saf_next_string(void);
const char *StringDataType(hid_t type);
char *StringDataTypeValue(hid_t type, ss_pers_t *value);

/*getting the name of saf objects*/
char *set_name(SAF_Set *set);
char *cat_name(SAF_Cat *cat);
char *collection_name(SAF_Set *set, SAF_Cat *cat);
char *field_name(SAF_Field *field);
char *field_tmpl_name(SAF_FieldTmpl *fieldtmpl);
char *state_tmpl_name(SAF_StateTmpl *statetmpl);
char *suite_name(SAF_Suite *suite);
char *algebraic_name(SAF_Algebraic *algebraic);
char *basis_name(SAF_Basis *algebraic);


/*for sorting*/
int compare_set_names(ss_pers_t *left, ss_pers_t *right);
int compare_cat_names(ss_pers_t *left, ss_pers_t *right);
int compare_field_names(ss_pers_t *left, ss_pers_t *right);
int compare_field_tmpl_names(ss_pers_t *left, ss_pers_t *right);
int compare_state_tmpl_names(ss_pers_t *left, ss_pers_t *right);
int compare_suite_names(ss_pers_t *left, ss_pers_t *right);


/*some type to string*/
const char *str_SAF_SilRole(SAF_SilRole role);
const char *str_hid_t(hid_t type);
const char *str_SAF_BoundMode(SAF_BoundMode mode);
const char *str_SAF_ExtendMode(SAF_ExtendMode mode);
const char *str_hbool_t(hbool_t t);
const char *str_SAF_TopMode(SAF_TopMode mode);
const char *str_SAF_TriState(SAF_TriState s);
const char *str_SAF_Interleave(SAF_Interleave t);
const char *str_SAF_RelRep(SAF_RelRep *t);
const char *str_SAF_Algebraic(SAF_Algebraic *t);
const char *str_SAF_ObjectType(ss_pers_t *object);

#endif /* !NAMES_H */
