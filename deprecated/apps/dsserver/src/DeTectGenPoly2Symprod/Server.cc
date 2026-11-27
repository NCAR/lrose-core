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
// Server.cc
//
// File Server object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 2011
//
///////////////////////////////////////////////////////////////

#include <map>
#include <string>

#include <toolsa/toolsa_macros.h>

#include "Server.hh"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Constructor
//
// Inherits from DsSymprodServer
///

Server::Server(const string &prog_name,
	       Params *initialParams)
  : DsSymprodServer(prog_name,
		 initialParams->instance,
		 (void*)(initialParams),
		 initialParams->port,
		 initialParams->qmax,
		 initialParams->max_clients,
		 initialParams->no_threads,
		 initialParams->debug >= Params::DEBUG_NORM,
		 initialParams->debug >= Params::DEBUG_VERBOSE)
{
  // Construct the coasted location icon

  Symprod::ppt_t icon_pt;

  icon_pt.x = 0;
  icon_pt.y = 1;
  _coastedIcon.add(&icon_pt, sizeof(icon_pt));
  
  icon_pt.x = 0;
  icon_pt.y = -1;
  _coastedIcon.add(&icon_pt, sizeof(icon_pt));
  
  icon_pt.x = Symprod::PPT_PENUP;
  icon_pt.y = Symprod::PPT_PENUP;
  _coastedIcon.add(&icon_pt, sizeof(icon_pt));
  
  icon_pt.x = -1;
  icon_pt.y = 0;
  _coastedIcon.add(&icon_pt, sizeof(icon_pt));
  
  icon_pt.x = 1;
  icon_pt.y = 0;
  _coastedIcon.add(&icon_pt, sizeof(icon_pt));
}


//////////////////////////////////////////////////////////////////////
// load local params if they are to be overridden.

int
Server::loadLocalParams( const string &paramFile, void **serverParams)

{
   Params  *localParams;
   char   **tdrpOverrideList = NULL;
   bool     expandEnvVars = true;

   const char *routine_name = "_allocLocalParams";

   if (_isDebug) {
     cerr << "Loading new params from file: " << paramFile << endl;
   }

   localParams = new Params( *((Params*)_initialParams) );
   if ( localParams->load( (char*)paramFile.c_str(),
                           tdrpOverrideList,
                           expandEnvVars,
                           _isVerbose ) != 0 ) {
      cerr << "ERROR - " << _executableName << "::" << routine_name << endl
           << "Cannot load parameter file: " << paramFile << endl;
      delete localParams;
      return( -1 );
   }

   if (_isVerbose) {
     localParams->print(stderr, PRINT_SHORT);
   }

   *serverParams = (void*)localParams;
   return( 0 );
}


//////////////////////////////////////////////////////////////////////
// convertToSymprod() - Convert the given data chunk from the SPDB
//                      database to symprod format.
//
// Returns 0 on success, -1 on failure

