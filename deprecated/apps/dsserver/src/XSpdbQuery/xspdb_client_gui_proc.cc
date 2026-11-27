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
/*****************************************************************************
 * XSPDB_CLIENT_GUI_PROC.c - Notify and event callback functions
 * Note :set tabstop=4
 */

#include "XSpdbQuery.h"


#include <fstream>
#include <Spdb/DsSpdb.hh>
#include "Print.hh"
#include <toolsa/udatetime.h>
#include <toolsa/utim.h>
using namespace std;

void update_controls(void);
void display_chunks(DsSpdb &spdb);


/*************************************************************************
 * Notify callback function for `del_bt'.
 */
void
delete_proc(Panel_item item, Event *event)
{
    DsSpdb spdb;
    char str[128];
    struct tm *tm;

    spdb.erase(gd.source_string,gd.valid_time,gd.data_type,gd.data_type2);

    sprintf(str,"ERASE from %s, Data_type: %d, Data_type2: %d\n",
	    gd.source_string,gd.data_type,gd.data_type2);
    textsw_insert(gd.win->textpane1,str,strlen(str));

    tm = gmtime(&gd.valid_time);
    strftime(str,128," at %H:%M:%S %m/%d/%Y\n",tm);
    textsw_insert(gd.win->textpane1,str,strlen(str));
}

/*****************************************************************************
 * Notify callback function for `now_bt'.
 */
void
set_now_proc( Panel_item  item, Event       *event)
{
    gd.end_time = time(0);
    gd.begin_time =  gd.end_time - gd.delta_time;

    update_controls();
} 

/*****************************************************************************
 * Notify callback function for `clear_bt'.
 */
void
clear_text_proc(Panel_item  item, Event       *event)
{
    textsw_erase(gd.win->textpane1,0,TEXTSW_INFINITY);
}
   

/*****************************************************************************
 * Notify callback function for `end_tx'.
 */
Panel_setting
end_tm_proc(Panel_item  item, Event       *event)
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);

    gd.end_time = UTIMstring_US_to_time(value);
    gd.begin_time = gd.end_time - gd.delta_time;
    
    update_controls();
    
    return panel_text_notify(item, event);
}


/*****************************************************************************
 * Notify callback function for `time_delta_ntx'.
 */
Panel_setting
time_delta_proc( Panel_item  item, Event       *event)
{
    int    value = (int) xv_get(item, PANEL_VALUE);
    
    gd.begin_time = gd.end_time - (value * 60);
    gd.delta_time = value * 60;

    return panel_text_notify(item, event);
}

/*****************************************************************************
 * Notify callback function for `source_tx'.
 */
Panel_setting
source_string_proc(Panel_item  item, Event       *event)
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    
    STRcopy(gd.source_string,value,1024);
     
    return panel_text_notify(item, event);
}
/*****************************************************************************
 * Notify callback function for `req_typ_st'.
 */
void
reqest_type_proc( Panel_item item, int value, Event *event)
{
    gd.request_type = value;
    update_controls();
}


/*****************************************************************************
 * Notify callback function for `prod_typ_st'.
 */
void
prod_typ_proc(Panel_item item, int value, Event *event)
{
    gd.product_type = Prod[value].product_type;
    update_controls();
    
}
/*****************************************************************************
 * Notify callback function for `prod_id_tx'.
 */
Panel_setting
prod_id_proc(Panel_item  item, Event       *event)
{
    char *    value = (char *) xv_get(item, PANEL_VALUE);
    
	if(sscanf(value,"%d",&gd.data_type) != 1) {
		   if(strlen(value) == 4) {
             gd.data_type = Spdb::hash4CharsToInt32(value);
		   } else {
             gd.data_type = Spdb::hash5CharsToInt32(value);
		   }
	}

    update_controls();
    
    return panel_text_notify(item, event);
}

/*************************************************************************
 * Notify callback function for `data2_tx'.
 */
