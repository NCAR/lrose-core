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
 * read_tape_chill.c
 *
 * reads a logical record from a chill tape
 *
 * returns the size of the logical record
 *
 * Mike Dixon/Gary Blackburn RAP NCAR September 1990
 *
 **************************************************************************/

/**************************************************************************
 * 
 * This document covers the data format recorded by the CHILL radar as of 
 * Spring 1992.					Dave Brunkow  7/13/92
 * 
 * I. Introduction
 * 
 * The SP20 integrates data on the returns from a series of transmitted pulses 
 * (usually 50-300) and outputs a group of fields (reflectivity, velocity, etc)
 * which will be referred to here as one ray.  During the aquisition of this
 * ray, the antenna will have moved; typically one degree or less.  The azimuth
 * and elevation angles are read at the end of the aquisition period and 
 * assigned to the ray.  During a typical scan, one axis is held at a fixed
 * angle while the other is swept through the sector of interest.  The group 
 * of data taken at one fixed angle is referred to one "sweep".  A set of 
 * sweeps representing a 3-d volume of data is referred to as a "volume".
 * Volumes are numbered sequentially starting from the beginning of an operation.
 * The sweep and ray numbers are reset to 1 at the beginning of each volume.
 * However, the initial 50 or so rays are not recorded since the ray counting
 * commences during the preparation segment when the antenna is being positioned
 * to start the scan.
 * 
 * More than one ray may be included in each physical block of data on tape.
 * Each ray consists of a variable length housekeeping header followed by 
 * one or more data fields.  Each data field has a short header followed by
 * one measurement for each gate recorded.  The number of gates recorded can
 * vary with elevation angle so that only data below a specified altitude will
 * be recorded.  The housekeeping headers consist of 16 bit words recorded
 * in the VAX byte ordering (LSB first).  The gate data is typically 8 bit
 * data, but the number of gates recorded will always be even to maintain
 * 16 bit alignment for all header words.  The first word of each ray contains
 * the ASCII characters "CD".  The second word is the total length of that ray
 * in 16 bit words (including the CD and record length words).  The third
 * word is the number of housekeeping words which follow before the first
 * data field is encountered.  Each field begins with a 2 character ASCII
 * id, followed by the total field length (counted in 16 bit words).  The
 * third word of each field header is the number of gates available.  The 
 * fourth word is the number of words in the field header including the ID
 * and field length words.  Subsequent words contain information of specific
 * interest to that field.  Over the years, the field headers have expanded,
 * so it is wise to use the header length words in locating the data rather
 * than assume a fixed length header.  
 * 
 * The general plan for using the data is to read in a physical block of data,
 * remembering the number of words read in;  unpack rays until the end of
 * block has been reached.   File chill.h contains the structures used
 * to overlay the various headers. The data is tested to see if byte
 * swapping is necessary.
 * 
 * Summary of procedure for reading Chill data:
 * 
 * Data is read until it finds a CD (chill data) record with extended
 * housekeeping.  Several of these extended hsk records will be found
 * at the start of each volume and sweep.  When these extended hsk
 * headers are found, they are saved for future reference.  Then the
 * readcal routine searches the calibration file to find the
 * calibration for the day in question.  The appropriate calibration
 * constants are used to construct a lookup table (ctab) which is used
 * to translate the 8 bit IP values on tape to dbZ at 1km.  The setrsq
 * routine is called to initialize the rsq array (to
 * 10.0*log10(range**2)) which is added to the ctab entry to complete
 * the dbZ value.  In the main processing loop, routine getray_CD
 * returns a pointer to the next ray of data which is assigned to rp
 * (ray pointer).  A check is made to see if any critical hsk items
 * have changed.  If so, the appropriate calibration routines are
 * called.  The ray pointer is passed to a routine called display(rp)
 * where the processing of data occurs.  display() just prints a few
 * hsk items, and the values the more popular fields.  There are notes
 * in display() concerning how to use the other fields which may be of
 * interest.
 * 	
 **************************************************************************/

#include "polar2gate.h"

#define MAXBLKLEN 32768
#define RSQ_SIZE 2048
#define UCHAR_MAX_VAL 256
#define MAX_LINE 256

/*
 * file scope variables
 */

static int Must_swap;
static int First_call = TRUE;

/*
 * lookup tables used in dbZ calibration
 */

static double ctab[UCHAR_MAX_VAL], rsq[RSQ_SIZE];	/* reflectivity cal. tables */

/*
 * stucture to save parameters
 */

static chill_params_t Save_params;

/*
 * pointers to field data
 */

static chlip_t *Ip_field;
static chlip_t *Ncp_field;
static chldr_t *Zdr_field;
static chlcor_t *R1_field;
static chlcor_t *R2_field;
static chlvel_t *Vel_field;
static chlvel_t *Rh_field;
static chlvel_t *Phidp_field;
static chlap_t *Ap_field;
static chlts_t *Ts_field;

/*
 * functions
 */

static void docal(ch_calib_t *c,
		  double *ar,
		  chill_params_t *ch_params);

static int readcal(ch_calib_t *cs,
		   chldat1_t *chldat1,
		   char *calfile);

static si32 getray_CD(chldat1_t **ret_ptr,
		      chill_params_t *ch_params);

static void load_field_data(ui08 *beam_buffer,
			    chill_params_t *ch_params);

static void load_gen_field(chill_params_t *ch_params,
			   chlgen_t *field_ptr,
			   ui08 *data_ptr,
			   si32 max_gates);

static void load_ip_field(chill_params_t *ch_params,
			  chlip_t *field_ptr,
			  ui08 *data_ptr,
			  si32 max_gates);

static void load_params(chill_params_t *ch_params,
			chldat1_t *chldat1);

static int load_time_from_str(date_time_t *time,
			      char *date_str,
			      char *time_str);

static si32 scan_type_valid(chill_params_t *ch_params);

static int set_field_ptrs(chill_params_t *ch_params,
			  chldat1_t *chldat1,
			  char *offset_reference);

