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
// pirepXml.cc
//
// C++ class for wrapping pirep.h, and adding XML components.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// March 2007
//////////////////////////////////////////////////////////////

#include <rapformats/pirepXml.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>

///////////////
// constructor

pirepXml::pirepXml()
  
{
  pirep_init(&_pirep);
}

/////////////
// destructor

pirepXml::~pirepXml()

{

}

//////////////////////////
// reset all data members

void pirepXml::reset()

{

  pirep_init(&_pirep);
  _message = "";
  _memBuf.free();

}

///////////////////////////////////////////
// assemble buffer starting with pirep_t struct
// and optionally followed by message text as XML

void pirepXml::assemble(bool appendMessageXml)

{

  // byte swap struct

  pirep_t be_pirep;
  memcpy(&be_pirep, &_pirep, sizeof(pirep_t));
  BE_from_pirep(&be_pirep);
  
  // check mem buffer is free
  
  _memBuf.free();
  
  // add byte-swapped struct to buffer
  
  _memBuf.add(&be_pirep, sizeof(pirep_t));
  
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

void pirepXml::assembleAsXml()
  
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

void pirepXml::loadXml(string &xml) const
  
{

  // print object to string as XML

  xml = "";

  xml += TaXml::writeStartTag("pirep", 0);

  xml += TaXml::writeTime("time", 1, _pirep.time);
  xml += TaXml::writeDouble("lat", 1, _pirep.lat);
  xml += TaXml::writeDouble("lon", 1, _pirep.lon);
  xml += TaXml::writeDouble("alt", 1, _pirep.alt);
  xml += TaXml::writeDouble("temp", 1, _pirep.temp);
  xml += TaXml::writeDouble("visibility", 1, _pirep.visibility);
  xml += TaXml::writeDouble("wind_speed", 1, _pirep.wind_speed);
  xml += TaXml::writeDouble("wind_dirn", 1, _pirep.wind_dirn);
  xml += TaXml::writeInt("turb_fl_base", 1, _pirep.turb_fl_base);
  xml += TaXml::writeInt("turb_fl_top", 1, _pirep.turb_fl_top);
  xml += TaXml::writeInt("icing_fl_base", 1, _pirep.icing_fl_base);
  xml += TaXml::writeInt("icing_fl_top", 1, _pirep.turb_fl_top);
  xml += TaXml::writeInt("sky_fl_base", 1, _pirep.sky_fl_base);
  xml += TaXml::writeInt("sky_fl_top", 1, _pirep.sky_fl_top);
  xml += TaXml::writeInt("turb_freq", 1, _pirep.turb_freq);
  xml += TaXml::writeInt("turb_index", 1, _pirep.turb_index);
  xml += TaXml::writeInt("icing_index", 1, _pirep.icing_index);
  xml += TaXml::writeInt("sky_index", 1, _pirep.sky_index);
  xml += TaXml::writeString("callsign", 1, _pirep.callsign);
  xml += TaXml::writeString("short_message", 1, _pirep.text);

  if (_message.size() > 0) {
    xml += TaXml::writeString("message", 1, _message);
  }
  
  xml += TaXml::writeEndTag("pirep", 0);
  
}

///////////////////////////////////////////
// load message text as XML

void pirepXml::loadMessageXml(string &xml) const
  
{

  xml = "";
  xml += TaXml::writeStartTag("pirep_message", 0);
  if (_message.size() > 0) {
    xml += TaXml::writeString("message", 1, _message);
  }
  xml += TaXml::writeEndTag("pirep_message", 0);
  
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int pirepXml::disassemble(const void *buf, int len)

{

  reset();

  // does buffer start with full XML?
  // if so, disassemble from XML

  if (len >= (int) strlen("<pirep>")) {
    const char *cbuf = (const char *) buf;
    if (strncmp(cbuf, "<pirep>", 7) == 0) {
      // starts with XML
      return _disassembleXml(cbuf, len);
    }
  }

  // long enough for struct
  
  if (len < (int) sizeof(pirep_t)) {
    return -1;
  }

  // copy in buffer and unswap

  memcpy(&_pirep, buf, sizeof(pirep_t));
  BE_to_pirep(&_pirep);

  // do we have message XML?

  const char *messageBuf = (const char *) buf + sizeof(pirep_t);
  int messageLen = len - sizeof(pirep_t);
  if (messageLen < (int) strlen("<pirep_message>")) {
    return 0;
  }

  _disassembleMessageXml(messageBuf, messageLen);

  return 0;
}

///////////////
// print object

void pirepXml::print(FILE *out, string spacer /* = ""*/ ) const

{
  
  pirep_print(out, spacer.c_str(), &_pirep);
  
  if (_message.size() > 0) {
    string xml;
    loadMessageXml(xml);
    fprintf(out, "%s\n", xml.c_str());
  }

}

///////////////
// print as XML

void pirepXml::printAsXml(ostream &out) const

{

  string xml;
  loadXml(xml);
  out << xml;

}
  
///////////////////////////////////////////////////////////
// Disassembles an XML buffer.
// Returns 0 on success, -1 on failure

int pirepXml::_disassembleXml(const char *buf, int len)

{

  string xml;
  xml.append(buf, len - 1);
  string contents;
  if (TaXml::readString(xml, "pirep", contents)) {
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

    if (tag == "callsign") {
      string tmpStr;
      TaXml::readString(content, tag, tmpStr);
      STRncopy(_pirep.callsign, tmpStr.c_str(), PIREP_CALLSIGN_LEN);
    } else if (tag == "message") {
      TaXml::readString(content, tag, _message);
    } else if (tag == "short_message") {
      string tmpStr;
      TaXml::readString(content, tag, tmpStr);
      STRncopy(_pirep.text, tmpStr.c_str(), PIREP_TEXT_LEN);
    } else if (tag == "time") {
      time_t time;
      TaXml::readTime(content, tag, time);
      _pirep.time = (ti32) time;
    } else if (tag == "lat") {
      double lat;
      TaXml::readDouble(content, tag, lat);
      _pirep.lat = lat;
    } else if (tag == "lon") {
      double lon;
      TaXml::readDouble(content, tag, lon);
      _pirep.lon = lon;
    } else if (tag == "alt") {
      double alt;
      TaXml::readDouble(content, tag, alt);
      _pirep.alt = alt;
    } else if (tag == "temp") {
      double temp;
      TaXml::readDouble(content, tag, temp);
      _pirep.temp = temp;
    } else if (tag == "visibility") {
      double visibility;
      TaXml::readDouble(content, tag, visibility);
      _pirep.visibility = visibility;
    } else if (tag == "wind_speed") {
      double wind_speed;
      TaXml::readDouble(content, tag, wind_speed);
      _pirep.wind_speed = wind_speed;
    } else if (tag == "wind_dirn") {
      double wind_dirn;
      TaXml::readDouble(content, tag, wind_dirn);
      _pirep.wind_dirn = wind_dirn;
    } else if (tag == "turb_fl_base") {
      int turb_fl_base;
      TaXml::readInt(content, tag, turb_fl_base);
      _pirep.turb_fl_base = turb_fl_base;
    } else if (tag == "turb_fl_top") {
      int turb_fl_top;
      TaXml::readInt(content, tag, turb_fl_top);
      _pirep.turb_fl_top = turb_fl_top;
    } else if (tag == "icing_fl_base") {
      int icing_fl_base;
      TaXml::readInt(content, tag, icing_fl_base);
      _pirep.icing_fl_base = icing_fl_base;
    } else if (tag == "icing_fl_top") {
      int icing_fl_top;
      TaXml::readInt(content, tag, icing_fl_top);
      _pirep.icing_fl_top = icing_fl_top;
    } else if (tag == "sky_fl_base") {
      int sky_fl_base;
      TaXml::readInt(content, tag, sky_fl_base);
      _pirep.sky_fl_base = sky_fl_base;
    } else if (tag == "sky_fl_top") {
      int sky_fl_top;
      TaXml::readInt(content, tag, sky_fl_top);
      _pirep.sky_fl_top = sky_fl_top;
    } else if (tag == "turb_freq") {
      int turb_freq;
      TaXml::readInt(content, tag, turb_freq);
      _pirep.turb_freq = turb_freq;
    } else if (tag == "turb_index") {
      int turb_index;
      TaXml::readInt(content, tag, turb_index);
      _pirep.turb_index = turb_index;
    } else if (tag == "icing_index") {
      int icing_index;
      TaXml::readInt(content, tag, icing_index);
      _pirep.icing_index = icing_index;
    } else if (tag == "sky_index") {
      int sky_index;
      TaXml::readInt(content, tag, sky_index);
      _pirep.sky_index = sky_index;
    }
    
  }
  
  return 0;

}

///////////////////////////////////////////////////////////
// Disassembles message text XML
// Returns 0 on success, -1 on failure

int pirepXml::_disassembleMessageXml(const char *buf, int len)

{

  string xml = "";
  xml.append(buf, len - 1);

  string contents;
  if (TaXml::readString(xml, "pirep_message", contents)) {
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

