///////////////////////////////////////////////////////////////////////
//
// FloatParamGui
//
// GUI class for FloatParam
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import javax.swing.*;

public class FloatParamGui extends AbstractParamGui

{

    private JTextField _jTextField;

    public FloatParamGui(FloatParam param) {
	super(param);
	_jTextField =
	    new JTextField(Float.toString(param.getValue()),
			   _textFieldLen);
	_selector = _jTextField;
	_selector.setToolTipText(param.getDescription());
    }

    public void syncParam2Gui() {
	FloatParam fp = (FloatParam) _param;
	_jTextField.setText(Float.toString(fp.getValue()));
    }
    
    public int syncGui2Param() {
	FloatParam fp = (FloatParam) _param;
	try {
	    fp.setValue(_jTextField.getText());
	}
	catch (java.lang.NumberFormatException e) {
	    // show error dialog
	    String errStr =
		"Illegal value for float parameter\n"
		+ fp.getLabel() + " = "	+ _jTextField.getText();
	    JOptionPane.showMessageDialog(_jTextField, errStr,
					  "Illegal Float Value",
					  JOptionPane.ERROR_MESSAGE);
	    return -1;
	}
	return 0;
    }

}
