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
// ModelTrackData.cc: ModelTrackData handling
//
// Serves out MODEL tracks and entries
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#include "ModelTrackData.hh"
#include <cerrno>
using namespace std;
// Constructor

ModelTrackData::ModelTrackData (const string &prog_name, const Params &params,
				DsInputPath &input) :
  TrackData(prog_name, params, input)

{

  // initialize

  _modelFile = NULL;

  _coord.proj_type = Mdvx::PROJ_FLAT;
  _coord.proj_origin_lat = _params.model_grid.origin_lat;
  _coord.proj_origin_lon = _params.model_grid.origin_lon;
  int ksize = _params.smoothing_kernel_size;
  _coord.minx = _params.model_grid.minx +
    _params.model_grid.dx * (ksize / 2);
  _coord.miny = _params.model_grid.miny +
    _params.model_grid.dy * (ksize / 2);
  _coord.dx = _params.model_grid.dx * ksize;
  _coord.dy = _params.model_grid.dy * ksize;
  _coord.nx = _params.model_grid.nx / ksize;
  _coord.ny = _params.model_grid.ny / ksize;

  // open initial track file
  
  char *model_file_path = _input.next();
  if (model_file_path == NULL) {
    cerr << "ERROR - " << _progName
	 << ":ModelTrackData::ModelTrackData" << endl;
    cerr << "No model track files available" << endl;
    OK = FALSE;
  }
  
  if (_openFile(model_file_path)) {
    cerr << "ERROR - " << _progName
	 << ":ModelTrackData::ModelTrackData" << endl;
    cerr << "Cannot open model track file: " << model_file_path << endl;
    OK = FALSE;
  }
  
  return;

}

// Destructor

ModelTrackData::~ModelTrackData()

{

  _closeFile();

}

//////////////////////////////////////////
// loadNextTrack()
//
// Load up next track.
//
// Sets no_more_tracks when all complex tracks have been read.
//
// Returns 0 on success, -1 on failure.
//
//////////////////////////////////////////

int ModelTrackData::loadNextTrack(int *no_more_tracks_p)

{

  *no_more_tracks_p = FALSE;

  char *model_file_path;

  while (_readTrack()) {
    
    // no tracks left in file - close model data file
    
    _closeFile();
    
    // open new track file
    
    model_file_path = _input.next();
    if (model_file_path == NULL) {
      *no_more_tracks_p = TRUE;
      return (0);
    }
    
    if (_openFile(model_file_path)) {
      cerr << "ERROR - ModelTrackData::loadNextTrack" << endl;
      cerr << "Cannot open track file: " << model_file_path << endl;
      return (-1);
    }

  } // if (_readTrack())

  // set track quantities

  _Amax = _AA / (_Dm * sqrt(M_PI * 2.0));

  scanIntervalSecs = (double) _params.scan_interval;
  durationInScans =
    (int) (_durationSecs / scanIntervalSecs);
  durationInSecs = (int) (durationInScans * scanIntervalSecs);
  startTime = (time_t) _startTime;
  endTime = startTime + durationInSecs;

  // initialize entry num

  _iScan = 0;

  // debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printTrack(stdout);
  }
  
//   fprintf(stdout, "\n");
//   fprintf(stdout, "TRACK: duration %g\n", durationInSecs / 3600.0);
//   fprintf(stdout, "       A        %g\n", _AA);
//   fprintf(stdout, "       Amax     %g\n", _Amax);
//   fprintf(stdout, "\n");

  return (0);

}

//////////////////////////////////////////
// loadNextEntry()
//
// Load up next track entry.
//
// Sets no_more_entries when all entries have been read.
//
// returns 0 on success, -1 on failure 
//
//////////////////////////////////////////

int ModelTrackData::loadNextEntry(int *no_more_entries_p)

