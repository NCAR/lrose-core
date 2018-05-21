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
/************************************************************************
 * InputDirRecurse: InputDirRecurse object code.  This object monitors
 *                  an input directory for new input files or for updates
 *                  to input files.  It will recurse through subdirectories
 *                  also looking for input files.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef InputDirRecurse_H
#define InputDirRecurse_H

#include <cstdio>
#include <string>
#include <cassert>
#include <vector>

#include <sys/types.h>
#include <dirent.h>

#include <toolsa/InputDir.hh>

using namespace std;


class InputDirRecurse : public InputDir
{
public :
    
  // Constructors

  InputDirRecurse(const string &dir_name = "",
		  const string &file_substring = "",
		  const string &exclude_substring = "",
		  const bool process_old_files = false,
                  bool debug = false,
                  bool verbose = false);

  InputDirRecurse(const char *dir_name,
		  const char *file_substring,
		  const char *exclude_substring,
		  int process_old_files,  // Set to 1 to force scan of old files
                  bool debug = false,
                  bool verbose = false);

  // Destructor

  virtual ~InputDirRecurse(void);
  
  // Get the next input filename.  Call this in a tight loop until
  // NULL is returned (indicating no more new files).

  char *getNextFilename(int check_dir_flag, // Set flag to 1 to check for new/updated files
			int max_input_file_age = -1);
  
protected :

  vector< char* > _fileList;
  

  /*********************************************************************
   * _generateFileList() - Recurse through the given directory, adding
   *                       files to the file list as appropriate.
   */

  void _generateFileList(const string &dir_name, DIR *dir_ptr,
			 const int max_input_data_age);
  

};


#endif
