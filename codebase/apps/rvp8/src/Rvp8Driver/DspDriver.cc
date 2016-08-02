/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
///////////////////////////////////////////////////////////////
// DspDriver.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// Controls the Digital Signal Processor
//
////////////////////////////////////////////////////////////////

#include <rvp8_rap/Rvp8Legacy.hh>

#include <cerrno>
#include <iostream>
#include <sigtypes.h>
#include <dsp.h>
#ifdef RVP8_LEGACY_V8
#include <rvp8.h>
#else
#include <rvpts.h>
#endif
#include <user_lib.h>
#include <setup.h>
#include <dsp_lib.h>
#include <antenna_lib.h>
#include <intelipp_lib.h>
#include <rdasubs_lib.h>
#include <sig_rtdisp.h>
#include <rtq_lib.h>
#include "DspDriver.hh"
using namespace std;

////////////////////////////////////////////////////////////
// Constructor for read-write access

DspDriver::DspDriver(bool dsp_available)
  
{
  
  _dspAvailable = dsp_available;
  _dsp = NULL;
  _readBuf = new UINT2[MAXREADWORDS];
  _nFields = 0;
  _nGates = 0;
  
}

/////////////
// destructor

DspDriver::~DspDriver()
  
{
  if (_readBuf) {
    delete[] _readBuf;
  }
  close();
}

////////////////////////////////////////////
// initalize the DSP
//
// returns 0 on success, -1 on failure

int DspDriver::init()
  
{

  if (!_dspAvailable) {
    if (_debug) {
      cerr << "DspDriver::init" << endl;
      cerr << "  No dsp available, simulating ..." << endl;
    }
    return 0;
  }

  MESSAGE iStatus;

  // close if open

  close();

  // ant power up
  // seems necessary for setting prf
  
  if (_debug) {
    cerr << "Bringing up antenna power" << endl;
  }
  
  iStatus = AntPowerUpMain(ANT_IAM_CLIENT, 0) ;
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::init" << endl;
    cerr << "  Cannot power up antenna" << endl;
    sig_signal(iStatus);
    return -1;
  }
  
  // Attempt to open the DSP
  
  if (_debug) {
    cerr << "Opening DSP" << endl;
  }
  _dsp = DspOpenForIo(&iStatus) ;
  if(NULL == _dsp) {
    cerr << "ERROR - DspDriver::init" << endl;
    cerr << "  Cannot open DSP for IO" << endl;
    sig_signal(iStatus);
    return -1;
  } else {
    sig_signal(iStatus);
  }
  
  if (_debug) {

    // print out type of interface 

    UINT4 interface_type = idsp_interface(_dsp);
    switch (interface_type) {
      case DSPIFACE_SCSI:
        cerr << "  interface type: SCSI" << endl;
        break;
      case DSPIFACE_NATIVE:
        cerr << "  interface type: NATIVE" << endl;
        break;
      case DSPIFACE_SOCKET:
        cerr << "  interface type: SOCKET" << endl;
        break;
      default:
        cerr << "  interface type: UNKNOWN" << endl;
        break;
    }

    // print out processor type

    UINT4 idspType = idsp_type(_dsp);
    cerr << "DSP type: " << (int) idspType << endl;

  }

  // Get the pointer to DSP Setup info 

  _dspSetup = DspGetSetupPointer(_dsp);
  
  // reset FIFO

  if (resetFifo()) {
    cerr << "ERROR - DspDriver::init" << endl;
    return -1;
  }

  // Check for correct byte order

  if (_debug) {
    cerr << "Checking for byte order" << endl;
  }

  iStatus = DspCheckByteOrder(_dsp);
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::init" << endl;
    cerr << "  Incorrect byte order" << endl;
    sig_signal(iStatus);
    return -1;
  }

  // Set up angle synchronization mode

  if (_debug) {
    cerr << "Initialize angle synchronization to NONE" << endl;
  }
  
  iStatus = dspw_syn_mode(_dsp, ANGSYN_NONE, 0);
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::init" << endl;
    cerr << "  Setting angle synchronization" << endl;
    sig_signal(iStatus);
    return -1;
  }

  // Set the task name

  if (_debug) {
    cerr << "Setting task ID" << endl;
    cerr << "-->> DspWriteTaskId" << endl;
  }
  
  iStatus = DspWriteTaskId(_dsp, "Rvp8Driver", 0, 0, TASK_SCAN_PPIFULL);
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::init" << endl;
    cerr << "  Cannot write task ID" << endl;
    sig_signal(iStatus);
    return -1;
  }

  // set ts playback options

  if (_debug) {
    cerr << "Setting time series playback options" << endl;
    cerr << "-->> dspw_playbackTSOpts" << endl;
  }
  
  iStatus = dspw_playbackTSOpts(_dsp, MMTS_PHASEMOD);
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::init" << endl;
    cerr << "  Cannot set playback options" << endl;
    sig_signal(iStatus);
    return -1;
  }

  return 0;

}

