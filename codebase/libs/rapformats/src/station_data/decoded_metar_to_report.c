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
 * DECODED_METAR_TO_REPORT.c
 *
 * Mike Dixon, RAP, NCAR, PO Box 3000, Boulder, CO, USA
 * Copied from metar2spdb.
 * June 2001.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <rapformats/station_reports.h>
#include <toolsa/toolsa_macros.h>

#include <toolsa/str.h>
#include <toolsa/utim.h>

#define P0 1013.25
#define A  0.0065
#define T0 288.0
#define N  0.190284

/*********************************************************************
 * _sl2StnPressure() - Calculate the station pressure given the altimeter
 *                    reading and station elevation.
 */

double _sl2StnPressure(const double altimeter, /* in mb */
		       const double elevation) /*  in m */
{
  /*  Calculate the station pressure value */

  double INV_N;
  double ELEV_CONST;
  
  INV_N = 1.0 / N;
  ELEV_CONST = (pow(P0, N) * A) / T0;
  
  return pow(pow(altimeter, N) - (ELEV_CONST * elevation), INV_N) + 0.01;
}

/********************************************************************
 * relh()
 *
 * Returns relative humidity (%),
 * given temp (C) and dewpoint (C).
 */

static double _relh(double t, double td)

{

  double ratio, relh;

  double vdt = 6.1121 * exp((17.502*td)/(td+240.97));
  double vt = 6.1121 * exp((17.502*t)/(t+240.97));

  ratio = vdt / vt;
  if (ratio > 1.0) {
    ratio = 1.0;
  }
  relh = 100.0 * ratio;
  
  return (relh);

}

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

#define NOBSTRUCT 10

