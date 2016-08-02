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
/****************************************************************************
 * DIST_CDATA.H : Defines & includes
 */

#include <toolsa/os_config.h>

#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>
#include <toolsa/ldata_info.h>

#define MAX_FILES_ON_START 1

#define MAX_CHILDREN 8	  /* Maximum number of clid processes allowed 
			   * to spawn at a time to transfer data */

#define MIN_CMD_LEN 3     /* Minimum reasonable size of a command to run */

#define QUIESCENT_SECS 3  /* Minimum number of seconds files should
			   * be quescent before transfer */

#define CHECK_SECS 15     /* Wait this long before checking again
			   * for new files */

#define TIMEOUT_SECS 60

#define MAX_NHOSTS 16
#define HOST_MAX 64
#define SUFFIX_MAX 32
#define STAMP_MAX 32
#define CMD_MAX 256
#define FORMAT_MAX 256
#define FILE_MAX 128

/******************************************************************************
 * GLOBAL DATA DECLARATIONS
 */

struct Global_data {

  int debug;          /* Normal debugging flag  */
  int num_hosts;
  int compress_flag;  /* Set this to 1 to compress data before copying  */
  int gzip_flag;      /* Set this to 1 to gzip data before copying  */
  int uncompress_flag; /* uncompress or gunzip after transfer */
  int timeout;        /* number of seconds to allow for file transfer */
  int cdata_index;    /* send cdata index file to remotes */
  int secure_shell;   /* use secure shell for transfers */

  int check_secs;     /* how often to check */

  int quiescent_secs; /* how log file must be quiescent */
  
  int max_files_on_start; /* Maximum number of files output at at startup 
			   * to catch up with unsent files */

  char app_instance[PROCMAP_INSTANCE_MAX];  /* Instance of this process */
  
  char suffix[SUFFIX_MAX];      /* suffix of data files in
				 * output directory */
  
  char stamp_file[MAX_PATH_LEN];  /* File watch for changes; indicates new
				   * data availible when updated */
  
  char per_file_cmd[CMD_MAX];  /* cmd to run on each remote host
				* after each file is transfered */
  
  char end_cmd[CMD_MAX];       /* cmd to run on each remote host
				* after all files are transfered */
  
  char remote_host[MAX_NHOSTS][HOST_MAX]; /* list of hosts to which to
					   * distribute the data */
  
  char source_dir[MAX_PATH_LEN]; /* directory in which to look for files */
  
  char dest_dir[MAX_PATH_LEN];   /* The destination directory
				  * for reomte host(s) */
  
  char cur_host[HOST_MAX];       /* name of host */
  
  char cur_file[MAX_PATH_LEN];   /* name of file being transferred */
  
};

/*
 * instance of global data struct
 */

#ifdef MAIN

struct Global_data gd;

#else

extern struct Global_data gd;

#endif

/*
 * function prototypes
 */

extern void build_formats(char *format_str,
			  char *rem_dir_format_str,
			  char *rem_file_format_str);

extern int find_times(time_t **time_ptr,
		      char *top_dir,
		      char *file_suffix,
		      time_t min_time,
		      time_t max_time);

extern int process_args( int argc, char *argv[]);

extern int safe_sys(const char * command, int timeout_seconds,
		    int debug);

extern int send_files(time_t file_time, time_t last_time,
		      char *tmp_dir, char *rsh, char *rcp);

extern void send_index_file(char *index_path, char *remote_host);

extern int send_to_host(char *host_name, int *num_children_p,
			char *date_dir, char *file_name,
			char *tmp_name, char *index_path,
			char *rsh, char *rcp);
  
extern void set_timer(int seconds);

extern void signal_trap(int signal);

extern void timer_func(int sig);

extern char *write_index_file(long data_time);

extern char *write_tmp_log(void);


