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

/*  station_reports.h  */

#ifndef STATION_REPORTS_H
#define STATION_REPORTS_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#include <dataport/port_types.h>
#include <rapformats/metar_decode.h>

#define STATION_NAN  262144.0 
#define WX_NAN  0xFFFFFFFF 
#define ST_LABEL_SIZE  8
#define STN_INDC_LEN 4
#define REM_INFO_SPARE_LEN 5

   /* maximum ceiling -- should match CLR, SKC, NSC, and CAVOK which is 12K ft; used to be 30 km */ 
#define MAX_CEILING_KM 3.6576

/* A Magic cookie to identify specific data structures */
/*
 * SENSOR_REPORT: does not use the shared union
 *
 * STATION_REPORT: uses shared.station
 *
 * STATION_REPORT_ARRAY: uses shared.statsion
 *
 * METAR_REPORT: uses shared.metar
 *
 * PRESSURE_STATION_REPORT: uses shared.pressure
 *
 * METAR_WITH_REMARKS_REPORT: uses shared.pressure_station
 *
 * REPORT_PLUS_METAR_XML:
 *   uses shared.station
 *   XML follows the station_report struct
 *
 * REPORT_PLUS_FULL_XML:
 *   uses shared.station
 *   XML follows the station_report struct
 */

enum msg_id_t { SENSOR_REQUEST = 17000,
		SENSOR_REPORT,
		STATION_REPORT,
		STATION_REPORT_ARRAY,
		METAR_REPORT,
                PRESSURE_STATION_REPORT,
		METAR_WITH_REMARKS_REPORT,
                REPORT_PLUS_METAR_XML,
                REPORT_PLUS_FULL_XML };

/* Enumerated Key values to indicate specific quanities */
enum sensor_type_t { TEMP = 18000, HUMIDITY,
		     WIND_SPEED, WIND_DIRECTION, PRESSURE,
                     LIQUID_ACCUMULATION, PRECITPITATION_RATE,
		     VISIBILITY, 
		     RVR, WEATHER_PHENOM, CEILING };
 
/* Weather Type Bitwize flags - Taken from NOAA/NWS METAR/SAO definitions */
#define WT_RA     0x00000001 /* Liquid precip - RAIN - not frozen */
#define WT_SN     0x00000002 /* Frozen precip - other than hail */
#define WT_UP     0x00000004 /* Unknown precip */
#define WT_FG     0x00000008 /* Fog */
#define WT_DS     0x00000010 /* Dust Storm */
#define WT_FZFG   0x00000020 /* Freezing FOG */
#define WT_BR     0x00000040 /* Mist */
#define WT_HZ     0x00000080 /* Haze */
#define WT_SQ     0x00000100 /* Squall */
#define WT_FC     0x00000200 /* Funnel cloud, tornado, water spout */
#define WT_TS     0x00000400 /* Thunderstorm */
#define WT_GR     0x00000800 /* Hail */
#define WT_GS     0x00001000 /* Small Hail - Deprecated */
#define WT_MFZDZ  0x00001000 /* Light FZDZ (-FZDZ) */
#define WT_FZRA   0x00002000 /* Freezing Rain */
#define WT_VA     0x00004000 /* Volcanic Ash */
#define WT_CLR    0x00008000 /* Clear Sky - Deprecated */
#define WT_FROST  0x00008000 /* Frost */
#define WT_SCT    0x00010000 /* Scattered Clouds */
#define WT_BKN    0x00020000 /* Broken Clouds */
#define WT_OVC    0x00040000 /* Overcast */
#define WT_FEW    0x00080000 /* Clouds 0/8 - 2/8 */
#define WT_PE     0x00100000 /* Ice Pellets */
#define WT_BLSN   0x00200000 /* Blowing Snow */
#define WT_FZDZ   0x00400000 /* Freezing Drizzle */
#define WT_DZ     0x00800000 /* Drizzle */
#define WT_MRA    0x01000000 /* Light RAIN -RA */
#define WT_PRA    0x02000000 /* Heavy RAIN +RA */
#define WT_MSN    0x04000000 /* Light SNOW -SN */
#define WT_PSN    0x08000000 /* Heavy SNOW +SN */
#define WT_PTS    0x10000000 /* Heavy Thunderstorm */
#define WT_MFZRA  0x20000000 /* Light Freezing Rain*/
#define WT_PFZRA  0x40000000 /* Heavy Freezing Rain*/
#define WT_SG     0x80000000 /* Snow Grains - Deprecated */
#define WT_PFZDZ  0x80000000 /* Heavy Freezing Drizzle (+FZDZ) */

