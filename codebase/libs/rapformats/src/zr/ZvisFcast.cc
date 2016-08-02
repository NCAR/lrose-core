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
// ZvisFcast.cc
//
// C++ class for dealing with radar sweep information
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// August 2006
//////////////////////////////////////////////////////////////

#include <rapformats/ZvisFcast.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <iomanip>
using namespace std;

///////////////
// constructor

ZvisFcast::ZvisFcast()
  
{
  
  _versionNum = 1;
  clear();

}

/////////////
// destructor

ZvisFcast::~ZvisFcast()

{
  clear();
}

//////////////////////////
// clear all data members

void ZvisFcast::clear()

{

  _visUnits = VIS_UNITS_MI;

  _calTime = 0;
  _calStatus = CAL_DEFAULTS;
  _calIntercept = 1.46;
  _calSlope = -0.035;

  _locationName = "";
  _locationLat = 0.0;
  _locationLon = 0.0;

  _cats.clear();

  _genTime = 0;
  _deltaSecs = 0;
  _entries.clear();

  _memBuf.reset();
  
}

//////////////////////////
// add a category

void ZvisFcast::addCategory(const string &name, double minVis)

{
  VisCat cat;
  cat.name = name;
  cat.minVis = minVis;
  _cats.push_back(cat);
}

//////////////////////////
// add a forecast
// Number of forecast probabilities must match the number of vis categories.
// returns 0 on success, -1 on failure.

int ZvisFcast::addEntry(const FcastEntry &entry)
{
  if (entry.prob.size() != _cats.size()) {
    cerr << "ERROR - ZvisFcast::addEntry" << endl;
    cerr << "  Num categories does not match num probabilities." << endl;
    cerr << "  Num categories: " << _cats.size() << endl;
    cerr << "  Num probabilities: " << entry.prob.size() << endl;
    return -1;
  }
  _entries.push_back(entry);
  return 0;
}

///////////////////////
// get vis units string

string ZvisFcast::getVisUnitsStr() const

{
  if (_visUnits == VIS_UNITS_KM) {
    return "km";
  } else if (_visUnits == VIS_UNITS_MI) {
    return "mi";
  } else {
    return "nm";
  }
}

///////////////////////
// get cal status str

string ZvisFcast::getCalStatusStr() const

{
  if (_calStatus == CALIBRATED) {
    return "calibrated";
  } else if (_calStatus == CLIMATOLOGY) {
    return "climatology";
  } else {
    return "cal_defaults";
  }
}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void ZvisFcast::assemble()
  
{

  // print object to string as XML

  string xml;

  xml += TaXml::writeStartTag("ZvisFcast", 0);
  
  xml += TaXml::writeInt("version", 1, _versionNum);

  xml += TaXml::writeString("vis_units", 1, getVisUnitsStr());
  xml += TaXml::writeTime("cal_time", 1, _calTime);
  xml += TaXml::writeString("cal_status", 1, getCalStatusStr());
  xml += TaXml::writeDouble("cal_intercept", 1, _calIntercept);
  xml += TaXml::writeDouble("cal_slope", 1, _calSlope);
  xml += TaXml::writeString("loc_name", 1, _locationName);
  xml += TaXml::writeDouble("loc_lat", 1, _locationLat);
  xml += TaXml::writeDouble("loc_lon", 1, _locationLon);
  xml += TaXml::writeTime("gen_time", 1, _genTime);
  xml += TaXml::writeInt("delta_secs", 1, _deltaSecs);
  
  for (int ii = 0; ii < (int) _cats.size(); ii++) {
    xml += TaXml::writeStartTag("cat", 1);
    xml += TaXml::writeString("name", 2, _cats[ii].name);
    xml += TaXml::writeDouble("min_vis", 2, _cats[ii].minVis);
    xml += TaXml::writeEndTag("cat", 1);
  }
  
  for (int ii = 0; ii < (int) _entries.size(); ii++) {
    const FcastEntry &entry = _entries[ii];
    xml += TaXml::writeStartTag("forecast", 1);
    xml += TaXml::writeInt("lead_secs", 2, entry.leadSecs);
    int nCat = _cats.size();
    if ((int) entry.prob.size() < nCat) {
      nCat = (int) entry.prob.size();
    }
    for (int jj = 0; jj < nCat; jj++) {
      vector<TaXml::attribute> attrs;
      TaXml::addStringAttr("cat", _cats[jj].name, attrs);
      xml += TaXml::writeDouble("prob", 2, attrs, entry.prob[jj]);
    }
    xml += TaXml::writeEndTag("forecast", 1);
  }
  
  xml += TaXml::writeEndTag("ZvisFcast", 0);

  // free up mem buffer
  
  _memBuf.free();
  
  // add xml string to buffer
  
  _memBuf.add(xml.c_str(), xml.size());

}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int ZvisFcast::disassemble(const void *buf, int len)

