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

/////////////////////////////////////////////////////////////
// 
// StormPrecipProduct.hh
//
/////////////////////////////////////////////////////////////

#ifndef STORMPRECIPPRODUCT_H
#define STORMPRECIPPRODUCT_H
#include <dataport/port_types.h>
#include "Product.hh"
#include <cmath>

using namespace std;

class StormPrecipProduct: public Product {
  
public:

  // Constructor. 
  StormPrecipProduct ( FILE *filePtr, bool byteSwapFlag, bool debugFlag);

  // Destructor.
  ~StormPrecipProduct ();

  fl32 convertData(ui08 x);
 
  /**
   * Converts short data to float data.
   * Not used by this product
   * @return 0
   */
  fl32 convertData(ui16 x) { return missingDataVal; }

  /**
   * RLE not used by this product
   * @return missing 
   */
  void convertRLEData(ui08 x, ui08 &run, fl32 &val) { val = missingDataVal;}

  void calcDependentVals();

  void printProdDependentVals(void) {};

  ui16 isCompressed() {return _isCompressed; }
   
  long int getUncompProdSize() {return _uncompProdSize;}

  /**
   * @return elevation angle.
   */
  float getElevAngle() {return 0;}

protected:
  
private:

  //
  // stored in hw31
  //
  fl32 _minDataVal;

  //
  // stored in hw32 (increment in .01" units)
  //
  fl32 _increment;

  //
  // stored in hw33
  // 
  fl32 _numLevels;

   //
  // stored in hw51
  //
  ui16 _isCompressed;
  
  //
  // stored in hw52 and hw53
  // 
  ui32 _uncompProdSize;

};

#endif





