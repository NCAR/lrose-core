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
// 
// SpecDiffPhase.cc
//
/////////////////////////////////////////////////////////////

#include "SpecDiffPhaseProduct.hh"


SpecDiffPhaseProduct::SpecDiffPhaseProduct(FILE *filePtr, 
			       bool byteSwapFlag, bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

SpecDiffPhaseProduct::~SpecDiffPhaseProduct ()
{
  
}

fl32 SpecDiffPhaseProduct::convertData(ui08 x)
{
  //
  // Returns a floating point physical value given a byte.
  //
  double val=0.0;
  
  val = (double(x)-_bias)/_scale;
 
  return val;
}

void SpecDiffPhaseProduct::calcDependentVals()
{

  //
  // Find elevation angle
  //
  Swap::swap2(&graphProdDesc.prodDependent30);

  _elevAngle = (fl32) graphProdDesc.prodDependent30 *.1;

  //
  // Find scale and bias 
  //
  memcpy(&_scale, &graphProdDesc.prodDependent31, 4);
  Swap::swap4((void*)&_scale);

  memcpy(&_bias, &graphProdDesc.prodDependent33, 4);
  Swap::swap4((void*)&_bias);
 
  //
  // Other product dependent members
  //
  Swap::swap2(&graphProdDesc.prodDependent36);
  _maxVal = (fl32) graphProdDesc.prodDependent36;  

  Swap::swap2(&graphProdDesc.prodDependent37);
  _leadingFlags = (fl32) graphProdDesc.prodDependent37;  

  Swap::swap2(&graphProdDesc.prodDependent38);
  _trailingFlags = (fl32) graphProdDesc.prodDependent38;  

  Swap::swap2(&graphProdDesc.prodDependent47);
  _minDataVal = (fl32) graphProdDesc.prodDependent47 * .05;  

  Swap::swap4(&graphProdDesc.prodDependent48);
  _maxDataVal = (fl32) graphProdDesc.prodDependent48 * .05;

 
  Swap::swap2(&graphProdDesc.prodDependent51);
  _isCompressed = (ui08)graphProdDesc.prodDependent51;

  memcpy(&_uncompProdSize, &graphProdDesc.prodDependent52, sizeof(ui32));
  Swap::swap4(&_uncompProdSize);
}

