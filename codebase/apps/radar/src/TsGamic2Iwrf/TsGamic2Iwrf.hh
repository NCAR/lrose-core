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
// TsGamic2Iwrf.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2016
//
///////////////////////////////////////////////////////////////
//
// TsGamic2Iwrf reads pulse IQ data in GAMIC format and
// and writes it to files in IWRF time series format
//
////////////////////////////////////////////////////////////////

#ifndef TsGamic2Iwrf_H
#define TsGamic2Iwrf_H

#include <string>
#include <vector>
#include <cstdio>

#include "Args.hh"
#include "Params.hh"
#include <toolsa/TaArray.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include <didss/DsInputPath.hh>

using namespace std;

////////////////////////
// This class

class TsGamic2Iwrf {
  
public:

  // constructor

  TsGamic2Iwrf(int argc, char **argv);

  // destructor
  
  ~TsGamic2Iwrf();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // reading input data files

  DsInputPath *_reader;

  // reading in GAMIC data

  typedef struct {

    ui32 command;        // 0
    ui32 length;         // 1
    ui32 acqModeA;       // 2
    ui32 acqModeB;       // 3

    ui32 numBurst;       // 4
    ui32 offsetBurst;    // 5
    ui32 numHorIQ;       // 6
    ui32 offsetHorIQ;    // 7
    ui32 numVerIQ;       // 8
    ui32 offsetVerIQ;    // 9
    ui32 numDualIQ;      // 10
    ui32 offsetDualIQ;   // 11

    ui32 reserve_num_offsetA[56];   // 12-67  x56 unused offset
    ui32 reserve_num_offsetB[64];   // 68-131 x64 unused offset

    ui32 reserve1[9];    // 132-140 x9 reserve

    ui32 aziTag;         // 141
    ui32 eleTag;         // 142
    fl32 aziSpeed;       // 143
    fl32 eleSpeed;       // 144

    ui32 angleSyncFlag;          // 145
    ui32 angleSyncPulseCount;    // 146
    ui32 pulseCounter;           // 147

    ui32 highPrf;                // 148
    ui32 lowPrf;                 // 149
    ui32 prf3;                   // 150
    ui32 prf4;                   // 151
    ui32 prfIndicator;           // 152

    fl32 rangeResolutionIQ;      // 153
    ui32 numberOfValidRangeBins; // 154

    ui32 timestampS;             // 155
    ui32 timestampUS;            // 156

    fl32 meanBurstPower;         // 157
    fl32 burstPower;             // 158
    fl32 meanBurstFreq;          // 159
    fl32 burstPhase;             // 160
    fl32 lastBurstPhase;         // 161

    fl32 IFSamplingFreq;         // 162

    fl32 converterPhaseDiffHorizontal;  // 163
    fl32 converterPhaseDiffVertical;    // 164
    fl32 converterPowerDiffHorizontal;  // 165
    fl32 converterPowerDiffVertical;    // 166

    ui32 reserve2[2];            // 167-168 x2 reserve
    
    fl32 afcOffset;              // 169
    ui32 afcMode;                // 170
    ui32 errorFlags;             // 171

    ui08 adcTemperature[8];      // 172-173 x8

    ui32 ifdPowerFlags;          // 174
    ui32 reserve3[4];            // 175-178 x4 reserve

    ui32 headerID;               // 179

    fl32 adcHLMaxAmplitude;      // 180
    fl32 adcHHMaxAmplitude;      // 181
    fl32 adcVLMaxAmplitude;      // 182
    fl32 adcVHMaxAmplitude;      // 183
    ui32 adcOverflowFlags;       // 184

    fl32 pciDMATransferRate;     // 185
    fl32 pciDMAFIFOFillDegree;   // 186
    fl32 reserve4[3];            // 187-189 x3 reserve

    ui32 errorConditionCounter;  // 190
    ui32 errorCondition[32];     // 191-222 x32
    fl32 reserve5[7];            // 223-229 x7 reserve

    ui32 dspFirmwareVersion;     // 230
    ui32 fpgaPciFirmwareVersion; // 231
    ui32 fpgaIfdFirmwareVersion; // 232
    ui32 reserve6a;              // 233 reserve
    fl32 reserve6[5];            // 234-238 x5 reserve

    ui32 checksum;               // 239

    fl32 radarWavelength;        // 240
    fl32 horizontalBeamwidth;    // 241
    fl32 verticalBeamwidth;      // 242

    ui32 pwIndex;                // 243

    fl32 noisePowerH;            // 244
    fl32 noisePowerV;            // 245
    fl32 dbz0H;                  // 246
    fl32 dbz0V;                  // 247
    fl32 dbz0C;                  // 248
    fl32 zdrOffset;              // 249
    fl32 ldrOffset;              // 250
    fl32 phidpOffset;            // 251

    ui32 sweepUID;               // 252
    ui32 reserve7[3];            // 253-255 x3

  } gamic_header_t;

  gamic_header_t _gamicHdr;

  // radar ops type

  typedef enum {
    HORIZONTAL_ONLY,
    VERTICAL_ONLY,
    DUAL_POL,
    OPS_UNKNOWN
  } ops_type_t;
  ops_type_t _opsType;

  // IQ data

  int _nGates;
  int _nChannels;
  int _nIQPerChannel;
  int _nIQ;
  TaArray<fl32> _iq_;
  fl32 *_iq;

  // IWRF format

  si64 _packetSeqNum;
  si64 _pulseSeqNum;

  time_t _pulseTimeSecs;
  int _pulseTimeNanoSecs;
  
  time_t _prevTimeSecs;
  int _prevTimeNanoSecs;

  IwrfTsInfo _info;

  double _prevBurstPhase;

  // output file

  FILE *_out;
  time_t _outputTime;
  string _outputDir;
  string _outputName;
  string _relPath;
  string _outputPath;
  si64 _filePulseCount;

  // functions
  
  int _processFile(const char* filePath);

  double _getPulseWidth(int index);
  int _handleInputPulse();

  void _loadIwrfInfo();
  void _loadIwrfPulse(IwrfTsPulse &pulse);

  int _openOutputFile(const IwrfTsPulse &pulse);
  int _closeOutputFile();

  void _print(const gamic_header_t &hdr, ostream &out);

};

#endif
