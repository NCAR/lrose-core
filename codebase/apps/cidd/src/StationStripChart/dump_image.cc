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
/*************************************************************************
 * DUMP_IMAGE.CC - Write finished images stored int the visible
 *  Pixmaps to the disk and optionally run an image conversion command.
 * 
 */

#define DUMP_IMAGE

#include "strip_chart.h"

#include <cidd/png_images.hh>

//////////////////////////////////////////////////////////////////////////////
// DUMP_IMAGE: Output a standard format image - Format based on file name extension
//
void dump_image(Display *dpy, Drawable xid, int width, int height, char *outputDir, char *fname, time_t fileTime, bool writeLdataInfo)
{

	//	DEBUG
	//	return;
    if(strlen(fname) < 1) {
      fprintf(stderr,"WARNING - fname not set\n");
      return;
    }

    png_image_dump(
         "StationStripChart",
         dpy,
         xid,
         width,
         height,
         outputDir,
         fname,
         fileTime,
         writeLdataInfo);

}
