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

#include "Server.hh"
#include "Params.hh"
#include <vector>
#include <set>
#include <iomanip>
#include <cassert>
#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/Socket.hh>
#include <toolsa/DateTime.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DsLocator.hh>
#include <rapformats/ac_posn.h>
#include <rapformats/ac_data.h>
#include <toolsa/toolsa_macros.h>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Constructor
//
// Inherits from DsProcessServer

Server::Server(const string &prog_name,
	       const Params *initialParams)
        : DsProcessServer(prog_name,
                          initialParams->instance,
                          initialParams->port,
                          initialParams->qmax,
                          initialParams->max_clients,
                          initialParams->debug >= Params::DEBUG_NORM,
                          initialParams->debug >= Params::DEBUG_VERBOSE,
                          true, true, true),
          _initialParams(initialParams)
  
{
  assert(initialParams != NULL);
  setNoThreadDebug(initialParams->no_threads);
  _horizLimitsSet = false;
  _vertLimitsSet = false;
}

/////////////////////////////////////////////////////////
// handle the data command from the parent class
// always return true, so that parent will not exit

int Server::handleDataCommand(Socket * socket,
                              const void * data, ssize_t dataSize)

{

  if (_isDebug) {
    cerr << "Entering Server::handleDataCommand()." << endl;
    cerr << "  " << DateTime::str() << endl;
  }

  if (_isVerbose) {
    cerr << "Client thread disassembling message..." << endl;
  }

  // disassemble the incoming request

  DsSpdbMsg msg;
  if (msg.disassemble((void *) data, dataSize)) {
    cerr << "ERROR - COMM - Server::handleDataCommand" << endl;
    cerr << "Invalid DsSpdbMsg message" << endl;
    return -1;
  }
  _setLimitsFromMsg(msg);
  _auxXml = msg.getAuxXml();
  
  // check message type - can only handle get messages

  if (msg.getSubType() != DsSpdbMsg::DS_SPDB_GET) {
    cerr << "ERROR - Server::handleDataCommand\n"
         << "  Cannot handle request" << endl;
    msg.print(cerr);
    return 0;
  }
  
  if (_isDebug) {
    cerr << "------------------------------------" << endl;
    msg.print(cerr);
  }
  
  // verify the url and determine the spdb directory path

  string url_str(msg.getUrlStr());
  DsURL url(url_str);
  if (!url.isValid()) {
    cerr << "ERROR - COMM - Server::handleDataCommand" << endl;
    cerr << "   Invalid URL: '" << url_str <<  "'\n";
    return 0;
  }
  string dirPath;
  RapDataDir.fillPath(url, dirPath);

  // Check for local copy of params
  // Override initial params if params file exists in the datatype directory

  Params localParams(*_initialParams);

  // NOTE: the DsLocator will resolve the parameter file name 
  //       and modify the url
  
  bool  paramsExist;
  if (DsLocator.resolveParam(url, _executableName,
                             &paramsExist) != 0) {
    cerr << "ERROR - COMM - Server::handleDataCommand\n"
         << "Cannot resolve parameter specification in url:\n"
         << url.getURLStr()
         << endl;
    return 0;
  }
  
  // The application-specfic code must load the override parameters
  
  if (paramsExist) {
    if (_loadLocalParams(url.getParamFile(), localParams)) {
      cerr << "ERROR - Server::handleDataCommand\n"
           << "  Cannot load parameter file:\n"
           << url.getParamFile()
           << endl;
    }
  }
  
  // copy the read message

  _readMsg = msg;
  
  // process the get request

  _handleGet(localParams, msg, dirPath, *socket);
  
  if (_isDebug) {
    cerr << "Exiting Server::handleDataCommand()." << endl;
  }
  
  return 0;

}
    
// Override base class on timeout and post handlers
// always return true - i.e. never exit
bool Server::timeoutMethod()
{
  DsProcessServer::timeoutMethod();
  return true; // Continue to wait for clients.
}
bool Server::postHandlerMethod()
{
  DsProcessServer::postHandlerMethod();
  return true; // Continue to wait for clients.
}

//////////////////////////////////////////////////////////////////////
// load local params if they are to be overridden.

int
Server::_loadLocalParams(const string &paramFile, Params &params)

