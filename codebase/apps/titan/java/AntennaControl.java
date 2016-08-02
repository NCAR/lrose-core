///////////////////////////////////////////////////////////////////////
//
// AntennaControl
//
// JFrame with ParamManager member
//
// Mike Dixon
//
// Oct 2002
//
////////////////////////////////////////////////////////////////////////

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.text.*;
import javax.swing.*;
import javax.swing.border.*;
import java.util.*;
import java.lang.Math;

import params.*;

public class AntennaControl extends JPanel

{

    private Parameters _params;
    private MessageQueue _guiCommands;

    private AntennaPosnDisplay _antennaPosnDisplay;
    private ManualSetPanel _manualSetPanel;
    private SunPosnDisplay _sunPosnDisplay;
    private AntennaModePanel _antennaModePanel;
    private SlewRatePanel _slewRatePanel;
    private double _lastAntennaUpdateTime = 0.0;
    private double _elLast = 0.0;
    private double _azLast = 0.0;
    private AntennaMode _antennaMode = AntennaMode.MANUAL;

    public AntennaControl(JFrame frame,
			  Parameters params,
			  MessageQueue guiCommands) {
	
	_params = params;
	_guiCommands = guiCommands;
	
	setLayout(new BorderLayout());
	setBorder(CustomBorder.createTop(frame, "Antenna control", 5));
	
	// create sub panels
	
	_antennaPosnDisplay = new AntennaPosnDisplay(frame);
	_manualSetPanel = new ManualSetPanel(frame);
	_sunPosnDisplay = new SunPosnDisplay(frame);
	_antennaModePanel = new AntennaModePanel(frame);
	_slewRatePanel = new SlewRatePanel(frame);
	
	// combine mode panel and slew rate panel
	
	JPanel modeAndSlewPanel = new JPanel(new BorderLayout());
	modeAndSlewPanel.add(_antennaModePanel, BorderLayout.NORTH);
	modeAndSlewPanel.add(_slewRatePanel, BorderLayout.SOUTH);
	
	// add to main panel
	
	add(_antennaPosnDisplay, BorderLayout.NORTH);
	add(_manualSetPanel, BorderLayout.CENTER);
	add(_sunPosnDisplay, BorderLayout.SOUTH);
	add(modeAndSlewPanel, BorderLayout.WEST);
	
    }
    
    public void setSunPosition(double el, double az) {
	_sunPosnDisplay.setEl(el);
	_sunPosnDisplay.setAz(az);
	if (_antennaMode == AntennaMode.FOLLOW_SUN) {
	    if (el < _params.radar.antMinElev.getValue()) {
		el = _params.radar.antMinElev.getValue();
	    }
	    if (el > _params.radar.antMaxElev.getValue()) {
		el = _params.radar.antMaxElev.getValue();
	    }
	    AsciiMessage elCommand =
		new AsciiMessage("elevation", NFormat.el.format(el));
	    _guiCommands.push(elCommand);
	    AsciiMessage azCommand =
		new AsciiMessage("azimuth", NFormat.az.format(az));
	    _guiCommands.push(azCommand);
	}
    }
    
    public void setAntennaPosition(double el, double az) {
	double now = TimeManager.getTime();
	if ((now - _lastAntennaUpdateTime) > 100) {
	    _antennaPosnDisplay.setEl(el);
	    _antennaPosnDisplay.setAz(az);
	    _lastAntennaUpdateTime = now;
	    _elLast = el;
	    _azLast = az;
	}
    }
	
    public void setManualEntryToLastPosition() {
	_manualSetPanel.setEntryPosition(_elLast, _azLast);
    }

    // manual position panel - inner class

    private class ManualSetPanel extends JPanel {

	private JTextField _elTextField;
	private JTextField _azTextField;
	private double _targetEl = 0.0;
	private double _targetAz = 0.0;
	
	private JButton _up;
	private JButton _down;
	private JButton _left;
	private JButton _right;
	    
	private DeltaDegPanel _deltaDegPanel;

	private AzEditListener _azEditListener;
	private ElEditListener _elEditListener;
	
	private double _deltaDeg;

