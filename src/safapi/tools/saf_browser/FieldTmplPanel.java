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


class FieldTmplPanel extends JPanel {


	public FieldTmplPanel( int fieldtmpl_info ) {

		super();

		final int fieldtmpl_id = fieldtmpl_info;
		String fieldtmpl_name = saf_wrap.get_fieldtmpl_name( fieldtmpl_id );
		// int base_space = saf_wrap.get_fieldtmpl_base_space_id( fieldtmpl_id );
		// String base_space_name = saf_wrap.get_fieldtmpl_base_space_name( fieldtmpl_id );
		String atype = saf_wrap.get_fieldtmpl_atype( fieldtmpl_id );
		String basis = saf_wrap.get_fieldtmpl_basis( fieldtmpl_id );
		int num_comps = saf_wrap.get_fieldtmpl_num_comp( fieldtmpl_id );
		int component_tmpl = saf_wrap.get_fieldtmpl_component( fieldtmpl_id, 0 );


		JPanel tmplinfopanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
		if( atype.equals("field") )
			tmplinfopanel.setBorder(BorderFactory.createTitledBorder("Template Info"));
		else
			tmplinfopanel.setBorder(BorderFactory.createTitledBorder("Template Info"));
		
		JPanel tmpllabels = new JPanel();
		tmpllabels.setLayout(new BoxLayout(tmpllabels, BoxLayout.Y_AXIS));

		JLabel tmplname_label;	
		if( atype.equals("field") )	
			tmplname_label =            new JLabel("Template Name:                     ");
		else
			tmplname_label =            new JLabel("Template Name:                     ");
		JTextField tmplname_message = new JTextField(fieldtmpl_name);
		JPanel tmplname_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		tmplname_wrapper.add(tmplname_label);
		tmplname_wrapper.add(tmplname_message);

		// JLabel tmplbasename_label =        new JLabel("Template Base Space:                    ");
		// JTextField tmplbasename_message = new JTextField(base_space_name);
		// JPanel tmplbasename_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		// tmplbasename_wrapper.add(tmplbasename_label);
		// tmplbasename_wrapper.add(tmplbasename_message);

		JLabel tmplatypename_label =     new JLabel("Algebraic Type:                              ");
		JTextField tmplatypename_message = new JTextField(atype);
		JPanel tmplatypename_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		tmplatypename_wrapper.add(tmplatypename_label);
		tmplatypename_wrapper.add(tmplatypename_message);

		JLabel tmplbasisname_label =       new JLabel("BASIS:                                           ");
		JTextField tmplbasisname_message = new JTextField(basis);
		JPanel tmplbasisname_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		tmplbasisname_wrapper.add(tmplbasisname_label);
		tmplbasisname_wrapper.add(tmplbasisname_message);

		JLabel tmplcomp_label =            new JLabel("Number of Components:                ");
		Integer Num_Comps = new Integer(num_comps);
		JTextField tmplcomp_message = new JTextField(Num_Comps.toString());
		JPanel tmplcomp_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		tmplcomp_wrapper.add(tmplcomp_label);
		tmplcomp_wrapper.add(tmplcomp_message);

		tmpllabels.add(tmplname_wrapper);
		// tmpllabels.add(tmplbasename_wrapper);
		tmpllabels.add(tmplatypename_wrapper);
		tmpllabels.add(tmplbasisname_wrapper);
		tmpllabels.add(tmplcomp_wrapper);

		if( component_tmpl != 0 ) {
		
			JPanel comptmplpanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
			comptmplpanel.setBorder(BorderFactory.createTitledBorder("Component Field Templates"));

			final java.awt.List comptmpllist = new java.awt.List(5)
                        { 
                        public Dimension getPreferredSize() 
                        { 
                        return new Dimension(250,100) ; 
                        } 
                        }; 
			comptmpllist.setMultipleMode(false);

			for( int i = 0; i < num_comps; i++ ) {
				component_tmpl = saf_wrap.get_fieldtmpl_component( fieldtmpl_id, i );
				comptmpllist.add( saf_wrap.get_fieldtmpl_name( component_tmpl ) );

			}	

			comptmpllist.select(0);

			
			comptmplpanel.add(comptmpllist);

			JButton comptmplbutton = new JButton("Component Template Info");
			comptmplpanel.add(comptmplbutton);

        		comptmplbutton.addActionListener(new ActionListener() {

                		public void actionPerformed(ActionEvent e) {

					int i = comptmpllist.getSelectedIndex();
					CompTmplDialog comptmpldialog = new CompTmplDialog(saf_wrap.get_fieldtmpl_component( fieldtmpl_id, i ));
					

				}
                        });

			tmpllabels.add(comptmplpanel);

		}



		tmplinfopanel.add(tmpllabels);

		this.add(tmplinfopanel);
	}


} 

class CompTmplDialog {


        public CompTmplDialog( int fieldtmpl_id )
        {

                FieldTmplPanel fieldtmplpanel = new FieldTmplPanel( fieldtmpl_id );

                JDialog comptmpldialog = new JDialog();
                comptmpldialog.getContentPane().setLayout(new BorderLayout());
                comptmpldialog.setTitle("Field Template: " + saf_wrap.get_fieldtmpl_name( fieldtmpl_id ) );
                comptmpldialog.getContentPane().add(fieldtmplpanel, BorderLayout.NORTH);

                JButton comptmpldialog_disposebutton = new JButton("Dispose");
                comptmpldialog_disposebutton.addActionListener(new disposeButtonListener( (Window)comptmpldialog) );

                comptmpldialog.getContentPane().add(comptmpldialog_disposebutton, BorderLayout.SOUTH);

                comptmpldialog.pack();

                comptmpldialog.setVisible(true);

        }

}


