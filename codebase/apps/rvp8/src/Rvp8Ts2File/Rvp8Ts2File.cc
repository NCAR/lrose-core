// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:29:54 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// Rvp8Ts2File.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Rvp8Ts2File reads raw LIRP IQ files and converts them
// to NetDCF
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <rvp8_rap/udatetime.h>
#include "Rvp8Ts2File.hh"

using namespace std;

// Constructor

Rvp8Ts2File::Rvp8Ts2File(int argc, char **argv)
  
{

  isOK = true;

  // set programe name
  
  _progName = "Rvp8Ts2File";

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

  // allocate array for packed data

  iTempPackedIQ = new UINT2[RECORD_SIZE];

  return;
  
}

// destructor

Rvp8Ts2File::~Rvp8Ts2File()

{
  if (iTempPackedIQ) {
    delete[] iTempPackedIQ;
  }
}

//////////////////////////////////////////////////
// Run

int Rvp8Ts2File::Run ()
{
  
  if (_args.verbose) {
    cerr << "Running Rvp8Ts2File - verbose debug mode" << endl;
  } else if (_args.debug) {
    cerr << "Running Rvp8Ts2File - debug mode" << endl;
  }

  // attach to RVP8

  _iAqModePrev = -1;
  MESSAGE status;
#ifdef RVP8_LEGACY_V8
  RvpTS *ts = rvp8tsAttach_(&status,
			    RVP8TS_UNIT_MAIN,
			    RVP8TS_CLIENT_READER);
#else
  RvpTS *ts = rvptsAttach(&status,
			  RVPTS_UNIT_MAIN,
			  RVPTS_CLIENT_READER, 0);
#endif
  
  if (ts == NULL) {
    cerr << "ERROR - Rvp8Ts2File::Run" << endl;
    cerr << "  Could not attach to RVP8 TS interface" << endl;
    sig_signal(status);
    return -1;
  }

  // get an initial sequence number into the pulse sequence

  int seqNum = rvptsCurrentSeqNum(ts);
  bool infoRead = false;

  while (true) {
    
    // wait until data is available
    
    _waitAvailable(ts, seqNum);
    
    // get pulse info if we have not yet done so
    
    if (!infoRead) {
      _pulseInfo = *(rvptsCurrentPulseInfo(ts));
      infoRead = true;
    }
    
    // read available pulses, up to number expected
    
    _readPulses(ts, seqNum);

    if (_fileWritten && _args.oneFile) {
      return 0;
    }

  } // while
  

  return 0;

}

//////////////////////////////////////////////////
// wait for available ts data from the RVP8

void Rvp8Ts2File::_waitAvailable(RvpTS *ts, int &seqNum)
  
{
  
  while (!rvptsSeqNumOkay(ts, seqNum)) {
    
    if (rvptsSeqNumOkay(ts, seqNum-1)) {
      
      /* If the sequence number is not okay but the previous one is,
       * then we have completely caught up with the RVP8's supply of
       * (I,Q) data.  There's no particular rush to resume, so sleep
       * long enough for others to get some work done.
       */
      
      sig_microSleep(100000) ;
      
    } else {
  
      /* If our current sequence number no longer represents a valid
       * window into the timeseries data, then print a message and
       * get a fresh sequence number with which to resume.
       */
      
      if (_args.debug) {
        cerr << "Expired seqNum: " << seqNum << endl;
        cerr << "  Resetting ..." << endl;
      }
      seqNum = rvptsCurrentSeqNum(ts) ;

    }

  } // while

  /* We now have at least one valid pulse to read.  Check that it
   * would actually be worthwhile to run right now, i.e., we have
   * either a minimum number of pulses, or the data are starting
   * to get too old.  If neither of these apply, then wait a
   * little while.
   */
  
  while (true) {
    
    SINT4 iFwdPulses = rvptsNumPulsesFwd(ts, seqNum);
    UINT4 iAgeMS =  rvptsRealTimeAge(ts, seqNum);
    
    if((iFwdPulses < 25) && (iAgeMS < 250)) {
      sig_microSleep(20000) ;
    } else {
      break;
    }

  } // while
    
}
    
