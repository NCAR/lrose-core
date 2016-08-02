///////////////////////////////////////////////////////////////////////
//
// CalibrationParams
//
// Calibration parameters
//
// Mike Dixon
//
// Jan 2003
//
////////////////////////////////////////////////////////////////////////

import params.*;

class CalibrationParams extends CollectionParam
	    
{

    public DoubleParam vipLow;
    public DoubleParam vipHigh;
    public DoubleParam powerLow;
    public DoubleParam powerHigh;
    public DoubleParam xmtPower;
    public DoubleParam radarConstant;
    public DoubleParam atmosAtten;

    // constructor

    public CalibrationParams(String name, String label, int depth)
    {
	super(name, label, depth);

	vipLow = new DoubleParam("vipLow");
	vipLow.setLabel("VipLow");
	vipLow.setDescription("VIP count low value (count)");
 	vipLow.setValue(273);
	
	vipHigh = new DoubleParam("vipHigh");
	vipHigh.setLabel("VipHigh");
	vipHigh.setDescription("VIP count high value (count)");
 	vipHigh.setValue(2835);
	
	powerLow = new DoubleParam("powerLow");
	powerLow.setLabel("PowerLow");
	powerLow.setDescription("Power low value (dBM)");
 	powerLow.setValue(-95.0);
	
	powerHigh = new DoubleParam("powerHigh");
	powerHigh.setLabel("PowerHigh");
	powerHigh.setDescription("Power high value (dBM)");
 	powerHigh.setValue(-40.0);
	
	xmtPower = new DoubleParam("xmtPower");
	xmtPower.setLabel("XMT power");
	xmtPower.setDescription("Transmitter power (dBM)");
 	xmtPower.setValue(84.0);
	
	radarConstant = new DoubleParam("radarConstant");
	radarConstant.setLabel("Radar constant");
	radarConstant.setDescription("Radar constant (mm6.m-3.mW.km-2)");
 	radarConstant.setValue(-158.0);

	atmosAtten = new DoubleParam("atmosAtten");
	atmosAtten.setLabel("Atmospheric attenuation");
	atmosAtten.setDescription
	    ("Atmospheric attenuation at radar wavelength (dB/km)");
 	atmosAtten.setValue(0.014);

	// add the params to the list
	
	add(vipLow);
	add(vipHigh);
	add(powerLow);
	add(powerHigh);
	add(xmtPower);
	add(radarConstant);
	add(atmosAtten);

	// copy the values to the defaults

	setDefaultFromValue();

    }

}
