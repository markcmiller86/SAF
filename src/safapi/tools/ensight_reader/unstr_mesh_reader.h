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
/*
 *  Functions for reading UNstructured mesh data from a SAF file
 *
 *  For general instructions, see the source files str_mesh_reader.c and
 *  unstr_mesh_reader.c
 */

#ifndef UNSTR_MESH_READER_H
#define UNSTR_MESH_READER_H

#include <str_mesh_reader.h>
#include <variable_names.h>



#ifdef __cplusplus 

class unstr_mesh_reader_t; //forward declaration

class SAFUnstrMeshReader 
{
 public:

  SAFUnstrMeshReader( );
  ~SAFUnstrMeshReader( );
  SAFStrMeshReader *getStrReader( )    { return(m_strReader); }

  /*IMPORTANT NOTE: IF COMPILING IN C++, YOU MUST USE THIS FUNCTION TO CONNECT
    THIS INSTANCE TO A STRUCTURED MESH READER INSTANCE, BEFORE ANYTHING ELSE*/
  void setStrReader( SAFStrMeshReader *a_strReader ) { m_strReader = a_strReader;  }
      
# endif


  /*
  ** init_unstr_mesh_reader must be called before any other functions in
  ** this header file (except is_unstr_mesh_reader_initialized). It counts
  ** the number of unstructured parts and variables, and stores some 
  ** information in some global arrays (or member vars if in C++). All of the 
  ** other functions in this header file access those global arrays.
  **
  ** Note: init_str_mesh_reader (from str_mesh_reader.h) must be called before
  ** init_unstr_mesh_reader, because it also initializes the timestep data.
  ** 
  ** Note: the filename for the saf database is specified as an argument
  ** to init_str_mesh_reader (from str_mesh_reader.h)
  */
  int init_unstr_mesh_reader( int a_allowQuadraticElements );
  int is_unstr_mesh_reader_initialized(void);

  /*
  ** finalize_unstr_mesh_reader frees and resets all of the global arrays
  ** and variables. It should be called before finalize_str_mesh_reader
  ** (from str_mesh_reader.h)
  */
  int finalize_unstr_mesh_reader( void );


  int get_num_unstr_vars(void);
  int get_num_unstr_parts( void );

  /*
  ** This function answers the question, "is this variable defined on this
  ** part at this timestep?"
  */
  int is_there_an_unstr_var_instance( int a_whichPart,
				      int a_whichVar,
				      int a_whichTimestep );


  int get_index_of_set(SAF_Set *a_set);
  SAF_Set *get_set_of_index(int a_index); 

  int is_this_unstr_var_mapped_to_quad( int a_whichVar );
  int is_this_unstr_var_mapped_to_tet( int a_whichVar );
  int is_this_unstr_var_mapped_to_hex( int a_whichVar );
  int is_this_unstr_var_mapped_to_tri( int a_whichVar );
  int is_this_unstr_var_mapped_to_pyramid( int a_whichVar );
  int is_this_unstr_var_mapped_to_point( int a_whichVar );
  int is_this_unstr_var_mapped_to_prism( int a_whichVar );
  int is_this_unstr_var_mapped_to_line( int a_whichVar );
  int is_this_unstr_var_mapped_to_0ball( int a_whichVar );
  int is_this_unstr_var_mapped_to_1ball( int a_whichVar );
  int is_this_unstr_var_mapped_to_2ball( int a_whichVar );
  int is_this_unstr_var_mapped_to_3ball( int a_whichVar );
  int is_this_unstr_var_mapped_to_multiple_cell_types( int a_whichVar );
  int is_this_unstr_var_mapped_to_mixed( int a_whichVar );


  int is_this_unstr_var_mapped_to_quadraticQuad( int a_whichVar );
  int is_this_unstr_var_mapped_to_quadraticTet( int a_whichVar );
  int is_this_unstr_var_mapped_to_quadraticHex( int a_whichVar );
  int is_this_unstr_var_mapped_to_quadraticTri( int a_whichVar );
  int is_this_unstr_var_mapped_to_quadraticPyramid( int a_whichVar );
  int is_this_unstr_var_mapped_to_quadraticLine( int a_whichVar );
  int is_this_unstr_var_mapped_to_quadraticPrism( int a_whichVar );