/*
 * station_report_t
 * Readings from a single mesonet station.
 */

#define METAR_WX_STR_LEN 12

typedef struct {

  ui32 msg_id;           /* A unique message identifier -
			  * STATION_REPORT or METAR_REPORT */

  ui32 time;             /* time of report Seconds since Jan 1 0Z 1970  */

  ui32 accum_start_time; /* Time when liquid accumulation was reset to 0 */

  ui32 weather_type;     /* Weather Phenomonon - Bitwize Indicators  */

  fl32 lat;         /* degrees latitude of station/sensors */
  fl32 lon;         /* degrees longitude of station/sensors */
  fl32 alt;         /* meters altitude of station/sensors */
  fl32 temp;        /* temperature (deg C) */ 
  fl32 dew_point;   /* dew point - (deg C) */
  fl32 relhum;      /* relative humidity percent */
  fl32 windspd;     /* wind speed (m/s)   */
  fl32 winddir;     /* wind direction (deg)   */
  fl32 windgust;    /* max wind gusts (m/sec)   */
  fl32 pres;        /* barometric pressure (mb)  */
  fl32 liquid_accum;/* Liquid accumulation - since being reset mm  */
  fl32 precip_rate; /* precipitation rate - mm/hr */
  fl32 visibility;  /* Visibility distance - km */
  fl32 rvr;         /* Runway Visual range km */
  fl32 ceiling;     /* Weather/ Cloud Ceiling  km */

  /*
   * The following union allows for use of this struct for various purposes.
   * For msg_id == STATION_REPORT, use station.
   * For msg_id == METAR_REPORT, use metar.
   */

  union {
    struct {
      fl32 liquid_accum2; /* Liquid accum, second gauge -
			   * since being reset mm  */
      fl32 Spare1;    /* Spare Sensor 1 - Unknown units  */
      fl32 Spare2;      /* Spare Sensor 2 - Indefinite type and units */
      /* NOTE: Spare1 and Spare2 are actually used for data in the WSDDM */
      /*       system so cannot be changed. */
    } station;
    struct {
      si32 int1;
      si32 int2;
      fl32 float1;
    } user_data;
    struct {
      char weather_str[METAR_WX_STR_LEN];
    } metar;
    struct 
    {
      fl32 stn_pres;     /* station pressure in mb */
      fl32 Spare1;
      fl32 Spare2;
    } pressure_station;
    struct 
    {
      char stn_indicator[STN_INDC_LEN];
      ui08 pwi_no;
      ui08 fzra_no;
      ui08 ts_no;
      ui08 spare[REM_INFO_SPARE_LEN];
    } remark_info;
    
  } shared;

  char  station_label[ST_LABEL_SIZE];  /* A human readable label for
					* the stations site */

} station_report_t;

#define SENSOR_REPORT_NBYTES_32 88
#define STATION_REPORT_NBYTES_32 88
#define METAR_REPORT_NBYTES_32 76
#define PRESSURE_STATION_REPORT_NBYTES_32 88
#define METAR_WITH_REMARKS_REPORT_NBYTES_32 76

// LWE intensity
#define LWE_NONE 0
#define LWE_VLIGHT 1
#define LWE_LIGHT 2
#define LWE_MODERATE 3
#define LWE_HEAVY 4

// LWE wx types
#define LWE_NOPRECIP 0
#define LWE_RAIN 1
#define LWE_SNOW 2
#define LWE_UNKNOWN 3
#define LWE_FZRA 4
#define LWE_FZDZ 5
#define LWE_DRIZZLE 6
#define LWE_FROST 7
#define LWE_FZ_FOG 8
#define LWE_MIST 9
#define LWE_FOG 10
#define LWE_PELLETS 11
#define LWE_ACTIVE_FROST 12

#define LWE_ERROR -1

typedef struct {
    ui32 msg_id; /* A unique message identifier - STATION_REPORT_ARRAY */
    ui32 num_reports;   
} station_report_array_header_t;
 