{
  char **tdrpOverrideList = NULL;
  bool expandEnvVars = true;

  const char *routine_name = "loadLocalParams";

  if (_isDebug) {
    cerr << "Loading new params from file: " << paramFile << endl;
  }

  if (params.load(paramFile.c_str(), tdrpOverrideList,
                  expandEnvVars, _isVerbose)) {
    cerr << "ERROR - " << _executableName << "::" << routine_name << endl
         << "Cannot load parameter file: " << paramFile << endl;
    return -1;
  }
  
  if (_isVerbose) {
    params.print(stderr, PRINT_SHORT);
  }

  // set the bounding box from the params, if not already set from
  // the message
  
  if (params.useBoundingBox && !_horizLimitsSet) {
    _minLat = params.bounding_box.min_lat;
    _minLon = params.bounding_box.min_lon;
    _maxLat = params.bounding_box.max_lat;
    _maxLon = params.bounding_box.max_lon;
    _horizLimitsSet = true;
  }

  if (_isDebug && _horizLimitsSet){
    cerr << "Horizontal limits set." << endl;
    cerr << "  Min lat: " << _minLat << endl;
    cerr << "  Min lon: " << _minLon << endl;
    cerr << "  Max lat: " << _maxLat << endl;
    cerr << "  Max lon: " << _maxLon << endl;
    cerr << endl;
  }
  
  if (_horizLimitsSet) {
    
    // adjust the min and max lat and lon values, to give 25% outside
    // the requested area, so we do not miss locations close to the
    // edge
    
    double deltaLat = fabs(_maxLat - _minLat);
    _minLat -= deltaLat * 0.25;
    _maxLat += deltaLat * 0.25;
    
    double deltaLon = fabs(_maxLon - _minLon);
    _minLon -= deltaLon * 0.25;
    _maxLon += deltaLon * 0.25;
    
  }
  
  return 0;

}

///////////////////////////////////
// set the limits from the message

void Server::_setLimitsFromMsg(const DsSpdbMsg &inMsg)

{

  if (inMsg.horizLimitsSet()) {
    const DsSpdbMsg::horiz_limits_t &hlimits = inMsg.getHorizLimits();
    _minLat = hlimits.min_lat;
    _minLon = hlimits.min_lon;
    _maxLat = hlimits.max_lat;
    _maxLon = hlimits.max_lon;
    _horizLimitsSet = true;
  }
  if (inMsg.vertLimitsSet()) {
    const DsSpdbMsg::vert_limits_t &vlimits = inMsg.getVertLimits();
    _minHt = vlimits.min_ht;
    _maxHt = vlimits.max_ht;
    _vertLimitsSet = true;
  }

}  

////////////////
// _handleGet()

int Server::_handleGet(const Params &params, const DsSpdbMsg &inMsg, 
		       string &dirPath, Socket &socket)
  
{
  
  if (params.debug >= Params::DEBUG_VERBOSE) {
    cerr << _executableName << "::Server::_handleGet" << endl;
  }

  string errStr = "ERROR -";
  errStr += _executableName;
  errStr += "::_handleGet\n";
  Spdb spdb;
  DsSpdbMsg::info_t get_info;
  time_t requestTime, startTime, endTime;
  DsSpdbMsg replyMsg;
  
  if (_doGet(params, spdb, dirPath,
	     inMsg.getMode(), inMsg.getInfo(),
	     &get_info, errStr,
	     requestTime, startTime, endTime)) {
    
    errStr += "  URL: ";
    errStr += inMsg.getUrlStr();
    errStr += "\n";
    replyMsg.assembleGetErrorReturn(inMsg.getSpdbMode(), errStr.c_str());
    
  } else {
    
    MemBuf refBufOut;
    MemBuf auxBufOut;
    MemBuf dataBufOut;
    int nChunksOut;
    
    _transformData(params, dirPath,
                   requestTime, startTime, endTime,
                   spdb.getProdId(), spdb.getProdLabel(),
                   spdb.getNChunks(),
                   spdb.getChunkRefs(),
                   spdb.getAuxRefs(),
                   spdb.getChunkData(),
                   nChunksOut, refBufOut, auxBufOut, dataBufOut);
    
    get_info.prod_id =  SPDB_SYMPROD_ID;
    STRncopy(get_info.prod_label, SPDB_SYMPROD_LABEL, SPDB_LABEL_MAX);
    get_info.n_chunks = nChunksOut;
    replyMsg.assembleGetDataSuccessReturn
      (inMsg.getSpdbMode(), get_info,
       refBufOut, auxBufOut,
       dataBufOut, inMsg.getDataBufCompression());

  }
    
  // send reply
  
  void *replyBuf = replyMsg.assembledMsg();
  int replyBuflen = replyMsg.lengthAssembled();
  
  if (socket.writeMessage(DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
			  replyBuf, replyBuflen, 1000)) {
    cerr << "ERROR - COMM -DsSpdbServer::_handleGet" << endl;
    cerr << "  Writing reply" << endl;
    return -1;
  }

  if (_isVerbose) {
    cerr << "Writing message, len: " << replyBuflen << endl;
  }

  return 0;
  
}

///////////////////////////////////////////////
// function for actually doing the get from disk
// Overloaded, non-virtual - hides DsSpdbServer::_doGet()

