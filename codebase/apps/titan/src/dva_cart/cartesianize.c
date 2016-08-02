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
/***************************************************************************
 * cartesianize.c
 *
 * Perform cartesian transformation. (DVA processing by Marion Mittermaier)
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Nov 1996
 *
 * Upgrades to DVA processing by Marion Mittermaier, BPRP, South Africa
 *
 * Oct 1997
 *
 ****************************************************************************/

#include "dva_cart.h"

/*
 * file scope variables
 */

static ui08 *Fin_dbz;

/*
 * Notes on gridfile struct :
 * 	- (x,y) pairs run from 1 to 300
 * 	- the bins (ii) run from 1 to 224
 * 	- the azimuth a1-a4 are from 1 to 360
 * 	- the elevation numbers (ll) are from 1 to 18, a ZERO is assigned to
 *	  gridpoints with elevations below base scan
 */

static dva_lookup_t *Pos;
static int Pos_nbytes;
static int Nlookup;
static int Radar_id;
static dva_grid_t *Grid;
static dva_rdas_cal_t *Cal;

static int *Val;
static int *Idiff;
static int *Rngcor;

static ui16 ***Ivip;
static si16 **Mvipa;
static si16 **Mvipb;

/*
 * file scope functions
 */

static si16 AVERAGE (si16 *ivip1, si16 *ivip2);

static void CONVERT (ui16 *bcdval, double *binval);

static si32 get_time(bprp_beam_t *beam);

static void HORIZ (int cht);

static int LOAD_FILES (void);

static int PROCESS_CAPPI (void);

static void UNPACK_DATA (int nbeams,
			 bprp_beam_t *beam_array,
			 si32 *start_time_p,
			 si32 *end_time_p);
  
static void VERTIGO (int cht);

/*
 * make BYTESWAP a no-op
 */

#define BYTESWAP(a) (a)

/*****************
 * main subroutine
 */

int cartesianize(int nbeams,
		 bprp_beam_t *beam_array,
		 dva_grid_t *grid,
		 dva_rdas_cal_t *cal,
		 int n_elev,
		 double *elevations)
     
{

  static int first_call = TRUE;
  si32 start_time, end_time;
  
  Grid = grid;
  Cal = cal;

  if (first_call) {

    Nlookup = (grid->nx * grid->ny + 3) / 4;
    
    /*
     * allocate memory
     */

    Pos_nbytes = (Nlookup * sizeof(dva_lookup_t));
    Pos = (dva_lookup_t *) umalloc(Pos_nbytes);
    Fin_dbz = (ui08 *) umalloc(grid->nx * grid->ny);

    Ivip = (ui16 ***) umalloc3(n_elev, 360, Cal->ngates, sizeof(ui16));
    Mvipa = (si16 **) umalloc2(grid->ny, grid->nx, sizeof(si16));
    Mvipb = (si16 **) umalloc2(grid->ny, grid->nx, sizeof(si16));

    /*
     * load files
     */
    
    if (LOAD_FILES()) {
      return (-1);
    }

    /*
     * initialize dobson file index
     */

    init_dobson(Grid, Cal, n_elev, elevations);
    
    first_call = FALSE;

  } /* if (first_call) */

  if (Glob->params.debug) {
    fprintf(stderr, "Start cartesianize\n");
  }

  UNPACK_DATA(nbeams, beam_array, &start_time, &end_time);

  PROCESS_CAPPI();

  write_dobson(Radar_id, start_time, end_time);

  if (Glob->params.debug) {
    fprintf(stderr, "End cartesianize\n");
  }

  system ( "date > /hd/titan5/mrl5/cdata/time_end &");
  return (0);

}

/***************
 * UNPACK_DATA()
 */

static void UNPACK_DATA (int nbeams,
			 bprp_beam_t *beam_array,
			 si32 *start_time_p,
			 si32 *end_time_p)

