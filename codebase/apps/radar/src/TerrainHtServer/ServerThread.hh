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
// ServerThread.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
// ServerThread instantiates the server, listens for clients,
// and services the client requests.
// 
////////////////////////////////////////////////////////////////

#ifndef ServerThread_HH
#define ServerThread_HH

#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <toolsa/TaThread.hh>
#include "Params.hh"
class TerrainHtServer;

using namespace std;

////////////////////////
// This class

class ServerThread : public TaThread {
  
public:

  // constructor
  
  ServerThread(TerrainHtServer *parent, const Params &params);
  
  // destructor
  
  ~ServerThread();

  // run method - where the work is done

  void run();

protected:
  
private:
  
  TerrainHtServer *_parent;
  Params _params;
  xmlrpc_c::serverAbyss *_abyss;

  // inner class for handling ht requests
  
  class getHeightMethod : public xmlrpc_c::method {
  public:
    getHeightMethod(TerrainHtServer *parent,
                    const Params &params);
    void execute(xmlrpc_c::paramList const& paramList,
                 xmlrpc_c::value * const retvalP);
  protected:
  private:
    TerrainHtServer *_parent;
    Params _params;
  };

};

#endif
