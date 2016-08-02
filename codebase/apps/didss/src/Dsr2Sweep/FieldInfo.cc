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
// FieldInfo.cc
//
// Field info object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2001
//
///////////////////////////////////////////////////////////////

#include "FieldInfo.hh"

using namespace std;

/////////////////////////////////////////////////////////
// FieldInfo constructor

FieldInfo::FieldInfo(const string &dsr_name,
		     const string &name,
		     const int units,
                     int byte_width) :

  dsrName(dsr_name),
  name(name),
  units(units),
  dsrIndex(-1),
  inputByteWidth(byte_width),
  inputMissingValue(0),
  inputScale(1.0),
  inputBias(0.0),
  outputByteWidth(byte_width),
  outputMissingValue(0),
  outputScale(1.0),
  outputBias(0.0)

{
  
}

  int inputByteWidth;
  int inputMissingValue;
  double inputScale;
  double inputBias;

  int outputByteWidth;
  int outputMissingValue;
  double outputScale;
  double outputBias;

void FieldInfo::print(ostream &out) const

{

  out << "============ FieldInfo ============" << endl;
  out << "  dsrName: " << dsrName << endl;
  out << "  name: " << name << endl;
  out << "  units: " << units << endl;
  out << "  dsrIndex: " << dsrIndex << endl;
  out << "  inputByteWidth: " << inputByteWidth << endl;
  out << "  inputMissingValue: " << inputMissingValue << endl;
  out << "  inputScale: " << inputScale << endl;
  out << "  inputBias: " << inputBias << endl;
  out << "  outputByteWidth: " << outputByteWidth << endl;
  out << "  outputMissingValue: " << outputMissingValue << endl;
  out << "  outputScale: " << outputScale << endl;
  out << "  outputBias: " << outputBias << endl;
  out << "===================================" << endl;

}