  int is_this_unstr_var_mapped_to_quad_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_tet_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_hex_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_tri_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_pyramid_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_point_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_prism_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_line_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_0ball_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_1ball_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_2ball_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_3ball_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_mixed_on_this_part( int a_whichVar, int a_whichPart );


  int is_this_unstr_var_mapped_to_quadraticQuad_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_quadraticTet_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_quadraticHex_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_quadraticTri_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_quadraticPyramid_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_quadraticPrism_on_this_part( int a_whichVar, int a_whichPart );
  int is_this_unstr_var_mapped_to_quadraticLine_on_this_part( int a_whichVar, int a_whichPart );


  int is_this_unstr_var_scalar( int a_whichVar );
  int is_this_unstr_var_vector( int a_whichVar );
  int is_this_unstr_var_tensor( int a_whichVar );
  int is_this_unstr_var_symtensor( int a_whichVar );

  int is_a_nodal_unstr_var( int a_whichVar );

  /*
  ** The read_unstr_*_variable functions all contain the argument
  ** a_howManyEntriesIExpect, which must contain the number of variable
  ** elements in the part for that time. (e.g. if it is a scalar variable with
  ** 10 elements, a_howManyEntriesIExpect must be 10; if it is a vector
  ** variable with 10 elements, a_howManyEntriesIExpect must still be 10)
  ** If a_howManyEntriesIExpect is not correct, the function will fail.
  */

  int read_unstr_variable_internal( int a_whichPart,
				    int a_whichVar,
				    int a_whichTimestep,
				    MY_PRECISION *a_data,
				    int a_howManyEntriesIExpect,
				    const int a_whichComponent,
				    SAF_Interleave a_interleave );



  int read_unstr_scalar_variable( int a_whichPart,
				  int a_whichVar,
				  int a_whichTimestep,
				  MY_PRECISION *a_data,
				  int a_howManyEntriesIExpect );

  int read_unstr_vector_variable( int a_whichPart,
				  int a_whichVar,
				  int a_whichTimestep,
				  int a_whichComponent,
				  MY_PRECISION *a_data,
				  int a_howManyEntriesIExpect );


  int read_unstr_vector_variable_entire( int a_whichPart,
				  int a_whichVar,
				  int a_whichTimestep,
				  MY_PRECISION *a_data,
				  int a_howManyEntriesIExpect );

  int read_unstr_symtensor_variable( int a_whichPart,
				     int a_whichVar,
				     int a_whichTimestep,
				     int a_whichComponent,
				     MY_PRECISION *a_data,
				     int a_howManyEntriesIExpect );
  int read_unstr_tensor_variable( int a_whichPart,
				  int a_whichVar,
				  int a_whichTimestep,
				  int a_whichComponent,
				  MY_PRECISION *a_data,
				  int a_howManyEntriesIExpect );


  int get_unstr_var_name( int a_whichVar, char *a_str, int a_maxlen );
  int get_unstr_part_name( int a_whichPart, char *a_str, int a_maxlen );

  /*
   * Get the unit name for the given variable, e.g. Kelvin, Celsius...
   */
  char *get_unstr_var_unit_name( int a_which );

  /*
   * Get the quantity name for the given variable, e.g. Temperature, Frequency ...
   */
  char *get_unstr_var_quant_name( int a_which );

  /*
  ** This function will get the coordinates, whether they are contained on the
  ** set itself or not. It returns the number of points. If a_data is non-zero,
  ** then the coordinates will be copied to it in xyzxyzxyz.... order. So, a_data
  ** should be either correctly allocated or zero.
  */
  int get_unstr_point_coords_used_by_part( int a_whichPart, int a_whichTimestep, MY_PRECISION *a_data );

