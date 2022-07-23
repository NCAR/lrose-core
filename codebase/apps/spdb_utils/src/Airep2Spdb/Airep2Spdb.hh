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
/************************************************************************
 * Airep2Spdb.hh
 *
 * RAP, NCAR, Boulder CO
 *
 * Dec 2003
 *
 ************************************************************************/

#ifndef Airep2Spdb_HH
#define Airep2Spdb_HH

#include <vector>
#include <rapformats/pirep.h>
#include <toolsa/udatetime.h>
#include <Spdb/DsSpdb.hh>

#include "Args.hh"
#include "Params.hh"
#include <Spdb/StationLoc.hh>
using namespace std;

class DsInputPath;

class Airep2Spdb
{

 public:

  // constructor
  
  Airep2Spdb(int argc, char **argv);
  
  // Destructor

  ~Airep2Spdb(void);
  
  // Run the program.

  int Run();
  
  // Flag indicating whether the program status is currently okay.

  bool isOK;
  
 private:

  static const int SPDB_BUFFER_INCR = 10;

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // station location

  StationLoc _stationLoc;
  
  // Input objects

  DsInputPath *_inputPath;

  // flags from decoding airep

  bool _inAirep;
  bool _decodedAirep;
  bool _locationFound;
  bool _timeFound;
  bool _windFound;
  bool _tempFound;

  // airep information

  pirep_t _airepInfo;
  date_time_t _airepTime;
  date_time_t _inputFileTime;
  string _callSign;
  int _flevel;
  string _hdrStr;
  string _bodyStr;
  string _extraStr;
  vector<string> _tokens;
  
  // constants

  static const int MAX_LINE = 256;
  static const float MISSING_ALT = -999.0;
  static const float FT_TO_M = 0.3048; 

  char fgets_buffer[MAX_LINE];

  // functions

  void _addDecodedSpdb(DsSpdb &spdb);
  void _addAsciiSpdb(DsSpdb &spdb);
  
  int _writeAsciiSpdb(DsSpdb &spdb);
  int _writeDecodedSpdb(DsSpdb &spdb);
  
  int _read(FILE *input_file);
  char *_fgets(char *buffer, int len, FILE *input_file);
  void _fgets_save(char *buffer);
  void _tokenize(const char *line);

  int _decodeAirep();
  int _findFlevel(int &flevel);
  int _findAbeam();
  int _findLocation();
  int _findTime();
  int _findWind();
  int _findTemp();
  void _findText();

};



#endif


