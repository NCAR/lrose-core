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
 *  @file Args.cc
 *
 *  @class Args
 *
 *  handles command line arguments
 *
 *  @author P. Prestopnik
 * 
 *  @date July 2014
 *
 *  @version $Id: Args.hh,v 1.2 2016/03/04 02:22:10 dixon Exp $
 *
 */



#ifndef ARGS_H
#define ARGS_H

// C++ include files
#include <string>
#include <vector>
#include <iostream>


// System/RAP include files
#include "tdrp/tdrp.h"

class Args {
  
public:
  /** 
  * creates new object, processes arguments
  *
  * @param[in] argc number of command line arguments
  * @param[in] argv pointer to arguments
  * @param[in] progName name of program
  *
  */
  Args( int argc, char **argv, const std::string& progName );

  /** destructor */
  virtual ~Args();

  ////////////////////
  // Access methods //
  ////////////////////

  time_t getArchiveStartTime() const
  {
    return _archiveStartTime;
  }
  
  time_t getArchiveEndTime() const
  {
    return _archiveEndTime;
  }

  tdrp_override_t getOverride() const
  {
    return override;
  }
  
protected:
  
private:

  /** TDRP overrides specified in the command line arguments. */
  tdrp_override_t override;

	/** Are Args ok? */
	bool isOK;

  /** start of run in archive mode */
  time_t _archiveStartTime;
  /** end of run in archive mode */
  time_t _archiveEndTime;

  static const std::string _className;

  /**  Disallow the copy constructor (singleton)*/
  Args( const Args & );
  /**  Disallow the assignment operator (singleton)*/
  Args &operator=( const Args & );

  /** 
   *  Convert a time string entered on the command line to a UNIX
   *  time value.
   *
   * @params[in] timeString time to convert 
   *
   *  @return the converted time value if successful, 0 if unsuccessful.
   */
  static time_t _convertTimeString(const char *timeString);

  void _setOverride( const std::string& );
  void _setOverride( const std::string&, const char* );

  /** 
   * Print the usage for this program.
   *
   * @params[in] progName name of program
   * @param[out] out output stream 
   *
   * @return None
   */
  void _usage( const std::string &progName,
	       std::ostream& out);
  
};

#endif
