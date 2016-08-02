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
/***************************************************************************
 * test_data.c
 *
 * Creates test aircraft data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1998
 *
 ****************************************************************************/

#include "ac_ingest.h"
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <rapmath/stats.h>

typedef struct {

  time_t change_time;
  double head_bias;
  double start_lat, start_lon;
  double lat, lon;
  double altitude; /* m */
  double speed;    /* km/s */
  double heading;  /* deg T */
  char *callsign;
  time_t time_last;

} test_ac_data_t;

/*
 * file scope
 */

static test_ac_data_t *Ac_data = NULL;
static int N_ac;
static int Ac_num;

/****************
 * initialization
 */

void init_test_data(void)

{

  int i;
  time_t now = time(NULL);
  test_ac_data_t *ac;

  /*
   * set up random number generator
   */
  
  STATS_uniform_seed(now);

  Ac_num = 0;
  N_ac = Glob->params.test_aircraft.len;

  Ac_data = (test_ac_data_t *) umalloc (N_ac * sizeof(test_ac_data_t));
  
  ac = Ac_data;
  for (i = 0; i < N_ac; i++, ac++) {

    ac->change_time = 0;
    ac->start_lat = Glob->params.test_aircraft.val[i].start_lat;
    ac->start_lon = Glob->params.test_aircraft.val[i].start_lon;
    ac->lat = ac->start_lat;
    ac->lon = ac->start_lon;
    ac->altitude = Glob->params.test_aircraft.val[i].altitude * 0.3048;
    ac->speed = (Glob->params.test_aircraft.val[i].speed * 1.852) / 3600.0;
    ac->callsign = STRdup(Glob->params.test_aircraft.val[i].callsign);
    ac->time_last = now;
    ac->heading = STATS_uniform_gen() * 360.0;
    ac->head_bias = 0.0;

  }

}

/****************
 * get data
 */

void get_test_data(char **callsign_p,
		   double *lat_p, double *lon_p,
		   double *alt_p)

{

  time_t now = time(NULL);
  test_ac_data_t *ac;
  double dhead;
  double dtime;
  double dist;
  double new_lat, new_lon;
  double range, theta;

  Ac_num = (++Ac_num % N_ac);
  ac = Ac_data + Ac_num;

  if (now - ac->change_time > 120) {
    ac->change_time = now;
    ac->head_bias = -15.0 + STATS_uniform_gen() * 40.0;
  }

  /*
   * time since last computation
   */

  dtime = now - ac->time_last;
  ac->time_last = now;

  /*
   * compute heading
   */

  dhead = STATS_uniform_gen() * ac->head_bias;
  ac->heading += dhead;

  /*
   * distance moved
   */

  dist = ac->speed * dtime;

  /*
   * compute new location
   */
  
  PJGLatLonPlusRTheta(ac->lat, ac->lon,
		      dist, ac->heading,
		      &new_lat, &new_lon);

  /*
   * check dist from start point
   */

  PJGLatLon2RTheta(new_lat, new_lon,
		   ac->start_lat, ac->start_lon,
		   &range, &theta);

  if (range > 100.0) {
    ac->lat = ac->start_lat;
    ac->lon = ac->start_lon;
  } else {
    ac->lat = new_lat;
    ac->lon = new_lon;
  }

  *callsign_p = ac->callsign;
  *lat_p = ac->lat;
  *lon_p = ac->lon;
  *alt_p = ac->altitude;

}

/**********
 * clean up
 */

void free_test_data(void)

{

  int i;

  for (i = 0; i < N_ac; i++) {
    STRfree(Ac_data[i].callsign);
  }
  ufree(Ac_data);
  Ac_data = NULL;

}

