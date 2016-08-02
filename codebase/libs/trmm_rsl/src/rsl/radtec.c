/*
 * General RADTEC ingest code.
 *
 * By John H. Merritt
 *
 * May 21, 1998
 */
/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1998
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_LIBIMPLODE
#include <stdio.h>
#include <stdlib.h>
#include "radtec.h"
#include <implode.h>

static int nray_headers_expected = 0;
static int nrays_expected        = 0;
Radtec_ray_header *ray_header_array;
Radtec_ray *ray_array;

void radtec_print_header(Radtec_header *h)
{
  printf("version = %d\n", h->version);
  printf("scan_type = %d\n", h->scan_type);
  printf("scan_mode = %d\n", h->scan_mode);
  printf("seqno = %d\n", h->seqno);
  printf("month = %d\n", h->month);
  printf("day = %d\n", h->day);
  printf("year = %d\n", h->year);
  printf("hour = %d\n", h->hour);
  printf("min = %d\n", h->min);
  printf("sec = %d\n", h->sec);
  printf("az_el = %f\n", h->az_el);
  printf("azim_resolution = %f\n", h->azim_resolution);
  printf("azim_offset = %f\n", h->azim_offset);
  printf("elev_resolution = %f\n", h->elev_resolution);
  printf("elev_offset = %f\n", h->elev_offset);
  printf("site_elevation = %f\n", h->site_elevation);
  printf("site_latitude = %f\n", h->site_latitude);
  printf("site_longitude = %f\n", h->site_longitude);
  printf("skip = %f\n", h->skip);
  printf("range_bin_size = %f\n", h->range_bin_size);
  printf("num_range_bins = %d\n", h->num_range_bins);
  printf("num_integrations = %d\n", h->num_integrations);
  printf("num_rays = %d\n", h->num_rays);
}	

void radtec_print_ray_header(Radtec_ray_header *h)
{
  fprintf(stderr, "ray_num    = %d\n", h->ray_num);
  fprintf(stderr, "azim_angle = %f\n", h->azim_angle);
  fprintf(stderr, "elev_angle = %f\n", h->elev_angle);
  fprintf(stderr, "hour       = %d\n", h->hour);
  fprintf(stderr, "min        = %d\n", h->min);
  fprintf(stderr, "sec        = %d\n", h->sec);
  fprintf(stderr, "\n");
}

struct PassedParam
{
   unsigned int CmpPhase;
   FILE *InFile;
   FILE *OutFile;
   unsigned long CRC;
};

/*-------------------------------------------------------------------
   Routine to supply data to the implode() or explode() routines.
   When this routine returns 0 bytes read, the implode() or explode()
   routines will terminate.  Also calculate the CRC-32 on the original
   uncompressed data during the implode() call.
*/

#define explode _explode
#define implode _implode
#define crc32 _crc32

int total_bytes_read = 0;
int total_bytes_written = 0;
unsigned int ReadFile(char *Buff, unsigned int *Size, void *Param)
{
   size_t Read;
   struct PassedParam *Par = (struct PassedParam *)Param;

   Read = fread(Buff, 1, *Size, Par->InFile);
   total_bytes_read += *Size;
   if (Par->CmpPhase)
      Par->CRC = crc32(Buff, (unsigned int *)&Read, &Par->CRC);

   return (unsigned int)Read;
}

/*-------------------------------------------------------------------
   Routine to write compressed data output from implode() or
   uncompressed data from explode().  Also calculate the CRC on
   the uncompressed data during the explode() call.
*/

