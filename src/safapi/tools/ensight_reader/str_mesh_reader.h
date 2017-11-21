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
 * Functions for reading structured mesh data from a SAF file
 *
 * IF COMPILING IN C:
 *
 * To use this code to read structured mesh data from a SAF file, the
 * first function that must be called is:
 *	init_str_mesh_reader(filename)
 * This function will read, sort and store information like number of
 * structured blocks and number of timesteps. 
 *
 * Note: if unstructured mesh data is also desired, the reader should
 * then call the function:
 *	init_unstr_mesh_reader()
 *
 * Next, the reader should call the appropriate functions in this 
 * header file (and/or in unstr_mesh_reader.h) to get the desired information.
 *
 * Once finished extracting data, if the reader has called init_unstr_mesh_reader,
 * then it must call:
 *	finalize_unstr_mesh_reader()
 *
 * Finally, the reader must call the function:
 *	finalize_str_mesh_reader()
 * This function frees memory allocated by these functions, closes the saf
 * file, and finalizes the saf library.
 *
 *
 *
 * IF COMPILING IN C++:
 *
 * Create an instance of the class SAFStrMeshReader, and follow the directions
 * above, noting that all of the functions are now methods of the class.
 * Another important note is that after creating an instance of the 
 * unstructured mesh reader (SAFUnstrMeshReader), you must link the two
 * classes using SAFUnstrMeshReader::setStrReader.
 *
 *
 * FOR MORE DETAILED INFORMATION, SEE THE SOURCE FILE.
 */

#ifndef STR_MESH_READER_H
#define STR_MESH_READER_H


/* Note that the extern "C" is needed because it is in the wrong place in saf.h
   (it should encompass the whole file, but does not) */
#ifdef __cplusplus
  extern "C" 
	{
#      include <saf.h> 
	}
#else
#      include <saf.h> 
#endif



/*The ensight shared library crashes on exiting ensight, because of the
unfreed saf stuff on saf_final (?). We can avoid some of that by not
calling saf functions before the library is actually needed. Note that
this will have to be undefined if/when there are actually multiple
saf versions in use  11nov2003*/
#define HARDWIRE_SAF_READER_VERSION_FOR_ENSIGHT




#if defined WIN32 || defined JANUS
int snprintf(char *buf, size_t count, const char *fmt, ... );
#endif



/*Must change these together: CURRENTLY SET TO FLOAT, FOR ENSIGHT'S SAKE.*/
  #define MY_PRECISION float
  #define MY_SAF_PRECISION H5T_NATIVE_FLOAT /*SAF_FLOAT*/


#ifndef __UNUSED__
#define __UNUSED__
#endif


/*
** Added this temporary replacement for strdup 02/26/03 because strdup 
** doesnt seem to work on sass3276
*/
extern char *ens_strdup(const char *a_str);


/* Note: before 052303, this was in start,count,skip,start,count,skip.... order,
   by mistake. SAF requires it to be in start,start,start,count,count ...
*/
typedef struct {
  int x_start; int y_start; int z_start;
  int x_count; int y_count; int z_count; 
  int x_skip; int y_skip; int z_skip;
} struct_hyperslab;

typedef enum 
{
  SAF_STR_MESH_NODE,
  SAF_STR_MESH_XEDGE,
  SAF_STR_MESH_YEDGE,
  SAF_STR_MESH_ZEDGE,
  SAF_STR_MESH_XFACE,
  SAF_STR_MESH_YFACE,
  SAF_STR_MESH_ZFACE,
  SAF_STR_MESH_ELEM,
  SAF_STR_MESH_UNKNOWN
} saf_str_mesh_var_type;



#ifdef __cplusplus 

class str_mesh_reader_t; //forward declaration
class VariableNames; //forward declaration

class SAFStrMeshReader 
{
 public:

  //constructor
  SAFStrMeshReader();
  
  ~SAFStrMeshReader();
  
# endif


  /*
   * Returns 1 if the file is a saf database, 0 otherwise
   */
  int is_file_a_saf_database( const char *a_safFilename );


  /* 
   * init_str_mesh_reader must be called before any other reader function
   * except for is_file_a_saf_database or is_str_mesh_reader_initialized
   */
  int init_str_mesh_reader( const char *a_safFilename );

