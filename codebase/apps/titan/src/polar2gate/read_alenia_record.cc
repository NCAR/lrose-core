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
 * read_alenia_record.c
 *
 * reads an alenia record from disk
 *
 * On success, returns the size of the record read, and loads the data
 * into buffer
 *
 * On failure returns -1
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * October 1995
 *
 **************************************************************************/

#include "polar2gate.h"
#include <time.h>

#define BOOL_STR(a) ((a)? "true" : "false")

/*
 * file scope
 */

static FILE *Radar_file;
static si32 File_offset;

/*
 * prototypes
 */

static ui08 *alloc_buffer(si32 nbytes_needed);
static si32 get_header_offset(si32 start_pos, FILE *radar_file);
static void open_file(void);
static void reset_file(void);
static int bcd2int(ui08 bcd_byte);

si32 read_alenia_record (char *output_buffer)

{

  static int first_call = TRUE;
  
  char *out;
  ui08 *in;
  ui08 *data_buffer;

  si32 nbuf;
  si32 new_offset;
  si32 ndata_in;

  date_time_t beam_time;

  alenia_header_t header;
  alenia_params_t params;
  
  /*
   * on first call, open radar file
   */

  if (first_call) {
    open_file();
    first_call = FALSE;
  } /* if (first_call) */

  /*
   * because there are problems with some of the files
   * search ahead to next good header
   */
  
  new_offset = get_header_offset(File_offset, Radar_file);
  if (new_offset < 0) {
    reset_file();
  } else {
    File_offset = new_offset;
    fseek(Radar_file, File_offset, SEEK_SET);
  }
  
  /*
   * read in header
   */

 start_read:

  if (fread(&header, sizeof(alenia_header_t),
	    1, Radar_file) != 1) {

    /*
     * reset file and try again
     */

    reset_file();
    goto start_read;

  }
  File_offset += sizeof(alenia_header_t);

  /*
   * beam time time
   */

  beam_time.year = bcd2int(header.year);
  beam_time.month = bcd2int(header.month);
  beam_time.day = bcd2int(header.day);
  beam_time.hour = bcd2int(header.hour);
  beam_time.min = bcd2int(header.min);
  beam_time.sec = bcd2int(header.sec);

  /*
   * make sure year is in full resolution
   */
  
  if (beam_time.year < 1900) {
    if (beam_time.year > 70)
      beam_time.year += 1900;
    else
      beam_time.year += 2000;
  }

  /*
   * convert to unix time
   */

  uconvert_to_utime(&beam_time);

  params.time = beam_time.unix_time;

  /*
   * azimuth and elevation
   */
  
  params.azimuth = (double) ((int) header.azim_h * 256 +
			     header.azim_l) * ALENIA_ANGLE_CONV;
  params.elevation = (double) ((int) header.elev_h * 256 +
			       header.elev_l) * ALENIA_ANGLE_CONV;

  /*
   * pulse width
   */
  
  switch (header.scan_mode & 3) {
    
  case 0:
    params.pulse_width = 3.0;
    break;
    
  case 1:
    params.pulse_width = 1.5;
    break;
    
  case 2:
    params.pulse_width = 0.5;
    break;
    
  default:
    fprintf(stderr, "ERROR - %s:read_alenia_record\n", Glob->prog_name);
    fprintf(stderr, "pulse_width bits incorrect\n");
    
  } /* switch (header.scan_mode & 3) */
  
  
  /*
   * scan mode
   */
  
  switch ((header.scan_mode >> 2) & 3) {

  case 0:
    params.scan_mode = GATE_DATA_SURVEILLANCE_MODE;
    break;
    
  case 1:
    params.scan_mode = GATE_DATA_SURVEILLANCE_MODE;
    break;
    
  case 2:
    params.scan_mode = GATE_DATA_SECTOR_MODE;
    break;
    
  case 3:
    params.scan_mode = GATE_DATA_RHI_MODE;
    break;
    
  } /* switch (header.scan_mode & 3) */

  /*
   * dual_prf?
   */

  params.dual_prf = (header.parameters >> 3) & 1;

  /*
   * available fields
   */
  
  params.dbz_avail = header.parameters & 1;
  params.zdr_avail = (header.parameters >> 1) & 1;
  params.vel_avail = (header.parameters >> 2) & 1;
  params.width_avail = (header.parameters >> 7) & 1;
  
  params.nfields = 0;
  params.nfields += params.dbz_avail;
  params.nfields += params.zdr_avail;
  params.nfields += params.vel_avail;
  params.nfields += params.width_avail;
  
  /*
   * gate spacing
   */
  
  switch ((header.parameters >> 4) & 7) {
    
  case 0:
    params.gate_spacing = 0.0625;
    break;
    
  case 1:
    params.gate_spacing = 0.125;
    break;
    
  case 2:
    params.gate_spacing = 0.25;
    break;

  case 3:
    params.gate_spacing = 0.5;
    break;

  case 4:
    params.gate_spacing = 1.0;
    break;

  case 5:
    params.gate_spacing = 2.0;
    break;

  default:
    fprintf(stderr, "ERROR - %s:read_alenia_record\n", Glob->prog_name);
    fprintf(stderr, "gate_spacing bits incorrect\n");
    
  } /* switch ((header.parameters >> 4) & 7) */
  
  params.start_range = params.gate_spacing / 2.0;
  
  /*
   * number of gates
   */
  
  params.ngates =
    ((int) (header.avarie >> 6) * 256) + header.num_bin_l;

  /*
   * freqency code
   */

  params.freq_num = (header.avarie >> 3) & 7;

  /*
   * filters and clutter maps
   */

  params.clutter_map = header.clutter & 1;
  params.rejected_by_filter = !((header.clutter > 1) & 1);
  params.filter_num = (header.clutter > 2) & 15;
  params.clutter_filter = (header.clutter > 6) & 1;
  params.clutter_correction = (header.clutter > 7);
  
  /*
   * number of pulses
   */
  
  params.npulses =
    ((int) (header.num_pulses_h >> 6) * 256) + header.num_pulses_l;

  /*
   * add in other info
   */

  params.vol_num = 0;
  params.tilt_num = 0;

  /*
   * dbz
   */

  params.scale[0] = 0.3125;
  params.bias[0] = -20.0;

  /*
   * zdr
   */

  params.scale[1] = 0.0625;
  params.bias[1] = -6.0;

  /*
   * vel
   */

  if (params.dual_prf) {
    params.prf = 1200;
    params.scale[2] = 0.3828;
    params.bias[2] = -49.0;
  } else {
    params.prf = 1000;
    params.scale[2] = 0.125;
    params.bias[2] = -16.0;
  }

  /*
   * width
   */

  params.scale[3] = 0.0391;
  params.bias[3] = 0.0;

  /*
   * read in data buffer
   */

  ndata_in = params.nfields * params.ngates;
  data_buffer = alloc_buffer(ndata_in);
  if (fread(data_buffer, 1, ndata_in, Radar_file) != ndata_in) {
    reset_file();
    goto start_read;
  }
  File_offset += ndata_in;

  /*
   * copy the params to the output buffer
   */

  params.ndata = N_ALENIA_FIELDS * params.ngates;
  nbuf = params.ndata + sizeof(alenia_params_t);

  out = output_buffer;
  memset((void *) out, 0, nbuf);

  memcpy((void *) out, (void *) &params, sizeof(alenia_params_t));
  out += sizeof(alenia_params_t);

  /*
   * copy the data to the output buffer
   */

  in = data_buffer;

  if (params.dbz_avail) {
    memcpy((void *) out, (void *) in, params.ngates);
    in += params.ngates;
  }
  out += params.ngates;
  
  if (params.zdr_avail) {
    memcpy((void *) out, (void *) in, params.ngates);
    in += params.ngates;
  }
  out += params.ngates;
  
  
#ifdef CORRECT_VEL

  if (params.vel_avail) {
    memcpy((void *) out, (void *) in, params.ngates);
    in += params.ngates;
  }
  out += params.ngates;

#else

    /*
     * This code is put in to alter the velocity data.
     * The vel bytes should range from 0 for max neg velocity
     * to 255 for max pos velocity. Instead, it seems that
     * a byte val of 0 represents vel of 0, byte val 127 represents
     * max pos vel, byte val 128 represents max neg velocity
     * and byte val 255 represents just less than 0.
     * Therefore, we need to add 128 to the lower 127 vals,
     * and subtract 128 from the upper 128 values.
     */

  if (params.vel_avail) {
    int i;
    for (i = 0; i < params.ngates; i++, in++, out++) {
      if (*in > 127) {
	*out = *in - 127;
      } else {
	*out = *in + 128;
      }
    } /* i */ 
  } else {
    out += params.ngates;
  } /* if (params.vel_avail) */

#endif


  if (params.width_avail) {
    memcpy((void *) out, (void *) in, params.ngates);
    in += params.ngates;
  }
  
  /*
   * return number of bytes placed in the buffer
   */
		
  return (nbuf);
	 
}

