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
 // SigAirMet2Spdb.cc
 //
 // SigAirMet2Spdb object
 // Additional functions for the object are found in the following
 // files: ModifyFirBoundary.cc, Points.cc, Utils.cc
 //
 // Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 //
 // Jan 2003
 //
 ///////////////////////////////////////////////////////////////

#include <dsserver/DmapAccess.hh>
#include <didss/DsURL.hh>

#include <string>
#include <vector>
#include <cerrno>
#include <toolsa/toolsa_macros.h>
#include <toolsa/globals.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <euclid/geometry.h>
#include <Spdb/DsSpdb.hh>
#include "SigAirMet2Spdb.hh"
#include "Input.hh"

// Constructor

SigAirMet2Spdb::SigAirMet2Spdb(int argc, char **argv)
  
{
  
  isOK = true;
  _inputPath = NULL;
  _usAirmetHdrInfo.is_update = false;

  // set programe name

  _progName = "SigAirMet2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params

  //  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
                           &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end times." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }

  // input path object

  if (_params.mode == Params::REALTIME) {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 _params.input_dir_path,
                                 _params.max_realtime_valid_age,
                                 PMU_auto_register);

  } else if (_params.mode == Params::ARCHIVE) {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 _params.input_dir_path,
                                 _args.startTime,
                                 _args.endTime);

  } else  if (_params.mode == Params::FILELIST) {

    _inputPath = new DsInputPath((char *) _progName.c_str(),
                                 _params.debug >= Params::DEBUG_VERBOSE,
                                 _args.inputFileList);

  }

  // read in station locations

  if (_stationLoc.ReadData(_params.st_location_path)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot load station locations, file : "
         << _params.st_location_path << endl;
    isOK = false;
    return;
  }

  // read in FIR locations

  if (_firLoc.ReadData(_params.fir_location_path)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot load FIR locations, file : "
         << _params.fir_location_path << endl;
    isOK = false;
    return;
  }

  // read in states list

  if (_readStates.ReadData(_params.us_states_location_path)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot load US States abbreviations, file : "
         << _params.us_states_location_path << endl;
    isOK = false;
    return;
  }

  // initialize direction strings
  // These are intentionally in order of specificity
  // (4 char, 3 char, then 2 char, then 1 char)
   
  _directions.push_back("NORTH");
  _directions.push_back("EAST");
  _directions.push_back("SOUTH");
  _directions.push_back("WEST");
  _directions.push_back("NNE");
  _directions.push_back("ENE");
  _directions.push_back("ESE");
  _directions.push_back("SSE");
  _directions.push_back("SSW");
  _directions.push_back("WSW");
  _directions.push_back("WNW");
  _directions.push_back("NNW");
  _directions.push_back("NE");
  _directions.push_back("SE");
  _directions.push_back("SW");
  _directions.push_back("NW");
  _directions.push_back("N");
  _directions.push_back("E");
  _directions.push_back("S");
  _directions.push_back("W");
   
  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

SigAirMet2Spdb::~SigAirMet2Spdb()

{

  // unregister process

  PMU_auto_unregister();

  // Free contained objects

  if (_inputPath) {
    delete _inputPath;
  }

}

//////////////////////////////////////////////////
// Run

int SigAirMet2Spdb::Run ()
{

  // register with procmap

  PMU_auto_register("Run");

  if (_params.mode == Params::ARCHIVE) {
    _inputPath->reset();
  }

  char *inputFilePath;
  while ((inputFilePath = _inputPath->next()) != NULL) {
    _fileTime = _inputPath->getDataTime(inputFilePath);
    _processFile(inputFilePath);
  } // while

  return (0);

}

//////////////////////////////////////////////////
// process file

int SigAirMet2Spdb::_processFile (const char *file_path)

{

  char procmap_string[256];
  Path path(file_path);
  sprintf(procmap_string, "Processing file <%s>", path.getFile().c_str());
  PMU_auto_register(procmap_string);

  // open file

  Input input(_progName, _params);
  if (input.open(file_path)) {
    cerr << "ERROR - SigAirMet2Spdb::_processFile" << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
    cerr << "  File time: " << DateTime::str(_fileTime) << endl;
  }
  
  DsSpdb asciiSpdb;
  DsSpdb transSpdb;
  string reportStr;
  int nTotal = 0;
  int nDecoded = 0;

  while (input.readNext(reportStr) == 0) {

    PMU_auto_register(procmap_string);
    
    if (reportStr.size() == 0) {
      continue;
    }
    nTotal++;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Got report =========================" << endl;
      cerr << reportStr << endl;
    }
  
    // split the report into messages

    vector<string> messages;
    _splitReport(reportStr, messages);

    for (int ii = 0; ii < (int) messages.size(); ii++) {
      if (_handleMessage(ii, messages[ii], asciiSpdb, transSpdb) == 0) {
        nDecoded++;
      }
    } // ii
    
  } // while

  // close file

  input.close();

  // debug print

  if ((_params.debug >= Params::DEBUG_VERBOSE) ||
      (_params.print_decode_problems_to_stderr)) {
    cerr << "---------------------------------------" << endl;
    cerr << "Done with file: " << file_path << endl;
    cerr << "  N messages found: " << nTotal << endl;
    cerr << "  N messages decoded: " << nDecoded << endl;
    cerr << "---------------------------------------" << endl;
  }
  
  // Write out
  
  int iret = 0;

  if (_params.store_ascii_format) {
    if (asciiSpdb.put(_params.ascii_output_url,
                      SPDB_ASCII_ID, SPDB_ASCII_LABEL)) {
      cerr << "ERROR - SigAirMet2Spdb::_processFile" << endl;
      cerr << asciiSpdb.getErrStr() << endl;
      iret = -1;
    }
  }

  if (_params.store_decoded_format) {
    if (transSpdb.put(_params.decoded_output_url,
                      SPDB_SIGAIRMET_ID, SPDB_SIGAIRMET_LABEL)) {
      cerr << "ERROR - SigAirMet2Spdb::_processFile" << endl;
      cerr << transSpdb.getErrStr() << endl;
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// split report into messages

void SigAirMet2Spdb::_splitReport(const string &reportStr,
                                  vector<string> &messages)

{

  messages.clear();

  // search for more than one '='
  // if there are not at least two '=' characters, then we
  // have only a single message in the report

  size_t firstPos = reportStr.find('=', 0);
  size_t nextPos = reportStr.find('=', firstPos + 1);
 
  // test for "="
  while (nextPos == firstPos + 1) {
    firstPos = nextPos;
    nextPos = reportStr.find('=', firstPos + 1);
  }

  // did we get 2 '=' separated by at least one position

  if (firstPos == string::npos || nextPos == string::npos) {
    // check for multiple TC reports
    if (_splitOnMultipleTc(reportStr, messages)) {
      messages.push_back(reportStr);
    }
    return;
  }

  // tokenize the report string
  
  vector<string> toks;
  _tokenize(reportStr, " \n\t\r", toks);

  // look for a 4-char station name followed by a '-'
  // this is the signed group

  int signedTokNum = -1;
  for (int ii = 0; ii < (int) toks.size(); ii++) {
    const string &tok = toks[ii];
    if (tok.size() == 5) {
      if (tok[4] == '-') {
        signedTokNum = ii;
        break;
      }
    }
  }

  if (signedTokNum < 0) {
    // did not find signed group
    messages.push_back(reportStr);
    return;
  }

  // found signed group

  string signedTok = toks[signedTokNum];

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Found multiple messages in report" << endl;
    cerr << "  Signed tok: " << signedTok << endl;
  }

  // locate the signed group in the original string

  size_t signedPos = reportStr.find(signedTok, 0);
  if (signedPos == string::npos) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - Could not find signed tok again" << endl;
      messages.push_back(reportStr);
      return;
    }
  }

  // load up the upper part of message

  size_t startOfSubMessages = signedPos + signedTok.size();
  string upperPart = reportStr.substr(0, startOfSubMessages);

  // now find all sub parts delimited by the '=' characters

  size_t prevPos = startOfSubMessages;
  while (true) {
    // find next '='
    size_t pos = reportStr.find('=', prevPos);
    if (pos == string::npos) {
      break;
    }
    // move past the '=';
    pos++;
    int len = pos - prevPos;
    if (len < 5) {
      break;
    }
    string lowerPart = reportStr.substr(prevPos, len);
    string combined = upperPart;
    combined += "\n";
    combined += lowerPart;
    combined += "\n";
    messages.push_back(combined);
    prevPos = pos;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "****************************" << endl;
      cerr << "Creating message from parts:" << endl;
      cerr << combined << endl;
      cerr << "****************************" << endl;
    }
  } // while
 
}

////////////////////////////////////////////////////////////
// split report into messages, based on
// multiple TC messages
//
// Returns 0 on success (multiple TC's found), -1 otherwise

int SigAirMet2Spdb::_splitOnMultipleTc(const string &reportStr,
                                       vector<string> &messages)
  
