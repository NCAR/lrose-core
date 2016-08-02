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
/*
 *  Ingest a Darwin_Toga file and fill a Radar structure with the data.
 *  Darwin Type 1 data contains 4 fields: corrected & uncorrected refl,
 *  velocity, and spectrum width.
 *  Darwin type 19 data contains 1 field: uncorrected reflectivity.
 *  Ignore all other Darwin data types.
 *  Handle only PPI scans; ignore RHI scans.
 *
 * Object code from this file must be linked with the following
 * libraries:
 *    -ltg -lradar
 * 
 *   "libtg.a" handles the reading and decompression of Darwin TOGA
 * data records from the input toga file. 
 * 
 * 
 *   Kolander
 * Applied Research Corp.
 * NASA/GSFC
 *
 */

#include <unistd.h>
#include <string.h>
#include "toga.h"
#include <trmm_rsl/rsl.h> 

#define MAX_RAYS   512
#define MAX_SWEEPS  20
#define MISSING_VAL 0

extern int radar_verbose_flag;
int tg_open(char *infile, tg_file_str *tg_file);
int tg_read_ray(tg_file_str *tg_file);

/* Toga radar routines */
void fill_ray(Ray *, tg_file_str *, int);
void fill_ray_header(Ray *, tg_file_str *, int, int);
void fill_sweep_header(Radar *, tg_map_head_str *, int, int);
void fill_volume_header(Radar *, tg_map_head_str *);
void fill_radar_header(Radar *, tg_map_head_str *);




/********************************************************************/
void fill_ray(Ray *ray, tg_file_str *tg_file, int datatype)
   /* transfer ray data values from the toga ray structure
	  to the radar ray  */
   {
   int j;

   if (ray == NULL) return;
   for (j=0; j<tg_file->ray.num_bins[datatype]; j++)
	  {
	  /* check to see if this is valid data */
	  if (tg_file->ray.data[datatype][j] == TG_NO_DATA)
		 ray->range[j] = ray->h.invf(BADVAL);  /* bad data, store flag */
	  else   /* good data, store into radar ray */
		 ray->range[j] = ray->h.invf(tg_file->ray.data[datatype][j]);
	  }  /* end for */
   }



void fill_ray_header(Ray *ray, tg_file_str *tg_file, int elev_num,
					 int datatype)
   /* Unresolved: Many of the following values */
   {
   if (ray == NULL) return;
   ray->h.month = tg_file->ray_head.mon;
   ray->h.day = tg_file->ray_head.day;
   ray->h.year = tg_file->ray_head.year;
   ray->h.hour = tg_file->ray_head.hour;
   ray->h.minute = tg_file->ray_head.min;
   ray->h.sec = tg_file->ray_head.hunsec/100.0;
   ray->h.unam_rng = MISSING_VAL;           /* ?? */
   ray->h.azimuth = tg_file->ray.azm;
   ray->h.ray_num = tg_file->ray_num;
   ray->h.elev = tg_file->ray.elev;
   ray->h.elev_num = elev_num;
   ray->h.range_bin1 = (int)(tg_file->ray.start_km[datatype]*1000.0);
   ray->h.gate_size = (int)(tg_file->ray.interval_km[datatype]*1000.0);

   ray->h.vel_res = MISSING_VAL;           /* ?? */
   ray->h.sweep_rate = MISSING_VAL;        /* ?? */
   ray->h.prf = tg_file->map_head.prf;
   ray->h.azim_rate = MISSING_VAL;         /* ?? */
   ray->h.fix_angle = MISSING_VAL;         /* ?? */
   ray->h.pulse_count = MISSING_VAL;       /* ?? */
   ray->h.pulse_width = tg_file->map_head.pulsewd/100.0;
   ray->h.beam_width = 1.65;
   ray->h.frequency = 5625;
   ray->h.wavelength = (tg_file->map_head.wavelen/100.0)/100.0; /* m */
   ray->h.nyq_vel = (ray->h.wavelength * ray->h.prf)/4.0;  /* m/s */
   ray->h.nbins = tg_file->ray.num_bins[datatype];

   if (datatype == TG_DM_IND)
	  {
	  ray->h.f = DZ_F;
	  ray->h.invf = DZ_INVF;
	  }
   else if (datatype == TG_DZ_IND)
	  {
	  ray->h.f = DZ_F;
	  ray->h.invf = DZ_INVF;
	  }
   else if (datatype == TG_VR_IND)
	  {
	  ray->h.f = VR_F;
	  ray->h.invf = VR_INVF;
	  }
   else if (datatype == TG_SW_IND)
	  {
	  ray->h.f = SW_F;
	  ray->h.invf = SW_INVF;
	  }
   }
	  


