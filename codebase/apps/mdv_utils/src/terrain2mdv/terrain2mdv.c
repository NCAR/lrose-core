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
/*
 * USGS 1 degree Digital Elevation Model -> Zebra netCDF converter
 *
 */

#define MAIN
#include "terrain2mdv.h"
#undef MAIN 
#include <dataport/swap.h>

#define MISSING -999.0
#define LL_FACTOR 1.5
#define MAX_FILES 10000

static time_t U_time;
static date_time_t *File_time;

static double *lat_out, *lon_out, ll_limit;

int main (int argc, char *argv[])
{

  /*
   * basic declarations
   */

  char *params_file_path = NULL;

  int check_params;
  int print_params;

  int	nfiles;
  char *files_path = NULL;
  char  **files;

  char line[200];
  FILE *Filesp;

  path_parts_t progname_parts;
  tdrp_override_t override;

  /*
   * allocate space for the global structure
   */

  Glob = (global_t *)
    umalloc((ui32) sizeof(global_t));

  /*
   * set program name
   */
 
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
 
  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
 

  /*
   * parse command line arguments
   */

  parse_args(argc, argv, &check_params, &print_params,
             &params_file_path, &override, &files_path);

  /*
   * load up parameters
   */
 
  Glob->table = terrain2mdv_tdrp_init(&Glob->params);

  if (FALSE == TDRP_read(params_file_path,
                         Glob->table,
                         &Glob->params,
                         override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Problems with params file '%s'\n",
              params_file_path);
    tidy_and_exit(-1);
  }
 
  TDRP_free_override(&override);
 
  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    tidy_and_exit(0);
  }
 
  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    tidy_and_exit(0);
  }
 
  if ( ( files = (char **) umalloc( MAX_FILES )) == NULL ) {
     fprintf(stderr, "Malloc error\n");
     tidy_and_exit(0);
  }

  if (Glob->params.input_type == USGS ) {
  /*
   * Read in the list of files containing the USGS data
   */

    Filesp = fopen( files_path, "r");
    nfiles=0;
    while (fgets (line, 200, Filesp))
    {
       if ( ( files[nfiles] = (char *) umalloc ((ui32) (strlen(line) + 1))) == NULL ) {
         fprintf(stderr, "Malloc error\n");
         tidy_and_exit(0);
        }
        memset( files[nfiles], '\0',(strlen(line) + 1) );
        strncpy( files[nfiles], line, strlen(line)-1);
/*        strcat( files[nfiles], "");*/
        nfiles++;
    }

    fclose(Filesp);

  } else {

   if ( ( files[0] = (char *) umalloc ((ui32) (strlen(line) + 1))) == NULL ) {
     fprintf(stderr, "Malloc error\n");
     tidy_and_exit(0);
    }
    strncpy( files[0], files_path, strlen(files_path) );
    nfiles = 1;

 }

  if ( Glob->params.output_type == NETCDF ) {
    if (Glob->params.input_type != USGS ) {
      fprintf(stdout,"GTOPO30 with NETCDF is not supported at this time!\n");
      exit(0);
    }
    /*
     * Get the bounds for the file we're generating
     */
    Map.south = (int)(3600 * Glob->params.map_south);
    Map.west = (int)(3600 * Glob->params.map_west);
    Map.north = (int)(3600 * Glob->params.map_north);
    Map.east = (int)(3600 * Glob->params.map_east);

    /*
     * Initialize the output map file
     */
    InitMap (Glob->params.outfname);

    /*
     * Process the input files to extract the terrain data
     */
    ProcessFiles (files, nfiles);
    ncclose (Map.nc_id);

  } else {

    /*
     * Get the bounds for the file we're generating
     */
    Map.south = (int)(3600 * Glob->params.map_south);
    Map.west = (int)(3600 * Glob->params.map_west);
    Map.north = (int)(3600 * Glob->params.map_north);
    Map.east = (int)(3600 * Glob->params.map_east);

    /*
     * Initialize the output map file
     */
    InitMapMdv (Glob->params.outfname);

    /*
     * Process the input files to extract the terrain data
     */
    ProcessFilesMdv (files, nfiles);

  }
  /*
   * quit
   */
 
  tidy_and_exit(0);

  return(0);

}


