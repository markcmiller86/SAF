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
#define H5_USE_16_API
#ifdef HAVE_SAF
#   include <saf.h>
#elif defined(HAVE_SSLIB)
#   include <sslib.h>
#else
#   include <hdf5.h>
#endif

#include <assert.h>
#include <fcntl.h>
#ifdef HAVE_GMGH_H
#   include <gmgh.h>
#endif
#ifdef HAVE_GPFS_FCNTL
#   include <gpfs_fcntl.h>
#endif
#include <hdf5.h>
#ifdef HAVE_PARALLEL
#   include <mpi.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/* These things are normally defined in sslib.h */
#ifndef HAVE_SSLIB
#   ifndef TRUE
#       define FALSE 0
#       define TRUE  1
#       define false FALSE
#       define true  TRUE
#   endif
#   define H5_VERS_NUMBER  (H5_VERS_MAJOR*1000000 + H5_VERS_MINOR*1000 + H5_VERS_RELEASE)
#   ifndef MIN
#       define MIN(X,Y) ((X)<(Y)?(X):(Y))
#       define MAX(X,Y) ((X)>(Y)?(X):(Y))
#   endif
#endif

#ifndef GPFS_FCNTL_CURRENT_VERSION
#  undef DAN_MCNABB_METHOD
#endif

#define FILENAME        "rb-%<LAYER>-%04<NTASKS>.data"                  /* this is the default name */
#define P0              if (0==self && !quiet)
#define ALIGN(X,A)      (A*((X+A-1)/A))

#ifdef HAVE_READ_REAL_TIME
typedef timebasestruct_t wallclock_t;
#else
typedef struct timeval wallclock_t;
#endif

extern const char *time_method;
double deltaTime(wallclock_t *startP, wallclock_t *stopP);
void get_current_time(wallclock_t *now);