Panel_setting
datatype2_proc(Panel_item item, Event *event)
{
        char *  value = (char *) xv_get(item, PANEL_VALUE);

	if(sscanf(value,"%d",&gd.data_type2) != 1) {
           gd.data_type2 = Spdb::hash4CharsToInt32(value);
	}

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
    char *inst_txt;
    char buf[64];

    xv_set(gd.win->source_tx,PANEL_VALUE,gd.source_string,NULL);
     
    sprintf(buf,"%d",gd.data_type);
    xv_set(gd.win->prod_id_tx,PANEL_VALUE,buf,NULL);
     
    sprintf(buf,"%d",gd.data_type2);
    xv_set(gd.win->data2_tx,PANEL_VALUE,buf,NULL);

    e_time = gmtime(&gd.end_time);
    strftime(e_time_str,64,"%H:%M:%S %m/%d/%Y\n",e_time);
    xv_set(gd.win->end_tx,PANEL_VALUE,e_time_str,NULL);

    xv_set(gd.win->time_delta_ntx,PANEL_VALUE,
        (int)((gd.end_time - gd.begin_time) / 60),
        NULL);

    switch(gd.request_type) {
	case 0:
	     inst_txt = "Returns products which have Exact Time Matches";
             xv_set(gd.win->time_delta_ntx,PANEL_LABEL_STRING,"N/A",NULL);
	break;

	case 1:
	     inst_txt = "Returns products Closest to Interest Time, within N minutes";
             xv_set(gd.win->time_delta_ntx,PANEL_LABEL_STRING,"Within",NULL);
	break;

	case 2:
	     inst_txt = "Returns products Before Interest Time within N minutes";
             xv_set(gd.win->time_delta_ntx,PANEL_LABEL_STRING,"Within",NULL);
	break;

	case 3:
	     inst_txt = "Get Products valid at Interest time";
             xv_set(gd.win->time_delta_ntx,PANEL_LABEL_STRING,"N/A",NULL);
	break;

	case 4:
	     inst_txt = "Get Latest Interval of Data Within N minutes Latest Data Time";
             xv_set(gd.win->time_delta_ntx,PANEL_LABEL_STRING,"Within",NULL);
	break;

	case 5:
	     inst_txt = "Get the First product before Interest time, within N minutes";
             xv_set(gd.win->time_delta_ntx,PANEL_LABEL_STRING,"Within",NULL);
	break;

	case 6:
	     inst_txt = "Get The First product After the Interest time, within N minutes";
             xv_set(gd.win->time_delta_ntx,PANEL_LABEL_STRING,"Within",NULL);
	break;

	case 7:
	     inst_txt = "Returns products which have Exact Time Matches";
             xv_set(gd.win->time_delta_ntx,PANEL_LABEL_STRING,"N/A",NULL);
	break;

    }

     xv_set(gd.win->Instruct_msg,PANEL_LABEL_STRING,inst_txt,NULL);

}


/*****************************************************************************
 * SUBMIT_PROC
 * Notify callback function for `submit_bt'.
 */
