import java.awt.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;

public class ButtonCellEditor extends JButton implements TableCellEditor {

	public String originalValue;
	Object [] buttondata;
	Integer field_id;
        private ActionListener actionlistener = null;

	public ButtonCellEditor() {
		super();
	}

	public Component getTableCellEditorComponent(
		JTable table, Object value, boolean isSelected,
		int row, int col) {

		buttondata = (Object [])value;
		// originalValue = (String)value;
		originalValue = (String)buttondata[0];
		field_id = (Integer)buttondata[1];
		setText(originalValue);

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
				FieldDialog fielddialog = new FieldDialog( field_id.intValue() );
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
