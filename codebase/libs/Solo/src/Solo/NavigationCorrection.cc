// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// NavigationCorrection.cc
//
// Apply corrections to the radar angles for airborne systems
//
// Original code from Soloii developed by Dick Oye
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2022
//
///////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include <string>
#include <Solo/dd_math.h>
#include <Solo/GeneralDefinitions.hh>

//
// results are stored in the cfac structure
//
/* original code ...

void dd_radar_angles( asib, cfac, ra, dgi )
  struct platform_i *asib;
  struct correction_d *cfac;
  struct radar_angles *ra;
  struct dd_general_info *dgi;
{
    // compute the true azimuth, elevation, etc. from platform
    // parameters using Testud's equations with their different
    // definitions of rotation angle, etc.
    //

    double R, H, P, D, T, theta_a, tau_a, theta_t, tau_t, lambda_t, phi_t;
    double sinP, cosP, sinT, cosT, sinD, cosD, sinR, cosR;
    double sin_theta_rc, cos_theta_rc, sin_tau_a, cos_tau_a;
    double xsubt, ysubt, zsubt, lambda, phi;

    double d, bigR, az;
    double sin_el, cos_az, cos_el, cos_psi, cos_phi;
    double sin_pitch, cos_pitch, cos_drift, sin_tilt, cos_tilt;
    double rtilt, rtrack, rphi;
    double rpitch=RADIANS(asib_pitch +cfac_pitch_corr);
    double rhead=RADIANS(asib_heading +cfac_heading_corr);
    double rdrift=RADIANS(asib_drift_angle +cfac_drift_corr);
    double azr, azd;
    double lambda_a, phi_a;
    double sin_lambda_a, cos_lambda_a, sin_phi_a, cos_phi_a;


# ifdef WEN_CHOWS_ALGORITHM
    //
    // see Wen-Chau Lee's paper
    // "Mapping of the Airborne Doppler Radar Data"
    //
    d = asib_roll;
    R = dd_isnanf(d) ? 0 : RADIANS(d +cfac_roll_corr);

    d = asib_pitch;
    P = dd_isnanf(d) ? 0 : RADIANS(d +cfac_pitch_corr);

    d = asib_heading;
    H = dd_isnanf(d) ? 0 : RADIANS(d +cfac_heading_corr);

    d = asib_drift_angle;
    D = dd_isnanf(d) ? 0 : RADIANS(d +cfac_drift_corr);

    sinP = sin(P);
    cosP = cos(P);
    sinD = sin(D);
    cosD = cos(D);
    
    T = H + D;

    if( dgi->dds->radd->radar_type == AIR_LF ||
	dgi->dds->radd->radar_type == AIR_NOSE ) {

	d = ( dgi->dds->ryib->azimuth );
	lambda_a = dd_isnanf(d) ? 0 : RADIANS
	  ( CART_ANGLE( d +cfac_azimuth_corr ));
	sin_lambda_a = sin(lambda_a);
	cos_lambda_a = cos(lambda_a);

	d = ( dgi->dds->ryib->elevation );
	phi_a = dd_isnanf(d) ? 0 : RADIANS( d +cfac_elevation_corr );
	sin_phi_a = sin(phi_a);
	cos_phi_a = cos(phi_a);

	sinR = sin(R);
	cosR = cos(R);

	ra_x = xsubt = cos_lambda_a * cos_phi_a *
	    (cosD * cosR - sinD * sinP * sinR) -
	    sinD * cosP * sin_lambda_a * cos_phi_a +
	    sin_phi_a * ( cosD * sinR + sinD * sinP * cosR );

	ra_y = ysubt = cos_lambda_a * cos_phi_a *
	    ( sinD * cosR + cosD * sinP * sinR ) +
	    cosD * cosP * sin_lambda_a * cos_phi_a +
	    sin_phi_a * ( sinD * sinR - cosD * sinP * cosR );

	ra_z = zsubt = -cosP * sinR * cos_lambda_a * cos_phi_a +
	    sinP * sin_lambda_a * cos_phi_a +
	    cosP * cosR * sin_phi_a;

	ra_rotation_angle = theta_t = atan2(xsubt, zsubt);
	ra_tilt = tau_t = asin(ysubt);
	lambda_t = atan2(xsubt, ysubt);
	ra_azimuth = fmod(lambda_t + T, TWOPI);
	ra_elevation = phi_t = asin(zsubt);

	return;
    }

    sinT = sin(T);
    cosT = cos(T);

    d = asib_rotation_angle;
    theta_a = dd_isnanf(d) ? 0 : RADIANS(d +cfac_rot_angle_corr);
    
    d = asib_tilt;
    tau_a = dd_isnanf(d) ? 0 : RADIANS(d +cfac_tilt_corr);
    sin_tau_a = sin(tau_a);
    cos_tau_a = cos(tau_a);
    sin_theta_rc = sin(theta_a + R); // roll corrected rotation angle 
    cos_theta_rc = cos(theta_a + R); // roll corrected rotation angle 

    ra_x = xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
	     + cosD * sin_theta_rc * cos_tau_a
	     -sinD * cosP * sin_tau_a);
    ra_y = ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
	     + sinD * sin_theta_rc * cos_tau_a
	     + cosP * cosD * sin_tau_a);
    ra_z = zsubt = (cosP * cos_tau_a * cos_theta_rc
	     + sinP * sin_tau_a);

    ra_rotation_angle = theta_t = atan2(xsubt, zsubt);
    ra_tilt = tau_t = asin(ysubt);
    lambda_t = atan2(xsubt, ysubt);
    ra_azimuth = fmod(lambda_t + T, TWOPI);
    ra_elevation = phi_t = asin(zsubt);
    return;

# else

    rtrack = rhead +rdrift;
    ra_rotation_angle = RADIANS(asib_rotation_angle +cfac_rot_angle_corr
			+asib_roll +cfac_roll_corr);
    ra_tilt = RADIANS(asib_tilt +cfac_tilt_corr);
    azr = rhead +PI -(PIOVR2+ra_tilt)*sin(ra_rotation_angle);
    
    rphi = (ra_rotation_angle + PI);
    sin_pitch = sin(rpitch); cos_pitch = cos(rpitch);
    sin_tilt = sin(ra_tilt); cos_tilt = cos(ra_tilt);
    cos_drift = cos(rdrift);
    cos_phi = cos(rphi);
    
    cos_psi = cos_pitch*cos_drift*sin_tilt
	  + sin_pitch*cos_drift*cos_tilt*cos_phi
		- sin(rdrift)*cos_tilt*sin(rphi);
    // psi is the angle between the beam and the
    // horizontal component of the aircraft velocity vector
    //
    ra_psi = acos(cos_psi);

    sin_el = sin_tilt*sin_pitch-cos_tilt*cos_pitch*cos_phi;
    ra_elevation = asin(sin_el);
    cos_el = cos(ra_elevation);
    cos_az = (sin_tilt-sin_pitch*sin_el)/(cos_pitch*cos_el);

    if(fabs(cos_az) <= 1.0 )
	  az = acos(cos_az);
    else {
	az = cos_az > 1.0 ? 0 : PI;
	printf( "cos_az:%f\n", cos_az );
    }

    if(ra_rotation_angle >= PI && ra_rotation_angle < TWOPI ) {
	az = TWOPI -az;
    }
    az -= rdrift;

    // compute x,y,z 
    ra_x = sin(az)*cos_el;
    ra_y = cos(az)*cos_el;
    ra_z = sin_el;

    if( ra_elevation >= 0 ) {
	ra_z = fabs((double)ra_z);
    }
    else {
	ra_z = -fabs((double)ra_z);
    }
    if(az >= 0 && az <= PIOVR2) {
	ra_x = fabs((double)ra_x);
	ra_y = fabs((double)ra_y);
    }
    else if( az > PIOVR2 && az <= PI ) {
	ra_x = fabs((double)ra_x);
	ra_y = -fabs((double)ra_y);
    }
    else if( az > PI && az <= PI+PIOVR2 ) {
	ra_x = -fabs((double)ra_x);
	ra_y = -fabs((double)ra_y);
    }
    else if( az > PI+PIOVR2 && az < TWOPI ) {
	ra_x = -fabs((double)ra_x);
	ra_y = fabs((double)ra_y);
    }
    ra_azimuth = fmod(az+rtrack, TWOPI);
    d = ra_x*ra_x + ra_z*ra_z;
    bigR = sqrt(d);
    ra_tilt = atan2((double)ra_y, bigR);
    ra_rotation_angle = atan2((double)ra_x, (double)ra_z);

    return;
# endif
}
*/

