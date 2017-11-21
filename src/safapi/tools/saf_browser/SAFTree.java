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

/**
 * SAF JTree 
 *
 */
public class SAFTree extends SAFModule {
	// JEditorPane treedetail;

	static JDesktopPane desktop;
	static JTextArea treedetail;
	static JScrollPane detailpane;
	// JPanel detailpane;
	static String eventstring;
	static String topo_rel_string;
	static JScrollPane subsetpane;
	static JScrollPane topopane;
	static JScrollPane suitepane;
	static boolean first = true;
	static String saf_filename = null;
	static JSplitPane mainsplitpane;
	static SAFGUI safgui;


	static JTree tree = null;
	static JTree suitetree = null;
	static JTree topotree = null;
	static DefaultMutableTreeNode subset_lattice;
	static DefaultMutableTreeNode topology;
	static DefaultMutableTreeNode subset_collection;
	static DefaultMutableTreeNode topo_collection;
	static DefaultMutableTreeNode suite_lattice;
	static DefaultMutableTreeNode suite_collection;

        JTextField windowTitleField = new JTextField(getString("InternalSAFFrame.frame_label"));
        JLabel windowTitleLabel     = new JLabel(getString("InternalSAFFrame.title_text_field_label"));
        JCheckBox windowResizable   = new JCheckBox(getString("InternalSAFFrame.resizable_label"), true);
        JCheckBox windowClosable    = new JCheckBox(getString("InternalSAFFrame.closable_label"), true);
        JCheckBox windowIconifiable = new JCheckBox(getString("InternalSAFFrame.iconifiable_label"), true);
        JCheckBox windowMaximizable = new JCheckBox(getString("InternalSAFFrame.maximizable_label"), true);

	static int top_sets_id=-1;
	static int num_tops;
	static int num_top_colls;

	static int suite_sets_id=-1;
	static int num_suites;
	static int num_suite_colls;


	// enumeration button
	static JButton button = new JButton("Subset Lattice Dump");
	static JButton topo_button = new JButton("Categories");
	static JButton relations_button = new JButton("Field Templates");
	static JButton fields_button = new JButton("Fields");
	static JToolBar toolbar = new JToolBar();


    /**
     * main method allows us to run as a standalone saftree.
     */
    public static void main(String[] args) {
    RepaintManager.currentManager(null).setDoubleBufferingEnabled(false);
 	SAFTree saftree = new SAFTree(null);
	saftree.mainImpl();
    }

    /**
     * SAFTree Constructor
     */
    public SAFTree(SAFGUI saf_gui) {

	// Set the title for this saftree, and an icon used to represent this
	// saftree inside the SAFGUI app.
	super(saf_gui, "SAFTree", "toolbar/JTree.gif");
	initSAFTree(saf_gui);
    }

    public ImageIcon createImageIcon(String filename, String description) {
        String path = "/resources/images/" + filename;
        return new ImageIcon(getClass().getResource(path));
    }

    public ImageIcon createImageIcon(String filename) {
        String path = "/resources/images/" + filename;
        return new ImageIcon(getClass().getResource(path));
    }


