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
/*******************************************************************************
 * MDV_SERVER.H: Defines and Globals for the Cedric File format data service
 *
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <malloc.h>
#include <memory.h>
#include <sys/socket.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <netinet/in.h>

#include <toolsa/os_config.h>

#include <cidd/cdata_util.h>
#include <cidd/cidd_files.h>
#include <cidd/cidd_lib.h>

#include <dataport/bigend.h>
#include <dataport/port_types.h>
#include <dataport/swap.h>

#include <mdv/mdv_user.h>

#include <toolsa/file_io.h>
#include <toolsa/pjg.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/procmap.h>
#include <toolsa/smu.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#define MDV_SERVICE_NAME     "MDV"
#define MDV_SERVICE_INSTANCE "Operational"
#define MDV_SERVER_PORT      65151        /* This services default port */
#define MAX_SUFFIX_LEN       32
#define MAX_INPUT_DIRS       8
#define MAX_FILE_FIELDS      256
#define MAX_DATA_PLANES      256

#define DATA_FILE_SUFFIX     "mdv"        /* The data file default suffix */
#define DATA_NAME_BASE_LEN   6            /* The data file base name length
                                           * base is currently "HHMMSS" */
#define VALID_TIME           356          /* number of seconds that data
                                          * remains valid */

/* DATA INFO NOT AVAILABLE IN FILES - But used in Protocol - (IMPLIED) */
#define DATA_ORDER           1       /* 1 = Y increases UP, (right hand)
                                      * 0 = Down (left hand) */
#define GATE_SPACING         0
#define WAVELENGTH           0 
#define FREQUENCY            0  

#define MIN_RANGE            0
#define MAX_RANGE            0
#define NUM_GATES            0
#define MIN_GATES            0
#define MAX_GATES            0

#define NUM_TILTS            0
#define MIN_ELEV             0
#define MAX_ELEV             0

#define RADAR_CONST          0
#define NYQUIST_VEL          0
#define DELTA_AZIMUTH        0
#define START_AZIMUTH        0
#define BEAM_WIDTH           0.0
#define PULSE_WIDTH          0.0
#define NOISE_THRESH         0.0 

/* Server Mapper information */
#define SERVMAP_HOST1        "localhost"
#define SERVMAP_HOST2        ""

#define KM_PER_DEG_AT_EQ 111.198487

/*  OR's flag with 1 if cliped and "returns" a value >= l && <= u */
#define CLIP(x,l,u,flag) (x < l) ? flag|=1,l: ((x > u)? flag|=1,u:x)
#define ABSDIFF(a,b) (((a) < (b))? (b)-(a): (a)-(b))
#define DIST(a,b,c,d) sqrt((((c)-(a))*((c)-(a))) + (((d)-(b))*((d)-(b))))

/***************************** FUNCTION PROTOTYPES *************************/
#ifndef MDV_SERVER_ARGS
void process_args(int argc, char *argv[]);
#endif

#ifndef MDV_SERVER_CHOOSE_FILE
int choose_and_open_file(cdata_ieee_comm_t *com, cdata_ieee_comm2_t * com2);
char *get_data_filename(long time, int num_top_dirs,
                        char **top_dir, char *suffix,
                        time_t *update_time);
#endif

#ifndef MDV_SERVER_GET_CLIENT
void get_client_loop(void);
void process_request(int id, cdata_comm_t *com, void *comm2_buf);
void  com2_struct_from_BE(void * buf);
#endif

#ifndef MDV_SERVER_GET_DATA
ui08 *get_grid_data(cdata_ieee_comm_t *com,cdata_ieee_comm2_t * com2);
#endif

#ifndef MDV_SERVER_GET_V_PLANE
int get_v_plane(cdata_ieee_comm_t *com,cdata_ieee_comm2_t * com2,
                ui08 **mdv_plane);

int get_v_grid_array(cdata_ieee_comm_t *com,
                  cdata_ieee_comm2_t * com2,
                  int *x_grid_array,    /* RETURNED VALUES */
                  int *y_grid_array);

void get_v_grid_zlimits(cdata_ieee_comm_t *com,
                  cdata_ieee_comm2_t * com2,
                  int *rz1,    /* RETURNED VALUES */
                  int *rz2);
#endif

#ifndef MDV_SERVER_GET_MAX_XY_PLANE
int get_max_xy_plane(cdata_ieee_comm_t *com, cdata_ieee_comm2_t * com2,
                 ui08 **mdv_plane);
