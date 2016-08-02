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
// SimpleFilter.hh
//
// A simple filter that applies min/max to several fields.  Employed
// by the server to restrict which metars are returned and assign
// colors.  The filters are setup via the auxillary XML.
//
// Paul Prestopnik, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2009
//
///////////////////////////////////////////////////////////////

#ifndef _SimpleFilter_HH
#define _SimpleFilter_HH

#include "Filter.hh"

using namespace std;

class SimpleFilter : public Filter
{
  
public:

  // constructor

  SimpleFilter (Params* params);

  // destructor
  
  virtual ~SimpleFilter();


  virtual int setRulesFromXml(string XML);
  virtual void printRules();  

  //returns true if obs passes the filter, false otherwise
  virtual bool testObs(WxObs obs);
  virtual void _clearRules();

 protected:
  
   // checking field limits, with instructions sent in via XML

  bool _checkVisLimits;
  double _minVisKm, _maxVisKm;

  bool _checkCeilLimits;
  double _minCeilKm, _maxCeilKm;

  bool _checkTempLimits;
  double _minTempC, _maxTempC;

  bool _checkRhLimits;
  double _minRhPercent, _maxRhPercent;

  bool _checkRvrLimits;
  double _minRvrKm, _maxRvrKm;

  bool _checkWspdLimits;
  double _minWspdMps, _maxWspdMps;


  //helper functions to parse the XML
  int _activateGreaterThanRule(const string &xml);
  int _activateLessThanRule(const string &xml);

  int _checkFieldValue(bool limitFlag,
                       const string &name,
                       double val,
                       double minVal,
                       double maxVal);

  void _printFieldRule(bool limitFlag,
		       const string &name,
		       double minVal,
		       double maxVal,
		       ostream &out);

};

#endif
