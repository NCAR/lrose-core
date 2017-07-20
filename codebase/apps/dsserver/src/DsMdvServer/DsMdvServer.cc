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
// DsMdvServer.cc
//
// Mdv Server Object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 1999
//
///////////////////////////////////////////////////////////////

#include "Params.hh"
#include "DsMdvServer.hh"

#include <toolsa/Path.hh>
#include <dsserver/DsLocator.hh>
#include <Mdv/DsMdvxMsg.hh>
#include <Mdv/climo/DailyByYearFileFinder.hh>
#include <Mdv/climo/DailyFileFinder.hh>
#include <Mdv/climo/ExternalDiurnalFileFinder.hh>
#include <Mdv/climo/HourlyByMonthFileFinder.hh>
#include <Mdv/climo/HourlyDiurnalFileFinder.hh>
#include <Mdv/climo/HourlyFileFinder.hh>
#include <Mdv/climo/MonthlyByYearFileFinder.hh>
#include <Mdv/climo/MonthlyFileFinder.hh>
#include <Mdv/climo/ThreeHourlyByMonthFileFinder.hh>
#include <Mdv/climo/ThreeHourlyDiurnalFileFinder.hh>

using namespace std;

DsMdvServer::DsMdvServer(string executableName,
                         const Params &params)
        : DsProcessServer(executableName,
                          params.instance,
                          params.port,
                          params.qmax,
                          params.max_clients,
                          params.debug >= Params::DEBUG_NORM,
                          params.debug >= Params::DEBUG_VERBOSE,
                          params.run_secure,
                          params.run_read_only,
                          params.allow_http),
          _paramsOrig(params),
          _climoFileFinder(0)

{
    setNoThreadDebug(params.no_threads);

    // Set up the DsDataFile class with this server's messaging state.
    DsDataFile::setDebug(_isDebug);
    DsDataFile::setVerbose(_isVerbose);

    if (_isDebug) {
      cerr << "debug on" << endl;
    }
    if (_isVerbose) {
      cerr << "verbose on" << endl;
    }

    _createClimoObjects();
}

DsMdvServer::~DsMdvServer()
{
}

// Handle data commands from the client.
//   Returning failure (-1) from this method makes the server die.
//   So don't ever do that.
// 
// virtual
int DsMdvServer::handleDataCommand(Socket * socket,
                                   const void * data, ssize_t dataSize)
{

  if (_isVerbose) {
    cerr << "Handling data command in DsMdvServer." << endl;
  }

  // peek at the message

  DsServerMsg msg;
  if (msg.decodeHeader(data, dataSize)) {
    string errMsg  = "Error in DsMdvServer::handleDataCommand(): ";
    errMsg += "Could not decode header.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM -DsMdvServer - " << statusString << endl;
    }
    return 0;
  }

  // disassemble the message, to make URL available for params

  if (msg.disassemble(data, dataSize)) {
    string errMsg  = "Error in DsMdvServer::handleDataCommand(): ";
    errMsg += "Could not disassemble message.";
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
      cerr << "ERROR - COMM -DsMdvServer - " << statusString << endl;
    }
    return 0;
  }

  // check mode and subtype for validity

  int subType = msg.getSubType();

  if (subType != DsMdvxMsg::MDVP_READ_ALL_HDRS &&
      subType != DsMdvxMsg::MDVP_READ_VOLUME &&
      subType != DsMdvxMsg::MDVP_READ_VSECTION &&
      subType != DsMdvxMsg::MDVP_WRITE_TO_DIR &&
      subType != DsMdvxMsg::MDVP_WRITE_TO_PATH &&
      subType != DsMdvxMsg::MDVP_COMPILE_TIME_LIST &&
      subType != DsMdvxMsg::MDVP_COMPILE_TIME_HEIGHT) {

    string errMsg  = "Error in DsMdvServer::handleDataCommand(): ";
    errMsg += "Unknown subType: ";
    char str[32];
    sprintf(str, "%d\n", msg.getSubType());
    errMsg += str;
    if (_isDebug) {
      cerr << errMsg << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errMsg, statusString, 10000)) {
	cerr << "ERROR - COMM -DsMdvServer - " << statusString << endl;
    }
    return 0;

  }

  // Check for local copy of params
  // Override initial params if params file exists in the datatype directory
  
  _params = _paramsOrig;
  
  if (_isDebug) {
    umsleep(20);
    cerr << "Checking for local params." << endl;
  }
  
  string errStr = "ERROR - DsMdvServer::handleDataCommand\n";
  if (_checkForLocalParams(msg, errStr)) {
    if (_isDebug) {
      cerr << errStr << endl;
    }
    // Send error reply to client.
    string statusString;
    if (DsProcessServer::sendReply(socket, DsServerMsg::BAD_MESSAGE,
				   errStr, statusString, 10000)) {
      cerr << "ERROR - COMM -DsMdvServer - " << statusString << endl;
    }
    return 0;
    
  } // if (checkForLocalParams( ...
  
  handleMdvxCommand(socket, data, dataSize);

  return 0;

}

