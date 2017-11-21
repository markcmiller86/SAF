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
 * Audience:    Public
 * Chapter:     Database Properties
 * Description:	There are a number of properties that affect the behavior of a database. Each member function of this
 *		portion of the API sets a property to be associated with a database to a given value. See the individual
 *		member functions for a more detailed description of the database properties and their meaning.
 *		For a general description of how properties are used (See Properties).
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Database Properties
 * Purpose:     Create a new database property list with default values
 *
 * Concepts:    database properties, initialization of; initializing default database properties
 *
 * Description: This function creates a database property list which can be passed to the saf_open_database() function. All
 *              properties in this list are set to their default values:
 *
 *		   Clobber = false;
 *		   DbComm = LibComm;
 *		   ImportFile = $SAF_STD_TYPES_PATH  //or
 *                              std_types.saf        //or
 *                              FILE:~/.std_types.saf     //or
 *                              FILE:$SAF_INSTALL/share/std_types.saf;
 *		   ReadOnly = false;
 *
 * Return:      A handle to a new database properties list initialized to default values. Otherwise either an error value is
 *              returned or an exception is thrown depending on the error handling library property currently in effect
 *		(See Properties).
 *
 * Bugs:        This function sometimes return an error instead of throwing an exception when the library error mode is
 *              SAF_ERRMODE_THROW.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_DbProps *
saf_createProps_database(void)
{
    SAF_ENTER(saf_createProps_database, NULL);

    SAF_DbProps *p = calloc(1, sizeof *p);

    /* Allocate properties */
    if (!p)
        SAF_ERROR(NULL, _saf_errmsg("out of memory--unable to allocate properties object"));

    /* parallel mpi communicator */
    p->DbComm = _SAF_GLOBALS.p.LibComm;

    SAF_LEAVE(p);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Database Properties
 * Purpose:     Specify MPI database communicator
 *
 * Description: When a database is opened it uses the library communicator by default. However, this function can be called to
 *              set up a different MPI communicator to open a database. However, that communicator *must*be* a subset of
 *		the communicator used to initialize the library.
 *
 * Parallel:    This function can be called independently. It is not defined in serial installations of the library.
 *		This function *does*not* duplicate the communicator. It simply copies it to the properties. When these properties
 *		are used in a saf_open_database() call, the communicator will at that time be duplicated. So, don't free the
 *		MPI communicator between the time this property is set in a given SAF_DbProps structure and the time that
 *		SAF_DbProps structure is used in a saf_open_database() call.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issue:	If the client is going to override the MPI communicator that would ordinarily be associated with the database
 *		handle, we have a minor problem with calls to set other properties whose behavior might require special
 *		action for parallel: which communicator should they use?
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_DbComm(SAF_DbProps *properties,    /* The database property list which will be modified by this function
						   (See Properties). */
                    MPI_Comm communicator       /* The MPI communicator. */
                    )
{
    SAF_ENTER(saf_setProps_DbComm, SAF_PRECONDITION_ERROR);
    SAF_DbProps         *p = properties;

    SAF_REQUIRE(p, SAF_HIGH_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must not be null"));

    p->DbComm = communicator;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Database Properties
 * Purpose:     Specify read-only database access
 *
 * Description: By default a database is opened for read/write access. This function changes the access property
 *              so the database is opened for read-only access.
 *
 *		Opening for read-only when the client is, in fact, only reading the database can potentially have 
 *		a dramatic impact on parallel performance. The reason is that the lower level data modeling kernel, VBT,
 *		can duplicate the metadata tables on all processors and wholly eliminate all MPI communication involved
 *		in interacting with the database. At present, VBT does *not* actually do this but will, in the future, be
 *		modified to do so.
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
saf_setProps_ReadOnly(SAF_DbProps *properties   /* The database property list which will be modified by this function
						   (See Properties). */
                      )
{
    SAF_ENTER(saf_setProps_ReadOnly, SAF_PRECONDITION_ERROR);
    SAF_DbProps *p = properties;
    
    SAF_REQUIRE(p, SAF_HIGH_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must not be null"));

    p->ReadOnly = true;
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Database Properties
 * Purpose:     Clobber an existing database on open 
 *
 * Description: When saf_open_database() is called with the name of an existing database the default action is to open that
 *              database. New data will be appended to it. However, if this property is set, then the existing database will
 *		be unlinked before it is opened. 
 *
 * Parallel:    This function can be called independently.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Bugs:        This function sometimes return an error instead of throwing an exception when the library error mode is
 *              SAF_ERRMODE_THROW.
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
saf_setProps_Clobber(SAF_DbProps *properties    /* The database property list which will be modified by this function
						  (See Properties). */
                     )
{
    SAF_ENTER(saf_setProps_Clobber, SAF_PRECONDITION_ERROR);
    SAF_DbProps *p = properties;
    
    SAF_REQUIRE(p, SAF_HIGH_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PROPERTIES must not be null"));

    p->Clobber = true;
    SAF_LEAVE(SAF_SUCCESS);
}

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Database Properties
 * 
 * Purpose:    	Create an memory-resident database 
 *
 * Description: Setting this property permits the creation of a memory-resident database. A memory-resident database is convenient
 *              for creating objects that you would like to be transient. All objects created in a memory-resident database will
 *              be lost when the database is closed.
 *
 * Parallel:    This function can be called independently, however all tasks must agree whether promise mode is to be used for
 *              a particular database when that database is opened.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Programmer:  Mark C. Miller
 *              Wednesday, January 22, 2003 
 *--------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_setProps_MemoryResident(SAF_DbProps *properties)
{
    SAF_ENTER(saf_setProps_MemoryResident, SAF_PRECONDITION_ERROR);
    SAF_DbProps *p = properties;

    SAF_REQUIRE(p, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
      _saf_errmsg("PROPERTIES must not be null"));

    p->MemoryResident = true;
    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Database Properties
 * Purpose:     Free database property list
 *
 * Description: Releases resources inside the database property list and frees the property list that was allocated in
 *              saf_createProps_database().
 *
 * Return:      Always returns null.
 *
 * Parallel:    Independent
 *
 * Issue:       Releasing the resources used by the property list was never implemented and so is not implemented here yet
 *              either.
 *
 * Programmer:  Robb Matzke
 *              Monday, September 13, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_DbProps *
saf_freeProps_database(SAF_DbProps *properties)
{
    if (properties) free(properties);
    return NULL;
}

