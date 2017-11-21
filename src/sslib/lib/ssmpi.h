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
#ifndef SS_HAVE_SSMPI_H
#define SS_HAVE_SSMPI_H

/* Additional MPI datatypes not defined in the standard */
extern MPI_Datatype             ss_mpi_haddr_mt;
#define MPI_HADDR_T             (ss_mpi_init(), ss_mpi_haddr_mt)
extern MPI_Datatype             ss_mpi_hsize_mt;
#define MPI_HSIZE_T             (ss_mpi_init(), ss_mpi_hsize_mt)
extern MPI_Datatype             ss_mpi_size_mt;
#define MPI_SIZE_T              (ss_mpi_init(), ss_mpi_size_mt)
extern MPI_Datatype             ss_mpi_htri_mt;
#define MPI_HTRI_T              (ss_mpi_init(), ss_mpi_htri_mt)

/* Define a couple of communicators for use in the serial version of SSlib */
#ifdef HAVE_PARALLEL
#  define SS_COMM_WORLD         MPI_COMM_WORLD
#  define SS_COMM_SELF          MPI_COMM_SELF
#  define SS_COMM_NULL          MPI_COMM_NULL
#else
#  define SS_COMM_WORLD         2
#  define SS_COMM_SELF          1
#  define SS_COMM_NULL          0
#endif

/* Some of these are only available if we have MPI support */
#ifdef HAVE_PARALLEL
herr_t ss_mpi_type_create_stride(int nstrides, const ss_blob_stride_t *s, MPI_Datatype type, MPI_Datatype *result);
#endif

#ifdef __cplusplus
extern "C" {
#endif

herr_t ss_mpi_init(void);
htri_t ss_mpi_subcomm(MPI_Comm subcomm, MPI_Comm comm);
int ss_mpi_maptask(int task, MPI_Comm subcomm, MPI_Comm comm);
ss_scope_t *ss_mpi_extras(ss_pers_t **persp, ss_scope_t *buffer);
int ss_mpi_comm_rank(MPI_Comm comm);
int ss_mpi_comm_size(MPI_Comm comm);
herr_t ss_mpi_bcast(void *buffer, size_t count, MPI_Datatype datatype, int root, MPI_Comm comm);
herr_t ss_mpi_barrier(MPI_Comm comm);
herr_t ss_mpi_comm_dup(MPI_Comm comm, MPI_Comm *duped);
herr_t ss_mpi_comm_free(MPI_Comm *comm);
size_t ss_mpi_serial(MPI_Comm comm);
herr_t ss_mpi_allgather(void *buffer, size_t count, MPI_Datatype type, MPI_Comm comm);
herr_t ss_mpi_allgatherv(void *buffer, const size_t *counts, const size_t *offsets, MPI_Datatype type, MPI_Comm comm);
int ss_mpi_elect(ss_pers_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*!SS_HAVE_SSMPI_H*/
