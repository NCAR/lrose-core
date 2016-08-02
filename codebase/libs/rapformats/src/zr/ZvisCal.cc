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
// ZvisCal.cc
//
//  C++ class for dealing with z-v probability calibration
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// July 2006
//////////////////////////////////////////////////////////////

#include <rapformats/ZvisCal.hh>
#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/str.h>
#include <toolsa/TaXml.hh>
#include <toolsa/TaArray.hh>
#include <iomanip>
#include <cmath>
using namespace std;

///////////////
// constructor

ZvisCal::ZvisCal()
  
{
  
  _versionNum = 1;
  clear();

}

/////////////
// destructor

ZvisCal::~ZvisCal()

{

}

//////////////////////////
// clear all data members

void ZvisCal::clear()

{

  _startTime = ZvisCal::MISSING_VAL;
  _endTime = ZvisCal::MISSING_VAL;
  _calibrationPeriod = ZvisCal::MISSING_VAL;
  _surfaceWindAveragingPeriod = ZvisCal::MISSING_VAL;
  _trecWindAveragingPeriod = ZvisCal::MISSING_VAL;
  _trecWindKernelRadius = ZvisCal::MISSING_VAL;
  _minFractionTrecWindKernel = ZvisCal::MISSING_VAL;
  _trecWindWeight = ZvisCal::MISSING_VAL;
  _fallTime = ZvisCal::MISSING_VAL;
  _minQuantile = ZvisCal::MISSING_VAL;
  _maxQuantile = ZvisCal::MISSING_VAL;
  _deltaQuantile = ZvisCal::MISSING_VAL;
  _calSlope = -0.0667;
  _calIntercept = 2.2;
  _calCorr = 0.0;
  _cal.clear();

}

//////////////////////////
// add a quantile entry

void ZvisCal::addEntry(double quantile, double dbz, double viskm)

{
  cal_entry_t entry;
  entry.quantile = quantile;
  entry.dbz = dbz;
  entry.viskm = viskm;
  _cal.push_back(entry);
}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.

void ZvisCal::assemble()
  
