///////////////////////////////////////////////////////////////////////
//
// StatusMessage
//
// Mike Dixon
//
// Dec 2002
//
////////////////////////////////////////////////////////////////////////

public class StatusMessage implements Message

{
    
    private MessageType _type = MessageType.STATUS;
    private boolean _mainPower = false;
    private boolean _magnetronPower = false;
    private boolean _servoPower = false;
    private boolean _radiate = false;

    public StatusMessage() {
    }

    public MessageType getType() {
	return _type;
    }
    
    public boolean getMainPower() {
	return _mainPower;
    }
    
    public boolean getMagnetronPower() {
	return _magnetronPower;
    }
    
    public boolean getServoPower() {
	return _servoPower;
    }

    public boolean getRadiate() {
	return _radiate;
    }

    public void setMainPower(boolean state) {
	_mainPower = state;
    }
    
    public void setMagnetronPower(boolean state) {
	_magnetronPower = state;
    }
    
    public void setServoPower(boolean state) {
	_servoPower = state;
    }
    
    public void setRadiate(boolean state) {
	_radiate = state;
    }
    
}
