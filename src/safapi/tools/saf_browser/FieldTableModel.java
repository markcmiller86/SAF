import java.awt.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;

import java.awt.*;
import java.awt.event.*;


class FieldTableModel extends DefaultTableModel{


        public FieldTableModel( Object [][] data, Object [] columnNames) {
                super( data, columnNames );
        }


        public Class getColumnClass( int col ) {

                Object [] theobject;

                Vector v = (Vector)dataVector.elementAt(0);
                return v.elementAt(col).getClass();

        }

        public boolean isCellEditable( int row, int col ) {

                        return false;
        }

}


