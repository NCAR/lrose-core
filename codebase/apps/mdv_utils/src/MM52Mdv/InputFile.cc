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
// InputFile.cc
//
// Read in MM5 file.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
//////////////////////////////////////////////////////////

#include "InputFile.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <physics/thermo.h>
#include <sys/stat.h>
#include <rapmath/math_macros.h>
using namespace std;

#define MM5_HEADER_LEN 3360000

//////////////
// Constructor

InputFile::InputFile (char *prog_name,
		      Params *params,
		      char *path)

{

  OK = TRUE;

  _progName = STRdup(prog_name);
  _params = params;
  _path = STRdup(path);

  uu = (fl32 ***) NULL;
  vv = (fl32 ***) NULL;
  ww = (fl32 ***) NULL;
  wspd = (fl32 ***) NULL;
  tk = (fl32 ***) NULL;
  tc = (fl32 ***) NULL;
  qq = (fl32 ***) NULL;
  pres = (fl32 ***) NULL;
  zz = (fl32 ***) NULL;
  pp = (fl32 ***) NULL;
  rh = (fl32 ***) NULL;
  cloud = (fl32 ***) NULL;
  precip = (fl32 ***) NULL;
  turb = (fl32 ***) NULL;
  icing = (fl32 ***) NULL;
  pstar = (fl32 **) NULL;
  lat = (fl32 **) NULL;
  lon = (fl32 **) NULL;
  terrain = (fl32 **) NULL;
  freeze = (fl32 **) NULL;

  fieldInterp = (double *) NULL;

  forecastTimes = (time_t *) NULL;
  nForecasts = 0;

  _field3d = (fl32 ***) NULL;
  _field2d = (fl32 **) NULL;

  _mif = (si32 **) NULL;
  _mrf = (fl32 **) NULL;
  _halfSigma = (fl32 *) NULL;

  _flevel = (FlightLevel *) NULL;

  _mif = (si32 **) umalloc2(20, 1000, sizeof(si32));
  _mrf = (fl32 **) umalloc2(20, 1000, sizeof(fl32));

  // open the file

  _in = NULL;
  if ((_in = fopen(_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:InputFile::InputFile\n", _progName);
    fprintf(stderr, "Cannot open file for reading\n");
    perror(_path);
    OK = FALSE;
    return;
  }

  // create flight level object
  
  _flevel = new FlightLevel(_progName, _params);
  if (!_flevel->OK) {
    OK = FALSE;
    return;
  }

}

/////////////
// Destructor

InputFile::~InputFile()

{

  if (_in != NULL) {
    fclose(_in);
  }

  if (_flevel != NULL) {
    delete(_flevel);
  }

  if (uu != NULL) {
    ufree3((void ***) uu);
  }
  if (vv != NULL) {
    ufree3((void ***) vv);
  }
  if (ww != NULL) {
    ufree3((void ***) ww);
  }
  if (wspd != NULL) {
    ufree3((void ***) wspd);
  }
  if (tc != NULL) {
    ufree3((void ***) tc);
  }
  if (tk != NULL) {
    ufree3((void ***) tk);
  }
  if (qq != NULL) {
    ufree3((void ***) qq);
  }
  if (pres != NULL) {
    ufree3((void ***) pres);
  }
  if (pp != NULL) {
    ufree3((void ***) pp);
  }
  if (zz != NULL) {
    ufree3((void ***) zz);
  }
  if (rh != NULL) {
    ufree3((void ***) rh);
  }
  if (cloud != NULL) {
    ufree3((void ***) cloud);
  }
  if (precip != NULL) {
    ufree3((void ***) precip);
  }
  if (turb != NULL) {
    ufree3((void ***) turb);
  }
  if (icing != NULL) {
    ufree3((void ***) icing);
  }
  if (pstar != NULL) {
    ufree2((void **) pstar);
  }
  if (lat != NULL) {
    ufree2((void **) lat);
  }
  if (lon != NULL) {
    ufree2((void **) lon);
  }
  if (terrain != NULL) {
    ufree2((void **) terrain);
  }
  if (freeze != NULL) {
    ufree2((void **) freeze);
  }

  if (fieldInterp != NULL) {
    ufree(fieldInterp);
  }

  if (forecastTimes != NULL) {
    ufree(forecastTimes);
  }

  if (_field3d != NULL) {
    ufree3((void ***) _field3d);
  }
  if (_field2d != NULL) {
    ufree2((void **) _field2d);
  }

  if (_mif != NULL) {
    ufree2((void **) _mif);
  }
  if (_mrf != NULL) {
    ufree2((void **) _mrf);
  }
  if (_halfSigma != NULL) {
    ufree(_halfSigma);
  }

  
  STRfree(_progName);
  STRfree(_path);
  
}

////////////////
// readHeaders()
//
// returns 0 on success, -1 on failure

int InputFile::readHeaders()

{

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Reading file %s\n", _path);
  }

  if (_readHdrInfo()) {
    return (-1);
  }
  if (_readTimes()) {
    return (-1);
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    _print();
  }

  return (0);

}

////////////////////
// read a data set()

int InputFile::readDataset(time_t forecast_time)

{

  int setNum = -1;

  for (int i = 0; i < nForecasts; i++) {
    if (forecastTimes[i] == forecast_time) {
      setNum = i;
      break;
    }
  }

  if (setNum < 0) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot find forecast time %s\n",
	    utimstr(forecast_time));
    fprintf(stderr, "File is %s\n", _path);
    return (-1);
  }

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Reading dataset at time %s\n", utimstr(forecast_time));
  }

  // 2D fields

  if (_read2dField(setNum, _pstarField, "pstar", &pstar)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read pstar field\n");
    return (-1);
  }
  if (_read2dField(setNum, _latField, "lat", &lat)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read latitude field\n");
    return (-1);
  }
  if (_read2dField(setNum, _lonField, "lon", &lon)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read longitude field\n");
    return (-1);
  }
  if (_read2dField(setNum, _terrainField, "terrain", &terrain)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read terrain field\n");
    return (-1);
  }

  // 3D fields

  if (_read3dField(setNum, _uField, "U", &uu)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read U field\n");
    return (-1);
  }
  if (_read3dField(setNum, _vField, "V", &vv)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read V field\n");
    return (-1);
  }
  if (_read3dField(setNum, _tField, "temperature", &tk)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read t field\n");
    return (-1);
  }
  if (_read3dField(setNum, _qField, "Mixing ratio", &qq)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read q field\n");
    return (-1);
  }
  if (_read3dField(setNum, _ppField, "Pres pert", &pp)) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read pp field\n");
    return (-1);
  }
  _read3dField(setNum, _clwField, "cloud", &cloud);
  _read3dField(setNum, _rnwField, "precip", &precip);
  _readwField(setNum);

  _changeMixrUnits();
  _loadTempCField();
  _loadPressureField();
  _loadRhField();
  _loadIcingField();
  _loadTurbField();
  _loadFreezeField();
  _loadWspdField();

  if (_params->debug >= Params::DEBUG_VERBOSE) {

    for (int ilat = 0; ilat < nLat; ilat++) {
      fprintf(stderr,
	      "[%2s][%2s]: "
	      "%8s %8s %8s %8s %8s\n",
	      "iy", "ix",
	      "pstar", "lat", "lon", "ht", "fzl");
      
      for (int ilon = 0; ilon < nLon; ilon++) {
	fprintf(stderr, "[%.2d][%.2d]: %8g %8g %8g %8g %8.0f\n",
		ilat, ilon,
		pstar[ilat][ilon],
		lat[ilat][ilon],
		lon[ilat][ilon],
		terrain[ilat][ilon],
		freeze[ilat][ilon]);
      }
      fprintf(stderr,
	      "--------------------------------------"
	      "--------------------------------------\n");
    }
  
    for (int isig = 0; isig < nSigma; isig++) {
      for (int ilat = 0; ilat < nLat; ilat++) {
	
	fprintf(stderr,
		"[%2s][%2s][%2s]: "
		"%6s %6s %6s %7s %6s %6s "
		"%6s %5s %6s %7s %7s "
		"%5s %5s %5s %5s\n",
		"iz", "iy", "ix",
		"uu", "vv", "ww", "pres", "zz", "tc",
		"qq", "rh", "pp", "cloud", "precip",
		"icing", "turb", "ht", "fzl");
	
	for (int ilon = 0; ilon < nLon; ilon++) {
	  
	  fprintf(stderr,
		  "[%.2d][%.2d][%.2d]: "
		  "%6.1f %6.1f %6.3f %7.2f %6.1f %6.1f "
		  "%6.2f %5.1f %6.0f %7.4f %7.4f "
		  "%5.0f %5.0f %5.0f %5.0f\n",
		  isig, ilat, ilon,
		  uu[isig][ilat][ilon],
		  vv[isig][ilat][ilon],
		  ww[isig][ilat][ilon],
		  pres[isig][ilat][ilon],
		  zz[isig][ilat][ilon],
		  tc[isig][ilat][ilon],
		  qq[isig][ilat][ilon],
		  rh[isig][ilat][ilon],
		  pp[isig][ilat][ilon],
		  cloud[isig][ilat][ilon],
		  precip[isig][ilat][ilon],
		  icing[isig][ilat][ilon],
		  turb[isig][ilat][ilon],
		  terrain[ilat][ilon],
		  freeze[ilat][ilon]);
	  
	}
	
	fprintf(stderr,
		"--------------------------------------"
		"--------------------------------------\n");
      }
      
      fprintf(stderr,
	      "======================================"
	      "======================================\n");
    }

  } // if (_params->debug >= DEBUG_VERBOSE)

  return (0);

}

