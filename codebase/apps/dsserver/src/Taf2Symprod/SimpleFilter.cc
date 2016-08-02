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
// SimpleFilter.cc
//
// Abstract base class for filter objects.  Employed by the server
// to restrict which metars are returned and assign colors.
// The filters are setup via the auxillary XML.
//
// Paul Prestopnik, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2009
//
///////////////////////////////////////////////////////////////
#include <toolsa/TaXml.hh>
#include "SimpleFilter.hh"

SimpleFilter::SimpleFilter(const Params* p) : Filter(p)
{

}

SimpleFilter::~SimpleFilter()
{

}

  // return 0 on success, -1 on failure
int SimpleFilter::setRulesFromXml(string XML)
{
  _clearRules();

  // print the XML
  
  if (_isVerbose) {
    cerr << "====== Auxiliary XML ======" << endl;
    cerr << XML;
    cerr << "====== Auxiliary XML ======" << endl;
  }

  // remove comments

  string noCommentBuf = TaXml::removeComments(XML);

  // is there a filter section?

  string filterBuf;
  if (TaXml::readString(noCommentBuf, "ogc:Filter", filterBuf)) {
    if (_isVerbose) {
      cerr << "WARNING - Metar2Symprod" << endl;
      cerr << "  Trying to decode XML rules for filtering" << endl;
      cerr << "  Cannot find <ogc:Filter> tag" << endl;
      return -1;
    }
  }

  // find any greaterThan rules

  vector<string> gtArray;
  TaXml::readStringArray(filterBuf, "ogc:PropertyIsGreaterThan", gtArray);

  // find any lessThan rules

  vector<string> ltArray;
  TaXml::readStringArray(filterBuf, "ogc:PropertyIsLessThan", ltArray);

  // we should have at least 1 rule

  if (gtArray.size() == 0 && ltArray.size() == 0) {
    if (_isDebug) {
      cerr << "ERROR - Metar2Symprod" << endl;
      cerr << "  XML rules section included, but no rules found" << endl;
    }
    return -1;
  }

  // set the greater than rules

  int iret = 0;
  for (int ii = 0; ii < (int) gtArray.size(); ii++) {
    if (_activateGreaterThanRule(gtArray[ii])) {
      iret = -1;
    }
  }

  // set the less than rules

  for (int ii = 0; ii < (int) ltArray.size(); ii++) {
    if (_activateLessThanRule(ltArray[ii])) {
      iret = -1;
    }
  }

  if (_isVerbose) {
    printRules();
  }

  return iret;

}

void SimpleFilter::printRules()
{
  cerr << "Metar2Symprod - rules set from XML" << endl;
  _printFieldRule(_checkVisLimits, "vis", _minVisKm, _maxVisKm, cerr);
  _printFieldRule(_checkCeilLimits, "ceil", _minCeilKm, _maxCeilKm, cerr);
  _printFieldRule(_checkTempLimits, "temp", _minTempC, _maxTempC, cerr);
  _printFieldRule(_checkWspdLimits, "wspd", _minWspdMps, _maxWspdMps, cerr);
}

//returns true if obs passes the filter, false otherwise
bool SimpleFilter::testForecast(const Taf::ForecastPeriod &period)
{
  if (_checkFieldValue(_checkVisLimits, "vis",
                       period.visKm, _minVisKm, _maxVisKm)) {
    return false;
  }
  if (_checkFieldValue(_checkCeilLimits, "ceil",
                       period.ceilingKm, _minCeilKm, _maxCeilKm)) {
    return false;
  }
  if (period.maxTempTime != 0) {
    if (_checkFieldValue(_checkTempLimits, "maxTemp",
                         period.maxTempC, _minTempC, _maxTempC)) {
      return false;
    }
  }
  if (period.minTempTime != 0) {
    if (_checkFieldValue(_checkTempLimits, "maxTemp",
                         period.minTempC, _minTempC, _maxTempC)) {
      return false;
    }
  }
  double windSpeedMps = period.windSpeedKmh / 3.6;
  if (_checkFieldValue(_checkWspdLimits, "wspd",
                       windSpeedMps, _minWspdMps, _maxWspdMps)) {
    return false;
  }

  return true;
}

void SimpleFilter::_clearRules()
{

  _checkVisLimits = false;
  _minVisKm = -1.0e99;
  _maxVisKm = 1.0e99;

  _checkCeilLimits = false;
  _minCeilKm = -1.0e99;
  _maxCeilKm = 1.0e99;

  _checkTempLimits = false;
  _minTempC = -1.0e99;
  _maxTempC = 1.0e99;

  _checkWspdLimits = false;
  _minWspdMps = -1.0e99;
  _maxWspdMps = 1.0e99;

}


