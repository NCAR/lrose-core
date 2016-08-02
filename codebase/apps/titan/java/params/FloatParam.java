///////////////////////////////////////////////////////////////////////
//
// FloatParam
//
// Float class for Params
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public class FloatParam extends AbstractParam

{

    private float _value;
    private float _default;
    private FloatParamGui _gui = null;

    public FloatParam(String name) {
	super(name);
    }

    public void setValue(float value) {
	_value = value;
	setUnsavedChanges(true);
    }

    public void setValue(double value) {
	_value = (float) value;
	setUnsavedChanges(true);
    }

    public void setValue(Float value) {
	_value = value.floatValue();
	setUnsavedChanges(true);
    }

    public void setValue(String value) {
	_value = Float.parseFloat(value);
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
	return ParamType.FLOAT;
    }

    public float getValue() {
	return _value;
    }

    public String toString() {
	Float val = new Float(_value);
	return val.toString();
    }
    
    public AbstractParamGui getGui() {
	if (_gui == null) {
	    _gui = new FloatParamGui(this);
	}
	return (AbstractParamGui) _gui;
    }

}
