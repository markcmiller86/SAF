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
 * 
 *    Authors:
 *       William J. Arrighi	LLNL
 *       Peter K. Espen		SNL
 *       Ray T. Hitt 		SNL
 *       Robb P. Matzke 		LLNL
 *       Mark C. Miller 		LLNL
 *       James F. Reus 		LLNL
 *       Larry A. Schoof 		SNL
 * 
 *    Acknowledgements:
 *       Marty L. Barnaby		SNL - Red parallel perf. study/tuning
 *       David M. Butler		LPS - Data model design/implementation Spec.
 *       Albert K. Cheng		NCSA - Parallel HDF5 support
 *       Nancy Collins		IBM - Alpha/Beta user
 *       Linnea M. Cook		LLNL - Management advocate
 *       Michael J. Folk		NCSA - Management advocate 
 *       Richard M. Hedges		LLNL - Blue-Pacific parallel perf. study/tuning 
 *       Wilbur R. Johnson		SNL - Early developer
 *       Quincey Koziol		NCSA - Serial HDF5 Support 
 *       Celeste M. Matarazzo	LLNL - Management advocate
 *       Tom H. Robey 		SNL - Early developer
 *       Greg D. Sjaardema		SNL - Alpha/Beta user
 *       Reinhard W. Stotzer	SNL - Early developer
 *       Judy Sturtevant		SNL - Red parallel perf. study/tuning 
 *       Robert K. Yates		LLNL - Blue-Pacific parallel perf. study/tuning
 */

