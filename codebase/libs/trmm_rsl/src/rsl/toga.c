/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1992  Dennis F. Flanigan Jr. of Applied Research Corporation,
                        Landover, Maryland, a NASA/GSFC on-site contractor.

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
/* toga/old sigmet/Darwin access routines
 * V1.0 12/15/93   by John Merritt.
 *
 *  1. Use CFLAGS = -DUSE_PLOG if you want to use the PLOG library.
 *----------------------------------------------------------------------
 *
 * Dennis Flanigan, Jr.
 * Applied Research Corp.
 * NASA GSFC Code 910.1
 * 
 *
 * 15 Jul 93
 * Added tg_prt_head function
 *
 * 28 Jun 93
 * tg_file_str added
 *
 * 09 Jun 93
 * Added tg_open function    ...Mike
 *
 * 5/10/93
 * Modified tg_read_ray and created tg_decode_ray_data .
 * tg_read_ray now returns decoded real-valued ray data in a 
 * tg_ray_data structure instead of toga-format-encoded ray data
 * in a rp_ray structure.      Mike
 *
 * 8/13/92
 * Made changes so that code can be generated in library routines.
 * Library will be called libtg.a
 *
 * 7/10/92
 * Routines to access toga data from Darwin.   These routines work
 * with the old sigmet data format that was used in Darwin from 87 
 * to 91.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "toga.h"

#ifdef USE_PLOG
#include "plog.h"
#endif

#if defined(__linux)
void swab(const void *from, void *to, size_t n);
#endif
int tg_open(char *,tg_file_str *);
int tg_read_map_head(tg_file_str *);
float tg_make_ang(unsigned short);
int tg_read_map_bytes(tg_file_str *,void *,int);
int tg_read_rec_bytes(tg_file_str *,char *,int);
int tg_read_map_rec(tg_file_str *);
void tg_decode_ray_data(tg_file_str *,short *);
int tg_read_ray(tg_file_str *);
void tg_prt_head(tg_map_head_str *,int);

FILE *uncompress_pipe (FILE *fp);


int tg_open(char *filename,tg_file_str *tg_file)
   {
   /* open the toga data file */
   if (filename == NULL) tg_file->fd = STDIN_FILENO; /* Stdin */
   else 
	 if ((tg_file->fd=open(filename,O_RDONLY)) == -1)
	   {
#ifdef USE_PLOG
		 plog("tg_open: Error opening toga data file\n",PLOG_P);
#endif
		 return(TG_SYS_ERR);
	   }
   /* Unfortunately, there is no tg_close to modularize the following
    * pipe close.  Shouldn't be any problems anyway.
    */
   (void) uncompress_pipe(fdopen(tg_file->fd, "r")); /* Redirect through gunzip. */
   /* initialize buffer pointers, flags */
   tg_file->buf_ind = 32769;
   tg_file->buf_end = 32769;
   tg_file->first_rec = TRUE;
   tg_file->data_ind = 2044;

   /* read the map header from the toga file into the tg_file 
	  map_head str */
   if (tg_read_map_head(tg_file) < 0)
	  {
	  return(-1);  /* Can't read toga map header */
	  }

   return(TG_OK);
   }



