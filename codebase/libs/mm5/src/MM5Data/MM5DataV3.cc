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
//////////////////////////////////////////////////////////
// MM5DataV3.cc
//
// Read in V3 MM5 file.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
//////////////////////////////////////////////////////////


#include <mm5/MM5DataV3.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <physics/thermo.h>
#include <sys/stat.h>
#include <toolsa/toolsa_macros.h>
#include <iostream>
using namespace std;

//////////////
// Constructor

MM5DataV3::MM5DataV3 (const string &prog_name,
		      const string &path,
		      bool debug /* = false*/,
		      const heartbeat_t heartbeat_func /* = NULL */,
		      bool dbzConstantIntercepts /* = true */) :
  MM5Data(prog_name, path, debug, heartbeat_func, dbzConstantIntercepts)
{

  _version = 3;

  _mif = (si32 **) umalloc2(MIF_N1, MIF_N2, sizeof(si32));
  _mrf = (fl32 **) umalloc2(MRF_N1, MRF_N2, sizeof(fl32));
  _mifc = (label_t **) umalloc2(MIF_N1, MIF_N2, sizeof(label_t));
  _mrfc = (label_t **) umalloc2(MRF_N1, MRF_N2, sizeof(label_t));
  
}

/////////////
// Destructor

MM5DataV3::~MM5DataV3()

{

  if (_mif != NULL) {
    ufree2((void **) _mif);
    _mif = NULL;
  }
  if (_mrf != NULL) {
    ufree2((void **) _mrf);
    _mrf = NULL;
  }
  if (_mifc != NULL) {
    ufree2((void **) _mifc);
    _mifc = NULL;
  }
  if (_mrfc != NULL) {
    ufree2((void **) _mrfc);
    _mrfc = NULL;
  }

}

/////////
// read()
//
// returns 0 on success, -1 on failure

int MM5DataV3::read()

{

  if (_debug) {
    fprintf(stderr, "Reading file %s\n", _path.c_str());
  }

  if (_readHeaders()) {
    return (-1);
  }
  if (_readDataset()) {
    return (-1);
  }

  _setTimes();

  return (0);

}

///////////////////
// findHdrRecords()
//
// Find the fortran record markers around the header
//

void MM5DataV3::findHdrRecords()

{

  fprintf(stderr, "\n");
  fprintf(stderr, "%s: looking for FORTRAN headers in file:\n   %s\n",
	  _progName.c_str(), _path.c_str());
  fprintf(stderr, "\n");
  fprintf(stderr, "Addresses and lengths are 4-byte values\n\n");

  fseek(_in, 0, SEEK_SET);
  
  while (!feof(_in)) {
    long start_reclen = _readFortRecLen();
    if (start_reclen < 0) {
      break;
    }
    cerr << "Start F rec len: " << start_reclen << endl;
    if (start_reclen == MM5_HEADER_LEN) {
      cerr << "Found main header, offset: "
	   << ftell(_in) - sizeof(si32) << endl;
    } else if (start_reclen == sizeof(sub_header_t)) {
      cerr << "Found field header, offset: "
	   << ftell(_in) - sizeof(si32) << endl;
    }
    fseek(_in, start_reclen, SEEK_CUR);
    long end_reclen = _readFortRecLen();
    if (end_reclen < 0) {
      break;
    }
    cerr << "End F rec len: " << end_reclen << endl;
    cerr << "=============================" << endl;
    if (start_reclen != end_reclen) {
      cerr << "ERROR - start and end reclen not the same" << endl;
    }
  } // while
  
}

//////////////////
// printHeaders()
//
// Print out header labels etc.

void MM5DataV3::printHeaders(FILE *out) const

{

  _printHeaders(out);

  // MIFC

  fprintf(out, "\n================== MIF ===================\n\n");

  for (int jj = 0; jj < MIF_N1; jj++) {
    for (int ii = 0; ii < MIF_N2; ii++) {
      if (_mif[jj][ii] != -999) {
	fprintf(out, "[%2d][%4d] = %8d: %s\n",
		jj+1, ii+1, _mif[jj][ii], _mifc[jj][ii]);
      }
    }
  }

  // MRFC

  fprintf(out, "\n================== MRF ===================\n\n");

  for (int jj = 0; jj < MRF_N1; jj++) {
    for (int ii = 0; ii < MRF_N2; ii++) {
      if (_mrf[jj][ii] != -999) {
	fprintf(out, "[%2d][%4d] = %8g: %s\n",
		jj+1, ii+1, _mrf[jj][ii], _mrfc[jj][ii]);
      }
    }
  }

}

// get the FDDA start and end times - minutes

double MM5DataV3::getFddaStartTime() const

{
  if (file_type != 5) {
    return _mrf[15][0];
  } else {
    return 0;
  }
}

double MM5DataV3::getFddaEndTime() const

{
  if (file_type != 5) {
    return _mrf[15][1];
  } else {
    return 0;
  }
}

///////////////////////////////////////////
// get MIF or MRF values, given the label

int MM5DataV3::getMifVal(const char *label) const

{

  for (int jj = 0; jj < MIF_N1; jj++) {
    for (int ii = 0; ii < MIF_N2; ii++) {
      if (_mif[jj][ii] == -999) {
	continue;
      }
      size_t nlabel = strlen(label);
      if (nlabel > strlen(_mifc[jj][ii])) {
	continue;
      }
      if (strncmp(_mifc[jj][ii], label, nlabel) == 0) {
	return _mif[jj][ii];
      }
    }
  }

  return -999;

}

double MM5DataV3::getMrfVal(const char *label) const

{

  for (int jj = 0; jj < MRF_N1; jj++) {
    for (int ii = 0; ii < MRF_N2; ii++) {
      if (_mrf[jj][ii] == -999) {
	continue;
      }
      size_t nlabel = strlen(label);
      if (nlabel > strlen(_mrfc[jj][ii])) {
	continue;
      }
      if (strncmp(_mrfc[jj][ii], label, nlabel) == 0) {
	return _mrf[jj][ii];
      }
    }
  }

  return -999.0;

}

/////////////////
// _readHeaders()
//
// Reads the first header to set class members
//
// returns 0 on success, -1 on failure

int MM5DataV3::_readHeaders()

