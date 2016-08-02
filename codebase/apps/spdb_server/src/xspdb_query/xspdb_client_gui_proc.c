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
/*****************************************************************************
 * XSPDB_CLIENT_GUI_PROC.c - Notify and event callback functions
 * Note :set tabstop=4
 */

#include "xspdb_client.h"

void update_controls(void);
void display_chunks(ui32 n_chunks,
					spdb_chunk_ref_t *chunk_hdr,
					void *chunk_data);

/*****************************************************************************
 * Notify callback function for `now_bt'.
 */
void
set_now_proc(item, event)
    Panel_item  item;
    Event       *event;
{
	gd.end_time = time(0);
	gd.begin_time =  gd.end_time - gd.delta_time;

	update_controls();
} 

/*****************************************************************************
 * Notify callback function for `clear_bt'.
 */
void
clear_text_proc(item, event)
    Panel_item  item;
    Event       *event;
{
	textsw_erase(gd.win->textpane1,0,TEXTSW_INFINITY);
}
   

/*****************************************************************************
 * Notify callback function for `end_tx'.
 */
Panel_setting
end_tm_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	char *	value = (char *) xv_get(item, PANEL_VALUE);

	gd.end_time = UTIMstring_US_to_time(value);
	gd.begin_time = gd.end_time - gd.delta_time;
	
	update_controls();
	
	return panel_text_notify(item, event);
}


/*****************************************************************************
 * Notify callback function for `time_delta_ntx'.
 */
Panel_setting
time_delta_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	int	value = (int) xv_get(item, PANEL_VALUE);
	
	gd.begin_time = gd.end_time - (value * 60);
	gd.delta_time = value * 60;

	return panel_text_notify(item, event);
}

/*****************************************************************************
 * Notify callback function for `source_tx'.
 */
Panel_setting
source_string_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	char *	value = (char *) xv_get(item, PANEL_VALUE);
	
	STRcopy(gd.source_string,value,1024);
	 
	return panel_text_notify(item, event);
}
/*****************************************************************************
 * Notify callback function for `req_typ_st'.
 */
void
reqest_type_proc(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
    gd.request_type = value;
}


/*****************************************************************************
 * Notify callback function for `prod_typ_st'.
 */
void
prod_typ_proc(item, value, event)
	Panel_item	item;
	int		value;
	Event		*event;
{
	gd.product_type = 0;
	//gd.product_type = Prod[value].product_type;
	update_controls();
	
}
/*****************************************************************************
 * Notify callback function for `prod_id_tx'.
 */
Panel_setting
prod_id_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	char *	value = (char *) xv_get(item, PANEL_VALUE);
	
	gd.product_id = atoi(value);

	update_controls();
	
	return panel_text_notify(item, event);
}

/*****************************************************************************
 * UPDATE_CONTROLS
 */

void update_controls()
{
    struct tm *e_time;
	char e_time_str[64];
	char buf[64];

	xv_set(gd.win->source_tx,PANEL_VALUE,gd.source_string,NULL);
	 
	sprintf(buf,"%d",gd.product_id);
    xv_set(gd.win->prod_id_tx,PANEL_VALUE,buf,NULL);

	e_time = gmtime(&gd.end_time);
	strftime(e_time_str,64,"%H:%M %m/%d/%Y",e_time);
	xv_set(gd.win->end_tx,PANEL_VALUE,e_time_str,NULL);

	xv_set(gd.win->time_delta_ntx,PANEL_VALUE,
		(int)((gd.end_time - gd.begin_time) / 60),
		NULL);
}


/*****************************************************************************
 * SUBMIT_PROC
 * Notify callback function for `submit_bt'.
 */
