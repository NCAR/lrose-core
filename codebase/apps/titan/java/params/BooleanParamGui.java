///////////////////////////////////////////////////////////////////////
//
// BooleanParamGui
//
// GUI class for BooleanParam
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import javax.swing.*;

public class BooleanParamGui extends AbstractParamGui

{

    private BooleanPanel _booleanPanel;

    public BooleanParamGui(BooleanParam param) {
	super(param);
	_booleanPanel = new BooleanPanel(param.getValue());
	_selector = _booleanPanel;
	_selector.setToolTipText(param.getDescription());
    }

    public void syncParam2Gui() {
	BooleanParam bp = (BooleanParam) _param;
	if (bp.getValue() == true) {
	    _booleanPanel.setTrue();
	} else { 
	    _booleanPanel.setFalse();
	}
    }
    
    public int syncGui2Param() {
	BooleanParam bp = (BooleanParam) _param;
	if (_booleanPanel.trueButton.isSelected()) {
	    bp.setValue(true);
	} else {
	    bp.setValue(false);
	}
	return 0;
    }

    class BooleanPanel extends JPanel {
	JRadioButton trueButton, falseButton;
	public BooleanPanel(boolean initState) {
	    ButtonGroup group = new ButtonGroup();
	    trueButton = new JRadioButton("true");
	    falseButton = new JRadioButton("false");
	    add(trueButton);
	    add(falseButton);
	    if (initState) {
		trueButton.setSelected(true);
	    } else {
		falseButton.setSelected(true);
	    }
	    group.add(trueButton);
	    group.add(falseButton);
	}
	public void setTrue() {
	    trueButton.setSelected(true);
	}
	public void setFalse() {
	    falseButton.setSelected(true);
	}
    }

}
