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
 * @file SdsField.hh
 *
 * @class SdsField
 *
 * Base class for a field that is stored in the SDS section of a TRMM file.
 *  
 * @date 10/31/2008
 *
 */

#ifndef SdsField_HH
#define SdsField_HH

#include <string>

#include "HdfFile.hh"

using namespace std;

/** 
 * @class SdsField
 */

class SdsField
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] debug_flag Debug flag.
   * @param[in] verbose_flag Verbose debug flag.
   */

  SdsField(const bool debug_flag = false, const bool verbose_flag = false);
  

  /**
   * @brief Destructor
   */

  virtual ~SdsField(void);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief Maximum number of characters in a single field name in
   *        a TRMM file.
   */

  static const int MAX_FIELD_NAME_LEN;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Debug flag.
   */

  bool _debug;
  
  /**
   * @brief Verbose debug flag.
   */

  bool _verbose;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Read the data for the given SDS field.
   *
   * @param[in] hdf_file The HDF file to read from.
   * @param[in] field_name The name of the SDS field to read.
   * @param[out] dimensions The dimensions of the SDS data read.
   *
   * @return Returns a pointer to the read data on success, 0 on failure.
   */

  fl64 *_readData(const HdfFile &hdf_file, const string &field_name,
		  vector< int > &dimensions);
  

};


#endif
