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



#include "EchoTopsProduct.hh"

EchoTopsProduct::EchoTopsProduct( FILE *filePtr, bool byteSwapFlag, 
					bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
 
}

EchoTopsProduct::~EchoTopsProduct ()
{
  
}

fl32 EchoTopsProduct::convertData(ui08 x)
{
  //
  // If data is 0 (below threshold) or 1 (bad value)
  // return missing
  //
  if (x == 0 || x == 1)
  { 
    return missingDataVal;
  }
  
  //
  // Get rid of "data is topped" flag
  //
  if (x >= _toppedMask)
  {
    x = x - _toppedMask;
  }
  
  double val = double(x)/double(_scale) - double(_offset);

  return val;
}

void EchoTopsProduct::calcDependentVals()
{
  memcpy(&_uncompProdSize, &graphProdDesc.prodDependent52, sizeof(ui32));

  if (byteSwap)
  {
    Swap::swap2(&graphProdDesc.prodDependent30);
    
    Swap::swap2(&graphProdDesc.prodDependent31);
    
    Swap::swap2(&graphProdDesc.prodDependent32);

    Swap::swap2(&graphProdDesc.prodDependent33);

    Swap::swap2(&graphProdDesc.prodDependent34);

    Swap::swap2(&graphProdDesc.prodDependent51);

    Swap::swap4(&_uncompProdSize);

  }

  _elev =  (fl32) graphProdDesc.prodDependent30 *.1;
 
  _dataMask = (fl32) graphProdDesc.prodDependent31;
 
  _scale = (fl32) graphProdDesc.prodDependent32;
 
  _offset = (fl32) graphProdDesc.prodDependent33;

  _toppedMask = (fl32) graphProdDesc.prodDependent34;

  _isCompressed = (ui08)graphProdDesc.prodDependent51;
}

void EchoTopsProduct::printProdDependentVals()
{
  cerr << " elevation: " <<  _elev << endl;

  cerr << " data mask( should be 127): " << _dataMask << endl;
  
  cerr << " topped mask(should be 128): " << _toppedMask << endl;

  cerr << " scale: " <<  _scale << endl;

  cerr << " offset: " << _offset << endl;
  
  cerr << " compression: " <<  _isCompressed << endl;

  cerr << " uncompressed data size: " << _uncompProdSize << endl;
}
