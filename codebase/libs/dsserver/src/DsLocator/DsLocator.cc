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
/////////////////////////////////////////////////////////////////////////////
//
// NOTE: This class may modify IN PLACE the url that is passed in
//       by resolving the host, port, and file specifications
//       The caller is responsible for keeping a copy of the
//       original URL if it is needed.
//
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1999
//
/////////////////////////////////////////////////////////////////////////////

#include <toolsa/port.h>
#include <toolsa/Path.hh>
#include <toolsa/GetHost.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <didss/DsURL.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DmapAccess.hh>
#include <dsserver/DsSvrMgrSocket.hh>
#include <dsserver/DsServerMsg.hh>

#define __in_DsLocator_cc_file__
#include <dsserver/DsLocator.hh>
#undef __in_DsLocator_cc_file__

using namespace std;

DsLOCATOR::DsLOCATOR()
{

  // set base port
  
  basePort = DS_BASE_PORT;
  
  // Override the base port with DS_BASE_PORT environment variable,
  // if active

  const char *overrideEnv = getenv( "DS_BASE_PORT" );
  if ( overrideEnv != NULL ) {
    int overridePort;
    if ( (sscanf( overrideEnv, "%d", &overridePort ) == 1 ) &&
         overridePort > 1023 ) {
      basePort = overridePort;
    } 
    else {
      cerr << "WARNING - DsLocator::DsLocator" << endl;
      cerr << "  " << DateTime::str() << endl;
      cerr << "  $DS_BASE_PORT env variable not a valid port number\n"
           << "  Port number must be an integer greater than 5000\n"
           << "  Using default port " << basePort << endl;
    }
  }

  // calculate the port offset

  baseOffset = basePort - DS_BASE_PORT;
  
}

/////////////////////////////////////////////////////////////
// Resolve three parts of the URL host, port, and file.  
// The param specification of the URL must be resolved explicitly
//
// If allowMgrContact is false, will not contact DsServerMgr,
// just uses the default port instead if port must be resolved.
//
// If errStr is non-NULL, errors will be written to this string.
// If NULL, errors will be written to stderr/cerr.
//
// If forceServerContact is true, then the server port
// will always be resolved.
//
// returns: 0 on success, -1 on failure
//

int
DsLOCATOR::resolve( DsURL &url, bool *contactServer /* = NULL*/,
                    bool allowMgrContact /* = true */,
                    string *errStr /* = NULL */,
                    bool forceServerContact /* = false */ )
{
   //
   // NOTE: returns success(0) or failure(-1)
   //  Upon success, if contactServer is non-null, we'll also return a flag
   //  indicating the need to contact a server based on the resolved URL
   //  contactServer == true:  server contact is specified
   //  contactServer == false: local disk i/o is specified
   //
   int status;

   // if protocol is missing, the url must point to a file on the
   // local host

   const string &proto = url.getProtocol();
   if (proto.empty()) {
     if (contactServer) {
       *contactServer = false;
     }
     if (forceServerContact) {
       if (errStr) {
         TaStr::AddStr(*errStr, "ERROR - DsLOCATOR::resolve", "");
         TaStr::AddStr(*errStr, "  Server contact is forced", "");
         TaStr::AddStr(*errStr, "  Must specify protocol in URL", "");
         TaStr::AddStr(*errStr, "  URL: ", url.getURLStr());
         TaStr::AddStr(*errStr, "  Time: ", DateTime::str());
       }
       return -1;
     } else {
       return 0;
     }
   }

   //
   // Resolve the host name
   //
   status = resolveHost( url );
   if ( status != 0 ) {
     if (forceServerContact) {
       if (errStr) {
         TaStr::AddStr(*errStr, "ERROR - DsLOCATOR::resolve", "");
         TaStr::AddStr(*errStr, "  Server contact is forced", "");
         TaStr::AddStr(*errStr, "  Must specify host in URL", "");
         TaStr::AddStr(*errStr, "  URL: ", url.getURLStr());
         TaStr::AddStr(*errStr, "  Time: ", DateTime::str());
       }
       return -1;
     } else {
       return( status );
     }
   }

   // check forwarding
   if (url.forwardingActive()) {
     // set port to 0 to force contact
     if (url.getPort() < 0) {
       url.setPort(0);
     }
   }

   //
   // Resolve the port
   //
   bool serverIo;
   status = resolvePort( url, &serverIo, allowMgrContact,
                         errStr, forceServerContact);
   if ( contactServer ) {
      *contactServer = serverIo;
   }
   if ( status != 0 ) {
     if (forceServerContact) {
       if (errStr) {
         TaStr::AddStr(*errStr, "ERROR - DsLOCATOR::resolve", "");
         TaStr::AddStr(*errStr, "  Server contact is forced", "");
         TaStr::AddStr(*errStr, "  Cannot resolve port", "");
         TaStr::AddStr(*errStr, "  URL: ", url.getURLStr());
         TaStr::AddStr(*errStr, "  Time: ", DateTime::str());
       }
       return -1;
     } else {
       return( status );
     }
   }

   //
   // Resolve the file path
   //
   if (!serverIo) {
     status = resolveFile( url );
   }

   return( status );
}