{
  
  // tokenize the report string
  
  vector<string> toks;
  _tokenize(reportStr, " \n\t\r", toks);

  // look for start toks of multiple TC observations

  string tcStr = "TC";
  string obsStr = "OBS";
  vector<int> startToks;
  for (int ii = 0; ii < (int) toks.size() - 5; ii++) {
    if (toks[ii] == tcStr) {
      if (toks[ii + 1] == obsStr ||
          toks[ii + 2] == obsStr ||
          toks[ii + 3] == obsStr ||
          toks[ii + 4] == obsStr) {
        startToks.push_back(ii);
      }
    }
  }

  if (startToks.size() < 2) {
    // less than 2 TCs found, do not split message
    return -1;
  }

  // find the start locations of the multiple TCs

  vector<int> startLocs;

  int count = 0;
  size_t prevPos = 0;
  while (true) {

    // find a TC entry
    size_t pos = reportStr.find("TC", prevPos);
    if (pos == string::npos) {
      break;
    }

    // check for white space following
    
    int nextChar = reportStr[pos + 2];
    if (!isspace(nextChar)) {
      continue;
    }

    // check for following tok
    string followingTok = toks[startToks[count] + 1];
    size_t followingPos = reportStr.find(followingTok, pos + 3);
    if (followingPos == string::npos) {
      break;
    }

    if (followingPos - pos <= 16) {
      // we have the right location
      startLocs.push_back(pos);
      count++;
      if (startLocs.size() == startToks.size()) {
        break;
      }
    }

    prevPos = pos + 3;
    
  } // while (pos != string::npos)

  if (startLocs.size() != startToks.size()) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - splitOnMultipleTc" << endl;
      cerr << "  Could not split properly" << endl;
      cerr << "report: " << endl;
      cerr << reportStr << endl;
    }
    return -1;
  }
    
  // upper part is text up until the first start pos

  string upperPart = reportStr.substr(0, startLocs[0]);

  // subsequent parts

  for (int ii = 0; ii < (int) startLocs.size(); ii++) {

    int len;
    if (ii == (int) startLocs.size() - 1) {
      len = reportStr.size() - startLocs[ii];
    } else {
      len = startLocs[ii + 1] - startLocs[ii];
    }

    string lowerPart = reportStr.substr(startLocs[ii], len);

    string combined = upperPart;
    combined += "\n";
    combined += lowerPart;
    combined += "\n";
    messages.push_back(combined);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "*******************************" << endl;
      cerr << "Creating TC message from parts:" << endl;
      cerr << "-------------------------------" << endl;
      cerr << combined << endl;
      cerr << "-------------------------------" << endl;
    }

  } // ii

  return 0;

}

//////////////////////////////////////////////////
// handle message
//
// Returns 0 if decoded, -1 if not

int SigAirMet2Spdb::_handleMessage(int messageNum,
                                   const string &messageStr,
                                   DsSpdb &asciiSpdb,
                                   DsSpdb &transSpdb)
  
{

  // tokenize message
  
  _tokenize(messageStr, " \n\t\r", _msgToks);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Handling message ----------->>" << endl;
    cerr << "  " << messageStr << endl;
    cerr << "  ntoks: " << _msgToks.size() << endl;
  }
  
  // if no toks, continue
  
  if (_msgToks.size() == 0) {
    return -1;
  }
  
  // set up used array, initialize to false
  
  _used.clear();
  for (size_t ii = 0; ii < _msgToks.size(); ii++) {
    _used.push_back(false);
  }
  
  // Print a break at the start of each SIG/AIRMET
  
  if (_params.print_decode_problems_to_stderr) {
    cerr << "--------------------------------------" << endl;
  }
  
  // decode
  
  _doCancel = false;
  _decoded.clear();
  _decoded.setWx("UNKNOWN");
  _decoded.setFir("UNKNOWN");
  decode_return_t ableToDecode=_decodeMsg(messageStr);
  
  // print debug message if unable to decode, then go get next
  // sig/airmet
  
  if (ableToDecode == DECODE_FAILURE) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - SigAirMet2Spdb::_handleMessage" << endl;
      cerr << "  Cannot decode sigmet/airmet report" << endl;
      cerr << "  Report: --------------->>>>>" << endl;
      cerr << messageStr << endl;
    }
    return -1;
  }
  
  // print debug message if able to decode type but we do not
  // want to save to SPDB database. Do not increment nDecoded
  // before get next sig/airmet
  
  if (ableToDecode == DECODE_TYPE_NO_WANT) {
    if (_params.print_decode_problems_to_stderr) {
      cerr << "Skip this message. Not wanted type." << endl;
    }
    return -1;
  }
  
  // print debug message if able to decode but do not want to
  // add to SPDB database. Increment nDecoded before go get next
  // sig/airmet
  
  if (ableToDecode == DECODE_MSG_NO_SAVE) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - SigAirMet2Spdb::_handleMessage" << endl;
      cerr << "  Will not add sigmet/airmet report to SPDB databases" << endl;
      cerr << "  Report: ---------------->>>>>" << endl;
      cerr << messageStr << endl;
    }
    return 0;
  }
  
  // We successfully decoded the sig/airmet message.
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << ">>>> Decoded report:" << endl;
    _decoded.print(cerr, "    ");
  }
  
  // compute data_type2 string
  
  const string &id = _decoded.getId();
  string dataType2 = id;
  if (messageNum == 0) {
    // first message in report, business as usual
    if (id.size() < 4 && isdigit(id[0])) {
      if (_decoded.getGroup() == SIGMET_GROUP) {
        dataType2 = "S" + id;
      } else {
        dataType2 = "A" + id;
      }
    }
  } else {
    // multiple messages in report, set data_type_2 accordingly
    if (_decoded.getGroup() == SIGMET_GROUP) {
      dataType2 = "s";
    } else {
      dataType2 = "a";
    }
    char num[16];
    sprintf(num, "%.3d", messageNum);
    dataType2 += num;
  }
  
  // add to spdb objects as appropriate
  
  si32 data_type = Spdb::hash4CharsToInt32(_decoded.getSource());
  si32 data_type2 = Spdb::hash4CharsToInt32(dataType2.c_str());
  
  if (_params.store_ascii_format) {
    asciiSpdb.addPutChunk(data_type,
                          _decoded.getStartTime(),
                          _decoded.getEndTime(),
                          messageStr.size() + 1,
                          messageStr.c_str(),
                          data_type2);
  }
  
  if (_params.store_decoded_format) {
    _decoded.assemble();
    transSpdb.addPutChunk(data_type,
                          _decoded.getStartTime(),
                          _decoded.getEndTime(),
                          _decoded.getBufLen(),
                          _decoded.getBufPtr(),
                          data_type2);
  }

  return 0;

}

///////////////////////////////////////////////
// decode the SIGMET string
// Return 0 on success, -1 on error, or 1 if
//   can decode the string but do NOT want to add
//   to the SPDB database (e.g., NULL SIGMETS)
//

decode_return_t SigAirMet2Spdb::_decodeMsg(const string &messageStr)

{

  // find the SIGMET or AIRMET token

  bool foundStart = false;
  bool foundWanted = false;
  string foundType = "UNKNOWN";
  _startTokNum = 0;
  _tcFound = false;
  _centroidFromObsStation = false;
   
  for (size_t ii = 0; ii < _msgToks.size(); ii++) {
    const string &tok = _msgToks[ii];

    if (tok == "SIGMET") {
      foundStart = true;
      foundType=tok;
      if (_params.decode_sigmets) {
        _decoded.setGroup(SIGMET_GROUP);
        foundWanted=true;
      }
    } else if (tok == "AIRMET") {
      foundStart = true;
      foundType=tok;
      if (_params.decode_airmets) {
        _decoded.setGroup(AIRMET_GROUP);
        foundWanted=true;
      }
    }
    if (foundStart) {
      _startTokNum = ii;
      _lastTokUsed = _startTokNum + 1;
      break;
    }
     
    // NIL sigmets. Special case. Cannot decode these SIGMET...NONE
    // messages due to a lack of an ID. If uncomment the isNil(),
    // the messages will appear in the stats as decoded but ignored.
    // If comment out the isNil(), the messages will appear in the
    // stats as failed to decode due to lack of ID

    if ((tok == "SIGMET...NONE") && (_params.decode_sigmets)) {
      //       _startTokNum = ii-1;
      //       _decoded.setGroup(SIGMET_GROUP);
      //       if (_isNil(_startTokNum+1, _msgToks)) {
      //	 if (_params.print_decode_problems_to_stderr) {
      //	   cerr << "This is a NIL SIGMET" << endl;
      //	 }
      //	 return 1;
      //       }
      foundStart = true;
      foundType="SIGMET";
      foundWanted = true;
      _startTokNum=ii;
      break;
    }
  }

  if (!foundStart) {
    if (_params.debug) {
      cerr << "ERROR - SigAirMet2Spdb::_decode" << endl;
      cerr << "  Cannot find SIGMET or AIRMET keywords" << endl;
      cerr << "  Text:" << endl;
      cerr << messageStr << endl;
    }
    return DECODE_FAILURE;
  }

  if (!foundWanted) {
    if (_params.debug) {
      cerr << "Found message type: " << foundType << ", not wanted" << endl;
    }
    return DECODE_TYPE_NO_WANT;
  }

  if ((_params.debug) || (_params.print_decode_problems_to_stderr)) {
    cerr << "Type: " << foundType << endl;
  }

  for (size_t ii = 0; ii <= _startTokNum; ii++) {
    _used[ii] = true;
  }

  // Is this a test message?

  if (_isTestMessage(_startTokNum, _msgToks, 2)) {
    return DECODE_MSG_NO_SAVE;
  }

  // Is this a US AIRMET type? If so, the ID is handled differently

  _setUSAirmetID(_msgToks, _startTokNum);
  if (_params.debug) {
    if (_isUSAirmet) {
      cerr << "Is US Airmet" << endl;
    }
  }

  if (!_isUSAirmet) {

    // set the id and qualifier

    if (_setId() ) {
      if ((_params.debug) || (_params.print_decode_problems_to_stderr)) {
        cerr << "ERROR - SigAirMet2Spdb::_decode" << endl;
        cerr << "  Cannot find ID" << endl;
      }
      return DECODE_FAILURE;
    }
  }

  // find and set the source

  _setSource();

  // find and set the times

  bool foundTimes = false;
  if (_setTimes() == 0) {
    foundTimes = true;
  }

  // Is this an amend/correction or reissue message? This needs to be
  // done after the times are set and before the cancel is handled

  // These cause a core dump, and do nothing.
  // So commented out for now - Mi
  // _setAmend();
  // _setReissue();

  // If this is a AIRMET header, return after the times are set
  // but do not want to save the header as an AIRMET itself

  if (_isUSAirmetHeader) {
    return DECODE_MSG_NO_SAVE;
  }
     
  // find cancel flag as appropriate. This needs to be done AFTER
  // the times are set.

  if (_isUSAirmet && _usAirmetHdrInfo.is_update) {
    _USAirmetCheckCancel();
  } else {
    _checkCancel(0, _msgToks);
  }

  if (_doCancel) {
    if (_params.print_decode_problems_to_stderr) {
      cerr << "Found a CANCEL w/ " <<  _msgToks.size() << " tokens." << endl;
    }
    if (_params.store_decoded_format) {
      // overwrite entry in data base
      _overwriteCancelled();
    }
    if ((_params.token_threshold_for_cancel < 0) || 
	(_msgToks.size() < _params.token_threshold_for_cancel))
    return DECODE_MSG_NO_SAVE;
  }

  // check for times, except for cancel

  if (!foundTimes) {
    if ((_params.debug) || (_params.print_decode_problems_to_stderr)) {
      cerr << "ERROR - SigAirMet2Spdb::_decode" << endl;
      cerr << "  Cannot find times or cancel" << endl;
    }
    return DECODE_FAILURE;
  }

  // find and set the wx string
  
  _setWx();

  // find the OBS and/or FCST flag

  _setObsOrFcast();

  // try to find a location

  _setCentroidFromStationId();

  // find and set the flight levels if possible

  _setFlightLevels();

  // is this a NIL (empty) sigmet? Will not write to SPDB database

  string wxStr=_decoded.getWx();
  if ((wxStr.find("UNKNOWN", 0) != string::npos) &&
      (!_decoded.flightLevelsSet())) {
    if (_isNil(_startTokNum+1, _msgToks)) {
      if (_params.print_decode_problems_to_stderr) {
        cerr << "This is a NIL SIGMET" << endl;
      }
      return DECODE_MSG_NO_SAVE;
    }
  }

  // find and set the FIR if possible

  bool foundFir=_setFir();

  // is the MWO set? strictness check and remove it from the search

  size_t mwoIdx;
  bool foundMwo=_hasMWO(mwoIdx);

  // strictness checks

  _hasFirCtaOrAircraft(foundFir, foundMwo, mwoIdx);

  // set vertices

  if (!_setVertices()) {
    if (_params.print_decode_problems_to_stderr) {
      cerr << "Cannot decode vertices" << endl;
      cerr << messageStr << endl;
    }
  }

  // set movement

  _setMovement();

  // set intensity change

  _setIntensity();

  // set forecast position
   
  _setFcastPosition();

  // set outlook position(s)

  _setOutlookPosition();
   
  // set text from unused tokens

  _setText(messageStr, true);

  return DECODE_SUCCESS;

}

