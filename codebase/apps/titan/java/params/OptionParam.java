///////////////////////////////////////////////////////////////////////
//
// OptionParam
//
// Option class for Params
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public class OptionParam extends AbstractParam

{
    
    private String _value;
    private String _default;
    private String[] _options;
    private OptionParamGui _gui = null;

    public OptionParam(String name) {
	super(name);
    }

    public void setOptions(String[] options) {
	_options = new String[options.length];
	for (int i = 0; i < options.length; i++) {
	    _options[i] = new String(options[i]);
	}
    }
    
    public void setValue(String value) throws IllegalArgumentException {
	boolean valid = false;
	for (int i = 0; i < _options.length; i++) {
	    if (value.compareTo(_options[i]) == 0) {
		valid = true;
		break;
	    }
	}
	if (valid == false) {
	    throw new IllegalArgumentException(value + " not a valid option");
	}
	_value = value;
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
	return ParamType.OPTION;
    }

    public String getValue() {
	return _value;
    }

    public String toString() {
	return new String(_value);
    }

    public String[] getOptions() {
	String[] options = new String[_options.length];
	for (int ii = 0; ii < _options.length; ii++) {
	    options[ii] = new String(_options[ii]);
	}
	return options;
    }

    public AbstractParamGui getGui() {
	if (_gui == null) {
	    _gui = new OptionParamGui(this);
	}
	return (AbstractParamGui) _gui;
    }

}
