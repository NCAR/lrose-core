///////////////////////////////////////////////////////////////////////
//
// FrameWithParams
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

public class FrameWithParams extends JFrame implements ActionListener

{

    private ParamManager _params;
    
    public FrameWithParams(ParamManager params) {
	
	super();
	_params = params;

    }

    public void actionPerformed(ActionEvent event)
    {
    }

}
