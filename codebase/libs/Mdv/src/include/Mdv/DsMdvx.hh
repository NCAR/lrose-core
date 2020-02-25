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

///////////////////////////////////////////////////////////////
// DsMdvx.hh
//
// DsMdvx object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////
//
// The DsMdvx object adds server capability to the Mdvx class.
//
// Note: The server protocol allows tunneling (transmission) through
// A Http server which has RAP's DsServerTunnel (a mod_perl module)
// installed. Additionally, it will optionally forward the server
// protocols through an outgoing Http proxy server. You must have
// a DsServerTunnel in place and use the tunnel_url=URL argument
// when using the proxy forwarding mechanism.
// Append the arguments:  ?tunnel_url=http://host/TunnelLocation 
// and then optionally: &proxy_url=http://host:port
//
///////////////////////////////////////////////////////////////

#ifndef DsMdvx_HH
#define DsMdvx_HH

#include <vector>

#include <Mdv/Mdvx.hh>
#include <didss/DsURL.hh>
#include <toolsa/ThreadSocket.hh>
using namespace std;

class DsMdvxMsg;
class DsMdvServer;
class DsMdvClimoServer;
class Mdv2NcfTrans;

///////////////////////////////////////////////////////////////
// class definition

class DsMdvx : public Mdvx

{

  friend class DsMdvxMsg;
  friend class DsMdvServer;
  friend class DsMdvClimoServer;

public:
  
  // constructor

  DsMdvx();

  // copy constructor
  
  DsMdvx(const DsMdvx &rhs);

  // destructor
  
  virtual ~DsMdvx();

  // assignment
  
  DsMdvx & operator=(const DsMdvx &rhs);

  // clear all memory, reset to starting state
  
  virtual void clear();

  // Overload clearRead - see Mdvx_read.hh

  virtual void clearRead();

  // Overload setReadPath() and setReadTime() methods.
  // In setReadPath(), you set the URL for the read path.
  // In setReadTime(), you set the URL for the read directory.
  // All other args are the same - see Mdvx_read.hh

  virtual void setReadTime(const read_search_mode_t mode,
			   const string &read_dir_url,
			   const int search_margin = 0,
			   const time_t search_time = 0,
			   const int forecast_lead_time = 0);
  virtual void clearReadTime();
  
  virtual void setReadPath(const string &read_path_url);
  virtual void clearReadPath();

  // overload setTimeList mode methods, specifying url instead of dir.
  // All other args are the same - see Mdvx_timelist.hh

  // setTimeListModeValid
  //
  // Set the time list so that it finds all of the valid data
  // times between the start and end times.
  // For forecast data where multiple forecasts exist for the same
  // valid time, a single valid time will be returned.
  
  virtual void setTimeListModeValid(const string &url,
				    time_t start_time,
				    time_t end_time);

  // setTimeListModeGen
  //
  // Set the time list so that it finds all of the
  // generate times between the start and end times
  
  virtual void setTimeListModeGen(const string &url,
				  time_t start_gen_time,
				  time_t end_gen_time);

  // setTimeListModeForecast
  // setTimeListModeLead (equivalent, deprecated)
  //
  // Set the time list mode so that it returns all of the forecast
  // times for the given generate time.

  virtual void setTimeListModeForecast(const string &url,
				       time_t gen_time);

  virtual void setTimeListModeLead(const string &url,
				   time_t gen_time);

  // setTimeListModeGenPlusForecasts
  //
  // Set the time list so that it finds all of the
  // generate times between the start and end gen times.
  // Then, for each generate time, all of the forecast times are
  // found. These are made available in the
  // _forecastTimesArray, which is represented by vector<vector<time_t> >
  
  virtual void setTimeListModeGenPlusForecasts(const string &url,
					       time_t start_gen_time,
					       time_t end_gen_time);

  // setModeValidMultGen
  //
  // Set the time list so that it finds all of the forecasts
  // within the time interval specified. For each forecast found
  // the associated generate time is also determined.
  // The forecast times will be available in the _timeList array.
  // The generate times will be available in the _genTimes array.
  
  virtual void setTimeListModeValidMultGen(const string &url,
					   time_t start_time,
					   time_t end_time);
 
  // setTimeListModeFirst
  //
  // set the time list so that it finds the first available data time
  
  virtual void setTimeListModeFirst(const string &url);

  // setTimeListModeLast
  //
  // set the time list so that it finds the last available data time
  
  virtual void setTimeListModeLast(const string &url);

  // setTimeListModeClosest
  //
  // set the time list mode so that it finds the closest available data time
  // to the search time within the search margin

  virtual void setTimeListModeClosest(const string &url,
				      time_t search_time,
				      int time_margin);

  // setTimeListModeFirstBefore
  //
  // set the time list mode so that it finds the first available data time
  // before the search time within the search margin
  
  virtual void setTimeListModeFirstBefore(const string &url,
					  time_t search_time,
					  int time_margin);

