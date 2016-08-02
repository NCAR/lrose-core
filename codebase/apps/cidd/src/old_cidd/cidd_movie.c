/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*************************************************************************
 * CIDD_MOVIE.C: Routines used in Movie loops
 *
 *
 * For the Cartesian Radar Display (CIDD)
 * Frank Hage   July 1991 NCAR, Research Applications Program
 */

#define RD_MOVIE 1
#include "cidd.h"

#include "toolsa/utim.h"


/*************************************************************************
 * RENDER_H_MOVIE_FRAME: Render a horizontal display view
 */

render_h_movie_frame(index,xid)
    int    index;
    Drawable xid;
{
    int    c_field;
    int    frame;
    int stat;

    UTIMstruct    temp_utime;
    

    c_field = gd.h_win.field;
    if(gd.debug2)
        fprintf(stderr,
                "Rendering Horizontal movie_frame %d - field: %d, to XId: %d\n",
                index, c_field,xid);

    switch(gd.movie.mode) {
        case MOVIE_MR :
        case MOVIE_TS :
            stat = gather_hwin_data(c_field,
                                    gd.movie.frame[index].time_start.unix_time,
                                    gd.movie.frame[index].time_end.unix_time);
            if(stat == SUCCESS)  {
                render_horiz_display(xid,c_field,
                                     gd.movie.frame[index].time_start.unix_time,
                                     gd.movie.frame[index].time_end.unix_time);
            } else {
                return stat;
            }
        break;
         
        case MOVIE_EL :
            if(gd.movie.cur_frame >= 0) {
                frame = gd.movie.cur_frame;
            } else {
                frame = gd.movie.end_frame;
            }

            gd.h_win.cmin_ht = frame * gd.h_win.delta + gd.h_win.min_ht;
            gd.h_win.cmax_ht = gd.h_win.delta + gd.h_win.cmin_ht;

            stat = gather_hwin_data(c_field,
                                    gd.movie.frame[index].time_start.unix_time,
                                    gd.movie.frame[index].time_end.unix_time);
            if(stat == SUCCESS)  {
                render_horiz_display(xid,c_field,
                                     gd.movie.frame[index].time_start.unix_time,
                                     gd.movie.frame[index].time_end.unix_time);
            } else {
                return stat;
            }

        break;
     
        default:
            fprintf(stderr,
                    "Invalid movie mode %d in render_h_movie_frame\n",
                    gd.movie.mode);
            break;
    }

    /* In Archive Mode or in Each Frame Mode the Movie frames have products in them */
    if(gd.movie.mode ==  MOVIE_TS || gd.prod.product_time_select == 1) { 
	    render_horiz_products(xid,
		gd.movie.frame[index].time_start.unix_time,
		gd.movie.frame[index].time_end.unix_time);
    }

    return stat;
}

/*************************************************************************
 * SAVE_H_MOVIE_FRAME: Saves the given pixmap in either a pixmap cache
 *  Or on disk as a compressed image. In HTML Mode, it saves the
 *  Frame as an XWD file, after rendering products
 */

