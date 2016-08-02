///////////////////////////////////////////////////////////////////////
//
// ControlPanel
//
// JFrame with ParamManager member
//
// Mike Dixon
//
// Oct 2002
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

public class ControlPanel extends JFrame

{

    private Parameters _params;
    private MessageQueue _guiCommands;
    private PowerControl _powerControl;
    private AntennaControl _antennaControl;
    private BeamGeomControl _beamGeomControl;
    
    public ControlPanel(Parameters params,
			MessageQueue guiCommands) {
	
	super();
	_params = params;
	_guiCommands = guiCommands;

	// titles

	setTitle("Radar control panel");
	setSize(_params.controlPanel.width.getValue(),
		_params.controlPanel.height.getValue());
	setLocation(_params.controlPanel.xx.getValue(),
		    _params.controlPanel.yy.getValue());
	setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
	addComponentListener(new ChangeListener(_params));

	// power, beam and antenna sub-panels
	
	_powerControl = new PowerControl(this, _params, _guiCommands);
	_beamGeomControl = new BeamGeomControl(this, _params, _guiCommands);
	_antennaControl = new AntennaControl(this, _params, _guiCommands);

	// top panel
	
  	JPanel topPanel = new JPanel();
  	getContentPane().add(topPanel);

 	GridBagLayout gridbag = new GridBagLayout();
 	GridBagConstraints gbc = new GridBagConstraints();
 	topPanel.setLayout(gridbag);
	
 	gbc.gridx = 0;
 	gbc.gridy = 0;
 	gbc.weightx = 0;
 	gbc.weighty = 0;
 	gbc.anchor = GridBagConstraints.NORTHWEST;
 	topPanel.add(_powerControl, gbc);
	
 	gbc.gridx = 0;
 	gbc.gridy = 1;
 	gbc.weightx = 0;
 	gbc.weighty = 0;
	gbc.fill = GridBagConstraints.HORIZONTAL;
 	gbc.anchor = GridBagConstraints.NORTHEAST;
 	topPanel.add(_beamGeomControl, gbc);
	
 	gbc.gridx = 1;
 	gbc.gridy = 0;
	gbc.gridheight = 2;
 	gbc.weightx = 1.0;
 	gbc.weighty = 1.0;
 	gbc.anchor = GridBagConstraints.NORTHWEST;
 	topPanel.add(_antennaControl, gbc);
	
    }
    // public set methods

    public void setMainPowerIndicator(boolean on) {
	_powerControl.setMainPowerIndicator(on);
    }
    
    public void setMagnetronPowerIndicator(boolean on) {
	_powerControl.setMagnetronPowerIndicator(on);
    }
    
    public void setServoPowerIndicator(boolean on) {
	_powerControl.setServoPowerIndicator(on);
    }
    
    public void setRadiateIndicator(boolean on) {
	_powerControl.setRadiateIndicator(on);
    }

    public void setAntennaPosition(double el, double az) {
	_antennaControl.setAntennaPosition(el, az);
    }

    public void setSunPosition(double el, double az) {
	_antennaControl.setSunPosition(el, az);
    }

    private class ChangeListener extends ComponentAdapter {
	private Parameters _params;
	public ChangeListener(Parameters params) {
	    super();
	    _params = params;
	}
	
	public void componentMoved(ComponentEvent e) {
	    int paramX = _params.controlPanel.xx.getValue();
	    int paramY = _params.controlPanel.yy.getValue();
	    if (paramX != getX() || paramY != getY()) {
		_params.controlPanel.xx.setValue(getX());
		_params.controlPanel.yy.setValue(getY());
	    }
	}
	
	public void componentResized(ComponentEvent e) {
	    int paramWidth = _params.controlPanel.width.getValue();
	    int paramHeight = _params.controlPanel.height.getValue();
	    if (paramWidth != getWidth() || paramHeight != getHeight()) {
		_params.controlPanel.width.setValue(getWidth());
		_params.controlPanel.height.setValue(getHeight());
	    }
	}
	
    }

}

