///////////////////////////////////////////////////////////////////////
//
// TimeManager
//
// Mike Dixon
//
// Dec 2002
//
////////////////////////////////////////////////////////////////////////

import java.util.*;

public class TimeManager

{
    
    static private SimpleTimeZone _tz = new SimpleTimeZone(0, "UTC");

    static public long getTime() {
	Date date = new Date();
	return date.getTime();
    }
    
    static public Calendar getCal() {
	return Calendar.getInstance(_tz);
    }
    
}
