///////////////////////////////////////////////////////////////////////
//
// IntegerParam
//
// Integer class for Params
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public class IntegerParam extends AbstractParam

{

    private int _value;
    private int _default;
    private IntegerParamGui _gui = null;

    public IntegerParam(String name) {
	super(name);
    }

    public void setValue(int value) {
	_value = value;
	setUnsavedChanges(true);
    }

    public void setValue(Integer value) {
	_value = value.intValue();
	setUnsavedChanges(true);
    }

    public void setValue(String value) {
	_value = Integer.parseInt(value);
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
	return ParamType.INTEGER;
    }

    public int getValue() {
	return _value;
    }

    public String toString() {
	Integer val = new Integer(_value);
	return val.toString();
    }

    public AbstractParamGui getGui() {
	if (_gui == null) {
	    _gui = new IntegerParamGui(this);
	}
	return (AbstractParamGui) _gui;
    }

}
