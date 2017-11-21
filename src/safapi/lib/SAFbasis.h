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

#ifndef SAF_BASIS_H
/*DOCUMENTED*/
#define SAF_BASIS_H

SAF_BIND_SSLIB(SAF_Basis, basis);
#define SAF_MAGIC_SAF_Basis SS_MAGIC(ss_basis_t)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Basis types
 *
 * Description:	For every field, not just coordinate fields, SAF needs to be told what are the basis vectors for identifying
 *		the field's values. For example, if we have a field of /N/ pairs of floats representing complex numbers,
 *		do those floats represent the real and imaginary part of the complex number (e.g. cartesian basis) or do
 *		they represent the magnitude and phase (e.g. the polar basis).
 *
 *		Likewise, if we have /N/ triples representing color of each pixel in image are they RGB triples, LUV triples,
 *		YIQ triples, etc.? The basis type is designed to indicate what the basis vectors for a given field are.
 *		
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef SAF_Basis SAF_BasisConstants;
#define SAF_UNITY               saf_find_basis_unity() 		/* The basis set with a single basis vector; {1} */
#define SAF_CARTESIAN           saf_find_basis_cartesian()      /* The basis set with /N/ basis vectors; {e0, e1, ..., eN} */
#define SAF_SPHERICAL           saf_find_basis_spherical()      /* The basis set with 3 basis vectors {r, theta, phi} */
#define SAF_CYLINDRICAL         saf_find_basis_cylindrical()    /* The basis set with 3 basis vectors {r, theta, h} */
#define SAF_UPPERTRI            saf_find_basis_uppertri()       /* The basis set of a symmetric tensor. Why do we need this if
								 * the algebraic type already captures it? */
#define SAF_VARIYING            saf_find_basis_variable()       /* For a basis that is varying over the base space. Often needed
								 * if the basis is derived from local surface behavior such as
								 * surface normals. Although, shouldn't we use something like
								 * SAF_SURFACE_NORMAL for that? */
#define SAF_ANY_BASIS           NULL                            /* Wildcard for searching. */

/* Prototypes for API functions */
#ifdef __cplusplus
extern "C" {
#endif

SAF_Basis *saf_declare_basis(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, SAF_Basis *basis);
int saf_describe_basis(SAF_ParMode pmode, SAF_Basis *basis, char **name/*out*/, char **url/*out*/);
int saf_find_bases(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, int *num_bases, SAF_Basis **bases);
SAF_Basis *saf_find_one_basis(SAF_Db *database, const char *name, SAF_Basis *buf);
SAF_Basis *saf_find_basis_unity(void);
SAF_Basis *saf_find_basis_cartesian(void);
SAF_Basis *saf_find_basis_spherical(void);
SAF_Basis *saf_find_basis_cylindrical(void);
SAF_Basis *saf_find_basis_uppertri(void);
SAF_Basis *saf_find_basis_variable(void);

#ifdef __cplusplus
}
#endif

#endif /* !SAF_BASIS_H */
