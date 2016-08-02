///////////////////////////////////////////////////////////////////////
//
// AsciiMessage
//
// ASCII Message
//
// Mike Dixon
//
// Dec 2002
//
////////////////////////////////////////////////////////////////////////

public class AsciiMessage implements Message

{
    
    private MessageType _type = MessageType.ASCII;
    private String _key;
    private String _value;
    
    public AsciiMessage(String key,
			String value) {
	_key = new String(key);
	_value = new String(value);
    }

    public MessageType getType() {
	return _type;
    }

    public String getKey() {
	return _key;
    }

    public String getValue() {
	return _value;
    }

    public String toString() {
	return new String(_key + "=" + _value);
    }

}
