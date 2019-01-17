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
 *   Description: Class Dsr2Radar contains methods to create an RSL Radar 
 *                struct from DsRadarBeams, and to write a volume of 
 *                input(and saved) DsRadarMsgs to an fmq with the 
 *                field data of the radar beams replaced by the 
 *                (possibly dealiased) field data contained in the 
 *                RSL Radar struct. 
 */

#ifndef DSR2RADAR_HH
#define DSR2RADAR_HH

#include <cstdlib>
#include <utility>
#include <vector>
#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/DsBeamDataFieldParms.hh>
#include <rapformats/DsBeamData.hh>
#include <Fmq/DsRadarQueue.hh>
#include "Rsl.hh"
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
  Dsr2Radar(Params &parameters);

  //
  // destructor
  //
  ~Dsr2Radar();

  //
  // reformat(): Takes DsRadarMsgs and fills the various
  //             RSL (TRMM Radar Software Library) structs 
  //             Rays, Sweeps, Volumes, and Radar with
  //             DsRadarMsg data
  //
  int reformat( DsRadarMsg &radarMsg, int &contents);

  //
  // clearData(): Delete dynamically allocated memory, 
  //              re-initialize data members
  //
  void clearData();

  //
  // writeVol(): Write DsRadarMsgs to fmq and replace original beam data
  //             with data from RSL rays.
  //
  void writeVol(DsRadarQueue &outputQueue);

  //
  // getVelVolume(): return pointer to RSL Volume struct holding 
  //                 velocity data. 
  //
  Volume* getVelVolume();

  //
  // getVelVolume(): return pointer to RSL Volume struct holding
  //                 reflectivity data.
  //
  Volume* getDbzVolume();
  
  //
  // getVolTime(): return the volume time (time of last beam in volume). 
  //
  time_t getVolTime();

  bool isEmpty() {return radarIsEmpty;}

private:

  //
  // tdrp parameters
  //
  Params params;

  //
  // containers for RSL structs 
  //
  vector <Ray *>    *fieldRays;
  vector <Sweep *>  *fieldSweeps;
  vector <Volume *> volumes;

  //
  // RSL struct which holds an entire volume of radar data
  //
  Radar             *radar;

  //
  // Number of bytes per word in the RSL 'range' array
  // 
  int rsl_bytewidth;

  //
  // Container for original DsRadarMsg and its integral contents
  // indicator.
  //
  vector < RADARMSG_CONTENTS* > radarMsgs;

  //
  // Objects, data extracted from DsRadarMsg
  //
  vector <DsFieldParams*>  fieldParams;
  DsRadarParams            radarParams;
  DsRadarFlags             radarFlags;
  DsRadarBeam              radarBeam;
  int numFields;

  //
  // Objects needed for DsBeamData
  vector <DsBeamDataFieldParms> beamParams;


  //
  // DsRadarParams members
  //
  double   altitude;
  float    horizBeamWidth;
  double   latitude;
  double   longitude;
  int      numGates;

  //
  // DsRadarBeam members
  //
  DateTime beamDateTime;
  float    targetElev;
  
  //
  // DsRadarFlags (note that tiltNum is also a DsRadarBeam member)
  //
  bool end_of_vol;
  int tiltNum;

  //
  // Other flags
  //
  bool lastSweepIsDeleted;
  bool mustInit;
  bool paramsInitialized;
  bool gotFlags;
  bool radarIsEmpty;
  bool haveRequiredFields;

  //
  // Bookkeepping members
  //
  float lastTiltTargElev;
  int lastTiltNum;
  int nbeamsLastTilt;
  int nbeamsTilt;
  int nbeamsTotal;
  int nMsgsLastTilt;
  int nMsgsTilt;
  int nSweepsVol;

  //
  // indices for locating velocity and reflectivity volumes in 
  // vector volumes.
  // 
  int radialVelIndex;
  int dbzIndex;

  //
  // private methods:
  //

  //
  // beam2rays(): Create RSL Rays and Ray_headers from radarBeam
  //              (one ray per field)
  //
  void beam2rays();

  //
  // fillSweeps(): For each field, fill Sweep_headers and set pointers 
  //               to rays.
  //
  void fillSweeps();
  
  //
  // fillVolumes(): For each field, fill Volume_headers and  set pointers
  //                to sweeps of each volume.
  //
  void fillVolumes();

  //
  // fillRadar(): fill radar header, set pointers to Volumes
  //
  void fillRadar();

  //
  // initParams():  If this is first message containing params and beam data:
  //         Initialize data members which depend on DsRadarParams,
  //         DsFieldParams for necessary fields (as specified in the 
  //         tdrp parameter required_fields), and DsRadarBeam. 
  //         Set the indices which indicate 
  //         the order in which we find the the velocity and reflectivity fields
  //         in the DsRadarBeam, allocate memory to hold Rays and Sweeps.
  //         Else, return without setting flag.
  //
  void initParams(DsRadarMsg &radarMsg, int &contents);

  //
  // updateParams(): Update params if present in DsRadarMsg
  //
  void updateParams(DsRadarMsg &radarMsg, int &contents);

  //
  // updateRadarFlags(): Update params if present in DsRadarMsg
  //
  void updateRadarFlags(DsRadarMsg &radarMsg, int &contents);

  //
  // deleteLastSweep(): delete last sweep created. Delete corresponding 
  //                    DsRadarMsg-contents pairs. Erase spots in appropriate
  //                    vectors which held pointers to the sweep, the rays.
  //                    and the DsRadarMsg-contents pair.
  //
  //
  void deleteLastSweep();

  void _beam2ray_0(Ray &ray, DsFieldParams &j);
  void _beam2ray_data(Ray &ray, DsFieldParams &j, DsBeamData &beam, int i);
};


#endif /* DSR2RADAR_HH */












