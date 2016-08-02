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
////////////////////////////////////////////////////////////////////
// <dsserver/DsTitan.hh>
//
// Server-based TITAN C++ track file io
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// March 2001
//
////////////////////////////////////////////////////////////////////
//
// see TitanServer class in <titan/TitanServer.hh> for
// most of the methods
//
////////////////////////////////////////////////////////////////////

#ifndef DsTitan_HH
#define DsTitan_HH


#include <titan/TitanServer.hh>
#include <didss/DsURL.hh>
class DsTitanMsg;
using namespace std;

class DsTitan : public TitanServer
{

public:

  friend class DsTitanMsg;

  // constructor
  
  DsTitan();
  
  // destructor
  
  virtual ~DsTitan();

  // set the debugging state

  void setDebug(bool debug = true) { _debug = debug; }

  // option to set compression for the read
  // if true, the server will compress data before sending back to the client

  void setReadCompressed(bool compressed = true) { _readCompressed = compressed; }

  // member access

  const string &getUrlInUse() const { return _urlInUse; }
  bool getReadCompressed() const { return _readCompressed; }
  
  ///////////////////////////////
  // Override print read request
  
  void printReadRequest(ostream &out);

  /////////////////////////////////////////////////////
  // Override read
  //
  // Returns 0 on success, -1 on failure.
  //
  // On failure, use getErrStr() to get error string.

  virtual int read(const string &urlStr);

protected:

  bool _debug;
  string _urlInUse;
  bool _readCompressed;

  // functions
  
  int _communicate(DsURL &url,
		   DsTitanMsg &msg,
		   const void *msgBuf,
		   int msgLen);
  
private:
  
  // Private methods with no bodies. Copy and assignment not implemented.

  DsTitan(const DsTitan & orig);
  DsTitan & operator = (const DsTitan & other);
  
};

#endif


