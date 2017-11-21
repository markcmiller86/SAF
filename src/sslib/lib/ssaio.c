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
#include "sslib.h"
SS_IF(aio);

#ifdef SSLIB_ASYNC_THREADS
static struct {
    pthread_t           handler;                        /* Thread that handles I/O. We currently assume only one thread */
    hbool_t             handler_started;                /* Is the handler thread running? */
    pthread_mutex_t     mutex;                          /* Mutex to protect this struct */
    pthread_cond_t      cond;                           /* Signaled when `nused' is incremented. */
    ss_aio_t            **req;                          /* Queue of I/O requests not yet started */
    size_t              nalloc;                         /* Allocated size of `req' array */
    size_t              nused;                          /* Number of items in the `req' array */
    int                 do_exit;                        /* Handler thread will exit after all outstanding requests complete */
} ss_aio_g;
#endif

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Asynchronous I/O
 * Purpose:     Initialize the asyncronous I/O subsystem
 *
 * Description: Initialize SSlib's asynchronous I/O layer by spawning an I/O handler thread. This function should not be
 *              called when the asynchronous I/O layer is initialized (it's okay to call this a second time as long as
 *              ss_aio_finalize() was called prior to this second call).
 *
 * Return:      Non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December 17, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_aio_init(void)
{
    SS_ENTER_INIT;

#if defined(SSLIB_ASYNC_AIO)
    if (0==ss_mpi_comm_rank(sslib_g.comm))
        fprintf(sslib_g.warnings, "SSlib: using POSIX AIO for asynchronous I/O\n");
#elif defined(SSLIB_ASYNC_THREADS)
    int         en;

    if (0==ss_mpi_comm_rank(sslib_g.comm))
        fprintf(sslib_g.warnings, "SSlib: using POSIX threads for asynchronous I/O\n");

    /* Initialize global data structures */
    memset(&ss_aio_g, 0, sizeof ss_aio_g);
    pthread_mutex_init(&(ss_aio_g.mutex), NULL);
    pthread_cond_init(&(ss_aio_g.cond), NULL);
    
    /* Start handler thread */
    en = pthread_create(&(ss_aio_g.handler), NULL, ss_aio_handler_cb, NULL);
    if (en) SS_ERROR_FMT(FAILED, ("pthread_created failed: %s", strerror(en)));
    ss_aio_g.handler_started = TRUE;
SS_CLEANUP:
#else
    if (0==ss_mpi_comm_rank(sslib_g.comm))
        fprintf(sslib_g.warnings, "SSlib: faking asynchronous I/O\n");
#endif
    SS_LEAVE(0);
}

#ifdef SSLIB_ASYNC_THREADS
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Asynchronous I/O
 * Purpose:     I/O handler function
 *
 * Description: This function is executed by the I/O handler thread, which sits in a loop removing items from the head of the
 *              request queue and doing synchronous I/O operations for each request. The thread exits after ss_aio_finalize()
 *              is called provided that all requests have been processed.
 *
 * Note:        This function is executed by a thread and therefore should not call SSlib, HDF5, or MPI functions in order to
 *              be safe.
 *
 * Return:      Returns the RETVAL argument.
 *
 * Parallel:    Independent thread creation callback.
 *
 * Programmer:  Robb Matzke
 *              Friday, December 17, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
