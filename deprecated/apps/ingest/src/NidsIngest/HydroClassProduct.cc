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
 * @file HydroClassProduct.cc
 * @brief Source for HydroClassProduct class
 */

#include "HydroClassProduct.hh"
#include <string>
HydroClassProduct::HydroClassProduct( FILE *filePtr, bool byteSwapFlag, 
				      bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
 
}

HydroClassProduct::~HydroClassProduct ()
{
  
}

fl32 HydroClassProduct::convertData(ui08 x)
{
  return x;
}

void HydroClassProduct::calcDependentVals()
{
  si32 dataLevel;

  memcpy(&_uncompProdSize, &graphProdDesc.prodDependent52, sizeof(ui32));
  
  if(byteSwap)
  {
    Swap::swap2(&graphProdDesc.prodDependent30);
    
    Swap::swap2(&graphProdDesc.prodDependent47);

    Swap::swap2(&graphProdDesc.prodDependent48);

    Swap::swap2(&graphProdDesc.prodDependent51);

    Swap::swap4(&_uncompProdSize);
    
  }
  _elev = (fl32) graphProdDesc.prodDependent30 * .1;

  _modeFilterSize = (fl32) graphProdDesc.prodDependent47 * .1;  
 
  _isCompressed = (ui08) graphProdDesc.prodDependent51;

}


void HydroClassProduct::printProdDependentVals()
{
  
  cerr << " mode filter size " << _modeFilterSize  << endl;
  
  cerr << " elevation angle  " <<  _elev << endl;
  
  cerr << " compression: " <<  _isCompressed << endl;

  cerr << " uncompressed data size: " << _uncompProdSize << endl;
}
