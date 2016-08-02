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
// PPIField.cc
//
// PPIField object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#include "PPIField.hh"
#include "sRadl.hh"
#include "RapicRay.hh"
#include "ScanParams.hh"
#include <Radx/RadxTime.hh>
#include <cmath>
using namespace std;

// Constructor

PPIField::PPIField()

{

  _time = 0;
  _maxGates = 0;
  _rangeRes = 0;
  _startRange = 0;
  _scale = 1.0;
  _bias = 0.0;

}

// destructor

PPIField::~PPIField()

{
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    delete _rays[ii];
  }
}

// set field name
// also sets the scale and bias as a side effect

void PPIField::setName(const string &name,
                       const ScanParams &sParams)
{
  _name = name;
  if (_name == "Refl") {
    _scale = 0.5;
    _bias = -32.0;
    _units = "dBZ";
  } else if (_name == "Vel") {
    _scale = sParams.nyquist / 127.0;
    _bias = (-1.0 * sParams.nyquist * 128.0) / 127.0;
    _units = "m/s";
  } else if (_name == "Width") {
    _scale = sParams.nyquist / 255.0;
    _bias = 0;
    _units = "m/s";
  } else if (_name == "ZDR") {
    _scale = 0.1;
    _bias = (-12.7 * 128.0) / 127.0;
    _units = "dB";
  } else if (_name == "PHIDP") {
    _scale = 180.0 / 127.0;
    _bias = (-1.0 * 180 * 128.0) / 127.0;
    _units = "deg";
  } else if (_name == "RHOHV") {
    _scale = 1.0 / 255.0;
    _bias = 0.1;
    _units = "";
  } else {
    _scale = 1.0;
    _bias = 0.0;
    _units = "";
  }
}

// add a ray

void PPIField::addRay(const sRadl *radial, const ScanParams &sParams,
                      bool isBinary, double target_elev)

{
  RapicRay *ray = new RapicRay(radial, sParams, isBinary, target_elev);
  _rays.push_back(ray);
  if (_maxGates < radial->data_size) {
    _maxGates = radial->data_size;
  }
}

// print

void PPIField::print(ostream &out)

{
  out << "Name: " << _name << endl;
  out << "Time: " << RadxTime::str(_time) << endl;
  out << "nRays: " << _rays.size() << endl;
  out << "maxGates: " << _maxGates << endl;
  out << "startRange: " << _startRange << endl;
  out << "rangeRes: " << _rangeRes << endl;
 for (size_t i = 0; i < _rays.size(); i++) {
    out << i << ": ";
    _rays[i]->print(out);
  }
}

// print full

void PPIField::printFull(ostream &out)

{
  out << "PPIField::printFull" << endl;
  out << "Time: " << RadxTime::str(_time) << endl;
  out << "nRays: " << _rays.size() << endl;
  out << "maxGates: " << _maxGates << endl;
  out << "startRange: " << _startRange << endl;
  out << "rangeRes: " << _rangeRes << endl;
  for (size_t i = 0; i < _rays.size(); i++) {
    _rays[i]->printFull(out);
  }
}


// Remap refl data to same range resolution as vel data for Beijing data
// This assumes that original refl data has startRange = 0, rangeRes=1000 
// and vel data has startRange = 375, rangeRes = 250m. 
// This requires further work to make it more general. 
// (rjp 13 Sep 2006)
 
void PPIField::convertNexradResolution(int maxGatesLimit)
{

  RapicRay *new_ray = NULL;
  int numgates; 
  double index;
  
  //Set number of gates - assumes that rangeRes(refl) = 4*rangeRes(vel)
  //Need to set maxGates for refl to same as for vel. 
  int ngates = 4*_maxGates;
  if (ngates > maxGatesLimit) {
    ngates = maxGatesLimit;
  }

  vector<RapicRay *> newRays;

  for (size_t ii = 0; ii < _rays.size(); ii++) {

    //Create new_ray with ngates and val=0
    new_ray = new RapicRay(_rays[ii],ngates);
    
    //Remap old ray to new ray with new rangeRes.  Note the 
    //number of gates in each old ray will vary. 
    numgates = 4*((_rays[ii]->nGates))-1; 
    if (numgates > maxGatesLimit) {
      numgates = maxGatesLimit; 
    }

    for (int jj = 0; jj < numgates; jj++) {
      index = floor((jj + 2.0)/4.0);
      new_ray->vals[jj] = _rays[ii]->vals[(int)index]; 
    }

    newRays.push_back(new_ray);

    // if (ii > 90 && ii < 100) {
    //   cerr << "NUMGATES: " << numgates << endl;
    //   cerr << "old ray: el: " << rays[ii]->elevation  
    //        << ", az: " << rays[ii]->azimuth 
    //        << ", ngates: " << rays[ii]->nGates << endl;
    //   cerr << "new ray: az: " << new_ray->azimuth 
    //        << ", ngates: " << new_ray->nGates << endl;
    // }

  } // ii

  // free up old rays
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    delete _rays[ii];
  }

  // copy over vector

  _rays = newRays;

  // Set scan params explicitly.

  _maxGates = ngates;
  _rangeRes = 250;
  _startRange = 375; 

}

