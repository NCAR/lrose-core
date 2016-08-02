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
 * setup_x.c: sets up the x paramsironment
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "Rview.hh"
using namespace std;

void setup_x()

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
	  umalloc((ui32)strlen(Glob->argv[i+1]) + 1);
	strcpy(Glob->display_name, Glob->argv[i+1]);
      }
    }
  } /* i */
	
  if((Glob->rdisplay = XOpenDisplay(Glob->display_name)) == NULL) {
    fprintf(stderr, "ERROR - %s:setup_x\n", Glob->prog_name);
    fprintf(stderr,
	    "Cannot open display '%s' or '%s'\n",
	    Glob->display_name, getenv("DISPLAY"));
    tidy_and_exit(1);
  }

  Glob->rscreen = DefaultScreen(Glob->rdisplay);

  /*
   * colormap
   */

  Glob->cmap = DefaultColormap(Glob->rdisplay, Glob->rscreen);
  /*  Glob->cmap = XCreateColormap(Glob->rdisplay,
			       RootWindow(Glob->rdisplay,Glob->rscreen),
			       DefaultVisual(Glob->rdisplay,Glob->rscreen),
			       AllocAll); */

  /*
   * register x error handler
   */

  XSetErrorHandler(xerror_handler);

}
