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
// Filter.hh
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

#ifndef _Filter_HH
#define _Filter_HH

using namespace std;
#include <rapformats/WxObs.hh>  
#include "Params.hh"

class Filter
{
  
public:

  // constructor

  Filter(Params *p);

  // destructor
  
  virtual ~Filter();

  
  // returns the correct Filter subclass for this XML
   // It is the responsibility of the caller to correctly dispose of
  // 	the returned object
  static Filter* getCorrectFilter(string XML, Params* params);



  ////////////////////////////////////////////
  //  INTERFACE IMPLEMENTED BY SUB CLASSES
  
  // return 0 on success, -1 on failure
  virtual int setRulesFromXml(string XML) = 0;

  virtual void printRules() {};  

  //return true if obs passes the filter, false otherwise
  virtual bool testObs(WxObs obs) = 0;

  //return a null pointer if the filter does not define colors
  // this must be called after testObs(), and returns the color
  // for the latest obs.
  const char *getColor();

protected:
  virtual void _clearRules() = 0;

  
  bool _isVerbose;
  bool _isDebug;
  string _latestObsColorString;
};

#endif
