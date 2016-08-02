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
 * storm.h - header file for storm structures
 *
 * Mike Dixon RAP NCAR August 1990
 *
 **************************************************************************/

#ifndef titan_storm_h
#define titan_storm_h
      
#ifdef __cplusplus
extern "C" {
#endif

#include <titan/file_io.h>
#include <rapformats/titan_grid.h>
#include <titan/titan_hail.h>
#include <titan/storm_v5.h>
#include <titan/storm_v6.h>

#define STORM_HEADER_FILE_TYPE STORM_HEADER_FILE_TYPE_V5
#define STORM_DATA_FILE_TYPE STORM_DATA_FILE_TYPE_V5

#define STORM_HEADER_FILE_EXT STORM_HEADER_FILE_EXT_V5
#define STORM_DATA_FILE_EXT STORM_DATA_FILE_EXT_V5

#define STORM_FILE_MAJOR_REV STORM_FILE_MAJOR_REV_V5
#define STORM_FILE_MINOR_REV STORM_FILE_MINOR_REV_V5

#define STORM_FILE_HEADER_NBYTES_CHAR STORM_FILE_HEADER_NBYTES_CHAR_V5

/*
 * defines
 */

#define MISSING_VALUE -9999.0

/*
 * precipitation mode
 */

#define TITAN_PRECIP_FROM_COLUMN_MAX 0
#define TITAN_PRECIP_AT_SPECIFIED_HT 1
#define TITAN_PRECIP_AT_LOWEST_VALID_HT 2
#define TITAN_PRECIP_FROM_LOWEST_AVAILABLE_REFL 3

/*
 * Storm file formats.
 *
 * There are two files for each date - a header file and a data file.
 *
 * Header file.
 * -----------
 *
 * file label - char[R_FILE_LABEL_LEN]
 *
 * header - struct storm_file_header_t
 *
 * si32 scan_offset[header->n_scans] - file offset of each scan header
 *
 * Data file.
 * ---------
 *
 * file label - char[R_FILE_LABEL_LEN]
 *
 * for each scan :
 *
 *  at scan_offset[iscan]:
 *    struct storm_file_scan_header_t
 *
 *  followed by
 *
 *  at scan_header->gprops_offset:
 *    array of structs storm_file_global_props_t[nstorms],
 *      where nstorms is scan_header.nstorms
 *
 *  followed by
 *
 *  for each storm in the scan :
 *
 *    at gprops[istorm].layer_props_offset:
 *      array of structs storm_file_layer_props_t[n_layers],
 *        where n_layers is gprops[istorm].n_layers
 * 
 *    at gprops[istorm].dbz_hist_offset:
 *      array of structs storm_file_dbz_hist_t[n_dbz_intervals],
 *        where n_dbz_intervals is gprops[istorm].n_dbz_intervals
 *
 *    at gprops[istorm].runs_offset:
 *      array of structs storm_file_run_t[n_runs],
 *        where n_runs is gprops[istorm].n_runs
 *
 *    at gprops[istorm].proj_runs_offset:
 *      array of structs storm_file_run_t[n_proj_runs],
 *        where n_runs is gprops[istorm].n_proj_runs
 *
 * NOTE 1.
 * 
 * The offsets of scan headers in the file are stored in the array
 * scan_offset[].
 *
 * The global props for the storms are in the 'storm_file_global_props_t'
 * array, which are pointed to by gprops_offset in the scan header.
 *
 * The position of the two arrays of types 'storm_file_layer_props_t',
 * 'storm_file_dbz_hist_t' and storm_file_run_t are given by the entries
 * 'layer_props_offset', 'dbz_hist_offset', 'runs_offset' and
 * 'proj_runs_offset'.
 *
 * It is therefore possible to move reasonably directly to the data
 * for a particular storm, knowing the scan number and the storm number.
 *
 * NOTE 2.
 *
 * Integer values are stored as IEEE 32-bit intergers.
 * Floating point values are stored as IEEE 32-bit floats.
 * All ints and floats are stored in big-endian format.
 *
 * NOTE 3.
 *
 * The storm data is based on two types of data grid, a local-area
 * flat grid, and a lot-lon grid. The grid type is either TITAN_PROJ_FLAT
 * or TITAN_PROJ_LATLON.
 *
 * The position and shape properties of the storms are in either
 * in km or degrees, depending upon which grid was used.
 * Units are in km for flat grid, degrees for lat-lon grid.
 *
 * The area, volume and speed properties are always in km units.
 */

/*
 * typedef the structs from the current version number
 */

typedef storm_file_params_v5_t storm_file_params_t;
typedef storm_file_header_v5_t storm_file_header_t;
typedef storm_file_scan_header_v5_t storm_file_scan_header_t;
typedef storm_file_global_props_v5_t storm_file_global_props_t;
typedef storm_file_layer_props_v5_t storm_file_layer_props_t;
typedef storm_file_dbz_hist_v5_t storm_file_dbz_hist_t;
typedef storm_file_run_v5_t storm_file_run_t;

/*
 * storm_file_handle_t is a convenience structure which may be used for
 * referring to any or all component(s) of the storm properties file
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

  storm_file_header_t *header;
  si32 *scan_offsets;
  storm_file_scan_header_t *scan;
  storm_file_global_props_t *gprops;
  storm_file_layer_props_t *layer;
  storm_file_dbz_hist_t *hist;
  storm_file_run_t *runs;
  storm_file_run_t *proj_runs;
  si32 storm_num;

  /*
   * the following are used to keep track of allocated memory
   */

  int header_allocated;
  int scan_allocated;
  int props_allocated;
  si32 n_scans_allocated;
  si32 max_storms;
  si32 max_layers;
  si32 max_dbz_intervals;
  si32 max_runs;
  si32 max_proj_runs;

} storm_file_handle_t;

