import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;

class ButtonCellRenderer extends JButton implements TableCellRenderer {

	// private JButton mybutton = new JButton();

	Object [] buttondata;

	public ButtonCellRenderer() {

		// setLayout(new FlowLayout(FlowLayout.CENTER));
		// mybutton.setText("MyButton");
		// add(mybutton);

		super();
	}

	public Component getTableCellRendererComponent(
		JTable table, Object value, boolean isSelected,
		boolean hasFocus,
		int row, int col) {

		buttondata = (Object [])value;

		setText( (String)(buttondata[0]));
		// setEnabled(true);

		return this;
	}

}
