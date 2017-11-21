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
/*
 * @(#)SAFGUI.java	
 */

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.border.*;
import javax.swing.colorchooser.*;
import javax.swing.filechooser.*;
import javax.accessibility.*;
import javax.swing.plaf.metal.DefaultMetalTheme;
import javax.swing.plaf.metal.MetalLookAndFeel;
import java.lang.reflect.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.*;
import java.io.*;
import java.applet.*;
import java.net.*;

/**
 * A SAFGUI for accessing SAF databases
 *
 */
public class SAFGUI extends JPanel {

    public static String saf_filename = "default.saf";
    public static String saf_filepath;
    public static int top_sets_id = -1;
    public static int all_sets_id = -1;
    public String tree_string;
    public static SAFGUI safgui;
    public static SAFTree saf_tree = null;
    private static boolean dbOpened = false;
    public static int startup = 0;
    AbstractAction OpenFile = null;

    static MySharedLib mysharedlib = new MySharedLib("libsafgui.so");

    String[] saf_mods = {
      // "SAFTree"
    };

    void loadModules() {
	for(int i = 0; i < saf_mods.length;) {
            if(isApplet() && saf_mods[i].equals("FileChooserMod")) {
	       // don't load the file chooser mod if we are
               // an applet
	    } else {
	       loadModule(saf_mods[i]);
            }
	    i++;
	}
    }

    // Possible Look & Feels
    private String mac      = "com.sun.java.swing.plaf.mac.MacLookAndFeel";
    private String metal    = "javax.swing.plaf.metal.MetalLookAndFeel";
    private String motif    = "com.sun.java.swing.plaf.motif.MotifLookAndFeel";
    private String windows  = "com.sun.java.swing.plaf.windows.WindowsLookAndFeel";

    // The current Look & Feel
    private String currentLookAndFeel = metal;

    // List of saf_mods
    private Vector SAFVector = new Vector();
    private Vector ButtonVector = new Vector();

    // private FileDialog fileDialog = null;
    private JFileChooser  fileDialog = null;

    // The preferred size of the module
    private int PREFERRED_WIDTH = 1050;
    private int PREFERRED_HEIGHT = 680;
    
    // Box spacers
    private Dimension HGAP = new Dimension(1,5);
    private Dimension VGAP = new Dimension(5,1);

    // Resource bundle for internationalized and accessible text
    private ResourceBundle bundle = null;

    // A place to hold on to the visible module
    private SAFModule currentModule = null;
    private JPanel SAFPanel = null;

    // About Box
    private JDialog aboutBox = null;

    // Status Bar
    private JTextField statusField = null;

    // Tool Bar
    private ToggleButtonToolBar toolbar = null;
    private ButtonGroup toolbarGroup = new ButtonGroup();

    // Menus
    private JMenuBar menuBar = null;
    private JMenu themesMenu = null;
    private JMenu fileMenu = null;
    private ButtonGroup lafMenuGroup = new ButtonGroup();
    private ButtonGroup themesMenuGroup = new ButtonGroup();

    // Used only if saf_gui is an application 
    private static JFrame frame = null;
    private JWindow splashScreen = null;

    // Used only if SAFGUI is an applet 
    private SAFGUIApplet applet = null;

    // For debugging
    private boolean DEBUG = true;
    private int debugCounter = 0;

    // The tab pane that holds the module
    private JTabbedPane tabbedPane = null;

    // private JEditorPane SAFSrcPane = null;
    public JScrollPane SAFSrcPane = null;


    private JLabel splashLabel = null;

    // contentPane cache, saved from the applet or application frame
    Container contentPane = null;



    /**
     * SAFGUI Constructor
     */
    public SAFGUI(SAFGUIApplet applet) {
	// Note that the applet may null if this is started as an application
	this.applet = applet;

	safgui = this;

	// setLayout(new BorderLayout());
	setLayout(new BorderLayout());

	// set the preferred size of the gui
	setPreferredSize(new Dimension(PREFERRED_WIDTH,PREFERRED_HEIGHT));

	// use a thread to put up a splash screen
	createSplashScreen();

	// do the following on the gui thread
	SwingUtilities.invokeLater(new Runnable() {
	     public void run() {
		showSplashScreen();
	     }
	 });
	    
	initializeGUI();
	// preloadFirstModule();

	// Show the module and take down the splash screen. Note that
	// we again must do this on the GUI thread using invokeLater.
	SwingUtilities.invokeLater(new Runnable() {
	    public void run() {
		hideSplash();
		showSAFGUI();
	    }
	});

	// Start loading the rest of the gui in the background
		ModuleLoadThread moduleLoader = new ModuleLoadThread(this);
		moduleLoader.start();
    }


