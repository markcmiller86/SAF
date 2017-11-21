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
/**/
#ifndef WRAPPER_H_INCLUDED
#define WRAPPER_H_INCLUDED

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Wrappers
 *              
 * Description: The macros defined here are intended to be wrappers around various parts of saf functions.  Every saf function
 *              (with limited exceptions) will invoke SAF_ENTER() with the function name and error return value as the first
 *              line of the function and SAF_LEAVE() with a return value as the last action of the function.  No function may
 *              jump out of (via return, goto, or longjmp) the code enclosed by SAF_ENTER/LEAVE except by approved methods
 *              noted below.
 *
 *              A function has the following format:
 *
 *                  return_type
 *                  func_name(argument_list)
 *                  {
 *                      SAF_ENTER(func_name, error_return_value);          // or
 *                      SAF_ENTER_NOINIT(func_name, error_return_value);   // or
 *                      SAF_ENTER_GRABBED(func_name, error_return_value);
 *                      ...
 *                      SAF_REQUIRE(condition, cost, return_value, message_function); // optional
 *                      SAF_ASSERT(condition, cost, return_value, message_function);  // optional
 *                      ...
 *                      SAF_RETURN(return_value);  // early return, optional
 *                      ...
 *                      SAF_ENSURE(condition, cost, return_value, message_function); // optional
 *                      SAF_LEAVE(return_value);   // must be last
 *                  }
 *-------------------------------------------------------------------------------------------------------------------------------
 */

