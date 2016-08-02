/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1999
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
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include "dorade.h"

int dorade_verbose = 0;

void dorade_verbose_on()
{
  dorade_verbose = 1;
}
void dorade_verbose_off()
{
  dorade_verbose = 0;
}

static int do_swap = 0;

/**********************************************************************/
/*                                                                    */
/*                      read_extra_bytes                              */
/*                                                                    */
/**********************************************************************/
int read_extra_bytes(int nbytes, FILE *in)
{
  char *extra;
  int nread;
  /*
   * Read and discard nbytes bytes.  The return value is the byte count
   * returned by fread, unless there is an error, then it is 0.
   */
  extra = (char *) malloc(nbytes);
  if (!extra) {
    perror("\nError: read_extra_bytes");
    fprintf(stderr,"Tried to allocate %d bytes\n", nbytes);
    return 0;
  }
  nread = fread(extra, sizeof(char), nbytes, in);
  free(extra);
  return nread;
}

/**********************************************************************/
/*                                                                    */
/*                      dorade_read_comment_block                     */
/*                                                                    */
/**********************************************************************/
Comment_block *dorade_read_comment_block(FILE *in)
{
  Comment_block *cb;
  cb = (Comment_block *) calloc(1, sizeof(Comment_block));
  if (cb == NULL) {
	perror("dorade_read_comment_block");
	return NULL;
  }
  fread(cb->code, sizeof(cb->code), 1, in);
  fread(&cb->len, sizeof(cb->len), 1, in);

  /* Check for big endian data on little endian platform.  The smallest value
   * cb->len could have is 8 (length of cb->code + cb->len), so we put that in
   * first byte of test value, and also test for negative, since sign bit could
   * be set in a larger value.
   */
  if (cb->len > 0x08000000 || cb->len < 0) do_swap = 1;
  if (do_swap) cb->len = ntohl(cb->len);

  /* Length of cb->comment is cb->len-8 since cb->code and cb->len have
   * already been read.
   */
  cb->comment = (char *) calloc(cb->len-8, sizeof(char));
  if (cb->comment == NULL) {
	perror("dorade_read_comment_block: cb->comment");
	return cb;
  }
  fread(cb->comment, sizeof(char), cb->len-8, in);
  return cb;
}

/**********************************************************************/
/*                                                                    */
/*                      dorade_read_volume_desc                       */
/*                                                                    */
/**********************************************************************/
Volume_desc    *dorade_read_volume_desc    (FILE *in)
{
  Volume_desc *vd;

  vd = (Volume_desc *) calloc(1, sizeof(Volume_desc));
  if(!vd) {
	perror("dorade_read_volume_desc");
	return NULL;
  }

  fread(vd, sizeof(Volume_desc), 1, in);
  /* Now, convert from Big Endian. */
  if (do_swap) {
      vd->len = ntohl(vd->len);
      vd->version = ntohs(vd->version);
      vd->volume_number = ntohs(vd->volume_number);
      vd->max_bytes = ntohl(vd->max_bytes);
      vd->year = ntohs(vd->year);
      vd->month = ntohs(vd->month);
      vd->day = ntohs(vd->day);
      vd->hour = ntohs(vd->hour);
      vd->minute = ntohs(vd->minute);
      vd->second = ntohs(vd->second);
      vd->gen_year = ntohs(vd->gen_year);
      vd->gen_month = ntohs(vd->gen_month);
      vd->gen_day = ntohs(vd->gen_day);
      vd->nsensors = ntohs(vd->nsensors);
  }
  return vd;
}

extern int little_endian(void);
extern void swap_4_bytes(void *word);
extern void swap_2_bytes(void *word);


