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
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#   include "SAFconfig-WIN32.h"
#elif defined(JANUS)
#   include "SAFconfig-JANUS.h"
#else
#   include "SAFconfig.h"
#endif

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#ifdef HAVE_IO_H
#include <io.h> /*for isatty() on windows*/
#endif

#ifdef HAVE_PARALLEL
#include <mpi.h>
#endif
#include <testutil.h>

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example and Test Utilities	
 * Description:	This datatype is used to store information about command line arguments known to the program.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 19, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct _knownArgInfo {
   char *helpStr;		/* the help string for this command line argument */
   char *fmtStr;		/* format string for this command line argument */
   int argNameLength;		/* number of characters in this command line argument name */
   int paramCount;		/* number of parameters associated with this argument */
   char *paramTypes;		/* the string of parameter conversion specification characters */
   void **paramPtrs;		/* an array of pointers to caller-supplied scalar variables to be assigned */
   struct _knownArgInfo *next;	/* pointer to the next comand line argument */
} STU_KnownArgInfo_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example and Test Utilities	
 * Purpose:	Parse and assign command-line arguments	
 *
 * Description:	This routine is designed to do parsing of simple command-line arguments and assign values associated with
 *		them to caller-supplied scalar variables. It is used in the following manner.
 *
 *		   STU_ProccessCommandLine(argc, argv,
 *		      "-multifile",
 *		         "if specified, use a file-per-timestep",
 *		         &doMultifile,
 *		      "-numCycles %d",
 *		         "specify the number of cycles to run",
 *		         &numCycles,
 *		      "-dims %d %f %d %f",
 *		         "specify size (logical and geometric) of mesh",
 *		         &Ni, &Xi, &Nj, &Xj
 *		      STU_END_OF_ARGS);
 *
 *		After the argc,argv arguments, the remaining arguments come in groups of 3. The first of the three is a
 *		argument format specifier much like a printf statement. It indicates the type of each parameter to the
 *		argument and the number of parameters. Presently, it understands only %d, %f and %s types. the second
 *		of the three is a help line for the argument. Note, you can make this string as long as the C-compiler
 *		will permit. You *need*not* embed any '\n' charcters as the print routine will do that for you.
 *
 *		Command line arguments for which only existence of the argument is tested assume a caller-supplied return
 *		value of int and will be assigned `1' if the argument exists and `0' otherwise.
 *
 *		Do not name any argument with a substring `help' as that is reserved for obtaining help. Also, do not
 *		name any argument with the string `end_of_args' as that is used to indicate the end of the list of
 *		arguments passed to the function.
 *
 *		If any argument on the command line has the substring `help', help will be printed by processor 0 and then
 *		this function calls MPI_Finalize() (in parallel) and exit().
 *
 * Parallel:    This function must be called collectively in MPI_COMM_WORLD. Parallel and serial behavior is identical except in
 *		the
 *
 * Return:	On success, 0 is returned, -1 otherwise.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 19, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
STU_ProcessCommandLine(
   int ignoreUnknownArgs, /* flag to indicate what to do if encounter an unknown argument */
   int argc,		/* argc from main */
   char **argv,		/* argv from main */
   ...			/* a comma-separated list of 1) argument name and format specifier,  2) help-line for argument,
			   3) caller-supplied scalar variables to set */
)
{

   char *argvStr = NULL;
   int i, rank = 0;
   int helpWasRequested = 0;
   int invalidArgTypeFound = 0;
   int firstArg;
   int terminalWidth = 80 - 10;
   STU_KnownArgInfo_t *knownArgs;
   va_list ap;

#ifdef HAVE_PARALLEL
   {
      int result;
      if ((MPI_Initialized(&result) != MPI_SUCCESS) || !result)
	 return -1;
   }
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

   /* quick check for a help request */
   if (rank == 0)
   {
      for (i = 0; i < argc; i++)
      {
	 if (strstr(argv[i], "help") != NULL)
	 {
	    char *s;
	    helpWasRequested = 1;
            if ((s=getenv("COLUMNS")) && isdigit(*s))
	       terminalWidth = (int)strtol(s, NULL, 0) - 10;
	    break;
	 }
      }
   }

#ifdef HAVE_PARALLEL
   MPI_Bcast(&helpWasRequested, 1, MPI_INT, 0, MPI_COMM_WORLD);
   if (helpWasRequested)
      MPI_Bcast(&terminalWidth, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif

   /* everyone builds the command line parameter list */
   va_start(ap, argv);

   knownArgs = NULL;
   firstArg = 1;
   while (1)
   {
      int n, paramCount, argNameLength;
      char *fmtStr, *helpStr, *p, *paramTypes;
      void **paramPtrs;
      STU_KnownArgInfo_t *newArg, *oldArg;

      /* get this arg's format specifier string */
      fmtStr = va_arg(ap, char *);

      /* check to see if we're done */
      if (!strcmp(fmtStr, STU_END_OF_ARGS))
	 break;

      /* get this arg's help string */
      helpStr = va_arg(ap, char *);

      /* print this arg's help string from proc 0 if help was requested */
      if (helpWasRequested && rank == 0)
      {
	 static int first = 1;
	 char helpFmtStr[32];
	 FILE *outFILE = (isatty(2) ? stderr : stdout);

	 if (first)
	 {
	    first = 0;
	    fprintf(outFILE, "usage and help for %s\n", argv[0]);
	 }

	 /* this arguments format string */
	 fprintf(outFILE, "   %-s\n", fmtStr);

	 /* this arguments help-line format string */
	 sprintf(helpFmtStr, "      %%-%d.%ds", terminalWidth, terminalWidth);

	 /* this arguments help string */
	 p = helpStr;
	 n = (int)strlen(helpStr);
	 i = 0;
	 while (i < n)
	 {
	    fprintf(outFILE, helpFmtStr, p);
	    p += terminalWidth;
	    i += terminalWidth;
	    if ((i < n) && (*p != ' '))
	       fprintf(outFILE, "-\n");
	    else
	       fprintf(outFILE, "\n"); 
	 }
      }

      /* count number of parameters for this argument */
      paramCount = 0;
      n = (int)strlen(fmtStr);
      argNameLength = 0;
      for (i = 0; i < n; i++)
      {
	 if (fmtStr[i] == '%' && fmtStr[i+1] != '%')
	 {
	    paramCount++;
	    if (argNameLength == 0)
	       argNameLength = i-1;
	 }
      }

      if (paramCount)
      {
	 int k;

         /* allocate a string for the conversion characters */
         paramTypes = (char *) malloc((unsigned int)paramCount+1);

         /* allocate a string for the pointers to caller's arguments to set */ 
         paramPtrs = (void **) malloc(paramCount * sizeof(void*));

         /* ok, make a second pass through the string and setup argument pointers and conversion characters */
	 k = 0;
         for (i = 0; i < n; i++)
	 {
	    if (fmtStr[i] == '%' && fmtStr[i+1] != '%')
	    {
	       paramTypes[k] = fmtStr[i+1];
	       switch (paramTypes[k])
	       {
	          case 'd': paramPtrs[k] = va_arg(ap, int *); break;
	          case 's': paramPtrs[k] = va_arg(ap, char **); break;
	          case 'f': paramPtrs[k] = va_arg(ap, double *); break;
	          default:  invalidArgTypeFound = 1; break;
	       }
	       k++;
	    }
	 }
      }	 
      else
      {
	 /* in this case, we assume we just have a boolean (int) value */
	 argNameLength = n;
	 paramTypes = NULL;
	 paramPtrs = (void **) malloc (sizeof(void*));
	 paramPtrs[0] = va_arg(ap, int *);
      }

      /* ok, we're done with this parameter, so plug it into the paramInfo array */
      newArg = (STU_KnownArgInfo_t *) malloc(sizeof(STU_KnownArgInfo_t));
      newArg->helpStr = helpStr;
      newArg->fmtStr = fmtStr;
      newArg->argNameLength = argNameLength;
      newArg->paramCount = paramCount;
      newArg->paramTypes = paramTypes;
      newArg->paramPtrs = paramPtrs;
      newArg->next = NULL; 
      if (firstArg)
      {
	 firstArg = 0;
	 knownArgs = newArg;
	 oldArg = newArg;
      }
      else
         oldArg->next = newArg;
      oldArg = newArg;
   }
   va_end(ap);

#ifdef HAVE_PARALLEL
   MPI_Bcast(&invalidArgTypeFound, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
   if (invalidArgTypeFound)
      return -1;

   /* exit if help was requested */
   if (helpWasRequested)
   {
#ifdef HAVE_PARALLEL
      MPI_Finalize();
#endif
      exit(0);
   }

   /* ok, now broadcast the whole argc, argv data */ 
#ifdef HAVE_PARALLEL
   {
      int argvLen;
      char *p;

      /* processor zero computes size of linearized argv and builds it */
      if (rank == 0)
      {

	 if (getenv("SAF_IGNORE_UNKNOWN_ARGS"))
	    ignoreUnknownArgs = 1;

         /* compute size of argv */
         argvLen = 0;
         for (i = 0; i < argc; i++)
	    argvLen += (strlen(argv[i])) + 1;

         /* allocate an array of chars to broadcast */
         argvStr = (char *) malloc((unsigned int)argvLen);

         /* now, fill argvStr */
         p = argvStr;
         for (i = 0; i < argc; i++)
         {
	    strcpy(p, argv[i]);
	    p += (strlen(argv[i]) + 1);
	    *(p-1) = '\0';
         }
      }

      /* now broadcast the linearized argv */
      MPI_Bcast(&ignoreUnknownArgs, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(&argc, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(&argvLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
      if (rank != 0)
         argvStr = (char *) malloc((unsigned int)argvLen);
      MPI_Bcast(argvStr, argvLen, MPI_CHAR, 0, MPI_COMM_WORLD);

      /* Issue: We're relying upon embedded nulls in argvStr */
      /* now, put it back into the argv form */
      if (rank != 0)
      {
	 argv = (char **) malloc(argc * sizeof(char*));
	 p = argvStr;
	 for (i = 0; i < argc; i++)
	 {
	    argv[i] = p;
	    p += (strlen(p) + 1);
	 }
      }
   }
#endif

   /* And now, finally, we can process the arguments and assign them */
   i = 1;
   while (i < argc)
   {
      int foundArg;
      STU_KnownArgInfo_t *p;

      /* search known arguments for this command line argument */
      p = knownArgs;
      foundArg = 0;
      while (p && !foundArg)
      {
         if (!strncmp(argv[i], p->fmtStr, (unsigned int)(p->argNameLength) ))
	    foundArg = 1;
	 else
	    p = p->next;
      }
      if (foundArg)
      {
	 int j;

	 /* assign command-line arguments to caller supplied parameters */
	 if (p->paramCount)
	 {
	    for (j = 0; j < p->paramCount; j++)
	    {
	       switch (p->paramTypes[j])
	       {
	          case 'd':
	          {
		     int *pInt = (int *) (p->paramPtrs[j]);
		     *pInt = atoi(argv[++i]);
		     break;
	          }
	          case 's':
	          {
		     char **pChar = (char **) (p->paramPtrs[j]);
		     if (*pChar == NULL)
		     {
		        *pChar = (char*) malloc(strlen(argv[++i]+1));
		        strcpy(*pChar, argv[i]);
		     }
		     else
		        strcpy(*pChar, argv[++i]);
		     break;
	          }
	          case 'f':
	          {
		     double *pDouble = (double *) (p->paramPtrs[j]);
		     *pDouble = atof(argv[++i]);
		     break;
	          }
               }
   	    }
	 }
	 else
	 {
            int *pInt = (int *) (p->paramPtrs[0]);
            *pInt = 1; 
	 }
      }
      else
      {
	 if (!ignoreUnknownArgs)
	 {
	    char *p = strrchr(argv[0], '/');
	    FILE *outFILE = (isatty(2) ? stderr : stdout);
	    p = p ? p+1 : argv[0];
	    if (rank == 0)
	       fprintf(outFILE , "%s: unknown argument %s. Type %s -help for help\n", p, argv[i], p);
#ifdef HAVE_PARALLEL
            MPI_Finalize();
#endif
            exit(-1);
	 }
      }

      /* move to next argument */
      i++;
   }

   /* free argvStr */
   if (argvStr)
      free(argvStr);

   /* free the argv pointers we created, locally */
   if (rank != 0 && argv)
      free(argv);

   /* free the known args stuff */
   while (knownArgs)
   {
      STU_KnownArgInfo_t *next = knownArgs->next;

      if (knownArgs->paramTypes)
         free(knownArgs->paramTypes);
      if (knownArgs->paramPtrs)
         free(knownArgs->paramPtrs);
      free(knownArgs);
      knownArgs = next;
   }

   return 0;
}
