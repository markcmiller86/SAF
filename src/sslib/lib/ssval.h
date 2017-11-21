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
#ifndef SS_HAVE_SSVAL_H
#define SS_HAVE_SSVAL_H

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Values
 * Purpose:     Value comparison flags
 *
 * Description: The various comparison functions (such as ss_val_cmp()) can be influenced to use different algorithms during
 *              their operation by passing a bit vector describing the mode of operation.  For ss_val_cmp() the bit vector is
 *              contained in the first non-zero byte of the mask for each value being compared.  Generally, if the flags bits
 *              are unrecognized or unsupported by a particular comparator a default algorithm is used.
 *
 *              *The*Default*Algorithm*
 *
 *              All comparison functions support a notion of a default comparison algorithm. This is indicated by setting all
 *              eight low-order bits in the flag, which can be done with the constant SS_VAL_CMP_DFLT.
 *
 *              *String*Comparisons*
 *
 *              * SS_VAL_CMP_SUBSTR: If the /key/ value is a substring of the /buffer/ being tested then the comparator
 *                                   reports that they are equal. Otherwise the /key/ is always considered to be less than the
 *                                   /buffer/.  Both the /key/ and /buffer/ must be C-style NUL-terminated strings in order
 *                                   for a match to be detected.
 *
 *              * SS_VAL_CMP_SUBMEM: This is similar to SS_VAL_CMP_SUBSTRING except the /key/ and /mask/ are considered to be
 *                                   bytes of memory and all bytes of the /key/ (including NUL bytes if any) must match at
 *                                   some location in /buffer/.  If there is no match then the /key/ is considered to be
 *                                   less than the /buffer/.
 *
 *              * SS_VAL_CMP_RE:     The /key/ value is interpreted as a POSIX regular expression and if that regular
 *                                   expression matches the contents of /buffer/ then the comparator reports that /key/
 *                                   and /buffer/ are equal, otherwise the /key/ is considered to be less than the /buffer/.
 *
 *              If the SS_VAL_CMP_RE bit is set then the following bits are also supported (they actually each contain the
 *              SS_VAL_CMP_RE bit as well):
 *
 *              * SS_VAL_CMP_RE_EXTENDED: The /key/ value is treated as an extended regular expression rather than a basic
 *                                        regular expression. Refer to the documentation for the REG_EXTENDED flag of
 *                                        regcomp() for details.
 *
 *              * SS_VAL_CMP_RE_ICASE:    Ignore case when matching letters.
 *
 *              * SS_VAL_CMP_RE_NEWLINE:  Treat a newline in /key/ as dividing /buffer/ into multiple lines, so that a dollar
 *                                        sign can match before the newline and a carat can match after. Also, don't permit
 *                                        a dot to match a newline, and don't permit a complemented character class (square
 *                                        brackets with a leading carat) to match a newline. Otherwise, newline acts like any
 *                                        other ordinary character.
 *
 *              *Object*Link*Comparisons*
 *
 *              * SS_VAL_CMP_EQ:     When comparing two persistent object links use ss_pers_eq(). This is the default.
 *
 *              * SS_VAL_CMP_EQUAL:  When comparing two persistent object links use ss_pers_equal().
 *
 * Issue:       Since these bits must be passed as the bytes of a mask they must be only eight bits wide. The type, however,
 *              is defined as !unsigned because of argument promotion rules.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
typedef unsigned ss_val_cmp_t;
#define SS_VAL_CMP_DFLT                 0xFF
#define SS_VAL_CMP_SUBSTR               0xE0                    /* Strings: NUL terminated substring */
#define SS_VAL_CMP_SUBMEM               0xD0                    /* Strings: binary comparison of substring */
#define SS_VAL_CMP_RE                   0xC0                    /* Strings: NUL terminated regular expression */
#define   SS_VAL_CMP_RE_EXTENDED        (SS_VAL_CMP_RE|0x01)
#define   SS_VAL_CMP_RE_ICASE           (SS_VAL_CMP_RE|0x02)
#define   SS_VAL_CMP_RE_NEWLINE         (SS_VAL_CMP_RE|0x04)
#define SS_VAL_CMP_EQ                   SS_VALCMP_DFLT          /* Link: compare with ss_pers_eq() */
#define SS_VAL_CMP_EQUAL                0xE0                    /* Link: compare with ss_pers_equal() */

