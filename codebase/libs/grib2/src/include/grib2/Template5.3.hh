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
 * @file Template5.3.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_TEMPLATE_5_PT_3 
#define _GRIB2_TEMPLATE_5_PT_3

#include <cstdio>
#include <grib2/DataRepTemp.hh>

using namespace std;

namespace Grib2 {

/** 
 * @class Template5_pt_3
 *
 * Grid point data - Complex Packing and Spatial Differencing 
 *
 */
class Template5_pt_3: public DataRepTemp {

public:

  /** @brief Constructor for use in passing to GribFile.addField()
   *
   *  Public member variables are filled in during packing routine and do not 
   *  need to be explicitly set.
   *  @param[in] decimalScaleFactor Floating point precision to use in simple packing
   *  @param[in] spatialDifferenceOrder 1 = First-Order Spatial Differencing, 
   *    2 = Second-Order Spatial Differencing
   *  @param[in] origFieldTypes Original data type,
   *    0 = Floating point data, 1 = Integer data */
  Template5_pt_3(si32 decimalScaleFactor, si32 spatialDifferenceOrder,
		 si32 origFieldTypes = 0);

  /** 
   * @brief Internal constructor used during grib2 decoding
   *
   * @param[in] sectionsPtr Struct containing pointers to all
   * sections that come before this one.
   */
  Template5_pt_3(Grib2Record::Grib2Sections_t sectionsPtr);

  virtual ~Template5_pt_3();

  /** @brief Unpack a Data Representation Template 
   *  @param[in] projPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int unpack (ui08 *projPtr);

  /** @brief Pack up this Data Representation Template
   *  @param[in] projPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int pack (ui08 *projPtr);

  /** @brief Print to stream/file all information for this template */
  void print (FILE *) const;

  /** @brief Group Splitting method, 0 = Row by Row Splitting, 1 = General Group Splitting */
  si32 _splittingMethod;

  /** @brief Included Missing values, 
   * 0 = No explicit missing values included 
   * 1 = Primary missing values included 
   * 2 = Primary and secondary missing values included */
  si32 _missingType;

  /** @brief Primary missing value */
  fl32 _primaryMissingVal;

  /** @brief Secondary missing value */
  fl32 _secondaryMissingVal;

  /** @brief Number of groups of data values */
  si32 _numberGroups;

  /** @brief Reference value for group widths */
  si32 _groupWidths;

  /** @brief Number of bits used for the group widths */
  si32 _groupWidthsBits;

  /** @brief Reference value for group lengths */
  si32 _groupLength;

  /** @brief Length increment for the group lengths */
  si32 _lengthIncrement;

  /** @brief Length of the last group */
  si32 _lengthOfLastGroup;

  /** @brief Number of bits used for the group length */
  si32 _groupLengthsBits;
  /** @brief Order of spatial differencing, 1 = First-Order Spatial Differencing, 
   *    2 = Second-Order Spatial Differencing */
  si32 _spatialDifferenceOrder;
  /** @brief Number of octets required in the data section to specify extra 
   * descriptors needed for spatial differencing */
  si32 _octetsRequired;

  /** @brief Get the size of the packed template */
  virtual si32 getTemplateSize() { return TEMPLATE5_PT_3_SIZE; };

protected:


private: 
  static const si32 TEMPLATE5_PT_3_SIZE;

};

} // namespace Grib2

#endif