save_h_movie_frame(index,xid,field)
    int    index;
    Drawable    xid;
    int    field;
{
    u_int    length;
    char    fname[1024];
    char    cmd[1024];
    XImage *h_img;
    FILE    *img_file;
    u_char    *rle_data;
    Window w;
    XWindowAttributes win_att;
    
    if(gd.debug1) fprintf(stderr,"Saving  horiz movie_frame %d\n",index);

    /* Save the image for HTML inclusion */
    if(gd.html_image_dir != NULL && gd.html_mode) {
        set_busy_state(1);

	/* Copy the image to the final drawing area pixmap */
	XCopyArea(gd.dpy, xid,
	   gd.h_win.can_xid, 
	   gd.def_gc,    0,0,
           gd.h_win.can_dim.width,
           gd.h_win.can_dim.height,
           gd.h_win.can_dim.x_pos,
           gd.h_win.can_dim.y_pos);
                                    
	/* Render the products onto the final drawing area pixmap  */
	render_horiz_products(gd.h_win.can_xid,
	    gd.movie.frame[gd.movie.cur_frame].time_start.unix_time, 
	    gd.movie.frame[gd.movie.cur_frame].time_end.unix_time);

	w = xv_get(gd.h_win_horiz_bw->horiz_bw,XV_XID);

        /* Save the final drawing area pixmap as an XWD file and run the conversion script */
	if(XGetWindowAttributes(gd.dpy,w,&win_att) != 0) {

	  sprintf(fname,"%s/cidd_%s_%d_%d.xwd",
	    gd.html_image_dir,
	    gd.mrec[field]->field_name,
	    gd.h_win.zoom_level,
	    index);

	  if((img_file = fopen(fname,"w")) != NULL) {
	    XwuDumpWindow(gd.dpy,gd.h_win.can_xid,&win_att,img_file);
	    if(fclose(img_file) !=0) {
	      fprintf(stderr,"Problem closing %s\n",fname);
	      perror("CIDD");
	    }

	    if(gd.html_convert_script != NULL) {
	       sprintf(cmd,"%s %s & \n", gd.html_convert_script,fname);
	       safe_system(cmd,30);
	    }
	  } else {
	    perror("Cidd: Problem opening xwd file");
	  }
	}
        set_busy_state(0);
    }

    /* If not the primary field;
     * - Is a background rendered image not to be saved 
     */
    if(field != gd.h_win.field) return 0;

    if(gd.movie.frame[index].h_xid != 0) {        /* can save to the pixmap */
        
        if(gd.debug1) fprintf(stderr,"save_h_movie_frame: Copy From XID: %d to Movie frame %d XID: %d\n",
							 xid,index,gd.movie.frame[index].h_xid);
        
        XCopyArea(gd.dpy,xid,
            gd.movie.frame[index].h_xid,
            gd.def_gc,  0,0,
            gd.h_win.can_dim.width,
            gd.h_win.can_dim.height,
            gd.h_win.can_dim.x_pos,
            gd.h_win.can_dim.y_pos);

    } else {
        
        /********* Save the Horizontal Display Image *********/
        h_img = XGetImage(gd.dpy,xid,
            0,0,gd.h_win.can_dim.width,gd.h_win.can_dim.height,
            (unsigned long)0xffff,ZPixmap);
    
        if(h_img == NULL) {
            fprintf(stderr,"Couldn't Get Horiz image\n");
            return -1;
        }
    
    
        /* Build the complete filename */
        STRcopy(fname,gd.movie.frame[index].fname,NAME_LENGTH -1);
        strcat(fname,"h");

        if(gd.debug1) fprintf(stderr,"Saving  horiz movie_frame %d - name %s to disk\n", index,fname);
        /* open the file for writing */
        if((img_file = fopen(fname,"w")) == NULL) {
            fprintf(stderr,"Couldn't open %s\n",fname);
            perror("cidd save_movie_frame");
            XDestroyImage(h_img);
            return -1;
        }
    
        /* Write the header */
        if(fwrite(h_img,sizeof(*h_img),1,img_file) != 1) {
            fprintf(stderr,"Couldn't write to %s\n",fname);
            perror("cidd save_movie_frame");
        }
    
        rle_data = RLEncode8((u_char *)h_img->data,
	    (h_img->width * h_img->height * (h_img->depth/ 8)),
	    gd.movie.key,&length);
         
        /* Write the compressed file length */
        if(fwrite(&length,sizeof(length),1,img_file) != 1) {
            fprintf(stderr,"Couldn't write to %s\n",fname);
            perror("cidd save_movie_frame");
        }

    
        /* Write the image data */
        if(fwrite(rle_data,length,1,img_file) != 1) {
            fprintf(stderr,"Couldn't write to %s\n",fname);
            perror("cidd save_movie_frame");
        }
        fclose(img_file);
        XDestroyImage(h_img);
        ufree(rle_data);
    }
    
    return(0);
}

