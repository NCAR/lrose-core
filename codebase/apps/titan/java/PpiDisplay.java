///////////////////////////////////////////////////////////////////////
//
// PpiDisplay
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
import javax.swing.*;
import java.util.*;

import params.*;

public class PpiDisplay extends JFrame

{

    private Parameters _params;
    
    public PpiDisplay(Parameters params) {
	
	super();
	_params = params;
	
	setTitle("PPI Display");
	setSize(_params.ppiDisplay.width.getValue(),
		_params.ppiDisplay.height.getValue());
	setLocation(_params.ppiDisplay.xx.getValue(),
		    _params.ppiDisplay.yy.getValue());
	setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);

	addComponentListener(new MoveResizeListener(_params));
	
    }

    ////////////////////////////////////////////////////
    // Inner classes for listeners
    ////////////////////////////////////////////////////

    private class MoveResizeListener extends ComponentAdapter
    {
	private Parameters _params;
	public MoveResizeListener(Parameters params) {
	    super();
	    _params = params;
	}
	
	public void componentMoved(ComponentEvent e) {
	    int paramX = _params.ppiDisplay.xx.getValue();
	    int paramY = _params.ppiDisplay.yy.getValue();
	    if (paramX != getX() || paramY != getY()) {
		_params.ppiDisplay.xx.setValue(getX());
		_params.ppiDisplay.yy.setValue(getY());
	    }
	}
	
	public void componentResized(ComponentEvent e) {
	    int paramWidth = _params.ppiDisplay.width.getValue();
	    int paramHeight = _params.ppiDisplay.height.getValue();
	    if (paramWidth != getWidth() || paramHeight != getHeight()) {
		_params.ppiDisplay.width.setValue(getWidth());
		_params.ppiDisplay.height.setValue(getHeight());
	    }
	}
	
    }

}