int Server::_doGet(const Params &params,
		   Spdb &spdb,
		   const string &dir_path,
		   int get_mode,
		   const DsSpdbMsg::info_t &info,
		   DsSpdbMsg::info_t *get_info,
		   string &errStr,
		   time_t &requestTime,
		   time_t &startTime,
		   time_t &endTime)
  
{

  *get_info = info;

  // set the request time

  switch (get_mode) {
    
  case DsSpdbMsg::DS_SPDB_GET_MODE_EXACT:
    requestTime = info.request_time;
    break;
    
  case DsSpdbMsg::DS_SPDB_GET_MODE_CLOSEST:
    requestTime = info.request_time;
    break;
    
  case DsSpdbMsg::DS_SPDB_GET_MODE_INTERVAL:
    requestTime = info.start_time + (info.end_time - info.start_time) / 2;
    break;

  case DsSpdbMsg::DS_SPDB_GET_MODE_VALID:
    requestTime = info.request_time;
    break;

  case DsSpdbMsg::DS_SPDB_GET_MODE_LATEST:
    requestTime = time(NULL);
    break;

  case DsSpdbMsg::DS_SPDB_GET_MODE_FIRST_BEFORE:
    requestTime = info.request_time;
    break;

  case DsSpdbMsg::DS_SPDB_GET_MODE_FIRST_AFTER:
    requestTime = info.request_time;
    break;

  default:
    requestTime = time(NULL);
    break;
    
  } // switch

  // get the data in the required interval around the request time

  startTime = requestTime - params.before_secs;
  endTime = requestTime + params.after_secs;
  
  if (params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Interval: " << utimstr(startTime) << " to " 
	 << utimstr(endTime) << endl;
  }
  
  if (spdb.getInterval(dir_path, startTime, endTime,
		       info.data_type)) {
    errStr += "  get INTERVAL mode failed\n";
    errStr += spdb.getErrStr();
    return -1;
  }
  
  // check prod_id

  int prod_id = spdb.getProdId();
  if (prod_id != SPDB_AC_POSN_ID && prod_id != SPDB_AC_DATA_ID) {
    cerr << "WARNING - " << _executableName << "::Server::_doGet" << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_AC_POSN_ID: " << SPDB_AC_POSN_ID
	 << ", or" << endl;
    cerr << "  Should be SPDB_AC_DATA_ID: " << SPDB_AC_DATA_ID << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////
// transform the data
// non-virtual - hides DsSpdbServer::transformData()

void Server::_transformData(const Params &params,
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
                            MemBuf &dataBufOut)

{

  if (params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "n_chunks_in: " << n_chunks_in << endl;
    cerr << "dir_path: " << dir_path << endl;
    cerr << "prod_id: " << prod_id << endl;
  }

  // initialize the buffers
  
  refBufOut.free();
  auxBufOut.free();
  dataBufOut.free();
  n_chunks_out = 0;

  // load up vector of aircraft locations

  vector<location_t> locations;

  if (prod_id == SPDB_AC_POSN_ID) {

    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "prod_id: SPDB_AC_POSN_ID" << endl;
    }

    for (int i = 0; i < n_chunks_in; i++) {
      ac_posn_t posn;
      memcpy(&posn,
	     (char *) chunk_data_in + chunk_refs_in[i].offset,
	     sizeof(ac_posn_t));
      BE_to_ac_posn(&posn);
      if (_acceptPoint(posn.lat, posn.lon)) {
	location_t loc;
	loc.time = chunk_refs_in[i].valid_time;
	loc.lat = posn.lat;
	loc.lon = posn.lon;
	loc.alt = posn.alt;
	loc.callsign = posn.callsign;
	locations.push_back(loc);
      }
    } // i

  } else if (prod_id == SPDB_AC_DATA_ID) {

    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "prod_id: SPDB_AC_DATA_ID" << endl;
    }
    
    for (int i = 0; i < n_chunks_in; i++) {
      ac_data_t posn;
      memcpy(&posn,
	     (char *) chunk_data_in + chunk_refs_in[i].offset,
	     sizeof(ac_data_t));
      ac_data_from_BE(&posn);
      if (_acceptPoint(posn.lat, posn.lon)) {
	location_t loc;
	loc.time = chunk_refs_in[i].valid_time;
	loc.lat = posn.lat;
	loc.lon = posn.lon;
	loc.alt = posn.alt;
	loc.callsign = posn.callsign;
	locations.push_back(loc);
      }
    } // i
    
  }

  if (params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=======================================" << endl;
    cerr << "========>> Data retrieved <<===========" << endl;
    for (size_t ii = 0; ii < locations.size(); ii++) {
      const location_t &loc = locations[ii];
      cerr << "callsign, lat, lon, time: "
	   << loc.callsign << " "
	   << setw(9) << loc.lat << " "
	   << setw(9) << loc.lon << " "
	   << utimstr(loc.time) << endl;
    }
    cerr << "=======================================" << endl;
  }

  // compute the set of aircraft callsigns
  
  set<string, less<string> > callSigns;

  for (size_t ii = 0; ii < locations.size(); ii++) {

    if (params.specify_callsigns) {

      // callsigns list is specified

      for (int jj = 0; jj < params.callsign_list_n; jj++) {
	string thisCallSign = params._callsign_list[jj];
	if (locations[ii].callsign == thisCallSign) {
	  callSigns.insert(callSigns.begin(), locations[ii].callsign);
	  break;
	}
      }
      
    } else {
      
      // See if we are skipping GA aircraft
      
      if (params.display_ga) {
	callSigns.insert(callSigns.begin(), locations[ii].callsign);
      } else {
	if (isdigit(locations[ii].callsign[0]) ||
	    isdigit(locations[ii].callsign[1]) ||
	    isdigit(locations[ii].callsign[2])) {
	  if (params.debug >= Params::DEBUG_VERBOSE) {
	    cerr << "--->  Skipping GA aircraft: "
		 << locations[ii].callsign << endl;
	  }
	} else {
	  callSigns.insert(callSigns.begin(), locations[ii].callsign);
	}
      }

    }
    
  } // ii
  
  // loop through the callsigns

  set<string, less<string> >::iterator jj;
  for (jj = callSigns.begin(); jj != callSigns.end(); jj++) {

    // create a vector of positions for this track
    
    string callsign = *jj;
    vector<location_t> track;
    
    for (size_t ii = 0; ii < locations.size(); ii++) {
      location_t &loc = locations[ii];
      if (callsign == loc.callsign) {
	track.push_back(loc);
      }
    }

    if (params.debug >= Params::DEBUG_VERBOSE) {

      cerr << "=======================================" << endl;
      cerr << "==>> Track for callsign:" << callsign << endl;
      for (size_t ii = 0; ii < track.size(); ii++) {
	const location_t &loc = track[ii];
	cerr << "callsign, lat, lon, time: "
	     << loc.callsign << " "
	     << setw(9) << loc.lat << " "
	     << setw(9) << loc.lon << " "
	     << utimstr(loc.time) << endl;
      }
      cerr << "=======================================" << endl;
    }

    // convert the track to symprod

    MemBuf symprodBuf;
    if (_convertToSymprod(params, dir_path, prod_id, prod_label,
                          request_time, track, symprodBuf) == 0) {

      Spdb::chunk_ref_t ref;
      MEM_zero(ref);
      ref.offset = dataBufOut.getLen();
      ref.len = symprodBuf.getLen();
      refBufOut.add(&ref, sizeof(ref));

      Spdb::aux_ref_t aux;
      MEM_zero(aux);
      aux.write_time = (ti32) time(NULL);
      auxBufOut.add(&aux, sizeof(aux));

      dataBufOut.add(symprodBuf.getPtr(), symprodBuf.getLen());

      n_chunks_out++;

    }
    
  } // jj

}

