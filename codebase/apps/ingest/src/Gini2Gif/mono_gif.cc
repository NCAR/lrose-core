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

#include "gifsave.hh"

#include <math.h>
using namespace std;

/* Define a static var so that the GetPixel function
can operate. */
static unsigned char *pix;
static int Nx;

/* Define the GetPixel function. */
static int GetPixel(int x, int y)
{
    return pix[y*Nx+x];
}



/* 
Saves a monochrome GIF image from a byte buffer. 
This is built on some public domain functions.
Niles Oien, March 1999.
*/
void mono_gif(unsigned char *pixels, int nx, int ny,
	char *filename, int Negative, int Blue_tint)
{

  int i;

  const double e=2.7182818285;
  double red,green,blue;
  double t;

  /* Create the GIF file and initialise. */
  GIF_Create(filename,nx,ny, 256, 8);

  /* Set up a monochrome colormap. If Negative, go 
     white->black, else black->white. */

  for(i=0;i<256;i++){

    t=i/255.0; 
    if (Negative) t=1.0-t;

    if (Blue_tint){
      red=255.0*(pow(e,t) -1.0)/(e-1.0);
      green=255*t;
      blue=255.0*(1.0 - pow(e,-t))/(1.0 - 1.0/e);
    } else {
      red=255*t;
      green=255*t;
      blue=255*t;
    }

    //if (Negative) {
      GIF_SetColor(i,int(red),int(green),int(blue));
    //} else {
      //GIF_SetColor(i,255-int(red),255-int(green),255-int(blue));
    //}
  }

  /* Set up the GetPixel function. */
  pix=pixels; 
  Nx=nx;

  /* store the image, using the GetPixel function to extract pixel values */
  GIF_CompressImage(0, 0, -1, -1, GetPixel);

  /* finish it all and close the file */
  GIF_Close();

}

