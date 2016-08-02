///////////////////////////////////////////////////////////////////////
//
// AbstractParam
//
// Abstract base class for Params
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import java.io.*;

abstract class AbstractParam implements ParamInterface

{
    
    private String _name;                         // name of param variable
    private String _label = new String("");       // label for GUI
    private String _description = new String(""); // tooltip - simple info
    private String _info = new String("");        // detailed info
    private boolean _unsavedChanges = false;

    public AbstractParam(String name) {
	_name = new String(name);
    }
    
    public void setLabel(String label) {
        _label = new String(label);
    }
    
    public void setDescription(String description) {
        _description = new String(description);
    }
    
    public void setInfo(String info) {
        _info = new String(info);
    }

    public String getName() {
	return _name;
    }

    public String getLabel() {
	return _label;
    }

    public String getDescription() {
	return _description;
    }

    public String getInfo() {
	return _info;
    }

    // unsaved changes

    protected void setUnsavedChanges(boolean unsaved) {
	_unsavedChanges = unsaved;
    }
    
    public boolean hasUnsavedChanges() {
	return _unsavedChanges;
    }

    // sync param values to gui

    public void syncParam2Gui() {
	getGui().syncParam2Gui();
    }

    // print in XML form
    
    public void printAsXml(PrintWriter pwout, int depth) {

	StringBuffer spacer = new StringBuffer();
	for (int i = 0; i < depth; i++) {
	    spacer.append("  ");
	}

	printXmlHdr(pwout, spacer);
	pwout.println(spacer + "  <value>" + toString() + "</value>");
	pwout.println(spacer + "</param>");
	pwout.println("");
	
    }

    protected void printXmlHdr(PrintWriter pwout, StringBuffer spacer) {
	
	pwout.println(spacer +
		      "<param" +
		      " name=\"" + getName() + "\"" +
		      " type=\"" + getType() + "\"" +
		      ">");
	pwout.println(spacer + "  <label>" + getLabel() + "</label>");
	pwout.println(spacer + "  <description>" + getDescription() +
		      "</description>");
	if (getInfo().length() > 0) {
	    pwout.println(spacer + "  <info>");
	    pwout.println(spacer + "    <![CDATA[");
	    pwout.println(spacer + "    " + getInfo());
	    pwout.println(spacer + "    ]]>");
	    pwout.println(spacer + "  </info>");
	}
	
    }

}
