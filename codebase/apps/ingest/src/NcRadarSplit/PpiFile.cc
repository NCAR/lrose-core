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
// PpiFile.cc
//
// PpiFile object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////
//
// PpiFile creates a single PPI data set in an NcFile object and
// writes it.
//
////////////////////////////////////////////////////////////////

#include "PpiFile.hh"
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/os_config.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <math.h>
using namespace std;

// Constructor

PpiFile::PpiFile(const Params &params,
		 const Nc3File &ncf_in,
		 int start_beam,
		 int end_beam,
		 int tilt_num,
		 int vol_num) :
  _params(params),
  _ncfIn(ncf_in),
  _startBeam(start_beam),
  _endBeam(end_beam),
  _tiltNum(tilt_num),
  _volNum(vol_num)

{

  _baseTimeVar = NULL;
  _fixedAngleVar = NULL;
  _timeOffsetVar = NULL;
  _azimuthVar = NULL;
  _elevationVar = NULL;
  _timeOffsetVals = NULL;
  _elevationVals = NULL;
  _azimuthVals = NULL;

  if (_params.debug) {
    cerr << "PpiFile" << endl;
    cerr << "  startBeam: " << _startBeam << endl;
    cerr << "  endBeam: " << _endBeam << endl;
    cerr << "  tiltNum: " << _tiltNum << endl;
    cerr << "  volNum: " << _volNum << endl;
  }
  
}

// destructor

PpiFile::~PpiFile()

{

  if (_timeOffsetVals) {
    delete _timeOffsetVals;
  }
  if (_elevationVals) {
    delete _elevationVals;
  }
  if (_azimuthVals) {
    delete _azimuthVals;
  }
  
}

//////////////////////////////////
// Check that this is a valid file
//
// Returns 0 on success, -1 on failure

int PpiFile::checkFile()

