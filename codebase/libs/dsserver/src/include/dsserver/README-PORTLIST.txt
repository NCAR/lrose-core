/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2010 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2010/10/7 23:12:56 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
# README-PORTLIST.txt 
#
# This was copied directly from: cvs/libs/dsserver/src/DsLocator/DsLocater.cc
#
# - F. Hage December 2005.
# ADD the Port Offset to DS_BASE_PORT to get actual port used by a service.
#
const char*   DsLOCATOR::LocalHostKeyword    = "localhost";
const size_t  DsLOCATOR::DS_BASE_PORT        = 5430;

// WELL-KNOWN PORT LOOKUP TABLE
//
// When adding a server, you can use the Spare slots, that way you do
// not force a relink of the applications
//
// NOTE: for non-translating servers (URL Translator == false)
//       there must be a one-to-one correspondance between the
//       executable name and the URL Protocol.
//

const DsLOCATOR::serverInfo_t
             DsLOCATOR::serverInfo[] = {
               //
               // Executable           Port      URL               URL
               //    Name             Offset   Protocol        Translator
               //
               { "servmap",              2,     "",             false },
               { "procmap",              3,     "",             false },
               { "DataMapper",           4,     "",             false },
               { "DsServerMgr",          5,     "",             false },
               { "DsMdvServer",         10,     "mdvp",         false },
               { "DsSpdbServer",        11,     "spdbp",        false },
               { "DsProxyServer",       12,     "proxyp",       false },
               { "DsFmqServer",         13,     "fmqp",         false },
               { "DsFileServer",        14,     "filep",        false },
               { "DsFCopyServer",       15,     "fcopyp",       false },
               { "DsTitanServer",       16,     "titanp",       false },
               { "DsLdataServer",       17,     "ldatap",       false },
               { "CSpareServer",        18,     "",             false },
               { "DSpareServer",        19,     "",             false },
               { "Ltg2Symprod",         20,     "spdbp",        true  },
               { "AcTrack2Symprod",     21,     "spdbp",        true  },
               { "Bdry2Symprod",        22,     "spdbp",        true  },
               { "Chunk2Symprod",       23,     "spdbp",        true  },
               { "FltPath2Symprod",     24,     "spdbp",        true  },
               { "Mad2Symprod",         25,     "spdbp",        true  },
               { "Metar2Symprod",       26,     "spdbp",        true  },
               { "Pirep2Symprod",       27,     "spdbp",        true  },
               { "PosnRpt2Symprod",     28,     "spdbp",        true  },
               { "Sigmet2Symprod",      29,     "spdbp",        true  },
               { "Tstorms2Symprod",     30,     "spdbp",        true  },
               { "TrecGauge2Symprod",   31,     "spdbp",        true  },
               { "Vergrid2Symprod",     32,     "spdbp",        true  },
               { "WxHazards2Symprod",   33,     "spdbp",        true  },
               { "BasinGenPt2Symprod",  34,     "spdbp",        true  },
               { "GenPt2Symprod",       35,     "spdbp",        true  },
               { "GenPtField2Symprod",  36,     "spdbp",        true  },
               { "Acars2Symprod",       37,     "spdbp",        true  },
               { "HydroStation2Symprod",38,     "spdbp",        true  },
               { "SigAirMet2Symprod",   39,     "spdbp",        true  },
               { "StormPolygon2Symprod",40,     "spdbp",        true  },
               { "WMS2MdvServer",       41,     "mdvp",         true  },
               { "GenPoly2Symprod",     42,     "spdbp",        true  },
               { "Rhi2Symprod",         43,     "spdbp",        true  },
               { "DsMdvClimoServer",    44,     "mdvp",         true  },
               { "Edr2Symprod",         45,     "spdbp",        true  },
               { "Sndg2Symprod",        46,     "spdbp",        true  },
               { "simpleAcTrack2Symprod", 47,   "spdbp",        true  },

               //
               // Last entry must contain NULL members
               //
               { 0, 0, 0, 0 }};


