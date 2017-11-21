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
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h> /*for getpid() on WIN32*/
#endif

jmp_buf *_saf_place;
int _saf_place_max=0, _saf_place_cur=-1, _saf_except;
static int caught;
static char error_buffer[1024];
static int error_buffer_ptr;

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling  
 * Description: SAF can be used either in an exception-catching programming paradigm or test the return code programming paradigm.
 *		SAF will either throw exceptions or return error codes depending on a library property
 *		(see saf_setProps_ErrorMode()).
 *
 *		See [Environment] section where environment variables affecting error checking, etc. are discussed.
 *
 *		In addition, the client can control if and how error messages are reported and whether certain kinds of
 *		errors are detected. In every function, SAF does work to detect problematic conditions. However, the cost
 *		of this detection work is weighted relative to the real work of the function using a low, medium and
 *		high weighting scheme.
 *
 *		There are several macros available to use in an exception handling programming style rather than a 
 *              test the return value style. The basic structure of an exception handling style is...
 *
 *                  SAF_TRY_BEGIN {      // begin an exception/catch block
 *                      ...              // put your saf calls here
 *                  } SAF_CATCH {        // begin the catch block
 *                      // catch a particular error
 *                      SAF_CATCH_ERR(SAF_WRITE_ERROR) {
 *                          ...          // specific error handling here
 *                      }
 *                      SAF_CATCH_ALL {  // catch any error here
 *                          ...          // generic error handling here
 *                      }
 *                  } SAF_TRY_END;       // end the exception/catch block
 *
 */

/*-----------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:     Error Handling
 * Purpose:     Throw an exception
 *
 * Description: If a try block is active, this function jumps back to the
 *              corresponding catch block. Otherwise this function terminates
 *              execution of the program.
 *
 * Modifications:
 *              Robb Matzke, LLNL, 2000-02-17
 *              Added documentation.
 *
 *              Bill Arrighi, LLNL, 2000-08-21
 *              Modified for new error handling mechanism.  No longer has an
 *              error code arg.  This was simply a non-zero value passed to
 *              longjmp.  A non-zero constant is just fine for this.  No longer
 *              needs to call saf_error_print()--handled by saf_error.  If
 *              no catch block has been defined, revert to RETURN mode by
 *              simply returning to saf_error which called this function.
 *              Previous behavior was to abort.
 *
 *              Robb Matzke, LLNL, 2001-09-26
 *              Restored previous abort behavior that was removed by Bill.
 *
 *              Robb Matzke, LLNL, 2001-11-09
 *              Restored previous ability to throw any non-zero integer value that was removed by Bill.
 *-----------------------------------------------------------------------------
 */
void
_saf_throw(int throwval)
{
  caught = 0;

  if (_saf_place_cur >= 0)
      longjmp(_saf_place[_saf_place_cur], throwval);
  fprintf(stderr, "no currently active SAF_TRY block. Aborting...\n");
  abort();
}

/*-----------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:     Error Handling
 * Purpose:     Invoke error handling mechanism
 *
 * Description: This function will call saf_error_print and, depending on the
 *              error mode of the library (set in the saf_init() call) throw an
 *              exception or return.
 *
 * Return:      None.
 *
 * Modifications:
 *              Robb Matzke, LLNL, 2000-02-17
 *              Added documentation.
 *
 *              Bill Arrighi, LLNL, 2000-08-21
 *              Modified for new error handling mechanism.  No longer returns
 *              an int, return void now.  No longer needs input args.
 *
 *              Bill Arrighi, LLNL, 2000-09-01
 *              Added !_SAF_InitDone clause to protect from error being
 *              generated by _saf_rank call which would recur infinitely.
 *
 *              Jim Reus, 21Dec2000
 *              Removed call to _saf_rank as it still ended up in a
 *              resursion loop. The new code should perform the same
 *              task but inline.
 *
 *              Robb Matzke, LLNL, 2001-11-09
 *              Re-added the integer argument to pass to _saf_throw().
 *-----------------------------------------------------------------------------
 */
void
_saf_error(int throwval)
{
  /* If the library has not been initialized we had better print the message
   * no matter which processor we are.  A call to _saf_rank when the library is
   * uninitialized generates an error itself which would lead to an infinite
   * recursion.
   */
  if (_SAF_GLOBALS.RaiseSigStopOnError)
  {
     fprintf(stderr,"processor %d of %d [pid=%d] stopped with the following error...\n", _SAF_GLOBALS.Rank, _SAF_GLOBALS.Size,
	getpid());
     _saf_error_print();
     fprintf(stderr,"waiting for debugger to attach\n");

#ifdef WIN32
     raise(SIGABRT);
#else
     raise(SIGSTOP);
#endif

  }
  else
  {
     if ((!_SAF_GLOBALS.DontReportErrors) && (!_SAF_InitDone || _SAF_GLOBALS.p.ErrorLogMode != SAF_ERRLOG_NONE))
        _saf_error_print();
  }


   /* Issue: In parallel,
      we *have* to call abort. Why? Because each processor is engaged
      in a series of operations involving, potentially, collective
      communication. If one processor jumps out of this collective game early,
      none of the others can know about it. We either hang, which is *really* bad,
      or we can abort with error, which is bad but better than hanging with no
      information about why.
    */
#ifdef HAVE_PARALLEL
  if (_SAF_GLOBALS.p.DoAbort) {
      ss_zap(); /*because MPICH might call exit(), thereby invoking SSlib's atexit() handler*/
      MPI_Abort(_SAF_GLOBALS.p.LibComm,SAF_FATAL_ERROR);
  }
#endif

  if (_SAF_GLOBALS.p.ErrorMode == SAF_ERRMODE_THROW)
    _saf_throw(throwval);
  return;
}
  