///////////////////
// findHdrRecords()
//
// Find the fortran record markers around the header
//

void InputFile::findHdrRecords()

{

  si32 iii;
  int dataset = 0;
  int count = 0;
  int prev_end = 0;
  int inHeader = FALSE;
  int start = 0, end;

  fprintf(stderr, "\n");
  fprintf(stderr, "%s: looking for FORTRAN headers in file:\n   %s\n",
	  _progName, _path);
  fprintf(stderr, "\n");
  fprintf(stderr, "Addresses and lengths are 4-byte values\n\n");

  fseek(_in, 0, SEEK_SET);
  while (!feof(_in)) {
    if (ufread(&iii, sizeof(si32), 1, _in) != 1) {
      fprintf(stderr,
	      "Data   found: start %10d, end %10d, len %10d\n\n",
	      prev_end + 1, count - 1, count - prev_end - 1);
      fprintf(stderr, "End of file %s\n", _path);
      break;
    }
    BE_from_array_32(&iii, 4);
    if (iii == MM5_HEADER_LEN) {
      if (!inHeader) {
	start = count;
	inHeader = TRUE;
	dataset++;
      } else {
	end = count;
	inHeader = FALSE;
	if (prev_end != 0) {
	  fprintf(stderr,
		  "Data   found: start %10d, end %10d, len %10d\n",
		  prev_end + 1, start - 1, start - prev_end - 1);
	}
	fprintf(stderr,
		"Data set %2d ----------------------------"
		"----------------------\n", dataset);
	fprintf(stderr,
		"Header found: start %10d, end %10d, len %10d\n",
		start, end, end - start + 1);
	prev_end = end;
      }
    }
    count++;
  }

}

