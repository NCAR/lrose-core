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
// Udunits2.hh
//
// Udunits wrapper
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2013
//
///////////////////////////////////////////////////////////////

#ifndef Udunits2_HH
#define Udunits2_HH

#include <string>
#include <iostream>
#include <cassert>
#include "udunits2.h"

using namespace std;

///////////////////////////////////////////////////////////////
/// CLASS FOR NETCDF IO OPERATIONS

class Udunits2

{
  
public:
  
  // constructor 
  // will call assert if cannot be initialized
  
  Udunits2();
  
  /// Destructor
  
  virtual ~Udunits2();

  /// Clear all status
  
  void clear();

  //////////////////////////////////////////////////////////////
  /// \name get system and epoch
  //@{

  ut_system *getSystem() { return _uds; } 
  ut_unit *getEpoch() { return _udsEpoch; }
  
  //@}
  
  ////////////////////////
  /// \name Error string:
  //@{
  
  /// Get the Error String.
  /// The contents are only meaningful if an error has returned.
  
  string getErrStr() const { return _errStr; }
  
  //@}

protected:

private:

  // error string

  string _errStr; ///< Error string is set on read or write error
  
  // handles
  
  ut_system *_uds;
  ut_unit *_udsEpoch;

  // initialize udunits

  int _init();
  void _free();
  
  /// Clear error string.
  
  void _clearErrStr() { _errStr.clear(); }

  /// add integer value to error string, with label
  
  void _addErrInt(string label, int iarg,
                  bool cr = true);
  
  /// add double value to error string, with label
  
  void _addErrDbl(string label, double darg,
                  string format, bool cr = true);
  
  /// add string value to error string, with label
  
  void _addErrStr(string label, string strarg = "",
                  bool cr = true);
  
};

#endif
