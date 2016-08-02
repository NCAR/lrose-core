///////////////////////////////////////////////////////////////////////
//
// ParamType
//
// Type-safe enum for parameter types
//
// Mike Dixon
//
// October 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public class ParamType

{

    public final String name;

    private ParamType(String name) { this.name = name; }

    public String toString() { return name; }

    public static final ParamType STRING = new ParamType("String");
    public static final ParamType BOOLEAN = new ParamType("Boolean");
    public static final ParamType INTEGER = new ParamType("Integer");
    public static final ParamType FLOAT = new ParamType("Float");
    public static final ParamType DOUBLE = new ParamType("Double");
    public static final ParamType OPTION = new ParamType("Option");
    public static final ParamType COLLECTION = new ParamType("Collection");
    
}