//////////////////
// interp3dField()
//
// Load up the sigma field array interpolated for a given point.
//
// returns ptr to array on success, NULL on failure.

double *InputFile::interp3dField(int ilat, int ilon,
				 char *name, fl32 ***field,
				 double wt_sw, double wt_nw,
				 double wt_ne, double wt_se,
				 int *sigma_needed)
  
  
{
  
  if (field == NULL) {
    fprintf(stderr, "ERROR - %s:InputFile::interpPresForSigma\n", _progName);
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return (NULL);
  }
  
  if (fieldInterp == NULL) {
    fieldInterp = (double *) umalloc (nSigma * sizeof(double));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    if (sigma_needed == NULL ||
	sigma_needed[isig] == TRUE) {
      fieldInterp[isig] = (field[isig][ilat][ilon] * wt_sw +
			   field[isig][ilat+1][ilon] * wt_nw +
			   field[isig][ilat+1][ilon+1] * wt_ne +
			   field[isig][ilat][ilon+1] * wt_se);
    }
  }
  
  return (fieldInterp);
  
}

//////////////////
// interp2dField()
//
// Load up interp_val_p with value interpolated for a given point.
//
// returns val on success, MISSING_DOUBLE on failure.

double InputFile::interp2dField(int ilat, int ilon,
				char *name, fl32 **field,
				double wt_sw, double wt_nw,
				double wt_ne, double wt_se)
  
{
  
  if (field == NULL) {

    fprintf(stderr, "ERROR - %s:InputFile::interpPresForSigma\n", _progName);
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return (MISSING_DOUBLE);

  }
  
  double interp_val = (field[ilat][ilon] * wt_sw +
		       field[ilat+1][ilon] * wt_nw +
		       field[ilat+1][ilon+1] * wt_ne +
		       field[ilat][ilon+1] * wt_se);

  return (interp_val);
  
}

///////////////////
// get3dScaleBias()
//
// Compute the scale and bias for a 3d field
//
// Returns 0 on success, -1 on failure
//

int InputFile::get3dScaleBias(char *name, fl32 ***field,
			      double *scale_p, double *bias_p)

{
  
  if (field == NULL) {
    fprintf(stderr, "ERROR - %s:InputFile::get3dScaleBias\n", _progName);
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return (-1);
  }
  
  double minVal = 1.0e99;
  double maxVal = -1.0e99;

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	minVal = MIN(minVal, field[isig][ilat][ilon]);
	maxVal = MAX(maxVal, field[isig][ilat][ilon]);
      }
    }
  }
  
  if (minVal > maxVal) {
    // no valid data found
    return (-1);
  }
  
  // compute scale and bias
  double range = maxVal - minVal;
  double scale = range / 250.0;
  double bias = minVal - scale * 2.0;
  
  *scale_p = scale;
  *bias_p = bias;
  
  return (0);

}

///////////////////
// get2dScaleBias()
//
// Compute the scale and bias for a 2d field
//
// Returns 0 on success, -1 on failure
//

int InputFile::get2dScaleBias(char *name, fl32 **field,
			      double *scale_p, double *bias_p)

{
  
  if (field == NULL) {
    
    fprintf(stderr, "ERROR - %s:InputFile::get2dScaleBias\n", _progName);
    fprintf(stderr, "%s array not loaded yet, operation invalid.\n", name);
    return (-1);

  }
  
  double minVal = 1.0e99;
  double maxVal = -1.0e99;

  for (int ilat = 0; ilat < nLat; ilat++) {
    for (int ilon = 0; ilon < nLon; ilon++) {
      minVal = MIN(minVal, field[ilat][ilon]);
      maxVal = MAX(maxVal, field[ilat][ilon]);
    }
  }
  
  if (minVal > maxVal) {
    // no valid data found
    return (-1);
  }
  
  // compute scale and bias
  double range = maxVal - minVal;
  double scale = range / 250.0;
  double bias = minVal - scale * 2.0;
  
  *scale_p = scale;
  *bias_p = bias;
  
  return (0);

}

/////////////////
// _readHdrInfo()
//
// Reads the first header to set class members
//
// returns 0 on success, -1 on failure

int InputFile::_readHdrInfo()