////////////////////////////////////////////////
// find and set the id and qualifier
//
// Returns 0 on success, -1 on failure

int SigAirMet2Spdb::_setId()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setId()" << endl;
  }

  // Try the first 2 positions after the SIGMET/AIRMET token

  string id, qualifier;
  bool foundId, foundQualifier;
  vector <size_t> usedIdxs;
  if (_findIdAndQualifier(_startTokNum+1, _msgToks,
                          id, qualifier,
                          foundId, foundQualifier,
                          usedIdxs)) {
    if (foundId) {
      _decoded.setId(id);
    }
    if (foundQualifier) {
      _decoded.setQualifier(qualifier);
    }

    // set the used array
    for (size_t ii=0; ii<usedIdxs.size(); ii++) {
      size_t idx=usedIdxs[ii];
      _used[idx] = true;
    }

    // debug

    if (_params.print_decode_problems_to_stderr) {
      if (!foundQualifier) {
        cerr << "ID: " << id << endl;
      } else {
        cerr << "ID: " << _decoded.getId()
             << " Qualifier: " << _decoded.getQualifier() << endl;
      }
    }

  } else {
    // cannot find ID - invalid report
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// handle an amendment or correction. Get the ID of the
// sig/airmet to be amended or corrected
//

void SigAirMet2Spdb::_setAmend()
  
{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setAmend()" << endl;
  }

  // Need to start the search at idx 0 rather than _startTokNum
  // because occasionally AMEND or CORRECT is a modifier to the
  // initial SIG/AIRMET token
  
  string amendId, amendQualifier;
  size_t startIdx=0;
  if (_isAmendMessage(startIdx, _msgToks, amendId, amendQualifier)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "This is an amend/correction: ID: " << amendId;
      if (amendQualifier != "NULL") {
	cerr << ", qualifier: " << amendQualifier;
      }
      cerr << endl;
    }
  }
}

////////////////////////////////////////////////
// handle a reissue. Get the ID of the
// sig/airmet to be reissued.
//

void SigAirMet2Spdb::_setReissue()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setReissue()" << endl;
  }

  string reissueId, reissueQualifier;
  if (_isReissueMessage(_startTokNum, _msgToks,
			reissueId, reissueQualifier)) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "This is an reissue: ID: " << reissueId;
      if (reissueQualifier != "NULL") {
	cerr << ", qualifier: " << reissueQualifier;
      }
      cerr << endl;
    }
  }
}

////////////////////////////////////////////////
// set the weather type
// returns 0 on success, -1 on failure

int SigAirMet2Spdb::_setWx()

{
   
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setWx()" << endl;
  }

  // Skip types for US AIRMETS. This is set in _setUSAirmetID()
  
  if (_isUSAirmet) {
    return -1;
  }

  // start search just beyond the SIGMET/AIRMET location, but
  // need to check if US so skip header (and do not find VA the
  // state as VA volcanic ash!)
  
  size_t markTok = _startTokNum + 1;
  _mayBeUS=_skipUSHeader(_msgToks, markTok, _msgToks.size(), markTok);

  // search through tokens looking for match with possible wx types
  
  for (size_t ii = markTok; ii < _msgToks.size(); ii++) {

    string wxType;
    if (_getWxType(ii, wxType) == 0) {

      // if there was any structure to this program, this wouldn't be such a
      // hack. We are just stuck with it.

      // Amendment 77 has added surface winds and surface visiblity to the AIRMET.
      // This token can include details about the observed weather, so more
      // token parsing is needed to capture the full weather type.

      if ((wxType == "SFC WIND") || (wxType == "SFC VIS")) {
	_handleSfcWindAndVis(ii, wxType);
      }
      
      _decoded.setWx(wxType);
      if (wxType.find("TC", 0) != string::npos) {
        _tcFound = true;
      }
      if (_params.print_decode_problems_to_stderr) {
        cerr << "Weather: " << wxType << endl;
      }
      return 0;
    }

  } // ii

  if (_params.print_decode_problems_to_stderr) {
    cerr << "Cannot match wx string" << endl;
  }

  _decoded.setWx("UNKNOWN");

  return -1;

}

////////////////////////////////////////////////
// get the weather type if we can
//
// Weather string may be split up, so tokens
// are searched for in order
//
// Returns 0 on success, -1 on error

int SigAirMet2Spdb::_getWxType(int startPos, string &wxType)

{
   
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setWxType()" << endl;
  }
  
  // look through wx types - the long strings are first, so
  // that the more well-defined ones will be found first
  
  for (int ii = 0; ii < _params.wx_translator_n; ii++) {
    
    // get and tokenize the weather string
    
    string msgWxStr = _params._wx_translator[ii].message_text;
    vector<string> wxToks;
    _tokenize(msgWxStr, " \n\t\r", wxToks);
    
    // loop through the tokens in the wx string, checking for a match
    // in each token
    
    bool match = true;
    int pos = startPos;
    for (int jj = 0; jj < (int) wxToks.size(); jj++, pos++) {
      int pos = startPos + jj;
      if (pos >= (int) _msgToks.size()) {
        match = false;
        break;
      }
      if (wxToks[jj] != _msgToks[pos]) {
        match = false;
        break;
      }
    }

    // if match found, success

    if (match) {
      wxType = _params._wx_translator[ii].standard_text;
      _lastTokUsed = startPos + wxToks.size() - 1;
     return 0;
    }

  } // ii

  // failure

  return -1;

}

////////////////////////////////////////////////
// find and set the source station name
//

void SigAirMet2Spdb::_setSource()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setSource()" << endl;
  }

  bool confirmedFound = false;
  string possibleSource, confirmedSource;
  for (size_t ii = 0; ii < _startTokNum; ii++) {
    const string &tok = _msgToks[ii];

    if ((tok.size() == 4) && (tok != "CNCL")) {
      // possible ICAO identifier - check if it is in the list
      double lat, lon, alt;
      string type;
      if (_stationLoc.FindPosition(tok, lat, lon, alt, type) == 0) {
        confirmedSource = tok;
        confirmedFound = true;
        _decoded.setCentroid(lat, lon);
        _decoded.setSource(confirmedSource);
      } else {
        if (_allAlpha(tok)) {
          possibleSource = tok;
          _decoded.setSource(possibleSource);
        }
      }
    }
  }

  if (_params.print_decode_problems_to_stderr) {
    cerr << "Source: " << _decoded.getSource() << endl;
  }

  if ((!confirmedFound) && ((_params.debug) ||
                            (_params.print_decode_problems_to_stderr))) {
    cerr << "   setSource: cannot confirm "
         << _decoded.getSource() << " in st_location_path, will use anyway"
         << endl;
  }

}

////////////////////////////////////////////////
// find and set the times
//

