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
/************************************************************************
 * map.c
 *
 * module for reading in and displaying map data
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * October 1992
 *
 *************************************************************************/

#include "rview.h"

/*
 * type definitions for map objects
 */

typedef struct {
  double x;
  double y;
} double_coord_t;

typedef struct {
  si32 x;
  si32 y;
} long_coord_t;

typedef struct {
  char *name;
  si32 npts;
  long_coord_t *pt;
} icondef_t;

typedef struct {
  si32 defnum;
  char *name;
  double_coord_t loc;
  long_coord_t label_offset;
  char *label;
} icon_t;

typedef struct {
  char *name;
  si32 npts;
  double_coord_t *pt;
} polyline_t;

typedef struct {
  char *text;
  double_coord_t loc;
} simplelabel_t;

typedef struct {
  int selected_limited, selected_all;
  int projection;
  int xwidth;
  si32 nicondefs;
  si32 nicons;
  si32 npolylines;
  si32 nsimplelabels;
  icondef_t *icondef;
  icon_t *icon;
  polyline_t *polyline;
  simplelabel_t *simplelabel;
  GC gc;
  XFontStruct *xfont;
  psgc_t psgc;
  char label[64];
  char xcolor[64];
  char xfontname[64];
  char file_path[256];
} rview_map_t;

#define ICONDEF "ICONDEF"
#define ICON "ICON"
#define POLYLINE "POLYLINE"
#define SIMPLELABEL "SIMPLELABEL"
#define PROJ "PROJECTION"

#define ICON_FLAG 32767
#define POLYLINE_FILE_FLAG -999.0
#define POLYLINE_PLOT_PENUP -1000000.0
#define POLYLINE_PLOT_FLAG -999999.0

/*
 * file scope variables
 */

static rview_map_t *Maps = NULL;
static int Nmaps = 0;

static si32 line_num;

/*
 * file scope prototypes
 */

static void draw_icon(rview_map_t *map, si32 icon_num,
		      int dev, gframe_t *frame);
static void draw_map(int dev, gframe_t *frame, int imap);
static void print_error(char *line,
			char *message,
			rview_map_t *map);
static int read_map_conf(void);
static void read_map_file(int imap);

/******************************************************************
 * read_map_files()
 *
 * Reads in map configuration file, and individual map
 * files
 */

void read_map_files(void)

{

  int imap;
  
  if (Glob->debug) {
    fprintf(stderr, "** read_map_files **\n");
  }

  read_map_conf();
  
  for (imap = 0; imap < Nmaps; imap++) {
    read_map_file(imap);
  } /* imap */
  
  return;

}

/********************************************************************
 * draw_maps()
 *
 * Draw the maps
 */

void draw_maps(int dev,
	       gframe_t *frame)

{

  int imap;
  
  if (Glob->debug) {
    fprintf(stderr, "** draw_maps **\n");
  }

  for (imap = 0; imap < Nmaps; imap++) {
    if (Glob->plot_maps == MAPS_LIMITED &&
	Maps[imap].selected_limited) {
      draw_map(dev, frame, imap);
    } else if (Glob->plot_maps == MAPS_ALL &&
	       Maps[imap].selected_all) {
      draw_map(dev, frame, imap);
    }
  } /* imap */
  
  return;

}

/******************************************************************
 * read_map_file()
 *
 * reads in the map file, and stores the map data relative to the
 * plot grid
 */

static void read_map_file(int imap)

