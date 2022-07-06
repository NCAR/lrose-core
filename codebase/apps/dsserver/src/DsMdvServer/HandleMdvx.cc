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
// HandleMdvx.cc
//
// Handle Mdvx requests for DsMdvServer Object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 1999
//
///////////////////////////////////////////////////////////////

#include "Params.hh"
#include "DsMdvServer.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxMsg.hh>
#include <Mdv/climo/ClimoFileFinder.hh>
#include <dsserver/DmapAccess.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/pjg.h>
using namespace std;

/////////////////////////////////////////////////////////
// Handle Mdvx commands
//
// Returns 0 on success, -1 on failure

int DsMdvServer::handleMdvxCommand(Socket *socket,
                                   const void *data,
                                   ssize_t dataSize)
{

  if (_isVerbose) {
    cerr << "Handling Mdvx command in DsMdvServer." << endl;
  }

  // disassemble message
  
  DsMdvx mdvx;
  DsMdvxMsg msg;
  if (_isVerbose) {
    msg.setDebug();
    mdvx.setDebug();
  }
  
  if (msg.disassemble(data, dataSize, mdvx)) {
    string errMsg  = "Error in DsMdvServer::handleMdvxCommand(): ";
    TaStr::AddStr(errMsg, "Could not disassemble message");
    TaStr::AddStr(errMsg, msg.getErrStr());
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
                                   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM - DsMdvServer - " << statusString << endl;
    }
    return 0;
  }

  // check security
  
  if (msg.getSubType() == DsMdvxMsg::MDVP_WRITE_TO_DIR ||
      msg.getSubType() == DsMdvxMsg::MDVP_WRITE_TO_PATH) {
    
    if (_isReadOnly) {
      cerr << "ERROR - DsMdvServer::handleDataCommand" << endl;
      cerr << "  Cannot write, running in read-only mode." << endl;
      cerr << "  URL: " << msg.getFirstURLStr() << endl;
      return -1;
    }

    if (_isSecure) {
      string urlStr = mdvx._outputUrl;
      string securityErr;
      if (!DsServerMsg::urlIsSecure(urlStr, securityErr)) {
        cerr << "ERROR - DsMdvServer::handleDataCommand" << endl;
        cerr << "  Running in secure mode." << endl;
        cerr << securityErr;
        cerr << "  URL: " << msg.getFirstURLStr() << endl;
        return -1;
      }
    }

  }

  // set valid time search wt if appropriate

  if (_params.set_valid_time_search_wt) {
    mdvx.setValidTimeSearchWt
      (_params.valid_time_search_wt);
  }

  // set constraints on lead times if appropriate

  if (_params.constrain_forecast_lead_times) {
    mdvx.setConstrainFcastLeadTimes
      (_params.forecast_constraints.min_lead_time,
       _params.forecast_constraints.max_lead_time,
       _params.forecast_constraints.request_by_gen_time);
  }

  // set encoding type if appropriate

  if (_params.override_encoding_type_on_read) {
    mdvx.setReadEncodingType((Mdvx::encoding_type_t) _params.encoding_type_on_read);
  }

  // override client settings, using local params and env vars

  _overrideClientSettings(mdvx);

  // print client details in debug mode

  if (_isDebug) {
    cerr << "CLIENT DETAILS" << endl;
    cerr << "  Client host: " << msg.getClientHost() << endl;
    cerr << "  Client ipaddr: " << msg.getClientIpaddr() << endl;
    cerr << "  Client user: " << msg.getClientUser() << endl;
  }

  // handle major actions
  
  int iret = 0;
  switch (msg.getSubType()) {
    
    case DsMdvxMsg::MDVP_READ_ALL_HDRS: {
      iret = _readMdvxAllHeaders(mdvx);
      if (iret) {
        msg.assembleErrorReturn(DsMdvxMsg::MDVP_READ_ALL_HDRS,
                                mdvx.getErrStr());
      } else {
        msg.assembleReadAllHdrsReturn(mdvx);
      }
      break;
    }
    
    case DsMdvxMsg::MDVP_READ_VOLUME: {
      iret = _readMdvxVolume(mdvx);
      if (iret) {
        msg.assembleErrorReturn(DsMdvxMsg::MDVP_READ_VOLUME,
                                mdvx.getErrStr(),
                                mdvx.getNoFilesFoundOnRead());
      } else {
        msg.assembleReadVolumeReturn(mdvx);
        _printHeadersDebug(mdvx);
      }
      break;
    }

    case DsMdvxMsg::MDVP_READ_VSECTION: {
      if (_params.serve_rhi_data) {
        iret = _readVsectionFromRhi(mdvx);
      } else {
        iret = _readMdvxVsection(mdvx);
      }
      if (iret) {
        msg.assembleErrorReturn(DsMdvxMsg::MDVP_READ_VSECTION,
                                mdvx.getErrStr(),
                                mdvx.getNoFilesFoundOnRead());
      } else {
        msg.assembleReadVsectionReturn(mdvx);
        _printHeadersDebug(mdvx);
      }
      break;
    }
    
    case DsMdvxMsg::MDVP_WRITE_TO_DIR: {
      iret = _writeMdvxToDir(mdvx);
      if (iret) {
        msg.assembleErrorReturn(DsMdvxMsg::MDVP_WRITE_TO_DIR,
                                mdvx.getErrStr());
      } else {
        msg.assembleWriteReturn(DsMdvxMsg::MDVP_WRITE_TO_DIR, mdvx);
      }
      break;
    }
    
    case DsMdvxMsg::MDVP_WRITE_TO_PATH: {
      iret = _writeMdvxToPath(mdvx);
      if (iret) {
        msg.assembleErrorReturn(DsMdvxMsg::MDVP_WRITE_TO_PATH,
                                mdvx.getErrStr());
      } else {
        msg.assembleWriteReturn(DsMdvxMsg::MDVP_WRITE_TO_PATH, mdvx);
      }
      break;
    }
    
    case DsMdvxMsg::MDVP_COMPILE_TIME_LIST: {
      if (_isDebug) {
        mdvx.printTimeListRequest(cerr);
      }
      string errorStr;
      iret = _compileMdvxTimeList(mdvx, errorStr);
      if (iret) {
        msg.assembleErrorReturn(DsMdvxMsg::MDVP_COMPILE_TIME_LIST,
                                errorStr,
                                mdvx.getNoFilesFoundOnRead());
      } else {
        msg.assembleCompileTimeListReturn(mdvx);
      }
      break;
    }
    
    case DsMdvxMsg::MDVP_COMPILE_TIME_HEIGHT: {
      if (_isDebug) {
        mdvx.printTimeHeightRequest(cerr);
      }
      string errorStr;
      iret = _compileMdvxTimeHeight(mdvx, errorStr);
      if (iret) {
        msg.assembleErrorReturn(DsMdvxMsg::MDVP_COMPILE_TIME_HEIGHT,
                                errorStr,
                                mdvx.getNoFilesFoundOnRead());
      } else {
        msg.assembleCompileTimeHeightReturn(mdvx);
      }
      break;
    }
    
  }

  if (iret && _isDebug) {
    cerr << "==================================" << endl;
    cerr << mdvx.getErrStr();
    cerr << "==================================" << endl;
  }
  
  // send reply

  void *msgToSend = msg.assembledMsg();
  ssize_t msgLen = msg.lengthAssembled();
  if (socket->writeMessage(0, msgToSend, msgLen)) {
    cerr << "ERROR - COMM -HandleMdvxCommand." << endl;
    cerr << "  Sending reply to client." << endl;
    cerr << socket->getErrStr() << endl;
  } else {
    if (_isDebug) {
      cerr << "SUCCESS - DsMdvServer sent reply to client" << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////
// setup read for headers, volume or vsection
//
// Returns the search time

time_t DsMdvServer::_setupRead(DsMdvx &mdvx, bool readVolume)
  
{

  // save the request time because it is
  // overwritten by setReadPath()
  
  time_t searchTime = mdvx._readSearchTime;
  
  if (_params.use_static_file) {
    mdvx.setReadPath(_params.static_file_url);
  } else if (_params.use_climatology_url) {
    string url = _getClimatologyUrl(mdvx);
    mdvx.setReadPath(url);
    if (_isDebug) {
      cerr << "Reading volume for climatology data" << endl;
    }
  }
  
  if (_params.fill_missing_regions) {
    mdvx.setReadFillMissing();
  }
  
  if (_params.auto_remap_to_latlon) {
    mdvx.setReadAutoRemap2LatLon();
    if (_isDebug) {
      cerr << "Setting auto latlon mode on" << endl;
    }
  }
  
  if (_params.create_composite_on_read && readVolume) {
    mdvx.clearReadVertLimits();
    mdvx.setReadComposite();
  }

  if (_params.decimate) {
    mdvx.setReadDecimate(_params.decimation_max_nxy);
  }

  if (_params.vsection_set_nsamples) {
    mdvx.setReadNVsectSamples(_params.vsection_nsamples);
  }

  if (_params.vsection_disable_interp) {
    mdvx.setReadVsectDisableInterp();
  }

  // vertical units
  
  if (_params.specify_vertical_units) {
    switch (_params.vertical_units) {
      case Params::HEIGHT_KM:
        mdvx.setReadVlevelType(Mdvx::VERT_TYPE_Z);
        break;
      case Params::PRESSURE_MB:
        mdvx.setReadVlevelType(Mdvx::VERT_TYPE_PRESSURE);
        break;
      case Params::FLIGHT_LEVEL:
        mdvx.setReadVlevelType(Mdvx::VERT_FLIGHT_LEVEL);
        break;
    }
  }

  return searchTime;

}

//////////////////////////////////////////////////////
// follow up on read for headers, volume or vsection

void DsMdvServer::_adjustTimes(DsMdvx &mdvx, time_t searchTime)
  
{

  if (_params.use_static_file) {
    mdvx._mhdr.time_begin = searchTime;
    mdvx._mhdr.time_end = searchTime;
    mdvx._mhdr.time_centroid = searchTime;
    mdvx._mhdrFile.time_begin = searchTime;
    mdvx._mhdrFile.time_end = searchTime;
    mdvx._mhdrFile.time_centroid = searchTime;
  }

  if (_params.use_climatology_url) {
    mdvx._mhdr.time_begin =
      _climoFileFinder->calcDataBeginTime(searchTime).utime();
    mdvx._mhdr.time_end =
      _climoFileFinder->calcDataEndTime(searchTime).utime();
    mdvx._mhdr.time_centroid = searchTime;
    mdvx._mhdrFile.time_begin =
      _climoFileFinder->calcDataBeginTime(searchTime).utime();
    mdvx._mhdrFile.time_end =
      _climoFileFinder->calcDataEndTime(searchTime).utime();
    mdvx._mhdrFile.time_centroid = searchTime;
  }

}

/////////////////////////////////////////////////////////
// Read all headers
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_readMdvxAllHeaders(DsMdvx &mdvx)

{

  // set up the read, saving the search time

  time_t searchTime = _setupRead(mdvx, false);

  if (_isDebug) {
    cerr << "READ_ALL_HDRS REQUEST" << endl;
    mdvx.printReadRequest(cerr);
  }

  int iret = -1;

  if (_params.serve_multiple_domains &&
      _params.domains_n > 0) {
    
    // multiple domains specified
    
    vector<string> urlList;
    
    if (mdvx._readHorizLimitsSet) {
      // limits set, search for domain
      _getUrlList(mdvx._readMinLat, mdvx._readMinLon,
                  mdvx._readMaxLat, mdvx._readMaxLon,
                  urlList);
    } else {
      // no limits set, use largest domain
      urlList.push_back(_params._domains[_params.domains_n - 1].url);
    }

    // loop through the URL list, breaking out on the first success

    for (size_t i = 0; i < urlList.size(); i++) {

      string &url = urlList[i];
      
      if (_isDebug) {
        cerr << "--->> _readMdvxAllHeaders, using url: " << url << endl;
      }
      
      if (mdvx._readTimeSet) {
        mdvx._readDirUrl = url;
      } else {
        mdvx._readPathUrl = url;
      }
      
      if (mdvx.readAllHeaders()) {
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i

  } else if (_params.use_failover_urls &&
             _params.failover_urls_n > 0) {

    vector<string> urlList;
    
    for (int k=0; k < _params.failover_urls_n; k++){
      urlList.push_back(_params._failover_urls[k]);
    }
    
    // loop through the URL list, breaking out on the first success
    
    for (size_t i = 0; i < urlList.size(); i++) {
      
      string &url = urlList[i];
      
      if (_isDebug) {
        cerr << "--->> _readMdvxAllHeaders, using url: " << url << endl;
      }
      
      if (mdvx._readTimeSet) {
        mdvx._readDirUrl = url;
      } else {
        mdvx._readPathUrl = url;
      }
      
      if (mdvx.readAllHeaders()) {
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i

  } else if (_params.handle_derived_fields) {
    
    if (_readDerivedAllHdrs(mdvx)) {
      iret = -1;
    } else {
      iret = 0;
    }

  } else {

    // single domain - use current URL

    if (mdvx.readAllHeaders()) {
      iret = -1;
    } else {
      iret = 0;
    }
    
  }

  if (iret == 0) {
    _adjustTimes(mdvx, searchTime);
    _overrideMasterHeaderInfoOnRead(mdvx);
  }

  return iret;

}

/////////////////////////////////////////////////////////
// Read volume
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_readMdvxVolume(DsMdvx &mdvx)

{

  // set up the read, saving the search time
  
  time_t searchTime = _setupRead(mdvx, true);

  if (_isDebug) {
    cerr << "READ_VOLUME REQUEST" << endl;
    mdvx.printReadRequest(cerr);
  }

  int iret = -1;
  
  if (_params.serve_multiple_domains &&
      _params.domains_n > 0) {

    // multiple domains specified

    vector<string> urlList;

    if (mdvx._readHorizLimitsSet) {
      // limits set, search for domain
      _getUrlList(mdvx._readMinLat, mdvx._readMinLon,
                  mdvx._readMaxLat, mdvx._readMaxLon,
                  urlList);
    } else {
      // no limits set, use largest domain
      urlList.push_back(_params._domains[_params.domains_n - 1].url);
    }

    // loop through the URL list, breaking out on the first success

    for (size_t i = 0; i < urlList.size(); i++) {

      string &url = urlList[i];
      
      if (_isDebug) {
        cerr << "--->> _readMdvxVolume, using url: " << url << endl;
      }

      if (mdvx._readTimeSet) {
        mdvx._readDirUrl = url;
      } else {
        mdvx._readPathUrl = url;
      }
      
      if (mdvx.readVolume()) {
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i

  } else if (_params.use_failover_urls &&
             _params.failover_urls_n > 0) {
    
    vector<string> urlList;
    
    for (int k=0; k < _params.failover_urls_n; k++){
      urlList.push_back(_params._failover_urls[k]);
    }
    
    // loop through the URL list, breaking out on the first success
    
    for (size_t i = 0; i < urlList.size(); i++) {
      
      string &url = urlList[i];
      
      if (_isDebug) {
        cerr << "--->> _readMdvxAddHeaders, using url: " << url << endl;
      }
      
      if (mdvx._readTimeSet) {
        mdvx._readDirUrl = url;
      } else {
        mdvx._readPathUrl = url;
      }
      
      if (mdvx.readVolume()) {
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i
    
  } else if (_params.handle_derived_fields &&
             mdvx._readFieldNames.size() > 0) {

    if (_handleDerivedFields(mdvx, READ_VOLUME)) {
      iret = -1;
    } else {
      iret = 0;
    }

  } else {
    
    if (mdvx.readVolume()) {
      iret = -1;
    } else {
      iret = 0;
    }
    
  }

  if (_params.create_composite_on_read) {
    mdvx.convertAllFields2Composite();
  }

  if (iret == 0) {
    _adjustTimes(mdvx, searchTime);
    _overrideMasterHeaderInfoOnRead(mdvx);
  }

  return iret;

}

/////////////////////////////////////////////////////////
// Read vert section
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_readMdvxVsection(DsMdvx &mdvx)

{

  // set up the read, saving the search time

  time_t searchTime = _setupRead(mdvx, false);

  if (_isDebug) {
    cerr << "READ_VSECTION REQUEST" << endl;
    mdvx.printReadRequest(cerr);
  }

  int iret = -1;

  if (_params.serve_multiple_domains &&
      _params.domains_n > 0) {
    
    // multiple domains specified
    
    // compute the bounding box

    double region_min_lat, region_min_lon;
    double region_max_lat, region_max_lon;
    
    _computeVsectionRegion(mdvx,
                           region_min_lat, region_min_lon,
                           region_max_lat, region_max_lon);
    
    // get the url list
    
    vector<string> urlList;
    
    _getUrlList(region_min_lat, region_min_lon,
                region_max_lat, region_max_lon,
                urlList);

    // loop through the URL list, breaking out on the first success
    
    for (size_t i = 0; i < urlList.size(); i++) {
      
      string &url = urlList[i];
      
      if (_isDebug) {
        cerr << "--->> _readMdvxVsection, using url: " << url << endl;
      }

      if (mdvx._readTimeSet) {
        mdvx._readDirUrl = url;
      } else {
        mdvx._readPathUrl = url;
      }
      
      if (mdvx.readVsection()) {
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i

  } else if (_params.use_failover_urls &&
             _params.failover_urls_n > 0) {

    vector<string> urlList;
    
    for (int k=0; k < _params.failover_urls_n; k++){
      urlList.push_back(_params._failover_urls[k]);
    }
    
    // loop through the URL list, breaking out on the first success
    
    for (size_t i = 0; i < urlList.size(); i++) {
      
      string &url = urlList[i];
      
      if (_isDebug) {
        cerr << "--->> _readMdvxVsection, using url: " << url << endl;
      }
      
      if (mdvx._readTimeSet) {
        mdvx._readDirUrl = url;
      } else {
        mdvx._readPathUrl = url;
      }
      
      if (mdvx.readVsection()) {
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i

  } else if (_params.handle_derived_fields &&
             mdvx._readFieldNames.size() > 0) {

    if (_handleDerivedFields(mdvx, READ_VSECTION)) {
      iret = -1;
    } else {
      iret = 0;
    }

  } else {

    // single domain - use current URL

    if (mdvx.readVsection()) {
      iret = -1;
    } else {
      iret = 0;
    }

  }

  if (iret == 0) {
    _adjustTimes(mdvx, searchTime);
    _overrideMasterHeaderInfoOnRead(mdvx);
  }

  return iret;

}

/////////////////////////////////////////////////////////
// Use RHI to respond to VSECTION Request
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_readVsectionFromRhi(DsMdvx &mdvx)

{

  // set up the read, saving the search time
  
  time_t searchTime = _setupRead(mdvx, false);
  
  if (_isDebug) {
    cerr << "READ_VSECTION REQUEST - AS RHI" << endl;
    mdvx.printReadRequest(cerr);
    cerr << "RHI url: " << _params.rhi_url << endl;
  }

  // save original read object
  
  DsMdvx origMdvx(mdvx);
  
  // try reading an RHI

  if (_readRhi(mdvx, searchTime) == 0) {
    return 0;
  }

  // no measured RHI data, revert to reconstructed vert section
  
  if (_isDebug) {
    cerr << "INFO - DsMdvServer::_readVsectionFromRhi" << endl;
    cerr << "  No RHI data found at url: " << mdvx._readDirUrl << endl;
    cerr << "  Time: " << DateTime::strm(searchTime) << endl;
    cerr << "  Using reconstructed vert section instead" << endl;
  }
  
  mdvx.clear();
  mdvx = origMdvx;
  return _readMdvxVsection(mdvx);

}

/////////////////////////////////////////////////////////
// Read RHI
// Returns 0 on success, -1 on failure

int DsMdvServer::_readRhi(DsMdvx &mdvx, time_t searchTime)

{

  // replace original URL with measured RHI URL
  
  string rhiUrl = _params.rhi_url;
  if (mdvx._readTimeSet) {
    mdvx._readDirUrl = rhiUrl;
  } else {
    mdvx._readPathUrl = rhiUrl;
  }

  // compile time list for RHIs
  
  DsMdvx tlist(mdvx);
  int margin = _params.rhi_time_margin;
  tlist.setTimeListModeValid(rhiUrl,
                             searchTime - margin,
                             searchTime + margin);

  if (tlist.compileTimeList()) {
    // no RHI data, do vert section
    return -1;
  }
  const vector<time_t> rhiTimes = tlist.getTimeList();
  if (rhiTimes.size() < 1) {
    return -1;
  }

  vector<RhiAz> azimuths;
  for (size_t ii = 0; ii < rhiTimes.size(); ii++) {
    time_t rhiTime = rhiTimes[ii];
    if (_readRhiAzimuths(rhiTime, rhiUrl, azimuths)) {
      cerr << "WARNING - DsMdvServer::_readRhi()" << endl;
      cerr << "  Cannot read RHI azimuths" << endl;
      cerr << "  URL: " << rhiUrl << endl;
      cerr << "  Time: " << DateTime::strm(rhiTime) << endl;
      return -1;
    }
  }

  if (_isDebug) {
    cerr << "========= RHI AZIMUTHS =========" << endl;
    for (size_t ii = 0; ii < azimuths.size(); ii++) {
      cerr << "  ii, time, angle: " << ii << ", " 
           << DateTime::strm(azimuths[ii].time) << ", " << azimuths[ii].angle << endl;
    }
    cerr << "================================" << endl;
  }
  
  // compute requested azimuth

  double requestedAz;
  if (_computeRequestedRhiAz(mdvx, requestedAz)) {
    if (_isDebug) {
      cerr << "INFO - DsMdvServer::_readRhi" << endl;
      cerr << "  Cannot compute good azimuth" << endl;
    }
    return -1;
  }

  if (_isDebug) {
    cerr << "========= RHI =========" << endl;
    cerr << "====> Requested az: " << requestedAz << endl;
  }
  
  // determine the best azimuth relative to the requested az

  double minDiff = 1.0e99;
  int bestIndex = 0;
  for (size_t ii = 0; ii < azimuths.size(); ii++) {
    double azDiff = fabs(azimuths[ii].angle - requestedAz);
    double timeDiff = fabs((double) azimuths[ii].time - (double) searchTime);
    double weightedDiff = azDiff + (timeDiff / margin) * 2.0;
    if (weightedDiff < minDiff) {
      bestIndex = ii;
      minDiff = weightedDiff;
    }
  }

  time_t bestTime = azimuths[bestIndex].time;
  double bestAz = azimuths[bestIndex].angle;
  double azError = fabs(bestAz - requestedAz);
  if (azError > 180) {
    azError = fabs(azError - 360.0);
  }

  if (_isDebug) {
    cerr << "====>> Best available az: " << bestAz << endl;
    cerr << "====>> Az error: " << azError << endl;
    cerr << "========= RHI =========" << endl;
  }

  if (azError > _params.rhi_max_az_error) {
    if (_isDebug) {
      cerr << "====>> Az error exceeds: " << _params.rhi_max_az_error << endl;
      cerr << "====>> Cannot use measured RHI" << endl;
    }
    return -1;
  }
  
  // get the RHI for that azimuth
  
  mdvx.setReadTime(Mdvx::READ_CLOSEST, _params.rhi_url, 0, bestTime);
  mdvx.setReadVsectAsRhi(_params.polar_rhi,
                         _params.rhi_max_az_error,
                         _params.respect_user_rhi_distance);
  mdvx.setReadVlevelLimits(bestAz, bestAz);

  if (mdvx.readVolume()) {
    return -1;
  }
  
  const MdvxField *fld0 = mdvx.getField(0);
  if (fld0 == NULL) {
    return -1;
  }
  
  if (_params.polar_rhi) {
    if (fld0->getFieldHeader().proj_type != Mdvx::PROJ_RHI_RADAR) {
      if (_isDebug) {
        cerr << "WARNING: _readRhi, url: " << rhiUrl << endl;
        cerr << "  params.polar_rhi is false" << endl;
        cerr << "  Data not in PROJ_RHI_RADAR" << endl;
        cerr << "  projType: " <<
          Mdvx::projType2Str(fld0->getFieldHeader().proj_type) << endl;
        cerr << "  Reverting to reconstructed RHI" << endl;
      }
      return -1;
    }
  } else {
    if (fld0->getFieldHeader().proj_type != Mdvx::PROJ_VSECTION) {
      if (_isDebug) {
        cerr << "WARNING: _readRhi, url: " << rhiUrl << endl;
        cerr << "  params.polar_rhi is false" << endl;
        cerr << "  Data not in PROJ_VSECTION" << endl;
        cerr << "  projType: " <<
          Mdvx::projType2Str(fld0->getFieldHeader().proj_type) << endl;
        cerr << "  Reverting to reconstructed RHI" << endl;
      }
      return -1;
    }
  }

  _finalizeRhiWaypts(mdvx, bestAz);
  _adjustTimes(mdvx, searchTime);
  _overrideMasterHeaderInfoOnRead(mdvx);

  return 0;

}

/////////////////////////////////////////////////////////
// Read azimuths for RHI
// Returns 0 on success, -1 on failure

int DsMdvServer::_readRhiAzimuths(time_t searchTime,
                                  const string &rhiUrl,
                                  vector<RhiAz> &azimuths)

{

  DsMdvx rhi;
  rhi.setReadTime(Mdvx::READ_CLOSEST, rhiUrl, 0, searchTime);
  if (rhi.readAllHeaders()) {
    cerr << rhi.getErrStr() << endl;
    return -1;
  }

  _rhiMhdr = rhi.getMasterHeaderFile();
  int nFields = _rhiMhdr.n_fields;
  if (nFields < 1) {
    return -1;
  }

  const Mdvx::field_header_t fhdr0 = rhi.getFieldHeaderFile(0);
  const Mdvx::vlevel_header_t vhdr0 = rhi.getVlevelHeaderFile(0);
  
  for (int ii = 0; ii < fhdr0.nz; ii++) {
    RhiAz rhiAz(searchTime, vhdr0.level[ii]);
    azimuths.push_back(rhiAz);
  }

  return 0;

}

//////////////////////////////////////////////////////////
// Compute requested RHI azimuth from user vsection request
// Returns 0 on success, -1 on failure

int DsMdvServer::_computeRequestedRhiAz(DsMdvx &mdvx, double &az)
  
{
  
  // For this to be a measured RHI, both the start and end point
  // must be on much the same azimuth relative to the radar.
  // See if this is the case.
  
  Mdvx::vsect_waypt_t radarPos;
  radarPos.lat = _rhiMhdr.sensor_lat;
  radarPos.lon = _rhiMhdr.sensor_lon;

  double d1, a1, d2, a2;
  PJGLatLon2RTheta(radarPos.lat, radarPos.lon,
		   mdvx._vsectWayPts[0].lat, mdvx._vsectWayPts[0].lon,
		   &d1, &a1);
  PJGLatLon2RTheta(radarPos.lat, radarPos.lon,
		   mdvx._vsectWayPts[1].lat, mdvx._vsectWayPts[1].lon,
		   &d2, &a2);

  if (d2 == d1) {
    return -1;
  }
  
  // if d1 is not close to the radar, this is not an RHI

  if (d1 > 20) {
    return -1;
  }
  if (d2 < d1){
    return -1;
  }

  // Get the azimuth between the points and the azimuth from the
  // radar to the end point and compare the two to se if they
  // are within tolerance.

  double azRadarToEndPoint = a2;
  double azPointToPoint, dummy;
  PJGLatLon2RTheta(mdvx._vsectWayPts[0].lat, mdvx._vsectWayPts[0].lon,
		   mdvx._vsectWayPts[1].lat, mdvx._vsectWayPts[1].lon,
		   &dummy, &azPointToPoint);
  
  double azDiff = fabs(azRadarToEndPoint - azPointToPoint);
  if (azDiff > 180.0) {
    azDiff = fabs(azDiff - 360.0);
  }
  
  if (azDiff > mdvx._readRhiMaxAzError) {
    return -1;
  }
  
  // If we got here, then the two entered points are at least on a direct
  // line from the radar. See if at least 80% of the line lies between
  // the start and the end of our measured RHI. If it does, assume
  // we can return a measured RHI.

  double maxRange = 150.0;
  double minRange = 0;

  // Return if there is no overlap at all
  if (d1 > maxRange) {
    return -1;
  }
  if (d2 < minRange) {
    return -1;
  }
  
  double outsideDist = 0;
  
  if (d1 < minRange) outsideDist += (minRange - d1);
  if (d2 > maxRange) outsideDist += (d2 - maxRange);

  double percentOutside = 100.0*outsideDist/(d2-d1);
  
  if (percentOutside > 80) {
    return -1;
  }

  // compute the azimuth of the mid-pt of the first 2 way-points
  
  Mdvx::vsect_waypt_t midPt;
  double dist, azimuth;
  if (mdvx._vsectWayPts.size() < 2) {
    midPt = mdvx._vsectWayPts[0];
  } else {
    PJGLatLon2RTheta(mdvx._vsectWayPts[0].lat, mdvx._vsectWayPts[0].lon,
                     mdvx._vsectWayPts[1].lat, mdvx._vsectWayPts[1].lon,
                     &dist, &azimuth);
    PJGLatLonPlusRTheta(mdvx._vsectWayPts[0].lat, mdvx._vsectWayPts[0].lon,
			dist / 2.0, azimuth,
			&midPt.lat, &midPt.lon);
  }
  
  // compute the azimuth of this mid point from the radar
  
  PJGLatLon2RTheta(_rhiMhdr.sensor_lat, _rhiMhdr.sensor_lon,
		   midPt.lat, midPt.lon,
		   &dist, &azimuth);
  if (azimuth < 0) {
    azimuth += 360.0;
  }

  az = azimuth;

  return 0;

}
  
//////////////////////////////////////////////////////////
// Finalize RHI waypoints

void DsMdvServer::_finalizeRhiWaypts(DsMdvx &mdvx, double az)
  
{
  
  // reset the waypoints, to go from the radar to the end of the RHI
  
  Mdvx::vsect_waypt_t radarPos;
  radarPos.lat = _rhiMhdr.sensor_lat;
  radarPos.lon = _rhiMhdr.sensor_lon;
  Mdvx::vsect_waypt_t endPos;

  const Mdvx::field_header_t &fhdr = mdvx._fields[0]->getFieldHeader();
  double maxRange = fhdr.grid_minx + fhdr.nx * fhdr.grid_dx;

  if (mdvx._readRhiRespectUserDist){
    
    double userDist, userAz;
    PJGLatLon2RTheta(radarPos.lat, radarPos.lon,
		     mdvx._vsectWayPts[1].lat, mdvx._vsectWayPts[1].lon,
		     &userDist, &userAz);
    
    // It's arguable that we should be doing something more like this,
    // but we'll leave that for later, since it requires trimming
    // out the RHI data properly - Niles and Mike.
    //
    //    PJGLatLon2RTheta(_vsectWayPts[0].lat, _vsectWayPts[0].lon,
    //                     _vsectWayPts[1].lat, _vsectWayPts[1].lon,
    //                     &userDist, &userAz);
    
    if (userDist > maxRange) userDist = maxRange;
    
    PJGLatLonPlusRTheta(radarPos.lat, radarPos.lon,
			userDist, az,
			&endPos.lat, &endPos.lon);
  } else {
    PJGLatLonPlusRTheta(radarPos.lat, radarPos.lon,
			maxRange, az,
			&endPos.lat, &endPos.lon);
  }

  mdvx._vsectWayPts.clear();
  mdvx._vsectWayPts.push_back(radarPos);
  mdvx._vsectWayPts.push_back(endPos);

  mdvx._mhdr.vlevel_type = Mdvx::VERT_TYPE_AZ;
  mdvx._mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mdvx._mhdr.data_ordering = Mdvx::ORDER_XZY;

  MdvxVsectLut lut;
  lut.computeSamplePts(mdvx._vsectWayPts, fhdr.nx);
  mdvx._vsectWayPts = lut.getWayPts();
  mdvx._vsectSamplePts = lut.getSamplePts();
  mdvx._vsectSegments = lut.getSegments();
  mdvx._vsectDxKm = lut.getDxKm();
  mdvx._vsectTotalLength = lut.getTotalLength();

  mdvx.updateMasterHeader();

}
  
/////////////////////////////////////////////////////////
// Read time list
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_compileMdvxTimeList(DsMdvx &mdvx,
                                      string &errorStr)

{

  errorStr = "ERROR - DsMdvServer::_compileMdvxTimeList\n";

  if (_params.use_static_file) {
    mdvx._timeList.clearData();
    time_t now = time(NULL);
    mdvx._timeList.addValidTime(now);
    return 0;
  }

  int iret = -1;

  if (_params.use_climatology_url) {
    vector< DateTime > climo_time_list =
      _climoFileFinder->calcTimeList(mdvx._timeList.getStartTime(),
                                     mdvx._timeList.getEndTime(),
                                     mdvx._timeList.getDir());
    
    mdvx._timeList.clearData();
    
    vector< DateTime >::const_iterator climo_time;
    for (climo_time = climo_time_list.begin();
         climo_time != climo_time_list.end(); ++climo_time)
      mdvx._timeList.addValidTime(climo_time->utime());
    
    iret = 0;
    
  } else if (_params.serve_multiple_domains &&
             _params.domains_n > 0) {
    
    // multiple domains specified
    
    vector<string> urlList;

    if (mdvx._readHorizLimitsSet) {
      // limits set, search for domain
      _getUrlList(mdvx._readMinLat, mdvx._readMinLon,
                  mdvx._readMaxLat, mdvx._readMaxLon,
                  urlList);
    } else {
      // no limits set, use largest domain
      urlList.push_back(_params._domains[_params.domains_n - 1].url);
    }

    // loop through the URL list, breaking out on the first success
    
    for (size_t i = 0; i < urlList.size(); i++) {
      
      string &url = urlList[i];
      
      if (_isDebug) {
        cerr << "--->> _compileMdvxTimeList, using url: " << url << endl;
      }
      
      mdvx._timeListUrl = url;
    
      if (mdvx.compileTimeList()) {
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i
      
  } else if (_params.use_failover_urls &&
             _params.failover_urls_n > 0) {

    vector<string> urlList;
    
    for (int k=0; k < _params.failover_urls_n; k++){
      urlList.push_back(_params._failover_urls[k]);
    }
    
    // loop through the URL list, breaking out on the first success
    
    for (size_t i = 0; i < urlList.size(); i++) {
      
      string &url = urlList[i];
      
      if (_isDebug) {
        cerr << "--->> _compileMdvxTimeList, using url: " << url << endl;
      }

      mdvx._timeListUrl = url;
    
      if (mdvx.compileTimeList()) {
        iret = -1;
      } else {
        iret = 0;
        break;
      }
      
    } // i

  } else {
    
    // single domain - use current URL
    
    if (mdvx.compileTimeList()) {
      iret = -1;
    } else {
      iret = 0;
    }
    
  }

  if (iret == 0) {
    _overrideMasterHeaderInfoOnRead(mdvx);
  }

  errorStr += mdvx.getErrStr();
  return iret;

}

/////////////////////////////////////////////////////////
// Compile time height profile
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_compileMdvxTimeHeight(DsMdvx &mdvx,
                                        string &errorStr)
  
{

  errorStr = "ERROR - DsMdvServer::_compileMdvxTimeHeight\n";

  int iret = -1;

  // set the basic read properties
  
  _setBasicReadProperties(mdvx);
  
  if (_params.use_static_file) {

    mdvx._timeList.clearData();
    time_t now = time(NULL);
    mdvx._timeList.addValidTime(now);
    if (mdvx._compileTimeHeight()) {
      errorStr += mdvx.getErrStr();
      errorStr += "\n";
      iret = -1;
    } else {
      iret = 0;
    }
    
  } else if (_params.serve_multiple_domains &&
             _params.domains_n > 0) {
    
    // multiple domains specified
    
    vector<string> urlList;

    if (mdvx._vsectWayPts.size() > 0) {
      double lat = mdvx._vsectWayPts[0].lat;
      double lon = mdvx._vsectWayPts[0].lon;
      _getUrlList(lat, lon, lat, lon,
                  urlList);
    } else {
      // no limits set, use largest domain
      urlList.push_back(_params._domains[_params.domains_n - 1].url);
    }
    
    // loop through the URL list, breaking out on the first success
    
    for (size_t i = 0; i < urlList.size(); i++) {
      string &url = urlList[i];
      if (_isDebug) {
        cerr << "--->> _compileMdvxTimeHeight, using url: " << url << endl;
      }
      mdvx._timeListUrl = url;
      if (mdvx.compileTimeHeight()) {
        errorStr += mdvx.getErrStr();
        errorStr += "\n";
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i
      
  } else if (_params.use_failover_urls &&
             _params.failover_urls_n > 0) {

    vector<string> urlList;
    
    for (int k=0; k < _params.failover_urls_n; k++){
      urlList.push_back(_params._failover_urls[k]);
    }
    
    // loop through the URL list, breaking out on the first success
    
    for (size_t i = 0; i < urlList.size(); i++) {
      string &url = urlList[i];
      if (_isDebug) {
        cerr << "--->> _compileMdvxTimeHeight, using url: " << url << endl;
      }
      mdvx._timeListUrl = url;
      if (mdvx.compileTimeHeight()) {
        errorStr += mdvx.getErrStr();
        errorStr += "\n";
        iret = -1;
      } else {
        iret = 0;
        break;
      }

    } // i

  } else if (_params.handle_derived_fields &&
             mdvx._readFieldNames.size() > 0) {
    
    if (_compileDerivedTimeHeight(mdvx)) {
      iret = -1;
    } else {
      iret = 0;
    }

  } else {
    
    // single domain - use current URL
    
    if (mdvx.compileTimeHeight()) {
      errorStr += mdvx.getErrStr();
      errorStr += "\n";
      iret = -1;
    } else {
      iret = 0;
    }
    
  }

  if (iret == 0) {
    _overrideMasterHeaderInfoOnRead(mdvx);
  }

  return iret;

}

/////////////////////////////////////////////////////////
// Write to specified directory
// Returns 0 on success, -1 on failure

int DsMdvServer::_writeMdvxToDir(DsMdvx &mdvx)
  
{
  
  if (_isDebug) {
    cerr << "WRITE TO DIR" << endl;
    mdvx.printWriteOptions(cerr);
  }
  
  DsURL url(mdvx._outputUrl);
  string writeDir = url.getFile();

  int iret = mdvx.writeToDir(writeDir);

  if (iret == 0) {

    if (_isDebug) {
      cerr << "Wrote to dir: " << writeDir << endl;
      cerr << "Wrote to path: " << mdvx.getPathInUse() << endl;
    }

    // reg with data mapper
    
    time_t latestTime;
    if (mdvx.getWriteAsForecast()) {
      latestTime = mdvx.getMasterHeader().time_gen;
    } else {
      latestTime = mdvx.getMasterHeader().time_centroid;
    }
    DmapAccess dmap;
    dmap.regLatestInfo(latestTime, writeDir);
    
  }

  string mainPathInUse = mdvx.getPathInUse();
  if (_params.forward_on_write) {
    for (int ii = 0; ii < _params.forward_on_write_urls_n; ii++) {
      string url = _params._forward_on_write_urls[ii];
      if (_isDebug) {
        cerr << "Forwarding on write ->> URL: " << url << endl;
      }
      if (mdvx.writeToDir(url)) {
        iret = -1;
        break;
      }
    }
  }
  mdvx._pathInUse = mainPathInUse;

  return iret;

}

/////////////////////////////////////////////////////////
// Write to specified path
// Returns 0 on success, -1 on failure

int DsMdvServer::_writeMdvxToPath(DsMdvx &mdvx)
  
{
  
  if (_isDebug) {
    cerr << "WRITE TO PATH" << endl;
    mdvx.printWriteOptions(cerr);
  }
  
  DsURL url(mdvx._outputUrl);
  string writePath = url.getFile();

  int iret = mdvx.writeToPath(writePath);

  if (iret == 0) {
    
    if (_isDebug) {
      cerr << "Writing to path: " << writePath << endl;
    }

  }

  return iret;

}

///////////////////////////////////////////////////////////
// print headers for debug purposes

void DsMdvServer::_printHeadersDebug(const DsMdvx &mdvx) const
  
{
  
  if (_isVerbose && mdvx._internalFormat == Mdvx::FORMAT_MDV) {
    cerr << "!!!!!!!!!!!! START FILE HEADERS !!!!!!!!!!!!!!!!!!!!!" << endl;
    cerr << "!!!!! The following headers are as in the file" << endl;
    mdvx.printAllFileHeaders(cerr);
    cerr << "!!!!! The above headers are as in the file" << endl;
    cerr << "!!!!!!!!!!!! END FILE HEADERS !!!!!!!!!!!!!!!!!!!!!" << endl;
    cerr << endl;
    cerr << "!!!!!!!!!!! START RETURNED HEADERS !!!!!!!!!!!!!!!!!" << endl;
    cerr << "!!!!! The following headers are as returned to client" << endl;
    mdvx.printAllHeaders(cerr);
    cerr << "!!!!! The above headers are as returned to client" << endl;
    cerr << "!!!!!!!!!!!! END RETURNED HEADERS !!!!!!!!!!!!!!!!!" << endl;
    cerr.flush();
  }

}

///////////////////////////////////////////////////////////
// Override client settings on the mdvx object

void DsMdvServer::_overrideClientSettings(DsMdvx &mdvx)
  
{
  
  // write format

  if (_params.override_write_format) {
    if (_params.write_format == Params::FORMAT_MDV) {
      mdvx.setWriteFormat(Mdvx::FORMAT_MDV);
    } else if (_params.write_format == Params::FORMAT_XML) {
      mdvx.setWriteFormat(Mdvx::FORMAT_XML);
    } else if (_params.write_format == Params::FORMAT_NCF) {
      mdvx.setWriteFormat(Mdvx::FORMAT_NCF);
    }
    if (_isDebug) {
      cerr << "Overriding write format from param file: "
           << Mdvx::format2Str(mdvx._writeFormat) << endl;
    }
  }
  
  char *writeFormatStr = getenv("MDV_WRITE_FORMAT");
  if (writeFormatStr != NULL) {
    if (!strcmp(writeFormatStr, "FORMAT_MDV")) {
      mdvx.setWriteFormat(Mdvx::FORMAT_MDV);
    } else if (!strcmp(writeFormatStr, "FORMAT_XML")) {
      mdvx.setWriteFormat(Mdvx::FORMAT_XML);
    } else if (!strcmp(writeFormatStr, "FORMAT_NCF")) {
      mdvx.setWriteFormat(Mdvx::FORMAT_NCF);
    }
  }

  // write in forecast style?

  if (_params.write_as_forecast){
    mdvx.setWriteAsForecast();
  }
  
  if (_params.if_forecast_write_as_forecast){
    mdvx.setIfForecastWriteAsForecast();
  }

  // write using extended paths?

  if (_params.write_using_extended_paths) {
    mdvx._useExtendedPaths = true;
  }

  // netCDF MDV to NCF

  if (_params.control_mdv2ncf) {

    mdvx.clearMdv2Ncf();

    mdvx.setMdv2NcfAttr(_params.ncf_global_attributes.institution,
                        _params.ncf_global_attributes.references,
                        _params.ncf_global_attributes.comment);

    for (int ii = 0; ii < _params.mdv2ncf_field_transforms_n; ii++) {
      const Params::mdv2ncf_field_transform_t &trans =
        _params._mdv2ncf_field_transforms[ii];
      DsMdvx::ncf_pack_t packing = DsMdvx::NCF_PACK_ASIS;
      if (trans.packed_data_type == Params::DATA_PACK_FLOAT) {
        packing = DsMdvx::NCF_PACK_FLOAT;
      } else if (trans.packed_data_type == Params::DATA_PACK_BYTE) {
        packing = DsMdvx::NCF_PACK_BYTE;
      } else if (trans.packed_data_type == Params::DATA_PACK_SHORT) {
        packing = DsMdvx::NCF_PACK_SHORT;
      }
      mdvx.addMdv2NcfTrans(trans.mdv_field_name,
                           trans.ncf_field_name,
                           trans.ncf_standard_name,
                           trans.ncf_long_name,
                           trans.ncf_units,
                           trans.do_linear_transform,
                           trans.linear_multiplier,
                           trans.linear_const,
                           packing);
    } // ii

    if (_params.ncf_file_format == Params::CLASSIC) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_CLASSIC);
    } else if (_params.ncf_file_format == Params::NC64BIT) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_OFFSET64BITS);
    } else if  (_params.ncf_file_format == Params::NETCDF4_CLASSIC) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCFD4_CLASSIC);
    } else {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCDF4);
    }
    
    mdvx.setMdv2NcfCompression(_params.ncf_compress_data,
                               _params.ncf_compression_level);
    
    mdvx.setMdv2NcfOutput(_params.ncf_output_latlon_arrays,
                          _params.ncf_output_mdv_attributes,
                          _params.ncf_output_mdv_chunks,
                          _params.ncf_output_start_end_times);
    
    mdvx.setNcfFileSuffix(_params.ncf_filename_suffix);

  } // if (_params.control_mdv2ncf)

}


/////////////////////////////////////////////////////////
// Override data set info in master header, if appropriate

void DsMdvServer::_overrideMasterHeaderInfoOnRead(DsMdvx &mdvx)

{

  if (_params.override_data_set_name_on_read) {
    mdvx.setDataSetName(_params.data_set_name_read_xml);
  }

  if (_params.override_data_set_source_on_read) {
    mdvx.setDataSetSource(_params.data_set_source_read_xml);
  }

  if (_params.override_data_set_info_on_read) {
    mdvx.setDataSetInfo(_params.data_set_info_read_xml);
  }

}


////////////////////////////////////////////////////////////////////////////
// get the url for the correct climatology file
//
// Returns the climatology directory if the URL couldn't be constructed for
// some reason.

string DsMdvServer::_getClimatologyUrl(
        DsMdvx &mdvx) const
{
  string climo_full_url = mdvx._readDirUrl;
  
  DateTime climo_time =
    _climoFileFinder->calcClimoTime(mdvx._readSearchTime);
    
  char *full_path = new char[MAX_PATH_LEN];
  sprintf(full_path, "%s/%04d%02d%02d/%02d%02d%02d.mdv",
          mdvx._readDirUrl.c_str(),
          climo_time.getYear(), climo_time.getMonth(),
          climo_time.getDay(),
          climo_time.getHour(), climo_time.getMin(),
          climo_time.getSec());
  climo_full_url = full_path;
  delete [] full_path;
      
  if (_isDebug)
    cerr << "Found closest climatology URL: " << climo_full_url << endl;
    
  return climo_full_url;
}


////////////////////////////////////////////////////////////////////////////
// get the url for the correct domain
//
// The domains are in ascending size order. Therefore we search through all
// but the last one, to see if the region fits within the domain. If it
// does we return the url for this domain.
//
// If the above fails, we return the URL for the largest domain in the list,
// i.e. the last one.

string DsMdvServer::_getDomainUrl(double region_min_lat, double region_min_lon,
                                  double region_max_lat, double region_max_lon)

{

  for (int i = 0; i < _params.domains_n - 1; i++) {
    if (_withinDomain(region_min_lat, region_min_lon,
                      region_max_lat, region_max_lon,
                      _params._domains[i].min_lat,
                      _params._domains[i].min_lon,
                      _params._domains[i].max_lat,
                      _params._domains[i].max_lon)) {
      return (_params._domains[i].url);
    }
  }

  return (_params._domains[_params.domains_n - 1].url);

}


////////////////////////////////////////////////////////////////////////////
// get the url for the correct domain
//
// The domains are in ascending size order. Therefore we search through all
// but the last one, to see if the region fits within the domain. If it
// does we find the url for this domain.
//
// If the above fails, we find the URL for the largest domain in the list,
// i.e. the last one.
//
// Then we add to the list first the domains getting larger than the ideal,
// and then the domains getting smaller, so that we can try them if the
// main domain fails.

void DsMdvServer::_getUrlList(double region_min_lat, double region_min_lon,
                              double region_max_lat, double region_max_lon,
                              vector<string> &urlList)
  
{
  
  int nd = _params.domains_n;
  int domainNum = nd - 1;
  
  for (int i = 0; i < nd - 1; i++) {
    if (_withinDomain(region_min_lat, region_min_lon,
                      region_max_lat, region_max_lon,
                      _params._domains[i].min_lat,
                      _params._domains[i].min_lon,
                      _params._domains[i].max_lat,
                      _params._domains[i].max_lon)) {
      domainNum = i;
      break;
    }
  }

  urlList.erase(urlList.begin(), urlList.end());
  urlList.push_back(_params._domains[domainNum].url);

  // if no auto fail over, we only want a single domain in the list

  if (!_params.auto_fail_over) {
    return;
  }
  
  // if auto fail over, add the other domains to the list, first moving
  // out and then moving in

  for (int i = domainNum + 1; i < nd; i++) {
    urlList.push_back(_params._domains[i].url);
  }

  for (int i = domainNum - 1; i > 0; i--) {
    urlList.push_back(_params._domains[i].url);
  }

  return;

}


////////////////////////////////////////////////////////////////////////
// Check whether the given region is within the domain.
// This function takes into account the fact that the two frames of
// reference may be offest by 360 degrees.

bool DsMdvServer::_withinDomain(double region_min_lat, double region_min_lon,
                                double region_max_lat, double region_max_lon,
                                double domain_min_lat, double domain_min_lon,
                                double domain_max_lat, double domain_max_lon)

{
  
  // check as is

  if (region_min_lat >= domain_min_lat &&
      region_min_lon >= domain_min_lon &&
      region_max_lat <= domain_max_lat &&
      region_max_lon <= domain_max_lon) {
    return true;
  }

  // check with -360 offset

  if ((region_min_lat - 360.0) >= domain_min_lat &&
      (region_min_lon - 360.0) >= domain_min_lon &&
      (region_max_lat - 360.0) <= domain_max_lat &&
      (region_max_lon - 360.0) <= domain_max_lon) {
    return true;
  }

  // check with +360 offset

  if ((region_min_lat + 360.0) >= domain_min_lat &&
      (region_min_lon + 360.0) >= domain_min_lon &&
      (region_max_lat + 360.0) <= domain_max_lat &&
      (region_max_lon + 360.0) <= domain_max_lon) {
    return true;
  }

  return false;

}

////////////////////////////////////////////////////////////////
// compute the bounding region for the vert section

void DsMdvServer::_computeVsectionRegion(DsMdvx &mdvx,
                                         double &region_min_lat,
                                         double &region_min_lon,
                                         double &region_max_lat,
                                         double &region_max_lon)

{

  region_min_lat = 90.0;
  region_min_lon = 180.0;
  region_max_lat = -90.0;
  region_max_lon = -180.0;
  
  for (size_t i = 0; i < mdvx._vsectWayPts.size(); i++) {
    region_min_lat = MIN(region_min_lat, mdvx._vsectWayPts[i].lat);
    region_min_lon = MIN(region_min_lon, mdvx._vsectWayPts[i].lon);
    region_max_lat = MAX(region_max_lat, mdvx._vsectWayPts[i].lat);
    region_max_lon = MAX(region_max_lon, mdvx._vsectWayPts[i].lon);
  }

}

////////////////////////////////////////////////////////////////
// set basic read properties

void DsMdvServer::_setBasicReadProperties(DsMdvx &mdvx)

{
  
  if (_params.use_static_file) {
    mdvx.setReadPath(_params.static_file_url);
  } else if (_params.use_climatology_url) {
    string url = _getClimatologyUrl(mdvx);
    mdvx.setReadPath(url);
    if (_isDebug) {
      cerr << "Setting basic read properties for climatology data" << endl;
    }
  }
  
  if (_params.fill_missing_regions) {
    mdvx.setReadFillMissing();
  }
  
  if (_params.auto_remap_to_latlon) {
    mdvx.setReadAutoRemap2LatLon();
    if (_isDebug) {
      cerr << "Setting auto latlon mode on" << endl;
    }
  }
  
  if (_params.decimate) {
    mdvx.setReadDecimate(_params.decimation_max_nxy);
  }
  
  // vertical units
  
  if (_params.specify_vertical_units) {
    switch (_params.vertical_units) {
      case Params::HEIGHT_KM:
        mdvx.setReadVlevelType(Mdvx::VERT_TYPE_Z);
        break;
      case Params::PRESSURE_MB:
        mdvx.setReadVlevelType(Mdvx::VERT_TYPE_PRESSURE);
        break;
      case Params::FLIGHT_LEVEL:
        mdvx.setReadVlevelType(Mdvx::VERT_FLIGHT_LEVEL);
        break;
    }
  }

}


