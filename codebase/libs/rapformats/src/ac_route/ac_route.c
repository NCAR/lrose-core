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
/*********************************************************************
 *
 * ac_route.c
 *
 * Aircraft Route Data
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * June 2005
 *
 *********************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dataport/bigend.h>
#include <rapformats/ac_route.h>

/********************************************************************
 *
 * create_ac_route()
 *
 * creates an ac_route and returns a pointer to it
 * This function will handle the conversion to BE format
 * do not call ac_route_to_BE on the returned pointer
 * Returns total size of data in ac_route_size
 */

void *create_ac_route(ac_route_type_t route_type, ac_data_alt_type_t alt_type,
		      int routeSize, int destTime, int deptTime,
		      float altitude,   int airSpeed,
		      int coordTime, char *coordFix, char *aircraftData,
		      char *destination, char *departure,
		      char *callsign, ac_route_posn_t *routeArray, int *ac_route_size)
{
  void *ac_route;
  ac_route_header_t header;
  int i;

  *ac_route_size = sizeof(ac_route_header_t)+(routeSize*sizeof(ac_route_posn_t));
  ac_route = (void *)malloc(*ac_route_size);
  /*
   * Create header information
   */
  memset(&header, 0, sizeof(ac_route_header_t));
  header.route_type = route_type;
  header.alt_type = alt_type;
  header.routeSize = routeSize;
  header.destTime = destTime;
  header.deptTime = deptTime;
  header.altitude = altitude;
  header.airSpeed = airSpeed;
  header.coordTime = coordTime;
  snprintf(header.coordFix, AC_ROUTE_N_POSNAME-1, "%s", coordFix);
  snprintf(header.aircraftData, AC_ROUTE_N_DATA-1, "%s", aircraftData);
  snprintf(header.destination, AC_ROUTE_N_POSNAME-1, "%s", destination);
  snprintf(header.departure, AC_ROUTE_N_POSNAME-1, "%s", departure);
  snprintf(header.callsign, AC_ROUTE_N_CALLSIGN-1, "%s", callsign);
  BE_from_array_32(&header, (6 * sizeof(si32)) + (1 * sizeof(fl32)) + (2 * sizeof(ui32)));
  memcpy(ac_route, &header, sizeof(ac_route_header_t));
  /*
   * Create route information
   */
  memcpy((char *)ac_route+sizeof(ac_route_header_t), routeArray, sizeof(ac_route_posn_t)*routeSize);
  routeArray = (ac_route_posn_t *)((char *)ac_route+sizeof(ac_route_header_t));
  for(i = 0; i < routeSize; i++) {
    BE_from_array_32(&(routeArray[i]), 2 * sizeof(fl32));
  }

  return ac_route;
}

/********************************************************************
 *
 * decode_ac_route()
 *
 * given an ac_route returns the routeSize, estArrival, callsign, and routeArray
 * This function will handle the conversion from BE format
 * do not call ac_route_from_BE beforehand
 * Allocates the memory for the routeArray, which should be deleted later.
 *
 */

void decode_ac_route(void *ac_route, ac_route_type_t *route_type, ac_data_alt_type_t *alt_type,
		     int *routeSize, int *destTime, int *deptTime, float *altitude, int *airSpeed, 
		     int *coordTime, char *coordFix, char *aircraftData, char* destination,
		     char *departure, char *callsign, ac_route_posn_t *routeArray)
{
  ac_route_header_t header;
  int a;

  memcpy(&header, ac_route, sizeof(ac_route_header_t));
  BE_to_array_32(&header, (6 * sizeof(si32)) + (1 * sizeof(fl32)) + (2 * sizeof(ui32)));
  *route_type = header.route_type;
  *alt_type = header.alt_type;
  *routeSize = header.routeSize;
  *destTime = header.destTime;
  *deptTime = header.deptTime;
  *altitude = header.altitude;
  *airSpeed = header.airSpeed;
  *coordTime = header.coordTime;
  sprintf(coordFix, "%s", header.coordFix);
  sprintf(aircraftData, "%s", header.aircraftData);
  sprintf(destination, "%s", header.destination);
  sprintf(departure, "%s", header.departure);
  sprintf(callsign, "%s", header.callsign);

  routeArray = (ac_route_posn_t *)malloc(header.routeSize*sizeof(ac_route_posn_t));
  memcpy(routeArray, (char *)ac_route+sizeof(ac_route_header_t), sizeof(ac_route_posn_t)*header.routeSize);
  for(a = 0; a < header.routeSize; a++) {
    BE_to_array_32(&(routeArray[a]), 2 * sizeof(fl32));
  }

}

