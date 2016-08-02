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
// PowerInfo2Spdb.hh
//
// PowerInfo2Spdb object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2006
//
///////////////////////////////////////////////////////////////
//
// PowerInfo2Spdb reads sweep data from a catalog file and
// writes the info to an SPDB data base.
//
///////////////////////////////////////////////////////////////////////

#ifndef PowerInfo2Spdb_H
#define PowerInfo2Spdb_H

#include <string>
#include <rapformats/DsRadarPower.hh>
#include "Args.hh"
#include "Params.hh"
class DsSpdb;
using namespace std;

////////////////////////
// This class

class PowerInfo2Spdb {
  
public:

  // constructor

  PowerInfo2Spdb (int argc, char **argv);
  
  // destructor
  
  ~PowerInfo2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  time_t _timeLastWrite;

  // shared memory

  power_info_packet_t *_powerShmem;

  // functions

  int _processInfo(const power_info_packet_t &info);
  int _doPut(DsSpdb &spdb);
  void _printInfo(const power_info_packet_t &info, ostream &out);

};

#endif

