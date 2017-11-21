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


class MySharedLib {


	String mylibname;

	public MySharedLib( String libname ) {
		mylibname = libname;
	}

	public void load() {

		URL url;
		String path;

		url = getClass().getResource("/resources/" + mylibname);


		File filename = new File(mylibname);
		path = filename.getAbsolutePath();

		System.load(path);
	}

}

