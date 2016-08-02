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
///////////////////////////////////////////////////////////////
// SoundingChart_data.cc
//
// SoundingChart object
//
// Niles Oien  RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2004
//
///////////////////////////////////////////////////////////////
#define SoundingChart_data_CC

using namespace std;

#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>


#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/ushmem.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <physics/thermo.h>
#include <rapmath/math_macros.h>

#include <Spdb/Spdb.hh>
#include "SoundingChart.hh"

#include "soundingMgr.hh"
#include "mdvSoundingMgr.hh"


//////////////////////////////////////////////////
// GATHER_SOUNDING_DATA
void SoundingChart::gather_sounding_data(void)
{
  int id;
  string idstr;
  xv_set(P->SoundingChart_GUI->window1,FRAME_BUSY,TRUE,NULL);
  xv_set(P->SoundingChart_GUI->window1,FRAME_LABEL,"Gathering Data",NULL);
  P->update_time_msg((char*) "Gathering Sounding Data");
  notify_dispatch();
  XFlush(P->dpy);

  if (P->cur_sound_field < P->params.soundingSrc_n){
    //
    // Find the sounding closest to the CIDD click.
    //
    if(P->params.useStationLocator) {
	idstr = P->station_loc->FindClosest(P->interest_lat, P->interest_lon,P->params.max_station_dist_km);

       id = Spdb::hash4CharsToInt32(idstr.c_str());
    } else { 
       idstr = "none";
       id = P->params.station_id;
    }
    if (P->params.debug){
      cerr << "Searching for Sounding  from URL: "; 
      cerr << P->params._soundingSrc[P->cur_sound_field].url << " between ";
      cerr << utimstr(P->time_start) << " and ";
      cerr << utimstr(P->time_end) << endl;
      cerr << "Closest sounding to " << P->interest_lat << ", " << P->interest_lon;
      cerr << " ID " << idstr << ", " << id;
      cerr <<  endl;
    }
    
	// Delete Old Data 
	if(P->SG != NULL) {
		delete P->SG;
		P->SG = NULL;
	}

    //
    // Gather Sounding data
    //
        P->SG = new soundingMgr
          (params,
           P->time_start,P->time_end,
           (char *) P->params._soundingSrc[P->cur_sound_field].url, id,
           (char *) P->params._soundingSrc[P->cur_sound_field].label);
    
  }

  xv_set(P->SoundingChart_GUI->window1,FRAME_LABEL,P->params.window_title,NULL);
  xv_set(P->SoundingChart_GUI->window1,FRAME_BUSY,FALSE,NULL);
}

//////////////////////////////////////////////////
// GATHER_MDV_DATA
void SoundingChart::gather_mdv_data(void)
{
  xv_set(P->SoundingChart_GUI->window1,FRAME_BUSY,TRUE,NULL);
  xv_set(P->SoundingChart_GUI->window1,FRAME_LABEL,"Gathering Data",NULL);
  P->update_time_msg((char *) "Gathering MODEL Data");
  notify_dispatch();
  XFlush(P->dpy);

  //
  //  MDV pseudo-soundings.
  //
  if (P->cur_grid_field < P->params.GridSrc_n){
    
	// Delete Old Data 
	if(P->M != NULL) {
		delete P->M;
		P->M = NULL;
	}

    //
    // Gather data
    //
    vector <string> fieldNames;
    fieldNames.push_back(P->params._GridSrc[P->cur_grid_field].fieldname);
    fieldNames.push_back(P->params._GridSrc[P->cur_grid_field].u_fname);
    fieldNames.push_back(P->params._GridSrc[P->cur_grid_field].v_fname);

    P->M = new mdvSoundingMgr(P->time_start,P->time_end,
		     P->params._GridSrc[P->cur_grid_field].url,
		     P->interest_lat, P->interest_lon, fieldNames);
    
  }

  xv_set(P->SoundingChart_GUI->window1,FRAME_LABEL,P->params.window_title,NULL);
  xv_set(P->SoundingChart_GUI->window1,FRAME_BUSY,FALSE,NULL);
}

//////////////////////////////////////////////////
// GATHER_DATA
void SoundingChart::gather_data(void)
{
  static time_t  last_time = 0;
  static double  last_lat = 0.0;
  static double  last_lon = 0.0;
  static int  last_sfield = -1;
  static int  last_gfield = -1;

  // Avoid unnecessary data retrievals.
  if( P->interest_lon == last_lon &&
	  P->interest_lat == last_lat &&
	  P->cur_sound_field == last_sfield &&
	  P->cur_grid_field == last_gfield &&
	  P->interest_time == last_time) return;


  // Output a Click Point Reference Marker 
  if(P->reference_points_on) {
	P->num_points++;
    genPtMgr GenPT(P->params.click_point_url,P->params.debug);
    GenPT.output_point(P->interest_lat,P->interest_lon,
                       P->interest_time,
                       P->params.click_point_label,
                       (char *) "n/a",
                       P->num_points);
    P->notify_cidd();
  }

  xv_set(P->SoundingChart_GUI->window1,FRAME_BUSY,TRUE,NULL);
  xv_set(P->SoundingChart_GUI->window1,FRAME_LABEL,"Gathering Data",NULL);
  P->update_time_msg((char *) "Gathering Data");
  notify_dispatch();
  XFlush(P->dpy);

  P->gather_sounding_data();
  P->gather_mdv_data();

  last_lat = P->interest_lat;
  last_lon = P->interest_lon;
  last_time = P->interest_time;

  last_gfield = P->cur_grid_field;
  last_sfield = P->cur_sound_field;

  struct tm tms;

  char   tlabel1[128];
  char   tlabel2[256];
  strftime(tlabel1,128,"%H:%M %Y-%m-%d",gmtime_r(&P->time_start,&tms));
  strcat(tlabel1," to %H:%M %Y-%m-%d");
  strftime(tlabel2,256,tlabel1,gmtime_r(&P->time_end,&tms));


  xv_set(P->SoundingChart_GUI->window1,FRAME_LABEL,P->params.window_title,NULL);
  P->update_time_msg(tlabel2);

  return;
}

//////////////////////////////////////////////////
// NOTIFY_CIDD
//
void SoundingChart::notify_cidd(void)
{
	// Send a Client message to CIDD indicating New Data is available.

	int not_done = 5;
	while(not_done) {
		if(P->coordShmem->client_event == NO_MESSAGE) { // clear to go
		  strncpy(P->coordShmem->client_args,P->params.click_point_label,MAX_CLIENT_EVENT_ARG);
		  P->coordShmem->client_event = NEW_SPDB_AVAIL;
		  not_done = 0;
		} else { // Wait 50 miliseconds
		  uusleep(50000); // 50 miliseconds
		  not_done--;
		}
	}
}
