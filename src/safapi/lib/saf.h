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
#ifndef saf_h_included
/*DOCUMENTED*/
#define saf_h_included

/* System include files. This should be *only* those files which are needed for an application to #include saf.h. In other
 * words, saf.h should not include more than what is necessary for using saf.h, but it should include all that is necessary
 * for using saf.h. */
#include <setjmp.h>                     /*required for SAF_TRY_BEGIN et al*/

/* SAF Support Library */
#include "sslib.h"

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Introduction 
 *              
 * Description:
 *	This is the Sets and Fields (SAF pronounced "safe") Application Programming Interface (API) programmer's reference
 *	manual. This manual is organized into /Chapters/ where each chapter covers a different, top-level, set of functions
 *	(e.g. object and its supporting methods) SAF supports. 
 *
 *	SAF is designed first and foremost to support scalable I/O of shareable, scientific data.
 *
 *	The key words in this statement are /scalable/ and /shareable/. 
 *
 *	/Scalable/ means that SAF is designed to operate with high performance from single processor, workstation class machines,
 *	to large scale, parallel computing platforms such as are in use in the ASCI program ( http://www.lanl.gov/projects/asci/ )
 *	In turn, this also demands that SAF be portable across a variety of computing platforms. Currently, SAF operates
 *	in serial and parallel on Dec, Sun, Linux, IBM-SP2, Intel TeraFlops, SGI-O2k (single box). SAF is also supported
 *	in serial on Windows. A good measure of SAF's performance and portability is derived from its use of industry 
 *	standard software components such as HDF5 ( http://hdf.ncsa.uiuc.edu/ ) and MPI ( http://www.mpi-forum.org/ ). 
 *	However, scalable I/O is just one of SAF's primary goals. Making data /shareable/ is another.
 *
 *	/Shareable/ means that if one application uses SAF to write its data, other *wholly*independent* applications can
 *	easily read *and*interpret* that data. Of course, it is not all that impressive if one application can simply read a
 *	bunch of *bytes* that another has written. Thus, the key to understanding what /shareable/ means is the *and*interpret*
 *	part. SAF is designed to make it easy for one scientific computing application to interpret another's data.
 * 	Even more so, SAF is designed to enable this interpretation across a diverse and continually expanding gamut
 *	of scientific computing applications. In a nutshell, SAF lays the foundation for very large scale integration of
 *	scientific software.
 *
 *	The organizations involved in the development of SAF have plenty of experience
 *	with integration on smaller scales with products like netCDF, HDF, PATRAN, SEACAS, Silo and Exodus II.
 *	These technologies offer applications a menu of objects; some data structures
 *	(e.g. array, list, tree) and/or some mesh objects (e.g. structured-mesh, ucd-mesh, side-sets, etc.).
 * 	For application developers who use these products, the act of sharing their data is one of browsing the menu.
 *	If they are lucky, they will find an object that matches their data and use it. If
 *	they are unlucky, they will have to modify their data to put it into a form that matches one of the objects on the menu.
 *
 *	Thus, former approaches to making shareable data suffer from either requiring all clients to use the same data structures
 *	and/or objects to represent their data or by resulting in an ever expanding set of incrementally different data structures
 *	and/or objects to support each client's slightly different needs. The result is that these products can and have been
 *	highly successful within a *small* group of applications who either...
 *
 *      a) buy into the small menu of objects they do support 
 *
 *	or
 *
 *      b) don't require support for very many new objects (e.g. changes to the supporting library)
 *
 *	or
 *
 *      c) don't expect very many other applications to understand their data
 *
 *	In other words, previous approaches have succeeded in integration on the small scale but hold little promise for
 *	integration on the large scale.
 *
 *	The key to integration and sharing of data on the large scale is to find a small set of primitive, yet mathematically
 *	meaningful, building blocks out of which descriptions for many different kinds of scientific data can be constructed.
 *	In this approach, each new and slightly different kind of data requires the application of the same building blocks to
 *	form a slightly different /assembly/. Since every assembly is just a different application of the same building
 *	blocks, each is fully supported by existing software. In fact, every assembly of building blocks is simply a /model/
 *	for an instance of some scientific data. This is precisely how SAF is designed to operate.
 *	For application developers using SAF, the act of sharing their data is one of literally /modeling/ their data; not
 *	browsing a menu. This modeling is analogous to the user of a CAD/CAM tool when applying
 *	constructive solid geometry (CSG) primitives to build an engineering model for some physical part. In a nutshell, the
 *	act of sharing data with SAF is one of /scientific/data/modeling/.
 *
 *	This requires a revolution in the way scientific computing application developers think about their data. The details
 *	of bits and bytes, arrays and lists are pushed to the background. These concepts are still essential but less so
 *	than the modeling primitives used to characterize scientific data. These modeling primitives are firmly rooted in
 *	the mathematics underlying most, if not all, scientific computing applications. By and large, this means the model
 *	primitives will embody the mathematical and physical notions of /fields/ defined on /base/-/spaces/ or sets.
 *
 *	The term /field/ is used to describe any phenomenon that can be mathematically represented, at least locally, as a
 *	function over some, often continuous, base-space or domain. The term /base/-/space/ is used to describe an infinite
 *	point set, often continuous, with a topological dimension over which fields are defined.
 *	Thus, SAF provides three key modeling primitives; fields, sets, and relations between these entities. Fields may
 *	represent real physical phenomena such as pressure, stress and velocity. Fields may be related to other fields by
 *	integral, derivative or algebraic equations. Fields are defined on sets. Sets may represent real physical objects such
 *	parts in an assembly, materials and slide interfaces. And, sets may be related to other sets by set-algebraic equations
 *	involving union, intersection and difference.
 *
 *	A full description of modeling principles upon which SAF is based is outside this scope of this programmer's reference
 *	manual. User quality tutorials of this material will be forthcoming as SAF evolves. However, the reader should pause for
 *	a moment and confirm in his own mind just how general the notions of field and set are in describing scientific data. The
 *	columns of an Excel spreadsheet are fields. A time history is a field. The coordinates of a mesh is a field. A plot dump
 *	is a whole bunch of related fields. An image is a field. A video is a field. A load curve is a field. Likewise for sets.
 *	An individual node or zone is a set. A processor domain is a set. An element block is a set. A slide line or surface is a
 *	set. A part in an assembly is a set. And so on.
 *
 *	Understanding and applying set, field and relation primitives to model scientific data represents a revolutionary
 *	departure from previous, menu based approaches. SAF represents a first cut at a portable, parallel, high performance
 *	application programming interface for modeling scientific data. Over the course of development of SAF, the organizations
 *	involved have seen the value in applying this technology in several directions...
 *
 *	a) A publish/subscribe scenario for exchanging data between scientific computing clients, in-situ.
 *
 *	b) End-user tools for performing set operations and /restricting/ fields to subsets of the base space to
 *	take a closer look at portions of tera-scale data.
 *
 *	c) Operators which /transform/ data during exchange between clients such as changing the processor decomposition,
 *	evaluation method, node-order over elements, units, precision, etc. on a field.
 *
 *	d) Data consistency checkers which confirm a given bunch of scientific data does indeed conform to the
 *	mathematical and physical description that has been ascribed to it by its model. For example, that a volume or mass
 *	fraction field is indeed between 0.0 and 1.0, everywhere in its base-space.
 *	
 *	e) MPI-like parallel communication routines pitched in terms of /sets/ and /fields/ rather than data structures.
 *
 * 	And many others.
 *
 *	While each of these areas shows promise, our first goal has been to demonstrate that we can apply this technology
 *	to do the same job we previously achieved with mesh-object I/O libraries like Silo and Exodus II. In other words,
 *	our first and foremost goal is to demonstrate that we can read and write shareable scientific data files with
 *	good performance. Such a capability is fundamental to the success of any organization involved in scientific computing. 
 *	If we cannot demonstrate that, there is little point in trying to address these other areas of interest.
 *--------------------------------------------------------------------------------------------------------------------------------
 */

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Version Numbers
 *              
 * Description: The SAF source code has various version numbers assigned to parts of the system: source files, header files,
 *              library, API definition, and database files.
 *
 *              Source file versions are identical to CVS file revision numbers. These numbers are *not* stored in the source
 *              file but rather maintained by CVS. (We don't store them in the source file because it makes it more difficult
 *              to synchronize local and remote source trees since a `cvs commit' would modify all the source files.)  We use
 *              CVS in such a way that the main branch always contains the latest development version of SAF. When a public
 *              release is about to occur a new branch is created, version numbers are adjusted on both branches, and
 *              development stops on the new branch.
 *
 *              The header files and library each have a four-part version number: /major/, /minor/, /patch/, and /comment/.
 *              The version number of the header files must exactly match the version number of the library, or the library
 *              will refuse to operate. The major number is incremented only if the API changes in a way which is not backward
 *              compatible. The increment happens when the development branch is split to produce a new release branch, and the
 *              minor number is set to zero or one (depending on branch) and the patch number is reset to zero.  The minor
 *              number is incremented each time the main branch is split to produce a release branch. The minor number is
 *              always even on a release branch and odd on the development branch (the latest development version minor number
 *              is one greater than the latest release version).  The patch number is incremented each time bugs are fixed on
 *              the release branch, or each time a snapshot is produced on the development branch.  The comment is a character
 *              string indicating the scope of the release and is the empty string for all public releases and snapshots.
 *              Library version numbers are printed as /i.j.k-c/ where /i/ is the major number, /j/ is the minor number,
 *              /k/ is the patch number, and /-c/ is the comment string (the hyphen is printed only if the comment string is
 *              non-empty.
 *
 *              The API definition has a two-part version number which is the same as the major and minor version numbers of
 *              the header files and library. For any given release or snapshot the library must implement the corresponding
 *              version of the API. The API may document certain features as "not yet implemented".
 *
 *              Database files will contain the library version number as an attribute named "SAF" attached to the group
 *              containing the VBT files. The attribute will be of compound type and contain all global SAF metadata.
 *
 *              *Standard*Comment*Strings*: The comment string for all development versions which have not yet passed the
 *              snapshot operation will be `devel'. When the main branch is split to create a release branch the comment string
 *              on the release branch will be cleared. Pre-releases will then be created from the release branch while holding
 *              the patch number at zero so the release can be tested by the developers. Such prereleases will be commented as
 *              `preN' where /n/ is a number beginning at zero.  When a prerelease passes all developer tests the comment will
 *              be removed or changed to `beta'.
 *              
 *              Almost all programs call saf_init() and/or saf_open_database() in order to do something useful. So we've chosen
 *              to wrap those functions in macros which also make a reference to a global variable whose name is derived from
 *              the SAF version number. This variable is declared in the SAF library so that if an application is compiled with
 *              SAF header files which have a different version than the SAF library a link-time error will result. A version
 *              mismatch will result in an error similar to `undefined reference to SAF_version_0_1_0' from the linker.
 * */


/* Audience:    Public
 * Chapter:     Version Numbers
 * Purpose:     Major version number
 * Concepts:    Version number
 *
 * Description: The major version number of the SAF header files. If this number is not equal to the major version number of
 *              the SAF library with which the application was linked then the library will raise an error. */
#define SAF_VERSION_MAJOR       2

/* Audience:    Public
 * Chapter:     Version Numbers
 * Purpose:     Minor version number
 * Concepts:    Version number
 *
 * Description: The minor version number of the SAF header files. If this number is not equal to the minor version number of
 *              the SAF library with which the application was linked then the library will raise an error. */
#define SAF_VERSION_MINOR       0

/* Audience:    Public
 * Chapter:     Version Numbers
 * Purpose:     Release number
 * Concepts:    Version number
 *
 * Description: The patch number of the SAF header files. If this number is not equal to the patch number of
 *              the SAF library with which the application was linked then the library will raise an error. */
#define SAF_VERSION_RELEASE     3

/* Audience:    Public
 * Chapter:     Version Numbers
 * Purpose:     Version Annotation
 * Concepts:    Version number
 *
 * Description: The version annotation of the SAF header files. This indicates a restriction of the release (such as `beta'). */
#define SAF_VERSION_ANNOT     ""

/* Audience:    Public
 * Chapter:     Version Numbers
 * Purpose:     Version-dependent variable
 * Concepts:    Version number
 *
 * Description: This is simply a global integer variable whose name depends somehow on the SAF version numbers defined above.
 *              It is used to check at link-time whether the header files used by an application match the SAF library version
 *              number to which the application is linked. */
#define SAF_VERSION_VAR         SAF_version_2_0_3
extern int SAF_VERSION_VAR;

#ifdef HAVE_PARALLEL
/* Audience:    Public
 * Chapter:     Version Numbers
 * Purpose:     Serial/Parallel-dependent variable
 * Concepts:    Version number
 *
 * Description: This is simply a global integer variable whose name depends somehow on whether the library is being compiled
 *              for serial or parallel. It is used to check at link-time whether the header files used by an application match
 *              the SAF library to which the application is linked. */
#define SAF_PARALLEL_VAR        SAF_parallel_mode
#else
/*DOCUMENTED*/
#define SAF_PARALLEL_VAR        SAF_serial_mode
#endif
extern int SAF_PARALLEL_VAR;


/*---------------------------------------------------------------------------------------------------------------------------------
 * Note:	Constants 
 * Description:	Many constants defined in saf.h have similar limitations to some C programming language constants
 *		such as 'stderr' and 'stdout'. If you have a file-scope static variable initialized to stderr or
 *		stdout, you will find that your variable gets initialized with garbage. In C, the reason is that these
 *		constants don't really get defined *until*run-time* and when you initialize a file-scope static, you are
 *		relying upon it having been defined *at*compile*time*.
 *
 *		The same is true for many of SAF's constants. Therefore, you should take care *not*to*use*them* in a manner which
 *		assumes they are defined at compile time. We have made an effort to denote all such constants in the reference
 *		manual so that you can easily determine for which constants this is true.
 *
 *		Some constants require a database argument. This means the constant is not defined except within the scope of
 *		an open database. Thus, these constants are even more restricted in use than those that can be used at any
 *		run-time.
 *
 *		In summary, there are three classes of constants. Compile-time constants can be used anywhere. Run-time
 *		constants can be used only after the code has begun executing. Database-time constants can be used
 *		only after a database has been opened.
 *---------------------------------------------------------------------------------------------------------------------------------
 */


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling
 * Purpose:     Error codes returned by the library
 *
 * Description: These C preprocessor symbols define an integer bitmask where each bit represents an error condition.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef int SAF_error_t;
#define SAF_FATAL_ERROR           0x00000001    /* Any fatal error.                                                             */
#define SAF_MEMORY_ERROR          0x00000002    /* A memory-related error.                                                      */
#define SAF_FILE_ERROR            0x00000004    /* File-related errors.                                                         */
#define SAF_CONTEXT_ERROR         0x00000008    /* Context errors.                                                              */
#define SAF_LOOKUP_ERROR          0x00000010    /* Name lookup errors.                                                          */
#define SAF_MAPPING_ERROR         0x00000020    /* Mapping errors.                                                              */
#define SAF_WRITE_ERROR           0x00000040    /* File write errors.                                                           */
#define SAF_DEBUG_ERROR           0x00000080    /* Debugging messages.                                                          */
#define SAF_CONSTRAINT_ERROR      0x00000100    /* Failed constraints.                                                          */
#define SAF_PARAMETER_ERROR       0x00000200    /* Function parameter errors.                                                   */
#define SAF_COMMUNICATION_ERROR   0x00000400    /* MPI-related errors.                                                          */
#define SAF_READ_ERROR            0x00000800    /* File read errors.                                                            */
#define SAF_NOTIMPL_ERROR         0x00001000    /* Functionality has not been implemented.                                      */
#define SAF_BADHNDL_ERROR         0x00002000    /* Object handle errors.                                                        */
#define SAF_MISC_ERROR            0x00004000    /* Miscellaneous errors.                                                        */
#define SAF_SIZE_ERROR            0x00008000    /* Size-related errors.                                                         */
#define SAF_PMODE_ERROR           0x00010000    /* Errors in the parallel mode argument to a function.                          */
#define SAF_ASSERTION_ERROR       0x00020000    /* Failed assertions.                                                           */
#define SAF_PRECONDITION_ERROR    0x00040000    /* Failed preconditions.                                                        */
#define SAF_POSTCONDITION_ERROR   0x00080000    /* Failed postconditions.                                                       */
#define SAF_GENERIC_ERROR         0x00100000    /* Generic errors.                                                              */
#define SAF_SSLIB_ERROR           0x00200000    /* SSlib related errors                                                         */


/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Predefined scalar datatypes
 *
 * Description: These C preprocessor symbols represent various SAF predefined scalar datatypes. (See Constants)
 *
 * Issue:       These constants are mostly for backward compatibility. SAF now uses HDF5's datatype interface instead of the
 *              DSL interface. Applications should eventually switch over to HDF5 constants.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef int SAF_type_t;
#define SAF_CHAR        H5T_NATIVE_CHAR         /* Character datatype. */
#define SAF_INT         H5T_NATIVE_INT          /* Integer datatype. */
#define SAF_LONG        H5T_NATIVE_LONG         /* Long integer datatype. */
#define SAF_FLOAT       H5T_NATIVE_FLOAT        /* Single-precision floating-point datatype. */
#define SAF_DOUBLE      H5T_NATIVE_DOUBLE       /* Double-precision floating-point datatype. */
#define SAF_HANDLE      ss_pers_tm              /* Object handle datatype. */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Field component interleave modes
 *
 * Description:	When fields have multiple components, the components can be stored in the field's blob in different ways
 *		relative to each other. For example, in a 3D coordinate field, we will have 3 components for the x, y and
 *		z components of each coordinate. These can be stored as three different component fields or as a single
 *		composite field. If they are stored as a single composite field, they may be stored interleaved or 
 *		non-interleaved.
 *
 *              The SAF_INTERLEAVE_* constants are defined by the ss_interleave_t enumeration type. In addition we define
 *              aliases SAF_BLOCKED and SAF_INTERLEAVED.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef ss_interleave_t            SAF_Interleave;
#define SAF_BLOCKED                SAF_INTERLEAVE_COMPONENT	/* An alias for SAF_INTERLEAVE_COMPONENT. */
#define SAF_INTERLEAVED            SAF_INTERLEAVE_VECTOR	/* An alias for SAF_INTERLEAVE_VECTOR. */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Cell types
 *
 * Description:	Primitive cell types for collections. These are defined by the ss_celltype_t datatype.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef ss_celltype_t SAF_CellType;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Topological dimensions
 *
 * Description: These are really just more informative aliases for the numbers 0, 1, 2 and 3 so that when these are seen
 *		in saf function calls, the purpose of the argument will be more clear.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum {
   SAF_TOPODIM_0D = 0,	/* a zero dimensional topological dimension (e.g. a point) */
   SAF_TOPODIM_1D = 1,	/* a one dimensional topological dimension (e.g. a curve) */
   SAF_TOPODIM_2D = 2,	/* a two dimensional topological dimension (e.g. a surface) */
   SAF_TOPODIM_3D = 3	/* a three dimensional topological dimension (e.g. a volume) */
} SAF_TopoDim;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Subset inclusion lattice roles
 *
 * Description: Every subset inclusion lattice defines pieces of some all-encompassing space in which those pieces live.
 *		For example, the lattice may be specifying pieces of the time-base, or pieces of space, or pieces of
 *		some user defined parameter space.
 *
 *		In future versions of SAF, this information will be supplanted by the quantity associated with the
 *		coordinate field for a given base space.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef ss_silrole_t SAF_SilRole;
#define SAF_TIME                SAF_SROLE_TIME		/* For sets specifying pieces of time */
#define SAF_SPACE               SAF_SROLE_SPACE		/* For sets specifying pieces of space */
#define SAF_PARAM               SAF_SROLE_PARAM		/* For sets specifying pieces of some arbitrary, user defined parameter
							 * space */
#define SAF_SUITE		SAF_SROLE_SUITE		/* for sets specifying whole suites */
#define SAF_ANY_SILROLE         SAF_SROLE_ANY           /* Wildcard role for searching */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields 
 * Purpose:    	More meaningful alias for SAF_TOTALITY
 *
 * Description: The SAF_TOTALITY subset relation representation is most often only ever used during a saf_write_field() call
 *		to indicate the entire field is being written rather than just a portion. However, in that context, the meaning
 *		of a /totality/ is obscured. So, we provide SAF_WHOLE_FIELD as a more meaningful alias for that value. In
 *		addition, this macro replaces the three args, MEMBER_COUNT, REQUEST_TYPE, MEMBER_IDS, used in a partial
 *		saf_write_field() call. 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_WHOLE_FIELD		-1, SAF_TOTALITY, NULL

/*
 * Note:	Return Values
 * Description: Most SAF functions return SAF_SUCCESS when successful. Upon failure, the function either returns one of the
 *		error codes (see SAF_error_t) or throws an exception.
 */
/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Return codes
 *
 * Description: Not written yet.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef int SAF_return_t;
#define SAF_FAILURE      (-1)
#define SAF_SUCCESS      0

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Not implemented
 * Description: This is used in parts of the API that are not implemented yet.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_NOT_IMPL            NULL    

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Not applicable
 * Description: This is used for arguments of type int that aren't applicable in the current context
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_NOT_APPLICABLE_INT  (-1)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Compile Time Constants
 * Purpose:    	Set maximum number of dimensions for an array 
 *
 * Description:	Currently, to reduce complexity and I/O request fragmentation as well make the implementation easier, SAF deals
 *		with arrays of only a maximum number of dimensions. This constant specifies that maximum.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_MAX_NDIMS           SS_MAX_INDEXDIMS

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Wildcards for searching
 *
 * Description: In saf_find calls, the client may not want to limit the search to all of the available argument's values.
 *              SAF offers these wildcard values, all with the word /ANY/ in them, to pass as the value for an argument that
 *              the client does NOT wish to use in limiting a search. For example, see saf_find_matching_set().
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ANY_INT     INT_MAX
#define SAF_ANY_DOUBLE  (-1.e-64)
#define SAF_ANY_FLOAT   (-1.e-32)
#define SAF_ANY_TOPODIM SAF_ANY_INT
#define SAF_CELLTYPE_ANY ((SAF_CellType)(-1))
#define SAF_ANY_RATIO   SAF_ANY_INT
#define SAF_ANY_NAME    NULL
#define SAF_ANY_CAT     NULL

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling  
 * Purpose:     Begin a TRY/CATCH block 
 *
 * Description: Use this macro to demarcate the beginning of a block of code in which exceptions shall be caught.
 *
 * Issue:       We should be clear about what happens in this block if the library properties are set to SAF_ERRMODE_RETURN
 *              rather than SAF_ERRMODE_THROW.
 *              
 *              Should we add code here to check to make sure saf_init() is called first. I think that would make some sense.
 *
 * Programmer:  Tom Robey, SNL
 *              April 1, 1999 */
#define SAF_TRY_BEGIN                                                                 \
  _saf_place_cur++;                                                                   \
  if (_saf_place_max==_saf_place_cur)                                                 \
  {                                                                                   \
    if (_saf_place_max==0)                                                            \
      _saf_place=(jmp_buf *) calloc(10,sizeof(jmp_buf));                              \
    else                                                                              \
      _saf_place=(jmp_buf *) realloc(_saf_place,(_saf_place_max+10)*sizeof(jmp_buf)); \
    _saf_place_max+=10;                                                               \
  }                                                                                   \
  _saf_except=setjmp(_saf_place[_saf_place_cur]);                                     \
  if (_saf_except==0)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling  
 * Purpose:     Begin a the CATCH part of a TRY/CATCH block     
 *
 * Description: Use this macro to demarcate the beginning of the error catching portion of a TRY/CATCH block 
 *
 * Programmer:  Tom Robey, SNL
 *              April 1, 1999 */
#define SAF_CATCH else

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling  
 * Purpose:     Begin a block of error handling code
 *
 * Description: Use this macro to demarcate the beginning of a block of code that catches a specific error, ERR, in the
 *		error catching portion of a TRY/CATCH block
 *
 * Issue:       I am not sure I have confirmed that the catching does, in fact, fall through from a specific catch
 *              to a next specific catch or to the ALL case?
 *
 *              If we changed the SAF_CATCH_ERR macro to test the bit(s) of the argument, rather than equality
 *              we'd be able to catch several specific errors in that block.
 *
 * Programmer:  Tom Robey, SNL
 *              April 1, 1999 */
#define SAF_CATCH_ERR(err) if (_saf_except == err && _saf_check_catch())

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling  
 * Purpose:     Begin a block of error handling code for all errors
 *
 * Description: Use this macro to demarcate the beginning of a block of code that catches all errors in the
 *		error catching portion of a TRY/CATCH block
 *
 * Programmer:  Tom Robey, SNL
 *              April 1, 1999 */
#define SAF_CATCH_ALL if (_saf_check_catch())

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Error Handling  
 * Purpose:     End a TRY/CATCH block   
 *
 * Description: Use this macro to end a TRY/CATCH block
 *
 * Programmer:  Tom Robey, SNL
 *              April 1, 1999 */
#define SAF_TRY_END _saf_place_cur--;


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: Macros for dealing with common indexing schema (Fortran and C 1,2 and 3D arrays).
 *-------------------------------------------------------------------------------------------------------------------------------
 */
typedef int SAF_IndexSchema;
#define SAF_F_ORDER             -1
#define SAF_C_ORDER             -2

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: Fortran order array of N dimensions. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_FORDER(N)           _saf_fortran_order(N)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: C order array of N dimensions. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_CORDER(N)           _saf_c_order(N)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: One-dimensional C array of size NX. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_1DC(nx)             _saf_indexspec(1,nx,0,SAF_C_ORDER)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: Not applicable index scheme. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_NA_INDEXSPEC         _saf_indexspec_not_applicable()

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: Two-dimensional C array of size NX by NY. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_2DC(nx,ny)          _saf_indexspec(2,nx,ny,0,0,SAF_C_ORDER)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: Three-dimensional C array of size NX, NY, NZ elements. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_3DC(nx,ny,nz)       _saf_indexspec(3,nx,ny,nz,0,0,0,SAF_C_ORDER)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: One-dimensional Fortran array of size NX. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_1DF(nx)             _saf_indexspec(1,nx,1,SAF_F_ORDER)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: Two-dimensional Fortran array of size NX by NY. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_2DF(nx,ny)          _saf_indexspec(2,nx,ny,1,1,SAF_F_ORDER)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Indexing scheme
 * Description: Three-dimensional Fortran array of size NX, NY, NZ elements. (See Constants)
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_3DF(nx,ny,nz)       _saf_indexspec(3,nx,ny,nz,1,1,1,SAF_F_ORDER)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     NULL aliases
 * Description: A bunch of useful aliases for 'NULL'
 *-------------------------------------------------------------------------------------------------------------------------------
 */
typedef void *SAF_VoidPtr;
#define SAF_IDENTITY		NULL
#define SAF_NO_COMPONENTS	NULL
#define SAF_NO_DATA		NULL

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Reserved attribute name keys
 *
 * Description:
 *	There are some reserved attribute names. These reserved attribute names may be passed as the NAME argument in
 * 	any calls to get attributes (see saf_get_attribute()). The SAF_ATT_NAMES / SAF_ATT_COUNT pair of reserved names provide
 *	a mechanism to the client to determine the count of attributes defined for a given object and their names. Or,
 *	alternatively, the SAF_ATT_FIRST / SAF_ATT_NEXT provide a mechanism for the client to make repetitive calls to iterate
 *	through the attributes for a given object.
 *
 * Modifications:
 *	Mark C. Miller, LLNL, 12-12-2000
 *	Added documentation
 *---------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_ATT_NAMES           ".saf_att_names"	/* If the client passes SAF_ATT_NAMES for the NAME arg in a call to
							 * saf_get_attribute(), SAF will return a TYPE of DSL_STRING (if the TYPE
							 * return value is requested), a COUNT equal to the number of attributes
							 * (if the COUNT return value was requested), and a VALUE array
							 * containing the names of all attributes defined for the object. */
#define SAF_ATT_COUNT           ".saf_att_count"	/* If the client passes SAF_ATT_COUNT for the NAME arg in a SAF call to 
							 * saf_get_attribute(), SAF will return the count of number of
							 * attributes defined for the given object in the COUNT. It is an error
							 * to request a count with SAF_ATT_COUNT, but pass NULL for the COUNT
							 * argument in a call to get attributes. */
#define SAF_ATT_FIRST           ".saf_att_first"	/* If the client passes SAF_ATT_FIRST, for the NAME argument in a SAF call
							 * to saf_get_attribute(), SAF will return the *first* attribute that was 
							 * ever defined for the object. Thereafter, any call with SAF_ATT_NEXT
							 * will iterate through the list of attributes defined for the object. */
#define SAF_ATT_NEXT            ".saf_att_next"		/* This reserved attribute name works in conjunction with SAF_ATT_FIRST,
							 * to allow the client to iterate through all attributes defined for 
							 * a given object. It is an error to pass SAF_ATT_NEXT without at least
							 * one prior call with SAF_ATT_FIRST. */

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sets
 * Purpose:     The universe set handle
 *
 * Description: This macro evaluates to the set handle for the universe set of the database (See Constants). */
#define SAF_UNIVERSE(Db)        (&SAF_UNIVERSE_SET_g)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Sets
 * Purpose:     The null set handle
 *              
 * Description: This macro evaluates to the set handle for the null set of the database. The null set handle is most often
 *              only used in a SAF_ONE parallel call where many processors are participating solely for the sake of collectivity
 *		(See Constants). */
#define SAF_NULL_SET(Db)        NULL

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Collection Categories
 * Purpose:     The self decomposition of a set
 *
 * Description: This macro evaluates to the collection category handle for the self decomposition of a set (See Constants).
 */
#define SAF_SELF(Db)            (&SAF_SELF_CAT_g)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Relations
 * Purpose:     The null relation handle
 *              
 * Description: This macro evaluates to the relation handle for the null relation of the database. The null relation handle is
 *              most often only used in a SAF_ONE parallel call where many processors are participating solely for the sake of
 *              collectivity (See Constants). */
#define SAF_NULL_REL(Db)        NULL

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Field Templates
 * Purpose:     The null field template handle
 *              
 * Description: This macro evaluates to the field template handle for the null field template of the database. The null field
 *              template handle is most often only used in a SAF_ONE parallel call where many processors are participating
 *              solely for the sake of collectivity (See Constants). */
#define SAF_NULL_FTMPL(Db)      NULL

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields
 * Purpose:     The null field handle
 *              
 * Description: This macro evaluates to the field handle for the null field of the database. The null field handle is most
 *              often only used in a SAF_ONE parallel call where many processors are participating solely for the sake of
 *              collectivity (See Constants). */
#define SAF_NULL_FIELD(Db)      NULL

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     State Templates
 * Purpose:     The null state template handle
 *              
 * Description: This macro evaluates to the state template handle for the null state template of the database. The null set
 *              handle is most often only used in a SAF_ONE parallel call where many processors are participating solely for the
 *              sake of collectivity (See Constants). */
#define SAF_NULL_STMPL(Db)      NULL

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     States
 * Purpose:     The null state group handle
 *              
 * Description: This macro evaluates to the state handle for the null state of the database. The null state handle is most
 *              often only used in a SAF_ONE parallel call where many processors are participating solely for the sake of
 *              collectivity (See Constants). */
#define SAF_NULL_STATE_GRP(Db)   NULL

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Suites
 * Purpose:     The null suite handle
 *              
 * Description: This macro evaluates to the suite handle for the null suite of the database. The null suite is most
 *              often only used in a SAF_ONE parallel call where many processors are participating solely for the sake of
 *              collectivity (See Constants). */
#define SAF_NULL_SUITE(Db)   NULL

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities 
 * Purpose:     Determine if two handles refer to the same object
 *              
 * Description: This macro returns true if the two object handles passed to it refer to the same object.  Otherwise, it returns
 *              false. */ 
#define SAF_EQUIV(A,B)          SS_PERS_EQ(A, B)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Determine if a handle is a valid handle
 *              
 * Description: This macro returns true if the handle passed to it is valid, that is, that its members define a legitimate
 *              handle. Otherwise, it returns false. */
#define SAF_VALID(A)            (SS_MAGIC(ss_pers_t)==SS_MAGIC_OF(A))

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields 
 * Purpose:     Conveniently specify a node-centered field
 * Concept:     Convenience, declaring fields; Fields, node-centered;  
 *
 * Description: This macro provides a convenient way to specify four of the args, /coeff_assoc/,/assoc_ratio/,/eval_coll/,
 *              and /eval_func/ of the saf_declare_field() call. Use it when you have what is often referred to as a
 *              /node/ /centered/ /field/. The argument N is meant to be a collection category representing collections of
 *              SAF_CELLTYPE_POINT (nodes) cells.  The argument Z is meant to be a collection category representing
 *              collections of element cells. */
#define SAF_NODAL(N, Z)         N, 1, Z, SAF_SPACE_PWLINEAR

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Fields 
 * Purpose:     Conveniently specify a zone-centered field
 * Concept:     Convenience, declaring fields; Fields, zone-centered;  
 *
 * Description: This macro provides a convenient way to specify four of the args, /coeff_assoc/,/assoc_ratio/,/eval_coll/,
 *              and /eval_func/ of the saf_declare_field() call. Use it when you have what is often referred to as a
 *              /zone/ /centered/ /field/. The argument Z is meant to be a collection category representing collections of element
 *              cells. */
#define SAF_ZONAL(Z)            Z, 1, Z, SAF_SPACE_PWCONST

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Fields 
 * Purpose:     Conveniently specify a decomposition-centered field
 * Concept:     Convenience, declaring fields; Fields, decomposition-centered;  
 *
 * Description: This macro provides a convenient way to specify four of the args, /coeff_assoc/,/assoc_ratio/,/eval_coll/,
 *              and /eval_func/ of the saf_declare_field() call. Use it when you have a field in which you have 1 degree of
 *              freedom for each set in a collection of sets forming a decomposition of their parent set. For example, if you
 *              have a collection of sets where each set represents one processor's piece and you wish to characterize a field
 *              that represents the min (or max) of some field over each piece. The argument D is meant to be a collection
 *              category for a non-primitive collection of set known to form a decomposition of the set upon which the field is
 *              being defined. */
#define SAF_DECOMP(D)           D, 1, D, SAF_SPACE_PWCONST

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Fields 
 * Purpose:     Conveniently specify a constant field
 * Concept:     Convenience, declaring fields; Fields, constant 
 *
 * Description: This macro provides a convenient way to specify four of the args, /coeff_assoc/,/assoc_ratio/,/eval_coll/,
 *              and /eval_func/ of the saf_declare_field() call. Use it when you have a constant field. The DB argument is meant
 *              to represent the database handle. See saf_declare_field() for a more detailed description. */ 
#define SAF_CONSTANT(db)        SAF_SELF(db), 1, SAF_SELF(db), SAF_SPACE_CONSTANT

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Subset Relations 
 * Purpose:     Conveniently specify a typical subset
 * Concept:     Convenience, declaring relations; Relations, typical subset
 *
 * Description: This macro provides a convenient way to specify four of the args, /sup_cat/,/sub_cat/,/sbmode/,
 *              and /cbmode/ of the saf_declare_subset_relation() call. Use it when you have a typical subset. The argument C is
 *              meant to be a collection category in common to both sup and sub sets. See saf_declare_subset_relation() for a
 *		more detailed description. */ 
#define SAF_COMMON(C)           C,C,SAF_BOUNDARY_FALSE,SAF_BOUNDARY_FALSE

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Subset Relations 
 * Purpose:     Conveniently specify a boundary subset
 * Concept:     Convenience, declaring relations; Relations, boundary subset
 *
 * Description: This macro provides a convenient way to specify four of the args, /sup_cat/,/sub_cat/,/sbmode/,
 *              and /cbmode/ of the saf_declare_subset_relation() call. Use it when the subset is the boundary of the superset.
 *              The arguments P and B represent collection categories of collections on superset and subset, respectively. See
 *		saf_declare_subset_relation() for a more detailed description. */ 
#define SAF_BOUNDARY(P,B)       P,B,SAF_BOUNDARY_TRUE,SAF_BOUNDARY_TRUE

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Subset Relations 
 * Purpose:     Conveniently specify an embedded boundary subset
 * Concept:     Convenience, declaring relations; Relations, embedded boundary subset
 *
 * Description: This macro provides a convenient way to specify four of the args, /supcat/,/sub_cat/,/sbmode/,
 *              and /cbmode/ of the saf_declare_subset_relation() call. Use it when the subset is some internal boundary in the
 *              superset but is NOT the boundary of the superset. The arguments P and B represent collection categories of collections
 *              on superset and subset, respectively. */ 
#define SAF_EMBEDBND(P,B)       P,B,SAF_BOUNDARY_FALSE,SAF_BOUNDARY_TRUE

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Subset Relations 
 * Purpose:     Conveniently specify an general subset
 * Concept:     Convenience, declaring relations; Relations, general subset
 *
 * Description: This macro provides a convenient way to specify four of the args, /sup_cat/,/sub_cat/,/sbmode/,
 *              and /cbmode/ of the saf_declare_subset_relation() call. Use it when all that is known is that the subset is indeed
 *              a subset of the superset, but the details of their relationship are unknown (e.g. no data). The argument BND is a
 *              boolean meant to indicate if the subset is the boundary of the superset. */
#define SAF_GENERAL(BND)        NULL,NULL,BND,SAF_BOUNDARY_FALSE

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Synchronization barrier 
 *
 * Description: A macro which causes all processors in the communicator used to open the database or, if DB is NULL, to
 *              initialize the library, to wait until all reach this point (See Constants). */
#define SAF_BARRIER(Db)         _saf_barrier(Db)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities
 * Purpose:     The rank of the calling process
 *
 * Description: A macro which evaluates to the MPI_Rank of the calling processor in the communicator used to open the
 *              database. If NULL is passed for the DB argument, the MPI_Rank of the calling process in the communicator used
 *              to initialize the library is returned. In serial, a value of 0 is returned. If not called within an enclosing
 *              pair of saf_init() / saf_final() calls, the value -1 is returned (See Constants). */
#define SAF_RANK(Db)            _saf_rank(Db)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities
 * Purpose:     The size of the communicator
 *
 * Description: A macro which evaluates to the MPI_Size of the communicator used to open the database. If NULL is passed for
 *              the DB argument, the MPI_Size of the communicator used to initialize the library is returned. In serial
 *              a value of 1 is returned. If not called within an enclosing pair of saf_init() / saf_final() calls,
 *              the value -1 is returned (See Constants). */
#define SAF_SIZE(Db)            _saf_size(Db)

/*--------------------------------------------------------------------------------------------------------------------------------
 * Note:	Parallel Modes
 * Description: In many SAF calls, the first argument is a SAF_ParMode_t argument for the parallel mode in which the call 
 *		is to be executed. By and large, SAF's API operates like a SIMD machine. All processor always participate
 *		to do something. The possible parallel modes are SAF_ALL, SAF_EACH, and SAF_ONE(rank).
 *
 *		In SAF_ALL mode, all processors participate to operate on a single, globally common object
 *
 *		In SAF_EACH mode, all processors participate to operate on /nprocs/, objects each one local to that
 *		processor.
 *
 *		In SAF_ONE(root) mode, all processors participate to allow one processor, the /root/, to operate on
 *		one object.
 *--------------------------------------------------------------------------------------------------------------------------------
 */
typedef int SAF_ParMode;
#define SAF_ALL                 (-1)
#define SAF_EACH                (-2)
#define SAF_ONE(Root_Proc)      (Root_Proc)

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Description: These set of macros provide a slightly more convenient mechanism for allocating fixed arrays of
 *		handles on the stack as C-automatics that can be used in SAF calls that return multiple handles.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Purpose:     Make a C-automatic array of cat handles
 * Concept:     Convenience, object handles; Object handles; Categories
 *
 * Description: This macro puts the memory for an array of cat object handles on the stack as an automatic variable rather
 *              than having the library or client worry about allocating or freeing it. NAME is the name of the array
 *		returning an array of object handles. */
#define SAF_Cat(name, p_name, n)                                                \
        SAF_Cat name [ n ];                                               \
        SAF_Cat * p_name = name

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Purpose:     Make a C-automatic array of set handles
 * Concept:     Convenience, object handles; Object handles; Sets
 *
 * Description: This macro puts the memory for an array of set object handles on the stack as an automatic variable rather
 *              than having the library or client worry about allocating or freeing it. NAME is the name of the array
 *		returning an array of object handles. */
#define SAF_Set(name, p_name, n)                                                \
        SAF_Set name [ n ];                                               \
        SAF_Set * p_name = name

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Purpose:     Make a relation handle a C automatic variable
 * Concept:     Convenience, object handles; Object handles; Relations 
 *
 * Description: This macro puts the memory for a relation object handle on the stack as an automatic variable rather
 *              than having the library or client worry about allocating or freeing it. */
#define SAF_Rel(name, p_name, n)                                                \
        SAF_Rel name [ n ];                                               \
        SAF_Rel * p_name = name

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Purpose:     Make a field template handle a C automatic variable
 * Concept:     Convenience, object handles; Object handles; Field Templates 
 *
 * Description: This macro puts the memory for a field template object handle on the stack as an automatic variable rather
 *              than having the library or client worry about allocating or freeing it. */
#define SAF_FieldTmpl(name, p_name, n)                                          \
        SAF_FieldTmpl name [ n ];                                         \
        SAF_FieldTmpl * p_name = name

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Purpose:     Make a field handle a C automatic variable
 * Concept:     Convenience, object handles; Object handles; Fields
 *
 * Description: This macro puts the memory for a field object handle on the stack as an automatic variable rather
 *              than having the library or client worry about allocating or freeing it. */
#define SAF_Field(name, p_name, n)                                              \
        SAF_Field name [ n ];                                             \
        SAF_Field * p_name = name

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Purpose:     Make a state template handle a C automatic variable
 * Concept:     Convenience, object handles; Object handles; State Templates
 *
 * Description: This macro puts the memory for a state template object handle on the stack as an automatic variable rather
 *              than having the library or client worry about allocating or freeing it. */
#define SAF_StateTmpl(name, p_name, n)                                          \
        SAF_StateTmpl name [ n ];                                         \
        SAF_StateTmpl * p_name = name


/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Purpose:     Make a state handle a C automatic variable
 * Concept:     Convenience, object handles; Object handles; States
 *
 * Description: This macro puts the memory for a state object handle on the stack as an automatic variable rather
 *              than having the library or client worry about allocating or freeing it. */
#define SAF_StateGrp(name, p_name, n)                                              \
        SAF_StateGrp name [ n ];                                             \
        SAF_StateGrp * p_name = name

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     C-Automatic Handle Allocation
 * Purpose:     Make a suite handle a C automatic variable
 * Concept:     Convenience, object handles; Object handles; Sequences 
 *
 * Description: This macro puts the memory for a suite object handle on the stack as an automatic variable rather
 *              than having the library or client worry about allocating or freeing it. */
#define SAF_Suite(name, p_name, n)                                                \
        SAF_Suite name [ n ];                                               \
        SAF_Suite * p_name = name

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Standard tri-state values
 *
 * Description:	In many portions of SAF's API, there are boolean values to indicate if a particular feature of an object
 *		is true or false. In addition, it is possible to invoke searches using saf_find... kinds of functions
 *		that will search for objects for which the given boolean feature is true or false or either. So,
 *		we've defined a standard tri-state enumeration for these three cases.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef htri_t SAF_TriState;
#define SAF_TRISTATE_FALSE      0
#define SAF_TRISTATE_TRUE       1
#define SAF_TRISTATE_TORF       (-1)

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Extendable set tri-state
 *
 * Description: To make each function call made by a client a little more self-documenting, we provide specific tri-state
 *		tags to represent the meaning of that particular boolean. The one here is used to indicate whether a set 
 *		is extendible or not. See saf_declare_set() for more information.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum {
   SAF_EXTENDIBLE_FALSE = SAF_TRISTATE_FALSE,
   SAF_EXTENDIBLE_TRUE  = SAF_TRISTATE_TRUE,
   SAF_EXTENDIBLE_TORF  = SAF_TRISTATE_TORF
} SAF_ExtendMode;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Boundary set tri-state
 *
 * Description: To make each function call made by a client a little more self-documenting, we provide specific tri-state
 *		tags to represent the meaning of that particular boolean. The one here is used to indicate whether a one
 *		set in a subset relation is /the/ boundary of another set. See saf_declare_subset_relation() for more information.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum {
   SAF_BOUNDARY_FALSE = SAF_TRISTATE_FALSE,
   SAF_BOUNDARY_TRUE  = SAF_TRISTATE_TRUE,
   SAF_BOUNDARY_TORF  = SAF_TRISTATE_TORF
} SAF_BoundMode;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Decomposition tri-state
 *
 * Description: To make each function call made by a client a little more self-documenting, we provide specific tri-state
 *		tags to represent the meaning of that particular boolean. The one here is used to indicate whether a given
 *		collection is a decomposition of its containing set.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum {
   SAF_DECOMP_FALSE = SAF_TRISTATE_FALSE,
   SAF_DECOMP_TRUE  = SAF_TRISTATE_TRUE,
   SAF_DECOMP_TORF  = SAF_TRISTATE_TORF
} SAF_DecompMode;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Top mode tri-state
 *
 * Description: To make each function call made by a client a little more self-documenting, we provide specific tri-state
 *		tags to represent the meaning of that particular boolean. The one here is used to limit a search to top
 *		level sets in a saf_find_matching_sets() call.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum {
   SAF_TOP_FALSE = SAF_TRISTATE_FALSE,
   SAF_TOP_TRUE  = SAF_TRISTATE_TRUE,
   SAF_TOP_TORF  = SAF_TRISTATE_TORF
} SAF_TopMode;

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Datatypes
 * Purpose:     Set find modes
 *
 * Description: These are the possible modes that saf_find_set() can operate in. 
 *---------------------------------------------------------------------------------------------------------------------------------
 */
typedef enum {
   SAF_FSETS_TOP=1,	/* find the top-level from the given set */
   SAF_FSETS_BOUNDARY,	/* find the boundary of the given set */
   SAF_FSETS_SUBS,	/* find the immediate subsets of the given set */
   SAF_FSETS_SUPS,	/* find the immediate supersets of the given set */
   SAF_FSETS_LEAVES	/* find all the bottom most sets in the tree rooted at the given set */
} SAF_FindSetMode;

/* indexing specification (not a database "object") */
typedef struct {
    int                 ndims;
    int                 sizes[SAF_MAX_NDIMS];
    int                 origins[SAF_MAX_NDIMS];
    int                 order[SAF_MAX_NDIMS];
} SAF_IndexSpec;

/* version information packet */
typedef struct {
    int			vmajor;
    int			vminor;
    int			release;
    char		annot[8];
} SAF_VersionInfo;

/* This struct holds all information needed for field targeting. */
typedef struct {
    hbool_t             is_set;                 /* Is targeting in effect? Are any of the members below set? */
    ss_unit_t           units;
    ss_cat_t            decomp;
    ss_cat_t            coeff_assoc;
    int                 assoc_ratio;
    ss_cat_t            eval_coll;
    ss_evaluation_t     func;
    hid_t               data_type;
    ss_interleave_t     comp_intlv;
    int                 *comp_order;
} SAF_FieldTarget;

/* This struct holds all information needed for relation targeting */
typedef struct {
    hbool_t             is_set;                 /* Is targeting in effect? Are any of the members below set? */
    ss_set_t            range_set;
    ss_cat_t            range_cat;
    ss_cat_t            decomp;
    hid_t               data_type;
} SAF_RelTarget;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Bind a SAF datatype to an SSlib datatype
 * Description: This macro binds a SAF datatype such as SAF_Eval to an SSlib datatype such as ss_evaluation_t. The SSlib
 *              datatype is specified without the leading "ss_" or trailing "_t".
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#define SAF_BIND_SSLIB(saftype,sstype)                                                                                         \
    typedef ss_##sstype##_t saftype;                                                                                           \
    typedef ss_##sstype##obj_t saftype##Obj

/* Object handles */
#include <SAFlibprops.h>                                /* Libprops class */
#include <SAFdbprops.h>                                 /* Dbprops class */
#include <SAFdb.h>                                      /* Database class */
#include <SAFquant.h>                                   /* Quantity class */
#include <SAFunit.h>                                    /* Unit class */
#include <SAFrole.h>                                    /* Role class */
#include <SAFbasis.h>                                   /* Basis class */
#include <SAFalgebraic.h>                               /* Algebraic class */
#include <SAFevaluation.h>                              /* Evaluation class */
#include <SAFrelrep.h>                                  /* Topology relation representation class */

/* These types are here because we don't (yet) have a SAF*.h file for them */
SAF_BIND_SSLIB(SAF_Cat, cat);
#define SAF_MAGIC_SAF_Cat SS_MAGIC(ss_cat_t)
SAF_BIND_SSLIB(SAF_Set, set);
#define SAF_MAGIC_SAF_Set SS_MAGIC(ss_set_t)
SAF_BIND_SSLIB(SAF_Rel, rel);
#define SAF_MAGIC_SAF_Rel SS_MAGIC(ss_rel_t)
SAF_BIND_SSLIB(SAF_FieldTmpl, fieldtmpl);
#define SAF_MAGIC_SAF_FieldTmpl SS_MAGIC(ss_fieldtmpl_t)
SAF_BIND_SSLIB(SAF_Field, field);
#define SAF_MAGIC_SAF_Field SS_MAGIC(ss_field_t)
SAF_BIND_SSLIB(SAF_Collection, collection);
#define SAF_MAGIC_SAF_Collection SS_MAGIC(ss_collection_t)

/* IndexSpec is weird: VBT had an `indexspec' table but a SAF_IndexSpec is a non-database object while SAF_AltIndexSpec is
 * what was stored in VBT's indexspec table. */
SAF_BIND_SSLIB(SAF_AltIndexSpec, indexspec);
#define SAF_MAGIC_SAF_AltIndexSpec SS_MAGIC(ss_indexspec_t)

/* Other stuff */
#include <SAFinit.h>                                    /* Library initialization */
#include <SAFinfo.h>					/* path info queries */



/*--------------------------------------------------------------------------------------------------------------------------------
 * Audience:   	Private 
 * Chapter:    	Datatypes
 * Concepts:	Datatypes; Version Number
 *              
 * Description: Every SAF database consists of at least one scope. Each time a scope is created an HDF5 group is created to
 *              hold the persistent object tables of that scope. The group will be given an attribute named "version" whose
 *              type will be an HDF5 compound datatype with members "saf", "hdf5", "mpid", etc. as appropriate. Each such
 *              member will itself be a compound datatype with information about the available parts of the version number in
 *              that layer.
 *--------------------------------------------------------------------------------------------------------------------------------
 */


/* state template handle; a state template is a specializatino of a field template */
typedef  SAF_FieldTmpl SAF_StateTmpl;

/* state group handle; a state group is a specialization of a field */
typedef  SAF_Field     SAF_StateGrp;

/* suite handle; a suite is a specialization of a set */
typedef  SAF_Set       SAF_Suite;

/* ----------------------------------
  
              Globals 
  
   ---------------------------------- */

/* for exception handling macros defined in error.c */
extern jmp_buf *_saf_place;
extern int _saf_place_max;
extern int _saf_place_cur;
extern int _saf_except;
extern int _saf_caught;

extern SAF_Set          SAF_UNIVERSE_SET_g;
extern SAF_Cat          SAF_SELF_CAT_g;

/* global handles to indicate the following:
 * NOT_SET -- feature is left unspecified
 * NOT_APPLICABLE -- feature does not apply in the present context
 * NOT_IMPLEMENTED -- feature is not yet implemented
 * ERROR--handle to indicate function error return
 * These are read only handles--a client should never pass one of
 * these to a function that will modify its contents. */

/* These indicate that a feature is left unspecified. They are simply non-null pointers to persistent object links which in
 * turn are all zero. That is, a valid nil link. */
#define SAF_NOT_SET_FIELD               (&SS_FIELD_NULL)
#define SAF_NOT_SET_ROLE                (&SS_ROLE_NULL)
#define SAF_NOT_SET_BASIS               (&SS_BASIS_NULL)
#define SAF_NOT_SET_ALGEBRAIC           (&SS_ALGEBRAIC_NULL)
#define SAF_NOT_SET_EVALUATION          (&SS_EVALUATION_NULL)
#define SAF_NOT_SET_RELREP              (&SS_RELREP_NULL)
#define SAF_NOT_SET_QUANTITY            (&SS_QUANTITY_NULL)
#define SAF_NOT_SET_UNIT                (&SS_UNIT_NULL)
#define SAF_NOT_SET_CAT                 (&SS_CAT_NULL)
#define SAF_NOT_SET_SET                 (&SS_SET_NULL)
#define SAF_NOT_SET_COLLECTION          (&SS_COLLECTION_NULL)
#define SAF_NOT_SET_REL                 (&SS_REL_NULL)
#define SAF_NOT_SET_TOPS                (&SS_TOPS_NULL)
#define SAF_NOT_SET_BLOB                (&SS_BLOB_NULL)
#define SAF_NOT_SET_INDEXSPEC           (&SS_INDEXSPEC_NULL)

/* These indicate that a feature does not apply in the present context. The global variables are initialized in
 *_ saf_gen_stdtypes() called by _saf_init(). */
#define SAF_NOT_APPLICABLE_FIELD        (&SAF_NOT_APPLICABLE_FIELD_g)
extern SAF_Field                        SAF_NOT_APPLICABLE_FIELD_g;
#define SAF_NOT_APPLICABLE_ROLE         (&SAF_NOT_APPLICABLE_ROLE_g)
extern SAF_Role                         SAF_NOT_APPLICABLE_ROLE_g;
#define SAF_NOT_APPLICABLE_BASIS        (&SAF_NOT_APPLICABLE_BASIS_g)
extern SAF_Basis                        SAF_NOT_APPLICABLE_BASIS_g;
#define SAF_NOT_APPLICABLE_ALGEBRAIC    (&SAF_NOT_APPLICABLE_ALGEBRAIC_g)
extern SAF_Algebraic                    SAF_NOT_APPLICABLE_ALGEBRAIC_g;
#define SAF_NOT_APPLICABLE_EVALUATION   (&SAF_NOT_APPLICABLE_EVALUATION_g)
extern SAF_Eval                         SAF_NOT_APPLICABLE_EVALUATION_g;
#define SAF_NOT_APPLICABLE_RELREP       (&SAF_NOT_APPLICABLE_RELREP_g)
extern SAF_RelRep                       SAF_NOT_APPLICABLE_RELREP_g;
#define SAF_NOT_APPLICABLE_QUANTITY     (&SAF_NOT_APPLICABLE_QUANTITY_g)
extern SAF_Quantity                     SAF_NOT_APPLICABLE_QUANTITY_g;
#define SAF_NOT_APPLICABLE_UNIT         (&SAF_NOT_APPLICABLE_UNIT_g)
extern SAF_Unit                         SAF_NOT_APPLICABLE_UNIT_g;
#define SAF_NOT_APPLICABLE_CAT          (&SAF_NOT_APPLICABLE_CAT_g)
extern SAF_Cat                          SAF_NOT_APPLICABLE_CAT_g;
#define SAF_NOT_APPLICABLE_SET          (&SAF_NOT_APPLICABLE_SET_g)
extern SAF_Set                          SAF_NOT_APPLICABLE_SET_g;
#define SAF_NOT_APPLICABLE_COLLECTION   (&SAF_NOT_APPLICABLE_COLLECTION_g)
extern ss_collection_t                  SAF_NOT_APPLICABLE_COLLECTION_g;
#define SAF_NOT_APPLICABLE_REL          (&SAF_NOT_APPLICABLE_REL_g)
extern SAF_Rel                          SAF_NOT_APPLICABLE_REL_g;
#define SAF_NOT_APPLICABLE_TOPS         (&SAF_NOT_APPLICABLE_TOPS_g)
extern ss_tops_t                        SAF_NOT_APPLICABLE_TOPS_g;
#define SAF_NOT_APPLICABLE_BLOB         (&SAF_NOT_APPLICABLE_BLOB_g)
extern ss_blob_t                        SAF_NOT_APPLICABLE_BLOB_g;
#define SAF_NOT_APPLICABLE_INDEXSPEC    (&SAF_NOT_APPLICABLE_INDEXSPEC_g)
extern ss_indexspec_t                   SAF_NOT_APPLICABLE_INDEXSPEC_g;

/* These indicate that a feature is not yet implemented.  They are deprecated because the affected function should fail
 * instead. */
#define SAF_NOT_IMPLEMENTED_FIELD        NULL
#define SAF_NOT_IMPLEMENTED_ROLE         NULL
#define SAF_NOT_IMPLEMENTED_BASIS        NULL
#define SAF_NOT_IMPLEMENTED_ALGEBRAIC    NULL
#define SAF_NOT_IMPLEMENTED_EVALUATION   NULL
#define SAF_NOT_IMPLEMENTED_RELREP       NULL
#define SAF_NOT_IMPLEMENTED_QUANTITY     NULL
#define SAF_NOT_IMPLEMENTED_UNIT         NULL
#define SAF_NOT_IMPLEMENTED_CAT          NULL
#define SAF_NOT_IMPLEMENTED_SET          NULL
#define SAF_NOT_IMPLEMENTED_COLLECTION   NULL
#define SAF_NOT_IMPLEMENTED_REL          NULL
#define SAF_NOT_IMPLEMENTED_TOPS         NULL
#define SAF_NOT_IMPLEMENTED_BLOB         NULL
#define SAF_NOT_IMPLEMENTED_INDEXSPEC    NULL

/* These are return values for failed functions. They are deprecated -- use NULL instead. */
#define SAF_ERROR_FIELD                  NULL
#define SAF_ERROR_ROLE                   NULL
#define SAF_ERROR_BASIS                  NULL
#define SAF_ERROR_ALGEBRAIC              NULL
#define SAF_ERROR_EVALUATION             NULL
#define SAF_ERROR_RELREP                 NULL
#define SAF_ERROR_QUANTITY               NULL
#define SAF_ERROR_UNIT                   NULL
#define SAF_ERROR_CAT                    NULL
#define SAF_ERROR_SET                    NULL
#define SAF_ERROR_COLLECTION             NULL
#define SAF_ERROR_REL                    NULL
#define SAF_ERROR_TOPS                   NULL
#define SAF_ERROR_BLOB                   NULL
#define SAF_ERROR_INDEXSPEC              NULL

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------
  
     Public API Function Prototypes 
  
   ---------------------------------- */


/*
 *********************************************************
 * utility functions
 *********************************************************
 */
/* many of these that are really private to the library are here, rather than
   safP.h, simply because the client indirectly references these functions through
   macros. */
SAF_IndexSpec _saf_indexspec(int ndims, ...);
void _saf_barrier(SAF_Db *db);
int _saf_rank(SAF_Db *db);
int _saf_size(SAF_Db *db);
char *saf_version_string(int verbose, char *buf, size_t bufsize);
ss_pers_t *saf_allgather_handles(ss_pers_t *pers, int *commsize, ss_pers_t *result);

/*
 *********************************************************
 * functions for handling error conditions
 *********************************************************
 */
extern char * saf_error_str ( void );
extern int _saf_check_catch ( void );

/*
 *********************************************************
 * non-object specific functions
 *********************************************************
 */
int saf_put_attribute(SAF_ParMode pmode, ss_pers_t *obj, const char *name, hid_t type, int count, const void *value);
int saf_get_attribute(SAF_ParMode pmode, ss_pers_t *obj, const char *name, hid_t *type, int *count, void **value);

/*
 *********************************************************
 * functions for SAF_Cat (collection category) objects
 *********************************************************
 */
SAF_Cat *saf_declare_category(SAF_ParMode pmode, SAF_Db *db, const char *name, SAF_Role *role, int tdim, SAF_Cat *cat);
int saf_describe_category(SAF_ParMode pmode, SAF_Cat *cat, char **name, SAF_Role *role, int *tdim);
int saf_find_categories (SAF_ParMode pmode, SAF_Db *db, SAF_Set *containing_set, const char *name, SAF_Role *role, int tdim,
                         int *num, SAF_Cat **found);
int saf_put_cat_att(SAF_ParMode pmode, SAF_Cat *cat, const char *name, hid_t type, int count, const void *value);
int saf_get_cat_att(SAF_ParMode pmode, SAF_Cat *cat, const char *name, hid_t *type, int *count, void **value);

/*
 *********************************************************
 * functions for SAF_Set (sets) objects
 *********************************************************
 */
SAF_Set *saf_declare_set(SAF_ParMode pmode, SAF_Db *db, const char *name, int max_topo_dim, SAF_SilRole role,
                         SAF_ExtendMode extmode, SAF_Set *set);
int saf_describe_set(SAF_ParMode pmode, SAF_Set *set, char **name, int *max_topo_dim, SAF_SilRole *role, SAF_ExtendMode *extmode,
                     SAF_TopMode *topmode, int *num, SAF_Cat **found);
int saf_find_matching_sets(SAF_ParMode pmode, SAF_Db *db, const char *name, SAF_SilRole srole, int tdim, SAF_ExtendMode extmode,
                           SAF_TopMode topmode, int *num, SAF_Set **found);
int saf_find_sets(SAF_ParMode pmode, SAF_FindSetMode fmode, SAF_Set *set, SAF_Cat *cat, int *num, SAF_Set **found);
int saf_put_set_att(SAF_ParMode pmode, SAF_Set *set, const char *key, hid_t type, int count, const void *value);
int saf_get_set_att(SAF_ParMode pmode, SAF_Set *set, const char *key, hid_t *type, int *count, void **value);

/*
 *********************************************************
 * functions for collections
 *********************************************************
 * Note: there is no "object" for collections. Collections
 * are always identified by a combination of a set and a category.
 */
hbool_t saf_same_collections(SAF_Set*, SAF_Cat*, SAF_Set*, SAF_Cat*);
int saf_declare_collection(SAF_ParMode pmode, SAF_Set *containing_set, SAF_Cat *cat, SAF_CellType ctype, int count,
                           SAF_IndexSpec ispec, SAF_DecompMode is_decomp);
int saf_find_collections(SAF_ParMode pmode, SAF_Set *containing_set, SAF_Role *role, SAF_CellType ctype, int topo_dim,
                         SAF_DecompMode decomp, int *num, SAF_Cat **found);
int saf_describe_collection(SAF_ParMode pmode, SAF_Set *containing_set, SAF_Cat *cat, SAF_CellType *t, int *count,
                            SAF_IndexSpec *ispec, SAF_DecompMode *is_decomp, SAF_Set **member_sets);
int saf_extend_collection(SAF_ParMode pmode, SAF_Set *containing_set, SAF_Cat *cat, int add_count, SAF_IndexSpec add_ispec);

/*
 *********************************************************
 * functions for SAF_IndexSpec (alternate index specification) objects 
 *********************************************************
 */

SAF_AltIndexSpec *saf_declare_alternate_indexspec(SAF_ParMode pmode, SAF_Db *db, SAF_Set *containing_set, SAF_Cat *cat,
                                                  const char *name, hid_t data_type, hbool_t is_explicit,
                                                  SAF_IndexSpec implicit_ispec, hbool_t is_compact, hbool_t is_sorted,
                                                  SAF_AltIndexSpec *aspec);
int saf_describe_alternate_indexspec(SAF_ParMode pmode, SAF_AltIndexSpec *aspec, SAF_Set *containing_set, SAF_Cat *cat,
                                     char **name, hid_t *data_type, hbool_t *is_explicit, SAF_IndexSpec *implicit_ispec,
				     hbool_t *is_compact, hbool_t *is_sorted);
int saf_find_alternate_indexspecs(SAF_ParMode pmode, SAF_Set *containing_set, SAF_Cat *cat, const char *name_grep, int *num,
				  SAF_AltIndexSpec **found);
int saf_write_alternate_indexspec(SAF_ParMode pmode, SAF_AltIndexSpec *aspec, hid_t data_type, void *buf, SAF_Db *file);
int saf_read_alternate_indexspec(SAF_ParMode pmode, SAF_AltIndexSpec *aspec, void **buf);
SAF_IndexSpec _saf_indexspec_not_applicable(void);

/*
 *********************************************************
 * functions for SAF_Rel (relation) objects 
 *********************************************************
 */
SAF_Rel * saf_declare_subset_relation(SAF_ParMode pmode, SAF_Db *db, SAF_Set *sup, SAF_Set *sub, SAF_Cat *sup_cat,
                                      SAF_Cat *sub_cat, SAF_BoundMode sbmode, SAF_BoundMode cbmode, SAF_RelRep *srtype,
                                      hid_t A_type, void *A_buf, hid_t B_type, void *B_buf, SAF_Rel *rel);
int saf_find_subset_relations(SAF_ParMode pmode, SAF_Db *db, SAF_Set *sup, SAF_Set *sub, SAF_Cat *sup_cat, SAF_Cat *sub_cat,
                              SAF_BoundMode sbmode, SAF_BoundMode cbmode, int *num, SAF_Rel **found);
int saf_describe_subset_relation(SAF_ParMode pmode, SAF_Rel *rel, SAF_Set *sup, SAF_Set *sub, SAF_Cat *sup_cat, SAF_Cat *sub_cat,
                                 SAF_BoundMode *sbmode, SAF_BoundMode *cbmode, SAF_RelRep *srtype, hid_t *data_type);
int saf_target_subset_relation(SAF_RelTarget *target, SAF_RelRep *srtype, hid_t targ_data_type);
int saf_write_subset_relation(SAF_ParMode pmode, SAF_Rel *rel, hid_t A_type, void *A_buf, hid_t B_type, void *B_buf, SAF_Db *file);
int saf_use_written_subset_relation(SAF_ParMode pmode, SAF_Rel *rel, SAF_Rel *oldRel, hid_t A_type, hid_t B_type, SAF_Db *file);
int saf_read_subset_relation(SAF_ParMode pmode, SAF_Rel *rel, SAF_RelTarget *target, void **abuf, void **bbuf);
int saf_get_count_and_type_for_subset_relation(SAF_ParMode pmode, SAF_Rel *rel, SAF_RelTarget *target, size_t *abuf_sz,
                                               hid_t *buf_type, size_t *bbuf_sz, hid_t *bbuf_type);
SAF_Rel *saf_declare_topo_relation(SAF_ParMode pmode, SAF_Db *db, SAF_Set *set, SAF_Cat *pieces, SAF_Set *range_set,
                                   SAF_Cat *range_cat, SAF_Cat *storage_decomp, SAF_Set *my_piece, SAF_RelRep *trtype,
                                   hid_t A_type, void *A_buf, hid_t B_type, void *B_buf, SAF_Rel *rel);
int saf_describe_topo_relation(SAF_ParMode pmode, SAF_Rel *rel, SAF_Set *set, SAF_Cat *the_pieces, SAF_Set *range_set,
                               SAF_Cat *range_cat, SAF_Cat *storage_decomp, SAF_RelRep *trtype, hid_t *data_type);
int saf_find_topo_relations(SAF_ParMode pmode, SAF_Db *db, SAF_Set *set, SAF_Set *topo_ancestor, int *num, SAF_Rel **found);
int saf_is_self_stored_topo_relation(SAF_ParMode pmode, SAF_Rel *rel, hbool_t *Presult);
int saf_target_topo_relation(SAF_RelTarget *target, SAF_Set *range_set, SAF_Cat *range_cat, SAF_Cat *decomp,
                             SAF_RelRep *trtype, hid_t data_type);
int saf_write_topo_relation(SAF_ParMode pmode, SAF_Rel *rel, hid_t A_type, void *A_buf, hid_t B_type, void *B_buf, SAF_Db *file);
int saf_get_count_and_type_for_topo_relation(SAF_ParMode pmode, SAF_Rel *rel, SAF_RelTarget *target, SAF_RelRep *relRep,
                                             size_t *abuf_sz, hid_t *abuf_type, size_t *bbuf_sz, hid_t *bbuf_type);
int saf_read_topo_relation(SAF_ParMode pmode, SAF_Rel *rel, SAF_RelTarget *target, void **abuf, void **bbuf);

/*
 *********************************************************
 * functions for SAF_FieldTmpl (field template) objects
 *********************************************************
 */
SAF_FieldTmpl *saf_declare_field_tmpl(SAF_ParMode pmode, SAF_Db *db, const char *name, SAF_Algebraic *atype,
                                      SAF_Basis *basis, SAF_Quantity *quantity, int num_comp, SAF_FieldTmpl *ctmpl,
                                      SAF_FieldTmpl *ftmpl);
int saf_describe_field_tmpl(SAF_ParMode pmode, SAF_FieldTmpl *ftmpl, char **name, SAF_Algebraic *alg_type, SAF_Basis *basis,
                            SAF_Quantity *quantity, int *num_comp, SAF_FieldTmpl **ctmpl);
int saf_find_field_tmpls(SAF_ParMode pmode, SAF_Db *database, const char *name, SAF_Algebraic *atype, SAF_Basis *btype,
                         SAF_Quantity *quantity, int *num, SAF_FieldTmpl **found);
int saf_put_field_tmpl_att(SAF_ParMode pmode, SAF_FieldTmpl *fldTmpl, const char *name, hid_t type, int count,
                           const void *value);
int saf_get_field_tmpl_att(SAF_ParMode pmode, SAF_FieldTmpl *fldTmpl, const char *name, hid_t *type, int *count,
                           void **value);




/*
 *********************************************************
 * functions for SAF_Field (field) objects
 *********************************************************
 */
int saf_declare_coords(SAF_ParMode pmode, SAF_Field *field);
int saf_declare_default_coords(SAF_ParMode pmode, SAF_Set *base, SAF_Field *field);
SAF_Field *saf_find_default_coords(SAF_ParMode pmode, SAF_Set *base, SAF_Field *field);
int saf_find_coords(SAF_ParMode pmode, SAF_Db *db, SAF_Set *base, int *num, SAF_Field **found);
SAF_Field *saf_declare_field(SAF_ParMode pmode, SAF_Db *db, SAF_FieldTmpl *ftmpl, const char *name, SAF_Set *base_space,
                             SAF_Unit *units, SAF_Cat *storage_decomp, SAF_Cat *coeff_assoc, int assoc_ratio, SAF_Cat *eval_coll,
                             SAF_Eval *eval_func, hid_t data_type, SAF_Field *comp_flds, SAF_Interleave comp_interleave,
                             int *comp_order, void **bufs, SAF_Field *fld);
int saf_describe_field(SAF_ParMode pmode, SAF_Field *field, SAF_FieldTmpl *ftmpl, char **name, SAF_Set *base_space,
                       SAF_Unit *units, hbool_t *is_coord, SAF_Cat *storage_decomp, SAF_Cat *coeff_assoc, int *assoc_ratio,
                       SAF_Cat *eval_coll, SAF_Eval *eval_func, hid_t *data_type, int *num_comps, SAF_Field **comp_flds,
                       SAF_Interleave *comp_interleave, int **comp_order);
int saf_find_fields(SAF_ParMode pmode, SAF_Db *db, SAF_Set *base, const char *name, SAF_Quantity *quantity, SAF_Algebraic *atype,
                    SAF_Basis *basis, SAF_Unit *unit, SAF_Cat *coeff_assoc, int assoc_ratio, SAF_Cat *eval_decomp,
                    SAF_Eval *eval_func, int *nfound, SAF_Field **found);
int saf_is_self_stored_field(SAF_ParMode pmode, SAF_Field *field, hbool_t *result);
int saf_write_field(SAF_ParMode pmode, SAF_Field *field, int member_count, SAF_RelRep *req_type, int *member_ids, int nbufs,
                    hid_t buf_type, void **bufs, SAF_Db *file);
int saf_data_has_been_written_to_field(SAF_ParMode pmode, SAF_Field *field, hbool_t *Presult);
int saf_data_has_been_written_to_comp_field(SAF_ParMode pmode, SAF_Field *field, hbool_t *Presult);
int saf_get_count_and_type_for_field(SAF_ParMode pmode, SAF_Field *field, SAF_FieldTarget *target, size_t *Pcount, hid_t *Ptype);
int saf_read_field(SAF_ParMode pmode, SAF_Field *field, SAF_FieldTarget *target, int member_count, SAF_RelRep *req_type,
                   int *member_ids, void **buf);
int saf_target_field(SAF_FieldTarget *target, SAF_Unit *targ_units, SAF_Cat *targ_storage_decomp, SAF_Cat *targ_coeff_assoc,
                     int targ_assoc_ratio, SAF_Cat *targ_eval_coll, SAF_Eval *targ_func, hid_t targ_data_type,
                     SAF_Interleave comp_interleave, int *comp_order);
int saf_put_field_att(SAF_ParMode pmode, SAF_Field *fld, const char *att_key, hid_t att_type, int count, const void *value);
int saf_get_field_att(SAF_ParMode pmode, SAF_Field *fld, const char *att_key, hid_t *att_type, int *count, void **value);

/*
 *********************************************************
 * functions for SAF_StateTmpl (state template) objects
 *********************************************************
 */
SAF_StateTmpl *saf_declare_state_tmpl(SAF_ParMode pmode, SAF_Db *database, const char *name, int num_ftmpls, SAF_FieldTmpl *ftmpls,
                                      SAF_StateTmpl *stmpl);
int saf_describe_state_tmpl(SAF_ParMode pmode, SAF_StateTmpl *stmpl, char **name, int *num_ftmpls, SAF_FieldTmpl **ftmpls);
int saf_find_state_tmpl(SAF_ParMode pmode, SAF_Db *database, const char *name, int *num_stmpls, SAF_StateTmpl **stmpls);
int saf_put_state_tmpl_att(SAF_ParMode pmode, SAF_StateTmpl *stmpl, const char *key, hid_t type, int count, const void *value);
int saf_get_state_tmpl_att(SAF_ParMode pmode, SAF_StateTmpl *stmpl, const char *key, hid_t *type, int *count, void **value);

/*
 *********************************************************
 * functions for SAF_StateGrp (state) objects
 *********************************************************
 */
SAF_StateGrp *saf_declare_state_group(SAF_ParMode pmode, SAF_Db *database, const char *name, SAF_Suite *suite,
                                      SAF_Set *mesh_space, SAF_StateTmpl *stmpl, SAF_Quantity *quantity, SAF_Unit *unit,  
                                      hid_t coord_data_type, SAF_StateGrp *state_grp);

int saf_describe_state_group(SAF_ParMode pmode, SAF_StateGrp *state_grp, char **name, SAF_Suite *suite, SAF_StateTmpl *stmpl,	
                             SAF_Quantity *quantity, SAF_Unit *unit, hid_t *coord_data_type, int *num_states);
int saf_write_state(SAF_ParMode pmode, SAF_StateGrp *state_grp, int state_index, SAF_Set *mesh_space, hid_t coord_data_type, 
                    void *coord, SAF_Field *fields);
int saf_read_state(SAF_ParMode  pmode, SAF_StateGrp *state_grp, int state_index, SAF_Set *mesh, SAF_Field *deflt_coords,  
                   void *coord, SAF_Field **fields);
int saf_find_state_groups(SAF_ParMode pmode, SAF_Suite *suite, const char *name, int *num_state_grps, SAF_StateGrp **state_grps);
int saf_put_state_grp_att(SAF_ParMode pmode, SAF_StateGrp *state_grp, const char *key, hid_t type, int count, const void *value);
int saf_get_state_grp_att(SAF_ParMode pmode, SAF_StateGrp *state_grp, const char *key, hid_t *type, int *count, void **value);

/*
 *********************************************************
 * functions for SAF_Suite (suite) objects
 *********************************************************
 */
SAF_Suite *saf_declare_suite(SAF_ParMode  pmode, SAF_Db *database, const char *name, SAF_Set *mesh_space, SAF_Set *param_space, 
                             SAF_Suite *suite);
int saf_describe_suite(SAF_ParMode pmode, SAF_Suite *suite, char **name, int *num_space_sets, SAF_Set **mesh_space,   
                       SAF_Set **param_space);
int saf_find_suites(SAF_ParMode pmode, SAF_Db  *database, const char *name, int *num_suites, SAF_Suite **suites);
int saf_put_suite_att(SAF_ParMode pmode, SAF_Suite *suite, const char *att_key, hid_t att_type, int count, const void *value);
int saf_get_suite_att(SAF_ParMode pmode, SAF_Suite *suite, const char *att_key, hid_t *att_type, int *count, void **value);

/********************************************************************************************************************************/
/********************************************************************************************************************************/
/***                                            Declarations for Raw Data I/O						      ***/
/********************************************************************************************************************************/
/********************************************************************************************************************************/
hid_t saf_grab_hdf5(SAF_Db *file);
int saf_ungrab_hdf5(hid_t h5f);

#ifdef __cplusplus
}
#endif

#endif
