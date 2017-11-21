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

#ifdef HAVE_IO_H
#include <io.h> /*for access() on WIN32*/
#endif

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Databases
 * Purpose:     Access constants
 * Description: We have to define these because access' constants F_OK, W_OK and R_OK start with 0x0. Stupid!
 *-------------------------------------------------------------------------------------------------------------------------------
 */
typedef int SAF_FileAccess;
#define EXISTS          0x1
#define WRITEABLE       0x2
#define READABLE        0x4

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Databases
 * Description:
 *
 * A database is an abstraction used to represent the container in which all data that is part of a common,
 * aggregate collection is stored. For a typical simulation code, the database abstraction represents a single container in which
 * *all*fields*from*all*time*steps* for a given run of the simulation are stored. If, in fact, there are many simulation runs that
 * are part of some larger ensemble of runs, then the database abstraction ought to represent a single container in which
 * *all*fields*from*all*time*steps*from*all*simulations* are stored.
 *
 * In our current software, there are two serious limitations with respect to how we implement the database abstraction.
 *
 * First, no matter what container abstraction we introduce for our clients to read/write SAF field data, they ultimately
 * interact with the resulting data via a number of other tools outside the current scope of the SAF effort. Many of these tools
 * interact with the data as files in the filesystem. Examples are /rm,/cp,/ls,/f_stat,/ftp,/diff,/ etc. Granted, as files get
 * larger and larger, these tools become unwieldy. These tools provide a view of the data in terms of files. Because of this, our
 * customers have an expectation and a serious requirement to have control over how a database gets implemented in terms of files.
 *
 * We have no saf_del_xxx_handle functions for databases because the client *always* must call saf_open_database() to obtain a
 * database handle and saf_close_database to free a database handle
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Databases
 * Purpose:     Open a database
 *
 * Description: Opens or creates a database for read and/or write access (depending on PROPERTIES)
 *              using the communicator specified in PROPERTIES. The name of the database, PATH, is a file name. The
 *              PROPERTIES argument, if not SAF_DEFAULT_DBPROPS, provides database properties that will
 *              override the default properties set by saf_createProps_database().
 *
 * Issue:       It would be nice to identify the current processor decomposition, if possible. At the moment, we can't. But
 *              the idea would be that if we're opening an already existing database, we should search for a PROCESSOR
 *              collection on the top set(s) such that the size of that collection is equal to the value returnd by
 *              MPI_Comm_size() above. In this way, the database could "know" which sets are associated with which processors.
 *              At present we don't do this.
 *
 * Parallel:    This is a collective, SAF_ALL mode, call in the communicator specified by the PROPERTIES passed in the call.
 *
 * Return:      Returns a new handle to the opened database on success; NULL on failure (or an exception is raised).
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Db *
saf_open_database(const char *path,             /* The name of the database. */
                  SAF_DbProps *properties       /* This argument, if not null, provides database
                                                 * properties that will override the default properties provided by
                                                 * saf_createProps_database(). */
                  )
{
  SAF_ENTER(saf_open_database, NULL);

  SAF_Db        *db=NULL;
  SAF_DbProps   *p=properties;
  unsigned      flags=0;
  ss_prop_t     *fprops=NULL;
  size_t        regno;
  ss_scope_t    regscope;
  size_t        naux, i;
  ss_file_ref_t *filerefs=NULL;

  SAF_REQUIRE(path != NULL, SAF_LOW_CHK_COST, NULL, _saf_errmsg("PATH must be non-null"));

  /* Obtain properties. Either the user passed in a database properties handle or they passed null. In the latter
   * case we create our own handle to default properties and release that at the end. */
  if (!properties)
      p = saf_createProps_database();
  SAF_REQUIRE(p, SAF_HIGH_CHK_COST, NULL,
              _saf_errmsg("PROPERTIES must be a valid handle if supplied"));

  /* Build an SSlib property list and flags for opening the file */
  if (NULL==(fprops = ss_prop_new(path)))
      SAF_ERROR(NULL, _saf_errmsg("cannot create property list for ss_file_open"));
  if (ss_prop_add(fprops, "comm", H5T_NATIVE_MPI_COMM, &(p->DbComm))<0)
      SAF_ERROR(NULL, _saf_errmsg("cannot insert MPI communicator into property list for ss_file_open"));
  flags |= p->ReadOnly ? H5F_ACC_RDONLY : H5F_ACC_RDWR | H5F_ACC_CREAT;
  if (p->Clobber) flags |= H5F_ACC_TRUNC | H5F_ACC_CREAT | H5F_ACC_RDWR;
  if (p->MemoryResident) flags |= H5F_ACC_TRANSIENT | H5F_ACC_CREAT | H5F_ACC_RDWR;

  /* Open or create the file */
  if (NULL==(db=ss_file_open(NULL, path, flags, fprops)))
      SAF_ERROR(NULL, _saf_errmsg("ss_file_open failed"));

  /* Open all auxiliary files using the same communicator */
  if (NULL==(filerefs=ss_file_references(db, &naux, NULL, NULL)))
      SAF_ERROR(NULL, _saf_errmsg("cannot obtain list of auxiliary files"));
  if (ss_file_openall(naux, filerefs, flags, fprops)<0)
      SAF_ERROR(NULL, _saf_errmsg("cannot open some auxiliary files"));
  
  /* Attach registry scopes to the files unless we're opening a registry */
  if (!p->NoRegistries) {
      for (regno=0; regno<_SAF_GLOBALS.p.reg.nused; regno++) {
          if (!_SAF_GLOBALS.p.reg.db[regno]) continue;
          if (NULL==ss_file_topscope(_SAF_GLOBALS.p.reg.db[regno], &regscope))
              SAF_ERROR(NULL, _saf_errmsg("cannot get top scope for registry"));
          if (ss_file_registry(db, &regscope)<0)
              SAF_ERROR(NULL, _saf_errmsg("cannot set registry for file"));
          for (i=0; i<naux; i++) {
              if (ss_file_registry(&(filerefs[i].file), &regscope)<0)
                  SAF_ERROR(NULL, _saf_errmsg("cannot set registry for auxiliary file"));
          }
      }
  }

  /* Free the property list */
  ss_prop_dest(fprops);
  for (i=0; i<naux; i++) SS_FREE(filerefs[i].newname);
  SS_FREE(filerefs);

  SAF_LEAVE(db);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Databases
 * Purpose:     Close a database
 *
 * Description: This function closes an open database, DATABASE, freeing all resources associated with that database.
 *
 * Parallel:    This is a collective, SAF_ALL mode function which should be called across all processes in the database's
 *              communicator.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_close_database(SAF_Db *database/*The open database to be closed.*/)
{
    SAF_ENTER(saf_close_database, SAF_PRECONDITION_ERROR);

    SAF_REQUIRE(SS_MAGIC(ss_file_t)==SS_MAGIC_OF(database), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DATABASE must be a database handle"));
    SAF_REQUIRE(ss_file_isopen(database, NULL), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DATABASE must currently be open"));

    if (ss_file_close(database)<0)
        SAF_ERROR(-1, _saf_errmsg("ss_file_close() failed"));
    
    SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Databases
 * Purpose:     Update database contents
 *
 * Description: This function is used to force the library to update the contents of the database to the most recent
 *              operation issued by the client. In the case of file I/O, all pending writes will be flushed so that files
 *              are consistent with the most recent operation.
 *
 * Parallel:    This call is collective across all processes in the MPI communicator used to open the database.
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_update_database(SAF_Db *database/*The database to update*/)
{
    SAF_ENTER(saf_update_database, SAF_PRECONDITION_ERROR);
    ss_scope_t          topscope=SS_SCOPE_NULL;

    SAF_REQUIRE(SS_MAGIC(ss_file_t)==SS_MAGIC_OF(database), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DATABASE must be a database handle"));
    SAF_REQUIRE(!_saf_database_is_read_only(database), SAF_NO_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("DATABASE must not be open for read-only access"));

    /* Synchronize all scopes of the file, then flush all scopes of the file */
    if (ss_file_synchronize(database, NULL)<0)
        SAF_ERROR(-1, _saf_errmsg("ss_file_synchronize() failed"));
    if (ss_file_flush(database, NULL)<0)
        SAF_ERROR(-1, _saf_errmsg("ss_file_flush() failed"));

    /* Flush raw data to the file */
    if (NULL==ss_pers_topscope((ss_pers_t*)database, &topscope))
        SAF_ERROR(-1, _saf_errmsg("ss_pers_topscope() failed"));
    if (ss_blob_flush(&topscope, NULL, SS_STRICT, NULL)<0)
        SAF_ERROR(-1, _saf_errmsg("ss_blob_flush() failed"));

    SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Databases
 * Purpose:     Determines if the database db is read-only
 *
 * Description: 
 *
 * Return:      True if the database is read-only, false otherwise
 *
 * Programmer:  Bill Arrighi
 *              Friday, August 29, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_database_is_read_only(SAF_Db *db)
{
  SAF_ENTER(_saf_database_is_read_only, FALSE);
  htri_t        retval;

  SAF_REQUIRE(SS_MAGIC_OF(db)==SS_MAGIC(ss_file_t), SAF_HIGH_CHK_COST, false, _saf_errmsg("DB must be a valid database"));
  retval = !ss_file_iswritable(db);

  SAF_LEAVE(retval);
}

