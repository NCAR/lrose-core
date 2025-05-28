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
// Tstorms2NetCDF.hh
//
// Tstorms2NetCDF object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2025
//
///////////////////////////////////////////////////////////////
//
// Tstorms2NetCDF reads native TITAN binary data files,
// converts the data into NetCDF format,
// and writes the data out in NetCDF files.
//
////////////////////////////////////////////////////////////////

#ifndef Tstorms2NetCDF_H
#define Tstorms2NetCDF_H

#include <string>
#include "Args.hh"
#include "Params.hh"
#include <didss/DsInputPath.hh>
#include <rapformats/tstorm_spdb.h>
#include <titan/TitanStormFile.hh>
#include <titan/TitanTrackFile.hh>
#include <titan/DsTitan.hh>
using namespace std;

////////////////////////
// This class

class Tstorms2NetCDF {
  
public:

  // constructor

  Tstorms2NetCDF (int argc, char **argv);

  // destructor
  
  ~Tstorms2NetCDF();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  DsInputPath *_input;

  int _processInput(const char *input_file_path);

  int _loadScanTimes(const char *input_file_path,
		     vector<time_t> &scanTimes);

  int _openInput(const char *input_file_path,
		 TitanTrackFile &tFile,
		 TitanStormFile &sFile);

  int _processTime(time_t valid_time,
		   time_t expire_time);
  
  int _writeNetcdfFile(time_t start_time,
                       time_t end_time,
                       const DsTitan &titan);

#ifdef JUNK

  void _loadTstormsXml(time_t start_time,
                       time_t end_time,
                       time_t valid_time,
                       time_t expire_time,
                       const DsTitan &titan,
                       string &xml);
  
  void _loadWxml(time_t start_time,
                 time_t end_time,
                 time_t valid_time,
                 time_t expire_time,
                 const DsTitan &titan,
                 string &xml);
  
  void _addWxmlObs(const storm_file_params_t &stormParams,
                   const simple_track_params_t &SimpleParams,
                   const TitanTrackEntry &TrackEntry,
                   bool isCurrent,
                   bool checkForParents,
                   bool checkForChildren,
                   string &xml);
  
  void _addWxmlForecast(int leadTime,
                        const storm_file_params_t &stormParams,
                        const simple_track_params_t &SimpleParams,
                        const TitanTrackEntry &TrackEntry,
                        string &xml);

  void _addEllipse(const TitanTrackEntry &TrackEntry,
		   double latCent, double lonCent,
		   double Speed, double Direction,
		   int leadTime, double linealGrowth,
		   string &xml);
  
  void _addPolygon(const storm_file_params_t &sParams,
		   const TitanTrackEntry &TrackEntry,
		   double latCent, double lonCent,
		   double Speed, double Direction,
		   int leadTime, bool grow,
		   string &xml);
  
  void _addMovingPoint(double latCent, double lonCent,
		       double Speed, double Direction,
		       string &xml);
  
  void _addWxmlHeader(string &xml);
  void _addWxmlTrailer(string &xml);

  void _addNowcastHeader(string &xml, time_t valid_time, time_t expire_time);
  void _addNowcastTrailer(string &xml);
    
  void _addTstormsHeader(string &xml);

  void _addTstormsTrailer(string &xml);

  void _addStormDataHeader(string &xml,
                           time_t valid_time,
                           const storm_file_params_t &stormParams);

  void _addStormDataTrailer(string &xml);
  
  void _addTstormsObs(const storm_file_params_t &stormParams,
                      const simple_track_params_t &SimpleParams,
                      const TitanTrackEntry &TrackEntry,
                      bool isCurrent,
                      bool checkForParents,
                      bool checkForChildren,
                      string &xml);
  
  void _addTstormsForecast(int leadTime,
                           const storm_file_params_t &stormParams,
                           const simple_track_params_t &SimpleParams,
                           const TitanTrackEntry &TrackEntry,
                           string &xml);
  
  int _storeXmlFromFile();

#endif

};

#endif

