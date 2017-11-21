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
 * Note:	Properties
 * Description: A /property/ abstraction is used to control various behaviors of the library and its interactions with databases,
 *		supplemental files and other things. A specific set of properties is constructed *prior* to a scope-beginning
 *		call in which those properties are applied. For example, since library properties effect the behavior of the
 *		library, they are applied in the saf_init() call. Likewise, database properties are applied in the
 *		saf_open_database() call. Once applied, the properties remain in effect until a corresponding scope-ending call.
 *		For example, saf_final() for library properties or saf_close_database() for database properties.
 *		You cannot change properties mid-stream. If this is a desired modality, please contact
 *		mailto:saf-help@sourceforge.sandia.gov with such a request.
 *
 *		The general way to control properties is to build up the desired property set of properties by first calling a
 *		saf_createProps_xxx() function where 'xxx' is, for example, 'lib' or 'database'. This creates a default set of
 *		properties. You can then adjust specific properties in this set by calling individual functions described in the
 *		properties chapters of the API. The resultant set of properties is applied when they are passed in a
 *		scope-beginning call such as saf_init() or saf_open_database(). See descriptions of the individual member property
 *		functions for a description of the properties supported.
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Description:	There are a number of properties that affect the behavior of the library. For a general description of how
 *		properties are used (See Properties).
 *
 *		The functions to set library properties are the only functions that may be called prior to calling saf_init().
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/* Prototypes for file-scope functions */
static herr_t _saf_set_error_logging(const char *env_var_name, SAF_LibProps *p);
static void _saf_set_tracing(const char *env_var_name, SAF_LibProps *p);

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:     Create a new library property list with default values
 *
 * Description: This function creates a library property list which can be passed to the saf_init() function. All properties
 *              in this list are set to their default values...
 *
 *		   ErrFunc = NULL;
 *
 *		   ErrMsgMode = <ignored>;
 *
 *		   ErrLoggingMode = none;
 *
 *		   ErrorMode = SAF_ERRMODE_RETURN;
 *
 *		   LibComm = MPI_COMM_WORLD;
 *
 *		   StrMode = SAF_STRMODE_LIB;
 *
 *		   StrPoolSize = 4096;
 *
 * Return:      A handle to a new library properties list initialized to default values. NULL on failure or an exception is
 *              thrown depending on the error handling library property currently in effect (See Properties).
 *
 * Parallel:    This function must be called collectively by all processors in MPI_COMM_WORLD.
 *
 * Issue:	Since this function is called before saf_init(), the only communicator we can use to work correctly is
 *		MPI_COMM_WORLD. However, it we allowed the client to pass a communicator here, then we could avoid that.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_LibProps *