//////////////////////////////////////////////////////////////////////
// convertToSymprod() - Convert the given data chunk from the SPDB
//                      database to symprod format.
//
// Returns 0 on success, -1 on failure

int Server::_convertToSymprod(const Params &params,
                              const string &dir_path,
                              const int prod_id,
                              const string &prod_label,
                              const time_t request_time,
                              const vector<location_t> &track,
                              MemBuf &symprod_buf)
  
{

  if (params.debug >= Params::DEBUG_VERBOSE) {
    cerr << _executableName << "::Server::convertToSymprod" << endl;
    cerr << "request_time: " << utimstr(request_time) << endl;
    cerr << "track len: " << track.size() << endl;
  }

  if (track.size() < 1) {
    return 0;
  }

  string callsign = track[0].callsign;
  if (params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "callsign: " << callsign << endl;
  }

  // compute the index for current location

  int minDiff = 1000000000;
  int currentIndex = track.size() - 1;
  for (size_t ii = 0; ii < track.size(); ii++) {
    int diff = abs(track[ii].time - request_time);
    if (diff < minDiff) {
      currentIndex = ii;
      minDiff = diff;
    } else {
      break;
    }
  }

  // create Symprod object
  
  time_t now = time(NULL);
  time_t expire_time = request_time + params.valid_secs;
  Symprod prod(now, now, request_time, expire_time, 0, 0,
               track[0].callsign.c_str()); 
  
  // Initialize any icons we may need to use.

  ui08 *beforeIcon = NULL;
  MemBuf beforeIconBuf;
  int beforeIconNx = params.before_icon_n2;
  int beforeIconNy = params.before_icon_n1;
  
  if (params.display_before_icon) {
    beforeIcon = _loadIconBuf(params.__before_icon,
			      beforeIconNy, beforeIconNx,
			      beforeIconBuf);
  }
  
  ui08 *currentIcon = NULL;
  MemBuf currentIconBuf;
  int currentIconNx = params.current_icon_n2;
  int currentIconNy = params.current_icon_n1;

  if (params.display_current_icon) {
    currentIcon = _loadIconBuf(params.__current_icon,
			       currentIconNy, currentIconNx,
			       currentIconBuf);
  }
  
  ui08 *afterIcon = NULL;
  MemBuf afterIconBuf;
  int afterIconNx = params.after_icon_n2;
  int afterIconNy = params.after_icon_n1;
  
  if (params.display_after_icon) {
    afterIcon = _loadIconBuf(params.__after_icon,
			     afterIconNy, afterIconNx,
			     afterIconBuf);
  }
  
  if (params.color_by_altitude) {
    
    // Add polylines for track colored by altitude
    
    for (size_t i = 1; i < track.size(); i++) {
      double alt = (track[i].alt + track[i-1].alt) / 2.0;
      const char *color = _altitudeColor(params, alt, params.before_icon_color);
      _addPolyline(prod, params, track, i-1, i, color);
    }
    
  } else {
    
    // add polyline for track before current pos
    
    if (params.display_before_polyline) {
      const char *color = _callsignColor(params, callsign, params.before_polyline_color);
      _addPolyline(prod, params, track,
		   0, currentIndex, color);
    }
    
    // add polyline for track after current pos
    
    if (params.display_after_polyline) {
      const char *color = _callsignColor(params, callsign, params.after_polyline_color);
      _addPolyline(prod, params, track,
		   currentIndex, track.size() - 1, color);
    }

  } // if (params.color_by_altitude) 
  
  double prevLat = 1000.0;
  double prevLon = 1000.0;
  
  // Add icons for track before the current position
  
  if (params.display_before_icon) {
    
    if (params.color_by_altitude) {
      for (int i = 0; i < currentIndex; i++) {
	const char *color = _altitudeColor(params, track[i].alt,
				      params.before_icon_color);
	if (params.plot_altitude_text) {
	  _addAltText(prod, params, track[i], color, prevLat, prevLon);
	} else {
	  _addIcons(prod, params, track, i, i,
		    beforeIcon, beforeIconNx, beforeIconNy,
		    color,
		    prevLat, prevLon);
	}
      } // i
    } else {
      const char *color = _callsignColor(params, callsign, params.before_icon_color);
      _addIcons(prod, params, track, 0, currentIndex - 1,
		beforeIcon, beforeIconNx, beforeIconNy,
		color, prevLat, prevLon);
    }
  }
  
  // Add icons for track at the current position
  
  if (params.display_current_icon) {
    const char *color;
    if (params.color_by_altitude) {
      color = _altitudeColor(params, track[currentIndex].alt,
			     params.current_icon_color);
    } else {
      color = _callsignColor(params, callsign, params.current_icon_color);
    }
    if (params.plot_altitude_text) {
      _addAltText(prod, params, track[currentIndex], color, prevLat, prevLon);
    } else {
      _addIcons(prod, params, track, currentIndex, currentIndex,
		currentIcon, currentIconNx, currentIconNy,
		color, prevLat, prevLon);
    }
  }
  
  // Add icons for track after the current position
  
  if (params.display_after_icon) {
    
    if (params.color_by_altitude) {
      for (size_t i = currentIndex + 1; i < track.size(); i++) {
	const char *color = _altitudeColor(params, track[i].alt,
				      params.after_icon_color);
	if (params.plot_altitude_text) {
	  _addAltText(prod, params, track[i], color, prevLat, prevLon);
	} else {
	  _addIcons(prod, params, track, i, i,
		    afterIcon, afterIconNx, afterIconNy,
		    color,
		    prevLat, prevLon);
	}
      } // i
    } else {
      const char *color = _callsignColor(params, callsign, params.after_icon_color);
      _addIcons(prod, params, track, currentIndex + 1, track.size() - 1,
		afterIcon, afterIconNx, afterIconNy,
		color, prevLat, prevLon);
    }
  }
  
  // add label

  if (params.display_label) {
    _addCallsignLabel(prod,
		      track[currentIndex],
		      params.label_color,
		      params.label_font,
		      _vertAlign(params.label_vert_align),
		      _horizAlign(params.label_horiz_align),
		      params.label_offset.x_offset,
		      params.label_offset.y_offset);
  }
  
  // add arrow if required

  if (params.plot_dirn_arrow) {

    if (params.display_after_polyline) {
      const char *color =
	_callsignColor(params, callsign, params.after_polyline_color);
      _addDirnArrow(prod, params, track, track.size() -1, color);
    } else if (params.display_before_polyline) {
      const char *color =
	_callsignColor(params, callsign, params.before_polyline_color);
      _addDirnArrow(prod, params, track, currentIndex, color);
    } else {
      const char *color = _callsignColor(params, callsign, params.current_icon_color);
      _addDirnArrow(prod, params, track, currentIndex, color);
    }

  }

  // add time labels if required

  if (params.add_time_labels) {
    _addTimeLabels(prod, params, track);
  }
  
  // copy internal representation of product to output buffer

  prod.serialize(symprod_buf);
  return(0);
  
}

