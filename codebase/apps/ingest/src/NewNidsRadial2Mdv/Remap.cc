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
// Remap.cc
//
// Remap the radial data onto a cart grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
//////////////////////////////////////////////////////////

#include "Remap.hh"
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
using namespace std;

// constructor

Remap::Remap(const string &progName,
	     const string &inputDir,
	     const string &radarName,
	     const string &outputDir,
             const int maxDataAge,
             const bool useLDataInfo,
             const bool getLatestFileOnly,
             const bool computeScaleAndBias,
             const float specifiedScale,
             const float specifiedBias,
             const bool debug, const bool verbose,
             const int nx, const int ny, 
             const float dx, const float dy,
             const float minx, const float miny,
             const string &dataFieldNameLong,
             const string &dataFieldName,
             const string &dataUnits,
             const string &dataTransform,
             const int dataFieldCode,
             const long processingDelay) :
  _progName(progName),
  _isArchive(false),
  _inputDir(inputDir),
  _radarName(radarName),
  _outputDir(outputDir),
  _maxDataAge(maxDataAge),
  _useLDataInfo(useLDataInfo),
  _getLatestFileOnly(getLatestFileOnly),
  _calculateScaleAndBias(computeScaleAndBias),
  _specifiedScale(specifiedScale),
  _specifiedBias(specifiedBias),
  _debug(debug),
  _verbose(verbose),
  _nx(nx),
  _ny(ny),
  _dx(dx),
  _dy(dy),
  _minx(minx),
  _miny(miny), 
  _dataFieldNameLong(dataFieldNameLong),
  _dataFieldName(dataFieldName),
  _dataUnits(dataUnits),
  _dataTransform(dataTransform),
  _dataFieldCode(dataFieldCode),
  _processingDelay(processingDelay),
  _out(progName),
  _lastProcessedTime(0)

{

  _nptsGrid = _nx * _ny;

}

Remap::Remap(const string &progName,
	     vector<string> inputPaths,
	     const string &radarName,
	     const string &outputDir,
             const bool computeScaleAndBias,
             const float specifiedScale,
             const float specifiedBias,
             const bool debug, const bool verbose,
             const int nx, const int ny, 
             const float dx, const float dy,
             const float minx, const float miny,
             const string &dataFieldNameLong,
             const string &dataFieldName,
             const string &dataUnits,
             const string &dataTransform,
             const int dataFieldCode,
             const long processingDelay) :
  _progName(progName),
  _isArchive(true),
  _inputPaths(inputPaths),
  _radarName(radarName),
  _outputDir(outputDir),
  _calculateScaleAndBias(computeScaleAndBias),
  _specifiedScale(specifiedScale),
  _specifiedBias(specifiedBias),
  _debug(debug),
  _verbose(verbose),
  _nx(nx),
  _ny(ny),
  _dx(dx),
  _dy(dy),
  _minx(minx),
  _miny(miny), 
  _dataFieldNameLong(dataFieldNameLong),
  _dataFieldName(dataFieldName),
  _dataUnits(dataUnits),
  _dataTransform(dataTransform),
  _dataFieldCode(dataFieldCode),
  _processingDelay(processingDelay),
  _out(progName),
  _lastProcessedTime(0)

{

  _nptsGrid = _nx * _ny;

}

// destructor

Remap::~Remap()

{
  delete _input;
}

// Init the input paths, if necessary.

int Remap::initInputPaths()

{
  if (_isArchive) {
    _input = new DsInputPath( (char *) _progName.c_str(), _verbose, _inputPaths);
  }
  else {
    _input = new DsInputPath( (char *) _progName.c_str(),
                              _verbose,
                              (char *) _inputDir.c_str(), 
                              _maxDataAge,
                              NULL,
                              _useLDataInfo,
                              _getLatestFileOnly );
  }

  if (_input == NULL) {
    return -1;
  }
  else {
    return 0;
  }
}

int Remap::checkNextFile()

{
  if (_isArchive) {

    // Process files until they are all done.

    char *file_path;
    while ((file_path = _input->next()) != NULL) {
      if (processFile(file_path)) {
        cerr << "WARNING - could not process file: " << file_path << endl;
      }
    }
  }
  else {

    // Check if a new file is available, and process it if so.

    // Get the ldata handle (this makes a copy, since get() returns const)
    LdataInfo ldata = _input->getLdataInfo();
    ldata.read(-1);
    time_t ldataTime = ldata.getLatestTime();
    if (ldataTime > _lastProcessedTime) {
      string newFile = ldata.getDataDir() + PATH_DELIM + ldata.getRelDataPath();
      cerr << "Got New File: " << newFile << endl;
      if (processFile(newFile)) {
        cerr << "Could not process file: " << newFile << endl;
      }

      _lastProcessedTime = ldataTime;
    }
  }
  
  return (0);
}

//////////////////////////////////////////////////////
// compute scale and bias, and load up outputVal array

void Remap::_computeScaleAndBias(NIDS_header_t &nhdr)

{

  // Straighten out the data thresholds from header

  double levels[16];
  for (int i = 1; i < 16; i++ ) {
    ui08 msbyte, lsbyte;
    msbyte = (nhdr.pd[i+1] & 0xff00) >> 8;
    lsbyte = (nhdr.pd[i+1] & 0x00ff);
    levels[i] = lsbyte;
    if (msbyte & 0x01) {
      levels[i] *= -1.0;
    }
    if (msbyte & 0x10) {
      levels[i] /= 10.0;
    }
  }

  // compute the interval between bins.

  double interval = ((levels[15] - levels[1]) / 14.0);
  
  // compute a scale, bias based on the dynamic range of the data

  if (_calculateScaleAndBias) {
    _scale = interval / 10.0;
    _bias =  levels[1] - interval;
  }
  else {
    _scale = _specifiedScale;
    _bias  = _specifiedBias;
  }

  // compute output byte value for each data level

  _outputVal[0] = 0;
  for(int i = 1; i <= 15; i++) {
    _outputVal[i] = (int) ((levels[i] - _bias) / _scale + 0.5);
  }

}

