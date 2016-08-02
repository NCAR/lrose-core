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
 * print_copy.c
 *
 * printing routine
 * 
 * sends copy file to printer
 *
 * Mike Dixon, RAP, NCAR, June 1990
 *************************************************************************/

#include "TimeHist.hh"
using namespace std;

void print_copy(char *ps_file_path)

{

  char *ps_printer, *print_command;
  char call_string[BUFSIZ];
  
  if (Glob->debug) {
    fprintf(stderr, "** print_copy **\n");
  }

  /*
   * get printer name
   */
  
  ps_printer = uGetParamString(Glob->prog_name,
			       "ps_printer", PS_PRINTER);
  
  /*
   * get print command
   */
  
  print_command = uGetParamString(Glob->prog_name,
				  "print_command", PRINT_COMMAND);
  
  /*
   * print file
   */

  sprintf(call_string, "%s%s %s", print_command, ps_printer, ps_file_path);
  usystem_call(call_string);

  return;

}
