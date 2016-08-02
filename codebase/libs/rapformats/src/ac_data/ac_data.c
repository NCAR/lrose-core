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
 * ac_data.c
 *
 * Aircraft Position with associated data
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * July 1998
 *
 *********************************************************************/

#include <stdio.h>

#include <dataport/bigend.h>
#include <rapformats/ac_data.h>

/********************************************************************
 * General utility routines
 ********************************************************************/

/********************************************************************
 *
 * ac_data_callsign_hash()
 *
 * Generates a fairly unique, non-zero hash value for the given
 * callsign.
 *
 * Taken from code written by Gerry Wiener, based on code by Knuth.
 */

#define AC_DATA_HASH_MULT 314159
#define  AC_DATA_HASH_PRIME 516595003

int ac_data_callsign_hash(const char *callsign)
{
  int hash;

  for (hash = 0; *callsign; callsign++)
    {
      hash += (hash ^ (hash >> 1)) +  AC_DATA_HASH_MULT *
	(unsigned char)*callsign;
      while (hash >=  AC_DATA_HASH_PRIME)
        hash -=  AC_DATA_HASH_PRIME;
    }

  if (hash == 0)
    hash = 1;

  return(hash);
}


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

void ac_data_to_BE(ac_data_t *ac_data)
{
  BE_from_array_32((void *)ac_data, AC_DATA_NUM_32 * sizeof(fl32));
}

/********************************************************************
 *
 * ac_data_from_BE()
 *
 * Converts BE format to ac_data struct
 *
 */

void ac_data_from_BE(ac_data_t *ac_data)
{
  BE_to_array_32((void *)ac_data, AC_DATA_NUM_32 * sizeof(fl32));
}

/********************************************************************
 * Printing routines
 ********************************************************************/

/********************************************************************
 * ac_data_alt_type2string
 *
 * Converts ac_data_alt_type_t enum type to a string for printing.
 */

char *ac_data_alt_type2string(int alt_type)
{
  switch (alt_type)
  {
  case ALT_NORMAL:
    return("normal altitude");
    
  case ALT_VFR_ON_TOP :
    return("VFR-on-top plus an altitude");
    
  case ALT_INTERIM :
    return("interim altitude");
    
  case ALT_AVERAGE :
    return("average altitude from a block");
    
  case ALT_TRANSPONDER :
    return("transponder altitude");
    
  }
  
  return("*** unknown altitude type ***");
}


/********************************************************************
 * ac_data_print
 *
 * Prints out struct
 */

void ac_data_print(FILE *out,
		   const char *spacer,
		   ac_data_t *ac_data)
{
  fprintf(out, "%s %s %s %s %20.5f lat %20.5f lon %20.5f knots (ground) %20.5f km (%s) %20.5f deg M\n",
	  spacer,
	  ac_data->callsign,
	  ac_data->origin,
	  ac_data->destination,
	  ac_data->lat, ac_data->lon,
	  ac_data->ground_speed,
	  ac_data->alt, ac_data_alt_type2string(ac_data->alt_type),
	  ac_data->heading);

  if (ac_data->temperature != AC_DATA_UNKNOWN_VALUE)
    fprintf(out, "%s    temp = %20.5f deg C\n",
	    spacer, ac_data->temperature);

  if (ac_data->dew_point != AC_DATA_UNKNOWN_VALUE)
    fprintf(out, "%s    dew pt = %20.5f deg C\n",
	    spacer, ac_data->dew_point);

  if (ac_data->wind_u != AC_DATA_UNKNOWN_VALUE)
    fprintf(out, "%s    wind u = %20.5f m/sec\n",
	    spacer, ac_data->wind_u);

  if (ac_data->wind_v != AC_DATA_UNKNOWN_VALUE)
    fprintf(out, "%s    wind v = %20.5f m/sec\n",
	    spacer, ac_data->wind_v);

  if (ac_data->max_turb != AC_DATA_UNKNOWN_VALUE)
    fprintf(out, "%s    max_turb = %20.5f edr\n",
	    spacer, ac_data->max_turb);
  
  if (ac_data->avg_turb != AC_DATA_UNKNOWN_VALUE)
    fprintf(out, "%s    avg_turb = %20.5f edr\n",
	    spacer, ac_data->avg_turb);
  
  if (ac_data->data_quality == AC_DATA_DataOk) {
    fprintf(out, "%s    Data has no quality problems.\n", spacer);
  }
  else {
    fprintf(out, "%s    Data has poor-quality: ", spacer);
    if (ac_data->data_quality & AC_DATA_LatBad)
        fprintf(out, "Latitude ");
    if (ac_data->data_quality & AC_DATA_LonBad)
        fprintf(out, "Longitute ");
    if (ac_data->data_quality & AC_DATA_WindBad)
        fprintf(out, "Wind ");
    if (ac_data->data_quality & AC_DATA_AltitudeBad)
        fprintf(out, "Altitude ");
    if (ac_data->data_quality & AC_DATA_TurbulenceBad)
        fprintf(out, "Turbulence ");

    fprintf(out, "\n");
  }
}

