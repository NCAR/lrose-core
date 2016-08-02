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
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
using namespace std;

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#define SLEEP(seconds) sleep(seconds);

class getHeightMethod : public xmlrpc_c::method {
  
public:

  getHeightMethod() {
    // incoming args are lat/lon
    // send back [double heightM, bool isWater] as an array
    this->_signature = "A:dd";
    // method's result and two arguments are integers
    this->_help = "For given lat/lon, returns terrain ht (m) and water flag";
  }

  void
    execute(xmlrpc_c::paramList const& paramList,
            xmlrpc_c::value * const retvalP) {
    double const lat(paramList.getDouble(0));
    double const lon(paramList.getDouble(1));
    paramList.verifyEnd(2);
    cerr << "11111111 lat, lon: " << lat << ", " << lon << endl;

    // Make the vector value 'arrayData'
    vector<xmlrpc_c::value> arrayData;
    double heightM = lat + lon;
    bool isWater = (lat > 0);
    arrayData.push_back(xmlrpc_c::value_double(heightM));
    arrayData.push_back(xmlrpc_c::value_boolean(isWater));
    // Make an XML-RPC array out of it
    xmlrpc_c::value_array reply(arrayData);
    *retvalP = reply;
  }

};

int main(int const, const char ** const) {

  try {

    // create registry

    xmlrpc_c::registry myRegistry;

    // register methods

    xmlrpc_c::methodPtr const getHeightMethodP(new getHeightMethod);
    myRegistry.addMethod("sample.add", getHeightMethodP);

    // set up the server
    
    xmlrpc_c::serverAbyss myAbyssServer(
            xmlrpc_c::serverAbyss::constrOpt()
            .registryP(&myRegistry)
            .portNumber(8080));

    // run the server

    myAbyssServer.run();

  } catch (exception const& e) {
    
    cerr << "Something failed. " << e.what() << endl;

  }

  return 0;

} 
