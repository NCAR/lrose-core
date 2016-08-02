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

/*********************************************************************
 * polar2gate.h
 *
 * The header for polar2gate program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1991
 *
 *********************************************************************/

#include <toolsa/umisc.h>
#include <rapformats/rp7.h>
#include <rapformats/lincoln.h>
#include <rapformats/gate_data.h>
#include <titan/radar.h>
#include <rapformats/chill.h>
#include <rapformats/alenia.h>
#include <rapformats/lass.h>
#include <toolsa/sockutil.h>
#include <toolsa/xdru.h>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/time.h>

/*
 * local struct for chill header and field params
 */

/*
 * Adding Lass format details. ( sanjiv )
 */


#define MAX_CHILL_FIELDS 16
#define MAX_SEGNAME 32


typedef struct {

  chldat1_t chldat1;
  int scan_mode;
  int bypass;
  si32 pulse_len;
  si32 gate_len;
  si32 ngates;
  si32 txbin;
  si32 field_flag;
  si32 nfields_current;
  si32 scan_type;
  double prf;
  double wavelength;
  double gate_spacing;
  double start_range;
  double nyquist;
  double target_el;
  gate_data_field_params_t fparams[MAX_CHILL_FIELDS];
  char field_str[MAX_CHILL_FIELDS * 3];
  char segname[MAX_SEGNAME];

} chill_params_t;

/*
 * scales and biases
 */

#define DBZ_SCALE 0.5
#define DBZ_BIAS -30.0

/*
 * factor by which scale and bias values are to be multiplied
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

#define HEADER_TYPE "rp7"

enum precip_map_debug {
	DEBUG_OFF = 0,
	DEBUG_WARNINGS = 1,
	DEBUG_NORM = 2,
	DEBUG_EXTRA = 3};
typedef enum precip_map_debug precip_map_debug;

enum {
  LINCOLN_HEADER = 0,
  RP7_HEADER  = 1,
  CHILL_HEADER = 2,
  LASS_HEADER = 3,
  ALENIA_HEADER = 4
};

#define DEVICE "tape"

enum {
  TAPE_DEVICE = 0,
  DISK_DEVICE = 1
};

#define DEVICE_NAME "null"
#define DEVICE_READ_WAIT 0.1
#define MAX_DEVICE_READ_WAIT 10.0

#define INPUT_FORMAT "ncar"

enum {
  NCAR_FORMAT = 0,
  CHILL_FORMAT = 1,
  LASS_FORMAT  = 2,
  ALENIA_FORMAT = 3
};

#define OUTPUT_PORT 50000L

#define MAX_NBYTES_BEAM_BUFFER 65536L

/*
 * gate spacing checking
 */

#define CHECK_GATE_SPACING "true"
#define TARGET_GATE_SPACING 0.250

/*
 * flags
 */

#define DEBUG_STR "false"
#define REPORT_MISSING_BEAMS "true"
#define SCAN_MODE "surveillance"
#define CHILL_EXTENDED_HSK "false"

/*
 * printout params
 */

#define HEADER_INTERVAL 360L
#define SUMMARY_INTERVAL 1L

/*
 * parameter defaults
 */

#define OUT_FIELD_POS "0 1"
#define NFIELDS_IN 4L
#define NFIELDS_OUT 2L

#define NGATES_DROPPED 0L
#define NGATES_IN 660L
#define NGATES_OUT 660L

/*
 * time correction (secs)
 */

#define TIME_CORRECTION 0L
#define SET_TIME_TO_CURRENT "false"

/*
 * elevation table - for alenia ops, since they do not have
 * tilt_num and vol_num
 */

#define NELEV_TABLE (si32) 11
#define ELEV_TABLE "0.5 1.2 2.5 4.0 5.5 7.0 8.5 10.0 13.0 17.0 22.0"

/*
 * chill operations
 */

#define CHILL_FIELDS_OUT "IP"
#define NCHILL_SCAN_TYPES 1L
#define CHILL_SCAN_TYPES "EEPPI"
#define CHILL_VOL_START_SCAN_TYPE "EEPPI"
#define CHILL_VOL_START_TILT 1L

/*
 * radar details
 */

#define RADAR_ID 0L

#define SAMPLES_PER_BEAM 45L

#define ALTITUDE 0.0
#define LONGITUDE 0.0
#define LATITUDE 0.0  