    public static void initSAFTree(SAFGUI saf_gui) {

	safgui = saf_gui;
	MySharedLib mysharedlib = new MySharedLib("libsafgui.so");

        if ( saf_gui == null ) {
                 String saf_filepath = "/home/pkespen";
                 // String saf_filepath = "/homes/peter/saf/src/safapi/test";
                // String saf_filepath = "/homes/peter/java/gui_saf/newgui";
                // String saf_filename = "larry1w2.saf";
                // String saf_filename = "test_saf.saf";

                if( first ) {
                        // saf_filename = "larry1w_new.saf";
                        //saf_filename = "exo_basic_new.saf";
                        saf_filename = "birth_death.saf";
                        // saf_filename = "exo_par.saf";
                	//System.loadLibrary("safgui");
			mysharedlib.load();
                	saf_init.init();
                }
                else {
                        saf_filename = "test_saf.saf";
                }

                boolean dbOpened = false;


                // System.loadLibrary("safgui");
                // saf_init.init();
                top_sets_id = saf_wrap.open(saf_filepath, saf_filename,"r");

                // need to handle error condition if top_sets_id <= 0

        }
        else {
                top_sets_id = saf_gui.top_sets_id;
        }


        num_tops = saf_wrap.num_tops();
        num_top_colls = saf_wrap.get_num_colls(top_sets_id);

	num_suites = saf_wrap.num_suites();
	if( num_suites > 0 ) {
		suite_sets_id = saf_wrap.get_suite_sets_id();
        	num_suite_colls = saf_wrap.get_num_colls(suite_sets_id);
	}
	else
		suite_sets_id = 0;

        treedetail = new JTextArea();

        subsetpane = createSubsetTree(saf_gui);
        subsetpane.setBorder(BorderFactory.createTitledBorder("Mesh Subsets"));

	tree.setCellRenderer(new SubsetTreeRenderer());

        topopane = createTopoTree(saf_gui);
        topopane.setBorder(BorderFactory.createTitledBorder("Topology"));

        suitepane = createSuiteTree(saf_gui);
        suitepane.setBorder(BorderFactory.createTitledBorder("Suites"));
	suitetree.setCellRenderer(new SubsetTreeRenderer());


        JSplitPane ssplitpane = new JSplitPane(JSplitPane.VERTICAL_SPLIT, suitepane, topopane );
        ssplitpane.setContinuousLayout(true);
        ssplitpane.setOneTouchExpandable(false);
        ssplitpane.setDividerSize( 2 );
	ssplitpane.setResizeWeight(0.5);
	ssplitpane.setPreferredSize(new Dimension(200,100));
	ssplitpane.resetToPreferredSizes();
	ssplitpane.updateUI();

        JSplitPane treesplitpane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,subsetpane,ssplitpane);
        treesplitpane.setContinuousLayout(true);
        treesplitpane.setOneTouchExpandable(false);
        treesplitpane.setDividerSize( 2 );
	treesplitpane.setResizeWeight(0.75);
	treesplitpane.setPreferredSize(new Dimension(200,100));
	treesplitpane.resetToPreferredSizes();
	treesplitpane.updateUI();

	subsetpane.setPreferredSize(new Dimension(200,100));
	subsetpane.updateUI();

        detailpane = new JScrollPane(treedetail);
        detailpane.setBorder(BorderFactory.createTitledBorder("Detail"));
	detailpane.setPreferredSize(new Dimension(200,100));


        eventstring = "";
        topo_rel_string = "";

        if(first) {

        button.addActionListener(new ActionListener() {

                public void actionPerformed(ActionEvent e) {
                        Enumeration df = subset_lattice.depthFirstEnumeration();
                        Enumeration bf = subset_lattice.breadthFirstEnumeration();

                        while(df.hasMoreElements()) {
                                eventstring = df.nextElement().toString();
                                treedetail.append(eventstring + "\n");

                        }


                        while(bf.hasMoreElements()) {
                                eventstring = bf.nextElement().toString();
                                // System.out.println(eventstring);
                                // treedetail.append(eventstring + "\n");
                        }

                }

        });

        topo_button.addActionListener(new ActionListener() {

                public void actionPerformed(ActionEvent e) {

                        // treedetail.append(topo_rel_string + "\n");
                        closetree();

                }

        });

        relations_button.addActionListener(new ActionListener() {

                public void actionPerformed(ActionEvent e) {

                        // treedetail.append(topo_rel_string + "\n");
                        newtree(safgui);

                }

        });

        toolbar.add(button);
        // toolbar.add(topo_button);
        // toolbar.add(relations_button);
        // toolbar.add(fields_button);
        }