//////////////////////////////////////////////////////////////////////
// _addIcons() - Add the given group of icons.
//

void Server::_addIcons(Symprod &prod,
		       const Params &params,
		       const vector<location_t> &locations,
		       const int startIndex,
		       const int endIndex,
		       const ui08 *bitmap,
		       const int icon_ny,
		       const int icon_nx,
		       const char *color,
		       double &prevLat,
		       double &prevLon)

{

  if (endIndex < startIndex) {
    return;
  }
  if (!bitmap) {
    return;
  }

  MemBuf originBuf;

  int nIcons = 0;
  for (int ii = startIndex; ii <= endIndex; ii++) {

    Symprod::wpt_t origin;
    origin.lat = locations[ii].lat;
    origin.lon = locations[ii].lon;

    if (params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Icon: lat, lon: " << locations[ii].lat << ", "
	   << locations[ii].lon << endl;
    }

    if (params.sparse_icons) {
      double r, theta;
      PJGLatLon2RTheta(origin.lat, origin.lon, prevLat, prevLon, &r, &theta);
      if (r >= params.sparse_icons_min_distance) {
	prevLat = origin.lat;
	prevLon = origin.lon;
	originBuf.add(&origin, sizeof(origin));
	nIcons++;
      }
    } else {
      originBuf.add(&origin, sizeof(origin));
      nIcons++;
    }
    
  }

  if (params.debug >= Params::DEBUG_VERBOSE) {
    cerr << nIcons << " icons loaded." << endl;
  }

  // load up product

  if (nIcons > 0) {
    prod.addBitmapIcons(color, nIcons,
			(Symprod::wpt_t *) originBuf.getPtr(),
			icon_nx, icon_ny,
			bitmap);
  }

}

