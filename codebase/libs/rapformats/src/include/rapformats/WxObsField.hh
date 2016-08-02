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
// WxObsField.hh
//
// Field class used by WxObs class
// Private class, not intended for public use.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Feb 2007
//////////////////////////////////////////////////////////////

#ifndef _WxObsField_hh
#define _WxObsField_hh

#include <iostream>
#include <string>
#include <vector>
using namespace std;

/////////////////////////////////////
// class for observation measurements

class WxObsMeasurement {
  friend class WxObsField;
  friend class WxObs;
public:
  WxObsMeasurement();
  WxObsMeasurement(double value, const string &info = "");
private:
protected:
  double _value;
  double _qualifier;
  string _qual_label;
  string _info;
};

// observation field class

class WxObsField {

public:

  friend class WxObs;

  WxObsField();
  ~WxObsField();
  
  // clear measurements vector
  
  void clear();
  
  // add a value
  // returns index of value in measurement vector
  
  int addValue(double value);
  
  // set value, given the index position
  
  void setValue(double value, int index = 0);
  
  // set qualifier
  
  void setQualifier(double qualifier, int index = 0);
  
  // set qualifier label
  
  void setQualLabel(const string &qual_label, int index = 0);
  
  // set info
  
  void setInfo(const string &info, int index = 0);
  
  // get number of values
  
  inline int getSize() const { return (int) _measurements.size(); }
  
  // get a value
  
  inline double getValue(int index = 0) const {
    if ((int) _measurements.size() < index + 1) {
      return missing;
    }
    return _measurements[index]._value;
  }
  
  // get a qualifier
  
  inline double getQualifier(int index = 0) const {
    if ((int) _measurements.size() < index + 1) {
      return missing;
    }
    return _measurements[index]._qualifier;
  }
  
  // get a qualifier label
  
  inline string getQualLabel(int index = 0) const {
    if ((int) _measurements.size() < index + 1) {
      return "";
    }
    return _measurements[index]._qual_label;
  }
  
  // get info
  
  inline string getInfo(int index = 0) const {
    if ((int) _measurements.size() < index + 1) {
      return "";
    }
    return _measurements[index]._info;
  }
  
  // print all entries
  
  void print(ostream &out, const string &label, string spacer = "") const;

  // missing value
  
  static const double missing;

protected:    
private:

  // vector of measurements

  vector<WxObsMeasurement> _measurements;
  
  // check size
  
  void _checkSize(int index);
  
};

#ifdef _in_wx_obs_field_cc
const double WxObsField::missing = -9999.0;
#endif

#endif


