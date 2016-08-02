///////////////////////////////////////////////////////////////////////
//
// Parameters
//
// Mike Dixon
//
// October 2002
//
////////////////////////////////////////////////////////////////////////

import params.*;
import javax.swing.*;

public class Parameters extends ParamManager

{
    
    public BooleanParam debug;
    public BooleanParam verbose;
    public RadarParams radar;
    public ScanParams scan;
    public CalibrationParams calib;
    public ControlPanelParams controlPanel;
    public AScopeParams aScope;
    public PpiDisplayParams ppiDisplay;
    public MainWindowParams mainWindow;
    
    public Parameters(JPanel topPanel,
		      String initParamPath) {

	super(topPanel, initParamPath);
	
	// debug?
	
	debug = new BooleanParam("debug");
	debug.setLabel("Debugging flag");
	debug.setValue(false);
	
	// verbose debug?
	
	verbose = new BooleanParam("verbose");
	verbose.setLabel("Verbose debugging flag");
	verbose.setValue(false);
	
	// radar parameters
	
	radar = new RadarParams("radar", "Radar params", getNextDepth());
	radar.setDescription("Edit the radar parameters");

	// scan parameters
	
	scan = new ScanParams("scan", "Scan strategy", getNextDepth());
	scan.setDescription("Edit the radar scan strategy");
	
	// calib parameters
	
	calib = new CalibrationParams("calib",
				      "Calibration params", getNextDepth());
	calib.setDescription("Edit the calibration values");
	
	// control panel
	
	controlPanel = new ControlPanelParams("controlPanel",
					      "Control Panel",
					      getNextDepth());
	controlPanel.setDescription("Radar control panel");
	controlPanel.setInfo("Parameters for the size, location, etc." +
			     " of the control panel");

	// A-scope
	
	aScope = new AScopeParams("aScope", "A-Scope", getNextDepth());
	aScope.setDescription("A-scope display - for calibration");
	aScope.setInfo("Parameters for the size, location, etc." +
		       " of the A-scope");

	// Ppi display
	
	ppiDisplay = new PpiDisplayParams("ppiDisplay",
					  "PPI Display",
					  getNextDepth());
	ppiDisplay.setDescription("PPI Display for viewing raw radar data");
	ppiDisplay.setInfo("Parameters for the size, location, etc." +
		       " of the PPI display");

	// main window

	mainWindow = new MainWindowParams("mainWindow", "Main window",
					  getNextDepth());
	mainWindow.setDescription("Main window");

	// add the elements
	
	add(debug);
	add(verbose);
	add(radar);
	add(scan);
	add(calib);
	add(controlPanel);
	add(aScope);
	add(ppiDisplay);
	add(mainWindow);
	add(ParamGuiParams.getInstance());
	
	// activate the frame for the gui

	activateFrame();

    }

    public void setDebug(boolean isOn) {
	debug.setValue(isOn);
    }

    public void setVerboseDebug(boolean isOn) {
	verbose.setValue(isOn);
    }

}