int tg_read_map_head(tg_file_str *tg_file)
   {
   int n;
   tg_map_head_str buf;

   if((n = read(tg_file->fd,&buf,TG_HDSIZE)) != TG_HDSIZE)
	  {
	  if (n < 0)
		 {
		 fprintf(stderr,"tg_read_map_head: (%d)%s \n",errno,strerror(errno));
		 }
	  else
		 {
		 fprintf(stderr,"tg_read_map_head: Didn't read entire file header.\n\007");
		 fprintf(stderr,"tg_read_map_head: Bytes read: %d \n",n);
		 }
	  return (-1);
	  }
   
   /* Do we need to swap bytes ? 
    * Test for byte swapping is done by checking the storm year.
    * If less then 2050 then bytes are in correct order, otherwise
    * swap bytes.
    */
   if((buf.strm_year < 2050) && (buf.strm_year > 1960))
	  {
	  memcpy(&(tg_file->map_head),&buf,sizeof(tg_map_head_str));
	  tg_file->swap_bytes = FALSE;
	  }
   else
	  {
	  swab(&buf,&(tg_file->map_head),sizeof(tg_map_head_str));
	  tg_file->swap_bytes = TRUE;
	  }

   /* The file header has now been written into tg_file->map_head .
	  Check for reasonable strm_year and strm_mon.  If not reasonable,
	  we assume the file is garbled beyond legibility, or perhaps this
	  is not a TOGA format data file. */
   if ((tg_file->map_head.strm_year < 2050) && 
	  (tg_file->map_head.strm_year > 1960))
	  {
	  if ((tg_file->map_head.strm_mon > 0) &&
		  (tg_file->map_head.strm_mon < 13))
		 {
		 /* file header OK, reset pointers into buffer */
		 tg_file->buf_ind = 32769;
		 tg_file->buf_end = 32769;
		 return(0);
		 }
	  }

   /* If we've reached this point, we can't read the file header. */
#ifdef USE_PLOG
   plog("tg_read_map_head: Can't read TOGA file header\n",PLOG_P);
#endif
   return(-1);
   
   }

float tg_make_ang(unsigned short binang)
   {
   float maxval = 65536.0;
   
   return(360.0 * ((float)binang/maxval));
   }

int tg_read_map_bytes(tg_file_str *tg_file,void *buf,int size)
   {
   int m,n,wsize;
   short dec_key;
   int ret_val = 0;
   char *wbuf;
   
   /* Copy the pointer to buf to wbuf.  wbuf stands for working buf */
   wbuf = buf;
   
   /* size is size in bytes. wsize is size in words. */
   wsize = size / 2;
   
   while(wsize > 0)
	  {
	  /* Do we need to decompress more data? */
	  if((tg_file->buf_ind + wsize - 1) > tg_file->buf_end)
		 {
		 /* Yes we do, but first make sure that we don't need any data
		  * already in buffer.
		  */
		 if((tg_file->buf_end - tg_file->buf_ind) > 0)
			{
			/* There is data in buffer that we need, before we decompress
			 * next string of data. 
			 */
			memcpy(wbuf,&(tg_file->dec_buf[tg_file->buf_ind]),
				   (tg_file->buf_end - tg_file->buf_ind + 1) * 2);
			wsize = wsize - (tg_file->buf_end - tg_file->buf_ind + 1);
			wbuf = (wbuf + ((tg_file->buf_end - tg_file->buf_ind + 1) * 2));
			ret_val = ret_val + ((tg_file->buf_end - tg_file->buf_ind + 1)*2);
			}
		 
		 /* Is this data or is this length of zeros */
		 if((n = tg_read_rec_bytes(tg_file,(char *)&dec_key,2)) <= 0)
			{
			tg_file->buf_ind = tg_file->buf_end;
			return(n);
			}
		 
		 n = (0x1 & (dec_key >> 15));
		 if (n == 1)
			{
			/* No, it is not length of zeros.
			 */
			n = dec_key & 0x7FFF;
			
			if((m = tg_read_rec_bytes(tg_file,(char *)&(tg_file->dec_buf),
									  n*2)) <= 0)
			   {
			   return(m);
			   }
			tg_file->buf_ind = 0;
			tg_file->buf_end = n - 1;
			}
		 else
			{
			/* Is this end of data ? */
			if(dec_key == 0)
			   {
			   /* End of Data */
			   tg_file->buf_ind = 32769;
			   tg_file->buf_end = 32769;
			   return(TG_END_DATA);
			   }
			/* Is this end of ray? */
			else if(dec_key == 1)
			   {
			   /* plog("End of ray\n",LOG); */
			   tg_file->buf_ind = tg_file->buf_end;
			   return(TG_END_RAY);
			   }
			/* Fill decompress buffer with 0's */
			else
			   {
			   memset(&(tg_file->dec_buf),(char)0,dec_key * 2);
			   tg_file->buf_ind = 0;
			   tg_file->buf_end = dec_key - 1;
			   }
			}
		 }
	  
	  /* Is decompressed data enough to fill request ? */
	  if((tg_file->buf_end - tg_file->buf_ind + 1) < wsize)
		 {
		 /* Will need to decompress more data */
		 memcpy(wbuf,&(tg_file->dec_buf[tg_file->buf_ind]),
				(tg_file->buf_end - tg_file->buf_ind + 1) * 2);
		 wsize = wsize - (tg_file->buf_end - tg_file->buf_ind + 1);
		 
		 wbuf = (wbuf + ((tg_file->buf_end - tg_file->buf_ind + 1) * 2));
		 
		 ret_val = ret_val + ((tg_file->buf_end - tg_file->buf_ind + 1) * 2);
		 tg_file->buf_ind = tg_file->buf_end;
		 }
	  else
		 {
		 /* There is enough decompressed data for request */
		 memcpy(wbuf,&(tg_file->dec_buf[tg_file->buf_ind]),wsize * 2);
		 tg_file->buf_ind = tg_file->buf_ind + wsize;
		 ret_val = ret_val + (wsize * 2);
		 wsize = 0;
		 }
	  }
   return(ret_val);
   }   