/*
 * semaphore constants for storm_ident and storm_track
 */

typedef enum {
  STORM_TRACK_ACTIVE_SEM,
  STORM_IDENT_QUIT_SEM,
  N_STORM_IDENT_SEMS
} storm_ident_sem_t;

/*
 * flags for tracking mode
 */

#define RETRACK_FILE -1
#define PREPARE_NEW_FILE -2
#define PREPARE_FOR_APPEND -3
#define TRACK_LAST_SCAN -4

/*
 * shared memory for storm_ident and storm_track
 */

typedef struct {

  int auto_restart;     	/* set by storm_ident on exit - if set
				 * to TRUE, storm_track knows that the
				 * exit is for auto restart */

  int remove_track_file;	/* set by storm_ident on exit - if set
				 * to TRUE, storm_track will remove the
				 * track file before exiting */

  int tracking_mode;		/* set by storm_ident to indicate the
				 * tracking mode to storm_track
				 * PREPARE_NEW_FILE or
				 * PREPARE_FOR_APPEND or
				 * TRACK_LAST_SCAN
				 *
				 * Note: RETRACK mode is set by
				 * storm_track itself, since storm_ident
				 * is not run in retrack mode.
				 */
  
  char storm_header_file_path[MAX_PATH_LEN];
  char storm_data_dir[MAX_PATH_LEN];

} storm_tracking_shmem_t;

/*
 * prototypes
 */
 
extern void RfInitStormFileHandle(storm_file_handle_t *handle,
				  const char *prog_name);

extern void RfFreeStormFileHandle(storm_file_handle_t *handle);

extern int RfAllocStormHeader(storm_file_handle_t *s_handle,
			      const char *calling_routine);

extern int RfAllocStormProps(storm_file_handle_t *s_handle,
			     si32 n_layers,
			     si32 n_dbz_intervals,
			     si32 n_runs,
			     si32 n_proj_runs,
			     const char *calling_routine);

extern int RfAllocStormScan(storm_file_handle_t *s_handle,
			    si32 nstorms,
			    const char *calling_routine);

extern int RfAllocStormScanOffsets(storm_file_handle_t *s_handle,
				   si32 nscans_needed,
				   const char *calling_routine);

extern int RfCloseStormFiles(storm_file_handle_t *s_handle,
			     const char *calling_routine);

extern int RfCopyStormScan(storm_file_handle_t *s_handle1,
			   storm_file_handle_t *s_handle2);

extern int RfFlushStormFiles(storm_file_handle_t *s_handle,
			     const char *calling_routine);

extern int RfFreeStormHeader(storm_file_handle_t *s_handle,
			     const char *calling_routine);

extern int RfFreeStormProps(storm_file_handle_t *s_handle,
			    const char *calling_routine);

extern int RfFreeStormScan(storm_file_handle_t *s_handle,
			   const char *calling_routine);

