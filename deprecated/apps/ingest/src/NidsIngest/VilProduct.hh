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
// VilProduct.hh
//
// Definitons for class
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#ifndef VILPRODUCT_H
#define VILPRODUCT_H
#include <dataport/port_types.h>
#include "Product.hh"
#include <cmath>

using namespace std;

class VilProduct: public Product {
  
public:

  // Constructor. 
  VilProduct ( FILE *filePtr, bool byteSwapFlag, bool debugFlag);

  // Destructor.
  ~VilProduct ();

  fl32 convertData(ui08 x);

  /**
   * Converts short data to float data.
   * not used by this product
   * @return 0
   */
  fl32 convertData(ui16 x) { return missingDataVal; }

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
  
  fl32 _scale1;

  fl32 _bias1;

  fl32 _scale2;

  fl32 _bias2;

  fl32 _decodeThresh;
  
  fl32 _decode_fl32(ui16 hw);

  
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





