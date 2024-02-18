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
/************************************************************************
 * GUI_LABEL_FRAME.CC:  Change the Frame Labels on the GUI 
 */

#define GUI_LABEL_FRAME 1

#include "cidd.h"
#include "cidd_funcs.h"

/************************************************************************
 * GUI_LABEL_H_FRAME : Display a message in the title bar of the horiz
 *  `base window
 */

void gui_label_h_frame(const char    *str,int persistance)
{
    static time_t ok_time = 0;
    time_t now;

    now = time(0);
     
    if(now  >= ok_time || persistance < 0) {
        // xv_set(gd.h_win_horiz_bw->horiz_bw,
        //        XV_LABEL, str,
        //        NULL);
	    ok_time = now + abs(persistance);
    }
}

/************************************************************************
 * GUI_LABEL_V_FRAME : Display a message in the title bar of the Vert
 *   base window
 */

void gui_label_v_frame(const char *str)
{
    // xv_set(gd.h_win_horiz_bw->horiz_bw,
    //        XV_LABEL, str,
    //        NULL);
}


/**********************************************************************
 * SET_BUSY_STATE: Provide visual feedback that the application is busy
 *
 */

void set_busy_state(int state)   /* 0 = not busy, 1 = busy */
{
    static int last_state = -1;

    if(state == last_state) return;
    last_state = state;

    if (state) {

      // xv_set(gd.h_win_horiz_bw->horiz_bw,
      //          FRAME_BUSY, TRUE,
      //          NULL);
      //   xv_set(gd.v_win_v_win_pu->v_win_pu,
      //          FRAME_BUSY, TRUE,
      //          NULL);
      //   xv_set(gd.page_pu->page_pu,
      //          FRAME_BUSY, TRUE,
      //          NULL);

    } else {
        // xv_set(gd.h_win_horiz_bw->horiz_bw,
        //        FRAME_BUSY, FALSE,
        //        NULL);
        // xv_set(gd.v_win_v_win_pu->v_win_pu,
        //        FRAME_BUSY, FALSE,
        //        NULL);
        // xv_set(gd.page_pu->page_pu,
        //        FRAME_BUSY, FALSE,
        //        NULL);
    }
}

/************************************************************************
 * UPDATE_TICKER: Update the current time ticker if mapped - otherwise
 *   Make sure the correct panels are mapped
 */

void update_ticker(time_t cur_time)
{
    struct tm *loc_tm;
    char time_string[64];
    char fmt_string[64];
    struct stat s;
        FILE *sfile;

    if(gd.run_unmapped) {
                // xv_set(gd.h_win_horiz_bw->horiz_bw,XV_SHOW,FALSE,NULL);
    } else {
	   sprintf(fmt_string,"Current Time: %s",_params.label_time_format);

       struct tm res;
	   if(_params.use_local_timestamps)  {
          loc_tm = localtime_r(&cur_time,&res);
	   } else {
          loc_tm = gmtime_r(&cur_time,&res);
	   }

       strftime(time_string,64,fmt_string,loc_tm);
       // xv_set(gd.h_win_horiz_bw->cur_time_msg,
       //             PANEL_LABEL_STRING,time_string,NULL);
     }

     /* Update the NO DATA Status messages if the file was defined */
     if(gd.status.status_fname != NULL) { /* If its defined */

            /* And it exists and is a reasonable size */
            if(stat(gd.status.status_fname,&s) == 0 && s.st_size > 3) {

               /* And has been updated  */
               if(s.st_mtime > gd.status.last_accessed) {

                 if((sfile = fopen(gd.status.status_fname,"r")) != NULL) { /* And can be read */

                      /* Clear out the old one */
                       memset((void *) gd.status.stat_msg,0,TITLE_LENGTH);
                       /* Get the first line of the file */
                       fgets(gd.status.stat_msg,TITLE_LENGTH,sfile);
                       fclose(sfile);
                       _params.no_data_message = gd.status.stat_msg;
                       gd.status.last_accessed = s.st_mtime;
		       gd.status.is_dynamic = 1;
                 }
              }
          } else {  // Status file disappeared
#ifdef NOTNOW
	     // Replace the No Data Message with the original
	     if(gd.status.is_dynamic) {
		 _params.no_data_message = gd.uparams->getString(
		     "cidd.no_data_message",
		     "NO DATA FOUND (in this area at the selected time)");
	     }
#endif
	  }
     }
}  