int Server::convertToSymprod(const void *params,
			     const string &dir_path,
			     const TrackInfo &track_info,
			     MemBuf &symprod_buf)
{
  static const string method_name = "Server::convertToSymprod()";
  
  if (_isDebug)    cerr << method_name << ": entry" << endl;
  
  Params *serverParams = (Params*) params;

  time_t now;
  
  // create Symprod object

  now = time(NULL);
  Symprod prod(now, now, track_info.getTime().utime(),
	       track_info.getTime().utime() + serverParams->expire_secs, 0, 0,
	       "DeTectHorizontalSpdb");

  // Add the track to the symprod product

  if (serverParams->render_track)
    _addTrack(serverParams, prod, track_info);
  
  // Draw the vector

  if (serverParams->render_vector)
    _addVector(serverParams, prod, track_info);
  
  // Draw the ellipse

  if (serverParams->render_ellipse)
    _addEllipse(serverParams, prod, track_info);
  
  // Display any requested fields

  for (int i = 0; i < serverParams->display_fields_n; ++i)
  {
    string field_name;
    string field_units;
    double field_value;
    
    if (!track_info.getFieldInfo(serverParams->_display_fields[i].field_id,
				 field_name, field_units, field_value))
      continue;
    
    char format_string[80];

    if (serverParams->show_field_names)
    {
      if (serverParams->_display_fields[i].display_units)
	sprintf(format_string, "%s %s (%s)",
		field_name.c_str(),
		serverParams->_display_fields[i].format_string,
		field_units.c_str());
      else
	sprintf(format_string, "%s %s",
		field_name.c_str(),
		serverParams->_display_fields[i].format_string);
    }
    else
    {
      if (serverParams->_display_fields[i].display_units)
	sprintf(format_string, "%s (%s)",
		serverParams->_display_fields[i].format_string,
		field_units.c_str());
      else
	sprintf(format_string, "%s",
		serverParams->_display_fields[i].format_string);
    }
    
    _addText(field_value, format_string,
	     prod,
	     track_info.getTrack()[0].lat, track_info.getTrack()[0].lon,
	     serverParams->_display_fields[i].text_color,
	     serverParams->_display_fields[i].background_color,
	     serverParams->_display_fields[i].x_offset,
	     serverParams->_display_fields[i].y_offset,
	     serverParams->_display_fields[i].font_size,
	     serverParams->_display_fields[i].font_name,
	     _convertVertAlignParam(serverParams->_display_fields[i].vert_align),
	     _convertHorizAlignParam(serverParams->_display_fields[i].horiz_align));
  } /* endfor - i */

  // Add time labels, if requested

  if (serverParams->display_valid_time)
  {
    char time_label[1024];

    DateTime valid_time = track_info.getTime();
      
    sprintf(time_label, "Valid %d/%02d/%02d %02d:%02d:%02d ",
	    valid_time.getYear(), valid_time.getMonth(), valid_time.getDay(),
	    valid_time.getHour(), valid_time.getMin(), valid_time.getSec());
      
    prod.addText(time_label,
                 track_info.getTrack()[0].lat, track_info.getTrack()[0].lon,
		 serverParams->display_time.text_color,
		 serverParams->display_time.background_color,
		 serverParams->display_time.x_offset,
		 serverParams->display_time.y_offset,
		 _convertVertAlignParam(serverParams->display_time.vert_align),
		 _convertHorizAlignParam(serverParams->display_time.horiz_align),
		 serverParams->display_time.font_size,
		 Symprod::TEXT_NORM,
		 serverParams->display_time.font_name);
  }

  // set return buffer

  if (_isVerbose)
    prod.print(cerr);

  prod.serialize(symprod_buf);

  return 0;
}

/////////////////////////////////////////////////////////////////////
// transformData() - Transform the data from the database into
//                   symprod format.

