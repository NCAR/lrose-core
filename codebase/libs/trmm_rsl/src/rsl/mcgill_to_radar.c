/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            Mike Kolander
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/*******************************************************************
 *  Ingest a Mcgill format file and fill a RSL Radar structure 
 *  with the data. 
 *
 * Object code from this file must be linked with the following libraries:
 *     librsl  libmcg
 *  
 *-----------------------------------------------------------------
 * NOTE: Adjust Mcgill dbz bias in function RayFill() .
 * Presently, if (Mcgill_dbz_value != 0) 
 *            then Mcgill_dbz_value + 16.5 is stored in RSL Ray.
 *            if (Mcgill dbz value == 0)
 *            then 0 is stored in RSL Ray.
 * I don't find the Mcgill documentation very clear in this matter.
 *-----------------------------------------------------------------
 * Presently sets radar_head->radar_type = DARWIN_TOGA_SIGMET
 * (Need a radar_type defined in rsl.h for PAFB or MCGILL)
 *----------------------------------------------------------------
 * Minor changes will be required in the associated Mcgill library to 
 * accomodate Mcgill data from Sao Paulo. Since I don't have any files 
 * from Sao Paulo to test, I haven't incorporated any such changes.
 *-------------------------------------------------------------------
 * Functions defined in this file:
 *
 * void RayFill(Ray *rsl_ray, mcgRay_t *mcg_ray);
 * void Ray_headerInit(Ray *ray, mcgHeader_t *head, 
 *                    mcgRay_t *mcg_ray, int ray_num, int num_bins_rsl);
 * void Sweep_headerInit(Sweep *sweep, mcgRay_t *mcg_ray, 
 *                       int nrays);
 * void Volume_headerInit(Volume *volume, short vol_scan_format);
 * void Radar_headerInit(Radar *radar, mcgHeader_t *mcg_head);
 * Radar *RSL_mcgill_to_radar(char *infile);
 *
 *   Kolander
 *   09 May 95
 *
 *******************************************************************/

#include <string.h>
#include <stdlib.h>
#include "mcgill.h"
#include <trmm_rsl/rsl.h>

#define MAX_RAYS   512
#define MAX_SWEEPS  32
#define MISSING_VAL 0
#define MCG_DBZ_BIAS 16.5
#define MCG_NOISE_BIAS 0.0

extern int radar_verbose_flag;

/*********************** Function Prototypes ***************************/
static void RayFill(Ray *rsl_ray, mcgRay_t *mcg_ray);
static void Ray_headerInit(Ray *ray, mcgHeader_t *head, 
		    mcgRay_t *mcg_ray, int ray_num, int num_bins_rsl);
static void Sweep_headerInit(Sweep *sweep, mcgRay_t *mcg_ray, int nrays);
static void Volume_headerInit(Volume *volume, short vol_scan_format);
static void Radar_headerInit(Radar *radar, mcgHeader_t *mcg_head);
Radar *RSL_mcgill_to_radar(char *infile);

static float (*f)(Range x);
static Range (*invf)(float x);

/* Fixed number_of_RSL_bins for Mcgill sweeps. Note that
   the number of bins in the RSL structure differs from
   the number of bins in the Mcgill ray stucture, since
   the km/bin spacing differs. SEE MCGILL DOCUMENTATION */
static int num_bins_normal[24] =
   {
   /* PAFB volume scan formats 1 and 3 (normal mode) */
   480, 240, 240, 240, 240, 240, 240, 240, 240, 240, 
   240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
   240, 240, 240, 240
   };
static int num_bins_compressed[24] =
   {
   /* PAFB volume scan formats 2 and 4 (compressed mode) */
   240, 120, 120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
   120, 120, 120, 120
   };


