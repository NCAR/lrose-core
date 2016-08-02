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
 * @file PDS.hh
 * @brief Section 4, Product Definition Section
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_PDS_
#define _GRIB2_PDS_

#include <grib2/GribSection.hh>
#include <grib2/Grib2Record.hh>
#include <grib2/ProdDefTemp.hh>
#include <grib2/Template4.0.hh>
#include <grib2/Template4.1.hh>
#include <grib2/Template4.2.hh>
#include <grib2/Template4.5.hh>
#include <grib2/Template4.6.hh>
#include <grib2/Template4.7.hh>
#include <grib2/Template4.8.hh>
#include <grib2/Template4.9.hh>
#include <grib2/Template4.10.hh>
#include <grib2/Template4.11.hh>
#include <grib2/Template4.12.hh>
#include <grib2/Template4.15.hh>
#include <grib2/Template4.30.hh>

using namespace std;

namespace Grib2 {
/** 
 * @class PDS
 * @brief Section 4, Product Definition Section
 *
 * Product Definition Section (PDS) is section 4 in Grib2.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_sect4.shtml
 */
class PDS : public GribSection {

public:
  /**
   * @brief This constructor is generally only used from Grib2::Grib2Record::unpack() for reading a Grib2 file
   */
  PDS(Grib2Record::Grib2Sections_t sectionsPtr);

  /**
   * @brief This constructor is generally only used from Grib2::Grib2Record::addField() for creating a Grib2File
   */
  PDS(Grib2Record::Grib2Sections_t sectionsPtr, si32 prodDefNum,
      ProdDefTemp *productTemplate);

  ~PDS();
   
  /** @brief Unpacks the whole Product Definition Section
   *  @param[in] pdsPtr Pointer to start of the PDS section
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int unpack( ui08 *pdsPtr );

  /** @brief Packs up the Product Definition Section
   *  @param[in] pdsPtr Pointer to start of location to pack to
   *  @return Either Grib2::GRIB_SUCCESS or Grib2::GRIB_FAILURE */
  int pack( ui08 *pdsPtr ) ;

  /** @brief Print to stream/file all information contained in the PDS section */
  void print(FILE *) const;

  /** @brief Get the record summary for this grib record 
   *  @param[out] summary Record summary struct */
  void getRecSummary (Grib2Record::rec_summary_t *summary);

  /** @brief Get the forecast time for this grib record 
   *  @return Forecast lead time in seconds */
  long int getForecastTime() const;

  /** @brief Get the Product Definition Template number */
  inline ui32 getProdDefTempNum () {return _prodDefTempNum; };

  /** @brief Get the Product Definition Template */
  inline ProdDefTemp *getProdDefTemp () { return _prodDefinition; };

  /** @brief Returns the Generating Process Name 
   *  @return A string with name of the generating process */
  inline string getGeneratingProcess() const { return _prodDefinition->getGeneratingProcess(); };

protected:

  
private:

  /** @brief Struct containing pointers to all other sections of the Grib2 file. 
   * Only sections appearing before the PDS however will have valid pointers */
  Grib2Record::Grib2Sections_t _sectionsPtr;

  /** @brief Number of optional list of coordinate values 
   * (this is currently unimplemented and is set to 0) */
  ui32 _coordinateValsize;

  /** @brief Template number of ProdDefTemp */
  ui32 _prodDefTempNum;

  /** @brief The ProdDefTemp stores additional infromation needed for the product type */
  ProdDefTemp *_prodDefinition;

};

} // namespace Grib2

#endif

   
