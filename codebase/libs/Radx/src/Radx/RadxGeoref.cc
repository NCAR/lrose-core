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
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
using namespace std;

/////////////////////////////////////////////////////////
// RadxGeoref constructor

RadxGeoref::RadxGeoref()
  
{
  _init();
}

/////////////////////////////////////////////////////////
// RadxGeoref destructor

RadxGeoref::~RadxGeoref()
  
{
}

//////////////////////////////////////////////////
// initialize

void RadxGeoref::_init()
  
{

  _timeSecs = 0;
  _nanoSecs = 0;
  _unitNum = 0;
  _unitId = 0;
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
  _init();
}

//////////////////////////////////////////////////
// set all elements to 0

void RadxGeoref::setToZero()
  
{

  _timeSecs = 0;
  _nanoSecs = 0;
  _unitNum = 0;
  _unitId = 0;
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
// set time from RadxTime

void RadxGeoref::setRadxTime(const RadxTime &rtime)
{
  _timeSecs = rtime.utime();
  _nanoSecs = rtime.getSubSec() * 1.0e9;
}

//////////////////////////////////////////////////
// get time as RadxTime

RadxTime RadxGeoref::getRadxTime() const
{
  RadxTime rtime(_timeSecs, _nanoSecs / 1.0e9);
  return rtime;
}

//////////////////////////////////////////////////
/// increment count if elements are not missing
///
/// Goes through the object looking for non-missing elements and
/// increments the count accordingly
 
void RadxGeoref::incrementIfNotMissing(RadxGeoref &count)
  
{

  if (_timeSecs != Radx::missingMetaInt) count._timeSecs++;
  if (_unitNum != 0) count._unitNum++;
  if (_unitId != 0) count._unitId++;
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
  out << "  unitNum: " << _unitNum << endl;
  out << "  unitId: " << _unitId << endl;
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

/////////////////////////////////////////////////////////
// convert to XML

void RadxGeoref::convert2Xml(string &xml, int level /* = 0 */)  const
  
{

  xml.clear();
  xml += RadxXml::writeStartTag("RadxGeoref", level);

  xml += RadxXml::writeTime("timeSecs", level + 1, _timeSecs);
  xml += RadxXml::writeDouble("nanoSecs", level + 1, _nanoSecs);

  xml += RadxXml::writeLong("unitNum", level + 1, (long) _unitNum);
  xml += RadxXml::writeLong("unitId", level + 1, (long) _unitId);

  xml += RadxXml::writeDouble("longitude", level + 1, _longitude);
  xml += RadxXml::writeDouble("latitude", level + 1, _latitude);
  xml += RadxXml::writeDouble("altitudeKmMsl", level + 1, _altitudeKmMsl);
  xml += RadxXml::writeDouble("altitudeKmAgl", level + 1, _altitudeKmAgl);
  xml += RadxXml::writeDouble("ewVelocity", level + 1, _ewVelocity);
  xml += RadxXml::writeDouble("nsVelocity", level + 1, _nsVelocity);
  xml += RadxXml::writeDouble("vertVelocity", level + 1, _vertVelocity);
  xml += RadxXml::writeDouble("heading", level + 1, _heading);
  xml += RadxXml::writeDouble("track", level + 1, _track);
  xml += RadxXml::writeDouble("roll", level + 1, _roll);
  xml += RadxXml::writeDouble("pitch", level + 1, _pitch);
  xml += RadxXml::writeDouble("drift", level + 1, _drift);
  xml += RadxXml::writeDouble("rotation", level + 1, _rotation);
  xml += RadxXml::writeDouble("tilt", level + 1, _tilt);
  xml += RadxXml::writeDouble("ewWind", level + 1, _ewWind);
  xml += RadxXml::writeDouble("nsWind", level + 1, _nsWind);
  xml += RadxXml::writeDouble("vertWind", level + 1, _vertWind);
  xml += RadxXml::writeDouble("headingRate", level + 1, _headingRate);
  xml += RadxXml::writeDouble("pitchRate", level + 1, _pitchRate);
  xml += RadxXml::writeDouble("rollRate", level + 1, _rollRate);
  xml += RadxXml::writeDouble("driveAngle1", level + 1, _driveAngle1);
  xml += RadxXml::writeDouble("driveAngle2", level + 1, _driveAngle2);

  xml += RadxXml::writeEndTag("RadxGeoref", level);

}

/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxGeoref::serialize(RadxMsg &msg)
  
{

  // init

  msg.clearAll();
  msg.setMsgType(RadxMsg::RadxGeorefMsg);

  // add metadata numbers
  
  _loadMetaNumbersToMsg();
  msg.addPart(_metaNumbersPartId, &_metaNumbers, sizeof(msgMetaNumbers_t));

}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxGeoref::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  _init();

  // check type

  if (msg.getMsgType() != RadxMsg::RadxGeorefMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxGeoref::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata numbers
  
  const RadxMsg::Part *metaNumsPart = msg.getPartByType(_metaNumbersPartId);
  if (metaNumsPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxGeoref::deserialize" << endl;
    cerr << "  No metadata numbers part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaNumbersFromMsg((msgMetaNumbers_t *) metaNumsPart->getBuf(),
                             metaNumsPart->getLength(),
                             msg.getSwap())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxGeoref::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// load the meta number to the message struct

void RadxGeoref::_loadMetaNumbersToMsg()
  
{

  // clear

  memset(&_metaNumbers, 0, sizeof(_metaNumbers));
  
  // set

  _metaNumbers.timeSecs = _timeSecs;
  _metaNumbers.nanoSecs = _nanoSecs;
  _metaNumbers.unitNum = _unitNum;
  _metaNumbers.unitId = _unitId;
  _metaNumbers.longitude = _longitude;
  _metaNumbers.latitude = _latitude;
  _metaNumbers.altitudeKmMsl = _altitudeKmMsl;
  _metaNumbers.altitudeKmAgl = _altitudeKmAgl;
  _metaNumbers.ewVelocity = _ewVelocity;
  _metaNumbers.nsVelocity = _nsVelocity;
  _metaNumbers.vertVelocity = _vertVelocity;
  _metaNumbers.heading = _heading;
  _metaNumbers.track = _track;
  _metaNumbers.roll = _roll;
  _metaNumbers.pitch = _pitch;
  _metaNumbers.drift = _drift;
  _metaNumbers.rotation = _rotation;
  _metaNumbers.tilt = _tilt;
  _metaNumbers.ewWind = _ewWind;
  _metaNumbers.nsWind = _nsWind;
  _metaNumbers.vertWind = _vertWind;
  _metaNumbers.headingRate = _headingRate;
  _metaNumbers.pitchRate = _pitchRate;
  _metaNumbers.rollRate = _rollRate;
  _metaNumbers.driveAngle1 = _driveAngle1;
  _metaNumbers.driveAngle2 = _driveAngle2;

}

/////////////////////////////////////////////////////////
// set the meta number data from the message struct

int RadxGeoref::_setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                                       size_t bufLen,
                                       bool swap)
  
{
  
  // check size
  
  if (bufLen != sizeof(msgMetaNumbers_t)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxGeoref::_setMetaNumbersFromMsg" << endl;
    cerr << "  Incorrect message size: " << bufLen << endl;
    cerr << "  Should be: " << sizeof(msgMetaNumbers_t) << endl;
    return -1;
  }

  // copy into local struct
  
  _metaNumbers = *metaNumbers;
  
  // swap as needed

  if (swap) {
    _swapMetaNumbers(_metaNumbers); 
  }

  // set members
  
  _timeSecs = _metaNumbers.timeSecs;
  _nanoSecs = _metaNumbers.nanoSecs;
  _unitNum = _metaNumbers.unitNum;
  _unitId = _metaNumbers.unitId;
  _longitude = _metaNumbers.longitude;
  _latitude = _metaNumbers.latitude;
  _altitudeKmMsl = _metaNumbers.altitudeKmMsl;
  _altitudeKmAgl = _metaNumbers.altitudeKmAgl;
  _ewVelocity = _metaNumbers.ewVelocity;
  _nsVelocity = _metaNumbers.nsVelocity;
  _vertVelocity = _metaNumbers.vertVelocity;
  _heading = _metaNumbers.heading;
  _track = _metaNumbers.track;
  _roll = _metaNumbers.roll;
  _pitch = _metaNumbers.pitch;
  _drift = _metaNumbers.drift;
  _rotation = _metaNumbers.rotation;
  _tilt = _metaNumbers.tilt;
  _ewWind = _metaNumbers.ewWind;
  _nsWind = _metaNumbers.nsWind;
  _vertWind = _metaNumbers.vertWind;
  _headingRate = _metaNumbers.headingRate;
  _pitchRate = _metaNumbers.pitchRate;
  _rollRate = _metaNumbers.rollRate;
  _driveAngle1 = _metaNumbers.driveAngle1;
  _driveAngle2 = _metaNumbers.driveAngle2;

  return 0;

}

/////////////////////////////////////////////////////////
// swap meta numbers

void RadxGeoref::_swapMetaNumbers(msgMetaNumbers_t &meta)
{
  ByteOrder::swap64(&meta, sizeof(msgMetaNumbers_t));
}
