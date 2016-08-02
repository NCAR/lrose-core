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
// OutField.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#ifndef OutField_HH
#define OutField_HH

#include <vector>
#include <iostream>
#include "Params.hh"
#include "CP2Net.hh"
#include <rapformats/DsRadarMsg.hh>
using namespace std;

////////////////////////
// This class

class OutField {
  
public:
  
  // constructor
  
  OutField(const Params &params);
  
  // destructor
  
  ~OutField();
  
  // Set the values
  
  int init(const string &field_name,
	   int dsr_field_num,
	   int cp2_field_id,
	   const DsRadarParams &radar_params,
	   const DsFieldParams &field_params,
	   const DsRadarBeam &radar_beam,
	   long long beam_num);
  
  // print

  void print(ostream &out);

  // data

  string fieldName;
  int dsrFieldNum;
  int cp2FieldId;
  CP2Net::CP2ProductHeader hdr;
  double *data;

  static const double missing;

protected:
private:

  const Params &_params;

};

#ifdef __in_outfield_cc__
const double OutField::missing = -9999;
#endif

#endif

