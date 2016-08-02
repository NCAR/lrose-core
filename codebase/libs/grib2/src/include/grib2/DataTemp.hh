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
 * @file DataTemp.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_DATA_TEMPLATE
#define _GRIB2_DATA_TEMPLATE

#include <dataport/port_types.h>
#include <grib2/GribSection.hh>
#include <grib2/Grib2Record.hh>
#include <grib2/DataRepTemp.hh>
#include <grib2/constants.h>

using namespace std;

namespace Grib2 {

/** 
 * @class DataTemp
 *
 * DataTemp is an abstract class for grib Data Templates.
 * As an abstract class no object can be declared as type DataTemp, it
 * can only be used with as a reference or pointer.
 *
 * Known derived classes include:  Template7.0.hh, Template7.2.hh, 
 * Template7.4000.hh, and Template7.41.hh
 *
 * Data Template is part of the Data Section (DS).
 *
 */
class DataTemp {

public:

  /** 
   * @brief Internal constructor used during grib2 decoding
   *
   * @param[in] sectionsPtr Struct containing pointers to all
   * sections that come before this one.
   */
  DataTemp(Grib2Record::Grib2Sections_t sectionsPtr);

  virtual ~DataTemp();
  
  /** @brief Unpack a Data Template 
   *  @param[in] dataPtr Pointer to start of template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int unpack( ui08 *dataPtr ) = 0;

  /** @brief Pack up this Data Template
   *  @param[in] dataPtr Pointer to start of location to pack template
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  virtual int pack( fl32 *dataPtr ) = 0;

  /** @brief Print to stream/file all information for this template */
  virtual void print(FILE *output) const = 0;

  /** @brief Deletes the unpacked data object */
  void freeData();

  /** @brief Get the unpacked data pointer */
  inline fl32 *getData() { return _data; };

  /** @brief Get the packed data pointer, Packed data is of variable number of bits */
  inline ui08 *getPackedData() { return (ui08 *)_pdata; };

  /** @brief Get the packed data size in bytes */
  inline si32 getPackedDataSize() { return _lcpack; };

  /** @brief Get the size of the packed template */
  inline si32 getTemplateSize() { return _lcpack + 5; };

protected:

  /** @brief Apply the Bit map to unpack the data pointer. Data will be saved to _data */
  void _applyBitMapUnpack(fl32 *data);

  /** @brief Apply the Bit map to pack up the data pointer 
   *
   * The return pointer must be freed by the caller. UNLESS 
   * the return pointer equals the input pointer in which no bit map was applied. */
  fl32 *_applyBitMapPack(fl32 *data);

  /** @brief Struct containing pointers to other parts of this grib file 
   *  @note Only sections appearing before the DS will have valid pointers */
  Grib2Record::Grib2Sections_t _sectionsPtr;

  /** @brief Uncompressed data pointer */
  fl32 *_data;

  /** @brief Size (bytes) of compressed data */
  si32 _lcpack;

  /** @brief Compressed data pointer */
  fl32 *_pdata;

};

} // namespace Grib2

#endif

