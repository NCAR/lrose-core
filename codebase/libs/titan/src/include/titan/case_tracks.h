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
/************************************************************
 * case_tracks.h
 *
 * Seeding case track structs
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * November 1997
 *
 ************************************************************/

#ifndef CaseTracks_h
#define CaseTracks_h

#ifdef __cplusplus
extern "C" {
#endif

#include <toolsa/umisc.h>
#include <toolsa/membuf.h>

#define CASE_TRACK_HANDLE_INIT_FLAG 543210987

typedef struct {

  int num;                /* case number */

  /*
   * seeding details
   */
  
  int seed_flag;          /* TRUE/FALSE */
  int num_flares;         /* number of flares used */
  int seed_duration;      /* secs */

  /*
   * reference, start and end times
   */
  
  time_t ref_time;        /* reference time (in seeding ops
			   * usually decision time) */
  
  int ref_minus_start;    /* (secs) time before ref time at which
			   * the case starts */
  
  int end_minus_ref;      /* (secs) time after ref time at which
			   * the case ends */

  time_t start_time;      /* ref time - ref_minus_start */

  time_t end_time;        /* ref_time + end_time_minus_ref */
  
  /*
   * track numbers
   */
  
  int complex_track_num;
  int simple_track_num;

  /*
   * environmental data
   */
  
  double cloud_base;      /* km */
  double mixing_ratio;    /* g/kg */
  double temp_ccl;        /* C */
  double deltat_500mb;    /* C */

} case_track_t;

typedef struct {

  char *prog_name;
  char *file_path;

  int init_flag;
  int debug;
  int n_cases;

  case_track_t *cases; /* pointer to case tracks array */

  MEMbuf *mbuf; /* memory buffer for case tracks */
  
} case_track_handle_t;

extern void init_case_track_handle(case_track_handle_t *handle,
				   char *prog_name,
				   int debug);

extern void free_case_track_handle(case_track_handle_t *handle);

extern int read_case_track_file(case_track_handle_t *handle,
				char *file_path);

extern int case_tracks_find_case(case_track_handle_t *handle,
				 time_t scan_time,
				 int complex_track_num,
				 int simple_track_num,
				 case_track_t **this_case_p);

extern void case_track_print(FILE *out,
			     char *spacer,
			     case_track_t *this_case);

#ifdef __cplusplus
}
#endif

#endif
