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
#ifndef SS_HAVE_SSSTRING_H
#define SS_HAVE_SSSTRING_H

/* A variable length string in memory. See ssstring.c prologue and ss_string_init() for more details. This does not inherit
 * from ss_obj_t because we're trying to keep these as light-weight as possible. Besides, they normally appear as members of
 * other persistent objects anyway (copied rather than pointered). */
typedef struct ss_string_t {
    char        *p;             /* A pointer to the value of the string, or the null pointer if the string has no value. If a
                                 * value is present then if `offset' is non-zero it points directly into the String's dataset
                                 * memory, otherwise `p' is allocated on the heap. */
    size_t      nbytes;         /* The number of bytes in the string's value. If the value is a NUL-terminated C string then
                                 * the NUL character is included in the `nbytes'. */
    size_t     offset;          /* This is the offset into the dataset for the start of the string's value. Since the four
                                 * bytes prior to the start of the value are a four byte little endian length, the offset for
                                 * something in the dataset will never be less than four. */
} ss_string_t;

/* Every scope has a string table where variable length strings are ultimately stored. */
typedef struct ss_string_table_t {
    ss_obj_t    obj;            /* This must be first -- a string table is a kind of SSlib object */
    hid_t       dset;           /* An extendible one-dimensional H5T_NATIVE_CHAR dataset containing the strings. This is zero
                                 * for a transient scope. */                            
    hsize_t     dset_size;      /* Current dataset size. */
    unsigned char *buf;         /* The contents of the dataset plus additional strings that were added later.  The dataset
                                 * always begins with the bytes 0x01, 0x00, 0x00, 0x00, 0x00 so that a string offset of 4
                                 * points to a value consisting of a single NUL character. */
    size_t      buf_nalloc;     /* Number of bytes allocated for `buf', which must be at least as large as `buf_nused'. */
    size_t      buf_nused;      /* Number of bytes actually used in `buf'. */
    size_t      lo_w, hi_w;     /* Low and high water marks for changes; hi_w is first byte _past_ last changed byte */
    size_t      *index;         /* Array of indices into `buf' sorted by string values. */
    size_t      index_nalloc;   /* Number of slots allocated in the `index' array. */
    size_t      index_nused;    /* Number of slots actually used in the `index' array. */
} ss_string_table_t;

/* Memory and file datatypes for strings -- initialized in ss_string_init() */
extern hid_t ss_string_tm;
extern hid_t ss_string_tf;

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
herr_t ss_string_init(void);
char *ss_string_get(const ss_string_t *str, size_t bufsize, char *buf);
herr_t ss_string_set(ss_string_t *str, const char *s);
herr_t ss_string_memset(ss_string_t *str, const void *value, size_t nbytes);
herr_t ss_string_splice(ss_string_t *str, const char *value, size_t start, size_t nbytes, size_t nreplace);
const char *ss_string_ptr(const ss_string_t *str);
herr_t ss_string_reset(ss_string_t *str);
herr_t ss_string_realloc(ss_string_t *str);
int ss_string_cmp(const ss_string_t *s1, const ss_string_t *s2);
int ss_string_cmp_s(const ss_string_t *str, const char *s);
herr_t ss_string_cat(ss_string_t *str, const char *s);
size_t ss_string_len(const ss_string_t *str);
size_t ss_string_memlen(const ss_string_t *str);
ss_string_table_t *ss_string_boot(hid_t scopegrp, MPI_Comm scopecomm, hbool_t create, ss_string_table_t *exists);
herr_t ss_string_desttab(ss_string_table_t *strtab);
herr_t ss_string_flush(ss_string_table_t *strtab, MPI_Comm scopecomm, MPI_Comm filecomm);
herr_t ss_string_convert_mf(hid_t src, hid_t dst, H5T_cdata_t *cdata, size_t nelmts, size_t buf_stride, size_t bkg_stride,
                            void *buf, void *bkg, hid_t dxpl);
herr_t ss_string_convert_mf2(const ss_string_t *src, void *dst, ss_string_table_t *strtab);
herr_t ss_string_convert_fm(hid_t src, hid_t dst, H5T_cdata_t *cdata, size_t nelmts, size_t buf_stride, size_t bkg_stride,
                            void *buf, void *bkg, hid_t dxpl);
herr_t ss_string_convert_fm2(const void *src, ss_string_t *dst, ss_string_table_t *strtab);
herr_t ss_string_res_cb(void *buffer, size_t size, size_t nelmts);
herr_t ss_string_sum_cb(void *buffer, size_t size, size_t nelmts, ss_val_cksum_t *cksum);
int ss_string_cmp_cb(void *buf, void *key, unsigned flags, size_t size, size_t nelmts);
herr_t ss_string_copy_cb(void *_src, void *_dst, size_t size, size_t nelmts);
char *ss_string_encode_cb(void *buffer, char *serbuf, size_t *serbuf_nused, size_t *serbuf_nalloc,
                          size_t size, size_t nelmts);
size_t ss_string_decode_cb(void *buffer, const char *serbuf, size_t size, size_t nelmts);

#ifdef __cplusplus
}
#endif

#endif /* !SS_HAVE_SSSTRING_H */
