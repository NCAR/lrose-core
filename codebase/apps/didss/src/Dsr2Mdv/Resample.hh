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
// Resample.hh
//
// Resample class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#ifndef Resample_HH
#define Resample_HH

#include "Params.hh"
#include "Lookup.hh"
#include "MdvOutput.hh"
#include <rapformats/DsRadarMsg.hh>
using namespace std;

#define MISSING_DATA_VAL 0

class Resample {
  
public:
  
  // constructor
  
  Resample(char *prog_name,
           Dsr2Mdv_tdrp_struct *params,
	   Lookup *lookup);
  
  // destructor
  
  virtual ~Resample();

  int prepareVol(DsRadarMsg &radarMsg);

  int processBeam(DsRadarMsg &radarMsg);

  int writeCompleteVol();

  int writeIntermediateVol();

  int OK;

protected:
  
private:

  char *_progName;
  Dsr2Mdv_tdrp_struct *_params;

  Lookup *_lookup;
  MdvOutput *_output;

  int _volNum;
  int _nBeams;
  int _printCount;

  int _nX, _nY, _nZ;

  int _dbzFieldPos;
  int _timeFieldPos;
  int _nFieldsIn;
  int _nFieldsOut;

  int _nElev;
  int _nAz;
  int _nGates;
  int _noiseGateStart;
  int _nBytesBeam;

  int _nPlanes;
  int _nPointsPlane;
  int _nPointsField;
  int _nPointsVol;
  int _nBeamsArray;
  int _nBeamsTarget;
  int _maxNpointsBeam;

  time_t _volStartTime;
  time_t _volEndTime;
  time_t _latestDataTime;
  time_t _volReferenceTime;
  int _volDuration;

  int _azNum;
  int _elevNum;
  int _prevAzNum;
  int _prevElevNum;
  time_t _beamReferenceTime;

  // local copies of params for speed

  int _minValidRun;
  int _removeClutter;
  int _checkSn;
  int _useRepeatedElevations;
  double _azimuthOffset;
  double _timeScale;

  // arrays

  int *_fieldPos;

  ui08 **_volGrid;
  ui08 **_accumGrid;

  ui08 *_thisBeamData;
  ui08 *_prevBeamData;
  ui08 *_interpBeamData;

  ui08 **_beamCheck;
  ui08 **_rData;
  int *_dbzThresholdLevel;

  void _interpolateBeam(ui08 *field_data,
			ui08 *prev_field_data,
			ui08 *interp_field_data,
			int nbytes_interp);

  void _transformBeam(int az_num, int elev_num,
		      ui08 *field_data);
    
};

#endif
