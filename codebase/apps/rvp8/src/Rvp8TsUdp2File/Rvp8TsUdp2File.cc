// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 2006 
// ** University Corporation for Atmospheric Research(UCAR) 
// ** National Center for Atmospheric Research(NCAR) 
// ** Research Applications Laboratory(RAL) 
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA 
// ** 2006/9/5 14:33:20 
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
///////////////////////////////////////////////////////////////
// Rvp8TsUdp2File.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// Rvp8TsUdp2File reads data from UDP in RVP8 TimeSeries format and
// writes it to files in TsArchive format
//
////////////////////////////////////////////////////////////////
//
// Based on /usr/sigmet/ts/export/tsimport.C
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>  /* place after netinet/in.h on some platforms */
#include <ctime>
#include <rvp8_rap/udatetime.h>
#include <rvp8_rap/pmu.h>
#include "Rvp8TsUdp2File.hh"

using namespace std;

// Constructor

Rvp8TsUdp2File::Rvp8TsUdp2File(int argc, char **argv)
  
{

  isOK = true;
  iTempPackedIQ = NULL;

  // set programe name
  
  _progName = "Rvp8TsUdp2File";

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

  // initialize state

  memset(&State, 0, sizeof(State));
  State.iPulseState = PS_WAITING;
  State.lNewPulseInfo = TRUE;
  State.lWasWaiting = TRUE;

  // allocate array for packed data

  iTempPackedIQ = new UINT1[RECORD_SIZE];

  // init process mapper registration

  if (_args.regWithProcmap) {
    PMU_auto_init((char *) _progName.c_str(),
                  _args.instance.c_str(),
                  PROCMAP_REGISTER_INTERVAL);
  }
  
  return;
  
}

// destructor

Rvp8TsUdp2File::~Rvp8TsUdp2File()

{

  // free up

  if (iTempPackedIQ) {
    delete[] iTempPackedIQ;
  }

  // unregister process

  PMU_auto_unregister();
}

//////////////////////////////////////////////////
// Run

int Rvp8TsUdp2File::Run ()
{
  
  PMU_auto_register("Run");

  if (_args.verbose) {
    cerr << "Running Rvp8TsUdp2File - verbose debug mode" << endl;
  } else if (_args.debug) {
    cerr << "Running Rvp8TsUdp2File - debug mode" << endl;
  }

  // Flush the client socket address
  
  struct sockaddr_in dst_addr;
  memset(&dst_addr, 0, sizeof(struct sockaddr_in));
  
  // Open a socket that uses UDP protocol that we will receive from

  int iSocket;
  if((iSocket =socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "Error opening socket\n");
    perror("socket");
    return -1;
  }
  
  // set the socket for reuse

  int iVal=1;
  socklen_t iValLen=sizeof(iVal);
  int iError = setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR,
                          (char *)&iVal, iValLen);
  if (iError<0) {
    perror("setsockopt for reuse");
    return -1;exit(EXIT_FAILURE);
  }

  // set the receive buffer to 2M

  iVal = 2097152;
  iError = setsockopt(iSocket, SOL_SOCKET, SO_RCVBUF,
                      (char *)&iVal, iValLen);
  if (iError<0) {
    perror("setsockopt for receive buffer size");
    return -1;exit(EXIT_FAILURE);
  }

  // confirm the receive buffer len

  iError = getsockopt(iSocket, SOL_SOCKET, SO_RCVBUF,
                      (char *)&iVal, &iValLen);
  if (iError<0) {
    perror("getsockopt for receive buffer size");
    return -1;exit(EXIT_FAILURE);
  }

  if (_args.debug) {
    cerr << "Socket receive buffer len: " << iVal << endl;
  }

  dst_addr.sin_family = AF_INET;
  dst_addr.sin_port = htons(_args.port);
  dst_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the socket, making this address known to the system 

  PMU_auto_register("Before bind");

  if(bind(iSocket,
          (struct sockaddr *)(void*)&dst_addr, sizeof(dst_addr)) < 0) {
    fprintf(stderr,
            "Error binding UDP socket {%d, %d, %s}, errno=%d\n",
            dst_addr.sin_family, ntohs(dst_addr.sin_port),
            inet_ntoa(dst_addr.sin_addr), errno);
    return -1;
  }

  // read from UDP stream

  _iAqModePrev = -1;
  UINT1 Buffer[UDP_DATA_SIZE];
  
  while (true) {
    
    fd_set rset_c;
    FD_ZERO(&rset_c);
    FD_SET(iSocket, &rset_c);
    struct timeval wait;
    wait.tv_sec = 1;
    wait.tv_usec = 0;
    
    PMU_auto_register("Waiting for data");

    if (select(iSocket+1, &rset_c, NULL, NULL, &wait) > 0) {
    
      struct sockaddr_in from_name;
      socklen_t fromlen = sizeof(from_name);

      int recv_bytes =
        recvfrom(iSocket, Buffer, UDP_DATA_SIZE, 0,
                 (struct sockaddr *)(void*)&from_name, &fromlen);
      
      if (recv_bytes > 0) {
        if (ProcessUdpPacket(Buffer, recv_bytes)) {
          return 0;
        }
      }

    } // select

  } // while
  
  return 0;

}

