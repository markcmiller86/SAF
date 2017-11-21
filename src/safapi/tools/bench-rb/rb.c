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


/* A flexible I/O kernel */

#include "rb-perf.h"

typedef enum {
    TEST_POSIX          = 0,
    TEST_MPIIO          = 1,
    TEST_HDF5           = 2,
    TEST_SSLIB          = 3,
    TEST_SAF            = 4
} test_t;

typedef union {
    int         fd;
#ifdef HAVE_PARALLEL
    MPI_File    file;
#endif
    hid_t       hid;
#ifdef HAVE_SSLIB
    ss_file_t   *ssfile;
#endif
#ifdef HAVE_SAF
    SAF_Db      *db;
#endif
} file_t;

static int posix_check(void);
static int posix_open(const char*, file_t*, hbool_t);
static int posix_write(file_t*, void*, size_t, int);
static int posix_read(file_t*, void*, size_t, int);
static int posix_close(file_t*);

#ifdef HAVE_PARALLEL
static int mpiio_check(void);
static int mpiio_open(const char*, file_t*, hbool_t);
static int mpiio_write(file_t*, void*, size_t, int);
static int mpiio_read(file_t*, void*, size_t, int);
static int mpiio_close(file_t*);
#else
#   define mpiio_check NULL
#   define mpiio_open  NULL
#   define mpiio_write NULL
#   define mpiio_close NULL
#   define mpiio_read  NULL
#endif

static int hdf5_check(void);
static int hdf5_open(const char*, file_t*, hbool_t);
static int hdf5_write(file_t*, void*, size_t, int);
static int hdf5_read(file_t*, void*, size_t, int);
static int hdf5_close(file_t*);

#ifdef HAVE_SSLIB
static int ss_check(void);
static int ss_open(const char*, file_t*, hbool_t);
static int ss_write(file_t*, void*, size_t, int);
static int ss_read(file_t*, void*, size_t, int);
static int ss_flush(file_t*);
static int ss_close(file_t*);
#else
#   define ss_check NULL
#   define ss_open  NULL
#   define ss_write NULL
#   define ss_read  NULL
#   define ss_flush NULL
#   define ss_close NULL
#endif

#ifdef HAVE_SAF
static int saf_check(void);
static int saf_open(const char*, file_t*, hbool_t);
static int saf_write(file_t*, void*, size_t, int);
static int saf_read(file_t*, void*, size_t, int);
static int saf_close(file_t*);
#else
#   define saf_check NULL
#   define saf_open  NULL
#   define saf_write NULL
#   define saf_read  NULL
#   define saf_close NULL
#endif

static struct {
    int         (*do_check)(void);
    int         (*do_open)(const char *NAME, file_t *f, hbool_t);
    int         (*do_write)(file_t *f, void *buffer, size_t size, int nrecs);
    int         (*do_read)(file_t *f, void *buffer, size_t size, int nrecs);
    int         (*do_flush)(file_t *f);
    int         (*do_close)(file_t *f);
} dispatch[] = {
    {posix_check, posix_open, posix_write, posix_read, NULL,     posix_close},
    {mpiio_check, mpiio_open, mpiio_write, mpiio_read, NULL,     mpiio_close},
    {hdf5_check,  hdf5_open,  hdf5_write,  hdf5_read,  NULL,     hdf5_close},
    {ss_check,    ss_open,    ss_write,    ss_read,    ss_flush, ss_close},
    {saf_check,   saf_open,   saf_write,   saf_read,   NULL,     saf_close},
};

typedef enum {
    ASYNC_NONE=0,               /* Do synchronous I/O */
    ASYNC_RAW,                  /* Async I/O with flush attributed to raw data */
    ASYNC_META                  /* Async I/O with flush attributed to meta data */
} async_t;


/* Global vars initialized in main() */
static int self=0, ntasks=1, repeat=1;
static hbool_t collective=FALSE, skip_zero=FALSE, use_hints=FALSE, segregation=FALSE, quiet=FALSE, verify=TRUE;
static hbool_t close_every_test=TRUE;
static const char *vfd_name=NULL, *filename_template=FILENAME;
static size_t alignment=1;
static const char *argv0;
static async_t async=ASYNC_NONE;

/**/
static void
usage(FILE *stream, const char *argv0)
{
    if (0==self) {
        /*               -------------------------------------------------------------------------------- */
        fprintf(stream, "usage: %s [SWITCHES] LAYERS...\n", argv0);
        fprintf(stream, "Specify the `--help' switch for more assistance\n");
    }
}

