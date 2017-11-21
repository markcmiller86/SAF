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
#include <../test/testutil.h>
#include <exampleutil.h>

typedef struct {
   unsigned char l;     /* level in pyramid of resolution */
   unsigned char r;     /* row within a level in the pyramid */
   unsigned char c;     /* column within a level in the pyramid */
}LRCIndex_t;
hid_t LRCIndex_h5 = -1;


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	H-Adaptive Use Case 
 * Description: This is a simple example of using SAF to represent adaptation in a parallel-decomposed mesh. The use-case
 *		is hard-coded to write the 6 mesh states illustrated in "use case 5-1.gif"
 *
 *		[figure use_case_5-1.gif]
 *
 *		This example is designed to run on only 3 processors. It will abort with error if run in parallel
 *		on any other number of processors.
 *
 *		There are a total of 6 states output. The ending state is identical to the initial state. Other than
 *		a coordinate field, there are no other whole-mesh fields declared. This is simply due to expediency in
 *		completing this example code.
 *
 *		                                               State 0
 *		[figure use_case_5-2.gif]
 *
 *		                                               State 1
 *		[figure use_case_5-3.gif]
 *
 *		                                               State 2
 *		[figure use_case_5-4.gif]
 *
 *		                                               State 3
 *		[figure use_case_5-5.gif]
 *
 *		                                               State 4
 *		[figure use_case_5-6.gif]
 *
 *		                                               State 5
 *		[figure use_case_5-2.gif]
 *
 *		To give elements an immutable, global ID, we use something called the /LRC/ index which is a triple of
 *		the level of adaptivity and then the row and column index within that level starting from the origin
 *		in the upper right. The larges elements are defined, arbitrarily, to be at level 1. So, for example, the 
 *		lower-right child of the one elment that is refined in state 1 has an LRC index of {2,1,3} for level 2,
 *		row 1 and column 3. The LRC indexes are written on each step as alternative indexing for the elements.
 *		In the dialog that follows, any reference to an element enclosed in '{' and '}' braces is the element's
 *		LRC index.
 *
 *		In each state, each processor enumerates changes in the mesh /relative/ to the last known dump of the
 *		mesh to the database. Two broad categories of changes are tracked; refinements and re-balances. Refinements
 *		are tracked and enumerated in the /global/ context. Re-balancing is tracked and enumerated in the /local/
 *		context. In other words, refinement information is stored /above/ the processor decomposition while
 *		re-balancing information is stored /below/ it in the Subset Relation Graph (SRG). This was completely
 *		arbitrary and can be changed if desired. Given all the elements on a processor at a given state, each
 *		processor stores information to answer: "Which of my elements in the current state..."
 *
 *		   a. ...did I get from another processor in the last known state. 
 *		   b. ...are children of (refinements of) an element in the last known state. 
 *
 *		In addition, we arbitrarily choose to store information on UNrefinements and no-changes as well as elements
 *		that are kept (as opposed to re-balanced). There is no particular reason to do this other than trying
 *		to make the example a little more interesting. It costs more data that is non-essential.
 *
 *		In some cases, between two state dumps an element may be refined *and* some or all of its children may be
 *		re-balanced to other processors. When this happens, from the point of view of the database receiving 
 *		information about each state, the convention used is that the processor in the previous state that owned
 *		the parent made the decision to refine it *and* then re-balanced the elements to other processors. For
 *		example, between state 0 and state 1 in "use case 5-1.gif", element {1,0,1} which lived on processor 0 in state
 *		0 was refined *and* half (2) of its children were given to processor 1. Thus, in state 1, processor 1
 *		treats these two elements as being *both* refinements *and* re-balances.
 *
 *		Next, for re-balances, a field is stored on the re-balances set which identifies the processor from which
 *		each element in the set came.
 *
 *		[Side note: Why not store this information as a set of subsets? That too, is
 *		completely appropriate. The approach chosen here is merely more convenient and storage efficient.
 *		The fact is, there is a duality in how certain kinds of information can be captured in SAF. This duality is a
 *		fundamental aspect of the mathematical interpreation of a field defined on member(s) of a Set Relation Graph
 *		(SRG). In short, if one wishes to enumerate a value for each element in a collection, one has a choice of
 *		saying (in natural language), "for each element, which value does it have..." or "for each value, which
 *		elements have that value...". The former approach takes the form of a field while the latter approach takes
 *		the form of a set of subsets (a partition in fact). In fact, there is document that discusses these issues
 *		in detail available from the SAF web-pages at http://www.ca.sandia.gov/ASCI/sdm/SAF/docs/wips/fields_n_maps.ps.gz
 *		In summary, while one may have a natural way of /thinking/ about this kind of data, there is clear
 *		mathematical theory to explain why either approach is appropriate *and* there are even theoretical storage 
 *		and performance reasons to prefer one over the other depending on the situation. -- end side note]
 *
 *		Since developing this initial use-case, a couple of enhancemenets have been identified that would be
 *		make the use-case more realistic and facilitate certain kinds of queries. First, we've identified a way
 *		to use SAF to capture differences in the sets from one state to the next as opposed or in addition to
 *		each specific state. Second, we've identified a way to make forward references (as apposed or in addition to
 *		to backward) to facilitate forward tracking of changes in refinement and rebalancing.
 *
 * Issues: 	The ability to talk about the "difference" between two SRGs would be useful. If one is permitted
 *		only to enumerate a given state of the client's data, it is difficult to store information at state I
 *		that captures what is changed in going to state I+1. For example, in going from state 0 to state 1,
 *		element {1,0,1} is refined into 4 children. However, the output for state 0 can't mention any of these
 *		children because when state 0 is created, they don't exist. Because they do exist in state 1, we can
 *		talk about where they came from relative to state 0. Thus, a causality is imposed, which the current
 *		implementation demonstrates, in the direction in which we can talk about changes (as mentioned above,
 *		I think we have identified as solution to this).
 *
 *		If one wishes to capture the differences between states, where does that information "live"? The
 *		differences represent what happened in making the transition from one state to the next. In some sense
 *		the differences represent actions on objects and not objects themselves. For example, "...these elments were
 *		added by refinement of that element..." or "...these elements were obtained by rebalancing from that
 *		processor..." are the kinds of statements one might like to make. It would be nice of such differences could
 *		be captured using the existing objects available in SAF rather than having to create new ones. I think I have
 *		identifed a way of doing this. Given two states, 'a' and 'b', and two sets, S and P where S is the subset of P
 *		in both states, we can talk about the difference of Sa and Sb (that is S in state a and S in state b) in P
 *		by introducing two subsets of S, one in state a, called Dab and one in state b called Dba where
 *
 *		   Dab = Sa - Sb (all points in Sa but not in Sb)
 *		   Dba = Sb - Sa (all points in Sb but not in Sa)
 *
 *		Together, these two sets represent, in effect, additions and deletions of points in going from Sa to Sb
 *		or vice versa. Dab is the set of points deleated from Sa in arriving at Sb and Dba is the set of points
 *		added to Sa in arriving at Sb (or deleted from Sb to arrive at Sa in the reverse direction).
 *
 *		Both Dab and Dba are ordinary subsets in their respective SRGs. However, what we are missing from SAF is
 *		the ability to /declare/ that Dab is a difference subset and which set it is differenced with. There
 *		are two possible routes to take here. One is to simply add a SAF_DIFFERENCE() option to the
 *		saf_declare_subset_relation() call so that some subsets can be defined that are differences with other
 *		sets. The other route is to add a new function to declare expressions involving sets such as...
 *		saf_declare_set_expression(SAF_Set resultSet, char *expr) along with functions to build up the string
 *		representation for the expression. This would then permit a client to find sets in the SRG according
 *		to a given expression (implementation details would require something like an expr_blob_id member of a set
 *		object in VBT which could be implemented as a meta_blob). The latter approach is more general in that
 *		it permits a variety of set expressions to be characterized, not just a difference.
 *
 *		Because SAF is targeted primarily as a data modeling and I/O library, it is typically used to output
 *		restart or plot dumps for states that are /far/ apart relative to the physics time-step. For example,
 *		there may be many hundreds of time-steps from one state dump to the next. Consequently, the relationships
 *		that can be captured in such a scenario are how the two states *as*told*to*the*I/O*system* are related.
 *		For example, if a state is dumped at time I where an element, say K, is on processor 0 and then this
 *		element migrates from processor 0, to 1, to 5, to 17 and finally to 22 before a new state is dumped to
 *		the I/O system, the only fact that the I/O system can capture is that, somehow, element K on processor 0
 *		was given to processor 22. In order to capture the in-between information, each of those states must be
 *		enumerated to the I/O system. This might be where having the ability to enumerate state-transitions as
 *		opposed to just states would be useful. Then, it may be relatively simple to enumerate each of the states
 *		the code went through.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */


