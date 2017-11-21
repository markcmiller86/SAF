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

#ifndef SAFINIT_H
/*DOCUMENTED*/
#define SAFINIT_H

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Initialization 
 * Purpose:     Initialize the library 
 * Concepts:    Initialization, Version number
 *
 * Description: The saf_init() function must be called by the client to initialize the client's interaction with the library
 *              and should be called before any other SAF-API function except functions that set the properties to be passed
 *              in the saf_init() call.
 *
 *              Calling saf_init() when the library is initialized has absolutely no effect, even when a new, different list
 *              of properties is specified.
 *
 *              The counterpart of saf_init() is saf_final(), which releases all resources held by the library. The saf_init()
 *              function must be called after saf_final() if the client desires to interact with the library again.
 *
 *              Since all SAF clients are required to call saf_init(), we've chosen to wrap that
 *              function in a macro which also makes a reference to a global variable whose name is derived from the SAF
 *              version number. This variable is declared in the SAF library so that if an application is compiled with SAF
 *              header files which have a different version than the SAF library a link-time error will result. A version
 *              mismatch will result in an error similar to `undefined reference to SAF_version_1_4_0' from the linker.
 *
 * Parallel:    In parallel, saf_init() is collective and must be called by all processes in the library's communicator, which
 *              is MPI_COMM_WORLD by default. All processes must initialize the library with the same property values,
 *              although each may pass its own PROPERTIES argument.
 *
 *              If a new communicator is specified in the PROPERTIES argument then it will become the communicator for any
 *              database which doesn't override this communicator. It is the maximal communicator in the sense that no
 *              database can be opened on a set of processors which is not a subset of those in the communicator declared in
 *		the properties passed here.
 *              
 * Return:      The constant SAF_SUCCESS is returned for success; errors are returned as other values or by exception,
 *              depending on the setting of the error handling property in PROPERTIES (the default is to return an error number).
 * 
 * Issue:       Verify that the set of processors which must participate in this call is either MPI_COMM_WORLD or the
 *              communicator passed in the PROPERTIES.
 *
 *              We might want to communicate to confirm that all procs pass the same properties.
 *
 *              See the private function, _saf_init(), for the /real/ implementation of this function
 *
 *              Calling saf_init() after saf_final() is currently not supported due to design issues in DSL.
 *
 * Also:        SAF_REGISTRIES SAF_REGISTRY_SAVE
 *
 * Modifications:
 *              Robb Matzke, 2000-01-24
 *              Modified for documentation.
 *
 *              Robb Matzke, 2000-08-16
 *              Wrapped by a macro that checks header/lib version consistency.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define saf_init(PROPERTIES) (SAF_VERSION_VAR++, SAF_PARALLEL_VAR++, _saf_init((PROPERTIES)))

/* global librar variables defined in init.c */
extern hbool_t _SAF_InitDone;

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
extern int _saf_init(SAF_LibProps *libprops); /*must be here because it's called by macro*/
extern void saf_final(void);

#ifdef __cplusplus
}
#endif

#endif /* !SAFINIT_H */