void
submit_proc(item, event)
	Panel_item	item;
	Event		*event;
{
	ui32 n_chunks;
	spdb_chunk_ref_t *chunk_hdr;
	void *chunk_data;
	 
	switch(gd.request_type){
	  default :
	  case 0: /* Interval */
		  if (SPDB_get_interval(gd.source_string,
				 gd.product_type,
				 gd.product_id,
				 gd.begin_time +(gd.delta_time/2),
				 gd.end_time + (gd.delta_time/2),
				 &n_chunks,
				 &chunk_hdr,
				 &chunk_data))
		
		   { /* Error condition */ 
			 if(gd.debug) fprintf(stderr,
				  "SPDB GET Interval Failure\n");
		   }

	  break;

	  case 1: /* Closest */
		  if (SPDB_get_closest(gd.source_string,
				 gd.product_type,
				 gd.product_id,
				 gd.end_time,
				 gd.delta_time,
				 &n_chunks,
				 &chunk_hdr,
				 &chunk_data))
		
		   { /* Error condition */ 
			 if(gd.debug) fprintf(stderr,
				  "SPDB GET Closest Failure\n");
		   }
				 
		   
	  break;

	  case 2: /* First Before */
		  if (SPDB_get_first_before(gd.source_string,
				 gd.product_type,
				 gd.product_id,
				 gd.end_time,
				 gd.delta_time,
				 &n_chunks,
				 &chunk_hdr,
				 &chunk_data))
		
		   { /* Error condition */ 
			 if(gd.debug) fprintf(stderr,
				  "SPDB GET First Before Failure\n");
		   }
				 
	  break;

	  case 3: /* First After */
		  if (SPDB_get_first_after(gd.source_string,
				 gd.product_type,
				 gd.product_id,
				 gd.end_time,
				 gd.delta_time,
				 &n_chunks,
				 &chunk_hdr,
				 &chunk_data))
		
		   { /* Error condition */ 
			 if(gd.debug) fprintf(stderr,
				  "SPDB GET First After Failure\n");
		   }
				 
	  break;

	}

	if(gd.debug) fprintf(stderr,
		"Retrieved %d chunks\n",n_chunks);

	if(n_chunks > 0) display_chunks(n_chunks,chunk_hdr,chunk_data);
	
}

#define LOCAL_BUF_SIZE 4096
/*****************************************************************************
 * DISPLAY_CHUNKS
 */

void display_chunks(ui32 n_chunks,
					spdb_chunk_ref_t *chunk_hdr,
					void *chunk_data)
{
    int i;
	int n_read;
	void *chunk_data_ptr; 
	FILE *t_file;
	char t_file_path[1024];
	char *buf[LOCAL_BUF_SIZE];

	/* Open a temporary file for the output */
	sprintf(t_file_path,"/tmp/xspdb_client_%d.txt",getpid());
	if((t_file = fopen(t_file_path,"w+")) == NULL) {
		fprintf(stderr," Problems opening: %s\n",t_file_path);
		return;
	}

	/* Display A Visual separator */
        fprintf(t_file, "\n\n");
        fprintf(t_file, "##################################################\n");
        fprintf(t_file, "##################################################\n");
        fprintf(t_file, "##################################################\n");
        fprintf(t_file, "##################################################\n");
        fprintf(t_file, "\n\n");

	for (i = 0; i < n_chunks; i++) {

		/* Print the chunk Header info */
		fprintf(t_file, "Header for chunk %d:\n", i);
		print_chunk_hdr(t_file, &chunk_hdr[i]);
		fprintf(t_file, "\n");

		chunk_data_ptr = (void *)((char *)chunk_data + chunk_hdr[i].offset);

		/* Print out the chunk */
		switch(chunk_hdr[i].prod_id) {
		  default:
		      fprintf(t_file,"Unknown Product ID: %d\n",chunk_hdr[i].prod_id);
		  break;
		   
		  case SPDB_STATION_REPORT_ID :
			  print_stn_report(t_file, &chunk_hdr[i], chunk_data_ptr);
		  break;

          case SPDB_SYMPROD_ID :
             print_symprod_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_SIGMET_ID :
             print_sigmet_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_BDRY_ID :
            print_bdry_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_AC_POSN_ID :
            print_ac_posn_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_FLT_PATH_ID :
            print_flt_path_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_KAV_LTG_ID :
            print_kav_ltg_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_TSTORMS_ID :
            print_tstorms_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_TREC_GAUGE_ID :
            print_trec_gauge_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_ZR_PARAMS_ID :
            print_zr_params_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_ZRPF_ID :
            print_zrpf_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_SNDG_ID :
            print_sounding_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_RAW_METAR_ID :
          case SPDB_ASCII_ID :
            print_ascii_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

          case SPDB_PIREP_ID :
            print_pirep_data(t_file, &chunk_hdr[i], chunk_data_ptr);
          break;

		}
	}

	/* Load the file contents into the text sub-window */
	rewind(t_file);

	while((n_read = fread(buf,1,LOCAL_BUF_SIZE,t_file)) > 0) {
		textsw_insert(gd.win->textpane1,buf,n_read);
	}

	fclose(t_file);
	unlink(t_file_path);
}
