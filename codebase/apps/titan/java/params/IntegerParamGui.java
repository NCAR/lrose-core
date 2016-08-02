///////////////////////////////////////////////////////////////////////
//
// IntegerParamGui
//
// GUI class for IntegerParam
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import javax.swing.*;

public class IntegerParamGui extends AbstractParamGui

{

    private JTextField _jTextField;

    public IntegerParamGui(IntegerParam param) {
	super(param);
	_jTextField =
	    new JTextField(Integer.toString(param.getValue()),
			   _textFieldLen);
	_selector = _jTextField;
	_selector.setToolTipText(param.getDescription());
    }

    public void syncParam2Gui() {
	IntegerParam ip = (IntegerParam) _param;
	_jTextField.setText(Integer.toString(ip.getValue()));
    }
    
    public int syncGui2Param() {
	IntegerParam ip = (IntegerParam) _param;
	try {
	    ip.setValue(_jTextField.getText());
	}
	catch (java.lang.NumberFormatException e) {
	    // show error dialog
	    String errStr =
		"Illegal value for integer parameter\n"
		+ ip.getLabel() + " = "	+ _jTextField.getText();
	    JOptionPane.showMessageDialog(_jTextField, errStr,
					  "Illegal Integer Value",
					  JOptionPane.ERROR_MESSAGE);
	    return -1;
	}
	return 0;
    }

}