void fill_sweep_header(Radar *radar, tg_map_head_str *map_head,
					   int sweep_num, int nrays)
   /* Arrive here _after_ sweep ray data has been filled in. */
   {
   if (radar_verbose_flag)
      fprintf(stderr,"sweep_num:%02d  num_rays:%d\n",sweep_num, nrays);
   
   if (map_head->data_set == 1)
	  {
	  radar->v[DZ_INDEX]->sweep[sweep_num]->h.f    = DZ_F;
	  radar->v[DZ_INDEX]->sweep[sweep_num]->h.invf = DZ_INVF;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.f    = DZ_F;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.invf = DZ_INVF;
	  radar->v[VR_INDEX]->sweep[sweep_num]->h.f    = VR_F;
	  radar->v[VR_INDEX]->sweep[sweep_num]->h.invf = VR_INVF;
	  radar->v[SW_INDEX]->sweep[sweep_num]->h.f    = SW_F;
	  radar->v[SW_INDEX]->sweep[sweep_num]->h.invf = SW_INVF;

	  radar->v[DZ_INDEX]->sweep[sweep_num]->h.sweep_num = sweep_num;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.sweep_num = sweep_num;
	  radar->v[VR_INDEX]->sweep[sweep_num]->h.sweep_num = sweep_num;
	  radar->v[SW_INDEX]->sweep[sweep_num]->h.sweep_num = sweep_num;
	  
	  radar->v[DZ_INDEX]->sweep[sweep_num]->h.elev = 
	                              map_head->angfix[sweep_num]/10.0;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.elev = 
	                              map_head->angfix[sweep_num]/10.0;
	  radar->v[VR_INDEX]->sweep[sweep_num]->h.elev = 
	                              map_head->angfix[sweep_num]/10.0;
	  radar->v[SW_INDEX]->sweep[sweep_num]->h.elev = 
	                              map_head->angfix[sweep_num]/10.0;

	  radar->v[DZ_INDEX]->sweep[sweep_num]->h.beam_width = 1.65;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.beam_width = 1.65;
	  radar->v[VR_INDEX]->sweep[sweep_num]->h.beam_width = 1.65;
	  radar->v[SW_INDEX]->sweep[sweep_num]->h.beam_width = 1.65;

	  radar->v[DZ_INDEX]->sweep[sweep_num]->h.horz_half_bw = 0.85;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.horz_half_bw = 0.85;
	  radar->v[VR_INDEX]->sweep[sweep_num]->h.horz_half_bw = 0.85;
	  radar->v[SW_INDEX]->sweep[sweep_num]->h.horz_half_bw = 0.85;

	  radar->v[DZ_INDEX]->sweep[sweep_num]->h.vert_half_bw = 0.85;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.vert_half_bw = 0.85;
	  radar->v[VR_INDEX]->sweep[sweep_num]->h.vert_half_bw = 0.85;
	  radar->v[SW_INDEX]->sweep[sweep_num]->h.vert_half_bw = 0.85;

	  radar->v[DZ_INDEX]->sweep[sweep_num]->h.nrays = nrays;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.nrays = nrays;
	  radar->v[VR_INDEX]->sweep[sweep_num]->h.nrays = nrays;
	  radar->v[SW_INDEX]->sweep[sweep_num]->h.nrays = nrays;
	  }
   else if (map_head->data_set == 19)
	  {
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.sweep_num = sweep_num;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.elev = 
	                              map_head->angfix[sweep_num]/10.0;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.beam_width = 1.65;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.horz_half_bw = 0.825;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.vert_half_bw = 0.825;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.nrays = nrays;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.f    = DZ_F;
	  radar->v[ZT_INDEX]->sweep[sweep_num]->h.invf = DZ_INVF;
	  }
   else  /* Another data type? */
	  {
	  }
   }



