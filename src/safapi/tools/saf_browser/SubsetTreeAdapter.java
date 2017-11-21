import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import javax.swing.text.*;
import javax.accessibility.*;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.*;
import java.io.*;
import java.applet.*;
import java.net.*;


class SubsetTreeAdapter extends MouseAdapter {

	JTextArea treedetail;
	JTree tree;

	public SubsetTreeAdapter(  JTextArea thetreedetail, JTree thetree ) {
		super();
		tree = thetree;
		treedetail = thetreedetail;
	}

                public void mousePressed(MouseEvent e) {

                        String s = null;
                        SetObject parentobject = null;
                        SetObject tmpobject = null;
                        CollectionObject collectionobject;
                        int l_i;
                        int set_id=0, coll_id=-1, rel_id = 0, subset_data_size=0;
                        int topo_relation = 0;
                        int depth = 0;
                        String cat;
                        String set_name = null;
                        String coll_info="";
			String set_info="";
                        String field_info="";
                        int num_fields = 0;
                        JFrame framedisplay;
                        int result = -1;
                        DefaultMutableTreeNode node = null;

                        JTree t = (JTree)e.getSource();
                        int row = t.getRowForLocation(e.getX(), e.getY());
                        TreePath path = t.getPathForRow(row);
                        tree.setSelectionPath(path);

                        if( row >= 0 ) {
                                node = (DefaultMutableTreeNode)path.getLastPathComponent();
                                depth = node.getLevel() - 1 ;
                        }
                        else 
                                row = -1;

			if( node != null ) {
				tmpobject = (SetObject)(node.getUserObject());
				set_name = saf_wrap.get_set_name(tmpobject.set_id);
				set_id = tmpobject.set_id;
			}
			else {
				tmpobject  = null;
				set_name = "";
				set_id = -1;
			}

			final SetObject nodeobject = tmpobject;


			if( (SwingUtilities.isLeftMouseButton(e)) && (e.getClickCount() == 1) && (nodeobject != null) ) {
				CollectionObject tmpcollection;

				set_info = "\n" + set_info( set_id );

				// set_info = set_info + "\n\n\t" + saf_wrap.attribute_string( set_id ) + "\n";

				treedetail.append(set_info + "\n\tCollections on " + saf_wrap.get_set_name(set_id) + ":\n");
				// System.out.println(set_info + "\n\tCollections on " + saf_wrap.get_set_name(set_id) + ":");
			
		
				for( l_i = 0; l_i < nodeobject.num_colls ; l_i++ ) {
					cat = saf_wrap.get_coll_cat_name(nodeobject.set_id, l_i);
					
					coll_info = coll_info( nodeobject.set_id, l_i );
					treedetail.append(coll_info + "\n");
					// System.out.println(coll_info);
					
					// tmpcollection = (CollectionObject)(nodeobject.collections.get((Object)cat));
					// if( tmpcollection != null ) {
						// coll_info = saf_wrap.coll_info(nodeobject.set_id, tmpcollection.coll_id);
						// treedetail.append(coll_info + "\n");
					// }
				}
			}

                        if( depth >= 1 ) {
                                        DefaultMutableTreeNode parent = (DefaultMutableTreeNode)(node.getParent());
                                        parentobject = (SetObject)(parent.getUserObject());
					final SetObject po = parentobject;

				   if( (SwingUtilities.isRightMouseButton(e)) ){
					final JDialog subsetdialog = new JDialog();
					subsetdialog.getContentPane().setLayout(new BorderLayout());

					final java.awt.List subsetlist = new java.awt.List();
					// subsetlist.setSize(new Dimension(250,50));
					subsetlist.setMultipleMode(false);

					JPanel listpanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
					listpanel.add(subsetlist);
					listpanel.setBorder(BorderFactory.createTitledBorder("Collection Categories"));


					subsetdialog.getContentPane().add(listpanel, BorderLayout.CENTER);
					subsetdialog.setTitle("Subset Relations");


					JPanel buttonpanel = new JPanel();
					buttonpanel.setLayout(new FlowLayout(FlowLayout.CENTER));

					JButton canceldialogButton = new JButton("Cancel");
					final JButton cancelframeButton = new JButton("Dispose");
					JButton readButton = new JButton("Read Data");

					buttonpanel.add(readButton);
					buttonpanel.add(canceldialogButton);
					
		
					JLabel superlabel = new JLabel("Superset Name:");
					superlabel.setHorizontalAlignment(JLabel.LEFT);

					JLabel sublabel = new JLabel("Subset Name:   ");
					sublabel.setHorizontalAlignment(JLabel.LEFT);

					JLabel sizelabel = new JLabel("Relation Data Size:");
					sizelabel.setHorizontalAlignment(JLabel.LEFT);

				 	JTextField supernamemessage = new JTextField(parentobject.set_name);
				 	JTextField subnamemessage = new JTextField(nodeobject.set_name);
				 	final JTextField sizemessage = new JTextField("123456");

					JPanel topWrapper = new JPanel();
					
					JPanel supersetWrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
					JPanel subsetWrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));
					JPanel tfWrapper = new JPanel(new FlowLayout(FlowLayout.LEFT));



					topWrapper.setLayout(new BoxLayout(topWrapper, BoxLayout.Y_AXIS));

					supersetWrapper.add(superlabel);
					supersetWrapper.add(supernamemessage);
					superlabel.setLabelFor(supernamemessage);

					subsetWrapper.add(sublabel);
					subsetWrapper.add(subnamemessage);
					sublabel.setLabelFor(subnamemessage);

					tfWrapper.add(sizelabel);
					tfWrapper.add(sizemessage);
					sizelabel.setLabelFor(sizemessage);

					topWrapper.add(supersetWrapper);
					topWrapper.add(subsetWrapper);
					// topWrapper.add(tfWrapper);

					listpanel.add(tfWrapper);

					canceldialogButton.addActionListener(new disposeButtonListener( (Window)subsetdialog) );


					// get subset relation data and show in framedisplay window
					readButton.addActionListener(new ActionListener() {

						public void actionPerformed( ActionEvent e ) {
							       // String data = "";
							       // JFrame framedisplay;
                                                               // JTextArea textarea = new JTextArea();
							int subset_rel_id;
							CollectionObject tmpcollection;
							String catname;
								// JPanel readpanel = new JPanel(new BorderLayout());

							catname = subsetlist.getSelectedItem();
							tmpcollection = (CollectionObject)(nodeobject.collections.get((Object)catname));
							subset_rel_id = saf_wrap.get_subset_relation(nodeobject.set_id, tmpcollection.coll_id);
								
                                                               // data = subset_data(subset_rel_id);
                                                               // textarea.append(data);
								// readpanel.add(textarea,BorderLayout.NORTH);

                                                               // framedisplay = createNewFrame("Subset Relation Data: ",(JComponent)readpanel);

								// add a button that disposes the window
								// cancelframeButton.addActionListener( new disposeButtonListener( (Window)framedisplay) );

								// readpanel.add(cancelframeButton, BorderLayout.SOUTH);

                                                               // framedisplay.show();

							SubsetData subsetdata = new SubsetData( po.set_id, nodeobject.set_id, catname, subset_rel_id );
							JButton subsetdata_disposebutton = new JButton("Dispose");
							JFrame subsetdataframe = new JFrame("Subset Data");
							subsetdata_disposebutton.addActionListener(new disposeButtonListener( (Window)subsetdataframe) );
							subsetdata.add(subsetdata_disposebutton, BorderLayout.SOUTH);
							subsetdataframe.getContentPane().add( subsetdata );
							subsetdataframe.pack();
							subsetdataframe.show();

						}
					});


					// update info in subset dialog when list items are clicked
					subsetlist.addMouseListener(new MouseAdapter() {
						
						public void mouseClicked( MouseEvent e ) {
							int subset_rel_id;
							int rel_size;
							Integer subset_rel_size;
							CollectionObject tmpcollection;
							String catname;

							catname = subsetlist.getSelectedItem();
							tmpcollection = (CollectionObject)(nodeobject.collections.get((Object)catname));
							subset_rel_id = saf_wrap.get_subset_relation(nodeobject.set_id, tmpcollection.coll_id);
							rel_size = saf_wrap.get_subset_relation_size(subset_rel_id);
							subset_rel_size = new Integer( rel_size );
							sizemessage.setText(subset_rel_size.toString());

						}
					});


					subsetdialog.getContentPane().add(buttonpanel, BorderLayout.SOUTH);
					subsetdialog.getContentPane().add(topWrapper, BorderLayout.NORTH);
					subsetdialog.pack();

					boolean showdialog=false;
					int test_rel_id = 0;
					CollectionObject testcollection;
                                        for( l_i = 0; l_i < parentobject.num_colls ; l_i++ ) {
                                                cat = saf_wrap.get_coll_cat_name(parentobject.set_id, l_i);

                                                if( nodeobject.collections.containsKey(cat) ) {
                                                        collectionobject = parentobject.get_collection(l_i);
							testcollection = (CollectionObject)(nodeobject.collections.get((Object)cat));
							test_rel_id = saf_wrap.get_subset_relation(nodeobject.set_id,testcollection.coll_id);
							// System.out.println(parentobject.toString() + ":" + nodeobject.toString() + ":" + cat + ":" + test_rel_id);

							if( test_rel_id != 0 ) {
							  subsetlist.add(collectionobject.coll_name);
							  showdialog=true;
							}
                                                }
                                        }

					if( showdialog == true ) {

						int subset_rel_id;
						int rel_size;
						Integer subset_rel_size;
						CollectionObject tmpcollection;
						String catname;

						subsetlist.select(0);
						catname = subsetlist.getItem(0);
						tmpcollection = (CollectionObject)(nodeobject.collections.get((Object)catname));
						subset_rel_id = saf_wrap.get_subset_relation(nodeobject.set_id, tmpcollection.coll_id);
						rel_size = saf_wrap.get_subset_relation_size(subset_rel_id);
						subset_rel_size = new Integer( rel_size );
						sizemessage.setText(subset_rel_size.toString());

						subsetdialog.setVisible(true);
						// listpanel.show();
					}
				} // end double-click left mouse button

                        } // end depth > 1


			// Middle Mouse button in Subset Trees - creates popup menu of collections -> fields
			if ( SwingUtilities.isMiddleMouseButton(e) && (nodeobject != null) )  {
				CollectionObject tmpcollectionobject;
				int menu_set_id, menu_coll_id;
				int attribute_count;
				boolean show_coll_popup = false;
				int menu_num_fields;
				JMenuItem field_menu_item;

				JPopupMenu popup = new JPopupMenu();
				menu_set_id = nodeobject.set_id;
				attribute_count = saf_wrap.attribute_count( menu_set_id );
				if( attribute_count > 0 ) {
				    field_menu_item = new JMenuItem(saf_wrap.get_set_name(menu_set_id) + " : " + attribute_count + " attributes");
				    field_menu_item.addActionListener( new AttrMenuListener( menu_set_id, attribute_count ));
				    popup.add(field_menu_item);
				    popup.addSeparator();
				}

				// field_menu_item = new JMenuItem("Categories -> Fields");
				// popup.add(field_menu_item);
				// popup.addSeparator();
				
				
				for( int i = 0; i < nodeobject.num_colls; i++ ) {
					menu_coll_id = i;
					tmpcollectionobject = nodeobject.get_collection(i);
					if( tmpcollectionobject != null ) {
		
						menu_num_fields = tmpcollectionobject.num_fields;	

						if( menu_num_fields > 0 ) {


						    JMenuItem catitem = new JMenuItem(tmpcollectionobject.coll_name + " : " + menu_num_fields + " fields");
						    catitem.addActionListener( new FieldMenuListener( menu_set_id, menu_coll_id, menu_num_fields, tmpcollectionobject.coll_name ));
						    popup.add(catitem);
						    show_coll_popup = true;
							
					   


// 							JMenu fieldmenu = new JMenu(tmpcollectionobject.coll_name + " : " + menu_num_fields + " fields");
// 							JMenuItem fieldmenutitle = new JMenuItem("Fields on " + tmpcollectionobject.coll_name);
// 							fieldmenu.add(fieldmenutitle);
// 							fieldmenu.addSeparator();
// 							for( int field = 0 ; field < menu_num_fields; field++ ) {

// 								int field_id = saf_wrap.get_field_rel_id(menu_set_id, menu_coll_id, field);
// 								JMenuItem fielditem = new JMenuItem(saf_wrap.get_field_menu_string(menu_set_id, field));
//                                                         	FieldPopupListener listener = new FieldPopupListener(menu_set_id, tmpcollectionobject.coll_id, field_id);
//                                                         	fielditem.addActionListener(listener);
// 								fieldmenu.add(fielditem);
// 							}

// 							popup.add(fieldmenu);
// 							show_coll_popup = true;
						}
						else {
						  JMenuItem catitem = new JMenuItem( tmpcollectionobject.coll_name + " : 0 fields"  );	
						  popup.add(catitem);
						  show_coll_popup = true;
						}


					}


				}
				if( show_coll_popup || attribute_count > 0 ) {
					popup.show(tree,e.getX(),e.getY());

				}

			} // end middle button


		} // end of mousePressed handler in tree

    public JFrame createNewFrame(String title, JComponent textarea) {

        JFrame newframe;
        // JTextArea textarea;

        // if(  newframe != null ) {
                // newframe.dispose();
                // newframe = null;
        // }
        // textarea = new JTextArea();
        JScrollPane scrollpane = new JScrollPane(textarea);
        Dimension d = new Dimension(450,240);
        scrollpane.setSize(d);
        newframe = new JFrame(title);
        newframe.getContentPane().add(scrollpane);
        newframe.setSize(d);
        return newframe;
    }

    public String set_info( int set_id ) {

	String set_info;

	set_info = saf_wrap.get_set_name( set_id );
	set_info +=  ":   silrole: " + saf_wrap.get_set_silrole(set_id);
	set_info +=  ", max_topo_dim: " + saf_wrap.get_set_max_topo_dim( set_id );
	set_info +=  ", extmode: " + saf_wrap.get_set_extmode(set_id);

	return set_info;

    }


    public String coll_info( int set_id, int coll_num ) {

	String coll_info;
	boolean is_decomp;

	coll_info = "\t" + saf_wrap.get_coll_cat_name( set_id, coll_num );
	coll_info +=  ":   celltype: " + saf_wrap.get_coll_celltype(set_id, coll_num);
	coll_info +=  ", count: " + saf_wrap.get_coll_count(set_id , coll_num);

	is_decomp = saf_wrap.get_coll_is_decomp( set_id, coll_num );
	if( is_decomp )
		coll_info +=  ", is_decomp: TRUE";
	else
		coll_info +=  ", is_decomp: FALSE";
		

	return coll_info;

    }
	
   public String subset_data( int rel_id )
   {

	int num, i;
	String s = "";

	num = saf_wrap.get_subset_relation_size(rel_id);
	
	for(i = 0; i < num; i++ ) {
		s += saf_wrap.get_subset_relation_data(rel_id,i) + ", ";

	}
	return s;

   }

}

