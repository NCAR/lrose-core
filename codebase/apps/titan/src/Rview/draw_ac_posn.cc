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
/****************************************************************
 * draw_ac_posn.c
 *
 * module for reading in and displaying aircraft position data
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * May 1993
 *
 *****************************************************************/

#include "Rview.hh"
#include <toolsa/pjg.h>
#include <didss/LdataInfo.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <rapformats/ac_posn.h>
using namespace std;

#define ARROW_HEAD_ANGLE 15.0
#define POSN_RADIUS_KM 0.5
#define ARROW_HEAD_LENGTH_KM 0.7

typedef struct {

  double lat, lon;
  double x, y;
  double ht;
  long time;

} aircraft_posn_t;

typedef struct {

  int npoints;
  int npoints_allocated;
  si32 min_time_diff;
  char *ident;
  char *color;
  char *label;
  GC gc;
  GPoint *points;
  ac_posn_wmod_t *wposns;
  aircraft_posn_t image_posn;
  psgc_t psgc;
  int db_ok;

} ac_t;

/*
 * file scope variables
 */

static int Setup = FALSE;
static int Plot = FALSE;
static int Use_spdb = FALSE;
static char *Realtime_path;
static char *Archive_dir;
static char *Db_dir;
static si32 Fwd_time_margin;
static si32 Back_time_margin;
static si32 N_ident;
static ac_t *Aircraft;
static Colormap Cmap;
static double Max_speed_km_hr;
static int Max_bad_points;
static double posn_radius;
static double arrow_head_length;

static int PlotFlares = FALSE;
static double Ejectable_icon_size_km = 0.5;
static double ejectable_icon_size;
static int Plot_ejectables_as_cross = TRUE;
static int Plot_ejectables_as_plus = TRUE;
static int LineWidthPerFlare = 1;
static int DryIceLineWidth = 2;

GC End_burn_gc;
GC Bip_gc;
GC End_burn_and_bip_gc;
GC Dry_ice_gc;

/*
 * prototypes
 */

static void check_point_alloc(ac_t *aircraft);

static void draw_ascii_posn(int dev,
			    gframe_t *frame,
			    time_t ref_time);

static void draw_spdb_posn(int dev,
			   gframe_t *frame,
			   time_t ref_time,
			   int ac_num);

static void init_setup();

static void prepare_for_plot(gframe_t *frame);

static void draw_ejectable(int dev,
			   gframe_t *frame,
			   ac_t *ac,
			   double prev_x, double prev_y,
			   double x, double y,
			   ac_posn_wmod_t *wposn);

/**************
 * main routine
 */

void draw_ac_posn(int dev,
		  gframe_t *frame,
		  time_t ref_time)

{

  int i;
  
  if (Glob->verbose) {
    fprintf(stderr, "** draw_ac_posn **\n");
  }
  
  /*
   * set up if necessary
   */
  
  if (!Setup) {
    init_setup();
  }
  
  if (!Plot) {
    return;
  }

  /*
   * prepare for plot
   */

  prepare_for_plot(frame);

  /*
   * draw, using old (ascii) or new (spdb) mode
   */

  if (Use_spdb) {
    for (i = 0; i < N_ident; i++) {
      draw_spdb_posn(dev, frame, ref_time, i);
    }
  } else {
    draw_ascii_posn(dev, frame, ref_time);
  }

}


/*******************
 * memory allocation
 */

static void check_point_alloc(ac_t *aircraft)