{

  // read flag

  long startOffset = ftell(_in);
  _readFortRecLen();
  si32 flag;
  if (ufread(&flag, sizeof(si32), 1, _in) != 1) {
    fprintf(stderr, "ERROR - %s:MM5DataV3::_readHeaders\n", _progName.c_str());
    fprintf(stderr, "Cannot read flag\n");
    perror(_path.c_str());
    return (-1);
  }
  BE_from_array_32(&flag, 1 * sizeof(si32));
  if (flag != 0) {
    // no main header, go back to starting offset so that the data set
    // reader starts in the correct place
    fseek(_in, startOffset, SEEK_SET);
    return 0;
  }
  _readFortRecLen();

  // save the starting offset

  _headersOffset = startOffset;

  // fort rec len at start of MIF and MRF arrays

  _readFortRecLen();

  // MIF array
  
  if (ufread(*_mif, sizeof(si32), MIF_N, _in) != MIF_N) {
    fprintf(stderr, "ERROR - %s:MM5DataV3::_readHeaders\n", _progName.c_str());
    fprintf(stderr, "Cannot read mif array\n");
    perror(_path.c_str());
    return (-1);
  }
  BE_from_array_32(*_mif, MIF_N * sizeof(si32));

  // MRF array

  if (ufread(*_mrf, sizeof(fl32), MRF_N, _in) != MRF_N) {
    fprintf(stderr, "ERROR - %s:MM5DataV3::_readHeaders\n", _progName.c_str());
    fprintf(stderr, "Cannot read mrf array\n");
    perror(_path.c_str());
    return (-1);
  }
  BE_from_array_32(*_mrf, MRF_N * sizeof(fl32));

  // MIFC array
  
  if (ufread(*_mifc, sizeof(label_t), MIF_N, _in) != MIF_N) {
    fprintf(stderr, "ERROR - %s:MM5DataV3::_readHeaders\n", _progName.c_str());
    fprintf(stderr, "Cannot read mifc array\n");
    perror(_path.c_str());
    return (-1);
  }

  // make sure strings are clean and null terminated
  for (int jj = 0; jj < MIF_N1; jj++) {
    for (int ii = 0; ii < MIF_N2; ii++) {
      char *mifc = _mifc[jj][ii];
      mifc[79] = '\0';
      for (int kk = 78; kk >= 0; kk--) {
 	if (mifc[kk] != ' ') {
 	  break;
 	}
 	mifc[kk] = '\0';
      }
    }
  }

  // MRFC array
  
  if (ufread(*_mrfc, sizeof(label_t), MRF_N, _in) != MRF_N) {
    fprintf(stderr, "ERROR - %s:MM5DataV3::_readHeaders\n", _progName.c_str());
    fprintf(stderr, "Cannot read mrfc array\n");
    perror(_path.c_str());
    return (-1);
  }

  // make sure strings are clean and null terminated
  for (int jj = 0; jj < MRF_N1; jj++) {
    for (int ii = 0; ii < MRF_N2; ii++) {
      char *mrfc = _mrfc[jj][ii];
      mrfc[79] = '\0';
       for (int kk = 78; kk >= 0; kk--) {
 	if (mrfc[kk] != ' ') {
 	  break;
 	}
 	mrfc[kk] = '\0';
      }
    }
  }

  // fort rec len at end of MIF and MRF arrays

  _readFortRecLen();

  file_type = _mif[0][0];
  if (!(file_type == 11 || file_type ==1 || file_type == 5)) {
    fprintf(stderr, "ERROR - %s:MM5DataV3::_readHeaders\n", _progName.c_str());
    fprintf(stderr, "This is not a version 3 MM5 file. file_type: %d\n",file_type);
    return (-1);
  }
  
  // pTop etc

  pTop = _mrf[1][1] / 100.0;
  pos = _mrf[4][1] / 100.0;
  tso = _mrf[4][2];
  tlp = _mrf[4][3];

  // set grid info, sizes
  
  nSigma = _mif[4][11];
  nyDot = _mif[0][15];
  nxDot = _mif[0][16];
  nyDotCoarse = _mif[0][4];
  nxDotCoarse = _mif[0][5];
  nPtsDotPlane = nxDot * nyDot;
  nLon = nxDot - 1;
  nLat = nyDot - 1;

  // projection info

  if (_mif[0][6] == 1) {
    proj_type = LAMBERT_CONF;
  } else if (_mif[0][6] == 2) {
    proj_type = STEREOGRAPHIC;
  } else if (_mif[0][6] == 3) {
    proj_type = MERCATOR;
  } else {
    proj_type = UNKNOWN;
  }
  center_lat = _mrf[0][1];
  center_lon = _mrf[0][2];
  cone_factor =  _mrf[0][3];
  true_lat1 = _mrf[0][4];
  true_lat2 = _mrf[0][5];
  grid_distance = _mrf[0][8] / 1000.0;
  domain_scale_coarse = _mif[0][19];
  x1_in_coarse_domain = _mrf[0][10];
  y1_in_coarse_domain = _mrf[0][9];
  x1_in_mother_domain = _mif[0][18];
  y1_in_mother_domain = _mif[0][17];

  grid_distance_coarse = grid_distance * domain_scale_coarse;
  minx_dot_coarse = -1.0 * ((nxDotCoarse - 1.0) / 2.0) * grid_distance_coarse;
  miny_dot_coarse = -1.0 * ((nyDotCoarse - 1.0) / 2.0) * grid_distance_coarse;
  minx_dot =
    minx_dot_coarse + (x1_in_coarse_domain - 1.0) * grid_distance_coarse;
  miny_dot =
    miny_dot_coarse + (y1_in_coarse_domain -1.0) * grid_distance_coarse;
  minx_cross = minx_dot + grid_distance / 2.0;
  miny_cross = miny_dot + grid_distance / 2.0;

  // Allocate arrays for reading data
  // 3d field array has extra sigma level for reading w field

  if (_field3d != NULL) {
    ufree3((void ***) _field3d);
  }
  if(nSigma < 0 ) nSigma = 0; // In some files's it's -999
  _field3d = (fl32 ***) umalloc3(nSigma + 1, nxDot, nyDot, sizeof(fl32));
  
  if (_field2d != NULL) {
    ufree2((void **) _field2d);
  }
  _field2d = (fl32 **) umalloc2(nxDot, nyDot, sizeof(fl32));

  if (_halfSigma != NULL) {
    ufree(_halfSigma);
  }
  _halfSigma = (fl32 *) umalloc(nSigma * sizeof(fl32));

  // save the header len
  
  long endOffset = ftell(_in);
  _headersLen = endOffset - startOffset;

  return (0);

}

