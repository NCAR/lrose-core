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
/******************************************************************
 * CIDD_PRDS.C: Routines used to draw items selected in the
 *        Product Database/Selector (PRDS).
 *
 *    Specific to the CIDD program - Uses global color table,
 *        Window dimensions & coordinate transforms
 *
 * Based on Code from Z. Jing
 * Frank Hage, NCAR    July 1991
 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>


#include <rdi/overlay_object.h>
#include "cidd.h"

static int shmid;    /* Shared memory segment */
static char *shmpt = NULL;

Prds_product *prod;    /* Pointer to array of product structs */
Out_msg *omsg;    /* Output message to selector */
In_msg *imsg;    /* Input message */

int *data; /* address for data field (points and texts) */
/* Color Order: Red,Green,Blue Yellow,Cyan,Magenta White,Black */

/**********************************************************************
 * POLL_PRDS - Check to see if the status of prds has changed
 *  Returns 0 if no change, 1 otherwise 
 */


int poll_prds()

{
    static int product_state = -1;

    /*
     * Check the shared memory segment to see if prds is up
     */

    if (omsg->key != VALID) return(0);

    if (omsg->newly_updated || (gd.prod.products_on != product_state)) {
	product_state = gd.prod.products_on;
	return 1;
    } 

    return 0;

} /* end of poll_prds */


/*********************************************************************
 * INIT_IMPORT:  initialize shared memory, call in main
 */

int init_import()
{
    char *cpt;
    int  prds_key,shmem_size;

    prds_key = XRSgetLong(gd.cidd_db, "cidd.prds_key", SHM_KEY);
    shmem_size = XRSgetLong(gd.cidd_db, "cidd.prds_shmem_size", SHM_SIZE) * 1024;

    /* open shared memory segment */
    if((shmid=shmget((key_t)prds_key,shmem_size,0666|IPC_CREAT))<0){
        fprintf(stderr,"Error shmget - prds.\n");
        return(1);
    }
    shmpt=(char *)shmat(shmid,(char *)0,0);
    if((int)shmpt==-1){
        fprintf(stderr,"Error shmat - prds.\n");
        return(1);
    }
    imsg=(In_msg *)shmpt;        /* get pointer to input message */

    cpt=shmpt+sizeof(In_msg);    /* get pointer to Output mesage */
    omsg=(Out_msg *)(cpt);

    cpt=cpt+sizeof(Out_msg);    /* get pointer to array of product structures */
    prod=(Prds_product *)cpt;

    cpt=cpt+N_PROD*sizeof(Prds_product);        /* get pointer to start of vector & text data */
    data=(int *)cpt;

    /*lock_shm(0);  initialize semaphore */

    return (0);

}

/******************************************************************
 * SWITCH_THE_WINDOW: turn on/off the prds window. Called by overlay
 *             button     
 */

int switch_the_window_prds(w_on)
{
    if(omsg->key!=VALID) {
        return (1);
    }

    switch(w_on) {
        case 0:
            STRcopy(imsg->msg, PRDS_CLOSE_WIN_CMD,20);
            if(gd.debug1) printf("Sending signal SIGUSR1 to %d\n",omsg->pid);
            kill(omsg->pid,SIGUSR1);
        break;

        case 1:
            STRcopy(imsg->msg, PRDS_SHOW_WIN_CMD,20);
            if(gd.debug1) printf("Sending signal SIGUSR1 to %d\n",omsg->pid);
            kill(omsg->pid,SIGUSR1);
        break;
    }
    return (0);
}

/**********************************************************************
 * DRAW_PRDS_PRODUCTS : render the selected graphics products 
 */

