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
 * convert_xwd_to_web.c
 *
 * conversion routine
 * 
 * Converts an XWD format file to a format that can be used on the WEB.
 * The actual format of the output file is determined by the web_convert_cmd
 * parameter and the given web_file_path parameter.
 *
 * Nancy Rehak, RAP, NCAR, April 1997
 *************************************************************************/

#include "Rview.hh"
using namespace std;

void convert_xwd_to_web(char *xwd_file_path,
			char *web_file_path)
{
  static char *web_convert_cmd;
  static int first_time = TRUE;
  
  char call_string[BUFSIZ];
  
  if (Glob->debug)
  {
    fprintf(stderr, "** convert XWD file to web format **\n");
  }

  /*
   * Get the convert command to use from the parameter file.
   */

  if (first_time)
  {
    web_convert_cmd = uGetParamString(Glob->prog_name,
				    "web_convert_cmd", WEB_CONVERT_CMD);
    
    first_time = FALSE;
  }
  
  /*
   * convert the file
   */

  sprintf(call_string, "%s %s %s",
	  web_convert_cmd, xwd_file_path, web_file_path);
  usystem_call(call_string);

  return;

}
