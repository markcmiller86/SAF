/*
 * Copyright(C) 1999-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 1999-2005 Sandia Corporation.  
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
 * 
 * Active Developers:
 *     Peter K. Espen              SNL
 *     Eric A. Illescas            SNL
 *     Jake S. Jones               SNL
 *     Robb P. Matzke              LLNL
 *     Greg Sjaardema              SNL
 * 
 * Inactive Developers:
 *     William J. Arrighi          LLNL
 *     Ray T. Hitt                 SNL
 *     Mark C. Miller              LLNL
 *     Matthew O'Brien             LLNL
 *     James F. Reus               LLNL
 *     Larry A. Schoof             SNL
 * 
 * Acknowledgements:
 *     Marty L. Barnaby            SNL - Red parallel perf. study/tuning
 *     David M. Butler             LPS - Data model design/implementation Spec.
 *     Albert K. Cheng             NCSA - Parallel HDF5 support
 *     Nancy Collins               IBM - Alpha/Beta user
 *     Linnea M. Cook              LLNL - Management advocate
 *     Michael J. Folk             NCSA - Management advocate 
 *     Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
 *     Wilbur R. Johnson           SNL - Early developer
 *     Quincey Koziol              NCSA - Serial HDF5 Support 
 *     Celeste M. Matarazzo        LLNL - Management advocate
 *     Tyce T. McLarty             LLNL - parallel perf. study/tuning
 *     Tom H. Robey                SNL - Early developer
 *     Reinhard W. Stotzer         SNL - Early developer
 *     Judy Sturtevant             SNL - Red parallel perf. study/tuning 
 *     Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
 * 
 */
#include <safP.h>

/* This global array is used to determine the number of buckets in a hash table. These are all prime numbers where each prime
 * is approximatly double the previous prime.  This list can be modified by changing values, adding values, or deleting values
 * and controls the possible sizes of hash tables and how a hash table grows over time. The first value in the list determines
 * the initial size of a hash table and the last value is the maximum allowed size. */
static const size_t SAF_HashSize[] = {131, 311, 719, 1619, 3671, 8161, 17863, 38873, 84017, 180503,
                                      386093, 821641, 1742537, 3681131, 7754077, 16290047, 34136029,
                                      71378569, 148948139, 310248241, 645155197, 1339484197};

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Description:
 *      The functions defined in this interface are for the manipulation of very simple hash tables. The tables are indexed by
 *      a hash key which is computed with the various saf_hkey functions, and the values stored in a particular bucket are
 *      object links.
 *
 *      The hash table is just an array (indexed by hash key modulus the array size) which points to buckets. Each bucket is
 *      a variable length array of object links.
 *
 *      Hash tables are intended to be so general in nature that they can be used for a variety of things, but yet we try to
 *      keep them as simple as possible.  One thing they're used for is to speed up common object searches. For instance, it's
 *      common to search for quantities by name and abbreviation (e.g., saf_find_one_quantity()). Since quantities are only
 *      ever added to a scope with saf_declare_quantity() and since their names and abbreviations cannot be changed after
 *      they're declared (at least not with the SAF API), saf_declare_quantity() can create a hash keyed by both name and
 *      abbreviation and saf_find_one_quantity() can use that hash for near constant-time lookup.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Purpose:     Create a new hash table
 *
 * Description: A new hash table is created by calling this function. The new table has some initial size (number of buckets)
 *              controlled by the SAF_HashSize global array.
 *
 * Return:      Returns a pointer to a new hash table on success; returns null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_HashTable *
