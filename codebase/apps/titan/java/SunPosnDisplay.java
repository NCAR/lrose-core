///////////////////////////////////////////////////////////////////////
//
// SunPosnDisplay
//
// Mike Dixon
//
// Jan 2003
//
////////////////////////////////////////////////////////////////////////

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.text.*;
import javax.swing.*;
import javax.swing.border.*;
import java.util.*;
import java.lang.Math;

import params.*;

public class SunPosnDisplay extends JPanel {

    private JLabel _elValue;
    private JLabel _azValue;
    
    public SunPosnDisplay(JFrame frame) {
	
	setBorder(CustomBorder.createTop(frame, "Sun position", 3));
	
	Font defaultFont = getFont();
	Font midFont =
	    defaultFont.deriveFont((float) (defaultFont.getSize() + 0));
	Font bigFont =
	    defaultFont.deriveFont((float) (defaultFont.getSize() + 2));
	
	// elevation panel
	
	JLabel elLabel = new JLabel("EL", JLabel.CENTER);
	elLabel.setFont(midFont);
	
	_elValue = new JLabel("00.00", JLabel.CENTER);
	_elValue.setFont(bigFont);
	_elValue.setForeground(Color.green);
	
	JPanel elValuePanel = new JPanel();
	elValuePanel.setBackground(Color.black);
	elValuePanel.add(_elValue);
	
	BorderLayout elLayout = new BorderLayout();
	elLayout.setVgap(1);
	JPanel elPanel = new JPanel(elLayout);
	elPanel.add(elLabel, BorderLayout.NORTH);
	elPanel.add(elValuePanel, BorderLayout.SOUTH);
	
	// azimuth panel
	
	JLabel azLabel = new JLabel("AZ", JLabel.CENTER);
	azLabel.setFont(midFont);
	
	_azValue = new JLabel("000.00", JLabel.CENTER);
	_azValue.setFont(bigFont);
	_azValue.setForeground(Color.green);
	
	JPanel azValuePanel = new JPanel();
	azValuePanel.setBackground(Color.black);
	azValuePanel.add(_azValue);
	
	BorderLayout azLayout = new BorderLayout();
	azLayout.setVgap(1);
	JPanel azPanel = new JPanel(azLayout);
	azPanel.add(azLabel, BorderLayout.NORTH);
	azPanel.add(azValuePanel, BorderLayout.SOUTH);
	
	// create position panel
	
	JPanel posPanel = new JPanel();
	BorderLayout posLayout = new BorderLayout();
	posLayout.setHgap(5);
	posPanel.setLayout(posLayout);
	posPanel.add(elPanel, BorderLayout.WEST);
	posPanel.add(azPanel, BorderLayout.EAST);
	
	// add
	
	add(posPanel);
	
    }
    
    public void setEl(double el) {
	synchronized(_elValue) {
	    _elValue.setText(NFormat.el.format(el));
	}
    }
    
    public void setAz(double az) {
	synchronized(_azValue) {
	    _azValue.setText(NFormat.az.format(az));
	}
    }
    
    public double getEl() {
	synchronized(_elValue) {
	    return Double.parseDouble(_elValue.getText());
	}
    }
    
    public double getAz() {
	synchronized(_azValue) {
	    return Double.parseDouble(_azValue.getText());
	}
    }
    
}