// Resolve port.  
//
// If allowMgrContact is false, will not contact DsServerMgr,
// just uses the default port instead if port must be resolved.
//
// If errStr is non-NULL, errors will be written to this string.
// If NULL, errors will be written to stderr/cerr.
//
// If forceServerContact is true, then the server port
// will always be resolved.
//
// returns: 0 on success, -1 on failure
//

int
DsLOCATOR::resolvePort( DsURL &url, bool *contactServer /* = NULL*/,
			bool allowMgrContact /* = true */,
                        string *errStr /* = NULL */,
			bool forceServerContact /* = false */)
{
   bool    serverIo = false, contactMgr = false, setPort = false;
   int     port = 0;

   // initialize error string if set
   
   if (errStr) {
     *errStr = "";
   }

   //
   // Make sure our URL is valid
   //
   if ( !url.isValid() ) {
      url.getURLStr();
      if ( !url.isValid() ) {
         return( -1 );
      }
   }

   //
   // Get the port, specified or not
   //
   port = url.getPort();

   //
   // Determine if the localhost keyword is specified for no server contact.
   // Note that if there is a translator we must contact the server.
   //
   const string &host  = url.getHost();
   const string &trans = url.getTranslator();
   const string &param = url.getParamFile();

   //
   // Use the port number to see if we should force a server connect 
   // or if we have the option to use local disk I/O
   //
   switch( port ) {
   case -1:
     {
       //
       // Unspecified port, we decide based on hostname (via IP address)
       // NOTE: if tranform or parameter is specified, we must
       //       contact a server, even if the host is local
       //
       GetHost getHost;
       if ( trans.empty() && param.empty()  &&
	    getHost.hostIsLocal(host) &&
            !forceServerContact) {
	 serverIo   = false;
	 contactMgr = false;
	 setPort    = false;
       }
       else {
	 serverIo   = true;
	 contactMgr = true;
	 setPort    = true;
       }
     }
   break;
   case 0:
     //
     // Specified but unknown port, force a server connect
     // NOTE: useful for debugging client & server on localhost
     //
     serverIo   = true;
     contactMgr = true;
     setPort    = true;
     break;
   case DsURL::DefaultPortIndicator:
     //
     // Specified default port, force a server connect but don't
     // contact serverMgr -- assume server is already running
     //
     serverIo   = true;
     contactMgr = false;
     setPort    = true;
     break;
   default:
     //
     // Specified known port, use it without question
     //
     serverIo   = true;
     contactMgr = false;
     setPort    = false;
     break;
   }

   //
   // If necessary, set the url port
   //
   if ( setPort ) {
      port = getDefaultPort( url );
      if ( port == -1 ) {
        if (errStr) {
          TaStr::AddStr(*errStr, "", "ERROR - no default port for url");
          TaStr::AddStr(*errStr, "url", url.getURLStr());
          TaStr::AddStr(*errStr, "  Time: ", DateTime::str());
        } else {
          cerr << "ERROR - no default port for url" << endl;
          cerr << "url: " << url.getURLStr() << endl;
          cerr << "  " << DateTime::str() << endl;
        }
         return( -1 );
      }
      else {
         url.setPort( port );
         url.getURLStr();
      }
   }

   //
   // If necessary, contact the DsServerMgr to kick off the server
   //
   if ( contactMgr && allowMgrContact ) {
      DsSvrMgrSocket serverMgr;
      int waitMsecs = 30000;
      string mgrMsg;
      if ( serverMgr.findPortForURL( host.c_str(), url,
				     waitMsecs, mgrMsg )) {
        if (errStr) {
          TaStr::AddStr(*errStr, "",
                        "ERROR - cannot resolve port from ServerMgr");
          TaStr::AddStr(*errStr, "  Time: ", DateTime::str());
          TaStr::AddStr(*errStr, "  ", mgrMsg);
        } else {
          cerr << "ERROR - cannot resolve port from ServerMgr" << endl;
          cerr << "  " << DateTime::str() << endl;
          cerr << "  " << mgrMsg << endl;
        }
	return( -1 );
      }
   }
   
   //
   // Let the caller know the intent for server connection, if requested
   //
   if ( contactServer ) {
      *contactServer = serverIo;
   }
   return 0;
}

