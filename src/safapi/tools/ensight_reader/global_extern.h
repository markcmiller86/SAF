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
/*--------------------------------------------------------------*/
/* Header file for EnSight External Reader DSO Library Routines */
/*--------------------------------------------------------------*/
/*  *************************************************************
 *   Copyright 1998 Computational Engineering International, Inc.
 *   All Rights Reserved.
 *
 *        Restricted Rights Legend
 *
 *   Use, duplication, or disclosure of this
 *   software and its documentation by the
 *   Government is subject to restrictions as
 *   set forth in subdivision [(b)(3)(ii)] of
 *   the Rights in Technical Data and Computer
 *   Software clause at 52.227-7013.
 *  *************************************************************
 */
#ifndef GLOBAL_EXTERN_H
#define GLOBAL_EXTERN_H

/*--------------------------------
 * Set the reader version define
 * (only one can be set at a time)
 *--------------------------------*/
#if 0
#define USERD_API_100
#define USERD_API_200
#define USERD_API_201
#endif
#define USERD_API_202

/*----------------------------------------
 * Set this appropriately:
 *  DO_ENSIGHT  if using for EnSight itself
 *  DO_READER   if using in a reader
 *----------------------------------------*/
#if 1
#define DO_READER
#else
#define DO_ENSIGHT
#endif

/*---------------------------------------*/
/* True/False and Error conditions, etc. */
/*---------------------------------------*/
#define Z_ERR                  (-1)          /*Error return value.*/
#define Z_OK                    (1)          /*Success return value.*/
#define Z_UNDEF                 (2)          /*Undefined return value.*/
#define Z_NOT_IMPLEMENTED       (3)          /*Routine not implemented*/
                                             /*(currently only checked for */
                                             /* get_var_value_at_specific routine)*/

#ifndef TRUE
# define TRUE                   (1)
# define FALSE                  (0)
#endif

#define Z_BUFL                 (80)          /* Typical string length */

#define Z_COMPX                 (0)          /* x component */
#define Z_COMPY                 (1)          /* y component */
#define Z_COMPZ                 (2)          /* z component */

#define Z_STATIC                (0)          /* static geometry          */
#define Z_CHANGE_COORDS         (1)          /* coordinate changing only */
#define Z_CHANGE_CONN           (2)          /* conectivity changing     */

#define Z_GEOM                  (0)          /* Geometry type */
#define Z_VARI                  (1)          /* Variable type */

#define Z_SAVE_ARCHIVE          (0)          /* Save archive    */
#define Z_REST_ARCHIVE          (1)          /* Restore archive */

#define Z_MAX_USERD_NAME        (20)         /* max length of reader name */

#define Z_PER_NODE              (4)          /* At Nodes Variable classif.   */
#define Z_PER_ELEM              (1)          /* At Elements Variable classif.*/

#define Z_MAX_SETS              (300)

#ifndef GLOBALDEFS_H
/*-----------------------------------*/
/* Unstructured coordinate structure */
/*-----------------------------------*/
typedef struct {
  float xyz[3];
}CRD;
#endif

/*----------------*/ 
/* Variable Types */
/*----------------*/ 
enum z_var_type
{
  Z_CONSTANT,
  Z_SCALAR,
  Z_VECTOR,
  Z_TENSOR,
  Z_TENSOR9,
  MAX_Z_VAR_TYPES
};

/*---------------
 * Element Types
 *---------------
 * If you mess with these, you must also
 * change the get_z_maxtype
 *            to_z_elem_type
 *            to_int_elem_type routines
 * in userd_read.c
 *----------------------------------------*/
#if (defined USERD_API_100 || defined USERD_API_200) && defined DO_READER
enum z_elem_types {
  Z_POINT,         /* 00:  1 node point element */
  Z_BAR02,         /* 01:  2 node bar           */
  Z_BAR03,         /* 02:  3 node bar           */
  Z_TRI03,         /* 03:  3 node triangle      */
  Z_TRI06,         /* 04:  6 node triangle      */
  Z_QUA04,         /* 05:  4 node quad          */
  Z_QUA08,         /* 06:  8 node quad          */
  Z_TET04,         /* 07:  4 node tetrahedron   */
  Z_TET10,         /* 08: 10 node tetrahedron   */
  Z_PYR05,         /* 09:  5 node pyramid       */
  Z_PYR13,         /* 10: 13 node pyramid       */
  Z_PEN06,         /* 11:  6 node pentahedron   */
  Z_PEN15,         /* 12: 15 node pentahedron   */
  Z_HEX08,         /* 13:  8 node hexahedron    */
  Z_HEX20,         /* 14: 20 node hexahedron    */
  Z_MAXTYPE
};