int tg_read_rec_bytes(tg_file_str *tg_file,char *buf,int size)
   {
   /* Return size number of bytes in buf.  Read in new record
    * if needed.  Check to make sure missing records with
    * record number variable found in record header.
	*/
   int wsize,n;
   char *wbuf;
   
   wsize = size/2;
   wbuf = buf;
   
   /* Is there enough data in recbuf for request */
   if((tg_file->data_ind + wsize - 1) > 2043)
	  {
	  /* No there is not enough data for this request, but before
	   * we read the next buffer, we should copy what we have into
	   * the buffer pointed to by buf.
	   */
	  if(tg_file->data_ind < 2044)
		 {
		 memcpy(wbuf,&(tg_file->recbuf.data[tg_file->data_ind]),
				(2043 - tg_file->data_ind + 1) * 2);
		 wsize = wsize - (2043 - tg_file->data_ind + 1);
		 wbuf = (wbuf + ((2043 - tg_file->data_ind + 1) * 2));
		 }
	  
	  /* New record has to be read in */
	  tg_file->data_ind = 0;
	  if((n = tg_read_map_rec(tg_file)) < TG_RECSIZE)
		 {
		 if(n == 0)
			{
			return(n);
			}
		 else
			{
			fprintf(stderr,"tg_read_map_rec: %d \n",n);
			return(-1);
			}
		 }
	  if(tg_file->first_rec)
		 {
		 tg_file->recnum = tg_file->recbuf.rec_num;
		 tg_file->first_rec = FALSE;
		 }
	  else
		 {
		 if((tg_file->recnum + 1) != tg_file->recbuf.rec_num)
			{
			tg_file->recnum = tg_file->recbuf.rec_num;
			/* Set index to next ray */
			tg_file->data_ind = tg_file->recbuf.first_ray - 5;
			return(TG_REC_NOSEQ);
			}
		 else
			{
			tg_file->recnum = tg_file->recbuf.rec_num;
			}
		 }			   
	  tg_file->data_ind = 0;
	  }
   
   memcpy(wbuf,&(tg_file->recbuf.data[tg_file->data_ind]),wsize*2);
   tg_file->data_ind = tg_file->data_ind + wsize;
   
   return(size);
   }

int tg_read_map_rec(tg_file_str *tg_file)
   {
   int n;
   static char buf[TG_RECSIZE];
   
   if((n = read(tg_file->fd,buf,TG_RECSIZE)) < 0)
	  {
	  fprintf(stderr,"tg_read_map_rec: Error while reading data record.\n");
	  fprintf(stderr,"tg_read_map_rec: (%d)%s\n",errno,strerror(errno));
	  }
   else if(n == 0)
	  {
	  /* assume end of file */
	  }
   else if(n != TG_RECSIZE)
	  {
	  fprintf(stderr,"tg_read_map_rec: Did not read all of data record.\n\007");
	  fprintf(stderr,"tg_read_map_rec: Bytes read: %d \n",n);
	  }
   else
	  {   
	  if(tg_file->swap_bytes)
		 {
		 /* record read in correctly */
		 swab(buf,&(tg_file->recbuf),TG_RECSIZE);
		 }
	  else
		 {
		 memcpy(&(tg_file->recbuf),buf,TG_RECSIZE);
		 }
	  }

   return(n);
   }


