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
SS_IF(val);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Values
 *
 * Description: These functions operate on values of pretty much any HDF5 datatype.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/* Callbacks for parts of objects that don't need anything special. */
static herr_t ss_val_sum_cb(void *buffer, size_t size, size_t nelmts, ss_val_cksum_t *cksum);
static int ss_val_cmp_cb(void *b1, void *b2, unsigned flags, size_t size, size_t nelmts);
static int ss_val_copy_cb(void *src, void *dst, size_t size, size_t nelmts);
static char *ss_val_encode_cb(void *buffer,  char *serbuf, size_t *serbuf_nused, size_t *serbuf_nalloc,
                              size_t size, size_t nelmts);
static size_t ss_val_decode_cb(void *buffer, const char *serbuf, size_t size, size_t nelmts);
static herr_t ss_val_res_cb(void *buffer, size_t size, size_t nelmts);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Interface initializer
 *
 * Description: Initializes the value interface if not already initialized.
 *
 * Return:      Returns non-negative on success, negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, May 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_init(void)
{
    SS_ENTER_INIT;
    /* Nothing to do */
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Search array for matching element
 *
 * Description: An array of objects is searched in linear order until an object is found that matches the supplied
 *              KEY. Only the elements of KEY that correspond to elements of MASK with non-zero bits are actually compared and
 *              the comparison is performed by calling ss_val_cmp().  The buffer, key, and mask must all be the same type of
 *              object which are each SIZE bytes.
 *
 * Return:      Returns a pointer to the first element of BUFFER where a match is found; null on failure or if no match is
 *              found.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, May 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_val_search(void *buffer,             /* Array of persistent objects to be searched. These objects must all be of the same
                                         * datatype as KEY and MASK. */
              void *key,                /* The specific values for which to search. Only the values that correspond non-zero
                                         * bytes in MASK will be compared. */                                  
              const void *mask,         /* Any integral-type member of KEY which corresponds to a byte range in MASK that has
                                         * at least one non-zero byte will be used in the comparison.  In other words, KEY and
                                         * MASK are the same datatype and MASK says what parts of KEY to compare while KEY
                                         * holds the actual values to be compared. */
              size_t size,              /* Size in bytes of KEY, MASK, and each element of BUFFER. */
              size_t nitems,            /* Number of persistent objects in the BUFFER array. */
              size_t nvalinfo,          /* Number of elements in the VALINFO array */
              const ss_val_info_t *valinfo /* Information about how members are arranged in KEY, MASK, and BUFFER */
              )
{
    SS_ENTER(ss_val_search, voidP);
    void                *retval=NULL;
    int                 status;

    while (nitems>0) {
        status = ss_val_cmp(buffer, key, mask, nvalinfo, valinfo);
        if (status) {
            retval = buffer;
            break;
        }
        buffer = (char*)buffer + size;
        --nitems;
    }

    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Compare two objects
 *
 * Description: The objects pointed to by OBJ1 and OBJ2 are compared, but only those atomic data that correspond
 *              to a value in MASK which is not all zero bits.  Furthermore, for each element being compared, the first
 *              non-zero byte of MASK for that element is treated as the comparison flags to be used for that element (see
 *              ss_val_cmp_t).
 *
 * Return:      Returns negative one if OBJ1 is less than OBJ2; one if greater; zero if equal. The ordering of certain
 *              members of the objects is somewhat arbitrary but reproducible (e.g., it doesn't really make sense for
 *              one object link to be "less than" another, but given two object links they will always compare the same way).
 *              Returns negative two on failure.
 *
 * Also:        ss_pers_cmp()
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, November 11, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_val_cmp(void *obj1,                          /* The object (haystack) being compared. */
           void *obj2,                          /* The values (needle) for which we are searching. Same datatype as OBJ1 */
           const void *mask,                    /* Same datatype as OBJ1 and indicates (by non-zero elements) which elements
                                                 * of OBJ1 and OBJ2 are to be compared. If MASK is the null pointer then OBJ1
                                                 * and OBJ2 are considered to be equal by definition. */
           size_t nvalinfo,                     /* Number of elements in the VALINFO array. */
           const ss_val_info_t *valinfo         /* Information about the elements of OBJ1, OBJ2, and MASK. */
           )
{
    SS_ENTER(ss_val_cmp, int);
    size_t              vi_idx;                 /* Index into VALINFO */
    size_t              elmt_idx;               /* Counter for elements of a particular VALINFO part */
    size_t              byte_idx;               /* Counter for bytes of an element */
    void                *cmp1=NULL;             /* Starting address of OBJ1 to be compared */
    void                *cmp2=NULL;             /* Starting address of OBJ2 to be compared */
    unsigned            cmp_flags=0;            /* How to compare */
    size_t              nelmts=0;               /* Number of elements to compare */
    size_t              ncompared=0;            /* Number of elements compared */
    int                 retval=0;               /* Return value */

    SS_RETVAL(-2);                              /* Failure return value is -2 */

    SS_ASSERT(obj1);
    SS_ASSERT(obj2);
    SS_ASSERT(valinfo || 0==nvalinfo);

    if (!mask) goto done;

    for (vi_idx=0; vi_idx<nvalinfo; vi_idx++) {
        /* Pointer to first element of this part */
        char *p1 = (char*)obj1 + valinfo[vi_idx].start;
        char *p2 = (char*)obj2 + valinfo[vi_idx].start;
        const unsigned char *m  = (const unsigned char*)mask + valinfo[vi_idx].start;
        if (!valinfo[vi_idx].cmp_func) continue;
        
        for (elmt_idx=0; elmt_idx<valinfo[vi_idx].nelmts; elmt_idx++) {
            /* Find first non-zero byte (if any) in mask for this element */
            for (byte_idx=0; byte_idx<valinfo[vi_idx].size; byte_idx++)
                if (m[byte_idx]) break;

            if (byte_idx<valinfo[vi_idx].size) {
                /* Mask element is non-zero so we must compare. But instead of comparing now, accumulate adjacent elements of
                 * this compiled part (compno) of the datatype so we need only make one call to the comparator. If the mask
                 * specifies a different flag then process what we have first. */
                if (nelmts && m[byte_idx]!=cmp_flags) {
                    if (-2==(retval = (valinfo[vi_idx].cmp_func)(cmp1, cmp2, cmp_flags, valinfo[vi_idx].size, nelmts)))
                        SS_ERROR(FAILED);
                    ncompared += nelmts;
                    nelmts = 0;
                    if (retval) goto done;
                }
                if (0==nelmts++) {
                    cmp1 = p1;
                    cmp2 = p2;
                    cmp_flags = m[byte_idx];
                }
            } else if (nelmts>0) {
                /* We don't compare this element (mask is zero) but we've previously accumulated things to compare. */
                if (-2==(retval = (valinfo[vi_idx].cmp_func)(cmp1, cmp2, cmp_flags, valinfo[vi_idx].size, nelmts)))
                    SS_ERROR(FAILED);
                ncompared += nelmts;
                nelmts = 0;
                if (retval) goto done;
            }

            /* Advance to next element */
            p1 += valinfo[vi_idx].size;
            p2 += valinfo[vi_idx].size;
            m  += valinfo[vi_idx].size;
        }

        /* We've processed all elements of this part (vi_idx) of the datatype so we must compare what we've
         * accumulated so far. */
        if (nelmts>0) {
            if (-2==(retval = (valinfo[vi_idx].cmp_func)(cmp1, cmp2, cmp_flags, valinfo[vi_idx].size, nelmts)))
                SS_ERROR(FAILED);
            ncompared += nelmts;
            nelmts = 0;
            if (retval) goto done;
        }
    }

    if (0==ncompared) SS_ERROR_FMT(NOTFOUND, ("mask elements are all zero (nothing to compare)"));

done:
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Reset object
 *
 * Description: Given a pointer to memory and information about the objects in that memory, deallocate objects
 *
 * Return:      Returns non-negative on success with the checksum returned through CKSUM; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Erik Illescas
 *              Friday, April 27, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_reset(void *buffer,                              /* Buffer which is to be checksummed */
             size_t nvalinfo,                           /* Number of elements in the VALINFO argument */
             const ss_val_info_t *valinfo               /* Type information about the BUFFER (see ss_val_compile()). */
             )
{
    SS_ENTER(ss_val_reset, herr_t);
    herr_t              status;
    size_t              i;
                                                                                                                                   
    SS_ASSERT(buffer);
    SS_ASSERT(valinfo || 0==nvalinfo);
                                                                                                                                   
    for (i=0; i<nvalinfo; i++) {
        if (!valinfo[i].res_func) continue;
        status = (valinfo[i].res_func)((char*)buffer+valinfo[i].start, valinfo[i].size, valinfo[i].nelmts);
        if (status<0) SS_ERROR(FAILED);
    }
                                                                                                                                   
 SS_CLEANUP:
    SS_LEAVE(0);
}


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Compute object checksum
 *
 * Description: Given a pointer to memory and information about the objects in that memory, compute the current checksum for
 *              the memory and return it through the CKSUM argument.
 *
 * Return:      Returns non-negative on success with the checksum returned through CKSUM; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_cksum(void *buffer,                              /* Buffer which is to be checksummed */
             size_t nvalinfo,                           /* Number of elements in the VALINFO argument */
             const ss_val_info_t *valinfo,              /* Type information about the BUFFER (see ss_val_compile()). */
             ss_val_cksum_t *init,                      /* Optional initial value for checksum. If this is the null pointer
                                                         * then the checksum is initialized to zero instead. */
             ss_val_cksum_t *cksum                      /* OUT: Returned checksum for the object. */
             )
{
    SS_ENTER(ss_val_cksum, herr_t);
    herr_t              status;
    size_t              i;

    SS_ASSERT(buffer);
    SS_ASSERT(valinfo || 0==nvalinfo);
    SS_ASSERT(cksum);

    if (!init) {
        ss_val_cksum_reset(cksum);
    } else if (cksum!=init) {
        *cksum = *init;
    }
    for (i=0; i<nvalinfo; i++) {
        if (!valinfo[i].sum_func) continue;
        status = (valinfo[i].sum_func)((char*)buffer+valinfo[i].start, valinfo[i].size, valinfo[i].nelmts, cksum);
        if (status<0) SS_ERROR(FAILED);
    }
    
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Copies an object
 *
 * Description: Given a pointer to some object, copy things to which that object points when creating a new object. For
 *              instance, if the source object contains a variable length string then that value is copied into the
 *              destination so that the source and destination objects do not share resources.
 *
 *              If the SRC argument is not supplied then the DST object is assumed to have been mem-copied from some other
 *              object and therefore shares resources with that original object. In this case, the DST argument will serve as
 *              both the source value and the destination buffer and sharable resources will be copied in place.  It is also
 *              permissible for SRC and DST to be the same pointer value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, January 27, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_copy(void *src,                          /* Optional source value to be copied. If this is the null pointer
                                                 * then the DST is used as both source and destination; that is, the
                                                 * copy is performed in place. */
            void *dst,                          /* OUT: The destination buffer, which is also used as the source value
                                                 * if SRC is not supplied. */
            size_t nvalinfo,                    /* Number of elements in the VALINFO argument. */
            const ss_val_info_t *valinfo        /* Type information about the object being copied. */
            )
{
    SS_ENTER(ss_val_copy, herr_t);
    herr_t              status;
    size_t              i;

    SS_ASSERT(dst);
    SS_ASSERT(valinfo || 0==nvalinfo);
    
    if (!src) src=dst;

    for (i=0; i<nvalinfo; i++) {
        if (!valinfo[i].cpy_func) continue;
        status = (valinfo[i].cpy_func)((char*)src+valinfo[i].start, (char*)dst+valinfo[i].start, valinfo[i].size,
                                       valinfo[i].nelmts);
        if (status<0) SS_ERROR(FAILED);
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Serialize objects for transmission
 *
 * Description: Serializes objects for transmission to other MPI tasks.  The objects are specified as either an array of
 *              objects or an array of pointers to objects, the latter being useful when the objects are not contiguous.
 *              If the objects contain members that are not appropriate for inter-task transmission (such as pointers to
 *              variable length strings) then those things will be somehow converted and stored in a buffer which is returned
 *              by this function.
 *
 *              When transmitting data from one task to another one generally calls this function to serialize parts of the
 *              object that are not suitable for transmission and then communicates the size of the serialization buffer and
 *              that buffer to the peer. It then transmits the objects themselves using the MPI datatype for object
 *              transmission obtained from ss_val_ser_type().
 *
 * Return:      On success, returns an allocated buffer containing parts of the objects that have been converted for
 *              transmission and the size in bytes of that buffer.  Even if the returned size is zero a buffer is allocated in
 *              order to distinguish this case from a failure, which returns a null pointer.
 *
 * Parallel:    Independent
 *
 * Example:     Broadcast 100 units from task zero of MPI_COMM_WORLD to the other tasks.  We assume that task zero has an
 *              array of units called !units and the other tasks have a similar array not yet containing any valid data. All
 *              tasks have identical values for !nvalinfo and !valinfo obtained from ss_val_compile().
 *
 *                  ss_val_ser_type(nvalinfo,valinfo,&transtype);
 *                  if (0==task) buf = ss_val_ser_encode(units,NULL,100,&size,nvalinfo,valinfo);
 *                  MPI_Bcast(&size,1,MPI_SIZE_T,0,MPI_COMM_WORLD);
 *                  if (0!=task) buf = malloc(MAX(1,size));
 *                  if (size) MPI_Bcast(buf,size,MPI_BYTE,0,MPI_COMM_WORLD);
 *                  MPI_Bcast(units,100,transtype,0,MPI_COMM_WORLD);
 *                  if (0!=task) ss_val_ser_decode(units,NULL,100,buf,nvalinfo,valinfo);
 *                  free(buf);
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
ss_val_ser_encode(void *obj_array,                      /* An optional array of NELMTS objects to be serialized. This argument
                                                         * or OBJ_VECTOR must be specified but not both. */
                  void **obj_vector,                    /* An optional array of NELMTS pointers to objects to be serialized.
                                                         * This argument or OBJ_VECTOR must be specified but not both. */
                  size_t nelmts,                        /* Number of objects to be serialized. That is, the number of elements
                                                         * in the OBJ_ARRAY or OBJ_VECTOR argument. */
                  size_t *bufsize,                      /* OUT: size of returned buffer in bytes. */
                  size_t nvalinfo,                      /* Number of elements in the VALINFO argument. */
                  const ss_val_info_t *valinfo          /* Type information for the objects (see ss_val_compile()). */
                 )
{
    SS_ENTER(ss_val_ser_encode, charP);
    char        *retval=0, *status;
    size_t      i, j, nalloc=0, tsize;
    void        *obj;
    
    SS_ASSERT(obj_array || obj_vector);
    SS_ASSERT(bufsize);
    SS_ASSERT(nvalinfo && valinfo);

    retval = NULL;
    *bufsize = 0;
    tsize = valinfo[nvalinfo-1].start + valinfo[nvalinfo-1].size * valinfo[nvalinfo-1].nelmts;

    for (i=0; i<nelmts; i++) {
        obj = obj_array ? (char*)obj_array+i*tsize : obj_vector[i];
        if (!obj) SS_ERROR_FMT(USAGE, ("OBJ_VECTOR has null pointer for element %lu", (unsigned long)i));
        for (j=0; j<nvalinfo; j++) {
            if (!valinfo[j].enc_func) continue;
            status = (valinfo[j].enc_func)((char*)obj+valinfo[j].start, retval, bufsize, &nalloc,
                                           valinfo[j].size, valinfo[j].nelmts);
            if (NULL==status) SS_ERROR(FAILED);
            retval = status;
        }
    }
    SS_EXTEND(retval, 1, nalloc); /*just to prevent a null return value when successful*/

SS_CLEANUP:
    SS_FREE(retval);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Unserialize transmitted objects
 *
 * Description: This is the final step in transmitting objects from one task to another (see ss_val_ser_encode()). Given an
 *              array of partially initialized objects (or an array of pointers to such objects) and a buffer of serialized
 *              data from ss_val_ser_encode(), this function decodes the serialized data and copies it into the specified
 *              objects.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_ser_decode(void *obj_array,                      /* An array of objects to be used as the destination. This argument or
                                                         * OBJ_VECTOR must be specified but not both. */
                  void **obj_vector,                    /* An array of pointers to objects to be used as the destination. This
                                                         * argument or OBJ_ARRAY must be specified but not both. */
                  size_t nelmts,                        /* Number of elements in OBJ_ARRAY or OBJ_VECTOR */
                  const char *serbuf,                   /* The serialization buffer (can be null if no serialized part). */
                  size_t nvalinfo,                      /* Number of elements in the VALINFO argument. */
                  const ss_val_info_t *valinfo          /* Type information for the objects (see ss_val_compile()). */
                  )
{
    SS_ENTER(ss_val_ser_decode, herr_t);
    size_t              i, j, tsize, advance;
    void                *obj;

    tsize = valinfo[nvalinfo-1].start + valinfo[nvalinfo-1].size * valinfo[nvalinfo-1].nelmts;

    for (i=0; i<nelmts; i++) {
        obj = obj_array ? (char*)obj_array+i*tsize : obj_vector[i];
        SS_ASSERT_CLASS(obj, ss_persobj_t);
        if (!obj) SS_ERROR_FMT(USAGE, ("OBJ_VECTOR has null pointer for element %lu", (unsigned long)i));
        for (j=0; j<nvalinfo; j++) {
            if (!valinfo[j].dec_func) continue;
            advance = (valinfo[j].dec_func)((char*)obj+valinfo[j].start, serbuf, valinfo[j].size, valinfo[j].nelmts);
            if (SS_NOSIZE==advance) SS_ERROR(FAILED);
            serbuf += advance;
        }
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Create transmission datatype
 *
 * Description: Creates an MPI datatype that describes the parts of an object that can be transmitted directly from task to
 *              task. This type can be cached because it never changes for a given VALINFO array. It's just an indexed MPI
 *              type that points to the parts of the data that have the `transmit' bit set.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. Serial behavior is a no-op.
 *
 * Also:        ss_val_ser_encode(), ss_val_ser_decode()
 *
 * Programmer:  Robb Matzke
 *              Thursday, November 20, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_ser_type(size_t UNUSED_SERIAL nvalinfo,                  /* Number of elements in the VALINFO argument. */
                const ss_val_info_t UNUSED_SERIAL *valinfo,     /* Type information (see ss_val_compile()). */
                MPI_Datatype UNUSED_SERIAL *type                /* OUT: The MPI datatype that describes the parts of each
                                                                 * object to transmit between tasks. */
                )
{
    SS_ENTER(ss_val_ser_type, herr_t);
#ifdef HAVE_PARALLEL
    size_t              i, n=0;
    MPI_Datatype        subtype[64];
    MPI_Aint            offset[64];
    int                 nbytes[64];
    size_t              tsize;

    /* Total size of the type */
    tsize = valinfo[nvalinfo-1].start + valinfo[nvalinfo-1].size * valinfo[nvalinfo-1].nelmts;

    /* Obtain offsets and sizies for each part of the type that needs to be transmitted directly. Some of these types might
     * also have some sort of encoding function. */
    for (i=0; i<nvalinfo; i++) {
        if (valinfo[i].transmit) {
            SS_ASSERT(n<NELMTS(subtype));
            subtype[n] = MPI_BYTE;
            offset[n] = (MPI_Aint)(valinfo[i].start);
            SS_ASSERT(valinfo[i].size * valinfo[i].nelmts <= INT_MAX);
            nbytes[n] = (int)(valinfo[i].size * valinfo[i].nelmts);
            n++;
        }
    }

    /* MPI-1 doesn't have MPI_Type_create_resized() so we use MPI_UB instead */
    SS_ASSERT(n<NELMTS(subtype));
    subtype[n] = MPI_UB;
    offset[n] = (MPI_Aint)tsize;
    nbytes[n] = 1;
    n++;

    /* Create the type */
    if (MPI_Type_struct((int)n, nbytes, offset, subtype, type)) SS_ERROR(MPI);
    if (MPI_Type_commit(type)) SS_ERROR(MPI);
SS_CLEANUP:
#endif /*HAVE_PARALLEL*/
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Compile function callbacks for type
 *
 * Description: SSlib is required to be able to copy, checksum, and compare certain objects based on their run-time datatype.
 *              This function creates a datatype description that contains the callback function pointers and an indication of
 *              the bytes for which each callback is responsible.  This information can be computed once per type and then
 *              used repeatedly.
 *
 * Return:      On success, returns an array of callback information sorted by byte offset.  If the user supplied a VALINFO
 *              array then that pointer is returned, otherwise an array is allocated by this function.  Returns the null
 *              pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_val_info_t *
ss_val_compile(hid_t type,                      /* HDF5 datatype for which function callbacks are to be compiled. */
               size_t nvalinfo,                 /* If VALINFO is supplied this is the number of elements in that array */
               ss_val_info_t *valinfo,          /* Optional array to be filled in, which should contain at least NVALINFO
                                                 * elements. */
               size_t *nused                    /* OUT: Number of entries in the returned array that are valid. */
               )
{
    SS_ENTER(ss_val_compile, ss_val_info_tP);
    ss_val_info_t       *vi_here=valinfo;       /* return value */
    size_t              n=0;                    /* number of values used in `valinfo' */
    int                 nmembs, membno;         /* number of members in struct, enum, etc; and counter thereto */
    int                 ndims, dim;             /* array dimensionality and counter thereto */
    hsize_t             dims[H5S_MAX_RANK];     /* array dimensions */
    size_t              offset;                 /* byte offset of a member of a type within that type */
    size_t              size;                   /* size in bytes of a type member */
    size_t              nelmts;                 /* number of elements of a particular type */
    size_t              slotno;                 /* element index into `valinfo' array */
    hid_t               member=-1, super=-1;    /* member type of a compound type; super type of an array or enum type */
    char                *tag=NULL;              /* name associated with an opaque datatype */
    H5T_class_t         tclass;                 /* class of a compound type member */
    hsize_t             tsize;                  /* total size in bytes of the type being processed */
    size_t              start=0;                /* where current type starts relative to beginning of incoming TYPE memory */
    size_t              i;                      /* counter */

    /* In order to simplify building the valinfo array, specifically the combining of similar adjacent entries, we handle
     * recursion for nested compound datatypes by pushing certain things onto a stack. */
#define SS_VAL_MAXSAVED 3
    int                 saved_nmembs[SS_VAL_MAXSAVED];/* stack of saved `nmembs' values for struct recursion */
    int                 saved_membno[SS_VAL_MAXSAVED];/* stack of saved `membno' values for struct recursion */
    hid_t               saved_type[SS_VAL_MAXSAVED];  /* stack of saved datatypes for struct recursion */
    hid_t               saved_start[SS_VAL_MAXSAVED]; /* stack of saved `start' offsets for struct recursion */
    int                 nsaved=0;               /* number of items on saved_* stacks */

    /* These are the values that will go into the next valinfo slot */
    hbool_t             transmit;               /* transmit type directly from task to task */
    ss_val_resfunc_t    res_func=NULL;          /* reset function */
    ss_val_sumfunc_t    sum_func=NULL;          /* check sum function */
    ss_val_cmpfunc_t    cmp_func=NULL;          /* comparison function */
    ss_val_cpyfunc_t    cpy_func=NULL;          /* copy function */
    ss_val_encfunc_t    enc_func=NULL;          /* serial encoding function */
    ss_val_decfunc_t    dec_func=NULL;          /* serial decoding function */

    
    if (!valinfo) nvalinfo=0;
    if ((nmembs = H5Tget_nmembers(type))<0) SS_ERROR(HDF5);
    
    for (membno=-1; membno<nmembs; membno++) {
    recurse:
        if (-1==membno) {
            /* Automatically insert `saf_each', thereby treating it as if it were a persistent part of the object. */
            offset = HOFFSET(ss_persobj_t, saf_each);
            member = H5Tcopy(H5T_NATIVE_SIZE);
        } else {
            offset = H5Tget_member_offset(type, (unsigned)membno); /*will only fail if H5Tget_member_type() also fails*/
            if ((member = H5Tget_member_type(type, (unsigned)membno))<0) SS_ERROR(HDF5);
        }
        nelmts = 1;

        /* Initialize defaults */
    redo:
        transmit = FALSE;
        res_func = NULL;
        sum_func = NULL;
        cmp_func = NULL;
        cpy_func = NULL;
        if (0==(size = H5Tget_size(member))) SS_ERROR(HDF5);
        if ((tclass = H5Tget_class(member))<0) SS_ERROR(HDF5);

        /* Choose functions */
        switch (tclass) {
        case H5T_INTEGER:
        case H5T_FLOAT:
        case H5T_TIME:
        case H5T_BITFIELD:
        case H5T_ENUM:
            transmit = TRUE;
            res_func = ss_val_res_cb;
            sum_func = ss_val_sum_cb;
            cmp_func = ss_val_cmp_cb;
            cpy_func = ss_val_copy_cb;
            enc_func = ss_val_encode_cb;
            dec_func = ss_val_decode_cb;
            break;
                
        case H5T_STRING:
            /* SSlib doesn't use these */
            SS_ERROR(NOTIMP);
            break;

        case H5T_OPAQUE:
            /* Checksum should be calculated based on the datatype tag */
            if (NULL==(tag=H5Tget_tag(member))) SS_ERROR(HDF5);
            if (!strcmp(tag, "SSlib::string_tm")) {
                transmit = FALSE;
                res_func = ss_string_res_cb;
                sum_func = ss_string_sum_cb;
                cmp_func = ss_string_cmp_cb;
                cpy_func = ss_string_copy_cb;
                enc_func = ss_string_encode_cb;
                dec_func = ss_string_decode_cb;
            } else if (!strcmp(tag, "SSlib::pers_tm")) {
                transmit = FALSE;
                res_func = ss_val_res_cb; /*default*/
                sum_func = ss_pers_sum_cb;
                cmp_func = ss_pers_cmp_cb;
                cpy_func = ss_val_copy_cb; /*default*/
                enc_func = ss_pers_encode_cb;
                dec_func = ss_pers_decode_cb;
            } else if (!strcmp(tag, "SSlib::array_tm")) {
                transmit = FALSE;
                res_func = ss_array_res_cb;
                sum_func = ss_array_sum_cb;
                cmp_func = ss_array_cmp_cb;
                cpy_func = ss_array_copy_cb;
                enc_func = ss_array_encode_cb;
                dec_func = ss_array_decode_cb;
            } else {
                SS_ERROR_FMT(NOTIMP, ("opaque tag=\"%s\"", tag));
            }
            tag = SS_FREE(tag);
            break;

        case H5T_COMPOUND:
            /* A few SSlib data structures use compound datatypes. We only use compound datatypes if the type doesn't need any
             * special functionality for computing checksums, copying, etc. */
            SS_ASSERT(1==nelmts); /*we don't currently support arrays of compound types*/

            /* Save current values on a stack */
            SS_ASSERT(nsaved<SS_VAL_MAXSAVED);
            saved_nmembs[nsaved] = nmembs;
            saved_membno[nsaved] = membno;
            saved_type[nsaved] = type;
            saved_start[nsaved] = start;
            nsaved++;

            /* Inialize loop counters etc. with member type info */
            type = member;
            member = -1;
            if ((nmembs = H5Tget_nmembers(type))<0) SS_ERROR(HDF5);
            membno = 0;
            start += offset;
            
            goto recurse;

        case H5T_REFERENCE:
            /* I don't think these are used by sslib */
            SS_ERROR(NOTIMP);
            break;

        case H5T_VLEN:
            /* Variable length arrays are used by sslib. HDF5 returns variable length strings as H5T_STRING. */
            SS_ERROR(NOTIMP);
            break;

        case H5T_ARRAY:
            /* Handle arrays recursively by super-type, incrementing the `nelmts' to account for the array duplicity. */
            if ((ndims=H5Tget_array_dims(member, dims, NULL))<0) SS_ERROR(HDF5);
            for (dim=0; dim<ndims; dim++) nelmts *= dims[dim];
            super = H5Tget_super(member);
            H5Tclose(member);
            member = super;
            super = -1;
            goto redo;

        default:
            SS_ERROR(NOTIMP);
            break;
        }
        if (H5Tclose(member)<0) SS_ERROR(HDF5);
        member = -1;

        /* Increment the `offset' by `start' in order to obtain an offset relative to the beginning of the incoming TYPE */
        offset += start;

        /* Now that we have the offset, size, and function pointers, insert this information into the array of `comp'
         * information sorted by offset. We make use of the fact that members are usually inserted into the compound datatype
         * in offset order to achieve an O(1) insertion time, but we also have to watch out for other orders just in case. */
        SS_ASSERT(size && nelmts);

        /* Find where to insert this entry */
        if (n>0) {
            for (i=n, slotno=SS_NOSIZE; i>0; --i) {
                if (offset>vi_here[i-1].start) {
                    slotno=i;
                    break;
                }
            }
        } else {
            slotno = 0;
        }
        SS_ASSERT(slotno!=SS_NOSIZE);

        /* Insert the entry, joining it with the preceding and/or following entry if adjacent */
        SS_EXTEND(vi_here, MAX(8,n+1), nvalinfo);
        if (slotno>0 &&
            vi_here[slotno-1].size == size &&
            offset == vi_here[slotno-1].start + vi_here[slotno-1].size*vi_here[slotno-1].nelmts &&
            vi_here[slotno-1].transmit == transmit &&
            vi_here[slotno-1].res_func == res_func &&
            vi_here[slotno-1].sum_func == sum_func &&
            vi_here[slotno-1].cmp_func == cmp_func &&
            vi_here[slotno-1].cpy_func == cpy_func &&
            vi_here[slotno-1].enc_func == enc_func &&
            vi_here[slotno-1].dec_func == dec_func) {
            vi_here[slotno-1].nelmts += nelmts;
            --slotno;
        } else {
            memmove(vi_here+slotno+1, vi_here+slotno, n-slotno);
            n++;
            vi_here[slotno].start = offset;
            vi_here[slotno].size = size;
            vi_here[slotno].nelmts = nelmts;
            vi_here[slotno].transmit = transmit;
            vi_here[slotno].res_func = res_func;
            vi_here[slotno].sum_func = sum_func;
            vi_here[slotno].cmp_func = cmp_func;
            vi_here[slotno].cpy_func = cpy_func;
            vi_here[slotno].enc_func = enc_func;
            vi_here[slotno].dec_func = dec_func;
        }
        if (slotno+1 < n &&
            vi_here[slotno].size == vi_here[slotno+1].size &&
            vi_here[slotno].start + vi_here[slotno].size*vi_here[slotno].nelmts == vi_here[slotno+1].start &&
            vi_here[slotno].transmit == vi_here[slotno+1].transmit &&
            vi_here[slotno].res_func == vi_here[slotno+1].res_func &&
            vi_here[slotno].sum_func == vi_here[slotno+1].sum_func &&
            vi_here[slotno].cmp_func == vi_here[slotno+1].cmp_func &&
            vi_here[slotno].cpy_func == vi_here[slotno+1].cpy_func &&
            vi_here[slotno].enc_func == vi_here[slotno+1].enc_func &&
            vi_here[slotno].dec_func == vi_here[slotno+1].dec_func) {
            vi_here[slotno].nelmts += vi_here[slotno+1].nelmts;
            memmove(vi_here+slotno, vi_here+slotno+1, n-(slotno+1));
            --n;
        }

        /* If we've reached the end of this struct and we're recursing then pop certain things off the stack. */
        while (membno+1==nmembs && nsaved>0) {
            --nsaved;
            nmembs = saved_nmembs[nsaved];
            membno = saved_membno[nsaved];
            if (H5Tclose(type)<0) SS_ERROR(HDF5);
            type = saved_type[nsaved];
            start = saved_start[nsaved];
        }
    }

    /* If the last entry doesn't extend to the end of the datatype then add a no-op entry to pad things out */
    if (0==(tsize=H5Tget_size(type))) SS_ERROR(HDF5);
    if (0==n || vi_here[n-1].start+vi_here[n-1].size*vi_here[n-1].nelmts!=tsize) {
        SS_EXTEND(vi_here, n+1, nvalinfo);
        vi_here[n].size = 1;
        vi_here[n].start = vi_here[n-1].start + vi_here[n-1].size*vi_here[n-1].nelmts;
        vi_here[n].nelmts = tsize - vi_here[n].start;
        vi_here[n].transmit = FALSE;
        vi_here[n].res_func = NULL;
        vi_here[n].sum_func = NULL;
        vi_here[n].cmp_func = NULL;
        vi_here[n].cpy_func = NULL;
        vi_here[n].enc_func = NULL;
        vi_here[n].dec_func = NULL;
        n++;
    }
    
    if (nused) *nused = n;

 SS_CLEANUP:
    if (member>0) H5Tclose(member);
    if (super>0) H5Tclose(super);
    SS_FREE(tag);
    if (!valinfo) vi_here = SS_FREE(vi_here);
    SS_LEAVE(vi_here);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Reset checksum to initial value
 *
 * Description: Resets the checksum to an initial value which is not necessarily zero.
 *
 * Return:      Returns non-negative on success, negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_cksum_reset(ss_val_cksum_t *cksum /*OUT: Reset to an initiali value upon successful return.*/)
{
    SS_ENTER(ss_val_cksum_reset, herr_t);
    SS_ASSERT(cksum);
    memset(cksum, 0, sizeof(*cksum));
    cksum->adler = ss_adler32(0L, NULL, 0);
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Determine if a checksum has been computed
 *
 * Description: It is assumed that any non-zero checksum is, in fact, computed.
 *
 * Return:      Returns true (positive) if and only if CKSUM is non-zero, false otherwise; negative on error.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, December  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_val_cksum_isset(const ss_val_cksum_t *cksum)
{
    SS_ENTER(ss_val_cksum_isset, htri_t);
    htri_t                      retval=FALSE;
    static ss_val_cksum_t       zero;
    static int                  called=0;

    if (0==called) {
        called++;
        if (ss_val_cksum_reset(&zero)<0) SS_ERROR(FAILED);
    }

    retval = (cksum->adler!=zero.adler || cksum->flags) ? TRUE : FALSE;

SS_CLEANUP:
    SS_LEAVE(retval);
}


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Compare two checksums
 *
 * Description: Similar to memcmp() except for checksums.
 *
 * Return:      Similar to memcmp().
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, November 19, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_val_cksum_cmp(const ss_val_cksum_t *sum1, const ss_val_cksum_t *sum2)
{
    /* SS_ENTER -- skipped for performance */
    assert(sum1 && sum2);
    if (sum1->adler<sum2->adler) return -1;
    if (sum1->adler>sum2->adler) return  1;
    if (sum1->flags<sum2->flags) return -1;
    if (sum1->flags>sum2->flags) return  1;
    return 0;
}


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Compute checksum directly from memory
 *
 * Description: All SIZE*NELMTS bytes of BUFFER are used to update the checksum CKSUM.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August 22, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static herr_t
ss_val_sum_cb(void *buffer,
              size_t size,                      /* Size of each element in bytes. */
              size_t nelmts,                    /* Number of elements. */
              ss_val_cksum_t *cksum             /* INOUT: Checksum updated from SIZE*NELMTS bytes starting at BUFFER. */
              )
{
    SS_ENTER(ss_val_sum_cb, herr_t);
    SS_ASSERT(cksum);

    if (buffer && size*nelmts) {
        cksum->adler = ss_adler32(cksum->adler, buffer, size*nelmts);
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Compare parts of two objects
 *
 * Description: This function compares all SIZE bytes of NELMTS objects beginning at address BUF to the values stored at
 *              address KEY. The FLAGS bit vector argument indicates how that comparison should be made (see SS_VAL_CMP).
 *
 * Return:      On success returns negative one if BUF is less than KEY, zero if they are equal, and one if BUF is greater than
 *              KEY. Returns -2 on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, November 10, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
static int
ss_val_cmp_cb(void *buf,                        /* Array of haystack values when looking for a needle in the haystack. */
              void *key,                        /* Array of needle values when looking for a needle in the haystack. */
              unsigned UNUSED flags,            /* See SS_VAL_CMP. */
              size_t size,                      /* Size in bytes of each element. */
              size_t nelmts                     /* Number of elements to compare. */
              )
{
    /* SS_ENTER skipped for efficiency */
    int status;
    assert(buf && key);
    assert(SS_VAL_CMP_DFLT==flags);
    status = memcmp(buf, key, size*nelmts);
    return MINMAX(-1, status, 1);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Copy part of an object
 *
 * Description: Copies NELMTS elements each of size SIZE bytes from SRC to DST. Same semantics as memcpy().  It is permissible
 *              for DST and SRC to have the same pointer value.
 *
 * Return:      Always returns non-negative
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, November 10, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static herr_t
ss_val_copy_cb(void *src,                       /* Source buffer */
               void *dst,                       /* Destination buffer */
               size_t size,                     /* Size in bytes of each element */
               size_t nelmts                    /* Number of elements to copy */
               )
{
    /* SS_ENTER skipped for efficiency */
    if (src && src!=dst) memcpy(dst, src, size*nelmts);
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Encode object data
 *
 * Description: Encodes part of an object that cannot be sent directly to another task by copying it into a serialization
 *              buffer, SERBUF, which is allocated and/or extended as necessary.
 *
 * Return:      The encoding functions return a pointer to the possibly reallocated SERBUF on success or the null pointer on
 *              failure. However, this particular function is a no-op and always succeeds.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, November 21, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
static char *
ss_val_encode_cb(UNUSED void *buffer,           /* Data to be encoded. */
                 UNUSED char *serbuf,           /* Allocated buffer into which data is decoded. */
                 UNUSED size_t *serbuf_nused,   /* Current bytes used in the `serbuf'. */
                 UNUSED size_t *serbuf_nalloc,  /* Current bytes allocated for `serbuf'. */
                 UNUSED size_t size,            /* Size in bytes of each thing to be encoded. */
                 UNUSED size_t nelmts           /* Number of things to be encoded. */
                 )
{
    SS_ENTER(ss_val_encode_cb, charP);
    SS_EXTEND(serbuf, (size_t)32, *serbuf_nalloc); /*make sure return value is not null*/
SS_CLEANUP:
    SS_LEAVE(serbuf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Decode object data
 *
 * Description: Decodes some data from SERBUF and inserts it into the beginning of BUFFER.
 *
 * Return:      Returns the number of bytes consumed from SERBUF on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent.
 *
 * Programmer:  Robb Matzke
 *              Friday, November 21, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
static size_t
ss_val_decode_cb(UNUSED void *buffer,                   /* Buffer where data is to be decoded. */
                 UNUSED const char *serbuf,             /* Data to be decoded, i.e., unserialized. */
                 UNUSED size_t size,                    /* Size of each thing after being unserialized. */
                 UNUSED size_t nelmts                   /* Number of things to unserialize. */
                 )
{
    SS_ENTER(ss_val_decode_cb, herr_t);
    /* intentionally empty */
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Values
 * Purpose:     Reset object data
 *
 * Description: This is the generic reset function. It just sets the value to zero bits.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, May 10, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_res_cb(void *buffer,                             /* Buffer to be reset */
              size_t size,                              /* Number of bytes per element */
              size_t nelmts                             /* Number of elements */
              )
{
    SS_ENTER(ss_val_res_cb, herr_t);
    memset(buffer, 0, size*nelmts);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Values
 * Purpose:     Print an arbitrary datum
 *
 * Description: Given memory of a certain type, print it to the specified stream.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, August 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_val_dump(void *val,                  /* Value to be printed */
            hid_t type,                 /* Datatype of VAL */
            void *_parent,              /* Optional persistent object into which VAL points */
            FILE *out,                  /* Stream to which output should be sent */
            const char *html_tag        /* Optional HTML tag to use in output; NULL means output plain text */
            )
{
    SS_ENTER(ss_val_dump, herr_t);
    char        tmp[128], name[64];
    hsize_t     nelmts, i;
    hid_t       elmttype;
    size_t      elmtsize;

    assert(H5Tget_size(type)<=sizeof tmp);
    memcpy(tmp, val, H5Tget_size(type));
    if (html_tag) fprintf(out, "<%s>", html_tag);

    if (H5Tequal(type, ss_string_tm)) {
        const char *quote = html_tag ? "" : "\"";
        fprintf(out, "%s%s%s", quote, ss_string_ptr(val), quote);
    } else if (H5Tequal(type, ss_pers_tm)) {
        ss_scope_t      parent_scope=SS_SCOPE_NULL;
        size_t          gfile_idx, file_idx, scope_idx, obj_idx;
        ss_file_t       file=SS_FILE_NULL;
        ss_pers_class_t *pc=NULL;
        hbool_t         is_indirect;

        if (SS_PERS_ISNULL(val)) {
            fprintf(out, "(ss_pers_t*)NULL");
        } else {
            pc = SS_PERS_CLASS(SS_MAGIC_SEQUENCE(SS_MAGIC_OF(val)));
            assert(pc);

            /* Find the index of the file to which VAL belongs by consulting the "file" table in the scope to which the parent
             * object (_parent) belongs. */
            if (NULL==ss_pers_scope(_parent, &parent_scope)) {
                fprintf(out, "ERROR(scope)");
                SS_ERROR(FAILED);
            }
            gfile_idx = ss_pers_link_gfileidx(val);
            for (file_idx=0; /*void*/; file_idx++) {
                if (NULL==ss_pers_refer_c(&parent_scope, SS_MAGIC(ss_file_t), file_idx, (ss_pers_t*)&file)) {
                    fprintf(out, "ERROR(link)");
                    SS_ERROR(FAILED);
                }
                if (NULL==SS_FILE(&file)) {
                    /* This can happen if the VAL link points to a file that hasn't yet been entered into the file table of
                     * the _PARENT object. */
                    file_idx = SS_NOSIZE;
                    break;
                }
                if (SS_FILE(&file)->m.gfileidx==gfile_idx) {
                    break;
                }
            }
    
            /* The scope index within that file */
            scope_idx = ss_pers_link_scopeidx(val);

            /* The object index within that scope */
            obj_idx = ss_pers_link_objidx(val);
            if (obj_idx & SS_TABLE_INDIRECT) {
                is_indirect = TRUE;
                obj_idx &= ~SS_TABLE_INDIRECT;
            } else {
                is_indirect = FALSE;
            }
            
            /* Now print the info. If the file index is zero then omit it since it's the same file to which VAL belongs. If
             * the scope index is the same as that of the object then omit it also. */
            if (html_tag) fprintf(out, "<a href=\"XXX\">");
            fprintf(out, "(ss_%s_t*)", pc->name);
            if (SS_NOSIZE==file_idx) {
                ss_gfile_t *gfile = SS_GFILE_IDX(gfile_idx);
                fprintf(out, "{S%08lx,%lu,%s%lu}", (unsigned long)gfile->serial, (unsigned long)scope_idx,
                        is_indirect?"I":"", (unsigned long)obj_idx);
            } else if (file_idx>0) {
                fprintf(out, "{%lu,%lu,%s%lu}", (unsigned long)file_idx, (unsigned long)scope_idx,
                        is_indirect?"I":"", (unsigned long)obj_idx);
            } else if (scope_idx!=ss_pers_link_scopeidx(_parent)) {
                fprintf(out, "{%lu,%s%lu}", (unsigned long)scope_idx, is_indirect?"I":"", (unsigned long)obj_idx);
            } else {
                fprintf(out, "%s%lu", is_indirect?"I":"", (unsigned long)obj_idx);
            }
            if (html_tag) fprintf(out, "</a>");
        }
    } else if (H5Tequal(type, ss_array_tm)) {
        nelmts = ss_array_nelmts(val);
        elmttype = ss_array_targeted(val);
        if (H5Tequal(elmttype, ss_pers_tf)) {
            H5Tclose(elmttype);
            elmttype = H5Tcopy(ss_pers_tm);
        }
        if (html_tag) fprintf(out, "<table><tr>");
        else fprintf(out, "[");
        for (i=0; i<nelmts; i++) {
            if (!html_tag && i) fprintf(out, ", ");
            ss_array_get(val, elmttype, (size_t)i, (size_t)1, &tmp);
            ss_val_dump(tmp, elmttype, _parent, out, html_tag);
        }
        if (html_tag) fprintf(out, "</tr></table>");
        else fprintf(out, "]");
        H5Tclose(elmttype);
    } else {
        switch (H5Tget_class(type)) {
        case H5T_ARRAY:
            assert(1==H5Tget_array_ndims(type));
            H5Tget_array_dims(type, &nelmts, NULL);
            elmttype = H5Tget_super(type);
            elmtsize = H5Tget_size(elmttype);
            if (html_tag) fprintf(out, "<table><tr>");
            else fprintf(out, "[");
            for (i=0; i<nelmts; i++) {
                if (i && !html_tag) fprintf(out, ", ");
                ss_val_dump((char*)val+i*elmtsize, elmttype, _parent, out, html_tag);
            }
            if (html_tag) fprintf(out, "</tr></table>");
            else fprintf(out, "]");
            H5Tclose(elmttype);
            break;
        case H5T_INTEGER:
            H5Tconvert(type, H5T_NATIVE_LLONG, 1, tmp, NULL, H5P_DEFAULT);
            fprintf(out, "%lld", *(long_long*)tmp);
            break;
        case H5T_FLOAT:
            H5Tconvert(type, H5T_NATIVE_DOUBLE, 1, tmp, NULL, H5P_DEFAULT);
            fprintf(out, "%g", *(double*)tmp);
            break;
        case H5T_ENUM:
            H5Tenum_nameof(type, val, name, sizeof name);
            fprintf(out, "%s", name);
            break;
        case H5T_STRING:
            elmtsize = H5Tget_size(type);
            if (H5T_CSET_ASCII!=H5Tget_cset(type) || H5T_STR_NULLTERM!=H5Tget_strpad(type)) {
                fprintf(out, "%sSTRING%s", html_tag?"&lt;":"<", html_tag?"&gt;":">");
            } else {
                fprintf(out, "\"%s\"", (char*)val);
            }
            break;
        case H5T_COMPOUND:
            fprintf(out, "%sCOMPOUND%s", html_tag?"&lt;":"<", html_tag?"&gt;":">");
            break;
        default:
            fprintf(out, "%sUNKNOWN%s", html_tag?"&lt;":"<", html_tag?"&gt;":">");
            break;
        }
    }
    
    if (html_tag) fprintf(out, "</%s>", html_tag);
SS_CLEANUP:
    SS_LEAVE(0);
}
