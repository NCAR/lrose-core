///////////////////////////////////////////////////////////////////////
//
// AScopeParams
//
// Entry panel for control panel parameters
//
// Mike Dixon
//
// Oct 2002
//
////////////////////////////////////////////////////////////////////////

import params.*;

class AScopeParams extends CollectionParam
	    
{

    public IntegerParam xx;
    public IntegerParam yy;
    public IntegerParam width;
    public IntegerParam height;
    public BooleanParam startVisible;
    public IntegerParam topMargin;
    public IntegerParam bottomMargin;
    public IntegerParam leftMargin;
    public IntegerParam rightMargin;
    public IntegerParam maxCount; 
    public DoubleParam minDbz;
    public DoubleParam maxDbz;
    public BooleanParam plotCount;
    public BooleanParam plotDbz;
    public BooleanParam plotSample;
    public IntegerParam sampleCenter;
    public IntegerParam sampleNGates;

    // constructor
    
    public AScopeParams(String name, String label, int depth)
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
	yy.setValue(510);

	// window width in pixels
	
	width = new IntegerParam("width");
	width.setLabel("Width");
	width.setDescription("The window width in pixels");
	width.setValue(1149);

	// window height in pixels
	
	height = new IntegerParam("height");
	height.setLabel("Height");
	height.setDescription("The window height in pixels");
	height.setValue(443);

	// start with window visible

	startVisible = new BooleanParam("startVisible");
	startVisible.setLabel("Start visible");
	startVisible.setDescription("Start with the window visible?");
	startVisible.setInfo("If true, the program will start with the " +
			     "A-Scope visible. If not, it will " +
			     "be hidden at startup");
	startVisible.setValue(false);

	// top margin
	
	topMargin = new IntegerParam("topMargin");
	topMargin.setLabel("Top margin");
	topMargin.setDescription("Top margin (pixels)");
	topMargin.setValue(30);

	// bottom margin
	
	bottomMargin = new IntegerParam("bottomMargin");
	bottomMargin.setLabel("Bottom margin");
	bottomMargin.setDescription("Bottom margin (pixels)");
	bottomMargin.setValue(30);

	// left margin
	
	leftMargin = new IntegerParam("leftMargin");
	leftMargin.setLabel("Left margin");
	leftMargin.setDescription("Left margin (pixels)");
	leftMargin.setValue(45);

	// right margin
	
	rightMargin = new IntegerParam("rightMargin");
	rightMargin.setLabel("Right margin");
	rightMargin.setDescription("Right margin (pixels)");
	rightMargin.setValue(50);

	// max Count
	
	maxCount = new IntegerParam("maxCount");
	maxCount.setLabel("Max count");
	maxCount.setDescription("Max count to be displayed - Y axis");
	maxCount.setValue(4096);

	// min Dbz
	
	minDbz = new DoubleParam("minDbz");
	minDbz.setLabel("Min dbz");
	minDbz.setDescription("Min dbz to be displayed - Y axis");
	minDbz.setValue(-20);

	// max Dbz
	
	maxDbz = new DoubleParam("maxDbz");
	maxDbz.setLabel("Max dbz");
	maxDbz.setDescription("Max dbz to be displayed - Y axis");
	maxDbz.setValue(80);

	// plot count
	
	plotCount = new BooleanParam("plotCount");
	plotCount.setLabel("Plot count?");
	plotCount.setDescription("Option to plot count values");
	plotCount.setValue(true);

	// plot dbz
	
	plotDbz = new BooleanParam("plotDbz");
	plotDbz.setLabel("Plot dbz?");
	plotDbz.setDescription("Option to plot dbz values");
	plotDbz.setValue(true);

	// plot sample
	
	plotSample = new BooleanParam("plotSample");
	plotSample.setLabel("Plot sampled values?");
	plotSample.setDescription("Option to plot sampled values.");
	plotSample.setInfo("You have the option to sample the values" +
			   " over a selected number of gates.");
	plotSample.setValue(false);

	// center of sampled location
	
	sampleCenter = new IntegerParam("sampleCenter");
	sampleCenter.setLabel("Sample center");
	sampleCenter.setDescription
	    ("Location of the center of the sampled region (gates).");
	sampleCenter.setValue(320);

	// number of gates in sample
	
	sampleNGates = new IntegerParam("sampleNGates");
	sampleNGates.setLabel("Sample N Gates");
	sampleNGates.setDescription("Number of gates in sample.");
	sampleNGates.setInfo("This is the width of the sample in gates");
	sampleNGates.setValue(11);

	// add the params to the list
	
	add(xx);
	add(yy);
	add(width);
	add(height);
	add(startVisible);
	add(topMargin);
	add(bottomMargin);
	add(leftMargin);
	add(rightMargin);
	add(maxCount);
	add(minDbz);
	add(maxDbz);
	add(plotCount);
	add(plotDbz);
	add(plotSample);
	add(sampleCenter);
	add(sampleNGates);

	// copy the values to the defaults

	setDefaultFromValue();

    }

}


