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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exampleutil.h>
#include <safP.h> /* needed for SAF_ERROR macro */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private	
 * Chapter:	Example Utilities
 * Description:	
 *---------------------------------------------------------------------------------------------------------------------------------
 */


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Constant for all zero index	
 *
 * Description:	This is a convenient constant for the all zero index.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
const RelRectIndex_t relAllZeros = {{0,0,0}};
/*DOCUMENTED*/
const AbsRectIndex_t absAllZeros = {{0,0,0}};

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Constant for all one index	
 *
 * Description:	This is a convenient constant for the all one index.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
const RelRectIndex_t relAllOnes = {{1,1,1}};
/*DOCUMENTED*/
const AbsRectIndex_t absAllOnes = {{1,1,1}};

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Define node index offsets from element index
 *
 * Description:	This constant structure defines the rectangular index offset of each node of a 1,2 or 3 dimensional cell
 *		(line, quad or hex) relative to the cell's rectangular element index.
 *
 * Issues:	The order of RectIndex_t members in this array *must* be kept consistent with the order of members of
 *		nodeLinearIndexOffset. 
 *
 *		It probably would have been more consistent with the rest of the implementation to write functions to produce
 *		these arrays for any dimension. But, that took more thought than I was willing to give.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
const RelRectIndex_t nodeRectIndexOffset[MAX_DIMS][1<<MAX_DIMS] =
{
   /* edge elements */
   {
      {{0,0,0}},
      {{1,0,0}},
      {{0,0,0}}, /* unused */
      {{0,0,0}}, /* unused */
      {{0,0,0}}, /* unused */
      {{0,0,0}}, /* unused */
      {{0,0,0}}, /* unused */
      {{0,0,0}}  /* unused */
   },
   
   /* quad elements */
   {
      {{0,0,0}},
      {{0,1,0}},
      {{1,0,0}},
      {{1,1,0}},
      {{0,0,0}}, /* unused */
      {{0,0,0}}, /* unused */
      {{0,0,0}}, /* unused */
      {{0,0,0}}  /* unused */
   },

   /* hex elements */
   {
      {{0,0,0}},
      {{0,0,1}},
      {{0,1,0}},
      {{0,1,1}},
      {{1,0,0}},
      {{1,0,1}},
      {{1,1,0}},
      {{1,1,1}}
   }
};

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Define local linear index
 *
 * Description:	This constant structure defines the linear index offset of each node of a 1,2 or 3 dimensional cell
 *		(line, quad or hex) relative to the cell's linear element index. The values in this structure are determined
 *		by the standard indexing of local node numbers over cell-types.
 *
 * Issues:	The order of members in this array *must* be kept consistent with the order of members of nodeRectIndexOffset.
 *
 *		It probably would have been more consistent with the rest of the implementation to write functions to produce
 *		these arrays for any dimension. But, that took more thought than I was willing to give.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
const RelIndex_t nodeLinearIndexOffset[MAX_DIMS][1<<MAX_DIMS] =
{
   /* edge elements */
   {
      0, /* 0 */
      1, /* 1 */
      0, /* ignored */
      0, /* ignored */
      0, /* ignored */
      0, /* ignored */
      0, /* ignored */
      0  /* ignored */
   },

   /* quad elements */
   {
      0, /* 00 */
      3, /* 01 */
      1, /* 10 */
      2, /* 11 */
      0, /* ignored */
      0, /* ignored */
      0, /* ignored */
      0  /* ignored */
   },

   /* hex elements */
   {
      3, /* 000 */
      0, /* 001 */
      7, /* 010 */
      4, /* 011 */
      2, /* 100 */
      1, /* 101 */
      6, /* 110 */
      5  /* 111 */
   }
};

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Linearize a relative rectangular index
 *
 * Description:	This function maps a linear address space onto the rectangular address space specified by the 's' argument
 *		and then computes the linear index within that space of the rectangular index 'r'. 
 *
 * Return:	The returned value is the linear index of the element or node
 *---------------------------------------------------------------------------------------------------------------------------------
 */
