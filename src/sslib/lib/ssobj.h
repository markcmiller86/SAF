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
#ifndef SS_HAVE_SSOBJ_H
#define SS_HAVE_SSOBJ_H

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Magic Numbers
 * Description: Many of the SSlib data structures have an !unsigned !int member that will contain a magic number
 *              while the struct is allocated. The magic number serves to run-time type the struct. The most significant 12
 *              bits are 0x5af (looks sort of like "Saf"). The next eight bits are a type class number (e.g., all storable
 *              object handles belong to a certain class). The least significant 12 bits are a unique sequence number for that
 *              particular type class and are sometimes used as indices into various arrays.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Miscellaneous (class 0x5af01000) */
#define SS_MAGIC_ss_prop_t              0x5af01000
#define SS_MAGIC_ss_table_t             0x5af01001
#define SS_MAGIC_ss_string_table_t      0x5af01002
#define SS_MAGIC_ss_gblob_t             0x5af01003

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Persistent object links (class 0x5af02000)
 * Description: These are the magic numbers for persistent object links, which are the handles to persistent objects that
 *              the client usually works with.
 * Issue:       These magic numbers must be in the same order as the persistent object magic numbers (class 0x5af03000). */
#define SS_MAGIC_ss_pers_t              0x5af02000      /* just the class part */
#define SS_MAGIC_ss_scope_t             0x5af02000
#define SS_MAGIC_ss_field_t             0x5af02001
#define SS_MAGIC_ss_role_t              0x5af02002
#define SS_MAGIC_ss_basis_t             0x5af02003
#define SS_MAGIC_ss_algebraic_t         0x5af02004
#define SS_MAGIC_ss_evaluation_t        0x5af02005
#define SS_MAGIC_ss_relrep_t            0x5af02006
#define SS_MAGIC_ss_quantity_t          0x5af02007
#define SS_MAGIC_ss_unit_t              0x5af02008
#define SS_MAGIC_ss_cat_t               0x5af02009
#define SS_MAGIC_ss_collection_t        0x5af0200a
#define SS_MAGIC_ss_set_t               0x5af0200b
#define SS_MAGIC_ss_rel_t               0x5af0200c
#define SS_MAGIC_ss_fieldtmpl_t         0x5af0200d
#define SS_MAGIC_ss_tops_t              0x5af0200e
#define SS_MAGIC_ss_blob_t              0x5af0200f
#define SS_MAGIC_ss_indexspec_t         0x5af02010
#define SS_MAGIC_ss_file_t              0x5af02011
#define SS_MAGIC_ss_attr_t              0x5af02012

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Persistent objects (class 0x5af03000)
 * Description: These are the magic numbers for the persistent objects themselves. They do not appear in the file but are
 *              part of the transient information for an object.  The order of things here is such that when synchronizing
 *              a scope we minimize the number of forward references. That is, if objects of type A can point to objects of
 *              type B then we should synchronize type B before type A.
 * Issue:       These magic numbers must be in the same order as the persistent object link magic numbers (class 0x5af02000).
 *              Also, they are mentioned in ss_pers_init() when constructing an HDF5 enumeration datatype.
 *
 *              If you add items here and they don't show up as tables in the files then the SS_PERS_NCLASSES constant defined
 *              in sspers.h is probably not large enough. */
#define SS_MAGIC_ss_persobj_t           0x5af03000      /* just the class part */
#define SS_MAGIC_ss_scopeobj_t          0x5af03000
#define SS_MAGIC_ss_fieldobj_t          0x5af03001
#define SS_MAGIC_ss_roleobj_t           0x5af03002
#define SS_MAGIC_ss_basisobj_t          0x5af03003
#define SS_MAGIC_ss_algebraicobj_t      0x5af03004
#define SS_MAGIC_ss_evaluationobj_t     0x5af03005
#define SS_MAGIC_ss_relrepobj_t         0x5af03006
#define SS_MAGIC_ss_quantityobj_t       0x5af03007
#define SS_MAGIC_ss_unitobj_t           0x5af03008
#define SS_MAGIC_ss_catobj_t            0x5af03009
#define SS_MAGIC_ss_collectionobj_t     0x5af0300a
#define SS_MAGIC_ss_setobj_t            0x5af0300b
#define SS_MAGIC_ss_relobj_t            0x5af0300c
#define SS_MAGIC_ss_fieldtmplobj_t      0x5af0300d
#define SS_MAGIC_ss_topsobj_t           0x5af0300e
#define SS_MAGIC_ss_blobobj_t           0x5af0300f
#define SS_MAGIC_ss_indexspecobj_t      0x5af03010
#define SS_MAGIC_ss_fileobj_t           0x5af03011
#define SS_MAGIC_ss_attrobj_t           0x5af03012

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Obtain magic number for type
 * Description: Returns the magic number for the specified SSlib datatype.
 * Return:      Returns an !unsigned magic number. */
