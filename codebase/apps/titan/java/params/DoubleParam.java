///////////////////////////////////////////////////////////////////////
//
// DoubleParam
//
// Double class for Params
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public class DoubleParam extends AbstractParam

{

    private double _value;
    private double _default;
    private DoubleParamGui _gui = null;

    public DoubleParam(String name) {
	super(name);
    }

    public void setValue(float value) {
	_value = value;
	setUnsavedChanges(true);
    }

    public void setValue(double value) {
	_value = (double) value;
	setUnsavedChanges(true);
    }

    public void setValue(Double value) {
	_value = value.doubleValue();
	setUnsavedChanges(true);
    }

    public void setValue(String value) {
	_value = Double.parseDouble(value);
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
	return ParamType.DOUBLE;
    }

    public double getValue() {
	return _value;
    }

    public String toString() {
	Double val = new Double(_value);
	return val.toString();
    }

    public AbstractParamGui getGui() {
	if (_gui == null) {
	    _gui = new DoubleParamGui(this);
	}
	return (AbstractParamGui) _gui;
    }

}
