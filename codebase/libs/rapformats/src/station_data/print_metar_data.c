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

/*****************************************************************
 * PRINT_METAR_DATA - Print the METAR data to stdout
 *
 * Nancy Rehak, RAP, NCAR, Boulder, Co, USA, 80307
 *
 * October 1997
 *
 * Moved from apps/wsddm/src/metar_select to librapformats.a in
 * October 1999.
 *
 */

#include <stdio.h>

#include <rapformats/station_reports.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/udatetime.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

/*
 * Prototypes for static functions.
 */


static 
double nearest(double target,double delta);

/*================================================================*/

void print_metar_data(const station_report_t *metar_table,
		      const int num_metars,
		      const metar_select_out_style_t out_style)
{
  int i;
  
  /*
   * Start by printing 5 blank lines to scroll the old data off
   * the display.
   */

  for (i = 0; i < 5; i++)
    fprintf(stdout, "\n");
  
  /*
   * Now print the actual metars, in
   * the appropriate style.
   */

  switch (out_style) {

  case STYLE_WSDDM :
    print_wsddm(metar_table, num_metars);
    break;

  case STYLE_AOAWS :
    print_aoaws(metar_table, num_metars);
    break;

  default :
    fprintf(stderr,"Unrecognised output style!\n");
    exit(-1);
    break;

  }

}

/*************************************************************************
 * NEAREST: Compute the value nearest the target which is divisible by
 *         the absolute value of delta
 */

double
nearest(double target,double delta)
{
    double answer;
    double rem;                                                                 

    delta = fabs(delta);                   
    rem = remainder(target,delta);

    if(target >= 0.0) {
        if(rem > (delta / 2.0)) {
           answer = target + (delta - rem);
        } else {
          answer = target -  rem;
        }
    } else {
        if(fabs(rem) > (delta / 2.0)) {
           answer = target - (delta + rem);
        } else {
          answer = target -  rem;
        }
    }

    return answer;
}

/*----------------------------------------------------*/

void print_wsddm(const station_report_t *metar_table,
		 const int num_metars){

  int i;
  int printed_int_value;
  double vis_miles;
  double integral_val;
  double frac_val;
  double windgust;
  
  /*
   * Print WSDDM style.
   */

  /*
   * Print the header
   */

  fprintf(stdout, "STN GMT  TMP DEW DIR SPD GST VSBY  CEIL WEATHER\n");
  fprintf(stdout, "           C   C deg  kt  kt   mi    ft\n");
  fprintf(stdout, "=== ==== === === === === === ====  ==== =======\n");
  
  /*
   * Now print the METAR data
   */

  /*
  for(i=0; i<10; i++)
    fprintf(stdout,"\n");
    */

  for (i = 0; i < num_metars; i++)
    {
      date_time_t *time_struct = udate_time(metar_table[i].time);
    
      /* station name (don't print the first character) */

      fprintf(stdout, "%3s", &metar_table[i].station_label[1]);
    
      /* report time */

      fprintf(stdout, " %02d%02d",
	      time_struct->hour,
	      time_struct->min);
    
      /* temperature */

      if (metar_table[i].temp == STATION_NAN)
	fprintf(stdout, "    ");
      else
	{
	  fprintf(stdout, " %3.0f", metar_table[i].temp);
	}
    
      /* dew point */

      if (metar_table[i].dew_point == STATION_NAN)
	fprintf(stdout, "    ");
      else
	{
	  fprintf(stdout, " %3.0f", metar_table[i].dew_point);
	}
    
      /* wind direction  - ouput to nearest 10 degrees */

      if (metar_table[i].winddir == STATION_NAN)
	fprintf(stdout, "    ");
      else
	{
	  fprintf(stdout, " %3.0f", metar_table[i].winddir);
	}
    
      /* wind speed */

      if (metar_table[i].windspd == STATION_NAN)
	fprintf(stdout, "    ");
      else
	{
	  fprintf(stdout, " %3.0f", metar_table[i].windspd * NMH_PER_MS);
	}
    
      /* wind gust */

      windgust = metar_table[i].windgust * NMH_PER_MS;
    
      if (metar_table[i].windgust != STATION_NAN &&
	  windgust > 5.0)
	{
	  fprintf(stdout, "G");
	  fprintf(stdout, "%3.0f", windgust);
	}
      else
	fprintf(stdout, "    ");
    
      /* visibility */

      vis_miles = metar_table[i].visibility / KM_PER_MI;
      frac_val = modf(vis_miles, &integral_val);

      if (metar_table[i].visibility == STATION_NAN)
	{
	  fprintf(stdout, "     ");
	}
      else if (vis_miles > 4 || frac_val < 0.001 || frac_val > 0.999)
	{
	  fprintf(stdout, " %4.0f", vis_miles);
	}
      else
	{
	  fprintf(stdout, " %4.2f", vis_miles);
	}
    
      /* ceiling */

      if (metar_table[i].ceiling == STATION_NAN)
	fprintf(stdout, "      ");
      else
	{
	  printed_int_value =
	    (int)(nearest(((metar_table[i].ceiling / KM_PER_MI * FT_PER_MI) + 0.5),100.0));
      
	  fprintf(stdout, " %5d",
		  printed_int_value);
	}
    
      /* weather string */

      if (metar_table[i].msg_id == METAR_REPORT) {
	fprintf(stdout, " %s\n", metar_table[i].shared.metar.weather_str);
      } else {
	fprintf(stdout, " %s\n",
		weather_type2string_trunc(metar_table[i].weather_type, 2));
      }
    
    }

}



