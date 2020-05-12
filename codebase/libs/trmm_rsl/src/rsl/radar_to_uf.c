/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define USE_RSL_VARS
#include <trmm_rsl/rsl.h> 
extern int radar_verbose_flag;
/* Missing data flag : -32768 when a signed short. */
#define UF_NO_DATA 0X8000


/* Field names. Any convensions may be observed. */
/* Typically:
 *    DZ = Reflectivity (dBZ).
 *    VR = Radial Velocity.
 *    SW = Spectrum Width.
 *    CZ = Corrected Reflectivity. (Quality controlled: AP removed, etc.)
 *    ZT = Total Reflectivity (dB(mW)).  Becomes UZ in UF files.
 *    DR = Differential Reflectivity.
 *    LR = Another DR (LDR).
 *    ZD = Tina Johnson use this one.
 *    DM = Received power.
 *    RH = Rho coefficient.
 *    PH = Phi (MCTEX parameter).
 *    XZ = X-band reflectivity.
 *    CD = Corrected ZD.
 *    MZ = DZ mask for 1C-51 HDF.
 *    MD = ZD mask for 1C-51 HDF.
 *    ZE = Edited reflectivity.
 *    VE = Edited velocity.
 *    KD = KDP wavelength*deg/km
 *    TI = TIME (units unknown).
 * These fields may appear in any order in the UF file.
 * There are more fields than appear here.  See rsl.h.
 */


typedef short UF_buffer[16384]; /* Bigger than documented 4096. */

void swap_uf_buffer(UF_buffer uf);
void swap2(short *buf, int n);

/**********************************************************************/
/*                                                                    */
/*                     RSL_radar_to_uf_fp                             */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      May  20, 1994                                                 */
/**********************************************************************/
void RSL_radar_to_uf_fp(Radar *r, FILE *fp)

