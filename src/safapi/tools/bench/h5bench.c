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

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:	HDF5 Benchmark for SAF
 * Description:	This is a simple HDF5 level benchmark that currently tests only write performance of HDF5. It is designed to
 *		drive HDF5 in much the same way that SAF does for all of the saf_write_xxx calls. However, it is incomplete
 *		in that it does not mimic output of SAF's attributes or VBT's tables. That can be added later.
 *
 *		Alternatively, this benchmark can be used to write an ascii file via the POSIX open/write interface. Note
 *		this is *not* the fopen/fwrite or fprintf interface. It is the binary POSIX interface. Nonetheless, the
 *		data buffer passed in the write calls is filled with ascii data so that we can confirm the writes occured
 *		correctly. The ascii data buffer is constructed in the same way a binar data buffer is constructed so that
 *		numbers A.B are <rank>.<local-index>. This mode is provided as a test of the /guarenteed/not/to/exceed/
 *		capability of the platform. The resultant output is nearly braindead in that none of the operations
 *		usually avialable in more abstract I/O libraries like HDF5 are possible.
 *
 *		Basically, this benchmark loops over several /cycles/ and in each /cycle/ writes several datasets. The datasets
 *		are 1 dimensional and each processor's piece is a non-overlapping, rank-ordered hyperslab of the whole.
 *
 *		With an aggregation ratio set, (-aggRatio) the bencmark will aggregate write requets from N processors to
 *		1. N is the parameter specified for the -aggRatio command-line argument. N must evenly divide the total
 *		number of processors. If it doesn't, it will be adjusted upwards to do so. Aggregation is done by sending
 *		data via MPI Point to Point communication. Then, writes are issued only from every Nth processor. However,
 *		we don't go the extra step and only open the file on those N processors. So, currently, we only support
 *		independent writes. Hmmm, there are a couple of optimizations we could do here...
 *
 *		   1. open the file on only N processors and support collective or indpendent I/O on those.
 *		   2. open the file on all processors and do collective I/O but with all but every Nth processor
 *		      contributing 0 to the write.
 *
 *		A number of parameters can be set to control I/O. Use the '-help' command-line option to see them.
 *
 *		The benchmark is designed to be compiled within SAF's source code tree or as a stand-alone code. However,
 *		you will need a couple of include files from SAF's source code tree to make it work. Ordinarily, from
 *		within a SAF /build/ tree, the command 'make h5bench.tar.gz' in the saf/src/safapi/tools/bench directory
 *		will generate a tarball of the stand-alone code. You might also need to set some flags on the compile line
 *		not compile in functions like gethostname(), etc. Those flags should all be visible from the Makefile that
 *		is produced in the tarball.
 *
 *		A simple approach to collecting timing data is used here. Each HDF5 API function is redefined by creating
 *		a macro with the same name which calls an equivalent H5B_xxx version of the function. The H5B_xxx implementations
 *		are all at the bottom of the source file where the macros redefining the HDF5 interface are undefined. Each
 *		redefined function does some work to collect timing data before calling the real HDF5 function.
 *
 *		For each HDF5 API call, h5bench computes the time spent in that call on each processor by making calls to
 *		MPI_Wtime just before and just after the call. These times are accumulated in an array and written to files,
 *		one for each HDF5 API call (an processor if you specify per-processor stats), after the entire test is completed.
 *		In addition, we reduce these arrays across processors using MPI_MAX reduction operator and then compute the
 *		min, max, average and total time of this set of maximum times. Consequently, the min, max, average and total
 *		time reported for each call in the summary message at the end is probably a little mis-leading as these are
 *		the stats for the maximum over all processors on each call. For example, the sum of all the maximum times can,
 *		indeed, be larger than the total time reported. The total time reported is the latest stop time minus the
 *		earliest start time across all processors.
 *
 *		A sample output from running the benchmark looks like this...
 *
 *		   mpirun -np 6 h5bench
 *  		   starting on 6 processors (1 nodes with 6 tasks per node)
 *		   will write 228.9 Megabytes over 5 cycles...
 *		      finished cycle 1 of 5
 *		      finished cycle 2 of 5
 *		      finished cycle 3 of 5
 *		      finished cycle 4 of 5
 *		      finished cycle 5 of 5
 *		   *********************************************************************
 *		   ************************** Aggregate Results ************************
 *		   *********************************************************************
 *		   total bytes written                           = 228.881836 Megabytes
 *		   total time                                    = 2.687500 seconds
 *		   aggregate throughput (totalBytes / totalTime) = 85.165334 Megabytes/second
 *		   summary results appended to /home/mcmille/h5bench_cummstats.txt
 *		   Writing collected performance data to files...
 *		   H5Dwrite:       avg=   0.0078,max=   0.0117,dev=   0.0018,total=   3.9062,called=500
 *		   H5Dopen:        avg=   0.0000,max=   0.0000,dev=   0.0000,total=   0.0000,called=0
 *		   H5Dclose:       avg=   0.0001,max=   0.0039,dev=   0.0006,total=   0.0547,called=500
 *		   H5Dcreate:      avg=   0.0008,max=   0.0039,dev=   0.0015,total=   0.3789,called=500
 *		   H5Fopen:        avg=   0.0000,max=   0.0000,dev=   0.0000,total=   0.0000,called=0
 *		   H5Fcreate:      avg=   0.0273,max=   0.0273,dev=   0.0000,total=   0.0273,called=1
 *		   H5Fclose:       avg=   0.0000,max=   0.0000,dev=   0.0000,total=   0.0000,called=0
 *		   H5Gcreate:      avg=   0.0008,max=   0.0039,dev=   0.0016,total=   0.0039,called=5
 *		   H5Gclose:       avg=   0.0000,max=   0.0000,dev=   0.0000,total=   0.0000,called=5
 *
 *		   Did not encounter any outliars
 *
 *
 *		The aggregate I/O rate is computed as total bytes written divided by total time where total time is defined
 *		to be the maximum stop time over all processors minus the minimum start time over all processors.
 *
 *		The detailed data for each call output at the end of the test after the aggrgate results may *total* more
 *		than the total time. For example, in the sample output above, the total time in H5Dwrite is 3.9062 secnds
 *		while the total aggregate time is 2.6 seconds. This is due to the fact that the total time reported for
 *		H5Dwrite is a sum of the maximum time over all processor for each of the 500 calls.
 *
 *		Occasionally, we encounter situations where a call that ordinarily takes 0.5 seconds winds up taking
 *		10's to 100's of seconds. We call these situations /outliars/. There is code here to detect outliars. It
 *		basically computes the average and standard deviation for the time in a call and then finds all calls
 *		where the max (over all processors) is more than one standard deviation above the average. Its an imperfect
 *		algorithm and will sometimes generate misleading results. However, all such results are printed as
 *		/adjusted/ results in addition to the raw aggregate results. So, its easy to spot when the outliar info
 *		is misleading.
 *
 *		Each time this benchmark is run, it will attempt to output summary results to a file in user's home dir or
 *		or in dot (.) called h5bench_cummstats.txt. Note that it is best not to have multiple invokations of this
 *		benchmark running simultaneously as both may wind up writing to this file and stomping on each other. Of
 *		course, there are more fundamental reasons for not having more than one invokation running at any one time. 
 *
 *		In the function documentation here, any output argument taged with /[OUT-X]/ is only relevant on processor
 *		X in the communicator in which it was called.
 *		
 * Modifications:
 *              Mark Miller, LLNL, 15May2002 
 *		Initial implementation
 *---------------------------------------------------------------------------------------------------------------------------------
 */

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#include <hdf5.h>