{

  char *token;
  char *end_pt;
  char line[BUFSIZ];
  char tline[BUFSIZ];

  int def_found;

  si32 i, j;
  si32 icon_x, icon_y;
  si32 icondef_num, icon_num, polyline_num, simplelabel_num;
  si32 ipt, l;
  
  double lat, lon;

  rview_map_t *map;
  FILE *map_file;

  /*
   * set local variables
   */

  map = Maps + imap;

  /*
   * open map file
   */

  if ((map_file = fopen(map->file_path, "r")) == NULL) {
    
    fprintf(stderr, "ERROR - %s:read_map_file\n", Glob->prog_name);
    perror(map->file_path);
    tidy_and_exit(-1);
    
  }

  /*
   * determine the number of each object
   */

  while (fgets(line, BUFSIZ, map_file) != NULL) {
    
    if (!strncmp(line, ICONDEF, strlen(ICONDEF)))
      map->nicondefs++;
    else if (!strncmp(line, ICON, strlen(ICON)))
      map->nicons++;
    else if (!strncmp(line, POLYLINE, strlen(POLYLINE)))
      map->npolylines++;
    else if (!strncmp(line, SIMPLELABEL, strlen(SIMPLELABEL)))
      map->nsimplelabels++;

  } /* while */

  /*
   * allocate memory for map objects
   */

  map->icondef = (icondef_t *) umalloc
    ((ui32)(map->nicondefs * sizeof(icondef_t)));

  map->icon = (icon_t *) umalloc
    ((ui32)(map->nicons * sizeof(icon_t)));

  map->polyline = (polyline_t *) umalloc
    ((ui32)(map->npolylines * sizeof(polyline_t)));

  map->simplelabel = (simplelabel_t *) umalloc
    ((ui32)(map->nsimplelabels * sizeof(simplelabel_t)));

  /*
   * rewind file
   */
  
  fseek (map_file, 0L, 0);

  /*
   * read in map object data
   */

  icondef_num = 0;
  icon_num = 0;
  polyline_num = 0;
  simplelabel_num = 0;
  line_num = 0;

  while (fgets(line, BUFSIZ, map_file) != NULL) {

    strncpy(tline, line, BUFSIZ);
    line_num++;
    if (tline[0] == '#') {
      continue;
    }

    if (!strncmp(line, PROJ, strlen(PROJ))) {
      
      token = strtok(tline, " \n\t");
      token = strtok((char *) NULL, " \n\t");

      errno = 0;
      map->projection = strtol(token, &end_pt, 10);

      if (errno) {
	print_error(line,
		    "Reading projection - setting to TITAN_PROJ_FLAT", map);
	map->projection = TITAN_PROJ_FLAT;
	continue;
      }

    } else if (!strncmp(line, ICONDEF, strlen(ICONDEF))) {

      /*
       * ICONDEF
       */

      token = strtok(tline, " \n\t");

      /*
       * name
       */

      token = strtok((char *) NULL, " \n\t");
      if (token == NULL) {
	print_error(line, "Reading icondef name", map);
	continue;
      }
      
      map->icondef[icondef_num].name = (char *) umalloc
	((ui32) strlen(token) + 1);

      strcpy(map->icondef[icondef_num].name, token);

      /*
       * npts
       */

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      l = strtol(token, &end_pt, 10);
      if (errno) {
	print_error(line, "Reading icondef npts", map);
	continue;
      }
      map->icondef[icondef_num].npts = l;

      map->icondef[icondef_num].pt = (long_coord_t *) umalloc
	((ui32) l * sizeof(long_coord_t));

      /*
       * coords for each point
       */

      for (ipt = 0; ipt < l; ipt++) {

	if (fgets(line, BUFSIZ, map_file) == NULL) {
	  break;
	}
	line_num++;
	if (line[0] == '#') {
	  ipt--;
	  continue;
	}

	errno = 0;
	icon_x = strtol(line, &end_pt, 10);
	if (errno) {
	  print_error(line, "Reading icondef x value", map);
	  continue;
	}
	map->icondef[icondef_num].pt[ipt].x = icon_x;
    
	errno = 0;
	icon_y = strtol(end_pt, &end_pt, 10);
	if (errno) {
	  print_error(line, "Reading icondef y value", map);
	  continue;
	}
	map->icondef[icondef_num].pt[ipt].y = icon_y;

      } /* ipt */
      
      icondef_num++;

    } else if (!strncmp(line, ICON, strlen(ICON))) {

      /*
       * ICON
       */

      token = strtok(tline, " \n\t");

      /*
       * name
       */

      token = strtok((char *) NULL, " \n\t");
      if (token == NULL) {
	print_error(line, "Reading icon name", map);
	continue;
      }
      
      map->icon[icon_num].name = (char *) umalloc
	((ui32) strlen(token) + 1);
      strcpy(map->icon[icon_num].name, token);

      /*
       * location
       */

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      lat = strtod(token, &end_pt);
      if (errno) {
	print_error(line, "Reading icon lat", map);
	continue;
      }

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      lon = strtod(token, &end_pt);
      if (errno) {
	print_error(line, "Reading icon lon", map);
	continue;
      }

      TITAN_latlon2xy(&Glob->grid_comps, lat, lon,
		    &map->icon[icon_num].loc.x, 
		    &map->icon[icon_num].loc.y);
      
      /*
       * text offset
       */

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      l = strtol(token, &end_pt, 10);
      if (errno) {
	print_error(line, "Reading icon x text offset", map);
	continue;
      }
      map->icon[icon_num].label_offset.x = l;

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      l = strtol(token, &end_pt, 10);
      if (errno) {
	print_error(line, "Reading icon y text offset", map);
	continue;
      }
      map->icon[icon_num].label_offset.y = l;

      /*
       * label
       */

      if (map->icon[icon_num].label_offset.x != ICON_FLAG ||
	  map->icon[icon_num].label_offset.x != ICON_FLAG) {

	token = strtok((char *) NULL, " \n\t");
	if (token == NULL) {
	  print_error(line, "Reading icon label", map);
	  continue;
	}
      
	map->icon[icon_num].label = (char *) umalloc
	  ((ui32) strlen(token) + 1);
	strcpy(map->icon[icon_num].label, token);

      } else {
	
	map->icon[icon_num].label = (char *) NULL;	

      } 

      icon_num++;

    } else if (!strncmp(line, POLYLINE, strlen(POLYLINE))) {

      /*
       * POLYLINE
       */

      token = strtok(tline, " \n\t");

      /*
       * name
       */

      token = strtok((char *) NULL, " \n\t");
      if (token == NULL) {
	print_error(line, "Reading polyline name", map);
	continue;
      }
      
      map->polyline[polyline_num].name = (char *) umalloc
	((ui32) strlen(token) + 1);

      strcpy(map->polyline[polyline_num].name, token);

      /*
       * npts
       */

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      l = strtol(token, &end_pt, 10);
      if (errno) {
	print_error(line, "Reading polyline npts", map);
	continue;
      }
      map->polyline[polyline_num].npts = l;

      map->polyline[polyline_num].pt = (double_coord_t *) umalloc
	((ui32) l * sizeof(double_coord_t));

      /*
       * coords for each point
       */

      for (ipt = 0; ipt < l; ipt++) {

	if (fgets(line, BUFSIZ, map_file) == NULL) {
	  break;
	}
	line_num++;
	if (line[0] == '#') {
	  ipt--;
	  continue;
	}

	errno = 0;
	lat = strtod(line, &end_pt);
	if (errno) {
	  print_error(line, "Reading polyline lat", map);
	  continue;
	}
    
	errno = 0;
	lon = strtod(end_pt, &end_pt);
	if (errno) {
	  print_error(line, "Reading polyline lon", map);
	  continue;
	}

	if (lat < POLYLINE_FILE_FLAG &&
	    lon < POLYLINE_FILE_FLAG) {

	  map->polyline[polyline_num].pt[ipt].x = POLYLINE_PLOT_PENUP;
	  map->polyline[polyline_num].pt[ipt].y = POLYLINE_PLOT_PENUP;

	} else {

	  TITAN_latlon2xy(&Glob->grid_comps, lat, lon,
			&map->polyline[polyline_num].pt[ipt].x,
			&map->polyline[polyline_num].pt[ipt].y);
      
	}

      } /* ipt */
      
      polyline_num++;

    } else if (!strncmp(line, SIMPLELABEL, strlen(SIMPLELABEL))) {

      /*
       * SIMPLELABEL
       */
      
      token = strtok(tline, " \n\t");

      /*
       * location
       */

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      lat = strtod(token, &end_pt);
      if (errno) {
	print_error(line, "Reading simplelabel lat", map);
	continue;
      }

      token = strtok((char *) NULL, " \n\t");
      errno = 0;
      lon = strtod(token, &end_pt);
      if (errno) {
	print_error(line, "Reading simplelabel lon", map);
	continue;
      }

      TITAN_latlon2xy(&Glob->grid_comps, lat, lon,
		    &map->simplelabel[simplelabel_num].loc.x,
		    &map->simplelabel[simplelabel_num].loc.y);

      /*
       * text
       */

      token = strtok((char *) NULL, " \n\t");
      if (token == NULL) {
	print_error(line, "Reading simplelabel text", map);
	continue;
      }
      
      map->simplelabel[simplelabel_num].text = (char *) umalloc
	((ui32) strlen(token) + 1);
      strcpy(map->simplelabel[simplelabel_num].text, token);

      simplelabel_num++;

    }

  } /* while */

  /*
   * set the icondef numbers associated with each icon
   */

  for (i = 0; i < map->nicons; i++) {

    def_found = FALSE;

    for (j = 0; j < map->nicondefs; j++) {

      if (!strcmp(map->icon[i].name, map->icondef[j].name)) {
	map->icon[i].defnum = j;
	def_found = TRUE;
	break;
      }

    } /* j */

    if (!def_found) {
      map->icon[i].defnum = -1;
      fprintf(stderr, "Warning - icon name '%s' not defined\n",
	      map->icon[i].name);
      fprintf(stderr, "Check file %s\n", map->file_path);
    }

  } /* i */

  fclose(map_file);

}