{

  *no_more_entries_p = FALSE;

  // check that there is data still to be read

  if (_iScan >= durationInScans) {
    *no_more_entries_p = TRUE;
    return (0);
  }

  // times

  double rel_time_secs = _iScan * scanIntervalSecs;
  double rel_time_hrs = rel_time_secs / 3600.0;
  double offset_time_secs = fabs(_durationSecs / 2.0 - rel_time_secs);
  double offset_time_hrs = offset_time_secs / 3600.0;
  entryTime = startTime + (int) rel_time_secs;
  entryHistoryInScans = _iScan + 1;

  // speed

  dxDt = _u;
  dyDt = _v;

  // position

  centroidX = _startX + rel_time_hrs * dxDt;
  centroidY = _startY + rel_time_hrs * dyDt;

  // compute area from gaussian function of Dm and A

  double dtt = offset_time_hrs / _Dm;
  double dtt2 = dtt * dtt;
  double ati = _AA * _durationHrs;
  double c = ati / (_Dm * sqrt(2.0 * M_PI));
  area = c * exp((-1.0 * dtt2) / 2.0);
  precipArea = area;
  precipFlux = 0.0; // should compute from hist

  // set volume and tops

  volume = area;
  tops = 10.0;

  // ellipse shape

  majorRadius = sqrt((area * _ellipseRatio) / M_PI);
  minorRadius = majorRadius / _ellipseRatio;
  ellipseOrientation = _ellipseOrientation;

  // reflectivity props

  lowDbzThreshold = _dbzThresh;
  dbzHistInterval = _params.dbz_hist_interval;
  
  dbzMax = (lowDbzThreshold + 10.0) +
    (area / _Amax) * (_dbzMax - lowDbzThreshold - 10.0);

  nDbzIntervals = (int) ((dbzMax - lowDbzThreshold) / dbzHistInterval) + 1;

  if (areaHist != NULL) {
    ufree(areaHist);
  }
  areaHist = (double *) umalloc (nDbzIntervals * sizeof(double));

  // dbz area histogram is computed relative to the
  // dbz threshold

  double sum = 0.0;
  for (int i = 0; i < nDbzIntervals; i++) {
    double dbzRel = dbzMax - lowDbzThreshold;
    double b = 4.0 / (M_PI * dbzRel);
    double dbz = (i + 0.5) * dbzHistInterval;
    double diff2 = dbzRel * dbzRel - dbz * dbz;
    if (diff2 >= 0.0) {
      areaHist[i] = dbzHistInterval * (b / dbzRel) * sqrt(diff2);
    } else {
      areaHist[i] = 0.0;
    }
    sum += areaHist[i];
  }

  // adjust to force to 100.0%

  for (int i = 0; i < nDbzIntervals; i++) {
    areaHist[i] = areaHist[i] * 100.0 / sum;
  }
  
  // compute precip flux

  precipFlux = 0.0;
  for (int i = 0; i < nDbzIntervals; i++) {
    double dbz = lowDbzThreshold + (i + 0.5) * dbzHistInterval;
    double z = pow(10.0, dbz / 10.0);
    double precip_rate = pow((z / _params.z_r_coeff),
			     (1.0 / _params.z_r_exponent));
    double darea = precipArea * areaHist[i] / 100.0;
    precipFlux += precip_rate * darea;
  }
  // double precipVolM3 = precipFlux * scanIntervalSecs;
  // double precipDepthMm = 0.0;
  // if (precipArea > 0) {
  //   precipDepthMm = (precipVolM3 / precipArea) / 1000.0;
  // }
  
  // debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _printEntry(stdout);
  }
  
  _iScan++;
  return (0);

}

//////////////////////////////////////////
// _openFiles()
//
// Open model data file for track data
//
// returns 0 on success, -1 on failure 
//
//////////////////////////////////////////

int ModelTrackData::_openFile(char *model_file_path)

{

  // open track properties files
  
  if ((_modelFile = fopen(model_file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - ModelTrackData::_openFile" << endl;
    cerr << "  Cannot open model data file: " << model_file_path << endl;
    cerr << "  " << strerror(errNum);
    return (-1);
  }

  cerr << "ModelTrackData processing file: " << model_file_path << endl;
    
  return (0);

}

//////////////////////////////////////////
// _readTrack()
//
// Read track from model data file
//
// returns 0 on success, -1 on failure 
//
//////////////////////////////////////////

int ModelTrackData::_readTrack()

{

  char line[1024];

  while (!feof(_modelFile)) {
    
    if (fgets(line, 1024, _modelFile) != NULL) {
      
      if (line[0] != '#') {
	
	if (sscanf(line,
		   "%d %lg %d "
		   "%lg %lg %lg "
		   "%lg %lg %lg %lg "
		   "%lg %lg "
		   "%lg %lg %lg %lg "
		   "%lg %lg",
		   &_trackCount, &_startTime, &_nScans,
		   &_durationHrs, &_Dm, &_AA,
		   &_dbzThresh, &_dbzMean, &_dbzMin, &_dbzMax,
		   &_startX, &_startY,
		   &_u, &_v, &_speed, &_dirn,
		   &_ellipseRatio, &_ellipseOrientation) == 18) {
	  _durationSecs = _durationHrs * 3600.0;
	  if (_params.debug >= Params::DEBUG_NORM) {
	    fprintf(stderr, "Processing track num %d...\n", _trackCount);
	  }
	  return (0);
	} else {
	  cerr << "ERROR - ModelTrackData::_readTrack" << endl;
	  cerr << "  Cannot read line from model data file" << endl;
	  cerr << "  Line number: " << line << endl;
	}

      } // if (line[0] != '#') 
	
    }  // if (fgets( ...
    
  } // while

  return (-1);

}

//////////////////////////////////////////
// _closeFile()
//
// Close model data file.
//
//////////////////////////////////////////

void ModelTrackData::_closeFile()
  
{

  fclose(_modelFile);
  
}