/* ===================================================
 * Process a buffer received from the txexport.
 *
 * Returns -1 to exit, 0 to continue
 */

int Rvp8TsUdp2File::ProcessUdpPacket
  (const UINT1 Buffer_a[UDP_DATA_SIZE],   /* Data packet */
   int iRecvBytes_a                       /* Number of bytes in the packet */
   )

{

  /* Packet must be long enough to at least contain the header string */
  if (iRecvBytes_a<10) {
    if (_args.verbose) {
      fprintf(stderr, "\nUnknown packet at: %8.8x\n",
	      PulseHeader.iSeqNum);
    }
    State.iUnknownCnt++;
    return 0;
  }
  
  /* The pulse header packet consists of the string "PULSE HEAD" followed
   * by a 0 followed by an uninitialized byte followed by the structure
   * rvp8PulseHdr.
   */
  
  if ((0 == strncmp("PULSE HEAD", (const char *)Buffer_a, 10)) &&
      (iRecvBytes_a == (sizeof(struct rvptsPulseHdr) + 12))) {
    ProcessHeadPacket(Buffer_a, iRecvBytes_a);
    State.iHdrCnt++;
    //     if (State.iPulseState == PS_PHDR_DONE) {
    //       if (_checkNewFile()) {
    //         return -1;
    //       }
    //     }
  } else if (0 == strncmp("PULSE INF", (const char *)Buffer_a, 9)) {
    ProcessInfoPacket(Buffer_a, iRecvBytes_a);
  } else if (0 == strncmp("DATA", (const char *)Buffer_a, 4)) {
    if (ProcessDataPacket(Buffer_a+4, iRecvBytes_a-4)) {
      return -1;
    }
    State.iDataCnt++;
  } else {
    if (_args.verbose) {
      fprintf(stderr, "\nUnknown packet at: %8.8x\n",
	      PulseHeader.iSeqNum);
    }
    State.iUnknownCnt++;
  }

  return 0;
  
}

/* =========================================================
 * Process a packet containing the pulse header structure 
 */
void Rvp8TsUdp2File::ProcessHeadPacket(const UINT1 Buffer_a[],
                                   int iRecvBytes_a)
  
{
  /* Ignore the new pulse header until we have a valid info */

  if (State.iPulseState==PS_WAITING) return;
  if (State.iPulseState==PS_PINFO_STARTED) return;

  /* we have a pulse header marker, so process a pulse header */
  memcpy(&PulseHeader, Buffer_a+11, sizeof(struct rvptsPulseHdr));
  
  /* Diagnostic printout for missing rays, indicating why */
  if (_args.verbose) {
    if (PulseHeader.iSeqNum != State.iLastSeqNum + 1) {
      fprintf(stderr,
	      "Ray sequence, got: %8.8x, wanted: %8.8x, skip count: %ld\n",
	      PulseHeader.iSeqNum, State.iLastSeqNum + 1,
              State.iOutOfSeqCnt+1);
    } else if (PulseHeader.iFlags & PHDRFLG_DATAGAP) {
      fprintf(stderr, "\nGap at: %8.8x\n", PulseHeader.iSeqNum);
    }
  }
  
  /* Mark a data gap if there is a skipped sequence number */
  if (PulseHeader.iSeqNum != State.iLastSeqNum + 1) {
    PulseHeader.iFlags |= PHDRFLG_DATAGAP;
    State.iOutOfSeqCnt++;
  }
  State.iLastSeqNum = PulseHeader.iSeqNum;
  
  /* If the Acq mode has changed, then we must wait for a new
   * pulse info structure.
   */
  if (PulseHeader.iAqMode != PulseInfo.iAqMode) {
    if (State.iPulseState != PS_WAITING) {
      if (_args.verbose) {
	fprintf(stderr, "\nInfo Skip at: %8.8x\n", PulseHeader.iSeqNum);
      }
    }
    State.iPulseState = PS_WAITING;
  } else {
    State.iPulseState = PS_PHDR_DONE;
  }

}
      