/********************************************************************/
void RayFill(Ray *rsl_ray, mcgRay_t *mcg_ray)
/********************************************************************/
   /* transfer ray data values from the mcgRay structure
	  to the radar->v[]->sweep[]->ray[] structure  */
   {
   int j, mcg_bin; 
   int rsl_bin=0;
   int index=1;    /* Used to accomodate varying Mcgill bin spacing */

   if (rsl_ray == NULL) return;

   /* Note that the number of bins in the Mcgill ray structure
	  is different from the number of bins in the RSL ray structure,
	  since the Mcgill bin spacing changes with range. RSL ray bin 
	  spacing is constant.
	  ---------------------------------------------------------
      MCGILL NORMAL FORMAT:
		 The first 120 mcg_bins have a length of 1 km, and span the 
		 the range [0, 120] km.
		 Mcg_bins 120 to 180 have a length of 2 km, and span the 
		 range [120, 240] km.
		 Mcg_bins 180 to 240 have a length of 4 km, and span the
		 range [240, 480] km.
	  ---------------------------------------------------------
	  MCGILL COMPRESSED FORMAT:
		 The first 120 mcg_bins have a length of 2 km, and span the
		 range [0, 240] km.
		 Mcg_bins 120 to 180 have a length of 4 km, and span the
		 range [240, 480] km.
   */
   for (mcg_bin=0; mcg_bin<mcg_ray->num_bins; mcg_bin++)
	  {
	  /* I do the case dbz_value of zero separately, so produced
		 color images have a black background. Otherwise, if >>all<<
		 dbz values are biased by 16.5, we get a colored background
		 where there is no precipitation. Looks peculiar. Somebody
		 knowledgable should look into this matter. */
	  if (mcg_ray->data[mcg_bin] == 0)
		 {
		 for (j=0; j<index; j++)
			{
			rsl_ray->range[rsl_bin] = invf(MCG_NOISE_BIAS);
			rsl_bin += 1;
			}
		 }
	  else
		 {
		 for (j=0; j<index; j++)
			{
			rsl_ray->range[rsl_bin] = invf(mcg_ray->data[mcg_bin] + 
										   MCG_DBZ_BIAS);
			rsl_bin += 1;
			}
		 }  /* end else */
	  /* Mcgill bin spacing changes at bins 120 and 180 */
	  if ((mcg_bin == 119) || (mcg_bin == 179))
		 {
		 /* In normal format, put range rings at 120km and 240km.
			In compressed format, put range rings at 240km and 480km */
/*		 rsl_ray->range[rsl_bin-1] = RSL_INVF(60.0); */
		 index = index*2;  /* Set up new bin spacing index */
		 }
	  }  /* end for (mcg_bin=0...*/
   }


/*************************************************************************/
void Ray_headerInit(Ray *ray, mcgHeader_t *head, mcgRay_t *mcg_ray,
					int ray_num, int num_bins_rsl)
/*************************************************************************/
   {
   if (ray == NULL) return;
   ray->h.month = (int)head->month;
   ray->h.day = (int)head->day;
   ray->h.year = 1900 + (int)head->year;
   if (ray->h.year < 1980) ray->h.year += 100; /* Year > 2000 */
   ray->h.hour = (int)head->hour;
   ray->h.minute = (int)head->min;
   ray->h.sec = (float)head->sec;
   ray->h.unam_rng = MISSING_VAL;           /***** ?? ******/
   ray->h.azimuth = mcg_ray->azm;
   ray->h.ray_num = ray_num;
   ray->h.elev = mcg_ray->elev;
   ray->h.elev_num = mcg_ray->sweep_num;
   ray->h.range_bin1 = 0;
   switch (head->vol_scan_format)
	  {
	case 1: 
	  ray->h.gate_size = 1000;  /* 1 km/bin */
	  ray->h.fix_angle = 24;    /* 24 sweeps */
	  break;
	case 2: 
	  ray->h.gate_size = 2000;  /* 2 km/bin */
	  ray->h.fix_angle = 24;    /* 24 sweeps */
	  break;
	case 3: 
	  ray->h.gate_size = 1000;  /* 1 km/bin */
	  ray->h.fix_angle = 12;    /* 12 sweeps */
	  break;
	case 4: 
	  ray->h.gate_size = 2000;  /* 2 km/bin */
	  ray->h.fix_angle = 12;    /* 12 sweeps */
	  break;
	default:
	  break;
	  }  /* end switch */
   ray->h.vel_res = MISSING_VAL;           /* ?? */
   ray->h.sweep_rate = MISSING_VAL;        /* ?? */
   ray->h.prf = MISSING_VAL;
   ray->h.azim_rate = MISSING_VAL;         /* ?? */
   ray->h.pulse_count = MISSING_VAL;       /* ?? */
   ray->h.pulse_width = MISSING_VAL;       /* ?? */
   ray->h.frequency = MISSING_VAL;         /* ?? */
   ray->h.wavelength = MISSING_VAL;        /* ?? */
   ray->h.nyq_vel = MISSING_VAL;           /* ?? */
   ray->h.nbins = num_bins_rsl;
   ray->h.f = f;
   ray->h.invf = invf;
   
   }
	  


