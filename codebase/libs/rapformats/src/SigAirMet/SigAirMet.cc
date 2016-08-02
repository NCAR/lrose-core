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
// SigAirMet.cc
//
// C++ class for dealing with radar-visibility calibration.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// July 2002
//////////////////////////////////////////////////////////////


#include <rapformats/SigAirMet.hh>
#include <dataport/bigend.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <set>
#include <sstream>
#include <algorithm>

using namespace std;

///////////////
// constructor

SigAirMet::SigAirMet()

{
  MEM_zero(_hdr);
}

/////////////
// destructor

SigAirMet::~SigAirMet()

{
  
}

////////////////////
// clear the object

void SigAirMet::clear()

{
  MEM_zero(_hdr);
  _vertices.clear();
  _forecasts.clear();
  _outlooks.clear();
  _text = "";
}

///////////////
// set methods

void SigAirMet::setId(const string &id) {
  MEM_zero(_hdr.id);
  STRncopy(_hdr.id, id.c_str(), SIGAIRMET_ID_NBYTES);
}

void SigAirMet::setSource(const string &source) {
  MEM_zero(_hdr.source);
  STRncopy(_hdr.source, source.c_str(),
	   SIGAIRMET_SOURCE_NBYTES);
}

void SigAirMet::setQualifier(const string &qualifier) {
  MEM_zero(_hdr.qualifier);
  STRncopy(_hdr.qualifier, qualifier.c_str(), SIGAIRMET_QUALIFIER_NBYTES);
}

void SigAirMet::setWx(const string &wx) {
  MEM_zero(_hdr.wx);
  STRncopy(_hdr.wx, wx.c_str(), SIGAIRMET_WX_NBYTES);
}

void SigAirMet::setFir(const string &fir) {
  MEM_zero(_hdr.fir);
  STRncopy(_hdr.fir, fir.c_str(), SIGAIRMET_FIR_NBYTES);
  _hdr.fir_set = 1;
}

void SigAirMet::setText(const string &text) {
  if (text.size() > 0) {
    _hdr.text_len = text.size() + 1;
    _text = text;
  }
}

void SigAirMet::setGroup(sigairmet_group_t group) {
  _hdr.group = group;
}

void SigAirMet::setAction(sigairmet_action_t action) {
  _hdr.action = action;
}

void SigAirMet::setIssueTime(time_t issue_time) {
  _hdr.issue_time = issue_time;
}

void SigAirMet::setStartTime(time_t start_time) {
  _hdr.start_time = start_time;
}

void SigAirMet::setEndTime(time_t end_time)  {
  _hdr.end_time = end_time;
}

void SigAirMet::setObsTime(time_t obs_time)  {
  _hdr.obs_time = obs_time;
}

void SigAirMet::setFcastTime(time_t fcast_time)  {
  _hdr.fcast_time = fcast_time;
}

void SigAirMet::setCancelFlag(bool cancel_flag) {
  _hdr.cancel_flag = cancel_flag;
}

void SigAirMet::setCancelTime(time_t cancel_time)  {
  _hdr.cancel_time = cancel_time;
}

void SigAirMet::setPolygonIsFirBdry() {
  _hdr.polygon_is_fir_bdry = TRUE;
}

void SigAirMet::setFlightLevels(double bottom_flevel, double top_flevel) {
  _hdr.bottom_flevel = (fl32) bottom_flevel;
  _hdr.top_flevel = (fl32) top_flevel;
  _hdr.flevels_set = TRUE;
}

void SigAirMet::setObsAndOrFcst(int obsfcst)
{
  // 1=obs, 2=fcst, 3=obs and fcst
  _hdr.obs_and_or_fcst = obsfcst;
}

void SigAirMet::setCentroid(double lat, double lon)
{
  _hdr.centroid_lat = (fl32) lat;
  _hdr.centroid_lon = (fl32) lon;
  _hdr.centroid_set = TRUE;
}

void SigAirMet::setMovementSpeed(double speed)
{
  _hdr.movement_speed = speed;
}

void SigAirMet::setMovementDirn(double dirn)
{
  _hdr.movement_dirn = dirn;
}