//////////////////////////////////////////////////////////////////////
// add_alt_text()
//
// Add the altitude text centered on the aircraft position.

void Server::_addAltText(Symprod &prod,
			 const Params &params,
			 const location_t &loc,
			 const char *color,
			 double &prevLat,
			 double &prevLon)
  
{
  
  // if icons are to be sparse, check min icon distance.
  // return early if distance from prev is less than the minimum.

  if (params.sparse_icons) {
    double r, theta;
    PJGLatLon2RTheta(loc.lat, loc.lon,
		     prevLat, prevLon,
		     &r, &theta);
    if (r < params.sparse_icons_min_distance) {
      return;
    }
  }
  prevLat = loc.lat;
  prevLon = loc.lon;

  // create the string

  static char alt_str[128];
  sprintf(alt_str, "%g", loc.alt);
  
  // add text

  prod.addText(alt_str, loc.lat, loc.lon,
	       color, "",
	       0, 0, 
	       Symprod::VERT_ALIGN_CENTER,
	       Symprod::HORIZ_ALIGN_CENTER,
	       0, Symprod::TEXT_NORM,
	       params.altitude_font);

}

//////////////////////////////////////////////////////////////////////
// add_polyline() - Add the polyline defined by the given
//                  aircraft positions.

void Server::_addPolyline(Symprod &prod,
			  const Params &params,
			  const vector<location_t> locations,
			  const int startIndex,
			  const int endIndex,
			  const char *color)

{

  if (endIndex <= startIndex) {
    return;
  }

  // load up polyline

  MemBuf ptBuf;
  int npts = 0;
  
  for (int ii = startIndex; ii <= endIndex; ii++) {
    Symprod::wpt_t vertex;
    vertex.lat = locations[ii].lat;
    vertex.lon = locations[ii].lon;
    // check for bad data (0, 0)
    if (fabs(vertex.lat) > 0.001 && fabs(vertex.lon) > 0.001) {
      ptBuf.add(&vertex, sizeof(vertex));
      npts++;
    }
  }
  
  // load up product

  prod.addPolyline(npts,
		   (Symprod::wpt_t *) ptBuf.getPtr(),
		   color,
		   (Symprod::linetype_t) Symprod::LINETYPE_SOLID,
		   params.polyline_width);

}

///////////////////////////////////////////////////////////////////////
// _addDirnArrow() - Add the dirn arrow for the given
//                   aircraft position.

void Server::_addDirnArrow(Symprod &prod,
			   const Params &params,
			   const vector<location_t> locations,
			   const int index,
			   const char *color)

{

  prod.addArrowBothPts(color,
		       Symprod::LINETYPE_SOLID,
		       params.polyline_width,
		       Symprod::CAPSTYLE_BUTT,
		       Symprod::JOINSTYLE_BEVEL,
		       locations[index-1].lat, locations[index-1].lon,
		       locations[index].lat, locations[index].lon,
		       params.dirn_arrow_head_len, 30.0);

}

