#ifdef SSLIB_SUPPORT_PENDING
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

#include <bitvector.h>



/*global variables*/
const long g_sizeof_long = sizeof(long);
const long g_bits_per_long = sizeof(long)*8;


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Bit Vector
 * Purpose:     calloc a bit vector of a certain size.
 * 
 * Description: calloc a bit vector of a certain size.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL December 19, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int initializeBitVector(BitVector *b, long num_bits)
{

	b->bits = num_bits;
	b->longs = (num_bits+g_sizeof_long-1L) / g_sizeof_long;  /*ceiling*/
	if(num_bits>0){
		b->data=(long *)calloc(b->longs, g_sizeof_long);
	}else{
		b->data=NULL;
	}
	return 0;
}

/* 0 -> MSB
 g_bits_per_long - 1 --> LSB
*/
#define NthMSB(mylong, n) (   (mylong) & ( (1L)<<(g_bits_per_long-(n)-1) )   )   /*the nth most significant bit of mylong*/


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Bit Vector
 * Purpose:     get a certain bit and return it.
 * 
 * Description: get a certain bit and return it.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL December 19, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int getBit(BitVector b, long which_bit)
{
	long q,r;
	long match;
	q = which_bit/g_sizeof_long;
	r = which_bit%(g_bits_per_long);
	match = b.data[q];

	return (   (match) & ( (1L)<<(g_bits_per_long-(r)-1L) )   ) ? 1 : 0;

/*	return NthMSB(match, r);*/
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Bit Vector
 * Purpose:     set a certain bit and return it.
 * 
 * Description: set a certain bit and return it.
 *
 * Parallel:    Parallel and serial behavior is identical due to use of SAF_ALL mode in all calls.
 *
 * Programmer:  Matthew O'Brien, LLNL December 19, 2001
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
long setBit(BitVector *b, long which_bit, long value)
{
	long q,r;
	long correct_bit_set;
	long correct_bit_value;
	q = which_bit/g_sizeof_long;
	r = which_bit%(g_bits_per_long);

	correct_bit_set = (1L) <<(g_bits_per_long-r-1L);	  /* 00001000 */
	correct_bit_value = value <<(g_bits_per_long-r-1L);  /* 0000v000 */
	b->data[q] = (b->data[q]) & (~correct_bit_set);   /*zeros correct bit position*/
	b->data[q] = (b->data[q]) | correct_bit_value;    /*sets correct bit position*/

#if 0
	printf("after: which_bit %i, q %i, r %i, data %i\n", which_bit,q,r,b->data[q]);
	for(i=0; i<b->longs; i++){
		printf("%s",binaryll(b->data[i]));
	}
	printf("\n");
	for(i=0; i<which_bit; i++){
		printf(" ");
	}
	printf("*\n");
#endif


	return value;
}
#endif   /* SSLIB_SUPPORT_PENDING */
