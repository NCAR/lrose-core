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
/**********************************************************************/
/*                                                                    */
/*                     RSL_read_ray                                   */
/*                     RSL_read_sweep                                 */
/*                     RSL_read_volume                                */
/*                     RSL_read_radar                                 */
/*                                                                    */
/*                     RSL_write_ray                                  */
/*                     RSL_write_sweep                                */
/*                     RSL_write_volume                               */
/*                     RSL_write_radar                                */
/*                                                                    */
/*  By: John Merritt                                                  */
/*      Space Applications Corporation                                */
/*      April 7, 1994                                                 */
/**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <trmm_rsl/rsl.h> 

extern int radar_verbose_flag;
/**********************************************************************/
/**********************************************************************/
/*                                                                    */
/*                             I N P U T                              */
/*                                                                    */
/**********************************************************************/
/**********************************************************************/

Ray *RSL_read_ray(FILE *fp)
{
  char header_buf[512];
  Ray_header ray_h;
  Ray *r;
  int nbins;

  (void)fread(header_buf, sizeof(char), sizeof(header_buf), fp);
  (void)fread(&nbins, sizeof(int), 1, fp);
  if (nbins == 0) return NULL;

  memcpy(&ray_h, header_buf, sizeof(Ray_header));

  r = RSL_new_ray(ray_h.nbins);
  r->h = ray_h;

  (void)fread(r->range, sizeof(Range), r->h.nbins, fp);
  return r;
}
Sweep *RSL_read_sweep(FILE *fp)
{
  char header_buf[512];
  Sweep_header sweep_h;
  int i;
  Sweep *s;
  int nrays;

  (void)fread(header_buf, sizeof(char), sizeof(header_buf), fp);

  (void)fread(&nrays, sizeof(int), 1, fp);
  if (nrays == 0) return NULL;

  if (radar_verbose_flag)
	fprintf(stderr,"Reading %d rays. ", nrays);
  memcpy(&sweep_h, header_buf, sizeof(Sweep_header));
  if (radar_verbose_flag)
	fprintf(stderr,"From header info nrays = %d\n", sweep_h.nrays);
  s = RSL_new_sweep(sweep_h.nrays);
  s->h = sweep_h;
  for (i=0; i<s->h.nrays; i++) {
	s->ray[i] = RSL_read_ray(fp); 
 }
  return s;
}
Volume *RSL_read_volume(FILE *fp)
{
  char header_buf[512];
  Volume_header vol_h;
  int i;
  Volume *v;
  int nsweeps;



  (void)fread(header_buf, sizeof(char), sizeof(header_buf), fp);
  (void)fread(&nsweeps, sizeof(int), 1, fp);
  if (nsweeps == 0)	return NULL;

  if (radar_verbose_flag)
	fprintf(stderr,"Reading %d sweeps. ", nsweeps);
  memcpy(&vol_h, header_buf, sizeof(Volume_header));
  if (radar_verbose_flag)
	fprintf(stderr,"From header info nsweeps = %d\n", vol_h.nsweeps);
  v = RSL_new_volume(vol_h.nsweeps);
  v->h = vol_h;
  for (i=0; i<v->h.nsweeps; i++) {
  if (radar_verbose_flag)
	fprintf(stderr,"RSL_read_sweep %d ", i);
	v->sweep[i] = RSL_read_sweep(fp);
  }
  return v;
}

