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
//////////////////////////////////////////////////////////
// Channel.cc
//
// Class for representing characteristics of a channel.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
//////////////////////////////////////////////////////////

#include "Channel.hh"
#include <cmath>
#include <iostream>

// constants are in MKS

const double Channel::kelvinToCelsius = -273.16; // deg
const double Channel::planck = 6.626068e-34;     // joule.s
const double Channel::boltzman = 1.38066e-23;    // joule/deg
const double Channel::speedOfLight = 2.997925e8; // m/s
const double Channel::C1 = 1.19104e-5;           // mw/(m2.sr.cm-4)
const double Channel::C2 = 1.43877;              // K/(cm-1)

// Constructor

Channel::Channel(int id, const string &name,
		 double wave_length, double wave_number,
		 double cal_slope, double cal_offset,
	         double temp_correction_scale, double temp_correction_bias,
	         bool is_passive,
                 double solar_factor) :
        _id(id),
        _name(name),
        _waveLength(wave_length),
        _waveNumber(wave_number),
        _calSlope(cal_slope),
        _calOffset(cal_offset),
	_tempCorrectionScale(temp_correction_scale),
	_tempCorrectionBias(temp_correction_bias),
        _isPassive(is_passive),
        _solarFactor(solar_factor)
  
{

  // compute constants for retrieving brightness temp

  _k1 = 2.0 * planck * speedOfLight * speedOfLight * pow(_waveLength, -5.0);
  _k2 = (planck * speedOfLight) / (boltzman * _waveLength);

  _c1 = C1 * pow(_waveNumber, 3.0);
  _c2 = C2 * _waveNumber;

//   cerr << "waveLength, k1, k2, C1, C2: " << _waveLength << ", "
//        << _k1 << ", " << _k2 << ", " << _c1 << ", " << _c2 << endl;

}

// Destructor

Channel::~Channel()

{

}

///////////////////////////////////////////////
// compute brightness temperature from radiance
//
// Radiance is specified in mW/(m2.sr.cm-1)

double Channel::temp(double radiance) const

{

  //   double radMKS = radiance * 1.0e5;
  //   double temp = _k2 / log((_k1 / radMKS) + 1.0);

  double temp = _c2 / log((_c1 / radiance) + 1.0);
  double correctedTemp = (temp - _tempCorrectionBias) / _tempCorrectionScale;
  return correctedTemp;

}