static void setrsq(chldat1_t *chldat1,
		   int tabsize,
		   double *tp,
		   chill_params_t *ch_params);

static void swap_chldat1(chldat1_t *chldat1);

static void swap_gen(chlgen_t *gen);

static void swap_shorts(si16 *array,
			si32 nshorts);

static si16 swap_short(si16 s);

static si16 swap_u_short(ui16 us);

/*
 * main routine
 */

si32 read_tape_chill (ui08 *beam_buffer)

{

  char *offset_reference;

  si32 log_rec_size;
  si32 offset_from_chldat1;
  si32 raylen;

  chill_params_t *ch_params;
  chldat1_t *chldat1;
  static ch_calib_t ch_calib;

  ch_params = (chill_params_t *) beam_buffer;

  /*
   * first find a CD record with extended housekeeping -
   * these will appear at the start of each vol and sweep
   */

  if (First_call) {

    if (Glob->chill_extended_hsk) {

      raylen = 0;
      
      while (raylen < (int) sizeof(chldat1_t)) {
	
      init_read:
	
	if ((log_rec_size = getray_CD(&chldat1, ch_params)) < 0)
	  return (0L);
	
	if(chldat1->offset1 > CHLDAT1_OFFSET_LIMITED) {
	  
	  if (chldat1->offset1 >= CHLDAT1_SEGNAME_POS) {
	    strncpy(ch_params->segname,
		    chldat1->segname,
		    MAX_SEGNAME);
	  } else {
	    strncpy(ch_params->segname,
		    "UNSET",
		    MAX_SEGNAME);
	  }
	  
	  if ((ch_params->scan_type = scan_type_valid(ch_params)) < 0)
	    goto init_read;

	  /*
	   * exit initial loop if data fields present
	   */
	  
	  raylen = chldat1->raylen * 2;
	  
	} /* if (chldat1->offset1 ... */

      } /* while (raylen ... */

    } else {

      /*
       * no extended housekeeping
       */
      
      if ((log_rec_size = getray_CD(&chldat1, ch_params)) < 0)
	return (0L);
	
      /*
       * exit initial loop if data fields present
       */
      
      raylen = chldat1->raylen * 2;
	  
    } /* if (Glob->chill_extended_hsk) */

    /*
     * get calib data
     */

    if (readcal(&ch_calib, chldat1, Glob->chill_calibration_path))
      tidy_and_exit(-1);

    /*
     * load up the parameters, and save in static
     */

    load_params(ch_params, chldat1);
    Save_params = *ch_params;

    /*
     * fill in ctab
     */

    docal(&ch_calib, ctab, ch_params); 
    setrsq(chldat1, RSQ_SIZE, rsq, ch_params);

    /*
     * compute the offsets relative to the header
     */
    
    offset_reference = (char *) &chldat1->offset1 + sizeof(si16);
    offset_from_chldat1 = offset_reference - (char *) chldat1;
  
    /*
     * set the pointers to the data fields
     */
    
    if (set_field_ptrs(ch_params, chldat1, offset_reference))
      goto init_read;

    First_call = FALSE;

  } /* if (First_call) */

  /*
   * read in a beam
   */

 main_read:

  if ((log_rec_size = getray_CD(&chldat1, ch_params)) < 0)
    return (0L);

  /*
   * compute the offsets relative to the header
   */

  offset_reference = (char *) &chldat1->offset1 + sizeof(si16);
  offset_from_chldat1 = offset_reference - (char *) chldat1;
  
  /*
   * set the pointers to the data fields
   */

  if (set_field_ptrs(ch_params, chldat1, offset_reference))
    goto main_read;

  /*
   * store params if extended housekeeping
   */

  if (chldat1->offset1 > CHLDAT1_OFFSET_LIMITED) {
    
    if (chldat1->offset1 >= CHLDAT1_SEGNAME_POS) {
      strncpy(ch_params->segname,
	      chldat1->segname,
	      MAX_SEGNAME);
    } else {
      strncpy(ch_params->segname,
	      "UNSET",
	      MAX_SEGNAME);
    }
	  
    load_params(ch_params, chldat1);

  } /* if (chldat1->offset1 > CHLDAT1_OFFSET_LIMITED) */
    
  /*
   * get calibration if needed
   */
  
  if (ch_calib.year != chldat1->year ||
      ch_calib.month != chldat1->month ||
      ch_calib.day != chldat1->day ||
      Save_params.bypass != ch_params->bypass ||
      Save_params.pulse_len != ch_params->pulse_len ||
      Save_params.gate_len != ch_params->gate_len) {
    
    if (readcal(&ch_calib, chldat1, Glob->chill_calibration_path))
      tidy_and_exit(-1);
    
    docal(&ch_calib, ctab, ch_params);
    setrsq(chldat1, RSQ_SIZE, rsq, ch_params);

  } /* if (ch_calib.year != chldat1->year ... */
  
  /*
   * save params in static
   */

  Save_params = *ch_params;

  /*
   * reject if scan type not valid
   */

  if ((ch_params->scan_type = scan_type_valid(ch_params)) < 0)
    goto main_read;
  
  /*
   * if hsk only record, skip to next read
   */
  
/*  if(chldat1->raylen <= sizeof(chldat1_t))
    goto main_read; */
    
  /*
   *  make sure year is in full resolution
   */

  if (chldat1->year < 1900) {
    if (chldat1->year> 70)
      chldat1->year += 1900;
    else
      chldat1->year += 2000;
  }

  if (Glob->debug) {
    fprintf(stderr, "yr, month, day : %d/%d/%d\n",
	    chldat1->year,chldat1->month,chldat1->day);
  }
  
  /*
   * set number of gates
   */

  ch_params->ngates = Ip_field->gates - ch_params->txbin;

  /*
   * copy housekeeping
   */

  memcpy ((void *) &ch_params->chldat1,
          (void *) chldat1,
          (size_t) (chldat1->offset1 * sizeof(si16) +
		    offset_from_chldat1));

  /*
   * load field data
   */

  load_field_data (beam_buffer, ch_params);
  
  /*
   * wait a given time
   */

  if (Glob->device_read_wait > 0)
    uusleep((ui32) Glob->device_read_wait);

  /*
   * return record size
   */

  return log_rec_size;

}

