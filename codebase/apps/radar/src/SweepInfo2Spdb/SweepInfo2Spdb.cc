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
////////////////////////////////////////////////////////////////////////
// SweepInfo2Spdb.cc
//
// SweepInfo2Spdb object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2006
//
///////////////////////////////////////////////////////////////
//
// SweepInfo2Spdb reads sweep data from UDP or a catalog file and
// writes the info to an SPDB data base.
//
///////////////////////////////////////////////////////////////////////

#include <dirent.h>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/sockutil.h>
#include <toolsa/ushmem.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <dataport/bigend.h>
#include "SweepInfo2Spdb.hh"
using namespace std;

// Constructor

SweepInfo2Spdb::SweepInfo2Spdb(int argc, char **argv)

{

  isOK = true;
  _currentPos = 0;
  _currentVolNum = -1;
  _sweepNumStartCurrentVol = 0;
  _currentInfo.sweep_num = 9999;
  _currentInfo.volume_num = 9999;
  _sweepInProgress = false;
  _sweepShmem = NULL;

  // set programe name

  _progName = "SweepInfo2Spdb";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
    return;
  }

  // create shmem if needed

  if (_params.write_to_shmem) {
    _sweepShmem =
      (sweep_info_packet_t *) ushm_create(_params.shmem_key,
                                          sizeof(sweep_info_packet_t),
                                          0666);
    if (_sweepShmem == NULL) {
      cerr << "ERROR - SweepInfo2Spdb" << endl;
      cerr << "  Cannot attach shared memory for sweep info" << endl;
      cerr << "  key: " << _params.shmem_key << endl;
    }
  }

  // init process mapper registration
  
  if (_params.register_with_procmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

  return;

}

// destructor

SweepInfo2Spdb::~SweepInfo2Spdb()

