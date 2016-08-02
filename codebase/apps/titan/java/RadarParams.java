///////////////////////////////////////////////////////////////////////
//
// RadarParams
//
// Entry panel for radar parameters
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

import params.*;

class RadarParams extends CollectionParam
	    
{

    public IntegerParam siteNum;
    public StringParam siteName;
    public StringParam userName;
    public DoubleParam latitude;
    public DoubleParam longitude;
    public DoubleParam altitude;
    public DoubleParam peakXmitPower;
    public DoubleParam xmitFreq;
    public DoubleParam beamWidth;
    public DoubleParam antGain;
    public DoubleParam waveguideLoss;
    public DoubleParam radomeLoss;
    public DoubleParam testCableAtten;
    public DoubleParam couplerAtten;
    public OptionParam displaceType;
    public BooleanParam clutFilter;
    public DoubleParam antMinElev;
    public DoubleParam antMaxElev;
    public DoubleParam antMaxElSlewRate;
    public DoubleParam antMaxAzSlewRate;
    
    // constructor

    public RadarParams(String name, String label, int depth)
    {

	super(name, label, depth);

	// initialize the parameter objects
    
	// site number
	
	siteNum = new IntegerParam("siteNum");
	siteNum.setLabel("Site number");
	siteNum.setDescription("The indexed number for this site");
	siteNum.setInfo("You should arrange these numbers with reference" +
			" to other sites in the area. The number is used" +
			" identify the radar site if mode than one radar" +
			" is used in the network");
	siteNum.setValue(0);

	// site name
	
	siteName = new StringParam("siteName");
	siteName.setLabel("Site name");
	siteName.setDescription("The name of the site");
	siteName.setInfo("The site name must be a text description" +
			 " of the geographical geographical location of" +
			 " the radar installation, such as a farm name" +
			 " or nearby town.");
	siteName.setValue("Bethlehem");
	
	// user name
	
	userName = new StringParam("userName");
	userName.setLabel("User name");
	userName.setDescription("The name of the technician or user");
	userName.setInfo
	    ("The technician or user of the program can" +
	     " put their name in this field. The reason for" +
	     " having it is to keep track of who performed" +
	     " the last calibration.");
	userName.setValue("Unknown");
	
	// latitude
	
	latitude = new DoubleParam("latitude");
	latitude.setLabel("Latitude");
	latitude.setDescription("Radar latitude (decimal deg)");
	latitude.setValue(40.0);
	
	// longitude
	
	longitude = new DoubleParam("longitude");
	longitude.setLabel("Longitude");
	longitude.setDescription("Radar longitude (decimal deg)");
	longitude.setValue(-105.0);
	
	// altitude
	
	altitude = new DoubleParam("altitude");
	altitude.setLabel("Altitude");
	altitude.setDescription("Radar altitude (km)");
	altitude.setValue(1.7);
	
	// peak transmitter power
	
	peakXmitPower = new DoubleParam("peakXmitPower");
	peakXmitPower.setLabel("Peak power");
	peakXmitPower.setDescription("Peak transmitter power (dBM)");
	peakXmitPower.setValue(80);
	
	// transmitter freq
	
	xmitFreq = new DoubleParam("xmitFreq");
	xmitFreq.setLabel("Xmit Frequency");
	xmitFreq.setDescription("Transmitter Frequency (GHz)");
	xmitFreq.setInfo ("Transmitter frequency in GigaHertz");
	xmitFreq.setValue(5.66);
	
	// beam width
	
	beamWidth = new DoubleParam("beamWidth");
	beamWidth.setLabel("Beam width");
	beamWidth.setDescription("Antenna beam width (deg)");
	beamWidth.setInfo("Half-power beam width in degrees");
	beamWidth.setValue(1.0);
	
	// antenna gain
	
	antGain = new DoubleParam("antGain");
	antGain.setLabel("Antenna gain");
	antGain.setDescription("Antenna gain (dB)");
	antGain.setInfo("Antenna gain in dB");
	antGain.setValue(44.0);
	
	// one way waveguide loss
	
	waveguideLoss = new DoubleParam("waveguideLoss");
	waveguideLoss.setLabel("Waveguide loss");
	waveguideLoss.setDescription("One-way waveguide loss (dB)");
	waveguideLoss.setInfo("Typical loss values:\n" +
			      "<p>  C-band: 1.75 dB per 100 ft / 30 m\n" +
			      "<p>  S-band: 0.75 dB per 100 ft / 30 m\n");
	waveguideLoss.setValue(0.5);
	
	// two-way radome loss
	
	radomeLoss = new DoubleParam("radomeLoss");
	radomeLoss.setLabel("Radome loss");
	radomeLoss.setDescription("Two-way radome loss (dB)");
	radomeLoss.setInfo("If no radome set to 0.");
	radomeLoss.setValue(0.5);
	
	// test cable attenuation
	
	testCableAtten = new DoubleParam("testCableAtten");
	testCableAtten.setLabel("Test cable attenuation");
	testCableAtten.setDescription("Test cable attenuation (dB)");
	testCableAtten.setInfo("Antenna gain in dB");
	testCableAtten.setInfo("Typical values:\n" +
			       "<p>C-band: 0.6 dB/ft with RG58 or similar\n" +
			       "<p>S-band: 0.4 dB/ft with RG58 or similar\n");
	testCableAtten.setValue(3.8);
	
	// coupler attenuation
	
	couplerAtten = new DoubleParam("couplerAtten");
	couplerAtten.setLabel("Coupler attenuation");
	couplerAtten.setDescription("Coupler attenuation (dB)");
	couplerAtten.setInfo("If the signal generator input is connected " +
			     "directly to the receiver set this to 0");
	couplerAtten.setValue(30.75);
	
	// displace lookup table type
	
	displaceType = new OptionParam("displaceType");
	displaceType.setOptions(new String[] {"Log", "Linear", "Quadratic"});
	displaceType.setLabel("Displace type");
	displaceType.setDescription("Displace table type");
	displaceType.setInfo
	    ("Quadratic should be used unless the user has a specific" +
	     " reason for choosing a different type. Further information" +
	     " on the table types may be found in the RDAS documentation");
	displaceType.setValue("Quadratic");

	// clutter filter
	
	clutFilter = new BooleanParam("clutFilter");
	clutFilter.setLabel("Clutter filter on");
	clutFilter.setDescription("Is the clutter filter on or off?");
	clutFilter.setInfo
	    ("Select true to turn clutter filtering on," +
	     " false to turn it off");
	clutFilter.setValue(false);
	
	// antenna min and max elevation
	
	antMinElev = new DoubleParam("antMinElev");
	antMinElev.setLabel("Antenna min el");
	antMinElev.setDescription("Min antenna elevation (deg)");
	antMinElev.setInfo
	    ("The antenna cannot go below this elevation." +
	     " Therefore the control program will not request elevation" +
	     " angles below this value.");
	antMinElev.setValue(0.0);
	
	antMaxElev = new DoubleParam("antMaxElev");
	antMaxElev.setLabel("Antenna max el");
	antMaxElev.setDescription("Max antenna elevation (deg)");
	antMaxElev.setInfo
	    ("The antenna cannot go above this elevation." +
	     " Therefore the control program will not request elevation" +
	     " angles above this value.");
	antMaxElev.setValue(45.0);
	
	// antenna max slew rates
	
	antMaxElSlewRate = new DoubleParam("antMaxElSlewRate");
	antMaxElSlewRate.setLabel("Antenna max el slew rate");
	antMaxElSlewRate.setDescription("Max el slew rate (deg/s)");
	antMaxElSlewRate.setInfo
	    ("The antenna cannot slew faster than this." +
	     " Therefore the control program will not request slew" +
	     " rates above this value.");
	antMaxElSlewRate.setValue(10.0);
	
	antMaxAzSlewRate = new DoubleParam("antMaxAzSlewRate");
	antMaxAzSlewRate.setLabel("Antenna max az slew rate");
	antMaxAzSlewRate.setDescription("Max az slew rate (deg/s)");
	antMaxAzSlewRate.setInfo
	    ("The antenna cannot slew faster than this." +
	     " Therefore the control program will not request slew" +
	     " rates above this value.");
	antMaxAzSlewRate.setValue(24.0);
	
	// add the params to the list
	
	add(siteNum);
	add(siteName);
	add(userName);
	add(latitude);
	add(longitude);
	add(altitude);
	add(peakXmitPower);
	add(xmitFreq);
	add(beamWidth);
	add(antGain);
	add(waveguideLoss);
	add(radomeLoss);
	add(testCableAtten);
	add(couplerAtten);
	add(displaceType);
	add(clutFilter);
	add(antMinElev);
	add(antMaxElev);
	add(antMaxElSlewRate);
	add(antMaxAzSlewRate);

	// copy the values to the defaults

	setDefaultFromValue();

    }

}


