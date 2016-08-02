///////////////////////////////////////////////////////////////////////
//
// PowerControl
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

public class PowerControl extends JPanel {

    private Parameters _params;
    private MessageQueue _guiCommands;
    private OnOffPanel _mainPanel;
    private OnOffPanel _magnetronPanel;
    private OnOffPanel _servoPanel;
    private OnOffPanel _radiatePanel;
    
    public PowerControl(JFrame frame,
			Parameters params,
			MessageQueue guiCommands) {
	
	_params = params;
	_guiCommands = guiCommands;
	
	setBorder(CustomBorder.createTop(frame, "Power management", 5));
	setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	
	// create the on-off panels
	
	_mainPanel =
	    new OnOffPanel("Main Power", "main_power", false, _guiCommands);
	_magnetronPanel =
	    new OnOffPanel("Mag power", "magnetron_power", false, _guiCommands);
	_servoPanel =
	    new OnOffPanel("Servo power", "servo_power", false, _guiCommands);
	_radiatePanel =
	    new OnOffPanel("Radiate", "radiate", false, _guiCommands);
	
	// get the max width
	
	int maxWidth = (int) _mainPanel.getPreferredLabelWidth();
	maxWidth = Math.max(maxWidth, (int)
			    _magnetronPanel.getPreferredLabelWidth());
	maxWidth = Math.max(maxWidth, (int)
			    _servoPanel.getPreferredLabelWidth());
	maxWidth = Math.max(maxWidth, (int)
			    _radiatePanel.getPreferredLabelWidth());
	
	// set all to max width
	
	_mainPanel.setPreferredLabelWidth(maxWidth);
	_magnetronPanel.setPreferredLabelWidth(maxWidth);
	_servoPanel.setPreferredLabelWidth(maxWidth);
	_radiatePanel.setPreferredLabelWidth(maxWidth);
	
	add(_mainPanel);
	add(_magnetronPanel);
	add(_servoPanel);
	add(_radiatePanel);
	
    }
    
    public void setMainPowerIndicator(boolean on) {
	_mainPanel.setIndicator(on);
    }
    
    public void setMagnetronPowerIndicator(boolean on) {
	_magnetronPanel.setIndicator(on);
    }
    
    public void setServoPowerIndicator(boolean on) {
	_servoPanel.setIndicator(on);
    }
    
    public void setRadiateIndicator(boolean on) {
	_radiatePanel.setIndicator(on);
    }
    
}
