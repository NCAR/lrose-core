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
 * track.h - header file for storm track data
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder CO USA
 *
 * July 1991
 *
 **************************************************************************/

#ifndef titan_track_h
#define titan_track_h

#ifdef __cplusplus
extern "C" {
#endif

#include <titan/storm.h>
#include <titan/track_v5.h>
#include <titan/track_v6.h>

/*
 * major rev 5, Jan 6 1996
 * minor rev 1, Jan 6 1996
 */

#define TRACK_HEADER_FILE_TYPE TRACK_HEADER_FILE_TYPE_V5
#define TRACK_DATA_FILE_TYPE TRACK_DATA_FILE_TYPE_V5

#define TRACK_HEADER_FILE_EXT TRACK_HEADER_FILE_EXT_V5
#define TRACK_DATA_FILE_EXT TRACK_DATA_FILE_EXT_V5

#define TRACK_FILE_MAJOR_REV TRACK_FILE_MAJOR_REV_V5
#define TRACK_FILE_MINOR_REV TRACK_FILE_MINOR_REV_V5

#define TRACK_FILE_HEADER_NBYTES_CHAR TRACK_FILE_HEADER_NBYTES_CHAR_V5

#define SIMPLE_TRACK 0
#define COMPLEX_TRACK 1
#define PARTIAL_TRACK 2
#define NO_TRACK_TYPE 3

/*
 * max number of simple tracks in a complex track
 */

#define MAX_PARENTS MAX_PARENTS_V5   /* max number of parents allowed per track */
#define MAX_CHILDREN MAX_CHILDREN_V5 /* max number of children allowed per track */

/*
 * the max number of weights for the linear forecasts
 */
  
#define MAX_NWEIGHTS_FORECAST MAX_NWEIGHTS_FORECAST_V5

/* 
 * TRACK FILE FORMATS
 * ------------------
 *
 * There are two files for each date - a header file and a data file.
 *
 * Header file.
 * -----------
 *
 * file label - char[R_FILE_LABEL_LEN]
 *
 * header - struct track_file_header_t
 *
 * si32 complex_track[header->n_complex_tracks]
 *   this stores the numbers of the complex tracks, which are in
 *   ascending order but not necessarily contiguous.
 *
 * si32 complex_track_offset[header->n_complex_tracks]
 *   these are the file offsets to the complex_track_params_t struct
 *   for each complex track.
 *
 * si32 simple_track_offset[header->n_simple_tracks]
 *   these are the file offsets to the simple_track_params_t struct
 *   for each simple track.
 *
 * track_file_scan_index_t scan_index[header->n_scans]
 *   info about the track entry list for a given scan number
 *
 * si32 n_simples_per_complex[header->n_simple_tracks]
 *   This array lists the number of simple tracks per complex track
 *
 * si32 simples_per_complex_offsets[header->n_simple_tracks]
 *   This array lists the offsets for the arrays which contain
 *   the simple track numbers for each complex track
 *
 * For each complex track:
 *
 *   si32 simples_per_complex[n_simples_per_complex[icomplex]]
 *     This array holds the simple track numbers for the
 *     relevant complex track number. The array offsets are
 *     stored in simples_per_complex_offsets.
 *
 * Data file.
 * ---------
 *
 * file label - char[R_FILE_LABEL_LEN] followed by
 * track_data (described below).
 *
 * The track data is made up of the following structs:
 *
 *   complex_track_params_t
 *   simple_track_params_t
 *   track_file_entry_t
 *
 * Each complex track has a complex_track_params_t struct.
 * The offset of this struct is obtained from the complex_track_offset
 * array in the header file.
 * 
 * Each complex track is made up of one or more simple tracks.
 * Each of these has a simple_track_params_t struct pointed at by the
 * simple_track_offset array in the header file.
 *
 * Each simple track is made up of one or more track entries.
 * The entries for a simple track are stored as a doubly-linked list.
 * The offset to the first entry in the list is obtained from the
 * simple_track_params_t struct (first_entry_offset).
 *
 * The entries for a given scan are also stored as a singly-linked list.
 * The offset to the first entry in the list is obtained from the
 * scan index array in the header file. Using this list allows one
 * to get data on all of the entries at a given time (scan).
 *
 * DATA RETRIEVAL
 * --------------
 *
 * A track is made up of a complex track and one or more simple tracks.
 * A simple track is a part of a complex track which has no mergers or
 * splits in it. A complex track is therefore made up of a number of 
 * simple tracks.
 *
 * To retrieve a complex track, find its position in the array by searching
 * for the track number in complex_track_num[].
 *
 * Then read the complex_track_params_t struct at the offset given by
 * complex_track_offset[].
 *
 * Then for each of the simple tracks in the complex track
 * read the simple_track_params_t at the offset
 * given by simple_track_offset[].
 *
 * The first entry in a simple track is pointed to by first_entry_offset
 * in the simple params. Read in the first entry. The entries form a
 * linked list, with the next_entry_offset pointing to the next
 * entry in the simple track.
 *
 * You may also retrieve the entries relevant to a particular scan.
 * The scan_index array contains the number of entries and the offset to
 * the first entry for that scan. Read in the first track entry, and then
 * follow a (second) linked  list to other entries in the scan by using the
 * next_scan_entry_offset values in the entry.
 */

/*
 * forecast types
 */

#define FORECAST_BY_TREND 1
#define FORECAST_BY_PARABOLA 2
#define FORECAST_BY_REGRESSION 3

#define N_MOVEMENT_PROPS_FORECAST N_MOVEMENT_PROPS_FORECAST_V5
#define N_GROWTH_PROPS_FORECAST N_GROWTH_PROPS_FORECAST_V5
#define N_TOTAL_PROPS_FORECAST N_TOTAL_PROPS_FORECAST_V5

#define TRACK_FILE_HEADER_NBYTES_CHAR TRACK_FILE_HEADER_NBYTES_CHAR_V5

/*
 * typedef the current version of the structs
 */
  
typedef track_file_forecast_props_v5_t track_file_forecast_props_t;
typedef track_file_params_v5_t track_file_params_t;
typedef track_file_verify_v5_t track_file_verify_t;
typedef track_file_contingency_data_v5_t track_file_contingency_data_t;
typedef track_file_header_v5_t track_file_header_t;
typedef simple_track_params_v5_t simple_track_params_t;
typedef complex_track_params_v5_t complex_track_params_t;
typedef track_file_entry_v5_t track_file_entry_t;
typedef track_file_scan_index_v5_t track_file_scan_index_t;

/*
 * structure for the arrays which keep track of the start end end times
 * of the simple and complex tracks
 */

typedef struct {

  si32 start_simple;
  si32 end_simple;
  si32 start_complex;
  si32 end_complex;

} track_utime_t;


/*
 * track_file_handle_t is a convenience structure which may be used for
 * referring to any or all component(s) of a track data ile
 */

typedef struct {

  char *prog_name;

  /*
   * the  following must match rf_dual_handle_t
   */

  char *header_file_path;
  char *header_file_label;
  FILE *header_file;
  char *data_file_path;
  char *data_file_label;
  FILE *data_file;
  int handle_initialized;

  /*
   * memory-mapped IO
   */

  int use_mmio;

  /*
   * end of match with rf_dual_handle_t
   */

  char *header_mmio_buf;
  size_t header_mmio_len;
  char *data_mmio_buf;
  size_t data_mmio_len;

  int first_entry;		/* set to TRUE if first entry of a track */

  /*
   * pointers to data
   */

  track_file_header_t *header;

  si32 *complex_track_nums;
  si32 *complex_track_offsets;
  si32 *simple_track_offsets;
  si32 *nsimples_per_complex;
  si32 *simples_per_complex_offsets;
  si32 **simples_per_complex;
  si32 n_scan_entries;

  track_file_scan_index_t *scan_index;
  simple_track_params_t *simple_params;
  complex_track_params_t *complex_params;
  track_file_entry_t *entry;
  track_file_entry_t *scan_entries;
  track_utime_t *track_utime;

  /*
   * memory allocation control
   */

  int header_allocated;
  int simple_params_allocated;
  int complex_params_allocated;
  int entry_allocated;
  int scan_entries_allocated;
  int track_utime_allocated;

  si32 n_scans_allocated;
  si32 n_complex_allocated;
  si32 n_simple_allocated;
  si32 n_simples_per_complex_allocated;
  si32 n_track_utime_allocated;
  si32 n_scan_entries_allocated;

  si32 lowest_avail_complex_slot;

} track_file_handle_t;

/*
 * prototypes
 */

extern void RfInitTrackFileHandle(track_file_handle_t *handle,
				  const char *prog_name);

extern void RfFreeTrackFileHandle(track_file_handle_t *handle);
     
extern int RfAllocComplexTrackParams(track_file_handle_t *t_handle,
				     const char *calling_routine);

extern int RfAllocSimplesPerComplex(track_file_handle_t *t_handle,
				    si32 n_simple_needed,
				    const char *calling_routine);

extern int RfAllocSimpleTrackParams(track_file_handle_t *t_handle,
				    const char *calling_routine);

extern int RfAllocTrackComplexArrays(track_file_handle_t *t_handle,
				     si32 n_complex_needed,
				     const char *calling_routine);

extern int RfAllocTrackEntry(track_file_handle_t *t_handle,
			     const char *calling_routine);

extern int RfAllocTrackHeader(track_file_handle_t *t_handle,
			      const char *calling_routine);

extern int RfAllocTrackScanEntries(track_file_handle_t *t_handle,
				   const char *calling_routine);

extern int RfAllocTrackScanIndex(track_file_handle_t *t_handle,
				 si32 n_scans_needed,
				 const char *calling_routine);

extern int RfAllocTrackSimpleArrays(track_file_handle_t *t_handle,
				    si32 n_simple_needed,
				    const char *calling_routine);

extern void RfAllocTrackUtime(track_file_handle_t *t_handle,
			      const char *calling_routine);

extern int RfCheckTrackMaxValues(track_file_handle_t *t_handle,
				 const char *calling_routine,
				 si32 file_header_max,
				 si32 declared_max,
				 const char *description_small,
				 const char *description_caps);

extern int RfCloseTrackFiles(track_file_handle_t *t_handle,
			     const char *calling_routine);

extern int RfFlushTrackFiles(track_file_handle_t *t_handle,
			     const char *calling_routine);

extern int RfFreeComplexTrackParams(track_file_handle_t *t_handle,
				    const char *calling_routine);

extern int RfFreeSimpleTrackParams(track_file_handle_t *t_handle,
				   const char *calling_routine);

extern int RfFreeTrackArrays(track_file_handle_t *t_handle,
			     const char *calling_routine);

extern int RfFreeTrackEntry(track_file_handle_t *t_handle,
			    const char *calling_routine);

extern int RfFreeTrackHeader(track_file_handle_t *t_handle,
			     const char *calling_routine);

extern int RfFreeTrackScanEntries(track_file_handle_t *t_handle,
				  const char *calling_routine);

extern int RfTrackReinit(track_file_handle_t *t_handle,
			 const char *calling_routine);
     
extern void RfPrintComplexTrackParams(FILE *out,
				      const char *spacer,
				      int verification_performed,
				      const track_file_params_t *params,
				      const complex_track_params_t *cparams,
				      const si32 *simples_per_complex);

extern void RfPrintComplexTrackParamsXML(FILE *out,
				      const char *spacer,
				      int verification_performed,
				      const track_file_params_t *params,
				      const complex_track_params_t *cparams,
				      const si32 *simples_per_complex);

extern void RfPrintContTable(FILE *out,
			     const char *label,
			     const char *spacer,
			     const track_file_contingency_data_t *count);

extern void RfPrintForecastProps(FILE *out,
				 const char *label,
				 const char *space,
				 const track_file_params_t *params,
				 const track_file_forecast_props_t *forecast);

extern void RfPrintForecastPropsXML(FILE *out,
				 const char *label,
				 const char *space,
				 const track_file_params_t *params,
				 const track_file_forecast_props_t *forecast);

extern void RfPrintSimpleTrackParams(FILE *out,
				     const char *spacer,
				     const simple_track_params_t *sparams);

extern void RfPrintSimpleTrackParamsXML(FILE *out,
				     const char *spacer,
				     const simple_track_params_t *sparams);

extern void RfPrintTrackEntry(FILE *out,
			      const char *spacer,
			      si32 entry_num,
			      const track_file_params_t *params,
			      const track_file_entry_t *entry);

extern void RfPrintTrackEntryXML(FILE *out,
			      const char *spacer,
			      si32 entry_num,
			      const track_file_params_t *params,
			      const track_file_entry_t *entry);

extern void RfPrintTrackHeader(FILE *out,
			       const char *spacer,
			       const track_file_header_t *header);

extern void RfPrintTrackHeaderArrays(FILE *out,
				     const char *spacer,
				     const track_file_header_t *header,
				     const si32 *complex_track_nums,
				     const si32 *complex_track_offsets,
				     const si32 *simple_track_offsets,
				     const si32 *nsimples_per_complex,
				     const si32 *simples_per_complex_offsets,
				     const si32 **simples_per_complex,
				     const track_file_scan_index_t *scan_index);
     
extern void RfPrintTrackParams(FILE *out,
			       const char *spacer,
			       const track_file_params_t *params);

extern void RfPrintTrackVerify(FILE *out,
			       const char *spacer,
			       const track_file_verify_t *verify);

extern int RfOpenTrackFiles(track_file_handle_t *t_handle,
			    const char *mode,
			    const char *header_file_path,
			    const char *data_file_ext,
			    const char *calling_routine);

extern int RfReadSimplesPerComplex(track_file_handle_t *t_handle,
				   const char *calling_routine);

extern int RfReadComplexTrackParams(track_file_handle_t *t_handle,
				    si32 track_num,
				    int read_simples_per_complex,
				    const char *calling_routine);

extern int RfReadSimpleTrackParams(track_file_handle_t *t_handle,
				   si32 track_num,
				   const char *calling_routine);

extern int RfReadTrackEntry(track_file_handle_t *t_handle,
			    const char *calling_routine);

extern int RfReadTrackHeader(track_file_handle_t *t_handle,
			     const char *calling_routine);

extern int RfReadTrackScanEntries(track_file_handle_t *t_handle,
				  si32 scan_num,
				  const char *calling_routine);

extern int RfReadTrackUtime(track_file_handle_t *t_handle,
			    const char *calling_routine);

extern int RfReuseTrackComplexSlot(track_file_handle_t *t_handle,
				   si32 track_num,
				   const char *calling_routine);

extern int RfRewindSimpleTrack(track_file_handle_t *t_handle,
			       si32 track_num,
			       const char *calling_routine);

extern int RfRewriteTrackEntry(track_file_handle_t *t_handle,
			       const char *calling_routine);

extern int RfSeekEndTrackData(track_file_handle_t *t_handle,
			      const char *calling_routine);

extern int RfSeekStartTrackData(track_file_handle_t *t_handle,
				const char *calling_routine);

extern int RfWriteComplexTrackParams(track_file_handle_t *t_handle,
				     si32 track_num,
				     const char *calling_routine);

extern int RfWriteSimpleTrackParams(track_file_handle_t *t_handle,
				    si32 track_num,
				    const char *calling_routine);

extern int RfWriteTrackHeader(track_file_handle_t *t_handle,
			      const char *calling_routine);

extern si32 RfWriteTrackEntry(track_file_handle_t *t_handle,
			      si32 prev_in_track_offset,
			      si32 prev_in_scan_offset,
			      const char *calling_routine);

#ifdef __cplusplus
}
#endif

#endif