  /*
  ** The get_*_element_connectivity functions each have an a_data argument
  ** which must already be allocated, and will be returned with 0-based 
  ** connectivity data.
  */
  int get_point_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_tri_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_quad_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_tet_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_hex_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_pyramid_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_prism_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_line_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_0ball_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_1ball_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_2ball_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_3ball_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );

  int get_quadraticTri_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_quadraticQuad_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_quadraticTet_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_quadraticHex_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_quadraticPyramid_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_quadraticPrism_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );
  int get_quadraticLine_element_connectivity( int a_whichPart, int a_whichTimestep, int *a_data );

  int get_num_point_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_tri_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_quad_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_tet_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_hex_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_pyramid_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_prism_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_line_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_0ball_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_1ball_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_2ball_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_3ball_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );

  int get_num_quadraticTri_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_quadraticQuad_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_quadraticTet_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_quadraticHex_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_quadraticPyramid_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_quadraticPrism_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_quadraticLine_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );


  int get_num_unknown_elements_in_unstr_part( int a_whichPart, int a_whichTimestep );
  int get_num_unknown_elements_in_set( SAF_Set );

  int get_num_nodes_in_celltype( SAF_CellType a_type, int a_isQuadratic );

  /*
  ** Answers the question: if we call get_unstr_point_coords_used_by_part() for the
  ** same part but with different timesteps, will we get the same data?
  ** Return -1 for error, 0 for no, 1 for yes.
  **
  ** Note: this is currently very strict, in that it could return no when
  ** the answer is really yes.
  */
  int are_unstr_point_coords_used_by_part_time_independent( int a_whichPart );

  /*
   * Returns 1 if this var is made up of coordinate fields (the first field, at least).
   * This would imply (usually?) that the field represents the displacement from
   * the time-independent default coordinates.
   */
  int is_this_a_coordinate_var( int a_whichVar );




  void name_all_parts( void );


  /**************************************************************************
   * note that the following functions should not be used by a reader, only internally
   */

  void initialize_unstr_globals(void);
  SAF_Set on_which_other_set_are_this_sets_nodes_stored(SAF_Set a_set, int a_timestep, int **a_xform);
  int does_this_set_have_unstr_point_coords_for_this_time( SAF_Set a_set, int a_whichTimestep, int a_numDims );
  int on_which_other_set_index_are_this_sets_nodes_stored(SAF_Set a_set, int a_timestep, int **a_xform);



  int get_num_var_fields_on_subsets_of_set( SAF_Set a_set, int a_whichTimestep, int a_maxNum, 
					    SAF_Field *a_fields, SAF_CellType a_cellType );
  int get_num_var_fields_on_supersets_of_set( SAF_Set a_set, int a_whichTimestep, int a_maxNum, 
					      SAF_Field *a_fields, SAF_CellType a_cellType );

  int can_this_field_be_a_variable( SAF_Set a_set, SAF_Field a_field );
  int get_all_fields_for_set( SAF_Set a_set, SAF_Field **a_fields, int **a_times, int **a_var, 
			      int *a_numCoordFields, SAF_Field **a_coordFields );

  int get_num_tri_elements_in_set( SAF_Set a_set );
  int get_num_quad_elements_in_set( SAF_Set a_set );
  int get_num_hex_elements_in_set( SAF_Set a_set );
  int get_num_pyramid_elements_in_set( SAF_Set a_set );
  int get_num_tet_elements_in_set( SAF_Set a_set );
  int get_num_prism_elements_in_set( SAF_Set a_set );
  int get_num_line_elements_in_set( SAF_Set a_set );
  int get_num_0ball_elements_in_set( SAF_Set a_set );
  int get_num_1ball_elements_in_set( SAF_Set a_set );
  int get_num_2ball_elements_in_set( SAF_Set a_set );
  int get_num_3ball_elements_in_set( SAF_Set a_set );
  int get_num_mixed_elements_in_set( SAF_Set a_set );

  int get_num_quadraticTri_elements_in_set( SAF_Set a_set );
  int get_num_quadraticQuad_elements_in_set( SAF_Set a_set );
  int get_num_quadraticHex_elements_in_set( SAF_Set a_set );
  int get_num_quadraticPyramid_elements_in_set( SAF_Set a_set );
  int get_num_quadraticTet_elements_in_set( SAF_Set a_set );
  int get_num_quadraticPrism_elements_in_set( SAF_Set a_set );
  int get_num_quadraticLine_elements_in_set( SAF_Set a_set );


  int is_this_unstr_var_of_this_algebraic_type( int a_whichVar, SAF_Algebraic *a_algType );
  int is_this_unstr_var_mapped_to_this_celltype( int a_whichVar, SAF_CellType a_cellType );
  int is_this_unstr_var_mapped_to_this_celltype_on_this_part( int a_whichVar, SAF_CellType a_cellType, int a_whichPart );


  int is_this_a_valid_unstr_set( SAF_Set a_set, int *a_isConnectivityPointsOnly );

  void get_all_unstr_sets(void);
  int get_num_topo_rels_in_set( SAF_Set a_set, SAF_CellType a_cellType, int *a_totalNumElems, SAF_Rel **a_rels,
				int a_isQuadraticCellType );
  int get_num_subsets_of_set( SAF_Set a_set, SAF_Set a_subset, SAF_CellType a_cellType, SAF_Rel **a_rels );
  int get_num_2d_coord_points_in_set( SAF_Set a_set, int a_timestep );
  int change_from_component_to_vector_interleave( int a_numComponents, size_t a_numEntries, MY_PRECISION *a_data );
  int change_from_vector_to_component_interleave( int a_numComponents, size_t a_numEntries, MY_PRECISION *a_data );
  int change_to_3d_points( int a_numComponents, int a_numEntries, MY_PRECISION *a_data );
  int get_unstr_point_coords_in_set( SAF_Set a_set, MY_PRECISION *a_data, int a_timestep );
  int get_topo_rel_connectivity( SAF_Set a_set, SAF_Set a_pointSet, int a_numRels, SAF_Rel *a_rels, 
				 int *a_data, SAF_CellType a_cellType, int a_isQuadraticCellType );


  int get_num_var_fields_on_set_itself( SAF_Set a_set, SAF_Cat a_cat, int a_whichTimestep, int a_maxNum, 
					SAF_Field *a_fields, int *a_error );



  int get_num_var_fields_in_unstr_part( int a_whichPart, int a_whichTimestep, int a_maxNum, SAF_Field *a_fields );

  const char *get_algebraictype_desc( SAF_Algebraic a_type );
  const char *get_celltype_desc( SAF_CellType a_type );

  char *get_role_desc( SAF_Role a_role );




  int translate_topo_rel(SAF_Rel a_topoRel, MY_PRECISION *a_srcData, MY_PRECISION *a_destData);
  int get_unstr_point_coords_in_part( int a_whichPart, int a_whichTimestep, MY_PRECISION *a_data );

  int get_num_points_in_set( SAF_Set a_set, int a_timestep );
  int get_num_3d_coord_points_in_set( SAF_Set a_set, int a_timestep );
  void get_all_unstr_var_names( unsigned int a_maxlen );



  int is_part_arg_valid( int a_whichPart );
  int is_time_arg_valid( int a_whichPart );



