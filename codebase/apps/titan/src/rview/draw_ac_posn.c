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

#include "rview.h"
#include <toolsa/pjg.h>
#include <toolsa/ldata_info.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <rapformats/ac_posn.h>

#define ARROW_HEAD_ANGLE 15.0
#define POSN_RADIUS 0.5
#define ARROW_HEAD_LENGTH 0.7

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

static int PlotFlares = FALSE;
static double Ejectable_icon_size_km = 0.5;
static int Plot_ejectables_as_cross = TRUE;
static int Plot_ejectables_as_plus = TRUE;
static int LineWidthPerFlare = 1;

GC End_burn_gc;
GC Bip_gc;
GC End_burn_and_bip_gc;

/*
 * prototypes
 */

static void check_point_alloc(ac_t *aircraft);

static void draw_ascii_posn(int dev,
			    gframe_t *frame,
			    date_time_t *cappi_time);

static void draw_spdb_posn(int dev,
			   gframe_t *frame,
			   date_time_t *cappi_time,
			   int ac_num);

static void init_setup(void);

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
		  date_time_t *cappi_time)

{

  int i;
  
  if (Glob->debug) {
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
      draw_spdb_posn(dev, frame, cappi_time, i);
    }
  } else {
    draw_ascii_posn(dev, frame, cappi_time);
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
			    date_time_t *cappi_time)

