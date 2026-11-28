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
// DiffRefl.cc
//
/////////////////////////////////////////////////////////////

#include "DiffRefl.hh"


DiffRefl::DiffRefl(FILE *filePtr, 
			       bool byteSwapFlag, bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

DiffRefl::~DiffRefl ()
{
  
}

fl32 DiffRefl::convertData(ui08 x)
{
  //
  // Returns a floating point physical value given a byte.
  //
  double val=0.0;
  
  val = (double(x)-_bias)/_scale;
 
  return val;
}

void DiffRefl::calcDependentVals()
{

  //
  // Find elevation angle
  //
  swap.swap2(&graphProdMsg.prodDependent30);

  _elevAngle = (fl32) graphProdMsg.prodDependent30 *.1;

  //
  // Find scale and bias 
  //
  memcpy(&_scale, &graphProdMsg.prodDependent31, 4);
  swap.swap4((void*)&_scale);

  memcpy(&_bias, &graphProdMsg.prodDependent33, 4);
  swap.swap4((void*)&_bias);
 
  //
  // Other product dependent members
  //
  swap.swap2(&graphProdMsg.prodDependent36);
  _maxVal = (fl32) graphProdMsg.prodDependent36;  

  swap.swap2(&graphProdMsg.prodDependent37);
  _leadingFlags = (fl32) graphProdMsg.prodDependent37;  

  swap.swap2(&graphProdMsg.prodDependent38);
  _trailingFlags = (fl32) graphProdMsg.prodDependent38;  

  swap.swap2(&graphProdMsg.prodDependent47);
  _minDataVal = (fl32) graphProdMsg.prodDependent47 * .1;  

  swap.swap4(&graphProdMsg.prodDependent48);
  _maxDataVal = (fl32) graphProdMsg.prodDependent48 * .1;

  swap.swap2(&graphProdMsg.prodDependent51);
  _compression = (fl32) graphProdMsg.prodDependent51;
  
  swap.swap2(&graphProdMsg.prodDependent52);
  _sizeMSW = (fl32) graphProdMsg.prodDependent52;

  swap.swap2(&graphProdMsg.prodDependent53);
  _sizeLSW = (fl32) graphProdMsg.prodDependent53;

}

