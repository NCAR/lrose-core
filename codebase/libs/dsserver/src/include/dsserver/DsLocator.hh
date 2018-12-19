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
// This file not only implements the class,
// but also declares a static instance of the class.
// For example, to get a DsURL resolved:
//
//    #include <dsserver/DsLocator.hh>
//    int status = DsLocator.resolve( myUrl, &contactServer );
// 
// Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
// January 1999
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _DSLOCATOR_INC_
#define _DSLOCATOR_INC_

#include <string>
using namespace std;

//
// Forward references
//
class DsURL;


class DsLOCATOR
{

public:

   DsLOCATOR();
  ~DsLOCATOR(){};


   // The BASE PORT may be reset by setting the environment variable:
   //   DS_BASE_PORT
   // All ports will be reset by the difference between DS_BASE_PORT and
   // the default base port of 5430

   static const char*   LocalHostKeyword;
   static const size_t  DS_BASE_PORT;

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
   int   resolve( DsURL &url, bool *contactServer = NULL,
		  bool allowMgrContact = true,
                  string *errStr = NULL,
                  bool forceServerContact = false );

   // Resolve three parts of the URL host, port, and file.  
   // The param specification of the URL must be resolved explicitly.
   //
   // Forces contact with server and server manager, as appropriate.
   //
   // If errStr is non-NULL, errors will be written to this string.
   // If NULL, errors will be written to stderr/cerr.
   //
   // returns: 0 on success, -1 on failure
   //
   int   resolveForced( DsURL &url, string *errStr = NULL );

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
   int   resolvePort( DsURL &url, bool *contactServer = NULL,
                      bool allowMgrContact = true,
                      string *errStr = NULL,
                      bool forceServerContact = false);

   // Resolve host and file.
   // Can be resolved explicitly or through the resolve() method
   // returns: 0 on success, -1 on failure
   //
   int   resolveHost( DsURL &url );
   int   resolveFile( DsURL &url );

   //
   // Parameter specification must be resolved explicitly,
   // i.e., is not called from resolve()
   //
   // returns: 0 on success, -1 on failure
   //
   int   resolveParam( DsURL &url, const string &serverName, 
                       bool *doesParamExist = NULL );

  // ping the server specified in the URL, to make sure it is available.
  //
  // If not up and port is not specified,
  // contact the server mgr to set up the server.
  //
  // If errStr is non-NULL, errors will be written to this string.
  // If NULL, errors will be written to stderr/cerr.

  int pingServer( DsURL &url,
                  string *errStr = NULL );
    
   //
   // Get server executable name from URL
   // The server name is derived from a unique combination of the
   // URL protocol and translator.  Called by DsServerMgr.
   //
   // returns: 0 on success, -1 on failure
   //
   int getServerName( const DsURL &url, string& serverName );

   //
   // Get the default port for a named server.  Called by DsServerMgr.
   //
   // returns: port number on success, -1 on failure
   //
   int getDefaultPort( const string& serverName );

   // 
   // Get the default port for a DsURL
   // The default port is derived from a unique combination of the
   // URL protocol and translator.
   // 
   // returns: port number on success, -1 on failure
   //
   int getDefaultPort( const DsURL& url );

private:

  int basePort;
  int baseOffset;

  // Initialization struct for server/port/prototype mapping
  
  typedef struct {
    const char* serverName;
    int   port;
    const char* protocol;
    bool  translator;
  } serverInfo_t;
  
  static const serverInfo_t serverInfo[];

  static int _checkServerStatus(const DsURL &url,
                                string *errStr);

};

//
// Make one static copy available for everyone to get to
//
static DsLOCATOR DsLocator;

// initialize variables and the serverInfo array.
// this is only done once, in the DsLocator.cc file.

#ifdef __in_DsLocator_cc_file__

const char*   DsLOCATOR::LocalHostKeyword    = "localhost";
const size_t  DsLOCATOR::DS_BASE_PORT        = 5430;

// well-known port lookup table
//
// When adding a server, you can use the Spare slots, that way you do
// not force a relink of the applications
//
// NOTE: for non-translating servers (URL Translator == false)
//       there must be a one-to-one correspondance between the
//       executable name and the URL Protocol.  
//       