Radar *set_default_function_pointers(Radar *radar)
{
  int i,j,k;
  Volume *v;
  Sweep *s;
  Ray *r;
  float (*f[MAX_RADAR_VOLUMES])(Range x);
  Range (*invf[MAX_RADAR_VOLUMES])(float x);
  f[DZ_INDEX] = DZ_F;
  f[VR_INDEX] = VR_F;
  f[SW_INDEX] = SW_F;
  f[CZ_INDEX] = CZ_F;
  f[ZT_INDEX] = ZT_F;
  f[DR_INDEX] = DR_F;
  f[LR_INDEX] = LR_F;

  invf[DZ_INDEX] = DZ_INVF;
  invf[VR_INDEX] = VR_INVF;
  invf[SW_INDEX] = SW_INVF;
  invf[CZ_INDEX] = CZ_INVF;
  invf[ZT_INDEX] = ZT_INVF;
  invf[DR_INDEX] = DR_INVF;
  invf[LR_INDEX] = LR_INVF;
  
  if (radar == NULL) return NULL;
  for (i=0; i<radar->h.nvolumes; i++) {
	v = radar->v[i];
	if (v) {
	  for (j=0; j<v->h.nsweeps; j++) {
		s = v->sweep[j];
		if (s) {
		  for (k=0; k<s->h.nrays; k++) {
			r = s->ray[k];
			if (r) {
			  r->h.f = f[i];
			  r->h.invf = invf[i];
			}
		  }
		  s->h.f = f[i];
		  s->h.invf = invf[i];
		}
	  }
	  v->h.f = f[i];
	  v->h.invf = invf[i];
	}
  }
  return radar;
}

Radar *RSL_read_radar(char *infile)
{
  /* On disk each header buffer size is this big to reserve space for
   * any new members.  This will make older radar files readable as
   * development proceeds.
   */
  char header_buf[512];
  Radar_header radar_h;
  Radar *radar;
  FILE *fp;
  int i;
  int nradar;
  char title[100];

  if ((fp = fopen(infile, "r")) == NULL) {
	perror(infile);
	return NULL;
  }
  fp = uncompress_pipe(fp);
  (void)fread(title, sizeof(char), sizeof(title), fp);
  if (strncmp(title, "RSL", 3) != 0) return NULL;

  (void)fread(header_buf, sizeof(char), sizeof(header_buf), fp);
  memcpy(&radar_h, header_buf, sizeof(Radar_header));
  radar = RSL_new_radar(MAX_RADAR_VOLUMES);
  radar->h = radar_h;

  (void)fread(&nradar, sizeof(int), 1, fp);
  if (radar_verbose_flag)
	fprintf(stderr,"Reading %d volumes.\n", nradar);

  for (i=0; i<nradar; i++) {
	if (radar_verbose_flag)
	  fprintf(stderr,"RSL_read_volume %d ", i);
	radar->v[i] = RSL_read_volume(fp);
  }

  rsl_pclose(fp);
  radar = set_default_function_pointers(radar);
  return radar;
}
  

/**********************************************************************/
/**********************************************************************/
/*                                                                    */
/*                           O U T P U T                              */
/*                                                                    */
/**********************************************************************/
/**********************************************************************/

