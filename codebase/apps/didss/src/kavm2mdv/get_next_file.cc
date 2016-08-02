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
/*********************************************************************
 * get_next_file.cc
 *
 * Gets the next available file - returns file name.
 *
 * Blocks until file available
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 * Taken from get_next_file.c by Mike Dixon.
 *
 *********************************************************************/

#include <cstdio>

#include <toolsa/InputDir.hh>
#include <toolsa/str.h>

#include "kavm2mdv.h"
using namespace std;

char *get_next_file(void)
{
  static InputDir *input_dir = new InputDir(Glob->params.input_dir,
					    Glob->params.input_file_ext,
					    FALSE);

  PMU_auto_register("Checking for next file");
  
  char *next_file = input_dir->getNextFilename(TRUE);
  static char return_file[MAX_PATH_LEN];
  
  while(next_file == (char *)NULL)
  {
    PMU_auto_register("Waiting for next file");
    sleep(1);
    next_file = input_dir->getNextFilename(TRUE);
  } /* endwhile */

  STRcopy(return_file, next_file, MAX_PATH_LEN);
  delete [] next_file;
  
  return(return_file);
}