#endif

#ifndef MDV_SERVER_GET_XY_PLANE
int get_xy_plane(cdata_ieee_comm_t *com,  ui08 **mdv_plane);

void native_units_to_grid(double native_x, double native_y,
                          int *return_grid_x, int *return_grid_y);
#endif

#ifndef MDV_SERVER_GRID_INFO
void copy_to_int_info(cdata_info_t *i_info, cdata_ieee_info_t  *info);
void copy_header_to_info(cdata_ieee_info_t *info);
#endif

#ifndef MDV_SERVER_INIT
void init_data_space(void);
void init_sockets(void);
void signal_trap(int sig);
#endif

#ifndef MDV_SERVER_REPLY
void send_reply(int id,
                cdata_ieee_comm_t    *com, 
                cdata_ieee_reply_t   *reply,
                cdata_ieee_info_t    *info,
                ui08    *ptr); 

void initialize_reply(cdata_ieee_reply_t *reply);
void copy_to_int_reply(cdata_reply_t *i_reply, cdata_ieee_reply_t *src_reply);


#endif

/*********** Global Data ********************************************************
 *
 */
struct    Global_Data {
    int  port;
    int  protocol_version; /* 0 = Original, 2 = IEEE,URL, Multi point Xsects */
    int  reg;        /* flag for registering with the servermapper */
    int  num_children;  /* Number of children to spawn for service */
    int  clients_serviced;  /* The total number of client requests serviced so far */
    int  compress_flag;
    int  request_field_index;
    int  found_field_index;
    int  daemonize_flag;
    int  num_fheads_alloc;   /* the number of allocated slots for field heads*/
    int  num_top_dirs;       /* the number of top level directories */
    int  live_update;        /* flag to indicate updating data avail */
    int  override_origin;    /* flag 1 = Use origin on command line,
                              * 0 = use file's origin */
    char app_name[PROCMAP_NAME_MAX];          /* the applications name */
    char app_instance[SERVMAP_INSTANCE_MAX];  /* the services instance */
    char service_subtype[SERVMAP_NAME_MAX];   /* the services subtype name */
    char user_info[SERVMAP_USER_INFO_MAX];    /* the services user info */
    char servmap_host1[SERVMAP_HOST_MAX];     /* the servermapper host 1 */
    char servmap_host2[SERVMAP_HOST_MAX];     /* the servermapper host 2 */

    char data_file_suffix[MAX_SUFFIX_LEN];    /* the data file names suffix */

    long    data_length;                      /* data length in bytes */
    time_t  last_request_time;
    time_t  last_file_time;                   /* The last data handle */
    time_t  file_read_time;                   /* Time file was read, in case */
                                              /* it is updated and needs to */
                                              /* be re-read. */
    time_t  last_time_cent;
    int     last_requested_field;             /* Which data field was last requested */

    MDV_master_header_t cur_m_head;           /* Current Master Header */
    MDV_field_vlevel_header_t *cur_f_head;    /* Field of interest Header */
    /* Current File Headers */
    MDV_field_vlevel_header_t *fhead[MAX_FILE_FIELDS];
     
    cdata_ieee_info_t current_ieee_info;   /* Current Grid info */
    cdata_ieee_reply_t current_ieee_reply; /* Current reply */
     
    char *top_dir[MAX_INPUT_DIRS];       /* the data directories */
    char *top_dir_url[MAX_INPUT_DIRS];   /* the data dir with url dir added */
    char last_data_filename[1024];       /* the data filename last opened */
    char *url;                           /* The URL */
    char *req_field_name;                /* The Requested Field Name */
    int  data_file_open;                 /* status for data file */
    FILE *data_file;                     /* Stream discriptor for data */

    char *input_filename;                /* The name of the single input */
                                         /* file to use.  This overrides */
                                         /* the top_dir array.  This is */
                                         /* for use with applications like */
                                         /* cart_slave that rewrite a single */
                                         /* input file frequently. */
    
    double  origin_lat;     /* Origin of the coordinate system */
    double  origin_lon;
    double  origin_alt;
    double  x_offset;       /* Distance between the local origin */
    double  y_offset;       /*  ...  and the clients origin */

    float   plane_heights[MAX_DATA_PLANES * 3];
    int     num_planes;

    int        protofd;
    int        sockfd;
};


#ifdef MDV_SERVER_MAIN

struct Global_Data gd;

#else

extern struct Global_Data gd;

#endif

