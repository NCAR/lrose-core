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
// UsgsData.cc
//
// Handles Volcanoes and Earthquakes point data from USGS
//
// Jason Craig, RAP, NCAR, Boulder, CO
//
// Nov 2006
//
//////////////////////////////////////////////////////////////


#include <rapformats/UsgsData.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
using namespace std;

const fl32 UsgsData::VALUE_UNKNOWN = -9999.0;

///////////////
// constructor

UsgsData::UsgsData()
{
 MEM_zero(_usgsData);
}

/////////////
// destructor

UsgsData::~UsgsData()
{

}

// set methods

void UsgsData::setUsgsData(const usgsData_t &usgsData)
{
  _usgsData = usgsData;
}


///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void UsgsData::assemble()
  
{

  _memBuf.free();
  
  usgsData_t usgsData = _usgsData;
  _usgsData_to_BE(usgsData);
  _memBuf.add(&usgsData, sizeof(usgsData));
  
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int UsgsData::disassemble(const void *buf, int len)

{

  int minLen = sizeof(usgsData_t);
  if (len < minLen) {
    cerr << "ERROR - UsgsData::disassemble" << endl;
    cerr << "  Buffer too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    return -1;
  }


  memcpy(&_usgsData, (ui08 *) buf, sizeof(usgsData_t));
  _usgsData_from_BE(_usgsData);
  
  return 0;

}

//////////////////////
// printing object


void UsgsData::print(ostream &out, string spacer /* = ""*/ ) const

{

  out << spacer << "-----------------------------------" << endl;
  if(_usgsData.data_type == USGS_DATA_EARTHQUAKE) {
    out << spacer << "USGS Earthquake Report" << endl;
    out << spacer << "Data Time: " << DateTime::str(_usgsData.time) << endl;
    out << spacer << "Latitude (deg): " << _usgsData.lat << endl;
    out << spacer << "Longitude (deg): " << _usgsData.lon << endl;
    out << spacer << "Depth (km): "  << _usgsData.alt << endl;
    out << spacer << "Version: "  << _usgsData.version << endl;
    out << spacer << "Magnitude: "  << _usgsData.magnitude << endl;
    if(_usgsData.magnitude_type == MAGNITUDE_DURATIAON)
      out << spacer << "Magnitude Type: Duration" << endl;
    else if(_usgsData.magnitude_type == MAGNITUDE_LOCAL)
      out << spacer << "Magnitude Type: Local" << endl;
    else if(_usgsData.magnitude_type == MAGNITUDE_SURFACE_WAVE)
      out << spacer << "Magnitude Type: Surface Wave" << endl;
    else if(_usgsData.magnitude_type == MAGNITUDE_MOMENT)
      out << spacer << "Magnitude Type: Moment" << endl;
    else if(_usgsData.magnitude_type == MAGNITUDE_BODY)
      out << spacer << "Magnitude Type: Body" << endl;
    else
      out << spacer << "Magnitude Type: Unknown" << endl;
    out << spacer << "Horizontal Error: "  << _usgsData.horizontalError << endl;
    out << spacer << "Veritcal Error: "  << _usgsData.verticalError << endl;
    out << spacer << "Min Distance: "  << _usgsData.minDistance << endl;
    out << spacer << "RMS Time Error: "  << _usgsData.rmsTimeError << endl;
    out << spacer << "Azimuthal Gap: "  << _usgsData.azimuthalGap << endl;
    out << spacer << "Num Stations: "  << _usgsData.numStations << endl;
    out << spacer << "Num Phases: "  << _usgsData.numPhases << endl;
  } else if(_usgsData.data_type == USGS_DATA_VOLCANO) {
    out << spacer << "USGS Volcano Report" << endl;
    out << spacer << "Data Time: " << DateTime::str(_usgsData.time) << endl;
    out << spacer << "Latitude (deg): " << _usgsData.lat << endl;
    out << spacer << "Longitude (deg): " << _usgsData.lon << endl;
    out << spacer << "Summit Altitude (km): "  << _usgsData.alt << endl;
    if(_usgsData.magnitude_type == COLOR_CODE_GREEN)
      out << spacer << "Aviation Code: GREEN" << endl;
    else if(_usgsData.magnitude_type == COLOR_CODE_YELLOW)
      out << spacer << "Aviation Code: YELLOW" << endl;
    else if(_usgsData.magnitude_type == COLOR_CODE_ORANGE)
      out << spacer << "Aviation Code: ORANGE" << endl;
    else if(_usgsData.magnitude_type == COLOR_CODE_RED)
      out << spacer << "Aviation Code: RED" << endl;
    else 
      out << spacer << "Aviation Code: UNKOWN" << endl;
  }
  out << spacer << "Source Name: " << _usgsData.sourceName << endl;
  out << spacer << "Event Title: " << _usgsData.eventTitle << endl;
  out << spacer << "Event ID: " << _usgsData.eventID << endl;
}

/////////////////
// byte swapping

void UsgsData::_usgsData_to_BE(usgsData_t &rep)
{
  BE_from_array_32(&rep, NBYTES_32);
}

void UsgsData::_usgsData_from_BE(usgsData_t &rep)
{
  BE_to_array_32(&rep, NBYTES_32);
}



