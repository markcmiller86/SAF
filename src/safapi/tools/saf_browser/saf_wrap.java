import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.border.*;
import javax.swing.colorchooser.*;
import javax.swing.filechooser.*;
import javax.accessibility.*;

import java.lang.reflect.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.*;
import java.io.*;
import java.applet.*;
import java.net.*;



class saf_wrap {

  public native int native_open(String saf_filepath, String saf_filename, String mode);

  public native void native_close(int file_id);

  public native int native_num_tops();

  public native int native_num_suites();

  public native String native_get_set_name(int file_id);

  public native String native_get_set_silrole( int set_id );

  public native String native_get_set_extmode( int set_id );

  public native String native_get_set_topmode( int set_id );

  public native int native_get_set_max_topo_dim( int set_id );

  public native boolean native_is_set_equiv(int set1, int set2);

  public native int native_get_all_sets_id();

  public native int native_get_suite_sets_id();

  public native String native_get_coll_cat_name(int set_id, int coll_id);

  public native String native_get_cat_name( int cat_id);

  public native int native_get_coll_count( int set_id, int coll_id );

  public native String native_get_coll_celltype( int set_id, int coll_id );

  public native boolean native_get_coll_is_decomp( int set_id, int coll_id );

  public native String native_get_category_name( int cat_id );

  public native int native_lookup_coll_num(int set_id, String cat_name);

  public native int native_get_coll_id( int set_id, int coll_num );

  public native int native_get_subsets(int set_id, int coll_id);

  public native String native_get_subset_relation_info(int rel_id);

  public native int native_range_set(int rel_id);

  public native int native_range_coll(int rel_id);

  public native int native_get_topos(int set_id, int coll_id);
 
  public native int native_get_topo_relation(int set_id, int coll_id);
  
  public native int native_get_topo_domain_id(int set_id, int coll_id);

  public native int native_get_subset_relation(int set_id, int coll_id);

  public native int native_get_topo_rel_size(int rel_id, int a_or_b);

  public native String native_get_topo_rel_type( int rel_id );
 
  public native int native_get_topo_rel_data(int rel_id, int i, int a_or_b);
 
  public native int native_get_subset_rel_size(int rel_id);

  public native String native_get_subset_rel_type( int rel_id);

  public native int native_get_subset_rel_data(int rel_id, int i);

  public native float native_get_field_data( int field_id, int i );

  public native int native_get_field_data_int( int field_id, int i );

  public native int native_remap_field( int field_id , int dest_field_id );

  public native int native_get_indfield_data( int field_id, int i, int clear );

  public native int native_get_state_data( int field_id, int state_num, int i );

  public native int native_number_of_sets( int set_id );

  public native String native_coll_info( int set_id, int coll_id );

  public native String native_field_info( int set_id, int coll_id );

  public native int native_get_num_fields_on_coll( int set_id, int coll_id );

  public native int native_get_field_rel_id( int set_id, int coll_id, int field_num);

  public native String native_get_field_menu_string( int set_id, int field_num );

  public native String native_get_field_name( int field_id );

  public native String native_get_field_data_type( int field_id );
 
  public native String native_get_field_interleave( int field_id );
 
  public native int native_get_field_data_size( int field_id );

  public native boolean native_get_field_is_coord( int field_id ); 
 
  public native int native_get_field_coeff_assoc_id( int field_id );

  public native String native_attribute_string( int handle );

  public native String native_get_attr_name( int handle, int num );

  public native String native_get_attr_data( int handle, String name, int num);

  public native String native_get_attr_type( int handle, String name );

  public native int native_attribute_count( int handle );

  public native int native_sub_attribute_count( int handle, String name );

  public native boolean native_is_attribute_primitive( int handle, String name );

  public native int native_get_field_assoc_ratio( int field_id );
 
  public native int native_get_field_storage_decomp_id( int field_id );

  public native int native_get_field_eval_coll_id( int field_id );

  public native int native_get_field_num_comps( int field_id );

  public native int native_get_field_component( int field_id, int i);

  public native String native_get_field_eval_func( int field_id );

//
//
  public native int native_get_field_id( int set_id, int field_num );

  public native int native_get_fieldtmpl_id( int field_id );

  public native String native_get_fieldtmpl_name( int fieldtmpl_id );

  public native int native_get_field_base_space_id( int field_id );

  public native String native_get_field_base_space_name( int field_id );
  
  public native int native_get_fieldtmpl_num_comp( int fieldtmpl_id );

  public native String native_get_fieldtmpl_basis( int fieldtmpl_id );

  public native String native_get_fieldtmpl_atype( int fieldtmpl_id );

  public native int native_get_fieldtmpl_component( int fieldtmpl_id, int i );

//
//