{

  if (_ncfIn.rec_dim() == NULL) {
    cerr << "ERROR - PpiFile::checkFile" << endl;
    cerr << "  Time dimension missing" << endl;
    return -1;
  }
  
  if (_ncfIn.get_dim("maxCells") == NULL) {
    cerr << "ERROR - PpiFile::checkFile" << endl;
    cerr << "  maxCells dimension missing" << endl;
    return -1;
  }
  
  if (_ncfIn.get_var("base_time") == NULL) {
    cerr << "ERROR - PpiFile::checkFile" << endl;
    cerr << "  base_time variable missing" << endl;
    return -1;
  }

  if (_ncfIn.get_var("Fixed_Angle") == NULL) {
    cerr << "ERROR - PpiFile::checkFile" << endl;
    cerr << "  base_time variable missing" << endl;
    return -1;
  }

  if (_ncfIn.get_var("time_offset") == NULL) {
    cerr << "ERROR - PpiFile::checkFile" << endl;
    cerr << "  time_offset variable missing" << endl;
    return -1;
  }

  if (_ncfIn.get_var("Azimuth") == NULL) {
    cerr << "ERROR - PpiFile::checkFile" << endl;
    cerr << "  Azimuth variable missing" << endl;
    return -1;
  }

  if (_ncfIn.get_var("Elevation") == NULL) {
    cerr << "ERROR - PpiFile::checkFile" << endl;
    cerr << "  Elevation variable missing" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////
// Set the variables
//
// Returns 0 on success, -1 on failure

void PpiFile::setVars()

{

  _timeDimId = _ncfIn.rec_dim()->id();
  _maxCellsDimId = _ncfIn.get_dim("maxCells")->id();
  
  _nTimes = _ncfIn.rec_dim()->size();
  _maxCells = _ncfIn.get_dim("maxCells")->size();
  _baseTime = _ncfIn.get_var("base_time")->as_long(0);

  _baseTimeVar = _ncfIn.get_var("base_time");
  _fixedAngleVar = _ncfIn.get_var("Fixed_Angle");
  _timeOffsetVar = _ncfIn.get_var("time_offset");
  _azimuthVar = _ncfIn.get_var("Azimuth");
  _elevationVar = _ncfIn.get_var("Elevation");

  _timeOffsetVals = _ncfIn.get_var("time_offset")->values();
  _timeOffsetData = (double *) _timeOffsetVals->base();
  
  _elevationVals = _ncfIn.get_var("Elevation")->values();
  _elevationData = (float *) _elevationVals->base();
  
  _azimuthVals = _ncfIn.get_var("Azimuth")->values();
  _azimuthData = (float *) _azimuthVals->base();

}

////////////////////////////////////////
// Write ppi file
//
// Returns 0 on success, -1 on failure

int PpiFile::write()

{

  // compute file name

  time_t startTime;
  if (_params.mode == Params::SIMULATE) {
    double scanLen =
      _timeOffsetData[_endBeam] - _timeOffsetData[_startBeam];
    startTime = time(NULL) - (int) (scanLen + 0.5);
  } else {
    startTime = _baseTime + (int) (_timeOffsetData[_startBeam] + 0.5);
  }
  DateTime dtime(startTime);

  string instName;
  Nc3Att *instNameAtt = _ncfIn.get_att("Instrument_Name");
  if (instNameAtt != NULL) {
    instName = instNameAtt->as_string(0);
  }

  string scanMode;
  Nc3Att *scanModeAtt = _ncfIn.get_att("Scan_Mode");
  if (scanModeAtt != NULL) {
    scanMode = scanModeAtt->as_string(0);
  }

  float fixedAngle = 0.05;
  int nBeams = _endBeam - _startBeam + 1;
  for (int ii = _startBeam; ii <= _endBeam; ii++) {
    if (_elevationData[ii] >= 0 &&
	_elevationData[ii] <= 90) {
      fixedAngle += _elevationData[ii] / nBeams;
    }
  }
  fixedAngle = rint(fixedAngle * 20.0) / 20.0;

  char outputName[MAX_PATH_LEN];

  sprintf(outputName,
	  "ncswp_%s_%.4d%.2d%.2d_%.2d%.2d%.2d.000_v%d_s%d_%.1f_%s.nc",
	  instName.c_str(),
	  dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	  dtime.getHour(), dtime.getMin(), dtime.getSec(),
	  _volNum, _tiltNum,
	  fixedAngle,
	  scanMode.c_str());

  if (_params.debug) {
    cerr << "Output file name: " << outputName << endl;
  }
 
  // ensure directories exists

  if (ta_makedir_recurse(_params.output_dir)) {
    cerr << "ERROR - PpiFile::write" << endl;
    cerr << "  Cannot make output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  if (ta_makedir_recurse(_params.tmp_dir)) {
    cerr << "ERROR - PpiFile::write" << endl;
    cerr << "  Cannot make tmp dir: " << _params.tmp_dir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  // compute path names

  char tmpPath[MAX_PATH_LEN];
  sprintf(tmpPath, "%s%s%s", _params.tmp_dir, PATH_DELIM, outputName);

  char outputPath[MAX_PATH_LEN];
  sprintf(outputPath, "%s%s%s", _params.output_dir, PATH_DELIM, outputName);

  // create the tmp file

  if (_createTmp(tmpPath, startTime, fixedAngle, nBeams)) {
    cerr << "ERROR - PpiFile::write" << endl;
    cerr << "  Cannot write tmp file: " << tmpPath << endl;
    unlink(tmpPath);
    return -1;
  }

  // move the file to the output path

  if (rename(tmpPath, outputPath)) {
    cerr << "ERROR - PpiFile::write" << endl;
    cerr << "  Cannot rename tmp file: " << tmpPath << endl;
    cerr << "                to  file: " << outputPath << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  // write latest data info file if required

  if (_params.write_ldata_info_file) {

    Path outP(outputPath);

    LdataInfo ldata(outP.getDirectory().c_str());
    ldata.setDataFileExt(outP.getExt().c_str());
    ldata.setWriter("NcRadarSplit");
    ldata.setRelDataPath(outP.getFile().c_str());

    if (ldata.write(startTime)) {
      cerr << "ERROR - PpiFile::write" << endl;
      cerr << "  Cannot write ldata file to dir: "
	   << outP.getDirectory() << endl;
      return -1;
    }
    
  }
  
  return 0;
  
}


////////////////////////////////////////
// Write tmp file
//
// Returns 0 on success, -1 on failure

int PpiFile::_createTmp(const char *tmp_path,
			time_t start_time,
			float fixed_angle,
			int n_beams)

{

  // create NcFile object

  Nc3Error err(Nc3Error::verbose_nonfatal);

  Nc3File out(tmp_path, Nc3File::Replace);
  if (!out.is_valid()) {
    cerr << "ERROR - PpiFile::write" << endl;
    cerr << "  Cannot create file: " << tmp_path << endl;
    return -1;
  }
  int iret = 0;

  // add dimensions

  int timeId = -1;
  int maxCellsId = -1;
  for (int idim = 0; idim < _ncfIn.num_dims(); idim++) {
    Nc3Dim *dim = _ncfIn.get_dim(idim);
    if (dim == _ncfIn.rec_dim()) {
      Nc3Dim *timeDim = out.add_dim("Time");
      timeId = timeDim->id();
    } else {
      Nc3Dim *outDim = out.add_dim(dim->name(), dim->size());
      if (!strcmp(outDim->name(), "maxCells")) {
	maxCellsId = outDim->id();
      }
    }
  } // idim

  // add global attributes

  for (int iatt = 0; iatt < _ncfIn.num_atts(); iatt++) {
     Nc3Att *att = _ncfIn.get_att(iatt);
    if (!strcmp(att->name(), "Volume_Number")) {
      out.add_att(att->name(), _volNum);
    } else if (!strcmp(att->name(), "Scan_Number")) {
      out.add_att(att->name(), _tiltNum);
    } else {
      Nc3Values *vals = att->values();
      switch (att->type()) {
      case nc3Byte:
	out.add_att(att->name(), att->num_vals(),
		    (char *) vals->base());
	break;
      case nc3Char:
	out.add_att(att->name(), att->num_vals(),
		    (char *) vals->base());
	break;
      case nc3Short:
	out.add_att(att->name(), att->num_vals(),
		    (short *) vals->base());
	break;
      case nc3Int:
	out.add_att(att->name(), att->num_vals(),
		    (int *) vals->base());
	break;
      case nc3Float:
	out.add_att(att->name(), att->num_vals(),
		    (float *) vals->base());
	break;
      case nc3Double:
	out.add_att(att->name(), att->num_vals(),
		    (double *) vals->base());
	break;
      default: {}
      }
      delete vals;
    }
    delete att;
  } // iatt

  // add vars and their attributes

  for (int ivar = 0; ivar < _ncfIn.num_vars(); ivar++) {
    
    Nc3Var *varIn = _ncfIn.get_var(ivar);
    int nDims = varIn->num_dims();
    const Nc3Dim *dims[nDims];
    for (int ii = 0; ii < nDims; ii++) {
      dims[ii] = out.get_dim(varIn->get_dim(ii)->id());
    }

    Nc3Var *varOut =
      out.add_var(varIn->name(), varIn->type(), nDims, dims);

    // add attributes for this variable

    for (int iatt = 0; iatt < varIn->num_atts(); iatt++) {
      Nc3Att *att = varIn->get_att(iatt);
      Nc3Values *vals = att->values();
      switch (att->type()) {
      case nc3Byte:
	varOut->add_att(att->name(), att->num_vals(),
			(char *) vals->base());
	break;
      case nc3Char:
	varOut->add_att(att->name(), att->num_vals(),
			(char *) vals->base());
	break;
      case nc3Short:
	varOut->add_att(att->name(), att->num_vals(),
			(short *) vals->base());
	break;
      case nc3Int:
	varOut->add_att(att->name(), att->num_vals(),
			(int *) vals->base());
	break;
      case nc3Float:
	varOut->add_att(att->name(), att->num_vals(),
			(float *) vals->base());
	break;
      case nc3Double:
	varOut->add_att(att->name(), att->num_vals(),
			(double *) vals->base());
	break;
      default: {}
      }
      delete vals;
      delete att;
    } // iatt
    
  } // ivar

  // add data to the variables

  for (int ivar = 0; ivar < out.num_vars(); ivar++) {

    // first get the dimensions

    Nc3Var *varIn = _ncfIn.get_var(ivar);
    Nc3Var *var = out.get_var(ivar);
    int nDims = var->num_dims();
    const Nc3Dim *dims[nDims];
    long dimSizes[nDims];
    for (int ii = 0; ii < nDims; ii++) {
      dims[ii] = var->get_dim(ii);
      dimSizes[ii] = dims[ii]->size();
    }

    // add data for this variable

    if (!strcmp(var->name(), "base_time")) {
      
      // base time
      
      int baseTimeOut = start_time;
      
      if (!var->put(&baseTimeOut)) {
	cerr << "ERROR - PpiFile::write" << endl;
	cerr << "  Cannot put base_time" << endl;
	iret = -1;
      }
      
    } else if (!strcmp(var->name(), "Fixed_Angle")) {

      // fixed angle

      if (!var->put(&fixed_angle)) {
	cerr << "ERROR - PpiFile::write" << endl;
	cerr << "  Cannot put Fixed_Angle" << endl;
	iret = -1;
      }

    } else if (nDims == 1 &&
	       var->get_dim(0)->id() == timeId) {

      // time_offset, azimuth and elevation

      if (!strcmp(var->name(), "Azimuth")) {
	long size = n_beams;
	if (!var->put(_azimuthData + _startBeam, &size)) {
	  cerr << "ERROR - PpiFile::write" << endl;
	  cerr << "  Cannot put Azimuth" << endl;
	  iret = -1;
	}
      } else if (!strcmp(var->name(), "Elevation")) {
	long size = n_beams;
	if (!var->put(_elevationData + _startBeam, &size)) {
	  cerr << "ERROR - PpiFile::write" << endl;
	  cerr << "  Cannot put Elevation" << endl;
	  iret = -1;
	}
      } else if (!strcmp(var->name(), "time_offset")) {
	long size = n_beams;
	double offset[n_beams];
	double startOffset = _timeOffsetData[_startBeam];
	for (int i = 0; i < n_beams; i++) {
	  offset[i] = _timeOffsetData[_startBeam + i] - startOffset;
	  if (offset[i] == 0.0) {
	    offset[i] = 0.0000001;
	  }
	}
	if (!var->put(offset, &size)) {
	  cerr << "ERROR - PpiFile::write" << endl;
	  cerr << "  Cannot put time_offset" << endl;
	  iret = -1;
	}
      }

    } else if (nDims == 2 &&
	       var->get_dim(0)->id() == timeId &&
	       var->get_dim(1)->id() == maxCellsId) {

      // field data - shorts

      Nc3Values *vals = varIn->values();
      long sizes[2];
      sizes[0] = n_beams;
      sizes[1] = _maxCells;
      int offset = _startBeam * _maxCells;
      if (!var->put((short *) vals->base() + offset, sizes)) {
	cerr << "ERROR - PpiFile::write" << endl;
	cerr << "  Cannot write var:" << var->name() << endl;
	iret = -1;
      }
      delete vals;
      
      
    } else {
      
      // the rest

      Nc3Values *vals = varIn->values();
      bool success = false;
      switch (varIn->type()) {
      case nc3Byte:
	success = var->put((char *) vals->base(), dimSizes);
	break;
      case nc3Char:
	success = var->put((char *) vals->base(), dimSizes);
	break;
      case nc3Short:
	success = var->put((short *) vals->base(), dimSizes);
	break;
      case nc3Int:
	success = var->put((int *) vals->base(), dimSizes);
	break;
      case nc3Float:
	success = var->put((float *) vals->base(), dimSizes);
	break;
      case nc3Double:
	success = var->put((double *) vals->base(), dimSizes);
	break;
      default: {}
      }
      if (!success) {
	cerr << "ERROR - PpiFile::write" << endl;
	cerr << "  put failed for var: " << var->name() << endl;
      }
      delete vals;
    }

  } // ivar

  return iret;

}