/*************************************************************************
 * RETRIEVE_H_MOVIE_FRAME:
 */

retrieve_h_movie_frame(index,xid)
    int    index;
    Drawable xid;
{
    u_int    length,im_size;
    char    fname[1024];
    XImage *h_img;
    Visual    *visual;
    u_char    *im_data;
    u_char    *rle_data;
    u_char    *rld_data;
    FILE    *img_file;


    if(gd.movie.frame[index].h_xid != 0) {        /* can get from the pixmap */
        if(gd.movie.frame[index].h_xid != xid) {
            if(gd.debug1) fprintf(stderr, "retrieve_h_movie_frame: Copying Horizontal movie_frame %d Xid: %d to XID: %d\n",
                index,gd.movie.frame[index].h_xid, xid);
            XCopyArea(gd.dpy,gd.movie.frame[index].h_xid,
                xid,
                gd.def_gc,  0,0,
                gd.h_win.can_dim.width,
                gd.h_win.can_dim.height,
                gd.h_win.can_dim.x_pos,
                gd.h_win.can_dim.y_pos);
        }
    
    } else {
    
        /********* Retrieve the Horizontal Display Image *********/
        visual = DefaultVisual(gd.dpy,0);
    
        /* get memory areas for header and data */
        im_data = (u_char *) ucalloc((DefaultDepth(gd.dpy,0)/8),gd.h_win.can_dim.width * (gd.h_win.can_dim.height +1));

        h_img = XCreateImage(gd.dpy,visual,DefaultDepth(gd.dpy,0),ZPixmap,0,(char *)im_data,
            gd.h_win.can_dim.width,gd.h_win.can_dim.height,8,0);
    
        STRcopy(fname,gd.movie.frame[index].fname,NAME_LENGTH -1);    /* Build the complete filename */
        strcat(fname,"h");
        if((img_file = fopen(fname,"r")) == NULL) {    /* open the file for reading */
            fprintf(stderr,"Couldn't open %s\n",fname);
            perror("cidd display_movie_frame");
            return;
        }
        if(gd.debug1) fprintf(stderr, "Retrieving H movie_frame %d file: %s to XID: %d\n",
            index,fname, xid);
    
        if(fread(h_img,sizeof(*h_img),1,img_file) != 1) {    /* Read the header */
            fprintf(stderr,"Couldn't Read from %s\n",fname);
            perror("cidd display_movie_frame");
        }

        h_img->data = (char *) im_data;    /* Set the pointer to the ucalloc'ed area */
    
        /* Read the RLE image data length */
        if(fread(&length,sizeof(length),1,img_file) != 1) {
            fprintf(stderr,"Couldn't read from %s\n",fname);
            perror("cidd display_movie_frame");
        }
         
        rle_data = (u_char *) ucalloc(1,length);

        /* Read the RLE image data */
        if(fread(rle_data,length,1,img_file) != 1) {
            fprintf(stderr,"Couldn't read from %s\n",fname);
            perror("cidd display_movie_frame");
        }
        fclose(img_file);
    
        rld_data = RLDecode8(rle_data,&length);
        memcpy((char *)im_data,(char *)rld_data,length);
    
        h_img->data =  (char *)im_data;    /* Set the pointer to the calloc'ed area */
        /* Now copy to the canvas */
        XPutImage(gd.dpy,xid,gd.def_gc,h_img,0,0,0,0,
            gd.h_win.can_dim.width,gd.h_win.can_dim.height);
    
        ufree(rle_data);
        ufree(rld_data);
        XDestroyImage(h_img);    /* This call also frees im_data */
    
    }

}

/*************************************************************************
 * RENDER_V_MOVIE_FRAME:
 */

