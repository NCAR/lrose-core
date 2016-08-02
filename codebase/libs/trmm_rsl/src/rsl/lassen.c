#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LASSEN
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
/*----------------------------------------------------------------------
 * PURPOSE:
 *
 * Ingest a Lassen file and fill the Lassen_volume data structure.
 * It can handle version 1.3 and 1.4 files (MCTEX data).
 *
 * The algorthm is pretty simple, call an 'xdr' routine each time for
 * each and every member of Lassen_volume and all substructures.
 *
 * The order and type of xdr routine called defines the file organization.
 *----------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include <netinet/in.h>
#include "lassen.h"

/* xdr_destroy is broken on HPUX, SGI, and SUN; Linux, the only working one? */
/* This broken behavior could be from old xdr versions, too. */
#undef BROKEN_XDR_DESTROY
#if defined(__hpux) || defined(__sgi) || defined(__sun)
#define BROKEN_XDR_DESTROY
#endif
/*************************************************************/
/*                                                           */
/*                     read_lassen_head                      */
/*                                                           */
/*************************************************************/
int read_lassen_head(FILE *f, Lassen_head *head)
{
  int rc=1, i;
  char *tmp;
  XDR xdrs;
  
  memset(head->magic, 0, sizeof(head->magic));
  
  xdrstdio_create(&xdrs, f, XDR_DECODE);
  
  /*
   * Can I read the first string? If not, then this is probably
   * not a valid header.  If this happens, then return.
   */
  tmp=head->magic;
  if((rc &= xdr_string(&xdrs, &tmp, 8))==0) {
#ifndef BROKEN_XDR_DESTROY
	xdr_destroy(&xdrs);
#endif
	return(0);
  }
  rc &= xdr_u_char(&xdrs, &head->mdate.year);
  rc &= xdr_u_char(&xdrs, &head->mdate.month);
  rc &= xdr_u_char(&xdrs, &head->mdate.day);
  rc &= xdr_u_char(&xdrs, &head->mdate.hour);
  rc &= xdr_u_char(&xdrs, &head->mdate.minute);
  rc &= xdr_u_char(&xdrs, &head->mdate.second);
  rc &= xdr_u_char(&xdrs, &head->cdate.year);
  rc &= xdr_u_char(&xdrs, &head->cdate.month);
  rc &= xdr_u_char(&xdrs, &head->cdate.day);
  rc &= xdr_u_char(&xdrs, &head->cdate.hour);
  rc &= xdr_u_char(&xdrs, &head->cdate.minute);
  rc &= xdr_u_char(&xdrs, &head->cdate.second);
  rc &= xdr_int(&xdrs, &head->type);
  tmp=head->mwho;
  rc &= xdr_string(&xdrs, &tmp, 16);
  tmp=head->cwho;
  rc &= xdr_string(&xdrs, &tmp, 16);
  rc &= xdr_int(&xdrs, &head->protection);
  rc &= xdr_int(&xdrs, &head->checksum);
  tmp=head->description;
  rc &= xdr_string(&xdrs, &tmp, 40);
  rc &= xdr_int(&xdrs, &head->id);
  for(i=0;i<12;i++)
	rc &= xdr_int(&xdrs, &head->spare[i]);

#ifndef BROKEN_XDR_DESTROY
  xdr_destroy(&xdrs);
#endif
  
  return rc;
}

/*************************************************************/
/*                                                           */
/*                   free_lassen_volume                      */
/*                                                           */
/*************************************************************/

void free_lassen_volume(Lassen_volume *vol)
{
  int i,j;

  if (vol == NULL) return;
  for(i=0; i<vol->numsweeps; i++ ) {
	if(vol->index[i] == NULL) continue;
	for(j=0; j<vol->index[i]->numrays; j++ )
	  if (vol->index[i]->ray[j] != NULL)
		free( vol->index[i]->ray[j] );
	free( vol->index[i] );
  }
  free( vol );
  return;
}

static void short_shift(unsigned short *n)
{
  *n = ntohs(*n);
}

