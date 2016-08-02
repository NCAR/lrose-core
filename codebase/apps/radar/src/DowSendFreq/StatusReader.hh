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
// StatusReader
//
// Read the master FMQ
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////

#ifndef StatusReader_hh
#define StatusReader_hh

#include <string>
#include <toolsa/DateTime.hh>
#include <Fmq/DsFmq.hh>
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include <radar/iwrf_functions.hh>
#include "Params.hh"
using namespace std;

////////////////////////
// Base class

class StatusReader {
  
public:
  
  // constructor
  
  StatusReader(const Params &params,
               const string &fmqPath,
               bool isActive,
               double defaultFreqMhz);
  
  // destructor
  
  ~StatusReader();

  // read in status xml
  // returns 0 on success, -1 on failure
  
  int readStatus();
  
  // get latest info
  
  const string &getLatestStatusXml() const { return _latestStatusXml; }
  const DateTime &getLatestStatusTime() const { return _latestStatusTime; }
  double getLatestFreqMhz() const { return _latestFreqMhz; }

protected:
private:
  
  const Params &_params;
  string _fmqPath;
  bool _isActive;

  // input queue

  DsFmq _fmq;
  bool _fmqIsOpen;

  // input message

  DsMessage _msg;
  DsMsgPart *_part;
  int _nParts;
  int _pos;

  // save latest status info
  
  string _latestStatusXml;
  DateTime _latestStatusTime;
  double _latestFreqMhz;

  // private methods

  int _getNextPart(bool &gotOne);

};

#endif

