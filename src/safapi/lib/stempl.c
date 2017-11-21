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

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	State Templates
 *
 * Description: A state template is a pattern for what types of fields can be grouped into a state.  This pattern is specified by
 *              a list of field templates.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	State Templates
 * Purpose:	Declare a state template
 *
 * Description:	This creates a state template associated with a specified suite.  
 *
 * Return:      Returns a pointer to the new state template on success; null on failure. If the caller supplies a non-null
 *              STMPL argument then this pointer will be the return value, otherwise a state template link will be allocated.
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_StateTmpl *
saf_declare_state_tmpl(SAF_ParMode   pmode,	/* The parallel mode. */
		       SAF_Db        *database,
		       const char    *name,	/* The name of the state template. */
		       int           num_ftmpls,/* Number of field templates that will comprise this state template. */
		       SAF_FieldTmpl *ftmpls,	/* Array of field template handles. */
		       SAF_StateTmpl *stmpl	/* The returned state template handle. */
		       )
{
  SAF_ENTER(saf_declare_state_tmpl, NULL);

  stmpl = saf_declare_field_tmpl(pmode, database, name, SAF_ALGTYPE_FIELD, SAF_ANY_BASIS, SAF_NOT_APPLICABLE_QUANTITY, num_ftmpls,
                                 ftmpls, stmpl);

  SAF_LEAVE(stmpl);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	State Templates
 * Purpose:	Get a description of a state template
 *
 * Description:	Returns a description of a state template.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_describe_state_tmpl(SAF_ParMode pmode,		/* The parallel mode. */
                        SAF_StateTmpl *stmpl,		/* The state template handle. */
			char **name,			/* OUT: The returned name.  Pass NULL if you do not want the name
                                                         * returned. */                                              
			int *num_ftmpls,		/* OUT: The returned number of field templates which comprise this state 
                                                         * template. */
			SAF_FieldTmpl **ftmpls		/* OUT: The returned field templates. */
			)
{
  SAF_ENTER(saf_describe_state_tmpl, SAF_PRECONDITION_ERROR);

  saf_describe_field_tmpl (pmode, stmpl, name, NULL, NULL, NULL, num_ftmpls, ftmpls); 


  SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	State Templates
 * Purpose:	Find a state template
 *
 * Description:	Find state templates in a suite.
 *
 * Return:	The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *		an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Larry Schoof, SNL, 2000-10-09
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_find_state_tmpl(SAF_ParMode   pmode,	/* The parallel mode. */
                    SAF_Db *database,           /* the database context for this search */
                    const char    *name,	/* The name of the state template you are searching for.  Pass SAF_ANY_NAME if you
                                                 * do not wish to limit your search to just this name. */
		    int           *num_stmpls,	/* For this and the succeeding argument [see Returned Handles]. */
		    SAF_StateTmpl **stmpls	/* For this and the preceding argument [see Returned Handles]. */
		    )
{
  SAF_ENTER(saf_find_state_tmpl, SAF_PRECONDITION_ERROR);

  saf_find_field_tmpls (pmode, database, name, SAF_ALGTYPE_ANY, SAF_ANY_BASIS, SAF_ANY_QUANTITY, 
                        num_stmpls, stmpls);

  SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	State Templates	
 * Purpose:    	Attach an attribute to a state template
 *
 * Description: This function is identical to the generic saf_put_attribute() function except that it is specific to
 *    		SAF_StateTmpl objects to provide the client with compile time type checking. For a description,
 *		see saf_put_attribute().
 *
 * Programmer:  Mark Miller, LLNL, 03/10/00
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_put_state_tmpl_att(SAF_ParMode pmode, SAF_StateTmpl *stmpl, const char *att_key, hid_t att_type, int count, const void *value)
{
  SAF_ENTER(saf_put_state_tmpl_att, SAF_PRECONDITION_ERROR);
  int retval = saf_put_attribute(pmode, (ss_pers_t*)stmpl, att_key, att_type, count, value);
  SAF_LEAVE(retval);
}



/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	State Templates	
 * Purpose:    	Get an attribute attached to a state template
 *
 * Description: This function is identical to the generic saf_get_attribute() function except that it is specific to
 *    		SAF_StateTmpl objects to provide the client with compile time type checking. For a description,
 *		see saf_get_attribute().
 *
 * Programmer:  Mark Miller, LLNL, 03/10/00
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_state_tmpl_att(SAF_ParMode pmode, SAF_StateTmpl *stmpl, const char *att_key, hid_t *att_type, int *count, void **value)
{
  SAF_ENTER(saf_get_state_tmpl_att, SAF_PRECONDITION_ERROR);
  int retval = saf_get_attribute(pmode, (ss_pers_t*)stmpl, att_key, att_type, count, value);
  SAF_LEAVE(retval);
}

