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
// MdvxStdAtmos.cc
//
// Computations related to the ICAO standard atmosphere
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2002
//
///////////////////////////////////////////////////////////////


#include <Mdv/MdvxStdAtmos.hh>

#include <iostream>
#include <math.h>
using namespace std;

// initialize statics

// altitude limits in meters

const double MdvxStdAtmos::_limits[8] = { 0, 11000, 20000, 32000,
					  47000, 51000, 71000, 84852 };

// lapse rate in K/km (0.009 represents 0)

const double MdvxStdAtmos::_lrs[7] = { -0.0065, 0.009, 0.001, 0.0028,
				       0.009, -0.0028, -0.002 };

const double MdvxStdAtmos::G = 9.80655;
const double MdvxStdAtmos::R = 287.053;
const double MdvxStdAtmos::GMR = 9.80655 / 287.053;

// is lapse rate zero

const double MdvxStdAtmos::_isZero[7] = { 0, 1, 0, 0, 1, 0, 0 };

// Constructor

MdvxStdAtmos::MdvxStdAtmos()

{

  // initialize

  _pb[0] = 1013.25;
  _tb[0] = 288.15;

  // loop over layers and get pressures and temperatures at level bases

  for (int i = 0; i < 6; i++) {
    _tb[i+1] = _tb[i] +
      (1 - _isZero[i]) * _lrs[i] * (_limits[i+1] - _limits[i]);
    _pb[i+1] = (1 - _isZero[i]) * _pb[i] *
      exp(log(_tb[i] / _tb[i+1]) * GMR / _lrs[i])  +
      _isZero[i] * _pb[i] * exp(-GMR * (_limits[i+1] - _limits[i]) / _tb[i]);
  }

}

// destructor

MdvxStdAtmos::~MdvxStdAtmos()

{

}

////////////////////////////////
// convert pressure(mb) to ht(m)

double MdvxStdAtmos::pres2ht(double pres) const

{

  int ii = 6;
  if (pres > _pb[1]) {
    ii = 0;
  } else {
    for (int i = 1; i < 8; i++) {
      if (pres > _pb[i]) {
	ii = i - 1;
	break;
      }
    }
  }

  double ht =
    _isZero[ii] * (-log(pres / _pb[ii]) * _tb[ii] / GMR + _limits[ii]) +
    (1- _isZero[ii]) *
    ((_tb[ii] / pow((pres/_pb[ii]),(_lrs[ii] / GMR)) -
      _tb[ii]) / _lrs[ii] + _limits[ii]);
  
  return ht;

}

/////////////////////////////////////////////
// convert pressure(mb) to flight level (hft)

double MdvxStdAtmos::pres2flevel(double pres) const

{
  return pres2ht(pres) / 30.48;
}

/////////////////////////////////
// convert ht(m) to pressure(mb)

double MdvxStdAtmos::ht2pres(double ht) const

{

  int ii = 6;
  if (ht < _limits[1]) {
    ii = 0;
  } else {
    for (int i = 1; i < 8; i++) {
      if (ht < _limits[i]) {
	ii = i - 1;
	break;
      }
    }
  }

  double pres =
    _isZero[ii] * _pb[ii] *exp(-GMR* (ht- _limits[ii]) / _tb[ii]) +
    (1.0 - _isZero[ii]) * _pb[ii] *
    pow((_tb[ii] / (_tb[ii] + _lrs[ii] * (ht - _limits[ii]))),
	(GMR / _lrs[ii]));
  
  return pres;

}

/////////////////////////////////////////////
// convert flight level (hft) to pressure(mb)

double MdvxStdAtmos::flevel2pres(double flevel) const

{
  return ht2pres(flevel * 30.48);
}

/////////////////////////////////
// convert ht(m) to tmp (K)

double MdvxStdAtmos::ht2temp(double ht) const

{

  int ii = 6;
  if (ht < _limits[1]) {
    ii = 0;
  } else {
    for (int i = 1; i < 8; i++) {
      if (ht < _limits[i]) {
	ii = i - 1;
	break;
      }
    }
  }

  double htdiff = ht - _limits[ii];
  double temp = _tb[ii] + (1.0 - _isZero[ii]) * _lrs[ii] * htdiff;

  return temp;

}


/////////////////////////////////////////////
// convert flight level (hft) to height (m)
  
double MdvxStdAtmos::flevel2ht(double flevel) const

{
  return (flevel * 30.48);
}

/////////////////////////////////////////////
// convert ht(m) to flight level (hft)
  
double MdvxStdAtmos::ht2flevel(double ht) const

{
  return (ht / 30.48);
}

