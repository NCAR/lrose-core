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
// VilProduct.cc
//
/////////////////////////////////////////////////////////////

#include "VilProduct.hh"

VilProduct::VilProduct( FILE *filePtr, bool byteSwapFlag, 
					bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
 
}

VilProduct::~VilProduct ()
{
  
}

fl32 VilProduct::convertData(ui08 x)
{
  // 
  // Returns a floating point physical value given a byte.
  // This is a rather involved encoding scheme.
  //
  double val=0.0;
  if (x < _decodeThresh){
    val = (double(x)-_bias1)/_scale1;
  } else {
    double y = (double(x)-_bias2)/_scale2;
    val = exp(y);
  }

  return val;
}

void VilProduct::calcDependentVals()
{
  //
  // Decode half words into floats
  //
  _scale1 = _decode_fl32(graphProdDesc.prodDependent31);

  _bias1 = _decode_fl32(graphProdDesc.prodDependent32);

  _scale2 = _decode_fl32(graphProdDesc.prodDependent34);

  _bias2 = _decode_fl32(graphProdDesc.prodDependent35);
  
  Swap::swap2(&graphProdDesc.prodDependent33);
   
  _decodeThresh = graphProdDesc.prodDependent33;

  Swap::swap2(&graphProdDesc.prodDependent51);
  _isCompressed = (ui08)graphProdDesc.prodDependent51;

  memcpy(&_uncompProdSize, &graphProdDesc.prodDependent52, sizeof(ui32));
  Swap::swap4(&_uncompProdSize);


}

//
// Decode the NOAA halfword method of storing a float
fl32  VilProduct::_decode_fl32(ui16 hw){

  // break out the two bytes
  unsigned char *p = (unsigned char *) &hw;
  unsigned char b0 = *p;
  unsigned char b1 = *(p+1);

  // If the MSB of b0 is set, the result is negative.
  int s=0;
  if (b0 > 127){
    s = 1;
    b0 = b0 - 128;
  }

  // Integer division gives the exponent, e
  int e = b0 / 4;
 
  // The remainer of that division is actually the msb
  // (top two bits) of the data
  int msb = b0 % 4;

  int f = 256*(int)msb + b1;

  double fact = pow(2.0, e-16);
  if (s) fact = -fact;
  double ans = fact*(1.0+(double(f)/1024.0));

  return ans;
}
