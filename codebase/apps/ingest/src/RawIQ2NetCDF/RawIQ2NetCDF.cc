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
// RawIQ2NetCDF.cc
//
// RawIQ2NetCDF object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////
//
// RawIQ2NetCDF splits a netCDF radar file into PPI files.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <ctime>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <netcdf.hh>
#include "RawIQ2NetCDF.hh"

using namespace std;

// Constructor

RawIQ2NetCDF::RawIQ2NetCDF(int argc, char **argv)

{

  _input = NULL;
  isOK = true;

  // set programe name

  _progName = "RawIQ2NetCDF";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that file list set in archive mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: RawIQ2NetCDF::RawIQ2NetCDF." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  return;

}

// destructor

RawIQ2NetCDF::~RawIQ2NetCDF()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RawIQ2NetCDF::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  if (_params.debug) {
    cerr << "netCDF lib version: " << nc_inq_libvers() << endl;
  }

  // loop until end of data
  
  char *inputPath;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Processing file");
    
    if (_processFile(inputPath)) {
      cerr << "ERROR = RawIQ2NetCDF::Run" << endl;
      cerr << "  Processing file: " << inputPath << endl;
      iret = -1;
    }
    
  }
  
  return iret;

}

///////////////////////////////
// process file

int RawIQ2NetCDF::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // open file
  
  FILE *in;
  if ((in = fopen(input_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - RawIQ2NetCDF::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // read in data
  
  vector<iq_t> iqVec;
  while (!feof(in)) {
    char line[128];
    if (fgets(line, 128, in) == NULL) {
      break;
    }
    iq_t iq;
    if (sscanf(line, "%g %g", &iq.i, &iq.q) != 2) {
      break;
    }
    iqVec.push_back(iq);
  } // while
  fclose(in);

  // compute output and tmp paths

  Path inPath(input_path);
  string outName = inPath.getFile();
  outName += ".nc";
  string outPath = _params.output_dir;
  outPath += PATH_DELIM;
  outPath += outName;
  string tmpPath = outPath;
  tmpPath += ".tmp";
  
  if (_params.debug) {
    cerr << "Output file path: " << outPath << endl;
  }

  // write out tmp file

  if (_writeTmpFile(tmpPath, iqVec)) {
    cerr << "ERROR - RawIQ2NetCDF::_processFile" << endl;
    cerr << "  Cannot write tmp file: " << tmpPath << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "  Wrote tmp file: " << tmpPath << endl;
  }

  // move the tmp file to final name
  
  if (rename(tmpPath.c_str(), outPath.c_str())) {
    int errNum = errno;
    cerr << "ERROR - RawIQ2NetCDF::_processFile" << endl;
    cerr << "  Cannot rename file: " << tmpPath << endl;
    cerr << "             to file: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "  Renamed file: " << tmpPath << endl;
    cerr << "       to file: " << outPath << endl;
  }

  // write latest data info

  if (_params.write_ldata_info_file) {
    LdataInfo ldata(_params.output_dir);
    ldata.setDataFileExt("nc");
    ldata.setWriter("RawIQ2NetCDF");
    ldata.setRelDataPath(outName.c_str());
    int now = time(NULL);
    if (ldata.write(now)) {
      cerr << "ERROR - RawIQ2NetCDF::_processFile" << endl;
      cerr << "  Cannot write ldata file to dir: "
	   << _params.output_dir << endl;
      return -1;
    }
  }

  return 0;

}

////////////////////////////////////////
// write out the netDCF file to tmp name
//
// Returns 0 on success, -1 on failure

int RawIQ2NetCDF::_writeTmpFile(const string &tmpPath,
				const vector<iq_t> &iqVec)

{

  // ensure directory exists

  if (ta_makedir_recurse(_params.output_dir)) {
    cerr << "ERROR - RawIQ2NetCDF::_writeTmpFile" << endl;
    cerr << "  Cannot make output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  ////////////////////////
  // create NcFile object
  
  NcError err(NcError::verbose_nonfatal);
  
  NcFile out(tmpPath.c_str(), NcFile::Replace);
  if (!out.is_valid()) {
    cerr << "ERROR - RawIQ2NetCDF::_writeTmpFile" << endl;
    cerr << "  Cannot create file: " << tmpPath << endl;
    return -1;
  }
  int iret = 0;

  /////////////////////
  // global attributes
  
  int nTimes = iqVec.size() / _params.ngates;
  int startingSample = 0;
  int endingSample = startingSample + nTimes - 1;
  int startGate = _params.start_gate;
  int endGate = startGate + _params.ngates - 1;
  
  char desc[1024];
  sprintf(desc,
	  "Radar time series reformatted by RawIQ2NetCDF\n"
	  "Starting Sample =%d, Ending Sample =%d, "
	  "Start Gate= %d, End Gate = %d\n"
	  "Azimuth = %.2f, Elevation = %.2f\n",
	  startingSample, endingSample, startGate, endGate,
	  _params.start_az, _params.elevation);
  out.add_att("Description", desc);
  out.add_att("FirstGate", startGate);
  out.add_att("LastGate", endGate);

  //////////////////
  // add dimensions
  
  NcDim *gatesDim = out.add_dim("gates", _params.ngates);
  //int gatesId = gatesDim->id();

  NcDim *frtimeDim = out.add_dim("frtime");
  //int frtimeId = frtimeDim->id();

  /////////////////////////////////
  // add vars and their attributes

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
    float idata[iqVec.size()];
    for (size_t jj = 0; jj < iqVec.size(); jj++) {
      idata[jj] = iqVec[jj].i;
    }
    long edges[2];
    edges[0] = nTimes;
    edges[1] = _params.ngates;
    iVar->put(idata, edges);
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
    float qdata[iqVec.size()];
    for (size_t jj = 0; jj < iqVec.size(); jj++) {
      qdata[jj] = iqVec[jj].q;
    }
    long edges[2];
    edges[0] = nTimes;
    edges[1] = _params.ngates;
    qVar->put(qdata, edges);
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
      azimuths[jj] = (float) (_params.start_az + jj * _params.delta_az);
    }
    long edge = nTimes;
    azVar->put(azimuths, &edge);
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
      elevations[jj] = (float) _params.elevation;
    }
    long edge = nTimes;
    elVar->put(elevations, &edge);
  }

  // PRT variable

  NcVar *prtVar = out.add_var("Prt", ncInt, frtimeDim);
  prtVar->add_att("long_name", "Pulse Repetition Time");
  prtVar->add_att("units", "microseconds");
  prtVar->add_att("valid_range", 1000000);
  {
    int prts[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      prts[jj] = _params.prt;
    }
    long edge = nTimes;
    prtVar->put(prts, &edge);
  }
  
  // Time variable

  NcVar *timeVar = out.add_var("Time", ncDouble, frtimeDim);
  timeVar->add_att("long_name", "Date/Time value");
  timeVar->add_att("units", "days since 0000-01-01");
  timeVar->add_att("_FillValue", 0.0);
  time_t now = time(NULL);
  {
    double times[nTimes];
    for (int jj = 0; jj < nTimes; jj++) {
      times[jj] = (double) (now + jj * _params.delta_time) / 86400.0;
    }
    long edge = nTimes;
    timeVar->put(times, &edge);
  }

  return iret;

}