/**/
static void
help(FILE *stream, const char *argv0)
{
    if (0==self) {
        fprintf(stream, "usage: %s [SWITCHES] LAYERS...\n", argv0);
        fprintf(stream, "\nCommand-line Switches\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --sleep=N       Sleep for N seconds before each layer test. The default\n");
        fprintf(stream, "                  is to not sleep at all. Inserting a delay can result in\n");
        fprintf(stream, "                  significant performance improvement because it gives the\n");
        fprintf(stream, "                  operating system a chance to flush dirty pages from the\n");
        fprintf(stream, "                  block device caches used by file systems.  For instance,\n");
        fprintf(stream, "                  on ASCI/Blue the mmfsd flushes all dirty GPFS pages every\n");
        fprintf(stream, "                  two minutes. One should note however that there may be\n");
        fprintf(stream, "                  other processes running on a node that cause pages to be\n");
        fprintf(stream, "                  dirtied during this period.\n");
        fprintf(stream, "                  N can also be the word \"sync\" which replaces the sleep call\n");
        fprintf(stream, "                  with an sync() call. Although this isn't as close a\n");
        fprintf(stream, "                  simulation to an actualy physics application, it will achieve\n");
        fprintf(stream, "                  almost the same thing in much less time. If N is negative\n");
        fprintf(stream, "                  then a sync() call is made followed by a sleep() for -(N+1)\n");
        fprintf(stream, "                  seconds.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --record=SIZE   Transfer SIZE bytes at a time (default 256kB). This\n");
        fprintf(stream, "                  is the size of the data passed per write operation at\n");
        fprintf(stream, "                  the programming layer being tested. In other words, this\n");
        fprintf(stream, "                  is the size of the buffer that each task passes to one of\n");
        fprintf(stream, "                  the following functions:\n");
        fprintf(stream, "                    POSIX: write()\n");
        fprintf(stream, "                    MPI:   MPI_File_write_at()     [independent]\n");
        fprintf(stream, "                           MPI_File_write_at_all() [collective]\n");
        fprintf(stream, "                    HDF5:  H5Dwrite()\n");
        fprintf(stream, "                    SSlib: ss_blob_write()\n");
        fprintf(stream, "                    SAF:   saf_write_field()\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --nrecs=N       Number of records to write from each task per layer test.\n");
        fprintf(stream, "                  This is the number of calls made to one of the functions\n");
        fprintf(stream, "                  listed under the `--record' switch.  The default is 15\n");
        fprintf(stream, "                  calls per task per layer test.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --repeat=N      Each test is run N times (default is one) as if its name was\n");
        fprintf(stream, "                  specified N times on the command line.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --collective    Use collective I/O (the default is independent I/O calls).\n");
        fprintf(stream, "                  This benchmark always opens, writes, and closes files\n");
        fprintf(stream, "                  collectively although under the covers the call might not\n");
        fprintf(stream, "                  perform any communication (e.g., POSIX write()). However,\n");
        fprintf(stream, "                  for the layers that support both independent and collective\n");
        fprintf(stream, "                  I/O, specifying this switch will cause the I/O call to\n");
        fprintf(stream, "                  assume that it's a collective call and possibly communicate\n");
        fprintf(stream, "                  with other tasks.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --file=NAME     Use the specified name for the data file. The NAME may contain\n");
        fprintf(stream, "                  format specifiers of the form %%W<S> where W is an optional\n");
        fprintf(stream, "                  field width integer (negative means left justified) and S is\n");
        fprintf(stream, "                  one of the following words:\n");
        fprintf(stream, "                    LAYER:  The name of the layer being tested (e.g., posix or\n");
        fprintf(stream, "                            hdf5, or similar)\n");
        fprintf(stream, "                    REPEAT: The integer counter associated with the --repeat\n");
        fprintf(stream, "                            switch.\n");
        fprintf(stream, "                    PID:    The process ID. Use care with this since it's\n");
        fprintf(stream, "                            likely that the PID is different on each task\n");
        fprintf(stream, "                            of an MPI job.\n");
        fprintf(stream, "                  The default file name is \"%s\"\n", FILENAME);

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --vfd=NAME      Use the specified HDF5 virtual file driver.\n");
        fprintf(stream, "                    POSIX:  This switch has no effect.\n");
        fprintf(stream, "                    MPI:    This switch has no effect.\n");
        fprintf(stream, "                    HDF5:   Valid parallel drivers are mpiio (the default)\n");
        fprintf(stream, "                            and mpiposix. Valid serial drivers are stdio,\n");
        fprintf(stream, "                            sec2 (the default), and core.\n");
        fprintf(stream, "                    SSlib:  Valid parallel drivers are mpiio, mpiposix (the\n");
        fprintf(stream, "                            default), and core. Valid serial drivers are\n");
        fprintf(stream, "                            stdio, sec2 (the default), and core.\n");
        fprintf(stream, "                    SAF:    Valid parallel drivers are mpiio, mpiposix (the\n");
        fprintf(stream, "                            default), and core. Valid serial drivers are\n");
        fprintf(stream, "                            sec2 (the default) and core.\n");
        fprintf(stream, "                  The drivers are defined as follows\n");
        fprintf(stream, "                    mpiio:    Uses the MPI-2 file operations supplied by\n");
        fprintf(stream, "                              either a vendor supplied library or by ROMIO.\n");
        fprintf(stream, "                    mpiposix: Like mpiio except makes calls to the POSIX\n");
        fprintf(stream, "                              functions open(), close(), read() and write().\n");
        fprintf(stream, "                              Inter-task communication is handled by MPI.\n");
        fprintf(stream, "                    stdio:    A serial driver that uses the standard I/O\n");
        fprintf(stream, "                              functions of the C library such as fopen()\n");
        fprintf(stream, "                              fclose(), etc.\n");
        fprintf(stream, "                    sec2:     A serial driver that uses the POSIX file\n");
        fprintf(stream, "                              system calls like the mpiposix driver.\n");
        fprintf(stream, "                    core:     With the HDF5 layer, this is a file that is\n");
        fprintf(stream, "                              stored only in the memory address space and\n");
        fprintf(stream, "                              doesn't correspond to any file in the\n");
        fprintf(stream, "                              filesystem. With SSlib and SAF this refers to\n");
        fprintf(stream, "                              a \"transient\" file where SSlib maintains the\n");
        fprintf(stream, "                              necessary bookkeeping but may not actually ever\n");
        fprintf(stream, "                              create an HDF5 file.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --align=NBYTES  Set alignment to NBYTES (kB and MB suffixes allowed). The\n");
        fprintf(stream, "                  default is 1 byte, which is equivalent to no alignment.\n");
        fprintf(stream, "                  this switch has the following effect for each layer test:\n");
        fprintf(stream, "                    POSIX: Each task aligns each write to begin at a file\n");
        fprintf(stream, "                           address that is a multiple of NBYTES. Space\n");
        fprintf(stream, "                           between the end of one record and the beginning\n");
        fprintf(stream, "                           of the next record is never written to\n");
        fprintf(stream, "                           explicitly, although the operating system may\n");
        fprintf(stream, "                           write (part of) that region to the page cache\n");
        fprintf(stream, "                           and disk.\n");
        fprintf(stream, "                    MPI:   Same as for POSIX.\n");
        fprintf(stream, "                    HDF5:  The HDF5 library is informed that it should\n");
        fprintf(stream, "                           align any HDF5 file object that is 64kB or\n");
        fprintf(stream, "                           larger on a file address that is a multiple of\n");
        fprintf(stream, "                           NBYTES.  This has the effect that every\n");
        fprintf(stream, "                           contiguous dataset (provided it is large enough)\n");
        fprintf(stream, "                           will begin at the specified alignment, but the\n");
        fprintf(stream, "                           individual low-level write operations coming out\n");
        fprintf(stream, "                           of the bottom of HDF5 will most likely not be\n");
        fprintf(stream, "                           aligned unless the per-task record size is also\n");
        fprintf(stream, "                           a multiple of NBYTES.\n");
        fprintf(stream, "                    SSlib: This switch is ignored. However, if the --vfd\n");
        fprintf(stream, "                           switch is used to select the mpiposix driver then\n");
        fprintf(stream, "                           SSlib will set the HDF5 alignment to 512kB as\n");
        fprintf(stream, "                           described above, with the addition that the first\n");
        fprintf(stream, "                           512kB of the file will be unused. See the\n");
        fprintf(stream, "                           ss_file_open() function for details.\n");
        fprintf(stream, "                    SAF:   Same as for SSlib.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --skipzero      Advance one record distance in the file before starting\n");
        fprintf(stream, "                  the test.  This is only honored for the POSIX, MPI, and\n");
        fprintf(stream, "                  HDF5 layer tests. The SSlib and SAF layer tests will\n");
        fprintf(stream, "                  seek 512kB into the file when the mpiposix virtual\n");
        fprintf(stream, "                  file driver is used.  In the case of HDF5 the alignment\n");
        fprintf(stream, "                  must also be set to something larger than one and\n");
        fprintf(stream, "                  determines the number of bytes that will be skipped.  This\n");
        fprintf(stream, "                  switch has absolutely no effect on the number of bytes\n");
        fprintf(stream, "                  written through the layer's API (although it will affect\n");
        fprintf(stream, "                  the final file size and possibly the amount of data written\n");
        fprintf(stream, "                  by the operating system.\n");
        
        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, " --segregate      For the layer tests that normally perform meta data ops\n");
        fprintf(stream, "                  interleaved with raw data ops (HDF5, SSlib, and SAF) this\n");
        fprintf(stream, "                  switch will cause the two types of operations to be\n");
        fprintf(stream, "                  segregated from one another. For example, for the SAF test\n");
        fprintf(stream, "                  all the saf_declare_field() calls will be made first, then\n");
        fprintf(stream, "                  all the saf_write_field() calls.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --usehints      Use GPFS hints to improve performance.  It's meaning\n");
        fprintf(stream, "                  depends on the layer being tested:\n");
        fprintf(stream, "                    POSIX:  There are two methods for specifying hints and\n");
        fprintf(stream, "                            the method to be used is chosen by compile time\n");
        fprintf(stream, "                            constants (see below).\n");
        fprintf(stream, "                    MPI:    This switch is not applicable to this layer.\n");
        fprintf(stream, "                    HDF5:   If the \"mpiposix\" virtual file driver is used\n");
        fprintf(stream, "                            (see the --vfd switch above) then HDF5 is told\n");
        fprintf(stream, "                            to enable certain GPFS hints as detailed in the\n");
        fprintf(stream, "                            H5FD_mpiposix_open() function.\n");
        fprintf(stream, "                    SSlib:  HDF5's GPFS hints are always turned on when the\n");
        fprintf(stream, "                            mpiposix virtual file driver is used.\n");
        fprintf(stream, "                    SAF:    Same as SSlib.\n");
        fprintf(stream, "                  The two methods for specifying hints in the posix layer\n");
        fprintf(stream, "                  test are:\n");
        fprintf(stream, "                    DAN_MCNABB_METHOD:");
#ifdef DAN_MCNABB_METHOD
        fprintf(stream, " (enabled)\n");
#else
        fprintf(stream, " (disabled)\n");
#endif
        fprintf(stream, "                      Each task initially releases the entire byte range,\n");
        fprintf(stream, "                      and then requests a token for the first GPFS block\n");
        fprintf(stream, "                      to which it writes, thus indicating to GPFS that the\n");
        fprintf(stream, "                      access pattern is user defined.  After doing this each\n");
        fprintf(stream, "                      node can freely write whatever blocks of the file it\n");
        fprintf(stream, "                      wishes without any more hints.  This only works well\n");
        fprintf(stream, "                      if the nodes don't share GPFS blocks--each node will\n");
        fprintf(stream, "                      be assigned tokens for the byte ranges to which it\n");
        fprintf(stream, "                      writes. Otherwise the time-expensive token revocation\n");
        fprintf(stream, "                      algorithm kicks in.  The Token manager (TM) state to\n");
        fprintf(stream, "                      manage all the byte-ranges (BR) that are individually\n");
        fprintf(stream, "                      held by different nodes will grow as large as it needs\n");
        fprintf(stream, "                      to. Therefore we need to be prudent in subdividing the\n");
        fprintf(stream, "                      file because this state for BR tokens has to fit in\n");
        fprintf(stream, "                      256MB of memory (ASCI/Blue). Each BR token takes about\n");
        fprintf(stream, "                      48 bytes of TM memory, but this memory is used for the\n");
        fprintf(stream, "                      hundreds of thousands of other file tokens in use by\n");
        fprintf(stream, "                      other nodes.\n");
        fprintf(stream, "                    HAVE_GMGH_H:");
#if !defined(DAN_MCNABB_METHOD) && defined(HAVE_GMGH_H)
        fprintf(stream, " (enabled)\n");
#else
        fprintf(stream, " (disabled)\n");
#endif
        fprintf(stream, "                      Uses IBM's GMGH API. The source code is provided or\n");
        fprintf(stream, "                      it may exist in a precompiled state on the system.\n");
        fprintf(stream, "                      See the IBM GPFS redbook for more information [1]\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --quiet         Turns off most of the usual chatter and only prints the\n");
        fprintf(stream, "                  final timing results for each test.  Use with care since\n");
        fprintf(stream, "                  you'll have no hard record of any details.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --async[=raw|meta]\n");
        fprintf(stream, "                  Use asynchronous I/O when possible.  This currently only works\n");
        fprintf(stream, "                  in the sslib test when two-phase I/O is enabled. It causes the\n");
        fprintf(stream, "                  ss_blob_write() calls to initiate asynchronous data shipping.\n");
        fprintf(stream, "                  Writes will be completed during the ss_file_close() call and\n");
        fprintf(stream, "                  attributed to the total meta-data I/O time unless the \"raw\"\n");
        fprintf(stream, "                  option is specified, in which case ss_blob_flush() is called\n");
        fprintf(stream, "                  and attributed to the raw I/O time.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --verify\n");
        fprintf(stream, "  --no-verify     After writing to and closing the file, the file is opened\n");
        fprintf(stream, "                  for read-only access, the data is read and compared with the\n");
        fprintf(stream, "                  buffer used for writing. The MPI task that wrote the data is\n");
        fprintf(stream, "                  the same task that reads it.  No particular optimizations are\n");
        fprintf(stream, "                  made for reading since the purpose of this benchmark is to\n");
        fprintf(stream, "                  time only write speed. The default is to verify\n");
        
        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "  --close\n");
        fprintf(stream, "  --no-close      Normally the benchmark will open a file, write to it, then\n");
        fprintf(stream, "                  close it for each test. Specifying the --no-close switch will\n");
        fprintf(stream, "                  cause the file to be opened just once for each of the LAYERS\n");
        fprintf(stream, "                  command-line arguments regardless of how many times each\n");
        fprintf(stream, "                  layer is tested due to the --repeat switch.  This is\n");
        fprintf(stream, "                  particularly useful with the --async=meta switch because it\n");
        fprintf(stream, "                  simulates the mode of operation for a physics application\n");
        fprintf(stream, "                  which flushes data one compute cycle after writing the data,\n");
        fprintf(stream, "                  thereby giving the OS some time to complete the async write.\n");
        fprintf(stream, "                  Note that this currently only works for the sslib layer.\n");
        fprintf(stream, "                  Also note that some of the layers don't yet know how to\n");
        fprintf(stream, "                  properly verify the data with this switch, and therefore the\n");
        fprintf(stream, "                  --no-verify switch should probably also be supplied.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "\nAdditional Non-switch Arguments\n");
        fprintf(stream, "  The LAYERS is a list of key words: posix, mpiio, hdf5, sslib, and/or saf. One\n");
        fprintf(stream, "  test will be performed for each word (or more if the --repeat switch was\n");
        fprintf(stream, "  specified.  The various libraries are not reinitialized between each test,\n");
        fprintf(stream, "  but rather all layers are initialized once at the beginning and finalized\n");
        fprintf(stream, "  at the very end of the run.\n");
        fprintf(stream, "  The following layers were compiled into this executable:\n");
        fprintf(stream, "      posix\n");
#ifdef HAVE_PARALLEL
        fprintf(stream, "      mpiio\n");
#endif
        fprintf(stream, "      hdf5\n");
#ifdef HAVE_SSLIB
        fprintf(stream, "      sslib\n");
#endif
#ifdef HAVE_SAF
        fprintf(stream, "      saf\n");
#endif

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "\nEnvironment Variables\n");
        fprintf(stream, "  The following environment variables have significant performance effects:\n");
        fprintf(stream, "    MP_WAIT_MODE=sleep (IBM's MPI)\n");
        fprintf(stream, "      This improves performance when the number of MPI tasks per node equals\n");
        fprintf(stream, "      the number of CPUs per node.  Setting this to `sleep' suspends blocked\n");
        fprintf(stream, "      processes in the OS after a short period of spin-waiting and results in\n");
        fprintf(stream, "      better CPU utilization for the node's process mix.\n");
        fprintf(stream, "    SSLIB_2PIO\n");
        fprintf(stream, "      Controls SSlib's two-phase I/O algorighm. See ss_blob_set_2pio() for\n");
        fprintf(stream, "      details, defined in sslib/lib/ssblob.c.\n");
        fprintf(stream, "    SAF_PRECOND_DISABLE=all\n");
        fprintf(stream, "    SAF_POSTCOND_DISABLE=all\n");
        fprintf(stream, "    SAF_ASSERT_DISABLE=all\n");
        fprintf(stream, "      Setting these variables will prevent all versions of SAF (debug and\n");
        fprintf(stream, "      production) from performing potentially expensive run-time checking.\n");

        fprintf(stream, "\n"); /*------------------ do not exceed 80 columns ----------------------------*/
        fprintf(stream, "\nReferences\n");
        fprintf(stream, "  [1] IBM GPFS Redbook chapter for GMGH\n");
        fprintf(stream, "      http://www.redbooks.ibm.com/pubs/pdfs/redbooks/sg246035.pdf\n");
    }
}

