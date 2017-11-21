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
SS_IF(string);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 *
 * Description: Variable length strings as stored in persistent objects are manipulated through the SSlib !string interface
 *              and use a datatype ss_string_t which is opaque to the client.  This allows the implementation of persistent
 *              object strings to be changed as necessary to keep pace with functionality and performance improvements in the
 *              HDF5 string datatype.
 *
 *              As it turns out, HDF5 is unable to output variable length strings in parallel (1.7.3 2003-09-12). Therefore it
 *              has become necessary to change the implementation in SSlib already: all character strings for all objects of a
 *              particular scope will be stored in an extendible "Strings" dataset of type H5T_NATIVE_UCHAR in the same scope.
 *              Any object that contains a variable length string will contain an index into the "Strings" dataset, and when the
 *              object is in memory it will also contain a pointer directly to the string value.  We employ an opaque HDF5
 *              datatype to represent the string in memory and register a conversion function to allocate/find the string in
 *              the "strings" dataset during I/O.  The only problem with this approach is that HDF5-level tools don't
 *              understand that the offset is an index into the Strings dataset for a character string.
 *
 *              When a new task is opened all strings will initially have the same contents for the variable length string
 *              buffer, which is read by ss_string_boot().  As execution progresses different tasks will add different strings
 *              to the buffer in different orders and the tasks will become out of sync. When objects of a scope are
 *              synchronized we will be guaranteed that all tasks contain a valid Strings buffer, although the order of the
 *              new values in the buffer may differ between tasks. The ss_string_flush() function is responsible for choosing
 *              one of the scope tasks to write the string data back to the file.
 *
 *              SSlib variable length strings support uses length rather than NUL characters to mark the end of a string and
 *              are therefore capable of storing strings of bytes that might have embedded NUL characters.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

