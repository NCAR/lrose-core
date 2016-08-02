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
 * @file DRS.hh
 * @brief Section 5, Data Representation Section
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_DRS_
#define _GRIB2_DRS_

#include <grib2/Template5.41.hh>
#include <grib2/Template5.4000.hh>
#include <grib2/Template5.0.hh>
#include <grib2/Template5.2.hh>
#include <grib2/Template5.3.hh>
#include <grib2/DataRepTemp.hh>
#include <grib2/Grib2Record.hh>
#include <grib2/constants.h>
#include <grib2/GribSection.hh>

using namespace std;

namespace Grib2 {
/** 
 * @class DRS
 * @brief Section 5, Data Representation Section
 *
 * Data Representation Section (DRS) is section 5 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect5.shtml
 */
class DRS: public GribSection {

public:

  /**
   * @brief This constructor is generally only used from Grib2::Grib2Record::unpack() for reading a Grib2 file
   */
  DRS(Grib2Record::Grib2Sections_t sectionsPtr);

  /**
   * @brief This constructor is generally only used from Grib2::Grib2Record::addField() for creating a Grib2File
   */
  DRS(Grib2Record::Grib2Sections_t sectionsPtr, si32 dataRepNum, 
      DataRepTemp *dataRepTemplate);

  ~DRS();
   
  /** @brief Unpacks the whole Data Representation Section
   *  @param[in] drsPtr Pointer to start of the DRS section
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack( ui08 *drsPtr );

  /** @brief Packs up the Data Representation Section
   *  @param[in] drsPtr Pointer to start of location to pack to
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int pack( ui08 *drsPtr );

  /** @brief Print to stream/file all information contained in the DRS section */
  void  print(FILE *) const;

  // Get Routines

  /** @brief Get the Data Representation template pointer */
  inline DataRepTemp *getDrsTemplate() { return _dataRepresentation; };

  /** @brief Get the number of data points after packing */
  inline si32 getNumPackedDataPoints() { return _numPackedDataPoints; };

  /** @brief Get the data constants struct */
  inline DataRepTemp::data_representation_t getDrsConstants() 
    { return _dataRepresentation->getDrsConstants(); };

  // Set Routines

  /** @brief Set the data constants struct */
  inline void setDrsConstants(DataRepTemp::data_representation_t dataRep) 
    { _dataRepresentation->setDrsConstants(dataRep); };

  /** @brief Set the number of data points after applying the packing method */
  inline void setNumPackedDataPoints(si32 numPackedDataPoints ) 
    { _numPackedDataPoints = numPackedDataPoints; };

private:

  /** @brief Struct containing pointers to all other sections of the Grib2 file. 
   * Only sections appearing before the DRS however will have valid pointers */
  Grib2Record::Grib2Sections_t _sectionsPtr;

  /** @brief Number of data points defined in the bitmap or total number of data points 
   *
   * If there is a bitMap this is the number of data points defined in the bitmap
   * When there is no bitMap this is the total number of data points */
  si32 _numPackedDataPoints;

  /** @briefTemplate number of DataRepTemp */
  si32 _dataTemplateNum;

  /** @brief Pointer to the Data Representation Template
   *
   * Stores additional information needed for various encoding / decoding methods */
  DataRepTemp *_dataRepresentation;

};

} // namespace Grib2;

#endif
