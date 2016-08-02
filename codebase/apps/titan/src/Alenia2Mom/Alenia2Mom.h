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
#ifdef __cplusplus
 extern "C" {
#endif

/*************************************************************************
 * Alenia2Mom.h
 *
 * The header for Alenia2Mom program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 *************************************************************************/

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <rapformats/alenia.h>
#include <rapformats/lincoln.h>
#include <rapformats/ds_radar.h>
#include <tdrp/tdrp.h>
#include "Alenia2Mom_tdrp.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>

#define UDP_OUTPUT_SIZE  1472 

/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                     /* program name */

  TDRPtable *table;                    /* TDRP parsing table */

  Alenia2Mom_tdrp_struct params;       /* parameter struct */

  int nfields_out;

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

extern int decode_date(alenia_header_t *al_header,
		       date_time_t *beam_time);

extern void delete_output_shmem(void);

extern date_time_t *get_beam_time(void);

extern int init_output_shmem(int shmem_key, int debug);

extern int load_alenia_params(alenia_header_t *al_header,
			      alenia_params_t *al_params);

extern void load_output_data(alenia_params_t *al_params,
			     ui08 *input_data,
			     ui08 *output_data);

extern int open_input_udp(int port, int debug);

extern int open_archive_fmq(char *fmq_path, int buf_size, int nslots,
			    int compress, char *prog_name, int debug);

extern int open_output_fmq(char *fmq_path, int buf_size, int nslots,
			   int compress, char *prog_name, int debug);

extern void 
  init_reformat2ds(char *radar_name,
		   char *site_name,
		   double latitude,
		   double longitude,
		   double altitude,
		   int polarization,
		   double beam_width,
		   double avg_xmit_pwr,
		   double wavelength,
		   double noise_dbz_at_100km,
		   int debug);

extern void 
init_reformat2ll(char *radar_name,
		 char *site_name,
		 double latitude,
		 double longitude,
		 double altitude,
		 int polarization,
		 double beam_width,
		 double avg_xmit_pwr,
		 double wavelength,
		 double noise_dbz_at_100km,
		 int debug);

extern int init_streams(void);

extern void list_tape_files(void);

extern int load_alenia_params(alenia_header_t *header,
			      alenia_params_t *params);

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
		       ui08 **write_buffer,
		       int *nwrite);

extern int reformat2ll(ui08 *read_buffer, int nread,
		       ui08 **write_buffer,
		       int *nwrite);

extern void rescale_dbz_init(void);

extern void rescale_dbz(ui08 *dbz_bytes, int ngates);

extern void rescale_set_z_range(int z_range);
  
extern void set_derived_params(void);

extern void tidy_and_exit(int sig);

extern int write_archive_eov(void);

extern int write_archive_fmq(ui08 *buffer, int buflen);

extern int write_output_fmq(ui08 *buffer, int buflen);

extern int write_output_shmem(int sw, ui08 *ray, int length);
     
extern int write_output_udp(ui08 *buffer, int buflen);

extern int write_stream(ui08 *write_buffer, int nwrite);

#ifdef __cplusplus
}
#endif

