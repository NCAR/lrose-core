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
// RadxGeoref.cc
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
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
using namespace std;

/////////////////////////////////////////////////////////
// RadxGeoref constructor

RadxGeoref::RadxGeoref()
  
{
  _setToMissing();
}

/////////////////////////////////////////////////////////
// RadxGeoref destructor

RadxGeoref::~RadxGeoref()
  
{
}

//////////////////////////////////////////////////
// initialize

void RadxGeoref::_setToMissing()
  
{

  _timeSecs = 0;
  _nanoSecs = 0;
  _longitude = Radx::missingMetaDouble;
  _latitude = Radx::missingMetaDouble;
  _altitudeKmMsl = Radx::missingMetaDouble;
  _altitudeKmAgl = Radx::missingMetaDouble;
  _ewVelocity = Radx::missingMetaDouble;
  _nsVelocity = Radx::missingMetaDouble;
  _vertVelocity = Radx::missingMetaDouble;
  _heading = Radx::missingMetaDouble;
  _track = Radx::missingMetaDouble;
  _roll = Radx::missingMetaDouble;
  _pitch = Radx::missingMetaDouble;
  _drift = Radx::missingMetaDouble;
  _rotation = Radx::missingMetaDouble;
  _tilt = Radx::missingMetaDouble;
  _ewWind = Radx::missingMetaDouble;
  _nsWind = Radx::missingMetaDouble;
  _vertWind = Radx::missingMetaDouble;
  _headingRate = Radx::missingMetaDouble;
  _pitchRate = Radx::missingMetaDouble;
  _rollRate = Radx::missingMetaDouble;
  _driveAngle1 = Radx::missingMetaDouble;
  _driveAngle2 = Radx::missingMetaDouble;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxGeoref::clear()
  
{
  _setToMissing();
}

//////////////////////////////////////////////////
// set all elements to 0

void RadxGeoref::setToZero()
  
{

  _timeSecs = 0;
  _nanoSecs = 0;
  _longitude = 0.0;
  _latitude = 0.0;
  _altitudeKmMsl = 0.0;
  _altitudeKmAgl = 0.0;
  _ewVelocity = 0.0;
  _nsVelocity = 0.0;
  _vertVelocity = 0.0;
  _heading = 0.0;
  _track = 0.0;
  _roll = 0.0;
  _pitch = 0.0;
  _drift = 0.0;
  _rotation = 0.0;
  _tilt = 0.0;
  _ewWind = 0.0;
  _nsWind = 0.0;
  _vertWind = 0.0;
  _headingRate = 0.0;
  _pitchRate = 0.0;
  _rollRate = 0.0;
  _driveAngle1 = 0.0;
  _driveAngle2 = 0.0;

}

//////////////////////////////////////////////////
/// increment count if elements are not missing
///
/// Goes through the object looking for non-missing elements and
/// increments the count accordingly
 
void RadxGeoref::incrementIfNotMissing(RadxGeoref &count)
  
{

  if (_timeSecs != Radx::missingMetaInt) count._timeSecs++;
  if (_nanoSecs != Radx::missingMetaInt) count._nanoSecs++;
  if (_longitude != Radx::missingMetaDouble) count._longitude++;
  if (_latitude != Radx::missingMetaDouble) count._latitude++;
  if (_altitudeKmMsl != Radx::missingMetaDouble) count._altitudeKmMsl++;
  if (_altitudeKmAgl != Radx::missingMetaDouble) count._altitudeKmAgl++;
  if (_ewVelocity != Radx::missingMetaDouble) count._ewVelocity++;
  if (_nsVelocity != Radx::missingMetaDouble) count._nsVelocity++;
  if (_vertVelocity != Radx::missingMetaDouble) count._vertVelocity++;
  if (_heading != Radx::missingMetaDouble) count._heading++;
  if (_track != Radx::missingMetaDouble) count._track++;
  if (_roll != Radx::missingMetaDouble) count._roll++;
  if (_pitch != Radx::missingMetaDouble) count._pitch++;
  if (_drift != Radx::missingMetaDouble) count._drift++;
  if (_rotation != Radx::missingMetaDouble) count._rotation++;
  if (_tilt != Radx::missingMetaDouble) count._tilt++;
  if (_ewWind != Radx::missingMetaDouble) count._ewWind++;
  if (_nsWind != Radx::missingMetaDouble) count._nsWind++;
  if (_vertWind != Radx::missingMetaDouble) count._vertWind++;
  if (_headingRate != Radx::missingMetaDouble) count._headingRate++;
  if (_pitchRate != Radx::missingMetaDouble) count._pitchRate++;
  if (_rollRate != Radx::missingMetaDouble) count._rollRate++;
  if (_driveAngle1 != Radx::missingMetaDouble) count._driveAngle1++;
  if (_driveAngle2 != Radx::missingMetaDouble) count._driveAngle2++;

}

/////////////////////////////////////////////////////////
// print

void RadxGeoref::print(ostream &out) const
  
{
  
  out << "=============== RadxGeoref ===============" << endl;
  out << "Geo-reference variables:" << endl;
  if (_timeSecs != 0) {
    RadxTime gtime(_timeSecs, _nanoSecs / 1.0e9);
    out << "  time: " << gtime.getStr(6) << endl;
  }
  out << "  longitude: " << _longitude << endl;
  out << "  latitude: " << _latitude << endl;
  out << "  altitudeKmMsl: " << _altitudeKmMsl << endl;
  out << "  altitudeKmAgl: " << _altitudeKmAgl << endl;
  out << "  ewVelocity: " << _ewVelocity << endl;
  out << "  nsVelocity: " << _nsVelocity << endl;
  out << "  vertVelocity: " << _vertVelocity << endl;
  out << "  heading: " << _heading << endl;
  out << "  track: " << _track << endl;
  out << "  roll: " << _roll << endl;
  out << "  pitch: " << _pitch << endl;
  out << "  drift: " << _drift << endl;
  out << "  rotation: " << _rotation << endl;
  out << "  tilt: " << _tilt << endl;
  out << "  ewWind: " << _ewWind << endl;
  out << "  nsWind: " << _nsWind << endl;
  out << "  vertWind: " << _vertWind << endl;
  out << "  headingRate: " << _headingRate << endl;
  out << "  pitchRate: " << _pitchRate << endl;
  out << "  rollRate: " << _rollRate << endl;
  out << "  driveAngle1: " << _driveAngle1 << endl;
  out << "  driveAngle2: " << _driveAngle2 << endl;
  out << "==========================================" << endl;

}