/* Sensor descriptor routines. */
/**********************************************************************/
/*                                                                    */
/*                      dorade_read_radar_desc                        */
/*                                                                    */
/**********************************************************************/
Radar_desc     *dorade_read_radar_desc     (FILE *in)
{
  Radar_desc *rd;
  int i;

  rd = (Radar_desc *) calloc(1, sizeof(Radar_desc));
  if(!rd) {
	perror("dorade_read_radar_desc");
	return NULL;
  }

  fread(rd, sizeof(Radar_desc), 1, in);
  /* Now, convert from Big Endian. */
  if (do_swap) {
	swap_4_bytes(&rd->len);
	swap_4_bytes(&rd->radar_constant); /* Yes, even the ieee floating values. */
	swap_4_bytes(&rd->peak_power);
	swap_4_bytes(&rd->noise_power);
	swap_4_bytes(&rd->rcvr_gain);
	swap_4_bytes(&rd->ant_gain);
	swap_4_bytes(&rd->radar_system_gain);
	swap_4_bytes(&rd->horizontal_beam_width);
	swap_4_bytes(&rd->vertical_beam_width);
	swap_2_bytes(&rd->radar_type);
	swap_2_bytes(&rd->scan_mode);
	swap_4_bytes(&rd->scan_rate);
	swap_4_bytes(&rd->start_angle);
	swap_4_bytes(&rd->stop_angle);
	swap_2_bytes(&rd->nparam_desc);
	swap_2_bytes(&rd->ndesc);
	swap_2_bytes(&rd->compress_code);
	swap_2_bytes(&rd->compress_algo);
	swap_4_bytes(&rd->data_reduction_param1);
	swap_4_bytes(&rd->data_reduction_param2);
	swap_4_bytes(&rd->longitude);
	swap_4_bytes(&rd->latitude);
	swap_4_bytes(&rd->altitude);
	swap_4_bytes(&rd->unambiguous_velocity);
	swap_4_bytes(&rd->unambiguous_range);
	swap_2_bytes(&rd->nfreq);
	swap_2_bytes(&rd->npulse_periods);
	for (i=0; i<5; i++) {
	  swap_4_bytes(&rd->freq[i]);
	  swap_4_bytes(&rd->period[i]);
	}
  }
  /* If RADD block is longer than structure, read through extra bytes.
   * This sometimes happens.
   */
  if (rd->len > sizeof(Radar_desc)) {
      if (read_extra_bytes(rd->len - sizeof(Radar_desc), in) <= 0)
          fprintf(stderr,"Called from %s, line: %d\n", __FILE__, __LINE__ - 1);
  }
  return rd;
}
/**********************************************************************/
/*                                                                    */
/*                      dorade_read_parameter_desc                    */
/*                                                                    */
/**********************************************************************/
Parameter_desc *dorade_read_parameter_desc (FILE *in)
{
  Parameter_desc *pd;

  pd = (Parameter_desc *) calloc(1, sizeof(Parameter_desc));
  if(!pd) {
	perror("dorade_read_parameter_desc");
	return NULL;
  }

  fread(pd, sizeof(Parameter_desc), 1, in);
  /* Now, convert from Big Endian. */
  if (do_swap) {
	swap_4_bytes(&pd->len);
	swap_2_bytes(&pd->ipp);
	swap_2_bytes(&pd->xmit_freq);
	swap_4_bytes(&pd->rcvr_bandwidth);
	swap_2_bytes(&pd->pulse_width);
	swap_2_bytes(&pd->polarization);
	swap_2_bytes(&pd->nsamp_in_dwell_time);
	swap_2_bytes(&pd->parameter_type);
	swap_4_bytes(&pd->threshold_value);
	swap_4_bytes(&pd->scale_factor);
	swap_4_bytes(&pd->offset_factor);
	swap_4_bytes(&pd->missing_data_flag);
  }
  /* If the descriptor block is longer than the structure, read past the extra bytes.
   */
  if (pd->len > sizeof(Parameter_desc)) {
      if (read_extra_bytes(pd->len - sizeof(Parameter_desc), in) <= 0)
          fprintf(stderr,"Called from %s, line: %d\n", __FILE__, __LINE__ - 1);
  }
  return pd;
}

