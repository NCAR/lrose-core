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
///////////////////////////////////////////////
// GribField
//////////////////////////////////////////////

#include <iostream>
#include <cstring>
#include <cassert>

#include "GribField.hh"
#include "Params.hh"

using namespace std;

GribField::GribField(const bool debug_flag) :
  _debug(debug_flag),
  _parameterId(0),
  _levelId(0),
  _verticalLevelType(0),
  _forecastTime(0),
  _unitConversion(0),
  _projection(0),
  _data(NULL),
  _dataSize(0),
  _dzConstant(false)
{

}

GribField::~GribField() 
{

  delete _projection;

  map< int, fl32*>::iterator iter; 
  for(iter = _planeData.begin(); iter != _planeData.end(); iter++) {
    delete [] (*iter).second;
  }
  _planeData.erase(_planeData.begin(), _planeData.end());

  delete [] _data;
  _levels.clear();
}

void
GribField::init( const int& pid, const int& lid, const time_t& gt, 
		 const int& ft, const int& vlt, const int& uc, const string& name, 
		 const string& long_name, const string& units, const Pjg *proj )
{
  _parameterId = pid;
  _levelId = lid;
  _verticalLevelType = vlt;
  _forecastTime = ft;
  _generateTime = gt;
  _unitConversion = uc;
  _longName = long_name;
  _name = name;
  _units = units;
  _projection = new Pjg(*proj);

  delete [] _data;
  _data = NULL;
  _dataSize = 0;
}


void GribField::addPlane(const int& level, fl32 *new_data)
// The map<> container is a Sorted Associative Container,
// which guarantees that _planeData is sorted by level.
{
  map< int, fl32*>::iterator iter;
  if((iter = _planeData.find(level)) == _planeData.end()) {

    int npts = _projection->getNx()*_projection->getNy();
    fl32*  level_data = new fl32[npts];

    // copy the new_data into level_data
    memcpy( (void*) level_data, new_data, sizeof(fl32)*npts );

    _planeData[level] = level_data;	// adds a new entry to _planeData

  } else {
    if(_debug) {
     cerr << "WARNING: Did not add duplicate level " << level
      << " for parameter "  << _parameterId
      << " level id " << _levelId << endl;
    }
  }

}

void GribField::assemble()
{
  // Detemine nz, dz and minz.
  int sign = 1;
  if (_levelId == Params::ISOBARIC)
  {
    // This is a pressure file.
    // We want to represent the atmosphere, so the pressure should
    // be at a maximum at the surface.  minZ represents the minimum
    // altitude, but it's calculated by finding the maximum pressure.
    // Remember that altitude and pressure are inversely proportional.
    // We set the sign to -1 here to reverse comparisons in the code below.
    sign = -1;
  }

  float dz = 0;
  float lastDz = -1.0;
  _dzConstant = true;
  int lastLevel = 0;
  float minZ = sign * 100000.0;	// the altitude/pressure way out in space
 
  // loop through the planes
  int planeIndex = 0;
  map< int, fl32*>::iterator iter;
  for(iter = _planeData.begin(); iter != _planeData.end(); iter++) {
    if((*iter).first < sign * minZ) {
      minZ  = (*iter).first;
    }

    dz = (*iter).first - lastLevel;
    lastLevel = (*iter).first;

    if(dz != lastDz && planeIndex > 1) {
      _dzConstant = false;
    }
    lastDz = dz;

    _levels.push_back((*iter).first);
    planeIndex++;
  }

  // At this point minZ might be something like 1000 (millibars at zero elevation).

  _projection->setGridDims(_projection->getNx(), _projection->getNy(), _planeData.size());
  _projection->setGridDeltas(_projection->getDx(), _projection->getDy(), dz);

  // The Pjg Projection class appears to be broken. 
  // Somewhere deep in the code, a copy operators is broken
  // I spent over 2 hours trying to track down a a simple error and was unable to
  // locate any code that can possibly cause this problem. The Pjg class uses
  // intrinsic constructors (there's no code) where a new Pjg is constructed
  // from another. I believe the error occurs here.  - F. Hage  May 2007
  double miny = _projection->getMiny();
  if(miny == 90.0) miny = -90.0;
  _projection->setGridMins(_projection->getMinx(),miny , minZ);

  int dataSizeNeeded = _projection->getNx()*_projection->getNy()*_projection->getNz();
  if(_dataSize < dataSizeNeeded) {
    delete [] _data;
    _data = new fl32[dataSizeNeeded];
    _dataSize = dataSizeNeeded;
  }

  // copy into _data
  for(iter = _planeData.begin(); iter != _planeData.end(); iter++) {
    int npts = _projection->getNx()*_projection->getNy();
    int levelIndex = findLevelIndex((*iter).first);
    assert(levelIndex >= 0);
    long offset = levelIndex*npts;
    assert(offset < _dataSize);
    fl32 *dataPtr = _data + offset;
    memcpy( (void*) dataPtr, (*iter).second, sizeof(fl32)*npts);

    // cleanup _planeData as we go
    delete [] (*iter).second;
  }

  _planeData.erase(_planeData.begin(), _planeData.end());
}


int GribField::findLevelIndex(int level)
{
	// search for the supplied value
	for (int index = 0; index < (int) _levels.size(); index++) {
		if (_levels[index] == level) {
			return index;
		}
	}

	return -1;
}


void GribField::print() {
  cout << "  param ID: " << _parameterId 
       << "  level ID: " << _levelId << endl << "   Long Name: " << _longName 
       << "   Name: " << _name << "   Units: " << _units << endl << flush;
  _projection->print(cout);
}


 