{
  
  static ui16 azold = 0;
  int  i, n;
  
  ui16 el, az, bcdval, ray;
  ui16 rvpc;            
  ui16 first_fl;
  ui16 rawvip;
  static double binval = 0.0;

  /*
   *  Set flag to store start time.
   */

  first_fl = 1;

  /*
   * set radar id
   */

  Radar_id = (int) (beam_array[0].hdr.xmt & 0x1f);

  /*
   *  Load cappi structure.
   */
  
  for  (i = 0; i < nbeams; i++) {

    /*
     * set times
     */

    if (i == 0) {
      *start_time_p = get_time(beam_array + i);
    }
    if (i == nbeams - 1) {
      *end_time_p = get_time(beam_array + i);
    }
      
    ray = BYTESWAP (beam_array[i].hdr.raycount) & 0x01FF;
    el = (BYTESWAP (beam_array[i].hdr.raycount) & 0xFE00) >> 9;
    
    /*
     * el is the elevation number extracted from the header,
     * which is between 1 and nz.
     */
    
    bcdval = BYTESWAP (beam_array[i].hdr.azimuth);	 
    CONVERT (&bcdval, &binval);
    binval = binval - 0.45 + 0.5;
    az = (ui16) binval;	 
    if (az == 360) az = 0;
    if (az == azold) continue;
    azold = az;
      
    /*
     * range bins run from 0 to 223, az runs from 0 to 359 ( see above) and
     * el from 0 to nz-1 (i.e.subtract 1 below) 
     */
    
    for  (n = 0; n < Cal->ngates; n++) {

      rvpc = BYTESWAP (beam_array[i].vip[n]);
      rawvip = (ui16) (rvpc / 8.);
      Ivip[el-1][az][n] = rawvip + Rngcor[n] - Rngcor[0];

    } /* n */

  } /* i */

}

static si32 get_time(bprp_beam_t *beam)

{

  date_time_t btime;
	
  btime.year = beam->hdr.date / 0x200;
  if (btime.year < 2000) {
    if (btime.year < 50) {
      btime.year += 2000;
    } else {
      btime.year += 1900;
    }
  }
  btime.month = 1;
  btime.day = beam->hdr.date & 0x1ff; /* Julian day */
  btime.hour = beam->hdr.hour;
  btime.min = beam->hdr.min / 60;
  btime.sec = beam->hdr.min % 60;
  
  uconvert_to_utime(&btime);
  uconvert_from_utime(&btime);

  return (btime.unix_time);

}

/***********
 * CONVERT()
 */

static void CONVERT (ui16 *bcdval, double *binval)

{

  /*
   *  Conversion of bcd (binary coded decimal) format to integer.
   */
  
  ui16 val_12, val_8, val_4, val_0;
  
  val_12 = (*bcdval >> 12) * 1000;
  val_8 = ((*bcdval >> 8) & 0x0F) * 100;
  val_4 = ((*bcdval >> 4) & 0x0F) * 10;
  val_0 = *bcdval & 0x0F;
  *binval = (val_12 + val_8 + val_4 + val_0) / 10.;
}

/*****************
 * PROCESS_CAPPI()
 *
 * returns 0 on success, -1 on failure
 */

static int PROCESS_CAPPI (void)

{

  int cht;
  char grid_path[MAX_PATH_LEN];
  FILE *grid_file;
  
  for  (cht = 1; cht < Grid->nz+1; cht++) {

    sprintf (grid_path, "%s%s%s%.2d",
	     Glob->params.lookup_files_dir, PATH_DELIM,
	     "lookup_cappi_", cht-1);
    
    if ((grid_file = fopen (grid_path, "r")) == NULL) {
      fprintf(stderr, "ERROR - %s:PROCESS_CAPPI\n", Glob->prog_name);
      fprintf(stderr, "Cannot open grid file for reading.\n");
      perror(grid_path);
      return (-1);
    }
    
    if (fread(Pos, Pos_nbytes, 1, grid_file) != 1) {
      fprintf(stderr, "ERROR - %s:PROCESS_CAPPI\n", Glob->prog_name);
      fprintf(stderr, "Cannot read grid file.\n");
      perror(grid_path);
      return (-1);
    }
    
    fclose (grid_file);

    if (Glob->params.debug) {
      fprintf (stderr, 
	       "%s - LOAD GRID FILE  %d - successful\n",
	       Glob->prog_name, cht);
    }
    
    HORIZ (cht);

    VERTIGO (cht); 

    load_dobson(cht - 1, Fin_dbz);
    
  } /* cht */

  return (0);
   
}

/*********
 * HORIZ()
 */

static void HORIZ (int cht)

