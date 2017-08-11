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
// VarTransform.hh
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

#ifndef VarTransform_hh
#define VarTransform_hh

#include <string>
#include <vector>
#include "Params.hh"
using namespace std;

class VarTransform {
  
public:

  class LutPoint {
  public:
    LutPoint(double val, double scale, double offset) {
      _val = val;
      _scale = scale;
      _offset = offset;
    }
    inline double getVal() const { return _val; }
    inline double getScale() const { return _scale; }
    inline double getOffset() const { return _offset; }
  private:
    double _val;
    double _scale;
    double _offset;
  };

  VarTransform(const string &fieldName,
               Params::variable_transform_control_t control,
               const string &xmlTag,
               const string &lutStr);

  ~VarTransform();

  // get methods

  bool isOK() const { return _isOK; }
  inline Params::variable_transform_control_t getControl() const { return _control; }
  inline const string &getFieldName() const { return _fieldName; }
  inline const string &getXmlTag() const { return _xmlTag; }
  
  // get scale and offset for a given val

  void getCoeffs(double val, double &scale, double &offset) const;
  
  // Print
  
  void print(ostream &out);
  
protected:
private:

  int _isOK;
  string _fieldName;
  Params::variable_transform_control_t _control;
  const string _xmlTag;
  string _lutStr;
  vector<LutPoint> _lut;

};

#endif

