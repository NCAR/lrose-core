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

/**
 * @file CorrCoef.cc
 * @brief Source for CorrCoef class
 */

#include "CorrCoef.hh"

CorrCoef::CorrCoef(FILE *filePtr, 
		   bool byteSwapFlag, bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

CorrCoef::~CorrCoef ()
{
  
}

fl32 CorrCoef::convertData(ui08 x)
{
  //
  // Returns a floating point physical value given a byte.
  //
  double val=0.0;
  
  val = (double(x)-_bias)/_scale;
 
  return val;
}

void CorrCoef::calcDependentVals()
{
  memcpy(&_scale, &graphProdDesc.prodDependent31, 4);

  memcpy(&_bias, &graphProdDesc.prodDependent33, 4);
  
  memcpy(&_uncompProdSize, &graphProdDesc.prodDependent52, sizeof(ui32));

  if( byteSwap)
  {
    Swap::swap2(&graphProdDesc.prodDependent30);

    Swap::swap4((void*)&_scale);

    Swap::swap4((void*)&_bias);
  
    Swap::swap2(&graphProdDesc.prodDependent36);

    Swap::swap2(&graphProdDesc.prodDependent37);

    Swap::swap2(&graphProdDesc.prodDependent38);

    Swap::swap2(&graphProdDesc.prodDependent47);
     
    Swap::swap4(&graphProdDesc.prodDependent48);

    Swap::swap2(&graphProdDesc.prodDependent51);

    Swap::swap4(&_uncompProdSize);
  }

  _elevAngle = (fl32) graphProdDesc.prodDependent30 *.1;

  _max = (fl32) graphProdDesc.prodDependent36;  
  
  _leadingFlags = (fl32) graphProdDesc.prodDependent37;  
 
  _trailingFlags = (fl32) graphProdDesc.prodDependent38;  

  _minDataVal = (fl32) graphProdDesc.prodDependent47 * .00333;  
  
  _maxDataVal = (fl32) graphProdDesc.prodDependent48 * .00333;
 
  _isCompressed = (ui08)graphProdDesc.prodDependent51;
}

void CorrCoef::printProdDependentVals()
{
  cerr << "Product dependent values: " << endl;

  cerr << " elevAngle " <<  _elevAngle << endl;
  
  cerr << " scale  " << _scale << endl;
  
  cerr << " bias  " << _bias << endl;
  
  cerr << " max val " << _max  << endl;
  
  cerr << " leadingFlags " << _leadingFlags  <<endl;
  
  cerr << " trailing values " << _trailingFlags << endl;
  
  cerr << " min correlation coeff " << _minDataVal << endl;

  cerr << " max correlation coeff " << _maxDataVal << endl;
  
  cerr << " compression  " << _isCompressed << endl;
  
  cerr << " size uncompressed product " << _uncompProdSize <<  endl;
}