////////////////////////////////
// activate a greather-than rule
//
// Returns 0 on success, -1 on failure

int SimpleFilter::_activateGreaterThanRule(const string &xml)

{

  if (_isVerbose) {
    cerr << "Activating GreaterThan rule" << endl;
    cerr << "XML:" << endl;
    cerr << xml << endl;
  }

  // get the property name

  string propName;
  if (TaXml::readString(xml, "ogc:PropertyName", propName)) {
    cerr << "ERROR - SimpleFilter::_activateGreaterThanRule" << endl;
    cerr << "  Cannot find property name" << endl;
    cerr << "  XML: " << endl;
    cerr << xml << endl;
    return -1;
  }

  // get the property value

  double value;
  if (TaXml::readDouble(xml, "ogc:Literal", value)) {
    cerr << "ERROR - SimpleFilter::_activateGreaterThanRule" << endl;
    cerr << "  Cannot find value for property: " << propName << endl;
    cerr << "  XML: " << endl;
    cerr << xml << endl;
    return -1;
  }

  if (propName == "visibilityKm") {
    _checkVisLimits = true;
    _minVisKm = value;
  } else if (propName == "ceilingKm") {
    _checkCeilLimits = true;
    _minCeilKm = value;
  } else if (propName == "temperatureC") {
    _checkTempLimits = true;
    _minTempC = value;
  } else if (propName == "windSpeedMps") {
    _checkWspdLimits = true;
    _minWspdMps = value;
  }

  return 0;

}

////////////////////////////////
// activate a less-than rule
//
// Returns 0 on success, -1 on failure

int SimpleFilter::_activateLessThanRule(const string &xml)

{

  if (_isVerbose) {
    cerr << "Activating LessThan rule" << endl;
    cerr << "XML:" << endl;
    cerr << xml << endl;
  }

  // get the property name

  string propName;
  if (TaXml::readString(xml, "ogc:PropertyName", propName)) {
    cerr << "ERROR - SimpleFilter::_activateLessThanRule" << endl;
    cerr << "  Cannot find property name" << endl;
    cerr << "  XML: " << endl;
    cerr << xml << endl;
    return -1;
  }

  // get the property value

  double value;
  if (TaXml::readDouble(xml, "ogc:Literal", value)) {
    cerr << "ERROR - SimpleFilter::_activateLessThanRule" << endl;
    cerr << "  Cannot find value for property: " << propName << endl;
    cerr << "  XML: " << endl;
    cerr << xml << endl;
    return -1;
  }

  if (propName == "visibilityKm") {
    _checkVisLimits = true;
    _maxVisKm = value;
  } else if (propName == "ceilingKm") {
    _checkCeilLimits = true;
    _maxCeilKm = value;
  } else if (propName == "temperatureC") {
    _checkTempLimits = true;
    _maxTempC = value;
  } else if (propName == "windSpeedMps") {
    _checkWspdLimits = true;
    _maxWspdMps = value;
  }

  return 0;

}

//////////////////////////////////////////////////
// print field limits

void SimpleFilter::_printFieldRule(bool limitFlag,
                             const string &name,
                             double minVal,
                             double maxVal,
                             ostream &out)

{

  if (limitFlag) {
    out << "Limiting field " << name
        << ", min: " << minVal
        << ", max: " << maxVal << endl;
  }

}

////////////////////////////
// check for field limits
//
// Returns 0 on success (all fields within limits)
//        -1 on failure (some fields outside limits)


int SimpleFilter::_checkFieldValue(bool limitFlag,
                             const string &name,
                             double val,
                             double minVal,
                             double maxVal)

  
{

  if (!limitFlag) {
    return 0;
  }

  if (val < -9900) {
    if (_isVerbose) {
      cerr << "Field " << name << " missing, rejecting!" << endl;
    }
    return -1;
  }
  
  if (val < minVal) {
    if (_isVerbose) {
      cerr << "Field " << name
          << ", val: " << val
          << ", below min val: " << minVal
          << ", rejecting!" << endl;
    }
    return -1;
  }
  
  if (val > maxVal) {
    if (_isVerbose) {
      cerr << "Field " << name
          << ", val: " << val
          << ", exceeds max val: " << maxVal
          << ", rejecting!" << endl;
    }
    return -1;
  }
  
  return 0;

}