    /**
     * SAFGUI Main. Called only if we're an application, not an applet.
     */
    public static void main(String[] args) {
	RepaintManager.currentManager(null).setDoubleBufferingEnabled(false);
	frame = createFrame();
	mysharedlib.load();
	saf_init.init();
	SAFGUI saf_gui = new SAFGUI(null);
    }




    // *******************************************************
    // *************** SAFModule Loading Methods ******************
    // *******************************************************
    
        
    public void initializeGUI() {

	OpenFile = new OpenAction(this);

	JPanel top = new JPanel();
	top.setLayout(new BorderLayout());
	add(top, BorderLayout.NORTH);

	menuBar = createMenus();
	top.add(menuBar, BorderLayout.NORTH);

	JPanel toolbarPanel = new JPanel();
	toolbarPanel.setLayout(new BorderLayout());
	toolbar = new ToggleButtonToolBar();
	toolbarPanel.add(toolbar, BorderLayout.CENTER);
	top.add(toolbarPanel, BorderLayout.SOUTH);

	tabbedPane = new JTabbedPane();
	// add(tabbedPane, BorderLayout.CENTER);
	tabbedPane.getModel().addChangeListener(new TabListener());

	statusField = new JTextField("");
	statusField.setEditable(false);
	add(statusField, BorderLayout.SOUTH);
	
	SAFPanel = new JPanel();
	SAFPanel.setLayout(new BorderLayout());
	SAFPanel.setBorder(new EtchedBorder());
	tabbedPane.addTab("Hi There!", SAFPanel);
	
	// Add html src code viewer 
	// SAFSrcPane = new JEditorPane("text/html", getString("SourceCode.loading"));
	// SAFSrcPane.setEditable(false);
	GetFile html_file = new GetFile("safapi.html");
   	SAFSrcPane = html_file.html;
	SAFSrcPane.setPreferredSize(new Dimension(100,100));

 
	// JPanel p = new JPanel();
	// p.setLayout(new BorderLayout() );
	// p.add(SAFSrcPane, BorderLayout.CENTER);
    
	// JScrollPane scroller = new JScrollPane();
	// scroller.getViewport().add(p);
    
	// tabbedPane.addTab( getString("TabbedPane.src_label"), null, // scroller, SAFSrcPane, getString("TabbedPane.src_tooltip"));
	tabbedPane.addTab( getString("TabbedPane.src_label"), null, SAFSrcPane, getString("TabbedPane.src_tooltip"));
	OpenFile.actionPerformed(null);
    }

    SAFModule currentSAFTab = null;
    class TabListener implements ChangeListener {
	public void stateChanged(ChangeEvent e) {
	    SingleSelectionModel model = (SingleSelectionModel) e.getSource();
	    boolean srcSelected = model.getSelectedIndex() == 1;
	    if(currentSAFTab != currentModule && SAFSrcPane != null && srcSelected) {
		// SAFSrcPane.setText(getString("SourceCode.loading"));
		repaint();
	    }
	    if(currentSAFTab != currentModule && srcSelected) {
		currentSAFTab = currentModule;
		setSourceCode(currentModule);
	    } 
	}
    }


    /**
     * Create menus
     */