{
  
  if (aircraft->npoints_allocated == 0) {
    aircraft->npoints_allocated = 10;
    aircraft->points = (GPoint *) umalloc
      ((ui32) (aircraft->npoints_allocated  * sizeof(GPoint)));
    aircraft->wposns = (ac_posn_wmod_t *) umalloc
      ((ui32) (aircraft->npoints_allocated  * sizeof(ac_posn_wmod_t)));
  } else if (aircraft->npoints_allocated < aircraft->npoints + 1) {
    aircraft->npoints_allocated += 10;
    aircraft->points = (GPoint *) urealloc
      ((char *) aircraft->points,
       (ui32) (aircraft->npoints_allocated  * sizeof(GPoint)));
    aircraft->wposns = (ac_posn_wmod_t *) urealloc
      (aircraft->wposns,
       (ui32) (aircraft->npoints_allocated  * sizeof(ac_posn_wmod_t)));
  }

  return;

}


/*******************
 * draw_ascii_posn()
 *
 * Posn based on ascii file input
 */

static void draw_ascii_posn(int dev,
			    gframe_t *frame,
			    time_t ref_time)

{

  char line[BUFSIZ];
  char ident_str[32];
  char archive_path[MAX_PATH_LEN];
  char *file_path;
  int i, index;
  int within_time_limits;
  int npoints;
  si32 image_time = ref_time;
  si32 time_diff;
  FILE *file;
  aircraft_posn_t posn;

  /*
   * open ac_posn file
   */

  if (Glob->mode == ARCHIVE) {
    date_time_t rtime;
    rtime.unix_time = ref_time;
    uconvert_from_utime(&rtime);
    sprintf(archive_path, "%s%s%.4d%.2d%.2d",
	    Archive_dir, PATH_DELIM,
	    rtime.year, rtime.month, rtime.day);
    file_path = archive_path;
  } else {
    file_path = Realtime_path;
  }
  
  if ((file = fopen(file_path, "r")) == NULL) {     
    if (Glob->debug) {
      fprintf(stderr, "%s:draw_ac_posn\n", Glob->prog_name);
      fprintf(stderr, "Cannot open ac_posn file for reading\n");
      perror(file_path);
    }
    return;
  }
  
  /*
   * rewind the ac_posn file
   */
  
  fseek(file, 0L, 0);
  clearerr(file);

  /*
   * read in positions
   */

  while (!feof(file)) {
    
    if (fgets(line, BUFSIZ, file) != NULL) {
      
      if (sscanf(line, "%ld %lg %lg %lg %s",
		 &posn.time,
		 &posn.lat,
		 &posn.lon,
		 &posn.ht,
		 ident_str) != 5) {
	continue;
      }

      /*
       * get aircraft index
       */
      
      index = -1;
      for (i = 0; i < N_ident; i++) {
	if (!strcmp(ident_str, Aircraft[i].ident)) {
	  index = i;
	  break;
	}
      }

      within_time_limits = FALSE;
      if (Glob->mode == ARCHIVE) {
	if ((posn.time >= image_time - Back_time_margin) &&
	    (posn.time <= image_time + Fwd_time_margin)) {
	  within_time_limits = TRUE;
	}
      } else {
	if (posn.time >= image_time - Back_time_margin) {
	  within_time_limits = TRUE;
	}
      }
      
      if ((index > -1) && within_time_limits) {
	
	/*
	 * compute (x, y) relative to grid
	 */
	
	Glob->proj.latlon2xy(posn.lat, posn.lon,
			     posn.x, posn.y);

	/*
	 * add a point to the list
	 */
	
	check_point_alloc(Aircraft + index);
	npoints = Aircraft[index].npoints;
	Aircraft[index].points[npoints].x = posn.x;
	Aircraft[index].points[npoints].y = posn.y;
	Aircraft[index].npoints++;

	/*
	 * save point closest to image time
	 */

	time_diff = abs((int) (posn.time - image_time));
	if (time_diff < Aircraft[index].min_time_diff) {
	  Aircraft[index].min_time_diff = time_diff;
	  Aircraft[index].image_posn = posn;
	}

      } /* if ((index > -1) && ... */

    } /* if (fgets ... */

  } /* while (!feof ... */

  /*
   * plot tracks
   */

  for (i = 0; i < N_ident; i++) {

    npoints = Aircraft[i].npoints;

    if (npoints > 1) {
      
      if (dev == PSDEV)
	PsGsave(frame->psgc->file);
      
      if (dev == PSDEV) {
	PsSetLineStyle(frame->psgc->file, &Aircraft[i].psgc);
      }
      
      GDrawLines(dev, frame, Aircraft[i].gc, frame->psgc,
		 Aircraft[i].points, npoints,
		 CoordModeOrigin);
      
      GDrawArrow(dev, frame, Aircraft[i].gc, frame->psgc,
		 Aircraft[i].points[npoints - 2].x,
		 Aircraft[i].points[npoints - 2].y,
		 Aircraft[i].points[npoints - 1].x,
		 Aircraft[i].points[npoints - 1].y,
		 ARROW_HEAD_ANGLE, arrow_head_length);

      GDrawArc(dev, frame, Aircraft[i].gc, frame->psgc,
	       Aircraft[i].image_posn.x, Aircraft[i].image_posn.y,
	       posn_radius, posn_radius, 0.0, 360.0, 0.0, 20);
      
      if (dev == PSDEV)
	PsGrestore(frame->psgc->file);
  
    } /* if (Aircraft[i] ... */

  } /* i */
  
  /*
   * close file
   */

  fclose(file);

}

