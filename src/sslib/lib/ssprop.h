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
#ifndef SS_HAVE_SSPROP_H
#define SS_HAVE_SSPROP_H

/* A property list is a list of name/type/value triplets. See ssprops.c for details. */
typedef struct ss_prop_t {
    ss_obj_t            obj;                    /* fields common to almost all SSlib objects (must be first) */
    char                *name;                  /* optional name for debugging */
    hbool_t             appendable;             /* can new properties be added? */
    hbool_t             modifiable;             /* can property values be changed? */
    hbool_t             destroyable;            /* can property list be destroyed? */
    hbool_t             managed;                /* is the memory managed by SSlib? */
    hid_t               type;                   /* compound datatype describing whole property */
    void                *values;                /* a buffer to hold the property values */
} ss_prop_t;

#ifdef __cplusplus
extern "C" {
#endif

herr_t ss_prop_init(void);
ss_prop_t *ss_prop_new(const char *name);
ss_prop_t *ss_prop_dup(ss_prop_t *prop, const char *name);
ss_prop_t *ss_prop_cons(hid_t type, void *values, const char *name);
herr_t ss_prop_dest(ss_prop_t *prop);
herr_t ss_prop_add(ss_prop_t *prop, const char *name, hid_t type, const void *value);
htri_t ss_prop_has(ss_prop_t *prop, const char *name);
herr_t ss_prop_set(ss_prop_t *prop, const char *name, hid_t type, const void *value);
herr_t ss_prop_set_i(ss_prop_t *prop, const char *name, int value);
herr_t ss_prop_set_u(ss_prop_t *prop, const char *name, size_t value);
herr_t ss_prop_set_f(ss_prop_t *prop, const char *name, double value);
void *ss_prop_get(ss_prop_t *prop, const char *name, hid_t type, void *buffer);
int ss_prop_get_i(ss_prop_t *prop, const char *name);
size_t ss_prop_get_u(ss_prop_t *prop, const char *name);
double ss_prop_get_f(ss_prop_t *prop, const char *name);
void *ss_prop_buffer(ss_prop_t *prop, const char *name);
hid_t ss_prop_type(ss_prop_t *prop, const char *name);
htri_t ss_prop_appendable(ss_prop_t *prop, htri_t new_value);
htri_t ss_prop_modifiable(ss_prop_t *prop, htri_t new_value);
herr_t ss_prop_immutable(ss_prop_t *prop);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSPROP_H*/
