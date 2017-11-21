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
SS_IF(hdf5);

hid_t ss_hdf5_native_size_g             = -1;
hid_t ss_hdf5_native_hid_g              = -1;
hid_t ss_hdf5_native_voidp_g            = -1;
hid_t ss_hdf5_native_mpi_comm_g         = -1;
hid_t ss_hdf5_native_mpi_info_g         = -1;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     HDF5
 *
 * Description: These functions provide features that are missing from HDF5.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Interface initializer
 *
 * Description: Initializes the HDF5 compatibility layer of SSlib.
 *
 * Return:      Returns non-negative on success, negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, July 29, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_hdf5_init(void)
{
    hid_t       x;
    SS_ENTER_INIT;

    /* The HDF5 datatype for size_t */
    ss_hdf5_native_size_g = x = H5Tcopy(H5T_NATIVE_UINT);
    H5Tset_precision(x, 8*sizeof(size_t));
    H5Tset_size(x, sizeof(size_t));
    H5Tlock(x);

    /* The HDF5 datatype for hid_t */
    ss_hdf5_native_hid_g = x = H5Tcopy(H5T_NATIVE_INT);
    H5Tset_precision(x, 8*sizeof(hid_t));
    H5Tset_size(x, sizeof(hid_t));
    H5Tlock(x);

    /* The HDF5 datatype for void* */
    ss_hdf5_native_voidp_g = x = H5Tcopy(H5T_NATIVE_UINT);
    H5Tset_precision(x, 8*sizeof(void*));
    H5Tset_size(x, sizeof(void*));
    H5Tlock(x);

    /* The HDF5 datatype for MPI_Comm */
    ss_hdf5_native_mpi_comm_g = x = H5Tcopy(H5T_NATIVE_INT);
    H5Tset_precision(x, 8*sizeof(MPI_Comm));
    H5Tset_size(x, sizeof(MPI_Comm));
    H5Tlock(x);

    /* The HDF5 datatype for MPI_Info */
    ss_hdf5_native_mpi_info_g = x = H5Tcopy(H5T_NATIVE_INT);
    H5Tset_precision(x, 8*sizeof(MPI_Info));
    H5Tset_size(x, sizeof(MPI_Info));
    H5Tlock(x);
    
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Initialize global variable
 *
 * Description: This function initializes a global variable belonging to this interface and then returns its value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, August  4, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
ss_hdf5_init1(hid_t *to_return)
{
    SS_ENTER(ss_hdf5_init1, hid_t);             /* This makes sure the global var is initialized. See ss_if_init(). */
    /* Nothing else to do */
    SS_LEAVE(*to_return);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Create equivalent MPI datatype
 *
 * Description: Create an MPI datatype from an HDF5 datatype. HDF5's datatype description abilities far surpass MPI's
 *              abilities, and therefore there are many cases when this function is unable to return a suitable MPI type.
 *
 * Return:      Returns non-negative on success, negative on failure.  The MPI datatype is returned by reference through the
 *              RESULT pointer on success.
 *
 * Parallel:    Independent. Serial behavior is a no-op (it just sets RESULT to zero).
 *
 * Programmer:  Robb Matzke
 *              Thursday, May 15, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
H5Tmpi(hid_t UNUSED_SERIAL type,/* The incoming HDF5 datatype. */
       MPI_Datatype *result     /* The resulting MPI datatype. */
       )
{
    SS_ENTER(H5Tmpi, herr_t);
#ifndef HAVE_PARALLEL
    *result = 0;
#else
    size_t              nbytes;

    /* This isn't implemented, but for now we can just create an opaque MPI type and assume that all tasks
     * have the same data representation. */
    if (0==(nbytes=H5Tget_size(type))) SS_ERROR(HDF5);
    if (MPI_Type_contiguous((int)nbytes, MPI_BYTE, result)) SS_ERROR(MPI);
    if (MPI_Type_commit(result)) SS_ERROR(MPI);
 SS_CLEANUP:
#endif

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Creates a slab selection
 *
 * Description: Given a description of a slab (a slab is a contiguous hyperslab) such as the index of the "top left" (i.e.,
 *              lowest indexed) element and the size of the hyperslab, modify the selection in the supplied data space
 *              according to the selection operator SELOP.
 *
 *              The BUF_START argument is used to adjust the slab's position in the extent so that an alternate pointer can be
 *              passed to H5Dread() or H5Dwrite().  For instance if BUF_START is 100 then the address passed to the H5Dread() or
 *              H5Dwrite() function as the buffer should actually be 100 elements past the beginning of the buffer.  It is an
 *              error if the specified slab includes elements before BUF_START. BUF_START is linear, computed assuming C
 *              element ordering.
 *
 * Example:     Given a dataset of size (11,29) and a buffer in memory representing the element at (3,9) and continuing
 *              linearly in C order until the element at (9,25) inclusive, then three different slabs will produce the
 *              following hyperslab selections in the supplied data space.
 *              
 *                                 1         2
 *                       01234567890123456789012345678     Slab     Start     Size
 *                      +-----------------------------+    -----    -------   -------
 *                     0|.............................|      1      (3,11)    (4,8)
 *                     1|.............................|      2      (4,21)    (3,5)
 *                     2|.............................|      3      (6,2)     (3,3)
 *                     3|.........;;11111111;;;;;;;;;;|      4      (8,6)     (2,18)
 *                     4|;;;;;;;;;;;11111111;;22222;;;|
 *                     5|;;;;;;;;;;;11111111;;22222;;;|
 *                     6|;;333;;;;;;11111111;;22222;;;|
 *                     7|;;333;;;;;;;;;;;;;;;;;;;;;;;;|
 *                     8|;;333;444444444444444444;;;;;|
 *                     9|;;;;;;444444444444444444;;...|
 *                    10|.............................|
 *                      +-----------------------------+
 *                       01234567890123456789012345678
 *                                 1         2
 *
 *              What happens internally is we warp the buffer so its first element is considered to be (0,0) and the size of
 *              the fastest varying dimension remains the size of the dataset in that dimension, but the size of the other
 *              dimensions is based on the ending element of the buffer and rounded up to cause the buffer to be a slab. The
 *              rounding up is only conceptual -- it is an error to try to select elements that would be beyond the actual end
 *              of the buffer.
 *
 *              The size of the hyperslabs remains the same, but their starting positions are adjusted in each dimension so as
 *              to correspond to the beginning of the buffer:
 *
 *                  Slab     Start     Size   
 *                  -----    -------   -------
 *                    1      (0,2)     (4,8)
 *                    2      (1,12)    (3,5)
 *                    3      (2,22)    (3,3)
 *                    4      (4,26)    (2,18) [see below]
 *
 *              Slab 4 is a bit problematic because the adjusted starting offset plus size in the fast varying dimension is
 *              larger than the dataset.  We solve that by dividing that selection in two along that dimension:
 *              
 *                  Slab     Start     Size   
 *                  -----    -------   -------
 *                    4a     (4,26)    (2,3)
 *                    4b     (5,0)     (2,15)
 *
 *              This slab division is performed recursively, so we could end up with 2^(n-1) smaller slabs where N is the
 *              dataset dimensionality. Fortunately our dimensionality seldom exceeds four.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Issue:       The slab should not contain elements after the end of the buffer even if they would still be within the
 *              dataset. This is considered a logic error but is not checked because this function is not given enough
 *              information. SSlib has already checked for this situation elsewhere anyway and so we are just avoiding the
 *              need to keep track of the buffer size in the functions that call this.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, October 28, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
H5Sselect_slab(hid_t space,             /* Data space for the dataset on which we're operating */
               H5S_seloper_t selop,     /* One of the selection operators */
               hsize_t buf_start,       /* Linear base element for start of buffer */
               const hsize_t *start,    /* Optional multidimensional starting position w.r.t. data space extent. If null then
                                         * a start of zero is used. */                                      
               const hsize_t *count     /* Multidimensional size of the slab */
               )
{
    SS_ENTER(H5Sselect_slab, herr_t);
    int         ndims, dim, i;
    hsize_t     one[H5S_MAX_RANK];              /* Constant unit vector */
    hsize_t     dset_size[H5S_MAX_RANK];        /* Size of the dataset */
    hsize_t     adj_start[H5S_MAX_RANK];        /* Adjusted (ie., w.r.t. the buffer) starting position of the slab */
    hsize_t     extra_start[H5S_MAX_RANK];      /* Second starting position when splitting the slab */
    hsize_t     adj_count[H5S_MAX_RANK];        /* First slab size when splitting the slab */
    hsize_t     extra_count[H5S_MAX_RANK];      /* Second slab size when splitting the slab */
    static const hsize_t zero[H5S_MAX_RANK];    /* Zero vector */
    hsize_t     dset_last_1;                    /* Linear address of last element of the dataset */
    hsize_t     slab_start_1;                   /* Linear address of first element of slab w.r.t. the entire dataset */

    if (!start) start = zero;

    /* Copy `count' so we can modify; initialize the `one' vector. */
    if ((ndims = H5Sget_simple_extent_dims(space, dset_size, NULL))<0) SS_ERROR(HDF5);
    SS_ASSERT(ndims>0); /*can't handle scalar slabs since start[] and count[] are empty*/
    for (i=0; i<ndims; i++) {
        adj_count[i] = count[i];
        one[i] = 1;
    }
    
    /* Check for degenerate cases */
    if (0==buf_start) {
        if (H5Sselect_hyperslab(space, selop, start, NULL, one, count)<0) SS_ERROR(HDF5);
        goto done;
    }
    for (dim=0; dim<ndims; dim++)
        if (count[dim]>0) break;
    if (dim==ndims) {
        if (H5Sselect_hyperslab(space, selop, start, NULL, one, count)<0) SS_ERROR(HDF5);
        goto done;
    }
    
    /* We work mostly with linear addresses herein. This also means we can relax a bit below because when we split the
     * hyperslab and increment one of the middle dimension offsets we don't have to worry about carrying when there's an
     * overflow. */
    dset_last_1 = ss_blob_array_linear(ndims, dset_size, NULL, dset_size);
    slab_start_1 = ss_blob_array_linear(ndims, dset_size, start, NULL);

    /* Check that the buffer is entirely within the dataset */
    if (buf_start>dset_last_1) SS_ERROR_FMT(DOMAIN, ("buffer begins past end of dataset"));

    /* Check that the slab is entirely within the buffer */
    if (slab_start_1<buf_start) SS_ERROR_FMT(DOMAIN, ("slab starts before buffer"));

    /* Adjusted starting position of the slab */
    ss_blob_array_multidim(ndims, dset_size, slab_start_1-buf_start, adj_start);

    /* Look at each dimension of the slab to determine whether it needs to be split into two slabs in that dimension, and if
     * so invoke this function recursively to take care of one of those two. */
    for (dim=ndims-1; dim>=0; --dim) {
        if (adj_start[dim]+adj_count[dim]>dset_size[dim]) {
            for (i=0; i<ndims; i++) {
                extra_start[i] = adj_start[i];
                extra_count[i] = adj_count[i];
            }
            SS_ASSERT(dim>0);
            extra_start[dim-1] += 1; /*we'll normalize it in the recursive call*/
            extra_start[dim] = 0;
            extra_count[dim] = adj_start[dim]+adj_count[dim] - dset_size[dim];
            adj_count[dim] = dset_size[dim] - adj_start[dim];

            SS_ASSERT(H5S_SELECT_SET==selop || H5S_SELECT_OR==selop);
            if (H5Sselect_slab(space, selop, (hsize_t)0, extra_start, extra_count)<0) SS_ERROR(FAILED);
            selop = H5S_SELECT_OR;
        }
    }

    /* Add the slab into the data space selection. */
    if (H5Sselect_hyperslab(space, selop, adj_start, NULL, one, adj_count)<0) SS_ERROR(HDF5);

done:
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Encode HDF5 ID's into an array
 *
 * Description: Given an hid_t value and a pointer to a serialization buffer, a starting byte offset into that
 *              buffer, and an indication of how much space has already been allocated for the buffer, this function will
 *              convert the hid_t value into a format that is task independent and which can be unserialized later in order
 *              to build a new hid_t value. This is particularly useful for checksumming a datatype or transmitting an HDF5
 *              datatype description between tasks.
 *
 * Return:      Returns the (possibly reallocated) SERBUF on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, January 30, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
H5encode(hid_t hid,                     /* The hid_t value to encode */
         char *serbuf,                  /* An optional buffer into which the HIDS are encoded. */
         size_t *serbuf_nused,          /* On input (consulted only if SERBUF is non-null) this is the offset into SERBUF
                                         * where the encoding begins; on successful return it is an offset to the first byte
                                         * after the end of the encoding. */
         size_t *serbuf_nalloc          /* Optional: on input (consulted only if SERBUF is non-null) this is the allocated
                                         * size of SERBUF; on successful return it is the new allocated size of SERBUF. */
         )
{
    SS_ENTER(H5encode, charP);
    const unsigned      version=1;
    H5I_type_t          hidtype;
    H5T_class_t         typeclass;
    int                 i;
    unsigned            membno, nmembs;
    size_t              offset, prec, my_nalloc=0;
    size_t              nbytes;
    char                *name=NULL;
    hid_t               subtype=-1;
    size_t              subsize;
    H5T_order_t         order;
    H5T_pad_t           lsb, msb, ipad;
    H5T_sign_t          sign;
    size_t              spos, epos, esize, mpos, msize, ebias;
    H5T_norm_t          norm;
    H5T_cset_t          cset;
    H5T_str_t           spad;
    int                 ndims;
    hsize_t             dim[H5S_MAX_RANK];
    int                 perm[H5S_MAX_RANK] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    if (!serbuf) {
        if (!serbuf_nalloc) serbuf_nalloc = &my_nalloc;
        *serbuf_nused = *serbuf_nalloc = 0;
    } else {
        SS_ASSERT(serbuf_nalloc);
    }

    hidtype = H5Iget_type(hid);
    SS_STATUS_OK;

    /* Encode a version number since we might be storing the serialization long term. */
    SS_H5_ENCODE(serbuf, 1, version);

    /* The class of hid_t */
    SS_H5_ENCODE(serbuf, 1, hidtype<0 ? 0xff : hidtype);
    if (H5I_BADID==hidtype) goto done;

    /* We can only handle datatypes at this time */
    if (H5I_DATATYPE!=hidtype) SS_ERROR(NOTIMP);

    /* The total size of the datatype */
    if (0==(nbytes = H5Tget_size(hid))) SS_ERROR(HDF5);
    SS_H5_ENCODE(serbuf, 4, nbytes);

    /* The datatype class */
    if ((typeclass=H5Tget_class(hid))<0) SS_ERROR(HDF5);
    SS_H5_ENCODE(serbuf, 1, typeclass);

    switch (typeclass) {
    case H5T_COMPOUND:
        /* First we encode the number of members. Then each member encodes an offset, a name, and recursively the type */
        if ((i = H5Tget_nmembers(hid))<0) SS_ERROR(HDF5);
        nmembs = i;
        SS_H5_ENCODE(serbuf, 2, nmembs);
        for (membno=0; membno<nmembs; membno++) {
            offset = H5Tget_member_offset(hid, membno);
            SS_H5_ENCODE(serbuf, 4, offset);
                
            if (NULL==(name = H5Tget_member_name(hid, membno))) SS_ERROR(HDF5);
            nbytes = strlen(name)+1;
            SS_EXTEND(serbuf, *serbuf_nused+nbytes, *serbuf_nalloc);
            strcpy(serbuf + *serbuf_nused, name);
            *serbuf_nused += nbytes;
            name = SS_FREE(name);
                
            if ((subtype = H5Tget_member_type(hid, membno))<0) SS_ERROR(HDF5);
            if (NULL==(serbuf = H5encode(subtype, serbuf, serbuf_nused, serbuf_nalloc))) SS_ERROR(FAILED);
            if (H5Tclose(subtype)<0) SS_ERROR(HDF5);
            subtype = -1;
        }
        break;

    case H5T_INTEGER:
    case H5T_FLOAT:
    case H5T_STRING:
    case H5T_BITFIELD:
        /* Encode order, size, offset, and padding for any of these types. Then encode things that are specific to a type. */
        if ((order = H5Tget_order(hid))<0) SS_ERROR(HDF5);
        SS_H5_ENCODE(serbuf, 1, order);

        prec = H5Tget_precision(hid);
        SS_H5_ENCODE(serbuf, 4, prec);

        if ((i = H5Tget_offset(hid))<0) SS_ERROR(HDF5);
        SS_H5_ENCODE(serbuf, 4, i);

        if (H5Tget_pad(hid, &lsb, &msb)<0) SS_ERROR(HDF5);
        SS_H5_ENCODE(serbuf, 1, lsb);
        SS_H5_ENCODE(serbuf, 1, msb);

        if (H5T_INTEGER==typeclass) {
            if ((sign=H5Tget_sign(hid))<0) SS_ERROR(HDF5);
            SS_H5_ENCODE(serbuf, 1, sign);
        } else if (H5T_FLOAT==typeclass) {
            if (H5Tget_fields(hid, &spos, &epos, &esize, &mpos, &msize)<0) SS_ERROR(HDF5);
            SS_H5_ENCODE(serbuf, 1, spos);
            SS_H5_ENCODE(serbuf, 1, epos);
            SS_H5_ENCODE(serbuf, 1, esize);
            SS_H5_ENCODE(serbuf, 1, mpos);
            SS_H5_ENCODE(serbuf, 1, msize);

            ebias = H5Tget_ebias(hid);
            SS_H5_ENCODE(serbuf, 4, ebias);
            
            if ((norm=H5Tget_norm(hid))<0) SS_ERROR(HDF5);
            SS_H5_ENCODE(serbuf, 1, norm);
            
            if ((ipad=H5Tget_inpad(hid))<0) SS_ERROR(HDF5);
            SS_H5_ENCODE(serbuf, 1, ipad);
        } else if (H5T_STRING==typeclass) {
            if ((cset = H5Tget_cset(hid))<0) SS_ERROR(HDF5);
            SS_H5_ENCODE(serbuf, 1, cset);

            if ((spad = H5Tget_strpad(hid))<0) SS_ERROR(HDF5);
            SS_H5_ENCODE(serbuf, 1, spad);
        }
        break;

    case H5T_ENUM:
        /* Recursively encode the parent type. Then encode the total number of elements followed by the name and value of
             * each element. */
        if ((subtype=H5Tget_super(hid))<0) SS_ERROR(HDF5);
        if (NULL==(serbuf=H5encode(subtype, serbuf, serbuf_nused, serbuf_nalloc))) SS_ERROR(FAILED);
        subsize = H5Tget_size(subtype);
        if (H5Tclose(subtype)<0) SS_ERROR(HDF5);

        if ((i = H5Tget_nmembers(hid))<0) SS_ERROR(HDF5);
        nmembs = i;
        SS_H5_ENCODE(serbuf, 2, nmembs);
        for (membno=0; membno<nmembs; membno++) {
            if (NULL==(name=H5Tget_member_name(hid, membno))) SS_ERROR(HDF5);
            nbytes = strlen(name)+1;
            SS_EXTEND(serbuf, *serbuf_nused+nbytes, *serbuf_nalloc);
            strcpy(serbuf + *serbuf_nused, name);
            *serbuf_nused += nbytes;
            name = SS_FREE(name);

            SS_EXTEND(serbuf, *serbuf_nused+subsize, *serbuf_nalloc);
            if (H5Tget_member_value(hid, membno, serbuf + *serbuf_nused)<0) SS_ERROR(HDF5);
            *serbuf_nused += subsize;
        }
        break;

    case H5T_OPAQUE:
        /* Encode the tag name of the opaque type */
        if (NULL==(name=H5Tget_tag(hid))) SS_ERROR(HDF5);
        nbytes = strlen(name)+1;
        SS_EXTEND(serbuf, *serbuf_nused+nbytes, *serbuf_nalloc);
        strcpy(serbuf + *serbuf_nused, name);
        *serbuf_nused += nbytes;
        name = SS_FREE(name);
        break;

    case H5T_ARRAY:
        /* Recursively encode the parent type. Then encode the dimensionality and size of each dimension. */
        if ((subtype=H5Tget_super(hid))<0) SS_ERROR(HDF5);
        if (NULL==(serbuf=H5encode(subtype, serbuf, serbuf_nused, serbuf_nalloc))) SS_ERROR(FAILED);
        subsize = H5Tget_size(subtype);
        if (H5Tclose(subtype)<0) SS_ERROR(HDF5);

        if ((ndims = H5Tget_array_ndims(hid))<0) SS_ERROR(HDF5);
        SS_H5_ENCODE(serbuf, 1, ndims);

        if (H5Tget_array_dims1(hid, dim, 0)<0) SS_ERROR(HDF5);
        for (i=0; i<ndims; i++) {
            SS_H5_ENCODE(serbuf, 2, dim[i]);
            SS_H5_ENCODE(serbuf, 1, perm[i]);
        }
        break;

    case H5T_TIME:
    case H5T_REFERENCE:
    case H5T_VLEN:
        SS_ERROR(NOTIMP);

    case H5T_NO_CLASS:
    case H5T_NCLASSES:
        SS_ERROR(HDF5);
    }

done:
SS_CLEANUP:
    SS_LEAVE(serbuf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Decode HDF5 ID's into an array
 *
 * Description: Given a pointer to a serialization buffer, decode the hid_t that was previously encoded there.
 *
 * Return:      Returns the decoded hid_t on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, January 30, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
H5decode(const char *serbuf,            /* The encoded hid_t values */
         const char **rest              /* OUT: The first byte of SERBUF that was not consumed. This argument may be a null
                                         * pointer in which case the information is not returned. */
         )
{
    SS_ENTER(H5decode, hid_t);
    unsigned            version;
    hid_t               hid=-1, subtype=-1;
    H5I_type_t          hidtype;
    H5T_class_t         typeclass;
    int                 i;
    unsigned            membno, nmembs;
    size_t              offset, prec;
    size_t              nbytes;
    const char          *name=NULL;
    size_t              subsize;
    H5T_order_t         order;
    H5T_pad_t           lsb, msb, ipad;
    H5T_sign_t          sign;
    size_t              spos, epos, esize, mpos, msize, ebias;
    H5T_norm_t          norm;
    H5T_cset_t          cset;
    H5T_str_t           spad;
    int                 ndims;
    hsize_t             dim[H5S_MAX_RANK];
    int                 perm[H5S_MAX_RANK];

    if (!serbuf) SS_ERROR_FMT(USAGE, ("no serialized buffer supplied"));

    /* The version number */
    SS_H5_DECODE(serbuf, 1, version);
    if (1!=version) SS_ERROR(NOTIMP);

    /* The class of hid_t */
    SS_H5_DECODE(serbuf, 1, hidtype);
    if (H5I_BADID==hidtype) {
        hid = H5I_INVALID_HID;
        goto done;
    }

    /* We can only handle datatypes at this time */
    if (H5I_DATATYPE!=hidtype) SS_ERROR(NOTIMP);

    /* Stuff common to all types */
    SS_H5_DECODE(serbuf, 4, nbytes);
    SS_H5_DECODE(serbuf, 1, typeclass);

    switch (typeclass) {
    case H5T_COMPOUND:
        SS_H5_DECODE(serbuf, 2, nmembs);
        if ((hid = H5Tcreate(typeclass, nbytes))<0) SS_ERROR(HDF5);
        for (membno=0; membno<nmembs; membno++) {
            SS_H5_DECODE(serbuf, 4, offset);
            name = serbuf; serbuf+=strlen(name)+1;
            if ((subtype=H5decode(serbuf, &serbuf))<0) SS_ERROR(FAILED);
            if (H5Tinsert(hid, name, offset, subtype)<0) SS_ERROR(HDF5);
            if (H5Tclose(subtype)<0) SS_ERROR(HDF5);
        }
        break;

    case H5T_INTEGER:
    case H5T_FLOAT:
    case H5T_STRING:
    case H5T_BITFIELD:
        SS_H5_DECODE(serbuf, 1, order);
        SS_H5_DECODE(serbuf, 4, prec);
        SS_H5_DECODE(serbuf, 4, offset);
        SS_H5_DECODE(serbuf, 1, lsb);
        SS_H5_DECODE(serbuf, 1, msb);

        if (H5T_INTEGER==typeclass) {
            SS_H5_DECODE(serbuf, 1, sign);
            if ((hid=H5Tcopy(H5T_NATIVE_UCHAR))<0) SS_ERROR(HDF5);
            if (H5Tset_precision(hid, prec)<0) SS_ERROR(HDF5);
            if (H5Tset_size(hid, nbytes)<0) SS_ERROR(HDF5);
            if (H5Tset_order(hid, order)<0) SS_ERROR(HDF5);
            if (H5Tset_offset(hid, offset)<0) SS_ERROR(HDF5);
            if (H5Tset_pad(hid, lsb, msb)<0) SS_ERROR(HDF5);
            if (H5Tset_sign(hid, sign)<0) SS_ERROR(HDF5);
        } else if (H5T_FLOAT==typeclass) {
            SS_H5_DECODE(serbuf, 1, spos);
            SS_H5_DECODE(serbuf, 1, epos);
            SS_H5_DECODE(serbuf, 1, esize);
            SS_H5_DECODE(serbuf, 1, mpos);
            SS_H5_DECODE(serbuf, 1, msize);
            SS_H5_DECODE(serbuf, 4, ebias);
            SS_H5_DECODE(serbuf, 1, norm);
            SS_H5_DECODE(serbuf, 1, ipad);
            if ((hid=H5Tcopy(H5T_NATIVE_FLOAT))<0) SS_ERROR(HDF5);
            if (H5Tset_size(hid, nbytes)<0) SS_ERROR(HDF5);
            if (H5Tset_precision(hid, prec)<0) SS_ERROR(HDF5);
            if (H5Tset_order(hid, order)<0) SS_ERROR(HDF5);
            if (H5Tset_offset(hid, offset)<0) SS_ERROR(HDF5);
            if (H5Tset_pad(hid, lsb, msb)<0) SS_ERROR(HDF5);
            if (H5Tset_fields(hid, spos, epos, esize, mpos, msize)<0) SS_ERROR(HDF5);
            if (H5Tset_ebias(hid, ebias)<0) SS_ERROR(HDF5);
            if (H5Tset_norm(hid, norm)<0) SS_ERROR(HDF5);
            if (H5Tset_inpad(hid, ipad)<0) SS_ERROR(HDF5);
        } else if (H5T_STRING==typeclass) {
            SS_H5_DECODE(serbuf, 1, cset);
            SS_H5_DECODE(serbuf, 1, spad);
            if ((hid=H5Tcopy(H5T_C_S1))<0) SS_ERROR(HDF5);
            if (H5Tset_size(hid, nbytes)<0) SS_ERROR(HDF5);
            if (H5Tset_cset(hid, cset)<0) SS_ERROR(HDF5);
            if (H5Tset_strpad(hid, spad)<0) SS_ERROR(HDF5);
        }
        break;

    case H5T_ENUM:
        if ((subtype=H5decode(serbuf, &serbuf))<0) SS_ERROR(FAILED);
        if ((hid=H5Tenum_create(subtype))<0) SS_ERROR(HDF5);
        SS_H5_DECODE(serbuf, 2, nmembs);
        subsize = H5Tget_size(subtype);
        for (membno=0; membno<nmembs; membno++) {
            name = serbuf; serbuf += strlen(name)+1;
            if (H5Tenum_insert(hid, name, serbuf)<0) SS_ERROR(HDF5);
            serbuf += subsize;
        }
        if (H5Tclose(subtype)<0) SS_ERROR(HDF5);
        break;

    case H5T_OPAQUE:
        name = serbuf; serbuf += strlen(name)+1;
        if ((hid=H5Tcreate(H5T_OPAQUE, nbytes))<0) SS_ERROR(HDF5);
        if (H5Tset_tag(hid, name)<0) SS_ERROR(HDF5);
        break;

    case H5T_ARRAY:
        if ((subtype=H5decode(serbuf, &serbuf))<0) SS_ERROR(FAILED);
        SS_H5_DECODE(serbuf, 1, ndims);
        for (i=0; i<ndims; i++) {
            SS_H5_DECODE(serbuf, 2, dim[i]);
            SS_H5_DECODE(serbuf, 1, perm[i]);
        }
        if ((hid=H5Tarray_create(subtype, ndims, dim, perm))<0) SS_ERROR(FAILED);
        if (H5Tclose(subtype)<0) SS_ERROR(HDF5);
        break;

    case H5T_TIME:
    case H5T_REFERENCE:
    case H5T_VLEN:
        SS_ERROR(NOTIMP);

    case H5T_NO_CLASS:
    case H5T_NCLASSES:
        SS_ERROR(HDF5);
    }

done:
    if (rest) *rest = serbuf;

SS_CLEANUP:
    SS_LEAVE(hid);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     HDF5
 * Purpose:     Compares two datatypes
 *
 * Description: This is essentially a public version of H5T_cmp()
 *
 * Return:      -1, 0, or 1 depending on whether T1 is less than, equal to, or greater than T2. Returns -2 on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, November  1, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
H5Tcmp(hid_t t1, hid_t t2)
{
    SS_ENTER(H5Tcmp, int);

    void        *dt1=NULL, *dt2=NULL;
    int         retval=-2;
    extern void *H5I_object_verify(hid_t, H5I_type_t);
    extern int H5T_cmp(void*, void*);

    SS_RETVAL(-2);

    /* Issue: This function calls internal HDF5 functions for which we have no prototypes. */
    if (NULL==(dt1=H5I_object_verify(t1, H5I_DATATYPE)) || NULL==(dt2=H5I_object_verify(t2, H5I_DATATYPE)))
        SS_ERROR(HDF5);
    retval = H5T_cmp(dt1, dt2);

SS_CLEANUP:
    SS_LEAVE(retval);
}