///////////////
// _setTimes()
//
// Set the times of the data set
//
// returns 0 on success, -1 on failure

void MM5DataV3::_setTimes()

{

  int year, month, day, hour, min, sec;

  if (file_type == 5) {
    year  = _mif[4][4];
    month = _mif[4][5];
    day   = _mif[4][6];
    hour  = _mif[4][7];
    min   = _mif[4][8];
    sec   = _mif[4][9];

  } else {

    year  = _mif[10][4];
    month = _mif[10][5];
    day   = _mif[10][6];
    hour  = _mif[10][7];
    min   = _mif[10][8];
    sec   = _mif[10][9];
  }

  DateTime mtime(year, month, day, hour, min, sec);
  modelTime = mtime.utime();

  // for MMINP file, the time in field header might not be set,
  // so forecastLeadTime is not correct. Set it here manually.
  if (file_type == 5) {
    forecastLeadTime = outputTime - modelTime;
  }

  if (forecastLeadTime != (outputTime - modelTime)) {
    fprintf(stderr, "WARNING: MM5DataV3::_setTimes()\n");
    fprintf(stderr, "  Model time, lead time and output time do not match\n");
    fprintf(stderr, "  Model  time: %s\n", utimstr(modelTime));
    fprintf(stderr, "  Output time: %s\n", utimstr(outputTime));
    fprintf(stderr, "  Forecast lead time: %d secs\n", forecastLeadTime);
  }

  if (file_type == 5) {
    forecastDelta = (int) (_mrf[4][0] + 0.5);
  } else {
    forecastDelta = (int) (_mrf[10][0] + 0.5);
  }
  if (_debug) {
    fprintf(stderr, "Forecast delta: %d\n", forecastDelta);
  }
  
}

////////////////////
// read a data set()

int MM5DataV3::_readDataset()