int
DsLOCATOR::resolveHost( DsURL &url )
{
   //
   // Make sure our URL is valid
   //
   if ( !url.isValid() ) {
      url.getURLStr();
      if ( !url.isValid() ) {
         return( -1 );
      }
   }

   //
   // Fill out the host name if necessary
   //
   string host = url.getHost();
   if ( host.empty() ){
      // 
      // Contact DataMapper to get the host name
      //
      DmapAccess dmap;
      if ( dmap.reqSelectedInfo( url.getDatatype(), url.getFile() ))
	return( -1 );
      if (dmap.getNInfo() < 1) {
	return (-1);
      }
      DMAP_info_t dmapInfo = dmap.getInfo( 0 );
      url.setHost(dmapInfo.hostname);

   }

   return( 0 );
}

int
DsLOCATOR::resolveFile( DsURL &url )
{
   //
   // Make sure our URL is valid
   //
   if ( !url.isValid() ) {
      url.getURLStr();
      if ( !url.isValid() ) {
         return( -1 );
      }
   }

   //
   // Fill out the file path
   //
   string filePath;
   RapDataDir.fillPath( url, filePath );
   url.setFile( filePath );
   url.getURLStr();
   if ( !url.isValid() ) {
      return( -1 );   
   }

   return( 0 );
}

int
DsLOCATOR::resolveParam( DsURL &url, const string &serverName,
                         bool* doesParamExist /* = NULL */ )
{
   //
   // Initialize the flag for parameter file existence
   //
   if ( doesParamExist ) {
      *doesParamExist = false;
   }

   //
   // Make sure our URL is valid
   //
   if ( !url.isValid() ) {
      url.getURLStr();
      if ( !url.isValid() ) {
         return( -1 );
      }
   }

   //
   // Resolve the file first because we're going to
   // use the file path in locating the parameter file
   //
   if ( resolveFile( url )) {
      return( -1 );
   }

   //
   // See if there is a parameter specification in the url
   //
   string paramSpec;
   string paramPath;
   string filePath;

   paramSpec = url.getParamFile();  

   // if no param spec, then use default string "params"

   if ( paramSpec.size() == 0) {
     paramSpec = "params";
   }
   
   if ( paramSpec[0] == '/' ) {

     //
     // Explicit parameter path starts with '/', 
     // so it is fully qualified -- use it as is.
     //
     paramPath = paramSpec;

   } else {

     // compute param name

     string paramName = "_";
     paramName += serverName;
     paramName += ".";
     paramName += paramSpec;
     
     // look for a '_serverName.paramSpec' file
     // in the URL data directory
     
     RapDataDir.fillPath( url.getFile(), paramPath );
     paramPath += PATH_DELIM;
     paramPath += paramName;

     // check if file exists
     
     if (!Path::exists(paramPath)) {

       // no luck, try DS_PARAMS_DIR if it exists

       char *dsParamsDir = getenv("DS_PARAMS_DIR");
       if (dsParamsDir != NULL) {
	 paramPath = dsParamsDir;
	 paramPath += PATH_DELIM;
	 paramPath += paramName;
       }
       
     } // if (!Path::exists ...
     
   } // if ( paramSpec[0] == '/' )

   if ( Path::exists( paramPath ) ) {
      //
      // We've got a param path, put it in the url
      //
      url.setParamFile( paramPath );
      url.getURLStr();
      if ( !url.isValid() ) {
         return( -1 );
      }

      //
      // Set the existence flag, if requested
      //
      if ( doesParamExist ) {
         *doesParamExist = true;
      }
   }

   return( 0 );
}

////////////////////////////////////////////////////////
// ping the server specified in the URL
//
// If not up and port is not specified,
// contact the server mgr to set up the server.
//
// If errStr is non-NULL, errors will be written to this string.
// If NULL, errors will be written to stderr/cerr.

int
DsLOCATOR::pingServer( DsURL &url,
                       string *errStr /* = NULL */ )

{

  // initialize error string if set

  if (errStr) {
    *errStr = "";
  }

  if (_checkServerStatus(url, errStr)) {
    
    if (url.getPort() == DsLocator.getDefaultPort(url)) {
      
      url.setPort(0);
      if (DsLocator.resolve(url, NULL, true, errStr)) {
	return -1;
      }

      // try the check again, using ServerMgr-supplied port

      if (_checkServerStatus(url, errStr)) {
	return -1;
      }

    } else {
      
      // not using default port, so no point in retrying
      return -1;
      
    }
    
  }
  
 return 0;

}

////////////////////////////////////////////////
// Use status request to check that server is OK

int
DsLOCATOR::_checkServerStatus(const DsURL &url,
                              string *errStr)
  
