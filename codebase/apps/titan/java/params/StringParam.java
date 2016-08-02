///////////////////////////////////////////////////////////////////////
//
// StringParam
//
// String class for Params
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public class StringParam extends AbstractParam

{

    private String _value;
    private String _default;
    private StringParamGui _gui = null;
    
    public StringParam(String name) {
	super(name);
    }
    
    public void setValue(String value) {
	_value = new String(value);
	setUnsavedChanges(true);
    }

    public void setValueFromDefault() {
	_value = _default;
	setUnsavedChanges(true);
    }

    public void setDefaultFromValue() {
	_default = _value;
    }

    public ParamType getType() {
	return ParamType.STRING;
    }

    public String getValue() {
	return _value;
    }

    public String toString() {
	return new String(_value);
    }
    
    public AbstractParamGui getGui() {
	if (_gui == null) {
	    _gui = new StringParamGui(this);
	}
	return (AbstractParamGui) _gui;
    }

}
