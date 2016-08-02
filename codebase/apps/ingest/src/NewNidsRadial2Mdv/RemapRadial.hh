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
// RemapRadial.hh
//
// Remap the radial data onto a cart grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Paddy McCarthy created abstract class, added RemapRast
//
/////////////////////////////////////////////////////////////

#ifndef RemapRadial_HH
#define RemapRadial_HH

#include "Remap.hh"
using namespace std;

#define NAZPOS 3600

class RemapRadial : public Remap {
  
public:

  // comstructor

  RemapRadial(const string &progName,
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

  RemapRadial(const string &progName,
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

  virtual ~RemapRadial();

  // process a file

  virtual int processFile(const string &filePath);

protected:
  
  virtual void _doRemapping(ui08 *radials);

private:

  double _elevAngle;
  int _nGates;
  double _gateSpacing;
  double _startRange;

  si16 *_lutAz;
  si16 *_lutRng;

  int _azIndex[NAZPOS];
  
  void _computeGridLookup(double elevAngle, int nGates,
			  double gateSpacing, double startRange);
  
  int _uncompressRadials(const NIDS_radial_header_t &rhdr, int nGates,
			 ui08 *rptr, ui08 *radials);

};

#endif

