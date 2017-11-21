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
#ifndef _EXAMPLEUTIL_H
#define _EXAMPLEUTIL_H

#include <saf.h>

#define True	1
#define False	0

#define MAP_SORTED_TRUE		True
#define MAP_SORTED_FALSE	False
#define IN_PLACE_TRUE		True
#define IN_PLACE_FALSE		False

#define NAMESPACE_GLOBAL	0
#define NAMESPACE_LOCAL		1

#define PIECEWISE_CONSTANT	0
#define PIECEWISE_LINEAR	1

#define UNIQUE_MAP_TAG	100
#define UNIQUE_LIST_TAG	101

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Set maximum number of dimensions of a mesh
 *
 * Description:	This constant specifies the maximum number of dimensions, logical *and* geometric, a mesh produced by the
 *		test client will have.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_DIMS	3

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Compute absolute value	
 *
 * Description:	This macro returns the absolute value of its operand. 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define ABS(X)		((RelIndex_t) ((X)>0?(X):-(X)))

#ifdef MAX
#undef MAX
#endif
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Compute maximum value	
 *
 * Description:	This macro returns the maximum value of its operands
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX(X,Y)	((X)>(Y)?(X):(Y))	

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Define an element slab datatype 
 *
 * Description:	This datatype defines the /element/slab/ datatype. It is an array of MAX_DIMS, signed integers representing the
 *		number of elements in each dimension and the relative end (+ or -) of the axial dimension it is associated with. 
 *		For example, in the one dimensional, 3 element mesh below
 *
 *		                +---+---+---+
 *		   (-x) <---    | 2 | 1 | 0 |   ---> (+x)
 *		                +---+---+---+
 *
 *		an ElemSlab_t value of -2,0,0 represents the element slab consisting of elements 2 and 1.
 *		Likewise, for the two dimensional, 6 element mesh below
 *
 *                                  (+y)
 *		                     ^
 *		                     |
 *
 *		                +---+---+---+
 *		                | 3 | 4 | 5 |
 *		   (-x) <---    +---+---+---+   ---> (+x)
 *		                | 2 | 1 | 0 |
 *		                +---+---+---+
 *
 *		                     |
 *		                     v
 *                                  (+y)
 *
 *		an ElemSlab_t value of -2,+1,0 represents the element slab consisting of elements 1,2,3,4 and 5. That is,
 *		it is the /union/ of the 2 layers of elements at the left (-) end of the X axis and the one layer of 
 *		elements at the top (+) end of the Y axis. This specification generalizes to any number of dimensions.
 *
 *		When combined with a second ElemSlab_t value representing an /origin/, we can represent each instance
 *		of the current mesh.
 *
 *		If the mesh is less than MAX_DIMS in dimension, the remaining numbers in an ElemSlab_t value are simply ignored.
 *		They are neither set nor used.
 *
 *		We're wrapping this in a struct to get the benefit of struct copy.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef	struct {
   int idx[MAX_DIMS];
} ElemSlab_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Define relative index datatype 
 *
 * Description:	A relative index datatype is an unsigned integer
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef unsigned int RelIndex_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Define half-space index datatype 
 *
 * Description:	A half-space index datatype is defined to be same as a RelIndex_t
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef RelIndex_t HalfSpaceIndex_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Define relative rectangular index datatype 
 *
 * Description:	A relative rectangular index datatype is an n-tuple necessary to identify any element or node in a structured grid.
 *
 * Issues:	We're wrapping this in a struct to get the benefit of struct copy.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct {
   RelIndex_t idx[MAX_DIMS];
} RelRectIndex_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Define absolute rectangular index datatype 
 *
 * Description:	An absolute rectangular index datatype is an n-tuple necessary to identify the absolute coordinates of 
 *		any element or node in a structured grid.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef ElemSlab_t AbsRectIndex_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Struct for half-space element lists 
 *
 * Description:	This structure defines the element lists used to represent half-spaces
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct _halfSpace {
   int allocated;		/* number of entries in element list allocated */
   int count;			/* number of entries in element list actually in use */
   RelIndex_t *elemList;	/* the list of elements */
} HalfSpace_t;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Define adaptive indexing datatype	
 *
 * Description:	Any element in a uniform, pyramid of resolution can be specified by its level, row and column within
 *		the level. The level specifies the size of the element, 1 / 2^l while the row and column specify its position.
 *		We use unsigned char here because we're not going to create very large pyramids.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef SSLIB_SUPPORT_PENDING


