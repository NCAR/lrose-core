///////////////////////////////////////////////////////////////////////
//
// MainMenu
//
// Main menu for TrControl
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import params.*;

class MainMenu

{

    private final int ITEM_PLAIN = 0;	// Item types
    private final int ITEM_CHECK = 1;
    private final int ITEM_RADIO = 2;

    // members
    
    private TrControl _app = null;
    private JPanel _topPanel = null;
    private JMenuBar _menuBar = null;
    private Parameters _params = null;
    private MessageQueue _outQueue = null;
    private ControlPanel _controlPanel = null;
    private AScope _aScope = null;
    private PpiDisplay _ppiDisplay = null;
    
    public MainMenu(TrControl app,
		    JPanel topPanel,
		    Parameters params,
		    MessageQueue outQueue)
    {

	// save context
	_app = app;
	_topPanel = topPanel;
	_params = params;
	_outQueue = outQueue;
	
	// Create the main menu bar
	
	_menuBar = new JMenuBar();
	
	// Set this instance as the application's menu bar

	_app.setJMenuBar(_menuBar);
	
	// Create a new toolbar

 	JToolBar toolBar = new JToolBar();
	toolBar.setFloatable(false);
	_topPanel.add(toolBar, BorderLayout.NORTH);
	
	// Create the file & configuration menu, add exit to it
	
	JMenu fileMenu = _params.getMenu("File");
	fileMenu.setMnemonic('F');
	fileMenu.setToolTipText("Configuration menu");

	JMenuItem exit = fileMenu.add(new ExitAction("Exit"));
	exit.setMnemonic('x');
	exit.setToolTipText("Exit the program");
	
	// Create the calibrate menu

	JMenu calMenu = new JMenu("Calibrate");
	calMenu.setMnemonic('a');
	calMenu.setToolTipText("Perform calibration");

	// Create the operate menu

	JMenu opMenu = new JMenu("Operate");
	opMenu.setMnemonic('O');
	opMenu.setToolTipText("Operate radar");
	populateOpMenu(toolBar, opMenu);
	
	_menuBar.add(fileMenu);
	_menuBar.add(calMenu);
	_menuBar.add(opMenu);

	// create the active file label

	JLabel activeFileLabel = params.getActiveFileLabel();
	_topPanel.add(activeFileLabel, BorderLayout.SOUTH);
	
	// create toolbar
	
  	JButton openTool = toolBar.add(_params.getFileOpenAction());
  	openTool.setMnemonic('O');
  	openTool.setToolTipText("Open an existing config");
	
  	JButton editTool = toolBar.add(_params.getEditAction());
  	editTool.setMnemonic('E');
  	editTool.setToolTipText("Edit the parameters");
	
  	JButton saveTool = toolBar.add(_params.getFileSaveAction());
  	saveTool.setMnemonic('S');
  	saveTool.setToolTipText("Save this config");
	
    }

    private void populateEditMenu(JMenu editMenu)
    {
	
	// Create setup menu options
	
	JMenuItem cut = editMenu.add(new CutAction("Cut", "cut.gif"));
	cut.setMnemonic('t');
	cut.setToolTipText("Cut data to the clipboard");

	JMenuItem copy =
	    editMenu.add(new CopyAction("Copy", "copy.gif"));
	copy.setMnemonic('C');
	copy.setToolTipText("Copy data to the clipboard");
	
	JMenuItem paste =
	    editMenu.add(new PasteAction("Paste", "paste.gif"));
	paste.setMnemonic('P');
	paste.setToolTipText("Paste data from the clipboard");

    }