void Server::transformData(const void *localParams,
			   const string &dir_path,
			   const DateTime &request_time,
			   int prod_id,
			   const string &prod_label,
			   int n_chunks_in,
			   const Spdb::chunk_ref_t *chunk_refs_in,
			   const Spdb::aux_ref_t *aux_refs_in,
			   const void *chunk_data_in,
			   int &n_chunks_out,
			   MemBuf &refBufOut,
			   MemBuf &auxBufOut,
			   MemBuf &dataBufOut)
{
  static const string method_name = "Server::transformData()";
  
  // We need to construct the tracks here.  We need to send two things to
  // convertToSymprod(): the constructed track and the SPDB record for the
  // latest position.  The track will be a vector of points where each point
  // consists of lat, lon, coasting flag.  The track plus SPDB info should
  // be a separate object so we can do a simple loop here.

  // initialize the buffers
  
  if (_isDebug) cerr << method_name << ": entry n_chunks_in: "
    << n_chunks_in << "  prod_id: " << prod_id << endl;


  refBufOut.free();
  auxBufOut.free();
  dataBufOut.free();

  // Loop through the chunks, constructing the individual tracks.  We want to
  // loop through backwards so that we process things from the latest time to
  // the earliest.

  map< string, TrackInfo> tracks;
  
  for (int i = n_chunks_in - 1; i >= 0; --i)
  {

    Spdb::chunk_ref_t ref = chunk_refs_in[i];
    Spdb::aux_ref_t aux = aux_refs_in[i];
    void *chunk_data = (void *)((char *)chunk_data_in + ref.offset);

    if (_isDebug)
      cerr << method_name << ": chunk "
	   << i << " of " << n_chunks_in << "  data_type: " << ref.data_type
	   << "  data_type2: " << ref.data_type2 << "  len: "
	   << ref.len << endl;

    // Copy the SPDB data to the local buffer and convert it to native format
    // so we can use it.

    MemBuf polylineBuf;
    polylineBuf.load(chunk_data, ref.len);
  
    DeTectHorizontalSpdb polyline;
    if (!polyline.disassemble(polylineBuf.getPtr(), polylineBuf.getLen()))
    {
      cerr << "ERROR - " << _executableName << ":" << method_name << endl;
      cerr << "  Error disassembling DeTectHorizontalSpdb buffer" << endl; 

      continue;
    }
    
    // Find the associated track and add the new entry

    map< string, TrackInfo >::iterator record = tracks.find(polyline.getName());
    
    if (record == tracks.end())
    {
      tracks.insert(pair< string, TrackInfo >(polyline.getName(), polyline));
    }
    else
    {
      (record->second).addRecord(polyline);
    }
    
  } /* endfor - i */
  
  if (_isDebug)
  {
    for (map< string, TrackInfo >::const_iterator track = tracks.begin();
	 track != tracks.end(); ++track)
      cerr << "    Track name: " << (track->second).getName()
	   << "   " << (track->second).getTime()
	   << "   " << (track->second).getTrack().size() << " pts" << endl;
  }
  
  // Transform each chunk and add it to the memory buffers

  n_chunks_out = 0;
  MemBuf symprodBuf;
  
  for (map< string, TrackInfo >::const_iterator track = tracks.begin();
       track != tracks.end(); ++track)
  {
    symprodBuf.free();

    if (convertToSymprod(localParams, dir_path, track->second,
                         symprodBuf) == 0)
    {
      Spdb::chunk_ref_t ref;
      MEM_zero(ref);
      ref.offset = dataBufOut.getLen();
      ref.len = symprodBuf.getLen();
      refBufOut.add(&ref, sizeof(ref));
      
      Spdb::aux_ref_t aux;
      MEM_zero(aux);
      aux.write_time = (ti32)time(NULL);
      auxBufOut.add(&aux, sizeof(aux));
      
      dataBufOut.add(symprodBuf.getPtr(), symprodBuf.getLen());

      n_chunks_out++;
    }
  } /* endfor - track */
  
  if (_isDebug)
    cerr << method_name << ": exit\n";
}

//////////////////////////////////////////////////////////////////////
// _addEllipse()

void Server::_addEllipse(const Params *serverParams, 
			 Symprod &prod,
			 const TrackInfo &track_info) const
{
  // We don't have an ellipse if the current portion of the track is coasted

  if (track_info.isCoasted())
    return;
  
  // Get the ellipse information

  double ellipse_major = track_info.getEllipseMajorKm();
  double ellipse_minor = track_info.getEllipseMinorKm();
  double orientation = track_info.getOrientation();
  
  if (ellipse_major == track_info.getMissingValue() ||
      ellipse_minor == track_info.getMissingValue() ||
      orientation == track_info.getMissingValue())
  {
    cerr << "*** Missing ellipse data -- cannot render" << endl;
    return;
  }
  
  // Add the ellipse to the Symprod product

  prod.addArc(track_info.getTrack()[0].lat,
	      track_info.getTrack()[0].lon,
	      ellipse_major, ellipse_minor,
	      serverParams->ellipse_color,
	      false, 0.0, 360.0, orientation);
  
}


//////////////////////////////////////////////////////////////////////
// _addTrack() - Add a SYMPROD polyline object for the track to the
//               product buffer.

