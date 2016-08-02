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

#if defined(__HAVE_IMLIB)
#include <Imlib2.h>
#endif

#include <toolsa/DateTime.hh>
#include <didss/LdataInfo.hh>


#if defined(__HAVE_IMLIB)
void dump_cidd_xid(Drawable xid, Window w, char *dir,
	char *fname, char *cmd, int confirm_flag, int page, int width, int height);
void dump_image_xml(char *dir, char *fname);
#else
static void dump_cidd_xid(Drawable xid, Window w, char *dir, char *fname, char *cmd, int confirm_flag);
#endif

/*************************************************************************
 * GEN_IMAGE_FNAME
 *
 */

char * gen_image_fname(char *prefix,met_record_t *mr)
{
    static char nbuf[2048];
    char    tbuf[1024];
    char    zbuf[256];
    struct tm res;
    time_t t;
    char *ptr;

    // build output name
    
	if(gd.series_save_active) { // Create simplified file name and bail out.
      sprintf(nbuf,"%s%03d.%s",prefix,gd.movie.cur_frame,gd.image_ext);
      // replace any spaces in the file name with underscores - .
      while(gd.replace_underscores && (ptr = strchr(nbuf,' ')) != NULL)  *ptr = '_';

      return nbuf;
	}

	// Always start with the Prefix.
    sprintf(nbuf,"%s", prefix);

	// Optionally add a frame number 
	if(gd.add_frame_num_to_filename) {
        sprintf(tbuf,"%03d", gd.movie.cur_frame);
        strncat(nbuf,tbuf,2048);
	}
	
	sprintf(zbuf,"cidd.level%d_label",gd.h_win.zoom_level+1);
    strncpy(zbuf,gd.uparams->getString(zbuf ,"Z"),256);

    sprintf(tbuf,"_%s_%s", mr->legend_name, zbuf);
    strncat(nbuf,tbuf,2048);
    
    // add height if requested
    if(gd.add_height_to_filename) {
      sprintf(tbuf, "_%g", gd.h_win.cur_ht);
      strncat(nbuf,tbuf,2048);
    }

    // Optionally add a frame time
    if(gd.add_frame_time_to_filename) {
      switch (gd.movieframe_time_mode)  {
      default:
      case 0:
	strftime(tbuf,1024,gd.movieframe_time_format,
		 gmtime_r(&(gd.movie.frame[gd.movie.cur_frame].time_end),&res));
	break;
      case 1:
	strftime(tbuf,1024,gd.movieframe_time_format,
		 gmtime_r(&(gd.movie.frame[gd.movie.cur_frame].time_mid),&res));
	break;
      case 2:
	strftime(tbuf,1024,gd.movieframe_time_format,
		 gmtime_r(&(gd.movie.frame[gd.movie.cur_frame].time_start),&res));
	break;
      }
      strncat(nbuf,"_",2048);
      strncat(nbuf,tbuf,2048);
    }

    // Optionally add a generation time - (for model data)
    if(gd.add_gen_time_to_filename) {
      strncat(nbuf,"_",2048);
      t = (time_t)mr->h_mhdr.time_gen;
      strftime(tbuf,1024,gd.movieframe_time_format, gmtime_r(&t,&res));
      strncat(nbuf,tbuf,2048);

    }

    // Optionally add a data valid time
    if(gd.add_valid_time_to_filename) {
      strncat(nbuf,"_",2048);
      t = (time_t)mr->h_mhdr.time_centroid;
      strftime(tbuf,1024,gd.movieframe_time_format, gmtime_r(&t,&res));
      strncat(nbuf,tbuf,2048);
    }

    // Add the final extension
    strncat(nbuf,".",2048);
    strncat(nbuf,gd.image_ext,2048);

    // replace any spaces in the file name with underscores - .
    while(gd.replace_underscores && (ptr = strchr(nbuf,' ')) != NULL)  *ptr = '_';

    return nbuf;
}

#if defined(__HAVE_IMLIB)

/*************************************************************************
 * Dump a cidd pixmap to a Image file and optionally run a conversion program
 */