#include <safP.h>
#include <stdarg.h>

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Initialize kay and mask values for searching
 *
 * Description: This function is normally called only by the SAF_KEYMASK macro and is used to create and/or initialize a
 *              key/mask pair for searching.  The MASK is allocated by the caller and initialized to all zeros here. The KEY
 *              is created in the local scratch file if it doesn't exist or is reset to all zero if it does.
 *
 * Return:      Returns a pointer to the key on success; null on failure
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  1, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
_saf_keymask(unsigned magic, ss_persobj_t *mask)
{
    SAF_ENTER(_saf_keymask, NULL);

    unsigned sequence = SS_MAGIC_SEQUENCE(magic);
    ss_pers_class_t *pc = SS_PERS_CLASS(SS_MAGIC_SEQUENCE(magic));

    /* Reset the mask to all zero */
    memset(mask, 0, pc->t_size);

    /* Create the key if not already or reset to all zero */
    if (NULL==_SAF_GLOBALS.key[sequence]) {
        _SAF_GLOBALS.key[sequence] = ss_pers_new(_SAF_GLOBALS.local_scope, magic, NULL, SS_ALLSAME, NULL, NULL);
    } else {
        ss_pers_reset(_SAF_GLOBALS.key[sequence], SS_ALLSAME);
    }
    
    SAF_LEAVE(_SAF_GLOBALS.key[sequence]);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Copy a string
 *
 * Description: This is a more convenient version of strncpy() which treats DST as an empty string if it's the null pointer
 *              and which always null terminates the result when NCHARS is positive. This function is a no-op when DST is the
 *              null pointer or when NCHARS is zero.
 *
 * Return:      Returns DST
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 29, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
_saf_strncpy(char *dst, const char *src, size_t nchars)
{
    SAF_ENTER(_saf_strncpy,NULL);

    if (dst && nchars) {
        if (src) {
            strncpy(dst, src, nchars);
            dst[nchars-1] = '\0';
        } else {
            dst[0] = '\0';
        }
    }
    SAF_LEAVE(dst);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Copy a string
 *
 * Description: Same functionality as strdup() but returns an empty string when S is the null pointer.
 *
 * Return:      Returns an allocated, null terminated string on success; null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Wednesday, April  7, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
_saf_strdup(const char *s)
{
    SAF_ENTER(_saf_strdup, NULL);
    char *retval = malloc(s?strlen(s)+1:1);
    if (retval) strcpy(retval, s?s:"");
    SAF_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Free memory
 *
 * Description: A convenient version of free() which is a no-op when PTR is null.
 *
 * Return:      Always returns NULL
 *
 * Programmer:  Robb Matzke
 *              Wednesday, March 29, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
_saf_free(void *ptr)
{
    SAF_ENTER(_saf_free,NULL);

    if (ptr) free(ptr);
    SAF_LEAVE(NULL);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Return the MPI rank of the calling processor
 *
 * Description: If the library has been initialized, in serial, this function will always return 0
 *              and in parallel, it will return the MPI rank of calling processor in the communicator
 *              associated with the database, DB, or, if DB is NULL, the communicator associated with
 *              the library in the saf_init() call which further defaults to MPI_COMM_WORLD.
 *
 * Modifications: Mark Miller, LLNL, 2000-04-20
 *                Added documentation
 *
 *                Bill Arrighi, LLNL, 2000-08-29
 *                Replace if (_SAF_InitDone) protection with SAF_ENTER.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_rank(SAF_Db *db)
{
  SAF_ENTER(_saf_rank, SAF_PRECONDITION_ERROR);
  int self = _SAF_GLOBALS.Rank;

  if (db) {
      ss_scope_t topscope;
      ss_file_topscope(db, &topscope);
      ss_scope_comm(&topscope, NULL, &self, NULL);
  }
  
  SAF_LEAVE(self);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Return the size of the dabase communicator
 *
 * Description: If the library has been initialized, in serial, this function will always return 1 
 *              and in parallel, it will return the MPI size of the communictor
 *              associated with the database, DB, or, if DB is NULL, the communicator associated with
 *              the library in the saf_init() call which further defaults to MPI_COMM_WORLD.
 *
 * Modifications: Mark Miller, LLNL, 2000-04-20
 *                Added documentation
 *
 *                Bill Arrighi, LLNL, 2000-08-29
 *                Replace if (_SAF_InitDone) protection with SAF_ENTER.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_size(SAF_Db *db)
{
  SAF_ENTER(_saf_size, SAF_PRECONDITION_ERROR);
  int ntasks = _SAF_GLOBALS.Size;

  if (db) {
      ss_scope_t topscope;
      ss_file_topscope(db, &topscope);
      ss_scope_comm(&topscope, NULL, NULL, &ntasks);
  }

  SAF_LEAVE(ntasks);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Parallel Barrier 
 *
 * Description: In serial, this function has no effect.
 *              In parallel, if the library has not been initialized, this function also has no effect. 
 *              If the library has been initialized, in parallel it will execute a barrier synchronization
 *              on the communicator associated with the database, DB, or, if DB is NULL, the communicator
 *              associated with the library in the saf_init() call which further defaults to MPI_COMM_WORLD.
 *
 * Modifications: Mark Miller, LLNL, 2000-04-20
 *                Added documentation
 *
 *		Mark Miller, LLNL, 2001-06-28
 *		   added ARGSUSED to quite the compiler for serial compile
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void
_saf_barrier(SAF_Db *db)
{
  SAF_ENTER(_saf_barrier, /*void*/);
  MPI_Comm      comm = _SAF_GLOBALS.p.LibComm;

  if (db) {
      ss_scope_t topscope;
      ss_file_topscope(db, &topscope);
      ss_scope_comm(&topscope, &comm, NULL, NULL);
  }
  
  ss_mpi_barrier(comm);
  SAF_LEAVE(/*void*/);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Note:	Returned Strings
 * Description: SAF offers three ways for string valued arguments to be returned by the client; client allocates and frees,
 *              lib allocates and client frees, or strings are allocated from a string /pool/ and freed automatically
 *              by the library using a least recently used policy. See saf_setProps_StrMode() for more information.
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Allocate a string for return by caller
 *
 * Description: Based on an input string, this function will return a result string (copy) according to the current string
 *		mode (see Returned Strings) or do nothing if the client passes NULL for result_str.
 *
 * Parallel:    Parallel and serial behavior are identical 
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function returns 
 *              an error number.
 *
 * Programmer:  Mark Miller, LLNL, Nov. 1999
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_setupReturned_string(char **result_str,
                          const char *input_str
                          )
{
    SAF_ENTER(_saf_setupReturned_string,SAF_PRECONDITION_ERROR);
    size_t               i;

    SAF_REQUIRE(_SAF_GLOBALS.p.StrMode != SAF_STRMODE_CLIENT || result_str == NULL || *result_str != NULL,
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if the string mode is SAF_STRMODE_CLIENT  and the client wants the string returned "
                            "then the client must have allocated RESULT_STR"));
    SAF_REQUIRE(_SAF_GLOBALS.p.StrMode == SAF_STRMODE_CLIENT || result_str == NULL || *result_str == NULL,
                SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("if the string mode is not SAF_STRMODE_CLIENT and the client wants the string returned "
                            "then the client must not have allocated RESULT_STR"));

    /* note, it is not an error if the caller attempts to return a string to a NULL. It means the caller did NOT want the string
      returned */
    if (result_str == NULL)
        SAF_RETURN(SAF_SUCCESS);

    /* in all the cases in the switch statement, the caller has indicated that it *wants* the string returned in result_str */
    switch (_SAF_GLOBALS.p.StrMode) {
    case SAF_STRMODE_CLIENT:
        strcpy(*result_str, input_str);
        break;
    case SAF_STRMODE_POOL:
        i = _SAF_GLOBALS.str.next;
        _SAF_GLOBALS.str.next = (_SAF_GLOBALS.str.next+1) % _SAF_GLOBALS.p.StrPoolSize;
        _SAF_GLOBALS.str.pool[i] = SS_FREE(_SAF_GLOBALS.str.pool[i]);
        if (NULL==(*result_str=_SAF_GLOBALS.str.pool[i]=malloc(strlen(input_str?input_str:"<nil>")+1)))
            SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("malloc failed"));
        strcpy(*result_str, input_str?input_str:"<nil>");
        break;
    case SAF_STRMODE_LIB:
        *result_str = _saf_strdup(input_str);
        if (!*result_str && input_str)
            SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate string"));
        break;
    default:
        SAF_ERROR(SAF_GENERIC_ERROR, _saf_errmsg("unknown string mode"));
    }
    SAF_LEAVE(SAF_SUCCESS);
}

/*
 * The following function is to address component ordering
 */
/*------------------------------------------------------------------------------
|  Audience:    Private
|  Chapter:     Miscellaneous Utilities
|  Purpose:     Check if vector is a permutation vector
|
|  Description: This function is used to check if the given vector of a given
|               number of integers is a permutation vector.  A permutation
|               is a vector of N integers whose elements are in 0..N-1 and
|               have no duplications.  Such a vector may be transformed to
|               the identity permutation vector {0,1, ... ,N-1} by performing
|               zero or more swaps between any two elements.
|
|  Return:      A value of true is returned if the given vector is indeed a
|               permutation vector, otherwise a value of false is returned.
|
|  Programmer:  Jim Reus 14Jul2000
|               Initial implementation.
|
|  Modifications:
|
+-----------------------------------------------------------------------------*/
hbool_t
_saf_is_permutation ( int N      /* The size of the given vector. */
                    , int vec[]  /* A given vector of integers which is to be
                                  * examined.
                                  */
                    )
{
   SAF_ENTER(_saf_is_permutation,false);

   hbool_t result;

   result = false;
   if (vec != NULL)
   {  if (1 <= N)
      {  if (N <= 16)
         {  hbool_t have[16];
            int           i;

           /*
            * For the typical small vector
            * we'll just keep a list of
            * which integers are in the
            * given vector and which aren't
            * since the list and the given
            * vector are the same size, any
            * duplications will result in
            * a missing integer.  This is
            * a O(N) test.
            */
            for (i=0; i<N; ++i)
               have[i] = false;
            for (i=0; i<N; ++i)
            {  int Vi;

               Vi = vec[i];
               if (0 <= Vi && Vi < N)
                  have[Vi] = true;
            }
            result = true;
            for (i=0; i<N; ++i)
               if (!have[i])
               {  result = false;
                  break;
               }
         }
         else
         {  int i;

           /*
            * For the rare LARGE vector we'll
            * have to do it the hard way.  We
            * check the N element vector for
            * each of 0..N-1 if one isn't found
            * then the given vector can't be a
            * permutation vector.  This is a
            * O(N^2) solution but works for
            * any N without costly allocation.
            */
            result = true;
            for (i=0; i<N; ++i)
            {  int j;

               for (j=0; j<N; ++j)
                  if (vec[j] == i)
                     break;
               if (j == N)
               {  result = false;
                  break;
               }
            }
         }
      }
   }
   SAF_LEAVE(result);
}

#if 0
/*DOCUMENTED*/
int *
_saf_identity_permutation(int n)
{
   SAF_ENTER(_saf_identity_permutation,NULL);

   int *first_ten[] = { {0},
                        {0,1},
                        {0,1,2},
                        {0,1,2,3},
                        {0,1,2,3,4},
                        {0,1,2,3,4,5},
                        {0,1,2,3,4,5,6},
                        {0,1,2,3,4,5,6,7},
                        {0,1,2,3,4,5,6,7,8},
                        {0,1,2,3,4,5,6,7,8,9}
                      };
   static int *result = NULL;
   static DSL_Boolean_t last_returned_first_ten = false;

   if ((result != NULL) and (not last_returned_first_ten))
      _saf_free(result);

   if (n < 10)
   {
      result = first_ten[n];
      last_returned_first_ten = true;
   }
   else
   {
      result = (int *) calloc(n,sizeof(int));
      for (i = 0; i < n; i++)
         result[i] = i;
      last_returned_first_ten = false;
   }

   SAF_LEAVE(result);
}

/*DOCUMENTED*/
DSL_Boolean_t
_saf_is_identity_permutation(int n, int *perm)
{
   SAF_ENTER(_saf_is_identity_permutation,false);

   int i;

   /* a null perumtation vector is treated as identity */
   if (perm == NULL)
      SAF_RETURN(true);

   for (i = 0; i < n; i++)
      if (perm[i] != i)
         SAF_RETURN(false);
   SAF_LEAVE(true);
}

/*
 * pre-defined indexing orders for up to 10 dimensions these are defined relative to the left-to-right notational expression
 * "array[K][J][I]". Thus, the right-most digit in the returned results from these functions corresponds to index 'I' in the
 * notation. The next digit in the results corresponds to index "J", etc,. Each digit, itself represents the relative speed of
 * variation of that index as we march through linearized storage from buf[0] to the end of the buffer
 *
 * These ARE NOT EXPORTED to client but a re called by client via the macros SAF_FORDER(N), SAF_CORDER(N)
 *
 * the "BAD"s are necessary to detect clients mistakenly using a 'N' in these macros that is not consistent with other
 * information about the array such as the array dimension.
 *
 */

/*DOCUMENTED*/
#define BAD     -999

/*DOCUMENTED*/
int *
_saf_c_order(int n)
{
   SAF_ENTER(_saf_c_order,NULL);

   static int results[10][10] =
        /* array[I9][I8][I7][I6][I5][I4][I3][I2][I1][I0] */
           {    BAD,BAD,BAD,BAD,BAD,BAD,BAD,BAD,BAD,  0,
                BAD,BAD,BAD,BAD,BAD,BAD,BAD,BAD,  1,  0,
                BAD,BAD,BAD,BAD,BAD,BAD,BAD,  2,  1,  0,
                BAD,BAD,BAD,BAD,BAD,BAD,  3,  2,  1,  0,
                BAD,BAD,BAD,BAD,BAD,  4,  3,  2,  1,  0,
                BAD,BAD,BAD,BAD,  5,  4,  3,  2,  1,  0,
                BAD,BAD,BAD,  6,  5,  4,  3,  2,  1,  0,
                BAD,BAD,  7,  6,  5,  4,  3,  2,  1,  0,
                BAD,  8,  7,  6,  5,  4,  3,  2,  1,  0,
                  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
            };

   if (n < 10)
      SAF_RETURN(results[n]);
   SAF_LEAVE(NULL);
}

/*DOCUMENTED*/
int *
_saf_fortran_order(int n)
{
   SAF_ENTER(_saf_fortran_order,NULL);

   static int results[10][10] =
        /* array[I9][I8][I7][I6][I5][I4][I3][I2][I1][I0] */
           {    BAD,BAD,BAD,BAD,BAD,BAD,BAD,BAD,BAD,  0,
                BAD,BAD,BAD,BAD,BAD,BAD,BAD,BAD,  0,  1,
                BAD,BAD,BAD,BAD,BAD,BAD,BAD,  0,  1,  2,
                BAD,BAD,BAD,BAD,BAD,BAD,  0,  1,  2,  3,
                BAD,BAD,BAD,BAD,BAD,  0,  1,  2,  3,  4,
                BAD,BAD,BAD,BAD,  0,  1,  2,  3,  4,  4,
                BAD,BAD,BAD,  0,  1,  2,  3,  4,  4,  6,
                BAD,BAD,  0,  1,  2,  3,  4,  4,  6,  7,
                BAD,  0,  1,  2,  3,  4,  4,  6,  7,  8,
                  0,  1,  2,  3,  4,  4,  6,  7,  8,  9
            };

   if (n < 10)
       SAF_RETURN(results[n]);
   SAF_LEAVE(NULL);
}

/*DOCUMENTED*/
DSL_Boolean_t
_saf_is_fortran_order(int n, int *dim_order)
{
   SAF_ENTER(_saf_is_fortran_order,false);

   int *fortran_order = _saf_fortran_order(n);
   int i;

   if (n > 10)
       SAF_RETURN(false);

   for (i = 0; i < n; i++)
      if (dim_order[i] != fortran_order[i])
         SAF_RETURN(false);

   SAF_LEAVE(true);
}

/*DOCUMENTED*/
DSL_Boolean_t
_saf_is_c_order(int n, int *dim_order)
{
   SAF_ENTER(_saf_is_c_order,false);

   int *c_order = _saf_c_order(n);
   int i;

   if (n > 10)
       SAF_RETURN(false);

   for (i = 0; i < n; i++)
      if (dim_order[i] != c_order[i])
          SAF_RETURN(false);

   SAF_LEAVE(true);
}

#endif

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private 
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Build an indexing specification struct 
 *
 * Description: An index specification struct indicates the number of dimensions, the origin in each dimension and the
 *              relative ordering of dimensions when layed out into a linear address space. The first argument to this
 *              function is the number of dimensions. The remaining arguments are...
 *
 *              a) a comma separated list of N sizes representing the size in each of the N dimensins
 *
 *              b) a comma separated list of N origins indicating the origin in each dimension
 *
 *              c) a negative number indicating one of the standard orders (e.g. SAF_F_ORDER or SAF_C_ORDER) or 
 *                 a comma separated list of N relative orders indicating the relative orders of the dimensions
 *                 when layed out in linear storage.
 *
 * Parallel:    Parallel and serial behavior are identical 
 *
 * Return:      an indexing specification struct.       
 *
 * Programmer:  Mark Miller, LLNL, Nov. 1999
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
SAF_IndexSpec
_saf_indexspec(int n, ...)
{
   SAF_IndexSpec error_ispec;
   memset(&error_ispec, 0, sizeof error_ispec);
   SAF_ENTER(_saf_indexspec,error_ispec);

   SAF_IndexSpec ispec;
   int i,order_flag;
   va_list ap;

   /* initalize the struct's values */
   ispec.ndims = 0;
   for (i = 0; i < SAF_MAX_NDIMS; i++)
   {
      ispec.sizes[i] = 0;
      ispec.origins[i] = 0;
      ispec.order[i] = 0;
   }

   /* fill in the struct */
   ispec.ndims = n;
   va_start(ap, n);
      for (i = 0; i < n; i++)
         ispec.sizes[i] = va_arg(ap, int);
      for (i = 0; i < n; i++)
         ispec.origins[i] = va_arg(ap, int);
      order_flag = va_arg(ap, int);
      if (order_flag == SAF_F_ORDER)
      {
         for (i = 0; i < n; i++)
            ispec.order[i] = n-i-1;
      }
      else if (order_flag == SAF_C_ORDER)
      {
         for (i = 0; i < n; i++)
            ispec.order[i] = i; 
      }
      else
      {
         ispec.order[0] = order_flag;
         for (i = 1; i < n; i++)
            ispec.order[i] = va_arg(ap, int);
      }
   va_end(ap);

   /* this return copies the struct contents */
   SAF_LEAVE(ispec);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public  
 * Chapter:     Attributes
 *
 * Description: As mentioned in the object handles chapter (see Object Handles) there currently (saf-1.2.0) exist two styles
 *              of handles: "old" handles and "new" handles. For each "old" object class there are functions to put
 *              (saf_put_XXX_att()) and get (saf_get_XXX_att()) attributes, as well as generic forms of these functions
 *              (saf_put_attribute() and saf_get_attribute()) which operate on any object type but do not provide rigorous
 *              compile-time type checking.  The "new" object classes use only saf_putAttribute() and saf_getAttribute(),
 *              which employ compile-time and run-time type checking.
 *
 *              There is an important limitation to the attributes interface in SAF. First and foremost, it should be
 *              clearly understood that there is *no*expectation* that any data stored in attributes be shareable. If there is
 *              any expectation that *any* software other than the writer of the attributes should be sensitive to and/or aware
 *              of the data stored in them, the data should *not* be stored in attributes. If for some reason, your client is
 *              unable to model important features of the data without encoding something into attributes, then the current
 *              implementation of this data model is failing.
 *
 *              By convention, attributes whose names begin with a dot (".") are read-only. Thus, a client may create and
 *              initialize a new attribute whose name begins with a dot, but thereafter any client operating on the database
 *              can only read the value of that attribute.
 *
 * Issue:       Each attribute has its own HDF5 dataset in the SAF file. For a SAF_EACH mode call, we need to loop
 *              creating /num_procs/ datasets.
 *
 *		Also, performance of attribute access is likely to be poor, particularly in parallel.
 *
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Attributes
 * Purpose:     Create or update a non-sharable attribute
 *
 * Description: This function provides a method by which generic, non-sharable attributes may be added to an object. Attributes
 *              are pieces of meta data which fall outside the scope of the sharable data model (i.e., things which are not
 *              fields) but which are often useful for conveying additional information. The meaning of a particular attribute is
 *              determined by convention, requiring additional, apriori agreement between the writer and the reader (often in the
 *		form of documentation or word of mouth) as to the meaning and intent of a given attribute/value pair.
 *
 *              If TYPE is H5T_C_S1 (which isn't very useful by itself since it's just a one-byte string that's always the NUL
 *              character) then a temporary datatype is created which is exactly as long as the VALUE string including its NUL
 *              terminator.  VALUE in this case should be a pointer to !char. Be aware that querying the attribute for its
 *              datatype will not return H5T_C_S1 unless the string was empty.
 *
 * Parallel:    Depends on PMODE
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_put_attribute(SAF_ParMode pmode,    /* One of the parallel modes. */
                  ss_pers_t *obj,       /* The handle to the object the attribute is to be associated with. */
                  const char *name,     /* The name of the attribute. */
                  hid_t type,           /* The datatype of the attribute. */
                  int count,            /* The number of items of type TYPE pointed to by *VALUE. */
                  const void *value     /* The attribute value(s) (an array of COUNT value(s) of type TYPE). */
                  )
{
   SAF_ENTER(saf_put_attribute, SAF_PRECONDITION_ERROR);
   ss_attr_t    attr;
   hid_t        type_here=-1;

   SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("PMODE must be valid"));
   if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);
   
   SAF_REQUIRE(obj, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("OBJ must not be null"));
   SAF_REQUIRE(name != NULL, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("NAME must not be null"));
   SAF_REQUIRE(count >= 0, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("COUNT must be non-negative"));
   SAF_REQUIRE(count==0 || value!=NULL, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("if count is non-zero, VALUE must not be null"));
   SAF_REQUIRE(true, SAF_NO_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("database in which object exists must not be open for read-only access"));

   /* As a special case, if TYPE is H5T_C_S1 then create a temporary datatype which is exactly the same length as the
    * NUL-terminated string passed in as the VALUE argument. */
   if (H5Tequal(type, H5T_C_S1)) {
       if (count>1)
           SAF_ERROR(SAF_PARAMETER_ERROR, _saf_errmsg("H5T_C_S1 type can only be used if COUNT is zero or one"));
       if ((type = type_here = H5Tcopy(H5T_C_S1))<0)
           SAF_ERROR(SAF_MISC_ERROR, _saf_errmsg("H5Tcopy() failed"));
       if (H5Tset_size(type, strlen(value)+1)<0)
           SAF_ERROR(SAF_MISC_ERROR, _saf_errmsg("H5Tset_size() failed"));
   }

   /* Create a new attribute object */
   if (NULL==ss_attr_new(obj, name, type, (size_t)count, value, SAF_ALL==pmode?SS_ALLSAME:0U, &attr, NULL))
       SAF_ERROR(SAF_SSLIB_ERROR, _saf_errmsg("unable to create attribute \"%s\"", name));

   /* Cleanup */
   if (type_here>0) H5Tclose(type_here);

   SAF_LEAVE(SAF_SUCCESS);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Attributes      
 * Purpose:     Read a non-sharable attribute
 *
 * Description: This function provides a method by which existing, generic, non-sharable attributes may be read from an
 *              object. Attributes are pieces of meta data which fall outside the scope of the sharable data model (i.e.,
 *              things which are not fields) but which are often useful for conveying additional information. The meaning of a
 *              particular attribute is determined by convention, requiring additional communication between the writer and
 *              the reader (often in the form of documentation or word of mouth).
 *
 * Parallel:    Depends on PMODE
 *
 * Return:      The constant SAF_SUCCESS is returned when this function is successful. Otherwise this function either returns
 *              an error number or throws an exception, depending on the value of the library's error handling property.
 *
 * Issues:      On error, the argument return values are undefined.
 *
 *              If the attribute NAME is SAF_ATT_NAMES then the client must not preallocate space for the VALUE return value,
 *              but must allow the library to handle the allocation. That is, if the arg passed for NAME is SAF_ATT_NAMES, the
 *		client must not pass VALUE such that it points to a non-null pointer.
 *
 *              The reserved attribute name queries, SAF_ATT_FIRST and SAF_ATT_NEXT, are not yet implemented.
 *
 *              This is a weird interface. There should be a separate function to obtain the datatype and count of an
 *              attribute so that this function doesn't need to return those values. The TYPE and COUNT arguments should
 *              instead specify what value is returned by this function. And the VALUE should be just an optional `void*'
 *              buffer which if not supplied is allocated and which is the successful return value of this function.
 *              [rpm 2004-08-25]
 *---------------------------------------------------------------------------------------------------------------------------------
 */
int
saf_get_attribute(SAF_ParMode pmode,    /* One of the parallel modes. */
                  ss_pers_t *obj,       /* The handle to the object from which the attribute will be read. */
                  const char *name,     /* The name of the attribute. See SAF_ATT_NAMES and other reserved attribute names
					 * for special kinds of attribute queries. */
                  hid_t *type,          /* INOUT: If TYPE is NULL, this argument will be ignored. If TYPE points
                                         * to a valid datatype, then the attribute will be converted to the specified type as
                                         * it is read. If it does not, there will be *no*data*conversion* and the output value
                                         * will be the datatype of the data returned (the caller should invoke H5Tclose()). */
                  int *count,           /* OUT: The number of items in the attribute. If COUNT is NULL, then the value of
                                         * COUNT will not be returned. */            
                  void **value          /* INOUT: Points to an array of COUNT values each having datatype
                                         * TYPE. If VALUE is NULL, then no attribute values will be returned. If
                                         * VALUE points to NULL, then the library will allocate the array of values which is
					 * returned. Otherwise the library assumes that VALUE
                                         * points to an array whose size is sufficient for storing COUNT values of datatype
                                         * TYPE. That is, if VALUE is pointing to non-NULL, then so must COUNT point to non-NULL
					 * *and* the value pointed to by COUNT will be used by SAF as the size, in items of type
                                         * TYPE, of the block of memory pointed to by VALUE. For a SAF_ATT_NAMES query if the
                                         * caller supplies a buffer for this argument then it should be a buffer of !char
                                         * pointers, the values of which will be allocated by this function. */
                  )
{
   SAF_ENTER(saf_get_attribute, SAF_PRECONDITION_ERROR);
   ss_attr_t            attr, *attrs=NULL;
   size_t               nfound, my_count=SS_NOSIZE, i, maxfind;
   hid_t                stored_type=-1;

   SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("PMODE must be valid"));
   if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

   SAF_REQUIRE(name != NULL, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR, _saf_errmsg("NAME must not be null"));
   SAF_REQUIRE(_saf_valid_memhints(count, (void**)value), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
               _saf_errmsg("COUNT and VALUE must be compatible for return value allocation"));
   SAF_REQUIRE(obj != NULL, SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR, _saf_errmsg("OBJ must not be null"));

   if (!strcmp(name, SAF_ATT_COUNT)) {
       /* Only count the matching attributes */
       ss_attr_find(obj, NULL, 0, SS_NOSIZE, &nfound, SS_PERS_TEST);
       if (count) *count = nfound;
   } else if (!strcmp(name, SAF_ATT_NAMES)) {
       /* Return the matching attribute names */
       maxfind = value && *value && count ? (size_t)*count : SS_NOSIZE;
       if (NULL==(attrs=ss_attr_find(obj, NULL, 0, maxfind, &nfound, NULL)))
           SAF_ERROR(SAF_SSLIB_ERROR, _saf_errmsg("ss_attr_find failed"));

       /* allocate string pointers */
       if (!*value) {
           if (NULL==(*value=calloc(nfound, sizeof(char *))))
               SAF_ERROR(SAF_MEMORY_ERROR, _saf_errmsg("unable to allocate string pointers for names"));
       } else {
           memset(*value, 0, nfound*sizeof(char*));
       }

       /* Copy attribute names into string pointers */
       /* ISSUE: If the pool allocation is being used then we'll have a problem if there are more attributes than what the
        *        string pool can store. */
       for (i=0; i<nfound; i++)
           _saf_setupReturned_string((char**)*value+i, ss_string_ptr(SS_ATTR_P(attrs+i,name)));

       /* Output arguments */
       if (count) *count = nfound;
       if (type) *type = -1;

   } else {
       /* Find the attribute with the specified name */
       if (NULL==ss_attr_find(obj, name, 0, 1, &nfound, &attr))
           SAF_ERROR(SAF_SSLIB_ERROR, _saf_errmsg("ss_attr_find failed for \"%s\"", name));

       /* As per defect HYPer03337, it is not an error to request an attribute that is not defined. */
       if (0==nfound) {
           if (count) *count = 0;
           goto done;
       }
       
       /* Obtain the datatype and count. */
       ss_attr_describe(&attr, NULL, &stored_type, &my_count);
       if (!type) {
           type = &stored_type;
       } else if (*type<=0) {
           *type = stored_type;
       }
       if (count) *count = my_count;

       /* Read the attribute data */
       *value = ss_attr_get(&attr, *type, 0, my_count, *value);
       if (NULL==*value) SAF_ERROR(SAF_SSLIB_ERROR, _saf_errmsg("ss_attr_read failed for \"%s\"", name));
   }

done:
   SS_FREE(attrs);
   SAF_LEAVE(SAF_SUCCESS);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Check if given set is the Universe Set
 *
 * Description: This function checks to see if the given set is the Universe set of the database. The only thing that makes a
 *              set a Universe set is it's name is "universe".
 *
 * Return:      True if S is found to be the Universe set. False for any other reason.
 *
 * Modifications:
 *              Mark Miller, LLNL, 2000-04-20
 *              Added documentation
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_is_universe(SAF_Set *s /* the set handle */)
{
   SAF_ENTER(_saf_is_universe, false);
   htri_t retval=FALSE;
   if (s) retval = SS_PERS_EQUAL(s, &SAF_UNIVERSE_SET_g);
   SAF_LEAVE(retval==TRUE);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Check if given set is the Null Set      
 *
 * Description: This function checks to see if the given set is the Null set of the database. 
 *
 * Return:      Returns true if S is found to be the Null set. False for any other reason.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_is_null(SAF_Set *s /* the set handle */)
{
   SAF_ENTER(_saf_is_null, false);
   SAF_LEAVE(s==NULL);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Check if given category is the self decomposition category
 *
 * Description: This function checks to see if the given category is the self decomposition category of the database. 
 *
 * Return:      Returns true if cat is found to be the self decomposition category. False for any other reason.
 *
 * Programmer:  Bill Arrighi, LLNL, 2000-08-30
 *              Created
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_is_self_decomp(SAF_Cat *cat)
{
   SAF_ENTER(_saf_is_self_decomp, false);
   htri_t retval=FALSE;
   if (cat) retval = SS_PERS_EQUAL(cat, &SAF_SELF_CAT_g);
   SAF_LEAVE(retval);
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Validate the parallel mode      
 *
 * Description: All three parallel modes, SAF_ALL, SAF_EACH and SAF_ONE(RANK) are allowed in both serial and parallel
 *              However, in serial, SAF_ALL = SAF_EACH = SAF_ONE(0). In parallel, the RANK arg in SAF_ONE(RANK), must
 *              be within the size of the communicator associated with the database. 
 *
 * Return:      Returns true if the parallel mode is valid, false otherwise.
 *
 * Programmer:  Mark Miller, LLNL, Feb. 2000
 *---------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_valid_pmode(SAF_ParMode pmode)
{
    SAF_ENTER(_saf_valid_pmode, false);
    SAF_LEAVE(pmode==SAF_ALL || pmode==SAF_EACH || (pmode>=0 && pmode<_SAF_GLOBALS.Size));
}

/*---------------------------------------------------------------------------------------------------------------------------------
 * Note:	Returned Handles
 * Description: Many API functions take a pointer to an integer indicating the size of a returned list of object handles. 
 *              For example, saf_find_files() has int *num_files and SAF_File **files arguments. Many of SAF's other find
 *		functions have an analogous pair of args; one for the /number/ of returned items and the other for the /list/ of
 *		returned items. In the text below, we refer to these, generically, as the /number/ and /list/ arguments.
 *
 *		The memory for the returned handles can be allocated by the client or by the lib. Furthermore, the client can
 *		determine the size of the returned list by first calling the function with a non-NULL number argument and
 *		the list argument set to NULL. For example, to determine the number of supplemental files whose names begin
 *		with "gorfo"...
 *
 *                 int count;
 *                 saf_find_files(db, "gorfo*", &count, NULL);
 *
 *		will return in COUNT, the number of matching supplemental files.
 * 
 *		If the client allocates the memory for the returned handles, the number argument is used both as an input and
 *		as an output argument. On input, the number argument is expected to point to a value indicating the number
 *		of items that can be stored in the list argument *and* the list argument is expected to point to the memory
 *		for the returned handles allocated by the client. For example...
 *
 *                 int count = 5;
 *                 SAF_Files files[5]; *pfiles = &files[0];
 *                 saf_find_files(db, "gorfo*", &count, &pfiles);
 *
 *		Furthermore, if SAF attempts to return more items into this memory than the
 *		size indicated by the client via the number argument, the function will return an error.
 * 
 *		If the client wants the library to allocate the memory for the returned list of handles, the client is expected
 *		to pass a value for the list argument that points to NULL. For example...
 *
 *                 int count = 0;
 *                 SAF_Files *files = NULL;
 *                 saf_find_files(db, "gorfo*", &count, &pfiles);
 *---------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Checks memory hints
 *
 * Description: (See Returned Handles)
 *
 * Return:      Returns true if N and PTR are appropriate, false otherwise.
 *
 * Programmer:  Robb Matzke
 *              Thursday, February 17, 2000
 *
 * Modifications:
 *              Jim Reus, 08Dec2000
 *              - Rewritten.
 *
 *---------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_valid_memhints(int *Pcount, void **Pptr)
{
    SAF_ENTER(_saf_valid_memhints, FALSE);

    /* There are three combinations supported:
     * 
     *   1) Client wants count only.
     *   2) Client provides a buffer of known size, library returns amount used.
     *   3) Library allocates a buffer and returns size. */

    /* If the client only wants the count then a reference to the variable to receive the count must be provided and NO
     * refernce to a buffer pointer may be provided. */
    if (Pcount && !Pptr)
        SAF_RETURN(TRUE);

    /* If the client is supplying the buffer then a reference to the variable containing the buffer size and to receive the
     * count must be provided and a refernce to a buffer pointer must be provided. */
    if (Pptr && *Pptr && Pcount && *Pcount>0)
        SAF_RETURN(TRUE);

    /* If the library is allocating the buffer then a reference to the variable to receive the count must be provided and a
     * refernce to a NULL buffer pointer must be provided. */
    if (Pcount && Pptr && !*Pptr)
        SAF_RETURN(TRUE);

    /* Otherwise the client has screwed-up. */
    SAF_LEAVE(false);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Version Numbers
 * Purpose:     Returns string representation of version number
 *
 * Description: Provides a function that should be used so version numbers all have a common format. If VERBOSE is set then
 *              the returned string will be of the form `version 1.2 release 3 (comment)', otherwise the returned string will
 *              be of the form `1.2.3-comment'. The ` (comment)' or `-comment' part of the string is omitted if there is no
 *              version annotation.
 *
 * Return:      BUFFER
 *
 * Programmer:  Robb Matzke
 *              Wednesday, August  9, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
char *
saf_version_string(int verbose, char *buffer, size_t bufsize)
{
    SAF_ENTER(saf_version_string,NULL);

    char        tmp[64];
    const char  *fmt, *pre, *suf, *annot = SAF_VERSION_ANNOT;

    /* setup */
    if (verbose) {
        fmt = "version %d.%d release %d";
        pre = " (";
        suf = ")";
    } else {

        fmt = "%d.%d.%d";
        pre = "-";
        suf = "";
    }

    /* version and release numbers */
    sprintf(tmp, fmt, SAF_VERSION_MAJOR, SAF_VERSION_MINOR, SAF_VERSION_RELEASE);
    strncpy(buffer, tmp, bufsize);

    /* version annotation */
    if (annot) {
        size_t len = strlen(tmp);
        strncpy(buffer+len, pre, len<bufsize?bufsize-len:0);
        len += strlen(pre);
        strncpy(buffer+len, annot, len<bufsize?bufsize-len:0);
        len += strlen(annot);
        strncpy(buffer+len, suf, len<bufsize?bufsize-len:0);
    }
    SAF_LEAVE(buffer);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Checks that an index spec struct is valid
 *
 * Description: Check that the dimension of an index spec is non-negative, less than SAF_MAX_NDIMS and that
 *              all sizes of the index spec are non-negative and that their product equals COUNT.
 *
 * Return:      True if index spec is valid, false otherwise
 *
 * Programmer:  Bill Arrighi
 *              Friday, August 25, 2000
 *
 * Modifications:
 *	Jake Jones, 28jun2002, added check that .order is correct
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_valid_indexspec(SAF_IndexSpec ispec, int count)
{
  SAF_ENTER(_saf_valid_indexspec,false);

  hbool_t result;
  int i, n = 1, num_ispec_dims;

  num_ispec_dims = ispec.ndims;
  result = num_ispec_dims >= 0 && num_ispec_dims < SAF_MAX_NDIMS;
  for (i = 0; i < num_ispec_dims && result; i++)
    {
      n *= ispec.sizes[i];
      result = ispec.sizes[i] >= 0;
    }
  result &= count == n;

  /*check that .order is correct*/
  {
    int l_used[SAF_MAX_NDIMS];
    for(i=0;i<num_ispec_dims;i++) l_used[i]=0;
    for(i=0;i<num_ispec_dims;i++) 
    {
      if( ispec.order[i]>=0 && ispec.order[i]<num_ispec_dims && l_used[ispec.order[i]]==0 )
      {
        l_used[ispec.order[i]]=1;
      }
      else
      {
        result=false;
        break;
      }
    }
  }

  SAF_LEAVE(result);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Checks that an added index spec is valid
 *
 * Description: Check that an index spec. added in saf_extend_collection() is valid. It must be _saf_valid_indexspec AND
 *		it must be same dimensions as existing index specification.
 *
 * Return:      True if added index spec is valid and consistent with existing index spec. False otherwise
 *
 * Programmer:  Mark Miller, LLNL, 2000-09-28
 *
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_valid_add_indexspec(SAF_ParMode pmode,             /* the parallel mode */
                         SAF_Set *set,		        /* the containing set of the collection */
                         SAF_Cat *cat,		        /* the collection category */
                         SAF_IndexSpec add_ispec,	/* the indexing of the members to be added */
                         int add_count		        /* a count of the members to be added */
                         )
{
    SAF_ENTER(_saf_valid_add_indexspec, false);
    int                 i, j;
    ss_collection_t     coll = SS_COLLECTION_NULL;
    ss_indexspec_t      idx = SS_INDEXSPEC_NULL;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_MED_CHK_COST, FALSE, _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(TRUE);
    SAF_REQUIRE(SS_SET(set), SAF_MED_CHK_COST, FALSE, _saf_errmsg("SET must be a valid set for all participating processes"));
    SAF_REQUIRE(SS_CAT(cat), SAF_MED_CHK_COST, FALSE, _saf_errmsg("CAT must be a valid category for all participating processes"));

    /* start by confirming the index spec in and of itself is valid */
    if (!_saf_valid_indexspec(add_ispec, add_count)) SAF_RETURN(FALSE);

    /* Get the collection and its index spec. */
    _saf_getCollection_set(set, cat, &coll);
    ss_array_get(SS_COLLECTION_P(&coll,indexing), ss_pers_tm, (size_t)0, (size_t)1, &idx);

    /* ok, now verify the added indexing is consistent with current indexing */
    if (SS_INDEXSPEC(&idx)->ndims != add_ispec.ndims) SAF_RETURN(FALSE);

    /* find which dimension is being extended and confirm there is only one */
    for (i = 0; i < add_ispec.ndims; i++) {
        if (add_ispec.sizes[i] != 0) {
            for (j = i+1; j < add_ispec.ndims; j++) {
                if (add_ispec.sizes[j] != 0)
                    SAF_RETURN(FALSE);
            }
            break;
        }
    }
    
    SAF_LEAVE(TRUE);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Confirm if caller is a participating processor
 *
 * Description: Determines if given the parallel mode, pmode, and the database, db, the processor participates in
 * 		the function call from which this function is called.
 *
 * Return:      True if the indicated processor participates in the call, false otherwise
 *
 * Issues:      At this time this function always returns true.
 *
 * Programmer:  Bill Arrighi
 *              Friday, August 25, 2000
 *
 * Modifications:
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_is_participating_proc(SAF_ParMode pmode)
{
  SAF_ENTER(_saf_is_participating_proc,FALSE);
  SAF_LEAVE(SAF_ALL==pmode || SAF_EACH==pmode || _SAF_GLOBALS.Rank==pmode);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Confirm units and field template quantity are compatible
 *
 * Description: Given handles to units and a field template, confirm that the units identified by the units handle
 *		are compatible with the quantity identified by the field template handle.
 *
 *              The SAF_NOT_APPLICABLE_UNIT is considered to be a valid unit.
 *
 * Return:      True if the units are compatible with the quantity, false otherwise
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_is_valid_units(SAF_Unit *unit, SAF_FieldTmpl *ftmpl)
{
   SAF_ENTER(_saf_is_valid_units, FALSE);
   hbool_t retval;
   SAF_Quantity quant;

   if (!SS_FIELDTMPL(ftmpl)) SAF_RETURN(FALSE);
   quant = SS_FIELDTMPL(ftmpl)->quantity;

   if (SS_PERS_ISNULL(&quant)) SAF_RETURN(SS_UNIT(unit)?FALSE:TRUE);
   if (!SS_UNIT(unit)) SAF_RETURN(TRUE);

   /* Not applicable situations */
   if (SS_PERS_EQUAL(&quant, SAF_NOT_APPLICABLE_QUANTITY)) {
       if (SS_PERS_EQUAL(unit, SAF_NOT_APPLICABLE_UNIT)) {
           SAF_RETURN(TRUE);
       } else {
           SAF_RETURN(FALSE);
       }
   }
   
   /* The unit's quantity must match the field template's quantity */
   retval = SS_PERS_EQUAL(SS_UNIT_P(unit,quant), &quant);
   SAF_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:    	Raw Data I/O 
 *
 * Description: SAF is designed to enable clients to read/write data that is /shareable/. In order for data to be shareable,
 *		it must be characterized in terms of the data model, that is, as a set, field or relation.
 *
 *		However, it is sometimes the case that some given data in a client may not be characterized in terms of
 *		the data model yet the client, nonetheless, wishes to store the data to the database. The lower
 *		layers of software upon with SAF is built fully support any arbitrary data to be read/written from the
 *		client. Those layers are DSL, SAF's data storage layer, HDF5 and MPIO, when operating in parallel or
 *		libc I/O when operating in serial.
 *
 *		Rather than re-wrapping the functions in these lower layers with some SAF equivalent functionality,
 *		we provide the client with the ability to access the file handle used at these lower layers and then
 *		refer the client to the programmer's reference manual for those layers.
 *
 *		Using SAF in this way comes at a price. First and foremost, none of the data written in this way is
 *		expressed in terms of the data model. So, there can be no expectation that the resultant data can be
 *		read and correctly interpreted by any other SAF client. More importantly, going underneath SAF's /hood/ 
 *		can potentially jeopardize the integrity of the entire database. For this reason, we provide a 
 *		/grab/and/ungrab/ methodology for accessing lower level software.
 *
 *		There is nothing to prevent a SAF client from using DSL's or HDF5's interfaces directly with /external/files/
 *		(that is files that are not part of a SAF database). However, if the client wishes the resultant data
 *		to, at least, reside /in/ the database, the grab and ungrab methodology described here will provide a reliable
 *		mechanism for doing this.
 *
 *		Presently, a lower level interface can be grabbed for only one file at a time over the entire library.
 *		That is, the scope of a grab is a single file out of all files of all databases the library is interacting
 *		with. Later, this will be relaxed to a database and then to specific supplemental files in a database.
 *		Furthermore only DSL *or* HDF5 can be grabbed. They cannot both be grabbed.
 *-------------------------------------------------------------------------------------------------------------------------------
 */

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:   	Public 
 * Chapter:     Raw Data I/O 
 * Purpose:    	Grab HDF5 I/O library	
 *
 * Description:	This function is used to grab the HDF5 lower level I/O library to then interact with
 *		a file in the database using that library.
 *
 *		Once HDF5 is grabbed for a given file in the database (supplemental or master), all
 *		SAF operations on the database are suspended until HDF5 is ungrabbed. Any SAF operation that
 *		is attempted involving a database whose lower levels have been grabbed will fail. The lower level interfaces
 *		to the database must be ungrabbed before normal SAF operations can resume.
 *
 * 		For documentation on the HDF5 API, please see http://hdf.ncsa.uiuc.edu/HDF5/doc/RM_H5Front.html
 *
 *		Presently, a lower level interface can be grabbed for only one file in the database at a time.
 *
 * Return:	(See Return Values)
 *
 * Parallel:	This call is collective in the communicator of the database containing the specified file.
 *
 * Modifications:
 *		Mark C. Miller, LLNL, 2000-12-22
 *		Initial implementation
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hid_t
saf_grab_hdf5(SAF_Db *file)
{
    SAF_ENTER(saf_grab_hdf5, SAF_FAILURE);
    SAF_LEAVE(ss_file_isopen(file, NULL));
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:   	Public 
 * Chapter:     Raw Data I/O 
 * Purpose:    	Ungrab HDF5 I/O library	
 *
 * Description:	This function is used to ungrab the HDF5 lower level I/O library.
 *
 * Return:	(See Return Values)
 *
 * Parallel:	This call is collective in the communicator of the database containing the file whose handle was grabbed
 *		by saf_grab_hdf5()
 *
 * Modifications:
 *		Mark C. Miller, LLNL, 2000-12-22
 *		Initial implementation
 *-------------------------------------------------------------------------------------------------------------------------------
 */
/*ARGSUSED*/
int
saf_ungrab_hdf5(hid_t __UNUSED__ h5f_id /*HDF5 file handle previously obtained from a call to saf_grab_hdf5()*/)
{
    SAF_ENTER_GRABBED(saf_ungrab_hdf5, SAF_FAILURE);
    SAF_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:    	Set default error checking threshold	
 *
 * Description: This function is used to set the default error checking threshold for assertions, pre-conditions and
 *		post-conditions. It first looks for the specified environment variable. If the environment variable is
 *		*not*present*, it sets the error checking threshold to a default value, depending on whether this is a
 *		production compile or not. For a production compile, the default checking threshold is set to minimal
 *		error checking. For a development compile, it is set to maximal error checking.
 *
 * Return:	None
 *
 * Modifications:
 *		Mark C. Miller, 2000-09-20
 *		Initial Implementation
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void
_saf_set_disable_threshold(
   const char *env_var_name,	/* environment variable name to look for */
   int *property_value		/* pointer to integer error check property to set */
)
{
   SAF_ENTER(_saf_set_disable_threshold, /*void*/);
   char *s=0;

   SAF_GETENV_WORLD(s,env_var_name);

   if (s == NULL)
#ifdef PRODUCTION_COMPILE    
      *property_value = SAF_NO_CHK_COST;	/* production default if no env. setting */
#else
      *property_value = SAF_HIGH_CHK_COST;	/* non-production default in no env. setting */
#endif
   else if (!strcmp(s,"none"))
      *property_value = SAF_HIGH_CHK_COST;	/* none means set disable threshold to high */ 
   else if (!strcmp(s,"high"))
      *property_value = SAF_MED_CHK_COST;	/* high means set disable threshold to medium */
   else if (!strcmp(s,"med"))
      *property_value = SAF_LOW_CHK_COST;	/* medium means set disable threshold to low */
   else if (!strcmp(s,"all"))
      *property_value = SAF_NO_CHK_COST;	/* all means set disable threshold to no */              
   else
#ifdef PRODUCTION_COMPILE    
      *property_value = SAF_NO_CHK_COST;	/* production default if no env. setting */
#else
      *property_value = SAF_HIGH_CHK_COST;	/* non-production default in no env. setting */
#endif

   if(s) free(s);/*cannot use _saf_free until lib is initialized*/
   SAF_LEAVE(/*void*/);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:    	Set/Get wall clock time
 *
 * Description: This function is used to set and get current wall clock time. It relies upon existence of gettimeofday.
 *		If gettimeofday is not available, it always returns -1, indicating it was unable to obtain wall clock time.
 *
 *		If the init argument is true, it resets the global wall clock timer to the current time and returns 0.
 *		Otherwise, it returns elapsed time (e.g. wall clock time) in seconds since the timer was last initialized.
 *
 * Return:	wall clock time since last init'd in seconds.
 *
 * Modifications:
 *		Mark C. Miller, 2000-10-16
 *		Initial Implementation
 *-------------------------------------------------------------------------------------------------------------------------------
 */
double
_saf_wall_clock(hbool_t init    /* pass true if you want to initialize the timer */
                )
{
   double t;

#ifdef HAVE_PARALLEL

   t = MPI_Wtime();

#else

#   ifdef HAVE_GETTIMEOFDAY
       struct timeval tv;
       struct timezone tz;

       gettimeofday(&tv, &tz);

       t = (double) tv.tv_sec + (double) tv.tv_usec / (1.0e6);

#   else

       t = -1.0;

#   endif

#endif

   if (init)
      _SAF_GLOBALS.WallClockTime = t; 

   return (t - _SAF_GLOBALS.WallClockTime);

}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:    	Convert buffers of field handles to a 1d array
 *
 * Description: This function is used to convert several buffers of field types to a single buffer. We return
 *		only a single buffer because often the purpose is to write the fields to a VL-array and that
 *		interface only supports a single buffer of data.
 *
 *		The caller is expected to free the buffer that is allocated for the returned fields.
 *
 * Return:	A non-null pointer to a one-dimensional array of field links; null on failure.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_Field *
_saf_field_handles_1d(int nbufs,		/* number of input buffers */
                      SAF_Field **theBufs,      /* pointer to the buffers */
                      int size		        /* size of each buffer */
                      )
{
    SAF_ENTER(_saf_field_handles_1d, NULL);
    int         i;
    SAF_Field   *retval;

    if (NULL==(retval=malloc(nbufs*size*sizeof(*retval))))
        SAF_RETURN(NULL);

    for (i=0; i<nbufs; i++)
        memcpy(retval+i*size, theBufs[i], size*sizeof(*retval));

    SAF_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:    	Validate a field I/O request	
 *
 * Description: This function is used to confirm an I/O request, possibly partial, for a field is valid. Currently, there are a
 *		number of limitations on partial I/O requests and this function checks to make sure all of those limitations are
 *		satisfied.
 *
 * Return:	True if the partial I/O request is validatable and one SAF can support; false otherwise.
 *
 * Modifications:
 *		Mark C. Miller, 2000-11-17
 *		Initial Implementation
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_is_valid_io_request(SAF_ParMode pmode,             /* the parallel mode. */
                         SAF_Field *field,		/* the field handle */
                         int member_count,		/* the number of members whose dofs are being written */
                         SAF_RelRep *req_type,	        /* the io request type */
                         int *member_ids,		/* the data indicating which members dofs are being written */
                         int nbufs			/* the number of buffers used to store the dofs */
                         )
{
    SAF_ENTER(_saf_is_valid_io_request, FALSE);
    SAF_FieldTmpl       ftmpl;
    SAF_Set             base;
    ss_collection_t     coll;
    ss_indexspec_t      idx;
    hbool_t             result = TRUE;
    int                 i;

    /* do as little work as possible for typical case */
    if (!req_type)
        SAF_RETURN(FALSE);
    if (!SS_RELREP(req_type))
        SAF_RETURN(FALSE);
    if (SAF_TOTALITY_ID == SS_RELREP(req_type)->id)
        SAF_RETURN(TRUE);

    {
        /* obtain the field template record */
        ftmpl = SS_FIELD(field)->ftmpl;


        /* obtain the set record for the base space of the field */
        base = SS_FIELD(field)->base_space;

        /* we cannot do partial I/O requests on extendible collections in SAF_EACH mode */
        if (SS_SET(&base)->is_extendible && (pmode != SAF_ALL))
            SAF_RETURN(false);

        /* we cannot do partial I/O requests on extendible collections that are not vector interleaved */
        if (SS_SET(&base)->is_extendible &&
            SS_FIELDTMPL(&ftmpl)->num_comps>1 &&
            SS_FIELD(field)->comp_intlv!=SAF_INTERLEAVE_VECTOR)
            SAF_RETURN(false);

        /* obtain the collection record for the collection the field's dofs are n:1 associated with */
        _saf_getCollection_set(&base, SS_FIELD_P(field,dof_assoc_cat), &coll);
   
        /* obtain the indexing record for the collection */
        ss_array_get(SS_COLLECTION_P(&coll,indexing), ss_pers_tm, (size_t)0, (size_t)1, &idx);

        if (SAF_HSLAB_ID == SS_RELREP(req_type)->id) {
            if (SS_INDEXSPEC(&idx)->ndims != 1)
                result = false;

            /* for a 1D collection, member_ids points to 3 ints of start,count,stride */
            if (member_ids[0] < SS_INDEXSPEC(&idx)->origins[0])
                result = false;

            if (member_ids[0] > SS_INDEXSPEC(&idx)->origins[0] + SS_INDEXSPEC(&idx)->sizes[0] - 1)
                result = false;

            if (member_count < 1)
                result = false;

            if (member_count > SS_INDEXSPEC(&idx)->sizes[0])
                result = false;

            if (member_ids[1] < 1)
                result = false;

            if (member_ids[1] > SS_INDEXSPEC(&idx)->sizes[0])
                result = false;

            if (member_ids[1] != member_count)
                result = false;

            if (member_ids[2] != 1)
                result = false;

            if (nbufs != 1)
                result = false;

        } else if (SAF_TUPLES_ID == SS_RELREP(req_type)->id) {
            if ((member_count != 1) && (member_count != 0)) /* we permit 0 when only some processors want to read the value */
                result = false;

            if (member_count > 0) {
                /* for a single member, member ids points to ndims ints identifying one member of the collection */
                for (i = 0; i < SS_INDEXSPEC(&idx)->ndims; i++) {
                    if (member_ids[i] < SS_INDEXSPEC(&idx)->origins[i]) {
                        result = false;
                        break;
                    }

                    if (member_ids[i] > SS_INDEXSPEC(&idx)->origins[i] + SS_INDEXSPEC(&idx)->sizes[i] - 1) {
                        result = false;
                        break;
                    }
                }
            }

            if (nbufs != 1)
                result = false;

        } else {
            result = false;
        }
       
    }

    SAF_LEAVE(result);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Determine if MPI_Finalize has been called
 *
 * Description: This function is a workaround for problems with certain versions of MPI; not all versions have a call to determine
 *		if MPI_Finalize has been called. That is part of the MPI-2 specification but was not part of MPI-1.
 *
 *		If HAVE_MPI_FINALIZED is defined, we just use MPI's MPI_Finalized function. Otherwise, the implementation of this
 *		function depends on the implementation of MPI.
 *
 * Return:	SAF_MPI_FINALIZED_TRUE if SAF is able to determine if MPI_Finalize() has been called and MPI_Finalize() has
 *		indeed been called.
 *
 *		SAF_MPI_FINALIZED_FALSE if SAF is able to determine if MPI_Finalize() has been called and MPI_Finalize() has
 *		indeed *not* been called.
 *
 *		SAF_MPI_FINALIZED_TORF if SAF is unable to determine if MPI_Finalize() has been called.
 *
 * Modifications:
 *		Mark C. Miller, 2000-10-03
 *		Initial Implementation
 *
 *-------------------------------------------------------------------------------------------------------------------------------
 */
SAF_MPIFinalizeMode
_saf_is_mpi_finalized(void)
{
    SAF_ENTER(_saf_is_mpi_finalized, SAF_MPI_FINALIZED_TORF);
    SAF_MPIFinalizeMode retval;

#ifdef HAVE_PARALLEL

#   ifdef HAVE_MPI_FINALIZED
    
    /* here is IBM's implementation */
    int mpi_finalized;
    MPI_Finalized(&mpi_finalized);
    retval = mpi_finalized ? SAF_MPI_FINALIZED_TRUE : SAF_MPI_FINALIZED_FALSE;

#   elif defined(HAVE_PARALLEL) && defined(MPICH_NAME) && MPI_VERSION==1 && MPI_SUBVERSION==1
    /* here is an MPICH-1.1 implementation */
    retval = MPIR_Has_been_initialized == 2 ? SAF_MPI_FINALIZED_TRUE :SAF_MPI_FINALIZED_FALSE;

#   else
    /* Here is the catchall in case we don't have an MPI specific implementation. Basically, it says we don't
       know what the state of MPI is. Not much help! */
    retval = SAF_MPI_FINALIZED_TORF;

#   endif

#else
   /* serial code always returns false */
   retval = SAF_MPI_FINALIZED_FALSE;
#endif

   SAF_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Data Types
 * Purpose:    	Stores whether MPI has been finalized
 *
 * Description: The datatypes defined here are externs declared in order implement the MPI_Finalized functionality for
 *		any version 1 implementation of MPI. These types are specific to the implementation of MPI SAF is linking to
 *		so all should be surrounded by the appropriate pre-processor directives to confine their declaration to
 *		only those situations where they are actually defined by the associated MPI.
 *
 * Modifications:
 *		Mark C. Miller, 2000-10-03
 *		Initial Implementation
 *
 *-------------------------------------------------------------------------------------------------------------------------------
 */
#if defined(HAVE_PARALLEL) && defined(MPICH_NAME) && MPI_VERSION==1 && MPI_SUBVERSION==1
extern int MPIR_Has_been_initialized;
#endif

/*---------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Get a collection
 *
 * Description: SAF originally didn't treat collections as first class objects but rather always referred to them with a
 *              set/category pair. This function takes a set/category pair and returns the collection.
 *
 * Return:      On success returns BUF or an allocated result that the caller should eventually free; returns a null
 *              pointer on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Thursday, April  8, 2004
 *---------------------------------------------------------------------------------------------------------------------------
 */
ss_collection_t *
_saf_getCollection_set(SAF_Set *set,            /* Set to which the desired collection belongs. */
                       SAF_Cat *cat,            /* Category of the desired collection. */
                       ss_collection_t *buf     /* OUT: Optional buffer to hold result. If non is supplied then one will
                                                 * be allocated. */
                      )
{
    SAF_ENTER(_saf_getCollection_set, NULL);
    size_t              i, ncolls;
    ss_collection_t     coll, *retval=NULL;
    ss_cat_t            coll_cat;

    SAF_REQUIRE(SS_SET(set), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("SET must be a valid set handle"));
    SAF_REQUIRE(SS_CAT(cat), SAF_LOW_CHK_COST, NULL,
                _saf_errmsg("CAT must be a valid collection category"));

    if (SS_NOSIZE==(ncolls=ss_array_nelmts(SS_SET_P(set,colls))))
        SAF_ERROR(NULL, _saf_errmsg("couldn't get number of collections defined on the set"));

    /* Scan through the array of collections until we find a collection whose collection category matches the `cat' argument. */
    for (i=0; i<ncolls; i++) {
        ss_array_get(SS_SET_P(set,colls), 0, i, 1, &coll);
        if (!SS_COLLECTION(&coll))
            SAF_ERROR(NULL, _saf_errmsg("cannot obtain collection %lu of set", (unsigned long)i));
        coll_cat = SS_COLLECTION(&coll)->cat;
        if (!SS_CAT(&coll_cat))
            SAF_ERROR(NULL, _saf_errmsg("cannot obtain category for collection %lu of set", (unsigned long)i));
        if (SS_PERS_EQUAL(cat, &coll_cat)>=1) {
            retval = &coll;
            break;
        }
    }

    /* Allocate return value if necessary */
    if (!retval) SAF_RETURN(NULL);
    if (!buf) buf = SS_OBJ_NEW(ss_collection_t);
    *buf = *retval;
    SAF_LEAVE(buf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Private
 * Chapter:     Miscellaneous Utilities
 * Purpose:     Put a collection
 *
 * Description: This function takes a set/category pair and causes the pair to point to the specified colleciton.
 *
 * Return:      Returns non-negative on success; negative on failure.
 *
 * Parallel:    Independent unless PMODE is SAF_ALL, in which case the call must be collective across the communicator
 *              associated with SET and the arguments must be the same on all tasks.
 *
 * Programmer:  Robb Matzke
 *              Monday, June 21, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
int
_saf_putCollection_set(SAF_ParMode pmode,
                       SAF_Set *set,            /* Set to which the new collection belongs. */
                       SAF_Cat *cat,            /* Collection category for the new collection. */
                       ss_collection_t *coll    /* The new collection */
                       )
{
    SAF_ENTER(_saf_putCollection_set, -1);
    ss_collection_t             old;
    size_t                      nelmts;

    SAF_REQUIRE(_saf_valid_pmode(pmode), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("PMODE must be valid"));
    if (!_saf_is_participating_proc(pmode)) SAF_RETURN(-1);

    SAF_REQUIRE(SS_SET(set), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("SET must be a valid set handle"));
    SAF_REQUIRE(SS_CAT(cat), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("CAT must be a valid collection category"));
    SAF_REQUIRE(SS_COLLECTION(coll), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("COLL must be a valid collection"));
    SAF_REQUIRE(SS_PERS_EQ(cat, SS_COLLECTION_P(coll,cat)), SAF_LOW_CHK_COST, SAF_PRECONDITION_ERROR,
                _saf_errmsg("COLL does not point back to CAT"));
    
    /* Make sure this set/cat pair isn't already defined */
    if (NULL!=_saf_getCollection_set(set, cat, &old))
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("%s collection on set %s is already defined",
                                              ss_string_ptr(SS_CAT_P(cat,name)), ss_string_ptr(SS_SET_P(set,name))));

    /* Add the new collection to the array */
    nelmts = ss_array_nelmts(SS_SET_P(set,colls));
    if (ss_array_resize(SS_SET_P(set,colls), nelmts+1)<0 ||
        ss_array_put(SS_SET_P(set,colls), ss_pers_tm, nelmts, (size_t)1, coll)<0)
        SAF_ERROR(SAF_FILE_ERROR, _saf_errmsg("cannot add an additional collection"));
    SAF_DIRTY(set, pmode);

    SAF_LEAVE(0);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Data Types
 * Purpose:     Determine if datatype is primitive
 *
 * Description: A "primitive" datatype is anything that's an integer or floating point type.
 *
 * Return:      Returns true if TYPE is primitive; false otherwise.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Friday, May 21, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
hbool_t
_saf_is_primitive_type(hid_t type)
{
    SAF_ENTER(_saf_is_primitive_type, FALSE);
    hbool_t     retval=FALSE;

    /* Opaque types are specificaly excluded from this list because they're sometimes used for much more complex SSlib
     * datatypes. */
    retval = (H5T_INTEGER==H5Tget_class(type) ||
              H5T_FLOAT==H5Tget_class(type) ||
              H5T_TIME==H5Tget_class(type) ||
              H5T_BITFIELD==H5Tget_class(type) ||
              H5T_ENUM==H5Tget_class(type));

    SAF_LEAVE(retval);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Data Types
 * Purpose:     Convert a single value
 *
 * Description: Converts a single value from one datatype to another. This is most often used to convert a runtime typed value
 *              into an integer to be used by the library.
 *
 * Return:      Returns DSTBUF (or an allocated buffer if DSTBUF is null) on success; returns null on failure.
 *
 * Parallel:    Independent
 *
 * Programmer:  Robb Matzke
 *              Tuesday, May 25, 2004
 *-------------------------------------------------------------------------------------------------------------------------------
 */
void *
_saf_convert(hid_t srctype,                     /* Source datatype; type of SRCBUF value. */
             const void *srcbuf,                /* Source datum to be converted to a new type. */
             hid_t dsttype,                     /* Destination datatype; type of DSTBUF value. */
             void *dstbuf                       /* Optional destination buffer. If not supplied then a buffer is allocated. */
             )
{
    SAF_ENTER(_saf_convert, NULL);
    static char tmp[128];
    size_t srcsize = H5Tget_size(srctype);
    size_t dstsize = H5Tget_size(dsttype);

    if (!srcsize || !dstsize)
        SAF_ERROR(NULL, _saf_errmsg("unable to get size of source or destination datatype"));
    assert(srcsize<=sizeof(tmp) && dstsize<=sizeof(tmp));
    memcpy(tmp, srcbuf, srcsize);
    if (H5Tconvert(srctype, dsttype, 1, tmp, NULL, H5P_DEFAULT)<0)
        SAF_ERROR(NULL, _saf_errmsg("datatype conversion failed"));
    if (!dstbuf && NULL==(dstbuf=malloc(dstsize)))
        SAF_ERROR(NULL, _saf_errmsg("unable to allocate return value"));
    memcpy(dstbuf, tmp, dstsize);
    SAF_LEAVE(dstbuf);
}

/*-------------------------------------------------------------------------------------------------------------------------------
 * Audience:    Public
 * Chapter:     Miscellaneous Utilities
 * Purpose:    	Exchange handles
 *
 * Description: This function is used to exchange handles created locally by processes for global writing.  This
 *		is generally done when collecting the local handles to be written stored with an indirect
 *		relation or field.
 *
 * Return:      Returns a buffer of handles on success or null on failure. The buffer is either the non-null value of the
 *              RESULT argument or a buffer which is allocated by this function.
 *
 * Parallel:    This call must be collective across the communicator for the given database.
 *-------------------------------------------------------------------------------------------------------------------------------
 */
ss_pers_t *
saf_allgather_handles(ss_pers_t *_pers, /* A Pointer to the handle to be exchanged.  Every participant must supply a valid
                                         * handle of the same type and in the same scope as every other participant. */
		      int *commsize,    /* OUT: A pointer to optional caller supplied memory which is to receive the integer
                                         * number of handles returned by this function.  This is the number of participants or
                                         * the size of the communicator associated with the given database. */
		      ss_pers_t *result /* OUT: An optional pointer to an array that will will be initialized with a handle
                                         * from each MPI task in task rank order. If this buffer is supplied then it should be
                                         * at least as large as the communicator associated with the DB argument. If not
                                         * supplied (i.e., null) then a buffer will be allocated for the return value. */
		      )
{
    SAF_ENTER(saf_allgather_handles, NULL);
    ss_scope_t          scope=SS_SCOPE_NULL;
    ss_file_t           file=SS_FILE_NULL;
    int                 self, ntasks=0;
    size_t              encoded_nused=0, encoded_nalloc=0, consumed;
    char                *encoded=NULL, *all_encoded=NULL;
    MPI_Comm            comm=SS_COMM_NULL;

    /* It might be possible that the ss_pers_t pointer _PERS is a pointer into some object. If that object is a new object
     * then the ss_file_synchronize() call below may clobber that memory. Therefore we copy the link onto the stack first. */
    ss_pers_t           pers=*_pers;

    SAF_REQUIRE(ss_pers_deref(&pers), SAF_LOW_CHK_COST, NULL, _saf_errmsg("_PERS must be a valid object link"));

    /* Obtain communicator info. */
    ss_pers_scope(&pers, &scope);
    ss_scope_comm(&scope, &comm, &self, &ntasks);
    if (commsize) *commsize = ntasks;
    
    /* Synchronize the table, thus turning the _PERS arguments into global objects. */
    ss_pers_file(&pers, &file);
    if (ss_file_synchronize(&file, NULL)<0)
        SAF_ERROR(NULL, _saf_errmsg("file synchronization failed"));
    
    /* Each task encodes its _PERS argument. They should all be the same size when encoded. */
    encoded = ss_pers_encode_cb(&pers, NULL, &encoded_nused, &encoded_nalloc, sizeof(pers), 1);

    /* Gather all the encoded object links */
    all_encoded = malloc(ntasks*encoded_nused);
    memcpy(all_encoded+self*encoded_nused, encoded, encoded_nused);
    ss_mpi_allgather(all_encoded, encoded_nused, MPI_BYTE, comm);

    /* Decode the object links into the result buffer. */
    if (!result) result = malloc(ntasks*sizeof(*result));
    consumed = ss_pers_decode_cb(result, all_encoded, sizeof(*result), (size_t)ntasks);
    assert(consumed==ntasks*encoded_nused);
    
    /* Free temporary resources */
    encoded = SS_FREE(encoded);
    all_encoded = SS_FREE(all_encoded);

    SAF_LEAVE(result);
}
