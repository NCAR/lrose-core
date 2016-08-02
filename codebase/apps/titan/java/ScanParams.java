///////////////////////////////////////////////////////////////////////
//
// ScanParams
//
// Scan strategy parameters
//
// Mike Dixon
//
// Sept 2002
//
////////////////////////////////////////////////////////////////////////

import params.*;

class ScanParams extends CollectionParam
	    
{

    public StringParam elevationList;
    public DoubleParam azSlewRate;
    public DoubleParam elSlewRate;
    public IntegerParam nGates;
    public DoubleParam startRange;
    public DoubleParam gateSpacing;
    public DoubleParam prf;
    
    // constructor

    public ScanParams(String name, String label, int depth)
    {
	super(name, label, depth);

	elevationList = new StringParam("elevationList");
	elevationList.setLabel("Elevation list");
	elevationList.setDescription("Comma-delimited elevation angle list");
	elevationList.setInfo("Enter the elevation angles in a" +
			      " comma-delimited list.");
	elevationList.setValue("0.5, 1.5, 2.5, 3.5, 4.5," +
			       " 5.5, 6.5, 7.5, 8.5, 9.9");
	
	azSlewRate = new DoubleParam("azSlewRate");
	azSlewRate.setLabel("Azimuth slew rate");
	azSlewRate.setDescription("Antenna slew rate in azimuth (deg/sec)");
 	azSlewRate.setValue(18.0);
	
	elSlewRate = new DoubleParam("elSlewRate");
	elSlewRate.setLabel("Elevation slew rate");
	elSlewRate.setDescription("Antenna slew rate in elevation (deg/sec)");
 	elSlewRate.setValue(10.0);
	
	nGates = new IntegerParam("nGates");
	nGates.setLabel("N Gates");
	nGates.setDescription("Number of gates per beam");
 	nGates.setValue(500);

	startRange = new DoubleParam("startRange");
	startRange.setLabel("Start range");
	startRange.setDescription("Range to center of first gate (km)");
 	startRange.setValue(0.125);

	gateSpacing = new DoubleParam("gateSpacing");
	gateSpacing.setLabel("Gate spacing");
	gateSpacing.setDescription("Spacing between gates (km)");
 	gateSpacing.setValue(0.250);
	
	prf = new DoubleParam("prf");
	prf.setLabel("PRF");
	prf.setDescription("Pulse Repetition Frequency (/s)");
 	prf.setValue(1000);

	// add the params to the list
	
	add(elevationList);
	add(azSlewRate);
	add(elSlewRate);
	add(nGates);
	add(startRange);
	add(gateSpacing);
	add(prf);

	// copy the values to the defaults

	setDefaultFromValue();

    }

}
