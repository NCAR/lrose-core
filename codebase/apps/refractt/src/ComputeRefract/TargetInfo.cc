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
/**
 *
 * @file TargetInfo.cc
 *
 * @class TargetInfo
 *
 * Class representing a target data point.
 *  
 * @date 3/31/2010
 *
 */

#include "TargetInfo.hh"
#include <Refract/RefractConstants.hh>

/*********************************************************************
 * Constructors
 */

TargetInfo::TargetInfo() :
  strength(0.0),
  phase_diff(0.0),
  phase_diff_er(0.0),
  dif_iq(0.0, 0.0),
  phase(0.0),
  phase_er(0.0),
  phase_cor(0.0),
  iq(0.0, 0.0)
{
}

/*********************************************************************
 * Destructor
 */

TargetInfo::~TargetInfo()
{
}

//----------------------------------------------------------------------
void TargetInfo::compute_phase_diff(const IQ &difPrevScan, double norm,
				    const IQ &difFromRef)
{
  if (norm != 0.0)
  {
    phase_diff = difPrevScan.phase();
    phase_diff_er = sqrt(-2.0*log(norm)/norm)/DEG_TO_RAD;
    dif_iq = difPrevScan;
  }
  else
  {
    phase_diff = refract::INVALID;
    phase_diff_er = refract::INVALID;
    dif_iq.set(0.0, 0.0);
  }
  if (difFromRef.hasZero())
  {
    phase = refract::INVALID;
    phase_cor = refract::INVALID;
  }
  else
  {
    phase = difFromRef.phase();
    phase_cor = phase;
  }
}

//----------------------------------------------------------------------
void TargetInfo::setStrength(float snr, bool isBad)
{
  if (isBad)
  {
    strength = refract::INVALID;
  }
  else
  {
    strength = snr;
  }
}

/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
