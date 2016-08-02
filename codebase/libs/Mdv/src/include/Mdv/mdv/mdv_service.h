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
ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
*                    
* MDV_SERVICE.H : Meterological Data VOlume messaging definitions
*                   
*                      April  1997
*************************************************************************
# ifndef    MDV_SERVICE_H
# define    MDV_SERVICE_H

#include <Mdv/mdv/mdv_file.h>

/* MDATA Message ID's */
enum {  MDVS_Get_Times = 1,    /* Gets data times, or generation times */
        MDVS_Get_Forecasts,    /* Gets the forecast times of a data set */
	MDVS_Get_RT_Status,    /* Obtains real time data access information */
	MDVS_Request_Notification, /* Ask the server to notify the client of
                                   new data */

        MDVS_Get_Dataset,      /* Gets data set */
        MDVS_Get_Info,         /* Gets data set info common to all fields */
	MDVS_Get_Volume,       /* Gets a raw volume of data */
	MDVS_Get_Plane,        /* Extracts a single plane of data */
	MDVS_Get_Point,        /* Extracts a single point of data */
	MDVS_Get_Chunk,        /* Retrieve a specific chunk of data */

        MDVS_Put_Dataset,      /* Puts data set */
        MDVS_Put_Info,         /* Put data set info common to all fields */
	MDVS_Put_Volume,       /* Put a raw volume of data */
	MDVS_Put_Plane,        /* Put a single plane of data */
	MDVS_Put_Point,        /* Put a single point of data */
	MDVS_Put_Chunk,        /* Store a specific chunk of data */

	MDVS_COMMAND_SPARE[242];      /* Enough room for 256 commands */

	MDVS_Notification,     /* Notification Reply */
	MDVS_Times_Reply,      /* Contains a list of data [generation] times */
	MDVS_Forecasts_Reply,  /* Contains a list of forecast times */
	MDVS_Info_Reply,       /* Is a MDVS_master_header_t struct */
	MDVS_Dataset_Reply,    /* Contains a MDVS_field_header_t and a data array */
	MDVS_Volume_Reply,     /* Contains a MDVS_field_header_t and a data array */
	MDVS_Plane_Reply,      /* Contains a abreviated MDVS_field_header_t and a data plane */
	MDVS_Point_Reply,      /* Contains a abreviated MDVS_field_header_t and a data point */
	MDVS_Chunk_Reply,      /* Contains externally defined, arbitrary data structs */
	MDVS_RT_Status_Reply,  /* Contains real time data access information */
	MDVS_Request_Failed    /* Contains info on why requests failed */
	MDVS_REPLY_SPARE[118]; /* enough spares for 128 replys */
};

/* This structure is sent before all messages and replies */
/* and gives an indication of the type and size of the associated message */

typedef struct {
    ui32  msg_id;                /* The Message ID */
    ui32  msg_size;              /* Size of the message in bytes */
    ui32  msg_serial;            /* A Serial number - used to match requests
				  and replies */
} MDVS_msg_head_t;

/*  MDVS_Get_Dataset, MDVS_Get_Volume, MDVS_Get_Plane, MDVS_Get_Point Requests */
 
enum Req_type { DATASET = 1, VOLUME,  HORIZ_PLANE , HORIZ_MAXIMA_PLANE,
                VERT_PLANE, SINGLE_POINT };
enum Req_mode { DEFAULT = 0, WANT_REALTIME , WANT_NEW, WANT_FORECAST,
                WANT_LATEST_FORECAST };
 
typedef struct {
    ui32    struct_id;
    si32    request_type;    /* The primary data request */
    si32    request_mode;    /* A secondary mode/modifier to the command */

    si32    field_code;      /* Which grib field we want */
    si32    plane_num;       /* Which plane we want */

    si32    time_min;        /* Data must be more recent than this time */
    si32    time_cent;       /* As close as possible to this time */
    si32    time_max;        /* But not newer than this time */
    si32    time_last;       /* Used for WANT_REALTIME, WANT_NEW modes - */
    si32    time_generated;  /* Used for WANT_FORECAST -
			      *Choose data generated closest to this time  */

    si32    ireserved[22];

    fl32  lat_min;
    fl32  lat_max;
    fl32  lon_min;
    fl32  lon_max;
    fl32  alt_min;
    fl32  alt_max;

    fl32  freserved[26];
}  MDVS_data_request_t;

/*  MDVS_Get_Volume Reply */
typedef struct {
    MDV_field_header_t fhead;
    void	*data;
} MDVS_data_reply_t;

/*  MDVS_Get_Plane,Point Replies */
typedef struct {
    MDV_field_header_t fhead; /* Minus 4*sizeof(int)*MAX_V_LEVELS (last 1536 bytes) */
    void	*data;
} MDVS_data_reply_t;



/* MDVS_Get_RT_Status Request */
typedef struct {
    si32    struct_id;
    si32    time_last;
}  MDVS_status_request_t;

/* MDVS_RT_Status Reply */
typedef struct {
    si32    struct_id;
    si32    time_updated;
    si08   status[256]; /* The Source's Realtime status - Human readible */
    si08   reason[256]; /* Why the source has this status */
} MDVS_status_reply_t;

/* MDVS_Get_Times Request */
typedef struct {
    si32    struct_id;
    si32    time_begin;
    si32    time_end;
}  MDVS_time_list_request_t;

/* MDVS_Times Reply */
typedef struct {
    si32    struct_id;
    si32    n_entries;   /* Number of entries in the list */
    si32    time[1];     /* Actually n_entries in size when sent */
} MDVS_time_list_msg_t;


/* MDVS_Get_Forecasts Request */
typedef struct {
    si32    struct_id;
    si32    time_begin;
    si32    time_end;
}  MDVS_fcast_list_request_t;

/* MDVS_Forecasts Reply */
typedef struct {
    si32    struct_id;
    si32    n_entries;   /* Number of entries in the list */
    si32    time[1];     /* Actually n_entries in size when sent */
} MDVS_fcast_list_msg_t;

/* MDVS_Get_Chunk Request */
typedef struct {
    si32    struct_id;
    si32    request_mode;    /* The primary data request */
    si32    chunk_id;        /* The Id of a chunk */
     
    si32    time_min;        /* Chunk must be more recent than this time */
    si32    time_cent;       /* As close as possible to this time */
    si32    time_max;        /* But not newer than this time */
    si32    time_last;       /* Used for WANT_REALTIME, WANT_NEW modes - */
}  MDVS_chunk_request_t;

/* MDVS_Chunk Reply */
/* Chunks have their own, arbitrary, externally defined structures */
 

/* MDVS_Request_Notification Request */
typedef struct {
    si32    struct_id;
    si32    port;
    si08   host[256];
}  MDVS_fcast_list_request_t;

/* MDVS_Notification Reply */
typedef struct {
    si32   struct_id;
    si32   port;
    si08   host[128];
    si08   type[128];
    si08   instance[128];

    si32    n_entries;   /* Number of entries in the list */
    si32    time[1];     /* Actually n_entries in size when sent */
} MDVS_fcast_list_msg_t;

# endif     /* MDATA_MSGS_H */


ifdef __cplusplus
}
#endif
