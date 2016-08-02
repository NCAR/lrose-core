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
 * @file DataRepTemp.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_DATA_REP_TEMPLATE
#define _GRIB2_DATA_REP_TEMPLATE

#include <dataport/port_types.h>
#include <grib2/Grib2Record.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

/** 
 * @class DataRepTemp
 *
 * DataRepTemp is an abstract class for grib Data Representation Templates.
 * As an abstract class no object can be declared as type DataRepTemp, it
 * can only be used with as a reference or pointer.
 *
 * Known derived classes include:  Template5.0.hh, Template5.2.hh, Template5.3.hh
 * Template5.4000.hh, and Template5.41.hh
 *
 * Data Representation Template is part of the Data Representation Section (DRS).
 *
 * @note The pack and unpack routines are static methods in GribSection.hh
 *
 */
class DataRepTemp {

public:

  /** 
   * @brief Defualt Constructor usually called only from derived classes.
   */
  DataRepTemp();

  /** 
   * @brief Internal constructor used during grib2 decoding
   *
   * @param[in] sectionsPtr Struct containing pointers to all
   * sections that come before this one.
   */
  DataRepTemp(Grib2Record::Grib2Sections_t sectionsPtr);

  virtual ~DataRepTemp();

  /** @brief Unpack a Data Representation Template 
   *  @param[in] projPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int unpack( ui08 *projPtr ) = 0;

  /** @brief Pack up this Data Representation Template
   *  @param[in] projPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int pack( ui08 *projPtr ) = 0;

  /** @brief Print to stream/file all information for this template */
  virtual void print (FILE *) const = 0;

  /** @details Struct for data constants */
  typedef struct {
    /** DataRep Template number 
     * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table5-0.shtml */
    si32 templateNumber;
    /** Data Reference Value (R) IEEE 32- bit floating-point value */
    fl32 referenceValue;
    /** Data Binary Scale Factor (E) */
    si32 binaryScaleFactor;
    /** Data Decimal Scale Factor (D) */
    si32 decimalScaleFactor;
    /** Number of bits used for each packed value for 
     * simple packing, or for each group reference value 
     * for complex packing or spatial differencing */
    si32 numberOfBits;
    /** Type of original field values 
     * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table5-1.shtml */
    si32 origFieldTypes;
   } data_representation_t;  

  /** @brief Get the data constants struct */
  inline data_representation_t getDrsConstants() { return _dataRepresentation; };

  /** @brief Get the original field value types */
  si32 getOrigFieldTypes() { return _dataRepresentation.origFieldTypes; };

  /** @brief Get the size of the packed template */
  virtual si32 getTemplateSize() = 0;

  /** @brief Set the data constants struct */
  inline void setDrsConstants(data_representation_t dataRep) { _dataRepresentation = dataRep; };

  /** @brief Set the original field value types */
  inline void setOrigFieldTypes(si32 origFieldTypes) { _dataRepresentation.origFieldTypes = origFieldTypes; };

  /** @brief Used internally to set pointers to other sections of the grib file.
   *  @param[in] sectionsPtr Sections pointer struct */
  void setSectionsPtr(Grib2Record::Grib2Sections_t sectionsPtr);

protected:

  /** @brief Struct containing pointers to other parts of this grib file 
   *  @note Only sections appearing before the DRS will have valid pointers */
  Grib2Record::Grib2Sections_t _sectionsPtr;

  /** @brief The data constants struct */
  data_representation_t  _dataRepresentation;


private: 


};

} // namespace Grib2

#endif

