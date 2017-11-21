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
#ifndef SS_HAVE_SSATTR_H
#define SS_HAVE_SSATTR_H

typedef struct ss_attrobj_tm {          /* Memory-resident transient information */
    ss_persobj_t        pers;           /* Must be first! */
} ss_attrobj_tm;

typedef struct ss_attrobj_tf {          /* Memory-resident persistent information */
    ss_string_t         name;           /* Name of the attribute */
    ss_pers_t           owner;          /* Object to which this attribute belongs. */
    ss_array_t          value;          /* Variable length array of values */
} ss_attrobj_tf;

/* Types generated from above declarations */
#include "ssattrtab.h"

#ifdef __cplusplus
extern "C" {
#endif

herr_t ss_attr_init(void);
herr_t ss_attrtab_init(void);
ss_attr_t *ss_attr_new(ss_pers_t *owner, const char *name, hid_t type, size_t count, const void *value, unsigned flags,
                       ss_attr_t *buf, ss_prop_t *props);
ss_attr_t *ss_attr_find(ss_pers_t *owner, const char *name, size_t skip, size_t maxret, size_t *nret, ss_attr_t *result);
size_t ss_attr_count(ss_pers_t *owner, const char *name);
void *ss_attr_get(ss_attr_t *attr, hid_t type, size_t offset, size_t nelmts, void *buffer);
herr_t ss_attr_put(ss_attr_t *attr, hid_t type, size_t offset, size_t nelmts, const void *value, unsigned flags);
herr_t ss_attr_modify(ss_attr_t *attr, hid_t type, size_t nelmts, unsigned flags);
const char *ss_attr_describe(ss_attr_t *attr, ss_pers_t *owner, hid_t *type, size_t *nelmts);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSATTR_H*/