        mainsplitpane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,treesplitpane,detailpane);
        mainsplitpane.setContinuousLayout(true);
        mainsplitpane.setOneTouchExpandable(false);
        mainsplitpane.setDividerSize( 2 );
        mainsplitpane.setResizeWeight(0.4);
	mainsplitpane.setPreferredSize(new Dimension(500,500));


        getSAFPanel().setOpaque(false);
        getSAFPanel().add(mainsplitpane,BorderLayout.NORTH);
        // if( first ) {
        //         getSAFPanel().add(toolbar,BorderLayout.SOUTH);
        // }

        first = false;


    }


    public static DefaultMutableTreeNode addSubsetNode(SetObject parentobject, DefaultMutableTreeNode Parent, int coll_num, int level)
    {


	int i, j, k;
	int set_id, num_subsets, subsets, rel_id, rel_id2;
	int a_set, num_colls,sub_num_colls ;
	DefaultMutableTreeNode subset_node;
	CollectionObject collectionobject;
	SetObject setobject;
	String set_name;

	set_id = parentobject.set_id;
	subsets = saf_wrap.get_subsets(set_id, coll_num);

	if( subsets == 0 )
		return Parent; 


	num_subsets = saf_wrap.number_of_sets(subsets);

	int created_subset_node = 0;
	

	for( i = 0; i < num_subsets; i++ ) {
		a_set = saf_wrap.get_a_set(subsets,i);
		num_colls = saf_wrap.get_num_colls(a_set);
		set_name = saf_wrap.get_set_name(a_set);

		rel_id = saf_wrap.get_subset_relation(a_set,saf_wrap.lookup_coll_num(a_set,saf_wrap.get_coll_cat_name(set_id,coll_num)));


		// System.out.println("addSubsetNode: " + parentobject.toString() + ":" + set_name + ":" + saf_wrap.get_coll_cat_name(set_id,coll_num) + ":" + rel_id);




		setobject = parentobject.get_subset(a_set);
		subset_node = new DefaultMutableTreeNode();
		if( setobject == null ) {
		  created_subset_node = 1;
		  setobject = new SetObject(a_set, num_colls);
		}
		else {
		  created_subset_node = 0;
		}

		subset_node.setUserObject(setobject);

		for( j = 0; j < num_colls; j++ ) {

		  collectionobject = new CollectionObject(a_set,j);
		  collectionobject.rel_id = rel_id;
		  collectionobject.num_fields = saf_wrap.get_num_fields_on_coll(a_set,j);

		  if( rel_id != 0 ) {
		  setobject.add_collection(j,collectionobject);

		  if( saf_wrap.get_coll_cat_name(a_set,j).equals(saf_wrap.get_coll_cat_name(set_id,coll_num)) ){

		    parentobject.add_subset(setobject);
		    if( created_subset_node == 1 ) {
		      Parent.add(subset_node);
		    }

		  }
		  }
		}

		for( k = 0 ; k < num_colls; k++ ) {
			addSubsetNode( setobject, subset_node, k, level+1);  // recursive call to create a subset
		}

	}

	return(Parent);

    }


   public static JScrollPane createTopoTree(SAFGUI saf_gui) {

	topology = new DefaultMutableTreeNode("Topology");
	topo_collection = null;
	int i,j;
	int a_set, num_colls;
	String set_name;
	TopoObject topoobject;
	JScrollPane return_scrollpane;


	int all_sets_id = saf_wrap.get_all_sets_id();
	int num_all_sets = saf_wrap.number_of_sets(all_sets_id);

	for( i = 0; i < num_all_sets; i++ ) {

		a_set = saf_wrap.get_a_set(all_sets_id,i);
		set_name = saf_wrap.get_set_name(a_set);
		num_colls = saf_wrap.get_num_colls(a_set);
		for( j = 0; j < num_colls; j++ ) {
			topo_collection = new DefaultMutableTreeNode();
			topoobject = new TopoObject(a_set, j);
			topo_collection.setUserObject(topoobject);
			// topology.add(topo_collection);
			// addTopoNode(topo_collection,a_set, j);
			// this.addTopoNode(topology,a_set, j);
			addTopoNode(topology,a_set, j);
		}
	}


        final JTree topotree = new JTree(topology) {
            public Insets getInsets() {
                return new Insets(5,5,5,5);
            }
        };
        //topotree.setRootVisible(true);
        topotree.setRootVisible(false);
	topotree.putClientProperty("JTree.lineStyle","Angled");


        topotree.addMouseListener(new TopoTreeAdapter( treedetail,  topotree ));

	topotree.setCellRenderer( new TopoTreeRenderer() );

	return_scrollpane = new JScrollPane( topotree );
	// return_scrollpane.setPreferredSize( new Dimension(50,50) );
	return( return_scrollpane );

   }


   public static void addTopoNode( DefaultMutableTreeNode parent, int a_set, int coll_num)
   {

	int i,j;
	int num_topo_childs;
	int topo;
	int range_set, range_cat;
	String set_name;
	TopoObject topoobject;
	DefaultMutableTreeNode topo_node;
	DefaultMutableTreeNode child_node;


	num_topo_childs = 0;
	topo = 0;
	topo = saf_wrap.get_topos( a_set, coll_num );


	if( topo > 0 )  {
		topo_node = new DefaultMutableTreeNode();
		topoobject = new TopoObject(a_set,coll_num);
		topo_node.setUserObject(topoobject);
		parent.add(topo_node);
		range_set = saf_wrap.range_set(topo);
		range_cat = saf_wrap.range_coll(topo);
		set_name = saf_wrap.get_set_name(range_set);
		child_node = new DefaultMutableTreeNode();
		topoobject = new TopoObject(range_set,range_cat);
		topoobject.rel_id = topo;
		child_node.setUserObject(topoobject);
		topo_node.add(child_node);
		// this.addTopoNode(child_node, range_set, range_cat);
		addTopoNode(child_node, range_set, range_cat);

	}

   }


    public static JScrollPane createSubsetTree(SAFGUI saf_gui) {
        subset_lattice = new DefaultMutableTreeNode("Mesh Subsets");
	subset_collection = null;
	CollectionObject collectionobject;
	SetObject setobject;
	JScrollPane return_scrollpane;
	int i, top_num;
	int top_id;

	
	String set_name = saf_wrap.get_set_name(top_sets_id);

       // Go through all sets 1st set

	for( top_num = 0; top_num < num_tops; top_num++ ) {
		// String top_set_string = new String("Top Set ");
		// top_set_string += top_num+1;
		// DefaultMutableTreeNode top_set_node = new DefaultMutableTreeNode(top_set_string);
		// subset_lattice.add(top_set_node);
		top_id = saf_wrap.get_a_set(top_sets_id,top_num);
        	num_top_colls = saf_wrap.get_num_colls(top_id);

		setobject = new SetObject(top_id,num_top_colls);
		subset_collection = new DefaultMutableTreeNode();
		subset_collection.setUserObject(setobject);
		// top_set_node.add(subset_collection);
		subset_lattice.add(subset_collection);


        // Go through all collections in the top set
	for( i = 0; i < num_top_colls; i++ ) {
		collectionobject = new CollectionObject(top_id,i);
		collectionobject.num_fields = saf_wrap.get_num_fields_on_coll(top_id,i);

		setobject.add_collection(i,collectionobject);			

		addSubsetNode(setobject, subset_collection, i, 0 );
	}
	}


	tree = new JTree(subset_lattice) {
	    	public Insets getInsets() {
		return new Insets(5,5,5,5);
	    }
	};

	tree.setRootVisible(false);
	tree.putClientProperty("JTree.lineStyle","Angled");


	tree.addMouseListener(new SubsetTreeAdapter( treedetail,  tree )); 

	return_scrollpane = new JScrollPane(tree);
	// return_scrollpane.setPreferredSize(new Dimension(400,400));
	return return_scrollpane;
	}


    public static String set_info( int set_id ) {

	String set_info;

	set_info = saf_wrap.get_set_name( set_id );
	set_info +=  ":   silrole: " + saf_wrap.get_set_silrole(set_id);
	set_info +=  ", max_topo_dim: " + saf_wrap.get_set_max_topo_dim( set_id );
	set_info +=  ", extmode: " + saf_wrap.get_set_extmode(set_id);

	return set_info;

    }

    public static String coll_info( int set_id, int coll_num ) {

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
	

    public static JScrollPane createSuiteTree(SAFGUI saf_gui) {
        suite_lattice = new DefaultMutableTreeNode("Mesh Subsets");
	suite_collection = null;
	CollectionObject suiteobject;
	SetObject setobject;
	JScrollPane return_scrollpane;
	int i, suite_num;
	int suite_id;
	int suite_sets_id;


	if( num_suites <= 0 ) {
		suitetree = new JTree(new DefaultMutableTreeNode("empty suite tree"));
		return new JScrollPane(suitetree);
	}
		
	
	suite_sets_id = saf_wrap.get_suite_sets_id();	
	String set_name = saf_wrap.get_set_name(suite_sets_id);

	for( suite_num = 0; suite_num < num_suites; suite_num++ ) {
		// String suite_set_string = new String("Suites");
		// suite_set_string += suite_num+1;
		// DefaultMutableTreeNode suite_set_node = new DefaultMutableTreeNode(suite_set_string);
		// suite_lattice.add(suite_set_node);
		suite_id = saf_wrap.get_a_set(suite_sets_id,suite_num);
        	num_suite_colls = saf_wrap.get_num_colls(suite_id);

		setobject = new SetObject(suite_id, num_suite_colls);
		
		suite_collection = new DefaultMutableTreeNode();
		suite_collection.setUserObject(setobject);
		// suite_set_node.add(suite_collection);
		suite_lattice.add(suite_collection);

	for( i = 0; i < num_suite_colls; i++ ) {
	       	suiteobject = new CollectionObject(suite_id,i);
		suiteobject.num_fields = saf_wrap.get_num_fields_on_coll(suite_id,i);
		setobject.add_collection( i, suiteobject);
		// addSubsetNode(setobject, suite_collection, i, 0 );
	}
	}


	suitetree = new JTree(suite_lattice) {
	    	public Insets getInsets() {
		return new Insets(5,5,5,5);
	    }
	};

	suitetree.setRootVisible(false);
	suitetree.putClientProperty("JTree.lineStyle","Angled");

	suitetree.addMouseListener(new SubsetTreeAdapter(treedetail, suitetree));

	return_scrollpane = new JScrollPane(suitetree);
	// return_scrollpane.setPreferredSize(new Dimension(400,400));
	return return_scrollpane;
	}

  public static void rebuildTree() {

	;
  }


  public static void closetree() {
        getSAFPanel().remove(mainsplitpane);
        // getSAFPanel().remove(toolbar);
	getSAFPanel().setVisible(false);
        getSAFPanel().repaint();
	if( safgui == null ) {
		saf_wrap.close(top_sets_id);
	}
        getSAFFrame().getContentPane().remove(getSAFPanel());
	getSAFFrame().repaint();
  }

  public static void closetree(JPanel SAFPanel) {
        getSAFPanel().remove(mainsplitpane);
        // getSAFPanel().remove(toolbar);
	getSAFPanel().setVisible(false);
        getSAFPanel().repaint();
	if( safgui == null ) {
		saf_wrap.close(top_sets_id);
	}
        SAFPanel.remove(getSAFPanel());
	SAFPanel.repaint();
  }
  

  public static void newtree(SAFGUI saf_gui) {
        initSAFTree(saf_gui);
        getSAFFrame().getContentPane().add(getSAFPanel(),BorderLayout.CENTER);
        getSAFFrame().repaint();
	getSAFPanel().setVisible(true);
	getSAFPanel().repaint();
  }

  public static void newtree(SAFGUI saf_gui, JPanel SAFPanel) {
        initSAFTree(saf_gui);
        SAFPanel.add(getSAFPanel(),BorderLayout.CENTER);
	getSAFPanel().setVisible(true);
        SAFPanel.repaint();
	SAFPanel.setVisible(true);
	getSAFPanel().repaint();
  }


}