int SigAirMet2Spdb::_setTimes()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setTimes()" << endl;
  }
   
  // set the issue time
   
  time_t issueTime = _fileTime;
  for (size_t ii = 0; ii < _startTokNum; ii++) {
    const string &tok = _msgToks[ii];
    if (_computeIssueTime(tok, issueTime) == 0) {
      break;
    }
  }
   
  // correct issue time for closeness to file time, and store

  _correctTimeForFileTime(issueTime);
  _decoded.setIssueTime(issueTime);

  // set the start and end times

  time_t startTime, endTime;
  if (_findStartEndTimes(startTime, endTime)) {
    // check for US airmet
    if (_isUSAirmet && !_isUSAirmetHeader &&
        (_usAirmetHdrInfo.start_time > 0) &&
        (_usAirmetHdrInfo.end_time > 0)) {
      startTime = _usAirmetHdrInfo.start_time;
      endTime = _usAirmetHdrInfo.end_time;
      if (_params.debug) {
        cerr << "setTimes: using header times, start: "
             << utimstr(startTime) << ", end: "
             << utimstr(endTime) << endl;
      }
    } else {
      return -1;
    }
  }
  
  // If start time is later than end time, set both times
  // to the issue time
  
  if (startTime > endTime) {
    if (_params.debug) {
      cerr << "ERROR - start is greater than end time" << endl;
      cerr << "  Start time: " << utimstr(startTime) << endl;
      cerr << "  End time: " << utimstr(endTime) << endl;
      cerr << "  Setting start and end times to issue time." << endl;
    }
    startTime = issueTime;
    endTime = issueTime;
  }

  // if duration is excessive, set both times to issue time

  double maxDuration = _params.sigmet_max_duration;
  if (_decoded.getGroup() == AIRMET_GROUP) {
    maxDuration = _params.airmet_max_duration;
  }
  double duration = (double) endTime - (double) startTime;
  if (duration > maxDuration) {
    if (_params.debug) {
      cerr << "ERROR - duration exceeds secs: "
           << maxDuration << endl;
      cerr << "  Setting start and end times to issue time." << endl;
    }
    startTime = issueTime;
    endTime = issueTime;
  }

  // if start time is more than max_duration after the issue time,
  // set both start and end times to issue time

  double diff = (double) startTime - (double) issueTime;
  if (fabs(diff) > maxDuration) {
    if (_params.debug) {
      cerr << "ERROR - start time (" <<  startTime << ") differs from issue time by more than secs: "
           << maxDuration << endl;
      cerr << "  Setting start and end times to issue time." << endl;
    }
    startTime = issueTime;
    endTime = issueTime;
  }

  // If a US AIRMET header, set the start and end times

  if (_isUSAirmet) {
    _usAirmetHdrInfo.start_time = startTime;
    _usAirmetHdrInfo.end_time = endTime;
  }

  _decoded.setStartTime(startTime);
  _decoded.setEndTime(endTime);

  // Debug
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "setTimes: start: " << utimstr(startTime)
         << ", end: " << utimstr(endTime) << endl;
  }

  return 0;

}


////////////////////////////////////////////////
// find and set whether this is OBS, FCST or both
//

void SigAirMet2Spdb::_setObsOrFcast()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setObsOrFcast()" << endl;
  }

  // 1=obs, 2=fcst, 3=obs and fcst

  int obsfcst=-1;
  bool foundObs=false;
  bool foundFcst=false;
  bool foundAndFcst=false;

  for (size_t ii = _startTokNum + 1; ii < _msgToks.size(); ii++) {

    // look for OBS

    const string &tok = _msgToks[ii];
    if (tok == "OBS") {

      foundObs=true;
      _lastTokUsed = ii;

      // try to set OBS time
      
      if (ii < _msgToks.size() - 2) {
        const string &atStr = _msgToks[ii+1];
        const string &timeStr = _msgToks[ii+2];
        if (atStr == "AT") {
          if (_setObsTime(timeStr) == 0) {
            _lastTokUsed += 2;
          }
        }
      }

    } // if (tok == "OBS")

    // look for FCST

    if (tok == "FCST") {
      foundFcst=true;
      _lastTokUsed = ii;
      
      // try to set FCST time (new standard Nov 2010)
      if (ii < _msgToks.size() - 2) {
        const string &atStr = _msgToks[ii+1];
        const string &timeStr = _msgToks[ii+2];
        if (atStr == "AT") {
          if (_setFcastTime(timeStr) == 0) {
            _lastTokUsed += 2;
          }
        }
      }
     

      // ICAO standard expects AND FCST if both obs and fcast

      const string &prevTok = _msgToks[ii-1];
      if (prevTok == "AND") {
        foundAndFcst=true;
      }
    }

  } // ii

  if (foundObs && !foundFcst) {
    obsfcst=1;
  }
  else if (!foundObs && foundFcst) {
    obsfcst=2;
  }
  else if (foundObs && foundFcst) {
    obsfcst=3;
  }

  // Decode info

  if (_params.print_decode_problems_to_stderr) {
    cerr << "OBS and/or FCST: OBS: " <<
      foundObs << ", FCST: " << foundFcst << endl;
  }

  // ICAO special check

  if (_params.print_decode_problems_to_stderr) {
    if ((!foundObs) && (!foundFcst)) {
      cerr << "   Strictness info - neither FCST nor OBS found" << endl;
    }
    if ((foundObs) && (foundFcst) && (!foundAndFcst)) {
      cerr << "   Strictness info" << endl;
      cerr <<"      includes both FCST and OBS but not AND FCST" << endl;
    }
  }
}

////////////////////////////////////////////////
// set observed time if possible
//

int SigAirMet2Spdb::_setObsTime(const string &timeStr)

{

  int hour, min;
  if (sscanf(timeStr.c_str(), "%2d%2d", &hour, &min) != 2) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Could not set observed time from str: " << timeStr << endl;
    }
    return -1;
  }

  // fill out based on the issue time
  
  time_t issueTime = _decoded.getIssueTime();
  DateTime obsTime(issueTime);
  obsTime.setHour(hour);
  obsTime.setMin(min);
  time_t obsUtime = obsTime.utime();
  
  // correct for midnight passage
  double diff = (double) obsUtime - (double) issueTime;
  if (diff > 43200) {
    obsUtime -= 86400;
  } else if (diff < -43200) {
    obsUtime += 86400;
  }

  _decoded.setObsTime(obsUtime);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "set observed time: " << DateTime::strm(obsUtime) << endl;
  }

  return 0;

}
////////////////////////////////////////////////
// set "FCAST AT" time if possible 
// note: This is the FCAST time for the valid time of the 'met
//	 not the FCAST times given at the end of the 'met for 
// 	 the end of the valid period.

int SigAirMet2Spdb::_setFcastTime(const string &timeStr)

{

  int hour, min;
  if (sscanf(timeStr.c_str(), "%2d%2d", &hour, &min) != 2) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Could not set forecast time from str: " << timeStr << endl;
    }
    return -1;
  }

  // fill out based on the issue time
  
  time_t issueTime = _decoded.getIssueTime();
  DateTime fcastTime(issueTime);
  fcastTime.setHour(hour);
  fcastTime.setMin(min);
  time_t fcastUtime = fcastTime.utime();
  
  // correct for midnight passage
  double diff = (double) fcastUtime - (double) issueTime;
  if (diff > 43200) {
    fcastUtime -= 86400;
  } else if (diff < -43200) {
    fcastUtime += 86400;
  }

  _decoded.setFcastTime(fcastUtime);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "set fcast time: " << DateTime::strm(fcastUtime) << endl;
  }

  return 0;

}

////////////////////////////////////////////////
// find and set the location if available
//

void SigAirMet2Spdb::_setCentroidFromStationId()

{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setCentroidFromStationId()" << endl;
  }

  size_t endTokNum = _lastTokUsed + 6;
  if (endTokNum > _msgToks.size()) {
    endTokNum = _msgToks.size();
  }
  
  for (size_t ii = _lastTokUsed; ii < endTokNum; ii++) {
    
    const string &tok = _msgToks[ii];
    if (tok.size() == 4) {

      // possible ICAO identifier - check if it is in the list
      double lat, lon, alt;
      string type;
      if (_stationLoc.FindPosition(tok, lat, lon, alt, type) == 0) {
        _decoded.setCentroid(lat, lon);
        _lastTokUsed = ii;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Setting centroid from station identifier: " << tok << endl;
          cerr << "Lat,lon: " << lat << ", " << lon << endl;
        }
        _centroidFromObsStation = true;
        return;
      }

    }

  } // ii

}

////////////////////////////////////////////////
// find and set levels
//

void SigAirMet2Spdb::_setFlightLevels()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setFlightLevels()" << endl;
  }

  // Set defaults

  vector <size_t> usedTokIdxs;
  bool foundTop;
  bool foundBot;
  int bot;
  int top;
  
  int success=_findFlightLevels(_msgToks, _used, _startTokNum+1,
                                _msgToks.size(), foundTop, foundBot,
                                bot, top, usedTokIdxs);

  if (bot > top && top > 0) {
    int temp = bot;
    bot = top;
    top = temp;
  }

  // Set the flight levels

  if (success >= 0) {

    _decoded.setFlightLevels(bot, top);

    // Set the used tokens

    for (size_t ii=0; ii<usedTokIdxs.size(); ii++) {
      size_t idx=usedTokIdxs[ii];
      _used[idx]=true;
    }
  }
  else {
    if (_params.print_decode_problems_to_stderr) {
      cerr << "Cannot find or decode flight levels" << endl;
    }
  }

  return;
}


////////////////////////////////////////////////
// find and set the FIR
//
//