class AttrListPanel extends JPanel {

    int l_set_id;

    public AttrListPanel( Vector attr_list, int set_id, JFrame attr_listframe ) {

	super(true);
	l_set_id = set_id;
	
	setLayout( new BoxLayout(this, BoxLayout.Y_AXIS));
	final JList jattrlist = new JList( attr_list );
	final Vector l_attr_list = attr_list;
	
	jattrlist.addMouseListener( new MouseAdapter() {
		public void mouseClicked( MouseEvent e ) {
		    if( e.getClickCount() == 2 ) {
			int item_num = jattrlist.getSelectedIndex();
			String attr_name = (String)l_attr_list.elementAt(item_num);
			// System.out.println( saf_wrap.attribute_string( l_set_id ) );
			// System.out.println(attr_name + " : " + saf_wrap.get_attr_data( l_set_id, attr_name, 1) );
			// AttrData attr_data = new AttrData( l_set_id, attr_name );

			JButton attrdata_disposebutton = new JButton("Dispose");

			JFrame attrdataframe = new JFrame( saf_wrap.get_set_name(l_set_id) + " attribute : " + attr_name  );

				attrdata_disposebutton.addActionListener(new disposeButtonListener( (Window)attrdataframe) );
				//	attr_data.add(attrdata_disposebutton, BorderLayout.SOUTH);

				// attrdataframe.getContentPane().add( attr_data );

				attrdataframe.pack();
				attrdataframe.show();

		    }
		}
	    });


				       
				      
    JScrollPane attrpane = new JScrollPane(jattrlist);

    JLabel attrlistheader = new JLabel("Attributes on :  " + saf_wrap.get_set_name(l_set_id)); 
    
    JPanel headerpanel = new JPanel();
    headerpanel.setLayout(new FlowLayout(FlowLayout.CENTER));
    headerpanel.add(attrlistheader);
    add(headerpanel);

    add(attrpane);

	 JButton disposebutton = new JButton("Dispose");
	 disposebutton.addActionListener(new disposeButtonListener( (Window)attr_listframe ));

    JLabel attrlistfooter = new JLabel("Double click attribute name for attribute contents");

    // add(fieldlistfooter);

    JPanel buttonpanel = new JPanel();
    JPanel footerpanel = new JPanel();
    footerpanel.setLayout(new FlowLayout(FlowLayout.CENTER));
    buttonpanel.setLayout(new FlowLayout(FlowLayout.CENTER));

    footerpanel.add(attrlistfooter);
    buttonpanel.add(disposebutton);
    add(footerpanel);
    add(buttonpanel);
    setPreferredSize(getPreferredSize());

    }

}


