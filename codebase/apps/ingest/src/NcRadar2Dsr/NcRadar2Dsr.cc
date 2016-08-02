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
// NcRadar2Dsr.cc
//
// NcRadar2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
///////////////////////////////////////////////////////////////
//
// NcRadar2Dsr reads netCDF radar beam-by-beam files and copies
// the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include "NcRadar2Dsr.hh"
#include "File2Fmq.hh"
using namespace std;

// Constructor

NcRadar2Dsr::NcRadar2Dsr(int argc, char **argv)

{

  _input = NULL;
  _prevVolNum = -1;
  _volNum = 0;
  _tiltNum = 0;
  isOK = true;

  // set programe name

  _progName = "NcRadar2Dsr";
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

  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: NcRadar2Dsr::NcRadar2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: NcRadar2Dsr::NcRadar2Dsr." << endl;
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

  // initialize the output queue

  if (_rQueue.init(_params.output_fmq_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_fmq_compress,
		   _params.output_fmq_nslots,
		   _params.output_fmq_size)) {
    cerr << "ERROR - NcRadar2Dsr" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    isOK = false;
    return;
  }

  if (_params.output_fmq_compress) {
    _rQueue.setCompressionMethod(TA_COMPRESSION_ZLIB);
  }

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  return;

}

// destructor

NcRadar2Dsr::~NcRadar2Dsr()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int NcRadar2Dsr::Run ()
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
	  cerr << "ERROR = NcRadar2Dsr::Run" << endl;
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
	cerr << "ERROR = NcRadar2Dsr::Run" << endl;
	cerr << "  Processing file: " << inputPath << endl;
	iret = -1;
      }
      
    }

  } // if (_params.mode == Params::SIMULATE)

  // put final end of tilt and volume flags

  _rQueue.putEndOfTilt(_tiltNum, _endTime);
  _rQueue.putEndOfVolume(_volNum, _endTime);

  return iret;

}

///////////////////////////////
// process file

int NcRadar2Dsr::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  // open file

  NcFile ncf(input_path);
  if (!ncf.is_valid()) {
    cerr << "ERROR - NcRadar2Dsr::_processFile" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }

  // declare an error object

  NcError err(NcError::silent_nonfatal);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printFile(ncf);
  }

  // check that this is a valid file

  if (_checkFile(ncf)) {
    cerr << "ERROR - NcRadar2Dsr::_processFile" << endl;
    cerr << "  Not a valid radar file for splitting" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }
  
  // set up the file output object
  
  File2Fmq f2fmq(_params, ncf, _rQueue);
  
  // put start and end of vol flags as appropriate

  if (_prevVolNum != _volNum) {
    if (_prevVolNum >= 0) {
      _rQueue.putEndOfVolume(_prevVolNum, _startTime);
    }
    _rQueue.putStartOfVolume(_volNum, _startTime);
    _tiltNum = 0;
    _prevVolNum = _volNum;
  } else {
    _tiltNum++;
  }

  // put start of tilt flag

  _rQueue.putStartOfTilt(_tiltNum, _startTime);
  
  // put the params

  if (f2fmq.writeParams()) {
    cerr << "ERROR - NcRadar2Dsr::_processFile" << endl;
    cerr << "  Cannot write the radar and field params to the queue" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }
  
  // put the beam data to the queue

  if (f2fmq.writeBeams(_volNum, _tiltNum, _startTime)) {
    cerr << "ERROR - NcRadar2Dsr::_processFile" << endl;
    cerr << "  Cannot write the beam data to the queue" << endl;
    cerr << "  File: " << input_path << endl;
    return -1;
  }
  
  // put end of tilt flag

  _rQueue.putEndOfTilt(_tiltNum, _endTime);
  
  return 0;

}

//////////////////////////////////
// Check that this is a valid file
//
// Returns 0 on success, -1 on failure

int NcRadar2Dsr::_checkFile(NcFile &ncf)

{

  if (ncf.rec_dim() == NULL) {
    cerr << "ERROR - NcRadar2Dsr::_checkFile" << endl;
    cerr << "  Time dimension missing" << endl;
    return -1;
  }
  
  if (ncf.get_dim("maxCells") == NULL) {
    cerr << "ERROR - NcRadar2Dsr::_checkFile" << endl;
    cerr << "  maxCells dimension missing" << endl;
    return -1;
  }
  
  if (ncf.get_var("time_offset") == NULL) {
    cerr << "ERROR - NcRadar2Dsr::_checkFile" << endl;
    cerr << "  time_offset variable missing" << endl;
    return -1;
  }

  if (ncf.get_var("Azimuth") == NULL) {
    cerr << "ERROR - NcRadar2Dsr::_checkFile" << endl;
    cerr << "  Azimuth variable missing" << endl;
    return -1;
  }

  if (ncf.get_var("Elevation") == NULL) {
    cerr << "ERROR - NcRadar2Dsr::_checkFile" << endl;
    cerr << "  Elevation variable missing" << endl;
    return -1;
  }

  NcVar *btime = ncf.get_var("base_time");
  if (btime == NULL) {
    cerr << "ERROR - NcRadar2Dsr::_checkFile" << endl;
    cerr << "  base_time variable missing" << endl;
    return -1;
  }
  _startTime = btime->as_long(0);

  NcVar *toffsetVar = ncf.get_var("time_offset");
  if (toffsetVar == NULL) {
    cerr << "ERROR - NcRadar2Dsr::_checkFile" << endl;
    cerr << "  time_offset variable missing" << endl;
    return -1;
  }
  NcValues *toffsetVals = toffsetVar->values();
  double *toffsets = (double *) toffsetVals->base();
  _endTime = _startTime + (int) (toffsets[toffsetVar->num_vals()] + 0.5);
  delete toffsetVals;

  // modify times in simulate mode

  if (_params.mode == Params::SIMULATE) {
    int tdiff = time(NULL) - _endTime;
    _endTime += tdiff;
    _startTime += tdiff;
  }
  
  NcAtt *vnum = ncf.get_att("Volume_Number");
  if (vnum == NULL) {
    cerr << "ERROR - NcRadar2Dsr::_checkFile" << endl;
    cerr << "  Volume_Number attribute missing" << endl;
    return -1;
  }
  _volNum = vnum->as_int(0);
  delete vnum;

  return 0;

}