/****************
 * alloc_buffer()
 *
 * memory management for the data buffer
 */

static ui08 *alloc_buffer(si32 nbytes_needed)

{

  static ui08 *buffer = NULL;
  static si32 nbytes_alloc = 0;

  if (nbytes_needed > nbytes_alloc) {

    if (buffer == NULL) {
      buffer = (ui08 *) umalloc(nbytes_needed);
    } else {
      buffer = (ui08 *) urealloc((char *) buffer,
				   nbytes_needed);
    }

    nbytes_alloc = nbytes_needed;

  }

  return (buffer);

}

/*********************
 * get_header_offset()
 *
 * This routine is used because some of the files do not
 * have the headers at the correct locations.
 * Therefore it is necessary to search forward to 
 * get a logical match between the possible header data
 * and the values found at successive locations in the
 * file.
 *
 * If a match is found, the header offset is returned.
 *
 * Returns -1 on end of file (read error)
 */

static si32 get_header_offset(si32 start_pos, FILE *radar_file)

{

  static int first_call = TRUE;
  static si32 ngates_init;
  static si32 npulses_init;

  si32 pos = start_pos;
  si32 ngates;
  si32 npulses;

  alenia_header_t header;
  
  while (!feof(radar_file)) {
    
    fseek(radar_file, pos, SEEK_SET);

    if (fread(&header, sizeof(header), 1, radar_file) != 1) {
      return (-1);
    }

    ngates = ((int) (header.avarie >> 6) * 256) + header.num_bin_l;

    npulses =
      ((int) (header.num_pulses_h >> 6) * 256) + header.num_pulses_l;
    
    if (first_call) {

      ngates_init = ngates;
      npulses_init = npulses;
      first_call = FALSE;

    }
    
    if (ngates != ngates_init || npulses != npulses_init) {
      pos++;
      continue;
    }

    return (pos);
    
  } /* while */

  return (-1);

}

/*************
 * open_file()
 *
 * Open file and set offset to 0
 */

static void open_file(void)

{
  if ((Radar_file = fopen(Glob->device_name, "r")) == NULL) {
    fprintf(stderr, "ERROR - read_alenia_record\n");
    fprintf(stderr, "Cannot open input file\n");
    perror(Glob->device_name);
    tidy_and_exit (-1);
  }
  File_offset = 0;
}

/**************
 * reset_file()
 *
 * Go back to start of file, reset offset to 0.
 */

static void reset_file(void)

{
  fseek(Radar_file, 0, SEEK_SET);
  File_offset = 0;
}

/**************
 * bcd2int()
 *
 * Decode a byte in binary-coded decimal
 */

static int bcd2int(ui08 bcd_byte)

{
  int units, tenths;
  units = bcd_byte & 0x0f;
  tenths= (bcd_byte >> 4) & 0x0f;
  return (tenths * 10 + units);
}

