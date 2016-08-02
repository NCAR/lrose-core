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
// Lirp2NetCdf.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2003
//
///////////////////////////////////////////////////////////////
//
// Lirp2NetCdf reads raw LIRP IQ files and converts them
// to NetDCF
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <netcdf.hh>
#include "Lirp2NetCdf.hh"
#include "InputPath.hh"

#define MAX_PATH_LEN 1024
#define PATH_DELIM '/'

using namespace std;

// Constructor

Lirp2NetCdf::Lirp2NetCdf(int argc, char **argv)
  
{

  _input = NULL;
  isOK = true;

  // set programe name
  
  _progName = "Lirp2NetCdf";

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  return;
  
}

// destructor

Lirp2NetCdf::~Lirp2NetCdf()

{

  if (_input) {
    delete _input;
  }

}

//////////////////////////////////////////////////
// Run

int Lirp2NetCdf::Run ()
{

  // initialize the data input object
  
  if (_args.realtime) {
    _input = new InputPath(_args.inDir, 0);
  } else {
    _input = new InputPath(_args.inputFileList);
  }
  if (_args.debug) {
    _input->setDebug(TRUE);
  }

  int iret = 0;

  if (_args.debug) {
    cerr << "Running Lirp2NetCdf - debug mode" << endl;
  }

  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    if (_processFile(inputPath)) {
      cerr << "ERROR = Lirp2NetCdf::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int Lirp2NetCdf::_processFile(const char *input_path)

{
  
  if (_args.debug) {
    cerr << "Processing file: " << input_path << endl;
  }
  
  // open file
  
  FILE *in;
  if ((in = fopen(input_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Lirp2NetCdf::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // is the data in the file in HIGHSNRPACK

  _highSnrPack = false;
  if (strstr(input_path, "_hp_") != NULL) {
    _highSnrPack = true;
  }

  // read in first 4 bytes to see if we have a ROC hdr

  UINT4 rocId;
  if (fread(&rocId, sizeof(UINT4), 1, in) != 1) {
    int errNum = errno;
    cerr << "ERROR - Lirp2NetCdf::_processFile" << endl;
    cerr << "  Cannot read rocId" << endl;
    cerr << "  " << strerror(errNum) << endl;
    fclose(in);
    return -1;
  }
  bool hasRocHdr = FALSE;
  if (rocId == 0x53545652) {
    hasRocHdr = TRUE;
  }
  
  // rewind

  rewind(in);
  
  // read in data, load up vectors
  
  vector<iq_t> iqVec;
  vector<float> elVec, azVec;
  vector<double> timeVec;
  vector<int> prtVec;
  vector<float> modCodeVec;

  _startTime = 0;
  _startAz = 0.0;
  double sumEl = 0.0;

  while (!feof(in)) {

    // read in headers
    
    rocPulseHdr rocHdr;
    rvp8PulseHdr rvp8Hdr;

    if (_readPulseHeaders(in, hasRocHdr, rocHdr, rvp8Hdr)) {
      break;
    }
    
    if (_args.verbose) {
      _printPulseHeader(cerr, rvp8Hdr);
      cerr << "  iNumVecs: " << rvp8Hdr.iNumVecs << endl;
    }
    
    if (rvp8Hdr.iFlags != PHDRFLG_VALID) {
      continue;
    }
    
    double pulseTime =
      (double) rvp8Hdr.iTimeUTC + rvp8Hdr.iMSecUTC / 1000.0;
    timeVec.push_back(pulseTime);

    int prt = 1;
    if (rvp8Hdr.iPrevPRT != 0) {
      prt = (int) (rvp8Hdr.iPrevPRT / 36.0 + 0.5); 
    }
    prtVec.push_back(prt);
    
    double el = (rvp8Hdr.iEl / 65535.0) * 360.0;
    double az = (rvp8Hdr.iAz / 65535.0) * 360.0;
    elVec.push_back(el);
    azVec.push_back(az);

    float phaseDiff = 0.0;
    phaseDiff =  (rvp8Hdr.Rx[0].iBurstArg / 65536.0) * 360.0;
    modCodeVec.push_back(phaseDiff);
    
    if (_startTime == 0) {
      _startTime = rvp8Hdr.iTimeUTC;
      _startAz = az;
    }
    sumEl += el;
    
    // read in iq data
    
    _nGates = rvp8Hdr.iNumVecs;
    int nVals = _nGates * 2;
    UINT2 packed[nVals];
    if (fread(packed, sizeof(UINT2), nVals, in) != nVals) {
      continue;
    }
    
    // convert packed data to floats

    FLT4 unpacked[nVals];
    _vecFloatIQFromPackIQ(unpacked, packed, nVals, (UINT1) _highSnrPack);

    // load up iq data

    FLT4 *iqp = unpacked;
    for (int igate = 0; igate < _nGates; igate++) {
      iq_t iq;
      iq.i = *iqp;
      iqp++;
      iq.q = *iqp;
      iqp++;
      iqVec.push_back(iq);
    }
    
  } // while
  fclose(in);

  _elevation = sumEl / elVec.size();

  // compute output and tmp paths

  string outName;
  const char *lastDelim = strrchr(input_path, PATH_DELIM);
  if (lastDelim) {
    outName = lastDelim + 1;
  } else {
    outName = input_path;
  }
  outName += ".nc";
  string outPath = _args.outDir;
  outPath += PATH_DELIM;
  outPath += outName;
  string tmpPath = outPath;
  tmpPath += ".tmp";
  
  if (_args.debug) {
    cerr << "Output file path: " << outPath << endl;
  }

  // write out tmp file

  if (_writeTmpFile(tmpPath, iqVec, elVec, azVec,
		    prtVec, timeVec, modCodeVec)) {
    cerr << "ERROR - Lirp2NetCdf::_processFile" << endl;
    cerr << "  Cannot write tmp file: " << tmpPath << endl;
    return -1;
  }
  
  if (_args.debug) {
    cerr << "  Wrote tmp file: " << tmpPath << endl;
  }

  // move the tmp file to final name
  
  if (rename(tmpPath.c_str(), outPath.c_str())) {
    int errNum = errno;
    cerr << "ERROR - Lirp2NetCdf::_processFile" << endl;
    cerr << "  Cannot rename file: " << tmpPath << endl;
    cerr << "             to file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_args.debug) {
    cerr << "  Renamed file: " << tmpPath << endl;
    cerr << "       to file: " << outPath << endl;
  }

  return 0;

}

///////////////////////////////////////////////////////////
// read in the pulse header
//
// Returns 0 on success, -1 on failure

int Lirp2NetCdf::_readPulseHeaders(FILE *in,
				   bool hasRocHdr,
				   rocPulseHdr &rocHdr,
				   rvp8PulseHdr &rvp8Hdr)
{

  // read in ROC header
    
  if (hasRocHdr) {
    if (fread(&rocHdr, sizeof(rocPulseHdr), 1, in) != 1) {
      return -1;
    }
  }

  if (_args.verbose) {
    if (hasRocHdr) {
      cerr << "  Has ROC header" << endl;
    }
    cerr << "  ROC header version: " << rocHdr.version << endl;
  }
    
  // if RCO header version is 1, or no ROC header, read in 
  // rvp8 pulse header V0, and convert. Otherwise read in
  // rvp8 pulse header

  if (hasRocHdr && rocHdr.version > 1) {

    if (_args.verbose) {
      cerr << "Reading new RVP8 pulse header" << endl;
    }

    if (fread(&rvp8Hdr, sizeof(rvp8Hdr), 1, in) != 1) {
      return -1;
    }

  } else {

    if (_args.verbose) {
      cerr << "Reading old RVP8 pulse header, converting to new" << endl;
    }

    rvp8PulseHdr_v0 rvp8HdrV0;
    if (fread(&rvp8HdrV0, sizeof(rvp8HdrV0), 1, in) != 1) {
      return -1;
    }

    memset(&rvp8Hdr, 0, sizeof(rvp8Hdr));
    if (rvp8HdrV0.lValid) {
      rvp8Hdr.iFlags = PHDRFLG_VALID;
    } else {
      rvp8Hdr.iFlags = PHDRFLG_DATAGAP;
    }
    
    rvp8Hdr.iMSecUTC = rvp8HdrV0.iMSecUTC;
    rvp8Hdr.iTimeUTC = rvp8HdrV0.iTimeUTC;
    rvp8Hdr.iBtime = rvp8HdrV0.iBtime;
    rvp8Hdr.iSysTime = rvp8HdrV0.iSysTime;
    rvp8Hdr.iPrevPRT = rvp8HdrV0.iPrevPRT;
    rvp8Hdr.iNextPRT = rvp8HdrV0.iNextPRT;
    rvp8Hdr.iSeqNum = rvp8HdrV0.iSeqNum;
    rvp8Hdr.iAqMode = rvp8HdrV0.iAqMode;
    rvp8Hdr.iAz = rvp8HdrV0.iAz;
    rvp8Hdr.iEl = rvp8HdrV0.iEl;
    rvp8Hdr.iNumVecs = rvp8HdrV0.iNumVecs;
    rvp8Hdr.iMaxVecs = rvp8HdrV0.iMaxVecs;
    rvp8Hdr.iVIQPerBin = rvp8HdrV0.iVIQPerBin;
    rvp8Hdr.iTgBank = rvp8HdrV0.iTgBank;
    rvp8Hdr.iTgWave = rvp8HdrV0.iTgWave;
    rvp8Hdr.uiqPerm = rvp8HdrV0.uiqPerm;
    rvp8Hdr.uiqOnce = rvp8HdrV0.uiqOnce;
    for (int ii = 0; ii < MAXVIQPERBIN; ii++) {
      rvp8Hdr.Rx[ii].iDataOff = rvp8HdrV0.iDataOffs[ii];
      rvp8Hdr.Rx[ii].fBurstMag = rvp8HdrV0.fBurstMags[ii];
      rvp8Hdr.Rx[ii].iBurstArg = rvp8HdrV0.iBurstArgs[ii];
      rvp8Hdr.Rx[ii].iWrapIQ = rvp8HdrV0.iWrapIQ;
    }

  }

  return 0;

}
  
///////////////////////////////////////////////////////////
// print the pulse header
//

void Lirp2NetCdf::_printPulseHeader(ostream &out,
				    const rvp8PulseHdr &hdr)
{

  out << "==================== Pulse header ==================" << endl;
  out << "  iVersion: " << (int) hdr.iVersion << endl;
  out << "  iFlags: " << (int) hdr.iFlags << endl;
  out << "  iMSecUTC: " << (int) hdr.iMSecUTC << endl;
  out << "  iTimeUTC: " << (int) hdr.iTimeUTC << endl;
  out << "  iBtime: " << (int) hdr.iBtime << endl;
  out << "  iSysTime: " << (int) hdr.iSysTime << endl;
  out << "  iPrevPRT: " << (int) hdr.iPrevPRT << endl;
  out << "  iNextPRT: " << (int) hdr.iNextPRT << endl;
  out << "  iSeqNum: " << (int) hdr.iSeqNum << endl;
  out << "  iAqMode: " << (int) hdr.iAqMode << endl;
  out << "  iAz: " << (int) hdr.iAz << endl;
  out << "  iEl: " << (int) hdr.iEl << endl;
  out << "  iNumVecs: " << hdr.iNumVecs << endl;
  out << "  iMaxVecs: " << hdr.iMaxVecs << endl;
  out << "  iVIQPerBin: " << (int) hdr.iVIQPerBin << endl;
  out << "  iTgBank: " << (int) hdr.iTgBank << endl;
  out << "  iTgWave: " << (int) hdr.iTgWave << endl;
  out << "  uiqPerm.iLong[0]: " << (int) hdr.uiqPerm.iLong[0] << endl;
  out << "  uiqPerm.iLong[1]: " << (int) hdr.uiqPerm.iLong[1] << endl;
  out << "  uiqOnce.iLong[0]: " << (int) hdr.uiqOnce.iLong[0] << endl;
  out << "  uiqOnce.iLong[1]: " << (int) hdr.uiqOnce.iLong[1] << endl;
  for (int i = 0; i < MAXVIQPERBIN; i++) {
    out << "  iDataOffs[" << i << "]: " << hdr.Rx[i].iDataOff << endl;
    out << "  fBurstMag[" << i << "]: " << hdr.Rx[i].fBurstMag << endl;
    out << "  iBurstArg[" << i << "]: " << hdr.Rx[i].iBurstArg << endl;
    out << "  iWrapIQ[" << i << "]: " << hdr.Rx[i].iWrapIQ << endl;
  }

  double pulseTime =
    (double) hdr.iTimeUTC + hdr.iMSecUTC / 1000.0;
  out << "  pulseTime: " << pulseTime << endl;
  out << "  usTime: " << hdr.iMSecUTC / 1000.0 << endl;
  
  double az = (hdr.iAz / 65535.0) * 360.0;
  double el = (hdr.iEl / 65535.0) * 360.0;
  
  out << "  el: " << el << endl;
  out << "  az: " << az << endl;

}

////////////////////////////////////////
// write out the netDCF file to tmp name
//
// Returns 0 on success, -1 on failure

int Lirp2NetCdf::_writeTmpFile(const string &tmpPath,
			       const vector<iq_t> &iqVec,
			       const vector<float> &elVec,
			       const vector<float> &azVec,
			       const vector<int> &prtVec,
			       const vector<double> &timeVec,
			       const vector<float> &modCodeVec)
  
{

  // ensure output directory exists
  
  if (_makedir_recurse(_args.outDir.c_str())) {
    cerr << "ERROR - Lirp2NetCdf::_writeTmpFile" << endl;
    cerr << "  Cannot make output dir: " << _args.outDir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  ////////////////////////
  // create NcFile object
  
  NcError err(NcError::verbose_nonfatal);
  
  NcFile out(tmpPath.c_str(), NcFile::Replace);
  if (!out.is_valid()) {
    cerr << "ERROR - Lirp2NetCdf::_writeTmpFile" << endl;
    cerr << "  Cannot create file: " << tmpPath << endl;
    return -1;
  }
  int iret = 0;

  /////////////////////
  // global attributes
  
  int nTimes = iqVec.size() / _nGates;
  int startingSample = 0;
  int endingSample = startingSample + nTimes - 1;
  int startGate = 0;
  int endGate = startGate + _nGates - 1;
  
  char desc[1024];
  sprintf(desc,
	  "Radar time series reformatted by Lirp2NetCdf\n"
	  "Starting Sample =%d, Ending Sample =%d, "
	  "Start Gate= %d, End Gate = %d\n"
	  "Azimuth = %.2f, Elevation = %.2f\n",
	  startingSample, endingSample, startGate, endGate,
	  _startAz, _elevation);
  out.add_att("Description", desc);
  out.add_att("FirstGate", startGate);
  out.add_att("LastGate", endGate);

  //////////////////
  // add dimensions
  
  NcDim *gatesDim = out.add_dim("gates", _nGates);
  //int gatesId = gatesDim->id();

  NcDim *frtimeDim = out.add_dim("frtime");
  //int frtimeId = frtimeDim->id();

  /////////////////////////////////
  // add vars and their attributes
  
  // Time variable

  NcVar *timeVar = out.add_var("Time", ncDouble, frtimeDim);
  timeVar->add_att("long_name", "Date/Time value");
  timeVar->add_att("units", "days since 0000-01-01");
  timeVar->add_att("_FillValue", 0.0);
  {
    double times[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      times[jj] = timeVec[jj] / 86400.0;
    }
    long edge = nTimes;
    timeVar->put(times, &edge);
  }

  // Elevation variable
  
  NcVar *elVar = out.add_var("Elevation", ncFloat, frtimeDim);
  elVar->add_att("long_name", "Antenna Elevation");
  elVar->add_att("units", "degrees");
  {
    float validRange[2] = {0.0f, 360.0f};
    elVar->add_att("valid_range", 2, validRange);
  }
  elVar->add_att("_FillValue", (float) 0.0);
  {
    float elevations[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      elevations[jj] = elVec[jj];
    }
    long edge = nTimes;
    elVar->put(elevations, &edge);
  }

  // Azimuth variable

  NcVar *azVar = out.add_var("Azimuth", ncFloat, frtimeDim);
  azVar->add_att("long_name", "Antenna Azimuth");
  azVar->add_att("units", "degrees");
  {
    float validRange[2] = {0.0f, 360.0f};
    azVar->add_att("valid_range", 2, validRange);
  }
  azVar->add_att("_FillValue", (float) 0.0);
  {
    float azimuths[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      azimuths[jj] = azVec[jj];
    }
    long edge = nTimes;
    azVar->put(azimuths, &edge);
  }

  // modulation code variable

  NcVar *modCodeVar = out.add_var("ModCode", ncFloat, frtimeDim);
  modCodeVar->add_att("long_name", "Modulation code, pulse-to-pulse");
  modCodeVar->add_att("units", "degrees");
  {
    float validRange[2] = {0.0f, 360.0f};
    modCodeVar->add_att("valid_range", 2, validRange);
  }
  modCodeVar->add_att("_FillValue", (float) 0.0);
  {
    float modCodes[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      modCodes[jj] = modCodeVec[jj];
    }
    long edge = nTimes;
    modCodeVar->put(modCodes, &edge);
  }

  // PRT variable

  NcVar *prtVar = out.add_var("Prt", ncInt, frtimeDim);
  prtVar->add_att("long_name", "Pulse Repetition Time");
  prtVar->add_att("units", "microseconds");
  prtVar->add_att("valid_range", 1000000);
  {
    int prts[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      prts[jj] = prtVec[jj];
    }
    long edge = nTimes;
    prtVar->put(prts, &edge);
  }
  
  // SampleNum variable

  NcVar *sampleNumVar = out.add_var("SampleNum", ncInt, frtimeDim);
  sampleNumVar->add_att("long_name", "Sample Number");
  sampleNumVar->add_att("units", "Counter");
  sampleNumVar->add_att("valid_range", 100000000);
  {
    int sampleNums[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      sampleNums[jj] = jj;
    }
    long edge = nTimes;
    sampleNumVar->put(sampleNums, &edge);
  }

  // I variable
  
  NcVar *iVar = out.add_var("I", ncFloat, frtimeDim, gatesDim);
  iVar->add_att("long_name", "In-phase time series variable");
  iVar->add_att("units", "scaled A/D counts");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    iVar->add_att("valid_range", 2, validRange);
  }
  iVar->add_att("_FillValue", (float) 0.0);
  {
    float *idata = new float[iqVec.size()];
    for (size_t jj = 0; jj < iqVec.size(); jj++) {
      idata[jj] = iqVec[jj].i;
    }
    long edges[2];
    edges[0] = nTimes;
    edges[1] = _nGates;
    iVar->put(idata, edges);
    delete[] idata;
  }

  // Q variable

  NcVar *qVar = out.add_var("Q", ncFloat, frtimeDim, gatesDim);
  qVar->add_att("long_name", "Quadruture time series variable");
  qVar->add_att("units", "scaled A/D counts");
  {
    float validRange[2] = {-1.e+20f, 1.e+20f};
    qVar->add_att("valid_range", 2, validRange);
  }
  qVar->add_att("_FillValue", (float) 0.0);
  {
    float *qdata = new float[iqVec.size()];
    for (size_t jj = 0; jj < iqVec.size(); jj++) {
      qdata[jj] = iqVec[jj].q;
    }
    long edges[2];
    edges[0] = nTimes;
    edges[1] = _nGates;
    qVar->put(qdata, edges);
    delete[] qdata;
  }

  return iret;

}

//////////////////////////////////////////////////////////
// _makedir()
//
// Utility routine to create a directory.  If the directory
// already exists, does nothing.
//
// Returns -1 on error, 0 otherwise.

int Lirp2NetCdf::_makedir(const char *path)
{
  
  struct stat stat_buf;
  
  // Status the directory to see if it already exists.

  if (stat(path, &stat_buf) == 0) {
    return(0);
  }
  
  // Directory doesn't exist, create it.
  
  if (mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    return(-1);
  
  return(0);

}

///////////////////////////////////////////////////////////
// _makedir_recurse()
//
// Utility routine to create a directory recursively.
// If the directory already exists, does nothing.
// Otherwise it recurses through the path, making all
// needed directories.
//
// Returns -1 on error, 0 otherwise.

int Lirp2NetCdf::_makedir_recurse(const char *path)
{

  char up_dir[MAX_PATH_LEN];
  char *last_delim;
  struct stat dir_stat;
  
  // Stat the directory to see if it already exists.
  // '/' dir will always exist, so this stops the recursion
  // automatically.
  
  if (stat(path, &dir_stat) == 0) {
    return(0);
  }
  
  // create up dir - one up the directory tree -
  // by searching for the previous delim and removing it
  // from the string.
  // If no delim, try to make the directory non-recursively.
  
  strncpy(up_dir, path, MAX_PATH_LEN);
  last_delim = strrchr(up_dir, PATH_DELIM);
  if (!last_delim) {
    return _makedir(up_dir);
  }
  *last_delim = '\0';
  
  // make the up dir
  
  if (_makedir_recurse(up_dir)) {
    return (-1);
  }

  // make this dir

  if (_makedir(path)) {
    return -1;
  } else {
    return 0;
  }
  
}

/* ======================================================================
 * Convert a normalized floating "I" or "Q" value from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 */

void Lirp2NetCdf::_vecFloatIQFromPackIQ
( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[], SINT4 iCount_a )

{

  SINT4 iCount ; volatile const UINT2 *iCodes = iCodes_a ;
  volatile FLT4 *fIQVals = fIQVals_a ;

  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    UINT2 iCode = *iCodes++ ; FLT4 fVal = 0.0 ;

    if( iCode ) {
      SINT4 iMan =  iCode        & 0x3FF ;
      SINT4 iExp = (iCode >> 11) & 0x01F ;

      if( iCode & 0x0400 ) iMan |= 0xFFFFF800 ;
      else                 iMan |= 0x00000400 ;

      fVal = ((FLT4)iMan) * ((FLT4)(UINT4)(1 << iExp)) / 1.09951163E12 ;
    }
    *fIQVals++ = fVal ;
  }
}

/* ------------------------------
 * Convert an array of packed floating to FLT4.
 */

void Lirp2NetCdf::_vecFloatIQFromPackIQ
( volatile FLT4 fIQVals_a[], volatile const UINT2 iCodes_a[],
  SINT4 iCount_a, UINT1 lHiSNR_a )
{
  SINT4 iCount ; volatile const UINT2 *iCodes = iCodes_a ;
  volatile FLT4 *fIQVals = fIQVals_a ;

  if( lHiSNR_a ) {
    /* High SNR packed format with 12-bit mantissa
     */
    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      UINT2 iCode = *iCodes++ ; FLT4 fVal = 0.0 ;

      if( iCode & 0xF000 ) {
        SINT4 iMan =  iCode        & 0x7FF ;
        SINT4 iExp = (iCode >> 12) & 0x00F ;

        if( iCode & 0x0800 ) iMan |= 0xFFFFF000 ;
        else                 iMan |= 0x00000800 ;

        fVal = ((FLT4)iMan) * ((FLT4)(UINT4)(1 << iExp)) / 3.355443E7 ;
      }
      else {
        fVal = ( (FLT4)(((SINT4)iCode) << 20) ) / 1.759218E13 ;
      }
      *fIQVals++ = fVal ;
    }
  } else {
    /* Legacy packed format with 11-bit mantissa
     */
    for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
      UINT2 iCode = *iCodes++ ; FLT4 fVal = 0.0 ;

      if( iCode ) {
        SINT4 iMan =  iCode        & 0x3FF ;
        SINT4 iExp = (iCode >> 11) & 0x01F ;

        if( iCode & 0x0400 ) iMan |= 0xFFFFF800 ;
        else                 iMan |= 0x00000400 ;

        fVal = ((FLT4)iMan) * ((FLT4)(UINT4)(1 << iExp)) / 1.099512E12 ;
      }
      *fIQVals++ = fVal ;
    }
  }
}

