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
 * ac_posn.h
 *
 * structs and defines for aircraft position
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO
 *
 * August 1996
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ac_posn_h
#define ac_posn_h

#include <dataport/port_types.h>

#define AC_POSN_PROD_CODE 20001
#define AC_POSN_PROD_LABEL "AC_POSN"
#define AC_POSN_N_CALLSIGN 12
#define AC_POSN_N_FL32 3
#define AC_POSN_MISSING_FLOAT -9999.0

typedef struct {

  fl32 lat;
  fl32 lon;
  fl32 alt;
  char callsign[AC_POSN_N_CALLSIGN];

} ac_posn_t;

/*
 * Version for weather modification operations.
 * ac_posn_t maps onto the top 24 bytes of this struct.
 * Struct length: 128
 */

#define AC_POSN_WMOD_PROD_CODE 20003
#define AC_POSN_WMOD_PROD_LABEL "AC_POSN_WMOD"
#define AC_POSN_WMOD_N_TEXT 16
#define AC_POSN_WMOD_N_32 14

/* flare flag bit-wise defines */

#define RIGHT_BURN_FLAG      0x01
#define LEFT_BURN_FLAG       0x02
#define BURN_IN_PLACE_FLAG   0x04
#define EJECTABLE_FLAG       0x08
#define DRY_ICE_FLAG         0x10

typedef struct {

  fl32 lat;
  fl32 lon;
  fl32 alt;

  char callsign[AC_POSN_N_CALLSIGN];
  char text[AC_POSN_WMOD_N_TEXT];

  fl32 tas;
  fl32 gs;
  fl32 temp;
  fl32 dew_pt;
  fl32 lw;
  fl32 fssp;
  fl32 rosemount;
  fl32 headingDeg;
  fl32 spare_fl32[2];
  
  si32 flare_flags;      /* bit-wise flag for flare activity */
  si32 n_ejectable;      /* number of ejectables so far */
  si32 n_burn_in_place;  /* number of burn-in-place so far */
  si32 spare_si32;

} ac_posn_wmod_t;

/*
 * prototypes
 */

extern void BE_from_ac_posn(ac_posn_t *posn);

extern void BE_to_ac_posn(ac_posn_t *posn);

extern void BE_from_ac_posn_wmod(ac_posn_wmod_t *posn);

extern void BE_to_ac_posn_wmod(ac_posn_wmod_t *posn);

extern void ac_posn_print(FILE *out,
			  const char *spacer,
			  ac_posn_t *ac_posn);


extern void ac_posn_wmod_print(FILE *out,
			       const char *spacer,
			       ac_posn_wmod_t *ac_posn);


#endif

#ifdef __cplusplus
}
#endif