ss_aio_handler_cb(void *retval)
{
    /* SS_ENTER() -- not required */

    ss_aio_t    *req;

    while (1) {
        /* Wait until there are requests to process */
        pthread_mutex_lock(&(ss_aio_g.mutex));
        while (0==ss_aio_g.nused && !ss_aio_g.do_exit) {
            pthread_cond_wait(&(ss_aio_g.cond), &(ss_aio_g.mutex));
        }

        /* Exit thread if necessary. */
        if (0==ss_aio_g.nused) {
            pthread_mutex_unlock(&(ss_aio_g.mutex));
            break;
        }

        /* Remove a request from the head of the queue while we're holding the queue mutex */
        req = ss_aio_g.req[0];
        --ss_aio_g.nused;
        memmove(ss_aio_g.req, ss_aio_g.req+1, ss_aio_g.nused*sizeof(ss_aio_g.req[0]));
        pthread_mutex_unlock(&(ss_aio_g.mutex));

        /* Do the I/O */
        if (lseek(req->aio_fildes, req->aio_offset, SEEK_SET)<0) {
            req->errnum = errno;
        } else {
            size_t size = req->aio_nbytes;
            char *buf = req->aio_buf;
            while (size>0) {
                ssize_t n = write(req->aio_fildes, buf, size);
                if (n<0 && EINTR==errno) continue;
                if (n<0) {
                    req->errnum = errno;
                    break;
                }
                size -= (size_t)n;
                buf += n;
            }
            if (0==size) req->errnum = 0;
        }

        /* Mark the request as being done */
        pthread_mutex_unlock(&(req->in_progress));
    }
    return retval;
}
#endif
    
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Asynchronous I/O
 * Purpose:     Terminate the asynchronous I/O subsystem
 *
 * Description: This function blocks until all pending requests have completed and the handler thread has exited.
 *
 * Return:      Non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December 17, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_aio_finalize(void)
{
    SS_ENTER(ss_aio_finalize, herr_t);

#ifdef SSLIB_ASYNC_THREADS
    if (ss_aio_g.handler_started) {
        int en;
        pthread_mutex_lock(&(ss_aio_g.mutex));
        ss_aio_g.do_exit = 1;
        pthread_cond_signal(&(ss_aio_g.cond));
        pthread_mutex_unlock(&(ss_aio_g.mutex));
    
        en = pthread_join(ss_aio_g.handler, NULL);
        if (en) SS_ERROR_FMT(FAILED, ("pthread_join failed: %s", strerror(en)));
    }
SS_CLEANUP:
#endif
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Asynchronous I/O
 * Purpose:     Initiate a write operation
 *
 * Description: Adds a write request to the queue of pending writes and immidiately returns.
 *
 * Return:      Returns non-negative on success; negative on failure.  The AIO argument is modified during the execution of
 *              the I/O operation.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December 17, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_aio_write(ss_aio_t *aio)
{
    SS_ENTER(ss_aio_write, herr_t);

#if defined(SSLIB_ASYNC_AIO)
    if (aio_write(aio)<0)
        SS_ERROR_FMT(FAILED, ("aio_write failed: %s", strerror(errno)));
SS_CLEANUP:
    
#elif defined(SSLIB_ASYNC_THREADS)
    hbool_t locked=FALSE;

    /* Initialize the aio control struct */
    aio->errnum = EINPROGRESS;
    pthread_mutex_init(&(aio->in_progress), NULL);
    pthread_mutex_lock(&(aio->in_progress));

    /* Append the new request to the queue */
    pthread_mutex_lock(&(ss_aio_g.mutex));
    locked = TRUE;
    if (ss_aio_g.nused>=ss_aio_g.nalloc)
        SS_EXTEND(ss_aio_g.req, MAX(64,ss_aio_g.nused+1), ss_aio_g.nalloc);
    ss_aio_g.req[ss_aio_g.nused++] = aio;
    pthread_cond_signal(&(ss_aio_g.cond));
    pthread_mutex_unlock(&(ss_aio_g.mutex));
SS_CLEANUP:
    if (locked) pthread_mutex_unlock(&(ss_aio_g.mutex));

#else
    size_t size = aio->aio_nbytes;
    char *buf = aio->aio_buf;
    if (lseek(aio->aio_fildes, aio->aio_offset, SEEK_SET)<0)
        SS_ERROR_FMT(FAILED, ("lseek failed: %s", strerror(errno)));
    while (size>0) {
        ssize_t n = write(aio->aio_fildes, buf, size);
        if (n<0 && EINTR==errno) continue;
        if (n<0) {
            aio->errnum = errno;
            SS_ERROR_FMT(FAILED, ("write failed: %s", strerror(errno)));
        }
        size -= (size_t)n;
        buf += n;
    }
    if (0==size) aio->errnum = 0;
SS_CLEANUP:
#endif
    
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Asynchronous I/O
 * Purpose:     Block until requests complete
 *
 * Description: Blocks until the specified I/O requests have completed.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December 17, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_aio_suspend(ss_aio_t UNUSED **aio, size_t UNUSED nreq)
{
    SS_ENTER(ss_aio_suspend, herr_t);
    
#if defined(SSLIB_ASYNC_AIO)
    if (aio_suspend(aio, (int)nreq, NULL)<0)
        SS_ERROR_FMT(FAILED, ("aio_suspend failed: %s", strerror(errno)));
SS_CLEANUP:
#elif defined(SSLIB_ASYNC_THREADS)
    size_t              i;
    for (i=0; i<nreq; i++) {
        pthread_mutex_lock(&(aio[i]->in_progress));
        pthread_mutex_unlock(&(aio[i]->in_progress));
    }
#else
    /* Nothing to do for faked async I/O */
#endif
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Asynchronous I/O
 * Purpose:     Returns current status of a request
 *
 * Description: Returns the current status of a request.
 *
 * Return:      Returns the constant EINPRGRESS if a request is in progress, zero if the request has completed successfully,
 *              or some other error value if an error occurred.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December 17, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_aio_error(ss_aio_t *aio)
{
#if defined(SSLIB_ASYNC_AIO)
    return aio_error(aio);
#elif defined(SSLIB_ASYNC_THREADS)
    return aio->errnum;
#else
    return aio->errnum;
#endif
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Asynchronous I/O
 * Purpose:     Reaps resources for completed I/O requests
 *
 * Description: Once the specified request has completed (i.e., when ss_aio_error() returns something other than EINPROGRESS)
 *              then this function should be called to clean up resources associated with the request.  This function should
 *              be called exactly once for each request.
 *
 * Note:        Although the POSIX aio_return() function should only be called when a request has completed (i.e., when
 *              aio_error() returns something other than EINPROGRESS), the SSlib version can be called on any request and will
 *              block until the request completes.
 *
 * Return:      Same return value as ss_aio_error().
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December 17, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_aio_return(ss_aio_t *aio)
{
    int         retval;

#if defined(SSLIB_ASYNC_AIO)
    retval = aio_return(aio);
#elif defined(SSLIB_ASYNC_THREADS)
    pthread_mutex_lock(&(aio->in_progress));
    retval = aio->errnum;
    assert(EINPROGRESS!=retval);
    pthread_mutex_unlock(&(aio->in_progress));
    pthread_mutex_destroy(&(aio->in_progress));
#else
    retval = aio->errnum;
#endif
    return retval;
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Asynchronous I/O
 * Purpose:     HDF5 async callback
 *
 * Description: HDF5 invokes this function when it wants to do asynchronous I/O
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, December 17, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_aio_hdf5_cb(int fd, hid_t dxpl, haddr_t addr, size_t size, const void *buf)
{
    SS_ENTER(ss_aio_hdf5_cb, herr_t);
    ss_aio_t    *req;
    int         retval=0;

    if (H5Pget(dxpl, "async_req", &req)<0) SS_ERROR_FMT(HDF5, ("no async_req property"));
    if (!req || req->aio_buf!=buf) {
        retval = -1; /* cause HDF5 to process request synchronously, but this is not an error condition */
        goto done;
    }
    req->aio_fildes = fd;
    req->aio_offset = addr;
    req->aio_nbytes = size;
    if (ss_aio_write(req)>=0) {
        req = NULL;
        H5Pset(dxpl, "async_req", &req);
    }

done:
SS_CLEANUP:
    SS_LEAVE(retval);
}
    
