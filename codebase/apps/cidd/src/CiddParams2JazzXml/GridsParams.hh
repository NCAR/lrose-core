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
 * @file GridsParams.hh
 *
 * @class GridsParams
 *
 * Class controlling access to the GRIDS section of the CIDD parameter
 * file.
 *  
 * @date 9/24/2010
 *
 */

#ifndef GridsParams_HH
#define GridsParams_HH

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "GridField.hh"
#include "MainParams.hh"
#include "ParamSection.hh"

using namespace std;


class GridsParams : public ParamSection
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  GridsParams();
  
  /**
   * @brief Destructor
   */

  virtual ~GridsParams(void);
  

  /**
   * @brief Initialize the parameters from the given buffer.
   *
   * @param[in] params_buf Parameter file buffer.  Must be null-terminated.
   * @param[in] buf_size   Size of the parameter buffer.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const MainParams &main_params,
	    const char *params_buf, const size_t buf_size);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the list of grid fields.
   *
   * @return Returns the list of grid fields.
   */

  const vector< GridField > &getGridFields() const
  {
    return _gridFields;
  }

  
 protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * Maximum length of a parse line.
   */

  static const size_t PARSE_LINE_LEN;
  
  /**
   * @brief Maximum number of tokens in a line.
   */

  static const int MAX_TOKENS;
  
  /**
   * @brief Maximum length of a token.
   */

  static const int MAX_TOKEN_LEN;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Temporary buffer for parsing the lines in the parameter file.
   */

  char *_lineBuffer;
  
  /**
   * @brief Token buffer.
   */

  char **_tokens;
  
  /**
   * @brief The grid fields found in the parameter file.
   */

  vector< GridField > _gridFields;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  bool _initDataLinks(const MainParams &main_params, const char *param_buf,
		      long param_buf_len, long line_no);
  

  bool _parseLine(const MainParams &main_params,
		  const char *parse_line, const long line_len);
  
};


#endif
