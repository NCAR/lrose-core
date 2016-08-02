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
#include <cstdlib>
#include <string>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client_simple.hpp>

using namespace std;

int main(int argc, char **argv) {

  if (argc != 3) {
    cerr << "Usage: client lat lon" << endl;
    return -1;
  }

  double lat = atof(argv[1]);
  double lon = atof(argv[2]);

  cerr << "Getting ht for lat, lon: " << lat << ", " << lon << endl;
  
  try {

    // make a call to the server, sending lat/lon as double args
    
    string const serverUrl("http://localhost:9090/RPC2");
    string const methodName("get.height");
    xmlrpc_c::clientSimple myClient;
    xmlrpc_c::value result;
    myClient.call(serverUrl, methodName, "dd", &result, lat, lon);
    
    // decode response struct { double ht, bool isWater }
    
    xmlrpc_c::value_struct reply(result);
    map<string, xmlrpc_c::value> const replyData =
      static_cast<map<string, xmlrpc_c::value> >(reply);
    map<string, xmlrpc_c::value>::const_iterator htP = replyData.find("heightM");
    map<string, xmlrpc_c::value>::const_iterator waterP = replyData.find("isWater");
    map<string, xmlrpc_c::value>::const_iterator errorP = replyData.find("isError");
    
    double heightM = -9999.0;
    bool isWater = false;
    bool isError = false;
    
    if (htP != replyData.end()) {
      heightM = (xmlrpc_c::value_double) htP->second;
    }

    if (waterP != replyData.end()) {
      isWater = (xmlrpc_c::value_boolean) waterP->second;
    }
    
    if (errorP != replyData.end()) {
      isError = (xmlrpc_c::value_boolean) errorP->second;
    }
    
    cerr << "  ==>> heightM: " << heightM << endl;
    cerr << "  ==>> isWater: " << isWater << endl;
    cerr << "  ==>> isError: " << isError << endl;
    
  } catch (exception const& e) {

    cerr << "Client threw error: " << e.what() << endl;

  } catch (...) {

    cerr << "Client threw unexpected error." << endl;

  }
  
  return 0;

} 

  
  
