/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * RCS info
 *   $Author: dettling $
 *   $Locker:  $
 *   $Date: 2016/04/26 23:55:15 $
 *   $Revision: 1.1 $
 *   $State: Exp $
 *   $Log: sunae.h,v $
 *   Revision 1.1  2016/04/26 23:55:15  dettling
 *   *** empty log message ***
 *
 *   Revision 1.1  2014/03/11 20:10:50  dettling
 *   *** empty log message ***
 *
 *   Revision 1.2  2007-02-27 20:46:56  jcraig
 *   Addition of rcs tags.
 *
 *   Revision 1.1  2007/02/26 22:58:41  sgc
 *   Initial commit of NTDA v1.10
 *
 *   $Id: sunae.h,v 1.1 2016/04/26 23:55:15 dettling Exp $
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
-------------------------------------------------------------------------------
sunae.hh
-------------------------------------------------------------------------------
*/

#ifndef SUNAE_H

#define SUNAE_H

#include <math.h>

typedef struct {
    int		year, doy;
    double	hour, lat, lon,
		/* azimuth, elevation, hour angle, declination,
		   solar distance, zenith, air mass */
		az, el, ha, dec, zen, soldst, am;
} ae_pack;

double sunae(ae_pack *);

#ifndef M_DTOR
#define M_DTOR  0.0174532925199433
#define M_RTOD  57.2957795130823230
#define	M_2PI	6.2831853071795862320E0
#define M_HTOR  0.2617993877991494
#define M_RTOH  3.8197186342054881
#define M_HTOD  15.0
#define M_DTOH  0.0666666666666667
#endif

#endif