extern int _saf_skip_init_g,  _saf_check_grabbed_g;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Wrappers
 * Purpose:     Determines scope of saf function
 *              
 * Description: Returns true if the specified function is a non-API function based on its name. All non-API (private)
 *              functions start with an underscore.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_IS_PRIVATE(NAME) (*NAME=='_')

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Wrappers
 * Purpose:     Notes function entrance
 *              
 * Description: Every function that is entered should call this macro with the function name. There are a very limited number
 *              of functions that omit the ENTER/LEAVE macros and they are commented.
 *
 * Also:        SAF_ENTER_NOINIT SAF_ENTER_GRABBED
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ENTER(FUNCNAME,RETVAL) {                                                                                           \
    static int _call_count = -1;                                                                                               \
    const char *_entrance;                                                                                                     \
                                                                                                                               \
    /* Profiling */                                                                                                            \
    _entrance = #FUNCNAME;                                                                                                     \
                                                                                                                               \
    /* Check initialized, not grabbed */                                                                                       \
    if (!SAF_IS_PRIVATE(_entrance) && !_SAF_InitDone && !_saf_skip_init_g) {                                                   \
        SAF_NR_ERROR(SAF_PRECONDITION_ERROR, _saf_errmsg("the library must be initialized"));                                  \
        return RETVAL;                                                                                                         \
    }                                                                                                                          \
    if (_saf_check_grabbed_g) {                                                                                                \
        if (!_SAF_GLOBALS.Grabbed) {                                                                                           \
            SAF_NR_ERROR(SAF_PRECONDITION_ERROR, _saf_errmsg("the library must be in a grabbed state"));                       \
            return RETVAL;                                                                                                     \
        }                                                                                                                      \
    } else if (_SAF_GLOBALS.Grabbed) {                                                                                         \
        SAF_NR_ERROR(SAF_PRECONDITION_ERROR, _saf_errmsg("the library must not be in a grabbed state"));                       \
        return RETVAL;                                                                                                         \
    }                                                                                                                          \
                                                                                                                               \
    /* Function tracing */                                                                                                     \
    _call_count++;                                                                                                             \
    if (SAF_ERRLOG_PROCFILE == _SAF_GLOBALS.p.ErrorLogMode ||                                                                  \
        (SAF_ERRLOG_PROCFILE != _SAF_GLOBALS.p.ErrorLogMode && 0 == _SAF_GLOBALS.Rank)) {                                      \
        switch (_SAF_GLOBALS.p.TraceLogMode) {                                                                                 \
        case SAF_TRACELOG_NONE:                                                                                                \
            break;                                                                                                             \
        case SAF_TRACELOG_PUBLIC:                                                                                              \
            if (SAF_IS_PRIVATE(_entrance)) break;                                                                              \
            /*fall through*/                                                                                                   \
        case SAF_TRACELOG_PRIVATE:                                                                                             \
            if (_SAF_GLOBALS.p.TraceTimes) {                                                                                   \
                fprintf(_SAF_GLOBALS.ErrorLogFile, "entering %s [%04d call @ %6.2f vc-secs]\n",                                \
                        _entrance, _call_count, _saf_wall_clock(false));                                                       \
            } else {                                                                                                           \
                fprintf(_SAF_GLOBALS.ErrorLogFile, "entering %s\n", _entrance);                                                \
            }                                                                                                                  \
            break;                                                                                                             \
        default:                                                                                                               \
            break;                                                                                                             \
        }                                                                                                                      \
    }                                                                                                                          \
                                                                                                                               \
    /* New scope for function body */                                                                                          \
    {                                                                                                                          \
        const char *me = _entrance /*need a declaration here because semicolon appears next*/

    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Wrappers
 * Purpose:     Notes function entrance without init
 *              
 * Description: Some functions (e.g., library property functions, saf_init, etc) don't require that the library be initialized
 *              before the function is entered.  This macro can be used in place of FUNC_ENTER() to prevent that check.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ENTER_NOINIT(FUNCNAME,RETVAL)                                                                                       \
    int _saf_skip_init_g=1; /*hides global with same name*/                                                                     \
    SAF_ENTER(FUNCNAME, RETVAL)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Wrappers
 * Purpose:     Notes function entrance without checking for grabbed state
 *              
 * Description: Some functions (e.g., ungrab functions) allow, require the library to be in a grabbed state instead of the
 *              usual ungrabbed state. This macro is identical to SAF_ENTER() except it checks that the library is in a
 *              grabbed state instead of ungrabbed state.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ENTER_GRABBED(FUNCNAME,RETVAL)                                                                                     \
    int _saf_check_grabbed_g=1; /*hides global with same name*/                                                                \
    SAF_ENTER(FUNCNAME, RETVAL)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Wrappers
 * Purpose:     Early return from function
 *              
 * Description: This macro can be used to return early from the body of a SAF_ENTER() / SAF_LEAVE() construct.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_RETURN(RETVAL) {                                                                                                   \
        _entrance = me; /*to shut up compiler warning about unused `me'*/                                                      \
        return RETVAL;                                                                                                         \
}
 
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Wrappers
 * Purpose:     Function exit
 *              
 * Description: This macro is the counterpart to SAF_ENTER(). All functions that begin with SAF_ENTER() must end with
 *              SAF_LEAVE().
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_LEAVE(RETVAL)                                                                                                      \
        SAF_RETURN(RETVAL);                                                                                                    \
    } /*end scope from end of SAF_ENTER*/                                                                                      \
} /*end scope from beginning of SAF_ENTER*/

    
/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Report error
 *
 * Description: Report an error via saf_error and if using SAF_ERRMODE_RETURN, return indicated return value. Whenever an
 *              error code is returned to a calling SAF routine this macro is invoked to log the error and enforce the return
 *              policy in effect.
 *
 * Modifications:
 *              Bill Arrighi, LLNL, 2000-09-05
 *              Eliminate do-while construction since SUN compiler generates warnings about end-of-loop code not being
 *              reached.  NOTE: use of this simpler construction makes it necessary to use some care when using SAF_ERROR in
 *              an if-else.  If used as the if clause in an if-else then the terminating ";" must be omitted or the if
 *              conditional must be inverted so SAF_ERROR is the else clause. (Or always use curly braces with "if" statements
 *              that have "else" clauses --rpm).
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ERROR(RETVAL,MSGFCN) {                                                                                              \
    SAF_NR_ERROR(RETVAL,MSGFCN);                                                                                                \
    SAF_RETURN(RETVAL);                                                                                                         \
}

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Report error
 *
 * Description: Report an error via saf_error and if using SAF_ERRMODE_RETURN, return indicated return value. Whenever an
 *              error code is returned to a calling SAF routine this macro is invoked to log the error and enforce the return
 *              policy in effect.
 *
 * Modifications:
 *              Bill Arrighi, LLNL, 2000-09-05
 *              Eliminate do-while construction since SUN compiler generates warnings about end-of-loop code not being
 *              reached.  NOTE: use of this simpler construction makes it necessary to use some care when using SAF_ERROR in
 *              an if-else.  If used as the if clause in an if-else then the terminating ';' must be omitted
 *              or the if conditional must be inverted so SAF_ERROR is the else clause.
 *
 *              Robb Matzke, LLNL, 2001-11-09
 *              Added the ThrowVal argument, which is usually the same thing as the RetVal argument of the SAF_ERROR() macro.
 *              Thus, we must cast it to an int because the return value is sometimes the null pointer. The underlying longjmp()
 *              will get zero but the corresponding setjmp() will return one.
 *
 *		Mark Miller, LLNL, 2001-11-20
 *		Added the '+0' to ThrowVal to deal with functions returning void 
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_NR_ERROR(THROWVAL,MSGFCN) {                                                                                        \
    _saf_generic_errmsg(__FILE__, _entrance, __LINE__);                                                                        \
    MSGFCN;                                                                                                                    \
    _saf_error((int)THROWVAL+0);                                                                                               \
}

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Check for validity of logical assertions
 *
 * Description: The following macro is written using a strange "do-while" construct to achieve the desired behavior in the
 *              caller with semicolon termination.
 *
 * Modifications:
 *              Robb Matzke, LLNL, 2000-01-03
 *              If the error message, MSG, is null then the string printed to the standard output stream will contain the actual
 *              expression, E, instead.  This is because some expressions are just too difficult to explain in enough detail in
 *              English, or they are so simple that no English documentation is required.
 *              
 *              Bill Arrighi, LLNL, 2000-08-21
 *              Modified for new error handling mechanism.  Now uses standard saf_error error handling so no distinction between
 *              serial and parallel versions of this macro are necessary. Control over execution of assertions is now purely at
 *              run-time.  Arguments are now different--added Cost of evaluation of assertion, value to be returned by function
 *              macro resides in should assertion be false and message function to construct the error message specific to this
 *              assertion.
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ASSERT(Assertion, Cost, RetVal, MsgFcn) do {                                                                       \
    if ((Cost) <= _SAF_GLOBALS.AssertDisableCost) {                                                                            \
        if (!(Assertion)) {                                                                                                    \
            _saf_generic_errmsg(__FILE__, _entrance, __LINE__);                                                                \
            MsgFcn;                                                                                                            \
            _saf_error((int)RetVal+0);                                                                                         \
            return RetVal;                                                                                                     \
        }                                                                                                                      \
    }                                                                                                                          \
} while(false)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Begin assertion validation code block
 *
 * Description: Not all assertions can be stated in terms of simple boolean expressions that can be passed as the
 * 		"Assertion" arg	of SAF_ASSERT. Some assertions involve a chunk of code. SAF_ASSERT_BEGIN together
 *		with SAF_ASSERT_END are used in the latter case.
 *
 *              Somewhere between the SAF_ASSERT_BEGIN() and SAF_ASSERT_END() there should be an assignment to the variable
 *              `ok'.  The SAF_ASSERT_END() macro will fail if `ok' is still false (the default) at that point.
 *
 * Example:     Here's how this construct could be used.
 *                  // Check buffer pointers if buf is non-null.
 *                  SAF_ASSERT_BEGIN(SAF_LOW_CHK_COST) {
 *                      int i;
 *                      ok = TRUE; // assume true, prove otherwise
 *                      if (bufs && nbufs>1) {
 *                          for (i=0; i<nbufs && ok; i++) {
 *                              if (!bufs[i]) ok = FALSE;
 *                          }
 *                      }
 *                  } SAF_ASSERT_END(NULL, saf_errmsg("BUFS must point to NBUFS valid (i.e., non-null) pointers"));
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ASSERT_BEGIN(Cost)                                                                                                 \
    do {                                                                                                                       \
        if ((Cost) <= _SAF_GLOBALS.AssertDisableCost) {                                                                        \
            hbool_t ok=FALSE;

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Ends a code block to validate an assertion
 *
 * Description: See SAF_ASSERT_BEGIN().
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ASSERT_END(RetVal,MsgFcn)                                                                                          \
            if (!ok) {                                                                                                         \
                _saf_generic_errmsg(__FILE__, _entrance, __LINE__);                                                            \
                MsgFcn;                                                                                                        \
                _saf_error((int)(RetVal)+0);                                                                                   \
                return (RetVal);                                                                                               \
            }                                                                                                                  \
        }                                                                                                                      \
    } while (false)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Check for validity of function precondition
 *
 * Description: The following macro is written using a strange "do-while" construct to achieve the desired behavior in the
 *              caller with semicolon termination.  Check precondition if cost does not exceed PreCondDisableCost.  If
 *              precondition is not true call saf_error to handle resulting error.
 *
 * Programmer:  Bill Arrighi,  LLNL
 *
 * Modifications:
 *      Robb Matzke, LLNL, 2001-11-09
 *      Throws SAF_PRECONDITION_ERROR
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_REQUIRE(PreCond,Cost,RetVal,MsgFcn) do {                                                                           \
    if ((Cost) <= _SAF_GLOBALS.PreCondDisableCost && !(PreCond)) {                                                             \
        _saf_generic_errmsg(__FILE__, _entrance, __LINE__);                                                                    \
        MsgFcn;                                                                                                                \
        _saf_error(SAF_PRECONDITION_ERROR);                                                                                    \
        return RetVal;                                                                                                         \
    }                                                                                                                          \
} while(false)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Begins a code block to validate an pre-condition
 *
 * Description: Not all preconditions can be stated in terms of simple boolean expressions that can be passed as the
 * 		"PreCond" of SAF_BEGIN. Some pre-conditions involve a chunk of code. SAF_REQUIRE_BEGIN together
 *		with SAF_REQUIRE_END are used in the latter case.
 *
 *		Note: both these macros include the encompassing '{' and '}' to bracket the code. Thus, no
 *		semicolons are used after either.
 *
 * Modifications:	Mark Miller, 2000-10-12
 *			Initial implimentation
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_REQUIRE_BEGIN(Cost)                                                                                                 \
     if ((Cost) <= _SAF_GLOBALS.PreCondDisableCost) {

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Ends a code block to validate an pre-condition
 *
 * Description: Not all preconditions can be stated in terms of simple boolean expressions that can be passed as the
 * 		"PreCond" of SAF_BEGIN. Some pre-conditions involve a chunk of code. SAF_REQUIRE_BEGIN together
 *		with SAF_REQUIRE_END are used in the latter case.
 *
 *		Note: both these macros include the encompassing '{' and '}' to bracket the code. Thus, no
 *		semicolons are used after either.
 *
 * Programmer:  Mark Miller, LLNL, 2000-10-12
 *
 * Modification:
 *      Robb Matzke, LLNL, 2001-11-09
 *      Throws SAF_PRECONDITION_ERROR
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_REQUIRE_END(PreCond, RetVal, MsgFcn)                                                                               \
         if (!(PreCond)) {                                                                                                     \
             _saf_generic_errmsg(__FILE__, _entrance, __LINE__);                                                               \
             MsgFcn;                                                                                                           \
             _saf_error(SAF_PRECONDITION_ERROR);                                                                               \
             return RetVal;                                                                                                    \
         }                                                                                                                     \
     } /*end if from SAF_REQUIRE_BEGIN*/

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Check for validity of function postcondition
 *
 * Description: The following macro is written using a strange "do-while" construct to achieve the desired behavior in the
 *              caller with semicolon termination.  Check postcondition if cost does not exceed PostCondDisableCost.  If
 *              postcondition is not true call saf_error to handle resulting error.
 *
 * Programmer:  Bill Arrighi, LLNL, 2000-08-21
 *
 * Modification:
 *      Robb Matzke, LLNL, 2001-11-09
 *      Throws SAF_POSTCONDITION_ERROR
 *--------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ENSURE(PostCond, Cost, RetVal, MsgFcn) do {                                                                        \
    if ((Cost) <= _SAF_GLOBALS.PostCondDisableCost) {                                                                          \
        if (!(PostCond)) {                                                                                                     \
            _saf_generic_errmsg(__FILE__, _entrance, __LINE__);                                                                \
            MsgFcn;                                                                                                            \
            _saf_error(SAF_POSTCONDITION_ERROR);                                                                               \
            return RetVal;                                                                                                     \
        }                                                                                                                      \
    }                                                                                                                          \
} while(false)

#endif /*WRAPPER_H_INCLUDED*/
