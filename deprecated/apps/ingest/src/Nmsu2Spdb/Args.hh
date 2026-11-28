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
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 01:23:04 $
 *   $Id: Args.hh,v 1.3 2016/03/07 01:23:04 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Args.hh : header file for the Args class.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2003
 *
 * Kay Levesque
 *
 ************************************************************************/

#ifndef Args_HH
#define Args_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
#include <string>
#include <vector>

#include <tdrp/tdrp.h>
using namespace std;


class Args
{
 public:

  // Constructor

  Args(int argc, char **argv, char *prog_name);
  
  // Destructor

  ~Args(void);
  
  // TDRP overrides specified in the command line arguments.

  tdrp_override_t override;
  
  // Determine the running mode based on the command line arguments

  bool isRealtime(void) const
  {
    return _fileList.empty();
  }
  

  // Return the next file in the file list.  Returns an empty string ("")
  // if the file list is exhausted.

  const string& nextFile(void)
  {
    if (_fileListStarted)
    {
      // Make sure the list wasn't already exhausted

      if (_fileListIter == _fileList.end())
      {
	_nextFile = "";
	return _nextFile;
      }
    
      // Increment the file list iterator

      ++_fileListIter;
    }
    else
    {
      _fileListIter = _fileList.begin();
      _fileListStarted = true;
    }
    
    // Return the next file name

    if (_fileListIter == _fileList.end())
      _nextFile = "";
    else
      _nextFile = *_fileListIter;
    
    return _nextFile;
  }
  
 private:

  // The program name for error messages

  string _progName;
  

  // The list of files to process if in archive mode

  vector< string > _fileList;
  vector< string >::iterator _fileListIter;
  bool _fileListStarted;
  
  string _nextFile;
  

  // Print the usage for this program.

  void _usage(FILE *stream);
  
};


#endif