/* =======================================================
 * Process the pulse info packets 
 */
void Rvp8TsUdp2File::ProcessInfoPacket(const UINT1 Buffer_a[],
                                   int iRecvBytes_a)

{

  if (('1' == Buffer_a[9]) && (iRecvBytes_a == UDP_DATA_SIZE)) {

    /* We have a pulse info 1 marker, so process it.
     * These are always welcome. */

    memcpy(&PulseInfo, &Buffer_a[11], UDP_DATA_SIZE - 11);
    State.lWasWaiting = (State.iPulseState==PS_WAITING);
    State.iPulseState = PS_PINFO_STARTED;
    State.iPInf1Cnt++;
    
  } else if (('2' == Buffer_a[9]) && 
             (iRecvBytes_a == (sizeof(struct rvptsPulseInfo) +
                               11 - UDP_DATA_SIZE))) {
    
    memset(&lastPulseInfo, 0, sizeof(lastPulseInfo));
    
    /* We have a pulse info 2 marker, so process it.
     * Only valid if just after pulse info 1.
     */
    
    if (State.iPulseState == PS_PINFO_STARTED) {
      
      memcpy(((UINT1*)&PulseInfo) + UDP_DATA_SIZE - 11,
             &Buffer_a[11], 
             sizeof(struct rvptsPulseInfo) - UDP_DATA_SIZE - 11);

      if (State.lWasWaiting ||
          memcmp(&PulseInfo, &lastPulseInfo,
                  sizeof(struct rvptsPulseInfo))) {

        memcpy(&lastPulseInfo, &PulseInfo,
                sizeof(struct rvptsPulseInfo));
        State.lNewPulseInfo = TRUE;
      }

      State.iPulseState = PS_PINFO_DONE;

    } else { // if (State ... 
      
      if (_args.verbose) {
	fprintf(stderr, "\nMissing pulse info fragment\n");
      }

    } // if (State ... 

    State.iPInf2Cnt++;

  } else { // else if (('2' ...
    
    if (_args.verbose) {
      fprintf(stderr, "\nBad pulse info fragment\n");
    }
  
  } // if (('1' ...

}

/* =======================================================
 * Process a data packet.
 *
 * Return 0 to continue, -1 to exit
 */

int Rvp8TsUdp2File::ProcessDataPacket(const UINT1 Buffer_a[], int iRecvBytes_a)

