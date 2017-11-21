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
#ifndef SS_HAVE_SSARRAY_H
#define SS_HAVE_SSARRAY_H

/* A variable length array of persistent object links or other user-defined datatype. */
typedef struct ss_array_t {
    /* Persistent stuff. These two strings should be adjacent to one another */
    ss_string_t fbuf;           /* Links stored as stored in file format. */
    ss_string_t enc_ftype;      /* Encoded datatype of array elements (or empty for ss_pers_t) */

    /* Transient stuff */
    size_t      nelmts;         /* Number of elements defined in the variable length array. We can calculate this because the
                                 * `fbuf' string has a length which can be divided by the size of the ss_pers_tf type. */
    void        *mbuf;          /* Persistent object links cached in memory, type is `mtype' */
    hid_t       mtype;          /* Datatype of elements in `mbuf' (zero if no memory data cached or type is ss_pers_tm) */
    hbool_t     dirty;          /* True if links were modified in memory but `fbuf' string is not updated yet. */
    hid_t       ftype;          /* Target (file) datatype of the array elements. Zero iff type is ss_pers_tf. */
} ss_array_t;

/* Memory and file datatypes for variable length arrays  -- initialized in ss_array_init() */
extern hid_t ss_array_tm;
extern hid_t ss_array_tf;

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
herr_t ss_array_init(void);
herr_t ss_array_cache(ss_array_t *array, hid_t mtype);
herr_t ss_array_clean(ss_array_t *array);
herr_t ss_array_target(ss_array_t *array, hid_t ftype);
hid_t ss_array_targeted(ss_array_t *array);
herr_t ss_array_resize(ss_array_t *array, size_t nelmts);
void *ss_array_get(ss_array_t *array, hid_t mtype, size_t offset, size_t nelmts, void *buffer);
herr_t ss_array_put(ss_array_t *array, hid_t mtype, size_t offset, size_t nelmts, const void *value);
herr_t ss_array_reset(ss_array_t *array);
size_t ss_array_nelmts(const ss_array_t *array);
herr_t ss_array_convert_mf(hid_t src_type, hid_t dst_type, H5T_cdata_t *cdata, size_t nelmts, size_t buf_stride,
                           size_t bkg_stride, void *buf, void *bkg, hid_t dxpl);
herr_t ss_array_convert_fm(hid_t src_type, hid_t dst_type, H5T_cdata_t *cdata, size_t nelmts, size_t buf_stride,
                           size_t bkg_stride, void *buf, void *bkg, hid_t dxpl);
herr_t ss_array_res_cb(void *buffer, size_t size, size_t narrays);
herr_t ss_array_sum_cb(void *buffer, size_t size, size_t narrays, ss_val_cksum_t *cksum);
int ss_array_cmp_cb(void *abuf, void *akey, unsigned flags, size_t size, size_t nelmts);
herr_t ss_array_copy_cb(void *_src, void *_dst, size_t size, size_t narrays);
char *ss_array_encode_cb(void *buffer, char *serbuf, size_t *serbuf_nused, size_t *serbuf_nalloc,
                         size_t size, size_t narrays);
size_t ss_array_decode_cb(void *buffer, const char *serbuf, size_t size, size_t narrays);

#ifdef __cplusplus
}
#endif

#endif /* !SS_HAVE_SSSTRING_H */
