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
//
// $Id: GribField.cc,v 1.11 2016/03/07 01:23:10 dixon Exp $
//
//////////////////////////////////////////////

#include <iostream>
#include <cstring>

#include <grib/GribVertType.hh>

#include "GribField.hh"
using namespace std;

GribField::GribField(const bool debug_flag) :
  _debug(debug_flag),
  _parameterId(0),
  _levelId(0),
  _projection(0)
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
}

void
GribField::init( const int& pid, const int& lid, const time_t& gt, 
		 const int& ft, const int& vlt, const int& uc,
		 const float& u_rng, const float& l_rng, const string& name, 
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
  _upperLimit = u_rng;
  _lowerLimit = l_rng;
  _projection = new Pjg(*proj);
  _data = 0;
}


void
GribField::addPlane(const int& level, fl32 *new_data)
{

  int localLevel = level;
  if((_levelId == 1) && (level > 1)) {
    cerr << "WARNING: Vertical level type is surface, but level greater than 1." << endl;
    cerr << "         Resetting level to 1." << endl;
    localLevel = 1;
  }

  map< int, fl32*>::iterator iter;

  if((iter = _planeData.find(localLevel)) == _planeData.end()) {

    int npts = _projection->getNx()*_projection->getNy();  
    fl32*  level_data = new fl32[npts];

    //
    // copy the data
    //
    memcpy( (void*) level_data, new_data, sizeof(fl32)*npts );

    _planeData[localLevel] = level_data;

  }

}

void
GribField::assemble()
{

  //
  // detemine nz, dz and minz
  //
  // assume the ordering of the _planeData is arbitrary
  //

  //
  // WARNING:: bad hack to handle pressure grid files.
  //      
  // We want to represent the atmosphere, so the pressure should
  // be at a maximum at the surface
  //
  if(_levelId == GribVertType::ISOBARIC) {
    float dz;
    float lastDz = -1.0;
    bool dzConstant = true;
    int lastLevel = 0;
    float minZ = -100000.0;
    //  map< int, fl32*>::iterator iter;
    map< int, fl32*>::reverse_iterator iter;
    for(iter = _planeData.rbegin(); iter != _planeData.rend(); iter++) {
      if((*iter).first > minZ) {
	minZ  = (*iter).first;
      }

      dz = (*iter).first - lastLevel;
      lastLevel = (*iter).first;

      if((dz != lastDz) && (iter != _planeData.rbegin())) {
	dzConstant = false;
      }

      _levels.push_back((*iter).first);
    }

    _projection->setGridDims(_projection->getNx(), _projection->getNy(), _planeData.size());
    _projection->setGridDeltas(_projection->getDx(), _projection->getDy(), dz);
    _projection->setGridMins(_projection->getMinx(), _projection->getMiny(), minZ);

    if(!_data) {
      int npts = _projection->getNx()*_projection->getNy()*_projection->getNz();
      _data = new fl32[npts];
    }

    for(iter = _planeData.rbegin(); iter != _planeData.rend(); iter++) {
      int npts = _projection->getNx()*_projection->getNy();
      int level = (int)(((*iter).first- minZ)/dz);
      long offset = level*npts;
      fl32 *dataPtr = _data + offset;
      memcpy( (void*) dataPtr, (*iter).second, sizeof(fl32)*npts);

      //
      // cleanup _planeData as we go
      //
      delete [] (*iter).second;
    }
	
  }
  else {
    float dz;
    float lastDz = -1.0;
    bool dzConstant = true;
    int lastLevel = 0;
    float minZ = 100000.0;
    map< int, fl32*>::iterator iter;
    for(iter = _planeData.begin(); iter != _planeData.end(); iter++) {
      if((*iter).first < minZ) {
	minZ  = (*iter).first;
      }

      dz = (*iter).first - lastLevel;
      lastLevel = (*iter).first;

      if((dz != lastDz) && (iter != _planeData.begin())) {
	dzConstant = false;
      }

      _levels.push_back((*iter).first);
    }

    _projection->setGridDims(_projection->getNx(), _projection->getNy(), _planeData.size());
    _projection->setGridDeltas(_projection->getDx(), _projection->getDy(), dz);
    _projection->setGridMins(_projection->getMinx(), _projection->getMiny(), minZ);

    if(!_data) {
      int npts = _projection->getNx()*_projection->getNy()*_projection->getNz();
      _data = new fl32[npts];
    }

    for(iter = _planeData.begin(); iter != _planeData.end(); iter++) {
      int npts = _projection->getNx()*_projection->getNy();
      int level = (int)(((*iter).first - minZ)/dz);
      long offset = level*npts;
      fl32 *dataPtr = _data + offset;
      memcpy( (void*) dataPtr, (*iter).second, sizeof(fl32)*npts);

      //
      // cleanup _planeData as we go
      //
      delete [] (*iter).second;
    }
  }
	
  _planeData.erase(_planeData.begin(), _planeData.end());

}

void
GribField::print() {
  cout << "  param ID: " << _parameterId 
       << "  level ID: " << _levelId << endl << "   Long Name: " << _longName 
       << "   Name: " << _name << "   Units: " << _units << endl << flush;
  _projection->print(cout);
}


 
