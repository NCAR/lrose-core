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
 * @file HailIndex.hh
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        the Hail Index.  Product NHI, code 59.
 * 
 * @class HailIndex.hh
 * @brief  Class for decoding Graphic Product Message Description Block for 
 *         the Hail Index.  Product NHI, code 59.
 */
#ifndef HailIndex_HH
#define HailIndex_HH

#include "Product.hh"
#include <dataport/port_types.h>
using namespace std;

class HailIndex: public Product {
  
public:
 /**
   * Constructor
   * Initializes public and private members
   * @params[in] filePtr  Pointer to file containing radar data
   * @params[in] byteSwapFlag  Flag to indicate byte swapping the various 
   *                           message components for handling endian issues 
   * @params[in] debugFlag  Flag to indicate debug messaging
   */
  HailIndex( FILE *filePtr, bool byteSwapFlag, bool debugFlag);
  
  /**
   * Constructor
   */
  HailIndex();

  /**
   * Destructor
   */
  ~HailIndex();
  
  /** 
   * Converts byte data to float data. 
   * Not relevant for this product
   * @param[in] x  Input byte
   * @return decoded float value
   */
  fl32 convertData(ui08 x) {return 0;}

  /**
   * Converts short data to float data.
   * Not used by this product
   * @return 0
   */
   fl32 convertData(ui16 x) { return missingDataVal; }

  /** 
   * Not defined for this product
   */
  void convertRLEData(ui08 x, ui08 &run, fl32 &val){val = missingDataVal;}

  /**
   * Calculate product dependent values. See Interface Control Document(ICD) 
   * Table V. (Nothing for Hail Index)
   */
  void calcDependentVals(){};

  /**
   * Print the product dependent information for debugging purposes
   */
  void printProdDependentVals(void){};

  /**
   * @return 1 of product is compressed and 0 otherwise. 
   */
  ui16 isCompressed() {return 0;}
   
  /**
   * @return size in bytes of uncompressed product. This is only valid if
   *         isCompressed returns 1. Else the uncompressed product size is 
   *         known by the file size.
   */
  long int getUncompProdSize() {return 0; }

  /**
   * @return elevation angle.
   */
  float getElevAngle() {}
 
protected:
  
private:

};

#endif





