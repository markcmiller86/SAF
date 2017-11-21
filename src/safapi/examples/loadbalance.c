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
#include <saf.h>
#include <../test/testutil.h>
#include <exampleutil.h>

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Dynamic Load Balance Use Case
 * Description: This is testing code that demonstrates how to use SAF to output a mesh in which elements are being
 *		shifted amoung processors between each restart /dump/. The bulk of the code here is used simply to create some
 *              interesting meshes and has nothing to do with reading/writing from/to SAF. The only routines in which SAF
 *              calls are made are main(), OpenDatabase(), WriteCurrentMesh(), UpdateDatabase(), CloseDatabase() and
 *		ReadBackElementHistory(). As such, these are the only /Public/ functions defined in this file. If you are
 *		interested in seeing the private stuff, and don't see it in what you are reading, you probably need to re-gen
 *		the documentation with /mkdoc/ and specify an audience of /Private/ (as well as /Public/) with the -a option.
 *		If you are viewing this documentation with an HTML browser, don't forget to following the links to the actual
 *		source subroutines described here.
 *
 *		This use case can generate 1, 2 or 3D, unstructured meshes of edge, quad or hex elements, respectively. Use the
 *		-dims %d %d %d command line option (always pass three args even if you want a 1 or 2D mesh in which case pass
 *		zero for the remaining arg(s)). The default mesh is a 2D mesh of 10 x 10 quads.
 *
 *		Upon each /cycle/ of this use case, a certain number of elements are shifted from one processor to the next
 *		starting with processor 0. This shifting is continued through all the processors until the elements again
 *		appear on processor 0.
 *
 *		In the initial decomposition, elements are assigned round-robin to processors. This is, of course, a 
 *		brain-dead decomposition but is sufficient for purposes of this use case. However, before the round-robin
 *		assignment begins, a certain number of elements are held-back from the list of elements to initially assign.
 *		This is the number of elements to be shifted between processors. By defualt, this number is 10. You can specify
 *		a different number using the -numToShift %d command line argument. These elements are the elements of highest
 *		numerical index in the global collection of elements on the whole mesh. Thus, in the default case, elements
 *		90...99 wind up as the shifted elements.  Initially, these elements are assigned to processor 0. Then, with
 *		each cycle output, they are shifted to the next processor. Consequently, in each /cycle/ output by this use
 *		case, the element-based processor decomposition is, indeed, a /partition/ of the whole. No elements are 
 *		shared between processors. This decomposition is illusrtrated for the default case run on 3 processors in
 *		"loadbalance diagrams-2.gif".
 *
 *		[figure loadbalance_diagrams-1.gif]	
 *
 *		Since each /cycle/ in the output is a different decomposition of the whole, we create different /instances/
 *		of the whole mesh in an /self/ collection on the whole. The interpretation is that the top-level set
 *		and every member of the /self/ collection on that set are equivalent point-sets. They are, indeed, the
 *		same set. However, each is decomposed into processor pieces differently.
 *
 *		Two fields are created. One is the coordinate field for the mesh; a piecewise-linear field with 1 dof for each
 *		node in the problem. The other is a pressure field with the following time-space behavior...
 *
 *		      2
 *		     t 
 *		  -------
 *		        2
 *		  (1 + r) 
 *
 *		where /t/ is time and /r/ is distance from the origin. This is a piecewise constant field with 1 dof for each
 *		element in the problem.
 *
 *		Finally, both the the coordinate field on the whole on the given dump and the pressure field on the whole
 *		are written to a state-field.  Due to the fact that the state/suite interface does not currently support
 *		subsuites (that is subsetting of a suite) we are forced to create a separate state/suite for each dump. This
 *		will be corrected in the SAF library shortly. A high-level diagram of the Subset Relation Graph (SRG) is
 *		illustrated in "loadbalance diagrams-2.gif"
 *
 *		[figure loadbalance_diagrams-2.gif]
 *
 *		Note that this diagram does *not* show the field indirections from a particular decomposed set to its processor
 *		pieces. That would have made the diagram even busier than it already is.
 *
 *		Optionally, this use case will output the /dump/history/ of a given element you specify with the -histElem %d
 *		command-line option. Within the context of this use case, we define the /dump/history/ of an element to be 
 *		the sequence of processor assignments and pressure dofs for each /dump/cycle/ of the use case. If you specify
 *		an element number in the interval [N-numToShift...N-1], where /N/ is the total number of elements in the mesh,
 *		the element's processor assignement should vary, increasing by one each step. Otherwise, it should remain
 *		constant. This so because high-numbered elements are shifted while low-numbered ones stay on the processor
 *		they were initially assigned to.
 *
 *		An element's dump history is not output by default. You have to request it by specifying the -histElem %d
 *		command-line option. Also, to confirm that the use case does, indeed, query back from the database the 
 *		correct dump history, it also computes and stores the history as the database is generated. At the end of
 *		the run, it then prints both the history as it was generated and the history as it was queried from the
 *		database for a visual inspection that the results are identical.
 *---------------------------------------------------------------------------------------------------------------------------------
 */


/* we enclose all code here in a single #ifdef HAVE_PARALLEL block so we don't have to individually do it in
   a number of places */
#ifdef HAVE_PARALLEL


#define MAX_FILENAME    256
#define MAX_OBJNAME     32

#define MY_LOCAL_INDEX_TAG	1
#define MY_LOCAL_DOFVAL_TAG	2



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Struct for current mesh params	
 *
 * Description:	This structure houses all the parameters for each current step of the mesh. It is divided into two
 *		kinds of information; global information about the whole mesh that is the same on all processors and
 *		local information that is different on each processor.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct _currentMeshParams {

   /* global mesh parameters, same on all processors */
   AbsRectIndex_t lowerBounds;		/* absolute lower bounds */
   AbsRectIndex_t upperBounds;		/* absolute upper bounds */
   RelRectIndex_t sizeInElems;		/* numer of elements in each dimension */
   RelRectIndex_t sizeInNodes;		/* number of nodes in each dimension */
   RelIndex_t	  numElems;		/* total (global) number of elements */
   RelIndex_t	  numNodes;		/* total (global) number of nodes */

   /* local mesh parameters, unique to each processor */
   hbool_t  isLongList;		/* flag indicating if this processor currently has the shifted elements or not */
   SAF_Rel	  shortListRel;		/* the relation handle for the short list of elements */
   SAF_Db         *shortListFile;	/* the file handle for where the short list of elements was written */
   RelIndex_t	  numElemsIown;		/* number of elements this processor owns (length of `elemList') */
   RelIndex_t     *elemList;		/* the global element ID's of the elements owned by this processor */ 
   RelIndex_t     *globalRangeTopoBuf;	/* topology buffer (2^numDims * numElems indices) for elements on this processor. Each
					 * element is specified by references to nodes in the global context */
   RelIndex_t     *localRangeTopoBuf;	/* topology buffer (2^numDims * numElems indices) for elements on this processor. Each
					 * element is specified by references to nodes in the local context on the processor */
   RelIndex_t	  numNodesIown;		/* number of nodes this processor owns (length of `uniqueNodesList') */
   RelIndex_t	  *uniqueNodesList;	/* the sorted list of unique, global IDs of nodes on this processor */ 
   float	  *coordBuf;		/* coordinate data (numDims * numNodes floats) for nodes on this processor */
   float	  *pressureBuf;		/* pressure data (one float for each element on this processor) */

} CurrentMeshParams_t;


