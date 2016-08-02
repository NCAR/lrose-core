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
#ifndef METAR_DECODE
#define METAR_DECODE

#ifdef __cplusplus
  extern "C" {
#endif

#include <rapformats/metar.h>

/********************************************************************/
/*  Abstract:      DcdMETAR takes a pointer to a METAR report char- */
/*                 acter string as input, decodes the report, and   */
/*                 puts the individual decoded/parsed groups into   */
/*                 a structure that has the variable type           */
/*                 Decoded_METAR.                                   */
/*                                                                  */
/*  Input:         string - a pointer to a METAR report character   */
/*                          string.                                 */
/*                 clear_flag - Set to Non Zero to clear out        */
/*                 Decoded_METAR struct.                            */
/*                                                                  */
/*  Output:        Mptr   - a pointer to a structure that has the   */
/*                          variable type Decoded_METAR.            */
/*                                                                  */
/********************************************************************/
 
extern int DcdMETAR( char *string, Decoded_METAR *Mptr, int clear_flag );
      
/********************************************************************/
/*  Abstract:      DcdMTRmk takes a pointer to a METAR              */
/*                 report and parses/decodes data elements from     */
/*                 the remarks section of the report.               */
/*                                                                  */
/*  Input:         token - the address of a pointer to a METAR      */
/*                         report character string.                 */
/*                 Mptr  - a pointer to a structure of the vari-    */
/*                         able type Decoded_METAR.                 */
/*                                                                  */
/********************************************************************/
extern void DcdMTRmk( char **token, Decoded_METAR *Mptr );

#ifdef __cplusplus
}
#endif

#endif


   