render_v_movie_frame(index,xid)
    int    index;
    Drawable xid;
{
    int    c_field;
    int    frame;
    int    stat;

    UTIMstruct     temp_utime;
    

    c_field = gd.v_win.field;

    if(gd.debug2) fprintf(stderr, "Rendering Vertical movie_frame %d - field %d\n", index, c_field);

    switch(gd.movie.mode) {
        case MOVIE_MR:
        case MOVIE_TS:
            stat = render_vert_display(xid, c_field,
                                       gd.movie.frame[index].time_start.unix_time,
                                       gd.movie.frame[index].time_end.unix_time);
        break;
         
        case MOVIE_EL:
            if(gd.movie.cur_frame >= 0) { frame = gd.movie.cur_frame;
            } else { frame = gd.movie.end_frame; }
            gd.h_win.cmin_ht = frame * gd.h_win.delta + gd.h_win.min_ht;
            gd.h_win.cmax_ht = gd.h_win.delta + gd.h_win.cmin_ht;

            stat = render_vert_display(xid,c_field, gd.movie.frame[index].time_start.unix_time,
                gd.movie.frame[index].time_end.unix_time);
        break;
     
    }

    return stat;
}

/*************************************************************************
 * SAVE_V_MOVIE_FRAME:
 */

save_v_movie_frame(index,xid)
    int    index;
    Drawable xid;
{
    int    length;
    char    fname[1024];
    XImage *v_img;
    FILE    *img_file;
    u_char    *rle_data;
    
    if(gd.debug1) fprintf(stderr,"Saving movie_frame %d\n",index);

    if(gd.movie.frame[index].v_xid != 0) {        /* can save to the pixmap */
        if(gd.debug1) fprintf(stderr, "save_v_movie_frame: Copying Vertical Xid: %d to movie_frame %d XID: %d\n",
             xid,index,gd.movie.frame[index].v_xid);
        XCopyArea(gd.dpy,xid,
            gd.movie.frame[index].v_xid,
            gd.def_gc,  0,0,
            gd.v_win.can_dim.width,
            gd.v_win.can_dim.height,
            gd.v_win.can_dim.x_pos,
            gd.v_win.can_dim.y_pos);

    } else {

        /********* Save the Vertical Display Image *********/
        v_img = XGetImage(gd.dpy,xid,
            0,0,gd.v_win.can_dim.width,gd.v_win.can_dim.height,
            (unsigned long)0x00ff,ZPixmap);
    
        if(v_img == NULL) {
            fprintf(stderr,"Couldn't Get Vert image\n");
            return -1;
        }
         
    
        STRcopy(fname,gd.movie.frame[index].fname,NAME_LENGTH -1);    /* Build the complete filename */
        strcat(fname,"v");
        if((img_file = fopen(fname,"w")) == NULL) {    /* open the file for writing */
            fprintf(stderr,"Couldn't open %s\n",fname);
            perror("cidd save_movie_frame");
            return -1;
        }
    
        if(fwrite(v_img,sizeof(*v_img),1,img_file) != 1) {    /* Write the header */
            fprintf(stderr,"Couldn't write to %s\n",fname);
            perror("cidd save_movie_frame");
        }
    
        rle_data = RLEncode8((u_char *)v_img->data,(v_img->width * v_img->height),gd.movie.key,&length);
         
        /* Write the compressed file length */
        if(fwrite(&length,sizeof(length),1,img_file) != 1) {
            fprintf(stderr,"Couldn't write to %s\n",fname);
            perror("cidd save_movie_frame");
        }
    
        /* Write the image data */
        if(fwrite(rle_data,length,1,img_file) != 1) {
            fprintf(stderr,"Couldn't write to %s\n",fname);
            perror("cidd save_movie_frame");
        }
        fclose(img_file);
        ufree(rle_data);
        XDestroyImage(v_img);
    }
    return(0);
}

/*************************************************************************
 * RETRIEVE_V_MOVIE_FRAME:
 */

