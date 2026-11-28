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
 * @file DigitalAccumArray.hh
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Digital Accum Array product DAA . Product code 170.
 * 
 * @class DigitalAccumArray
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Digital Accum Array products. Product DAA code 170.
 */

#ifndef DIGITAL_ACCUM_ARRAY_H
#define DIGITAL_ACCUM_ARRAY_H

#include <dataport/port_types.h>
#include "Product.hh"
#include <cmath>

using namespace std;

class DigitalAccumArray: public Product {
  
public:
  /**
   * Constructor
   * Initializes public and private members
   * @params[in] filePtr  Pointer to file containing radar data
   * @params[in] byteSwapFlag  Flag to indicate byte swapping the various 
   *                           message components for handling endian issues 
   * @params[in] debugFlag  Flag to indicate debug messaging
   */
   DigitalAccumArray (FILE *filePtr, bool byteSwapFlag, bool debugFlag);

  /**
   * Destructor
   */
  ~DigitalAccumArray ();
  
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
  void convertRLEData(ui08 x, ui08 &run, fl32 &val) {};

  /**
   * Calculate product dependent values. See Interface Control Document(ICD) 
   * Table V, Note 1 of Figure 3-6 message 6 for each product.
   */
  void calcDependentVals();

  /**
   * Print the product dependent information for debugging purposes
   */
  void printProdDependentVals();

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
  float getElevAngle() {return 0;}

protected:
  
private:


  /**
   * Scale 
   * See ICD Fig 3-6, sheet 6, Note 1 
   * Stored in hw31 and hw32
   */ 
  fl32 _scale;

  /**
   * Scale 
   * See ICD Fig 3-6, sheet 6, Note 1 
   * Stored in hw31 and hw32
   */ 
  fl32 _bias;

  /**
   * Maximum data value
   * See ICD Table V
   * Stored in hw47
   */ 
  fl32 _maxAccum;

  /**
   * Ending data of accumulation( Julian date)
   * See ICD Table V
   * Stored in hw47
   */ 
  ui16 _endTimeJDate;
  
  /**
   * Ending time of accumulation (minutes)
   * See ICD Table V
   * Stored in hw47
   */ 
  ui16 _endTimeMinutes;
  
  /**
   * Mean field bias( Gage bias not being implemented at time of writing
   * of ICD. Parameter may be a place holder)
   * See ICD Table V
   * Stored in hw47
   */ 
  fl32 _meanFieldBias;

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





