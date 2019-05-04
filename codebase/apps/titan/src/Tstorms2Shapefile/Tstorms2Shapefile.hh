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
// Tstorms2Shapefile.hh
// Taken from pieces of Mike Dixon's Tstorms2Shapefile and Rview
//
// Terri L. Betancourt, RAP, NCAR
// June 2003
//
///////////////////////////////////////////////////////////////
//
// Tstorms2Shapefile reads native TITAN data files, 
// converts the data into ESRI shapefile format
//
////////////////////////////////////////////////////////////////

#ifndef Tstorms2Shapefile_H
#define Tstorms2Shapefile_H

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include <didss/DsInputPath.hh>
#include <titan/TitanStormFile.hh>
#include <titan/TitanTrackFile.hh>
#include <toolsa/MsgLog.hh>
#include <shapelib/shapefil.h>

using namespace std;


class Tstorms2Shapefile {
  
public:

  // constructor

  Tstorms2Shapefile (int argc, char **argv);

  // destructor
  
  ~Tstorms2Shapefile();

  // run 

  int Run();

  // data members

  bool isOK;

  //
  // Messaging
  //
  MsgLog&             getMsgLog(){ return msgLog; }

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  DsInputPath *_input;

  int _processTrackFile (const char *input_file_path);

  int _openFiles(const char *input_file_path,
		 TitanTrackFile &tFile,
		 TitanStormFile &sFile);

  int _loadScanTimes(TitanStormFile &sFile,
		     vector<time_t> &scanTimes);

  int _processScan(TitanStormFile &sFile,
		   TitanTrackFile &tFile,
		   int scan_num,
		   time_t valid_time,
		   time_t expire_time);

     
  int  openShapefile( time_t validTime, time_t leadTimeMin );

  int  storm2shape( const storm_file_params_t &sparams,
		    const track_file_entry_t &entry,
		    const storm_file_global_props_t &gprops,
		    const track_file_forecast_props_t &fprops,
		    const titan_grid_t &grid,  double leadTimeHr, time_t valid_time);
  
      
  //
  // Output file management 
  //

  Path             _outputPath;

  DBFHandle        _outputDbf;
  SHPHandle        _outputShp;

  SHPHandle        _directionShp;
  DBFHandle        _directionDbf;
  
  int _datetimeField, _tracknumberField, _vilField, _vildField, _precipField;
  int _complextracknumberField;
  int _areaField, _massField, _hailProbField;
  int _speedField, _directionField;
  int _VolField, _VolCentrxField, _VolCentryField, _topField;
  int _baseField, _hmaxZField, _ZmaxField, _ZmeanField;
  int _vilhailField, _hailmassaloftField, _FOKRField;
  int _tiltanField, _tiltdirField;
  int _radmajorField, _radminorField, _hailsizeField;
  int _leadTimeMinField;
  
  //
  // Messaging
  //
  MsgLog             msgLog;

};

//
// Make one instance global
//
#ifdef _TSTORMS2SHAPEFILE_MAIN_
Tstorms2Shapefile *driver;
#else
extern Tstorms2Shapefile *driver;
#endif

//
// Macros for message logging
//
#define POSTMSG          driver->getMsgLog().postMsg
#define DEBUG_ENABLED    driver->getMsgLog().isEnabled( DEBUG )

#endif

