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
// StormPrecipProduct.cc
//
/////////////////////////////////////////////////////////////

#include "StormPrecipProduct.hh"

StormPrecipProduct::StormPrecipProduct( FILE *filePtr, bool byteSwapFlag, 
					bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

StormPrecipProduct::~StormPrecipProduct ()
{
  
}

fl32 StormPrecipProduct::convertData(ui08 x)
{
  //
  // Returns a floating point physical value given a byte.
  //
  double val = double(x)*double(_increment) + double(_minDataVal);

  return val/100.0; // Go from hundredths of an inch to inches
}

void StormPrecipProduct::calcDependentVals()
{
  Swap::swap2(&graphProdDesc.prodDependent31);
  _minDataVal = (fl32) graphProdDesc.prodDependent31;

  Swap::swap2(&graphProdDesc.prodDependent32);
  _increment = (fl32) graphProdDesc.prodDependent32;

  Swap::swap2(&graphProdDesc.prodDependent33);
  _numLevels = (fl32) graphProdDesc.prodDependent33;

  Swap::swap2(&graphProdDesc.prodDependent51);
  _isCompressed = (ui08)graphProdDesc.prodDependent51;

  memcpy(&_uncompProdSize, &graphProdDesc.prodDependent52, sizeof(ui32));
   Swap::swap4(&_uncompProdSize);

}