#elif defined USERD_API_201 && defined DO_READER
enum z_elem_types {
  Z_POINT,         /* 00:  1 node point element              */
  Z_G_POINT,       /* 01:  1 node point element (ghost call) */
  Z_BAR02,         /* 02:  2 node bar                        */
  Z_G_BAR02,       /* 03:  2 node bar           (ghost cell) */
  Z_BAR03,         /* 04:  3 node bar                        */
  Z_G_BAR03,       /* 05:  3 node bar           (ghost cell) */
  Z_TRI03,         /* 06:  3 node triangle                   */
  Z_G_TRI03,       /* 07:  3 node triangle      (ghost cell) */
  Z_TRI06,         /* 08:  6 node triangle                   */
  Z_G_TRI06,       /* 09:  6 node triangle      (ghost cell) */
  Z_QUA04,         /* 10:  4 node quad                       */
  Z_G_QUA04,       /* 11:  4 node quad          (ghost cell) */
  Z_QUA08,         /* 12:  8 node quad                       */
  Z_G_QUA08,       /* 13:  8 node quad          (ghost cell) */
  Z_TET04,         /* 14:  4 node tetrahedron                */
  Z_G_TET04,       /* 15:  4 node tetrahedron   (ghost cell) */
  Z_TET10,         /* 16: 10 node tetrahedron                */
  Z_G_TET10,       /* 17: 10 node tetrahedron   (ghost cell) */
  Z_PYR05,         /* 18:  5 node pyramid                    */
  Z_G_PYR05,       /* 19:  5 node pyramid       (ghost cell) */
  Z_PYR13,         /* 20: 13 node pyramid                    */
  Z_G_PYR13,       /* 21: 13 node pyramid       (ghost cell) */
  Z_PEN06,         /* 22:  6 node pentahedron                */
  Z_G_PEN06,       /* 23:  6 node pentahedron   (ghost cell) */
  Z_PEN15,         /* 24: 15 node pentahedron                */
  Z_G_PEN15,       /* 25: 15 node pentahedron   (ghost cell) */
  Z_HEX08,         /* 26:  8 node hexahedron                 */
  Z_G_HEX08,       /* 27:  8 node hexahedron    (ghost cell) */
  Z_HEX20,         /* 28: 20 node hexahedron                 */
  Z_G_HEX20,       /* 29: 20 node hexahedron    (ghost cell) */
  Z_MAXTYPE
};

#else
enum z_elem_types {
  Z_POINT,         /* 00:  1 node point element              */
  Z_G_POINT,       /* 01:  1 node point element (ghost call) */
  Z_BAR02,         /* 02:  2 node bar                        */
  Z_G_BAR02,       /* 03:  2 node bar           (ghost cell) */
  Z_BAR03,         /* 04:  3 node bar                        */
  Z_G_BAR03,       /* 05:  3 node bar           (ghost cell) */
  Z_TRI03,         /* 06:  3 node triangle                   */
  Z_G_TRI03,       /* 07:  3 node triangle      (ghost cell) */
  Z_TRI06,         /* 08:  6 node triangle                   */
  Z_G_TRI06,       /* 09:  6 node triangle      (ghost cell) */
  Z_QUA04,         /* 10:  4 node quad                       */
  Z_G_QUA04,       /* 11:  4 node quad          (ghost cell) */
  Z_QUA08,         /* 12:  8 node quad                       */
  Z_G_QUA08,       /* 13:  8 node quad          (ghost cell) */
  Z_TET04,         /* 14:  4 node tetrahedron                */
  Z_G_TET04,       /* 15:  4 node tetrahedron   (ghost cell) */
  Z_TET10,         /* 16: 10 node tetrahedron                */
  Z_G_TET10,       /* 17: 10 node tetrahedron   (ghost cell) */
  Z_PYR05,         /* 18:  5 node pyramid                    */
  Z_G_PYR05,       /* 19:  5 node pyramid       (ghost cell) */
  Z_PYR13,         /* 20: 13 node pyramid                    */
  Z_G_PYR13,       /* 21: 13 node pyramid       (ghost cell) */
  Z_PEN06,         /* 22:  6 node pentahedron                */
  Z_G_PEN06,       /* 23:  6 node pentahedron   (ghost cell) */
  Z_PEN15,         /* 24: 15 node pentahedron                */
  Z_G_PEN15,       /* 25: 15 node pentahedron   (ghost cell) */
  Z_HEX08,         /* 26:  8 node hexahedron                 */
  Z_G_HEX08,       /* 27:  8 node hexahedron    (ghost cell) */
  Z_HEX20,         /* 28: 20 node hexahedron                 */
  Z_G_HEX20,       /* 29: 20 node hexahedron    (ghost cell) */
  Z_NSIDED,        /* 30:  n node polygon                    */
  Z_G_NSIDED,      /* 31:  n node polygon       (ghost cell) */
  Z_NFACED,        /* 32:  n faced polyhedron                */
  Z_G_NFACED,      /* 33:  n faced polyhedron   (ghost cell) */
  Z_MAXTYPE
};

