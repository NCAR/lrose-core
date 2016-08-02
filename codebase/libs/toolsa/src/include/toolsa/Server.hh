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
//
// $Id: Server.hh,v 1.9 2016/03/03 19:26:12 dixon Exp $
//

#ifndef _SERVER_INC_
#define _SERVER_INC_

#include <string>
using namespace std;


//
// This is an old server class moved from libs/nowcast
// It will soon become obsolete with the new DIDSS implementation
// In the meantime, it just needed a new home
//

class Server
{
public:
   Server();
   Server( const char *type, const char *subtype, const char *instance, 
           const char *host = NULL, int port = 0 );
  ~Server(){};

   //
   // Fetching server characteristics
   //
   const string&  getInstance(){ return instance; }
   const string&  getHost(){ return host; }
   int            getPort(){ return port; }

   //
   // Setting server characteristics
   //
   int setInfo( const char* type, 
                const char* subtype, 
                const char* instance,
                const char* host = NULL, 
                int port = 0 );

   //
   // Locating the server
   //
   static const char*   localHost;

   int            locate();
   const char*    getServmapHost();


protected:
   string         type;
   string         subtype;
   string         instance;
   string         host;
   int            port;
};

# endif