/******************
 * draw_spdb_posn()
 *
 * Posn based on spdb data base
 */

static void draw_spdb_posn(int dev,
			   gframe_t *frame,
			   time_t ref_time,
			   int ac_num)

{

  char db_path[MAX_PATH_LEN];
  int point_valid;
  int bad_count = 0;
  ac_posn_t posn;
  ac_posn_wmod_t wposn;
  int wmod_posn_avail = 0;
  time_t image_time = ref_time;
  int time_diff;
  double lat, lon, x, y;
  double prev_lat = 0.0, prev_lon = 0.0;
  double r, theta, speed_km_hr;
  time_t prev_time = 0;
  double dtime;
  GRectangle clip_rectangle[1];
  ac_t *ac = Aircraft + ac_num;

  /*
   * get data
   */

  DsSpdb spdb;

  // Are we using a server?
  
  if (strstr(Db_dir, "spdbp") != NULL) {

    // use server

    si32 chunk_type = Spdb::hash4CharsToInt32(ac->ident);
    
    if (spdb.getInterval(Db_dir,
                         image_time - Back_time_margin,
                         image_time + Fwd_time_margin,
                         chunk_type)) {
      fprintf(stderr, "WARNING %s:draw_ac_posn\n", Glob->prog_name);
      fprintf(stderr, "Cannot read ac_posn data base %s\n", db_path);
      fprintf(stderr, "  %s\n", spdb.getErrStr().c_str());
      return;
    }
    
  } else {

    LdataInfo ldata(Db_dir);

    /*
     * check to see if ldata_info file exists at top of db_dir
     * if so this indicates a single data base
     */

    if (ldata.read() == 0) {
      
      /*
       * single data base
       */
      
      si32 chunk_type = Spdb::hash4CharsToInt32(ac->ident);
      
      if (spdb.getInterval(Db_dir,
			   image_time - Back_time_margin,
			   image_time + Fwd_time_margin,
			   chunk_type)) {
	fprintf(stderr, "WARNING %s:draw_ac_posn\n", Glob->prog_name);
	fprintf(stderr, "Cannot read ac_posn data base %s\n", db_path);
	fprintf(stderr, "  %s\n", spdb.getErrStr().c_str());
	return;
      }
      
    } else {
      
      /*
       * multiple data base - so compute db path for ac subdirectory
       */
      
      sprintf(db_path, "%s%s%s", Db_dir, PATH_DELIM, ac->ident);
      
      if (spdb.getInterval(db_path,
			   image_time - Back_time_margin,
			   image_time + Fwd_time_margin)) {
	fprintf(stderr, "WARNING %s:draw_ac_posn\n", Glob->prog_name);
	fprintf(stderr, "Cannot read ac_posn data base %s\n", db_path);
	fprintf(stderr, "  %s\n", spdb.getErrStr().c_str());
	return;
      }
      
    }

  }

  /*
   * set the clipping rectangles
   */
  
  clip_rectangle->x = Glob->zoom[Glob->zoom_level].min_x;
  clip_rectangle->y = Glob->zoom[Glob->zoom_level].min_y;
  clip_rectangle->width =
    Glob->zoom[Glob->zoom_level].max_x - Glob->zoom[Glob->zoom_level].min_x;
  clip_rectangle->height =
    Glob->zoom[Glob->zoom_level].max_y - Glob->zoom[Glob->zoom_level].min_y;
  
  /*
   * load up positions for plotting
   */
  
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();

  for (int i = 0; i < (int) chunks.size(); i++) {

    const Spdb::chunk_t &chunk = chunks[i];

    if (chunk.data_type2 == SPDB_AC_POSN_WMOD_ID) {
      
      wmod_posn_avail = TRUE;
      wposn = *((ac_posn_wmod_t *) chunk.data);
      BE_to_ac_posn_wmod(&wposn);
      
      lat = wposn.lat;
      lon = wposn.lon;

    } else {

      wmod_posn_avail = FALSE;
      posn = *((ac_posn_t *) chunk.data);
      BE_to_ac_posn(&posn);
      
      lat = posn.lat;
      lon = posn.lon;

    }
      
    /*
     * compute (x, y) relative to grid
     */

    Glob->proj.latlon2xy(lat, lon, x, y);

    point_valid = TRUE;
    if (prev_time != 0 && Max_speed_km_hr < 10000.0) {
      PJGLatLon2RTheta(prev_lat, prev_lon, lat, lon, &r, &theta);
      dtime = (double) (chunk.valid_time - prev_time) / 3600.0;
      speed_km_hr = r / dtime;
      if (speed_km_hr > Max_speed_km_hr) {
	point_valid = FALSE;
      }
    }

    /*
     * make sure we do not get a whole string of bad points, which
     * may occur when data is lost for some period of time
     */
    
    if (point_valid) {
      bad_count = 0;
    } else {
      bad_count++;
    }

    if (bad_count > Max_bad_points) {
      bad_count = 0;
      point_valid = TRUE;
    }
    
    if (point_valid && (lat != 0.0 || lon != 0.0)) {

      /*
       * add a point to the list
       */
	
      check_point_alloc(ac);
      ac->points[ac->npoints].x = x;
      ac->points[ac->npoints].y = y;
      if (wmod_posn_avail) {
	ac->wposns[ac->npoints] = wposn;
      }
      ac->npoints++;

      /*
       * save point closest to image time
       */
      
      time_diff = abs((int) (chunk.valid_time - image_time));
      if (time_diff < ac->min_time_diff) {
	ac->min_time_diff = time_diff;
	ac->image_posn.x = x;
	ac->image_posn.y = y;
      }

      prev_lat = lat;
      prev_lon = lon;
      prev_time = chunk.valid_time;

    }

  } /* i */

  /*
   * plot track
   */
  
  if (ac->npoints) {
    
    if (!wmod_posn_avail) {

      /* normal plot - no flare info */

      if (dev == PSDEV)
	PsGsave(frame->psgc->file);
      
      if (dev == PSDEV) {
	PsSetLineStyle(frame->psgc->file, &ac->psgc);
      }
      
      GDrawLines(dev, frame, ac->gc, frame->psgc,
		 ac->points, ac->npoints,
		 CoordModeOrigin);
      
      if (ac->npoints > 1)
	GDrawArrow(dev, frame, ac->gc, frame->psgc,
		   ac->points[ac->npoints - 2].x,
		   ac->points[ac->npoints - 2].y,
		   ac->points[ac->npoints - 1].x,
		   ac->points[ac->npoints - 1].y,
		   ARROW_HEAD_ANGLE, arrow_head_length);
      
      GDrawArc(dev, frame, ac->gc, frame->psgc,
	       ac->image_posn.x, ac->image_posn.y,
	       posn_radius, posn_radius, 0.0, 360.0, 0.0, 20);
      
      if (dev == PSDEV)
	PsGrestore(frame->psgc->file);
    
    } else {
      
      /*  we have flare info */
      
      if (dev == PSDEV)
	PsGsave(frame->psgc->file);
      
      if (dev == PSDEV) {
	PsSetLineStyle(frame->psgc->file, &ac->psgc);
      }
      
      for (int i = 1; i < (int) ac->npoints; i++) {

	GC gc;
	int n_end_burn = 0;
	int n_bip = 0;
        int dry_ice = 0;

	if (PlotFlares) {

	  /* ejectables */
	  
	  draw_ejectable(dev, frame, ac, 
			 ac->points[i-1].x, ac->points[i-1].y,
			 ac->points[i].x, ac->points[i].y,
			 &ac->wposns[i]);
	  
	  if (ac->wposns[i-1].flare_flags & DRY_ICE_FLAG) {
	    dry_ice = 1;
	  }
	  if (ac->wposns[i-1].flare_flags & RIGHT_BURN_FLAG) {
	    n_end_burn++;
	  }
	  if (ac->wposns[i-1].flare_flags & LEFT_BURN_FLAG) {
	    n_end_burn++;
	  }
	  n_bip = ac->wposns[i-1].n_burn_in_place;
	  
	  if (dry_ice) {
	    XSetLineAttributes(Glob->rdisplay, Dry_ice_gc,
			       DryIceLineWidth,
			       LineSolid, CapButt, JoinMiter);
	    gc = Dry_ice_gc;
          } else if (n_end_burn && n_bip) {
	    XSetLineAttributes(Glob->rdisplay, End_burn_and_bip_gc,
			       (n_end_burn + n_bip) * LineWidthPerFlare,
			       LineSolid, CapButt, JoinMiter);
	    gc = End_burn_and_bip_gc;
	  } else if (n_end_burn) {
	    XSetLineAttributes(Glob->rdisplay, End_burn_gc,
			       n_end_burn * LineWidthPerFlare,
			       LineSolid, CapButt, JoinMiter);
	    gc = End_burn_gc;
	  } else if (n_bip) {
	    XSetLineAttributes(Glob->rdisplay, Bip_gc,
			       n_bip * LineWidthPerFlare,
			       LineSolid, CapButt, JoinMiter);
	    gc = Bip_gc;
	  } else {
	    gc = ac->gc;
	  }

	} else {
	  
	  gc = ac->gc;

	} /* if (PlotFlares) */

	GDrawLine(dev, frame, gc, frame->psgc,
		  ac->points[i-1].x, ac->points[i-1].y,
		  ac->points[i].x, ac->points[i].y);

      } /* i */
      
      if (ac->npoints > 1)
	GDrawArrow(dev, frame, ac->gc, frame->psgc,
		   ac->points[ac->npoints - 2].x,
		   ac->points[ac->npoints - 2].y,
		   ac->points[ac->npoints - 1].x,
		   ac->points[ac->npoints - 1].y,
		   ARROW_HEAD_ANGLE, arrow_head_length);
      
      GDrawArc(dev, frame, ac->gc, frame->psgc,
	       ac->image_posn.x, ac->image_posn.y,
	       posn_radius, posn_radius, 0.0, 360.0, 0.0, 20);
      
      if (dev == PSDEV)
	PsGrestore(frame->psgc->file);
      
    } /* if (!wmod_posn_avail) */
      
  } /* if (ac->npoints > 1) */

  /*
   * close files
   */

  /* SPDB_close(&ac->db); */

}

