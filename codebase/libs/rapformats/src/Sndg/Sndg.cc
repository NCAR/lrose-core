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
// Sndg.cc
//
// C++ class for dealing with radar-visibility calibration.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// July 2002
//////////////////////////////////////////////////////////////


#include <rapformats/Sndg.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
using namespace std;

const fl32 Sndg::VALUE_UNKNOWN = -9999.0;
const fl32 Sndg::QC_GOOD = 1.0;
const fl32 Sndg::QC_MAYBE = 2.0;
const fl32 Sndg::QC_BAD = 3.0;
const fl32 Sndg::QC_INTERP = 4.0;
const fl32 Sndg::QC_MISSING = 9.0;

///////////////
// constructor

Sndg::Sndg()

{
  _version = 1;
  MEM_zero(_hdr);
  _hdr.version = _version;
}

/////////////
// destructor

Sndg::~Sndg()

{

}

// set methods

void Sndg::setHeader(const header_t &hdr)
{
  _hdr = hdr;
}

void Sndg::clearPoints()
{
  _points.clear();
}

void Sndg::addPoint(const point_t &pt,
		    const bool update_pt_count)
{
  _points.push_back(pt);
  if (update_pt_count)
    ++_hdr.nPoints;
}

void Sndg::setPoints(const vector< point_t > &new_points)
{
  _points = new_points;
  _hdr.nPoints = _points.size();
}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void Sndg::assemble()
  
