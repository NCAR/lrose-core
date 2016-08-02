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
 * setup_x.c
 *
 * TimeHist routine - sets up the x paramteres
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
using namespace std;

void setup_x(void)

{

  int i;

  if (Glob->debug) {
    fprintf(stderr, "** setup_x **\n");
  }

  /*
   * search for display name on command line
   */

  Glob->display_name = NULL;

  for (i =  1; i < Glob->argc; i++) {
    if (!strcmp(Glob->argv[i], "-display") || !strcmp(Glob->argv[i], "-d")) {
      if (i < Glob->argc - 1) {
	Glob->display_name = (char *)
	  umalloc((ui32) (strlen(Glob->argv[i+1]) + 1));
	fprintf(stderr, "display_name addr = %ld\n",
                (long) Glob->display_name);
	strcpy(Glob->display_name, Glob->argv[i+1]);
	fprintf(stderr, "display_name = %s\n", Glob->display_name);
	sleep(1);
      }
    }
  } /* i */
	
  if((Glob->rdisplay = XOpenDisplay(Glob->display_name)) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_x\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot open display '%s'\n", Glob->display_name);
    tidy_and_exit(1);
  }

  Glob->rscreen = DefaultScreen(Glob->rdisplay);

  /*
   * register x error handler
   */

  XSetErrorHandler(xerror_handler);

}
