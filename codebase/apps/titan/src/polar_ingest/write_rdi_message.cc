// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/***************************************************************************
 * write_rdi_message.c
 *
 * loads up a beam in rdi message queue
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * Jan 1995
 *
 **************************************************************************/

#include <time.h>
#include <toolsa/str.h>
#include <rdi/mmq.h>
#include <rdi/r_data.h>
#include "polar_ingest.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define ERR_COUNT 1000

static char *Prog_name;
static int Mmq_id = -1;
static int Debug;
static int Nfields;
static int Ray_len;
static int Radar_header_offset;
static Ray_header *Ray;
static char *Ray_buf;
static Radar_header *Radar;
static Field_header **Fhdr_p;
static ui08 **Data_p;

int init_rdi_mmq(int mmq_key, int nfields, int ngates,
		 char *prog_name, int debug)

{

  int ifield;
  int field_header_len;
  int field_data_len;
  int field_total_len;
  int field_header_offset;

  Debug = debug;
  Prog_name = STRdup(prog_name);
  Nfields = nfields;

  /*
   * open message queue
   */
  
  MMQ_old();
      
  if ((Mmq_id = MMQ_open (MMQ_WRITE, mmq_key)) < 0) {

    fprintf(stderr, "WARNING - %s:write_rdi_message\n", Prog_name);
    fprintf(stderr, "Failed in opening rdi mmq, key = %d, ret = %d\n",
	    mmq_key, Mmq_id);
  } else {
    
    if (Debug) {
      fprintf(stderr, "Opened rdi mmq, key = %d, ret = %d\n",
	      mmq_key, Mmq_id);
    }

  }
  
  /*
   * compute field data len - this must be a multiple of 4 bytes
   * to prevent alignment problems for the field header
   */
  
  field_header_len = sizeof(Field_header);
  field_data_len = ((ngates + 3) / 4) * 4;
  field_total_len = field_header_len + field_data_len;
    
  /*
   * compute total ray len
   */
  
  Ray_len = (sizeof(Ray_header) + sizeof(Radar_header) +
	     Nfields * field_total_len);
  
  /*
   * allocate and initialize
   */
  
  Ray_buf = (char *) umalloc ((ui32) Ray_len);
  Fhdr_p = (Field_header **) umalloc
    ((ui32) (Nfields * sizeof(Field_header *)));
  Data_p = (ui08 **) umalloc((ui32) (Nfields * sizeof(ui08 *)));
  
  memset((void *) Ray_buf, 0, (int) Ray_len);

  /*
   * set pointers into buffer
   */
  
  Ray = (Ray_header *) Ray_buf;
  Radar_header_offset = sizeof(Ray_header);
  Radar = (Radar_header *) (Ray_buf + Radar_header_offset);
  field_header_offset = Radar_header_offset + sizeof(Radar_header);
  
  for (ifield = 0; ifield < Nfields;
       ifield++, field_header_offset += field_total_len) {
    
    Fhdr_p[ifield] = (Field_header *) (Ray_buf + field_header_offset);
    Data_p[ifield] = (ui08 *) Fhdr_p[ifield] + sizeof(Field_header);
    Ray->f_pt[ifield] = field_header_offset;
    
  } /* ifield */

  return (0);
  
}

void write_rdi_message (rdata_shmem_beam_header_t *bhdr,
			ui08 *bdata,
			si32 nfields_current)
     