class FieldListPanel extends JPanel {
  
  int l_set_id;
  int l_coll_id;
  String l_coll_name;
 

  public FieldListPanel( Vector fieldlist, int set_id, int coll_id, String coll_name, JFrame fieldlistframe ) {
    super(true);
    l_set_id = set_id;
    l_coll_id = coll_id;
    l_coll_name = coll_name;


    setLayout(new BoxLayout(this,BoxLayout.Y_AXIS));

    final JList jfieldlist = new JList( fieldlist );
    


					// update info in subset dialog when list items are clicked
					jfieldlist.addMouseListener(new MouseAdapter() {
					 
						public void mouseClicked( MouseEvent e ) {
						  if( e.getClickCount() == 2 ) {
						    int field_num = jfieldlist.getSelectedIndex();
						    int field_id = saf_wrap.get_field_rel_id(l_set_id, l_coll_id, field_num);
						    FieldDialog field_dialog = new FieldDialog( l_set_id, l_coll_id, field_id);
						  }
						}
					});


				       
				      
    JScrollPane fieldpane = new JScrollPane(jfieldlist);

    JLabel fieldlistheader = new JLabel("Fields on :  " + saf_wrap.get_set_name(l_set_id) + " : " + l_coll_name); 
    
    JPanel headerpanel = new JPanel();
    headerpanel.setLayout(new FlowLayout(FlowLayout.CENTER));
    headerpanel.add(fieldlistheader);
    add(headerpanel);

    add(fieldpane);

	 JButton disposebutton = new JButton("Dispose");
	 disposebutton.addActionListener(new disposeButtonListener( (Window)fieldlistframe ));

    JLabel fieldlistfooter = new JLabel("Double click field name to get field information");

    // add(fieldlistfooter);

    JPanel buttonpanel = new JPanel();
    JPanel footerpanel = new JPanel();
    footerpanel.setLayout(new FlowLayout(FlowLayout.CENTER));
    buttonpanel.setLayout(new FlowLayout(FlowLayout.CENTER));

    footerpanel.add(fieldlistfooter);
    buttonpanel.add(disposebutton);
    add(footerpanel);
    add(buttonpanel);
    setPreferredSize(getPreferredSize());

   
  }
}

