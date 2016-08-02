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
 * @file Template7.41.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_TEMPLATE_7_PT_41 
#define _GRIB2_TEMPLATE_7_PT_41

#include <grib2/DataTemp.hh>
#include <grib2/Grib2Record.hh>
#include <cstdio>
#include <png.h>

using namespace std;

namespace Grib2 {

/** 
 * @class Template7_pt_41
 *
 * Grid point data - PNG Code Stream
 *
 */
class Template7_pt_41: public DataTemp {

public:

  /** 
   * @brief Internal constructor used during grib2 encoding/decoding
   *
   * @param[in] sectionsPtr Struct containing pointers to all
   * sections that come before this one.
   */
  Template7_pt_41(Grib2Record::Grib2Sections_t sectionsPtr);

  virtual ~Template7_pt_41();

  /** @brief Unpack the data
   *  @param[in] dataPtr Pointer to start of packed data
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int unpack (ui08 *dataPtr);

  /** @brief Pack up this data
   *  @param[in] dataPtr Pointer to start of location to pack data to
   *  @return Either GRIB_SUCCESS or GRIB_FAILURE */
  int pack (fl32 *dataPtr);

  /** @brief Print to stream/file the data */
  virtual void print(FILE *output) const;


protected:


  /** @brief Function to read a PNG stream from memory instead of a file on disk
   *  @details Passed to the png library for reading a png file */
 static void user_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length);

  /** @brief Function to write a PNG stream to memory instead of a file on disk
   *  @details Passed to the png library for writing a png file */
 static void user_write_data(png_structp png_ptr, png_bytep data, png_uint_32 length);
  /** @brief Data flush function for PNG stream 
   *  @details Passed to the png library */
 static void user_flush_data(png_structp png_ptr);


private: 
  /** @brief Decode a png data stream */
  int decode_png (char *input, char *output);

  /** @brief Encode a png data stream */
  int encode_png (ui08 *cin,int width,int height,int nbits, char *out);
  
  /** @details Struct for representing a png stream */
  struct png_stream {
    /** location to write PNG stream  */
    char *stream_ptr;
    /**  number of bytes written       */
    si32 stream_len;
  };
  typedef struct png_stream png_stream;

};

} // namespace Grib2

#endif