int RSL_write_ray(Ray *r, FILE *fp)
{
  char header_buf[512];
  int n = 0;
  int zero = 0;
  
  memset(header_buf, 0, sizeof(header_buf));
  if (r == NULL) {
	n += fwrite(header_buf, sizeof(header_buf), 1, fp) * sizeof(header_buf);
	n += fwrite(&zero, sizeof(int), 1, fp) * sizeof(int);
	return n;
  }
  memcpy(header_buf, &r->h, sizeof(r->h));
  n += fwrite(header_buf, sizeof(char), sizeof(header_buf), fp);
  n += fwrite(&r->h.nbins, sizeof(int), 1, fp) * sizeof(int);
  n += fwrite(r->range, sizeof(Range), r->h.nbins, fp) * sizeof(Range);
  return n;
}
int RSL_write_sweep(Sweep *s, FILE *fp)
{
  char header_buf[512];
  int i;
  int n = 0;
  int zero = 0;


  memset(header_buf, 0, sizeof(header_buf));
  if (s == NULL) {
	n += fwrite(header_buf, sizeof(header_buf), 1, fp) * sizeof(header_buf);
	n += fwrite(&zero, sizeof(int), 1, fp) * sizeof(int);
	return n;
  }
  memcpy(header_buf, &s->h, sizeof(s->h));
  n += fwrite(header_buf, sizeof(char), sizeof(header_buf), fp);
  if (radar_verbose_flag)
	fprintf(stderr,"Expect to output %d rays.\n", s->h.nrays);
  n += fwrite(&s->h.nrays, sizeof(int), 1, fp) * sizeof(int);
  for (i=0; i<s->h.nrays; i++) {
	n += RSL_write_ray(s->ray[i], fp);
  }
  return n;
}
int RSL_write_volume(Volume *v, FILE *fp)
{
  char header_buf[512];
  int i;
  int n = 0;
  int zero = 0;

  memset(header_buf, 0, sizeof(header_buf));
  if (v == NULL) { /* Special case for missing volume.  Volume type
                    * is identified by it ordinal within the array.
		    * See DZ_INDEX, VR_INDEX, and SW_INDEX in volume.h
		    */
	/* Write a place holder volume. Just an empty header will do since
	 * the reading code is data driven.
	 */
	n += fwrite(header_buf, sizeof(header_buf), 1, fp) * sizeof(header_buf);
	n += fwrite(&zero, sizeof(int), 1, fp) * sizeof(int);
	return n;
  }

  memcpy(header_buf, &v->h, sizeof(v->h));
  n += fwrite(header_buf, sizeof(char), sizeof(header_buf), fp);

  if (radar_verbose_flag)
	fprintf(stderr,"Expect to output %d sweeps.\n", v->h.nsweeps);
  n += fwrite(&v->h.nsweeps, sizeof(int), 1, fp) * sizeof(int);

  for (i=0; i<v->h.nsweeps; i++) {
	if (radar_verbose_flag)
	  fprintf(stderr,"write_sweep %d ", i);
	n += RSL_write_sweep(v->sweep[i], fp);
  }
  return n;
}

int RSL_write_radar_fp(Radar *radar, FILE *fp)
{	 
  /* On disk each header buffer size is this big to reserve space for
   * any new members.  This will make older radar files readable as
   * development proceeds.
   */
  char header_buf[512];
  int i;
  int n = 0;
  int nradar;
  char title[100];
  
  if (radar == NULL) return 0;
  
  memset(title, 0, sizeof(title));
  snprintf(title, 100, "RSL v%s. sizeof(Range) %ld", RSL_VERSION_STR, sizeof(Range));
  n += fwrite(title, sizeof(char), sizeof(title), fp);
  
  memset(header_buf, 0, sizeof(header_buf));
  memcpy(header_buf, &radar->h, sizeof(radar->h));
  n += fwrite(header_buf, sizeof(char), sizeof(header_buf), fp);
  
  nradar = radar->h.nvolumes;
  n += fwrite(&nradar, sizeof(int), 1, fp) * sizeof(int);
  if (radar_verbose_flag)
	fprintf(stderr,"Number of volumes to write: %d\n", nradar);
  
  for (i=0; i<nradar; i++) {
	if (radar_verbose_flag)
	  fprintf(stderr,"write_volume %d ", i);
	n += RSL_write_volume(radar->v[i], fp);
	
  }
  
  if (radar_verbose_flag)
	fprintf(stderr,"write_radar done.  Wrote %d bytes.\n", n);
  return n;
}
int RSL_write_radar(Radar *radar, char *outfile)
{	 
  /* On disk each header buffer size is this big to reserve space for
   * any new members.  This will make older radar files readable as
   * development proceeds.
   */
  FILE *fp;
  int n;

  if (radar == NULL) return 0;
  
  if ((fp = fopen(outfile, "w")) == NULL) {
	perror(outfile);
	return -1;
  }
  n = RSL_write_radar_fp(radar, fp);

  (void)fclose(fp);
  
  return n;
}
int RSL_write_radar_gzip(Radar *radar, char *outfile)
{	 
  /* On disk each header buffer size is this big to reserve space for
   * any new members.  This will make older radar files readable as
   * development proceeds.
   */
  FILE *fp;
  int n;
  if (radar == NULL) return 0;
  
  if ((fp = fopen(outfile, "w")) == NULL) {
	perror(outfile);
	return -1;
  }
  fp = compress_pipe(fp);
  n = RSL_write_radar_fp(radar, fp);

  rsl_pclose(fp);
  
  return n;
}