int draw_prds_products(xid,start,end)
    Drawable    xid;                        /* the canvas to draw into */
    long    start,end;        /* time range for this image */
{
    int i,t,npoints;
    int color;
    int tp;                /* text position */
    int *ipt;            
    int    *ept;
    int x,y;
    int x1,y1,x2,y2,xx,yy,xpix,ypix;
    int    cord;            /* packed coordinate */
    int read_h;
    int func;            /* flag to indicate which primitive function */
    int tmp;            /* temporary variable to hold current object */
    int text_done;        /* flag indicating text is done */
    int pen_up;
    long t_start, t_end;   /* temp start, end times */
    double     xloc,yloc;    /* world coordinates in KM */
    GC    gc;
    XRectangle    rect;
    XPoint xpoints[1024];

static     int xorg = 0;     /* not every product sets the origin */
static     int yorg = 0;     /* not every product sets the origin */

    /* Set the clipping rectangle for the window */
    rect.x = gd.h_win.img_dim.x_pos;
    rect.y = gd.h_win.img_dim.y_pos;
    rect.width = gd.h_win.img_dim.width;
    rect.height = gd.h_win.img_dim.height;

    for(i=0; i < PRDS_NUM_COLORS; i++) {
        XSetClipRectangles(gd.dpy,gd.extras.prod.prds_color[i]->gc,0,0,&rect,1,YXSorted);
    }
    t_start = ((start + end) / 2) - (gd.prod.product_time_width / 2);
    t_end =   ((start + end) / 2) + (gd.prod.product_time_width / 2);

    lock_shm(1);    /* Lock the shared memory segment */

    if(gd.debug2) {
        printf("%d products Xid: %d Start: %s,",omsg->np,xid,asctime(gmtime(&start)));
        printf(" End: %s\n",asctime(gmtime(&end)));
    }

    for (i = 0; i < omsg->np; i++) {    /* Loop through all products */
        if (prod[i].selected <= 12)     /* Skip deselected products */
            continue;
        
	 if(gd.debug2) printf("Prds_product %d: Start: %d, End: %d\n",i,prod[i].v_time,prod[i].e_time);
         switch(gd.prod.product_time_select) {
	    case 1:   /* Image window timing */
            /* Skip if not in the selected time interval unless it is a map */
             if((prod[i].tindex >= 0) && (start > prod[i].e_time ))
                continue;
    
             if((prod[i].tindex >= 0) && (end < prod[i].v_time))
                continue;
	    break;

	    case 2:   /* variable window timing */
            /* Skip if not in the selected time interval unless it is a map */
             if((prod[i].tindex >= 0) && (t_start > prod[i].e_time ))
                continue;
    
             if((prod[i].tindex >= 0) && (t_end < prod[i].v_time))
                continue;
	    break;

	    default:  /* do nothing */
	    break;
	}

        /* draw the graphics objects */
           if (prod[i].llen > 0) {
            tp=prod[i].lpt;    /* get pointer to where the graphics data starts */
            ipt=&data[tp];
            ept = ipt + prod[i].llen;

            read_h=1;

            while(1){
                tmp=*ipt;
                if(tmp== END_OF_DATA) break;
    
                if(read_h==1) { /* header - Get commands in first long word */        
                    color= (tmp>>16) & 0xff ;    /* get color index */
                    if(color > 7) color = 7;    /* Clamp to available colors */
                    if(color < 0) color = 0;

		    gc = gd.extras.prod.prds_color[color]->gc;

                    func = tmp & 0xff;            /* Get which type of primitive object (func - 0,  lines, 1 or offsets -2) */
         
                    if(ipt >= ept -1) break;
                     
                    xx=((ipt[1]<<16)>>16);    /*     get X coordinate */
                    yy=(ipt[1]>>16);        /*  get Y coordinate */

                    if(func== RESET_ORIGIN){             /*  set the origin of the object (in 10 meter units)  */
                         /*  xorg=xx; /*  */
                          /*  yorg=yy; /*  */

                    } else {    /* Lines, Icons or text */
                     
                        xx += xorg;                /* Adds the origin offset to the object coords */
                        yy += yorg;
    
                        /* map the (first object location)  coordinate to the screen */
                        xloc = xx / 100.0;
                        yloc = -yy / 100.0;
                        km_to_pixel(&(gd.h_win.margin),xloc,yloc,&xpix,&ypix);
                        x1 = xpix;
                        y1 = ypix;
                    }

                    text_done=0;
                    pen_up = 1;
                    if(func == DRAW_ICON) pen_up = 1;
                    if(func == DRAW_LINES && ipt[1] != PEN_UP) pen_up = 0;
                    read_h=0;
        
                    ipt++;                            /* move to the first x,y  point */
                    if(ipt >= ept) break;
    
                         if(*ipt== END_OF_DATA) break;
                    ipt++;                            /* move past the first x,y  point */
                    if(ipt >= ept) break;
                    continue;
                    }

                 if(tmp== END_SESSION){
                        read_h=1;
                        ipt++;
                        if(ipt >= ept) break;
                        continue;                    /* to next object */
                    }

                    if(func== DRAW_TEXT){
                        char *str;
                        int max_len,len,xs,ys;


                        if(text_done==1){ /* skip bytes left after string */
                               ipt++;
                            if(ipt >= ept) break;
                            continue;
                        }

                        str=(char *)ipt;        /* cast object to char * */
                        max_len=(ept-ipt)<<2;

                        len=strlen(str);
                        if(len >= max_len) len = max_len -1;


                        /* Center the string */
                        xs=xpix;        /* subtract half the string width */
                        ys=ypix;        /* add half the height */

                        XDrawImageString(gd.dpy,xid,gc,xs,ys,str,len);
                        len++;    /* now count in the null terminator */

                        ipt += (len>>2);
                        if(len % 4) ipt++;
                         
                        if(ipt >= ept) break;
                        text_done=1;             /* only one string per session */
                        continue;
                        }

                   if(func== DRAW_LINES || func == DRAW_ICON) {
                        /* graphics */
                         if(tmp == PEN_UP) {        /* Process pen up command */
                            pen_up=1;
                            ipt++;
                            if(ipt >= ept) break;
                            continue;
                       }
    
                       if(func == DRAW_ICON){            /* switch on the coordinate type */
                             if(pen_up == 0){            
                                   x2=xpix+((tmp<<16)>>16);    /* Screen coordinates (ICONS) */
                                y2=ypix+(tmp>>16);
                                XDrawLine(gd.dpy,xid,gc,x1,y1,x2,y2);
                                x1 = x2;
                                y1 = y2;
                          } else {
                                   x1=xpix+((tmp<<16)>>16);    /* Screen coordinates (ICONS) */
                                y1=ypix+(tmp>>16);
                                pen_up = 0;
                          }

                       } else{                        /* DRAW_LINES function */
                             if(pen_up == 0){                /* Draw a line segment */
                                  x2=xorg+((tmp<<16)>>16);
                                        y2=yorg+(tmp>>16);
    
                                xloc = x2 / 100.0;    /* convert to screen coords */
                                yloc = -y2 / 100.0;
                                km_to_pixel(&(gd.h_win.margin),xloc,yloc,&x2,&y2);
                                XDrawLine(gd.dpy,xid,gc,x1,y1,x2,y2);
                                x1 = x2;
                                y1 = y2;
                          } else {
                                  x1=xorg+((tmp<<16)>>16);
                                        y1=yorg+(tmp>>16);
    
                                xloc = x1 / 100.0;    /* convert to screen coords */
                                yloc = -y1 / 100.0;
                                km_to_pixel(&(gd.h_win.margin),xloc,yloc,&x1,&y1);
                                pen_up = 0;
                          }
                     }
                }
    
                ipt++;    /* move to the next point */
                if(ipt >= ept) break;
            }
        }
    }

    /* indicate that we have rendered the images */
    omsg->newly_updated = 0;
    
    lock_shm(0);    /* Unlock the shared memory segment */
    return (0);

}