// redo ...

void dd_radar_angles( 
	float asib_roll,
	float asib_pitch,
	float asib_heading,
	float asib_drift_angle,
	float asib_rotation_angle,
	float asib_tilt,


	float cfac_pitch_corr,
	float cfac_heading_corr,
	float cfac_drift_corr,
	float cfac_roll_corr,
	float cfac_elevation_corr,
	float cfac_azimuth_corr,
	float cfac_rot_angle_corr,
	float cfac_tilt_corr,
	int radar_type,  // from dgi->dds->radd->radar_type
	bool use_Wen_Chaus_algorithm,
    float dgi_dds_ryib_azimuth,
    float dgi_dds_ryib_elevation,
	float *ra_x,
	float *ra_y,
	float *ra_z,
	float *ra_rotation_angle,
	float *ra_tilt,
	float *ra_azimuth,
	float *ra_elevation,
	float *ra_psi
)




//	asib, cfac, ra, dgi )
//  struct platform_i *asib;
//  struct correction_d *cfac;
//  struct radar_angles *ra;
//  struct dd_general_info *dgi;
{
    /* compute the true azimuth, elevation, etc. from platform
     * parameters using Testud's equations with their different
     * definitions of rotation angle, etc.
     */
    double R, H, P, D, T, theta_a, tau_a, theta_t, tau_t, lambda_t, phi_t;
    double sinP, cosP, sinT, cosT, sinD, cosD, sinR, cosR;
    double sin_theta_rc, cos_theta_rc, sin_tau_a, cos_tau_a;
    double xsubt, ysubt, zsubt, lambda, phi;

    double d, bigR, az;
    double sin_el, cos_az, cos_el, cos_psi, cos_phi;
    double sin_pitch, cos_pitch, cos_drift, sin_tilt, cos_tilt;
    double rtilt, rtrack, rphi;
    double rpitch=RADIANS(asib_pitch +cfac_pitch_corr);
    double rhead=RADIANS(asib_heading +cfac_heading_corr);
    double rdrift=RADIANS(asib_drift_angle +cfac_drift_corr);
    double azr, azd;
    double lambda_a, phi_a;
    double sin_lambda_a, cos_lambda_a, sin_phi_a, cos_phi_a;


//# ifdef WEN_CHOWS_ALGORITHM
  if (use_Wen_Chaus_algorithm) {
    /*
     * see Wen-Chau Lee's paper
     * "Mapping of the Airborne Doppler Radar Data"
     */
    d = asib_roll;
    R = dd_isnanf(d) ? 0 : RADIANS(d +cfac_roll_corr);

    d = asib_pitch;
    P = dd_isnanf(d) ? 0 : RADIANS(d +cfac_pitch_corr);

    d = asib_heading;
    H = dd_isnanf(d) ? 0 : RADIANS(d +cfac_heading_corr);

    d = asib_drift_angle;
    D = dd_isnanf(d) ? 0 : RADIANS(d +cfac_drift_corr);

    sinP = sin(P);
    cosP = cos(P);
    sinD = sin(D);
    cosD = cos(D);
    
    T = H + D;

    if( radar_type == AIR_LF || radar_type == AIR_NOSE ) {

	d = ( dgi_dds_ryib_azimuth );
	lambda_a = dd_isnanf(d) ? 0 : RADIANS
	  ( CART_ANGLE( d +cfac_azimuth_corr ));
	sin_lambda_a = sin(lambda_a);
	cos_lambda_a = cos(lambda_a);

	d = ( dgi_dds_ryib_elevation );
	phi_a = dd_isnanf(d) ? 0 : RADIANS( d +cfac_elevation_corr );
	sin_phi_a = sin(phi_a);
	cos_phi_a = cos(phi_a);

	sinR = sin(R);
	cosR = cos(R);

	*ra_x = xsubt = cos_lambda_a * cos_phi_a *
	    (cosD * cosR - sinD * sinP * sinR) -
	    sinD * cosP * sin_lambda_a * cos_phi_a +
	    sin_phi_a * ( cosD * sinR + sinD * sinP * cosR );

	*ra_y = ysubt = cos_lambda_a * cos_phi_a *
	    ( sinD * cosR + cosD * sinP * sinR ) +
	    cosD * cosP * sin_lambda_a * cos_phi_a +
	    sin_phi_a * ( sinD * sinR - cosD * sinP * cosR );

	*ra_z = zsubt = -cosP * sinR * cos_lambda_a * cos_phi_a +
	    sinP * sin_lambda_a * cos_phi_a +
	    cosP * cosR * sin_phi_a;

	*ra_rotation_angle = theta_t = atan2(xsubt, zsubt);
	*ra_tilt = tau_t = asin(ysubt);
	lambda_t = atan2(xsubt, ysubt);
	*ra_azimuth = fmod(lambda_t + T, TWOPI);
	*ra_elevation = phi_t = asin(zsubt);

	return;
    }

    sinT = sin(T);
    cosT = cos(T);

    d = asib_rotation_angle;
    theta_a = dd_isnanf(d) ? 0 : RADIANS(d +cfac_rot_angle_corr);
    
    d = asib_tilt;
    tau_a = dd_isnanf(d) ? 0 : RADIANS(d +cfac_tilt_corr);
    sin_tau_a = sin(tau_a);
    cos_tau_a = cos(tau_a);
    sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
    cos_theta_rc = cos(theta_a + R); /* roll corrected rotation angle */

    *ra_x = xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
	     + cosD * sin_theta_rc * cos_tau_a
	     -sinD * cosP * sin_tau_a);
    *ra_y = ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
	     + sinD * sin_theta_rc * cos_tau_a
	     + cosP * cosD * sin_tau_a);
    *ra_z = zsubt = (cosP * cos_tau_a * cos_theta_rc
	     + sinP * sin_tau_a);

    *ra_rotation_angle = theta_t = atan2(xsubt, zsubt);
    *ra_tilt = tau_t = asin(ysubt);
    lambda_t = atan2(xsubt, ysubt);
    *ra_azimuth = fmod(lambda_t + T, TWOPI);
    *ra_elevation = phi_t = asin(zsubt);
    return;
  } else {
//# else

    rtrack = rhead +rdrift;
    *ra_rotation_angle = RADIANS(asib_rotation_angle +cfac_rot_angle_corr
			+asib_roll +cfac_roll_corr);
    *ra_tilt = RADIANS(asib_tilt +cfac_tilt_corr);
    azr = rhead +PI -(PIOVR2+ *ra_tilt)*sin(*ra_rotation_angle);
    
    rphi = (*ra_rotation_angle + PI);
    sin_pitch = sin(rpitch); cos_pitch = cos(rpitch);
    sin_tilt = sin(*ra_tilt); cos_tilt = cos(*ra_tilt);
    cos_drift = cos(rdrift);
    cos_phi = cos(rphi);
    
    cos_psi = cos_pitch*cos_drift*sin_tilt
	  + sin_pitch*cos_drift*cos_tilt*cos_phi
		- sin(rdrift)*cos_tilt*sin(rphi);
    /* psi is the angle between the beam and the
     * horizontal component of the aircraft velocity vector
     */
    *ra_psi = acos(cos_psi);

    sin_el = sin_tilt*sin_pitch-cos_tilt*cos_pitch*cos_phi;
    *ra_elevation = asin(sin_el);
    cos_el = cos(*ra_elevation);
    cos_az = (sin_tilt-sin_pitch*sin_el)/(cos_pitch*cos_el);

    if(fabs(cos_az) <= 1.0 )
	  az = acos(cos_az);
    else {
	az = cos_az > 1.0 ? 0 : PI;
	printf( "cos_az:%f\n", cos_az );
    }

    if(*ra_rotation_angle >= PI && *ra_rotation_angle < TWOPI ) {
	az = TWOPI -az;
    }
    az -= rdrift;

    /* compute x,y,z */
    *ra_x = sin(az)*cos_el;
    *ra_y = cos(az)*cos_el;
    *ra_z = sin_el;

    if( *ra_elevation >= 0 ) {
	*ra_z = fabs((double) *ra_z);
    }
    else {
	*ra_z = -fabs((double) *ra_z);
    }
    if(az >= 0 && az <= PIOVR2) {
	*ra_x = fabs((double) *ra_x);
	*ra_y = fabs((double) *ra_y);
    }
    else if( az > PIOVR2 && az <= PI ) {
	*ra_x = fabs((double) *ra_x);
	*ra_y = -fabs((double) *ra_y);
    }
    else if( az > PI && az <= PI+PIOVR2 ) {
	*ra_x = -fabs((double) *ra_x);
	*ra_y = -fabs((double) *ra_y);
    }
    else if( az > PI+PIOVR2 && az < TWOPI ) {
	*ra_x = -fabs((double) *ra_x);
	*ra_y = fabs((double) *ra_y);
    }
    *ra_azimuth = fmod(az+rtrack, TWOPI);
    d = *ra_x* *ra_x + *ra_z * *ra_z;
    bigR = sqrt(d);
    *ra_tilt = atan2((double) *ra_y, bigR);
    *ra_rotation_angle = atan2((double) *ra_x, (double) *ra_z);

    return;
  }
//# endif
}

