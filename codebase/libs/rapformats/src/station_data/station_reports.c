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
/*************************************************************************
 * STATION_REPORTS.c: Interface routines for the STATION REPORT SERVICES
 *
 *  Originally authored By W. Adams, RAP Jan 1994.
 *  Reworked by F. Hage. RAP April 1994.
 *  New structures/names - More generalized - Fhage 10/94
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <dataport/bigend.h>
#include <rapformats/station_reports.h>

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>

/*
 * Define values used by qsort in sort_stations_by_distance().
 */

double Distance_sort_lat = 0.0;
double Distance_sort_lon = 0.0;

/*
 * Prototypes for static functions.
 */

static int compare_distance(const void *element1,
			    const void *element2);


/******************************************************************************
 * STATION_REPORT_TO_BE  Swap the data in a station report into big-endian
 *                       format.  This is done in preparation for sending
 *                       a station report across the network or for writing
 *                       a station report to disk.
 */

void station_report_to_be(station_report_t *report)
{
  switch (report->msg_id)
  {
  case SENSOR_REQUEST :
  case SENSOR_REPORT :
    BE_from_array_32(report, SENSOR_REPORT_NBYTES_32);
    break;
    
  case STATION_REPORT :
  case STATION_REPORT_ARRAY :
    BE_from_array_32(report, STATION_REPORT_NBYTES_32);
    break;
    
  case METAR_REPORT :
  case REPORT_PLUS_METAR_XML:
  case REPORT_PLUS_FULL_XML:
    BE_from_array_32(report, METAR_REPORT_NBYTES_32);
    break;

  case PRESSURE_STATION_REPORT :
    BE_from_array_32(report, PRESSURE_STATION_REPORT_NBYTES_32);
    break;

  case METAR_WITH_REMARKS_REPORT :
    BE_from_array_32(report, METAR_WITH_REMARKS_REPORT_NBYTES_32);
    break;
  }

} /* station_report_to_be() */

/******************************************************************************
 * STATION_REPORT_FROM_BE  Swap the data in a station report from big-endian
 *                         format into native format.  This must be done
 *                         after receiving a report across the network or
 *                         reading a station report from disk.
 */

void station_report_from_be(station_report_t *report)
{

  si32 msg_id;
  memcpy(&msg_id, &report->msg_id, sizeof(si32));
  
  if (msg_id >= 17000 && msg_id <= 17020) {
    /* already swapped */
    return;
  }

  msg_id = BE_to_si32(msg_id);

  switch (msg_id)
  {
  case SENSOR_REQUEST :
  case SENSOR_REPORT :
    BE_to_array_32(report, SENSOR_REPORT_NBYTES_32);
    break;

  case STATION_REPORT :
  case STATION_REPORT_ARRAY :
    BE_to_array_32(report, STATION_REPORT_NBYTES_32);
    break;

  case METAR_REPORT :
  case REPORT_PLUS_METAR_XML:
  case REPORT_PLUS_FULL_XML:
    BE_to_array_32(report, METAR_REPORT_NBYTES_32);
    break;
    
  case PRESSURE_STATION_REPORT :
    BE_to_array_32(report, PRESSURE_STATION_REPORT_NBYTES_32);
    break;
    
  case METAR_WITH_REMARKS_REPORT :
    BE_to_array_32(report, METAR_WITH_REMARKS_REPORT_NBYTES_32);
    break;

  default:
    BE_to_array_32(report, SENSOR_REPORT_NBYTES_32);
      
  }

} /* station_report_from_be() */

/******************************************************************************
 * PRINT_STATION_REPORT  Print the station report to the given stream.  The
 *                       prefix_string is prepended to each output line so
 *                       you can indent the output, if desired.
 */