//////////////////////////////////////////////////////////////////////
// _addCallsignLabel()
// Add the callsign as a label.

void Server::_addCallsignLabel(Symprod &prod,
			       const location_t &loc,
			       const char *color,
			       const char *font,
			       const Symprod::vert_align_t vert_align,
			       const Symprod::horiz_align_t horiz_align,
			       const int x_offset,
			       const int y_offset)
  
{
  prod.addText(loc.callsign.c_str(), loc.lat, loc.lon,
	       color, "",
	       x_offset, y_offset,
	       vert_align, horiz_align,
	       0, Symprod::TEXT_NORM, font);
}

/////////////////////////////////////////////////////////////////////////
// loadIconBuf() - Load an icon buffer for use in the SYMPROD object
//                 from the icon array as specified in the parameter
//                 file.

ui08 *Server::_loadIconBuf(int **param_icon,
			   int &param_icon_ny,
			   int &param_icon_nx,
			   MemBuf &iconBuf)
  
{

  iconBuf.free();
  
  // If the icon has no length, create a default one

  if (param_icon_ny == 0 || param_icon_nx == 0) {
    cerr << "WARNING - " << _executableName << "::Server::loadIconBuf" << endl;
    cerr << "  Icons should have at least 1 element.";
    ui08 element = 1;
    iconBuf.add(&element, sizeof(element));
    param_icon_ny = 1;
    param_icon_nx = 1;
    return (ui08 *) iconBuf.getPtr();
  }

  for (int ii = 0; ii < param_icon_ny; ii++) {
    for (int jj = 0; jj < param_icon_ny; jj++) {
      ui08 element = 0;
      if (param_icon[ii][jj]) {
	element = 1;
      }
      iconBuf.add(&element, sizeof(element));
    }
  }
  return (ui08 *) iconBuf.getPtr();

}

//////////////////////////////////////////////////////////////////
// altitudeColor()
//
// Returns the color for a given altitude. If
// there is no relevant color the default is
// returned.

const char *Server::_altitudeColor(const Params &params,
				   const double altitude,
				   const char *default_color)
  
{
  
  for (int i = 0; i < params.altitude_color_scale_n; i++) {
    
    if (params._altitude_color_scale[i].min_val <= altitude &&
	params._altitude_color_scale[i].max_val >= altitude) {
      return(params._altitude_color_scale[i].color);
    }
    
  } // i
  
  return (default_color);

}

//////////////////////////////////////////////////////////////////
// callsignColor()
//
// Returns the color for a given callsign. If
// there is no relevant color the default is
// returned.

const char *Server::_callsignColor(const Params &params,
				   const string &callsign,
				   const char *default_color)
  
{

  if (!params.color_by_callsign) {
    return default_color;
  }
  
  for (int i = 0; i < params.callsign_colors_n; i++) {

    string csign = params._callsign_colors[i].callsign;
    if (csign == callsign) {
      return(params._callsign_colors[i].color);
    }
    
  } // i
  
  return default_color;

}

/////////////////////////////////////////////
// return Symprod::vert_align_t from params t

Symprod::vert_align_t Server::_vertAlign(Params::label_vert_align_t align)

{

  switch (align) {
  case Params::VERT_ALIGN_TOP:
    return Symprod::VERT_ALIGN_TOP;
    break;
  case Params::VERT_ALIGN_CENTER:
    return Symprod::VERT_ALIGN_CENTER;
    break;
  case Params::VERT_ALIGN_BOTTOM:
    return Symprod::VERT_ALIGN_BOTTOM;
    break;
  default:
    return Symprod::VERT_ALIGN_CENTER;
  }

}

/////////////////////////////////////////////
// return Symprod::horiz_align_t from params t

Symprod::horiz_align_t Server::_horizAlign(Params::label_horiz_align_t align)

{

  switch (align) {
  case Params::HORIZ_ALIGN_LEFT:
    return Symprod::HORIZ_ALIGN_LEFT;
    break;
  case Params::HORIZ_ALIGN_CENTER:
    return Symprod::HORIZ_ALIGN_CENTER;
    break;
  case Params::HORIZ_ALIGN_RIGHT:
    return Symprod::HORIZ_ALIGN_RIGHT;
    break;
  default:
    return Symprod::HORIZ_ALIGN_CENTER;
  }

}

/////////////////////////////////////////////
// check spatial validity of position
//
// Returns true if spatially valid,
//         false if not

bool Server::_acceptPoint(double lat, double lon)