void dump_cidd_image(int win, int confirm_flag, int print_flag,int page)
{
 
    char cmd[MAX_PATH_LEN * 2];
    char pathname[MAX_PATH_LEN];
    char pathname_xml[MAX_PATH_LEN];
    char *fname;
    char *dir;
    Window w;
    Drawable xid;

    set_busy_state(1);

    switch(win) {
        default:
        case PLAN_VIEW:  /* The horizontal window */
    	  if(gd.generate_filename) {
		strncpy(gd.h_win.image_fname,
			gen_image_fname(gd.image_horiz_prefix,gd.mrec[page]),
			MAX_PATH_LEN);
		if(strstr(gd.h_win.image_fname,gd.image_ext) == NULL) { 
	    	  strncat(gd.h_win.image_fname,".",MAX_PATH_LEN);
	    	  strncat(gd.h_win.image_fname,gd.image_ext,MAX_PATH_LEN);
		}

		gd.generate_filename = 0;
    	  }
         fname = gd.h_win.image_fname;
	 if(print_flag) {
             sprintf(cmd,"%s",gd.print_script);
	 } else {
			 if(gd.series_save_active) {
                 strcpy(cmd,""); // Don't run any command - in series save.
			 } else {
                 sprintf(cmd,"%s",gd.h_win.image_command);
			 }
	 }
         dir = gd.image_dir;
         xid = gd.h_win.can_xid[gd.h_win.cur_cache_im];
         w = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

	 dump_cidd_xid(xid,w,dir,fname,cmd,confirm_flag,page,gd.h_win.can_dim.width,gd.h_win.can_dim.height);
     if(gd.output_geo_xml) dump_image_xml(dir, fname);

	 sprintf(pathname,"%s/%s",dir,fname);
         strncpy(gd.movie.frame[gd.movie.cur_frame].fname,pathname,NAME_LENGTH);


        break;

        case XSECT_VIEW:  /* The vertical cross section  window */
    	  if(gd.generate_filename) {
		strncpy(gd.v_win.image_fname,
			gen_image_fname(gd.image_vert_prefix,gd.mrec[page]), MAX_PATH_LEN);
		if(strstr(gd.v_win.image_fname,gd.image_ext) == NULL) { 
	    	  strncat(gd.v_win.image_fname,".",MAX_PATH_LEN);
	    	  strncat(gd.v_win.image_fname,gd.image_ext,MAX_PATH_LEN);
		}

		gd.generate_filename = 0;
    	  }
         fname = gd.v_win.image_fname;
	 if(print_flag) {
             sprintf(cmd,"%s",gd.print_script);
	 } else {
			 if(gd.series_save_active) {
                 strcpy(cmd,""); // Don't run any command - in series save.
			 } else {
                 sprintf(cmd,"%s",gd.v_win.image_command);
			 }
	 }
         dir = gd.image_dir;
         xid = gd.v_win.can_xid[gd.v_win.cur_cache_im];
         w = xv_get(gd.v_win_v_win_pu->v_win_pu,XV_XID);
	 sprintf(pathname,"%s/%s",dir,fname);
         strncpy(gd.movie.frame[gd.movie.cur_frame].vfname,pathname,NAME_LENGTH);


	 dump_cidd_xid(xid,w,dir,fname,cmd,confirm_flag,page,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
        break;

        case BOTH_VIEWS:  /* The Both windows */
         fname = gd.h_win.image_fname;
	 if(print_flag) {
             sprintf(cmd,"%s",gd.print_script);
	 } else {
			 if(gd.series_save_active) {
                 strcpy(cmd,""); // Don't run any command - in series save.
			 } else {
                 sprintf(cmd,"%s",gd.h_win.image_command);
			 }
	 }
         dir = gd.image_dir;
         xid = gd.h_win.can_xid[gd.h_win.cur_cache_im];
         w = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
	 dump_cidd_xid(xid,w,dir,fname,cmd,confirm_flag,page,gd.h_win.can_dim.width,gd.h_win.can_dim.height);
     if(gd.output_geo_xml) dump_image_xml(dir, fname);

         fname = gd.v_win.image_fname;
	 if(print_flag) {
             sprintf(cmd,"%s",gd.print_script);
	 } else {
			 if(gd.series_save_active) {
                 strcpy(cmd,""); // Don't run any command - in series save.
			 } else {
                 sprintf(cmd,"%s",gd.v_win.image_command);
			 }
	 }
         dir = gd.image_dir;
         xid = gd.v_win.can_xid[gd.v_win.cur_cache_im];
         w = xv_get(gd.v_win_v_win_pu->v_win_pu,XV_XID);
	 dump_cidd_xid(xid,w,dir,fname,cmd,confirm_flag,page,gd.v_win.can_dim.width,gd.v_win.can_dim.height);
        break;
    }

    if(confirm_flag)xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);

    set_busy_state(0);
}

