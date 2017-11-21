import java.awt.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;


class AttrTableModel extends DefaultTableModel{


        public AttrTableModel( Object [][] data, Object [] columnNames) {
                super( data, columnNames );
        }


        public Class getColumnClass( int col ) {

                Object [] theobject;

                Vector v = (Vector)dataVector.elementAt(0);
                return v.elementAt(col).getClass();

        }

        public boolean isCellEditable( int row, int col ) {

	    Object [] value;
	    String attr_name;
	    int handle_id;
	    
	    if( col == 1 ) {
		value = (Object [])getValueAt( row, col );
		attr_name = (String)value[0];
		handle_id = ((Integer)value[1]).intValue();
		// System.out.println("AttrTableModel: " + attr_name + " : " + handle_id);
		if( saf_wrap.is_attribute_primitive( handle_id, attr_name ) && (saf_wrap.sub_attribute_count( handle_id, attr_name ) < 3) ) {
		    return false;
		}
	    }

                if( col > 0  )
                        return true;
                else
                        return false;
        }

}


