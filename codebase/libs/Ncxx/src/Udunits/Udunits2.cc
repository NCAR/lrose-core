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
// Udunits2.cc
//
// NetCDF file wrapper
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2016
//
///////////////////////////////////////////////////////////////

#include <Ncxx/Udunits2.hh>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
using namespace std;

//////////////
// Constructor

Udunits2::Udunits2()
  
{

  _uds = NULL;
  _udsEpoch = NULL;
  if (_init()) {
    cerr << "ERROR - Udunits2 constructor" << endl;
    cerr << _errStr << endl;
    assert(false);
  }
  
}

/////////////
// destructor

Udunits2::~Udunits2()

{
  _free();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void Udunits2::clear()
  
{
  _clearErrStr();
}

//////////////////////////////////////////////////////////////////
// initialize udunits
// returns 0 on success, -1 on failure

int Udunits2::_init()

{
  
  ut_set_error_message_handler(ut_ignore);
  
  if (_uds == NULL) {
    _uds = ut_read_xml(NULL);
    if (_uds == NULL) {
      _addErrStr("ERROR - Udunits2::_udunitsInit");
      _addErrStr("  Cannot initialize udunits");
      _addErrStr(_errStr,
                   "  Please check environment variable UDUNITS2_XML_PATH");
      return -1;
    }
  }
  
  // set up epoch time unit
  
  string epochStr("seconds since 1970-01-01T00:00:00Z");
  if (_udsEpoch == NULL) {
    _udsEpoch = ut_parse(_uds, epochStr.c_str(), UT_ASCII);
    if (_udsEpoch == NULL) {
      _addErrStr("ERROR - Udunits2::_udunitsInit");
      _addErrStr("  Cannot initialize epoch time");
      _addErrStr("  epoch: ", epochStr);
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////////////////////////
// free up

void Udunits2::_free()
  
{

  if (_udsEpoch != NULL) {
    ut_free(_udsEpoch);
    _udsEpoch = NULL;
  }

  if (_uds != NULL) {
    ut_free_system(_uds);
    _uds = NULL;
  }

}

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void Udunits2::_addErrInt(string label, int iarg, bool cr)
{
  _errStr += label;
  char str[32];
  sprintf(str, "%d", iarg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void Udunits2::_addErrDbl(string label, double darg,
                                   string format, bool cr)

{
  _errStr += label;
  char str[128];
  sprintf(str, format.c_str(), darg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void Udunits2::_addErrStr(string label, string strarg, bool cr)

{
  _errStr += label;
  _errStr += strarg;
  if (cr) {
    _errStr += "\n";
  }
}

