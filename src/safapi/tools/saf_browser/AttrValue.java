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


class AttrValue extends JPanel {

	Float float_value;
	Integer int_value;

	public AttrValue( int handle_id, String attr_name ) {

		super();

		int i,j, rownumber, rowheadernum, colnumber;
		int data_size;
		int component_id;
		JButton rowButton;
		String data_string="";
		String rowheader;
		Object[] column_headers=new Object[2];


		int attr_count = saf_wrap.sub_attribute_count( handle_id, attr_name );
		System.out.println("sub_attribute count is " + attr_count );

		int columns = 2;

		Object [][] tabledata = new Object[attr_count][columns];
		Object[] rowdata = new Object[columns] ;

		String columnzeroheader = attr_name + " : item";
		column_headers[0] = columnzeroheader;
		column_headers[1] = attr_name + " : value";

		data_size = attr_count;


		 if( data_size > 0 ) {
		 	for( i = 0; i < data_size; i++  ) {
			    Integer the_i = new Integer(i);
			    tabledata[i][0] = the_i.toString();
			    tabledata[i][1] = saf_wrap.get_attr_data( handle_id, attr_name, i);
			    // System.out.println(tabledata[i][0] + " : " + tabledata[i][1] );
			}
			    
		 }
	
		 	

		FieldTableModel model = new FieldTableModel( tabledata, column_headers );
		JTable table = new JTable(model);

		 JScrollPane rawdataarea = new JScrollPane( table );

		this.setLayout( new BorderLayout());

		this.add(rawdataarea, BorderLayout.NORTH);

	}
}