{
  
  // compose message

  DsServerMsg msg;
  msg.setCategory(DsServerMsg::ServerStatus);
  msg.setType(DsServerMsg::IS_ALIVE);
  void * msgToSend = msg.assemble();
  int msgLen = msg.lengthAssembled();
  
  // check for forwarding
  
  if (url.prepareForwarding("DsLOCATOR::pingServer", msgLen)) {
    if (errStr) {
      TaStr::AddStr(*errStr, "",
                    "ERROR - DsLOCATOR::pingServer::_checkServerStatus");
      TaStr::AddStr(*errStr, "", url.getErrString());
    } else {
      cerr << "ERROR - DsLOCATOR::pingServer::_checkServerStatus" << endl;
      cerr << url.getErrString() << endl;
    }
    return -1;
  }

  // compute ping timeout

  int pingTimeoutMsecs = DS_DEFAULT_PING_TIMEOUT_MSECS;
  char *DS_PING_TIMEOUT_MSECS = getenv("DS_PING_TIMEOUT_MSECS");
  if (DS_PING_TIMEOUT_MSECS != NULL) {
    int timeout;
    if (sscanf(DS_PING_TIMEOUT_MSECS, "%d", &timeout) == 1) {
      pingTimeoutMsecs = timeout;
    }
  }
  if (url.useForwarding()) {
    pingTimeoutMsecs *= 5;
  }

  // open socket for status request

  ThreadSocket sock;
  if (url.useForwarding()) {
    if (sock.open(url.getForwardingHost().c_str(), url.getForwardingPort())) {
      if (errStr) {
        TaStr::AddStr(*errStr, "",
                      "ERROR - DsLOCATOR::pingServer::_checkServerStatus");
        TaStr::AddStr(*errStr, "", sock.getErrString());
      } else {
        cerr << "ERROR - DsLOCATOR::pingServer::_checkServerStatus" << endl;
        cerr << sock.getErrString() << endl;
      }
      return -1;
    }
  } else {
    if (sock.open(url.getHost().c_str(), url.getPort())) {
      return -1;
    }
  }
  
  // if forwarding is active, send the http header

  if (url.useForwarding()) {
    if (sock.writeBuffer((void *) url.getHttpHeader().c_str(),
			 url.getHttpHeader().size(),
			 pingTimeoutMsecs)) {
      if (errStr) {
        TaStr::AddStr(*errStr, "",
                      "ERROR - DsLOCATOR::pingServer::_checkServerStatus");
        TaStr::AddStr(*errStr, "", sock.getErrString());
      } else {
        cerr << "ERROR - DsLOCATOR::pingServer::_checkServerStatus" << endl;
        cerr << sock.getErrString() << endl;
      }
      sock.close();
      return -1;
    }
  }

  // ping the server to get status

  int status = sock.writeMessage(0, msgToSend, msgLen, pingTimeoutMsecs);
  if (status < 0) {
      if (errStr) {
        TaStr::AddInt(*errStr,
                      "Ping to server timed out, waited msecs: ", 
                      pingTimeoutMsecs);
      } else {
        cerr << "Ping to server timed out, waited msecs: "
             << pingTimeoutMsecs << endl;
      }
    sock.close();
    return -1;
  }
  
  // if forwarding is active, strip the http header from reply

  string httpHeader;
  if (url.useForwarding()) {
    if (sock.stripHttpHeader(httpHeader, pingTimeoutMsecs)) {
      sock.close();
      return -1;
    }
  }
  
  // Read in the reply.
  if (sock.readMessage(pingTimeoutMsecs)) {
    sock.close();
    return -1;
  }
  sock.close();

  return 0;
  
}



int
DsLOCATOR::getServerName( const DsURL &url, string& serverName )
{
   bool          found = false;
   serverInfo_t *info;

   //
   // If a traslator is specified in the url,
   // it must be the name of the executable
   //
   string urlTranslator = url.getTranslator();

   if ( urlTranslator.size() ) {
      serverName = urlTranslator;
      return( 0 );
   }

   //
   // Otherwise, the serverName is based on the protocol
   //
   string urlProtocol   = url.getProtocol();

   for( info=(serverInfo_t*)serverInfo; info->serverName; info++ ) {
      if ( urlProtocol == info->protocol  && !info->translator ) {
         found = true;
         serverName = info->serverName;
         break;
      }
   }
   return( found ? 0 : -1 );
}

int
DsLOCATOR::getDefaultPort( const string& serverName )
{
   int           port = -1;
   serverInfo_t *info;

   for( info=(serverInfo_t*)serverInfo; info->serverName; info++ ) {
      if ( serverName == info->serverName ) {
         port = info->port + baseOffset;
         break;
      }
   }
   return( port );
}

int
DsLOCATOR::getDefaultPort( const DsURL &url )
{
   int     port = -1;
   string  serverName;

   if ( getServerName( url, serverName ) == 0 ) {
      port = getDefaultPort( serverName );
   }

   return( port );
}