/* This is a checksum.  WARNING: THERE IS A CORRESPONDING MPI DATATYPE DEFINED IN ss_table_init() THAT MUST ALSO BE
 * ADJUSTED!!!!! */
typedef struct ss_val_cksum_t {
    unsigned long       adler;          /* The Adler32 checksum */
    unsigned char       flags;          /* Various SS_VAL_CKSUM_* flags */
} ss_val_cksum_t;

#define SS_VAL_CKSUM_NEW        0x01    /* The object had references to other objects that are "new", i.e., not having a
                                         * permanent home in a table. */                              

/* Functions for checksumming, comparing, and copying persistent objects */
typedef herr_t (*ss_val_resfunc_t)(void *obj, size_t size, size_t nelmts);
typedef herr_t (*ss_val_sumfunc_t)(void *obj, size_t size, size_t nelmts, ss_val_cksum_t *cksum);
typedef int (*ss_val_cmpfunc_t)(void *obj1, void *obj2, unsigned flags, size_t size, size_t nelmts);
typedef herr_t (*ss_val_cpyfunc_t)(void *src, void *dst, size_t size, size_t nelmts);
typedef char *(*ss_val_encfunc_t)(void *buffer, char *serbuf, size_t *serbuf_nused, size_t *serbuf_nalloc,
                                  size_t size, size_t nelmts);
typedef size_t (*ss_val_decfunc_t)(void *buffer, const char *serbuf, size_t size, size_t nelmts);

/* Compiled type information for checksumming, comparing, and copying persistent types. See ss_val_compile(). */
typedef struct ss_val_info_t {
    size_t              start;          /* starting byte address of members on which to operate */
    size_t              size;           /* size in bytes of each element on which to operate */
    size_t              nelmts;         /* number of elements on which to operate */
    hbool_t             transmit;       /* if set then data is allowed to be transmitted between tasks */
    ss_val_resfunc_t    res_func;       /* reset function */
    ss_val_sumfunc_t    sum_func;       /* checksum function */
    ss_val_cmpfunc_t    cmp_func;       /* comparison function */
    ss_val_cpyfunc_t    cpy_func;       /* copying function */
    ss_val_encfunc_t    enc_func;       /* serial encoding function */
    ss_val_decfunc_t    dec_func;       /* serial decoding function */
} ss_val_info_t;

#ifdef __cplusplus
extern "C" {
#endif

herr_t ss_val_init(void);
void *ss_val_search(void *buffer, void *key, const void *mask, size_t size, size_t nitems, size_t nvalinfo,
                    const ss_val_info_t *valinfo);
int ss_val_cmp(void *obj1, void *obj2, const void *mask, size_t nvalinfo, const ss_val_info_t *valinfo);
herr_t ss_val_copy(void *src, void *dst, size_t nvalinfo, const ss_val_info_t *valinfo);
herr_t ss_val_reset(void *buffer, size_t nvalinfo, const ss_val_info_t *valinfo);
herr_t ss_val_cksum(void *buffer, size_t nvalinfo, const ss_val_info_t *valinfo, ss_val_cksum_t *init, ss_val_cksum_t *cksum);
htri_t ss_val_cksum_isset(const ss_val_cksum_t *cksum);
htri_t ss_val_cksum_reset(ss_val_cksum_t *cksum/*out*/);
ss_val_info_t *ss_val_compile(hid_t type, size_t nvalinfo, ss_val_info_t *valinfo, size_t *nused);
int ss_val_cksum_cmp(const ss_val_cksum_t *sum1, const ss_val_cksum_t *sum2);
char *ss_val_ser_encode(void *obj_array, void **obj_vector, size_t nelmts, size_t *bufsize, size_t nvalinfo,
                        const ss_val_info_t *valinfo);
herr_t ss_val_ser_decode(void *obj_array, void **obj_vector, size_t nelmts, const char *serbuf, size_t nvalinfo,
                         const ss_val_info_t *valinfo);
herr_t ss_val_ser_type(size_t nvalinfo, const ss_val_info_t *valinfo, MPI_Datatype *type);
herr_t ss_val_dump(void *val, hid_t type, void *pers, FILE *out, const char *html_tag);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSVAL_H*/
