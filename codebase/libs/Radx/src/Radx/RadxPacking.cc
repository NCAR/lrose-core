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
// RadxPacking.cc
//
// Range geometry for Radx classes
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxPacking.hh>
using namespace std;

//////////////
// Constructor

RadxPacking::RadxPacking()
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxPacking::RadxPacking(const RadxPacking &rhs)
     
{
  _copy(rhs);
}

/////////////
// destructor

RadxPacking::~RadxPacking()

{
}

/////////////////////////////
// Assignment
//

RadxPacking &RadxPacking::operator=(const RadxPacking &rhs)
  

{
  return _copy(rhs);
}

/////////////////////////////////////////////////////////
// initialize data members

void RadxPacking::_init()
  
{
  clearPacking();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxPacking::clearPacking()
  
{
  _nPoints = 0;
  _maxNGates = 0;
  _nGatesVary = false;
  _rayNGates.clear();
  _rayStartIndex.clear();
}

/////////////////////////////////////////////////////////
// copy the packing

void RadxPacking::copyPacking(const RadxPacking &rhs)
  
{
  _copy(rhs);
}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxPacking &RadxPacking::_copy(const RadxPacking &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }

  _nPoints = rhs._nPoints;
  _maxNGates = rhs._maxNGates;
  _maxNGates = rhs._maxNGates;
  _rayNGates = rhs._rayNGates;
  _rayStartIndex = rhs._rayStartIndex;

  return *this;
  
}

/////////////////////////////////////////////////////////
// add a ray with nGates

void RadxPacking::addToPacking(size_t nGates)
  
{
  if (_maxNGates != nGates) {
    if (_rayNGates.size() > 0) {
      _nGatesVary = true;
    }
  }
  if (_maxNGates < nGates) {
    _maxNGates = nGates;
  }
  _rayNGates.push_back(nGates);
  _rayStartIndex.push_back(_nPoints);
  _nPoints += nGates;
}

/////////////////////////////////////////////////////////
// set geom for a vector of rays

void RadxPacking::setPacking(const vector<size_t> &rayNGates)
  
{
  clearPacking();
  _maxNGates = 0;
  _nGatesVary = false;
  for (size_t ii = 0; ii < rayNGates.size(); ii++) {
    size_t nGates = rayNGates[ii];
    if (_maxNGates < nGates) {
      _maxNGates = nGates;
      if (_rayNGates.size() > 1) {
        _nGatesVary = true;
      }
    }
    _rayNGates.push_back(nGates);
    _rayStartIndex.push_back(_nPoints);
    _nPoints += nGates;
  }
}

/////////////////////////////////////////////////////////
// print packing

void RadxPacking::printSummary(ostream &out) const
  
{

  out << "  RadxPacking: data packing" << endl;
  out << "    nRays: " << getNRays() << endl;
  out << "    nPoints: " << _nPoints << endl;
  out << "    maxNGates: " << _maxNGates << endl;
  out << "    nGatesVary: " << (_nGatesVary? "Y" : "N") << endl;

}

void RadxPacking::printFull(ostream &out) const
  
{

  printSummary(out);
  out << "    rayNGates(ray): ";
  for (int ii = 0; ii < (int) _rayNGates.size(); ii++) {
    out << _rayNGates[ii];
    if (ii != (int) _rayNGates.size() - 1) {
      out << ", ";
    }
  }
  out << endl;
  out << "    rayStartIndex(ray): ";
  for (int ii = 0; ii < (int) _rayStartIndex.size(); ii++) {
    out << _rayStartIndex[ii];
    if (ii != (int) _rayStartIndex.size() - 1) {
      out << ", ";
    }
  }
  out << endl;

}

