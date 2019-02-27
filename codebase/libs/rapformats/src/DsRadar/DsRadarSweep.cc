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
// DsRadarSweep.cc
//
// C++ class for dealing with radar sweep information
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// June 2006
//////////////////////////////////////////////////////////////

#include <rapformats/DsRadarSweep.hh>
#include <rapformats/ds_radar.h>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
using namespace std;

///////////////
// constructor

DsRadarSweep::DsRadarSweep()
  
{
  
  _versionNum = 1;
  clear();

}

/////////////
// destructor

DsRadarSweep::~DsRadarSweep()

{

}

//////////////////////////
// clear all data members

void DsRadarSweep::clear()

{

  _name = "";
  _scanModeStr = "";
  _scanMode = -1;

  _startUTime = DsRadarSweep::MISSING_VAL;
  _startNanoSecs = DsRadarSweep::MISSING_VAL;

  _endUTime = DsRadarSweep::MISSING_VAL;
  _endNanoSecs = DsRadarSweep::MISSING_VAL;

  _startEl = DsRadarSweep::MISSING_VAL;
  _startAz = DsRadarSweep::MISSING_VAL;
  
  _endEl = DsRadarSweep::MISSING_VAL;
  _endAz = DsRadarSweep::MISSING_VAL;
  
  _fixedEl = DsRadarSweep::MISSING_VAL;
  _fixedAz = DsRadarSweep::MISSING_VAL;
  
  _isClockWise = true;
  
  _prf = DsRadarSweep::MISSING_VAL;
  
  _volNum = DsRadarSweep::MISSING_VAL;
  _tiltNum = DsRadarSweep::MISSING_VAL;
  _sweepNum = DsRadarSweep::MISSING_VAL;

  _nSamples = DsRadarSweep::MISSING_VAL;
  _nGates = DsRadarSweep::MISSING_VAL;
  
  _antennaTransition = false;
  
}

/////////////////
// set scan mode

void DsRadarSweep::setScanMode(const string &modeStr) { 
  _scanModeStr = modeStr;
  _scanMode = str2ScanMode(modeStr);
}

void DsRadarSweep::setScanMode(int mode) {
  _scanMode = mode;
  _scanModeStr = scanMode2Str(mode);
}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void DsRadarSweep::assemble()
  