  /* 
   * finalize_str_mesh_reader must be called after all other reader functions
   * Set the argument to zero, to NOT call saf_final, if the app will be
   *  opening other saf files.
   */ 
  int finalize_str_mesh_reader( int a_finalizeSAF  );

  /*
   * Return 1 if init_str_mesh_reader has already been called successfully,
   * 0 otherwise.
   */
  int is_str_mesh_reader_initialized(void);

  /* printinfo replaces printf in order to give the reader the option
   * to write to a file instead of stdout. 
   */
  void printinfo( const char *fmt, ... );

  /* Set the file name to print to. If name is NULL, then print to stdout*/
  void printinfo_set_filename( const char *name );

  /*
   * Return the number of available timesteps. The same timesteps are used for
   * structured and unstructured (unstr_mesh_reader.h) data.
   */
  int get_num_timesteps(void);

  /*
   * Returns the time value for the given timestep.
   */
  MY_PRECISION get_time_value_for_timestep(int a_which);

  /*
   * Get the unit name for the given variable, e.g. Kelvin, Celsius...
   */
  char *get_str_var_unit_name( int a_which );

  /*
   * Get the quantity name for the given variable, e.g. Temperature, Frequency ...
   */
  char *get_str_var_quant_name( int a_which );

  /*
   * Return the number of available structured mesh blocks.
   */
  int get_num_blocks( void );

  /*
   * Return the number of variables that can be mapped to the blocks.
   */
  int get_num_str_vars(void);




  /* This function reads block coordinate data as if it were separable. If the 
  ** data is not actually separable (i.e. a 3d array of size X*Y*Z that can be
  ** represented by 3 1d arrays of size X, Y and Z), then the results will
  ** not make sense. This function is especially for the IDL reader used by
  ** EMPHASIS.
  */
  int read_block_coords_for_selected_variable_as_separable( int a_whichBlock,
							    int a_whichVar,
							    int a_whichTimestep,
							    MY_PRECISION *a_x, MY_PRECISION *a_y, MY_PRECISION *a_z );


  /*
  ** Returns 1 if the block has separable (i.e. a block of size X*Y*Z is described 
  ** by 3 1d arrays of size X, Y and Z) coordinates.  
  */
  int is_block_separable( int a_whichBlock );

  /*
  ** Read coords on the specified block, and assume they are curvilinear, whether they are
  ** stored as curvilinear or not. If is_block_separable returned 0, then this
  ** function should definitely be used to read the block. If is_block_separable returned
  ** 1, then this function can still be used, but will waste a lot of space.
  ** If an argument array pointer is NULL, then it will be ignored. Otherwise is
  ** must be pre-allocated to length X*Y*Z, where X Y and Z are the lengths returned
  ** by get_block_size().
  */
  int read_block_coords_as_curvilinear( int a_whichBlock, MY_PRECISION *a_xCoords, 
					MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );


  /*
  ** Read coords on the specified block, and assume they are separable, whether they are
  ** stored as separable or not. If is_block_separable returned 0, then this
  ** function might return garbage coordinates, though it will not fail. If 
  ** is_block_separable returned 1, then this function should be used.
  ** If an argument array pointer is NULL, then it will be ignored. Otherwise is
  ** must be pre-allocated to length X Y or Z, where X Y and Z are the lengths returned
  ** by get_block_size().
  */
  int read_block_coords_as_separable( int a_whichBlock, MY_PRECISION *a_xCoords, 
				      MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );



  /*
  ** Put the x,y, and z dimensions of the block in *a_xSize, *a_ySize and *a_zSize
  */
  void get_block_size( int a_whichBlock, int *a_xSize, int *a_ySize, int *a_zSize );


  /*
   * Get the x,y z size of the array that will be read when reading a given
   * block, variable, timestep combo IF USING read_simple_variable (important!!!). 
   * Note that if using read_variable, the size of the array will be either
   * the number of hex elements in the block for a SAF_STR_MESH_ELEM var, or 
   * the number of nodes for any other type of var.
   *
   * This function is especially for the IDL reader used by EMPHASIS.
   */
  int get_variable_size( int a_whichBlock,
			 int a_whichVar,
			 int a_whichTimestep, int *a_x, int *a_y, int *a_z );