#define SS_MAGIC(_type_)        SS_MAGIC_##_type_

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Determine magicness
 * Description: Determines if number or class M looks magic.
 * Return:      True if M is probably a magic number; false otherwise. */
#define SS_MAGIC_OK(M)          (0x5af00000==((M)&0xfff00000))

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Obtain magic number class
 * Description: Given a magic number M, return the class part by masking off the low-order 12 bits.
 * Return:      An !unsigned !int magic class number. */
#define SS_MAGIC_CLASS(M)       ((M) & 0xfffff000)

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Obtain magic sequence number
 * Description: Given a magic number M, return the sequence number stored in the 12 low-order bits.
 * Return:      An !unsigned !int magic sequence number. */
#define SS_MAGIC_SEQUENCE(M)    ((M) & 0x00000fff)

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Construct a magic number
 * Description: Given a magic class number like what is returned by SS_MAGIC_CLASS() and a sequence number like
 *              what is returned by SS_MAGIC_SEQUENCE(), construct a magic number.  The class, C, and sequence, S, don't have
 *              to be purely a class or sequence because they'll be filtered.
 * Return:      An !unsigned !int magic number constructed from a class and sequence number. */
#define SS_MAGIC_CONS(C,S)      (SS_MAGIC_CLASS(C)|SS_MAGIC_SEQUENCE(S))

/* Audience:    Public
 * Chapter:     Magic Numbers
 * Purpose:     Obtain magic number from a pointer
 * Description: Given a pointer to any object, return the magic number stored in that object.
 * Return:      An !unsigned !int magic number or zero if OBJ is the null pointer. */
#define SS_MAGIC_OF(OBJ)        ((OBJ)?*(const unsigned int*)(OBJ):(unsigned int)0)

/* Audience:    Private
 * Chapter:     Objects
 * Purpose:     Create a new object
 * Description: This macro is just a convenience for a call to ss_obj_new(), which allocates a new object of type _type_,
 *              initializes it to all zero, and fills in the appropriate magic number, etc. Usually one creates objects
 *              in the subclasses, as with ss_pers_new() or ss_prop_new().
 * Example:
 *              ss_table_t *table = SS_OBJ_NEW(ss_table_t);
 *              if (!table) SS_ERROR(CONS);
 */
#define SS_OBJ_NEW(_type_) (_type_*)ss_obj_new(NULL,SS_MAGIC(_type_),sizeof(_type_),NULL)

/* Audience:    Private
 * Chapter:     Objects
 * Purpose:     Destroy an object
 * Description: This is simply a convenience function for ss_obj_dest() so that the caller doesn't have to cast the argument
 *              and return value. One normally uses the destructor for the object's class since most objects are derived from
 *              ss_obj_t.
 * Return:      Always returns null so it can be easily assigned to the object being destroyed.
 * Issue:       The cast is for Irix cc that thinks the type of the whole expression is !long.
 * Example:     table = SS_OBJ_DEST(table); */
#define SS_OBJ_DEST(_obj_) ((void*)((ss_obj_dest((ss_obj_t*)_obj_)<0?H5Eclear():0),NULL))

/* Most structs (certainly any that have a magic number) will have this as their first member. This allows us to add
 * additional functionality to all objects of the library at a later time. Certain objects (e.g., persistent object links) can
 * be copied from one memory location to another with the C assignment operator and therefore this ss_obj_t struct must
 * contain only things that would be valid to copy in that way. */
typedef struct ss_obj_t {
    unsigned            magic;                  /* magic number unigue to each particular datatype, for run-time type checking */
} ss_obj_t;

#ifdef __cplusplus
extern "C" {
#endif

/* prototypes */
herr_t ss_obj_init(void);
ss_obj_t *ss_obj_new(ss_obj_t *obj, unsigned magic, size_t size, const void *init);
herr_t ss_obj_dest(ss_obj_t *obj);
herr_t ss_obj_move(ss_obj_t *oldmem, ss_obj_t *newmem, size_t objsize);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSOBJ_H*/


