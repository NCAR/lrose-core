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
// SoundingChart.cc
//
// SoundingChart object
//
// Niles Oien  RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2004
//
///////////////////////////////////////////////////////////////
#define SoundingChart_CC

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
#include <rapmath/math_macros.h>

#include "SoundingChart.hh"

#include "soundingMgr.hh"
#include "mdvSoundingMgr.hh"
#include "mdvSounding.hh"

//////////////////////////////////////////////////
// Constructor
SoundingChart::SoundingChart(int argc, char **argv)
  
{

  // initialize

  paramsPath = NULL;
  coordShmem = NULL;
  station_loc = NULL;
  dpy = NULL;
  fontst = NULL;
  win_height = 0;
  win_width = 0;
  can_height = 0;
  can_width = 0;
  canvas_xid = 0;
  backing_xid = 0;
  def_gc = 0;
  font = 0;
  fg_cell = 0;
  bg_cell = 0;
  ref_cell = 0;
  grid_winds_cell = 0;
  sounding_winds_cell = 0;
  interest_time = 0;
  time_start = 0;
  time_end = 0;
  cur_grid_field = 0;
  cur_sound_field = 0;
  grid_winds_on = 0;
  grid_lines_on = 0;
  grid_color_on = 0;
  sound_winds_on = 0;
  sound_lines_on = 0;
  sound_color_on = 0;
  reference_points_on = 0;
  sounding_width_secs = 0;
  num_points = 0;
  interest_lat = 0;
  interest_lon = 0;
  interest_alt = 0;

  // set programe name

  progName = "SoundingChart";
  ucopyright((char *) progName.c_str());
  
  // get command line args

  if (args.parse(argc, argv, progName)) {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with command line args" << endl;
	exit(-1);
  }

  // get TDRP params
  
  paramsPath = (char *) "unknown";
  if (params.loadFromArgs(argc, argv, args.override.list,
			   &paramsPath)) {
    cerr << "ERROR: " << progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
	exit(-1);
  }

  // init process mapper registration

  PMU_auto_init((char *) progName.c_str(),
		params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // Attach to Cidd's shared memory segment
  
  if ((coordShmem = (coord_export_t *) ushm_create(params.coord_shmem_key,
				      sizeof(coord_export_t), 0666)) == NULL) {

    cerr << "ERROR: " << progName << endl;
    cerr << "  Cannot create/attach Cidd's coord export shmem segment." << endl;
	exit(-1);
  }

  backing_xid = 0;
  num_points = 0;
  SG = NULL;
  M = NULL;
  
  return;
}

//////////////////////////////////////////////////
// destructor

SoundingChart::~SoundingChart()
  
{

  if(P->SG != NULL) {
    delete P->SG;
    P->SG = NULL;
  }

  // detach from shared memory
  
  ushm_detach(P->coordShmem);
  
  // unregister process
  
  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// RUN

int SoundingChart::Run (int argc, char **argv)
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory
  //while (!P->coordShmem->shmem_ready && !P->params.no_wait_for_shmem) {
  //  PMU_auto_register("Waiting for shmem_ready flag");
  //  umsleep(P->params.sleep_msecs);
  //}

  // Init the Station Locator Service class.
  if(P->params.useStationLocator) {
     P->station_loc =  new StationLoc();
     if(P->station_loc->ReadData(P->params.StationLocatorURL) < 0) {
         cerr << "Can't load Station Data from " << P->params.StationLocatorURL << endl;
         exit(-1);
     }
  }

   // Initialize Xview
   xv_init(XV_INIT_ARGC_PTR_ARGV,
		&argc,argv,
		NULL);
  
  init_xview(); // Create all objects.

  // make changes to xview objects not available from DevGuide
  modify_xview_objects();


  // Load Grid Colorscales
  for(int i=0; i < P->params.GridSrc_n; i++) {
    ColorScale* cs  = new ColorScale(P->dpy,
                                     (char *) P->params.cscale_dir,
                                     (char *) P->params._GridSrc[i].cscale,
                                     P->params.debug);
    P->grid_cs.push_back(cs);
  }

  for(int i=0; i < P->params.soundingSrc_n; i++) {
	  ColorScale* cs  = new ColorScale(P->dpy,
			  P->params.cscale_dir,
			  P->params._soundingSrc[i].cscale,
			  (int)P->params.debug);

	  P->sound_cs.push_back(cs);
  }

  start_timer();

  // Turn control over to XView.

  xv_main_loop(P->SoundingChart_GUI->window1);
    
  return (0);

}

/*****************************************************************
 * BASE_WIN_DESTROY: Interposition for base frame destroys
 */

Notify_value  SoundingChart::base_win_destroy(Notify_client client, Destroy_status  status)
{

  switch(status) {
  case DESTROY_CLEANUP:

  case DESTROY_PROCESS_DEATH:
    PMU_unregister(P->progName.c_str(), P->params.instance);
    return notify_next_destroy_func(client,status);
    break;

  case DESTROY_CHECKING:
    return NOTIFY_DONE;
    break;

  case DESTROY_SAVE_YOURSELF:
    return NOTIFY_DONE;
    break;

  }
  return NOTIFY_DONE;
}

/***************************************************************************
 * INIT_XVIEW : Initialize the base frame and other global objects
 */

void SoundingChart::init_xview()
{

  P->SoundingChart_GUI =
    SoundingChart_GUI_window1_objects_initialize(NULL, (Xv_opaque) NULL);

  P->SoundingChart_GUI_config_pu =
    SoundingChart_GUI_config_pu_objects_initialize(NULL, (Xv_opaque) P->SoundingChart_GUI->window1);

  INSTANCE = xv_unique_key(); /* get keys for retrieving data */

  /* notify_interpose_destroy_func(P->SoundingChart_GUI->window1,
		 (enum Notify_value (*)(...))  base_win_destroy);
   */
  P->dpy = (Display *) xv_get(P->SoundingChart_GUI->window1,XV_DISPLAY);

}

/*****************************************************************
 * MODIFY_XVIEW_OBJECTS : Modify any Xview objects that couldn't
 *    be set up in Devguide. This is primarily to avoid manually
 *    changing any *ui.c file
 */

void SoundingChart::modify_xview_objects(void)
{
  Colormap cmap;
  XColor cell_def;
  XColor rgb_def;

  // Set window height
  xv_set(P->SoundingChart_GUI->window1,
         WIN_HEIGHT,P->params.window_height,
         WIN_WIDTH,P->params.window_width, NULL);

  // Get X Drawing resources.
  P->canvas_xid = xv_get(canvas_paint_window(P->SoundingChart_GUI->canvas1),XV_XID);

  cmap = DefaultColormap(P->dpy,DefaultScreen(P->dpy));
  P->def_gc = DefaultGC(P->dpy, DefaultScreen(P->dpy));

  // Allocate colors
  XAllocNamedColor(P->dpy,cmap, P->params.foreground_color, &cell_def,&rgb_def);
  P->fg_cell = cell_def.pixel;

  XAllocNamedColor(P->dpy,cmap, P->params.background_color, &cell_def,&rgb_def);
  P->bg_cell = cell_def.pixel;

  XAllocNamedColor(P->dpy,cmap, P->params.reference_color, &cell_def,&rgb_def);
  P->ref_cell = cell_def.pixel;

  XAllocNamedColor(P->dpy,cmap, P->params.grid_winds_color, &cell_def,&rgb_def);
  P->grid_winds_cell = cell_def.pixel;

  XAllocNamedColor(P->dpy,cmap, P->params.sound_winds_color, &cell_def,&rgb_def);
  P->sounding_winds_cell = cell_def.pixel;

  // Load Fonts.
  P->fontst = (XFontStruct *) XLoadQueryFont(P->dpy, P->params.font_name);
  if(P->fontst == NULL) {
    fprintf(stderr,"Can't load font %s\n", P->params.font_name);
    exit(-1);
  }
  P->font  = P->fontst->fid;
  XSetFont(P->dpy, P->def_gc, P->font);

  // Init the Grid data render features 
  int pvalue = 0;
  if(P->params.grid_color_on) {
	  pvalue |= 1;
	  P->grid_color_on = 1;
  }
  if(P->params.grid_lines_on) {
	  pvalue |= 2;
	  P->grid_lines_on = 1;
  }
  if(P->params.grid_winds_on) {
	  pvalue |= 4;
	  P->grid_winds_on = 1;
  }
  xv_set(P->SoundingChart_GUI_config_pu->grid_wind_st,PANEL_VALUE,pvalue,NULL);

  // Set width slider to 100%
  xv_set(P->SoundingChart_GUI->width_sl,PANEL_VALUE,100,NULL);
  P->sounding_width_secs = (int) P->params.sounding_width_minutes * 60; 

  // Init the Sounding data render features 
  pvalue = 0;
  if(P->params.sound_color_on) {
	  pvalue |= 1;
	  P->sound_color_on = 1;
  }
  if(P->params.sound_lines_on) {
	  pvalue |= 2;
	  P->sound_lines_on = 1;
  }
  if(P->params.sound_winds_on) {
	  pvalue |= 4;
	  P->sound_winds_on = 1;
  }
  xv_set(P->SoundingChart_GUI_config_pu->sound_wind_st,PANEL_VALUE,pvalue,NULL);

  // Fill in the Grid Data choice widget.
  for(int i=0; i < P->params.GridSrc_n; i++) {
	  xv_set(P->SoundingChart_GUI_config_pu->Grid_st,PANEL_CHOICE_STRING,i,P->params._GridSrc[i].label,NULL);
  }
  xv_set(P->SoundingChart_GUI_config_pu->Grid_st,PANEL_CHOICE_STRING,P->params.GridSrc_n,"None",NULL);

  // Fill in the Sounding Data choice widget.
  for(int i=0; i < P->params.soundingSrc_n; i++) {
	  xv_set(P->SoundingChart_GUI_config_pu->sound_st,PANEL_CHOICE_STRING,i,P->params._soundingSrc[i].label,NULL);
  }
  xv_set(P->SoundingChart_GUI_config_pu->sound_st,PANEL_CHOICE_STRING,P->params.soundingSrc_n,"None",NULL);


  if( P->params.output_ref_points) {
	  P->reference_points_on = 1;
	  xv_set(P->SoundingChart_GUI_config_pu->refs_st,PANEL_VALUE,1,NULL);
  } else {
	  P->reference_points_on = 0;
	  xv_set(P->SoundingChart_GUI_config_pu->refs_st,PANEL_VALUE,0,NULL);
  }
}

/*************************************************************************
 * UPDATE_TIME_MSG
 */
void SoundingChart::update_time_msg(char *message)
{

   xv_set(P->SoundingChart_GUI->time_msg,PANEL_LABEL_STRING,message,NULL);

   // Position messages on right edge.
   xv_set(P->SoundingChart_GUI->time_msg,XV_X,
			  P->win_width -  xv_get(P->SoundingChart_GUI->time_msg,XV_WIDTH) -10,
			 NULL);
}

/*************************************************************************
 * UPDATE_DATA_MSG
 */
void SoundingChart::update_data_msg(char *message)
{

   xv_set(P->SoundingChart_GUI_config_pu->data_msg,PANEL_LABEL_STRING,message,NULL);

   // Position messages on right edge.
   xv_set(P->SoundingChart_GUI_config_pu->data_msg,XV_X,
			  P->win_width -  xv_get(P->SoundingChart_GUI_config_pu->data_msg,XV_WIDTH) -10,
			 NULL);
}