  /*
   * Get the name of the block (the name of the containing SAF Set). 
   * a_str must be already allocated to length a_maxlen
   */
  int get_block_name( int a_which, char *a_str, int a_maxlen );

  /*
   * Get the name of the reader (composed of the SAF version number and the HDF5
   * version number).
   * a_str must be already allocated to length a_maxlen
   */
  void get_reader_name( char *a_str, unsigned int a_maxlen );


  /* 
  ** Read a block-var-timestep combination of a scalar variable. a_data must already
  ** be allocated
  **
  ** If the variable type is SAF_STR_MESH_ELEM, then one entry for each
  ** hex elem is read. Otherwise, one entry for each node is read.
  */
  int read_variable( int a_whichBlock,
		     int a_whichVar,
		     int a_whichTimestep,
		     float *a_data );

  /*
  ** read a variable simply by reading the field: dont worry about
  ** translating the data to fit a block, subset, or whatever
  ** This function is especially for the IDL reader used by
  ** EMPHASIS.
  */
  int read_simple_variable( int a_whichBlock,
			    int a_whichVar,
			    int a_whichTimestep,
			    MY_PRECISION *a_data );


  /*
  ** Return the version of this reader. This is currently meaningless.
  ** a_buffer must be already allocated to size a_size
  */
  int get_str_mesh_reader_version( char *a_buffer, size_t a_size );


  /*
  ** Get attributes of this block and print them out as strings.
  ** a_str must be already allocated to size a_maxlen
  */
  int get_block_attributes_as_string( int a_whichBlock, char *a_str, size_t a_maxlen );


  /*
  ** Get the name of the variable. The name is based on the names of the fields
  ** that are part of the variable, and the quantity and unit names.
  ** a_str must be already allocated to size a_maxlen
  */
  int get_str_var_name( int a_whichVar, char *a_str, unsigned int a_maxlen );


  /*
  ** Get the name of the variable. The name is simply the name of the first
  ** field found with that variable and timestep. This function is especially 
  ** for the IDL reader used by EMPHASIS.
  ** a_str must be already allocated to size a_maxlen
  */
  int get_field_name_for_var( int a_whichVar, int a_whichTimestep, char *a_str, size_t a_maxlen );


  /*
  ** return 1 if a variable does exist on a given block and timestep
  */
  int does_var_exist_on_block( int a_whichBlock, int a_whichVar, int a_whichTimestep );

  /*
  ** return a corresponding string for a variable type
  */
  const char *get_var_type_name( saf_str_mesh_var_type a_which );

  /*
  ** return the variable type for a given block, variable, and timestep
  */
  saf_str_mesh_var_type get_variable_type( int a_whichBlock, int a_whichVar, int a_whichTimestep );


  /*
  ** returns 1 if a_left and a_right are equal, or if they are both edges, or if they
  ** are both faces
  */
  int are_similar_str_mesh_var_types( saf_str_mesh_var_type a_left, saf_str_mesh_var_type a_right );









  /**********************************************************************************
   * The following functions use saf objects, and should not be used by any new readers.
   *
   */

  /*
  ** Count the number of variable fields on a set. The set should have been created
  ** by add_block, or any of the create_*set functions defined here. The variable
  ** should have been added to the set with write_scalar_variable or
  ** write_scalar_variable_using_existing_template. 
  **
  ** A "variable field" is defined here as a non-trivial non-coordinate field.
  */
  int get_num_variable_fields_on_set( SAF_Set a_set );




  /*
  ** Return the number of blocks. If the argument a_blocks is not NULL, then
  ** allocate a_blocks[0] as a 1d array containing each block.
  **
  ** SAF Definition: Find every set that is:
  ** a top set,
  ** a SilRole of SAF_SPACE,
  ** topological dim 3,
  ** has the 8 structured collections on it (nodes,xedges,yedges,zedges,xfaces,yfaces,zfaces,elems)
  ** AND has the token SAF_STRUCTURED topo relation
  */
  int get_num_block_sets( SAF_Set **a_blocks );


