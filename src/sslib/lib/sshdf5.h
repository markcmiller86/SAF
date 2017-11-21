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
#ifndef SS_HAVE_SSHDF5_H
#define SS_HAVE_SSHDF5_H

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     HDF5
 * Purpose:     Declare a file to be transient
 * Description: When this bit is passed as the !/flags/ argument to ss_file_open() or ss_file_create() then a no-op HDF5 file is
 *              created.  Since HDF5 doesn't support this functionality, SSlib simply notes that there is no underlying HDF5
 *              file.
 * Issues:      We commandeer a high-order bit for our purposes, knowing that the H5F API uses the low-order bits. This
 *              bit will never make it into HDF5, but we need to insure that it doesn't conflict with the other H5F_ACC bits.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define H5F_ACC_TRANSIENT       0x01000000

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     HDF5
 * Purpose:     Extra native datatypes
 * Description: These are useful native datatypes that are missing from HDF5.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define H5T_NATIVE_SIZE                 ss_hdf5_init1(&ss_hdf5_native_size_g)
#define H5T_NATIVE_HID                  ss_hdf5_init1(&ss_hdf5_native_hid_g)
#define H5T_NATIVE_VOIDP                ss_hdf5_init1(&ss_hdf5_native_voidp_g)
#define H5T_NATIVE_MPI_COMM             ss_hdf5_init1(&ss_hdf5_native_mpi_comm_g)
#define H5T_NATIVE_MPI_INFO             ss_hdf5_init1(&ss_hdf5_native_mpi_info_g)
extern hid_t ss_hdf5_native_size_g;
extern hid_t ss_hdf5_native_hid_g;
extern hid_t ss_hdf5_native_voidp_g;
extern hid_t ss_hdf5_native_mpi_comm_g;
extern hid_t ss_hdf5_native_mpi_info_g;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Version number
 * Description: An easier to use version number for #if directives.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define H5_VERS_NUMBER  (H5_VERS_MAJOR*1000000 + H5_VERS_MINOR*1000 + H5_VERS_RELEASE)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Serialize a number
 * Description: Serializes _VAL_ into _NBYTES_ of _BUF_, reallocating _BUF_ if necessary.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_H5_ENCODE(_buf_, _nbytes_, _val_) do {                                                                              \
    int _encode_i;                                                                                                             \
    SS_EXTEND(_buf_, *_buf_##_nused+_nbytes_, *_buf_##_nalloc);                                                                \
    assert(0==(hsize_t)(_val_)>>(8*(_nbytes_)));                                                                               \
    for (_encode_i=0; _encode_i<(_nbytes_); _encode_i++) {                                                                     \
        ((unsigned char*)_buf_)[(*_buf_##_nused)++] = ((size_t)(_val_)>>(8*_encode_i)) & 0xff;                                 \
    }                                                                                                                          \
} while (false);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     HDF5
 * Purpose:     Unserialize a number
 * Description: Unserializes _NBYTES_ of _BUF_ into _VAL_ and increments _BUF_.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SS_H5_DECODE(_buf_, _nbytes_, _val_)                                                                                   \
do {                                                                                                                           \
    size_t _decode_i;                                                                                                          \
    memset(&(_val_), 0, sizeof(_val_)); /*done this way to avoid enum/int mixed warning*/                                      \
    for (_decode_i=0; _decode_i<(size_t)(_nbytes_); _decode_i++) {                                                             \
        _val_ |= ((size_t)*((const unsigned char*)_buf_++)) << (8*_decode_i);                                                  \
    }                                                                                                                          \
} while (false);

#ifdef __cplusplus
extern "C" {
#endif

herr_t H5Tmpi(hid_t type, MPI_Datatype *result);
herr_t ss_hdf5_init(void);
hid_t ss_hdf5_init1(hid_t *to_return);
herr_t H5Sselect_slab(hid_t space, H5S_seloper_t selop, hsize_t buf_start, const hsize_t *start, const hsize_t *count);
char *H5encode(hid_t hid, char *serbuf, size_t *serbuf_nused, size_t *serbuf_nalloc);
hid_t H5decode(const char *serbuf, const char **rest);
int H5Tcmp(hid_t t1, hid_t t2);

#ifdef __cplusplus
}
#endif

#endif /* !SS_HAVE_SSHDF5_H */