{

  // print object to string as XML

  string xml;

  xml += TaXml::writeStartTag("DsRadarSweep", 0);
  xml += TaXml::writeInt("version", 1, _versionNum);
  xml += TaXml::writeString("name", 1, _name);
  xml += TaXml::writeString("mode", 1, _scanModeStr);
  xml += TaXml::writeUtime("start_utime", 1, _startUTime);
  xml += TaXml::writeInt("start_nano_secs", 1, _startNanoSecs);
  xml += TaXml::writeUtime("end_utime", 1, _endUTime);
  xml += TaXml::writeInt("end_nano_secs", 1, _endNanoSecs);
  xml += TaXml::writeDouble("start_el", 1, _startEl);
  xml += TaXml::writeDouble("start_az", 1, _startAz);
  xml += TaXml::writeDouble("end_el", 1, _endEl);
  xml += TaXml::writeDouble("end_az", 1, _endAz);
  xml += TaXml::writeDouble("fixed_el", 1, _fixedEl);
  xml += TaXml::writeDouble("fixed_az", 1, _fixedAz);
  xml += TaXml::writeBoolean("is_clock_wise", 1, _isClockWise);
  xml += TaXml::writeDouble("prf", 1, _prf);
  xml += TaXml::writeInt("vol_num", 1, _volNum);
  xml += TaXml::writeInt("tilt_num", 1, _tiltNum);
  xml += TaXml::writeInt("sweep_num", 1, _sweepNum);
  xml += TaXml::writeInt("n_samples", 1, _nSamples);
  xml += TaXml::writeInt("n_gates", 1, _nGates);
  xml += TaXml::writeBoolean("antenna_transition", 1, _antennaTransition);
  xml += TaXml::writeEndTag("DsRadarSweep", 0);

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

int DsRadarSweep::disassemble(const void *buf, int len)

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
  if (TaXml::readString(xmlBuf, "name", _name)) {
    cerr << "  ERROR - cannot find <name>" << endl;
    iret = -1;
  }
  if (TaXml::readString(xmlBuf, "mode", _scanModeStr)) {
    cerr << "  ERROR - cannot find <mode>" << endl;
    iret = -1;
  }
  _scanMode = str2ScanMode(_scanModeStr);
  if (TaXml::readTime(xmlBuf, "start_utime", _startUTime)) {
    cerr << "  ERROR - cannot find <start_utime>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "start_nano_secs", _startNanoSecs)) {
    cerr << "  ERROR - cannot find <start_nano_secs>" << endl;
    iret = -1;
  }
  if (TaXml::readTime(xmlBuf, "end_utime", _endUTime)) {
    cerr << "  ERROR - cannot find <end_utime>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "end_nano_secs", _endNanoSecs)) {
    cerr << "  ERROR - cannot find <end_nano_secs>" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "start_el", _startEl)) {
    cerr << "  ERROR - cannot find <start_el>" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "start_az", _startAz)) {
    cerr << "  ERROR - cannot find <start_az>" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "end_el", _endEl)) {
    cerr << "  ERROR - cannot find <end_el>" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "end_az", _endAz)) {
    cerr << "  ERROR - cannot find <end_az>" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "fixed_el", _fixedEl)) {
    if (TaXml::readDouble(xmlBuf, "fixed_angle", _fixedEl)) {
      cerr << "  ERROR - cannot find <fixed_el>" << endl;
      iret = -1;
    }
  }
  if (TaXml::readDouble(xmlBuf, "fixed_az", _fixedAz)) {
     cerr << "  ERROR - cannot find <fixed_az>" << endl;
     iret = -1;
  }
  if (TaXml::readBoolean(xmlBuf, "is_clock_wise", _isClockWise)) {
    cerr << "  ERROR - cannot find <is_clock_wise>" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "prf", _prf)) {
    cerr << "  ERROR - cannot find <prf>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "vol_num", _volNum)) {
    cerr << "  ERROR - cannot find <vol_num>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "tilt_num", _tiltNum)) {
    cerr << "  ERROR - cannot find <tilt_num>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "sweep_num", _sweepNum)) {
    cerr << "  ERROR - cannot find <sweep_num>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "n_samples", _nSamples)) {
    cerr << "  ERROR - cannot find <n_samples>" << endl;
    iret = -1;
  }
  if (TaXml::readInt(xmlBuf, "n_gates", _nGates)) {
    cerr << "  ERROR - cannot find <n_gates>" << endl;
    iret = -1;
  }

  if (TaXml::readBoolean(xmlBuf, "antenna_transition", _antennaTransition)) {
    // this is assumed an optional xml field, with default set to false
  }
  if (iret) {
    cerr << "ERROR -  DsRadarSweep::disassemble" << endl;
    cerr << "XML buffer: " << endl;
    cerr << copy << endl;
    return -1;
  }

  return 0;

}

//////////////////////
// printing object


void DsRadarSweep::print(ostream &out, string spacer /* = ""*/ ) const

{

  out << "===================================" << endl;
  out << spacer << "Sweep info object" << endl;
  out << spacer << "  Version number: " << _versionNum << endl;
  out << spacer << "    name: " << _name << endl;
  out << spacer << "    mode: " << _scanModeStr << endl;
  out << spacer << "    startUTime: " << utimstr(_startUTime) << endl;
  out << spacer << "    startNanoSecs: " << _startNanoSecs << endl;
  out << spacer << "    endUTime: " << utimstr(_endUTime) << endl;
  out << spacer << "    endNanoSecs: " << _endNanoSecs << endl;
  out << spacer << "    startEl: " << _startEl << endl;
  out << spacer << "    startAz: " << _startAz << endl;
  out << spacer << "    endEl: " << _endEl << endl;
  out << spacer << "    endAz: " << _endAz << endl;
  out << spacer << "    fixedEl: " << _fixedEl << endl;
  out << spacer << "    fixedAz: " << _fixedAz << endl;
  out << spacer << "    isClockWise: " <<
    (_isClockWise? "true" : "false") << endl;
  out << spacer << "    prf: " << _prf << endl;
  out << spacer << "    volNum: " << _volNum << endl;
  out << spacer << "    tiltNum: " << _tiltNum << endl;
  out << spacer << "    sweepNum: " << _sweepNum << endl;
  out << spacer << "    nSamples: " << _nSamples << endl;
  out << spacer << "    nGates: " << _nGates << endl;
  out << spacer << "    antennaTransition: " <<
    (_antennaTransition? "true" : "false") << endl;
  out << endl;
  
}

