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
 * read_tape_bprp.c
 *
 * reads a logical record from an bprp radar tape
 *
 * returns 0 on success, -1 on error or end-of-tape
 *
 *
 **************************************************************************/

#include "polar_ingest.h"
#include <toolsa/ttape.h>
#include <toolsa/sockutil.h>

#define XMT_CORR 157.5
#define PLO -94.0

static double Range_corr[BPRP_GATES_PER_BEAM];

static void swap_bytes(ui16 *array, int nbytes);
static int tilt_num(double elevation, double *target_elevation_p);
static void init_range_corr(void);

static si32 read_tape_record (char *tape_buffer);

int read_tape_bprp ()
     
{
  static int first_call = TRUE;
  static char Tape_buffer[MAX_TAPE_REC_SIZE];
  static bprp_params_t Bprp_params;
  static bprp_data_t Bprp_data;

  static si32 offset = 0;   /* Offset of a logical record */ 
  static si32 left = 0;	    /* Number of logical records left in buffer */
  static si32 nread = 0;

  static int prev_tilt_num = 0;

  int i;      
  int shift, mult;

  si32 log_rec_size;        /* size of logical record */
  si32 dbz_byte;
  
  si16 days[ 13 ] = 
    { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  double vip;
  double xmt_factor;
  double noise_threshold;
  double power;
  double dbz;
  double azimuth, elevation;
  double start_azimuth, end_azimuth;
  double jday, year, rest;
  double min, sec;

  bprp_beam_t *bprp_beam;

  if (first_call) {
    init_range_corr();
    first_call = FALSE;
  }

  if (left == 0) {

    /*
     * read file mark    
     */

    nread = read_tape_record(Tape_buffer);

    /*
     * swap the bytes of each 16 bit word
     */

    swap_bytes((ui16 *) Tape_buffer, (int) nread);
    offset = 0;
    left = nread;

  }

  if (nread < 0) {
  
    /*
     * read error or end of tape
     */

    return (-1);
  
  }

  bprp_beam = (bprp_beam_t *) (Tape_buffer + offset);   /* raw data */
  log_rec_size = sizeof(bprp_beam_t);
  offset += log_rec_size;
  left -= log_rec_size;
  if (left < log_rec_size) left = 0;
  
  /*
   * unpack the date                        
   */
  
  year = (double) (bprp_beam->hdr.date & 0xFC00) / 512.;               
  jday = (double) (bprp_beam->hdr.date & 0x03FF);                 
  
  rest = fmod ( year, 4.0 );
  if    ( rest == 0 ) {
    days[2] = 29;
  }
  
  for ( i = 1; i < 13; i++ ) {
    if    ( jday >= days[i] ) {
      jday -= days[i];
    } else {
      break;
    }
  }

  Bprp_params.year = (ui16) ( (int)year );
  Bprp_params.month = (ui16) ( (int)i );
  Bprp_params.day = (ui16) ( (int)jday );

  if (Bprp_params.year < 1900) {
    if (Bprp_params.year> 70)
      Bprp_params.year += 1900;
    else
      Bprp_params.year += 2000;
  }

  /*
   * unpack the time                        
   */
  
  Bprp_params.hour = (ui16) bprp_beam->hdr.hour;
  sec = fmod ( (double)bprp_beam->hdr.min, 60. );
  min = ( (double) bprp_beam->hdr.min - sec ) / 60.;
  Bprp_params.min = (ui16) ( (int) min );
  Bprp_params.sec = (ui16) ( (int) sec );
  
  Bprp_params.scan_mode = BPRP_SURVEILLANCE_MODE;           

  /*
   * the next 4 words are binary coded decimals
   */

  start_azimuth = 0.0;
  end_azimuth = 0.0;
  azimuth = 0.0;
  elevation = 0.0;

  for ( i = 0, shift = 12, mult = 1000, azimuth = 0;
       i < 4;
       i++, shift -= 4, mult /= 10 ) {
    start_azimuth += ( (bprp_beam->hdr.start_azimuth >> shift) & 0x0F ) * mult; 
    end_azimuth += ( (bprp_beam->hdr.end_azimuth >> shift) & 0x0F ) * mult;
    azimuth += ( (bprp_beam->hdr.azimuth >> shift) & 0x0F ) * mult; 
    elevation += ( ((bprp_beam->hdr.elevation >> shift) & 0x0F) * mult ); 
  }
  
  start_azimuth /= 10.0;
  end_azimuth /= 10.0;
  azimuth /= 10.0;
  elevation /= 10.0;

  Bprp_params.tilt_num =
    tilt_num(elevation, &Bprp_params.target_elevation);

  if (Bprp_params.tilt_num != prev_tilt_num &&
      Bprp_params.tilt_num == 1) {
    Bprp_params.vol_num++;
  }

  prev_tilt_num = Bprp_params.tilt_num;
  
  Bprp_params.start_azimuth = start_azimuth;
  Bprp_params.end_azimuth = end_azimuth;   
  Bprp_params.azimuth = azimuth;       
  Bprp_params.elevation = elevation;      
  
  Bprp_params.raycount = (ui16) bprp_beam->hdr.raycount;      
  Bprp_params.mds = (double) bprp_beam->hdr.mds;             
  Bprp_params.noise = (double) bprp_beam->hdr.noise;            
  Bprp_params.viphi = (double) bprp_beam->hdr.viphi;
  Bprp_params.viplo = (double) bprp_beam->hdr.viplo;
  Bprp_params.phi = (double) (bprp_beam->hdr.phi >> 5); 
  Bprp_params.plo = (double) (bprp_beam->hdr.plo >> 5); 
  Bprp_params.rec_slope = ((Bprp_params.phi - Bprp_params.plo) /
			   (Bprp_params.viphi - Bprp_params.viplo));
  Bprp_params.azcwlim = 1;
  Bprp_params.azccwlim = 0;
  Bprp_params.xmt = (double) ((bprp_beam->hdr.xmt & 0xFFE0) / 32);
  Bprp_params.site = (ui16) ((bprp_beam->hdr.xmt & 0x001F));
  Bprp_params.skip = (ui16) ((bprp_beam->hdr.site_blk & 0x00FF));
  Bprp_params.binwidth = (ui16) ((bprp_beam->hdr.site_blk & 0x0F00) / 256);
  Bprp_params.ints = (ui16) ((bprp_beam->hdr.site_blk & 0xF000) / 4096);

  /*
   * load up dbz data
   */

  Bprp_params.lscale[0].scale = BPRP_DBZ_SCALE;
  Bprp_params.lscale[0].bias = BPRP_DBZ_BIAS;

  noise_threshold = Bprp_params.mds + Bprp_params.noise;
  xmt_factor = Bprp_params.xmt - XMT_CORR;
  
  for  ( i = 0; i < BPRP_GATES_PER_BEAM; i++ ) {

    vip = (double) (bprp_beam->vip[i] + 4) / 8.0;

    if (vip > noise_threshold) {

      power = (PLO + ((vip - Bprp_params.viplo) * Bprp_params.rec_slope));

      dbz = power + Range_corr[i] - xmt_factor;

      dbz_byte = (si32) ((dbz - BPRP_DBZ_BIAS) / BPRP_DBZ_SCALE + 0.5);
      if (dbz_byte < 0) dbz_byte = 0;
      if (dbz_byte > 255) dbz_byte = 255;
      Bprp_data.dbz[i] = dbz_byte;

    } else {

      Bprp_data.dbz[i] = 0;

    }
    
  }

  /*
   * load up iff data
   */

  memcpy(Bprp_data.iff, bprp_beam->iff, BPRP_NIFF * sizeof(bprp_iff_t));

  /*
   * set the pointers
   */

  set_bprp_ptrs(&Bprp_params, &Bprp_data);

  /*
   * wait a given time
   */

  if (Glob->tape_read_wait > 0)
    uusleep((ui32) Glob->tape_read_wait);

  /*
   * return
   */

  return (0);

}

/**********************************************************************
 * swap_bytes()
 *
 * Swaps adjacent bytes in place - fast swap
 *
 */

static void swap_bytes(ui16 *array, int nbytes)

{

  ui32 i, nlongs, nshorts;
  ui32 l;
  ui16 s;
  ui32 *this_long;

  nlongs = nbytes / sizeof(si32);
  this_long = (ui32 *)array;

  for (i = 0; i < nlongs; i++) {
    
    l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 8) |
		  ((l & 0x00ff0000) << 8) |
		  ((l & 0x0000ff00) >> 8) |
		  ((l & 0x000000ff) << 8));
    
    this_long++;
  }
  
  if (nlongs * sizeof(si32) != nbytes) {
    nshorts = nbytes / sizeof(si16);
    s = array[nshorts-1];
    array[nshorts-1]= (((s & 0xff00) >> 8) | ((s & 0x00ff) << 8));
  }
	
}

