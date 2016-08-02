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
 * polar_ingest.h
 *
 * The header for polar_ingest program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 *************************************************************************/

#include <toolsa/umisc.h>
#include <titan/radar.h>
#include <rapformats/rp7.h>
#include <rapformats/lincoln.h>
#include <rapformats/gate_data.h>
#include <toolsa/pmu.h>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>

#include <rapformats/bprp.h>

/*
 * factor by which sacle and bias values are to be multiplied
 * before storage
 */

#define RDATA_SCALE_AND_BIAS_MULT 10000

/*
 * max record size for tape and beam buffers
 */

#define MAX_TAPE_REC_SIZE 65536
#define MAX_BEAM_REC_SIZE 32768

/*
 * incoming packet params
 */

#define MAX_NIT_PACKET_BYTES 1512
#define MAX_UDP_PACKET_BYTES 1512
#define BEAM_PACKET 0x6001

/*
 * permissions for shared mem
 */

#define S_PERMISSIONS 0666

/*
 * device details
 */

#define DEVICE "tape"
#define HEADER_TYPE "rp7"

#define LINCOLN_HEADER 0
#define RP7_HEADER 1
#define GATE_DATA_HEADER 2
#define BPRP_HEADER 3

#define TAPE_DEVICE 0
#define TCPIP_DEVICE 1
#define LL_UDP_DEVICE 2
#define NCAR_UDP_DEVICE 3
#define FMQ_DEVICE 4

#define TAPE_NAME "null"
#define TAPE_READ_WAIT 0.0

#define TCPIP_HOST "null"
#define TCPIP_PORT 50000L

#define UDP_PORT 50000L

#define FMQ_PATH "fmq.test"
#define FMQ_READ_WAIT 0.0

/*
 * gate spacing checking
 */

#define CHECK_GATE_SPACING "false"
#define TARGET_GATE_SPACING 0.250

/*
 * tilt number heading
 */

#define CHECK_TILT_NUMBER "false"
#define TILT_NUMBERS "1"

/*
 * check elevation limits
 */

#define CHECK_ELEV_LIMITS "false"
#define MIN_ELEVATION 0.0
#define MAX_ELEVATION 90.0

/*
 * start polar2mdv
 */

#define START_POLAR2MDV "true"
#define POLAR2MDV_COMMAND_LINE "polar2mdv &"

/*
 * output rdi buffer ?
 */

#define WRITE_RDI_MMQ "false"
#define RDI_MMQ_KEY 3740201L

/*
 * flags
 */

#define DEBUG_STR "false"
#define RHI_MODE "false"
#define SECTOR_MODE "false"
#define SURVEILLANCE_MODE "true"

#define FMQ_SEEK_TO_END "true"

#define APPLY_FLAGS "false"
#define USE_BIT_MASK "true"
#define FLAG_CHECK_VAL 0
#define FLAG_VALUE_MIN 0
#define FLAG_VALUE_MAX 3

/*
 * printout params
 */

#define HEADER_INTERVAL 360L
#define SUMMARY_INTERVAL 1L

/*
 * parameter defaults
 */

#define DBZ_FIELD_POS 0L
#define SN_FIELD_POS 3L
#define FLAG_FIELD_POS 2L

#define NS_HEMISPHERE "north"
#define EW_HEMISPHERE "west"

#define NFIELDS_IN 4L
#define FIELD_NAMES "Reflectivity,Vel,Width,S/N"
#define FIELD_TRANSFORM "dbz,none,none,none"
#define FIELD_UNITS "Dbz,m/s,m/s,Db"

#define NGATES_DROPPED 0L
#define NGATES_OUT 660L

#define NOTE "Mile High Radar, Denver, Colorado, USA"
#define RADAR_ID 0L
#define RADAR_NAME "Mile High, Denver"

#define SAMPLES_PER_BEAM 45L
#define NOISE_DBZ_AT_100KM -8.0

#define END_OF_VOL_DECISION "end_of_vol_flag"
#define LAST_TILT_IN_VOL 1L

#define END_OF_GIVEN_TILT 0
#define END_OF_VOL_FLAG 1

#define NBEAMS_BUFFER 1000L
#define SHMEM_KEY 66666L
  
#define MAX_TAPE_READ_WAIT 10.0

