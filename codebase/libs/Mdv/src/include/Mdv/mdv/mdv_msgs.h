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

/************************************************************************
*                    
* MDV_MSGS.H : Meterological Data Vector messaging definitions
*                   
*                      August 1995
*************************************************************************/
# ifndef    MDV_MSGS_H
# define    MDV_MSGS_H

#include <Mdv/mdv/mdv_file.h>

/* MDV Message ID's */
enum {  MDV_Get_Times = 1,         /* Gets data times, or generation times */
        MDV_Get_Forecasts,         /* Gets the forecast times of a data set */
        MDV_Get_Info,              /* Gets data set info common to all fields */
	MDV_Get_Volume,            /* Gets a raw volume of data */
	MDV_Get_Plane,             /* Extracts a single plane of data */
	MDV_Get_Point,             /* Extracts a single point of data */
	MDV_Get_Chunk,             /* Retrieve a specific chunk of data */
	MDV_Get_RT_Status,         /* Obtains real time data access info */
	MDV_Request_Notification,  /* Ask server to notify client of new data */
	MDV_Client_Goodbye,        /* Clients: send this before disconnecting */

	MDV_Notification,          /* Notification Reply */
	MDV_Times_Reply,           /* Contains list of data generation times */
	MDV_Forecasts_Reply,       /* Contains list of forecast times */
	MDV_Info_Reply,            /* Is a MDV_master_header_t struct */
	MDV_Volume_Reply,          /* Contains field header and data array */
	MDV_Plane_Reply,           /* Contains abreviated field header and 
                                      a data plane */
	MDV_Point_Reply,           /* Contains abreviated field header and 
                                      a data point */
	MDV_Chunk_Reply,           /* Contains externally defined, 
                                      arbitrary data structs */
	MDV_RT_Status_Reply,       /* Contains real time data access info */
	MDV_Request_Failed         /* Contains info on why requests failed */
};

/* This structure is sent before all messages and replies */
/* and gives an indication of the type and size of the associated message */

typedef struct {
    Int32 msg_id;                /* The Message ID */
    Int32 msg_size;              /* Size of the message in bytes */
    Int32 msg_serial;            /* A Serial number - used to match 
                                    requests and replies */
    Int32 msg_reserved;          /* Reserved for future use */
} MDV_msg_head_t;

/*  MDV_Get_Volume, MDV_Get_Plane, MDV_Get_Point Requests */
 
enum Req_type { VOLUME = 1, 
                HORIZ_PLANE, 
                HORIZ_MAXIMA_PLANE, 
                VERT_PLANE, 
                SINGLE_POINT };

enum Req_mode { DEFAULT = 0, 
                WANT_REALTIME, 
                WANT_NEW, 
                WANT_FORECAST, 
                WANT_LATEST_FORECAST };
 
typedef struct {
    Int32  struct_id;
    Int32  request_type;    /* The primary data request */
    Int32  request_mode;    /* A secondary mode/modifier to the command */

    Int32  field_code;      /* Which grib field we want */
    Int32  plane_num;       /* Which plane we want */

    Int32  time_min;        /* Data must be more recent than this time */
    Int32  time_cent;       /* As close as possible to this time */
    Int32  time_max;        /* But not newer than this time */
    Int32  time_last;       /* Used for WANT_REALTIME, WANT_NEW modes - */
    Int32  time_generated;  /* Used for WANT_FORECAST - 
                               Choose data generated closest to this time  */

    Int32  ireserved[22];

    Float32 lat_min;
    Float32 lat_max;
    Float32 lon_min;
    Float32 lon_max;
    Float32 alt_min;
    Float32 alt_max;

    Float32 freserved[26];
}  MDV_data_request_t;

/*  MDV_Get_Volume Reply */
typedef struct {
    MDV_field_header_t fhead;
    void	*data;
} MDV_data_reply_t;

/*  MDV_Get_Plane,Point Replies */
typedef struct {
    MDV_field_header_t fhead;    /* Minus 4*sizeof(int)*MAX_V_LEVELS 
                                    (last 1536 bytes) */
    void	*data;
} MDV_data_reply_t;



/* MDV_Get_RT_Status Request */
typedef struct {
    Int32  struct_id;
    Int32  time_last;
    Int32  ireserved[30];
    Float32 freserved[32];
}  MDV_status_request_t;

/* MDV_RT_Status Reply */
typedef struct {
    Int32  struct_id;
    Int32  time_updated;

    Int32  ireserved[30];
    Float32 freserved[32];
     
    Byte   status[256]; /* The Source's Realtime status - Human readible */
    Byte   reason[256]; /* Why the source has this status */
} MDV_status_reply_t;

/* MDV_Get_Times Request */
typedef struct {
    Int32  struct_id;
    Int32  time_begin;
    Int32  time_end;

    Int32  ireserved[29];
    Float32 freserved[32];
}  MDV_time_list_request_t;

/* MDV_Times Reply */
typedef struct {
    Int32  struct_id;
    Int32  n_entries;   /* Number of entries in the list */
    Int32  time[1];     /* Actually n_entries in size when sent */
} MDV_time_list_msg_t;


/* MDV_Get_Forecasts Request */
typedef struct {
    Int32  struct_id;
    Int32  generation_time;  /* Explicit generation time */

    Int32  ireserved[30];
    Float32 freserved[32];
}  MDV_fcast_list_request_t;

/* MDV_Forecasts Reply */
typedef struct {
    Int32  struct_id;
    Int32  n_entries;   /* Number of entries in the list */
    Int32  time[1];     /* Actually n_entries in size when sent */
} MDV_fcast_list_msg_t;

/* MDV_Get_Chunk Request */
typedef struct {
    Int32  struct_id;
    Int32  request_mode;    /* The primary data request */
    Int32  chunk_id;        /* The Id of a chunk */
     
    Int32  generation_time; /* Set data generation time to get explicit chunk */
    Int32  forecast_time;   /* Chunk must be more recent than this time */
    Int32  time_min;        /* Chunk must be more recent than this time */
    Int32  time_cent;       /* As close as possible to this time */
    Int32  time_max;        /* But not newer than this time */
    Int32  time_last;       /* Used for WANT_REALTIME, WANT_NEW modes - */

    Int32  ireserved[23];
    Float32 freserved[32];
}  MDV_chunk_request_t;

/* MDV_Chunk Reply */
/* Chunks have their own, arbitrary, externally defined structures */
 

/* MDV_Request_Notification Request */
typedef struct {
    Int32  struct_id;
    Int32  port;

    Int32  ireserved[14];
    Float32 freserved[16];
    Byte   host[256];
}  MDV_fcast_list_request_t;

/* MDV_Notification Reply */
typedef struct {
    Int32  struct_id;
    Int32  port;
    Byte   host[128];
    Byte   type[128];
    Byte   instance[128];

    Int32  n_entries;   /* Number of entries in the list */
    Int32  time[1];     /* Actually n_entries in size when sent */
} MDV_fcast_list_msg_t;
# endif     /* MDV_MSGS_H */


#ifdef __cplusplus
}
#endif
