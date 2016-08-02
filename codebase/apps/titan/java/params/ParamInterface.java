///////////////////////////////////////////////////////////////////////
//
// ParamInterface
//
// Interface for standard methods for parameters
//
// Mike Dixon
//
// August 2002
//
////////////////////////////////////////////////////////////////////////

package params;

public interface ParamInterface

{

    public void setValue(String value);

    public void setValueFromDefault();

    public void setDefaultFromValue();

    public ParamType getType();

    public AbstractParamGui getGui();

}
