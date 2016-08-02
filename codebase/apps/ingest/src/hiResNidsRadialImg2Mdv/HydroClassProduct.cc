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

    Swap::swap2(&graphProdDesc.prodDependent31);

    Swap::swap2(&graphProdDesc.prodDependent32);

    Swap::swap2(&graphProdDesc.prodDependent34);
    
    Swap::swap2(&graphProdDesc.prodDependent35);

    Swap::swap2(&graphProdDesc.prodDependent36);

    Swap::swap2(&graphProdDesc.prodDependent37);
    
    Swap::swap2(&graphProdDesc.prodDependent38);
    
    Swap::swap2(&graphProdDesc.prodDependent39);

    Swap::swap2(&graphProdDesc.prodDependent40);

    Swap::swap2(&graphProdDesc.prodDependent41);

    Swap::swap2(&graphProdDesc.prodDependent42);
    
    Swap::swap2(&graphProdDesc.prodDependent43);
    
    Swap::swap2(&graphProdDesc.prodDependent44);
    
    Swap::swap2(&graphProdDesc.prodDependent45);

    Swap::swap2(&graphProdDesc.prodDependent46);
    
    Swap::swap2(&graphProdDesc.prodDependent47);

    Swap::swap2(&graphProdDesc.prodDependent48);

    Swap::swap2(&graphProdDesc.prodDependent51);

    Swap::swap4(&_uncompProdSize);
    
  }
  _elev = (fl32) graphProdDesc.prodDependent30 * .1;

  dataLevel = (fl32) graphProdDesc.prodDependent31;
  cerr << "DataLevel31: " << endl;
  _decodeDataLevel(dataLevel);
 
  dataLevel = (fl32) graphProdDesc.prodDependent32;
  cerr << "DataLevel32: " << endl; 
  _decodeDataLevel(dataLevel);
 
  dataLevel = (fl32) graphProdDesc.prodDependent33;
  cerr << "DataLevel33: " << endl; 
  _decodeDataLevel(dataLevel);
  
  dataLevel = (fl32) graphProdDesc.prodDependent34;
  cerr << "DataLevel34: " << endl; 
  _decodeDataLevel(dataLevel);
  
  dataLevel = (fl32) graphProdDesc.prodDependent35;
  cerr << "DataLevel35: " << endl; 
  _decodeDataLevel(dataLevel);
 
  dataLevel = (fl32) graphProdDesc.prodDependent36;
  cerr << "DataLevel36: " << endl; 
  _decodeDataLevel(dataLevel);
 
  dataLevel = (fl32) graphProdDesc.prodDependent37;
  cerr << "DataLevel37: " << endl; 
  _decodeDataLevel(dataLevel);
 
  dataLevel = (fl32) graphProdDesc.prodDependent38;
  cerr << "DataLevel38: " << endl;
  _decodeDataLevel(dataLevel);

 
  dataLevel = (fl32) graphProdDesc.prodDependent39;
  cerr << "DataLevel39: " << endl; 
  _decodeDataLevel(dataLevel); 

  dataLevel = (fl32) graphProdDesc.prodDependent40;
  cerr << "DataLevel40: " << endl; 
  _decodeDataLevel(dataLevel);
 
  dataLevel = (fl32) graphProdDesc.prodDependent41;
  cerr << "DataLevel41:" << endl;
  _decodeDataLevel(dataLevel);
  
  dataLevel = (fl32) graphProdDesc.prodDependent42;
  cerr << "DataLevel42: " << endl;
  _decodeDataLevel(dataLevel);

  dataLevel = (fl32) graphProdDesc.prodDependent43;
  cerr << "DataLevel43 " << endl;
  _decodeDataLevel(dataLevel);
 
  dataLevel = (fl32) graphProdDesc.prodDependent44;
  cerr << "DataLevel44: " << endl;
  _decodeDataLevel(dataLevel);
  
  dataLevel = (fl32) graphProdDesc.prodDependent45;
  cerr << "DataLevel45: " << endl;
  _decodeDataLevel(dataLevel);
   
  dataLevel = (fl32) graphProdDesc.prodDependent46;
  cerr << "DataLevel46: " << endl;
  _decodeDataLevel(dataLevel);

  _modeFilterSize = (fl32) graphProdDesc.prodDependent47 * .1;  
 
  _isCompressed = (ui08)graphProdDesc.prodDependent51;

}

void HydroClassProduct::_decodeDataLevel(si32 dataLevel)
{

  cerr << dataLevel << endl;

  ui08 *bytePtr;

  ui08 byte1;

  ui08 byte2;

  bytePtr = (ui08 *)&dataLevel;

  memcpy( (void*) &byte1, &dataLevel, sizeof(ui08));

  bytePtr++;

  memcpy( (void*) &byte2, (void*) bytePtr, sizeof(ui08));
   
  bool divideBy100 = false;

  bool divideBy20 = false;

  bool divideBy10 = false;

  bool useGreaterThan = false;

  bool useLessThan = false;

  bool usePlus = false;
  
  bool useMinus = false;
  
  if (byte2 & 128 )
  {
    cerr << "Hydrometeor classification" << endl;

    if( byte2 & 64)
      divideBy100 = true;
    if( byte2 & 32 )
      divideBy20 = true;
    if( byte2 & 16)
      divideBy10 = true;
    if( byte2 &  8)
      useGreaterThan = true;
    if( byte2 &  4)
      useGreaterThan = true;
    if( byte2 &  2)
      usePlus = true;
    if(byte2 & 1 )
      useMinus = true;

    fl32 thresh; 

    memcpy( (void*)&thresh, (void*) &byte1 ,sizeof(ui08));

    if (divideBy100)
    {
      thresh = thresh/100;
    }
    if(divideBy20)
    {
      thresh = thresh/20;
    }
    if(divideBy10)
    {
      thresh = thresh/10;
    }
    
    string threshStr = "";

    if (useGreaterThan)
      threshStr += "> ";
    if(useLessThan)
      threshStr += "< ";
    if(usePlus)
      threshStr += "+ ";
     if(useMinus)
      threshStr += "- ";

     cerr << threshStr << thresh << endl;
  } 

}
