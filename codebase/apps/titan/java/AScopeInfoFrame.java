///////////////////////////////////////////////////////////////////////
//
// AScopeInfoFrame
//
// Info panel for AScope
//
// Mike Dixon
//
// Jan 2003
//
////////////////////////////////////////////////////////////////////////

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

class AScopeInfoFrame extends JFrame implements ActionListener

{

    public AScopeInfoFrame(int xx, int yy, int width, int height)
    {
	
	setSize(width, height);
	setLocation(xx, yy);
	setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
	
	JPanel topPanel = new JPanel(new BorderLayout());
	Container cp = getContentPane();
	cp.add(topPanel);
	
	JTextPane textPane = new JTextPane();
	cp.add(new JScrollPane(textPane),  BorderLayout.CENTER);

	textPane.setContentType("text/html");
	textPane.setAutoscrolls(true);

	setTitle("Info for AScope");
	
	String infoStr =
	    "<h2>AScope window help info</h2>\n\n" +
	    "<p>Toggle count or dBZ plotting using buttons at lower left.\n" +
	    "<p>Drag with mouse to zoom.\n" +
	    "<p>Hit UnZoom button to reset to full view.\n" +
	    "<p>To sample data in a selected range interval, \n" +
	    "toggle the Sample gates button.\n" +
	    "<p>Adjust the sample position with the slider.\n" +
	    "<p>Adjust the number of gates in the sample.\n";
	
	textPane.setText(infoStr);
	JButton okButton = new JButton("OK");
	okButton.addActionListener(this);
	okButton.setToolTipText("Close window");
	JPanel okPanel = new JPanel();
	okPanel.add(okButton);
	topPanel.add(okPanel, BorderLayout.SOUTH);
	
	setVisible(true);

    }

    // close when OK is hit
    public void actionPerformed(ActionEvent event)
    {
	if (event.getActionCommand().equals("OK")) {
	    setVisible(false);
	}
    }

}


