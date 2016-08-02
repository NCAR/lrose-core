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
/*********************************************
 * ds_message.c
 *
 * C routines for DsMessage structs
 *
 * Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
 *
 * May 1998
 */

#include <dataport/bigend.h>
#include <didss/ds_message.h>

/*
 * BE swapping routines
 */

/*******************
 * BE_to_DsMsgHdr()
 *
 * Convert BE to DsMsgHdr_t
 */

void BE_to_DsMsgHdr(DsMsgHdr_t *hdr)
     
{
  BE_to_array_32(hdr, sizeof(DsMsgHdr_t));
}

/*******************
 * BE_from_DsMsgHdr()
 *
 * Convert DsMsgHdr_t to BE
 */

void BE_from_DsMsgHdr(DsMsgHdr_t *hdr)

{
  BE_from_array_32(hdr, sizeof(DsMsgHdr_t));
}

/*******************
 * BE_to_DsMsgPart()
 *
 * Convert BE to DsMsgPart_t
 */

void BE_to_DsMsgPart(DsMsgPart_t *part)

{
  BE_to_array_32(part, sizeof(DsMsgPart_t));
}

/***************************
 * BE_from_DsMsgPart()
 *
 * Convert DsMsgPart_t to BE
 */

void BE_from_DsMsgPart(DsMsgPart_t *part)

{
  BE_from_array_32(part, sizeof(DsMsgPart_t));
}
