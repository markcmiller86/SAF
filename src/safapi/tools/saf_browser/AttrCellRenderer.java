import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;

class AttrCellRenderer extends JButton implements TableCellRenderer {

	Object [] buttondata;

	public AttrCellRenderer() {

		super();
	}

	public Component getTableCellRendererComponent(
		JTable table, Object value, boolean isSelected,
		boolean hasFocus,
		int row, int col) {

		buttondata = (Object [])value;
		int handle_id = ((Integer)buttondata[1]).intValue();
		String attr_name = (String)buttondata[0];
		if( saf_wrap.is_attribute_primitive( handle_id , attr_name ) && (saf_wrap.sub_attribute_count(handle_id, attr_name) < 3) ) {
		    setText( saf_wrap.get_attr_data( handle_id, attr_name , 0) );
		    setBackground( Color.white );
		}
		else {
		    setText( " Click here for data " );
		    setBackground( Color.pink );
		}

		return this;
	}

}
