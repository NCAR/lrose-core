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
// SequenceFilter.hh
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

#ifndef _SequenceFilter_HH
#define _SequenceFilter_HH

#include <string>

#include "Filter.hh"


using namespace std;

class SequenceFilter : public Filter
{
  
public:

  class MetarFilter 
  {
  public:
    MetarFilter();
    string color;
    bool checkVisLimits;
    double minVisKm, maxVisKm;

    bool checkCeilLimits;
    double minCeilKm, maxCeilKm;

    bool checkTempLimits;
    double minTempC, maxTempC;

    bool checkRhLimits;
    double minRhPercent, maxRhPercent;

    bool checkRvrLimits;
    double minRvrKm, maxRvrKm;

    bool checkWspdLimits;
    double minWspdMps, maxWspdMps;
  };

  // constructor

  SequenceFilter (const Params* params);

  // destructor
  
  virtual ~SequenceFilter();

  //types
  typedef vector<MetarFilter*>::iterator filterStackIter;



  virtual int setRulesFromXml(string XML);
  virtual void printRules();  
  void printRule(filterStackIter fsi);
  
  //returns true if obs passes the filter, false otherwise
  virtual bool testForecast(const Taf::ForecastPeriod &period);
  virtual void _clearRules();

 protected:
  
  vector<MetarFilter*> filterStack;

  //helper functions to parse the XML
  int _activateGreaterThanRule(const string &xml, MetarFilter *mf);
  int _activateLessThanRule(const string &xml, MetarFilter *mf);

  int _checkFieldValue(bool limitFlag,
                       const string &name,
                       double val,
                       double minVal,
                       double maxVal);

  void _printFieldRule(bool limitFlag,
		       const string &name,
		       double minVal,
		       double maxVal);





};

#endif
