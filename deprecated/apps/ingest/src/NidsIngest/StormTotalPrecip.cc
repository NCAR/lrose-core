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
 * @file StormTotalPrecip.cc
 * @brief Source for StormTotalPrecip class
 */
#include "StormTotalPrecip.hh"


StormTotalPrecip::StormTotalPrecip(FILE *filePtr, 
			       bool byteSwapFlag, bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

StormTotalPrecip::~StormTotalPrecip ()
{
  
}

void StormTotalPrecip::convertRLEData(ui08 x, ui08 &run, fl32 &val)
{
  typedef struct 
  {
    unsigned low : 4;
    unsigned high : 4;
  } nibbles;

  nibbles *dataNibble = (nibbles *) &x; 
  
  run = dataNibble->high;

  val = dataNibble->low;

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
    val = 0;
    break;
  case 1:
    val = .15;
    break;
  case 2:
    val = .45;
    break;
  case 3:
    val = .8;
    break;
  case 4:
    val = 1.25;
    break;
  case 5:
    val = 1.75;
    break;
  case 6:
    val = 2.25;
    break;
  case 7:
    val = 2.75;
    break;
  case 8:
    val = 3.5;
    break;
  case 9:
    val = 4.5;
    break;
  case 0xA:
    val = 5.5;
    break;
  case 0xB:
    val = 7;
    break;
  case 0xC:
    val = 9;
    break;
  case 0xD:
    val = 11.0;
    break;
  case 0xE:
    val = 13.5;
    break;
  case 0xF:
    val = 15;
    break;
  default:
    val = 0;
  }
};

void StormTotalPrecip::calcDependentVals()
{
  if(byteSwap)
  {
     Swap::swap2(&graphProdDesc.prodDependent47);
  
     Swap::swap2(&graphProdDesc.prodDependent48);

     Swap::swap2(&graphProdDesc.prodDependent49);

     Swap::swap2(&graphProdDesc.prodDependent50);

     Swap::swap2(&graphProdDesc.prodDependent51);

     Swap::swap2(&graphProdDesc.prodDependent52);

     Swap::swap2(&graphProdDesc.prodDependent53);
  } 

  
  _maxRainfall = (fl32) graphProdDesc.prodDependent47 * .1;  
  
  _meanFieldBias = (fl32) graphProdDesc.prodDependent52 * .01;

  _numGRPairs = (fl32) graphProdDesc.prodDependent53 * .01;
    
  _startTimeJDate = (ui16) graphProdDesc.prodDependent48;

  _startTimeMinutes  = (ui16) graphProdDesc.prodDependent49;

  _endTimeJDate = (ui16) graphProdDesc.prodDependent50;

  _endTimeMinutes  = (ui16) graphProdDesc.prodDependent51;

}

void StormTotalPrecip::printProdDependentVals()
{
  cerr << " maxRainfall = " << _maxRainfall << endl;

  cerr << " meanFieldBias = " << _meanFieldBias  << endl;
  
  cerr << " numGRPairs = " <<  _numGRPairs << endl;
  
  cerr << " rainfall startTimeJDate " << _startTimeJDate << endl;

  cerr << " rainfall startTimeMinutes " << _startTimeMinutes << endl;

  cerr << " rainfall endTimeJDate " << _startTimeJDate << endl;

  cerr << " rainfall endTimeMinutes " << _startTimeMinutes << endl;
}