void print_station_report(FILE *stream, const char *prefix_string,
			  const station_report_t *report)
{
  time_t print_time;
  
  fprintf(stream, "%smsg_id = %d\n", prefix_string, report->msg_id);

  print_time = report->time;
  fprintf(stream, "%stime = %s", prefix_string, asctime(gmtime(&print_time)));
  
  print_time = report->accum_start_time;
  fprintf(stream, "%saccum_start_time = %s",
	  prefix_string, asctime(gmtime(&print_time)));
  
  fprintf(stream, "%sweather_type = %s\n",
	  prefix_string, weather_type2string(report->weather_type));
  
  if(report->lat != STATION_NAN) {
      fprintf(stream, "%slat = %.3f deg\n", prefix_string, report->lat);
  } else {
      fprintf(stream, "%slat = NOT SET\n", prefix_string);
  }

  if(report->lon != STATION_NAN) {
      fprintf(stream, "%slon = %.3f deg\n", prefix_string, report->lon);
  } else {
      fprintf(stream, "%slon = NOT SET\n", prefix_string);
  }

  if(report->alt != STATION_NAN) {
      fprintf(stream, "%salt = %.3f m\n", prefix_string, report->alt);
  } else {
      fprintf(stream, "%salt = NOT SET\n", prefix_string);
  }

  if(report->temp != STATION_NAN) {
      fprintf(stream, "%stemp = %.3f deg C\n", prefix_string, report->temp);
  } else {
      fprintf(stream, "%stemp = NOT SET\n", prefix_string);
  }

  if(report->dew_point != STATION_NAN) {
      fprintf(stream, "%sdew_point = %.3f deg C\n", prefix_string, report->dew_point);
  } else {
      fprintf(stream, "%sdew_point = NOT SET\n", prefix_string);
  }

  if(report->relhum != STATION_NAN) {
      fprintf(stream, "%srelhum = %.3f %%\n", prefix_string, report->relhum);
  } else {
      fprintf(stream, "%srelhum = NOT SET\n", prefix_string);
  }

  if(report->windspd != STATION_NAN) {
      fprintf(stream, "%swindspd = %.3f m/s\n", prefix_string, report->windspd);
  } else {
      fprintf(stream, "%swindspd = NOT SET\n", prefix_string);
  }

  if(report->winddir != STATION_NAN) {
    if (report->winddir == -0.0) {
      fprintf(stream, "%swinddir = variable\n", prefix_string);
    } else {
      fprintf(stream, "%swinddir = %.3f deg\n", prefix_string, report->winddir);
    }
  } else {
      fprintf(stream, "%swinddir = NOT SET\n", prefix_string);
  }

  if(report->windgust != STATION_NAN) {
      fprintf(stream, "%swindgust = %.3f m/s\n", prefix_string, report->windgust);
  } else {
      fprintf(stream, "%swindgust = NOT SET\n", prefix_string);
  }

  if(report->pres != STATION_NAN) {
      fprintf(stream, "%spres = %.3f mb\n", prefix_string, report->pres);
  } else {
      fprintf(stream, "%spres = NOT SET\n", prefix_string);
  }

  if(report->liquid_accum != STATION_NAN) {
      fprintf(stream, "%sliquid_accum = %.3f mm\n",
	  prefix_string, report->liquid_accum);
  } else {
      fprintf(stream, "%sliquid_accum = NOT SET\n", prefix_string);
  }

  if(report->precip_rate != STATION_NAN) {
      fprintf(stream, "%sprecip_rate = %.3f mm/hr\n", 
	  prefix_string, report->precip_rate);
  } else {
      fprintf(stream, "%sprecip_rate = NOT SET\n", prefix_string);
  }

  if(report->visibility != STATION_NAN) {
    if (report->visibility >= 0) {
      fprintf(stream, "%svisibility = %.3f km\n", prefix_string, report->visibility);
    } else {
      fprintf(stream, "%sExtinction coeff: = %.3f /km\n", prefix_string, report->visibility * -1.0);
    }
  } else {
      fprintf(stream, "%svisibility = NOT SET\n", prefix_string);
  }
   
  if(report->rvr != STATION_NAN) {
      fprintf(stream, "%srvr = %.3f km\n", prefix_string, report->rvr);
  } else {
      fprintf(stream, "%srvr = NOT SET\n", prefix_string);
  }
   
  if(report->ceiling != STATION_NAN) {
      fprintf(stream, "%sceiling = %.3f km\n", prefix_string, report->ceiling);
  } else {
      fprintf(stream, "%sceiling = NOT SET\n", prefix_string);
  }

  switch (report->msg_id)
  {
  case SENSOR_REQUEST :
  case SENSOR_REPORT :
    /* do nothing */
    break;
    
  case STATION_REPORT :
  case STATION_REPORT_ARRAY :
    if(report->shared.station.liquid_accum2 != STATION_NAN) {
      fprintf(stream, "%sliquid_accum2 = %.3f mm\n",
	      prefix_string, report->shared.station.liquid_accum2);
    } else {
      fprintf(stream, "%sliquid_accum2 = NOT SET\n", prefix_string);
    }
	   
    if(report->shared.station.Spare1 != STATION_NAN) {
      fprintf(stream, "%sSpare1 = %.3f \n",
	      prefix_string, report->shared.station.Spare1);
    } else {
      fprintf(stream, "%sSpare1 = NOT SET\n", prefix_string);
    }
    
    if(report->shared.station.Spare2 != STATION_NAN) {
      fprintf(stream, "%sSpare2 = %.3f \n",
	      prefix_string, report->shared.station.Spare2);
    } else {
      fprintf(stream, "%sSpare2 = NOT SET\n", prefix_string);
    }
    break;

  case METAR_REPORT :
  case REPORT_PLUS_METAR_XML:
  case REPORT_PLUS_FULL_XML:
    fprintf(stream, "%sMetar weather = <%s>\n",
	    prefix_string, report->shared.metar.weather_str);
    break;
    
  case PRESSURE_STATION_REPORT :
    if(report->shared.pressure_station.stn_pres != STATION_NAN) {
      fprintf(stream, "%sstn_pres = %.3f mb\n",
	      prefix_string, report->shared.pressure_station.stn_pres);
    } else {
      fprintf(stream, "%sstn_pres = NOT SET\n", prefix_string);
    }
	   
    if(report->shared.pressure_station.Spare1 != STATION_NAN) {
      fprintf(stream, "%sSpare1 = %.3f \n",
	      prefix_string, report->shared.pressure_station.Spare1);
    } else {
      fprintf(stream, "%sSpare1 = NOT SET\n", prefix_string);
    }
    
    if(report->shared.pressure_station.Spare2 != STATION_NAN) {
      fprintf(stream, "%sSpare2 = %.3f \n",
	      prefix_string, report->shared.pressure_station.Spare2);
    } else {
      fprintf(stream, "%sSpare2 = NOT SET\n", prefix_string);
    }
    break;
  case METAR_WITH_REMARKS_REPORT :
    fprintf(stream, "%sStation indicator = <%s>\n",
	    prefix_string, report->shared.remark_info.stn_indicator);
    fprintf(stream, "%spwi_no = <%d>\n",
	    prefix_string, report->shared.remark_info.pwi_no);
    fprintf(stream, "%sfzra_no = <%d>\n",
	    prefix_string, report->shared.remark_info.fzra_no);
    fprintf(stream, "%sts_no = <%d>\n",
	    prefix_string, report->shared.remark_info.ts_no);
    break;
    
    
  }

  fprintf(stream, "%sstation_label = <%s>\n",
	  prefix_string, report->station_label);
  
} /* print_station_report() */


