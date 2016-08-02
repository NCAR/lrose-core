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
 *   $Date: 2016/03/04 02:22:13 $
 *   $Revision: 1.4 $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Args.hh : header file for the Args class.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Sept 2006
 *
 ************************************************************************/

#ifndef Args_HH
#define Args_HH

#include <stdio.h>
#include <string>
#include <vector>
#include <time.h>

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

  // Optional secondary tdrp file
  char *additional_tdrp_file;

  // Start_time for TIME_LIST mode
  DateTime start_time;

  // End_time for TIME_LIST mode
  DateTime end_time;

  ////////////////////
  // Access methods //
  ////////////////////

  const vector<string>& getInputFileList(void) const
  {
    return _inputFileList;
  }

 private:

  // The program name for error messages
  string _progName;

  // Vector of input files for FILE_LIST mode
  vector<string> _inputFileList;

  // Print the usage for this program.
  void _usage(FILE *stream);
  
};


#endif
