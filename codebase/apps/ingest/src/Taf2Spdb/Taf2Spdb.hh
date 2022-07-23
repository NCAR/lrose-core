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
// Taf2Spdb.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2009
//
///////////////////////////////////////////////////////////////
//
// Taf2Spdb reads TAF data from text files
// and stores them in SPDB format.
//
///////////////////////////////////////////////////////////////

#ifndef Taf2Spdb_H
#define Taf2Spdb_H

#include <string>
#include <vector>
#include <didss/DsInputPath.hh>
#include <rapformats/Taf.hh>
#include <Spdb/StationLoc.hh>
#include <toolsa/DateTime.hh>
#include "Args.hh"
#include "Params.hh"
#include "Input.hh"
class DsSpdb;

using namespace std;

////////////////////////
// This class

class Taf2Spdb {
  
public:

  // constructor

  Taf2Spdb (int argc, char **argv);

  // destructor
  
  ~Taf2Spdb();

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

  // station location
  
  StationLoc _stationLoc;
  
  // Input objects

  DsInputPath *_inputPath;
  Input *_input;

  // tokenization

  vector<string> _toks;
  vector<bool> _used;
  int _tafTokNum;
  int _nameTokNum;
  int _issueTimeTokNum;
  int _validTimeTokNum;
  int _firstTokMainData;
  int _lastTokMainData;

  // time periods

  class PeriodLimits {
  public:
    int startTok; // first tok in period: e.g. TEMPO
    int startData; // first tok with data, i.e. wind info
    int endData; // last tok with data
    Taf::ForecastPeriod period;
    PeriodLimits() {
      clear();
    }
    void clear() {
      startTok = -1;
      startData = -1;
      endData = -1;
      period.clear();
    }
  };
  vector<PeriodLimits> _periodLimits;

  // station name

  string _name;

  // times

  time_t _fileTime;
  DateTime _refTime;

  // decoded TAF

  Taf _taf;
  
  // Functions to process the file and decode the message/report.
  
  int _processFile (const char *file_path);
  
  int _decodeMessage(string tafStr,
                     DsSpdb &asciiSpdb, DsSpdb &xmlSpdb);

  void _addPut(string tafStr,
               DsSpdb &asciiSpdb, DsSpdb &xmlSpdb);

  bool _acceptStation();
  int _findTafToken();
  void _setAmendedCorrected();
  int _setStationName();
  int _incrementDay(int day);
  time_t _computeTime(int day, int hour, int minute);
  int _setIssueTime();
  DateTime _getWmoHeaderTime();
  int _setValidExpireTimes();
  int _setStartEndTimes(const string &tok, time_t &startTime, time_t &endTime);
  int _setFromTime(const string &tok, time_t &fromTime);

  void _setPeriodLimits();
  void _setPeriodText();
  void _printPeriodLimits();

  int _setFields(int startTokNum,
                 int endTokNum,
                 Taf::ForecastPeriod &period);
  int _setWind(int startTokNum,
               int endTokNum,
               Taf::ForecastPeriod &period);
  bool _checkCavok(int startTokNum,
                   int endTokNum,
                   Taf::ForecastPeriod &period);
  int _setVis(int startTokNum,
              int endTokNum,
              Taf::ForecastPeriod &period);

  int _setClouds(int startTokNum,
                 int endTokNum,
                 Taf::ForecastPeriod &period);
    
  void _setWx(int startTokNum,
              int endTokNum,
              Taf::ForecastPeriod &period);
    
  void _setTemps(int startTokNum,
                 int endTokNum,
                 Taf::ForecastPeriod &period);
    
  void _overwriteCancelled();

  static void _tokenize(const string &str, const string &spacer,
			vector<string> &toks);
  
};

#endif