	public ManualSetPanel(JFrame frame) {
	    
	    setLayout(new BorderLayout());
	    setBorder(CustomBorder.createTop(frame, "Manual set", 3));
	    
	    Font defaultFont = getFont();
	    Font midFont =
		defaultFont.deriveFont((float) (defaultFont.getSize() + 0));
	    Font bigFont =
		defaultFont.deriveFont((float) (defaultFont.getSize() + 2));
	    
	    // elevation entry panel
	    
	    JLabel elLabel = new JLabel("EL", JLabel.CENTER);
	    elLabel.setFont(midFont);
	    
	    _elTextField = new JTextField("00.00", 5);
	    _elTextField.setToolTipText("Set desired elevation");
	    _elEditListener = new ElEditListener();
	    _elTextField.addActionListener(_elEditListener);

	    BorderLayout elLayout = new BorderLayout();
	    elLayout.setVgap(3);
	    JPanel elPanel = new JPanel(elLayout);
	    elPanel.add(elLabel, BorderLayout.NORTH);
	    elPanel.add(_elTextField, BorderLayout.SOUTH);
	    
	    // azimuth entry panel
	    
	    JLabel azLabel = new JLabel("AZ", JLabel.CENTER);
	    azLabel.setFont(midFont);
	    
	    _azTextField = new JTextField("000.00", 5);
	    _azTextField.setToolTipText("Set desired azimuth");
	    _azEditListener = new AzEditListener();
	    _azTextField.addActionListener(_azEditListener);
	    
	    BorderLayout azLayout = new BorderLayout();
	    azLayout.setVgap(3);
	    JPanel azPanel = new JPanel(azLayout);
	    azPanel.add(azLabel, BorderLayout.NORTH);
	    azPanel.add(_azTextField, BorderLayout.SOUTH);
	    
	    // combine entry panels
	    
	    BorderLayout comboLayout = new BorderLayout();
	    comboLayout.setHgap(10);
	    JPanel entryPanel = new JPanel(comboLayout);
	    entryPanel.add(elPanel, BorderLayout.WEST);
	    entryPanel.add(azPanel, BorderLayout.EAST);
	    JPanel entryContainer = new JPanel();
	    entryContainer.add(entryPanel);
	    entryContainer.setBorder(CustomBorder.createTop(frame, "Enter position", 3));
	    
	    // arrow panel
	    
	    JPanel arrowPanel = new JPanel(new BorderLayout());
	    ImageIcon upIcon =
		new ImageIcon(ControlPanel.class.getResource("./up.png"));
	    ImageIcon downIcon =
		new ImageIcon(ControlPanel.class.getResource("./down.png"));
	    ImageIcon leftIcon =
		new ImageIcon(ControlPanel.class.getResource("./left.png"));
	    ImageIcon rightIcon =
		new ImageIcon(ControlPanel.class.getResource("./right.png"));
	    
	    _up = new JButton(upIcon);
	    _down = new JButton(downIcon);
	    _left = new JButton(leftIcon);
	    _right = new JButton(rightIcon);
	    
	    _up.setToolTipText("Click to go up, or ALT up-arrow");
	    _up.setMnemonic(KeyEvent.VK_UP);
	    _down.setToolTipText("Click to go down, or ALT down-arrow");
	    _down.setMnemonic(KeyEvent.VK_DOWN);
	    _left.setToolTipText("Click to go left, or ALT left-arrow");
	    _left.setMnemonic(KeyEvent.VK_LEFT);
	    _right.setToolTipText("Click to go right, or ALT right-arrow");
	    _right.setMnemonic(KeyEvent.VK_RIGHT);

	    _up.addActionListener(new UpListener());
	    _down.addActionListener(new DownListener());
	    _left.addActionListener(new LeftListener());
	    _right.addActionListener(new RightListener());
	    
	    arrowPanel.add(_up, BorderLayout.NORTH);
	    arrowPanel.add(_down, BorderLayout.SOUTH);
	    arrowPanel.add(_left, BorderLayout.WEST);
	    arrowPanel.add(_right, BorderLayout.EAST);

	    JPanel arrowContainer = new JPanel();
	    arrowContainer.add(arrowPanel);

	    // adj res panel

	    _deltaDegPanel = new DeltaDegPanel(frame);

	    // put it all together

	    add(arrowContainer, BorderLayout.NORTH);
	    add(_deltaDegPanel, BorderLayout.CENTER);
	    add(entryContainer, BorderLayout.SOUTH);

	    // disable at startup if not manual
	    
	    if (_antennaMode != AntennaMode.MANUAL) {
		setEnabled(false);
	    }

	} // constructor
	
