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
#ifndef SAF_ROLE_H
/*DOCUMENTED*/
#define SAF_ROLE_H

SAF_BIND_SSLIB(SAF_Role, role);
#define SAF_MAGIC_SAF_Role SS_MAGIC(ss_role_t)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Associating a role with a collection category
 *
 * Description: The Role object is used in calls to saf_declare_category() to associate a /role/ with a collection category.
 *	 	We use the role of a collection category to hint at the purpose or intent of collections created of a given
 *		category. Some collections are used to represent processor pieces. Some are used to knit individual computational
 *		elements together into a mesh. Some are used to represent different materials, etc. The list of roles here is by
 *		no means complete.
 *
 * Issues:	It is unclear whether any routines in SAF will be or ought to be sensitive to the value of /role/ or whether SAF
 *		simply passes the role around without ever interpreting it. There are two clear cases in which SAF itself might
 *		need to interpret the role; topology and boundary information. It might also be useful if SAF could interpret
 *		the processor role as this could help to make SAF knowledgeable about what pieces of the mesh are on which
 *		processors.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef SAF_Role SAF_RoleConstants;
#define SAF_TOPOLOGY            saf_find_role_topology()     /* This role is associated with collection categories whose purpose is 
							 * to knit the fine grained /topology/ of the mesh together. */
#define SAF_PROCESSOR           saf_find_role_processor()	/* This role is associated with collection categories whose purpose is
							 * to represent different processor's pieces */
#define SAF_BLOCK               saf_find_role_block()	/* This role is associated with collection categories whose purpose is
							 * to represent different /blocks/ (regions of homogenous cell type) */
#define SAF_DOMAIN              saf_find_role_domain()	/* This role is associated with collection categories whose purpose is
							 * to represent different domains; fundamental quanta of a mesh that can
							 * be assigned to or, perhaps, migrate between, different processors. */
#define SAF_ASSEMBLY            saf_find_role_assembly()	/* This role is associated with collection categories whose purpose is
							 * to represent parts in an assembly of parts. */
#define SAF_MATERIAL            saf_find_role_material()	/* This role is associated with collection categories whose purpose is
							 * to represent materials. */
#define SAF_SPACE_SLICE         saf_find_role_space_slice()
#define SAF_PARAM_SLICE         saf_find_role_param_slice()
#define SAF_ANY_ROLE            NULL                    /* Wildcard role for searching */

#ifdef __cplusplus
extern "C" {
#endif

/* Prototypes for API functions */
SAF_Role *saf_declare_role(SAF_ParMode pmode, SAF_Db *db, const char *name, const char *url, SAF_Role *role);
int saf_describe_role(SAF_ParMode pmode, SAF_Role *role, char **name, char **url);
int saf_find_roles(SAF_ParMode pmode, SAF_Db *db, const char *name, char *url, int *num_roles, SAF_Role **roles);
SAF_Role *saf_find_one_role(SAF_Db *db, const char *name, SAF_Role *buf);


SAF_Role *saf_find_role_topology(void);
SAF_Role *saf_find_role_processor(void);
SAF_Role *saf_find_role_block(void);
SAF_Role *saf_find_role_domain(void);
SAF_Role *saf_find_role_assembly(void);
SAF_Role *saf_find_role_material(void);
SAF_Role *saf_find_role_space_slice(void);
SAF_Role *saf_find_role_param_slice(void);

#ifdef __cplusplus
}
#endif

#endif /* !SAF_ROLE_H */
