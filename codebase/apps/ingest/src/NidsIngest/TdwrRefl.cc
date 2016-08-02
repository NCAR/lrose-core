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
 * @file TdwrRefl.cc
 * @brief Source for TdwrRefl class
 */
#include "TdwrRefl.hh"


TdwrRefl::TdwrRefl(FILE *filePtr, 
			       bool byteSwapFlag, bool debugFlag):
  Product(filePtr, byteSwapFlag, debugFlag)
{
  
}

TdwrRefl::~TdwrRefl ()
{
  
}

void TdwrRefl::convertRLEData(ui08 x, ui08 &run, fl32 &val)
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
  
  //
  // Interpretation of code is found in:
  //Document Number 26200xx
  //Code Identification 0WY55
  //TDWR SPG NPI
  //23 March 2008
  //TDWR SPG Bd 3.0
  // The bottom value of the interval is returned.
  // 
  switch (colorCode)
  {
  case 0:
    val = 0;
    break;
  case 1:
    val = 5;
    break;
  case 2:
    val = 10;
    break;
  case 3:
    val = 15;
    break;
  case 4:
    val = 20;
    break;
  case 5:
    val = 25;
    break;
  case 6:
    val = 30;
    break;
  case 7:
    val = 35;
    break;
  case 8:
    val = 40;
    break;
  case 9:
    val = 45;
    break;
  case 0xA:
    val = 50;
    break;
  case 0xB:
    val = 55;
    break;
  case 0xC:
    val = 60;
    break;
  case 0xD:
    val = 65;
    break;
  case 0xE:
    val = 70;
    break;
  case 0xF:
    val = 75;
    break;
  default:
    val = 0;
  }
};

void TdwrRefl::calcDependentVals()
{
  if( byteSwap)
  {
    Swap::swap2(&graphProdDesc.prodDependent30);
  }

 _elev = (fl32) graphProdDesc.prodDependent30 * .1; 

 cerr << "ELEVATION IN TDWR OBJECT: " <<  _elev  << endl;
 
}

void TdwrRefl::printProdDependentVals()
{
   cerr << "Product dependent values" << endl;
  
  cerr << " elevation: " << _elev << endl;

}