int decoded_metar_to_generic_report(const Decoded_METAR *dcdMetar,
				    station_report_t *report,
				    time_t valid_time,
				    double lat, double lon, double alt)
{
  int valid = FALSE;
  int i;
  const char *tstPtr;

  report->time = valid_time;

  /*
   * For 24 hour precip amount
   */

  report->accum_start_time = report->time - 86400;

  /*
   * Search through the 10 remark slots for condition indicators.
   */

  report->weather_type = 0;
  for( i = 0; i < NOBSTRUCT; i++ ) {
    
    tstPtr = dcdMetar->WxObstruct[i];
    
    if(strstr(tstPtr,"RA") != NULL) {
      if(strstr(tstPtr,"+RA") != NULL) {
	report->weather_type |= WT_PRA;
      } else if(strstr(tstPtr,"-RA") != NULL) {
	report->weather_type |= WT_MRA;
      } else {
	report->weather_type |= WT_RA;
      }
    }
    
    if(strstr(tstPtr,"SN") != NULL) {
      if(strstr(tstPtr,"+SN") != NULL) {
	report->weather_type |= WT_PSN;
      } else if(strstr(tstPtr,"-SN") != NULL) {
	report->weather_type |= WT_MSN;
      } else {
	report->weather_type |= WT_SN;
      }
    }
    
    if(strstr(tstPtr,"IC") != NULL) {
      report->weather_type |= WT_SN;
    }
    
    if(strstr(tstPtr,"PE") != NULL) {
      report->weather_type |= WT_PE;
    }
    
    if(strstr(tstPtr,"SG") != NULL) { /* Fudge snow grains to snow pellets. */
      report->weather_type |= WT_PE;
    }
    
    if(strstr(tstPtr,"UP") != NULL) {
      report->weather_type |= WT_UP;
    }
    
    if(strstr(tstPtr,"FG") != NULL) {
      report->weather_type |= WT_FG;
    }
    
    if(strstr(tstPtr,"FZFG") != NULL) {
      /*
       * If it is freezing, there is no fog
       */
      report->weather_type |= WT_FZFG;
      report->weather_type &= ~WT_FG; 
    }
    
    if(strstr(tstPtr,"BR") != NULL) {
      report->weather_type |= WT_BR;
    }
    
    if(strstr(tstPtr,"HZ") != NULL) {
      report->weather_type |= WT_HZ;
    }
    
    if(strstr(tstPtr,"SQ") != NULL) {
      report->weather_type |= WT_SQ;
    }
    
    if(strstr(tstPtr,"DS") != NULL) {
      report->weather_type |= WT_DS;
    }
    
    if(strstr(tstPtr,"SS") != NULL) {  /* Fudge Sand Storm to Dust Storm. */
      report->weather_type |= WT_DS;
    }
    
    if(strstr(tstPtr,"FC") != NULL) {
      report->weather_type |= WT_FC;
    }
    
    if(strstr(tstPtr,"TS") != NULL) {
      if(strstr(tstPtr,"+TS") != NULL) {
	report->weather_type |= WT_PTS;
      } else {
	report->weather_type |= WT_TS;
      }
    }
    
    if(strstr(tstPtr,"GR") != NULL) {
      report->weather_type |= WT_GR;
    }
    
    if(strstr(tstPtr,"GS") != NULL) { /* Fudge Small Hail to Snow Pellets  */
      report->weather_type |= WT_PE;
    }
    
    if(strstr(tstPtr,"FZRA") != NULL) {
      if(strstr(tstPtr,"+FZRA") != NULL) {
	report->weather_type |= WT_PFZRA;
      } else if(strstr(tstPtr,"-FZRA") != NULL) {
	report->weather_type |= WT_MFZRA;
      } else {
	report->weather_type |= WT_FZRA;
      }
      
      /*
       * If it is freezing rain, no moderate rain, heavy rain or
       * light rain
       */
      report->weather_type &= ~WT_RA; 
      report->weather_type &= ~WT_PRA;
      report->weather_type &= ~WT_MRA;
    }
    
    if(strstr(tstPtr,"VA") != NULL) {
      report->weather_type |= WT_VA;
    }
    
    if(strstr(tstPtr,"BLSN") != NULL) {
      report->weather_type |= WT_BLSN;
    }
    
    if(strstr(tstPtr,"DZ") != NULL) {
      report->weather_type |= WT_DZ;
    }
    
    if(strstr(tstPtr,"FZDZ") != NULL) {

      if(strstr(tstPtr,"-FZDZ") != NULL) {
        report->weather_type |= WT_MFZDZ;

      } else if(strstr(tstPtr,"+FZDZ") != NULL) {
        report->weather_type |= WT_PFZDZ;

      } else {
         report->weather_type |= WT_FZDZ;
	  }
    
      /*
       * If it's freezing no drizzle
       */
      report->weather_type &= ~WT_DZ;
    }
    
    
  }
  
  report->lat = lat;
  report->lon = lon;
  report->alt = alt;
  
  if(dcdMetar->temp != MAXINT) {
    report->temp = dcdMetar->temp;
    valid = TRUE;
  } else {
    report->temp = STATION_NAN;
  }
  
  if(dcdMetar->dew_pt_temp != MAXINT) {
    report->dew_point = dcdMetar->dew_pt_temp;
    valid = TRUE;
  } else {
    report->dew_point = STATION_NAN;
  }
  
  if (report->temp != STATION_NAN &&
      report->dew_point != STATION_NAN) {
    report->relhum = _relh(report->temp,
			   report->dew_point);
  } else {
    report->relhum = STATION_NAN;
  }
  
  if(dcdMetar->winData.windSpeed != MAXINT) {
    valid = TRUE;
    if (!strcmp(dcdMetar->winData.windUnits, "MPS")) {
      /* m/s */
      report->windspd = dcdMetar->winData.windSpeed;
    } else if (!strcmp(dcdMetar->winData.windUnits, "KMH")) {
      /* kmh */
      report->windspd =
	dcdMetar->winData.windSpeed / MPERSEC_TO_KMPERHOUR;
    } else {
      /* assume knots */
      report->windspd = dcdMetar->winData.windSpeed / NMH_PER_MS;
    }
  } else {
    report->windspd = STATION_NAN;
  }
  
  if(dcdMetar->winData.windGust != MAXINT) {
    valid = TRUE;
    if (!strcmp(dcdMetar->winData.windUnits, "MPS")) {
      /* m/s */
      report->windgust = dcdMetar->winData.windGust;
    } else if (!strcmp(dcdMetar->winData.windUnits, "KMH")) {
      /* kmh */
      report->windgust =
	dcdMetar->winData.windGust / MPERSEC_TO_KMPERHOUR;
    } else {
      /* assume knots */
      report->windgust = dcdMetar->winData.windGust / NMH_PER_MS;
    }
  } else {
    report->windgust = STATION_NAN;
  }
  
  if (dcdMetar->winData.windVRB) {
    report->winddir = -0.0; /* set for variable winds */
    valid = TRUE;
  } else if (dcdMetar->winData.windDir == MAXINT) {
    report->winddir = STATION_NAN;
  } else {
    valid = TRUE;
    report->winddir = dcdMetar->winData.windDir;
  }
  
  if(dcdMetar->hectoPasc_altstng != MAXINT) {
    valid = TRUE;
    report->pres = dcdMetar->hectoPasc_altstng;
  } else {
    report->pres = STATION_NAN;
  }
  
  if(dcdMetar->precip_24_amt != MAXINT) {
    valid = TRUE;
    report->liquid_accum = dcdMetar->precip_24_amt;
  } else {
    report->liquid_accum = STATION_NAN;
  }
  
  if(dcdMetar->hourlyPrecip != MAXINT) {
    valid = TRUE;
    report->precip_rate = dcdMetar->hourlyPrecip;
  } else {
    report->precip_rate = STATION_NAN;
  }
  
  if(dcdMetar->prevail_vsbySM != MAXINT) {
    valid = TRUE;
    report->visibility = dcdMetar->prevail_vsbySM * KM_PER_MI;
  } else {
    report->visibility = STATION_NAN;
  }
  
  if(dcdMetar->RRVR[0].visRange != MAXINT) {
    valid = TRUE;
    report->rvr = dcdMetar->RRVR[0].visRange;
  } else {
    report->rvr = STATION_NAN;
  }
  
  for(i=1; i < 12; i++) {
    if(dcdMetar->RRVR[i].visRange != MAXINT && 
       dcdMetar->RRVR[i].visRange < report->rvr ) {
      valid = TRUE;
      report->rvr = dcdMetar->RRVR[i].visRange;
    }
  }
  
  report->ceiling = STATION_NAN;
  
  if (dcdMetar->Ceiling != MAXINT) {
    
    valid = TRUE;
    report->ceiling = dcdMetar->Ceiling * 0.0003048;
    
  } else if (dcdMetar->Estimated_Ceiling != MAXINT) {
    
    valid = TRUE;
    report->ceiling = dcdMetar->Estimated_Ceiling * 0.0003048;
    
  } else if (strlen(dcdMetar->cldTypHgt[0].cloud_type) < 3) {
    
    report->ceiling = STATION_NAN;
    
  } else if (!strcmp(dcdMetar->cldTypHgt[0].cloud_type, "CLR") ||
             !strcmp(dcdMetar->cldTypHgt[0].cloud_type, "SKC") ||
             !strcmp(dcdMetar->cldTypHgt[0].cloud_type, "NSC") ) {
    report->ceiling = STATION_NAN;
    
  } else {

    double min_ceiling = STATION_NAN;
    if(report->msg_id == METAR_WITH_REMARKS_REPORT)
       {
       /* This conditional was added for the CIP in-flight icing product. */
       /* Refer to CVS log comments for explanation. */
       for (i = 0; i < 6; i++)
          {
          if(dcdMetar->cldTypHgt[i].cloud_hgt_meters != MAXINT)
	     {
             if (dcdMetar->cldTypHgt[i].cloud_type[0] == 'X' ||
                !strcmp(dcdMetar->cldTypHgt[i].cloud_type, "BKN") ||
                !strcmp(dcdMetar->cldTypHgt[i].cloud_type, "SCT") ||
                !strcmp(dcdMetar->cldTypHgt[i].cloud_type, "OVC"))
	        {
                double ceiling = dcdMetar->cldTypHgt[i].cloud_hgt_meters / 1000.0;
                valid = TRUE;
                if (ceiling < min_ceiling)
                   {
                   min_ceiling = ceiling;
                   }
                }
	     }
          }
       }
    else
       {
       for (i = 0; i < 6; i++)
          {
          if(dcdMetar->cldTypHgt[i].cloud_hgt_meters != MAXINT)
	     {
             if (dcdMetar->cldTypHgt[i].cloud_type[0] == 'X' ||
                !strcmp(dcdMetar->cldTypHgt[i].cloud_type, "BKN") ||
                !strcmp(dcdMetar->cldTypHgt[i].cloud_type, "VV") ||
                !strcmp(dcdMetar->cldTypHgt[i].cloud_type, "OVC"))
	        {
                double ceiling = dcdMetar->cldTypHgt[i].cloud_hgt_meters / 1000.0;
                valid = TRUE;
                if (ceiling < min_ceiling)
                   {
                   min_ceiling = ceiling;
                   }
                }
	     }
          }
       }
    report->ceiling = min_ceiling;
  }
  
  for (i = 0; i < 6; i++) {
    if(dcdMetar->cldTypHgt[i].cloud_hgt_meters != MAXINT) {
      if (!strcmp(dcdMetar->cldTypHgt[i].cloud_type, "OVC")) {
	report->weather_type |= WT_OVC;
      } else if (!strcmp(dcdMetar->cldTypHgt[i].cloud_type, "BKN")) {
	report->weather_type |= WT_BKN;
      } else if (!strcmp(dcdMetar->cldTypHgt[i].cloud_type, "SCT")) {
	report->weather_type |= WT_SCT;
      } else if (!strcmp(dcdMetar->cldTypHgt[i].cloud_type, "FEW")) {
	report->weather_type |= WT_FEW;
      }
    }
  }

  strncpy(report->station_label, dcdMetar->stnid, 4);

  /*
   * Make sure station label is null terminated
   */

  report->station_label[4] = '\0'; 
  
  if (valid) {
    return(0);
  } else {
    return (-1);
  }
  
}