retrieve_v_movie_frame(index,xid)
    int    index;
    Drawable xid;
{
    int    length,im_size;
    char    fname[1024];
    XImage *v_img;
    Visual    *visual;
    u_char    *im_data;
    u_char    *rle_data;
    u_char    *rld_data;
    FILE    *img_file;

    if(gd.debug1) fprintf(stderr,"Retrieving v_movie_frame %d\n",index);

    if(gd.movie.frame[index].v_xid != 0) {        /* can save to the pixmap */
        if(gd.movie.frame[index].v_xid != xid) {
            if(gd.debug1) fprintf(stderr, "retrieve_v_movie_frame: Copying Horizontal movie_frame %d Xid: %d to XID: %d\n",
                index,gd.movie.frame[index].v_xid, xid);
            XCopyArea(gd.dpy, gd.movie.frame[index].v_xid,
                xid,
                gd.def_gc,  0,0,
                gd.v_win.can_dim.width,
                gd.v_win.can_dim.height,
                gd.v_win.can_dim.x_pos,
                gd.v_win.can_dim.y_pos);
        }

    
    } else {
    
        /********* Retrieve the Vertical Display Image *********/
        visual = DefaultVisual(gd.dpy,0);
    
        /* get memory areas for header and data */
        im_data = (u_char *) ucalloc(1,gd.v_win.can_dim.width * (gd.v_win.can_dim.height +1));
        rle_data = (u_char *) ucalloc(1,gd.v_win.can_dim.width * (gd.v_win.can_dim.height +1));
    
        v_img = XCreateImage(gd.dpy,visual,8,ZPixmap,0, (char *)im_data,
            gd.v_win.can_dim.width,gd.v_win.can_dim.height,8,0);
    
        /* Build the complete filename */
        STRcopy(fname,gd.movie.frame[index].fname,NAME_LENGTH -1);
        strcat(fname,"v");
        /* open the file for reading */
        if((img_file = fopen(fname,"r")) == NULL) {
            fprintf(stderr,"Couldn't open %s\n",fname);
            perror("cidd display_movie_frame");
            return -1;
        }
    
        /* read the header */
        if(fread(v_img,sizeof(*v_img),1,img_file) != 1) {
            fprintf(stderr,"Couldn't read from %s\n",fname);
            perror("cidd display_movie_frame");
        }
    
        /* Read the RLE image data length */
        if(fread(&length,sizeof(length),1,img_file) != 1) {
            fprintf(stderr,"Couldn't read from %s\n",fname);
            perror("cidd display_movie_frame");
        }
         
        /* Read the image data */
        if(fread(rle_data,length,1,img_file) != 1) {
            fprintf(stderr,"Couldn't read from %s\n",fname);
            perror("cidd display_movie_frame");
        }
        fclose(img_file);
    
    
        /* im_size = rld_image(rle_data,im_data,gd.movie.key,length); */

        rld_data = RLDecode8(rle_data,&length);
        memcpy((char *)im_data,(char *)rld_data,length);

        v_img->data = (char *) im_data;
            
        /* Now copy to the canvas */
        XPutImage(gd.dpy,xid,gd.def_gc,v_img,0,0,0,0,
            gd.v_win.can_dim.width,gd.v_win.can_dim.height);
    
        ufree(rle_data);
        ufree(rld_data);
        XDestroyImage(v_img);    /* This call also frees im_data */
    }
    return 0;
}
    
/*************************************************************************
 * TIME_FOR_A_NEW_FRAME: Decide if it is time to create a new movie frame
 *    in the Most Recent movie mode
 */

time_for_a_new_frame()
{
    long c_time = time(0);

    c_time += (gd.movie.forecast_interval * 60.0);

    if(c_time > (gd.movie.start_time.unix_time + (gd.movie.num_frames * gd.movie.time_interval * 60))) {
        return 1;
    }
    
    return 0;
}

#ifndef LINT
static char RCS_id[] = "$Id: cidd_movie.c,v 1.25 2016/03/07 18:28:26 dixon Exp $";
#endif /* not LINT */