// add vertex
  
void SigAirMet::addVertex(double lat, double lon) {
  sigairmet_vertex_t vertex;
  vertex.lat = lat;
  vertex.lon = lon;
  _vertices.push_back(vertex);
  _hdr.n_vertices = _vertices.size();
}

// add forecast
  
void SigAirMet::addForecast(time_t time,
                            double lat, double lon,
			    int id /* = 0 */) {
  sigairmet_forecast_t forecast;
  MEM_zero(forecast);
  forecast.time = time;
  forecast.lat = lat;
  forecast.lon = lon;
  forecast.id = id;
  _forecasts.push_back(forecast);
  _hdr.n_forecasts = _forecasts.size();
}

// add outlook
  
void SigAirMet::addOutlook(time_t time, double lat, double lon,
			   int id /* = 0 */) {
  sigairmet_forecast_t outlook;
  MEM_zero(outlook);
  outlook.time = time;
  outlook.lat = lat;
  outlook.lon = lon;
  outlook.id = id;
  _outlooks.push_back(outlook);
  _hdr.n_outlooks = _outlooks.size();
}

// clear vertices

void SigAirMet::clearVertices()
{
  _vertices.clear();
  _hdr.n_vertices = 0;
}

// clear forecasts

void SigAirMet::clearForecasts()
{
  _forecasts.clear();
  _hdr.n_forecasts = 0;
}

// clear outlooks

void SigAirMet::clearOutlooks()
{
  _outlooks.clear();
  _hdr.n_outlooks = 0;
}



////////////////////////
// compute the centroid

int SigAirMet::computeCentroid()
{

  if (_vertices.size() < 1) {
    
    _hdr.centroid_lat = 0.0;
    _hdr.centroid_lon = 0.0;
    return -1;
    
    
  } else {

    // condition the longitudes to avoid problems crossing the
    // 180 or 0 degree line

    _conditionLongitudes();

    // compute means

    double sumLon = 0.0;
    double sumLat = 0.0;
    for (size_t ii = 0; ii < _vertices.size(); ii++) {
      sumLon += _vertices[ii].lon;
      sumLat += _vertices[ii].lat;
    }
    
    _hdr.centroid_lon = sumLon / _vertices.size();
    _hdr.centroid_lat = sumLat / _vertices.size();

  }
  
  _hdr.centroid_set = TRUE;
  return 0;

}

////////////////////////////////////////////////////////////////
// condition the longitude values to avoid problems crossing the
// 180 or 0 degree line


void SigAirMet::_conditionLongitudes()
{

  // compute min and max longitudes

  double min_lon = _vertices[0].lon;
  double max_lon = min_lon;
  
  for (size_t ii = 1; ii < _vertices.size(); ii++) {
    min_lon = MIN(min_lon, _vertices[ii].lon);
    max_lon = MAX(max_lon, _vertices[ii].lon);
  }

  // check if longitudes span either the 0 or 180 lines

  if (fabs(min_lon - max_lon) < 180) {
    return;
  }
    
  if (min_lon < 0) {

    // spans the 180 line

    for (size_t ii = 0; ii < _vertices.size(); ii++) {
      if (fabs(min_lon - _vertices[ii].lon) < 180) {
	_vertices[ii].lon += 360.0;
      }
    }
    
  } else {
    
    // spans the 0 line
    
    for (size_t ii = 0; ii < _vertices.size(); ii++) {
      if (fabs(max_lon - _vertices[ii].lon) < 180) {
	_vertices[ii].lon -= 360.0;
      }
    }

  } // if (min_lon < 0)

}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void SigAirMet::assemble()
  