extern int RfFreeStormScanOffsets(storm_file_handle_t *s_handle,
				  const char *calling_routine);

extern void RfPrintStormHeader(FILE *out,
			       const char *spacer,
			       const storm_file_header_t *header);

extern void RfPrintStormHist(FILE *out,
			     const char *spacer,
			     const storm_file_params_t *params,
			     const storm_file_global_props_t *gprops,
			     const storm_file_dbz_hist_t *hist);

extern void RfPrintStormHistXML(FILE *out,
			     const char *spacer,
			     const storm_file_params_t *params,
			     const storm_file_global_props_t *gprops,
			     const storm_file_dbz_hist_t *hist);

extern void RfPrintStormLayer(FILE *out,
			      const char *spacer,
			      const storm_file_params_t *params,
			      const storm_file_scan_header_t *scan,
			      const storm_file_global_props_t *gprops,
			      const storm_file_layer_props_t *layer);

extern void RfPrintStormLayerXML(FILE *out,
			      const char *spacer,
			      const storm_file_params_t *params,
			      const storm_file_scan_header_t *scan,
			      const storm_file_global_props_t *gprops,
			      const storm_file_layer_props_t *layer);

extern void RfPrintStormParams(FILE *out,
			       const char *spacer,
			       const storm_file_params_t *params);

extern void RfPrintStormProps(FILE *out,
			      const char *spacer,
			      const storm_file_params_t *params,
			      const storm_file_scan_header_t *scan,
			      const storm_file_global_props_t *gprops);

extern void RfPrintStormPropsXML(FILE *out,
			      const char *spacer,
			      const storm_file_params_t *params,
			      const storm_file_scan_header_t *scan,
			      const storm_file_global_props_t *gprops);

extern void RfPrintStormProjRuns(FILE *out,
				 const char *spacer,
				 const storm_file_global_props_t *gprops,
				 const storm_file_run_t *proj_runs);

extern void RfPrintStormProjRunsXML(FILE *out,
                                    const char *spacer,
                                    const storm_file_global_props_t *gprops,
                                    const storm_file_run_t *proj_runs);

extern void RfPrintStormRuns(FILE *out,
			     const char *spacer,
			     const storm_file_global_props_t *gprops,
			     const storm_file_run_t *runs);

extern void RfPrintStormRunsXML(FILE *out,
                                const char *spacer,
                                const storm_file_global_props_t *gprops,
                                const storm_file_run_t *runs);

extern void RfPrintStormScan(FILE *out,
			     const char *spacer,
			     const storm_file_params_t *params,
			     const storm_file_scan_header_t *scan);

extern void RfPrintStormScanXML(FILE *out,
                                const char *spacer,
                                const storm_file_params_t *params,
                                const storm_file_scan_header_t *scan);

extern int RfOpenStormFiles(storm_file_handle_t *s_handle,
                            const char *mode,
                            char *header_file_path,
                            const char *data_file_ext,
                            const char *calling_routine);

extern int RfReadStormHeader(storm_file_handle_t *s_handle,
			     const char *calling_routine);

extern int RfReadStormProjRuns(storm_file_handle_t *s_handle,
			       si32 storm_num,
			       const char *calling_routine);

extern int RfReadStormProps(storm_file_handle_t *s_handle,
			    si32 storm_num,
			    const char *calling_routine);

extern int RfReadStormScan(storm_file_handle_t *s_handle,
			   si32 scan_num,
			   const char *calling_routine);

extern int RfSeekEndStormData(storm_file_handle_t *s_handle,
			      const char *calling_routine);

extern int RfSeekStartStormData(storm_file_handle_t *s_handle,
				const char *calling_routine);

extern int RfWriteStormHeader(storm_file_handle_t *s_handle,
			      const char *calling_routine);

extern int RfWriteStormProps(storm_file_handle_t *s_handle,
			     si32 storm_num,
			     const char *calling_routine);

extern int RfWriteStormScan(storm_file_handle_t *s_handle,
			    si32 scan_num,
			    const char *calling_routine);

extern void RfStormGpropsEllipses2Km(storm_file_scan_header_t *scan,
				     storm_file_global_props_t *gprops);

extern void RfStormGpropsXY2LatLon(storm_file_scan_header_t *scan,
				   storm_file_global_props_t *gprops);

#ifdef __cplusplus
}
#endif

#endif