#ifdef H5B_STANDALONE
#include <mpi.h>
#include <testutil.h>
#include <testutil.c> /* hack to deal with fact that code actually depends on some SAF test utilities */
#else
#include <../../test/testutil.h>
#endif

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose:	Call tags for redefined HDF5 API	
 *
 * Description:	Tags for manner in which re-defined HDF5 interface funcs are called
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define H5B_RUNNING	5	/* benchmark is running */
#define H5B_TERMINATING	6	/* benchmark is terminating */

#define H5B_DATA_TAG	1	/* message tag for a data send */

/* constant to throttle number of processors writing call history files at one time */
#define H5B_PROC_GROUP	4

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose:	Redefine relevant HDF5 functions 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define H5Fcreate(N,F,CP,AP)	H5B_H5Fcreate(N,F,CP,AP,H5B_RUNNING,NULL)
#define H5Fopen(N,F,P)		H5B_H5Fopen(N,F,P,H5B_RUNNING,NULL)
#define H5Fclose(F)		H5B_H5Fclose(F,H5B_RUNNING,NULL)
#define H5Dcreate(F,N,T,S,P)	H5B_H5Dcreate(F,N,T,S,P,H5B_RUNNING,NULL)	
#define H5Dopen(F,N)		H5B_H5Dopen(F,N,H5B_RUNNING,NULL)
#define H5Dclose(D)		H5B_H5Dclose(D,H5B_RUNNING,NULL)
#define H5Dwrite(D,T,M,F,X,B)	H5B_H5Dwrite(D,T,M,F,X,B,H5B_RUNNING,NULL)
#define H5Gcreate(F,N,SH)       H5B_H5Gcreate(F,N,SH,H5B_RUNNING,NULL)
#define H5Gclose(G)             H5B_H5Gclose(G,H5B_RUNNING,NULL)

/* re-define malloc if you want to track mallocs */ 
/* #define malloc(N)		H5B_malloc(N,__LINE__)*/

typedef int (*compareFunc)(const void*,const void*);

/* our implementation of the re-defined HDF5 interface */
static herr_t H5B_H5Dwrite(hid_t dataset_id, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id,
	            hid_t xfer_plist_id, const void *buf, int status, double *avgCallTime);
static hid_t H5B_H5Dopen(hid_t loc_id, const char *name, int status, double *avgCallTime);
static hid_t H5B_H5Dcreate(hid_t loc_id, const char *name, hid_t type_id, hid_t space_id, hid_t create_plist_id, int status,
			   double *avgCallTime);
static herr_t H5B_H5Dclose(hid_t dataset_id, int status, double *avgCallTime);
static hid_t H5B_H5Fopen(const char *name, unsigned flags, hid_t access_id, int status, double *avgCallTime);
static hid_t H5B_H5Fcreate(const char *name, unsigned flags, hid_t create_id, hid_t access_id, int status, double *avgCallTime);
static herr_t H5B_H5Fclose(hid_t file_id, int status, double *avgCallTime);
static hid_t H5B_H5Gcreate(hid_t loc_id, const char *name, size_t size_hint,int status, double *avgCallTime);
static herr_t H5B_H5Gclose(hid_t group_id,int status, double *avgCallTime);

#ifdef malloc
static void *H5B_malloc(int n, int line);
#endif

/* global environment (see C language specification) */
extern char **environ;