/**********************************************************************/
/*                                                                    */
/*                      dorade_read_cell_range_vector                 */
/*                                                                    */
/**********************************************************************/
Cell_range_vector      *dorade_read_cell_range_vector     (FILE *in)
{
  Cell_range_vector *cv;
  char *buff;
  int i;

  cv = (Cell_range_vector *) calloc(1, sizeof(Cell_range_vector));
  if(!cv) {
	perror("dorade_read_cell_range_vector");
	return NULL;
  }

  fread(&cv->code, sizeof(cv->code), 1, in);
  fread(&cv->len, sizeof(cv->len), 1, in);
  fread(&cv->ncells, sizeof(cv->ncells), 1, in);
  if (do_swap) {
	swap_4_bytes(&cv->len);
	swap_4_bytes(&cv->ncells);
  }
  cv->range_cell = (float *)calloc(cv->ncells, sizeof(float));
  if (!cv->range_cell) {
	perror("dorade_read_cell_range_vector: cv->range_cell");
	return cv;
  }
  fread(cv->range_cell, sizeof(float), cv->ncells, in);

  if (do_swap) {
	for (i=0; i<cv->ncells; i++)
	  swap_4_bytes(&cv->range_cell[i]);
  }

  /* Usually reading the range cells does not read to the end
   * of the Cell_range_vector structure.  We may be reading
   * a non seekable device!
   */
  i = cv->len            /* Remove a few bytes that precede. */
	- sizeof(cv->code)
	- sizeof(cv->len)
	- sizeof(cv->ncells)
	- cv->ncells*4;
  buff = (char *)malloc(i);
  if (!buff) return cv;
  fread(buff, sizeof(char), i, in);
  free(buff);
  return cv;
}

/**********************************************************************/
/*                                                                    */
/*                      dorade_read_correction_factor_desc            */
/*                                                                    */
/**********************************************************************/
Correction_factor_desc *dorade_read_correction_factor_desc(FILE *in)
{
  Correction_factor_desc *cf;
  char *remaining;
  int is_cfac = 0;

  cf = (Correction_factor_desc *) calloc(1, sizeof(Correction_factor_desc));
  if(!cf) {
	perror("dorade_read_correction_factor_desc");
	return NULL;
  }

  /* Make sure we have Correction Factor Descriptor. */
  while (!is_cfac) {
      fread(cf->code, sizeof(cf->code), 1, in);
      if (strncmp(cf->code, "CFAC", 4) == 0)
	  is_cfac = 1;
      else {
	  fread(&cf->len, sizeof(cf->len), 1, in);
	  if (do_swap) swap_4_bytes(&cf->len);
	  remaining = (char *) malloc(cf->len-8);
	  if (!remaining) {
	      perror("\ndorade_read_correction_factor_desc");
	      fprintf(stderr,"cf->len = %d\n\n", cf->len);
	      return NULL;
	  }
	  fread(remaining, sizeof(char), cf->len-8, in);
	  free(remaining);
      }
  }
  fread(&cf->len, sizeof(Correction_factor_desc)-4, 1, in);
  /* Now, convert from Big Endian. */
  if (do_swap) {
	swap_4_bytes(&cf->len);
	swap_4_bytes(&cf->azimuth);
	swap_4_bytes(&cf->elevation);
	swap_4_bytes(&cf->range);
	swap_4_bytes(&cf->longitude);
	swap_4_bytes(&cf->latitude);
	swap_4_bytes(&cf->altitude);
	swap_4_bytes(&cf->height);
	swap_4_bytes(&cf->speed_east_west);
	swap_4_bytes(&cf->speed_north_south);
	swap_4_bytes(&cf->vertical_velocity);
	swap_4_bytes(&cf->heading);
	swap_4_bytes(&cf->roll);
	swap_4_bytes(&cf->pitch);
	swap_4_bytes(&cf->drift);
	swap_4_bytes(&cf->rotation_angle);
	swap_4_bytes(&cf->tilt_angle);
  }
  return cf;
}

