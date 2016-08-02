///////////////////////////////////////////////////////////////////////
//
// DisplayParams
//
// Entry panel for display parameters
//
// Mike Dixon
//
// Oct 2002
//
////////////////////////////////////////////////////////////////////////

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import params.*;

class DisplayParams extends CollectionParam
	    
{

    public ControlPanelParams controlPanel;
    public AScopeParams aScope;
    public PpiDisplayParams ppiDisplay;
    
    // constructor
    
    public DisplayParams(String name, String label, int depth)
    {
	
	super(name, label, depth);
	
	// initialize the parameter objects
	
	// control panel
	
	controlPanel = new ControlPanelParams("controlPanel",
					      "Control Panel",
					      getNextDepth());
	// controlPanel.setLabel("Control Panel");
	controlPanel.setDescription("Radar control panel");
	controlPanel.setInfo("Parameters for the size, location, etc." +
			     " of the control panel");

	// A-scope
	
	aScope = new AScopeParams("aScope", "A-Scope", getNextDepth());
	// aScope.setLabel("A-Scope");
	aScope.setDescription("A-scope display - for calibration");
	aScope.setInfo("Parameters for the size, location, etc." +
		       " of the A-scope");

	// Ppi display
	
	ppiDisplay = new PpiDisplayParams("ppiDisplay",
					  "PPI Display",
					  getNextDepth());
	// ppiDisplay.setLabel("PPI Display");
	ppiDisplay.setDescription("PPI Display for viewing raw radar data");
	ppiDisplay.setInfo("Parameters for the size, location, etc." +
		       " of the PPI display");

	// add the params to the list
	
	add(controlPanel);
	add(aScope);
	add(ppiDisplay);

	// copy the values to the defaults
	
	setDefaultFromValue();

    }

}


