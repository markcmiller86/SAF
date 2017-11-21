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


public class SubsetMouseAdapter extends MouseAdapter
{

	JTree tree;

        public SubsetMouseAdapter( JTree mytree ) {
		super();
		tree = mytree;
		
        }

                public void mousePressed(MouseEvent e) {
                        String s = null;
                        SetObject nodeobject = null;
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
                        String field_info="";
                        int num_fields = 0;
                        JFrame framedisplay;
                        int result = -1;
                        String data = "";
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

                        if( depth >= 1 ) {
                                nodeobject = (SetObject)(node.getUserObject());
                                set_name = saf_wrap.get_set_name(nodeobject.set_id);
                                set_id = nodeobject.set_id;
                                System.out.println(saf_wrap.get_set_name(set_id) + " --> ");
                                if( depth >= 1 ) {
                                        DefaultMutableTreeNode parent = (DefaultMutableTreeNode)(node.getParent(
));
                                        parentobject = (SetObject)(parent.getUserObject());
                                        System.out.println(saf_wrap.get_set_name(parentobject.set_id));

                                        for( l_i = 0; l_i < parentobject.num_colls ; l_i++ ) {
                                                cat = saf_wrap.get_coll_cat_name(parentobject.set_id, l_i);

                                                if( nodeobject.collections.containsKey(cat) ) {
                                                        collectionobject = parentobject.get_collection(l_i);
                                                        System.out.println(collectionobject.coll_name);
                                                }

                                        }
                                }

                        }
                }



}