bool SigAirMet2Spdb::_setFir()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setFir()" << endl;
  }

  // Set default

  bool foundFir=false;

  // start search at the SIGMET/AIRMET token
  // Search for the FIR string itself. Since have encountered
  // xxx UIR xxx FIR, try to find a match do not break after
  // finding just a UIR or FIR token.

  bool found=false;
  size_t firTok=0;
  for (size_t ii = _startTokNum+1; ii < _msgToks.size(); ii++) {
    if (_isFirTok(_msgToks[ii])) {
      string firName;
      vector <size_t> firToks;
      found=_isFirName(_msgToks, ii, firToks, firName);
      if (found) {
        firTok=ii;
        break;
      }
    }
  }

  // Try the source as an alternative FIR

  bool sourceFir=_firLoc.FirExists(_decoded.getSource());

  if ((!found) && (!sourceFir)) {
    return(foundFir);
  }

  // Best case, try to find the name of the FIR from the FIR token

  string firName;
  if (found) {
    vector <size_t> firToks;
    foundFir=_isFirName(_msgToks, firTok, firToks, firName);

    // Set the _used[] for the FIR name

    //     for (size_t jj=0; jj < firToks.size(); jj++) {
    //       size_t tnum=firToks[jj];
    //       _used[tnum] = true;
    //     }
  }

  // Exit if we have not found a FIR

  if ((!found) && (!sourceFir)) {
    return(foundFir);
  }
   
  // Set the firName to the source if that is all we have
  // But this fails the strictness check

  if ((!found) && (sourceFir)) {
    firName=_decoded.getSource();
    found=true;
  }

  // Set the decoded portion

  if (found) {
    _decoded.setFir(firName);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "FIR: " << _decoded.getFir() << endl;
    }
  } else {
    if (_params.print_decode_problems_to_stderr) {
      cerr << "Cannot match FIR string" << endl;
    }
    _decoded.setFir("UNKNOWN");
  }

  // If we have a FIR but not a confirmed source (i.e., the centroid is
  // not set) then set the centroid from the FIR.

  if (!_decoded.centroidSet()) {
    _setCentroidFromFir();
  }

  // Return the strictness check bool

  return(foundFir);

}

////////////////////////////////////////////////
// find the movement or expected movement token.
//
//

void SigAirMet2Spdb::_setMovement()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setMovement()" << endl;
  }

  // Set the used flags or not

  bool doUsed=false;

  // Set defaults

  double dirn=0;
  double speed=0;

  // start search at the SIGMET/AIRMET token

  string moveType="NONE";
  string moveDir="NONE";
  string moveSpeed="NONE";
  bool found=false;
  for (size_t ii = _startTokNum+1; ii < _msgToks.size(); ii++) {
    const string &tok = _msgToks[ii];

    // Strip off trailing delimiters if they exist

    string tmpTok;
    _stripTermChars(tok, tmpTok);

    // Stationary movement.

    string tok2;
    if (ii+1 < _msgToks.size()) {
      tok2=_msgToks[ii+1];
    } else {
      tok2="NULL";
    }
    bool useTok2;
    if (_isStatMovTok(tmpTok, tok2, useTok2)) {
      found=true;
      moveType="STNR";
      dirn=-1;
      speed=-1;

      // Set used
      if (doUsed) {
        _used[ii]=true;
      }

      break;
    }

    // Moving in some direction

    vector <size_t> moveToks;
    if (_isMovementTok(_msgToks, ii, moveToks, moveType, moveDir, moveSpeed)) {

      found=true;

      // Convert units. Dir to degrees true N. Speed to km/hr.
       
      bool convDir=_dir2Angle(moveDir, dirn);
      if (!convDir) {
        dirn=0;
        if (_allDigits(moveDir)) {
          int idir=-1;
          if (sscanf(moveDir.c_str(), "%3d", &idir) == 1) {
            dirn=(float)idir;
          }
        }
      }

      bool convSpeed=_speed2Kph(moveSpeed, speed);
      if (!convSpeed) {
        speed=0;
      }

      // Set used

      if (doUsed) {
        for (size_t jj=0; jj< moveToks.size(); jj++) {
          size_t idx=moveToks[jj];
          _used[idx]=true;
        }
      }
      break;
    }
  } //endfor

  // Exit if not found

  if (!found) {
    return;
  }

  // Debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "setMovement: " << moveType
         << ", str dir: " << moveDir << ", str speed: " << moveSpeed
         << ", dirn: " << dirn << ", speed: " << speed << endl;
  }
  else if (_params.print_decode_problems_to_stderr) {
    cerr << "Movement: dir: " << dirn
         << ", speed: " << speed << endl;
  }

  // Set the speed and direction

  _decoded.setMovementSpeed(speed);
  _decoded.setMovementDirn(dirn);

  return;
}


////////////////////////////////////////////////
// find the intensity
//
//

void SigAirMet2Spdb::_setIntensity()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setIntensity()" << endl;
  }

  // start search at the SIGMET/AIRMET token

  bool found=false;
  string intensity="NONE";
  for (size_t ii = _startTokNum+1; ii < _msgToks.size(); ii++) {
    const string &tok = _msgToks[ii];

    // Strip off trailing delimiters if they exist

    string tmpTok;
    _stripTermChars(tok, tmpTok);

    // Intensity.

    if (_isIntensityTok(tmpTok)) {
      found=true;
      intensity=tok;
      break;
    }
  }

  // Exit if not found

  if (!found) {
    return;
  }

  // Print and exit

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Intensity change: " << intensity << endl;
  }

  return;
}



////////////////////////////////////////////////
// find the forecast position
//
//

void SigAirMet2Spdb::_setFcastPosition()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setFcastPosition()" << endl;
  }

  // Clear the forecast vectors

  _decoded.clearForecasts();

  // Is this a TC or VA? These are the only Wx types with fcast positions

  bool isWantedWx=_isFcastOutlookWxType();
  if (!isWantedWx) {
    return;
  }

  // start search at the SIGMET/AIRMET token

  bool found=false;
  size_t fcastTok=0;
  for (size_t ii = _startTokNum+1; ii < _msgToks.size(); ii++) {
    string &tok = _msgToks[ii];
    _stripTermChars(tok, tok);

    // Forecast.

    if (tok == "FCST") {
      found=true;
      fcastTok=ii;
      break;
    }
  }

  // Exit if not found

  if (!found) {
    return;
  }

  // first thing after FCST should be the time

  time_t fcastTime;
  bool foundTime=false;
  if (fcastTok+1 < _msgToks.size()) {
    foundTime=_isTimeTok(_msgToks[fcastTok+1], fcastTime);
  }

  // handle FCST AT TIME
  if (!foundTime && fcastTok+2 < _msgToks.size() && _msgToks[fcastTok+1] == "AT")
    {
      foundTime = _isTimeTok(_msgToks[fcastTok+2],fcastTime);
    }

  if (!foundTime) {
    if (_params.debug) {
      cerr << "setFcastPosition: Could not find forecast time" << endl;
      return;
    }
  }
     
  // now look for position -- could be either a CENTER or a POLYGON
  // or a LINE. Only one forecast position is expected.

  found=false;
  vector <double> lats;
  vector <double> lons;
  bool needWidth=false;
  bool forceCenter=false;

  if (_verticesFromCenter(fcastTok+1, _msgToks.size(), _msgToks,
                          needWidth, forceCenter, lats, lons) == 0)
    { 
    found=true;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "setFcastPosition: SUCCESS from verticesFromCenter()" << endl;
    }
  }

  if (!found) {
    if (_verticesFromLine(fcastTok+1, _msgToks.size(), _msgToks,
                          lats, lons) == 0) {
      found=true;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "setFcastPosition: SUCCESS from verticesFromLine()" << endl;
      }
    }
  }

  if (!found) {
    if (_verticesFromPolygon(fcastTok+1, _msgToks.size(), _msgToks,
                             lats, lons) == 0) {
      found=true;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "setFcastPosition: SUCCESS from verticesFromPolygon()" << endl;
      }
    }
  }

  if (!found && !_centroidFromObsStation) {
    if (_verticesFromFir(fcastTok+1, _msgToks.size(), _msgToks,
                         lats, lons) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "setFcastPosition: SUCCESS from verticesFromFir()" << endl;
      }
      found=true;
    }
  }

  // Set points

  if (found) {
    int id=0;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "adding a forecast polygon, ID: " << id << endl;
    }  
    for (size_t jj=0; jj<lats.size(); jj++) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "  lat,lon: " << lats[jj] << ", " << lons[jj] << endl;
      }  
      _decoded.addForecast(fcastTime, lats[jj], lons[jj], id);
    }
     
  } 

  else {

    // We could not find the position

    if (_params.debug) {
      cerr << "setFcastPosition: Could not find forecast point(s)" << endl;
    }
  }

  return;

}



////////////////////////////////////////////////
// find the outlook position(s)
//
//

