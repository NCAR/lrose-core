///////////////////////////////////////////////////////////////////////
//
// Gutil  - graphics utilities
//
// Mike Dixon
//
// Jan 2003
//
////////////////////////////////////////////////////////////////////////

import java.util.*;

public class Gutil

{
    
    static public double[] linearTicks(double minVal,
				       double maxVal,
				       long approxNticks)
	
    {

	double  approxInterval =
	    (maxVal - minVal) / (double) (approxNticks + 1);
	double logInterval = Math.log(Math.abs(approxInterval)) / 2.30259;
	double intPart = (int) logInterval;
	double fractPart = logInterval - intPart;

	if (fractPart < 0) {
	    fractPart += 1.0;
	    intPart -= 1.0;
	}
	
	double rem = Math.pow(10.0, fractPart);
	double base;
	if (rem > 7.5) {
	    base = 10.0;
	} else if (rem > 3.5) {
	    base = 5.0;
	} else if (rem > 1.5) {
	    base = 2.0;
	} else {
	    base = 1.0;
	}
	
	double deltaTick = (base * Math.pow (10.0, intPart));
	double tickMin = Math.floor(minVal / deltaTick) * deltaTick;
	int nTicks = (int) ((maxVal - tickMin) / deltaTick) + 1;
	
	double ticks[] = new double[nTicks];
	for (int i = 0; i < nTicks; i++) {
	    ticks[i] = tickMin + i * deltaTick;
	}

	return ticks;
	
    }

    
}