/*
 * Prototypes.
 */
void InitMeshParams( int numDims, RelRectIndex_t dims, CurrentMeshParams_t *p );
void PrintCurrentMeshParams( int numProcs, int myProcNum, int numDims, CurrentMeshParams_t p );
float AnalyticPressureFunc( int numDims, float dofCoords[MAX_DIMS], float timeStep );
void AccumulateElementHistory( int myProcNum, int localElemID, float dofVal, ElementHistory_t *pHist );
void InitDecomp( int numDims, int numProcs, int myProcNum, int *numShift, int *histElem, CurrentMeshParams_t *p	);
void UpdateDecomp( int numDims, int numProcs, int myProcNum, int numToShift, int stepNum, int histElem, 
		   CurrentMeshParams_t *m, ElementHistory_t *histBuf );
void OpenDatabase( char *dbname, hbool_t do_multifile, int numDims, int numProcs, DbInfo_t *dbInfo );
void WriteCurrentMesh( DbInfo_t *dbInfo, int theStep, int numDims, int numProcs, int myProcNum,	
		       CurrentMeshParams_t *theMesh, SAF_Field *fieldList, int *fieldListSize );
void ReadBackElementHistory( DbInfo_t *dbInfo, int myProcNum, int histElem, int *numReadBack, ElementHistory_t **hist );

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Initialize mesh parameters
 *
 * Description:	This function initializes the mesh parameters (size and geometric boundaries)
 *
 * Programmer:	Mark Miller, LLNL, Sat Dec 22, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