{

  // fortran record length
  _readFortRecLen();

  // MIF array
  
  if (ufread(*_mif, sizeof(si32), 20000, _in) != 20000) {
    fprintf(stderr, "ERROR - %s:InputFile::_readHdrInfo\n", _progName);
    fprintf(stderr, "Cannot read mif array\n");
    perror(_path);
    return (-1);
  }
  BE_from_array_32(*_mif, 20000 * sizeof(si32));

  // MRF array

  if (ufread(*_mrf, sizeof(fl32), 20000, _in) != 20000) {
    fprintf(stderr, "ERROR - %s:InputFile::_readHdrInfo\n", _progName);
    fprintf(stderr, "Cannot read mrf array\n");
    perror(_path);
    return (-1);
  }
  BE_from_array_32(*_mrf, 20000 * sizeof(fl32));

  if (_mif[0][0] != 6) {
    fprintf(stderr, "ERROR - %s:InputFile::_readHdrInfo\n", _progName);
    fprintf(stderr, "This is not an MM5 output file.\n");
    return (-1);
  }
  unsigned int iset = _mif[0][0]-1;
  
  // set grid info, sizes
  
  nSigma = (int) (_mrf[4][100] + 0.5);

  _nyDot = _mif[0][103];
  _nxDot = _mif[0][104];
  _nPtsDotPlane = _nxDot * _nyDot;

  nLon = _nxDot - 1;
  nLat = _nyDot - 1;

  _n3d = _mif[iset][200];
  _n2d = _mif[iset][201];

  // 3d field array has extra level for reading w field
  if (_field3d == NULL) {
    _field3d = (fl32 ***) umalloc3(nSigma+1, _nyDot, _nxDot, sizeof(fl32));
  }

  if (_field2d == NULL) {
    _field2d = (fl32 **) umalloc2(_nyDot, _nxDot, sizeof(fl32));
  }

  if (_halfSigma == NULL) {
    _halfSigma = (fl32 *) umalloc(nSigma * sizeof(fl32));
  }

  // load up half sigma levels

  for (int i = 0; i < nSigma; i++) {
    _halfSigma[nSigma - i - 1] = _mrf[5][i+101];
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "nsigma: %d\n", nSigma);
    fprintf(stderr, "nxDot, nyDot: %d, %d\n", _nxDot, _nyDot);
    fprintf(stderr, "n3d, n2d: %d, %d\n", _n3d, _n2d);
    for (int i = 0; i < nSigma; i++) {
      fprintf(stderr, "half sigma[%d]: %g\n", i, _halfSigma[i]);
    }
  }

  // read the field names

  long field_name_start =
    sizeof(si32) +            // fortran rec len
    40000 * sizeof(si32) +    // mif and mrf
    5204 * 80;                // mifc strings

  char mifc[81];
  
  _uField = -1;
  _vField = -1;
  _tField = -1;
  _qField = -1;
  _clwField = -1;
  _rnwField = -1;
  _wField = -1;
  _ppField = -1;

  fseek(_in, field_name_start, SEEK_SET);

  for (int i = 0; i < _n3d; i++) {
    if (ufread(mifc, 80, 1, _in) != 1) {
      fprintf(stderr, "ERROR - %s:InputFile::_readHdrInfo\n", _progName);
      fprintf(stderr, "Cannot read 3d field from mifc\n");
      perror(_path);
      return (-1);
    }
    mifc[8] = '\0';
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "3d field %d: %s\n", i, mifc);
    }
    if (!strcmp(mifc, "U       ")) {
      _uField = i;
    } else if (!strcmp(mifc, "V       ")) {
      _vField = i;
    } else if (!strcmp(mifc, "T       ")) {
      _tField = i;
    } else if (!strcmp(mifc, "Q       ")) {
      _qField = i;
    } else if (!strcmp(mifc, "CLW     ")) {
      _clwField = i;
    } else if (!strcmp(mifc, "RNW     ")) {
      _rnwField = i;
    } else if (!strcmp(mifc, "W       ")) {
      _wField = i;
    } else if (!strcmp(mifc, "PP      ")) {
      _ppField = i;
    }
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "uField: %d\n", _uField);
    fprintf(stderr, "vField: %d\n", _vField);
    fprintf(stderr, "tField: %d\n", _tField);
    fprintf(stderr, "qField: %d\n", _qField);
    fprintf(stderr, "clwField: %d\n", _clwField);
    fprintf(stderr, "rnwField: %d\n", _rnwField);
    fprintf(stderr, "wField: %d\n", _wField);
    fprintf(stderr, "ppField: %d\n", _ppField);
  }
    
  _pstarField = -1;
  _latField = -1;
  _lonField = -1;
  _terrainField = -1;
  
  for (int i = 0; i < _n2d; i++) {
    if (ufread(mifc, 80, 1, _in) != 1) {
      fprintf(stderr, "ERROR - %s:InputFile::_readHdrInfo\n", _progName);
      fprintf(stderr, "Cannot read 2d field from mifc\n");
      perror(_path);
      return (-1);
    }
    mifc[8] = '\0';
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "2d field %d: %s\n", i, mifc);
    }
    if (!strcmp(mifc, "TERRAIN ")) {
      _terrainField = i;
    } else if (!strcmp(mifc, "PSTARCRS")) {
      _pstarField = i;
    } else if (!strcmp(mifc, "LATITCRS")) {
      _latField = i;
    } else if (!strcmp(mifc, "LONGICRS")) {
      _lonField = i;
    }
  }
    
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "terrainField: %d\n", _terrainField);
    fprintf(stderr, "pstarField: %d\n", _pstarField);
    fprintf(stderr, "latField: %d\n", _latField);
    fprintf(stderr, "lonField: %d\n", _lonField);
  }

  // compute field data length
  // includes 2*sizeof(si32) for each record to account for the
  // fortran record length entries

  _fieldDataLen =
    (_n3d * (_nxDot * _nyDot * nSigma * sizeof(fl32) + 2 * sizeof(si32))) +
    (_n2d * (_nxDot * _nyDot * sizeof(fl32) + 2 * sizeof(si32)));

  // wfield has one extra level

  if (_wField >= 0) {
    _fieldDataLen += _nxDot * _nyDot * sizeof(fl32);
  }

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "fieldDataLen: %d bytes\n", _fieldDataLen);
  }

  // pTop etc

  _pTop = _mrf[1][0];
  _pos = _mrf[5][1] / 100.0;
  _tso = _mrf[5][2];
  _tlp = _mrf[5][3];

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "pTop, pos, tos, tlp: %g, %g, %g, %g\n",
	    _pTop, _pos, _tso, _tlp);
  }
    
  // compute single forecast output len and number of
  // forecasts in the file

  _datasetLen = MM5_HEADER_LEN + 2 * sizeof(si32) + _fieldDataLen;

  struct stat file_stat;

  if (stat(_path, &file_stat)) {
    fprintf(stderr, "ERROR - %s:InputFile::_readHdrInfo\n", _progName);
    fprintf(stderr, "Cannot stat file %s\n", _path);
    perror(_path);
    return (-1);
  }

  nForecasts = file_stat.st_size / _datasetLen;

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "datasetLen: %d bytes\n", _datasetLen);
    fprintf(stderr, "nForecasts: %d\n", nForecasts);
  }

  return (0);

}

