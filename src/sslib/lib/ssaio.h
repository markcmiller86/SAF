/*
 * Copyright(C) 2004-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 2004-2005 Sandia Corporation.
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
 *     Robb P. Matzke              LLNL
 *     Eric A. Illescas            SNL
 * 
 * Acknowledgements:
 *     Mark C. Miller              LLNL - Design input
 * 
 */
#ifndef SS_HAVE_SSAIO_H
#define SS_HAVE_SSAIO_H

/* #define SSLIB_ASYNC_FAKE */ /*DEBUGGING*/

/* This API provides a portable asynchronous I/O interface using POSIX AIO, POSIX threads, or by faking things by doing
 * synchronous I/O. A particular method can be selected by defining SSLIB_ASYNC_AIO, SSLIB_ASYNC_THREADS or SSLIB_ASYNC_FAKE
 * prior to including this file. If none of these are defined then we prefer AIO above threads, but fall back to synchronous
 * I/O if support for neither one is available. */
#if defined(SSLIB_ASYNC_AIO)
#   undef  SSLIB_ASYNC_THREADS
#   undef  SSLIB_ASYNC_FAKE
#elif defined(SSLIB_ASYNC_THREADS)
#   undef  SSLIB_ASYNC_AIO
#   undef  SSLIB_ASYNC_FAKE
#elif defined(SSLIB_ASYNC_FAKE)
#   undef  SSLIB_ASYNC_AIO
#   undef  SSLIB_ASYNC_THREADS
#elif defined(HAVE_AIO_H)
#   define SSLIB_ASYNC_AIO
#   undef  SSLIB_ASYNC_THREADS
#   undef  SSLIB_ASYNC_FAKE
#elif defined(HAVE_PTHREAD_H)
#   define SSLIB_ASYNC_THREADS
#   undef  SSLIB_ASYNC_AIO
#   undef  SSLIB_ASYNC_FAKE
#else
#   define SSLIB_ASYNC_FAKE
#   undef  SSLIB_ASYNC_AIO
#   undef  SSLIB_ASYNC_THREADS
#endif

/* Define datatypes */
#if defined(SSLIB_ASYNC_AIO)
#   include <aio.h>
#   define ss_aio_t struct aiocb
#elif defined(SSLIB_ASYNC_THREADS)
#   include <pthread.h>
    typedef struct {
        int             aio_fildes;                     /* File descriptor where I/O will occur */
        off_t           aio_offset;                     /* File address where I/O will occur */
        size_t          aio_nbytes;                     /* Bytes to read/write */
        void            *aio_buf;                       /* Buffer to contain data */
        int             errnum;                         /* Error number or EINPROGRESS */
        pthread_mutex_t in_progress;                    /* Lock held while I/O is in progress */
    } ss_aio_t;
#else
    typedef struct {
        int             aio_fildes;
        off_t           aio_offset;
        size_t          aio_nbytes;
        void            *aio_buf;
        int             errnum;
    } ss_aio_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
herr_t ss_aio_init(void);
void *ss_aio_handler_cb(void *retval);
herr_t ss_aio_finalize(void);
herr_t ss_aio_write(ss_aio_t *aio);
herr_t ss_aio_suspend(ss_aio_t **aio, size_t nreq);
int ss_aio_error(ss_aio_t *aio);
int ss_aio_return(ss_aio_t *aio);
herr_t ss_aio_hdf5_cb(int fd, hid_t dxpl, haddr_t addr, size_t size, const void *buf);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSAIO_H*/