/**********************************************************************/
/*                                                                    */
/*                      dorade_read_sensor                            */
/*                                                                    */
/**********************************************************************/
Sensor_desc            *dorade_read_sensor (FILE *in)

	 /* Read one 'Sensor #n' descriptor from FILE. */
{
  Sensor_desc            *sd;
  int i;

  sd = (Sensor_desc *) calloc (1, sizeof(Sensor_desc));
  if (!sd) {
	perror("dorade_read_sensor");
	return NULL;
  }

  sd->radar_desc = dorade_read_radar_desc(in);
  sd->nparam = sd->radar_desc->nparam_desc;

  sd->p_desc = (Parameter_desc **) calloc(sd->nparam, sizeof(Parameter_desc *));
  if (!sd->p_desc) {
	perror("dorade_read_sensor: sd->p_desc");
	return sd;
  }
  for (i=0; i<sd->nparam; i++) {
	sd->p_desc[i] = dorade_read_parameter_desc(in);
  }

  sd->cell_range_vector = dorade_read_cell_range_vector(in);
  sd->correction_factor_desc = dorade_read_correction_factor_desc(in);
  return sd;
}


/**********************************************************************/
/*                                                                    */
/*                      dorade_read_sweep_info                        */
/*                                                                    */
/**********************************************************************/
Sweep_info *dorade_read_sweep_info(FILE *in)
{
  Sweep_info *si;

  si = (Sweep_info *) calloc(1, sizeof(Sweep_info));
  if(!si) {
	perror("dorade_read_sweep_info");
	return NULL;
  }

  fread(si, sizeof(Sweep_info), 1, in);
  /* FIXME: ?? For now, VOLD is what we expect when there
   *           are no more SWIB.  This is a data driven EOF.
   *           Returning NULL should suffice.
   */
  if(strncmp(si->code, "SWIB", 4) != 0) {
	/* Ignore the rest of the file. */
	free(si);
	return NULL;
  }

  /* Now, convert from Big Endian. */
  if (do_swap) {
	swap_4_bytes(&si->len);
	swap_4_bytes(&si->sweep_num);
	swap_4_bytes(&si->nrays);
	swap_4_bytes(&si->start_angle);
	swap_4_bytes(&si->stop_angle);
	swap_4_bytes(&si->fixed_angle);
	swap_4_bytes(&si->filter_flag);
  }

  return si;
}

/* Data Ray routines. */

/**********************************************************************/
/*                                                                    */
/*                      dorade_read_ray_info                          */
/*                                                                    */
/**********************************************************************/
Ray_info       *dorade_read_ray_info      (FILE *in)
{
  Ray_info *ri;

  ri = (Ray_info *) calloc(1, sizeof(Ray_info));
  if(!ri) {
	perror("dorade_read_ray_info");
	return NULL;
  }

  fread(ri, sizeof(Ray_info), 1, in);
  /* Now, convert from Big Endian. */
  if (do_swap) {
	swap_4_bytes(&ri->len);
	swap_4_bytes(&ri->sweep_num);
	swap_4_bytes(&ri->jday);
	swap_2_bytes(&ri->hour);
	swap_2_bytes(&ri->minute);
	swap_2_bytes(&ri->second);
	swap_2_bytes(&ri->msec);
	swap_4_bytes(&ri->azimuth);
	swap_4_bytes(&ri->elevation);
	swap_4_bytes(&ri->peak_power);
	swap_4_bytes(&ri->scan_rate);
	swap_4_bytes(&ri->status);
  }

  return ri;
}

