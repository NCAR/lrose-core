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
/* include file for history_forecast library */

#ifdef __cplusplus
 extern "C" {
#endif

/* system includes */
#include <toolsa/os_config.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <dataport/swap.h>
#include <dataport/bigend.h>
#include <dataport/port_types.h>
#include <toolsa/mem.h>

#ifndef TRUE 
#define TRUE 1
#endif
#ifndef FALSE 
#define FALSE 0
#endif

#define HF_STRSIZE 256
#define NUM_HF_ISPARE 3 /* # of integer spares in hf_file and hf_chunk*/
#define NUM_HF_FSPARE 5 /* # of float spares in hf_file and hf_chunk*/
#define NUM_HF_SPARE  3 /* # of float spares in hf_data */

#define SUCCESS 0
#define FAILURE -1


/* actual history/forecast data, if this changes be sure to update NUM_HFD_32 */
typedef struct {

   ui32 time;		     /* time of data (Unix time) */
   fl32 dbz;		     /* reflectivity in dbz at time */
   fl32 umotion;	     /* u-component of echo motion at time */
   fl32 vmotion;	     /* v-component of echo motion at time */
   fl32 dbzf;		     /* forecasted dbz at forecast_length time */
   fl32 spare[NUM_HF_SPARE]; /* spares */ 

} hf_data_t;

 
/* station information.  Contains history/forecast data. */

typedef struct {

   fl32 station_lon;			/* Station longitude */
   fl32 station_lat;			/* Station latitude */
   char station_name[HF_STRSIZE];	/* Station Name (no spaces) */
   hf_data_t *hf_data;			/* history/forecast data */

} sta_data_t;

/* history/forecast data for each instance of ctrec (each output image) */
/* if this struct changes be sure to update NUM_STA_32 */
typedef struct {

   ui32 time;			/* time forecast/file was made */
   si32 forecast_length;  	/* total time that forecasts made (secs) */
   si32 n_stations;		/* number of stations */
   si32 n_history;		/* number of history points for each station 
                                 * (must be the same for each station */
   si32 n_forecast;		/* number of forecast points for each station
                                 * (must be the same for each station */
   si32 ispare[NUM_HF_ISPARE];	/* spares */

   fl32 radar_lat;              /* latitude of radar in degrees */
   fl32 radar_lon;              /* longitude of radar in degrees */
   fl32 radar_alt;              /* elevation of radar in km above sea level */
   fl32 elev_angle;             /* elevation angle of radar (degrees) (-1. if N/A)*/
   fl32 elev_km;                /* elevation (km) above radar if projection 
                                 *        is flat (-1. if N/A) */
   fl32 ave_u;			/* u-component of motion for whole image */
   fl32 ave_v;			/* v-component of motion for whole image */
   fl32 fspare[NUM_HF_FSPARE];	/* spares */

   char instance[HF_STRSIZE];

   sta_data_t *sta_data;	/* station info (with history/forecast data) */

} hf_file_t;

/*********************************************************************/
/* chunk station information
 * If this changes be sure to update NUM_STA_32 */
typedef struct {

   fl32 station_lon;			/* Station longitude */
   fl32 station_lat;			/* Station latitude */
   char station_name[HF_STRSIZE];	/* Station Name (no spaces) */
   hf_data_t hf_data[1];		/* (n hist+fore points of these) */

} sta_chunk_t;


/* chunk file information
 * If this struct changes be sure to update NUM_HFF_32 */
typedef struct {
   ui32 time;			/* time forecast/file was made */
   si32 forecast_length;  	/* total time that forecasts made (secs) */
   si32 n_stations;		/* number of stations */
   si32 n_history;		/* number of history points for each station 
                                 * (must be the same for each station */
   si32 n_forecast;		/* number of forecast points for each station
                                 * (must be the same for each station */
   si32 ispare[NUM_HF_ISPARE];	/* spare */

   fl32 radar_lat;              /* latitude of radar in degrees */
   fl32 radar_lon;              /* longitude of radar in degrees */
   fl32 radar_alt;              /* elevation of radar in km above sea level */
   fl32 elev_angle;             /* elevation angle of radar (degrees) (-1. if N/A)*/
   fl32 elev_km;                /* elevation (km) above radar if projection 
                                 *        is flat (-1. if N/A) */
   fl32 ave_u;			/* u-component of ave motion for whole image */
   fl32 ave_v;			/* v-component of ave motion for whole image */
   fl32 fspare[NUM_HF_FSPARE];	/* spare */
   char instance[HF_STRSIZE];

   sta_chunk_t sta_chunk[1];	/* nstations of these which have h/f data in them */

} hf_chunk_t;


/* number of 32 bit elements at beginning of each structure */
/* used for swapping */
#define NUM_HFD_32  8 	/* hf_data_t */
#define NUM_STA_32  2 	/* sta_dat_t, sta_chunk_t */
#define NUM_HFF_32 20 	/* hf_file_t hf_chunk_t */

/**********************************************************************/

/* writes out a history/forecast spdb */
int write_hf_spdb(hf_file_t *hff);

/* writes out a history/forecast spdb */
int calloc_hf_struct(hf_file_t *hff, int nsta, int npts);

/* swap history/forecast info into big-endian */
int hf_chunk_bigend(hf_chunk_t **hfc);

/* swap history/forecast info into native */
int hf_chunk_native(hf_chunk_t **hfc);

/* puts an chunk block into a hf_file struct */
int hf2chunk(hf_file_t *hff, hf_chunk_t **hf_chunk, int *hf_chunk_size);

/* puts an chunk block into a hf_file struct */
int chunk2hf(hf_file_t *hff, hf_chunk_t *hfc, int hf_chunk_size);

/* puts an hf_chunk into native format */
int hf_chunk_native(hf_chunk_t **hfc);

/* prints history/forecast file struct */
void print_hf_file(FILE *stream, hf_file_t *hff);

/* prints history/forecast chunk */
void print_hf_chunk(FILE *stream, hf_chunk_t *hfc, int chunk_size);

#ifdef __cplusplus
}
#endif
