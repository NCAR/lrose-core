/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1997
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
#include <string.h>
#include "africa.h"

int africa_read_buffer(FILE *fp, Africa_buffer *buffer)
{
  int n;

  n = fread(buffer, 1, sizeof(Africa_buffer), fp);
  return n;
}

float africa_bcd_convert(unsigned short bcd)
{
  int val;

  val = 
      ((bcd >> 12) & 0xf) * 1000
	+ ((bcd >> 8)  & 0xf) * 100
	+ ((bcd >> 4)  & 0xf) * 10
	+ ( bcd        & 0xf);
  return val/10.0;
}

Africa_sweep * africa_new_sweep(int nray)
{
  Africa_sweep *sweep;

  sweep = (Africa_sweep *)calloc(nray, sizeof(Africa_sweep));
  if (! sweep) {
	perror("africa_new_sweep:sweep");
	return NULL;
  }
  sweep->nrays = nray;
  sweep->ray = (Africa_ray **) calloc(nray, sizeof(Africa_ray *));
  if (! sweep -> ray) {
	perror("africa_new_sweep:ray");
	return NULL;
  }
  return sweep;
}

Africa_ray *africa_new_ray(void)
{
  Africa_ray *ray;
  ray = (Africa_ray *)calloc(1, sizeof(Africa_ray));
  if (! ray) {
	perror("africa_new_ray:ray");
	return NULL;
  }
  return ray;
}
  
void africa_free_ray(Africa_ray *r)
{
  if (! r) return;
  free(r);
}

void africa_free_sweep(Africa_sweep *s)
{
  int i;
  if (! s) return;
  if (! s->ray) return;
  for (i=0; i<s->nrays; i++) {
	africa_free_ray(s->ray[i]);
  }
  free(s->ray);
  free(s);
}

Africa_sweep *africa_read_sweep(FILE *fp)
{
  /* This contains the next ray of data, except
   * when this is the first ray.  This is a
   * read-ahead buffer.
   */
  static Africa_buffer *buf = NULL; /* The read ahead buffer, too. */
  Africa_sweep *sweep = NULL;
  int cur_elev, ielev, iray;
  Africa_ray *ray = NULL;
  int nray;
  
  if (! buf)	{
	buf = (Africa_buffer *) calloc(1, sizeof(Africa_buffer));
	if (! buf) {
	  perror("allocate buf in read_sweep");
	  return NULL;
	}
	/* (pre)Read a record */
	africa_read_buffer(fp, buf); /* allocates space for buffer */

  }
  /* Allocate 500 (should be enough) ray pointers. */
  
  sweep = africa_new_sweep(1000); /* 500 times 2 field types ? */
  /* Determine the elevation step we're on so we know when to
   * return the sweep.  Basically, the last read will be the 
   * data for the next sweep; the meaning of the above code.
   */
  cur_elev = ielev = buf->raycount/512;
  nray = 0;
  while(cur_elev == ielev) {
	iray  = buf->raycount - cur_elev*512;
	if (iray != 0) {
	  ray = sweep->ray[nray];
	  if (!ray) sweep->ray[nray] = ray = africa_new_ray();
	  memcpy(ray, buf, sizeof(Africa_buffer));
	  nray++;
	}
	if (africa_read_buffer(fp, buf) == 0) break;
	cur_elev = buf->raycount/512;
	/*	isite    = buf->xmit_power_site & 0x1f; */
  }
  if (nray == 0) return NULL;
  sweep->nrays = nray;

  return sweep;
}
