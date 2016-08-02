///////////////////////////////////////////////////////////////////////
//
// OptionParamGui
//
// GUI class for OptionParam
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import javax.swing.*;

public class OptionParamGui extends AbstractParamGui

{

    private JComboBox _jComboBox;
    
    public OptionParamGui(OptionParam param) {
	super(param);
	_jComboBox = new JComboBox(param.getOptions());
	_selector = _jComboBox;
	_selector.setToolTipText(param.getDescription());
	String selected = param.getValue();
	for (int ii = 0; ii < _jComboBox.getItemCount(); ii++) {
	    if (selected.equals(_jComboBox.getItemAt(ii))) {
		_jComboBox.setSelectedIndex(ii);
	    }
	}
    }

    public void syncParam2Gui() {
	OptionParam op = (OptionParam) _param;
	String selected = op.getValue();
	for (int ii = 0; ii < _jComboBox.getItemCount(); ii++) {
	    if (selected.equals(_jComboBox.getItemAt(ii))) {
		_jComboBox.setSelectedIndex(ii);
	    }
	}
    }
    
    public int syncGui2Param() {
	OptionParam op = (OptionParam) _param;
	String selected = (String) _jComboBox.getSelectedItem();
	op.setValue(selected);
	return 0;
    }

}