{

  // make copy of buffer, make sure it is null terminated

  TaArray<char> copyBuf;
  char *copy = copyBuf.alloc(len + 1);
  memcpy(copy, buf, len);
  copy[len] = '\0';

  // remove comments
  
  string xmlBuf = TaXml::removeComments(copy);
  
  // clear state

  clear();
  
  // set state from the XML

  int iret = 0;

  TaXml::readInt(xmlBuf, "version", _versionNum);

  // units

  string unitsStr;
  if (TaXml::readString(xmlBuf, "vis_units", unitsStr)) {
    cerr << "  ERROR - cannot read vis_units" << endl;
    iret = -1;
  }
  if (unitsStr == "km") {
    _visUnits = VIS_UNITS_KM;
  } else if (unitsStr == "mi") {
    _visUnits = VIS_UNITS_MI;
  } else if (unitsStr == "nm") {
    _visUnits = VIS_UNITS_NM;
  }

  // calibration
  
  if (TaXml::readTime(xmlBuf, "cal_time", _calTime)) {
    cerr << "  ERROR - cannot read cal_time" << endl;
    iret = -1;
  }
  
  string statusStr;
  if (TaXml::readString(xmlBuf, "cal_status", statusStr)) {
    cerr << "  ERROR - cannot read cal_status" << endl;
    iret = -1;
  }
  if (statusStr == "calibrated") {
    _calStatus = CALIBRATED;
  } else if (statusStr == "climatology") {
    _calStatus = CLIMATOLOGY;
  } else if (statusStr == "cal_defaults") {
    _calStatus = CAL_DEFAULTS;
  }

  if (TaXml::readDouble(xmlBuf, "cal_intercept", _calIntercept)) {
    cerr << "  ERROR - cannot read cal_intercept" << endl;
    iret = -1;
  }

  if (TaXml::readDouble(xmlBuf, "cal_slope", _calSlope)) {
    cerr << "  ERROR - cannot read cal_slope" << endl;
    iret = -1;
  }

  // location

  if (TaXml::readString(xmlBuf, "loc_name", _locationName)) {
    cerr << "  ERROR - cannot read loc_name" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "loc_lat", _locationLat)) {
    cerr << "  ERROR - cannot read loc_lat" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "loc_lon", _locationLon)) {
    cerr << "  ERROR - cannot read loc_lon" << endl;
    iret = -1;
  }

  // forecast times

  if (TaXml::readTime(xmlBuf, "gen_time", _genTime)) {
    cerr << "  ERROR - cannot read gen_time" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "delta_secs", _deltaSecs)) {
    cerr << "  ERROR - cannot read delta_secs" << endl;
    iret = -1;
  }
  
  // categories

  vector<string> catArray;
  if (TaXml::readStringArray(xmlBuf, "cat", catArray)) {
    cerr << "  ERROR - cannot read cat array" << endl;
    iret = -1;
  }
  for (int ii = 0; ii < (int) catArray.size(); ii++) {
    string name;
    if (TaXml::readString(catArray[ii], "name", name)) {
      cerr << "  ERROR - cannot read cat name" << endl;
      cerr << "  SubBuffer: " << endl;
      cerr << catArray[ii] << endl;
      iret = -1;
    }
    double minVis = 0;
    if (TaXml::readDouble(catArray[ii], "min_vis", minVis)) {
      cerr << "  ERROR - cannot read cat min_vis" << endl;
      cerr << "  SubBuffer: " << endl;
      cerr << catArray[ii] << endl;
      iret = -1;
    }
    if (iret == 0) {
      addCategory(name, minVis);
    }
  }

  // forecasts

  vector<string> fcastArray;
  if (TaXml::readStringArray(xmlBuf, "forecast", fcastArray)) {
    cerr << "  ERROR - cannot read forecast array" << endl;
    iret = -1;
  }
  for (int ii = 0; ii < (int) fcastArray.size(); ii++) {
    int leadSecs = 0;
    if (TaXml::readInt(fcastArray[ii], "lead_secs", leadSecs)) {
      cerr << "  ERROR - cannot read forecast lead_secs" << endl;
      cerr << "  SubBuffer: " << endl;
      cerr << fcastArray[ii] << endl;
      iret = -1;
    }
    FcastEntry entry;
    entry.leadSecs = leadSecs;
    vector<string> probArray;
    if (TaXml::readTagBufArray(fcastArray[ii], "prob", probArray)) {
      cerr << "  ERROR - cannot read prob array" << endl;
      cerr << "  SubBuffer: " << endl;
      cerr << fcastArray[ii] << endl;
      iret = -1;
    }
    for (int jj = 0; jj < (int) probArray.size(); jj++) {
      double prob = 0;
      if (TaXml::readDouble(probArray[jj], "prob", prob)) {
        cerr << "  ERROR - cannot find prob tag" << endl;
        cerr << "  SubBuffer: " << endl;
        cerr << probArray[jj] << endl;
        iret = -1;
      }
      entry.prob.push_back(prob);
    } // jj

    addEntry(entry);
    
  } // ii
  
  if (iret) {
    cerr << "ERROR -  ZvisFcast::disassemble" << endl;
    cerr << "XML buffer: " << endl;
    cerr << copy << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////
// printing object


void ZvisFcast::print(ostream &out, string spacer /* = ""*/ ) const

{

  out << "=============================================" << endl;
  out << spacer << "Z-V probability-based forecast" << endl;
  out << spacer << "Version number: " << _versionNum << endl;

  out << spacer << "  visUnits: " << getVisUnitsStr() << endl;

  out << spacer << "  calTime: " << DateTime::strm(_calTime) << endl;
  out << spacer << "  calStatus: " << getCalStatusStr() << endl;
  out << spacer << "  calIntercept: " << _calIntercept << endl;
  out << spacer << "  calSlope: " << _calSlope << endl;
  
  out << spacer << "  locationName: " << _locationName << endl;
  out << spacer << "  locationLat: " << _locationLat << endl;
  out << spacer << "  locationLon: " << _locationLon << endl;

  out << spacer << "  Categories:" << endl;
  for (int ii = 0; ii < (int) _cats.size(); ii++) {
    out << spacer << "    name, minVis: "
        << setw(8) << _cats[ii].name << ", "
        << setw(8) << _cats[ii].minVis << endl;
  }
  
  out << spacer << "  genTime: " << DateTime::strm(_genTime) << endl;
  out << spacer << "  deltaSecs: " << _deltaSecs << endl;

  out << spacer << "  Forecasts:" << endl;
  for (int ii = 0; ii < (int) _entries.size(); ii++) {
    const FcastEntry &entry = _entries[ii];
    out << spacer << "    leadSecs, probs: "
        << setw(6) << entry.leadSecs;
    for (int jj = 0; jj < (int) entry.prob.size(); jj++) {
      out << " " << setw(6) << ((int) (entry.prob[jj] * 100.0 + 0.5)) / 100.0;
    } // jj
    out << endl;
  } // ii

  out << endl;
  
}