/******************************************************************************
 * DECODED_METAR_TO_REPORT
 *
 * Fill out a METAR_REPORT style report from a decoded metar struct.
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

int decoded_metar_to_report(const Decoded_METAR *dcdMetar,
			    station_report_t *report,
			    time_t valid_time,
			    double lat, double lon, double alt)

{
  int valid = FALSE;
  int wxlen;
  char *wxstr; 
  
  report->msg_id = METAR_REPORT;

  /*
   * Load up weather string from 3 possible weather tokens
   */

  wxstr = report->shared.metar.weather_str;
  *wxstr = '\0';
  if (strlen(dcdMetar->WxObstruct[0]) > 0) {
    STRconcat(wxstr, dcdMetar->WxObstruct[0], METAR_WX_STR_LEN);
    valid = TRUE;
  }
  wxlen = strlen(dcdMetar->WxObstruct[1]);
  if ((wxlen > 0) && (strlen(wxstr) + wxlen + 2 < METAR_WX_STR_LEN)) {
    STRconcat(wxstr, " ", METAR_WX_STR_LEN);
    STRconcat(wxstr, dcdMetar->WxObstruct[1], METAR_WX_STR_LEN);
    valid = TRUE;
  }
  wxlen = strlen(dcdMetar->WxObstruct[2]);
  if ((wxlen > 0) && (strlen(wxstr) + wxlen + 2 < METAR_WX_STR_LEN)) {
    STRconcat(wxstr, " ", METAR_WX_STR_LEN);
    STRconcat(wxstr, dcdMetar->WxObstruct[2], METAR_WX_STR_LEN);
    valid = TRUE;
  }
  wxstr[METAR_WX_STR_LEN - 1] = '\0';

  if (decoded_metar_to_generic_report(dcdMetar, report, valid_time,
				      lat, lon, alt) == 0)
    valid = TRUE;
  
  if (valid)
    return 0;
  else
    return -1;
  
}


