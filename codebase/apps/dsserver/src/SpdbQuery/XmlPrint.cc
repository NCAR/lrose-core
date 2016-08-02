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
//////////////////////////////////////////////////////////
// XmlPrint.cc
//
// XmlPrinting class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
//////////////////////////////////////////////////////////

#include "XmlPrint.hh"
#include <Spdb/Symprod.hh>
#include <rapformats/LtgWrapper.hh>
#include <rapformats/pirepXml.hh>
#include <rapformats/SigAirMet.hh>
#include <rapformats/Taf.hh>
#include <rapformats/Amdar.hh>
#include <rapformats/WxObs.hh>
#include <rapformats/ac_georef.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/TaStr.hh>

using namespace std;

// Constructor

XmlPrint::XmlPrint(FILE *out, ostream &ostr,
                   const Args &args) :
        _out(out), _ostr(ostr), _args(args)
        
{

  // set up spatial limits

  _minLat = -90.0;
  _maxLat = 90.0;
  _minLon = -360.0;
  _maxLon = 360.0;
  
  if (_args.horizLimitsSet) {
    _minLat = _args.minLat;
    _maxLat = _args.maxLat;
    _minLon = _args.minLon;
    _maxLon = _args.maxLon;
    if (_minLon > _maxLon) {
      _minLon -= 360.0;
    }
  }  

}

// Destructor

XmlPrint::~XmlPrint()

{

}

///////////////////////////////////////////////////////////////
// CHUNK_HDR

void XmlPrint::chunkHdr(const Spdb::chunk_t &chunk,
                        int startIndentLevel)
{

  int sil = startIndentLevel;
  string xmlStr;
  xmlStr += TaXml::writeStartTag("chunk_header", sil + 0);

  bool print_hashed = true;
  string data_type_str; 
  if(chunk.data_type > 1048576) { // These are probably 4 char hashes.
	  data_type_str = Spdb::dehashInt32To4Chars(chunk.data_type);
  } else if (chunk.data_type > 0)  { // A numerical ID
	  print_hashed = false;
  } else {
	  data_type_str = Spdb::dehashInt32To5Chars(chunk.data_type);
  }

  // Check for non printable chars.
  for (size_t i = 0; i < data_type_str.size(); i++) {
    if (!isprint(data_type_str[i])) {
	  print_hashed = false;
    }
  }

  xmlStr += TaXml::writeInt("data_type", sil + 1, chunk.data_type);
  if (print_hashed) {
    xmlStr += TaXml::writeString("data_type_hash", sil + 1, data_type_str);
  }

  bool print_hashed2 = true;
  string data_type2_str = Spdb::dehashInt32To4Chars(chunk.data_type2);
  if (data_type2_str.size() < 1) {
    print_hashed2 = false;
  } else {
    for (size_t i = 0; i < data_type2_str.size(); i++) {
      if (!isprint(data_type2_str[i])) {
        print_hashed2 = false;
      }
    }
  }

  xmlStr += TaXml::writeInt("data_type2", sil + 1, chunk.data_type2);
  if (print_hashed2) {
    xmlStr +=
      TaXml::writeString("data_type_hash", sil + 1, 
                         Spdb::dehashInt32To4Chars(chunk.data_type2));
  }
  xmlStr += TaXml::writeTime("valid_time", sil + 1, chunk.valid_time);
  xmlStr += TaXml::writeTime("expire_time", sil + 1, chunk.expire_time);
  if (chunk.write_time > 0) {
    xmlStr += TaXml::writeTime("write_time", sil + 1, chunk.write_time);
  }
  // xmlStr += TaXml::writeInt("len", sil + 1, chunk.len);

  if (chunk.current_compression == Spdb::COMPRESSION_GZIP) {
    TaXml::writeString("current_compression", sil + 1, "gzip");
  } else if (chunk.current_compression == Spdb::COMPRESSION_BZIP2) {
    TaXml::writeString("current_compression", sil + 1, "bzip2");
  } else {
    TaXml::writeString("current_compression", sil + 1, "none");
  }
  
  if (chunk.stored_compression == Spdb::COMPRESSION_GZIP) {
    TaXml::writeString("stored_compression", sil + 1, "gzip");
  } else if (chunk.stored_compression == Spdb::COMPRESSION_BZIP2) {
    TaXml::writeString("stored_compression", sil + 1, "bzip2");
  } else {
    TaXml::writeString("stored_compression", sil + 1, "none");
  }
  
  if (chunk.tag.size() > 0) {
    TaXml::writeString("tag", sil + 1, chunk.tag);
  }

  xmlStr += TaXml::writeEndTag("chunk_header", sil + 0);

  _ostr << xmlStr;

}