DSL_TYPE_DEFINE(LRCIndex_t,
struct {
   unsigned char l;	/* level in pyramid of resolution */
   unsigned char r;	/* row within a level in the pyramid */
   unsigned char c;	/* column within a level in the pyramid */
});
#endif /*SSLIB_SUPPORT_PENDING*/


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Dynamic Load Balance Use Case 
 * Purpose:	Struct for element history 
 *
 * Description:	This structure houses all the information we use when printing the history of an element. 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct _elementHistory {
   int processorID;
   RelIndex_t localElemID;
   float dofValue;
} ElementHistory_t;


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Function type for computing dofs 
 *
 * Description:	This function type is used to compute the dofs for piecewise linear or piecewise constant fields 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef float (*AnalyticDofFunc_t) (int numDims, float dofCoords[MAX_DIMS], float timeStep);

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Function type comparing values in a list
 *
 * Description:	This function type is used to compare values in a list
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef int (*CompareFunc_t)(const void*,const void*);



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Birth and Death Use Case 
 * Purpose:	Struct for database stuff 
 *
 * Description:	This structure houses all the database objects routinely used by various functions.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef struct _DbInfo {
  SAF_Db *db;
  SAF_Set mainMesh;
  SAF_Set currentMesh;
  SAF_Set lastMesh;
  SAF_Cat nodes;
  SAF_Cat elems;
  SAF_Cat blocks;
  SAF_Db *currentFile;
} DbInfo_t;




extern const RelRectIndex_t relAllZeros;
extern const AbsRectIndex_t absAllZeros;
extern const RelRectIndex_t relAllOnes;
extern const AbsRectIndex_t absAllOnes;
extern const RelRectIndex_t nodeRectIndexOffset[MAX_DIMS][1<<MAX_DIMS];
extern const RelIndex_t     nodeLinearIndexOffset[MAX_DIMS][1<<MAX_DIMS];

extern int 		CompareNodeIDs(const void *arg1, const void *arg2);
extern RelIndex_t	LinearIndex(int n, RelRectIndex_t s, RelRectIndex_t r);
extern RelRectIndex_t	RelRectIndex(int n, RelRectIndex_t s, RelIndex_t l);
extern RelRectIndex_t	AddRelRectIndex(int n, RelRectIndex_t a, RelRectIndex_t b);
extern AbsRectIndex_t	AddAbsRectIndex(int n, AbsRectIndex_t a, AbsRectIndex_t b);
extern RelRectIndex_t	SubRelRectIndex(int n, RelRectIndex_t a, RelRectIndex_t b);
extern AbsRectIndex_t	SubAbsRectIndex(int n, AbsRectIndex_t a, AbsRectIndex_t b);
extern RelRectIndex_t	Abs2RelIndex(int n, AbsRectIndex_t o, AbsRectIndex_t a);
extern AbsRectIndex_t	Rel2AbsIndex(int n, AbsRectIndex_t o, RelRectIndex_t r);
extern HalfSpaceIndex_t Abs2HalfSpaceIndex(int n, AbsRectIndex_t a);
extern int 		AbsIndexIn(int n, AbsRectIndex_t min, AbsRectIndex_t max, AbsRectIndex_t val);
extern char* 		RelRectIndexStr(int n, RelRectIndex_t r);
extern char*		AbsRectIndexStr(int n, AbsRectIndex_t r);
extern char*		HalfSpaceIndexStr(int n, HalfSpaceIndex_t h);
extern RelIndex_t*	BuildTopologyData(int numDims, int numElems, RelRectIndex_t sizeInElems, RelRectIndex_t sizeInNodes,
			   RelIndex_t *elemList);
extern RelIndex_t*	BuildSimplicializedTopologyData(int numDims, int numElems, RelIndex_t *topoBuf);
extern float*		BuildCoordinateData(int numDims, int numNodes, RelRectIndex_t sizeInNodes, AbsRectIndex_t lowerBounds,
			   RelIndex_t *nodeList);
extern void		UniqueNodes(int topoBufLength, RelIndex_t *topoBuf, int *uniqueNodeCount, RelIndex_t **uniqueNodeList);
void			UniqueList(int listLength, void *theList, int itemSize, CompareFunc_t compareFunc, int *uniqueLength,
			   void **uniqueList, int **mapList);
extern int		IndexOfValue(RelIndex_t *theMap, int mapLength, RelIndex_t mapEntry);
extern int		IndexOfItem(void *theList, int listLength, int itemSize, CompareFunc_t compareFunc, void *theEntry,
			   int sortedFlag);
extern RelIndex_t *	ConvertRange(RelIndex_t *relBuf, int relBufLength, int currentNS, int newNS, RelIndex_t *nsMap,
			   int nsMapRange, int nsMapLength, int nsMapSorted, int inPlace);
extern float *		BuildInterpolatingDofs(int fieldType, int numDims, RelRectIndex_t size, AbsRectIndex_t lowerBounds,
			   int numMembers, RelIndex_t *memberList, float timeStep, AnalyticDofFunc_t analyticFunc);
extern void		PrintElementHistory(ElementHistory_t *history, int length, int elemID, const char *heading);
extern void		AbortThisMess(const char *msg);


void UpdateDatabase( DbInfo_t *dbInfo, int stepNum, int numSteps, hbool_t do_multifile,
		     int numFields, SAF_Field *theFields );
void CloseDatabase(DbInfo_t dbInfo);

#endif /* _EXAMPLEUTIL_H */