/////////////
// close

void DspDriver::close()
  
{

  if (_dsp) {
    if (_debug) {
      cerr << "-->> Closing DSP" << endl;
    }
    DspResetFifo(_dsp);
    int iStatus = DspClose(_dsp);
    if( iStatus != SS_NORMAL ) {
      sig_signal( iStatus );
    }
    _dsp = NULL;
  }
  
}

////////////////////////////////////////////
// reset the FIFO
//
// returns 0 on success, -1 on failure

int DspDriver::resetFifo()
  
{

  if (_debug) {
    cerr << "Resetting FIFO" << endl;
  }

  if (!_dsp) {
    return 0;
  }

  MESSAGE iStatus = DspResetFifo(_dsp);
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::resetFifo" << endl;
    cerr << "  Cannot reset FIFO" << endl;
    sig_signal(iStatus);
    return -1;
  }

  return 0;

}

////////////////////////////////////////////
// set the fields and proc mode
//
// returns 0 on success, -1 on failure

int DspDriver::setProc(struct dsp_data_mask &fieldMask,
                       int procMode)
  
{

  // define what information the proc command puts in the
  // header before the ray data
  //
  // we're currently requesting:
  //       DSP_HDR_TAGS   4 words: begin/end az and begin/end el
  //       DSP_HDR_TSTAMP 1 word:  time series time stamp (milliseconds)
  //       DSP_HDR_MMTS   1 word:  bits showing a mismatch between the
  //                               data and processing setup */
  
  if (!_dsp) {
    _nFields = getNFields(fieldMask);
    return 0;
  }

  {
    int HDRBITS = (DSP_HDR_TAGS | DSP_HDR_TSTAMP |
                   DSP_HDR_MMTS | DSP_HDR_GPARM);
    MESSAGE iStatus = dspw_header_cfg(_dsp, HDRBITS);
    if(iStatus != SS_NORMAL) {
      cerr << "ERROR - DspDriver::setProc" << endl;
      cerr << "  Cannot set header bits" << endl;
      sig_signal(iStatus);
      return -1;
    }
  }

  MESSAGE iStatus = dspw_proc(_dsp, 
                              FALSE,  /* we do NOT want the archive bit set,
                                       * use 16 bit data */
                              &fieldMask,
                              TSOUT_HISNR,
			      DSP_PROC_FREE_RUNNING);
  // procMode);
  
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::setProc" << endl;
    cerr << "  Cannot set proc" << endl;
    sig_signal(iStatus);
    return -1;
  }

  // save number of fields

  _nFields = getNFields(fieldMask);

  return 0;

}

////////////////////////////////////////////
// set the options
//
// returns 0 on success, -1 on failure

int DspDriver::setOptions(int nSamples,
                          int majorMode,
                          int procFlags,
                          int prfMode,
                          int windowType,
                          int rangeSmooth)
  
