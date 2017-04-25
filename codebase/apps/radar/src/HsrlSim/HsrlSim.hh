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
////////////////////////////////////////////////////////////////////////////////
//
// HsrlSim.hh
// HsrlSim class
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// April 2017
//
////////////////////////////////////////////////////////////////////////////////
//
// HsrlSim listens for clients. When a client connects, it spawns a
// child to handle the client. The child opens an HSRL raw NetCDF
// file, and loops reading the file, creating raw rays and sending
// them to the client.";
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef  HSRL_SIM_HH
#define  HSRL_SIM_HH

#include <Fmq/Fmq.hh>
#include <string>
#include <dsserver/ProcessServer.hh>
#include <radar/HsrlRawRay.hh>
#include "Params.hh"
using namespace std;

class HsrlSim : public ProcessServer
{
public:
  
  HsrlSim(const string& progName,
          const Params &params);
  
  virtual ~HsrlSim();
  
protected:
  
  string _progName;
  const Params &_params;
  
  // provide method missing in base class to handle client
  // Returns 0 on success, -1 on failure
  
  virtual int handleClient(Socket* socket);
    
private:


};

#endif