void
InitMap (char *fname)
/*
 * Open up and initialize our output map file
 */
{
    int		ncid, i, time_dim, lat_dim, lon_dim, lat_var, lon_var;
    int		base_var, offset_var, dims[3], flags;
    long	start, count;
    long	base;
    double	offset;
    char	*fullname = malloc (strlen (fname) + 4);
    char	attrval[128];

    /*
     * Open our netCDF output file, appending ".cdf" to the name if necessary
     */
    strcpy (fullname, fname);
    if (! strstr (fullname,".cdf"))
	strcat (fullname, ".cdf");

    flags = NC_NOCLOBBER;
    if ((ncid = nccreate (fullname, flags)) < 0)
    {
	fprintf (stderr, "Error creating map file %s\n", fullname);
	exit (1);
    }
    
    Map.nc_id = ncid;

/*    free (fullname); */

    /*
     * To match the DEM maps, we use 3 arc second longitude spacing if our
     * north edge is at or south of 50 degrees, 6 arc second if the north
     * edge is between 50 and 70 degrees, and 9 arc second north of 70
     * degrees.  We always use 3 second spacing in latitude. 
     */
    Map.lon_spacing = (Map.north <= (50 * 3600)) ? 3 : 
	              (Map.north <= (70 * 3600)) ? 6 : 9;
    Map.lat_spacing = 3;

    Map.nlons = (int)((Map.east - Map.west) / Map.lon_spacing + 0.5) + 1;
    Map.nlats = (int)((Map.north - Map.south) / Map.lat_spacing + 0.5) + 1;
    /*
     * Adjust spacing if necessary to get us below a 750x750 final map
     */
    if (Map.nlons > Glob->params.output_grid_size)
    {
	Map.lon_spacing *= (1 + Map.nlons / Glob->params.output_grid_size);
	Map.nlons = (int)((Map.east - Map.west) / Map.lon_spacing + 0.5) + 1;
    }
    
    if (Map.nlats > Glob->params.output_grid_size)
    {
	Map.lat_spacing *= (1 + Map.nlats / Glob->params.output_grid_size);
	Map.nlats = (int)((Map.north - Map.south) / Map.lat_spacing + 0.5) + 1;
    }

    printf ("\n");
    printf ("Using lon/lat spacing of %d/%d arc seconds\n", Map.lon_spacing,
	    Map.lat_spacing);
    ll_limit = MAX(Map.lat_spacing/3600.0,Map.lon_spacing/3600.0) * LL_FACTOR;
    printf ("for a resulting map size of %dx%d\n\n", Map.nlons, Map.nlats);
    printf("with a nearest neighbor limit of %f\n\n",ll_limit);

    /*
     * Adjust our north and east edges to exactly match the spacing we just 
     * determined
     */
    Map.north = Map.south + (Map.nlats - 1) * Map.lat_spacing;
    Map.east = Map.west + (Map.nlons - 1) * Map.lon_spacing;

    /*
     * Create our dimensions
     */
    time_dim = ncdimdef (ncid, "time", 1);
    lat_dim = ncdimdef (ncid, "latitude", Map.nlats);
    lon_dim = ncdimdef (ncid, "longitude", Map.nlons);

    /*
     * and our variables
     */
    
    lat_var = ncvardef (ncid, "latitude", NC_FLOAT, 1, &lat_dim);
    strcpy (attrval, "north latitude");
    ncattput (ncid, lat_var, "long_name", NC_CHAR, strlen (attrval) + 1, 
	      attrval);
    strcpy (attrval, "degrees");
    ncattput (ncid, lat_var, "units", NC_CHAR, strlen (attrval) + 1, attrval);
    
    lon_var = ncvardef (ncid, "longitude", NC_FLOAT, 1, &lon_dim);
    strcpy (attrval, "east longitude");
    ncattput (ncid, lon_var, "long_name", NC_CHAR, strlen (attrval) + 1, 
	      attrval);
    strcpy (attrval, "degrees");
    ncattput (ncid, lon_var, "units", NC_CHAR, strlen (attrval) + 1, attrval);

    dims[0] = time_dim;
    dims[1] = lon_dim;
    dims[2] = lat_dim;
    Map.alt_var = ncvardef (ncid, "altitude", NC_SHORT, 3, dims);
    strcpy (attrval, "altitude");
    ncattput (ncid, Map.alt_var, "long_name", NC_CHAR, strlen (attrval) + 1, 
	      attrval);
    strcpy (attrval, "meters");
    ncattput (ncid, Map.alt_var, "units", NC_CHAR, strlen (attrval) + 1, 
	      attrval);

    base_var = ncvardef (ncid, "base_time", NC_LONG, 0, 0);
    strcpy (attrval, "Base time in Epoch");
    ncattput (ncid, base_var, "long_name", NC_CHAR, strlen (attrval) + 1, 
	      attrval);
    strcpy (attrval, "seconds since 1970-1-1 0:00:00 0:00");
    ncattput (ncid, base_var, "units", NC_CHAR, strlen (attrval) + 1, 
	      attrval);

    offset_var = ncvardef (ncid, "time_offset", NC_DOUBLE, 1, &time_dim);
    strcpy (attrval, "Time offset from base_time");
    ncattput (ncid, offset_var, "long_name", NC_CHAR, strlen (attrval) + 1, 
	      attrval);
    strcpy (attrval, "seconds since 1970-1-1 0:00:00 0:00");
    ncattput (ncid, offset_var, "units", NC_CHAR, strlen (attrval) + 1, 
	      attrval);
    /*
     * Get out of definition mode and write our times (we just use zero...)
     */
    ncendef (ncid);

    base = 0;
    offset = 0.0;
    start = 0;
    count = 1;
    
    ncvarput (ncid, base_var, &start, &count, &base);
    ncvarput (ncid, offset_var, &start, &count, &offset);
    /*
     * Write our lats & lons
     */
    for (i = 0; i < Map.nlats; i++)
    {
	float	lat = (float)(Map.south + i * Map.lat_spacing) / 3600.0;

	start = i;
	count = 1;
	ncvarput (ncid, lat_var, &start, &count, &lat);
    }

    for (i = 0; i < Map.nlons; i++)
    {
	float	lon = (float)(Map.west + i * Map.lon_spacing) / 3600.0;

	start = i;
	count = 1;
	ncvarput (ncid, lon_var, &start, &count, &lon);
    }
    /*
     * Done here
     */
    return;
}



