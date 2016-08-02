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
 * @file StormTotalPrecip.hh
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        product NTP. Product codes 80. This products use radial data packet 
 *        code AF1F. See figure 3-10 of Build 12.1 for document number 
 *        2620001, code id 0WY55 (RPG to Class 1 User ICD) which can be found 
 *        at: http://www.roc.noaa.gov/WSR88D/DualPol/documentation.aspx.
 * 
 * @class StormTotalPrecip
 * @brief  Class for decoding Graphic Product Message Description Block for 
 *        product NTP. Product codes 80. This products use radial data packet 
 *        code AF1F. See figure 3-10 of Build 12.1 for document number 
 *        2620001, code id 0WY55 (RPG to Class 1 User ICD) which can be found 
 *        at: http://www.roc.noaa.gov/WSR88D/DualPol/documentation.aspx. 
 */

#ifndef STORM_TOTAL_PRECIP_H
#define STORM_TOTAL_PRECIP_H

#include <dataport/port_types.h>
#include "Product.hh"
#include <cmath>

using namespace std;

class StormTotalPrecip: public Product {
  
public:

  /**
   * Constructor
   * Initializes public and private members
   * @params[in] filePtr  Pointer to file containing radar data
   * @params[in] byteSwapFlag  Flag to indicate byte swapping the various 
   *                           message components for handling endian issues 
   * @params[in] debugFlag  Flag to indicate debug messaging
   */
  StormTotalPrecip (FILE *filePtr, bool byteSwapFlag, bool debugFlag);

   /**
   * Destructor
   */
  ~StormTotalPrecip ();

  /** 
   * Not defined for this product
   */
  fl32 convertData(ui08 x) {return missingDataVal; }

  /**
   * Converts short data to float data.
   * not used by this product
   * @return 0
   */
   fl32 convertData(ui16 x) { return missingDataVal; }

 
  /**
   * Convert Run Length Encoded byte data. The lower 'nibble' is decoded as 
   * the 'run' or the number of array elements assigned the same value.  The
   * higher 'nibble' is data value applied to consecutive array elements. 
   */
  void convertRLEData(ui08 x, ui08 &run, fl32 &val);

  /**
   * Calculate product dependent values. See Interface Control Document(ICD) 
   * Table V for product 80.
   */
  void calcDependentVals();

  
  /**
   * Print the product dependent information for debugging purposes
   */
  void printProdDependentVals();

  /**
   * @return 0 Product not compressed. 
   */
  ui16 isCompressed() {return 0; }
   
  /**
   * @return -1. Product is not compressed. Product size can be 
   *             determined from file size.
   */
  long int getUncompProdSize() {return -1;}

  /**
   * @return elevation angle.
   */
  float getElevAngle() {return 0;}

protected:
  
private:

  /**
   * Max rainfall in inches
   */
  fl32 _maxRainfall;
  
  /**
   * Start Date  of rainfall (Julian date)
   * See ICD Table V
   * Stored in hw48
   */
  ui16 _startTimeJDate;

  /**
   * Start Time of rainfall (minutes)
   * See ICD Table V
   * Stored in hw49
   */
  ui16 _startTimeMinutes;
  
  /**
   * End Date  of rainfall (Julian date)
   * See ICD Table V
   * Stored in hw50
   */
  ui16 _endTimeJDate;

  /**
   * End Time of rainfall (minutes)
   * See ICD Table V
   * Stored in hw51
   */
  ui16 _endTimeMinutes;

  /**
   * Mean field Bias. Stored in hw52
   */ 
  fl32 _meanFieldBias;

  /**
   * Effective number of G_R Pairs (sample size)
   * Stored in hw53
   */ 
  fl32 _numGRPairs;
 
};

#endif





