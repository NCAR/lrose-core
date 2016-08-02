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


#ifndef PRODUCT_H
#define PRODUCT_H

#include <cstdlib>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <dataport/port_types.h>
#include "GraphicProductMsg.hh"
#include "Swap.hh"

using namespace std;

/**
 * @file Product.hh
 * @brief Product class handles reading the Graphic Product Message Description
 *        Block. The Despcription Block is described in ICD Section 3.3.1.1 and
 *        Figure 3-6 (Sheet 2 and 6). It contains information radar latitude,
 *        longitude, altitude, time, VCP, operational mode, etc. It also has
 *        several fields that contain product specific information like scale
 *        and bias needed to decode the radial byte data, or a base value, an
 *        increment, and a multiplier for the increment. The product specific
 *        members of the Graphic Product Message Description Block are found in
 *        the Interface Control Document(ICD) Table V, Note 1 of Figure 3-6 
 *        message 6.
 * 
 *        ICD Build 12.1 for document number 2620001, code id 0WY55 
 *        (RPG to Class 1 User ICD) which can be found at:
 *        http://www.roc.noaa.gov/WSR88D/DualPol/documentation.aspx.
 * 
 * @class Product
 * @brief Product class handles reading the Graphic Product Message Description
 *        Block. The Despcription Block is described in ICD Section 3.3.1.1 and
 *        Figure 3-6 (Sheet 2 and 6). It contains information radar latitude,
 *        longitude, altitude, time, VCP, operational mode, etc. It also has
 *        several fields that contain product specific information like scale
 *        and bias needed to decode the radial byte data, or a base value, an
 *        increment, and a multiplier for the increment. The product specific
 *        members of the Graphic Product Message Description Block are found in
 *        the Interface Control Document(ICD) Table V, Note 1 of Figure 3-6 
 *        message 6.
 * 
 *        ICD Build 12.1 for document number 2620001, code id 0WY55 
 *        (RPG to Class 1 User ICD) which can be found at:
 *        http://www.roc.noaa.gov/WSR88D/DualPol/documentation.aspx.
 * 
 */

class Product {

public:
  /**
   * Return value for Product methods
   * PRODUCT_SUCCESS indicates successful method execution
   * PRODUCT_FAILURE indicates unsuccessful method execution
   */
  enum Status_t {PRODUCT_SUCCESS, PRODUCT_FAILURE};

  /**
   * Constructor
   * Initializes public and private members
   * @params[in] filePtr  Pointer to file containing radar data
   * @params[in] byteSwapFlag  Flag to indicate byte swapping the various 
   *                           message components for handling endian issues 
   * @params[in] debugFlag  Flag to indicate debug messaging
   */
  Product( FILE *filePtr, bool byteSwapFlag, bool debugFlag);

  /**
   * Destructor
   */
  ~Product();

  /**
   * The ReadFromFile method reads the Graphic Product Message Description 
   * Block, calculates product dependent values, and checks that the Symbology
   * Block(which contains the radial data) exists and advances the file pointer 
   * to that Block. 
   * @return PRODUCT_SUCCESS or PRODUCT_FAILURE if reading is successful
   */
  int readFromFile();

  /** 
   * Converts byte data to float data.  
   * @param[in] x  Input byte
   * @return decoded float value
   */
  virtual fl32 convertData(ui08 x)= 0;

  /** 
   * Converts Run Length Encdoded converts byte data to float data.
   * @param[in] x  Input byte
   * @param[out] run  Number of bins to which to assign data value
   * @param[out] val  Value assigned to bins
   */
  virtual void convertRLEData(ui08 x, ui08 &run, fl32 &val)=0;

  /**
   * Calculate product dependent values. See Interface Control Document(ICD) 
   * Table V, Note 1 of Figure 3-6 message 6 for each product.
   */
  virtual void calcDependentVals(void) = 0;

  /**
   * Print the product dependent information for debugging purposes
   */
  virtual void printProdDependentVals(void) = 0;

  /**
   * @return 1 of product is compressed and 0 otherwise. 
   */
  virtual ui16 isCompressed() = 0;

  /**
   * @return size in bytes of uncompressed product. This is only valid if
   *         isCompressed returns 1. Else the uncompressed product size is known
   *         by the file size.
   */
  virtual long int getUncompProdSize()= 0;

  /**
   * @return elevation angle.
   */
  virtual float getElevAngle()= 0;
  
  /**
   * Flag to indicate byte swapping the various message components for 
   * handling endian issues
   */
  bool byteSwap;
  
  /**
   * Flag to indicate debug messaging
   */
  bool debug;

  const static float missingDataVal;
  
  /**
   * Struct for storing components of Graphic Product Message Description 
   * Block.  The Despcription Block is described in ICD Section 3.3.1.1 
   * and Figure 3-6 (Sheet 2 and 6).
   */
  GraphicProductMsg::productDescription_t graphProdDesc;

protected:
  
private:

  FILE *fp;
};

#endif





