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
// Lirp2NetCdf.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// Lirp2NetCdf reads raw LIRP IQ files and converts them
// to NetDCF
//
////////////////////////////////////////////////////////////////

#ifndef Lirp2NetCdf_H
#define Lirp2NetCdf_H

#include <string>
#include <vector>
#include <cstdio>
#include <netcdf.hh>
#include "Args.hh"
using namespace std;
class InputPath;

typedef struct {
  float i;
  float q;
} iq_t;

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

// generic user bits

typedef struct {
  UINT4 iLong[2];
} uiqbits;

// header information for each pulse
// RVP8 header structs copied from rvp8.h in SIGMET source distribution

#define HAS_IBURST_ARGS true

typedef struct {
  UINT1 lValid;     /* Header plus associated data are all valid */
  UINT1 iMajorMode; /* RVP8 major mode, one of PMODE_xxx */
  UINT2 iMSecUTC;   /* MiliSeconds absolute time */
  UINT4 iTimeUTC;   /* Seconds absolute time */
  UINT4 iSeqNum;    /* Sequence number by which pulses are referenced */
  UINT4 iSysTime;   /* AQCLK1X (approx 36MHz) time of arrival */
  UINT4 iBtime;     /* System BTime (milliseconds) of arrival */
  UINT4 iPrevPRT;   /* SysTime ticks to previous pulse */
  UINT4 iNextPRT;   /* SysTime ticks to next pulse */
  SINT4 iDataOffs[MAXVIQPERBIN] ; /* Data offset in fIQ[]
				   * for each Rx channel */
  BIN2 iAz;         /* Antenna azimuth */
  BIN2 iEl;         /* Antenna elevation */
  SINT2 iNumVecs;   /* Actual (I,Q) vectors (burst+data) for each Rx, */
  SINT2 iMaxVecs;   /*   and maximum number (which sets data stride) */
  UINT2 iWrapIQ;    /* Data wraparound counter for validity checks */
  UINT1 iAqMode;    /* Sequence numbers for acquisition mode */
  UINT1 iTgBank;    /* Trigger bank number (one of TBANK_xxx) */
  UINT1 iTgWave;    /* Trigger waveform sequence number */
  UINT1 iVIQPerBin; /* (I,Q) vectors/bin, i.e., # of Rx channels */
  char pad50x2[2];
  uiqbits uiqPerm;  /* User specified bits (Permanent) */
  uiqbits uiqOnce;  /* User specified bits (One-Shot) */
  FLT4 fBurstMags[MAXVIQPERBIN]; /* Burst pulse mag for each Rx channel */
#ifdef HAS_IBURST_ARGS
  BIN2 iBurstArgs[MAXVIQPERBIN] ; /* Burst pulse phase change from
				   * previous pulse */
#endif
} rvp8PulseHdr_v0;

struct rvp8PulseHdrRx {
  SINT4 iDataOff ;   /* Offset of the start of this pulse in fIQ[] */
  FLT4  fBurstMag ;  /* Burst pulse magnitude (amplitude, not power) */
  BIN2  iBurstArg ;  /* Burst phase changes (PrevPulse - ThisPulse) */
  UINT1 iWrapIQ ;    /* Data wraparound count for validity check */
  char  pad11x9[9] ;
} ;

struct rvp8PulseHdr {
  UINT1 iVersion ;     /* Version of this public structure */
#define PHDRVERSION 0  /*   0: Initial public element layout */

  UINT1 iFlags ;              /* Control flags */
#define PHDRFLG_VALID   0x01  /* Header plus associated data are all valid */
#define PHDRFLG_DATAGAP 0x02  /* One or more pulses are missing prior to this one */

  UINT2 iMSecUTC ;    /* MilliSeconds absolute time */
  UINT4 iTimeUTC ;    /* Seconds absolute time */

  UINT4 iBtime ;      /* System BTime (milliseconds) of arrival */
  UINT4 iSysTime ;    /* AQCLK1X (approx 36MHz) time of arrival */
  UINT4 iPrevPRT, iNextPRT ; /* SysTime ticks to previous/next pulse */

  UINT4 iSeqNum ;     /* Sequence number by which pulses are referenced */
  UINT1 iAqMode ;     /* Sequence numbers for acquisition mode */
  char pad29x19[19] ;

  BIN2  iAz, iEl ;    /* Antenna azimuth and elevation */

  SINT2 iNumVecs ;    /* Actual (I,Q) vectors (burst+data) for each Rx, */
  SINT2 iMaxVecs ;    /*   and maximum number (which sets data stride). */
  UINT1 iVIQPerBin ;  /* (I,Q) vectors/bin, i.e., # of Rx channels */

  UINT1 iTgBank ;     /* Trigger bank number (one of TBANK_xxx) */
  UINT2 iTgWave ;     /* Trigger waveform sequence number within a bank */

  uiqbits uiqPerm ;   /* User specified bits (Permanent) */
  uiqbits uiqOnce ;   /* User specified bits (One-Shot) */

  struct rvp8PulseHdrRx	   /* Header information that is different for */
    Rx[ MAXVIQPERBIN ] ;   /*   each receive channel. */

  char pad116x56[56] ;
} ;

typedef struct { 
  UINT4 id;
  UINT4 recordSize;            /* size of this data record in bytes */
  UINT4 version;               /* version of data structure */
  SINT4 packedData;            /* packed data format? TRUE/FALSE */
  UINT2 startGate;             /* beginning range bin of data */
  UINT2 endGate;               /* ending gate of data */
  UINT4 spare[27];
} rocPulseHdr;

////////////////////////
// This class

class Lirp2NetCdf {
  
public:

  // constructor

  Lirp2NetCdf (int argc, char **argv);

  // destructor
  
  ~Lirp2NetCdf();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  InputPath *_input;

  int _nGates;
  time_t _startTime;
  double _startAz;
  double _elevation;
  bool _highSnrPack;

  int _processFile(const char *input_path);

  int _readPulseHeaders(FILE *in,
			bool hasRocHdr,
			rocPulseHdr &rocHdr,
			rvp8PulseHdr &rvp8Hdr);

  int _writeOut(const vector<iq_t> &iqVec);

  int _writeTmpFile(const string &tmpPath,
		    const vector<iq_t> &iqVec,
		    const vector<float> &elVec,
		    const vector<float> &azVec,
		    const vector<int> &prtVec,
		    const vector<double> &timeVec,
		    const vector<float> &modCodeVec);

  void _printPulseHeader(ostream &out,
			 const rvp8PulseHdr &hdr);

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





