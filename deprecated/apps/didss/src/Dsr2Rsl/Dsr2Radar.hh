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
 *   Module: Dsr2Radar.hh
 *
 *   Author: Sue Dettling
 *
 *   Date:   10/5/01
 *
 *   Description: Class Dsr2Radar creates a RSL Radar struct from
 *                DsrRadarBeams
 */

#ifndef DSR2RADAR_HH
#define DSR2RADAR_HH

#include <cstdlib>
#include <utility>
#include <vector>
#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsRadarMsg.hh>
#include <Fmq/DsRadarQueue.hh>
#include <trmm_rsl/rsl.h>
#include <toolsa/DateTime.hh>
#include "Params.hh" 
#include "Dsr2Radar.hh" 
using namespace std;
		
typedef pair< DsRadarMsg*, int > RADARMSG_CONTENTS;

   
class Dsr2Radar {

public:

  //
  // constructor
  //
  Dsr2Radar(string prog_name, Params &parameters);
  
  ~Dsr2Radar();

  int reformat( DsRadarMsg &radarMsg, int &contents);

  void clearData();

  void writeVol(DsRadarQueue &outputQueue);

  Volume* getVelVolume();

  Volume* getDbzVolume();
  
  time_t getVolTime();

private:

  string progName;
  Params params;

  //
  // containers for RSL structs 
  //
  vector <Ray*> *fieldRays;
  vector <Sweep *> *fieldSweeps;
  vector <Volume *> volumes;
  Radar *radar;

  //
  // Containers for original DsRadarMsg and its integral contents
  // indicator.
  //
  vector < RADARMSG_CONTENTS* > radarMsgs;

  //
  // Params, flags, and data extracted from DsRadarMsg
  //
  vector <DsFieldParams*>  fieldParams;
  DsRadarParams radarParams;
  DsRadarFlags radarFlags;
  DsRadarBeam radarBeam;
  int numFields;
  int numGates;
  int dataLen;
  DateTime beamDateTime;
  float unambigRange;
  float elevation;
  float gateSpacing;
  float pulseRepFreq;
  float targetElev;
  double latitude;
  double longitude;
  double altitude;
  int samplesPerBeam;
  float horizBeamWidth;
  float wavelength;
  float unambigVelocity;
  float startRange;
  int tiltNum;
  float azimuth;
  bool firstMsg;

  int nbeamsTilt;
  int nbeamsTotal;
  int nSweepsVol;
  

  int nbeamsLastTilt;
  int nMsgsLastTilt;
  int nMsgsTilt;
  float lastTiltTargElev;
  int lastTiltNum;

  bool end_of_vol;
  
  //
  // private methods
  //
  void beam2rays();
  void fillSweeps();
  void fillVolumes();
  void fillRadar();
  void init(DsRadarMsg &radarMsg, int &contents);
  void updateParamsFlags(DsRadarMsg &radarMsg, int &contents);
  void deleteLastSweep();

  int radialVelIndex;
  int dbzIndex;

  bool isDeleted;
};


#endif /* DSR2RADAR_HH */












