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
/*
 *   Module: FourDD.hh
 *
 *   Author: Sue Dettling(just packaging Curtis James' code 
 *           with a few mods.)
 *
 *   Date:   11/13/2001
 * 
 *   UW Radial Velocity Unfolding Algorithm
 *   Four-Dimensional Dealiasing (4DD)
 *
 *   DESCRIPTION:
 *     This algorithm unfolds a volume of single Doppler radial velocity data.
 *   The algorithm uses a previously unfolded volume (or VAD if previous volume
 *   is unavailable) and a higher elevation sweep to unfold some of the bins
 *   in each sweep. Then, it completes the unfolding, assuming spatial
 *   continuity around each bin.
 *
 *   DEVELOPER:
 *     Curtis N. James            25 Jan 99
 * 
 */


#ifndef FOURDD_HH
#define FOURDD_HH

//
// Radius of earth in kilometers. 
// 
#define A 6372.0 

#define PI 3.1415927 

#include <cstdlib>
#include <math.h>
#include <Spdb/SoundingGet.hh>
#include <trmm_rsl/rsl.h>
#include <toolsa/DateTime.hh>
#include "Params.hh"
using namespace std;

class FourDD 
{
public:
  
  FourDD(Params &parameters);

  ~FourDD();
  
  int Dealias(Volume *lastVelVol, Volume *currVelVol, Volume *currDbzVol,
	      time_t volTime);
 
  void firstGuess(Volume* soundVolume, float missingVal,short unsigned int *,time_t volTime);

  void unfoldVolume(Volume* rvVolume, Volume* soundVolume, Volume* lastVolume,
     float missingVal, unsigned short rm, unsigned short* success);

  float window(Volume* rvVolume, int sweepIndex, int startray, int endray,
     int firstbin, int lastbin, float* std, float missingVal, unsigned 
     short* success);

  void prepVolume (Volume* DBZVolume, Volume* rvVolume, float missingVal);

  int findRay (Volume* rvVolume1, Volume* rvVolume2, int sweepIndex1, int 
	sweepIndex2, int currIndex1, float missingVal);

  float previousVal (Volume* rvVolume, Volume* lastVolume, int sweepIndex, int
	currIndex, int rangeIndex, float missingVal);

  int loadSoundingData( time_t issueTime );

private:

  Params params;

  SoundingGet       sounding;

};

#endif // FOURDD_HH










