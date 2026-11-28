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


#include "Product.hh"
#include <iostream>
#include <toolsa/umisc.h>

const float Product:: missingDataVal = -999.0;

Product::Product(  FILE *filePtr, bool byteSwapFlag, bool debugFlag):
  fp(filePtr),
  byteSwap(byteSwapFlag),
  debug(debugFlag)
{ 
}

Product::~Product ()
{
  
}

int Product::readFromFile(bool hasHdr)
{
  if(GraphicProductMsg::readProdDescr( graphProdDesc,fp, byteSwap,debug))
  {
    return PRODUCT_FAILURE;
  }
  
  calcDependentVals();
  
  if(debug)
  {
    printProdDependentVals();
  } 

  //
  // Advance offset to the symbology block. If this is not possible then there 
  // isnt any data!
  //
  if ( graphProdDesc.symbOff == 0)
  {
    if (debug)
    {
      cerr << "Product::readFromFile():No symbology block found in file." 
	   << endl;
    }
    return PRODUCT_FAILURE; 
  }
  
  //
  // The offset is given in units of two byte halfwords.
  //
  long offset;
  
  if (hasHdr)
  {
    offset = 2 * graphProdDesc.symbOff + 30;
  }
  else
  {
    offset = 2 * graphProdDesc.symbOff;
  }

  if (fseek(fp, offset, SEEK_SET))
  {
    if (debug)
    {
      cerr << "Product::readFromFile(): Seek to symbology block failed" << endl;
    }
    return PRODUCT_FAILURE;
  }
  
  return PRODUCT_SUCCESS;
}
