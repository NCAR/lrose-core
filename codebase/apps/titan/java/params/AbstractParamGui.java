///////////////////////////////////////////////////////////////////////
//
// AbstractParamGui
//
// Abstract base class for Params GUI
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

abstract class AbstractParamGui implements ParamGuiInterface

{

    private JLabel _jLabel;
    private JButton _infoButton;
    static final protected int _textFieldLen = 24;
    protected JComponent _selector;
    protected AbstractParam _param;

    public AbstractParamGui(AbstractParam param) {
	_param = param;
	_jLabel = new JLabel(param.getLabel() + ": ");
	_jLabel.setToolTipText(param.getDescription());
	ImageIcon infoIcon =
	    new ImageIcon(this.getClass().getResource("./info.png"));
	_infoButton = new JButton("", infoIcon);
	_infoButton.setToolTipText("Click for full info on "
				   + param.getLabel());
	_infoButton.addActionListener(new InfoListener(_param));
    }

    public JLabel getJLabel() {
	return _jLabel;
    }

    public AbstractParam getParam() {
	return _param;
    }

    public void addTo3ColLayout(GridBagLayout gridbag,
				Container container) {
	
        GridBagConstraints c = new GridBagConstraints();
	
	c.insets = new Insets(2, 3, 3, 2);
        c.anchor = GridBagConstraints.EAST;
	c.gridwidth = 1;
	c.fill = GridBagConstraints.NONE;      //reset to default
	c.weightx = 0.0;                       //reset to default
	gridbag.setConstraints(_jLabel, c);
	container.add(_jLabel);
	
        c.anchor = GridBagConstraints.WEST;
	c.gridwidth = GridBagConstraints.RELATIVE; //next-to-last
	c.fill = GridBagConstraints.NONE;      //reset to default
	c.weightx = 0.0;                       //reset to default
	gridbag.setConstraints(_selector, c);
	container.add(_selector);
	
        c.anchor = GridBagConstraints.CENTER;
	c.gridwidth = GridBagConstraints.REMAINDER;     //end row
	// c.fill = GridBagConstraints.HORIZONTAL;
	c.fill = GridBagConstraints.NONE;
	c.weightx = 1.0;
	gridbag.setConstraints(_infoButton, c);
	container.add(_infoButton);
    }

    // Inner classes for listeners

    private class InfoListener implements ActionListener
    {
	private AbstractParam _param;
	public InfoListener(AbstractParam param) {
	    _param = param;
	}
	public void actionPerformed(ActionEvent event)
	{
	    ParamInfoFrame infoFrame = ParamInfoFrame.getInstance();
	    infoFrame.show(_param);
	}
    }

}