{
  /*
   * 1. Fill the UF buffers with data from the Radar structure.
   * 2. Write to a stream.  Assume open and leave it so.
   */


  UF_buffer uf;

/* These are pointers to various locations within the UF buffer 'uf'.
 * They are used to index the different components of the UF structure in
 * a manor consistant with the UF documentation.  For instance, uf_ma[1]
 * will be equivalenced to the second word (2 bytes/each) of the UF
 * buffer.
 */
  short *uf_ma;  /* Mandatory header block. */
  short *uf_op;  /* Optional header block.  */
  short *uf_lu;  /* Local Use header block.  */
  short *uf_dh;  /* Data header.  */
  short *uf_fh;  /* Field header. */
  short *uf_data; /* Data. */

/* The length of each header. */
  int len_ma, len_op, len_lu, len_dh, len_fh, len_data;

/* Booleans to flag inclusion of headers. */
  int q_op, q_lu, q_dh, q_fh;

  int current_fh_index; 
  int scale_factor;
  int rec_len, save_rec_len;
  int nfield;
  float vr_az;
  int max_field_names;

  struct tm *tm;
  time_t the_time;

  int i,j,k,m;
  int degree, minute;
  float second;


/* Here are the arrays for each field type.  Each dimension is the number
 * of fields in the radar structure.  I do this because the radar organization
 * is by volumes (field types) and the UF demands that each ray contain
 * all the field types.
 */
  Volume **volume;
  Sweep  **sweep;
  Ray     *ray;
  int     *nsweeps;
  int nvolumes, maxsweeps, nrays;
  int true_nvolumes;
  int sweep_num, ray_num, rec_num;
  float x;

  if (r == NULL) {
    fprintf(stderr, "radar_to_uf_fp: radar pointer NULL\n");
    return;
  }

/* Do all the headers first time around.  Then, prune OP and LU. */
  q_op = q_lu = q_dh = q_fh = 1;

  memset(&uf, 0, sizeof(uf)); /* Init to 0 or NULL for pointers. */

  sweep_num = ray_num = rec_num = 0;
  true_nvolumes = nvolumes = maxsweeps = nrays = 0;

/*
 * The organization of the Radar structure is by volumes, then sweeps, then
 * rays, then gates.  This is different from the UF file organization.
 * The UF format wants sweeps, rays, then gates for all field types (volumes).
 * So, we have to do a back flip, here.  This is achieved by maintaining 
 * an array of volume pointers and sweep pointers, each dimensioned by 
 * 'nvolumes', which contains the data for the different field types; this
 * is our innermost loop.  The variables are 'volume[i]' and 'sweep[i]' where
 * 'i' is the volume index.
 *
 * In other words, we are getting all the field types together, when we
 * are looping on the number of rays in a sweep, so we can load the UF_buffer
 * appropriately.
 */

  nvolumes = r->h.nvolumes;
  volume   = (Volume **) calloc(nvolumes, sizeof(Volume *));
  sweep    = (Sweep  **) calloc(nvolumes, sizeof(Sweep  *));
  nsweeps  = (int *)     calloc(nvolumes, sizeof(int));

/* Get the the number of sweeps in the radar structure.  This will be
 * the main controlling loop variable.
 */
  for (i=0; i<nvolumes; i++) {
    volume[i] = r->v[i];
    if(volume[i]) {
      nsweeps[i] = volume[i]->h.nsweeps;
      if (nsweeps[i] > maxsweeps) maxsweeps = nsweeps[i];
      true_nvolumes++;
    }
  }

  if (radar_verbose_flag) {
    fprintf(stderr,"True number of volumes for UF is %d\n", true_nvolumes);
    fprintf(stderr,"Maximum #   of volumes for UF is %d\n", nvolumes);
  }

  max_field_names = sizeof(RSL_ftype) / 4;

/*--------
 *   LOOP for all sweeps (typically 11 or 16 for wsr88d data.
 *
 */
  for (i=0; i<maxsweeps; i++) {
    /* Get the array of volume and sweep pointers; one for each field type. */
    nrays = 0;
    for (k=0; k<nvolumes; k++) {
      if (volume[k]) sweep[k] = volume[k]->sweep[i];
      
      /* Check if we really can access this sweep.  Paul discovered that
       * if the actual number of sweeps is less than the maximum that we
       * could be chasing a bad pointer (a NON-NULL garbage pointer).
       */
      if (i >= nsweeps[k]) sweep[k] = NULL;

      if (sweep[k]) if (sweep[k]->h.nrays > nrays) nrays = sweep[k]->h.nrays;
    }

    sweep_num++;  /* I guess it will be ok to count NULL sweeps. */
    ray_num = 0;
  if (radar_verbose_flag) 
    fprintf(stderr,"Processing sweep %d for %d rays.", i, nrays);
  if (radar_verbose_flag) {
    if (little_endian()) {
      fprintf(stderr," ... On Little endian.\n");
    }
  } else {
    fprintf(stderr,"\n");
  }


/* Now LOOP for all rays within this particular sweep (i).
 *    Get all the field types together for the ray, see ray[k], and
 *    fill the UF data buffer appropriately.
 */
    for (j=0; j<nrays; j++) {
      memset(uf, 0, sizeof(uf));
      nfield = 0;
      ray_num++;  /* And counting, possibly, NULL rays. */
      current_fh_index = 0;


      /* Find any ray for header information. It does not matter which
       * ray, since the information for the MANDITORY, OPTIONAL, and LOCAL
       * USE headers is common to any field type ray.
       */
      ray = NULL;
      for (k=0; k<nvolumes; k++) {
        if (sweep[k])
          if (j < sweep[k]->h.nrays)
            if (sweep[k]->ray)
              if ((ray = sweep[k]->ray[j])) break;
      }

      /* If there is no such ray, then continue on to the next ray. */
      if (ray) {
/*
                  fprintf(stderr,"Ray: %.4d, Time: %2.2d:%2.2d:%f  %.2d/%.2d/%.4d\n", ray_num, ray->h.hour, ray->h.minute, ray->h.sec, ray->h.month, ray->h.day, ray->h.year);
*/

        /* 
         * ---- Begining of MANDITORY HEADER BLOCK.
         */
        uf_ma = uf;
        memcpy(&uf_ma[0], "UF", 2);
        if (little_endian()) memcpy(&uf_ma[0], "FU", 2);
        uf_ma[1]  = 0;  /* Not known yet. */
        uf_ma[2]  = 0;  /* Not known yet. Really, I do. */
        uf_ma[3]  = 0;  /* Not known yet. */
        uf_ma[4]  = 0;  /* Not known yet. */

        uf_ma[6]  = 1;
        uf_ma[7]  = ray_num;
        uf_ma[8 ] = 1;
        uf_ma[9 ] = sweep_num;
        memcpy(&uf_ma[10], r->h.radar_name, 8);
        if (little_endian()) swap2(&uf_ma[10], 8/2);
        memcpy(&uf_ma[14], r->h.name, 8);
        if (little_endian()) swap2(&uf_ma[14], 8/2);
        /* Convert decimal lat/lon to d:m:s */

        if (ray->h.lat != 0.0) {
          degree = (int)ray->h.lat;
          minute = (int)((ray->h.lat - degree) * 60);
          second = (ray->h.lat - degree - minute/60.0) * 3600.0;
        } else {
          degree = r->h.latd;
          minute = r->h.latm;
          second = r->h.lats;
        }
        uf_ma[18] = degree;
        uf_ma[19] = minute;
        if (second > 0.0) uf_ma[20] = second*64 + 0.5;
        else uf_ma[20] = second*64 - 0.5;

        if (ray->h.lon != 0.0) {
          degree = (int)ray->h.lon;
          minute = (int)((ray->h.lon - degree) * 60);
          second = (ray->h.lon - degree - minute/60.0) * 3600.0;
        } else {
          degree = r->h.lond;
          minute = r->h.lonm;
          second = r->h.lons;
        }
        uf_ma[21] = degree;
        uf_ma[22] = minute;
        if (second > 0.0) uf_ma[23] = second*64 + 0.5;
        else uf_ma[23] = second*64 - 0.5;
        if (ray->h.alt != 0) 
          uf_ma[24] = ray->h.alt;
        else
          uf_ma[24] = r->h.height;

        uf_ma[25] = ray->h.year % 100; /* By definition: not year 2000 compliant. */
        uf_ma[26] = ray->h.month;
        uf_ma[27] = ray->h.day;
        uf_ma[28] = ray->h.hour;
        uf_ma[29] = ray->h.minute;
        uf_ma[30] = ray->h.sec;
        memcpy(&uf_ma[31], "UT", 2);
        if (little_endian()) memcpy(&uf_ma[31], "TU", 2);
        if (ray->h.azimuth > 0) uf_ma[32] = ray->h.azimuth*64 + 0.5;
        else uf_ma[32] = ray->h.azimuth*64 - 0.5;
        uf_ma[33] = ray->h.elev*64 + 0.5;
        uf_ma[34] = 1;      /* Sweep mode: PPI = 1 */
        if (ray->h.fix_angle != 0.)
             uf_ma[35] = ray->h.fix_angle*64.0 + 0.5;
        else uf_ma[35] = sweep[k]->h.elev*64.0 + 0.5;
        uf_ma[36] = ray->h.sweep_rate*(360.0/60.0)*64.0 + 0.5;
        
        the_time = time(NULL);
        tm = gmtime(&the_time);
        
        uf_ma[37] = tm->tm_year % 100; /* Same format as data year */
        uf_ma[38] = tm->tm_mon+1;
        uf_ma[39] = tm->tm_mday;
        memcpy(&uf_ma[40], "RSL" RSL_VERSION_STR, 8);
        if (little_endian()) swap2(&uf_ma[40], 8/2);
        uf_ma[44] = (signed short)UF_NO_DATA;
        len_ma = 45;
        uf_ma[2] = len_ma+1;
        /*
         * ---- End of MANDITORY HEADER BLOCK.
         */
        
        /* ---- Begining of OPTIONAL HEADER BLOCK. */
        len_op = 0;
        if (q_op) {
          q_op = 0;  /* Only once. */
          uf_op = uf+len_ma;
          memcpy(&uf_op[0], "TRMMGVUF", 8);
          if (little_endian()) swap2(&uf_op[0], 8/2);
          uf_op[4] = (signed short)UF_NO_DATA;
          uf_op[5] = (signed short)UF_NO_DATA;
          uf_op[6] = ray->h.hour;
          uf_op[7] = ray->h.minute;
          uf_op[8] = ray->h.sec;
          memcpy(&uf_op[9], "RADAR_UF", 8);
          if (little_endian()) swap2(&uf_op[9], 8/2);
          uf_op[13] = 2;
          len_op = 14;
        }
        /* ---- End of OPTIONAL HEADER BLOCK. */
        
        /* ---- Begining of LOCAL USE HEADER BLOCK. */
        /* If we have DZ and VR, check to see if their azimuths are
         * different. If they are, we store VR azimuth in Local Use
         * Header. These differences occur with WSR-88D radars, which
         * run separate sweeps for DZ and VR at low elevations.
         */
            q_lu = 0;
        if (sweep[DZ_INDEX] && sweep[VR_INDEX]) {
            if (sweep[DZ_INDEX]->ray[j] && sweep[VR_INDEX]->ray[j]) {
            vr_az = sweep[VR_INDEX]->ray[j]->h.azimuth;
            if (sweep[DZ_INDEX]->ray[j]->h.azimuth != vr_az)
                q_lu = 1; /* Set to use Local Use Header block. */
            }
        }
        len_lu = 0;
        if (q_lu) {
          /* Store azimuth for WSR-88D VR ray in Local Use Header. */
          uf_lu = uf+len_ma+len_op;
          memcpy(&uf_lu[0], "AZ", 2);
          if (little_endian()) memcpy(&uf_lu[0], "ZA", 2);
          if (vr_az > 0) uf_lu[1] = vr_az*64 + 0.5;
          else uf_lu[1] = vr_az*64 - 0.5;
          len_lu = 2;
        }
        /* ---- End  of LOCAL USE HEADER BLOCK. */


       /* Here is where we loop on each field type.  We need to keep
        * track of how many FIELD HEADER and FIELD DATA sections, one
        * for each field type, we fill.  The variable that tracks this
        * index into 'uf' is 'current_fh_index'.  It is bumped by
        * the length of the FIELD HEADER and FIELD DATA for each field
        * type encountered.  Field types expected are: Reflectivity,
        * Velocity, and Spectrum width; this is a typicial list but it
        * is not restricted to it.
        */
        
         for (k=0; k<nvolumes; k++) {
          if (sweep[k])
            if (j < sweep[k]->h.nrays && sweep[k]->ray[j])
              ray = sweep[k]->ray[j];
            else
              ray = NULL;
          else ray = NULL;

          if (ray) {
            /* ---- Begining of DATA HEADER. */
            nfield++;
            if (q_dh) {
              len_dh = 2*true_nvolumes + 3;
              uf_dh = uf+len_ma+len_op+len_lu;
              uf_dh[0] = nfield;
              uf_dh[1] = 1;
              uf_dh[2] = nfield;
              /* 'nfield' indexes the field number.
               * 'k' indexes the particular field from the volume.
	       *  RSL_ftype contains field names and is defined in rsl.h.
               */
              if (k > max_field_names-1) {
	        fprintf(stderr,
                  "RSL_uf_to_radar: No field name for volume index %d\n", k);
	        fprintf(stderr,"RSL_ftype must be updated in rsl.h for new field.\n");
	        fprintf(stderr,"Quitting now.\n");
                return;
	      }
              memcpy(&uf_dh[3+2*(nfield-1)], RSL_ftype[k], 2);
              if (little_endian()) swap2(&uf_dh[3+2*(nfield-1)], 2/2);
              if (current_fh_index == 0) current_fh_index = len_ma+len_op+len_lu+len_dh;
              uf_dh[4+2*(nfield-1)] = current_fh_index + 1;
            }
            /* ---- End of DATA HEADER. */
            
            /* ---- Begining of FIELD HEADER. */
            if (q_fh) {
              uf_fh = uf+current_fh_index;
              uf_fh[1] = scale_factor = 100;
              uf_fh[2] = ray->h.range_bin1/1000.0;
              uf_fh[3] = ray->h.range_bin1 - (1000*uf_fh[2]);
              uf_fh[4] = ray->h.gate_size;
              uf_fh[5] = ray->h.nbins;
              uf_fh[6] = ray->h.pulse_width*(RSL_SPEED_OF_LIGHT/1.0e6);
              uf_fh[7] = sweep[k]->h.beam_width*64.0 + 0.5;
              uf_fh[8] = sweep[k]->h.beam_width*64.0 + 0.5;
              uf_fh[9] = ray->h.frequency*64.0 + 0.5; /* Bandwidth (mHz). */
              uf_fh[10] = 0; /* Horizontal polarization. */ 
              uf_fh[11] = ray->h.wavelength*64.0*100.0; /* m to cm. */
              uf_fh[12] = ray->h.pulse_count;
              memcpy(&uf_fh[13], "  ", 2);
              uf_fh[14] = (signed short)UF_NO_DATA;
              uf_fh[15] = (signed short)UF_NO_DATA;
              if (k == DZ_INDEX || k == ZT_INDEX) {
                uf_fh[16] = volume[k]->h.calibr_const*100.0 + 0.5;
              }
              else {
                memcpy(&uf_fh[16], "  ", 2);
              }
              if (ray->h.prf != 0)
                uf_fh[17] = 1.0/ray->h.prf*1000000.0; /* Pulse repetition time(msec)  = 1/prf */
              else
                uf_fh[17] = (signed short)UF_NO_DATA; /* Pulse repetition time  = 1/prf */
              uf_fh[18] = 16;
              if (VR_INDEX == k || VE_INDEX == k) {
                uf_fh[19] = scale_factor*ray->h.nyq_vel;
                uf_fh[20] = 1;
                len_fh = 21;
              } else {
                len_fh = 19;
              }
              
              uf_fh[0] = current_fh_index + len_fh + 1;
              /* ---- End of FIELD HEADER. */
              
              /* ---- Begining of FIELD DATA. */
              uf_data = uf+len_fh+current_fh_index;
              len_data = ray->h.nbins;
              for (m=0; m<len_data; m++) {
                x = ray->h.f(ray->range[m]);
                if (x == BADVAL || x == RFVAL || x == APFLAG || x == NOECHO)
                  uf_data[m] = (signed short)UF_NO_DATA;
                else
                  uf_data[m] = scale_factor * x;
              }
              
              current_fh_index += (len_fh+len_data);
            }
          }
        /* ---- End of FIELD DATA. */
        }
        /* Fill in some infomation we didn't know.  Like, buffer length,
         * record number, etc.
         */
        rec_num++;
        uf_ma[1] = current_fh_index;
        uf_ma[3] = len_ma + len_op + 1;
        uf_ma[4] = len_ma + len_op + len_lu + 1;
        uf_ma[5] = rec_num;
        
        /* WRITE the UF buffer. */
        rec_len =(int)uf_ma[1]*2;
        save_rec_len = rec_len;  /* We destroy 'rec_len' when making it
                        big endian on a little endian machine. */
        if (little_endian()) swap_4_bytes(&rec_len);
        (void)fwrite(&rec_len, sizeof(int), 1, fp);
        if (little_endian()) swap_uf_buffer(uf);
        (void)fwrite(uf, sizeof(char), save_rec_len, fp);
        (void)fwrite(&rec_len, sizeof(int), 1, fp);
      } /* if (ray) */
    }
  }
}

/**********************************************************************/
/*                                                                    */
/*                     RSL_radar_to_uf                                */
/*                                                                    */
/**********************************************************************/
void RSL_radar_to_uf(Radar *r, char *outfile)
{
  FILE *fp;
  if (r == NULL) {
    fprintf(stderr, "radar_to_uf: radar pointer NULL\n");
    return;
  }

  if ((fp = fopen(outfile, "w")) == NULL) {
    perror(outfile);
    return;
  }

  RSL_radar_to_uf_fp(r, fp);
  fclose(fp);
}

/**********************************************************************/
/*                                                                    */
/*                     RSL_radar_to_uf_gzip                           */
/*                                                                    */
/**********************************************************************/
void RSL_radar_to_uf_gzip(Radar *r, char *outfile)
{
  FILE *fp;
  if (r == NULL) {
    fprintf(stderr, "radar_to_uf_gzip: radar pointer NULL\n");
    return;
  }

  if ((fp = fopen(outfile, "w")) == NULL) {
    perror(outfile);
    return;
  }

  fp = compress_pipe(fp);
  RSL_radar_to_uf_fp(r, fp);
  rsl_pclose(fp);
}