/* draw in ejectable flare icon */

static void draw_ejectable(int dev,
			   gframe_t *frame,
			   ac_t *ac,
			   double prev_x, double prev_y,
			   double xx, double yy,
			   ac_posn_wmod_t *wposn)
     
{

  int i;
  double size = ejectable_icon_size;
  double xxx, yyy;
  
  if (wposn->n_ejectable == 0) {
    /* no flares at this time */
    return;
  }

  /*
   * plot cross
   */

  if (Plot_ejectables_as_cross) {
    GDrawLine(dev, frame, ac->gc, frame->psgc, xx - size, yy - size, xx + size, yy + size);
    GDrawLine(dev, frame, ac->gc, frame->psgc, xx + size, yy - size, xx - size, yy + size);
  }
  if (Plot_ejectables_as_plus) {
    GDrawLine(dev, frame, ac->gc, frame->psgc, xx - size, yy, xx + size, yy);
    GDrawLine(dev, frame, ac->gc, frame->psgc, xx, yy - size, xx, yy + size);
  }
  
  /*
   * more than 1?
   */
  
  if (wposn->n_ejectable == 1) {
    /* no label necessary */
    return;
  }

  for (i = 1; i < wposn->n_ejectable; i++) {

    xxx = xx + (prev_x - xx) * ((double) i / (double) wposn->n_ejectable);
    yyy = yy + (prev_y - yy) * ((double) i / (double) wposn->n_ejectable);

    if (Plot_ejectables_as_cross) {
      GDrawLine(dev, frame, ac->gc, frame->psgc, xxx - size, yyy - size, xxx + size, yyy + size);
      GDrawLine(dev, frame, ac->gc, frame->psgc, xxx + size, yyy - size, xxx - size, yyy + size);
    }
    if (Plot_ejectables_as_plus) {
      GDrawLine(dev, frame, ac->gc, frame->psgc, xxx - size, yyy, xxx + size, yyy);
      GDrawLine(dev, frame, ac->gc, frame->psgc, xxx, yyy - size, xxx, yyy + size);
    }
    
  }

}