/*-----------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:     Error Handling
 * Purpose:     Print an error message
 *
 * Description: saf_error_print() prints the most recent error message.
 *
 * Modifications:
 *              Robb Matzke, LLNL, 2000-02-17
 *              Added documentation.
 *
 *              Jim Reus, 30Nov2000
 *              Cleaned-up a unused variable warning.
 *
 *-----------------------------------------------------------------------------
 */
void
_saf_error_print(void)
{
   int off = (int)strlen(error_buffer);
   char tmp = '\0';

   if (off >= (int)sizeof(error_buffer) - 1)
   {
      off = sizeof(error_buffer) - 2;
      tmp = error_buffer[off];
   }

#ifdef HAVE_PARALLEL
   /* for segmented file mode, we need to seek to correct offset in file */
   if (_SAF_GLOBALS.p.ErrorLogMode == SAF_ERRLOG_SEGFILE)
   {
      static int call_count = 0;
      int rows  = _SAF_GLOBALS.p.ErrorLogRows;
      int cols  = _SAF_GLOBALS.p.ErrorLogCols;
      int rank  = _SAF_GLOBALS.Rank;
      long seek = rows * cols * rank + (call_count % rows) * cols;

      if (off >= cols - 1)
      {
         off = cols - 2;
         tmp = error_buffer[off];
      }
      error_buffer[off]   = '\n';
      error_buffer[off+1] = '\0';

      if (_SAF_GLOBALS.ErrorLogFile)
          fseek(_SAF_GLOBALS.ErrorLogFile, seek, SEEK_SET);
      fwrite(error_buffer, (unsigned int)cols, 1, _SAF_GLOBALS.ErrorLogFile?_SAF_GLOBALS.ErrorLogFile:stderr);
      /*
      fprintf(_SAF_GLOBALS.ErrorLogFile, "%.*s\n", cols-1, error_buffer);
      */
      call_count++;
   }
   else
   {
      error_buffer[off] = '\n';
      error_buffer[off+1] = '\0';
      fprintf(_SAF_GLOBALS.ErrorLogFile?_SAF_GLOBALS.ErrorLogFile:stderr, "%s", error_buffer);
   }
#else
   fprintf(_SAF_GLOBALS.ErrorLogFile?_SAF_GLOBALS.ErrorLogFile:stderr, "%s\n", error_buffer);
#endif

   if (tmp != '\0')
      error_buffer[off] = tmp;

   fflush(_SAF_GLOBALS.ErrorLogFile?_SAF_GLOBALS.ErrorLogFile:stderr);

   return;
}

/*-----------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Error Handling
 * Purpose:     Fill the error message buffer with location
 *
 * Description: _saf_generic_errmsg() adds name of the source file, name of the
 *              function and line number for the most recent error.
 *
 * Modifications:
 *              Bill Arrighi, LLNL, 2000-08-21
 *              Created.
 *-----------------------------------------------------------------------------
 */
void
_saf_generic_errmsg(const char *file,   /* The name of the source file from which this
                                         * function is called. */
                    const char *func,   /* The name of the function from which this
                                         * function is called. */
                    int line            /* The line number from which this function is
                                         * called. */
                    )
{
#ifdef HAVE_PARALLEL
  /* Issue: We use the rank of the processor in lib communicator (SAF_NOT_SET_DB) because we don't have the
     database handle here. We probably ought to provide for an error stream on a per-database basis. Thats
     for later. */
  error_buffer_ptr = sprintf(error_buffer, "%03d:%s[%s<%d>]: ", _SAF_GLOBALS.Rank, file, func, line);
#else
  error_buffer_ptr = sprintf(error_buffer, "%s[%s<%d>]: ", file, func, line);
#endif
  return;
}

/*-----------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:     Error Handling
 * Purpose:     Fill the error message buffer with message
 *
 * Description: _saf_errmsg() adds an error message and any supplied diagnostics
 *              for the most recent error.
 *
 * Modifications:
 *              Bill Arrighi, LLNL, 2000-08-21
 *              Created.
 *-----------------------------------------------------------------------------
 */
void
_saf_errmsg(const char *format, /* A printf-like error message. */
            ...                 /* Variable length debugging info specified by
                                 * format to be printed out. */
            )
{
  va_list ptr;

  va_start(ptr, format);
#ifdef HAVE_VSNPRINTF
  vsnprintf(error_buffer + error_buffer_ptr, sizeof(error_buffer) - error_buffer_ptr - 1, format, ptr);
#else
  {  size_t L,Lmax;
     char   tmp[sizeof(error_buffer)];

     vsprintf(tmp, format, ptr);
     L    = strlen(tmp);
     Lmax = sizeof(error_buffer) - error_buffer_ptr - 1;
     if (Lmax < L)
        tmp[Lmax-1] = '\0';
     strcpy(error_buffer + error_buffer_ptr,tmp);
  }
#endif
  va_end(ptr);

  return;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Return a pointer to an error string
 *
 * Description: saf_error_str() returns a pointer to a string containing the most recent error message.
 *
 * Modifications:
 *              Robb Matzke, LLNL, 2000-02-17
 *              Added documentation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
char *
saf_error_str(void)
{
  return error_buffer;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:     Error Handling
 * Purpose:     Uknown
 *
 * Description: I think this checks to see if an error has been caught. Hold-over from the 0.8 driver.
 *
 * Modifications:
 *              Mark Miller, LLNL, 2000-04-20
 *              Added documentation.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_check_catch(void)
{
  if (caught == 0)
  {
    caught = 1;
    return(1);
  }
  else
    return(0);
}
