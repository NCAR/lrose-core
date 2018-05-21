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
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
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

/////////////////////////////////////////////////////////
// convert to XML

void RadxCfactors::convert2Xml(string &xml, int level /* = 0 */)  const
  
{

  xml.clear();
  xml += RadxXml::writeStartTag("RadxCfactors", level);

  xml += RadxXml::writeDouble("azimuthCorr", level + 1, _azimuthCorr);
  xml += RadxXml::writeDouble("elevationCorr", level + 1, _elevationCorr);
  xml += RadxXml::writeDouble("rangeCorr", level + 1, _rangeCorr);
  xml += RadxXml::writeDouble("longitudeCorr", level + 1, _longitudeCorr);
  xml += RadxXml::writeDouble("latitudeCorr", level + 1, _latitudeCorr);
  xml += RadxXml::writeDouble("pressureAltCorr", level + 1, _pressureAltCorr);
  xml += RadxXml::writeDouble("altitudeCorr", level + 1, _altitudeCorr);
  xml += RadxXml::writeDouble("ewVelCorr", level + 1, _ewVelCorr);
  xml += RadxXml::writeDouble("nsVelCorr", level + 1, _nsVelCorr);
  xml += RadxXml::writeDouble("vertVelCorr", level + 1, _vertVelCorr);
  xml += RadxXml::writeDouble("headingCorr", level + 1, _headingCorr);
  xml += RadxXml::writeDouble("rollCorr", level + 1, _rollCorr);
  xml += RadxXml::writeDouble("pitchCorr", level + 1, _pitchCorr);
  xml += RadxXml::writeDouble("driftCorr", level + 1, _driftCorr);
  xml += RadxXml::writeDouble("rotationCorr", level + 1, _rotationCorr);
  xml += RadxXml::writeDouble("tiltCorr", level + 1, _tiltCorr);

  xml += RadxXml::writeEndTag("RadxCFactors", level);

}

/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxCfactors::serialize(RadxMsg &msg)
  
{

  // init

  msg.clearAll();
  msg.setMsgType(RadxMsg::RadxCfactorsMsg);

  // add metadata numbers
  
  _loadMetaNumbersToMsg();
  msg.addPart(_metaNumbersPartId, &_metaNumbers, sizeof(msgMetaNumbers_t));

}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxCfactors::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  _init();

  // check type

  if (msg.getMsgType() != RadxMsg::RadxCfactorsMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxCfactors::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata numbers
  
  const RadxMsg::Part *metaNumsPart = msg.getPartByType(_metaNumbersPartId);
  if (metaNumsPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxCfactors::deserialize" << endl;
    cerr << "  No metadata numbers part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaNumbersFromMsg((msgMetaNumbers_t *) metaNumsPart->getBuf(),
                             metaNumsPart->getLength(),
                             msg.getSwap())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxCfactors::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// load the meta number to the message struct

void RadxCfactors::_loadMetaNumbersToMsg()
  
{

  // clear

  memset(&_metaNumbers, 0, sizeof(_metaNumbers));
  
  // set

  _metaNumbers.azimuthCorr = _azimuthCorr;
  _metaNumbers.elevationCorr = _elevationCorr;
  _metaNumbers.rangeCorr = _rangeCorr;
  _metaNumbers.longitudeCorr = _longitudeCorr;
  _metaNumbers.latitudeCorr = _latitudeCorr;
  _metaNumbers.pressureAltCorr = _pressureAltCorr;
  _metaNumbers.altitudeCorr = _altitudeCorr;
  _metaNumbers.ewVelCorr = _ewVelCorr;
  _metaNumbers.nsVelCorr = _nsVelCorr;
  _metaNumbers.vertVelCorr = _vertVelCorr;
  _metaNumbers.headingCorr = _headingCorr;
  _metaNumbers.rollCorr = _rollCorr;
  _metaNumbers.pitchCorr = _pitchCorr;
  _metaNumbers.driftCorr = _driftCorr;
  _metaNumbers.rotationCorr = _rotationCorr;
  _metaNumbers.tiltCorr = _tiltCorr;

}

/////////////////////////////////////////////////////////
// set the meta number data from the message struct

int RadxCfactors::_setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                                         size_t bufLen,
                                         bool swap)
  
{
  
  // check size
  
  if (bufLen != sizeof(msgMetaNumbers_t)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxCfactors::_setMetaNumbersFromMsg" << endl;
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
  
  _azimuthCorr = _metaNumbers.azimuthCorr;
  _elevationCorr = _metaNumbers.elevationCorr;
  _rangeCorr = _metaNumbers.rangeCorr;
  _longitudeCorr = _metaNumbers.longitudeCorr;
  _latitudeCorr = _metaNumbers.latitudeCorr;
  _pressureAltCorr = _metaNumbers.pressureAltCorr;
  _altitudeCorr = _metaNumbers.altitudeCorr;
  _ewVelCorr = _metaNumbers.ewVelCorr;
  _nsVelCorr = _metaNumbers.nsVelCorr;
  _vertVelCorr = _metaNumbers.vertVelCorr;
  _headingCorr = _metaNumbers.headingCorr;
  _rollCorr = _metaNumbers.rollCorr;
  _pitchCorr = _metaNumbers.pitchCorr;
  _driftCorr = _metaNumbers.driftCorr;
  _rotationCorr = _metaNumbers.rotationCorr;
  _tiltCorr = _metaNumbers.tiltCorr;

  return 0;

}

/////////////////////////////////////////////////////////
// swap meta numbers

void RadxCfactors::_swapMetaNumbers(msgMetaNumbers_t &meta)
{
  ByteOrder::swap64(&meta, sizeof(msgMetaNumbers_t));
}