//////////////////////////////////////////////
// Convert scan mode to string and vice versa

string DsRadarSweep::scanMode2Str(int scanMode)

{

  switch (scanMode) {

    case DS_RADAR_CALIBRATION_MODE:
      return "CALIBRATION";

    case DS_RADAR_SECTOR_MODE:
      return "PPI_SECTOR";

    case DS_RADAR_COPLANE_MODE:
      return "COPLANE";

    case DS_RADAR_RHI_MODE:
      return "RHI";

    case DS_RADAR_VERTICAL_POINTING_MODE:
      return "VERT_POINT";

    case DS_RADAR_TARGET_MODE:
      return "TARGET";

    case DS_RADAR_MANUAL_MODE:
      return "MANUAL";

    case DS_RADAR_IDLE_MODE:
      return "IDLE";

    case DS_RADAR_SURVEILLANCE_MODE:
      return "SURVEILLANCE";

    case DS_RADAR_AIRBORNE_MODE:
      return "AIR";

    case DS_RADAR_HORIZONTAL_MODE:
      return "HORIZONTAL";

    case DS_RADAR_SUNSCAN_MODE:
      return "SUNSCAN";

    case DS_RADAR_SUNSCAN_RHI_MODE:
      return "SUNSCAN_RHI";

    case DS_RADAR_POINTING_MODE:
      return "POINTING";
      
    case DS_RADAR_FOLLOW_VEHICLE_MODE:
      return "FOLLOW_VEHICLE";

    case DS_RADAR_EL_SURV_MODE:
      return "EL_SURV";

    case DS_RADAR_MANPPI_MODE:
      return "MANPPI";

    case DS_RADAR_MANRHI_MODE:
      return "MANRHI";

    default:
      return "UNKNOWN";

  }

} 

int DsRadarSweep::str2ScanMode(const string &modeStr)

{

  if (modeStr == "CALIBRATION") {
    return DS_RADAR_CALIBRATION_MODE;
  }

  if (modeStr == "PPI_SECTOR") {
    return DS_RADAR_SECTOR_MODE;
  }

  if (modeStr == "COPLANE") {
    return DS_RADAR_COPLANE_MODE;
  }

  if (modeStr == "RHI") {
    return DS_RADAR_RHI_MODE;
  }

  if (modeStr == "VERT_POINT") {
    return DS_RADAR_VERTICAL_POINTING_MODE;
  }

  if (modeStr == "TARGET") {
    return DS_RADAR_TARGET_MODE;
  }

  if (modeStr == "MANUAL") {
    return DS_RADAR_MANUAL_MODE;
  }

  if (modeStr == "IDLE") {
    return DS_RADAR_IDLE_MODE;
  }

  if (modeStr == "SURVEILLANCE") {
    return DS_RADAR_SURVEILLANCE_MODE;
  }

  if (modeStr == "AIR") {
    return DS_RADAR_AIRBORNE_MODE;
  }

  if (modeStr == "HORIZONTAL") {
    return DS_RADAR_HORIZONTAL_MODE;
  }

  if (modeStr == "SUNSCAN") {
    return DS_RADAR_SUNSCAN_MODE;
  }

  if (modeStr == "POINTING") {
    return DS_RADAR_POINTING_MODE;
  }
      
  if (modeStr == "FOLLOW_VEHICLE") {
    return DS_RADAR_FOLLOW_VEHICLE_MODE;
  }

  if (modeStr == "EL_SURV") {
    return DS_RADAR_EL_SURV_MODE;
  }

  if (modeStr == "MANPPI") {
    return DS_RADAR_MANPPI_MODE;
  }

  if (modeStr == "MANRHI") {
    return DS_RADAR_MANRHI_MODE;
  }

  if (modeStr == "SUNSCAN_RHI") {
    return DS_RADAR_SUNSCAN_RHI_MODE;
  }

  return DS_RADAR_UNKNOWN_MODE;

} 

