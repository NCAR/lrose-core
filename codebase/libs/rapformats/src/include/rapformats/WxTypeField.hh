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
// WxTypeField.hh
//
// Field class used by WxObs class
// Private class, not intended for public use.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Feb 2007
//////////////////////////////////////////////////////////////

#ifndef _WxTypeField_hh
#define _WxTypeField_hh

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// weather type enumerated values

typedef enum {
  WxT_RA,      /* Liquid precip - RAIN - not frozen */
  WxT_SN,      /* Frozen precip - other than hail */
  WxT_UP,      /* Unknown precip */
  WxT_FG,      /* Fog */
  WxT_DS,      /* Dust Storm */
  WxT_FZFG,    /* Freezing FOG */
  WxT_BR,      /* Mist */
  WxT_HZ,      /* Haze */
  WxT_SQ,      /* Squall */
  WxT_FC,      /* Funnel cloud, tornado, water spout */
  WxT_TS,      /* Thunderstorm */
  WxT_GR,      /* Hail */
  WxT_GS,      /* Small Hail - Deprecated */
  WxT_MFZDZ,   /* Light FZDZ (-FZDZ) */
  WxT_FZRA,    /* Freezing Rain */
  WxT_VA,      /* Volcanic Ash */
  WxT_CLR,     /* Clear Sky - Deprecated */
  WxT_FROST,   /* Frost */
  WxT_SCT,     /* Scattered Clouds */
  WxT_BKN,     /* Broken Clouds */
  WxT_OVC,     /* Overcast */
  WxT_FEW,     /* Clouds 0/8 - 2/8 */
  WxT_PE,      /* Ice Pellets */
  WxT_BLSN,    /* Blowing Snow */
  WxT_FZDZ,    /* Freezing Drizzle */
  WxT_DZ,      /* Drizzle */
  WxT_MRA,     /* Light RAIN -RA */
  WxT_PRA,     /* Heavy RAIN +RA */
  WxT_MSN,     /* Light SNOW -SN */
  WxT_PSN,     /* Heavy SNOW +SN */
  WxT_PTS,     /* Heavy Thunderstorm */
  WxT_MFZRA,   /* Light Freezing Rain*/
  WxT_PFZRA,   /* Heavy Freezing Rain*/
  WxT_SG,      /* Snow Grains - Deprecated */
  WxT_PFZDZ,   /* Heavy Freezing Drizzle (+FZDZ) */
  WxT_DU,      /* Dust */
  WxT_SA,      /* Sand */
  WxT_SS,      /* Sand storm */
  WxT_UNKNOWN, /* Unknown type */
  WxT_MISSING  /* Missing value */
} wx_type_t;

// class for measurements

class WxTypeMeasurement {
  friend class WxTypeField;
  friend class WxObs;
public:
  WxTypeMeasurement();
  WxTypeMeasurement(wx_type_t value, const string &info = "");
protected:
private:
  wx_type_t _value;
  string _info;
};
  
class WxTypeField {

public:
  
  friend class WxObs;

  WxTypeField();
  ~WxTypeField();
  
  // clear measurements vector
  
  void clear();
  
  // add a value
  // returns index of value in measurement vector
  
  int addValue(wx_type_t value);
  
  // set value, given the index position
  
  void setValue(wx_type_t value, int index = 0);
  
  // set info
  
  void setInfo(const string &info, int index = 0);
  
  // get number of values
  
  inline int getSize() const { return (int) _measurements.size(); }
  
  // get a value
  
  inline wx_type_t getValue(int index = 0) const {
    if ((int) _measurements.size() < index + 1) {
      return WxT_MISSING;
    }
    return _measurements[index]._value;
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

protected:    
private:
  
  // vector of measurements

  vector<WxTypeMeasurement> _measurements;
  
  // check size
  
  void _checkSize(int index);
  
};

#endif


