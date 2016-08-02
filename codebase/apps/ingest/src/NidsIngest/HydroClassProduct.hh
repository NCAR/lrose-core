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
 * @file HydroClassProduct.hh
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Hydrometeor Classification products N0H, NAH, N1H, NBH, N2H,N3H.
 *        Product code 165.
 * 
 * @class HydroClassProduct
 * @brief Class for decoding Graphic Product Message Description Block for 
 *        Hydrometeor Classification products N0H, NAH, N1H, NBH, N2H,N3H.
 *        Product code 165.
 */
#ifndef HYDROCLASSPRODUCT_H
#define HYDROCLASSPPRODUCT_H

#include <dataport/port_types.h>
#include "Product.hh"
#include <cmath>

using namespace std;

class HydroClassProduct: public Product {
  
public:

  /**
   * Constructor
   * Initializes public and private members
   * @params[in] filePtr  Pointer to file containing radar data
   * @params[in] byteSwapFlag  Flag to indicate byte swapping the various 
   *                           message components for handling endian issues 
   * @params[in] debugFlag  Flag to indicate debug messaging
   */
  HydroClassProduct ( FILE *filePtr, bool byteSwapFlag, 
		      bool debugFlag);

  /**
   * Destructor
   */
  ~HydroClassProduct();

  /** 
   * Converts byte data to float data.  
   * @param[in] x  Input byte
   * @return decoded float value which is just x.
   */
  fl32 convertData(ui08 x);

  /**
   * Converts short data to float data.
   * not used by this product
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
   * @return 1 if product is compressed and 0 otherwise. 
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
   * Elevation angle. See ICD Table V
   * Stored in hw30
   */
  fl32 _elev;

  /**
   * Mode filter size-- range is [1,15]
   * stored in hw47. See ICD Table V.
   */
  fl32 _modeFilterSize;

  /**
   * Flag to indicate compression of radial data
   * Stored in hw51. See ICD Table V.
   */
  ui16 _isCompressed;
  
  /**
   * If data are compressed, this is the uncompressed data size in bytes
   * Stored in hw52 and hw53. See ICD Table V.
   */
  ui32 _uncompProdSize;
};

#endif