RelIndex_t
LinearIndex(
   int n,		/* number of dimensions in the rectangular index space */
   RelRectIndex_t s,	/* the rectangular dimensions onto which the linear address space is mapped */
   RelRectIndex_t r	/* a rectangular index for an element or node */
)
{
   int i;
   RelIndex_t dimProduct = 1;
   RelIndex_t result = 0;

   for (i = 0; i < n; i++)
   {
      if (r.idx[i] >= s.idx[i])
         return 0;
      result += (r.idx[i] * dimProduct);
      dimProduct *= s.idx[i];
   }

   return result;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Rectangularize a linear index	
 *
 * Description:	This function performs the inverse of LinearIndex(). 
 *
 * Return:	The returned value is the rectangular index of the element or node
 *---------------------------------------------------------------------------------------------------------------------------------
 */
RelRectIndex_t
RelRectIndex(
   int n,		/* number of dimensions in the rectangular index space */
   RelRectIndex_t s,	/* the rectangular dimensions onto which the linear address space is mapped */
   RelIndex_t l		/* the linear index of an element or node in the space */
)
{
   int i;
   RelIndex_t dimProduct = 1;
   RelRectIndex_t result = relAllZeros;

   /* start with maximum product of dimensions and work backwards */
   for (i = 0; i < n-1; i++)
      dimProduct *= s.idx[i];

   for (i = n-1; i >= 0; i--)
   {
      result.idx[i] = l / dimProduct;
      l %= dimProduct;
      dimProduct /= (i==0?1:s.idx[i-1]); 
   }

   return result;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Add two rectangular indexes	
 *
 * Description:	This function adds two rectangular indexes 
 *
 * Return:	The returned value is the sum of the two rectangular indexes passed in 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
RelRectIndex_t
AddRelRectIndex(
   int n,		/* number of dimensions in the rectangular index space */
   RelRectIndex_t a,	/* the first rectangular index to add */
   RelRectIndex_t b	/* the second rectangular index to add */ 
)
{
   int i;
   RelRectIndex_t result = relAllZeros;

   for (i = 0; i < n; i++)
      result.idx[i] = a.idx[i] + b.idx[i];

   return result;
}
/*DOCUMENTED*/
AbsRectIndex_t
AddAbsRectIndex(
   int n,		/* number of dimensions in the rectangular index space */
   AbsRectIndex_t a,	/* the first rectangular index to add */
   AbsRectIndex_t b	/* the second rectangular index to add */ 
)
{
   int i;
   AbsRectIndex_t result = absAllZeros;

   for (i = 0; i < n; i++)
      result.idx[i] = a.idx[i] + b.idx[i];

   return result;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Subtract rectangular indexes	
 *
 * Description:	This function subtracts the second rectangular index 'b' from the first 'a'.
 *
 * Return:	The returned value is the difference of the two rectangular indexes passed in 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
RelRectIndex_t
SubRelRectIndex(
   int n,		/* number of dimensions in the rectangular index space */
   RelRectIndex_t a,	/* the first rectangular index */
   RelRectIndex_t b	/* rectangular index to subtract */ 
)
{
   int i;
   RelRectIndex_t result = relAllZeros;

   for (i = 0; i < n; i++)
      result.idx[i] = a.idx[i] - b.idx[i];

   return result;
}
/*DOCUMENTED*/
AbsRectIndex_t
SubAbsRectIndex(
   int n,		/* number of dimensions in the rectangular index space */
   AbsRectIndex_t a,	/* the first rectangular index */
   AbsRectIndex_t b	/* rectangular index to subtract */ 
)
{
   int i;
   AbsRectIndex_t result = absAllZeros;

   for (i = 0; i < n; i++)
      result.idx[i] = a.idx[i] - b.idx[i];

   return result;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Convert absolute index to relative
 *
 * Description:	This function converts an absolute index to a relative index 
 *
 * Return:	The returned value is the relative index
 *---------------------------------------------------------------------------------------------------------------------------------
 */
RelRectIndex_t
Abs2RelIndex(
   int n,		/* number of dimensions in the rectangular index space */
   AbsRectIndex_t o,	/* the origin in absolute space */
   AbsRectIndex_t a	/* the absolute index to be converted */
)
{
   int i;
   RelRectIndex_t result = relAllZeros;
   AbsRectIndex_t preResult;

   preResult = SubAbsRectIndex(n, a, o);

   for (i = 0; i < n; i++)
      result.idx[i] = (preResult.idx[i] < 0 ? (RelIndex_t) 0 : (RelIndex_t) preResult.idx[i]);

   return result;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Convert relative index to absolute
 *
 * Description:	This function converts a relative index to an absolute index 
 *
 * Return:	The returned value is the absolute index 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
AbsRectIndex_t
Rel2AbsIndex(
   int n,		/* number of dimensions in the rectangular index space */
   AbsRectIndex_t o,	/* the origin in absolute space */
   RelRectIndex_t r	/* the relative index to be converted */
)
{

   int i;
   AbsRectIndex_t result = absAllZeros;

   for (i = 0; i < n; i++)
      result.idx[i] = (int) r.idx[i];

   result = AddAbsRectIndex(n, result, o);

   return result;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Convert an absolute index to a half-space index	
 *
 * Description:	This function converts an absolute index to a half-space index. Half spaces are indexed using the sign of the
 *		absolute indices. A positive sign on the ith index converts to a 1-bit at the ith position in the half-space
 *		index.
 *
 * Return:	The returned value is half-space index 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
HalfSpaceIndex_t
Abs2HalfSpaceIndex(
   int n,		/* number of dimensions of absolute index */
   AbsRectIndex_t a	/* the absolute index */
)
{
   int i;
   HalfSpaceIndex_t result = 0x00;

   for (i = 0; i < n; i++)
   {
      if (a.idx[i] >= 0)
         result |= (0x1 << i);
   }

   return result;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Check absolute index for in/out of given boundaries	
 *
 * Description:	This function returns True (1) if the given index is within the min/max boundaries, False (0) otherwise.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
AbsIndexIn(
   int n,		/* number of dimensions in the rectangular index space */
   AbsRectIndex_t min,	/* absolute lower bound */
   AbsRectIndex_t max,	/* absolute upper bound */ 
   AbsRectIndex_t val	/* absolute index to check */
)
{
   int i;

   for (i = 0; i < n; i++)
   {
      if (max.idx[i] == min.idx[i] || val.idx[i] < min.idx[i] || val.idx[i] >= max.idx[i])
	 return False;
   }

   return True;

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Form string for a relative index
 *
 * Description:	This function is used primarily for debugging. It returns a static string for a relative index.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
char *
RelRectIndexStr(
   int n,		/* number of dimensions in the index */
   RelRectIndex_t r 	/* the relative index to print */
)
{
   int i;
   static char result[32];

   /* put the first value in (4 chars, a relative index is always positive) */
   sprintf(result, "+%03d", r.idx[0]);

   /* put remaining values in (each additional entry is 6 chars) */
   for (i = 1; i < n; i++)
      sprintf(&result[(i-1)*6+4], ", +%03d", r.idx[i]);

   /* and, terminate it */
   result[(n-1)*6+4+1] = '\0';

   return result;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Form string for an absolute index
 *
 * Description:	This function is used primarily for debugging. It returns a static string for an absolute index.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
char *
AbsRectIndexStr(
   int n,		/* number of dimensions in the index */
   AbsRectIndex_t r 	/* the relative index to print */
)
{
   int i;
   static char result[32];

   /* put the first value in (4 chars) */
   sprintf(result, "%+04d", r.idx[0]);

   /* put remaining values in (each additional entry is 6 chars) */
   for (i = 1; i < n; i++)
      sprintf(&result[(i-1)*6+4], ", %+04d", r.idx[i]);

   /* and, terminate it */
   result[(n-1)*6+4+1] = '\0';

   return result;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Form string for a half-space index 
 *
 * Description:	This function is used primarily for debugging. It returns a static string for a half-space index.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
char *
HalfSpaceIndexStr(
   int n,		/* number of dimensions in the index */
   HalfSpaceIndex_t h	/* a half space index */
)
{
   static char result[4];
   int i;

   for (i = 0; i < n; i++)
   {
      if (h & 0x1)
         result[i] = '+';
      else
         result[i] = '-';
      h = h >> 1;
   }
   result[n] = '\0';
   return result;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Compute topology data from an element list
 *
 * Description:	This function will compute nodal-connectivity, topology relation data for a set of elements according to the
 *		element and node rectangular index numbering schemes defined by the RelRectIndex_t datatype.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
RelIndex_t *
BuildTopologyData(
   int numDims,			/* [IN] number of dimensions of the mesh for which topology is being computed */
   int numElems,		/* [IN] number of elements in the mesh for which topology is being computed */
   RelRectIndex_t sizeInElems,	/* [IN] size of the mesh in elements */
   RelRectIndex_t sizeInNodes,	/* [IN] size of mesh in nodes */
   RelIndex_t *elemList		/* [IN] the list of elements in the mesh for which topology is to be computed. Pass NULL if
				 * topology for all elements in the mesh is to be computed */
)
{
   int i, j;
   int nodesPerElem = 1 << numDims;
   RelIndex_t *topoBuf, linearElem, linearNode;
   RelRectIndex_t relRectElem, relRectNode;

   /* allocate a buffer for topology relation */
   topoBuf = (RelIndex_t *) malloc (numElems * nodesPerElem * sizeof(RelIndex_t));
   if (topoBuf == NULL)
      return NULL;

   /* construct topology relation data */
   for (i = 0; i < numElems; i++)
   {

      /* determine the element ID for which topology is needed */
      linearElem = elemList ? elemList[i] : i==0 ? 0 : linearElem + 1;

      /* compute this elem's rectangular index */
      relRectElem = RelRectIndex(numDims, sizeInElems, linearElem);

      for (j = 0; j < nodesPerElem; j++)
      {
         /* form this node's rectangular index */
	 relRectNode = AddRelRectIndex(numDims, relRectElem, nodeRectIndexOffset[numDims-1][j]);

	 /* linearize this index */
	 linearNode = LinearIndex(numDims, sizeInNodes, relRectNode);

	 /* store value to topo buffer at this element's linear index plus the linear offset of the node */ 
	 topoBuf[i * nodesPerElem + nodeLinearIndexOffset[numDims-1][j]] = linearNode;
      }
   }

   return topoBuf;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Refine rectagonal topology data into simplicialized topology data	
 *
 * Description: Given the topology for a regular array of nCubes in 2 or 3 dimensions, this function will compute and return
 *		the topology obtained by decomposing each nCube into simplices. In two dimensions, each quad is decomposed int
 *		2 triangles. In three dimensions, each hex is decomposed into 6 tets. The three dimensional version assumes
 *		a rectangularly structured block of hexahedral elements. This operation is meaningless for 1 dimension.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
RelIndex_t *
BuildSimplicializedTopologyData(
   int numDims,			/* [IN] number of dimensions of the mesh for which topology is being computed */
   int numElems,		/* [IN] number of elements in the input topology buffer */
   RelIndex_t *topoBuf		/* [IN] the input topology buffer */
)
{

   int i;
   int nodesPerInputElem = (1<<numDims);
   int nodesPerOutputElem = numDims+1;
   int outputSimplicesPerInputElem = (numDims == 2) ? 2 : 6; 
   RelIndex_t *result = NULL;

   switch (numDims)
   {
      case 1:
      {
         result = (RelIndex_t *) malloc(numElems * nodesPerInputElem * sizeof(RelIndex_t));
         memcpy(result, topoBuf, numElems * nodesPerInputElem * sizeof(RelIndex_t));
         break;
      }
      case 2: /* refining a quad into triangles */
      {
         result = (RelIndex_t *) malloc(numElems * outputSimplicesPerInputElem * nodesPerOutputElem * sizeof(RelIndex_t));
	 for (i = 0; i < numElems; i++)
	 {
	    RelIndex_t *currentElem = &topoBuf[i * nodesPerInputElem];
	    int ii = i * outputSimplicesPerInputElem * nodesPerOutputElem;

	    /* tri 0 */
	    result[ii+0] = currentElem[0];
	    result[ii+1] = currentElem[1];
	    result[ii+2] = currentElem[2];

	    /* tri 1 */
	    ii += nodesPerOutputElem;
	    result[ii+0] = currentElem[0];
	    result[ii+1] = currentElem[2];
	    result[ii+2] = currentElem[3];
	 }
         break;
      }
      case 3: /* refining a hex into tets */
      {
         result = (RelIndex_t *) malloc(numElems * outputSimplicesPerInputElem * nodesPerOutputElem * sizeof(RelIndex_t));
	 for (i = 0; i < numElems; i++)
	 {
	    RelIndex_t *currentElem = &topoBuf[i * nodesPerInputElem];
	    int ii = i * outputSimplicesPerInputElem * nodesPerOutputElem;

	    /* tet 0 */
	    result[ii+0] = currentElem[0];
	    result[ii+1] = currentElem[4];
	    result[ii+2] = currentElem[5];
	    result[ii+3] = currentElem[7];

	    /* tet 1 */
	    ii += nodesPerOutputElem;
	    result[ii+0] = currentElem[0];
	    result[ii+1] = currentElem[3];
	    result[ii+2] = currentElem[5];
	    result[ii+3] = currentElem[7];

	    /* tet 2 */
	    ii += nodesPerOutputElem;
	    result[ii+0] = currentElem[0];
	    result[ii+1] = currentElem[1];
	    result[ii+2] = currentElem[3];
	    result[ii+3] = currentElem[5];

	    /* tet 3 */
	    ii += nodesPerOutputElem;
	    result[ii+0] = currentElem[3];
	    result[ii+1] = currentElem[5];
	    result[ii+2] = currentElem[6];
	    result[ii+3] = currentElem[7];

	    /* tet 4 */
	    ii += nodesPerOutputElem;
	    result[ii+0] = currentElem[1];
	    result[ii+1] = currentElem[3];
	    result[ii+2] = currentElem[5];
	    result[ii+3] = currentElem[6];

	    /* tet 5 */
	    ii += nodesPerOutputElem;
	    result[ii+0] = currentElem[1];
	    result[ii+1] = currentElem[2];
	    result[ii+2] = currentElem[3];
	    result[ii+3] = currentElem[6];
	 }
	 break;
      }
   }

   return result;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Compute coordinate data from node list
 *
 * Description:	This function will compute coordinate data for a set of nodes
 *---------------------------------------------------------------------------------------------------------------------------------
 */
float *
BuildCoordinateData(
   int numDims,			/* [IN] number of dimensions of the mesh for which coordinate data is being computed */
   int numNodes,		/* [IN] number of nodes in the mesh for which coordinate data is being computed */
   RelRectIndex_t sizeInNodes,	/* [IN] size of mesh in nodes */
   AbsRectIndex_t lowerBounds,	/* [IN] lower bounds of mesh coordinates */
   RelIndex_t *nodeList		/* [IN] the list of nodes in the mesh for which coordinate data is to be computed. Pass NULL if
				 * coordinate data for all nodes in the mesh is to be computed */
)
{
   int i, j;
   float *coordBuf;
   RelIndex_t linearNode;
   RelRectIndex_t relRectNode;
   AbsRectIndex_t absRectNode;

   /* allocate a buffer for the coordinate array */
   coordBuf = (float *) malloc(numNodes * numDims * sizeof(float));
   if (coordBuf == NULL)
      return NULL;

   /* fill in the coordinate array */
   for (i = 0; i < numNodes; i++)
   {
      linearNode = nodeList ? nodeList[i] : i==0 ? 0 : linearNode + 1;

      relRectNode = RelRectIndex(numDims, sizeInNodes, linearNode);
      absRectNode = Rel2AbsIndex(numDims, lowerBounds, relRectNode);

      for (j = 0; j < numDims; j++)
         coordBuf[i * numDims + j] = (float) absRectNode.idx[j];
   }

   return coordBuf;

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Compare 2 node IDs
 *
 * Description:	This function is used as the compare function for the qsort routine
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
CompareNodeIDs(
   const void *arg1,	/* first arg */ 
   const void *arg2	/* second arg */ 
)
{
   RelIndex_t val1 = *(const RelIndex_t *) arg1;
   RelIndex_t val2 = *(const RelIndex_t *) arg2;

   if (val1 < val2)
      return -1;
   if (val1 > val2)
      return 1;
   return 0;

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Compute unique nodes 
 *
 * Description:	Given a topology relation, this function will compute the unique set of nodes referenced by the relation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
UniqueNodes(
   int topoBufLength,		/* [IN] number of node refernces in the topology relation data buffer */
   RelIndex_t *topoBuf,		/* [IN] the topology relation data */
   int *uniqueNodeCount,	/* [OUT] the number of unique nodes in the topology relation data buffer */
   RelIndex_t **uniqueNodeList	/* [OUT] the unique set of nodes in the topology relation data buffer */
)
{

   UniqueList(topoBufLength, topoBuf, sizeof(RelIndex_t), CompareNodeIDs, uniqueNodeCount, (void **) uniqueNodeList, NULL);

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Compute unique list from given list 
 *
 * Description:	Given a list of items of specified type size and which can be compared using comparison function, this function
 *		computes a new list of only the unique items.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
UniqueList(
   int listLength,		/* [IN] number of items in the input list */
   void *theList,		/* [IN] the list of items */
   int itemSize,		/* [IN] the size of each item */
   CompareFunc_t compareFunc,	/* [IN] function for comparing values in a list */
   int *uniqueLength,		/* [OUT] the number of unique items in the list */
   void **uniqueList,		/* [OUT] the unique list of items */
   int **mapList		/* [OUT] optional map (of length listLength) of position of each entry in input list in the
   				   unique list */
)
{
   int i, count;
   char *sortedBuf, *resultBuf;
   int *theMap;

   /* return immediately for the degenerate case */
   if (listLength <= 0)
   {
      *uniqueLength = 0;
      *uniqueList = NULL;
      if (mapList)
         *mapList = NULL;
      return;
   }

   /* copy the topology buffer so we can sort the data without changing the topology relation data */
   sortedBuf = (char *) malloc((size_t) (listLength * itemSize));
   memcpy((void *) sortedBuf, theList, (size_t)(listLength * itemSize));

   /* return almost as quickly for the really simply case */
   if (listLength == 1)
   {
      *uniqueLength = 1;
      *uniqueList = (void *) sortedBuf;
      if (mapList)
      {
         theMap = (int *) malloc(sizeof(int));
	 *theMap = 0;
         *mapList = theMap;
      }
      return;
   }

   /*
    * beyond this point, sortedTopoBuf has at least two values in it
    */

   /* sort the data */
   qsort((void*) sortedBuf, (size_t) listLength, (size_t) itemSize, compareFunc); 

   /* scan sorted array for value transitions */
   count = (*compareFunc)(sortedBuf, sortedBuf+itemSize) ? 2 : 1;
   for (i = 2; i < listLength; i++)
      count += ((*compareFunc)(sortedBuf+i*itemSize, sortedBuf+(i-1)*itemSize) ? 1 : 0);

   /* return early if count is equal to listLength */
   if (count == listLength)
      resultBuf = sortedBuf;
   else
   {

      /* ok, we know how many there are, so allocate the return array */
      resultBuf = (char *) malloc((size_t)(count * itemSize));

      /* scan sorted array, again, and copy the unqiue values to the result buffer */
      count = (*compareFunc)(sortedBuf, sortedBuf+itemSize) ? 2 : 1;
      memcpy(resultBuf, sortedBuf,(size_t) itemSize);
      if (count == 2)
         memcpy(resultBuf+itemSize, sortedBuf+itemSize, (size_t) itemSize);
      for (i = 2; i < listLength ; i++)
      {
         if ((*compareFunc)(sortedBuf+i*itemSize, sortedBuf+(i-1)*itemSize))
         {
            memcpy(resultBuf+count*itemSize, sortedBuf+i*itemSize, (size_t) itemSize);
	    count++;
         }
      }
   }

   if (sortedBuf != resultBuf)
      free(sortedBuf);

   /* return the results */
   if (mapList)
   {
      theMap = (int *) malloc(count * sizeof(int));
      for (i = 0; i < count; i++)
	 theMap[i] = IndexOfItem(resultBuf, count, itemSize, compareFunc, resultBuf+i*itemSize, 1);
      *mapList = theMap;
   }
   *uniqueLength = count;
   *uniqueList = resultBuf;
   return;

}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Return index of a given, unqiue value in a namespace map
 *
 * Description:	This function assumes the namespace map is sorted *and* that there are no duplicate entries in the map. 
 *		It performs a binary search on the map for the given value.
 *
 * Return:	the index in the map of the given value 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int 
IndexOfValue(
   RelIndex_t *theMap,	/* the map data */
   int mapLength,	/* number of entries in the map */
   RelIndex_t mapEntry	/* the entry in the map we are looking for */ 
)
{
   return IndexOfItem(theMap, mapLength, sizeof(RelIndex_t), CompareNodeIDs, (void *) &mapEntry, 1);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Return index of a given, unqiue value in a namespace map
 *
 * Description:	This function assumes the namespace map is sorted *and* that there are no duplicate entries in the map. 
 *		It performs a binary search on the map for the given value.
 *
 * Return:	the index in the map of the given value 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int 
IndexOfItem(
   void *theList,		/* [IN] the list to search for value in */
   int listLength,		/* [IN] number of entries in the list */
   int itemSize,		/* [IN] size of an item in the list */
   CompareFunc_t compareFunc,	/* [IN] function for comparing values in the list */
   void *theEntry,		/* [IN] pointer to a copy of the entry in the list we are looking for */ 
   int sortedFlag		/* [IN] flag if theList is sorted. A binary search will be used if this is non-zero */
)
{
   if (sortedFlag)
   {
      int done = 0;
      int i = listLength>>1;
      int di = MAX(listLength>>2, 1); 
      char *listP = (char *) theList;

      while (!done)
      {
         int compare = (*compareFunc)(theEntry, listP+i*itemSize);
         if (!compare)
	    done = 1;
         else
         {
	    i += (compare * di);
	    if (i >= listLength || i < 0)
	       return -1;
	    di = MAX(di>>2, 1);
         }
      }
      return i;
   }
   else
   {
      int done = 0;
      int i = 0;
      char *listP = (char *) theList;

      while (!done)
      {
         int compare = (*compareFunc)(theEntry, listP+i*itemSize);
         if (!compare)
	    done = 1;
         else
	 {
	    i++;
	    if (i >= listLength || i < 0)
	       return -1;
	 }
      }
      return i;
   }
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Convert range of relation data
 *
 * Description:	Given relation data for any relation and a mapping between the range of that relation and the desired, new
 *		range, this function will change the range of the relation data to the new range.
 *
 * Return:	the relation data in terms of the new range
 *
 * Issues:	We would like to write this function and others that have to do with remapping relation data in such a way that
 *		it could be migrated into the SAF library with ease. In preparation for this, we've tried to name this function
 *		and its arguments as well as include arguments in such a way to address the more general setting.
 *		
 *		The range of a relation is more generally, a /namespace/. A namespace has a type, such as int, float, string,
 *		as well as being compact or not. Furthermore, a namespace can be implicit (e.g. {0...N-1} or explicit
 *		(e.g. {1,2,4,6,9} or {"banana","grape","orange"}). In addition, a namespace would also define a comparison
 *		function for sorting members of the namespace.
 *
 *		In SAF, we don't yet fully support the notion of a general namespace. However, we do support collections and
 *		collections have indexing schemes associated with them.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
RelIndex_t *
ConvertRange(
   RelIndex_t *relBuf,		/* [IN/OUT] the relation data buffer to be converted. Ordinarily, this data will *not* be
   				 * modified. However, if the `inPlace' argument is non-zero, this buffer will be modified. */ 
   int relBufLength,		/* [IN] length of the relation data buffer */
   int currentNS,		/* [IN] identifies the range of the data in relBuf. Currently, we *do*not* have an object that
				 * represents a range or, more generally, a namespace. So, we just use a simple int. For further
				 * details, please be sure to read the comments for the newNS and nsMapRange args. */
   int newNS,			/* [IN] identifies the new, desired range of the data in relBuf. While the client may choose
				 * any int to pass here as well as for currentNS, it is a requirement that the ints for
				 * both the currentNS and newNS be different. If there were the same, it would imply the
				 * desired range is the same as the current range and no work need be done. */
   RelIndex_t *nsMap,		/* [IN] a map between the namespaces identified by the currentNS and newNS arguments. Remember,
				 * the relation data in relBuf is, a slew of references (e.g. points) in the currentNS
				 * namespace and this function is supposed to convert those to references in the newNS namespace.
				 * The nsMap argument is a mapping which takes points in one namespace and returns a point
				 * in the other namespace. */
   int nsMapRange,		/* [IN] identifies which of the two namespaces, currentNS or newNS, the range of the nsMap is.
				 * If the range of nsMap is the same as currentNS, it means we actually need to invert it
				 * to complete the mapping. However, we don't actually compute an inverse map. Instead we
				 * sort it and use a binary search. */
   int nsMapLength,		/* length of the map */
   int nsMapSorted,		/* [IN] boolean to indicate if the map is sorted or not */
   int inPlace			/* [IN] boolean to indicate if the operation should be done in place, modifying the data in
				 * relBuf */
)
{
   int i;
   RelIndex_t *newRelBuf;

   /* some simple checks */
   if (relBuf == NULL || nsMap == NULL)
      return NULL;

   if (nsMapRange != newNS && nsMapRange != currentNS)
      return NULL;

   if (currentNS == newNS)
      return relBuf;

   /* allocate the output buffer, if necessary */
   if (!inPlace)
      newRelBuf = (RelIndex_t *) malloc(relBufLength * sizeof(RelIndex_t));
   else
      newRelBuf = relBuf;
   if (newRelBuf == NULL)
      return NULL;

   /* simple case is when nsMapRange is the new namespace */
   if (nsMapRange == newNS)
   {
      for (i = 0; i < relBufLength; i++)
	 newRelBuf[i] = nsMap[relBuf[i]];
   }
   else
   {
      int allocatedMap = 0;
      RelIndex_t *theMap;

      /* if the map isn't sorted, we need to allocate a temporary one and sort it */
      if (!nsMapSorted)
      {
	 theMap = (RelIndex_t *) malloc(nsMapLength * sizeof(RelIndex_t));
	 if (theMap == NULL)
	    return NULL;
	 memcpy(theMap, nsMap, nsMapLength * sizeof(RelIndex_t));
	 qsort(theMap, (size_t) nsMapLength, sizeof(RelIndex_t), CompareNodeIDs);
	 allocatedMap = 1;
      }
      else
	 theMap = nsMap;

      /* do the inverse mapping */
      for (i = 0; i < relBufLength; i++)
	 newRelBuf[i] = (RelIndex_t) IndexOfValue(theMap, nsMapLength, relBuf[i]);

      if (allocatedMap)
	 free(theMap);
   }

   return newRelBuf;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Default interpolating dofs	
 *
 * Description:	This function returns default interpolating dofs. It is a mono-, bi- or tri-linear analytic function that
 *		is also modulated by the current time step.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
static float
DefaultFieldFunc(
   int numDims,			/* [IN] number of coordinate dimensions in the mesh */
   float dofCoords[MAX_DIMS],	/* [IN] coordinates of the interpolating dof to compute a value for */
   float timeStep		/* [IN] current time step of the simulation being simulated */
)
{
   int i;
   float result = 1.0;
   for (i = 0; i < numDims; i++)
      result *= dofCoords[i];
   return result * timeStep;
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Build the dofs for a field	
 *
 * Description:	This function will build interoplating dofs for piecewise linear (/nodal/ or /node/-/centered/) or piecewise
 *		constant (/zonal/ or /zone/-/centered/) field. Given the size of the mesh (in nodes or elements) and its
 *		lower-bounds in each dimension, this function will compute the given analytic function for each node or element
 *		in the list specified by memberList or, if memberList is NULL, for all nodes or elements in the mesh. 
 *
 * Return:	the computed field data 
 *
 * Issues:	We wrote this function to accept any analytic function to evaluate. In this way, a use-case client can invent
 *		new kinds of fields relatively easily.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
float *
BuildInterpolatingDofs(
   int fieldType,		/* [IN] flag indicating the type of field to compute dofs for. Pass PIECEWISE_LINEAR or
   				    PIECEWISE_CONSTANT. */
   int numDims,			/* [IN] number of dimensions in the mesh. */
   RelRectIndex_t size,		/* [IN] size of the mesh, in nodes if PIECEWISE_LINEAR or elements if PIECEWISE_CONSTANT. */ 
   AbsRectIndex_t lowerBounds,	/* [IN] lower bounds of the mesh. */
   int numMembers,		/* [IN] number of members in memberList. Ignored if memberList is NULL. */
   RelIndex_t *memberList,	/* [IN] the list of members to compute dofs for. This is a list of node ids if PIECEWISE_LINEAR
   				   or a list of element ids if PIECEWISE_CONSTANT. */
   float timeStep,		/* [IN] the time-step of the simulation. This permits time-variation of successive fields
   				   computed by this function */
   AnalyticDofFunc_t analyticFunc	/* [IN] the function used to evaluate the dofs for each member. If NULL, a default
   					   function will be used. */
)
{

   int i;
   float *fieldBuf;
   RelIndex_t linearMember;

   /* compute number of members from mesh size, if no memberList given */
   if (!memberList)
   {
      RelRectIndex_t maxRectMember;

      maxRectMember = SubRelRectIndex(numDims, size, relAllOnes);
      numMembers = LinearIndex(numDims, size, maxRectMember) + 1;
   }

   /* allocate a buffer for the field data */
   fieldBuf = (float *) malloc(numMembers * sizeof(float));
   if (fieldBuf == NULL)
      return NULL;

   /* fill in the field data */
   for (i = 0; i < numMembers; i++)
   {
      int j;
      float dofCoord[MAX_DIMS];
      RelRectIndex_t relRectMember;
      AbsRectIndex_t absRectMember;

      linearMember = memberList ? memberList[i] : i==0 ? 0 : linearMember + 1;
      relRectMember = RelRectIndex(numDims, size, linearMember);
      absRectMember = Rel2AbsIndex(numDims, lowerBounds, relRectMember);

      /* determine floating point coordinates at which to compute a dof */
      for (j = 0; j < numDims; j++)
         dofCoord[j] = fieldType == PIECEWISE_CONSTANT ? (float) absRectMember.idx[j] + 0.5 : (float) absRectMember.idx[j];

      fieldBuf[i] = analyticFunc ? (*analyticFunc)(numDims, dofCoord, timeStep) : DefaultFieldFunc(numDims, dofCoord, timeStep) ;

   }

   return fieldBuf;

}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Print element history
 *
 * Description:	This function prints the history buffer accumulated for an element. This function should only be called from
 *		a single processor.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
PrintElementHistory(
   ElementHistory_t *history,	/* thie history information to print */
   int length,			/* the number of entries in the history buffer */
   int globalElemID,		/* the global element ID whose history is in the history buffer */
   const char *heading		/* a heading to print before the history is printed */
)
{
   int i;

   printf("%s\n", heading);
   printf("History for Global Element ID %d\n", globalElemID);
   printf("  owner   |  local ID  | dof value\n");
   printf("----------|------------|----------\n");

   for (i = 0; i < length; i++)
      printf("   %03d    |   %05d    | %f\n", history[i].processorID, history[i].localElemID, history[i].dofValue);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private
 * Chapter:	Example Utilities 
 * Purpose:	Abort using SAF_ERROR	
 *
 * Description:	This function is a wrapper for SAF's SAF_ERROR macro. This keeps us from having to include safP.h in the examples
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
AbortThisMess(const char *msg /* the abort message */)
{
   SAF_ENTER(AbortThisMess, /*void*/);
   SAF_ERROR(/*void*/,_saf_errmsg("%s",msg));
   SAF_LEAVE(/*void*/);
}
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public	
 * Chapter:	Example Utilities
 * Purpose:	Update the database to the current step	
 *
 * Description:	This function performs a number of tasks involved in updating the database to the current step in the
 *		sequence of additions and deletions.
 *
 * Issues:	We are forced to create a new state field each time this function is called. This can be viewed as either 
 *		approrpriate or counter-intuitive depending on how the SAF client /views/ its data. Certainly, since each step
 *		in the sequence of additions and deletions changes the mesh, the state output by the client is different on each
 *		step. This follows the strict definition of state currently supported by SAF's states and suites API. However,
 *		if the list of fields that are output *do*not*change* and only their base-space varies with time, is the state
 *		output by the client, in the eyes of the client, really different? We could support this somewhat looser
 *		definition of state by instead of associating with each state the fields on the mesh instances, we used fields on
 *		the aggregate. However, this would require a minor alteration to our current interpretation of inhomogeneous fields.
 *		If interested, we can persue further.
 *
 * 		Because the fields from each state are defined on a different base-space, we need to define a new suite
 *		for every step in the simulation. This will be corrected when SAF supports sub-setting on suites. Then, we'll
 *		be able to define an inhomogenous field on the suite for its different pieces.
 *
 * Programmer:	Mark Miller, LLNL, Tue Dec 11, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
UpdateDatabase(
   DbInfo_t *dbInfo,		/* [IN/OUT] database info object (currentFile member can be modified) */
   int stepNum,			/* [IN] the current step number in the sequence starting from zero */
   int numSteps,		/* [IN] total number of steps to be output (>=1) */
   hbool_t do_multifile,	/* [IN] boolean to indicate if each step will go to a different supplemental file */
   int numFields,		/* [IN] the number of fields on the mesh */
   SAF_Field *theFields		/* [IN] array of length numFields of the field handles */
)
{
  char tmpName[32];
  static int stepZero = 0;
  int i;
  float stepVal;
  SAF_FieldTmpl *fieldTmpls;
  SAF_StateGrp currentStateGrp;
  SAF_StateTmpl currentStateTmpl;
  SAF_Suite currentSuite;
  SAF_Rel rel;
  SAF_Unit usec;

  /* local vars obtained from dbinfo object */
  SAF_Db *db = dbInfo->db; 
  SAF_Set aggregateMesh = dbInfo->mainMesh;
  SAF_Set currentMesh = dbInfo->currentMesh;
  SAF_Db *stepFile = dbInfo->currentFile;

  /* link currentMesh into aggregate at the current position */
  saf_declare_subset_relation(SAF_ALL, db, &aggregateMesh, &currentMesh, SAF_COMMON(SAF_SELF(db)), SAF_TUPLES,
			      H5I_INVALID_HID, NULL, H5I_INVALID_HID, NULL, &rel);
  saf_write_subset_relation(SAF_ALL, &rel, SAF_INT, &stepNum, H5I_INVALID_HID, NULL, stepFile); 

  /* get the handle for the seconds unit */
  saf_find_one_unit(db, "second", &usec);

  /* obtain all the field templates for all the fields */
  fieldTmpls = calloc((size_t)numFields, sizeof *fieldTmpls);
  for (i = 0; i < numFields; i++)
    saf_describe_field(SAF_ALL, theFields+i, fieldTmpls+i, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		       NULL, NULL, NULL, NULL, NULL, NULL);

  /* get the handle for the seconds unit */
  saf_find_one_unit(db, "second", &usec);

  /* create the suite */
  sprintf(tmpName, "stateSuite_%03d", stepNum);
  saf_declare_suite(SAF_ALL, db, tmpName, &currentMesh, NULL, &currentSuite);

  /* create a new state template */
  sprintf(tmpName, "stateTmpl_%03d", stepNum);
  saf_declare_state_tmpl(SAF_ALL, db, tmpName, numFields, fieldTmpls, &currentStateTmpl);
  free(fieldTmpls);

  /* create a new state group */
  sprintf(tmpName, "stateGrp_%03d", stepNum);
  saf_declare_state_group(SAF_ALL, db, tmpName, &currentSuite, &currentMesh, &currentStateTmpl, SAF_QTIME, 
			  &usec, SAF_FLOAT, &currentStateGrp);

  /* Issue: we always write to the 0'th index of this new suite */
  stepVal = (float) stepNum;
  saf_write_state(SAF_ALL, &currentStateGrp, stepZero, &currentMesh, SAF_FLOAT, (void*) &stepVal, theFields);


  /* work to do for the next step if there is a next step */
  if (stepNum + 1 < numSteps)
    {
	dbInfo->lastMesh = dbInfo->currentMesh;

      /* create a new supplemental file */
      if (do_multifile) {
          SAF_DbProps *dbprops = saf_createProps_database();
          saf_setProps_Clobber(dbprops);
	  sprintf(tmpName,"step_%03d", stepNum+1);
          stepFile = saf_open_database(tmpName, dbprops);
          saf_freeProps_database(dbprops);

	  /* update the currentFile handle in the dbinfo object */
	  dbInfo->currentFile = stepFile;
	}
    }

  /* flush the database */ 
  saf_update_database(db);

  /* keep track of the last mesh */
  dbInfo->lastMesh = dbInfo->currentMesh;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public	
 * Chapter:	Example Utilities
 * Purpose:	Close the database
 *
 * Description:	This function performs any tasks associated with closing the database. Currently, the only thing this does
 *		is call saf_close_database().
 *
 * Programmer:	Mark Miller, LLNL, Thu Dec 13, 2001 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
CloseDatabase(DbInfo_t dbInfo /* database info object */)
{
   saf_close_database(dbInfo.db);
}