/******************************************************************************
 *  SORT_STATIONS_BY_DISTANCE: Sort the given array of station reports based
 *  on their distances from the given location.  Reports from the same location
 *  are sorted by time, the most recent report coming first.
 */

void sort_stations_by_distance(station_report_t *station_list,
			       const int num_stations,
			       const double location_lat,
			       const double location_lon)
{
  /*
   * Set the static variables so they can be used within
   * the qsort routine.
   */

  Distance_sort_lat = location_lat;
  Distance_sort_lon = location_lon;
  
  /*
   * Now perform the sort
   */

  qsort((char *)station_list,
	num_stations,
	sizeof(station_report_t),
	compare_distance);
  
  return;
}


#define WEATHER_STRING_LEN 1024

/******************************************************************************
 *  WEATHER_TYPE2STRING: return a pointer to a static string contining the
 *  Mnemonic/ Acronym Weather type labels - Null terminated
 */

char *weather_type2string(const si32 weather_type)
{
  static char return_string[WEATHER_STRING_LEN];
  
  return_string[0] = '\0';
  
  if (weather_type == WX_NAN) {
    STRconcat(return_string, "NOT SET\0", WEATHER_STRING_LEN);
    return(return_string);
  }

  if (weather_type & WT_RA)
    STRconcat(return_string, "RA ", WEATHER_STRING_LEN);
  if (weather_type & WT_SN)
    STRconcat(return_string, "SN ", WEATHER_STRING_LEN);
  if (weather_type & WT_UP)
    STRconcat(return_string, "UP ", WEATHER_STRING_LEN);
  if (weather_type & WT_FG)
    STRconcat(return_string, "FG ", WEATHER_STRING_LEN);
  if (weather_type & WT_DS)
    STRconcat(return_string, "DS ", WEATHER_STRING_LEN);
  if (weather_type & WT_FZFG)
    STRconcat(return_string, "FZFG ", WEATHER_STRING_LEN);
  if (weather_type & WT_BR)
    STRconcat(return_string, "BR ", WEATHER_STRING_LEN);
  if (weather_type & WT_HZ)
    STRconcat(return_string, "HZ ", WEATHER_STRING_LEN);
  if (weather_type & WT_SQ)
    STRconcat(return_string, "SQ ", WEATHER_STRING_LEN);
  if (weather_type & WT_FC)
    STRconcat(return_string, "FC ", WEATHER_STRING_LEN);
  if (weather_type & WT_TS)
    STRconcat(return_string, "TS ", WEATHER_STRING_LEN);
  if (weather_type & WT_GR)
    STRconcat(return_string, "GR ", WEATHER_STRING_LEN);
  if (weather_type & WT_PFZDZ)
    STRconcat(return_string, "+FZDZ ", WEATHER_STRING_LEN);
  if (weather_type & WT_MFZDZ)
    STRconcat(return_string, "-FZDZ ", WEATHER_STRING_LEN);
  if (weather_type & WT_MFZRA)
    STRconcat(return_string, "-FZRA ", WEATHER_STRING_LEN);
  if (weather_type & WT_FZRA)
    STRconcat(return_string, "FZRA ", WEATHER_STRING_LEN);
  if (weather_type & WT_PFZRA)
    STRconcat(return_string, "+FZRA ", WEATHER_STRING_LEN);
  if (weather_type & WT_VA)
    STRconcat(return_string, "VA ", WEATHER_STRING_LEN);
  if (weather_type & WT_FROST)
    STRconcat(return_string, "FROST ", WEATHER_STRING_LEN);
  if (weather_type & WT_SCT)
    STRconcat(return_string, "SCT ", WEATHER_STRING_LEN);
  if (weather_type & WT_BKN)
    STRconcat(return_string, "BKN ", WEATHER_STRING_LEN);
  if (weather_type & WT_OVC)
    STRconcat(return_string, "OVC ", WEATHER_STRING_LEN);
  if (weather_type & WT_FEW)
    STRconcat(return_string, "FEW ", WEATHER_STRING_LEN);
  if (weather_type & WT_PE)
    STRconcat(return_string, "PL ", WEATHER_STRING_LEN);
  if (weather_type & WT_BLSN)
    STRconcat(return_string, "BLSN ", WEATHER_STRING_LEN);
  if (weather_type & WT_FZDZ)
    STRconcat(return_string, "FZDZ ", WEATHER_STRING_LEN);
  if (weather_type & WT_DZ)
    STRconcat(return_string, "DZ ", WEATHER_STRING_LEN);
  if (weather_type & WT_MRA)
    STRconcat(return_string, "-RA ", WEATHER_STRING_LEN);
  if (weather_type & WT_PRA)
    STRconcat(return_string, "+RA ", WEATHER_STRING_LEN);
  if (weather_type & WT_MSN)
    STRconcat(return_string, "-SN ", WEATHER_STRING_LEN);
  if (weather_type & WT_PSN)
    STRconcat(return_string, "+SN ", WEATHER_STRING_LEN);
  if (weather_type & WT_PTS)
    STRconcat(return_string, "+TS ", WEATHER_STRING_LEN);

  if (return_string[0] != '\0')
    return_string[strlen(return_string)-1] = '\0';
  
  return(return_string);
}