/************************************************************
 * docal()
 *
 * docal sets up a calibration table in ar[] which contains
 * the dbz values (at 1 km) when indexed by integrator number
 */

static void docal(ch_calib_t *cb,
		  double *ar,
		  chill_params_t *ch_params)

{                                                     

  int i, intns, istart, ibrk, isub;
  double a, calcon;
  double approx;
	
  /*
   * calculate pulse length correction 0 db for 1 usec pulse
   */

  a = 10.0 * log10 (1024.0 / (double) ch_params->pulse_len);  /* 10log(1/h) */
  calcon = a + cb->radcon - cb->pktxpwr - 2.0*cb->antgain + cb->noisepwr;

  /*
   * if in bypass mode, account for ratheon switch loss
   */

  if(ch_params->bypass == 0) calcon += cb->zdrloss;
  a = cb->slope/10.0;
  intns = (int) (cb->intnoise + .5);
  istart = intns;
  ibrk = istart + (int) (1.0 / a);
  isub = istart + 1;

  for(i=isub; i<=ibrk; ++i) {
    approx= pow(10.0,(i-intns) * a) -1.0;
    *(ar+i) = 10.0 * log10(approx) + calcon;
  }

  for(i = ibrk; i< UCHAR_MAX_VAL; ++i) 
    ar[i]= (i-intns) * cb->slope + calcon;

  for(i=0; i<=istart; ++i)
    ar[i]=ar[isub];

}
           
/************************************************************************
 * readcal()
 *
 * calibration lookup routine
 *
 * returns 0 on success, -1 on failure
 */

static int readcal(ch_calib_t *calibs,
		   chldat1_t *chldat1,
		   char *calfile)