/**********************************************************************/
/*                                                                    */
/*                      dorade_read_platform_info                     */
/*                                                                    */
/**********************************************************************/
Platform_info  *dorade_read_platform_info (FILE *in)
{
  Platform_info *pi;
  int len_first_two;

  pi = (Platform_info *) calloc(1, sizeof(Platform_info));
  if(!pi) {
	perror("dorade_read_platform_info");
	return NULL;
  }

  /* Read the id code to make sure we have "ASIB" for platform info.  If
   * id is ASIB, then read data into the Platform_info structure.  If it is
   * XSTF, read and discard remainder of block, which will have a different
   * size than Platform_info.  XSTF is undocumented, but apparently it takes
   * the place of ASIB when radar is grounded.
   */

  fread(pi->code, sizeof(pi->code), 1, in);
  fread(&pi->len, sizeof(pi->len), 1, in);
  if (do_swap) swap_4_bytes(&pi->len);
  len_first_two = sizeof(pi->code) + sizeof(pi->len);
    
  if (strncmp(pi->code, "ASIB", 4) == 0) {
      fread(&pi->longitude, sizeof(Platform_info)-len_first_two, 1, in);
      /* Read past any extra bytes. */
      if (pi->len > sizeof(Platform_info)) {
	  if (read_extra_bytes(pi->len - sizeof(Platform_info), in) <= 0)
	      fprintf(stderr,"Called from %s, line: %d\n",__FILE__,__LINE__-1);
      }
      /* Now, convert from Big Endian. */
      if (do_swap) {
	  swap_4_bytes(&pi->longitude);
	  swap_4_bytes(&pi->latitude);
	  swap_4_bytes(&pi->altitude);
	  swap_4_bytes(&pi->height);
	  swap_4_bytes(&pi->ew_speed);
	  swap_4_bytes(&pi->ns_speed);
	  swap_4_bytes(&pi->v_speed);
	  swap_4_bytes(&pi->heading);
	  swap_4_bytes(&pi->roll);
	  swap_4_bytes(&pi->pitch);
	  swap_4_bytes(&pi->drift);
	  swap_4_bytes(&pi->rotation);
	  swap_4_bytes(&pi->tilt);
	  swap_4_bytes(&pi->ew_wind_speed);
	  swap_4_bytes(&pi->ns_wind_speed);
	  swap_4_bytes(&pi->v_wind_speed);
	  swap_4_bytes(&pi->heading_rate);
	  swap_4_bytes(&pi->pitch_rate);
      }
  } else if (strncmp(pi->code, "XSTF", 4) == 0) {
      /* Read to end of XSTF block. */
      if (read_extra_bytes(pi->len - len_first_two, in) <= 0)
          fprintf(stderr,"Called from %s, line: %d\n", __FILE__, __LINE__ - 1);
  } else {
      fprintf(stderr,"Unexpected block id: \"%s\"."
          "  Expected \"ASIB\" or \"XSTF\"\n", pi->code);
  }

  return pi;
}

/**********************************************************************/
/*                                                                    */
/*                      dorade_read_parameter_data                    */
/*                                                                    */
/**********************************************************************/

