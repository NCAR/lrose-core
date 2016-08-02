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
///////////////////////////////////////////////////////////////
// VarTransform.cc
//
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2014
//
///////////////////////////////////////////////////////////////
//
// Handles variable linear transformation for fields
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cmath>
#include <cassert>
#include "VarTransform.hh"
using namespace std;

// Constructor

VarTransform::VarTransform(const string &fieldName,
                           Params::variable_transform_control_t control,
                           const string &xmlTag,
                           const string &lutStr) :
        _isOK(true),
        _fieldName(fieldName),
        _control(control),
        _xmlTag(xmlTag),
        _lutStr(lutStr)
        
{

  // parse the lookup table

  size_t pos = 0;
  while (pos < _lutStr.size()) {
    
    pos = _lutStr.find('(', pos);
    if (pos == string::npos) {
      break;
    }
    size_t start = pos;

    pos = _lutStr.find(')', start);
    if (pos == string::npos) {
      break;
    }
    size_t end = pos;
    string entry = _lutStr.substr(start + 1, end - start - 1);

    double val, scale, offset;
    if (sscanf(entry.c_str(), "%lg,%lg,%lg", &val, &scale, &offset) == 3) {
      LutPoint pt(val, scale, offset);
      _lut.push_back(pt);
    }

  } // while

  if (_lut.size() < 2) {
    cerr << "ERROR - VarTransform constructor" << endl;
    cerr << "  Too few entries" << endl;
    cerr << "  fieldName: " << _fieldName << endl;
    cerr << "  lutStr: " << _lutStr << endl;
    _isOK = false;
  }

  for (size_t ii = 1; ii < _lut.size(); ii++) {
    double delta = _lut[ii].getVal() - _lut[ii-1].getVal();
    if (delta < 0) {
      cerr << "ERROR - VarTransform constructor" << endl;
      cerr << "  Values not monotically increasing" << endl;
      cerr << "  fieldName: " << _fieldName << endl;
      cerr << "  lutStr: " << _lutStr << endl;
      cerr << "  index: " << ii << endl;
      cerr << "  _lut[index-1]: " << _lut[ii-1].getVal() << endl;
      cerr << "  _lut[index]: " << _lut[ii].getVal() << endl;
      cerr << "  delta: " << delta << endl;
      _isOK = false;
    }
  }
  
}

// destructor

VarTransform::~VarTransform()
  
{

}

///////////////////////////////////////////////////////////
// get scale and offset for a given val

void VarTransform::getCoeffs(double val, double &scale, double &offset) const

{

  // initialize

  scale = _lut[0].getScale();
  offset = _lut[0].getOffset();

  // check for value below lut range

  if (val <= _lut[0].getVal()) {
    scale = _lut[0].getScale();
    offset = _lut[0].getOffset();
    return;
  }

  // check for value above lut range

  size_t nn = _lut.size();
  if (val >= _lut[nn-1].getVal()) {
    scale = _lut[nn-1].getScale();
    offset = _lut[nn-1].getOffset();
    return;
  }

  for (size_t ii = 1; ii < _lut.size(); ii++) {
    double lower = _lut[ii-1].getVal();
    double upper = _lut[ii].getVal();
    if (val >= lower && val <= upper) {
      double delta = upper - lower;
      double frac = (val - lower) / delta;
      scale = _lut[ii-1].getScale() +
        frac * (_lut[ii].getScale() - _lut[ii-1].getScale());
      offset = _lut[ii-1].getOffset() +
        frac * (_lut[ii].getOffset() - _lut[ii-1].getOffset());
      return;
    }
  }

}

///////////////////////////////////////////////////////////
// Print

void VarTransform::print(ostream &out)
  
{

  out << "=============================================" << endl;
  out << "VarTransform - transforming field" << endl;
  out << "  Field name: " << _fieldName << endl;
  if (_control == Params::STATUS_XML_FIELD) {
    out << "  Control: STATUS_XML_FIELD" << endl;
    out << "  xmlTag: " << _xmlTag << endl;
  // } else if (_control == Params::ELEVATION_DEG) {
  //   out << "  Control: ELEVATION_DEG" << endl;
  // } else if (_control == Params::PULSE_WIDTH_US) {
  //   out << "  Control: PULSE_WIDTH_US" << endl;
  }
  out << "    lut size: " << _lut.size() << endl;
  for (size_t ii = 0; ii < _lut.size(); ii++) {
    cerr << "    lut ii, val, scale, offset: "
         << ii << ", "
         << _lut[ii].getVal()<< ", "
         << _lut[ii].getScale()<< ", "
         << _lut[ii].getOffset()<< endl;
  }
  out << "=============================================" << endl;
  
}