/******************************************************************************
 *  WEATHER_TYPE2STRING_TRUNC
 *
 *  return a pointer to a static string contining the truncated
 *  Mnemonic/ Acronym Weather type labels - Null terminated
 *
 *  Truncated at n_types types - the function is ordered to
 *  add the most significant weather types at the beginning of the list.
 */

char *weather_type2string_trunc(const si32 weather_type, const int n_types)
{
  static char return_string[WEATHER_STRING_LEN];
  
  int count = 0;
  
  return_string[0] = '\0';
  
  if (weather_type == WX_NAN) {
    STRconcat(return_string, "NOT SET", WEATHER_STRING_LEN);
    return(return_string);
  }
  
  if (weather_type & WT_FC) {
    STRconcat(return_string, "FC ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_PFZRA) {
    STRconcat(return_string, "+FZRA ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_FZRA) {
    STRconcat(return_string, "FZRA ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_MFZRA) {
    STRconcat(return_string, "-FZRA ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_GR) {
    STRconcat(return_string, "GR ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_MFZDZ) {
    STRconcat(return_string, "-FZDZ ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_PE) {
    STRconcat(return_string, "PL ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_PSN) {
    STRconcat(return_string, "+SN ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_FZDZ) {
    STRconcat(return_string, "FZDZ ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_PTS) {
    STRconcat(return_string, "+TS ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_PRA) {
    STRconcat(return_string, "+RA ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_SN) {
    STRconcat(return_string, "SN ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_PFZDZ) {
    STRconcat(return_string, "+FZDZ ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_SQ) {
    STRconcat(return_string, "SQ ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_FZFG) {
    STRconcat(return_string, "FZFG ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_BLSN) {
    STRconcat(return_string, "BLSN ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_MSN) {
    STRconcat(return_string, "-SN ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_TS) {
    STRconcat(return_string, "TS ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_RA) {
    STRconcat(return_string, "RA ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_MRA) {
    STRconcat(return_string, "-RA ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_DS) {
    STRconcat(return_string, "DS ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_DZ) {
    STRconcat(return_string, "DZ ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_UP) {
    STRconcat(return_string, "UP ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_HZ) {
    STRconcat(return_string, "HZ ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_FG) {
    STRconcat(return_string, "FG ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_BR) {
    STRconcat(return_string, "BR ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_VA) {
    STRconcat(return_string, "VA ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_FROST) {
    STRconcat(return_string, "FROST ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_SCT) {
    STRconcat(return_string, "SCT ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_BKN) {
    STRconcat(return_string, "BKN ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_OVC) {
    STRconcat(return_string, "OVC ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  if (weather_type & WT_FEW) {
    STRconcat(return_string, "FEW ", WEATHER_STRING_LEN);
    count++;
    if (count >= n_types) {
      return (return_string);
    }
  }

  return(return_string);

}

/******************************************************************************
 * STRING2WEATHER_TYPE: Returns the weather type bit mask from an ascii string
 *                      with all Weather type Acrynonyms in it.
 */

#define MAX_TOKENS       100
#define MAX_TOKEN_LEN     80

ui32 string2weather_type(const char *weather_string)
{
  static char *tokens[MAX_TOKENS];
  static int first_time = TRUE;

  int i;
  int num_tokens;

  ui32 weather_mask = 0;

  if (first_time)
  {
    for (i = 0; i < MAX_TOKENS; i++)
      tokens[i] = (char *)umalloc(MAX_TOKEN_LEN);

    first_time = FALSE;
  }

  num_tokens = STRparse(weather_string, tokens, strlen(weather_string),
			MAX_TOKENS, MAX_TOKEN_LEN);

  for (i = 0; i < num_tokens; i++)
  {
    if (STRequal_exact(tokens[i], "RA"))
      weather_mask |= WT_RA;
    else if (STRequal_exact(tokens[i], "SN"))
      weather_mask |= WT_SN;
    else if (STRequal_exact(tokens[i], "UP"))
      weather_mask |= WT_UP;
    else if (STRequal_exact(tokens[i], "FG"))
      weather_mask |= WT_FG;
    else if (STRequal_exact(tokens[i], "DS"))
      weather_mask |= WT_DS;
    else if (STRequal_exact(tokens[i], "FZFG"))
      weather_mask |= WT_FZFG;
    else if (STRequal_exact(tokens[i], "BR"))
      weather_mask |= WT_BR;
    else if (STRequal_exact(tokens[i], "HZ"))
      weather_mask |= WT_HZ;
    else if (STRequal_exact(tokens[i], "SQ"))
      weather_mask |= WT_SQ;
    else if (STRequal_exact(tokens[i], "FC"))
      weather_mask |= WT_FC;
    else if (STRequal_exact(tokens[i], "TS"))
      weather_mask |= WT_TS;
    else if (STRequal_exact(tokens[i], "GR"))
      weather_mask |= WT_GR;
    else if (STRequal_exact(tokens[i], "+FZDZ"))
      weather_mask |= WT_PFZDZ;
    else if (STRequal_exact(tokens[i], "-FZDZ"))
      weather_mask |= WT_MFZDZ;
    else if (STRequal_exact(tokens[i], "-FZRA"))
      weather_mask |= WT_MFZRA;
    else if (STRequal_exact(tokens[i], "FZRA"))
      weather_mask |= WT_FZRA;
    else if (STRequal_exact(tokens[i], "+FZRA"))
      weather_mask |= WT_PFZRA;
    else if (STRequal_exact(tokens[i], "VA"))
      weather_mask |= WT_VA;
    else if (STRequal_exact(tokens[i], "FROST"))
      weather_mask |= WT_FROST;
    else if (STRequal_exact(tokens[i], "SCT"))
      weather_mask |= WT_SCT;
    else if (STRequal_exact(tokens[i], "BKN"))
      weather_mask |= WT_BKN;
    else if (STRequal_exact(tokens[i], "OVC"))
      weather_mask |= WT_OVC;
    else if (STRequal_exact(tokens[i], "FEW"))
      weather_mask |= WT_FEW;
    else if (STRequal_exact(tokens[i], "PL"))
      weather_mask |= WT_PE;
    else if (STRequal_exact(tokens[i], "PE"))
      weather_mask |= WT_PE;
    else if (STRequal_exact(tokens[i], "BLSN"))
      weather_mask |= WT_BLSN;
    else if (STRequal_exact(tokens[i], "FZDZ"))
      weather_mask |= WT_FZDZ;
    else if (STRequal_exact(tokens[i], "DZ"))
      weather_mask |= WT_DZ;
    else if (STRequal_exact(tokens[i], "-RA"))
      weather_mask |= WT_MRA;
    else if (STRequal_exact(tokens[i], "+RA"))
      weather_mask |= WT_PRA;
    else if (STRequal_exact(tokens[i], "-SN"))
      weather_mask |= WT_MSN;
    else if (STRequal_exact(tokens[i], "+SN"))
      weather_mask |= WT_PSN;
    else if (STRequal_exact(tokens[i], "+TS"))
      weather_mask |= WT_PTS;
  }

  return(weather_mask);
}

/******************************************************************************
 * STATION_REPORT_ARRAY_TO_BE  Swap the data in a station report array into big-endian
 *            format.  This is done in preparation for sending
 *            a station report across the network or for writing
 *            a station report to disk.
 */

void station_report_array_to_be(station_report_array_header_t *head, station_report_t report[])
{
  int i;
  int num_reps;

  num_reps = head->num_reports;

  BE_from_array_32((void *)head,sizeof(station_report_array_header_t)); /* convert the header info */

  for(i=0;i<num_reps;i++) { /* Now convert each individual report */
      station_report_to_be(&(report[i]));
  }

} /* station_report_array_to_be() */

/******************************************************************************
 * STATION_REPORT_ARRAY_FROM_BE  Swap the data in a station report array from big-endian
 *                         format into native format.  This must be done
 *                         after receiving a report across the network or
 *                         reading a station report from disk.
 */

void station_report_array_from_be(station_report_array_header_t *head, station_report_t report[])
{
  int i;
  int num_reps;

  num_reps = head->num_reports;

  BE_to_array_32((void *)head,sizeof(station_report_array_header_t)); /* convert the header info */

  for(i=0;i<num_reps;i++) { /* Now convert each individual report */
      station_report_from_be(&(report[i]));
  }

} /* station_report_array_from_be() */

/******************************************************************************
 * PRINT_STATION_REPORT_ARRAY  Print the station reports to the given stream.  The
 *                       prefix_string is prepended to each output line so
 *                       you can indent the output, if desired.
 */

void print_station_report_array(FILE *stream, const char *prefix_string,
				const station_report_array_header_t *head,
				const station_report_t report[])
{
  int i;

  for(i=0;i<head->num_reports;i++) { /* Print each individual report */
      print_station_report(stream,prefix_string,&(report[i]));
  }
}


/*************************************************************************
 * E_SUB_S:  Return saturation vapor pressure (Pa) over liquid using
 *       polynomial fit of goff-gratch (1946) formulation. (walko, 1991)
 *  C by F. Hage from G. Thompson's FORTRAN. - 1995.
 */
#define C0 610.5851
#define C1 44.40316
#define C2 1.430341
#define C3 .2641412e-1
#define C4 .2995057e-3
#define C5 .2031998e-5
#define C6 .6936113e-8
#define C7 .2564861e-11
#define C8 -.3704404e-13
                        
double e_sub_s(const double deg_c)
{
    double t;
    double e_sub_s;

    t = (deg_c < -80.0)? -80.0: deg_c;
    e_sub_s = C0 + t*(C1 + t*(C2 + t*(C3 + t*(C4 + t*(C5 + t*(C6 + t*(C7 + t*C8)))))));
    return e_sub_s;
}  

/*************************************************************************
 * PRESS_MIXR2DEW : Determine Dewpoint given pressure and mixing ratio
 */

double press_mixr2dew (const double press_Pa, const double mix_r)
{
    double rr,es,esln;

    rr = mix_r + 1e-8;
    es = press_Pa * rr / (0.622 + rr);
    esln = log(es);
    return((35.86 * esln - 4947.2325) / (esln -23.6837));
}

/*************************************************************************
 * HUMID_TEMP2DEW: Determine Dewpoint given % humidity and temperature C
 */

double humid_temp2dew(const double rel_humid, const double temp_C)
{
     double esubs;
     double es_td;
     double ratio;
     double dewpt;

     esubs = e_sub_s(temp_C);
     es_td = (rel_humid * 0.01 ) * esubs;
     ratio = 0.622 * es_td / (100000.0 - es_td);

     dewpt = press_mixr2dew(101300.0, ratio) - 273.1;

     return dewpt;
}

/******************************************************
 * COMPUTE_RELHUM: Return Relative humidity - Percent
 * Input temperature and dew_point in Degrees C
 */

double compute_relhum(const double dry_temp, const double dew_temp)                             
{
  return (e_sub_s(dry_temp) / e_sub_s(dew_temp)) / 100.0;
}

/******************************************************************************
 * STATIC ROUTINES
 *****************************************************************************/

/******************************************************************************
 * 
 */

static int compare_distance(const void *element1,
			    const void *element2)
{
  station_report_t *station1 = (station_report_t *)element1;
  station_report_t *station2 = (station_report_t *)element2;
  
  double distance1, distance2;
  
  double lat_diff, lon_diff;
  
  /*
   * Check for the same station.  We don't need to check locations
   * if the stations are the same.
   */

  if (!STRequal_exact(station1->station_label,
		      station2->station_label))
  {
    /*
     * Calculate the distance from station1 to the selected point.
     */

    lat_diff = station1->lat - Distance_sort_lat;
    lon_diff = station1->lon - Distance_sort_lon;
  
    distance1 = sqrt(lat_diff * lat_diff + lon_diff * lon_diff);
  
    /*
     * Calculate the distance from station2 to the selected point.
     */

    lat_diff = station2->lat - Distance_sort_lat;
    lon_diff = station2->lon - Distance_sort_lon;
  
    distance2 = sqrt(lat_diff * lat_diff + lon_diff * lon_diff);
  
    /*
     * Return based on which station is closer to the selected
     * point.  If the distances are equal, we will continue past
     * the end of this if statement and return based on the
     * comparison of the report times.
     */

    if (distance1 < distance2)
      return(-1);
    else if (distance1 > distance2)
      return(1);
    
  }
  
  /*
   * The stations are the same.  Sort based on report time, with
   * the latest report appearing first.
   */

  return(station2->time - station1->time);
}