{

  _memBuf.free();
  
  header_t hdr = _hdr;
  _header_to_BE(hdr);
  _memBuf.add(&hdr, sizeof(hdr));
  
  for (size_t ii = 0; ii < _points.size(); ii++) {
    point_t pt = _points[ii];
    _point_to_BE(pt);
    _memBuf.add(&pt, sizeof(pt));
  }
  
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the values in the object.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int Sndg::disassemble(const void *buf, int len)

{

  int minLen = (int) sizeof(header_t);
  if (len < minLen) {
    cerr << "ERROR - Sndg::disassemble" << endl;
    cerr << "  Buffer too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    return -1;
  }

  int offset = 0;
  memcpy(&_hdr, (ui08 *) buf + offset, sizeof(header_t));
  _header_from_BE(_hdr);
  offset += sizeof(header_t);
  _version = _hdr.version;

  minLen += _hdr.nPoints * sizeof(point_t);
  if (len < minLen) {
    cerr << "ERROR - Sndg::disassemble" << endl;
    cerr << "  Buffer too small for disassemble" << endl;
    cerr << "  Buf len: " << len << endl;
    cerr << "  Min len: " << minLen << endl;
    return -1;
  }

  clearPoints();
  for (int ii = 0; ii < _hdr.nPoints; ii++) {
    point_t pt;
    memcpy(&pt, (ui08 *) buf + offset, sizeof(point_t));
    _point_from_BE(pt);
    _points.push_back(pt);
    offset += sizeof(point_t);
  }
  
  return 0;

}

//////////////////////
// printing object


void Sndg::print(ostream &out, const string &spacer /* = ""*/ ) const

{

  out << "===================================" << endl;
  out << spacer << "Sndg object" << endl;
  print_header(out, spacer);
  out << spacer << "  Points:" << endl;
  for (size_t ii = 0; ii < _points.size(); ii++) {
    _print_point(_points[ii], out, spacer);
  }
  out << endl;
  
}

void Sndg::print(FILE *out, const string &spacer /* = ""*/ ) const

{
  fprintf(out, "===================================\n");
  fprintf(out,  "%sSndg object\n", spacer.c_str());
  print_header(out, spacer);
  fprintf(out, "%s  Points:\n", spacer.c_str());
  for (size_t ii = 0; ii < _points.size(); ii++) {
    _print_point(_points[ii], out, spacer);
  }
  fprintf(out, "\n");
  
}

//////////////////////
// printing header


void Sndg::print_header( ostream &out,
			 const string &spacer /* = ""*/ ) const

{

  out << "-----------------------------------" << endl;
  out << spacer << " SNDG HEADER" << endl;

  out << spacer << " launchTime: " 
      << DateTime::str(_hdr.launchTime) << endl;
  out << spacer << " nPoints: " << _hdr.nPoints << endl;
  out << spacer << " sourceId: " << _hdr.sourceId << endl;  
  out << spacer << " leadSecs: " << _hdr.leadSecs << endl;
  out << spacer << " lat: " << _hdr.lat << endl; 
  out << spacer << " lon: " << _hdr.lon << endl;
  out << spacer << " alt: " << _hdr.alt << endl;
  out << spacer << " sourceName: " << _hdr.sourceName << endl;
  out << spacer << " sourceFmt: " << _hdr.sourceFmt << endl;
  out << spacer << " siteName: " << _hdr.siteName << endl;
  out << endl;

}

void Sndg::print_header( FILE *out,
			 const string &spacer /* = ""*/ ) const

{

  fprintf(out, "-----------------------------------\n");
  fprintf(out, "%s SNDG HEADER\n", spacer.c_str());

  fprintf(out, "%s launchTime: %s\n", spacer.c_str(),
	  DateTime::str(_hdr.launchTime).c_str());
  fprintf(out, "%s nPoints: %d\n", spacer.c_str(), _hdr.nPoints);
  fprintf(out, "%s sourceId: %d\n", spacer.c_str(), _hdr.sourceId);  
  fprintf(out, "%s leadSecs: %d\n", spacer.c_str(), _hdr.leadSecs);
  fprintf(out, "%s lat: %f\n", spacer.c_str(), _hdr.lat); 
  fprintf(out, "%s lon: %f\n", spacer.c_str(), _hdr.lon);
  fprintf(out, "%s alt: %f\n", spacer.c_str(), _hdr.alt);
  fprintf(out, "%s sourceName: %s\n", spacer.c_str(), _hdr.sourceName);
  fprintf(out, "%s sourceFmt: %s\n", spacer.c_str(), _hdr.sourceFmt);
  fprintf(out, "%s siteName: %s\n", spacer.c_str(), _hdr.siteName);
  fprintf(out, "\n");

}

//////////////////////
// print point


void Sndg::_print_point(const point_t &pt,
			ostream &out, const string &spacer /* = "" */)
{

  out << "-----------------------------------" << endl;
  out << spacer << "  Sounding point" << endl;
  out << spacer << "  time (seconds since launch time): " << pt.time << endl;
  _print_value(out, spacer + "  pressure (mb): ", pt.pressure, "");
  _print_value(out, spacer + "  altitude (m): ", pt.altitude, "");
  _print_value(out, spacer + "  u (m/s): ", pt.u, "");
  _print_value(out, spacer + "  v (m/s): ", pt.v, "");
  _print_value(out, spacer + "  w (m/s): ", pt.w, "");
  _print_value(out, spacer + "  rh (%): ", pt.rh, "");
  _print_value(out, spacer + "  temp (C): ", pt.temp, "");
  _print_value(out, spacer + "  dwpt (C): ", pt.dewpt, "");
  _print_value(out, spacer + "  windSpeed (m/s): ", pt.windSpeed, "");
  _print_value(out, spacer + "  windDir (deg): ", pt.windDir, "");
  _print_value(out, spacer + "  ascensionRate(m/s): ", pt.ascensionRate, "");
  _print_value(out, spacer + "  longitude (deg): ", pt.longitude, "");
  _print_value(out, spacer + "  latitude (deg): ", pt.latitude, "");
  _print_value(out, spacer + "  pressureQC : ", pt.pressureQC, "");
  _print_value(out, spacer + "  tempQC : ", pt.tempQC, "");
  _print_value(out, spacer + "  humidityQC : ", pt.humidityQC, "");
  _print_value(out, spacer + "  uwindQC : ", pt.uwindQC, "");
  _print_value(out, spacer + "  vwindQC : ", pt.vwindQC, "");
  _print_value(out, spacer + "  ascensionRateQC: ", pt.ascensionRateQC, "");
}

void Sndg::_print_point(const point_t &pt,
			FILE *out, const string &spacer /* = "" */)
{

  fprintf(out, "-----------------------------------\n");
  fprintf(out, "%s  Sounding point\n", spacer.c_str());
  fprintf(out, "%s  time (seconds since launch time): %f\n",
	  spacer.c_str(), pt.time);
  _print_value(out, spacer + "  pressure (mb): ", pt.pressure, "");
  _print_value(out, spacer + "  altitude (m): ", pt.altitude, "");
  _print_value(out, spacer + "  u (m/s): ", pt.u, "");
  _print_value(out, spacer + "  v (m/s)): ", pt.v, "");
  _print_value(out, spacer + "  w (m/s): ", pt.w, "");
  _print_value(out, spacer + "  rh (percent): ", pt.rh, "");
  _print_value(out, spacer + "  temp (C): ", pt.temp, "");
  _print_value(out, spacer + "  dwpt (C): ", pt.dewpt, "");
  _print_value(out, spacer + "  windSpeed (m/s): ", pt.windSpeed, "");
  _print_value(out, spacer + "  windDir (deg): ", pt.windDir, "");
  _print_value(out, spacer + "  ascensionRate(m/s): ", pt.ascensionRate, "");
  _print_value(out, spacer + "  longitude (deg): ", pt.longitude, "");
  _print_value(out, spacer + "  latitude (deg): ", pt.latitude, "");
  _print_value(out, spacer + "  pressureQC : ", pt.pressureQC, "");
  _print_value(out, spacer + "  tempQC : ", pt.tempQC, "");
  _print_value(out, spacer + "  humidityQC : ", pt.humidityQC, "");
  _print_value(out, spacer + "  uwindQC : ", pt.uwindQC, "");
  _print_value(out, spacer + "  vwindQC : ", pt.vwindQC, "");
  _print_value(out, spacer + "  ascensionRateQC: ", pt.ascensionRateQC, "");
}

/////////////////
// byte swapping

void Sndg::_header_to_BE(header_t &hdr)
{
  BE_from_array_32(&hdr, HDR_NBYTES_32);
}

void Sndg::_header_from_BE(header_t &hdr)
{
  BE_to_array_32(&hdr, HDR_NBYTES_32);
}

void Sndg::_point_to_BE(point_t &pt)
{
  BE_from_array_32(&pt, PT_NBYTES_32);
}

void Sndg::_point_from_BE(point_t &pt)
{
  BE_to_array_32(&pt, PT_NBYTES_32);
}

