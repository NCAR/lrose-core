///////////////////////////////////////////////////////////////////////
//
// ControlPanelParams
//
// Entry panel for control panel parameters
//
// Mike Dixon
//
// Oct 2002
//
////////////////////////////////////////////////////////////////////////

import params.*;

class ControlPanelParams extends CollectionParam
	    
{

    public IntegerParam xx;
    public IntegerParam yy;
    public IntegerParam width;
    public IntegerParam height;
    public BooleanParam startVisible;
    
    // constructor
    
    public ControlPanelParams(String name, String label, int depth)
    {
	
	super(name, label, depth);
	
	// initialize the parameter objects
    
	// x offset from parent
	
	xx = new IntegerParam("xx");
	xx.setLabel("X offset");
	xx.setDescription("The X offset from the parent");
	xx.setInfo("This the the X offset, in pixels, from the" +
			" main frame of the application.");
	xx.setValue(600);

	// y offset from parent
	
	yy = new IntegerParam("yy");
	yy.setLabel("Y offset");
	yy.setDescription("The Y offset from the parent");
	yy.setInfo("This the the Y offset, in pixels, from the" +
			" main frame of the application.");
	yy.setValue(0);

	// window width in pixels
	
	width = new IntegerParam("width");
	width.setLabel("Width");
	width.setDescription("The window width in pixels");
	width.setValue(550);

	// window height in pixels
	
	height = new IntegerParam("height");
	height.setLabel("Height");
	height.setDescription("The window height in pixels");
	height.setValue(507);

	// start with window visible

	startVisible = new BooleanParam("startVisible");
	startVisible.setLabel("Start visible");
	startVisible.setDescription("Start with the window visible?");
	startVisible.setInfo("If true, the program will start with the " +
			     "Control Panel visible. If not, it will " +
			     "be hidden at startup");
	startVisible.setValue(false);

	// add the params to the list
	
	add(xx);
	add(yy);
	add(width);
	add(height);
	add(startVisible);

	// copy the values to the defaults

	setDefaultFromValue();

    }

}