{
  
  static int radar_id;
  static int count = 0;

  ui08 *source, *dest;
  int retval;
  int ifield, igate;

  rdata_shmem_header_t *shmem_header;
  char *shmem_buffer;
  radar_params_t *rparams;
  field_params_t *fparams;

  /*
   * check that the message queue is OK
   */

  if (Mmq_id < 0) {
    return;
  }

  /*
   * set pointers
   */

  shmem_header = Glob->shmem_header;
  shmem_buffer = Glob->shmem_buffer;
  rparams = &shmem_header->radar;

  /*
   * set radar header
   */
  
  STRncopy(Radar->radar_name, rparams->name, 32);
  Radar->latitude = rparams->latitude / 10;
  Radar->longitude = rparams->longitude / 10;
  Radar->power = 0;
  Radar->altitude = rparams->altitude;
  Radar->pulse_width = rparams->pulse_width;
  Radar->beamwidth = rparams->beam_width / 10000;
    
  /*
   * set static sections of ray header
   */

  Ray->length = Ray_len;
  Ray->freq = 300000000 / rparams->wavelength;
  Ray->prf  = rparams->prf / 1000;
  Ray->polar = 0;
  Ray->bad_data = 0;
  radar_id = (rparams->radar_id & 0x7);
  Ray->id = 30303;
  Ray->n_fields = Nfields;
  Ray->r_h_pt = Radar_header_offset;
  Ray->r_h_pt = 0;
  
  /*
   * copy in the relevant parts of the field params
   */
  
  fparams = (field_params_t *)
    (shmem_buffer + shmem_header->field_params_offset);
    
  for (ifield = 0; ifield < Nfields; ifield++) {
      
    STRncopy(Fhdr_p[ifield]->f_name, fparams[ifield].name, 6);
    STRncopy(Fhdr_p[ifield]->f_unit, fparams[ifield].units, 6);
    
    Fhdr_p[ifield]->data_size = 8;
    
    Fhdr_p[ifield]->scale =
      (si16) floor(((double) fparams[ifield].scale /
		    (double) fparams[ifield].factor) * 100.0 + 0.5);
    
    Fhdr_p[ifield]->offset =
      (si16) floor(((double) fparams[ifield].bias /
		    (double) fparams[ifield].factor) * 100.0 + 0.5);
    
    Fhdr_p[ifield]->range =
      (ui16) ((double) rparams->start_range / 1000.0 + 0.5);
    
    Fhdr_p[ifield]->g_size =
      (ui16) (((double) rparams->gate_spacing / 1000.0) * 16.0 + 0.5);
    
    Fhdr_p[ifield]->n_gates = shmem_header->ngates;
      
  } /* ifield */
    
  /*
   * load up ray header
   */

  Ray->time = bhdr->beam_time;
  Ray->r_id = radar_id | bhdr->scan_mode;
  Ray->f_cnt = bhdr->tilt_num;
  Ray->v_cnt = bhdr->vol_num;
  Ray->azi = (ui16) floor(bhdr->azimuth * 100.0 + 0.5);
  Ray->t_ele = (ui16) floor(bhdr->target_elev * 100.0 + 0.5);
  Ray->ele = (ui16) floor(bhdr->elevation * 100.0 + 0.5);

  /*
   * load up field data
   */

  if (Glob->data_field_by_field) {

    for (ifield = 0; ifield < Nfields; ifield++) {
      memcpy((void *) Data_p[ifield],
	     (void *) (bdata + ifield * shmem_header->ngates),
	     (int) shmem_header->ngates);
    } /* ifield */

  } else {

    for (ifield = 0; ifield < Nfields; ifield++) {
	
      source = bdata + ifield;
      dest = Data_p[ifield];

      for (igate = 0; igate < shmem_header->ngates; igate++) {
	*dest = *source;
	dest++;
	source += nfields_current;
      } /* igate */

    } /* ifield */
    
  } /* if (Glob->data_field_by_field) */

  /*
   * write to message queue
   */

  retval = MMQ_write (Mmq_id, Ray_buf, Ray_len);

  if (count == 0) {
    
    if (Debug) {
      if (retval < 0) {
	fprintf(stderr, "WARNING - %s:write_rdi_message\n", Prog_name);
	fprintf(stderr, "Error on write - retval = %d\n", retval);
      } else if (retval == 0) {
	fprintf(stderr, "WARNING - %s:write_rdi_message\n", Prog_name);
	fprintf(stderr, "Output buffer full\n");
      }
    }

  }
  
  count++;
  if (count >= ERR_COUNT) {
    count = 0;
  }

}

void close_rdi_mmq()

{
  MMQ_close(Mmq_id);
}