  /*
  ** Return the number of unstructured node sets. If the argument a_unstrNodeSets 
  ** is not NULL, then allocate a_unstrNodeSets[0] as a 1d array containing each set.
  **
  ** SAF Definition: Find every set that is:
  ** a top set,
  ** a SilRole of SAF_SPACE,
  ** topological dim 0,
  ** has only 1 collection on it of:
  **    IndexSpec dim 1,
  **    SAF_CellType SAF_CELLTYPE_POINT
  **
  */
  int get_num_unstructured_node_sets( SAF_Set **a_unstrNodeSets );

  /*
  ** Return the number of node subsets. If the argument a_subsets is not NULL, then
  ** allocate a_subsets[0] as a 1d array containing each set.
  **
  ** SAF Definition: Find every set that is:
  ** not a top set,
  ** a SilRole of SAF_SPACE,
  ** topological dim 3,
  ** has 8 collections on it (nodes,xedges,yedges,zedges,xfaces,yfaces,zfaces,elems)
  **     (maybe in the future will check that each collection is correct. for now, no)
  **     NOTE: because the node subsets have all 8 collections on them, just like
  **     the blocks themselves, one could create subset elems that span many of the
  **     elems in the top level block!!
  */
  int get_num_node_subsets( SAF_Set **a_subsets );


  /*
  ** Return the number of edge subsets. If the argument a_subsets is not NULL, then
  ** allocate a_subsets[0] as a 1d array containing each set.
  **
  ** SAF Definition: Find every set that is:
  ** not a top set,
  ** a SilRole of SAF_SPACE,
  ** topological dim 1,
  ** has only 1 collection on it of:
  **    IndexSpec dim 3,
  **    SAF_CellType SAF_CELLTYPE_LINE
  **
  */
  int get_num_edge_subsets(  SAF_Set **a_subsets  );


  /*
  ** Return the number of face subsets. If the argument a_subsets is not NULL, then
  ** allocate a_subsets[0] as a 1d array containing each set.
  **
  ** SAF Definition: Find every set that is:
  ** not a top set,
  ** a SilRole of SAF_SPACE,
  ** topological dim 2,
  ** has only 1 collection on it of:
  **    IndexSpec dim 3,
  **    SAF_CellType SAF_CELLTYPE_QUAD
  **
  */
  int get_num_face_subsets(  SAF_Set **a_subsets  );


  /*
  ** Return the number of elem subsets. If the argument a_subsets is not NULL, then
  ** allocate a_subsets[0] as a 1d array containing each set.
  **
  ** SAF Definition: Find every set that is:
  ** not a top set,
  ** a SilRole of SAF_SPACE,
  ** topological dim 3,
  ** has only 1 collection on it of:
  **    IndexSpec dim 3,
  **    SAF_CellType SAF_CELLTYPE_HEX
  **
  */
  int get_num_elem_subsets(  SAF_Set **a_subsets  );



  /*
   * Get the 8 collections that define a structured mesh block
   */
  int get_str_mesh_collections( SAF_Set a_set,SAF_Cat **a_nodes, SAF_Cat **a_xEdges, 
				SAF_Cat **a_yEdges, SAF_Cat **a_zEdges, SAF_Cat **a_xFaces, 
				SAF_Cat **a_yFaces, SAF_Cat **a_zFaces, SAF_Cat **a_elems );







  /**************************************************************************
   * INTERNAL - these functions should not be used by a reader, but they are used by other
   * functions in this directory.
   */

  int get_matching_sets(  SAF_Set **a_subsets, SAF_TopMode a_topMode, int a_topoDim,
			  int a_numColl, int a_collDim, SAF_CellType a_cellType,
			  int a_mustHaveAllTheStructuredMeshCollections );
  int get_timestep_for_field( SAF_Field a_field );
  int is_this_field_on_this_timestep( SAF_Field a_field, int a_timestep );

  hid_t guess_dsl_var_type_for_composite_coord_field( int a_numFields, SAF_Field *a_fields );


  const char *get_dsl_type_string( hid_t a_varType );



  int read_whole_field_to_my_precision( SAF_Field *a_field, hid_t a_varType, 
					size_t a_numEntries, MY_PRECISION *a_data );