/********************************************************************
 * draw_map()
 */

static void draw_map(int dev, gframe_t *frame, int imap)

{

  si32 i, ipt;
  double xx1, yy1, xx2, yy2;
  rview_map_t *map;

  map = Maps + imap;

  /*
   * if postscript, set tick line width and ticklabel fontsize
   */
  
  if (dev == PSDEV) {
    PsGsave(frame->psgc->file);
    PsSetLineStyle(frame->psgc->file, &map->psgc);
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      map->psgc.fontsize);
  }
  
  /*
   * draw in labels
   */

  for (i = 0; i < map->nsimplelabels; i++) {

    GDrawString(dev, frame, map->gc, map->xfont,
		frame->psgc, XJ_CENTER, YJ_CENTER,
		map->simplelabel[i].loc.x,
		map->simplelabel[i].loc.y,
		map->simplelabel[i].text);

  } /* i */

  /*
   * draw in polylines
   */

  for (i = 0; i < map->npolylines; i++) {

    for (ipt = 0; ipt < map->polyline[i].npts - 1; ipt++) {

      xx1 = map->polyline[i].pt[ipt].x;
      yy1 = map->polyline[i].pt[ipt].y;
      xx2 = map->polyline[i].pt[ipt+1].x;
      yy2 = map->polyline[i].pt[ipt+1].y;

      if (xx1 > POLYLINE_PLOT_FLAG &&
	  yy1 > POLYLINE_PLOT_FLAG &&
	  xx2 > POLYLINE_PLOT_FLAG &&
	  yy2 > POLYLINE_PLOT_FLAG) {

	GDrawLine(dev, frame, map->gc, frame->psgc,
		  xx1, yy1, xx2, yy2);
    

      } 

    } /* ipt */

  } /* i */

  /*
   * draw in icons
   */

  for (i = 0; i < map->nicons; i++)
    draw_icon(map, i, dev, frame);

  /*
   * restore postscript
   */

  if (dev == PSDEV)
    PsGrestore(frame->psgc->file);

}