void
ProcessFiles (char *fnames[], int nfiles)
{
    int	f, lat_spacing, lon_spacing, ssec, nsec, wsec, esec, file_open_flag;
    int	lat_start, lat_stop, lon_start, lon_stop, lon;
    long	start[3], count[3];
    short	*alts;
    FILE	*infile = 0;
    char in_c;
/*
 * Loop through the input files
 */
    file_open_flag = FALSE;
    for (f = 0; f < nfiles; f++)
    {
        if (file_open_flag) {
	   fclose (infile);
           file_open_flag = FALSE;
        }

        if ( !strcmp (fnames[f], "WAIT") ) {
          /* Send a msg to the screen, and wait for user input - to change the CD */
          printf("Please insert the next CD... press any key to continue");
          fscanf( stdin, "%c", &in_c);
          continue;
        }

	printf ("%s\n", fnames[f]);

	if (! (infile = fopen (fnames[f], "r")))
	{
	    fprintf (stderr, "Error %d opening '%s'\n", errno, fnames[f]);
           file_open_flag = FALSE;
	    continue;
	}

        file_open_flag = TRUE;
	ReadHeader (infile, &lat_spacing, &lon_spacing, &ssec, &nsec, &wsec, 
		    &esec);

	if ((Map.lat_spacing % lat_spacing) || (Map.lon_spacing % lon_spacing))
	{
	    fprintf (stderr, 
		     "lat_spacing or lon_spacing mismatch with file %s!\n",
		     fnames[f]);
	    exit (1);
	}

	/*
	 * Start/stop lats & lons, forced to multiples of the output file's
	 * lat and lon spacings
	 */
	lat_start = (ssec < Map.south) ? Map.south : ssec;
	lat_start = (lat_start / Map.lat_spacing) * Map.lat_spacing;
	
	lat_stop = (nsec > Map.north) ? Map.north : nsec;
	lat_stop = (lat_stop / Map.lat_spacing) * Map.lat_spacing;

	if (lat_start > lat_stop)
	    continue;

	lon_start = (wsec < Map.west) ? Map.west : wsec;
	lon_start = (lon_start / Map.lon_spacing) * Map.lon_spacing;
	
	lon_stop = (esec > Map.east) ? Map.east : esec;
	lon_stop = (lon_stop / Map.lon_spacing) * Map.lon_spacing;
        lon_stop = (lon_stop<0) ? lon_stop-Map.lon_spacing : lon_stop; 

	if (lon_start > lon_stop)
	    continue;

	/*
	 * Loop through the columns in the file
	 */
	for (lon = wsec; lon <= lon_stop; lon += lon_spacing)
	{
	    int	firstneeded, lastneeded, step;

	    if (lon < lon_start || (lon % Map.lon_spacing))
	    {
		ReadColumn (infile, 0, -1, 0);
		continue;
	    }
	    /*
	     * Bounding indices of the lats we need from the next column
	     */
	    firstneeded = (lat_start - ssec) / lat_spacing;
	    lastneeded = (lat_stop - ssec) / lat_spacing;
	    step = Map.lat_spacing / lat_spacing;
	    
            while ( firstneeded < 0 ) {
              firstneeded += lat_spacing;
            }

	    alts = ReadColumn (infile, firstneeded, lastneeded, step);
	    /*
	     * Position and size for the portion of the output file we cover
	     * with this column
	     */
	    start[0] = 0;
	    start[1] = (lon - Map.west) / Map.lon_spacing;
	    start[2] = (lat_start - Map.south) / Map.lat_spacing;

	    count[0]= 1;
	    count[1] = 1;
	    count[2] = (lastneeded - firstneeded) / step + 1;
	    /*
	     * Write the part of the column that goes into the output file
	     */
	    ncvarput (Map.nc_id, Map.alt_var, start, count, alts);
	}
	printf ("\n");
    }
}


