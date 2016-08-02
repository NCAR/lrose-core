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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:40 $
//   $Id: DualPol1Products.cc,v 1.3 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DualPol1Products: Product handler that handles the dual polarization 1
 *                   HiQ products.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>

#include "DualPol1Products.hh"
using namespace std;


DualPol1Products::DualPol1Products(const bool debug) :
  Products(debug)
{
}

DualPol1Products::~DualPol1Products() 
{
}


/*********************************************************************
 * fillProducts() - Fill the products arrays based on the information
 *                   in the current HiQ messages.  The calling method
 *                   must have allocated enough space for each of the
 *                   product buffers.
 *
 * Returns true on success, false on failure.
 */

bool DualPol1Products::fillProducts(const double radar_constant,
				    const double xmit_pulsewidth,
				    const double rcvr_pulse_width,
				    const double peak_power,
				    const double noise_power,
				    const double vert_noise_power,
				    const double horiz_xmit_power,
				    const double frequency,
				    const double prt,
				    const double prt2,
				    const double data_sys_sat,
				    const double phase_offset,
				    const int hits,
				    const double receiver_gain,
				    const double vert_rcvr_gain,
				    const int num_gates,
				    si16 *abp,
				    float *reflectivity_buffer,
				    float *coherent_reflectivity_buffer,
				    float *velocity_buffer,
				    float *spectrum_width_buffer,
				    float *ncp_buffer,
				    float *power_buffer)
{
  // Calculate some needed values

  double rconst = radar_constant - 20.0 *
    log10(xmit_pulsewidth / rcvr_pulse_width);
  double noise = (noise_power > -10.0) ? 0.0 : 0.0;
  double velconst = ProductConstants::C /
    (2.0 * frequency * 2.0 * M_PI * fabs(prt));
  double pcorrect = data_sys_sat - 
    20.0 * log10(0x1000000 * rcvr_pulse_width / 1.25e-7) -
    10.0 * log10((double)hits) - receiver_gain;
  double widthconst = (ProductConstants::C / frequency) /
    prt / (2.0 * sqrt(2.0) * M_PI);

  // Now calculate the products

  si16 *aptr = abp;
  si16 *bptr = aptr + 3 * num_gates;
  double range = 0.0;

  for (int i = 0; i < num_gates; ++i)
  {
    double a = *aptr++;
    double b = *aptr++;
    double p = *aptr++;
    double a2 = *bptr++;
    double b2 = *bptr++;
    double p2 = *bptr++;

    double v1a = atan2(b, a);
    double v2a = atan2(b2, a2);

    double theta = v1a - v2a;
    if (theta > M_PI)
      theta -= 2.0 * M_PI;
    if (theta < -M_PI)
      theta += 2.0 * M_PI;

    double dp = theta * 0.5;
    if (dp > phase_offset + M_PI)
      dp -= M_PI;
    if (dp < phase_offset)
      dp += M_PI;

    double v = v1a - dp;
    if (v < -M_PI)
      v += 2.0 * M_PI;
    if (v > M_PI)
      v -= 2.0 * M_PI;

    double cp1 = sqrt(a * a + b * b);
    double cp2 = sqrt(a2 * a2 + b2 * b2);
    double cp = cp1 + cp2;

    double dbm = 10.0 * log10(fabs(p + p2)) + pcorrect;

    if (i > 0)
      range = 20.0 * log10(i * 0.0005 * ProductConstants::C *
			   rcvr_pulse_width);

    double width = log(fabs((p + p2 - 2.0 * noise) / cp));
    if (width < 0.0)
      width = 0.0001;

    // Compute floating point, scaled, scientific products

    velocity_buffer[i] = velconst * v;
    spectrum_width_buffer[i] = sqrt(width) * widthconst;
    reflectivity_buffer[i] = dbm + rconst + range;
    coherent_reflectivity_buffer[i] =
      10.0 * log10(fabs(cp)) + pcorrect + rconst + range;
    ncp_buffer[i] = cp / (p + p2);
    power_buffer[i] = p;
    
  } /* endfor - i */

  return true;
}

