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
// NcRadarSplit.cc
//
// NcRadarSplit object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////
//
// NcRadarSplit splits a netCDF radar file into PPI files.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include "NcRadarSplit.hh"
#include "PpiFile.hh"
using namespace std;

// Constructor

NcRadarSplit::NcRadarSplit(int argc, char **argv)

{

  _input = NULL;
  _volNum = 0;
  _prevTiltNum = -1;

  isOK = true;

  // set programe name

  _progName = "NcRadarSplit";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: NcRadarSplit::NcRadarSplit." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: NcRadarSplit::NcRadarSplit." << endl;
    cerr << "  Mode is SIMULATE."; 
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

NcRadarSplit::~NcRadarSplit()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int NcRadarSplit::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  if (_params.debug) {
    cerr << "netCDF lib version: " << nc_inq_libvers() << endl;
  }

  if (_params.mode == Params::SIMULATE) {

    // simulate mode - go through the file list repeatedly

    while (true) {
      
      char *inputPath;
      _input->reset();
      while ((inputPath = _input->next()) != NULL) {
	PMU_auto_register("Simulate mode");
	if (_processFile(inputPath)) {
	  cerr << "ERROR = NcRadarSplit::Run" << endl;
	  cerr << "  Processing file: " << inputPath << endl;
	  iret = -1;
	}
      } // while

    }

  } else {

    // loop until end of data
    
    char *inputPath;
    while ((inputPath = _input->next()) != NULL) {
      
      PMU_auto_register("Non-simulate mode");
      
      if (_processFile(inputPath)) {
	cerr << "ERROR = NcRadarSplit::Run" << endl;
	cerr << "  Processing file: " << inputPath << endl;
	iret = -1;
      }
      
    }

  } // if (_params.mode == Params::SIMULATE)
    
  return iret;

}

///////////////////////////////
// process file

