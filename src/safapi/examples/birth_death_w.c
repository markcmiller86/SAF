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

#define MAX_FILENAME	256
#define MAX_OBJNAME	32

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Birth and Death Use Case 
 * Description:	This is testing code that demonstrates how to use SAF to output a mesh in which elements are being created
 *		and destroyed over the course of a simulation. The bulk of the code here is used simply to create some
 *		interesting meshes and has nothing to do with reading/writing from/to SAF. The only routines in which SAF
 *		calls are made are main(), OpenDatabase(), WriteCurrentMesh(), UpdateDatabase() and CloseDatabase(). As such,
 *		these are the only /Public/ functions defined in this file. In addition, the function to parse a sequence
 *		of addition/deletion, GetAddDelSequence(), steps is also public so that a user can see how to program this
 *		client to create a variety of interesting databases.
 *
 *		The test is designed to be relatively flexible in the dimension of mesh (topological and geomertic dimensions
 *		are bound together) and in the various steps it goes through creating and deleting elements. However, elements
 *		are created and deleted in slabs (e.g. planes of elements). There is a default dimension and sequence of steps
 *		hardcoded into this test client. In addition, this test client is designed to also accept input from a text file
 *		that encodes the dimensionality of the mesh and the series of steps of deletions and additions (see
 *		GetAddDelSequence()).
 *
 *		For each step, this test client will output the mesh set, its topology relation, its nodal coordinate field
 *		and the node and element IDs on each instance of the mesh. All mesh instances are collected together
 *		into a user-defined collection on an aggregate set representing the union of the different mesh instances.
 *		In addition, the test client will create subsets (blocks) of the mesh for the various /half/spaces/ in which the
 *		mesh exists. For example, for a 2D mesh, it will create, at most, 4 blocks for the (+,+), (-,+), (-,-) and (+,-)
 *		quadrants of the 2D plane. Such a subset will only be created if, in fact, a portion of the mesh exists in
 *		that particular half-space. We do this primarily to add some interesting subsets to the mesh.
 *
 *              Next, unless '-noSimplices' is specified on the command-line, some of these blocks are refined into simplices.
 *              Those blocks that are refined are those whose low order 2 bits of the half-space index yield 2 or 3. In two
 *              dimensions, each quad is refined into 2 tringles. In three dimensions, each hex is refined into 6 tets. We do
 *              this only to make the element type generated by the code non-uniform.
 *
 *		If no arguments are given, the database will consist of a single file and the four mesh steps depicted in 
 *		"use case 4-1.gif" will be produced. The node and element IDs will be as defined in the diagram.
 *
 *		[figure use_case_4-1.gif]
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Struct for current mesh params	
 *
 * Description:	This structure houses all the parameters for each current step of the mesh. 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct _currentMeshParams {
   AbsRectIndex_t lowerBounds;		/* absolute lower bounds */
   AbsRectIndex_t upperBounds;		/* absolute upper bounds */
   RelRectIndex_t sizeInElems;		/* numer of elements in each dimension */
   RelRectIndex_t sizeInNodes;		/* number of nodes in each dimension */
   RelIndex_t	  numNcubes;		/* total number of Ncube elements */
   RelIndex_t	  numElems;		/* total number of elements (may be larger than numNcubes if some are refined) */
   RelIndex_t	  numNodes;		/* total number of nodes */
   float	  *coordBuf;		/* coordinate data (numDims * numNodes floats) */
   float	  *pressureBuf;		/* a made-up pressure field */
   float	  *nodalMassBuf;	/* a made-up nodal mass field */
   RelIndex_t	  *blockTopoBufs[1<<MAX_DIMS]; /* topology buffer for each block (half-space) */
   RelIndex_t     *nodeIDs;		/* the node ID's buffer (numNodes ids) */
   RelIndex_t     *elemIDs;		/* the elem ID's buffer (numElems ids) */
   HalfSpace_t	  halfSpaces[1<<MAX_DIMS];/* element lists for each half space subset */
} CurrentMeshParams_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Struct for ID tracking mesh params	
 *
 * Description:	This structure houses all the parameters for the ID tracking mesh. The ID tracking mesh is essentially the union
 *		of all meshes from each step. It is used to keep track of element and node IDs so that an ID is fixed with a given
 *		element or node in the mesh.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct _idTrackingMeshParams {
   CurrentMeshParams_t maxMesh;		/* basic mesh parameters for the maximum sized mesh over all steps */
   RelIndex_t   maxAssignedElemID;	/* highest node ID assigned so far */
   RelIndex_t	maxAssignedNodeID;	/* highest element ID assigned so far */
   RelIndex_t   *nodeIDs;		/* the node ID's buffer (1 for each node) */
   RelIndex_t   *elemIDs;		/* the elem ID's buffer (1 for each elem) */
} IDTrackingMeshParams_t;


/*
 * Prototypes
 */
void PrintAddDelSequence( int numDims, int numSteps, int *theOps, ElemSlab_t *theSlabs );
void GetAddDelSequence( const char *inFileName, int *numDims, int *numSteps, int **theOps, ElemSlab_t **theSlabs );
void OpenDatabase( char *dbname, hbool_t do_multifile, int numDims, DbInfo_t *dbInfo );

void ProcessIDs( int numDims, RelIndex_t *srcData, RelRectIndex_t srcSize, AbsRectIndex_t srcLowerBounds,
		 AbsRectIndex_t srcUpperBounds, RelIndex_t *dstData, RelIndex_t dstNum, RelRectIndex_t dstSize,
		 AbsRectIndex_t dstLowerBounds, RelIndex_t *maxAssigned );
void PrintCurrentMeshParams( const char *msg, hbool_t doHalfSpaces, int numDims, CurrentMeshParams_t p );
void PrintIDTrackingMeshParams( const char *msg, int numDims, IDTrackingMeshParams_t m );
void InitCurrentMeshParams( CurrentMeshParams_t *p );
void UpdateCurrentMeshParams( int numDims, int theOp, ElemSlab_t thisStep, CurrentMeshParams_t *p );
float AnalyticPressureFunc( int numDims, float dofCoords[MAX_DIMS], float timeStep );
float AnalyticNodalMassFunc( int numDims, float dofCoords[MAX_DIMS], float timeStep );
void UpdateCurrentMeshData( int numDims, IDTrackingMeshParams_t m, CurrentMeshParams_t *p, float currentTime );
void UpdateIDTrackingMesh( int numDims, CurrentMeshParams_t crnt, IDTrackingMeshParams_t *m );
void InitIDTrackingMeshParams( int numDims, ElemSlab_t firstStep, IDTrackingMeshParams_t *m );
void RefineSomeBlocksNcubesToSimplices( int numDims, CurrentMeshParams_t *p, float currentTime );
void WriteCurrentMesh( DbInfo_t *dbInfo, int theStep, int numDims,	
		       CurrentMeshParams_t theMesh, SAF_Field *fieldList, int *fieldListSize );
void TestIndexing(void);


/*
 * Global vars
 */
SAF_Db *db;


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Print the add/del sequence of steps
 *
 * Description:	This function is used primarily for debugging. It prints the sequence of addition and deletion steps entered
 *		either via the input file or obtained from the default sequence.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