{

  // detach from shared memory

  if (_sweepShmem) {
    ushm_detach(_sweepShmem);
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SweepInfo2Spdb::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  int iret = 0;
  if (_params.mode == Params::ARCHIVE) {
    iret = _runArchive();
  } else if (_params.mode == Params::REALTIME) {
    iret = _runRealtime();
  } else if (_params.mode == Params::UDP) {
    iret = _runUdp();
  } else {
    cerr << "ERROR - SweepInfo2Spdb::Run" << endl;
    cerr << "  Unknown mode" << endl;
    iret = -1;
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in ARCHIVE mode

int SweepInfo2Spdb::_runArchive()
{

  int iret = 0;
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    long int endPos;
    if (_processFile(_args.inputFileList[ii].c_str(), 0, endPos)) {
      iret = -1;
    }
  }
  return iret;

}

//////////////////////////////////////////////////
// Run in REALTIME mode

int SweepInfo2Spdb::_runRealtime()
{

  // restore state

  _readStateFromFile();

  int iret = 0;

  while (true) {

    PMU_auto_register("runRealtime ...");

    // find latest catalog file
    
    string latestCatPath;
    int fileLen;
    if (_findLatestCatPath(latestCatPath, fileLen) == 0) {

      // process if the size of the file has grown, or the 
      // file path has changed
      
      if (latestCatPath != _currentPath || _currentPos > fileLen) {
        _currentPos = 0;
      }

      if (fileLen > _currentPos) {
        long int endPos;
        if (_processFile(latestCatPath.c_str(), _currentPos, endPos)) {
          iret = -1;
        } else {
          // save state
          _currentPos = endPos;
          _currentPath = latestCatPath;
          _saveStateToFile();
        }
      } // if (latestCatPath ...

    } // if (_findLatestCatPath ...
    
    // sleep a bit

    PMU_auto_register("zzzz ...");
    umsleep(1000);

  } // while

  return iret;

}

//////////////////////////////////////////////////
// Run in UDP mode

int SweepInfo2Spdb::_runUdp()
{

  // open socket for UDP datagrams

  struct sockaddr_in my_addr;
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0) {
    cerr << "ERROR - SweepInfo2Spdb::_runUdp()" << endl;
    cerr << "  Cannot open UDP socket" << endl;
    return -1;
  }

  // set socket parameters

  int val = 1;
  int valen = sizeof(val);
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &val, valen);
  memset(&my_addr, 0, sizeof(my_addr));

  // bind to port
  
  my_addr.sin_family = AF_INET;
  short int port = _params.udp_port;
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  int rcb = ::bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
  if (rcb < 0) {
    cerr << "ERROR - SweepInfo2Spdb::_runUdp()" << endl;
    cerr << "  Cannot bind to port: " << port  << endl;
    cerr << "  " << strerror(rcb) << endl;
    return -1;
  }

  // read UDP packets
  
  while (true) {

    PMU_auto_register("Waiting for UDP ...");

    // check for available data, waiting 1 sec before timeout
    // returns 1 on success, -1 on timeout, -2 on failure

    int iret = SKU_read_select(sock, 1000);
    
    if (iret < -1) {
      cerr << "ERROR - SweepInfo2Spdb::_runUdp()" << endl;
      cerr << "  SKU_read_select failed" << endl;
      return -1;
    }
    
    if (iret == -1) {
      // timeout
      continue;
    }
    
    // read

    PMU_auto_register("Reading UDP ...");

    sweep_info_packet_t info;
    int nread = recv(sock, &info, sizeof(info), 0);
    
    if (nread == sizeof(info)) {
      
      // swap bytes

      BE_to_array_32(&info, sizeof(info));

      // check for cal mode at high elevation angle
      // and replace cawith vert pointing mode
      if ((info.scan_type == ScanMode_CALBRATION ||
	   info.scan_type == ScanMode_SURVEILLANCE) &&
	  info.cur_el > 89.0) {
	info.scan_type = ScanMode_VERT_POINT;
      }

      // check for -1 values in input data

      if (info.volume_num < 0 ||
	  info.sweep_num < 0 ||
	  info.scan_type < 0) {
	cerr << "=================================================" << endl;
	cerr << "==>> WARNING - SweepInfo2Spdb" << endl;
	cerr << "==>>   Negative values found" << endl;
	cerr << "==>>   Time: " << DateTime::strm(info.time_secs) << "."
	     << (info.nano_secs / 1000000) << endl;
	cerr << "==>>   scan_type: " << info.scan_type << endl;
	cerr << "==>>   volume_num: " << info.volume_num << endl;
	cerr << "==>>   sweep_num: " << info.sweep_num << endl;
	cerr << "==>>   end_of_sweep_flag: " << info.end_of_sweep_flag;
	cerr << "==>>   end_of_vol_flag: " << info.end_of_vol_flag;
	cerr << "==>>   cur_el: " << info.cur_el;
	cerr << "==>>   cur_az: " << info.cur_az;
	cerr << "==>>   fixed_el: " << info.fixed_el;
	cerr << "==>>   fixed_az: " << info.fixed_az;
	cerr << "==>>   num_gates: " << info.num_gates << endl;
	cerr << "==>>   num_samples: " << info.num_samples << endl;
	cerr << "==>>   transition: " << info.transition;
	cerr << "=================================================" << endl;
      }

      // save to shmem if appropriate

      if (_sweepShmem != NULL) {
        memcpy(_sweepShmem, &info, sizeof(info));
      }

      // process packet

      _processUdpInfo(info);

    } // nread

  } // while

  return -1;

}

////////////////////
// process a file

int SweepInfo2Spdb::_processFile(const char *file_path,
                                 long int startPos,
                                 long int &endPos)
  
