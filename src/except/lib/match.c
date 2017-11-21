/*
 * Copyright(C) 1999 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 1999 Sandia Corporation.
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
 * Authors:
 * 	William J. Arrighi	LLNL
 * 	Peter K. Espen		SNL
 * 	Ray T. Hitt 		SNL
 * 	Robb P. Matzke 		LLNL
 * 	Mark C. Miller 		LLNL
 * 	James F. Reus 		LLNL
 * 	Larry A. Schoof 	SNL
 * 
 * Acknowledgements:
 * 	Marty L. Barnaby	SNL - Red parallel perf. study/tuning
 * 	David M. Butler		LPS - Data model design/implementation Spec.
 * 	Albert K. Cheng		NCSA - Parallel HDF5 support
 * 	Nancy Collins		IBM - Alpha/Beta user
 * 	Linnea M. Cook		LLNL - Management advocate
 * 	Michael J. Folk		NCSA - Management advocate 
 * 	Richard M. Hedges	LLNL - Blue-Pacific parallel perf. study/tuning 
 * 	Quincey A. Koziol	NCSA - Serial HDF5 Support 
 * 	Celeste M. Matarazzo	LLNL - Management advocate
 * 	Tom H. Robey 		SNL - Early developer
 * 	Greg D. Sjaardema	SNL - Alpha/Beta user
 * 	Reinhard W. Stotzer	SNL - Early developer
 * 	Judy Sturtevant		SNL - Red parallel perf. study/tuning 
 * 	Robert K. Yates		LLNL - Blue-Pacific parallel perf. study/tuning
 * 
 */
/*-------------------------------------------------------------------------- - -
|
|       match.c
|
|       This file contains C/ANSI-C code used to implement an exception
|       handling mechanism.  The following functions are provided...
|
|               int except_match ( thrown,pattern )
|
|                   char *thrown;
|                   char *pattern;
|
|           This function is used to check if the thrown exception name
|           matches the pattern exception name.  A value of 1 (true) is
|           returned on match, 0 (false) if no match.
|
+--------------------------------------------------------------------------- - -
|
|       Author:
|
|           Dr. James F. Reus, Lawrence Livermore National Laboratory
|
|           This work is based on previous work outside the Laboratory
|           and the University of California by the author and has been
|           released to the Laboratory and the University of California
|           for free distribution only.  The author makes no
|           representations as to the suitability and operability of
|           this software for any purpose whatsoever.  It is provided
|           "as is" without express or implied warranty.  This software
|           is distributed in the hope that it will be useful, but
|           WITHOUT ANY WARRANTY; without even the implied warranty
|           of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
| */

#include <except.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int except_match ( char *thrown, char *pattern )
{  int   result;

   if (pattern != 0 && thrown != 0)
   {  int La,Lb;

      La = (int)strlen(thrown);
      Lb = (int)strlen(pattern);
      if (Lb <= La)
	 if (strcmp(&(thrown[La-Lb]),pattern) == 0)
	    result = 1;
	 else
	    result = 0;
      else
	 result = 0;
   }
   else
      result = 0;
   return result;
}

#ifdef __cplusplus
	   }
#endif