    public void populateOpMenu(JToolBar toolBar,
			       JMenu opMenu)
    {

	// control panel

	_controlPanel = new ControlPanel(_params, _outQueue);
	ImageIcon controlPanelIcon =
	    new ImageIcon(this.getClass().getResource("./gauge.png"));
	AbstractAction controlPanelAction =
	    new ControlPanelAction("Control panel", controlPanelIcon);
	JMenuItem controlPanelItem = opMenu.add(controlPanelAction);
	controlPanelItem.setToolTipText("Open control panel");
	if (_params.controlPanel.startVisible.getValue()) {
	    _controlPanel.setVisible(true);
	}
	
	// aScope
	
	_aScope = new AScope(_params);
	AbstractAction aScopeAction = new AScopeAction("A-scope", null);
	JMenuItem aScopeItem = opMenu.add(aScopeAction);
	aScopeItem.setToolTipText("Open A Scope");
	if (_params.aScope.startVisible.getValue()) {
	    _aScope.setVisible(true);
	}
	
	
	// ppiDisplay
	
	_ppiDisplay = new PpiDisplay(_params);
	AbstractAction ppiDisplayAction =
	    new PpiDisplayAction("PPI Display", null);
	JMenuItem ppiDisplayItem = opMenu.add(ppiDisplayAction);
	ppiDisplayItem.setToolTipText("Open PPI Display");
	
    }
    
    private JMenuItem createMenuItem(int iType, String sText,
				     ImageIcon image, int acceleratorKey,
				     String sToolTip)
    {
	// Create the item
	JMenuItem menuItem;

	switch(iType)
	    {
	    case ITEM_RADIO:
		menuItem = new JRadioButtonMenuItem();
		break;

	    case ITEM_CHECK:
		menuItem = new JCheckBoxMenuItem();
		break;

	    default:
		menuItem = new JMenuItem();
		break;
	    }

	// Add the item text
	menuItem.setText(sText);
	
	// Add the optional icon
	if(image != null)
	    menuItem.setIcon(image);
	
	// Add the accelerator key
	if(acceleratorKey > 0)
	    menuItem.setMnemonic(acceleratorKey);

	// Add the optional tool tip text
	if(sToolTip != null)
	    menuItem.setToolTipText(sToolTip);

	return menuItem;
    }

    ////////////////////////////////////////////////////
    // Inner classes for actions
    ////////////////////////////////////////////////////

    // exit action
    private class ExitAction extends AbstractAction
    {
	public ExitAction(String name )
	{
	    super( name, null );
	}
	public void actionPerformed(ActionEvent event) {
	    _app.doExit();
	}
    }

    // controlPanel action
    public class ControlPanelAction extends AbstractAction
    {
	public ControlPanelAction(String label, ImageIcon icon)
	{
	    super(label, icon);
	}
	
	public void actionPerformed( ActionEvent event)
	{
	    _controlPanel.setVisible(true);
	}
    }

    // aScope action
    public class AScopeAction extends AbstractAction
    {
	public AScopeAction(String label, ImageIcon icon)
	{
	    super(label, icon);
	}
	
	public void actionPerformed( ActionEvent event )
	{
	    _aScope.setVisible(true);
	}
    }
    
    // ppiDsiplay action
    public class PpiDisplayAction extends AbstractAction
    {
	public PpiDisplayAction(String label, ImageIcon icon)
	{
	    super(label, icon);
	}
	
	public void actionPerformed( ActionEvent event )
	{
	    _ppiDisplay.setVisible(true);
	}
    }

    // copy action
    public class CopyAction extends AbstractAction
    {
	public CopyAction(String label, String icon_file)
	{
	    super(label, new ImageIcon(icon_file));
	}
	
	public void actionPerformed( ActionEvent event )
	{
	    System.out.println(event);
	}
    }

    // copy action
    public class CutAction extends AbstractAction
    {
	public CutAction(String label, String icon_file)
	{
	    super(label, new ImageIcon(icon_file));
	}
	
	public void actionPerformed( ActionEvent event )
	{
	    System.out.println(event);
	}
    }

    // paste action
    private class PasteAction extends AbstractAction
    {
	public PasteAction(String label, String icon_file)
	{
	    super(label, new ImageIcon(icon_file));
	}
	
	public void actionPerformed( ActionEvent event )
	{
	    System.out.println(event);
	}
    }

    // get methods

    public ControlPanel getControlPanel() {
	return _controlPanel;
    }

    public AScope getAScope() {
	return _aScope;
    }

}