/* we enclose all code here in a single #ifdef HAVE_PARALLEL block so we don't have to individually do it in
   a number of places */
#ifdef HAVE_PARALLEL


#define MAX_FILENAME    256
#define MAX_OBJNAME     32



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	H-Adaptive Use Case 
 * Purpose:	Struct for current mesh params	
 *
 * Description:	This structure houses all the parameters for each current step of the mesh. It is divided into two
 *		kinds of information; global information about the whole mesh that is the same on all processors and
 *		local information that is different on each processor.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct _currentMeshParams {

   /* global mesh parameters, same on all processors */
   int 		numElems;	/* total (global) number of elements */
   int 		numNodes;	/* total (global) number of nodes */
   int		*refList;	/* list of cells refined from prev. step */
   int		*parentList;	/* for each refined cell, which cell in previous step is its parent */
   int		*unrefList;	/* list of cells unrefined from prev. step */
   int 		*sameList;	/* list of cells unchanged from prev. step */
   int		numRefs;	/* number of cells refined */
   int		numUnrefs;	/* number of cells unrefined */
   int 		numSame;	/* number of cells unchanged */

   /* local mesh parameters, unique to each processor */
   int 		numElemsIown;	/* number of elements this processor owns (length of `elemList') */
   LRCIndex_t	*elemLRCs;	/* the global LRC element ID's of the elements owned by this processor */ 
   int		*elemList;	/* the global element ID's of the elements owned by this processor */ 

   int		numNodesIown;	/* number of nodes this processor owns (length of `uniqueNodesList') */
   int		*nodeList;	/* the global node ID's of the nodes owned by this processor */ 

   float	*coordBuf;	/* coordinate data (2 * numNodes floats) for nodes on this processor */
   int		*topoBuf;	/* topology buffer */

   int		numTakes;
   int		*takeList;	/* elements I've received from other procs due to re-balancing from last dump */	
   int		*takeProc;	/* processor I took the element from */

   int		numKeeps;
   int		*keepList;	/* elements I've kept from last dump */

} CurrentMeshParams_t;

/*
 * Prototypes
 */
int CompareLRCValues(void *argA, void *argB );
int CompareCoords(void *argA, void *argB );
void InitMeshParams( CurrentMeshParams_t *p );
void UpdateMesh( int stepNum, int myRank, CurrentMeshParams_t *p );
void OpenDatabase( char *dbname, hbool_t do_multifile, DbInfo_t *dbInfo );
void WriteCurrentMesh( DbInfo_t *dbInfo, int theStep, int numProcs, int myProcNum,
		       CurrentMeshParams_t theMesh, SAF_Field *fieldList, int *fieldListSize  );
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Examples
 * Purpose:     Build hdf5 datatypes
 *
 * Description: Builds HDF5 datatypes corresponding to LRCIndex_t.
 *
 * Parallel:    Independent
 *
 *-------------------------------------------------------------------------------------------------------------------------------