/************
 * initialize
 */

static void init_setup()

{

  char *resource_str;
  char *token;
  char *idents;
  char *colors;
  char *labels;
  char *linestyles;
  int i;
  
  Cmap = Glob->cmap;
    
  /*
   * read params from data base
   */

  /*
   * plot ac posn??
   */

  resource_str =
    uGetParamString(Glob->prog_name,
		  "plot_ac_posn", "false");
    
  if (uset_true_false_param(Glob->prog_name, "draw_ac_posn",
			    Glob->params_path_name,
			    resource_str, &Plot,
			    "plot_ac_posn")) {
    Plot = FALSE;
    return;
  }
  ufree(resource_str);

  /*
   * use SPDB??
   */
  
  resource_str = uGetParamString(Glob->prog_name,
			       "use_spdb_for_ac_posn", "false");
  
  if (uset_true_false_param(Glob->prog_name, "draw_ac_posn",
			    Glob->params_path_name,
			    resource_str, &Use_spdb,
			    "use_spdb_for_ac_posn")) {
    Use_spdb = FALSE;
  }
  ufree(resource_str);

  /*
   * directories
   */
  
  Db_dir =
    uGetParamString(Glob->prog_name,
		  "ac_posn_db_dir", "null");

  Archive_dir =
    uGetParamString(Glob->prog_name,
		  "ac_posn_archive_dir", "null");
    
  Realtime_path =
    uGetParamString(Glob->prog_name,
		  "ac_posn_realtime_file", "null");
  
  Back_time_margin =
    uGetParamLong(Glob->prog_name,
		"ac_posn_back_time_margin", 180);
    
  Fwd_time_margin =
    uGetParamLong(Glob->prog_name,
		"ac_posn_fwd_time_margin", 180);
    
  N_ident =
    uGetParamLong(Glob->prog_name,
		"ac_posn_n_ident", 1);
    
  idents =
    uGetParamString(Glob->prog_name,
		  "ac_posn_idents", "LTK");
    
  colors =
    uGetParamString(Glob->prog_name,
		  "ac_posn_colors", "white");
  
  labels =
    uGetParamString(Glob->prog_name,
		    "ac_posn_labels", idents);
  
  linestyles =
    uGetParamString(Glob->prog_name,
		  "ac_posn_linestyles", "1 100 0");
  
  Max_speed_km_hr =
    uGetParamDouble(Glob->prog_name,
		  "ac_posn_max_speed_kts", 10000.0) * KM_PER_NM;
  
  Max_bad_points =
    uGetParamLong(Glob->prog_name,
		"ac_posn_max_bad_points", 2);
    
  /*
   * alloc and load up resource arrays
   */
  
  Aircraft = (ac_t *) ucalloc
    ((ui32) N_ident, (ui32) sizeof(ac_t));
  
  /*
   * idents
   */

  token = strtok(idents, " ,\n\t");
  for (i = 0; i < N_ident; i++) {
    if (token == NULL) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Reading in ac_posn_idents\n");
      Plot = FALSE;
      break;
    }
    Aircraft[i].ident = (char *) umalloc ((ui32) strlen(token) + 1);
    strcpy(Aircraft[i].ident, token);
    token = strtok(NULL, " ,\n\t");
  }

  /*
   * colors
   */

  token = strtok(colors, " ,\n\t");
  for (i = 0; i < N_ident; i++) {
    if (token == NULL) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Reading in ac_posn_colors\n");
      Plot = FALSE;
      break;
    }
    Aircraft[i].color = (char *) umalloc ((ui32) strlen(token) + 1);
    strcpy(Aircraft[i].color, token);
    Aircraft[i].gc = xGetColorGC(Glob->rdisplay, Cmap,
				 &Glob->color_index, Aircraft[i].color);
    token = strtok(NULL, " ,\n\t");
  }

  /*
   * labels
   */

  token = strtok(labels, " ,\n\t");
  for (i = 0; i < N_ident; i++) {
    if (token == NULL) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Reading in ac_posn_labels\n");
      Plot = FALSE;
      break;
    }
    Aircraft[i].label = (char *) umalloc ((ui32) strlen(token) + 1);
    strcpy(Aircraft[i].label, token);
    token = strtok(NULL, " ,\n\t");
  }

  /*
   * linestyles
   */

  token = strtok(linestyles, ",");
  for (i = 0; i < N_ident; i++) {
    if (token == NULL) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Reading in ac_posn_linestyles\n");
      Plot = FALSE;
      break;
    }
    double line_width;
    double dash_length;
    double space_length;
    if (sscanf(token, "%lg %lg %lg",
	       &line_width, &dash_length, &space_length) != 3) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Reading in ac_posn_linestyles\n");
      Plot = FALSE;
      break;
    }
    Aircraft[i].psgc.line_width = line_width;
    Aircraft[i].psgc.dash_length = dash_length;
    Aircraft[i].psgc.space_length = space_length;
    Aircraft[i].psgc.fontsize = Glob->ps_track_annotation_fontsize;
    Aircraft[i].psgc.fontname = (char *) PS_FONTNAME;

    XSetLineAttributes(Glob->rdisplay, Aircraft[i].gc,
		       line_width, LineSolid, CapButt, JoinMiter);
    
    token = strtok(NULL, ",");
  }

  /*
   * flares
   */

  resource_str =
    uGetParamString(Glob->prog_name,
		    "plot_flares", "true");
  
  if (uset_true_false_param(Glob->prog_name, "draw_ac_posn",
			    Glob->params_path_name,
			    resource_str, &PlotFlares,
			    "plot_flares")) {
    PlotFlares = FALSE;
  }
  
  Ejectable_icon_size_km =
    uGetParamDouble(Glob->prog_name,
		    "ejectable_icon_size_km", 0.5);
  
  resource_str =
    uGetParamString(Glob->prog_name,
		    "plot_ejectables_as_cross", "true");
    
  if (uset_true_false_param(Glob->prog_name, "plot_ejectables_as_cross",
			    Glob->params_path_name,
			    resource_str, &Plot_ejectables_as_cross,
			    "plot_ejectables_as_cross")) {
    Plot_ejectables_as_cross = FALSE;
  }
  ufree(resource_str);
    
  resource_str =
    uGetParamString(Glob->prog_name,
		    "plot_ejectables_as_plus", "true");
    
  if (uset_true_false_param(Glob->prog_name, "plot_ejectables_as_plus",
			    Glob->params_path_name,
			    resource_str, &Plot_ejectables_as_plus,
			    "plot_ejectables_as_plus")) {
    Plot_ejectables_as_plus = FALSE;
  }
  ufree(resource_str);
  
  LineWidthPerFlare =
    uGetParamLong(Glob->prog_name,
		  "line_width_per_flare", 1);

  DryIceLineWidth =
    uGetParamLong(Glob->prog_name,
		  "dry_ice_line_width", 3);

  /*
   * flare colors
   */
  
  char *color =
    uGetParamString(Glob->prog_name, "end_burn_color", "red");
  End_burn_gc = xGetColorGC(Glob->rdisplay, Cmap,
			    &Glob->color_index, color);
  
  color =
    uGetParamString(Glob->prog_name, "burn_in_place_color", "blue");
  Bip_gc = xGetColorGC(Glob->rdisplay, Cmap,
		       &Glob->color_index, color);
  
  color =
    uGetParamString(Glob->prog_name, "end_burn_and_bip_color", "magenta");
  End_burn_and_bip_gc = xGetColorGC(Glob->rdisplay, Cmap,
				    &Glob->color_index, color);
  
  color =
    uGetParamString(Glob->prog_name, "dry_ice_color", "white");
  Dry_ice_gc = xGetColorGC(Glob->rdisplay, Cmap,
                           &Glob->color_index, color);
  
  /*
   * sizes - Initialize these last since they can depend on the above
   * settings.
   */

  posn_radius =
    Glob->proj.km2xGrid(POSN_RADIUS_KM) * Glob->proj.getCoord().dx;
  arrow_head_length =
    Glob->proj.km2xGrid(ARROW_HEAD_LENGTH_KM) * Glob->proj.getCoord().dx;
  ejectable_icon_size =
    Glob->proj.km2xGrid(Ejectable_icon_size_km) * Glob->proj.getCoord().dx;
  
  Setup = TRUE;
  
}