{
  
  _memBuf.free();
  
  // header

  sigairmet_hdr_t hcopy = _hdr;
  _hdrToBE(hcopy);
  _memBuf.add(&hcopy, sizeof(sigairmet_hdr_t));

  // vertices

  for (size_t ii = 0; ii < _vertices.size(); ii++) {
    sigairmet_vertex_t vertex = _vertices[ii];
    _vertexToBE(vertex);
    _memBuf.add(&vertex, sizeof(vertex));
  }

  // forecasts

  for (size_t ii = 0; ii < _forecasts.size(); ii++) {
    sigairmet_forecast_t forecast = _forecasts[ii];
    _forecastToBE(forecast);
    _memBuf.add(&forecast, sizeof(forecast));
  }

  // outlooks

  for (size_t ii = 0; ii < _outlooks.size(); ii++) {
    sigairmet_forecast_t outlook = _outlooks[ii];
    _forecastToBE(outlook);
    _memBuf.add(&outlook, sizeof(outlook));
  }

  // text
  
  if (_text.size() > 0) {
    _memBuf.add(_text.c_str(), _hdr.text_len);
  }
  
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int SigAirMet::disassemble(const void *buf, int len)

{

  // header

  int minLen = (int) sizeof(sigairmet_hdr_t);
  if (len < minLen) {
    cerr << "ERROR - SigAirMet::disassemble" << endl;
    cerr << "  Buffer header too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    return -1;
  }

  int offset = 0;
  memcpy(&_hdr, (ui08 *) buf + offset, sizeof(sigairmet_hdr_t));
  _hdrFromBE(_hdr);
  offset += sizeof(sigairmet_hdr_t);

  // vertices

  minLen += _hdr.n_vertices * sizeof(sigairmet_vertex_t);
  if (len < minLen) {
    cerr << "ERROR - SigAirMet::disassemble" << endl;
    cerr << "  Buffer verticies too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    cerr << "  Number of verticies : " << _hdr.n_vertices << endl;
    return -1;
  }

  _vertices.clear();
  for (int ii = 0; ii < _hdr.n_vertices; ii++) {
    sigairmet_vertex_t vertex;
    memcpy(&vertex, (ui08 *) buf + offset, sizeof(vertex));
    _vertexFromBE(vertex);
    _vertices.push_back(vertex);
    offset += sizeof(vertex);
  }

  // forecasts
  
  minLen += _hdr.n_forecasts * sizeof(sigairmet_forecast_t);
  if (len < minLen) {
    cerr << "ERROR - SigAirMet::disassemble" << endl;
    cerr << "  Buffer forecasts too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    cerr << "  Number of forecasts : " << _hdr.n_forecasts << endl;
    return -1;
  }

  _forecasts.clear();
  for (int ii = 0; ii < _hdr.n_forecasts; ii++) {
    sigairmet_forecast_t forecast;
    memcpy(&forecast, (ui08 *) buf + offset, sizeof(forecast));
    _forecastFromBE(forecast);
    _forecasts.push_back(forecast);
    offset += sizeof(forecast);
  }

  // outlooks
  
  minLen += _hdr.n_outlooks * sizeof(sigairmet_forecast_t);
  if (len < minLen) {
    cerr << "ERROR - SigAirMet::disassemble" << endl;
    cerr << "  Buffer outlooks too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    cerr << "  Number of outlooks : " << _hdr.n_outlooks << endl;
    return -1;
  }

  _outlooks.clear();
  for (int ii = 0; ii < _hdr.n_outlooks; ii++) {
    sigairmet_forecast_t outlook;
    memcpy(&outlook, (ui08 *) buf + offset, sizeof(outlook));
    _forecastFromBE(outlook);
    _outlooks.push_back(outlook);
    offset += sizeof(outlook);
  }

  // text

  if (_hdr.text_len > 0) {

    minLen += _hdr.text_len;
    if (len < minLen) {
      cerr << "ERROR - SigAirMet::disassemble" << endl;
      cerr << "  Buffer text too small for disassemble" << endl;
      cerr << "  Buf len: " << len << endl;
      cerr << "  Min len: " << minLen << endl;
      cerr << "  Number of verticies : " << _hdr.n_vertices << endl;
      cerr << "  Text length : " << _hdr.text_len << endl;
      return -1;
    }

    // ensure null termination

    char *text = (char *) buf + offset;
    text[_hdr.text_len - 1] = '\0';
    _text = text;
  
  }

  return 0;

}

string SigAirMet::_trimString(const string& str,
                 const string& whitespace)
{
    size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    size_t strEnd = str.find_last_not_of(whitespace);
    size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void 
SigAirMet::_splitTextByHeader()
{

	// Tokenize the raw text
  string buf; 
  stringstream ss(_text); 

  vector<string> tokens; 
  while (ss >> buf){
	  if (buf.find_first_not_of(' ') != string::npos){ //non white space
	  tokens.push_back(buf);
	  }
	  //cout << buf << endl;
  }

  //check for 'AIRMET' or 'SIGMET'
  vector<string>::iterator sit = std::find(tokens.begin(), tokens.end(), "SIGMET");
  if (sit == tokens.end()){
	  sit = std::find(tokens.begin(), tokens.end(), "AIRMET");
  }
  if (sit == tokens.end()){
	  return;
  }

  // locate index of SIGMET/AIRMET and the ICAO location identifier, preceding it
  int sigairIdx = sit - tokens.begin(); 
  int identIdx = sit - tokens.begin() - 1; 
  if (identIdx < 0) {
	  return;
  }

  //find position of SIGMET and ICAO location identifiers in original text
  size_t sigairFound = _text.find(tokens[sigairIdx]);
  size_t identFound = _text.substr(0,sigairFound).rfind(tokens[identIdx]);

  _wmo_header = _trimString(_text.substr(0,identFound));
  _text_without_header = _trimString(_text.substr(identFound));
  
}

///////////////////////////////////////////
// load XML string

void SigAirMet::loadXml(string &xml, int startIndentLevel /* = 0 */)
  
{
 
	_splitTextByHeader();
  int sil = startIndentLevel;

  // print object to string as XML
  
  xml = "";
  
  xml += TaXml::writeStartTag("AIRSIGMET", sil+0);
 
 /*
  xml += TaXml::writeStartTag("raw_text", sil+1);
  xml += _text;
  xml += TaXml::writeEndTag("raw_text", sil+1);
 */
 

  xml += TaXml::writeStartTag("wmo_header", sil+1);
  xml += _wmo_header;
  xml += TaXml::writeEndTag("wmo_header", sil+1);


  xml += TaXml::writeStartTag("raw_text", sil+1);
  xml += _text_without_header;
  xml += TaXml::writeEndTag("raw_text", sil+1);


  if (_hdr.fir_set) {
	  xml += TaXml::writeStartTag("fir_id", sil+1);
	  xml += _hdr.fir;
	  xml += TaXml::writeEndTag("fir_id", sil+1);
  }
 
 
  xml += TaXml::writeTime("valid_time_from", sil+1, _hdr.start_time);
  xml += TaXml::writeTime("valid_time_to", sil+1, _hdr.end_time);
  
  if (_hdr.flevels_set)
  {
    vector< TaXml::attribute > alt_attrs;
    
    if (_hdr.bottom_flevel >= 0)
      TaXml::addIntAttr("min_ft_msl",
			(int)(_hdr.bottom_flevel * 100.0), alt_attrs);

    if (_hdr.top_flevel >= 0)
      TaXml::addIntAttr("max_ft_msl",
			(int)(_hdr.top_flevel * 100.0), alt_attrs);

    xml += TaXml::writeTagClosed("altitude", sil+1, alt_attrs);
  }
  
  if (_hdr.movement_dirn != -1.0)
  {
    xml += TaXml::writeInt("movement_dir_degrees", sil+1,
			   (int)(_hdr.movement_dirn + 0.5));
  }
  
  if (_hdr.movement_speed != -1.0)
  {
    double speed_kts =
      (_hdr.movement_speed / MPERSEC_TO_KMPERHOUR) * MS_TO_KNOTS;
    
    xml += TaXml::writeInt("movement_speed_kt", sil+1, (int)(speed_kts + 0.5));
  }
  
  string wx_string = _hdr.wx;
  if (wx_string != "" && wx_string != "UNKNOWN")
    xml += TaXml::writeString("hazard", sil+1, wx_string);

  if (_hdr.group == AIRMET_GROUP)
    xml += TaXml::writeString("airsigmet_type", sil+1, "AIRMET");
  else
    xml += TaXml::writeString("airsigmet_type", sil+1, "SIGMET");

  if (_vertices.size() > 0)
  {
    vector< TaXml::attribute > num_pts_attr;
    TaXml::addIntAttr("num_points", _vertices.size(), num_pts_attr);
    xml += TaXml::writeStartTag("area", sil+1, num_pts_attr, true);

    vector< sigairmet_vertex_t >::const_iterator vertex;
    for (vertex = _vertices.begin(); vertex != _vertices.end(); ++vertex)
    {
      xml += TaXml::writeStartTag("point", sil+2);
      xml += TaXml::writeDouble("longitude", sil+3, vertex->lon);
      xml += TaXml::writeDouble("latitude", sil+3, vertex->lat);
      xml += TaXml::writeEndTag("point", sil+2);
    }

    xml += TaXml::writeEndTag("area", sil+1);
  }
  
  xml += TaXml::writeEndTag("AIRSIGMET", sil+0);
}

//////////////////////
// printing object


void SigAirMet::print(ostream &out, string spacer /* = "" */ ) const
{
  
  out << "===================================" << endl;
  out << spacer << "SigAirMet object" << endl;
  out << spacer << "  id: " << _hdr.id << endl;
  out << spacer << "  source: " << _hdr.source << endl;
  out << spacer << "  qualifier: " << _hdr.qualifier << endl;
  out << spacer << "  wx: " << _hdr.wx << endl;
  out << spacer << "  text_len: " << _hdr.text_len << endl;
  out << spacer << "  text: " << _text << endl;
  out << spacer << "  group: " << group2String()  << endl;
  out << spacer << "  action: " << action2String()  << endl;
  out << spacer << "  issue_time: " << DateTime::strm(_hdr.issue_time) << endl;
  out << spacer << "  start_time: " << DateTime::strm(_hdr.start_time) << endl;
  out << spacer << "  end_time: " << DateTime::strm(_hdr.end_time) << endl;
  out << spacer << "  obs_time: " << DateTime::strm(_hdr.obs_time) << endl;
  out << spacer << "  fcast_time: " << DateTime::strm(_hdr.fcast_time) << endl;
  out << spacer << "  movement speed: " << _hdr.movement_speed << ", dir: " << _hdr.movement_dirn << endl;
  out << spacer << "  cancel_flag: " << _hdr.cancel_flag << endl;
  if (_hdr.cancel_flag) {
    out << spacer << "  cancel_time: "
	<< DateTime::str(_hdr.cancel_time) << endl;
  }
  if (_hdr.flevels_set) {
    out << spacer << "  bottom_flevel: " << _hdr.bottom_flevel << endl;
    out << spacer << "  top_flevel: " << _hdr.top_flevel << endl;
  }
  if (_hdr.centroid_set) {
    out << spacer << "  centroid_lat: " << _hdr.centroid_lat << endl;
    out << spacer << "  centroid_lon: " << _hdr.centroid_lon << endl;
  }
  if (_hdr.fir_set) {
    out << spacer << "  fir: " << _hdr.fir << endl;
  }
  if (_hdr.polygon_is_fir_bdry) {
    out << spacer << "  polygon_is_fir_bdry: " << _hdr.polygon_is_fir_bdry << endl;
  }
  out << spacer << "  n_vertices: " << _hdr.n_vertices << endl;
  if (_vertices.size() > 0) {
    out << spacer << "  Vertices:" << endl;
    for (size_t ii = 0; ii < _vertices.size(); ii++) {
      out << spacer << "      vertex[" << ii << "] "
	  << " lat: " << _vertices[ii].lat << ", "
	  << " lon: " << _vertices[ii].lon << endl;
    }
    out << endl;
  }

  set<time_t, less<time_t> > fcastTimes;
  for (int ii = 0; ii < (int) _forecasts.size(); ii++) {
    fcastTimes.insert(fcastTimes.begin(), _forecasts[ii].time);
  }
  
  if (fcastTimes.size() > 0) {
    out << spacer << "  n forecast times: " << fcastTimes.size() << endl;
    for (set<time_t>::iterator jj = fcastTimes.begin();
         jj != fcastTimes.end(); jj++) {
      time_t ftime = *jj;
      out << spacer << "    Forecast time: " << DateTime::strm(ftime) << endl;
      for (size_t ii = 0; ii < _forecasts.size(); ii++) {
        if (_forecasts[ii].time == ftime) {
          out << spacer << "      vertex[" << ii << "] "
              << " lat: " << _forecasts[ii].lat << ", "
              << " lon: " << _forecasts[ii].lon << ", "
              << "  id: " << _forecasts[ii].id << endl;
        }
      } // ii
    } // jj
    out << endl;
  }

  set<time_t, less<time_t> > outlkTimes;
  for (int ii = 0; ii < (int) _outlooks.size(); ii++) {
    outlkTimes.insert(outlkTimes.begin(), _outlooks[ii].time);
  }
  
  if (outlkTimes.size() > 0) {
    out << spacer << "  n outlook times: " << outlkTimes.size() << endl;
    for (set<time_t>::iterator jj = outlkTimes.begin();
         jj != outlkTimes.end(); jj++) {
      time_t otime = *jj;
      out << spacer << "    Outlook time: " << DateTime::strm(otime) << endl;
      for (size_t ii = 0; ii < _outlooks.size(); ii++) {
        if (_outlooks[ii].time == otime) {
          out << spacer << "      vertex[" << ii << "] "
              << " lat: " << _outlooks[ii].lat << ", "
              << " lon: " << _outlooks[ii].lon << ", "
              << "  id: " << _outlooks[ii].id << endl;
        }
      } // ii
    } // jj
    out << endl;
  }
  
}

///////////////
// print as XML

void SigAirMet::printAsXml(ostream &out, int startIndentLevel /* = 0 */) 

{

  if (_xml.size() == 0 || startIndentLevel != 0) {
    loadXml(_xml, startIndentLevel);
  }

  out << _xml;

}


const char *SigAirMet::group2String() const
{
  return group2String((sigairmet_group_t) _hdr.group);
}

const char *SigAirMet::group2String(sigairmet_group_t group)
{
  switch (group) {
  case AIRMET_GROUP:
    return "AIRMET";
  case SIGMET_GROUP:
    return "SIGMET";
  default:
    return "SIGMET";
  }
}

sigairmet_group_t SigAirMet::getGroup(const string &groupStr)
{
  if (groupStr == "AIRMET") {
    return AIRMET_GROUP;
  } else if (groupStr == "SIGMET") {
    return SIGMET_GROUP;
  }
  return SIGMET_GROUP;
}

const char *SigAirMet::action2String() const
{
  return action2String((sigairmet_action_t) _hdr.action);
}

const char *SigAirMet::action2String(sigairmet_action_t action)
{
  switch (action) {
  case SIGAIRMET_NEW:
    return "NEW";
  case SIGAIRMET_AMENDMENT:
    return "AMENDMENT";
  default:
    return "NEW";
  }
}

sigairmet_action_t SigAirMet::getAction(const string &actionStr)
{
  if (actionStr == "NEW") {
    return SIGAIRMET_NEW;
  } else if (actionStr == "AMENDMENT") {
    return SIGAIRMET_AMENDMENT;
  }
  return SIGAIRMET_NEW;
}

/////////////////
// byte swapping

void SigAirMet::_hdrToBE(sigairmet_hdr_t &hdr)
{
  BE_from_array_32(&hdr, SIGAIRMET_HDR_NBYTES_32);
}

void SigAirMet::_hdrFromBE(sigairmet_hdr_t &hdr)
{
  BE_to_array_32(&hdr, SIGAIRMET_HDR_NBYTES_32);
}

void SigAirMet::_vertexToBE(sigairmet_vertex_t &vertex)
{
  BE_from_array_32(&vertex, sizeof(vertex));
}

void SigAirMet::_vertexFromBE(sigairmet_vertex_t &vertex)
{
  BE_to_array_32(&vertex, sizeof(vertex));
}

void SigAirMet::_forecastToBE(sigairmet_forecast_t &forecast)
{
  BE_from_array_32(&forecast, sizeof(forecast));
}

void SigAirMet::_forecastFromBE(sigairmet_forecast_t &forecast)
{
  BE_to_array_32(&forecast, sizeof(forecast));
}

