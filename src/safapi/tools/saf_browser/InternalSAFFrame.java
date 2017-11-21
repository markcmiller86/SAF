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
 * @(#)InternalSAFFrame.java	
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
 * Internal SAF Frames
 *
 */

public class InternalSAFFrame extends SAFModule {
    int windowCount = 0;
    JDesktopPane desktop = null;

    ImageIcon icon1, icon2, icon3, icon4;
    ImageIcon smIcon1, smIcon2, smIcon3, smIcon4;

    public Integer FIRST_FRAME_LAYER  = new Integer(1);
    public Integer SAF_FRAME_LAYER   = new Integer(2);
    public Integer PALETTE_LAYER     = new Integer(3);

    public int FRAME0_X        = 15;
    public int FRAME0_Y        = 280;

    public int FRAME0_WIDTH    = 320;
    public int FRAME0_HEIGHT   = 230;

    public int FRAME_WIDTH     = 625;
    public int FRAME_HEIGHT    = 250;

    public int PALETTE_X      = 375;
    public int PALETTE_Y      = 20;

    public int PALETTE_WIDTH  = 260;
    public int PALETTE_HEIGHT = 230;

    JCheckBox windowResizable   = null;
    JCheckBox windowClosable    = null;
    JCheckBox windowIconifiable = null;
    JCheckBox windowMaximizable = null;

    JTextField windowTitleField = null;
    JLabel windowTitleLabel = null;

    /**
     * main method allows us to run as standalone.
     */
    public static void main(String[] args) {
	InternalSAFFrame saf_mod = new InternalSAFFrame(null);
	saf_mod.mainImpl();
    }

    /**
     * InternalSAFFrame Constructor
     */
    public InternalSAFFrame(SAFGUI saf_gui) {
	super(saf_gui, "InternalSAFFrame", "toolbar/JDesktop.gif");

	// Create the desktop pane
	desktop = new JDesktopPane();
	getSAFPanel().add(desktop, BorderLayout.CENTER);

	windowTitleField = new JTextField(getString("InternalSAFFrame.frame_label"));
	windowTitleLabel = new JLabel(getString("InternalSAFFrame.title_text_field_label"));
	windowResizable   = new JCheckBox(getString("InternalSAFFrame.resizable_label"), true);
	windowClosable    = new JCheckBox(getString("InternalSAFFrame.closable_label"), true);
	windowIconifiable = new JCheckBox(getString("InternalSAFFrame.iconifiable_label"), true);
	windowMaximizable = new JCheckBox(getString("InternalSAFFrame.maximizable_label"), true);


	// Create an initial internal frame to show
	// JInternalFrame frame1 = createInternalFrame(FIRST_FRAME_LAYER, 1, 1);
	// frame1.setBounds(FRAME0_X, FRAME0_Y, FRAME0_WIDTH, FRAME0_HEIGHT);

	// Create four more starter windows
	createInternalFrame(SAF_FRAME_LAYER, FRAME_WIDTH, FRAME_HEIGHT);
	createInternalFrame(SAF_FRAME_LAYER, FRAME_WIDTH, FRAME_HEIGHT);
	// createInternalFrame(SAF_FRAME_LAYER, FRAME_WIDTH, FRAME_HEIGHT);
	// createInternalFrame(SAF_FRAME_LAYER, FRAME_WIDTH, FRAME_HEIGHT);
    }



    /**
     * Create an internal frame and add a scrollable imageicon to it
     */
    public JInternalFrame createInternalFrame(Integer layer, int width, int height) {
	JInternalFrame jif = new JInternalFrame();

	if(!windowTitleField.getText().equals(getString("InternalSAFFrame.frame_label"))) {
	    jif.setTitle(windowTitleField.getText() + "  ");
	} else {
	    // jif = new JInternalFrame(getString("InternalSAFFrame.frame_label") + " " + windowCount + "  ");
	    jif = new JInternalFrame("SAF Sets" + " " + windowCount + "  ");
	}

        // HtmlPane html = new HtmlPane();
	GetFile html = new GetFile("test.html");
	// JEditorPane html = new JEditorPane();

	// set properties
	jif.setClosable(windowClosable.isSelected());
	jif.setMaximizable(windowMaximizable.isSelected());
	jif.setIconifiable(windowIconifiable.isSelected());
	jif.setResizable(windowResizable.isSelected());

	// jif.setBounds(20*(windowCount%10), 20*(windowCount%10), width, height);
	jif.setBounds(0, windowCount*height, width, height);
	//  jif.setContentPane(new ImageScroller(this, icon, 0, windowCount));
	jif.setContentPane(html.html);

	windowCount++;
	
	desktop.add(jif, layer);  

	// Set this internal frame to be selected

	try {
	    jif.setSelected(true);
	} catch (java.beans.PropertyVetoException e2) {
	}

	jif.show();

	return jif;
    }


    class ShowFrameAction extends AbstractAction {
	InternalSAFFrame saf_mod;
	Icon icon;
	
	
	public ShowFrameAction(InternalSAFFrame saf_mod, Icon icon) {
	    this.saf_mod = saf_mod;
	    this.icon = icon;
	}
	
	public void actionPerformed(ActionEvent e) {
	    saf_mod.createInternalFrame(getSAFFrameLayer(),
				     getFrameWidth(),
				     getFrameHeight()
	    );
	}
    }

    public int getFrameWidth() {
	return FRAME_WIDTH;
    }

    public int getFrameHeight() {
	return FRAME_HEIGHT;
    }

    public Integer getSAFFrameLayer() {
	return SAF_FRAME_LAYER;
    }
    
    class ImageScroller extends JScrollPane {
	
	public ImageScroller(InternalSAFFrame saf_mod, Icon icon, int layer, int count) {
	    super();
	    JPanel p = new JPanel();
	    p.setBackground(Color.white);
	    p.setLayout(new BorderLayout() );
	    
	    p.add(new JLabel(icon), BorderLayout.CENTER);
	    
	    getViewport().add(p);
	}
	
	public Dimension getMinimumSize() {
	    return new Dimension(25, 25);
	}
	
    }

}
