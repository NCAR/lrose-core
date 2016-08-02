///////////////////////////////////////////////////////////////////////
//
// Simulator
//
// Simulate the radar
//
// Mike Dixon
//
// Dec 2002
//
////////////////////////////////////////////////////////////////////////

import java.text.*;
import java.util.*;
import java.lang.*;

public class Simulator extends Thread {
    
    private Parameters _params;
    private MessageQueue _commandQueue;
    private MessageQueue _replyQueue;

    private long _antSleepMsecs = 30;
    private long _beamIntMsecs = 30;

    private boolean _requestedMainPower = false;
    private boolean _requestedMagnetronPower = false;
    private boolean _requestedServoPower = false;
    private boolean _requestedRadiate = false;

    private boolean _mainPower = false;
    private boolean _magnetronPower = false;
    private boolean _servoPower = false;
    private boolean _radiate = false;

    private double _requestedEl = 0.0;
    private double _requestedAz = 0.0;
    private double _actualEl = 0.0;
    private double _actualAz = 0.0;
    
    private double _prf = 1000.0;
    private double _azSlewRate = 18.0;
    private double _elSlewRate = 10.0;
    private int _nGates = 256;
    private double _startRange = 0.3;
    private double _gateSpacing = 0.6;
    private ArrayList _elList = new ArrayList();

    private boolean _startVol = false;
    
    private AntennaMode _antennaMode = AntennaMode.MANUAL;
    
    private NumberFormat _nformat2;

    private PowerManager _powerManager;
    private AntennaManager _antennaManager;
    private BeamManager _beamManager;
    private StatusManager _statusManager;
    
    public Simulator(Parameters params,
		     MessageQueue commandQueue,
		     MessageQueue replyQueue) {
	
	_params = params;
	_commandQueue = commandQueue;
	_replyQueue = replyQueue;

	_powerManager = new PowerManager();
	_powerManager.start();
	
	_antennaManager = new AntennaManager();
	_antennaManager.start();
	
	_beamManager = new BeamManager();
	_beamManager.start();
	
	_statusManager = new StatusManager();
	_statusManager.start();
	
	// prepare 2-digit formatter

	_nformat2 = NumberFormat.getInstance();
	_nformat2.setMinimumFractionDigits(2);
	_nformat2.setMaximumFractionDigits(2);
	_nformat2.setGroupingUsed(false);

    }

    public void setEl(double el) {
	_actualEl = el;
    }
    
    public void setAz(double az) {
	_actualAz = az;
    }

    public void setAntennaMode(AntennaMode mode) {
	synchronized(_antennaMode) {
	    _antennaMode = mode;
	}
    }
    
    public AntennaMode getAntennaMode() {
	synchronized(_antennaMode) {
	    return _antennaMode;
	}
    }
    
    // provide run method
    
    public void run() {
	
	while (true) {

	    // get a command from the queue
	    
	    AsciiMessage command = (AsciiMessage) _commandQueue.pop();
	    
	    if (command == null) {
		// sleep a bit
		Thread t = Thread.currentThread();
		try { t.sleep(50); }
		catch (InterruptedException e) { }
		continue;
	    }

	    if (_params.debug.getValue()) {
		System.err.println("Simulator received command: " + command);
	    }

	    // set things appropriately
	    
	    if (command.getKey().equals("main_power")) {
		
		if (command.getValue().equals("on")) {
		    _requestedMainPower = true;
		} else {
		    _requestedMainPower = false;
		    _requestedMagnetronPower = false;
		    _requestedServoPower = false;
		    _requestedRadiate = false;
		}

	    } else if (command.getKey().equals("magnetron_power")) {

		if (command.getValue().equals("on")) {
		    _requestedMagnetronPower = true;
		} else {
		    _requestedMagnetronPower = false;
		    _requestedRadiate = false;
		}

	    } else if (command.getKey().equals("servo_power")) {
		
		if (command.getValue().equals("on")) {
		    _requestedServoPower = true;
		} else {
		    _requestedServoPower = false;
		}

	    } else if (command.getKey().equals("radiate")) {

		if (command.getValue().equals("on")) {
		    _requestedRadiate = true;
		} else {
		    _requestedRadiate = false;
		}
		
	    } else if (command.getKey().equals("elevation")) {

		_requestedEl = Double.parseDouble(command.getValue());

	    } else if (command.getKey().equals("azimuth")) {

		_requestedAz = Double.parseDouble(command.getValue());

	    } else if (command.getKey().equals("prf")) {

		_prf = Double.parseDouble(command.getValue());

	    } else if (command.getKey().equals("az_slew_rate")) {
		
		_azSlewRate = Double.parseDouble(command.getValue());

	    } else if (command.getKey().equals("el_slew_rate")) {
		
		_elSlewRate = Double.parseDouble(command.getValue());

	    } else if (command.getKey().equals("start_range")) {

		_startRange = Double.parseDouble(command.getValue());

	    } else if (command.getKey().equals("gate_spacing")) {
		
		_gateSpacing = Double.parseDouble(command.getValue());
		
	    } else if (command.getKey().equals("n_gates")) {
		
		_nGates = Integer.parseInt(command.getValue());
		
	    } else if (command.getKey().equals("elevation_steps")) {
		
		String[] elevs = command.getValue().split(",");
		_elList.clear();
		for (int i = 0; i < elevs.length; i++) {
		    double el = 0.0;
		    try {
			el = Double.parseDouble(elevs[i]);
		    }
		    catch (java.lang.NumberFormatException nfe) {
			System.err.println(nfe);
		    }
		    _elList.add(new Double(el));
		}
		
	    } else if (command.getKey().equals("antenna_mode")) {

		if (command.getValue().equals("manual")) {
		    setAntennaMode(AntennaMode.MANUAL);
		} else if (command.getValue().equals("auto_vol")) {
		    if (_elList.size() > 0) {
			setAntennaMode(AntennaMode.AUTO_VOL);
			_startVol = true;
    		    }
		} else if (command.getValue().equals("auto_ppi")) {
		    _requestedEl = _actualEl;
		    _requestedAz = _actualAz;
		    setAntennaMode(AntennaMode.AUTO_PPI);
		} else if (command.getValue().equals("stop")) {
		    _requestedEl = _actualEl;
		    _requestedAz = _actualAz;
		    setAntennaMode(AntennaMode.STOP);
		}

	    }
	    
	} // while
	
    }

