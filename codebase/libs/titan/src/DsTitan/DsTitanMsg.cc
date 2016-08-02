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
// DsTitanMsg.cc
//
// DsTitanMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2001
//
///////////////////////////////////////////////////////////////
//
// The DsTitanMsg object provides the message protocol for
// the DsTitan service.
//
///////////////////////////////////////////////////////////////


#include <titan/DsTitanMsg.hh>
#include <didss/DsMsgPart.hh>
#include <toolsa/TaStr.hh>
#include <dataport/bigend.h>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

DsTitanMsg::DsTitanMsg() : DsServerMsg(CopyMem)

{

}

////////////////////////////////////////////////////////////
// destructor

DsTitanMsg::~DsTitanMsg()

{

}

///////////////////////////////////////////////
// assemble a DS_SPDB_PUT message
//
  
void *DsTitanMsg::assembleReadRequest(const string &url,
				      const DsTitan &tserver)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_TITAN, DS_TITAN_READ_REQUEST);
  
  // indicate that this is the start of a series of get messages
  
  setCategory(StartGet);
  
  // load up read request struct, and swap

  read_request_t request;
  MEM_zero(request);
  request.readTimeMode = tserver._readTimeMode;
  request.trackSet = tserver._trackSet;
  request.requestTime = tserver._requestTime;
  request.readTimeMargin = tserver._readTimeMargin;
  request.requestComplexNum = tserver._requestComplexNum;
  request.readLprops = tserver._readLprops;
  request.readDbzHist = tserver._readDbzHist;
  request.readRuns = tserver._readRuns;
  request.readProjRuns = tserver._readProjRuns;
  request.startTime = tserver._startTime;
  request.endTime = tserver._endTime;
  request.readCompressed = tserver._readCompressed;
  BE_from_array_32(&request, sizeof(request));

  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_TITAN_READ_URL_PART, url.size() + 1, url.c_str());
  addPart(DS_TITAN_READ_REQUEST_PART, sizeof(request), &request);
  
  // assemble
  
  return (DsMessage::assemble());

}

///////////////////////////////////////////////
// assemble read reply
//

void *DsTitanMsg::assembleReadReplySuccess(const DsTitan &tserver)
  
{
  
  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_TITAN, DS_TITAN_READ_REPLY);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // load up reply struct and swap

  read_reply_t reply;
  MEM_zero(reply);
  reply.nComplexTracks = tserver._complexTracks.size();
  reply.nCurrentEntries = tserver._currentEntries.size();
  reply.scanInUse = tserver._scanInUse;
  reply.timeInUse = tserver._timeInUse;
  reply.dataStartTime = tserver._dataStartTime;
  reply.dataEndTime = tserver._dataEndTime;
  reply.idayInUse = tserver._idayInUse;
  BE_from_array_32(&reply, sizeof(reply));

  // make copies of storm and track params and swap

  storm_file_params_t sparams = tserver._stormFileParams;
  BE_from_array_32(&sparams, sizeof(sparams));
  track_file_params_t tparams = tserver._trackFileParams;
  BE_from_array_32(&tparams, sizeof(tparams));

  // clear message parts

  clearParts();

  // add parts
  
  addPart(DS_TITAN_READ_REPLY_PART, sizeof(reply), &reply);
  if (tserver._readTimeMode != TITAN_SERVER_READ_LATEST_TIME) {
    addPart(DS_TITAN_STORM_PARAMS_PART, sizeof(sparams), &sparams);
    addPart(DS_TITAN_TRACK_PARAMS_PART, sizeof(tparams), &tparams);
    addPart(DS_TITAN_DIR_IN_USE_PART,
	    tserver._dirInUse.size() + 1,
	    tserver._dirInUse.c_str());
    addPart(DS_TITAN_STORM_PATH_IN_USE_PART,
	    tserver._stormPathInUse.size() + 1,
	    tserver._stormPathInUse.c_str());
    addPart(DS_TITAN_TRACK_PATH_IN_USE_PART,
	    tserver._trackPathInUse.size() + 1,
	    tserver._trackPathInUse.c_str());
  }

  // add complex tracks - one part for each

  MemBuf buf;
  for (size_t ii = 0; ii < tserver._complexTracks.size(); ii++) {
    tserver._complexTracks[ii]->assemble(buf, true);
    addPart(DS_TITAN_COMPLEX_TRACK_PART, buf.getLen(), buf.getPtr());
  }

  // add current entries - one part for all entries

  if (tserver._currentEntries.size() > 0) {
    buf.free();
    for (size_t ii = 0; ii < tserver._currentEntries.size(); ii++) {
      tserver._currentEntries[ii]->assemble(buf);
    }
    addPart(DS_TITAN_CURRENT_ENTRIES_PART, buf.getLen(), buf.getPtr());
  }

  // assemble
  
  return (DsMessage::assemble());

}

