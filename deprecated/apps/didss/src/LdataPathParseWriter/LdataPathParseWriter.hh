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
 * @file LdataPathParseWriter.hh
 * @brief  Allows the user to write a latest_data_info file
 *         by parsing an input file path
 * @class LdataPathParseWriter
 * @brief  Allows the user to write a latest_data_info file
 *         by parsing an input file path
 */

#ifndef LdataPathParseWriter_HH
#define LdataPathParseWriter_HH

#include <string>
#include "Args.hh"

class DsLdataInfo;

class LdataPathParseWriter
{
public:
  
  /**
   * Constructor
   * @param[in] argc
   * @param[in] argv
   */
  LdataPathParseWriter (int argc, char **argv);

  /**
   * Destructor
   */  
  ~LdataPathParseWriter(void);

  /**
   * Run the app (write out latest data info)
   * @return 0 for good, -1 for bad
   */
  int run(void);

  bool isOK; /**< True for well formed object */

protected:
  
private:

  std::string _progName;  /**< App name */
  Args _args;             /**< Command line argument handler */

  /**
   * Set DsLdataInfo object from _args object
   * @param[in,out] ldata  Object to set
   * @param[in] lt  Lead time seconds
   * @param[in] relPath  The relative path 
   */
  void _setFromArgs(DsLdataInfo &ldata, const int lt,
		    const std::string &relPath);
};

#endif