    // power management

    public class PowerManager extends Thread {

	public void run() {
	    
	    while (true) {
		
		// sleep a bit
		Thread t = Thread.currentThread();
		try { t.sleep(1500); }
		catch (InterruptedException e) { }
		
		// check for changes
		
		if (_mainPower != _requestedMainPower) {
		    _mainPower = _requestedMainPower;
		}
		
		if (_magnetronPower != _requestedMagnetronPower) {
		    if (_mainPower) {
			_magnetronPower = _requestedMagnetronPower;
		    } else {
			_magnetronPower = false;
		    }
		}
		
		if (_servoPower != _requestedServoPower) {
		    if (_mainPower) {
			 _servoPower = _requestedServoPower;
		    } else {
			_servoPower = false;
		    }
		}
		
		if (_radiate != _requestedRadiate) {
		    if (_magnetronPower) {
			_radiate = _requestedRadiate;
		    } else {
			_radiate = false;
		    }
		}

	    } // while

	} // run()
	
    } // power management

    // antenna management
    
    public class AntennaManager extends Thread {

	private double _sumAzPpi = 0.0;
	private int _volElevIndex = 0;

	public void run() {
	    
	    while (true) {
		
		// sleep
		try { Thread.currentThread().sleep(_antSleepMsecs); }
		catch (InterruptedException e) { }

		if (!_servoPower) {
		    continue;
		}
		
		if (getAntennaMode() == AntennaMode.MANUAL) {
		    
		    while (!adjustElAz(_requestedEl, _requestedAz)) {
			// sleep
			try { Thread.currentThread().sleep(_antSleepMsecs); }
			catch (InterruptedException e) { }
			if (getAntennaMode() != AntennaMode.MANUAL) {
			    break;
			}
		    }
		    
		} else if (getAntennaMode() == AntennaMode.AUTO_VOL) {
		    
		    // at start, move to starting location

		    if (_startVol) {
			_volElevIndex = 0;
			double el =
			    ((Double) _elList.get(_volElevIndex)).doubleValue();
			double az = 0.0;
			while (!adjustElAz(el, az)) {
			    // sleep
			    try { Thread.currentThread().sleep(_antSleepMsecs); }
			    catch (InterruptedException e) { }
			    if (getAntennaMode() != AntennaMode.AUTO_VOL) {
				break;
			    }
			}
			_startVol = false;
			_sumAzPpi = 0.0;
		    }

		    // if done with one ppi, move to next elevation
		    
		    if (_sumAzPpi >= 360.0) {
			
			_sumAzPpi = 0.0;
			_volElevIndex++;
			if (_volElevIndex > _elList.size() - 1) {
			    // done with this volume
			    _startVol = true;
			    continue;
			}
			
			// compute desired position
			
			double el =
			    ((Double) _elList.get(_volElevIndex)).doubleValue();
			
			// go there, slewing in azimuth
			
			while (!adjustElSlewAz(el)) {
			    // sleep
			    try { Thread.currentThread().sleep(_antSleepMsecs); }
			    catch (InterruptedException e) { }
			    if (getAntennaMode() != AntennaMode.AUTO_VOL) {
				break;
			    }
			}
			
		    }
		    
		    // move in azimuth, keeping track of how much we slewed
		    
		    double deltaAz = slewAz();
		    _sumAzPpi += Math.abs(deltaAz);
		    
		} else if (getAntennaMode() == AntennaMode.AUTO_PPI) {

		    slewAz();

		} else if (getAntennaMode() == AntennaMode.STOP) {

		} // if (getAntennaMode() ...
		
	    } // while

	} // run()