/******************************************************************************
 * DECODED_METAR_TO_REPORT_WITH_REMARKS
 *
 * Fill out a METAR_WITH_REMARKS_REPORT style report from a decoded metar 
 * struct.
 *
 * Inputs:
 *   dcdMetar: Decoded_METAR structure
 *   valid_time: time for METAR obs
 *   lat, lon, alt:: metar station data
 *
 * Output:
 *   station_report_t structure is populated.
 *
 * Returns 0 on success, -1 on failure.
 */

int decoded_metar_to_report_with_remarks(const Decoded_METAR *dcdMetar,
					 station_report_t *report,
					 time_t valid_time,
					 double lat, double lon, double alt)
   {
   int valid = FALSE;
   char *stn_indc; 
  
   report->msg_id = METAR_WITH_REMARKS_REPORT;

   /*
    * Load remark info
    */

   stn_indc = report->shared.remark_info.stn_indicator;
   *stn_indc = '\0';

   if(strlen(dcdMetar->autoIndicator) > 0)
      {
      STRcopy(stn_indc, dcdMetar->autoIndicator, STN_INDC_LEN);
      valid = TRUE;
      }

   report->shared.remark_info.pwi_no = (ui08) dcdMetar->PWINO;
   report->shared.remark_info.fzra_no = (ui08) dcdMetar->FZRANO;
   report->shared.remark_info.ts_no = (ui08) dcdMetar->TSNO;

   if(decoded_metar_to_generic_report(dcdMetar, report, valid_time, lat, lon, alt) == 0)
      {
      valid = TRUE;
      }
  
   if(valid)
      {
      return 0;
      }
   else
      {
      return -1;
      }
   }


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

