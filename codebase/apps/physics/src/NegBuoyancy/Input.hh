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
 * @file Input.hh
 *
 * @class Input
 *
 * Base class for classes that retrieve input files.
 *  
 * @date 6/10/2010
 *
 */

#ifndef Input_HH
#define Input_HH

#include <string>

#include <dsdata/TriggerInfo.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>


/**
 * @class Input
 */

class Input
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

  Input(const bool debug_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~Input(void);
  

  /**
   * @brief Set the remapping projection to use.
   *
   * @param[in] proj The projection to use.
   */

  void setRemap(const MdvxPjg &proj)
  {
    _inputProj = proj;
    _remap = true;
  }
  

  /**
   * @brief Read the indicated field data.
   *
   * @param[in] trigger_info The trigger information for the data.
   * @param[in] url The URL for the data.
   * @param[in] field_name The field name for the data.
   * @param[in] field_num The field number for the data.
   * @param[in] max_input_valid_secs Maximum number of seconds that this
   *                                 data field is valid.
   *
   * @return Returns a pointer to the data field on success, 0 on failure.
   */

  MdvxField *readField(const TriggerInfo &trigger_info,
		       const string &url,
		       const string &field_name,
		       const int field_num,
		       const int max_input_valid_secs) const;
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  
  /**
   * @brief Flag indicating whether to remap the input fields.
   */

  bool _remap;
  
  /**
   * @brief The projection to use for remapping.
   */

  MdvxPjg _inputProj;
  

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
			    const TriggerInfo &trigger_info) const = 0;


};


#endif
