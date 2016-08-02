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
/**********************************************************************
 * CIDD_EXPTI.C:  Routines for interfacing with expt in CIDD.
 *
 *
 * For the Cartesian Radar Display (CIDD)
 * N. Rehak     October 1991 NCAR, Research Applications Program
 *
 * Hacked from expti.c by Zhongqi Jing.
 */


#include <sys/signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <rdi/expt_user.h>
#include <toolsa/globals.h>

#include "cidd.h"
#include "cidd_macros.h"

#define SA_DATA_LENGTH 14     /* size of sa_data portion of sockaddr struct */
                              /* This is not #defined in sys/socket.h.      */

#define EXPT_DDATA    200
#define EXPT_ADATA    100

/*
 * expt message defines
 */

#define EXPT_MSG_SIZE               4

#define EXPT_START_CONNECTION    1000
#define EXPT_OPEN_WINDOW           -1
#define EXPT_CLOSE_WINDOW          -2
#define EXPT_NEW_SCALE             -3

/*
 * constants used by lock_shm1 routine
 */

#define SHMEM_UNLOCK                0
#define SHMEM_LOCK                  1

/*
 * socket states used in send_event
 */

#define SOCKET_NOT_CONNECTED        0
#define SOCKET_CONNECTED            2

static char sock_dir[16]={"./"};   /* directory to contain e_soc socket file */

/*
 * shared memory identifiers
 */

int shmid;
char *shmpt = NULL;

/*
 * pointers into shared memory
 */

Com_msg *cmsg;   /* pointer to Com_msg struct in shared memory */
Draw_data *dd;   /* pointer to first Draw_data struct in shared memory */
Wcgraph *wcg;    /* pointer to Wcgraph struct in shared memory */

/*
 * flag indicating current status of connection to expt
 */

static int export_on = FALSE;



/**********************************************************************
 * POLL_EXPT - Check to see if the status of expt has changed
 */


int poll_expt()

{

    static int last = -1;
    static double last_scale = -1.0;
    double new_scale;

    int msg[EXPT_MSG_SIZE];              /* message sent to expt */

    unsigned int extras_bits;
    Drawable xid;



    /*
     * Check the shared memory segment to see if expt is up
     */

    /*lock_shm1(SHMEM_LOCK);*/

    if (cmsg->keye == EXPT_KEY)
        export_on = TRUE;
    else
        export_on = FALSE;

    /*
     * See if the running status has changed
     */

    if (last != export_on) {
        if(!export_on) {
            if (gd.debug) fprintf(stderr, "Expt is off\n");

            cmsg->nd = 0;

            /*
             * clean up the display if we were in drawing mode when expt
             * stopped.
             */

            if (gd.drawing_mode) {
                /*
                 * display an error message in the window label
                 */

                rd_h_msg("Error: expt has stopped running.\n",0);
                
                if (gd.debug2) fprintf(stderr,"\nExpti: Displaying Horiz final image - xid: %d to xid: %d\n",
			         gd.h_win.can_xid,gd.h_win.vis_xid);
    
                XCopyArea(gd.dpy, gd.h_win.can_xid,
                          gd.h_win.vis_xid,
                          gd.def_gc,    0,0,
                          gd.h_win.can_dim.width,
                            gd.h_win.can_dim.height,
                          gd.h_win.can_dim.x_pos,
                           gd.h_win.can_dim.y_pos);
                /*
                 * deselect the "Draw" button 
                 */

                xv_set(gd.h_win_horiz_bw->export_st,PANEL_VALUE, 0, NULL);

                /*
                 * clear the drawing mode flag
                 */

                gd.drawing_mode = FALSE;
            } /* endif - drawing mode */
            
        } /* endif - expt stopped */
        else {
            if (gd.debug) fprintf(stderr, "Expt is on\n");
        } /* endelse - expt started */

    } /* endif - expt running status changed */

    /*
     * save the current state of expt
     */

    last = export_on;


    /*
     * Update display variables in shared memory
     */

    cmsg->dtime = gd.mrec[gd.h_win.field]->h_date.unix_time;
    cmsg->adata = EXPT_ADATA;

    new_scale = (gd.h_win.cmax_y  - gd.h_win.cmin_y) / gd.h_win.img_dim.height;

    if (new_scale != last_scale) {
        last_scale = new_scale;
        cmsg->scale = (int)((double)gd.expt_feature_scale * new_scale + 0.5);

        if (export_on) {
            msg[0] = EXPT_NEW_SCALE;
            send_event(msg);
        } /* endif */
        
    } /* endif - new scale value */
    

    /*lock_shm1(SHMEM_UNLOCK);*/

    if (!gd.drawing_mode) return (0);

    if (!export_on) return (1);

    /*
     * check to see if the graphics need updating
     */

    if (cmsg->newly_updated) {
        if (gd.debug2) fprintf(stderr,"\nExpti: Displaying Horiz final image - xid: %d to xid: %d\n",
			         gd.h_win.can_xid,gd.h_win.vis_xid);
    
        XCopyArea(gd.dpy, gd.h_win.can_xid,
                  gd.h_win.vis_xid,
                  gd.def_gc,    0,0,
                  gd.h_win.can_dim.width,
                  gd.h_win.can_dim.height,
                  gd.h_win.can_dim.x_pos,
                  gd.h_win.can_dim.y_pos);
        /*
         * redraw the expt images
         */

        redraw_expt_image();
    } /* endif */

    return (0);

} /* end of poll_expt */