#define GATE_SPACING 0.5
#define START_RANGE 0.25

#define BEAM_WIDTH 1.0
#define PRF_NOMINAL 1000.0
#define WAVELENGTH 10.0
#define PULSE_WIDTH 2.0

/*
 * globals struct - the defines which are applicable are inserted here
 */

typedef struct {

  char *prog_name;                  /* program name */
  char *params_path_name;           /* params file path name */

  int filelist;                     /* only list the files on tape
				     * TRUE or FALSE */

  int debug;                        /* TRUE or FALSE */
  int data_field_by_field;          /* TRUE or FALSE */
  int set_time_to_current;          /* TRUE or FALSE */
  int chill_extended_hsk;           /* TRUE or FALSE */

  int scan_mode;                    /* GATE_DATA_SECTOR_MODE or
				     * GATE_DATA_RHI_MODE or
				     * GATE_DATA_SURVEILLANCE_MODE */

  si32 summary_interval;            /* interval between prints */
  int summary_print;                /* TRUE or FALSE */
  si32 header_interval;             /* interval between prints */
  int header_print;                 /* TRUE or FALSE */

  char *device_name;

  char *chill_calibration_path;     /* path of file with Chill calibration
				     * data */

  int device;                       /* TAPE_DEVICE or DISK_DEVICE */

  int input_format;                 /* NCAR_FORMAT or CHILL_FORMAT */

  int output_port;                  /* port for socket */

  int header_type;                  /* LINCOLN_HEADER, RP7_HEADER or
				     * CHILL_HEADER */

  int check_gate_spacing;           /* TRUE or FALSE */
  si32 target_gate_spacing;         /* meters */

  si32 device_read_wait;            /* wait on read input in secs,
				     * stored as micro-secs */

  si32 max_nbytes_beam_buffer;

  si32 ngates_in;
  si32 ngates_out;
  si32 ngates_dropped;              /* number of start gates to
				     * be dropped */

  si32 nfields_in;
  si32 nfields_out;
  si32 *out_field_pos;

  int use_elev_table;               /* option to use elev table to
				     * set tilt_num and vol_num */
  si32 nelev_table;
  double *elev_table;

  si32 time_correction;              /* seconds added to the data time
				      * before transmission */

  char **chill_fields_out;           /* array of chill fields to be
				      * output */

  si32 nchill_scan_types;            /* number of chill scan types to be
				      * processed */

  char **chill_scan_types;           /* array of chill scan types to
				      * be processed */

  char *chill_vol_start_scan_type;   /* scan type for start of chill
				      * volume */

  si32 chill_vol_start_tilt;         /* tilt for start of chill volume */

  /*
   * radar parameters to go to socket - these override any parameters 
   * in the input stream
   */

  si32 radar_id;
  si32 samples_per_beam;
  double altitude;
  double longitude;
  double latitude;
  double gate_spacing;
  double start_range;
  double beam_width;
  double pulse_width;
  double prf_nominal;
  double wavelength;

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

extern int fwd_space_file(int tape,
			  si32 nfiles);

extern int fwd_space_record(int tape,
			    si32 nrec);

extern void list_tape_files(void);

extern void load_beam(ui08 *beam_buffer,
		      ui08 *gate_params_pkt,
		      ui08 *gate_data_packet,
		      int load_data);

extern void parse_args(int argc,
		       char **argv);

extern void print_header(ui08 *beam_data);

extern void print_summary(ui08 *beam_data);

extern void process_data_stream(void);

extern si32 read_alenia_record(char *output_buffer);

extern si32 read_disk_alenia (ui08 *beam_buffer);

extern si32 read_disk_lass (ui08 *beam_buffer);

extern si32 read_lass_record(char *output_buffer);

extern void read_params(void);

extern si32 read_tape_chill(ui08 *beam_buffer);

extern si32 read_tape_ncar(ui08 *beam_buffer);

extern si32 read_tape_record(char *tape_buffer);

extern int rewind_tape(int tape);

extern int set_beam_flags(ui08 *beam_buffer,
			  ui08 *gate_data_pkt,
			  ui08 *prev_gate_data_pkt);

extern int update_params(ui08 *beam_buffer,
			 ui08 *gate_params_pkt,
			 si32 nbytes_params_pkt);

extern void tidy_and_exit(int sig);

