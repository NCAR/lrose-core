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
//   $Date: 2016/03/06 23:53:41 $
//   $Id: SimpleProducts.cc,v 1.2 2016/03/06 23:53:41 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SimpleProducts: Product handler that handles the simple HiQ products.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>

#include "SimpleProducts.hh"
using namespace std;


SimpleProducts::SimpleProducts(const bool debug) :
  Products(debug)
{
}

SimpleProducts::~SimpleProducts() 
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

bool SimpleProducts::fillProducts(const HiqRadarMsg &radar_msg,
				  const HiqBeamMsg &beam_msg,
				  float *reflectivity_buffer,
				  float *coherent_reflectivity_buffer,
				  float *velocity_buffer,
				  float *spectrum_width_buffer,
				  float *ncp_buffer,
				  float *power_buffer)
{
  // Calculate some needed values

  double rconst = radar_msg.getRadarConstant() - 20.0 *
    log10(radar_msg.getXmitPulsewidth() / beam_msg.getRcvrPulseWidth());
  double noise = (radar_msg.getNoisePower() > 10.0) ? 0.0 : 0.0;
  double velconst = ProductConstants::C /
    (2.0 * radar_msg.getFrequency() * 2.0 * M_PI * beam_msg.getPrt());
  double pcorrect = radar_msg.getDataSysSat() -
    20.0 * log10(0x1000000 * beam_msg.getRcvrPulseWidth() / 1.25e-7) -
    10.0 * log10((double)beam_msg.getHits()) - radar_msg.getReceiverGain();
  double widthconst = (ProductConstants::C / radar_msg.getFrequency()) /
    beam_msg.getPrt() / (2.0 * sqrt(2.0) * M_PI);
  
  // Now calculate the products

  si16 *aptr = beam_msg.getAbp();
  double range = 0.0;

  for (int i = 0; i < beam_msg.getNumGates(); ++i)
  {
    double a = *aptr++;
    double b = *aptr++;
    double p = *aptr++;

    double dbm = 10.0 * log10(fabs(p)) + pcorrect;
    double r12 = a * a + b * b;
    double cp = sqrt(r12);

    double width = log(fabs((p - noise) / cp));
    if (width < 0.0)
      width = 0.0001;

    if (i)
      range = 20.0 * log10(i * 0.0005 * ProductConstants::C *
			   beam_msg.getRcvrPulseWidth());

    // Compute floating point, scaled , scientific products

    velocity_buffer[i] = velconst * atan2(b, a);
    spectrum_width_buffer[i] = sqrt(width) * widthconst;
    reflectivity_buffer[i] = dbm + rconst + range;
    coherent_reflectivity_buffer[i] =
      10.0 * log10(fabs(cp)) + pcorrect + rconst + range;
    ncp_buffer[i] = cp / p;
    power_buffer[i] = p;
    
  } /* endfor - i */

  return true;
}

