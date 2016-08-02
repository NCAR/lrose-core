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
// SweepFile.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2003
//
///////////////////////////////////////////////////////////////

#ifndef SweepFile_H
#define SweepFile_H

#include <string>
#include <vector>
#include "Params.hh"
#include <netcdf.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxRadar.hh>
#include <toolsa/DateTime.hh>
using namespace std;

////////////////////////
// This class

class SweepFile {
  
public:

  // constructor

  SweepFile (const Params &params,
	     const DsMdvx &mdvx,
	     int n_tilts,
	     int tilt_num,
	     double elev,
	     int vol_num,
	     int n_az,
	     int n_gates,
	     const DsRadarParams &radar_params);

  // destructor
  
  ~SweepFile();

  // write the file

  int write();

protected:
  
private:

  const Params &_params;
  const NcFile _ncfIn;
  const DsMdvx &_mdvx;
  const DsRadarParams &_radarParams;

  string _outPath;
  string _tmpPath;

  int _nTilts;
  int _tiltNum;
  double _elev;
  int _volNum;
  int _nAz;
  int _nGates;
  int _nFields;

  time_t _sweepStartTime;
  time_t _volStartTime;
  int _sweepDuration;
  double _beamDuration;

  int _writeTmp();

};

#endif