/*********************************************************************
 * INIT_EXPTI : Init shared memory and globals used by the drawing
 *              utility (expti)
 */

int init_expti()

{
    char *cpt;
    unsigned int size;   /* size of needed shared memory segment */
    int expt_key;
    int msg[EXPT_MSG_SIZE];          /* socket message sent to expt */
    char font_name[120];


    /*
     * Make sure the needed shared memory region exists
     */

    expt_key = XRSgetLong(gd.cidd_db, "cidd.expt_key", SHM_KEY_EXPT);

    size = sizeof(Com_msg) + MAX_DRAW * sizeof(Draw_data) + sizeof(Wcgraph);


    if ((shmid = shmget((key_t)expt_key, size, 0666 | IPC_CREAT)) < 0) {
        if (gd.debug) fprintf(stderr, "Error shmget - expti.\n");
        return(1);
    } /* endif */

    /*
     * Attach to the shared memory region
     */

    shmpt = (char *)shmat(shmid, (char *)0, 0);

    if ((int)shmpt == -1) {
        if (gd.debug) fprintf(stderr, "Error shmat - expti.\n");
        return(1);
    } /* endif */

    /*
     * Calculate the needed pointers into the shared memory region
     */

    cmsg = (Com_msg *)shmpt;
    cpt = shmpt + sizeof(Com_msg);
    dd = (Draw_data *)cpt;
    cpt = cpt + MAX_DRAW * sizeof(Draw_data);
    wcg=(Wcgraph *)cpt;

    /*
     * Update the appropriate values in shared memory to tell expt that
     * we are up and running
     */

    lock_shm1(SHMEM_LOCK); 

    cmsg->keyd = DISP_KEY;
    cmsg->ddata = EXPT_DDATA;   /* the feature size */

    lock_shm1(SHMEM_UNLOCK);

    /*
     * Send a msg to expt to start the connection.
     * This is not necessary, if the connection is not made here it will
     * be made when the draw button is clicked.
     */

    msg[0] = EXPT_START_CONNECTION; 
    send_event(msg);

    return (0);

} /* end of init_expti */


/**********************************************************************
 * LOCK_SHM1 : lock/unlock the expt shared memory region.  When locking,
 *             blocks until shared memory is accessible
 */

int lock_shm1(lock_flag)

int lock_flag;

{

    /*
     * initialize static structures
     */

    static struct sembuf op_lock[2] =
    {
        0, 0, 0,          /* wait for sem #0 to become 0 */
        0, 1, SEM_UNDO    /* increment sem */
    };

    static struct sembuf op_unlock[1] =
    {
        0, -1, (IPC_NOWAIT | SEM_UNDO)  /* decrement sem */
    };

    static int semid = -1;


    /*
     * If needed, get a semaphore from the system
     */

    if (semid < 0)         /* do not currently have a semaphore */
    {
        if ((semid = semget(SHM_KEY_EXPT, 1, IPC_CREAT | 0666)) < 0) {
            if (gd.debug) fprintf(stderr, "Error in semget.\n");
            return (1);
        } /* endif */
    } /* endif */

    if (lock_flag == SHMEM_LOCK)
    {

        while (semop(semid, &op_lock[0], 2) < 0) {
            if (errno != EINTR) {
                if (gd.debug) fprintf(stderr, "Error in lock shm (errno: %d).\n",errno);
                return (1);
            }/* endif */

        } /* endwhile */

        return (0);
    } /* endif - lock shared memory */

    if (lock_flag == SHMEM_UNLOCK) {
        if (semop(semid, &op_unlock[0], 1) < 0) {
            if (gd.debug) fprintf(stderr, "shm. not locked\n");
            return (1);
        } /* endif */

        return (0);

    } /* endif - unlock shared memory */

    return 0;
} /* end of lock_shm1 */


