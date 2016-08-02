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
 * @file SymprodsParams.hh
 *
 * @class SymprodsParams
 *
 * Class controlling access to the SYMPRODS section of the CIDD parameter
 * file.
 *  
 * @date 10/5/2010
 *
 */

#ifndef SymprodsParams_HH
#define SymprodsParams_HH

#include <vector>

#include "MainParams.hh"
#include "SymprodField.hh"
#include "TdrpParamSection.hh"

using namespace std;


class SymprodsParams : public TdrpParamSection
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor
   */

  SymprodsParams();
  
  /**
   * @brief Destructor
   */

  virtual ~SymprodsParams(void);
  

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
   * @brief Get the list of symprod fields.
   *
   * @return Returns the list of symprod fields.
   */

  const vector< SymprodField > &getSymprodFields() const
  {
    return _symprodFields;
  }


 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The symprod fields found in the parameter file.
   */

  vector< SymprodField > _symprodFields;
  
};


#endif
