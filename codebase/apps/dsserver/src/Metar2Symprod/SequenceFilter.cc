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
// SequenceFilter.cc
//
// A filter that applies min/max to several fields and associates
// passing a particular criteria with a color.  Employed
// by the server to restrict which metars are returned and assign
// colors.  The filters are setup via the auxillary XML.
//
// Paul Prestopnik, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2009
//
///////////////////////////////////////////////////////////////
#include <toolsa/TaXml.hh>
#include "SequenceFilter.hh"

SequenceFilter::SequenceFilter(Params* p) : Filter(p)
{

}


SequenceFilter::~SequenceFilter()
{
  _clearRules();  //free the memory from filterStack
}

  // return 0 on success, -1 on failure
int SequenceFilter::setRulesFromXml(string XML)
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
  if (TaXml::readString(noCommentBuf, "ogc:Sequence", filterBuf)) {
    if (_isVerbose) {
      cerr << "WARNING - Metar2Symprod" << endl;
      cerr << "SequenceFilter::setRulesFromXml" << endl;
      cerr << "  Trying to decode XML rules for filtering" << endl;
      cerr << "  Cannot find <ogc:Sequence> tag" << endl;
      return -1;
    }
  }

  //iterate over the various filter sections:
  vector<string> filterArray;
  if(TaXml::readTagBufArray(filterBuf, "ogc:Filter", filterArray)) {
    if (_isVerbose) {
      cerr << "WARNING - Metar2Symprod" << endl;
      cerr << "SequenceFilter::setRulesFromXml" << endl;
      cerr << "  Trying to decode XML rules for filtering" << endl;
      cerr << "  Cannot find <ogc:Filter> tags" << endl;
      return -1;
    }
  }

  vector<string>::iterator filterIter;
  MetarFilter *mf;
  vector<TaXml::attribute> tmpAttrs;
  string color;
  int iret = 0;
  
  for (filterIter = filterArray.begin();
       filterIter != filterArray.end(); filterIter++ )
    {
      mf = new MetarFilter();

      //get the color attribute
      tmpAttrs.clear();

      //temporarily reusing color string, but we can throw it away
      TaXml::readString(*filterIter, "ogc:Filter", color, tmpAttrs);

      TaXml::readStringAttr(tmpAttrs, "color", color);
      mf->color = color;

      // find any greaterThan rules
      vector<string> gtArray;
      TaXml::readStringArray(*filterIter, "ogc:PropertyIsGreaterThan", gtArray);

      // find any lessThan rules
      vector<string> ltArray;
      TaXml::readStringArray(*filterIter, "ogc:PropertyIsLessThan", ltArray);

      // set the greater than rules
     for (int ii = 0; ii < (int) gtArray.size(); ii++) {
	if (_activateGreaterThanRule(gtArray[ii], mf)) {
	  iret = -1;
	}
      }

      // set the less than rules
      for (int ii = 0; ii < (int) ltArray.size(); ii++) {
	if (_activateLessThanRule(ltArray[ii], mf)) {
	  iret = -1;
	}
      }
      filterStack.push_back(mf);
    }


  if (_isVerbose) {
    printRules();
  }

  return iret;

}

void SequenceFilter::printRules()
{
  cerr << "Metar2Symprod - rules set from XML" << endl;
  filterStackIter fsi;
  for(fsi = filterStack.begin(); fsi != filterStack.end(); fsi++)
    {
      cerr << "==Begin Individual Filter==\n";
      cerr << "COLOR: " << (*fsi)->color.c_str() << endl;
      _printFieldRule((*fsi)->checkVisLimits, "vis", 
		      (*fsi)->minVisKm, (*fsi)->maxVisKm);
      _printFieldRule((*fsi)->checkCeilLimits, "ceil", 
		      (*fsi)->minCeilKm, (*fsi)->maxCeilKm);
      _printFieldRule((*fsi)->checkTempLimits, "temp", 
		      (*fsi)->minTempC, (*fsi)->maxTempC);
      _printFieldRule((*fsi)->checkRhLimits, "rh", 
		      (*fsi)->minRhPercent, (*fsi)->maxRhPercent);
      _printFieldRule((*fsi)->checkRvrLimits, "rvr", 
		      (*fsi)->minRvrKm, (*fsi)->maxRvrKm);
      _printFieldRule((*fsi)->checkWspdLimits, "wspd", 
		      (*fsi)->minWspdMps, (*fsi)->maxWspdMps);
      cerr << "==========================\n";
    }
}