    public JMenuBar createMenus() {
	JMenuItem mi;
	// ***** create the menubar ****
	JMenuBar menuBar = new JMenuBar();
	menuBar.getAccessibleContext().setAccessibleName(
	    getString("MenuBar.accessible_description"));

	// ***** create File menu 
	JMenu fileMenu = (JMenu) menuBar.add(new JMenu(getString("FileMenu.file_label")));
        fileMenu.setMnemonic(getMnemonic("FileMenu.file_mnemonic"));
	fileMenu.getAccessibleContext().setAccessibleDescription(getString("FileMenu.accessible_description"));

	createMenuItem(fileMenu, "FileMenu.about_label", "FileMenu.about_mnemonic",
		       "FileMenu.about_accessible_description", new AboutAction(this));

        fileMenu.addSeparator();

	JMenuItem open_item = createMenuItem(fileMenu, "FileMenu.open_label", "FileMenu.open_mnemonic",
		       "FileMenu.open_accessible_description", OpenFile);

	createMenuItem(fileMenu, "FileMenu.close_label", "FileMenu.close_mnemonic",
		       "FileMenu.close_accessible_description", new CloseAction(this));


	if(!isApplet()) {
	    fileMenu.addSeparator();
	    
	    createMenuItem(fileMenu, "FileMenu.exit_label", "FileMenu.exit_mnemonic",
			   "FileMenu.exit_accessible_description", new ExitAction(this));
	}


	// ***** create laf switcher menu 
	JMenu lafMenu = (JMenu) menuBar.add(new JMenu(getString("LafMenu.laf_label")));
        lafMenu.setMnemonic(getMnemonic("LafMenu.laf_mnemonic"));
	lafMenu.getAccessibleContext().setAccessibleDescription(
	    getString("LafMenu.laf_accessible_description"));

	mi = createLafMenuItem(lafMenu, "LafMenu.java_label", "LafMenu.java_mnemonic",
		       "LafMenu.java_accessible_description", metal);
	mi.setSelected(true); // this is the default l&f

	createLafMenuItem(lafMenu, "LafMenu.mac_label", "LafMenu.mac_mnemonic",
		       "LafMenu.mac_accessible_description", mac);

	createLafMenuItem(lafMenu, "LafMenu.motif_label", "LafMenu.motif_mnemonic",
		       "LafMenu.motif_accessible_description", motif);

	createLafMenuItem(lafMenu, "LafMenu.windows_label", "LafMenu.windows_mnemonic",
		       "LafMenu.windows_accessible_description", windows);


	// ***** create themes menu 
	themesMenu = (JMenu) menuBar.add(new JMenu(getString("ThemesMenu.themes_label")));
        themesMenu.setMnemonic(getMnemonic("ThemesMenu.themes_mnemonic"));
	themesMenu.getAccessibleContext().setAccessibleDescription(
	    getString("ThemesMenu.themes_accessible_description"));

	mi = createThemesMenuItem(themesMenu, "ThemesMenu.default_label", "ThemesMenu.default_mnemonic",
		       "ThemesMenu.default_accessible_description", new DefaultMetalTheme());
	mi.setSelected(true); // This is the default theme
	
	createThemesMenuItem(themesMenu, "ThemesMenu.aqua_label", "ThemesMenu.aqua_mnemonic",
		       "ThemesMenu.aqua_accessible_description", new AquaTheme());

	createThemesMenuItem(themesMenu, "ThemesMenu.charcoal_label", "ThemesMenu.charcoal_mnemonic",
		       "ThemesMenu.charcoal_accessible_description", new CharcoalTheme());

	createThemesMenuItem(themesMenu, "ThemesMenu.contrast_label", "ThemesMenu.contrast_mnemonic",
		       "ThemesMenu.contrast_accessible_description", new ContrastTheme());

	createThemesMenuItem(themesMenu, "ThemesMenu.emerald_label", "ThemesMenu.emerald_mnemonic",
		       "ThemesMenu.emerald_accessible_description", new EmeraldTheme());

	createThemesMenuItem(themesMenu, "ThemesMenu.ruby_label", "ThemesMenu.ruby_mnemonic",
		       "ThemesMenu.ruby_accessible_description", new RubyTheme());

	return menuBar;
    }



    /**
     * Creates a generic menu item
     */
    public JMenuItem createMenuItem(JMenu menu, String label, String mnemonic,
			       String accessibleDescription, Action action) {
        JMenuItem mi = (JMenuItem) menu.add(new JMenuItem(getString(label)));
	mi.setMnemonic(getMnemonic(mnemonic));
	mi.getAccessibleContext().setAccessibleDescription(getString(accessibleDescription));
	mi.addActionListener(action);
	if(action == null) {
	    mi.setEnabled(false);
	}
	return mi;
    }