///////////////
// _readTimes()
//
// Reads the times of the data sets
//
// returns 0 on success, -1 on failure

int InputFile::_readTimes()

{
  
  forecastTimes = (time_t *) umalloc(nForecasts * sizeof(time_t));

  for (int i = 0; i < nForecasts; i++) {
    
    long start = i * _datasetLen + sizeof(si32) + 5000 * sizeof(si32);
    fseek(_in, start, SEEK_SET);

    si32 forecastHdr[20];
    
    if (ufread(forecastHdr, sizeof(si32), 20, _in) != 20) {
      fprintf(stderr, "ERROR - %s:InputFile::_readTimes\n", _progName);
      fprintf(stderr, "Cannot read forecastHdr\n");
      perror(_path);
      return (-1);
    }
    BE_from_array_32(forecastHdr, 20 * sizeof(si32));

    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "----> Forecast number %d\n", i);
      fprintf(stderr, "forecastTime: %d\n", forecastHdr[0]);
      fprintf(stderr, "modelTime: %d\n", forecastHdr[1]);
      fprintf(stderr, "min: %d\n", forecastHdr[10]);
      fprintf(stderr, "hour %d\n", forecastHdr[11]);
      fprintf(stderr, "day: %d\n", forecastHdr[12]);
      fprintf(stderr, "month: %d\n", forecastHdr[13]);
      fprintf(stderr, "year: %d\n", forecastHdr[14]);
      fprintf(stderr, "cent: %d\n", forecastHdr[15]);
    }
    
    date_time_t ftime;
    ftime.year = forecastHdr[15] * 100 + forecastHdr[14];
    ftime.month = forecastHdr[13];
    ftime.day = forecastHdr[12];
    ftime.hour = forecastHdr[11];
    ftime.min = forecastHdr[10];
    ftime.sec = 0;
    uconvert_to_utime(&ftime);
    forecastTimes[i] = ftime.unix_time;
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "Forecast time %d: %s\n", i, utimestr(&ftime));
    }

  }

  forecastDelta = (int)
    ((double) (forecastTimes[nForecasts - 1] - forecastTimes[0]) /
     (double) (nForecasts - 1) + 0.5);
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Forecast delta: %d\n", forecastDelta);
  }
  
  return (0);

}

///////////
// _print()
//
// Print out labels etc.

void InputFile::_print()

{

  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Reading file %s\n", _path);
  }

  fprintf(stderr, "*******************************************************\n");
  fprintf(stderr, "MM5 file %s\n", _path);
  fprintf(stderr, "*******************************************************\n");

  // MIFC

  // read the field names

  long mifc_start = sizeof(si32) + // fortran rec len
    40000 * sizeof(si32);          // mif and mrf
  fseek(_in, mifc_start, SEEK_SET);
  
  fprintf(stderr, "\n================== MIF ===================\n\n");

  char mifc[81];

  for (int i = 0; i < 20000; i++) {
    
    if (ufread(mifc, 80, 1, _in) != 1) {
      fprintf(stderr, "ERROR - %s:InputFile::_print\n", _progName);
      fprintf(stderr, "Cannot read mifc entry\n");
      perror(_path);
      return;
    }

    mifc[80] = '\0';
    for (int j = 79; j >= 0; j--) {
      if (mifc[j] != ' ') {
	break;
      }
      mifc[j] = '\0';
    }

    int ii = i % 1000;
    int jj = i / 1000;

    if (_mif[jj][ii] != -999) {
      fprintf(stderr, "[%2d][%4d] = %8d: %s\n",
	      jj+1, ii+1, _mif[jj][ii], mifc);
    }

  }

  // MRFC

  fprintf(stderr, "\n================== MRF ===================\n\n");

  char mrfc[81];

  for (int i = 0; i < 20000; i++) {

    if (ufread(mrfc, 80, 1, _in) != 1) {
      fprintf(stderr, "ERROR - %s:InputFile::_print\n", _progName);
      fprintf(stderr, "Cannot read mrfc entry\n");
      perror(_path);
      return;
    }

    mrfc[80] = '\0';
    for (int j = 79; j >= 0; j--) {
      if (mrfc[j] != ' ') {
	break;
      }
      mrfc[j] = '\0';
    }

    int ii = i % 1000;
    int jj = i / 1000;

    if (_mrf[jj][ii] > -998.0 || _mrf[jj][ii] < -1000.0) {
      fprintf(stderr, "[%2d][%4d] = %8g: %s\n",
	      jj+1, ii+1, _mrf[jj][ii], mrfc);
    }

  }

}

