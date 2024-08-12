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
 * GRAPHIC_DUMP_IMAGE.CC - Write finished images stored in the visible
 *  Pixmaps to the disk and optionally run an image conversion command.
 * 
 */

#define GRAPHIC_DUMP_IMAGE

#include "cidd.h"

#include <png.h>

#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>

static void dump_image_xml(const char *dir, const char *fname);
static void dump_png(Drawable xid, Window w, const char *dir, const char *fname, const char *cmd, int confirm_flag, int page, int width, int height);

/*************************************************************************
 * GEN_IMAGE_FNAME
 *
 */

const char * gen_image_fname(const char *prefix,met_record_t *mr)
{
  static char nbuf[4096];
  char    tbuf[2048];
  char    zbuf[2048];
  struct tm res;
  time_t t;
  char *ptr;

  // initialize

  MEM_zero(nbuf);
  MEM_zero(tbuf);
  MEM_zero(zbuf);

  // For time series, use simplified name
    
  if(gd.series_save_active) {
    // Create simplified file name and bail out.
    snprintf(nbuf,4096,"%s%03d.%s",prefix,gd.movie.cur_frame,_params.image_ext);
    // replace any spaces in the file name with underscores - .
    while(_params.replace_underscores && (ptr = strchr(nbuf,' ')) != NULL) {
      *ptr = '_';
    }
    return nbuf;
  }

  // get separator

  const char *sep = _params.image_name_separator;

  // prefix - required

  strncat(nbuf,prefix,2048);

  // Optionally add a frame number 

  if(_params.add_frame_num_to_filename) {
    snprintf(tbuf,2048,"%03d", gd.movie.cur_frame);
    strncat(nbuf,tbuf,2048);
  }
	
  // legend name

  strncat(nbuf,sep,2048);
  strncat(nbuf,mr->legend_name,2048);

  // optionally add button name

  if(_params.add_button_name_to_filename) {
    strncat(nbuf,sep,2048);
    strncat(nbuf,mr->button_name,2048);
  }

  // zoom label

  snprintf(zbuf,2048,"cidd.level%d_label",gd.h_win.zoom_level+1);
  strncpy(zbuf,_params._zoom_levels[gd.h_win.zoom_level].label,1024);

  strncat(nbuf,sep,2048);
  strncat(nbuf,zbuf,2048);
    
  // add height if requested
  if(_params.add_height_to_filename) {
    snprintf(tbuf, 2048, "%s%g", sep, gd.h_win.cur_ht);
    strncat(nbuf,tbuf,2048);
  }

  // Optionally add a frame time
  if(_params.add_frame_time_to_filename) {
    switch (_params.movieframe_time_mode)  {
      default:
      case 0:
	strftime(tbuf,1024,_params.movieframe_time_format,
		 gmtime_r(&(gd.movie.frame[gd.movie.cur_frame].time_end),&res));
	break;
      case 1:
	strftime(tbuf,1024,_params.movieframe_time_format,
		 gmtime_r(&(gd.movie.frame[gd.movie.cur_frame].time_mid),&res));
	break;
      case 2:
	strftime(tbuf,1024,_params.movieframe_time_format,
		 gmtime_r(&(gd.movie.frame[gd.movie.cur_frame].time_start),&res));
	break;
    }
    strncat(nbuf,sep,2048);
    strncat(nbuf,tbuf,2048);
  }

  // Optionally add a generation time - (for model data)
  if(_params.add_gen_time_to_filename) {
    strncat(nbuf,sep,2048);
    t = (time_t)mr->h_mhdr.time_gen;
    strftime(tbuf,1024,_params.movieframe_time_format, gmtime_r(&t,&res));
    strncat(nbuf,tbuf,2048);

  }

  // Optionally add a data valid time
  if(_params.add_valid_time_to_filename) {
    strncat(nbuf,sep,2048);
    t = (time_t)mr->h_mhdr.time_centroid;
    strftime(tbuf,1024,_params.movieframe_time_format, gmtime_r(&t,&res));
    strncat(nbuf,tbuf,2048);
  }

  // Add the final extension
  strncat(nbuf, ".", 2047);
  strncat(nbuf,_params.image_ext,2047);

  // replace any spaces in the file name with underscores - .
  while(_params.replace_underscores && (ptr = strchr(nbuf,' ')) != NULL)  *ptr = '_';

  return nbuf;
}