/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                     /* program name */
  char *params_path_name;              /* params file path name */
  char *instance;                      /* program instance */

  long malloc_debug_level;

  rdata_shmem_header_t *shmem_header;
  int shmem_header_attached;

  char *shmem_buffer;
  int shmem_buffer_attached;

  int sem_id;
  int semaphores_attached;

  rdata_shmem_beam_header_t *beam_headers;

  int filelist;                        /* only list the files on tape
					* TRUE or FALSE */

  int monitor;                         /* only monitor the stream -
					* do not wait for clients -
					* TRUE or FALSE */

  int apply_flags;                     /* TRUE or FALSE */
  int use_bit_mask;                    /* TRUE or FALSE */
  int flag_check_val;
  int flag_value_min;
  int flag_value_max;
  
  int start_polar2mdv;                 /* TRUE or FALSE */
  char *polar2mdv_command_line;        /* string for starting
					* polar2mdv */

  int debug;

  si32 summary_interval;                /* interval between prints */
  int summary_print;                    /* TRUE or FALSE */
  si32 header_interval;                 /* interval between prints */
  int header_print;                     /* TRUE or FALSE */

  key_t shmem_key;
  si32 nbeams_buffer;

  char *tape_name;

  char *tcpip_host;
  int tcpip_port;

  int udp_port;

  char *fmq_path;
  int fmq_seek_to_end;              /* TRUE or FALSE */

  int device;                       /* TAPE_DEVICE,
				     * TCPIP_DEVICE, LL_UDP_DEVICE,
				     * NCAR_UDP_DEVICE, FMQ_DEVICE */

  int header_type;                  /* LINCOLN_HEADER, RP7_HEADER
				     * or GATE_DATA_HEADER */

  int surveillance_mode;            /* TRUE or FALSE */
  int rhi_mode;                     /* TRUE or FALSE */
  int sector_mode;                  /* TRUE or FALSE */

  int check_tilt_number;            /* TRUE or FALSE */
  int check_gate_spacing;           /* TRUE or FALSE */
  int check_elev_limits;            /* TRUE or FALSE */
  int data_field_by_field;          /* TRUE or FALSE */

  si32 target_gate_spacing;         /* meters */

  int time_override;                /* override time with the current
				     * unix time */

  si32 time_correction;             /* for correcting the data stream
				     * time (secs) */

  double max_elevation;
  double min_elevation;

  int write_rdi_mmq;                /* TRUE or FALSE - write message queue
				     * for RDI */
  key_t rdi_mmq_key;                /* shmem key for rdi buffer */

  si32 n_tilts;
  si32 max_tilt_number;
  si32 *tilt_numbers;               /* holds tilt numbers */
  si32 *tilt_flags;                 /* set to TRUE if that tilt number is 
				     * requested */

  double noise_dbz_at_100km;

  si32 tape_read_wait;              /* wait on tape read  - input in secs
				     * stored as micro-secs */

  si32 fmq_read_wait;               /* wait on FMQ read  - input in secs
				     * stored as micro-secs */

  char *note;
  si32 radar_id;
  char *radar_name;

  int ns_factor;              /* -1 or +1 */
  int ew_factor;              /* -1 or +1 */

  int end_of_vol_decision;          /* END_OF_VOL_FLAG or END_OF_GIVEN_TILT */
  si32 last_tilt_in_vol;

  si32 samples_per_beam;  /* rp7 data has this, and the input
			   * value is overridden */

  si32 ngates_out;
  si32 ngates_dropped;

  si32 nfields_in;
  char **field_name;
  char **field_units;
  char **field_transform;
  si32 dbz_field_pos;
  si32 sn_field_pos;
  si32 flag_field_pos;
  
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

extern void BE_to_ncar_udp_frame_hdr(ncar_udp_frame_hdr_t *hdr);

extern void BE_to_ll_udp_frame_hdr(ll_udp_frame_hdr_t *hdr);

extern void BE_to_ll_params(ll_params_t *params);

extern void BE_to_rp7_params(rp7_params_t *params);

extern void catch_alarm(void);

extern void close_rdi_mmq(void);

extern int fwd_space_file(int tape,
			  si32 nfiles);

extern int fwd_space_record(int tape,
			    si32 nrec);

extern void get_bprp_ptrs(bprp_params_t **params,
			  bprp_data_t **data);
     
extern void get_lincoln_ptrs(ll_params_t **params,
			     ui08 **data);

extern void get_rp7_ptrs(rp7_params_t **params,
			 ui08 **data);

extern void get_gate_data_ptrs(gate_data_radar_params_t **rparams,
			       gate_data_field_params_t **fparams,
			       gate_data_beam_header_t **beam_hdr,
			       ui08 **data);
     
extern int init_rdi_mmq(int mmq_key, int nfields, int ngates,
			char *prog_name, int debug);

extern void list_tape_files(void);

extern void load_beam(si32 beam_num,
		      int load_data);

extern void parse_args(int argc, char **argv);

extern void print_header(void);

extern void print_summary(void);

extern void process_data_stream(void);

extern si32 read_fmq_beam (char **beam_buffer);

extern char *read_ll_udp_beam (void);

extern char *read_ncar_udp_beam (void);

extern void read_params(void);

extern si32 read_tape_beam(char **beam_buffer);

extern int read_tape_bprp(void);

extern void read_tcpip_beam (gate_data_radar_params_t **gate_rparams,
			     gate_data_field_params_t **gate_fparams,
			     gate_data_beam_header_t **gate_header,
			     ui08 **gate_data);

extern int rewind_tape(int tape);

extern void set_beam_flags (si32 prev_beam_num,
			    si32 current_beam_num,
			    int current_beam_valid,
			    int prev_beam_valid,
			    int scan_mode,
			    int *end_of_vol);

extern void set_bprp_ptrs(bprp_params_t *params,
			  bprp_data_t *data);
     
extern void set_gate_data_ptrs(gate_data_radar_params_t *rparams,
			       gate_data_field_params_t *fparams,
			       gate_data_beam_header_t *beam_hdr,
			       ui08 *data);

extern void set_lincoln_ptrs(char *buffer);

extern void set_rp7_ptrs(char *buffer);
     
extern void setup_sems(void);

extern void setup_shmem(void);

extern void tidy_and_exit(int sig);

extern void write_rdi_message(rdata_shmem_beam_header_t *bhdr,
			      ui08 *bdata,
			      si32 nfields_current);

#ifdef __cplusplus
}
#endif

