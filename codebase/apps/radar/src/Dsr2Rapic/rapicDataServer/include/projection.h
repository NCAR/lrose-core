#ifndef __PROJECTION_H__
#define __PROJECTION_H__

enum	proj_mode {PERSP, ORTHO};

enum	rpProjection {
//		        init win constraint	description
    projUndefined, 
    projSimpleSatellite,    //	full SVISSR Grid size	map Grid to corners
    projNavSatellite,	    // navigated satellite projection, NOT FULL DISK, only =/- 60deg
    projMercator,           //
    projEquiDistCyl,        //
    projStereo,             //
    proj3DOrthog            // most common proj for radars
    };

enum	rpProjUnits {
    arrayWidth,	    // projSatellite defines window to unit of buffer size
    degrees,	    // proLatLong uses units of degrees
    kms		    // most other projections will utilise km units
    };


#endif /* __PROJECTION_H__ */
