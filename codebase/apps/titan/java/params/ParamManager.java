///////////////////////////////////////////////////////////////////////
//
// ParamManager
//
// Overall manager for parameters
//
// Mike Dixon
//
// Sept 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import java.io.*;
import java.net.URL;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import org.jdom.*;
import org.jdom.input.SAXBuilder;

public class ParamManager
    extends CollectionParam
    
{
    
    private JPanel _topPanel = null;
    private String _paramFilePath = null;
    private JMenu _menu = null;
    private JLabel _activeFileLabel = null;
    private NewAction _newAction = null;
    private FileOpenAction _fileOpenAction = null;
    private FileSaveAction _fileSaveAction = null;
    private FileSaveAsAction _fileSaveAsAction = null;
    private EditAction _editAction = null;
    
    // constructor
    
    public ParamManager(JPanel topPanel,
			String initParamPath) {
	super("Root", "Param Manager", 1);
	setLabel("Param manager");
	setDescription("Root-depth parameter manager");
	_topPanel = topPanel;
	_paramFilePath = initParamPath;
    }
    
    // create a JMenu for the top-level collection
    
    public JMenu getMenu(String label) {

	if (_menu != null) {
	    return _menu;
	}
	
	_menu = new JMenu(label);
	
	// new-file button
	
	ImageIcon newIcon = new
	    ImageIcon(ParamManager.class.getResource("./new_file.png"));
	_newAction = new NewAction("New", newIcon);
	JMenuItem newItem = _menu.add(_newAction);
	newItem.setMnemonic('N');
	newItem.setToolTipText("Start new default config");
	
	// open-file button

	ImageIcon openIcon =
	    new ImageIcon(ParamManager.class.getResource("./open.png"));
	_fileOpenAction = new FileOpenAction("Open", openIcon);
	JMenuItem openItem = _menu.add(_fileOpenAction);
	openItem.setMnemonic('O');
	openItem.setToolTipText("Open existing config");
	
	// edit button
	
	ImageIcon editIcon =
	    new ImageIcon(ParamManager.class.getResource("./edit.png"));
  	_editAction = new EditAction("Edit", editIcon);
  	JMenuItem editItem = _menu.add(_editAction);
  	editItem.setMnemonic('E');
  	editItem.setToolTipText("Edit the parameters");

	// save button
	
	ImageIcon saveIcon =
	    new ImageIcon(ParamManager.class.getResource("./save.png"));
  	_fileSaveAction = new FileSaveAction("Save", saveIcon);
	_fileSaveAction.setEnabled(false);
  	JMenuItem saveItem = _menu.add(_fileSaveAction);
  	saveItem.setMnemonic('S');
  	saveItem.setToolTipText("Save this config");
	
	// save-as-file button
	
  	_fileSaveAsAction = new FileSaveAsAction("Save as");
  	JMenuItem saveAsItem = _menu.add(_fileSaveAsAction);
  	saveAsItem.setMnemonic('A');
  	saveAsItem.setToolTipText("Save this config to a new file");
	
	// initialize from XML
	
	initFromXml();

	// create active param file label
	
	_activeFileLabel = new JLabel();
	if (_paramFilePath == null) {
	    _activeFileLabel.setText("Current param file: none");
	} else {
	    _activeFileLabel.setText("Current param file: " + _paramFilePath);
	}
	_activeFileLabel.setHorizontalAlignment(SwingConstants.CENTER);
	_activeFileLabel.setVerticalAlignment(SwingConstants.CENTER);
	
	return _menu;

    }

    ///////////////////////////////////
    // public access to the actions etc

    public AbstractAction getNewAction() { 
	return _newAction;
    }
    public AbstractAction  getFileOpenAction() {
	return _fileOpenAction;
    }
    public AbstractAction  getFileSaveAction() {
	return _fileSaveAction;
    }
    public AbstractAction  getFileSaveAsAction() {
	return _fileSaveAsAction;
    }
    public AbstractAction  getEditAction() {
	return _editAction;
    }
    public JLabel getActiveFileLabel() {
	return _activeFileLabel;
    }

    // print the parameters out to a file as XML
    
    public void saveAsXml() throws IOException {
	saveAsXml(_paramFilePath);
    }

    public void saveAsXml(String paramFilePath) throws IOException {
	
	System.out.println("Saving to file: " + paramFilePath);

	// open output file
	
	FileWriter fw = new FileWriter(paramFilePath);

	PrintWriter pwout = new PrintWriter(fw);
	
	pwout.println("<?xml version=\"1.0\" " +
		      "encoding=\"ISO-8859-1\" " +
		      "standalone=\"yes\"?>");
	pwout.println("<!DOCTYPE parameters [");
	pwout.println(" <!ELEMENT parameters (param+) >");
	pwout.println(" <!ELEMENT param" +
		      " (label?, description?, info?, value?, param*) >");
	pwout.println(" <!ELEMENT label (#PCDATA)>");
	pwout.println(" <!ELEMENT description (#PCDATA)>");
	pwout.println(" <!ELEMENT info (#PCDATA)>");
	pwout.println(" <!ELEMENT value (#PCDATA)>");
	pwout.println(" <!ATTLIST param name NMTOKEN #REQUIRED>");
	pwout.println(" <!ATTLIST param type NMTOKEN #REQUIRED>");

	pwout.println(" <!ATTLIST param type ( " +
		      ParamType.STRING + " | " +
		      ParamType.BOOLEAN + " | " +
		      ParamType.INTEGER + " | " +
		      ParamType.FLOAT + " | " +
		      ParamType.DOUBLE + " | " +
		      ParamType.OPTION + " | " +
		      ParamType.COLLECTION +
		      " ) #REQUIRED>");

	pwout.println("]>");
	pwout.println("");

	pwout.println("<parameters>");
	pwout.println("");
	
	printAsXml(pwout, 1);

	pwout.println("</parameters>");
	pwout.println("");
	pwout.close();

	setUnsavedChanges(false);
	_fileSaveAction.setEnabled(false);

    }

    // initialize from XML file
    
    public void initFromXml() {

	if (_paramFilePath == null) {
	    setUnsavedChanges(false);
	    return;
	}
	
	try {
	    readFromXml();
	}
	catch (JDOMException e) {
	    System.err.println(e);
	    return;
	}
    }

    // read from XML file

    public void readFromXml() throws JDOMException {
	
	if (_paramFilePath == null) {
	    throw new JDOMException("Null parameter file");
	}
	
	try {
	    readFromXml(_paramFilePath);
	}
	catch (JDOMException e) {
	    _paramFilePath = null;
	    _activeFileLabel.setText("Current param file: none");
	    throw e;
	}
	
    }

    public void readFromXml(String paramFilePath) throws JDOMException {
	
	try {
	    _doReadFromXml(paramFilePath);
	}
	catch (JDOMException e) {
	    // error - show error dialog
	    String errStr =
		"ParamManager.readFromXml(): \n" +
		"Cannot initialize params from file: " +
		paramFilePath + "\n" +
		"Will use default parameters";
	    JOptionPane.showMessageDialog(_topPanel, errStr,
					  "Bad param file",
					  JOptionPane.ERROR_MESSAGE);
	    throw e;
	}
	setUnsavedChanges(false);
	_fileSaveAction.setEnabled(false);

    }

    // read from XML using JDom

    private void _doReadFromXml(String paramFilePath)  throws JDOMException {
	
	SAXBuilder builder = new SAXBuilder();
	Document doc;
	
	try {
	    doc = builder.build(paramFilePath);
	    // If there are no well-formedness errors, 
	    // then no exception is thrown
	}
	// indicates a well-formedness error
	catch (JDOMException e) { 
	    System.err.println("ERROR - ParamManager.readFromXml");
	    System.err.println("  Parsing XML param file");
	    System.err.println("  " + paramFilePath + " is not well-formed.");
	    System.err.println("  " + e.getMessage());
	    throw e;
	}
	
	Element root = doc.getRootElement();
	List children = root.getChildren();
	Iterator iterator = children.iterator();
	if (iterator.hasNext()) {
	    Element topCollectionElement = (Element) iterator.next();
	    try {
		loadFromXml(topCollectionElement);
	    }
	    catch (Exception e) {
		String errMsg =
		    "ERROR - ParamManager.readFromXml\n" +
		    "  Parsing XML param file: " +
		    paramFilePath + "\n";
		System.err.println(errMsg + e.getMessage());
		throw new JDOMException(errMsg, e);
	    }
	}
	
    }

    private static void printSpaces(int n) {
	
	for (int i = 0; i < n; i++) {
	    System.out.print(' '); 
	}
	
    }

    // override applyChanges()

    public void applyChanges() {
	setFileSaveActionState();
    }
    
    // set the file save action enabled state

    public void setFileSaveActionState() {
	if (hasUnsavedChanges()) {
	    _fileSaveAction.setEnabled(true);
	} else {
	    _fileSaveAction.setEnabled(false);
	}
    }

    // check if there are unsaved changes to be saved.
    // If returns true, caller can proceed.
    // if returns false, caller should not proceed.
    
    public boolean checkUnsavedChanges() {

	if (hasUnsavedChanges()) {

	    if (_paramFilePath == null) {
		
		String title = "Checking on unsaved changes";
		String message[] = {
		    "Parameters have not been saved.\n",
		    "Save changes before proceeding?"
		};
		int result = JOptionPane.showConfirmDialog
		    (_topPanel, message, title,
		     JOptionPane.YES_NO_CANCEL_OPTION,
		     JOptionPane.WARNING_MESSAGE);
		if (result == JOptionPane.YES_OPTION) {
		    _fileSaveAsAction.actionPerformed(null);
		    return true;
		} else if (result == JOptionPane.NO_OPTION) {
		    return true;
		} else {
		    return false;
		}
		
	    } else { // if (_paramFilePath == null)
		
		String title = "Checking on unsaved changes";
		String message[] = {
		    "Parameters have not been saved.\n",
		    "Save changes in " + _paramFilePath + " before proceeding?"
		};
		int result = JOptionPane.showConfirmDialog
		    (_topPanel, message, title,
		     JOptionPane.YES_NO_CANCEL_OPTION,
		     JOptionPane.WARNING_MESSAGE);
		if (result == JOptionPane.YES_OPTION) {
		    _fileSaveAction.actionPerformed(null);
		    return true;
		} else if (result == JOptionPane.NO_OPTION) {
		    return true;
		} else {
		    return false;
		}
		
	    } // if (_paramFilePath == null)

	} else {
	    
	    return true;

	}

    }

    ////////////////////////////////////////////////////
    // Inner classes for actions
    ////////////////////////////////////////////////////

    // new file action

    private class NewAction extends AbstractAction
    {
	public NewAction(String label, ImageIcon icon)
	{
	    super(label, icon);
	}
	
	public void actionPerformed(ActionEvent event)
	{
	    if (!checkUnsavedChanges()) {
		return;
	    }
	    setValueFromDefault();
	    syncParam2Gui();
	    _paramFilePath = null;
	    _fileSaveAction.setEnabled(false);
	}
    }

    // file open action

    private class FileOpenAction extends AbstractAction
    {
	public FileOpenAction(String label, ImageIcon icon)
	{
	    super(label, icon);
	}
	
	public void actionPerformed(ActionEvent event)
	{
	    if (!checkUnsavedChanges()) {
		return;
	    }
	    JFileChooser chooser = new JFileChooser();
	    int state = chooser.showOpenDialog(null);
	    File file = chooser.getSelectedFile();
	    if (file != null &&
		state == JFileChooser.APPROVE_OPTION) {
		try {
		    readFromXml(file.getPath());
		}
		catch (JDOMException e) {
		    return;
		}
		_paramFilePath = file.getPath();
		_activeFileLabel.setText("Current param file: " + _paramFilePath);
	    }
	}
    }

    // edit action

    private class EditAction extends AbstractAction
    {
	
	public EditAction(String label, ImageIcon icon)
	{
	    super(label, icon);
	}
	
	public void actionPerformed(ActionEvent event)
	{
	    syncParam2Gui();
	    getCollectionFrame().setVisible(true);
	}
    }
    
    // file save action

    private class FileSaveAction extends AbstractAction
    {
	public FileSaveAction(String label, ImageIcon icon)
	{
	    super(label, icon);
	}
	
	public void actionPerformed(ActionEvent event)
	{
	    if (_paramFilePath == null) {
		// show error dialog
		String errStr = "Cannot save - no file specified";
		JOptionPane.showMessageDialog(_topPanel,
					      errStr,
					      "Illegal save",
					      JOptionPane.ERROR_MESSAGE);
	    } else {
		String title = "Saving parameters to file";
		String message[] = {
		    "Overwrite file: " + _paramFilePath
		};
		int result = JOptionPane.showConfirmDialog
		    (_topPanel, message, title,
		     JOptionPane.YES_NO_CANCEL_OPTION,
		     JOptionPane.WARNING_MESSAGE);
		if (result == JOptionPane.YES_OPTION) {
		    try {
			saveAsXml();
		    }
		    catch (IOException e) {
			// show error dialog
			String errStr = e.getMessage();
			JOptionPane.showMessageDialog
			    (_topPanel,
			     errStr,
			     "Illegal save",
			     JOptionPane.ERROR_MESSAGE);
			return;
		    }
		}
	    }
	}
    }

    // file save-as action

    private class FileSaveAsAction extends AbstractAction
    {
	public FileSaveAsAction(String label)
	{
	    super(label, null);
	}
	
	public void actionPerformed(ActionEvent event)
	{
	    JFileChooser chooser = new JFileChooser();
	    int state = chooser.showSaveDialog(null);
	    File file = chooser.getSelectedFile();
	    if (file != null &&
		state == JFileChooser.APPROVE_OPTION) {
		try {
		    saveAsXml(file.getPath());
		}
		catch (IOException e) {
		    // show error dialog
		    String errStr = e.getMessage();
		    JOptionPane.showMessageDialog
			(_topPanel,
			 errStr,
			 "Illegal save",
			 JOptionPane.ERROR_MESSAGE);
		    return;
		}
		_paramFilePath = file.getPath();
		_activeFileLabel.setText("Current param file: " + _paramFilePath);
	    }
		
	}
    }

}