/********************************************************************
 * draw_icon()
 */

static void draw_icon(rview_map_t *map, si32 icon_num,
		      int dev, gframe_t *frame)

{

  si32 ipt;
  si32 ix1, iy1, ix2, iy2;
  double xx, yy, xx1, yy1, xx2, yy2;
  double xscale, yscale;
  icon_t *this_icon;
  icondef_t *this_def;

  /*
   * return now if icon has no definition
   */

  this_icon = map->icon + icon_num;
  
  if (this_icon->defnum < 0)
    return;

  /*
   * get definition
   */

  this_def = map->icondef + this_icon->defnum;

  /*
   * get scale factors
   */

  if (dev == XDEV) {
    xscale = frame->x->xscale;
    yscale = frame->x->yscale;
  } else {
    xscale = frame->ps->xscale;
    yscale = frame->ps->yscale;
  }

  /*
   * draw in label
   */

  if (this_icon->label != NULL) {

    xx = this_icon->loc.x + (double) this_icon->label_offset.x / xscale;
    yy = this_icon->loc.y + (double) this_icon->label_offset.y / yscale;
    
    GDrawString(dev, frame, map->gc, map->xfont,
		frame->psgc, XJ_CENTER, YJ_CENTER,
		xx, yy, this_icon->label);

  } /* if (this_icon->label ... */

  /*
   * draw in icon lines
   */

  for (ipt = 0; ipt < this_def->npts - 1; ipt++) {

    ix1 = this_def->pt[ipt].x;
    iy1 = this_def->pt[ipt].y;
    ix2 = this_def->pt[ipt+1].x;
    iy2 = this_def->pt[ipt+1].y;

    if (ix1 != ICON_FLAG &&
	iy1 != ICON_FLAG &&
	ix2 != ICON_FLAG &&
	iy2 != ICON_FLAG) {

      xx1 = this_icon->loc.x + (double) ix1 / xscale;
      yy1 = this_icon->loc.y + (double) iy1 / yscale;
      xx2 = this_icon->loc.x + (double) ix2 / xscale;
      yy2 = this_icon->loc.y + (double) iy2 / yscale;
    
      GDrawLine(dev, frame, map->gc, frame->psgc,
		xx1, yy1, xx2, yy2);

    } /* if (ix1 ... */

  } /* ipt */

}

