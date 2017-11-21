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
#ifndef _TESTUTIL_H
#define _TESTUTIL_H

#include <signal.h>

#ifdef WIN32
  #include <direct.h> /*for chdir() on WIN32*/
  #include <io.h> /*for access(), chmod() on WIN32*/
	/*The keywords R_OK, W_OK and F_OK are not defined for WIN32, but
	  the functionality of access is the same. On unix, this is in unistd.h.*/
	#define R_OK 4
	#define W_OK 2
	#define F_OK 0
#endif

/*
 * these macros are used as a parallel-independent and SAF-independent way of obtaining
 * rank of calling processor and size of MPI_COMM_WORLD
 */
#ifdef HAVE_PARALLEL
#   define RANK MPI_Comm_rank(MPI_COMM_WORLD, &rank)
#   define SIZE MPI_Comm_size(MPI_COMM_WORLD, &size)
#   define BARRIER MPI_Barrier(MPI_COMM_WORLD)
#else
#   define RANK
#   define SIZE
#   define BARRIER
#endif

#ifndef TEST_FILE_PATH
#define TEST_FILE_PATH "."
#endif

#ifndef WIN32
#define TESTING_COLUMN_WIDTH "93"
#else
#define TESTING_COLUMN_WIDTH "80"
#endif

/*
 * The following macros are written using a strange "do-while" construct
 * to achieve the desired behavior in the caller with semicolon termination.
 */
#define TESTING(WHAT)							\
	do								\
	{  int rank=0,size=1;						\
	   RANK;							\
	   SIZE;							\
	   if (rank==0) 						\
	   {								\
	      printf("Testing [on %03d procs] %-"TESTING_COLUMN_WIDTH"s",size,WHAT);	\
	      fflush(stdout);						\
           }								\
        }while(false)
#define PASSED				\
	do				\
	{  int rank=0;			\
	   RANK;			\
	   if (rank==0)			\
	      puts("        PASSED");	\
	}while(false)

#ifdef WIN32
#undef FAILED /*prevent lots of warnings because of FAILED in winerror.h*/
#endif

#define	FAILED				\
	do				\
	{  int rank=0;			\
	   RANK;			\
	   if (rank==0)			\
	      puts("---->  *FAILED*");	\
	}while(false)
#define SKIPPED				\
	do				\
	{  int rank=0;			\
	   RANK;			\
	   if (rank==0)			\
	      puts("        -SKIP-");	\
	}while(false)
#define AT_MASTER										\
	do											\
	{  int rank=0;										\
	   RANK;										\
           if (rank==0)										\
	      printf ("\n\nmaster at %s:%d in %s()...\n", __FILE__, __LINE__, __FUNCTION__);	\
	}while(false)
#define AT_ALL											\
	do											\
	{  int rank=0;										\
	   RANK;										\
	   printf ("\n\nproc %d at %s:%d in %s()...\n", rank, __FILE__, __LINE__, __FUNCTION__);\
	}while(false)


/*
 * macros for file system functions
 */
#define UNLINK(A)					\
		do					\
		{ int rank=0;				\
		  RANK;					\
		   if (rank==0)				\
                      unlink(A);			\
		   BARRIER;				\
                }while(false)
#define STAT(N,B)					\
		do					\
		{ int rank=0;				\
		   RANK;				\
		   if (rank==0)				\
                      stat(N, B);			\
		   BARRIER;				\
                }while(false)
#define CHMOD(PATH,MODE)				\
		do					\
		{ int rank=0;				\
		   RANK;				\
		   if (rank==0)				\
		      chmod(PATH,MODE);			\
		   BARRIER;				\
                }while(false)


/*
 * test file name macro
 */
#ifndef TEST_FILE_NAME
#define TEST_FILE_NAME "test_saf.saf"
#endif


/*
 * Saf Test Utility (STU) interface
 */
#define STU_END_OF_ARGS		"end_of_args"

extern int STU_ProcessCommandLine(int ignoreUnknownArgs, int argc, char **argv, ...);

#endif /* _TESTUTIL_H */
