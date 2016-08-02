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
///////////////////////////////////////////////////
// MdvForHt
//
// Holds data for a single height level
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// April 2015
//
///////////////////////////////////////////////////
#ifndef _MDV_FOR_HT_
#define _MDV_FOR_HT_

#include <Mdv/DsMdvx.hh>
#include "Params.hh"
class MdvxField;
using namespace std;

class MdvForHt {
  
public:

  // constructor

  MdvForHt(const Params &params);

  // destructor
  
  ~MdvForHt();
  
  // clear all

  void clear();

  // add a field
  
  void addField(MdvxField* inputField);

  // set methods
  
  void setMasterHdr(time_t validTime);

  inline void setVerticalType(const int& vt) { _verticalType = vt; }

  // get number of fields

  int getNumFields() const { return _mdvx.getNFields(); }

  // get the height of the layer
  // returns -9999 if no layer available
  
  double getHeightKm() const;

  // get data time

  time_t getDataTime() const;

  // Convert params encoding to Mdv encoding

  static Mdvx::encoding_type_t 
    mdvEncoding(Params::encoding_type_t paramEncoding);

  // get MDVX object

  DsMdvx &getMdvx() { return _mdvx; }
  const DsMdvx &getMdvx() const { return _mdvx; }
  
  // write the data

  int writeVol();

protected:
  
private:

  const Params &_params;
  DsMdvx _mdvx;
  int _verticalType;

};

#endif
