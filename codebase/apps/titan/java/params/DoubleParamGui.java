///////////////////////////////////////////////////////////////////////
//
// DoubleParamGui
//
// GUI class for DoubleParam
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import javax.swing.*;

public class DoubleParamGui extends AbstractParamGui

{

    private JTextField _jTextField;

    public DoubleParamGui(DoubleParam param) {
	super(param);
	_jTextField =
	    new JTextField(Double.toString(param.getValue()),
			   _textFieldLen);
	_selector = _jTextField;
	_selector.setToolTipText(param.getDescription());
    }

    public void syncParam2Gui() {
	DoubleParam dp = (DoubleParam) _param;
	_jTextField.setText(Double.toString(dp.getValue()));
    }
    
    public int syncGui2Param() {
	DoubleParam dp = (DoubleParam) _param;
	try {
	    dp.setValue(_jTextField.getText());
	}
	catch (java.lang.NumberFormatException e) {
	    // show error dialog
	    String errStr =
		"Illegal value for double parameter\n"
		+ dp.getLabel() + " = "	+ _jTextField.getText();
	    JOptionPane.showMessageDialog(_jTextField, errStr,
					  "Illegal Double Value",
					  JOptionPane.ERROR_MESSAGE);
	    return -1;
	}
	return 0;
    }

}
