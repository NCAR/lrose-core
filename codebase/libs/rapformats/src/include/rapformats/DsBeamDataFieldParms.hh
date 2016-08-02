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
//////////////////////////////////////////////////////////////
//
// DsBeamDataFieldParms Low level beam array field params.
//
// This class is used by DsBeamData to convert feilds to floating
// point from raw byte formats.
//
//
/////////////////////////////////////////////////////////////
#ifndef _DsBeamDataFieldParms_HH_
#define _DsBeamDataFieldParms_HH_

#include <cstdio>
using namespace std;

class DsBeamDataFieldParms
{
public:

  /*
   * constructor:
   *
   * scale, bias = values used to convert from raw format to floating point.
   *               same as in DsFieldParams.
   *
   * is_float = true if the data is floating point (in which case scale and
   * bias are ignored.)
   *
   * bytewidth   = number of bytes per data value.
   *               same as in DsFieldParams.
   *               if 1, data is ui08.
   *               if 2, data is ui16
   *               if 4, data is fl32
   *
   * missing  = missing data value in the raw data.
   *            same as in DsFieldParams.
   *
   * fmissing = user defined float point missing data value associated
   *            with missing data. 
   * 
   * The idea is that we have a "raw" missing data value and a "scaled data"
   * missing value.
   */
  DsBeamDataFieldParms(const float scale, const float bias,
		       const bool is_float, const int missing,
		       const float fmissing);
  DsBeamDataFieldParms(const DsBeamDataFieldParms &p);
  virtual ~DsBeamDataFieldParms();
  void operator=(const DsBeamDataFieldParms &p);
  bool operator==(const DsBeamDataFieldParms &p) const;

  void print(void) const;

  // return scaled value associated with input. Input is assumed raw data
  // If input == _missing, return _fmissing.
  float scaled_value(const float v) const;

  // return raw value associated with input. Input is assumed scaled data.
  // If input == _fmissing, return _missing
  //
  // Note value returned is represented as a float, even though in some
  // cases it is actually a ui08 or ui16
  float unscaled_value(const float v) const;

  // data members are public.
  float _scale;
  float _bias;
  bool _is_float;
  int _missing;
  float _fmissing;
    
private:

};
   
#endif

