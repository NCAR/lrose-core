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
// Server.cc for AcTracks2Symprod
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
///////////////////////////////////////////////////////////////

#ifndef _Server_HH
#define _Server_HH

#include <string>
#include <dsserver/DsProcessServer.hh>
#include <Spdb/Symprod.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <Spdb/Product_defines.hh>
#include "Params.hh"
using namespace std;
#define AC_CALLSIGN_LEN 12

class Server : public DsProcessServer
{

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              

  Server(const string &prog_name,
	 const Params *initialParams);

  // destructor
  
  virtual ~Server(){};
  
protected:

  // Override base class on timeout and post handlers
  // always return true - i.e. never exit

  virtual bool timeoutMethod();
  virtual bool postHandlerMethod();

  // Handle a client's request for data. The passed
  // message is a decoded DsMessage.
  
  virtual int handleDataCommand(Socket *socket,
				const void *data, ssize_t dataSize);
  
private:

  typedef struct {
    time_t time;
    fl32 lat;
    fl32 lon;
    fl32 alt;
    string callsign;
  } location_t;
  
  // initial params set in parent - do not modify in threads
  // local params are used in threads

  const Params *_initialParams;

  // copy of the read message

  DsSpdbMsg _readMsg;
  
  // horizontal and vertical limits

  bool _horizLimitsSet;
  double _minLat, _minLon, _maxLat, _maxLon;

  bool _vertLimitsSet;
  double _minHt, _maxHt;

  // auxiliary XML commands

  string _auxXml;
  
  // methods invoked from the base class for managing 
  // local override of parameter file
  
  int _loadLocalParams(const string &paramFile, Params &params);
  
  // set the limits from the message
  
  void _setLimitsFromMsg(const DsSpdbMsg &inMsg);

  // _handleGet()

  int _handleGet(const Params &params, const DsSpdbMsg &inMsg, 
                 string &dirPath, Socket &socket);

  
  // function for actually doing the get from disk
  
  int _doGet(const Params &params,
	     Spdb &spdb,
	     const string &dir_path,
	     int get_mode,
	     const DsSpdbMsg::info_t &info,
	     DsSpdbMsg::info_t *get_info,
	     string &errStr,
	     time_t &requestTime,
	     time_t &startTime,
	     time_t &endTime);
  
  // transform the data
  
  void _transformData(const Params &params,
                      const string &dir_path,
                      const time_t request_time,
                      const time_t start_time,
                      const time_t end_time,
                      const int prod_id,
                      const string &prod_label,
                      const int n_chunks_in,
                      const Spdb::chunk_ref_t *chunk_refs_in,
                      const Spdb::aux_ref_t *aux_refs_in,
                      const void *chunk_data_in,
                      int &n_chunks_out,
                      MemBuf &refBufOut,
                      MemBuf &auxBufOut,
                      MemBuf &dataBufOut);
  
 // Convert the given data chunk from the SPDB database to symprod format.

  int _convertToSymprod(const Params &params,
                        const string &dir_path,
                        const int prod_id,
                        const string &prod_label,
                        const time_t request_time,
                        const vector<location_t> &track,
                        MemBuf &symprod_buf);

  void _addIcons(Symprod &prod,
		 const Params &params,
		 const vector<location_t> &locations,
		 const int startIndex,
		 const int endIndex,
		 const ui08 *bitmap,
		 const int icon_ny,
		 const int icon_nx,
		 const char *color,
		 double &prevLat,
		 double &prevLon);

  void _addAltText(Symprod &prod,
		   const Params &params,
		   const location_t &loc,
		   const char *color,
		   double &prevLat,
		   double &prevLon);
  
  void _addPolyline(Symprod &prod,
		    const Params &params,
		    const vector<location_t> locations,
		    const int startIndex,
		    const int endIndex,
		    const char *color);

  void _addDirnArrow(Symprod &prod,
		     const Params &params,
		     const vector<location_t> locations,
		     const int index,
		     const char *color);

  void _addCallsignLabel(Symprod &prod,
			 const location_t &loc,
			 const char *color,
			 const char *font,
			 const Symprod::vert_align_t vert_align,
			 const Symprod::horiz_align_t horiz_align,
			 const int x_offset,
			 const int y_offset);
  
  ui08 *_loadIconBuf(int **param_icon,
		     int &param_icon_ny,
		     int &param_icon_nx,
		     MemBuf &iconBuf);
  
  const char *_altitudeColor(const Params &params,
			     const double altitude,
			     const char *default_color);

  const char *_callsignColor(const Params &params,
			     const string &callsign,
			     const char *default_color);
 
  Symprod::vert_align_t _vertAlign(Params::label_vert_align_t align);

  Symprod::horiz_align_t _horizAlign(Params::label_horiz_align_t align);

  bool _acceptPoint(double lat, double lon);

  void _addTimeLabels(Symprod &prod,
                      const Params &params,
                      const vector<location_t> &track);

};

#endif