/********************************************************************
 *
 * BE_from_ac_route()
 *
 * Gets BE format from ac_route pointer
 *
 */

void BE_from_ac_route(void *ac_route)
{
  int routeSize;
  ac_route_header_t *header;
  ac_route_posn_t *routeArray;
  int a;

  header = (ac_route_header_t *)ac_route;
  routeSize = header->routeSize;
  BE_from_array_32(header, (6 * sizeof(si32)) + (1 * sizeof(fl32)) + (2 * sizeof(ui32)));

  routeArray = (ac_route_posn_t *)((char *)ac_route+sizeof(ac_route_header_t));

  for(a = 0; a < routeSize; a++) {
    BE_from_array_32(&(routeArray[a]), 2 * sizeof(fl32));
  }
}

/********************************************************************
 *
 * BE_to_ac_posn()
 *
 * Converts BE format to ac_route pointer
 *
 */

void BE_to_ac_route(void *ac_route)
{
  int routeSize, a;
  ac_route_header_t *header;
  ac_route_posn_t *routeArray;

  header = (ac_route_header_t *)ac_route;
  BE_to_array_32(header, (6 * sizeof(si32)) + (1 * sizeof(fl32)) + (2 * sizeof(ui32)));
  routeSize = header->routeSize;
  routeArray = (ac_route_posn_t *)((char *)ac_route+sizeof(ac_route_header_t));

  for(a = 0; a < routeSize; a++) {
    BE_to_array_32(&(routeArray[a]), 2 * sizeof(fl32));
  }
}

/********************************************************************
 * ac_route_print
 *
 * Prints out struct
 */

void ac_route_print(FILE *out,
		    const char *spacer,
		    void *ac_route)
{
  ac_route_header_t *header;
  ac_route_posn_t *routeArray;
  int a;

  header = (ac_route_header_t *)ac_route;
  routeArray = (ac_route_posn_t *)((char *)ac_route+sizeof(ac_route_header_t));

  fprintf(out, "%s ac_route_t struct:\n", spacer);
  fprintf(out, "%s   callsign: %s\n", spacer, header->callsign);
  fprintf(out, "%s   destTime: %i\n", spacer, header->destTime);
  fprintf(out, "%s   deptTime: %i\n", spacer, header->deptTime);
  fprintf(out, "%s   altitude: %g (%s)\n", spacer, header->altitude, ac_data_alt_type2string(header->alt_type));
  fprintf(out, "%s   airSpeed: %i\n", spacer, header->airSpeed);
  fprintf(out, "%s  coordTime: %i\n", spacer, header->coordTime);

  fprintf(out, "%s   coordFix: %s\n", spacer, header->coordFix);
  fprintf(out, "%s   aircraft: %s\n", spacer, header->aircraftData);
  fprintf(out, "%sdestination: %s\n", spacer, header->destination);
  fprintf(out, "%s  departure: %s\n", spacer, header->departure);
  fprintf(out, "%s  routeSize: %i\n", spacer, header->routeSize);

  if(header->routeSize > 0) {
    fprintf(out, "%s     Route Points:\n", spacer);
    
    for(a = 0; a < header->routeSize; a++) {
      fprintf(out, "%s       name: %s\n", spacer, routeArray[a].name);
      fprintf(out, "%s        lat: %g\n", spacer, routeArray[a].lat);
      fprintf(out, "%s        lon: %g\n", spacer, routeArray[a].lon);
    }
  }
}
