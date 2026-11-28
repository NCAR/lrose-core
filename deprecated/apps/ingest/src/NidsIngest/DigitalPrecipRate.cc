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
 * @file DigitalPrecipRate.cc
 * @brief Source for DigitalPrecipRate class
 */

#include "DigitalPrecipRate.hh"

DigitalPrecipRate::DigitalPrecipRate(FILE *filePtr, 
			       bool byteSwapFlag, bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

DigitalPrecipRate::~DigitalPrecipRate ()
{
  
}

fl32 DigitalPrecipRate::convertData(ui16 x)
{
  //
  // Returns a floating point physical value given a byte.
  //
  double val=0.0;

  val = (double(x)-_bias)/_scale;

  return val;
}


void DigitalPrecipRate::calcDependentVals()
{
  memcpy(&_scale, &graphProdDesc.prodDependent31, 4);

  memcpy(&_bias, &graphProdDesc.prodDependent33, 4);
  
  memcpy(&_uncompProdSize, &graphProdDesc.prodDependent52, sizeof(ui32));
  
  if (byteSwap)
  {
    Swap::swap2(&graphProdDesc.prodDependent27);
    
    Swap::swap2(&graphProdDesc.prodDependent28);
    
    Swap::swap2(&graphProdDesc.prodDependent30);
    
    Swap::swap4((void*)&_scale);
    
    Swap::swap4((void*)&_bias);
    
    Swap::swap2(&graphProdDesc.prodDependent47);
    
    Swap::swap2(&graphProdDesc.prodDependent48);
    
    Swap::swap2(&graphProdDesc.prodDependent49);
    
    Swap::swap2(&graphProdDesc.prodDependent50); 
    
    Swap::swap2(&graphProdDesc.prodDependent51);
 
    Swap::swap4(&_uncompProdSize);
  }
 
  _hybridRateScanDate = graphProdDesc.prodDependent27;
 
  _hybridRateScanTime = graphProdDesc.prodDependent28;
   
  _precipFlag = graphProdDesc.prodDependent30;
  
  _maxRate = (fl32) graphProdDesc.prodDependent47 *.001;  
  
  _hybridRatePctBinsFill  = (fl32) graphProdDesc.prodDependent48 * .01;
  
  _highElev = (fl32) graphProdDesc.prodDependent49 * .1;
  
  _meanFieldBias = (fl32) graphProdDesc.prodDependent50 *.01;
  
  _isCompressed = (ui08)graphProdDesc.prodDependent51;  
}

void DigitalPrecipRate::printProdDependentVals()
{

  cerr << "Product dependent values: " << endl;
  cerr << "_scale: "  << _scale << endl;
  cerr << "_bias: "  << _bias << endl;
 
  cerr << " hybridRateScanDate " << _hybridRateScanDate << endl;
  
  cerr << " hybridRateScanTime  " << _hybridRateScanTime << endl;
  
  ui08 precipDetecFlag;

  memcpy(&precipDetecFlag,&_precipFlag, sizeof(ui08));
  
  cerr << " precipDetectFlag  " << precipDetecFlag << endl;
  
  ui08 gageBias;

  ui08 *ptr = (ui08*) (&_precipFlag) + 1;

  memcpy(&gageBias, ptr, sizeof(ui08));

  cerr << " gageBias  " << gageBias << endl;

  cerr << " maxRate(inches/hour)" << _maxRate  << endl;
  
  cerr << " hybridRatePctBinsFill " << _hybridRatePctBinsFill << "%" <<endl;
  
  cerr << " highElev  " << _highElev  << " degrees" << endl;
  
  cerr << " meanFieldBias " << _meanFieldBias << " note: not implemented for "
       << "dual pole QPE products at this time. This param is a placeholder for "
       << "future implementation" << endl;
  
  cerr << " compression: " <<  _isCompressed << endl;

  cerr << " uncompressed data size: " << _uncompProdSize << endl;

 
}