/*************************************************************/
/*                                                           */
/*                   read_lassen_volume                      */
/*                                                           */
/*************************************************************/
int read_lassen_volume( XDR *xdr, Lassen_volume *vol)
{
  int	i,j;
  int	rc = TRUE;
  char		*tmp;
  unsigned short	ushort_tmp;

  /*  Read the version number. */
  rc &= xdr_u_short(xdr, &vol->version);
  
  /*
   *  Currently, Version 1.3 and 1.4 seem to be the same on disk.
   *  I have no doc's.  The only thing that I know of to be different
   *  is how the ray azimuth angles are calculated.  For MCTEX data,
   *  this is time dependant.  The correction will happen in the 
   *  application; after RSL_lassen_to_radar is called, for example.
   *
   *  Version 1.3 is coded as 13 and version 1.4 is coded as 14.
   */
  if( (int)vol->version != 13 && (int)vol->version != 14 ) {
	(void)fprintf(stderr, "Incompatible version of LASSEN file.\n" );
	(void)fprintf(stderr, "File version: %d.%d\n",
				  (int)vol->version/10, (int)vol->version%10);
	return( 0 );
  }
  
  /*  Read in the Lassen_volume.  */
  rc &= xdr_short(xdr, &vol->filled);
  rc &= xdr_u_int(xdr, &vol->volume);
  rc &= xdr_u_short(xdr, &vol->sweep);
  rc &= xdr_u_short(xdr, &vol->sweep_type);
  rc &= xdr_u_short(xdr, &vol->max_height);
  rc &= xdr_u_short(xdr, &vol->status);
  rc &= xdr_u_short(xdr, &vol->min_fangle);
  rc &= xdr_u_short(xdr, &vol->max_fangle);
  rc &= xdr_u_short(xdr, &vol->min_var);
  rc &= xdr_u_short(xdr, &vol->max_var);
  rc &= xdr_u_short(xdr, &vol->a_start);
  rc &= xdr_u_short(xdr, &vol->a_stop);
  rc &= xdr_u_short(xdr, &vol->numsweeps);
  for(i=0;i<LASSEN_MAX_SWEEPS;i++)
	rc &= xdr_u_short(xdr, &vol->fangles[i]);
  rc &= xdr_u_short(xdr, &vol->gatewid);
  rc &= xdr_u_short(xdr, &vol->rangeg1);
  for(i=0;i<LASSEN_MAX_SWEEPS;i++)
	rc &= xdr_u_short(xdr, &vol->numgates[i]);
  rc &= xdr_u_short(xdr, &vol->maxgates);
  rc &= xdr_u_short(xdr, &ushort_tmp);
  rc &= xdr_u_short(xdr, &ushort_tmp);
  rc &= xdr_u_short(xdr, &ushort_tmp);
  rc &= xdr_u_short(xdr, &ushort_tmp);
  rc &= xdr_u_short(xdr, &vol->prf);
  rc &= xdr_u_short(xdr, &vol->prflow);
  rc &= xdr_u_int(xdr, &vol->freq);
  rc &= xdr_u_short(xdr, &vol->n_pulses);
  
  for(i=0;i<LASSEN_MAX_SWEEPS;i++)
	for(j=0;j<NUMOFFSETS;j++)
	  rc &= xdr_u_short(xdr, &vol->offset[i][j]);
  
  rc &= xdr_u_char(xdr, &vol->year);
  rc &= xdr_u_char(xdr, &vol->month);
  rc &= xdr_u_char(xdr, &vol->day);
  rc &= xdr_u_char(xdr, &vol->shour);
  rc &= xdr_u_char(xdr, &vol->sminute);
  rc &= xdr_u_char(xdr, &vol->ssecond);
  rc &= xdr_u_char(xdr, &vol->ehour);
  rc &= xdr_u_char(xdr, &vol->eminute);
  rc &= xdr_u_char(xdr, &vol->esecond);
  /* The flags use bits.  The data type length is short; I expect. */
  rc &= xdr_u_short(xdr, (unsigned short *)&vol->volflags);
  short_shift((unsigned short *)&vol->volflags);
  /* This is radar info. */
  tmp=vol->radinfo.radar_name;
  rc &= xdr_string(xdr, &tmp, 8);
  tmp=vol->radinfo.site_name;
  rc &= xdr_string(xdr, &tmp, 8);
  rc &= xdr_u_short(xdr, &vol->radinfo.antenna_height);
  rc &= xdr_short(xdr, &vol->radinfo.latitude.degree);
  rc &= xdr_short(xdr, &vol->radinfo.latitude.minute);
  rc &= xdr_short(xdr, &vol->radinfo.latitude.second);
  rc &= xdr_short(xdr, &vol->radinfo.longitude.degree);
  rc &= xdr_short(xdr, &vol->radinfo.longitude.minute);
  rc &= xdr_short(xdr, &vol->radinfo.longitude.second);
  
  /*
   *  The return value is the logical and of all the xdr reads.
   */
  return( rc );
}