/**/
static void
badarg(const char *arg, const char *why)
{
    P0 fprintf(stderr, "%s: command line switch `%s' %s\n", argv0, arg, why);
    exit(1);
}

/* print NBYTES in a human friendly format */
static char *
bytes(size_t nbytes)
{
    static char buf[6][128];
    static int ncalls=0;
    char *retval=NULL;

#ifdef HAVE_SSLIB
    retval = ss_bytes((hsize_t)nbytes, buf[ncalls]);
    assert(retval==buf[ncalls]);
    sprintf(retval+strlen(retval), " byte%s", 1==nbytes?"":"s");
#else
    retval = buf[ncalls];
    sprintf(retval, "%lu byte%s", (unsigned long)nbytes, 1==nbytes?"":"s");
#endif

    ncalls++;
    return retval;
}

/* Convert a file name template into an actual file name. Any occurrance of "%WIDTH<WORD>" is replaced by some value
 * where WIDTH is an integer field width (negative values are allowed to indicate left justification) and WORD is one
 * of the following:
 *    LAYER     The name of the layer being tested (e.g., posix, mpiio, hdf5, etc)
 *    NTASKS    The number of tasks in this MPI job
 *    PID       The process ID (probably different on every MPI task)
 *    REPEAT    The zero-origin counter for the --repeat switch.
 * Example:
 *    $ rb --file="$HOME/gpfs/%<LAYER>-`date +%Y%m%d`-%05<PID>.data"
 */
static const char *
expand_filename(const char *template,
                const char *layer_name,
                int repeat_counter) 
{
    static char format[1024], retval[4096];
    size_t outn=0, i;

    while (template && *template) {
        assert(outn<sizeof retval);
        if ('%'==*template) {
            for (i=1; template[i] && strchr("-0123456789", template[i]); i++) /*void*/;
            assert(i+1<sizeof format);
            strncpy(format, template, i);
            if (!strncmp(template+i, "<LAYER>", 7)) {
                strcpy(format+i, "s");
                sprintf(retval+outn, format, layer_name);
                outn += strlen(retval+outn);
                template += i + 7;
            } else if (!strncmp(template+i, "<NTASKS>", 8)) {
                strcpy(format+i, "d");
                sprintf(retval+outn, format, ntasks);
                outn += strlen(retval+outn);
                template += i + 8;
            } else if (!strncmp(template+i, "<REPEAT>", 8)) {
                strcpy(format+i, "d");
                sprintf(retval+outn, format, repeat_counter);
                outn += strlen(retval+outn);
                template += i + 8;
            } else if (!strncmp(template+i, "<PID>", 5)) {
                strcpy(format+i, "d");
                sprintf(retval+outn, format, getpid());
                outn += strlen(retval+outn);
                template += i + 5;
            } else {
                strncpy(retval+outn, template, i);
                template += i;
            }
        } else {
            retval[outn++] = *template++;
        }
    }
    assert(outn < sizeof retval);
    retval[outn] = '\0';
    return retval;
}