//////////////////////////////////////////////////////////////////////////////
// DUMP_IMAGE_XML: Write image Georeferencing XML 
//
void dump_image_xml(char *dir, char *fname)
{
    FILE *xfile = NULL;
    char pathname_xml[MAX_PATH_LEN];
    sprintf(pathname_xml,"%s/%s",dir,fname);

    // replace the image extension with .xml 
    char *ptr;
    if((ptr = rindex(pathname_xml,'.')) != NULL) strncpy(ptr,".xml",4);

    if((xfile = fopen(pathname_xml,"w+")) == NULL) {
	perror("CIDD: Couldn't open file for dumping image Georeferencing info- Aborted\n");
	return;
    }
    fputs("<GeoImage>\n",xfile);

    double east = gd.h_win.cmax_x;
    double west = gd.h_win.cmin_x;
    double north = gd.h_win.cmax_y;
    double south = gd.h_win.cmin_y;
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
    north -= dy * gd.h_win.margin.bot;
    
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
    fputs("</GeoImage>\n",xfile);
    if(fclose(xfile) !=0) {
           fprintf(stderr,"Problem closing %s\n",pathname_xml);
           perror("CIDD");
    }
} 

//////////////////////////////////////////////////////////////////////////////
// DUMP_CIDD_XID: Write the actual image - Output an Ldatainfo file too.
//
void dump_cidd_xid(Drawable xid, Window w, char *dir, char *fname, char *cmd, int confirm_flag, int page, int width, int height)
{
  //
  // Make sure the directory exists.
  //
  if (ta_makedir_recurse( dir )){
    fprintf(stderr,"ERROR : failed to create directory %s\n", dir);
    return;
  }
    Imlib_Image image;

    FILE *outfile = NULL;
	char pathname[MAX_PATH_LEN];
	char cmdbuf[MAX_PATH_LEN*2];

	sprintf(pathname,"%s/%s",dir,fname);

    if(strlen(pathname) < 1) {
      fprintf(stderr,"WARNING - copy path not set\n");
      return;
    }

    if( confirm_flag) {
      if((outfile = open_check_write(pathname,gd.h_win_horiz_bw->horiz_bw)) == NULL) {
	perror("CIDD: Couldn't open file for dumping image - Aborted\n");
	set_busy_state(0);
	return;
      }
    } else {
      if((outfile = fopen(pathname,"w+")) == NULL) {
	perror("CIDD: Couldn't open file for dumping image - Aborted\n");
	set_busy_state(0);
	return;
      }
    }
    if(fclose(outfile) !=0) {
           fprintf(stderr,"Problem closing %s\n",pathname);
           perror("CIDD");
    }
    if(unlink(pathname) !=0) {
           fprintf(stderr,"Problem Unlinking %s\n",pathname);
           perror("CIDD");
    }
 
	
	imlib_context_set_display(gd.dpy);
    imlib_context_set_visual(DefaultVisual(gd.dpy,0));
    imlib_context_set_colormap(gd.cmap);
    imlib_context_set_drawable(xid);

	image =  imlib_create_image_from_drawable (0,
			0, 0, width, height, '1');

	imlib_context_set_image(image);


    if(gd.debug || gd.debug1 || gd.series_save_active ) fprintf(stderr,"Saving: %s\n",pathname);
    
    imlib_save_image (pathname);

    imlib_free_image();

    LdataInfo LDI(dir); // Set up a LdataInfo File

    if(gd.debug) LDI.setDebug();
    LDI.setLatestTime((time_t)gd.mrec[page]->h_mhdr.time_centroid);
    LDI.setWriter(gd.app_name);
    LDI.setDataFileExt(gd.image_ext);
    LDI.setUserInfo1("CIDD Output Image");
    LDI.setRelDataPath(fname);

    if(LDI.write((time_t)gd.mrec[page]->h_mhdr.time_centroid)) {
      fprintf(stderr,"Problem Writing LdataInfo File in %s",dir);
      perror("CIDD");
    }

    if(LDI.write((time_t)gd.mrec[page]->h_mhdr.time_centroid)) {
      fprintf(stderr,"Problem Writing LdataInfo File in %s",dir);
      perror("CIDD");
    }

    if(strlen(cmd) > 3) {
	      sprintf(cmdbuf,"%s %s",cmd,pathname);
          safe_system(cmdbuf,gd.simple_command_timeout_secs);
    }
}

#else

/*************************************************************************
 * Dump a cidd pixmap to a XWD file and optionally run a conversion program
 * 
 * Non-imlib version
 */

void dump_cidd_image(int win, int confirm_flag, int print_flag, int page)
 