///////////////////////////////////////////////////////////////
// LTG : Print lightning data
///

void XmlPrint::ltg(const Spdb::chunk_t &chunk,
                   int startIndentLevel)
{
  
  // sanity check

  if (chunk.len < (int) sizeof(ui32)) {
    return;
  }
  
  // check cookie so we know if we have extended data

  ui32 cookie;
  memcpy(&cookie, chunk.data, sizeof(cookie));

  // do we have any valid strikes?

  vector<LTG_extended_t> extended;
  vector<LTG_strike_t> strikes;
  
  if (cookie == LTG_EXTENDED_COOKIE) {

    int num_strikes = chunk.len / sizeof(LTG_extended_t);
    LTG_extended_t *ltg_data = static_cast<LTG_extended_t *>(chunk.data);
    for (int istrike = 0; istrike < num_strikes; istrike++) {
      LTG_extended_t strike = ltg_data[istrike];
      LTG_extended_from_BE(&strike);
      if (_checkLatLon(strike.latitude, strike.longitude)) {
        extended.push_back(strike);
      }
    }

  } else {

    int num_strikes = chunk.len / sizeof(LTG_strike_t);
    LTG_strike_t *ltg_data = static_cast<LTG_strike_t *>(chunk.data);
    for (int istrike = 0; istrike < num_strikes; istrike++) {
      LTG_strike_t strike = ltg_data[istrike];
      LTG_from_BE(&strike);
      if (_checkLatLon(strike.latitude, strike.longitude)) {
        strikes.push_back(strike);
      }
    }

  }

  // print striks

  if (extended.size() > 0) {

    int sil = startIndentLevel;
    for (int ii = 0; ii < (int) extended.size(); ii++) {
      _ostr << TaXml::writeStartTag("entry", sil);
      if (_args.printXmlHeaders) {
        this->chunkHdr(chunk, sil + 1);
      }
      LtgWrapper::printAsXml(_ostr, extended[ii], sil + 1);
      _ostr << TaXml::writeEndTag("entry", sil);
    }

  } else if (strikes.size() > 0) {

    int sil = startIndentLevel;
    for (int ii = 0; ii < (int) strikes.size(); ii++) {
      _ostr << TaXml::writeStartTag("entry", sil);
      if (_args.printXmlHeaders) {
        this->chunkHdr(chunk, sil + 1);
      }
      LtgWrapper::printAsXml(_ostr, strikes[ii], sil + 1);
      _ostr << TaXml::writeEndTag("entry", sil);
    }

  }

}


///////////////////////////////////////////////////////////////
// AC_GEOREF : Aircraft georeference data
///

void XmlPrint::acGeoref(const Spdb::chunk_t &chunk,
                        int startIndentLevel)
{
  
  // sanity check

  if (chunk.len < (int) sizeof(ac_georef_t)) {
    return;
  }

  // get the data and swap it as needed

  ac_georef_t georef;
  memcpy(&georef, chunk.data, sizeof(ac_georef_t));
  BE_to_ac_georef(&georef);

  // do print
  
  int sil = startIndentLevel;
  _ostr << TaXml::writeStartTag("entry", sil);
  if (_args.printXmlHeaders) {
    this->chunkHdr(chunk, sil + 1);
  }
  ac_georef_print_as_xml(&georef, _ostr, sil + 1);
  _ostr << TaXml::writeEndTag("entry", sil);

}


///////////////////////////////////////////////////////////////
// SIGMET: Data in the SIG/AIRMET class
//

