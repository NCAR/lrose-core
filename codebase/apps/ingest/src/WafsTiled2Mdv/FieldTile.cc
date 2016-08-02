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
// FieldTile.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2004
//
// Copied from Grib2Mdv
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cstring>

#include "FieldTile.hh"
#include "Params.hh"

using namespace std;

FieldTile::FieldTile(const string &prog_name,
		     const Params &params,
		     int field_id,
		     int vlevel_type,
		     time_t generate_time, 
		     int forecast_time,
		     int units_conversion,
		     const string& name, 
		     const string& long_name,
		     const string& units,
		     const Pjg *proj) :
  _progName(prog_name),
  _params(params),
  _fieldId(field_id),
  _vlevelType((grib_vlevel_type_t) vlevel_type),
  _generateTime(generate_time),
  _forecastTime(forecast_time),
  _unitConversion(units_conversion),
  _name(name),
  _longName(long_name),
  _units(units),
  _projection(NULL)

{
  _projection = new Pjg(*proj);
}


FieldTile::~FieldTile() 
{

  if (_projection) {
    delete _projection;
  }

  map<int, fl32*>::iterator iter;
  for(iter = _planeData.begin(); iter != _planeData.end(); iter++) {
    delete [] (*iter).second;
  }

}

//////////////////////////////////////////////////////////
// Add a plane.
//
// The map<> container is a Sorted Associative Container,
// which guarantees that _planeData is sorted by level.

void FieldTile::addPlane(const int& level, const fl32 *new_data)
{
  
  map<int, fl32*>::iterator iter;
  if((iter = _planeData.find(level)) == _planeData.end()) {
    
    // copy the new_data into level_data

    int npts = _projection->getNx()*_projection->getNy();
    fl32* level_data = new fl32[npts];
    memcpy(level_data, new_data, sizeof(fl32)*npts);
    
    // add new entry to _planeData

    _planeData[level] = level_data;
    
  } else {
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING: Did not add duplicate level " << level
	   << " for field "  << _name
	   << ", id: " << _fieldId << endl;
    }

  } // iter

}

void FieldTile::assemble()
{
  
  // loop through the planes
  
  fl32 minZ = _planeData.begin()->first;
  fl32 prevZ = minZ;
  fl32 dZ = 1.0;
  map< int, fl32*>::iterator iter;
  for(iter = _planeData.begin(); iter != _planeData.end(); iter++) {
    fl32 zz = iter->first;
    dZ = zz - prevZ;
    prevZ = zz;
    _levels.push_back(iter->first);
    _data.push_back(iter->second);
  }
  
  _projection->setGridDims(_projection->getNx(),
			   _projection->getNy(),
			   _planeData.size());
  
  _projection->setGridDeltas(_projection->getDx(),
			     _projection->getDy(),
			     dZ);
  
  _projection->setGridMins(_projection->getMinx(),
			   _projection->getMiny(),
			   minZ);
  
}


int FieldTile::findLevelIndex(int level)
{
  // search for the supplied value
  for (int index = 0; index < (int) _levels.size(); index++) {
    if (_levels[index] == level) {
      return index;
    }
  }
  return -1;
}


void FieldTile::print(ostream &out) {
  
  out << "========== FieldTile ==========" << endl;
  out << endl;
  out << "  field ID: " << _fieldId << endl;
  out << "  Long Name: " << _longName << endl;
  out << "  Name: " << _name << endl;
  out << "  Units: " << _units << endl;
  out << endl;
  out << "  Levels:";
  for (size_t ii = 0; ii < _levels.size(); ii++) {
    out << " " << _levels[ii];
  }
  out << endl;
  out << endl;
  _projection->print(out);
  out << endl;

}


 
