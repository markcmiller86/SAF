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

#ifndef SAF_ALGEBRAIC_H
/*DOCUMENTED*/
#define SAF_ALGEBRAIC_H

SAF_BIND_SSLIB(SAF_Algebraic, algebraic);
#define SAF_MAGIC_SAF_Algebraic SS_MAGIC(ss_algebraic_t)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Algebraic Types
 * Purpose:     Common algebraic types
 *
 * Description: SAF supports the characterization of various algebraic types for fields. An algebraic type specifies
 *		the algebraic properties of the field.
 *
 *		We should probably identify an algebraic type for a /Barycentric/ field; a field whose components are
 *		between 0.0 and 1.0 and which sum to 1.0.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ALGTYPE_SCALAR      saf_find_algebraic_scalar()     /* Used to specify fields that obey properties of scalar
								 * algebra. */
#define SAF_ALGTYPE_VECTOR      saf_find_algebraic_vector()     /* Used, generically, for fields that obey properties of vector
								 * algebra. */
#define SAF_ALGTYPE_COMPONENT   saf_find_algebraic_component()  /* Used, generically, for any component of a multi-component
								 * field. In many cases, it might be just as well to treat
								 * each component of a multi-component field as a scalar field.
								 * However, this is not entirely mathematically correct. */ 
#define SAF_ALGTYPE_TENSOR      saf_find_algebraic_tensor()     /* Used for general, non-symmetric tensor fields */
#define SAF_ALGTYPE_SYMTENSOR   saf_find_algebraic_symmetric_tensor()   /* Used for general, symmetric tensor fields. */
#define SAF_ALGTYPE_TUPLE	saf_find_algebraic_tuple()      /* Used to identify a field which evaluates to a /group/ of
								 * otherwise unrelated fields. Typically used in a /State/
								 * field. */
#define SAF_ALGTYPE_FIELD       saf_find_algebraic_field()      /* This algebraic type is used for fields that are, in reality,
								 * simply /references/ to other fields. These are called
								 * /field/indirections/ or, /indirect/fields. Indirect fields are
								 * used, primarily for two kinds of fields; /inhomogeneous/ fields
								 * and /cross-product/ fields. An inhomogeneous field is
								 * represented as references to pieces of the field over subsets
								 * of its base-space over which each piece *is* homogenous.
								 * Likewise, a cross-product field is used to work around the
								 * fact that SAF does NOT deal with cross product sets in the
								 * base-spaces of fields. Thus, we represent such fields as
								 * references to fields over other base spaces. */ 
#define SAF_ALGTYPE_ANY         NULL                            /* Wildcard for find operations. */


/* Prototypes for API functions */
#ifdef __cplusplus
extern "C" {
#endif

SAF_Algebraic *saf_declare_algebraic(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, hbool_t indirect,
                                     SAF_Algebraic *buf);
int saf_describe_algebraic(SAF_ParMode pmode, SAF_Algebraic *algebraic, char **name, char **url, hbool_t *indirect);
int saf_find_algebraics(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, htri_t indirect, int *num,
                        SAF_Algebraic **found);
SAF_Algebraic *saf_find_one_algebraic(SAF_Db *database, const char *name, SAF_Algebraic *buf);
SAF_Algebraic *saf_find_algebraic_scalar(void);
SAF_Algebraic *saf_find_algebraic_vector(void);
SAF_Algebraic *saf_find_algebraic_component(void);
SAF_Algebraic *saf_find_algebraic_tensor(void);
SAF_Algebraic *saf_find_algebraic_symmetric_tensor(void);
SAF_Algebraic *saf_find_algebraic_tuple(void);
SAF_Algebraic *saf_find_algebraic_field(void);

#ifdef __cplusplus
}
#endif

#endif /* !SAF_ALGEBRAIC_H */
