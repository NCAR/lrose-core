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
 * @file Template4.30.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_TEMPLATE_4_PT_30 
#define _GRIB2_TEMPLATE_4_PT_30

#include <dataport/port_types.h>
#include <grib2/ProdDefTemp.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

/** 
 * @class Template4_pt_30
 *
 * A derived Product Definition Template class for satellite products.
 * Note that this template is deprecated, but we need to handle it for
 * incoming files.
 *
 * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_temp4-30.shtml
 *
 * @note The pack and unpack routines are static methods in Grib2::GribSection
 */
class Template4_pt_30: public ProdDefTemp {

public:
  /**
   * @class BandInfo
   *
   * Information about each of the bands in the satellite.
   *
   * See http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_temp4-30.shtml
   */

  class BandInfo
  {
  public:
    /** @brief Unpack a Product Definition Template 
     *  @param[in] projPtr Pointer to start of template
     *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
    int unpack (ui08 *projPtr);

    /** @brief Pack up this Product Definition Template
     *  @param[in] projPtr Pointer to start of location to pack template
     *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
    int pack (ui08 *projPtr);

    /** @brief Get the size of the info when packed in the GRIB file.
     *  @return Size of packed info in bytes */ 
    virtual si32 getPackedSize() { return BAND_INFO_SIZE; };

    /** @brief Print to stream/file all information for this template */
    void print (FILE *stream, const size_t band_num) const;

  private :
    /** @brief The size of the info when packed in the GRIB file */
    static const si32 BAND_INFO_SIZE;

    /** @brief Satellite series of band nb (code table defined by originating/generating centre) */
    si32 _satelliteSeries1;
    /** @brief Satellite series of band nb (code table defined by originating/generating centre) */
    si32 _satelliteSeries2;
    /** @brief Instrument types of band nb (code table defined by originating/generating centre)  */
    si32 _instrumentType;
    /** @brief Scale factor of central wave number of band nb */
    si32 _centralWaveNumScaleFactor;
    /** @brief Scaled value of central wave number of band nb (units: m-1) */
    si32 _centralWaveNumScaled;
  };
  
    
  /**
   * @brief Default constructor for use in passing object to GribFile.addField()
   *
   * @note After creation.. all public values variables below should be set
   * before passing this object to GribFile. 
   */
  Template4_pt_30();

  /**
   * @brief This constructor is used internally to the grib2 library when reading
   * a grib2 file.
   */
  Template4_pt_30(Grib2Record::Grib2Sections_t sectionsPtr);

  virtual ~Template4_pt_30();

  /** @brief Unpack a Product Definition Template 
   *  @param[in] projPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int unpack (ui08 *projPtr);

  /** @brief Pack up this Product Definition Template
   *  @param[in] projPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int pack (ui08 *projPtr);

  /** @brief Print to stream/file all information for this template */
  void print (FILE *) const;

  /** @brief Get the record summary for this grib record 
   *  @param[out] summary Record summary struct */
  void getRecSummary (Grib2Record::rec_summary_t *summary);


  /** @brief Get the forecast time for this grib record 
   *  @return Forecast lead time in seconds */
  long int getForecastTime() const { return 0; }
  

  /** @brief Get the size of the packed derived template class. 
   *  @return Size of packed template in bytes */ 
  virtual si32 getTemplateSize() { return TEMPLATE4_PT_30_SIZE; };

  /** @brief Type of generating process */
  si32 _processType;             
  /** @brief Observation generating process identifier */
  si32 _obsProcessId;        
  /** @brief Number of contributing spectral bands (NB) */
  ui32 _numBands; 
  /** @brief Repeating information defining each band */
  vector< BandInfo > _bands;
  
protected:

private: 
  
  static const si32 TEMPLATE4_PT_30_SIZE;

};

} // namespace Grib2

#endif