void tg_decode_ray_data(tg_file_str *tg_file,short ray_buf[])
   {
   int j,k;
   short dbval,vel,temp1,temp2;
   float nyq_vel;
   

   k = 0;  /* intialize ray->data index */
   switch (tg_file->ray_head.type)
	  {
	case 1:
	  /* TOGA record type 1 contains uncorrected & corrected 
		 reflectivity data, velocity, and spectrum width values. */ 
	  tg_file->ray.da_inv[TG_DM_IND] = TRUE;
	  tg_file->ray.da_inv[TG_DZ_IND] = TRUE;
	  tg_file->ray.da_inv[TG_VR_IND] = TRUE;
	  tg_file->ray.da_inv[TG_SW_IND] = TRUE;

	  tg_file->ray.num_bins[TG_DM_IND] = tg_file->ray_head.srngkill - 1;
	  tg_file->ray.num_bins[TG_DZ_IND] = tg_file->ray_head.srngkill - 1;
	  tg_file->ray.num_bins[TG_VR_IND] = tg_file->ray_head.srngkill - 1;
	  tg_file->ray.num_bins[TG_SW_IND] = tg_file->ray_head.srngkill - 1;

	  tg_file->ray.start_km[TG_DM_IND] = tg_file->ray_head.strt_rng/40.0;
	  tg_file->ray.start_km[TG_DZ_IND] = tg_file->ray_head.strt_rng/40.0;
	  tg_file->ray.start_km[TG_VR_IND] = tg_file->ray_head.strt_rng/40.0;
	  tg_file->ray.start_km[TG_SW_IND] = tg_file->ray_head.strt_rng/40.0;
	  
	  tg_file->ray.interval_km[TG_DM_IND] = tg_file->map_head.rnginc/1000.0;
	  tg_file->ray.interval_km[TG_DZ_IND] = tg_file->map_head.rnginc/1000.0;
	  tg_file->ray.interval_km[TG_VR_IND] = tg_file->map_head.rnginc/1000.0;
	  tg_file->ray.interval_km[TG_SW_IND] = tg_file->map_head.rnginc/1000.0;
	  
	  for (j=0; j<tg_file->ray.num_bins[TG_DM_IND]; j++)
		 {
		 /* check OK flag bit to see if this is valid data */
		 if ((ray_buf[j*3] & 0x8000) == 0)
			{
			/* bad data, store no_data flag into each field */
			tg_file->ray.data[TG_DM_IND][k] = TG_NO_DATA;
			tg_file->ray.data[TG_DZ_IND][k] = TG_NO_DATA;
			tg_file->ray.data[TG_VR_IND][k] = TG_NO_DATA;
			tg_file->ray.data[TG_SW_IND][k] = TG_NO_DATA;
			}
		 else   /* good data, uncode and store into tg_ray_data struct */
			{
			/******* do corrected dbz value *********/
			dbval = ray_buf[j*3 + 1] & 0x0FFF;
			/* The dbz data are signed 12 bit values. Check sign bit
			   of dbz value */
			if ((dbval & 0x0800) == 0)
			   {
			   /* store unscaled positive dbz value */
			   tg_file->ray.data[TG_DZ_IND][k] = dbval/16.0;
			   }
			else  /* 12 bit negative value */
			   {
			   /* make 12 bit value a 16 bit word by extending sign bit */
			   dbval = (dbval | ~0x0FFF); 
			   if (dbval == -2048)  /* -2048 indicates bad data */
				  {
				  tg_file->ray.data[TG_DZ_IND][k] = TG_NO_DATA;  /* bad data */
				  }
			   else
				  {
				  /* store unscaled negative dbz value */
				  tg_file->ray.data[TG_DZ_IND][k] = dbval/16.0;
				  }
			   }
			
			
			/******* do uncorrected dbz value *********/
			dbval = ray_buf[j*3 + 2] & 0x0FFF;
			/* The dbz data are signed 12 bit values. Check sign bit
			   of dbz value */
			if ((dbval & 0x0800) == 0)
			   {
			   /* store unscaled positive dbz value */
			   tg_file->ray.data[TG_DM_IND][k] = dbval/16.0;
			   }
			else  /* 12 bit negative value */
			   {
			   /* make 12 bit value a 16 bit word by extending sign bit */
			   dbval = (dbval | ~0x0FFF); 
			   if (dbval == -2048)  /* -2048 indicates bad data */
				  {
				  tg_file->ray.data[TG_DM_IND][k] = TG_NO_DATA;  /* bad data */
				  }
			   else
				  {
				  /* store unscaled negative dbz value */
				  tg_file->ray.data[TG_DM_IND][k] = dbval/16.0;
				  }
			   }
			
			/* compute the nyquist velocity for subsequent scaling
			   of velocity and spectrum width values. */
			/* nyquist velocity = wavelength/(4*pulse repetition period) */
			nyq_vel = (tg_file->map_head.wavelen/10000.0) /
			           (4.0*(1.0/tg_file->map_head.prf));        /*m/s*/
			
			/******** do velocity value **********/
			/* toga velocity values are 10 bits long and range from
			   -512 to 511 */
			vel = ray_buf[j*3] & 0x03FF; /* strip off leading 6 bits */
			/* This is a signed 10 bit value. Check sign bit */
			if ((vel & 0x0200) == 0)  /* sign bit set? */
			   {
			   /* no, store positive velocity value */
			   tg_file->ray.data[TG_VR_IND][k] = vel/511.0*nyq_vel;
			   }
			else  /* 10 bit negative value */
			   {
			   /* make 10 bit value a 16 bit word by extending sign bit */
			   vel = vel | ~0x03FF;
			   /* store negative velocity value */
			   tg_file->ray.data[TG_VR_IND][k] = vel/512.0*nyq_vel;
			   }
			
			/******** do spectrum width ***********/
			/* toga spectrum width values are 8 bits and range from
			   zero to 1/2 . see toga documentation */
			/* get the low order 4 bits into their correct positions */
			temp1 = (ray_buf[j*3 + 1] >> 12) & 0x000F;
			/* get the high order 4 bits into correct positions */
			temp2 = (ray_buf[j*3 + 2] >> 8) & 0x00F0;
			/* store the spectrum width value into tg_ray_data struct */
			tg_file->ray.data[TG_SW_IND][k] = ((temp1 | temp2)/512.0)*nyq_vel;
			
			}
		 k += 1;
		 }     /* end for */
	  break;   /* case 1: */
	  
	case 19:   /* uncorrected reflectivity data */
	  /* TOGA record type 19 contains only uncorrected reflectivity values */
	  tg_file->ray.da_inv[TG_DM_IND] = TRUE;
	  tg_file->ray.da_inv[TG_DZ_IND] = FALSE;
	  tg_file->ray.da_inv[TG_VR_IND] = FALSE;
	  tg_file->ray.da_inv[TG_SW_IND] = FALSE;
	  tg_file->ray.num_bins[TG_DM_IND] = tg_file->ray_head.srngkill - 1;
	  tg_file->ray.start_km[TG_DM_IND] = tg_file->ray_head.strt_rng/40.0;
	  tg_file->ray.interval_km[TG_DM_IND] = tg_file->map_head.rnginc/1000.0;

	  for (j=0; j<tg_file->ray.num_bins[TG_DM_IND]; j++)
		 {
		 /* check OK flag bit to see if this is valid data */
		 if ((ray_buf[j] & 0x8000) == 0)
			{
			tg_file->ray.data[TG_DM_IND][k] = TG_NO_DATA;  /* bad data, store flag */
			}
		 else   /* good data, uncode it and store into ray->data */
			{
			dbval = ray_buf[j] & 0x7FFF; /* strip off OK flag */
			/* This is a signed 15 bit dbz value. Check sign bit */
			if ((dbval & 0x4000) == 0)  /* sign bit set? */
			   {
			   /* no, positive db value. Unscale by factor of 16 and store */
			   tg_file->ray.data[TG_DM_IND][k] = dbval/16.0;
			   }
			else  /* 15 bit negative value */
			   {
			   /* make 15 bit value a 16 bit word by extending sign bit,
				  unscale by factor of 16 and store. */
			   tg_file->ray.data[TG_DM_IND][k] = (dbval | ~0x7FFF)/16.0;
			   }
			}
		 
		 k += 1;
		 }    /* end for */
	  break;   /* case 19: */

	default:
	  fprintf(stderr,"tg_decode_ray_data: found unknown toga data type\n");
	  fprintf(stderr,"tg_decode_ray_data: ignore this ray\n");
	  /* return TG_RAY_NOTYPE; */
	  break;
	  
	  }     /* end switch */

   }



