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
#ifndef tdata_partial_track_h
#define tdata_partial_track_h

#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************
 * tdata_partial_track.h
 *
 * Track server partial track support.
 *
 ***********************************************************************/
   
#include <titan/tdata_index.h>
#include <rapmath/bd_tree.h>

/*
 * handle struct for partial tracks based on 
 * tdata structs
 */

#define TDATA_PARTIAL_INIT_FLAG 9999

typedef struct {

  si32 init_flag;
  si32 past_period;
  si32 future_period;
  si32 n_basic_alloc;
  si32 n_complete_alloc;
  si32 tag;
  si32 start_time;
  si32 end_time;
  si32 complex_num;
  si32 simple_num;
  
  int debug;

  char *prog_name;

  tdata_basic_simple_params_t *sparams_basic;
  simple_track_params_t *sparams_complete;
  
  bd_tree_handle_t tree;

} tdata_partial_track_t;

/****************************
 * tdata_init_partial_track()
 */

extern void tdata_init_partial_track(tdata_partial_track_t *part,
				     char *prog_name,
				     int debug);

/****************************
 * tdata_free_partial_track()
 */

extern void tdata_free_partial_track(tdata_partial_track_t *part);

/************************************
 * tdata_find_basic_partial_track()
 *
 * Find the partial track,
 * based on basic data from the server.
 *
 * returns 0 on success, -1 on failure
 */

extern int
tdata_find_basic_partial_track (tdata_partial_track_t *part,
				time_t partial_time,
				int past_period,
				int future_period,
				int target_complex_num,
				int target_simple_num,
				tdata_basic_with_params_index_t *tdata_index);

/************************************
 * tdata_find_partial_track(),
 *
 * Find the partial track,
 * based on complete data from the server.
 *
 * returns 0 on success, -1 on failure
 */

extern int
  tdata_find_partial_track (tdata_partial_track_t *part,
			    time_t partial_time,
			    int past_period,
			    int future_period,
			    int target_complex_num,
			    int target_simple_num,
			    tdata_complete_index_t *tdata_index);

/********************************
 * tdata_entry_in_partial()
 *
 * Is this entry in the partial track,
 * based on complete data from the server?
 */

extern int
tdata_entry_in_partial(tdata_partial_track_t *part,
		       track_file_entry_t *entry);

/********************************
 * tdata_basic_entry_in_partial()
 *
 * Is this entry in the partial track,
 * based on basic data from the server?
 */

extern int
tdata_basic_entry_in_partial(tdata_partial_track_t *part,
			     tdata_basic_track_entry_t *entry);

#ifdef __cplusplus
}
#endif

#endif