//////////////////////////////////////////////////////
// check if we have local params

int DsMdvServer::_checkForLocalParams(DsServerMsg &msg,
                                      string errStr)

{

  // NOTE: the DsLocator will resolve the parameter file name 
  //       and modify the url

  bool paramsExist;
  _incomingUrl = msg.getFirstURLStr();
  if (_incomingUrl == "") {
    errStr += "No URL in message.\n";
    return -1;
  }
  DsURL url(_incomingUrl);
  if ( DsLocator.resolveParam( url, _executableName,
			       &paramsExist ) != 0 ) {
    errStr += "Cannot resolve parameter specification in url:\n";
    errStr += url.getURLStr();
    errStr += "\n";
    return -1;
  }
  
  //
  // The application-specfic code must load the override parameters
  //
  if (_isDebug) {
    cerr << "-->> Looking for params file: " << url.getParamFile() << endl;
  }
  if (paramsExist ) {
    if (_loadLocalParams(url.getParamFile()) != 0 ) {
      errStr += "Cannot load parameter file:\n";
      errStr += url.getParamFile();
      errStr += "\n";
      return( -1 );
    }
    if (_isDebug) {
      cerr << "-->> Found local params, file: " << url.getParamFile() << endl;
    }
  } else {
    // check for RHI dir, if found activate polar RHIs
    _checkForRhiDir(url.getFile());
  } // if ( paramsExist )
  
  return 0;

}
  
//////////////////////////////////////////////////////
// load params from local file

int DsMdvServer::_loadLocalParams(const string &paramFile)
{
  
  char **tdrpOverrideList = NULL;
  bool expandEnvVars = true;
  
  if (_isDebug) {
    cerr << "Loading new params from file: " << paramFile << endl;
  }

  if (_params.load(paramFile.c_str(), tdrpOverrideList,
                   expandEnvVars, false ) != 0 ) {
    return -1;
  }
  
  // climo

  if (_createClimoObjects() != 0) {
    return -1;
  }

  // debug

  if (_isVerbose) {
    _params.print(stderr, PRINT_SHORT);
  }
  
  return 0;

}

//////////////////////////////////////////////////////
// Check for RHI directory structure
// if found set polar RHI mode