void fill_volume_header(Radar *radar, tg_map_head_str *map_head)
   {
   if (map_head->data_set == 1)
	  {
	  radar->v[DZ_INDEX] = RSL_new_volume(MAX_SWEEPS);
	  radar->v[ZT_INDEX] = RSL_new_volume(MAX_SWEEPS);
	  radar->v[VR_INDEX] = RSL_new_volume(MAX_SWEEPS);
	  radar->v[SW_INDEX] = RSL_new_volume(MAX_SWEEPS);
	  radar->v[DZ_INDEX]->h.type_str = strdup("Reflectivity");
	  radar->v[ZT_INDEX]->h.type_str = strdup("Reflectivity");
	  radar->v[VR_INDEX]->h.type_str = strdup("Velocity");
	  radar->v[SW_INDEX]->h.type_str = strdup("Spectrum width");
	  radar->v[DZ_INDEX]->h.nsweeps = map_head->numfix_ang;
	  radar->v[ZT_INDEX]->h.nsweeps = map_head->numfix_ang;
	  radar->v[VR_INDEX]->h.nsweeps = map_head->numfix_ang;
	  radar->v[SW_INDEX]->h.nsweeps = map_head->numfix_ang;
	  radar->v[DZ_INDEX]->h.f    = DZ_F;
	  radar->v[DZ_INDEX]->h.invf = DZ_INVF;
	  radar->v[ZT_INDEX]->h.f    = DZ_F;
	  radar->v[ZT_INDEX]->h.invf = DZ_INVF;
	  radar->v[VR_INDEX]->h.f    = VR_F;
	  radar->v[VR_INDEX]->h.invf = VR_INVF;
	  radar->v[SW_INDEX]->h.f    = SW_F;
	  radar->v[SW_INDEX]->h.invf = SW_INVF;
	  }
   else if (map_head->data_set == 19)
	  {
	  radar->v[ZT_INDEX] = RSL_new_volume(MAX_SWEEPS);
	  radar->v[ZT_INDEX]->h.type_str = strdup("Reflectivity");
	  radar->v[ZT_INDEX]->h.nsweeps = map_head->numfix_ang;
	  radar->v[ZT_INDEX]->h.f    = DZ_F;
	  radar->v[ZT_INDEX]->h.invf = DZ_INVF;
	  }
   else  /* Another data type? */
	  {
	  }
   }


void fill_radar_header(Radar *radar, tg_map_head_str *map_head)
   {
   radar->h.month = map_head->scan_mon;
   radar->h.day = map_head->scan_day;
   radar->h.year = map_head->scan_year;
   radar->h.hour = map_head->scan_hour;
   radar->h.minute = map_head->scan_min;
   radar->h.sec = map_head->scan_sec/100.0;
   strcpy(radar->h.radar_type, "toga");
   radar->h.number = 0;
   strcpy(radar->h.name, "");
   strcpy(radar->h.radar_name, "TOGA");
   strcpy(radar->h.city, "Darwin");
   strcpy(radar->h.state, "");
   /* The following lat, lon coordinates obtained from Dave Wolff */
   radar->h.latd = -12;
   radar->h.latm = -27;
   radar->h.lats = -26;
   radar->h.lond = 130;
   radar->h.lonm = 55;
   radar->h.lons = 31;
   radar->h.height = 37;
   radar->h.spulse = 500;  /* ns */
   radar->h.lpulse = 2000; /* ns */
   }