/*************************************************************/
/*                                                           */
/*                   read_lassen_ray                         */
/*                                                           */
/*************************************************************/
int read_lassen_ray(XDR *xdr, Lassen_ray *ray )
{
	int	i, rc = 1;

	/*  Read in Lassen_ray */
	rc &= xdr_u_short( xdr, &ray->vangle);
	rc &= xdr_u_short( xdr, &ray->fanglet);
	rc &= xdr_u_short( xdr, &ray->fanglea);
	rc &= xdr_u_short( xdr, &ray->a_start);
	rc &= xdr_u_short( xdr, &ray->a_stop);
	rc &= xdr_u_char( xdr, &ray->max_height);
	rc &= xdr_u_char( xdr, &ray->volume);
	rc &= xdr_u_char( xdr, &ray->sweep);
	rc &= xdr_u_char( xdr, &ray->sweep_type);
	rc &= xdr_u_short( xdr, &ray->gatewid);
	rc &= xdr_u_short( xdr, &ray->rangeg1);
	rc &= xdr_u_short( xdr, &ray->numgates);
	rc &= xdr_u_short( xdr, &ray->prf);
	rc &= xdr_u_short( xdr, &ray->prflow);
	rc &= xdr_u_short( xdr, &ray->n_pulses);
	rc &= xdr_u_char( xdr, &ray->p_width);
	rc &= xdr_u_char( xdr, &ray->cfilter);
	rc &= xdr_u_short( xdr, &ray->status);
	memset(&ray->flags, 0, 4);
	/* I expect the length of the bit field to be a 'short', never 'long' */
	rc &= xdr_u_short( xdr, (unsigned short *)&ray->flags);	/* a bit field */
	short_shift((unsigned short *)&ray->flags);
	/*
	fprintf(stderr, "short_shift: flags = %x\n", ray->flags);
	fprintf(stderr, "packed = %d", (int)ray->flags.packed);
	fprintf(stderr, "good_data = %d", (int)ray->flags.good_data);
	fprintf(stderr, "uz = %d", (int)ray->flags.uz);
	fprintf(stderr, "cz = %d", (int)ray->flags.cz);
	fprintf(stderr, "vel= %d", (int)ray->flags.vel);
	fprintf(stderr, "wid= %d", (int)ray->flags.wid);
	fprintf(stderr, "zdr= %d", (int)ray->flags.zdr);
	fprintf(stderr, "phi= %d", (int)ray->flags.phi);
	fprintf(stderr, "rho= %d", (int)ray->flags.rho);
	fprintf(stderr, "ldr= %d", (int)ray->flags.ldr);
	fprintf(stderr, "kdp= %d", (int)ray->flags.kdp);
	fprintf(stderr, "tim= %d", (int)ray->flags.time);
	fprintf(stderr, "spa= %x", (int)ray->flags.spares);
	*/
	for(i=0;i<NUMOFFSETS;i++)
		rc &= xdr_u_short( xdr, &ray->offset[i]);

	rc &= xdr_u_char( xdr, &ray->year);
	rc &= xdr_u_char( xdr, &ray->month);
	rc &= xdr_u_char( xdr, &ray->day);
	rc &= xdr_u_char( xdr, &ray->hour);
	rc &= xdr_u_char( xdr, &ray->minute);
	rc &= xdr_u_char( xdr, &ray->second);

	return rc;
}


