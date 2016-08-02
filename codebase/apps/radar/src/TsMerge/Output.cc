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
// Output.cc
//
// Output object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
///////////////////////////////////////////////////////////////
//
// Output handles time series output to files.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <iomanip>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include "Output.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

Output::Output(const string &label,
               const Params &params) :
        _label(label),
        _params(params)

{

  _out = NULL;
  
}

//////////////////////////////////////////////////////////////////
// destructor

Output::~Output()

{

  closeFile();

}

/////////////////////////////////
// open an output file

int Output::openFile(time_t startTime, 
                     double startEl,
                     double startAz)
  
{

  closeFile();

  DateTime stime(startTime);
  
  int iaz = (int) startAz;
  int iel = (int) (startEl * 10.0);

  // compute output dir path

  char outDir[MAX_PATH_LEN];
  sprintf(outDir, "%s%sclutter_minus_%d",
          _params.output_dir, PATH_DELIM, _params.clutter_reduction_db);
  
  // make the output dir

  if (ta_makedir_recurse(outDir)) {
    int errNum = errno;
    cerr << "ERROR - Output" << endl;
    cerr << "  Cannot make output directory: " << outDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute output path

  char path[MAX_PATH_LEN];
  sprintf(path, "%s%s%.4d%.2d%.2d_%.2d%.2d%.2d_%.3d_%.3d.%s.tsarchive",
          outDir, PATH_DELIM,
          stime.getYear(), stime.getMonth(), stime.getDay(),
          stime.getHour(), stime.getMin(), stime.getSec(),
          iel, iaz, _label.c_str());
  _outputPath = path;
  
  // open file
  
  if ((_out = fopen(path, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Output for " << _label << endl;
    cerr << "  Cannot open output file: " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Opened new file for " << _label << endl;
    cerr << "  Path: " << path << endl;
  }

  return 0;

}

///////////////////////////////////////////
// close file
//
// Returns 0 if file already open, -1 if not

int Output::closeFile()

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

int Output::writeInfo(const OpsInfo &info)

{
  
  if (_out == NULL) {
    return -1;
  }

  fprintf(_out, "rvp8PulseInfo start\n");
  fprintf(_out, "iVersion=%d\n", info.iVersion);
  fprintf(_out, "iMajorMode=%d\n", info.iMajorMode);
  fprintf(_out, "iPolarization=%d\n", info.iPolarization);
  fprintf(_out, "iPhaseModSeq=%d\n", info.iPhaseModSeq);
  fprintf(_out, "taskID.iSweep=%d\n", info.iSweep);
  fprintf(_out, "taskID.iAuxNum=%d\n", info.iAuxNum);
  fprintf(_out, "taskID.sTaskName=%s\n", info.sTaskName.c_str());
  fprintf(_out, "sSiteName=%s\n", info.sSiteName.c_str());
  fprintf(_out, "iAqMode=%d\n", info.iAqMode);
  fprintf(_out, "iUnfoldMode=%d\n", info.iUnfoldMode);
  fprintf(_out, "iPWidthCode=%d\n", info.iPWidthCode);
  fprintf(_out, "fPWidthUSec=%g\n", info.fPWidthUSec);
  fprintf(_out, "fDBzCalib=%g\n", info.fDBzCalib);
  fprintf(_out, "iSampleSize=%d\n", info.iSampleSize);
  fprintf(_out, "iMeanAngleSync=%d\n", info.iMeanAngleSync);
  fprintf(_out, "iFlags=%d\n", info.iFlags);
  fprintf(_out, "iPlaybackVersion=%d\n", info.iPlaybackVersion);
  fprintf(_out, "fSyClkMHz=%g\n", info.fSyClkMHz);
  fprintf(_out, "fWavelengthCM=%g\n", info.fWavelengthCM);
  fprintf(_out, "fSaturationDBM=%g\n", info.fSaturationDBM);
  fprintf(_out, "fRangeMaskRes=%g\n", info.fRangeMaskRes);

  fprintf(_out, "iRangeMask=");
  for (int ii = 0; ii < 512; ii++) {
    fprintf(_out, "%d", info.iRangeMask[ii]);
    if (ii != 512 - 1) {
      fprintf(_out, " ");
    }
  }
  fprintf(_out, "\n");
  
  fprintf(_out, "fNoiseDBm=%g %g\n",
          info.fNoiseDBm[0],
          info.fNoiseDBm[1]);
  fprintf(_out, "fNoiseStdvDB=%g %g\n",
          info.fNoiseStdvDB[0],
          info.fNoiseStdvDB[1]);
  
  fprintf(_out, "fNoiseRangeKM=%g\n", info.fNoiseRangeKM);
  fprintf(_out, "fNoisePRFHz=%g\n", info.fNoisePRFHz);

  fprintf(_out, "iGparmLatchSts=%d %d\n",
          info.iGparmLatchSts[0],
          info.iGparmLatchSts[1]);

  fprintf(_out, "iGparmImmedSts=%d %d %d %d %d %d\n",
          info.iGparmImmedSts[0],
          info.iGparmImmedSts[1],
          info.iGparmImmedSts[2],
          info.iGparmImmedSts[3],
          info.iGparmImmedSts[4],
          info.iGparmImmedSts[5]);

  fprintf(_out, "iGparmDiagBits=%d %d %d %d\n",
          info.iGparmDiagBits[0],
          info.iGparmDiagBits[1],
          info.iGparmDiagBits[2],
          info.iGparmDiagBits[3]);

  fprintf(_out, "sVersionString=%s\n", info.sVersionString.c_str());

  fprintf(_out, "rvp8PulseInfo end\n");

  return 0;

}

////////////////////////
// write the pulse header

int Output::writePulseHeader(const Pulse &pulse)

{
  
  if (_out == NULL) {
    return -1;
  }

  fprintf(_out, "rvp8PulseHdr start\n");

  fprintf(_out, "iVersion=%d\n", pulse.iVersion);
  fprintf(_out, "iFlags=%d\n", pulse.iFlags);
  fprintf(_out, "iMSecUTC=%d\n", pulse.iMSecUTC);
  fprintf(_out, "iTimeUTC=%d\n", pulse.iTimeUTC);
  fprintf(_out, "iBtimeAPI=%d\n", pulse.iBtimeAPI);
  fprintf(_out, "iSysTime=%d\n", pulse.iSysTime);
  fprintf(_out, "iPrevPRT=%d\n", pulse.iPrevPRT);
  fprintf(_out, "iNextPRT=%d\n", pulse.iNextPRT);
  fprintf(_out, "iSeqNum=%d\n", pulse.iSeqNum);
  fprintf(_out, "iAqMode=%d\n", pulse.iAqMode);
  fprintf(_out, "iPolarBits=%d\n", pulse.iPolarBits);
  fprintf(_out, "iTxPhase=%d\n", pulse.iTxPhase);
  fprintf(_out, "iAz=%d\n", pulse.iAz);
  fprintf(_out, "iEl=%d\n", pulse.iEl);
  fprintf(_out, "iNumVecs=%d\n", pulse.iNumVecs);
  fprintf(_out, "iMaxVecs=%d\n", pulse.iMaxVecs);
  fprintf(_out, "iVIQPerBin=%d\n", pulse.iVIQPerBin);
  fprintf(_out, "iTgBank=%d\n", pulse.iTgBank);
  fprintf(_out, "iTgWave=%d\n", pulse.iTgWave);

  fprintf(_out, "uiqPerm.iLong=%d %d\n",
          pulse.uiqPerm[0],
          pulse.uiqPerm[1]);

  fprintf(_out, "uiqOnce.iLong=%d %d\n",
          pulse.uiqOnce[0],
          pulse.uiqOnce[1]);

  fprintf(_out, "RX[0].fBurstMag=%g\n", pulse.fBurstMag[0]);
  fprintf(_out, "RX[0].iBurstArg=%d\n", pulse.iBurstArg[0]);
  fprintf(_out, "RX[1].fBurstMag=%g\n", pulse.fBurstMag[1]);
  fprintf(_out, "RX[1].iBurstArg=%d\n", pulse.iBurstArg[1]);

  fprintf(_out, "rvp8PulseHdr end\n");

  return 0;

}

////////////////////////
// write the pulse data

int Output::writeIQ(const void *iqPacked, int nBytesPacked)

{
  
  if (_out == NULL) {
    return -1;
  }

  if ((int) fwrite(iqPacked, 1, nBytesPacked, _out) != nBytesPacked) {
    return -1;
  }

  return 0;

}

