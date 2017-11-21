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


class FieldDialog {

	JLabel titlelabel;
	String atype;
	String field_name;
	String set_name;
	int set_id = 0, coll_id = -1;
	String coll_name;
	int fieldtmpl_id;
	int field_id;
    int the_storage_decomp_id = 0;


	public FieldDialog( int the_set_id, int the_coll_id, int field_num)
	{

		set_id = the_set_id;
		coll_id = the_coll_id;

		JDialog fielddialog;

		field_id = saf_wrap.get_field_id( set_id, field_num );
		set_name = saf_wrap.get_set_name(set_id);
		coll_name = saf_wrap.get_coll_cat_name(set_id, coll_id);
                field_name = saf_wrap.get_field_name(field_id);
		the_storage_decomp_id = saf_wrap.get_field_storage_decomp_id( field_id );

		titlelabel = new JLabel(set_name + " -> " + coll_name + " -> " +  field_name);

		createFieldDialog( field_id );

	}
	public FieldDialog( int field_id)
	{
		String cat_name;
		int fieldtmpl_id, base_space_id, coeff_coll_id;

			fieldtmpl_id = saf_wrap.get_fieldtmpl_id( field_id );
			base_space_id = saf_wrap.get_field_base_space_id( field_id );
			coeff_coll_id = saf_wrap.get_field_coeff_assoc_id( field_id );
			set_id = base_space_id;
			cat_name = saf_wrap.get_category_name( coeff_coll_id );
			coll_id = saf_wrap.lookup_coll_num( set_id, cat_name );
			
			the_storage_decomp_id = saf_wrap.get_field_storage_decomp_id( field_id );

		set_name = saf_wrap.get_set_name(set_id);
		coll_name = saf_wrap.get_coll_cat_name(set_id, coll_id);
                field_name = saf_wrap.get_field_name(field_id);

		JDialog fielddialog;

		titlelabel = new JLabel(set_name + " -> " + coll_name + " -> " +  field_name);

		createFieldDialog( field_id );
	
	}


	public void createFieldDialog( int field_id ) {
	
		fieldtmpl_id = saf_wrap.get_fieldtmpl_id( field_id );
		atype = saf_wrap.get_fieldtmpl_atype( fieldtmpl_id );
                field_name = saf_wrap.get_field_name(field_id);


       
		final JDialog fielddialog = new JDialog();

		fielddialog.getContentPane().setLayout(new BorderLayout());
	
		// fielddialog.getContentPane().setLayout(new FlowLayout(FlowLayout.LEADING));

		if( atype.equals("field") )
		    if( the_storage_decomp_id == 0 ) {
			fielddialog.setTitle("State: " + field_name);
		    }
		    else {
			fielddialog.setTitle("Indirect Field: " + field_name);
		    }
		
		else
			fielddialog.setTitle("Field: " + field_name);



		JPanel toppanel = new JPanel();
		toppanel.setLayout(new BoxLayout(toppanel,BoxLayout.Y_AXIS));

		FieldTmplPanel tmplpanel = new FieldTmplPanel( fieldtmpl_id );

		if( titlelabel != null )
			toppanel.add(titlelabel);

		toppanel.add(tmplpanel);

		FieldPanel fieldpanel = new FieldPanel( field_id );

		toppanel.add(fieldpanel);

		//fielddialog.getContentPane().add(tmplpanel, BorderLayout.NORTH);
		fielddialog.getContentPane().add(toppanel, BorderLayout.NORTH);

		// JButton fielddialog_disposebutton = new JButton("Dispose");
		// fielddialog_disposebutton.addActionListener(new disposeButtonListener( (Window)fielddialog) );
		// fielddialog.getContentPane().add(fielddialog_disposebutton, BorderLayout.SOUTH);

		FieldButtonPanel buttonpanel = new FieldButtonPanel( (Window)fielddialog, set_id, coll_id, field_id );
		fielddialog.getContentPane().add( buttonpanel, BorderLayout.SOUTH );
	

		fielddialog.pack();

		fielddialog.setVisible(true);

	}

}

class FieldButtonPanel extends JPanel {


