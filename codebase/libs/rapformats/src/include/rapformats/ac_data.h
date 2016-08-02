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
 * ac_data.h
 *
 * structs and defines for aircraft position with associated data
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO
 *
 * July 1998
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ac_data_h
#define ac_data_h

#include <dataport/port_types.h>

/*
 * Number of 32 bit fields in the base ac_data_t structure.  Used for
 * internal byte-swapping routines.
 */

#define AC_DATA_NUM_32        20

/*
 * Structure field lengths.
 */

#define AC_DATA_AIRPORT_LEN  8
#define AC_DATA_CALLSIGN_LEN  12

/*
 * Unknown data value.  Used for any data field in the structure whose
 * value is not given.
 */

#define AC_DATA_UNKNOWN_VALUE -9999

/*
 * Structure sizes for use in determining the SPDB chunk header lengths.
 */

#define AC_DATA_STRUCT_BYTES  (sizeof(ac_data_t) - sizeof(ui08))

/*
 * Bits for masking errors in the data.
 */
typedef enum {
    AC_DATA_DataOk        = 0x0000,
    AC_DATA_LatBad        = 0x0001,
    AC_DATA_LonBad        = 0x0002,
    AC_DATA_WindBad       = 0x0004,
    AC_DATA_AltitudeBad   = 0x0008,
    AC_DATA_TurbulenceBad = 0x0010,
    AC_DATA_TempBad       = 0x0020,
    AC_DATA_Etc3          = 0x0040,
    AC_DATA_Etc4          = 0x0080,
    AC_DATA_Etc5          = 0x0100,
    AC_DATA_Etc6          = 0x0200,
    AC_DATA_Etc7          = 0x0400,
    AC_DATA_Etc8          = 0x0800
} AC_DATA_DataQualityBits;


/*
 * Altitude type enumeration.
 *
 * Determines how to interpret the value given as the aircraft
 * altitude.
 */

typedef enum
{
  ALT_NORMAL,         /* normal altitude reading */
  ALT_VFR_ON_TOP,     /* VFR-on-top plus an altitude */
  ALT_INTERIM,        /* interim altitude */
  ALT_AVERAGE,        /* average altitude based on min/max values */
  ALT_TRANSPONDER     /* altitude as determined by transponder */
} ac_data_alt_type_t;


/*
 * Aircraft data structure.
 *
 * Gives all of the data for the aircraft at the given position.
 * In SPDB, this structure will include client_data_len bytes of
 * client data at the end of the structure.  We currently do not
 * use the client data in any of our code so that implementation
 * will have to be done when the need arises.
 */

typedef struct
{
  ui32 alt_type;          /* see ac_data_alt_type_t above */
  ui32 client_data_type;
  ui32 client_data_len;   /* in bytes */
  
  fl32 lat;               /* in deg */
  fl32 lon;               /* in deg */
  fl32 alt;               /* in km, interpreted based on alt_type */
  fl32 ground_speed;      /* in knots */
  fl32 heading;           /* in deg M */

  fl32 temperature;       /* in deg C */
  fl32 dew_point;         /* in deg C */
  fl32 wind_u;            /* in m/sec */
  fl32 wind_v;            /* in m/sec */
  fl32 max_turb;          /* eddy dissipation rate in m**(2/3) / sec */
  fl32 avg_turb;          /* eddy dissipation rate in m**(2/3) / sec */
  ui32 data_quality;      /* Data quality -- masked bits described above */
  
  fl32 spare[5];          /* spare values for later additions */
  
  char callsign[AC_DATA_CALLSIGN_LEN];
  char origin[AC_DATA_AIRPORT_LEN];
  char destination[AC_DATA_AIRPORT_LEN];

  ui08 client_data[1];    /* There will be client_data_len bytes of */
                          /*   data here, to be interpreted according */
                          /*   to client_data_type. */

} ac_data_t;

/********************************************************************
 *
 * ac_data_callsign_hash()
 *
 * Generates a fairly unique, non-zero hash value for the given
 * callsign.
 *
 * Taken from code written by Gerry Wiener, based on code by Knuth.
 */

extern int ac_data_callsign_hash(const char *callsign);


/********************************************************************
 * Byte-swapping routines
 ********************************************************************/

/********************************************************************
 *
 * ac_data_to_BE()
 *
 * Gets BE format from ac_data struct
 *
 */

extern void ac_data_from_BE(ac_data_t *ac_data);

/********************************************************************
 *
 * ac_data_to_BE()
 *
 * Gets BE format from ac_data struct
 *
 */

extern void ac_data_to_BE(ac_data_t *ac_data);

/********************************************************************
 * Printing routines
 ********************************************************************/

extern char *ac_data_alt_type2string(int alt_type);

/********************************************************************
 * ac_data_print
 *
 * Prints out struct
 */

extern void ac_data_print(FILE *out,
			  const char *spacer,
			  ac_data_t *ac_data);


#endif

#ifdef __cplusplus
}
#endif