{

  ui16  i, k, n, kk, m, x[4], y[4], iy, ix, az[4];  
  si16  j, ivip1, ivip2, nvip1, nvip2, nvip;
  

  if (Glob->params.debug) {
    fprintf(stderr, "%s - HORIZ - %d\n",
	    Glob->prog_name, cht);
  }

  for (n = 0; n < Nlookup; n++) {

    i = Pos[n].ii - 1;     /* Bin number. */
    j = Pos[n].ll - 1;     /* Elevation step number
				        * from 1 - 18. */

    az[0] = Pos[n].a1; /* Azimuth. */
    az[1] = Pos[n].a2; /* Azimuth. */
    az[2] = Pos[n].a3; /* Azimuth. */
    az[3] = Pos[n].a4; /* Azimuth. */

    if (az[0] == 360) az[0] = 0;

    x[0] = Pos[n].x1 - 1;
    y[0] = Pos[n].y1 - 1;
    x[1] = Pos[n].x2 - 1;
    y[1] = Pos[n].y2 - 1;
    x[2] = Pos[n].x3 - 1;
    y[2] = Pos[n].y3 - 1;
    x[3] = Pos[n].x4 - 1;
    y[3] = Pos[n].y4 - 1;
    
/*
 * For below base scan, break loop
 */

    if (j == -1) continue;
    if (i >= Cal->ngates - 1) continue;
    
    for (m = 0; m < 4; m++) {

      k = az[m];
      ix = x[m];
      iy = y[m];
      
      kk = k + 1;

      if ( kk == 360 ) kk = 0;
      
      /* 
       *  Average #1, azi at bin i+1 - vips offset. 
       */
      
      ivip1 = Ivip[j][kk][i+1] + Pos[n].ad2;
      ivip2 = Ivip[j][k][i+1] + Pos[n].ad1;
      nvip = AVERAGE (&ivip1, &ivip2);
      nvip1 = nvip;
      
      /* 
       *  Average #2 , azi at bin i.
       */
      
      ivip1 = Ivip[j][kk][i] + Pos[n].ad2;
      ivip2 = Ivip[j][k][i]+ Pos[n].ad1;
      nvip = AVERAGE (&ivip1, &ivip2);
      nvip2 = nvip;
      
      /* 
       *  Average #3 , range.
       */
      
      ivip1 = nvip1 + Pos[n].rd2;
      ivip2 = nvip2 + Pos[n].rd1;
      nvip = AVERAGE (&ivip1, &ivip2);
      Mvipa[iy][ix] = nvip; 
      
      /* 
       *  Average #1, azi at bin i+1 - vips offset. 
       *  NEXT ELEVATION.
       */
      
      ivip1 = Ivip[j+1][kk][i+1] + Pos[n].ad2;
      ivip2 = Ivip[j+1][k][i+1] + Pos[n].ad1;
      nvip = AVERAGE (&ivip1, &ivip2);
      nvip1 = nvip;
      
      /* 
       *  Average #2 , azi at bin i.
       */
      
      ivip1 = Ivip[j+1][kk][i] + Pos[n].ad2;
      ivip2 = Ivip[j+1][k][i]+ Pos[n].ad1;
      nvip = AVERAGE (&ivip1, &ivip2);
      nvip2 = nvip;
      
      /* 
       *  Average #3 , range.         
       */
      
      ivip1 = nvip1 + Pos[n].rd2;
      ivip2 = nvip2 + Pos[n].rd1;
      nvip = AVERAGE (&ivip1, &ivip2);
      Mvipb[iy][ix] = nvip; 
      
    } /* m */

  } /* n */
  
}
	
/***********
 * VERTIGO()
 */

static void VERTIGO (int cht)