    /**
     * Creates a JRadioButtonMenuItem for the Themes menu
     */
    public JMenuItem createThemesMenuItem(JMenu menu, String label, String mnemonic,
			       String accessibleDescription, DefaultMetalTheme theme) {
        JRadioButtonMenuItem mi = (JRadioButtonMenuItem) menu.add(new JRadioButtonMenuItem(getString(label)));
	themesMenuGroup.add(mi);
	mi.setMnemonic(getMnemonic(mnemonic));
	mi.getAccessibleContext().setAccessibleDescription(getString(accessibleDescription));
	mi.addActionListener(new ChangeThemeAction(this, theme));

	return mi;
    }


    /**
     * Creates a JRadioButtonMenuItem for the Look and Feel menu
     */
    public JMenuItem createLafMenuItem(JMenu menu, String label, String mnemonic,
			       String accessibleDescription, String laf) {
        JMenuItem mi = (JRadioButtonMenuItem) menu.add(new JRadioButtonMenuItem(getString(label)));
	lafMenuGroup.add(mi);
	mi.setMnemonic(getMnemonic(mnemonic));
	mi.getAccessibleContext().setAccessibleDescription(getString(accessibleDescription));
	mi.addActionListener(new ChangeLookAndFeelAction(this, laf));

	mi.setEnabled(isAvailableLookAndFeel(laf));

	return mi;
    }


    /**
     * Load the first module. This is done separately from the remaining modules
     * so that we can get SAFGUI up and available to the user quickly.
     */
    public void preloadFirstModule() {
	// SAFModule saf_mod = addModule(new InternalSAFFrame(this));
	SAFModule saf_mod = addModule(new SAFTree(this));
	setModule(saf_mod);
    }



    /**
     * Add a Module  to the toolbar
     */

    public SAFModule addModule(SAFModule saf_mod) {
	SAFVector.addElement(saf_mod);
	// do the following on the gui thread
	SwingUtilities.invokeLater(new SAFRunnable(this, saf_mod) {
	    public void run() {
		SwitchToSAFAction action = new SwitchToSAFAction(saf_gui, (SAFModule) obj);
		JToggleButton tb = saf_gui.getToolBar().addToggleButton(action);
		ButtonVector.addElement(tb);
		saf_gui.getToolBarGroup().add(tb);
		if(saf_gui.getToolBarGroup().getSelection() == null) {
		    tb.setSelected(true);
		}
		tb.setText(null);
		tb.setToolTipText(((SAFModule)obj).getToolTip());

		// if(saf_mods[saf_mods.length-1].equals(obj.getClass().getName())) {
		//     setStatus("");
		// } 
		  
	    }
	});
	return saf_mod;
    }


    /**
     * Sets the current saf_mod
     */

    public void setModule(SAFModule saf_mod) {
	currentModule = saf_mod;

	SAFPanel.removeAll();
	SAFPanel.add(saf_mod.getSAFPanel(), BorderLayout.CENTER);

	tabbedPane.setSelectedIndex(0);
	tabbedPane.setTitleAt(0, saf_mod.getName());
	// tabbedPane.setToolTipTextAt(0, saf_mod.getToolTip());
    }



    /**
     * Bring up the SAFGUI module by showing the frame (only
     * applicable if coming up as an application, not an applet);
     */

    public void showSAFGUI() {
	if(!isApplet() && getFrame() != null) {
	    // put saf_gui in a frame and show it
	    JFrame f = getFrame();
	    f.setTitle(getString("Frame.title"));
	    f.getContentPane().add(this, BorderLayout.CENTER);
	    f.pack();
	    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
	    getFrame().setLocation(
		screenSize.width/2 - f.getSize().width/2,
		screenSize.height/2 - f.getSize().height/2);
	    getFrame().show();
	} 
    }

    /**
     * Show the spash screen while the rest loads
     */
    public void createSplashScreen() {
	splashLabel = new JLabel(createImageIcon("Splash.gif", "Splash.accessible_description"));
	
	if(!isApplet()) {
	    splashScreen = new JWindow();
	    splashScreen.getContentPane().add(splashLabel);
	    splashScreen.pack();
	    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
	    splashScreen.setLocation(screenSize.width/2 - splashScreen.getSize().width/2,
				     screenSize.height/2 - splashScreen.getSize().height/2);
	} 
    }