{

  int lDone=FALSE;

  switch(State.iPulseState) {

    case PS_PHDR_DONE:
      memset(&PacketState, 0, sizeof(PacketState));
    case PS_DATA_STARTED:
      lDone= AssemblePacket(iTempPackedIQ, RECORD_SIZE, Buffer_a,
                            iRecvBytes_a);
      State.iPulseState = PS_DATA_STARTED;
      break;
      
    case PS_WAITING:
      break;
      
    default:
      if (_args.verbose) {
	fprintf(stderr, "\nData Skip %5.5s at: %8.8x, %d\n", 
		(const char*)Buffer_a, PulseHeader.iSeqNum, 
		State.iPulseState);
      }
      break;
  }
  
  if (lDone) {

    SINT2 iIQCount = PulseHeader.iNumVecs*2;  /* In a single channel */
    SINT2 iIQCountTotal;

    if (PulseHeader.iVIQPerBin>1) {
      iIQCountTotal = 2*iIQCount;
    } else {
      iIQCountTotal = iIQCount;
    }
    
    if (PacketState.iLastOffset == (int)(iIQCountTotal * sizeof(UINT2))) {

      FLT4 *pIQFlt; UINT2 *pIQPacked;
      UINT4 iPackFlags =
        PACKIQ_HIGHSNR | ((0x1234 == htons(0x1234) ? PACKIQ_BYTESWAP : 0));
      State.iPulseState = PS_DATA_DONE;
      if (State.lNewPulseInfo) {
        State.lNewPulseInfo = FALSE;
      }

      // check if we need new file. returns -1 if we need to exit
      // pulse info will be written to start of file

      if (_checkNewFile()) {
        return -1;
      }
      
      // write pulse header to the file
      
      _writePulseHeader();

      // write the pulse data

      _writePulseData();

      if (_args.verbose) {
        debugLoopCount = 0;
        debugLoopCount++;
        debugLoopCount %= 25;
        if (debugLoopCount == 0)
	  if (_args.verbose) {
	    fprintf(stderr, 
		    "\rHdr Pkts: %ld, PInf1 %ld, PInf2 %ld, IQ Pkts %ld, bytes: %d", 
		    State.iHdrCnt, State.iPInf1Cnt, State.iPInf2Cnt, 
		    State.iDataCnt, iRecvBytes_a);
	  }
      } // debug
      
    } else {
      
      if (_args.verbose) {
	fprintf(stderr, "\nBad pulse data size at: %8.8x\n",
		PulseHeader.iSeqNum);
      }
      State.iPulseState = PS_PINFO_DONE;

    }

  }

  return 0;

}

/* =============================================================
 * Assemble a record from a series of individual packets.  The individual
 * packets start with 5 chars containing the packet index and the number
 * of expected packets.  Returns TRUE if done.
 */

int Rvp8TsUdp2File::AssemblePacket(UINT1 iRecord_a[],
                               int iMaxRecordSize_a,
                               const UINT1 iPacket_a[], 
                               int iPacketSize_a)
{

  char sTemp[4];
  memcpy(sTemp, iPacket_a, 2);
  sTemp[3] = 0;
  int iIndex = atoi(sTemp);
  memcpy(sTemp, iPacket_a+3, 2);
  int iCount = atoi(sTemp);
  int iSize = iPacketSize_a-5;
  
  if ((iIndex<=iCount)&&(iIndex==(PacketState.iLastIndex+1))) {
    
    if ((PacketState.iLastOffset+iSize)<=iMaxRecordSize_a) {

      memcpy(iRecord_a+PacketState.iLastOffset, iPacket_a+5, iSize);
      PacketState.iLastOffset+=iSize;
      PacketState.iLastIndex  =iIndex;
      if (iIndex==iCount) {
        return TRUE;
      }
      
    } else {

      if (_args.verbose) {
	fprintf(stderr, "\nPacket overlay got: %d byte in packet:%d\n", 
		PacketState.iLastOffset+iSize, iIndex);
      }
      memset(&PacketState, 0, sizeof(PacketState));

    }
    
  } else {
    
    if (_args.verbose) {
      fprintf(stderr, "\nPacket sequence got: %d, wanted:%d\n", 
	      iIndex, PacketState.iLastIndex+1);
    }
    memset(&PacketState, 0, sizeof(PacketState));
    
  }

  return FALSE;

}

/////////////////////////////////////////
// Check if we need a new file
//
// Returns 0 to continue, -1 to exit

int Rvp8TsUdp2File::_checkNewFile()

