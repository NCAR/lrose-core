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
//
// Get temperature profile from sounding
//
////////////////////////////////////////////////////////////////

#ifndef TempProfile_H
#define TempProfile_H

#include "Params.hh"
#include <string>
#include <radar/NcarParticleId.hh>

using namespace std;

class TempProfile {
  
public:

  // constructor
  
  TempProfile (const string progName,
               const Params &params);
  
  // destructor
  
  ~TempProfile();

  // get a valid temperature profile
  // returns 0 on success, -1 on failure

  int getTempProfile(time_t dataTime,
                     time_t &soundingTime,
                     vector<NcarParticleId::TmpPoint> &tmpProfile);

protected:
private:

  string _progName;
  Params _params;

  vector<NcarParticleId::TmpPoint> _tmpProfile;
  time_t _soundingTime;
  
  int _getTempProfile(time_t searchTime, int marginSecs);
  int _checkTempProfile();

};


#endif
