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


#include "DigitalAccumArray.hh"

DigitalAccumArray::DigitalAccumArray(FILE *filePtr, 
			       bool byteSwapFlag, bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

DigitalAccumArray::~DigitalAccumArray ()
{
  
}

fl32 DigitalAccumArray::convertData(ui08 x)
{
  //
  // Returns a floating point physical value given a byte.
  // multiplication by .01 is conversion from units of .01 inches to inches.
  //
  double val=0.0;

  val = (double(x)-_bias)/_scale * .01;

  return val;
}

void DigitalAccumArray::calcDependentVals()
{
  memcpy(&_scale, &graphProdDesc.prodDependent31, 4);

  memcpy(&_bias, &graphProdDesc.prodDependent33, 4);

  memcpy(&_uncompProdSize, &graphProdDesc.prodDependent52, sizeof(ui32));

  if (byteSwap)
  {
    Swap::swap4((void*)&_scale);
    
    Swap::swap4((void*)&_bias);
   
    Swap::swap2(&graphProdDesc.prodDependent37);
    
    Swap::swap2(&graphProdDesc.prodDependent47); 

    Swap::swap2(&graphProdDesc.prodDependent48);

    Swap::swap2(&graphProdDesc.prodDependent49);
    
    Swap::swap2(&graphProdDesc.prodDependent50);
    
    Swap::swap2(&graphProdDesc.prodDependent51);
    
    Swap::swap4(&_uncompProdSize); 
  }

  _leadingFlag = (ui16) graphProdDesc.prodDependent37;
 
  _maxAccum = (fl32) graphProdDesc.prodDependent47 *.1;  
  
  _endTimeJDate  = (fl32) graphProdDesc.prodDependent48;
 
  _endTimeMinutes = (fl32) graphProdDesc.prodDependent49;
  
  _meanFieldBias = (fl32) graphProdDesc.prodDependent50 *.01;
 
  _isCompressed = (ui08)graphProdDesc.prodDependent51;
}

void DigitalAccumArray::printProdDependentVals()
{
  cerr << "Product dependent values: " << endl;
  
   cerr << "Leading data flag: " <<  _leadingFlag << endl;

  cerr << " maxAccum(inches)" << _maxAccum  << endl;
  
  cerr << " endTimeJDate " << _endTimeJDate << endl;
  
  cerr << " endTimeMinutes  " << _endTimeMinutes << endl;
  
  cerr << " meanFieldBias " << _meanFieldBias << " note: not implemented for "
       << "dual pole QPE products at this time. This param is a placeholder for "
       << "future implementation" << endl;
  
  cerr << " compression: " <<  _isCompressed << endl;

  cerr << " uncompressed data size: " << _uncompProdSize << endl; 
}