/* For printing METAR data, different output styles */

typedef enum {
  STYLE_WSDDM = 0,
  STYLE_AOAWS = 1
} metar_select_out_style_t;


/******************************************************************************
 * STATION_REPORT_TO_BE  Swap the data in a station report into big-endian
 *                       format.  This is done in preparation for sending
 *                       a station report across the network or for writing
 *                       a station report to disk.
 */
extern void station_report_to_be(station_report_t *report);

/******************************************************************************
 * STATION_REPORT_FROM_BE  Swap the data in a station report from big-endian
 *                         format into native format.  This must be done
 *                         after receiving a report across the network or
 *                         reading a station report from disk.
 */
void station_report_from_be(station_report_t *report);

/******************************************************************************
 * PRINT_STATION_REPORT  Print the station report to the given stream.  The
 *                       prefix_string is prepended to each output line so
 *                       you can indent the output, if desired.
 */
void print_station_report(FILE *stream, const char *prefix_string,
			  const station_report_t *report);


/******************************************************************************
 *  SORT_STATIONS_BY_DISTANCE: Sort the given array of station reports based
 *  on their distances from the given location.  Reports from the same location
 *  are sorted by time, the most recent report coming first.
 */
void sort_stations_by_distance(station_report_t *station_list,
			       const int num_stations,
			       const double location_lat,
			       const double location_lon);

/******************************************************************************
 * STATION_REPORT_ARRAY_TO_BE  Swap the data in a station report_array_header
 *    and station data array into big-endian format.
 *
 * This is done in preparation for sending
 * a station report across the network or for writing
 * a station report to disk.
 */
extern void station_report_array_to_be(station_report_array_header_t *head,
				       station_report_t report[]);

/******************************************************************************
 * STATION_REPORT_ARRAY_FROM_BE
 * Swap the data in a station report_array_header 
 * and station data array from big-endian format into native format.
 *
 * This must be done
 * after receiving a report across the network or
 * reading a station report from disk.
 */

void station_report_array_from_be(station_report_array_header_t *head, station_report_t report[]);

/******************************************************************************
 * PRINT_STATION_REPORT_ARRAY  Print the station report to the given stream.  The
 *                       prefix_string is prepended to each output line so
 *                       you can indent the output, if desired.
 */
void print_station_report_array(FILE *stream, const char *prefix_string,
				const station_report_array_header_t *head,
				const station_report_t report[]);

/******************************************************************************
 * WEATHER_TYPE2STRING: Returns a pointer to an ascii string with all the
 *                       Weather type Acrynonyms in it.
 */
char *weather_type2string(const si32 weather_type); 

/******************************************************************************
 *  WEATHER_TYPE2STRING_TRUNC
 *
 *  return a pointer to a static string contining the truncated
 *  Mnemonic/ Acronym Weather type labels - Null terminated
 *
 *  Truncated at n_types types - the function is ordered to
 *  add the most significant weather types at the beginning of the list.
 */
char *weather_type2string_trunc(const si32 weather_type, const int n_types);

/******************************************************************************
 * STRING2WEATHER_TYPE: Returns the weather type bit mask from an ascii string
 *                      with all Weather type Acrynonyms in it.
 */
ui32 string2weather_type(const char *weather_string); 

/******************************************************************************
 * COMPUTE_RELHUM: Return Relativre humidity - Percent
 * Input temperature and dew_point in Degrees C
 */
double compute_relhum(const double temper, const double dew_temper);

/*************************************************************************
 * E_SUB_S:  Return saturation vapor pressure (Pa) over liquid using
 *       polynomial fit of goff-gratch (1946) formulation. (walko, 1991) 
 */
double e_sub_s(const double deg_c);

/*************************************************************************
 * HUMID_TEMP2DEW: Determine Dewpoint given % humidity and temperature C
 */
double humid_temp2dew(const double rel_humid, const double temp_C);

/*************************************************************************
 * PRESS_MIXR2DEW : Determine Dewpoint given pressure and mixing ratio
 */
double press_mixr2dew (const double press_Pa, const double mix_r);


/*************************************************************************
 * PRINT_WSDDM : Print a list of METARS in *metar_table to stdout in
 *               WSDDM-format.
 */
