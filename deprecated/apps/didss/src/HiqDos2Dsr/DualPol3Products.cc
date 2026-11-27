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
 * DualPol3Products: Product handler that handles the dual polarization 3
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

#include "DualPol3Products.hh"

using namespace std;

DualPol3Products::DualPol3Products(const bool debug) :
  Products(debug)
{
}

DualPol3Products::~DualPol3Products() 
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

bool DualPol3Products::fillProducts(const HiqRadarMsg &radar_msg,
				    const HiqBeamMsg &beam_msg,
				    float *reflectivity_buffer,
				    float *coherent_reflectivity_buffer,
				    float *velocity_buffer,
				    float *spectrum_width_buffer,
				    float *ncp_buffer,
				    float *power_buffer)
{
  // Calculate some needed values

  double h_channel_radar_constant =
    radar_msg.getRadarConstant() -
    20.0 * log10(radar_msg.getXmitPulsewidth() /
		 beam_msg.getRcvrPulseWidth()) +
    10.0 * log10(radar_msg.getPeakPower() / beam_msg.getHorizXmitPower());

  double angle_to_velocity_scale_factor =
    ProductConstants::C / (2.0 * radar_msg.getFrequency() *
			   2.0 * M_PI * beam_msg.getPrt());
  double horiz_offset_to_dBm =
    radar_msg.getDataSysSat() - 20.0 * log10((double)0x10000) -
    radar_msg.getReceiverGain() + 10.0 * log10(2.0);  // correct for half power measurement
  double vert_offset_to_dBm =
    radar_msg.getDataSysSat() - 20.0 * log10((double)0x10000) -
    radar_msg.getVertRcvrGain() + 10.0 * log10(2.0);  // correct for half power measurement

  double widthconst = (ProductConstants::C / radar_msg.getFrequency()) /
    beam_msg.getPrt() / (4.0 * sqrt(2.0) * M_PI);

  double ph_off = 20.0 * M_PI / 180.0;    // Set phase offset to be 20 deg

  // These powers reflect the LNA and waveguide performance.
  // They cannot be broken down into co and cross powers.

  double hchan_noise_power =
    (radar_msg.getNoisePower() > -10.0) ? 0.0 :
    exp((radar_msg.getNoisePower() - horiz_offset_to_dBm) /
	ProductConstants::SCALE2DB);
  double vchan_noise_power =
    (radar_msg.getVertNoisePower() > -10.0) ? 0.0 :
    exp((radar_msg.getVertNoisePower() - vert_offset_to_dBm) /
	ProductConstants::SCALE2DB);
  double coher_noise_power = exp((-129.0 - vert_offset_to_dBm) /
				 ProductConstants::SCALE2DB);

  // Now calculate the products

  si16 *aptr = beam_msg.getAbp();
  double range_correction = 0.0;

  for (int i = 0; i < beam_msg.getNumGates(); ++i)
  {
    double cp1 = *aptr++ * ProductConstants::SCALE2LN;   // natural log of |R(1)| from H to V pusle pair
    double v1a = *aptr++ * M_PI / 32768.0;  // radian angle of |R(1)| from H to V pulse pair
    double lnpv = *aptr++ * ProductConstants::SCALE2LN;  // natural log of V power (from 16 bit scaled log)
    double cp2 = *aptr++ * ProductConstants::SCALE2LN;   // natural log of |R(1)| from V to H pulse pair
    double v2a = *aptr++ * M_PI / 32768.0;  // radian angle of |R(1)| from V to H pulse pair
    double lnph = *aptr++ * ProductConstants::SCALE2LN;  // natural log of H power (from 16 bit scaled log)
    double lag2 = *aptr++ * ProductConstants::SCALE2LN;  // natural log of |R(2)| from H to H + |R(2)| from V to V
    double lnhv = *aptr++ * ProductConstants::SCALE2LN;  // natural log of V power on H xmit pulse

    double lncoherent =         // natural log of coherent power
      (cp1 + log(1 + exp(cp2 - cp1)) - ProductConstants::LOG2);

    double lncp = (lag2 - lnpv - log(1.0 + exp(lnph - lnpv)));

    // subtract raw noise power from the raw log powers
    double linear_h_power = exp(lnph) - hchan_noise_power;
    if (linear_h_power <= 0.0)
      linear_h_power = ProductConstants::SMALL;
    lnph = log(linear_h_power);      // corrected h chan power

    double linear_v_power = exp(lnpv) - vchan_noise_power;
    if (linear_v_power <= 0.0)
      linear_v_power = ProductConstants::SMALL;
    lnpv = log(linear_v_power);      // corrected v chan power
    
    double temp = exp(lnhv) - vchan_noise_power;
    lnhv = (temp < 0.0) ? ProductConstants::SMALL : log(temp);    // corrected cross power

    temp = exp(lncoherent) - coher_noise_power;
    lncoherent = (temp < 0.0) ? ProductConstants::SMALL : log(temp);  // corrected cross power

    // Convert the raw log powers to dBm at the test pulse
    // waveguide coupler

    double horiz_dBm_at_coupler =
      lnph * ProductConstants::SCALE2DB + horiz_offset_to_dBm;    // HH power in dBm
    double horiz_coherent_dBm_at_coupler =
      lncoherent * ProductConstants::SCALE2DB + horiz_offset_to_dBm;   // HH coherent power in dBm

    // Compute range correction in dB. Skip the first gate

    if (i > 0)
      range_correction = 20.0 * log10(i * 0.0005 * ProductConstants::C *
				      beam_msg.getRcvrPulseWidth());

    // Subtract out the system phase from v1a

    v1a -= radar_msg.getPhaseOffset();
    if (v1a < -M_PI)
      v1a += 2.0 * M_PI;
    else if (v1a > M_PI)
      v1a -= 2.0 * M_PI;

    // Add the system phase to v2a

    v2a += radar_msg.getPhaseOffset();
    if (v2a < -M_PI)
      v2a += 2.0 * M_PI;
    else if (v2a > M_PI)
      v2a -= 2.0 * M_PI;

    // Compute the total difference

    double theta = v2a - v1a;
    if (theta > M_PI)
      theta -= 2.0 * M_PI;
    else if (theta < -M_PI)
      theta += 2.0 * M_PI;

    // Figure the differential phase (from -20 to +160)
    
    double dp = theta * 0.5;
    if (dp < -ph_off)
      dp += M_PI;

    // Compute the velocity

    double v = v1a + dp;
    if (v < -M_PI)
      v += 2.0 * M_PI;
    else if (v > M_PI)
      v -= 2.0 * M_PI;

    // Compute floating point, scaled, scientific products

    // Compute the NCP value before the lncp value is updated
    // in the spectrum width calculation

    velocity_buffer[i] = v * angle_to_velocity_scale_factor;

    if (lncp > 0.0)
      spectrum_width_buffer[i] = 0.0;
    else
      spectrum_width_buffer[i] = sqrt(-lncp) * widthconst;

    reflectivity_buffer[i] =
      horiz_dBm_at_coupler + h_channel_radar_constant + range_correction;

    coherent_reflectivity_buffer[i] =
      horiz_coherent_dBm_at_coupler + h_channel_radar_constant +
      range_correction;

    ncp_buffer[i] = exp(lncp);

    power_buffer[i] = coher_noise_power;
    
  } /* endfor - i */

  return true;
}