/**********************************************************************
 * REDRAW_EXPT_IMAGE : Render all of the data in the expt shared memory
 *                     region.  This routine assumes that all previous
 *                     expt drawing has been cleared.
 */


int redraw_expt_image()

{
    int i;
    int num_pts;


    lock_shm1(SHMEM_LOCK);
    
    /*
     * draw visible features
     */

    for (i = 0; i < cmsg->nd; i++)
    {
        /*
         * mark the feature as not currently displayed
         */

        dd[i].displaied = 0;

        /*
         * display the visible features
         */

        if (dd[i].show == 1)
        {
            if (dd[i].act > 0)   /* this is the active feature */
                num_pts = dd[i].ng;
            else
                num_pts = dd[i].nt;

            /*
             * draw the feature
             */

            draw_feature(dd[i].xt, dd[i].yt, num_pts);

            /*
             * draw the label
             */

            draw_label(dd[i].lab_x, dd[i].lab_y, dd[i].lab_str);

            /*
             * indicate that the feature has been displayed
             */

            dd[i].displaied = 1;

        } /* endif - feature should be shown */

    } /* endfor - i */

    cmsg->newly_updated = 0;
    lock_shm1(SHMEM_UNLOCK);

    return(0);

} /* end of redraw_expt_image */


/******************************************************************
 * DRAW_LABEL : Draw the label on the display
 */


int draw_label(lab_x, lab_y, lab_str)

short lab_x, lab_y;
char *lab_str;

{
    double km_x, km_y;
    int pixel_x, pixel_y;
    GC    gc;


    km_x = (double)lab_x / (double)CIDD_TO_EXPT_DATA;
    km_y = (double)lab_y / (double)CIDD_TO_EXPT_DATA;
        
    km_to_pixel(&gd.h_win.margin, km_x, km_y, &pixel_x, &pixel_y);

    XDrawString(gd.dpy,gd.h_win.vis_xid,
                         gd.extras.foreground_color->gc,
			 pixel_x,pixel_y,lab_str,
			 strlen(lab_str));
        
    return (0);

} /* end of draw_label */


/***************************************************************
 * DRAW_FEATURE : Transfer the points from shared memory to
 *                the display.
 */

int draw_feature(x_array, y_array, num_pts)

int *x_array, *y_array;
int num_pts;

