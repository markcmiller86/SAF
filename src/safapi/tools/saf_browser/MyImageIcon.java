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


class MyImageIcon {


	String myimagename;

	public MyImageIcon( String imagename ) {
		myimagename = imagename;
	}

	public ImageIcon icon() {

		return new ImageIcon(getClass().getResource("/resources/images/" + myimagename));

	
	}

}

