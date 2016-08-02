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
// TsTcp2File.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// TsTcp2File reads data from TsTcpServer in RVP8 TimeSeries format
// and writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctime>
#include "TsTcp2File.hh"
#include <toolsa/udatetime.h>
#include <toolsa/uusleep.h>
#include <toolsa/pmu.h>

using namespace std;

// Constructor

TsTcp2File::TsTcp2File(int argc, char **argv)
  
{

  isOK = true;
  _iAqModePrev = -1;
  _nPulses = 0;
  _prevSeqNum = 0;
  _prevAz = -999;

  // set programe name
  
  _progName = "TsTcp2File";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // compute sector size etc for files

  _out = NULL;
  _needNewFile = true;
  _nPulsesFile = 0;
  _fileWritten = false;

  int numSectors = (int) (359.9 / _args.maxDegrees) + 1;
  if (numSectors < 2) {
    numSectors = 2;
  }
  _sectorWidth = 360.0 / numSectors;
  _currentSector = -1;

  // init process mapper registration

  if (_args.regWithProcmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _args.instance.c_str(),
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

TsTcp2File::~TsTcp2File()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int TsTcp2File::Run ()
{
  
  PMU_auto_register("Run");

  if (_args.verbose) {
    cerr << "Running TsTcp2File - verbose debug mode" << endl;
  } else if (_args.debug) {
    cerr << "Running TsTcp2File - debug mode" << endl;
  }

  int iret = 0;

  while (true) {

    PMU_auto_register("Opening socket");

    // open socket to server
    
    Socket sock;
    if (sock.open(_args.host.c_str(),
                  _args.port,
                  10000)) {
      if (sock.getErrNum() == Socket::TIMED_OUT) {
	if (_args.debug) {
	  cerr << "  Waiting for server to come up ..." << endl;
	}
      } else {
	if (_args.debug) {
	  cerr << "ERROR - TsTcp2File::Run" << endl;
	  cerr << "  Connecting to server" << endl;
	  cerr << "  " << sock.getErrStr() << endl;
	}
        iret = -1;
      }
      umsleep(1000);
      continue;
    }

    // read from the server

    if (_readFromServer(sock)) {
      iret = -1;
    }

    sock.close();

  } // while(true)

  return iret;
  
}

/////////////////////////////
// read data from the server

int TsTcp2File::_readFromServer(Socket &sock)

{

  // set number of pulses to 0

  _nPulses = 0;

  // close output file if open - if we have a new
  // connection we should start a new file

  _closeFile();

  // have we received the info?
  
  bool infoReceived = false;

  // read data
  
  while (true) {

    PMU_auto_register("Reading data");

    if (sock.readMessage(1000)) {
      if (sock.getErrNum() == Socket::TIMED_OUT) {
        PMU_auto_register("Timed out ...");
        continue;
      }
      cerr << "ERROR - TsTcp2File::_readFromServer" << endl;
      cerr << "  " << sock.getErrStr() << endl;
      return -1;
    }

    if (sock.getHeaderId() == PULSE_ID) {
      if (infoReceived) {
        if (_handlePulse(sock.getHeaderLength(),
                         sock.getData())) {
          return -1;
        }
      }
    } else if (sock.getHeaderId() == INFO_ID) {
      if (_handleInfo(sock.getHeaderLength(),
                      sock.getData())) {
        return -1;
      }
      infoReceived = true;
    } else {
      cerr << "ERROR - TsTcp2File::_readFromServer" << endl;
      cerr << "  Bad message id: " << sock.getHeaderId() << endl;
      return -1;
    }
    
  }

  return 0;

}

/////////////////////////////
// handle info packet

int TsTcp2File::_handleInfo(int len,
                            const void *buf)

{

  if (len != sizeof(_pulseInfo)) {
    cerr << "ERROR - TsTcp2File::_handleInfo" << endl;
    cerr << "  Info packet incorrect size" << endl;
    cerr << "  Packet len: " << len << endl;
    cerr << "  Should be: " << sizeof(_pulseInfo) << endl;
    return -1;
  }

  // copy it in

  memcpy(&_pulseInfo, buf, sizeof(_pulseInfo));

  return 0;

}

/////////////////////////////
// handle pulse packet

int TsTcp2File::_handlePulse(int len,
                             const void *buf)

{

  rvp8_pulse_hdr_t pulseHeader;
  if (len < (int) sizeof(pulseHeader)) {
    cerr << "ERROR - TsTcp2File::_handlePulse" << endl;
    cerr << "  Pulse packet too small" << endl;
    cerr << "  Packet len: " << len << endl;
    cerr << "  Header len: " << sizeof(pulseHeader) << endl;
    return -1;
  }

  // copy it in
  
  memcpy(&pulseHeader, buf, sizeof(pulseHeader));

  // check the iq data size
  
  int nChannels = pulseHeader.iVIQPerBin;
  int nIQ = pulseHeader.iNumVecs * 2;
  int nBytesPacked = nChannels * nIQ * sizeof(ui16);
  int nBytesMin = sizeof(pulseHeader) + nBytesPacked;

  if (len < nBytesMin) {
    cerr << "ERROR - TsTcp2File::_handlePulse" << endl;
    cerr << "  Pulse packet too small" << endl;
    cerr << "  Packet len: " << len << endl;
    cerr << "  Min length: " << nBytesMin << endl;
    return -1;
  }

  // set pointer to packed IQ data
  
  ui16 *iqPacked = (ui16 *) ((ui08 *) buf + sizeof(pulseHeader));
  
  // check if we need new file.
  // Returns -1 if we need to exit.
  // pulse info will be written to start of file
  
  if (_checkNewFile(pulseHeader)) {
    return -1;
  }
  
  // write pulse header to the file
  
  _writePulseHeader(pulseHeader);
  
  // write the packed iq data
  
  _writeIQ(iqPacked, nBytesPacked);

  _nPulses++;

  double az = (pulseHeader.iAz / 65535.0) * 360.0;
  double el = (pulseHeader.iEl / 65535.0) * 360.0;
  
  if (_args.verbose) {
    if ((_nPulses % 1000) == 0) {
      cerr << "El, az, npulses received: "
           << el << " " << az << " " << _nPulses << endl; 
    }
    if (_prevSeqNum > 0 &&
        pulseHeader.iSeqNum != (_prevSeqNum + 1)) {
      cerr << "Missing sequence numbers, prev: " << _prevSeqNum
           << ", this: " << pulseHeader.iSeqNum << endl;
    }
    if (_prevAz > -900) {
      double deltaAz = fabs(az - _prevAz);
      if (deltaAz >= 180) {
        deltaAz -= 360.0;
      }
      if (fabs(deltaAz) > 1.0) {
        cerr << "Missing azimuth, prev: " << _prevAz
             << ", this: " << az << endl;
      }
    }
  } // debug

  _prevSeqNum = pulseHeader.iSeqNum;
  _prevAz = az;
  
  return 0;

}

/////////////////////////////////////////
// Check if we need a new file
//
// Returns 0 to continue, -1 to exit

int TsTcp2File::_checkNewFile(const rvp8_pulse_hdr_t &pulseHeader)

{
  
  // pulse header is ready, do we need a new file?
  
  bool needNewFile = false;
  
  if(_iAqModePrev != pulseHeader.iAqMode) {
    if (_args.debug) {
      cerr << "\nNew (I,Q) acquisition mode: "
           << pulseHeader.iAqMode << endl;
    }
    _iAqModePrev = pulseHeader.iAqMode;
    needNewFile = true;
  }
  
  // compute azimuth and sector
  
  double az = (pulseHeader.iAz / 65536.0) * 360.0;
  int sector = (int) (az / _sectorWidth);
  if (sector != _currentSector && !_args.oneFile) {
    needNewFile = true;
    _currentSector = sector;
  }

  // do we have too many pulses in the file?
  
  _nPulsesFile++;
  if (_nPulsesFile > _args.maxPulses) {
    needNewFile = true;
  }

  // open a new file if needed
    
  if (needNewFile) {
      
    _openNewFile(pulseHeader);
    _nPulsesFile = 0;
      
    if (_fileWritten && _args.oneFile) {
      return -1;
    }

    // write pulse info to the file
    
    _writePulseInfo();

  }

  return 0;

}

/////////////////////////////////
// open a new file

int TsTcp2File::_openNewFile(const rvp8_pulse_hdr_t &pulseHeader)

{

  // close out old file

  if (_closeFile() == 0) {
    _fileWritten = true;
    if (_args.oneFile) {
      return -1;
    }
  }
  
  // get time and antenna position

  double ptime = ((double) pulseHeader.iTimeUTC +
                  pulseHeader.iMSecUTC / 1000.0);
  date_time_t ttime;
  ttime.unix_time = (time_t) ptime;
  uconvert_from_utime(&ttime);

  double az = (pulseHeader.iAz / 65536.0) * 360.0;
  double el = (pulseHeader.iEl / 65536.0) * 360.0;

  int iaz = (int) az;
  int iel = (int) (el * 10.0);
  
  // make the output dir

  char subdir[1024];
  sprintf(subdir, "%s/%.4d%.2d%.2d",
          _args.outDir.c_str(),
          ttime.year, ttime.month, ttime.day);
  
  if (_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - TsTcp2File" << endl;
    cerr << "  Cannot make output directory: " << subdir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute output path

  char path[1024];
  sprintf(path, "%s/%.4d%.2d%.2d_%.2d%.2d%.2d_%.3d_%.3d.tsarchive",
          subdir,
          ttime.year, ttime.month, ttime.day,
          ttime.hour, ttime.min, ttime.sec,
          iel, iaz);
  
  // open file

  if ((_out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - TsTcp2File" << endl;
    cerr << "  Cannot open output file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_args.debug) {
    cerr << "Opened new file: " << path << endl;
  }

  return 0;

}

///////////////////////////////////////////
// close file
//
// Returns 0 if file already open, -1 if not

int TsTcp2File::_closeFile()

{

  // close out old file
  
  if (_out != NULL) {
    fclose(_out);
    _out = NULL;
    return 0;
  }

  return -1;

}
  
////////////////////////
// write the pulse info

int TsTcp2File::_writePulseInfo()

{

  if (_out == NULL) {
    return -1;
  }

  fprintf(_out, "rvp8PulseInfo start\n");
  fprintf(_out, "iVersion=%d\n", _pulseInfo.iVersion);
  fprintf(_out, "iMajorMode=%d\n", _pulseInfo.iMajorMode);
  fprintf(_out, "iPolarization=%d\n", _pulseInfo.iPolarization);
  fprintf(_out, "iPhaseModSeq=%d\n", _pulseInfo.iPhaseModSeq);
  fprintf(_out, "taskID.iSweep=%d\n", _pulseInfo.taskID.iSweep);
  fprintf(_out, "taskID.iAuxNum=%d\n", _pulseInfo.taskID.iAuxNum);
  fprintf(_out, "taskID.sTaskName=%s\n", _pulseInfo.taskID.sTaskName);
  fprintf(_out, "sSiteName=%s\n", _pulseInfo.sSiteName);
  fprintf(_out, "iAqMode=%d\n", _pulseInfo.iAqMode);
  fprintf(_out, "iUnfoldMode=%d\n", _pulseInfo.iUnfoldMode);
  fprintf(_out, "iPWidthCode=%d\n", _pulseInfo.iPWidthCode);
  fprintf(_out, "fPWidthUSec=%g\n", _pulseInfo.fPWidthUSec);
  fprintf(_out, "fDBzCalib=%g\n", _pulseInfo.fDBzCalib);
  fprintf(_out, "iSampleSize=%d\n", _pulseInfo.iSampleSize);
  fprintf(_out, "iMeanAngleSync=%d\n", _pulseInfo.iMeanAngleSync);
  fprintf(_out, "iFlags=%d\n", _pulseInfo.iFlags);
  fprintf(_out, "iPlaybackVersion=%d\n", _pulseInfo.iPlaybackVersion);
  fprintf(_out, "fSyClkMHz=%g\n", _pulseInfo.fSyClkMHz);
  fprintf(_out, "fWavelengthCM=%g\n", _pulseInfo.fWavelengthCM);
  fprintf(_out, "fSaturationDBM=%g\n", _pulseInfo.fSaturationDBM);
  fprintf(_out, "fRangeMaskRes=%g\n", _pulseInfo.fRangeMaskRes);

  fprintf(_out, "iRangeMask=");
  for (int ii = 0; ii < USERMASKLEN; ii++) {
    fprintf(_out, "%d", _pulseInfo.iRangeMask[ii]);
    if (ii != USERMASKLEN - 1) {
      fprintf(_out, " ");
    }
  }
  fprintf(_out, "\n");
  
  fprintf(_out, "fNoiseDBm=%g %g\n",
          _pulseInfo.fNoiseDBm[0],
          _pulseInfo.fNoiseDBm[1]);
  fprintf(_out, "fNoiseStdvDB=%g %g\n",
          _pulseInfo.fNoiseStdvDB[0],
          _pulseInfo.fNoiseStdvDB[1]);
  
  fprintf(_out, "fNoiseRangeKM=%g\n", _pulseInfo.fNoiseRangeKM);
  fprintf(_out, "fNoisePRFHz=%g\n", _pulseInfo.fNoisePRFHz);

  fprintf(_out, "iGparmLatchSts=%d %d\n",
          _pulseInfo.iGparmLatchSts[0],
          _pulseInfo.iGparmLatchSts[1]);

  fprintf(_out, "iGparmImmedSts=%d %d %d %d %d %d\n",
          _pulseInfo.iGparmImmedSts[0],
          _pulseInfo.iGparmImmedSts[1],
          _pulseInfo.iGparmImmedSts[2],
          _pulseInfo.iGparmImmedSts[3],
          _pulseInfo.iGparmImmedSts[4],
          _pulseInfo.iGparmImmedSts[5]);

  fprintf(_out, "iGparmDiagBits=%d %d %d %d\n",
          _pulseInfo.iGparmDiagBits[0],
          _pulseInfo.iGparmDiagBits[1],
          _pulseInfo.iGparmDiagBits[2],
          _pulseInfo.iGparmDiagBits[3]);

  fprintf(_out, "sVersionString=%s\n", _pulseInfo.sVersionString);

  fprintf(_out, "rvp8PulseInfo end\n");

  return 0;

}

////////////////////////
// write the pulse header

int TsTcp2File::_writePulseHeader(const rvp8_pulse_hdr_t &pulseHeader)

{
  
  if (_out == NULL) {
    return -1;
  }

  fprintf(_out, "rvp8PulseHdr start\n");

  fprintf(_out, "iVersion=%d\n", pulseHeader.iVersion);
  fprintf(_out, "iFlags=%d\n", pulseHeader.iFlags);
  fprintf(_out, "iMSecUTC=%d\n", pulseHeader.iMSecUTC);
  fprintf(_out, "iTimeUTC=%d\n", pulseHeader.iTimeUTC);
  fprintf(_out, "iBtimeAPI=%d\n", pulseHeader.iBtimeAPI);
  fprintf(_out, "iSysTime=%d\n", pulseHeader.iSysTime);
  fprintf(_out, "iPrevPRT=%d\n", pulseHeader.iPrevPRT);
  fprintf(_out, "iNextPRT=%d\n", pulseHeader.iNextPRT);
  fprintf(_out, "iSeqNum=%d\n", pulseHeader.iSeqNum);
  fprintf(_out, "iAqMode=%d\n", pulseHeader.iAqMode);
  fprintf(_out, "iPolarBits=%d\n", pulseHeader.iPolarBits);
  fprintf(_out, "iTxPhase=%d\n", pulseHeader.iTxPhase);
  fprintf(_out, "iAz=%d\n", pulseHeader.iAz);
  fprintf(_out, "iEl=%d\n", pulseHeader.iEl);
  fprintf(_out, "iNumVecs=%d\n", pulseHeader.iNumVecs);
  fprintf(_out, "iMaxVecs=%d\n", pulseHeader.iMaxVecs);
  fprintf(_out, "iVIQPerBin=%d\n", pulseHeader.iVIQPerBin);
  fprintf(_out, "iTgBank=%d\n", pulseHeader.iTgBank);
  fprintf(_out, "iTgWave=%d\n", pulseHeader.iTgWave);

  fprintf(_out, "uiqPerm.iLong=%d %d\n",
          pulseHeader.uiqPerm.iLong[0],
          pulseHeader.uiqPerm.iLong[1]);

  fprintf(_out, "uiqOnce.iLong=%d %d\n",
          pulseHeader.uiqOnce.iLong[0],
          pulseHeader.uiqOnce.iLong[1]);

  fprintf(_out, "RX[0].fBurstMag=%g\n", pulseHeader.Rx[0].fBurstMag);
  fprintf(_out, "RX[0].iBurstArg=%d\n", pulseHeader.Rx[0].iBurstArg);
  fprintf(_out, "RX[1].fBurstMag=%g\n", pulseHeader.Rx[1].fBurstMag);
  fprintf(_out, "RX[1].iBurstArg=%d\n", pulseHeader.Rx[1].iBurstArg);

  fprintf(_out, "rvp8PulseHdr end\n");

  return 0;

}

////////////////////////
// write the pulse data

int TsTcp2File::_writeIQ(void *iqPacked, int nBytesPacked)

{
  
  if (_out == NULL) {
    return -1;
  }

  if ((int) fwrite(iqPacked, 1, nBytesPacked, _out) != nBytesPacked) {
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////
// Directory creation routines

///////////////////////////////////////////////////////
// makedir()
//
// Utility routine to create a directory.  If the directory
// already exists, does nothing.
//
// Returns -1 on error, 0 otherwise.

int TsTcp2File::_makedir(const char *path)
{
  
  struct stat stat_buf;
  
  // Status the directory to see if it already exists.

  if (stat(path, &stat_buf) == 0) {
    return 0;
  }
  
  // Directory doesn't exist, create it.

  if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
    return -1;
  }
  
  return 0;

}


///////////////////////////////////////////////////////
// makedir_recurse()
//
// Utility routine to create a directory recursively.
// If the directory already exists, does nothing.
// Otherwise it recurses through the path, making all
// needed directories.
//
// Returns -1 on error, 0 otherwise.

int TsTcp2File::_makedir_recurse(const char *path)
{

  int delim = '/';
  
  // Status the directory to see if it already exists.
  // '/' dir will always exist, so this stops the recursion
  // automatically.
  
  struct stat dir_stat;
  if (stat(path, &dir_stat) == 0) {
    return 0;
  }
  
  // create up dir - one up the directory tree -
  // by searching for the previous delim and removing it
  // from the string.
  // If no delim, try to make the directory non-recursively.
  
  char up_dir[1024];
  strncpy(up_dir, path, 1024);
  char *last_delim = strrchr(up_dir, delim);
  if (last_delim == NULL) {
    return (_makedir(up_dir));
  }
  *last_delim = '\0';
  
  // make the up dir
  
  if (_makedir_recurse(up_dir)) {
    return -1;
  }
  
  // make this dir

  if (_makedir(path)) {
    return -1;
  } else {
    return 0;
  }

}