void
ReadHeader (FILE *infile, int *lat_spacing, int *lon_spacing, int *ssec,
	    int *nsec, int *wsec, int *esec)
/*
 * Read the "DEM Type A Logical Record" header from the file, returning the
 * latitude spacing, longitude spacing, south, north, west, and east edges,
 * all in integer arc seconds.
 */
{
    char	buf[1024];
    double	s, n, w, e, dlon, dlat;

    fread (buf, 1, sizeof (buf), infile);

/*
 * Grr.  We have to change from D to E notation.
 */
    buf[566] = 'E';	buf[590] = 'E';
    buf[662] = 'E';	buf[686] = 'E';

    sscanf (buf + 546, "%24lg%24lg", &w, &s);
    sscanf (buf + 642, "%24lg%24lg", &e, &n);
    *ssec = s;
    *nsec = n;
    *wsec = w;
    *esec = e;
    
    sscanf (buf + 816, "%12lf%12lf", &dlon, &dlat);
    *lat_spacing = dlat;
    *lon_spacing = dlon;
}


short*
ReadColumn (FILE *infile, int firstneeded, int lastneeded, int step)
{
    char	buf[8192];
    int		i, n, nlats_in;
    static short	lats[1201];

    fread (buf, 1, sizeof (buf), infile);
/*
 * We assume 1201 lats per column (1 degree column, 3 arc second spacing)
 */
    sscanf (buf + 12, "%d", &nlats_in);
    if (nlats_in != 1201)
    {
	fprintf (stderr, "Eek! Column does not have 1201 entries!\n");
	exit (1);
    }
/*
 * Read out the needed lats
 */
    for (i = firstneeded, n = 0; i <= lastneeded; i += step, n++)
    {
	int	offset;
	/*
	 * Offset to the desired lat.  144 byte heading, 6 bytes per lat, plus
	 * four spaces of filler at the end of each 1024 byte segment.
	 */
	offset = 144 + 6 * i;
	offset += (offset / 1024) * 4;

	sscanf (buf + offset, "%6hd", lats + n);
    }

    return lats;
}

