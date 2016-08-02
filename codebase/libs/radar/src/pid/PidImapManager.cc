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
// PidImapManager.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2008
//
///////////////////////////////////////////////////////////////
//
// Manages a series of interest maps for different Dbz ranges
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cmath>
#include <radar/PidImapManager.hh>
using namespace std;

// Constructor

PidImapManager::PidImapManager(const string particleLabel,
			       const string particleDescr,
			       const string &field,
			       double weight,
			       double missingVal) :
  _particleLabel(particleLabel),
  _particleDescr(particleDescr),
  _field(field),
  _weight(weight),
  _missingDouble(missingVal)

{
  
  for (int ii = 0; ii < _nLut; ii++) {
    _mapLut[ii] = NULL;
  }

}

// destructor

PidImapManager::~PidImapManager()
  
{

  for (int ii = 0; ii < (int) _maps.size(); ii++) {
    delete _maps[ii];
  }
  _maps.clear();

}

//////////////////////
// add an interest map

void PidImapManager::addInterestMap(double minDbz,
				    double maxDbz,
				    const vector<PidInterestMap::ImPoint> &map)
  
{

  string mapLabel = _field;
  mapLabel += ".";
  mapLabel += _particleLabel;

  PidInterestMap *imap = new
    PidInterestMap(mapLabel, minDbz, maxDbz, map, _weight, _missingDouble);
  _maps.push_back(imap);

  // clear lookup table
  
  for (int ii = 0; ii < _nLut; ii++) {
    _mapLut[ii] = NULL;
  }

  // recompute lookup table for maps

  for (int ii = 0; ii < (int) _maps.size(); ii++) {

    double minDbz = _maps[ii]->getMinDbz();
    double maxDbz = _maps[ii]->getMaxDbz();

    int minIndex = getIndex(minDbz);
    int maxIndex = getIndex(maxDbz);
    
    for (int jj = minIndex; jj <= maxIndex; jj++) {
      _mapLut[jj] = _maps[ii];
    }
    
  }

  // find the start and end active indexes
  
  int startLut = _nLut;
  for (int ii = 0; ii < _nLut; ii++) {
    if (_mapLut[ii] != NULL) {
      startLut = ii;
      break;
    }
  } // ii
  if (startLut == (_nLut)) {
    // no entries
    return;
  }

  int endLut = 0;
  for (int ii = _nLut - 1; ii >= 0; ii--) {
    if (_mapLut[ii] != NULL) {
      endLut = ii;
      break;
    }
  } // ii

  // fill in the gaps, so that all valid reflectivity values have an interest map
  
  while (true) {
    
    // find start of gap
    
    int startGap = endLut;
    for (int ii = startLut + 1; ii < endLut; ii++) {
      if (_mapLut[ii] == NULL) {
	startGap = ii;
	break;
      }
    } // ii
    if (startGap == endLut) {
      // no gaps - done
      break;
    }
    
    // gap exists - find end
    
    int endGap = startGap;
    for (int ii = startGap + 1; ii < endLut; ii++) {
      if (_mapLut[ii] != NULL) {
	endGap = ii;
	break;
      }
    } // ii

    // fill gap

    for (int ii = startGap; ii < endGap; ii++) {
      _mapLut[ii] = _mapLut[startGap - 1];
    } // ii

  } // while

}

///////////////////////////////////////////////////////////
// compute interest from value

double PidImapManager::getInterest(double dbz, double val) const

{

  int index = getIndex(dbz);
  const PidInterestMap *map = _mapLut[index];
  if (map == NULL) {
    return 0;
  }
  return map->getInterest(val);

}

///////////////////////////////////////////////////////////
// compute weighted interest from value

void PidImapManager::getWeightedInterest(double dbz,
					 double val,
					 double &interest,
					 double &wt) const

{
  
  int index = getIndex(dbz);
  const PidInterestMap *map = _mapLut[index];
  if (map == NULL) {
    interest = 0;
    wt = 0;
    return;
  }

  map->getWeightedInterest(val, interest, wt);
  
}

///////////////////////////////////////////////////////////
// print

void PidImapManager::print(ostream &out)
  
{

  out << "------>> Interest map manager <<------" << endl;
  out << "  Particle label: " << _particleLabel << endl;
  out << "  Particle descr: " << _particleDescr << endl;
  out << "  Field: " << _field << endl;
  out << "  Weight: " << _weight << endl;
  for (int ii = 0; ii < (int) _maps.size(); ii++) {
    _maps[ii]->print(out);
  }
  out << "-----------------------------------------" << endl;

}  