    public void showSplashScreen() {
	if(!isApplet()) {
	    splashScreen.show();
	} else {
	    add(splashLabel, BorderLayout.CENTER);
	    validate();
	    repaint();
	}
    }


    /**
     * pop down the spash screen
     */
    public void hideSplash() {
	if(!isApplet()) {
	    splashScreen.setVisible(false);
	    splashScreen = null;
	    splashLabel = null;
	}
    }


    // *******************************************************
    // ****************** Utility Methods ********************
    // *******************************************************


    /**
     * Loads a module from a classname
     */
    void loadModule(String classname) {
	setStatus(getString("Status.loading") + getString(classname + ".name"));
	SAFModule saf_mod = null;
	try {
	    Class SAFClass = Class.forName(classname);
	    Constructor SAFConstructor = SAFClass.getConstructor(new Class[]{SAFGUI.class});
	    saf_mod = (SAFModule) SAFConstructor.newInstance(new Object[]{this});
	    if( classname.equals("SAFTree") ) {
	    	saf_tree = (SAFTree)saf_mod;
	    }
	    addModule(saf_mod);
	} catch (Exception e) {
	    System.out.println("Error occurred loading module : " + classname);
	}
	add(tabbedPane, BorderLayout.CENTER);

    }

    
    /**
     * A utility function that layers on top of the LookAndFeel's
     * isSupportedLookAndFeel() method. Returns true if the LookAndFeel
     * is supported. Returns false if the LookAndFeel is not supported
     * and/or if there is any kind of error checking if the LookAndFeel
     * is supported.
     *
     * The L&F menu will use this method to detemine whether the various
     * L&F options should be active or inactive.
     *
     */

     protected boolean isAvailableLookAndFeel(String laf) {
         try { 
             Class lnfClass = Class.forName(laf);
             LookAndFeel newLAF = (LookAndFeel)(lnfClass.newInstance());
             return newLAF.isSupportedLookAndFeel();
         } catch(Exception e) { // If ANYTHING weird happens, return false
             return false;
         }
     }


    /**
     * Determines if this is an applet or application
     */

    public boolean isApplet() {
	return (applet != null);
    }



    /**
     * Returns the applet instance
     */

    public SAFGUIApplet getApplet() {
	return applet;
    }



    /**
     * Returns the frame instance
     */

    public JFrame getFrame() {
	return frame;
    }


    /**
     * Returns the menubar
     */

    public JMenuBar getMenuBar() {
	return menuBar;
    }


    /**
     * Returns the toolbar
     */

    public ToggleButtonToolBar getToolBar() {
	return toolbar;
    }


    /**
     * Returns the toolbar button group
     */

    public ButtonGroup getToolBarGroup() {
	return toolbarGroup;
    }


    /**
     * Returns the content pane wether we're in an applet
     * or application
     */

    public Container getContentPane() {
	if(contentPane == null) {
	    if(getFrame() != null) {
		contentPane = getFrame().getContentPane();
	    } else if (getApplet() != null) {
		contentPane = getApplet().getContentPane();
	    }
	}
	return contentPane;
    }


    /**
     * Create a frame for SAFGUI to reside in if brought up
     * as an application.
     */

    public static JFrame createFrame() {
	JFrame frame = new JFrame();
	WindowListener l = new WindowAdapter() {
	    public void windowClosing(WindowEvent e) {
		System.exit(0);
	    }
	};
	frame.addWindowListener(l);
	return frame;
    }


    /**
     * Set the status 
     */

    public void setStatus(String s) {
	// do the following on the gui thread
	SwingUtilities.invokeLater(new SAFRunnable(this, s) {
	    public void run() {
		saf_gui.statusField.setText((String) obj);
	    }
	});
    }



    /**
     * This method returns a string from the module's resource bundle.
     */
    public String getString(String key) {
	String value = null;
	try {
	    value = getResourceBundle().getString(key);
	} catch (MissingResourceException e) {
	    System.out.println("java.util.MissingResourceException: Couldn't find value for: " + key);
	}
	if(value == null) {
	    value = "Could not find resource: " + key + "  ";
	}
	return value;
    }


    /**
     * Returns the resource bundle associated with this module . Used
     * to get accessable and internationalized strings.
     */