class disposeButtonListener implements ActionListener {

	Window mywindow;

	public disposeButtonListener( Window thewindow ) {
		super();
		mywindow = thewindow;
	}

        public void actionPerformed( ActionEvent e ) {
		mywindow.dispose();
	}

}

class SubsetData extends JPanel {


	Integer int_value;

	public SubsetData( int parent_id, int subset_id, String catname, int subset_relation ) {

		super();

		int i,j, rownumber, colnumber;
		int data_size;
		String field_name, rowheader;
		String rel_type;

		String parentname = saf_wrap.get_set_name( parent_id );
		String subsetname = saf_wrap.get_set_name( subset_id );

		rel_type = saf_wrap.get_subset_relation_type( subset_relation );
		// System.out.println(rel_type);

		int columns = 2;
		data_size = saf_wrap.get_subset_relation_size( subset_relation );

		Object[] rowdata = new Object[columns];
	
		DefaultTableModel model = new DefaultTableModel();
		JTable table = new JTable(model);

		model.addColumn(subsetname + ":" + catname);
		model.addColumn(parentname + ":" + catname);
		
		for( i = 0 ; i < data_size; i++ ) {
			rowdata[0] = new Integer(i);
			rowdata[1] =  new Integer(saf_wrap.get_subset_relation_data( subset_relation, i));
			model.addRow(rowdata);
		}
		 
		JScrollPane rawdataarea = new JScrollPane( table );

		this.setLayout( new BorderLayout());

		this.add(rawdataarea, BorderLayout.NORTH);

	}
}