/*********************************************************************/
void Sweep_headerInit(Sweep *sweep, mcgRay_t *mcg_ray, int nrays)
/*********************************************************************/
   /* Arrive here _after_ sweep ray data has been filled in. */
   {
   if (radar_verbose_flag)
      fprintf(stderr,"sweep_num:%02d  num_rays:%d\n",mcg_ray->sweep_num, nrays);
   if (sweep == NULL) return;
   sweep->h.sweep_num = mcg_ray->sweep_num;
   sweep->h.elev = mcg_ray->elev;
   sweep->h.beam_width = 2.0;  /* From Dennis' function mcg2uf(). 
								     I don't know where he got it. */
   sweep->h.horz_half_bw = 1.0;
   sweep->h.vert_half_bw = 1.0;
   sweep->h.nrays = nrays;
   sweep->h.f = f;
   sweep->h.invf = invf;
   }



/************************************************************************/
void Volume_headerInit(Volume *volume, short vol_scan_format)
/************************************************************************/
   {
   if (volume == NULL) return;
   volume->h.type_str = strdup("Reflectivity");
   volume->h.f = f;
   volume->h.invf = invf;

   switch (vol_scan_format)
	  {
	case 1: case 2:
	  volume->h.nsweeps = 24;
	  break;
	case 3: case 4:
	  volume->h.nsweeps = 12;
	  break;
	default:
	  break;
	  }
   }


/***********************************************************************/
void Radar_headerInit(Radar *radar, mcgHeader_t *mcg_head)
/***********************************************************************/
   {
   radar->h.month = (int)mcg_head->month;
   radar->h.day = (int)mcg_head->day;
   radar->h.year = 1900 + (int)mcg_head->year;
   radar->h.hour = (int)mcg_head->hour;
   radar->h.minute = (int)mcg_head->min;
   radar->h.sec = (float)mcg_head->sec;
   strcpy(radar->h.radar_type, "mcgill");
   radar->h.nvolumes = 1;  /* Mcgill contains only refl data */
   radar->h.number = MISSING_VAL;         /****************/
   strcpy(radar->h.name, "PAFB");
   strcpy(radar->h.radar_name, "MCGILL");
   strcpy(radar->h.city, "MELB");
   strcpy(radar->h.state, "FL");
   radar->h.latd = 28;
   radar->h.latm = 15;
   radar->h.lats = 19;
   radar->h.lond = -80;
   radar->h.lonm = -36;
   radar->h.lons = -21;
   radar->h.height = 0;
   radar->h.spulse = MISSING_VAL;  /* ns */    /*************/
   radar->h.lpulse = MISSING_VAL;  /* ns */    /*************/
   }