void SigAirMet2Spdb::_setOutlookPosition()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setOutlookPosition()" << endl;
  }

  // Clear the outlook vectors

  _decoded.clearOutlooks();

  // Is this a TC or VA? These are the only Wx types with outlook positions

  bool isWantedWx=_isFcastOutlookWxType();
  if (!isWantedWx) {
    return;
  }

  // start search at the SIGMET/AIRMET token

  bool found=false;
  size_t outlookTok=0;
  for (size_t ii = _startTokNum+1; ii < _msgToks.size(); ii++) {
    const string &tok = _msgToks[ii];

    // Outlook. Cannot use an absolute search for the
    // string because is often followed by a = or .

    if ((tok.find("OTLK", 0) != string::npos) ||
        (tok.find("OUTLOOK", 0) != string::npos)) {
      found=true;
      outlookTok=ii;
      break;
    }
  }

  // Exit if not found

  if (!found) {
    return;
  }

  // Now that we have the OTLK start, look for sets of time plus
  // positions (either as CENTER or polygon)

  int id=0;
  found=false;
  bool forceCenter=false;
  for (size_t ii=outlookTok; ii< _msgToks.size(); ii++) {
       
    string &tok = _msgToks[ii];

    // Sometimes we get only one CENTER or CENTRE string but it is implied
    // for subsequent points. Only true for TC.

    if ((tok == "CENTER" || tok == "CENTRE") && (isWantedWx)) {
      forceCenter=true;
    }

    // Look for a time

    time_t outlookTime;
    if (_isTimeTok(tok, outlookTime)) {

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "setOutlook: time: " << tok << endl;
      }

      size_t timeIdx=ii;
      bool foundPoints=false;

      // now look for outlook position -- could be either a CENTER or a POLYGON
      // or a LINE.
       
      vector <double> lats;
      vector <double> lons;
      bool needWidth=false;

      if (_verticesFromCenter(timeIdx+1, _msgToks.size(), _msgToks,
                              needWidth, forceCenter, lats, lons) == 0) {
        foundPoints=true;
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "setOutlook: SUCCESS from verticesFromCenter()" << endl;
        }
      }
       
      if (!foundPoints) {
        if (_verticesFromLine(timeIdx+1, _msgToks.size(), _msgToks,
                              lats, lons) == 0) {
          foundPoints=true;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "setOutlook: SUCCESS from verticesFromLine()" << endl;
          }
        }
      }
       
      if (!foundPoints) {
        if (_verticesFromPolygon(timeIdx+1, _msgToks.size(), _msgToks,
                                 lats, lons) == 0) {
          foundPoints=true;
          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "setOutlook: SUCCESS from verticesFromPolygon()" << endl;
          }
        }
      }
	 
      // Set points. Increment the id number for each polygon

      found=true;
      if (foundPoints) {
        for (size_t jj=0; jj<lats.size(); jj++) {

          if (_params.debug >= Params::DEBUG_VERBOSE) {
            cerr << "setOutlook: add outlook: time: "
                 << utimstr(outlookTime) << " lat,lon: " << lats[jj] << ", "
                 << lons[jj] << ", id: " << id << endl;
          }

          _decoded.addOutlook(outlookTime, lats[jj], lons[jj], id);
        }
        id++;
      }
	 
      // Reset ii past the time string so can try to find the next location

      ii=ii+2;

    }// end _isTimeTok()
  }

  // Debug

  if (!found && _params.debug) {
    cerr << "setOutlookPosition: Could not find outlook point(s)" << endl;
  }

  if (found && _params.print_decode_problems_to_stderr) {
    cerr << "N Outlooks: " << id << endl;
  }
   
  return;
}



////////////////////////////////////////////////
// over-write cancelled sigmet

void SigAirMet2Spdb::_overwriteCancelled()
  
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _overwriteCancelled()" << endl;
  }

  if (_params.debug) {
    cerr << "Searching to cancel "
         << ((_cancelGroup == SIGMET_GROUP)? "SIGMET" : "AIRMET")
         << " "
         << _decoded.getSource() << ":" << _cancelId << endl;
  }

  // set the data type

  si32 data_type = Spdb::hash4CharsToInt32(_decoded.getSource());

  // grab interval holding the entry to cancel
  
  DsSpdb in;
  if(in.getInterval(_params.cancel_input_url,
                    _decoded.getIssueTime() - SECS_IN_DAY,
                    _decoded.getIssueTime(),
                    data_type)) {
    if (_params.debug) {
      cerr << "ERROR - SigAirMet2Spdb::_overwriteCancelled" << endl;
      cerr << "  Cannot retrieve "
           << ((_cancelGroup == SIGMET_GROUP)? "SIGMET" : "AIRMET")
           << " "
           << _decoded.getSource() << ":" << _cancelId << endl;
      cerr << "  " << in.getErrStr() << endl;
    }
    return;
  }

  // get chunk vector
  
  const vector<Spdb::chunk_t> &chunks = in.getChunks();
  if (chunks.size() < 1) {
    if (_params.debug) {
      cerr << "ERROR - SigAirMet2Spdb::_overwriteCancelled" << endl;
      cerr << "  Cannot find "
           << ((_cancelGroup == SIGMET_GROUP)? "SIGMET" : "AIRMET")
           << " "
	   << _decoded.getSource() << ":" << _cancelId << endl;
    }
    return;
  }
  
  for (int ii = chunks.size() - 1; ii >= 0; ii--) {
    
    SigAirMet candidate;
    candidate.disassemble(chunks[ii].data, chunks[ii].len);
    
    if (candidate.getId() != _cancelId ||
        candidate.getGroup() != _cancelGroup) {
      // not the same ID and group
      continue;
    }

    if (candidate.getCancelFlag()) {
      // already cancelled
      if (_params.debug) {
        cerr << "-->> Previously cancelled" << endl;
      }
      continue;
    }

    if (_params.debug) {
      cerr << "Cancelling "
           << ((_cancelGroup == SIGMET_GROUP)? "SIGMET" : "AIRMET")
           << " "
           << _decoded.getSource() << ":" << _cancelId << endl;
      cerr << "================== text =======================" << endl;
      cerr << candidate.getText() << endl;
      cerr << "===============================================" << endl;
    }

    time_t cancelTime = _decoded.getIssueTime();
    candidate.setCancelFlag(true);
    candidate.setCancelTime(cancelTime);
    
    // add to spdb objects as appropriate
    
    candidate.assemble();
    
    DsSpdb out;
    time_t expireTime = chunks[ii].expire_time;
    if (expireTime > cancelTime) {
      expireTime = cancelTime;
    }
    out.addPutChunk(chunks[ii].data_type,
		    chunks[ii].valid_time,
		    expireTime,
		    candidate.getBufLen(),
		    candidate.getBufPtr(),
		    chunks[ii].data_type2);
    if (out.put(_params.decoded_output_url,
		SPDB_SIGAIRMET_ID, SPDB_SIGAIRMET_LABEL)) {
      cerr << "ERROR - SigAirMet2Spdb::_overwriteCancelled" << endl;
      cerr << out.getErrStr() << endl;
    }

  } // ii

}


////////////////////////////////////////////////
//  Look for the location indicator/MWO in the header
// remove it to prevent problems with points search
//
//

bool SigAirMet2Spdb::_hasMWO(size_t &mwoIdx)

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _hasMWO()" << endl;
  }

  // Remove the ICAO location indicator/MWO from the search

  bool foundMWO=false;
  if (_isLocationIndicator(_startTokNum+1, _msgToks.size(), 
                           _msgToks, mwoIdx)) {
    _used[mwoIdx]=true;
    foundMWO=true;
  }

  if (_params.print_decode_problems_to_stderr) {
    if (foundMWO) {
      cerr << "Location indicator/MWO in header: " << _msgToks[mwoIdx] << endl;
    }
    else {
      cerr << "Strictness info: No location indicator/MWO found in header" << endl;
    }
  }

  return(foundMWO);
}


////////////////////////////////////////////////
//  Strictness check - look for the FIR/CTA or aircraft id
//   AIRMETS only allow the FIR
//
//

void SigAirMet2Spdb::_hasFirCtaOrAircraft(const bool &foundFir, 
					  const bool &foundMwo, 
					  const size_t &mwoIdx)

{

  // Has a FIR/UIR set, return

  // if ((foundFir) && (_decoded.getFir() != "UNKNOWN")) {
  if ((foundFir) && (strncmp(_decoded.getFir(), "UNKNOWN", SIGAIRMET_FIR_NBYTES) != 0) )  {
    return;
  }

  // If an AIRMET, print message and return

  if (_decoded.getGroup() == AIRMET_GROUP) {
    if (_params.print_decode_problems_to_stderr) {
      cerr << "Strictness info: no FIR" << endl;
    }
    return;
  }

  // Alternatives are CTA or aircraft radiotelephony call

  bool found=false;

  if ((foundMwo) && (mwoIdx > 0)) {
    for (size_t ii = mwoIdx; ii < _msgToks.size(); ii++) {
      const string tok=_msgToks[ii];
      const string prevTok=_msgToks[ii-1];
       
      // Look for xxxxxx CTA
       
      if (tok == "CTA") {
        found=true;
        if (_params.print_decode_problems_to_stderr) {
          cerr << "Strictness info: found CTA: " << prevTok << ", FIR/UIR or CTA is needed" << endl;
        }
        return;
      }
    }
  }

  // Try for aircraft radiotelephony call
   
  size_t max=_msgToks.size();
  if ((!found) && (foundMwo) && (mwoIdx > 0) && (mwoIdx+2 < max)) {
    if (_params.print_decode_problems_to_stderr) {
      cerr << "Strictness info: if either token is FIR/UIR name, or aircraft radiotelephony call then is ok: " << _msgToks[mwoIdx+1] << " " << _msgToks[mwoIdx+2] << endl;
    }
    return;
  }

  // No foundMwo

  if (_params.print_decode_problems_to_stderr) {
    cerr << "Strictness info: Cannot find FIR/UIR/CTA or aircraft radiotelephony call after MWO" << endl;
  }
}


////////////////////////////////////////////////
//  Look for the weather type as valid for this group
//
//

void SigAirMet2Spdb::_hasValidWx()

{

  // Not yet implemented

  return;
}


////////////////////////////////////////////////
// find and set vertices
//
// Returns true if able to set vertices, false if not