///////////////////////////////
// _readFortRecLen()
//
// Read a fortran record length
//

void InputFile::_readFortRecLen()
  
{
  
  si32 reclen;
  
  if (ufread(&reclen, sizeof(si32), 1, _in) != 1) {
    fprintf(stderr, "ERROR - %s:InputFile::_readFortRecLen\n", _progName);
    fprintf(stderr, "Cannot read fortran rec len\n");
    perror(_path);
  }
  
  BE_from_array_32(&reclen, sizeof(si32));

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "Fortran rec len 2: %d\n", reclen);
  }

}

//////////////
// _offset3d()
//
// Compute the offset of a 3d field
//

long InputFile::_offset3d(int data_set_num, int field_num_3d)

{

  long offset;

  offset = data_set_num * _datasetLen; // previous data sets

  offset += MM5_HEADER_LEN + 2 * sizeof(si32); // Header + fortran rec lengths

  // previous 3d fields

  offset += field_num_3d *
    (_nxDot * _nyDot * nSigma * sizeof(fl32) + 2 * sizeof(si32));

  // w field has one extra level

  if (_wField >= 0 && field_num_3d > _wField) {
    offset += _nxDot * _nyDot * sizeof(fl32);
  }

  // fortran record length

  offset += sizeof(si32);

  return (offset);

}

//////////////
// _offset2d()
//
// Compute the offset of a 2d field
//

long InputFile::_offset2d(int data_set_num, int field_num_2d)

{
  
  long offset;

  // 3d fields

  offset = _offset3d(data_set_num, _n3d);
  
  // prev 2d fields

  offset += field_num_2d *
    (_nxDot * _nyDot * sizeof(fl32) + 2 * sizeof(si32));
  
  return (offset);

}

/////////////////
// _read3dField()
//
// Read in a 3D field
//

int InputFile::_read3dField(int data_set_num, int field_num_3d,
			    char *field_name, fl32 ****field_p)

{
  
  if (*field_p == NULL) {
    *field_p = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  // zero out field
  memset(***field_p, 0, nSigma * nLat * nLon * sizeof(fl32));

  if (field_num_3d < 0) {
    fprintf(stderr, "WARNING - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "3d field %d for %s not in data set\n", field_num_3d,
	    field_name);
    return (-1);
  }

  int offset = _offset3d(data_set_num, field_num_3d);
  fseek(_in, offset, SEEK_SET);

  int npts = _nPtsDotPlane * nSigma;
  if (ufread(**_field3d, sizeof(fl32), npts, _in) != npts) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read 3d field %d for %s\n", field_num_3d,
	    field_name);
    perror(_path);
    return (-1);
  }
  
  BE_from_array_32(**_field3d, npts * sizeof(fl32));

  int idot = _mif[5][204+field_num_3d] / 10;
    
  if (idot == 0) {

    // cross field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  (*field_p)[jsig][ilat][ilon] =
	    _field3d[isig][ilon][ilat] / pstar[ilat][ilon];
	}
      }
    }

  } else {

    // dot field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  (*field_p)[jsig][ilat][ilon] =
	    ((_field3d[isig][ilon][ilat] +
	      _field3d[isig][ilon][ilat+1] +
	      _field3d[isig][ilon+1][ilat] +
	      _field3d[isig][ilon+1][ilat+1]) /
	     (pstar[ilat][ilon] * 4.0));
	}
      }
    }

  }

  return (0);

}
   
/////////////////
// _read2dField()
//
// Read in a 2D field
//

int InputFile::_read2dField(int data_set_num, int field_num_2d,
			    char *field_name, fl32 ***field_p)

{

  int offset = _offset2d(data_set_num, field_num_2d);
  fseek(_in, offset, SEEK_SET);

  if (ufread(*_field2d, sizeof(fl32), _nPtsDotPlane, _in) != _nPtsDotPlane) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read 2d field %d for %s\n", field_num_2d,
	    field_name);
    perror(_path);
    return (-1);
  }

  BE_from_array_32(*_field2d, _nPtsDotPlane * sizeof(fl32));

  if (*field_p == NULL) {
    *field_p = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }

  int idot = _mif[5][204+_n3d+field_num_2d] / 10;

  if (idot == 0) {

    // cross field

    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	(*field_p)[ilat][ilon] = _field2d[ilon][ilat];
      }
    }

  } else {

    // dot field

    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	(*field_p)[ilat][ilon] =
	  (_field2d[ilon][ilat] +
	   _field2d[ilon][ilat+1] +
	   _field2d[ilon+1][ilat] +
	   _field2d[ilon+1][ilat+1]) / 4.0;
      }
    }
    
  }

  return (0);

}

/////////////////
// _readwField()
//
// Read in W field - this is on full sigma levels and must be
// interpolated onto half sigma levels
//

int InputFile::_readwField(int data_set_num)

