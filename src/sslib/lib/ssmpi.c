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
SS_IF(mpi);

MPI_Datatype ss_mpi_haddr_mt = MPI_DATATYPE_NULL;
MPI_Datatype ss_mpi_hsize_mt = MPI_DATATYPE_NULL;
MPI_Datatype ss_mpi_size_mt = MPI_DATATYPE_NULL;
MPI_Datatype ss_mpi_htri_mt = MPI_DATATYPE_NULL;

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 *
 * Description: The entities documented in this chapter provide extended message passing functionality.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Interface initializer
 *
 * Description: Initializes the mpi support interface.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, August  1, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_mpi_init(void)
{
    SS_ENTER_INIT;

    if (H5Tmpi(H5T_NATIVE_HADDR, &ss_mpi_haddr_mt)<0) SS_ERROR(INIT);
    if (H5Tmpi(H5T_NATIVE_HSIZE, &ss_mpi_hsize_mt)<0) SS_ERROR(INIT);
    if (H5Tmpi(H5T_NATIVE_SIZE, &ss_mpi_size_mt)<0) SS_ERROR(INIT);

#if 0 /*Not defined yet in HDF5 1.7.15*/
    if (H5Tmpi(H5T_NATIVE_HTRI, &ss_mpi_htri_mt)<0) SS_ERROR(INIT);
#else
    assert(sizeof(int)==sizeof(htri_t));
    ss_mpi_htri_mt = MPI_INT;
#endif

SS_CLEANUP:
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Test whether communicator is a subset
 *
 * Description: Tests whether the tasks in SUBCOMM are a subset of the tasks in COMM.
 *
 * Return:      Returns true (positive) if SUBCOMM is a subset of COMM, false if not. Returns negative on error.
 *
 * Parallel:    Independent. Serial behavior is to always return true as if both communicators were MPI_COMM_SELF.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
htri_t
ss_mpi_subcomm(MPI_Comm UNUSED_SERIAL subcomm,  /* The subset of tasks. */
               MPI_Comm UNUSED_SERIAL comm      /* The superset of tasks. */
               )
{
    SS_ENTER(ss_mpi_subcomm, htri_t);
    htri_t              retval = TRUE;
#ifdef HAVE_PARALLEL
    int                 b, i, ntasks, *ranks1=NULL, *ranks2=NULL;
    MPI_Group           g1=MPI_GROUP_NULL, g2=MPI_GROUP_NULL;

    retval = FALSE;     /* When MPI support is present we assume false and prove otherwise. */

    /* Both communicators must be intracommunicators */
    if (MPI_Comm_test_inter(subcomm, &b)) SS_ERROR(MPI);
    if (b) SS_ERROR_FMT(NOTIMP, ("subcomm refers to an intercommunicator"));
    if (MPI_Comm_test_inter(comm, &b)) SS_ERROR(MPI);
    if (b) SS_ERROR_FMT(NOTIMP, ("comm refers to an intercommunicator"));

    /* Get the group associated with each communicator */
    if (MPI_Comm_group(subcomm, &g1)) SS_ERROR(MPI);
    if (MPI_Comm_group(comm, &g2)) SS_ERROR(MPI);

    /* Translate ranks of subcomm into ranks in comm and make sure they all translate */
    if (MPI_Group_size(g1, &ntasks)) SS_ERROR(MPI);
    if (ntasks) {
        if (NULL==(ranks1 = malloc(ntasks*sizeof(*ranks1)))) SS_ERROR(RESOURCE);
        if (NULL==(ranks2 = malloc(ntasks*sizeof(*ranks2)))) SS_ERROR(RESOURCE);
        for (i=0; i<ntasks; i++) ranks1[i] = i;
        if (MPI_Group_translate_ranks(g1, ntasks, ranks1, g2, ranks2)) SS_ERROR(MPI);
        for (i=0; i<ntasks; i++) {
            if (MPI_UNDEFINED==ranks2[i]) goto done;
        }
    }
    retval = TRUE;

 done:
    ranks1 = SS_FREE(ranks1);
    ranks2 = SS_FREE(ranks2);
    if (MPI_Group_free(&g1)) SS_ERROR(MPI);
    g1 = MPI_GROUP_NULL;
    if (MPI_Group_free(&g2)) SS_ERROR(MPI);
    g2 = MPI_GROUP_NULL;
    
 SS_CLEANUP:
    SS_FREE(ranks1);
    SS_FREE(ranks2);
    if (g1!=MPI_GROUP_NULL) MPI_Group_free(&g1);
    if (g2!=MPI_GROUP_NULL) MPI_Group_free(&g2);
#endif /*HAVE_PARALLEL*/

    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Remap task number
 *
 * Description: Given two communicators, one which is a subset of the other, and a task rank in the subset, return the rank of
 *              that same task in the superset.
 *
 * Return:      Returns the rank in COMM of the SUBCOMM task TASK. Returns negative on failure.
 *
 * Parallel:    Independent. Serial behavior is to return zero as if TASK were zero and both communicators were MPI_COMM_SELF.
 *
 * Programmer:  Robb Matzke
 *              Tuesday, September 16, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
int
ss_mpi_maptask(int UNUSED_SERIAL task,          /* Task number with respect to SUBCOMM. */
               MPI_Comm UNUSED_SERIAL subcomm,  /* Sub communicator of COMM. */
               MPI_Comm UNUSED_SERIAL comm      /* The whole communicator. */
               )
{
    SS_ENTER(ss_mpi_maptask, int);
    int                 retval=0;
#ifdef HAVE_PARALLEL
    int                 b;
    MPI_Group           g1=MPI_GROUP_NULL, g2=MPI_GROUP_NULL;

    /* Both communicators must be intracommunicators */
    if (MPI_Comm_test_inter(subcomm, &b)) SS_ERROR(MPI);
    if (b) SS_ERROR_FMT(NOTIMP, ("subcomm refers to an intercommunicator"));
    if (MPI_Comm_test_inter(comm, &b)) SS_ERROR(MPI);
    if (b) SS_ERROR_FMT(NOTIMP, ("comm refers to an intercommunicator"));

    /* Get the group associated with each communicator */
    if (MPI_Comm_group(subcomm, &g1)) SS_ERROR(MPI);
    if (MPI_Comm_group(comm, &g2)) SS_ERROR(MPI);

    /* Do the translation */
    if (MPI_Group_translate_ranks(g1, 1, &task, g2, &retval)) SS_ERROR(MPI);
    if (MPI_UNDEFINED==retval) SS_ERROR_FMT(USAGE, ("subcomm is not a subset of comm"));

    /* Successful cleanup */
    if (MPI_Group_free(&g1)) SS_ERROR(MPI);
    g1 = MPI_GROUP_NULL;
    if (MPI_Group_free(&g2)) SS_ERROR(MPI);
    g2 = MPI_GROUP_NULL;

 SS_CLEANUP:
    if (g1!=MPI_GROUP_NULL) MPI_Group_free(&g1);
    if (g2!=MPI_GROUP_NULL) MPI_Group_free(&g2);
#endif /*HAVE_PARALLEL*/
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Determine if task is extra
 *
 * Description: Certain functions in SSlib must be called collectively over a file communicator for the sake of satisfying
 *              HDF5 requirements. The SSlib collectivity requirements in the absense of HDF5 constraints would be
 *              collectivity over a scope's communicator.  The policy adopted by SSlib is that any task that's calling a
 *              function soley to satisfy the HDF5 requirements should pass a top-scope handle in place of the persistent
 *              object handled passed by the other tasks.  For instance, if a collective operation is being performed on a
 *              particular field then all tasks of that field's scope are most likely required to participate in the call. But
 *              if that operation invokes HDF5 file-collective operations then all tasks of the file's communicator (which is
 *              a superset of the scope's communicator) must participate. These "extra" tasks should all pass the link to the
 *              top scope of the field's file instead of the link to the field itself. In fact, the extra tasks might not even
 *              have a link to the field!
 *
 *              This function looks at PERSP and if it points to a link that points to a scope then we assume the caller is an
 *              extra task.
 *
 * Return:      Returns a link to the top scope, either BUFFER or a newly allocated handle.  If the caller is an extra task
 *              then PERSP is set to the null pointer. Returns the null pointer on error.
 *
 * Parallel:    Independent, although normally called collectively across a file communicator.
 *
 * Programmer:  Robb Matzke
 *              Thursday, October  2, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_scope_t *
ss_mpi_extras(ss_pers_t **persp,                /* Pointer to a persistent object link pointer. */
              ss_scope_t *buffer                /* Buffer in which to place the returned scope link. */
              )
{
    SS_ENTER(ss_mpi_extras, ss_scope_tP);
    if (!buffer) SS_ERROR_FMT(NOTIMP, ("BUFFER must be non-null"));

    if (SS_MAGIC(ss_scope_t)==SS_MAGIC_OF(*persp)) {
        ss_scope_t *topscope = *(ss_scope_t**)persp;
        if (!ss_scope_isopentop(topscope)) SS_ERROR_FMT(USAGE, ("passed scope is not an open top scope"));
        *buffer = *topscope;
        *persp = NULL;
    } else {
        if (!ss_pers_topscope(*persp, buffer)) SS_ERROR(FAILED);
    }
SS_CLEANUP:
    SS_LEAVE(buffer);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Gather to all
 *
 * Description: This function is almost identical to MPI_Allgather() with MPI_IN_PLACE as the send buffer, except that the
 *              COUNT is size_t instead of !int and an herr_t value is returned in order to mesh better with the rest of SSlib.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across COMM. In serial mode this is a no-op since the data is already at the beginning of BUFFER.
 *
 * Programmer:  Robb Matzke
 *              Saturday, October  4, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_mpi_allgather(void UNUSED_SERIAL *buffer,            /* INOUT: exchanged data */
                 size_t UNUSED_SERIAL count,            /* Number of elements to exchange per task */
                 MPI_Datatype UNUSED_SERIAL type,       /* Datatype of an element */
                 MPI_Comm UNUSED_SERIAL comm            /* Communicator over which to operate */
                 )
{
    SS_ENTER(ss_mpi_allgather, herr_t);

#ifdef HAVE_PARALLEL
    static void         *send_buffer=NULL;      /* Send buffer. Keep it around to reduce load on malloc() */
    static size_t       send_nalloc=0;          /* Current allocated size of send_buffer */
    MPI_Aint            tsize;                  /* Size in bytes of dataset, always positive */
    int                 self;                   /* Calling task's rank in COMM */

    if (MPI_Comm_rank(comm, &self)) SS_ERROR(MPI);

    /* We can't use MPI_IN_PLACE because it's MPI-2. Therefore copy the send information into its own buffer so that
     * MPI can copy it back into the result buffer. */
    if (MPI_Type_extent(type, &tsize)) SS_ERROR(MPI);
    SS_ASSERT(tsize>0);
    if (count*tsize>send_nalloc) {
        send_nalloc = count*tsize;
        if (NULL==(send_buffer=malloc(send_nalloc))) SS_ERROR(RESOURCE);
    }
    memcpy(send_buffer, (char*)buffer+self*count*tsize, count*tsize);

    if (MPI_Allgather(send_buffer, (int)count, type, buffer, (int)count, type, comm)) SS_ERROR(MPI);
SS_CLEANUP:
#endif /*HAVE_PARALLEL*/
    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Vector gather to all
 *
 * Description: This function is almost identical to MPI_Allgatherv() with MPI_IN_PLACE as the send buffer. The differences
 *              are that the COUNT and OFFSET arrays are of type size_t and the return value is an herr_t in order to mesh better
 *              with SSlib.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across COMM. In serial mode this function does nothing since the data is already at the beginning
 *              of BUFFER.
 *
 * Programmer:  Robb Matzke
 *              Saturday, October  4, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_mpi_allgatherv(void UNUSED_SERIAL *buffer,           /* INOUT: exchanged data */
                  const size_t UNUSED_SERIAL *count,    /* How many elements to exchange per task */
                  const size_t UNUSED_SERIAL *offset,   /* Starting element index into BUFFER per task */
                  MPI_Datatype UNUSED_SERIAL type,      /* Datatype of an element */
                  MPI_Comm UNUSED_SERIAL comm           /* Communicator over which to operate */
                  )
{
    SS_ENTER(ss_mpi_allgatherv, herr_t);
#ifdef HAVE_PARALLEL
    static void         *send_buffer=NULL;      /* Send buffer. Keep it around to reduce load on malloc() */
    static size_t       send_nalloc=0;          /* Current allocated size of send_buffer */
    static int          *my_count=NULL;         /* The int version of COUNT */
    static int          *my_offset=NULL;        /* The int version of OFFSET */
    static size_t       max_ntasks=0;           /* Allocated sizes of my_count and my_offset */
    MPI_Aint            tsize;                  /* Size in bytes of dataset, always positive */
    int                 self, ntasks;           /* Calling task's rank in COMM and COMM size */
    int                 i;

    if (MPI_Comm_rank(comm, &self)) SS_ERROR(MPI);
    if (MPI_Comm_size(comm, &ntasks)) SS_ERROR(MPI);

    /* We can't use MPI_IN_PLACE because it's MPI-2. Therefore copy the send information into its own buffer so that
     * MPI can copy it back into the result buffer. */
    if (MPI_Type_extent(type, &tsize)) SS_ERROR(MPI);
    SS_ASSERT(tsize>0);
    if ((size_t)tsize*count[self]>send_nalloc) {
        send_nalloc = tsize * count[self];
        if (NULL==(send_buffer=malloc(send_nalloc))) SS_ERROR(RESOURCE);
    }
    memcpy(send_buffer, (char*)buffer+offset[self]*tsize, count[self]*tsize);

    /* Copy COUNT and OFFSET values into an `int' array for MPI. Also get rid of the `const' qualifier. */
    if ((size_t)ntasks>max_ntasks) {
        if (NULL==(my_count=malloc(ntasks*sizeof(*my_count)))) SS_ERROR(RESOURCE);
        if (NULL==(my_offset=malloc(ntasks*sizeof(*my_offset)))) SS_ERROR(RESOURCE);
        max_ntasks = ntasks;
    }
    for (i=0; i<ntasks; i++) {
        SS_ASSERT(count[i]<=INT_MAX);
        my_count[i] = (int)(count[i]);
        SS_ASSERT(offset[i]<=INT_MAX);
        my_offset[i] = (int)(offset[i]);
    }

    /* Call MPI */
    if (MPI_Allgatherv(send_buffer, my_count[self], type, buffer, my_count, my_offset, type, comm)) SS_ERROR(MPI);
SS_CLEANUP:
#endif /*HAVE_PARALLEL*/
    SS_LEAVE(0);
}

#ifdef HAVE_PARALLEL
/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Convert stride to MPI datatype
 *
 * Description: Converts an array of stride objects into a single MPI datatype.
 *
 * Return:      Returns non-negative on success with the result datatype returned by reference; returns negative on failure.
 *
 * Parallel:    Independent
 *
 * Issue:       MPICH-1.2.5 doesn't have MPI_Type_dup(), which means it's not feasible for SSlib to release MPI datatypes when
 *              they are no longer used.
 *
 * Programmer:  Robb Matzke
 *              Monday, October 27, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
herr_t
ss_mpi_type_create_stride(int nstrides,                 /* Number of elements in S array */
                          const ss_blob_stride_t *s,    /* Strides */
                          MPI_Datatype type,            /* The base datatype */
                          MPI_Datatype *result          /* The result on successful return */
                          )
{
    SS_ENTER(ss_mpi_type_create_stride, herr_t);
    MPI_Datatype        mpistride[H5S_MAX_RANK*2];      /* MPI equivalent types for each stride object in s[] */
    int                 blklen[H5S_MAX_RANK*2];         /* Set to all 1's */
    MPI_Aint            blkdisp[H5S_MAX_RANK*2];        /* Offset into the aggregation buffer for first selected elmt */
    int                 nmpi=0;                         /* Number of mpistride[] elements set */
    MPI_Datatype        temp=MPI_DATATYPE_NULL;
    MPI_Aint            tsize;                          /* Size in bytes of a single dataset element */
    int                 i, sts;                         /* Bytes from start of one type to the start of next instance */

    SS_ASSERT(nstrides>0 && nstrides<=H5S_MAX_RANK*2);

    if (MPI_Type_extent(type, &tsize)) SS_ERROR(MPI);
    for (nmpi=0; nmpi<nstrides; nmpi++) {
        if (s[nmpi].duplicity==1) {
            mpistride[nmpi] = type;
        } else {
            if (MPI_Type_contiguous((int)s[nmpi].duplicity, type, mpistride+nmpi)) SS_ERROR(MPI);
            if (MPI_Type_commit(mpistride+nmpi)) SS_ERROR(MPI);
        }

        for (i=s[nmpi].ndims-1, sts=0; i>=0; --i) {
            sts += s[nmpi].stride[i];
            temp = mpistride[nmpi];
            if (MPI_Type_hvector((int)(s[nmpi].count[i]), 1, sts*tsize, temp, mpistride+nmpi)) SS_ERROR(MPI);
            if (MPI_Type_commit(mpistride+nmpi)) SS_ERROR(MPI);
            sts *= s[nmpi].count[i];
        }
        blklen[nmpi] = 1;
        blkdisp[nmpi] = (MPI_Aint)(s[nmpi].offset * tsize); /*conversion ok because these are memory offsets and sizes*/
    }
    if (MPI_Type_struct(nmpi, blklen, blkdisp, mpistride, result)) SS_ERROR(MPI);
    if (MPI_Type_commit(result)) SS_ERROR(MPI);

SS_CLEANUP:
    SS_LEAVE(0);
}
#endif /*HAVE_PARALLEL*/

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Returns callers rank in communicator
 *
 * Description: Similar to MPI_Comm_rank() except it returns the rank by value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent.  Serial behavior is to always succeed returning zero as if COMM were MPI_COMM_SELF.
 *
 * Programmer:  Robb Matzke
 *              Friday, December 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
int
ss_mpi_comm_rank(MPI_Comm UNUSED_SERIAL comm)
{
    SS_ENTER(ss_mpi_comm_rank, int);
    int retval=0;
    
#ifdef HAVE_PARALLEL
    if (MPI_Comm_rank(comm, &retval)) SS_ERROR(MPI);
SS_CLEANUP:
#endif
    
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Returns size of communicator
 *
 * Description: Similar to MPI_Comm_size() except it returns the size by value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent. Serial behavior is to always return one as if COMM were MPI_COMM_SELF.
 *
 * Programmer:  Robb Matzke
 *              Friday, December 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
int
ss_mpi_comm_size(MPI_Comm UNUSED_SERIAL comm)
{
    SS_ENTER(ss_mpi_comm_size, int);
    int retval=1;

#ifdef HAVE_PARALLEL
    if (MPI_Comm_size(comm, &retval)) SS_ERROR(MPI);
SS_CLEANUP:
#endif

    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Broadcast a buffer to other tasks
 *
 * Description: Similar to MPI_Bcast() except in return value and that the COUNT argument is type size_t.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across COMM. Serial behavior is a no-op as if COMM were MPI_COMM_SELF.
 *
 * Programmer:  Robb Matzke
 *              Friday, December 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_mpi_bcast(void UNUSED_SERIAL *buffer, size_t UNUSED_SERIAL count, MPI_Datatype UNUSED_SERIAL datatype,
             int UNUSED_SERIAL root, MPI_Comm UNUSED_SERIAL comm)
{
    SS_ENTER(ss_mpi_bcast, herr_t);
#ifdef HAVE_PARALLEL
    int         my_count = (int)count;
    SS_ASSERT(count<=INT_MAX);

    if (my_count>0 && MPI_Bcast(buffer, my_count, datatype, root, comm)) SS_ERROR(MPI);
SS_CLEANUP:
#endif

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Barrier synchronization
 *
 * Description: Similar to MPI_Barrier() except in return value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective across COMM. Serial behavior is a no-op as if COMM were MPI_COMM_SELF.
 *
 * Programmer:  Robb Matzke
 *              Friday, December 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_mpi_barrier(MPI_Comm UNUSED_SERIAL comm)
{
    SS_ENTER(ss_mpi_barrier, herr_t);

#ifdef HAVE_PARALLEL
    if (MPI_Barrier(comm)) SS_ERROR(MPI);
SS_CLEANUP:
#endif

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Duplicate communicator
 *
 * Description: Similar to MPI_Comm_dup() except in return value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective over COMM. Serial behavior is a no-op.
 *
 * Programmer:  Robb Matzke
 *              Friday, December 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_mpi_comm_dup(MPI_Comm comm, MPI_Comm *duped)
{
    SS_ENTER(ss_mpi_comm_dup, herr_t);

#ifdef HAVE_PARALLEL
    if (MPI_Comm_dup(comm, duped)) SS_ERROR(MPI);
SS_CLEANUP:
#else
    *duped = comm;
#endif

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Free a communicator
 *
 * Description: Similar to MPI_Comm_free() except in return value.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Collective over COMM. Serial behavior is a no-op.
 *
 * Programmer:  Robb Matzke
 *              Friday, December 12, 2003
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
herr_t
ss_mpi_comm_free(MPI_Comm UNUSED_SERIAL *comm)
{
    SS_ENTER(ss_mpi_comm_free, herr_t);

#ifdef HAVE_PARALLEL
    if (MPI_Comm_free(comm)) SS_ERROR(MPI);
SS_CLEANUP:
#endif

    SS_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Obtain a unique serial number
 *
 * Description: This function generates a unique serial number and returns the same number to all tasks of COMM that called
 *              it collectively.  This is returned as a size_t value where the high-order bits represent a task number and the
 *              remaining bits are a serial number within that task.
 *
 *              Note that serial numbers in this context are not sequential because requiring such would also require this
 *              function to be library-collective.
 *
 * Return:      Returns a unique serial number.
 *
 * Parallel:    Collective across COMM. The same serial number is returned to all callers. Returns SS_NOSIZE on failure.
 *
 * Programmer:  Robb Matzke
 *              Friday, January  9, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
size_t
ss_mpi_serial(MPI_Comm UNUSED_SERIAL comm)
{
    SS_ENTER(ss_mpi_serial, size_t);
    int                 self_lib, ntasks_lib;   /* rank and size of library communicator */
    int                 self;                   /* rank of caller in COMM */
    size_t              retval;
    static size_t       my_serial;
    static size_t       nbits;                  /* bits used for the serial number within each task */

    if (0==my_serial) {
        /* Initialization */
        if ((self_lib=ss_mpi_comm_rank(sslib_g.comm))<0) SS_ERROR(FAILED);
        if ((ntasks_lib=ss_mpi_comm_size(sslib_g.comm))<0) SS_ERROR(FAILED);
        for (nbits=1; nbits<8*sizeof(int); nbits++) if (ntasks_lib<=(1<<nbits)) break;
        nbits = 8*sizeof(my_serial) - nbits;
        my_serial = (size_t)self_lib<<nbits;
    }

    /* Task zero of COMM increments its static serial number and broadcasts it to the other tasks. */
    if ((self=ss_mpi_comm_rank(comm))<0) SS_ERROR(FAILED);
    if (0==self) retval = my_serial++;
    if (ss_mpi_bcast(&retval, 1, MPI_SIZE_T, 0, comm)<0) SS_ERROR(FAILED);
        
SS_CLEANUP:
    SS_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     MPI Support Functions
 * Purpose:     Elect a controlling task
 *
 * Description: Sometimes we need to communicate data across a file communicator on behalf of some object. For instance,
 *              extending a blob often means that the file communicator for the blob needs to know the new dataset size since
 *              H5Dextend() is a file-collective operation. This function is called by all tasks of a file communicator in
 *              order to elect a specific task that will serve as the root task for broadcasts. The constraint is that the
 *              root task must be one of the tasks that own the object in question.
 *
 * Return:      Returns a task rank within the communicator for the file containing OBJ; returns negative on failure.
 *
 * Parallel:    Collective across the file containing OBJ. For any calling task that does not own OBJ (i.e., the scope of OBJ
 *              is not open on that task) it should pass the top scope for the file in question.
 *
 * Programmer:  Robb Matzke
 *              Monday, June 28, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
ss_mpi_elect(ss_pers_t *obj)
{
    SS_ENTER(ss_mpi_elect, int);
    ss_scope_t  topscope=SS_SCOPE_NULL;
    ss_scope_t  objscope=SS_SCOPE_NULL;
    MPI_Comm    filecomm=SS_COMM_NULL;
    MPI_Comm    objcomm=SS_COMM_NULL;
    int         root=0, retval=-1;
    
    if (!ss_mpi_extras(&obj, &topscope)) SS_ERROR(FAILED);
    if (ss_scope_comm(&topscope, &filecomm, NULL, NULL)<0) SS_ERROR(FAILED);
    if (obj) {
        if (NULL==ss_pers_scope(obj, &objscope)) SS_ERROR(FAILED);
        if (ss_scope_comm(&objscope, &objcomm, NULL, NULL)<0) SS_ERROR(FAILED);
        if ((root=ss_mpi_maptask(0, objcomm, filecomm))<0) SS_ERROR(FAILED);
    }
#ifdef HAVE_PARALLEL
    if (MPI_Allreduce(&root, &retval, 1, MPI_INT, MPI_MAX, filecomm)) SS_ERROR(MPI);
#else
    retval = root;
#endif

SS_CLEANUP:
    SS_LEAVE(retval);
}