bool SigAirMet2Spdb::_setVertices()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setVertices()" << endl;
  }

  vector <double> lats;
  vector <double> lons;
  bool needWidth=true;
  bool forceCenter=false;
  bool found=false;

  if (_verticesFromLine(_startTokNum+1, _msgToks.size(), _msgToks,
                        lats, lons) == 0) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "SUCCESS from verticesFromLine()" << endl;
    }
    found=true;
  }

  if (!found) {
    if (_verticesFromCenter(_startTokNum+1, _msgToks.size(), _msgToks,
                            needWidth, forceCenter, lats, lons) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "SUCCESS from verticesFromCenter()" << endl;
      }
      found=true;
    }
  }

  if (!found) {
    if (_verticesFromPolygon(_startTokNum+1, _msgToks.size(), _msgToks,
                             lats, lons) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "SUCCESS from verticesFromPolygon()" << endl;
      }
      found=true;
    }
  }

  if (!found && !_centroidFromObsStation) {
    if (_verticesFromFir(_startTokNum+1, _msgToks.size(), _msgToks,
                         lats, lons) == 0) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "SUCCESS from verticesFromFir()" << endl;
      }
      found=true;
    }
  }

  if (lats.size() < 1) {
    found = false;
  }

  // Add the points

  if (found) {

    // make sure polygon closes

    int npts = (int) lats.size();
    if (lats[0] != lats[npts - 1] ||
        lons[0] != lons[npts - 1]) {
      lats.push_back(lats[0]);
      lons.push_back(lons[0]);
    }

    for (size_t ii = 0; ii < lats.size(); ii++) {
      _decoded.addVertex(lats[ii], lons[ii]);
    }

    if (_params.print_decode_problems_to_stderr && (lats.size() == 1)) {
      cerr << "Found a single point polygon!" << endl;
      cerr << "lat, lon: " << lats[0] << ", " << lons[0] << endl;
    }

    // Generate a new centroid based on the polygon not the
    // originating station

    _decoded.computeCentroid();

    return true;
  }

  // Error return

  if (_params.print_decode_problems_to_stderr && 
      (lats.size() == 0) && (_decoded.centroidSet() != 0)) {
    cerr << "No vertices but do have centroid" << endl;
  }

  return false;
}


////////////////////////////////////////////////
// set vertices from polygon points
//
// returns 0 on success, -1 on failure

int SigAirMet2Spdb::_verticesFromPolygon(const size_t &startIdx,
                                         const size_t &endIdx,
                                         const vector <string> &toks,
                                         vector <double> &lats,
                                         vector <double> &lons)


{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _verticesFromPolygon()" << endl;
  }


  // first check for a LINE indication that was not handled by 
  // verticesFromLine.  If this is here, it probably indicates a
  // modified fir & we should just return -1 and let verticesFromFir 
  // handle it.

  for (size_t ii = 0; ii < toks.size(); ii++) {
    string tok = toks[ii];
    
    if (_isLineTok(tok)){
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "   _verticesFromPolygon(): Found a LINE.  Bailing out to let verticesFromFir handle this one." << endl;
      }	
    return -1;

    }
  }
       




  // Get the points from the input vector
  // For a polygon we want at least 3 points

  vector <bool> sourceIsLatLon;
  bool skipIcaoStation=true;
  int minNpoints=3;
  size_t usedStartIdx=0;
  size_t usedEndIdx=0;
  int nPoints=_findPoints(toks, startIdx, endIdx,
                          skipIcaoStation, minNpoints,
                          lats, lons, sourceIsLatLon,
                          usedStartIdx, usedEndIdx);

  if (nPoints < 1) {
    return -1;
  }

  // if only one point, from latlon, return now, cannot check validity

  if (nPoints == 1) {
    if (sourceIsLatLon[0]) {
      return 0;
    } else {
      return -1;
    }
  }
   
  // make sure polygon closes
   
  if (lats[0] != lats[lats.size() - 1] ||
      lons[0] != lons[lons.size() - 1]) {
    lats.push_back(lats[0]);
    lons.push_back(lons[0]);
  }
   
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "verticesFromPolygon: nPoints: " << nPoints << endl;
  }

  // Remove outliers

  _fixPolylinePoints(lats, lons, sourceIsLatLon, true);
  nPoints=(int) lats.size();
   
  // Do not want polygons with less than 3 points (a line)

  if (nPoints < 3) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////
// set vertices from line points
//
// returns 0 on success, -1 on failure

int SigAirMet2Spdb::_verticesFromLine(const size_t &startIdx,
                                      const size_t &endIdx,
                                      const vector <string> &toks,
                                      vector <double> &lats,
                                      vector <double> &lons)

{
   
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _verticesFromLine()" << endl;
  }

  // find LINE token

  bool foundLine = false;
  size_t lineTokNum;
  for (size_t ii = startIdx; ii < endIdx; ii++) {
    if (_isLineTok(toks[ii])) {
      lineTokNum = ii;
      foundLine = true;
      break;
    }
  } // ii

  if (!foundLine) {
    return -1;
  }

  // find width

  int widthNm;
  size_t widthStartIdx, widthEndIdx;
  bool widthFound = _findWidth(startIdx, endIdx, toks, widthNm,
                               widthStartIdx, widthEndIdx);

  if (!widthFound) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << ">>>>> ERROR - line found with no width" << endl;
    }
    return -1;
  }

  double halfWidthKm = (widthNm * KM_PER_NM) / 2.0;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "verticesFromLine: line widthNm: " << widthNm << endl;
  }

  // find start token for points, look for FROM

  size_t startLineTokNum;
  bool foundFrom=_findFrom(startIdx, endIdx, toks, startLineTokNum);
  if (!foundFrom && _params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "verticesFromLine: Warning: line found with no FROM" << endl;
  }

  // Find line points

  vector <bool> sourceIsLatLon;
  bool skipIcaoStation=false;
  int minNpoints=2;
  size_t usedStartIdx=0;
  size_t usedEndIdx=0;
  vector<double> lineLats, lineLons;
  int nPoints=_findPoints(toks, startLineTokNum, toks.size(),
                          skipIcaoStation, minNpoints,
                          lineLats, lineLons, sourceIsLatLon,
                          usedStartIdx, usedEndIdx);
   
  if (nPoints < 2) {
    return -1;
  }

  // Remove outliers

  _fixPolylinePoints(lineLats, lineLons, sourceIsLatLon, false);
  size_t nPtsLine = lineLats.size();
  if (nPtsLine < 2) {
    return -1;
  }

  // compute line segments

  vector<line_segment_t> segments;

  for (size_t ii = 0; ii < nPtsLine - 1; ii++) {
    line_segment_t seg;
    seg.lat0 = lineLats[ii];
    seg.lon0 = lineLons[ii];
    seg.lat1 = lineLats[ii+1];
    seg.lon1 = lineLons[ii+1];
    PJGLatLon2RTheta(seg.lat0, seg.lon0,
                     seg.lat1, seg.lon1,
                     &seg.dist, &seg.bearing);
    segments.push_back(seg);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (int ii = 0; ii < (int) lineLats.size(); ii++) {
      cerr << "line lat, lon: " << lineLats[ii] << ", " << lineLons[ii] << endl;
    }
  }

  // compute vertices, first go along L side, then come back along R side

  for (int ii = 0; ii < (int) segments.size(); ii++) {

    const line_segment_t &seg = segments[ii];

    double lat, lon;
    PJGLatLonPlusRTheta(seg.lat0, seg.lon0,
                        halfWidthKm, seg.bearing - 90.0,
                        &lat, &lon);
    lats.push_back(lat);
    lons.push_back(lon);

    PJGLatLonPlusRTheta(seg.lat1, seg.lon1,
                        halfWidthKm, seg.bearing - 90.0,
                        &lat, &lon);
    lats.push_back(lat);
    lons.push_back(lon);

  }

  for (int ii = segments.size() - 1; ii >= 0; ii--) {

    const line_segment_t &seg = segments[ii];
     
    double lat, lon;
    PJGLatLonPlusRTheta(seg.lat1, seg.lon1,
                        halfWidthKm, seg.bearing + 90.0,
                        &lat, &lon);
    lats.push_back(lat);
    lons.push_back(lon);
     
    PJGLatLonPlusRTheta(seg.lat0, seg.lon0,
                        halfWidthKm, seg.bearing + 90.0,
                        &lat, &lon);
    lats.push_back(lat);
    lons.push_back(lon);

  } // ii


  // close polygon

  lats.push_back(lats[0]);
  lons.push_back(lons[0]);

  return 0;

}


////////////////////////////////////////////////
// find a center and a circle
//
// returns 0 on success, -1 on failure

