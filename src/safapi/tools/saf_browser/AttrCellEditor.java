import java.awt.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;

public class AttrCellEditor extends JButton implements TableCellEditor {

	public String originalValue;
	Object [] buttondata;
	Integer handle_id;
        private ActionListener actionlistener = null;

	public AttrCellEditor() {
		super();
	}

	public Component getTableCellEditorComponent(
		JTable table, Object value, boolean isSelected,
		int row, int col) {

		buttondata = (Object [])value;
		// originalValue = (String)value;
		originalValue = (String)buttondata[0];
		String attr_name = originalValue;
		handle_id = (Integer)buttondata[1];
		if( saf_wrap.is_attribute_primitive( handle_id.intValue(), attr_name ) && (saf_wrap.sub_attribute_count(handle_id.intValue(), attr_name) < 3) ) {
		    setText( saf_wrap.get_attr_data( handle_id.intValue(), attr_name, 0) );
		    setBackground( Color.white );
		}
		else {
		    setText( " Click here for data " );
		    setBackground( Color.pink );
		}

		if( actionlistener == null ) {
			actionlistener = newActionListener();
			// System.out.println("ButtonCellEditor.etTableCellEditorComponent: created new ActionListener");
		}

		addActionListener( actionlistener );

		return this;
	}

	public ActionListener newActionListener() {

		return new ActionListener() {
                	public void actionPerformed(ActionEvent e) {
				// setText(originalValue);
				// System.out.println("pressed" + originalValue);
				AttrValue attrvalue = new AttrValue( handle_id.intValue(), originalValue );



			JButton attrvalue_disposebutton = new JButton("Dispose");

			JFrame attrvalueframe = new JFrame( originalValue );

				attrvalue_disposebutton.addActionListener(new disposeButtonListener( (Window)attrvalueframe) );
				attrvalue.add(attrvalue_disposebutton, BorderLayout.SOUTH);

				attrvalueframe.getContentPane().add( attrvalue );

				attrvalueframe.pack();
				attrvalueframe.show();







			}
                };
	}

	public void cancelCellEditing() { fireEditingCanceled(); }

	public boolean isCellEditable( EventObject eo ) {return true; }

	public Object getCellEditorValue() {return new String(this.getText()); } 

	public boolean shouldSelectCell(EventObject eo ) {
		return true;
	}

	public boolean stopCellEditing() {
		fireEditingStopped();
		removeActionListener(actionlistener);
		return true;
	}

	public void addCellEditorListener(CellEditorListener cel) {
	}

	public void removeCellEditorListener(CellEditorListener cel) {
	}

	protected void fireEditingCanceled() {

	}

	protected void fireEditingStopped() {
	}

}