PrintAddDelSequence(
   int numDims,		/* number of dimensions in the mesh */
   int numSteps,	/* number of add/del steps */
   int *theOps,		/* the add/del operations */
   ElemSlab_t *theSlabs	/* the slabs to be added or deleted */
)
{
   int i, rank=0;

#ifdef HAVE_PARALLEL
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

   if (rank != 0)
      return;
   printf("Dimension = %d\n", numDims);
   printf("Got %d steps...\n", numSteps);
   for (i = 0; i < numSteps; i++)
      printf("   step %02d [%s]: %s\n", i, theOps[i]>0 ? "add" : "del", AbsRectIndexStr(numDims,theSlabs[i]));
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public	
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Form the sequence of element additions and deletions	
 *
 * Description:	This function constructs the sequence of element additions or deletions. It either reads input from a file
 *		to construct the sequence or, if no file is specified, generates some default data.
 *
 *		The format of the file is described below...
 *
 *		      ndims=<n>
 *		      step +|- +|-<K0>,+|-<K1>,...,+|-<Kn-1>
 *		      step +|- +|-<K0>,+|-<K1>,...,+|-<Kn-1>
 *		      step +|- +|-<K0>,+|-<K1>,...,+|-<Kn-1>
 *		      .
 *		      .
 *		      .
 *
 *		For example, the file
 *
 *		      ndims=2
 *		      step + +2,+3
 *		      step + +2,+0
 *		      step - +0,-1
 *                    step - -1,+0
 *
 *		creates several steps in the life of a 2 dimensional mesh of quads illustrated in "use case 4-1.gif".
 *		[figure use_case_4-1.gif]
 *		The mesh depicted in "use case 4-1.gif" is, in fact, the default sequence if no input file is specified.
 *
 *		The "ndims=2" line specifies the fact that
 *		the mesh will be 2D. In turn, this means that every "step" line in the file will be a 2-tuple of integers.
 *		The only valid values for ndims are 1, 2 and 3. Each step line specifies elements to add or delete. A plus ('+')
 *		sign immediately after "step" indicates an addition while a minus sign ('-') indicates a deletion. The signs
 *		on the entries of the tuples indicate which /end/ of the corresponding axis at which the addition or deletion
 *		will occur. The *first* step line is always an addition. For example, in the above steps, the first step
 *		line creates a mesh 2 elements wide extending in the positive 'x' direction by three elements high extending
 *		in the positive 'y' direction from the origin. A +0 or -0 entry means the addition or deletion *does*not*
 *		involve elements along this axis.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 6, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
GetAddDelSequence(
   const char *inFileName,	/* [IN] name of input file or NULL if no input file specified */
   int *numDims,		/* [OUT] the number of spatial and topological dimensions of the mesh */
   int *numSteps,		/* [OUT] the number of addition/deletion steps */
   int **theOps,		/* [OUT] array of length numSteps indicating the operation (add=+1,delete=-1) */
   ElemSlab_t **theSlabs	/* [OUT] array of element slabs, one for each step */
)
{
   int rank = 0;

#ifdef HAVE_PARALLEL
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

   /* only processor 0 reads the file */
   if (rank == 0)
   {
      if (inFileName[0] == '\0')
      {
         int n = 0;

         /* generate and return the default data */
         *numDims = 2;
         *numSteps = 4;
         *theSlabs = (ElemSlab_t *) malloc((*numSteps) * sizeof(ElemSlab_t));
         *theOps = (int *) malloc((*numSteps) * sizeof(int));

         /* first step (add) */
         (*theOps)[n] = 1; (*theSlabs)[n].idx[0] = 2; (*theSlabs)[n].idx[1] = 3; n += 1;

         /* second step (add) */
         (*theOps)[n] = 1; (*theSlabs)[n].idx[0] = 2; (*theSlabs)[n].idx[1] = 0; n += 1;

         /* third step (delete) */
         (*theOps)[n] = -1; (*theSlabs)[n].idx[0] = 0; (*theSlabs)[n].idx[1] = -1; n += 1;

         /* fourth step (delete) */
         (*theOps)[n] = -1; (*theSlabs)[n].idx[0] = -1; (*theSlabs)[n].idx[1] = 0; n += 1;

      }
      else
      {
         int count;
         char line[128];
         const char *fmtStr[3] = {"step %c %c%d\n","step %c %c%d, %c%d\n","step %c %c%d, %c%d, %c%d\n"};
         FILE *f;

         /* open and read the input file */
         if ((f = fopen(inFileName, "r")) == NULL)
            AbortThisMess("unable to open input file");

         /* get the number of dimensions */
         if (fscanf(f,"ndims=%d\n", numDims) != 1)
            AbortThisMess("cannot read \"ndims\" line");

         if (*numDims < 1 || *numDims > 3)
            AbortThisMess("ndims out of range [1,3]");

         /* count number of "step" lines */
         count = 0;
         while (fgets(line, sizeof(line), f) != NULL)
            count++;
         *numSteps = count;

         if (*numSteps < 1)
            AbortThisMess("must have at least one step in the birth/death sequence");

         /* allocate the steps output array */
         *theOps = (int *) malloc(count * sizeof(int));
         *theSlabs = (ElemSlab_t *) calloc((size_t)count,sizeof(ElemSlab_t));

         /* rewind back to beginning and skip past first line */
         rewind(f);
         fgets(line, sizeof(line), f);

         /* now, read the step lines into the steps array */
         {
	    char op, s1, s2, s3; int n1, n2, n3;
	    int reCount = 0;
	    int *thisOp = *theOps;
	    ElemSlab_t *thisSlab = *theSlabs;

            switch (*numDims)
            {
               case 1:
	          while (fscanf(f, fmtStr[0], &op, &s1, &n1) == 3)
	          {
		     *thisOp++ = (op == '+' ? 1 : -1); 
		     (*thisSlab++).idx[0] = (s1 == '+' ? n1 : -n1);
	             reCount++;
	          }
	          break;
	       case 2:
	          while (fscanf(f, fmtStr[1], &op, &s1, &n1, &s2, &n2) == 5)
	          {
		     *thisOp++ = (op == '+' ? 1 : -1); 
		     (*thisSlab  ).idx[0] = (s1 == '+' ? n1 : -n1);
		     (*thisSlab++).idx[1] = (s2 == '+' ? n2 : -n2);
	             reCount++;
	          }
	          break;
	       case 3:
	          while (fscanf(f, fmtStr[2], &op, &s1, &n1, &s2, &n2, &s3, &n3) == 7)
	          {
		     *thisOp++ = (op == '+' ? 1 : -1); 
		     (*thisSlab  ).idx[0] = (s1 == '+' ? n1 : -n1);
		     (*thisSlab  ).idx[1] = (s2 == '+' ? n2 : -n2);
		     (*thisSlab++).idx[2] = (s3 == '+' ? n3 : -n3);
	             reCount++;
	          }
	          break;
	       default:
	          break;
            }

	    if (reCount != count)
	    {
	       char tmpMsg[1024];
	       sprintf(tmpMsg, "input file error, perhaps on line %d", reCount+1);
               AbortThisMess(tmpMsg);
	    }
         }

         /* close the file */
         fclose(f);

      }
   }

#ifdef HAVE_PARALLEL
   MPI_Bcast(numDims, 1, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Bcast(numSteps, 1, MPI_INT, 0, MPI_COMM_WORLD);
   if (rank != 0)
   {
      *theOps = (int *) malloc(*numSteps * sizeof(int));
      *theSlabs = (ElemSlab_t *) malloc(*numSteps * sizeof(ElemSlab_t));
   }
   MPI_Bcast(*theOps, *numSteps, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Bcast(*theSlabs, *numSteps * MAX_DIMS, MPI_INT, 0, MPI_COMM_WORLD);
#endif

}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public	
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Open a new database and do some prepratory work on it
 *
 * Description:	This function creates the initial database and some key objects such as the top-most aggregate set,
 *		and the state suite.
 *
 *		The aggregate set *is*extendible* because the infinity of points that comprise it can grow (or shrink).
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 6, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
OpenDatabase(
   char *dbname,		/* [IN] name of the database */
   hbool_t do_multifile,	/* [IN] boolean to indicate if each step will go to a different supplemental file */
   int numDims,			/* [IN] number of topological and geometric dimensions in the mesh */
   DbInfo_t *dbInfo		/* [OUT] database info object */
)
{
   SAF_DbProps *dbprops=NULL;
   /* SAF_Db db; */
   SAF_Cat nodes, elems, blocks;
   SAF_Set aggregateMesh;
   SAF_Db *theFile; 

   /* create the database */
   dbprops = saf_createProps_database();
   saf_setProps_Clobber(dbprops);
   db = saf_open_database(dbname,dbprops);
   dbInfo->db = db;

   /* declare nodes, elems and blocks categories */
   saf_declare_category(SAF_ALL, db, "nodes", SAF_TOPOLOGY, 0, &nodes);
   dbInfo->nodes = nodes;
   saf_declare_category(SAF_ALL, db, "elems", SAF_TOPOLOGY, numDims, &elems);
   dbInfo->elems = elems;
   saf_declare_category(SAF_ALL, db, "blocks", SAF_BLOCK, numDims, &blocks);
   dbInfo->blocks = blocks;

   /* create the aggregate set */
   saf_declare_set(SAF_ALL, db, "max-whole", numDims, SAF_SPACE, SAF_EXTENDIBLE_TRUE, &aggregateMesh);
   dbInfo->mainMesh = aggregateMesh;

   /* if necessary, create the first supplemental file */
   if (do_multifile)
      theFile = saf_open_database("step_000",dbprops);
   else
      theFile = db;
   dbInfo->currentFile = theFile;
}




/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Build ID fields
 *
 * Description:	This function computes new ID fields from existing ID fields.  The algorithm works in such a way that once
 *		an ID is assigned to a particular member (node or element), that ID is forever associated with that object.
 *		If the object is deleted and then added again, its ID will be the same.
 *
 * Issue:	We assign element ids that are multiples of 8. This is arbitrary but helpful when it comes to doing
 *		simplicial refinement, if any, later as it leaves /holes/ in the namespace for us to insert the names of the 
 *		simplicial elements we add by refinement.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 13, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
ProcessIDs(
   int numDims,
   RelIndex_t *srcData,
   RelRectIndex_t srcSize,
   AbsRectIndex_t srcLowerBounds,
   AbsRectIndex_t srcUpperBounds,
   RelIndex_t *dstData,
   RelIndex_t dstNum,
   RelRectIndex_t dstSize,
   AbsRectIndex_t dstLowerBounds,
   RelIndex_t *maxAssigned
)
{
   RelIndex_t dstIndex, srcIndex;
   RelRectIndex_t dstRectIndex, srcRectIndex;
   AbsRectIndex_t absRectIndex;

   /* fill ID buffer by copies from existing ID buffer or assigning new IDs */ 
   for (dstIndex = 0; dstIndex < dstNum; dstIndex++)
   {
      /* compute relative rect-index on the destination mesh */
      dstRectIndex = RelRectIndex(numDims, dstSize, dstIndex);

      /* compute absolute rect-index */
      absRectIndex = Rel2AbsIndex(numDims, dstLowerBounds, dstRectIndex); 

      /* If this index is "IN" the src mesh, copy its ID from the src. Otherwise, assign a new one */
      if (AbsIndexIn(numDims, srcLowerBounds, srcUpperBounds, absRectIndex))
      {
	 /* compute relative index on src mesh */
	 srcRectIndex = Abs2RelIndex(numDims, srcLowerBounds, absRectIndex);
	 srcIndex = LinearIndex(numDims, srcSize, srcRectIndex);
	 dstData[dstIndex] = srcData[srcIndex];
      }
      else
      {
	 if (maxAssigned == NULL)
	    AbortThisMess("destination index not in source");
	 dstData[dstIndex] = (*maxAssigned)+=8;
      }
   }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Print parameters of the current mesh
 *
 * Description:	This function is used primarily for debugging. It prints the parameters of the current mesh.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 13, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
PrintCurrentMeshParams(
   const char *msg,		/* [IN] message to print before printing the mesh info. Pass NULL for a generic message. */
   hbool_t doHalfSpaces,	/* [IN] flag to indicate if half-space information should also be printed */
   int numDims,			/* [IN] number of dimensions in current mesh */
   CurrentMeshParams_t p	/* [IN] mesh parameters to be printed */
)
{
   int i, rank=0;

#ifdef HAVE_PARALLEL
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

   if (rank != 0)
      return;
   if (msg)
      printf("%s...\n",msg);
   printf("   lowerBounds:  %s\n", AbsRectIndexStr(numDims,p.lowerBounds));
   printf("   upperBounds:  %s\n", AbsRectIndexStr(numDims,p.upperBounds));
   printf("   sizeInElems:  %s\n", RelRectIndexStr(numDims,p.sizeInElems));
   printf("   sizeInNodes:  %s\n", RelRectIndexStr(numDims,p.sizeInNodes));
   printf("   numNcubes:    %03d\n", p.numNcubes);
   printf("   numElems:     %03d\n", p.numElems);
   printf("   numNodes:     %03d\n", p.numNodes);
   if (doHalfSpaces)
   {
      printf("   halfSpaces...\n");
      for (i = 0; i < (1<<numDims); i++)
         printf("      space %s: %d elems\n", HalfSpaceIndexStr(numDims, (HalfSpaceIndex_t) i), p.halfSpaces[i].count);
   }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Print parameters of the ID tracking mesh
 *
 * Description:	This function is used primarily for debugging. It prints the parameters of the ID tracking mesh.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 13, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
PrintIDTrackingMeshParams(
   const char *msg,		/* [IN] message to print before printing the mesh info. Pass NULL for a generic message. */
   int numDims,			/* [IN] number of dimensions in current mesh */
   IDTrackingMeshParams_t m	/* [IN] mesh parameters to be printed */
)
{
   int rank = 0;

#ifdef HAVE_PARALLEL
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

   if (rank != 0)
      return;
   if (msg)
      printf("%s...\n", msg);
   PrintCurrentMeshParams(NULL, 0, numDims, m.maxMesh);
   printf("   maxElemID:    %03d\n", m.maxAssignedElemID);
   printf("   maxNodeID:    %03d\n", m.maxAssignedNodeID);
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Initialize new mesh parameters
 *
 * Description:	This function initialize a new set of mesh parameters 
 *
 * Programmer:	Mark Miller, LLNL, Tue Dec 18, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
InitCurrentMeshParams(
   CurrentMeshParams_t *p	/* [IN/OUT] mesh parameter structure to initialize */
)
{
   int i;

   for (i = 0; i < MAX_DIMS; i++)
   {
      p->lowerBounds.idx[i] = 0;
      p->upperBounds.idx[i] = 0;
      p->sizeInElems.idx[i] = 0;
      p->sizeInNodes.idx[i] = 0;
   }
   for (i = 0; i < (1<<MAX_DIMS); i++)
   {
      p->blockTopoBufs[i] = NULL;
      p->halfSpaces[i].allocated = 0;
      p->halfSpaces[i].count = 0;
      p->halfSpaces[i].elemList = NULL;
   }
   p->numNcubes = 0;
   p->numElems = 0;
   p->numNodes = 0;
   p->coordBuf = NULL;
   p->pressureBuf = NULL;
   p->nodalMassBuf = NULL;
   p->nodeIDs = NULL;
   p->elemIDs = NULL;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Compute new mesh parameters
 *
 * Description:	This function computes the new mesh parameters for the current step including number of nodes and elemnts,
 *		size in each dimension and absolute bounds.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 13, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
UpdateCurrentMeshParams(
   int numDims,			/* [IN] number of dimensions in the mesh */
   int theOp,			/* [IN] this steps add/del operation (+1=add, -1=del, 0=init) */
   ElemSlab_t thisStep,		/* [IN] this steps slab of elements */
   CurrentMeshParams_t *p	/* [IN/OUT] mesh parameters to be updated */
)
{
   int i;
   RelRectIndex_t maxRectElem, maxRectNode;

   /* compute current size */
   for (i = 0; i < numDims; i++)
   {
      if ((theOp < 0) && (ABS(thisStep.idx[i]) > p->sizeInElems.idx[i]))
      {
         char tmpMsg[1024];
	 sprintf(tmpMsg, "current step produces a negative size on dimension %d", i);
	 AbortThisMess(tmpMsg);
      }

      p->sizeInElems.idx[i] += (RelIndex_t) (theOp * ABS(thisStep.idx[i]));
   }

   /* number of nodes in each dimension is always 1 greater than number of elems */
   p->sizeInNodes = AddRelRectIndex(numDims, p->sizeInElems, relAllOnes);

   /* determine total number of elements and nodes (= linear index of highest rectangular index + 1)  */
   maxRectElem = SubRelRectIndex(numDims, p->sizeInElems, relAllOnes);
   maxRectNode = SubRelRectIndex(numDims, p->sizeInNodes, relAllOnes);
   p->numNcubes = LinearIndex(numDims, p->sizeInElems, maxRectElem) + 1;
   p->numElems = p->numNcubes; /* numElems may get larger than numNcubes if some blocks are refined */
   p->numNodes = LinearIndex(numDims, p->sizeInNodes, maxRectNode) + 1;

   /* adjust absolute upper and lower bounds */
   for (i = 0; i < numDims; i++)
   {
      if (thisStep.idx[i] < 0)
         p->lowerBounds.idx[i] -= (theOp * ABS(thisStep.idx[i])); 
      else
         p->upperBounds.idx[i] += (theOp * ABS(thisStep.idx[i])); 
   }

}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Define Interesting Pressure Function	
 *
 * Description:	This function is used to define a pressure function...
 *
 *                        2
 *		     t * r
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

   return timeStep * r; 
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Define Interesting Nodal Mass Function	
 *
 * Description:	This function is used to define a nodal mass function...
 *
 *		    2    2
 *		   r / (t + 1)
 *
 *		where `t' is the time and r is the radius from the origin.
 *
 * Programmer:	Mark Miller, LLNL, Wed Jan 30, 2002
 *---------------------------------------------------------------------------------------------------------------------------------
 */
float
AnalyticNodalMassFunc(
   int numDims,			/* [IN] number of dimensions in the mesh */ 
   float dofCoords[MAX_DIMS],	/* [IN] coordinates on the mesh at which to compute the interpolating dof */
   float timeStep		/* [IN] coordinate in time at which to compute the interpolating dof */
)
{
   int i;
   float r = 0.0;

   for (i = 0; i < numDims; i++)
      r += dofCoords[i] * dofCoords[i];

   return r / (timeStep * timeStep + 1);
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Compute all fields on the new mesh
 *
 * Description:	This function computes the topolgy relation data, subset relation data and all the fields on the new mesh for
 *		the current step.
 *
 * Programmer:	Mark Miller, LLNL, Tue Dec 18, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
UpdateCurrentMeshData(
   int numDims,			/* [IN] number of dimensions in the mesh */
   IDTrackingMeshParams_t m,	/* [IN] the id tracking mesh */
   CurrentMeshParams_t *p,	/* [IN/OUT] the current mesh object */
   float currentTime		/* [IN] current simulated time in the simulation */
)
{
   int j;
   RelIndex_t linearElem;
   RelRectIndex_t relRectElem;
   AbsRectIndex_t nodesUpperBound;


   /*************************
    compute new coordinate data
    *************************/
   if (p->coordBuf)
      free(p->coordBuf);

   p->coordBuf = BuildCoordinateData(numDims, (int) p->numNodes, p->sizeInNodes, p->lowerBounds, NULL);


   /*************************
    compute new pressure data
    *************************/
   if (p->pressureBuf)
      free(p->pressureBuf);

   p->pressureBuf = BuildInterpolatingDofs(PIECEWISE_CONSTANT, numDims, p->sizeInElems, p->lowerBounds, 0, NULL,
      currentTime, AnalyticPressureFunc);

   /*************************
    compute new nodal mass data
    *************************/
   if (p->nodalMassBuf)
      free(p->nodalMassBuf);

   p->nodalMassBuf = BuildInterpolatingDofs(PIECEWISE_LINEAR, numDims, p->sizeInNodes, p->lowerBounds, 0, NULL,
      currentTime, AnalyticNodalMassFunc);

   /*************************
    compute new elem ID data
    *************************/
   if (p->elemIDs)
      free(p->elemIDs);
   p->elemIDs = (RelIndex_t *) malloc(p->numNcubes * sizeof(RelIndex_t));
   ProcessIDs(numDims, m.maxMesh.elemIDs, m.maxMesh.sizeInElems, m.maxMesh.lowerBounds, m.maxMesh.upperBounds,
      p->elemIDs, p->numNcubes, p->sizeInElems, p->lowerBounds, NULL);

   /*************************
    compute new node ID data
    *************************/
   if (p->nodeIDs)
      free(p->nodeIDs);
   p->nodeIDs = (RelIndex_t *) malloc(p->numNodes * sizeof(RelIndex_t));
   /* need a slightly different upper bound for the nodes */
   if (m.maxMesh.numNcubes)
      nodesUpperBound = AddAbsRectIndex(numDims, m.maxMesh.upperBounds, absAllOnes);
   else
      nodesUpperBound = m.maxMesh.lowerBounds;
   ProcessIDs(numDims, m.maxMesh.nodeIDs, m.maxMesh.sizeInNodes, m.maxMesh.lowerBounds, nodesUpperBound, 
      p->nodeIDs, p->numNodes, p->sizeInNodes, p->lowerBounds, NULL);

   /******************************
    compute new half-space subsets
    ******************************/
   /* reset all the counts for the halfSpace lists */
   for (j = 0; j < (1<<numDims); j++)
      p->halfSpaces[j].count = 0;

   /* march through the elements determining their assignment to half-spaces */
   for (linearElem = 0; linearElem < p->numNcubes; linearElem++)
   {
      AbsRectIndex_t absRectElem;
      HalfSpaceIndex_t halfSpace;

      /* compute this elem's absolute index */
      relRectElem = RelRectIndex(numDims, p->sizeInElems, linearElem);
      absRectElem = Rel2AbsIndex(numDims, p->lowerBounds, relRectElem);

      /* convert this absolute index to a half-space id */
      halfSpace = Abs2HalfSpaceIndex(numDims, absRectElem);

      /* put the current element in this half space list */
      {
         int allocated = p->halfSpaces[halfSpace].allocated;
	 int count = p->halfSpaces[halfSpace].count;
	 RelIndex_t *elemList = p->halfSpaces[halfSpace].elemList;

	 /* check if new allocation is needed */
         if (count >= allocated-1)
         {
	    allocated = allocated * 3 / 2 + 2;
            elemList = (RelIndex_t *) realloc(elemList, allocated * sizeof(RelIndex_t));
         }
         elemList[count++] = linearElem;
	 p->halfSpaces[halfSpace].count = count;
	 p->halfSpaces[halfSpace].allocated = allocated;
	 p->halfSpaces[halfSpace].elemList = elemList;
      }
   }

   /*****************************************************
    compute new topology data for each block (half-space)
    *****************************************************/
   for (j = 0; j < (1<<numDims); j++)
   {
      if (p->halfSpaces[j].count)
      {
         if (p->blockTopoBufs[j])
            free(p->blockTopoBufs[j]);

         p->blockTopoBufs[j] = BuildTopologyData(numDims, (int) p->halfSpaces[j].count, p->sizeInElems, p->sizeInNodes,
	    p->halfSpaces[j].elemList);
      }
   }

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Compute ID tracking mesh parameters	
 *
 * Description:	This function computes a possibly new set of ID tracking mesh parameters from the given current mesh 
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 13, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
UpdateIDTrackingMesh(
   int numDims,			/* [IN] number of dimensions in the mesh */
   CurrentMeshParams_t crnt,	/* [IN] current mesh parameters */
   IDTrackingMeshParams_t *m 	/* [IN/OUT] ID tracking mesh parameters to be updated */
)
{
   int i;
   hbool_t newOverallMax;
   AbsRectIndex_t nodesUpperBound;
   IDTrackingMeshParams_t old, new;

   /* save current ID tracking mesh params */ 
   old = *m;
   new = *m;

   /* compare current mesh bounds to the ID tracking mesh, adjust ID tracking mesh bounds if necessary */
   newOverallMax = false;
   for (i = 0; i < numDims; i++)
   {
      if (crnt.lowerBounds.idx[i] < new.maxMesh.lowerBounds.idx[i])
      {
	 new.maxMesh.lowerBounds.idx[i] = crnt.lowerBounds.idx[i];
	 newOverallMax = true;
      }
      if (crnt.upperBounds.idx[i] > new.maxMesh.upperBounds.idx[i])
      {
	 new.maxMesh.upperBounds.idx[i] = crnt.upperBounds.idx[i];
	 newOverallMax = true;
      }
   }

   /* if we have a new ID tracking mesh, construct new elem and node ids buffers */ 
   if (newOverallMax)
   {
      AbsRectIndex_t extentInElems;
      RelRectIndex_t maxRelRectElem, maxRelRectNode;

      /* compute max relative index of nodes and elems */
      extentInElems = SubAbsRectIndex(numDims, new.maxMesh.upperBounds, new.maxMesh.lowerBounds);
      for (i = 0; i < numDims; i++)
	 new.maxMesh.sizeInElems.idx[i] = (RelIndex_t) extentInElems.idx[i];
      new.maxMesh.sizeInNodes = AddRelRectIndex(numDims, new.maxMesh.sizeInElems, relAllOnes);
      maxRelRectElem = SubRelRectIndex(numDims, new.maxMesh.sizeInElems, relAllOnes);
      maxRelRectNode = SubRelRectIndex(numDims, new.maxMesh.sizeInNodes, relAllOnes);

      /* compute total number of elements and nodes in the new ID tracking mesh */
      new.maxMesh.numNcubes = LinearIndex(numDims, new.maxMesh.sizeInElems, maxRelRectElem) + 1;
      new.maxMesh.numNodes = LinearIndex(numDims, new.maxMesh.sizeInNodes, maxRelRectNode) + 1;
      
      /* allocate new buffers for ID data */
      new.maxMesh.elemIDs = (RelIndex_t *) malloc(new.maxMesh.numNcubes * sizeof(RelIndex_t));

      /* build the element IDs */
      ProcessIDs(numDims, old.maxMesh.elemIDs, old.maxMesh.sizeInElems, old.maxMesh.lowerBounds, old.maxMesh.upperBounds,
         new.maxMesh.elemIDs, new.maxMesh.numNcubes, new.maxMesh.sizeInElems, new.maxMesh.lowerBounds, &new.maxAssignedElemID);

      /* now do same for nodes */
      new.maxMesh.nodeIDs = (RelIndex_t *) malloc(new.maxMesh.numNodes * sizeof(RelIndex_t));

      /* need a slightly different upper bound for the nodes */
      if (old.maxMesh.numNcubes)
         nodesUpperBound = AddAbsRectIndex(numDims, old.maxMesh.upperBounds, absAllOnes);
      else
         nodesUpperBound = old.maxMesh.lowerBounds;

      /* build the node IDs */
      ProcessIDs(numDims, old.maxMesh.nodeIDs, old.maxMesh.sizeInNodes, old.maxMesh.lowerBounds, nodesUpperBound,
         new.maxMesh.nodeIDs, new.maxMesh.numNodes, new.maxMesh.sizeInNodes, new.maxMesh.lowerBounds, &new.maxAssignedNodeID);

      /* free the old ID buffers */
      if (old.maxMesh.elemIDs != NULL)
         free(old.maxMesh.elemIDs);
      if (old.maxMesh.nodeIDs != NULL)
         free(old.maxMesh.nodeIDs);

      /* set the new ID tracking mesh parameters */
      *m = new;

   }

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Initialize ID tracking mesh parameters	
 *
 * Description:	This function initializes a new set of ID tracking mesh parameters 
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 13, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
InitIDTrackingMeshParams(
   int numDims,			/* [IN] number of dimensions in the mesh */
   ElemSlab_t firstStep,	/* [IN] the first step in the sequence */
   IDTrackingMeshParams_t *m 	/* [IN/OUT] ID tracking mesh parameters to be updated */
)
{
   int i;
   RelIndex_t numNodes, numNcubes;
   CurrentMeshParams_t firstMesh;

   InitCurrentMeshParams(&(m->maxMesh));
   m->maxAssignedNodeID = 0;
   m->maxAssignedElemID = 0;
   m->nodeIDs = NULL;
   m->elemIDs = NULL;

   /* create a CurrentMeshParams_t object that would be generated in the first step */
   numNodes = 1;
   numNcubes = 1;
   for (i = 0; i < MAX_DIMS; i++)
   {
      if (firstStep.idx[i] < 0)
      {
         firstMesh.lowerBounds.idx[i] = firstStep.idx[i];
	 firstMesh.upperBounds.idx[i] = 0;
      }
      else
      {
	 firstMesh.lowerBounds.idx[i] = 0;
         firstMesh.upperBounds.idx[i] = firstStep.idx[i];
      }
      firstMesh.sizeInElems.idx[i] = ABS(firstStep.idx[i]);
      firstMesh.sizeInNodes.idx[i] = ABS(firstStep.idx[i])+1;
      numNcubes *= ABS(firstStep.idx[i]);
      numNodes *= (ABS(firstStep.idx[i])+1);
   }
   firstMesh.numNcubes = numNcubes;
   firstMesh.numNodes = numNodes;

   /* ok, now we can update the ID tracking mesh from the firstMesh to complete its initialization */ 
   UpdateIDTrackingMesh(numDims, firstMesh, m);
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private	
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Refine some of the blocks in the mesh into simplices
 *
 * Description:	This function modifies the current mesh data so that some of its blocks' elements get turned into N dimensionsal
 *		simplices. Any block whose /parity/ (low order 2 bits of half space index) is 2 or 3 will be refined from N
 *		dimensional cubes into simplices (tris in 2D, tets in 3D). In 1D, this operation is meaningless.
 *
 *		We do this soley to create some element blocks in the output database with a different element type.
 *
 *		This involves 1) adjusting the topology relations on each block, 2) adjusting the element ids on each block
 *		(note that node ids *do*not* change), 3) adjusting any zonal field data and adjusting the total element
 *		count in the overall mesh
 *
 * Programmer:	Mark Miller, LLNL, May 01, 2002 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
RefineSomeBlocksNcubesToSimplices(
   int numDims,			/* [IN] number of dimensions in the mesh */
   CurrentMeshParams_t *p,	/* [IN/OUT] current mesh parameters (buffers may change) */
   float currentTime		/* [IN] current time step time (currently unused) */
)
{
   int i,j;
   int newTotalNumElems;
   int numSimplicesPerNCube = (numDims == 2) ? 2 : 6; /* 2 tris per quad or 6 tets per hex */
   int numNcubes = p->numNcubes;
   float *oldPressureBuf = p->pressureBuf;
   float *newPressureBuf = NULL;
   RelIndex_t *oldElemIDs = p->elemIDs;
   RelIndex_t *refinedIndices = NULL;
   RelIndex_t *newElemIDs = NULL;
   RelIndex_t *newElemList = NULL;

   currentTime = currentTime; /* quiet the compiler */

   /* if this is a 1D mesh, this operation is meaningless */
   if (numDims == 1)
      return;

   /* first, compute the new total number of elements we'll have after refinements of cubes into simplices */ 
   newTotalNumElems = numNcubes;
   for (i = 0; i < (1<<numDims); i++)
   {
      int count = p->halfSpaces[i].count;
      int parity = i & 0x3; /* use lower order 2 bits to decide which half-spaces get refined */

      if (((parity == 1) || (parity == 2)) && (count))
	 newTotalNumElems += (count * (numSimplicesPerNCube-1));
   }

   /* return immediately if we aren't going to wind up refining any cubes into simplices */ 
   if (newTotalNumElems == numNcubes)
      return;

   /* allocate space for the new element IDs and zonal fields */
   newElemIDs = (RelIndex_t *) malloc(newTotalNumElems * sizeof(RelIndex_t));
   newPressureBuf = (float *) malloc(newTotalNumElems * sizeof(float)); 

   /* Allocate and build the refined indices to aid in indexing into the new element list. The call to calloc will put zeros
      everywhere. The 1rst loop puts 1's wherever we're refining an Ncube into simplices. Finally, the 2nd loop goes
      through finding 0's and 1's and replacing these with correct element indices. */
   refinedIndices = (RelIndex_t *) calloc( (size_t)numNcubes, sizeof(RelIndex_t));

   /* 1rst pass to put 1's everywhere we're refining and nCube */
   for (i = 0; i < (1<<numDims); i++)
   {
      int count = p->halfSpaces[i].count;
      int parity = i & 0x3; /* use lower order 2 bits to decide which half-spaces get refined */

      if (((parity == 1) || (parity == 2)) && (count))
      {
	 for (j = 0; j < count ; j++)
	 {
	    int elemIndex = p->halfSpaces[i].elemList[j];
	    refinedIndices[elemIndex] = 1;
	 }
      }
   }

   /* 2nd pass to replace 0's and 1's with accumulated indices */
   j = 0;
   for (i = 0; i < numNcubes; i++)
   {
      int flag = refinedIndices[i];
      refinedIndices[i] = j;
      if (flag)
	 j += numSimplicesPerNCube;
      else
	 j += 1;
   }


   /* Loop over each of the half-space blocks. For each block we'll refine, refine that block into a block of simplices */
   for (i = 0; i < (1<<numDims); i++)
   {
      int count = p->halfSpaces[i].count;
      int parity = i & 0x3; /* use lower order 2 bits to decide which half-spaces get refined */

      if (((parity == 1) || (parity == 2)) && (count))
      {
	 RelIndex_t *oldTopoBuf = p->blockTopoBufs[i];
	 RelIndex_t *newTopoBuf;

	 /* compute new, refined topology data, replacing every Ncube with numSimplicesPerNCube simplices */
	 newTopoBuf = BuildSimplicializedTopologyData(numDims, count, oldTopoBuf);
	 free(oldTopoBuf);
	 p->blockTopoBufs[i] = newTopoBuf;

	 /* compute new element ids and zonal field, replacing every element id with numSimplicesPerNCube ids
	    and every pressure value with numSimplicesPerNCube pressure values */
	 for (j = 0; j < count ; j++)
	 {
	    int elemIndex = p->halfSpaces[i].elemList[j];
	    int kstart = refinedIndices[elemIndex];
	    int k, kk;

	    for (kk = 1, k = kstart; k < kstart + numSimplicesPerNCube; kk++, k++)
	    {
	       newElemIDs[k] = oldElemIDs[elemIndex] + kk; 
	       newPressureBuf[k] = oldPressureBuf[elemIndex];
	    }
	 }

	 /* finally, create the new element list for this block */
	 newElemList = (RelIndex_t *) malloc(count * numSimplicesPerNCube * sizeof(RelIndex_t));
	 for (j = 0; j < count; j++)
	 {
	    int elemIndex = p->halfSpaces[i].elemList[j];
	    int kstart = refinedIndices[elemIndex];
	    int k, kk;

	    for (kk = j * numSimplicesPerNCube, k = kstart; k < kstart + numSimplicesPerNCube; kk++, k++)
	       newElemList[kk] = k; 
	 }
	 free(p->halfSpaces[i].elemList);
	 p->halfSpaces[i].elemList = newElemList;
	 p->halfSpaces[i].count *= numSimplicesPerNCube;
      }
      else
      {
	 /* just copy the old element ids and pressure values into the new positions */
	 for (j = 0; j < count ; j++)
	 {
	    int elemIndex = p->halfSpaces[i].elemList[j];
	    int refinedIndex = refinedIndices[elemIndex];

	    newElemIDs[refinedIndex] = oldElemIDs[elemIndex]; 
	    newPressureBuf[refinedIndex] = oldPressureBuf[elemIndex];
	 }
      }
   }

   /* replace the old element IDs and pressure buf with the new ones */
   free(oldElemIDs);
   free(oldPressureBuf);
   p->elemIDs = newElemIDs;
   p->pressureBuf = newPressureBuf;

   /* ok, now set the numElems to the new total */
   p->numElems = newTotalNumElems;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Write current mesh to the SAF database	
 *
 * Description:	This function does all the work of writing the current mesh, including its topology relation, subset relations
 *		and fields, to the SAF database.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 13, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
WriteCurrentMesh(
   DbInfo_t *dbInfo,		/* [IN/OUT] database info object */
   int theStep,			/* current step number */
   int numDims,			/* [IN] number of dimensions in mesh */
   CurrentMeshParams_t theMesh,	/* [IN] current mesh parameters */
   SAF_Field *fieldList,	/* [IN/OUT] list of fields we'll append new fields too */
   int *fieldListSize		/* [IN/OUT] On input, the current size of the field list. On output, its new size */
)
{
   SAF_Unit ugram;
   SAF_Unit upascal;
   char tmpName[MAX_OBJNAME];
   int i, nonEmptySpaces;
   float *coordBuf = theMesh.coordBuf;
   float *pressureBuf = theMesh.pressureBuf;
   float *nodalMassBuf = theMesh.nodalMassBuf;
   RelIndex_t *elemIDs = theMesh.elemIDs;
   RelIndex_t *nodeIDs = theMesh.nodeIDs;
   int numNcubes = theMesh.numNcubes;
   int numElems = theMesh.numElems;
   int numNodes = theMesh.numNodes;
   SAF_CellType cellType;
   SAF_Set currentMesh;
   SAF_FieldTmpl coords_ctmpl, coords_ftmpl, pressureFtmpl, nodalMassFtmpl;
   SAF_Field coordField, coordComponent[MAX_DIMS], pressureField, nodalMassField;
   SAF_Rel trel;
   SAF_AltIndexSpec aspec;

   SAF_Db *db = dbInfo->db;
   SAF_Unit umeter;
   SAF_Cat nodes = dbInfo->nodes;
   SAF_Cat elems = dbInfo->elems;
   SAF_Cat blocks = dbInfo->blocks;
   SAF_Db *stepFile = dbInfo->currentFile;

   saf_find_one_unit(db, "meter", &umeter);
   /* count number of non-empty half-spaces (for blocks) */
   nonEmptySpaces = 0;
   for (i = 0; i < (1<<numDims); i++)
   {
      if (theMesh.halfSpaces[i].count)
         nonEmptySpaces++;
   }

   /***********************************************
    ******* this step's mesh (base-space) *********
    ***********************************************/
   sprintf(tmpName, "mesh_step_%03d", theStep);
   saf_declare_set(SAF_ALL, db, tmpName, numDims, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &currentMesh);
   dbInfo->currentMesh = currentMesh;

   /*******************************************************************
    ******* collections on the mesh (nodes, elems, and blocks *********
    *******************************************************************/
   /* nodes and elems collections on the current mesh */
   saf_declare_collection(SAF_ALL, &currentMesh, &nodes, SAF_CELLTYPE_POINT, numNodes, SAF_1DC(numNodes), SAF_DECOMP_FALSE);
   saf_declare_collection(SAF_ALL, &currentMesh, &elems, SAF_CELLTYPE_MIXED, numNcubes, SAF_1DC(numNcubes), SAF_DECOMP_TRUE);
   saf_declare_collection(SAF_ALL, &currentMesh, &blocks, SAF_CELLTYPE_SET, nonEmptySpaces, SAF_1DC(nonEmptySpaces), SAF_DECOMP_TRUE);

   /************************************************************************************
    ******* subset relations on elems and blocks for each non-empty half-space *********
    ************************************************************************************/
   nonEmptySpaces = 0;
   for (i = 0; i < (1<<numDims); i++)
   {
      int nodesPerElem;
      int count = theMesh.halfSpaces[i].count;
      RelIndex_t *elemList = theMesh.halfSpaces[i].elemList;
      RelIndex_t *theTopoBuf = theMesh.blockTopoBufs[i];
      SAF_Set aBlock;
      SAF_Rel rel;

      if (count)
      {
         int parity = i & 0x3; /* use lower order 2 bits to decide which half-spaces get refined */

	 switch (numDims)
	 {
	 case 1:
	    {
	       cellType = SAF_CELLTYPE_LINE;
	       nodesPerElem = 2;
	       break;
	    }
	 case 2:
	    {
	       if ((numElems > numNcubes) && ((parity == 1) || (parity == 2)))
	       {
		  nodesPerElem = 3;
	          cellType = SAF_CELLTYPE_TRI;
	       }
	       else
	       {
		  nodesPerElem = 4;
	          cellType = SAF_CELLTYPE_QUAD;
	       }
	       break;
	    }
	 case 3:
	    {
	       if ((numElems > numNcubes) && ((parity == 1) || (parity == 2)))
	       {
		  nodesPerElem = 4;
	          cellType = SAF_CELLTYPE_TET;
	       }
	       else
	       {
		  nodesPerElem = 8;
	          cellType = SAF_CELLTYPE_HEX;
	       }
	       break;
	    }
	 }

         sprintf(tmpName, "block_(%s)_%03d", HalfSpaceIndexStr(numDims, (HalfSpaceIndex_t) i), theStep);
         saf_declare_set(SAF_ALL, db, tmpName, numDims, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &aBlock);

	 /* elems collection on this block */
         saf_declare_collection(SAF_ALL, &aBlock, &elems, cellType, count, SAF_1DC(count), SAF_DECOMP_TRUE);

	 /* blocks collection on this block (always of size 1) */
         saf_declare_collection(SAF_ALL, &aBlock, &blocks, SAF_CELLTYPE_SET, 1, SAF_1DC(1), SAF_DECOMP_TRUE);

	 /* subset relation on elems */
         saf_declare_subset_relation(SAF_ALL, db, &currentMesh, &aBlock, SAF_COMMON(&elems), SAF_TUPLES,
            H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
         saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, elemList, H5I_INVALID_HID, NULL, stepFile); 

	 /* subset relation on the blocks */
         saf_declare_subset_relation(SAF_ALL, db, &currentMesh, &aBlock, SAF_COMMON(&blocks), SAF_TUPLES,
            H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
         saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &nonEmptySpaces, H5I_INVALID_HID, NULL, stepFile); 

         /*****************************************************************************************
          ******* topology relation for this block (elems on block to nodes on the whole) *********
          *****************************************************************************************/
         /* declare and write topology relation */
         saf_declare_topo_relation(SAF_ALL, db, &aBlock, &elems, &currentMesh, &nodes, SAF_SELF(db), &currentMesh,
            SAF_UNSTRUCTURED, H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &trel);
         saf_write_topo_relation(SAF_ALL, &trel, SAF_INT, &nodesPerElem, SAF_INT, theTopoBuf, stepFile);

         nonEmptySpaces++;
      }
   }

   /**********************************
    ******* coordinate field *********
    **********************************/
   /* declare scalar coordinate field template for components */
   saf_declare_field_tmpl(SAF_ALL, db, "coordinate_ctmpl", SAF_ALGTYPE_SCALAR,
      SAF_CARTESIAN, SAF_QLENGTH, 1, NULL, &coords_ctmpl);

   /* declare vector composite field template */ 
   if (numDims > 1)
   {
      SAF_FieldTmpl cftmpls[MAX_DIMS];

      for (i = 0; i < numDims; i++)
         cftmpls[i] = coords_ctmpl;
      saf_declare_field_tmpl(SAF_ALL, db, "coordinate_ftmpl", SAF_ALGTYPE_VECTOR,
         SAF_CARTESIAN, SAF_QLENGTH, numDims, cftmpls, &coords_ftmpl);
   }

   /* declare the scalar, component fields */
   for (i = 0; i < numDims; i++)
   {
      sprintf(tmpName,"coord%1d",i);
      saf_declare_field(SAF_ALL, db, &coords_ctmpl, tmpName, &currentMesh, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems), SAF_FLOAT,
         NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &coordComponent[i]);
   }

   /* declare the vector, composite field */
   if (numDims > 1)
      saf_declare_field(SAF_ALL, db, &coords_ftmpl, "coords", &currentMesh, &umeter, SAF_SELF(db), SAF_NODAL(&nodes, &elems), SAF_FLOAT,
         coordComponent, SAF_INTERLEAVE_VECTOR, SAF_IDENTITY, NULL, &coordField);

   /* write the coordinate field data on the composite field */
   saf_write_field(SAF_ALL, &coordField, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, (void**) &coordBuf, stepFile);


   /*jake 02/03/2003 note: if you dont do this, saf_declare_state_group will fail*/
  saf_declare_coords(SAF_ALL, &coordField );
  saf_declare_default_coords(SAF_ALL,&currentMesh,&coordField);


   /* put the coordinate field in the output field list */ 
   fieldList[(*fieldListSize)++] = coordField;

   /**********************************
    ******** pressure field **********
    **********************************/
   saf_find_one_unit(db,"pascal",&upascal);
   /* declare and write pressure field */
   saf_declare_field_tmpl(SAF_ALL, db, "pressure_ftmpl", SAF_ALGTYPE_SCALAR,
      SAF_CARTESIAN, saf_find_one_quantity(db,"pressure",NULL), 1, NULL, &pressureFtmpl);
   saf_declare_field(SAF_ALL, db, &pressureFtmpl, "pressure", &currentMesh, &upascal, SAF_SELF(db), 
      SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &pressureField);
   saf_write_field(SAF_ALL, &pressureField, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, (void**) &pressureBuf, stepFile);

   /* put the pressure field in the output field list */ 
   fieldList[(*fieldListSize)++] = pressureField;

   /**********************************
    ******* nodal mass field *********
    **********************************/
   saf_find_one_unit(db,"gram",&ugram);
   /* declare and write nodal mass field */
   saf_declare_field_tmpl(SAF_ALL, db, "nodal_mass_ftmpl", SAF_ALGTYPE_SCALAR,
      SAF_CARTESIAN, saf_find_one_quantity(db,"mass",NULL), 1, NULL, &nodalMassFtmpl);
   saf_declare_field(SAF_ALL, db, &nodalMassFtmpl, "nodal mass", &currentMesh, &ugram, SAF_SELF(db), 
      SAF_ZONAL(&elems), SAF_FLOAT, NULL, SAF_INTERLEAVE_NONE, SAF_IDENTITY, NULL, &nodalMassField);
   saf_write_field(SAF_ALL, &nodalMassField, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, (void**) &nodalMassBuf, stepFile);

   /* put the pressure field in the output field list */ 
   fieldList[(*fieldListSize)++] = nodalMassField;

   /***************************
    ***** alternate IDs *******
    ***************************/
   saf_declare_alternate_indexspec(SAF_ALL, db, &currentMesh, &nodes, "node IDs", SAF_INT, true, SAF_NA_INDEXSPEC,
      false, false, &aspec);
   saf_write_alternate_indexspec(SAF_ALL, &aspec, SAF_INT, (void*) nodeIDs, stepFile);

   saf_declare_alternate_indexspec(SAF_ALL, db, &currentMesh, &elems, "elem IDs", SAF_INT, true, SAF_NA_INDEXSPEC,
      false, false, &aspec);
   saf_write_alternate_indexspec(SAF_ALL, &aspec, SAF_INT, (void*) elemIDs, stepFile);

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private	
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Testing indexing mechanism
 *
 * Description:	This function is used primarily for debugging/testing. It exercises the indexing functions. It assumes a
 *		3 x 8, 2D grid of elements.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
TestIndexing(void)
{
   RelRectIndex_t relRectIndex25 = {{2,5,0}}, sizeInElems = {{3, 7, 0}}, relRectIndex23={{2,3,0}};

   printf("On a mesh of size %s...\n", RelRectIndexStr(2, sizeInElems));

   /* compute linear index of element 2,5 */
   printf("   linear index of 2,5 is %d (should be 17)\n", LinearIndex(2, sizeInElems, relRectIndex25));

   printf("   rect index of 11 is %s (should be 2,3)\n", RelRectIndexStr(2, RelRectIndex(2, sizeInElems, 11)));

   printf("   rect index of 17 is %s (should be 2,5)\n", RelRectIndexStr(2, RelRectIndex(2, sizeInElems, 17)));

   printf("   linear index of 2,3 is %d (should be 11)\n", LinearIndex(2, sizeInElems, relRectIndex23));

   exit(0);

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Main program for Birth and Death of Elements Use Case	
 *
 * Description:	This is the main code for the birth and death use case. It gets the steps of additions/deletions of elements
 *		either from a file or uses the default steps, initializes the database, loops over all the steps and then
 *		closes the database.
 *
 *		If "do_multifile" is present on the command-line, each cycle output will be written to different files.
 *		Otherwise, it will all be written to one file.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 6, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
main(
   int argc,	/* command line argument count */
   char **argv	/* command line arguments */
)
{
  char inputFileName[MAX_FILENAME]="", *inputFileName_p = inputFileName;
  char dbname[1024]="", *dbname_p=dbname;
  hbool_t do_multifile = false;
  hbool_t verbose = false;
  hbool_t noSimplices = false;
  int i, failed=0;
  int numDims, numSteps, numFields, *theOps;
  ElemSlab_t *theSlabs;
  SAF_Field *theFields; 
  CurrentMeshParams_t currentMesh;
  IDTrackingMeshParams_t idTrackingMesh;
  DbInfo_t dbInfo;

#ifdef HAVE_PARALLEL
  MPI_Init(&argc,&argv);
#endif

  /* since we want to see whats happening make sure stdout and stderr are unbuffered */
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  STU_ProcessCommandLine(0, argc, argv,
			 "-multifile",
			 "if specified, write each cycle to a different supplemental file [false]",
			 &do_multifile,
			 "-dbname %s",
			 "specify output database name [\"birth_death.saf\"]",
			 &dbname_p,
			 "-seq %s",
			 "specify input file containing add/del sequence [\"\"]",
			 &inputFileName_p,
			 "-noSimplices",
			 "if specified, DO NOT refine some blocks into simplices (tris in 2D or tets in 3D) [false]",
			 &noSimplices,
			 "-verbose",
			 "provide verbose output during the run",
			 &verbose,
			 STU_END_OF_ARGS); 
  if (!strlen(dbname))
        strcpy(dbname, "birth_death.saf");


  saf_init(SAF_DEFAULT_LIBPROPS); /*moved this before other functions, so that the SAF_ERROR
				    reporting done by AbortThisMess would work ok*/

  /* get the sequence of additions and deletions (either from file or from default static storage) */
  GetAddDelSequence(inputFileName, &numDims, &numSteps, &theOps, &theSlabs);

  if (verbose)
    PrintAddDelSequence(numDims, numSteps, theOps, theSlabs);

  /* initialize the mesh params */ 
  InitCurrentMeshParams(&currentMesh);
  InitIDTrackingMeshParams(numDims, theSlabs[0], &idTrackingMesh);


  SAF_TRY_BEGIN
    {

      /* Issue: because we are in a try block here, all failures in any code here or in functions we call from here will send
         us to the one and only catch block at the end of this test */

      /* do some preperatory stuff for the database */
      OpenDatabase(dbname, do_multifile, numDims, &dbInfo);

      /* loop initialization stuff */
      theFields = calloc(10, sizeof(*theFields));


      /***********************************************
       ***********************************************
       *                 MAIN LOOP                   *
       ***********************************************
       ***********************************************/
      for (i = 0; i < numSteps; i++)
	{
	  numFields = 0;

	  /* update current and id tracking mesh parameters with the current step */
	  UpdateCurrentMeshParams(numDims, theOps[i], theSlabs[i], &currentMesh);
	  UpdateIDTrackingMesh(numDims, currentMesh, &idTrackingMesh);

	  /* update the current mesh relations and fields */
	  UpdateCurrentMeshData(numDims, idTrackingMesh, &currentMesh, (double) i);

	  if (verbose)
	    {
	      printf("---------------------------- step %03d ---------------------------\n", i);
	      PrintCurrentMeshParams("Current Mesh Parameters", 1, numDims,currentMesh);
	      PrintIDTrackingMeshParams("Maximal Mesh Parameters", numDims,idTrackingMesh);
	    }

	  /* refine some of the element blocks in the current mesh into blocks of simplices (tris in 2D or tets in 3D) */
	  if (!noSimplices)
	    RefineSomeBlocksNcubesToSimplices(numDims, &currentMesh, (double) i);

	  /* compute and write the current mesh, including topology relation and coordinate field */
	  WriteCurrentMesh(&dbInfo, i, numDims, currentMesh, &theFields[numFields], &numFields);

	  /* link this instance into the aggregate, update the state fields, flush the database, etc. */
	  UpdateDatabase(&dbInfo, i, numSteps, do_multifile, numFields, theFields);


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

#ifdef HAVE_PARALLEL
  /* make sure everyone returns the same error status */
  MPI_Bcast(&failed, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Finalize();
#endif

  return failed; 

}
