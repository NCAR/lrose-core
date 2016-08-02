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
// WxObsField.cc
//
// Field class used by WxObs class
// Private class, not intended for public use.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Feb 2007
//////////////////////////////////////////////////////////////

#define _in_wx_obs_field_cc
#include <rapformats/WxObsField.hh>

// constructor

WxObsField::WxObsField() {
}

// destructor

WxObsField::~WxObsField() {
}

// clear measurements vector

void WxObsField::clear() {
  _measurements.clear();
}

// add a value
// returns index of value in measurement vector

int WxObsField::addValue(double value) {
  int index = (int) _measurements.size();
  setValue(value, index);
  return index;
}
  
// set value, given the index position

void WxObsField::setValue(double value, int index /* = 0 */) {
  if (index < 0) return;
  _checkSize(index);
  _measurements[index]._value = value;
}

// set qualifier

void WxObsField::setQualifier(double qualifier, int index /* = 0 */) {
  if (index < 0) return;
  _checkSize(index);
  _measurements[index]._qualifier = qualifier;
}

// set qualifier label

void WxObsField::setQualLabel(const string &qual_label, int index /* = 0 */) {
  if (index < 0) return;
  _checkSize(index);
  _measurements[index]._qual_label = qual_label;
}

// set info

void WxObsField::setInfo(const string &info, int index /* = 0 */) {
  if (index < 0) return;
  _checkSize(index);
  _measurements[index]._info = info;
}

// print all entries

void WxObsField::print(ostream &out,
                       const string &label,
                       string spacer /* = "" */) const {
  for (int ii = 0; ii < (int) _measurements.size(); ii++) {
    out << spacer << label;
    if ( _measurements.size() > 1) {
      out << "[" << (ii) << "]";
    }
    out << ": " << _measurements[ii]._value << endl;
    if (_measurements[ii]._qualifier != missing) {
      out << spacer << "  For " << _measurements[ii]._qual_label << ": "
          << _measurements[ii]._qualifier << endl;
    }
    if (_measurements[ii]._info.size() > 0) {
      out << spacer << "  Info: " << _measurements[ii]._info << endl;
    }
  } // ii
}

////////////////////////
// check size
// increase as necessary

void WxObsField::_checkSize(int index) {
  if ((int) _measurements.size() < index + 1) {
    int nadd = index - (int) _measurements.size() + 1;
    for (int ii = 0; ii < nadd; ii++) {
      WxObsMeasurement meas;
      _measurements.push_back(meas);
    }
  }
}


////////////////////
// measurement class

WxObsMeasurement::WxObsMeasurement() {
  _value = WxObsField::missing;
  _qualifier = WxObsField::missing;
}

WxObsMeasurement::WxObsMeasurement(double value,
                                   const string &info /* = "" */) {
  _value = value;
  _info = info;
}


