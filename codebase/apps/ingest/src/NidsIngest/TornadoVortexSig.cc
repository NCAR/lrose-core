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
 * @file TornadoVortexSig.cc
 * @brief Source for TornadoVortexSig class
 */

#include "TornadoVortexSig.hh"

TornadoVortexSig::TornadoVortexSig( FILE *filePtr, bool byteSwapFlag, 
					bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

TornadoVortexSig::TornadoVortexSig():
Product(NULL, true, false)
{

}

TornadoVortexSig::~TornadoVortexSig()
{
  
}

void TornadoVortexSig::calcDependentVals()
{
  if( byteSwap)
  {
    Swap::swap2(&graphProdDesc.prodDependent47);
    
    Swap::swap2(&graphProdDesc.prodDependent48);
  }
  
  _totalTvs = (si16) (graphProdDesc.prodDependent47);

  _totalEtvs = (si16) (graphProdDesc.prodDependent48);
}

void  TornadoVortexSig::printProdDependentVals(void)
{
  cerr << "Product dependent values" << endl;
  
  cerr << " total TVS (negative number recorded indicates number exceeds max): " 
       <<   _totalTvs << endl;

  cerr << " total ETVS (negative number recorded indicates number exceeds max): " 
       <<   _totalEtvs << endl;
  
}


