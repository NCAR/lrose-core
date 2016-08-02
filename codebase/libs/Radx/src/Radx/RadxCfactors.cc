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
// RadxCfactors.cc
//
// Correction factors for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/Radx.hh>
#include <Radx/RadxCfactors.hh>
using namespace std;

/////////////////////////////////////////////////////////
// RadxCfactors constructor

RadxCfactors::RadxCfactors()
  
{
  _init();
}

/////////////////////////////////////////////////////////
// RadxCfactors destructor

RadxCfactors::~RadxCfactors()
  
{
}

//////////////////////////////////////////////////
// initialize

void RadxCfactors::_init()
  
{

  _azimuthCorr = 0.0;
  _elevationCorr = 0.0;
  _rangeCorr = 0.0;
  _longitudeCorr = 0.0;
  _latitudeCorr = 0.0;
  _pressureAltCorr = 0.0;
  _altitudeCorr = 0.0;
  _ewVelCorr = 0.0;
  _nsVelCorr = 0.0;
  _vertVelCorr = 0.0;
  _headingCorr = 0.0;
  _rollCorr = 0.0;
  _pitchCorr = 0.0;
  _driftCorr = 0.0;
  _rotationCorr = 0.0;
  _tiltCorr = 0.0;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxCfactors::clear()
  
{
  _init();
}

/////////////////////////////////////////////////////////
// print

void RadxCfactors::print(ostream &out) const
  
{
  
  out << "=============== RadxCfactors ===============" << endl;
  out << "Correction factors:" << endl;
  out << "  azimuthCorr: " << _azimuthCorr << endl;
  out << "  elevationCorr: " << _elevationCorr << endl;
  out << "  rangeCorr: " << _rangeCorr << endl;
  out << "  longitudeCorr: " << _longitudeCorr << endl;
  out << "  latitudeCorr: " << _latitudeCorr << endl;
  out << "  pressureAltCorr: " << _pressureAltCorr << endl;
  out << "  altitudeCorr: " << _altitudeCorr << endl;
  out << "  ewVelCorr: " << _ewVelCorr << endl;
  out << "  nsVelCorr: " << _nsVelCorr << endl;
  out << "  vertVelCorr: " << _vertVelCorr << endl;
  out << "  headingCorr: " << _headingCorr << endl;
  out << "  rollCorr: " << _rollCorr << endl;
  out << "  pitchCorr: " << _pitchCorr << endl;
  out << "  driftCorr: " << _driftCorr << endl;
  out << "  rotationCorr: " << _rotationCorr << endl;
  out << "  tiltCorr: " << _tiltCorr << endl;
  out << "============================================" << endl;

}


