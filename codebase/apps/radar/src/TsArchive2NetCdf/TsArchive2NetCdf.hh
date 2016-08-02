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
/////////////////////////////////////////////////////////////
// TsArchive2NetCdf.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
///////////////////////////////////////////////////////////////
//
// TsArchive2NetCdf reads raw LIRP IQ files and converts them
// to NetDCF
//
////////////////////////////////////////////////////////////////

#ifndef TsArchive2NetCdf_H
#define TsArchive2NetCdf_H

#include <string>
#include <vector>
#include <cstdio>
#include <netcdf.hh>
#include "Args.hh"
using namespace std;
class InputPath;

typedef unsigned char UINT1;
typedef unsigned short UINT2;
typedef unsigned int UINT4;
typedef char SINT1;
typedef short SINT2;
typedef int SINT4;
typedef float FLT4;
typedef unsigned short BIN2;

#define MAXBINS 1024 // max number of bins
#define MAXVIQPERBIN  2 // Max number of (I,Q) vectors per bin

typedef struct {
  float i;
  float q;
} iq_t;

// generic user bits

typedef struct {
  UINT4 iLong[2];
} uiqbits;

// header information for each pulse
// RVP8 header structs copied from rvp8.h in SIGMET source distribution

#define HAS_IBURST_ARGS true

struct rvp8PulseHdrRx {
  SINT4 iDataOff ;   /* Offset of the start of this pulse in fIQ[] */
  FLT4  fBurstMag ;  /* Burst pulse magnitude (amplitude, not power) */
  BIN2  iBurstArg ;  /* Burst phase changes (PrevPulse - ThisPulse) */
  UINT1 iWrapIQ ;    /* Data wraparound count for validity check */
  char  pad11x9[9] ;
} ;

struct rvp8PulseHdr {
  UINT1 iVersion ;		/* Version of this public structure */
#define PHDRVERSION    0	/*   0: Initial public element layout */

  UINT1 iFlags ;		/* Control flags */
#define PHDRFLG_VALID     0x01  /*   Header plus associated data are all valid */
#define PHDRFLG_DATAGAP   0x02  /*   One or more pulses are missing prior to this one */
#define PHDRFLG_BNKFIRST  0x04  /*   First pulse within the present trigger bank */
#define PHDRFLG_BNKLAST   0x08  /*   Last pulse within the present trigger bank */

  UINT2 iMSecUTC ;		/* MilliSeconds absolute time */
  UINT4 iTimeUTC ;		/* Seconds absolute time */

  UINT4 iBtimeAPI ;		/* Local time (ms) when pulse arrived in API */
  UINT4 iSysTime ;		/* AQCLK1X (approx 36MHz) time of arrival */
  UINT4 iPrevPRT, iNextPRT ;	/* SysTime ticks to previous/next pulse */

  UINT4 iSeqNum ;		/* Sequence number by which pulses are referenced */
  UINT1 iAqMode ;		/* Sequence numbers for acquisition mode */

  UINT1 iPolarBits ;		/*   State of polarization control bits */
  BIN2  iTxPhase ;		/*   Phase angle of transmit data */

  char pad32x16[16] ;

  BIN2  iAz, iEl ;		/* Antenna azimuth and elevation */

  SINT2 iNumVecs ;		/* Actual (I,Q) vectors (burst+data) for each Rx, */
  SINT2 iMaxVecs ;		/*   and maximum number (which sets data stride). */
  UINT1 iVIQPerBin ;		/* (I,Q) vectors/bin, i.e., # of Rx channels */

  UINT1 iTgBank ;		/* Trigger bank number (one of TBANK_xxx) */
  UINT2 iTgWave ;		/* Trigger waveform sequence number within a bank */

  uiqbits uiqPerm ;	/* User specified bits (Permanent) */
  uiqbits uiqOnce ;	/* User specified bits (One-Shot) */

  struct rvp8PulseHdrRx		/* Header information that is different for */
    Rx[ MAXVIQPERBIN ] ;	/*   each receive channel. */

  char pad116x56[56] ;
} ;

#define DSPSITENAMELEN 16	/* Site name length within DSPs */
#define DSPTASKNAMELEN 16	/* Task name length within DSPs */
#define USERMASKLEN 512		/* Number of 16-bit range mask words */

struct rvp8TaskID {
  UINT2 iSweep ;		/* Sweep number within a task */
  UINT2 iAuxNum ;		/* Auxiliary sequence number */
  char sTaskName[1+DSPTASKNAMELEN] ; /* Name of the task (NULL terminated) */
  char pad21x3[3] ;
} ;

struct rvp8PulseInfo {
  UINT1 iVersion ;		/* Version of this public structure */
#define PINFOVERSION   0	/*   0: Initial public element layout */