Radar *RSL_toga_to_radar(char *infile)
   /* Ingest a Darwin_Toga file and fill a Radar structure with the data.
	  Darwin Type 1 data contains 4 fields: corrected & uncorrected refl,
	  velocity, and spectrum width.
	  Darwin type 19 data contains 1 field: uncorrected reflectivity.
	  Ignore all other Darwin data types.
	  Handle only PPI scans; ignore RHI scans. */
   {
   int m, swp_num, num_bins;
   Radar *radar;
   Ray *new_ray;
   tg_file_str tg_file;
   extern int *rsl_qsweep; /* See RSL_read_these_sweeps in volume.c */
   extern int rsl_qsweep_max;
   
   /* open the toga data file and read toga file header into the 
	  tg_file map_head_structure  */
   if (tg_open(infile,&tg_file) < 0) /* Understands NULL 'infile' as stdin. */
	  {
	  if (radar_verbose_flag)
	     fprintf(stderr,"Error opening/reading data file\n");
	  close(tg_file.fd);
	  return NULL;
	  }
   if (radar_verbose_flag)
	  {
	  fprintf(stderr,"Input file:  %s\n",infile);
	  fprintf(stderr,"Scan_date: %.2d/%.2d/%d\n",tg_file.map_head.scan_mon,
			 tg_file.map_head.scan_day,tg_file.map_head.scan_year);
	  fprintf(stderr,"Scan_time: %.2d:%.2d\n",tg_file.map_head.scan_hour,
			 tg_file.map_head.scan_min);
	  }
   /* If this toga file does not contain a PPI (constant elevation)
	  scan, do not bother with it. Just quit. */
   if (tg_file.map_head.scanmod != 1)
	  {
	  if (radar_verbose_flag)
	     fprintf(stderr,"Darwtoga file %s does not contain a PPI scan.\n",infile);
	  close(tg_file.fd);
	  return NULL;
	  }
   /* Can handle only datatypes 1 and 19 */
   if ((tg_file.map_head.data_set != 1) &&
	   (tg_file.map_head.data_set != 19))
	  {
	  if (radar_verbose_flag)
	     fprintf(stderr,"File %s does not contain Doppler or refl data\n",infile);
	  close(tg_file.fd);
      return NULL;
	  }
   if (radar_verbose_flag)
	  {
	  if (tg_file.map_head.data_set == 1) fprintf(stderr,"Type 1 data\n");
	  else if (tg_file.map_head.data_set == 19) fprintf(stderr,"Type 19 data\n");
	  else fprintf(stderr,"Unknown data type\n");
	  }

   radar = RSL_new_radar(MAX_RADAR_VOLUMES);
   fill_radar_header(radar, &tg_file.map_head);
   fill_volume_header(radar, &tg_file.map_head);
   
   /* initialize counters */
   tg_file.ray_num = -1;
   swp_num = -1;

   /* Main loop to read in a toga ray from the toga
	  data file and store into radar structure */
   while ((m=tg_read_ray(&tg_file)) > 0)
	  {
	  /* check for end_of_sweep. Fill the sweep header _after_ we've 
		 read in the sweep */
	  if (swp_num < tg_file.ray_head.tilt - 1)  /* new sweep? */
		 {
		 if (swp_num >= 0)
			{
			/* fill the sweep header for the previous sweep */
			fill_sweep_header(radar, &tg_file.map_head, swp_num,
							  tg_file.ray_num+1);
			}		 
		 tg_file.ray_num = -1;  /* Reset ray_num. */
		 swp_num += 1;  /* increment sweep count */
		 if (rsl_qsweep != NULL) {
		   if (swp_num > rsl_qsweep_max) break;
		   if (rsl_qsweep[swp_num] == 0) continue;
		 }
		 /* Check for too many sweeps. */
		 if ((tg_file.map_head.numfix_ang < swp_num + 1) ||
			 (MAX_SWEEPS < swp_num + 1))
			{
			perror("toga_to_radar: Exceeded expected no. of sweeps");
			close(tg_file.fd);
			RSL_free_radar(radar);
			return NULL;
			}
		 /* Create new sweep structures. */
		 if (tg_file.map_head.data_set == 1)
			{
			radar->v[DZ_INDEX]->sweep[swp_num] = RSL_new_sweep(MAX_RAYS);
			radar->v[ZT_INDEX]->sweep[swp_num] = RSL_new_sweep(MAX_RAYS);
			radar->v[VR_INDEX]->sweep[swp_num] = RSL_new_sweep(MAX_RAYS);
			radar->v[SW_INDEX]->sweep[swp_num] = RSL_new_sweep(MAX_RAYS);
			}
		 else if (tg_file.map_head.data_set == 19)
			{
			radar->v[ZT_INDEX]->sweep[swp_num] = RSL_new_sweep(MAX_RAYS);
			}
		 }  /* end if new sweep */
	  
	  num_bins = tg_file.ray.num_bins[TG_DM_IND] + 4; /* types 1,19 data*/
	  tg_file.ray_num += 1;
	  /* Check for too many rays. */
	  if (tg_file.ray_num > MAX_RAYS)
		 {
		 perror("toga_to_radar: Exceeded maximal no. of rays");
		 close(tg_file.fd);
		 RSL_free_radar(radar);
		 return NULL;
		 }
	  /* check for uncorrected reflectivity data */
	  if (tg_file.ray.da_inv[TG_DM_IND] == TRUE)
		 {
		 new_ray = RSL_new_ray(num_bins);
		 radar->v[ZT_INDEX]->sweep[swp_num]->ray[tg_file.ray_num] = new_ray;
		 fill_ray_header(new_ray, &tg_file, swp_num, TG_DM_IND);
		 fill_ray(new_ray, &tg_file, TG_DM_IND);
		 }
	  /* check for corrected reflectivity data */
	  if (tg_file.ray.da_inv[TG_DZ_IND] == TRUE)
		 {
		 new_ray = RSL_new_ray(num_bins);
		 radar->v[DZ_INDEX]->sweep[swp_num]->ray[tg_file.ray_num] = new_ray;
		 fill_ray_header(new_ray, &tg_file, swp_num, TG_DZ_IND);
		 fill_ray(new_ray, &tg_file, TG_DZ_IND);
		 }
	  /* check for velocity data */
	  if (tg_file.ray.da_inv[TG_VR_IND] == TRUE)
		 {
		 new_ray = RSL_new_ray(num_bins);
		 radar->v[VR_INDEX]->sweep[swp_num]->ray[tg_file.ray_num] = new_ray;
		 fill_ray_header(new_ray, &tg_file, swp_num, TG_VR_IND);
		 fill_ray(new_ray, &tg_file, TG_VR_IND);
		 }
	  /* check for spectrum width data */
	  if (tg_file.ray.da_inv[TG_SW_IND] == TRUE)
		 {
		 new_ray = RSL_new_ray(num_bins);
		 radar->v[SW_INDEX]->sweep[swp_num]->ray[tg_file.ray_num] = new_ray;
		 fill_ray_header(new_ray, &tg_file, swp_num, TG_SW_IND);
		 fill_ray(new_ray, &tg_file, TG_SW_IND);
		 }
	  }  /* end while */

   /* At this point, we are finished reading toga data records.
	  Fill in the last sweep header. */
   fill_sweep_header(radar, &tg_file.map_head, swp_num,
					 tg_file.ray_num+1);
   
   /* Check which flag the TOGA read_record routines returned, and 
	  print appropriate terminating message */
   if (radar_verbose_flag)
	  {
	  switch (m)
		 {
	   case TG_END_DATA:
		 fprintf(stderr,"Reached end of data file\n");
		 break;
	   case TG_RAY_READ_ERR:
		 fprintf(stderr,"Error reading toga ray\n");
		 break;
	   case TG_RAY_NOTYPE:
		 fprintf(stderr,"Found unrecognized toga ray type\n");
		 fprintf(stderr,"ray_type: %d\n",tg_file.ray_head.type);
		 break;
	   case TG_REC_NOSEQ:
		 fprintf(stderr,"found out-of-sequence toga record \n");
		 break;
	   case -1:
		 fprintf(stderr,"Error reading toga ray_header \n");
		 break;
	   default:
		 fprintf(stderr,"Error reading toga file \n");
		 break;
		 }
	  }  /* end if (radar_verbose_flag) */
   
   close(tg_file.fd);
   if (m == TG_END_DATA) {
	 radar = RSL_prune_radar(radar);
	 return radar;
   } else
	  {
	  RSL_free_radar(radar);
	  return NULL;
	  }

   }