int SigAirMet2Spdb::_verticesFromCenter(const size_t &startIdx,
                                        const size_t &endIdx,
                                        const vector <string> &toks,
                                        const bool &needWidth,
                                        const bool &forceCenter,
                                        vector <double> &lats,
                                        vector <double> &lons)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _verticesFromCenter()" << endl;
  }

  // Special case. Can have a center that is a single point (no circle)
  // for specific weather types

  bool localNeedWidth=needWidth;

  bool possibleSinglePoint=_isSinglePointWxType();
  if (possibleSinglePoint) {
    localNeedWidth=false;
  }

  // find CENTER token, skip if force

  bool foundCenter = false, foundWI=false, foundDiam=false;
  int nNm=0, nLatLon=0;
  size_t centerTokNum;
  size_t endInd=endIdx;

  for (size_t ii = startIdx; ii < endIdx; ii++) {

    string tok=toks[ii];

    if (tok == "CENTER" ||
        tok == "CENTRE") {
      centerTokNum = ii;
      foundCenter = true;
      break;
    }

    // Alternative, can find a center by finding a width and only 1 lat lon point
    // and a keyword "WI"

    _stripTermChars(tok, tok);
     
    if (tok == "WI" ||
        tok == "WITHIN" ||
        tok == "WTN") {
      foundWI=true;
      centerTokNum=ii;
    }
    if ((foundWI) && (tok == "NM")) {
      nNm++;
    }
    double lat,lon;
    if ((foundWI) && (_hasLatLon(tok, lat, lon))) {
      nLatLon++;
    }

    // Alternative, can find a center (CONUS only) by finding a diameter
    // expressed as Dnn. Will not find a latlon in this case or a WI
    // keyword.

    int diam;
    if (_isDiameterTok(tok, diam)) {
      foundDiam=true;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "verticesFromCenter: diameter: " << diam
             << ", from tok: " << tok << endl;
      }
    }

    // We want the current location not the outlook so break if we have
    // not found the center yet.
    // When this is called from setForecast() we want to break
    // if we have found the outlook.

    if (tok == "OUTLOOK" || tok == "OTLK") {
      endInd=ii;
      break;
    }
  } // ii

  if ((foundWI) && (nNm == 1) && (nLatLon <= 1)) {
    foundCenter=true;
  }
   
  if (foundDiam) {
    foundCenter=true;
  }

  if (!foundCenter && !forceCenter) {
    return -1;
  }

  // find width

  double widthKm = 0.0;
  size_t widthStartIdx=0;
  size_t widthEndIdx=0;

  if (localNeedWidth) {
    int widthNm;
    bool widthFound = _findWidth(startIdx, endIdx, toks, widthNm,
                                 widthStartIdx, widthEndIdx);

    if (!widthFound) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << ">>>>> ERROR - center found with no width" << endl;
      }
      return -1;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "verticesFromCenter: widthNm: " << widthNm << endl;
    }

    widthKm = widthNm * KM_PER_NM;
  }

  // find latlon point for the center

  vector <bool> sourceIsLatLon;
  bool skipIcaoStation=true;
  int minNpoints=2;
  if (forceCenter) {
    minNpoints=1;
  }
  lats.clear();
  lons.clear();
  size_t usedStartIdx=0;
  size_t usedEndIdx=0;
  int nPoints=_findPoints(toks, startIdx, endInd,
                          skipIcaoStation, minNpoints,
                          lats, lons, sourceIsLatLon,
                          usedStartIdx, usedEndIdx);

  if (nPoints <= 0) {
    return -1;
  }

  // Search thru the list of returned points. Take the first point
  // found but we really want the first lat,lon found. See if we found
  // >1 latlon -- might have failed on the initial _hasLatLon() above
  // due to delimiters.

  nLatLon=0;
  double centerLat;
  double centerLon;
  bool firstPt=true;
  for (size_t ii=0; ii<lats.size(); ii++) {
    if (firstPt) {
      centerLat=lats[ii];
      centerLon=lons[ii];
      firstPt=false;
      if (sourceIsLatLon[ii]) {
        nLatLon++;
      }
    }
    else if (sourceIsLatLon[ii]) {
      if (nLatLon == 0) {
        centerLat=lats[ii];
        centerLon=lons[ii];
      }
      nLatLon++;
    }
  }

  // Exit if we found >1 lat lons. Not a center. Not true if we are
  // forcing a center, or center found

  if ((nLatLon > 1) && (!forceCenter) && (!foundCenter)) {
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "     center: lat,lon: " << centerLat
         << ", " << centerLon << endl;
  }

  // calculate angles to approximate a circle and compute vertices
  // compute vertices. Skip if not doing width.

  if (localNeedWidth && widthKm > 0) {
    int nPtsCircle=8;
    double interval=360.0/nPtsCircle;
    lats.clear();
    lons.clear();

    for (int ii = 0; ii < nPtsCircle; ii++) {
      double lat,lon;
      double theta=interval * (ii + 1);
      if (theta > 180) {
        theta = theta - 360.0 ;
      }
      PJGLatLonPlusRTheta(centerLat, centerLon,
                          widthKm, theta,
                          &lat, &lon);
      lats.push_back(lat);
      lons.push_back(lon);

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "    lat,lon: " << lat << ","
             << lon << ", theta: " << theta << endl;
      }
    }
  }

  // If not doing width->not doing circle.
  // So return the center in the lats, lons

  else {
    lats.clear();
    lons.clear();

    lats.push_back(centerLat);
    lons.push_back(centerLon);
  }

  // Set the used array

  //   if (localNeedWidth) {
  //     if (widthStartIdx > 0) {
  //       if (widthEndIdx <= 0) {
  //	 widthEndIdx=widthStartIdx;
  //       }
  //       for (size_t ii=widthStartIdx; ii <= widthEndIdx; ii++) {
  //	 _used[ii]=true;
  //       }
  //     }
  //   }

  //   if (usedStartIdx > 0) {
  //     if (usedEndIdx <= 0) {
  //       usedEndIdx=usedStartIdx;
  //     }
  //     for (size_t ii=usedStartIdx; ii <= usedEndIdx; ii++) {
  //       _used[ii]=true;
  //     }
  //   }

  return 0;

}


////////////////////////////////////////////////
// set vertices from FIR points
//
// returns 0 on success, -1 on failure

int SigAirMet2Spdb::_verticesFromFir(const size_t &startIdx,
                                     const size_t &endIdx,
                                     const vector <string> &toks,
                                     vector <double> &lats,
                                     vector <double> &lons)
{
   
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _verticesFromFir()" << endl;
  }

  // Return if the fir is not set

  //  if (!_decoded.getFir() || _decoded.getFir() == "UNKNOWN") {
  if (!_decoded.getFir() || (strncmp(_decoded.getFir(), "UNKNOWN", SIGAIRMET_FIR_NBYTES) == 0)){
    return -1;
  }

  // Get the FIR boundary polygon points

  vector <double> firLats;
  vector <double> firLons;
   
  bool found=_firLoc.GetFirPoints(_decoded.getFir(), firLats, firLons);
  if (!found) {
    return -1;
  }
  size_t nPoints=firLats.size();
   
  // Do not want polygons with only 2 points (a line)

  if (nPoints <= 2) {
    return -1;
  }

  // Clear polygon boundaries

  lats.clear();
  lons.clear();

  // Set the polygon boundaries to the FIR boundaries

  lats=firLats;
  lons=firLons;
  bool doSet=true;

  // Are there any modifications to the FIR boundaries? We can assume
  // that we would not have made it to this function if there were
  // a polygon or a line with a width so this will need to be something
  // else...

  if (_params.modify_fir_boundary) {

    vector<double> outlats;
    vector<double> outlons;
    outlats.clear();
    outlons.clear();
    bool foundMod=_searchForModifyFir(startIdx, endIdx, toks,
                                      firLats, firLons, outlats, outlons);
     
    if (foundMod && (outlats.size() > 2)) {
      if (_isValidPolygon(outlats, outlons)) {
        // use the outlats,outlons to replace the FIR boundary
        // IF we have enough
        // points and is a valid polygon(??)
        lats=outlats;
        lons=outlons;
        doSet=false;

        if (_params.print_decode_problems_to_stderr) {
          cerr << "Modified FIR boundary to: " << lats.size()
               << " pts from original FIR: " << firLats.size()
               << " pts" << endl;
        }
      }
    }
  }

  nPoints=lats.size();
  if (_params.debug) {
    cerr << "verticesFromFir: name: " << _decoded.getFir()
         << ", nPoints: " << nPoints << endl;
  }
   
  // Set the flag in the data that the polygon is the entire FIR boundary
   
  if (doSet) {
    _decoded.setPolygonIsFirBdry();
  }
  return 0;
}


////////////////////////////////////////////////
// set text from unused tokens
//
// If useMessageString is false, only take text segments which are 4 or more
// tokens long otherwise use the entire messageStr.

void SigAirMet2Spdb::_setText(const string &messageStr,
                              const bool useMessageString)
  
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setText()" << endl;
  }

  if (useMessageString) {
    _decoded.setText(messageStr);
    return;
  }

  else {
    size_t len = 0;
    string text;
    string subText;
  
    for (size_t ii = _startTokNum + 2; ii < _used.size(); ii++) {
      if (!_used[ii] && _msgToks[ii].find("END/", 0) == string::npos) {
	subText += _msgToks[ii];
	subText += " ";
	len++;
      } else {
	if (len >= 4) {
	  text += subText;
	}
	len = 0;
	subText = "";
      }
    }
    if (len >= 4) {
      text += subText;
    }

    _decoded.setText(text);
  }
}

  
///////////////////////////////////////////////////////
// Set the centroid from the FIR boundaries
//
// Returns true if set centroid, false if not
//

bool SigAirMet2Spdb::_setCentroidFromFir()

{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Entering _setCentroidFromFir()" << endl;
  }

  if (_decoded.centroidSet()) {
    return false;
  }

  // Is a FIR name defined?

  string firName=_decoded.getFir();
  if (firName == "UNKNOWN") {
    return false;
  }

  // Get the FIR boundaries

  vector <double> lats;
  vector <double> lons;
  if (!_firLoc.GetFirPoints(firName, lats, lons)) {
    return false;
  }

  // Add the points. This is needed to use computeCentroid()

  for (size_t ii = 0; ii < lats.size() ; ii++) {
    _decoded.addVertex(lats[ii], lons[ii]);
  }

  // Generate the centroid based on the FIR boundaries

  _decoded.computeCentroid();

  // Clear the vertices. We may not want to use the FIR boundaries
       
  _decoded.clearVertices();

  return true;
}

///////////////////////////////////////////////////////
// 
//

void SigAirMet2Spdb::_handleSfcWindAndVis(int start_pos, string &wx_type)

{
  // search for end of weather component by looking forward to a 'OBS' token
  // offset start_pos by two token, because it points to 'SFC' in _msgToks
  for (int i = start_pos+2; i < _msgToks.size(); i++) {
    if ((_msgToks[i] == "OBS") || (_msgToks[i] == "FCST")) {
      break;
    }
    wx_type += " " + _msgToks[i];
    _used[i]=true;

  }
}


