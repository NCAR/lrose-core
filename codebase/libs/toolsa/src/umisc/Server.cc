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
// $Id: Server.cc,v 1.10 2016/03/03 18:00:26 dixon Exp $
//


#include <cstdlib>
#include <toolsa/utim.h>
#include <toolsa/servmap.h>
#include <toolsa/sok2.h>
#include <toolsa/Server.hh>
using namespace std;

//
// Static initialization of local servmap host
//
const char* Server::localHost = "local";


Server::Server()
{
   port = 0;
}

Server::Server( const char *t, const char *s, const char *i, 
                const char *h, int p ) 
{
   setInfo( t, s, i, h, p );
}

int
Server::setInfo( const char *t, const char *s, const char *i,
                 const char *h, int p )
{
   int status = 0;

   type          = t;
   subtype       = s;
   instance      = i;

   //
   // If host or port were unspecified, try to locate the server via servmap
   //
   if ( h && p ) {
      host = h;
      port = p;
   }
   else {
      status = locate();
   }

   return status;
}

const char*
Server::getServmapHost()
{
   //
   // Return static char* to name of servmap host
   //
   const char *servmapHost;

   servmapHost = getenv( "SERVMAP_HOST" );
   if ( servmapHost == NULL )
      servmapHost = localHost;

   return( servmapHost );
}

int
Server::locate ()
{
   //
   // Locate a particular server based on type, subtype and instance
   //
   char *locatedHost;
   int   locatedPort;

   //
   // Where is the server mapper?
   //
   if ( SOK2setServiceFile( SOK2_SERVMAP, (char*)getServmapHost() ) != 1 ) {
      return -1;
   }

   //
   // Locate server of the proper type, subtype, and instance
   //
   if ( SOK2servmapInfo( (char*)type.c_str(), 
                         (char*)subtype.c_str(), 
                         (char*)instance.c_str(), 
                         &locatedHost, &locatedPort ) <= 0 ) {
      return -1;
   }

   //
   // Hang onto the located server info
   //
   host = locatedHost;
   port = locatedPort;

   return 0;
}
