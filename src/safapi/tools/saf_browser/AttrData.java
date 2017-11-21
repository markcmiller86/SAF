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


class AttrData extends JPanel {

	Float float_value;
	Integer int_value;

	public AttrData( Vector attr_list, int handle_id ) {

		super();

		int i,j, rownumber, rowheadernum, colnumber, attr_count;
		int data_size;
		int component_id;
		JButton rowButton;
		String data_string="";
		String rowheader;
		String attr_name;
		Object[] column_headers=new Object[2];

		// final JList jattrlist = new JList( attr_list );
		// item_num = jattrlist.getSelectedIndex();


		// int attr_count = saf_wrap.sub_attribute_count( handle_id, attr_name );
		// System.out.println("num items in list is : " + attr_list.size() );

		int columns = 2;
		attr_count = saf_wrap.attribute_count( handle_id );
		// System.out.println( saf_wrap.attribute_string( handle_id ));

		Object [][] tabledata = new Object[attr_count][columns];
		Object[] rowdata = new Object[columns] ;

		String columnzeroheader = "Attribute Name";
		column_headers[0] = columnzeroheader;
		column_headers[1] = "Attribute Value";


		for( i = 0 ; i < attr_list.size() ; i++ ) {
		    attr_name = (String)attr_list.elementAt(i);
		    tabledata[i][0] = attr_name;
		    Object [] attrcell = { attr_name, new Integer(handle_id) };
		    // Object [] attrcell = { saf_wrap.get_attr_data( handle_id, attr_name, 0) };
		    tabledata[i][1] = attrcell;
		    
		}
		//data_size = attr_count;


		//if( data_size > 0 ) {
		//for( i = 0; i < data_size; i++  ) {
		//    Integer the_i = new Integer(i);
		//    tabledata[i][0] = the_i.toString();
		//    tabledata[i][1] = saf_wrap.get_attr_data( handle_id, attr_name, i);
			    // System.out.println(tabledata[i][0] + " : " + tabledata[i][1] );
		//	}
			    
		// }
	
		 	
                AttrTableModel model = new AttrTableModel( tabledata, column_headers );
                JTable table = new JTable(model);

		TableCellRenderer buttoncellrenderer = new AttrCellRenderer();
		TableCellEditor   buttoncelleditor   = new AttrCellEditor();

		table.setDefaultRenderer( table.getColumnClass(1), buttoncellrenderer );
		table.setDefaultEditor( table.getColumnClass(1), buttoncelleditor );
		table.getColumn((String)column_headers[1]).setMinWidth(200);
      		table.getColumn((String)column_headers[0]).setMinWidth(150);

		 JScrollPane rawdataarea = new JScrollPane( table );

		this.setLayout( new BorderLayout());

		this.add(rawdataarea, BorderLayout.NORTH);

	}
}