{

  if (!_horizLimitsSet) {
    return true;
  }

  if (lat < _minLat || lat > _maxLat) {
    return false;
  }

  if (_minLon <= lon && _maxLon >= lon) {
    return true;
  } else if (_minLon <= (lon + 360.0) &&
	     _maxLon >= (lon + 360.0)) {
    return true;
  } else if (_minLon <= (lon - 360.0) &&
	     _maxLon >= (lon - 360.0)) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////
// Add the time labels

void Server::_addTimeLabels(Symprod &prod,
                            const Params &params,
                            const vector<location_t> &track)

{

  // get start and end time of track
  
  time_t trackStartTime = track[0].time;
  time_t trackEndTime = track[track.size()-1].time;
  int intervalSecs = params.time_label_interval_secs;
  if (intervalSecs <= 0) {
    intervalSecs = 300;
  }
  time_t labelStartTime = (((trackStartTime / intervalSecs) + 1) * intervalSecs);

  // load up list of times to be plotted
  
  vector<location_t> labelLoc;
  time_t labelTime = labelStartTime;
  while (labelTime <= trackEndTime) {
    location_t loc;
    loc.time = labelTime;
    loc.lat = -9999.0;
    loc.lon = -9999.0;
    labelLoc.push_back(loc);
    labelTime += intervalSecs;
  }

  // compute location of times to be plotted

  for (size_t ii = 0; ii < labelLoc.size(); ii++) {

    for (size_t jj = 0; jj < track.size() - 1; jj++) {
      if (labelLoc[ii].time >= track[jj].time &&
          labelLoc[ii].time <= track[jj+1].time) {
        double frac = ((double) (labelLoc[ii].time - track[jj].time) /
                       (double) (track[jj+1].time - track[jj].time));
        labelLoc[ii].lat = track[jj].lat + frac * (track[jj+1].lat - track[jj].lat);
        labelLoc[ii].lon = track[jj].lon + frac * (track[jj+1].lon - track[jj].lon);
        break;
      }
    } // jj

  } // ii

  if (params.debug >= Params::DEBUG_NORM) {
    cerr << "================>> Adding time labels <<================" << endl;
    cerr << endl;
    cerr << "  trackStartTime: " << DateTime::strm(trackStartTime) << endl;
    cerr << "  trackEndTime: " << DateTime::strm(trackEndTime) << endl;
    cerr << "  intervalSecs: " <<  intervalSecs << endl;
    cerr << "  labelStartTime: " << DateTime::strm(labelStartTime) << endl;
    cerr << endl;
    cerr << "  list of label time and locations:"  << endl;
    for (size_t ii = 0; ii < labelLoc.size(); ii++) {
      cerr << "  time, lat, lon: "
           << DateTime::strm(labelLoc[ii].time) << ", "
           << labelLoc[ii].lat << ", "
           << labelLoc[ii].lon << endl;
    }
    cerr << "========================================================" << endl;
  }

  // Initialize icon

  MemBuf iconBuf;
  int iconNx = params.time_label_icon_n2;
  int iconNy = params.time_label_icon_n1;
  ui08 *icon = _loadIconBuf(params.__time_label_icon,
                            iconNy, iconNx, iconBuf);
  
  for (size_t ii = 0; ii < labelLoc.size(); ii++) {

    // add icon

    Symprod::wpt_t pos;
    pos.lat = labelLoc[ii].lat;
    pos.lon = labelLoc[ii].lon;
    prod.addBitmapIcons(params.time_label_color, 1, &pos,
			iconNx, iconNy, icon);

    // load up time label

    char label[1024];
    DateTime ltime(labelLoc[ii].time);
    switch (params.time_label_format) {
      case Params::TIME_LABEL_YYYY_MM_DD_HH_MM_SS:
        sprintf(label, "%.4d/%.2d/%.2d_%.2d:%.2d:%.2d",
                ltime.getYear(),
                ltime.getMonth(),
                ltime.getDay(),
                ltime.getHour(),
                ltime.getMin(),
                ltime.getSec());
        break;
      case Params::TIME_LABEL_YYYY_MM_DD_HH_MM:
        sprintf(label, "%.4d/%.2d/%.2d_%.2d:%.2d",
                ltime.getYear(),
                ltime.getMonth(),
                ltime.getDay(),
                ltime.getHour(),
                ltime.getMin());
        break;
      case Params::TIME_LABEL_HH_MM_SS:
        sprintf(label, "%.2d:%.2d:%.2d",
                ltime.getHour(),
                ltime.getMin(),
                ltime.getSec());
        break;
      case Params::TIME_LABEL_HH_MM:
        sprintf(label, "%.2d %.2d",
                ltime.getHour(),
                ltime.getMin());
        break;
    }

    // add text

    prod.addText(label, labelLoc[ii].lat, labelLoc[ii].lon,
                 params.time_label_color, "",
                 params.time_label_offset.x_offset,
                 params.time_label_offset.y_offset,
                 _vertAlign(params.label_vert_align),
                 _horizAlign(params.label_horiz_align),
                 0, Symprod::TEXT_NORM, params.label_font);

  }

}

