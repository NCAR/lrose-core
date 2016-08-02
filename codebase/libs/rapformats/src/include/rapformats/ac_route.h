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
/**********************************************************************
 * ac_route.h
 *
 * structs and defines for aircraft route information
 *
 * An aircraft route is defined by a continuous block of memory that
 * starts with one ac_route_header_t struct followed by N number
 * of ac_route_posn_t structs, where N is defined in the header as
 * routeSize.
 *
 * Jason Craig, RAP, NCAR, Boulder, CO
 *
 * June 2005
 */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef ac_route_h
#define ac_route_h

#include <dataport/port_types.h>
#include <rapformats/ac_data.h>

#define AC_ROUTE_MISSING_INT -999
#define AC_ROUTE_MISSING_FLOAT -999.00

#define AC_ROUTE_N_CALLSIGN 15
#define AC_ROUTE_N_POSNAME 15
#define AC_ROUTE_N_DATA 10

/*
 * Route type enumeration.
 *
 * Details what type of route message this is.
 */
typedef enum
{
  AC_ROUTE_FlightPlan,  /* initial fligh plan */
  AC_ROUTE_Departure,   /* started the route */
  AC_ROUTE_Arrival,     /* finished the route */
  AC_ROUTE_Cancelled,   /* route is cancelled */
  AC_ROUTE_Revised,     /* revised route given */
  AC_ROUTE_BoundryCross,/* crossed a boundry crossing */
  AC_ROUTE_Oceanic      /* oceanic 1-3 point route */
} ac_route_type_t;

typedef struct {

  ui32 route_type; /* see ac_route_type_t above */
  ui32 alt_type;   /* see ac_data_alt_type_t in ac_data.h */
  si32 routeSize;  /* Number of ac_route_posn_t structs to follow, each one a point of the route */
  si32 destTime;   /* Destination time  (estimated or actual) */
  si32 deptTime;   /* Departure time */
  fl32 altitude;
  si32 airSpeed;
  si32 coordTime;  /* Coordination time */
  si32 spacer;     /* Extra space for future additions or flags */
  char coordFix[AC_ROUTE_N_POSNAME];
  char aircraftData[AC_ROUTE_N_DATA];
  char destination[AC_ROUTE_N_POSNAME];
  char departure[AC_ROUTE_N_POSNAME];
  char callsign[AC_ROUTE_N_CALLSIGN];

} ac_route_header_t;

typedef struct {

  fl32 lat;
  fl32 lon;
  char name[AC_ROUTE_N_POSNAME];

} ac_route_posn_t;

/********************************************************************
 *
 * create_ac_route()
 *
 * creates an ac_route and returns a pointer to it
 * This function will handle the conversion to BE format
 * do not call ac_route_to_BE on the returned pointer
 *
 */

extern void *create_ac_route(ac_route_type_t route_type, ac_data_alt_type_t alt_type,
			     int routeSize, int destTime, int deptTime,
			     float altitude,   int airSpeed,
			     int coordTime, char *coordFix, char *aircraftData,
			     char *destination, char *departure,
			     char *callsign, ac_route_posn_t *routeArray, int *ac_route_size);


/********************************************************************
 *
 * create_ac_route()
 *
 * given an ac_route returns the routeSize, estArrival, callsign, and routeArray
 * This function will handle the conversion from BE format
 * do not call ac_route_from_BE beforehand
 * Also allocates the memory for the routeArray, which should be deleted later.
 *
 */

extern void decode_ac_route(void *ac_route, ac_route_type_t *route_type, ac_data_alt_type_t *alt_type,
			    int *routeSize, int *destTime, int *deptTime, float *altitude, int *airSpeed, 
			    int *coordTime, char *coordFix, char *aircraftData, char* destination,
			    char *departure, char *callsign, ac_route_posn_t *routeArray);


/********************************************************************
 *
 * BE_from_ac_route()
 *
 * Gets BE format from ac_route pointer
 *
 */

extern void BE_from_ac_route(void *ac_route);

/********************************************************************
 *
 * BE_to_ac_posn()
 *
 * Converts BE format to ac_route pointer
 *
 */

extern void BE_to_ac_route(void *ac_route);

/********************************************************************
 * ac_route_print
 *
 * Prints out struct
 */

extern void ac_route_print(FILE *out,
                           const char *spacer,
                           void *ac_route);

#endif

#ifdef __cplusplus
}
#endif