    public ResourceBundle getResourceBundle() {
	if(bundle == null) {
	    bundle = ResourceBundle.getBundle("resources.saf_gui");
	}
	return bundle;
    }


    /**
     * Returns a mnemonic from the resource bundle. Typically used as
     * keyboard shortcuts in menu items.
     */

    public char getMnemonic(String key) {
	return (getString(key)).charAt(0);
    }

    /**
     * Creates an icon from an image contained in the "images" directory.
     */
    public ImageIcon createImageIcon(String filename, String description) {
	String path = "/resources/images/" + filename;
	return new ImageIcon(getClass().getResource(path)); 
    }

    public ImageIcon createImageIcon(String filename) {
	String path = "/resources/images/" + filename;
	return new ImageIcon(getClass().getResource(path)); 
    }


    /**
     * If DEBUG is defined, prints debug information out to std ouput.
     */

    public void debug(String s) {
	if(DEBUG) {
	    System.out.println((debugCounter++) + ": " + s);
	}
    }


    /**
     * Stores the current L&F, and calls updateLookAndFeel, below
     */

    public void setLookAndFeel(String laf) {
	if(currentLookAndFeel != laf) {
	    currentLookAndFeel = laf;
	    themesMenu.setEnabled(laf == metal);
	    updateLookAndFeel();
	}
    }


    /**
     * Sets the current L&F on each saf module
     */

    public void updateLookAndFeel() {
	try {
	    UIManager.setLookAndFeel(currentLookAndFeel);
	    SwingUtilities.updateComponentTreeUI(this);
	} catch (Exception ex) {
	    System.out.println("Failed loading L&F: " + currentLookAndFeel);
	    System.out.println(ex);
	}


	for (int i = 0; i < SAFVector.size(); i++) {
	    SAFModule saf_mod = (SAFModule) SAFVector.elementAt(i);
	    if(currentModule != saf_mod) {
		// do the following on the gui thread
		SwingUtilities.invokeLater(new SAFRunnable(this, saf_mod) {
		    public void run() {
			SwingUtilities.updateComponentTreeUI(((SAFModule)obj).getSAFPanel());
		    }
		});
	    }
	}
    }


    /**
     * Loads and puts source code text into JEditorPane in the "Source Code" tab
     */

    public void setSourceCode(SAFModule saf_mod) {
	// do the following on the gui thread
	SwingUtilities.invokeLater(new SAFRunnable(this, saf_mod) {
	    public void run() {
		// saf_gui.SAFSrcPane.setText(((SAFModule)obj).getSourceCode());
		// saf_gui.SAFSrcPane.setCaretPosition(0);
		// saf_gui.SAFSrcPane.repaint();

	    }
	});
    }



    // *******************************************************
    // **************   ToggleButtonToolbar  *****************
    // *******************************************************

    static Insets zeroInsets = new Insets(1,1,1,1);
    protected class ToggleButtonToolBar extends JToolBar {
	public ToggleButtonToolBar() {
	    super();
	}

	JToggleButton addToggleButton(Action a) {
	    JToggleButton tb = new JToggleButton(
		(String)a.getValue(Action.NAME),
		(Icon)a.getValue(Action.SMALL_ICON)
	    );
	    tb.setMargin(zeroInsets);
	    tb.setText(null);
	    tb.setEnabled(a.isEnabled());
	    tb.setToolTipText((String)a.getValue(Action.SHORT_DESCRIPTION));
	    // tb.setAction(a);
	    add(tb);
	    return tb;
	}
    }


    // *******************************************************
    // ******************   Runnables  ***********************
    // *******************************************************


    /**
     * Generic SAFGUI runnable. This is intended to run on the
     * AWT gui event thread so as not to muck things up by doing
     * gui work off the gui thread. Accepts a SAFGUI and an Object
     * as arguments, which gives subtypes of this class the two
     * "must haves" needed in most runnables for this module.
     */

    class SAFRunnable implements Runnable {
	protected SAFGUI saf_gui;
	protected Object obj;
	
	public SAFRunnable(SAFGUI saf_gui, Object obj) {
	    this.saf_gui = saf_gui;
	    this.obj = obj;
	}

	public void run() {
	}
    }
	

    
    // *******************************************************
    // ********************   Actions  ***********************
    // *******************************************************
    