///////////////////////////////
// print data in file

void NcRadar2Dsr::_printFile(NcFile &ncf)

{

  cout << "ndims: " << ncf.num_dims() << endl;
  cout << "nvars: " << ncf.num_vars() << endl;
  cout << "ngatts: " << ncf.num_atts() << endl;
  NcDim *unlimd = ncf.rec_dim();
  cout << "unlimdimid: " << unlimd->size() << endl;
  
  // dimensions

  NcDim *dims[ncf.num_dims()];
  for (int idim = 0; idim < ncf.num_dims(); idim++) {
    dims[idim] = ncf.get_dim(idim);

    cout << endl;
    cout << "Dim #: " << idim << endl;
    cout << "  Name: " << dims[idim]->name() << endl;
    cout << "  Length: " << dims[idim]->size() << endl;
    cout << "  Is valid: " << dims[idim]->is_valid() << endl;
    cout << "  Is unlimited: " << dims[idim]->is_unlimited() << endl;
    
  } // idim
  
  cout << endl;

  // global attributes

  cout << "Global attributes:" << endl;

  for (int iatt = 0; iatt < ncf.num_atts(); iatt++) {
    cout << "  Att num: " << iatt << endl;
    NcAtt *att = ncf.get_att(iatt);
    _printAtt(att);
    delete att;
  }

  // loop through variables

  NcVar *vars[ncf.num_vars()];
  for (int ivar = 0; ivar < ncf.num_vars(); ivar++) {

    vars[ivar] = ncf.get_var(ivar);
    cout << endl;
    cout << "Var #: " << ivar << endl;
    cout << "  Name: " << vars[ivar]->name() << endl;
    cout << "  Is valid: " << vars[ivar]->is_valid() << endl;
    cout << "  N dims: " << vars[ivar]->num_dims();
    NcDim *vdims[vars[ivar]->num_dims()];
    if (vars[ivar]->num_dims() > 0) {
      cout << ": (";
      for (int ii = 0; ii < vars[ivar]->num_dims(); ii++) {
	vdims[ii] = vars[ivar]->get_dim(ii);
	cout << " " << vdims[ii]->name();
	if (ii != vars[ivar]->num_dims() - 1) {
	  cout << ", ";
	}
      }
      cout << " )";
    }
    cout << endl;
    cout << "  N atts: " << vars[ivar]->num_atts() << endl;
    
    for (int iatt = 0; iatt < vars[ivar]->num_atts(); iatt++) {

      cout << "  Att num: " << iatt << endl;
      NcAtt *att = vars[ivar]->get_att(iatt);
      _printAtt(att);
      delete att;

    } // iatt

    cout << endl;
    _printVarVals(vars[ivar]);
    
  } // ivar
  
}

/////////////////////
// print an attribute

void NcRadar2Dsr::_printAtt(NcAtt *att)

{

  cout << "    Name: " << att->name() << endl;
  cout << "    Num vals: " << att->num_vals() << endl;
  cout << "    Type: ";
  
  NcValues *values = att->values();

  switch(att->type()) {
    
  case ncNoType: {
    cout << "No type: ";
  }
  break;
  
  case ncByte: {
    cout << "BYTE: ";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case ncChar: {
    cout << "CHAR: ";
    char vals[att->num_vals() + 1];
    MEM_zero(vals);
    memcpy(vals, values->base(), att->num_vals());
    cout << vals;
  }
  break;
  
  case ncShort: {
    cout << "SHORT: ";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case ncInt: {
    cout << "INT: ";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case ncFloat: {
    cout << "FLOAT: ";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case ncDouble: {
    cout << "DOUBLE: ";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < att->num_vals(); ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  }
  
  cout << endl;

  delete values;

}

    
void NcRadar2Dsr::_printVarVals(NcVar *var)

{

  int nprint = var->num_vals();
  if (nprint > 100) {
    nprint = 100;
  }

  NcValues *values = var->values();

  cout << "  Variable vals:";
  
  switch(var->type()) {
    
  case ncNoType: {
  }
  break;
  
  case ncByte: {
    cout << "(byte)";
    unsigned char *vals = (unsigned char *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case ncChar: {
    cout << "(char)";
    char str[nprint + 1];
    MEM_zero(str);
    memcpy(str, values->base(), nprint);
    cout << " " << str;
  }
  break;
  
  case ncShort: {
    cout << "(short)";
    short *vals = (short *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case ncInt: {
    cout << "(int)";
    int *vals = (int *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case ncFloat: {
    cout << "(float)";
    float *vals = (float *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  case ncDouble: {
    cout << "(double)";
    double *vals = (double *) values->base();
    for (long ii = 0; ii < nprint; ii++) {
      cout << " " << vals[ii];
    }
  }
  break;
  
  }
  
  cout << endl;

  delete values;

}