{
  
  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
    cerr << "  Starting pos: " << startPos << endl;
  }
  
  // registration
  
  char procmapString[BUFSIZ];
  Path path(file_path);
  sprintf(procmapString, "Processing file <%s>", path.getFile().c_str());
  PMU_auto_register(procmapString);

  // Open the file
  
  FILE *catalog;
  if((catalog = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
    cerr << "  Cannot open catalog file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // seek to starting position

  
  if (fseek(catalog, startPos, SEEK_SET)) {
    int errNum = errno;
    cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
    cerr << "  Cannot seek to start position: " << startPos << endl;
    cerr << "  File: " << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(catalog);
    return -1;
  }

  // create output object
  
  DsSpdb spdb;

  // read though the file, reading available lines

  char line[5000];
  int iret = 0;
  while(fgets(line, 5000, catalog) != NULL) {
    
    // tokenize the line, using commas as the delimiters

    vector<string> toks;
    _tokenize(line, ", ", toks);
    int minToksExpected = 23;

    if ((int) toks.size() < minToksExpected) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Not enough tokens in line: " << line << endl;
      cerr << "  Need at least: " << minToksExpected << endl;
      cerr << "  Found only: " << toks.size() << endl;
      cerr << "  File: " << file_path << endl;
      iret = -1;
      continue;
    }

    // skip top line

    if (toks[0].find("Sweep_File_Name") != string::npos) {
      continue;
    }

    // scan the tokens, set the data fields

    string fileName = toks[0];
    string scanMode = toks[4];

    int volNum;
    if (sscanf(toks[2].c_str(), "%d", &volNum) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read volNum" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }
    
    int sweepNum;
    if (sscanf(toks[3].c_str(), "%d", &sweepNum) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read sweepNum" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    if (volNum != _currentVolNum) {
      _currentVolNum = volNum;
      _sweepNumStartCurrentVol = sweepNum;
    }
    int tiltNum = sweepNum - _sweepNumStartCurrentVol;
    
    time_t startUTime;
    if (sscanf(toks[5].c_str(), "%ld", &startUTime) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read startUTime" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }
    
    int startNSecs;
    if (sscanf(toks[6].c_str(), "%d", &startNSecs) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read startNSecs" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }
    
    time_t endUTime;
    if (sscanf(toks[7].c_str(), "%ld", &endUTime) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read endUTime" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }
    
    int endNSecs;
    if (sscanf(toks[8].c_str(), "%d", &endNSecs) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read endNSecs" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }
    
    double fixedAngle;
    if (sscanf(toks[14].c_str(), "%lg", &fixedAngle) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read fixedAngle" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    double startAz;
    if (sscanf(toks[15].c_str(), "%lg", &startAz) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read startAz" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    double endAz;
    if (sscanf(toks[16].c_str(), "%lg", &endAz) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read endAz" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    double startEl;
    if (sscanf(toks[17].c_str(), "%lg", &startEl) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read startEl" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    double endEl;
    if (sscanf(toks[18].c_str(), "%lg", &endEl) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read endEl" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    bool isClockWise;
    if (toks[19] == "CW") {
      isClockWise = true;
    } else {
      isClockWise = false;
    }

    double prf;
    if (sscanf(toks[20].c_str(), "%lg", &prf) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read prf" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    int nSamples;
    if (sscanf(toks[21].c_str(), "%d", &nSamples) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read nSamples" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    int nGates;
    if (sscanf(toks[22].c_str(), "%d", &nGates) != 1) {
      cerr << "ERROR - SweepInfo2Spdb::_processFile" << endl;
      cerr << "  Cannot read nGates" << endl;
      cerr << "  line: " << line << endl;
      iret = -1;
      continue;
    }

    // create DsRadarSweep object

    DsRadarSweep sweep;
    
    sweep.setName(fileName);

    // check for cal mode at high elevation angle
    // and replace cawith vert pointing mode
    if ((sweep.getScanMode() == ScanMode_CALBRATION ||
	 sweep.getScanMode() == ScanMode_SURVEILLANCE) &&
	startEl > 89.0 && endEl > 89.0) {
      sweep.setScanMode(ScanMode_VERT_POINT);
    } else {
      sweep.setScanMode(scanMode);
    }

    sweep.setStartTime(startUTime, startNSecs);
    sweep.setEndTime(endUTime, endNSecs);
    sweep.setStartEl(startEl);
    sweep.setStartAz(startAz);
    sweep.setEndEl(endEl);
    sweep.setEndAz(endAz);
    sweep.setFixedEl(fixedAngle);
    sweep.setFixedAz(fixedAngle);
    sweep.setClockWise(isClockWise);
    sweep.setPrf(prf);
    sweep.setVolNum(volNum);
    sweep.setTiltNum(tiltNum);
    sweep.setSweepNum(sweepNum);
    sweep.setNSamples(nSamples);
    sweep.setNGates(nGates);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Success reading line:" << endl;
      cerr << line << endl;
      sweep.print(cerr, " ");
    }

    // assemble into chunk

    sweep.assemble();

    // add chunk
    
    spdb.addPutChunk(0, startUTime, endUTime,
                     sweep.getBufLen(), sweep.getBufPtr());

  } // while
  
  endPos = ftell(catalog);
  fclose(catalog);
  
  if (_params.debug) {
    cerr << "  Ending pos: " << endPos << endl;
  }

  // put to file

  if (_doPut(spdb)) {
    iret = -1;
  }

  return iret;

}

//////////////////////////////
// process sweep info from UDP

int SweepInfo2Spdb::_processUdpInfo(const sweep_info_packet_t &newInfo)

{

  PMU_auto_register("processing UDP ...");

  int iret = 0;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "=====>> INCOMING UDP PACKET <<=====" << endl;
    _printInfo(newInfo, cerr);
  }

  if (!_sweepInProgress ||
      newInfo.sweep_num != _currentInfo.sweep_num ||
      newInfo.volume_num != _currentInfo.volume_num) {
    
    // new sweep
    
    if (_params.debug) {
      cerr << "---->> New sweep" << endl;
      cerr << "=====>> CURRENT SWEEP <<=====" << endl;
      _printInfo(_currentInfo, cerr);
      cerr << "=====>> NEW SWEEP <<=====" << endl;
      _printInfo(newInfo, cerr);
    }

    // handle this new sweep

    if (_handleNewSweep(newInfo)) {
      iret = -1;
    }

    // save out current info
    
    _currentInfo = newInfo;
  
  }
  
  _latestInfo = newInfo;
  
  return iret;

}
      
//////////////////////////////
// handle a new sweep

int SweepInfo2Spdb::_handleNewSweep(const sweep_info_packet_t &newInfo)

{

  int iret = 0;

  DsSpdb spdb;
  bool isClockWise = true;

  if (!_sweepInProgress) {
    
    _sweepNumStartCurrentVol = newInfo.sweep_num;
    
  } else {

    // set end times in the current spdb entry, which was first 
    // written at the start of the sweep
    
    _currentSweep.setEndTime(_latestInfo.time_secs, _latestInfo.nano_secs);
    _currentSweep.setEndEl(_latestInfo.cur_el);
    _currentSweep.setEndAz(_latestInfo.cur_az);
    
    // assemble into chunk

    _currentSweep.assemble();
    
    // add chunk
    
    spdb.addPutChunk(0,
                     _currentSweep.getStartUTime(),
                     _currentSweep.getEndUTime(),
                     _currentSweep.getBufLen(),
                     _currentSweep.getBufPtr());

    if (_params.debug) {
      cerr << "===>> Writing out end of sweep <<===" << endl;
      _currentSweep.print(cerr, " ");
    }

    // check for isClockWise rotation
    
    double deltaAz = newInfo.cur_az - _latestInfo.cur_az;
    if (fabs(deltaAz) < 180) {
      if (deltaAz < 0) {
        isClockWise = false;
      } else {
        isClockWise = true;
      }
    } else {
      if (deltaAz < 0) {
        isClockWise = true;
      } else {
        isClockWise = false;
      }
    }

  } // if (_sweepInProgress) 
  
  // set sweep num for start of vol
  
  if (newInfo.volume_num != _currentInfo.volume_num) {
    _sweepNumStartCurrentVol = newInfo.sweep_num;
  }
  int tiltNum = newInfo.sweep_num - _sweepNumStartCurrentVol;

  // load current sweep
  // end time is start plus 10 mins - a guess for the max likely value
  
  _currentSweep.setName(_params.radar_name);
  _currentSweep.setScanMode(newInfo.scan_type);
  _currentSweep.setStartTime(newInfo.time_secs, newInfo.nano_secs);
  _currentSweep.setEndTime(-999, 0);
  _currentSweep.setStartEl(newInfo.cur_el);
  _currentSweep.setStartAz(newInfo.cur_az);
  _currentSweep.setEndEl(-999);
  _currentSweep.setEndAz(-999);
  _currentSweep.setFixedEl(newInfo.fixed_el);
  _currentSweep.setFixedAz(newInfo.fixed_az);
  _currentSweep.setClockWise(isClockWise);
  _currentSweep.setPrf(-999);
  _currentSweep.setVolNum(newInfo.volume_num);
  _currentSweep.setTiltNum(tiltNum);
  _currentSweep.setSweepNum(newInfo.sweep_num);
  _currentSweep.setNSamples(-999);
  _currentSweep.setNGates(-999);
  
  if (_params.debug) {
    cerr << "===>> Writing out start of sweep <<===" << endl;
    _currentSweep.print(cerr, " ");
  }

  // assemble into chunk
  
  _currentSweep.assemble();
  
    // add chunk
  
  spdb.addPutChunk(0,
                   _currentSweep.getStartUTime(),
                   _currentSweep.getStartUTime() + 60,
                   _currentSweep.getBufLen(),
                   _currentSweep.getBufPtr());
  
  // put to file
  
  if (_doPut(spdb)) {
    iret = -1;
  }

  // set sweep in progress flag
  
  _sweepInProgress = true;
  
  return iret;

}


////////////////////////////////
// do put to SPDB

int SweepInfo2Spdb::_doPut(DsSpdb &spdb)
  
{
  
  if (spdb.put(_params.output_url,
               SPDB_DS_RADAR_SWEEP_ID,
               SPDB_DS_RADAR_SWEEP_LABEL)) {
    cerr << "ERROR - SweepInfo2Spdb::_doPut" << endl;
    cerr << "  Cannot put sweep info to: "
         << _params.output_url << endl;
    cerr << "  " << spdb.getErrStr() << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////
// tokenize a string into a vector of strings

void SweepInfo2Spdb::_tokenize(const string &str,
			       const string &spacer,
			       vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  while (true) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      return;
    } else if (end == string::npos) {
      string tok;
      tok.assign(str, start, string::npos);
      toks.push_back(tok);
      return;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}

//////////////////////////////////////////////
// save state to file

int SweepInfo2Spdb::_saveStateToFile()
  
{

  if (_params.debug) {
    cerr << "Saving state to file: " << _params.state_file_path << endl;
  }

  // open state file
  
  FILE *out;
  if ((out = fopen(_params.state_file_path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SweepInfo2Spdb::_saveStateToFile" << endl;
    cerr << "  Cannot open state file: "
	 << _params.state_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // fill XML buffer

  string xml;
  xml += TaXml::writeStartTag("SweepInfo2Spdb_state", 0);
  xml += TaXml::writeString("current_path", 1, _currentPath);
  xml += TaXml::writeInt("current_pos", 1, _currentPos);
  xml += TaXml::writeInt("current_vol_num", 1, _currentVolNum);
  xml += TaXml::writeInt("sweep_num_start_current_vol", 1,
                         _sweepNumStartCurrentVol);
  xml += TaXml::writeEndTag("SweepInfo2Spdb_state", 0);
  
  // write to file

  fprintf(out, "%s", xml.c_str());
  fclose(out);
  
  return 0;
  
}

//////////////////////////////////////////////
// read in state file file

int SweepInfo2Spdb::_readStateFromFile()
  
{

  if (_params.debug) {
    cerr << "Reading state to file: " << _params.state_file_path << endl;
  }

  // open state file

  FILE *in;
  if ((in = fopen(_params.state_file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "WARNING - SweepInfo2Spdb::_readStateFromFile" << endl;
    cerr << "  Cannot open state file: "
	 << _params.state_file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read into string buffer
  
  string xmlBuf;
  char line[1024];
  while (fgets(line, 1024, in) != NULL) {
    xmlBuf += line;
  }
  fclose(in);
  
  // decode XML

  int iret = 0;
  if (TaXml::readString(xmlBuf, "current_path", _currentPath)) {
    cerr << "  ERROR - cannot find <current_path>" << endl;
    iret = -1;
  }
  if (TaXml::readLong(xmlBuf, "current_pos", _currentPos)) {
    cerr << "  ERROR - cannot find <current_pos>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "current_vol_num", _currentVolNum)) {
    cerr << "  ERROR - cannot find <current_vol_num>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "sweep_num_start_current_vol",
                     _sweepNumStartCurrentVol)) {
    cerr << "  ERROR - cannot find <sweep_num_start_current_vol>" << endl;
    iret = -1;
  }

  if (_params.debug) {
    cerr << "State:" << endl;
    cerr << xmlBuf << endl;
    cerr << "  _currentPath: " << _currentPath << endl;
    cerr << "  _currentPos: " << _currentPos << endl;
    cerr << "  _currentVolNum: " << _currentVolNum << endl;
    cerr << "  _sweepNumStartCurrentVol: " << _sweepNumStartCurrentVol << endl;
    cerr << endl;
  }

  return iret;

}

//////////////////////////////////////////////
// find the latest catalog path
//
// Returns 0 on success, -1 on failure

int SweepInfo2Spdb::_findLatestCatPath(string &latestPath,
                                       int &fileLen)
  
{

  bool found = false;
  time_t latestTime = 0;
  latestPath = "";

  // open the input dir
  
  DIR *dirp;
  if ((dirp = opendir(_params.catalog_dir)) == NULL) {
    int errNum = errno;
    cerr << "WARNING - SweepInfo2Spdb::_findLatestCatPath" << endl;
    cerr << "  Cannot open dir: " << _params.catalog_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // read through dir

  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    // exclude dir entries and files beginning with '.'
    
    if (dp->d_name[0] == '.') {
      continue;
    }

    // check for match on catalog name
    
    if (strncmp(dp->d_name,
                _params.sweep_catalog_name_root,
                strlen(_params.sweep_catalog_name_root)) == 0) {
      
      // compute path
      
      char path[MAX_PATH_LEN];
      sprintf(path, "%s%s%s", _params.catalog_dir, PATH_DELIM, dp->d_name);
      
      struct stat fileStat;
      if (ta_stat(path, &fileStat) == 0) {
        if (fileStat.st_mtime > latestTime) {
          latestTime = fileStat.st_mtime;
          latestPath = path;
          fileLen = fileStat.st_size;
          found = true;
        }
      }
      
    } // if (strncmp(dp->d_name ...

  } // dp
  
  closedir(dirp);

  if (found) {
    return 0;
  } else {
    return -1;
  }

}

//////////////////////////////////////////////
// Print info

void SweepInfo2Spdb::_printInfo(const sweep_info_packet_t &info,
                                ostream &out)

{
  
  out << "--------- SWEEP INFO -----------" << endl;
  out << "  msg id: " << info.id << endl;
  out << "  length: " << info.length << endl;
  out << "  time_secs: " << info.time_secs << endl;
  out << "  nano_secs: " << info.nano_secs << endl;
  out << "  scan_type: " << info.scan_type
       << " = " << DsRadarSweep::scanMode2Str(info.scan_type) << endl;
  out << "  volume_num: " << info.volume_num << endl;
  out << "  sweep_num: " << info.sweep_num << endl;
  out << "  end_of_sweep_flag: " << info.end_of_sweep_flag << endl;
  out << "  end_of_vol_flag: " << info.end_of_vol_flag << endl;
  out << "  cur_el: " << info.cur_el << endl;
  out << "  cur_az: " << info.cur_az << endl;
  out << "  fixed_el: " << info.fixed_el << endl;
  out << "  fixed_az: " << info.fixed_az << endl;
  if (info.scan_type == 8) {
    double elErr = info.fixed_el - info.cur_el;
    if (elErr < -180) {
      elErr += 360;
    } else if (elErr > 180) {
      elErr -= 360;
    }
    out << "  el_err: " << elErr << endl;
  }
  out << "  transition: " << info.transition << endl;
  out << "----------------------------------" << endl;

}