int tg_read_ray(tg_file_str *tg_file)
   {
   int n,a,o;
   int bin_size;   
   short ray_buf[1800];
   
   /* bin_size is size in bytes of data from one bin */

   n = tg_read_map_bytes(tg_file,&(tg_file->ray_head),sizeof(tg_ray_head_str));
   switch(n)
	  {
	case TG_END_DATA:
	  /* plog("tg_read_ray: TG_END_DATA for ray_head\n",LOG); */
	  break;
	case TG_REC_NOSEQ:
	  /* plog("tg_read_ray: TG_REC_NOSEQ for ray_head\n",LOG); */
	  break;
	case 0:
	  n = TG_RAY_READ_ERR;
	  /* plog("tg_read_ray: zero bytes for ray_head\n",LOG); */
	  break;
	case -1:
	  /* plog("tg_read_ray: error for ray_head\n",LOG); */
	  break;
	default:
	  tg_file->ray.azm = tg_make_ang(tg_file->ray_head.azm);
	  tg_file->ray.elev = tg_make_ang(tg_file->ray_head.elev);

	  switch(tg_file->ray_head.type)
		 {
	   case 1:
		 /* plog("tg_read_ray:doppler data\n",LOG); */
		 bin_size = 6 * (tg_file->map_head.numbin);
		 break;
	   case 19:
		 /* plog("tg_read_ray:reflectivity data\n",LOG); */
		 bin_size = 2 * (tg_file->map_head.numbin);
		 break;
	   default:
		 /* plog("tg_read_ray:undefined data type\n",PRINT); */
		 return TG_RAY_NOTYPE;
		 break;
		 }

	  /* read ray contents from disk file into ray_buf */
	  if((o = tg_read_map_bytes(tg_file,ray_buf,bin_size)) == bin_size)
		 {
		 /* decode the raw ray data in ray_buf[] and place into
			the tg_file ray structure */
		 tg_decode_ray_data(tg_file,ray_buf);
		 }
	  else
		 {
		 fprintf(stderr,"*** wrong number of bytes read from ray ***\n");
		 fprintf(stderr,"\n,number_bytes read:%d\n",o);
		 return TG_RAY_READ_ERR;
		 }

	  /* read end_of_ray flag from disk file */
	  o = 0;
	  a = 0;
	  while((o != TG_END_RAY) && (a < 100))
		 {
	     o=tg_read_map_bytes(tg_file,ray_buf,2);
		 a++;
		 }
	  if(a >= 100)
		 {
		 fprintf(stderr,"tg_read_ray: Could not find TG_END_RAY\n");
		 return TG_RAY_READ_ERR;
		 }
	  }
   return n;
   }