Parameter_data *dorade_read_parameter_data(FILE *in)
{
  Parameter_data *pd;
  int len;

  pd = (Parameter_data *) calloc(1, sizeof(Parameter_data));
  if(!pd) {
	perror("dorade_read_parameter_data: pd");
	return NULL;
  }

  fread(&pd->code, sizeof(pd->code), 1, in);
  fread(&pd->len, sizeof(pd->len), 1, in);
  fread(&pd->name, sizeof(pd->name), 1, in);
  if (do_swap) swap_4_bytes(&pd->len);
  /* Length is in parameter data block? or calculate if from pd->len. */

  len = pd->len  /* Use pd->len for now. */
	- sizeof(pd->code) /* Remove a few bytes from */
	- sizeof(pd->len)  /* the count.              */
	- sizeof(pd->name);
  pd->data = (char *)calloc(len, sizeof(char));
  if (!pd->data) {
	perror("dorade_read_parameter_data: pd->data");
	return pd;
  }
  fread(pd->data, sizeof(char), len, in);
  
  /* FIXME: Big endian conversion in caller?  Is that the right place? */

  return pd;
}
/**********************************************************************/
/*                                                                    */
/*                      dorade_read_sweep                             */
/*                                                                    */
/**********************************************************************/
Sweep_record *dorade_read_sweep(FILE *fp, Sensor_desc **sd)
{
  Sweep_record   *sr;

  Sweep_info     *si;
  Ray_info       *ri;
  Platform_info  *pi;
  Parameter_data *pd;
  Parameter_desc **parameter_desc;

  int i, j, k,len;
  int nparam;

  sr = (Sweep_record *) calloc (1, sizeof(Sweep_record));
  if (!sr) {
	perror("dorade_read_sweep");
	return NULL;
  }

  nparam         = sd[0]->nparam;
  parameter_desc = sd[0]->p_desc;

 /* Expect SWIB */
  sr->s_info = si = dorade_read_sweep_info(fp);
  if (!si) {
	free(sr);
	return NULL;  /* EOF or error. */
  }
  sr->nrays = si->nrays;
  if (dorade_verbose) {
	printf("=====< NEW SWIB >=====\n");
	dorade_print_sweep_info(si);
  }
  sr->data_ray = (Data_ray **) calloc(si->nrays, sizeof(Data_ray *));
  if (!sr->data_ray) {
	free(sr);
	return NULL;  /* EOF or error. */
  }

  for (i=0; i<si->nrays; i++) {
	if (dorade_verbose) printf("---------- Ray %d ----------\n", i);
	sr->data_ray[i] = (Data_ray *) calloc(1, sizeof(Data_ray));
	if (!sr->data_ray[0]) {
	  free(sr);
	  return NULL;  /* EOF or error. */
	}
	ri = dorade_read_ray_info(fp);
	if (dorade_verbose) {
	  dorade_print_ray_info(ri);
	}
	pi = dorade_read_platform_info(fp);
	if (dorade_verbose) {
	  dorade_print_platform_info(pi);
	}
	sr->data_ray[i]->ray_info = ri;
	sr->data_ray[i]->platform_info = pi;
	sr->data_ray[i]->parameter_data = (Parameter_data **) calloc(nparam, sizeof(Parameter_data *));
	sr->data_ray[i]->data_len  = (int *) calloc(nparam, sizeof(int));
	sr->data_ray[i]->word_size = (int *) calloc(nparam, sizeof(int));
	sr->data_ray[i]->nparam    = nparam;

	for (j=0; j<nparam; j++) {
	  pd = dorade_read_parameter_data(fp);
	  /* Perform big endian conversion. */
	  len = pd->len  /* Use pd->len for now. */
		- sizeof(pd->code) /* Remove a few bytes from */
		- sizeof(pd->len)  /* the count.              */
		- sizeof(pd->name);
	  sr->data_ray[i]->parameter_data[j] = pd;
	  sr->data_ray[i]->data_len[j] = len;
	  if (parameter_desc[j]->parameter_type == 2)
		sr->data_ray[i]->word_size[j] = 2; /* 2 bytes per word */
	  else if (parameter_desc[j]->parameter_type == 3 ||
			   parameter_desc[j]->parameter_type == 4)
		sr->data_ray[i]->word_size[j] = 4; /* 4 bytes per word */
		
	  if (do_swap) { /* Numbers were read big-endian. */
		if (sr->data_ray[i]->word_size[j] == 2)
		  for (k=0; k<len; k+=2)
			swap_2_bytes(&pd->data[k]);
		else if (sr->data_ray[i]->word_size[j] == 4)
		  for (k=0; k<len; k+=4)
			swap_4_bytes(&pd->data[k]);
	  }
	}
  }
  return sr;
}


/* MEMORY MANAGEMENT ROUTINES */

/**********************************************************************/
/*                                                                    */
/*                      dorade_free_data_ray                          */
/*                                                                    */
/**********************************************************************/
void dorade_free_data_ray(Data_ray *r)
{
  int i;
  if (r == NULL) return;

  free(r->ray_info);
  free(r->platform_info);
  if (r->parameter_data) {
	for (i=0; i<r->nparam; i++)
	  free(r->parameter_data[i]);
	free(r->parameter_data);
  }
  free(r);
}

/**********************************************************************/
/*                                                                    */
/*                      dorade_free_sweep                             */
/*                                                                    */
/**********************************************************************/
void dorade_free_sweep(Sweep_record *s)
{
  int i;
  if (s == NULL) return;
  
  if (s->data_ray) {
	for (i=0; i<s->nrays; i++)
	  dorade_free_data_ray(s->data_ray[i]);
	free(s->data_ray);
  }
  if (s->s_info) free(s->s_info);
  free(s);
}