int NcRadarSplit::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // open file

  NcFile ncf(input_path);
  if (!ncf.is_valid()) {
    cerr << "ERROR - NcRadarSplit::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // declare an error object

  NcError err(NcError::silent_nonfatal);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printFile(ncf);
  }

  // check that this is a valid file to split

  if (_checkFile(ncf)) {
    cerr << "ERROR - NcRadarSplit::_processFile" << endl;
    cerr << "  Not a valid radar file for splitting" << endl;
    return -1;
  }

  // find places to split file into ppis

  _findPpis(ncf);

  // split the file

  if (_doSplit(ncf)) {
    cerr << "ERROR - NcRadarSplit::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////
// Check that this is a valid file
//
// Returns 0 on success, -1 on failure

int NcRadarSplit::_checkFile(NcFile &ncf)

{

  if (ncf.rec_dim() == NULL) {
    cerr << "ERROR - NcRadarSplit::_checkFile" << endl;
    cerr << "  Time dimension missing" << endl;
    return -1;
  }
  
  if (ncf.get_dim("maxCells") == NULL) {
    cerr << "ERROR - NcRadarSplit::_checkFile" << endl;
    cerr << "  maxCells dimension missing" << endl;
    return -1;
  }
  
  if (ncf.get_var("base_time") == NULL) {
    cerr << "ERROR - NcRadarSplit::_checkFile" << endl;
    cerr << "  base_time variable missing" << endl;
    return -1;
  }

  if (ncf.get_var("Fixed_Angle") == NULL) {
    cerr << "ERROR - NcRadarSplit::_checkFile" << endl;
    cerr << "  base_time variable missing" << endl;
    return -1;
  }

  if (ncf.get_var("time_offset") == NULL) {
    cerr << "ERROR - NcRadarSplit::_checkFile" << endl;
    cerr << "  time_offset variable missing" << endl;
    return -1;
  }

  if (ncf.get_var("Azimuth") == NULL) {
    cerr << "ERROR - NcRadarSplit::_checkFile" << endl;
    cerr << "  Azimuth variable missing" << endl;
    return -1;
  }

  if (ncf.get_var("Elevation") == NULL) {
    cerr << "ERROR - NcRadarSplit::_checkFile" << endl;
    cerr << "  Elevation variable missing" << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////
// Find the ppis in the file

void NcRadarSplit::_findPpis(NcFile &ncf)

{

  _ppis.clear();

  int nTimes = ncf.rec_dim()->size();
  time_t baseTime = ncf.get_var("base_time")->as_long(0);

  NcValues *timeOffsetVals = ncf.get_var("time_offset")->values();
  double *timeOffsets = (double *) timeOffsetVals->base();

  NcValues *elevVals = ncf.get_var("Elevation")->values();
  float *elevations = (float *) elevVals->base();

  NcValues *azVals = ncf.get_var("Azimuth")->values();
  float *azimuths = (float *) azVals->base();

  int count = 0;
  int startBeam = 0;

  for (int ii = 0; ii < nTimes; ii++) {

    int tiltNum = _getTiltNum(elevations[ii]);
    
    if (ii == 0 && _prevTiltNum >= 0 && tiltNum < _prevTiltNum) {
      _volNum++;
    }

    if (tiltNum >= 0) {

      if (tiltNum == _prevTiltNum && ii < (nTimes - 1)) {

	count++;

      } else {

	int endBeam = ii - 1;
	if (ii == nTimes - 1) {
	  endBeam = ii;
	}
	
	if (count > 100) {

	  ppi_t ppi;
	  ppi.startBeam = startBeam;
	  ppi.endBeam = endBeam;
	  ppi.tiltNum = _prevTiltNum;
	  ppi.volNum = _volNum;

	  _ppis.push_back(ppi);

	  if (tiltNum < _prevTiltNum) {
	    _volNum++;
	  }

	} // if (count > 100)

	_prevTiltNum = tiltNum;
	startBeam = ii;
	count = 0;

      } // if (tiltNum == _prevTiltNum ...

    } // if (tiltNum >= 0) 

  } // ii

  if (_params.debug) {
    for (size_t ii = 0; ii < _ppis.size(); ii++) {

      time_t startTime = baseTime +
	(int) (timeOffsets[_ppis[ii].startBeam] + 0.5);
      time_t endTime = baseTime +
	(int) (timeOffsets[_ppis[ii].endBeam] + 0.5);
      
      cerr << "Valid tilt, vol num: " << _ppis[ii].volNum
	   << ", tilt num: " << _ppis[ii].tiltNum << endl;
      cerr << "  Start beam: " << _ppis[ii].startBeam
	   << ", time: " << DateTime::str(startTime).c_str()
	   << ", el: " << elevations[_ppis[ii].startBeam]
	   << ", az: " << azimuths[_ppis[ii].startBeam]
	   << endl;
      cerr << "  End   beam: " << _ppis[ii].endBeam
	   << ", time: " << DateTime::str(endTime).c_str()
	   << ", el: " << elevations[_ppis[ii].endBeam]
	   << ", az: " << azimuths[_ppis[ii].endBeam]
	   << endl;

    } // ii
  }

  delete timeOffsetVals;
  delete elevVals;
  delete azVals;

}

///////////////////////////////
// print data in file

void NcRadarSplit::_printFile(NcFile &ncf)

{

  cerr << "ndims: " << ncf.num_dims() << endl;
  cerr << "nvars: " << ncf.num_vars() << endl;
  cerr << "ngatts: " << ncf.num_atts() << endl;
  NcDim *unlimd = ncf.rec_dim();
  cerr << "unlimdimid: " << unlimd->size() << endl;
  
  // dimensions

  NcDim *dims[ncf.num_dims()];
  for (int idim = 0; idim < ncf.num_dims(); idim++) {
    dims[idim] = ncf.get_dim(idim);

    cerr << endl;
    cerr << "Dim #: " << idim << endl;
    cerr << "  Name: " << dims[idim]->name() << endl;
    cerr << "  Length: " << dims[idim]->size() << endl;
    cerr << "  Is valid: " << dims[idim]->is_valid() << endl;
    cerr << "  Is unlimited: " << dims[idim]->is_unlimited() << endl;
    
  } // idim
  
  cerr << endl;

  // global attributes

  cerr << "Global attributes:" << endl;

  for (int iatt = 0; iatt < ncf.num_atts(); iatt++) {
    cerr << "  Att num: " << iatt << endl;
    NcAtt *att = ncf.get_att(iatt);
    _printAtt(att);
    delete att;
  }

  // loop through variables

  NcVar *vars[ncf.num_vars()];
  for (int ivar = 0; ivar < ncf.num_vars(); ivar++) {

    vars[ivar] = ncf.get_var(ivar);
    cerr << endl;
    cerr << "Var #: " << ivar << endl;
    cerr << "  Name: " << vars[ivar]->name() << endl;
    cerr << "  Is valid: " << vars[ivar]->is_valid() << endl;
    cerr << "  N dims: " << vars[ivar]->num_dims();
    NcDim *vdims[vars[ivar]->num_dims()];
    if (vars[ivar]->num_dims() > 0) {
      cerr << ": (";
      for (int ii = 0; ii < vars[ivar]->num_dims(); ii++) {
	vdims[ii] = vars[ivar]->get_dim(ii);
	cerr << " " << vdims[ii]->name();
	if (ii != vars[ivar]->num_dims() - 1) {
	  cerr << ", ";
	}
      }
      cerr << " )";
    }
    cerr << endl;
    cerr << "  N atts: " << vars[ivar]->num_atts() << endl;
    
    for (int iatt = 0; iatt < vars[ivar]->num_atts(); iatt++) {

      cerr << "  Att num: " << iatt << endl;
      NcAtt *att = vars[ivar]->get_att(iatt);
      _printAtt(att);
      delete att;

    } // iatt

    cerr << endl;
    _printVarVals(vars[ivar]);
    
  } // ivar
  
}

/////////////////////
// print an attribute

void NcRadarSplit::_printAtt(NcAtt *att)

{

  cerr << "    Name: " << att->name() << endl;
  cerr << "    Num vals: " << att->num_vals() << endl;
  cerr << "    Type: ";
  
  NcValues *values = att->values();

  switch(att->type()) {
    
  case ncNoType: {
    cerr << "No type: ";
  }
  break;
  
  case ncByte: {
    cerr << "BYTE: ";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  case ncChar: {
    cerr << "CHAR: ";
    char vals[att->num_vals() + 1];
    MEM_zero(vals);
    memcpy(vals, values->base(), att->num_vals());
    cerr << vals;
  }
  break;
  
  case ncShort: {
    cerr << "SHORT: ";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  case ncInt: {
    cerr << "INT: ";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  case ncFloat: {
    cerr << "FLOAT: ";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  case ncDouble: {
    cerr << "DOUBLE: ";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  }
  
  cerr << endl;

  delete values;

}

    
void NcRadarSplit::_printVarVals(NcVar *var)

{

  int nprint = var->num_vals();
  if (nprint > 100) {
    nprint = 100;
  }

  NcValues *values = var->values();

  cerr << "  Variable vals:";
  
  switch(var->type()) {
    
  case ncNoType: {
  }
  break;
  
  case ncByte: {
    cerr << "(byte)";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  case ncChar: {
    cerr << "(char)";
    char str[nprint + 1];
    MEM_zero(str);
    memcpy(str, values->base(), nprint);
    cerr << " " << str;
  }
  break;
  
  case ncShort: {
    cerr << "(short)";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  case ncInt: {
    cerr << "(int)";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  case ncFloat: {
    cerr << "(float)";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  case ncDouble: {
    cerr << "(double)";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cerr << " " << vals[ii];
    }
  }
  break;
  
  }
  
  cerr << endl;

  delete values;

}

///////////////////////
// get the tilt number
//
// returns -1 on failure

int NcRadarSplit::_getTiltNum(double elevation)

{

  for (int ii = 0; ii < _params.elev_limits_n; ii++) {

    if (elevation > _params._elev_limits[ii].lower_elev &&
	elevation <= _params._elev_limits[ii].upper_elev) {
      return ii;
    }

  } // ii

  return -1;

}

////////////////////////////////////////
// Split the file, writing output files
//
// Returns 0 on success, -1 on failure

int NcRadarSplit::_doSplit(NcFile &ncf)

{

  for (size_t ii = 0; ii < _ppis.size(); ii++) {

    PMU_auto_register("Writing ppi file");
    
    time_t start = time(NULL);

    PpiFile ppi(_params, ncf,
		_ppis[ii].startBeam, _ppis[ii].endBeam,
		_ppis[ii].tiltNum, _ppis[ii].volNum);

    if (ppi.checkFile()) {
      cerr << "ERROR - NcRadarSplit::_doSplit" << endl;
      cerr << "  Bad input file" << endl;
      return -1;
    }

    ppi.setVars();
    
    if (ppi.write()) {
      cerr << "ERROR - NcRadarSplit::_doSplit" << endl;
      cerr << "  Cannot write output file" << endl;
      return -1;
    }

    if (_params.mode == Params::SIMULATE) {
      time_t end = time(NULL);
      int timeSoFar = end - start;
      int timeLeft = _params.simulate_rate_secs - timeSoFar;
      for (int i = 0; i < timeLeft; i++) {
	PMU_auto_register("zzzz ...");
	umsleep(1000);
      }
    }

  } // ii

  return 0;

}