/********************************************************************
 * tilt_num()
 *
 * Get tilt num and target elevation from actual elevation
 */

#define NTILTS 18

static double Target_elev[NTILTS] = {1.5, 2.5, 3.5, 4.5, 5.5, 6.7,
				       8.0, 9.6, 11.5, 13.7, 16.2, 19.2,
				       22.6, 26.5, 30.8, 35.5, 40.5, 45.6};

static int tilt_num(double elevation, double *target_elevation_p)

{

  int i;
  int tilt_num = 0;

  double delta;
  double min_delta = 1.0e6;
  
  for (i = 0; i < NTILTS; i++) {
    delta = fabs(elevation - Target_elev[i]);
    if (min_delta > delta) {
      min_delta = delta;
      tilt_num = i;
    }
  }
  
  *target_elevation_p = Target_elev[tilt_num];

  return (tilt_num + 1);

}

/**********************************************************************
 * init_range_corr()
 *
 * Initialize the range correction table
 */

static void init_range_corr(void)

{

  int i;
  double range_km;
  double range_nm;

  for (i = 0; i < BPRP_GATES_PER_BEAM; i++) {
    
    range_km = (BPRP_START_RANGE +
		BPRP_GATE_SPACING * ((double) i + 0.5));
    range_nm = range_km / 1.852;
    Range_corr[i] = 20.0 * log10(range_km);

  }

}

