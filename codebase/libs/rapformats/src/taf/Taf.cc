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
// Taf.cc
//
// C++ class for dealing with TAF - Terminal Aerodrome Forecast
// For TAF details, see ICAO Annex 3
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Oct 2009
//////////////////////////////////////////////////////////////

#define _in_taf_cc

#include <rapformats/Taf.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>

const char *Taf::wxTypes[nWxTypes] =
  { "FZDZ", "FZRA", "SHGR", "SHGS", "SHRA",
    "SHSN", "TSGR", "TSGS", "TSPL", "TSRA", "TSSN",
    "BCFG", "BLDU", "BLSA", "BLSN", "DRDU", "DRSA",
    "DRSN", "FZFG", "MIFG", "PRFG", 
    "DZ", "RA", "SN", "SG", "PL", "DS",
    "SS", "IC", "FG", "BR", "SA", "DU", "HZ",
    "FU", "VA", "SQ", "PO", "FC", "TS" };

using namespace std;

///////////////
// constructor

Taf::Taf()

{
  clear();
}

/////////////
// destructor

Taf::~Taf()

{
  clear();
}

//////////////////////////
// clear all data members

void Taf::clear()

{

  _issueTime = 0;
  _validTime = 0;
  _expireTime = 0;

  _stationId.clear();
  _latitude = missing;
  _longitude = missing;
  _elevationM = missing;

  _isNil = false;
  _isAmended = false;
  _isCorrected = false;
  _isCancelled = false;
  _cancelTime = 0;
  
  _text.clear();
  _xml.clear();
  _memBuf.free();
  _periods.clear();

}

///////////////////////////////////////////
// assemble()
// Assembles buffer as XML.

void Taf::assemble()

{

  // check mem buffer is free
  
  _memBuf.free();
  
  // convert to XML string
  
  loadXml(_xml);

  // add xml string to buffer, including trailing null
  
  _memBuf.add(_xml.c_str(), _xml.size() + 1);

}

///////////////////////////////////////////
// load XML string

void Taf::loadXml(string &xml, int startIndentLevel /* = 0 */) const
  