class AttrMenuListener implements ActionListener {
    int l_handle_id;
    int l_num_items;

    public AttrMenuListener( int menu_handle_id, int menu_num_items ) {
	super();
	
	l_handle_id = menu_handle_id;
	l_num_items = menu_num_items;

    }

    public void actionPerformed(ActionEvent e) {
	String attr_name;
	Vector attr_list = new Vector();

	for( int attr = 0 ; attr < l_num_items ; attr++ ) {
	    attr_name = saf_wrap.get_attr_name( l_handle_id, attr );
	    attr_list.add( attr_name );
	}
	// JFrame attrlistframe = new JFrame("Attributes on : " + saf_wrap.get_set_name(l_handle_id));

	AttrData attr_data = new AttrData( attr_list, l_handle_id );

			JButton attrdata_disposebutton = new JButton("Dispose");

			JFrame attrdataframe = new JFrame( saf_wrap.get_set_name(l_handle_id) + " Attributes(2)");

				attrdata_disposebutton.addActionListener(new disposeButtonListener( (Window)attrdataframe) );
				attr_data.add(attrdata_disposebutton, BorderLayout.SOUTH);

				attrdataframe.getContentPane().add( attr_data );

				attrdataframe.pack();
				attrdataframe.show();

	// attrlistframe.setContentPane(new AttrListPanel( attr_list, l_handle_id, attrlistframe ));
	// attrlistframe.pack();
	// attrlistframe.setVisible(true);
    }

}

