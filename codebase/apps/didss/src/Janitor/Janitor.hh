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
// Janitor.h
//
// Janitor object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

/**
 * @class Janitor
 *
 * Utility application for managing disk space by cleaning up data directories.
 *
 * @author Mike Dixon
 * @version $Id: Janitor.hh,v 1.7 2016/04/01 22:36:08 dixon Exp $
 * @see something
 */

#ifndef Janitor_H
#define Janitor_H

#include <string>
#include "Args.hh"
#include "Params.hh"
using namespace std;

/**
 * @defgroup Janitor Janitor disk/data management application
 * @ingroup 
 */
/*@{*/

class Janitor {
  
public:

  // constructor

  /**
   * Alternate constructor.
   *
   * @param[in] argc - number of command line arguments.
   * @param[in] argv - command line arguments.
   */
  Janitor (int argc, char **argv);

  // destructor
  
  /**
   * Destructor.
   */
  ~Janitor();

  // run 

  /**
   * Main method to run the Janitor application.
   *
   * @returns 0 on success; -1 otherwise.
   */
  int Run( void );

  // data members

  int OK;	/**< flag that command line arguments are okay.	*/

protected:
  
private:

  string _progName;	/**< program name.		*/
  char *_paramsPath;	/**< parameters file path.	*/
  Args _args;		/**< command line arguments.	*/
  Params _params;	/**< directory parameters.	*/

  /**
   * Traverse a directory tree performing janitorial functions.
   *
   * @param[in] top_dir - top of the directory tree.
   *
   * @returns 0 on success; -1 otherwise.
   */
  int _traverse(const string &top_dir);

};

/*@}*/

#endif
