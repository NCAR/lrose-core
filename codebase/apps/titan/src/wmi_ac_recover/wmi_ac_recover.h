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
/* include file for read_sea routines  -- Rachel Ames 5/97 */


# include <toolsa/os_config.h>
# include <stdio.h>
# include <string.h>
# include <math.h>
# include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>

#include <dataport/bigend.h>
#include <dataport/swap.h>

#include <titan/ac_posn.h>

#include <tdrp/tdrp.h>
#include "wmi_ac_recover_tdrp.h"

# define MAX( a, b ) ((a) > (b) ? (a) : (b))
#define ZERO_STRUCT(p) memset((void *) p, (si32)0, sizeof(*(p)))

# define TRUE 1
# define FALSE 0
# define ERROR -1
# define BAD_INDEX -1
# define OKAY 0
# define ZERO 0
# define TAPE 0
# define DISK 1


# define BUF_SIZE 1024
# define MAX_REC 32768
# define TIME_SIZE 18   /* size of ORIGINAL time struct */
# define MAX_INFO_LINES 2 /* number of info lines at beginning of aquisition table */
# define COMMENT_CHAR ';'

# define CENTURY 1900

# define NUM_SDS_BLOCKS 100

# define MAX_RAW_FIELDS 52
# define MAX_CALC_FIELDS 52

#define IMAGE_BUF_SIZE "image_buf_size"
#define STATE_BUF_SIZE "state_buf_size"

# define BAD_VAL -999.00
# define SMALL_VAL 0.0001

/* conversion factors */
#define C2K 273.15

/* predefined variable types */
#define LAT     "LAT     "
#define LON     "LON     "
#define PALT    "PALT    "
#define SPRES   "SPRES   "

typedef struct {

   ui08 var_type[12];
   ui08 gen_name[12];

} FieldInfo;

typedef struct {

  si16 tagNumber;
  si16 dataOffset;
  si16 numberBytes;
  si16 samples;
  si16 bytesPerSample;
  ui08 type;
  ui08 parameter1;
  ui08 parameter2;
  ui08 parameter3;
  si16 Address;

} DataDir;

typedef struct {

  si16 year;
  si16 mon;
  si16 day;
  si16 hour;
  si16 min;
  si16 sec;
  si16 fract;
  si16 freq;
  si16 life;
  si16 spare;

}  TimeStruct;

typedef struct {
  fl64 c1; 
  fl64 c2; 
  fl64 c3; 
  fl64 c4; 
  si32 tag;
  ui08 name[BUF_SIZE];
  ui08 var_type[12]; /* type of variable */
} FunctionCoefficients;

typedef struct {
  fl64 c1; 
  fl64 c2; 
  fl64 c3; 
  fl64 c4; 
  ui08 name[BUF_SIZE];
  ui08 var_type[12]; /* type of variable */
  ui08 units[12];    /* Data units as provided by calc functions */
} CalculatedVariables;

typedef struct {
  si32 buffer;
  si32 tag;
  si32 array_loc;     /* location of this data in the data_val array of DataStruct */
  si32 samples;     
  si32 size;     
  si32 type;    
  si32 para1;  
  si32 para2; 
  si32 para3;         /* gain = parameter3 + 1 */
  si32 spare;  
  ui08 name[BUF_SIZE];
  ui08 address[8];    
  ui08 var_type[12]; /* type of variable */
  ui08 units[12];    /* Units as provided by calc functions */
} Acqtbl;

typedef struct {

  char *prog_name;                    /* program name */
  TDRPtable *table;                   /* TDRP parsing table */
  wmi_ac_recover_tdrp_struct params;  /* parameter struct */

  int store;                          /* flag - store or not */

  si32 num_raw_fields;
  si32 num_calc_fields;
  si32 num_seconds;

  si32 image_buf_size;
  si32 state_buf_size;

  si32 swap_data; /* true  if data itself is not in native format, 
                  * false if data is in native format
                  */
  Acqtbl *acqtbl;
  FunctionCoefficients *fc;
  CalculatedVariables *cv;

} global_t;

#ifdef MAIN
   global_t *Glob = NULL;
   FieldInfo fi[] = {
        { LAT,   "LAT     " },
        { LON,   "LON     " },
        { SPRES, "SPRES   " },
        { PALT,  "PALT    " },
   };

#else
   extern global_t *Glob;
   extern FieldInfo fi[4];
#endif

/* define reserved tag numbers */

#define TIME_TAG 0
#define NEXT_TAG 999
#define SAME_TAG 65534
#define LAST_TAG 65535


/* extern declarations */

extern void close_output_files(void);

extern fl64 calc_latlon(si32 *int_val);

extern fl64 calc_palt(fl64 static_pres);

extern fl64 calc_pres(fl64 volts_val, fl64 scale, fl64 bias);

extern void exit_str(char * errstr);

extern si32 fill_acqtbl();

extern si32 fill_cvars();

extern si32 fill_fcoeff();

extern si32 find_cvar(ui08 *var_type, si32 vt_len);

extern si32 find_acqvar(ui08 *var_type, si32 vt_len);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern void parse_raw_data(ui08 *buf);

extern int read_tables(void);

extern void store_pos(date_time_t *ptime,
		      double lat,
		      double lon,
		      double alt,
		      char *callsign);

extern si32 swap_ddir(DataDir *ddir);

extern si32 swap_ddir_data(DataDir *ddir,DataDir *ddir_beg);

extern si32 swap_ddir_header(DataDir *ddir);

     