/***************************************************************************
 * read_tape_record.c
 *
 * reads a physical record from the bprp tape
 *
 * On success, returns the size of the record read, and loads the data
 * into buffer
 *
 * On failure returns -1: this is either a read failure, or logical
 * end of tape.
 *
 **************************************************************************/

static si32 read_tape_record (char *tape_buffer)

{

  static int tape;
  static int first_call = TRUE;
  si32 eof_flag = 0;              /* end of file flag */
  si32 nread;
  int errcnt;

  /*
   * on first call, open tape device
   */

  if (first_call) {

    if((tape = open(Glob->tape_name, O_RDONLY)) < 0) {
      fprintf(stderr, "\nERROR - %s:read_tape_record.\n",
	      Glob->prog_name);
      fprintf(stderr, "Opening tape unit\n");
      perror(Glob->tape_name);
      tidy_and_exit(-1);
    }

    /*
     * set variable block size
     */
    
    if (TTAPE_set_var(tape)) {
      fprintf(stderr, "ERROR - %s:read_tape_record - TAPE_set_var\n",
	      Glob->prog_name);
      fprintf(stderr, "Cannot set to variable block size.\n");
      perror(Glob->tape_name);
      tidy_and_exit(-1);
    }
  
    first_call = FALSE;

  }

  errcnt = 0;

 do_read:

  /*
   * wait on tape for up to 10 secs at a time
   */
  
  while (SKU_read_select(tape, 10000) < 0) {
    
    /*
     * timeout
     */
    
    PMU_auto_register("Waiting for tape");
    
  }
  
  PMU_auto_register("Reading tape");

  errno = EINTR;
  while (errno == EINTR) {
    errno = 0;
    nread = read (tape, tape_buffer, MAX_TAPE_REC_SIZE);
  }
  
  if (nread < 0) {

    fprintf(stderr, "ERROR - %s:read_tape_record\n",
	    Glob->prog_name);
    perror(Glob->tape_name);
    if(++errcnt > 20)
      return (-1L);  /* quit if many errors */
    goto do_read;

  } else if (nread == 0) {

    /*
     * no bytes returned from read
     */
    
    if (eof_flag >= 2) {

      /*
       * logical end of tape 
       */
      
      if (Glob->summary_print ||
	  Glob->header_print) {
	printf ("%s: logical end of tape encountered\n",
		Glob->tape_name);
      }

      return (-1L);

    } else {

      /*
       * end of file
       */

      eof_flag++;

      if (Glob->summary_print ||
	  Glob->header_print) {
	printf("%s: end of file\n", Glob->tape_name);
      }
      
      goto do_read;

    }

  } else {

    return (nread);

  }

}

