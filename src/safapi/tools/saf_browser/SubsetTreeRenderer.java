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


public class SubsetTreeRenderer extends DefaultTreeCellRenderer
{

	MyImageIcon parentof = new MyImageIcon("parentof.gif");
	MyImageIcon parentcf = new MyImageIcon("parentcf.gif");
	MyImageIcon flag = new MyImageIcon("flag.gif");
	MyImageIcon parentonf = new MyImageIcon("parentonf.gif");
	MyImageIcon parentnf = new MyImageIcon("parentnf.gif");
	MyImageIcon empty = new MyImageIcon("empty.gif");

	public SubsetTreeRenderer() {
	 }

           /**
           *  This method overrides the getTreeCellRendererComponent
           */
          public java.awt.Component getTreeCellRendererComponent(
                              JTree tree, Object value, boolean sel,
                              boolean expanded, boolean leaf,int row,
                              boolean hasFocus)
          {

		if( value instanceof DefaultMutableTreeNode) {
			setTextNonSelectionColor(Color.black);
			setTextSelectionColor(Color.black);
			DefaultMutableTreeNode node = (DefaultMutableTreeNode)value;
			// if( ! node.isRoot() ) {
			if( node.getLevel() > 0 ) {
				SetObject setobject = (SetObject)((DefaultMutableTreeNode)value).getUserObject();
				for( int i = 0 ; i < setobject.num_colls; i++ ) {
					CollectionObject collectionobject = setobject.get_collection(i);
					if( collectionobject != null ) {
						if(  collectionobject.num_fields > 0 ) {
	
							setTextNonSelectionColor(Color.red);
							setTextSelectionColor(Color.red);
							if( setobject.num_subsets > 0 ) {
								// setOpenIcon(new ImageIcon("/home/pkespen/saf/src/safapi/tools/browser_work/resources/images/parentof.gif"));
								setOpenIcon(parentof.icon());
								// setClosedIcon(new ImageIcon("/home/pkespen/saf/src/safapi/tools/browser_work/resources/images/parentcf.gif"));
								setClosedIcon(parentcf.icon());
							}
							else {
								// setLeafIcon(new ImageIcon("/home/pkespen/saf/src/safapi/tools/browser_work/resources/images/flag.gif"));
								setLeafIcon(flag.icon());
							}
							break;
						}


						else {

							if( setobject.num_subsets > 0 ) {
								// setOpenIcon(new ImageIcon("/home/pkespen/saf/src/safapi/tools/browser_work/resources/images/parentonf.gif"));
								setOpenIcon(parentonf.icon());
								// setClosedIcon(new ImageIcon("/home/pkespen/saf/src/safapi/tools/browser_work/resources/images/parentnf.gif"));
								setClosedIcon(parentnf.icon());
							}
							else {
								// setLeafIcon(new MyImageIcon("/home/pkespen/saf/src/safapi/tools/browser_work/resources/images/empty.gif"));
								setLeafIcon(empty.icon());
							}
						}


							
					}
				}
			}
		}

               super.getTreeCellRendererComponent(
                              tree, value, sel,
                              expanded, leaf, row,
                              hasFocus);

		return this;
	   }
}
