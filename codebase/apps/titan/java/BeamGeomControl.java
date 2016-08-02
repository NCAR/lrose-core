///////////////////////////////////////////////////////////////////////
//
// BeamGeomControl panel
//
// Mike Dixon
//
// Jan 2003
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

public class BeamGeomControl extends JPanel {
    
    private Parameters _params;
    private MessageQueue _guiCommands;

    private JTextField _nGatesTextField;
    private JTextField _startRangeTextField;
    private JTextField _gateSpacingTextField;
    private JTextField _prfTextField;
    private int _nGates;
    private double _startRange;
    private double _gateSpacing;
    private double _prf;
    
    private NGatesEditListener _nGatesEditListener;
    private StartRangeEditListener _startRangeEditListener;
    private GateSpacingEditListener _gateSpacingEditListener;
    private PrfEditListener _prfEditListener;
	
    public BeamGeomControl(JFrame frame,
			   Parameters params,
			   MessageQueue guiCommands) {
	
	_params = params;
	_guiCommands = guiCommands;
	
	setLayout(new BorderLayout());
	setBorder(CustomBorder.createTop
		  (frame, "Beam geometry", 3));
	
	_nGates = _params.scan.nGates.getValue();
	_startRange = _params.scan.startRange.getValue();
	_gateSpacing = _params.scan.gateSpacing.getValue();
	_prf = _params.scan.prf.getValue();
    
	// fonts
	
	Font defaultFont = getFont();
	Font midFont =
	    defaultFont.deriveFont((float) (defaultFont.getSize() + 0));
	Font bigFont =
	    defaultFont.deriveFont((float) (defaultFont.getSize() + 2));
	
	// nGates entry panel
	
	JLabel nGatesLabel = new JLabel("N Gates: ", JLabel.RIGHT);
	nGatesLabel.setFont(midFont);
	
	String nGatesStr = new
	    String(NFormat.f0.format(_params.scan.nGates.getValue()));
	_nGatesTextField = new JTextField(nGatesStr, 5);
	_nGatesTextField.setToolTipText("Set number of gates");
	_nGatesEditListener = new NGatesEditListener();
	_nGatesTextField.addActionListener(_nGatesEditListener);
	
	BorderLayout nGatesLayout = new BorderLayout();
	nGatesLayout.setVgap(3);
	JPanel nGatesPanel = new JPanel(nGatesLayout);
	nGatesPanel.add(nGatesLabel, BorderLayout.NORTH);
	nGatesPanel.add(_nGatesTextField, BorderLayout.SOUTH);
	
	// startRange entry panel
	
	JLabel startRangeLabel = new JLabel("Start range: ", JLabel.RIGHT);
	startRangeLabel.setFont(midFont);
	
	String startRangeStr = new
	    String(NFormat.f3.format(_params.scan.startRange.getValue()));
	_startRangeTextField = new JTextField(startRangeStr, 5);
	_startRangeTextField.setToolTipText("Set start range (km)");
	_startRangeEditListener = new StartRangeEditListener();
	_startRangeTextField.addActionListener(_startRangeEditListener);
	
	BorderLayout startRangeLayout = new BorderLayout();
	startRangeLayout.setVgap(3);
	JPanel startRangePanel = new JPanel(startRangeLayout);
	startRangePanel.add(startRangeLabel, BorderLayout.NORTH);
	startRangePanel.add(_startRangeTextField, BorderLayout.SOUTH);
	
	// gateSpacing entry panel
	
	JLabel gateSpacingLabel =
	    new JLabel("Gate spacing: ", JLabel.RIGHT);
	gateSpacingLabel.setFont(midFont);
	
	String gateSpacingStr = new
	    String(NFormat.f3.format(_params.scan.gateSpacing.getValue()));
	_gateSpacingTextField = new JTextField(gateSpacingStr, 5);
	_gateSpacingTextField.setToolTipText("Set gate spacing (km)");
	_gateSpacingEditListener = new GateSpacingEditListener();
	_gateSpacingTextField.addActionListener(_gateSpacingEditListener);
	
	BorderLayout gateSpacingLayout = new BorderLayout();
	gateSpacingLayout.setVgap(3);
	JPanel gateSpacingPanel = new JPanel(gateSpacingLayout);
	gateSpacingPanel.add(gateSpacingLabel, BorderLayout.NORTH);
	gateSpacingPanel.add(_gateSpacingTextField, BorderLayout.SOUTH);
	
	// prf entry panel
	
	JLabel prfLabel = new JLabel("PRF: ", JLabel.RIGHT);
	prfLabel.setFont(midFont);
	
	String prfStr = new
	    String(NFormat.f0.format(_params.scan.prf.getValue()));
	_prfTextField = new JTextField(prfStr, 5);
	_prfTextField.setToolTipText("Set PRF (/s)");
	_prfEditListener = new PrfEditListener();
	_prfTextField.addActionListener(_prfEditListener);
	
	BorderLayout prfLayout = new BorderLayout();
	prfLayout.setVgap(3);
	JPanel prfPanel = new JPanel(prfLayout);
	prfPanel.add(prfLabel, BorderLayout.NORTH);
	prfPanel.add(_prfTextField, BorderLayout.SOUTH);
	
	// combine entry panels
	
	GridLayout comboLayout = new GridLayout(4, 2, 3, 3);
	JPanel entryPanel = new JPanel(comboLayout);
	
	entryPanel.add(nGatesLabel);
	entryPanel.add(_nGatesTextField);
	entryPanel.add(startRangeLabel);
	entryPanel.add(_startRangeTextField);
	entryPanel.add(gateSpacingLabel);
	entryPanel.add(_gateSpacingTextField);
	entryPanel.add(prfLabel);
	entryPanel.add(_prfTextField);
	
	JPanel entryContainer = new JPanel();
	entryContainer.add(entryPanel);
	
	// add to main container
	
	add(entryContainer, BorderLayout.SOUTH);
	
	// set initial values;

	setNGatesFromTextField();
	setStartRangeFromTextField();
	setGateSpacingFromTextField();
	setPrfFromTextField();

    } // constructor
    