/********************
 * prepare_for_plot()
 */

static void prepare_for_plot(gframe_t *frame)

{

  int i;
  
  for (i = 0; i < N_ident; i++) {
    Aircraft[i].min_time_diff = 1000000000;
    Aircraft[i].psgc.file = frame->psgc->file;
    Aircraft[i].npoints = 0;
  }
  
}

/*************************************************
 * routines for providing details to other modules
 */

int n_aircraft()
{
  if (!Setup) {
    init_setup();
  }
  if (Plot) {
    return (N_ident);
  } else {
    return (0);
  }
}

GC get_aircraft_gc(int index)
{
  return (Aircraft[index].gc);
}

psgc_t *get_aircraft_psgc(int index)
{
  return (&Aircraft[index].psgc);
}

char *get_aircraft_ident(int index)
{
  return (Aircraft[index].ident);
}

char *get_aircraft_label(int index)
{
  return (Aircraft[index].label);
}

GC get_end_burn_gc() {
  return End_burn_gc;
}

GC get_bip_gc() {
  return Bip_gc;
}

GC get_end_burn_and_bip_gc() {
  return End_burn_and_bip_gc;
}

GC get_dry_ice_gc() {
  return Dry_ice_gc;
}

int get_plot_ac_posn() {
  return (Plot);
}

int get_plot_flares() {
  return (PlotFlares);
}