class FieldMenuListener implements ActionListener {


  int l_set_id;
  int l_coll_id;
  int l_num_items;
  String l_coll_name;

	public FieldMenuListener( int menu_set_id, int menu_coll_id, int menu_num_items, String coll_name) {

		super();

		l_set_id = menu_set_id;
		l_coll_id = menu_coll_id;
		l_num_items = menu_num_items;
		l_coll_name = coll_name;
	}


	public void actionPerformed(ActionEvent e) {

	  int field_id;

	  Vector field_list = new Vector( );
	  for( int field = 0 ; field < l_num_items; field++ ) {
	    field_id = saf_wrap.get_field_rel_id(l_set_id, l_coll_id, field);
							
	    field_list.add(saf_wrap.get_field_menu_string(l_set_id, field_id));

	  }

	  JFrame fieldlistframe = new JFrame("Fields on :   " + saf_wrap.get_set_name(l_set_id) + " : " + l_coll_name);
	  fieldlistframe.setContentPane(new FieldListPanel( field_list, l_set_id, l_coll_id, l_coll_name, fieldlistframe ));

	  
	  // FieldListPanel fieldlistpanel = new FieldListPanel( field_list, l_set_id, l_coll_id, fieldlistframe );
	  // fieldlistframe.getContentPane().add( fieldlistpanel );

	  fieldlistframe.pack();

	  fieldlistframe.setVisible(true);


	}

}

    
  
