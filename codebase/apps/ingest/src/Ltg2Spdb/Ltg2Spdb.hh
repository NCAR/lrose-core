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
// Ltg2Spdb.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2005
//
///////////////////////////////////////////////////////////////
//
// Ltg2Spdb reads LTG records from ASCII files, converts them to
// LTG_strike_t format (rapformats library) and stores them in SPDB.
//
////////////////////////////////////////////////////////////////

#ifndef Ltg2Spdb_H
#define Ltg2Spdb_H

#include <string>
#include <toolsa/MemBuf.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/ltg.h>

#include "Args.hh"
#include "Params.hh"
using namespace std;

class Nc3File;

////////////////////////
// This class

class Ltg2Spdb {
  
public:

  // constructor

  Ltg2Spdb (int argc, char **argv);

  // destructor
  
  ~Ltg2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  const static int _dataType = 0; // Constant arbitrary SPDB datatype.

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  time_t _startTime, _latestTime, _writeTime;

  // These entries used for getting rid of near duplicates - Niles.
  double _lastLat;
  double _lastLon;
  time_t _lastTime;
  int _strokeCount;
  int _flashDuration;
  int _lastNanosecs;

  // dataId
  //   SPDB_KAV_LTG_ID:  LTG_strike_t
  //   SPDB_LTG_ID:      LTG_extended_t
  
  int _dataId;
  string _dataLabel;

  // buffer holding strike data

  int _nStrikes;
  MemBuf _strikeBuffer;

  // NLDN binary

  typedef struct {
    char nldn[4]; // 'NLDN' marks the start of record
    si32 nhead;   // number of 28 byte header record blocks (includes 'NLDN' and nhead)
                  // (nhead*28 - 8) remaining header bytes (can be ignored) */
  } nldn_bin_header_t;

  typedef struct {

    si32 tsec; // seconds since 1970
    si32 nsec; // nanoseconds since tsec (NB: this appears be milliseconds, not nanoseconds)
    si32 lat; // latitude [deg] * 1000
    si32 lon; // longitude [deg] * 1000

    si16 fill1; // padding
    si16 sgnl; // signal strength * 10 (NB: 150 NLDN measures ~= 30 kAmps)
    si16 fill2; // padding

    si08 mult; // multiplicity (#strokes per flash)
    si08 fill3; // padding
    si08 semimaj; // semi-major axis
    si08 eccent; // eccentricity
    si08 angle; // ellipse angle
    si08 chisqr; // chi-square statistic
      
  } nldn_bin_record_t;

  // functions

  int _processFile(const char *file_path);
  int _processNldnFile(const char *file_path);
  int _processAOAWSNetCDFFile(const char *file_path);
  int _checkAOAWSNetCDFFile(const Nc3File &ncf);

  int _decode_format_1(const char *line);
  int _decode_format_2(const char *line);
  int _decode_format_3(const char *line);
  int _decode_format_4(const char *line);
  int _decode_ualf_lf_1(const char *line);
  int _decode_ualf_lf_1b(const char *line);
  int _decode_bom_axf(const char *line);
  int _decode_wwldn(const char *line);
  int _decode_wwlln(const char *line);
  int _decode_napln(const char *line);
  int _decode_ksc(const char *line, const char *file_path );
  int _decode_alblm(const char *line);
  int _checkNearDuplicate(const LTG_extended_t &strike );
  int _checkNearDuplicate(const LTG_strike_t &strike );
  int _checkNearDuplicate(const double lat, const double lon, const time_t time, const int nanosecs );

  void _addStrike(const LTG_strike_t &strike);
  void _addStrike(const LTG_extended_t &strike);

  int _writeIfReady();
  int _doWrite();


};

#endif

