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
 * @file EchoTopsProdcut.hh
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Enhanced Echo Tops Product. Product EET code 135. 
 * 
 * @class DigitalPrecipRate
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Endhanced Echo Tops Product. Product EET code 135. 
 */

#ifndef ECHOTOPSPRODUCT_H
#define ECHOTOPSPRODUCT_H
#include <dataport/port_types.h>
#include "Product.hh"
#include <cmath>

using namespace std;
class EchoTopsProduct: public Product {
  
public:

  /**
   * Constructor
   * Initializes public and private members
   * @params[in] filePtr  Pointer to file containing radar data
   * @params[in] byteSwapFlag  Flag to indicate byte swapping the various 
   *                           message components for handling endian issues 
   * @params[in] debugFlag  Flag to indicate debug messaging
   */
  EchoTopsProduct (FILE *filePtr, bool byteSwapFlag, bool debugFlag);

  /**
   * Destructor
   */
  ~EchoTopsProduct ();

  /** 
   * Converts byte data to float data. 
   *  val returned =( x-_bias)/_scale;
   * @param[in] x  Input byte
   * @return decoded float value
   */
  fl32 convertData(ui08 x);
  
  /** 
   * Not defined for this product
   */
  void convertRLEData(ui08 x, ui08 &run, fl32 &val){};
  
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
   * AVSET termination elevation angle 
   * stored in hw30. See ICD Table V.
   */
  fl32 _elev;
  
  /**
   * dataMask: Bit mask for data (value== 127)  
   * See ICD Fig 3-6, sheet 6, Note 1 
   */
  fl32 _dataMask;

  /**
   * See ICD Fig 3-6, sheet 6, Note 1
   * stored in hw32
   */
  fl32 _scale;

  /**
   * Data offset 
   * See ICD Fig 3-6, sheet 6, Note 1
   * stored in hw 33
   */ 
  fl32 _offset;

  /**
   * Topped Mask: Bit mask for topped data (value== 128)  
   * See ICD Fig 3-6, sheet 6, Note 1
   * stored in hw34
   */
  fl32 _toppedMask; 

  /**
   * Flag to indicate compression of radial data
   * See ICD Table V
   * Stored in hw51
   */
  ui16 _isCompressed;
  
  /**
   * If data are compressed, this is the uncompressed data size in bytes
   * See ICD Table V
   * Stored in hw52 and hw53
   */
  ui32 _uncompProdSize;
};

#endif