#ifdef __cplusplus 
  /*
   * functions actually found in str_mesh_reader.c
   * These functions are simply wrappers to call the same function in the 
   * structured mesh reader, m_strReader.
   */
  int get_num_timesteps(void);
  int is_str_mesh_reader_initialized(void);
  int are_these_two_fields_from_the_same_var(SAF_Field *a_fieldA, SAF_Field *a_fieldB);
  const char * get_dsl_type_string( hid_t a_varType );
  int read_whole_field_to_my_precision( SAF_Field *a_field, hid_t a_varType, 
					size_t a_numEntries, MY_PRECISION *a_data );
  int get_matching_sets(  SAF_Set **a_subsets, SAF_TopMode a_topMode, int a_topoDim,
			  int a_numColl, int a_collDim, SAF_CellType a_cellType,
			  int a_mustHaveAllTheStructuredMeshCollections );
  int get_timestep_for_field( SAF_Field a_field );
  int is_this_field_on_this_timestep( SAF_Field a_field, int a_timestep );
  hid_t guess_dsl_var_type_for_composite_coord_field( int a_numFields, SAF_Field *a_fields );
#endif


 void get_list_of_supersets( SAF_Set a_set, SAF_Cat a_cat, int *a_numSupersets, int **a_supersetIndices );
 int get_all_unstr_fields_for_set_and_cat( int a_setIndex, SAF_Cat *a_selectedCat, int a_whichTimestep, 
					   int a_maxNum, SAF_Field *a_fields );



#ifdef __cplusplus 

  SAFStrMeshReader *m_strReader; 
  unstr_mesh_reader_t *g_unstrGlobals;
  VariableNames *m_variableNames;

}; //class SAFUnstrMeshReader 
# endif





#endif /*UNSTR_MESH_READER_H*/
