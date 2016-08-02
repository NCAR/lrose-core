///////////////////////////////////////////////////////////////////////
//
// CollectionFrame
//
// Panel container for Collection GUI
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import java.util.*;

public class CollectionFrame extends JFrame implements ActionListener

{

    private CollectionParam _param;
    private String _name;         // name of frame variable
    private String _label;        // label for GUI
    private int _depth;
    private JPanel _topPanel;
    private JPanel _entryPanel;
    private GridBagLayout _gridbag;
    private GridBagConstraints _gbc;
    private ArrayList _paramGuiList;

    public CollectionFrame(CollectionParam param,
			   String name, String label, int depth) {
	
	_param = param;
	_label = new String(label);
	_name = new String(name);
	_depth = depth;
	
	// make a new JFrame
	
	setTitle(_label);
	setLocation(50 * _depth, 50 * depth);
	setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
	addComponentListener(new MoveListener());

	_paramGuiList = new ArrayList();

    }

    public void clear() {
	_paramGuiList.clear();
    }

    public void add(AbstractParamGui guiElement) {
	_paramGuiList.add(guiElement);
    }

    public void actionPerformed(ActionEvent event)
    {
	if (event.getActionCommand().equals("Apply")) {
	    int iret = 0;
	    for (Iterator ii = _paramGuiList.iterator(); ii.hasNext();) {
		AbstractParamGui guiElement = (AbstractParamGui) ii.next();
		if (guiElement.syncGui2Param() != 0) {
		    iret = -1;
		}
	    }
	    if (iret == 0) {
		setVisible(false);
		_param.applyChanges();
	    }
	} else if (event.getActionCommand().equals("Reset")) {
	    for (Iterator ii = _paramGuiList.iterator(); ii.hasNext();) {
		AbstractParamGui guiElement = (AbstractParamGui) ii.next();
		guiElement.syncParam2Gui();
		_param.applyChanges();
	    }
	} else if (event.getActionCommand().equals("Cancel")) {
	    for (Iterator ii = _paramGuiList.iterator(); ii.hasNext();) {
		AbstractParamGui guiElement = (AbstractParamGui) ii.next();
		guiElement.syncParam2Gui();
		_param.applyChanges();
	    }
	    setVisible(false);
	}
    }

    public void setVisible(boolean b) {
	ParamGuiParams guiParams = ParamGuiParams.getInstance();
	int offsetX = guiParams.cascadeOffsetX.getValue() * _depth;
	int offsetY = guiParams.cascadeOffsetY.getValue() * _depth;
	setLocation(offsetX, offsetY);
	super.setVisible(b);
    }

    public void activate() {

	ParamGuiParams guiParams = ParamGuiParams.getInstance();
	int offsetX = guiParams.cascadeOffsetX.getValue() * _depth;
	int offsetY = guiParams.cascadeOffsetY.getValue() * _depth;
	setLocation(offsetX, offsetY);

	_topPanel = new JPanel(new BorderLayout());
	getContentPane().add(_topPanel);
	
        _entryPanel = new JPanel();
	_entryPanel.setAutoscrolls(true);
        _gridbag = new GridBagLayout();
        _gbc = new GridBagConstraints();
        _entryPanel.setLayout(_gridbag);

	for (Iterator ii = _paramGuiList.iterator(); ii.hasNext();) {
	    AbstractParamGui guiElement = (AbstractParamGui) ii.next();
	    guiElement.addTo3ColLayout(_gridbag, _entryPanel);
	}
	
  	JScrollPane scrollPane = new JScrollPane(_entryPanel);
    	_topPanel.add(scrollPane, BorderLayout.CENTER);
	
	//JLabel headerLabel =
	//    new JLabel("Cancel will cancel and close window\n" +
	//	       "Reset will set values back to previous values.");
	//_topPanel.add(headerLabel, BorderLayout.NORTH);
	
	JButton applyButton = new JButton("Apply");
	applyButton.addActionListener(this);
	applyButton.setToolTipText("Accept and close");
	
	JButton resetButton = new JButton("Reset");
	resetButton.addActionListener(this);
	resetButton.setToolTipText("Reset all changes");
	
	JButton cancelButton = new JButton("Cancel");
	cancelButton.addActionListener(this);
	cancelButton.setToolTipText("Cancel all changes and close");

	JPanel applyPanel = new JPanel();
	applyPanel.add(applyButton);
	applyPanel.add(resetButton);
	applyPanel.add(cancelButton);

	_topPanel.add(applyPanel, BorderLayout.SOUTH);

  	Border basicBorder = BorderFactory.createCompoundBorder
  	    (BorderFactory.createEmptyBorder(5,5,5,5),
  	     BorderFactory.createEtchedBorder(EtchedBorder.LOWERED,
  					      getBackground().brighter(),
  					      getBackground().darker()));
        _topPanel.setBorder
	    (BorderFactory.createTitledBorder
	     (basicBorder, _label + " - edit parameters, then Apply"));
	
	pack();
	
	Dimension size = getSize();
	if (size.height > 550) {
	    size.height = 550;
	}
	if (size.width > 750) {
	    size.width = 750;
	}
	setSize(size);
	
    }

    // get name and label

    public String getName() {
	return _name;
    }

    public String getLabel() {
	return _label;
    }

    // Inner classes

    private class MoveListener extends ComponentAdapter
    {
	
	public void componentMoved(ComponentEvent e) {
	    // if (_depth != 1) {
	    //  return;
	    // }
	    // ParamGuiParams guiParams = ParamGuiParams.getInstance();
	    // int cascadeOffsetX = guiParams.cascadeOffsetX.getValue();
	    // int cascadeOffsetY = guiParams.cascadeOffsetY.getValue();
	    // int xx = cascadeOffsetX * _depth;
	    // int yy = cascadeOffsetY * _depth;
	    // if (xx != getX() || yy != getY()) {
	    //   guiParams.cascadeOffsetX.setValue(getX() / _depth);
	    //   guiParams.cascadeOffsetY.setValue(getY() / _depth);
	    // }
 	}
	
    }

}