/*************************************************************/
/*                                                           */
/*                  read_entire_lassen_file                  */
/*                                                           */
/*************************************************************/
int read_entire_lassen_file(FILE *f, Lassen_volume *vol)
{
  Lassen_sweep	*sweep;
  int i;
  XDR				xdr;
  Lassen_ray		ray;
  Lassen_head		head;
  int				isweep = (-99);
  int				lastray=0,numrays=0, rc;
  unsigned int      size;
  int tbytes = 0;
  unsigned char *p;


  /* Skip the header. */
  read_lassen_head(f, &head);

  /*  Connect to the XDR stream. */
  xdrstdio_create(&xdr, f, XDR_DECODE);

  /*  Check the volume header. Is the version is correct? */
  if( read_lassen_volume( &xdr, vol ) == 0) {
#ifndef BROKEN_XDR_DESTROY
	xdr_destroy(&xdr);
#endif
	return( 0 );
  }

  rc = TRUE;

  /*  Read until XDR error. */
  while(rc == TRUE) {

	/*
	 *  Read in the ray header.  If there is an error in reading
	 *	in the ray header, then make sure the rc value is
	 *	set to 0.
	 */
	if( read_lassen_ray( &xdr, &ray ) == 0 )
	  break;

	if( (short)ANGLE_CONVERT(ray.vangle) < 0 ||
		(short)ANGLE_CONVERT(ray.vangle) > 360 ) {
	  fprintf(stderr, "read_entire_lassen_file: angle out of range (%d)\n",
			  ANGLE_CONVERT(ray.vangle));
	  rc = 0;
	  break;
	}

	if(ray.volume != (vol->volume&0x00ff)) {
	  fprintf(stderr, "read_entire_lassen_file: Volume serial number out of sync:\n");
	  fprintf(stderr, "         Ray->volume = %u, Should be %u\n",
			  ray.volume, (unsigned char)vol->volume&0x00ff );
	  fprintf(stderr, "         (Ray->volume == Vol->volume(%d) & 0x00ff)\n",
			  vol->volume );
	  if(isweep == -99) rc = 0;
	  break;
	}

	/*
	 * If we have a new sweep, then create
	 * the new sweep data structure.  The 'isweep'
	 * contains the offset number in the volume. 'ray.sweep'
     * has the sweep number.
	 */
	if((ray.sweep-1) != isweep) {
	  if(ray.sweep == 0) {
		fprintf(stderr,"Sweep number in a ray is 0!\n");
		fprintf(stderr,"Aborting read_entire_lassen_file()\n");
#ifndef BROKEN_XDR_DESTROY
		xdr_destroy(&xdr);
#endif
		rc = 0;
		break;
	  }
	  
	  
	  isweep = ray.sweep-1;
	  
	  if(isweep > 0) {
		/* Reset the last ray when passing 2 pi. */
		lastray--;
		if( lastray < 0 ) lastray = 359;
		
		sweep = vol->index[isweep-1];
		if (sweep == NULL) {
		  rc = 0;
		  fprintf(stderr, "vol->index[%d] is NULL.\n", isweep-1);
		  break;
		}
		sweep->ehour   = sweep->ray[lastray]->hour;
		sweep->eminute = sweep->ray[lastray]->minute;
		sweep->esecond = sweep->ray[lastray]->second;
		sweep->numrays = numrays;
		sweep->max_var = sweep->ray[lastray]->vangle;
		
	  }
	  
	  /* We have a new sweep. */
	  if((vol->index[isweep] = (Lassen_sweep *)calloc
		  (1,sizeof(Lassen_sweep))) == 0) {
		perror("read_entire_lassen_file");
		rc = 0;
		break;
	  }

	  lastray = 0;
	  
	  /* Copy to the Sweep. */
	  sweep          = vol->index[isweep];
	  sweep->sweep   = isweep;
	  sweep->shour   = ray.hour;
	  sweep->sminute = ray.minute;
	  sweep->ssecond = ray.second;
	  sweep->volume  = ray.volume;
	  sweep->sweep   = ray.sweep;
	  sweep->sweep_type = ray.sweep_type;
	  sweep->max_height = ray.max_height;
	  sweep->fangle     = ray.fanglet;
	  sweep->a_start    = ray.a_start;
	  sweep->a_stop     = ray.a_stop;
	  sweep->gatewid    = ray.gatewid;
	  sweep->rangeg1    = ray.rangeg1;
	  sweep->numgates   = ray.numgates;
	  sweep->prf        = ray.prf;
	  sweep->prflow     = ray.prflow;
	  sweep->n_pulses   = ray.n_pulses;
	  sweep->p_width    = ray.p_width;
	  sweep->cfilter    = ray.cfilter;
	  for(i=0; i<NUMOFFSETS; i++)
		sweep->offset[i] = ray.offset[i];
	  sweep->year    = ray.year;
	  sweep->month   = ray.month;
	  sweep->day     = ray.day;
	  sweep->min_var = ray.vangle;
	  
	  numrays = 0;
	}
	
	/*
	 * If everything is ok, then add the ray to the
	 * volume.
	 */
	numrays++;
	vol->index[isweep]->status |= ray.status;
	
	/* Determine the 'size' of the ray and allocate memory. */
	size = 0;
	/*			fprintf(stderr,"\nOFFSETS (%d):", NUMOFFSETS); */
	for(i=0; i<NUMOFFSETS; i++) {
	  size += (ray.numgates*(ray.offset[i] !=0 ));
	  /*				fprintf(stderr, " %d", ray.numgates*(ray.offset[i])); */
	}

	if((vol->index[isweep]->ray[lastray] = 
		(Lassen_ray *) calloc(1,sizeof(Lassen_ray)+size))==0) {
	  perror("read_entire_lassen_file");
	  rc = 0;
#ifndef BROKEN_XDR_DESTROY
	  xdr_destroy(&xdr);
#endif
	  break;
	}
	
	
	p = (unsigned char *)vol->index[isweep]->ray[lastray];
	
	/* Copy the ray header. */
	memcpy(p, &ray, sizeof(Lassen_ray));
	p = (unsigned char *)p + sizeof(Lassen_ray);

	/* Read the data. */
	/*			printf("Previous rc=%d ... ", rc); */
	rc &= xdr_bytes(&xdr, (char **)&p, &size,size);
	tbytes += size;
	/*			printf("after rc=%d.  xdr_bytes %d bytes, tbytes %d.\n", rc, size, tbytes);
	 */
	lastray++;
	if(lastray > 359) lastray = 0;
  } /* End while. */

  if(rc == 0) {
	/* Clean up and get out. */
	free_lassen_volume(vol);
#ifndef BROKEN_XDR_DESTROY
	xdr_destroy(&xdr);
#endif
	fprintf(stderr,"\tAborting read_entire_lassen_file.\n\n");
	return(0);
  }

  /*
   * Check to see if we read a sweep!
   */
  if( isweep == -99 )
	vol->filled = -1;
  else {
	lastray--;
	if( lastray < 0 ) lastray = 359;
	
	/* Update the last sweep. */
	sweep          = vol->index[isweep];
	sweep->ehour   = sweep->ray[lastray]->hour;
	sweep->eminute = sweep->ray[lastray]->minute;
	sweep->esecond = sweep->ray[lastray]->second;
	sweep->max_var = ray.vangle;
	sweep->numrays = numrays;
	vol->filled = 1;
  }
  
  /* Check the number of tilts read vs. the number in the file. */
  if(vol->numsweeps != isweep+1) {
	fprintf(stderr, "read_entire_lassen_file: Warning, read error.  The number of sweeps ");
	fprintf(stderr, "detected is different from the number written.");
  }
  
  vol->numsweeps = isweep+1;
  
#ifndef BROKEN_XDR_DESTROY
  xdr_destroy(&xdr);
#endif  
  return( 1 );
}
#endif