	// adjust towards requested El and Az

	private boolean adjustElAz(double el, double az) {
	    
	    boolean done = true;
	    
	    // adjust elevation
	    
	    double errorEl = Math.abs(el - _actualEl);
	    if (errorEl < 0.0001) {
		_actualEl = el;
	    } else {
		done = false;
		double slewRate = _elSlewRate;
		if (errorEl < 5.0) {
		    slewRate *= (errorEl / 5.0);
		}
		if (slewRate < 0.25) {
		    slewRate = 0.25;
		}
		double deltaEl = slewRate * (_antSleepMsecs / 1000.0);
		if (deltaEl > errorEl) {
		    deltaEl = errorEl;
		}
		if (el < _actualEl) {
		    deltaEl *= -1.0;
		}
		_actualEl += deltaEl;
	    }
	    
	    // adjust azimuth
	    
	    double errorAz = Math.abs(az - _actualAz);
	    double sign = 1.0;
	    if (errorAz > 180.0) {
		errorAz = 360.0 - errorAz;
		sign = -1.0;
	    }
	    if (errorAz < 0.0001) {
		_actualAz = az;
	    } else {
		done = false;
		double slewRate = _azSlewRate;
		if (errorAz < 5.0) {
		    slewRate *= (errorAz / 5.0);
		}
		if (slewRate < 0.25) {
		    slewRate = 0.25;
		}
		double deltaAz = slewRate * (_antSleepMsecs / 1000.0);
		if (deltaAz > errorAz) {
		    deltaAz = errorAz;
		}
		if (az < _actualAz) {
		    deltaAz *= -1.0;
		}
		deltaAz *= sign;
		_actualAz += deltaAz;
		if (_actualAz < 0) {
		    _actualAz += 360.0;
		}
		if (_actualAz >= 360) {
		    _actualAz -= 360.0;
		}
		if (Math.abs(_actualAz - 0) < 0.00001 ||
		    Math.abs(_actualAz - 360) < 0.00001) {
		    _actualAz = 0.0;
		}
		
	    }

	    return done;

	} // adjustElAz()

	// adjust towards requested El, slew in azimuth

	private boolean adjustElSlewAz(double el) {
	    
	    boolean done = true;
	    
	    // adjust elevation
	    
	    double errorEl = Math.abs(el - _actualEl);
	    if (errorEl < 0.0001) {
		_actualEl = el;
	    } else {
		done = false;
		double slewRate = _elSlewRate;
		if (errorEl < 5.0) {
		    slewRate *= (errorEl / 5.0);
		}
		if (slewRate < 0.25) {
		    slewRate = 0.25;
		}
		double deltaEl = slewRate * (_antSleepMsecs / 1000.0);
		if (deltaEl > errorEl) {
		    deltaEl = errorEl;
		}
		if (el < _actualEl) {
		    deltaEl *= -1.0;
		}
		_actualEl += deltaEl;
	    }
	    
	    // slew azimuth

	    slewAz();

	    return done;

	} // adjustElSlewAz()
	
	// slew in azimuth - return how much slewed

	private double slewAz() {
	    
	    // slew azimuth
	    
	    double deltaAz = _azSlewRate * (_antSleepMsecs / 1000.0);
	    _actualAz += deltaAz;
	    if (_actualAz < 0) {
		_actualAz += 360.0;
	    }
	    if (_actualAz >= 360) {
		_actualAz -= 360.0;
	    }

	    return deltaAz;

	} // slewAz	

    } // antenna management

    // beam management
    
    public class BeamManager extends Thread {
	
	public void run() {
	    
	    while (true) {

		// sleep 
		Thread t = Thread.currentThread();
		try { t.sleep(_beamIntMsecs); }
		catch (InterruptedException e) { }
		
		BeamMessage beam = new BeamMessage();
		Date date = new Date();
		beam.setDate(date);
		beam.setEl(_actualEl);
		beam.setAz(_actualAz);
		beam.setStartRange(_startRange);
		beam.setGateSpacing(_gateSpacing);
		beam.setPrf(_prf);
		beam.setNGates(_nGates);
		if (_mainPower) {
		    beam.simCounts();
		}
		_replyQueue.push(beam);
		
	    }

	} // run
	
    } // BeamManager
		
    // status management
    
    public class StatusManager extends Thread {
	
	public void run() {
	    
	    while (true) {

		// sleep 1 s
		Thread t = Thread.currentThread();
		try { t.sleep(1000); }
		catch (InterruptedException e) { }
		
		StatusMessage status = new StatusMessage();
		status.setMainPower(_mainPower);
		status.setMagnetronPower(_magnetronPower);
		status.setServoPower(_servoPower);
		status.setRadiate(_radiate);
		_replyQueue.push(status);

	    }

	} // run
	
    } // StatusManager
		
}