InitMeshParams(
   int numDims,			/* [IN] number of dimensions in the mesh */
   RelRectIndex_t dims,		/* [IN] size in each dimension */
   CurrentMeshParams_t *p	/* [IN/OUT] mesh parameter structure to initialize */
)
{
   int i;
   RelRectIndex_t maxRectElem, maxRectNode;

   for (i = 0; i < numDims; i++)
   {
      p->lowerBounds.idx[i] = (int) -dims.idx[i] / 2;
      if (dims.idx[i] % 2)
         p->upperBounds.idx[i] =  dims.idx[i] / 2;
      else
         p->upperBounds.idx[i] =  dims.idx[i] / 2 - 1;
      p->sizeInElems.idx[i] = dims.idx[i];
      p->sizeInNodes.idx[i] = dims.idx[i]+1;
   }
   maxRectElem = SubRelRectIndex(numDims, p->sizeInElems, relAllOnes);
   maxRectNode = SubRelRectIndex(numDims, p->sizeInNodes, relAllOnes);
   p->numElems = LinearIndex(numDims, p->sizeInElems, maxRectElem) + 1;
   p->numNodes = LinearIndex(numDims, p->sizeInNodes, maxRectNode) + 1;
   p->isLongList = false;
   p->shortListRel = SS_REL_NULL;
   p->shortListFile = NULL;
   p->numElemsIown = 0;
   p->elemList = NULL;
   p->globalRangeTopoBuf = NULL;
   p->localRangeTopoBuf = NULL;
   p->numNodesIown = 0;
   p->uniqueNodesList = NULL;
   p->coordBuf = NULL;
   p->pressureBuf = NULL;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Print parameters of the current mesh
 *
 * Description:	This function is used primarily for debugging. It prints the parameters of the current mesh.
 *
 * Programmer:	Mark Miller, LLNL, Thu Jan 24, 2002
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
PrintCurrentMeshParams(
   int numProcs,		/* [IN] number of processors */
   int myProcNum,		/* [IN] calling processor rank */
   int numDims,			/* [IN] number of dimensions in current mesh */
   CurrentMeshParams_t p	/* [IN] mesh parameters to be printed */
)
{
   int i;
   int *elemCounts, *nodeCounts;

   /* allocate some buffers to receive data into */
   elemCounts = (int *) malloc(numProcs * sizeof(int));
   nodeCounts = (int *) malloc(numProcs * sizeof(int));

   MPI_Gather(&p.numElemsIown, 1, MPI_INT, elemCounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Gather(&p.numNodesIown, 1, MPI_INT, nodeCounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

   /* only processor 0 prints */
   if (myProcNum == 0)
   {
      printf("Current Mesh Parameters...\n");
      printf("   lowerBounds:  %s\n", AbsRectIndexStr(numDims,p.lowerBounds));
      printf("   upperBounds:  %s\n", AbsRectIndexStr(numDims,p.upperBounds));
      printf("   sizeInElems:  %s\n", RelRectIndexStr(numDims,p.sizeInElems));
      printf("   sizeInNodes:  %s\n", RelRectIndexStr(numDims,p.sizeInNodes));
      printf("   numElems:     %03d\n", p.numElems);
      printf("   numNodes:     %03d\n", p.numNodes);
      printf("   processor    ");
      for (i = 0; i < numProcs; i++)
	 printf("% 6d |", i);
      printf("\n");
      printf("   numElemsIown:");
      for (i = 0; i < numProcs; i++)
	 printf("% 6d |", elemCounts[i]);
      printf("\n");
      printf("   numNodesIown:");
      for (i = 0; i < numProcs; i++)
	 printf("% 6d |", nodeCounts[i]);
      printf("\n");
   }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Define Interesting Pressure Function	
 *
 * Description:	This function is used to define a pressure function...
 *
 *		      2
 *		     t 
 *		  -------
 *		        2
 *		  (1 + r) 
 *
 *		where `t' is the time and r is the radius from the origin.
 *
 * Programmer:	Mark Miller, LLNL, Wed Jan 30, 2002
 *---------------------------------------------------------------------------------------------------------------------------------
 */
float
AnalyticPressureFunc(
   int numDims,			/* [IN] number of dimensions in the mesh */ 
   float dofCoords[MAX_DIMS],	/* [IN] coordinates on the mesh at which to compute the interpolating dof */
   float timeStep		/* [IN] coordinate in time at which to compute the interpolating dof */
)
{
   int i;
   float r = 0.0;

   for (i = 0; i < numDims; i++)
      r += dofCoords[i] * dofCoords[i];

   return timeStep * timeStep / (1.0 + r);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Capture a step in element history from amoung all the processors	
 *
 * Description:	Given a local element index where -1 indicates the element doesn't exist on the calling processor, this
 *		function sets up communication to send and recive a message from the processor that owns the element to
 *		processor 0 so that processor 0 can accumlate that information into a history buffer. It is assumed all
 *		processors but one pass -1. In other words, only one processor thinks it owns the element.
 * 		If that one processor happens to be processor 0, then it just copies the data into the history buffer.
 *		Otherwise, processor 0 does a recieve from any source. The processor that does own the element does a send of
 *		the element's local index on that processor followed by another send of the dof value for the element. Processor
 *		0 recieves both messages. The second message is recieved specifically from the sender of the first message.
 *
 * Programmer:	Mark Miller, LLNL, Fri Feb 1, 2002
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
AccumulateElementHistory(
   int myProcNum,	/* calling processor's rank in MPI_COMM_WORLD */
   int localElemID,	/* the local ID of the element on this processor or -1 of it isn't on this processor */
   float dofVal,	/* the field dof on this element on this processor */
   ElementHistory_t *pHist /* the member of the history buffer to put the values into */
)
{
   if (localElemID != -1 && myProcNum == 0)
   {
      /* processor 0 owns the value so no messaging is necessary */
      pHist->processorID = 0;
      pHist->localElemID = localElemID;
      pHist->dofValue = dofVal;
   }
   else
   {

      /* proc 0 prepares to receive from anyone */
      if (myProcNum == 0)
      {
	 int sender, localID;
	 float localDofVal;
	 MPI_Status status;

	 MPI_Recv(&localID, 1, MPI_INT, MPI_ANY_SOURCE, MY_LOCAL_INDEX_TAG, MPI_COMM_WORLD, &status);
	 sender = status.MPI_SOURCE;
	 MPI_Recv(&localDofVal, 1, MPI_FLOAT, sender, MY_LOCAL_DOFVAL_TAG, MPI_COMM_WORLD, &status);

	 /* now, add the data to the history buffer */
	 pHist->processorID = sender;
	 pHist->localElemID = localID;
	 pHist->dofValue = localDofVal;
      }

      /* the processor that owns the element does a send to proc 0 */
      if (localElemID != -1)
      {
         MPI_Send(&localElemID, 1, MPI_INT, 0, MY_LOCAL_INDEX_TAG, MPI_COMM_WORLD);
	 MPI_Send(&dofVal, 1, MPI_FLOAT, 0, MY_LOCAL_DOFVAL_TAG, MPI_COMM_WORLD);
      }
   }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Compute initial decomposition	
 *
 * Description:	The initial decomposition consists of the round-robin assignment of elements to processors. The number of elements
 *		assigned in this round-robin fashion is the total number of elements *minus* the number to be shifted between
 *		processors on each step. The remaining elements *are*all*assigned*to*every*processor.
 *
 *		However, these elements are placed at the *end* of every processor's local element list. In this way, the shifted
 *		elements can be "added" or "removed" to/from a processor simply by adding or subtracting, respectively, from the
 *		processor's element list length, `numElemsIown'.
 *
 *		Finally, because it is also possible to do so, we compute once and for all the topology relation data for each
 *		processor here rather than doing it on each step in the UpdateDecomp() routine.
 *
 *
 * Programmer:	Mark Miller, LLNL, Tue Jan 8, 2002
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
InitDecomp(
   int numDims,			/* [IN] number of dimensions of mesh. */
   int numProcs,		/* [IN] number of processors. */
   int myProcNum,		/* [IN] calling processor's rank. */
   int *numShift,		/* [IN/OUT] number of elements to be shifted between processors during each step. This value 
				 * will be modified if it is out of range. */
   int *histElem,		/* [IN/OUT] global element ID to compute history for. This value will be modified if it is
				   out of range. */
   CurrentMeshParams_t *p	/* [IN/OUT] mesh parameter structure. The local part of this structure gets updated to reflect
    				 * the list of elements assigned to each processor. */
)
{
   int i;
   int numElems = p->numElems;
   int numToShift = *numShift;
   int maxElemsIwillOwn, assignedToProc, numAssignedToMe;
   RelIndex_t linearElem, *myElemList;

   /* adjust numToShift if necessary */ 
   while ((numElems - numToShift) / numProcs <= 0)
      numToShift--;
   if (numToShift <= 0)
   {
      fprintf(stderr, "fewer elements than processors!\n");
      exit(-1);
   }
   *numShift = numToShift;

   /* adjust histElem, if necessary */
   while (*histElem > numElems)
      (*histElem)--;

   /* compute max number of elements this proc will ever own over the whole run (the `+1' is to deal with integer round-off) */
   maxElemsIwillOwn = (numElems - numToShift) / numProcs + numToShift + 1;

   /* every processor allocates space for this array of elems */
   myElemList = (RelIndex_t *) malloc(maxElemsIwillOwn * sizeof(RelIndex_t));

   /* assign (numElems - numToShift) elements to processors, round-robin */
   assignedToProc = 0;
   numAssignedToMe = 0;
   for (linearElem = 0; linearElem < numElems - numToShift; linearElem++)
   {
      if (assignedToProc % numProcs == myProcNum)
	 myElemList[numAssignedToMe++] = linearElem;
      assignedToProc++;
   }

   /* assign remaining elements to all processors at the end of their respective lists */
   for (i = numAssignedToMe, linearElem = numElems - numToShift; linearElem < numElems; i++, linearElem++)
      myElemList[i] = linearElem;

   /* compute, once and for all, the global-range topology relation data for this processor's piece (including topology
      for the shifted elements). This defines each element in terms of references to nodes in the global context. It is fixed
      for the entire use-case (but will either include or exclude the shifted elements when we grow and shrink the element list
      for this processor) */
   p->globalRangeTopoBuf = BuildTopologyData(numDims, numAssignedToMe+numToShift, p->sizeInElems, p->sizeInNodes, myElemList);

   /* fill in the local parts of the current mesh params, THAT CAN BE FILLED IN, now */
   p->numElemsIown = numAssignedToMe;
   p->elemList = myElemList;

   /* as a last step in the initialization, add numToShift to the size of the element list on processor of max rank 
      so that when we later subtract it in UpdateDecomp(), it will be correct */
   if (myProcNum == numProcs - 1)
      p->numElemsIown += numToShift;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private	
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Update the decomposition for the current step	
 *
 * Description:	Each step in this use case involves assigning the elements from the shift list to the current processor and
 *		un-assigning them from the previous processor. On the very first step, the elements in the shift list
 *		aren't assigned to any processor so there is no un-assignment step. Every step thereafter, we un-assign the
 *		elements from the previous processor and assign them to the current processor.
 *
 *		Assignment and unassignment is, in reality, very simple. Since the shifted elements *always* occupy the last part
 *		of the element list on any processor, we simply vary the size of the element list for any processor to include
 *		or exclude, respectively, these last elements on that processor.
 *
 *		The hardest part of updating the decomposition is computing the unique set of nodes on a processor. This is
 *		because some of the elements in the shift list may share nodes with elements already on the processor.
 *
 * Issue:	To provide a cross-check of the history that is scanned out of the database later, we accumulate history
 *		as we generate the data.
 *
 * Programmer:	Mark Miller, LLNL, Tue Jan 8, 2002
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
UpdateDecomp(
   int numDims,			/* [IN] number of dimensions of mesh. */
   int numProcs,		/* [IN] number of processors. */
   int myProcNum,		/* [IN] calling processor's rank. */
   int numToShift,		/* [IN] number of elements to be shifted between processors during each step. */
   int stepNum,			/* [IN] step number in the sequence of shifts */
   int histElem,		/* [IN] element for which generated history should be stored */
   CurrentMeshParams_t *m,	/* [IN/OUT] mesh parameter structure. The local part of this structure gets updated to reflect
    				 * the list of elements assigned to each processor, the list of unique nodes on each processor
				 * and the coordinate fields. */
   ElementHistory_t *histBuf	/* [IN/OUT] buffer of history data for the element history is collected for. */
)
{

   int nodesPerElem = 1 << numDims;
   int gainProc, loseProc, numUniqueNodes;
   RelIndex_t *uniqueNodes;

   /* compute processor IDs of processors to gain and lose the shifted elements */
   gainProc = stepNum % numProcs;
   loseProc = gainProc == 0 ? numProcs - 1 : gainProc - 1;

   /* increase and decrease the number of elements on those processors */
   if (myProcNum == gainProc)
   {
      m->numElemsIown += numToShift;
      m->isLongList = true;
   }
   if (myProcNum == loseProc)
   {
      m->numElemsIown -= numToShift;
      m->isLongList = false;
   }

   /* compute the unique set of nodes in the topology data. Note that the resulting list of unique nodes will become the
      node-based processor subset relations */
   UniqueNodes((int)(m->numElemsIown * nodesPerElem), m->globalRangeTopoBuf, &numUniqueNodes, &uniqueNodes);
   m->numNodesIown = numUniqueNodes;
   if (m->uniqueNodesList)
      free(m->uniqueNodesList);
   m->uniqueNodesList = uniqueNodes;

   /* now, compute the local-range topology relation data by remapping the globalRangeTopoBuf */ 
   if (m->localRangeTopoBuf)
      free(m->localRangeTopoBuf);
   m->localRangeTopoBuf = ConvertRange(m->globalRangeTopoBuf, (int)(m->numElemsIown * nodesPerElem), NAMESPACE_GLOBAL, NAMESPACE_LOCAL,
			     uniqueNodes, NAMESPACE_GLOBAL, (int)(m->numNodesIown), MAP_SORTED_TRUE, IN_PLACE_FALSE); 

   /* compute coordinate fields */
   if (m->coordBuf)
      free(m->coordBuf);
   m->coordBuf = BuildCoordinateData(numDims, (int)(m->numNodesIown), m->sizeInNodes, m->lowerBounds, uniqueNodes);

   /* compute an extra "pressure" field */
   if (m->pressureBuf)
      free(m->pressureBuf);
   m->pressureBuf = BuildInterpolatingDofs(PIECEWISE_CONSTANT, numDims, m->sizeInElems, m->lowerBounds, (int)(m->numElemsIown),
		       m->elemList, (float) stepNum, AnalyticPressureFunc);

   /*
    * Here's where we build the history buffer, as generated by the client. This history buffer is accumulated on
    * processor 0, only.  All processors attempt to find the history element in their element maps. Only one should succeed.
    */
   if (histElem >= 0)
   {
      /* figure out where this element is in my element list (a -1 return indicates I don't own it) */
      int localElemID = IndexOfValue(m->elemList,(int)(m->numElemsIown), (RelIndex_t) histElem);
      float dofVal = localElemID == -1 ? 0.0 : m->pressureBuf[localElemID];

      AccumulateElementHistory(myProcNum, localElemID, dofVal, &histBuf[stepNum]);
   }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public	
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Open a new database and do some prepratory work on it
 *
 * Description:	This function creates the initial database and some key objects such as the top-most aggregate set,
 *		the nodes, elems and procs collection categories and the state suite.
 *
 * Programmer:	Mark Miller, LLNL, Sat Dec 22, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
OpenDatabase(
   char *dbname,		/* [IN] name of the database */
   hbool_t do_multifile,	/* [IN] boolean to indicate if each step will go to a different supplemental file */
   int numDims,			/* [IN] number of topological and geometric dimensions in the mesh */
   int numProcs,		/* [IN] number of processors */
   DbInfo_t *dbInfo		/* [OUT] database info object */
)
{
   SAF_DbProps *dbprops=NULL;
   SAF_Db *db;
   SAF_Cat nodes, elems, procs;
   SAF_Set invariantMesh;
   SAF_Db *theFile; 

   numProcs = numProcs; /* quiet the compiler */

   /* create the database */
   dbprops = saf_createProps_database();
   saf_setProps_Clobber(dbprops);
   db = saf_open_database(dbname,dbprops);
   dbInfo->db = db;

   /* declare nodes, elems and blocks categories */
   saf_declare_category(SAF_ALL, db,  "nodes", SAF_TOPOLOGY, 0, &nodes);
   dbInfo->nodes = nodes;
   saf_declare_category(SAF_ALL, db,  "elems", SAF_TOPOLOGY, numDims, &elems);
   dbInfo->elems = elems;
   saf_declare_category(SAF_ALL, db,  "procs", SAF_PROCESSOR, numDims, &procs);
   dbInfo->blocks = procs;

   /* create the invariant set */
   saf_declare_set(SAF_ALL, db, "whole", numDims, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &invariantMesh);
   dbInfo->mainMesh = invariantMesh;

   /* if necessary, create the first supplemental file */
   if (do_multifile)
      theFile = saf_open_database("step_000", dbprops);

   else
      theFile = db;
   dbInfo->currentFile = theFile;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Write current mesh to the SAF database	
 *
 * Description:	This function does all the work of writing the current mesh, including its domain-decomposed topology relation,
 *		processor subset relations, coordinate field, and pressure field to the SAF database.
 *
 * Programmer:	Mark Miller, LLNL, Tue Jan 8, 2002 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
WriteCurrentMesh(
   DbInfo_t *dbInfo,		/* [IN/OUT] database info object */
   int theStep,			/* [IN] current step number */
   int numDims,			/* [IN] number of dimensions in mesh */
   int numProcs,		/* [IN] number of processors */
   int myProcNum,		/* [IN] the rank of calling processor */ 
   CurrentMeshParams_t *theMesh,/* [IN/OUT] current mesh parameters (relation and file handle updated) */ 
   SAF_Field *fieldList,        /* [IN/OUT] list of fields we'll append newly written fields too */
   int *fieldListSize           /* [IN/OUT] On input, the current size of the field list. On output, its new size */
)
{
   char tmpName[MAX_OBJNAME];
   int i;
   RelIndex_t *topoBuf = theMesh->localRangeTopoBuf;
   float *coordBuf = theMesh->coordBuf;
   float *pressureBuf = theMesh->pressureBuf;
   hbool_t isLongList = theMesh->isLongList;
   RelIndex_t *elemList = theMesh->elemList;
   RelIndex_t *nodeList = theMesh->uniqueNodesList;
   int numElems = theMesh->numElems;
   int numNodes = theMesh->numNodes;
   int numElemsIown = theMesh->numElemsIown;
   int numNodesIown = theMesh->numNodesIown;
   int nodesPerElem = (1 << numDims);
   int numHandles;
   SAF_Rel shortListRel = theMesh->shortListRel;
   /*SAF_File shortListFile = theMesh->shortListFile;*/
   SAF_CellType cellType;
   SAF_Set currentMesh, myProcSet;
   SAF_FieldTmpl coords_ctmpl, coords_ftmpl, mesh_coord_ftmpl;
   SAF_FieldTmpl procPressureFtmpl, meshPressureFtmpl;
   SAF_Field coordField, coordComponent[MAX_DIMS], *fields=NULL, coords;
   SAF_Field procPressureField, meshPressureField;
   SAF_Rel rel, trel, *rels = NULL;

   SAF_Db *db = dbInfo->db;
   SAF_Unit umeter;
   SAF_Unit uPascal;
   SAF_Quantity *qPressure;
   SAF_Cat nodes = dbInfo->nodes;
   SAF_Cat elems = dbInfo->elems;
   SAF_Cat procs = dbInfo->blocks;
   SAF_Db *stepFile = dbInfo->currentFile;
  
   saf_find_one_unit(db, "meter", &umeter);
   saf_find_one_unit(db, "pascal", &uPascal);
   qPressure = saf_find_one_quantity(db, "pressure",NULL);
   

   /******************************************************
    ******* this step's global mesh (base-space) *********
    ******************************************************/
   sprintf(tmpName, "mesh_step_%03d", theStep);
   saf_declare_set(SAF_ALL, db, tmpName, numDims, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &currentMesh);
   dbInfo->currentMesh = currentMesh;

   /************************************************************************
    ******* collections on the global mesh (nodes, elems and procs *********
    ************************************************************************/
   /* nodes and elems collections on the current mesh */
   saf_declare_collection(SAF_ALL, &currentMesh, &nodes, SAF_CELLTYPE_POINT, numNodes, SAF_1DC(numNodes), SAF_DECOMP_FALSE);
   switch (numDims)
   {
      case 1: cellType = SAF_CELLTYPE_LINE; break;
      case 2: cellType = SAF_CELLTYPE_QUAD; break;
      case 3: cellType = SAF_CELLTYPE_HEX;  break;
   }
   saf_declare_collection(SAF_ALL, &currentMesh, &elems, cellType, numElems, SAF_1DC(numElems), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &currentMesh, &procs, SAF_CELLTYPE_SET, numProcs, SAF_1DC(numProcs), SAF_DECOMP_TRUE);


   /************************************
    ******* the processor sets *********
    ************************************/
   sprintf(tmpName, "proc_%03d", myProcNum);
   saf_declare_set(SAF_EACH, db, tmpName, numDims, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &myProcSet);

   /***************************************************************************
    ******* collections on the processor sets (nodes, elems and procs *********
    ***************************************************************************/
   saf_declare_collection(SAF_EACH, &myProcSet, &nodes, SAF_CELLTYPE_POINT, numNodesIown, SAF_1DC(numNodesIown), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_EACH, &myProcSet, &elems, cellType, numElemsIown, SAF_1DC(numElemsIown), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_EACH, &myProcSet, &procs, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

   /******************************************************************************
    ******* processor to top subsets relations on elems, nodes and procs *********
    ******************************************************************************/
    saf_declare_subset_relation(SAF_EACH, db, &currentMesh, &myProcSet, SAF_COMMON(&procs), SAF_TUPLES,
       H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &myProcNum, H5I_INVALID_HID, NULL, stepFile); 
    saf_declare_subset_relation(SAF_EACH, db, &currentMesh, &myProcSet, SAF_COMMON(&elems), SAF_TUPLES,
       H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
    if (isLongList)
       saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, elemList, H5I_INVALID_HID, NULL, stepFile); 
    else
    {
       if (SAF_EQUIV(&(shortListRel),NULL))
       {
          saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, elemList, H5I_INVALID_HID, NULL, stepFile); 
	  theMesh->shortListRel = rel;
	  theMesh->shortListFile = stepFile;
       }
       else
          saf_use_written_subset_relation(SAF_EACH, &rel, &shortListRel, SAF_INT, H5I_INVALID_HID, stepFile);
    }
    saf_declare_subset_relation(SAF_EACH, db, &currentMesh, &myProcSet, SAF_COMMON(&nodes), SAF_TUPLES,
       H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, nodeList, H5I_INVALID_HID, NULL, stepFile); 

   /**************************************************************************
    ******* topology relation (elems to nodes) on the processor sets *********
    **************************************************************************/
   /* declare and write topology relation */
   saf_declare_topo_relation(SAF_EACH, db, &myProcSet, &elems, &myProcSet, &nodes, SAF_SELF(db), &myProcSet,
      SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_EACH, &trel, SAF_INT, &nodesPerElem, SAF_INT, topoBuf, stepFile);

   /**********************************************************************
    ******* topology relation (elems to nodes) on the global set *********
    **********************************************************************/
   rels = (SAF_Rel *)saf_allgather_handles((ss_pers_t*)&trel, &numHandles, NULL);
   saf_declare_topo_relation(SAF_ALL, db, &currentMesh, &elems, &currentMesh, &nodes, &procs, &currentMesh,
      SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
   saf_write_topo_relation(SAF_ALL, &trel, SAF_HANDLE, rels, H5I_INVALID_HID, NULL, db);
   free(rels);

   /********************************************************
    ******* coordinate field on the processor sets *********
    ********************************************************/
   /* declare scalar coordinate field template for components */
   saf_declare_field_tmpl(SAF_EACH, db, "proc_coordinate_ctmpl", SAF_ALGTYPE_SCALAR,
      SAF_CARTESIAN, SAF_QLENGTH, 1, NULL, &coords_ctmpl);

   /* declare vector composite field template */ 
   if (numDims > 1)
   {
      SAF_FieldTmpl cftmpls[MAX_DIMS];

      for (i = 0; i < numDims; i++)
         cftmpls[i] = coords_ctmpl;
      saf_declare_field_tmpl(SAF_EACH, db, "proc_coordinate_ftmpl", SAF_ALGTYPE_VECTOR,
         SAF_CARTESIAN, SAF_QLENGTH, numDims, cftmpls, &coords_ftmpl);
   }

   /* declare the scalar, component fields */
   for (i = 0; i < numDims; i++)
   {
      sprintf(tmpName,"coord%1d",i);
      saf_declare_field(SAF_EACH, db, &coords_ctmpl, tmpName, &myProcSet, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems), SAF_FLOAT,
         NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &coordComponent[i]);
   }

   /* declare the vector, composite field */
   if (numDims > 1)
   {
      saf_declare_field(SAF_EACH, db, &coords_ftmpl, "coords", &myProcSet, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems), SAF_FLOAT,
         coordComponent, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coordField);
   }
   else
      coordField = coordComponent[0];

   saf_declare_coords(SAF_EACH, &coordField);
   saf_declare_default_coords(SAF_EACH, &myProcSet, &coordField);

   /* write the coordinate field data on the composite field */
   saf_write_field(SAF_EACH, &coordField, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, (void**) &coordBuf, stepFile);

   /****************************************************
    ******* coordinate field on the global set *********
    ****************************************************/
   fields = (SAF_Field *)saf_allgather_handles((ss_pers_t*)&coordField, &numHandles, NULL);
   saf_declare_field_tmpl(SAF_ALL, db, "mesh_coord_ftmpl", SAF_ALGTYPE_FIELD, NULL,
      SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &mesh_coord_ftmpl);
   saf_declare_field(SAF_ALL, db, &mesh_coord_ftmpl, "coords", &currentMesh, SAF_NOT_APPLICABLE_UNIT, &procs, SAF_NODAL(&nodes, &elems),
      H5I_INVALID_HID, NULL, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coords);
   saf_declare_coords(SAF_ALL, &coords);
   saf_declare_default_coords(SAF_ALL, &currentMesh, &coords);
   saf_write_field(SAF_ALL, &coords, SAF_WHOLE_FIELD, 1, SAF_HANDLE, (void**) &fields, db);
   free(fields);

   /* append the field(s) written here to the list of fields */
   fieldList[(*fieldListSize)++] = coords;

   /******************************************************
    ******* pressure field on the processor sets *********
    ******************************************************/
   /* declare scalar pressure field template */
   saf_declare_field_tmpl(SAF_EACH, db, "Analytic Pressures", SAF_ALGTYPE_SCALAR,
      SAF_CARTESIAN, qPressure, 1, NULL, &procPressureFtmpl);
   saf_declare_field(SAF_EACH, db, &procPressureFtmpl, "pressure", &myProcSet, &uPascal, SAF_SELF(db), SAF_ZONAL(&elems), SAF_FLOAT,
      NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &procPressureField);
   saf_write_field(SAF_EACH, &procPressureField, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, (void**) &pressureBuf, stepFile);

   /***************************************************
    ******* pressure field on the global sets *********
    ***************************************************/
   fields = (SAF_Field *) saf_allgather_handles((ss_pers_t*) &procPressureField, &numHandles, NULL);
   saf_declare_field_tmpl(SAF_ALL, db, "Analytic Pressures", SAF_ALGTYPE_FIELD, NULL,
      SAF_NOT_APPLICABLE_QUANTITY, SAF_NOT_APPLICABLE_INT, NULL, &meshPressureFtmpl);
   saf_declare_field(SAF_ALL, db, &meshPressureFtmpl, "pressure", &currentMesh, SAF_NOT_APPLICABLE_UNIT, &procs, SAF_ZONAL(&elems),
      H5I_INVALID_HID, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &meshPressureField);
   saf_write_field(SAF_ALL, &meshPressureField, SAF_WHOLE_FIELD, 1, SAF_HANDLE, (void**) &fields, db);
   free(fields);

   /* append the field(s) written here to the list of fields */
   fieldList[(*fieldListSize)++] = meshPressureField;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public	
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Query an element's history from the database	
 *
 * Description:	This function queries the pressure dump history for a specific element back out of the database.
 *
 * Issues:	For expediency in completing the use-case, this function was written to be fairly specific to what was written
 *		to the database. Note that the approach taken here is written assuming each dump is a different set in the
 *		/self/ collection on the top-level set.
 *
 *		A more general dump history tool would involve the following...
 *
 *		   Command-line arguments...
 *		      -elemID %d (0 or more times)    identify the element(s) you want dump history for
 *		      -nodeID %d (0 or more times)    identify the node(s) you want dump history for
 *		      -field %s (1 or more times)     identify the field(s) you want dumped for each elem/node
 *                                                       use "all" for all fields
 *	      	      -IDfield                        If the node or element IDs you specified with -elemID or
 *                                                       -nodeID are not native collection indices, specify the name
 *                                                       of the field in which these ID's are stored
 *
 *		   Possible output...
 *
 *		   Dump History for Element 5...
 *
 *		   stepIdx | step coord |  lives in  |    pressure    |   velocity   |    gauss-pts
 *		           |            |            |                |  vx  vy  vz  | d0  d1  d2  d3
 *		   --------|------------|------------|----------------|--------------|-------------
 *		     000   |   0.000    | whole-005  |      2.4       |  0.1 2.0 2.2 | 2.0 2.2 2.2 2.3
 *		     020   |   0.059    | whole-005  |      3.9       |  blah-blah-blah
 *
 *
 *		   or in some decomposed database...
 *		   
 *		   Dump History for Element 16...
 *		   
 *		   stepIdx | step coord |  lives in  |    pressure    |   velocity   |    gauss-pts
 *		           |            |            |                |  vx  vy  vz  | d0  d1  d2  d3
 *		   --------|------------|------------|----------------|--------------|-------------
 *		     000   |   0.000    | proc0-000  |      2.4       |  0.1 2.0 2.2 | 2.0 2.2 2.2 2.3
 *		     000   |   0.000    | proc1-007  |      2.4       |  0.1 2.0 2.2 | 2.0 2.2 2.2 2.3
 *		     000   |   0.000    | proc2-004  |      2.4       |  0.1 2.0 2.2 | 2.0 2.2 2.2 2.3
 *		     020   |   0.059    | proc0-000  |     42.3       |  0.1 2.0 2.2 | 2.0 2.2 2.2 2.3
 *		     020   |   0.059    | proc1-007  |     42.4       |  0.1 2.0 2.2 | 2.0 2.2 2.2 2.3
 *		     020   |   0.059    | proc2-004  |     42.3       |  0.1 2.0 2.2 | 2.0 2.2 2.2 2.3
 *
 *		   In this example, element 16 is shared by three processor pieces. And, this shows example output
 *		   where one processor doesn't agree with the others on the pressure value.
 *
 *		   algorithm...
 *		      1. open the database
 *		      2. find all suites
 *		      3. order suites (by coord value associated with first member state or something)
 *		      4. For each suite...
 *		         a. read a state field
 *		         b. for each field in the state matching field(s) specified on command-line.
 *		               a. if field is inhomog, find pieces on which it is homog
 *		                  1. locate the identified nodes and elements in each piece
 *		                     by reading subset relations and examining them
 *		                  2. for each piece...
 *		                     partially read the field to obtain the dofs for all 
 *		                     specified node/elems
 *		                  3. capture dof values, names of pieces (sets) and local
 *		                     indexes on these sets
 *		               b. otherwise, just partially read the field to obtain the dofs
 *		                  for all specified nodes/elemes 
 *		               c. build up buffers of dof values, names of pieces (sets) and local
 *		                  indexes on these sets
 *
 * Programmer:	Mark Miller, LLNL, Fri Feb 1, 2002
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
ReadBackElementHistory(
   DbInfo_t *dbInfo,	/* database info object */
   int myProcNum,	/* processor rank in MPI_COMM_WORLD */
   int histElem,	/* the element for which history */
   int *numReadBack,	/* number of dump for which history was read back */
   ElementHistory_t **hist /* the resulting history buffer */
)
{
   int i, numHist;
   ElementHistory_t *histBuf;

   SAF_Db *db = dbInfo->db;
   SAF_Cat procs = dbInfo->blocks;
   SAF_Cat elems = dbInfo->elems;
   SAF_Set invariantSet = dbInfo->mainMesh;
   SAF_Set *dumpSets = NULL;

   /* describe the self collection on the aggregate set */
   saf_describe_collection(SAF_ALL, &invariantSet, SAF_SELF(db), NULL, &numHist, NULL, NULL, &dumpSets);

   /* allocate some space for the history buffer */
   histBuf = (ElementHistory_t *) malloc(numHist * sizeof(ElementHistory_t));

   /* for each member of this collection, describe the set and the proc's collection on that set */
   for (i = 0; i < numHist; i++)
   {
      int numPieces, numRels, numFields, numElems, localElemID;
      float dofVal, *dofVals=&dofVal;
      SAF_Set *procSets = NULL;
      SAF_Set myPiece;
      SAF_Rel *ssRels=NULL;
      SAF_Field *theFields = NULL;
      RelIndex_t *elemList = NULL;

      /* get the processor collection at this dump */
      saf_describe_collection(SAF_ALL, &(dumpSets[i]), &procs, NULL, &numPieces, NULL, NULL, &procSets);

      /* this processor will take responsibility for the member of this collection whose index in the collection is the
         same as its rank */
      myPiece = procSets[myProcNum];

      /* how many elements are on this processor's piece? */
      saf_describe_collection(SAF_EACH, &myPiece, &elems, NULL, &numElems, NULL, NULL, NULL);

      /* everybody reads their respective subset relations */
      saf_find_subset_relations(SAF_EACH, db, &(dumpSets[i]), &myPiece, &elems, &elems, SAF_BOUNDARY_FALSE, SAF_BOUNDARY_FALSE,
         &numRels, &ssRels);
      if (numRels != 1)
         AbortThisMess("found more than 1 dumpSet to procSet subset relation on elems");

      /* read the subset relation data */
      saf_read_subset_relation(SAF_EACH, &(ssRels[0]), NULL, (void**) &elemList, NULL);

      /* everybody finds the pressure field on thier respective pieces */
      saf_find_fields(SAF_EACH, db, &myPiece, "pressure", SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY, SAF_ANY_BASIS,
         SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC, &numFields, &theFields);
      if (numFields != 1)
         AbortThisMess("found more than 1 field named \"pressure\" on myPiece set");

      /* everybody checks to see if they own the hist elem. This assumes elemList is sorted in ascending order. */
      localElemID = IndexOfValue(elemList, numElems, (RelIndex_t) histElem);

      /* everybody reads (partially) the field (most procs attempt to read 0 dofs) */
      saf_read_field(SAF_EACH, &(theFields[0]), NULL, localElemID==-1?0:1, SAF_TUPLES, &localElemID, (void**) &dofVals);

      AccumulateElementHistory(myProcNum, localElemID, dofVals[0], &histBuf[i]);

      free(theFields); theFields = NULL;
      free(elemList); elemList = NULL;
      free(ssRels); ssRels = NULL;
      free(procSets); procSets = NULL;
   }

   free(dumpSets);

   /* setup return values */
   *numReadBack = numHist;
   *hist = histBuf;

   return;

}
 

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Dynamic Load Balance Use Case
 * Purpose:	Main program for Dynamic Load Balance Use Case 
 *
 * Description:	This is the main code for the dynamic load balance use case.
 *
 *		Here are the command-line options...
 *		
 *		   -multifile
 *		      each cycle output will be written to different files. Otherwise, it will all be written to one file.
 *		   -numToShift %d
 *		      specify the number of elements to shift on each step [10].
 *		   -meshSize %d %d %d
 *		      specify size of mesh in 1, 2 or 3 dimensions. Specify 0 for each dimension you do not want to have.
 *		      For example, -meshSize 5 0 0 specifies a 1D mesh of size 5 elements [10 10 0]
 *		   -histElem %d
 *		      specify an element, using a global element id, whose pressure history is to displayed at the
 *		      end of the run. In this case, the database is closed and then re-opened. For each instant of the mesh in
 *		      the database, the specified element's pressure is is found by first finding which processor set the
 *		      element was assigned to. This find step is done in parallel. Once the processor-set is known, that
 *		      processor re-opens the database and reads the field, using partial I/O on that specifc set for the 
 *		      specific element and prints a value. If you want to specify an element that you know has been
 *		      shifted, use an element id within <numToShift> elements of the highest element number.
 *
 * Issues:	Only two of the proc-to-top subset relations are different in each step. It would be nice to re-use the
 *		data already written when the relations are identical to some other previous step. A function such as
 *		saf_usewritten_rel(SAF_Rel theRel, SAF_Rel alreadyWrittenRel); would do the job.
 *
 *		It might be nice to provide a -histNode command-line option. Node history is a little different because 
 *		some nodes are shared between processors.
 *
 *		This is intended to be only a parallel client. In serial, this example should be skipped.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 21, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(
   int argc,	/* command line argument count */
   char **argv	/* command line arguments */
)
{
  char dbName[MAX_FILENAME] = "loadbalance.saf";
  hbool_t do_multifile = false;
  hbool_t verbose = false;
  int numToShift = 10;
  int histElem = -1;
  int i, numDims, numFields;
  int failed=0;
  int myProcNum = 0;
  int numProcs = 1;
  RelRectIndex_t meshSize = {{ 10, 10, 0}};
  ElementHistory_t *elemHistAsGenerated;
  ElementHistory_t *elemHistAsReadBack;
  CurrentMeshParams_t currentMeshParams;
  SAF_Field *theFields; 
  DbInfo_t dbInfo;

  /* Issue: This is really only a parallel test. It doesn't make much sense to run it in serial */ 
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myProcNum);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

  /* since we want to see whats happening make sure stdout and stderr are unbuffered */
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  STU_ProcessCommandLine(0, argc, argv,
			 "-multifile",
			 "if specified, write each cycle to a different supplemental file [single file]",
			 &do_multifile,
			 "-numToShift %d",
			 "specify the number of elements to shift each cycle [max{10% on proc 0,1}]",
			 &numToShift,
			 "-dims %d %d %d",
			 "specify size of the mesh in x, y and z dimensions [10 10 0]",
			 &meshSize.idx[0], &meshSize.idx[1], &meshSize.idx[2],
			 "-histElem %d",
			 "specify an element whose pressure history is to be printed at the end of the run [-1]",
			 &histElem,
			 "-dbName %s",
			 "specify output database name [\"loadbalance.saf\"]",
			 &dbName,
			 "-verbose",
			 "provide verbose output during the run",
			 &verbose,
			 STU_END_OF_ARGS); 

  /* compute number of dimensions from size parameters */
  for (numDims = 0; numDims < MAX_DIMS; numDims++)
    if (meshSize.idx[numDims] == 0)
      break;

  saf_init(SAF_DEFAULT_LIBPROPS);

  /* initialize the mesh parameters */ 
  InitMeshParams(numDims, meshSize, &currentMeshParams);

  /* perform initial decomposition */
  InitDecomp(numDims, numProcs, myProcNum, &numToShift, &histElem, &currentMeshParams);

  /* allocate element history buffers, if necessary */
  if (histElem >= 0)
    elemHistAsGenerated = (ElementHistory_t *) malloc((numProcs + 1) * sizeof(ElementHistory_t));

  SAF_TRY_BEGIN
    {

      /* do some preperatory stuff for the database */
      OpenDatabase(dbName, do_multifile, numDims, numProcs, &dbInfo);

      /***********************************************
       ***********************************************
       *                 MAIN LOOP                   *
       ***********************************************
       ***********************************************/
      theFields=calloc(10, sizeof(*theFields));
      for (i = 0; i <= numProcs; i++)
	{
	  numFields = 0;

	  /* update the current mesh relations and fields */
	  UpdateDecomp(numDims, numProcs, myProcNum, numToShift, i, histElem, &currentMeshParams, elemHistAsGenerated);

	  if (verbose)
	    {
	      if (myProcNum == 0)
		printf("---------------------------- step %03d ---------------------------\n", i);
	      PrintCurrentMeshParams(numProcs, myProcNum, numDims, currentMeshParams);
	    }

	  /* write the current mesh and all of its fields */
	  WriteCurrentMesh(&dbInfo, i, numDims, numProcs, myProcNum, &currentMeshParams, &theFields[numFields], &numFields);

	  /* link this mesh instance into the aggregate, update the state fields, flush the database, etc. */
	  UpdateDatabase(&dbInfo, i, numProcs+1, do_multifile, numFields, theFields);

	}
      free(theFields);


      /* if history of an element was requested, query the database and do the history thing */
      if (histElem >= 0)
	{
	  int numReadBack;

	  ReadBackElementHistory(&dbInfo, myProcNum, histElem, &numReadBack, &elemHistAsReadBack);

	  if (myProcNum == 0)
	    {
	      PrintElementHistory(elemHistAsGenerated, numProcs+1, histElem, "Element History As Generated");
	      PrintElementHistory(elemHistAsReadBack, numReadBack, histElem, "Element History As Read Back");
	    }

	}

      /* close the database */
      CloseDatabase(dbInfo);

    }
  SAF_CATCH
    {
      SAF_CATCH_ALL
	{
	  failed = 1;
	}
    }
  SAF_TRY_END

    saf_final();

  if (failed)
    FAILED;
  else
    PASSED;

  MPI_Bcast(&failed, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Finalize();

  return failed; 
}

#else /* HAVE_PARALLEL */

int
main(int argc, char **argv)
{
   saf_init(SAF_DEFAULT_LIBPROPS);
   saf_final();
   SKIPPED;
   argc = argc;
   argv = argv;
   return 0;
}

#endif
