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
// Remap.hh
//
// Remap the radial data onto a cart grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Paddy McCarthy made abstract, and put remapping in RemapRadial, RemapRast.
//
// March 1999
//
/////////////////////////////////////////////////////////////

#ifndef Remap_HH
#define Remap_HH

#include <vector>
#include <string>
#include <iostream>
#include <didss/DsInputPath.hh>
#include <dataport/port_types.h>
#include <rapformats/nids_file.h>
#include "Params.hh"
#include "OutputMdv.hh"
using namespace std;

#define NAZPOS 3600

class Remap {
  
public:

  // comstructor

  Remap(const string &progName,
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
        const long processingDelay);

  Remap(const string &progName,
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
        const long processingDelay);

  // destructor

  virtual ~Remap();

  // process a file

  int initInputPaths();
  int checkNextFile();
  virtual int processFile(const string &filePath) = 0;

  // accessors

  const string & getRadarName() const { return _radarName; }

protected:
  
  DsInputPath * _input;

  const string &_progName;
  bool _isArchive;
  vector <string> _inputPaths;
  string _inputDir;
  string _radarName;
  string _outputDir;

  int _maxDataAge;
  bool _useLDataInfo, _getLatestFileOnly;

  bool _calculateScaleAndBias;
  float _specifiedScale;
  float _specifiedBias;

  bool _debug, _verbose;
  int _nx, _ny;
  float _dx, _dy, _minx, _miny;

  string _dataFieldNameLong, _dataFieldName, _dataUnits, _dataTransform;
  int _dataFieldCode;

  long _processingDelay;

  si32 _nptsGrid;

  int _outputVal[16];
  double _scale, _bias;

  OutputMdv _out;

  time_t _lastProcessedTime;


  virtual void _doRemapping(ui08 *radials) = 0;

  void _computeScaleAndBias(NIDS_header_t &nhdr);

private:

  // Private default constructor with no impl -- do not use.
  Remap();

};

#endif

