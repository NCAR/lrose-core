///////////////////////////////////////////////////////////////////////
//
// ParamGuiInterface
//
// Interface for standard methods for ParamGui
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

import javax.swing.*;

public interface ParamGuiInterface

{

    public JLabel getJLabel();

    public AbstractParam getParam();

    public void syncParam2Gui();

    public int syncGui2Param(); // returns 0 on success, -1 on failure
    
}
