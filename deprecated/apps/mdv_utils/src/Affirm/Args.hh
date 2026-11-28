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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: prestop $
 *   $Locker:  $
 *   $Date: 2017/06/14 18:41:37 $
 *   $Id: Args.hh,v 1.4 2017/06/14 18:41:37 prestop Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Args.hh : header file for the Args class.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Args_HH
#define Args_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>

#include <tdrp/tdrp.h>
using namespace std;

/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class Args
{
 public:

  // Constructor

  Args(int argc, char **argv, char *prog_name);
  
  // Destructor

  ~Args(void);
  
  // TDRP overrides specified in the command line arguments.

  tdrp_override_t override;
  
  ////////////////////
  // Access methods //
  ////////////////////

  const time_t& getArchiveStartTime(void) const
  {
    return _archiveStartTime;
  }
  
  const time_t& getArchiveEndTime(void) const
  {
    return _archiveEndTime;
  }

	  // Print the usage for this program.

	void usage(char *prog_name,
	      FILE *stream);


 private:

  // Archive start and end times

  time_t _archiveStartTime;
  time_t _archiveEndTime;
  
  // Disallow the copy constructor and assignment operator

  Args(const Args&);
  const Args& operator=(const Args&);
  
  // Convert a time string entered on the command line to a UNIX
  // time value.
  //
  // Returns the converted time value if successful, 0 if unsuccessful.

  static time_t _convertTimeString(const char *time_string);
  
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("Args");
  }
  
};


#endif