{
  
  int sil = startIndentLevel;

  // print object to string as XML
  
  xml = "";
  
  xml += TaXml::writeStartTag("taf", sil+0);
  
  xml += TaXml::writeString("station_id", sil+1, _stationId);
  xml += TaXml::writeTime("issue_time", sil+1, _issueTime);
  xml += TaXml::writeTime("valid_time", sil+1, _validTime);
  xml += TaXml::writeTime("expire_time", sil+1, _expireTime);
  xml += TaXml::writeDouble("latitude", sil+1, _latitude);
  xml += TaXml::writeDouble("longitude", sil+1, _longitude);
  xml += TaXml::writeDouble("elevation_m", sil+1, _elevationM);

  xml += TaXml::writeBoolean("is_nil", sil+1, _isNil);
  xml += TaXml::writeBoolean("is_amended", sil+1, _isAmended);
  xml += TaXml::writeBoolean("is_corrected", sil+1, _isCorrected);
  xml += TaXml::writeBoolean("is_cancelled", sil+1, _isCancelled);
  if (_isCancelled) {
    xml += TaXml::writeTime("cancel_time", sil+1, _cancelTime);
  }
  
  if (_text.size() > 0) {
    xml += TaXml::writeString("text", sil+1, _text);
  }

  for (int ii = 0; ii < (int) _periods.size(); ii++) {
    const ForecastPeriod &period = _periods[ii];
    xml += TaXml::writeStartTag("period", sil+1);
    xml += TaXml::writeTime("start_time", sil+2, period.startTime);
    xml += TaXml::writeTime("end_time", sil+2, period.endTime);
    xml += TaXml::writeString("type", sil+2, periodType2Str(period.pType));
    xml += TaXml::writeDouble("prob_percent", sil+2, period.probPercent);
    xml += TaXml::writeDouble("wind_dirn_degt", sil+2, period.windDirnDegT);
    xml += TaXml::writeBoolean("wind_dirn_vrb", sil+2, period.windDirnVrb);
    xml += TaXml::writeDouble("wind_speed_kmh", sil+2, period.windSpeedKmh);
    xml += TaXml::writeDouble("wind_gust_kmh", sil+2, period.windGustKmh);
    xml += TaXml::writeDouble("vis_km", sil+2, period.visKm);
    xml += TaXml::writeBoolean("is_cavok", sil+2, period.isCavok);
    xml += TaXml::writeDouble("ceiling_km", sil+2, period.ceilingKm);
    for (int jj = 0; jj < (int) period.layers.size(); jj++) {
      const CloudLayer &layer = period.layers[jj];
      xml += TaXml::writeStartTag("layer", sil+2);
      xml += TaXml::writeDouble("height_km", sil+3, layer.heightKm);
      xml += TaXml::writeDouble("cloud_cover", sil+3, layer.cloudCover);
      xml += TaXml::writeBoolean("cb_present", sil+3, layer.cbPresent);
      xml += TaXml::writeEndTag("layer", sil+2);
    }
    for (int jj = 0; jj < (int) period.wx.size(); jj++) {
      xml += TaXml::writeString("wx", sil+2, period.wx[jj]);
    }
    if (period.maxTempTime != 0) {
      xml += TaXml::writeDouble("max_temp_c", sil+2, period.maxTempC);
      xml += TaXml::writeTime("max_temp_time", sil+2, period.maxTempTime);
    }
    if (period.minTempTime != 0) {
      xml += TaXml::writeDouble("min_temp_c", sil+2, period.minTempC);
      xml += TaXml::writeTime("min_temp_time", sil+2, period.minTempTime);
    }
    xml += TaXml::writeString("text", sil+2, period.text);
    xml += TaXml::writeEndTag("period", sil+1);
  } // ii
  
  xml += TaXml::writeEndTag("taf", sil+0);
  
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Returns 0 on success, -1 on failure

int Taf::disassemble(const void *buf, int len)

{

  clear();
  _xml.clear();
  _xml.append((char *) buf, len - 1);

  string tafStr;
  if (TaXml::readString(_xml, "taf", tafStr)) {
    return -1;
  }

  // find the tags
  
  vector<TaXml::TagLimits> tags;
  TaXml::readTagLimits(tafStr, 0, tags);
  
  for (int ii = 0; ii < (int) tags.size(); ii++) {
    
    string tag = tags[ii].getTag();
    size_t start = tags[ii].getStartPosn();
    size_t end = tags[ii].getEndPosn();
    size_t len = end - start;
    string content = tafStr.substr(start, len);
    
    if (tag == "station_id") {
      TaXml::readString(content, tag, _stationId);
    } else if (tag == "issue_time") {
      TaXml::readTime(content, tag, _issueTime);
    } else if (tag == "valid_time") {
      TaXml::readTime(content, tag, _validTime);
    } else if (tag == "expire_time") {
      TaXml::readTime(content, tag, _expireTime);
    } else if (tag == "latitude") {
      TaXml::readDouble(content, tag, _latitude);
    } else if (tag == "longitude") {
      TaXml::readDouble(content, tag, _longitude);
    } else if (tag == "elevation_m") {
      TaXml::readDouble(content, tag, _elevationM);
    } else if (tag == "text") {
      TaXml::readString(content, tag, _text);
    } else if (tag == "is_nil") {
      TaXml::readBoolean(content, tag, _isNil);
    } else if (tag == "is_amended") {
      TaXml::readBoolean(content, tag, _isAmended);
    } else if (tag == "is_corrected") {
      TaXml::readBoolean(content, tag, _isCorrected);
    } else if (tag == "is_cancelled") {
      TaXml::readBoolean(content, tag, _isCancelled);
    } else if (tag == "cancel_time") {
      TaXml::readTime(content, tag, _cancelTime);
    }

    vector<string> periodsStr;
    TaXml::readStringArray(content, "period", periodsStr);
    for (int jj = 0; jj < (int) periodsStr.size(); jj++) {
      const string  &periodStr = periodsStr[jj];
      ForecastPeriod period;
      if (TaXml::readTime(periodStr, "start_time", period.startTime)) {
        period.startTime = 0;
      }
      if (TaXml::readTime(periodStr, "end_time", period.endTime)) {
        period.endTime = 0;
      }
      string pTypeStr;
      if (TaXml::readString(periodStr, "type", pTypeStr)) {
        period.pType = PERIOD_MAIN;
      } else {
        period.pType = periodTypeStr2Enum(pTypeStr);
      }
      if (TaXml::readDouble(periodStr, "prob_percent", period.probPercent)) {
        period.probPercent = missing;
      }
      if (TaXml::readDouble(periodStr, "wind_dirn_degt", period.windDirnDegT)) {
        period.windDirnDegT = missing;
      }
      if (TaXml::readBoolean(periodStr, "wind_dirn_vrb", period.windDirnVrb)) {
        period.windDirnDegT = missing;
      }
      if (TaXml::readDouble(periodStr, "wind_speed_kmh", period.windSpeedKmh)) {
        period.windSpeedKmh = missing;
      }
      if (TaXml::readDouble(periodStr, "wind_gust_kmh", period.windGustKmh)) {
        period.windGustKmh = missing;
      }
      if (TaXml::readDouble(periodStr, "vis_km", period.visKm)) {
        period.visKm = missing;
      }
      if (TaXml::readBoolean(periodStr, "is_cavok", period.isCavok)) {
        period.isCavok = false;
      }
      if (TaXml::readDouble(periodStr, "ceiling_km", period.ceilingKm)) {
        period.ceilingKm = missing;
      }
      vector<string> layersStr;
      TaXml::readStringArray(periodStr, "layer", layersStr);
      for (int kk = 0; kk < (int) layersStr.size(); kk++) {
        const string  &layerStr = layersStr[kk];
        CloudLayer layer;
        if (TaXml::readDouble(layerStr, "height_km", layer.heightKm)) {
          layer.heightKm = missing;
        }
        if (TaXml::readDouble(layerStr, "cloud_cover", layer.cloudCover)) {
          layer.cloudCover = missing;
        }
        if (TaXml::readBoolean(layerStr, "cb_present", layer.cbPresent)) {
          layer.cbPresent = false;
        }
        period.layers.push_back(layer);
      } // kk
      vector<string> wxsStr;
      TaXml::readStringArray(periodStr, "wx", wxsStr);
      for (int kk = 0; kk < (int) wxsStr.size(); kk++) {
        period.wx.push_back(wxsStr[kk]);
      }
      if (TaXml::readDouble(periodStr, "max_temp_c", period.maxTempC)) {
        period.maxTempC = missing;
      }
      if (TaXml::readTime(periodStr, "max_temp_time", period.maxTempTime)) {
        period.maxTempTime = 0;
      }
      if (TaXml::readDouble(periodStr, "min_temp_c", period.minTempC)) {
        period.minTempC = missing;
      }
      if (TaXml::readTime(periodStr, "min_temp_time", period.minTempTime)) {
        period.minTempTime = 0;
      }
      string text;
      if (TaXml::readString(periodStr, "text", text) == 0) {
        period.text = text;
      }
      _periods.push_back(period);
    } // jj
    
  } // ii
  
  return 0;
  
}

///////////////
// print object

void Taf::print(ostream &out, string spacer /* = ""*/ ) const

{
  
  out << spacer << "=================== TAF =================" << endl;

  out << spacer << "---------- Location and Time -----------" << endl;
  out << spacer << "stationId: " << _stationId << endl;
  out << spacer << "issueTime: " << DateTime::strm(_issueTime) << endl;
  out << spacer << "validTime: " << DateTime::strm(_validTime) << endl;
  out << spacer << "expireTime: " << DateTime::strm(_expireTime) << endl;
  out << spacer << "latitude: " << _latitude << endl;
  out << spacer << "longitude: " << _longitude << endl;
  out << spacer << "elevationM (ft): " << _elevationM
      << " (" << _elevationM / FEET_TO_M << ")" << endl;
  out << spacer << "isNil: " << string(_isNil? "Y" : "N") << endl;
  out << spacer << "isAmended: " << string(_isAmended? "Y" : "N") << endl;
  out << spacer << "isCorrected: " << string(_isCorrected? "Y" : "N") << endl;
  out << spacer << "isCancelled: " << string(_isCancelled? "Y" : "N") << endl;
  if (_isCancelled) {
    out << spacer << "cancelTime: " << DateTime::strm(_cancelTime) << endl;
  }
  out << spacer << "-----------------------------------------" << endl;
  out << spacer << "text: " << _text << endl;
  for (int ii = 0; ii < (int) _periods.size(); ii++) {
    out << spacer << "-----------------------------------------" << endl;
    const ForecastPeriod &period = _periods[ii];
    out << spacer << "Period num: " << ii << endl;
    out << spacer << "  startTime: "
        << DateTime::strm(period.startTime) << endl;
    out << spacer << "  endTime: " << DateTime::strm(period.endTime) << endl;
    out << spacer << "  type: " << periodType2Str(period.pType) << endl;
    out << spacer << "  windDirnDegT: " << period.windDirnDegT << endl;
    out << spacer << "  windDirnVrb: "
        << string(period.windDirnVrb? "Y" : "N") << endl;
    out << spacer << "  windSpeedKmh (kts): " << period.windSpeedKmh
        << " (" << period.windSpeedKmh / KM_PER_NM << ")" << endl;
    out << spacer << "  windGustKmh (kts): " << period.windGustKmh
        << " (" << period.windGustKmh / KM_PER_NM << ")" << endl;
    out << spacer << "  visKm (sm): " << period.visKm
        << " (" << (period.visKm / KM_PER_MI) << ")" << endl;
    out << spacer << "  isCavok: " << string(period.isCavok? "Y" : "N") << endl;
    out << spacer << "  ceilingKm (ft): " << period.ceilingKm 
        << " (" << (period.ceilingKm / FEET_TO_KM) << ")" << endl;
    for (int jj = 0; jj < (int) period.layers.size(); jj++) {
      const CloudLayer &layer = period.layers[jj];
      out << spacer << "  Cloud layer num: " << jj << endl;
      out << spacer << "    heightKm (ft): " << layer.heightKm
          << " (" << (layer.heightKm / FEET_TO_KM) << ")" << endl;
      out << spacer << "    cloudCover: " << layer.cloudCover << endl;
      out << spacer << "    CB present: " << string(layer.cbPresent? "Y" : "N") << endl;
    } // jj
    if (period.wx.size() > 0) {
      out << spacer << "  wx:";
      for (int jj = 0; jj < (int) period.wx.size(); jj++) {
        out << " " << period.wx[jj];
      }
      out << endl;
    }
    if (period.maxTempTime != 0) {
      out << spacer << "  maxTempC: " << period.maxTempC << endl;
      out << spacer << "  maxTempTime: "
          << DateTime::strm(period.maxTempTime) << endl;
    }
    if (period.minTempTime != 0) {
      out << spacer << "  minTempC: " << period.minTempC << endl;
      out << spacer << "  minTempTime: "
          << DateTime::strm(period.minTempTime) << endl;
    }
    out << spacer << "  text: " << period.text << endl;
  } // ii
  out << spacer << "=========================================" << endl;

}

///////////////
// print as XML

void Taf::printAsXml(ostream &out, int startIndentLevel /* = 0 */) const

{

  if (_xml.size() == 0 || startIndentLevel != 0) {
    loadXml(_xml, startIndentLevel);
  }

  out << _xml;

}
  
//////////////////////////////////////////////
// tokenize a string into a vector of strings

void Taf::_tokenize(const string &str,
                    const string &spacer,
                    vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

//////////////////////////////////////////////
// convert period type to string

string Taf::periodType2Str(period_type_t ptype)
  
{
    
  switch (ptype) {
    case PERIOD_BECMG:
      return "BECMG";
    case PERIOD_TEMPO:
      return "TEMPO";
    case PERIOD_FROM:
      return "FROM";
    case PERIOD_PROB:
      return "PROB";
    default:
      return "MAIN";
  }

}

//////////////////////////////////////////////
// convert period type string to enum

Taf::period_type_t Taf::periodTypeStr2Enum(const string ptypeStr)
  
{

  if (ptypeStr == "BECMG") {
    return PERIOD_BECMG;
  } else if (ptypeStr == "TEMPO") {
    return PERIOD_TEMPO;
  } else if (ptypeStr == "FROM") {
    return PERIOD_FROM;
  } else if (ptypeStr == "PROB") {
    return PERIOD_PROB;
  } else {
    return PERIOD_MAIN;
  }

}

