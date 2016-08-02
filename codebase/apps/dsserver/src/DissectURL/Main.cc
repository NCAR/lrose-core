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
#include <cstdlib>
using namespace std;

void Usage();

int main(int argc, char *argv[]){


  if (argc < 2){
    Usage();
  }


  string Url(argv[1]);
  DsURL U( Url );

  //
  // Process the URL 
  //

  cout << "The following information is from the DsLocator class : " << endl;
  //
  // Port number.
  //
  cout << "Default port for URL " <<  Url << " from DsLocator : ";
  int port = DsLocator.getDefaultPort( U );
  if (-1 == port){
    cout << "UNRESOLVED";
  } else {
    cout << port;
  }
  cout << endl;
  //
  // Protocol.
  //
  cout << "Protocol : " << U.getProtocol() << endl;
  //
  //
  // Server name.
  //
  string serverName;
  cout << "Default server name for URL " <<  Url << " from DsLocator : ";
  int ret = DsLocator.getServerName( U, serverName );
  if (-1 == ret){
    cout << "UNRESOLVED";
  } else {
    cout << serverName;
  }
  cout << endl << endl;
  cout << "The following information is from the DsURL class : " << endl;
  //
  // The rest is up to U
  //
  cout << "Translator for URL " << Url << " : ";
  cout << U.getTranslator() << endl;
  //
  cout << "Parameter file for URL " << Url << " : ";
  cout << U.getParamFile() << endl;
  //
  cout << "Host for URL " << Url << " : ";
  cout << U.getHost() << endl;
  //
  cout << "Port for URL " << Url << " : ";
  cout << U.getPort() << endl;
  //
  cout << "File for URL " << Url << " : ";
  cout << U.getFile() << endl;
  //
  cout << "Args for URL " << Url << " : ";
  cout << U.getArgs() << endl;
  //
  cout << "Is forwarding specified in URL " << Url << " : ";
  if (U.forwardingActive()){
    cout << "TRUE";
  } else {
     cout << "FALSE";
  }
  cout << endl;

  return 0;
}
//
//
void Usage(){
  cerr << "Usage : " << endl;
  cerr << "DissectURL <url>" << endl;
  exit(0);

}