////////////////////////
// assemble error return
//

void *DsTitanMsg::assembleReadReplyError(const DsTitan &tserver)

{

  // set header attributes
  
  setHdrAttr(DS_MESSAGE_TYPE_TITAN, DS_TITAN_READ_REPLY);

  // indicate that this is an error

  setError(true);
  
  // indicate that this is the end of a series of messages
  
  setCategory(EndSeries);
  
  // clear message parts
  
  clearParts();

  // add parts
  
  if (tserver._errStr.size() > 0) {
    addPart(DS_TITAN_ERR_STRING_PART,
	    tserver._errStr.size() + 1, tserver._errStr.c_str());
  }

  // assemble
  
  return (DsMessage::assemble());

}

/////////////////////////////////////
// override the disassemble function
//
// Load up DsTitan object
//

int DsTitanMsg::disassemble(const void *in_msg, const int msg_len,
			    DsTitan &tserver)

{

  _errStr = "ERROR - DsTitanMsg::disassemble\n";

  // peek at the header to make sure we're looking at the
  // right type of message

  if (decodeHeader(in_msg, msg_len)) {
    TaStr::AddStr(_errStr, "  ", "Bad message header");
    TaStr::AddInt(_errStr, "  Messsage len: ", msg_len);
    return (-1);
  }

  if (_type != DS_MESSAGE_TYPE_TITAN) {
    TaStr::AddInt(_errStr, "  Unknown message type: ", _type);
    TaStr::AddInt(_errStr, "  Messsage len: ", msg_len);
    return (-1);
  }

  // disassemble the message using the base-class routine
  
  if (DsMessage::disassemble(in_msg, msg_len)) {
    TaStr::AddStr(_errStr, "  ", "ERROR in DsMessage::disassemble()");
    return (-1);
  }

  // set tserver data members to parts

  // strings

  if (partExists(DS_TITAN_READ_URL_PART)) {
    tserver._urlInUse =
      (char *) getPartByType(DS_TITAN_READ_URL_PART)->getBuf();
  }

  if (partExists(DS_TITAN_ERR_STRING_PART)) {
    tserver._errStr +=
      (char *) getPartByType(DS_TITAN_ERR_STRING_PART)->getBuf();
  }

  if (partExists(DS_TITAN_DIR_IN_USE_PART)) {
    tserver._dirInUse =
      (char *) getPartByType(DS_TITAN_DIR_IN_USE_PART)->getBuf();
  }

  if (partExists(DS_TITAN_STORM_PATH_IN_USE_PART)) {
    tserver._stormPathInUse =
      (char *) getPartByType(DS_TITAN_STORM_PATH_IN_USE_PART)->getBuf();
  }

  if (partExists(DS_TITAN_TRACK_PATH_IN_USE_PART)) {
    tserver._trackPathInUse =
      (char *) getPartByType(DS_TITAN_TRACK_PATH_IN_USE_PART)->getBuf();
  }

  // storm and track params

  if (partExists(DS_TITAN_STORM_PARAMS_PART)) {
    memcpy(&tserver._stormFileParams,
	   getPartByType(DS_TITAN_STORM_PARAMS_PART)->getBuf(),
	   sizeof(storm_file_params_t));
    BE_to_array_32(&tserver._stormFileParams, sizeof(storm_file_params_t));
  }
  if (partExists(DS_TITAN_TRACK_PARAMS_PART)) {
    memcpy(&tserver._trackFileParams,
	   getPartByType(DS_TITAN_TRACK_PARAMS_PART)->getBuf(),
	   sizeof(track_file_params_t));
    BE_to_array_32(&tserver._trackFileParams, sizeof(track_file_params_t));
  }

  // read request

  if (partExists(DS_TITAN_READ_REQUEST_PART)) {
    read_request_t request;
    memcpy(&request,
	   getPartByType(DS_TITAN_READ_REQUEST_PART)->getBuf(),
	   sizeof(request));
    BE_to_array_32(&request, sizeof(request));
    tserver._readTimeMode = (tserver_read_time_mode_t) request.readTimeMode;
    tserver._trackSet = (tserver_track_set_t) request.trackSet;
    tserver._requestTime = request.requestTime;
    tserver._readTimeMargin = request.readTimeMargin;
    tserver._requestComplexNum = request.requestComplexNum;
    tserver._readLprops = request.readLprops;
    tserver._readDbzHist = request.readDbzHist;
    tserver._readRuns = request.readRuns;
    tserver._readProjRuns = request.readProjRuns;
    tserver._startTime = request.startTime;
    tserver._endTime = request.endTime;
    tserver._readCompressed = request.readCompressed;
  }
  
  // read reply if it exists

  if (partExists(DS_TITAN_READ_REPLY_PART)) {

    // reply struct
    
    read_reply_t reply;
    memcpy(&reply,
	   getPartByType(DS_TITAN_READ_REPLY_PART)->getBuf(),
	   sizeof(reply));
    BE_to_array_32(&reply, sizeof(reply));
    tserver._scanInUse = reply.scanInUse;
    tserver._timeInUse = reply.timeInUse;
    tserver._dataStartTime = reply.dataStartTime;
    tserver._dataEndTime = reply.dataEndTime;
    tserver._idayInUse = reply.idayInUse;

    // clear ready for tracks and entries

    tserver.clearArrays();

    // complex tracks

    if (reply.nComplexTracks > 0) {
      
      for (int ii = 0; ii < reply.nComplexTracks; ii++) {
	DsMsgPart *part = getPartByType(DS_TITAN_COMPLEX_TRACK_PART, ii);
	if (part == NULL) {
	  TaStr::AddInt(_errStr,
			"  Cannot find part for complex track, ii: ", ii);
	  return -1;
	}
	TitanComplexTrack *ctrack = new TitanComplexTrack;
	int lenUsed;
	if (ctrack->disassemble(part->getBuf(), part->getLength(), lenUsed)) {
	  TaStr::AddInt(_errStr,
			"  Cannot disassemble complex track, ii: ", ii);
	  delete ctrack;
	  return -1;
	}
	tserver._complexTracks.push_back(ctrack);
      } // ii

    } // if (reply.nComplexTracks > 0)

    // current entries

    if (reply.nCurrentEntries > 0) {
      
      DsMsgPart *part = getPartByType(DS_TITAN_CURRENT_ENTRIES_PART);
      if (part == NULL) {
	TaStr::AddStr(_errStr, "  ",
		      "Cannot find part for current entries.");
	return -1;
      }

      const ui08 *bufPtr = part->getBuf();
      int lenLeft = part->getLength();

      for (int ii = 0; ii < reply.nCurrentEntries; ii++) {

	TitanTrackEntry *entry = new TitanTrackEntry;
	
	int lenUsed;
	if (entry->disassemble(bufPtr, lenLeft, lenUsed)) {
	  TaStr::AddInt(_errStr, "  Cannot disassemble entry, ii: ", ii);
	  delete entry;
	  return -1;
	}

	tserver._currentEntries.push_back(entry);
	bufPtr += lenUsed;
	lenLeft -= lenUsed;
	
      } // ii

    } // if (reply.nCurrentEntries > 0)

  } // if (partExists(DS_TITAN_READ_REPLY_PART))
  
  return (0);

}
