import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;

import java.awt.*;
import java.awt.event.*;

class TopoCellRenderer extends DefaultTableCellRenderer implements TableCellRenderer {


	public TopoCellRenderer() {

		// setLayout(new FlowLayout(FlowLayout.CENTER));
		// mybutton.setText("MyButton");
		// add(mybutton);

		super();
	}

	public Component getTableCellRendererComponent(
		JTable table, Object value, boolean isSelected,
		boolean hasFocus,
		int row, int col) {
		

		if( value == null )
			setBackground(Color.lightGray);
		else
			setBackground(Color.white);

		if( col == 0 )
			setBackground(Color.pink);

		setValue(value);


		return this;
	}

}