{

  char line[BUFSIZ];
  char ident_str[32];
  char archive_path[MAX_PATH_LEN];
  char *file_path;
  int i, index;
  int within_time_limits;
  int npoints;
  si32 image_time = cappi_time->unix_time;
  si32 time_diff;
  FILE *file;
  aircraft_posn_t posn;

  /*
   * open ac_posn file
   */

  if (Glob->mode == TDATA_ARCHIVE) {
    sprintf(archive_path, "%s%s%.4d%.2d%.2d",
	    Archive_dir, PATH_DELIM,
	    cappi_time->year, cappi_time->month, cappi_time->day);
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
      if (Glob->mode == TDATA_ARCHIVE) {
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
	
	TITAN_latlon2xy(&Glob->grid_comps, posn.lat, posn.lon,
		      &posn.x, &posn.y);

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
		 ARROW_HEAD_ANGLE, ARROW_HEAD_LENGTH);

      GDrawArc(dev, frame, Aircraft[i].gc, frame->psgc,
	       Aircraft[i].image_posn.x, Aircraft[i].image_posn.y,
	       POSN_RADIUS, POSN_RADIUS, 0.0, 360.0, 0.0, 20);
      
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
			   date_time_t *cappi_time,
			   int ac_num)

{

  char db_path[MAX_PATH_LEN];
  int i;
  int point_valid;
  int bad_count = 0;
  si32 chunk_type;
  ui32 npoints;
  spdb_chunk_ref_t *refs, *ref;
  void *ac_data;
  ac_posn_t posn;
  ac_posn_wmod_t wposn;
  int wmod_posn_avail;
  si32 image_time = cappi_time->unix_time;
  si32 time_diff;
  double lat, lon, x, y;
  double prev_lat, prev_lon;
  double r, theta, speed_km_hr;
  time_t prev_time = 0;
  double dtime;
  GRectangle clip_rectangle[1];
  ac_t *ac;
  LDATA_handle_t ldata;

  ac = Aircraft + ac_num;

  /*
   * get data
   */

  /*
   * check to see if ldata_info file exists at top of db_dir - if so this indicates
   * a single data base
   */

  LDATA_init_handle(&ldata, Glob->prog_name, Glob->debug);

  if (LDATA_info_read(&ldata, Db_dir, -1) == 0) {
    
    /*
     * single data base
     */

    chunk_type = SPDB_4chars_to_int32(ac->ident);
  
    if (SPDB_get_interval(Db_dir,
			  AC_POSN_PROD_CODE, chunk_type,
			  image_time - Back_time_margin,
			  image_time + Fwd_time_margin,
			  &npoints, &refs, &ac_data)) {
      fprintf(stderr, "WARNING %s:draw_ac_posn\n", Glob->prog_name);
      fprintf(stderr, "Cannot read ac_posn data base %s\n", db_path);
      LDATA_free_handle(&ldata);
      return;
    }

  } else {
      
    /*
     * multiple data base - so compute db path for ac subdirectory
     */
    
    sprintf(db_path, "%s%s%s", Db_dir, PATH_DELIM, ac->ident);
    
    if (SPDB_get_interval(db_path,
			  AC_POSN_PROD_CODE, 0,
			  image_time - Back_time_margin,
			  image_time + Fwd_time_margin,
			  &npoints, &refs, &ac_data)) {
      fprintf(stderr, "WARNING %s:draw_ac_posn\n", Glob->prog_name);
      fprintf(stderr, "Cannot read ac_posn data base %s\n", db_path);
      LDATA_free_handle(&ldata);
      return;
    }

  }

  LDATA_free_handle(&ldata);

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

  ref = refs;

  for (i = 0; i < npoints; i++, ref++) {

    if (ref->prod_id == SPDB_AC_POSN_WMOD_ID) {

      wmod_posn_avail = TRUE;
      wposn = *((ac_posn_wmod_t *) ((char *) ac_data + ref->offset));
      BE_to_ac_posn_wmod(&wposn);
      
      /*
       * compute (x, y) relative to grid
       */
      
      lat = wposn.lat;
      lon = wposn.lon;

    } else {

      wmod_posn_avail = FALSE;
      posn = *((ac_posn_t *) ((char *) ac_data + ref->offset));
      BE_to_ac_posn(&posn);
      
      /*
       * compute (x, y) relative to grid
       */
      
      lat = posn.lat;
      lon = posn.lon;

    }
      
    TITAN_latlon2xy(&Glob->grid_comps, lat, lon, &x, &y);

    point_valid = TRUE;
    if (prev_time != 0 && Max_speed_km_hr < 10000.0) {
      PJGLatLon2RTheta(prev_lat, prev_lon, lat, lon, &r, &theta);
      dtime = (double) (ref->valid_time - prev_time) / 3600.0;
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
      
      time_diff = abs((int) (ref->valid_time - image_time));
      if (time_diff < ac->min_time_diff) {
	ac->min_time_diff = time_diff;
	ac->image_posn.x = x;
	ac->image_posn.y = y;
      }

      prev_lat = lat;
      prev_lon = lon;
      prev_time = ref->valid_time;

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
      
      GDrawArrow(dev, frame, ac->gc, frame->psgc,
		 ac->points[npoints - 2].x,
		 ac->points[npoints - 2].y,
		 ac->points[npoints - 1].x,
		 ac->points[npoints - 1].y,
		 ARROW_HEAD_ANGLE, ARROW_HEAD_LENGTH);
      
      GDrawArc(dev, frame, ac->gc, frame->psgc,
	       ac->image_posn.x, ac->image_posn.y,
	       POSN_RADIUS, POSN_RADIUS, 0.0, 360.0, 0.0, 20);
      
      if (dev == PSDEV)
	PsGrestore(frame->psgc->file);
      
    } else {
      
      /*  we have flare info */
      
      if (dev == PSDEV)
	PsGsave(frame->psgc->file);
      
      if (dev == PSDEV) {
	PsSetLineStyle(frame->psgc->file, &ac->psgc);
      }
      
      for (i = 1; i < npoints; i++) {

	GC gc;
	int n_end_burn = 0;
	int n_bip = 0;

	if (PlotFlares) {

	  /* ejectables */
	  
	  draw_ejectable(dev, frame, ac, 
			 ac->points[i-1].x, ac->points[i-1].y,
			 ac->points[i].x, ac->points[i].y,
			 &ac->wposns[i]);
	  
	  if (ac->wposns[i-1].flare_flags & RIGHT_BURN_FLAG) {
	    n_end_burn++;
	  }
	  if (ac->wposns[i-1].flare_flags & LEFT_BURN_FLAG) {
	    n_end_burn++;
	  }
	  n_bip = ac->wposns[i-1].n_burn_in_place;
	  
	  if (n_end_burn && n_bip) {
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
      
      GDrawArrow(dev, frame, ac->gc, frame->psgc,
		 ac->points[npoints - 2].x,
		 ac->points[npoints - 2].y,
		 ac->points[npoints - 1].x,
		 ac->points[npoints - 1].y,
		 ARROW_HEAD_ANGLE, ARROW_HEAD_LENGTH);
      
      GDrawArc(dev, frame, ac->gc, frame->psgc,
	       ac->image_posn.x, ac->image_posn.y,
	       POSN_RADIUS, POSN_RADIUS, 0.0, 360.0, 0.0, 20);
      
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
  double size = Ejectable_icon_size_km;
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

static void init_setup(void)

{

  char *resource_str;
  char *token;
  char *idents;
  char *colors;
  char *color;
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
    xGetResString(Glob->rdisplay, Glob->prog_name,
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
  
  resource_str = xGetResString(Glob->rdisplay, Glob->prog_name,
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
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ac_posn_db_dir", "null");

  Archive_dir =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ac_posn_archive_dir", "null");
    
  Realtime_path =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ac_posn_realtime_file", "null");
  
  Back_time_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"ac_posn_back_time_margin", 180);
    
  Fwd_time_margin =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"ac_posn_fwd_time_margin", 180);
    
  N_ident =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"ac_posn_n_ident", 1);
    
  idents =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ac_posn_idents", "LTK");
    
  colors =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ac_posn_colors", "white");
  
  linestyles =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "ac_posn_linestyles", "1 100 0");
  
  Max_speed_km_hr =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ac_posn_max_speed_kts", 10000.0) * KM_PER_NM;
  
  Max_bad_points =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
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
    if (sscanf(token, "%lg %lg %lg",
	       &Aircraft[i].psgc.line_width,
	       &Aircraft[i].psgc.dash_length,
	       &Aircraft[i].psgc.space_length) != 3) {
      fprintf(stderr, "ERROR - %s\n", Glob->prog_name);
      fprintf(stderr, "Reading in ac_posn_linestyles\n");
      Plot = FALSE;
      break;
    }
    Aircraft[i].psgc.fontsize = Glob->ps_track_annotation_fontsize;
    Aircraft[i].psgc.fontname = PS_FONTNAME;
    token = strtok(NULL, ",");
  }

  /*
   * flares
   */
  
  resource_str =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "plot_flares", "true");
  
  if (uset_true_false_param(Glob->prog_name, "draw_ac_posn",
			    Glob->params_path_name,
			    resource_str, &PlotFlares,
			    "plot_flares")) {
    PlotFlares = FALSE;
  }

  Ejectable_icon_size_km =
    xGetResDouble(Glob->rdisplay, Glob->prog_name,
		  "ejectable_icon_size_km", 0.5);
  
  resource_str =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "plot_ejectables_as_cross", "true");
    
  if (uset_true_false_param(Glob->prog_name, "plot_ejectables_as_cross",
			    Glob->params_path_name,
			    resource_str, &Plot_ejectables_as_cross,
			    "plot_ejectables_as_cross")) {
    Plot_ejectables_as_cross = FALSE;
  }
  ufree(resource_str);
    
  resource_str =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "plot_ejectables_as_plus", "true");
    
  if (uset_true_false_param(Glob->prog_name, "plot_ejectables_as_plus",
			    Glob->params_path_name,
			    resource_str, &Plot_ejectables_as_plus,
			    "plot_ejectables_as_plus")) {
    Plot_ejectables_as_plus = FALSE;
  }
  ufree(resource_str);

  LineWidthPerFlare =
    xGetResLong(Glob->rdisplay, Glob->prog_name,
		"line_width_per_flare", 1);

  /*
   * flare colors
   */
  
  color =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "end_burn_color", "red");
  End_burn_gc = xGetColorGC(Glob->rdisplay, Cmap,
			    &Glob->color_index, color);
  
  color =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "burn_in_place_color", "blue");
  Bip_gc = xGetColorGC(Glob->rdisplay, Cmap,
		       &Glob->color_index, color);
  
  color =
    xGetResString(Glob->rdisplay, Glob->prog_name,
		  "end_burn_and_bip_color", "magenta");
  End_burn_and_bip_gc = xGetColorGC(Glob->rdisplay, Cmap,
				    &Glob->color_index, color);
  
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

int n_aircraft(void)
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

GC get_end_burn_gc(void) {
  return End_burn_gc;
}

GC get_bip_gc(void) {
  return Bip_gc;
}

GC get_end_burn_and_bip_gc(void) {
  return End_burn_and_bip_gc;
}

int get_plot_flares(void) {
  return (PlotFlares);
}
