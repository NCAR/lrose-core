/*
 *	$Id: Platform.h,v 1.1 2007/09/22 21:08:21 dixon Exp $
 *
 *	Module:		 Platform.h
 *	Original Author: Richard E. K. Neitzel
 *      Copywrited by the National Center for Atmospheric Research
 *	Date:		 $Date: 2007/09/22 21:08:21 $
 *
 * revision history
 * ----------------
 * $Log: Platform.h,v $
 * Revision 1.1  2007/09/22 21:08:21  dixon
 * adding
 *
 * Revision 1.1  2002/02/07 18:31:35  dixon
 * added
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

#ifdef OK_RPC

#if defined(UNIX) && defined(sun)
#include <rpc/rpc.h>
#else
#if defined(WRS)
#include "rpc/rpc.h"
#endif
#endif /* UNIX */

#endif /* OK_RPC */

struct platform_i {
    char  platform_info[4];	/* Identifier for the aircraft/ship */
				/* parameters block (ascii characters ASIB) */
    si32 platform_info_length;	/* Length in Bytes of the */
				/* aircraft/ship arameters block */
    fl32 longitude;		/* Antenna Longitude (Eastern */
				/* Hemisphere is positive, West */
				/* negative) in degrees */
    fl32 latitude;		/* Antenna Latitude (Northern */
				/* Hemisphere is positive, South */
				/* Negative) in degrees */
    fl32 altitude_msl;		/* Antenna Altitude above mean sea */
				/* level (MSL) in m */
    fl32 altitude_agl;		/* Antenna Altitude above ground level */
				/* (AGL) in m */
    fl32 ew_velocity;		/* Antenna east-west ground speed */
				/* (towards East is positive) in m/sec */
    fl32 ns_velocity;		/* Antenna north-south ground speed */
				/* (towards North is positive) in m/sec */
    fl32 vert_velocity;	/* Antenna vertical velocity (Up is */
				/* positive) in m/sec */
    fl32 heading;		/* Antenna heading (angle between */
				/* rotodome rotational axis and true */
				/* North, clockwise (looking down) */
				/* positive) in degrees */
    fl32 roll;			/* Roll angle of aircraft tail section */
				/* (Horizontal zero, Positive left wing up) */
				/* in degrees */
    fl32 pitch;		/* Pitch angle of rotodome (Horizontal */
				/* is zero positive front up) in degrees*/
    fl32 drift_angle;		/* Antenna drift Angle. (angle between */
				/* platform true velocity and heading, */
				/* positive is drift more clockwise */
				/* looking down) in degrees */
    fl32 rotation_angle;	/* Angle of the radar beam with */
				/* respect to the airframe (zero is */
				/* along vertical stabilizer, positive */
				/* is clockwise) in deg */
    fl32 tilt;			/* Angle of radar beam and line normal */
				/* to longitudinal axis of aircraft, */
				/* positive is towards nose of */
				/* aircraft) in degrees */
    fl32 ew_horiz_wind;	/* east - west wind velocity at the */
				/* platform (towards East is positive) */
				/* in m/sec */
    fl32 ns_horiz_wind;	/* North - South wind velocity at the */
				/* platform (towards North is */
				/* positive) in m/sec */
    fl32 vert_wind;		/* Vertical wind velocity at the */
				/* platform (up is positive) in m/sec */
    fl32 heading_change;	/* Heading change rate in degrees/second. */
    fl32 pitch_change;		/* Pitch change rate in degrees/second. */
}; /* End of Structure */


typedef struct platform_i platform_i;
typedef struct platform_i PLATFORM;
typedef struct platform_i AIRCRAFT;

#ifdef OK_RPC
#if defined(sun) || defined(WRS)
bool_t xdr_platform_i(XDR *, PLATFORM *);
#endif

#endif /* OK_RPC */

#endif /* INCPlatformh */

