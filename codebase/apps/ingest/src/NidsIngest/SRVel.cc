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
 * @file SRVel.cc
 * @brief Source for SRVel class
 */

#include "SRVel.hh"

SRVel::SRVel( FILE *filePtr, bool byteSwapFlag, 
					bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

SRVel::~SRVel ()
{
  
}

void SRVel::convertRLEData(ui08 x, ui08 &run, fl32 &val)
 {
  typedef struct
  {
    unsigned low : 4;
    unsigned high : 4;
  } nibbles;

  nibbles *dataNibble = (nibbles *) &x;
 
  run = dataNibble->high;

  ui08 colorCode = dataNibble->low;


  // Interpretation of code is found in: 
  // Document Number 2620003R
  // Code Identification 0WY55
  // WSR-88D ROC
  // Build Date 03/07/2012
  // RPG Build 13.0
  // The average value of the interval is returned.
  // 
  switch (colorCode)
  {
  case 0:
    val =  missingDataVal;
    break;
  case 1:
    val = -50;
    break;
  case 2:
    val = -45;
    break;
  case 3:
    val = -35;
    break;
  case 4:
    val = -26;
    break;
  case 5:
    val = -16;
    break;
  case 6:
    val = -7.5;
    break;
  case 7:
    val = -2.5;
    break;
  case 8:
    val =  2.5;
    break;
  case 9:
    val = 7.5;
    break;
  case 0xA:
    val = 16;
    break;
  case 0xB:
    val = 26;
    break;
  case 0xC:
    val = 35;
    break;
  case 0xD:
    val = 45.0;
    break;
  case 0xE:
    val = 50;
    break;
  case 0xF:
    // radar folding
    val = missingDataVal;
    break;
  default:
    val = missingDataVal;
  }
}

void SRVel::calcDependentVals()
{
  
  if( byteSwap)
  {
    Swap::swap2(&graphProdDesc.prodDependent30);
   
    Swap::swap2(&graphProdDesc.prodDependent47);
    
    Swap::swap2(&graphProdDesc.prodDependent48);
    
    Swap::swap2(&graphProdDesc.prodDependent49);
    
    Swap::swap2(&graphProdDesc.prodDependent51);
    
    Swap::swap2(&graphProdDesc.prodDependent52);
  }


  _elev = (fl32) graphProdDesc.prodDependent30 * .1;

  _maxNegVel =  (fl32) ((si16)graphProdDesc.prodDependent47);

  _maxPosVel = (fl32) ((si16)graphProdDesc.prodDependent48);

  _motionSourceFlag = (si16) graphProdDesc.prodDependent49;

  _aveStormSpeed = (fl32) (graphProdDesc.prodDependent51) * .1;

  _aveStormDir = (fl32) (graphProdDesc.prodDependent52) * .1;

 
}

void  SRVel::printProdDependentVals(void)
{

  cerr << "Product dependent values" << endl;
  
  cerr << " elevation: " << _elev << endl;

  cerr << " max neg vel: " <<  _maxNegVel << endl;

  cerr << " max pos vel: " <<  _maxPosVel << endl;

  cerr << " motion source flag: " << _motionSourceFlag << endl;

  cerr << " ave storm speed: " <<  _aveStormSpeed << endl;

  cerr << " ave storm direction: " <<  _aveStormDir << endl;
 
}
