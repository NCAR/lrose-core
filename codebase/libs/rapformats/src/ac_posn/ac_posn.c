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
 * AcPosn.c
 *
 * Aircraft Position data
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * August 1996
 *
 *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <dataport/bigend.h>
#include <rapformats/ac_posn.h>

/********************************************************************
 *
 * BE_from_ac_posn()
 *
 * Gets BE format from ac posn struct
 *
 *********************************************************************/

void BE_from_ac_posn(ac_posn_t *posn)

{
  BE_from_array_32((void *) posn, AC_POSN_N_FL32 * sizeof(fl32));
}

/********************************************************************
 *
 * BE_to_ac_posn()
 *
 * Converts BE format to ac_posn struct
 *
 *********************************************************************/

void BE_to_ac_posn(ac_posn_t *posn)

{
  BE_to_array_32((void *) posn, AC_POSN_N_FL32 * sizeof(fl32));
}

/********************************************************************
 *
 * BE_from_ac_posn_wmod()
 *
 * Gets BE format from ac_posn_wmod struct
 *
 *********************************************************************/

void BE_from_ac_posn_wmod(ac_posn_wmod_t *posn)

{
  BE_from_array_32((void *) posn, AC_POSN_N_FL32 * sizeof(fl32));
  BE_from_array_32(&posn->tas, AC_POSN_WMOD_N_32 * sizeof(fl32));
}

/********************************************************************
 *
 * BE_to_ac_posn_wmod()
 *
 * Converts BE format to ac_posn_wmod struct
 *
 *********************************************************************/

void BE_to_ac_posn_wmod(ac_posn_wmod_t *posn)

{
  BE_to_array_32((void *) posn, AC_POSN_N_FL32 * sizeof(fl32));
  BE_to_array_32(&posn->tas, AC_POSN_WMOD_N_32 * sizeof(fl32));
}


/****************************
 * ac_posn_print
 *
 * Prints out ac_posn struct
 */

void ac_posn_print(FILE *out,
		   const char *spacer,
		   ac_posn_t *ac_posn)

{

  fprintf(out, "%s ac_posn %20.5f %20.5f %20.5f %s\n", spacer,
	  ac_posn->lat, ac_posn->lon,
	  ac_posn->alt, ac_posn->callsign);

}

/****************************
 * ac_posn_wmod_print
 *
 * Prints out ac_posn_wmod struct
 */

void ac_posn_wmod_print(FILE *out,
			const char *spacer,
			ac_posn_wmod_t *ac_posn)

{

  fl32 missing = AC_POSN_MISSING_FLOAT;
  fprintf(out, "%s ac_posn_wmod_t struct:\n", spacer);
  fprintf(out, "%s callsign: %s\n", spacer, ac_posn->callsign);
  if (strlen(ac_posn->text) > 0) {
    fprintf(out, "%s   text: %s\n", spacer, ac_posn->text);
  }
  if (ac_posn->lat != missing) {
    fprintf(out, "%s   lat: %10.5f\n", spacer, ac_posn->lat);
  }
  if (ac_posn->lon != missing) {
    fprintf(out, "%s   lon: %10.5f\n", spacer, ac_posn->lon);
  }
  if (ac_posn->alt != missing) {
    fprintf(out, "%s   alt: %10.5f\n", spacer, ac_posn->alt);
  }
  if (ac_posn->tas != missing) {
    fprintf(out, "%s   tas: %g\n", spacer, ac_posn->tas);
  }
  if (ac_posn->gs != missing) {
    fprintf(out, "%s   gs: %g\n", spacer, ac_posn->gs);
  }
  if (ac_posn->temp != missing) {
    fprintf(out, "%s   temp: %g\n", spacer, ac_posn->temp);
  }
  if (ac_posn->dew_pt != missing) {
    fprintf(out, "%s   dew_pt: %g\n", spacer, ac_posn->dew_pt);
  }
  if (ac_posn->lw != missing) {
    fprintf(out, "%s   lw: %g\n", spacer, ac_posn->lw);
  }
  if (ac_posn->fssp != missing) {
    fprintf(out, "%s   fssp: %g\n", spacer, ac_posn->fssp);
  }
  if (ac_posn->rosemount != missing) {
    fprintf(out, "%s   rosemount: %g\n", spacer, ac_posn->rosemount);
  }
  if (ac_posn->headingDeg != missing) {
    fprintf(out, "%s   headingDeg: %g\n", spacer, ac_posn->headingDeg);
  }
  if (ac_posn->flare_flags & RIGHT_BURN_FLAG) {
    fprintf(out, "%s   right_burn: ON\n", spacer);
  }
  if (ac_posn->flare_flags & LEFT_BURN_FLAG) {
    fprintf(out, "%s   left_burn: ON\n", spacer);
  }
  if (ac_posn->flare_flags & BURN_IN_PLACE_FLAG) {
    fprintf(out, "%s   burn_in_place: ON\n", spacer);
  }
  if (ac_posn->flare_flags & EJECTABLE_FLAG) {
    fprintf(out, "%s   ejectable: ON\n", spacer);
  }
  if (ac_posn->flare_flags & DRY_ICE_FLAG) {
    fprintf(out, "%s   dry_ice: ON\n", spacer);
  }
  if (ac_posn->n_ejectable > 0) {
    fprintf(out, "%s   n_ejectable: %d\n", spacer, ac_posn->n_ejectable);
  }
  if (ac_posn->n_burn_in_place > 0) {
    fprintf(out, "%s   n_burn_in_place: %d\n", spacer,
	    ac_posn->n_burn_in_place);
  }
  
}

