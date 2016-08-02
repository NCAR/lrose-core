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
/**************************************************************************
 * partial_track.h
 *
 * header file for partial storm track data
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder CO USA
 *
 * Feb 1997
 *
 **************************************************************************/

#ifndef partial_track_h
#define partial_track_h

#ifdef __cplusplus
extern "C" {
#endif

#include <rapmath/bd_tree.h>
#include <titan/track.h>

#define RF_PARTIAL_INIT_FLAG 9999

typedef struct {

  si32 init_flag;
  si32 past_period;
  si32 future_period;
  si32 n_sparams_alloc;
  si32 tag;
  si32 start_time;
  si32 end_time;
  si32 complex_num;
  si32 simple_num;

  int debug;

  char *prog_name;
  simple_track_params_t *sparams_array;
  track_file_handle_t *t_handle;
  bd_tree_handle_t tree;

} rf_partial_track_t;

/****************************
 * RfInitPartialTrack()
 */

extern void RfInitPartialTrack(rf_partial_track_t *part,
			       track_file_handle_t *t_handle,
			       char *prog_name,
			       int debug);

/**********************
 * RfFreePartialTrack()
 */

extern void RfFreePartialTrack(rf_partial_track_t *part);

/************************
 * RfPrintPartialTrack()
 *
 * print the partial track,
 */

extern void RfPrintPartialTrack(FILE *out,
                                const char *label,
                                const char *space,
                                rf_partial_track_t *ptrack);
     
/**********************
 * RfFindPartialTrack()
 *
 * Find the partial track,
 *
 * returns 0 on success, -1 on failure
 */

extern int RfFindPartialTrack(rf_partial_track_t *part,
			      time_t partial_time,
			      int past_period,
			      int future_period,
			      si32 target_complex_num,
			      si32 target_simple_num);

/********************
 * RfEntryInPartial()
 *
 * Is this entry in the partial track?
 */

extern int RfEntryInPartial(rf_partial_track_t *part,
			    track_file_entry_t *entry);


#ifdef __cplusplus
}
#endif

#endif