//////////////////////////////////////////////////
// read available ts data from the RVP8
//
// Returns the number of samples read

int Rvp8Ts2File::_readPulses(RvpTS *ts, int &seqNum)
  
{
  
  // get as many pulses as we can
  // up to the maximum imposed by our local buffer size
  
  const struct rvptsPulseHdr *pHdrs[MaxPulses];
  SINT4 nPulsesAvail = rvptsGetPulses(ts, pHdrs, seqNum, MaxPulses);
  if (nPulsesAvail == 0) {
    return -1;
  }
  
  // If the acquisition mode has changed, save the pulse info
  
  if(_iAqModePrev != pHdrs[0]->iAqMode) {
    if (_args.debug) {
      cerr << "\nNew (I,Q) acquisition mode: " << pHdrs[0]->iAqMode << endl;
    }
    _iAqModePrev = pHdrs[0]->iAqMode;
    _pulseInfo = *(rvptsGetPulseInfo(ts, pHdrs[0]));
    _needNewFile = true;
  }

  // loop through the pulses
  
  for (int ipulse = 0; ipulse < nPulsesAvail; ipulse++, seqNum++) {
    
    struct rvptsPulseHdr pulseHdr = *(pHdrs[ipulse]);
    
    // check the sequence number

    if (seqNum != pulseHdr.iSeqNum) {
      if (_args.debug) {
        cerr << "ERROR in sequence number, resetting" << endl;
        cerr << "  Expecting: " << seqNum << endl;
        cerr << "  Found: " << pulseHdr.iSeqNum << endl;
      }
      seqNum = pulseHdr.iSeqNum;
    }

    // compute azimuth and sector

    double az = (pulseHdr.iAz / 65536.0) * 360.0;
    int sector = (int) (az / _sectorWidth);
    if (sector != _currentSector && !_args.oneFile) {
      _needNewFile = true;
      _currentSector = sector;
    }

    // do we have too many pulses in the file?

    _nPulsesFile++;
    if (_nPulsesFile > _args.maxPulses) {
      _needNewFile = true;
    }

    // open a new file if needed
    
    if (_needNewFile) {

      _openNewFile(pulseHdr);
      _needNewFile = false;
      _nPulsesFile = 0;
      
      if (_fileWritten && _args.oneFile) {
        return 0;
      }

      // write pulse info to the file
      
      _writePulseInfo();
  
    }

    // write the pulse header

    _writePulseHeader(pulseHdr);

    // write pulse data

    _writePulseData(ts, pulseHdr);

    // check that this sequence number is still OK, it may have been
    // overwritten by the RVP8 if we are too slow
    
    if (!rvptsSeqNumOkay(ts, seqNum)) {
      if (_args.debug) {
        cerr << "WARNING - sequence number expired: " << seqNum << endl;
        cerr << "  Getting new sequence number" << endl;
      }
      seqNum = rvptsCurrentSeqNum(ts);
      return -1;
    }
    
  } // ipulse
  
  return 0;

}

/////////////////////////////////
// open a new file

int Rvp8Ts2File::_openNewFile(const struct rvptsPulseHdr &pulseHdr)

{

  // close out old file
  
  if (_out != NULL) {
    fclose(_out);
    _out = NULL;
    _fileWritten = true;
    if (_args.oneFile) {
      return -1;
    }
  }
  
  // get time and antenna position

  double ptime = (double) pulseHdr.iTimeUTC + pulseHdr.iMSecUTC / 1000.0;
  date_time_t ttime;
  ttime.unix_time = (time_t) ptime;
  uconvert_from_utime(&ttime);

  double az = (pulseHdr.iAz / 65536.0) * 360.0;
  double el = (pulseHdr.iEl / 65536.0) * 360.0;

  int iaz = (int) az;
  int iel = (int) (el * 10.0);
  
  // make the output dir

  char subdir[1024];
  sprintf(subdir, "%s/%.4d%.2d%.2d",
          _args.outDir.c_str(),
          ttime.year, ttime.month, ttime.day);
  
  if (_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - Rvp8Ts2File" << endl;
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
    cerr << "ERROR - Rvp8Ts2File" << endl;
    cerr << "  Cannot open output file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_args.debug) {
    cerr << "Opened new file: " << path << endl;
  }

  return 0;

}