class TopoData extends JPanel {


	final static int A = 0;
	final static int B = 1;
	Integer int_value;

	public TopoData( String psn, String pcn, String csn, String ccn, int topo_relation ) {

		super();

		int i,j, rownumber, colnumber, coeff_assoc_id, ratio;
		String coeff_assoc;
		int a_data_size;
		int b_data_size;
		String data_string="";
		String field_name, rowheader;

		a_data_size = saf_wrap.get_topo_relation_size( topo_relation, A );

		int[] ratios = new int[a_data_size];
		int max = 0;
		if( a_data_size > 1 ) {
			for( i = 0 ; i < a_data_size; i++ ) {
				ratios[i] =  saf_wrap.get_topo_relation_data( topo_relation, i, A);
				if (ratios[i] > max)
					max = ratios[i];
			}
		}
		else {
			ratios[0] =  saf_wrap.get_topo_relation_data( topo_relation, 0, A);
			max = ratios[0];
		}

		// ratio = saf_wrap.get_topo_a_relation_data(topo_relation,0);

		// int columns = ratio+1;
		int columns = max+1;

		Object[] rowdata = new Object[columns];
	
		DefaultTableModel model = new DefaultTableModel();
		JTable table = new JTable(model);

		model.addColumn(psn + ": " + pcn );
		for( i = 0; i < max; i++ ) {
			// colnumber = i+1;
			model.addColumn(csn + ": " + ccn );
		}
		

		 rownumber = 1;	
		 b_data_size = saf_wrap.get_topo_relation_size( topo_relation, B );
		 if( b_data_size > 0 ) {
		 	for( i = 0; i < b_data_size;  ) {
				rowheader = "" + rownumber;
				rowdata[0] = pcn + " " + rowheader;

				if( a_data_size > 1 )
					ratio = ratios[rownumber-1];
				else
					ratio = max;

				for( j = 0; j < ratio ; j++ ) {
					int_value = new Integer( saf_wrap.get_topo_relation_data( topo_relation, i, B));
					i++;
					rowdata[j+1] = int_value.toString();
				}
				while( j < (max-1) ) {
					rowdata[j+1] = " ";
					j++;
				}
				model.addRow(rowdata);
				rownumber++;
		 	}
		 }
	
		TableCellRenderer topocellrenderer = new TopoCellRenderer();	

	 	for( i = 1 ; i <= max; i++ ) {
			table.setDefaultRenderer( table.getColumnClass(i), topocellrenderer);   
		}

		JScrollPane rawdataarea = new JScrollPane( table );

		this.setLayout( new BorderLayout());

		this.add(rawdataarea, BorderLayout.NORTH);

	}

}

