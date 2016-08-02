///////////////////////////////////////////////////////////////////////
//
// SunTrack
//
// Keeps track of the sun position
//
// Mike Dixon
//
// Nov 2002
//
////////////////////////////////////////////////////////////////////////

import java.util.*;
import javax.swing.*;

public class SunTrack implements Runnable {

    Runnable updateControlPanel;
    private Parameters _params;
    private SimpleTimeZone _tz;
    private ControlPanel _controlPanel;
    private double _el, _az;
    
    public SunTrack(Parameters params,
		    ControlPanel controlPanel) {
	_params = params;
	_tz = new SimpleTimeZone(0, "UTC");
	_controlPanel = controlPanel;
	_el = -99.0;
	_az = -99.0;

	// special method to update control panel correctly
	// since we are in a separate thread
	updateControlPanel = new Runnable() {
		public void run() {
		    _controlPanel.setSunPosition(_el, _az);
		}
	    };

    }
    
    // provide run method

    public void run() {
	
	while (true) {
	    
	    // get the time in UTC
	    Calendar cal = Calendar.getInstance(_tz);

	    // compute the sun position

	    computePosn(cal);

	    // update the control panel

	    SwingUtilities.invokeLater(updateControlPanel);

	    // sleep for a second
	    
	    Thread t = Thread.currentThread();
	    try { t.sleep(1000); }
	    catch (InterruptedException e) { return; }
	    
	} // while
	
    }
    
    // compute sun position

    private void computePosn(Calendar cal) {

	// get the time now in msecs

	double now = cal.getTime().getTime();
	
	// get the time at the start of the year in msecs

	cal.set(Calendar.MONTH, 0);
	cal.set(Calendar.DAY_OF_MONTH, 1);
	cal.set(Calendar.HOUR_OF_DAY, 0);
	cal.set(Calendar.MINUTE, 0);
	cal.set(Calendar.SECOND, 0);
	cal.set(Calendar.MILLISECOND, 0);
	double yearStart = cal.getTime().getTime();
	double jday = (now - yearStart) / 86400000.0;
	double jdeg = jday * (360.0 / 365.25);
	
	double eqt = 0.123 * Math.cos(Math.toRadians(jdeg + 87.0)) -
	    0.16666667 * Math.sin(Math.toRadians(jdeg + 10.0) * 2.0);
	
	double decl =
	    -23.5 * Math.cos(Math.toRadians((jday + 10.3) * (360.0 / 365.25)));
	double decl_rad = Math.toRadians(decl);
	
	double gmt_hr =
	    Math.IEEEremainder(((now / 3600000.0) - 12.0), 24.0);

	double solar_time =
	    (gmt_hr + (_params.radar.longitude.getValue() / 15.0) + eqt);
	
	double hour_deg = solar_time * 15.0;
	double hour_rad = Math.toRadians(hour_deg);
	
	double lat = Math.toRadians(_params.radar.latitude.getValue());
	double sin_el = Math.sin(lat) * Math.sin(decl_rad) +
	    Math.cos(lat) * Math.cos(decl_rad) * Math.cos(hour_rad);
	double el = Math.asin(sin_el);
	double el_deg = Math.toDegrees(el);
	
	double tan_az = (Math.sin(hour_rad) /
			 ((Math.cos(hour_rad) * Math.sin(lat)) -
			  (Math.tan(decl_rad) * Math.cos(lat))));
	
	double az = Math.atan(tan_az);
	double az_deg = Math.toDegrees(az);
	
	if (hour_deg >= 0.0 && hour_deg < 180.0) {
	    if (az_deg > 0.0) {
		az_deg += 180.0;
	    } else {
		az_deg += 360.0;
	    }
	} else {
	    if (az_deg < 0.0) {
		az_deg += 180.0;
	    }
	}
	
	if (_params.verbose.getValue()) {
	    System.err.println("    jday: " + jday);
	    System.err.println("    jdeg: " + jdeg);
	    System.err.println("    gmt_hr: " + gmt_hr);
	    System.err.println("    solar_time: " + solar_time);
	    System.err.println("    hour_deg: " + hour_deg);
	    System.err.println("    lat: " + lat);
	    System.err.println("    el: " + el_deg);
	    System.err.println("    az: " + az_deg);
	}

	_el = el_deg;
	_az = az_deg;

    }
    
}