_saf_htab_new(void)
{
    SAF_ENTER(_saf_htab_new, NULL);
    SAF_HashTable *retval = calloc(1, sizeof(*retval));
    retval->bucket = calloc(SAF_HashSize[0], sizeof(retval->bucket[0]));
    SAF_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Purpose:     Destroy a hash table
 *
 * Description: The HTAB is destroyed by freeing all value arrays, the buckets, and the table itself.  The objects to which
 *              the object links point are not affected in any way.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
_saf_htab_dest(SAF_HashTable *htab)
{
    SAF_ENTER(_saf_htab_dest, -1);
    size_t      i;

    for (i=0; i<SAF_HashSize[htab->rank]; i++) {
        SS_FREE(htab->bucket[i].key);
        SS_FREE(htab->bucket[i].pers);
    }
    SS_FREE(htab->bucket);

    SAF_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Purpose:     Insert a new key/value pair into the table
 *
 * Description: The object link VALUE is inserted into table HTAB using the KEY.  Hash tables can have multiple values (or
 *              even the same value) all having the same key, and a _saf_htab_find() will return all those values with the
 *              specified key.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
_saf_htab_insert(SAF_HashTable *htab, size_t key, ss_pers_t *value)
{
    SAF_ENTER(_saf_htab_insert, -1);
    SAF_HashBucket      *bucket=NULL;

    /* Rehash the table if appropriate */
    if (htab->nvals/SAF_HashSize[htab->rank] > 8)
        _saf_htab_rehash(htab, htab->rank+1);

    /* Extend the bucket if necessary */
    bucket = htab->bucket + (key % SAF_HashSize[htab->rank]);
    if (bucket->nused>=bucket->nalloc) {
        bucket->nalloc = MAX(8, 2*bucket->nalloc);
        bucket->key = realloc(bucket->key, bucket->nalloc*sizeof(bucket->key[0]));
        bucket->pers = realloc(bucket->pers, bucket->nalloc*sizeof(bucket->pers[0]));
    }

    /* Add the new value to the end of the bucket */
    bucket->key[bucket->nused] = key;
    bucket->pers[bucket->nused] = *value;
    bucket->nused++;
    htab->nvals++;

    SAF_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Purpose:     Find values for specified key
 *
 * Description: Given a hash table and a key, return (some of) the values that are stored with that key. The number of values
 *              to return is indicated by the LIMIT argument. The caller may supply a buffer for the result, or this function
 *              will allocate a buffer to hold the result.
 *
 * Return:      On success, returns a pointer to the buffer containing the values that were stored with the specified KEY.
 *              This buffer may be either the buffer that the caller passed in as the BUFFER argument, or if the caller passed
 *              a null pointer then a buffer is allocated.  Returns null on failure.  It is not a failure to not find any
 *              objects.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
_saf_htab_find(SAF_HashTable *htab,             /* The table to be searched. */
               size_t key,                      /* Return only values that were inserted using this key. */
               size_t *limit,                   /* On input, this is the maximum number of values that should be returned. Upon
                                                 * return this will be the number of values actually returned. The input value
                                                 * of LIMIT may be the constant SS_NOSIZE to indicate that all matches should
                                                 * be returned. Obviously, the caller shouldn't pass in SS_NOSIZE if they're
                                                 * also supplying a non-null BUFFER argument. */
               ss_pers_t *buffer                /* An optional buffer in which to store the found values. If the caller
                                                 * supplies this buffer then the input value of LIMIT should be used to limit
                                                 * the number of values this function writes into that return buffer. If the
                                                 * caller does not supply a buffer then this function allocates one. */
               )
{
    SAF_ENTER(_saf_htab_find, NULL);
    SAF_HashBucket      *bucket = htab->bucket + (key % SAF_HashSize[htab->rank]);
    size_t              nfound=0, nalloc=0, i;
    ss_pers_t           *retval=buffer;

    /* Append values with specified KEY to the return buffer */
    for (i=0; i<bucket->nused; i++) {
        if (nfound>=*limit) break;
        if (bucket->key[i]==key) {
            if (!buffer && nfound>=nalloc) {
                nalloc = MAX(8, 2*nalloc);
                retval = realloc(retval, nalloc*sizeof(*retval));
            }
            retval[nfound++] = bucket->pers[i];
        }
    }

    /* If we didn't find anything then make sure we return a non-null value */
    if (!retval) retval = calloc(1, sizeof(*retval));

    *limit = nfound;
    SAF_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Purpose:     Change number of buckets
 *
 * Description: As entries are added to a hash table the buckets must become larger and larger. At some point hash table
 *              lookups begin take on a linear-order complexity and the number of buckets should be increased in order to give
 *              more constant time lookups. Rehashing is a linear time operation.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
_saf_htab_rehash(SAF_HashTable *htab, int newrank)
{
    SAF_ENTER(_saf_htab_rehash, -1);
    SAF_HashBucket      *oldbucket=NULL;
    size_t              oldrank, i, j;

    if (newrank!=htab->rank) {
        oldrank = htab->rank;
        oldbucket = htab->bucket;
        htab->rank = newrank;
        htab->bucket = calloc(SAF_HashSize[newrank], sizeof(htab->bucket[0]));

        for (i=0; i<SAF_HashSize[oldrank]; i++) {
            for (j=0; j<oldbucket[i].nused; j++) {
                _saf_htab_insert(htab, oldbucket[i].key[j], oldbucket[i].pers+j);
            }
            SS_FREE(oldbucket[i].key);
            SS_FREE(oldbucket[i].pers);
        }
    }

    SAF_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Purpose:     Mix 32 bit values reversibly
 *
 * Description: For every delta with one or two bits set, and the deltas of all three high bits or all three low bits, whether
 *              the original value of a,b,c is almost all zero or is uniformly distributed,
 *
 *              * If mix() is run forward or backward, at least 32 bits in a,b,c have at least 1/4 probability of changing.
 *                
 *              * If mix() is run forward, every bit of c will change between 1/3 and 2/3 of the time.  (Well, 22/100 and
 *                78/100 for some 2-bit deltas.)
 *
 *              This macro was built out of 36 single-cycle latency instructions in a structure that could supported 2x
 *              parallelism, like so:
 *
 *                a -= b; 
 *                a -= c; x = (c>>13);
 *                b -= c; a ^= x;
 *                b -= a; x = (a<<8);
 *                c -= a; b ^= x;
 *                c -= b; x = (b>>13);
 *                ...
 * 
 *              Unfortunately, superscalar Pentiums and Sparcs can't take advantage of that parallelism.  They've also turned
 *              some of those single-cycle latency instructions into multi-cycle latency instructions.  Still, this is the
 *              fastest good hash I could find.  There were about 2^^68 to choose from.  I only looked at a billion or so.
 *
 *              This code (and documentation) came from http://burtleburtle.net/bob/hash/doobs.html
 *
 * Return:      Modifies A, B, and C arguments in place to mix them.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define mix(a,b,c) {                                                                                                           \
  a -= b; a -= c; a ^= (c>>13);                                                                                                \
  b -= c; b -= a; b ^= (a<<8);                                                                                                 \
  c -= a; c -= b; c ^= (b>>13);                                                                                                \
  a -= b; a -= c; a ^= (c>>12);                                                                                                \
  b -= c; b -= a; b ^= (a<<16);                                                                                                \
  c -= a; c -= b; c ^= (b>>5);                                                                                                 \
  a -= b; a -= c; a ^= (c>>3);                                                                                                 \
  b -= c; b -= a; b ^= (a<<10);                                                                                                \
  c -= a; c -= b; c ^= (b>>15);                                                                                                \
}

#if defined JANUS || defined WIN32
#define uint32_t unsigned
#endif

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Purpose:     Compute a hash value
 *
 * Description: Every bit of the key affects every bit of the return value.  Every 1-bit and 2-bit delta achieves avalanche.
 *              About 6*len+35 instructions.
 *
 *              The best hash table sizes are powers of 2.  There is no need to do mod a prime (mod is sooo slow!).  If you
 *              need less than 32 bits, use a bitmask.  For example, if you need only 10 bits, do
 *
 *                h = (h & hashmask(10));
 *
 * Return:      Returns a hash value
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 25, 2004
 *              Borrowed heavily from http://burtleburtle.net/bob/hash/evahash.html with permission.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static uint32_t
hash(const unsigned char *k,    /* the memory to be hashed */
     size_t length,             /* The length of K */
     size_t initval             /* Previous hash, or an arbitrary initial value */
     )
{
    uint32_t a, b, c, len;

    /* Set up the internal state */
    len = length;
    a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
    c = initval;         /* the previous hash value */

    /* Handle most of the key */
    while (len >= 12) {
        a += (k[0] +((uint32_t)k[1]<<8) +((uint32_t)k[2]<<16) +((uint32_t)k[3]<<24));
        b += (k[4] +((uint32_t)k[5]<<8) +((uint32_t)k[6]<<16) +((uint32_t)k[7]<<24));
        c += (k[8] +((uint32_t)k[9]<<8) +((uint32_t)k[10]<<16)+((uint32_t)k[11]<<24));
        mix(a,b,c);
        k += 12; len -= 12;
    }

    /* Handle the last 11 or fewer bytes. All case statements fall through. */
    c += length;
    switch(len) {
    case 11: c+=((uint32_t)k[10]<<24);
    case 10: c+=((uint32_t)k[9]<<16);
    case 9 : c+=((uint32_t)k[8]<<8);
        /* the first byte of c is reserved for the length */
    case 8 : b+=((uint32_t)k[7]<<24);
    case 7 : b+=((uint32_t)k[6]<<16);
    case 6 : b+=((uint32_t)k[5]<<8);
    case 5 : b+=k[4];
    case 4 : a+=((uint32_t)k[3]<<24);
    case 3 : a+=((uint32_t)k[2]<<16);
    case 2 : a+=((uint32_t)k[1]<<8);
    case 1 : a+=k[0];
        /* case 0: nothing left to add */
    }
    mix(a,b,c);

    return c;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Hash Tables
 * Purpose:     Hash a character string
 *
 * Description: Given a NUL-terminated character string, compute a hash key and return it.
 *
 * Return:      Returns a hash key.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, September 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
_saf_hkey_str(const char *s)
{
    size_t len = s?strlen(s):0;
    return hash(s, len, 0);
}