{
  
  // pulse header is ready, do we need a new file?
  
  bool needNewFile = false;
  
  if(_iAqModePrev != PulseHeader.iAqMode) {
    if (_args.debug) {
      cerr << "\nNew (I,Q) acquisition mode: "
           << PulseHeader.iAqMode << endl;
    }
    _iAqModePrev = PulseHeader.iAqMode;
    needNewFile = true;
  }
  
  // compute azimuth and sector
  
  double az = (PulseHeader.iAz / 65536.0) * 360.0;
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
      
    _openNewFile();
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

int Rvp8TsUdp2File::_openNewFile()

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

  double ptime = ((double) PulseHeader.iTimeUTC +
                  PulseHeader.iMSecUTC / 1000.0);
  date_time_t ttime;
  ttime.unix_time = (time_t) ptime;
  uconvert_from_utime(&ttime);

  double az = (PulseHeader.iAz / 65536.0) * 360.0;
  double el = (PulseHeader.iEl / 65536.0) * 360.0;

  int iaz = (int) az;
  int iel = (int) (el * 10.0);
  
  // make the output dir

  char subdir[1024];
  sprintf(subdir, "%s/%.4d%.2d%.2d",
          _args.outDir.c_str(),
          ttime.year, ttime.month, ttime.day);
  
  if (_makedir_recurse(subdir)) {
    int errNum = errno;
    cerr << "ERROR - Rvp8TsUdp2File" << endl;
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
    cerr << "ERROR - Rvp8TsUdp2File" << endl;
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

int Rvp8TsUdp2File::_writePulseInfo()

{

  if (_out == NULL) {
    return -1;
  }

  fprintf(_out, "rvp8PulseInfo start\n");
  fprintf(_out, "iVersion=%d\n", PulseInfo.iVersion);
  fprintf(_out, "iMajorMode=%d\n", PulseInfo.iMajorMode);
  fprintf(_out, "iPolarization=%d\n", PulseInfo.iPolarization);
  fprintf(_out, "iPhaseModSeq=%d\n", PulseInfo.iPhaseModSeq);
  fprintf(_out, "taskID.iSweep=%d\n", PulseInfo.taskID.iSweep);
  fprintf(_out, "taskID.iAuxNum=%d\n", PulseInfo.taskID.iAuxNum);
  fprintf(_out, "taskID.sTaskName=%s\n", PulseInfo.taskID.sTaskName);
  fprintf(_out, "sSiteName=%s\n", PulseInfo.sSiteName);
  fprintf(_out, "iAqMode=%d\n", PulseInfo.iAqMode);
  fprintf(_out, "iUnfoldMode=%d\n", PulseInfo.iUnfoldMode);
  fprintf(_out, "iPWidthCode=%d\n", PulseInfo.iPWidthCode);
  fprintf(_out, "fPWidthUSec=%g\n", PulseInfo.fPWidthUSec);
  fprintf(_out, "fDBzCalib=%g\n", PulseInfo.fDBzCalib);
  fprintf(_out, "iSampleSize=%d\n", PulseInfo.iSampleSize);
  fprintf(_out, "iMeanAngleSync=%d\n", PulseInfo.iMeanAngleSync);
  fprintf(_out, "iFlags=%d\n", PulseInfo.iFlags);
  fprintf(_out, "iPlaybackVersion=%d\n", PulseInfo.iPlaybackVersion);
  fprintf(_out, "fSyClkMHz=%g\n", PulseInfo.fSyClkMHz);
  fprintf(_out, "fWavelengthCM=%g\n", PulseInfo.fWavelengthCM);
  fprintf(_out, "fSaturationDBM=%g\n", PulseInfo.fSaturationDBM);
  fprintf(_out, "fRangeMaskRes=%g\n", PulseInfo.fRangeMaskRes);

  fprintf(_out, "iRangeMask=");
  for (int ii = 0; ii < USERMASKLEN; ii++) {
    fprintf(_out, "%d", PulseInfo.iRangeMask[ii]);
    if (ii != USERMASKLEN - 1) {
      fprintf(_out, " ");
    }
  }
  fprintf(_out, "\n");
  
  fprintf(_out, "fNoiseDBm=%g %g\n",
          PulseInfo.fNoiseDBm[0],
          PulseInfo.fNoiseDBm[1]);
  fprintf(_out, "fNoiseStdvDB=%g %g\n",
          PulseInfo.fNoiseStdvDB[0],
          PulseInfo.fNoiseStdvDB[1]);
  
  fprintf(_out, "fNoiseRangeKM=%g\n", PulseInfo.fNoiseRangeKM);
  fprintf(_out, "fNoisePRFHz=%g\n", PulseInfo.fNoisePRFHz);

  fprintf(_out, "iGparmLatchSts=%d %d\n",
          PulseInfo.iGparmLatchSts[0],
          PulseInfo.iGparmLatchSts[1]);

  fprintf(_out, "iGparmImmedSts=%d %d %d %d %d %d\n",
          PulseInfo.iGparmImmedSts[0],
          PulseInfo.iGparmImmedSts[1],
          PulseInfo.iGparmImmedSts[2],
          PulseInfo.iGparmImmedSts[3],
          PulseInfo.iGparmImmedSts[4],
          PulseInfo.iGparmImmedSts[5]);

  fprintf(_out, "iGparmDiagBits=%d %d %d %d\n",
          PulseInfo.iGparmDiagBits[0],
          PulseInfo.iGparmDiagBits[1],
          PulseInfo.iGparmDiagBits[2],
          PulseInfo.iGparmDiagBits[3]);

  fprintf(_out, "sVersionString=%s\n", PulseInfo.sVersionString);

  fprintf(_out, "rvp8PulseInfo end\n");

  return 0;

}

////////////////////////
// write the pulse header

int Rvp8TsUdp2File::_writePulseHeader()

{
  
  if (_out == NULL) {
    return -1;
  }

  fprintf(_out, "rvp8PulseHdr start\n");

  fprintf(_out, "iVersion=%d\n", PulseHeader.iVersion);
  fprintf(_out, "iFlags=%d\n", PulseHeader.iFlags);
  fprintf(_out, "iMSecUTC=%d\n", PulseHeader.iMSecUTC);
  fprintf(_out, "iTimeUTC=%d\n", PulseHeader.iTimeUTC);
  fprintf(_out, "iBtimeAPI=%d\n", PulseHeader.iBtimeAPI);
  fprintf(_out, "iSysTime=%d\n", PulseHeader.iSysTime);
  fprintf(_out, "iPrevPRT=%d\n", PulseHeader.iPrevPRT);
  fprintf(_out, "iNextPRT=%d\n", PulseHeader.iNextPRT);
  fprintf(_out, "iSeqNum=%d\n", PulseHeader.iSeqNum);
  fprintf(_out, "iAqMode=%d\n", PulseHeader.iAqMode);
  fprintf(_out, "iPolarBits=%d\n", PulseHeader.iPolarBits);
  fprintf(_out, "iTxPhase=%d\n", PulseHeader.iTxPhase);
  fprintf(_out, "iAz=%d\n", PulseHeader.iAz);
  fprintf(_out, "iEl=%d\n", PulseHeader.iEl);
  fprintf(_out, "iNumVecs=%d\n", PulseHeader.iNumVecs);
  fprintf(_out, "iMaxVecs=%d\n", PulseHeader.iMaxVecs);
  fprintf(_out, "iVIQPerBin=%d\n", PulseHeader.iVIQPerBin);
  fprintf(_out, "iTgBank=%d\n", PulseHeader.iTgBank);
  fprintf(_out, "iTgWave=%d\n", PulseHeader.iTgWave);

  fprintf(_out, "uiqPerm.iLong=%d %d\n",
          PulseHeader.uiqPerm.iLong[0],
          PulseHeader.uiqPerm.iLong[1]);

  fprintf(_out, "uiqOnce.iLong=%d %d\n",
          PulseHeader.uiqOnce.iLong[0],
          PulseHeader.uiqOnce.iLong[1]);

  fprintf(_out, "RX[0].fBurstMag=%g\n", PulseHeader.Rx[0].fBurstMag);
  fprintf(_out, "RX[0].iBurstArg=%d\n", PulseHeader.Rx[0].iBurstArg);
  fprintf(_out, "RX[1].fBurstMag=%g\n", PulseHeader.Rx[1].fBurstMag);
  fprintf(_out, "RX[1].iBurstArg=%d\n", PulseHeader.Rx[1].iBurstArg);

  fprintf(_out, "rvp8PulseHdr end\n");

  return 0;

}

////////////////////////
// write the pulse data

int Rvp8TsUdp2File::_writePulseData()

{
  
  if (_out == NULL) {
    return -1;
  }

  int nChannels = PulseHeader.iVIQPerBin;
  int nGates = PulseHeader.iNumVecs;
  int nVals = nGates * nChannels * 2;

  if (fwrite(iTempPackedIQ, 2, nVals, _out) != nVals) {
    return -1;
  }

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

int Rvp8TsUdp2File::_makedir(const char *path)
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

int Rvp8TsUdp2File::_makedir_recurse(const char *path)
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