{

  // print object to string as XML

  string xml;

  xml += TaXml::writeStartTag("ZvisCal", 0);

  xml += TaXml::writeInt("version", 1, _versionNum);
  xml += TaXml::writeUtime("start_time", 1, _startTime);
  xml += TaXml::writeUtime("end_time", 1, _endTime);
  xml += TaXml::writeInt("calibration_period", 1, _calibrationPeriod);
  xml += TaXml::writeInt("surface_wind_averaging_period",
                         1, _surfaceWindAveragingPeriod);
  xml += TaXml::writeInt("trec_wind_averaging_period",
                         1, _trecWindAveragingPeriod);
  xml += TaXml::writeDouble("trec_wind_kernel_radius",
                            1, _trecWindKernelRadius);
  xml += TaXml::writeDouble("min_fraction_trec_wind_kernel",
                            1, _minFractionTrecWindKernel);
  xml += TaXml::writeDouble("trec_wind_weight", 1, _trecWindWeight);
  xml += TaXml::writeInt("fall_time", 1, _fallTime);
  xml += TaXml::writeDouble("min_quantile", 1, _minQuantile);
  xml += TaXml::writeDouble("max_quantile", 1, _maxQuantile);
  xml += TaXml::writeDouble("delta_quantile", 1, _deltaQuantile);

  xml += TaXml::writeStartTag("cal_array", 1);
  for (int ii = 0; ii < (int) _cal.size(); ii++) {
    xml += TaXml::writeStartTag("cal_entry", 2);
    xml += TaXml::writeDouble("quantile", 3, _cal[ii].quantile);
    xml += TaXml::writeDouble("dbz", 3, _cal[ii].dbz);
    xml += TaXml::writeDouble("viskm", 3, _cal[ii].viskm);
    xml += TaXml::writeEndTag("cal_entry", 2);
  }
  xml += TaXml::writeEndTag("cal_array", 1);

  xml += TaXml::writeDouble("cal_intercept", 1, _calIntercept);
  xml += TaXml::writeDouble("cal_slope", 1, _calSlope);
  xml += TaXml::writeDouble("cal_corr", 1, _calCorr);

  xml += TaXml::writeEndTag("ZvisCal", 0);
  
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

int ZvisCal::disassemble(const void *buf, int len)

{

  // make copy of buffer, make sure it is null terminated

  TaArray<char> copyBuf;
  char *copy = copyBuf.alloc(len + 1);
  memcpy(copy, buf, len);
  copy[len] = '\0';

  // remove comments
  
  string xmlBuf = TaXml::removeComments(copy);
  
  // clear state

  clear();
  
  // set state from the XML

  int iret = 0;

  TaXml::readInt(xmlBuf, "version", _versionNum);
  
  if (TaXml::readTime(xmlBuf, "start_time", _startTime)) {
    cerr << "  Cannot read start_time" << endl;
    iret = -1;
  }
  if (TaXml::readTime(xmlBuf, "end_time", _endTime)) {
    cerr << "  Cannot read end_time" << endl;
    iret = -1;
  }
  
  if (TaXml::readInt(xmlBuf, "calibration_period", _calibrationPeriod)) {
    cerr << "  Cannot read calibration_period" << endl;
    iret = -1;
  }

  TaXml::readInt(xmlBuf, "surface_wind_averaging_period",
                 _surfaceWindAveragingPeriod);
  TaXml::readInt(xmlBuf, "trec_wind_averaging_period",
                 _trecWindAveragingPeriod);
  TaXml::readDouble(xmlBuf, "trec_wind_kernel_radius",
                    _trecWindKernelRadius);
  TaXml::readDouble(xmlBuf, "min_fraction_trec_wind_kernel",
                    _minFractionTrecWindKernel);
  TaXml::readDouble(xmlBuf, "trec_wind_weight", _trecWindWeight);
  TaXml::readInt(xmlBuf, "fall_time", _fallTime);

  if (TaXml::readDouble(xmlBuf, "min_quantile", _minQuantile)) {
    cerr << "  Cannot read min_quantile" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "max_quantile", _maxQuantile)) {
    cerr << "  Cannot read max_quantile" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "delta_quantile", _deltaQuantile)) {
    cerr << "  Cannot read delta_quantile" << endl;
    iret = -1;
  }

  string arrayStr;
  if (TaXml::readString(xmlBuf, "cal_array", arrayStr)) {
    cerr << "  Cannot read cal_array" << endl;
    iret = -1;
  }

  if (iret) {
    cerr << "ERROR -  ZvisCal::disassemble" << endl;
    cerr << "XML buffer: " << endl;
    cerr << copy << endl;
    return -1;
  }

  vector<string> entries;
  TaXml::readStringArray(arrayStr, "cal_entry", entries);
  if (entries.size() == 0) {
    cerr << "ERROR - ZvisCal::disassemble" << endl;
    cerr << "  No cal_entry entries" << endl;
    iret = -1;
  }

  for (int ii = 0; ii < (int) entries.size(); ii++) {
    
    const string &entryStr = entries[ii];
    cal_entry_t entry;
    double val;
    
    if (TaXml::readDouble(entryStr, "quantile", val)) {
      cerr << "ERROR - ZvisCal::disassemble" << endl;
      cerr << "  Cannot read quantile" << endl;
      cerr << "  Entry: " << entryStr << endl;
      iret = -1;
    } else {
      entry.quantile = val;
    }
    
    if (TaXml::readDouble(entryStr, "dbz", val)) {
      cerr << "ERROR - ZvisCal::disassemble" << endl;
      cerr << "  Cannot read dbz" << endl;
      cerr << "  Entry: " << entryStr << endl;
      iret = -1;
    } else {
      entry.dbz = val;
    }

    if (TaXml::readDouble(entryStr, "viskm", val)) {
      cerr << "ERROR - ZvisCal::disassemble" << endl;
      cerr << "  Cannot read viskm" << endl;
      cerr << "  Entry: " << entryStr << endl;
      iret = -1;
    } else {
      entry.viskm = val;
    }

    _cal.push_back(entry);
    
  } // ii
  
  if (TaXml::readDouble(xmlBuf, "cal_intercept", _calIntercept)) {
    cerr << "  Cannot read cal_intercept" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "cal_slope", _calSlope)) {
    cerr << "  Cannot read cal_slope" << endl;
    iret = -1;
  }
  if (TaXml::readDouble(xmlBuf, "cal_corr", _calCorr)) {
    cerr << "  Cannot read cal_corr" << endl;
    iret = -1;
  }

  if (iret) {
    cerr << "ERROR -  ZvisCal::disassemble" << endl;
    cerr << "XML buffer: " << endl;
    cerr << copy << endl;
    return -1;
  }

  return 0;

}