    public class SwitchToSAFAction extends AbstractAction {
	SAFGUI saf_gui;
	SAFModule saf_mod;
	
	public SwitchToSAFAction(SAFGUI saf_gui, SAFModule saf_mod) {
	    super(saf_mod.getName(), saf_mod.getIcon());
	    this.saf_gui = saf_gui;
	    this.saf_mod = saf_mod;
	}

	public void actionPerformed(ActionEvent e) {
	    saf_gui.setModule(saf_mod);
	}
    }


    class OkAction extends AbstractAction {
	JDialog aboutBox;

        protected OkAction(JDialog aboutBox) {
            super("OkAction");
	    this.aboutBox = aboutBox;
        }

        public void actionPerformed(ActionEvent e) {
	    aboutBox.setVisible(false);
	}
    }


    class ChangeLookAndFeelAction extends AbstractAction {
	SAFGUI saf_gui;
	String laf;
        protected ChangeLookAndFeelAction(SAFGUI saf_gui, String laf) {
            super("ChangeTheme");
	    this.saf_gui = saf_gui;
	    this.laf = laf;
        }

        public void actionPerformed(ActionEvent e) {
	    saf_gui.setLookAndFeel(laf);
	}
    }


    class ChangeThemeAction extends AbstractAction {
	SAFGUI saf_gui;
	DefaultMetalTheme theme;
        protected ChangeThemeAction(SAFGUI saf_gui, DefaultMetalTheme theme) {
            super("ChangeTheme");
	    this.saf_gui = saf_gui;
	    this.theme = theme;
        }

        public void actionPerformed(ActionEvent e) {
	    MetalLookAndFeel.setCurrentTheme(theme);
	    saf_gui.updateLookAndFeel();
	}
    }


    class ExitAction extends AbstractAction {
	SAFGUI saf_gui;
        protected ExitAction(SAFGUI saf_gui) {
            super("ExitAction");
	    this.saf_gui = saf_gui;
        }

        public void actionPerformed(ActionEvent e) {
	    if( dbOpened == true ) {
		saf_wrap.close(top_sets_id);
	    }
	    saf_init.fin();
	    System.exit(0);
        }
    }

    class OpenAction extends AbstractAction {
	int goodFile = 0;
	int filestate;
	File file;
	SAFGUI saf_gui;

	protected OpenAction(SAFGUI saf_gui) {
		super("OpenAction");
		this.saf_gui = saf_gui;
	}

        public void actionPerformed(ActionEvent e) {
            Frame frame = getFrame();

	    if( dbOpened == true ) {
		JOptionPane.showMessageDialog(null, saf_filename + " is already open");
		return;
	    }
            if (fileDialog == null) {
                // fileDialog = new FileDialog(frame, "Open a SAF Database");
                fileDialog = new JFileChooser(".");
		fileDialog.setDialogTitle("Open SAF Database");
		fileDialog.setFileFilter(new SAFFilter());
            }
            // fileDialog.setMode(FileDialog.LOAD);
            // fileDialog.show();

	    while( goodFile == 0 ) {
	    	filestate = fileDialog.showOpenDialog(null);

            // String file = fileDialog.getFile();
	        file = fileDialog.getSelectedFile();

		if( filestate == JFileChooser.CANCEL_OPTION )
			break;

	    	if ( ! file.exists() ) {
				goodFile = 0;
	    	}
	    	else {
	       	goodFile = 1;
	    	}
	    }

	    if( file != null && filestate == JFileChooser.APPROVE_OPTION) {
			// JOptionPane.showMessageDialog(null, file.getPath());
			saf_filepath = file.getParent();
			// saf_filename = file.getPath();
			saf_filename = file.getName();
			top_sets_id = saf_wrap.open(saf_filepath, saf_filename,"r");
			if( top_sets_id != -1 ) {
				dbOpened = true;
				goodFile = 0;
				getFrame().setTitle(saf_filename);
				all_sets_id = saf_wrap.get_all_sets_id();
				tree_string = saf_wrap.saftree();
				if( startup == 0 ) {
					// preloadFirstModule();
					loadModule("SAFTree");
					setModule((SAFModule)saf_tree);
					startup = 1;
				}
				else {
					saf_tree.newtree(safgui, SAFPanel);
					// saf_gui.add(tabbedPane, BorderLayout.CENTER);
					saf_gui.tabbedPane.setVisible(true);
					saf_gui.tabbedPane.repaint();
					saf_gui.repaint();
					getFrame().repaint();
				}
			}
	    }
	    else if(filestate == JFileChooser.CANCEL_OPTION) {
		// JOptionPane.showMessageDialog(null, "Canceled");
		System.exit(0);
	    }

            // String directory = fileDialog.getDirectory();
            // File f = new File(directory, file);
        }
    }

