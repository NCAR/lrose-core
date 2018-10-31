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
// DsMdvServer.hh
//
// Mdv Server Object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////

#ifndef DsMdvServerINCLUDED
#define DsMdvServerINCLUDED

#include <didss/DsDataFile.hh>
#include <dsserver/DsProcessServer.hh>
#include <Mdv/MdvxField.hh>
#include <string>
using namespace std;

class Grid;
class DsURL;
class MdvField;
class DsMdvMsg;
class MdvFile;
class DsMdvSocket;
class DsMdvx;
class ClimoFileFinder;

class DsMdvServer : public DsProcessServer {
  
public:

  static const string DataTypeString;
  
  DsMdvServer(string executableName, const Params &params);
  
  virtual ~DsMdvServer();

protected:
    virtual int handleDataCommand(Socket * socket,
                                  const void * data, ssize_t dataSize);
  
private:

  const Params &_paramsOrig;
  Params _params; // params used when serving clients
  string _incomingUrl;

  ClimoFileFinder *_climoFileFinder;
  
  // reading rhi azimuths

  class RhiAz {
  public:
    time_t time;
    double angle;
    RhiAz(time_t time_, double angle_) {
      time = time_;
      angle = angle_;
    }
  };
  Mdvx::master_header_t _rhiMhdr;
  
  // Private methods with no bodies. DO NOT USE!
  //
  DsMdvServer();
  DsMdvServer(const DsMdvServer & orig);
  DsMdvServer & operator = (const DsMdvServer & other);
  
  // Handle Mdvx data commands from the client.
  int handleMdvxCommand(Socket * socket,
                        const void * data, ssize_t dataSize);
  
  // load the local params
  
  int _checkForLocalParams( DsServerMsg &msg,
                            string errStr);
  
  int _loadLocalParams(const string &paramFile);
  
  void _checkForRhiDir(const string &paramsFile);

  int _createClimoObjects();
  
  // set up and follow up on reads for headers, vol and vsection
  
  time_t _setupRead(DsMdvx &mdvx, bool readVolume);
  void _adjustTimes(DsMdvx &mdvx, time_t searchTime);
  
  // wrappers for reading Mdvx volume and vsection
  
  int _readMdvxAllHeaders(DsMdvx &mdvx);
  int _readMdvxVolume(DsMdvx &mdvx);
  int _readMdvxVsection(DsMdvx &mdvx);
  int _readVsectionFromRhi(DsMdvx &mdvx);
  int _readRhi(DsMdvx &mdvx, time_t searchTime);
  int _readRhiAzimuths(time_t searchTime,
                       const string &rhiUrl,
                       vector<RhiAz> &azimuths);
  void _finalizeRhiWaypts(DsMdvx &mdvx, double az);
  int _computeRequestedRhiAz(DsMdvx &mdvx, double &az);
  int _compileMdvxTimeList(DsMdvx &mdvx, string &errorStr);
  int _compileMdvxTimeHeight(DsMdvx &mdvx, string &errorStr);
  int _writeMdvxToDir(DsMdvx &mdvx);
  int _writeMdvxToPath(DsMdvx &mdvx);
 
  void _printHeadersDebug(const DsMdvx &mdvx) const;

  // Override client settings on the mdvx object
  
  void _overrideClientSettings(DsMdvx &mdvx);
  
  // override info on read
  
  void _overrideMasterHeaderInfoOnRead(DsMdvx &mdvx);
  
  //  getting the url for climatology data
  
  string _getClimatologyUrl(DsMdvx &mdvx) const;
  
  //  getting the url if there are multiple domains
  
  string _getDomainUrl(double region_min_lat, double region_min_lon,
                       double region_max_lat, double region_max_lon);
  
  void _getUrlList(double region_min_lat, double region_min_lon,
                   double region_max_lat, double region_max_lon,
                   vector<string> &urlList);
  
  bool _withinDomain(double region_min_lat, double region_min_lon,
                     double region_max_lat, double region_max_lon,
                     double domain_min_lat, double domain_min_lon,
                     double domain_max_lat, double domain_max_lon);
  
  void _computeVsectionRegion(DsMdvx &mdvx,
                              double &region_min_lat,
                              double &region_min_lon,
                              double &region_max_lat,
                              double &region_max_lon);
  
  void _setBasicReadProperties(DsMdvx &mdvx);
  
  typedef enum {
    READ_VOLUME, READ_VSECTION
  } read_action_t;
  
  int _readDerivedAllHdrs(DsMdvx &mdvx);

  int _handleDerivedFields(DsMdvx &mdvx,
			   DsMdvServer::read_action_t action);
  
  MdvxField *_createDerivedField(Params::derived_field_t *derived,
				 const DsMdvx &base,
				 DsMdvServer::read_action_t action,
				 string &errStr);
  
  MdvxField *_createLinear(Params::derived_field_t *derived,
			   const DsMdvx &base,
			   string &errStr);

  MdvxField *_createSpeedFromUV(Params::derived_field_t *derived,
				const DsMdvx &base,
				string &errStr);

  MdvxField *_createDirnFromUV(Params::derived_field_t *derived,
			       const DsMdvx &base,
			       string &errStr);

  MdvxField *_createDiffFieldsSameFile(Params::derived_field_t *derived,
				       const DsMdvx &base,
				       string &errStr);

  MdvxField *_createDiffFields(Params::derived_field_t *derived,
			       const DsMdvx &base,
			       DsMdvServer::read_action_t action,
			       string &errStr);

  MdvxField *_createDiffInTime(Params::derived_field_t *derived,
			       const DsMdvx &base,
			       DsMdvServer::read_action_t action,
			       string &errStr);
  
  MdvxField *_createVertComposite(Params::derived_field_t *derived,
                                  const DsMdvx &base,
                                  DsMdvServer::read_action_t action,
                                  string &errStr);
  
  MdvxField *_createPseudoColorImage(Params::derived_field_t *derived,
                                     const DsMdvx &base,
                                     DsMdvServer::read_action_t action,
                                     string &errStr);
  
  bool _checkFieldGeometry(const Mdvx::field_header_t &fhdr1,
			   const Mdvx::field_header_t &fhdr2);
  
  int _compileDerivedTimeHeight(DsMdvx &mdvx);
  
  int _doCompileDerivedTimeHeight(DsMdvx &mdvx,
                                  vector<DsMdvx *> &vsects);
  
};

#endif