#endif


/*-------------------------------*/
/* Unstructured/Structured types */
/*-------------------------------*/
enum z_structured_defs
{
  Z_UNSTRUCTURED,         /* for unstructured part */
  Z_STRUCTURED,           /* for structured (non-iblanked) part */
  Z_IBLANKED,             /* for structured iblanked part */
  Z_MAXMESHTYPES
};

/*----------------------------*/
/* Structured Iblanking types */
/*----------------------------*/
enum z_iblank_domain
{
  Z_EXT,                  /* Exterior */
  Z_INT,                  /* Interior */
  Z_BND,                  /* Boundary */
  Z_INTBND,               /* Internal boundary/baffle */
  Z_SYM,                  /* Symmetry surface */
  Z_NO_OF_IBLANK_DOMAIN_ITEMS
};


/*-----------------------------------*/
/* Dataset Query file info Structure */
/*-----------------------------------*/
#define Z_MAXFILENP    255  /* Max file name and path.*/
#define Z_MAXTIMLEN     40  /* Max time str length */
#define Z_BUFLEN        82  /* Allocated length of the f_desc strings */
typedef struct {
    char name[Z_MAXFILENP];
    long sizeb;
    char timemod[Z_MAXTIMLEN];
    int num_d_lines;
    char **f_desc;
} Z_QFILES;


/* Prototypes for userd_lib */
/*--------------------------*/
#ifdef WIN32
#define W32IMPORT __declspec( dllimport )
#else
#define W32IMPORT extern
#endif


/*----------------------
 * Same in All Versions
 *----------------------*/
W32IMPORT int
USERD_get_number_of_model_parts( void );

W32IMPORT int
USERD_get_block_coords_by_component(int block_number,
                                    int which_component,
                                    float *coord_array);

W32IMPORT int
USERD_get_block_iblanking(int block_number,
                          int *iblank_array);

W32IMPORT int
USERD_get_block_scalar_values(int block_number,
                              int which_scalar,
                              float *scalar_array);

W32IMPORT int
USERD_get_block_vector_values_by_component(int block_number,
                                           int which_vector,
                                           int which_component,
                                           float *vector_array);

W32IMPORT int
USERD_get_name_of_reader(char reader_name[Z_MAX_USERD_NAME],
                         int *two_fields);
     
W32IMPORT int
USERD_set_filenames(char filename_1[],
                    char filename_2[],
                    char the_path[],
                    int swapbytes);
     
W32IMPORT int
USERD_get_number_of_files_in_dataset( void );

W32IMPORT int
USERD_get_dataset_query_file_info(Z_QFILES *qfiles);
     
W32IMPORT int
USERD_get_changing_geometry_status( void );

W32IMPORT int
USERD_get_node_label_status( void );

W32IMPORT int
USERD_get_element_label_status( void );

W32IMPORT int
USERD_get_number_of_variables( void );

W32IMPORT void
USERD_stop_part_building( void );

W32IMPORT int
USERD_bkup(FILE *archive_file,
           int backup_type);



/*-----------------------
 * For Version 1.000 Only
 *-----------------------*/
#if defined USERD_API_100 || defined DO_ENSIGHT

W32IMPORT int
USERD_get_number_of_global_nodes();
     
W32IMPORT int
USERD_get_global_coords(CRD *coord_array);
     
W32IMPORT int
USERD_get_global_node_ids(int *nodeid_array);
     
W32IMPORT int
USERD_get_element_connectivities_for_part(int part_number,
                                          int **conn_array[Z_MAXTYPE]);

W32IMPORT int
USERD_get_element_ids_for_part(int part_number,
                               int *elemid_array[Z_MAXTYPE]);

W32IMPORT int
USERD_get_vector_values(int which_vector,
                        int which_part,
                        int which_type,
                        float *vector_array);
     
W32IMPORT int
USERD_get_part_build_info(int *part_id,
                          int *part_types,
                          char *part_descriptions[Z_BUFL],
                          int *number_of_elements[Z_MAXTYPE],
                          int *ijk_dimensions[3],
                          int *iblanking_options[6]);

W32IMPORT int
USERD_get_scalar_values(int which_scalar,
                        int which_part,
                        int which_type,
                        float *scalar_array);

W32IMPORT int
USERD_get_variable_info(char **var_description,
                        char **var_filename,
                        int *var_type,
                        int *var_classify);