  int are_these_two_fields_from_the_same_var(SAF_Field *a_fieldA, SAF_Field *a_fieldB);




  SAF_Db *getDatabase(void);
  int getNumStrSets(void);
  int getNumStrSubsets(void);
  int getNumStrEdgeSubsets(void);
  int getNumStrFaceSubsets(void);
  int getNumStrElemSubsets(void);
  SAF_Set *getStrSets(void);
  SAF_Set *getStrSubsets(void);
  SAF_Set *getStrEdgeSubsets(void);
  SAF_Set *getStrFaceSubsets(void);
  SAF_Set *getStrElemSubsets(void);

  int getMaxNumStrVars(void);




  void initialize_str_globals(void);
  int are_str_mesh_reader_globals_initialized(void);
  void get_block_size_from_set( SAF_Set *a_set, int *a_xSize, int *a_ySize, int *a_zSize );
  int get_total_num_var_instances(void);
  int get_num_var_instances_on_full_blocks( int **a_timesteps, int **a_blockNumbers  );
  int is_field_below_set( int a_whichField, SAF_Set *a_set );
  int is_field_on_set( int a_whichField, SAF_Set *a_set );
  int which_topset_is_field_from( int a_which );
  int get_num_var_instances_on_subsets_of_set( SAF_Set *a_set, SAF_Cat *a_coll, int a_numSubsets, SAF_Set *a_subsets  );
  int get_subset_hyperslab( SAF_Set *a_superSet, SAF_Set *a_subSet, SAF_Cat *a_coll, struct_hyperslab *a_slab );
  void get_timesteps_for_all_indirect_fields( void );
  int read_node_variable( int a_whichBlock,int a_whichVar,int a_whichTimestep,float *a_data );
  int read_edge_or_face_variable_as_node_variable( int a_whichBlock,int a_whichVar, int a_whichTimestep,MY_PRECISION *a_data );
  int read_set_coords_as_separable( SAF_Set a_set, MY_PRECISION *a_xCoords,
				    MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );
  int read_separable_3d_coords( SAF_Set a_set, MY_PRECISION *a_xCoords,
				MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );
  int read_curvilinear_3d_coords( SAF_Set a_set, MY_PRECISION *a_xCoords, 
				  MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );
  int does_block_contain_3d_curvilinear_coords( SAF_Set a_set );
  saf_str_mesh_var_type get_variable_type_from_field( SAF_Set a_set, SAF_Field a_field );
  void checkBufferSize( int a_size );
  void freeBuffer( void );
  void change_ordering_scheme( SAF_IndexSpec a_oldIspec, SAF_IndexSpec a_newIspec,
			       int a_bytesPerType, void *a_data );
  int fill_data_from_hyperslab(int a_x, int a_y, int a_z, struct_hyperslab a_slab,
			       MY_PRECISION *a_from, MY_PRECISION *a_to );
  int is_field_below_nontopset_cached( int a_whichField, int a_whichNonTopset );
  int is_field_below_topset_cached( int a_whichField, int a_whichTopset );
  int add_fields_not_in_suites_to_list( void );
  int read_all_suites( void );
  int get_all_str_var_names( unsigned int a_maxlen );
  int get_variables_on_set( SAF_Set a_set, SAF_Field *a_fields );
  int does_set_have_the_8_structured_collections( SAF_Set a_set );
  int read_block_coords_as_curvilinear_from_set( SAF_Set a_set, MY_PRECISION *a_xCoords,
						 MY_PRECISION *a_yCoords, MY_PRECISION *a_zCoords );




  void look_for_registry_in_ensight_tree( void );


#ifdef __cplusplus 
 private:

  str_mesh_reader_t *g_strGlobals;
  VariableNames *m_variableNames;

  int m_whichInstance;
  static int m_numInstances;

}; //class SAFStrMeshReader 
# endif



  hbool_t my_saf_is_valid_field_handle(SAF_Field *a_field);
  hbool_t my_saf_is_valid_ftmpl_handle(SAF_FieldTmpl *a_ftmpl);
  hbool_t my_saf_is_valid_set_handle(SAF_Set *a_set);


#endif /*STR_MESH_READER_H*/
