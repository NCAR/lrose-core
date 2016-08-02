///////////////////////////////////////////////////////////////////////
//
// StringParamGui
//
// GUI class for StringParam
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import javax.swing.*;

public class StringParamGui extends AbstractParamGui

{

    private JTextField _jTextField;

    public StringParamGui(StringParam param) {
	super(param);
	_jTextField = new JTextField(param.getValue(), _textFieldLen);
	_selector = _jTextField;
	_selector.setToolTipText(param.getDescription());
    }

    public void syncParam2Gui() {
	StringParam sp = (StringParam) _param;
	_jTextField.setText(sp.getValue());
    }
    
    public int syncGui2Param() {
	StringParam sp = (StringParam) _param;
	sp.setValue(_jTextField.getText());
	return 0;
    }

}
