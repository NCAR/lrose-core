///////////////////////////////////////////////////////////////////////
//
// MainWindowParams
//
// Mike Dixon
//
// Jan 2003
//
////////////////////////////////////////////////////////////////////////

import params.*;

class MainWindowParams extends CollectionParam
	    
{

    public IntegerParam xx;
    public IntegerParam yy;
    public IntegerParam width;
    public IntegerParam height;
    public StringParam imageName;

    // constructor
    
    public MainWindowParams(String name, String label, int depth)
    {
	
	super(name, label, depth);
	
	// initialize the parameter objects
    
	// x offset from parent
	
	xx = new IntegerParam("xx");
	xx.setLabel("X offset");
	xx.setDescription("The X offset from the parent");
	xx.setInfo("This the the X offset, in pixels, from the" +
			" main frame of the application.");
	xx.setValue(0);

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
	width.setValue(600);

	// window height in pixels
	
	height = new IntegerParam("height");
	height.setLabel("Height");
	height.setDescription("The window height in pixels");
	height.setValue(506);

	// name of file for image in main window

	imageName = new StringParam("imageName");
	imageName.setLabel("Image name");
	imageName.setDescription("Name of image in main window");
	imageName.setValue("./main_window.png");
	
	// add the params
	
	add(xx);
	add(yy);
	add(width);
	add(height);
	add(imageName);
	
	// copy the values to the defaults
	
	setDefaultFromValue();

    }

}


