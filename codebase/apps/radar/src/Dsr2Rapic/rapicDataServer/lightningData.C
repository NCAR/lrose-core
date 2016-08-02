/*

  Implementation of lightningData class

*/

#include "obsData.h"

void lightningData::set(time_t _tm, float _lat, float _lng, float _kamps)
{
  tm = _tm;
  lat = _lat;
  lng = _lng;
  kamps = _kamps;
}


void lightningData::dumphdr(FILE *dumpfile, bool endline)
{
  if (!dumpfile) return;
  fprintf(dumpfile, "time                lat     lng     kAmps");
  if (endline) 
    fprintf(dumpfile, "\n");
}

void lightningData::dump(FILE *dumpfile, bool endline)
{
  if (!dumpfile) return;
  char timestr[128];
  fprintf(dumpfile, "%s %7.1f %7.1f %6.1f",
	  ShortTimeString(tm, timestr, true),
	  lat, lng, kamps);
  if (endline) 
    fprintf(dumpfile, "\n");
}