void XmlPrint::sigmet(const Spdb::chunk_t &chunk,
		      int startIndentLevel)
{

  SigAirMet sigmet;
  if (sigmet.disassemble(chunk.data, chunk.len)) {
    cerr << "ERROR - SpdbQuery::XmlPrint::sigmet" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  
  // check location.  The sigmet will be considered to be inside the area
  // if any vertex is within the area

  const vector< sigairmet_vertex_t > vertices = sigmet.getVertices();
  vector< sigairmet_vertex_t >::const_iterator vertex;
  bool in_area = false;
  
  for (vertex = vertices.begin(); vertex != vertices.end(); ++vertex)
  {
    if (_checkLatLon(vertex->lat, vertex->lon))
    {
      in_area = true;
      break;
    }
  }
  
  if (!in_area)
    return;
  
  // do print
  
  int sil = startIndentLevel;
  _ostr << TaXml::writeStartTag("entry", sil);
  if (_args.printXmlHeaders) {
    this->chunkHdr(chunk, sil + 1);
  }
  sigmet.printAsXml(_ostr, sil + 1);
  _ostr << TaXml::writeEndTag("entry", sil);
  
}

///////////////////////////////////////////////////////////////
// TAF: Data in the TAF class
//

void XmlPrint::taf(const Spdb::chunk_t &chunk,
                   int startIndentLevel)

{
  Taf taf;
  if (taf.disassemble(chunk.data, chunk.len)) {
    cerr << "ERROR - SpdbQuery::XmlPrint::taf" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  
  // check location

  if (!_checkLatLon(taf.getLatitude(), taf.getLongitude())) {
    // outside bounds
    return;
  }

  // do print
  
  int sil = startIndentLevel;
  _ostr << TaXml::writeStartTag("entry", sil);
  if (_args.printXmlHeaders) {
    this->chunkHdr(chunk, sil + 1);
  }
  taf.printAsXml(_ostr, sil + 1);
  _ostr << TaXml::writeEndTag("entry", sil);
  
}

///////////////////////////////////////////////////////////////
// AMDAR: Data in the AMDAR class
//

void XmlPrint::amdar(const Spdb::chunk_t &chunk,
                   int startIndentLevel)

{

  Amdar amdar;
  if (amdar.disassemble(chunk.data, chunk.len)) {
    cerr << "ERROR - SpdbQuery::XmlPrint::amdar" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  
  // check location

  if (!_checkLatLon(amdar.getLatitude(), amdar.getLongitude())) {
    // outside bounds
    return;
  }

  // do print
  
  int sil = startIndentLevel;
  _ostr << TaXml::writeStartTag("entry", sil);
  if (_args.printXmlHeaders) {
    this->chunkHdr(chunk, sil + 1);
  }
  amdar.printAsXml(_ostr, sil + 1);
  _ostr << TaXml::writeEndTag("entry", sil);
  
}

///////////////////////////////////////////////////////////////
// WxObs : Print weather observation

void XmlPrint::wxObs(const Spdb::chunk_t &chunk,
                     int startIndentLevel)
{

  // decode WxObs object

  WxObs obs;
  obs.disassemble(chunk.data, chunk.len);

  // check location

  if (!_checkLatLon(obs.getLatitude(), obs.getLongitude())) {
    // outside bounds
    return;
  }

  // do print

  int sil = startIndentLevel;
  _ostr << TaXml::writeStartTag("entry", sil);
  if (_args.printXmlHeaders) {
    this->chunkHdr(chunk, sil + 1);
  }
  obs.printAsXml(_ostr, sil + 1);
  _ostr << TaXml::writeEndTag("entry", sil);
  
}

///////////////////////////////////////////////////////////////
// Already in XMl

void XmlPrint::xml(const Spdb::chunk_t &chunk,
                   int startIndentLevel)
{
  
  if (chunk.len == 0) {
    return;
  }

  // get data as string

  char *str = (char *) chunk.data;
  // ensure null-terminated string
  str[chunk.len - 1] = '\0';

  // check for <latitude> and <longitude> if in the XML

  double lat, lon;
  if ((TaXml::readDouble(str, "latitude", lat) == 0) &&
      (TaXml::readDouble(str, "longitude", lon) == 0)) {
    if (!_checkLatLon(lat, lon)) {
      // outside bounds
      return;
    }
  }

  // tokenize the string on line feeds
  
  vector<string> lines;
  TaStr::tokenize(str, "\n", lines);

  // print lines

  int sil = startIndentLevel;
  _ostr << TaXml::writeStartTag("entry", sil);
  if (_args.printXmlHeaders) {
    this->chunkHdr(chunk, sil + 1);
  }
  int nIndent = (sil + 1) * TaXml::indentPerLevel;
  for (int ii = 0; ii < (int) lines.size(); ii++) {
    for (int jj = 0; jj < nIndent; jj++) {
      _ostr << " ";
    }
    _ostr << lines[ii] << endl;
  }
  _ostr << TaXml::writeEndTag("entry", sil);
  
}

///////////////////////////////////////////////////////////////
// Check lat/lon
// Returns true if we should print, false otherwise

bool XmlPrint::_checkLatLon(double lat, double lon)
{

  // do we need to check?

  if (!_args.horizLimitsSet) {
    return true;
  }

  // is lat within bounds?

  if (lat < _args.minLat || lat > _args.maxLat) {
    return false;
  }

  // is lon within bounds
  
  if (lon >= _minLon && lon <= _maxLon) {
    return true;
  }

  // try 360 below
  
  if ((lon - 360.0) >= _minLon && (lon - 360.0) <= _maxLon) {
    return true;
  }

  // try 360 above
  
  if ((lon + 360.0) >= _minLon && (lon + 360.0) <= _maxLon) {
    return true;
  }

  // failure

  return false;

}