{
    int i,x,y;
    double km_x, km_y;
    int pixel_x1 = 0, pixel_y1 = 0, pixel_x2 = 0, pixel_y2 = 0;
    int first_pt = TRUE;


    /*
     * make sure we have an appropriate number of points for drawing
     */

    if (num_pts <= 0)
        return (0);

    if (num_pts >= MAX_GPT)
    {
        if (gd.debug2)
            fprintf(stderr, "Too many points in expt feature.\n");
        return (0);
    } /* endif */

    /*
     * process the first point in the feature
     */

    if (x_array[0] < SYM_XT)    /* the point is "drawable" */
    {
        /*
         * convert from expt world to pixel coordinates
         */

        km_x = (double)x_array[0] / (double)CIDD_TO_EXPT_DATA;
        km_y = (double)y_array[0] / (double)CIDD_TO_EXPT_DATA;
        
        km_to_pixel(&gd.h_win.margin, km_x, km_y,
                    &pixel_x1, &pixel_y1);
        
        /*
         * "draw" the point
         */

        if (num_pts == 1)
            draw_mark(pixel_x1, pixel_y1);
        else
        {
            if (x_array[1] == DIC_XT)       /* this point is separate from */
                                            /* the rest of the feature     */
                draw_mark(pixel_x1, pixel_y1);
            else                 /* we have the first point in a line */
                first_pt = FALSE;
        } /* endif */
    } /* endif */
 
    /*
     * process the middle points in the feature
     */

    for (i = 1; i < num_pts - 1; i++)
    {
        if (x_array[i] == DIC_XT)      /* the next point is separate from */
                                       /* the previous point in the feature */
        {
            first_pt = TRUE;
            continue;
        } /* endif */

        if (x_array[i] == SYM_XT)      /* symbol termination */
            continue;

        /*
         * convert from expt world to pixel coordinates
         */

        km_x = (double)x_array[i] / (double)CIDD_TO_EXPT_DATA;
        km_y = (double)y_array[i] / (double)CIDD_TO_EXPT_DATA;
        
        km_to_pixel(&gd.h_win.margin, km_x, km_y,
                    &pixel_x2, &pixel_y2);
        
        /*
         * "draw" the point -- check for this point being completely
         * separate from the rest of the feature
         */

        if (x_array[i-1] == DIC_XT &&
            x_array[i+1] == DIC_XT &&
            x_array[i] != DIC_XT)
            draw_mark(pixel_x2, pixel_y2);
        else if (!first_pt)
            XDrawLine(gd.dpy, gd.h_win.vis_xid,
		gd.extras.foreground_color->gc,
                pixel_x1, pixel_y1, pixel_x2, pixel_y2);
        
        /*
         * we have drawn a point, so we are no longer looking for the first
         * point in a series of connected points
         */

        first_pt = FALSE;

        /*
         * save the end point to be used as the beginning point in the next
         * line segment
         */

        pixel_x1 = pixel_x2;
        pixel_y1 = pixel_y2;
    } /* endfor - i */

    /*
     * process the last point in the feature
     */

    if (x_array[num_pts-1] < SYM_XT)
    {
        /*
         * convert from expt world to pixel coordinates
         */

        km_x = (double)x_array[num_pts-1] / (double)CIDD_TO_EXPT_DATA;
        km_y = (double)y_array[num_pts-1] / (double)CIDD_TO_EXPT_DATA;
        
        km_to_pixel(&gd.h_win.margin,
                    km_x, km_y,
                    &pixel_x2, &pixel_y2);
        
        /*
         * draw the point -- check for this point being completely
         * separate from the rest of the feature
         */

        if (x_array[num_pts-2] == DIC_XT)
            draw_mark(pixel_x2, pixel_y2);
        else if (!first_pt)
            XDrawLine(gd.dpy, gd.h_win.vis_xid, gd.extras.foreground_color->gc,
                      pixel_x1, pixel_y1, pixel_x2, pixel_y2);
    } /* endif */

    return (0);

} /* end of draw_feature */


/**************************************************************
 * DRAW_MARK : Draw an isolated point on the display.
 */

int draw_mark(x, y)

int x, y;

{

    XDrawLine(gd.dpy, gd.h_win.vis_xid, gd.extras.foreground_color->gc,
              x - gd.expt_mark_size, y,
              x + gd.expt_mark_size, y);

    XDrawLine(gd.dpy, gd.h_win.vis_xid, gd.extras.foreground_color->gc,
              x,                  y - gd.expt_mark_size,
              x,                  y + gd.expt_mark_size);
    

    return (0);

} /* end of draw_mark */


/******************************************************************
 * SEND_EVENT : Send a user event to the drawing utility - expt
 */


int send_event(msg)

int *msg;

{
    static struct sockaddr sn;
    static int socket_desc;
    static int status = SOCKET_NOT_CONNECTED;
                            /* current socket connection status: */
                            /*      SOCKET_NOT_CONNECTED         */
                            /*      SOCKET_CONNECTED             */

    extern int errno;

    int len;                /* length of message to send */
    char socket_name[32];


    /*
     * Open the socket
     */

    if (status == SOCKET_NOT_CONNECTED)
    {

        /*
         * Create the socket descriptor
         */

        if ((socket_desc = socket(AF_UNIX, SOCK_STREAM, 0))
                         < 0)
        {
            if (gd.debug)
                fprintf(stderr, "Failed in opening a socket.\n");
            return(-1);
        } /* endif */

        /*
         * Create the socket address
         */

        sn.sa_family = AF_UNIX;
        sprintf(socket_name, "%s%s", sock_dir, "e_soc");
        STRcopy(sn.sa_data, socket_name, SA_DATA_LENGTH);

        /*
         * Try to get connection 
         */

        if (connect(socket_desc, &sn, sizeof(sn)) == -1)
        {
            close(socket_desc);
            /*printf("Failed in connect (errno=%d)\n",errno);*/
            return (-1);
        } /* endif */

        status = SOCKET_CONNECTED;
    } /* endif */

    /*
     * Write the message to the socket
     */

    len = EXPT_MSG_SIZE * sizeof(int);
    if (write(socket_desc, msg, len) == len)
        return (1);

    /*
     * If we get here, there was an error writing the message to the socket.
     */

    if (errno == EPIPE)
    {
        if (gd.debug) fprintf(stderr, "connection to expt is stopped.\n");
        status = SOCKET_NOT_CONNECTED;
        gd.drawing_mode = FALSE;
        close(socket_desc);
        return(-3);
    } /* endif */
    
    if (gd.debug1)
        fprintf(stderr, "Socket buffer full - display to expt.\n");

    return (0);

} /* end of send_event */