/*********************************************************************/
Radar *RSL_mcgill_to_radar(char *infile)
/*********************************************************************/
   {
   /* Ingest a Mcgill format radar data file and fill a Radar RSL
	  structure with the data. 
   */
   int ray_num, code;
   int *num_bins_rsl;
   mcgFile_t *file;
   Radar *radar;
   mcgRay_t *mcg_ray, *mcg_ray_last, *swap;
   extern int *rsl_qsweep; /* See RSL_read_these_sweeps in volume.c */
   extern int rsl_qsweep_max;
   
   /* If no filename has been passed, there's nothing to do. */
   if (infile == NULL) 
      return NULL;

   /* Default conversion functions. */
   f = DZ_F;
   invf = DZ_INVF;

   /* Create a structure of type Radar */
   radar = (Radar *)RSL_new_radar(MAX_RADAR_VOLUMES);
   if (radar == NULL) {
	 perror("RSL_mcgill_to_radar: RSL_new_radar:");
	 return NULL;
   }


   /* Open the Mcgill data file and read Mcgill file header into the 
	  mcgFile.head structure */
   file = (mcgFile_t *)mcgFileOpen(&code, infile);
   if (file == NULL)
      goto quit;
   
   if (radar_verbose_flag)
	  {
	  fprintf(stderr,"Input file:  %s\n", infile);
	  fprintf(stderr,"Scan_date: %d/%d/%d\n", file->head.month, file->head.day, 
			 file->head.year);
	  fprintf(stderr,"Scan_time: %d:%d:%d\n", file->head.hour, file->head.min, 
			 file->head.sec);
	  fprintf(stderr,"num_recs:%d  format:%d\n\n",
			 file->head.num_records, file->head.vol_scan_format);
	  }

   /* Initialize num_bins_rsl to point to the proper bin count array for
	  the scan format. */
   if ((file->head.vol_scan_format == 1) || 
	   (file->head.vol_scan_format == 3)) /* Normal mode */
	  {
	  num_bins_rsl = num_bins_normal;
	  }
   else  /* Compressed mode */
      num_bins_rsl = num_bins_compressed;

   /* Allocate space for two mcg_rays. mcg_ray will contain the current
	  ray, and mcg_ray_last will contain the previous ray. The previous ray
	  is needed only at sweep increments. */
   mcg_ray = (mcgRay_t *)malloc(sizeof(mcgRay_t));
   mcg_ray_last = (mcgRay_t *)malloc(sizeof(mcgRay_t));

   /* Initialize the values of the Radar_header structure */
   Radar_headerInit(radar, &file->head);
   /* Create a structure of type Volume */
   radar->v[DZ_INDEX] = RSL_new_volume(MAX_SWEEPS);
   if (radar->v[DZ_INDEX] == NULL) {
	 perror("mcgill_to_radar: RSL_new_volume");
	 return radar;
   }
   /* Initialize the values of the Volume_header structure. */
   Volume_headerInit(radar->v[DZ_INDEX], file->head.vol_scan_format);
   
   /* initialize counters */
   ray_num = -1;
   mcg_ray_last->sweep_num = -1;

   /* Main loop to read in a ray from the Mcgill data file and store 
	  into RSL radar structure */
   while ((code = mcgRayBuild(mcg_ray, file)) == MCG_OK)
	  {
	  /* Discard rays with bogus azimuth values. */
	  if (mcg_ray->azm > 360.0)
		 {
		 if (radar_verbose_flag)
		    fprintf(stderr,"**** Bogus azm:%.1f, discarding ray.\n", mcg_ray->azm);
	     continue;
		 }
	  /* Check for end_of_sweep. Fill the sweep header _after_ we've 
		 read in the sweep, since we don't know the number of
		 rays in the sweep until we find the start of the next sweep*/
	  if (mcg_ray_last->sweep_num < mcg_ray->sweep_num)  /* new sweep? */
		 {
		 if (mcg_ray_last->sweep_num >= 0)
			{
			/* fill the sweep header for the previous sweep */
			Sweep_headerInit(radar->v[DZ_INDEX]->sweep[mcg_ray_last->sweep_num],
							  mcg_ray_last, ray_num+1);
			}		 
		 ray_num = -1;  /* Reset ray counter at start of each sweep */

		 /* Check for too many sweeps. If we've exceeded MAX_SWEEPS,
		    we're lost. */
		 if (MAX_SWEEPS < mcg_ray->sweep_num + 1)
			{
			perror("RSL_mcgill_to_radar: Exceeded expected no. of sweeps");
			mcgFileClose(file);
			RSL_free_radar(radar);
			return NULL;
			}
		 if (rsl_qsweep != NULL) {
		   if (mcg_ray->sweep_num > rsl_qsweep_max) break;
		   if (rsl_qsweep[mcg_ray->sweep_num] == 0) continue;
		 }

		 /* Create new sweep structure. */
		 radar->v[DZ_INDEX]->sweep[mcg_ray->sweep_num] = RSL_new_sweep(MAX_RAYS);
		 if (radar->v[DZ_INDEX]->sweep[mcg_ray->sweep_num] == NULL) {
		   perror("mcgill_to_radar: RSL_new_sweep");
		   return radar;
		 }
		 }  /* end if new sweep */
	  
	  ray_num += 1;  /* Increment ray counter */
	  /* Check for too many rays. */
	  if (ray_num > MAX_RAYS)
		 {
		 perror("mcgill_to_radar: Exceeded maximal no. of rays");
		 mcgFileClose(file);
		 RSL_free_radar(radar);
		 return NULL;
		 }

	  /* Create a new RSL ray containing correct # of bins, and fill it. */
	  radar->v[DZ_INDEX]->sweep[mcg_ray->sweep_num]->ray[ray_num] = 
	                             RSL_new_ray(num_bins_rsl[mcg_ray->sweep_num] + 8);
	  if (radar->v[DZ_INDEX]->sweep[mcg_ray->sweep_num]->ray[ray_num] == NULL ) {
		perror("mcgill_to_radar: RSL_new_ray");
		return radar;
	  }

	  Ray_headerInit(radar->v[DZ_INDEX]->sweep[mcg_ray->sweep_num]->ray[ray_num],
					 &file->head, mcg_ray, ray_num, num_bins_rsl[mcg_ray->sweep_num]);
	  RayFill(radar->v[DZ_INDEX]->sweep[mcg_ray->sweep_num]->ray[ray_num], 
			  mcg_ray);

	  /* Swap mcg_ray pointers so that the current mcg_ray, which we've
		 finished loading into an RSL_ray, becomes mcg_ray_last . */
	  swap = (mcgRay_t *)mcg_ray_last;
	  mcg_ray_last = (mcgRay_t *)mcg_ray;
	  mcg_ray = (mcgRay_t *)swap;
	  }  /* end while */

   /* At this point, we're finished reading file records.
	  Fill in the last sweep header. */
   Sweep_headerInit(radar->v[DZ_INDEX]->sweep[mcg_ray_last->sweep_num], 
					mcg_ray_last, ray_num+1);
   
   /* Check which flag the mcgill routines returned, and 
	  print appropriate terminating message */
quit: if (radar_verbose_flag)
	  {
	  switch (code)
		 {
	   case MCG_EOD:
		 fprintf(stderr,"MCG_EOD: Reached end of data file\n");
		 break;
	   case MCG_OPEN_FILE_ERR:
		 fprintf(stderr,"MCG_OPEN_FILE_ERR: Error opening Mcgill data file\n");
		 break;
	   case MCG_EOF:
		 fprintf(stderr,"MCG_EOF: End of data file reached prematurely\n");
		 break;
	   case MCG_READ_ERR:
		 fprintf(stderr,"MCG_READ_ERR: Error occurred while reading from data file\n");
		 break;
	   case MCG_FORMAT_ERR:
		 fprintf(stderr,"MCG_FORMAT_ERR: Format error encountered in data file\n");
		 break;
	   default:
		 fprintf(stderr,"Error reading data file \n");
		 break;
		 }
	  }  /* end if (radar_verbose_flag) */
   
   if (code == MCG_EOD)  /* Successfully read in Mcgill file? */
	  {
	  mcgFileClose(file);
	  radar = RSL_prune_radar(radar);
      return(radar);
	  }
   else  /* Fatal error occurred */
	  {
	  RSL_free_radar(radar);
	  return(NULL);
	  }
   }