*/
static void
build_h5_types(void)
{
    hid_t       str1 = H5Tcopy(H5T_C_S1);
                                                                                
    H5Tset_size(str1, 1);
                                                                                
    /* LRCIndex_t */
    LRCIndex_h5 = H5Tcreate(H5T_COMPOUND, sizeof(LRCIndex_t));
    H5Tinsert(LRCIndex_h5, "l", HOFFSET(LRCIndex_t, l), str1);
    H5Tinsert(LRCIndex_h5, "r", HOFFSET(LRCIndex_t, r), str1);
    H5Tinsert(LRCIndex_h5, "c", HOFFSET(LRCIndex_t, c), str1);
    H5Tlock(LRCIndex_h5);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	H-Adaptive Use Case 
 * Purpose:	Comare L,R,C Values	
 *
 * Description:	This function is used for comparing L,R,C values. It returns -1 if value A is less than value B,
 *		0 if value A is equal to value B and +1 if value A is greater than value B.
 *
 * Programmer:	Mark Miller, LLNL, 29May02
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
CompareLRCValues(
   void *argA,	/* [IN] The /first/ value */
   void *argB	/* [IN] The /second/ value */
)
{
   LRCIndex_t *valA = (LRCIndex_t *) argA;
   LRCIndex_t *valB = (LRCIndex_t *) argB;

   if (valA->l < valB->l)
      return 1;
   else if (valA->l > valB->l)
      return -1;
   else
   {
      if (valA->r < valB->r)
         return 1;
      else if (valA->r > valB->r)
         return -1;
      else
      {
         if (valA->c < valB->c)
	    return 1;
	 else if (valA->c > valB->c)
	    return -1;
	 else
	    return 0;
      }
   }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	H-Adaptive Use Case 
 * Purpose:	Comare Coordinates
 *
 * Description:	This function is used for comparing coordinates.
 *
 * Programmer:	Mark Miller, LLNL, 30May02
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
CompareCoords(
   void *argA,	/* [IN] The /first/ value */
   void *argB	/* [IN] The /second/ value */
)
{
   float Ax = *((float*) argA);
   float Ay = *((float*) argA+1);
   float Bx = *((float*) argB);
   float By = *((float*) argB+1);

   if (Ax < Bx)
      return 1;
   else if (Ax > Bx)
      return -1;
   else
   {
      if (Ay < By)
         return 1;
      else if (Ay > By)
         return -1;
      else return 0;
   }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	H-Adaptive Use Case 
 * Purpose:	Initialize mesh parameters
 *
 * Description:	This function initializes the mesh parameters (size and geometric boundaries)
 *
 * Programmer:	Mark Miller, LLNL, 29May02 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
InitMeshParams(
   CurrentMeshParams_t *p	/* [IN/OUT] mesh parameter structure to initialize */
)
{

   p->numElems = 0;
   p->numNodes = 0;
   p->refList = NULL;
   p->unrefList = NULL;
   p->sameList = NULL;
   p->numRefs = 0;
   p->numUnrefs = 0;
   p->numSame = 0; 
   p->numElemsIown = 0;
   p->elemLRCs = NULL;
   p->elemList = NULL;
   p->numNodesIown = 0;
   p->nodeList = NULL;
   p->coordBuf = NULL;
   p->topoBuf = NULL;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	H-Adaptive Use Case 
 * Purpose:	Initialize mesh parameters
 *
 * Description:	This function sets the mesh parameters on each step. The steps are hard-coded. In each step, and for each
 *		processor, the elements assigned are hard-coded. The hard-coded assignment is done locally. From this starting
 *		point, the remaining code does work to derive other information via computation.
 *
 * Programmer:	Mark Miller, LLNL, 29May02 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
UpdateMesh(
   int stepNum,			/* [IN] dump step */
   int myRank,			/* [IN] rank of calling processor */
   CurrentMeshParams_t *p	/* [IN/OUT] mesh parameter structure to update */
)
{
   int i;
   float *nodeCoords;
   hbool_t badData = false;

   /* hard-coded assignment of elements to processors based on step number and rank of calling processor */
   switch (stepNum)
   {

      /* ************************************************************************************************************** */
      /* ******************************************** STEP's 0 and 5 ************************************************** */
      /* ************************************************************************************************************** */
      case 0:
      case 5:
      {
	 /* global values */
	 p->numNodes = 21;
	 p->numElems = 12;

	 if (stepNum == 0)
	 {
	    static int sameList[] = {0,1,2,3,4,5,6,7,8,9,10,11};
	    static int *takeList = NULL;
	    static int *takeProc = NULL;
	    static int keepList[] = {0,1,2,3}; /* it turns out takes/keeps is same for all procs on step 0 */

	    p->numTakes = 0; 
	    p->numKeeps = sizeof(keepList) / sizeof(int);
	    p->keepList = keepList;
	    p->takeList = takeList;
	    p->takeProc = takeProc;
	    p->refList = NULL;
	    p->unrefList = NULL; 
	    p->sameList = sameList;
	    p->parentList = NULL;
	    p->numRefs = 0;
	    p->numUnrefs = 0; 
	    p->numSame = 12;
	 }
	 else
	 {
	    static int unrefList[] = {1,4,5};
	    static int sameList[] = {0,1,3,6,7,8,9,10,11};

	    p->refList = NULL;
	    p->unrefList = unrefList; 
	    p->sameList = sameList;
	    p->parentList = NULL;
	    p->numRefs = 0; 
	    p->numUnrefs =  sizeof(unrefList) / sizeof(int);
	    p->numSame = sizeof(sameList) / sizeof(int);
	 }

         switch (myRank)
	 {
	    case 0: /* processor 0 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,0},{1,0,1},{1,1,0},{1,1,1}};
	       static int myElemList[] = {0, 1, 10, 11};
	       static int myNodeList[] = {0,1,2,7,8,9,14,15,16};
	       static int myTopoBuf[] = {0,1,4,3,  1,2,5,4,  3,4,7,6,  4,5,8,7};
	       static int *takeList = NULL;
	       static int *takeProc = NULL;
	       static int keepList[] = {0,2,3};

	       if (stepNum == 5)
	       {
	          p->numTakes = 0; 
	          p->numKeeps = sizeof(keepList) / sizeof(int);
	          p->keepList = keepList;
	          p->takeList = takeList;
		  p->takeProc = takeProc;
	       }

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 1: /* processor 1 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,2},{1,0,3},{1,1,2},{1,1,3}};
	       static int myElemList[] = {2,3,8,9};
	       static int myNodeList[] = {2,3,4,9,10,11,16,17,18};
	       static int myTopoBuf[] = {0,1,4,3,  1,2,5,4,  3,4,7,6,  4,5,8,7};
	       static int takeList[] = {1,2,3};
	       static int takeProc[] = {2,0,2};
	       static int *keepList = NULL;

	       if (stepNum == 5)
	       {
	          p->numTakes = sizeof(takeList) / sizeof(int); 
	          p->numKeeps = 0; 
	          p->keepList = keepList;
	          p->takeList = takeList;
		  p->takeProc = takeProc;
	       }

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 2: /* processor 2 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,4},{1,0,5},{1,1,4},{1,1,5}};
	       static int myElemList[] = {4,5,10,11};
	       static int myNodeList[] = {4,5,6,11,12,13,18,19,20};
	       static int myTopoBuf[] = {0,1,4,3,  1,2,5,4,  3,4,7,6,  4,5,8,7};
	       static int *takeList = NULL;
	       static int *takeProc = NULL;
	       static int keepList[] = {1,2,3,4};

	       if (stepNum == 5)
	       {
	          p->numTakes = 0;
	          p->numKeeps = sizeof(keepList) / sizeof(int); 
	          p->keepList = keepList;
	          p->takeList = takeList;
		  p->takeProc = takeProc;
	       }


               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	 }
      }
      break;

      /* ************************************************************************************************************** */
      /* ************************************************** STEP 1 **************************************************** */
      /* ************************************************************************************************************** */
      case 1:
      {
	 static int refList[]    = {1,2,5,7};
	 static int parentList[] = {1,1,1,1};
	 static int sameList[] = {0,3,4,6,8,9,10,11,12,13,14};

         /* global values */
	 p->numNodes = 26;
	 p->numElems = 15;
	 p->refList = refList;
	 p->parentList = parentList;
	 p->unrefList = NULL; 
	 p->sameList = sameList;
	 p->numRefs = sizeof(refList) / sizeof(int);
	 p->numUnrefs = 0; 
	 p->numSame = sizeof(sameList) / sizeof(int);

         switch (myRank)
	 {
	    case 0: /* processor 0 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,0},{2,0,2},{2,1,2},{1,1,0},{1,1,1}};
	       static int myElemList[] = {0,1,7,9,10};
	       static int myNodeList[] = {0,1,2,8,9,11,12,13,14,19,20,21};
	       static int myTopoBuf[] = {0,1,6,5,  1,2,4,3,  3,4,7,6,  5,6,10,9,  6,8,11,10};
	       static int *takeList = NULL;
	       static int *takeProc = NULL;
	       static int keepList[] = {0,3,4};

	       p->numTakes = 0; 
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 1: /* processor 1 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{2,0,3},{2,1,3},{1,0,2},{1,1,2},{1,1,3}};
	       static int myElemList[] = {2,3,8,11,12}; 
	       static int myNodeList[] = {2,3,4,9,10,13,14,15,16,21,22,23};
	       static int myTopoBuf[] = {0,1,4,3,  1,2,7,6,  3,4,6,5,  6,7,10,9,  7,8,11,10};
	       static int takeList[] = {0,2};
	       static int takeProc[] = {0,0};
	       static int keepList[] = {2,3,4};

	       p->numTakes = sizeof(takeList) / sizeof(int); 
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 2: /* processor 2 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,3},{1,0,4},{1,0,5},{1,1,4},{1,1,5}};
	       static int myElemList[] = {4,5,6,13,14};
	       static int myNodeList[] = {4,5,6,7,15,16,17,18,23,24,25};
	       static int myTopoBuf[] = {0,1,5,4,  1,2,6,5,  2,3,7,6,  5,6,9,8,  6,7,10,9};
	       static int takeList[] = {0};
	       static int takeProc[] = {1};
	       static int keepList[] = {1,2,3,4};

	       p->numTakes = sizeof(takeList) / sizeof(int); 
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	 }
      }
      break;

      /* ************************************************************************************************************** */
      /* ************************************************** STEP 2 **************************************************** */
      /* ************************************************************************************************************** */
      case 2:
      {
	 static int refList[]    = {2,5,6,7};
	 static int parentList[] = {6,6,6,6};
	 static int sameList[] = {0,3,4,8,9,10,11,12,13,14};
	 static int unrefList[] = {1};

	 /* global values */
	 p->numNodes = 26;
	 p->numElems = 15;
	 p->refList = refList;
	 p->parentList = parentList;
	 p->unrefList = unrefList;
	 p->sameList = sameList;
	 p->numRefs = sizeof(refList) / sizeof(int);
	 p->numUnrefs = sizeof(unrefList) / sizeof(int); 
	 p->numSame = sizeof(sameList) / sizeof(int);

         switch (myRank)
	 {
	    case 0: /* processor 0 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,0},{1,0,1},{2,0,4},{1,1,0},{1,1,1}};
	       static int myElemList[] = {0,1,2,9,10};
	       static int myNodeList[] = {0,1,2,3,8,9,11,12,13,19,20,21};
	       static int myTopoBuf[] = {0,1,7,6,  1,2,8,7,  2,3,5,4,  6,7,10,9,  7,8,11,10};
	       static int takeList[] = {1};
	       static int takeProc[] = {1};
	       static int keepList[] = {0,2,3};

	       p->numTakes = sizeof(takeList) / sizeof(int); 
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 1: /* processor 1 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{2,0,5},{2,1,4},{2,1,5},{1,1,2},{1,1,3}};
	       static int myElemList[] = {3,7,8,11,12};
	       static int myNodeList[] = {3,4,8,9,10,13,14,15,16,21,22,23};
	       static int myTopoBuf[] = {0,1,4,3,  2,3,6,5,  3,4,7,6,  5,7,10,9,  7,8,11,10};
	       static int *takeList = NULL;
	       static int *takeProc = NULL;
	       static int keepList[] = {3,4};

	       p->numTakes = 0; 
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 2: /* processor 2 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,3},{1,0,4},{1,0,5},{1,1,4},{1,1,5}};
	       static int myElemList[] = {4,5,6,13,14};
	       static int myNodeList[] = {4,5,6,7,10,15,16,17,18,23,24,25};
	       static int myTopoBuf[] = {0,1,6,5,  1,2,7,6,  2,3,8,7,  6,7,10,9,  7,8,11,10};
	       static int *takeList = NULL;
	       static int *takeProc = NULL;
	       static int keepList[] = {0,1,2,3,4};

	       p->numTakes = 0; 
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	 }
      }
      break;

      /* ************************************************************************************************************** */
      /* ************************************************** STEP 3 **************************************************** */
      /* ************************************************************************************************************** */
      case 3:
      {
	 static int refList[] =    {6,6,7,8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27};
	 static int parentList[] = {7,7,7,7,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
	 static int sameList[] = {0,1,2,3,4,28,29,30,31,32,33,34,35};
	 static int *unrefList = NULL; 

	 /* global values */
	 p->numNodes = 55;
	 p->numElems = 36;
	 p->refList = refList;
	 p->unrefList = unrefList;
	 p->parentList = parentList;
	 p->sameList = sameList;
	 p->numRefs = sizeof(refList) / sizeof(int); 
	 p->numUnrefs = 0;
	 p->numSame = sizeof(sameList) / sizeof(int);

         switch (myRank)
	 {
	    case 0: /* processor 0 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,0},{1,0,1},{2,0,4},{2,0,5},{2,1,4},{3,2,10},{3,2,11},{3,3,10},{3,3,11}};
	       static int myElemList[] = {0,1,2,3,14,15,16,24,25};
	       static int myNodeList[] = {0,1,2,3,4,16,17,18,19,28,29,30,36,37,38,39,40,41};
	       static int myTopoBuf[] = {0,1,13,12,  1,2,14,13,  2,3,6,5,  3,4,8,6,  5,6,15,14,  6,7,10,9,  7,8,11,10,
					 9,10,16,15,  10,11,17,16};
	       static int takeList[] = {3,4,5,6,7,8};
	       static int takeProc[] = {1,1,1,1,1,1};
	       static int keepList[] = {0,1};

	       p->numTakes = sizeof(takeList) / sizeof(int); 
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 1: /* processor 1 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{3,0,12},{3,0,13},{3,0,14},{3,0,15},{3,1,12},{3,1,13},{3,1,14},{3,1,15},
						 {4,4,24},{4,4,25},{3,2,13},{3,2,14},{3,2,15},{4,5,24},{4,5,25},{3,3,13},
						 {3,3,14},{3,3,15},{3,3,16}};
	       static int myElemList[] = {4,5,6,7,10,11,12,13,17,18,19,20,21,22,23,26,27,28,29};
	       static int myNodeList[] = {4,5,6,7,8,11,12,13,14,15,19,20,21,22,23,24,25,26,27,30,31,32,33,34,35,41,42,43,44,45};
	       static int myTopoBuf[] = {0,1,6,5,  1,2,7,6,  2,3,8,7,  3,4,9,8,  5,6,12,10,  6,7,13,12,  7,8,14,13,  8,9,15,14,
					 10,11,17,16,  11,12,18,17,  12,13,22,21,  13,14,23,22,  14,15,24,22,  16,17,20,19,
					 17,18,21,20,  19,21,26,25,  21,22,27,26,  22,23,28,27,  23,24,29,28};
	       static int takeList[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};
	       static int takeProc[] = {2,2,2,2,2,2,2,2,2,2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
	       static int *keepList = NULL;

	       p->numTakes = sizeof(takeList) / sizeof(int); 
	       p->numKeeps = 0;
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 2: /* processor 2 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,4},{1,0,5},{1,1,0},{1,1,1},{1,1,2},{1,1,3},{1,1,4},{1,1,5}};
	       static int myElemList[] = {8,9,30,31,32,33,34,35};
	       static int myNodeList[] = {8,9,10,15,24,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54};
	       static int myTopoBuf[] = {0,1,16,15,  1,2,17,16,  6,7,19,18,  7,8,20,19,  8,11,21,20,  11,15,22,21,
					 15,16,23,22,  16,17,24,23};
	       static int takeList[] = {2,3,4,5};
	       /* static int takeProc[] = {0,0,1,1}; # unused */
	       static int keepList[] = {0,1,6,7};

	       p->numTakes = sizeof(takeList) / sizeof(int);
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	 }
      }
      break;

      /* ************************************************************************************************************** */
      /* ************************************************** STEP 4 **************************************************** */
      /* ************************************************************************************************************** */
      case 4:
      {
	 static int refList[] =    {1,2,3,4,5,6,7,8,9,10,14,15,16,17,18,20,21,22,23,24,25};
	 static int parentList[] = {1,1,1,1,1,1,1,1,1,1 ,2 ,2 ,2 ,3 ,3 ,2 ,2 ,2 ,2 ,3 ,3};
	 static int sameList[] = {0,11,12,13,26,30,31,33,34,35};
	 static int unrefList[] = {19,27,28,29,32};

	 /* global values */
	 p->numNodes = 56;
	 p->numElems = 36;
	 p->refList = refList;
	 p->parentList = parentList;
	 p->unrefList = unrefList;
	 p->sameList = sameList;
	 p->numRefs = sizeof(refList) / sizeof(int);
	 p->numUnrefs = sizeof(unrefList) / sizeof(int); 
	 p->numSame = sizeof(sameList) / sizeof(int);

         switch (myRank)
	 {
	    case 0: /* processor 0 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{1,0,0},{2,0,2},{3,0,6},{4,0,14},{4,0,15},{4,1,14},{4,1,15},{3,1,6},{3,1,7},
						 {2,1,2},{2,1,3},{1,1,0},{1,1,1},{1,1,2}};
	       static int myElemList[] = {0,1,2,3,4,14,15,18,19,24,25,30,31,32};
	       static int myNodeList[] = {0,1,2,3,4,5,15,16,17,20,21,22,23,29,30,31,32,39,40,41,42,43,44,49,50,51,52};
	       static int myTopoBuf[] = {0,1,18,17,  1,2,14,13,  2,3,10,9,  3,4,7,6,  4,5,8,7,  6,7,11,10,  7,8,12,11,
					 9,10,15,14,  10,12,16,15,  13,14,19,18,  14,16,20,19,  17,18,24,23,  18,20,25,24,
					 20,22,26,25};
	       static int takeList[] = {11,12,13};
	       static int takeProc[] = { 2, 2, 2};
	       static int keepList[] = {0};

	       p->numTakes = sizeof(takeList) / sizeof(int);
	       p->numKeeps = sizeof(keepList) / sizeof(int);
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 1: /* processor 1 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{4,0,16},{4,0,17},{3,0,9},{3,0,10},{3,0,11},{2,0,7},{4,1,16},{4,1,17},
						 {3,1,8},{3,1,9},{3,1,10},{3,1,11},{2,1,4},{2,1,5},{2,1,6}};
	       static int myElemList[] = {5,6,7,8,9,10,16,17,20,21,22,23,26,27,28};
	       static int myNodeList[] = {5,6,7,8,9,10,11,17,18,19,23,24,25,26,27,28,32,33,34,35,36,37,42,43,44,45};
	       static int myTopoBuf[] = {0,1,8,7,  1,2,9,8,  2,3,13,12,  3,4,14,13,  4,5,15,14,  5,6,21,20,  7,8,11,10,  
					 8,9,12,11,  10,12,17,16, 12,13,18,17,  13,14,19,18,  14,15,20,19,  16,18,23,22,
					 18,20,24,23,  20,21,25,24};
	       static int takeList[] = {0,1,2,3,4,6,7,8,9,10,11,12,13};
	       static int takeProc[] = {0,0,0,0,0,0,0,0,0, 0, 0, 0, 0};
	       static int *keepList = NULL;

	       p->numTakes = sizeof(takeList) / sizeof(int);
	       p->numKeeps = 0;
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	    case 2: /* processor 2 */
	    {
	       static LRCIndex_t myElemLRCs[] = {{2,0,7},{1,0,4},{1,0,5},{2,1,7},{1,1,3},{1,1,4},{1,1,5}};
	       static int myElemList[] = {11,12,13,29,33,34,35};
	       static int myNodeList[] = {11,12,13,14,37,38,44,45,46,47,48,52,53,54,55};
	       static int myTopoBuf[] = {0,1,5,4,  1,2,9,8,  2,3,10,9,  4,5,8,7,  6,8,12,11,  8,9,13,12,  9,10,14,13};
	       static int takeList[] = {0,3};
	       static int takeProc[] = {1,1};
	       static int keepList[] = {1,2,4,5,6};

	       p->numTakes = sizeof(takeList) / sizeof(int);
	       p->numKeeps = sizeof(keepList) / sizeof(int); 
	       p->keepList = keepList;
	       p->takeList = takeList;
	       p->takeProc = takeProc;

               p->numElemsIown = sizeof(myElemList) / sizeof(int);
	       p->numNodesIown = sizeof(myNodeList) / sizeof(int);
	       if (sizeof(myTopoBuf) != 4*sizeof(myElemList))
	       {
		  printf("sizes of topoBuf(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myTopoBuf), sizeof(myElemList), myRank);
		  badData = true;
               }
	       if (sizeof(myElemLRCs)/sizeof(LRCIndex_t) != sizeof(myElemList) / sizeof(int))
	       {
		  printf("sizes of myElemLRCs(%u) and elemList(%u) don't agree on processor %d\n",
		     sizeof(myElemLRCs)/sizeof(LRCIndex_t), sizeof(myElemList)/sizeof(int), myRank);
		  badData = true;
               }
	       p->elemLRCs = myElemLRCs;
	       p->elemList = myElemList;
	       p->nodeList = myNodeList;
	       p->topoBuf  = myTopoBuf;
	    }
	    break;
	 }
      }
      break;

      default:
      {
         printf("Invalid step number\n");
	 MPI_Abort(MPI_COMM_WORLD,1);
	 exit(1);
      }
   }

   /* validate the hand-entered data */
   if (p->numRefs + p->numUnrefs + p->numSame != p->numElems)
   {
      printf("refs(%d) + unrefs(%d) + same(%d) != numElems(%d)\n",
         p->numRefs, p->numUnrefs, p->numSame, p->numElems);
      MPI_Abort(MPI_COMM_WORLD,1);
      exit(1);
   }

   /* validate element numbers */
   for (i = 0; i < p->numElemsIown; i++)
   {
      if (p->elemList[i] < 0 || p->elemList[i] >= p->numElems)
      {
	 printf("invalid element number %d (local = %d) on processor %d\n",
	    p->elemList[i], i, myRank);
	 badData = true;
      }
   }
   if (badData)
   {
      MPI_Abort(MPI_COMM_WORLD,1);
      exit(1);
   }

   /* validate node numbers */
   for (i = 0; i < p->numNodesIown; i++)
   {
      if (p->nodeList[i] < 0 || p->nodeList[i] >= p->numNodes)
      {
	 printf("invalid node number %d (local = %d) on processor %d\n",
	    p->nodeList[i], i, myRank);
	 badData = true;
      }
   }
   if (badData)
   {
      MPI_Abort(MPI_COMM_WORLD,1);
      exit(1);
   }

   /* validate local topology relation references */
   for (i = 0; i < p->numElemsIown * 4; i++)
   {
      if (p->topoBuf[i] < 0 || p->topoBuf[i] >= p->numNodesIown)
      {
	 printf("invalid topo node number %d (local = %d) on processor %d\n",
	    p->topoBuf[i], i, myRank);
	 badData = true;
      }
   }
   if (badData)
   {
      MPI_Abort(MPI_COMM_WORLD,1);
      exit(1);
   }

   /* build the nodal coordinate field dofs */
   nodeCoords = (float *) malloc(p->numNodesIown * 2 * sizeof(float));
   for (i = 0; i < p->numElemsIown; i++)
   {
      int l = p->elemLRCs[i].l, r = p->elemLRCs[i].r, c = p->elemLRCs[i].c;
      int k;
      float elemSize = 2.0 / (1<<l);

      k = p->topoBuf[4*i+0];
      nodeCoords[k+0] = (c+0) * elemSize;
      nodeCoords[k+1] = (r+0) * elemSize;

      k = p->topoBuf[4*i+1];
      nodeCoords[k+0] = (c+1) * elemSize;
      nodeCoords[k+1] = (r+0) * elemSize;

      k = p->topoBuf[4*i+2];
      nodeCoords[k+0] = (c+1) * elemSize;
      nodeCoords[k+1] = (r+1) * elemSize;

      k = p->topoBuf[4*i+3];
      nodeCoords[k+0] = (c+0) * elemSize;
      nodeCoords[k+1] = (r+1) * elemSize;
   }
   if (p->coordBuf)
      free(p->coordBuf);
   p->coordBuf = nodeCoords;

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public	
 * Chapter:	H-Adaptive Use Case 
 * Purpose:	Open a new database and do some prepratory work on it
 *
 * Description:	This function creates the initial database and some key objects such as the top-most aggregate set,
 *		the nodes, elems and procs collection categories and the state suite.
 *
 * Programmer:	Mark Miller, LLNL, 29May02 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
OpenDatabase(
   char *dbname,		/* [IN] name of the database */
   hbool_t do_multifile,	/* [IN] boolean to indicate if each step will go to a different supplemental file */
   DbInfo_t *dbInfo		/* [OUT] database info object */
)
{
   SAF_DbProps *dbprops=NULL;
   SAF_Db *db=NULL;
   SAF_Cat nodes, elems, procs;
   SAF_Set invariantMesh;
   SAF_Db *theFile; 

   /* create the database */
   dbprops = saf_createProps_database();
   saf_setProps_Clobber(dbprops);
   db = saf_open_database(dbname,dbprops);
   dbInfo->db = db;

   /* declare nodes, elems and blocks categories */
   saf_declare_category(SAF_ALL, db,  "nodes", SAF_TOPOLOGY, 0, &nodes);
   dbInfo->nodes = nodes;
   saf_declare_category(SAF_ALL, db,  "elems", SAF_TOPOLOGY, 2, &elems);
   dbInfo->elems = elems;
   saf_declare_category(SAF_ALL, db,  "procs", SAF_PROCESSOR, 2, &procs);
   dbInfo->blocks = procs;

   /* create the invariant set */
   saf_declare_set(SAF_ALL, db, "whole", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &invariantMesh);
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
 * Chapter:	H-Adaptive Use Case 
 * Purpose:	Write current mesh to the SAF database	
 *
 * Description:	This function does all the work of writing the current mesh, including its domain-decomposed topology relation,
 *		processor subset relations, coordinate field, and pressure field to the SAF database.
 *
 * Programmer:	Mark Miller, LLNL, 29May02 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
WriteCurrentMesh(
   DbInfo_t *dbInfo,		/* [IN/OUT] database info object */
   int theStep,			/* [IN] current step number */
   int numProcs,		/* [IN] number of processors */
   int myProcNum,		/* [IN] the rank of calling processor */ 
   CurrentMeshParams_t theMesh,	/* [IN] current mesh parameters */
   SAF_Field *fieldList,        /* [IN/OUT] list of fields we'll append newly written fields too */
   int *fieldListSize           /* [IN/OUT] On input, the current size of the field list. On output, its new size */
)
{
   char tmpName[MAX_OBJNAME];
   int i;
   LRCIndex_t *elemLRCs = theMesh.elemLRCs;
   int *topoBuf = theMesh.topoBuf;
   float *coordBuf = theMesh.coordBuf;
   int *elemList = theMesh.elemList;
   int *nodeList = theMesh.nodeList;
   int numElems = theMesh.numElems;
   int numNodes = theMesh.numNodes;
   int numRefs = theMesh.numRefs;
   int *refList = theMesh.refList;
   int *parentList = theMesh.parentList;
   int numUnrefs = theMesh.numUnrefs;
   int *unrefList = theMesh.unrefList;
   int numSame = theMesh.numSame;
   int *sameList = theMesh.sameList;
   int numKeeps = theMesh.numKeeps;
   int *keepList = theMesh.keepList;
   int numTakes = theMesh.numTakes;
   int *takeList = theMesh.takeList;
   int *takeProc = theMesh.takeProc;
   int numElemsIown = theMesh.numElemsIown;
   int numNodesIown = theMesh.numNodesIown;
   int nodesPerElem = 4; 
   int numHandles;
   int theCount;
   SAF_Set currentMesh, myProcSet;
   SAF_FieldTmpl coords_ctmpl, coords_ftmpl, mesh_coord_ftmpl;
   SAF_Field coordField, coordComponent[MAX_DIMS], *fields=NULL, coords;
   SAF_Rel rel, trel, *rels = NULL;
   SAF_AltIndexSpec aspec;

   SAF_Db *db = dbInfo->db;
   SAF_Unit umeter;
   SAF_Cat nodes = dbInfo->nodes;
   SAF_Cat elems = dbInfo->elems;
   SAF_Cat procs = dbInfo->blocks;
   SAF_Set lastMesh = dbInfo->lastMesh;
   SAF_Set myKeeps, myTakes;
   SAF_Db *stepFile = dbInfo->currentFile;

   saf_find_one_unit(db, "meter", &umeter);

   /******************************************************
    ******* this step's global mesh (base-space) *********
    ******************************************************/
   sprintf(tmpName, "mesh_step_%03d", theStep);
   saf_declare_set(SAF_ALL, db, tmpName, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &currentMesh);
   dbInfo->currentMesh = currentMesh;

   /************************************************************************
    ******* collections on the global mesh (nodes, elems and procs *********
    ************************************************************************/
   /* nodes and elems collections on the current mesh */
   saf_declare_collection(SAF_ALL, &currentMesh, &nodes, SAF_CELLTYPE_POINT, numNodes, SAF_1DC(numNodes), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &currentMesh, &elems, SAF_CELLTYPE_QUAD, numElems, SAF_1DC(numElems), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &currentMesh, &procs, SAF_CELLTYPE_SET, numProcs, SAF_1DC(numProcs), SAF_DECOMP_TRUE);

   /****************************************
    ******* refinement information *********
    ****************************************/
   if (numRefs)
   {
      SAF_Set theSet;

      saf_declare_set(SAF_ALL, db, "refinements", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &theSet);
      saf_declare_collection(SAF_ALL, &theSet, &elems, SAF_CELLTYPE_QUAD, numRefs, SAF_1DC(numRefs), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &currentMesh, &theSet, SAF_COMMON(&elems), SAF_TUPLES,
         H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, refList, H5I_INVALID_HID, NULL, stepFile); 
      if (theStep > 0)
      {
         saf_declare_subset_relation(SAF_ALL, db, &lastMesh, &theSet, SAF_COMMON(&elems), SAF_TUPLES,
            H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
         saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, parentList, H5I_INVALID_HID, NULL, stepFile); 
      }
   }
   if (numUnrefs)
   {
      SAF_Set theSet;

      saf_declare_set(SAF_ALL, db, "UNrefinements", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &theSet);
      saf_declare_collection(SAF_ALL, &theSet, &elems, SAF_CELLTYPE_QUAD, numUnrefs, SAF_1DC(numUnrefs), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &currentMesh, &theSet, SAF_COMMON(&elems), SAF_TUPLES,
         H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, unrefList, H5I_INVALID_HID, NULL, stepFile); 
   }
   if (numSame)
   {
      SAF_Set theSet;

      saf_declare_set(SAF_ALL, db, "same", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &theSet);
      saf_declare_collection(SAF_ALL, &theSet, &elems, SAF_CELLTYPE_QUAD, numSame, SAF_1DC(numSame), SAF_DECOMP_TRUE);
      saf_declare_subset_relation(SAF_ALL, db, &currentMesh, &theSet, SAF_COMMON(&elems), SAF_TUPLES,
         H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
      saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, sameList, H5I_INVALID_HID, NULL, stepFile); 
   }

   /************************************
    ******* the processor sets *********
    ************************************/
   sprintf(tmpName, "proc_%03d", myProcNum);
   saf_declare_set(SAF_EACH, db, tmpName, 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &myProcSet);

   /***************************************************************************
    ******* collections on the processor sets (nodes, elems and procs *********
    ***************************************************************************/
   saf_declare_collection(SAF_EACH, &myProcSet, &nodes, SAF_CELLTYPE_POINT, numNodesIown, SAF_1DC(numNodesIown), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_EACH, &myProcSet, &elems, SAF_CELLTYPE_QUAD, numElemsIown, SAF_1DC(numElemsIown), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_EACH, &myProcSet, &procs, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

   /******************************************************************************
    ******* processor to top subsets relations on elems, nodes and procs *********
    ******************************************************************************/
    saf_declare_subset_relation(SAF_EACH, db, &currentMesh, &myProcSet, SAF_COMMON(&procs), SAF_TUPLES,
       H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, &myProcNum, H5I_INVALID_HID, NULL, stepFile); 
    saf_declare_subset_relation(SAF_EACH, db, &currentMesh, &myProcSet, SAF_COMMON(&elems), SAF_TUPLES,
       H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, elemList, H5I_INVALID_HID, NULL, stepFile); 
    saf_declare_subset_relation(SAF_EACH, db, &currentMesh, &myProcSet, SAF_COMMON(&nodes), SAF_TUPLES,
       H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
    saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, nodeList, H5I_INVALID_HID, NULL, stepFile); 

   /******************************************
    ******* load balance information *********
    ******************************************/
    MPI_Allreduce(&numKeeps,&theCount,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
    if (theCount)
    {  int bogusData = 1;
       void *bogusBuf = (void*) &bogusData;
       saf_declare_set(SAF_EACH, db, "keeps", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &myKeeps);
       saf_declare_collection(SAF_EACH, &myKeeps, &elems, SAF_CELLTYPE_QUAD, numKeeps, SAF_1DC(numKeeps), SAF_DECOMP_TRUE);
       saf_declare_subset_relation(SAF_EACH, db, &myProcSet, &myKeeps, SAF_COMMON(&elems), SAF_TUPLES,
          H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
       saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, keepList?(void*)keepList:bogusBuf, H5I_INVALID_HID, NULL, stepFile); 
    }
    MPI_Allreduce(&numTakes,&theCount,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
    if (theCount)
    {  int bogusData = 1;
       void *bogusBuf = (void*) &bogusData;
       SAF_FieldTmpl theFtmpl;
       SAF_Field theField;

       saf_declare_set(SAF_EACH, db, "takes", 2, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &myTakes);
       saf_declare_collection(SAF_EACH, &myTakes, &elems, SAF_CELLTYPE_QUAD, numTakes, SAF_1DC(numTakes), SAF_DECOMP_TRUE);
       saf_declare_subset_relation(SAF_EACH, db, &myProcSet, &myTakes, SAF_COMMON(&elems), SAF_TUPLES,
          H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
       saf_write_subset_relation(SAF_EACH, &rel, SAF_INT, takeList?(void*)takeList:bogusBuf, H5I_INVALID_HID, NULL, stepFile); 
       saf_declare_field_tmpl(SAF_EACH, db, "givingProc_tmpl", SAF_ALGTYPE_SCALAR,
          SAF_CARTESIAN, SAF_NOT_APPLICABLE_QUANTITY, 1, NULL, &theFtmpl);
       saf_declare_field(SAF_EACH, db, &theFtmpl, "givingProc", &myProcSet, SAF_NOT_APPLICABLE_UNIT, SAF_SELF(db), SAF_ZONAL(&elems),
          SAF_INT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &theField);
       saf_write_field(SAF_EACH, &theField, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, (takeProc?(void**)&takeProc:&bogusBuf), stepFile);
    }

   
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
   rels = (SAF_Rel *)saf_allgather_handles((ss_pers_t*) &trel, &numHandles, NULL);
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
   {
      SAF_FieldTmpl cftmpls[MAX_DIMS];

      for (i = 0; i < 2; i++)
         cftmpls[i] = coords_ctmpl;
      saf_declare_field_tmpl(SAF_EACH, db, "proc_coordinate_ftmpl", SAF_ALGTYPE_VECTOR,
         SAF_CARTESIAN, SAF_QLENGTH, 2, cftmpls, &coords_ftmpl);
   }

   /* declare the scalar, component fields */
   for (i = 0; i < 2; i++)
   {
      sprintf(tmpName,"coord%1d",i);
      saf_declare_field(SAF_EACH, db, &coords_ctmpl, tmpName, &myProcSet, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems), SAF_FLOAT,
         NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &coordComponent[i]);
   }

   /* declare the vector, composite field */
   {
      saf_declare_field(SAF_EACH, db, &coords_ftmpl, "coords", &myProcSet, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems), SAF_FLOAT,
         coordComponent, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coordField);
   }

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

   /***************************
    ***** alternate IDs *******
    ***************************/
   saf_declare_alternate_indexspec(SAF_EACH, db, &myProcSet, &elems, "elem LRCs", LRCIndex_h5, true, SAF_NA_INDEXSPEC,
      false, false, &aspec);
   saf_write_alternate_indexspec(SAF_EACH, &aspec, SAF_INT, (void*) elemLRCs, stepFile);

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	H-Adaptive Use Case
 * Purpose:	Main program for H-Adaptive Use Case 
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(
   int argc,	/* command line argument count */
   char **argv	/* command line arguments */
)
{
  char dbName[MAX_FILENAME] = "hadaptive.saf";
  hbool_t do_multifile = false;
  hbool_t verbose = false;
  int i;
  int failed=0;
  int myProcNum = 0;
  int numProcs = 1;
  int numFields;
  CurrentMeshParams_t currentMeshParams;
  SAF_Field *theFields; 
  DbInfo_t dbInfo;
 
  build_h5_types(); 
  /* Issue: This is really only a parallel test. It doesn't make much sense to run it in serial */ 
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myProcNum);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

  /* this use-case was hard-coded for three processors */
  if (numProcs != 3)
    {
      if (!myProcNum)
	printf("This example must be run on three (3) processors\n");
      SKIPPED;
      MPI_Finalize();
      exit(0);
    }

  /* since we want to see whats happening make sure stdout and stderr are unbuffered */
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  STU_ProcessCommandLine(0, argc, argv,
			 "-multifile",
			 "if specified, write each cycle to a different supplemental file [single file]",
			 &do_multifile,
			 "-dbName %s",
			 "specify output database name [\"loadbalance.saf\"]",
			 &dbName,
			 "-verbose",
			 "provide verbose output during the run",
			 &verbose,
			 STU_END_OF_ARGS); 

  InitMeshParams(&currentMeshParams);

  saf_init(SAF_DEFAULT_LIBPROPS);

  SAF_TRY_BEGIN
    {

      /* do some preperatory stuff for the database */
      OpenDatabase(dbName, do_multifile, &dbInfo);

      /***********************************************
       ***********************************************
       *                 MAIN LOOP                   *
       ***********************************************
       ***********************************************/
      theFields=calloc(10, sizeof(*theFields));
      for (i = 0; i < 6; i++)
	{
	  numFields = 0;

	  if (!myProcNum)
	    printf("Writing step %d\n", i);

	  /* update the current mesh */ 
	  UpdateMesh(i, myProcNum, &currentMeshParams); 

	  /* write the current mesh and all of its fields */
	  WriteCurrentMesh(&dbInfo, i, numProcs, myProcNum, currentMeshParams, &theFields[numFields], &numFields);

	  /* link this mesh instance into the aggregate, update the state fields, flush the database, etc. */
	  UpdateDatabase(&dbInfo, i, numProcs+1, do_multifile, numFields, theFields);

	}
      free(theFields);

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
