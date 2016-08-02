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
 * zr.h
 *
 * ZR analysis
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * Feb 1996
 *
 **********************************************************************/

#ifndef zr_h
#define zr_h

#ifdef __cplusplus
extern "C" {
#endif

#include <titan/file_io.h>
#include <dataport/port_types.h>

#define ZR_FILE_TYPE "Z-R file type 1"

typedef struct {
  
  si32 n_entries;
  si32 start_time;
  si32 end_time;
  si32 spare;

} zr_header_t;

typedef struct {

  si32 time;
  fl32 coeff;
  fl32 expon;
  fl32 correction;

} zr_entry_t;

/*
 * prototypes
 */

extern int RfAddZrEntry(char *zr_dir,
			zr_entry_t *entry);

extern int RfGetZrClosest(char *zr_dir,
			  time_t req_time, int time_margin,
			  double *coeff_p, double *expon_p);
     
extern int RfGetZrEntry(char *zr_dir,
			si32 entry_time,
			zr_entry_t *entry);

extern int RfPrintZrFile(char *file_path);

#ifdef __cplusplus
}
#endif

#endif

