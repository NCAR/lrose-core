/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/***************************************************************************
 * file_input.c
 *
 * alenia data input from disk
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * July 1997
 *
 **************************************************************************/

#include "Alenia2Udp.h"
#include <time.h>

#define BOOL_STR(a) ((a)? "true" : "false")

/*
 * file scope
 */

static int Ngates_init;
static int Npulses_init;
static long File_offset;
static FILE *In_fp = NULL;
static char *File_path;

/*
 * prototypes
 */

static ui08 *alloc_buffer(int nbytes_data);
static long get_header_offset(void);
static int bcd2int(ui08 bcd_byte);

/******************
 * open_input_file()
 *
 * Open and initialize
 */

int open_input_file(char *file_path)

{

  alenia_header_t header;
  
  /*
   * open the file
   */
  
  File_path = file_path;
  if ((In_fp = fopen(file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:open_input_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot open input file\n");
    perror(file_path);
    return (-1);
  }

  /*
   * read in header to get start ngates and npulses
   */
  
  if (fread(&header, sizeof(header), 1, In_fp) != 1) {
    fprintf(stderr, "ERROR - %s:open_input_file\n", Glob->prog_name);
    fprintf(stderr, "Cannot read header from input file\n");
    perror(file_path);
    fclose(In_fp);
    In_fp = NULL;
    return (-1);
  }
  
  /*
   * initialize
   */
  
  Ngates_init = ((int) (header.avarie >> 6) * 256) + header.num_bin_l;
  Npulses_init =
    ((int) (header.num_pulses_h >> 6) * 256) + header.num_pulses_l;

  /*
   * rewind
   */

  File_offset = 0;
  fseek(In_fp, File_offset, SEEK_SET);

  return (0);

}

void close_input_file(void)

{

  fclose(In_fp);
  In_fp = NULL;

}

/*******************
 * read_input_beam()
 *
 * On success, returns the size of the record read, and loads the data
 * into buffer
 *
 * On success return 0, failure returns -1
 */

int read_input_beam (ui08 **beam_p, int *len_p)

{

  ui08 *beam_buffer;
  long new_offset;
  int data_len;

  date_time_t beam_time;
  alenia_header_t header;
  alenia_params_t params;
  
  /*
   * because there are problems with some of the files
   * search ahead to next good header
   */
  
  new_offset = get_header_offset();
  if (new_offset < 0) {
    return (-1);
  } else {
    File_offset = new_offset;
    fseek(In_fp, File_offset, SEEK_SET);
  }
  
  /*
   * read in header
   */

  if (fread(&header, sizeof(alenia_header_t),
	    1, In_fp) != 1) {
    /*
     * end of data
     */
    return (-1);
  }
  File_offset += sizeof(alenia_header_t);

  /*
   * compute data size
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
  
  params.ngates =
    ((int) (header.avarie >> 6) * 256) + header.num_bin_l;

  if (Glob->params.debug) {

    /*
     * beam time time
     */
    
    beam_time.year = bcd2int(header.year);
    beam_time.month = bcd2int(header.month);
    beam_time.day = bcd2int(header.day);
    beam_time.hour = bcd2int(header.hour);
    beam_time.min = bcd2int(header.min);
    beam_time.sec = bcd2int(header.sec);
    
    if (beam_time.year < 1900) {
      if (beam_time.year > 70)
	beam_time.year += 1900;
      else
	beam_time.year += 2000;
    }

    /*
     * azimuth and elevation
     */
    
    params.azimuth = (double) ((int) header.azim_h * 256 +
			       header.azim_l) * ALENIA_ANGLE_CONV;
    params.elevation = (double) ((int) header.elev_h * 256 +
				 header.elev_l) * ALENIA_ANGLE_CONV;
    
    fprintf(stderr, "Beam time %s, ngates %d, nfields %d, el %g, az %g\n",
	    utimestr(&beam_time), params.ngates, params.nfields,
	    params.azimuth, params.elevation);

  }

  /*
   * read in data buffer
   */

  data_len = params.ngates * params.nfields;

  beam_buffer = alloc_buffer(data_len);
  
  memcpy(beam_buffer, &header, sizeof(header));

  if (fread(beam_buffer + sizeof(header), 1, data_len, In_fp)
      != data_len) {
    /*
     * end of data
     */
    return (-1);
  }
  File_offset += data_len;

  /*
   * set return vals
   */

  *beam_p = beam_buffer;
  *len_p = data_len + sizeof(header);

  return (0);
	 
}

/****************
 * alloc_buffer()
 *
 * memory management for the header & data buffer
 */

static ui08 *alloc_buffer(int nbytes_data)

{

  static ui08 *buffer = NULL;
  static int nbytes_alloc = 0;
  int nbytes_needed;

  nbytes_needed = nbytes_data + sizeof(alenia_header_t);

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

static long get_header_offset(void)

{

  long pos = File_offset;
  int ngates;
  int npulses;

  alenia_header_t header;
  
  while (!feof(In_fp)) {
    
    fseek(In_fp, pos, SEEK_SET);

    if (fread(&header, sizeof(header), 1, In_fp) != 1) {
      return (-1);
    }

    ngates = ((int) (header.avarie >> 6) * 256) + header.num_bin_l;

    npulses =
      ((int) (header.num_pulses_h >> 6) * 256) + header.num_pulses_l;
    
    if (ngates != Ngates_init || npulses != Npulses_init) {
      pos++;
      continue;
    }

    return (pos);
    
  } /* while */

  return (-1);

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

