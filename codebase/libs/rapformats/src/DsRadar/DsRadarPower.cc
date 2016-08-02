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
// DsRadarPower.cc
//
// C++ class for dealing with radar power information
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Sept 2007
//////////////////////////////////////////////////////////////

#include <rapformats/DsRadarPower.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
using namespace std;

///////////////
// constructor

DsRadarPower::DsRadarPower()
  
{
  
  _versionNum = 1;
  clear();

}

/////////////
// destructor

DsRadarPower::~DsRadarPower()

{

}

//////////////////////////
// clear all data members

void DsRadarPower::clear()

{

  _utimeSecs = DsRadarPower::MISSING_VAL;
  _utimeNanoSecs = DsRadarPower::MISSING_VAL;

  _xmitPowerH = DsRadarPower::MISSING_VAL;
  _xmitPowerV = DsRadarPower::MISSING_VAL;

}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void DsRadarPower::assemble()
  
{

  // print object to string as XML

  string xml;

  xml += TaXml::writeStartTag("DsRadarPower", 0);
  xml += TaXml::writeInt("version", 1, _versionNum);
  xml += TaXml::writeUtime("utime_secs", 1, _utimeSecs);
  xml += TaXml::writeUtime("utime_nano_secs", 1, _utimeNanoSecs);
  xml += TaXml::writeDouble("xmit_power_h", 1, _xmitPowerH);
  xml += TaXml::writeDouble("xmit_power_v", 1, _xmitPowerV);
  xml += TaXml::writeEndTag("DsRadarPower", 0);

  // free up mem buffer
  
  _memBuf.free();

  // add xml string to buffer
  
  _memBuf.add(xml.c_str(), xml.size());

}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int DsRadarPower::disassemble(const void *buf, int len)

{

  // make copy of buffer, make sure it is null terminated

  TaArray<char> copyArray;
  char *copy = copyArray.alloc(len + 1);
  memcpy(copy, buf, len);
  copy[len] = '\0';

  // remove comments
  
  string xmlBuf = TaXml::removeComments(copy);
  
  // clear state

  clear();
  
  // set state from the XML

  int iret = 0;

  if (TaXml::readInt(xmlBuf, "version", _versionNum)) {
    cerr << "  ERROR - cannot find <version>" << endl;
    iret = -1;
  }
  if (TaXml::readTime(xmlBuf, "utime_secs", _utimeSecs)) {
    cerr << "  ERROR - cannot find <utime_secs>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "utime_nano_secs", _utimeNanoSecs)) {
    cerr << "  ERROR - cannot find <utime_nano_secs>" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "xmit_power_h", _xmitPowerH)) {
    cerr << "  ERROR - cannot find <xmit_power_h>" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "xmit_power_v", _xmitPowerV)) {
    cerr << "  ERROR - cannot find <xmit_power_v>" << endl;
    iret = -1;
  }

  if (iret) {
    cerr << "ERROR -  DsRadarPower::disassemble" << endl;
    cerr << "XML buffer: " << endl;
    cerr << copy << endl;
    return -1;
  }

  return 0;

}

//////////////////////
// printing object


void DsRadarPower::print(ostream &out, string spacer /* = ""*/ ) const

{

  out << "===================================" << endl;
  out << spacer << "Power info object" << endl;
  out << spacer << "  Version number: " << _versionNum << endl;
  out << spacer << "    utimeSecs: " << utimstr(_utimeSecs) << endl;
  out << spacer << "    utimeNanoSecs: " << _utimeNanoSecs << endl;
  out << spacer << "    xmitPowerH: " << _xmitPowerH << endl;
  out << spacer << "    xmitPowerV: " << _xmitPowerV << endl;
  
}