//////////////////////
// printing object


void ZvisCal::print(ostream &out, string spacer /* = ""*/ ) const

{

  out << "=============================================" << endl;
  out << spacer << "Z-V probability-based calibration" << endl;
  out << spacer << "Version number: " << _versionNum << endl;
  out << spacer << "  startTime: " << DateTime::strm(_startTime) << endl;
  out << spacer << "  endTime: " << DateTime::strm(_endTime) << endl;

  out << spacer << "  calibrationPeriod: "
      << _calibrationPeriod << endl;
  out << spacer << "  surfaceWindAveragingPeriod: "
      << _surfaceWindAveragingPeriod << endl;
  out << spacer << "  trecWindAveragingPeriod: "
      << _trecWindAveragingPeriod << endl;
  out << spacer << "  trecWindKernelRadius: "
      << _trecWindKernelRadius << endl;
  out << spacer << "  minFractionTrecWindKernel: "
      << _minFractionTrecWindKernel << endl;

  out << spacer << "  trecWindWeight: " << _trecWindWeight << endl;
  out << spacer << "  fallTime: " << _fallTime << endl;
  out << spacer << "  minQuantile: " << _minQuantile << endl;
  out << spacer << "  maxQuantile: " << _maxQuantile << endl;
  out << spacer << "  deltaQuantile: " << _deltaQuantile << endl;

  out << spacer << "  Calibration array:" << endl;
  out << spacer << "    quantile, dbz, viskm:" << endl;
  for (int ii = 0; ii < (int) _cal.size(); ii++) {
    out << spacer
        << "    " << setw(4) << _cal[ii].quantile
        << " " << setw(10) << _cal[ii].dbz
        << " " << setw(10) << _cal[ii].viskm << endl;
  }
  out << spacer << "  calIntercept: " << _calIntercept << endl;
  out << spacer << "  calSlope: " << _calSlope << endl;
  out << spacer << "  calCorr: " << _calCorr << endl;

  out << endl;
  
}

/////////////////////////////////////////////////
// get an estimated viskm value given a dBZ value

double ZvisCal::getVisKm(double dbz) const

{

  double logVis = _calIntercept + _calSlope * dbz;
  double viskm = pow(10.0, logVis);

  return viskm;

}

/////////////////////////////////////////////////
// compute the fit of vis vs dbz, using the values
// in the quantiles from 0.1 to 0.9
//
// returns 0 on success, -1 on failure

int ZvisCal::computeFit()

{

  // compute number of points for quantiles within the
  // range of 0.1 to 0.9 inclusive

  double sumx = 0.0, sumx2 = 0.0;
  double sumy = 0.0, sumy2 = 0.0, sumxy = 0.0;
  double dn = 0;

  for (int ii = 0; ii < (int) _cal.size(); ii++) {
    if (_cal[ii].quantile >= 0.1 && _cal[ii].quantile <= 0.9 &&
        _cal[ii].viskm > 0) {
      double xx = _cal[ii].dbz;
      double yy = log10(_cal[ii].viskm);
      sumx += xx;
      sumx2 += xx * xx;
      sumy += yy;
      sumy2 += yy * yy;
      sumxy += xx * yy;
      dn++;
    }
  }
  if (dn < 10) {
    return -1;
  }

  // compute the terms

  double term1 = dn * sumx2  - sumx * sumx;
  double term2 = sumy * sumx2 - sumx * sumxy;
  double term3 = dn * sumxy - sumx * sumy;
  double term4 = (dn * sumx2 - sumx * sumx);
  double term5 = (dn * sumy2 - sumy * sumy);

  // compute regression parameters

  _calIntercept = term2 / term1;
  _calSlope = term3 / term1;
  _calCorr = fabs(term3 / sqrt(fabs(term4 * term5)));

  return 0;

}

