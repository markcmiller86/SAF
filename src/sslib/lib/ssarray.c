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
SS_IF(array);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 *
 * Description: SSlib has support for small, variable-length, arrays with independent operations. The array elements can be
 *              either persistent object links or a user-defined datatype without embedded SSlib special types.
 *              The data for all variable length arrays in a scope is aggregated (together with the variable length strings)
 *              into a single dataset which is read in its entirety when a scope is opened.
 *
 *              The reason for the distinction between whether the array stores SSlib datatypes or not is because conversion
 *              between memory and file representations, calculation of checksums, and interprocess communication require
 *              facilities provided by SSlib for those types and aren't available (or are handled entirely differently) for
 *              the non-SSlib datatypes.  In practice this doesn't turn out to be a problem because variable length arrays are
 *              generally only used to store persistent object links (as in a Set object pointing to Collections) or native
 *              integers (as in a Field's permutation vector).
 *
 *              An array is born with zero elements of of ss_pers_t type.  If the array is intended to store something other
 *              than object links then its datatype must be changed with ss_array_target(). The number of elements in an array
 *              is changed with ss_array_resize(). The ss_array_get() and ss_array_put() functions query or modify elements of
 *              an array and ss_array_reset() sets the array back to an initial state.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

hid_t ss_array_tm = -1;                 /* HDF5 datatype for variable length arrays in memory */
hid_t ss_array_tf = -1;                 /* HDF5 datatype for variable length arrays in a file */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Interface initializer
 *
 * Description: Initializes the array interface.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, February 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_init(void)
{
    size_t              tf_size;
    SS_ENTER_INIT;

    /* HDF5 datatypes for an array in memory and in the file. We use opaque types for these rather than compound types because
     * it allows us to get control of the datatype conversion at the variable length array level rather than down in its
     * components. */
    
    /* Array in memory */
    if ((ss_array_tm = H5Tcreate(H5T_OPAQUE, sizeof(ss_array_t)))<0) SS_ERROR(INIT);
    if (H5Tset_tag(ss_array_tm, "SSlib::array_tm")<0) SS_ERROR(INIT);
    if (H5Tlock(ss_array_tm)<0) SS_ERROR(INIT);

    /* Array in a file -- two variable length strings: the elements and the encoded datatype thereof */
    if (0==(tf_size=H5Tget_size(ss_string_tf))) SS_ERROR(HDF5);
    if ((ss_array_tf = H5Tcreate(H5T_OPAQUE, 2*tf_size))<0) SS_ERROR(INIT);
    if (H5Tset_tag(ss_array_tf, "SSlib::array_tf")<0) SS_ERROR(INIT);
    if (H5Tlock(ss_array_tf)<0) SS_ERROR(INIT);

    /* Conversion functions */
    if (H5Tregister(H5T_PERS_HARD, "ss_array_t(mf)", ss_array_tm, ss_array_tf, ss_array_convert_mf)<0) SS_ERROR(INIT);
    if (H5Tregister(H5T_PERS_HARD, "ss_array_t(fm)", ss_array_tf, ss_array_tm, ss_array_convert_fm)<0) SS_ERROR(INIT);

 SS_CLEANUP:
    ss_array_tf = ss_array_tm = -1;
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Convert array elements to memory type
 *
 * Description: The ss_array_convert_fm() converts the array itself into the memory datatype but does not necessarily convert
 *              each element of the array into the memory datatype. This function makes sure that the elements have been
 *              converted to memory datatype.  It can also be called after the !nelmts member of the ARRAY struct has been
 *              incremented or decremented.
 *
 * Return:      Returns non-negative on success; negetive on falure.
 *
 * Parallel:    Independent
 *
 * Issue:       Changing the MTYPE for an array that is already cached is likely to result in previously modified array
 *              elements being lost.  This needs more testing. [rpm 2004-04-26]
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April 20, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_cache(ss_array_t *array,
               hid_t mtype                      /* Memory datatype. This argument is unused if the array stores persistent
                                                 * object links (in which case the memory datatype is always ss_pers_tm). */
               )
{
    SS_ENTER(ss_array_cache, herr_t);
    size_t      elmt_msize=0;
    size_t      elmt_fsize=0;
    const char  *elmt_fbuf=NULL;
    size_t      elmtno;
    ss_table_t  *filetab=NULL;

    SS_ASSERT(array);
    
    /* The datatypes of the elements */
    if (0==array->ftype) {
        if (0==(elmt_fsize = H5Tget_size(ss_pers_tf))) SS_ERROR(HDF5);
        if (0==(elmt_msize = H5Tget_size(ss_pers_tm))) SS_ERROR(HDF5);
        SS_ASSERT(mtype<=0 || H5Tequal(mtype, ss_pers_tm));
        mtype = 0;
    } else {
        if (0==(elmt_fsize = H5Tget_size(array->ftype))) SS_ERROR(HDF5);
        if (0==(elmt_msize = H5Tget_size(mtype))) SS_ERROR(HDF5);
    }

    /* If there are no elements then there is nothing to do. It might be the case that there were elements (even cached in
     * mbuf) but the array was resized to contain no elements. In such a case the dirty bit will be set and the `fbuf' will be
     * adjusted during conversion from memory to file. */
    if (0==array->nelmts) goto done;

    /* If data is cached and it's the correct datatype then there is nothing to do. */
    if (array->mbuf) {
        if (!mtype && !array->mtype) goto done;
        if (mtype && array->mtype && H5Tequal(mtype, array->mtype)) goto done;
    }
    
    /* Allocate the new mbuf, which should be large enough for both the file and memory datatypes since we'll be using it to
     * do data type conversion. The reason we need to clean a dirty array is so that if the caller modifies an element, then
     * changes the memory datatype, then reads the element they get a correct value. We could save some time by converting
     * directly from the old datatype to the new datatype but this is probably not a common enough occurrence to make it worth
     * the effort. */
    if (array->mbuf && ss_array_clean(array)<0) SS_ERROR(FAILED);
    if (NULL==(array->mbuf=realloc(array->mbuf, array->nelmts*MAX(elmt_fsize,elmt_msize)))) SS_ERROR(RESOURCE);

    /* Reset the array's mtype */
    if (array->mtype>0) H5Tclose(array->mtype);
    array->mtype = mtype ? H5Tcopy(mtype) : 0;

    /* Fill the mbuf with memory data. */
    if (0==ss_string_memlen(&(array->fbuf))) {
        /* The array contents has never been saved to the file. The memory values should be all zero bits. */
        memset(array->mbuf, 0, array->nelmts*elmt_msize);
    } else if (0==array->ftype) {
        /* The array contains persistent object links. We need to know to which table the links belong, thus we can only cache
         * these kinds of arrays when we're actually reading from the scope.  See ss_array_convert_fm(). */
        SS_ASSERT(!SS_PERS_ISNULL(&ss_table_scope_g)); /*a scope I/O operation is in progress*/
        if (NULL==(filetab=ss_scope_table(&ss_table_scope_g, SS_MAGIC(ss_file_t), NULL))) SS_ERROR(FAILED);
        SS_ASSERT(ss_string_memlen(&(array->fbuf))==array->nelmts*elmt_fsize);
        if (NULL==(elmt_fbuf=ss_string_ptr(&(array->fbuf)))) SS_ERROR(FAILED);
        SS_ASSERT(elmt_fsize<=elmt_msize); /*determines direction of loop below*/
        for (elmtno=0; elmtno<array->nelmts; elmtno++) {
            memcpy((char*)(array->mbuf)+elmtno*elmt_msize, (const char*)elmt_fbuf+elmtno*elmt_fsize, elmt_fsize);
            if (ss_pers_convert_fm2(((char*)(array->mbuf)+elmtno*elmt_msize),
                                    (ss_pers_t*)((char*)(array->mbuf)+elmtno*elmt_msize),
                                    filetab)<0)
                SS_ERROR(FAILED);
        }
    } else {
        /* The array holds non-SSlib datatypes. */
        SS_ASSERT(ss_string_memlen(&(array->fbuf))==array->nelmts*elmt_fsize);
        memcpy(array->mbuf, ss_string_ptr(&(array->fbuf)), array->nelmts*elmt_fsize);
        if (H5Tconvert(array->ftype, array->mtype, array->nelmts, array->mbuf, NULL, H5P_DEFAULT)<0) SS_ERROR(HDF5);
    }
done:
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Clear dirty bit in array
 *
 * Description: When an array is modified by changing its target (file) datatype, the number of elements in the array, or the
 *              value of an element then the "dirty" bit is set for the array. This function clears that bit by making sure
 *              that the persistent variable length string and encoded file datatype are up-to-date.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent for user-defined datatypes; For arrays with persistent object links see the parallel notes for
 *              ss_pers_convert_mf().
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April 27, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_clean(ss_array_t *array)
{
    SS_ENTER(ss_array_clean, herr_t);
    size_t      elmt_msize=0;           /* Size of an array element in memory */
    size_t      elmt_fsize=0;           /* Size of an array element in the file */
    void        *convbuf=NULL;          /* Temporary conversion buffer */
    size_t      cur_len;                /* Current length of the ARRAY `fbuf' */
    void        *enc_ftype=NULL;        /* Encoded file datatype */
    size_t      enc_ftype_size=0;       /* Number of bytes used in allocated enc_ftype */
    size_t      enc_ftype_nalloc=0;     /* Number of bytes allocated for enc_ftype */

    SS_ASSERT(array);
    if (!array->dirty) goto done;

    /* Size of elements in memory and the file */
    if (0==(elmt_msize=H5Tget_size(array->mtype>0?array->mtype:ss_pers_tm))) SS_ERROR(HDF5);
    if (0==(elmt_fsize=H5Tget_size(array->ftype>0?array->ftype:ss_pers_tf))) SS_ERROR(HDF5);

    if (array->mbuf) {
        /* The `mbuf' holds the data. Although the `mbuf' is large enough to hold both the memory and the file datatypes we
         * will allocate a separate conversion buffer so as not to destroy what's already cached in memory. */
        if (NULL==(convbuf=malloc(array->nelmts * MAX(elmt_fsize, elmt_msize)))) SS_ERROR(RESOURCE);
        memcpy(convbuf, array->mbuf, array->nelmts*elmt_msize);
        if (H5Tconvert(array->mtype>0?array->mtype:ss_pers_tm, array->ftype>0?array->ftype:ss_pers_tf,
                       array->nelmts, convbuf, NULL, H5P_DEFAULT)<0) SS_ERROR(HDF5);
        if (ss_string_memset(&(array->fbuf), convbuf, array->nelmts*elmt_fsize)<0) SS_ERROR(FAILED);
        convbuf = SS_FREE(convbuf);
    } else if (SS_NOSIZE==(cur_len=ss_string_memlen(&(array->fbuf)))) {
        SS_ERROR(FAILED);
    } else if (array->nelmts*elmt_fsize > cur_len) {
        /* Nothing is cached but the number of elements was increased. Increase the length of the `fbuf' string. */
        if (ss_string_splice(&(array->fbuf), NULL, cur_len, array->nelmts*elmt_fsize-cur_len, 0)<0) SS_ERROR(FAILED);
    } else {
        /* Nothing cached but the number of elements was decreased. */
        if (ss_string_splice(&(array->fbuf), NULL, array->nelmts*elmt_fsize, 0, SS_NOSIZE)<0) SS_ERROR(FAILED);
    }

    /* Convert file datatype to a string */
    if (array->ftype>0) {
        if (NULL==(enc_ftype=H5encode(array->ftype, enc_ftype, &enc_ftype_size, &enc_ftype_nalloc))) SS_ERROR(FAILED);
        if (ss_string_memset(&(array->enc_ftype), enc_ftype, enc_ftype_size)<0) SS_ERROR(FAILED);
    } else {
        ss_string_reset(&(array->enc_ftype));
    }

    array->dirty = FALSE;
done:
    enc_ftype = SS_FREE(enc_ftype);
SS_CLEANUP:
    SS_FREE(enc_ftype);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Change array element datatype
 *
 * Description: Every array has two datatypes associated with it: a datatype for the elements as they are stored in the file,
 *              and a datatype of the elements as they exist in memory with the ss_array_put() and ss_array_get() functions.
 *              When a new array is created the memory datatype is the HDF5 equivalent of ss_pers_t (a persistent object link)
 *              and the file datatype is its counterpart, ss_pers_tf.  This function sets the file datatype to the specified
 *              value and clears the memory buffer and associated memory datatype.
 *
 * Return:      Returns non-negative on success; negative on failure.  It is an error to modify the file datatype if the array
 *              size is positive.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 21, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_target(ss_array_t *array,              /* Array whose datatype is to be modified. */
                hid_t ftype                     /* Datatype of the array elements as stored in the file. */
                )
{
    SS_ENTER(ss_array_target, herr_t);

    if (!array) SS_ERROR_FMT(USAGE, ("no ss_array_t supplied"));
    if (ftype<=0 || H5Tequal(ftype, ss_pers_tf)) ftype = 0;

    if ((!ftype && array->ftype) ||
        (ftype && !array->ftype) ||
        (ftype && !H5Tequal(ftype, array->ftype))) {
        /* New type is different than old type */
        if (array->nelmts>0) SS_ERROR_FMT(USAGE, ("cannot retarget a non-empty array"));
        if (array->ftype>0) H5Tclose(array->ftype);
        array->ftype = ftype ? H5Tcopy(ftype) : 0;
        array->dirty = TRUE; /*because array->ftype is newer than array->enc_ftype*/
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Inquire about array file datatype
 *
 * Description: Every array has a file datatype that dermines how values are stored in a file. This function returns a copy of
 *              that datatype.
 *
 * Return:      On success, a positive object ID for a copy of the file datatype. If an array stores links to other objects
 *              then the returned datatype is a copy of ss_pers_tf.  Returns negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, June  4, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
ss_array_targeted(ss_array_t *array)
{
    SS_ENTER(ss_array_targeted, hid_t);
    hid_t retval=-1;

    if (!array) SS_ERROR_FMT(USAGE, ("no ss_array_t supplied"));
    if ((retval=H5Tcopy(array->ftype?array->ftype:ss_pers_tf))<0) SS_ERROR(FAILED);
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Change the size of a variable length array
 *
 * Description: Elements can be added or removed from the end of an array. If items are added then they are also initialized
 *              to zero.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. If more than one task changes the size of an array then they must all make identical changes to
 *              the size.
 *
 * Programmer:  Robb Matzke
 *              Friday, March  5, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_resize(ss_array_t *array,              /* Array whose size is to be changed. */
                size_t nelmts                   /* Number of total elements to be contained in the array. */
                )
{
    SS_ENTER(ss_array_resize, herr_t);
    size_t              mtype_size;             /* Size of each array element in memory (i.e., in `mbuf') */

    if (!array) SS_ERROR_FMT(USAGE, ("no ss_array_t supplied"));
    if (array->nelmts==nelmts) goto done;

    /* Extend the memory buffer with zeros */
    if (array->mbuf && nelmts>array->nelmts) {
        mtype_size = H5Tget_size(array->mtype>0 ? array->mtype : ss_pers_tm);
        if (NULL==(array->mbuf=realloc(array->mbuf, nelmts*mtype_size))) SS_ERROR(RESOURCE);
        memset((char*)(array->mbuf)+array->nelmts*mtype_size, 0, (nelmts-array->nelmts)*mtype_size);
    }

    /* Mark as dirty so that the `fbuf' string gets updated */
    array->nelmts = nelmts;
    array->dirty = TRUE;

done:
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Obtain array value
 *
 * Description: This function extracts the array value or subpart thereof. The value is copied into the optional
 *              caller-supplied BUFFER (or a buffer is allocated). If the value consists of more than one
 *              element then desired elements to be returned can be specified with an offset and length.
 *
 * Return:      Returns a pointer (BUFFER if non-null) on success; null on failure. If the caller doesn's supply BUFFER then
 *              this function will allocate one.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, February 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_array_get(ss_array_t *array,                 /* Array from which to retrieve data. */
             hid_t mtype,                       /* Datatype for memory. Pass ss_pers_tm (or preferably negative) for an array
                                                 * of persistent object links. */                                             
             size_t offset,                     /* First element to be returned. It is an error to specify a starting element
                                                 * that is outside the valid range of values defined for the array. */
             size_t nelmts,                     /* Number of elements to return.  The OFFSET and NELMTS define a range of
                                                 * elements to be returned. If the range extends beyond the end of the defined
                                                 * range of elements for ARRAY then an error is raised; but if NELMTS is the
                                                 * constant SS_NOSIZE then all elements up to and including the last element
                                                 * are returned. */
             void *buffer                       /* The optional caller-supplied buffer to be filled in by the request. If the
                                                 * caller didn't supply a buffer then one will be created. */
             )
{
    SS_ENTER(ss_array_get, ss_pers_tP);
    size_t      mtype_size;                     /* Size of each array element in memory. */

    if (!array) SS_ERROR_FMT(USAGE, ("no ss_array_t supplied"));

    /* Check requested range of values against those currently defined for the array */
    if (offset>array->nelmts)
        SS_ERROR_FMT(DOMAIN, ("starting offset %lu but array has only %lu elements",
                              (unsigned long)offset, (unsigned long)(array->nelmts)));
    if (SS_NOSIZE==nelmts) {
        nelmts = array->nelmts - offset;
    } else if (offset+nelmts>array->nelmts) {
        SS_ERROR_FMT(DOMAIN, ("requested elements %lu through %lu but array has only %lu elements",
                              (unsigned long)offset, (unsigned long)(offset+nelmts-1), (unsigned long)(array->nelmts)));
    }
    if (0==nelmts) {
        if (!buffer && NULL==(buffer=malloc(1))) SS_ERROR(RESOURCE);
        goto done;
    }

    /* Make sure data has been converted to memory format. */
    if (ss_array_cache(array, mtype)<0) SS_ERROR(FAILED);

    /* Copy data into return value */
    if (0==(mtype_size=H5Tget_size(mtype>0?mtype:ss_pers_tm))) SS_ERROR(HDF5);
    if (!buffer && NULL==(buffer=malloc(nelmts*mtype_size))) SS_ERROR(RESOURCE);
    memcpy(buffer, (char*)(array->mbuf)+offset*mtype_size, nelmts*mtype_size);

done:
SS_CLEANUP:
    SS_LEAVE(buffer);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Modify part of an array
 *
 * Description: NELMTS values beginning at array element OFFSET are modified by setting them to VALUE.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, February 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_put(ss_array_t *array,                 /* The array whose value will be modified. */
             hid_t mtype,                       /* The datatype of the values pointed to by VALUE. If the array contains
                                                 * persistent object links then pass ss_pers_tm (or preferably negative). */
             size_t offset,                     /* The array element number at which to put VALUE. */
             size_t nelmts,                     /* The number of array elements in VALUE. If this is the constant SS_NOSIZE
                                                 * then we assume that VALUE contains enough data to fill up the current size
                                                 * of the array beginning at the specified OFFSET. */
             const void *value                  /* The value to be written into the array. */
             )
{
    SS_ENTER(ss_array_put, herr_t);
    size_t      mtype_size=0;                   /* Size of MTYPE in bytes */

    if (!array) SS_ERROR_FMT(USAGE, ("no ss_array_t supplied"));

    /* Check requested range of elements against those defined for the array */
    if (offset>array->nelmts)
        SS_ERROR_FMT(DOMAIN, ("starting offset %lu but array has only %lu elements",
                              (unsigned long)offset, (unsigned long)(array->nelmts)));
    if (SS_NOSIZE==nelmts) nelmts = array->nelmts - offset;
    if (offset+nelmts>array->nelmts)
        SS_ERROR_FMT(DOMAIN, ("elements %lu through %lu selected but array has only %lu elements",
                              (unsigned long)offset, (unsigned long)(offset+nelmts-1), (unsigned long)(array->nelmts)));

    /* Make sure array values are in memory. */
    /* ISSUE: there really isn't any point in actually converting the existing values to memory format and initializing the
     *        array's mbuf if we're about to overwrite the whole thing anyway. */
    if (ss_array_cache(array, mtype)<0) SS_ERROR(FAILED);

    /* Copy data from client into array */
    if (0==(mtype_size=H5Tget_size(array->mtype>0?array->mtype:ss_pers_tm))) SS_ERROR(HDF5);
    memcpy((char*)(array->mbuf)+offset*mtype_size, value, nelmts*mtype_size);
    array->dirty = TRUE;

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Free memory associated with the array
 *
 * Description: Frees the array value stored in ARRAY but does not free ARRAY itself.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, February 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_reset(ss_array_t *array)
{
    SS_ENTER(ss_array_reset, herr_t);
    if (!array) SS_ERROR_FMT(USAGE, ("no ss_array_t supplied"));
    if (ss_string_reset(&(array->fbuf))<0) SS_ERROR(HDF5);
    if (ss_string_reset(&(array->enc_ftype))<0) SS_ERROR(HDF5);
    if (array->mtype>0) H5Tclose(array->mtype);
    if (array->ftype>0) H5Tclose(array->ftype);
    SS_FREE(array->mbuf);
    array->dirty = TRUE;
    memset(array, 0, sizeof(*array));

 SS_CLEANUP:
    if (array) memset(array, 0, sizeof(*array));
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Variable Length Arrays
 * Purpose:     Query the number of elements
 *
 * Description: This function returns the number of elements defined in ARRAY.
 *
 * Return:      Number of elements on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, February 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_array_nelmts(const ss_array_t *array)
{
    SS_ENTER(ss_array_nelmts, size_t);

    if (!array) SS_ERROR_FMT(USAGE, ("no ss_array_t supplied"));
    
SS_CLEANUP:
    SS_LEAVE(array->nelmts);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Convert variable length array to file type
 *
 * Description: This is an H5Tconvert() callback to convert ss_array_t objects in memory into the file representation.  If the
 *              array is caching elements in the memory datatype then the elements are first converted to the file datatype
 *              and then the array is converted to the file representation.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    This is a scope collective function because the conversion from the memory to file datatype of the elements
 *              themselves might be scope collective.  For example, if the elements are object links then ss_pers_convert_mf()
 *              will be called, which is collective.
 *
 * Programmer:  Robb Matzke
 *              Friday, March 5, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_array_convert_mf(hid_t UNUSED src_type,      /* Source data type (should be ss_array_tm) */
                    hid_t dst_type,             /* Destination data type (should be ss_array_tf) */
                    H5T_cdata_t *cdata,         /* Data type conversion control data */
                    size_t narrays,             /* Number of arrays to convert */
                    size_t buf_stride,          /* Distance between elements in bytes */
                    size_t UNUSED bkg_stride,   /* Distance between elements in the background buffer. */
                    void *buf,                  /* The buffer which is to be converted in place. */
                    void UNUSED *bkg,           /* The background buffer. */
                    hid_t UNUSED dxpl           /* Dataset transfer property list. */
                    )
{
    SS_ENTER(ss_array_convert_mf, herr_t);
    char                *src = buf;             /* The source data pointer */
    char                *dst = buf;             /* The destination data pointer */
    ss_array_t          tmp;                    /* Temporary for non-overlapping conversion calls */
    size_t              dst_size;               /* Size of the destination datatype */
    hsize_t             arrayno;                /* Element number currently being converted */

    SS_ASSERT(cdata);

    switch (cdata->command) {
    case H5T_CONV_INIT:
        /* See ss_array_init() for definitions of these types */
        SS_ASSERT(H5Tequal(src_type, ss_array_tm));
        SS_ASSERT(H5Tequal(dst_type, ss_array_tf));
        SS_ASSERT(H5Tget_size(src_type)==sizeof(ss_array_t));
        SS_ASSERT(H5Tget_size(dst_type)==2*H5Tget_size(ss_string_tf)); /*the array elements and the datatype thereof*/
        SS_ASSERT(H5Tget_size(src_type)>=H5Tget_size(dst_type)); /*implies left-to-right conversion*/
        break;

    case H5T_CONV_FREE:
        break;

    case H5T_CONV_CONV:
        if (0==(dst_size=H5Tget_size(dst_type))) SS_ERROR(HDF5);
        for (arrayno=0; arrayno<narrays; arrayno++) {
           /* Copy source into temporary buffer since `src' and `dst' might overlap. We can't just use memcpy() because the
            * call to ss_array_clean() may reallocate various heap pointers, which would be bad because if we're being called
            * from H5Dwrite() `src' itself might only be a copy. */
           if (ss_array_copy_cb(src, &tmp, sizeof tmp, 1)<0) SS_ERROR(FAILED);

            if (ss_array_clean(&tmp)<0) SS_ERROR(FAILED);

            /* Convert variable length strings into file representation */
            if (ss_string_convert_mf2(&(tmp.fbuf), dst, NULL)<0) SS_ERROR(FAILED);
            if (ss_string_convert_mf2(&(tmp.enc_ftype), dst+H5Tget_size(ss_string_tf), NULL)<0) SS_ERROR(FAILED);

           /* Reclaim memory from the ss_array_copy_cb() above. */
           if (ss_array_reset(&tmp)<0) SS_ERROR(FAILED);


            /* Advance pointers */
            src += buf_stride ? buf_stride : sizeof tmp;
            dst += buf_stride ? buf_stride : dst_size;
        }
        break;

    default:
        SS_ERROR_FMT(HDF5, ("unexpected value for cdata->command"));
    }

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Convert variable length array to memory type
 *
 * Description: This is an H5Tconvert() callback to convert ss_array_t objects in a dataset into memory. All we do is convert
 *              the members of the ss_array_t struct -- we don't attempt to convert the array elements into their memory
 *              datatype because we don't know what that type might be yet.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, March 5, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_array_convert_fm(hid_t src_type,             /* Source data type (should be ss_array_tf) */
                    hid_t UNUSED dst_type,      /* Destination data type (should be ss_array_tm) */
                    H5T_cdata_t *cdata,         /* Data type conversion control data */
                    size_t narrays,             /* Number of arrays to convert */
                    size_t buf_stride,          /* Distance between elements in bytes */
                    size_t UNUSED bkg_stride,   /* Distance between elements in the background buffer. */
                    void *buf,                  /* The buffer which is to be converted in place. */
                    void UNUSED *bkg,           /* The background buffer. */
                    hid_t UNUSED dxpl           /* Dataset transfer property list. */
                    )
{
    SS_ENTER(ss_array_convert_fm, herr_t);
    char                *src=NULL;              /* Pointer to array in file format */
    char                *dst=NULL;              /* Pointer to array in memory format */
    hsize_t             arrayno;                /* Index through arrays to be converted */
    size_t              src_size;               /* Size of the array in file format */
    size_t              elmt_fsize;             /* Size of the array elements in file format */
    size_t              nbytes;                 /* Number of bytes to represent all the array elements in the file */
    ss_array_t          tmp;

    SS_ASSERT(cdata);

    switch (cdata->command) {
    case H5T_CONV_INIT:
        /* See ss_array_init() for definitions of these types */
        SS_ASSERT(H5Tequal(dst_type, ss_array_tm));
        SS_ASSERT(H5Tequal(src_type, ss_array_tf));
        SS_ASSERT(H5Tget_size(dst_type)==sizeof(ss_array_t));
        SS_ASSERT(H5Tget_size(src_type)==2*H5Tget_size(ss_string_tf)); /*the array elements and the datatype thereof*/
        SS_ASSERT(H5Tget_size(dst_type)>=H5Tget_size(src_type)); /*implies right-to-left conversion*/
        break;

    case H5T_CONV_FREE:
        break;

    case H5T_CONV_CONV:
        if (0==(src_size=H5Tget_size(src_type))) SS_ERROR(HDF5);
        src = (char*)buf + (narrays-1) * (buf_stride ? buf_stride : src_size);
        dst = (char*)buf + (narrays-1) * (buf_stride ? buf_stride : sizeof tmp);
        for (arrayno=0; arrayno<narrays; arrayno++) {
            memset(&tmp, 0, sizeof tmp);
            
            /* Convert the variable length strings from file to memory format */
            if (ss_string_convert_fm2(src, &(tmp.fbuf), NULL)<0) SS_ERROR(FAILED);
            if (ss_string_convert_fm2(src+H5Tget_size(ss_string_tf), &(tmp.enc_ftype), NULL)<0) SS_ERROR(FAILED);

            /* Decode the target (file) datatype regardless of how many array elements exist */
            if (SS_NOSIZE==(nbytes=ss_string_memlen(&(tmp.enc_ftype)))) SS_ERROR(FAILED);
            if (nbytes>0 && (tmp.ftype=H5decode(ss_string_ptr(&(tmp.enc_ftype)), NULL))<0) SS_ERROR(FAILED);
            if (0==(elmt_fsize=H5Tget_size(tmp.ftype>0?tmp.ftype:ss_pers_tf))) SS_ERROR(HDF5);

            /* Count how many elements are defined (data size divided by datatype size) */
            if (SS_NOSIZE==(nbytes=ss_string_memlen(&(tmp.fbuf)))) SS_ERROR(FAILED);
            SS_ASSERT(0==nbytes % elmt_fsize);
            tmp.nelmts = nbytes / elmt_fsize;

            /* Convert array elements to memory representation if they're persistent object links */
            if (0==tmp.ftype) ss_array_cache(&tmp, -1);

            /* Move result to `buf' and retreat pointers */
            memcpy(dst, &tmp, sizeof tmp);
            src -= buf_stride ? buf_stride : src_size;
            dst -= buf_stride ? buf_stride : sizeof tmp;
        }
        break;

    default:
        SS_ERROR_FMT(HDF5, ("unexpected value for cdata->command"));
    }
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Checksum variable length arrays
 *
 * Description: Free ss_array_t objects.
 *
 * Return:      Returns non-negative on success; negative on failure.  On success, memory is freed
 *
 * Parallel:    Independent
 *
 * Programmer:  Erik Illescas
 *              Monday, May 2, 2005
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_res_cb(void *buffer,                   /* Address of first object to free. */
                size_t size,                    /* Size in bytes of each element of BUFFER. */
                size_t narrays                  /* Number of variable length arrays to free. */
                )
{
    SS_ENTER(ss_array_res_cb, herr_t);
    ss_array_t          *array = (ss_array_t*)buffer;
    size_t              i;

    SS_ASSERT(size==sizeof(ss_array_t));

    for (i=0; i<narrays; i++, array++) {
         ss_array_reset(array);
    }
                                                                                                                                                                                                                                                                               
SS_CLEANUP:
    SS_LEAVE(0);
}



/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Checksum variable length arrays
 *
 * Description: Compute a checksum for zero or more ss_array_t objects.
 *
 * Return:      Returns non-negative on success; negative on failure.  On success, CKSUM is updated by folding in the new
 *              checksum value.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, March 5, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_array_sum_cb(void *buffer,                   /* Address of first object to checksum. */
                size_t UNUSED size,             /* Size in bytes of each element of BUFFER. */
                size_t narrays,                 /* Number of variable length arrays to checksum. */
                ss_val_cksum_t *cksum           /* INOUT: Checksum value to be updated. */
                )
{
    SS_ENTER(ss_array_sum_cb, herr_t);
    ss_array_t          *array = (ss_array_t*)buffer;
    size_t              i, elmt_fsize=0, elmt_msize=0;
    char                *serbuf=NULL;
    size_t              serbuf_nused=0, serbuf_nalloc=0;
    void                *convbuf=NULL;
    
    SS_ASSERT(cksum);
    SS_ASSERT(size==sizeof(ss_array_t));

    for (i=0; i<narrays; i++, array++) {
        if (0==array->ftype) {
            /* Checksum persistent objects. Be careful because if the array size changed then the array->mbuf buffer might not
             * be the right size. */
            if (array->nelmts) {
                if (0==(elmt_msize=H5Tget_size(ss_pers_tm))) SS_ERROR(HDF5);
                if (!array->mbuf && NULL==(array->mbuf = calloc(array->nelmts, elmt_msize))) SS_ERROR(RESOURCE);
                if (ss_pers_sum_cb(array->mbuf, sizeof(ss_pers_t), array->nelmts, cksum)<0) SS_ERROR(FAILED);
            }
        } else {
            /* Compute checksum of elements in target (file) format. We must also checksum the target datatype because if the
             * array is empty then its target type is the only thing to distinguish it from other empty arrays. We have to do
             * all this without modifying the array, which was passed in with `const' since checksumming isn't expected to
             * modify the object (see ss_pers_sum_cb()). */

            /* The target (file) datatype */
            if (array->dirty) {
                if (NULL==(serbuf=H5encode(array->ftype, serbuf, &serbuf_nused, &serbuf_nalloc))) SS_ERROR(HDF5);
                cksum->adler = ss_adler32(cksum->adler, serbuf, serbuf_nused);
                serbuf = SS_FREE(serbuf);
                serbuf_nused = serbuf_nalloc = 0;
            } else {
                cksum->adler = ss_adler32(cksum->adler, ss_string_ptr(&(array->enc_ftype)), ss_string_memlen(&(array->enc_ftype)));
            }

            /* The data in file format */
            if (array->dirty && array->mbuf && array->nelmts) {
                SS_ASSERT(array->mtype>0);
                if (0==(elmt_msize=H5Tget_size(array->mtype))) SS_ERROR(HDF5);
                if (0==(elmt_fsize=H5Tget_size(array->ftype))) SS_ERROR(HDF5);
                if (NULL==(convbuf=malloc(array->nelmts*MAX(elmt_msize,elmt_fsize)))) SS_ERROR(RESOURCE);
                memcpy(convbuf, array->mbuf, array->nelmts*elmt_msize);
                if (H5Tconvert(array->mtype, array->ftype, array->nelmts, convbuf, NULL, H5P_DEFAULT)<0) SS_ERROR(HDF5);
                cksum->adler = ss_adler32(cksum->adler, convbuf, array->nelmts*elmt_fsize);
                convbuf = SS_FREE(convbuf);
            } else {
                cksum->adler = ss_adler32(cksum->adler, ss_string_ptr(&(array->fbuf)), ss_string_memlen(&(array->fbuf)));
            }
        }
    }

SS_CLEANUP:
    SS_FREE(serbuf);
    SS_FREE(convbuf);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Compare variable length arrays
 *
 * Description: Pairwise comparison of zero or more objects of the BUF array with the KEY array.  The KEY describes the value
 *              for which we're searching and the FLAGS says how to search.
 *
 * Return:      On success returns negative if BUF is less than KEY, zero if they are equal, and one if BUF is greater than
 *              KEY. Returns -2 on failure.
 *
 * Issue:       The only form of comparison allowed is equality implied when both arrays have the same number of elements and
 *              those elements are component-wise equal by the default comparison. Eventually we might need additional
 *              comparison operators. [rpm 2004-04-27]
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, March 5, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_array_cmp_cb(void *abuf,                     /* Haystack array. */
                void *akey,                     /* Array of values for which we are searching. */
                unsigned UNUSED flags,          /* How to search (see SS_VAL_CMP). */
                size_t UNUSED size,             /* Size of each object in the BUF and KEY arrays. */
                size_t narrays                  /* Number of variable length arrays in the BUF and KEY arrays. */
                )
{
    SS_ENTER(ss_array_cmp_cb, int);
    ss_array_t          *buf=(ss_array_t*)abuf;
    ss_array_t          *key=(ss_array_t*)akey;
    size_t              i, elmtno;
    int                 retval=0;
    ss_pers_t           p1, p2;

    SS_RETVAL(-2);

    SS_ASSERT(size==sizeof(ss_array_t));
    SS_ASSERT(SS_VAL_CMP_DFLT==flags);
    for (i=0, retval=0; i<narrays && 0==retval; i++, buf++, key++) {
        if (buf->nelmts < key->nelmts) {
            retval = -1;
        } else if (buf->nelmts > key->nelmts) {
            retval = 1;
        } else if (0==buf->ftype && 0!=key->ftype) {
            retval = -1;
        } else if (0!=buf->ftype && 0==key->ftype) {
            retval = 1;
        } else if (0==buf->ftype && 0==key->ftype) {
            /* Both arrays store links to other objects. Do a shallow comparison */
            for (elmtno=0; elmtno<buf->nelmts && 0==retval;  elmtno++) {
                retval = ss_pers_cmp(ss_array_get(buf, -1, elmtno, 1, &p1), ss_array_get(key, -1, elmtno, 1, &p2), NULL);
            }
        } else if (0!=(retval=H5Tcmp(buf->ftype, key->ftype))) {
            /*void*/
        } else {
            SS_ERROR(NOTIMP);
        }
    }
    retval = MINMAX(-1, retval, 1);

SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Copy variable length arrays
 *
 * Description: Copies NELMTS elements each of size SIZE bytes from _SRC to _DST. Same semantics as memcpy(). It is permissible
 *              for _DST and _SRC to have the same pointer value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, November 10, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_array_copy_cb(void *_src, void *_dst, size_t UNUSED size, size_t narrays)
{
    SS_ENTER(ss_array_copy_cb, herr_t);
    ss_array_t *dst = (ss_array_t*)_dst;
    ss_array_t *src = (ss_array_t*)_src;

    if (!src) src=dst;
    SS_ASSERT(sizeof(ss_array_t)==size);
    while (narrays>0) {
        if (src!=dst) *dst = *src;
        if (ss_string_realloc(&(dst->fbuf))<0) SS_ERROR(FAILED);
        if (ss_string_realloc(&(dst->enc_ftype))<0) SS_ERROR(FAILED);
        if (dst->ftype>0 && (dst->ftype=H5Tcopy(dst->ftype))<0) SS_ERROR(HDF5);
        if (dst->mtype>0 && (dst->mtype=H5Tcopy(dst->mtype))<0) SS_ERROR(HDF5);
        if (dst->mbuf) {
            size_t elmt_msize;
            void *x;
            if (0==(elmt_msize=H5Tget_size(dst->mtype>0?dst->mtype:ss_pers_tm))) SS_ERROR(HDF5);
            if (NULL==(x=malloc(dst->nelmts * elmt_msize))) SS_ERROR(RESOURCE);
            memcpy(x, dst->mbuf, dst->nelmts*elmt_msize);
            dst->mbuf = x;
        }
        dst->dirty = TRUE;
        dst++;
        src++;
        --narrays;
    }
    
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Encode variable length array elements
 *
 * Description: Encodes part of an object that cannot be sent directly to another task by copying it into a serialization
 *              buffer, SERBUF, which is allocated and/or extended as necessary.  In the case of variable length arrays, we
 *              have to encode the entire contents of the array elements because the MPI datatype only describes the
 *              ss_array_t struct and not the elements to which it points.
 *
 * Return:      On success, returns a pointer to the possibly reallocated SERBUF; the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 10, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
ss_array_encode_cb(void *buffer,                        /* Array of objects to be encoded. */
                   char *serbuf,                        /* Buffer into which to place results. */
                   size_t *serbuf_nused,                /* INOUT: Number of bytes actually used in SERBUF. */
                   size_t *serbuf_nalloc,               /* INOUT: Number of bytes allocated for SERBUF. */
                   size_t UNUSED size,                  /* Size of each object in BUFFER. */
                   size_t narrays                       /* Number of variable length arrays in BUFFER. */
                   )
{
    SS_ENTER(ss_array_encode_cb, charP);
    size_t              i, nbytes;
    ss_array_t          *array = (ss_array_t*)buffer;
    size_t              elmt_msize=0, elmt_fsize=0;
    
    SS_ASSERT(sizeof(ss_array_t)==size);

    for (i=0; i<narrays; i++, array++) {
        /* Encode the target datatype (even if zero or negative). If the dirty bit isn't set then we already have an encoded
         * version of this. */
        if (array->dirty) {
            if (NULL==(serbuf=H5encode(array->ftype, serbuf, serbuf_nused, serbuf_nalloc))) SS_ERROR(HDF5);
        } else if (array->ftype>0) {
            if (SS_NOSIZE==(nbytes=ss_string_memlen(&(array->enc_ftype)))) SS_ERROR(FAILED);
            SS_EXTEND(serbuf, *serbuf_nused+nbytes, *serbuf_nalloc);
            memcpy(serbuf+(*serbuf_nused), ss_string_ptr(&(array->enc_ftype)), nbytes);
            *serbuf_nused += nbytes;
        } else {
            if (NULL==(serbuf=H5encode(H5I_INVALID_HID, serbuf, serbuf_nused, serbuf_nalloc))) SS_ERROR(HDF5);
        }

        /* Encode the number of elements */
        SS_H5_ENCODE(serbuf, 4, array->nelmts);

        /* Encode the data */
        if (0==array->nelmts) continue;
        if (0==array->ftype) {
            SS_ASSERT(0==array->mtype);
            if (NULL==(serbuf=ss_pers_encode_cb(array->mbuf, serbuf, serbuf_nused, serbuf_nalloc, sizeof(ss_pers_t),
                                                array->nelmts)))
                SS_ERROR(FAILED);
        } else if (array->dirty && array->mbuf) {
            if (0==(elmt_msize=H5Tget_size(array->mtype))) SS_ERROR(HDF5);
            if (0==(elmt_fsize=H5Tget_size(array->ftype))) SS_ERROR(HDF5);
            SS_EXTEND(serbuf, *serbuf_nused+array->nelmts*MAX(elmt_msize,elmt_fsize), *serbuf_nalloc);
            memcpy(serbuf+(*serbuf_nused), array->mbuf, array->nelmts*elmt_msize);
            if (H5Tconvert(array->mtype, array->ftype, array->nelmts, serbuf+(*serbuf_nused), NULL, H5P_DEFAULT)<0)
                SS_ERROR(HDF5);
            *serbuf_nused += array->nelmts * elmt_fsize;
        } else {
            SS_EXTEND(serbuf, *serbuf_nused+array->nelmts*elmt_msize, *serbuf_nalloc);
            memcpy(serbuf+(*serbuf_nused), ss_string_ptr(&(array->fbuf)), array->nelmts*elmt_fsize);
            *serbuf_nused += array->nelmts * elmt_fsize;
        }
    }

SS_CLEANUP:
    SS_LEAVE(serbuf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Variable Length Arrays
 * Purpose:     Decode variable length arrays
 *
 * Description: Decodes stuff encoded by ss_array_encode_cb().
 *
 * Return:      Returns total number of bytes consumed from SERBUF on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 10, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_array_decode_cb(void *buffer,                        /* Array of objects into which to decode SERBUF. */
                   const char *serbuf,                  /* Encoded information to be decoded. */
                   size_t size,                         /* Size of each element in BUFFER array. */
                   size_t narrays                       /* Number of elements in BUFFER array. */
                   )
{
    SS_ENTER(ss_array_decode_cb, size_t);
    size_t              i, retval=0, nelmts, nbytes, elmt_msize, elmt_fsize;
    ss_array_t          *array=(ss_array_t*)buffer;
    const char          *rest=NULL;
    hid_t               ftype=-1;

    SS_ASSERT(sizeof(ss_array_t)==size);
    for (i=0; i<narrays; i++, array++) {
        /* Decode the target datatype */
        if ((ftype=H5decode(serbuf, &rest))<0) {
            SS_STATUS_OK; /*probably passed an zero hid_t as the ftype to the encoder*/
            ftype = 0;
        }
        SS_ASSERT(rest && rest>=serbuf);
        retval += rest-serbuf;
        serbuf = rest;
        if ((!ftype && array->ftype) ||
            (ftype && !array->ftype) ||
            (ftype && array->ftype && !H5Tequal(ftype, array->ftype))) {
            /* Decoded datatype is different than previous datatype */
            if (array->ftype>0) H5Tclose(array->ftype);
            array->ftype = ftype;
            ftype = -1;
            array->mbuf = SS_FREE(array->mbuf);
            if (array->mtype>0) H5Tclose(array->mtype);
            array->mtype = 0;
            array->nelmts = 0;
            array->dirty = TRUE;
        } else {
            /* Decoded datatype matches previous datatype */
            if (ftype) H5Tclose(ftype);
            ftype = -1;
        }

        /* Decode the number of elements */
        SS_H5_DECODE(serbuf, 4, nelmts);
        retval += 4;
        if (nelmts!=array->nelmts && ss_array_resize(array, nelmts)<0) SS_ERROR(FAILED);

        /* Decode the data */
        if (0==array->nelmts) continue;
        if (0==array->ftype) {
            SS_ASSERT(0==array->mtype);
            if (0==(elmt_fsize=H5Tget_size(ss_pers_tf))) SS_ERROR(HDF5);
            elmt_msize = sizeof(ss_pers_t);
            if (NULL==(array->mbuf=realloc(array->mbuf, array->nelmts*MAX(elmt_msize, elmt_fsize)))) SS_ERROR(RESOURCE);
            nbytes = ss_pers_decode_cb(array->mbuf, serbuf, sizeof(ss_pers_t), array->nelmts);
            if (SS_NOSIZE==nbytes) SS_ERROR(FAILED);
            serbuf += nbytes;
            retval += nbytes;
            array->dirty = TRUE;
        } else {
            if (0==(elmt_fsize=H5Tget_size(array->ftype))) SS_ERROR(HDF5);
            array->mbuf = SS_FREE(array->mbuf);
            ss_string_memset(&(array->fbuf), serbuf, array->nelmts*elmt_fsize);
            serbuf += array->nelmts * elmt_fsize;
            retval += array->nelmts * elmt_fsize;
        }
    }

SS_CLEANUP:
    memset(buffer, 0, size*narrays); /*make them all empty arrays on failure*/
    SS_LEAVE(retval);
}
