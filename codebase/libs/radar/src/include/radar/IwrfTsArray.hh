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
// IwrfTsArray
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2021
//
///////////////////////////////////////////////////////////////

#ifndef IwrfTsArray_hh
#define IwrfTsArray_hh

#include <string>
#include <toolsa/pmu.h>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
using namespace std;

////////////////////////
// Base class

class IwrfTsArray {
  
public:

  // constructor
  
  IwrfTsArray(IwrfDebug_t debug = IWRF_DEBUG_OFF);
  
  // destructor
  
  virtual ~IwrfTsArray();

  // debugging

  void setDebug(IwrfDebug_t debug) { _debug = debug; }
  
  // reset the queue to the beginning

  virtual void resetToStart();

  // reset the queue at the end
  
  virtual void resetToEnd();
  
  // get the current file path in use

  virtual const string getPathInUse() const { return _pathInUse; }
  
  // get the previous file path in use
  
  virtual const string getPrevPathInUse() const { return _prevPathInUse; }
  
protected:
  
  IwrfDebug_t _debug;
  string _pathInUse;
  string _prevPathInUse;
  
private:
  
};

#endif