const DsLOCATOR::serverInfo_t DsLOCATOR::serverInfo[] =

  {

    // Executable               Port    URL       URL
    // Name                     Num     Protocol  Translator

    { "servmap",                 5432,   "",       false },
    { "procmap",                 5433,   "",       false },
    { "DataMapper",              5434,   "",       false },
    { "DsServerMgr",             5435,   "",       false },
    { "DsMdvServer",             5440,   "mdvp",   false },
    { "DsSpdbServer",            5441,   "spdbp",  false },
    { "DsProxyServer",           5442,   "proxyp", false },
    { "DsFmqServer",             5443,   "fmqp",   false },
    { "DsFileServer",            5444,   "filep",  false },
    { "DsFCopyServer",           5445,   "fcopyp", false },
    { "DsTitanServer",           5446,   "titanp", false },
    { "DsLdataServer",           5447,   "ldatap", false },
    { "CSpareServer",            5448,   "",       false },
    { "DSpareServer",            5449,   "",       false },
    { "Ltg2Symprod",             5450,   "spdbp",  true  },
    { "AcTrack2Symprod",         5451,   "spdbp",  true  },
    { "Bdry2Symprod",            5452,   "spdbp",  true  },
    { "Chunk2Symprod",           5453,   "spdbp",  true  },
    { "FltPath2Symprod",         5454,   "spdbp",  true  },
    { "Mad2Symprod",             5455,   "spdbp",  true  },
    { "Metar2Symprod",           5456,   "spdbp",  true  },
    { "Pirep2Symprod",           5457,   "spdbp",  true  },
    { "PosnRpt2Symprod",         5458,   "spdbp",  true  },
    { "Sigmet2Symprod",          5459,   "spdbp",  true  },
    { "Tstorms2Symprod",         5460,   "spdbp",  true  },
    { "TrecGauge2Symprod",       5461,   "spdbp",  true  },
    { "Vergrid2Symprod",         5462,   "spdbp",  true  },
    { "WxHazards2Symprod",       5463,   "spdbp",  true  },
    { "BasinGenPt2Symprod",      5464,   "spdbp",  true  },
    { "GenPt2Symprod",           5465,   "spdbp",  true  },
    { "GenPtField2Symprod",      5466,   "spdbp",  true  },
    { "Acars2Symprod",           5467,   "spdbp",  true  },
    { "HydroStation2Symprod",    5468,   "spdbp",  true  },
    { "SigAirMet2Symprod",       5469,   "spdbp",  true  },
    { "StormPolygon2Symprod",    5470,   "spdbp",  true  },
    { "WMS2MdvServer",           5471,   "mdvp",   true  },
    { "GenPoly2Symprod",         5472,   "spdbp",  true  },
    { "Rhi2Symprod",             5473,   "spdbp",  true  },
    { "DsMdvClimoServer",        5474,   "mdvp",   true  },
    { "Edr2Symprod",             5475,   "spdbp",  true  },
    { "Sndg2Symprod",            5476,   "spdbp",  true  },
    { "simpleAcTrack2Symprod",   5477,   "spdbp",  true  },
    { "Asdi2Symprod",            5478,   "spdbp",  true  },
    { "acPosVector2Symprod",     5479,   "spdbp",  true  },
    { "WWA2Symprod",             5480,   "spdbp",  true  },
    { "StnAscii2Symprod",        5481,   "spdbp",  true  },
    { "NcMdvServer",             5482,   "mdvp",  true  },
    { "Sigwx2Symprod",           5483,   "spdbp",  true  },
    { "GenPolyStats2Symprod",    5484,   "spdbp",  true  },
    { "LtgGroup2Symprod",        5485,   "spdbp",  true  },
    { "Taf2Symprod",             5486,   "spdbp",  true  },
    { "SunCal2Symprod",          5487,   "spdbp",  true  },
    { "DeTectGenPoly2Symprod",   5488,   "spdbp",  true  },
    { "Amdar2Symprod",           5489,   "spdbp",  true  },
    { "GenPtCircle2Symprod",     5490,   "spdbp",  true  },
    { "spare_mm",                5491,   "spdbp",  true  },
    { "spare_nn",                5492,   "spdbp",  true  },
    { "spare_oo",                5493,   "spdbp",  true  },
    { "spare_pp",                5494,   "spdbp",  true  },
    { "spare_qq",                5495,   "spdbp",  true  },
    { "spare_rr",                5496,   "spdbp",  true  },
    { "spare_ss",                5497,   "spdbp",  true  },
    { "spare_tt",                5498,   "spdbp",  true  },
    { "spare_uu",                5499,   "spdbp",  true  },
    { "spare_vv",                5500,   "spdbp",  true  },
  
    // Last entry must contain NULL members

    { NULL, 0, NULL, false}

  };

// endif __in_DsLocator_cc_file__
#endif

// endif _DSLOCATOR_INC_
#endif