class CollectionObject {

	int set_id;
	int coll_id;
	int rel_id;
	int num_fields;
	int num_subsets;
	String coll_name;

  public CollectionObject(int s_id, int c_id)
  {

	int i;

	set_id = s_id;
	coll_id = c_id;
	rel_id = 0;
	num_fields = 0;
	coll_name = saf_wrap.get_coll_cat_name(s_id, c_id);
  }

  public String toString() {

	String result;

	result = saf_wrap.get_set_name(set_id);
	result += ", " + this.coll_name;

	return(result);
  }

}


class SetObject {

	int set_id;
	int num_colls;
	int num_subsets;
	String set_name;
	Hashtable collections;
	Hashtable subsets;


  public SetObject(int s_id, int n_c)
  {

	int i;

	set_id = s_id;
	num_colls = n_c;
	set_name = saf_wrap.get_set_name(s_id);
	collections = new Hashtable();
	subsets = new Hashtable();
	num_subsets = 0;
  }

  public String toString() {

	String result;

	result = saf_wrap.get_set_name(set_id);

	return(result);
  }

  public void add_collection(int c_n, CollectionObject collectionobject ) {

	String coll_name;

	coll_name = saf_wrap.get_coll_cat_name(this.set_id, c_n);
	if (coll_name != null)collections.put(coll_name, collectionobject);

  }