void Server::_addTrack(const Params *serverParams, 
		       Symprod &prod,
		       const TrackInfo &track_info) const
{
  // Load the polyline data and save the coasted locations

  const vector< TrackInfo::point_t > &track = track_info.getTrack();
  
  MemBuf point_buf;
  MemBuf coasted_buf;
  
  size_t i = 0;
  
  for (vector< TrackInfo::point_t >::const_iterator vertex = track.begin();
       vertex != track.end(); ++vertex, ++i)
  {
    Symprod::wpt_t point;
    
    point.lat = vertex->lat;
    point.lon = vertex->lon;
    point_buf.add(&point, sizeof(point));

    if (vertex->coasted_flag)
      coasted_buf.add(&point, sizeof(point));
    
  } /* endfor - vertex */
  
  // Add the polyline to the product

  prod.addPolyline(point_buf.getLen() / sizeof(Symprod::wpt_t),
		   (Symprod::wpt_t *)point_buf.getPtr(),
		   serverParams->track_color,
		   _convertLineTypeParam(serverParams->suggested_line_type),
		   serverParams->suggested_line_width,
		   _convertCapstyleParam(serverParams->suggested_capstyle),
		   _convertJoinstyleParam(serverParams->suggested_joinstyle));

  // Add the coasted locations to the product

  if (coasted_buf.getLen() > 0)
  {
    prod.addStrokedIcons(serverParams->coasted_location_color,
			 _coastedIcon.getLen() / sizeof(Symprod::ppt_t),
			 (Symprod::ppt_t *)_coastedIcon.getPtr(),
			 coasted_buf.getLen() / sizeof(Symprod::wpt_t),
			 (Symprod::wpt_t *)coasted_buf.getPtr());
  }
  
  return;
}


//////////////////////////////////////////////////////////////////////
// _addVector()

void Server::_addVector(const Params *serverParams, 
			Symprod &prod,
			const TrackInfo &track_info) const
{
  // Get the speed and direction values for the vector

  double speed = track_info.getSpeed();
  double direction;

  if (serverParams->use_predicted_heading)
    direction = track_info.getPHeading();
  else
    direction = track_info.getHeading();
  
  if (speed == track_info.getMissingValue() ||
      direction == track_info.getMissingValue())
    return;
  
  // Calculate the length of the vector

  double length;

  if (serverParams->fixed_length_vectors)
    length = serverParams->vector_length;
  else
    length = speed * serverParams->vector_lead_time / 1000.0;
  
  // Finally, add the vector to the product

  prod.addArrowStartPt(serverParams->vector_color,
		       _convertLineTypeParam(serverParams->suggested_line_type),
		       serverParams->suggested_line_width,
		       _convertCapstyleParam(serverParams->suggested_capstyle),
		       _convertJoinstyleParam(serverParams->suggested_joinstyle),
		       track_info.getTrack()[0].lat,
		       track_info.getTrack()[0].lon,
		       length, direction,
		       serverParams->arrow_head_len,
		       serverParams->arrow_head_half_angle);

}


//////////////////////////////////////////////////////////////////////
// _convertCapstyleParam() - Convert the TDRP capstyle parameter to
//                           the matching symprod value.

Symprod::capstyle_t Server::_convertCapstyleParam(int capstyle)
{
  switch(capstyle)
  {
  case Params::CAPSTYLE_BUTT :
    return(Symprod::CAPSTYLE_BUTT);
    
  case Params::CAPSTYLE_NOT_LAST :
    return(Symprod::CAPSTYLE_NOT_LAST);
    
  case Params::CAPSTYLE_PROJECTING :
    return(Symprod::CAPSTYLE_PROJECTING);

  case Params::CAPSTYLE_ROUND :
    return(Symprod::CAPSTYLE_ROUND);
  }
  
  return(Symprod::CAPSTYLE_BUTT);
}


//////////////////////////////////////////////////////////////////////
// _convertHorizAlignParam() - Convert the TDRP horizontal alignment
//                             parameter to the matching symprod value.