  UINT1 iMajorMode ;		/* RVP8 major mode, one of PMODE_xxx */  
  UINT1 iPolarization ;		/* Polarization selection, one of POL_xxx */
  UINT1 iPhaseModSeq ;		/* Tx phase modulation, one of PHSEQ_xxx */

  struct rvp8TaskID taskID ;	/* Task identification info */

  char sSiteName                /* Data are from this site */
    [1+DSPSITENAMELEN] ; 	/*   (NULL terminated) */

  UINT1 iAqMode ;		/* Acquisition mode (filled in by local API) */
  UINT1 iUnfoldMode ;		/* Dual-PRF unfolding  0:None, 1:2/3, 2:3/4, 3:4/5 */

  UINT1 iPWidthCode ;		/* Pulse width selection (0 to NPWIDS-1), */
  FLT4  fPWidthUSec ;		/*   and actual width in microseconds */

  char pad52x60[60] ;

  FLT4 fAqClkMHz ;		/* Acquisition clock frequency (MHz) */
  FLT4 fWavelengthCM ;		/* Radar wavelength (cm) */
  FLT4 fSaturationDBM ;         /* Power (dBm) corresponding to MAG(I,Q) of 1.0 */

  FLT4 fRangeMaskRes ;		/* Spacing between range bins (meters) */
  UINT2				/* Bit mask of bins that have actually been */
    iRangeMask[ USERMASKLEN ] ;	/*   selected at the above spacing. */

  FLT4 fNoiseDBm   [ MAXVIQPERBIN ] ; /* Noise level in dBm, and standard */
  FLT4 fNoiseStdvDB[ MAXVIQPERBIN ] ; /*   deviation of measurement in dB. */
  FLT4 fNoiseRangeKM ;		/* Range (km) and PRF (Hz) at which the */
  FLT4 fNoisePRFHz ;		/*   last noise sample was taken. */

  UINT2 iGparmLatchSts[2] ;	/* Copies of latched and immediate status */
  UINT2 iGparmImmedSts[6] ;	/*   words from the GPARM structure. */
  UINT2 iGparmDiagBits[4] ;	/* Copies of GPARM diagnostic bits */

  char sVersionString[12] ;	/* IRIS/RDA version (full Major/Minor string) */
				/*   that created the data. */

  char pad1212x1188[1188] ;
} ;

////////////////////////
// This class

class TsArchive2NetCdf {
  
public:

  // constructor

  TsArchive2NetCdf (int argc, char **argv);

  // destructor
  
  ~TsArchive2NetCdf();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  InputPath *_input;

  time_t _startTime;
  double _startAz;
  double _elevation;
  int _nGatesSave;
  int _nGatesFirst;

  static const int _maxNameLen = 256;

  // pulse info fields

  string _versionStr;
  string _taskName;
  string _siteName;

  int _infoVersionNum;
  int _infoFlags;
  int _sampleSize;
  int _polarization;
  int _majorMode;
  int _sweepNum;

  double _pulseWidthUsec;
  double _dbzCalib;
  double _syClkMhz;
  double _wavelengthCm;
  double _rangeMaskRes;
  double _saturationDbm;
  double _noiseDbm;
  double _noiseRangeKm;
  double _noisePrf;

  // pulse header fields

  int _headerVersionNum;
  int _headerFlags;
  int _msecUTC;
  int _timeUTC;
  int _btimeAPI;
  int _sysTime;
  int _prevPRT;
  int _nextPRT;
  int _seqNum;
  int _aqMode;
  int _polarBits;
  int _txPhase;
  int _iaz;
  int _iel;
  int _numVecs;
  int _maxVecs;
  int _vIQPerBin;
  int _tgBank;
  int _tgWave;

  double _burstMag0;
  double _burstArg0;
  double _burstMag1;
  double _burstArg1;

  // derived from pulse header

  time_t _pulseTimeSecs;
  double _pulseTime;

  int _nGates;
  double _el;
  double _az;
  double _prt;
  int _iprt;
  double _phaseDiff;

  // prototypes
  
  int _processFile(const char *input_path);

  int _readPulseInfo(FILE *in);

  int _readPulseHeader(FILE *in);

  void _deriveFromPulseHeader();

  int _writeOut(const vector<iq_t> &iqVec);

  int _writeTmpFile(const string &tmpPath,
		    const vector<iq_t> &iqVec,
		    const vector<float> &elVec,
		    const vector<float> &azVec,
		    const vector<int> &prtVec,
		    const vector<double> &timeVec,
		    const vector<float> &modCodeVec);

  void _printPulseInfo(ostream &out) const;

  void _printPulseHeader(ostream &out) const;
  
  static int _makedir(const char *path);
  static int _makedir_recurse(const char *path);

  void _vecFloatIQFromPackIQ
  ( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
    SINT4 iCount_a );

  void _vecFloatIQFromPackIQ
  ( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
    SINT4 iCount_a, UINT1 lHiSNR_a );

};

#endif





