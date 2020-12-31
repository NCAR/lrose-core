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
 * @file TargetInfo.hh
 *
 * @class TargetInfo
 *
 * Class representing a target data point.
 *  
 * @date 3/31/2010
 *
 */

#ifndef TargetInfo_H
#define TargetInfo_H
#include <Refract/IQ.hh>

/** 
 * @class TargetInfo
 */

class TargetInfo
{
  
public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Strength (SNR-based) of target.
   */
  
  float strength;

  /**
   * @brief Scan-to-scan difference.
   */

  float phase_diff;

  /**
   * @brief Expected error in the scan-to-scan difference (phase_diff).
   */

  float phase_diff_er;

  /**
   * @brief scan-to-scan difference (phase_diff).
   */
  IQ dif_iq;

  /**
   * @brief Scan-to-reference difference.
   */

  float phase;

  /**
   * @brief Expected error in the scan-to-reference difference (phase).
   */

  float phase_er;

  /**
   * @brief Scan-to-reference difference (phase) corrected for dN/dz,
   *        sub-range.
   */
  float phase_cor;

  /**
   * @brief IQ phase_cor.
   */
  IQ iq;


  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  TargetInfo();
  
  /**
   * @brief Destructor.
   */

  virtual ~TargetInfo();

  void compute_phase_diff(const IQ &difPrevScan, double norm,
			  const IQ &difFromRef);
  void setStrength(float snr, bool snrIsBad);
};

#endif