void
InitMapMdv (char *fname)
/*
 * initialize our MDV struct for the output map file
 */
{

    double lat, lon, x,y;
    int ix,iy, index, i;

    if ( Glob->params.proj == FLAT ) {

      PJGflat_init(Glob->params.lat, Glob->params.lon, 0.0);
      PJGflat_xy2latlon( Glob->params.xmin,Glob->params.ymin, &lat,&lon);       
      /* Convert to arc seconds */
      Map.south = lat*3600.0;
      Map.west  = lon*3600.0;
      PJGflat_xy2latlon( Glob->params.xmax,Glob->params.ymax, &lat, &lon);       
      Map.north = lat*3600.0;
      Map.east = lon*3600.0;
      if ( Glob->params.input_type == USGS ) {
        /*
         * To match the DEM maps, we use 3 arc second longitude spacing if our
         * north edge is at or south of 50 degrees, 6 arc second if the north
         * edge is between 50 and 70 degrees, and 9 arc second north of 70
         * degrees.  We always use 3 second spacing in latitude.
         */
        Map.lon_spacing = (Map.north <= (50 * 3600)) ? 3 :
                          (Map.north <= (70 * 3600)) ? 6 : 9;
        Map.lat_spacing = 3;

      } else {
         /*
          * The Global data is 30 arc second data
          */

         Map.lat_spacing = Glob->params.tile_delta;
         Map.lon_spacing = Glob->params.tile_delta;
      }

      Map.nlons = (int)((Map.east - Map.west) / Map.lon_spacing + 0.5) + 1;
      Map.nlats = (int)((Map.north - Map.south) / Map.lat_spacing + 0.5) + 1;
      /*
       * Adjust spacing if necessary to get us below a 750x750 final map
       */
      if (Map.nlons > Glob->params.output_grid_size)
      {
          Map.lon_spacing *= (1 + Map.nlons / Glob->params.output_grid_size);
          Map.nlons = (int)((Map.east - Map.west) / Map.lon_spacing + 0.5) + 1;
      }
   
      if (Map.nlats > Glob->params.output_grid_size)
      {
          Map.lat_spacing *= (1 + Map.nlats / Glob->params.output_grid_size);
          Map.nlats = (int)((Map.north - Map.south) / Map.lat_spacing + 0.5) + 1;
      }

      PJGLatLon2DxDy(Glob->params.lat,Glob->params.lon,
                     Glob->params.lat+Map.lat_spacing/3600.0,Glob->params.lon+Map.lon_spacing/3600.0,
                     &Glob->dx, &Glob->dy);
      Glob->nx = (Glob->params.xmax - Glob->params.xmin) / Glob->dx;
      Glob->ny = (Glob->params.ymax - Glob->params.ymin) / Glob->dy;

      if ( (lon_out= (double *) umalloc (Glob->nx* Glob->ny*sizeof(double)) ) == NULL ) {
         fprintf(stderr, "Malloc error\n");
         tidy_and_exit(0);
        }
      if ( (lat_out= (double *) umalloc (Glob->nx* Glob->ny*sizeof(double)) ) == NULL ) {
         fprintf(stderr, "Malloc error\n");
         tidy_and_exit(0);
        }
      for ( i=0; i<Glob->nx*Glob->ny; i++){
         lon_out[i]=MISSING;
         lat_out[i]=MISSING;
      }

      /* This is an XY projection with a specified center lat/lon.  
         Compute the corresponding lat/lon for each XY point, 
      */
      index=0;
      for ( iy = 0; iy < Glob->ny; iy++) {
        for ( ix = 0; ix < Glob->nx; ix++) {
          y = Glob->params.ymin + iy * Glob->dy;
          x = Glob->params.xmin + ix * Glob->dx;
          PJGflat_xy2latlon( x, y, &lat,&lon);       
          lat_out[index] = lat;
          lon_out[index] = lon;
          index++;
        }
      }

    } else {
         
      if ( Glob->params.input_type == USGS ) {
      /*
       * To match the DEM maps, we use 3 arc second longitude spacing if our
       * north edge is at or south of 50 degrees, 6 arc second if the north
       * edge is between 50 and 70 degrees, and 9 arc second north of 70
       * degrees.  We always use 3 second spacing in latitude.
       */
      Map.lon_spacing = (Map.north <= (50 * 3600)) ? 3 :
                        (Map.north <= (70 * 3600)) ? 6 : 9;
      Map.lat_spacing = 3;

      } else {
         /*
          * The Global data is 30 arc second data
          */

         Map.lat_spacing = Glob->params.tile_delta;
         Map.lon_spacing = Glob->params.tile_delta;

      }

      Map.nlons = (int)((Map.east - Map.west) / Map.lon_spacing + 0.5) + 1;
      Map.nlats = (int)((Map.north - Map.south) / Map.lat_spacing + 0.5) + 1;
      /*
       * Adjust spacing if necessary to get us below a 750x750 final map
       */
      if (Map.nlons > Glob->params.output_grid_size)
      {
          Map.lon_spacing *= (1 + Map.nlons / Glob->params.output_grid_size);
          Map.nlons = (int)((Map.east - Map.west) / Map.lon_spacing + 0.5) + 1;
      }
   
      if (Map.nlats > Glob->params.output_grid_size)
      {
          Map.lat_spacing *= (1 + Map.nlats / Glob->params.output_grid_size);
          Map.nlats = (int)((Map.north - Map.south) / Map.lat_spacing + 0.5) + 1;
      }

    }

    ll_limit = MAX(Map.lat_spacing/3600.0,Map.lon_spacing/3600.0) * LL_FACTOR;
    if( Glob->params.proj == FLAT ) {
      printf ("\n");
      printf ("Using X/Y spacing of %f by %f km\n", Glob->dx, Glob->dy );
      printf ("for a resulting map size of %dx%d\n\n", Glob->nx, Glob->ny);
    } else {
      printf ("\n");
      printf ("Using lon/lat spacing of %d/%d arc seconds\n", Map.lon_spacing,
              Map.lat_spacing);
      printf ("for a resulting map size of %dx%d\n\n", Map.nlons, Map.nlats);
    }
    printf("with a nearest neighbor limit of %f\n\n",ll_limit);


  /*
   * load the MDV headers
   */

  U_time = time(&U_time);
  LoadMdvFieldHdr(U_time);
  LoadMdvMasterHdr(U_time);

}
void
ProcessFilesMdv (char *fnames[], int nfiles)
{
    int f, lat_spacing, lon_spacing, ssec, nsec, wsec, esec;
    int lat_start, lat_stop, lat, lon_start, lon_stop, lon;
    long        count;
    short       *alts;
    FILE        *infile = 0;
    static ui08 *plane, *plane_xy;
    static double *lat_in;
    static double *lon_in;
    static double *alt_in;
    int index, lat0, lon0, i,j, lat_index, lon_index, alt_index, ix,iy, new_lats, new_lons;
    double la,lo;
    char in_c;



/*
 * allocate space for the data plane
 */
  
  if ( Glob->params.proj == FLAT ) {
    if ( (lat_in= (double *) umalloc (Map.nlats* Map.nlons* sizeof(double)) ) == NULL ) {
         fprintf(stderr, "Malloc error\n");
         tidy_and_exit(0);
        }
    memset(lat_in, 0, Map.nlats * Map.nlons);
    if ( (lon_in= (double *) umalloc (Map.nlats* Map.nlons* sizeof(double))) == NULL ) {
         fprintf(stderr, "Malloc error\n");
         tidy_and_exit(0);
        }
    memset(lon_in, 0, Map.nlats * Map.nlons);
    if ( (alt_in= (double *) umalloc (Map.nlats* Map.nlons* sizeof(double))) == NULL ) {
         fprintf(stderr, "Malloc error\n");
         tidy_and_exit(0);
        }
    memset(alt_in, 0, Map.nlats * Map.nlons);
    for ( i=0; i<Map.nlats*Map.nlons; i++){
       lon_in[i]=MISSING;
       lat_in[i]=MISSING;
    }
    if ( (plane_xy = (u_char *) umalloc (Glob->nx* Glob->ny) ) == NULL ) {
         fprintf(stderr, "Malloc error\n");
         tidy_and_exit(0);
        }
    memset(plane_xy, 0, Glob->nx * Glob->ny);
  
  } else {
    if ( (plane = (u_char *) umalloc (Map.nlats* Map.nlons) ) == NULL ) {
         fprintf(stderr, "Malloc error\n");
         tidy_and_exit(0);
        }
    memset(plane, 0, Map.nlats * Map.nlons);
  }
  
/*
 * Loop through the input files
 */
    for (f = 0; f < nfiles; f++)
    {
        if (infile)
            fclose (infile);

        if ( !strcmp (fnames[f], "WAIT") ) {
          /* Send a msg to the screen, and wait for user input - to change the CD */
          printf("Please insert the next CD... press any key to continue");
          fflush(stdout);
          fscanf( stdin, "%c", &in_c);
          continue;
        } 

        printf ("%s\n", fnames[f]);
        fflush(stdout);

        if (! (infile = fopen (fnames[f], "r")))
        {
            fprintf (stderr, "Error %d opening '%s'\n", errno, fnames[f]);
            continue;
        }

        if ( Glob->params.input_type == USGS ) {

          ReadHeader (infile, &lat_spacing, &lon_spacing, &ssec, &nsec, &wsec,
                      &esec);

          if ((Map.lat_spacing % lat_spacing) || (Map.lon_spacing % lon_spacing))
          {
              fprintf (stderr,
                       "lat_spacing or lon_spacing mismatch with file %s!\n",
                       fnames[f]);
              exit (1);
          }


          /*
           * Start/stop lats & lons, forced to multiples of the output file's
           * lat and lon spacings
           */
          lat_start = (ssec < Map.south) ? Map.south : ssec;
          lat_start = (lat_start / Map.lat_spacing) * Map.lat_spacing;

          lat_stop = (nsec > Map.north) ? Map.north : nsec;
          lat_stop = (lat_stop / Map.lat_spacing) * Map.lat_spacing;
  
          if (lat_start > lat_stop)
              continue;

          lon_start = (wsec < Map.west) ? Map.west : wsec;
          lon_start = (lon_start / Map.lon_spacing) * Map.lon_spacing;
    
          lon_stop = (esec > Map.east) ? Map.east : esec;
          lon_stop = (lon_stop / Map.lon_spacing) * Map.lon_spacing;
          lon_stop = (lon_stop<0) ? lon_stop-Map.lon_spacing : lon_stop; 
  
          if (lon_start > lon_stop)
              continue;
  
          /* Find the starting lat/lon indices into the mdv file */
          lat0 = (MAX(lat_start,Map.south) - Map.south) / Map.lat_spacing ; 
          lon0 = (MAX(lon_start,Map.west) - Map.west ) / Map.lon_spacing ;
          lon_index = lon0;
          /*
           * Loop through the columns in the file
           */

          new_lats = TRUE;
          for (lon = wsec; lon <= lon_stop; lon += lon_spacing)
          {
              int firstneeded, lastneeded, step;

              if (lon < lon_start || (lon % Map.lon_spacing))
              {
                  ReadColumn (infile, 0, -1, 0);
                  continue;
              }

              /*
               * Bounding indices of the lats we need from the next column
               */
              firstneeded = (lat_start - ssec) / lat_spacing;
              lastneeded = (lat_stop - ssec) / lat_spacing;
              step = Map.lat_spacing / lat_spacing;
  
              while ( firstneeded < 0 ) {
                firstneeded += lat_spacing;
              }
              alts = ReadColumn (infile, firstneeded, lastneeded, step);
  
              if ( Glob->params.proj == FLAT ) {
  
                count = (lastneeded - firstneeded) / step + 1;
                index = (lat0 * Map.nlons) + lon_index;
                lat_index = lat0;
                for ( j = 0; j < count; j++ ) {
                  alt_in[index] = (unsigned char )((alts[j] - Glob->params.bias) / Glob->params.scale);

		  /* This next bit was added by Niles to stop low lying areas */
		  /* getting a zero elevation and being mistaken for sea. */

		  if ((alt_in[index]==0) && (alts[j] > 0.005)) {
		    alt_in[index]=1; /* Move up to next byte value. */
		  }

		  /* End of Niles additions. */

                  if ( new_lats == TRUE) {
                    lat_in[lat_index] = (lat_start + j * Map.lat_spacing)/3600.0;
                    lat_index++;
                  }
                  index += Map.nlons;
                }
                lon_in[lon_index] = lon/3600.0;
                lon_index++;
                new_lats = FALSE;

              } else {
                /* For LATLON projection, use the data as is */
                /*
                 * Position and size for the portion of the output file we cover
                 * with this column - rotate to be row-major order for the mdv file
                 */
                count = (lastneeded - firstneeded) / step + 1;
                index = lat0;
                for ( j = 0; j < count; j++ ) {
                  plane[index] = (unsigned char )((alts[j] - Glob->params.bias) / Glob->params.scale);

		  /* This next bit was added by Niles to stop low lying areas */
		  /* getting a zero elevation and being mistaken for sea. */

		  if ((plane[index]==0) && (alts[j] > 0.005)) {
		    plane[index]=1; /* Move up to next byte value. */
		  }

		  /* End of Niles additions. */


                  index += Map.nlons;
                }
                lat0++;
              }
          }
          printf ("\n");

    } else {

      /* Global data file */
          lat_spacing = Glob->params.tile_delta;
          lon_spacing = Glob->params.tile_delta;
          ssec = Glob->params.tile_lat*3600.0;
          wsec = Glob->params.tile_lon*3600.0;
          nsec = ssec + lat_spacing * Glob->params.tile_nlats;
          esec = wsec + lon_spacing * Glob->params.tile_nlons;

          if ((Map.lat_spacing % lat_spacing) || (Map.lon_spacing % lon_spacing))
          {
              fprintf (stderr,
                       "lat_spacing or lon_spacing mismatch with file %s!\n",
                       fnames[f]);
              exit (1);
          }


          /*
           * Start/stop lats & lons, forced to multiples of the output file's
           * lat and lon spacings
           */
          lat_start = (ssec < Map.south) ? Map.south : ssec;
          lat_start = (lat_start / Map.lat_spacing) * Map.lat_spacing;

          lat_stop = (nsec > Map.north) ? Map.north : nsec;
          lat_stop = (lat_stop / Map.lat_spacing) * Map.lat_spacing;
 
          if (lat_start > lat_stop)
              continue;

          lon_start = (wsec < Map.west) ? Map.west : wsec;
          lon_start = (lon_start / Map.lon_spacing) * Map.lon_spacing;
   
          lon_stop = (esec > Map.east) ? Map.east : esec;
          lon_stop = (lon_stop / Map.lon_spacing) * Map.lon_spacing;
          lon_stop = (lon_stop<0) ? lon_stop-Map.lon_spacing : lon_stop;
 
          if (lon_start > lon_stop)
              continue;
 
          /* Find the starting lat/lon indices into the mdv file */
          lat0 = (MIN(lat_stop,Map.north) - Map.south) / Map.lat_spacing ;
          lat_index = lat0;
          lat0 = ((lat0-1) * Map.nlons);
          lon0 = (MAX(lon_start,Map.west) - Map.west ) / Map.lon_spacing ;
          lon_index = lon0;

          /*
           * Loop through the rows in the file
           */

          index = lat0;
          new_lons = TRUE;
          for (lat = nsec; lat >= lat_start; lat -= lat_spacing)
          {
              int firstneeded, lastneeded, step;
              if (lat > lat_stop  || (lat % Map.lat_spacing))
              {
                  ReadRow (infile, 0, -1, 0);
                  continue;
              }

              /*
               * Bounding indices of the lons we need from the next row
               */
              firstneeded = (lon_start - wsec) / lon_spacing;
              lastneeded = (lon_stop - wsec) / lon_spacing;
              step = Map.lon_spacing / lon_spacing;

              while ( firstneeded < 0 ) {
                firstneeded += lon_spacing;
              }
              alts = ReadRow(infile, firstneeded, lastneeded, step);

              if ( Glob->params.proj == FLAT ) {

                count = (lastneeded - firstneeded) / step + 1;
                index = lat0 + lon_index;
                lon_index = lon0;
                for ( j = 0; j < count; j++ ) {
                  if ( alts[j] == -9999 ) {
                    if ( Glob->params.missing_is_zero ) {
                      alt_in[index] = 0;
                    } else {
                      alt_in[index] = BAD_VALUE;
                    }
                  } else {
                  alt_in[index] = (unsigned char )((alts[j] - Glob->params.bias) / Glob->params.scale);
                  }

		  /* This next bit was added by Niles to stop low lying areas */
		  /* getting a zero elevation and being mistaken for sea. */

		  if ((alt_in[index]==0) && (alts[j] > 0.005)) {
		    alt_in[index]=1;
		  }

		  /* End of Niles additions. */

                  if ( new_lons == TRUE) {
                    lon_in[lon_index] = (lon_start + j * Map.lon_spacing)/3600.0;
                    lon_index++;
                  }
                  index++;
                }
                lat_in[lat_index] = lat/3600.0;
                lat_index--;
                lat0 -= Map.nlons;
                new_lons = FALSE;

              } else {
                /* For LATLON projection, use the data as is */
                /*
                 * Position and size for the portion of the output file we cover
                 * with this column - rotate to be row-major order for the mdv file
                 */
                count = (lastneeded - firstneeded) / step + 1;
                index=lat0;
                for ( j = 0; j < count; j++ ) {
                  if ( alts[j] != Glob->params.input_bad_value ) {
                    plane[index] = (unsigned char )((alts[j] - Glob->params.bias) / Glob->params.scale);

		    /* This next bit was added by Niles to stop low lying areas */
		    /* getting a zero elevation and being mistaken for sea. */

		    if ((plane[index]==0) && (alts[j] > 0.005)) {
		      plane[index]=1; /* Move up to next byte value. */
		    }
		    
		    /* End of Niles additions. */

                  } else {
                   plane[index] = 0;
                  }
                  index++;
                }
                lat0 -= Map.nlons;
              }
          }
          printf ("\n");
      }


    }
  
    if ( Glob->params.proj == FLAT ) {
      /* We need to remap the lat/lon/alt data to x/y/alt */

      index = 0;
      for ( iy = 0; iy < Glob->ny; iy++) {
        for ( ix = 0; ix < Glob->nx; ix++) {

          lat_index=0;
            while (( lat_out[index] > lat_in[lat_index] && lat_index < Map.nlats )
                || ( lat_in[lat_index] == MISSING && lat_index < Map.nlats) )
                lat_index++;
            lon_index=0;
            while ( (lon_out[index] > lon_in[lon_index] && lon_index < Map.nlons )
                 || (lon_in[lon_index] == MISSING && lon_index < Map.nlons) )
                lon_index++;

            la = fabs(lat_out[index]-lat_in[lat_index]);
            lo = fabs(lon_out[index]-lon_in[lon_index]);
            if ( la < ll_limit && lo < ll_limit ) {

              alt_index = lat_index*Map.nlons + lon_index;
              plane_xy[index] = alt_in[alt_index];
        fflush(stderr);
            } else {
              plane_xy[index] = BAD_VALUE;
        fflush(stderr);
            }

          index++;
        }
      }
      /*
       * write the mdv file
       */                       

      File_time = udate_time(U_time);
      WriteMdv(File_time, plane_xy);

    } else {
      /*
       * write the mdv file
       */                       

      File_time = udate_time(U_time);
      WriteMdv(File_time, plane);

    }

}


short*                                                             
ReadRow (FILE *infile, int firstneeded, int lastneeded, int step)
{
    static short buf[4800]; 
    int         i, n, nlats_in;
    static short        lats[4800];                                     
             
/*                                                             
 * We assume 4800 lons per row (40 degree column, 30 arc second spacing)
 */
    nlats_in = fread (buf, 2, 4800, infile);
    if (nlats_in != 4800)                
    {
        fprintf (stderr, "Eek! Row does not have 4800 entries!\n");
        tidy_and_exit (1);
    }
/*                                                                          
 * Read out the needed lats                                            
 */
    n=0;
    for (i = firstneeded, n = 0; i <= lastneeded; i += step, n++)
    {
        lats[n] = buf[i];
    }

    i = SWAP_array_16(lats, n*2);
    if ( i != n*2 ) {
      printf("Error swapping bytes from input data \n");
      tidy_and_exit(1);
    }

    return lats;
}