  public native int native_get_row_id( int object_id );

  public native int native_get_state_num_states( int state_id );
 
  public native String native_get_state_name( int state_id );

  public native int native_get_state_field_on_state( int state_id, int num );

  public native int native_get_a_set( int set_id , int set_num );

  public native int native_get_num_colls(int set_id);

  public native int native_find_set_field_tmpls(int set_id);

  public native String native_tree();


  // static {
    // System.loadLibrary("safgui"); }


  public static int open(String saf_filepath, String saf_filename,String mode) {
    int result;
    result = new saf_wrap().native_open(saf_filepath, saf_filename,mode);
    return(result); }


  public static void close(int file_id) {
    new saf_wrap().native_close(file_id); }


  public static int num_tops() {
    int result;
    result = new saf_wrap().native_num_tops();
    return(result);
   }

  public static int num_suites() {
    int result;
    result = new saf_wrap().native_num_suites();
    return(result);
   }

  public static int get_all_sets_id() {

	int result;

	result = new saf_wrap().native_get_all_sets_id();
	return(result);
  }

  public static int get_suite_sets_id() {

	int result;

	result = new saf_wrap().native_get_suite_sets_id();
	return(result);
  }


  public static int number_of_sets( int set_id )
  {
	int result;

	result = new saf_wrap().native_number_of_sets( set_id );
	return(result);

  }

  public static String coll_info( int set_id, int coll_id )
  {
	String result;

	result = new saf_wrap().native_coll_info( set_id, coll_id );
	return(result);

  }

  public static String field_info( int set_id, int coll_id )
  {
	String result;

	result = new saf_wrap().native_field_info(set_id, coll_id);
	return(result);
  }

    public static int attribute_count( int handle )
    {
    int result;

    result = new saf_wrap().native_attribute_count( handle );
    return(result);

    }

    public static boolean is_attribute_primitive( int handle, String name )
    {

    boolean result;

    result = new saf_wrap().native_is_attribute_primitive( handle, name );
    return result;
    }

    public static String get_attr_name( int handle, int num )
    {
    String result;

    result = new saf_wrap().native_get_attr_name( handle, num );
    return(result);
    }

    public static String get_attr_type( int handle, String name )
    {
    String result;

    result = new saf_wrap().native_get_attr_type( handle, name );
    return(result);
    }

    public static String get_attr_data( int handle, String name, int num )
    {
    String result;

    result = new saf_wrap().native_get_attr_data( handle, name, num );
    return(result);
    }


    public static int  sub_attribute_count( int handle, String name )
    {
        int result;

    result = new saf_wrap().native_sub_attribute_count( handle, name );
    return(result);
    }


  public static String attribute_string( int handle )
  {
	String result;

	result = new saf_wrap().native_attribute_string( handle );
	return(result);
  }

  public static int get_num_fields_on_coll( int set_id, int coll_id )
  {
	int result;

        result = new saf_wrap().native_get_num_fields_on_coll(set_id, coll_id);
	return(result);

  }

  public static int get_field_rel_id( int set_id, int coll_id, int field_num )
  {
	int result;

        result = new saf_wrap().native_get_field_rel_id(set_id, coll_id, field_num);
	return(result);

  }

  public static int get_field_id( int set_id, int field_num )
  {
	int result;

	result = new saf_wrap().native_get_field_id( set_id, field_num );
	return(result);

  }

  public static String get_field_menu_string( int set_id, int field_num )
  {
	String result;

        result = new saf_wrap().native_get_field_menu_string(set_id, field_num);
	return(result);

  }

  public static String get_field_name(  int field_id )
  {
  
	String result;

        result = new saf_wrap().native_get_field_name( field_id );
	return(result);

  }
  public static String get_field_data_type(  int field_id )
  {
  
	String result;

        result = new saf_wrap().native_get_field_data_type( field_id );
	return(result);

  }


