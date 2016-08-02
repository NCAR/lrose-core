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
// AtmosAtten.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Provide estimate of 2-way atmospheric attenuation for
// elev / range
//
////////////////////////////////////////////////////////////////

#include <radar/AtmosAtten.hh>
#include <toolsa/mem.h>
#include <iostream>
using namespace std;

const double AtmosAtten::_tableDeltaRange = 1.0;
const int AtmosAtten::_tableNRange = 500;

const double AtmosAtten::_tableDeltaElev = 0.1;
const int AtmosAtten::_tableNElev = 200;

// Constructor

AtmosAtten::AtmosAtten()
{
  
  _tableMaxRange = _tableDeltaRange * (_tableNRange - 1);
  _tableMaxElev = _tableDeltaElev * (_tableNElev - 1);

  _method = METHOD_NONE;
  _table = NULL;

  _attenDbPerKm = 0.0;
  
}

// destructor

AtmosAtten::~AtmosAtten()

{

  if (_table) {
    ufree2((void **) _table);
  }

}

///////////////////////////////////////////////////
// set zero atmospheric attenuation
// this sets the method to METHOD_NONE

void AtmosAtten::setAttenNone()

{ 
  
  _method = METHOD_NONE;
  _attenDbPerKm = 0.0;

}
  
///////////////////////////////////////////////////
// set constant atmospheric attenuation in db/km
// this sets the method to METHOD_CONSTANT

void AtmosAtten::setAttenConstant(double val)

{ 
  
  _method = METHOD_CONSTANT;
  _attenDbPerKm = val;

}
  
/////////////////////////////////////////////////////////////
// set method to CRPL - Central Radio Propagation Laboratory
// See Doviak and Zrnic page 44

void AtmosAtten::setAttenCrpl(double wavelengthCm)

{ 

  if (_method == METHOD_CRPL &&
      fabs(wavelengthCm - _wavelengthCm) < 0.0001) {
    // no change
    return;
  }

  _method = METHOD_CRPL;
  _wavelengthCm = wavelengthCm;

  // compute correction factor for wavelength by using a piece-wise linear
  // fit to the points in Doviak and Zrnic, Fig 3.6, page 45:
  //   correction at 10.0 cm = 1.0
  //   correction at  5.0 cm = 1.2
  //   correction at  3.3 cm = 1.5
  
  if (wavelengthCm >= 5.0) {
    double fraction = (wavelengthCm - 5.0) / 5.0;
    _wavelengthCorrection = 1.2 - fraction * 0.2;
  } else {
    double fraction = (wavelengthCm - 3.3) / 1.7;
    _wavelengthCorrection = 1.5 - fraction * 0.3;
  }

  // load up table

  if (_table == NULL) {
    _table = (double **) umalloc2(_tableNElev, _tableNRange, sizeof(double));
  }

  double elev = 0.0;
  for (int ielev = 0; ielev < _tableNElev; ielev++, elev += _tableDeltaElev) {

    double range = 0.0;
    for (int irange = 0; irange < _tableNRange; irange++, range += _tableDeltaRange) {

      double atten10cm =
        (0.4 + 3.45 * exp(-elev / 1.8)) *
        (1.0 - exp(-range / (27.8 + 154.0 * exp(-elev / 2.2))));
      
      // adjust for wavelength
      
      double attenDb = atten10cm * _wavelengthCorrection;
      _table[ielev][irange] = attenDb;
      
    } // irange

  } // ielev

}

//////////////////////////////////////////////////////
// get estimated attenuation at an elevation and range
// Note: this is 2-way attenuation

double AtmosAtten::getAtten(double elevationDeg, double rangeKm) const

{

  double attenDb = 0.0;

  if (_method == METHOD_CONSTANT) {

    attenDb = rangeKm * _attenDbPerKm;

  } else if (_method == METHOD_CRPL) {

    int ielev = (int) ((elevationDeg / _tableDeltaElev) + 0.5);
    if (ielev < 0) {
      ielev = 0;
    } else if (ielev > _tableNElev - 1) {
      ielev = _tableNElev - 1;
    }

    int irange = (int) ((rangeKm / _tableDeltaRange) + 0.5);
    if (irange > _tableNRange - 1) {
      irange = _tableNRange - 1;
    } else if (irange < 0) {
      irange = 0;
    }

    attenDb = _table[ielev][irange];

  }

  return attenDb;

}
  
/////////////////////////////////////  
// print attenuation table

void AtmosAtten::printTable(ostream &out) const

{

  if (_table == NULL) {
    out << "AtmosAtten - no table defined" << endl;
    return;
  }

  out << "==============================================" << endl;
  out << "AtmosAtten table:" << endl;
  out << "  wavelengthCm: " << _wavelengthCm << endl;
  out << "  wavelengthCorrection: " << _wavelengthCorrection << endl;
  for (int ielev = 0; ielev < _tableNElev; ielev++) {
    double elev = ielev * _tableDeltaElev;
    for (int irange = 0; irange < _tableNRange; irange += 10) {
      double range = irange * _tableDeltaRange;
      out << "elev, range, atten: "
          << elev << ", "
          << range << ", "
          << _table[ielev][irange] << endl;
    } // irange
  } // ielev
  out << "==============================================" << endl;


}


