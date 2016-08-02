///////////////////////////////////////////////////////////////////////
//
// MessageType
//
// Type-safe enum for message types
//
// Mike Dixon
//
// Dec 2002
//
////////////////////////////////////////////////////////////////////////

public class MessageType

{

    public final String name;

    private MessageType(String name) { this.name = name; }

    public String toString() { return name; }

    public static final MessageType ASCII = new MessageType("ascii");
    public static final MessageType BEAM = new MessageType("beam");
    public static final MessageType STATUS = new MessageType("status");
    
}