{
  
  if (ww == NULL) {
    ww = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  // zero out field
  memset(**ww, 0, nSigma * nLat * nLon * sizeof(fl32));
  
  if (_wField < 0) {
    fprintf(stderr, "WARNING - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "3d w field not in data set\n");
    return (-1);
  }

  int offset = _offset3d(data_set_num, _wField);
  fseek(_in, offset, SEEK_SET);
  
  int npts = _nPtsDotPlane * (nSigma + 1);
  if (ufread(**_field3d, sizeof(fl32), npts, _in) != npts) {
    fprintf(stderr, "ERROR - %s:InputFile::readDataset\n", _progName);
    fprintf(stderr, "Cannot read 3d field for w\n");
    perror(_path);
    return (-1);
  }
  
  BE_from_array_32(**_field3d, npts * sizeof(fl32));

  int idot = _mif[5][204+_wField] / 10;
    
  if (idot == 0) {

    // cross field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  ww[jsig][ilat][ilon] =
	    (_field3d[isig][ilon][ilat] + _field3d[isig+1][ilon][ilat]) /
	    (pstar[ilat][ilon] * 2.0);
	}
      }
    }

  } else {

    // dot field

    for (int isig = 0; isig < nSigma; isig++) {
      int jsig = nSigma - isig - 1;
      for (int ilat = 0; ilat < nLat; ilat++) {
	for (int ilon = 0; ilon < nLon; ilon++) {
	  ww[jsig][ilat][ilon] =
	    ((_field3d[isig][ilon][ilat] +
	      _field3d[isig][ilon][ilat+1] +
	      _field3d[isig][ilon+1][ilat] +
	      _field3d[isig][ilon+1][ilat+1] +
	      _field3d[isig+1][ilon][ilat] +
	      _field3d[isig+1][ilon][ilat+1] +
	      _field3d[isig+1][ilon+1][ilat] +
	      _field3d[isig+1][ilon+1][ilat+1]) /
	     (pstar[ilat][ilon] * 8.0));
	}
      }
    }

  }

  return (0);

}
   
///////////////////////
// _changeMixrUnits()
//
// Change mixing ratio units to g/kg
//

void InputFile::_changeMixrUnits()
  
{
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	qq[isig][ilat][ilon] *= 1000.0;
	cloud[isig][ilat][ilon] *= 1000.0;
	precip[isig][ilat][ilon] *= 1000.0;
      }
    }
  }

}
   
///////////////////////
// _loadTempCField()
//
// Load Temp in C from Temp in K
//

void InputFile::_loadTempCField()
  
{
  
  if (tc == NULL) {
    tc = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	tc[isig][ilat][ilon] = tk[isig][ilat][ilon] - 273.15;
      }
    }
  }

}
   
///////////////////////
// _loadPressureField()
//
// Load pressure (MB) field from pstar and pTop
//

#define RR 287.04
#define GG 9.80665

void InputFile::_loadPressureField()
  
{

  if (pres == NULL) {
    pres = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  if (zz == NULL) {
    zz = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	double pnot = pstar[ilat][ilon] * 10.0 * _halfSigma[isig] + _pTop;
	pres[isig][ilat][ilon] = pnot + pp[isig][ilat][ilon] / 100.0;
	double aa = log(pnot / _pos);
	zz[isig][ilat][ilon] =
	  -1.0 * ((RR * _tlp * pow(aa, 2.0)) / (2.0 * GG) +
		  (RR * _tso * aa) / GG);
      }
    }
  }
  
}

///////////////////////
// _loadRhField()
//
// Load relative humidity (%) field from mixing ratio,
// pressure and temperature
//

void InputFile::_loadRhField()
  
{
  
  if (rh == NULL) {
    rh = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }

  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	rh[isig][ilat][ilon] =
	  PHYrhmr(qq[isig][ilat][ilon],
		  pres[isig][ilat][ilon],
		  tc[isig][ilat][ilon]);
      }
    }
  }

}
   
///////////////////////
// _loadWspdField()
//
// Load wind speed in knots from U and V components.
// Also translate the U and V into knots.
//

void InputFile::_loadWspdField()
  
{
  
  if (wspd == NULL) {
    wspd = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	(uu[isig][ilat][ilon]) *= MS_TO_KNOTS;
	(vv[isig][ilat][ilon]) *= MS_TO_KNOTS;
	double u = uu[isig][ilat][ilon];
	double v = vv[isig][ilat][ilon];
	wspd[isig][ilat][ilon] = sqrt(u * u + v * v);
      }
    }
  }

}
   
///////////////////////
// _loadIcingField()
//
// Load icing severity field from cloud mixing ratio and temperature
//

#define TRACE_ICING_THRESH  0.01
#define LIGHT_ICING_THRESH  0.1
#define MOD_ICING_THRESH    0.6
#define SEVERE_ICING_THRESH 1.2

#define RPRIME 287.04

void InputFile::_loadIcingField()
  
{
  
  if (icing == NULL) {
    icing = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  
  for (int isig = 0; isig < nSigma; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {
	double pressure = pres[isig][ilat][ilon];
	double tempk = tk[isig][ilat][ilon];
	double clw = cloud[isig][ilat][ilon];
	double dens = (pressure * 100.0) / (RPRIME * tempk);
	double g_per_m3 = clw * dens;
	double severity;
	if (g_per_m3 < TRACE_ICING_THRESH) {
	  severity = 1.0;
	} else if (g_per_m3 < LIGHT_ICING_THRESH) {
	  severity = 2.0;
	} else if (g_per_m3 < MOD_ICING_THRESH) {
	  severity = 3.0;
	} else if (g_per_m3 < SEVERE_ICING_THRESH) {
	  severity = 4.0;
	} else {
	  severity = 5.0;
	}
	if (tempk > 273.15) {
	  severity = 0.0;
	} else if (tempk > 263.15) {
	  severity++;
	}

	icing[isig][ilat][ilon] = severity;

      } // ilon
    } // ilat
  } // isig

}
   