void tg_prt_head(tg_map_head_str *head,int verbose)
   {
   /* print out the contents of a file map header */

   
#ifdef USE_PLOG
   plog("strm_year:     %d\n",PLOG_L,head->strm_year);
   plog("strm_mon:      %d\n",PLOG_L,head->strm_mon);
   plog("strm_day:      %d\n",PLOG_L,head->strm_day);
   plog("strm_num:      %d\n",PLOG_L,head->strm_num);
   plog("map_num:       %d\n",PLOG_L,head->map_num);
   plog("\n",PLOG_L);
   plog("scan_year:     %d\n",PLOG_L,head->scan_year);
   plog("scan_mon:      %d\n",PLOG_L,head->scan_mon);
   plog("scan_day:      %d\n",PLOG_L,head->scan_day);
   plog("scan_hour:     %d\n",PLOG_L,head->scan_hour);
   plog("scan_min:      %d\n",PLOG_L,head->scan_min);
   plog("scan_sec:      %d\n",PLOG_L,head->scan_sec);
   plog("data_set:      %d\n",PLOG_L,head->data_set);
   plog("\n",PLOG_L);
   if(verbose)
	  {
	  plog("tp1_ar:        %d\n",PLOG_L,head->tp1_ar);
	  plog("tp1_occw:      %d\n",PLOG_L,head->tp1_occw);
	  plog("tp1_dibit:     %d\n",PLOG_L,head->tp1_dibit);
	  plog("tp1_debit:     %d\n",PLOG_L,head->tp1_debit);
	  plog("\n",PLOG_L);
	  plog("tp2_ar:        %d\n",PLOG_L,head->tp2_ar);
	  plog("tp2_occw:      %d\n",PLOG_L,head->tp2_occw);
	  plog("tp2_dibit:     %d\n",PLOG_L,head->tp2_dibit);
	  plog("tp2_debit:     %d\n",PLOG_L,head->tp2_debit);
	  plog("\n",PLOG_L);
	  plog("status:        %d\n",PLOG_L,head->status);
	  plog("strng:         %d\n",PLOG_L,head->strng);
	  plog("numbin:        %d\n",PLOG_L,head->numbin);
	  plog("rnginc:        %d\n",PLOG_L,head->rnginc);
	  plog("rngjit:        %d\n",PLOG_L,head->rngjit);
	  plog("numcbin:       %d\n",PLOG_L,head->numcbin);
	  plog("\n",PLOG_L);
	  plog("strtcal1:      %d\n",PLOG_L,head->strtcal1);
	  plog("strtcal2:      %d\n",PLOG_L,head->strtcal2);
	  plog("stepcal:       %d\n",PLOG_L,head->stepcal);
	  }
   plog("azmleft:       %d\n",PLOG_L,head->azmleft);
   plog("azmrght:       %d\n",PLOG_L,head->azmrght);
   plog("elev_low:      %d\n",PLOG_L,head->elev_low);
   plog("elev_hgh:      %d\n",PLOG_L,head->elev_hgh);
   plog("\n",PLOG_L);
   plog("at_angres:     %d\n",PLOG_L,head->at_angres);
   plog("numfix_ang:    %d\n",PLOG_L,head->numfix_ang);
   if(verbose)
	  {
	  for(a=0;a<20;a++)
		 {
		 plog("angfix[%2d]:    %d\n",PLOG_L,a,head->angfix[a]);
		 }
	  plog("\n",PLOG_L);
	  plog("rlparm:        %d\n",PLOG_L,head->rlparm);
	  plog("signois:       %d\n",PLOG_L,head->signois);
	  plog("sigcltr:       %d\n",PLOG_L,head->sigcltr);
	  plog("thrsh_flg:     %d\n",PLOG_L,head->thrsh_flg);
	  plog("\n",PLOG_L);
	  plog("numdsp:        %d\n",PLOG_L,head->numdsp);
	  plog("numwrd:        %d\n",PLOG_L,head->numwrd);
	  plog("\n",PLOG_L);
	  plog("scanmod:       %d\n",PLOG_L,head->scanmod);
	  plog("filename:      %s\n",PLOG_L,head->filename);
	  plog("\n",PLOG_L);
	  plog("prf:           %d\n",PLOG_L,head->prf);
	  plog("transiz:       %d\n",PLOG_L,head->transiz);
	  plog("spconf:        %d\n",PLOG_L,head->spconf);
	  plog("sufchar:       %d\n",PLOG_L,head->sufchar);
	  plog("recsat1:       %d\n",PLOG_L,head->recsat1);
	  plog("recsat2:       %d\n",PLOG_L,head->recsat2);
	  plog("\n",PLOG_L);
	  plog("dsp1cor_log:   %d\n",PLOG_L,head->dsp1cor_log);
	  plog("dsp1cor_iad:   %d\n",PLOG_L,head->dsp1cor_iad);
	  plog("dsp1cor_qad:   %d\n",PLOG_L,head->dsp1cor_qad);
	  plog("dsp1crr_log:   %d\n",PLOG_L,head->dsp1crr_log);
	  plog("dsp1crr_iad:   %d\n",PLOG_L,head->dsp1crr_iad);
	  plog("dsp1crr_qad:   %d\n",PLOG_L,head->dsp1crr_qad);
	  plog("\n",PLOG_L);
	  plog("dsp2cor_log:   %d\n",PLOG_L,head->dsp2cor_log);
	  plog("dsp2cor_iad:   %d\n",PLOG_L,head->dsp2cor_iad);
	  plog("dsp2cor_qad:   %d\n",PLOG_L,head->dsp2cor_qad);
	  plog("dsp2crr_log:   %d\n",PLOG_L,head->dsp2crr_log);
	  plog("dsp2crr_iad:   %d\n",PLOG_L,head->dsp2crr_iad);
	  plog("dsp2crr_qad:   %d\n",PLOG_L,head->dsp2crr_qad);
	  plog("\n",PLOG_L);
	  }
   plog("wavelen:       %d\n",PLOG_L,head->wavelen);
   plog("pulsewd:       %d\n",PLOG_L,head->pulsewd);
   plog("hortran_pow:   %d\n",PLOG_L,head->hortran_pow);
   plog("vertran_pow:   %d\n",PLOG_L,head->vertran_pow);
   plog("\n",PLOG_L);
   if(verbose)
	  {
	  plog("high_zero:     %d\n",PLOG_L,head->high_zero);
	  }
   plog("sitelat:       %d\n",PLOG_L,head->sitelat);
   plog("sitelong:      %d\n",PLOG_L,head->sitelong);
   plog("time_zone:     %d\n",PLOG_L,head->time_zone);
   plog("\n",PLOG_L);
   if(verbose)
	  {
	  plog("zm_dsp1_mas:   %d\n",PLOG_L,head->zm_dsp1_mas);
	  plog("zm_dsp1_slv:   %d\n",PLOG_L,head->zm_dsp1_slv);
	  plog("zm_dsp2_mas:   %d\n",PLOG_L,head->zm_dsp2_mas);
	  plog("zm_dsp2_slv:   %d\n",PLOG_L,head->zm_dsp2_slv);
	  plog("\n",PLOG_L);
	  plog("minz_dsp1_mas: %d\n",PLOG_L,head->minz_dsp1_mas);
	  plog("minz_dsp1_slv: %d\n",PLOG_L,head->minz_dsp1_slv);
	  plog("minz_dsp2_mas: %d\n",PLOG_L,head->minz_dsp2_mas);
	  plog("minz_dsp2_slv: %d\n",PLOG_L,head->minz_dsp2_slv);
	  plog("\n",PLOG_L);
	  plog("num_pol:       %d\n",PLOG_L,head->num_pol);
	  plog("exinfo_rayhd:  %d\n",PLOG_L,head->exinfo_rayhd);
	  plog("len_exhd:      %d\n",PLOG_L,head->len_exhd);
	  plog("\n",PLOG_L);
	  }
   plog("lat_deg:       %d\n",PLOG_L,head->lat_deg);
   plog("lat_hun_min:   %d\n",PLOG_L,head->lat_hun_min);
   plog("lon_deg:       %d\n",PLOG_L,head->lon_deg);
   plog("lon_hun_min:   %d\n",PLOG_L,head->lon_hun_min);
   plog("\n",PLOG_L);
   if(verbose)
	  {
	  plog("alt_atn:       %d\n",PLOG_L,head->alt_atn);
	  plog("alt_grn:       %d\n",PLOG_L,head->alt_grn);
	  plog("\n",PLOG_L);
	  plog("vel_plat:      %d\n",PLOG_L,head->vel_plat);
	  plog("vel_cor:       %d\n",PLOG_L,head->vel_cor);
	  plog("head_plat:     %d\n",PLOG_L,head->head_plat);
	  plog("head_dsp:      %d\n",PLOG_L,head->head_dsp);
	  plog("\n",PLOG_L);
	  plog("set_plat:      %d\n",PLOG_L,head->set_plat);
	  plog("drift_plat:    %d\n",PLOG_L,head->drift_plat);
	  plog("ok_plat:       %d\n",PLOG_L,head->ok_plat);
	  plog("\n",PLOG_L);
	  plog("comments:      %s\n",PLOG_L,head->comments);
	  }
#else
   fprintf(stderr, "You must link with -lplog and compile the toga library with -DUSE_PLOG to get a printout of the header.\n");
#endif
   }
