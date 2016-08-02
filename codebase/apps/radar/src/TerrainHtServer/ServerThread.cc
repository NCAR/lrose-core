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
///////////////////////////////////////////////////////////////
// IwrfServerThread.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////
//
//
// ServerThread instantiates the server, listens for clients,
// and services the client requests.
// 
////////////////////////////////////////////////////////////////

#include "TerrainHtServer.hh"
#include "ServerThread.hh"
#include <iostream>

using namespace std;

// Constructor

ServerThread::ServerThread(TerrainHtServer *parent, const Params &params) :
        _parent(parent),
        _params(params)
  
{

  _abyss = NULL;

}

// destructor

ServerThread::~ServerThread()

{

  if (_abyss) {
    _abyss->terminate();
  }

}

//////////////////////////////////////////////////
// run

void ServerThread::run()
{
  
  try {
    
    // create registry

    xmlrpc_c::registry myRegistry;
    
    // register methods

    xmlrpc_c::methodPtr const 
      getHeightMethodP(new getHeightMethod(_parent, _params));
    myRegistry.addMethod("get.height", getHeightMethodP);
    
    // create the server
    
    int port = _params.xmlrpc_server_port;
    _abyss = new xmlrpc_c::serverAbyss
      (xmlrpc_c::serverAbyss::constrOpt().registryP(&myRegistry).portNumber(port));

    // run the server

    _abyss->run();
    
  } catch (exception const& e) {

    cerr << "ERROR - TerrainHtServer::ServerThread::run()" << endl;
    cerr << e.what() << endl;

  }

}

/////////////////////////////////////////////////////
// define rpc method for getting height

ServerThread::getHeightMethod::getHeightMethod(TerrainHtServer *parent, 
                                               const Params &params) :
        _parent(parent),
        _params(params)

{

  // incoming args are lat/lon
  // send back [double heightM, bool isWater] as an array

  _signature = "S:dd";

  // method's result and two arguments are integers

  _help = "For given lat/lon, returns terrain ht (m) and water flag";

}

///////////////////////////////////////////////////////
// get the terrain ht and return to the client

void ServerThread::getHeightMethod::execute
  (xmlrpc_c::paramList const& paramList,
   xmlrpc_c::value * const retvalP)

{

  // get lat/lon from calling params

  double const lat(paramList.getDouble(0));
  double const lon(paramList.getDouble(1));
  paramList.verifyEnd(2);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Server got request, lat, lon: " << lat << ", " << lon << endl;
  }

  // get the ht and water flag

  double heightM = -9999.0;
  bool isWater = false;
  bool isError = false;
  
  if (_parent->getHt(lat, lon, heightM, isWater)) {
    cerr << "ERROR - ServerThread::getHeightMethod()" << endl;
    cerr << "  Cannot get height for lat, lon: " << lat << ", " << lon << endl;
    isError = true;
  }

  // Make the return struct - { double ht, bool isWater }

  map<string, xmlrpc_c::value> replyData;
  pair<string, xmlrpc_c::value> heightVal("heightM", xmlrpc_c::value_double(heightM));
  replyData.insert(heightVal);
  pair<string, xmlrpc_c::value> waterVal("isWater", xmlrpc_c::value_boolean(isWater));
  replyData.insert(waterVal);
  pair<string, xmlrpc_c::value> errorVal("isError", xmlrpc_c::value_boolean(isError));
  replyData.insert(errorVal);
  
  // Make an XML-RPC struct for reply
  
  xmlrpc_c::value_struct const reply(replyData);
  *retvalP = reply;

}