    // NGates edit listener
    private class NGatesEditListener implements ActionListener {
	public void actionPerformed(ActionEvent e) {
	    setNGatesFromTextField();
	}
    }
    
    // StartRange edit listener
    private class StartRangeEditListener implements ActionListener {
	public void actionPerformed(ActionEvent e) {
	    setStartRangeFromTextField();
	}
    }
    
    // GateSpacing edit listener
    private class GateSpacingEditListener implements ActionListener {
	public void actionPerformed(ActionEvent e) {
	    setGateSpacingFromTextField();
	}
    }
    
    // Prf edit listener
    private class PrfEditListener implements ActionListener {
	public void actionPerformed(ActionEvent e) {
	    setPrfFromTextField();
	}
    }
    
    // set the NGates from text field
    
    private void setNGatesFromTextField() { 
	int nGates = 0;
	boolean error = false;
	try {
	    nGates = Integer.parseInt(_nGatesTextField.getText());
	}
	catch (java.lang.NumberFormatException nfe) {
	    error = true;
	}
	if (nGates < 0) {
	    error = true;
	}
	if (error) {
	    // show error dialog
	    String errStr =
		"Illegal value for nGates: " +
		_nGatesTextField.getText() + "\n";
	    JOptionPane.showMessageDialog(_nGatesTextField, errStr,
					  "Illegal nGates value",
					  JOptionPane.ERROR_MESSAGE);
	    return;
	}
	_nGates = nGates;
	AsciiMessage command =
	    new AsciiMessage("n_gates", NFormat.f0.format(_nGates));
	_guiCommands.push(command);
    }
    
    // set the startRange from text field
    
    private void setStartRangeFromTextField() { 
	double startRange = 0.0;
	boolean error = false;
	try {
	    startRange = Double.parseDouble(_startRangeTextField.getText());
	}
	catch (java.lang.NumberFormatException nfe) {
	    error = true;
	}
	if (startRange < 0) {
	    error = true;
	}
	if (error) {
	    // show error dialog
	    String errStr =
		"Illegal value for startRange: " +
		_startRangeTextField.getText() + "\n";
	    JOptionPane.showMessageDialog(_startRangeTextField, errStr,
					  "Illegal startRange value",
					  JOptionPane.ERROR_MESSAGE);
	    return;
	}
	_startRange = startRange;
	AsciiMessage command =
	    new AsciiMessage("start_range", NFormat.f3.format(_startRange));
	_guiCommands.push(command);
    }
    
    // set the gateSpacing from text field
    
    private void setGateSpacingFromTextField() { 
	double gateSpacing = 0.0;
	boolean error = false;
	try {
	    gateSpacing = Double.parseDouble(_gateSpacingTextField.getText());
	}
	catch (java.lang.NumberFormatException nfe) {
	    error = true;
	}
	if (gateSpacing < 0) {
	    error = true;
	}
	if (error) {
	    // show error dialog
	    String errStr =
		"Illegal value for gateSpacing: " +
		_gateSpacingTextField.getText() + "\n";
	    JOptionPane.showMessageDialog(_gateSpacingTextField, errStr,
					  "Illegal gateSpacing value",
					  JOptionPane.ERROR_MESSAGE);
	    return;
	}
	_gateSpacing = gateSpacing;
	AsciiMessage command =
	    new AsciiMessage("gate_spacing", NFormat.f3.format(_gateSpacing));
	_guiCommands.push(command);
    }
    
    // set the prf from text field
    
    private void setPrfFromTextField() { 
	double prf = 0.0;
	boolean error = false;
	try {
	    prf = Double.parseDouble(_prfTextField.getText());
	}
	catch (java.lang.NumberFormatException nfe) {
	    error = true;
	}
	if (prf <= 0) {
	    error = true;
	}
	if (error) {
	    // show error dialog
	    String errStr =
		"Illegal value for prf: " +
		_prfTextField.getText() + "\n";
	    JOptionPane.showMessageDialog(_prfTextField, errStr,
					  "Illegal prf value",
					  JOptionPane.ERROR_MESSAGE);
	    return;
	}
	_prf = prf;
	AsciiMessage command =
	    new AsciiMessage("prf", NFormat.f3.format(_prf));
	_guiCommands.push(command);
    }
    
} // Beam Management Panel
    
