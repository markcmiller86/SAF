/* include/SAFconfig.h.  Generated automatically by configure.  */
/* include/SAFconfig.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#undef WORDS_BIGENDIAN

/*
 * Copyright(C) 1999 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 1999 Sandia Corporation.
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
 *  William J. Arrighi LLNL
 *  Peter K. Espen  SNL
 *  Ray T. Hitt   SNL
 *  Robb P. Matzke   LLNL
 *  Mark C. Miller   LLNL
 *  James F. Reus   LLNL
 *  Larry A. Schoof  SNL
 * 
 * Acknowledgements:
 *  Marty L. Barnaby SNL - Red parallel perf. study/tuning
 *  David M. Butler  LPS - Data model design/implementation Spec.
 *  Albert K. Cheng  NCSA - Parallel HDF5 support
 *  Nancy Collins  IBM - Alpha/Beta user
 *  Linnea M. Cook  LLNL - Management advocate
 *  Michael J. Folk  NCSA - Management advocate 
 *  Richard M. Hedges LLNL - Blue-Pacific parallel perf. study/tuning 
 *  Quincey A. Koziol NCSA - Serial HDF5 Support 
 *  Celeste M. Matarazzo LLNL - Management advocate
 *  Tom H. Robey   SNL - Early developer
 *  Greg D. Sjaardema SNL - Alpha/Beta user
 *  Reinhard W. Stotzer SNL - Early developer
 *  Judy Sturtevant  SNL - Red parallel perf. study/tuning 
 *  Robert K. Yates  LLNL - Blue-Pacific parallel perf. study/tuning
 * 
 */
/* Define if the __attribute__(()) extension is present */
/* #define HAVE_ATTRIBUTE 1 */

/* Define if the compiler understands the __FUNCTION__ keyword. */
/* #define HAVE_FUNCTION 1 */

/* Define if we have parallel support */
#define HAVE_PARALLEL 1

/* Define to `long' if <sys/types.h> doesn't define. */
/* #undef ssize_t */

/* Define if we are compiling a production version of SAF */
/* #undef PRODUCTION_COMPILE */

/* Define if we are generating test coverage data */
/* #undef COVERAGE_COMPILE */

/* The number of bytes in a __int64.  */
#define SIZEOF___INT64 0

/* The number of bytes in a boolean.  */
#define SIZEOF_BOOLEAN 0

/* The number of bytes in a char.  */
#define SIZEOF_CHAR 1

/* The number of bytes in a double.  */
#define SIZEOF_DOUBLE 8

/* The number of bytes in a float.  */
#define SIZEOF_FLOAT 4

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a int16_t.  */
#define SIZEOF_INT16_T 2

/* The number of bytes in a int32_t.  */
#define SIZEOF_INT32_T 4

/* The number of bytes in a int64_t.  */
#define SIZEOF_INT64_T 8

/* The number of bytes in a int8_t.  */
#define SIZEOF_INT8_T 1

/* The number of bytes in a int_fast16_t.  */
#define SIZEOF_INT_FAST16_T 4

/* The number of bytes in a int_fast32_t.  */
#define SIZEOF_INT_FAST32_T 4

/* The number of bytes in a int_fast64_t.  */
#define SIZEOF_INT_FAST64_T 8

/* The number of bytes in a int_fast8_t.  */
#define SIZEOF_INT_FAST8_T 1

/* The number of bytes in a int_least16_t.  */
#define SIZEOF_INT_LEAST16_T 2

/* The number of bytes in a int_least32_t.  */
#define SIZEOF_INT_LEAST32_T 4

/* The number of bytes in a int_least64_t.  */
#define SIZEOF_INT_LEAST64_T 8

/* The number of bytes in a int_least8_t.  */
#define SIZEOF_INT_LEAST8_T 1

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* The number of bytes in a long long.  */
#define SIZEOF_LONG_LONG 8

