///////////////////////////////////////////////////////////////////////
//
// ParamGuiParams
//
// Parameters for the parameter GUI itself
//
// Singleton
//
// Mike Dixon
//
// Nov 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public class ParamGuiParams extends CollectionParam
	    
{

    private static final ParamGuiParams INSTANCE = new ParamGuiParams();
    
    public IntegerParam cascadeOffsetX;
    public IntegerParam cascadeOffsetY;
    public IntegerParam infoX;
    public IntegerParam infoY;
    public IntegerParam infoWidth;
    public IntegerParam infoHeight;
    
    // private constructor
    
    private ParamGuiParams()
    {

	super("paramGui", "Param GUI", 2);
	
	// X offset when cascading parameters to next depth
	
	cascadeOffsetX = new IntegerParam("cascadeOffsetX");
	cascadeOffsetX.setLabel("Cascade offset X");
	cascadeOffsetX.setDescription("The X offset when cascading.");
	cascadeOffsetX.setInfo
	    ("Parameters collections may be nested to any depth." +
	     " This is the offset applied to cascade the X offset for the" +
	     " GUI window as the user opens a parameter at the next level" +
	     " down.");
	cascadeOffsetX.setValue(50);

	// Y offset when cascading parameters to next depth
	
	cascadeOffsetY = new IntegerParam("cascadeOffsetY");
	cascadeOffsetY.setLabel("Cascade offset Y");
	cascadeOffsetY.setDescription("The Y offset when cascading.");
	cascadeOffsetY.setInfo
	    ("Parameters collections may be nested to any depth." +
	     " This is the offset applied to cascade the Y offset for the" +
	     " GUI window as the user opens a parameter at the next level" +
	     " down.");
	cascadeOffsetY.setValue(50);

	// X location of Info window
	
	infoX = new IntegerParam("infoX");
	infoX.setLabel("Info window X");
	infoX.setDescription("The X location of the Info window");
	infoX.setValue(800);

	// Y location of Info window
	
	infoY = new IntegerParam("infoY");
	infoY.setLabel("Info window Y");
	infoY.setDescription("The Y location of the Info window");
	infoY.setValue(0);

	// Width location of Info window
	
	infoWidth = new IntegerParam("infoWidth");
	infoWidth.setLabel("Info window width");
	infoWidth.setDescription("The width of the Info window");
	infoWidth.setValue(500);

	// Height location of Info window
	
	infoHeight = new IntegerParam("infoHeight");
	infoHeight.setLabel("Info window height");
	infoHeight.setDescription("The height of the Info window");
	infoHeight.setValue(300);

	// add the params to the list
	
	add(cascadeOffsetX);
	add(cascadeOffsetY);
	add(infoX);
	add(infoY);
	add(infoWidth);
	add(infoHeight);

	// copy the values to the defaults

	setDefaultFromValue();

	// set label and description

	setLabel("Param GUI params");
	setDescription("Parameters for controlling the parameter GUI.");
	setInfo("This sets parameters which control how the GUI for" +
		" editing the parameters is laid out.");

    }

    // get singleton instance

    public static ParamGuiParams getInstance() {
	return INSTANCE;
    }

}