/******************************************************************
 * SWITCH_THE_WINDOW_EXPT : Send the appropriate message to expt to
 *                          toggle on/off the expt window
 */

int switch_the_window_expt(int state)

{
    int msg[EXPT_MSG_SIZE];    /* message sent to expt */
    Drawable xid;


    /*
     * Check to see if expt is currently running
     */

    if (!export_on) {
        if (gd.debug)
            fprintf(stderr, "Expt is not running\n");
        return (1);

    } /* endif - expt isn't currently running */

    /*
     * Turn on/off the expt window based on the current drawing mode
     */

    if (state == 0) {
        /* set the drawing mode */
        gd.drawing_mode = FALSE;

        /*
         * send a "close window" message to expt
         */

        msg[0] = EXPT_CLOSE_WINDOW;
        send_event(msg);
        
        if (gd.debug) fprintf(stderr, "Sending close window msg to expt\n");

        if (gd.debug2) fprintf(stderr,"\nExpti: Displaying Horiz final image - xid: %d to xid: %d\n",
			         gd.h_win.can_xid,gd.h_win.vis_xid);
    
	/* Copy clean image to the visible pixmap */
        XCopyArea(gd.dpy, gd.h_win.can_xid,
                  gd.h_win.vis_xid,
                  gd.def_gc,    0,0,
                  gd.h_win.can_dim.width,
                  gd.h_win.can_dim.height,
                  gd.h_win.can_dim.x_pos,
                  gd.h_win.can_dim.y_pos);
                   
    }  else {
     
       /* set the drawing mode */
        gd.drawing_mode = TRUE;

        /*
        * send an "open window" message to expt
        */

        msg[0] = EXPT_OPEN_WINDOW;
        send_event(msg);
         if (gd.debug) fprintf(stderr, "Sending open window msg to expt\n");
   
         /* redraw the features the user has already drawn */

        redraw_expt_image();

    }

    return (0);
} 


/******************************************************************
 * GET_CURRENT_TIME : Get the current time from the system
 */


int get_current_time()

{
    struct timeval tp;
    struct timezone tzp;

    if (gettimeofday(&tp, &tzp) != 0)
        return (-1);

    return(tp.tv_sec);

} /* end of get_current_time */


/****************************************************************
 * WINDOW_TO_WORLD : Convert pixel coordinates to expt world
 *                   coordinates
 */

int window_to_world(win_x, win_y, world_x, world_y)

int win_x, win_y;
int *world_x, *world_y;

{
    double km_x, km_y;
    

    pixel_to_km(&gd.h_win.margin, win_x, win_y, &km_x, &km_y);
    *world_x = (int)(km_x * CIDD_TO_EXPT_DATA + 0.5);
    *world_y = (int)(km_y * CIDD_TO_EXPT_DATA + 0.5);
    
    return(0);
    
} /* end of window_to_world */

/**********************************************************************
 * DELETE_EXPT_SHMEM - Detach from the PRDS shared memory segment
 */

void delete_expt_shmem(void)
{
  if (shmpt != NULL)
  {
    if (shmdt((char *)shmpt) < 0)
      fprintf(stderr, "Error detaching from expt shared memory segment\n");

    if (ushm_nattach(XRSgetLong(gd.cidd_db, "cidd.expt_key", 6713)) <= 0)
      ushm_remove(XRSgetLong(gd.cidd_db, "cidd.expt_key", 6713));
  }
}

#ifndef LINT
static char RCS_id[] = "$Id: cidd_expti.c,v 1.14 2016/03/07 18:28:26 dixon Exp $";
#endif /* not LINT */
