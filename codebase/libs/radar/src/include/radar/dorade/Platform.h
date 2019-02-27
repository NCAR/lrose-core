/*
 *	$Id: Platform.h,v 1.2 2019/02/27 02:59:40 dixon Exp $
 *
 *	Module:		 Platform.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2019/02/27 02:59:40 $
 *
 * revision history
 * ----------------
 * $Log: Platform.h,v $
 * Revision 1.2  2019/02/27 02:59:40  dixon
 * Sync with changes and additions in EOL LROSE Git
 *
 * Revision 1.1  2008/01/24 22:22:32  rehak
 * Added dorade module
 *
 * Revision 1.1  2007/01/10 22:29:51  rehak
 * Initial version -- copied from refr1 and then updated to get rid of most compiler warnings and to reflect new location of include files
 *
 * Revision 1.1  2003/12/02 20:59:52  vanandel
 *
 * : Added Files:
 * : 	CellSpacingFP.h CellVector.h Comment.h Correction.h
 * : 	DataInput.hh IndexFields.hh Parameter.h Pdata.h Platform.h
 * : 	Qdata.h RadarDesc.h RadarPktHdr.hh Ray.h Sweep.h Volume.h
 * : 	WhichFloat.hh Xtra_stuff.h controller.hh dd_defines.h
 * : 	dd_math.h dorade_includes.h inline.h message.h super_SWIB.h
 * : 	viraq.h
 * : ----------------------------------------------------------------------
 * "adopted" from rdss/spol/include
 *
 * Revision 1.1  1996/12/20 19:57:18  oye
 * New.
 *
 * Revision 1.4  1995/01/18  20:45:33  case
 * Removed an ending comment from the log section.
 *
 * Revision 1.3  1995/01/18  20:08:03  case
 * Added a missing end of comment 
 *
 * Revision 1.2  1994/04/05  15:33:22  case
 * moved an ifdef RPC and changed an else if to make HP compiler happy..
 *
 * Revision 1.2  1991/10/15  19:17:45  thor
 * Added new variables.
 *
 * Revision 1.2  1991/10/15  19:17:45  thor
 * Added new variables.
 *
 * Revision 1.1  1991/10/15  16:45:52  thor
 * Initial revision
 *
 * Revision 1.1  1991/08/30  18:39:34  thor
 * Initial revision
 *
 *
 *
 * description:
 *        
 */
#ifndef INCPlatformh
#define INCPlatformh

struct platform_i {
    char  platform_info[4];	/* Identifier for the aircraft/ship */
				/* parameters block (ascii characters ASIB) */
    long platform_info_length;	/* Length in Bytes of the */
				/* aircraft/ship arameters block */
    float longitude;		/* Antenna Longitude (Eastern */
				/* Hemisphere is positive, West */
				/* negative) in degrees */
    float latitude;		/* Antenna Latitude (Northern */
				/* Hemisphere is positive, South */
				/* Negative) in degrees */
    float altitude_msl;		/* Antenna Altitude above mean sea */
				/* level (MSL) in m */
    float altitude_agl;		/* Antenna Altitude above ground level */
				/* (AGL) in m */
    float ew_velocity;		/* Antenna east-west ground speed */
				/* (towards East is positive) in m/sec */
    float ns_velocity;		/* Antenna north-south ground speed */
				/* (towards North is positive) in m/sec */
    float vert_velocity;	/* Antenna vertical velocity (Up is */
				/* positive) in m/sec */
    float heading;		/* Antenna heading (angle between */
				/* rotodome rotational axis and true */
				/* North, clockwise (looking down) */
				/* positive) in degrees */
    float roll;			/* Roll angle of aircraft tail section */
				/* (Horizontal zero, Positive left wing up) */
				/* in degrees */
    float pitch;		/* Pitch angle of rotodome (Horizontal */
				/* is zero positive front up) in degrees*/
    float drift_angle;		/* Antenna drift Angle. (angle between */
				/* platform true velocity and heading, */
				/* positive is drift more clockwise */
				/* looking down) in degrees */
    float rotation_angle;	/* Angle of the radar beam with */
				/* respect to the airframe (zero is */
				/* along vertical stabilizer, positive */
				/* is clockwise) in deg */
    float tilt;			/* Angle of radar beam and line normal */
				/* to longitudinal axis of aircraft, */
				/* positive is towards nose of */
				/* aircraft) in degrees */
    float ew_horiz_wind;	/* east - west wind velocity at the */
				/* platform (towards East is positive) */
				/* in m/sec */
    float ns_horiz_wind;	/* North - South wind velocity at the */
				/* platform (towards North is */
				/* positive) in m/sec */
    float vert_wind;		/* Vertical wind velocity at the */
				/* platform (up is positive) in m/sec */
    float heading_change;	/* Heading change rate in degrees/second. */
    float pitch_change;		/* Pitch change rate in degrees/second. */
}; /* End of Structure */


typedef struct platform_i platform_i;
typedef struct platform_i PLATFORM;
typedef struct platform_i AIRCRAFT;

#endif /* INCPlatformh */

