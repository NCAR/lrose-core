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
 * @file DigitalPrecipRate.hh
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Digital Precip Rate. Product DPA code 176. Product has Generic
 *        Radial Format)
 * 
 * @class DigitalPrecipRate
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Digital Precip Rate. Product DPA code 176.  Product has Generic
 *        Radial Format)
 */

#ifndef DIGITAL_PRECIP_RATE_H
#define DIGITAL_PRECIP_RATE_H
#include <dataport/port_types.h>
#include "Product.hh"
#include <cmath>

using namespace std;

class DigitalPrecipRate: public Product {
  
public:

  /**
   * Constructor
   * Initializes public and private members
   * @params[in] filePtr  Pointer to file containing radar data
   * @params[in] byteSwapFlag  Flag to indicate byte swapping the various 
   *                           message components for handling endian issues 
   * @params[in] debugFlag  Flag to indicate debug messaging
   */
  DigitalPrecipRate (FILE *filePtr, bool byteSwapFlag, bool debugFlag);

  
  /**
   * Destructor
   */
  ~DigitalPrecipRate ();

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
   * Hybrid Rate Scan Date (Julian date)
   * See ICD Table V
   * Stored in hw27
   */
  ui16 _hybridRateScanDate;

  /**
   * Hybrid Rate Scan Time (minutes)
   * See ICD Table V
   * Stored in hw28
   */
  ui16 _hybridRateScanTime;
  
  /**
   * Precipitation Detected Flag( high byte) and Gage Bias to be applied 
   * flag (low byte). ( Gage bias not being implemented at time of writing
   * of ICD. Parameter may be a place holder. See ICD Table V)
   * Stored in hw30
   */
  ui16 _precipFlag;

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
   * Maximum Precip Rate (inches/hour)
   * See ICD Table V
   * Stored in hw47
   */ 
  fl32 _maxRate;

  /**
   * Hybrid rate percent bins filled
   * See ICD Table V
   * Stored in hw47
   */ 
  fl32 _hybridRatePctBinsFill;

  /**
   * Highest elevation used (degrees)
   * See ICD Table V
   * Stored in hw49
   */ 
  fl32 _highElev;
  
  /**
   * Mean field bias( Gage bias not being implemented at time of writing
   * of ICD. Parameter may be a place holder)
   * See ICD Table V
   * Stored in hw50
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





