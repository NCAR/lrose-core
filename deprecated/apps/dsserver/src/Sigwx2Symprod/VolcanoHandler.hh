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
 *
 * @file VolcanoHandler.hh
 *
 * @class VolcanoHandler
 *
 * Class for handling volcano data.
 *  
 * @date 10/10/2009
 *
 */

#ifndef _VolcanoHandler_hh
#define _VolcanoHandler_hh

#include <map>
#include <string>

#include <Spdb/Spdb.hh>
#include <Spdb/Symprod.hh>
#include <toolsa/MemBuf.hh>
#include <xmlformats/SigwxVolcano.hh>

#include "BoundingBox.hh"
#include "DisplayItem.hh"
#include "IconDef.hh"
#include "Params.hh"
#include "ProductHandler.hh"

using namespace std;

/**
 * @class VolcanoHandler
 */

class VolcanoHandler : public ProductHandler
{

public:

  /**
   * @brief Constructor.
   */

  VolcanoHandler(Params *params, const int debug_level = 0);
  
  /**
   * @brief Destructor.
   */

  virtual ~VolcanoHandler();
  

  ///////////////////////
  // Rendering methods //
  ///////////////////////

  /**
   * @brief Convert the volcano data to Symprod format.
   *
   * @param[in] dir_path Data directory path.
   * @param[in] prod_id SPDB product ID.
   * @param[in] prod_label SPDB product label.
   * @param[in] chunk_ref Chunk header for this data.
   * @param[in] aux_ref Auxilliary chunk header for this data.
   * @param[in] spdb_data Data pointer.
   * @param[in] spdb_len Length of data buffer.
   * @param[in,out] symprod_buf Symprod buffer.
   * @param[in] bbox Bounding box of the display area.
   *
   * @return Returns 0 on success, -1 on failure.
   */

  int convertToSymprod(const string &dir_path,
		       const int prod_id,
		       const string &prod_label,
		       const Spdb::chunk_ref_t &chunk_ref,
		       const Spdb::aux_ref_t &aux_ref,
		       const void *spdb_data,
		       const int spdb_len,
		       MemBuf &symprod_buf,
		       const BoundingBox &bbox) const;
  
};

  
#endif