hid_t ss_string_tm = -1;                /* HDF5 datatype for persistent strings in memory */
hid_t ss_string_tf = -1;                /* HDF5 datatype for persistent strings in a file */
static ss_string_table_t *ss_string_table_g; /* to pass the string table around the qsort() call to the comparison callback */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Interface initializer
 *
 * Description: Although this function is called implicitly by the other functions of the string interface, it can also be
 *              called explicitly at any time.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  3, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_init(void)
{
    SS_ENTER_INIT;

    /* A string in a file is simply an index into the `String' dataset in the same scope. However, we make it an opaque type
     * because that allows tools to detect its special nature and display a string instead of a number. */
    if ((ss_string_tf = H5Tcreate(H5T_OPAQUE, 4))<0) SS_ERROR(INIT);
    if (H5Tset_tag(ss_string_tf, "SSlib::string_tf")<0) SS_ERROR(INIT);
    if (H5Tlock(ss_string_tf)<0) SS_ERROR(INIT);

    /* A string in memory (see ss_string_t defined in ssstring.h) */
    if ((ss_string_tm = H5Tcreate(H5T_OPAQUE, sizeof(ss_string_t)))<0) SS_ERROR(INIT);
    if (H5Tset_tag(ss_string_tm, "SSlib::string_tm")<0) SS_ERROR(INIT);
    if (H5Tlock(ss_string_tm)<0) SS_ERROR(INIT);

    /* Conversion functions */
    if (H5Tregister(H5T_PERS_HARD, "ss_string_t(mf)", ss_string_tm, ss_string_tf, ss_string_convert_mf)<0) SS_ERROR(INIT);
    if (H5Tregister(H5T_PERS_HARD, "ss_string_t(fm)", ss_string_tf, ss_string_tm, ss_string_convert_fm)<0) SS_ERROR(INIT);

 SS_CLEANUP:
    ss_string_tf = ss_string_tm = -1;
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Get a C string from a persistent string
 *
 * Description: Given information about a persistent string, return a pointer to a C string, i.e., an array of NUL-terminated
 *              characters.  If the caller supplies a buffer then the string will be copied into that buffer and NUL
 *              terminated, otherwise this function mallocs a new buffer to hold the result.
 *
 * Return:      On success, returns BUF if non-null or else allocates a result buffer.  On failure returns the null pointer.
 *              It is a failure to supply a BUFSIZE which is not large enough to hold the entire string value with its NUL
 *              terminator. The caller is responsible for freeing any return value allocated by this function.
 *
 * Issue:       SSlib stores strings with a byte count, so if the string was stored without a terminating NUL character then
 *              it will also be returned as such.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  3, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
ss_string_get(const ss_string_t *str,
              size_t bufsize,           /* Size of BUF (only referenced if BUF is non-null). */
              char *buf                 /* Optional buffer in which to store the C string. This buffer is assumed to be an
                                         * array of at least BUFSIZE characters. */
              )
{
    SS_ENTER(ss_string_get, charP);
    char        *tmp=NULL;

    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));
    if (buf) {
        if (bufsize<str->nbytes) SS_ERROR_FMT(OVERFLOW, ("supplied buffer is too short"));
    } else {
        tmp = buf = malloc(MAX(1,str->nbytes));
        if (!tmp) SS_ERROR(RESOURCE);
        if (0==str->nbytes) tmp[0] = '\0';
    }
    memcpy(buf, str->p, str->nbytes);

 SS_CLEANUP:
    SS_FREE(tmp);
    SS_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Obtain pointer into string object
 *
 * Description: This function is similar to ss_string_get() except rather than copying the string to some other memory it
 *              returns a pointer directly into the ss_string_t object.  The caller should expect that the pointer is valid
 *              only until some other operation on that object.
 *
 * Return:      On success, returns a temporary pointer directly into the STR object. If the value is zero bytes long then a
 *              pointer to a NUL character is returned instead of null.
 *
 * Issue:       The returned value is `const' because if it points into the Strings array then it might be the case that
 *              multiple strings currently having the same value are sharing the same storage.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, July 30, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
const char *
ss_string_ptr(const ss_string_t *str)
{
    SS_ENTER(ss_string_ptr, const_charP);
    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));
 SS_CLEANUP:
    SS_LEAVE(str->p?str->p:"");
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Store a C string in a persistent string
 *
 * Description: Given a C-style NUL-terminated character string stored in S, make the persistent string object STR have that
 *              same value.  The ss_string_memset() function can be used to store arbitrary data that might have embedded NUL
 *              characters or that might not be NUL-terminated.
 *
 * Return:      Returns non-negative on success and negative on failure. The success side effect is that STR has the same
 *              value as S.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  3, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_set(ss_string_t *str,         /* The destination persistent string. */
              const char *s             /* The source C string to copy. */
              )
{
    SS_ENTER(ss_string_set, herr_t);
    if (ss_string_splice(str, s, 0, s?strlen(s)+1:0, str->nbytes)<0) SS_ERROR(FAILED);
        
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Store a byte array in a string
 *
 * Description: This function is similar to ss_string_set() except the number of bytes in VALUE is explicitly passed instead
 *              of looking for the first NUL character.
 *
 * Return:      Returns non-negative on success and negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, January 30, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_memset(ss_string_t *str,      /* The destination variable length string. */
                 const void *value,     /* The optional value to assign to STR. If NULL then a value of all zero bytes is
                                         * used. */                                  
                 size_t nbytes          /* The number of bytes in VALUE. */
                 )
{
    SS_ENTER(ss_string_memset, herr_t);
    void *value_here=NULL;

    if (NULL==value && NULL==(value=value_here=calloc(1, nbytes))) SS_ERROR(RESOURCE);
    if (ss_string_splice(str, value, 0, nbytes, str->nbytes)<0) SS_ERROR(FAILED);
    value_here = SS_FREE(value_here);

SS_CLEANUP:
    SS_FREE(value_here);
    SS_LEAVE(0);
}
                 

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Free memory associated with the string
 *
 * Description: Frees the character array value stored in STR but does not free STR itself.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, August  5, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_reset(ss_string_t *str)
{
    SS_ENTER(ss_string_reset, herr_t);
    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));

    if (0==str->offset)
       { SS_FREE(str->p);}
    memset(str, 0, sizeof(*str));

 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Weakly reset a string
 *
 * Description: Sometimes we want a variable length string to keep the same value it had but to be reallocated in the string
 *              backing store. For instance, when a variable length string is copied from one scope to another we want to to
 *              keep the same value but it cannot continue to have the same dataset offset since the new scope has a
 *              completely different dataset for string storage.
 *
 * Return:      Returns non-negative on success; negative on failure
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, January 27, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_realloc(ss_string_t *str)
{
    SS_ENTER(ss_string_realloc, herr_t);
    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));

    if (str->p) {
        char *tmp;
        if (NULL==(tmp=malloc(str->nbytes))) SS_ERROR(FAILED);
        memcpy(tmp, str->p, str->nbytes);
        str->p = tmp;
        str->offset = 0;
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Compares two variable length strings
 *
 * Description: This function is similar to memcmp() but its arguments are variable length strings instead.
 *
 * Return:      Returns -1 if the value of S1 is less than S2, 1 if S1 is greater than S2, and zero if they are equal in
 *              value.  Returns -2 on failure (beware that this is a refinement of the more general negative returns on
 *              failure used throughout SSlib).  It is an error if S1 or S2 is a null pointer, but not if either or both have
 *              no associated value. Lack of a value is less than any other value and if S1 and S2 both lack a value they are
 *              considered equal.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, January 30, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_string_cmp(const ss_string_t *s1, const ss_string_t *s2)
{
    SS_ENTER(ss_string_cmp, int);
    int         retval=0;

    if (!s1 || !s2) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));
    if (!s1->p && !s2->p) {
        retval = 0;
    } else if (!s1->p) {
        retval = -1;
    } else if (!s2->p) {
        retval = 1;
    } else {
        retval = memcmp(s1->p, s2->p, MIN(s1->nbytes, s2->nbytes));
        if (0==retval) {
            if (s1->nbytes>s2->nbytes) retval = 1;
            if (s1->nbytes<s2->nbytes) retval = -1;
        }
    }

SS_CLEANUP:
    SS_RETVAL(-2);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Compare persistent string with C string
 *
 * Description: This function is similar to strcmp() but its first argument is a persistent string instead. It compares the
 *              value of the persistent string with the C string S.
 *
 * Return:      Returns -1 if the value of STR is less than S, 1 if STR is greater than S, and zero if they are
 *              equal in value.  Returns -2 on failure (beware that this is a refinement of the more general negative returns
 *              on failure used throughout SSlib).
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  3, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_string_cmp_s(const ss_string_t *str, const char *s)
{
    SS_ENTER(ss_string_cmp_s, int);
    size_t      slen = s ? strlen(s)+1 : 0;
    int         retval = 0;

    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));
    if (!str->p && !s) {
        retval = 0;
    } else if (!str->p) {
        retval = -1;
    } else if (!s) {
        retval = 1;
    } else {
        retval = memcmp(str->p, s, MIN(str->nbytes, slen));
        if (0==retval) {
            if (str->nbytes>slen) retval = 1;
            if (str->nbytes<slen) retval = -1;
        }
    }
 SS_CLEANUP:
    SS_RETVAL(-2);
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Append one string to another
 *
 * Description: Changes the value of the persistent string by appending another string.
 *
 * Return:      Returns non-negative on success; negative on failure.  Successful side effect is that the value of STR is
 *              modified by appending the string S, which should be a C NUL-terminated string.  If the original value of STR
 *              is NUL-terminated then the additional S value will replace that NUL, otherwise the additional value will be
 *              added after the existing value.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  3, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_cat(ss_string_t *str, const char *s)
{
    SS_ENTER(ss_string_cat, herr_t);

    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));
    if (s && str->nbytes>0 && '\0'!=str->p[str->nbytes-1]) {
        if (ss_string_splice(str, s, str->nbytes-1, strlen(s)+1, 1)<0) SS_ERROR(FAILED);
    } else if (s) {
        if (ss_string_splice(str, s, str->nbytes, strlen(s)+1, 0)<0) SS_ERROR(FAILED);
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Substring modification
 *
 * Description: This function is able to modify a string value by modifying, inserting, or deleting bytes.  It does not assume
 *              that VALUE is a C-style NUL-terminated string.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, February  9, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_splice(ss_string_t *str,              /* String object to be modified by this operation. */
                 const char *value,             /* Optional new data for part of the string value. If this argument is the
                                                 * null pointer and NBYTES is positive then the new data will be all NUL
                                                 * characters (this allows for an easy way to extend the length of a string). */
                 size_t start,                  /* Byte offset at which to place the new data in the string. */
                 size_t nbytes,                 /* Length of the new data in bytes. */
                 size_t nreplace                /* Number of bytes replaced by the new data. If SS_NOSIZE is passed then all
                                                 * bytes from START to the end of the original value will be replaced by the
                                                 * new value. */
                  )
{
    SS_ENTER(ss_string_splice, herr_t);
    size_t              newsize;                /* New size of the string after modification */
    char                *tmpbuf=NULL;           /* Temporary buffer for new value */

    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));
    if (start>str->nbytes) SS_ERROR_FMT(DOMAIN, ("new value starts after end of existing value"));
    if (SS_NOSIZE==nreplace || start+nreplace>str->nbytes) nreplace = str->nbytes - start;
    newsize = str->nbytes - nreplace + nbytes;

    if (nbytes<=nreplace && 0==str->offset) {
        /* String is not growing and str->p points to allocated memory and not into the string table. We can just splice
         * in the new value in place. */
        if (value) {
            memcpy(str->p+start, value, nbytes); /*the inserted value*/
        } else {
            memset(str->p+start, 0, nbytes); /*the inserted NUL characters*/
        }
        memmove(str->p+start+nbytes, str->p+start+nreplace, str->nbytes-(start+nreplace)); /*move the rhs down*/
        str->nbytes = newsize;
    } else {
        /* Value is growing or already assigned to a string table offset */
        if (NULL==(tmpbuf=malloc(newsize))) SS_ERROR(RESOURCE);
        memcpy(tmpbuf, str->p, start); /*the lhs stays put*/
        if (value) {
            memcpy(tmpbuf+start, value, nbytes); /*the inserted value*/
        } else {
            memset(tmpbuf+start, 0, nbytes); /*the inserted NUL characters*/
        }
        memcpy(tmpbuf+start+nbytes, str->p+start+nreplace, str->nbytes-(start+nreplace)); /*move the rhs down*/

        if (nbytes!=nreplace || memcmp(str->p, tmpbuf, str->nbytes)) {
            /* Value changed */
            if (str->offset) {
                str->offset = 0;
            } else {
                ss_string_reset(str);
                /* SS_FREE(str->p); */
            }
            str->p = tmpbuf;
            tmpbuf = NULL;
            str->nbytes = newsize;
        }
    }

    tmpbuf = SS_FREE(tmpbuf);

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Query the length of a persistent string
 *
 * Description: This function returns the number of initial non-NUL characters in a string's value. If STR's value is a
 *              typicall NUL-terminated C-style string then this function's return value is identical to strlen().  However,
 *              if STR's value has no NUL characters then this function's return value is identical to ss_string_memlen().
 *
 * Return:      Returns the number of initial non-NUL characters in a string on success; returns SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, January 30, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_string_len(const ss_string_t *str)
{
    SS_ENTER(ss_string_len, size_t);
    size_t      i=0;
    
    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));
    for (i=0; i<str->nbytes && str->p[i]; i++) /*void*/;
SS_CLEANUP:
    SS_LEAVE(i);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Strings
 * Purpose:     Query the length of a persistent string
 *
 * Description: Given a persistent string, return the number of characters contained in that string, including the terminating NUL
 *              character if any.  Note that for NUL terminated strings this is one more than what strlen() would have
 *              returned, but this behavior is necessary since SSlib byte-counts all string data in order to allow strings
 *              that have embedded NUL characters and that might lack a NUL terminator. In other words, SSlib strings can
 *              store any type of data.
 *
 * Return:      On success, returns the actual number of bytes stored for the string including a NUL terminator if any.
 *              Returns SS_NOSIZE on error.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, June  3, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_string_memlen(const ss_string_t *str)
{
    SS_ENTER(ss_string_memlen, size_t);
    if (!str) SS_ERROR_FMT(USAGE, ("no ss_string_t supplied"));
SS_CLEANUP:
    SS_LEAVE(str->nbytes);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Sorting callback
 *
 * Description: This is a callback function for qsort() to sort the ss_string_table_t object stored in ss_string_table_g.
 *
 * Return:      Same as strcmp()
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, September 17, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static int
ss_string_sort_cb(const void *_a, const void *_b)
{
    /* SS_ENTER() -- not called for efficiency reasons. This function doesn't raise errors anyway. */
    size_t      a = *(const size_t*)_a;
    size_t      b = *(const size_t*)_b;
    size_t      a_len, b_len;
    int         cmp;

    assert(ss_string_table_g);
    assert(a>=4);
    assert(a<ss_string_table_g->buf_nused);
    assert(b>=4);
    assert(b<ss_string_table_g->buf_nused);

    a_len = ss_string_table_g->buf[a-4] +
            ss_string_table_g->buf[a-3] * 256 +
            ss_string_table_g->buf[a-2] * 256 * 256 +
            ss_string_table_g->buf[a-1] * 256 * 256 * 256;

    b_len = ss_string_table_g->buf[b-4] +
            ss_string_table_g->buf[b-3] * 256 +
            ss_string_table_g->buf[b-2] * 256 * 256 +
            ss_string_table_g->buf[b-1] * 256 * 256 * 256;

    cmp = memcmp(ss_string_table_g->buf+a, ss_string_table_g->buf+b, MIN(a_len,b_len));
    if (!cmp) {
        if (a_len>b_len) cmp = 1;
        if (a_len<b_len) cmp = -1;
    }

    return cmp;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Read variable length strings from file
 *
 * Description: Every scope has a variable length strings data structure (ss_string_table_t) that holds all the variable
 *              length strings for all objects defined in the scope.  Such strings are stored in a one-dimensional extendible
 *              dataset of type H5T_NATIVE_UCHAR, each value preceded by a four-byte little-endian length in bytes. The stored
 *              length does not include the four-byte length itself.
 *
 *              This function opens and reads that dataset. Although this could normally be delayed until the first string is
 *              needed by some object in the scope, we call this when a scope is initialized because (1) opening the dataset
 *              is file-collective just like opening the scope's group, and (2) we can broadcast the strings to the other
 *              scope tasks faster than each of them reading the dataset.
 *
 * Return:      Returns a non-null string table pointer on success; null on failure.
 *
 * Parallel:    Conceptually this is a scope-collective operation, but because H5Dopen() and H5Dcreate() are file-collective
 *              this function must be file collective.  The "extra" tasks also get a valid return value, but the data is not
 *              supplied to those functions (strtab->buf==NULL) and the only thing those tasks should do is participate in the
 *              call to ss_string_flush().
 *
 * Programmer:  Robb Matzke
 *              Monday, September 15, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_string_table_t *
ss_string_boot(hid_t scopegrp,                          /* The HDF5 group where the scope lives. */
               MPI_Comm scopecomm,                      /* Communicator for the scope (unused when compiled without MPI support) */
               hbool_t create,                          /* We are creating a new file if this is true. */
               ss_string_table_t *exists                /* Optional string table that has already been booted. If we are
                                                         * rebooting a table then we don't need to re-read the data. */
               )
{
    SS_ENTER(ss_string_boot, ss_string_table_tP);
    hsize_t             max_size = H5S_UNLIMITED;       /* Dataset is potentially unlimited size */
    hsize_t             chunk_size = 1024;              /* Bytes per storage chunk */
    hid_t               space=-1, dcpl=-1, dxpl=-1;     /* Various HDF5 objects */
    int                 scopetask=MPI_UNDEFINED;        /* Rank of this task in the scope communicator */
    ss_string_table_t   *strtab=NULL;                   /* The return value */
    size_t              i, nstrings, strno;

    if (NULL==(strtab=exists) && NULL==(strtab=SS_OBJ_NEW(ss_string_table_t))) SS_ERROR(RESOURCE);
    if (scopegrp<=0) SS_ERROR_FMT(USAGE, ("bad scope group"));
    if ((scopetask=ss_mpi_comm_rank(scopecomm))<0) SS_ERROR(FAILED); /*MPI_UNDEFINED if caller is not in SCOPECOMM */

    /* Create and/or read the Strings database */
    if (1==scopegrp) {
        /* Transient scope -- nothing to do. */
    } else if (create) {
        if ((space = H5Screate_simple(1, &(strtab->dset_size), &max_size))<0) SS_ERROR(HDF5);
        if ((dcpl = H5Pcreate(H5P_DATASET_CREATE))<0) SS_ERROR(HDF5);
        if (H5Pset_chunk(dcpl, 1, &chunk_size)<0) SS_ERROR(HDF5);
        if ((strtab->dset=H5Dcreate(scopegrp, "Strings", H5T_NATIVE_UCHAR, space, dcpl))<0) SS_ERROR(HDF5);
        if (H5Pclose(dcpl)<0) SS_ERROR(HDF5);
        dcpl = -1;
        if (H5Sclose(space)<0) SS_ERROR(HDF5);
        space = -1;

        /* Initialize memory to 0x01, 0x00, 0x00, 0x00, 0x00 so that an offset of 4 points to a value consisting of a single
         * NUL byte because C-style empty strings are common. */    
        if (NULL==(strtab->buf=calloc(1, (size_t)chunk_size))) SS_ERROR(RESOURCE);
        strtab->buf[0] = 1;
        strtab->buf_nused = 5;
        strtab->buf_nalloc = chunk_size;
    } else {
        /* In ss_string_flush() only task zero of the scope ever calls H5Dwrite() for the Strings dataset and is therefore the
         * only one that has converted temporary strings (strings with a zero `offset' and non-null `p') into permanent
         * strings by adding them to the dataset. Therefore task zero will have a strtab->buf_nused value that is at least as
         * large (probably larger) than any other task. When re-reading the strings table it should be safe to just reread the
         * table on all the tasks. */
        if ((strtab->dset=H5Dopen(scopegrp, "Strings"))<0) SS_ERROR(HDF5);
        if ((space = H5Dget_space(strtab->dset))<0) SS_ERROR(HDF5);
        if (1!=H5Sget_simple_extent_ndims(space)) SS_ERROR_FMT(HDF5, ("Strings dataset is not one dimensional"));
        if (H5Sget_simple_extent_dims(space, &(strtab->dset_size), NULL)<0) SS_ERROR(HDF5);
        SS_ASSERT(!exists ||
                  (scopetask==0 && strtab->buf_nused==strtab->dset_size) ||
                  (scopetask!=0 && strtab->buf_nused<=strtab->dset_size));
        strtab->buf_nused = strtab->dset_size;
        if (strtab->buf_nused>0) {
            if (NULL==(strtab->buf=realloc(strtab->buf, (size_t)(strtab->buf_nused)))) SS_ERROR(RESOURCE);
            strtab->buf_nalloc = strtab->buf_nused;
            if ((dxpl=H5Pcreate(H5P_DATASET_XFER))<0) SS_ERROR(HDF5);
#ifdef HAVE_PARALLEL
            if (H5Pset_dxpl_mpio(dxpl, H5FD_MPIO_INDEPENDENT)<0) SS_ERROR(HDF5);
#endif
            if (0==scopetask && H5Dread(strtab->dset, H5T_NATIVE_UCHAR, space, space, dxpl, strtab->buf)<0) SS_ERROR(HDF5);
            if (H5Pclose(dxpl)<0) SS_ERROR(HDF5);
            dxpl = -1;
            if (MPI_UNDEFINED!=scopetask && ss_mpi_bcast(strtab->buf, strtab->buf_nused, MPI_CHAR, 0, scopecomm)<0)
                SS_ERROR(MPI);
        }
    }

    /* Build a sorted index of string values */
    if (scopetask!=MPI_UNDEFINED && (!exists || create)) {
        /* Count how many strings are present by starting at the beginning of the buffer and reading each of the 4-byte length
         * values until we get to the end of the buffer. */
        nstrings = i = 0;
        while (i+4<=strtab->buf_nused) {
            size_t nbytes = strtab->buf[i] +
                            strtab->buf[i+1] * 256 +
                            strtab->buf[i+2] * 256 * 256 +
                            strtab->buf[i+3] * 256 * 256 * 256;
            SS_ASSERT(i+4+nbytes > i);
            i += 4 + nbytes;
            nstrings++;
        }
        if (i!=strtab->buf_nused) SS_ERROR_FMT(CORRUPT, ("failed to parse Strings value"));

        /* Allocate (or reallocate) arrays */
        SS_EXTEND(strtab->index, MAX(64,nstrings), strtab->index_nalloc);
        strtab->index_nused = nstrings;
        
        /* Initialize index offsets by traversing the dataset value again and this time recording each offset. The offsets are
         * to the start of the value, which is four bytes past the start of the encoded length. */
        strno = i = 0;
        while (i+4<=strtab->buf_nused) {
            size_t nbytes = strtab->buf[i] +
                            strtab->buf[i+1] * 256 +
                            strtab->buf[i+2] * 256 * 256 +
                            strtab->buf[i+3] * 256 * 256 * 256;
            strtab->index[strno++] = i+4;
            SS_ASSERT(i+4+nbytes > i);
            i += 4 + nbytes;
        }
        SS_ASSERT(i==strtab->buf_nused);
        SS_ASSERT(strno==nstrings);

        /* Sort the index. The comparison function needs to know the dataset buffer's base address in order to look up the
         * strings to compare. We supply that through the ss_string_table_g variable which we must clear before returning. */
        SS_ASSERT(NULL==ss_string_table_g);
        ss_string_table_g = strtab;
        qsort(strtab->index, nstrings, sizeof(strtab->index[0]), ss_string_sort_cb);
        ss_string_table_g = NULL;

        /* Make sure there are no duplicate string values. A duplicate value implies that something went wrong elsewhere. */
        for (strno=1; strno<nstrings; strno++) {
            ss_string_t a, b;

            memset(&a, 0, sizeof a);
            a.offset = strtab->index[strno-1];
            a.p = (char*)(strtab->buf) + a.offset;
            a.nbytes = strtab->buf[a.offset-4] +
                       strtab->buf[a.offset-3] * 256 +
                       strtab->buf[a.offset-2] * 256 * 256 +
                       strtab->buf[a.offset-1] * 256 * 256 * 256;

            memset(&b, 0, sizeof b);
            b.offset = strtab->index[strno];
            b.p = (char*)(strtab->buf) + b.offset;
            b.nbytes = strtab->buf[b.offset-4] +
                       strtab->buf[b.offset-3] * 256 +
                       strtab->buf[b.offset-2] * 256 * 256 +
                       strtab->buf[b.offset-1] * 256 * 256 * 256;
            
            if (0==ss_string_cmp(&a, &b)) {
                SS_ERROR_FMT(CORRUPT, ("Strings table has same string at offset %lu and %lu",
                                       (unsigned long)(strtab->index[strno-1]), (unsigned long)(strtab->index[strno])));
            }
        }
    }
        
 SS_CLEANUP:
    if (dcpl>0) H5Pclose(dcpl);
    if (space>0) H5Sclose(space);
    if (strtab && !exists) {
        if (strtab->dset>0) H5Dclose(strtab->dset);
        SS_OBJ_DEST(strtab);
        SS_FREE(strtab);
    }
    
    SS_LEAVE(strtab);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Destroy a string table
 *
 * Description: When a scope is destroyed we must also destroy the variable length strings table, and this function does just
 *              that.  It does not even attempt to flush data to the strings dataset.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. In order to be independent this function does not close the dataset associated with the strings
 *              table.
 *
 * Programmer:  Robb Matzke
 *              Monday, September 15, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_desttab(ss_string_table_t *strtab     /* Strings table to be destroyed. This memory is also freed since it must have
                                                 * been allocated by ss_string_boot(). */
                  )
{
    SS_ENTER(ss_string_desttab, herr_t);
    if (strtab) {
        SS_ASSERT_TYPE(strtab, ss_string_table_t);
        strtab->buf = SS_FREE(strtab->buf);
        strtab->index = SS_FREE(strtab->index);
        SS_OBJ_DEST(strtab);
        strtab = SS_FREE(strtab);
    }
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Flush variable length strings
 *
 * Description: Writes all variable length strings to the HDF5 file.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Conceptually scope-collective, but we need to be file-collective because of a possible call to H5Dextend().
 *              Task zero of the scope communicator is the only one that will write the dataset. In fact, the other tasks
 *              might not even have identical data to write because their strings might have been stored in a different order.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_flush(ss_string_table_t *strtab,      /* The string table to be written (the "extra" tasks must also pass this) */
                MPI_Comm scopecomm,             /* The communicator for the scope that owns this string table */
                MPI_Comm filecomm               /* The communicator for the file that owns the scope. As mentioned above, this
                                                 * argument is temporary because of restrictions in the HDF5 H5Dextend() call. */
                )
{
    SS_ENTER(ss_string_flush, herr_t);
    unsigned long       size_ul;                /* Dataset size as unsigned long for MPI message */
    hsize_t             hsize;                  /* Dataset size as hsize_t for H5 operations */
    int                 scopetask=0;            /* Rank of calling task in scope communicator or MPI_UNDEFINED */
    int                 scopezero;              /* Rank in filecomm of scope task zero */
    hid_t               space=-1, dxpl=-1;

    SS_ASSERT(ss_mpi_subcomm(scopecomm, filecomm)>0);
    SS_ASSERT_TYPE(strtab, ss_string_table_t);

    if (strtab->dset>0) {
        if ((scopetask=ss_mpi_comm_rank(scopecomm))<0) SS_ERROR(FAILED);
        if ((scopezero=ss_mpi_maptask(0, scopecomm, filecomm))<0) SS_ERROR(FAILED);

        /* Extend the dataset if necessary. Task zero of the scope communicator is the only one that knows the size since it's the
         * only one that has the correct data, due to it being the only one to have called ss_string_convert_mf() as it was
         * writing out the other persistent tables. */
        size_ul = strtab->buf_nused;
        if (ss_mpi_bcast(&size_ul, 1, MPI_UNSIGNED_LONG, scopezero, filecomm)<0) SS_ERROR(FAILED);
        hsize = size_ul;
        if (hsize>strtab->dset_size) {
            if (H5Dextend(strtab->dset, &hsize)<0) SS_ERROR(HDF5);
            strtab->dset_size = hsize;
        }
    
        /* Scope task zero writes the data independently. It only writes the data from the low water mark (inclusive) to the
         * high water mark (exclusive). */   
        if (0==scopetask && hsize>0 && strtab->hi_w>strtab->lo_w) {
            hsize_t lo_w = strtab->lo_w; /*type conversion*/
            hsize_t hi_w = strtab->hi_w; /*type conversion*/
            if ((dxpl = H5Pcreate(H5P_DATASET_XFER))<0) SS_ERROR(HDF5);
#ifdef HAVE_PARALLEL
            if (H5Pset_dxpl_mpio(dxpl, H5FD_MPIO_INDEPENDENT)<0) SS_ERROR(HDF5);
#endif
            if ((space=H5Screate_simple(1, &hsize, NULL))<0) SS_ERROR(HDF5);
            if (H5Sselect_slab(space, H5S_SELECT_SET, (hsize_t)0, &lo_w, &hi_w)<0) SS_ERROR(HDF5);
            if (H5Dwrite(strtab->dset, H5T_NATIVE_UCHAR, space, space, dxpl, strtab->buf)<0) SS_ERROR(HDF5);
            if (H5Sclose(space)<0) SS_ERROR(HDF5);
            space = -1;
            if (H5Pclose(dxpl)<0) SS_ERROR(HDF5);
            dxpl = -1;
        }
    }

    strtab->lo_w = strtab->hi_w = 0; /*consider table to be clean*/

 SS_CLEANUP:
    if (space>0) H5Sclose(space);
    if (dxpl>0) H5Pclose(dxpl);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Convert string to file type
 *
 * Description: This function converts a single variable length string from the memory representation SRC to the file
 *              representation DST. The SRC and DST buffers must not overlap.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 10, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_convert_mf2(const ss_string_t *src,           /* Memory representation of the string. */
                      void *dst,                        /* OUT: File representation of the string. */
                      ss_string_table_t *strtab         /* Optional string table. The purpose of this argument is to avoid an
                                                         * unnecessary lookup of the string table when the caller already has
                                                         * that information. */
                      )
{
    SS_ENTER(ss_string_convert_mf2, herr_t);
    size_t              lt, md, rt;             /* left, mid, and right indices for binary search */
    int                 cmp;                    /* comparison result during binary search */
    size_t              md_nbytes;              /* number of bytes in `md' string value */
    unsigned char       *md_p;                  /* pointer to `md' string value */
    size_t              dst_offset;             /* offset to eventually store in the destination */
    size_t              i, nbytes;

    if (!strtab) strtab = SS_SCOPE(&ss_table_scope_g)->m.strings;
    SS_ASSERT(strtab);
        
    if (0==src->offset && src->p) {
        /* String has a value but not saved in the dataset. First we look at the index to see if the value already appears in
         * the dataset's buffer. This is a binary search and on exit the `md' index points to either the value that was found
         * or the position in the `index' array in which to insert the new value in order to keep the array sorted. */
        lt = md = 0;
        rt = strtab->index_nused;
        cmp = -1; /* in case we never make it through the loop */
        while (lt<rt) {
            md = (lt+rt)/2;
            md_p = strtab->buf + strtab->index[md];
            SS_ASSERT(strtab->index[md]>=4);
            SS_ASSERT(strtab->index[md]<=strtab->buf_nused);
            md_nbytes = strtab->buf[strtab->index[md]-4] +
                        strtab->buf[strtab->index[md]-3] * 256 +
                        strtab->buf[strtab->index[md]-2] * 256 * 256 +
                        strtab->buf[strtab->index[md]-1] * 256 * 256 * 256;
            SS_ASSERT(strtab->index[md]+md_nbytes<=strtab->buf_nused);
            cmp = memcmp(src->p, md_p, MIN(src->nbytes,md_nbytes));
            if (0==cmp) {
                if (src->nbytes>md_nbytes) cmp=1;
                else if (src->nbytes<md_nbytes) cmp=-1;
            }
            if (cmp<0) {
                rt = md;
            } else if (cmp>0) {
                lt = md+1;
            } else {
                break;
            }
        }
        if (cmp>0) md++;
        if (0==cmp) {
            dst_offset = strtab->index[md];
        } else {
            /* Increase the size of the index array */
            SS_EXTEND(strtab->index, MAX(64,strtab->index_nused+1), strtab->index_nalloc);

            /* Increase the size of the dataset buffer */
            SS_EXTEND_ZERO(strtab->buf, MAX(1024,strtab->buf_nused+4+src->nbytes), strtab->buf_nalloc);

            /* Insert the dataset offset into the index array at the `md' element of that array. */
            memmove(strtab->index+md+1, strtab->index+md, (strtab->index_nused-md)*sizeof(strtab->index[0]));
            strtab->index_nused++;
            strtab->index[md] = strtab->buf_nused + 4; /*index is to value, which is just after 4-byte size*/

            /* Insert the string value into the dataset buffer. */
            strtab->buf[strtab->buf_nused+0] = (src->nbytes >>  0) & 0xff;
            strtab->buf[strtab->buf_nused+1] = (src->nbytes >>  8) & 0xff;
            strtab->buf[strtab->buf_nused+2] = (src->nbytes >> 16) & 0xff;
            strtab->buf[strtab->buf_nused+3] = (src->nbytes >> 24) & 0xff;
            memcpy(strtab->buf+strtab->buf_nused+4, src->p, src->nbytes);
            strtab->lo_w = MIN(strtab->lo_w, strtab->buf_nused);
            strtab->buf_nused += 4 + src->nbytes;
            strtab->hi_w = MAX(strtab->hi_w, strtab->buf_nused);

            SS_ASSERT(ss_scope_iswritable(&ss_table_scope_g)>0);

            /* Final destination value */
            dst_offset = strtab->index[md];
#if 0 /*Permanently disabled because it seems to be working just fine. [rpm 2004-09-22]*/
#ifndef NDEBUG
            /* Make sure the index array is still sorted. This should catch any logic errors above, but we only do it for
             * debugging because it ruins the O(log N) time complexity. */
            for (i=0; i<strtab->index_nused; i++) {
                SS_ASSERT(strtab->index[i]>=4); /*first value is preceded by a four byte size */
                SS_ASSERT(strtab->index[i]<strtab->buf_nused); /* index is within array size */
                if (i>0) {
                    size_t a_len = strtab->buf[strtab->index[i-1]-4] +
                                   strtab->buf[strtab->index[i-1]-3] * 256 +
                                   strtab->buf[strtab->index[i-1]-2] * 256 * 256 +
                                   strtab->buf[strtab->index[i-1]-1] * 256 * 256 * 256;
                    size_t b_len = strtab->buf[strtab->index[i-0]-4] +
                                   strtab->buf[strtab->index[i-0]-3] * 256 +
                                   strtab->buf[strtab->index[i-0]-2] * 256 * 256 +
                                   strtab->buf[strtab->index[i-0]-1] * 256 * 256 * 256;
                    cmp = memcmp(strtab->buf+strtab->index[i-1], strtab->buf+strtab->index[i-0], MIN(a_len,b_len));
                    if (0==cmp) {
                        if (a_len>b_len) cmp = 1;
                        if (a_len<b_len) cmp = -1;
                    }
                    SS_ASSERT(cmp<0);
                }
            }
#endif
#endif
        }
    } else {
        dst_offset = src->offset;
    }

    /* Now encode the dst_offset into the destination in little-endian order */
    if (0==(nbytes=H5Tget_size(ss_string_tf))) SS_ERROR(HDF5);
    for (i=0; i<nbytes; i++) {
        ((unsigned char*)dst)[i] = (unsigned char)(dst_offset & 0xff);
        dst_offset = (dst_offset & ~(size_t)0xff) >> 8;
    }
    SS_ASSERT(0==dst_offset);

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Convert string to file type
 *
 * Description: This is an H5Tconvert() callback to convert ss_string_t objects in memory into dataset offsets into a strings
 *              table to be stored in the scope that owns the strings.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Issue:       When converting ss_string_t data from memory representation to file representation we allocate space in the
 *              scope's "Strings" dataset by appending the string value to that dataset. However, we never adjust the memory
 *              datatype to show that the string was added to the dataset and there's no way to do that down here because
 *              BUF is only a copy of the original data by then.  So if we just naively appended the string to the end of the
 *              dataset we could end up with /lots/ of duplicate values.  We prevent that by also keeping a sorted index of
 *              all stored values so when adding a new string we can point it to an already existing value.
 *
 * Programmer:  Robb Matzke
 *              Monday, September 15, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_string_convert_mf(hid_t UNUSED src_type,     /* Source data type (should be ss_string_tm) */
                     hid_t dst_type,            /* Destination data type (should be ss_string_tf) */
                     H5T_cdata_t *cdata,        /* Data type conversion control data */
                     size_t nelmts,             /* Number of elements to convert */
                     size_t buf_stride,         /* Distance between elements in bytes */
                     size_t UNUSED bkg_stride,  /* Distance between elements in the background buffer. */
                     void *buf,                 /* The buffer which is to be converted in place. */
                     void UNUSED *bkg,          /* The background buffer. */
                     hid_t UNUSED dxpl          /* Dataset transfer property list. */
                     )
{
    SS_ENTER(ss_string_convert_mf, herr_t);
    char                *src = buf;             /* The source data pointer */
    char                *dst = buf;             /* The destination data pointer */
    size_t              dst_size;               /* Size of the string in the file */
    ss_string_t         tmp;                    /* Temporary for non-overlapping conversion */
    hsize_t             elmtno;                 /* Element number currently being converted */
    ss_string_table_t   *strtab=NULL;           /* String table for current scope */

    SS_ASSERT(cdata);

    switch (cdata->command) {
    case H5T_CONV_INIT:
        /* See ss_string_init() for definitions of these types */
        SS_ASSERT(H5Tequal(src_type, ss_string_tm));
        SS_ASSERT(H5Tequal(dst_type, ss_string_tf));
        SS_ASSERT(H5Tget_size(src_type)==sizeof(ss_string_t));
        SS_ASSERT(sizeof(ss_string_t)>=H5Tget_size(dst_type)); /*implies left-to-right conversion*/
        break;

    case H5T_CONV_FREE:
        break;

    case H5T_CONV_CONV:
        if (0==(dst_size=H5Tget_size(dst_type))) SS_ERROR(HDF5);
        strtab = SS_SCOPE(&ss_table_scope_g)->m.strings;
        SS_ASSERT(strtab);
        for (elmtno=0; elmtno<nelmts; elmtno++) {
            memcpy(&tmp, src, sizeof tmp);
            if (ss_string_convert_mf2(&tmp, dst, strtab)<0) SS_ERROR(FAILED);
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
 * Chapter:     Strings
 * Purpose:     Convert string to memory type
 *
 * Description: This function converts a single file-format variable length string SRC into its memory representation DST. The
 *              two buffers SRC and DST must not point to the same or overlapping memory.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 10, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_convert_fm2(const void *src,                  /* File representation of the string */
                      ss_string_t *dst,                 /* OUT: Memory representation of the string. */
                      ss_string_table_t *strtab         /* Optional string table. If not supplied then it will be looked up by
                                                         * this function. The purpose of this argument is only to prevent an
                                                         * unnecessary lookup. */
                      )
{
    SS_ENTER(ss_string_convert_fm2, herr_t);
    size_t              src_size;                       /* Size of the source (file) datatype */
    size_t              offset;                         /* Decoded offset into string table */
    size_t              i;

    if (!strtab) strtab = SS_SCOPE(&ss_table_scope_g)->m.strings;
    SS_ASSERT(strtab);

#if 1
    memset(dst, 0, sizeof *dst);
#endif

    /* Strings are stored as a little-endian offset into a strings table */
    if (0==(src_size=H5Tget_size(ss_string_tf))) SS_ERROR(HDF5);
    for (i=0, offset=0; i<src_size; i++) {
        offset |= ((const unsigned char*)src)[i] << (8*i);
    }
    dst->offset = offset;

    if (0==dst->offset) {
        dst->p = NULL; /*this could be strdup("") if so desired*/
        dst->nbytes = 0;
    } else {
        SS_ASSERT(strtab->buf);
        SS_ASSERT(dst->offset>=4); /*first value preceded by a four-byte length*/
        SS_ASSERT(dst->offset<strtab->buf_nused);
        SS_ASSERT(dst->offset+strlen((char*)(strtab->buf)+dst->offset)<strtab->buf_nused);
        dst->p = (char*)(strtab->buf) + dst->offset;
        dst->nbytes = strtab->buf[dst->offset-4] +
                      strtab->buf[dst->offset-3] * 256 +
                      strtab->buf[dst->offset-2] * 256 * 256 +
                      strtab->buf[dst->offset-1] * 256 * 256 * 256;
    }

SS_CLEANUP:
    SS_LEAVE(0);
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Convert string to memory type
 *
 * Description: This is an H5Tconvert() callback to convert ss_string_t objects in a dataset into memory.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, September 15, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_string_convert_fm(hid_t UNUSED src_type,     /* Source data type (should be ss_string_tf) */
                     hid_t UNUSED dst_type,     /* Destination data type (should be ss_string_tm) */
                     H5T_cdata_t *cdata,        /* Data type conversion control data */
                     size_t nelmts,             /* Number of elements to convert */
                     size_t buf_stride,         /* Distance between elements in bytes */
                     size_t UNUSED bkg_stride,  /* Distance between elements in the background buffer. */
                     void *buf,                 /* The buffer which is to be converted in place. */
                     void UNUSED *bkg,          /* The background buffer. */
                     hid_t UNUSED dxpl          /* Dataset transfer property list. */
                     )
{
    SS_ENTER(ss_string_convert_fm, herr_t);
    char                *src=NULL;
    char                *dst=NULL;
    ss_string_t         tmp;
    hsize_t             elmtno;
    ss_string_table_t   *strtab = NULL;
    size_t              src_size;

    SS_ASSERT(cdata);

    switch (cdata->command) {
    case H5T_CONV_INIT:
        /* See ss_string_init() for definitions of these types */
        SS_ASSERT(H5Tequal(dst_type, ss_string_tm));
        SS_ASSERT(H5Tequal(src_type, ss_string_tf));
        SS_ASSERT(H5Tget_size(dst_type)==sizeof(ss_string_t));
        SS_ASSERT(sizeof(ss_string_t)>=H5Tget_size(src_type)); /*implies right-to-left conversion*/
        break;

    case H5T_CONV_FREE:
        break;

    case H5T_CONV_CONV:
        if (0==(src_size=H5Tget_size(ss_string_tf))) SS_ERROR(HDF5);
        strtab = SS_SCOPE(&ss_table_scope_g)->m.strings;
        SS_ASSERT(strtab);
        src = (char*)buf + (nelmts-1) * (buf_stride ? buf_stride : src_size);
        dst = (char*)buf + (nelmts-1) * (buf_stride ? buf_stride : sizeof tmp);
        for (elmtno=0; elmtno<nelmts; elmtno++) {
            if (ss_string_convert_fm2(src, &tmp, strtab)<0) SS_ERROR(FAILED);
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
 * Chapter:     Strings
 * Purpose:     Checksum variable length strings
 *
 * Description: Free ss_string_t objects.
 *
 * Return:      Returns non-negative on success; negative on failure.  On success, Memory is freed.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, November 10, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_res_cb(void *buffer,          /* Address of first object to free. */
                 size_t size,           /* Size in bytes of each element of BUFFER. */
                 size_t nelmts          /* Number of objects to free. */
                 )
{
    SS_ENTER(ss_string_res_cb, herr_t);
    ss_string_t         *str = (ss_string_t*)buffer;
    size_t              i;

    SS_ASSERT(size==sizeof(ss_string_t));

    for (i=0; i<nelmts; i++, str++) {
        ss_string_reset(str);
    }

SS_CLEANUP:
    SS_LEAVE(0);
}


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Checksum variable length strings
 *
 * Description: Compute a checksum for zero or more ss_string_t objects.
 *
 * Return:      Returns non-negative on success; negative on failure.  On success, CKSUM is updated by folding in the new
 *              checksum value.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, November 10, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_string_sum_cb(void *buffer,                  /* Address of first object to checksum. */
                 size_t UNUSED size,            /* Size in bytes of each element of BUFFER. */
                 size_t nelmts,                 /* Number of objects to checksum. */
                 ss_val_cksum_t *cksum          /* INOUT: Checksum value to be updated. */
                 )
{
    SS_ENTER(ss_string_sum_cb, herr_t);
    const ss_string_t   *str = (const ss_string_t*)buffer;
    size_t              i;
    
    SS_ASSERT(cksum);
    SS_ASSERT(size==sizeof(ss_string_t));

    for (i=0; i<nelmts; i++, str++) {
        size_t slen = ss_string_memlen(str);
        cksum->adler = ss_adler32(cksum->adler, ss_string_ptr(str), slen);
    }

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Compare variable length strings
 *
 * Description: Pairwise comparison of zero or more objects of the BUF array with the KEY array.  The KEY describes the value
 *              for which we're searching and the FLAGS says how to search.
 *
 * Return:      On success returns negative if BUF is less than KEY, zero if they are equal, and one if BUF is greater than
 *              KEY. Returns -2 on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, November 10, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_string_cmp_cb(void *buf,                     /* Haystack array. */
                 void *key,                     /* Array of values for which we are searching. */
                 unsigned flags,                /* How to search (see SS_VAL_CMP). */
                 size_t UNUSED size,            /* Size of each object in the BUF and KEY arrays. */
                 size_t nelmts                  /* Number of elements in the BUF and KEY arrays. */
                 )
{
    SS_ENTER(ss_string_cmp_cb, int);
    ss_string_t         *str1=(ss_string_t*)buf;
    ss_string_t         *str2=(ss_string_t*)key;
    size_t              str1_len, str2_len;             /* length of string values in bytes, counting NUL terminator if any */
    const char          *str1_ptr, *str2_ptr;           /* pointers directly to STR1 and STR2 values */
    size_t              i;
    int                 retval=0;
    hbool_t             re_compiled=FALSE;
#if defined(HAVE_REGCOMP) && defined(HAVE_REGEXEC) && defined(REG_EXTENDED)
    int                 ecode, re_flags;
    char                estring[256];
    regex_t             re;

    memset(&re, 0, sizeof re);
#endif

    SS_RETVAL(-2);

    SS_ASSERT(size==sizeof(ss_string_t));
    for (i=0, retval=0; i<nelmts && 0==retval; i++, str1++, str2++) {

        /* Get string size and pointers to make things easier below */
        if (SS_NOSIZE==(str1_len=ss_string_memlen(str1))) SS_ERROR(FAILED);
        if (SS_NOSIZE==(str2_len=ss_string_memlen(str2))) SS_ERROR(FAILED);
        str1_ptr = ss_string_ptr(str1);
        str2_ptr = ss_string_ptr(str2);
        SS_ASSERT(0==str1_len || str1_ptr);
        SS_ASSERT(0==str2_len || str2_ptr);

        if (SS_VAL_CMP_RE==(flags & 0xF0)) {
#if defined(HAVE_REGCOMP) && defined(HAVE_REGEXEC) && defined(REG_EXTENDED)
            if (str1_len>0 && '\0'==str1_ptr[str1_len-1] && str2_len>0 && '\0'==str2_ptr[str2_len-1]) {
                /* The KEY contains a POSIX regular expression and both KEY and BUFFER are non-null C-style NUL-terminated
                 * strings. If the KEY doesn't match the BUFFER then we arbitrarily say that the KEY is less than the buffer. */

                /* Regular expression flags */
                re_flags = REG_NOSUB;
                if (SS_VAL_CMP_RE_EXTENDED==(flags & 0x0F)) re_flags |= REG_EXTENDED;
                if (SS_VAL_CMP_RE_ICASE==(flags & 0x0F))    re_flags |= REG_ICASE;
                if (SS_VAL_CMP_RE_NEWLINE==(flags & 0x0F))  re_flags |= REG_NEWLINE;

                /* Encode the regular expression string */
                ecode = regcomp(&re, str2_ptr, re_flags);
                re_compiled = TRUE;
                if (ecode) {
                    regerror(ecode, &re, estring, sizeof estring);
                    SS_ERROR_FMT(FAILED, ("regcomp: %s", estring));
                }

                /* Match the compiled regular expression against the search space. */
                ecode = regexec(&re, str1_ptr, 0, NULL, 0);
                if (ecode && REG_NOMATCH!=ecode) {
                    regerror(ecode, &re, estring, sizeof estring);
                    SS_ERROR_FMT(FAILED, ("regexec: %s", estring));
                }
                regfree(&re);
                re_compiled = FALSE;
                if (ecode) retval = 1;
            } else {
                retval = 1;
            }
#else
            SS_ERROR_FMT(NOTIMP, ("POSIX regular expressions are not supported on this platform"));
#endif
            
        } else if (SS_VAL_CMP_SUBSTR==(flags & 0xF0)) {
            if (str1_len>0 && '\0'==str1_ptr[str1_len-1] &&str2_len>0 && '\0'==str2_ptr[str2_len-1]) {
                /* Return equal if the C-style NUL-terminated string in KEY is a substring of the C-style NUL-terminted string in
                 * BUF. Arbitrarily return that KEY is less than BUF otherwise. */
                if (NULL==strstr(str1_ptr, str2_ptr)) retval = 1;
            } else {
                retval = 1;
            }
            
        } else if (SS_VAL_CMP_SUBMEM==(flags & 0xF0)) {
            if (str1_ptr && str2_ptr) {
                /* Return equal if the KEY is a substring of BUFFER, otherwise just arbitrarily say that the KEY is less than the
                 * buffer. The string values need not be NUL terminated and all bytes of the substring, including any NUL
                 * characters, must match. We have to use our own comparison loop here because memmem() is a GNU extension. */
                while (str1_len>=str2_len) {
                    if (0==memcmp(str1_ptr, str2_ptr, MIN(str1_len, str2_len))) break;
                    str1_ptr++;
                    --str1_len;
                }
                if (str1_len<str2_len) retval = 1;
            } else {
                retval = 1;
            }
        } else {
            /* Default is to just compare the values directly byte by byte. If one string is longer than the other but
             * otherwise equal in the leading bytes then the longer string is considered to be the greater value. If both
             * strings are null (both have length zero) then they are considered equal. */
            SS_ASSERT(SS_VAL_CMP_DFLT==flags);
            if (!str1_ptr && !str2_ptr) {
                retval = 0;
            } else if (!str1_ptr) {
                retval = -1;
            } else if (!str2_ptr) {
                retval = 1;
            } else if (0==(retval=memcmp(str1_ptr, str2_ptr, MIN(str1_len, str2_len)))) {
                if (str1_len>str2_len) retval = 1;
                else if (str1_len<str2_len) retval = -1;
            }
        }
    }
    retval = MINMAX(-1, retval, 1);

SS_CLEANUP:
#if defined(HAVE_REGCOMP) && defined(HAVE_REGEXEC) && defined(REG_EXTENDED)
    if (re_compiled) regfree(&re);
#endif
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Copy variable length strings
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
ss_string_copy_cb(void *_src, void *_dst, size_t UNUSED size, size_t nelmts)
{
    SS_ENTER(ss_string_copy_cb, herr_t);
    ss_string_t *dst = (ss_string_t*)_dst;
    const ss_string_t *src = (const ss_string_t*)_src;

    if (!src) src=dst;
    SS_ASSERT(sizeof(ss_string_t)==size);
    while (nelmts>0) {
        if (src==dst) {
            if (ss_string_realloc(dst)<0) SS_ERROR(FAILED);
        } else {
            memset(dst, 0, sizeof(*dst));
            if (ss_string_memset(dst, ss_string_ptr(src), ss_string_memlen(src))<0) SS_ERROR(FAILED);
        }
        dst++;
        src++;
        --nelmts;
    }
    
SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Encode strings
 *
 * Description: Encodes part of an object that cannot be sent directly to another task by copying it into a serialization
 *              buffer, SERBUF, which is allocated and/or extended as necessary.
 *
 * Return:      On success, returns a pointer to the possibly reallocated SERBUF; the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, November 21, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
ss_string_encode_cb(void *buffer,                       /* Array of objects to be encoded. */
                    char *serbuf,                       /* Buffer into which to place results. */
                    size_t *serbuf_nused,               /* INOUT: Number of bytes actually used in SERBUF. */
                    size_t *serbuf_nalloc,              /* INOUT: Number of bytes allocated for SERBUF. */
                    size_t UNUSED size,                 /* Size of each object in BUFFER. */
                    size_t nelmts                       /* Number of elements in BUFFER. */
                    )
{
    SS_ENTER(ss_string_encode_cb, charP);
    size_t              i, len;
    const ss_string_t   *str = buffer;
    
    SS_ASSERT(sizeof(ss_string_t)==size);

    for (i=0; i<nelmts; i++) {
        if (SS_NOSIZE==(len = ss_string_memlen(str))) SS_ERROR(FAILED);
        SS_EXTEND(serbuf, MAX(64,*serbuf_nused+4+len), *serbuf_nalloc);
        serbuf[(*serbuf_nused)++] = len & 0xff;
        serbuf[(*serbuf_nused)++] = (len >> 8) & 0xff;
        serbuf[(*serbuf_nused)++] = (len >> 16) & 0xff;
        serbuf[(*serbuf_nused)++] = (len >> 24) & 0xff;
        memcpy(serbuf + *serbuf_nused, ss_string_ptr(str), len);
        *serbuf_nused += len;
        str++;
    }

SS_CLEANUP:
    SS_LEAVE(serbuf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Strings
 * Purpose:     Decode strings
 *
 * Description: Decodes stuff encoded by ss_string_encode_cb().
 *
 * Return:      Returns total number of bytes consumed from SERBUF on success; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, November 21, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_string_decode_cb(void *buffer,                       /* Array of objects into which to decode SERBUF. */
                    const char *serbuf,                 /* Encoded information to be decoded. */
                    size_t size,                        /* Size of each element in BUFFER array. */
                    size_t nelmts                       /* Number of elements in BUFFER array. */
                    )
{
    SS_ENTER(ss_string_decode_cb, size_t);
    size_t              i, len;
    ss_string_t         *str = buffer;
    const char          *serbuf_orig=serbuf;

    SS_ASSERT(sizeof(ss_string_t)==size);
    for (i=0; i<nelmts; i++, str++) {
        SS_H5_DECODE(serbuf, 4, len);
        memset(str, 0, sizeof(*str));
        if (ss_string_memset(str, serbuf, len)<0) SS_ERROR(FAILED);
        serbuf += len;
    }

SS_CLEANUP:
    memset(buffer, 0, size*nelmts); /*make them all null strings on failure*/
    SS_LEAVE((size_t)(serbuf-serbuf_orig));
}

