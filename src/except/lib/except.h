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
#ifndef except_h_included
#define except_h_included

/*-------------------------------------------------------------------------- - -
|
|       except.h
|
|       This file contains various macros and structure definitions used
|       to implement an exception handling mechanism for C/ANSI-C which was
|       patterned after the mechanism provided by C++
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
|
| */


#ifdef __cplusplus
extern "C" {
#endif

#include <setjmp.h>

#include <stdio.h>
#include <stdlib.h>

/*---------------------------------- - -
|
|       Identification of scope...
|
+-------------------------------------*/

#define except_seg_outside   1
#define except_seg_in_try    2
#define except_seg_in_catch  3

#define except_outside   (sizeof(except_seg)==except_seg_outside)
#define except_in_try    (sizeof(except_seg)==except_seg_in_try)
#define except_in_catch  (sizeof(except_seg)==except_seg_in_catch)

/*-------------------------------------------------------------------------- - -
|
|       Structures...
|
+-----------------------------------------------------------------------------*/

#define except_t struct _except_t

except_t
{  char     *name;
   char     *file;
   int       line;
   jmp_buf   jbuf;
   except_t *prev;
};

#define except_unwind            except_stack = except_stack->prev;

/*---------------------------------- - -
|
|       Throwing exceptions...
|
+-------------------------------------*/

#define except_throw(X)          {  except_t *except_p; \
				    char     *except_tmp; \
				    \
				    if ((except_tmp=X) == 0) \
				       except_tmp = "nil_exception.except"; \
				    if ((except_p=except_stack) != 0) \
				    {  except_stack   = except_p->prev; \
				       except_p->name = except_tmp; \
				       except_p->file = __FILE__; \
				       except_p->line = __LINE__; \
				       longjmp(except_p->jbuf,1); \
				    } \
				    except_uncaught(except_tmp,__FILE__,__LINE__); \
				 }

#define except_rethrow           if (except_in_catch) \
				 {  except_t *except_p; \
				    \
				    if ((except_p=except_stack) != 0) \
				    {  except_stack   = except_p->prev; \
				       except_p->name = except_pkt.name; \
				       except_p->file = except_pkt.file; \
				       except_p->line = except_pkt.line; \
				       longjmp(except_p->jbuf,1); \
				    } \
				    else \
				       except_uncaught(except_pkt.name,except_pkt.file,except_pkt.line); \
				 } \
				 else \
				    except_uncaught("inappropriate_rethrow.except",__FILE__,__LINE__);

/*---------------------------------- - -
|
|       Examining caught exceptions...
|
+-------------------------------------*/

#define except_name              (except_in_catch?except_pkt.name:"no_exception.except")
#define except_file              (except_in_catch?except_pkt.file:"no_exception.except")
#define except_line              (except_in_catch?except_pkt.line:0)

/*-------------------------------------------------------------------------- - -
|
|       Basic structure macros...
|
|           except_try
|           {
|
|              Suspect code.  Note that use of either
|              the goto or return statements is NOT
|              permitted in this section.  If such
|              flow-of-control is needed then the
|              macros except_goto, except_return and
|              except_return_value are provided.
|
|                   :
|
|           }
|           except_catch("range.gorfo")
|           {
|
|              Code to be envoked if/when an
|              exception that "matches" the
|              exception "range.gorfo"
|
|                   :
|
|           }
|           except_catch("gorfo")
|           {
|
|              Code to be envoked if/when an
|              exception that "matches" the
|              exception "gorfo" (more general
|              than "range.gorfo").
|
|                   :
|
|           }
|           except_else
|           {
|
|              Code to be envoked if/when any
|              exception is thrown (the most
|              general).
|
|                   :
|
|           }
|           except_end
|
|       Note that use of except_catch and except_else is optional and that
|       if except_else is used it must follow all uses of except_catch (if
|       any). If more than one except_catch is used then they should be
|       arranged from most specific to most general.  The except_else is
|       the most general of all in that it will catch any exception.
|
+-----------------------------------------------------------------------------*/

