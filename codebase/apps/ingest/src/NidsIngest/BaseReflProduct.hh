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
 * @file BaseRefl.hh
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Base Reflectivity products BREF1, BREF2, BREF3, BREF4. Product
 *        code 94.
 * 
 * @class BaseRefl
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Base Reflectivity products BREF1, BREF2, BREF3, BREF4. Product
 *        code 94.
 */
#ifndef BASEREFLPPRODUCT_H
#define BASEREFLPPRODUCT_H

#include <dataport/port_types.h>
#include "Product.hh"
#include <cmath>

using namespace std;

class BaseReflProduct: public Product {
  
public:
 /**
   * Constructor
   * Initializes public and private members
   * @params[in] filePtr  Pointer to file containing radar data
   * @params[in] byteSwapFlag  Flag to indicate byte swapping the various 
   *                           message components for handling endian issues 
   * @params[in] debugFlag  Flag to indicate debug messaging
   */
  BaseReflProduct( FILE *filePtr, bool byteSwapFlag, bool debugFlag);

  /**
   * Destructor
   */
  ~BaseReflProduct ();
  
  /** 
   * Converts byte data to float data. 
   * value = x * _increment/10 + _baseDataVal/10; 
   * @param[in] x  Input byte
   * @return decoded float value
   */
  fl32 convertData(ui08 x);
 
  /**
   * Converts short data to float data.
   * Not used by this product
   * @return 0
   */
   fl32 convertData(ui16 x) { return missingDataVal; }

  /** 
   * Not defined for this product
   */
  void convertRLEData(ui08 x, ui08 &run, fl32 &val){ val = missingDataVal;}

  /**
   * Calculate product dependent values. See Interface Control Document(ICD) 
   * Table V, Note 1 of Figure 3-6 message 6 for each product.
   */
  void calcDependentVals();

  /**
   * Print the product dependent information for debugging purposes
   */
  void printProdDependentVals(void);

  /**
   * @return 1 of product is compressed and 0 otherwise. 
   */
  ui16 isCompressed() {return _isCompressed; }
   
  /**
   * @return size in bytes of uncompressed product. This is only valid if
   *         isCompressed returns 1. Else the uncompressed product size is 
   *         known by the file size.
   */
  long int getUncompProdSize() {return _uncompProdSize;}

  /**
   * @return elevation angle.
   */
  float getElevAngle() {return _elev;}
 
protected:
  
private:
  
  /**
   * Elevation angle
   * Stored in hw30
   */
  fl32 _elev;

  /**
   * Base for dbz values in units of dbz*10
   * See ICD Fig 3-6, sheet 6, Note 1 
   * Stored in hw31 
   */
  fl32 _baseDataVal;

  /**
   * Increment for dbz values in units of dbz*10 
   * See ICD Fig 3-6, sheet 6, Note 1
   * Stored in hw32
   */
  fl32 _increment;

  /**
   * Multiplier for increment
   * See ICD Fig 3-6, sheet 6, Note 1
   * Stored in hw33
   */ 
  fl32 _numLevels;

  /**
   * Maximum refectivity
   * See ICD Fig 3-6, sheet 6, Note 1 
   * Stored in hw33
   */
  fl32 _maxRefl;

  /**
   * Flag to indicate compression of radial data
   * Stored in hw51
   */
  ui16 _isCompressed;
  
  /**
   * If data are compressed, this is the uncompressed data size in bytes
   * Stored in hw52 and hw53
   */
  ui32 _uncompProdSize;
};

#endif





