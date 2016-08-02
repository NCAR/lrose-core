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

/*************************************************************************
 * ridds2mom.h
 *
 * The header for ridds2mom program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 *************************************************************************/

#include <toolsa/umisc.h>
#include <rapformats/moments.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <rapformats/ridds.h>
#include <rapformats/lincoln.h>
#include <tdrp/tdrp.h>
#include <rapformats/DsRadarMsg.hh>
#include "DsReformQueue.hh"
#include "ridds2mom_tdrp.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>

#define FIRST_TIME 999                     

#define SPD_OF_LITE 299800000

#define DBZ_SCALE 50
#define DBZ_BIAS -3200

#define VEL_SCALE 50
#define VEL_SCALE_1 100
#define VEL_SCALE_HALF 50
#define VEL_BIAS_HALF -6350
#define VEL_BIAS_1 -12700

#define SW_SCALE 50
#define SW_BIAS -6350

#define MAX_AZI_PER_TILT  3600
#define MAX_AZI  480
#define MAX_NUM_GATES 1024
#define MAX_BEAM_SZ  2250


typedef struct {
  ui08  snr;
  ui08  dbz;
  ui08  vel;
  ui08  width;
} Reformat_data;

typedef struct {
  ui08  ref[MAX_BEAM_SZ];
  unsigned short azimuth;
} Low_prf_data;

typedef struct {
  ncar_udp_frame_hdr_t hdr;
  ui08 data[UDP_OUTPUT_SIZE - sizeof(ncar_udp_frame_hdr_t)];
} ncar_udp_frame_t;

typedef struct {
  int pattern_number;
  int num_of_fixed_angles;
  float **fixed_angle;
  char *next_pattern;
} NEXRAD_vcp;

typedef struct {
  int vcp_total;
  NEXRAD_vcp *scan_strategy;
} NEXRAD_vcp_set;

/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                     /* program name */

  TDRPtable *table;                    /* TDRP parsing table */

  ridds2mom_tdrp_struct params;        /* parameter struct */

  int filelist;                        /* only list the files on tape
					* TRUE or FALSE */

  DsReformQueue *radarQueue;           /* used for fmq radar output device */
  DsReformQueue *archiveQueue;         /* used for fmq archive output device */
  DsRadarMsg    *radarMsg;             /* DS_FORMAT radar data translator */

  long tape_wait_usec;
  int  gate_ratio;                     /* number of short range gates 
                                        * per long range gate */

} global_t;

/*
 * globals
 */

#ifdef MAIN

global_t *Glob = NULL;

/*
 * if not main, declare global struct as extern
 */

#else

extern global_t *Glob;

#endif

/*
 * function prototypes
 */

extern void close_input_tape(void);

extern void close_input_udp(void);

extern void close_archive_fmq(void);

extern void close_output_fmq(void);

extern void close_output_udp(void);

extern int close_streams(void);

extern void delete_output_shmem(void);

extern void end_of_tilt( int tilt_num );

extern void end_of_volume( int vol_num );

extern void get_beam_time( RIDDS_data_hdr *riddsBeam, si32 Time_correction,
                           date_time_t *ttime );

extern int get_vcp(char *vcp_path,
		   NEXRAD_vcp_set **vol_cntrl_patterns);

extern short get_target_elev(short *vcp_type, short *tilt_num,
                             NEXRAD_vcp_set * vol_cntrl_patterns);

extern int open_input_udp(int port, int debug, int verbose = 0);

extern int open_archive_fmq(char *fmq_url, int buf_size, int nslots,
			    int compress, char *prog_name, int debug);

extern int open_output_fmq(char *fmq_url, int buf_size, int nslots,
			   int compress, char *prog_name, int debug);

extern int init_output_shmem(int shmem_key, int debug);

extern void 
  init_reformat2ds(char *radar_name,
		   int radar_id,
		   char *site_name,
		   double latitude,
		   double longitude,
		   double altitude,
		   int time_correction,
		   int polarization,
		   double wavelength,
		   double beam_width,
		   double peak_xmit_pwr,
		   double receiver_mds,
		   double noise_dbz_at_100km,
		   int samples_per_beam,
		   double receiver_gain,
		   double antenna_gain,
		   double system_gain,
		   int debug);

extern void 
init_reformat2ll(char *radar_name,
		 char *site_name,
		 double latitude,
		 double longitude,
		 double altitude,
		 int time_correction,
		 int polarization,
		 double beam_width,
		 double avg_xmit_pwr,
		 double wavelength,
		 double noise_dbz_at_100km,
		 int debug);

extern void init_moments( double noise_dbz_at_100km );

extern int init_streams(void);

extern void list_tape_files(void);

extern void new_scan_type(int scan_type);

extern int open_input_tape(char *device, int wait_msecs, int debug);

extern int open_output_udp(char *broadcast_address, int port, int debug);

extern void parse_args(int argc, 
                       char **argv, 
                       int *check_params_p,
                       int *print_params_p,
                       tdrp_override_t *override,
                       char **params_file_path_p);

extern void print_header(ui08 *buffer);

extern void print_summary(ui08 *buffer);

extern int process_data_stream(void);

extern int read_input_udp(ui08 **buf_p, int *len_p);

extern int read_stream(ui08 **buf_p, int *nread_p);

extern int read_input_tape(ui08 **buf_p, int *len_p);
     
extern int reformat2ds(ui08 *read_buffer, int nread,
		       NEXRAD_vcp_set *vcp_set,
		       ui08 **write_buffer,
		       int *nwrite);

extern int reformat2ll(ui08 *read_buffer, int nread,
		       NEXRAD_vcp_set *vcp_set,
		       ui08 **write_buffer,
		       int *nwrite);

extern void set_derived_params(void);

extern void set_time_to_wallclock(ui08 *buffer);

extern void start_of_tilt( int tilt_num, time_t beamTime );

extern void start_of_volume( int vol_num, time_t beamTime );

extern int store_data(RIDDS_data_hdr * input_data,
                      ui08 *output_data,
                      unsigned short *azimuth,
                      int new_vol);

extern void tidy_and_exit(int sig);

extern int write_archive_fmq( ui08 *buffer, int buflen );

extern int write_output_ll_shmem(int sw, ui08 *ray, int length);
     
extern int write_output_ncar_udp(ui08 *buffer, int buflen);
extern int write_output_ridds_udp(ui08 *buffer, int buflen);

extern int write_stream(ui08 *write_buffer, int nwrite);

