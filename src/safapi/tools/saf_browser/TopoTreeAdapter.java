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


class TopoTreeAdapter extends MouseAdapter {

        JTextArea treedetail;
        JTree topotree;

        public TopoTreeAdapter(  JTextArea thetreedetail, JTree thetree ) {
                super();
                topotree = thetree;
                treedetail = thetreedetail;
        }

		public void mousePressed(MouseEvent e) {
			String s = null;
			TopoObject l_topoobject;
			TopoObject parent_object = null;
			int set_id=0, coll_id=0, topo_data_size = 0;
			int rel_id, topo_relation = 0;
			int result = 0;
			int depth = -1;
			String cat, data="";
			String coll_info="";
			JTextArea textarea;
			JFrame framedisplay;
			DefaultMutableTreeNode node = null;
			String psn="";
			String pcn="";
			String csn="";
			String ccn="";
			
			JTree t = (JTree)e.getSource();
			int row = t.getRowForLocation(e.getX(), e.getY());
                        TreePath path = t.getPathForRow(row);
                        // tree.setSelectionPath(path);


                        if( row >= 0 ) {
                                node = (DefaultMutableTreeNode)path.getLastPathComponent();
                                depth = node.getLevel() - 1 ;
                        }
                        else
                                row = -1;


			if( depth >= 0 ) {
				
				// TreePath path = t.getPathForRow(row);
				// topotree.setSelectionPath(path);
				// DefaultMutableTreeNode node = (DefaultMutableTreeNode)path.getLastPathComponent();
				l_topoobject = (TopoObject)(node.getUserObject());
				if( depth >= 1 ) {
					DefaultMutableTreeNode parent = (DefaultMutableTreeNode)(node.getParent());
					parent_object = (TopoObject)(parent.getUserObject());
					psn = saf_wrap.get_set_name( parent_object.set_id);
					pcn = saf_wrap.get_coll_cat_name( parent_object.set_id, parent_object.coll_id);
				}
				topo_relation = l_topoobject.rel_id;
				set_id = l_topoobject.set_id;
				coll_id = l_topoobject.coll_id;

				csn = saf_wrap.get_set_name( set_id );
				ccn = saf_wrap.get_coll_cat_name( set_id, coll_id );
				// coll_info = saf_wrap.coll_info(set_id,coll_id);

				// rel_id = saf_wrap.get_subset_relation(set_id, coll_id);
				// topo_relation = saf_wrap.get_topo_relation(set_id, coll_id);
				// topo_relation = saf_wrap.get_topo_domain_id(set_id, coll_id);
			}
			else {
				rel_id = 0;
				topo_relation = 0;
			}


			s = "";

			if (SwingUtilities.isLeftMouseButton(e)) {
				if( e.getClickCount() == 1) {
					s = coll_info;
					if( depth >= 0 )
						treedetail.append(s + "\n");
				}
			}
			if (SwingUtilities.isMiddleMouseButton(e)) {
				// s = "middle mouse button ";
				;
			}
			if (SwingUtilities.isRightMouseButton(e)) {
				// s = "right mouse button ";
				if( e.getClickCount() == 2) {

					if( topo_relation != 0 ) {

						String topo_relation_string = "";
						String topo_relation_type = "";
						topo_relation_string = saf_wrap.get_set_name(parent_object.set_id);
						topo_relation_string += ", " + saf_wrap.get_coll_cat_name(parent_object.set_id, parent_object.coll_id);
						topo_relation_string += " -> " + saf_wrap.get_set_name(set_id);
						topo_relation_string += ", " + saf_wrap.get_coll_cat_name(set_id, coll_id);
						topo_data_size = saf_wrap.get_topo_relation_size(topo_relation, 1);
						topo_relation_type = saf_wrap.get_topo_relation_type(topo_relation);

						// if( topo_relation_type.equals("DSL_HANDLE") ) {
						//  System.out.println("topo_relation type is DSL_HANDLE");
						// }

						JButton button = new JButton("data query");
						// String title = "Topo Relation";
						boolean read_data = false;
						if( topo_data_size > 100  ) {

							String message[] = {
								"Topology Relation Data Size: " + topo_data_size,
								" ",
								"Are you sure you want to read this data?",
								" ",
							};
							result = JOptionPane.showConfirmDialog(
								button,
								message,
								// title,
								topo_relation_string,
								JOptionPane.YES_NO_OPTION,
								JOptionPane.WARNING_MESSAGE);
	
							switch(result) {
								case JOptionPane.YES_OPTION:

									read_data = true;
									break;
							}
						}
						else
							if( topo_data_size > 0 )
								read_data = true;

						if( read_data == true ) {
						TopoData topodata = new TopoData( psn, pcn, 
										csn, ccn, 
										topo_relation );
						JButton topodata_disposebutton = new JButton("Dispose");

						JFrame topodataframe = new JFrame("Topology: " + psn + ":" + pcn + " -> " + 
										csn + ":" + ccn);
			
						topodata_disposebutton.addActionListener(new disposeButtonListener( (Window)topodataframe) );
						topodata.add(topodata_disposebutton, BorderLayout.SOUTH);

						topodataframe.getContentPane().add( topodata );
						topodataframe.pack();
						topodataframe.show();
						}

						
					}
				}
			}



			// System.out.println(s + "\n");
			// treedetail.append(s + "\n");
		}
}
