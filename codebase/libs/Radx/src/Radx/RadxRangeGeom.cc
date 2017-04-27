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
// RadxRangeGeom.cc
//
// Range geometry for Radx classes
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2010
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxRangeGeom.hh>
#include <Radx/RadxXml.hh>
#include <cmath>
using namespace std;

//////////////
// Constructor

RadxRangeGeom::RadxRangeGeom()
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxRangeGeom::RadxRangeGeom(const RadxRangeGeom &rhs)
     
{
  _copy(rhs);
}

/////////////
// destructor

RadxRangeGeom::~RadxRangeGeom()

{
}

/////////////////////////////
// Assignment
//

RadxRangeGeom &RadxRangeGeom::operator=(const RadxRangeGeom &rhs)
  

{
  return _copy(rhs);
}

/////////////////////////////////////////////////////////
// initialize data members

void RadxRangeGeom::_init()
  
{
  clearRangeGeom();
}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxRangeGeom::clearRangeGeom()
  
{
  _rangeGeomSet = false;
  _startRangeKm = 0.0;
  _gateSpacingKm = 1.0;
}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxRangeGeom &RadxRangeGeom::_copy(const RadxRangeGeom &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }
  
  _rangeGeomSet = rhs._rangeGeomSet;
  _startRangeKm = rhs._startRangeKm;
  _gateSpacingKm = rhs._gateSpacingKm;

  return *this;
  
}

/////////////////////////////////////////////////////////
// set geometry for constant gate spacing

void RadxRangeGeom::setRangeGeom(double startRangeKm,
                                 double gateSpacingKm)
  
{
  _rangeGeomSet = true;
  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;
}

/////////////////////////////////////////////////////////
// copy the geometry from another object

void RadxRangeGeom::copyRangeGeom(const RadxRangeGeom &master)

{
  RadxRangeGeom::_copy(master);
}

/////////////////////////////////////////////////////////
// print geometry

void RadxRangeGeom::print(ostream &out) const
  
{

  out << "  RadxRangeGeom:" << endl;
  out << "    rangeGeomSet: " << (_rangeGeomSet?"Y":"N") << endl;
  out << "    startRangeKm: " << _startRangeKm << endl;
  out << "    gateSpacingKm: " << _gateSpacingKm << endl;

}

/////////////////////////////////////////////////////////
// convert to XML

void RadxRangeGeom::convertToXml(string &xml, int level /* = 0 */)  const
  
{

  xml.clear();
  xml += RadxXml::writeStartTag("RadxRangeGeom", level);

  xml += RadxXml::writeBoolean("rangeGeomSet", level + 1, _rangeGeomSet);
  xml += RadxXml::writeDouble("startRangeKm", level + 1, _startRangeKm);
  xml += RadxXml::writeDouble("gateSpacingKm", level + 1, _gateSpacingKm);

  xml += RadxXml::writeEndTag("RadxRangeGeom", level);


}

