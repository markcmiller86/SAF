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
 * @(#)SAFModule.java	
 *
 */

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.border.*;
import javax.swing.colorchooser.*;
import javax.swing.filechooser.*;
import javax.accessibility.*;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.*;
import java.io.*;
import java.applet.*;
import java.net.*;

/**
 * A generic SAFGUI saf_mod module
 */
public class SAFModule extends JApplet {

    // The preferred size of the saf_mod
    private int PREFERRED_WIDTH = 380;
    private int PREFERRED_HEIGHT = 300;

    Border loweredBorder = new CompoundBorder(new SoftBevelBorder(SoftBevelBorder.LOWERED), 
					      new EmptyBorder(5,5,5,5));

    // public static JTextArea textarea = null;
    // public static JFrame newframe = null;
    public static JFrame frame = null;

    private SAFGUI saf_gui = null;
    private static JPanel panel = null;
    private String resourceName = null;
    private String iconPath = null;
    private String sourceCode = null;


    // Resource bundle for internationalized and accessible text
    private ResourceBundle bundle = null;

    public SAFModule(SAFGUI saf_gui) {
	this(saf_gui, null, null);
    }

    public SAFModule(SAFGUI saf_gui, String resourceName, String iconPath) {
	panel = new JPanel();
	panel.setLayout(new BorderLayout());
	panel.setOpaque(false);

	this.resourceName = resourceName;
	this.iconPath = iconPath;
	this.saf_gui = saf_gui;

	loadSourceCode();
    }

    public String getResourceName() {
	return resourceName;
    }

    public static JPanel getSAFPanel() {
	return panel;
    }

    public static JFrame getSAFFrame() {
	return frame;
    }

    // public static JFrame createNewFrame(String title, JTextArea textarea) {
    public static JFrame createNewFrame(String title, JComponent textarea) {

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

   public static JTextArea getFrameTextArea() {

 	return new JTextArea();
   }


    public SAFGUI getSAFGUI() {
	return saf_gui;
    }


    public String getString(String key) {
	String value = "nada";
	if(bundle == null) {
	    if(getSAFGUI() != null) {
		bundle = getSAFGUI().getResourceBundle();
	    } else {
		bundle = ResourceBundle.getBundle("resources.saf_gui");
	    }
	}
	try {
	    value = bundle.getString(key);
	} catch (MissingResourceException e) {
	    System.out.println("java.util.MissingResourceException: Couldn't find value for: " + key);
	}
	return value;
    }

    public char getMnemonic(String key) {
	return (getString(key)).charAt(0);
    }

    public ImageIcon createImageIcon(String filename, String description) {
	if(getSAFGUI() != null) {
	    return getSAFGUI().createImageIcon(filename, description);
	} else {
	    String path = "/resources/images/" + filename;
	    return new ImageIcon(getClass().getResource(path), description); 
	}
    }
    

    public String getSourceCode() {
	return sourceCode;
    }

    public void loadSourceCode() {
	if(getResourceName() != null) {
	    // String filename = "src/" + getResourceName() + ".java";
	    String filename = "safapi.html";
	    sourceCode = new String("<html><pre>");
	    char[] buff = new char[50000];
	    InputStream is;
	    InputStreamReader isr;
	    // CodeViewer cv = new CodeViewer();
	    URL url;
	    
	    try {
		url = getClass().getResource(filename); 
		is = url.openStream();
		isr = new InputStreamReader(is);
		BufferedReader reader = new BufferedReader(isr);
		
		// Read one line at a time, htmlize using super-spiffy
		// html java code formating utility from www.CoolServlets.com
		String line = reader.readLine();
		while(line != null) {
		    // sourceCode += cv.syntaxHighlight(line) + " \n ";
		    sourceCode += line + " \n ";
		    line = reader.readLine();
		}
		// sourceCode += new String("</pre></html>");
            } catch (Exception ex) {
                sourceCode = "Could not load file: " + filename;
            }
	}
    }

    public String getName() {
	return getString(getResourceName() + ".name");
    };

    public Icon getIcon() {
	return createImageIcon(iconPath, getResourceName() + ".name");
    };

    public String getToolTip() {
	return getString(getResourceName() + ".tooltip");
    };

    public void mainImpl() {
	frame = new JFrame(getName());
        frame.getContentPane().setLayout(new BorderLayout());
	frame.getContentPane().add(getSAFPanel(), BorderLayout.CENTER);
	getSAFPanel().setPreferredSize(new Dimension(PREFERRED_WIDTH, PREFERRED_HEIGHT));
	frame.pack();
	frame.show();
    }

    public JPanel createHorizontalPanel(boolean threeD) {
        JPanel p = new JPanel();
        p.setLayout(new BoxLayout(p, BoxLayout.X_AXIS));
        p.setAlignmentY(TOP_ALIGNMENT);
        p.setAlignmentX(LEFT_ALIGNMENT);
        if(threeD) {
            p.setBorder(loweredBorder);
        }
        return p;
    }
    
    public JPanel createVerticalPanel(boolean threeD) {
        JPanel p = new JPanel();
        p.setLayout(new BoxLayout(p, BoxLayout.Y_AXIS));
        p.setAlignmentY(TOP_ALIGNMENT);
        p.setAlignmentX(LEFT_ALIGNMENT);
        if(threeD) {
            p.setBorder(loweredBorder);
        }
        return p;
    }

    public static void main(String[] args) {
	SAFModule saf_mod = new SAFModule(null);
	saf_mod.mainImpl();
    }

    public void init() {
        getContentPane().setLayout(new BorderLayout());
        getContentPane().add(getSAFPanel(), BorderLayout.CENTER);
    }
}

