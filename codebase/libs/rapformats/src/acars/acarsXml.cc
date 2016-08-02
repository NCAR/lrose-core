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
// acarsXml.cc
//
// C++ class for wrapping pirep.h, and adding XML components.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2007
//////////////////////////////////////////////////////////////

#include <rapformats/acarsXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>

///////////////
// constructor

acarsXml::acarsXml()
  
{
  acars_init(&_acars);
}

/////////////
// destructor

acarsXml::~acarsXml()

{

}

//////////////////////////
// reset all data members

void acarsXml::reset()

{

  acars_init(&_acars);
  _message = "";
  _memBuf.free();

}

///////////////////////////////////////////
// assemble buffer starting with acars_t struct
// and optionally followed by message text as XML

void acarsXml::assemble(bool appendMessageXml)

{

  // byte swap struct

  acars_t be_acars;
  memcpy(&be_acars, &_acars, sizeof(acars_t));
  BE_from_acars(&be_acars);
  
  // check mem buffer is free
  
  _memBuf.free();
  
  // add byte-swapped struct to buffer
  
  _memBuf.add(&be_acars, sizeof(acars_t));
  
  // check if we need to add message text as XML to the buffer

  if (appendMessageXml) {
    
    string xml;
    loadMessageXml(xml);
    
    // add xml string to buffer, including trailing null
    
    _memBuf.add(xml.c_str(), xml.size() + 1);

  }

}

///////////////////////////////////////////
// assemble as XML
// Load up an XML buffer from the object.

void acarsXml::assembleAsXml()
  
{

  // check mem buffer is free
  
  _memBuf.free();
  
  // convert to XML string

  string xml;
  loadXml(xml);

  // add xml string to buffer, including trailing null
  
  _memBuf.add(xml.c_str(), xml.size() + 1);
  
}

///////////////////////////////////////////
// load XML string

void acarsXml::loadXml(string &xml) const
  
{

  // print object to string as XML

  xml = "";
  
  xml += TaXml::writeStartTag("acars", 0);
  
  xml += TaXml::writeTime("time", 1, _acars.time);
  xml += TaXml::writeDouble("lat", 1, _acars.lat);
  xml += TaXml::writeDouble("lon", 1, _acars.lon);
  xml += TaXml::writeDouble("alt", 1, _acars.alt);
  xml += TaXml::writeDouble("temp", 1, _acars.temp);
  xml += TaXml::writeDouble("wind_speed", 1, _acars.wind_speed);
  xml += TaXml::writeDouble("wind_dirn", 1, _acars.wind_dirn);
  xml += TaXml::writeDouble("accel_lateral", 1, _acars.accel_lateral);
  xml += TaXml::writeDouble("accel_vertical", 1, _acars.accel_vertical);
  xml += TaXml::writeTime("eta", 1, _acars.eta);
  xml += TaXml::writeDouble("fuel_remain", 1, _acars.fuel_remain);
  xml += TaXml::writeString("flight_number", 1, _acars.flight_number);
  xml += TaXml::writeString("depart_airport", 1, _acars.depart_airport);
  xml += TaXml::writeString("dest_airport", 1, _acars.dest_airport);
  
  if (_message.size() > 0) {
    xml += TaXml::writeString("message", 1, _message);
  }
  
  xml += TaXml::writeEndTag("acars", 0);
  
}

///////////////////////////////////////////
// load message text as XML

void acarsXml::loadMessageXml(string &xml) const
  