/* Prints information about which MPI tasks had errors and returns negative if any had errors */
static int
verification_errors(int nerrors)
{
    int         *all_nerrors=NULL, i;
    
    /* Send all error counts to task zero so we can print them in task rank order. */
    if (0==self)
        all_nerrors = calloc((size_t)ntasks, sizeof(int));
#ifdef HAVE_PARALLEL
    MPI_Gather(&nerrors, 1, MPI_INT, all_nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
#else
    all_nerrors[0] = nerrors;
#endif
    if (0==self) {
        for (i=0; i<ntasks; i++) {
            if (all_nerrors[i]) {
                P0 printf("    ERROR: task %d had %d error%s verifying data\n", i, all_nerrors[i], 1==all_nerrors[i]?"":"s");
                if (i!=self) nerrors += all_nerrors[i];
            }
        }
        free(all_nerrors);
    }

    /* Task zero knows if any task had errors, so we broadcast that so all tasks return the same value */
#ifdef HAVE_PARALLEL
    MPI_Bcast(&nerrors, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
    return nerrors ? -1 : 0;
}

/********************************************************************************************************************************/
/********************************************************************************************************************************/
/***           P O S I X   F u n c t i o n s . . .                                                                            ***/
/********************************************************************************************************************************/
/********************************************************************************************************************************/

/**/
static int
posix_check(void)
{
    if (collective)
        P0 printf("    WARNING: the --collective switch does not apply to this layer\n");
    if (vfd_name)
        P0 printf("    WARNING: the --vfd switch does not apply to this layer\n");
    if (segregation)
        P0 printf("    WARNING: the --segregate switch does not apply to this layer\n");
    if (async!=ASYNC_NONE)
        P0 printf("    WARNING: the --async switch does not apply to this layer\n");
    if (!close_every_test)
        P0 printf("    WARNING: the --no-close switch does not apply to this layer\n");
    if (use_hints) {
#ifdef DAN_MCNABB_METHOD
        P0 printf("    using Dan McNabb's method for GPFS hints\n");
#elif defined(HAVE_GMGH_H)
        P0 printf("    using the GMGH library for GPFS hints\n");
#else
        P0 printf("    WARNING: the --usehits switch was ignored\n");
#endif
    }
    return 0;
}

/**/
static int
posix_open(const char *name, file_t *f/*out*/, hbool_t rdonly)
{
    if (rdonly) {
        f->fd = open(name, O_RDONLY);
    } else {
        /* One task creates the file and then everyone opens it */
        if (0==self) f->fd = open(name, O_CREAT|O_TRUNC|O_RDWR, 0666);
#ifdef HAVE_PARALLEL
        MPI_Barrier(MPI_COMM_WORLD);
#endif
        if (0!=self) f->fd = open(name, O_RDWR, 0666);
        fprintf(stderr, "posix_open() self=%2d fd=%d\n", self, f->fd);
    }
    if (f->fd<0) perror(name);
    assert(f->fd>=0);

#ifdef DAN_MCNABB_METHOD
    if (use_hints) {
        /*Dan's second email said that everyone can release tokens*/
        /* Task 0 has owns the entire byte range since it did the create. We will now release the entire range */
        struct {
            gpfsFcntlHeader_t   hdr;
            gpfsFreeRange_t     fr;
        } hint;
        memset(&hint, 0, sizeof hint);
        hint.hdr.totalLength = sizeof hint;
        hint.hdr.fcntlVersion = GPFS_FCNTL_CURRENT_VERSION;
        hint.fr.structLen = sizeof hint.fr;
        hint.fr.structType = GPFS_FREE_RANGE;
        hint.fr.start = 0;
        hint.fr.length = 0;
        if (gpfs_fcntl(f->fd, &hint)<0) {
            perror("gpfs_fcntl");
            exit(1);
        }
    }
#endif
    return 0;
}

/**/
static int
posix_write(file_t *f, void *buffer, size_t recsize, int nrecs)
{
    int         i;
    ssize_t     n;
    int         aligned_recsize = ALIGN(recsize, alignment);
    off_t       offset=(skip_zero?aligned_recsize:0) + self*aligned_recsize;
    off_t       o2;

    P0 printf("    starting file offset = %s\n", bytes(skip_zero ? (size_t)aligned_recsize : 0));
    if (alignment>1) {
        P0 printf("    records are aligned on boundaries of %s\n", bytes(alignment));
        P0 printf("    effective record size for alignment is %lu+%lu = %s\n",
                  (unsigned long)recsize, (unsigned long)(aligned_recsize-recsize), bytes((size_t)aligned_recsize));
    }

    if (use_hints) {
#ifdef DAN_MCNABB_METHOD
        /* Dan McNabb's method. Each process requests a token for the first block which it accesses, thus indicating
         * to gpfs that the access pattern is user defined. After doing this, then each node can freely write whatever
         * blocks of the file it wishes without any more hints. This only works well if each node keeps working on
         * block boundaries. Each node will acquire a byte-range token for each block it writes, and as long as there
         * is not overlapping block writes, there will not have to be any revoke processing between the nodes.
         *
         * The Token Manager (TM) state to manage all the byte-ranges (BR) that are independently held by different nodes
         * will grow as large as we make it. Therefore, we need to be prudent in subdividing the file because this state
         * for BR tokens has to fit in the 256MB of memory. Each BR only takes about 48 bytes of TM memory, but this
         * TM memory is used for the hundreds of thousdands of other file tokens in use by other nodes.
         *
         * If we finish one section of the file and move on to another and do not close the file, it would help reduce
         * TM usage by using the GPFS_FREE_RANGE hint to release old unused tokens. */
        struct {
            gpfsFcntlHeader_t   hdr;
            gpfsMultipleAccessRange_t mar;
        } hint;
        struct stat sb;
        
        if (fstat(f->fd, &sb)<0) {
            perror("fstat");
            exit(1);
        }

        memset(&hint, 0, sizeof hint);
        hint.hdr.totalLength = sizeof hint;
        hint.hdr.fcntlVersion = GPFS_FCNTL_CURRENT_VERSION;
        hint.mar.structLen = sizeof hint.mar;
        hint.mar.structType = GPFS_MULTIPLE_ACCESS_RANGE;
        hint.mar.accRangeCnt = 1;
        hint.mar.relRangeCnt = 0;
        hint.mar.accRangeArray[0].blockNumber = offset / sb.st_blksize;
        hint.mar.accRangeArray[0].start = 0;
        hint.mar.accRangeArray[0].length = sb.st_blksize;
        hint.mar.accRangeArray[0].isWrite = TRUE;

        if (gpfs_fcntl(f->fd, &hint)<0) {
            perror("gpfs_fcntl");
            exit(1);
        }

        /* Now the write */
        for (i=0; i<nrecs; i++) {
            o2 = lseek(f->fd, offset, SEEK_SET);
            assert(offset==o2);
            n = write(f->fd, buffer, recsize);
            assert(n==recsize);
            offset += ntasks * aligned_recsize;
        }
        

#elif defined(HAVE_GMGH_H)
        /* GMGH method from the IBM GPFS redbook: http://www.redbooks.ibm.com/pubs/pdfs/redbooks/sg246035.pdf */
        gmgh    *p = malloc(sizeof(*p));
        memset(p, 0, sizeof(*p));
        gmgh_init_hint(p, f->fd, recsize, MAXHINT);
        while (nrecs>0) {
            int nhints = MIN(nrecs, MAXHINT);
            for (i=0; i<nhints; i++) {
                gmgh_post_hint(p, offset, recsize, i, TRUE);
                offset += ntasks * aligned_recsize;
            }
            gmgh_declare_1st_hint(p);
            for (i=0; i<nhints; i++)
                gmgh_xfer(p, buffer, i);
            nrecs -= nhints;
        }
        free(p);
#endif
    } else {
        for (i=0; i<nrecs; i++) {
            o2 = lseek(f->fd, offset, SEEK_SET);
            assert(offset==o2);
            n = write(f->fd, buffer, recsize);
            if (n<0) perror("write");
            if ((size_t)n!=recsize) fprintf(stderr, "posix_write() write(fd=%d, buffer=0x%08lx, recsize=%lu) = %ld\n",
                                            f->fd, (unsigned long)buffer, (unsigned long)recsize, (long)n);
            assert(n==(ssize_t)recsize);
            offset += ntasks * aligned_recsize;
        }
    }
    return 0;
}

/**/
static int
posix_read(file_t *f, void *_buffer, size_t recsize, int nrecs)
{
    char        *buffer=_buffer, *tmpbuf=calloc(recsize, 1);
    int         i, nerrors=0, status;
    ssize_t     n;
    int         aligned_recsize = ALIGN(recsize, alignment);
    off_t       offset = (skip_zero?aligned_recsize:0) + self * aligned_recsize;
    off_t       o2;
    size_t      elmtno;

    for (i=0; i<nrecs; i++) {
        o2 = lseek(f->fd, offset, SEEK_SET);
        assert(offset==o2);
        n = read(f->fd, tmpbuf, recsize);
        assert(n==(ssize_t)recsize);
        offset += ntasks * aligned_recsize;
        for (elmtno=0; elmtno<recsize; elmtno++) {
            if (tmpbuf[elmtno]!=buffer[elmtno])
                nerrors++;
        }
    }

    free(tmpbuf);
    status = verification_errors(nerrors);
    return status;
}

/**/
static int
posix_close(file_t *f)
{
    int         status = close(f->fd);
    assert(status>=0);
    return 0;
}

/********************************************************************************************************************************/
/********************************************************************************************************************************/
/***           M P I - I O   F u n c t i o n s . . .                                                                          ***/
/********************************************************************************************************************************/
/********************************************************************************************************************************/

#ifdef HAVE_PARALLEL
/**/
static int
mpiio_check(void)
{
    if (vfd_name)
        P0 printf("    WARNING: the --vfd switch does not apply to this layer\n");
    if (segregation)
        P0 printf("    WARNING: the --segregate switch does not apply to this layer\n");
    if (use_hints)
        P0 printf("    WARNING: the --usehints switch does not apply to this layer\n");
    if (async!=ASYNC_NONE)
        P0 printf("    WARNING: the --async switch does not apply to this layer\n");
    if (!close_every_test)
        P0 printf("    WARNING: the --no-close switch does not apply to this layer\n");
    return 0;
}

/**/
static int
mpiio_open(const char *name, file_t *f, hbool_t rdonly)
{
    if (rdonly) {
        MPI_File_open(MPI_COMM_WORLD, name, MPI_MODE_RDONLY, MPI_INFO_NULL, &(f->file));
    } else {
        MPI_File_delete(name, MPI_INFO_NULL);
        MPI_File_open(MPI_COMM_WORLD, name, MPI_MODE_CREATE|MPI_MODE_RDWR, MPI_INFO_NULL, &(f->file));
    }
    return 0;
}

/**/
static int
mpiio_write(file_t *f, void *buffer, size_t recsize, int nrecs)
{
    int         i, n;
    size_t      aligned_recsize = ALIGN(recsize, alignment);
    MPI_Offset  offset=(skip_zero?aligned_recsize:0) + self*aligned_recsize;
    MPI_Status  status;

    P0 printf("    writes are %s\n", collective?"collective":"independent");
    if (alignment>1) {
        P0 printf("    records are aligned on boundaries of %s\n", bytes(alignment));
        P0 printf("    effective record size for alignment is %lu+%lu = %s\n",
                  (unsigned long)recsize, (unsigned long)(aligned_recsize-recsize), bytes(aligned_recsize));
    }
    P0 printf("    starting file offset = %s\n", bytes(skip_zero?recsize:0));

    for (i=0; i<nrecs; i++) {
        if (collective) {
            MPI_File_write_at_all(f->file, offset, buffer, recsize, MPI_BYTE, &status);
        } else {
            MPI_File_write_at(f->file, offset, buffer, recsize, MPI_BYTE, &status);
        }
        MPI_Get_count(&status, MPI_BYTE, &n);
        assert(n==(int)recsize);
        offset += ntasks * aligned_recsize;
    }
    return 0;
}

/**/
static int
mpiio_read(file_t *f, void *_buffer, size_t recsize, int nrecs)
{
    char        *buffer=_buffer, *tmpbuf=calloc(recsize, 1);
    int         i, n, nerrors=0;
    size_t      aligned_recsize = ALIGN(recsize, alignment);
    MPI_Offset  offset = (skip_zero?aligned_recsize:0) + self*aligned_recsize;
    MPI_Status  status;
    size_t      elmtno;

    for (i=0; i<nrecs; i++) {
        MPI_File_read_at(f->file, offset, tmpbuf, recsize, MPI_BYTE, &status);
        MPI_Get_count(&status, MPI_BYTE, &n);
        assert(n==(int)recsize);
        offset += ntasks * aligned_recsize;

        for (elmtno=0; elmtno<recsize; elmtno++) {
            if (buffer[elmtno]!=tmpbuf[elmtno])
                nerrors++;
        }
    }

    free(tmpbuf);
    return verification_errors(nerrors);
}

/**/
static int
mpiio_close(file_t *f)
{
    MPI_File_close(&(f->file));
    return 0;
}
#endif

/********************************************************************************************************************************/
/********************************************************************************************************************************/
/***           H D F 5   F u n c t i o n s . . .                                                                              ***/
/********************************************************************************************************************************/
/********************************************************************************************************************************/

/**/
static int
hdf5_check(void)
{
    if (async!=ASYNC_NONE)
        P0 printf("    WARNING: The --async flag does not apply to this layer\n");
    if (!close_every_test)
        P0 printf("    WARNING: The --no-close flag does not apply to this layer\n");
    return 0;
}

/**/
static int
hdf5_open(const char *name, file_t *f, hbool_t rdonly)
{
    hid_t       fcpl = H5Pcreate(H5P_FILE_CREATE);
    hid_t       fapl = H5Pcreate(H5P_FILE_ACCESS);
    herr_t      status;

    if (vfd_name) {
        if (!strcmp(vfd_name, "stdio")) {
            P0 printf("    using the \"stdio\" virtual file driver\n");
            status = H5Pset_fapl_stdio(fapl);
            assert(status>=0);
        } else if (!strcmp(vfd_name, "sec2")) {
            P0 printf("    using the \"sec2\" virtual file driver\n");
            status = H5Pset_fapl_sec2(fapl);
            assert(status>=0);
        } else if (!strcmp(vfd_name, "core")) {
            P0 printf("    using the \"core\" virtual file driver\n");
            status = H5Pset_fapl_core(fapl, 256*1024, FALSE);
            assert(status>=0);
#ifdef HAVE_PARALLEL
        } else if (!strcmp(vfd_name, "mpiio")) {
            P0 printf("    using the \"mpiio\" virtual file driver\n");
            if (use_hints)
                P0 printf("    WARNING: the --usehints switch was ignored\n");
            status = H5Pset_fapl_mpio(fapl, MPI_COMM_WORLD, MPI_INFO_NULL);
            assert(status>=0);
        } else if (!strcmp(vfd_name, "mpiposix")) {
            P0 printf("    using the \"mpiposix\" virtual file driver\n");
            if (use_hints)
                P0 printf("    using HDF5's GPFS hints mechanism\n");
            status = H5Pset_fapl_mpiposix(fapl, MPI_COMM_WORLD, use_hints);
            assert(status>=0);
#endif
        } else {
            P0 printf("    ERROR: invalid virtual file driver: %s\n", vfd_name);
            return -1;
        }
    } else {
#ifdef HAVE_PARALLEL
        P0 printf("    defaulting to the mpiio virtual file driver\n");
        if (use_hints)
            P0 printf("    WARNING: the --usehints switch was ignored\n");
        status = H5Pset_fapl_mpio(fapl, MPI_COMM_WORLD, MPI_INFO_NULL);
        assert(status>=0);
#else
        P0 printf("    defaulting to the sec2 virtual file driver\n");
        status = H5Pset_fapl_sec2(fapl);
        assert(status>=0);
#endif
    }

    if (alignment>1) {
        size_t threshold = 64*1024;
        P0 printf("    objects larger than %s are aligned on boundaries of %s\n", bytes(threshold), bytes(alignment));
#if H5_VERS_NUMBER<=1007036
        if (vfd_name && !strcmp(vfd_name, "core") && (alignment & 1)) {
            P0 printf("    ERROR: Alignment must be even to work around bugs in hdf5-1.7.36 and earlier\n");
            return -1;
        }
#endif
        status = H5Pset_alignment(fapl, (hsize_t)threshold, (hsize_t)alignment);
        assert(status>=0);
    }

    if (alignment>1 && skip_zero) {
        hsize_t start_at;
        int i;
        for (i=8; i<32; i++) {
            start_at = (8==i?0:(hsize_t)1<<i);
            if (start_at>=alignment) break;
        }
        P0 printf("    starting file offset = %s\n", bytes((size_t)start_at));
        status = H5Pset_userblock(fcpl, start_at);
        assert(status>=0);
    } else {
        P0 printf("    starting file offset = 0\n");
    }

    if (rdonly) {
        f->hid = H5Fopen(name, H5F_ACC_RDONLY, fapl);
    } else {
        f->hid = H5Fcreate(name, H5F_ACC_TRUNC, fcpl, fapl);
    }
    assert(f->hid>=0);
    
    H5Pclose(fcpl);
    H5Pclose(fapl);
    return 0;
}

/**/
static int
hdf5_write(file_t *f, void *buffer, size_t _recsize, int nrecs)
{
    hsize_t     recsize = _recsize;
    hid_t       dxpl    = H5Pcreate(H5P_DATASET_XFER);
    hsize_t     dsize   = recsize * ntasks;
    hid_t       mspace  = H5Screate_simple(1, &recsize, NULL);
    hid_t       fspace  = H5Screate_simple(1, &dsize, NULL);
    hssize_t    offset  = self * recsize;
    herr_t      status;
    int         pass, i;
    char        dname[64];
    hid_t       *dset = malloc(nrecs*sizeof(*dset));
    
    /* Create dataset transfer properties */
#ifdef HAVE_PARALLEL
    status = H5Pset_dxpl_mpio(dxpl, collective?H5FD_MPIO_COLLECTIVE:H5FD_MPIO_INDEPENDENT);
    assert(status>=0);
    P0 printf("    writes are %s\n", collective?"collective":"independent");
#endif
    P0 printf("    meta/raw data ops are %ssegregated\n", segregation?"":"not ");

    /* If raw/meta ops are segregated then make three passes over the records:
     *    0 -- create all datasets
     *    1 -- write all datasets
     *    2 -- close all datasets
     * Otherwise do everything in a single pass. */
    for (pass=0; pass<(segregation?3:1); pass++) {
        for (i=0; i<nrecs; i++) {
            
            /* Create the dataset (always on the first pass) */
            if (0==pass) {
                sprintf(dname, "d%d", i);
                dset[i] = H5Dcreate(f->hid, dname, H5T_NATIVE_CHAR, fspace, H5P_DEFAULT);
                assert(dset[i]>=0);
            }

            /* Write to the dataset (first or second pass depending on segregation) */
            if (!segregation || 1==pass) {
                status = H5Sselect_hyperslab(fspace, H5S_SELECT_SET, &offset, NULL, &recsize, NULL);
                assert(status>=0);
                status = H5Dwrite(dset[i], H5T_NATIVE_CHAR, mspace, fspace, dxpl, buffer);
                assert(status>=0);
            }

            /* Close the dataset (first or third pass depending on segregation) */
            if (!segregation || 2==pass) {
                H5Dclose(dset[i]);
                dset[i] = -1;
            }
        }
    }

    H5Pclose(dxpl);
    H5Sclose(mspace);
    H5Sclose(fspace);
    free(dset);
    return 0;
}

/**/
static int
hdf5_read(file_t *f, void *_buffer, size_t _recsize, int nrecs)
{
    hsize_t     recsize = _recsize;
    char        *buffer = _buffer, *tmpbuf=calloc(_recsize, 1);
    hid_t       dxpl = H5Pcreate(H5P_DATASET_XFER);
    hid_t       mspace = H5Screate_simple(1, &recsize, NULL);
    hid_t       fspace;
    hssize_t    offset = self * recsize;
    herr_t      status;
    int         i, nerrors=0;
    char        dname[64];
    hid_t       dset;
    size_t      elmtno;

    /* Read and compare all data */
    for (i=0; i<nrecs; i++) {
        sprintf(dname, "d%d", i);
        dset = H5Dopen(f->hid, dname);
        assert(dset>=0);
        fspace = H5Dget_space(dset);
        assert(fspace>=0);
        status = H5Sselect_hyperslab(fspace, H5S_SELECT_SET, &offset, NULL, &recsize, NULL);
        assert(status>=0);
        status = H5Dread(dset, H5T_NATIVE_CHAR, mspace, fspace, dxpl, tmpbuf);
        assert(status>=0);
        status = H5Sclose(fspace);
        assert(status>=0);
        status = H5Dclose(dset);
        assert(status>=0);

        for (elmtno=0; elmtno<recsize; elmtno++) {
            if (tmpbuf[elmtno]!=buffer[elmtno])
                nerrors++;
        }
    }

    /* Release resources */
    free(tmpbuf);
    H5Pclose(dxpl);
    H5Sclose(mspace);

    /* Count and report errors */
    status = verification_errors(nerrors);
    return status;
}

/**/
static int
hdf5_close(file_t *file)
{
    H5Fclose(file->hid);
    return 0;
}

/********************************************************************************************************************************/
/********************************************************************************************************************************/
/***           S S l i b   F u n c t i o n s . . .                                                                            ***/
/********************************************************************************************************************************/
/********************************************************************************************************************************/
#ifdef HAVE_SSLIB

/**/
static int
ss_check(void)
{
    hbool_t     use_mpiposix=TRUE, do_2pio=TRUE;
    ss_prop_t   *props=NULL;

    /* Determine which virtual file driver ss_file_open() will eventually choose. It will be the mpiposix driver if and only
     * if there is more than one MPI task and the mpiio driver isn't requested. */
    if (1==ntasks) {
        if (vfd_name && (!strcmp(vfd_name, "mpiposix") || !strcmp(vfd_name, "mpiio"))) {
            P0 printf("    ERROR: %s driver is not valid for a serial run\n", vfd_name);
            return -1;
        }
        use_mpiposix = FALSE;
    } else if (!vfd_name || !strcmp(vfd_name, "mpiposix")) {
        use_mpiposix = TRUE;
    } else {
        use_mpiposix = FALSE;
    }

    if (use_mpiposix) {
        if (alignment>1 && alignment!=512*1024) {
            P0 printf("    WARNING: the --alignment switch was ignored\n");
            P0 printf("    alignment set to 512kB for any object larger than 64kB (see ss_file_open)\n");
        }
        if (!skip_zero) {
            P0 printf("    WARNING: the --skipzero switch was turned on\n");
            P0 printf("    HDF5 will seek past the first 512kB of the file (see ss_file_open)\n");
        }
        if (!use_hints) {
            P0 printf("    WARNING: the --usehints switch was turned on (see ss_file_open)\n");
        }
    } else {
        if (alignment>1) {
            P0 printf("    WARNING: the --alignment switch was ignored (no alignment)\n");
        }
        if (skip_zero) {
            P0 printf("    WARNING: the --skipzero switch was ignored\n");
        }
        if (use_hints) {
            P0 printf("    WARNING: the --usehints switch was ignored\n");
        }
    }

    /* Info about two phase I/O */
    props = ss_blob_get_2pio(NULL, NULL);
    if (0==ss_prop_get_i(props, "maxaggtasks")) {
        P0 printf("    two-phase I/O is disabled\n");
        do_2pio = FALSE;
    } else {
        P0 printf("    two-phase I/O is enabled:\n");
        P0 printf("      minbufsize =  %s\n",         bytes(ss_prop_get_u(props, "minbufsize")));
        P0 printf("      alignment =   %s\n",         bytes(ss_prop_get_u(props, "alignment")));
        P0 printf("      maxaggtasks = %d tasks\n",   ss_prop_get_i(props, "maxaggtasks"));
        P0 printf("      sendqueue =   %d buffers\n", ss_prop_get_i(props, "sendqueue"));
        P0 printf("      aggbuflimit = %s\n",         bytes(ss_prop_get_u(props, "aggbuflimit")));
        P0 printf("      asynchdf5 =   %s\n",         ss_prop_get_i(props, "asynchdf5")?"enabled":"disabled");
        P0 printf("      aggbase =     %d%s\n",      ss_prop_get_i(props, "aggbase"),
                  ss_prop_get_i(props, "aggbase")<0 ? " (task selected by hashing dataset address)":"");
        P0 printf("      tpn =         %d tasks%s\n", ss_prop_get_i(props, "tpn"), 
                  ss_prop_get_i(props, "tpn")<=0 ? " (using 16/4 algorithm)":"");
        do_2pio = TRUE;
    }

    if (ASYNC_NONE==async) {
        P0 printf("    using synchronous I/O\n");
    } else if (async!=ASYNC_NONE) {
        if (do_2pio) {
            P0 printf("    asynchronous I/O with flush attributed to %s I/O time\n",
                      ASYNC_RAW==async?"raw":"meta");
        } else {
            P0 printf("    WARNING: the --async switch has no effect when 2-phase I/O is disabled\n");
            P0 printf("    using synchronous I/O\n");
        }
    }
    
    return 0;
}

/**/
static int
ss_open(const char *name, file_t *f, hbool_t rdonly)
{
    ss_prop_t   *props=NULL;
    unsigned    flags=0;
    herr_t      status;
    hid_t       fapl=-1;

    /* Choose file creation properties */
    if (vfd_name) {
        if (!strcmp(vfd_name, "core")) {
            P0 printf("    using a transient SSlib file\n");
            flags |= H5F_ACC_TRANSIENT;
        } else if (!strcmp(vfd_name, "mpiio")) {
            P0 printf("    using the \"mpiio\" virtual file driver\n");
            assert(ntasks>1);
            if (!props) props = ss_prop_new("mpiio file properties");
            assert(props);
            status = ss_prop_add(props, "use_mpiio", H5T_NATIVE_HBOOL, &true);
            assert(status>=0);
        } else if (!strcmp(vfd_name, "mpiposix")) {
            assert(ntasks>1);
            P0 printf("    using the \"mpiposix\" virtual file driver\n");
        } else if (!strcmp(vfd_name, "stdio")) {
            P0 printf("    using the \"stdio\" virtual file driver\n");
            assert(1==ntasks);
            fapl = H5Pcreate(H5P_FILE_ACCESS);
            assert(fapl>=0);
            status = H5Pset_fapl_stdio(fapl);
            assert(status>=0);
            if (!props) props = ss_prop_new("stdio file properties");
            assert(props);
            status = ss_prop_add(props, "fapl", H5T_NATIVE_HID, &fapl);
            assert(status>=0);
        } else if (!strcmp(vfd_name, "sec2")) {
            P0 printf("    using the \"sec2\" virtual file driver\n");
            assert(1==ntasks);
        } else {
            P0 printf("    ERROR: invalid virtual file driver: %s\n", vfd_name);
            return -1;
        }
    } else {
        P0 printf("    using the default virtual file driver chosen by ss_file_open()\n");
    }

    /* Create the file */
    if (rdonly) {
        f->ssfile = ss_file_open(NULL, name, flags, props);
    } else {
        f->ssfile = ss_file_create(name, flags, props);
    }
    assert(f->ssfile);

    /* This stuff comes from ss_file_open() */
    if (ntasks>1 && (!vfd_name || !strcmp(vfd_name, "mpiposix"))) {
        P0 printf("    alignment set to 512kB for objects >= 64kB in size\n");
        P0 printf("    avoiding writes to first 512kB of the file\n");
        P0 printf("    using HDF5's GPFS hints for the mpiposix driver\n");
    } else {
        P0 printf("    no alignment\n");
        P0 printf("    not skipping any bytes at the beginning of the file\n");
        P0 printf("    not using GPFS hints\n");
    }
    return 0;
}

/**/
static int
ss_write(file_t *f, void *buffer, size_t recsize, int nrecs)
{
    int         pass, i;
    ss_blob_t   *blob = malloc(nrecs*sizeof(*blob));
    hsize_t     offset;
    herr_t      status;
    ss_scope_t  topscope=SS_SCOPE_NULL;
    unsigned    flags_bc=SS_ALLSAME;                    /* blob creation flags */
    unsigned    flags_dc=SS_BLOB_RANK;                  /* dataset creation flags */
    unsigned    flags_wr=SS_BLOB_UNBIND |               /* write flags */
                         (async!=ASYNC_NONE?SS_BLOB_ASYNC:0U) |
                         (collective?SS_BLOB_COLLECTIVE:0U);
    
    
    P0 printf("    blob creation is collective (SS_ALLSAME)\n"); assert(SS_ALLSAME==flags_bc);
    P0 printf("    dataset creation is collective (SS_BLOB_RANK)\n"); assert(SS_BLOB_RANK==flags_dc);
    P0 printf("    writes are %s %s asynchronous\n", collective?"collective":"independent",
              ASYNC_NONE==async?"but not":"and");
    P0 printf("    meta/raw data ops are %ssegregated\n", segregation?"":"not ");

    ss_file_topscope(f->ssfile, &topscope);

    /* If raw/meta ops are segregated then make three passes over the records:
     *    0 -- create all datasets
     *    1 -- write all datasets
     * Otherwise do everything in a single pass. */
    for (pass=0; pass<(segregation?2:1); pass++) {
        for (i=0; i<nrecs; i++) {

            /* Create the blob and underlying dataset (always on pass zero) */
            if (0==pass) {
                ss_blob_t *x = (ss_blob_t*)ss_pers_new(&topscope, SS_MAGIC(ss_blob_t), NULL, flags_bc,
                                                       (ss_pers_t*)(blob+i), NULL);
                assert(x);
                status = ss_blob_bind_m1(blob+i, buffer, H5T_NATIVE_CHAR, (hsize_t)recsize);
                assert(status>=0);
                status = ss_blob_mkstorage(blob+i, &offset, flags_dc, NULL);
                assert(status>=0);
            }
            
            /* Write to the blob (pass zero or one) */
            if (!segregation || 1==pass) {
                status = ss_blob_write1(blob+i, offset, (hsize_t)recsize, flags_wr, NULL);
                assert(status>=0);
            }
        }
    }

    /* If the writes are independent we might as well go ahead and start the data shipping. Do not reap anything yet. */
    status = ss_blob_flush(&topscope, NULL, 0U, NULL);
    assert(status>=0);

    blob = SS_FREE(blob);
    return 0;
}

/**/
static int
ss_read(file_t *f, void *_buffer, size_t _recsize, int nrecs)
{
    char        *buffer=_buffer, *tmpbuf=calloc(_recsize, 1);
    hsize_t     recsize=_recsize;
    hsize_t     offset=self*recsize;
    int         i, nerrors=0;
    ss_blob_t   blob=SS_BLOB_NULL;
    herr_t      status;
    ss_scope_t  scope=SS_SCOPE_NULL;
    void        *x;
    size_t      elmtno;

    ss_file_topscope(f->ssfile, &scope);

    /* We assume that the only blobs in this file are the ones created by ss_write() */
    for (i=0; i<nrecs; i++) {
        x = ss_pers_refer_c(&scope, SS_MAGIC(ss_blob_t), (size_t)i, (ss_pers_t*)&blob);
        assert(x);
        status = ss_blob_bind_m1(&blob, tmpbuf, H5T_NATIVE_CHAR, recsize);
        assert(status>=0);
        x = ss_blob_read1(&blob, offset, recsize, SS_BLOB_UNBIND, NULL);
        assert(x==tmpbuf);

        for (elmtno=0; elmtno<recsize; elmtno++) {
            if (buffer[elmtno]!=tmpbuf[elmtno])
                nerrors++;
        }
    }

    SS_FREE(tmpbuf);
    return verification_errors(nerrors);
}

/**/
static int
ss_flush(file_t *f)
{
    herr_t status;
    ss_scope_t topscope;

    /* The flags (3rd arg) indicate that SSlib should block until all H5Dwrite() calls complete, then block until all data
     * shipping completes. Completing data shipping will start H5Dwrite() but after previous H5Dwrite() were completed. We
     * omit the SS_BLOB_REAP_* bits so that data shipping doesn't suddently turn into H5Dwrite() and thereby force SSlib to
     * wait for H5Dwrite() calls one cycle early. */
    P0 printf("    Flushing H5Dwrite calls from two dumps ago and data shipping from previous dump\n");
    status = ss_blob_flush(ss_file_topscope(f->ssfile, &topscope), NULL,
                           SS_BLOB_REAP_SEND|SS_BLOB_FLUSH_SHIP|SS_BLOB_FLUSH_WRITE, NULL);
    assert(status>=0);

    return 0;
}
    
/**/
static int
ss_close(file_t *f)
{
    herr_t status;

    if (ASYNC_META==async)
        P0 printf("    flushing asynchronous writes\n");
    status = ss_file_close(f->ssfile);
    assert(status>=0);
    return 0;
}

#endif /*HAVE_SSLIB*/

/********************************************************************************************************************************/
/********************************************************************************************************************************/
/***           S A F   F u n c t i o n s . . .                                                                                ***/
/********************************************************************************************************************************/
/********************************************************************************************************************************/
#ifdef HAVE_SAF

/**/
static int
saf_check(void)
{
    if (ss_check()<0) return -1;
    if (collective) {
        P0 printf("    WARNING: the --collective switch has been turned off because, although the saf_write_field()\n");
        P0 printf("             function is a collective operation, the underlying calls to ss_blob_write() are\n");
        P0 printf("             independent\n");
    }
    if (!close_every_test)
        P0 printf("    WARNING: the --no-close switch does not apply to this layer\n");
    return 0;
}

/**/
static int
saf_open(const char *name, file_t *f, hbool_t rdonly)
{
    SAF_DbProps *p=saf_createProps_database();
    int status;

    /* Choose file creation properties */
    assert(p);
    if (vfd_name) {
        if (!strcmp(vfd_name, "core")) {
            P0 printf("    using a memory resident file\n");
            status = saf_setProps_MemoryResident(p);
            assert(SAF_SUCCESS==status);
        } else if (!strcmp(vfd_name, "mpiio")) {
            P0 printf("    ERROR: SAF is unable to set HDF5's virtual file driver to mpiio\n");
            return -1;
        } else if (!strcmp(vfd_name, "mpiposix")) {
            if (1==ntasks) {
                P0 printf("    ERROR: SAF cannot set the mpiposix driver for a single-task run\n");
                return -1;
            }
            P0 printf("    using the \"mpiposix\" virtual file driver\n");
        } else if (!strcmp(vfd_name, "stdio")) {
            P0 printf("    ERROR: SAF is unable to set HDF5's virtual file driver to stdio\n");
            return -1;
        } else if (!strcmp(vfd_name, "sec2")) {
            if (1!=ntasks) {
                P0 printf("    ERROR: The sec2 driver cannot be used in a parallel run\n");
                return -1;
            }
            P0 printf("    using the \"sec2\" virtual file driver\n");
        } else {
            P0 printf("    ERROR: invalid virtual file driver: %s\n", vfd_name);
            return -1;
        }
    } else {
        P0 printf("    using the default virtual file driver chosen by ss_file_open()\n");
    }


    /* Open the file */
    if (rdonly) {
        f->db = saf_open_database(name, p);
    } else {
        saf_setProps_Clobber(p);
        f->db = saf_open_database(name, p);
    }
    assert(f->db);
    return 0;
}

/**/
static int
saf_write(file_t *f, void *buffer, size_t _recsize, int nrecs)
{
    int         pass, i, status, hslab[3];
    char        field_name[32];
    SAF_Set     base_space, domain;
    SAF_FieldTmpl ftmpl;
    SAF_Eval    eval_func = *SAF_SPACE_PWLINEAR;
    SAF_Field   *field = malloc(nrecs*sizeof(*field));
    SAF_Cat     nodes_cc;
    SAF_Rel     relation;
    int         nints = _recsize / sizeof(int);
    void        *x;
    SAF_Unit    meter;

    assert(nints>0);

    P0 printf("    fields are created with SAF_EACH mode\n");
    P0 printf("    saf_write_field() is SAF_EACH mode with underlying independent I/O\n");
    P0 printf("    meta/raw data ops are %ssegregated\n", segregation?"":"not ");
    
    /* The category for the collections below */
    x = saf_declare_category(SAF_ALL, f->db, "nodes", SAF_TOPOLOGY, 0, &nodes_cc);
    assert(x);

    /* Declare a single base space */
    x = saf_declare_set(SAF_ALL, f->db, "base space", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &base_space);
    assert(x);
    status = saf_declare_collection(SAF_ALL, &base_space, &nodes_cc, SAF_CELLTYPE_POINT, nints*ntasks, SAF_1DC(nints*ntasks),
                                    SAF_DECOMP_TRUE);
    assert(SAF_SUCCESS==status);

    /* Each task creates its own subset of the base space */
    x = saf_declare_set(SAF_EACH, f->db, "domain", 0, SAF_SPACE, SAF_EXTENDIBLE_FALSE, &domain);
    assert(x);
    status = saf_declare_collection(SAF_EACH, &domain, &nodes_cc, SAF_CELLTYPE_POINT, nints, SAF_1DC(nints), SAF_DECOMP_TRUE);
    assert(SAF_SUCCESS==status);
    hslab[0] = self * nints;
    hslab[1] = nints;
    hslab[2] = 1;
    x = saf_declare_subset_relation(SAF_EACH, f->db, &base_space, &domain, SAF_COMMON(&nodes_cc), SAF_HSLAB,
                                    SAF_INT, hslab, H5I_INVALID_HID, NULL, &relation);
    assert(x);

    /* The field template for all fields created below */
    x = saf_declare_field_tmpl(SAF_EACH, f->db, "ftmpl", SAF_ALGTYPE_SCALAR, NULL, SAF_QLENGTH, 1, NULL, &ftmpl);
    assert(x);
    x = saf_find_one_unit(f->db, "meter", &meter);
    assert(x);

    /* If raw/meta ops are segregated then make two passes over the records:
     *    0 -- declare all fields
     *    1 -- write all field data
     * Otherwise do everything in a single pass. */
    for (pass=0; pass<(segregation?2:1); pass++) {
        for (i=0; i<nrecs; i++) {
            
            /* Declare the field (pass zero) */
            if (0==pass) {
                sprintf(field_name, "f%d.%d", i, self);
                x = saf_declare_field(SAF_EACH, f->db, &ftmpl, field_name, &domain, &meter, SAF_SELF(f->db), &nodes_cc,
                                      1, SAF_SELF(f->db), &eval_func, SAF_INT, NULL, SAF_INTERLEAVE_NONE, NULL, NULL, field+i);
                assert(x);
            }

            /* Write field data (pass zero or one) */
            if (!segregation || 1==pass) {
                status = saf_write_field(SAF_EACH, field+i, SAF_WHOLE_FIELD, 1, H5I_INVALID_HID, (void**)&buffer, NULL);
                assert(SAF_SUCCESS==status);
            }
        }
    }
    free(field);
    return 0;
}

/**/
static int
saf_read(file_t *f, void *_buffer, size_t recsize, int nrecs)
{
    char        *buffer=_buffer, *tmpbuf=calloc(recsize, 1);
    int         i, status, nfound, nerrors=0;
    char        field_name[32];
    size_t      elmtno;
    SAF_Field(fields, p_fields, 2);

    for (i=0; i<nrecs; i++) {
        /* Find the field */
        sprintf(field_name, "f%d.%d", i, self);
        nfound = 1;
        status = saf_find_fields(SAF_EACH, f->db, SAF_UNIVERSE(XXX), field_name, SAF_ANY_QUANTITY, SAF_ALGTYPE_ANY,
                                 SAF_ANY_BASIS, SAF_ANY_UNIT, SAF_ANY_CAT, SAF_ANY_RATIO, SAF_ANY_CAT, SAF_ANY_EFUNC,
                                 &nfound, &p_fields);
        assert(status>=0);
        assert(1==nfound);
        assert(fields);

        /* Read and verify the field */
        saf_read_field(SAF_EACH, fields, NULL, SAF_WHOLE_FIELD, (void**)&tmpbuf);
        for (elmtno=0; elmtno<recsize; elmtno++) {
            if (buffer[elmtno]!=tmpbuf[elmtno])
                nerrors++;
        }
    }

    SS_FREE(tmpbuf);
    return verification_errors(nerrors);
}

/**/
static int
saf_close(file_t *f)
{
    saf_close_database(f->db);
    return 0;
}

#endif /*HAVE_SAF*/

/********************************************************************************************************************************/
/********************************************************************************************************************************/
/***           M a i n . . .                                                                                                  ***/
/********************************************************************************************************************************/
/********************************************************************************************************************************/

/**/
int
main(int argc, char *argv[])
{
    int         argno, nrecs=15, i, retval=0;
    int         how_long=0;                             /*how long to sleep; negative implies sync()*/
    char        *buffer, *rest;
    size_t      recsize=256*1024;
    wallclock_t overall_start, overall_end, raw_start, raw_end;
    herr_t      status;
    const char  *s, *filename=NULL;
    unsigned    h5maj, h5min, h5rel;
    hbool_t     exclude_boundaries;                     /* omit statistics for first and last run of each --repeat loop */

    /* Initialize all layers. */
#ifdef HAVE_PARALLEL
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &self);
#endif
#ifdef HAVE_SAF
    saf_init(NULL);
#elif defined(HAVE_SSLIB)
    ss_init(SS_COMM_WORLD);
#endif

    /* Make sure stdout is unbuffered so we can see benchmark progress as it occurs */
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);


    /* Parse arguments */
    argv0 = argv[0];
    for (argno=1; argno<argc && '-'==argv[argno][0]; argno++) {
        if (!strcmp(argv[argno], "--help") || !strcmp(argv[argno], "-help") ||
            !strcmp(argv[argno], "-h") || !strcmp(argv[argno], "-?")) {
            help(stdout, argv[0]);
            goto done;
        } else if (!strncmp(argv[argno], "--sleep", 7)) {
            if (!argv[argno][7]) {
                if (argno+1>=argc) badarg(argv[argno], "is missing its value");
                how_long = strtol(argv[++argno], NULL, 0);
            } else if ('='==argv[argno][7]) {
                if (!strcmp(argv[argno]+8, "sync")) {
                    how_long = -1;
                } else {
                    how_long = strtol(argv[argno]+8, NULL, 0);
                }
            } else {
                badarg(argv[argno], "is not recognized");
            }
        } else if (!strncmp(argv[argno], "--record", 8)) {
            if (!argv[argno][8]) {
                if (argno+1>=argc) badarg(argv[argno], "is missing its integer value");
                s = argv[++argno];
            } else if ('='==argv[argno][8]) {
                s = argv[argno]+9;
            } else {
                badarg(argv[argno], "is not recognized");
            }
            recsize = strtol(s, &rest, 0);
            if (!strcmp(rest, "kB") || !strcmp(rest, "kb") || !strcmp(rest, "k")) {
                recsize *= 1024;
            } else if (!strcmp(rest, "MB") || !strcmp(rest, "mb") || !strcmp(rest, "M") || !strcmp(rest, "m")) {
                recsize *= 1024*1024;
            } else if (*rest) {
                P0 fprintf(stderr, "%s: unknown unit for --record switch: %s\n", argv[0], rest);
                goto error;
            }
        } else if (!strncmp(argv[argno], "--align", 7)) {
            if (!argv[argno][7]) {
                if (argno+1>=argc) badarg(argv[argno], "is missing its integer value");
                s = argv[++argno];
            } else if ('='==argv[argno][7]) {
                s = argv[argno]+8;
            } else {
                badarg(argv[argno], "is not recognized");
            }
            alignment = strtol(s, &rest, 0);
            if (!strcmp(rest, "kB") || !strcmp(rest, "kb") || !strcmp(rest, "k")) {
                alignment *= 1024;
            } else if (!strcmp(rest, "MB") || !strcmp(rest, "mb") || !strcmp(rest, "M") || !strcmp(rest, "m")) {
                alignment *= 1024*1024;
            } else if (*rest) {
                P0 fprintf(stderr, "%s: unknown unit for --alignment switch: %s\n", argv[0], rest);
                goto error;
            }
        } else if (!strncmp(argv[argno], "--async", 7)) {
            if (!argv[argno][7]) {
                if (argno+1<argc) {
                    if (!strcmp(argv[argno+1], "raw")) {
                        async = ASYNC_RAW;
                        argno++;
                    } else if (!strcmp(argv[argno+1], "meta")) {
                        async = ASYNC_META;
                        argno++;
                    } else {
                        async = ASYNC_META;
                    }
                }
            } else if ('='==argv[argno][7]) {
                if (!strcmp(argv[argno]+8, "raw")) {
                    async = ASYNC_RAW;
                } else if (!strcmp(argv[argno]+8, "meta")) {
                    async = ASYNC_META;
                } else {
                    badarg(argv[argno], "async options are \"raw\" or \"meta\"");
                }
            } else {
                badarg(argv[argno], "is not recognized");
            }
        } else if (!strncmp(argv[argno], "--nrecs", 7)) {
            if (!argv[argno][7]) {
                if (argno+1>=argc) badarg(argv[argno], "is missing its integer value");
                s = argv[++argno];
            } else if ('='==argv[argno][7]) {
                s = argv[argno]+8;
            } else {
                badarg(argv[argno], "is not recognized");
            }
            nrecs = strtol(s, NULL, 0);
        } else if (!strcmp(argv[argno], "--collective")) {
            collective = TRUE;
        } else if (!strcmp(argv[argno], "--quiet")) {
            quiet = TRUE;
        } else if (!strcmp(argv[argno], "--verify")) {
            verify = TRUE;
        } else if (!strcmp(argv[argno], "--no-verify")) {
            verify = FALSE;
        } else if (!strcmp(argv[argno], "--close")) {
            close_every_test = TRUE;
        } else if (!strcmp(argv[argno], "--no-close")) {
            close_every_test = FALSE;
        } else if (!strncmp(argv[argno], "--vfd", 5)) {
            if (!argv[argno][5]) {
                if (argno+1>=argc) badarg(argv[argno], "is missing the VFD name");
                s = argv[++argno];
            } else if ('='==argv[argno][5]) {
                s = argv[argno]+6;
            } else {
                badarg(argv[argno], "is not recognized");
            }
            vfd_name = s; /*will check later*/
        } else if (!strncmp(argv[argno], "--file", 6)) {
            if (!argv[argno][6]) {
                if (argno+1>=argc) badarg(argv[argno], "is missing the file name");
                s = argv[++argno];
            } else if ('='==argv[argno][6]) {
                s = argv[argno]+7;
            } else {
                badarg(argv[argno], "is not recognized");
            }
            filename_template = s;
        } else if (!strcmp(argv[argno], "--skipzero")) {
            skip_zero = TRUE;
        } else if (!strcmp(argv[argno], "--usehints")) {
            use_hints = TRUE;
        } else if (!strcmp(argv[argno], "--segregate")) {
            segregation = TRUE;
        } else if (!strncmp(argv[argno], "--repeat", 8)) {
            if (!argv[argno][8]) {
                if (argno+1>=argc) badarg(argv[argno], "is missing its integer value");
                s = argv[++argno];
            } else if ('='==argv[argno][8]) {
                s = argv[argno]+9;
            } else {
                badarg(argv[argno], "is not recognized");
            }
            repeat *= strtol(s, NULL, 0);
        } else if (!strcmp(argv[argno], "--")) {
            argno++;
            break;
        } else {
            badarg(argv[argno], "is not recognized");
            goto error;
        }
    }
    if (argno>=argc) {
        usage(stdout, argv[0]);
        goto done;
    }

    P0 printf("Basic information:\n");
    
#ifdef HAVE_SAF
    {
        char versionbuf[64];
        P0 printf("  SAF version %s", saf_version_string(FALSE, versionbuf, sizeof versionbuf));
#ifdef HAVE_PARALLEL
        P0 printf(" (parallel support enabled)\n");
#else
        P0 printf(" (serial-only)\n");
#endif /*HAVE_PARALLEL*/
    }
#endif /*HAVE_SAF*/

#ifdef HAVE_SSLIB
    P0 printf("  SSlib version %d.%d.%d", SS_VERS_MAJOR, SS_VERS_MINOR, SS_VERS_RELEASE);
    if (SS_VERS_ANNOT && SS_VERS_ANNOT[0]) P0 printf("-%s", SS_VERS_ANNOT);
    P0 printf("\n");
#endif
    
    H5get_libversion(&h5maj, &h5min, &h5rel);
    P0 printf("  HDF5 version %u.%u.%u", h5maj, h5min, h5rel);
    if (H5_VERS_SUBRELEASE && strlen(H5_VERS_SUBRELEASE)>0)
        P0 printf("-%s", H5_VERS_SUBRELEASE);
    P0 printf("\n");

#ifdef HAVE_PARALLEL
    P0 printf("  Running on %d MPI task%s\n", ntasks, 1==ntasks?"":"s");
#endif

    P0 printf("  Time obtained by calling %s\n", time_method);
    P0 printf("  Transfer size: %s, %d time%s\n", bytes(recsize), nrecs, 1==nrecs?"":"s");

    /* Don't include the first two passes or the last pass through the --repeat loop in the final statistics if we go through
     * the loop at least four times and the --no-close switch was turned on.  This allows us to calculate the long-term
     * bandwidth for a physics application, avoid special cases:
     *   Test 0: the only one that opens the file
     *   Test 1: might not have any raw data to flush when H5Dwrite() is asynchronous
     *   Last:   the only one that closes the file (it also might flush an extra test's data) */
    exclude_boundaries = (repeat>=4 && !close_every_test);

    /* Create the buffer and write to it to ensure non-lazy allocation */
    buffer = malloc(recsize);
    assert(buffer);
    memset(buffer, (unsigned char)(self+1), recsize);

    /* Now perform each test in succession */
    for (/*void*/; argno<argc; argno++) {
        double overall_delta_min, overall_delta_max, overall_delta_total=0.0;
        double raw_delta_min, raw_delta_max, raw_delta_total=0.0;
        double open_delta_min, open_delta_max, open_delta_total=0.0;
        double close_delta_min, close_delta_max, close_delta_total=0.0;
        size_t total_bytes = recsize * nrecs * ntasks;
        file_t f;
        int ntests=0;                                           /* number of tests to include in statistics */

        for (i=0; i<repeat; i++) {
            test_t          test;

            if (0==self) {
                printf("testing %s", argv[argno]);
                if (repeat>1) printf(" %d of %d tests", i+1, repeat);
                printf("\n");
            }
            
            if (!strcmp(argv[argno], "posix")) {
                test = TEST_POSIX;
            } else if (!strcmp(argv[argno], "mpiio")) {
                test = TEST_MPIIO;
            } else if (!strcmp(argv[argno], "hdf5")) {
                test = TEST_HDF5;
            } else if (!strcmp(argv[argno], "sslib")) {
                test = TEST_SSLIB;
            } else if (!strcmp(argv[argno], "saf")) {
                test = TEST_SAF;
            } else {
                P0 fprintf(stderr, "%s: unknown test: %s\n", argv[0], argv[argno]);
                goto error;
            }
            if (!dispatch[test].do_open) {
                P0 fprintf(stderr, "%s: unsupported test: %s\n", argv[0], argv[argno]);
                goto error;
            }

            filename = expand_filename(filename_template, argv[argno], i);

            if (how_long<0) {
                P0 printf("  Synchronizing files with sync()...\n");
                sync();
            }
            if (how_long>0 || how_long<-1) {
                unsigned slp = how_long>0 ? how_long : -(how_long+1);
                P0 printf("  Sleeping for %u seconds...\n", slp);
                sleep(slp);
            }

            P0 printf("  Checking preconditions...\n");
            status = (dispatch[test].do_check)();
            assert(status>=0);

            /* ================================== OVER ALL TIME STARTS HERE ================================================ */
#ifdef HAVE_PARALLEL
            MPI_Barrier(MPI_COMM_WORLD);
#endif
            get_current_time(&overall_start);

            /* Prepare the file for use. That means the file is opened, or data from the previous pass through this
             * loop is flushed, or nothing happens. Data flushed here is counted in the overall time but not the raw time. */
            if (0==i || close_every_test) {
                P0 printf("  Opening file...\n");
                P0 printf("    file name is \"%s\"\n", filename);
                unlink(filename);
                status = (dispatch[test].do_open)(filename, &f, FALSE);
                assert(status>=0);
            } else {
                P0 printf("  Using file that was opened before...\n");
                if (ASYNC_META==async && dispatch[test].do_flush) {
                    P0 printf("  Flushing stale data (counted as opening time)...\n");
                    status = (dispatch[test].do_flush)(&f);
                    assert(status>=0);
                }
            }

            /* ==================================== RAW TIME STARTS HERE =================================================== */
#ifdef HAVE_PARALLEL
            MPI_Barrier(MPI_COMM_WORLD);
#endif
            get_current_time(&raw_start);

            if (ASYNC_RAW==async && dispatch[test].do_flush && !close_every_test && i>0) {
                P0 printf("  Flushing stale data (counted as writing time)...\n");
                status = (dispatch[test].do_flush)(&f);
                assert(status>=0);
            }

            P0 printf("  Writing data...\n");
            status = (dispatch[test].do_write)(&f, buffer, recsize, nrecs);
#ifdef HAVE_PARALLEL
            MPI_Barrier(MPI_COMM_WORLD);
#endif

            if (ASYNC_RAW==async && dispatch[test].do_flush && (close_every_test || i+1==repeat)) {
                P0 printf("  Flushing data... (counted as writing time)\n");
                status = (dispatch[test].do_flush)(&f);
                assert(status>=0);
            }
            
            /* ===================================== RAW TIME ENDS HERE ==================================================== */
            get_current_time(&raw_end);
            assert(status>=0);

            /* Close the file unless we're leaving it open for another pass through this loop */
            if (i+1==repeat || close_every_test) {
                P0 printf("  Closing file...\n");
                status = (dispatch[test].do_close)(&f);
                assert(status>=0);
            } else {
                P0 printf("  File close is delayed until later.\n");
            }
            
            /* =================================== OVER ALL TIME ENDS HERE ================================================= */
#ifdef HAVE_PARALLEL
            MPI_Barrier(MPI_COMM_WORLD);
#endif
            get_current_time(&overall_end);

            /* Verification */
            if (verify && dispatch[test].do_read) {
                P0 printf("  Opening file for verification... (not counted in statistics)\n");
                P0 printf("    file name is \"%s\"\n", filename);
                status = (dispatch[test].do_open)(filename, &f, TRUE);
                assert(status>=0);
                P0 printf("  Rereading and verifying data... (not counted in statistics)\n");
                status = (dispatch[test].do_read)(&f, buffer, recsize, nrecs);
                assert(status>=0);
                P0 printf("  Closing file... (not counted in statistics)\n");
                status = (dispatch[test].do_close)(&f);
                assert(status>=0);
            }
            
            /* Results */
            if (0==self) {
                double overall_delta = deltaTime(&overall_start, &overall_end);
                double raw_delta = deltaTime(&raw_start, &raw_end);
                double open_delta = deltaTime(&overall_start, &raw_start);
                double close_delta = deltaTime(&raw_end, &overall_end);
                struct stat sb;

                /* Accumulate statistics for `repeat' loop */
                if (!exclude_boundaries || (i>=2 && i+1<repeat)) {
                    if (0==ntests++) {
                        overall_delta_min = overall_delta_max = overall_delta_total = overall_delta;
                        raw_delta_min = raw_delta_max = raw_delta_total = raw_delta;
                        open_delta_min = open_delta_max = open_delta;
                        close_delta_min = close_delta_max = close_delta;
                    } else {
                        overall_delta_min = MIN(overall_delta_min, overall_delta);
                        overall_delta_max = MAX(overall_delta_max, overall_delta);
                        overall_delta_total += overall_delta;

                        raw_delta_min = MIN(raw_delta_min, raw_delta);
                        raw_delta_max = MAX(raw_delta_max, raw_delta);
                        raw_delta_total += raw_delta;

                        open_delta_min = MIN(open_delta_min, open_delta);
                        open_delta_max = MAX(open_delta_max, open_delta);
                        open_delta_total += open_delta;

                        close_delta_min = MIN(close_delta_min, close_delta);
                        close_delta_max = MAX(close_delta_max, close_delta);
                        close_delta_total += close_delta;
                    }
                }

                /* Print statistics for this particular instance of the test */
                printf("  Statistics for this test:\n");
                printf("    raw data: %ld byte%s * %d record%s * %d task%s = %s\n",
                       (long)recsize, 1==recsize?"":"s", nrecs, 1==nrecs?"":"s", ntasks, 1==ntasks?"":"s", bytes(total_bytes));
                if (stat(filename, &sb)>=0) {
                    printf("    final file size = %s\n", bytes((size_t)sb.st_size));
                    if (sb.st_size) {
                        printf("    storage efficiency: %7.3f%%\n", 100.0*total_bytes/sb.st_size);
                    }
                } else {
                    printf("    cannot stat final output file: %s\n", filename);
                }
                printf("    file open  time = %.6f s\n", open_delta);
                printf("    file close time = %.6f s\n", close_delta);
                printf("    raw I/O:   time = %.6f s,  aggregate bandwidth = %.2f MB/s\n",
                       raw_delta, (total_bytes/raw_delta)/(1024*1024));
                printf("    overall:   time = %.6f s,  aggregate bandwidth = %.2f MB/s\n",
                       overall_delta, (total_bytes/overall_delta)/(1024*1024));
            }
        }

        /* Print statistics for all instances of this test */
        if (0==self && ntests>1) {
            printf("Statistics for %d tests of the %s layer (min/avg/max):\n", ntests, argv[argno]);
            if (exclude_boundaries)
                printf("(Statistics exclude the first two and the last of the %d tests run due to --no-close)\n", repeat);
            printf("  file open time (s):       %10.6f %10.6f %10.6f\n",
                   open_delta_min, open_delta_total/ntests, open_delta_max);
            printf("  file close time (s):      %10.6f %10.6f %10.6f\n",
                   close_delta_min, close_delta_total/ntests, close_delta_max);
            printf("  raw I/O time (s):         %10.6f %10.6f %10.6f\n",
                   raw_delta_min, raw_delta_total/ntests, raw_delta_max);
            printf("  raw I/O bandwidth (MB/s): %10.6f %10.6f %10.6f\n",
                   total_bytes/(raw_delta_max*1024*1024),
                   total_bytes/(raw_delta_total*1024*1024/ntests),
                   total_bytes/(raw_delta_min*1024*1024));
            printf("  overall time (s):         %10.6f %10.6f %10.6f\n",
                   overall_delta_min, overall_delta_total/ntests, overall_delta_max);
            printf("  overall bandwidth (MB/s): %10.6f %10.6f %10.6f\n",
                   total_bytes/(overall_delta_max*1024*1024),
                   total_bytes/(overall_delta_total*1024*1024/ntests),
                   total_bytes/(overall_delta_min*1024*1024));
        }
    }

    /* A little spaghetti here to fix up some exit issues */
    if (FALSE) {
error:
        retval = 1;
    }
done:
#ifdef HAVE_SAF
    saf_final();
#elif defined(HAVE_SSLIB)
    ss_finalize();
#endif
#ifdef HAVE_PARALLEL
    MPI_Finalize();
#endif
    return retval;


}