  public static String get_field_interleave(  int field_id )
  {
  
	String result;

        result = new saf_wrap().native_get_field_interleave( field_id );
	return(result);

  }
  public static int get_field_data_size(  int field_id )
  {
  
	int result;

        result = new saf_wrap().native_get_field_data_size( field_id );
	return(result);

  }
  public static boolean get_field_is_coord(  int field_id )
  {
  
	boolean result;

        result = new saf_wrap().native_get_field_is_coord( field_id );
	return(result);

  }
  public static int get_field_coeff_assoc_id(  int field_id )
  {
  
	int result;

        result = new saf_wrap().native_get_field_coeff_assoc_id( field_id );
	return(result);

  }
  public static int get_field_assoc_ratio(  int field_id )
  {
  
	int result;

        result = new saf_wrap().native_get_field_assoc_ratio( field_id );
	return(result);
  }
  public static int get_field_storage_decomp_id(  int field_id )
  {
  
	int result;

        result = new saf_wrap().native_get_field_storage_decomp_id( field_id );
	return(result);

  }
  public static int get_field_eval_coll_id(  int field_id )
  {
  
	int result;

        result = new saf_wrap().native_get_field_eval_coll_id( field_id );
	return(result);

  }
  public static int get_field_num_comps(  int field_id )
  {
  
	int result;

        result = new saf_wrap().native_get_field_num_comps( field_id );
	return(result);

  }
  public static int get_field_component(  int field_id, int i )
  {
  
	int result;

        result = new saf_wrap().native_get_field_component( field_id, i );
	return(result);

  }
  public static String get_field_eval_func(  int field_id )
  {
  
	String result;

        result = new saf_wrap().native_get_field_eval_func( field_id );
	return(result);

  }

  

// Field Template Wrappers

  public static int get_fieldtmpl_id( int field_id )
  {
	int result;

        result = new saf_wrap().native_get_fieldtmpl_id( field_id );
	return(result);
  }

  public static String get_fieldtmpl_name( int fieldtmpl_id )
  {
	String result;

        result = new saf_wrap().native_get_fieldtmpl_name( fieldtmpl_id );
	return(result);
  }

  public static int get_field_base_space_id( int field_id )
  {
	int result;

        result = new saf_wrap().native_get_field_base_space_id( field_id );
	return(result);
  }

  public static String get_field_base_space_name( int field_id )
  {
	String result;

        result = new saf_wrap().native_get_field_base_space_name( field_id );

	return(result);
  }

  public static int get_fieldtmpl_num_comp( int fieldtmpl_id )
  {
	int result;

        result = new saf_wrap().native_get_fieldtmpl_num_comp(fieldtmpl_id );
	return(result);
  }

  public static String get_fieldtmpl_basis( int fieldtmpl_id )
  {
	String result;

        result = new saf_wrap().native_get_fieldtmpl_basis(fieldtmpl_id );
	return(result);
  }

  public static String get_fieldtmpl_atype( int fieldtmpl_id )
  {
	String result;

        result = new saf_wrap().native_get_fieldtmpl_atype(fieldtmpl_id );
	return(result);
  }

  public static int get_fieldtmpl_component( int fieldtmpl_id , int i )
  {
	int result;

        result = new saf_wrap().native_get_fieldtmpl_component(fieldtmpl_id, i );
	return(result);
  }

  public static String get_state_name( int state_id )
  {

	String result;

	result = new saf_wrap().native_get_state_name( state_id );
	return(result);

  }

  public static int get_state_num_states( int state_id )
  {

	int result;

	result = new saf_wrap().native_get_state_num_states( state_id );
	return(result);

  }
  
  public static int get_state_field_on_state( int state_id, int num )
  {

	int result;

	result = new saf_wrap().native_get_state_field_on_state( state_id, num );
	return(result);

  }

// end Field Template Wrappers

  public static int get_a_set( int set_id, int set_num ) 
  {
	int result;

	result = new saf_wrap().native_get_a_set( set_id, set_num );
	return(result);

  }


  public static int get_num_colls(int set_id) {
    int result;
    result = new saf_wrap().native_get_num_colls(set_id);
    return(result);
   }


  public static String get_set_name(int file_id) {
    String result;
    result = new saf_wrap().native_get_set_name(file_id);
    return(result);
   }

  public static String get_set_silrole(int file_id) {
    String result;
    result = new saf_wrap().native_get_set_silrole(file_id);
    return(result);
   }

  public static String get_set_extmode(int file_id) {
    String result;
    result = new saf_wrap().native_get_set_extmode(file_id);
    return(result);
   }

  public static int get_set_max_topo_dim(int file_id) {
    int result;
    result = new saf_wrap().native_get_set_max_topo_dim(file_id);
    return(result);
   }

  public static boolean is_set_equiv(int set1, int set2) {
    boolean result;
    result = new saf_wrap().native_is_set_equiv(set1, set2);
    return(result);
   }


  public static String get_coll_cat_name(int set_id, int coll_id) {
    String result;
    result = new saf_wrap().native_get_coll_cat_name(set_id, coll_id);
    return(result);
   }

  public static String get_cat_name( int cat_id) {
    String result;
    result = new saf_wrap().native_get_cat_name( cat_id);
    return(result);
   }

  public static String get_coll_celltype(int set_id, int coll_id) {
    String result;
    result = new saf_wrap().native_get_coll_celltype(set_id, coll_id);
    return(result);
   }

  public static int get_coll_count(int set_id, int coll_id) {
    int result;
    result = new saf_wrap().native_get_coll_count(set_id, coll_id);
    return(result);
   }

