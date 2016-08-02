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
 * @file FlatInput.hh
 *
 * @class FlatInput
 *
 * Class that retrieve input files from RAP flat directories.
 *  
 * @date 6/11/2010
 *
 */

#ifndef FlatInput_HH
#define FlatInput_HH

#include "Input.hh"

/**
 * @class FlatInput
 */

class FlatInput : public Input
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] debug_flag Debug flag.
   */

  FlatInput(const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  ~FlatInput(void);
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */

  bool init();
  

protected:

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Set the read time in the MDV request.
   *
   * @param[in,out] request MDV request.
   * @param[in] url The URL for the data.
   * @param[in] field_name Field name, used for debug output.
   * @param[in] max_input_valid_secs Maximum number of seconds that this
   *                                 data field is valid.
   * @param[in] trigger_info Trigger information to use for data times.
   */

  virtual void _setReadTime(DsMdvx &request,
			    const string &url,
			    const string &field_name,
			    const int max_input_valid_secs,
			    const TriggerInfo &trigger_info) const;


};


#endif