/*----------------------------------------------------*/

void print_aoaws(const station_report_t *metar_table,
		 const int num_metars){

  int i;
  int printed_int_value;
  double vis_km;
  double windgust;
  
  /*
   * Print AOAWS style.
   */

  /*
   * Print the header
   */

  fprintf(stdout,
	  "NAME TIME W/D W/S GST  VIS     WEATHER CEIL TMP DEW  QNH\n");
  fprintf(stdout,
	  "          deg  kt  kt km/m               ft   C   C  hPa\n");
  fprintf(stdout,
	  "==== ==== === === === ====     ======= ==== === === ====\n");

  /*
   * Now print the METAR data
   */

  if (num_metars == 0) fprintf(stdout,"\nNo METAR data found.\n");

  for (i = 0; i < num_metars; i++)
    {
      date_time_t *time_struct = udate_time(metar_table[i].time);
    
      /* station name */

      fprintf(stdout, "%4s", metar_table[i].station_label);
    
      /* report time */

      fprintf(stdout, " %02d%02d", time_struct->hour, time_struct->min);
    
      /* wind direction  - ouput to nearest degree */

      if (metar_table[i].winddir == STATION_NAN)
	fprintf(stdout, "    ");
      else
	{
	  fprintf(stdout, " %03d", (int)metar_table[i].winddir);
	}
    
      /* wind speed */

      if (metar_table[i].windspd == STATION_NAN)
	fprintf(stdout, "    ");
      else
	{
	  fprintf(stdout, " %3.0f", metar_table[i].windspd * NMH_PER_MS);
	}
    
      /* wind gust */

      windgust = metar_table[i].windgust * NMH_PER_MS;
    
      if (metar_table[i].windgust != STATION_NAN &&
	  windgust > 5.0)
	{
	  fprintf(stdout, " %3.0f", windgust);
	}
      else
	fprintf(stdout, "    ");
    
      /* visibility */

      vis_km = metar_table[i].visibility;

      /*
       * Now, we check to se if the vis is less than
       * 5 Km and if it is we print it in meters.
       */
      if (metar_table[i].visibility == STATION_NAN)
	{
	  fprintf(stdout, "     ");
	} else {
	  if (vis_km <= 5.0) {
	    fprintf(stdout, " %-4d", (int)(vis_km * 1000.0 + 0.5) );
	  } else {
	    fprintf(stdout, " %4.1f", vis_km );
	  }
	}


      /* weather string */

      if (metar_table[i].msg_id == METAR_REPORT) {
	fprintf(stdout, " %11s", metar_table[i].shared.metar.weather_str);
      } else {
	fprintf(stdout, " %11s",
		weather_type2string_trunc(metar_table[i].weather_type, 2));
      }

      /* ceiling */

      if (metar_table[i].ceiling == STATION_NAN)
	fprintf(stdout, "     ");
      else
	{

    
	  printed_int_value =
	    (int)(nearest(((metar_table[i].ceiling / KM_PER_MI * FT_PER_MI) + 0.5),100.0));
	  
	  if (printed_int_value <= 9999){
	    fprintf(stdout, " %4d",
		    printed_int_value);
	  } else {
	    fprintf(stdout,"  CLR");
	  }
	}
	
        
      /* temperature */

      if (metar_table[i].temp == STATION_NAN)
	fprintf(stdout, "    ");
      else
	{
	  fprintf(stdout, " %3.0f", metar_table[i].temp);
	}
    
      /* dew point */

      if (metar_table[i].dew_point == STATION_NAN)
	fprintf(stdout, "    ");
      else
	{
	  fprintf(stdout, " %3.0f", metar_table[i].dew_point);
	}

      /* pressure */

      if (metar_table[i].pres == STATION_NAN)
	fprintf(stdout, "     ");
      else
	{
	  fprintf(stdout, " %4.0f", metar_table[i].pres);
	}

      fprintf(stdout,"\n");
    

    }

}