#define except_try       {  except_t except_pkt; \
			    \
			    except_pkt.name = 0; \
			    except_pkt.file = 0; \
			    except_pkt.line = 0; \
			    except_pkt.prev = except_stack; \
			    except_stack    = &except_pkt; \
			    if (setjmp(except_pkt.jbuf) == 0) \
			    {  char except_seg[except_seg_in_try]; \
			       char except_tmpDepth[sizeof(except_depth)]; \
			       char except_depth[sizeof(except_tmpDepth)+1]; \
			       \
			       {

				 /*---------------------------------- - -
				 |
				 |   User supplied code which may
				 |   throw an exception...
				 |
				 |   NOTE: Can't use "goto" or "return"
				 |         from this area.
				 |
				 +-------------------------------------*/

#define except_catch(X)        } \
			       if (except_in_try) \
				  except_stack = except_stack->prev; \
			    } \
			    else if (except_match(except_pkt.name,X)) \
			    {  char except_seg[except_seg_in_catch]; \
			       \
			       {

				/*---------------------------------- - -
				|
				|   Optional user supplied code to be
				|   performed when a specific exception
				|   is caught.
				|
				+-------------------------------------*/

#define except_else            } \
			       if (except_in_try) \
				  except_stack = except_stack->prev; \
			    } \
			    else if (1) \
			    {  char except_seg[except_seg_in_catch]; \
			       \
			       {

				 /*---------------------------------- - -
				 |
				 |   Optional user supplied code to be
				 |   performed when "any" exception is
				 |   caught.
				 |
				 +-------------------------------------*/

#define except_end             } \
			       if (except_in_try) \
				  except_stack = except_stack->prev; \
			    } \
			    else \
			    {  except_t *except_p; \
			       \
			       if ((except_p=except_stack) != 0) \
			       {  except_stack   = except_p->prev; \
				  except_p->name = except_pkt.name; \
				  except_p->file = except_pkt.file; \
				  except_p->line = except_pkt.line; \
				  longjmp(except_p->jbuf,1); \
			       } \
			       else \
				  except_uncaught(except_pkt.name,except_pkt.file,except_pkt.line); \
			    } \
			 }

#define except_any       ""

/*-------------------------------------------------------------------------- - -
|
|       Flow-of-control macros...
|
|       As previously reported, use of return statements is NOT permitted
|       within the except_try section and use of goto statements that
|       cause flow-of-control to leave a except_try section are NOT
|       permitted.  To support such action the except_goto, except_return
|       and except_return_value macros are provided.
|
|
|              except_goto ( N, L )
|
|          The first parameter N is an integer indicating the number of
|          levels of try nesting are to be exited.  The second parameter
|          L is the name of the label to go to.
|
|
|              except_return
|
|          Exit from the function containing this macro.  The exception
|          stack will unwind to reflect the try nesting level where
|          this macro was encountered.
|
|
|              except_return_value ( V )
|
|          Exit from the function containing this macro returning the
|          given value (V).  The exception stack will unwind to reflect
|          the try nesting level where this macro was encountered.
|
|
|       Example:
|
|           int g ( int X, int Y )
|           {  if (X <= 0)
|                 except_throw("grape.exception");
|              else if (Y <= 0)
|                 except_throw("banana.exception");
|              else if (X <= Y)
|                 return Y;
|              else
|                 return X;
|           }
|
|           int f ( int X, int Y )
|           {
|              except_try
|              {
|                 g(X,Y);
|
|                 except_try
|                 {  int Z;
|
|                    Z = g(Y,X);
|                    if      (X == Z)
|                       except_goto(1,A)  // A is 1 level up
|                    else if (Y == Z)
|                       except_goto(2,B)  // B is 2 levels up
|                    else
|                    {
|                       except_return_value(Z);
|                    }
|                 }
|                 except_catch("grape.exception")
|                 {
|
|                 }
|                 except_end
|           A:    printf("1st is max\n");
|              }
|              except_catch("banana.exception")
|              {
|
|              }
|              except_else
|              {
|
|              }
|              except_end
|           B: printf("2nd is max\n");
|           }
|
+-----------------------------------------------------------------------------*/

#define except_goto(N,L)       {  int except_count; \
				  \
				  except_count = (N); \
				  while (0 < except_count) \
				  {  except_stack  = except_pkt.prev; \
				     except_count -= 1; \
				  } \
				  goto L; \
			       }

#define except_return          {  int except_count; \
				  \
				  except_count = sizeof(except_depth) - 1; \
				  while (0 < except_count) \
				  {  except_stack  = except_pkt.prev; \
				     except_count -= 1; \
				  } \
				  return; \
			       }

#define except_return_value(V) {  int except_count; \
				  \
				  except_count = sizeof(except_depth) - 1; \
				  while (0 < except_count) \
				  {  except_stack  = except_pkt.prev; \
				     except_count -= 1; \
				  } \
				  return V; \
			       }

/*-------------------------------------------------------------------------- - -
|
|       Externals...
|
+-----------------------------------------------------------------------------*/

extern char      except_depth[1];		/* never actually used        */
extern char      except_seg[except_seg_outside];/* never actually used        */
extern except_t *except_stack;


extern int       except_match ( char *thrown, char *pattern );
extern void      except_uncaught ( char *X, char *file, int line );

#ifdef __cplusplus
	   }
#endif

/*============================================================================*/

#endif

