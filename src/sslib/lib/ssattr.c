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
SS_IF(attr);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Attributes
 *
 * Description: Attributes are small pieces of data which fall outside the scope of the sharable data model and thus cannot be
 *              represented by sets, fields, etc. The meaning of a particular attribute is determined by convention, requiring
 *              additional a priori agreement between the writer and the reader, often in the form of documentation or word of
 *              mouth.
 *
 *              Any persistent object may have zero or more attributes and each attribute has a name, datatype, element count
 *              (as if it were a one dimensional array), and a value. Operations on attributes are largely independent and
 *              since attributes are implemented as a scope table, those operations are similar to operations that can be
 *              performed on other persistent objects.  One restriction on the attribute table, however, is that the attribute
 *              can only belong to an object stored in the same scope. This is necessary in order for an object to be able to
 *              efficiently find its attributes.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Object Attributes
 * Purpose:     Interface initializer
 *
 * Description: Initializes the attribute interface.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, January 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_attr_init(void)
{
    SS_ENTER_INIT;
    if (ss_attrtab_init()<0) SS_ERROR(INIT);
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Attributes
 * Purpose:     Add a new attribute to an object
 *
 * Description: This function adds a new attribute to the OWNER object (which must not be an attribute itself). An attribute
 *              can be thought of as a one dimensional array of values all having the same datatype.
 *
 * Return:      Returns a non-null attribute link on success; null on failure. If BUF is supplied then it will be the success
 *              return value.
 *
 * Parallel:    Independent. However if all tasks are collectively creating a single attribute and all are passing the same
 *              TYPE, COUNT, and VALUE then they may pass the SS_ALLSAME bit in the FLAGS argument, thereby allowing a
 *              synchronization to do less work later.  When the SS_ALLSAME bit is passed then the call should be collective
 *              across the communicator of the scope containing the OWNER object.
 *
 * Programmer:  Robb Matzke
 *              Saturday, January 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_attr_t *
ss_attr_new(ss_pers_t *owner,                   /* The object with which the new attribute is associated. */
            const char *name,                   /* The name of the new attribute. */
            hid_t type,                         /* The datatype of the attribute. */
            size_t count,                       /* Number of values in the attribute. */
            const void *value,                  /* Optional array of COUNT values each of type TYPE. If this array is not
                                                 * supplied then the attribute's value will be initialized to all zero bytes. */
            unsigned flags,                     /* Bit flags, such as SS_ALLSAME. */
            ss_attr_t *buf,                     /* The optional buffer for the returned attribute link. */
            ss_prop_t *props                    /* Attribute properties (none defined yet). */
            )
{
    SS_ENTER(ss_attr_new, ss_attr_tP);
    ss_attr_t   *attr=NULL;                     /* The returned attribute linke */
    ss_scope_t  scope;                          /* The scope containing OWNER and in which the attribute will be stored */

    /* Check arguments */
    SS_ASSERT_CLASS(owner, ss_pers_t);
    if (SS_MAGIC(ss_attr_t)==SS_MAGIC_OF(owner)) SS_ERROR_FMT(USAGE, ("attribute owner must not be an attribute"));
    if (!name || !*name) SS_ERROR_FMT(USAGE, ("attribute name must be specified"));
    if (count<=0) SS_ERROR_FMT(USAGE, ("attribute count must be positive"));

    /* Create a new attribute object */
    if (NULL==ss_pers_scope(owner, &scope)) SS_ERROR(FAILED);
    if (NULL==(attr=(ss_attr_t*)ss_pers_new(&scope, SS_MAGIC(ss_attr_t), NULL, flags, (ss_pers_t*)buf, props))) SS_ERROR(FAILED);

    /* Initialize the attribute */
    SS_ATTR(attr)->owner = *owner;
    if (ss_string_set(SS_ATTR_P(attr,name), name)<0) SS_ERROR(FAILED);
    if (ss_array_target(SS_ATTR_P(attr,value), type)<0) SS_ERROR(FAILED);
    if (ss_array_resize(SS_ATTR_P(attr,value), count)<0) SS_ERROR(FAILED);
    if (value && ss_array_put(SS_ATTR_P(attr,value), type, (size_t)0, count, value)<0) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(attr);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Attributes
 * Purpose:     Find attributes for an object
 *
 * Description: This function finds all attributes for the persistent object OWNER and returns handles to those attributes.
 *              The returned array of handles can be restricted by supplying an attribute name which must match all returned
 *              attributes. The SKIP and MAXRET arguments can select a contiguous subset of the available attributes.
 *
 * Return:      On success, returns an array of links to matching attributes; returns a null pointer on failure. If no
 *              matching attributes are found then a non-null malloc'd pointer is returned and NRET points to zero. That is,
 *              the case were no attributes match the search criteria is not considered an error.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, January 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_attr_t *
ss_attr_find(ss_pers_t *owner,                  /* The object for which we're searching for attributes. */
             const char *name,                  /* An optional attribute name on which to restrict the search. */
             size_t nskip,                      /* Skip the first SKIP matching attributes. */
             size_t maxret,                     /* Return at most MAXRET matching attributes. If the caller passes SS_NOSIZE
                                                 * then all matching attributes are returned. If more than MAXRET attributes
                                                 * could be returned the remainder are simply discarded.  If RESULT is
                                                 * non-null then this argument should reflect the size of that array. */
             size_t *nret,                      /* OUT: The number of attributes stored in the returned array of links. */
             ss_attr_t *result                  /* An optional buffer in which to store links to the matching attributes. If
                                                 * supplied, this will be the successful return value. The constant
                                                 * SS_PERS_TEST can be supplied in order to prevent the library from
                                                 * allocating a return value (this is useful if the caller simply wants to
                                                 * count the matches). */
             )
{
    SS_ENTER(ss_attr_find, ss_attr_tP);
    static ss_attr_t    *key;                   /* Key of values to use when searching the attribute table */
    ss_attrobj_t        mask;                   /* What parts of KEY are significant and how? */
    ss_scope_t          scope=SS_SCOPE_NULL;    /* The scope containing OWNER */
    
    SS_ASSERT_CLASS(owner, ss_pers_t);
    if (!nret) SS_ERROR_FMT(USAGE, ("NRET argument was null"));
    if (NULL==ss_pers_scope(owner, &scope)) SS_ERROR(FAILED);

    /* Create or reset key and mask */
    memset(&mask, 0, sizeof mask);
    if (!key && NULL==(key=SS_PERS_NEW(sslib_g.temp.tscope, ss_attr_t, 0))) {
        SS_ERROR(FAILED);
    } else if (ss_pers_reset((ss_pers_t*)key, 0)<0) {
        SS_ERROR(FAILED);
    }
    
    /* Initialize key and mask with values for which to search */
    SS_ATTR(key)->owner = *owner;
    memset(&(mask.owner), SS_VAL_CMP_DFLT, 1);
    if (name) {
        ss_string_set(SS_ATTR_P(key,name), name);
        memset(&(mask.name), SS_VAL_CMP_DFLT, 1);
    }
    
    /* Search */
    *nret = maxret;
    if (NULL==(result=(ss_attr_t*)ss_pers_find(&scope, (ss_pers_t*)key, (ss_persobj_t*)&mask, nskip, nret,
                                               (ss_pers_t*)result, NULL))) {
        SS_ERROR(FAILED);
    }
    
SS_CLEANUP:
    if (nret) *nret = 0;
    SS_LEAVE(result);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Attributes
 * Purpose:     Count matching attributes
 *
 * Description: Counts the number of attributes that belong to OWNER and have the optional string NAME as their name. If no
 *              NAME is supplied then all attributes belonging to OWNER are counted.
 *
 * Return:      On success, the number of attributes found to match the OWNER and NAME pair; SS_NOSIZE on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, January 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_attr_count(ss_pers_t *owner,
              const char *name
              )
{
    SS_ENTER(ss_attr_count, size_t);
    size_t      retval;

    if (NULL==ss_attr_find(owner, name, 0, SS_NOSIZE, &retval, SS_PERS_TEST)) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Attributes
 * Purpose:     Obtain attribute value
 *
 * Description: This function extracts the attribute value, or subpart thereof. The value is converted to the desired TYPE,
 *              which must be conversion compatible with the datatype used to store the attribute value. The converted value
 *              is copied into the optional caller-supplied BUFFER (or a buffer is allocated). If the value consists of more
 *              than one element then desired elements to be returned can be specified with an offset and length.
 *
 * Return:      Returns a pointer (BUFFER if non-null) on success; null on failure.  If the caller doesn't supply BUFFER then
 *              this function allocates one.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, January 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_attr_get(ss_attr_t *attr,                    /* The attribute in question. */
            hid_t type,                         /* The desired datatype of the returned value. */
            size_t offset,                      /* The first element of the value to return (an element index, not byte index) */
            size_t nelmts,                      /* The total number of elements to return. If the OFFSET and NELMTS arguments
                                                 * describe a range of elements that is outside that which is known to the
                                                 * attribute then an error is raised. If NELMTS is SS_NOSIZE then the number
                                                 * of returned values is not limited (except perhaps by the non-zero OFFSET). */
            void *buffer                        /* The optional buffer in which to store the returned values. */
            )
{
    SS_ENTER(ss_attr_get, voidP);

    SS_ASSERT_MEM(attr, ss_attr_t);
    if (NULL==(buffer=ss_array_get(SS_ATTR_P(attr,value), type, offset, nelmts, buffer))) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(buffer);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Attributes
 * Purpose:     Change an attribute value
 *
 * Description: An attribute's value can be changed by calling this function. If the attribute stores more than one value then
 *              the supplied new VALUE can be for either the whole attribute or for just part of the attribute as determined
 *              by the OFFSET and NELMTS arguments. The datatype of each element of VALUE is specified by TYPE and must be
 *              conversion compatible with the datatype already registered with the attribute.
 *
 * Return:      Returns non-negative on success; negative on failure. It is an error if OFFSET and NELMTS specify a range of
 *              elements ouside that which the attribute already knows about.
 *
 * Parallel:    Independent. However, if the SS_ALLSAME bit is passed in the FLAGS argument the call should be collective
 *              across all tasks of the communicator for the scope that owns the attribute and all such tasks must pass
 *              identical values for OFFSET and NELMTS and a TYPE and VALUE such that the converted value is identical on all
 *              tasks.
 *
 * Programmer:  Robb Matzke
 *              Saturday, January 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_attr_put(ss_attr_t *attr,                    /* The attribute in question. */
            hid_t type,                         /* The datatype of VALUE. */
            size_t offset,                      /* The element number at which to put VALUE. */
            size_t nelmts,                      /* The number of elements in VALUE. */
            const void *value,                  /* The value to be written to the attribute. */
            unsigned flags                      /* Flags such as SS_ALLSAME. */
            )
{
    SS_ENTER(ss_attr_put, herr_t);
    SS_ASSERT_MEM(attr, ss_attr_t);

    if (ss_pers_modified((ss_pers_t*)attr, flags)<0) SS_ERROR(FAILED);
    if (ss_array_put(SS_ATTR_P(attr,value), type, offset, nelmts, value)<0) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Attributes
 * Purpose:     Modify attribute type and size
 *
 * Description: This function modifies the storage datatype and/or number of elements of an attribute.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. However if the SS_ALLSAME bit is set in the FLAGS argument then this function is collective
 *              across the communicator of the scope that owns the attribute and all tasks must pass identical values for the
 *              TYPE and NELMTS.
 *
 * Programmer:  Robb Matzke
 *              Saturday, January 31, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_attr_modify(ss_attr_t *attr,                 /* The attribute whose size will be changed. */
               hid_t type,                      /* The new datatype for the attribute, or H5I_INVALID_HID if the type is not
                                                 * to be changed. If the datatype is changed but is conversion compatible with
                                                 * the previous type then the attribute's data will be converted to the new
                                                 * datatype. Otherwise the attribute's data will be initialized to all zero. */
               size_t nelmts,                   /* The new number of elements in the attribute value, or SS_NOSIZE if the
                                                 * number of elements is not to be changed. If the number of elements
                                                 * decreases then the extra elements are discarded. If the number of elements
                                                 * increases then the new elements will be initialized to all zero bytes. */
               unsigned flags                   /* Bit flags such as SS_ALLSAME. */
               )
{
    SS_ENTER(ss_attr_modify, herr_t);
    void                *values=NULL;
    size_t              old_nelmts;

    SS_ASSERT_MEM(attr, ss_attr_t);

    if (type>0) {
        if (NULL==(values=ss_array_get(SS_ATTR_P(attr,value), type, (size_t)0, SS_NOSIZE, NULL))) {
            /* Types are not conversion compatible; we'll zero them instead */
            SS_STATUS_OK;
        }
        if (SS_NOSIZE==(old_nelmts=ss_array_nelmts(SS_ATTR_P(attr,value)))) SS_ERROR(FAILED);
        if (ss_pers_modified((ss_pers_t*)attr, flags)<0) SS_ERROR(FAILED);
        if (ss_array_resize(SS_ATTR_P(attr,value), 0)<0) SS_ERROR(FAILED); /*zero size or else we can't change the type*/
        if (ss_array_target(SS_ATTR_P(attr,value), type)<0) SS_ERROR(FAILED);
        if (ss_array_resize(SS_ATTR_P(attr,value), old_nelmts)<0) SS_ERROR(FAILED); /*restore original size*/
        if (values && ss_array_put(SS_ATTR_P(attr,value), type, (size_t)0, old_nelmts, values)<0) SS_ERROR(FAILED);
        values = SS_FREE(values);
    }

    if (SS_NOSIZE!=nelmts) {
        if (ss_array_resize(SS_ATTR_P(attr,value), nelmts)<0) SS_ERROR(FAILED);
    }

SS_CLEANUP:
    SS_FREE(values);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Object Attributes
 * Purpose:     Query attribute metadata
 *
 * Description: This is really just a convenience function so if the caller wants the attribute datatype they don't need to
 *              do the work of calling H5decode() on the attribute !type field.
 *
 * Return:      Returns the attribute name on success; null on failure.  The name is simply the ss_string_ptr() value of the
 *              !name field of the attribute.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, February  9, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
const char *
ss_attr_describe(ss_attr_t *attr,               /* The attribute to be described. */
                 ss_pers_t *owner,              /* OUT: Optional object to which attribute belongs. */
                 hid_t *type,                   /* OUT: Optional datatype of attribute data. The caller should invoke
                                                 * H5Tclose() when this is no longer needed. */
                 size_t *nelmts                 /* OUT: Optional number of elements stored by the attribute. */
                 )
{
    SS_ENTER(ss_attr_describe, const_charP);
    const char *retval=NULL;

    SS_ASSERT_MEM(attr, ss_attr_t);

    if (owner) *owner = SS_ATTR(attr)->owner;
    if (type && (*type=ss_array_targeted(SS_ATTR_P(attr,value)))<0) SS_ERROR(FAILED);
    if (nelmts && (SS_NOSIZE==(*nelmts=ss_array_nelmts(SS_ATTR_P(attr,value))))) SS_ERROR(FAILED);
    if (NULL==(retval=ss_string_ptr(SS_ATTR_P(attr,name)))) SS_ERROR(FAILED);

SS_CLEANUP:
    SS_LEAVE(retval);
}
