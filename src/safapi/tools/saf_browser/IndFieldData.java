import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import javax.swing.text.*;
import javax.swing.table.*;
import javax.accessibility.*;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.*;
import java.io.*;
import java.applet.*;
import java.net.*;


class IndFieldData extends JPanel {

	Float float_value;
	Integer int_value;

	public IndFieldData( int set_id, int coll_id,  int field_id ) {

		super();

		int i,j, rownumber, rowheadernum, colnumber, coeff_assoc_id, assoc_ratio, num_comps;
		String coeff_assoc;
		int data_size;
		int coll_count;
		int fieldtmpl_id;
		int component_id;
		JButton rowButton;
		String data_string="";
		String field_name, rowheader;
		Object[] column_headers=new Object[2];



		// String data_type = saf_wrap.get_field_data_type( field_id );
		String data_type;

		fieldtmpl_id = saf_wrap.get_fieldtmpl_id( field_id );
		String atype = saf_wrap.get_fieldtmpl_atype( fieldtmpl_id  ); 
		String set_name = saf_wrap.get_set_name( set_id );
		coll_count = saf_wrap.get_coll_count( set_id, coll_id );
		field_name = saf_wrap.get_field_name( field_id );
		assoc_ratio = saf_wrap.get_field_assoc_ratio( field_id );
		num_comps = saf_wrap.get_field_num_comps( field_id );
		String interleave = saf_wrap.get_field_interleave( field_id );


		int subsets;
		int num_subsets;
		int a_set;
		int rel_id;
		int lookup_count = saf_wrap.get_coll_count(set_id , coll_id);

		int[][] field_lookup_table = new int[lookup_count+1][3];

		int num_fields;
		int a_field_id = -1;
		int a_field_num;
		int subset_value;
		int sub_coll_count = 1;
		subsets = saf_wrap.get_subsets(set_id, coll_id);
		if( subsets != 0 ) {

			num_subsets = saf_wrap.number_of_sets(subsets);
			int[] subset_rel_size = new int[num_subsets];
			int[] field_ids = new int[num_subsets];
			int a_set_coll_num;
			String a_set_name;
			String a_coll_name;

			for( i = 0 ; i < num_subsets ; i++ ) {
				a_set = saf_wrap.get_a_set(subsets,i);	
				a_set_coll_num = saf_wrap.lookup_coll_num(a_set,saf_wrap.get_coll_cat_name(set_id,coll_id));
				rel_id = saf_wrap.get_subset_relation(a_set, a_set_coll_num);
				subset_rel_size[i] = saf_wrap.get_subset_relation_size( rel_id );

				num_fields = saf_wrap.get_num_fields_on_coll( a_set, a_set_coll_num );
				a_set_name = saf_wrap.get_set_name(a_set);
				a_coll_name = saf_wrap.get_coll_cat_name(a_set,coll_id);
				sub_coll_count = saf_wrap.get_coll_count( a_set, coll_id );

				for( j = 0; j < num_fields; j++ ) {
					a_field_num = saf_wrap.get_field_rel_id(a_set, a_set_coll_num, j);
					a_field_id = saf_wrap.get_field_id( a_set, a_field_num );
					if( field_name.equals(saf_wrap.get_field_name(a_field_id)) ) {
						field_ids[i] = a_field_id;
					}
				}
				for( j = 0; j < subset_rel_size[i]; j++ ) {
					subset_value = saf_wrap.get_subset_relation_data( rel_id, j );
					field_lookup_table[subset_value][0] = j;
					field_lookup_table[subset_value][1] = field_ids[i]; 
					field_lookup_table[subset_value][2] = sub_coll_count; 
				}

			}

		}

		data_type = saf_wrap.get_field_data_type( field_lookup_table[0][1] );

		// DefaultTableModel model = new DefaultTableModel();

		// JTable table = new JTable(model);

		coeff_assoc_id = saf_wrap.get_field_coeff_assoc_id( field_id );

		component_id = saf_wrap.get_field_component( field_id, 0);

		if( (num_comps > 1) && (component_id != 0) ) {
			assoc_ratio = num_comps * assoc_ratio;
			column_headers = new Object[num_comps+1];
			for( i = 0; i < num_comps; i++ ) {
				component_id = saf_wrap.get_field_component( field_id, i);
				column_headers[i+1] = saf_wrap.get_field_name( component_id );
			}
		}


		int columns = assoc_ratio+1;

		Object [][] tabledata = new Object[coll_count][columns];
		Object[] rowdata = new Object[columns] ;
                if( coeff_assoc_id != 0 )
                        coeff_assoc = saf_wrap.get_category_name( coeff_assoc_id );
                else
                        coeff_assoc = "Undefined";

		// model.addColumn(set_name + ": " + coeff_assoc);
		String columnzeroheader = set_name + ": " + coeff_assoc;
		column_headers[0] = columnzeroheader;



		if( (num_comps > 1)  && (component_id != 0) && ! atype.equals("field") ) {

			for( i = 0; i < num_comps; i++ ) {
				// model.addColumn(column_headers[i]);
				;
			}
		}
		else {
			for( i = 0; i < assoc_ratio; i++ ) {
				colnumber = i+1;
				if( i > 0 ) {
					// model.addColumn(field_name + " " + colnumber );
					column_headers[colnumber] = field_name + " " + colnumber;
				}
				else {
					// model.addColumn(field_name);
					column_headers[colnumber] = field_name;
				}
			}
		}
		

		
		// if( atype.equals("field") )	{
			// data_size = coll_count;
		// }
//  
		// else
			// data_size = saf_wrap.get_field_data_size( field_id );

		// if( data_size < coll_count )
		// 	coll_count = data_size;

		data_size = lookup_count;
		coll_count = lookup_count;

		 if( data_size > 0 ) {
			rownumber = 1;
		 	for( i = 0; i < coll_count;  ) {
				rowheadernum = rownumber-1;
				rowheader = "" + rowheadernum;
				// Object [] rowheaderdata = { rowheader, "test: " + field_id };
				// tabledata[i][0] = rowheaderdata;
				tabledata[i][0] = rowheader;
				for( j = 0; j < assoc_ratio ; j++ ) {

					interleave = saf_wrap.get_field_interleave( field_lookup_table[i][1] );

					if( interleave.equals("VECTOR") ) {

						if( data_type.equals("SAF_FLOAT") || data_type.equals("SAF_DOUBLE") ) {
							//float_value = new Float( saf_wrap.get_field_data( field_id, i*assoc_ratio+j));
							float_value = new Float( saf_wrap.get_field_data( field_lookup_table[i][1], field_lookup_table[i][0]));
						}
						else {
							// int_value = new Integer( (int)(saf_wrap.get_field_data( field_id, i*assoc_ratio+j)));
							int_value = new Integer( (int)saf_wrap.get_field_data( field_lookup_table[i][1], field_lookup_table[i][0]));
						}

					}
					else {
						if( data_type.equals("SAF_FLOAT") || data_type.equals("SAF_DOUBLE") ) {
							// float_value = new Float( saf_wrap.get_field_data( field_id, j*coll_count+i));
							float_value = new Float( saf_wrap.get_field_data( field_lookup_table[i][1], (j * sub_coll_count) + field_lookup_table[i][0] ));

						}
						else {
							// int_value = new Integer( (int)(saf_wrap.get_field_data( field_id, j*coll_count+i)));
							// int_value = new Integer( (int)saf_wrap.get_field_data( field_lookup_table[i][1], field_lookup_table[i][0]));
							int_value = new Integer( (int)saf_wrap.get_field_data( field_lookup_table[i][1], (sub_coll_count * j) + field_lookup_table[i][0] ));
						}
					}

					// rowdata[j+1] = float_value.toString();
					// rowdata[j+1] = float_value;

					if( data_type.equals("SAF_FLOAT") || data_type.equals("SAF_DOUBLE") )
						tabledata[i][j+1] = float_value;
					else
						tabledata[i][j+1] = int_value;
					
				}
				i++;
				// model.addRow(rowdata);
				rownumber++;
		 	}
		 }
	
		 	

		FieldTableModel model = new FieldTableModel( tabledata, column_headers );
		JTable table = new JTable(model);
		// table.setRowHeight(30);
		// TableColumn buttonColumn = table.getColumn(columnzeroheader);
		// TableCellRenderer buttoncolumnrenderer = new ButtonCellRenderer();

		// buttonColumn.setCellRenderer(buttoncolumnrenderer);

		// TableCellEditor buttoncelleditor = new ButtonCellEditor();
		// buttonColumn.setCellEditor(buttoncelleditor);

		 JScrollPane rawdataarea = new JScrollPane( table );

		this.setLayout( new BorderLayout());

		// rawdataarea.append("abc123");
		this.add(rawdataarea, BorderLayout.NORTH);

	}
}

