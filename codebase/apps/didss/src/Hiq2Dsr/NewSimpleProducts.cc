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
/*********************************************************************
 * NewSimpleProducts: Product handler that handles the new simple
 *                    HiQ products.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>

#include "NewSimpleProducts.hh"
using namespace std;


NewSimpleProducts::NewSimpleProducts(const bool debug) :
  Products(debug)
{
}

NewSimpleProducts::~NewSimpleProducts() 
{
}


/*********************************************************************
 * fillProducts() - Fill the products arrays based on the information
 *                  in the current HiQ messages.  The calling method
 *                  must have allocated enough space for each of the
 *                  product buffers.
 *
 * Returns true on success, false on failure.
 */

bool NewSimpleProducts::fillProducts(const double radar_constant,
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

  double velconst = ProductConstants::C /
    (2.0 * frequency * 2.0 * fabs(prt) * 32768.0);
  double rconst = radar_constant - 20.0 *
    log10(xmit_pulsewidth / rcvr_pulse_width)
//    + 10.0 * log10(peak_power / horiz_xmit_power);
    - peak_power + horiz_xmit_power;
  double pcorrect = data_sys_sat - 
    20.0 * log10((double)ProductConstants::STANDARD_OFFSET) - receiver_gain;
  double widthconst = (ProductConstants::C / frequency) /
    prt / (2.0 * sqrt(2.0) * M_PI);

//  cerr << "rconst = " << rconst << endl;
//  cerr << "radar_constant = " << radar_constant << endl;
//  cerr << "xmit_pulsewidth = " << xmit_pulsewidth << endl;
//  cerr << "rcvr_pulse_width = " << rcvr_pulse_width << endl;
//  cerr << "peak_power = " << peak_power << endl;
//  cerr << "horiz_xmit_power = " << horiz_xmit_power << endl;
//  cerr << "log10(xmit_pulsewidth / rcvr_pulse_width) = " << log10(xmit_pulsewidth / rcvr_pulse_width) << endl;
//  cerr << "log10(peak_power / horiz_xmit_power) = " << log10(peak_power / horiz_xmit_power) << endl;

  // Calculate the noise power

  double local_noise_power = 0.0;
  
  if (noise_power <= -10.0)
    local_noise_power = exp((noise_power - pcorrect) /
			    ProductConstants::SCALE2DB);
  
  // Now calculate the products

  double range = 0.0;
  si16 *aptr = abp;
  
  for (int i = 0; i < num_gates; ++i)
  {
    double cp = *aptr++;    // 0.004 dB / bit
    double v = *aptr++;     // nyquist = 65536 = +/- 32768
    double p = *aptr++;     // 0.004 dB / bit
    double pn = p;

//    cerr << "Beam data:  p = " << p << " (" << (p * 0.004) << " dB), cp = "
//	 << cp << " (" << (cp * 0.004) << " dB), v = " << v << endl;
    
    // Get a linear version of power

    double linear = exp(p * ProductConstants::SCALE2LN);
    
    // Noise correction

    if (noise_power <= -10.0)
    {
      linear -= local_noise_power;     // subtract noise
      if (linear < 0.0)
	linear = ProductConstants::SMALL;
      pn = log(linear) / ProductConstants::SCALE2LN;
    }
    
    double dbm = 0.004 * pn + pcorrect;

//    cerr << "dbm = " << dbm << endl;
//    cerr << "rconst = " << rconst << endl;
//    cerr << "range = " << range << endl;
//    cerr << "radar_constant = " << radar_constant << endl;

    if (i > 0)
      range = 20.0 * log10(i * 0.0005 * ProductConstants::C *
			   rcvr_pulse_width);

    // Compute floating point, scaled, scientific products

    velocity_buffer[i] = velconst * v;
    spectrum_width_buffer[i] =
      sqrt(ProductConstants::SCALE2LN * fabs(p - cp)) * widthconst;
    reflectivity_buffer[i] = dbm + rconst + range;
    coherent_reflectivity_buffer[i] = 0.004 * cp + pcorrect + rconst + range;
    ncp_buffer[i] = exp(ProductConstants::SCALE2LN * (cp - p));
    power_buffer[i] = dbm;
    
//    cerr << "     refl = " << reflectivity_buffer[i]
//	 << ", coh refl = " << coherent_reflectivity_buffer[i]
//         << ", vel = " << velocity_buffer[i]
//	 << ", ncp = " << ncp_buffer[i]
//	 << ", power = " << power_buffer[i] << endl;
  } /* endfor - i */

  return true;
}