/*************************************************************************
 * Dump a cidd pixmap to a Image file and optionally run a conversion program
 */
void dump_cidd_image(int win, int confirm_flag, int print_flag,int page)
{

  char cmd[MAX_PATH_LEN * 2];
  // char pathname[MAX_PATH_LEN * 4];
  const char *fname;
  char dir[MAX_PATH_LEN * 2];
  Window w = 0;
  Drawable xid;

  // day dir?

  STRncopy(dir, _params.image_dir, MAX_PATH_LEN);
  if (_params.save_images_to_day_subdir) {
    // start with day subdir
    char daydir[MAX_PATH_LEN];
    DateTime centroidTime((time_t)gd.mrec[page]->h_mhdr.time_centroid);
    snprintf(daydir,MAX_PATH_LEN,"/%.4d%.2d%.2d",
            centroidTime.getYear(),
            centroidTime.getMonth(),
            centroidTime.getDay());
    strncat(dir,daydir,MAX_PATH_LEN);
  }

  set_busy_state(1);

  switch(win) {
    default:
    case PLAN_VIEW:  /* The horizontal window */
      if(gd.generate_filename) {
        STRcopy(gd.h_win.image_fname,
                gen_image_fname(_params.image_horiz_prefix,gd.mrec[page]),
                MAX_PATH_LEN);
        if(strstr(gd.h_win.image_fname,_params.image_ext) == NULL) { 
          strncat(gd.h_win.image_fname,".",MAX_PATH_LEN-1);
          strncat(gd.h_win.image_fname,_params.image_ext,MAX_PATH_LEN-1);
        }

        gd.generate_filename = 0;
      }
      fname = gd.h_win.image_fname;
      if(print_flag) {
        if (_params.print_script == NULL){
          fprintf(stderr,"WARNING : cidd.print_script not set, using \"ls\"\n");
          snprintf(cmd,MAX_PATH_LEN * 2,"%s","ls");
        } else {
          snprintf(cmd,MAX_PATH_LEN * 2,"%s",_params.print_script);
        }
      } else {
        if(gd.series_save_active) {
          strcpy(cmd,""); // Don't run any command - in series save.
        } else {
          snprintf(cmd,MAX_PATH_LEN * 2,"%s",gd.h_win.image_command);
        }
      }
      xid = gd.h_win.can_xid[gd.h_win.cur_cache_im];
      // w = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

      if(_params.output_geo_xml) dump_image_xml(dir, fname);
      dump_png(xid,w,dir,fname,cmd,confirm_flag,page,gd.h_win.can_dim.width,gd.h_win.can_dim.height);

      {
        string path(dir);
        path += "/";
        path += fname;
        STRcopy(gd.movie.frame[gd.movie.cur_frame].fname,path.c_str(),NAME_LENGTH);
        // snprintf(pathname,MAX_PATH_LEN * 4,"%s/%s",dir,fname);
        // STRcopy(gd.movie.frame[gd.movie.cur_frame].fname,pathname,NAME_LENGTH);
      }

      break;

    case XSECT_VIEW:  /* The vertical cross section  window */
      if(gd.generate_filename) {
        strncpy(gd.v_win.image_fname,
                gen_image_fname(_params.image_vert_prefix,gd.mrec[page]), MAX_PATH_LEN - 2);
        if(strstr(gd.v_win.image_fname,_params.image_ext) == NULL) { 
          strncat(gd.v_win.image_fname,".",MAX_PATH_LEN-1);
          strncat(gd.v_win.image_fname,_params.image_ext,MAX_PATH_LEN-1);
        }

        gd.generate_filename = 0;
      }
      fname = gd.v_win.image_fname;
      if(print_flag) {
        snprintf(cmd,MAX_PATH_LEN * 2,"%s",_params.print_script);
      } else {
        if(gd.series_save_active) {
          strcpy(cmd,""); // Don't run any command - in series save.
        } else {
          snprintf(cmd,MAX_PATH_LEN * 2,"%s",gd.v_win.image_command);
        }
      }
      xid = gd.v_win.can_xid[gd.v_win.cur_cache_im];
      // w = xv_get(gd.v_win_v_win_pu->v_win_pu,XV_XID);
      {
        string path(dir);
        path += "/";
        path += fname;
        // snprintf(pathname,MAX_PATH_LEN * 2,"%s/%s",dir,fname);
        // STRcopy(gd.movie.frame[gd.movie.cur_frame].vfname,pathname,NAME_LENGTH - 1);
        STRcopy(gd.movie.frame[gd.movie.cur_frame].vfname,path.c_str(),NAME_LENGTH - 1);
      }

      dump_png(xid,w,dir,fname,cmd,confirm_flag,page,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
      break;

    case BOTH_VIEWS:  /* The Both windows */
      fname = gd.h_win.image_fname;
      if(print_flag) {
        snprintf(cmd,MAX_PATH_LEN * 2,"%s",_params.print_script);
      } else {
        if(gd.series_save_active) {
          strcpy(cmd,""); // Don't run any command - in series save.
        } else {
          snprintf(cmd,MAX_PATH_LEN * 2,"%s",gd.h_win.image_command);
        }
      }
      xid = gd.h_win.can_xid[gd.h_win.cur_cache_im];
      // w = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
      if(_params.output_geo_xml) dump_image_xml(dir, fname);
      dump_png(xid,w,dir,fname,cmd,confirm_flag,page,gd.h_win.can_dim.width,gd.h_win.can_dim.height);

      fname = gd.v_win.image_fname;
      if(print_flag) {
        snprintf(cmd,MAX_PATH_LEN * 2,"%s",_params.print_script);
      } else {
        if(gd.series_save_active) {
          strcpy(cmd,""); // Don't run any command - in series save.
        } else {
          snprintf(cmd,MAX_PATH_LEN * 2,"%s",gd.v_win.image_command);
        }
      }
      xid = gd.v_win.can_xid[gd.v_win.cur_cache_im];
      // w = xv_get(gd.v_win_v_win_pu->v_win_pu,XV_XID);
      dump_png(xid,w,dir,fname,cmd,confirm_flag,page,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
      break;
  }

  // if(confirm_flag)xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);

  set_busy_state(0);
}

//////////////////////////////////////////////////////////////////////////////
// DUMP_IMAGE_XML: Write image Georeferencing XML 
//
static void dump_image_xml(const char *dir, const char *fname)
{
  FILE *xfile = NULL;
  char pathname_xml[MAX_PATH_LEN*2];
  snprintf(pathname_xml,MAX_PATH_LEN*2,"%s/%s",dir,fname);

  // Make sure the output directory exists
  if (ta_makedir_recurse( dir )){
    fprintf(stderr,"ERROR : failed to create directory %s\n", dir);
    return;
  }

  // replace the image extension with .xml 
  char *ptr;
  if((ptr = rindex(pathname_xml,'.')) != NULL) strncpy(ptr,".xml",5);

  if((xfile = fopen(pathname_xml,"w+")) == NULL) {
    perror("CIDD: Couldn't open file for dumping image Georeferencing info- Aborted\n");
    return;
  }
  fputs("<GeoImage>\n",xfile);

  // Get the projection values for the edges of the display.  These values will be in
  // projection units.

  double east = gd.h_win.cmax_x;
  double west = gd.h_win.cmin_x;
  double north = gd.h_win.cmax_y;
  double south = gd.h_win.cmin_y;

  // dx and dy are the projection units per pixel

  double dx = (gd.h_win.cmax_x - gd.h_win.cmin_x) / gd.h_win.img_dim.width;
  double dy = (gd.h_win.cmax_y - gd.h_win.cmin_y) / gd.h_win.img_dim.height;

  fprintf(xfile," <px> %d </px>\n",gd.h_win.can_dim.width);
  fprintf(xfile," <py> %d </py>\n",gd.h_win.can_dim.height);
  fprintf(xfile," <pdx> %g </pdx>\n",dx);
  fprintf(xfile," <pdy> %g </pdy>\n",dy);

  // Account for the margins
  east += dx * gd.h_win.margin.right;
  west -= dx * gd.h_win.margin.left;
  north += dy * gd.h_win.margin.top;
  south -= dy * gd.h_win.margin.bot;
    
  if (_params.use_latlon_in_geo_xml)
  {
    double south_lat, north_lat;
    double east_lon, west_lon;
    
    gd.proj.xy2latlon(west, south,
		      south_lat, west_lon);
    gd.proj.xy2latlon(east, north,
		      north_lat, east_lon);
    
    fputs("  <LatLonBox>\n",xfile);
    fprintf(xfile,"    <north> %g  </north>\n",north_lat);
    fprintf(xfile,"    <south> %g  </south>\n",south_lat);
    fprintf(xfile,"    <east> %g  </east>\n",east_lon);
    fprintf(xfile,"    <west> %g  </west>\n",west_lon);
    fputs("  </LatLonBox>\n",xfile);
  }
  else
  {
    switch(gd.display_projection) {
    case Mdvx::PROJ_LATLON:
      fputs("  <LatLonBox>\n",xfile);
      fprintf(xfile,"    <north> %g  </north>\n",north);
      fprintf(xfile,"    <south> %g  </south>\n",south);
      fprintf(xfile,"    <east> %g  </east>\n",east);
      fprintf(xfile,"    <west> %g  </west>\n",west);
      fputs("  </LatLonBox>\n",xfile);
      break;

    default:
      fputs("  <ProjectionBox>\n",xfile);
      fprintf(xfile,"    <north> %g  </north>\n",north);
      fprintf(xfile,"    <south> %g  </south>\n",south);
      fprintf(xfile,"    <east> %g  </east>\n",east);
      fprintf(xfile,"    <west> %g  </west>\n",west);
      fputs("  </ProjectionBox>\n",xfile);
      break;
    }
  }
      
  fputs("</GeoImage>\n",xfile);
  if(fclose(xfile) !=0) {
    fprintf(stderr,"Problem closing %s\n",pathname_xml);
    perror("CIDD");
  }
} 

//////////////////////////////////////////////////////////////////////////////
// DUMP_PNG: Write the actual image as a png
//
static void dump_png(Drawable xid,
                     Window w,
                     const char *dir,
                     const char *fname,
                     const char *cmd,
                     int confirm_flag,
                     int page,
                     int width,
                     int height)
{

  if (gd.debug || gd.debug1) {
    cerr << "DEBUG - dump_png:" << endl;
    cerr << " dir: " << dir << endl;
    cerr << " fname: " << fname << endl;
    cerr << " cmd: " << cmd << endl;
    cerr << " page: " << page << endl;
    cerr << " width: " << width << endl;
    cerr << " height: " << height << endl;
  }

  // Make sure the directory exists.

  FILE *outfile = NULL;

  if (ta_makedir_recurse( dir )){
    fprintf(stderr,"ERROR : failed to create directory %s\n", dir);
    return;
  }

  char pathname[MAX_PATH_LEN];
  snprintf(pathname,MAX_PATH_LEN,"%s/%s",dir,fname);
  
  if(strlen(pathname) < 1) {
    fprintf(stderr,"WARNING - copy path not set\n");
    return;
  }

  // check we can write to the file
  // display dialog if not

  if( confirm_flag) {
    // if((outfile = open_check_write(pathname,gd.h_win_horiz_bw->horiz_bw)) == NULL) {
    //   perror("CIDD: Couldn't open file for dumping image - Aborted\n");
    //   set_busy_state(0);
    //   return;
    // }
  } else {
    if((outfile = fopen(pathname,"w+")) == NULL) {
      perror("CIDD: Couldn't open file for dumping image - Aborted\n");
      set_busy_state(0);
      return;
    }
  }
  fclose(outfile);
  unlink(pathname);

  // open the file
  
  if (gd.debug || gd.debug1 || gd.series_save_active) {
    fprintf(stderr, "Opening png image file: %s\n", pathname);
  }

  outfile = fopen(pathname, "wb");
  if (!outfile) {
    int errNum = errno;
    cerr << "ERROR - graphic_dump_image::dump_png" << endl;
    cerr << "  Cannot open file: " << pathname << endl;
    cerr << strerror(errNum) << endl;
    set_busy_state(0);
    return;
  }

  // create png structs

  png_structp png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  
  if (!png_ptr) {
    cerr << "ERROR - graphic_dump_image::dump_png" << endl;
    cerr << "  Cannot create png_struct" << endl;
    return;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    cerr << "ERROR - graphic_dump_image::dump_png" << endl;
    cerr << "  Cannot create png_info" << endl;
    png_destroy_write_struct(&png_ptr, NULL);
    return;
  }

  // set up error jump return

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(outfile);
    cerr << "ERROR - graphic_dump_image::dump_png" << endl;
    cerr << "  setjmp() failed" << endl;
    return;
  }

  // initialize IO to use write(), the default

  png_init_io(png_ptr, outfile);

  // set info

  int defaultDepth = XDefaultDepth(gd.dpy,0);
  Visual *defaultVisual = DefaultVisual(gd.dpy,0);

  ui32 red_mask = defaultVisual->red_mask;
  ui32 green_mask = defaultVisual->green_mask;
  ui32 blue_mask = defaultVisual->blue_mask;
  int bits_per_rbg = defaultVisual->bits_per_rgb;

  if (gd.debug || gd.debug1) {
    cerr << " defaultDepth: " << defaultDepth << endl;
    cerr << " red_mask: " << defaultVisual->red_mask << endl;
    cerr << " green_mask: " << defaultVisual->green_mask << endl;
    cerr << " blue_mask: " << defaultVisual->blue_mask << endl;
    cerr << " bits_per_rgb: " << defaultVisual->bits_per_rgb << endl;
  }

  // get an image of the drawable

  XImage *ximage = XGetImage(gd.dpy, xid, 0, 0, 
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
  ui32 bgPixval = gd.legends.background_color->pixval;
  int pixel_size = 4; // RGBA
  for (int iy = 0; iy < height; iy++) {
    row_pointers[iy] = (png_bytep) png_malloc(png_ptr, width * pixel_size);
    for (int ix = 0; ix < width; ix++) {
      ui32 pixel = XGetPixel(ximage, ix, iy);
      int red = (pixel & red_mask) >> (bits_per_rbg * 2);
      int green = (pixel & green_mask) >> bits_per_rbg;
      int blue = (pixel & blue_mask);
      int opaque = 255;
      ui32 rgba = ((opaque << bits_per_rbg * 3) | 
                   (blue << bits_per_rbg * 2) |
                   (green << bits_per_rbg) | 
                   red);
      if (_params.transparent_images && pixel == bgPixval) {
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

  if(gd.debug || gd.debug1 || gd.series_save_active ) {
    fprintf(stderr, "Wrote image file: %s\n", pathname);
  }
  
  DsLdataInfo LDI(_params.image_dir); // Set up a LdataInfo File

  string relPath;
  Path::stripDir(_params.image_dir, pathname, relPath);
  
  if(gd.debug) LDI.setDebug();
  LDI.setLatestTime((time_t)gd.mrec[page]->h_mhdr.time_centroid);
  LDI.setWriter(gd.app_name);
  LDI.setDataFileExt(_params.image_ext);
  LDI.setDataType("png");
  LDI.setUserInfo1("CIDD Output Image");
  LDI.setRelDataPath(relPath);
  
  if(LDI.write((time_t)gd.mrec[page]->h_mhdr.time_centroid)) {
    fprintf(stderr,"Problem Writing LdataInfo File in %s",dir);
    perror("CIDD");
  }
  
  if(strlen(cmd) > 3) {
    char cmdbuf[MAX_PATH_LEN*2];
    snprintf(cmdbuf,MAX_PATH_LEN*2,"%s %s",cmd,pathname);
    safe_system(cmdbuf,_params.simple_command_timeout_secs);
  }

}

