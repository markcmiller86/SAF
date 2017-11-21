/*
 * Copyright(C) 1999-2005 The Regents of the University of California.
 *     This work  was produced, in  part, at the  University of California, Lawrence Livermore National
 *     Laboratory    (UC LLNL)  under    contract number   W-7405-ENG-48 (Contract    48)   between the
 *     U.S. Department of Energy (DOE) and The Regents of the University of California (University) for
 *     the  operation of UC LLNL.  Copyright  is reserved to  the University for purposes of controlled
 *     dissemination, commercialization  through formal licensing, or other  disposition under terms of
 *     Contract 48; DOE policies, regulations and orders; and U.S. statutes.  The rights of the Federal
 *     Government  are reserved under  Contract 48 subject  to the restrictions agreed  upon by DOE and
 *     University.
 * 
 * Copyright(C) 1999-2005 Sandia Corporation.  
 *     Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive license for use of this work
 *     on behalf of the U.S. Government.  Export  of this program may require a license from the United
 *     States Government.
 * 
 * Disclaimer:
 *     This document was  prepared as an account of  work sponsored by an agency  of  the United States
 *     Government. Neither the United States  Government nor the United States Department of Energy nor
 *     the  University  of  California  nor  Sandia  Corporation nor any  of their employees  makes any
 *     warranty, expressed  or  implied, or  assumes   any  legal liability  or responsibility  for the
 *     accuracy,  completeness,  or  usefulness  of  any  information, apparatus,  product,  or process
 *     disclosed,  or  represents that its  use would   not infringe  privately owned rights. Reference
 *     herein  to any  specific commercial  product,  process,  or  service by  trade  name, trademark,
 *     manufacturer,  or  otherwise,  does  not   necessarily  constitute  or  imply  its  endorsement,
 *     recommendation, or favoring by the  United States Government   or the University of  California.
 *     The views and opinions of authors expressed herein do not necessarily state  or reflect those of
 *     the  United  States Government or  the   University of California   and shall  not be  used  for
 *     advertising or product endorsement purposes.
 * 
 * 
 * Active Developers:
 *     Peter K. Espen              SNL
 *     Eric A. Illescas            SNL
 *     Jake S. Jones               SNL
 *     Robb P. Matzke              LLNL
 *     Greg Sjaardema              SNL
 * 
 * Inactive Developers:
 *     William J. Arrighi          LLNL
 *     Ray T. Hitt                 SNL
 *     Mark C. Miller              LLNL
 *     Matthew O'Brien             LLNL
 *     James F. Reus               LLNL
 *     Larry A. Schoof             SNL
 * 
 * Acknowledgements:
 *     Marty L. Barnaby            SNL - Red parallel perf. study/tuning
 *     David M. Butler             LPS - Data model design/implementation Spec.
 *     Albert K. Cheng             NCSA - Parallel HDF5 support
 *     Nancy Collins               IBM - Alpha/Beta user
 *     Linnea M. Cook              LLNL - Management advocate
 *     Michael J. Folk             NCSA - Management advocate 
 *     Richard M. Hedges           LLNL - Blue-Pacific parallel perf. study/tuning 
 *     Wilbur R. Johnson           SNL - Early developer
 *     Quincey Koziol              NCSA - Serial HDF5 Support 
 *     Celeste M. Matarazzo        LLNL - Management advocate
 *     Tyce T. McLarty             LLNL - parallel perf. study/tuning
 *     Tom H. Robey                SNL - Early developer
 *     Reinhard W. Stotzer         SNL - Early developer
 *     Judy Sturtevant             SNL - Red parallel perf. study/tuning 
 *     Robert K. Yates             LLNL - Blue-Pacific parallel perf. study/tuning
 * 
 */
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


class IndirectFieldData extends JPanel {

	Float float_value;

