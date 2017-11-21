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
#ifndef SAF_RELREP_H
/*DOCUMENTED*/
#define SAF_RELREP_H

SAF_BIND_SSLIB(SAF_RelRep, relrep);
#define SAF_MAGIC_SAF_RelRep SS_MAGIC(ss_relrep_t)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Relation representation types
 *
 * Description: There are three basic classes of topology supported by SAF; N-dimensional rectangular structured topology,
 *		unstructured, finite element zoo topology and completely arbitrary topology. These three tags are used
 *		to define which class is being used in a saf_declare_topology_relation() call.
 *
 *		In future versions of SAF, user defined cell types will be supported. Thus, the /zoo/ from which element
 *		types are used in defining topology will eventually be filled with whatever cell-types the client needs.
 *
 *		Also, in future versions of SAF, structured topology will be represented by a structuring template similar
 *		to the notion of a stencil in finite difference computations. This would permit the characterization of
 *		hexagonal grids, triangle-strips, etc.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef SAF_RelRep SAF_TopoRelRep;
#define SAF_STRUCTURED          saf_find_relrep_structured()	/* N-dimensional rectangular topology */
#define SAF_UNSTRUCTURED        saf_find_relrep_unstructured()	/* unstructured, finite element zoo topology */
#define SAF_ARBITRARY           saf_find_relrep_arbitrary()	/* arbitrary topology */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Subset relation representation types
 * Concept:	Subset relations, declaring;
 *
 * Description: The subset relationship between a superset and a subset can take many forms. In theory, the subset relation
 *		identifies every member of the superset that is *in* the subset. In practice, depending on the nature of the
 *		indexing schemes used to identify members of collections on the superset and subset, there are a number of
 *		different ways a client may /represent/ a subset relationship. In an unstructured gridded code, the natural
 *		thing to do is simply enumerate each member of the superset in the subset by listing them. In a structured gridded
 *		code, the natural approach is to specify a hyperslab (or hypersample). Another natural approach for a structured
 *		gridded code is to specify a chain-code boundary where everything surrounded by the boundary is in the
 *		subset. This latter form is *not*yet* supported by SAF.
 *
 * Issues:	These representational issues raise a more fundamental question. Is the act of defining a subset one of
 *		enumerating *every* point of the superset that is in the subset or can it also be achieved by enumerating a
 *		boundary in the superset where everything /inside/ the boundary is in the subset? In other words, do we deal
 *		only with solid representations or both solid and boundary representations for sets?
 *
 *		We do *not* support a list of hyperslabs (hypersamples) due to the confusion of this representation with
 *		the union of a number of individual sets which are hyperslab subsets of some parent superset.
 *--------------------------------------------------------------------------------------------------------------------------------
 */
typedef SAF_RelRep SAF_SubsetRelRep;
#define SAF_HSLAB	saf_find_relrep_hslab()                 /* Indicates a hyperslab which is stored as 3 N-tuples; N
                                                                 * indices for the start value in each of the N dimensions,
                                                                 * followed by N indices for the count in each of the N
                                                                 * dimensions followed by N indices for stride in each of the
                                                                 * N dimensions. Use a stride of 1 for each of the N
                                                                 * dimensions if you do *not* have a hypersample. */
#define SAF_TUPLES	saf_find_relrep_tuples()	        /* Indicates a list of N-tuples. Each N-tuple identifies one
                                                                 * member of an N dimensionally indexed collection. */
#define SAF_TOTALITY	saf_find_relrep_totality()	        /* Indicates that all members of the collection are
                                                                 * involved--which probably also means the subset is equal to
                                                                 * the superset. Perhaps a better name for this value would be
                                                                 * SAF_IDENTITY. However, that is being used elsewhere.
                                                                 * Typically, this value is only ever used during a
                                                                 * saf_write_field() call. */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Datatypes
 * Purpose:     Relation representation ID numbers
 *
 * Description: Some parts of saf check for specific relation representations (and are thus not very extensible). They use
 *              these ID numbers to identify certain representations.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
typedef int SAF_RelRepID;
#define SAF_STRUCTURED_ID       1
#define SAF_UNSTRUCTURED_ID     2
#define SAF_ARBITRARY_ID        3
#define SAF_HSLAB_ID            4
#define SAF_TUPLES_ID           5
#define SAF_TOTALITY_ID         6

/* Prototypes for API functions */
#ifdef __cplusplus
extern "C" {
#endif

SAF_RelRep *saf_declare_relrep(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, int id, SAF_RelRep *buf);
int saf_describe_relrep(SAF_ParMode pmode, SAF_RelRep *rep, char **name/*out*/, char **url/*out*/, int *id/*out*/);
int saf_find_relreps(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, int id, int *num, SAF_RelRep **found);
SAF_RelRep *saf_find_one_relrep(SAF_Db *database, const char *name, SAF_RelRep *buf);

SAF_RelRep *saf_find_relrep_hslab(void);
SAF_RelRep *saf_find_relrep_tuples(void);
SAF_RelRep *saf_find_relrep_totality(void);
SAF_RelRep *saf_find_relrep_structured(void);
SAF_RelRep *saf_find_relrep_unstructured(void);
SAF_RelRep *saf_find_relrep_arbitrary(void);

#ifdef __cplusplus
}
#endif

#endif /* !SAF_RELREP_H */