/* The number of bytes in a off_t.  */
#define SIZEOF_OFF_T 4

/* The number of bytes in a short.  */
#define SIZEOF_SHORT 2

/* The number of bytes in a signed char.  */
#define SIZEOF_SIGNED_CHAR 1

/* The number of bytes in a size_t.  */
#define SIZEOF_SIZE_T 4

/* The number of bytes in a uint16_t.  */
#define SIZEOF_UINT16_T 2

/* The number of bytes in a uint32_t.  */
#define SIZEOF_UINT32_T 4

/* The number of bytes in a uint64_t.  */
#define SIZEOF_UINT64_T 8

/* The number of bytes in a uint8_t.  */
#define SIZEOF_UINT8_T 1

/* The number of bytes in a uint_fast16_t.  */
#define SIZEOF_UINT_FAST16_T 4

/* The number of bytes in a uint_fast32_t.  */
#define SIZEOF_UINT_FAST32_T 4

/* The number of bytes in a uint_fast64_t.  */
#define SIZEOF_UINT_FAST64_T 8

/* The number of bytes in a uint_fast8_t.  */
#define SIZEOF_UINT_FAST8_T 1

/* The number of bytes in a uint_least16_t.  */
#define SIZEOF_UINT_LEAST16_T 2

/* The number of bytes in a uint_least32_t.  */
#define SIZEOF_UINT_LEAST32_T 4

/* The number of bytes in a uint_least64_t.  */
#define SIZEOF_UINT_LEAST64_T 8

/* The number of bytes in a uint_least8_t.  */
#define SIZEOF_UINT_LEAST8_T 1

/* The number of bytes in a unsigned char.  */
#define SIZEOF_UNSIGNED_CHAR 1

/* The number of bytes in a unsigned int.  */
#define SIZEOF_UNSIGNED_INT 4

/* The number of bytes in a unsigned long.  */
#define SIZEOF_UNSIGNED_LONG 4

/* The number of bytes in a unsigned long long.  */
#define SIZEOF_UNSIGNED_LONG_LONG 8

/* The number of bytes in a unsigned short.  */
#define SIZEOF_UNSIGNED_SHORT 2

/* The number of bytes in a void *.  */
#define SIZEOF_VOID_P 4

/* Define if you have the BSDgettimeofday function.  */
/* #undef HAVE_BSDGETTIMEOFDAY */

/* Define if you have the H5Eregister_class function.  */
#define HAVE_H5EREGISTER_CLASS 1

/* Define if you have the MPI_Finalized function.  */
#define HAVE_MPI_FINALIZED 1

/* Define if you have the compress2 function.  */
#define HAVE_COMPRESS2 1

/* Define if you have the difftime function.  */
/* #define HAVE_DIFFTIME 1 */

/* Define if you have the fork function.  */
/* #define HAVE_FORK 1 */

/* Define if you have the fseek64 function.  */
/* #undef HAVE_FSEEK64 */

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the getpid function.  */
#define HAVE_GETPID 1

/* Define if you have the getpwuid function.  */
#define HAVE_GETPWUID 1

/* Define if you have the getrusage function.  */
#define HAVE_GETRUSAGE 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the kill function.  */
#define HAVE_KILL 1

/* Define if you have the longjmp function.  */
#define HAVE_LONGJMP 1

/* Define if you have the lseek64 function.  */
/* #undef HAVE_LSEEK64 */

/* Define if you have the pause function.  */
#define HAVE_PAUSE 1

/* Define if you have the psignal function.  */
/* #define HAVE_PSIGNAL 1 */

/* Define if you have the read_real_time function.  */
/* #undef HAVE_READ_REAL_TIME */

/* Define if you have the regcmp function.  */
/* #undef HAVE_REGCMP */

/* Define if you have the regcomp function.  */
#define HAVE_REGCOMP 1

/* Define if you have the regex function.  */
/* #undef HAVE_REGEX */

