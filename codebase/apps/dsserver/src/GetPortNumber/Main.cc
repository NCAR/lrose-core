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

#include <iostream>
#include <dsserver/DsLocator.hh>
#include <didss/DsURL.hh>
#include <string>
#include <cstring>
#include <cstdlib>
using namespace std;

void Usage();

int main(int argc, char *argv[]){
  //
  // Parse command line args. Valid options are
  // -url <url> OR -Server <server> with -h being
  // a cry for help.
  //
  bool urlSpecified = false, serverNameSpecified = false;
  string Url, serverName;

  if (argc < 2){
    Usage();
  }

  for (int i=1; i < argc; i++){
    //
    // Answer requests for help.
    //
    if (
	(!(strcmp(argv[i],"-h"))) ||
	(!(strcmp(argv[i],"--")))
	){
      Usage();
    }
    //
    // Set the URL, if properly specified.
    //
    if (!(strcmp(argv[i],"-url"))){
      i++;
      if (i < argc){
	urlSpecified = true;
	Url = argv[i];
      } else {
	break;
      }
    }
    //
    // Set the Server Name, if properly specified.
    //
    if (!(strcmp(argv[i],"-Server"))){
      i++;
      if (i < argc){
	serverNameSpecified = true;
	serverName = argv[i];
      } else {
	break;
      }
    }
  }
  //
  // Now done with command line arguments.
  //
  
  if ( (!(serverNameSpecified)) && (!(urlSpecified)) ){
    Usage();
  }

  //
  // Process the server name, if specified.
  //
  if (serverNameSpecified){
    cout << "Default port for server name " <<  serverName << " : ";
    int port = DsLocator.getDefaultPort( serverName );
    if (-1 == port){
      cout << "UNRESOLVED";
    } else {
      cout << port;
    }
    cout << endl;
  }
  //
  // Process the URL name, if specified.
  //
  if (urlSpecified){
    cout << "Default port for URL " <<  Url << " : ";
    DsURL U(Url);
    int port = DsLocator.getDefaultPort( U );
    if (-1 == port){
      cout << "UNRESOLVED";
    } else {
      cout << port;
    }
    cout << endl;
  }








  return 0;
}
//
//
void Usage(){

  cerr << "Usage : " << endl;
  cerr << "GetPortNumber -url <url> and/or" << endl;
  cerr << "GetPortNumber -Server <serverName>" << endl;
  cerr << "GetPortNumber -h prints this message." << endl;

  exit(0);

}

