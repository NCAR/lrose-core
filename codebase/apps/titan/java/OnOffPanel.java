///////////////////////////////////////////////////////////////////////
//
// OnOffPanel
//
// JPanel with on/off behavior
//
// Mike Dixon
//
// Nov 2002
//
////////////////////////////////////////////////////////////////////////

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

import params.*;

class OnOffPanel extends JPanel {
    
    private String _labelStr;
    private String _commandStr;
    private MessageQueue _guiCommands;
    private ImageIcon _onIcon, _offIcon;
    private JLabel _label, _indicator;
    private JRadioButton _onButton, _offButton;
    private ButtonGroup _buttonGroup;
    private long _timeLastSet = 0;
    private boolean _state = false;
    private boolean _indicatorState = false;
    
    public OnOffPanel(String labelStr,
		      String commandStr,
		      boolean initState,
		      MessageQueue guiCommands) {

	_labelStr = labelStr;
	_commandStr = commandStr;
	_guiCommands = guiCommands;
	
	_offIcon = new ImageIcon(OnOffPanel.class.getResource("./off_light.png"));
	_onIcon = new ImageIcon(OnOffPanel.class.getResource("./on_light.png"));

	_label = new JLabel(labelStr);
	_indicator = new JLabel(_offIcon);
	_onButton = new JRadioButton("On");
	_offButton = new JRadioButton("Off");
	
	add(_label);
	add(_offButton);
	add(_onButton);
	add(_indicator);
	
	_state = initState;
	_indicatorState = initState;
	if (_state) {
	    _onButton.setSelected(true);
	    _indicator.setIcon(_onIcon);
	} else {
	    _offButton.setSelected(true);
	}
	_timeLastSet = TimeManager.getTime();
	
	_buttonGroup = new ButtonGroup();
	_buttonGroup.add(_onButton);
	_buttonGroup.add(_offButton);
	
	ActionListener offListener = new OffListener();
	_offButton.addActionListener(offListener);
	
	ActionListener onListener = new OnListener();
	_onButton.addActionListener(onListener);
	
    }
    
    public int getPreferredLabelWidth() {
	return (int) _label.getPreferredSize().getWidth();
    }

    public void setPreferredLabelWidth(int width) {
	Dimension size = _label.getPreferredSize();
	size.width = width;
	_label.setPreferredSize(size);
    }

    public void setIndicator(boolean state) {
	synchronized(_indicator) {
	    if (_indicatorState != state) {
		if (state) {
		    _indicator.setIcon(_onIcon);
		} else {
		    _indicator.setIcon(_offIcon);
		}
	    }
	    _indicatorState = state;
	    if (_indicatorState != _state) {
		long now = TimeManager.getTime();
		if ((now - _timeLastSet) > 5000) {
		    setState(_indicatorState);
		    if (_state) {
			_onButton.setSelected(true);
		    } else {
			_offButton.setSelected(true);
		    }
		}
	    }
	}
    }
    
    private void setState(boolean state) {
	synchronized(_buttonGroup) {
	    _state = state;
	    _timeLastSet = TimeManager.getTime();
	    _onButton.setSelected(state);
	    AsciiMessage command;
	    if (_state) {
		command = new AsciiMessage(_commandStr, "on");
	    } else {
		command = new AsciiMessage(_commandStr, "off");
	    }
	    _guiCommands.push(command);
	}
    }
    
    class OffListener implements ActionListener {
	public void actionPerformed(ActionEvent e) {
	    setState(false);
	}
    }
    
    class OnListener implements ActionListener {
	public void actionPerformed(ActionEvent e) {
	    setState(true);
	}
    }
    
}