	// el edit action
	class ElEditListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		setElFromTextField();
	    }
	}

	// az edit action
	class AzEditListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		setAzFromTextField();
	    }
	}
	
	// up action listener
	class UpListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		_targetEl += _deltaDeg;
		long rounded = Math.round(_targetEl / _deltaDeg);
		_targetEl = rounded * _deltaDeg;
		if (_targetEl > _params.radar.antMaxElev.getValue()) {
		    _targetEl = _params.radar.antMaxElev.getValue();
		}
		_elTextField.setText(NFormat.el.format(_targetEl));
		setElFromTextField();
	    }
	}

	// down action listener
	class DownListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		_targetEl -= _deltaDeg;
		long rounded = Math.round(_targetEl / _deltaDeg);
		_targetEl = rounded * _deltaDeg;
		if (_targetEl < _params.radar.antMinElev.getValue()) {
		    _targetEl = _params.radar.antMinElev.getValue();
		}
		_elTextField.setText(NFormat.el.format(_targetEl));
		setElFromTextField();
	    }
	}

	// left action listener
	class LeftListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		_targetAz -= _deltaDeg;
		long rounded = Math.round(_targetAz / _deltaDeg);
		_targetAz = rounded * _deltaDeg;
		if (_targetAz < 0) {
		    _targetAz += 360.0;
		}
		if (Math.abs(_targetAz - 0) < 0.00001 ||
		    Math.abs(_targetAz - 360) < 0.00001) {
		    _targetAz = 0.0;
		}
		_azTextField.setText(NFormat.az.format(_targetAz));
		setAzFromTextField();
	    }
	}

	// right action listener

	class RightListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		_targetAz += _deltaDeg;
		long rounded = Math.round(_targetAz / _deltaDeg);
		_targetAz = rounded * _deltaDeg;
		if (_targetAz >= 360) {
		    _targetAz -= 360.0;
		}
		if (Math.abs(_targetAz - 0) < 0.00001 ||
		    Math.abs(_targetAz - 360) < 0.00001) {
		    _targetAz = 0.0;
		}
		_azTextField.setText(NFormat.az.format(_targetAz));
		setAzFromTextField();
	    }
	}

	// set the el from field

	public void setElFromTextField() { 
	    double el = 0.0;
	    boolean error = false;
	    try {
		el = Double.parseDouble(_elTextField.getText());
	    }
	    catch (java.lang.NumberFormatException nfe) {
		error = true;
	    }
	    if (el < _params.radar.antMinElev.getValue() ||
		el > _params.radar.antMaxElev.getValue()) {
		error = true;
	    }
	    if (error) {
		// show error dialog
		String errStr =
		    "Illegal value for elevation: " +
		    _elTextField.getText() + "\n" +
		    "Min el: " + _params.radar.antMinElev.getValue() + "\n" +
		    "Max el: " + _params.radar.antMaxElev.getValue() + "\n";
		JOptionPane.showMessageDialog(_elTextField, errStr,
					      "Illegal Double Value",
					      JOptionPane.ERROR_MESSAGE);
		return;
	    }
	    _targetEl = el;
	    AsciiMessage command =
		new AsciiMessage("elevation", NFormat.el.format(_targetEl));
	    _guiCommands.push(command);
	}
	
	// set the az from field

	public void setAzFromTextField() { 
	    double az = 0.0;
	    boolean error = false;
	    try {
		az = Double.parseDouble(_azTextField.getText());
	    }
	    catch (java.lang.NumberFormatException nfe) {
		error = true;
	    }
	    if (az < 0.0 || az > 360.0) {
		error = true;
	    }
	    if (error) {
		// show error dialog
		String errStr =
		    "Illegal value for azimuth: " +
		    _azTextField.getText();
		JOptionPane.showMessageDialog(_azTextField, errStr,
					      "Illegal Double Value",
					      JOptionPane.ERROR_MESSAGE);
		return;
	    }
	    _targetAz = az;
	    AsciiMessage command =
		new AsciiMessage("azimuth", NFormat.az.format(_targetAz));
	    _guiCommands.push(command);
	}

	// enable the GUI components

	public void setEnabled(boolean enabled) {
	    _elTextField.setEnabled(enabled);
	    _azTextField.setEnabled(enabled);
	    _up.setEnabled(enabled);
	    _down.setEnabled(enabled);
	    _left.setEnabled(enabled);
	    _right.setEnabled(enabled);
	    _deltaDegPanel.setEnabled(enabled);
	}

	// DeltaDegPanel - adjustment resolution
	
	private class DeltaDegPanel extends JPanel {
	
	    private JRadioButton _coarse, _medium, _fine;
	    
	    public DeltaDegPanel(JFrame frame) {
		
		setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		setBorder(CustomBorder.createTop(frame, "Delta deg", 3));
		
		_coarse = new JRadioButton("1.0");
		_medium = new JRadioButton("0.1");
		_fine = new JRadioButton("0.01");

		add(_coarse);
		add(_medium);
		add(_fine);

		ButtonGroup group = new ButtonGroup();
		group.add(_coarse);
		group.add(_medium);
		group.add(_fine);
		
		_coarse.setSelected(true);
		_deltaDeg = 1.0;
		
		_coarse.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
			    _deltaDeg = 1.0;
			}
		    });
			
		_medium.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
			    _deltaDeg = 0.1;
			}
		    });
			
		_fine.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
			    _deltaDeg = 0.01;
			}
		    });
		
	    }
	    
	    // enable the GUI components
	    
	    public void setEnabled(boolean enabled) {
		_coarse.setEnabled(enabled);
		_medium.setEnabled(enabled);
		_fine.setEnabled(enabled);
	    }

	}
	
	public void setEntryPosition(double el, double az) {
	    _elTextField.setText(NFormat.el.format(el));
	    _azTextField.setText(NFormat.az.format(az));
	    setElFromTextField();
	    setAzFromTextField();
	}

    } // ManualSetPanel

    // antenna mode panel - inner class

    private class AntennaModePanel extends JPanel {
	
	private JRadioButton _manualButton;
	private JRadioButton _autoVolButton;
	private JRadioButton _autoPpiButton;
	private JRadioButton _followSunButton;
	private JRadioButton _stopButton;

	private ManualListener _manualListener;
	private AutoVolListener _autoVolListener;
	private AutoPpiListener _autoPpiListener;
	private FollowSunListener _followSunListener;
	private StopListener _stopListener;

	public AntennaModePanel(JFrame frame) {
	    
	    setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	    setBorder(CustomBorder.createTop(frame, "Mode", 3));
	    
	    _manualButton = new JRadioButton("Manual");
	    _autoVolButton = new JRadioButton("Auto volume");
	    _autoPpiButton = new JRadioButton("Auto PPI");
	    _followSunButton = new JRadioButton("Follow sun");
	    _stopButton = new JRadioButton("Stop");
	    
	    add(_manualButton);
	    add(_autoVolButton);
	    add(_autoPpiButton);
	    add(_followSunButton);
	    add(_stopButton);
	    
	    ButtonGroup group = new ButtonGroup();
	    group.add(_manualButton);
	    group.add(_autoVolButton);
	    group.add(_autoPpiButton);
	    group.add(_followSunButton);
	    group.add(_stopButton);
	    
	    if (_antennaMode == AntennaMode.MANUAL) {
		_manualButton.setSelected(true);
	    } else if (_antennaMode == AntennaMode.AUTO_VOL) {
		_autoVolButton.setSelected(true);
	    } else if (_antennaMode == AntennaMode.AUTO_PPI) {
		_autoPpiButton.setSelected(true);
	    } else if (_antennaMode == AntennaMode.FOLLOW_SUN) {
		_followSunButton.setSelected(true);
	    } else if (_antennaMode == AntennaMode.STOP) {
		_stopButton.setSelected(true);
	    }

	    _manualListener = new ManualListener();
	    _manualButton.addActionListener(_manualListener);

	    _autoVolListener = new AutoVolListener();
	    _autoVolButton.addActionListener(_autoVolListener);

	    _autoPpiListener = new AutoPpiListener();
	    _autoPpiButton.addActionListener(_autoPpiListener);

	    _followSunListener = new FollowSunListener();
	    _followSunButton.addActionListener(_followSunListener);

	    _stopListener = new StopListener();
	    _stopButton.addActionListener(_stopListener);
	    
	}
	
	// manual action listener
	class ManualListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		_antennaMode = AntennaMode.MANUAL;
		_manualSetPanel.setEnabled(true);
		AsciiMessage command = new AsciiMessage("antenna_mode", "manual");
		_guiCommands.push(command);
		setManualEntryToLastPosition();
	    }
	}
	
	// auto vol action listener
	class AutoVolListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {

		_antennaMode = AntennaMode.AUTO_VOL;
		_manualSetPanel.setEnabled(false);
		
		AsciiMessage elevs =
		    new AsciiMessage("elevation_steps",
				     _params.scan.elevationList.getValue());
		_guiCommands.push(elevs);
		
		String azSlewRateStr = new
		    String(NFormat.f2.format(_params.scan.azSlewRate.getValue()));
		AsciiMessage azSlewRate = new AsciiMessage("az_slew_rate", azSlewRateStr);
		_guiCommands.push(azSlewRate);
		
		String elSlewRateStr = new
		    String(NFormat.f2.format(_params.scan.elSlewRate.getValue()));
		AsciiMessage elSlewRate = new AsciiMessage("el_slew_rate", elSlewRateStr);
		_guiCommands.push(elSlewRate);
		
		String nGatesStr = new
		    String(NFormat.f0.format(_params.scan.nGates.getValue()));
		AsciiMessage nGates = new AsciiMessage("n_gates", nGatesStr);
		_guiCommands.push(nGates);
		
		String rangeStr = new
		    String(NFormat.f3.format(_params.scan.startRange.getValue()));
		AsciiMessage startRange = new AsciiMessage("start_range", rangeStr);
		_guiCommands.push(startRange);
		
		String spacingStr = new
		    String(NFormat.f3.format(_params.scan.gateSpacing.getValue()));
		AsciiMessage gateSpacing =
		    new AsciiMessage("gate_spacing", spacingStr);
		_guiCommands.push(gateSpacing);

		String prfStr = new
		    String(NFormat.f0.format(_params.scan.prf.getValue()));
		AsciiMessage prf = new AsciiMessage("prf", prfStr);
		_guiCommands.push(prf);

		AsciiMessage mode = new AsciiMessage("antenna_mode", "auto_vol");
		_guiCommands.push(mode);

	    }
	}
	
	// auto ppi action listener
	class AutoPpiListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		
		_antennaMode = AntennaMode.AUTO_PPI;
		_manualSetPanel.setEnabled(false);

		String azSlewRateStr = new
		    String(NFormat.f2.format(_params.scan.azSlewRate.getValue()));
		AsciiMessage azSlewRate = new AsciiMessage("az_slew_rate", azSlewRateStr);
		_guiCommands.push(azSlewRate);
		
		AsciiMessage mode = new AsciiMessage("antenna_mode", "auto_ppi");
		_guiCommands.push(mode);
		
	    }
	}
	
	// followSun action listener
	class FollowSunListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		_antennaMode = AntennaMode.FOLLOW_SUN;
		_manualSetPanel.setEnabled(false);
		AsciiMessage command =
		    new AsciiMessage("antenna_mode", "manual");
		_guiCommands.push(command);
	    }
	}
	
	// stop action listener
	class StopListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		_antennaMode = AntennaMode.STOP;
		_manualSetPanel.setEnabled(false);
		AsciiMessage command =
		    new AsciiMessage("antenna_mode", "stop");
		_guiCommands.push(command);
	    }
	}
	
    } // antennaModePanel
    
    // slew rate panel - inner class

    private class SlewRatePanel extends JPanel {

	private JTextField _elRateTextField;
	private JTextField _azRateTextField;
	private double _elRate = 0.0;
	private double _azRate = 0.0;
	
	private AzRateEditListener _azRateEditListener;
	private ElRateEditListener _elRateEditListener;

	public SlewRatePanel(JFrame frame) {
	    
	    setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
	    setBorder(CustomBorder.createTop(frame, "Slew rates", 3));

	    // fonts
	    
	    Font defaultFont = getFont();
	    Font midFont =
		defaultFont.deriveFont((float) (defaultFont.getSize() + 0));
	    Font bigFont =
		defaultFont.deriveFont((float) (defaultFont.getSize() + 2));
	    
	    // elevation rate entry panel
	    
	    JLabel elRateLabel = new JLabel("El rate (deg/s)", JLabel.CENTER);
	    elRateLabel.setFont(midFont);

	    String elRateStr = new
		String(NFormat.f2.format(_params.scan.elSlewRate.getValue()));
	    _elRateTextField = new JTextField(elRateStr, 5);
	    _elRateTextField.setToolTipText("Set el slew rate (deg/s)");
	    _elRateEditListener = new ElRateEditListener();
	    _elRateTextField.addActionListener(_elRateEditListener);
	    
	    BorderLayout elLayout = new BorderLayout();
	    elLayout.setVgap(3);
	    JPanel elRatePanel = new JPanel(elLayout);
	    elRatePanel.add(elRateLabel, BorderLayout.NORTH);
	    elRatePanel.add(_elRateTextField, BorderLayout.SOUTH);
	    
	    // azimuth rate entry panel
	    
	    JLabel azRateLabel = new JLabel("Az rate (deg/s)", JLabel.CENTER);
	    azRateLabel.setFont(midFont);
	    
	    String azRateStr = new
		String(NFormat.f2.format(_params.scan.azSlewRate.getValue()));
	    _azRateTextField = new JTextField(azRateStr, 5);
	    _azRateTextField.setToolTipText("Set az slew rate (deg/s)");
	    _azRateEditListener = new AzRateEditListener();
	    _azRateTextField.addActionListener(_azRateEditListener);
	    
	    BorderLayout azLayout = new BorderLayout();
	    azLayout.setVgap(3);
	    JPanel azRatePanel = new JPanel(azLayout);
	    azRatePanel.add(azRateLabel, BorderLayout.NORTH);
	    azRatePanel.add(_azRateTextField, BorderLayout.SOUTH);
	    
	    // combine entry panels
	    
	    BorderLayout comboLayout = new BorderLayout();
	    comboLayout.setVgap(6);
	    JPanel entryPanel = new JPanel(comboLayout);
	    entryPanel.add(elRatePanel, BorderLayout.NORTH);
	    entryPanel.add(azRatePanel, BorderLayout.SOUTH);
	    JPanel entryContainer = new JPanel();
	    entryContainer.add(entryPanel);
	    
	    // add to main container
	    
	    add(entryContainer, BorderLayout.SOUTH);

	} // constructor
	
	// el edit action
	class ElRateEditListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		setElRateFromTextField();
	    }
	}

	// az edit action
	class AzRateEditListener implements ActionListener {
	    public void actionPerformed(ActionEvent e) {
		setAzRateFromTextField();
	    }
	}
	
	// set the el rate from field
	
	public void setElRateFromTextField() { 
	    double rate = 0.0;
	    boolean error = false;
	    try {
		rate = Double.parseDouble(_elRateTextField.getText());
	    }
	    catch (java.lang.NumberFormatException nfe) {
		error = true;
	    }
	    if (rate < 0 ||
		rate > _params.radar.antMaxElSlewRate.getValue()) {
		error = true;
	    }
	    if (error) {
		// show error dialog
		String errStr =
		    "Illegal value for elevation slew rate: " +
		    _elRateTextField.getText() + "\n" +
		    "Min rate: 0\n" +
		    "Max rate: " + _params.radar.antMaxElSlewRate.getValue() + "\n";
		JOptionPane.showMessageDialog(_elRateTextField, errStr,
					      "Illegal el rate value",
					      JOptionPane.ERROR_MESSAGE);
		return;
	    }
	    _elRate = rate;
	    AsciiMessage command =
		new AsciiMessage("el_slew_rate", NFormat.f2.format(_elRate));
	    _guiCommands.push(command);
	}
	
	// set the az rate from field
	
	public void setAzRateFromTextField() { 
	    double rate = 0.0;
	    boolean error = false;
	    try {
		rate = Double.parseDouble(_azRateTextField.getText());
	    }
	    catch (java.lang.NumberFormatException nfe) {
		error = true;
	    }
	    if (rate < 0 ||
		rate > _params.radar.antMaxAzSlewRate.getValue()) {
		error = true;
	    }
	    if (error) {
		// show error dialog
		String errStr =
		    "Illegal value for azimuth slew rate: " +
		    _azRateTextField.getText() + "\n" +
		    "Min rate: 0\n" +
		    "Max rate: " + _params.radar.antMaxAzSlewRate.getValue() + "\n";
		JOptionPane.showMessageDialog(_azRateTextField, errStr,
					      "Illegal az rate value",
					      JOptionPane.ERROR_MESSAGE);
		return;
	    }
	    _azRate = rate;
	    AsciiMessage command =
		new AsciiMessage("az_slew_rate", NFormat.f2.format(_azRate));
	    _guiCommands.push(command);
	}
	
    } // SlewRatePanel
    
}