/* file scope variables */
static char cummStatFileName[128];
static int numCycles = 5;
static int numDatasets = 50;
static int myProcNum = 0;
static int numProcs = 1;
static int numNodes;
static int tasksPerNode;
static int perProcStats = 0;
static int asciiPosix = 0;
static double totalTime = 0.0;
static double adjustedTotalTime = 0.0;
static double outLiarTotalTime = 0.0;
static char h5VersionStr[32];
static char hostName[128];
static int perProcIOSize = 20000;
static int overWrite = 0;
static int closePerCycle = 0;
static int useIndependent = 0;
static int alignSize = 0;
static int numHints = 0;
static int aggRatio = 0;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose:	Compare processor names	
 *
 * Description: This is just a wrapper for a const qualified call to strcmp.	
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
H5B_ProcessorNameCompare(const void* nameA, const void* nameB)
{
   char *name1 = strdup((char*)nameA);
   char *name2 = strdup((char*)nameB);
   int result = strcmp(name1, name2);
   free(name1);
   free(name2);
   return result;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose:	Compute Node Configuration	
 *
 * Description:	This function is called by all processors, collectively to determine how many nodes there are and how many 
 *		processors there are on each node.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
H5B_ComputeNodeConfiguration(
   char *hostBaseName,	/* [OUT-0] returned base name of host machine */
   int *numNodes,	/* [OUT-0] number of nodes (as opposed to processors) */
   int *tasksPerNode,	/* [OUT-0] maximum number of tasks per node */
   int *unBalanced	/* [OUT-0] boolean flag indicating if number of tasks per node is fixed across all nodes */
)
{

   /* compute number of nodes (as opposed to processors) */
   {
      char hname[MPI_MAX_PROCESSOR_NAME];
      char *hostNames;
      int i,j;

      /* get hostname */
      MPI_Get_processor_name(hname, &i);

      /* get first part of hostname up to some numeric chars in the string */
      for (i = 0; i < strlen(hname); i++)
         if ('0' <= hname[i] && hname[i] <= '9')
	    break;
      for (j = 0; j < i; j++)
	 hostBaseName[j] = hname[j];
      hostBaseName[j] = '\0';

      /* gather node names to processor 0 */
      if (myProcNum == 0)
	 hostNames = (char *) malloc(numProcs * MPI_MAX_PROCESSOR_NAME * sizeof(char));
      MPI_Gather(hname,MPI_MAX_PROCESSOR_NAME,MPI_CHAR,hostNames,MPI_MAX_PROCESSOR_NAME,MPI_CHAR,0,MPI_COMM_WORLD);

      /* processor 0 does the work to compute node configuration from unique names of processors */
      if (myProcNum == 0)
      {
	 int unbalanced = 0;
	 int maxProcsPerNode = 0;
	 int procNum = 0;
	 int nodeNum = 0;

	 /* sort the names of the processors */
	 qsort(hostNames, numProcs, MPI_MAX_PROCESSOR_NAME, (compareFunc) strcmp);

	 while (procNum < numProcs)
	 {
	    int procsPerNode = 0;
	    char *name = &hostNames[procNum*MPI_MAX_PROCESSOR_NAME];
	    while ((procNum < numProcs) && !strcmp(name,&hostNames[procNum*MPI_MAX_PROCESSOR_NAME]))
	    {
	       procNum++;
	       procsPerNode++;
	    }
	    if (procsPerNode > maxProcsPerNode)
	    {
	       if (maxProcsPerNode)
		  unbalanced = 1;
	       maxProcsPerNode = procsPerNode;
	    }
	    nodeNum++;
         }
	 *numNodes = nodeNum;
	 *tasksPerNode = maxProcsPerNode; 
	 *unBalanced = unbalanced;
	 free(hostNames);
      }
   }
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose:	Process command line MPIO hints	
 *
 * Description:	This function processes all command line hints. This can't be handled by STU_ProcessCommandLine because it
 *		is possible to permit more than one mpio hint with the same command-line switch.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
H5B_ProcessCommandLineHints(int argc, char **argv, int *numHints, char ***hintKeys, char ***hintVals)
{
   int i, num_hints;
   char **hint_keys = NULL, **hint_vals = NULL;

   /* processor 0 first determines how many, if any, mpio hints there are on the command line */
   if (myProcNum == 0)
   {
      num_hints = 0;
      i = 0;
      while (i < argc)
      {
         if (!strcmp(argv[i], "-mpioHint"))
         {
	    num_hints++;
	    i += 3;
         }
         else
	    i++;
      }

      /* if we have hints, collect hint values and keys into arrays */
      if (num_hints)
      {  int hint_num = 0;

         /* allocate space for the hint keys and values */
         hint_keys = (char **) malloc(num_hints * sizeof(char*));
         hint_vals = (char **) malloc(num_hints * sizeof(char*));

         i = 0;
         while (i < argc)
         {
            if (!strcmp(argv[i], "-mpioHint"))
            {
	       hint_keys[hint_num] = strdup(argv[i+1]);
	       hint_vals[hint_num] = strdup(argv[i+2]);
	       hint_num++;
	       i += 3;
            }
            else
	       i++;
         }
      }
   }

   /* broadcast the results (stupid algorithm but it works and was easy to code) */
   MPI_Bcast(&num_hints, 1, MPI_INT, 0, MPI_COMM_WORLD);
   if (num_hints)
   {
      if (myProcNum != 0)
      {
         hint_keys = (char **) malloc(num_hints * sizeof(char*));
         hint_vals = (char **) malloc(num_hints * sizeof(char*));
      }

      for (i = 0; i < num_hints; i++)
      {  int keyLen, valLen;

	 if (myProcNum == 0)
	 {
	    keyLen = (int) strlen(hint_keys[i])+1;
	    valLen = (int) strlen(hint_vals[i])+1;
	 }
         MPI_Bcast(&keyLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
         MPI_Bcast(&valLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
	 if (myProcNum != 0)
	 {
	    hint_keys[i] = (char *) malloc(keyLen * sizeof(char));
	    hint_vals[i] = (char *) malloc(valLen * sizeof(char));
	 }
         MPI_Bcast(hint_keys[i], keyLen, MPI_CHAR, 0, MPI_COMM_WORLD);
         MPI_Bcast(hint_vals[i], valLen, MPI_CHAR, 0, MPI_COMM_WORLD);
      }
   }

   /* set the return values */
   *numHints = num_hints;
   *hintKeys = hint_keys;
   *hintVals = hint_vals;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose:	Build data buffer used in I/O calls	
 *
 * Description:	This function builds a data buffer used in I/O calls. The data buffer is constructed so that the floating
 *		point numbers in the buffer A.B are of the form <rank>.<local-index>. This makes it possible to inspect the
 *		file after the run and confirm that data has been written correctly.
 *
 *		In binary, HDF5 mode which is the default, the buffer contains floating point numbers. In asciiPosix mode
 *		the buffer contains 16 character strings, each terminated at the 16th character with a '\n' char.
 *
 *		In either HDF5 or asciiPosix mode, the buffer size is always perProcIOSize * sizeof(float) bytes. The 
 *		number of numbers represented in the buffer is different by a factor of 4 (16 bytes per number in asciiPosix
 *		mode and sizoef(float) bytes in HDF5 mode).
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
H5B_AllocAndFillDataBuffer(
   int perProcIOSize,		/* [IN] buffer size at each processor in terms of numbers of floats */
   int aggRatio,		/* [IN] aggregation ratio */
   float **dataBuf,		/* [OUT] allocated and filled data buffer */
   float **aggBuf		/* [OUT] allocated aggregation buffer */
)
{
   int i, expo10 = 1;

   /* compute order of magnitude of perProcIOSize */
   while (expo10 < perProcIOSize)
      expo10 *= 10;

   /* allocate and file data buffer */
   if (asciiPosix)
   {
      char *p;
      int charsPerEntry = 16;

      /* adjust perProcIOSize to an even multiple of charsPerEntry */
      while ((perProcIOSize * sizeof(float)) % charsPerEntry)
	 perProcIOSize++;

      *dataBuf = (float *) malloc(perProcIOSize * sizeof(float));
      p = (char *) *dataBuf;
      for (i = 0; i < perProcIOSize * sizeof(float) / charsPerEntry; i++)
      {
         sprintf(p,"%10.7f     ", myProcNum + (double) i / expo10);
	 p += charsPerEntry;
	 *(p-1) = '\n';
      }
   }
   else
   {
      float *p;
      *dataBuf = (float *) malloc(perProcIOSize * sizeof(float));
      p = *dataBuf;
      for (i = 0; i < perProcIOSize; i++)
         p[i] = (float) myProcNum + (float) i / expo10;
   }

   if (aggRatio)
      *aggBuf = (float *) malloc(perProcIOSize * aggRatio * sizeof(float));
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose: 	Validate an info object against an MPIO file 
 *
 * Description:	This function compares the info object passed in with one it obtains from an MPI_File_get_info call to
 *		confirm the given info object doesn't contain any key/values that MPIO doesn't understand. To do its work,
 *		it must have an MPIO file handle and so calls MPI_File_open on a temporary file
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
H5B_ValidateHints(
   MPI_Info someInfo	/* [IN] an info object populated with keys and values to validate */
)
{
   int i, nKeys=0, badHint=0;
   MPI_File theFile;
   MPI_Info fileInfo;

   if (MPI_File_open(MPI_COMM_WORLD,"h5bench_validateHints",MPI_MODE_CREATE|MPI_MODE_DELETE_ON_CLOSE|MPI_MODE_WRONLY,
          someInfo,&theFile) != 0)
   {
      printf("ERROR: MPI_File_open failed. Unable to validate hints. Continuing...\n");
      return;
   }
   MPI_File_get_info(theFile, &fileInfo);

   /* walk through someInfo and confirm each key/value there is in fileInfo */
   MPI_Info_get_nkeys(someInfo, &nKeys);
   for (i = 0; i < nKeys; i++)
   {
      int someValueExists, fileValueExists;
      char keyName[1024], someValue[1024], fileValue[1024];

      MPI_Info_get_nthkey(someInfo, i, keyName);
      MPI_Info_get(someInfo, keyName, 1024, someValue, &someValueExists);
      MPI_Info_get(fileInfo, keyName, 1024, fileValue, &fileValueExists);
      if (!fileValueExists)
      {
	 printf("requested hint key \"%s\" not returned from MPI_File_get_info() on processor %d\n", keyName, myProcNum);
	 badHint = 1;
      }
      else if (strcmp(someValue,fileValue))
      {
	 printf("requested value, \"%s\" for hint key \"%s\" not returned from MPI_File_get_info() on processor %d\n",
	    someValue, keyName, myProcNum);
	 badHint = 1;
      }
   }

   MPI_File_close(&theFile);
   MPI_Info_free(&fileInfo);
   if (badHint)
      MPI_Abort(MPI_COMM_WORLD,1);

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose: 	Return an HDF5 file handle	
 *
 * Description:	This function either opens or creates an HDF5 file depending on a number of parameters effecting the 
 *		operational mode of the benchmark.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
H5B_CreateOrOpenFile(
   int closePerCycle,		/* [IN] flag indicating if file should be closed each cycle (only relevant for non-multifile
				 * case */
   int NOmultifile, 		/* [IN] flag indicating if multifile mode should be used or not */
   int ioCycle,			/* [IN] current cycle */
   const char *baseName,	/* [IN] base name of file */
   hid_t fileAccessProps,	/* [IN] file access property list */
   hid_t *hFile,		/* [OUT] an hdf5 file opened or created */
   int *aFile			/* [OUT] an ascii file if in asciiPosix mode */
)
{
   char tmpStr[128];

      /* create or open file */
      if (closePerCycle)
      {
         if (!NOmultifile)
	 {
	    if (asciiPosix)
	    {
	       sprintf(tmpStr,"%s_%05d.txt", baseName, ioCycle);
	       if (!myProcNum)
	          *aFile = open64(tmpStr, O_CREAT|O_TRUNC|O_WRONLY, 00600);
	       MPI_Barrier(MPI_COMM_WORLD);
	       if (myProcNum)
	          *aFile = open64(tmpStr, O_WRONLY, 00600);
	       MPI_Barrier(MPI_COMM_WORLD);
	       lseek64(*aFile, (off64_t) 0, SEEK_SET);

	    }
	    else
	    {
	       sprintf(tmpStr,"%s_%05d.hdf5", baseName, ioCycle);
	       *hFile = H5Fcreate(tmpStr, H5F_ACC_TRUNC, H5P_DEFAULT, fileAccessProps);
	    }
	 }
         else
         {
	    if (ioCycle == 0)
	    {
	       if (asciiPosix)
	       {
	          sprintf(tmpStr,"%s.txt", baseName);
		  if (!myProcNum)
	             *aFile = open64(tmpStr, O_CREAT|O_TRUNC|O_WRONLY, 00600);
	          MPI_Barrier(MPI_COMM_WORLD);
	          if (myProcNum)
	             *aFile = open64(tmpStr, O_WRONLY, 00600);
	          MPI_Barrier(MPI_COMM_WORLD);
	          lseek64(*aFile, (off64_t) 0, SEEK_SET);
	       }
	       else
	       {
	          sprintf(tmpStr,"%s.hdf5", baseName);
	          *hFile = H5Fcreate(tmpStr, H5F_ACC_TRUNC, H5P_DEFAULT, fileAccessProps);
	       }
	    }
	    else
	    {
	       if (asciiPosix)
	       {
	          sprintf(tmpStr,"%s.txt", baseName);
	          *aFile = open64(tmpStr, O_WRONLY, 00600);
	          MPI_Barrier(MPI_COMM_WORLD);
	          lseek64(*aFile, (off64_t) 0, SEEK_SET);
	       }
	       else
	       {
	          sprintf(tmpStr,"%s.hdf5", baseName);
	          *hFile = H5Fopen(tmpStr, H5F_ACC_RDWR, fileAccessProps);
	       }
	    }
         }
      }
      else
      {
	 if (ioCycle == 0)
	 {
	    if (asciiPosix)
	    {
	       sprintf(tmpStr,"%s.txt", baseName);
	       if (!myProcNum)
	          *aFile = open64(tmpStr, O_CREAT|O_TRUNC|O_WRONLY, 00600);
	       MPI_Barrier(MPI_COMM_WORLD);
	       if (myProcNum)
	          *aFile = open64(tmpStr, O_WRONLY, 00600);
	       MPI_Barrier(MPI_COMM_WORLD);
	       lseek64(*aFile, (off64_t) 0, SEEK_SET);
	    }
	    else
	    {
	       sprintf(tmpStr,"%s.hdf5", baseName);
	       *hFile = H5Fcreate(tmpStr, H5F_ACC_TRUNC, H5P_DEFAULT, fileAccessProps);
	    }
	 }
      }
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose: 	Return an HDF5 dataseet handle	
 *
 * Description:	This function either opens or creates an HDF5 dataset depending on a number of parameters effecting the 
 *		operational mode of the benchmark. If all data is going to a single file, this function will also creates
 *		a new group for each cycle.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
H5B_CreateOrOpenDataset(
   hid_t theFile,		/* [IN] the HDF5 file handle */
   int overWrite,		/* [IN] boolean flag indicating if datasets are being overwritten */
   int cycleNumber,		/* [IN] current cycle number */
   int datasetNumber,		/* [IN] current dataset number */
   hid_t wholeDataspace,	/* [IN] the dataspace for this dataset */
   hid_t datasetCreationProps,	/* [IN] the dataset creation properties (if its being created) */
   hid_t *theDataset,		/* [OUT] returned dataset handle */
   hid_t *theGroup		/* [OUT] in case we're not overwritting the datasets, return the group we've created */
)
{
   char tmpStr[128];

         /* create dataset */
	 if (overWrite)
	 {
	    sprintf(tmpStr,"dataset_%05d", datasetNumber);
	    if (cycleNumber == 0)
	       *theDataset = H5Dcreate(theFile, tmpStr, H5T_NATIVE_FLOAT, wholeDataspace, datasetCreationProps);
	    else
	       *theDataset = H5Dopen(theFile, tmpStr);
	 }
	 else
	 {
	    if (datasetNumber == 0)
	    {
	       sprintf(tmpStr,"group_%05d", cycleNumber);
	       *theGroup = H5Gcreate(theFile, tmpStr, (size_t) numDatasets * 20); 
	    }
	    sprintf(tmpStr,"dataset_%05d", datasetNumber);
	    *theDataset = H5Dcreate(*theGroup, tmpStr, H5T_NATIVE_FLOAT, wholeDataspace, datasetCreationProps);
         }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose: 	Perform an N:1 data aggregation
 *
 * Description: This function simply aggregates data from every group of N processors onto every Nth processor where N==aggRatio.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
H5B_AggregateData(
   int myProcNum,	/* [IN] calling processor's rank */
   int aggRatio,	/* [IN] aggregation ratio */
   float *dataBuf,	/* [IN] calling processor's actuall data buffer */
   float *aggBuf,	/* [IN/OUT] aggregation buffer */
   double *aggTime	/* [OUT] accumulated aggregation timing. Time spent in recieves (and the memcpy) */
)
{
   /* ship data, if we're aggregating */
   if (aggRatio)
   {
      /* all non-aggregators send data */
      if (myProcNum % aggRatio)
         MPI_Send(dataBuf, perProcIOSize, MPI_FLOAT, (myProcNum / aggRatio) * aggRatio, H5B_DATA_TAG, MPI_COMM_WORLD);

      /* aggregating processor receives data */
      if (!(myProcNum % aggRatio))
      {
         int k;
	 double ta, tb;

	 ta = MPI_Wtime();
	 /* copy this proc's data into the aggBuf */
	 memcpy(aggBuf, dataBuf, perProcIOSize * sizeof(float));

	 /* receive all other sender's data into aggBuf */
	 for (k = 1; k < aggRatio; k++)
	 {
	    MPI_Status mpiStatus;

	    MPI_Recv((void *) &aggBuf[k*perProcIOSize], perProcIOSize, MPI_FLOAT, myProcNum + k, H5B_DATA_TAG,
	       MPI_COMM_WORLD, &mpiStatus);
       	 }
	 tb = MPI_Wtime();
	 *aggTime += (tb - ta);
      }
   }
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:	HDF5 Benchmark for SAF
 * Purpose: 	Print summary results	
 *
 * Description:	This prints summary results on standard out and also cats them to the cummulative statistics file. 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
H5B_PrintResults(
   double totalTime,		/* [IN] latest stop time minus earliest start time over all processors */
   double adjustedTotalTime,	/* [IN] sum of total time in detected non-outliers */
   double outLiarTotalTime,	/* [IN] sum of total time in detected outliers */ 
   double avgWriteTime,		/* [IN] average time in writes */
   double aggregatingTime,	/* [IN] sum of all time spent aggregating data */
   int argc,			/* [IN] argument count of command line */
   char *argv[]			/* [IN] arguments on the command line */
)
{
   adjustedTotalTime = adjustedTotalTime; /* quiet the compiler */
   if (myProcNum==0)
   {  double totalBytes = (double) numCycles * (double) numDatasets * (double) perProcIOSize * (double) numProcs * sizeof(float);
      double writeBytes = ((double) (aggRatio?aggRatio:1)) * (double) perProcIOSize * (double) sizeof(float);
      int i,megaByte = (1<<20);
      FILE *f;

      printf("*********************************************************************\n");
      printf("********************* Command Line and Environment ******************\n");
      printf("*********************************************************************\n");
      printf("command line...\n");
      printf("   ");
      for (i = 0; i < argc; i++)
	 printf("%s ", argv[i]);
      printf("\n\n\n");
      printf("Enviornment...\n");
      i = 0;
      while (environ[i] && strlen(environ[i]))
         printf("%s\n", environ[i++]);
      printf("*********************************************************************\n");
      printf("************************** Aggregate Results ************************\n");
      printf("*********************************************************************\n");
      printf("!!!!!!!!!!!!!!!!!!WARNING: SAME NUMBERS, NEW TERMINOLOGY!!!!!!!!!!!!!\n");
      printf("total bytes written                                        = %f Megabytes\n", totalBytes / megaByte);
      printf("total time                                                 = %f seconds\n", totalTime);
      if (aggRatio)
         printf("total time spent aggregating data                         = %f seconds\n", aggregatingTime);
      printf("overall bandwidth (totalBytes / totalTime)                 = %f Megabytes/second\n", totalBytes/megaByte/totalTime);
      printf("bytes written by every %02dth processor in each write        = %f Megabytes\n", aggRatio?aggRatio:1,
	 writeBytes / megaByte);
      printf("average time in write over all calls on all writers        = %f seconds\n", avgWriteTime);
      if (aggRatio)
         printf("total time spent aggregating data                         = %f seconds\n", aggregatingTime);
      printf("avg local raw bandwidth                                    = %f Megabytes/second\n",
         writeBytes/megaByte/avgWriteTime);
      /* note, since each proc sees writeBytes/megaByte/avgWriteTime Mb/sec to the same file, the total is numProcs
	 times that amount */
      printf("avg global raw bandwidth                                   = %f Megabytes/second\n",
	 numProcs*writeBytes/megaByte/avgWriteTime/(aggRatio?aggRatio:1));
      printf("ratio of overall to raw                                    = %4.1f %%\n",
	 100.0 * totalBytes/megaByte/totalTime / (numProcs*writeBytes/megaByte/avgWriteTime/(aggRatio?aggRatio:1)));
      f = fopen(cummStatFileName,"a");
      fprintf(f,"%s\t%s\t%d\t%d\t%d\t\t%d\t% 5.1f\t% 5.1f\t% 5.1f\t%d\t%s\t%s\t%s\t%s\t%s\n", hostName, h5VersionStr,
	 numProcs, numNodes, perProcIOSize*(int)sizeof(float), aggRatio,
	 totalBytes / megaByte, totalBytes / megaByte / totalTime,
	 numProcs * writeBytes / megaByte / avgWriteTime / (aggRatio?aggRatio:1),
	 alignSize, useIndependent ? "yes" : "no", overWrite ? "yes" : "no", closePerCycle ? "yes" : "no", 
	 numHints ? "yes" : "no", "no");
      fclose(f);
      printf("summary results appended to %s\n",cummStatFileName);
      if (outLiarTotalTime != 0.0)
      {
         printf("\n\nEncountered some outliars totaling %f seconds\n\n", outLiarTotalTime);
         printf("total bytes written                                                = %f Megabytes\n", totalBytes / megaByte);
         printf("adjusted total time                                                = %f seconds\n", totalTime - outLiarTotalTime);
         printf("adjusted bandwidth (totalBytes / (totalTime - outLiarTotalTime))   = %f Megabytes/second\n",
	    totalBytes / megaByte / (totalTime - outLiarTotalTime));
      }
      else
         printf("\n\nDid not encounter any outliars\n\n");
   }
}


int
main(int argc, char **argv)
{
   char defaultName[128] = "h5bench", *fileName = &defaultName[0];
   char dummyHintKey[128], dummyHintVal[128];
   char *dummyHintKey_p = dummyHintKey, *dummyHintVal_p = dummyHintVal;
   char **hintKeys, **hintVals;
   int NOmultifile = 0;
   int perNodeIOProcs = 0;
   int doSmallData = 0;
   int metaBlock = 0;
   int unBalanced = 0;
   int exitStatus = 0;
   int i,j;
   float *dataBuf=NULL, *aggBuf=NULL;
   double t0,t1,avgH5DWriteTime=0.0,avgPosixWriteTime=0.0,aggregatingTime;
   hid_t fileAccessProps, datasetCreationProps, datasetTransferProps,
         wholeDataspace, fileDataspace, memDataspace, fileAggDataspace, memAggDataspace;
   MPI_Info mpiInfo;

   Zero = Zero; /* quiet the compiler */

   /* Issue: This is really only a parallel test. It doesn't make much sense to run it in serial */ 
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &myProcNum);
   MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

   /* since we want to see whats happening make sure stdout and stderr are unbuffered */
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);

   /* process the basic argument list */
   STU_ProcessCommandLine(0, argc, argv,
      "-asciiPosix",
	 "if specified, instead of writing an HDF5 file, write an braindead ASCII file via POSIX (open/write) interface", 
         &asciiPosix,
      "-doSmallData",
	 "if specified, add a small, extendible dataset with intermittent writes to it [not yet implemented]",
         &doSmallData,
      "-numDatasets %d",
	 "specify the number of datasets to write in each cycle [100]",
	&numDatasets,
      "-perProcIOSize %d",
	 "specify size of each I/O request (in floats) per processor [20,000]",
         &perProcIOSize,
      "-perNodeIOProcs %d",
	 "specify number of processors on a node to be used for real I/O to the file [not implemented]",
         &perNodeIOProcs,
      "-numCycles %d",
	 "number of cycles (iterations through loop) in which numDatasets are written [5]",
	 &numCycles,
      "-name %s",
	 "specify output database name [\"h5bench.hdf5\"]",
	 &fileName,
      "-NOmultifile",
	 "if specified, DO NOT write each cycle (iteration) to a new hdf5 file [no]",
         &NOmultifile,
      "-overWrite",
	 "rather than writing new datasets each cycle, overwrite the ones created on the first cycle [no]",
         &overWrite,
      "-perProcStats",
	 "after running the test and collecting the stats, write per-processor stats in addition to max-stats [no]",
         &perProcStats,
      "-useIndependent",
	 "use independent rather than collective I/O",
         &useIndependent,
      "-align %d",
	 "specify power-of-2 alignment, in bytes, for datasets [0]",
         &alignSize,
      "-metaBlock %d",
	 "set meta data aggregation block size [0]",
         &metaBlock,
      "-aggRatio %d",
	 "specify the aggregation ratio--number of processor's requests that are collected onto a common processor "
	 "prior to issuing a write to disk [0]",
         &aggRatio,
      "-closePerCycle",
	 "for single file output, close the file each cycle [no]",
         &closePerCycle,
      "-mpioHint %s %s",
	 "specify one or more key/value pairs to pass as hints to MPIO (arg1 is key, arg2 is value). "
	 "Specify as many -mpioHint switches as you need to. Consult your local parallel filesystem user's guide "
	 "for hints that may be applicable on your platform.",
         &dummyHintKey_p, &dummyHintVal_p,
      STU_END_OF_ARGS);

   /* process mpio hint command line arguments */
   H5B_ProcessCommandLineHints(argc, argv, &numHints, &hintKeys, &hintVals); 

   /* sanity checks */
   if (!NOmultifile)
   {
      closePerCycle = 1;
      overWrite = 0;
   }
   if (aggRatio)
   {
#if 0
      int i = 0;
      int globMemSize = 0;

      /* search the env for "P4_GLOBMEMSIZE" and fail if it doesn't exist or isn't big enough */
      while (environ[i] && strlen(environ[i]))
      {
	 if (!strncmp(environ[i],"P4_GLOBMEMSIZE",strlen("P4_GLOBMEMSIZE")))
	 {
	    sscanf(environ[i],"P4_GLOBMEMSIZE=%d",&globMemSize);
	    break;
	 }
	 i++;
      }
      if (globMemSize < 20 * aggRatio * numProcs * perProcIOSize * sizeof(float))
      {
	 printf("The env. variable, \"P4_GLOBMEMSIZE=%d\" is not set or not set large enough. Try setting it to %d\n",
	    globMemSize, 20 * aggRatio * numProcs * perProcIOSize * (int) sizeof(float));
	 MPI_Abort(MPI_COMM_WORLD,1);
      }
#endif

#if defined(sgi) && defined(MPICH_NAME) && MPI_VERSION==1
      if (!myProcNum)
         printf("Warning: Data Aggregation Turned off on SGI's due to a problem with malloc's in MPICH\n");
      aggRatio = 0;
#else
      useIndependent = 1;
      if (aggRatio > numProcs)
	 aggRatio = numProcs;
      while (numProcs % aggRatio)
	 aggRatio++;
#endif

   }

   /* compute processors per node */
   H5B_ComputeNodeConfiguration(hostName, &numNodes, &tasksPerNode, &unBalanced);

   /* output the startup message */
   if (myProcNum==0)
   {
      sprintf(cummStatFileName, "%s/h5bench_cummstats.txt", getenv("HOME") ? getenv("HOME") : ".");
      printf("starting on %d processors (%d nodes with %s%d tasks per node)\n",
	  numProcs, numNodes, unBalanced ? "a maximum of " : "", tasksPerNode);
      printf("will write %5.1f Megabytes over %d cycles...\n",
	 (double) numProcs * numCycles * numDatasets * perProcIOSize * sizeof(float) / (1<<20),
	 numCycles);
   }

   H5B_AllocAndFillDataBuffer(perProcIOSize, aggRatio, &dataBuf, &aggBuf);

   /* initialize HDF5 */
   if (!asciiPosix)
   {
      H5open();
      {  unsigned major, minor, patch;
         H5get_libversion(&major, &minor, &patch);
         sprintf(h5VersionStr,"%u.%u.%u", major, minor, patch);
      }

      /* build some HDF5 objects we'll reuse throughout the run */
      fileAccessProps = H5Pcreate(H5P_FILE_ACCESS);

      /* build any MPI info hints specified on the command line */
      if (numHints)
      {  int badHints = 0;

         MPI_Info_create(&mpiInfo);
         for (i = 0; i < numHints; i++)
	 {  int nKeys;
	    MPI_Info_set(mpiInfo, hintKeys[i], hintVals[i]);
	    MPI_Info_get_nkeys(mpiInfo, &nKeys);
	    if (nKeys != i+1)
	    {
	       printf("hint key \"%s\" with value \"%s\" was apparently rejected during MPI_Info_set() on processor %d\n",
		  hintKeys[i], hintVals[i], myProcNum);
	       badHints = 1;
	    }
	 }
	 if (badHints)
	    MPI_Abort(MPI_COMM_WORLD,1);
         H5B_ValidateHints(mpiInfo);
#if H5_VERS_MAJOR*1000000+H5_VERS_MINOR*1000+H5_VERS_RELEASE < 1007018
         H5Pset_fapl_mpio(fileAccessProps, MPI_COMM_WORLD, mpiInfo);
#else
         H5Pset_fapl_mpio(fileAccessProps, MPI_COMM_WORLD, mpiInfo, H5FD_MPIO_FS_DEFAULT);
#endif
      } else {
#if H5_VERS_MAJOR*1000000+H5_VERS_MINOR*1000+H5_VERS_RELEASE < 1007018
          H5Pset_fapl_mpio(fileAccessProps, MPI_COMM_WORLD, MPI_INFO_NULL);
#else
          H5Pset_fapl_mpio(fileAccessProps, MPI_COMM_WORLD, MPI_INFO_NULL, H5FD_MPIO_FS_DEFAULT);
#endif
      }

      /* set other HDF5 file properties */
      if (alignSize)
         H5Pset_alignment(fileAccessProps, 0, (1<<alignSize));
      if (metaBlock)
         H5Pset_meta_block_size(fileAccessProps, metaBlock);

      datasetCreationProps = H5Pcreate(H5P_DATASET_CREATE);
      H5Pset_layout(datasetCreationProps, H5D_CONTIGUOUS);

      datasetTransferProps = H5Pcreate(H5P_DATASET_XFER);
      if (useIndependent)
         H5Pset_dxpl_mpio(datasetTransferProps, H5FD_MPIO_INDEPENDENT);
      else
         H5Pset_dxpl_mpio(datasetTransferProps, H5FD_MPIO_COLLECTIVE);

      /* HDF5 dataspaces and selections */
      {  const hsize_t wholeSize = (hsize_t) perProcIOSize * numProcs;
         const hsize_t myCount = (hsize_t) perProcIOSize;
	 const hsize_t myAggCount = aggRatio * myCount;
         const hssize_t myStart = (hsize_t) perProcIOSize * myProcNum;
         const hssize_t myAggStart = (hsize_t) perProcIOSize * (myProcNum / (aggRatio?aggRatio:1)) * aggRatio; 

         wholeDataspace = H5Screate_simple(1,&wholeSize,&wholeSize);
         fileDataspace = H5Scopy(wholeDataspace);
         H5Sselect_hyperslab(fileDataspace, H5S_SELECT_SET, &myStart, NULL, &myCount, NULL);
	 if (aggRatio)
	 {
            fileAggDataspace = H5Scopy(wholeDataspace);
            H5Sselect_hyperslab(fileAggDataspace, H5S_SELECT_SET, &myAggStart, NULL, &myAggCount, NULL);
	 }
         memDataspace = H5Screate_simple(1,&myCount,&myCount);
         H5Sselect_all(memDataspace);
	 if (aggRatio)
	 {
            memAggDataspace = H5Screate_simple(1,&myAggCount,&myAggCount);
            H5Sselect_all(memAggDataspace);
	 }
      }
   }
   else
      sprintf(h5VersionStr,"posix");

   /* set zero time, t0 */
   t0 = MPI_Wtime();
   {  double mint0;
      MPI_Allreduce(&t0, &mint0, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
      t0 = mint0;
   }

   /*****************************************************************
    ********************  BEGIN MAIN I/O LOOP ***********************
    *****************************************************************/
   for (i = 0; i < numCycles; i++)
   {
      hid_t theGroup;
      hid_t hFile;
      int aFile;
      double ta, tb;

      H5B_CreateOrOpenFile(closePerCycle, NOmultifile, i, fileName, fileAccessProps, &hFile, &aFile);

      /* write a bunch of datasets */
      for (j = 0; j < numDatasets; j++)
      {
	 hid_t theDataset;

	 /* create or open the dataset */
	 if (!asciiPosix)
	    H5B_CreateOrOpenDataset(hFile, overWrite, i, j, wholeDataspace, datasetCreationProps, &theDataset, &theGroup);

	 H5B_AggregateData(myProcNum, aggRatio, dataBuf, aggBuf, &aggregatingTime);

	 /* write data */
	 if (asciiPosix)
	 {
	    size_t myDataSize = perProcIOSize * sizeof(float);
	    size_t dataSegmentSize = numProcs * myDataSize; 
	    size_t cycleSegmentSize = numDatasets * dataSegmentSize; 
	    size_t seekSet = j*dataSegmentSize + (aggRatio ? (myProcNum/aggRatio)*aggRatio*myDataSize : myProcNum*myDataSize);

	    if (NOmultifile)
	       seekSet += i*cycleSegmentSize;

	    if (lseek64(aFile, (off64_t) seekSet, SEEK_SET) < 0)
	       MPI_Abort(MPI_COMM_WORLD,errno);
	    ta = MPI_Wtime();
	    if (aggRatio)
	    {
	       if (!(myProcNum % aggRatio))
	          if (write(aFile, aggBuf, aggRatio * myDataSize) != aggRatio * myDataSize)
	             MPI_Abort(MPI_COMM_WORLD,errno);
	    }
	    else
	    {
	       if (write(aFile, dataBuf, myDataSize) != myDataSize)
	          MPI_Abort(MPI_COMM_WORLD,errno);
	    }
	    tb = MPI_Wtime();
	    avgPosixWriteTime += (tb - ta);

	 }
	 else
	 {
	    if (aggRatio)
	    {
	       if (!(myProcNum % aggRatio))
	          H5Dwrite(theDataset, H5T_NATIVE_FLOAT, memAggDataspace, fileAggDataspace, datasetTransferProps, aggBuf);
	    }
	    else
	       H5Dwrite(theDataset, H5T_NATIVE_FLOAT, memDataspace, fileDataspace, datasetTransferProps, dataBuf);
         }

	 /* close dataset */
	 if (!asciiPosix)
	    H5Dclose(theDataset);
      }

      /* close file */
      if ((i + 1 == numCycles) || closePerCycle)
      {
	 if (NOmultifile)
	 {
	    if (!asciiPosix)
	       H5Gclose(theGroup);
	 }
	 if (asciiPosix)
	    close(aFile);
	 else
	    H5Fclose(hFile);
      }
      else
      {
         if (!overWrite)
	 {
	    if (!asciiPosix)
	       H5Gclose(theGroup);
	 }
      }

      if (myProcNum==0)
	 printf("   finished cycle %d of %d\n", i+1, numCycles); 
   }
   if (asciiPosix)
      avgPosixWriteTime /= (numCycles * numDatasets);

   /*****************************************************************
    ********************* END MAIN I/O LOOP *************************
    *****************************************************************/

   /* set stop time */
   t1 = MPI_Wtime();
   {  double maxt1;
      MPI_Allreduce(&t1, &maxt1, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
      t1 = maxt1;
   }
   totalTime = t1 - t0; 

   /* close all the HDF5 objects we initially created */
   if (!asciiPosix)
   {
      H5Sclose(memDataspace);
      H5Sclose(fileDataspace);
      if (aggRatio)
      {
         H5Sclose(memAggDataspace);
         H5Sclose(fileAggDataspace);
      }
      H5Sclose(wholeDataspace);
      H5Pclose(datasetTransferProps);
      H5Pclose(datasetCreationProps);
      H5Pclose(fileAccessProps);

      /* terminate HDF5 */
      H5close();

      if (myProcNum==0)
         printf("Writing collected performance data to files...\n");

      H5B_H5Dwrite(0,0,0,0,0,0,H5B_TERMINATING,&avgH5DWriteTime);
      H5B_H5Dopen(0,0,H5B_TERMINATING,NULL);
      H5B_H5Dclose(0,H5B_TERMINATING,NULL);
      H5B_H5Dcreate(0,0,0,0,0,H5B_TERMINATING,NULL);
      H5B_H5Fopen(0,0,0,H5B_TERMINATING,NULL);
      H5B_H5Fcreate(0,0,0,0,H5B_TERMINATING,NULL);
      H5B_H5Fclose(0,H5B_TERMINATING,NULL);
      H5B_H5Gcreate(0,0,0,H5B_TERMINATING,NULL);
      H5B_H5Gclose(0,H5B_TERMINATING,NULL);
   }

   /* write out summary info */
   if (asciiPosix)
      H5B_PrintResults(totalTime,adjustedTotalTime,outLiarTotalTime,avgPosixWriteTime,aggregatingTime,argc,argv);
   else
      H5B_PrintResults(totalTime,adjustedTotalTime,outLiarTotalTime,avgH5DWriteTime,aggregatingTime,argc,argv);

   if (dataBuf)
      free(dataBuf);
   if (aggBuf)
      free(aggBuf);

   if (numHints && !asciiPosix)
      MPI_Info_free(&mpiInfo);
   MPI_Bcast(&exitStatus, 1, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Finalize();

   return exitStatus;

}



/* PRE-AMBLE and POST-AMBLE stubs for ease of re-defining HDF5 interface */
#define H5B_STUB_PREAMBLE							\
   static int first = 1;							\
   static int callCount = 0;							\
   static double *callTimes;							\
   int allocSize = numCycles * numDatasets;					\
   hid_t result = 0;								\
   herr_t errResult = 0;							\
   double t, dt;								\
 										\
   result = 0;									\
   errResult = 0;								\
   result = result;								\
   errResult = errResult;							\
   if (first)									\
   {										\
      first = 0;								\
      callTimes = (double *) calloc(allocSize,sizeof(double));			\
   }										\
   if (status == H5B_RUNNING)							\
   {										\
      t = MPI_Wtime();

#define H5B_STUB_POSTAMBLE(funcName)						\
      if (errResult < 0 || result < 0)						\
      {										\
	 MPI_Abort(MPI_COMM_WORLD,1);						\
	 exit(1);								\
      }										\
      dt = MPI_Wtime() - t;							\
      callTimes[callCount++] = dt;						\
   }										\
   else										\
   {  int i, j;									\
      double *maxTimes;								\
      char tmpName[32];								\
      FILE *f;									\
      if (perProcStats)								\
      {										\
         for (j = 0; j < numProcs; j += H5B_PROC_GROUP)				\
         {									\
	    if (j <= myProcNum && myProcNum < j + H5B_PROC_GROUP)		\
	    {									\
	       sprintf(tmpName,"%s.times_%03d_of_%03d", #funcName, myProcNum, numProcs);\
               f = fopen(tmpName,"w");						\
               fprintf(f,"time-per-call history for " #funcName " on processor %d of %d\n", myProcNum, numProcs);\
               for (i = 0; i < callCount; i++)					\
                  fprintf(f, "%f\n", callTimes[i]);				\
	    }									\
         }									\
      }										\
      if (avgCallTime)								\
      {  double avgTime = 0.0, allAvgTime;					\
	 for (i = 0; i < callCount; i++)					\
	    avgTime += callTimes[i];						\
	 if (callCount)								\
	    avgTime /= callCount;						\
         MPI_Reduce((void*) &avgTime, (void*) &allAvgTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);\
	 *avgCallTime = allAvgTime / numProcs;					\
	 if (aggRatio)								\
	    *avgCallTime *= aggRatio; 						\
      }										\
      if (myProcNum == 0)							\
         maxTimes = (double *) calloc(allocSize,sizeof(double));		\
      MPI_Reduce(callTimes, maxTimes, allocSize, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);\
      if (myProcNum == 0)							\
      {	 double avg = 0.0, stdDev = 0.0, adjustedTotal = 0.0, outLiarTotal = 0;	\
	 double max = 0.0, totalTimeInCall = 0.0;				\
	 int outLiars = 0;							\
	 int maxCall = 0;							\
										\
	 for (i = 0; i < callCount; i++)					\
	    avg += maxTimes[i];							\
	 totalTimeInCall = avg;							\
	 if (callCount)								\
	    avg /= callCount;							\
	 for (i = 0; i < callCount; i++)					\
	    stdDev += (avg - maxTimes[i]) * (avg - maxTimes[i]);		\
	 if (callCount)								\
            stdDev /= callCount;						\
	 stdDev = sqrt(stdDev);							\
	 if ((avg > 0.01) && (stdDev > 2*avg))					\
	 {									\
	    for (i = 0; i < callCount; i++)					\
	    {									\
	       if (maxTimes[i] > max)						\
	       {								\
		  max = maxTimes[i];						\
		  maxCall = i;							\
	       }								\
	       if (maxTimes[i] < avg + stdDev)					\
	          adjustedTotal += maxTimes[i];					\
	       else								\
	       {								\
	          outLiarTotal += maxTimes[i];					\
	          outLiars++;							\
	       }								\
	    }									\
            adjustedTotalTime += adjustedTotal;					\
	    outLiarTotalTime += outLiarTotal;					\
	 }                                                                      \
         else                                                                   \
         {                                                                      \
            for(i = 0; i < callCount; i++)                                      \
	    {                                                                   \
	       if(maxTimes[i] > max)                                            \
	       {                                                                \
	          max = maxTimes[i];                                            \
	          maxCall = i;                                                  \
	       }                                                                \
	    }                                                                   \
         }                                                                      \
	 printf( #funcName ":\tavg=%9.4f,max=%9.4f,dev=%9.4f,total=%9.4f,called=%d\n",	\
	    avg, max, stdDev, totalTimeInCall, callCount);					\
         sprintf(tmpName,"%s.times_max_of_%03d", #funcName, numProcs);		\
         f = fopen(tmpName,"w");						\
         fprintf(f,"maximum time-per-call history for " #funcName " on %d processors\n", numProcs);\
         fprintf(f,"average = %f, stdDev = %f, max = %f\n", avg, stdDev, max);	\
         fprintf(f,"max time occured at call %d\n", maxCall);			\
	 fprintf(f,"#outLiars = %d, timeInOutLiars = %f\n", outLiars, outLiarTotal);\
	 fprintf(f,"details...\n");						\
         for (i = 0; i < callCount; i++)					\
            fprintf(f, "%f\n", maxTimes[i]);					\
	 free(maxTimes);							\
	 fclose(f);								\
      }										\
      free(callTimes);								\
   }


/**********************************************************************************************************************
 *                                                                                                                    * 
 *                                                                                                                    * 
 *                                              REDEFINE THE HDF5 API                                                 * 
 *                                                                                                                    * 
 *                                                                                                                    * 
 **********************************************************************************************************************/

/* Implement the re-defined HDF5 API */
#undef H5Fcreate
#undef H5Fopen
#undef H5Fclose
#undef H5Dcreate
#undef H5Dopen
#undef H5Dclose
#undef H5Dwrite
#undef H5Gcreate
#undef H5Gclose

herr_t H5B_H5Dwrite(hid_t dataset_id, hid_t mem_type_id, hid_t mem_space_id, hid_t file_space_id,
	            hid_t xfer_plist_id, const void *buf, int status, double *avgCallTime)
{
   H5B_STUB_PREAMBLE
      errResult = H5Dwrite(dataset_id, mem_type_id, mem_space_id, file_space_id, xfer_plist_id, buf);
   H5B_STUB_POSTAMBLE(H5Dwrite)
   return errResult;
}

hid_t H5B_H5Dopen(hid_t loc_id, const char *name, int status, double *avgCallTime)
{
   H5B_STUB_PREAMBLE
      result = H5Dopen(loc_id, name);
   H5B_STUB_POSTAMBLE(H5Dopen)
   return result;
}

hid_t H5B_H5Dcreate(hid_t loc_id, const char *name, hid_t type_id, hid_t space_id, hid_t create_plist_id, int status,
                    double *avgCallTime)
{
   H5B_STUB_PREAMBLE
      result = H5Dcreate(loc_id, name, type_id, space_id, create_plist_id);
   H5B_STUB_POSTAMBLE(H5Dcreate)
   return result;
}

herr_t H5B_H5Dclose(hid_t dataset_id, int status, double *avgCallTime)
{
   H5B_STUB_PREAMBLE
      errResult = H5Dclose(dataset_id);
   H5B_STUB_POSTAMBLE(H5Dclose)
   return errResult;
}

hid_t H5B_H5Fopen(const char *name, unsigned flags, hid_t access_id, int status, double *avgCallTime)
{
   H5B_STUB_PREAMBLE
      result = H5Fopen(name, flags, access_id);
   H5B_STUB_POSTAMBLE(H5Fopen)
   return result;
}

hid_t H5B_H5Fcreate(const char *name, unsigned flags, hid_t create_id, hid_t access_id, int status, double *avgCallTime)
{
   H5B_STUB_PREAMBLE
     result = H5Fcreate(name, flags, create_id, access_id);
   H5B_STUB_POSTAMBLE(H5Fcreate)
   return result;
}

herr_t H5B_H5Fclose(hid_t file_id, int status, double *avgCallTime)
{
   H5B_STUB_PREAMBLE
      errResult = H5Fclose(file_id);
   H5B_STUB_POSTAMBLE(H5Fclose)
   return errResult;
}

hid_t H5B_H5Gcreate(hid_t loc_id, const char *name, size_t size_hint, int status, double *avgCallTime)
{
   H5B_STUB_PREAMBLE
     result = H5Gcreate(loc_id, name, size_hint);
   H5B_STUB_POSTAMBLE(H5Gcreate)
   return result;
}

herr_t H5B_H5Gclose(hid_t group_id, int status, double *avgCallTime)
{
    H5B_STUB_PREAMBLE
      errResult = H5Gclose(group_id);
    H5B_STUB_POSTAMBLE(H5Gclose)
    return errResult;
}

#ifdef malloc
#undef malloc

void* H5B_malloc(int n, int line)
{
   printf("malloc'ing %d bytes at line %d\n", n, line);
   return malloc(n);
}
#endif
