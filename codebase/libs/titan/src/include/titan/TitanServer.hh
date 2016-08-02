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
////////////////////////////////////////////////////////////////////
// <titan/TitanServer.hh>
//
// TITAN C++ track file io
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////////

#ifndef TitanServer_HH
#define TitanServer_HH


#include <string>
#include <titan/TitanComplexTrack.hh>
#include <titan/storm.h>
#include <titan/track.h>
using namespace std;

class TitanStormFile;
class TitanTrackFile;

// read time modes

typedef enum {
  TITAN_SERVER_READ_LATEST,         // latest available
  TITAN_SERVER_READ_CLOSEST,        // closest to requested time
  TITAN_SERVER_READ_FIRST_BEFORE,   // first before request time
  TITAN_SERVER_READ_NEXT_SCAN,      // from one scan after request time
  TITAN_SERVER_READ_PREV_SCAN,      // from one scan before request time
  TITAN_SERVER_READ_INTERVAL,       // between start and end times
  TITAN_SERVER_READ_LATEST_TIME     // from one scan before request time
} tserver_read_time_mode_t;
  
// track set

typedef enum {
  TITAN_SERVER_ALL_AT_TIME,         // all data for request time
  TITAN_SERVER_ALL_IN_FILE,         // all data in file for request time
  TITAN_SERVER_SINGLE_TRACK,        // all data in file for request time
  TITAN_SERVER_CURRENT_ENTRIES      // Only current entries for relevant tracks
} tserver_track_set_t;
  
// struct for searching for the requested scan
  
typedef struct {
  int iday;
  int num;
  time_t time;
} _tserver_scan_t;

class TitanServer
{

public:

  friend class DsTitanMsg;

  // constructor
  
  TitanServer();
  
  // destructor
  
  virtual ~TitanServer();
  
  // clear arrays
  
  void clearArrays();
  
  // clear all read requests, set defaults

  virtual void clearRead();

  ///////////////////////
  // set read time modes
  //
  // When a read is performed, we first find the relevant radar scan.
  // All other aspects of the read are relative to this scan.
  //
  // Default is readLatest.

  void setReadLatestTime(); // returns latest time only
                            // does not read any storm or track data
                            // After the read, use  getTimeInUse()
                            // to get latest time

  void setReadLatest();                    // latest scan

  void setReadClosest(time_t request_time, // closest scan, within
		      int margin);         // time margin

  void setReadInterval(time_t start_time,  // with interval
		       time_t end_time);

  void setReadFirstBefore(time_t request_time, // first scan before, within
			  int margin);         // time margin

  void setReadNext(time_t request_time, // find closest scan within time
		   int margin);         // margin, read next scan

  void setReadPrev(time_t request_time, // find closest scan within time
		   int margin);         // margin, read previous scan
  
  /////////////////
  // set track set
  //
  // You have the following track set options:
  //
  //  (a) all tracks alive at the scan time, or in the
  //      specified time interval. This is the default.
  //  (b) all tracks in the file which contains the scan time
  //  (c) select a particular complex track
  //  (d) read only entries for the current time - no history or future

  void setReadAllAtTime();
  void setReadAllInFile();
  void setReadSingleTrack(int complex_num);
  void setReadCurrentEntries();

  ////////////////////////////////////////////////////////////////////
  // set flags to fill out data arrays in the TitanTrackEntry objects.
  //
  // You can control whether you read:
  //  (a) layer props
  //  (b) dbz histograms
  //  (c) storm runs
  //  (d) projected-area runs
  //
  // All default to false.

  void setReadLprops() { _readLprops = true; }
  void setReadDbzHist() { _readDbzHist = true; }
  void setReadRuns() { _readRuns = true; }
  void setReadProjRuns() { _readProjRuns = true; }

  /////////////////////
  // Print read request
  
  void printReadRequest(ostream &out);

  /////////////////////////////////////////////////////
  // perform the read
  //
  // Returns 0 on success, -1 on failure.
  //
  // On failure, use getErrStr() to get error string.

  virtual int read(const string &dir);

  /////////////////////////////////////////////////////////////////////
  // time and file information after a read

  const string &getDirInUse() const { return _dirInUse; }
  const string &getStormPathInUse() const { return _stormPathInUse; }
  const string &getTrackPathInUse() const { return _trackPathInUse; }
  int getScanInUse() const { return _scanInUse; }
  time_t getTimeInUse() const { return _timeInUse; }
  time_t getDataStartTime() const { return _dataStartTime; }
  time_t getDataEndTime() const { return _dataEndTime; }
  int getIdayInUse() const { return _idayInUse; }
  
  /////////////////////////////////////////////////////////////////////
  // storm and track data access after a read

  const storm_file_params_t &storm_params() const { return _stormFileParams; }
  const track_file_params_t &track_params() const { return _trackFileParams; }

  // for track requests

  const vector<TitanComplexTrack *> &complex_tracks() const {
    return _complexTracks;
  }

  // for current entry requests

  const vector<TitanTrackEntry *> &current_entries() const {
    return _currentEntries;
  }

  ///////////////////////////////////////////////////////////////////
  // error string
  
  const string &getErrStr() const { return (_errStr); }

  // Print object
  
  void print(FILE *out);

  void printXML(FILE *out);

protected:

  // errors
  
  string _errStr;

  // setting up the read

  tserver_read_time_mode_t _readTimeMode;
  tserver_track_set_t _trackSet;
  time_t _requestTime;
  time_t _startTime;
  time_t _endTime;
  int _readTimeMargin;
  int _requestComplexNum;

  bool _readLprops;
  bool _readDbzHist;
  bool _readRuns;
  bool _readProjRuns;

  // storm and track file parameters
  
  storm_file_params_t _stormFileParams;
  track_file_params_t _trackFileParams;

  // vector of complex tracks

  vector<TitanComplexTrack *> _complexTracks;

  // vector of track entries
  // for _trackSet == TITAN_SERVER_CURRENT_ENTRIES

  vector<TitanTrackEntry *> _currentEntries;

  // track numbers in track set

  vector<int> _trackSetNums;
  
  // dir, file and scan currently in use

  string _dirInUse;
  string _stormPathInUse;
  string _trackPathInUse;
  int _scanInUse;
  time_t _timeInUse;
  time_t _dataStartTime;
  time_t _dataEndTime;
  int _idayInUse;

  // functions
  
  void _clearErrStr() { _errStr = ""; }
  int _findScan();
  int _findLatestScan();
  void _loadScanList(int iday, vector<_tserver_scan_t> &scanList);
  int _findLastDay(time_t &last_day);
  int _compileTrackSet(TitanTrackFile &tfile);

  int _readLatestTime();

  int _readTracks(TitanStormFile &sfile,
		  TitanTrackFile &tfile);

  int _readCurrentEntries(TitanStormFile &sfile,
			  TitanTrackFile &tfile);

private:
  
  // Private methods with no bodies. Copy and assignment not implemented.

  TitanServer(const TitanServer & orig);
  TitanServer & operator = (const TitanServer & other);
  
};

#endif


