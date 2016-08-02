///////////////////////////////////////////////////////////////////////
//
// AntennaMode
//
// Type-safe enum for antenna modes
//
// Mike Dixon
//
// Dec 2002
//
////////////////////////////////////////////////////////////////////////

public class AntennaMode

{

    public final String name;

    private AntennaMode(String name) { this.name = name; }

    public String toString() { return name; }

    public static final AntennaMode MANUAL = new AntennaMode("manual");
    public static final AntennaMode AUTO_VOL = new AntennaMode("auto_vol");
    public static final AntennaMode AUTO_PPI = new AntennaMode("auto_ppi");
    public static final AntennaMode AUTO_RHI = new AntennaMode("auto_rhi");
    public static final AntennaMode FOLLOW_SUN = new AntennaMode("follow_sun");
    public static final AntennaMode STOP = new AntennaMode("stop");
    
}
