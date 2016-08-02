#ifdef __cplusplus
 extern "C" {
#endif
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
#include <toolsa/umisc.h>
#include <titan/file_io.h>
#include <rapmath/umath.h>
#include <titan/radar.h>
#include <toolsa/sockutil.h>
#include <toolsa/pmu.h>

#define NFIELDS_PROCESSED    	1
#define FIELD_POSITIONS      	0

#define MISSING_DATA_VAL	0x00
#define DEBUG_STR           	"false" 
#define AUTO_MID_TIME       	"true" 
#define RUN_LENGTH_ENCODE   	"true" 
#define CHECK_MISSING_BEAMS 	"true" 
#define REPORT_MISSING_BEAMS 	"false"
#define MAX_MISSING_BEAMS	 200


/*
 * global structure
 */

typedef struct {

  char *prog_name;                       /* program name */
  char *params_path_name;                /* parameters file path name */
  int debug;
  char *path_delim;                      /* path delimiter */

  char *rdata_dir;
  char *local_tmp_dir;
  char *output_file_ext;
  
  int auto_mid_time;              /* TRUE or FALSE */



  long maxgates;
  int  maxazs;
  float  maxrange;
  int maxpoints;
  long nazimuths;
  long nelevations;

  /*
   * number of fields to be processed, and their positions in the
   * data stream
   */

  float scaleFactor[1];
  float biasFactor[1];
  long  samples_per_beam;
  long  pulse_width;
  long  prf;
  long nfields_processed;
  long *field_positions;
  int   field_positions_alloced;
  field_params_t *field_params;

  double sn_threshold;

  long   min_valid_run;
  int check_missing_beams;
  int report_missing_beams;
  int max_missing_beams;

  int run_length_encode;

  double deltax,deltay,deltaz;
  double minx,miny,minz;
  double maxx,maxy,maxz;

  int    ngates;
  double startRange; // All In Km  
  double gateSpace;
  double radarZ;

  //vol_file_index_t vindex;   //SD excised 24jun05
  cart_params_t *cart;

  double *radar_elevations;
  double *elev_limits;
  int     elevations_alloced;
  
  long **plane_heights;
  date_time_t gtime;

  radar_params_t *rparams;
  double noise_dbz_at_100km;
  char *note;

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


extern void handle_end_of_volume(void);
extern void parse_args(int argc,
		       char **argv);
extern void process_beam(rdata_shmem_beam_header_t *bhdr,
			 u_char *field_data);
extern void read_params(void);

extern void tidy_and_exit(int sig);
extern void write_volume(long int nscan,
			 long int nrays_target,
			 long int nrays,
			 long int nnoise,
			 long int *sum_noise,
			 char *name);
#ifdef __cplusplus
}
#endif