////////////////////////
// write the pulse info

int Rvp8Ts2File::_writePulseInfo()

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

int Rvp8Ts2File::_writePulseHeader(const rvptsPulseHdr &pulseHdr)

{
  
  if (_out == NULL) {
    return -1;
  }

  fprintf(_out, "rvp8PulseHdr start\n");

  fprintf(_out, "iVersion=%d\n", pulseHdr.iVersion);
  fprintf(_out, "iFlags=%d\n", pulseHdr.iFlags);
  fprintf(_out, "iMSecUTC=%d\n", pulseHdr.iMSecUTC);
  fprintf(_out, "iTimeUTC=%d\n", pulseHdr.iTimeUTC);
  fprintf(_out, "iBtimeAPI=%d\n", pulseHdr.iBtimeAPI);
  fprintf(_out, "iSysTime=%d\n", pulseHdr.iSysTime);
  fprintf(_out, "iPrevPRT=%d\n", pulseHdr.iPrevPRT);
  fprintf(_out, "iNextPRT=%d\n", pulseHdr.iNextPRT);
  fprintf(_out, "iSeqNum=%d\n", pulseHdr.iSeqNum);
  fprintf(_out, "iAqMode=%d\n", pulseHdr.iAqMode);
  fprintf(_out, "iPolarBits=%d\n", pulseHdr.iPolarBits);
  fprintf(_out, "iTxPhase=%d\n", pulseHdr.iTxPhase);
  fprintf(_out, "iAz=%d\n", pulseHdr.iAz);
  fprintf(_out, "iEl=%d\n", pulseHdr.iEl);
  fprintf(_out, "iNumVecs=%d\n", pulseHdr.iNumVecs);
  fprintf(_out, "iMaxVecs=%d\n", pulseHdr.iMaxVecs);
  fprintf(_out, "iVIQPerBin=%d\n", pulseHdr.iVIQPerBin);
  fprintf(_out, "iTgBank=%d\n", pulseHdr.iTgBank);
  fprintf(_out, "iTgWave=%d\n", pulseHdr.iTgWave);

  fprintf(_out, "uiqPerm.iLong=%d %d\n",
          pulseHdr.uiqPerm.iLong[0],
          pulseHdr.uiqPerm.iLong[1]);

  fprintf(_out, "uiqOnce.iLong=%d %d\n",
          pulseHdr.uiqOnce.iLong[0],
          pulseHdr.uiqOnce.iLong[1]);

  fprintf(_out, "RX[0].fBurstMag=%g\n", pulseHdr.Rx[0].fBurstMag);
  fprintf(_out, "RX[0].iBurstArg=%d\n", pulseHdr.Rx[0].iBurstArg);
  fprintf(_out, "RX[1].fBurstMag=%g\n", pulseHdr.Rx[1].fBurstMag);
  fprintf(_out, "RX[1].iBurstArg=%d\n", pulseHdr.Rx[1].iBurstArg);

  fprintf(_out, "rvp8PulseHdr end\n");

  return 0;

}

////////////////////////
// write the pulse data

int Rvp8Ts2File::_writePulseData(RvpTS *ts,
				 const rvptsPulseHdr &pulseHdr)

{
  
  if (_out == NULL) {
    return -1;
  }

  int nChannels = pulseHdr.iVIQPerBin;
  int iret = 0;

  // loop through channels

  for (int ichan = 0; ichan < nChannels; ichan++) {

    // get floating point IQ data

    FLT4 *iqFloat =  rvptsIQDataFLT4(ts, &pulseHdr, ichan);
    
    // allocate memory for packed data
    
    int nVals = pulseHdr.iNumVecs * 2;

    // pack the data

    vecPackIQFromFloatIQ_(iTempPackedIQ, iqFloat,
                          (2 * nVals),  PACKIQ_HIGHSNR);

    // write it to file

    if (fwrite(iTempPackedIQ, 2, nVals, _out) != nVals) {
      iret = -1;
    }

  } // ichan

  return iret;

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

int Rvp8Ts2File::_makedir(const char *path)
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

int Rvp8Ts2File::_makedir_recurse(const char *path)
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