///////////////////////
// _loadTurbField()
//
// Load turb severity field from Richardson's Number
//

#define LIGHT_TURB_THRESH  1.0
#define MOD_TURB_THRESH    0.5
#define SEVERE_TURB_THRESH 0.0

#define RPRIME 287.04
#define CP 1004.0
#define KK 0.2857142857142
#define GG 9.80665

void InputFile::_loadTurbField()
  
{
  
  if (turb == NULL) {
    turb = (fl32 ***) umalloc3(nSigma, nLat, nLon, sizeof(fl32));
  }
  memset(**turb, 0, nSigma * nLat * nLon * sizeof(fl32));
  
  for (int isig = 1; isig < nSigma - 1; isig++) {
    for (int ilat = 0; ilat < nLat; ilat++) {
      for (int ilon = 0; ilon < nLon; ilon++) {

	// compute the change in sigma

	double delta_sigma = _halfSigma[isig+1] - _halfSigma[isig-1];
	
	// compute dsigma by dz
	
	double pres_pa = pres[isig][ilat][ilon] * 100.0 - pp[isig][ilat][ilon];
	
	double ref_dens =
	  pres_pa / (RPRIME * (_tso + _tlp * log(pres_pa / 1000.0)));
	
	double dsigma_dz = - (ref_dens * GG) / (pstar[ilat][ilon] * 1000.0);

	// compute the shear

	double u_below = uu[isig-1][ilat][ilon];
	double v_below = vv[isig-1][ilat][ilon];
	double u_above = uu[isig+1][ilat][ilon];
	double v_above = vv[isig+1][ilat][ilon];

	double du = u_above - u_below;
	double dv = v_above - v_below;

	double dv_dsigma = sqrt(du * du + dv * dv) / delta_sigma;
	double dv_dz = dv_dsigma * dsigma_dz;

	// compute N-squared
	
	double theta =
	  tk[isig][ilat][ilon] * pow((1000.0 / pres[isig][ilat][ilon]), KK);
	double theta_above =
	  tk[isig+1][ilat][ilon] * pow((1000.0 / pres[isig+1][ilat][ilon]), KK);
	double theta_below =
	  tk[isig-1][ilat][ilon] * pow((1000.0 / pres[isig-1][ilat][ilon]), KK);
	double delta_theta = theta_above - theta_below;

 	double n2 = ((GG / theta) * (delta_theta / delta_sigma)) * dsigma_dz;

	// compute richardson's number

	double ri;
	if (dv_dz == 0.0) {
	  ri = 10000.0;
	} else {
	  ri = n2 / (dv_dz * dv_dz);
	}

#ifdef NOTNOW
 	if (ri < 1.1) {
	  fprintf(stderr, "isig, ilat, ilon: %d, %d %d\n", isig, ilat, ilon);
	  fprintf(stderr, "delta_sigma, pres_pa, ref_dens, dsigma_dz: "
		  "%g, %g, %g, %g\n",
		  delta_sigma, pres_pa, ref_dens, dsigma_dz);
	  fprintf(stderr, "u_below, v_below, u_above, v_above: "
		  "%g, %g, %g, %g\n",
		  u_below, v_below, u_above, v_above);
	  fprintf(stderr, "du, dv, dv_sigma, dv_dz: %g, %g, %g, %g\n",
		  du, dv, dv_dsigma, dv_dz);
	  fprintf(stderr, "theta, theta_above, theta_below, delta_theta: "
		  "%g, %g, %g, %g\n",
		  theta, theta_above, theta_below, delta_theta);
	  fprintf(stderr, "--------->> n2, ri: %g, %g\n", n2, ri);
	}
#endif

	double severity;

	if (ri > LIGHT_TURB_THRESH) {
	  severity = 1.0;
	} else if (ri > MOD_TURB_THRESH) {
	  severity = 2.0;
	} else if (ri > SEVERE_TURB_THRESH) {
	  severity = 3.0;
	} else {
	  severity = 4.0;
	}

	turb[isig][ilat][ilon] = severity;

      } // ilon
    } // ilat
  } // isig

}

///////////////////////
// _loadFreezeField()
//
// Load the freezing level field
//
// Freezing level is defined as the lowest occurrence of freezing
// temperature.
//

void InputFile::_loadFreezeField()
  
{
  
  if (freeze == NULL) {
    freeze = (fl32 **) umalloc2(nLat, nLon, sizeof(fl32));
  }
  
  for (int ilat = 0; ilat < nLat; ilat++) {

    for (int ilon = 0; ilon < nLon; ilon++) {

      freeze[ilat][ilon] = 0.0;
      
      for (int isig = 0; isig < nSigma - 1; isig++) {
	
	double t1 = tc[isig][ilat][ilon];
	double t2 = tc[isig+1][ilat][ilon];
	
	if (t1 >= 0.0 && t2 <= 0.0) {
	  
	  double fraction = t1 / (t1 - t2);
	  
	  double p1 = pres[isig][ilat][ilon];
	  double p2 = pres[isig+1][ilat][ilon];
	  
	  double pressure = p1 + fraction * (p2 - p1);
	  
	  double flight_level = _flevel->pres2level(pressure);
	  
	  freeze[ilat][ilon] = flight_level;

	  break;
	  
	} // if (t1 >= 0.0 && t2 <= 0.0) {

      } // isig

    } // ilon

  } // ilat
  
}
   