void SequenceFilter::printRule(filterStackIter fsi)
{
      cerr << "==Begin Individual Filter==\n";
      cerr << "COLOR: " << (*fsi)->color.c_str() << endl;
      _printFieldRule((*fsi)->checkVisLimits, "vis", 
		      (*fsi)->minVisKm, (*fsi)->maxVisKm);
      _printFieldRule((*fsi)->checkCeilLimits, "ceil", 
		      (*fsi)->minCeilKm, (*fsi)->maxCeilKm);
      _printFieldRule((*fsi)->checkTempLimits, "temp", 
		      (*fsi)->minTempC, (*fsi)->maxTempC);
      _printFieldRule((*fsi)->checkRhLimits, "rh", 
		      (*fsi)->minRhPercent, (*fsi)->maxRhPercent);
      _printFieldRule((*fsi)->checkRvrLimits, "rvr", 
		      (*fsi)->minRvrKm, (*fsi)->maxRvrKm);
      _printFieldRule((*fsi)->checkWspdLimits, "wspd", 
		      (*fsi)->minWspdMps, (*fsi)->maxWspdMps);
      cerr << "==========================\n";
}

//returns true if obs passes the filter, false otherwise
bool SequenceFilter::testObs(WxObs obs)
{
 filterStackIter fsi;
 int i;
  for(i=0, fsi = filterStack.begin(); fsi != filterStack.end(); fsi++, i++)
    {
      if (0 != _checkFieldValue((*fsi)->checkVisLimits, "vis",
			   obs.getVisibilityKm(), (*fsi)->minVisKm, (*fsi)->maxVisKm)) {
	continue;
      }
      if (0 != _checkFieldValue((*fsi)->checkCeilLimits, "ceil",
			   obs.getCeilingKm(), (*fsi)->minCeilKm, (*fsi)->maxCeilKm)) {
	continue;
      }
      if (0 != _checkFieldValue((*fsi)->checkTempLimits, "temp",
			   obs.getTempC(), (*fsi)->minTempC, (*fsi)->maxTempC)) {
	continue;
      }
      if (0 != _checkFieldValue((*fsi)->checkRhLimits, "rh",
			   obs.getRhPercent(), (*fsi)->minRhPercent, (*fsi)->maxRhPercent)) {
	continue;
      }
      if (0 != _checkFieldValue((*fsi)->checkRvrLimits, "rvr",
			   obs.getRvrKm(), (*fsi)->minRvrKm, (*fsi)->maxRvrKm)) {
	continue;
      }
      if (0 != _checkFieldValue((*fsi)->checkWspdLimits, "wspd",
			   obs.getWindSpeedMps(), (*fsi)->minWspdMps, (*fsi)->maxWspdMps)) {
	continue;
      }
      //cerr << "PASS!\n";
      _latestObsColorString = (*fsi)->color;
      return true;
    }
  return false;
}

void SequenceFilter::_clearRules()
{
  filterStackIter fsi;
  for(fsi = filterStack.begin(); fsi != filterStack.end(); fsi++)
    {
      delete *fsi;
    }

}


////////////////////////////////
// activate a greather-than rule
//
// Returns 0 on success, -1 on failure

