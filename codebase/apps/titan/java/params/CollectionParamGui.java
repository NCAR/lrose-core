///////////////////////////////////////////////////////////////////////
//
// CollectionParamGui
//
// GUI class for CollectionParam
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import javax.swing.*;
import java.awt.event.*;

public class CollectionParamGui extends AbstractParamGui

{

    private JButton _editButton;
    
    public CollectionParamGui(CollectionParam param) {
	super(param);
	ImageIcon editIcon =
	    new ImageIcon(this.getClass().getResource("./edit.png"));
	_editButton = new JButton("Edit", editIcon);
	_editButton.addActionListener(new ActionListener() {
		public void actionPerformed(ActionEvent event)
		{
		    CollectionParam cp = (CollectionParam) _param;
		    CollectionFrame frame = cp.getCollectionFrame();
		    frame.setVisible(true);
		}
	    });
	_selector = _editButton;
	_selector.setToolTipText(param.getDescription());
    }

    // interface methods - no-op for this class

    public void syncParam2Gui() {
    }
    
    public int syncGui2Param() {
	return 0;
    }

}

