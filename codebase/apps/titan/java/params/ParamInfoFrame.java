///////////////////////////////////////////////////////////////////////
//
// ParamInfoFrame
//
// Info panel for parameters
//
// Singleton
//
// Mike Dixon
//
// Sept 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

class ParamInfoFrame extends JFrame implements ActionListener

{

    private static final ParamInfoFrame INSTANCE = new ParamInfoFrame();
    private static JTextPane _textPane;
    
    // private constructor

    private ParamInfoFrame()
    {

	ParamGuiParams guiParams = ParamGuiParams.getInstance();
	int infoX = guiParams.infoX.getValue();
	int infoY = guiParams.infoY.getValue();
	int infoWidth = guiParams.infoWidth.getValue();
	int infoHeight = guiParams.infoHeight.getValue();
	
	setSize(infoWidth, infoHeight);
	setLocation(infoX, infoY);
	setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
	addComponentListener(new MoveResizeListener());

	JPanel topPanel = new JPanel(new BorderLayout());
	Container cp = getContentPane();
	cp.add(topPanel);
	
	_textPane = new JTextPane();
	cp.add(new JScrollPane(_textPane),  BorderLayout.CENTER);

	_textPane.setContentType("text/html");
	_textPane.setAutoscrolls(true);

	JButton okButton = new JButton("OK");
	okButton.addActionListener(this);
	okButton.setToolTipText("Close window");
	JPanel okPanel = new JPanel();
	okPanel.add(okButton);
	topPanel.add(okPanel, BorderLayout.SOUTH);
	
    }

    // get singleton instance

    public static ParamInfoFrame getInstance() {
	return INSTANCE;
    }

    // show info

    public void show(AbstractParam param) {

	setTitle("Info for param: " + param.getLabel());
	
	String infoStr =
	    "<h2>" + param.getLabel() + "</h2>\n\n" +
	    "<p>"  + param.getDescription() + "\n\n" +
	    "<p>"  + param.getInfo() + "\n\n";

	_textPane.setText(infoStr);
	setVisible(true);

    }
    
    // close when OK is hit
    public void actionPerformed(ActionEvent event)
    {
	if (event.getActionCommand().equals("OK")) {
	    setVisible(false);
	}
    }

    // Inner classes

    private class MoveResizeListener extends ComponentAdapter
    {
	
	public void componentMoved(ComponentEvent e) {
	    ParamGuiParams guiParams = ParamGuiParams.getInstance();
	    int infoX = guiParams.infoX.getValue();
	    int infoY = guiParams.infoY.getValue();
	    if (infoX != getX() || infoY != getY()) {
		guiParams.infoX.setValue(getX());
		guiParams.infoY.setValue(getY());
	    }
	}
	
	public void componentResized(ComponentEvent e) {
	    ParamGuiParams guiParams = ParamGuiParams.getInstance();
	    int infoWidth = guiParams.infoWidth.getValue();
	    int infoHeight = guiParams.infoHeight.getValue();
	    if (infoWidth != getWidth() || infoHeight != getHeight()) {
		guiParams.infoWidth.setValue(getWidth());
		guiParams.infoHeight.setValue(getHeight());
	    }
	}
	
    }

}


