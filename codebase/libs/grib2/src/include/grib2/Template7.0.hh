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
 * @file Template7.0.hh
 * @author Jason Craig
 * @author Gary Blackburn
 * @date   Aug 2006
 */

#ifndef _GRIB2_TEMPLATE_7_PT_0 
#define _GRIB2_TEMPLATE_7_PT_0

#include <cstdio>
#include <grib2/DataTemp.hh>

using namespace std;

namespace Grib2 {

/** 
 * @class Template7_pt_0
 *
 * Grid point data - Simple Packing 
 *
 */
class Template7_pt_0: public DataTemp {

public:

  /** 
   * @brief Internal constructor used during grib2 encoding/decoding
   *
   * @param[in] sectionsPtr Struct containing pointers to all
   * sections that come before this one.
   */
  Template7_pt_0(Grib2Record::Grib2Sections_t sectionsPtr);

  virtual ~Template7_pt_0();

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


private: 
  

};

} // namespace Grib2

#endif
