///////////////////////////////////////////////////////////////////////
//
// TrControl
//
// TITAN RDAS Control
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import params.*;

class TrControl extends JFrame

{
    
    private boolean _debug = false;
    private boolean _verbose = false;
    private String _initParamPath = null;
    private Parameters _params;
    private MainMenu _mainMenu;
    private ControlPanel _controlPanel;
    private AScope _aScope;

    public TrControl(String args[])
    {

	// parse command line

	parseArgs(args);
	
	// set up frame and top panel
	
	setTitle("TITAN RDAS Control");
	JPanel topPanel = new JPanel();
	topPanel.setLayout(new BorderLayout());
	getContentPane().add(topPanel);

	// create the Parameters
	
	_params = new Parameters(topPanel, _initParamPath);

	// display the main image

	String mainImagePath = _params.mainWindow.imageName.getValue();
	ImageIcon mainIcon =
	    new ImageIcon(this.getClass().getResource(mainImagePath));
	JLabel mainImage = new JLabel(mainIcon);
	topPanel.add(mainImage, BorderLayout.CENTER);

	// create the message queue for relaying GUI commands to the
	// radar communicator
	
	MessageQueue guiCommands = new MessageQueue();
	
	// create the menus and associated objects
	
  	_mainMenu = new MainMenu(this, topPanel, _params, guiCommands);
	_controlPanel = _mainMenu.getControlPanel();
	_aScope = _mainMenu.getAScope();
	
	if (_debug) {
	    _params.setDebug(true);
	}
	if (_verbose) {
	    _params.setVerboseDebug(true);
	}

	// set main window size

	setSize(_params.mainWindow.width.getValue(),
		_params.mainWindow.height.getValue());
	addComponentListener(new MoveResizeListener(_params));

	// set to exit on close
	
	setDefaultCloseOperation(EXIT_ON_CLOSE);

	// Set sun track thread going
	
	Thread sunTrackThread =
	    new Thread(new SunTrack(_params, _controlPanel));
	sunTrackThread.start();

	// set communicator thread going - passes GUI commands on to the
	// radar and reads the replies from the radar
	
	Communicator communicator =
	    new Communicator(this, _params,
			     _controlPanel, _aScope, guiCommands);

    }

    // print out usage and exit
    
    private void usage() {
        System.err.println("Usage: TrControl [-opts as below]");
        System.err.println("       [ -h, -help, -usage ] print usage");
        System.err.println("       [ -debug ] set debugging on");
        System.err.println("       [ -params ? ] specify params file");
        System.err.println("       [ -verbose ] set verbose debugging on");
        System.exit(1);
    }
    
    // parse command line
    
    void parseArgs(String[] args) {
	
	for (int i = 0; i < args.length; i++) {
	    if (args[i].equals("-params")) {
		if (i < args.length - 1) {
		    _initParamPath = new String(args[++i]);
		} else {
		    usage();
		}
	    } else if (args[i].equals("-debug")) {
		_debug = true;
	    } else if (args[i].equals("-verbose")) {
		_verbose = true;
	    } else if (args[i].equals("-h")) {
		usage();
	    } else if (args[i].equals("-help")) {
		usage();
	    } else if (args[i].equals("-usage")) {
		usage();
	    }
	} // i
	
    }

    public void doExit() {

	// check visibility of sub windows

	boolean cpVisible = _controlPanel.isShowing();
	if (cpVisible !=
	    _params.controlPanel.startVisible.getValue()) {
	    _params.controlPanel.startVisible.setValue(cpVisible);
	}

	boolean asVisible = _aScope.isShowing();
	if (asVisible !=
	    _params.aScope.startVisible.getValue()) {
	    _params.aScope.startVisible.setValue(asVisible);
	}

	if (_params.checkUnsavedChanges()) {
	    System.exit(0);
	}

    }

    // main
    public static void main(String args[])
    {

	// Create an instance of the application
	TrControl mainFrame = new TrControl(args);
	mainFrame.setVisible(true);
    }

    ////////////////////////////////////////////////////
    // Inner classes for listeners
    ////////////////////////////////////////////////////

    private class MoveResizeListener extends ComponentAdapter
    {
	private Parameters _params;
	public MoveResizeListener(Parameters params) {
	    super();
	    _params = params;
	}
	
	public void componentMoved(ComponentEvent e) {
	    int paramX = _params.mainWindow.xx.getValue();
	    int paramY = _params.mainWindow.yy.getValue();
	    if (paramX != getX() || paramY != getY()) {
		_params.mainWindow.xx.setValue(getX());
		_params.mainWindow.yy.setValue(getY());
	    }
	}
	
	public void componentResized(ComponentEvent e) {
	    int paramWidth = _params.mainWindow.width.getValue();
	    int paramHeight = _params.mainWindow.height.getValue();
	    if (paramWidth != getWidth() || paramHeight != getHeight()) {
		_params.mainWindow.width.setValue(getWidth());
		_params.mainWindow.height.setValue(getHeight());
	    }
	}
	
    }

}