{

  if (!_dsp) {
    return 0;
  }

  MESSAGE iStatus = dspw_options(_dsp,
                                 majorMode,
                                 nSamples,
                                 procFlags,
                                 0,
                                 0,
                                 prfMode,
                                 windowType,
                                 rangeSmooth);
  
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::setOptions" << endl;
    cerr << "  Cannot set options" << endl;
    sig_signal(iStatus);
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////
// set the range mask
//
// returns 0 on success, -1 on failure

int DspDriver::setRangeMask(int nGates,
                            double gateSpacing)
  
{

  if (!_dsp) {
    _nGates = nGates;
    return 0;
  }

  UINT2 imask[512] ;
  range_mask_gen(_dsp, imask, 0.0, nGates, gateSpacing, 0) ;
  MESSAGE iStatus = dspw_range_mask(_dsp, imask, 0) ;
    
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::setRangeMask" << endl;
    cerr << "  Cannot set range mask" << endl;
    sig_signal(iStatus);
    return -1;
  }

  // save number of gates

  _nGates = nGates;

  return 0;

}

////////////////////////////////////////////
// set the PRF
//
// returns 0 on success, -1 on failure

int DspDriver::setPrf(double prf)
  
{
  
  if (!_dsp) {
    return 0;
  }

  MESSAGE iStatus = dsp_set_prf(_dsp, prf, 0, TRUE);
  
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::setPrf" << endl;
    cerr << "  Cannot set prf" << endl;
    sig_signal(iStatus);
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// set the phase coding (random, sz864 etc)
//
// returns 0 on success, -1 on failure

int DspDriver::setPhaseCoding(int phaseCoding)
  
{
  
  if (!_dsp) {
    return 0;
  }

  MESSAGE iStatus = dspw_phaseSeq(_dsp, phaseCoding, 0, 0);

  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::setPhaseSeq" << endl;
    cerr << "  Cannot set phase sequence" << endl;
    sig_signal(iStatus);
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// set the clutter filter number
//
// returns 0 on success, -1 on failure

int DspDriver::setClutFiltNum(int clutFiltNum)
  
{

  if (!_dsp) {
    return 0;
  }

  // SINT4 maxBins = idsp_bin_max(_dsp) ;
  SINT4 maxBins = RVPX_MAXBINS;

  UINT2 *filtmask = new UINT2[maxBins];
  for (int i = 0; i < maxBins; i++) {
    filtmask[i] = clutFiltNum;
  }
  
  MESSAGE iStatus = dspw_filt(_dsp, filtmask);

  delete[] filtmask;

  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::setClutFiltNum" << endl;
    cerr << "  Cannot set clutter filter number" << endl;
    sig_signal(iStatus);
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// set the polarization
//
// returns 0 on success, -1 on failure

int DspDriver::setPolarization(int polarization)
  
{
  
  if (!_dsp) {
    return 0;
  }

  MESSAGE iStatus = dsp_set_polarization(_dsp, polarization, TRUE);
  
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::setPolarization" << endl;
    cerr << "  Cannot set polarization" << endl;
    sig_signal(iStatus);
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////
// set the angle synchronization mode
//
// returns 0 on success, -1 on failure

int DspDriver::setAngSyncMode(int angSyncMode)
  
{
  
  if (!_dsp) {
    return 0;
  }

  MESSAGE iStatus = dspw_syn_mode(_dsp, angSyncMode, 0);
  
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::setAngSyncMode" << endl;
    cerr << "  Cannot set angSyncMode" << endl;
    sig_signal(iStatus);
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// Check the DSP status
//
// Returns 0 on success, -1 on failure

int DspDriver::checkStatus()

{

  if (!_dsp) {
    return 0;
  }

  if (_verbose) {
    cerr << "-->> Checking DSP status <<--" << endl;
  }

  UINT2 istat;
  MESSAGE iStatus = dspr_status(_dsp, &istat) ;
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::checkStatus" << endl;
    sig_signal(iStatus);
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// Read parameters
//
// Returns 0 on success, -1 on failure

int DspDriver::readParms()

{

  if (_verbose) {
    cerr << "-->> Reading DSP parms <<--" << endl;
  }

  if (!_dsp) {
    if (_verbose) {
      cerr << "-->> Simulating, cannot read params" << endl;
    }
    return 0;
  }

  struct gparm gParm;
  MESSAGE iStatus = dspr_gparm(_dsp, &gParm, GPARM_LSTATUS_CLEAR);
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::checkStatus" << endl;
    sig_signal(iStatus);
    return -1;
  }

  if (_verbose) {
    cerr << "-->> Reading DSP global params" << endl;
    cerr << "  nSamples: " << (int) gParm.isamp << endl;
    cerr << "  ibin_out_num: " << (int) gParm.ibin_out_num << endl;
    cerr << "  iaqbins: " << (int) gParm.iaqbins << endl;
    cerr << "  iprbins: " << (int) gParm.iprbins << endl;
    cerr << "  iprt_start: " << (int) gParm.iprt_start << endl;
   }
  
  struct dspExParm exParm;
  iStatus = dspr_exParm(_dsp, &exParm);
  if(iStatus != SS_NORMAL) {
    cerr << "ERROR - DspDriver::checkStatus" << endl;
    sig_signal(iStatus);
    return -1;
  }

  if (_verbose) {
    cerr << "-->> Reading DSP extended parameters" << endl;
    cerr << "  iRayMajorMode: " << (int) exParm.iRayMajorMode << endl;
    cerr << "  iRayBinCount: " << (int) exParm.iRayBinCount << endl;
    cerr << "  iRayUmode: " << (int) exParm.iRayUmode << endl;
    cerr << "  iRayPWidth: " << (int) exParm.iRayPWidth << endl;
    cerr << "  iRayPhaseMod: " << exParm.iRayPhaseMod << endl;
    cerr << "  iRayPolar: " << (int) exParm.iRayPolar << endl;
    cerr << "  fRayNomPRF: " << (int) exParm.fRayNomPRF << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// Read data available from DSP
//
// Returns 0 on success, -1 on failure

int DspDriver::readAvailable()

{

  if (!_dsp) {
    sig_microSleep( 10000 );
    return 0;
  }

  int nTotal = 0;
  SINT4 nActuallyRead = 1;
  int nToRead = _nFields * _nGates + HDRSIZE;

  while (nActuallyRead != 0 && nTotal < 100000) {
    
    sig_microSleep( 10000 );
    
    UINT2 istat;
    MESSAGE iStatus = dspr_status(_dsp, &istat) ;
    if(iStatus != SS_NORMAL) {
      cerr << "ERROR - DspDriver::readAvailable" << endl;
      sig_signal(iStatus);
      return -1;
    }
    
    iStatus = DspReadAvail(_dsp, _readBuf, nToRead, 2048, &nActuallyRead);
    nTotal += nActuallyRead;
    
    //    if (nActuallyRead > 0 && nActuallyRead != nToRead) {
    //       cerr << "Read nwords: " << nActuallyRead << endl;
    //       cerr << "  nTotal: " << nTotal << endl;
    //    }

    if (iStatus != SS_NORMAL) {
      cerr << "ERROR - DspDriver::readAvailable" << endl;
      sig_signal(iStatus);
      return -1;
    }
    
  } // while

  return 0;

}

////////////////////////////////////////////
// get number of fields from the field mask

int DspDriver::getNFields(struct dsp_data_mask &fieldMask)

{

  int nFields = 0;

  if (lDspMaskTest(&fieldMask, DB_DBT2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_DBZ2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_DBZC2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_FLAGS2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_KDP2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_LDRH2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_LDRV2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_PHIDP2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_PHIH2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_PHIV2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_RHOH2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_RHOHV2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_RHOV2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_SQI2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_VEL2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_VELC2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_WIDTH2)) {
    nFields++;
  }
  if (lDspMaskTest(&fieldMask, DB_ZDR2)) {
    nFields++;
  }

  return nFields;

}
  