	public FieldButtonPanel( Window fielddialog, int set_id, int coll_id,  int the_field_id ) {

		super();

		final int field_id = the_field_id;
		final String title = saf_wrap.get_field_name( field_id );
		final int the_set_id = set_id;
		final int the_coll_id = coll_id;

		final int fieldtmpl_id = saf_wrap.get_fieldtmpl_id( the_field_id );
		final String atype = saf_wrap.get_fieldtmpl_atype( fieldtmpl_id  );
		final int storage_decomp_id = saf_wrap.get_field_storage_decomp_id( field_id );

		String data_type = saf_wrap.get_field_data_type( the_field_id );
		int data_count = saf_wrap.get_field_data_size( the_field_id );
		String frame_title = "";
		
		// this.setLayout(new BoxLayout(this,BoxLayout.X_AXIS));
		this.setLayout(new FlowLayout(FlowLayout.CENTER));
		
		JButton fielddialog_disposebutton = new JButton("Dispose");
		fielddialog_disposebutton.addActionListener(new disposeButtonListener( (Window)fielddialog) );
		// fielddialog.getContentPane().add(fielddialog_disposebutton, BorderLayout.SOUTH);

		String category_name = saf_wrap.get_coll_cat_name( set_id, coll_id );

		this.add( fielddialog_disposebutton );


		if( ! category_name.equals("stategroups") )
		    if( atype.equals("field") && (storage_decomp_id == 0) )
			return;

		JButton readbutton;

		if( atype.equals("field") ) {
		    readbutton = new JButton("Remap & Read Fields");
		}
		else
		    readbutton = new JButton("Read Field Data");

        	readbutton.addActionListener(new ActionListener() {

                	public void actionPerformed(ActionEvent e) {
			    int targeted_field_id;
			    int re_base_space_id;
			    int re_coeff_coll_id;
			    String re_cat_name;
			    int re_coll_id;
			    int re_fieldtmpl_id, temp_field_tmpl_id;
			    int dest_field_id;
			    int the_state_field;
			    String dest_atype;


				if( saf_wrap.get_field_data_size( field_id ) > 0 ) {


				JPanel fielddata;

				if( atype.equals("field") ) {
				    if( storage_decomp_id != 0 ) {
					dest_field_id = (int)(saf_wrap.get_indfield_data( field_id, 0, 1));
					temp_field_tmpl_id = saf_wrap.get_fieldtmpl_id( dest_field_id );
					while( ((saf_wrap.get_fieldtmpl_atype(temp_field_tmpl_id)).equals("field")) ) {
					    dest_field_id = (int)(saf_wrap.get_indfield_data( dest_field_id, 0, 1));
					    temp_field_tmpl_id = saf_wrap.get_fieldtmpl_id( dest_field_id );
					}

					targeted_field_id = saf_wrap.remap_field( field_id ,dest_field_id );
					// fielddata = new IndirectFieldData( the_set_id, the_coll_id, field_id);

					re_fieldtmpl_id = saf_wrap.get_fieldtmpl_id( targeted_field_id );
					re_base_space_id = saf_wrap.get_field_base_space_id( targeted_field_id );
					re_coeff_coll_id = saf_wrap.get_field_coeff_assoc_id( targeted_field_id );
					re_cat_name = saf_wrap.get_category_name( re_coeff_coll_id );
					re_coll_id = saf_wrap.lookup_coll_num( re_base_space_id, re_cat_name );


					// tmp_fielddialog = new FieldDialog( targeted_field_id );
					fielddata = new FieldData( re_base_space_id, re_coll_id, targeted_field_id );

				    }
				    else {

					fielddata = new StateData( the_set_id, the_coll_id, field_id );

				    }
				}
				else {
				  if( storage_decomp_id != 0 ) {				  
						fielddata = new IndFieldData( the_set_id, the_coll_id, field_id );
				  }
				  else {
						fielddata = new FieldData( the_set_id, the_coll_id, field_id );
				  }
				}

				JButton fielddata_disposebutton = new JButton("Dispose");

				JFrame fielddataframe = new JFrame(saf_wrap.get_set_name( the_set_id ) + "->" + title  );

				fielddata_disposebutton.addActionListener(new disposeButtonListener( (Window)fielddataframe) );
				fielddata.add(fielddata_disposebutton, BorderLayout.SOUTH);

				// fielddata.setPreferredSize( new Dimension(800,500));

				fielddataframe.getContentPane().add( fielddata );

				fielddataframe.pack();
				fielddataframe.show();
				}

			}
		});

		// if( !(data_type.equals("DSL_HANDLE")) && (data_count != 0)  )
		this.add( readbutton );

	}

}


class FieldPanel extends JPanel {