{

  xml = "";
  xml += TaXml::writeStartTag("acars_message", 0);
  if (_message.size() > 0) {
    xml += TaXml::writeString("message", 1, _message);
  }
  xml += TaXml::writeEndTag("acars_message", 0);
  
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int acarsXml::disassemble(const void *buf, int len)

{

  reset();

  // does buffer start with full XML?
  // if so, disassemble from XML

  if (len >= (int) strlen("<acars>")) {
    const char *cbuf = (const char *) buf;
    if (strncmp(cbuf, "<acars>", 7) == 0) {
      // starts with XML
      return _disassembleXml(cbuf, len);
    }
  }

  // long enough for struct
  
  if (len < (int) sizeof(acars_t)) {
    return -1;
  }

  // copy in buffer and unswap

  memcpy(&_acars, buf, sizeof(acars_t));
  BE_to_acars(&_acars);

  // do we have message XML?

  const char *messageBuf = (const char *) buf + sizeof(acars_t);
  int messageLen = len - sizeof(acars_t);
  if (messageLen < (int) strlen("<acars_message>")) {
    return 0;
  }

  _disassembleMessageXml(messageBuf, messageLen);

  return 0;
}

///////////////
// print object

void acarsXml::print(FILE *out, string spacer /* = ""*/ ) const

{
  
  acars_print(out, spacer.c_str(), &_acars);
  
  if (_message.size() > 0) {
    string xml;
    loadMessageXml(xml);
    fprintf(out, "%s\n", xml.c_str());
  }

}

///////////////
// print as XML

void acarsXml::printAsXml(ostream &out) const

{

  string xml;
  loadXml(xml);
  out << xml;

}
  
///////////////////////////////////////////////////////////
// Disassembles an XML buffer.
// Returns 0 on success, -1 on failure

int acarsXml::_disassembleXml(const char *buf, int len)

{

  string xml;
  xml.append(buf, len - 1);
  string contents;
  if (TaXml::readString(xml, "acars", contents)) {
    return -1;
  }

  // find the tags

  vector<TaXml::TagLimits> tags;
  TaXml::readTagLimits(contents, 0, tags);
  
  for (int ii = 0; ii < (int) tags.size(); ii++) {
    
    string tag = tags[ii].getTag();
    size_t start = tags[ii].getStartPosn();
    size_t end = tags[ii].getEndPosn();
    size_t len = end - start;
    string content = contents.substr(start, len);

    if (tag == "time") {
      time_t time;
      TaXml::readTime(content, tag, time);
      _acars.time = (ti32) time;
    } else if (tag == "lat") {
      double lat;
      TaXml::readDouble(content, tag, lat);
      _acars.lat = lat;
    } else if (tag == "lon") {
      double lon;
      TaXml::readDouble(content, tag, lon);
      _acars.lon = lon;
    } else if (tag == "alt") {
      double alt;
      TaXml::readDouble(content, tag, alt);
      _acars.alt = alt;
    } else if (tag == "temp") {
      double temp;
      TaXml::readDouble(content, tag, temp);
      _acars.temp = temp;
    } else if (tag == "wind_speed") {
      double wind_speed;
      TaXml::readDouble(content, tag, wind_speed);
      _acars.wind_speed = wind_speed;
    } else if (tag == "wind_dirn") {
      double wind_dirn;
      TaXml::readDouble(content, tag, wind_dirn);
      _acars.wind_dirn = wind_dirn;
    } else if (tag == "accel_lateral") {
      double accel_lateral;
      TaXml::readDouble(content, tag, accel_lateral);
      _acars.accel_lateral = accel_lateral;
    } else if (tag == "accel_vertical") {
      double accel_vertical;
      TaXml::readDouble(content, tag, accel_vertical);
      _acars.accel_vertical = accel_vertical;
    } else if (tag == "eta") {
      time_t eta;
      TaXml::readTime(content, tag, eta);
      _acars.eta = (ti32) eta;
    } else if (tag == "fuel_remain") {
      double fuel_remain;
      TaXml::readDouble(content, tag, fuel_remain);
      _acars.fuel_remain = fuel_remain;
    } else if (tag == "flight_number") {
      string tmpStr;
      TaXml::readString(content, tag, tmpStr);
      STRncopy(_acars.flight_number, tmpStr.c_str(), ACARS_FLIGHTNO_LEN);
    } else if (tag == "depart_airport") {
      string tmpStr;
      TaXml::readString(content, tag, tmpStr);
      STRncopy(_acars.depart_airport, tmpStr.c_str(), ACARS_TEXT_LEN);
    } else if (tag == "dest_airport") {
      string tmpStr;
      TaXml::readString(content, tag, tmpStr);
      STRncopy(_acars.dest_airport, tmpStr.c_str(), ACARS_TEXT_LEN);
    } else if (tag == "message") {
      TaXml::readString(content, tag, _message);
    }
    
  }
  
  return 0;

}

///////////////////////////////////////////////////////////
// Disassembles message text XML
// Returns 0 on success, -1 on failure

int acarsXml::_disassembleMessageXml(const char *buf, int len)

{

  string xml = "";
  xml.append(buf, len - 1);

  string contents;
  if (TaXml::readString(xml, "acars_message", contents)) {
    return -1;
  }

  // find the tags

  vector<TaXml::TagLimits> tags;
  TaXml::readTagLimits(contents, 0, tags);
  
  for (int ii = 0; ii < (int) tags.size(); ii++) {
    
    string tag = tags[ii].getTag();
    size_t start = tags[ii].getStartPosn();
    size_t end = tags[ii].getEndPosn();
    size_t len = end - start;
    string content = contents.substr(start, len);
    
    if (tag == "message") {
      TaXml::readString(content, tag, _message);
    }

  }
  
  return 0;

}

