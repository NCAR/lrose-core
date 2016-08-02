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
 * set_xfonts.c: set xfonts
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "TimeHist.hh"
using namespace std;

void set_xfonts(void)

{

  char *x_text_font, *x_title_font, *x_scale_font;
  char *x_ticklabel_font, *x_header_font, *x_button_font;

  /*
   * get font names from resource file
   */
  
  x_text_font = uGetParamString(Glob->prog_name,
				"x_text_font", X_TEXT_FONT);
  x_title_font = uGetParamString(Glob->prog_name,
				 "x_title_font", X_TITLE_FONT);
  x_scale_font = uGetParamString(Glob->prog_name,
				 "x_scale_font", X_SCALE_FONT);
  x_ticklabel_font = uGetParamString(Glob->prog_name,
				     "x_ticklabel_font",
				     X_TICKLABEL_FONT);
  x_header_font = uGetParamString(Glob->prog_name,
				  "x_header_font", X_HEADER_FONT);
  x_button_font = uGetParamString(Glob->prog_name,
				  "x_button_font", X_BUTTON_FONT);
  
  Glob->x_text_font = xLoadFont(Glob->rdisplay, x_text_font);
  Glob->x_title_font = xLoadFont(Glob->rdisplay, x_title_font);
  Glob->x_scale_font = xLoadFont(Glob->rdisplay, x_scale_font);
  Glob->x_ticklabel_font = xLoadFont(Glob->rdisplay, x_ticklabel_font);
  Glob->x_header_font = xLoadFont(Glob->rdisplay, x_header_font);
  Glob->x_button_font = xLoadFont(Glob->rdisplay, x_button_font);

  if (Glob->debug == TRUE) {

    fprintf(stderr, "x_title_font ID = %p\n", Glob->x_text_font);
    fprintf(stderr, "x_title_font ID = %p\n", Glob->x_title_font);
    fprintf(stderr, "x_scale_font ID = %p\n", Glob->x_scale_font);
    fprintf(stderr, "x_ticklabel_font ID = %p\n", Glob->x_ticklabel_font);
    fprintf(stderr, "x_header_font ID = %p\n", Glob->x_header_font);
    fprintf(stderr, "x_button_font ID = %p\n", Glob->x_button_font);

  } /* if (Glob->debug == TRUE) */

}