void
submit_proc(Panel_item  item, Event       *event)
{
    char buf[1024];
    DsSpdb spdb;
    struct tm *tm;
    time_t tim;
    time_t first_tim;
    time_t last_tim;
    time_t last_valid_tim;
    char tm_str[128];
     
    spdb.setProdId(gd.product_type);

    switch(gd.request_type){
      default :
      case 0: /* Exact */
          tm = gmtime(&gd.end_time);
          strftime(tm_str,128,"getExact at %H:%M:%S %m/%d/%Y\n",tm);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

          if (spdb.getExact(gd.source_string,
                    gd.end_time,
                    gd.data_type,
                    gd.data_type2)) 
           { 
             if(gd.debug) fprintf(stderr,
                  "Spdb getExact Failure:\n %s\n",
                  spdb.getErrStr().c_str());
           }

      break;

      case 1: /* Closest */
          tm = gmtime(&gd.end_time);
          strftime(tm_str,128,"getClosest to %H:%M:%S %m/%d/%Y\n",tm);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

	  tim = gd.end_time - gd.delta_time;
          tm = gmtime(&tim);
          strftime(tm_str,128,"But after %H:%M:%S %m/%d/%Y\n",tm);

          if (spdb.getClosest(gd.source_string,
                    gd.end_time,
                    gd.delta_time,
                    gd.data_type,
                    gd.data_type2)) 
           { 
             if(gd.debug) fprintf(stderr,
                  "Spdb getClosest Failure:\n %s\n",
                  spdb.getErrStr().c_str());
           }

      break;

      case 2: /* Interval Before */
          tm = gmtime(&gd.begin_time);
          strftime(tm_str,128,"getInterval between %H:%M:%S %m/%d/%Y\n",tm);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

          tm = gmtime(&gd.end_time);
          strftime(tm_str,128,"       and          %H:%M:%S %m/%d/%Y\n",tm);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

          if (spdb.getInterval(gd.source_string,
                    gd.begin_time,
                    gd.end_time,
                    gd.data_type,
                    gd.data_type2)) 
           { 
             if(gd.debug) fprintf(stderr,
                  "Spdb getInterval Failure:\n %s\n",
                  spdb.getErrStr().c_str());
           }

      break;

      case 3: /* Valid at */
          tm = gmtime(&gd.end_time);
          strftime(tm_str,128,"getValid at %H:%M:%S %m/%d/%Y\n",tm);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

          if (spdb.getValid(gd.source_string,
                    gd.end_time,
                    gd.data_type,
                    gd.data_type2)) 
           { 
             if(gd.debug) fprintf(stderr,
                  "Spdb getValid Failure:\n %s\n",
                  spdb.getErrStr().c_str());
           }

      break;

      case 4: /* Latest */
	  sprintf(tm_str,"getLatest -  within %d seconds\n",(int)gd.delta_time);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

          if (spdb.getLatest(gd.source_string,
                    gd.delta_time,
                    gd.data_type,
                    gd.data_type2)) 
           { 
             if(gd.debug) fprintf(stderr,
                  "Spdb getLatest Failure:\n %s\n",
                  spdb.getErrStr().c_str());
           }

      break;

      case 5: /* First Before */
          tm = gmtime(&gd.end_time);
          strftime(tm_str,128,"getFirstBefore %H:%M:%S %m/%d/%Y\n",tm);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

	  tim = gd.end_time - gd.delta_time;
          tm = gmtime(&tim);
          strftime(tm_str,128,"But after %H:%M:%S %m/%d/%Y\n",tm);
	  sprintf(tm_str,"(Within %d seconds)\n",(int)gd.delta_time);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

          if (spdb.getFirstBefore(gd.source_string,
                    gd.end_time,
                    gd.delta_time,
                    gd.data_type,
                    gd.data_type2)) 
           { 
             if(gd.debug) fprintf(stderr,
                  "Spdb getFirstBefore Failure:\n %s\n",
                  spdb.getErrStr().c_str());
           }

      break;

      case 6: /* First After */
          tm = gmtime(&gd.end_time);
          strftime(tm_str,128,"getFirstAfter %H:%M:%S %m/%d/%Y\n",tm);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

	  tim = gd.end_time + gd.delta_time;
          tm = gmtime(&tim);
          strftime(tm_str,128,"But before %H:%M:%S %m/%d/%Y\n",tm);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

	  sprintf(tm_str,"(Within %d seconds)\n",(int)gd.delta_time);
          textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str));

          if (spdb.getFirstAfter(gd.source_string,
                    gd.end_time,
                    gd.delta_time,
                    gd.data_type,
                    gd.data_type2)) 
           { 
             if(gd.debug) fprintf(stderr,
                  "Spdb getFirstAfter Failure:\n %s\n",
                  spdb.getErrStr().c_str());
           }

      break;

      case 7: /* GET TIMES*/
	  sprintf(tm_str,"Getting Data Temporal Limits\n");
	  textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str)); 

          if (spdb.getTimes(gd.source_string,
                    first_tim,last_tim,last_valid_tim)) { 
             if(gd.debug) fprintf(stderr,
                  "Spdb getTimes Failure:\n %s\n",
                  spdb.getErrStr().c_str());
          }

	  tm = gmtime(&first_tim);
	  strftime(tm_str,128,"First Data Time %H:%M:%S %m/%d/%Y\n",tm);
	  textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str)); 

	  tm = gmtime(&last_tim);
	  strftime(tm_str,128,"Last Data Time %H:%M:%S %m/%d/%Y\n",tm);
	  textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str)); 

	  tm = gmtime(&last_valid_tim);
	  strftime(tm_str,128,"Last Valid Time %H:%M:%S %m/%d/%Y\n",tm);
	  textsw_insert(gd.win->textpane1,tm_str,strlen(tm_str)); 
      break;

    }

    if(gd.request_type > 7) return;  // No chunks to display

    gd.num_active_products = spdb.getNChunks();

    sprintf(buf, "Retrieved %d chunks from %s\n",
		gd.num_active_products, gd.source_string);
    textsw_insert(gd.win->textpane1,buf,strlen(buf));

    if(gd.num_active_products > 0) display_chunks(spdb);

    if(gd.num_active_products == 1 ) {  // Enable delete function
      xv_set(gd.win->del_bt,PANEL_INACTIVE,FALSE,NULL);
     } else {
        if(gd.dangerous_delete == 1) {
            xv_set(gd.win->del_bt,PANEL_INACTIVE,FALSE,NULL);
        } else {
          xv_set(gd.win->del_bt,PANEL_INACTIVE,TRUE,NULL);
        }
    } 
}