void radtec_load_rsl_ray_data(char *Buff, unsigned int *Size, void *Param)
{
   struct PassedParam *Par = (struct PassedParam *)Param;
   Radtec_ray_header ray_header;
   static int i = 0;
   static int nray_headers_seen = 0;
   static int nrays_seen = 0;
   int ray_size;
   static int bytes_remaining = 0;


   /*   fwrite(Buff, 1, *Size, Par->OutFile); */
   /* Buff  -- Contains the data.
    * *Size -- Contains the length of data.
    * Par   -- Contains CRC, and FILE* information.
    */

   if (!Par->CmpPhase)
      Par->CRC = crc32(Buff, Size, &Par->CRC);
   total_bytes_written += *Size;

   while(i<*Size && nray_headers_seen < nray_headers_expected) {
	 /* Because of word alignment problems, use this painful memcpy approach. */
	 memcpy(&ray_header.ray_num,    &Buff[i], sizeof(short)); i+=sizeof(short);
	 memcpy(&ray_header.azim_angle, &Buff[i], sizeof(float)); i+=sizeof(float);
	 memcpy(&ray_header.elev_angle, &Buff[i], sizeof(float)); i+=sizeof(float);
	 memcpy(&ray_header.hour,       &Buff[i], sizeof(short)); i+=sizeof(short);
	 memcpy(&ray_header.min,        &Buff[i], sizeof(short)); i+=sizeof(short);
	 memcpy(&ray_header.sec,        &Buff[i], sizeof(short)); i+=sizeof(short);
	 i+=4; /* Fill to 20 bytes. */
#define RSL_DEBUG
#undef  RSL_DEBUG
#ifdef RSL_DEBUG
	 radtec_print_ray_header(&ray_header);
#endif
	 ray_header_array[nray_headers_seen] = ray_header;
	 nray_headers_seen++;
   }
   /* Ok, whenever 'i' exceeds *Size, we must return to the explode routine
    * so that we get another buffer 'Buff'.  This new 'Buff' will pick
    * up where explode left off, and therefore, we must also pick up where
    * we left off.
	*/
   if (i >= *Size) {
	 i = 0;
#ifdef RSL_DEBUG
	 fprintf(stderr, "Need another Buff for ray headers.\n");
#endif
	 return;
   }

   /* Getting to this point means that i < *Size and we have seen
    * all the expected number of ray headers.  Now, we must collect
    * the expected number of rays (the data). 
    */

   ray_size = sizeof(Radtec_ray);
   while(i<*Size && nrays_seen < nrays_expected) {
#ifdef RSL_DEBUG
	 	 fprintf(stderr, "WHILE i=%d, i+ray_size=%d\n", i, i+ray_size);
#endif
	 if (i+ray_size > *Size) { /* Possible over flow. */
	   /* Load what we can. */
	   memcpy(&ray_array[nrays_seen].dbz, &Buff[i], *Size-i);
	   bytes_remaining =  ray_size - *Size + i;
#ifdef RSL_DEBUG
	   	   fprintf(stderr, "Buffer overflow : i=%d ray_size=%d loading %d bytes_remaining=%d\n", 
			  i, ray_size, *Size-i, bytes_remaining);
#endif
	   i=*Size;
	   break;
	 } else {
	   if (bytes_remaining > 0) {
#ifdef RSL_DEBUG
		 fprintf(stderr,"Load remaining: i=%d ray_size=%d bytes_remaining=%d\n", 
				i, ray_size, bytes_remaining);
#endif
		 memcpy(&ray_array[nrays_seen].dbz[(ray_size-bytes_remaining)/4], &Buff[i], bytes_remaining);
		 i+=bytes_remaining;
		 bytes_remaining = 0;
	   } else {
#ifdef RSL_DEBUG
		 fprintf(stderr, "Load full buff: i=%d ray_size=%d bytes_remaining=%d\n", 
				i, ray_size, bytes_remaining);
#endif
		 memcpy(&ray_array[nrays_seen].dbz, &Buff[i], ray_size);
		 i+=ray_size;
	   }
	 }
	 ray_array[nrays_seen].h = &ray_header_array[nrays_seen];
	 nrays_seen++;
#ifdef RSL_DEBUG
	 fprintf(stderr, "Ray data # %d, sizeof(Radtec_ray)=%d, i=%d\n",  nrays_seen, sizeof(Radtec_ray), i);
#endif
   }

   if (i >= *Size) {
	 i = 0;
#ifdef RSL_DEBUG
	 fprintf(stderr, "Need another Buff for ray data. i=%d *Size=%d\n", i, *Size);
#endif
	 return;
   }


}

Radtec_file *radtec_read_file(char *infile)
{
  FILE *fp;
  Radtec_file *rfile;

  char *WorkBuff;               /* buffer for compression tables */
  unsigned int Error;
  struct PassedParam Param;     /* Parameters passed to callback functions */
  

  if (infile == NULL) {
	fp = stdin;
  } else {
	if((fp = fopen(infile, "r")) == NULL) {
	  perror(infile);
	  return NULL;
	}
  }

  rfile = (Radtec_file *)calloc(1, sizeof(Radtec_file));
  if (rfile == NULL) { perror("calloc Radtec_file"); return NULL; }
  fread(&rfile->h, sizeof(Radtec_header), 1, fp);

  /* Initialize the global for the unpacking routine. The unpacking
   * routine is a callback for 'explode'; the second argument.
   */
  nray_headers_expected = rfile->h.num_rays;
  nrays_expected        = rfile->h.num_rays;

  /* Allocate space for all the headers and rays expected. */
  ray_header_array = (Radtec_ray_header *)calloc(rfile->h.num_rays, sizeof(Radtec_ray_header));
  if (ray_header_array == NULL) { perror("calloc Radtec_ray_header"); return NULL; }
  ray_array = (Radtec_ray *)calloc(rfile->h.num_rays, sizeof(Radtec_ray));
  if (ray_array == NULL) { perror("calloc Radtec_ray"); return NULL; }

  /* -------------- PKWARE ----------- */
  WorkBuff = (char *)malloc(EXP_BUFFER_SIZE);
  if (WorkBuff == NULL) {
	perror("RADETC, unable to allocate work buffer.");
	return NULL;
  }
  
  Param.InFile  = fp;
  Param.OutFile = NULL;
  
  /* Initialize CRC */
  Param.CmpPhase = 0;
  Param.CRC = (unsigned long) -1;
  
  Error = explode(ReadFile, radtec_load_rsl_ray_data, WorkBuff, &Param);
  
  Param.CRC = ~Param.CRC;
  free(WorkBuff);
  fclose(Param.InFile);
  fclose(Param.OutFile);
  if (Error != 0) {
	fprintf(stderr, "RADTEC: uncompression completed - Error %d\n", Error);
	fprintf(stderr, "RADTEC: Total bytes read    = %d\n", total_bytes_read);
	fprintf(stderr, "RADTEC: Total bytes written = %d\n", total_bytes_written);
  }
  /* -------------- PKWARE ----------- */
  rfile->ray   = ray_array;

  return rfile;
}

void radtec_free_file(Radtec_file *rfile)
{
  free(rfile->ray);
  free(rfile);
}
#endif
