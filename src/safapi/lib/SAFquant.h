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

#ifndef SAF_QUANT_H
/*DOCUMENTED*/
#define SAF_QUANT_H

SAF_BIND_SSLIB(SAF_Quantity, quantity);
#define SAF_MAGIC_SAF_Quantity SS_MAGIC(ss_quantity_t)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     Basic quantity indices
 *
 * Description: A quantity is a vector of seven integer powers, the powers corresponding to the seven basic quantities
 *              described by this enumeration type.
 *--------------------------------------------------------------------------------------------------------------------------------
 */
typedef ss_basequant_t SAF_BaseQuant;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities 
 * Purpose:     The quantity Time
 * Concept:     Quantities, pre-defined; Base quantities 
 *
 * Description: A macro which refers to /Time/, one of the 7 basic quantities defined at
 *              http://physics.nist.gov/cuu/Units/units.html */
#define SAF_QTIME           saf_find_quantity_time()

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     The quantity Mass
 * Concept:     Quantities, pre-defined; Base quantities 
 *
 * Description: A macro which refers to /Mass/, one of the 7 basic quantities defined at
 *              http://physics.nist.gov/cuu/Units/units.html */
#define SAF_QMASS           saf_find_quantity_mass()

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     The quantity Current
 * Concept:     Quantities, pre-defined; Base quantities 
 *
 * Description: A macro which refers to /Current/, one of the 7 basic quantities defined at
 *              http://physics.nist.gov/cuu/Units/units.html */
#define SAF_QCURRENT        saf_find_quantity_electric_current()

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     The quantity Length
 * Concept:     Quantities, pre-defined; Base quantities 
 *
 * Description: A macro which refers to /Length/, one of the 7 basic quantities defined at
 *              http://physics.nist.gov/cuu/Units/units.html */
#define SAF_QLENGTH         saf_find_quantity_length()

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     The quantity Light
 * Concept:     Quantities, pre-defined; Base quantities 
 *
 * Description: A macro which refers to /Light/, one of the 7 basic quantities defined at
 *              http://physics.nist.gov/cuu/Units/units.html */
#define SAF_QLIGHT          saf_find_quantity_luminous_intensity()

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     The quantity Temperature
 * Concept:     Quantities, pre-defined; Base quantities 
 *
 * Description: A macro which refers to /Temperature/, one of the 7 basic quantities defined at
 *              http://physics.nist.gov/cuu/Units/units.html */
#define SAF_QTEMP           saf_find_quantity_thermodynamic_temperature()

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     The quantity Amount
 * Concept:     Quantities, pre-defined; Base quantities 
 *
 * Description: A macro which refers to /Amount/, one of the 7 basic quantities defined at
 *              http://physics.nist.gov/cuu/Units/units.html */
#define SAF_QAMOUNT         saf_find_quantity_amount_of_a_substance()

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     An arbitrary named quantity     
 * Concept:     Quantities, finding;
 *
 * Description: A macro which refers to an arbitrary named quantity */
#define SAF_QNAME(DB, NAME)     saf_find_one_quantity((DB),(NAME), NULL)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Quantities
 * Purpose:     Divide a quantity into a quantity definition
 * Concepts:    Quantities, division of; Dividing quantities
 *
 * Description: This macro simply calls saf_multiply_quantity() with a negated POWER argument.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, March 21, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define saf_divide_quantity(PMODE,Q,DIVISOR,POWER)      saf_multiply_quantity(PMODE,(Q),(DIVISOR),-(POWER))

#define SAF_ANY_QUANTITY NULL

/* Prototypes for public functions */
#ifdef __cplusplus
extern "C" {
#endif

SAF_Quantity *saf_declare_quantity(SAF_ParMode pmode, SAF_Db *db, const char *description, const char *abbreviation,
                                   const char *url, SAF_Quantity *quant);
int saf_multiply_quantity(SAF_ParMode pmode, SAF_Quantity *quantity, SAF_Quantity *multiplier, int power);
int saf_describe_quantity(SAF_ParMode pmode, SAF_Quantity *quantity, char **description, char **abbreviation, char **url,
                          unsigned *flags, unsigned *power);
int saf_find_quantities(SAF_ParMode pmode, SAF_Db *db, const char *desc, const char *abbr, const char *url, unsigned flags,
                        int power[SS_MAX_BASEQS], int *num, SAF_Quantity **found);
SAF_Quantity *saf_find_one_quantity(SAF_Db *db, const char *description, SAF_Quantity *buf);
SAF_Quantity *saf_find_quantity_not_applicable(void);
SAF_Quantity *saf_find_quantity_time(void);
SAF_Quantity *saf_find_quantity_mass(void);
SAF_Quantity *saf_find_quantity_electric_current(void);
SAF_Quantity *saf_find_quantity_length(void);
SAF_Quantity *saf_find_quantity_luminous_intensity(void);
SAF_Quantity *saf_find_quantity_thermodynamic_temperature(void);
SAF_Quantity *saf_find_quantity_amount_of_a_substance(void);

#ifdef __cplusplus
}
#endif

#endif /* !SAF_DB_H */