/* Define if you have the regexec function.  */
#define HAVE_REGEXEC 1

/* Define if you have the setsysinfo function.  */
/* #undef HAVE_SETSYSINFO */

/* Define if you have the sigaction function.  */
#define HAVE_SIGACTION 1

/* Define if you have the signal function.  */
#define HAVE_SIGNAL 1

/* Define if you have the snprintf function.  */
/* #define HAVE_SNPRINTF 1 */

/* Define if you have the sysconf function.  */
/* #define HAVE_SYSCONF 1 */

/* Define if you have the system function.  */
/* #define HAVE_SYSTEM 1 */

/* Define if you have the vsnprintf function.  */
/* #define HAVE_VSNPRINTF 1 */

/* Define if you have the waitpid function.  */
/* #define HAVE_WAITPID 1 */

/* Define if you have the <aio.h> header file.  */
/* #define HAVE_AIO_H 1 */

/* Define if you have the <direct.h> header file.  */
/* #undef HAVE_DIRECT_H */

/* Define if you have the <gmgh.h> header file.  */
/* #undef HAVE_GMGH_H */

/* Define if you have the <hdf5.h> header file.  */
#define HAVE_HDF5_H 1

/* Define if you have the <io.h> header file.  */
/* #undef HAVE_IO_H */

/* Define if you have the <libgen.h> header file.  */
#define HAVE_LIBGEN_H 1

/* Define if you have the <process.h> header file.  */
/* #undef HAVE_PROCESS_H */

/* Define if you have the <regex.h> header file.  */
#define HAVE_REGEX_H 1

/* Define if you have the <setjmp.h> header file.  */
#define HAVE_SETJMP_H 1

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <stdint.h> header file.  */
#define HAVE_STDINT_H 1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/proc.h> header file.  */
/* #undef HAVE_SYS_PROC_H */

/* Define if you have the <sys/resource.h> header file.  */
#define HAVE_SYS_RESOURCE_H 1

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/sysinfo.h> header file.  */
/* #undef HAVE_SYS_SYSINFO_H */

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <zlib.h> header file.  */
#define HAVE_ZLIB_H 1

/* Define if you have the coug library (-lcoug).  */
/* #undef HAVE_LIBCOUG */

/* Define if you have the gen library (-lgen).  */
/* #undef HAVE_LIBGEN */

/* Define if you have the gpfs library (-lgpfs).  */
/* #undef HAVE_LIBGPFS */

/* Define if you have the hdf5 library (-lhdf5).  */
#define HAVE_LIBHDF5 1

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Define if you have the mpich library (-lmpich).  */
/* #undef HAVE_LIBMPICH */

/* Define if you have the rt library (-lrt).  */
/* #define HAVE_LIBRT 1 */

/* Define if you have the z library (-lz).  */
#define HAVE_LIBZ 1

/* test directory name or . */
#define TEST_FILE_PATH "."

/* installation prefix */
#define SS_INSTALL_PREFIX "/sierra/Release/saf/2.0.0/install_janus2/dbg"

/* library installation directory */
#define SS_INSTALL_LIBDIR "/sierra/Release/saf/2.0.0/install_janus2/dbg/lib"

/* binary installation directory */
#define SS_INSTALL_BINDIR "/sierra/Release/saf/2.0.0/install_janus2/dbg/bin"

/* configuration installation directory */
#define SS_INSTALL_SYSCONFDIR "/sierra/Release/saf/2.0.0/install_janus2/dbg/etc"

/* include file installation directory */
#define SS_INSTALL_INCLUDEDIR "/sierra/Release/saf/2.0.0/install_janus2/dbg/include"

/* machine independent data installation directory */
#define SS_INSTALL_DATADIR "/sierra/Release/saf/2.0.0/install_janus2/dbg/share"

/* source code directory */
#define SS_INSTALL_SRCDIR "/sierra/Release/saf/2.0.0/saf/src"