	public FieldPanel( int the_field_id ) {

		final int field_id = the_field_id;

		String atype = saf_wrap.get_fieldtmpl_atype( saf_wrap.get_fieldtmpl_id( field_id ) );
		String field_name = saf_wrap.get_field_name( field_id );
		String base_space_name = saf_wrap.get_field_base_space_name( field_id );
		int coeff_assoc_id = saf_wrap.get_field_coeff_assoc_id( field_id );
		int assoc_ratio = saf_wrap.get_field_assoc_ratio( field_id );
		int eval_coll_id = saf_wrap.get_field_eval_coll_id( field_id );
		String eval_func = saf_wrap.get_field_eval_func( field_id );
		int storage_decomp_id = saf_wrap.get_field_storage_decomp_id( field_id );
		int num_field_comps = saf_wrap.get_field_num_comps( field_id );
		int fieldtmpl_id = saf_wrap.get_fieldtmpl_id( field_id );
		int num_comps = saf_wrap.get_fieldtmpl_num_comp( fieldtmpl_id );
		boolean is_coord = saf_wrap.get_field_is_coord( field_id );
		
		int component = saf_wrap.get_field_component( field_id, 0 );
		int base_space_id = saf_wrap.get_field_base_space_id( field_id );
		int base_collection_num = 0;
		int suite_dimension = 0;

		String coeff_assoc;
		String eval_coll;
		String storage_decomp;
		String is_coord_answer;

		if( coeff_assoc_id != 0 ) {
			coeff_assoc = saf_wrap.get_category_name( coeff_assoc_id );
			base_collection_num = saf_wrap.lookup_coll_num( base_space_id, coeff_assoc );
			suite_dimension = saf_wrap.get_coll_count( base_space_id, base_collection_num );
		}
		else
			coeff_assoc = "Undefined";
			
		if( eval_coll_id != 0 )
			eval_coll = saf_wrap.get_category_name( eval_coll_id );
		else
			eval_coll = "Undefined";
			
		if( storage_decomp_id != 0 )
			storage_decomp = saf_wrap.get_category_name( storage_decomp_id );
		else
			storage_decomp = "SAF_SELF";
			

		JPanel fieldinfopanel = new JPanel(new FlowLayout(FlowLayout.LEFT));

		JLabel fieldname_label;
		if( atype.equals("field") ) {
		    if( storage_decomp_id == 0 ) {
			fieldinfopanel.setBorder(BorderFactory.createTitledBorder("State Field Info"));
			fieldname_label =            new JLabel("State Name:                             ");
		    }
		    else {
			fieldinfopanel.setBorder(BorderFactory.createTitledBorder("Indirect Field Info"));
			fieldname_label =            new JLabel("Field Name:                             ");
			
		    }
		}
		else {
			fieldname_label =            new JLabel("Field Name:                             ");
			fieldinfopanel.setBorder(BorderFactory.createTitledBorder("Field Info"));
		}

		JPanel fieldlabels = new JPanel();
		fieldlabels.setLayout(new BoxLayout(fieldlabels, BoxLayout.Y_AXIS));
		
		JTextField fieldname_message = new JTextField(field_name);
		JPanel fieldname_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		fieldname_wrapper.add(fieldname_label);
		fieldname_wrapper.add(fieldname_message);

		JLabel stodcompname_label =        new JLabel("Storage Decomposition:            ");
		JTextField stodcompname_message = new JTextField(storage_decomp);
		JPanel stodcompname_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		stodcompname_wrapper.add(stodcompname_label);
		stodcompname_wrapper.add(stodcompname_message);

		JLabel coassocname_label =        new JLabel("Coefficient Association:            ");
		JTextField coassocname_message = new JTextField(coeff_assoc);
		JPanel coassocname_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		coassocname_wrapper.add(coassocname_label);
		coassocname_wrapper.add(coassocname_message);

		JLabel basename_label =        new JLabel("Base Space:                    ");
		JTextField basename_message = new JTextField(base_space_name);
		JPanel basename_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		basename_wrapper.add(basename_label);
		basename_wrapper.add(basename_message);



		JLabel assrationame_label =        new JLabel("Association Ratio:                   ");
		JTextField assrationame_message = new JTextField(assoc_ratio + ":1");
		JPanel assrationame_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		assrationame_wrapper.add(assrationame_label);
		assrationame_wrapper.add(assrationame_message);

		JLabel evcollname_label =        new JLabel("Evaluation Collection:              ");
		JTextField evcollname_message = new JTextField(eval_coll);
		JPanel evcollname_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		evcollname_wrapper.add(evcollname_label);
		evcollname_wrapper.add(evcollname_message);

		JLabel evfuncname_label =        new JLabel("Evaluation Function:                ");
		JTextField evfuncname_message = new JTextField(eval_func);
		JPanel evfuncname_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		evfuncname_wrapper.add(evfuncname_label);
		evfuncname_wrapper.add(evfuncname_message);

		if( is_coord )
			is_coord_answer = "TRUE";
		else
			is_coord_answer = "FALSE";
				
		JLabel iscoordname_label =        new JLabel("Is a Coordinate Field?:              ");
		JTextField iscoordname_message = new JTextField(is_coord_answer);
		JPanel iscoordname_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		iscoordname_wrapper.add(iscoordname_label);
		iscoordname_wrapper.add(iscoordname_message);

		String data_type = saf_wrap.get_field_data_type( field_id );
		JLabel typename_label =        new JLabel("SAF_Type:                                ");
		JTextField typename_message = new JTextField(data_type);
		JPanel typename_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		typename_wrapper.add(typename_label);
		typename_wrapper.add(typename_message);

		Integer field_data_size;
		JLabel sizename_label;
		if( atype.equals("field") )
		    if( storage_decomp_id == 0 ) {
			field_data_size = new Integer( saf_wrap.get_state_num_states(field_id) );
			sizename_label = new JLabel("Number of States:                       ");
		    }
		    else {
			field_data_size =  new Integer( saf_wrap.get_field_data_size( field_id ));
			sizename_label =        new JLabel("Field Data Count:                       ");
		    }
		else {
			field_data_size = new Integer( saf_wrap.get_field_data_size( field_id ));
			sizename_label =        new JLabel("Field Data Count:                       ");
		}
		JTextField sizename_message = new JTextField(field_data_size.toString());
		JPanel sizename_wrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
		sizename_wrapper.add(sizename_label);
		sizename_wrapper.add(sizename_message);


		fieldlabels.add(fieldname_wrapper);
		fieldlabels.add(basename_wrapper);
		fieldlabels.add(stodcompname_wrapper);
		fieldlabels.add(coassocname_wrapper);
		fieldlabels.add(assrationame_wrapper);
		fieldlabels.add(evcollname_wrapper);
		fieldlabels.add(evfuncname_wrapper);
		fieldlabels.add(iscoordname_wrapper);
		fieldlabels.add(typename_wrapper);
		fieldlabels.add(sizename_wrapper);


		if( atype.equals("field") && (storage_decomp_id != 0)) {
		    JPanel childrenpanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
		    childrenpanel.setBorder(BorderFactory.createTitledBorder("Indirect Field Contents: Base Space , Field"));

		    final java.awt.List childlist = new java.awt.List(field_data_size.intValue())
		    {
			public Dimension getPreferredSize(){
			    return new Dimension(250,100);
			}
		    };

		    int child_base_space_id;
		    int comp_fieldtmpl_id;
		    int comp_base_space_id;
		    String comp_base_space_name;
		    childlist.setMultipleMode(false);
		    for( int i = 0; i < field_data_size.intValue(); i++ ) {
			component = saf_wrap.get_indfield_data( field_id, i, 0);
			comp_fieldtmpl_id = saf_wrap.get_fieldtmpl_id( component );
			child_base_space_id = saf_wrap.get_field_base_space_id( field_id );
			comp_base_space_name = saf_wrap.get_set_name( child_base_space_id );
			
			childlist.add( comp_base_space_name + " , " + saf_wrap.get_field_name(component) );

		    }
		    childlist.select(0);
		    childrenpanel.add(childlist);

		    JButton childbutton = new JButton("Destination Field Info");
		    childrenpanel.add(childbutton);
		    childbutton.addActionListener( new ActionListener() {
			    public void actionPerformed(ActionEvent e) {
				int i = childlist.getSelectedIndex();
				FieldDialog fielddialog = new FieldDialog(saf_wrap.get_indfield_data( field_id, i, 0 ));
			    }
			});
		    fieldlabels.add(childrenpanel);


		}
		else if( component != 0 ) {
	
			JPanel comppanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
			comppanel.setBorder(BorderFactory.createTitledBorder("Component Fields"));

			final java.awt.List complist = new java.awt.List(5)
                        { 
                        public Dimension getPreferredSize() 
                        { 
                        return new Dimension(250,100) ; 
                        } 
                        }; 
			complist.setMultipleMode(false);

			for( int i = 0; i < num_comps; i++ ) {
				component = saf_wrap.get_field_component( field_id, i );
				complist.add( saf_wrap.get_field_name( component ) );

			}	

			complist.select(0);

			
			comppanel.add(complist);

			JButton compbutton = new JButton("Component Info");
			comppanel.add(compbutton);

        		compbutton.addActionListener(new ActionListener() {

                		public void actionPerformed(ActionEvent e) {

					int i = complist.getSelectedIndex();
					FieldDialog fielddialog = new FieldDialog(saf_wrap.get_field_component( field_id, i ));
					

				}
                        });

			fieldlabels.add(comppanel);
		}

		fieldinfopanel.add(fieldlabels);

		this.add(fieldinfopanel);
	}

}