/*******************************************************************
 * print_error();
 */

static void print_error(char *line,
			char *message,
			rview_map_t *map)

{

  fprintf (stderr, "\nERROR - %s:read_map_file\n", Glob->prog_name);
  fprintf (stderr, "Decoding line number %ld in map file %s\n",
	   (long) line_num, map->file_path);
  fprintf (stderr, "%s", line);
  fprintf (stderr, "%s\n", message);
  
}

/*****************
 * read_map_conf()
 *
 * Reads the map configuration file
 *
 * Returns 0 on success, -1 on failure
 */

static int read_map_conf(void)
     
{

  char *conf_file_path;
  char line[BUFSIZ];
  int valid_line;
  int line_num = 0;
  int ntokens;
  FILE *conf_file;
  rview_map_t map;
  GC tmp_gc;
  Colormap cmap;
  
  /*
   * get default color map
   */

  cmap = Glob->cmap;

  /*
   * get conf file path
   */
  
  conf_file_path = xGetResString(Glob->rdisplay, Glob->prog_name,
				 "map_conf_file", "null");
  
  /*
   * return now if map file not set
   */
  
  if (!strcmp(conf_file_path, "null")) {
    return (0);
  }
  
  if (Glob->debug) {
    fprintf(stdout,"Reading map conf in file '%s'\n", conf_file_path);
  }

  /*
   * open conf file
   */

  if ((conf_file = fopen(conf_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:maps - cannot open map conf file\n",
	    Glob->prog_name);
    perror(conf_file_path);
    return (-1);
  }
  
  /*
   * read the file
   */

  while (!feof(conf_file)) {

    if (fgets(line, BUFSIZ, conf_file) != NULL) {
      
      line[strlen(line) - 1] = '\0';
      line_num++;
      valid_line = TRUE;
      
      if (line[0] != '#') {

	/*
	 * substitute env variables
	 */

	usubstitute_env(line, BUFSIZ);

	/*
	 * scan the line
	 */

	ntokens = sscanf(line, "%s%s%d%s%lg%lg%lg%lg%d%d%s",
			 map.label,
			 map.xcolor, &map.xwidth,
			 (char *) map.xfontname,
			 &map.psgc.fontsize, &map.psgc.line_width,
			 &map.psgc.dash_length, &map.psgc.space_length,
			 &map.selected_limited, &map.selected_all,
			 map.file_path);


	if (ntokens <= 0) {

	  /*
	   * blank line 
	   */
	  
	  valid_line = FALSE;

	} else if (ntokens != 11) {
	  
	  fprintf(stderr, "WARNING - %s:maps.c - bad line in map conf file\n",
		  Glob->prog_name);
	  fprintf(stderr, "conf file: %s\n", conf_file_path);
	  fprintf(stderr, "line num: %d\n", line_num);
	  fprintf(stderr, "ntokens: %d\n", ntokens);
	  fprintf(stderr, "%s\n", line);
	  valid_line = FALSE;
	  
	}

	if (valid_line) {
	  
	  Nmaps++;

	  map.nicondefs = 0;
	  map.nicons = 0;
	  map.npolylines = 0;
	  map.nsimplelabels = 0;

	  /*
	   * map gc
	   */
	  
	  tmp_gc = xGetColorGC(Glob->rdisplay, cmap,
			       &Glob->color_index, map.xcolor);

	  if (tmp_gc == NULL) {
	    fprintf(stderr, "ERROR - %s:read_map_conf\n", Glob->prog_name);
	    fprintf(stderr, "Getting GC for color '%s'\n", map.xcolor);
	    tidy_and_exit(1);
	  }

	  map.gc = XCreateGC(Glob->rdisplay,
			     DefaultRootWindow(Glob->rdisplay), 0, 0);

	  XCopyGC(Glob->rdisplay, tmp_gc, GCForeground, map.gc);
	  XSetBackground(Glob->rdisplay, map.gc, Glob->background);

	  XSetLineAttributes(Glob->rdisplay, map.gc, map.xwidth,
			     LineSolid, CapButt, JoinMiter);
	  
	  map.xfont = xLoadFont(Glob->rdisplay, map.xfontname);
	  XSetFont(Glob->rdisplay, map.gc, map.xfont->fid);
	  
	  /*
	   * alloc for map
	   */

	  if (Maps == NULL) {
	    Maps = (rview_map_t *) umalloc(Nmaps * sizeof(rview_map_t));
	  } else {
	    Maps = (rview_map_t *) urealloc((char *) Maps,
					    Nmaps * sizeof(rview_map_t));
	  }

	  /*
	   * copy in
	   */
	  
	  Maps[Nmaps - 1] = map;
	  
	} /* if (valid_line) */
	
      } /* if (line[0] != '#') */

    } /* if (fgets(line, BUFSIZ, conf_file) != NULL) */

  } /* while */
  
  fclose(conf_file);

  return (0);

}

/****************
 * clip_map_gcs()
 */

void clip_map_gcs(int dev, gframe_t *frame,
		  GRectangle *clip_rectangle)

{
  int imap;
  if (Glob->debug) {
    fprintf(stderr, "** clip_map_gcs **\n");
  }
  for (imap = 0; imap < Nmaps; imap++) {
    GSetClipRectangles(dev, frame, Maps[imap].gc, frame->psgc,
		       clip_rectangle, (int) 1);
  }
}

/******************
 * unclip_map_gcs()
 */

void unclip_map_gcs(void)
     
{
  int imap;
  if (Glob->debug) {
    fprintf(stderr, "** unclip_map_gcs **\n");
  }
  for (imap = 0; imap < Nmaps; imap++) {
    XSetClipMask(Glob->rdisplay, Maps[imap].gc, None);
  }
}

  
/****************
 * free_map_gcs()
 */

void free_map_gcs(void)

{
  int imap;
  if (Glob->debug) {
    fprintf(stderr, "** free_map_gcs **\n");
  }
  for (imap = 0; imap < Nmaps; imap++) {
    xFreeGC(Glob->rdisplay, Maps[imap].gc);
  }
}

/******************
 * free_map_fonts()
 */

void free_map_fonts(void)

{
  int imap;
  if (Glob->debug) {
    fprintf(stderr, "** free_map_fonts **\n");
  }
  for (imap = 0; imap < Nmaps; imap++) {
    xFreeFont(Glob->rdisplay, Maps[imap].xfont);
  }
}

