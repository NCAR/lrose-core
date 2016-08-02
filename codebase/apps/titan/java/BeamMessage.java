///////////////////////////////////////////////////////////////////////
//
// BeamMessage
//
// Mike Dixon
//
// Dec 2002
//
////////////////////////////////////////////////////////////////////////

import java.util.*;

public class BeamMessage implements Message

{
    
    private MessageType _type = MessageType.BEAM;
    private Date _date;
    private double _el;
    private double _az;
    private double _startRange;
    private double _gateSpacing;
    private int _nGates;
    private double _prf;
    private short _counts[];
    
    public BeamMessage() {
    }

    public MessageType getType() {
	return _type;
    }

    public Date getDate() {
	return _date;
    }

    public double getEl() {
	return _el;
    }

    public double getAz() {
	return _az;
    }

    public double getStartRange() {
	return _startRange;
    }

    public double getGateSpacing() {
	return _gateSpacing;
    }
    
    public int getNGates() {
	return _nGates;
    }

    public double getPrf() {
	return _prf;
    }

    public short[] getCounts() {
	return _counts;
    }

    public void setDate(Date date) {
	_date = date;
    }
    
    public void setEl(double el) {
	_el = el;
    }
    
    public void setAz(double az) {
	_az = az;
    }

    public void setStartRange(double startRange) {
	_startRange = startRange;
    }

    public void setGateSpacing(double gateSpacing) {
	_gateSpacing = gateSpacing;
    }

    public void setPrf(double prf) {
	_prf = prf;
    }
    
    public void setNGates(int nGates) {
	_nGates = nGates;
	_counts = new short[nGates];
    }
    
    public void simCounts() {
	for (int i = 0; i < _nGates; i++) {
	    _counts[i] = (short) (500 + (int) (Math.random() * 30));
	}
    }

}