  // setTimeListModeFirstAfter
  //
  // set the time list mode so that it finds the first available data time
  // after the search time within the search margin
  
  virtual void setTimeListModeFirstAfter(const string &url,
					 time_t search_time,
					 int time_margin);

  // setTimeListModeBestForecast
  //
  // Set the time list so that it returns the best forecast
  // for the search time, within the time margin
  
  virtual void setTimeListModeBestForecast(const string &url,
					   time_t search_time,
					   int time_margin);

  // setTimeListModeSpecifiedForecast
  //
  // Set the time list so that it returns the forecast for the given
  // generate time, closest to the search time, within the time margin
  
  virtual void setTimeListModeSpecifiedForecast(const string &url,
						time_t gen_time,
						time_t search_time,
						int time_margin);

  // clearTimeListMode

  virtual void clearTimeListMode();

  // Overload read methods.
  // See Mdvx_read.hh and Mdvx_write.hh for details.
  
  virtual int readAllHeaders();
  virtual int readVolume();
  virtual int readVsection();

  // Overload time list methods.
  // See Mdvx_read.hh and Mdvx_write.hh for details.
  
  virtual int compileTimeList();
  virtual int compileTimeHeight();

  // Overload write methods.
  // See Mdvx_read.hh and Mdvx_write.hh for details.
  
  // Write to directory
  //
  // File path is computed - see setWriteAsForecast().
  // _latest_data_info file is written as appropriate - 
  //    see setWriteLdataInfo().
  //
  // Returns 0 on success, -1 on error.
  // getErrStr() retrieves the error string.
  
  virtual int writeToDir(const string &output_url);
  
  // Write to path
  //
  // File is  written to specified path.
  // Note: no _latest_data_info file is written.
  //
  // Returns 0 on success, -1 on error.
  // getErrStr() retrieves the error string.
  
  virtual int writeToPath(const string &output_path);
  
  // write files for an object which contains data for multiple times
  //
  // Some applications (such as converters from NetCDF) produce an
  // Mdvx object which contains fields from different forecast times.
  //
  // This routine separates out the fields for each time, and writes
  // one file per time
  //
  // Returns 0 on success, -1 on failure

  int writeMultForecasts(const string &output_url);
  
  // set/clear calcClimo option
  // This option specifies that the server should calculate
  // a climatology from the requested field(s).  This option
  // is only used by the DsMdvClimoServer.  It is ignored by
  // the DsMdvServer.
  
  void clearClimoRead();
  
  void setCalcClimo();
  void clearCalcClimo();

  // set the climo statistic type values

  void clearClimoTypeList();
  
  void addClimoStatType(const climo_type_t climo_type,
			const bool divide_by_num_obs,
			const double param1 = 0.0,
			const double param2 = 0.0);
  
  // set the climo date range

  void setClimoDataRange(const DateTime start_time,
			 const DateTime end_time);
  void setClimoDataRange(const time_t start_time,
			 const time_t end_time);
  
  // set the climo time range

  void setClimoTimeRange(const int start_hour,
			 const int start_minute,
			 const int start_second,
			 const int end_hour,
			 const int end_minute,
			 const int end_second);
  
  // overload prints

  virtual void printReadRequest(ostream &out);
  virtual void printTimeListRequest(ostream &out);
  virtual void printTimeHeightRequest(ostream &out);

  // get URLs

  const string &getReadDirUrl() const { return _readDirUrl; }
  const string &getReadPathUrl() const { return _readPathUrl; }
  const string &getTimeListUrl() const { return _timeListUrl; }

protected:

  string _readDirUrl;
  string _readPathUrl;
  string _timeListUrl;
  string _outputUrl;
  
  // Climatology read request members.  Climatology requests must go
  // through a server so the extra calculations can be done.

  bool _calcClimo;
  
  typedef struct
  {
    climo_type_t type;
    bool divide_by_num_obs;
    double params[2];
  } climo_stat_t;
  
  vector< climo_stat_t > _climoTypeList;
  
  DateTime _climoDataStart;
  DateTime _climoDataEnd;
  
  typedef struct
  {
    int hour;
    int minute;
    int second;
  } climo_time_t;
   
  climo_time_t _climoStartTime;
  climo_time_t _climoEndTime;

  ///////////////////
  // socket to server
  
  ThreadSocket _sock;

  // functions

  DsMdvx &_copy(const DsMdvx &rhs);

  int _resolveReadUrl(DsURL &url, bool *contact_server);

  int _resolveOutputUrl(DsURL &url, const string &output_url,
                        bool *contact_server);

  int _resolveTimeListUrl(DsURL &url, bool *contact_server);

  int _communicate(const DsURL &url, DsMdvxMsg &msg,
                   const void *msgBuf, const int msgLen);

  int _readAllHeadersRemote(const DsURL &url);
  int _readVolumeRemote(const DsURL &url);
  int _readVsectionRemote(const DsURL &url);

#ifdef JUNK
  int _writeToDirLocal(const string &url);
#endif

private:
};

#endif


