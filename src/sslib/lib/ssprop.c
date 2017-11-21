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
SS_IF(prop);

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 *
 * Description: SSlib uses property lists to pass miscellaneous information to various functions, to pass properties through
 *              entire layers of the library, and as a mechanism to message pass information between MPI tasks. It relies
 *              heavily on HDF5's datatype description interface and data conversion functionality. The property list class
 *              ss_prop_t is a subclass of ss_obj_t and therefore has a magic number (see Magic Numbers).
 *
 *              Property lists are created and destroyed with ss_prop_new(), ss_prop_dup(), ss_prop_cons(), and ss_prop_dest().
 *
 *              Once a property list exists, properties can be added and initialized to default values with ss_prop_add(). The
 *              various ss_prop_set() and ss_prop_get() functions and their variants set and retrieve values of individual
 *              properties. The ss_prop_buffer() function is similar to the ss_prop_get() functions, but returns a pointer
 *              without copying the memory, and ss_prop_type() returns a datatype instead of the value.
 *
 *              The ss_prop_appendable(), ss_prop_modifiable(), and ss_prop_immutable() define (or query) what operations can
 *              be performed on a property list.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Properties
 * Purpose:     Interface initializer
 *
 * Description: Initializes the property list interface if it isn't already.  Initialization normally happens automatically
 *              the first time a property function is called, so explicit calls to this function are not normally necessary.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Monday, May 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_prop_init(void)
{
    SS_ENTER_INIT;
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Create a new property list from scratch
 *
 * Description: Creates a new property list and initializes it. An optional NAME may be specified and is used only for debugging.
 *
 * Return:      Returns new property list on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_prop_t *
ss_prop_new(const char *name            /* optional name for debugging */
            )
{
    SS_ENTER(ss_prop_new, ss_prop_tP);
    ss_prop_t   *prop = SS_OBJ_NEW(ss_prop_t);

    if (!prop) SS_ERROR(RESOURCE);
    if (name && NULL==(prop->name=strdup(name))) SS_ERROR(RESOURCE);
    
    prop->appendable = TRUE;
    prop->modifiable = TRUE;
    prop->destroyable = TRUE;
    prop->managed = TRUE;
    prop->type = -1;

 SS_CLEANUP:
    if (prop) ss_prop_dest(prop);
    SS_LEAVE(prop);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Create a new property list from an existing list
 *
 * Description: Duplicates an existing property list, giving it a new name (or a name generated from the source property
 *              list). The new property list's property names, values, and datatypes are copied from the specified PROP list.
 *              The new list is marked as appendable (new properties can be added) and modifiable (property values can be
 *              changed).
 *
 * Return:      Returns a new property list on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_prop_t *
ss_prop_dup(ss_prop_t *prop,            /* source property list */
            const char *name            /* optional name for debugging */
            )
{
    SS_ENTER(ss_prop_dup, ss_prop_tP);
    ss_prop_t   *newprop = NULL;
    
    SS_ASSERT_TYPE(prop, ss_prop_t);
    if (NULL==(newprop=ss_prop_new(name))) SS_ERROR(CONS);
    if (!name && prop->name)
        newprop->name = strdup(prop->name);
    if (prop->type>0) {
        size_t size;
        if (0==(size=H5Tget_size(newprop->type))) SS_ERROR(HDF5);
        if ((newprop->type=H5Tcopy(prop->type))<0) SS_ERROR(HDF5);
        if (NULL==(newprop->values=malloc(size))) SS_ERROR(RESOURCE);
        memcpy(newprop->values, prop->values, size);
    }

 SS_CLEANUP:
    if (newprop) ss_prop_dest(newprop);
    SS_LEAVE(newprop);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Property constructor
 *
 * Description: Construct a property from a datatype and optional memory. The datatype must be an HDF5 compound datatype and
 *              the memory must match that datatype.  If a VALUES buffer is supplied then the property list will be marked as
 *              non-appendable (new properties cannot be added) but modifiable (property values can be changed) and the
 *              ss_prop_dest() function will not free the memory.  But if VALUES is null then a buffer will be allocated and
 *              initialized to zeros.
 *
 * Return:      Returns a new property list on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_prop_t *
ss_prop_cons(hid_t type,                /* Property datatype (copied by this function) */
             void *values,              /* Optional initial values, of type TYPE */
             const char *name           /* Optional property list name */
             )
{
    SS_ENTER(ss_prop_cons, ss_prop_tP);
    ss_prop_t   *newprop = NULL;

    if (NULL==(newprop=ss_prop_new(name))) SS_ERROR(CONS);
    if ((newprop->type=H5Tcopy(type))<0) SS_ERROR(CONS);
    if (values) {
        newprop->values = values;
        newprop->appendable = FALSE;
        newprop->managed = FALSE;
    } else {
        if (NULL==(newprop->values=calloc(1, H5Tget_size(type)))) SS_ERROR(RESOURCE);
    }

 SS_CLEANUP:
    if (newprop) ss_prop_dest(newprop);
    SS_LEAVE(newprop);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Destroy a property list
 *
 * Description: All resources associated with the specified property list are released and the property list is marked as
 *              invalid and should not be referenced.  If the caller had not supplied a buffer to hold the values then that
 *              memory is also freed.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_prop_dest(ss_prop_t *prop)
{
    SS_ENTER(ss_prop_dest, herr_t);

    if (prop) {
        SS_ASSERT_TYPE(prop, ss_prop_t);
        if (!prop->destroyable) SS_ERROR(PERM);
        if (prop->managed) prop->values = SS_FREE(prop->values);
        prop->name = SS_FREE(prop->name);
        if (ss_obj_dest((ss_obj_t*)prop)<0) SS_ERROR(FAILED);
        prop = SS_FREE(prop);
    }

 SS_CLEANUP:
    SS_LEAVE(0);
}
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Add new property to a list
 *
 * Description: A new property called NAME is added to property list PROP. The new property can be given an initial value (or
 *              else all bits are cleared). If a value is specified it must be of type TYPE, which is the datatype of the
 *              property as stored in the property list.
 *
 * Return:      Returns non-negative on success; negative on failure. It is an error to add a property to the list if a
 *              property by that name already exists in the list. Only appendable property lists can have new properties
 *              added.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_prop_add(ss_prop_t *prop,            /* property list to which is added a property */
            const char *name,           /* name of property to add */
            hid_t type,                 /* datatype for stored property value */
            const void *value           /* optional initial property value */
            )
{
    SS_ENTER(ss_prop_add, herr_t);
    size_t      offset;

    SS_ASSERT_TYPE(prop, ss_prop_t);
    if (!prop->appendable) SS_ERROR(PERM);

    if (prop->type<=0) {
        if ((prop->type=H5Tcreate(H5T_COMPOUND, H5Tget_size(type)))<0) SS_ERROR(HDF5);
        offset = 0;
    } else {
        if (0==(offset=H5Tget_size(prop->type))) SS_ERROR(HDF5);
        if (H5Tset_size(prop->type, offset+H5Tget_size(type))<0) SS_ERROR(HDF5);
    }
    if (NULL==(prop->values=realloc(prop->values, H5Tget_size(prop->type)))) SS_ERROR(RESOURCE);
    
    if (H5Tinsert(prop->type, name, offset, type)<0) SS_ERROR(HDF5);
    if (value) {
        memcpy((char*)(prop->values)+offset, value, H5Tget_size(type));
    } else {
        memset((char*)(prop->values)+offset, 0, H5Tget_size(type));
    }
    
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Determine if property exists
 *
 * Description: This function determines if a property by the specified NAME exists in the property list.
 *
 * Return:      Returns true (positive) if NAME exists in PROP and false if not. Returns negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Saturday, November  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_prop_has(ss_prop_t *prop,            /* property list being queried */
            const char *name            /* name of property to be tested */
            )
{
    SS_ENTER(ss_prop_has, htri_t);
    htri_t      retval = FALSE;
    SS_ASSERT_TYPE(prop, ss_prop_t);

    if (prop->type && name && *name && H5Tget_member_index(prop->type, name)>=0)
        retval = TRUE;

SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Change a property value
 *
 * Description: A property's value can be modified by supplying a new VALUE with this function. If TYPE is specified then the
 *              VALUE will be of this type, which must be conversion-compatible with the type declared when the property was
 *              added to the list.  Otherwise, when no TYPE is specified the VALUE should be of the type originally specified
 *              to ss_prop_add(). If NAME is null then TYPE and VALUE refer to the entire property list rather than a single
 *              property.  The VALUE is copied into the property list, and if VALUE is a null pointer then the property (or
 *              entire property list if NAME is null) is reset to zero.
 *
 * Return:      Returns non-negative on success; negative on failure.  The property list must be marked as "modifiable" in
 *              order to change a property value.   If the specified TYPE is not conversion compatible with the stored type
 *              then the property is not modified.
 *
 * Parallel:    Indepdnent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_prop_set(ss_prop_t *prop,            /* property list to be modified */
            const char *name,           /* optional name of property to be modified */
            hid_t type,                 /* optional datatype for supplied value */
            const void *value           /* optional new property value */
            )
{
    SS_ENTER(ss_prop_set, herr_t);
    hid_t       stored_type=-1;         /* datatype stored in the property list */
    void        *convbuf=NULL;          /* temporary buffer for data conversion */
    void        *dest=NULL;             /* value's destination in the property list */

    if (!prop->modifiable) SS_ERROR(PERM);
    if ((stored_type=ss_prop_type(prop, name))<0) SS_ERROR(FAILED);
    if (NULL==(dest=ss_prop_buffer(prop, name))) SS_ERROR(FAILED);

    /* Store new value. If a conversion is necessary we must allocate a temporary buffer for it because we don't want to
     * modify the caller's memory nor do we want to change the property value until we're sure the conversion worked. */
    if (value) {
        if (type<=0 || H5Tequal(type, stored_type)>0) {
            memcpy(dest, value, H5Tget_size(stored_type));
        } else {
            size_t buf_size = MAX(H5Tget_size(type), H5Tget_size(stored_type));
            if (NULL==(convbuf = malloc(buf_size))) SS_ERROR(RESOURCE);
            memcpy(convbuf, value, H5Tget_size(type));
            if (H5Tconvert(type, stored_type, 1, convbuf, NULL, H5P_DEFAULT)<0) SS_ERROR(HDF5);
            memcpy(dest, convbuf, H5Tget_size(stored_type));
            convbuf = SS_FREE(convbuf);
        }
    } else {
        memset(dest, 0, H5Tget_size(stored_type));
    }
    H5Tclose(stored_type);
    stored_type=-1;
    
 SS_CLEANUP:
    if (stored_type>0) H5Tclose(stored_type);
    SS_FREE(convbuf);
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Change a signed integer property value
 *
 * Description: This is a convenience function for modifying an integer-valued property. See ss_prop_set() for details.
 *
 * Return:      See ss_prop_set().
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_prop_set_i(ss_prop_t *prop,          /* property list to be modified */
              const char *name,         /* name of property to be modified */
              int value                 /* new integer value */
              )
{
    SS_ENTER(ss_prop_set_i, herr_t);
    if (ss_prop_set(prop, name, H5T_NATIVE_INT, &value)<0) SS_ERROR(FAILED);
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Change an unsigned integer property value
 *
 * Description: This is a convenience function for modifying an unsigned integer-valued property. See ss_prop_set() for
 *              details.
 *
 * Return:      See ss_prop_set().
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_prop_set_u(ss_prop_t *prop,          /* property list to be modified */
              const char *name,         /* name of property to be modified */
              size_t value              /* new unsigned (size_t) value */
              )
{
    SS_ENTER(ss_prop_set_u, herr_t);
    if (ss_prop_set(prop, name, H5T_NATIVE_SIZE, &value)<0) SS_ERROR(FAILED);
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Change a floating-point property value
 *
 * Description: This is a convenience function for modifying a floating-point property. See ss_prop_set() for details.
 *
 * Return:      See ss_prop_set().
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_prop_set_f(ss_prop_t *prop,          /* property list to be modified */
              const char *name,         /* name of property to be modified */
              double value              /* new floating-point value */
              )
{
    SS_ENTER(ss_prop_set_f, herr_t);
    if (ss_prop_set(prop, name, H5T_NATIVE_DOUBLE, &value)) SS_ERROR(FAILED);
 SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Query a property value
 *
 * Description: The value of a property (or entire property list) can be queried by providing a handle to the property list
 *              and the name of the property (or NULL).  If a datatype is supplied then it must be conversion-compatible with
 *              the declared property (or property list) datatype, and the result will be returned as the specified datatype,
 *              otherwise the result is returned in the original datatype.  If a buffer is supplied then the value is copied
 *              into the buffer (the caller must ensure that the buffer is large enough), otherwise a buffer is allocated for
 *              the result.
 *
 * Return:      On success, returns either the supplied buffer or an newly allocated buffer which the caller should eventually
 *              free.  Returns the null pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_prop_get(ss_prop_t *prop,            /* property list to be queried */
            const char *name,           /* name of queried property */
            hid_t type,                 /* optional type of data to return */
            void *buffer                /* optional result buffer */
            )
{
    SS_ENTER(ss_prop_get, voidP);
    hid_t       stored_type=-1;         /* datatype stored in the property list */
    void        *convbuf=NULL;          /* temporary buffer for data conversion */
    void        *src=NULL;              /* source data from property list */
    void        *retval=NULL;           /* return value */

    if ((stored_type=ss_prop_type(prop, name))<0) SS_ERROR(FAILED);
    if (NULL==(src=ss_prop_buffer(prop, name))) SS_ERROR(FAILED);

    if (buffer) {
        if (type<=0 || H5Tequal(type, stored_type)>0) {
            /* No datatype conversion necessary */
            memcpy(buffer, src, H5Tget_size(stored_type));
        } else if (H5Tget_size(type)>=H5Tget_size(stored_type)) {
            /* User-supplied buffer is large enough to use for conversion */
            memcpy(buffer, src, H5Tget_size(stored_type));
            if (H5Tconvert(stored_type, type, 1, buffer, NULL, H5P_DEFAULT)<0) SS_ERROR(HDF5);
        } else {
            /* User-supplied buffer is too small for conversion */
            if (NULL==(convbuf=malloc(H5Tget_size(stored_type)))) SS_ERROR(RESOURCE);
            memcpy(convbuf, src, H5Tget_size(stored_type));
            if (H5Tconvert(stored_type, type, 1, convbuf, NULL, H5P_DEFAULT)<0) SS_ERROR(HDF5);
            memcpy(buffer, convbuf, H5Tget_size(type));
            convbuf = SS_FREE(convbuf);
        }
        retval = buffer;
    } else if (type<=0 || H5Tequal(type, stored_type)>0) {
        /* Allocate result buffer but no conversion necessary */
        if (NULL==(retval=malloc(H5Tget_size(stored_type)))) SS_ERROR(RESOURCE);
        memcpy(retval, src, H5Tget_size(stored_type));
    } else {
        /* Allocate result buffer and do data conversion */
        size_t buf_size = MAX(H5Tget_size(type), H5Tget_size(stored_type));
        if (NULL==(retval=malloc(buf_size))) SS_ERROR(RESOURCE);
        memcpy(retval, src, H5Tget_size(stored_type));
        if (H5Tconvert(stored_type, type, 1, retval, NULL, H5P_DEFAULT)<0) SS_ERROR(HDF5);
    }

 SS_CLEANUP:
    if (stored_type>0) H5Tclose(stored_type);
    if (!buffer) SS_FREE(retval);
    SS_FREE(convbuf);

    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Query an integer property
 *
 * Description: This is a convenience function for querying integer-valued properties. See ss_prop_get() for details.
 *
 * Return:      Returns -1 on failure.  Since this can also be a valid property value, the caller should examine the error
 *              stack to determine if an error in fact occurred.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_prop_get_i(ss_prop_t *prop,          /* property list to be queried */
              const char *name          /* name of queried property */
              )
{
    SS_ENTER(ss_prop_get_i, int);
    int         value;
    if (!ss_prop_get(prop, name, H5T_NATIVE_INT, &value)) SS_ERROR(FAILED);
 SS_CLEANUP:
    SS_LEAVE(value);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Query an unsigned integer property
 *
 * Description: This is a convenience function for querying unsigned integer-valued properties. See ss_prop_get() for details.
 *
 * Return:      Returns SS_NOSIZE on failure.  Since this can also be a valid property value, the caller should examine the
 *              error stack to determine if an error in fact occurred.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_prop_get_u(ss_prop_t *prop,          /* property list to be queried */
              const char *name          /* name of queried property */
              )
{
    SS_ENTER(ss_prop_get_u, size_t);
    size_t      value;
    if (!ss_prop_get(prop, name, H5T_NATIVE_SIZE, &value)) SS_ERROR(FAILED);
 SS_CLEANUP:
    SS_LEAVE(value);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Query a floating point property
 *
 * Description: This is a convenience function for querying floating point properties. See ss_prop_get() for details.
 *
 * Return:      Returns negative on failure.  Since this can also be a valid property value, the caller should examine the
 *              error stack to determine if an error in fact occurred.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
double
ss_prop_get_f(ss_prop_t *prop,          /* property list to be queried */
              const char *name          /* name of queried property */
              )
{
    SS_ENTER(ss_prop_get_f, double);
    double      value;
    if (!ss_prop_get(prop, name, H5T_NATIVE_DOUBLE, &value)) SS_ERROR(FAILED);
 SS_CLEANUP:
    SS_LEAVE(value);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Obtain pointer direct to value
 *
 * Description: This function is similar to ss_prop_get() except instead of copying the value or values into a new buffer, it
 *              returns a pointer directly into the property list values buffer.  If a property name is specified then the
 *              pointer is to the beginning of the specified property, otherwise the pointer is to the beginning of the entire
 *              property values buffer.
 *
 * Return:      Returns a pointer into the buffer holding property values on success; null on failure. If the client supplied
 *              the buffer for the values via the ss_prop_cons() function then the returned pointer is valid at least until
 *              the property list is destroyed, otherwise the pointer is valid only until the list is destroyed or a new
 *              property is added, whichever occurs first.
 *
 * Parallel:    
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_prop_buffer(ss_prop_t *prop,         /* property list to be queried */
               const char *name         /* optional property name */
               )
{
    SS_ENTER(ss_prop_buffer, voidP);
    size_t      offset=0;
    
    SS_ASSERT_TYPE(prop, ss_prop_t);

    /* Get the offset. This will fail if the property list has no properties yet or if the
     * specified name cannot be found in the compound datatype. */
    if (name) {
        int idx = H5Tget_member_index(prop->type, name);
        if (idx<0) SS_ERROR(HDF5);
        if (SS_NOSIZE==(offset=H5Tget_member_offset(prop->type, (unsigned)idx))) SS_ERROR(HDF5);
    }

 SS_CLEANUP:
    SS_LEAVE((char*)(prop->values) + offset);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Query the datatype of a property or property list
 *
 * Description: When given a property list and a property name, return the HDF5 datatype that was originally used to declare
 *              the property. If no property name is specified then return the HDF5 datatype of the whole property list (which
 *              is guaranteed to be a compound datatype).
 *
 * Return:      Returns an HDF5 datatype on success; negative on failure.  The client should eventually call H5Tclose() on the
 *              returned datatype. It is a failure to try to obtain the datatype of an empty property.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, February 25, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
ss_prop_type(ss_prop_t *prop,           /* property list to be queried */
             const char *name           /* optional property name */
             )
{
    SS_ENTER(ss_prop_type, hid_t);
    hid_t       retval=-1;
    
    SS_ASSERT_TYPE(prop, ss_prop_t);

    /* Get a copy of the datatype. This will fail if the property list has no properties yet or if the
     * specified name cannot be found in the compound datatype. */
    if (name) {
        int idx = H5Tget_member_index(prop->type, name);
        if (idx<0) SS_ERROR(HDF5);
        if ((retval=H5Tget_member_type(prop->type, (unsigned)idx))<0) SS_ERROR(HDF5);
    } else {
        if ((retval=H5Tcopy(prop->type))<0) SS_ERROR(HDF5);
    }

 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Queries/sets property list appendability
 *
 * Description: Queries and/or sets whether the property list is appendable. That is, whether new properties can be added to
 *              the property list. If NEW_VALUE is negative then the appendability status remains unchanged, otherwise it is
 *              set to the new value. Once a property list is marked as not appendable it cannot be later marked as appendable.
 *
 * Return:      Returns true (positive) if the property list was appendable before this call, false otherwise. Returns
 *              negative on failure. It is an error to attempt to make a non-appendable property list appendable.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_prop_appendable(ss_prop_t *prop, htri_t new_value)
{
    SS_ENTER(ss_prop_appendable, htri_t);
    htri_t retval = prop->appendable;
    SS_ASSERT_TYPE(prop, ss_prop_t);
    
    if (new_value>=0) {
        if (!prop->appendable && new_value) SS_ERROR(PERM);
        prop->appendable = new_value;
    }
 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Queries/sets property list modifiability
 *
 * Description: Queries and/or sets whether the property list is modifiable. That is, whether property values can be changed.
 *              If NEW_VALUE is negative then the modifiability status remains unchanged, otherwise it is set to the new
 *              value. Once a property list is marked as not modifiable it cannot be later marked as modifiable.
 *
 * Return:      Returns true (positive) if the property list was modifiable before this call, false otherwise. Returns
 *              negative on failure. It is an error to attempt to make a non-modifiable property list modifiable.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
htri_t
ss_prop_modifiable(ss_prop_t *prop, htri_t new_value)
{
    SS_ENTER(ss_prop_modifiable, htri_t);
    htri_t retval = prop->modifiable;
    SS_ASSERT_TYPE(prop, ss_prop_t);
    
    if (new_value>=0) {
        if (!prop->modifiable && new_value) SS_ERROR(PERM);
        prop->modifiable = new_value;
    }
 SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Properties
 * Purpose:     Make a property list immutable
 *
 * Description: This function that marks a property list as non-appendable and non-modifiable and non-destroyable. Once a
 *              property list is marked this way it is said to be immutable and cannot be changed in any way (not even
 *              destroyed except when the library is closed).
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_prop_immutable(ss_prop_t *prop)
{
    SS_ENTER(ss_prop_immutable, herr_t);
    SS_ASSERT_TYPE(prop, ss_prop_t);

    prop->modifiable = FALSE;
    prop->appendable = FALSE;
    prop->destroyable = FALSE;
 SS_CLEANUP:
    SS_LEAVE(0);
}

