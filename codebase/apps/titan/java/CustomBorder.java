///////////////////////////////////////////////////////////////////////
//
// CustomBorder
//
// Static factory methods for returning custom borders
//
// Mike Dixon
//
// Nov 2002
//
////////////////////////////////////////////////////////////////////////

import javax.swing.*;
import javax.swing.border.*;

public class CustomBorder

{

    public static Border createAboveTop(JFrame frame, String titleLabel,
					int spacing) {
	
  	Border basicBorder = BorderFactory.createCompoundBorder
  	    (BorderFactory.createEtchedBorder(EtchedBorder.LOWERED,
  					      frame.getBackground().brighter(),
  					      frame.getBackground().darker()),
	     BorderFactory.createEmptyBorder(spacing,spacing,spacing,spacing));

        return BorderFactory.createTitledBorder
	    (basicBorder, titleLabel,
	     TitledBorder.CENTER, TitledBorder.ABOVE_TOP);


    }

    public static Border createTop(JFrame frame, String titleLabel,
				   int spacing) {

	Border etched =
	    BorderFactory.createEtchedBorder(EtchedBorder.LOWERED,
					     frame.getBackground().brighter(),
					     frame.getBackground().darker());

	Border titled = BorderFactory.createTitledBorder
	    (etched, titleLabel,
	     TitledBorder.CENTER, TitledBorder.TOP);

  	return BorderFactory.createCompoundBorder
  	    (titled,
	     BorderFactory.createEmptyBorder(0,spacing,spacing,spacing));
	
    }

}