W32IMPORT int
USERD_get_description_lines(int which_type,
                            int which_var,
                            char line1[Z_BUFL],
                            char line2[Z_BUFL]);

W32IMPORT int
USERD_get_variable_value_at_specific(int which_var,
                                     int which_node_or_elem,
                                     int which_part,
                                     int which_elem_type,
                                     int time_step,
                                     float values[3]);

W32IMPORT float
USERD_get_constant_value(int which_var);

W32IMPORT int
USERD_get_solution_times(float *solution_times);
W32IMPORT void
USERD_set_time_step(int time_step);

W32IMPORT int
USERD_get_number_of_time_steps(void);

#endif


/*----------------------
 * New For Version 2.000
 *----------------------*/
#if !defined USERD_API_100 || defined DO_ENSIGHT

W32IMPORT int
USERD_get_part_coords(int part_number,
                      float **coord_array);

W32IMPORT int
USERD_get_part_node_ids(int part_number,
                        int *nodeid_array);
     
W32IMPORT int
USERD_get_part_elements_by_type(int part_number,
                                int element_type,
                                int **conn_array);
W32IMPORT int
USERD_get_part_element_ids_by_type(int part_number,
                                   int element_type,
                                   int *elemid_array);
     
W32IMPORT int
USERD_get_reader_version(char version_number[Z_MAX_USERD_NAME]);

W32IMPORT int
USERD_get_var_by_component(int which_variable,
                           int which_part,
                           int var_type,
                           int which_type,
                           int complex,
                           int component,
                           float *var_array);
W32IMPORT int
USERD_get_gold_part_build_info(int *part_id,
                               int *part_types,
                               char *part_descriptions[Z_BUFL],
                               int *number_of_nodes,
                               int *number_of_elements[Z_MAXTYPE],
                               int *ijk_dimensions[3],
                               int *iblanking_options[6]);
W32IMPORT int
USERD_get_maxsize_info(int *max_number_of_nodes,
                       int *max_number_of_elements[Z_MAXTYPE],
                       int *max_ijk_dimensions[3]);

W32IMPORT void
USERD_exit_routine( void );

W32IMPORT int
USERD_get_gold_variable_info(char **var_description,
                             char **var_filename,
                             int *var_type,
                             int *var_classify,
                             int *var_complex,
                             char **var_ifilename,
                             float *var_freq,
                             int *var_contran,
                             int *var_timeset);
W32IMPORT int
USERD_get_model_extents( float extents[6] );

W32IMPORT int
USERD_get_descrip_lines(int which_type,
                        int which_var,
                        int imag_data,
                        char line1[Z_BUFL],
                        char line2[Z_BUFL]);

W32IMPORT int
USERD_get_var_value_at_specific(int which_var,
                                int which_node_or_elem,
                                int which_part,
                                int which_elem_type,
                                int time_step,
                                float values[3],
                                int imag_data);

W32IMPORT float
USERD_get_constant_val(int which_var, int imag_data);

W32IMPORT int
USERD_get_geom_timeset_number(void);

W32IMPORT int
USERD_get_number_of_timesets(void);

W32IMPORT int
USERD_get_timeset_description(int timeset_number,
                              char timeset_description[Z_BUFL]);

W32IMPORT int
USERD_get_sol_times(int timeset_number,
                    float *solution_times);
W32IMPORT void
USERD_set_time_set_and_step(int timeset_number,
                            int time_step);
W32IMPORT int
USERD_get_num_of_time_steps(int timeset_number);

W32IMPORT int
USERD_get_border_availability(int part_number,
                              int number_of_elements[Z_MAXTYPE]);

W32IMPORT int
USERD_get_border_elements_by_type(int part_number,
                                  int element_type,
                                  int **conn_array,
                                  short *parent_element_type,
                                  int *parent_element_num);

W32IMPORT void
USERD_set_server_number(int serv_num,
                        int tot_servs);

#endif


/*----------------------
 * New For Version 2.010
 *----------------------*/
#if defined USERD_API_201 || defined USERD_API_202 || defined DO_ENSIGHT
W32IMPORT int
USERD_get_ghosts_in_model_flag( void );

W32IMPORT int
USERD_get_ghosts_in_block_flag(int block_number);

W32IMPORT int
USERD_get_block_ghost_flags(int block_number,
                            int *ghost_flags);

#endif




int USERD_get_reader_descrip(char descrip[Z_MAXFILENP]);/*jake added*/
int USERD_get_reader_release(char version_number[Z_MAX_USERD_NAME]);/*jake added*/

     
/*--------------------------------------------------------------------*/
#endif /*GLOBAL_EXTERN_H*/

