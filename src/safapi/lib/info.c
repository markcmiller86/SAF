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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Path Info 
 * Description:	These functions are designed to permit a SAF client to obtain information about a named file without
 *		actually requiring the client to succesfully open the file with a call to saf_open_database(). This is
 *		particularly useful for clients that need to be responsive to different versions of SAF databases.
 *
 *		All the functions in this part of the interface return information about the file identified in the
 *		saf_readInfo_path() call and how that file was generated. The typical usage of these functions is to first
 *		obtain information about the given file with a call to saf_readInfo_path(). Then, use the various query functions
 *		here to obtain specific information about the file name (path) passed into saf_readInfo_path() and finally
 *		to free up any resources with a call to saf_freeInfo_path().
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*DOCUMENTED*/
#define ENOTHDF5	(-1)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private 
 * Chapter:	Path Info
 * Purpose:	Traverse an HDF5 type
 *
 * Description:	Given an hdf5 data type and the name of a supposed member of that type, this function will return, all
 *		optionally, the byte-offset of the member within the type, the /index/ of the member within the type and
 *		the data type of the member.
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
static herr_t
_saf_h5t_descend(
   hid_t parentType,	/* [IN] the parent (struct) in which the named member exists */
   const char *theName,	/* [IN] the name of the member within the type to search for */
   size_t *offset,	/* [OUT] the offset of the named member within the type. Ignored if NULL. */
   int *index,		/* [OUT] the index within the type of the named member. Ignored if NULL. */
   hid_t *memberType	/* [OUT] the type of the named member. Ignored if NULL. */
)
{
    SAF_ENTER(_saf_h5t_descend, -1);

   int memIdx;

   assert(parentType>0);
   assert(theName);

   memIdx = H5Tget_member_index(parentType, theName);
   if (memIdx<0)
   {
      if (offset)
         *offset = (size_t)-1;
      if (index)
         *index = -1;
      if (memberType)
         *memberType = -1;
      SAF_RETURN(-1);
   }

   if (offset)
       *offset = H5Tget_member_offset(parentType, (unsigned)memIdx);
   if (index)
       *index = memIdx;
   if (memberType)
       *memberType = H5Tget_member_type(parentType, (unsigned)memIdx);

   SAF_LEAVE(0);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private 
 * Chapter:	Path Info
 * Purpose:	Copy data from a member of a type	
 *
 * Description:	Given pointers to source and destination buffers and hdf5 type identifers for the source and destination types,
 *		and a count of the number of values to copy, this function copies values from source to destination, possibly
 *		converting the values. Note that there must exist a conversion path, in HDF5, for the types.
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
static void
_saf_h5t_copyvals(
   void *dstBuf,	/* [OUT] the destination buffer (allocated by caller) */
   hid_t dstType,	/* [IN] the destination data type */
   size_t nVals,	/* [IN] the number of values to copy */
   void *srcBuf,	/* [IN] the source buffer */
   hid_t srcType	/* [IN] the source data type */
)
{
   SAF_ENTER(_saf_h5t_copyvals, /*void*/);
   size_t nSrcBytes, nDstBytes;
   void *convBuf;

   assert(dstBuf);
   assert(nVals>0);
   assert(srcBuf);
   assert(srcType>0);

   nSrcBytes = H5Tget_size(srcType) * nVals;

   if ((dstType>0) && !H5Tequal(srcType,dstType))
   {
      nDstBytes = H5Tget_size(dstType) * nVals;

      /* A call to H5Tconvert changes the buffer. We don't want to change the source data. And, we can't guarentee the
         destination buffer is large enough to copy the source buffer into because there may be type conversion involved.
         So, we make an extra copy of the source data */
      convBuf = (void *) malloc(nSrcBytes);
      assert(convBuf);
      memcpy(convBuf, srcBuf, nSrcBytes);

      /* now do the convert */
      H5Tconvert(srcType, dstType, nVals, convBuf, NULL, H5P_DEFAULT);

      /* now, copy the results, which should now be the correct size, into the destination buffer */
      memcpy(dstBuf, convBuf, nDstBytes); 
      free(convBuf);
   }
   else
      memcpy(dstBuf, srcBuf, nSrcBytes);

   SAF_LEAVE(/*void*/);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Private 
 * Chapter:	Path Info
 * Purpose:	Get parts of the version number structure	
 *
 * Description:	Given the name of the software layer, this function obtains the major, minor, patch and annotation string
 *              portions of a version number from the SAF_PathInfo object. The software layer is the name of a member of a
 *              compound struct stored in the "version" attribute of the "/SAF" group of an HDF5 file.
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
static void
_saf_getInfo_version(
   const SAF_PathInfo info,/* [IN] database info object obtained from a saf_readInfo_path() call. */
   const char *vname,	/* [IN] version member struct name */
   int *major, 		/* [OUT] major version number. Ignored if NULL. */
   int *minor,		/* [OUT] minor version number. Ignored if NULL. */
   int *patch, 		/* [OUT] patch (aka "release") version number. Ignored if NULL. */
   char *annot		/* [OUT] annotation string of at most 8 chars including null. Caller allocates. Ignored if NULL. */
)
{
   SAF_ENTER(_saf_getInfo_version, /*void*/);
    
   hid_t dbPropsType;
   char *dbPropsBuf;
   size_t vStructOffset;
   size_t vNumberOffset;
   hid_t vStructType;
   hid_t vNumberType;

   assert(info);
   assert(vname);

   dbPropsType = info->dbPropsType;
   dbPropsBuf = (char*) (info->dbPropsBuf);

   if (major) *major = 0;
   if (minor) *minor = 0;
   if (patch) *patch = 0;
   if (annot) *annot = '\0';

   /* find the named member in the DbProps structure */
   _saf_h5t_descend(dbPropsType, vname, &vStructOffset, NULL, &vStructType);
#ifdef WIN32
   if (SS_NOSIZE==vStructOffset) SAF_ERROR(0, _saf_errmsg("no %s version", vname))
#else
   if (SS_NOSIZE==vStructOffset) SAF_ERROR(/*void*/, _saf_errmsg("no %s version", vname))
#endif
   assert(vStructType>0);

   if (major)
   {
      _saf_h5t_descend(vStructType, "major", &vNumberOffset, NULL, &vNumberType);
      if (vNumberOffset!=(size_t)-1)
      {
         _saf_h5t_copyvals(major, H5T_NATIVE_INT, 1, (void*) (dbPropsBuf + vStructOffset + vNumberOffset), vNumberType);
         H5Tclose(vNumberType);
      }
      else
	 *major = -1;
   }
   if (minor)
   {
      _saf_h5t_descend(vStructType, "minor", &vNumberOffset, NULL, &vNumberType);
      if (vNumberOffset!=(size_t)-1)
      {
         _saf_h5t_copyvals(minor, H5T_NATIVE_INT, 1, (void*) (dbPropsBuf + vStructOffset + vNumberOffset), vNumberType);
         H5Tclose(vNumberType);
      }
      else
	 *minor = -1;
   }
   if (patch)
   {
      _saf_h5t_descend(vStructType, "release", &vNumberOffset, NULL, &vNumberType);
      if (vNumberOffset!=(size_t)-1)
      {
         _saf_h5t_copyvals(patch, H5T_NATIVE_INT, 1, (void*) (dbPropsBuf + vStructOffset + vNumberOffset), vNumberType);
         H5Tclose(vNumberType);
      }
      else
	 *patch = -1;
   }
   if (annot)
   {
      _saf_h5t_descend(vStructType, "annot", &vNumberOffset, NULL, &vNumberType);
      if (vNumberOffset!=(size_t)-1)
      {
         _saf_h5t_copyvals(annot, -1, 1, (void*) (dbPropsBuf + vStructOffset + vNumberOffset), vNumberType);
         H5Tclose(vNumberType);
      }
      else
	 annot[0] = '\0';
   }
   H5Tclose(vStructType);
   SAF_LEAVE(/*void*/);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Load information from the specified path
 *
 * Description:	This function is used to query a file in the filesystem to obtain information about it. The information is
 *		returned in the SAF_PathInfo object. Once this object is obtained, one can query a number of things about
 *		the database using various query functions. Once you are done, remember to free the resources with
 *		saf_freeInfo_path().
 *
 *		Note that any path, except NULL, is acceptable to pass here. This function will obtain as much information about
 *		the specified path as possible. If the path either does not exist or the user does not have permission to
 *		read it, that fact can be obtained from saf_getInfo_permissions(). Likewise, if the path does exist, is
 *		readable and is a SAF database, the particular version information, etc. can be obtained from functions
 *		like saf_getInfo_libversion(), etc. A variety of failure modes are detectable by calling various functions
 *		in this part of the interface to learn about the kind of file to which /path/ refers.
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_PathInfo
saf_readInfo_path(
   const char *path, 	/* [IN] path of a file to get the info for */
   int independent	/* [IN] A flag for independent operation. If non-zero, perform the work and return the results only on 
			   the calling processor. Otherwise, this function must be called collectively by all processors in
			   the communicator used to init the SAF library. In other words, call this function from one processor
			   with a non-zero value for this argument or call it an all processors with a zero argument on all
			   processors. Note also that if this call is made independently, then all succeeding calls involving
			   the returned SAF_PathInfo object must be made independently and by the same processor. */
)
{
   SAF_ENTER(saf_readInfo_path,NULL);

   int rank=0;
   SAF_PathInfo result;
   int theStats[2];

   SAF_REQUIRE(path!=NULL,SAF_LOW_CHK_COST,NULL,_saf_errmsg("path must be non-NULL"));

   independent = independent; /* quite the compiler */

#ifdef HAVE_PARALLEL
   if (!independent)
      MPI_Comm_rank(_SAF_GLOBALS.p.LibComm,&rank);
#endif

   /* allocate the SAF_PathInfo stuff */
   result = (SAF_PathInfo) calloc(1, sizeof(SAF_PathInfoPkt));

   /* in all cases, only stat the file on processor 0 in the lib communicator */
   if (rank == 0)
   {
      struct stat statBuf;

      /* stat the file */
      if (stat(path, &statBuf)==-1)
         theStats[0] = errno;
      else
      {
         theStats[0] = 0;

#ifdef WIN32
	/* These S_* #defines dont exist in WIN32*/
	#define S_IRWXU _S_IREAD|_S_IWRITE|_S_IEXEC /*owner, = 00700*/
	#define S_IRWXG 00070 /*group*/
	#define S_IRWXO 00007 /*others*/
#endif

	 theStats[1] = (int) (statBuf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
      }

      /* if the stat succeeded, theStats[0] will be zero and we can proceed. In such case, to save a broadcast, I next test
	 whether the file is an HDF5 file and if not, set theStats[0] to a contrived error code for that case. The only worry
	 is to make sure my contrived error code does not collide with another case in the switch statement below. */ 
      if (!theStats[0] && !H5Fis_hdf5(path))
         theStats[0] = ENOTHDF5;

   }

#ifdef HAVE_PARALLEL
   if (!independent)
      MPI_Bcast(&theStats, 2, MPI_INT, 0, _SAF_GLOBALS.p.LibComm);     
#endif

   /* handle any inital errors */
   if (theStats[0])
   {
      result->statError = 1;
      result->permissions = theStats[1];
      switch (theStats[0])
      {
	 case ENOTHDF5: result->errStr = _saf_strdup("not an hdf5 file"); break;
         case EACCES:   result->errStr = _saf_strdup("permission denied"); break;
         case ENOENT:   result->errStr = _saf_strdup("does not exist"); break;
         case ENOTDIR:  result->errStr = _saf_strdup("invalid path"); break;
         default:
         {
	    result->errStr = (char *) malloc(32);
	    sprintf(result->errStr,"unspecified stat error %d",theStats[0]);
	    break;
         }
      }
      goto done;
   }
   else
   {
      result->statError = 0;
      result->permissions = theStats[1];
   }

   /* at this point, we know we can read the file and that it is an HDF5 file */

   /* we proceed using HDF interface to obtain SAF's version stuff because we can't always be assured we will have
      a valid SSlib file AND we'd like to return as much information to the client as possible AND, once we've coded
      to use HDF5, there is little point in also doing it with anything higher */
   {
       hid_t fid, topgroup, attr, dtype, fprop_id;

#ifdef HAVE_PARALLEL
      fprop_id = H5Pcreate(H5P_FILE_ACCESS);
      if (independent) {
          H5Pset_fapl_mpio(fprop_id, MPI_COMM_SELF, MPI_INFO_NULL);
      } else {
          H5Pset_fapl_mpio(fprop_id, _SAF_GLOBALS.p.LibComm, MPI_INFO_NULL);
      }
#else
      fprop_id = H5Pcopy(H5P_DEFAULT);
#endif

      /* try to open the file */
      fid = H5Fopen(path, H5F_ACC_RDONLY, fprop_id);
      H5Pclose(fprop_id);

      if (fid < 0)
      {
         result->isHDFfile = 0;
         goto done;
      }
      else
         result->isHDFfile = 1;

      /* open the "/SAF" group, which is the name of the top-level scope in an SSlib file */
      H5E_BEGIN_TRY {
         topgroup = H5Gopen(fid,"/SAF");
      } H5E_END_TRY
       
      if (topgroup < 0)
      {
         H5Fclose(fid);
         goto done;
      }

      /* obtain data type, size information and allocate space for data */
      attr = H5Aopen_name(topgroup, "version");
      dtype = H5Aget_type(attr);
      result->dbPropsType = H5Tget_native_type(dtype, H5T_DIR_DEFAULT);
      result->dbPropsBuf = malloc(H5Tget_size(result->dbPropsType));
      
      /* read the DbProps attribute */
      H5Aread(attr, result->dbPropsType, result->dbPropsBuf);

      H5Tclose(dtype);
      H5Aclose(attr);
      H5Gclose(topgroup);
      H5Fclose(fid);
   }

   result->allOk = 1;

 done:
   SAF_LEAVE(result);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Free SAF_PathInfo	
 *
 * Description:	This function is used to free a SAF_PathInfo object.
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
saf_freeInfo_path(SAF_PathInfo info /* a SAF_PathInfo object obtained from a saf_readInfo_path() call. */)
{
   SAF_ENTER(saf_freeInfo_path,/*void*/);
   SAF_REQUIRE(info,SAF_LOW_CHK_COST,/*void*/,_saf_errmsg("INFO must be non-NULL"));

   if (info->errStr)
      free(info->errStr);
   if (info->dbPropsBuf)
   {
      free(info->dbPropsBuf);
      H5Tclose(info->dbPropsType);
   }
   free(info);
   SAF_LEAVE(/*void*/);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Get the SAF library version	
 *
 * Description:	This function is used to obtain SAF library version information from a given path queried with saf_getInfo_path().
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
saf_getInfo_libversion(
   const SAF_PathInfo info,/* [IN] database info object obtained from a saf_readInfo_path() call. */
   int *major, 		/* [OUT] major version number. Ignored if NULL. */
   int *minor,		/* [OUT] minor version number. Ignored if NULL. */
   int *patch, 		/* [OUT] patch (aka "release") version number. Ignored if NULL. */
   char *annot		/* [OUT] annotation string of at most 8 chars including null. Caller allocates. Ignored if NULL. */
)
{
   SAF_ENTER(saf_getInfo_libversion,/*void*/);
   SAF_REQUIRE(info,SAF_LOW_CHK_COST,/*void*/,_saf_errmsg("INFO must be non-NULL"));
   if (info->allOk)
      _saf_getInfo_version(info, "saf", major, minor, patch, annot);
   SAF_LEAVE(/*void*/);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Get the HDF5 version	
 *
 * Description:	This function is used to obtain HDF5 library version information from a given path queried with
 *		saf_getInfo_path().
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
saf_getInfo_hdfversion(
   const SAF_PathInfo info,/* [IN] database info object obtained from a saf_readInfo_path() call. */
   int *major, 		/* [OUT] major version number. Ignored if NULL. */
   int *minor,		/* [OUT] minor version number. Ignored if NULL. */
   int *patch, 		/* [OUT] patch (aka "release") version number. Ignored if NULL. */
   char *annot		/* [OUT] annotation string of at most 8 chars including null. Caller allocates. Ignored if NULL. */
)
{
   SAF_ENTER(saf_getInfo_hdfversion,/*void*/);
   SAF_REQUIRE(info,SAF_LOW_CHK_COST,/*void*/,_saf_errmsg("INFO must be non-NULL"));
   if (info->allOk)
      _saf_getInfo_version(info, "hdf5", major, minor, patch, annot);
   SAF_LEAVE(/*void*/);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Get the MPI library version
 *
 * Description:	This function is used to obtain MPI library version information from a given path queried with saf_getInfo_path().
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
void
saf_getInfo_mpiversion(
   const SAF_PathInfo info,/* [IN] database info object obtained from a saf_readInfo_path() call. */
   int *major, 		/* [OUT] major version number. Ignored if NULL. */
   int *minor,		/* [OUT] minor version number. Ignored if NULL. */
   int *patch, 		/* [OUT] patch (aka "release") version number. Ignored if NULL. */
   char *annot		/* [OUT] annotation string of at most 8 chars including null. Caller allocates. Ignored if NULL. */
)
{
   SAF_ENTER(saf_getInfo_mpiversion,/*void*/);
   SAF_REQUIRE(info,SAF_LOW_CHK_COST,/*void*/,_saf_errmsg("INFO must be non-NULL"));
   if (info->allOk)
      _saf_getInfo_version(info, "mpi", major, minor, patch, annot);
   SAF_LEAVE(/*void*/);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Check if path is a SAF database	
 *
 * Description:	This function returns true if the path queried with saf_getInfo_path() is a SAF database (of any version).
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_getInfo_isSAFdatabase(
   const SAF_PathInfo info 	/* [IN] database info object obtained from a saf_readInfo_path() call. */
)
{
    SAF_ENTER(saf_getInfo_isSAFdatabase, info->allOk);
    SAF_LEAVE(info->allOk);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Obtain permissions of path	
 *
 * Description:	This function returns the permissions of the path queried with saf_getInfo_path().
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_getInfo_permissions(
   const SAF_PathInfo info 	/* [IN] database info object obtained from a saf_readInfo_path() call. */
)
{
    SAF_ENTER(saf_getInfo_permissions, info->permissions);
    SAF_LEAVE(info->permissions);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Check if any stat errors occured	
 *
 * Description:	This function returns true if any errors occured stating the path specified in saf_readInfo_path(). 
 *
 * Parallel:	Independent semantics only
 *
 * Also:        saf_getInfo_errmsg()
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_getInfo_staterror(
   const SAF_PathInfo info 	/* [IN] database info object obtained from a saf_readInfo_path() call. */
)
{
    SAF_ENTER(saf_getInfo_staterror, info->statError);
    SAF_LEAVE(info->statError);
}


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Get stat error message	
 *
 * Description:	This function returns error message associated with any stat errors on the path specified in saf_getInfo_path().
 *		Caller should *not* free the returned string.
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
const char *
saf_getInfo_errmsg(
   const SAF_PathInfo info 	/* [IN] database info object obtained from a saf_readInfo_path() call. */
)
{
    SAF_ENTER(saf_getInfo_errmsg, "");
    const char *retval = info->statError ? info->errStr : "";
    SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:	Public
 * Chapter:	Path Info
 * Purpose:	Check if path is an HDF5 file	
 *
 * Description:	This function returns true if the path queried with saf_getInfo_path() is an HDF5 file. That is, if
 *		H5Fopen() can succeed on it.
 *
 * Parallel:	Independent semantics only
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_getInfo_isHDFfile(const SAF_PathInfo info /* [IN] database info object obtained from a saf_readInfo_path() call. */)
{
    SAF_ENTER(saf_getInfo_isHDFfile, info->isHDFfile);
    SAF_LEAVE(info->isHDFfile);
}
