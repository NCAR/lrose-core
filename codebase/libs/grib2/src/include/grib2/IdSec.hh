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
 * @file IdSec.hh
 * @brief Section 1, Identification Section 
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_ID_SECTION
#define _GRIB2_ID_SECTION

#include <iostream>
#include <grib2/constants.h>
#include <grib2/GribSection.hh>

using namespace std;

namespace Grib2 {
/** 
 * @class IdSec
 * @brief Section 1, Identification Section 
 *
 * Identification Section is section 1 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect1.shtml
 */
class IdSec : public GribSection {

public:
  /**
   * @brief Default and only constructor
   */
  IdSec();

  ~IdSec(){};

  /** @brief Unpacks the Id Section
   *  @param[in] idPtr Pointer to start of the Id section
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack( ui08 *idPtr );

  /** @brief Packs up the Id Section
   *  @param[in] idPtr Pointer to start of location to pack to
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int pack( ui08 *idPtr );

  /** @brief Print to stream/file all information contained in the Id section */
  void print (FILE *);

  // Get Functions

  /** @brief Get the reference year */
  inline si32 getYear() { return _year; };

  /** @brief Get the reference month */
  inline si32 getMonth() { return _month; };

  /** @brief Get the reference day */
  inline si32 getDay() { return _day; };

  /** @brief Get the reference hour */
  inline si32 getHour() { return _hour; };

  /** @brief Get the reference min */
  inline si32 getMin() { return _min; };

  /** @brief Get the reference sec */
  inline si32 getSec() { return _sec; };

  /** @brief Get the generating center and subCenter name */
  string getGeneratingCenterName();

  /** @brief Get the generating center id number */
  inline si32 getCenterId() { return _generatingCenter; };

  /** @brief Get the generating sub-center id number */
  inline si32 getSubCenterId() { return _subCenter; };

  /** @brief Get the reference unix time */
  time_t getGenerateTime() const;

  /** @brief Get the data type */
  inline si32 getProcessedDataType() { return _proccesedDataType; };

  /** @brief Get the production status */
  inline si32 getProductionStatus() { return _productionStatus; };

  /**  @brief Get the local table version number */
  inline si32 getLocalTableVersion() { return _localTableVer; };
  
  /** @brief Get the Signifigance of the Reference time */
  inline si32 getGenerateTimeType() { return _referenceTimeSig; };
  


  // Set Functions

  /** @brief Set the generating center id number */
  inline void setCenterId(si32 generatingCenter) { _generatingCenter = generatingCenter; };

  /** @brief Set the generation sub-centre id number */
  inline void setSubCenterId(si32 subCenter) { _subCenter = subCenter; };

  /** @brief Set the reference unix time */
  void setGenerateTime(time_t generateTime);

  /** @brief Set the Signifigance of the Reference time */
  inline void setGenerateTimeType(si32 referenceTimeSig) { _referenceTimeSig = referenceTimeSig; };

  /** @brief Set the production status code */
  inline void setProductionStatus(si32 productionStatus) { _productionStatus = productionStatus; };

  /** @brief Set the processed data type code */
  inline void setProccesedDataType(si32 proccesedDataType) { _proccesedDataType = proccesedDataType; };

  /**  @brief Set the local table version number */
  inline void setLocalTableVersion(si32 localTableVer) { _localTableVer = localTableVer; };

  /**  @brief Set the Master table version number */
  inline void setMasterTableVersion(si32 masterTableVer) { _masterTableVer = masterTableVer; };


private:

  /** @brief Originating/generating centers. ON388 - TABLE 0 */
  static const string _centers[256];

  /** @details Struct used for subCenter names, centerNumber is index in _centers */
  typedef struct {
    si32 center;
    si32 subCenter;
    string name;
  } _GRIB2SubCenter;

  /** @brief Size of table _subCenters */
  static const ui32 _subCenters_numElements = 16;
  /** @brief Generating SubCenter table */
  static const _GRIB2SubCenter _subCenters[_subCenters_numElements];

  /** @brief Originating/generating center
   *
   * See http://www.nco.ncep.noaa.gov/pmb/docs/on388/table0.html */
  si32 _generatingCenter;

  /** @brief Originating/generating sub-center (allocated by generating centre) */
  si32 _subCenter;

  /** @brief Grib2 Master tales version */
  si32 _masterTableVer;

  /** @brief Local tables version  0 = local tables not used */
  si32 _localTableVer;

  /** @brief Signifigance of Reference time
   *
   * 0 = Analysis
   * 1 = Start of Forecast
   * 2 = Verifying Time of Forecast
   * 3 = Observation Time */
  si32 _referenceTimeSig;

  /** @brief Reference Time year */          
  si32 _year;

  /** @brief Reference Time month */
  si32 _month;

  /** @brief Reference Time day */
  si32 _day;

  /** @brief Reference Time hour */
  si32 _hour;

  /** @brief Reference Time minute */
  si32 _min;

  /** @brief Reference Time second */
  si32 _sec;

  /** @brief Production status of data
   *
   * 0 = Operational Products
   * 1 = Operational Test Products
   * 2 = Research Products
   * 3 = Re-Analysis Products */
  si32 _productionStatus;

  /** @brief Type of processed data 
   *
   * 0 = Analysis Products
   * 1 = Forecast Products
   * 2 = Analysis and Forecast Products
   * 3 = Control Forecast Products
   * 4 = Perturbed Forecast Products
   * 5 = Control and Perturbed Forecast Products
   * 6 = Processed Satellite Observations
   * 7 = Processed Radar Observations */
  si32 _proccesedDataType;


};

} // namespace Grib2

#endif
