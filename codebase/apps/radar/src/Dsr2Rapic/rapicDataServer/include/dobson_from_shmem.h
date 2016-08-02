#ifdef __cplusplus
 extern "C" {
#endif
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright (c) 1992, UCAR
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1993/3/9 16:6:22
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* SCCS info
 *   %W% %D% %T%
 *   %F% %E% %U%
 *
 * RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2007/10/25 03:37:27 $
 *   $Id: dobson_from_shmem.h,v 1.1 2007/10/25 03:37:27 dixon Exp $
 *   $Revision: 1.1 $
 *   $State: Exp $
 *
 *   $Log: dobson_from_shmem.h,v $
 *   Revision 1.1  2007/10/25 03:37:27  dixon
 *   Adding BOM rapic server code for building Dsr2Rapic
 *
 *   Revision 1.1.1.1  2003/06/19 01:19:15  pjp
 *   3DRapic Sources
 *
 *   Revision 1.2  1999/02/04 05:54:03  sandy
 *   Rapic version 372
 *
 * Revision 3.26  1997/11/11  23:34:02  pjp
 * 3DRapic Source Code
 *
 * Revision 3.24  1997/10/23  05:11:50  pjp
 * 3DRapic Source Code
 *
 * Revision 3.24  1997/10/23  05:11:50  pjp
 * 3DRapic Source Code
 *
 * Revision 3.23  1997/10/17  08:40:03  pjp
 * 3DRapic Source Code
 *
 * Revision 3.23  1997/10/17  08:40:03  pjp
 * 3DRapic Source Code
 *
 * Revision 3.19  1997/08/29  03:46:33  pjp
 * 3DRapic Source Code
 *
 * Revision 3.19  1997/08/29  03:46:33  pjp
 * 3DRapic Source Code
 *
 * Revision 3.19  1997/08/29  03:15:25  pjp
 * 3DRapic Source Code
 *
 * Revision 1.4  94/12/08  03:26:07  dixon
 * Added interpolation for single missing beam
 * 
 * Revision 1.3  1994/12/03  19:28:42  dixon
 * Added check_missing_beam option
 *
 * Revision 1.2  1994/11/18  01:53:09  dixon
 * Added cpluplus link directive
 *
 * Revision 1.1.1.1  1994/11/17  18:52:58  dixon
 * Initial import
 *
 * Revision 1.5  1994/02/09  04:07:01  dixon
 * Changed to 8 bit compression on dobson files
 *
 * Revision 1.4  1994/01/18  01:19:22  dixon
 * Rearranged include file directory structure
 *
 * Revision 1.3  1994/01/16  17:18:45  dixon
 * Bug fix
 *
 * Revision 1.2  1993/08/23  19:55:57  dixon
 * Bug fixes
 *
 * Revision 1.1.1.1  1993/07/13  18:24:10  dixon
 * Initial import
 *
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/*************************************************************************
 * dobson_from_shmem.h
 *
 * The header for dobson_from_shmem program
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, CO, USA
 *
 * December 1990
 *
 *************************************************************************/


#include <dix_util/dix_util.h>
#include <dix_util/dix_mutil.h>
#include <dix_util/dix_rfutil.h>
#include <toolsa/sockutil.h>

#include <sys/param.h>

/*
 * default defines
 */

/*
 * output socket objects
 */

#define OUT_OBJ_NAME "cartesian_radar_data"
#define N_OUT_OBJS 1
#define OUT_OBJ 0

/*
 * server mapper
 */

#define SERVMAP_HOST1_STR "stratus"
#define SERVMAP_HOST2_STR "cyclone"

#define INFO "Cartesian slave server"
#define INSTANCE "Generic"

/*
 * flags
 */

#define AUTO_MID_TIME "false"
#define CHECK_MISSING_BEAMS "false"
#define REPORT_MISSING_BEAMS "false"
#define DEBUG_STR "false"
#define LOCAL_FILES "false"
#define REMOTE_FILES "false"
#define REMOVE_CLUTTER "false"
#define RUN_LENGTH_ENCODE "false"
#define TRANSMIT_CART_DATA "false"
#define USE_REPEATED_ELEVATIONS "true"
#define USE_MAX_ON_REPEAT "false"

/*
 * default values
 */

#define AZIMUTH_OFFSET 0.5
#define AGE_AT_END_OF_VOLUME 180L
#define CHECK_CLIENTS_INTERVAL 1.0
#define CLUTTER_TABLE_PATH "clutter_table"
#define DOBSON_FILE_EXT "dob"
#define FIELD_POSITIONS "1 2 3 4 5 6"
#define LOCAL_RDATA_DIR "rdata"
#define MAX_MISSING_BEAMS 50L
#define MAX_PACKET_LENGTH 1400L
#define MIN_SN_THRESHOLD -28.0
#define MIN_VALID_RUN 5L
#define MISSING_DATA_VAL 0x00
#define NFIELDS_PROCESSED 6L
#define PORT 50001L
#define RC_TABLE_PATH "rc_table"
#define REMOTE_HOSTNAME "virga"
#define REMOTE_RDATA_DIR "rdata"
#define SHMEM_KEY 66666L
#define SHMEM_READ_WAIT 0.1
#define SN_THRESHOLD 0.0

/*
 * global structure
 */

typedef struct {

  char *prog_name;                       /* program name */
  char *params_path_name;                /* parameters file path name */
  char *path_delim;                      /* path delimiter */

  /*
   * shmem data areas
   */

  char *shmem_buffer;
  rdata_shmem_header_t *shmem_header;
  int sem_id;
  int port;

  char *info;                   /* server mapper info for this
				 * server */

  char *instance;               /* server mapper instance - matched
				 * on requests to the server mapper */

  char *servmap_host1;          /* server mapper hosts */
  char *servmap_host2;

  long latest_data_time;        /* times for registering with server */
  long latest_request_time;	/* mapper */

  /*
   * pointers into shared memory
   */

  field_params_t *field_params;
  rdata_shmem_beam_header_t *beam_headers;

  /*
   * cartesian table and clutter table index pointers
   */

  rc_table_file_index_t rc_index;
  clutter_table_file_index_t clutter_index;

  /*
   * radar volume file index
   */

  vol_file_index_t vindex;

  /*

   * files, directories and hosts
   */

  char *local_rdata_dir;
  char *remote_rdata_dir;
  char *remote_hostname;
  char *dobson_file_ext;

  /*
   * clutter and cartesian lookup tables
   */
  
  char *clutter_table_path;
  char *rc_table_path;

  /*
   * flag to indicate whether to overwrite the array with data
   * from later tilts at the same target elevation
   */

  int use_repeated_elevations;

  /*
   * flag to indicate whether to take max value if an elevation is repeated.
   * The use of this option depends upon using 0 for the missing value.
   */

  int use_max_on_repeat;

  /*
   * age at end of volume (secs) - this is the estimated mean age of the
   * data when the volume is complete. If auto_mid_time is set TRUE, then
   * the mid time is computed as the mean of the start and end times of
   * the data. Otherwise, it is computed as the end time minus the
   * age at the end of the volume
   */

  int auto_mid_time;              /* TRUE or FALSE */
  long age_at_end_of_volume;

  /*
   * shared memory
   */

  key_t shmem_key;
  long shmem_read_wait;            /* wait on the read if no data is
				    * ready - input as secs, stored
				    * as micro-secs */

  /*
   * allowable number of missing beams
   */

  long max_missing_beams;          /* max allowed number of missing beams
				    * per volume */

  /*
   * azimuths, elevations, gates
   */

  long ngates;
  long nazimuths;
  long nelevations;
  double *radar_elevations;
  double *elev_limits;
  double start_azimuth;
  double delta_azimuth;
  double azimuth_offset;

  /*
   * noise removal
   */

  double sn_threshold;             /* signal/noise threshold */
  long min_valid_run;              /* min run length (in a beam) of data
				    * above the signal/noise threshold for
				    * the data to be considered valid */

  /*
   * number of fields to be processed, and their positions in the
   * data stream
   */

  long nfields_processed;
  long *field_positions;

  /*
   * socket parameters
   */

  long max_packet_length;
  double check_clients_interval;
  char *service_name;

  /*
   * decision flags
   */

  int debug;
  int local_files;
  int remote_files;
  int run_length_encode;
  int remove_clutter;
  int transmit_cart_data;
  int check_missing_beams;
  int report_missing_beams;

} global_t;

/*
 * globals
 */

#ifdef MAIN

global_t *Glob = NULL;

/*
 * if not main, declare global struct as extern
 */

#endif
#ifndef MAIN 

extern global_t *Glob;

#endif

/*
 * function prototypes
 */

extern long elev_index(double elev);

extern void check_for_new_clients();

extern void check_geom(void);

extern void handle_end_of_volume(void);

extern void init_cart_vol_index(int allocFlag);

extern void init_radar_vol_index(void);

extern void parse_args(int argc,
		       char **argv);

extern void process_beam(rdata_shmem_beam_header_t *bhdr,
			 u_char *field_data);

extern void read_params(void);

extern void read_shmem(void);

extern void register_server(void);

extern void setup_shmem(void);

extern void setup_sockets();

extern void tidy_and_exit(int sig);

extern void unlink_tmp_files(void);

extern void write_volume(long int nscan,
			 long int nrays_target,
			 long int nrays,
			 long int nnoise,
			 long int *sum_noise,
			 char *name);
#ifdef __cplusplus
}
#endif