int SequenceFilter::_activateGreaterThanRule(const string &xml, MetarFilter *mf)
{

  if (_isVerbose) {
    cerr << "Activating GreaterThan rule" << endl;
    cerr << "XML:" << endl;
    cerr << xml << endl;
  }

  // get the property name

  string propName;
  if (TaXml::readString(xml, "ogc:PropertyName", propName)) {
    cerr << "ERROR - SequenceFilter::_activateGreaterThanRule" << endl;
    cerr << "  Cannot find property name" << endl;
    cerr << "  XML: " << endl;
    cerr << xml << endl;
    return -1;
  }

  // get the property value

  double value;
  if (TaXml::readDouble(xml, "ogc:Literal", value)) {
    cerr << "ERROR - SequenceFilter::_activateGreaterThanRule" << endl;
    cerr << "  Cannot find value for property: " << propName << endl;
    cerr << "  XML: " << endl;
    cerr << xml << endl;
    return -1;
  }

  if (propName == "visibilityKm") {
    mf->checkVisLimits = true;
    mf->minVisKm = value;
  } else if (propName == "ceilingKm") {
    mf->checkCeilLimits = true;
    mf->minCeilKm = value;
  } else if (propName == "temperatureC") {
    mf->checkTempLimits = true;
    mf->minTempC = value;
  } else if (propName == "rhPercent") {
    mf->checkRhLimits = true;
    mf->minRhPercent = value;
  } else if (propName == "rvrKm") {
    mf->checkRvrLimits = true;
    mf->minRvrKm = value;
  } else if (propName == "windSpeedMps") {
    mf->checkWspdLimits = true;
    mf->minWspdMps = value;
  }

  return 0;

}

////////////////////////////////
// activate a less-than rule
//
// Returns 0 on success, -1 on failure

int SequenceFilter::_activateLessThanRule(const string &xml, MetarFilter *mf)

{

  if (_isVerbose) {
    cerr << "Activating LessThan rule" << endl;
    cerr << "XML:" << endl;
    cerr << xml << endl;
  }

  // get the property name

  string propName;
  if (TaXml::readString(xml, "ogc:PropertyName", propName)) {
    cerr << "ERROR - SequenceFilter::_activateLessThanRule" << endl;
    cerr << "  Cannot find property name" << endl;
    cerr << "  XML: " << endl;
    cerr << xml << endl;
    return -1;
  }

  // get the property value

  double value;
  if (TaXml::readDouble(xml, "ogc:Literal", value)) {
    cerr << "ERROR - SequenceFilter::_activateLessThanRule" << endl;
    cerr << "  Cannot find value for property: " << propName << endl;
    cerr << "  XML: " << endl;
    cerr << xml << endl;
    return -1;
  }

  if (propName == "visibilityKm") {
    mf->checkVisLimits = true;
    mf->maxVisKm = value;
  } else if (propName == "ceilingKm") {
    mf->checkCeilLimits = true;
    mf->maxCeilKm = value;
  } else if (propName == "temperatureC") {
    mf->checkTempLimits = true;
    mf->maxTempC = value;
  } else if (propName == "rhPercent") {
    mf->checkRhLimits = true;
    mf->maxRhPercent = value;
  } else if (propName == "rvrKm") {
    mf->checkRvrLimits = true;
    mf->maxRvrKm = value;
  } else if (propName == "windSpeedMps") {
    mf->checkWspdLimits = true;
    mf->maxWspdMps = value;
  }

  return 0;

}

//////////////////////////////////////////////////
// print field limits

void SequenceFilter::_printFieldRule(bool limitFlag,
                             const string &name,
                             double minVal,
                             double maxVal)

{

  if (limitFlag) {
    cerr << "Limiting field " << name
        << ", min: " << minVal
        << ", max: " << maxVal << endl;
  }

}

////////////////////////////
// check for field limits
//
// Returns 0 on success (all fields within limits)
//        -1 on failure (some fields outside limits)


int SequenceFilter::_checkFieldValue(bool limitFlag,
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

 SequenceFilter::MetarFilter::MetarFilter()
{
  checkVisLimits = false;
  minVisKm = -1.0e99;
  maxVisKm = 1.0e99;

  checkCeilLimits = false;
  minCeilKm = -1.0e99;
  maxCeilKm = 1.0e99;

  checkTempLimits = false;
  minTempC = -1.0e99;
  maxTempC = 1.0e99;

  checkRhLimits = false;
  minRhPercent = -1.0e99;
  maxRhPercent = 1.0e99;

  checkRvrLimits = false;
  minRvrKm = -1.0e99;
  maxRvrKm = 1.0e99;

  checkWspdLimits = false;
  minWspdMps = -1.0e99;
  maxWspdMps = 1.0e99;
}