Symprod::horiz_align_t Server::_convertHorizAlignParam(int horiz_align)
{
  switch (horiz_align)
  {
  case Params::HORIZ_ALIGN_LEFT :
    return(Symprod::HORIZ_ALIGN_LEFT);
    
  case Params::HORIZ_ALIGN_CENTER :
    return(Symprod::HORIZ_ALIGN_CENTER);
    
  case Params::HORIZ_ALIGN_RIGHT :
    return(Symprod::HORIZ_ALIGN_RIGHT);
  }
  
  return(Symprod::HORIZ_ALIGN_LEFT);
}


//////////////////////////////////////////////////////////////////////
// _convertJoinstyleParam() - Convert the TDRP joinstyle parameter to
//                            the matching symprod value.

Symprod::joinstyle_t Server::_convertJoinstyleParam(int joinstyle)
{
  switch(joinstyle)
  {
  case Params::JOINSTYLE_BEVEL :
    return(Symprod::JOINSTYLE_BEVEL);
    
  case Params::JOINSTYLE_MITER :
    return(Symprod::JOINSTYLE_MITER);
    
  case Params::JOINSTYLE_ROUND :
    return(Symprod::JOINSTYLE_ROUND);
  }
  
  return(Symprod::JOINSTYLE_BEVEL);
}


//////////////////////////////////////////////////////////////////////
// _convertLineTypeParam() - Convert the TDRP line type parameter to
//                           the matching symprod value.

Symprod::linetype_t Server::_convertLineTypeParam(int line_type)
{
  switch(line_type)
  {
  case Params::LINETYPE_SOLID :
    return(Symprod::LINETYPE_SOLID);
    
  case Params::LINETYPE_DASH :
    return(Symprod::LINETYPE_DASH);
    
  case Params::LINETYPE_DOT_DASH :
    return(Symprod::LINETYPE_DOT_DASH);
  }
  
  return(Symprod::LINETYPE_SOLID);
}

//////////////////////////////////////////////////////////////////////
// _convertVertAlignParam() - Convert the TDRP vertical alignment
//                            parameter to the matching symprod value.

Symprod::vert_align_t Server::_convertVertAlignParam(int vert_align)
{
  switch (vert_align)
  {
  case Params::VERT_ALIGN_TOP :
    return(Symprod::VERT_ALIGN_TOP);
    
  case Params::VERT_ALIGN_CENTER :
    return(Symprod::VERT_ALIGN_CENTER);
    
  case Params::VERT_ALIGN_BOTTOM :
    return(Symprod::VERT_ALIGN_BOTTOM);
  }
  
  return(Symprod::VERT_ALIGN_TOP);
}

///////////////////////////////////////////////
// function for actually doing the get from disk