int decoded_metar_to_station_report(const Decoded_METAR *dcdMetar,
				    station_report_t *report,
				    time_t valid_time,
				    double lat, double lon, double alt)
{
  int valid = FALSE;
  
  report->msg_id = STATION_REPORT;

  report->shared.station.liquid_accum2 = STATION_NAN;
  report->shared.station.Spare1 = STATION_NAN;
  report->shared.station.Spare2 = STATION_NAN;
  
  if (decoded_metar_to_generic_report(dcdMetar, report, valid_time,
				      lat, lon, alt) == 0)
    valid = TRUE;
  
  if (valid)
    return 0;
  else
    return -1;
  
}


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

int decoded_metar_to_pressure_station_report(const Decoded_METAR *dcdMetar,
					     station_report_t *report,
					     time_t valid_time,
					     double lat, double lon, double alt)
{
  int valid = FALSE;
  
  report->msg_id = PRESSURE_STATION_REPORT;

  report->shared.pressure_station.stn_pres = STATION_NAN;
  report->shared.pressure_station.Spare1 = STATION_NAN;
  report->shared.pressure_station.Spare2 = STATION_NAN;
  
  if (decoded_metar_to_generic_report(dcdMetar, report, valid_time,
				      lat, lon, alt) == 0)
    valid = TRUE;
  
  if (report->pres != STATION_NAN &&
      report->alt != STATION_NAN)
    report->shared.pressure_station.stn_pres =
      _sl2StnPressure(report->pres, report->alt);
  
  if (valid)
    return 0;
  else
    return -1;
  
}