  public static boolean get_coll_is_decomp(int set_id, int coll_id) {
    boolean result;
    result = new saf_wrap().native_get_coll_is_decomp(set_id, coll_id);
    return(result);
   }

  public static String get_category_name(int cat_id) {
    String result;
    result = new saf_wrap().native_get_category_name(cat_id);
    return(result);
   }


  public static int lookup_coll_num( int set_id, String cat_name)
  {
	int result;

	result = new saf_wrap().native_lookup_coll_num( set_id, cat_name );
	return(result);

  }

  public static int get_coll_id( int set_id, int coll_num)
  {
	int result;

	result = new saf_wrap().native_get_coll_id( set_id, coll_num );
	return(result);

  }


  public static int get_subsets( int set_id, int coll_id)
  {
	int result;

	result = new saf_wrap().native_get_subsets(set_id, coll_id);
	return(result);

  }

  public static String get_subset_relation_info( int rel_id )
  {
	String result;

	result  = new saf_wrap().native_get_subset_relation_info( rel_id );
	return(result);

  }


  public static int get_subset_relation_size( int rel_id )
  {

	int result;

	result = new saf_wrap().native_get_subset_rel_size( rel_id );
	return(result);
   
  }

  public static String get_subset_relation_type( int rel_id )
  {

    String result;

    result = new saf_wrap().native_get_subset_rel_type( rel_id );
    return(result);

  }


  public static int get_subset_relation_data( int rel_id, int i)
  {
	int result;

	result = new saf_wrap().native_get_subset_rel_data(rel_id, i);
	return(result);

  }

  public static float get_field_data( int field_id, int i)
  {
	float result;

	result = new saf_wrap().native_get_field_data(field_id, i);
	return(result);

  }

  public static int get_field_data_int( int field_id, int i)
  {
	int result;

	result = new saf_wrap().native_get_field_data_int(field_id, i);
	return(result);

  }

  public static int remap_field( int field_id, int dest_field_id )
   {

     int result;

     result = new saf_wrap().native_remap_field( field_id, dest_field_id );
     return(result);
   }

  public static int get_indfield_data( int field_id, int i, int clear)
  {
	int result;

	result = new saf_wrap().native_get_indfield_data(field_id, i, clear);
	return(result);

  }


  public static int get_state_data( int field_id, int state_num, int i)
  {
	int result;

	result = new saf_wrap().native_get_state_data(field_id, state_num, i);
	return(result);

  }

  public static int get_topo_relation_size( int rel_id, int a_or_b )
  {

	int result;

	result = new saf_wrap().native_get_topo_rel_size( rel_id, a_or_b );
	return(result);
   
  }

  public static String get_topo_relation_type( int rel_id )
  {

	String result;

	result = new saf_wrap().native_get_topo_rel_type( rel_id );
	return(result);
   
  }


  public static int get_topo_relation_data( int rel_id, int i, int a_or_b)
  {
	int result;

	result = new saf_wrap().native_get_topo_rel_data(rel_id, i, a_or_b);
	return(result);

  }

  public static int get_topos( int set_id, int coll_id)
  {
	int result;

	result = new saf_wrap().native_get_topos(set_id, coll_id);
	return(result);

  }


  public static int get_topo_relation( int set_id, int coll_id )
  {
	int result;
	
	result = new saf_wrap().native_get_topo_relation(set_id, coll_id);
	return(result);

  }

  public static int get_topo_domain_id( int set_id, int coll_id )
  {
	int result;
	
	result = new saf_wrap().native_get_topo_domain_id(set_id, coll_id);
	return(result);

  }


  public static int range_set( int rel_id)
  {

	int result;

	result = new saf_wrap().native_range_set( rel_id );
	return(result);
  }

  public static int range_coll( int rel_id)
  {

	int result;

	result = new saf_wrap().native_range_coll( rel_id );
	return(result);
  }


  public static int get_topo_subsets(int set_id, int coll_id)
  {
	int result;

	// result = new saf_wrap().native_get_subsets(set_id, coll_id);
	result = set_id + coll_id;
	return(result);
  }

  public static int get_subset_relation(int set_id, int coll_id ) 
  {

	int result;

	result = new saf_wrap().native_get_subset_relation(set_id, coll_id);
	return(result);

  }

  public static int get_row_id( int object_id )
  {

    int result;

    result = new saf_wrap().native_get_row_id( object_id );
    return(result);

  }

  public static int find_set_field_tmpls(int set_id)
  {

	int result;

	result = new saf_wrap().native_find_set_field_tmpls(set_id);
	return(result);

  }


  public static String saftree() {

        String result;
        result = new saf_wrap().native_tree();
        return(result);
        }

}