saf_createProps_lib(void)
{
   SAF_ENTER_NOINIT(saf_createProps_lib, NULL);

   SAF_LibProps *p = NULL;

#ifdef HAVE_PARALLEL
   {  int result;
      /* confirm MPI is initialized */
      if ((MPI_Initialized(&result) != MPI_SUCCESS) || !result)
      {
         _SAF_GLOBALS.p.DoAbort = false; /* we have to turn off call to abort here */
         SAF_ERROR(NULL, _saf_errmsg("MPI is not running"));
      }
   }
#endif

   /* allocate a property packet */
   p = calloc(1, sizeof *p);
   if (p == NULL)
      SAF_ERROR(NULL, _saf_errmsg("out of memory--unable to allocate properties object"));

   {  char *s=0;
      SAF_GETENV_WORLD(s,"SAF_ERROR_REPORTING");
      _saf_set_error_logging(s, p);
      if(s) free(s);/*cannot use _saf_free until lib is initialized*/
   }

   /* set api tracing (this call MUST COME AFTER call to setup error logging) */
   _saf_set_tracing("SAF_TRACING", p);
   
   /* some properties used primarily in testing and installation */
   p->DoAbort 		= true; /* always do the abort by default */

   /* parallel mpi communicator */
   p->LibComm = SS_COMM_WORLD;

   /* string pool size */
   p->StrPoolSize = 4096;

   SAF_LEAVE(p);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:     Free library property list
 *
 * Description: Releases resources inside the library property list and frees the property list that was allocated in
 *              saf_createProps_lib().
 *
 * Return:      Always returns null.
 *
 * Parallel:    Independent
 *
 * Issue:       Releasing the resources used by the property list was never implemented and so is not implemented here yet
 *              either.
 *
 * Programmer:  Robb Matzke
 *              Friday, November 14, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_LibProps *
saf_freeProps_lib(SAF_LibProps *properties)
{
    if (properties) free(properties);
    return NULL;
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:     Set the MPI communicator for the library
 *
 * Description: This function sets the MPI communicator in the specified library property list to COMMUNICATOR (the default is
 *              MPI_COMM_WORLD). After this property list is used to initialize the library by calling saf_init(), it will
 *              become the communicator for all collective calls. However, a database can override this communicator in the
 *              saf_open_database() call.
 *
 * Parallel:    This function can be called independently. It is not defined in a non-parallel version of the library.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Bugs:        This function sometimes returns an error instead of throwing an exception when the library error mode is
 *              SAF_ERRMODE_THROW.
 *
 * Issue:	Should this function even be defined if the library is not compiled for parallel. My reasoning is that it
 *		would only be called if the client is compiled for parallel and therefore it only makes sense to link the
 *		application if the SAF-API is also compiled for parallel. Getting a link error is probably better than a
 *		runtime error for two reasons: the error comes earlier (what if the application did a day of number crunching
 *		before trying I/O), and we can guarantee that it's an error (what if the client failed to check return values).
 *
 * Modifications:
 *              Robb Matzke, 2000-01-24
 *              Added documentation. Changed formal argument p to properties and c to communicator.
 *
 *              Robb Matzke, 2001-04-06
 *              Changed to SAF_Handle
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_LibComm(SAF_LibProps *properties,   /* The library property list which will be modified by this function
						   (See Properties). */
                     MPI_Comm communicator      /*The new MPI communicator.*/
                     )
{
    SAF_ENTER_NOINIT(saf_setProps_LibComm, SAF_PRECONDITION_ERROR);

    SAF_LibProps *p = properties;

    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must be a valid library properties handle"));

    p->LibComm = communicator;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:     Set string allocation style
 *
 * Description: By default, the library allocates memory for returned strings and the client is expected to free that memory
 *              when it is no longer needed. However, by calling saf_setProps_StrMode() the client can set library properties
 *              which change this mode of operation. Possible values are SAF_STRMODE_LIB, the default; SAF_STRMODE_CLIENT,
 *              which means that the client will always allocate space for the string return value (and free it also); and
 *              SAF_STRMODE_POOL, which means that the library will allocate space for the string return values from a
 *              recirculating pool, freeing the memory which has been least recently allocated. The saf_setProps_StrPoolSize()
 *              function can be used to change the default size of the pool. (See Returned Strings).
 *
 * Parallel:    This function can be called independently.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Modifications:
 *              Robb Matzke, 2000-01-24
 *              Added documentation. Changed formal argument p to properties.
 *
 *              Robb Matzke, 2001-04-06
 *              Converted to SAF_Handle
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_StrMode(SAF_LibProps *properties,   /* The library property list which will be modified by this function
						   (See Properties). */
                     SAF_StrMode mode           /* The string allocation mode, one of SAF_STRMODE_LIB, SAF_STRMODE_CLIENT, or
                                                 * SAF_STRMODE_POOL. */                         
                     )
{
    SAF_ENTER_NOINIT(saf_setProps_StrMode, SAF_PRECONDITION_ERROR);
    
    SAF_LibProps *p = properties;

    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must be a valid library properties handle"));

    p->StrMode = mode;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:     Set the pool size for string return value allocations
 *
 * Description: If the library string mode property is SAF_STRMODE_POOL then string return values are allocated by the library
 *              from a pool with some number of entries (4096 by default). When all entries of the pool have been allocated for
 *              return values then the library begins freeing the least recently allocated entry.
 *
 * Parallel:    This function can be called independently.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Bugs:        This function sometimes returns an error instead of throwing an exception when the library error mode is
 *              SAF_ERRMODE_THROW.
 *
 *              The pool and its entries are not freed by saf_final().
 *
 * Modifications:
 *              Robb Matzke, 2000-01-24
 *              Added documentation. Changed formal argument p to properties
 *
 *              Robb Matzke, 2001-04-06
 *              Converted to SAF_Handle
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_StrPoolSize(SAF_LibProps *properties,       /* The library property list which will be modified by this function (See
							   Properties). */
                         int size                       /*The new pool size.*/
                         )
{
    SAF_ENTER_NOINIT(saf_setProps_StrPoolSize, SAF_PRECONDITION_ERROR);
    
    SAF_LibProps *p = properties;

    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must be a valid library properties handle"));

    p->StrPoolSize = size;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:     Set the library error handling mode
 *
 * Description: The library normally handles error conditions by causing the erring function to return a non-zero error
 *              number. However, the error mode can be set to SAF_ERRMODE_THROW which causes the erring function to throw an
 *              exception instead.
 *
 * Parallel:    This function can be called independently.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Bugs:        This function sometimes returns an error instead of throwing an exception when the library error mode is
 *              SAF_ERRMODE_THROW.
 *
 * Modifications:
 *              Robb Matzke, 2000-01-24
 *              Added documentation. Changed formal argument p to properties
 *
 *              Robb Matzke, 2001-04-06
 *              Converted to SAF_Handle
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_ErrorMode(SAF_LibProps *properties, /* The library property list which will be modified by this function
						   (See Properties). */
                       SAF_ErrMode mode         /* The new error handling mode. Valid values are SAF_ERRMODE_RETURN (the
                                                 * default) and SAF_ERRMODE_THROW. */           
                       )
{
    SAF_ENTER_NOINIT(saf_setProps_ErrorMode, SAF_PRECONDITION_ERROR);
    
    SAF_LibProps *p = properties;

    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must be a valid library properties handle"));

    p->ErrorMode = mode;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:     Specify a callback for error conditions
 *
 * Description: The specified function (or no function if FUNC is the null pointer) will be called when the library is
 *              recovering from an error condition. The default is that no callback is invoked.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issues:      Not implemented yet (always returns SAF_NOTIMPL_ERROR).
 *
 * Programmer:  Robb Matzke (just the stub)
 *              Monday, February 21, 2000
 *
 * Modifications:
 *              Robb Matzke, 2001-04-06
 *              Converted to SAF_Handle
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_ErrFunc(SAF_LibProps *properties,   /* The library property list which will be modified by this function
						   (See Properties). */
                     SAF_ErrFunc func           /* The callback to invoke when an error occurs. */
                     )
{
    SAF_ENTER_NOINIT(saf_setProps_ErrFunc, SAF_PRECONDITION_ERROR);

    SAF_LibProps *p = properties;
    
    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must be a valid library properties handle"));

    /* Not implemented */
    if (false) SAF_ERROR(SAF_NOTIMPL_ERROR, _saf_errmsg("not implemented yet"));
    SAF_LEAVE(SAF_NOTIMPL_ERROR);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:    	Set the error logging mode
 *
 * Description: This library property controls how the library reports errors. See section on environment variables where
 *		the environment variable SAF_ERROR_REPORTING is described.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer: 	Mark C. Miller, LLNL, 2000-10-03
 *
 * Modifications:
 *              Robb Matzke, 2001-04-06
 *              Converted to SAF_Handle
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_ErrorLogging(SAF_LibProps *properties,	/* The library property list which will be modified by this function
							   (See Properties). */
                          const char *mode		/* The error logging mode. */
                          )
{
    SAF_ENTER_NOINIT(saf_setProps_ErrorLogging, SAF_PRECONDITION_ERROR);
    
    SAF_LibProps *p = properties;
    
    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must be a valid library properties handle"));
    
    _saf_set_error_logging(mode, p);
    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:    	Set error logging mode
 *
 * Description: This function is used to set the error logging mode based upon an input string.
 *		See discussion of environment variables for a description of the input string and its meaning.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static herr_t
_saf_set_error_logging(const char *s,           /* Character representation of the mode */
                       SAF_LibProps *q	        /* OUT: Properties */
                       )
{
    SAF_ENTER_NOINIT(_saf_set_error_logging, -1);
    
    char tmp[1024];
    const char *pa, *pb, *pd, *pr, *pc;
    const char *pempty="";

    /* Initialize error stuff depending on whether SAF is compiled for production or debugging */
#ifdef PRODUCTION_COMPILE
    q->ErrorLogMode	= SAF_ERRLOG_NONE;
#else
    q->ErrorLogMode	= SAF_ERRLOG_STDERR;
#endif
    q->ErrorLogName[0]	= '\0';
    q->ErrorLogRows	= 0;
    q->ErrorLogCols	= 0;
    if (!s) SAF_RETURN(0);
   
    /* make one broad check to make sure strings are big enough. The +4 is for the PROCFILE case. */
    if (strlen(s)+3>sizeof tmp) {
        q->ErrorLogMode	= SAF_ERRLOG_NONE;
        SAF_RETURN(-1);
    }
    
    if (!strcmp(s,"none")) {
        q->ErrorLogMode	= SAF_ERRLOG_NONE;
    } else if (!strcmp(s,"stderr")) {
        q->ErrorLogMode	= SAF_ERRLOG_STDERR;
    } else if (!strncmp(s,"file:",strlen("file:"))) {
        if (strlen(s+strlen("file:"))>=sizeof(q->ErrorLogName)) SAF_RETURN(-1);
        q->ErrorLogMode	= SAF_ERRLOG_FILE;
        strcpy(q->ErrorLogName, s+strlen("file:"));
    } else if (!strncmp(s,"procfile:",strlen("procfile:"))) {
        strncpy(tmp, s, sizeof(tmp));
        strtok(tmp, ":");
        if ((pa = strtok(NULL,",")) == NULL)
            pa = pempty;
        if ((pd = strtok(NULL,",")) == NULL)
            pd = pempty;
        if ((pb = strtok(NULL,",")) == NULL)
            pb = pempty;
#ifdef HAVE_PARALLEL
        q->ErrorLogMode	= SAF_ERRLOG_PROCFILE;
        sprintf(q->ErrorLogName, "%s%s%s", pa, pd, pb);
#else
        q->ErrorLogMode	= SAF_ERRLOG_FILE;
        sprintf(q->ErrorLogName, "%s%s", pa, pb);
#endif
    } else if (!strncmp(s,"segfile:",strlen("segfile:"))) {
        strncpy(tmp, s, sizeof(tmp));
        strtok(tmp, ":");
        if ((pa = strtok(NULL,",")) == NULL)
            pa = pempty;
        if ((pr = strtok(NULL,",")) == NULL)
            pr = pempty;
        if ((pc = strtok(NULL,",")) == NULL)
            pc = pempty;
        if ((pb = strtok(NULL,",")) == NULL)
            pb = pempty;
#ifdef HAVE_PARALLEL
        q->ErrorLogMode	= SAF_ERRLOG_SEGFILE;
        sprintf(q->ErrorLogName,"%s%sx%s%s",pa,pr,pc,pb);
        q->ErrorLogRows	= atoi(pr);
        q->ErrorLogCols	= atoi(pc);
#else
        q->ErrorLogMode	= SAF_ERRLOG_FILE;
        sprintf(q->ErrorLogName, "%s%s", pa, pb);
        q->ErrorLogRows	= 0;
        q->ErrorLogCols	= 0;
#endif
    } else {
        q->ErrorLogMode	= SAF_ERRLOG_STDERR;
    }

    SAF_LEAVE(-1);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:    	Set tracing mode
 *
 * Description: This function is used to set the API tracing mode based upon an input string.
 *		See discussion of environment variables for a description of the input string and its meaning.
 *
 *		This call must be done AFTER the call to set up error logging modes because if SAF_TRACING is requested
 *		but SAF_ERROR_REPORTING is "none", SAF will set error logging mode to stderr.
 *
 * Return:	Non-negative on success; negative on failure.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
static void
_saf_set_tracing(const char *env_var_name,      /* Name of env. variable that governs tracing */
                 SAF_LibProps *q	        /* OUT: Properties to set */
                 )
{
    SAF_ENTER_NOINIT(_saf_set_tracing, /*void*/);
    
   char *s = NULL;

   SAF_GETENV_WORLD(s,env_var_name);

   if (s == NULL)
   {
      q->TraceLogMode = SAF_TRACELOG_NONE;
      q->TraceTimes = false;
   }
   else if (!strncmp(s,"public,private,times",strlen("public,private,times")))
   {
      q->TraceLogMode = SAF_TRACELOG_PRIVATE;
      q->TraceTimes = true;
   }
   else if (!strncmp(s,"public,times",strlen("public,times")))
   {
      q->TraceLogMode = SAF_TRACELOG_PUBLIC;
      q->TraceTimes = true;
   }
   else if (!strncmp(s,"public,private",strlen("public,private")))
   {
      q->TraceLogMode = SAF_TRACELOG_PRIVATE;
      q->TraceTimes = false;
   }
   else if (!strncmp(s,"public",strlen("public")))
   {
      q->TraceLogMode = SAF_TRACELOG_PUBLIC;
      q->TraceTimes = false;
   }
   else if (!strncmp(s,"times",strlen("times")))
   {
      q->TraceLogMode = SAF_TRACELOG_NONE;
      q->TraceTimes = true;
   }
   else
   {
      q->TraceLogMode = SAF_TRACELOG_NONE;
      q->TraceTimes = false;
   }

   /* force tracetimes to be off if we don't have gettimeofday */
#ifndef HAVE_GETTIMEOFDAY
   q->TraceTimes = false;
#endif

   if(s) free(s);/*cannot use _saf_free until lib is initialized*/

   SAF_LEAVE(/*void*/);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:     Specify registry file
 *
 * Description: The registry consists of one or more SAF databases which will be consulted when an object query cannot
 *              be satisfied from the primary database.  For instance, if the client performed a saf_find_one_unit() to obtain a
 *              handle for something called "millimeter" and the find operation could not find any matching definition in the
 *              specified database, then each SAF registry database will be queried until a definition can be found or all
 *              registered registry files have been exhausted.
 *
 *              The library consults registries in the following order: First all files specified with the SAF_REGISTRIES
 *              environment variable (if the variable is set to the word `none' then no registries are consulted). The
 *              environment variable can specify multiple registries by separating them from one another with colons.  Second,
 *              all files registered with saf_setProps_Registry() are searched in the order they were specified. Third, a file
 *              by the name of FILE:Registry.saf in the current working directory, then the home directory (as specified by the
 *              environment variable `HOME').  Last, SAF will check for a file named FILE:Registry.saf in the data installation
 *              directory specified during the saf configuration with the `--datadir' switch (defaults to FILE:/usr/local/share).
 *
 * Issue:       The library does not attempt to open the registry file until a database is opened. Therefore,
 *              specifying an invalid file name here will not result in an error until the call to saf_open_database() is
 *              made.
 *
 * Return:      A non-negative value on success. Otherwise this function either returns a negative error number or throws an
 *              exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, April 17, 2001
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_Registry(SAF_LibProps *properties,            /* Library properties (See Properties) */
                      const char *name                    /* Name of object registry file */
                      )   
{
    SAF_ENTER_NOINIT(saf_setProps_Registry, SAF_PRECONDITION_ERROR);
    
    SAF_LibProps *p = properties;
    size_t i;

    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must be a valid library properties handle"));
    SAF_REQUIRE(name && *name, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("NAME is required to be non-empty"));

    /* An easy-to-fool check that we're not adding the same thing more than once */
    for (i=0; i<p->reg.nused; i++) {
        if (!strcmp(name, p->reg.name[i]))
            SAF_RETURN(0);
    }

    /* Allocate more space for table */
    if (p->reg.nused>=p->reg.nalloc) {
        p->reg.nalloc = MAX(16, 2*p->reg.nalloc);
        p->reg.name = realloc(p->reg.name, p->reg.nalloc*sizeof(char*));
        p->reg.nowarn = realloc(p->reg.nowarn, p->reg.nalloc*sizeof(p->reg.nowarn[0]));
    }

    /* Add the name to the end of the list */
    p->reg.name[p->reg.nused] = _saf_strdup(name);
    p->reg.nowarn[p->reg.nused] = SAF_TRISTATE_FALSE;
    p->reg.nused++;

    SAF_LEAVE(0);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Library Properties
 * Purpose:    	Turn off aborts 
 *
 * Description: In certain cases, the library will abort when it encounters an error condition. This function turns that
 *		behavior off.
 *
 * Parallel:    This function can be called independently. Nonetheless, properties passed to saf_init must be consistent
 * 		on all processors.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_DontAbort(SAF_LibProps *properties   /* The library property list which will be modified by this function */)
{
    SAF_ENTER_NOINIT(saf_setProps_DontAbort, SAF_PRECONDITION_ERROR);
    
    SAF_LibProps *p = properties;

    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must be a valid library properties handle"));

    p->DoAbort = false;
    SAF_LEAVE(SAF_SUCCESS);
}