    class CloseAction extends AbstractAction {
	SAFGUI saf_gui;
        protected CloseAction(SAFGUI saf_gui) {
            super("CloseAction");
	    this.saf_gui = saf_gui;
        }

        public void actionPerformed(ActionEvent e) {
		if( dbOpened == true ) {
	    		saf_wrap.close(top_sets_id);
			top_sets_id = -1;
			dbOpened = false;
			saf_tree.closetree(SAFPanel);

			saf_gui.remove(tabbedPane);
			saf_gui.tabbedPane.setVisible(false);
			for (int i = 0; i < ButtonVector.size(); i++) {
				JToggleButton tb = (JToggleButton) ButtonVector.elementAt(i);
				tb.setVisible(false);
				tb.repaint();
			 }
			 saf_gui.repaint();


			// JOptionPane.showMessageDialog(null, saf_filename + " is closed");
			// OpenFile.actionPerformed(null);
		    System.exit(0);
			
		}
		else {
			JOptionPane.showMessageDialog(null, "No SAF Database is Open");
		}
        }
    }

    class AboutAction extends AbstractAction {
	SAFGUI saf_gui;
        protected AboutAction(SAFGUI saf_gui) {
            super("AboutAction");
	    this.saf_gui = saf_gui;
        }
	
        public void actionPerformed(ActionEvent e) {
	    if(aboutBox == null) {
		// JPanel panel = new JPanel(new BorderLayout());
		JPanel panel = new AboutPanel(saf_gui);
		panel.setLayout(new BorderLayout());

		aboutBox = new JDialog(saf_gui.getFrame(), getString("AboutBox.title"), false);
		aboutBox.getContentPane().add(panel, BorderLayout.CENTER);

		// JButton button = new JButton(getString("AboutBox.ok_button_text"));
		JPanel buttonpanel = new JPanel();
		buttonpanel.setOpaque(false);
		JButton button = (JButton) buttonpanel.add(
		    new JButton(getString("AboutBox.ok_button_text"))
		);
		panel.add(buttonpanel, BorderLayout.SOUTH);

		button.addActionListener(new OkAction(aboutBox));
	    }
	    aboutBox.pack();
	    Point p = saf_gui.getLocationOnScreen();
	    aboutBox.setLocation(p.x + 10, p.y +10);
	    aboutBox.show();
	}
    }



    // *******************************************************
    // **********************  Misc  *************************
    // *******************************************************


    class ModuleLoadThread extends Thread {
	SAFGUI saf_gui;
	
	public ModuleLoadThread(SAFGUI saf_gui) {
	    this.saf_gui = saf_gui;
	}

	public void run() {
	    // saf_gui.loadModules();
	}
    }


    class AboutPanel extends JPanel {
	ImageIcon aboutimage = null;
	SAFGUI saf_gui = null;

	public AboutPanel(SAFGUI saf_gui) {
	    this.saf_gui = saf_gui ;
	    aboutimage = saf_gui.createImageIcon("NewAbout.gif", "AboutBox.accessible_description");
	    setOpaque(false);
	}

	public void paint(Graphics g) {
	    aboutimage.paintIcon(this, g, 0, 0);
	    super.paint(g);
	}

	public Dimension getPreferredSize() {
	    return new Dimension(aboutimage.getIconWidth(),
				 aboutimage.getIconHeight());
	}
    }

}

/* SafFilter class is used by file chooser in open() method
 * to display only files with suffix .saf
 */

class SAFFilter extends
  javax.swing.filechooser.FileFilter {
  public boolean accept(File f) {
    return(f.getName().toLowerCase().endsWith(".saf") || f.isDirectory()); }
  public String getDescription() {
    return "Saf Files(*.saf)"; }
}


