/*
 * Copyright(C) 2004-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 2004-2005 Sandia Corporation.
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
 *     Robb P. Matzke              LLNL
 *     Eric A. Illescas            SNL
 * 
 * Acknowledgements:
 *     Mark C. Miller              LLNL - Design input
 * 
 */
#include "sslib.h"
SS_IF(obj);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Objects
 * Description: The SSlib /Object/ class is the base class for a variety of entities in SSlib, including property lists,
 *              handles to persistent objects (a.k.a., "links"), the persistent objects themselves, etc.  What all of these
 *              have in common is some class variables, instance variables, and methods which are defined by the ss_obj_t
 *              interface.
 *
 *              All SSlib objects contain a magic number at the beginning of the object's memory. See [Magic Numbers] for
 *              details.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Objects
 * Purpose:     Interface initializer
 *
 * Description: Initializes the object interface.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, July 31, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_obj_init(void)
{
    SS_ENTER_INIT;
    /* Nothing to do */
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Objects
 * Purpose:     Create a new object
 *
 * Description: This function creates a new object by initializing the object to all zero (or the contents of INIT) and then
 *              filling in the initial ss_obj_t member.  The SS_OBJ_NEW() macro is a convenience for this function.  It is
 *              valid for INIT and OBJ to point to the same memory, in which case just the ss_obj_t stuff is initialized.
 *              Objects should generally be created by calling the constructor for the object's class, which then usually
 *              calls this function.
 *
 * Return:      Returns a pointer to the object on success; the null pointer on failure.  If OBJ is non-null then it becomes
 *              the successful return value, otherwise a new object is allocated. On failure, the variables related to the
 *              ss_obj_t class are zeroed even if the caller supplied the memory.
 *
 * Parallel:    Indpendent
 *
 * Programmer:  Robb Matzke
 *              Friday, June 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_obj_t *
ss_obj_new(ss_obj_t *obj,               /* Optional memory for the new object */
           unsigned magic,              /* Magic number for the new objet */
           size_t size,                 /* Size of the new object */
           const void *init             /* Optional value to which to initialize the object.  It is possible, using this
                                         * argument, to copy an object.  The initial ss_obj_t member of INIT is not copied
                                         * into the new object, but rather is initialized by this function. */
           )
{
    SS_ENTER(ss_obj_new, ss_obj_tP);
    SS_ASSERT(SS_MAGIC_OK(magic));

    /* Allocate the object if necessary */
    SS_ASSERT(size>=sizeof(ss_obj_t));
    if (!obj && NULL==(obj=malloc(size))) SS_ERROR(RESOURCE);

    /* Initialize the payload and the ss_obj_t members */
    if (!init) memset(obj, 0, size);
    else if (init==obj) memset(obj, 0, sizeof(ss_obj_t));
    else memcpy(obj, init, size);
    obj->magic = magic;

 SS_CLEANUP:
    SS_LEAVE(obj);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Objects
 * Purpose:     Destroy an object
 *
 * Description: Destroys an object by clearing its magic number. Even if the object was allocated by ss_obj_new() it is not
 *              freed by this function.  Objects should generally be destroyed by calling the destructor for the object's
 *              class, which then usually calls this function.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, June 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_obj_dest(ss_obj_t *obj)
{
    SS_ENTER(ss_obj_dest, herr_t);
    obj->magic = 0;
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Objects
 * Purpose:     Move an object
 *
 * Description: This function moves an object from one location in memory to another. If what you really want is to copy an
 *              object then use the ss_obj_new() function.  Even if the original object was allocated by ss_obj_new() its
 *              memory will not be freed by this function.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, July 31, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_obj_move(ss_obj_t *oldmem, ss_obj_t *newmem, size_t objsize)
{
    SS_ENTER(ss_obj_move, herr_t);
    
    SS_ASSERT(oldmem);
    SS_ASSERT(newmem);
    SS_ASSERT(SS_MAGIC_OK(oldmem->magic));
    SS_ASSERT(!SS_MAGIC_OK(newmem->magic));
    SS_ASSERT(objsize>0);

    if (NULL==ss_obj_new(newmem, oldmem->magic, objsize, oldmem)) SS_ERROR(FAILED);
    if (ss_obj_dest(oldmem)<0) SS_ERROR(FAILED);
    
 SS_CLEANUP:
    SS_LEAVE(0);
}
