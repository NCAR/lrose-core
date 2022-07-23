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
// SoundingText.hh
//
// SoundingText object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
///////////////////////////////////////////////////////////////

#ifndef SoundingText_H
#define SoundingText_H

#include <string>
#include <rapformats/coord_export.h>
#include <Mdv/DsMdvx.hh>
#include <Spdb/StationLoc.hh>
#include <toolsa/MsgLog.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

class Socket;

////////////////////////
// This class

class SoundingText {
  
public:

  // constructor

  SoundingText (int argc, char **argv);

  // destructor
  
  ~SoundingText();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  static const double _missing;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  coord_export_t  *_coords;
  MsgLog *_log;

  bool _dataToPrint;
  bool _printWind;
  bool _printTemp;
  bool _printRh;
  bool _printDp;
  MdvxField *_uField;
  MdvxField *_vField;
  MdvxField *_tempField;
  MdvxField *_rhField;
  Mdvx::vlevel_header_t _vhdr;
  int _nz;
  time_t _dataTime;
  string _locName;

  int _gatherData(time_t sample_time,
		  double lat, double lon,
		  DsMdvx &mdvx,
		  const StationLoc &stationLoc);
  
  int _loadPrintInfo(double lat, double lon,
		     const DsMdvx &mdvx,
		     const StationLoc &stationLoc);

  int _writeToStdout(time_t sample_time,
		     double lat, double lon,
		     const DsMdvx &mdvx,
		     const StationLoc &stationLoc);
  
  int _writeClassFile(time_t sample_time,
		      double lat, double lon,
		      const DsMdvx &mdvx,
		      const StationLoc &stationLoc);
  
};

#endif