{
  
  int cal_found;
  int exact_match;

  char cal_date_str[MAX_LINE];
  char cal_time_str[MAX_LINE];
  char start_date_str[MAX_LINE];
  char start_time_str[MAX_LINE];
  char end_date_str[MAX_LINE];
  char end_time_str[MAX_LINE];
  char spacer[MAX_LINE];
  char line[MAX_LINE];

  si32 min_diff;
  si32 time_diff;

  date_time_t req_time;
  date_time_t cal_time;
  date_time_t start_time;
  date_time_t end_time;
  date_time_t best_time;

  ch_calib_t cb, best_cb;

  FILE *cf;
  
  if ((cf = fopen(calfile, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:read_tape_chill:readcal\n",
	    Glob->prog_name);
    fprintf(stderr, "Unable to open calibration file\n");
    perror(calfile);
    return (-1);
  }

  req_time.year = chldat1->year;
  req_time.month = chldat1->month;
  req_time.day = chldat1->day;
  req_time.hour = chldat1->hour;
  req_time.min = chldat1->min;
  req_time.sec = chldat1->sec;

  uconvert_to_utime(&req_time);

  cb.year = chldat1->year;
  cb.month = chldat1->month;
  cb.day = chldat1->day;

  cal_found = FALSE;
  min_diff = 1000000000;
  exact_match = FALSE;

  while (fgets(line, MAX_LINE, cf) != NULL) {

    if (sscanf(line,
	       "%s %s %lf %lf %lf %lf %lf %lf %lf %lf %s %s %s %s %s",
	       cal_date_str, cal_time_str,
	       &cb.slope, &cb.noisepwr, &cb.intnoise, &cb.pktxpwr,
	       &cb.antgain, &cb.radcon, &cb.zdrloss, &cb.zdrcal,
	       spacer,
	       start_date_str, start_time_str,
	       end_date_str, end_time_str) == 15) {
	
      if (load_time_from_str(&cal_time, cal_date_str, cal_time_str))
	continue;
      
      if(load_time_from_str(&start_time, start_date_str, start_time_str))
	continue;
	
      if (load_time_from_str(&end_time, end_date_str, end_time_str))
	continue;
	
      if (start_time.unix_time <= req_time.unix_time &&
	  end_time.unix_time >= req_time.unix_time) {
	  
	/*
	 * req time spanned by start and end
	 */
	  
	best_cb = cb;
	cal_found = TRUE;
	exact_match = TRUE;
	best_time = cal_time;
	break;
	  
      } else {
	  
	/*
	 * set best if diff is at min
	 */
	  
	time_diff = abs((double) req_time.unix_time - cal_time.unix_time);
	  
	if (time_diff < min_diff) {
	  
	  best_cb = cb;
	  best_time = cal_time;
	  cal_found = TRUE;

	} /* if (diff < min_diff) */

      } /* if (start_time.unix_time <= req_time.unix_time ... */

    } /* if (sscanf(line ... */

  } /* while (fgets(line, MAX_LINE, cf) != NULL) */

  fclose(cf);

  if (cal_found) {

    if (!exact_match) {

      fprintf(stderr, "WARNING - %s:read_tape_chill:readcal\n",
	      Glob->prog_name);
      fprintf(stderr, "Exact date match not found, using %s\n\n",
	      utimestr(&best_time));

    }
    
    if (Glob->summary_print || Glob->header_print) {
      
      printf("\nreadcal: Using calibration from %s\n",
	     utimestr(&best_time));
      printf("Slope=%.3f  Noise power=%.1f  IP noise=%.1f\n",
	     best_cb.slope, best_cb.noisepwr, best_cb.intnoise);
      
      printf("TX power=%.2f Ant Gain=%.1f  Constant=%.2f\n",
	     best_cb.pktxpwr, best_cb.antgain, best_cb.radcon);
      
      printf("Switch loss=%.2f  Zdrcal=%.2f\n\n",
	     best_cb.zdrloss, best_cb.zdrcal);
      
    } /* if (Glob->summary_print || Glob->header_print) */
  
    *calibs = best_cb;
    return (0);

  } else {

    return (-1);

  }

}

#ifdef NOTNOW
static void print_chl(FILE *out, chldat1_t *chldat1)

{

  char btype[4];
  chldat1_t hdr = *chldat1;

  memcpy(btype, &hdr, 2);
  btype[2] = '\0';

  if (Must_swap) {
    swap_shorts((si16 *) ((char *) &hdr + 2), 17);
  }

  fprintf(out, "raylen, raynum: %hd, %hd\n", hdr.raylen, hdr.raynum);
  fprintf(out, "az, el: %hd, %hd\n", hdr.az, hdr.el);
  fprintf(out, "hour,min,sec,tenths: %hd, %hd, %hd, %hd\n",
	  hdr.hour,hdr.min,hdr.sec,hdr.tenths);
  fprintf(out, "year,month,day: %hd, %hd, %hd\n",
	  hdr.year,hdr.month,hdr.day);
  fprintf(out, "volnum, sweepnum: %hd, %hd\n",
	  hdr.volnum, hdr.sweepnum);
  
}
#endif

/*******************************************************************
 * getray_CD()
 *
 * routine to find next CHILL data record (ray)
 *
 * returns number of bytes in record, -1 on error or end-of-file
 */

static si32 getray_CD(chldat1_t **ret_ptr,
		      chill_params_t *ch_params)

{

  static char tape_buffer[MAX_TAPE_REC_SIZE];
  static char *bufptr, *buflim;
  static int need_new_beam = TRUE;
  static si32 nread;
  static si16 last_az = 2048, last_el = 2048;

  char *bp;
  int cd_found;
  int this_az, this_el, mean_az, mean_el, dif;
  si32 raylen;

  chldat1_t *chldat1;

 do_read:

  if (need_new_beam) {
    
    /*
     * read next data block
     */

    nread = read_tape_record (tape_buffer);

    /*
     * return on end-of-tape or error
     */

    if (nread < 0)
      return (-1L);

    /*
     * set pointer to start and end of buffer
     */

    buflim= tape_buffer + nread;	 /* set end of block pointer */
    bufptr = tape_buffer;

    need_new_beam = FALSE;

  }
 
  /*
   * move ahead to area in buffer starting with "CD"
   */

  cd_found = FALSE;
  for (bp = bufptr; bp < buflim; bp += 2) {
    if (*bp == 'C' && *(bp + 1) == 'D') {
      cd_found = TRUE;
      break;
    }
  }
  
  if (!cd_found) {
    /*
     * no CD records in remaining buffer
     */
    need_new_beam = TRUE;
    goto do_read;
  }

  bufptr = bp;
  chldat1= (chldat1_t *) bufptr;

  /*
   * check swapping status
   */

  if (chldat1->day > 0 && chldat1->day < 32)
    Must_swap = FALSE;
  else
    Must_swap = TRUE;

  if (Must_swap)
    raylen= swap_short (chldat1->raylen) * 2;
  else
    raylen= chldat1->raylen * 2;
  
  /*
   * check for reasonable block length, reject if not
   */
  
  if(raylen <= 0 || raylen > MAXBLKLEN * 2) {
    need_new_beam = TRUE;
    goto do_read;
  }

  /*
   * chldat1 and sp point to CD record
   */

  if (Must_swap) {
    swap_chldat1(chldat1);
  }

  /*
   * check for reasonable block length, reject if not
   */

  bufptr += raylen;

  /*
   * position report is from end of integration cycle, so
   * average position with previous if change is small
   */

  this_az =  chldat1->az;             /* i = this az */

  dif = abs(this_az - last_az);
  if (dif < 64) {
    mean_az = (this_az + last_az + 1) >> 1; /* do avg */
  } else if (dif > 4000) {                  /* crossing North  case */
    if (this_az > 2047) this_az -= 4096;    /* go signed angles */
    if (last_az > 2047) last_az -= 4096;
    mean_az = (this_az + last_az+1) >> 1;   /* avg */
    if (mean_az < 0) mean_az += 4096;
  } else {
    mean_az = this_az;                      /* large change, dont avg */
  }

  last_az = this_az;
  chldat1->az = mean_az;

  /*
   * do the same for elevation
   */
  
  this_el = chldat1->el;
  dif = abs(this_el - last_el);

  if(dif < 64) {
    mean_el= (this_el + last_el + 1) >> 1;  /* small change avg */
  } else if (dif > 4000) {	            /* crossing horizon */
    if (this_el > 2047) this_el-= 4096;	    /* go signed angles */
    if (last_el > 2047) last_el -= 4096;
    mean_el = (this_el + last_el + 1) >> 1; /* avg */
    if (mean_el < 0) mean_el += 4096;
  } else {
    mean_el = this_el;	                    /* large change, don't avg */
  }

  last_el = this_el;
  chldat1->el = mean_el;

  /*
   * for some short house-keeping hdrs, target elevation is missing
   */

  if (chldat1->offset1 < CHLDAT1_OFFSET_LIMITED)
    ch_params->target_el = (double) chldat1->el * CHILL_DEG_CONV;
  else
    ch_params->target_el = (double) chldat1->elprogpos * CHILL_DEG_CONV;

  /*
   * return
   */

  *ret_ptr = chldat1;
  return (sizeof(chldat1_t));

}

#ifdef OBSOLETE

/*
 * the following version of getray_CD worked fine with post 1986 data,
 * but seemed to get out of sunc with 1986 data. Therefore it was
 * replaced with the version above.
 */

/*******************************************************************
 * getray_CD()
 *
 * routine to find next CHILL data record (ray)
 *
 * returns number of bytes in record, -1 on error or end-of-file
 */

static si32 getray_CD(chldat1_t **ret_ptr,
		      chill_params_t *ch_params)

{

  static char tape_buffer[MAX_TAPE_REC_SIZE];
  static char *bufptr, *buflim;
  static int swapping_checked = FALSE;
  static int need_new_beam = TRUE;
  static si32 nread;
  static si16 last_az = 2048, last_el = 2048;

  int this_az, this_el, mean_az, mean_el, dif;
  si32 raylen;

  chldat1_t *chldat1;

 do_read:

  if (need_new_beam) {
    
    /*
     * read next data block
     */

    nread = read_tape_record (tape_buffer);

    /*
     * return on end-of-tape or error
     */

    if (nread < 0)
      return (-1L);

    /*
     * set pointer to start and end of buffer
     */

    buflim= tape_buffer + nread;	 /* set end of block pointer */
    bufptr = tape_buffer;

    need_new_beam = FALSE;

  }
 
  chldat1= (chldat1_t *) bufptr;

  /*
   * if swapping status has not been checked, do it now
   */

  if (!swapping_checked) {

    if (chldat1->day > 0 && chldat1->day < 32)
      must_swap = FALSE;
    else
      must_swap = TRUE;

    swapping_checked = TRUE;

  }

  /*
   * find a record (ray) that start with ascii "CD"
   */

  while (strncmp((char *) bufptr, "CD", 2)) {

    /*
     * reject non-CD records
     */

    if (must_swap)
      raylen= swap_short (chldat1->raylen) * 2;
    else
      raylen= chldat1->raylen * 2;

    /*
     * check for reasonable block length, reject if not
     */

    if(raylen <= 0 || raylen > MAXBLKLEN) {
      need_new_beam = TRUE;
      goto do_read;
    }

    bufptr += raylen;

    if (bufptr >= buflim) {
      need_new_beam = TRUE;
      goto do_read;
    }

    chldat1= (chldat1_t *) bufptr;

  } /* while (strncmp ... */

  /*
   * chldat1 and sp point to CD record
   */

  if (must_swap) {

    swap_chldat1(chldat1);
    raylen= chldat1->raylen * 2;

  }

  /*
   * check for reasonable block length, reject if not
   */

  if(raylen <= 0 || raylen > MAXBLKLEN * 2) goto do_read;
  bufptr += raylen;
  if (bufptr >= buflim)
    need_new_beam = TRUE;


  /*
   * position report is from end of integration cycle, so
   * average position with previous if change is small
   */

  this_az =  chldat1->az;             /* i = this az */

  dif = abs(this_az - last_az);
  if (dif < 64) {
    mean_az = (this_az + last_az + 1) >> 1; /* do avg */
  } else if (dif > 4000) {                  /* crossing North  case */
    if (this_az > 2047) this_az -= 4096;    /* go signed angles */
    if (last_az > 2047) last_az -= 4096;
    mean_az = (this_az + last_az+1) >> 1;   /* avg */
    if (mean_az < 0) mean_az += 4096;
  } else {
    mean_az = this_az;                      /* large change, dont avg */
  }

  last_az = this_az;
  chldat1->az = mean_az;

  /*
   * do the same for elevation
   */
  
  this_el = chldat1->el;
  dif = abs(this_el - last_el);

  if(dif < 64) {
    mean_el= (this_el + last_el + 1) >> 1;  /* small change avg */
  } else if (dif > 4000) {	            /* crossing horizon */
    if (this_el > 2047) this_el-= 4096;	    /* go signed angles */
    if (last_el > 2047) last_el -= 4096;
    mean_el = (this_el + last_el + 1) >> 1; /* avg */
    if (mean_el < 0) mean_el += 4096;
  } else {
    mean_el = this_el;	                    /* large change, don't avg */
  }

  last_el = this_el;
  chldat1->el = mean_el;

  /*
   * for some short house-keeping hdrs, target elevation is missing
   */

  if (chldat1->offset1 < CHLDAT1_OFFSET_LIMITED)
    ch_params->target_el = (double) chldat1->el * CHILL_DEG_CONV;
  else
    ch_params->target_el = (double) chldat1->elprogpos * CHILL_DEG_CONV;

  /*
   * return
   */

  *ret_ptr = chldat1;
  return (sizeof(chldat1_t));

}

#endif

/***********************************************************************
 * load_field_data()
 */

static void load_field_data (ui08 *beam_buffer,
			     chill_params_t *ch_params)

{

  ui08 *field_data;
  ui08 *data_ptr;
  int field_active;
  si32 i;
  si32 ngates;
  si32 this_flag;

  field_data = beam_buffer + sizeof(chill_params_t);
  ngates = ch_params->ngates;
  ch_params->nfields_current = 0L;
  ch_params->field_flag = 0L;

  /*
   * zero out field data
   */

  memset ((void *)  field_data,
          (int) 0, (size_t) (Glob->nfields_out * ngates));

  /*
   * load in fields which are available
   */

  data_ptr = field_data;

  for (i = 0; i < Glob->nfields_out; i++) {

    field_active = FALSE;
    
    if (!strcmp(Glob->chill_fields_out[i], "IP")) {

      if (Ip_field != NULL) {
	load_ip_field(ch_params, Ip_field, data_ptr, ngates);
	field_active = TRUE;
      }

    } else if (!strcmp(Glob->chill_fields_out[i], "VE")) {

      if (Vel_field != NULL) {
	load_gen_field(ch_params, (chlgen_t *) Vel_field, data_ptr, ngates);
	field_active = TRUE;
      }

    } else if (!strcmp(Glob->chill_fields_out[i], "DR")) {

      if (Zdr_field != NULL) {
	load_gen_field(ch_params, (chlgen_t *) Zdr_field, data_ptr, ngates);
	field_active = TRUE;
      }

    } else if (!strcmp(Glob->chill_fields_out[i], "DP")) {

      if (Phidp_field != NULL) {
	load_gen_field(ch_params, (chlgen_t *) Phidp_field, data_ptr, ngates);
	field_active = TRUE;
      }

    } else if (!strcmp(Glob->chill_fields_out[i], "RH")) {

      if (Rh_field != NULL) {
	load_gen_field(ch_params, (chlgen_t *) Rh_field, data_ptr, ngates);
	field_active = TRUE;
      }

    } else if (!strcmp(Glob->chill_fields_out[i], "R1")) {

      if (R1_field != NULL) {
	load_gen_field(ch_params, (chlgen_t *) R1_field, data_ptr, ngates);
	field_active = TRUE;
      }

    } else if (!strcmp(Glob->chill_fields_out[i], "R2")) {

      if (R2_field != NULL) {
	load_gen_field(ch_params, (chlgen_t *) R2_field, data_ptr, ngates);
	field_active = TRUE;
      }

    }

    if (field_active) {
      this_flag = 1L << i;
      ch_params->field_flag |= this_flag;
      ch_params->nfields_current++;
      data_ptr += ngates;
    }
      
  } /* i */

}

/*************************************************************************
 * load_gen_field()
 */

static void load_gen_field(chill_params_t *ch_params,
			   chlgen_t *field_ptr,
			   ui08 *data_ptr,
			   si32 max_gates)

{

  si32 ngates;
  ui08 *ptr;

  /*
   * return now if there is no data for this field
   */

  if (field_ptr == NULL)
      return;

  ngates = field_ptr->gates - ch_params->txbin;
  if (ngates > max_gates)
    ngates = max_gates;

  ptr = (ui08 *) field_ptr + field_ptr->numhed * 2 + ch_params->txbin;
  memcpy ((void *) data_ptr,
          (void *) ptr,
          (size_t) ngates);
  
}

/*************************************************************************
 * load_ip_field()
 */

static void load_ip_field(chill_params_t *ch_params,
			  chlip_t *field_ptr,
			  ui08 *data_ptr,
			  si32 max_gates)

{

  si32 i;
  si32 ngates;
  si32 dbz_count;
  ui08 *ptr;
  double dbz;

  ngates = field_ptr->gates - ch_params->txbin;
  if (ngates > max_gates)
    ngates = max_gates;

  ptr = (ui08 *) field_ptr + field_ptr->numhed * 2 + ch_params->txbin;

  for (i = 0; i < ngates; i++) {

    dbz = ctab[*ptr] + rsq[i];
    dbz_count = (si32) ((dbz - CHILL_DBZ_BIAS) / CHILL_DBZ_SCALE + 0.5);
    if (dbz_count < 0)
      dbz_count = 0;
    else if (dbz_count > 255)
      dbz_count = 255;
      
    *data_ptr = dbz_count;
    data_ptr++;
    ptr++;

  } /* i */
  
}

/***********************************************************************
 * load_params()
 */

static void load_params (chill_params_t *ch_params,
			 chldat1_t *chldat1)

{
  
  si32 i;
  double prt;
  si32 offset;

  /*
   * following params set from parameter file
   */

  ch_params->gate_spacing = Glob->gate_spacing;
  ch_params->start_range = ch_params->gate_spacing / 2.0;

  /*
   * following params set from housekeeping if avalilable,
   * otherwise parameter file
   */

  offset = chldat1->offset1;

   if (offset > CHLDAT1_PRT_POS) {
    prt = chldat1->gate_space / 1024.0; /* usec */
    ch_params->prf = 1000000.0 / (double) chldat1->prt;
  } else {
    ch_params->prf = Glob->prf_nominal;
  }

  if (offset > CHLDAT1_NYQVEL_POS) {
    ch_params->nyquist = (double) chldat1->nyqvel / 256.0;  /* m/sec */
    ch_params->wavelength =
      (400.0 * ch_params->nyquist / ch_params->prf); /* cm */
  } else {
    ch_params->wavelength = Glob->wavelength;
    ch_params->nyquist =
      (ch_params->wavelength * Glob->prf_nominal) / 400.0; /* m/s */
  }
  
  if (offset > CHLDAT1_GATE_SPACE_POS) {
    ch_params->pulse_len = chldat1->pulse_len;
    ch_params->gate_len = chldat1->gate_space;
  } else {
    ch_params->pulse_len = (si32) (Glob->pulse_width * 1024.0 + 0.5);
    ch_params->gate_len = ch_params->pulse_len;
  }
  
  if (offset >= CHLDAT1_SCANMODE_POS) {
    
    if (chldat1->scanmode == CHILL_SURVEILLANCE_MODE)
      ch_params->scan_mode = GATE_DATA_SURVEILLANCE_MODE;
    else if (chldat1->scanmode == CHILL_SECTOR_MODE)
      ch_params->scan_mode = GATE_DATA_SECTOR_MODE;
    else if (chldat1->scanmode == CHILL_RHI_MODE)
      ch_params->scan_mode = GATE_DATA_RHI_MODE; 
    
  } else {
    
    ch_params->scan_mode = GATE_DATA_SURVEILLANCE_MODE;
    
  }

  if (offset >= CHLDAT1_BYPASS_POS) {
    ch_params->bypass = chldat1->bypass;
  } else {
    ch_params->bypass = 1;
  }
  
  /*
   * load params for fields which are available
   */

  for (i = 0; i < Glob->nfields_out; i++) {

    ch_params->fparams[i].factor = (si32) CHILL_PARAMS_FACTOR;
    
    if (!strcmp(Glob->chill_fields_out[i], "IP")) {

      ch_params->fparams[i].scale =
	(si32) (CHILL_DBZ_SCALE * CHILL_PARAMS_FACTOR + 0.5);
      ch_params->fparams[i].bias =
	(si32) (CHILL_DBZ_BIAS * CHILL_PARAMS_FACTOR + 0.5);

    } else if (!strcmp(Glob->chill_fields_out[i], "VE")) {

      ch_params->fparams[i].scale =
	(si32) ((ch_params->nyquist / 128.0) * CHILL_PARAMS_FACTOR + 0.5);
      ch_params->fparams[i].bias =
	(si32) floor (-1.0 * ch_params->nyquist * CHILL_PARAMS_FACTOR + 0.5);
      
    } else if (!strcmp(Glob->chill_fields_out[i], "DR")) {

      ch_params->fparams[i].scale =
	(si32) (CHILL_ZDR_SCALE * CHILL_PARAMS_FACTOR + 0.5);
      ch_params->fparams[i].bias =
	(si32) (CHILL_ZDR_BIAS * CHILL_PARAMS_FACTOR + 0.5);

    } else if (!strcmp(Glob->chill_fields_out[i], "DP") ||
	       !strcmp(Glob->chill_fields_out[i], "RH") ||
	       !strcmp(Glob->chill_fields_out[i], "R1") ||
	       !strcmp(Glob->chill_fields_out[i], "R2")) {

      ch_params->fparams[i].scale =
	(si32) (CHILL_PHASE_SCALE * CHILL_PARAMS_FACTOR + 0.5);
      ch_params->fparams[i].bias =
	(si32) (CHILL_PHASE_BIAS * CHILL_PARAMS_FACTOR + 0.5);

    }

  } /* i */

}

/******************************************************************
 * load_time_from_str()
 * 
 * loads time struct from calibration file strings
 *
 * returns 0 on success, -1 otherwise
 */

static int load_time_from_str(date_time_t *time,
			      char *date_str,
			      char *time_str)


{

  if (sscanf(date_str, "%2d%2d%2d",
	     &time->year, &time->month, &time->day) != 3)
    return (-1);

  if (time->year > 50)
    time->year += 1900;
  else
    time->year += 2000;

  if (sscanf(time_str, "%2d%2d",
	     &time->hour, &time->min) != 2)
    return (-1);

  time->sec = 0;

  if (!uvalid_datetime(time))
    return (-1);

  uconvert_to_utime(time);

  return (0);

}

/******************************************************************
 * scan_type_valid()
 * 
 * returns scan type number if valid, -1 otherwise
 */

static si32 scan_type_valid(chill_params_t *ch_params)

{
  
  int type_valid;
  si32 i;
  si32 scan_type;

  /*
   * valid if all scan types requested
   */

  if (!strcmp(Glob->chill_scan_types[0], "ALL")) {
    return (0L);
  }

  /*
   * valid if no extended housekeeping
   */

  if (!Glob->chill_extended_hsk) {
    return (0L);
  }

  /*
   * find scan type
   */

  type_valid = FALSE;
  
  for (i = 0; i < Glob->nchill_scan_types; i++) {
    
    if (!strcmp(Glob->chill_scan_types[i], ch_params->segname)) {
      type_valid = TRUE;
      scan_type = i;
      break;
    }
    
  } /* i */
  
  if (!type_valid)
    return (-1L);
  else
    return (scan_type);

}

/**************************************************************
 * set_field_ptrs()
 *
 * sets the field pointers in the data
 *
 * returns 0 on success, -1 on failure
 */

static int set_field_ptrs(chill_params_t *ch_params,
			  chldat1_t *chldat1,
			  char *offset_reference)

{

  char *field_ptr;
  char *field_limit;
  char *str_ptr;
  si32 field_len;
  si32 i;

  field_ptr = offset_reference + chldat1->offset1 * sizeof(si16);
  field_limit = (char *) chldat1 + chldat1->raylen * sizeof(si16);

  Ip_field = (chlip_t *) NULL;
  Ncp_field = (chlip_t *) NULL;
  Zdr_field = (chldr_t *) NULL;
  R1_field = (chlcor_t *) NULL;
  R2_field = (chlcor_t *) NULL;
  Vel_field = (chlvel_t *) NULL;
  Rh_field = (chlvel_t *) NULL;
  Phidp_field = (chlvel_t *) NULL;
  Ap_field = (chlap_t *) NULL;
  Ts_field = (chlts_t *) NULL;

  str_ptr = ch_params->field_str;
  memset ((void *) str_ptr,
          (int) 0, (size_t) MAX_CHILL_FIELDS * 3);

  do { /* while (field_ptr < field_limit) */

    if(!strncmp(field_ptr, "IP", 2)) {
      
      /*
       * integrated power
       */
      
      Ip_field = (chlip_t *) field_ptr;
      if (Must_swap) {
	swap_shorts((si16 *) &Ip_field->lrecl, (si32) IP_NSHORTS1);
	swap_shorts((si16 *) &Ip_field->resol, (si32) IP_NSHORTS2);
      }
      field_len = Ip_field->lrecl;

    } else if(!strncmp(field_ptr, "DM", 2)) {
      
      /*
       * reflectivity pre-1988, set to IP
       */

      strncpy(field_ptr, "IP", 2);
      
      Ip_field = (chlip_t *) field_ptr;
      Ip_field->lrecl = 16 + 512;
      Ip_field->gates = 512;
      Ip_field->numhed = 8;
      Ip_field->format = -1;

      field_len = 512;

    } else if(!strncmp(field_ptr, "NC", 2)) {

      /*
       * integrated power
       */

      Ncp_field = (chlip_t *) field_ptr;
      if (Must_swap) {
	swap_shorts((si16 *) &Ncp_field->lrecl, (si32) IP_NSHORTS1);
	swap_shorts((si16 *) &Ncp_field->resol, (si32) IP_NSHORTS2);
      }
      field_len = Ncp_field->lrecl;

    } else if(!strncmp(field_ptr, "DR", 2)) {

      /*
       * zdr
       */

      Zdr_field = (chldr_t *) field_ptr;
      if (Must_swap)
	swap_gen((chlgen_t *) Zdr_field);
      field_len = Zdr_field->lrecl;

    } else if(!strncmp(field_ptr, "R1", 2)) {

      /*
       * rho 1
       */

      R1_field = (chlcor_t *) field_ptr;
      if (Must_swap)
	swap_gen((chlgen_t *) R1_field);
      field_len = R1_field->lrecl;

    } else if(!strncmp(field_ptr, "R2", 2)) {

      /*
       * rho 2
       */

      R2_field = (chlcor_t *) field_ptr;
      if (Must_swap)
	swap_gen((chlgen_t *) R2_field);
      field_len = R2_field->lrecl;

    } else if(!strncmp(field_ptr, "VE", 2) ||
	      !strncmp(field_ptr, "VR", 2)) {

      /*
       * velocity
       */

      Vel_field = (chlvel_t *) field_ptr;
      if (Must_swap)
	swap_gen((chlgen_t *) Vel_field);
      field_len = Vel_field->lrecl;

    } else if(!strncmp(field_ptr, "RH", 2)) {

      /*
       * vr
       */

      Rh_field = (chlvel_t *) field_ptr;
      if (Must_swap)
	swap_gen((chlgen_t *) Rh_field);
      field_len = Rh_field->lrecl;

    } else if(!strncmp(field_ptr, "DP", 2)) {

      /*
       * phi_dp
       */

      Phidp_field = (chlvel_t *) field_ptr;
      if (Must_swap)
	swap_gen((chlgen_t *) Phidp_field);
      field_len = Phidp_field->lrecl;

    } else if(!strncmp(field_ptr, "AP", 2)) {

      /*
       * aircraft track
       */

      Ap_field = (chlap_t *) field_ptr;
      if (Must_swap) {
	swap_shorts((si16 *) &Ap_field->lrecl, (si32) AP_NSHORTS);
	for (i = 0; i < 3; i++)
	  swap_shorts((si16 *) &Ap_field->plarch[i].x,
		      (si32) AIRPLARCH_NSHORTS);
      }
      field_len = Ap_field->lrecl;
      
    } else if(!strncmp(field_ptr, "DP", 2)) {
      
      /*
       * time series
       */
      
      Ts_field = (chlts_t *) field_ptr;
      if (Must_swap)
	swap_shorts((si16 *) &Ts_field->lrecl, (si32) TS_NSHORTS);
      field_len = Ts_field->lrecl;

    } else {

      if (Must_swap)
	field_len = swap_u_short(*((ui16 *) field_ptr + 1));
      else
	field_len = *((ui16 *) field_ptr + 1);

    }

    memcpy ((void *) str_ptr,
            (void *) field_ptr,
            (size_t) 2);
    str_ptr += 2;
    *str_ptr = ' ';
    str_ptr += 1;
    
    /*
     * reject bad field lengths
     */
    
    if (field_len <= 0 || field_len > MAXBLKLEN)
      return (-1);

    field_ptr += field_len * 2;

  } while (field_ptr < field_limit);
  
  if(Ip_field == NULL) {

    return (-1);

  } else {

    if(Ip_field->numhed >= 10 && Ip_field->txbin < 8)
      ch_params->txbin = Ip_field->txbin;
    else
      ch_params->txbin = 3;
  
    /*
     * tx pulse located 1 bin further out than expected
     */
    
    ch_params->txbin++;
    
    return (0);

  }

}

/*************************************************************************
 * setrsq()
 *
 * fill in table with 20.0 log(range in km)
 */

static void setrsq(chldat1_t *chldat1,
		   int tabsize,
		   double *tp,
		   chill_params_t *ch_params)

{

  int i;
  double range;
  double gate_spacing;
  
  gate_spacing = ch_params->gate_spacing;
  range = gate_spacing / 2.0;

  for(i = 0; i < tabsize; ++i) {

    *tp = 20.0 * log10 (range);
    range += gate_spacing;
    tp++;
    
  } /* i */

}

/**********************************************************************
 * swap_chldat1()
 *
 */

static void swap_chldat1(chldat1_t *chldat1)

{

  si32 words_left;

  swap_shorts(&chldat1->raylen, CHLDAT1_NSHORTS0);
  words_left = chldat1->offset1;

  if (words_left > CHLDAT1_NSHORTS1) {

    swap_shorts(&chldat1->az, CHLDAT1_NSHORTS1);
    words_left -= (CHLDAT1_NSHORTS1 + 1);

  } else {

    swap_shorts(&chldat1->az, words_left);
    words_left = 0;

  }
    
  if (words_left > CHLDAT1_NSHORTS2) {

    swap_shorts(&chldat1->opt_rmax, CHLDAT1_NSHORTS2);

  } else if (words_left > 0) {

    swap_shorts(&chldat1->opt_rmax, words_left);

  }
    
}
     
/**********************************************************************
 * swap_gen()
 *
 * Swap generic field header
 */

static void swap_gen(chlgen_t *gen)

{

  si16 numhed;

  numhed = swap_short((int) gen->numhed);
  gen->lrecl = swap_u_short(gen->lrecl);
  swap_shorts(&gen->gates, numhed - 2);
    
}

#ifdef BEFORE
     
/**********************************************************************
 * swap_ip_field()
 *
 * Swap ip field header
 */

static void swap_ip_field(chlip_t *ip)

{

  si16 numhed;
  si32 words_left;

  numhed = swap_short((int) ip->numhed);
  ip->lrecl = swap_u_short(ip->lrecl);
  
  words_left = numhed - 2;
  
  if (words_left < IP_NSHORTS1) {
    swap_shorts(&ip->gates, IP_NSHORTS1);
    words_left -= (IP_NSHORTS1 + 1);
  } else if (words_left > 0) {
    swap_shorts(&ip->gates, words_left);
    words_left = 0;
  }

  if (words_left < IP_NSHORTS2) {
    swap_shorts(&ip->resol, IP_NSHORTS2);
  } else if (words_left > 0) {
    swap_shorts(&ip->resol, words_left);
  }
  
}

#endif
     
/**********************************************************************
 * swap_short()
 *
 */

static si16 swap_short(si16 s)

{

  ui16 us;

  us = (ui16) s;
  
  return ((si16) (((si32) (us & 0xff00) >> 8) |
		   ((si32) (us & 0x00ff) << 8)));
	
}

/**********************************************************************
 * swap_shorts()
 *
 */

static void swap_shorts(si16 *array,
			si32 nshorts)

{

  si32 i;
  ui16 us, *this_short;

  this_short = (ui16 *) array;

  for (i = 0; i < nshorts; i++) {
    us = *this_short;
    *this_short = (ui16) (((si32) (us & 0xff00) >> 8) |
			     ((si32) (us & 0x00ff) << 8));
    this_short++;
  }
  
}

/**********************************************************************
 * swap_u_short()
 *
 */

static short swap_u_short(ui16 us)

{

  ui16 ret;

  ret = ((ui16) (((si32) (us & 0xff00) >> 8) |
		    ((si32) (us & 0x00ff) << 8)));
  return (ret);

}