{
    char cmd[MAX_PATH_LEN * 2];
    char *fname;
    char *dir;
    Window w;
    Drawable xid;
 
    set_busy_state(1);

    if(gd.generate_filename) {
	strncpy(gd.h_win.image_fname,
		gen_image_fname("cidd",gd.mrec[gd.h_win.page]),
		MAX_PATH_LEN);
	if(strstr(gd.h_win.image_fname,".xwd") == NULL) { 
	    strncat(gd.h_win.image_fname,".xwd",MAX_PATH_LEN);
	}

	gd.generate_filename = 0;
    }

    switch(win) {
        default:
        case PLAN_VIEW:  /* The horizontal window */
         fname = gd.h_win.image_fname;
	 if(print_flag) {
             sprintf(cmd,"%s %s",gd.print_script,fname);
	 } else {
             sprintf(cmd,"%s %s",gd.h_win.image_command,fname);
	 }
         dir = gd.h_win.image_dir;
         xid = gd.h_win.can_xid[gd.h_win.cur_cache_im];
         w = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
	 dump_cidd_xid(xid,w,dir,fname,cmd,confirm_flag);
        break;

        case XSECT_VIEW:  /* The vertical cross section  window */
         fname = gd.v_win.image_fname;
	 if(print_flag) {
             sprintf(cmd,"%s %s",gd.print_script,fname);
	 } else {
             sprintf(cmd,"%s %s",gd.h_win.image_command,fname);
	 }
         dir = gd.v_win.image_dir;
         xid = gd.v_win.can_xid[gd.v_win.cur_cache_im];
         w = xv_get(gd.v_win_v_win_pu->v_win_pu,XV_XID);
	 dump_cidd_xid(xid,w,dir,fname,cmd,confirm_flag);
        break;

        case BOTH_VIEWS:  /* The Both windows */
         fname = gd.h_win.image_fname;
	 if(print_flag) {
             sprintf(cmd,"%s %s",gd.print_script,fname);
	 } else {
             sprintf(cmd,"%s %s",gd.h_win.image_command,fname);
	 }
         dir = gd.h_win.image_dir;
         xid = gd.h_win.can_xid[gd.h_win.cur_cache_im];
         w = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);
	 dump_cidd_xid(xid,w,dir,fname,cmd,confirm_flag);

         fname = gd.v_win.image_fname;
	 if(print_flag) {
             sprintf(cmd,"%s %s",gd.print_script,fname);
	 } else {
             sprintf(cmd,"%s %s",gd.h_win.image_command,fname);
	 }
         dir = gd.v_win.image_dir;
         xid = gd.v_win.can_xid[gd.v_win.cur_cache_im];
         w = xv_get(gd.v_win_v_win_pu->v_win_pu,XV_XID);
	 dump_cidd_xid(xid,w,dir,fname,cmd,confirm_flag);
        break;
    }

    if(confirm_flag)xv_set(gd.save_pu->save_im_pu,FRAME_CMD_PUSHPIN_IN, FALSE,XV_SHOW, FALSE,NULL);

    set_busy_state(0);
}

//////////////////////////////////////////////////////////////////////////////
// DUMP_CIDD_XID
//
// Non-Imlib version
//

static void dump_cidd_xid(Drawable xid, Window w, char *dir, char *fname, char *cmd, int confirm_flag)
{
    XWindowAttributes win_att;
    FILE *outfile = NULL;

    if(XGetWindowAttributes(gd.dpy,w,&win_att) == 0) {
        fprintf(stderr,"Problem getting window attributes\n");
        return;
    }

    /* Change into the proper directory - Popup a warning notice panel on errors */
    if( chdir_check(dir,gd.h_win_horiz_bw->horiz_bw) < 0) {
        set_busy_state(0);
        return;
    }

    if(strlen(fname) < 1) {
      fprintf(stderr,"WARNING - copy path not set\n");
      return;
    }

    if( confirm_flag) {
      if((outfile = open_check_write(fname,gd.h_win_horiz_bw->horiz_bw)) == NULL) {
	perror("CIDD: Couldn't open file for dumping image - Aborted\n");
	set_busy_state(0);
	return;
      }
    } else {
      if((outfile = fopen(fname,"w+")) == NULL) {
	perror("CIDD: Couldn't open file for dumping image - Aborted\n");
	set_busy_state(0);
	return;
      }
    }
 
    XwuDumpWindow(gd.dpy,xid,&win_att,outfile);

    if(fclose(outfile) !=0) {
           fprintf(stderr,"Problem closing %s\n",fname);
           perror("CIDD");
    }
    
    if(strlen(cmd) > 3) {
          safe_system(cmd,gd.simple_command_timeout_secs);
    }
}

#endif