{
  
  ui16  m, n, x[4], y[4], iy, ix;
  si16  ivip1, ivip2, nvip, mvip;
  
  /*
   * Initializing the CAPPI field to ZERO.
   */

  memset(Fin_dbz, 0, Grid->ny * Grid->nx * sizeof(ui08));
  
  if (Glob->params.debug) {
    fprintf(stderr, "%s - VERTIGO - start\n", Glob->prog_name);
  }

  for (n = 0; n < Nlookup; n++) {

    x[0] = Pos[n].x1 - 1;
    y[0] = Pos[n].y1 - 1;
    x[1] = Pos[n].x2 - 1;
    y[1] = Pos[n].y2 - 1;
    x[2] = Pos[n].x3 - 1;
    y[2] = Pos[n].y3 - 1;
    x[3] = Pos[n].x4 - 1;
    y[3] = Pos[n].y4 - 1;
    
    for (m = 0; m < 4; m++) {

      ix = x[m];
      iy = y[m];
      
      /* 
       *  Average ( 3 horiz + 3 horiz ) + 1 vert = 7.
       */
      
      ivip1 = Mvipa[iy][ix] + Pos[n].ed1;
      ivip2 = Mvipb[iy][ix] + Pos[n].ed2;
      nvip = AVERAGE (&ivip1, &ivip2);
      
      mvip = nvip;

      if (mvip < (Pos[n].noise - Rngcor[0])) {
	Fin_dbz[iy * Grid->nx + ix] = 0;
      } else  {
	Fin_dbz[iy * Grid->nx + ix] =  Val[mvip];
      }

       
    } /* m */
  
  } /* n */
  
}

/***********
 * AVERAGE()
 */

static si16 AVERAGE (si16 *ivip1, si16 *ivip2)

{

  si32 ix, max, min, verskil, nvip;

  if (*ivip1 > *ivip2) 
    {
      verskil =  *ivip1 - *ivip2;
      max = *ivip1;
      min = *ivip2;
    }
  else 
    {
      verskil =  *ivip2 - *ivip1;
      max = *ivip2;
      min = *ivip1;
    }
  
  if (verskil <= 4095)
    {
      ix = verskil;      
    }
  else
    { 
      if (verskil > 4095)
	{
	  ix = 4095;
	}
    }
  
  nvip = max - Idiff[ix];
  return (nvip);

}	

/**************
 * LOAD_FILES()
 * 
 * returns 0 on success, -1 on failure
 */

static int LOAD_FILES (void)

{
  
  int i, testval;
  FILE *fp;
  
  if (Glob->params.debug) {
    fprintf (stderr, "%s - LOAD TABLES\n", Glob->prog_name);
  }

  /*
   *  Vip difference lookup table.
   */
  
  if ((fp = fopen (Glob->params.displace_table_file_path, "r"))
      == NULL) {
    fprintf(stderr, "ERROR - %s:LOAD_FILES1\n", Glob->prog_name);
    perror(Glob->params.displace_table_file_path);
    return (-1);
  }
  
  Idiff = (int *) umalloc (MAXVIP * sizeof(int));

  for (i = 0; i < MAXVIP + 1; i++) {
    fscanf (fp, "%d", &Idiff[i]);
  }

  fclose (fp);

  /*
   *  Range correction table.
   */

  if ((fp = fopen (Glob->params.range_correction_table_file_path, "r"))
      == NULL) {
    fprintf(stderr, "ERROR - %s:LOAD_FILES3\n", Glob->prog_name);
    perror(Glob->params.range_correction_table_file_path);
    return (-1);
  }
  
  Rngcor = (int *) umalloc (Cal->ngates * sizeof(int));
  for (i = 0; i < Cal->ngates; i++) {
    fscanf (fp, "%d", &Rngcor[i]);
  }

  fclose (fp);

  /*
   *  Vip to byte encoding table.
   */
  
  if ((fp = fopen (Glob->params.vip2byte_table_file_path, "r"))
      == NULL) {
    fprintf(stderr, "ERROR - %s:LOAD_FILES2\n", Glob->prog_name);
    perror(Glob->params.vip2byte_table_file_path);
    return (-1);
  }
  
  testval = 4096 - Rngcor[0];
  Val = (int *) umalloc (testval * sizeof(int));
  
  for (i = 0; i < testval; i++) {
    fscanf (fp, "%d", &Val[i]);
  }
  
  fclose (fp);

  return (0);

}

/**************************************
 * old BYTESWAP code - not used for now
 */

#ifdef NOTNOW

BYTESWAP (ui16 *ndata)

{

   #define LSB(x) * (((unsigned char *) &x) + 1)
   #define MSB(x) * ((unsigned char *) &x) 

   ui16 nword;

   MSB (nword) = LSB (ndata);
   LSB (nword) = MSB (ndata);

   return (nword);

}

#endif


