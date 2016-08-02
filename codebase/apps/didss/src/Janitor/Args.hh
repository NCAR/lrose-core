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
/////////////////////////////////////////////////////////////
// Args.h: Command line object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
/////////////////////////////////////////////////////////////

/**
 * @class Args
 *
 * Command line arguments for the Janitor application.
 * 
 * \#include "Args.hh" <BR>
 *
 * @author Mike Dixon
 * @version $Id: Args.hh,v 1.9 2016/04/01 22:36:08 dixon Exp $
 * @see something
 */

#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <iostream>
#include <tdrp/tdrp.h>
using namespace std;

/**
 * @addtogroup Janitor
 */
/*@{*/

class Args {
  
public:

  // constructor

  Args();

  // Destructor

  ~Args();

  /**
   * Parse command line args
   *
   * @param[in] argc - number of command line arguments.
   * @param[in] argv - command line arguments.
   * @param[in] progName - program name.
   */

  int parse(int argc, char **argv, const string &progName);

  // public data
  
  tdrp_override_t override;	/**< flag for TDRP override.			*/
  char *topDir;			/**< top level directory of the data tree.	*/

protected:
  
private:

  /**
   * Print the usage statement.
   *
   * @param[in] prog_name - program name.
   * @param[in] out - file ptr to output to.
   */

  void _usage(const string &progName, ostream &out);
  
};

/*@}*/

#endif