void DsMdvServer::_checkForRhiDir(const string &dataDir)
{
  
  if (_isDebug) {
    cerr << "Checking for RHI data, dir: " << dataDir << endl;
  }

  // check for RHI directory structure

  Path dirPath(dataDir);
  string dirParent(dirPath.getDirectory());
  string dirName(dirPath.getFile());
  if (dirName.compare("sur") == 0 ||
      dirName.compare("sec") == 0 ||
      dirName.compare("SUR") == 0 ||
      dirName.compare("SEC") == 0) {

    string rhiDir = dirParent + PATH_DELIM;
    rhiDir += "rhi";
    if (ta_stat_exists(rhiDir.c_str())) {
      // rhi sub directory exists
      // set polar RHI mode on
      _params.serve_rhi_data = pTRUE;
      _params.polar_rhi = pTRUE;
      string rhiUrl = "mdvp:://localhost:0:" + rhiDir;
      TDRP_str_replace(&_params.rhi_url, rhiUrl.c_str());
    } else {
      rhiDir = dirParent + PATH_DELIM;
      rhiDir += "RHI";
      if (ta_stat_exists(rhiDir.c_str())) {
        // RHI sub directory exists
        // set polar RHI mode on
        _params.serve_rhi_data = pTRUE;
        _params.polar_rhi = pTRUE;
        string rhiUrl = "mdvp:://localhost:0:" + rhiDir;
        TDRP_str_replace(&_params.rhi_url, rhiUrl.c_str());
      }
    }
    
  }

  // debug
  
  if (_isDebug) {
    cerr << "  serve_rhi_data: " << _params.serve_rhi_data << endl;
    if (_params.serve_rhi_data) {
      cerr << "  polar_rhi: " << _params.polar_rhi << endl;
      cerr << "  rhi_url: " << _params.rhi_url << endl;
    }
  }
  
}

//////////////////////////////////////////////////////
// Create the climatology objects if applicable

int
DsMdvServer::_createClimoObjects()
{

  // Create the object for finding climo files times

  if (_params.use_climatology_url)
  {
    if (_isDebug)
      cerr << "*** use_climatology_url is true" << endl;
  
    delete _climoFileFinder;
    
    switch (_params.climatology_type)
    {
    case Params::DIURNAL_CLIMATOLOGY :
      {
      if (_isDebug)
	cerr << "Creating DIURNAL_CLIMATOLOGY file finder..." << endl;
      ExternalDiurnalFileFinder *temp_finder =
	new ExternalDiurnalFileFinder();
      if (!temp_finder->init(_params.climatology_dir))
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... DIURNAL_CLIMATOLOGY file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_HOURLY :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_HOURLY file finder..." << endl;
      HourlyFileFinder *temp_finder = new HourlyFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_HOURLY file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_HOURLY_BY_MONTH :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_HOURLY_BY_MONTH file finder..." << endl;
      HourlyByMonthFileFinder *temp_finder = new HourlyByMonthFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_HOURLY_BY_MONTH file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_3HOURLY_BY_MONTH :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_3HOURLY_BY_MONTH file finder..." << endl;
      ThreeHourlyByMonthFileFinder *temp_finder =
	new ThreeHourlyByMonthFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_3HOURLY_BY_MONTH file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_HOURLY_DIURNAL :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_HOURLY_DIURNAL file finder..." << endl;
      HourlyDiurnalFileFinder *temp_finder = new HourlyDiurnalFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_HOURLY_DIURNAL file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_3HOURLY_DIURNAL :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_3HOURLY_DIURNAL file finder..." << endl;
      ThreeHourlyDiurnalFileFinder *temp_finder =
	new ThreeHourlyDiurnalFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_3HOURLY_DIURNAL file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_DAILY :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_DAILY file finder..." << endl;
      DailyFileFinder *temp_finder = new DailyFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_DAILY file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_DAILY_BY_YEAR :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_DAILY_BY_YEAR file finder..." << endl;
      DailyByYearFileFinder *temp_finder = new DailyByYearFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_DAILY_BY_YEAR file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_MONTHLY :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_MONTHLY file finder..." << endl;
      MonthlyFileFinder *temp_finder = new MonthlyFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_MONTHLY file finder successfully created" << endl;
      break;
    }
    case Params::CLIMO_MONTHLY_BY_YEAR :
    {
      if (_isDebug)
	cerr << "Creating CLIMO_MONTHLY_BY_YEAR file finder..." << endl;
      MonthlyByYearFileFinder *temp_finder = new MonthlyByYearFileFinder();
      if (!temp_finder->init())
	return -1;
      _climoFileFinder = temp_finder;
      if (_isDebug)
	cerr << "  ... CLIMO_MONTHLY_BY_YEAR file finder successfully created" << endl;
      break;
    }
    
    } /* endswitch - _params.climatology_type */
  }

  return 0;
}