  public CollectionObject get_collection( int c_n ) {

	String coll_name;
	CollectionObject collobj;

	coll_name = saf_wrap.get_coll_cat_name(this.set_id, c_n);
	collobj = (CollectionObject)collections.get(coll_name);
	return collobj;
  }

  public boolean add_subset( SetObject setobject ) {
//
// Use the name of the set as a key to store incoming setobject into a hash table. 
//
	String set_name;
        String set_id_c;

        set_id_c = Integer.toString(set_id);
	set_name = saf_wrap.get_set_name(setobject.set_id);

	if( ! this.subsets.containsKey(set_name) ) {

		this.subsets.put(set_name,setobject);
		num_subsets = num_subsets + 1;
		return(true);
	}
	else {
		this.subsets.put(set_name,setobject);
	}
	return(false);
	
  }

  public SetObject get_subset( int set_id) {
//
// Use the name of the set as a key to retrieve a setobject from a hash table. 
//
	SetObject returnobject = null;

	String set_name;
        String set_id_c;

        set_id_c = Integer.toString(set_id);
	set_name = saf_wrap.get_set_name(set_id);

	if( this.subsets.containsKey(set_name) )  {
		return( (SetObject)subsets.get(set_name) );
	}
	else {
		return( (SetObject)null );
	}
  }


} // SetObject


class TopoObject {

	int set_id;
	int coll_id;
	int rel_id;

  public TopoObject( int i, int j) {
	set_id = i;
	coll_id = j;
	rel_id = 0;
  }
	
  public String toString() {

	String result;

	result = saf_wrap.get_set_name(set_id);
	result += ", " + saf_wrap.get_coll_cat_name(set_id,coll_id);

	return(result);

  }

}


class FieldPopupListener implements ActionListener {

		int set_id;
		int coll_id;
		int field_id;

	public FieldPopupListener( int s_id , int c_id, int f_id ) {

		super();
		set_id = s_id;
		coll_id = c_id;
		field_id = f_id;
	}


	public void actionPerformed(ActionEvent e) {

		FieldDialog fielddialog = new FieldDialog( set_id, coll_id, field_id );

	}

}
