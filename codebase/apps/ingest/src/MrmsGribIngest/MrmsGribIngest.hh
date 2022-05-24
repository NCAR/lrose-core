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
// MrmsGribIngest reads individual height layer files for the MRMS
// radar mosaic, and compiles them into a single MDV file
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// April 2015
//
//////////////////////////////////////////////////////////////////////////

#ifndef _MRMS_GRIB_INGEST_HH
#define _MRMS_GRIB_INGEST_HH

#include <string>
#include <map>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "Params.hh"
#include "Args.hh"
using namespace std;

// Forward class declarations

class Grib2Mdv;
class DsInputPath;

class MrmsGribIngest {

public:
  
  // Constructor

  MrmsGribIngest(int argc, char **argv);
  
  // destructor
  
  ~MrmsGribIngest();
  
  // Flag indicating whether the constructor worked OK
  
  bool isOK;
  
  // Execution

  int Run();
  
private:
  
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // Processing input data

  DsInputPath *_input;
  Grib2Mdv *_grib2Mdv;
  time_t _latestDataTime;
  time_t _prevDataWriteTime;
  set<time_t> _fileDataTimes;
  
  // map of input levels

  typedef pair<double, DsMdvx*> LayerPair;
  typedef map<double, DsMdvx *>::iterator LayersIter;
  map<double, DsMdvx *> _layers;

  // output volume

  DsMdvx _outVol;
  MdvxRemapLut _remapLut;

  // private methods
  
  void _clearLayers();
  int _processFileRealtime(const string &inputPath);
  int _processFileArchive(const string &inputPath);
  int _getInputPathList(const string &inputPath, time_t dataTime,
                        vector<string> &timedPathList);
  int _combineVolume();

  double _getHeightKm(const DsMdvx &mdvx) const;
  time_t _getDataTime(const DsMdvx &mdvx) const;

  void _remapProjection(DsMdvx &mdvx);
  void _encodeFields(DsMdvx &mdvx);

};

#endif



