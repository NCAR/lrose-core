///////////////////////////////////////////////////////////////////////
//
// BooleanParam
//
// Boolean class for Params
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public class BooleanParam extends AbstractParam

{

    private boolean _value;
    private boolean _default;
    private BooleanParamGui _gui = null;

    public BooleanParam(String name) {
	super(name);
    }

    public void setValue(boolean value) {
	_value = value;
	setUnsavedChanges(true);
    }

    public void setValue(Boolean value) {
	_value = value.booleanValue();
	setUnsavedChanges(true);
    }

    public void setValue(String value) {
	_value = Boolean.valueOf(value).booleanValue();
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
	return ParamType.BOOLEAN;
    }

    public boolean getValue() {
	return _value;
    }

    public String toString() {
	Boolean val = new Boolean(_value);
	return val.toString();
    }

    public AbstractParamGui getGui() {
	if (_gui == null) {
	    _gui = new BooleanParamGui(this);
	}
	return (AbstractParamGui) _gui;
    }
}