extern void print_wsddm(const station_report_t *metar_table,
			const int num_metars);

/*************************************************************************
 * PRINT_AOAWS : Print a list of METARS in *metar_table to stdout in
 *               AOAWS-format.
 */
extern void print_aoaws(const station_report_t *metar_table,
			const int num_metars);

/*************************************************************************
 * PRINT_WSDDM : Print a list of METARS in *metar_table to stdout in
 *               the input-format. Calls either print_wsddm or print_aoaws.
 */
extern void print_metar_data(const station_report_t *metar_table,
			     const int num_metars,
			     const metar_select_out_style_t out_style);

/******************************************************************************
 * DECODED_METAR_TO_GENERIC_REPORT
 *
 * Fill out a generic report from a decoded metar struct.  This means that
 * only the non-union parts of the station report are filled in by this
 * function.  Also, the msg_id field is left alone.
 *
 * Inputs:
 *   dcdMetar: decoded metar struct
 *   valid_time: time for METAR obs
 *   lat, lon, alt:: position of metar station
 *
 * Output:
 *   report: Filled out
 *
 * If realtime is TRUE, correct time of in the future.
 *
 * Returns 0 on success, -1 on failuer.
 */

extern int decoded_metar_to_generic_report(const Decoded_METAR *dcdMetar,
					   station_report_t *report,
					   time_t valid_time,
					   double lat, double lon, double alt);
   
/******************************************************************************
 * DECODED_METAR_TO_REPORT
 *
 * Fill out a report from a decoded metar struct.
 *
 * Inputs:
 *   dcdMetar: decoded metar struct
 *   valid_time: time for METAR obs
 *   lat, lon, alt:: position of metar station
 *
 * Output:
 *   report: Filled out
 *
 * If realtime is TRUE, correct time of in the future.
 *
 * Returns 0 on success, -1 on failuer.
 */

extern int decoded_metar_to_report(const Decoded_METAR *dcdMetar,
				   station_report_t *report,
				   time_t valid_time,
				   double lat, double lon, double alt);

/******************************************************************************
 * DECODED_METAR_TO_REPORT_WITH_REMARKS
 *
 * Fill out a METAR_WITH_REMARKS_REPORT style report from a decoded metar 
 * struct.
 *
 * Inputs:
 *   dcdMetar: decoded metar struct
 *   valid_time: time for METAR obs
 *   lat, lon, alt:: position of metar station
 *
 * Output:
 *   report: Filled out
 *
 * If realtime is TRUE, correct time of in the future.
 *
 * Returns 0 on success, -1 on failuer.
 */

extern int decoded_metar_to_report_with_remarks(const Decoded_METAR *dcdMetar,
						station_report_t *report,
						time_t valid_time,
						double lat, double lon, double alt);

/******************************************************************************
 * DECODED_METAR_TO_STATION_REPORT
 *
 * Fill out a STATION_REPORT style report from a decoded metar struct.
 *
 * Inputs:
 *   dcdMetar: decoded metar struct
 *   valid_time: time for METAR obs
 *   lat, lon, alt:: position of metar station
 *
 * Output:
 *   report: Filled out
 *
 * If realtime is TRUE, correct time of in the future.
 *
 * Returns 0 on success, -1 on failuer.
 */

extern int decoded_metar_to_station_report(const Decoded_METAR *dcdMetar,
					   station_report_t *report,
					   time_t valid_time,
					   double lat, double lon, double alt);
   

/******************************************************************************
 * DECODED_METAR_TO_PRESSURE_STATION_REPORT
 *
 * Fill out a PRESSURE_STATION_REPORT style report from a decoded metar struct.
 *
 * Inputs:
 *   dcdMetar: decoded metar struct
 *   valid_time: time for METAR obs
 *   lat, lon, alt:: position of metar station
 *
 * Output:
 *   report: Filled out
 *
 * If realtime is TRUE, correct time of in the future.
 *
 * Returns 0 on success, -1 on failuer.
 */

extern int decoded_metar_to_pressure_station_report(const Decoded_METAR *dcdMetar,
						    station_report_t *report,
						    time_t valid_time,
						    double lat, double lon, double alt);
   

#endif
 
#ifdef __cplusplus
}
#endif