int Server::_doGet(const Params &params,
                   DsSpdb &spdb,
                   const string &dir_path,
                   int get_mode,
                   const DsSpdbMsg::info_t &info,
                   DsSpdbMsg::info_t *get_info,
                   string &errStr,
                   time_t &requestTime,
                   time_t &startTime,
                   time_t &endTime)
{
  static const string method_name = "Server::_doGet()";
  
  if (_isDebug)
    cerr << method_name << ":entry" << endl;
  
  *get_info = info;

  // set the request time

  int request_interval_secs = params.track_lookback_secs;
  
  switch (get_mode)
  {
  case DsSpdbMsg::DS_SPDB_GET_MODE_EXACT:
    requestTime = info.request_time;
    break;
    
  case DsSpdbMsg::DS_SPDB_GET_MODE_CLOSEST:
    requestTime = info.request_time;
    break;
    
  case DsSpdbMsg::DS_SPDB_GET_MODE_INTERVAL:
    requestTime = info.end_time;
    request_interval_secs =
      (info.end_time - info.start_time) + params.track_lookback_secs;
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

  startTime = requestTime - request_interval_secs;
  endTime = requestTime;
  
  if (params.debug >= Params::DEBUG_VERBOSE)
  {
    cerr << "Interval: " << DateTime::str(startTime) << " to " 
         << DateTime::str(endTime) << endl;
  }
  
  if (spdb.getInterval(dir_path, startTime, endTime,
                       info.data_type))
  {
    errStr += "  get INTERVAL mode failed\n";
    errStr += spdb.getErrStr();
    return -1;
  }
  
  // check prod_id

  int prod_id = spdb.getProdId();
  if (prod_id != SPDB_GENERIC_POLYLINE_ID)
  {
    cerr << "WARNING - " << _executableName << "::" << method_name << endl;
    cerr << "  Incorrect prod_id: " << prod_id << endl;
    cerr << "  Should be SPDB_GENERIC_POLYLINE_ID: "
	 << SPDB_GENERIC_POLYLINE_ID << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////////
// Handle a get request -- sets the prod_id and prod_label 
// in the get info appropriately

int Server::_handleGet(const void *localParams,
		       const DsSpdbMsg &inMsg, 
		       const string &dirPath,
		       Socket &socket)
{
  static const string method_name = "Server::_handleGet()";
  
  if (_isDebug) cerr << method_name << ": entry\n";

  // Replace this "doMsgGet()" call with the _doGet() stuff from AcPosn2Symprod
  // Then override the transformData() method to collect the GenPoly database
  // elements into full tracks.  We need to do the construction of the tracks
  // in the server rather than the ingester so that we can color the "coasted"
  // locations differently from the rest of the locations.

  string errStr = "ERROR -" + _executableName + "::" + method_name + "\n";
  
  DsSpdbMsg::info_t getInfo;
  DsSpdb spdb;
  DsSpdbMsg replyMsg;
  time_t requestTime, startTime, endTime;
  
  if (_doGet(*(Params *)localParams, spdb, dirPath,
	     inMsg.getMode(), inMsg.getInfo(),
	     &getInfo, errStr,
	     requestTime, startTime, endTime)) {
    
    errStr += spdb.getErrStr();
    errStr += "  URL: ";
    errStr += inMsg.getUrlStr();
    errStr += "\n";
    replyMsg.assembleGetErrorReturn(inMsg.getSpdbMode(), errStr.c_str());
    if (_isDebug) {
      cerr << errStr << endl;
    }

  } else {

    if (inMsg.getMode() == DsSpdbMsg::DS_SPDB_GET_MODE_TIMES) {

      replyMsg.assembleGetTimesSuccessReturn(inMsg.getSpdbMode(), getInfo);

    } else {

      if (_unique == Spdb::UniqueLatest) {
        spdb.makeUniqueLatest();
      } else if (_unique == Spdb::UniqueEarliest) {
        spdb.makeUniqueEarliest();
      }

      MemBuf refBufOut;
      MemBuf auxBufOut;
      MemBuf dataBufOut;
      int nChunksOut;
      
      transformData(localParams,
                    dirPath,
		    requestTime,
                    spdb.getProdId(), spdb.getProdLabel(),
                    spdb.getNChunks(),
                    spdb.getChunkRefs(), spdb.getAuxRefs(),
                    spdb.getChunkData(),
                    nChunksOut, refBufOut, auxBufOut, dataBufOut);
      
      if (_isDebug) {
        cerr << "Found data, nChunks: " << spdb.getNChunks() << endl;
      }

      getInfo.prod_id = SPDB_SYMPROD_ID;
      STRncopy(getInfo.prod_label, SPDB_SYMPROD_LABEL, SPDB_LABEL_MAX);
      getInfo.n_chunks = nChunksOut;
      replyMsg.assembleGetDataSuccessReturn
        (inMsg.getSpdbMode(),
         getInfo, refBufOut,
         auxBufOut, dataBufOut,
         inMsg.getDataBufCompression());

    }
    
  }
    
  // send reply

  void *replyMsgBuf = replyMsg.assembledMsg();
  int replyBuflen = replyMsg.lengthAssembled();

  if (_isDebug) {
    cerr << "==== reply message ====" << endl;
    replyMsg.print(cerr);
    cerr << "=======================" << endl;
  }

  if (socket.writeMessage(DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
                          replyMsgBuf, replyBuflen, 1000)) {
    cerr << "ERROR - COMM - " << method_name  << endl;
    cerr << "  Writing reply" << endl;
    return -1;
  }

  if (_isDebug) cerr << method_name << ": exit\n";
  return 0;

}