#define LOCAL_BUF_SIZE 4096
#define MAX_DISPLAY_PRODUCTS 500
/*****************************************************************************
 * DISPLAY_CHUNKS
 */

void display_chunks(DsSpdb &spdb)
{
    int i,data_len;
    int n_read;
    void *chunk_data; 

    FILE *t_file;
    char t_file_path[1024];
    char buf[LOCAL_BUF_SIZE];

    /* Open a temporary file for the output */
    sprintf(t_file_path,"/tmp/XSdbQuery%d.txt",getpid());
    
    ofstream ofs;
    ofs.open(t_file_path,ios::out);
    
    if((t_file =  fopen(t_file_path, "w")) == NULL) {
      fprintf(stderr," Problems opening temp file: %s\n", t_file_path);
      perror(t_file_path);
      return;
    }

    // Instantiate a print object
    Print print(t_file, ofs);
    
    /* Display A Visual separator */
        fprintf(t_file, "\n");
        fprintf(t_file, "######################################################################\n");
        fprintf(t_file, "######################################################################\n");
        fprintf(t_file, "######################################################################\n");
        fprintf(t_file, "URL: %s\n",gd.source_string);
        fprintf(t_file, "%s\n",spdb.getProdLabel().c_str());
        fprintf(t_file, "ID: %d,   N_Chunks: %d \n",spdb.getProdId(),spdb.getNChunks());
        fprintf(t_file, "######################################################################\n");
        fprintf(t_file, "######################################################################\n");
        fprintf(t_file, "######################################################################\n");
        fprintf(t_file, "\n");

    int start = 0;
    if(gd.num_active_products > MAX_DISPLAY_PRODUCTS) {
	start = gd.num_active_products - MAX_DISPLAY_PRODUCTS;
	fprintf(t_file,"\n\t\tNOTE: Displaying the last %d of %d objects\n",
		MAX_DISPLAY_PRODUCTS,gd.num_active_products);
    }

    const vector< Spdb::chunk_t > &chunks = spdb.getChunks();
    
    for (i = start; i < gd.num_active_products; i++) {

        /* Print the chunk Header info */
        fprintf(t_file, "Header for chunk ------------->  %d <--------------\n", i+1);
        print.chunk_hdr(chunks[i]);
        fprintf(t_file, "\n");

	gd.valid_time = chunks[i].valid_time;
        data_len = chunks[i].len;
        chunk_data = chunks[i].data;

	// Some broken data bases have the ID as 0 - Try data type2
	int p_id = spdb.getProdId();
	if(p_id == 0) p_id = chunks[i].data_type2;

        /* Print out the chunk */
        switch(p_id) {
          default:
              fprintf(t_file,"Unknown Product ID: %d\n",p_id);
              break;

           case SPDB_ASCII_ID :
			case SPDB_RAW_METAR_ID :
              print.ascii(data_len, chunk_data);
              break;

            case SPDB_GENERIC_POINT_ID :
              print.generic_pt(data_len, chunk_data);
              break;

			case SPDB_AC_VECTOR_ID :
			  print.ac_vector(data_len, chunk_data);
			  break;

            case SPDB_COMBO_POINT_ID :
              print.combo_pt(data_len, chunk_data);
              break;

            case SPDB_STATION_REPORT_ID :
              print.stn_report(chunk_data);
              break;

            case SPDB_TREC_PT_FORECAST_ID :
              print.history_forecast(data_len, chunk_data);
              break;

            case SPDB_SYMPROD_ID :
              print.symprod(data_len, chunk_data);
              break;

            case SPDB_SIGMET_ID :
              print.sigmet(chunk_data);
              break;

			case SPDB_SIGAIRMET_ID :
	          print.sigAirMet(data_len, chunk_data);
		      break;

            case SPDB_BDRY_ID :
              print.bdry(data_len, chunk_data);
              break;

            case SPDB_ACARS_ID :
              print.acars(data_len, chunk_data);
              break;
             case SPDB_AC_POSN_ID :
              if (chunks[i].data_type2 ==  SPDB_AC_POSN_WMOD_ID) {
                print.ac_posn_wmod(data_len, chunk_data);
              } else {
                print.ac_posn(data_len, chunk_data);
              }
              break;

            case SPDB_AC_POSN_WMOD_ID :
              print.ac_posn_wmod(data_len, chunk_data);
              break;

            case SPDB_AC_DATA_ID :
              print.ac_data(chunk_data);
              break;

            case SPDB_FLT_PATH_ID :
              print.flt_path(chunk_data);
              break;

            case SPDB_LTG_ID :
              print.ltg(data_len, chunk_data);
              break;

            case SPDB_LTG_GROUP_ID :
              print.ltg_group(data_len, chunk_data);
              break;

            case SPDB_TSTORMS_ID :
              print.tstorms(chunk_data);
              break;

            case SPDB_TREC_GAUGE_ID :
              print.trec_gauge(data_len, chunk_data);
              break;

            case SPDB_ZR_PARAMS_ID :
              print.zr_params(chunk_data);
              break;

            case SPDB_ZRPF_ID :
              print.zrpf(data_len, chunk_data);
              break;

			case SPDB_ZVPF_ID :
			  print.zvpf(data_len, chunk_data);
			  break;


            case SPDB_VERGRID_REGION_ID :
              print.vergrid_region(data_len, chunk_data);
              break;

            case SPDB_SNDG_ID:
              print.sndg(chunk_data);
              break;
          
            case SPDB_SNDG_PLUS_ID:
	      print.sndg_plus(data_len, chunk_data);
              break;

            case SPDB_EDR_POINT_ID:
              print.edr_point(data_len, chunk_data);
              break;

            case SPDB_PIREP_ID:
              print.pirep(data_len, chunk_data);
              break;

			case SPDB_POSN_RPT_ID:
			  print.posn_rpt (chunk_data);
			  break;
         
            case SPDB_GENERIC_POLYLINE_ID:
              print.gen_poly(data_len, chunk_data);
              break;

			case SPDB_DS_RADAR_SWEEP_ID:
	          print.ds_radar_sweep(data_len, chunk_data);
		      break;
         
            case SPDB_NWS_WWA_ID:
              print.nws_wwa(data_len, chunk_data);
              break;

	case SPDB_WAFS_SIGWX_ID :
	case SPDB_WAFS_SIGWX_CLOUD_ID :
	case SPDB_WAFS_SIGWX_JETSTREAM_ID :
	case SPDB_WAFS_SIGWX_TROPOPAUSE_ID :
	case SPDB_WAFS_SIGWX_TURBULENCE_ID :
	  print.wafs_sigwx(data_len, chunk_data);
	  break;
	  
        }

	fflush(t_file);
	ofs.flush();
    }

    fclose(t_file);
    ofs.close();

    if((t_file =  fopen(t_file_path,"r")) == NULL) {
      perror("XSpdbQuery:");
      fprintf(stderr," Problems opening: %s\n",t_file_path);
      return;
    }

    /* Load the file contents into the text sub-window */
    while((n_read = fread(buf,1,LOCAL_BUF_SIZE,t_file)) > 0) {
      textsw_insert(gd.win->textpane1,buf,n_read);
    }

    unlink(t_file_path);
}
