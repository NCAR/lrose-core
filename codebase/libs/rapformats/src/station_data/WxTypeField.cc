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
// WxTypeField.cc
//
// Field class used by WxObs class
// Private class, not intended for public use.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Feb 2007
//////////////////////////////////////////////////////////////

#include <rapformats/WxTypeField.hh>

// constructor

WxTypeField::WxTypeField() {
}

// destructor

WxTypeField::~WxTypeField() {
}

// clear measurements vector

void WxTypeField::clear() {
  _measurements.clear();
}

// add a value
// returns index of value in measurement vector

int WxTypeField::addValue(wx_type_t value) {
  int index = (int) _measurements.size();
  setValue(value, index);
  return index;
}

// set value, given the index position

void WxTypeField::setValue(wx_type_t value, int index /* = 0 */) {
  if (index < 0) return;
  _checkSize(index);
  _measurements[index]._value = value;
}

// set info

void WxTypeField::setInfo(const string &info, int index /* = 0 */) {
  if (index < 0) return;
  _checkSize(index);
  _measurements[index]._info = info;
}

// print all entries

void WxTypeField::print(ostream &out,
                        const string &label,
                        string spacer /* = ""*/) const {
  for (int ii = 0; ii < (int) _measurements.size(); ii++) {
    out << spacer << label;
    if ( _measurements.size() > 1) {
      out << "[" << (ii) << "]";
    }
    out << ": " << _measurements[ii]._value << endl;
    if (_measurements[ii]._info.size() > 0) {
      out << spacer << "  Info: " << _measurements[ii]._info << endl;
    }
  } // ii
}

// check size

void WxTypeField::_checkSize(int index) {
  if ((int) _measurements.size() < index + 1) {
    int nadd = index - (int) _measurements.size() + 1;
    for (int ii = 0; ii < nadd; ii++) {
      WxTypeMeasurement meas;
      _measurements.push_back(meas);
    }
  }
}

////////////////////
// measurement class

WxTypeMeasurement::WxTypeMeasurement() {
  _value = WxT_MISSING;
}

WxTypeMeasurement::WxTypeMeasurement(wx_type_t value,
                                     const string &info /* = "" */) {
  _value = value;
  _info = info;
}

