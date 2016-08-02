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
/////////////////////////////////////////////////////////////
// TempProfile.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2011
//
///////////////////////////////////////////////////////////////

/**
 * @file TempProfile.hh
 * @brief Get temperature profile from sounding
 * @class TempProfile
 *
 * Get temperature profile from sounding
 *
 */

#ifndef TempProfile_H
#define TempProfile_H

#include "Params.hh"
#include <string>
#include <radar/NcarParticleId.hh>

using namespace std;

class TempProfile {
  
public:

  /**
   * Constructor
   * @param[in] progName The name of the aplication
   * @param[in] Params The application parameters
   */
  TempProfile (const string progName,
               const Params &params);
  
  /**
   * Destructor
   */
  ~TempProfile();

  /**
   * Get a valid temperature profile
   * @param[in] dataTime The time of the radar data
   * @param[out] soundingTime The time of the sounding data used
   *             to construct the temperature profile
   * @param[out] tmpProfile The vector of profile temperatures
   * @return 0 on success, -1 on failure
   */
  int getTempProfile(time_t dataTime,
                     time_t &soundingTime,
                     vector<NcarParticleId::TmpPoint> &tmpProfile);

protected:
private:

  string _progName;  /**< Name of the application (for debugging messages) */
  Params _params;    /**< Application parameters */

  vector<NcarParticleId::TmpPoint> _tmpProfile; /**< The vector of profile temperatures */
  time_t _soundingTime;                         /**< The time of the sounding data used to
                                                     construct the temperature profile*/
  
  /**
   * Get temp profile from first sounding before given time
   * @param[in] searchTime The desired time of the sounding data
   * @param[in] marginSecs Number of seconds to look back from searcgTime
   *            for a usable sounding, before giving up
   *
   * @return 0 on success, -1 on failure
   */
  int _getTempProfile(time_t searchTime, int marginSecs);

  /**
   * Check the temperature profile for quality
   * @return 0 on success, -1 on failure
   */
  int _checkTempProfile();

};


#endif