/**********************************************************************
 * LOCK_SHM: wait until shm is accessible and set busy
 */

int lock_shm(sw)
int sw;
{
    static struct sembuf op_lock[2] = {
        0,0,0,   /* wait for sem#0 to become 0 */
        0,1,SEM_UNDO    /* increment sem */
    };
    static struct sembuf op_unlock[1] = {
        0,-1,(IPC_NOWAIT | SEM_UNDO)  /* decrement sem */
    };
    static int semid=-1;
    extern int errno;

    if(semid<0){
        if((semid=semget(SHM_KEY,1,IPC_CREAT | 0666))<0){
            if(gd.debug1) printf("Error in semget.\n");
            return (1);
        }
    }

    if(sw==1){ /* lock */
again:
        if(semop(semid,&op_lock[0],2)<0){
            if(errno==4) goto again;
            if(gd.debug1) printf("Error in lock shm (errno: %d).\n",errno);
            return (1);
        }
        return (0);
    }

    if(sw==0){ /* unlock */
        if(semop(semid,&op_unlock[0],1)<0){
            if(gd.debug1) printf("shm. not locked\n");
            return (1);
        }
        return (0);
    }
    return(0);
}

/**********************************************************************
 * SET_IMPORT_TIMES - Tell PRDS the time range for the products to import
 */

int set_import_times(start_time, end_time)

long start_time;
long end_time;

{

    /*
     * check to see if PRDS is currently running
     */

    if (omsg->key != VALID)
        return(1);
    
    /*
     * put the message information in shared memory
     */

    STRcopy(imsg->msg, PRDS_CHANGE_TIME_CMD,20);
    imsg->start_time = start_time;
    imsg->end_time = end_time;
    
    /*
     * signal PRDS that we have a new command
     */

    if (gd.debug1)
        printf("Sending signal SIGUSR1 to %d\n", omsg->pid);
    kill(omsg->pid, SIGUSR1);
    

    return(0);

} /* end of set_import_times */

/**********************************************************************
 * DELETE_PRDS_SHMEM - Detach from the PRDS shared memory segment
 */

void delete_prds_shmem(void)
{
  if (shmpt != NULL)
  {
    if (shmdt((char *)shmpt) < 0)
      fprintf(stderr, "Error detaching from prds shared memory segment\n");

    if (ushm_nattach(XRSgetLong(gd.cidd_db, "cidd.prds_key", 6713)) <= 0)
      ushm_remove(XRSgetLong(gd.cidd_db, "cidd.prds_key", 6713));
  }
}
