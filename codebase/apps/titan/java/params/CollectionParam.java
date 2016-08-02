///////////////////////////////////////////////////////////////////////
//
// CollectionParam
//
// Collection class for Params
//
// Mike Dixon
//
// October 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import java.io.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import org.jdom.*;

public class CollectionParam
    extends AbstractParam
    
{

    private int _depth;
    private ArrayList _paramList = new ArrayList(); // array of param objects
    private CollectionParamGui _gui = null;
    private CollectionFrame _frame;
    private CollectionFrame _parentFrame;
    
    public CollectionParam(String name, String label, int depth) {

	super(name);
	setLabel(label);
	_depth = depth;
	_frame = new CollectionFrame(this, getName(), getLabel(), _depth);

    }

    // applyChanges() is no-op - this is overridded in the
    // ParamManager class

    public void applyChanges() {
    }
    
    // setValue() is no-op for this class

    public void setValue(String value) {
    }
    
    // add a parameter to the list

    public void add(AbstractParam param) {
	_paramList.add(param);
    }
    
    // after adding all the elements, activate the frames recursively

    public void activateFrame() {
	
	_frame.clear();
	for (int ii = 0; ii < getNParams(); ii++) {
	    AbstractParam aparam = getParam(ii);
	    if (aparam.getType() == ParamType.COLLECTION) {
		CollectionParam cparam = (CollectionParam) aparam;
		cparam.activateFrame();
	    }
	    _frame.add(getParam(ii).getGui());
	}
	_frame.activate();

    }
    
    // copy values to/from the defaults

    public void setValueFromDefault() {
	for (int ii = 0; ii < getNParams(); ii++) {
	    AbstractParam aparam = getParam(ii);
	    aparam.setValueFromDefault();
	}
    }

    public void setDefaultFromValue() {
	for (int ii = 0; ii < getNParams(); ii++) {
	    AbstractParam aparam = getParam(ii);
	    aparam.setDefaultFromValue();
	}
    }

    // get type

    public ParamType getType() {
	return ParamType.COLLECTION;
    }

    // get this and next depths

    public int getDepth() {
	return _depth;
    }
    public int getNextDepth() {
	return _depth + 1;
    }

    // get number of params
    
    public int getNParams() {
	return _paramList.size();
    }

    // get a param from the list
    
    public AbstractParam getParam(int index)
	throws IndexOutOfBoundsException {
	return (AbstractParam) _paramList.get(index);
    }
    
    // get the GUI for this object
    // The creation of the GUI is delayed until the first use so that
    // the properties of the object, such as label and description,
    // can be set first.

    public AbstractParamGui getGui() {
 	if (_gui == null) {
 	    _gui = new CollectionParamGui(this);
 	}
	return (AbstractParamGui) _gui;
    }

    // get the Collection frame
    
    public CollectionFrame getCollectionFrame() {
	return _frame;
    }

    // override setUnsavedChanges

    public void setUnsavedChanges(boolean unsaved) {
	for (int ii = 0; ii < getNParams(); ii++) {
	    getParam(ii).setUnsavedChanges(unsaved);
	}
    }
    
    // override hasUnsavedChanges()
    // are there unsaved changes in this collection?

    public boolean hasUnsavedChanges() {
	for (int ii = 0; ii < getNParams(); ii++) {
	    if (getParam(ii).hasUnsavedChanges()) {
		return true;
	    }
	}
	return false;
    }

    // sync param values to gui

    public void syncParam2Gui() {
	for (int ii = 0; ii < getNParams(); ii++) {
	    getParam(ii).syncParam2Gui();
	}
    }

    // print as XML

    public void printAsXml(PrintWriter pwout, int depth) {

	StringBuffer spacer = new StringBuffer();
	for (int i = 0; i < depth; i++) {
	    spacer.append("  ");
	}

	pwout.println(spacer + "<!-- ====== start of collection "
		      + getName() + " ====== -->");
	pwout.println("");
	printXmlHdr(pwout, spacer);
	pwout.println("");
	
	for (int ii = 0; ii < getNParams(); ii++) {
	    getParam(ii).printAsXml(pwout, depth + 1);
	}

	pwout.println(spacer + "</param>");
	pwout.println("");
	pwout.println(spacer + "<!-- ======= end of collection " +
		      getName() + " ======= -->");
	pwout.println("");
	
    }

    // load from JDom XML

    public void loadFromXml(Element top)

    {
	
	// make sure the top element points to a collection
	// of the correct name
	
	String nameAtt = top.getAttribute("name").getValue();
	String typeAtt = top.getAttribute("type").getValue();
	
	if (!nameAtt.equals(getName())) {
	    System.err.println("ERROR - incorrect name: " + nameAtt);
	    System.err.println("        Expecting name: " + getName());
	    return;
	}
 	if (!typeAtt.equals("Collection")) {
 	    System.err.println("ERROR - incorrect type: " + typeAtt);
 	    System.err.println("        Expecting type: " + "Collection");
 	    return;
	}
	
	// iterate through the params in this collection,
	// matching up with the correct XML entry
	
	for (int ii = 0; ii < getNParams(); ii++) {
	    
	    AbstractParam aparam = getParam(ii);
	    Iterator itt = top.getChildren().iterator();
	    while (itt.hasNext()) {
		Element child = (Element) itt.next();
		if (child.getName().equals("param")) {
		    Attribute childNameAtt = child.getAttribute("name");
		    if (childNameAtt != null &&
			childNameAtt.getValue().equals(aparam.getName())) {
			loadParam(child, aparam);
			break;
		    }
		}
	    } // while

	} // ii

    }
    
    private void loadParam(Element top, AbstractParam aparam) {

	// is this a collection?
	
	String nameAtt = top.getAttribute("name").getValue();
	String typeAtt = top.getAttribute("type").getValue();
	if (typeAtt.equals("Collection")) {
	    CollectionParam cparam = (CollectionParam) aparam;
	    cparam.loadFromXml(top);
	    return;
	}

	List children = top.getChildren();
	Iterator itt = children.iterator();

	// set value from the XML text
	
	Iterator jtt = children.iterator();
	while (jtt.hasNext()) {
	    Element child = (Element) jtt.next();
	    if (child.getName().equals("value")) {
		aparam.setValue(child.getText());
	    }
	}
	
    }

    private boolean collectionMatches(Element top, AbstractParam aparam) {

	List children = top.getChildren();
	Iterator itt = children.iterator();
	boolean nameMatches = false;
	while (itt.hasNext()) {
	    Element child = (Element) itt.next();
	    if (child.getName().equals("name") &&
		child.getText().equals(aparam.getName())) {
		nameMatches = true;
		break;
	    }
	}
	
	if (!nameMatches) {
	    return false;
	}
	
	CollectionParam cparam = (CollectionParam) aparam;
	cparam.loadFromXml(top);
	return true;
	
    }


    public void listChildren(Element current) {
	
	System.out.println("--->Child:");
	System.out.println("    xml name: " + current.getName());
	System.out.println("    xml text: " + current.getText());
	List children = current.getChildren();
	Iterator itt = children.iterator();
	while (itt.hasNext()) {
	    Element child = (Element) itt.next();
	    // listChildren(child);
	}
	
    }
    
}