{

  // save the data set offset
  
  _dataSetOffset = ftell(_in);

  // initialize field number flags

  uFieldNum = -1;
  vFieldNum = -1;
  tFieldNum = -1;
  qFieldNum = -1;
  clwFieldNum = -1;
  rnwFieldNum = -1;
  iceFieldNum = -1;
  snowFieldNum = -1;
  graupelFieldNum = -1;
  nciFieldNum = -1;
  radTendFieldNum = -1;
  wFieldNum = -1;
  ppFieldNum = -1;
  pstarFieldNum = -1;
  groundTFieldNum = -1;
  rainConFieldNum = -1;
  rainNonFieldNum = -1;
  terrainFieldNum = -1;
  coriolisFieldNum = -1;
  resTempFieldNum = -1;
  latFieldNum = -1;
  lonFieldNum = -1;
  landUseFieldNum = -1;
  snowcovrFieldNum = -1;
  tseasfcFieldNum = -1;
  pblHgtFieldNum = -1;
  regimeFieldNum = -1;
  shfluxFieldNum = -1;
  lhfluxFieldNum = -1;
  ustFieldNum = -1;
  swdownFieldNum = -1;
  lwdownFieldNum = -1;
  soilT1FieldNum = -1;
  soilT2FieldNum = -1;
  soilT3FieldNum = -1;
  soilT4FieldNum = -1;
  soilT5FieldNum = -1;
  soilT6FieldNum = -1;
  soilM1FieldNum = -1;
  soilM2FieldNum = -1;
  soilM3FieldNum = -1;
  soilM4FieldNum = -1;
  sfcrnoffFieldNum = -1;
  t2FieldNum = -1;
  q2FieldNum = -1;
  u10FieldNum = -1;
  v10FieldNum = -1;
  mapfXFieldNum = -1;
  mapfDotFieldNum = -1;
  weasdFieldNum = -1;
  snowhFieldNum = -1;
  hc_rainFieldNum = -1;
  hn_rainFieldNum = -1;

  swfracFieldNum = -1;
  sunaltFieldNum = -1;
  sunazmFieldNum = -1;
  moonaltFieldNum = -1;
  moonazmFieldNum = -1;
  sunillFieldNum = -1;
  moonillFieldNum = -1;
  totalillFieldNum = -1;

  clwiFieldNum = -1;
  rnwiFieldNum = -1;
  iceiFieldNum = -1;
  snowiFieldNum = -1;
  pwvFieldNum = -1;

  sunBtwFieldNum = -1;
  sunEtwFieldNum = -1;
  sunAbtwFieldNum = -1;
  sunAetwFieldNum = -1;
  sunRiseFieldNum = -1;
  sunSetFieldNum = -1;
  sunArisFieldNum = -1;
  sunAsetFieldNum = -1;
  moonRisFieldNum = -1;
  moonSetFieldNum = -1;
  moonAriFieldNum = -1;
  moonAseFieldNum = -1;

  // loop through the fields
  
  int fieldNum = 0;

  while (true) {

    // read flag
    
    _readFortRecLen();
    si32 flag;
    if (ufread(&flag, sizeof(si32), 1, _in) != 1) {
      fprintf(stderr, "ERROR - %s:MM5DataV3::_readDataset\n",
	      _progName.c_str());
      fprintf(stderr, "Cannot read flag\n");
      perror(_path.c_str());
      return (-1);
    }
    _readFortRecLen();
    BE_from_array_32(&flag, 1 * sizeof(si32));
    if (flag == 2) {
      break; // end of data set
    }
    if (flag != 1) {
      fprintf(stderr, "ERROR - %s:MM5DataV3::_readDataset\n",
	      _progName.c_str());
      fprintf(stderr, "  Sub header flag value: %d\n", flag);
      fprintf(stderr, "  Sub header flag must be 1\n");
      return (-1);
    }

    // read sub header
    
    _readFortRecLen();
    sub_header_t subHdr;
    if (ufread(&subHdr, sizeof(sub_header_t), 1, _in) != 1) {
      fprintf(stderr, "ERROR - %s:MM5DataV3::_readDataset\n",
	      _progName.c_str());
      fprintf(stderr, "Cannot read sub header\n");
      perror(_path.c_str());
      return (-1);
    }
    _readFortRecLen();

    // swap header and null-terminate strings

    BE_from_array_32(&subHdr, 10 * sizeof(si32));
    subHdr.staggering[3] = '\0';
    subHdr.ordering[3] = '\0';
    subHdr.current_date[23] = '\0';
    subHdr.name[8] = '\0';
    subHdr.units[24] = '\0';
    subHdr.descr[45] = '\0';

    if (_debug) {
      cerr << "  ============================================" << endl;
      cerr << "  name: " << subHdr.name << endl;
      cerr << "  units: " << subHdr.units << endl;
      cerr << "  descr: " << subHdr.descr << endl;
      cerr << "  ndim: " << subHdr.ndim << endl;
      cerr << "  time: " << subHdr.time << endl;
      cerr << "  staggering: " << subHdr.staggering << endl;
      cerr << "  ordering: " << subHdr.ordering << endl;
      cerr << "  current_date: " << subHdr.current_date << endl;
      cerr << "  start_index[0]: " << subHdr.start_index[0] << endl;
      cerr << "  start_index[1]: " << subHdr.start_index[1] << endl;
      cerr << "  start_index[2]: " << subHdr.start_index[2] << endl;
      cerr << "  start_index[3]: " << subHdr.start_index[3] << endl;
      cerr << "  end_index[0]: " << subHdr.end_index[0] << endl;
      cerr << "  end_index[1]: " << subHdr.end_index[1] << endl;
      cerr << "  end_index[2]: " << subHdr.end_index[2] << endl;
      cerr << "  end_index[3]: " << subHdr.end_index[3] << endl;
    }

    fieldNames.push_back(subHdr.name);
    fieldUnits.push_back(subHdr.units);

    // scan in the model output time

    int year, month, day, hour, min, sec;
    if (sscanf(subHdr.current_date, "%4d-%2d-%2d_%2d:%2d:%2d",
	       &year, &month, &day, &hour, &min, &sec) != 6) {
      fprintf(stderr, "ERROR - %s:MM5DataV3::_readDataset\n",
	      _progName.c_str());
      fprintf(stderr, "  File: %s\n", _path.c_str());
      fprintf(stderr, "  Cannot parse output time str: %s\n",
	      subHdr.current_date);
      return -1;
    }
    DateTime dataTime(year, month, day, hour, min, sec);
    outputTime = dataTime.utime();
    forecastLeadTime = (int) (subHdr.time * 60 + 0.5);
    
    // compute field size

    int nx = 0, ny = 0, nz = 0;
    int nPoints;

    if (subHdr.ndim == 1) {
      nx = subHdr.end_index[0] - subHdr.start_index[0] + 1;
      nPoints = nx;
    } else if (subHdr.ndim == 2) {
      ny = subHdr.end_index[0] - subHdr.start_index[0] + 1;
      nx = subHdr.end_index[1] - subHdr.start_index[1] + 1;
      nPoints = nx * ny;
    } else if (subHdr.ndim == 3) {
      ny = subHdr.end_index[0] - subHdr.start_index[0] + 1;
      nx = subHdr.end_index[1] - subHdr.start_index[1] + 1;
      nz = subHdr.end_index[2] - subHdr.start_index[2] + 1;
      nPoints = nx * ny * nz;
    } else {
      cerr << "ERROR - ndim: " << subHdr.ndim << endl;
      return -1;
    }

    int nBytes = nPoints * sizeof(fl32);
    
    if (_debug) {
      cerr << "  nPoints: " << nPoints << endl;
      cerr << "  nBytes: " << nBytes << endl;
    }

    // read in field

    fl32 *fieldData = NULL;

    if (subHdr.ndim == 3) {
      fieldData = **_field3d;
    } else if (subHdr.ndim == 2) {
      fieldData = *_field2d;
    } else if (subHdr.ndim == 1) {
      fieldData = (fl32*) umalloc (nBytes);
    }

    _readFortRecLen();
    if (ufread(fieldData, sizeof(fl32), nPoints, _in) != nPoints) {
      fprintf(stderr, "ERROR - %s:MM5DataV3::_readDataset\n", _progName.c_str());
      fprintf(stderr, "Cannot read field data\n");
      perror(_path.c_str());
      return (-1);
    }
    _readFortRecLen();
    BE_to_array_32(fieldData, nBytes);

    // is this a dot or cross field

    bool isDot;
    if (subHdr.staggering[0] == 'D') {
      isDot = true;
    } else {
      isDot = false;
    }

    // load up field
    // You have to pad the name out to 8 characters long.

    if (!strcmp(subHdr.name, "U       ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      uFieldNum = fieldNum;
      _load3dField(&uu, isDot);
      _load3dField2Dot(&uu_dot);

    } else if (!strcmp(subHdr.name, "V       ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      vFieldNum = fieldNum;
      _load3dField(&vv, isDot);
      _load3dField2Dot(&vv_dot);

    } else if (!strcmp(subHdr.name, "T       ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      tFieldNum = fieldNum;
      _load3dField(&tk, isDot);

    } else if (!strcmp(subHdr.name, "Q       ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      qFieldNum = fieldNum;
      _load3dField(&qq, isDot);

    } else if (!strcmp(subHdr.name, "CLW     ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      clwFieldNum = fieldNum;
      _load3dField(&clw, isDot);

    } else if (!strcmp(subHdr.name, "RNW     ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      rnwFieldNum = fieldNum;
      _load3dField(&rnw, isDot);

    } else if (!strcmp(subHdr.name, "ICE     ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      iceFieldNum = fieldNum;
      _load3dField(&ice, isDot);

    } else if (!strcmp(subHdr.name, "SNOW    ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      snowFieldNum = fieldNum;
      _load3dField(&snow, isDot);

    } else if (!strcmp(subHdr.name, "GRAUPEL ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      graupelFieldNum = fieldNum;
      _load3dField(&graupel, isDot);

    } else if (!strcmp(subHdr.name, "NCI     ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      nciFieldNum = fieldNum;
      _load3dField(&nci, isDot);

    } else if (!strcmp(subHdr.name, "RAD TEND")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      radTendFieldNum = fieldNum;
      _load3dField(&rad_tend, isDot);

    } else if (!strcmp(subHdr.name, "W       ")) {

      // W field is on full sigma levels, has 1 extra level
      if (_check3dSize(nz - 1, ny, nx, subHdr.name)) {
	return -1;
      }
      wFieldNum = fieldNum;
      _loadWField();

    } else if (!strcmp(subHdr.name, "PP      ")) {

      if (_check3dSize(nz, ny, nx, subHdr.name)) {
	return -1;
      }
      ppFieldNum = fieldNum;
      _load3dField(&pp, isDot);

    } else if (!strcmp(subHdr.name, "PSTARCRS")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      pstarFieldNum = fieldNum;
      _load2dField(&pstar, isDot);

    } else if (!strcmp(subHdr.name, "GROUND T")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      groundTFieldNum = fieldNum;
      _load2dField(&ground_t, isDot);
      
    } else if (!strcmp(subHdr.name, "RAIN CON")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      rainConFieldNum = fieldNum;
      _load2dField(&rain_con, isDot);

    } else if (!strcmp(subHdr.name, "RAIN NON")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      rainNonFieldNum = fieldNum;
      _load2dField(&rain_non, isDot);

    } else if (!strcmp(subHdr.name, "TERRAIN ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      terrainFieldNum = fieldNum;
      _load2dField(&terrain, isDot);

    } else if (!strcmp(subHdr.name, "MAPFACCR")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      mapfXFieldNum = fieldNum;
      _load2dField(&mapf_x, false);

    } else if (!strcmp(subHdr.name, "MAPFACDT")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      mapfDotFieldNum = fieldNum;
      _load2dField2Dot(&mapf_dot);

    } else if (!strcmp(subHdr.name, "CORIOLIS")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      coriolisFieldNum = fieldNum;
      _load2dField(&coriolis, isDot);
      _load2dField2Dot(&coriolis_dot);

    } else if (!strcmp(subHdr.name, "RES TEMP")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      resTempFieldNum = fieldNum;
      _load2dField(&res_temp, isDot);

    } else if (!strcmp(subHdr.name, "LATITCRS")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      latFieldNum = fieldNum;
      _load2dField(&lat, isDot);

    } else if (!strcmp(subHdr.name, "LONGICRS")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      lonFieldNum = fieldNum;
      _load2dField(&lon, isDot);

    } else if (!strcmp(subHdr.name, "LAND USE")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      landUseFieldNum = fieldNum;
      _load2dField(&land_use, isDot);

    } else if (!strcmp(subHdr.name, "SNOWCOVR")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      snowcovrFieldNum = fieldNum;
      _load2dField(&snowcovr, isDot);

    } else if (!strcmp(subHdr.name, "TSEASFC ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      tseasfcFieldNum = fieldNum;
      _load2dField(&tseasfc, isDot);

    } else if (!strcmp(subHdr.name, "PBL HGT ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      pblHgtFieldNum = fieldNum;
      _load2dField(&pbl_hgt, isDot);

    } else if (!strcmp(subHdr.name, "REGIME  ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      regimeFieldNum = fieldNum;
      _load2dField(&regime, isDot);
      
    } else if (!strcmp(subHdr.name, "SHFLUX  ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      shfluxFieldNum = fieldNum;
      _load2dField(&shflux, isDot);

    } else if (!strcmp(subHdr.name, "LHFLUX  ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      lhfluxFieldNum = fieldNum;
      _load2dField(&lhflux, isDot);

    } else if (!strcmp(subHdr.name, "UST     ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      ustFieldNum = fieldNum;
      _load2dField(&ust, isDot);

    } else if (!strcmp(subHdr.name, "SWDOWN  ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      swdownFieldNum = fieldNum;
      _load2dField(&swdown, isDot);

    } else if (!strcmp(subHdr.name, "LWDOWN  ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      lwdownFieldNum = fieldNum;
      _load2dField(&lwdown, isDot);

    } else if (
      !strcmp(subHdr.name, "SOIL T 1") || // for MMOUTPUT
      !strcmp(subHdr.name, "SOILT010")    // for MMINPUT
    ) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      soilT1FieldNum = fieldNum;
      _load2dField(&soil_t_1, isDot);

    } else if (
      !strcmp(subHdr.name, "SOIL T 2") || // for MMOUTPUT
      !strcmp(subHdr.name, "SOILT200")    // for MMINPUT
    ) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -2;
      }
      soilT2FieldNum = fieldNum;
      _load2dField(&soil_t_2, isDot);

    } else if (!strcmp(subHdr.name, "SOIL T 3")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -3;
      }
      soilT3FieldNum = fieldNum;
      _load2dField(&soil_t_3, isDot);

    } else if (!strcmp(subHdr.name, "SOIL T 4")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -4;
      }
      soilT4FieldNum = fieldNum;
      _load2dField(&soil_t_4, isDot);

    } else if (!strcmp(subHdr.name, "SOIL T 5")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -5;
      }
      soilT5FieldNum = fieldNum;
      _load2dField(&soil_t_5, isDot);

    } else if (!strcmp(subHdr.name, "SOIL T 6")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -6;
      }
      soilT6FieldNum = fieldNum;
      _load2dField(&soil_t_6, isDot);

    } else if (
      !strcmp(subHdr.name, "SOIL M 1") || // for MMOUTPUT
      !strcmp(subHdr.name, "SOILM010")    // for MMINPUT
    ) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      soilM1FieldNum = fieldNum;
      _load2dField(&soil_m_1, isDot);

    } else if (
      !strcmp(subHdr.name, "SOIL M 2") || // for MMOUTPUT
      !strcmp(subHdr.name, "SOILM200")    // for MMINPUT
    ) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      soilM2FieldNum = fieldNum;
      _load2dField(&soil_m_2, isDot);

    } else if (!strcmp(subHdr.name, "SOIL M 3")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      soilM3FieldNum = fieldNum;
      _load2dField(&soil_m_3, isDot);

    } else if (!strcmp(subHdr.name, "SOIL M 4")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      soilM4FieldNum = fieldNum;
      _load2dField(&soil_m_4, isDot);

    } else if (!strcmp(subHdr.name, "SFCRNOFF")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sfcrnoffFieldNum = fieldNum;
      _load2dField(&sfcrnoff, isDot);

    } else if (
      !strcmp(subHdr.name, "T2      ") ||  // for MMOUTPUT
      !strcmp(subHdr.name, "TSFC    ")     // for MMINPUT
    ) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -2;
      }
      t2FieldNum = fieldNum;
      _load2dField(&t2, isDot);

    } else if (!strcmp(subHdr.name, "Q2      ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -2;
      }
      q2FieldNum = fieldNum;
      _load2dField(&q2, isDot);

    } else if (
      !strcmp(subHdr.name, "U10     ") || // for MMOUTPUT
      !strcmp(subHdr.name, "USFC    ")    // for MMINPUT
    ) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -10;
      }
      u10FieldNum = fieldNum;
      _load2dField(&u10, isDot);

    } else if (
      !strcmp(subHdr.name, "V10     ") || // for MMOUTPUT
      !strcmp(subHdr.name, "VSFC    ")    // for MMINPUT
    ) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -10;
      }
      v10FieldNum = fieldNum;
      _load2dField(&v10, isDot);

    } else if (!strcmp(subHdr.name, "SIGMAH  ")) {

      if (_loadSigmaLevels(nx, fieldData)) {
	return -1;
      }

    } else if (!strcmp(subHdr.name, "WEASD   ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      weasdFieldNum = fieldNum;
      _load2dField(&weasd, isDot);

    } else if (!strcmp(subHdr.name, "SNOWH   ")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      snowhFieldNum = fieldNum;
      _load2dField(&snowh, isDot);

    } else if (!strcmp(subHdr.name, "HRAINCON")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      hc_rainFieldNum = fieldNum;
      _load2dField(&hc_rain, isDot);

    } else if (!strcmp(subHdr.name, "HRAINNON")) {

      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      hn_rainFieldNum = fieldNum;
      _load2dField(&hn_rain, isDot);

    } else if (!strcmp(subHdr.name, "SWFRAC  ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      swfracFieldNum = fieldNum;
      _load2dField(&swfrac, isDot);

    } else if (!strcmp(subHdr.name, "SUNALT  ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunaltFieldNum = fieldNum;
      _load2dField(&sunalt, isDot);

    } else if (!strcmp(subHdr.name, "SUNAZM  ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunazmFieldNum = fieldNum;
      _load2dField(&sunazm, isDot);

    } else if (!strcmp(subHdr.name, "MOONALT ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      moonaltFieldNum = fieldNum;
      _load2dField(&moonalt, isDot);

    } else if (!strcmp(subHdr.name, "MOONAZM ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      moonazmFieldNum = fieldNum;
      _load2dField(&moonazm, isDot);

    } else if (!strcmp(subHdr.name, "SUNILL  ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunillFieldNum = fieldNum;
      _load2dField(&sunill, isDot);

    } else if (!strcmp(subHdr.name, "MOONILL ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      moonillFieldNum = fieldNum;
      _load2dField(&moonill, isDot);

    } else if (!strcmp(subHdr.name, "TOTALILL")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      totalillFieldNum = fieldNum;
      _load2dField(&totalill, isDot);

    } else if (!strcmp(subHdr.name, "CLWI    ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      clwiFieldNum = fieldNum;
      _load2dField(&clwi, isDot);

    } else if (!strcmp(subHdr.name, "RNWI    ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      rnwiFieldNum = fieldNum;
      _load2dField(&rnwi, isDot);

    } else if (!strcmp(subHdr.name, "ICEI    ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      iceiFieldNum = fieldNum;
      _load2dField(&icei, isDot);

    } else if (!strcmp(subHdr.name, "SNOWI   ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      snowiFieldNum = fieldNum;
      _load2dField(&snowi, isDot);

    } else if (!strcmp(subHdr.name, "PWV     ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      pwvFieldNum = fieldNum;
      _load2dField(&pwv, isDot);

    } else if (!strcmp(subHdr.name, "SUN-BTW ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunBtwFieldNum = fieldNum;
      _load2dField(&sun_btw, isDot);

    } else if (!strcmp(subHdr.name, "SUN-ETW ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunEtwFieldNum = fieldNum;
      _load2dField(&sun_etw, isDot);

    } else if (!strcmp(subHdr.name, "SUN-ABTW")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunAbtwFieldNum = fieldNum;
      _load2dField(&sun_abtw, isDot);

    } else if (!strcmp(subHdr.name, "SUN-AETW")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunAetwFieldNum = fieldNum;
      _load2dField(&sun_aetw, isDot);

    } else if (!strcmp(subHdr.name, "SUN-RISE")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunRiseFieldNum = fieldNum;
      _load2dField(&sun_rise, isDot);

    } else if (!strcmp(subHdr.name, "SUN-SET ")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunSetFieldNum = fieldNum;
      _load2dField(&sun_set, fieldData);

    } else if (!strcmp(subHdr.name, "SUN-ARIS")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunArisFieldNum = fieldNum;
      _load2dField(&sun_aris, isDot);

    } else if (!strcmp(subHdr.name, "SUN-ASET")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      sunAsetFieldNum = fieldNum;
      _load2dField(&sun_aset, isDot);

    } else if (!strcmp(subHdr.name, "MOON-RIS")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      moonRisFieldNum = fieldNum;
      _load2dField(&moon_ris, isDot);

    } else if (!strcmp(subHdr.name, "MOON-SET")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      moonSetFieldNum = fieldNum;
      _load2dField(&moon_set, isDot);

    } else if (!strcmp(subHdr.name, "MOON-ARI")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      moonAriFieldNum = fieldNum;
      _load2dField(&moon_ari, isDot);

    } else if (!strcmp(subHdr.name, "MOON-ASE")) {
      if (_check2dSize(ny, nx, subHdr.name)) {
	return -1;
      }
      moonAseFieldNum = fieldNum;
      _load2dField(&moon_ase, isDot);

    } else if (!strcmp(subHdr.name, "RHSFC   ")) {

      /* This is only for MMINPUT.
       * RH2 is a raw field in MMINPUT file, while it is a derived
       * field in MMOUTPUT file.
       */
      if (_check2dSize(ny, nx, subHdr.name)) {
        return -1;
      }
      _load2dField(&rh2, isDot);

    }

    if (_debug) {
      cerr << "Field name: " << subHdr.name << endl;
      if (subHdr.ndim == 1) {
	for (int i = 0; i < nx; i++) {
	  cerr << "    " << i << ": " << fieldData[i] << endl;
	}
      }
    }

    if (subHdr.ndim == 1) {
      ufree(fieldData);
    }
    
    fieldNum++;

  } // while (true)

  if (_debug) {

    cerr << "uFieldNum: " << uFieldNum << endl;
    cerr << "vFieldNum: " << vFieldNum << endl;
    cerr << "tFieldNum: " << tFieldNum << endl;
    cerr << "qFieldNum: " << qFieldNum << endl;
    cerr << "clwFieldNum: " << clwFieldNum << endl;
    cerr << "rnwFieldNum: " << rnwFieldNum << endl;
    cerr << "iceFieldNum: " << iceFieldNum << endl;
    cerr << "snowFieldNum: " << snowFieldNum << endl;
    cerr << "radTendFieldNum: " << radTendFieldNum << endl;
    cerr << "wFieldNum: " << wFieldNum << endl;
    cerr << "ppFieldNum: " << ppFieldNum << endl;
    cerr << "pstarFieldNum: " << pstarFieldNum << endl;
    cerr << "groundTFieldNum: " << groundTFieldNum << endl;
    cerr << "rainConFieldNum: " << rainConFieldNum << endl;
    cerr << "rainNonFieldNum: " << rainNonFieldNum << endl;
    cerr << "terrainFieldNum: " << terrainFieldNum << endl;
    cerr << "coriolisFieldNum: " << coriolisFieldNum << endl;
    cerr << "resTempFieldNum: " << resTempFieldNum << endl;
    cerr << "latFieldNum: " << latFieldNum << endl;
    cerr << "lonFieldNum: " << lonFieldNum << endl;
    cerr << "landUseFieldNum: " << landUseFieldNum << endl;
    cerr << "snowcovrFieldNum: " << snowcovrFieldNum << endl;
    cerr << "tseasfcFieldNum: " << tseasfcFieldNum << endl;
    cerr << "pblHgtFieldNum: " << pblHgtFieldNum << endl;
    cerr << "regimeFieldNum: " << regimeFieldNum << endl;
    cerr << "shfluxFieldNum: " << shfluxFieldNum << endl;
    cerr << "lhfluxFieldNum: " << lhfluxFieldNum << endl;
    cerr << "ustFieldNum: " << ustFieldNum << endl;
    cerr << "swdownFieldNum: " << swdownFieldNum << endl;
    cerr << "lwdownFieldNum: " << lwdownFieldNum << endl;
    cerr << "soilT1FieldNum: " << soilT1FieldNum << endl;
    cerr << "soilT2FieldNum: " << soilT2FieldNum << endl;
    cerr << "soilT3FieldNum: " << soilT3FieldNum << endl;
    cerr << "soilT4FieldNum: " << soilT4FieldNum << endl;
    cerr << "soilT5FieldNum: " << soilT5FieldNum << endl;
    cerr << "soilT6FieldNum: " << soilT6FieldNum << endl;
    cerr << "soilM1FieldNum: " << soilM1FieldNum << endl;
    cerr << "soilM2FieldNum: " << soilM2FieldNum << endl;
    cerr << "soilM3FieldNum: " << soilM3FieldNum << endl;
    cerr << "soilM4FieldNum: " << soilM4FieldNum << endl;
    cerr << "sfcrnoffFieldNum: " << sfcrnoffFieldNum << endl;
    cerr << "t2FieldNum: " << t2FieldNum << endl;
    cerr << "q2FieldNum: " << q2FieldNum << endl;
    cerr << "u10FieldNum: " << u10FieldNum << endl;
    cerr << "v10FieldNum: " << v10FieldNum << endl;
    cerr << "weasdFieldNum: " << weasdFieldNum << endl;
    cerr << "snowhFieldNum: " << snowhFieldNum << endl;
    cerr << "hc_rainField: " <<  hc_rainFieldNum << endl;
    cerr << "hn_rainField: " <<  hn_rainFieldNum << endl;

    cerr << "swfracFieldNum: " << swfracFieldNum << endl;
    cerr << "sunaltFieldNum: " << sunaltFieldNum << endl;
    cerr << "sunazmFieldNum: " << sunazmFieldNum << endl;
    cerr << "moonaltFieldNum: " << moonaltFieldNum << endl;
    cerr << "moonazmFieldNum: " << moonazmFieldNum << endl;
    cerr << "sunillFieldNum: " << sunillFieldNum << endl;
    cerr << "moonillFieldNum: " << moonillFieldNum << endl;
    cerr << "totalillFieldNum: " << totalillFieldNum << endl;

    cerr << "clwiFieldNum: " << clwiFieldNum << endl;
    cerr << "rnwiFieldNum: " << rnwiFieldNum << endl;
    cerr << "iceiFieldNum: " << iceiFieldNum << endl;
    cerr << "snowiFieldNum: " << snowiFieldNum << endl;
    cerr << "pwvFieldNum: " << pwvFieldNum << endl;

    cerr << "sunBtwFieldNum: " << sunBtwFieldNum << endl;
    cerr << "sunEtwFieldNum: " << sunEtwFieldNum << endl;
    cerr << "sunAbtwFieldNum: " << sunAbtwFieldNum << endl;
    cerr << "sunAetwFieldNum: " << sunAetwFieldNum << endl;
    cerr << "sunRiseFieldNum: " << sunRiseFieldNum << endl;
    cerr << "sunSetFieldNum: " << sunSetFieldNum << endl;
    cerr << "sunArisFieldNum: " << sunArisFieldNum << endl;
    cerr << "sunAsetFieldNum: " << sunAsetFieldNum << endl;
    cerr << "moonRisFieldNum: " << moonRisFieldNum << endl;
    cerr << "moonSetFieldNum: " << moonSetFieldNum << endl;
    cerr << "moonAriFieldNum: " << moonAriFieldNum << endl;
    cerr << "moonAseFieldNum: " << moonAseFieldNum << endl;
  }

  // set file state markers

  long nextOffset = ftell(_in);
  if(_fileSize - nextOffset > 100) {
    _more = true;
  } else {
    _more = false;
  }

  // save the data set len
  
  _dataSetLen = nextOffset - _dataSetOffset;

  return (0);

}

/////////////////////////////////////
// check the dimensions of a 3D field

int MM5DataV3::_check3dSize(int nz, int ny, int nx, const char *field_name)

{

  if (nz != nSigma || ny != nyDot || nx != nxDot) {
    cerr << "ERROR - MM5DataV3::_readDataset::_check3dSize" << endl;
    cerr << "  Field: " << field_name << endl;
    cerr << "  Dimensions do not match." << endl;
    cerr << "  nSigma, nyDot, nxDot: "
	 << nSigma << ", " << nyDot << ", " << nxDot << endl;
    cerr << "  nz, ny, nx: "
	 << nz << ", " << ny << ", " << nx << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////
// check the dimensions of a 2D field

int MM5DataV3::_check2dSize(int ny, int nx, const char *field_name)

{

  if (ny != nyDot || nx != nxDot) {
    cerr << "ERROR - MM5DataV3::_readDataset::_check2dSize" << endl;
    cerr << "  Field: " << field_name << endl;
    cerr << "  Dimensions do not match." << endl;
    cerr << "  nyDot, nxDot: "
	 << nyDot << ", " << nxDot << endl;
    cerr << "  ny, nx: "
	 << ny << ", " << nx << endl;
    return -1;
  }

  return 0;
}


/////////////////
// _load3dField()
//
// Load in a 3D field
//

void MM5DataV3::_load3dField(fl32 ****field_p, bool is_dot)

{

  // allocate field
  if (*field_p != NULL) {
    ufree3((void ***) *field_p);
  }
  *field_p = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  fl32 ***field = *field_p;

  // zero out field
  memset(**field, 0, nSigma * nLat * nLon * sizeof(fl32));

  if (is_dot) {

    // dot field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  field[jsig][ilat][ilon] =
	    ((_field3d[isig][ilon][ilat] +
	      _field3d[isig][ilon][ilat+1] +
	      _field3d[isig][ilon+1][ilat] +
	      _field3d[isig][ilon+1][ilat+1]) / 4.0);
	}
      }
    }

  } else {

    // cross field
    
    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  field[jsig][ilat][ilon] = _field3d[isig][ilon][ilat];
	}
      }
    }

  }

}
   
////////////////////
// _load3dField2Dot()
//
// Load in a 3D field stored out at dot locations
//

void MM5DataV3::_load3dField2Dot(fl32 ****field_p)

{

  // allocate field
  if (*field_p != NULL) {
    ufree3((void ***) *field_p);
  }
  *field_p = (fl32 ***) umalloc3(nSigma, nyDot, nxDot, sizeof(fl32));
  fl32 ***field = *field_p;

  // zero out field
  memset(**field, 0, nSigma * nyDot * nxDot * sizeof(fl32));

  for (int isig = 0; isig < nSigma; isig++) {
    int jsig = nSigma - isig - 1;
    for (int ilat = 0; ilat < nyDot; ilat++) {
      for (int ilon = 0; ilon < nxDot; ilon++) {
	field[jsig][ilat][ilon] = _field3d[isig][ilon][ilat];
      }
    }
  }
  
}
   
/////////////////
// _load2dField
//
// Load in a 2D field
//

void MM5DataV3::_load2dField(fl32 ***field_p, bool is_dot)
  
{

  if (*field_p != NULL) {
    ufree2((void **) *field_p);
  }
  *field_p = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  fl32 **field = *field_p;

  // zero out field
  memset(*field, 0, nLat * nLon * sizeof(fl32));

  if (is_dot) {

    // dot field
    
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	field[ilat][ilon] =
	  (_field2d[ilon][ilat] +
	   _field2d[ilon][ilat+1] +
	   _field2d[ilon+1][ilat] +
	   _field2d[ilon+1][ilat+1]) / 4.0;
      }
    }
    
  } else {

    // cross field

    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	field[ilat][ilon] = _field2d[ilon][ilat];
      }
    }

  }

}

////////////////////
// _load2dField2Dot()
//
// Load in a 2D field stored out at dot locations
//

void MM5DataV3::_load2dField2Dot(fl32 ***field_p)

{

  // allocate field
  if (*field_p != NULL) {
    ufree2((void **) *field_p);
  }
  *field_p = (fl32 **) umalloc2(nyDot, nxDot, sizeof(fl32));
  fl32 **field = *field_p;

  // zero out field
  memset(*field, 0, nyDot * nxDot * sizeof(fl32));

  for (int ilat = 0; ilat < nyDot; ilat++) {
    for (int ilon = 0; ilon < nxDot; ilon++) {
      field[ilat][ilon] = _field2d[ilon][ilat];
    }
  }
  
}


/////////////////
// _loadWField()
//
// Load up W field - this is on full sigma levels and must be
// interpolated onto half sigma levels
//

void MM5DataV3::_loadWField()

{

  // allocate and zero out

  if (ww != NULL) {
    ufree3((void ***) ww);
  }
  ww = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  memset(**ww, 0, nSigma * nLat * nLon * sizeof(fl32));
  
  if (ww_full != NULL) {
    ufree3((void ***) ww_full);
  }
  ww_full = (fl32 ***) umalloc3(nSigma + 1, nLat, nLon, sizeof(fl32));
  memset(**ww_full, 0, (nSigma + 1) * nLat * nLon * sizeof(fl32));
  
  for (int isig = 0; isig < nSigma; isig++) {
    int jsig = nSigma - isig - 1;
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	ww[jsig][ilat][ilon] =
	  (_field3d[isig][ilon][ilat] + _field3d[isig+1][ilon][ilat]) / 2.0;
      }
    }
  }
  
  for (int isig = 0; isig < nSigma + 1; isig++) {
    int jsig = nSigma - isig;
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	ww_full[jsig][ilat][ilon] = _field3d[isig][ilon][ilat];
      }
    }
  }
  
}

/////////////////////
// _loadSigmaLevels()
//
// Load up sigma level array
//

int MM5DataV3::_loadSigmaLevels(int nz, fl32 *fieldData)

{

  if (nz != nSigma) {
    cerr << "ERROR - MM5DataV3::_readDataset::_loadSigmaLevels" << endl;
    cerr << "  Dimensions do not match." << endl;
    cerr << "  nSigma: " << nSigma << endl;
    cerr << "  nz: " << nz << endl;
    return -1;
  }

  for (int i = 0; i < nSigma; i++) {
    _halfSigma[nSigma - i - 1] = fieldData[i];
  }

  return 0;

}
