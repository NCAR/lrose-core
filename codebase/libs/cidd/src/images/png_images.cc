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
 * png_images.cc
 *
 * Routines to handle dumping of png images.
 * 
 */

#include <sys/types.h>
#include <cerrno>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <cidd/png_images.hh>
#include <dsserver/DsLdataInfo.hh>

//////////////////////////////////////////////////////////////////////////////
// dump a png image to a given directory, specifying the file name
//
// Optionally write a latest_data_info file.
//
// Optionally transparent -
//     if so, the background color pixval must be specified.
//
// returns 0 on success, -1 on failure.

int png_image_dump(const char *appName,    // caller
                   Display *dpy,           // X display
                   Drawable xid,           // X ID of drawable
                   int width,              // width of drawable
                   int height,             // height of drawable
                   const char *outputDir,
                   const char *fileName,
                   time_t dataTime,
                   bool writeLdataInfo /* = true */,
                   bool transparent /* = false */,
                   int backgroundPixval /* = 0 */,
                   bool debug /* = false */)

{
  
  // Make sure the directory exists.
  
  if (!ta_stat_is_dir(outputDir)) {
    if (ta_makedir_recurse(outputDir)) {
      cerr << "ERROR - png_dump_image" << endl;
      cerr << "  Cannot create dir: " << outputDir << endl;
      return -1;
    }
  }

  // compute the path name
  
  char pathName[MAX_PATH_LEN];
  sprintf(pathName,"%s%s%s", outputDir, PATH_DELIM, fileName);
  
  // open the file
  
  if (debug) {
    cerr << "Opening png image file: " << pathName << endl;
  }

  FILE *outfile = fopen(pathName, "wb");
  if (!outfile) {
    int errNum = errno;
    cerr << "ERROR - png_dump_image" << endl;
    cerr << "  Cannot open file: " << pathName << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // create png structs

  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  
  if (!png_ptr) {
    cerr << "ERROR - png_dump_image" << endl;
    cerr << "  Cannot create png_struct" << endl;
    return -1;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    cerr << "ERROR - png_dump_image" << endl;
    cerr << "  Cannot create png_info" << endl;
    png_destroy_write_struct(&png_ptr, NULL);
    return -1;
  }
  
  // set up error jump return
  
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(outfile);
    return -1;
  }

  // initialize IO to use write(), the default

  png_init_io(png_ptr, outfile);

  // set info

  int defaultDepth = XDefaultDepth(dpy, 0);
  Visual *defaultVisual = DefaultVisual(dpy, 0);

  u_int32_t red_mask = defaultVisual->red_mask;
  u_int32_t green_mask = defaultVisual->green_mask;
  u_int32_t blue_mask = defaultVisual->blue_mask;
  int bits_per_rbg = defaultVisual->bits_per_rgb;
  
  if (debug) {
    cerr << "INFO - png_dump_image" << endl;
    cerr << " defaultDepth: " << defaultDepth << endl;
    cerr << " red_mask: " << defaultVisual->red_mask << endl;
    cerr << " green_mask: " << defaultVisual->green_mask << endl;
    cerr << " blue_mask: " << defaultVisual->blue_mask << endl;
    cerr << " bits_per_rgb: " << defaultVisual->bits_per_rgb << endl;
  }

  // get an image of the drawable

  XImage *ximage = XGetImage(dpy, xid, 0, 0, 
                             width, height, 0xffffffff, XYPixmap);

  int bit_depth = 8;
  int color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  png_set_IHDR(png_ptr, info_ptr, width, height,
               bit_depth,
               color_type,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  // set time

  png_time_struct ptime;
  png_convert_from_time_t(&ptime, time(NULL));
  png_set_tIME(png_ptr, info_ptr, &ptime);

  // set data by row

  png_bytepp row_pointers = 
    (png_bytepp) png_malloc(png_ptr,
                            height * sizeof(png_bytep));
  for (int iy = 0; iy < height; iy++) {
    row_pointers[iy]=NULL; /* security precaution */
  }
  int pixel_size = 4; // RGBA
  for (int iy = 0; iy < height; iy++) {
    row_pointers[iy] = (png_bytep) png_malloc(png_ptr, width * pixel_size);
    for (int ix = 0; ix < width; ix++) {
      u_int32_t pixel = XGetPixel(ximage, ix, iy);
      int red = (pixel & red_mask) >> (bits_per_rbg * 2);
      int green = (pixel & green_mask) >> bits_per_rbg;
      int blue = (pixel & blue_mask);
      int opaque = 255;
      u_int32_t rgba = ((opaque << bits_per_rbg * 3) | 
                        (blue << bits_per_rbg * 2) |
                        (green << bits_per_rbg) | 
                        red);
      if (transparent && pixel == (u_int32_t) backgroundPixval) {
        rgba = 0;
      }
      memcpy(row_pointers[iy] + ix * pixel_size, &rgba, pixel_size);
    }
    // memset(row_pointers[iy], 0, width * pixel_size);
  }

  // free up X image

  XDestroyImage(ximage);

  // set the rows in the PNG object

  png_set_rows(png_ptr, info_ptr, row_pointers);

  // write file

  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  
  // free up PNG data

  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(outfile);

  if(debug) {
    cerr << "INFO - png_dump_image" << endl;
    cerr << "  Wrote image file: " << pathName << endl;
  }

  // Set up a LdataInfo File  

  if (writeLdataInfo) {
    
    DsLdataInfo ldata(outputDir);
    if(debug) ldata.setDebug();
    ldata.setLatestTime(dataTime);
    ldata.setWriter(appName);
    ldata.setDataFileExt("png");
    ldata.setRelDataPath(fileName);
    
    if(ldata.write(dataTime)) {
      cerr << "ERROR - png_dump_image" << endl;
      cerr << " writing latest_data_info file, dir: " << outputDir << endl;
      return -1;
    }
    
  }

  return 0;
    
}