	public IndirectFieldData( int set_id, int coll_id,  int field_id ) {

		super();

		int i,j, rownumber, colnumber, coeff_assoc_id, assoc_ratio, num_comps;
		String coeff_assoc;
		int data_size;
		int coll_count;
		int fieldtmpl_id;
		int state_count;
		JButton rowButton;
		String data_string="";
		String state_name, rowheader;
		int field_tmpl_id;

		int comp_field_id;
		String comp_field_name;

		fieldtmpl_id = saf_wrap.get_fieldtmpl_id( field_id );
		String atype = saf_wrap.get_fieldtmpl_atype( fieldtmpl_id  ); 
		String set_name = saf_wrap.get_set_name( set_id );
		coll_count = saf_wrap.get_coll_count( set_id, coll_id );
		state_name = saf_wrap.get_field_name( field_id );
		data_size = saf_wrap.get_field_data_size( field_id );
		state_count = coll_count;
		assoc_ratio = saf_wrap.get_field_assoc_ratio( field_id );


		num_comps = saf_wrap.get_fieldtmpl_num_comp( fieldtmpl_id );

		// System.out.println("StateData: num_comps is " + num_comps);

		// num_comps = 1;
	
		
		String interleave = saf_wrap.get_field_interleave( field_id );

		// DefaultTableModel model = new DefaultTableModel();
		// StateFieldModel model = new StateFieldModel();

		// JTable table = new JTable(model);

		coeff_assoc_id = saf_wrap.get_field_coeff_assoc_id( field_id );

		int columns = assoc_ratio+1;

                if( coeff_assoc_id != 0 )
                        coeff_assoc = saf_wrap.get_category_name( coeff_assoc_id );
                else
                        coeff_assoc = "Undefined";

		// model.addColumn(set_name + ": " + coeff_assoc);

		String columnzeroheader = "Base Space";
		Object [] column_headers = new Object[2];

		column_headers[0] = columnzeroheader;
  
		column_headers[1] = "fields"; 

		Object [][] tabledata = new Object[data_size][2];
		int state_id = field_id;


		int column_num;

		// System.out.println("IndirectFieldData: data_size is " + data_size );

		for( i = 0; i < data_size ; i++ ) {
			// comp_field_id = saf_wrap.get_state_field_on_state( field_id, 0);

			comp_field_id = (int)(saf_wrap.get_indfield_data( field_id, i));

			field_tmpl_id = saf_wrap.get_fieldtmpl_id( comp_field_id );
			

			// System.out.println("IndirectFieldData: comp_field_id is " + comp_field_id );

			comp_field_name = saf_wrap.get_field_name( comp_field_id );

			tabledata[i][0] = saf_wrap.get_field_base_space_name( comp_field_id );

/*
			for( j = 0; j < state_count; j++ ) {
				column_num = j+1;
				comp_field_id = saf_wrap.get_state_data( field_id, j, i);
				Object [] statecell = { "state " + column_num + ": " + tabledata[i][0], new Integer(comp_field_id) };
				tabledata[i][j+1] = statecell;
			}
*/

		}

		for( i = 0 ; i < data_size; i++ ) {
		      
		    comp_field_id = (int)(saf_wrap.get_indfield_data( field_id, i));
		    Object [] statecell = { "field " + ": " + tabledata[i][0], new Integer(comp_field_id) };
		    tabledata[i][1] = statecell;


		}


                StateFieldModel model = new StateFieldModel( tabledata, column_headers );
                JTable table = new JTable(model);
                // table.setRowHeight(30);

		TableCellRenderer buttoncellrenderer = new ButtonCellRenderer();
		TableCellEditor   buttoncelleditor   = new ButtonCellEditor();

 
			table.setDefaultRenderer( table.getColumnClass(1), buttoncellrenderer );
			table.setDefaultEditor( table.getColumnClass(1), buttoncelleditor );
			table.getColumn((String)column_headers[1]).setMinWidth(200);

		table.getColumn((String)column_headers[0]).setMinWidth(150);

                TableColumn nameColumn = table.getColumn(columnzeroheader);
                nameColumn.setCellRenderer(new DefaultTableCellRenderer());

                 JScrollPane rawdataarea = new JScrollPane( table );

		 rawdataarea.setHorizontalScrollBarPolicy(
                                           JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS); 
                 table.setAutoResizeMode(JTable.AUTO_RESIZE_OFF); 

                this.setLayout( new BorderLayout());

                // rawdataarea.append("abc123");
                this.add(rawdataarea, BorderLayout.NORTH);

	}
}


