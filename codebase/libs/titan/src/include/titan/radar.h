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
 * titan/radar.h
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, Sept 1990
 *
 **********************************************************************/

#ifndef titan_radar_h
#define titan_radar_h

#ifdef __cplusplus
extern "C" {
#endif

#include <toolsa/umisc.h>
#include <rapformats/titan_grid.h>
#include <titan/scan_table.h>
#include <titan/dobson.h>

/*
 * current file labels
 */

#define RADAR_VOLUME_FILE_TYPE "Radar volume file type 2"
#define RADAR_TO_CART_TABLE_FILE "Radar-to-cartesian table"
#define RADAR_TO_CART_SLAVE_TABLE_FILE "Radar-to-cartesian slave table"
#define CLUTTER_TABLE_FILE "Clutter table"

/*
 * vol_file_handle_t is a convenience structure which may be used for
 * referring to any or all component(s) of the radar volume file
 */

typedef struct {
  
  char *prog_name;
  char *vol_file_path;
  char *vol_file_label;
  FILE *vol_file;
  int handle_initialized;
  
  vol_params_t *vol_params;
  
  si32 *radar_elevations;     /* deg */
  si32 **plane_heights;       /* km */
  
  si32 *field_params_offset;	/* file offset of field params */
  
  field_params_t **field_params;
  
  si32 **plane_offset;
  si32 **plane_allocated;
  
  ui08 ***field_plane;
  
  /*
   * the following are used to keep track of allocated memory
   */
  
  int params_allocated;
  int arrays_allocated;
  
  si32 nelevations_allocated;
  si32 nfields_allocated;
  si32 nplanes_allocated;
  
} vol_file_handle_t;

/*
 * rc table params
 */

typedef struct {

  si32 nbytes_char;		/* number of character bytes at the
				 * end of this struct */
  
  si32 use_azimuth_table;	/* flag to indicate whether a
				 * variable azimuth table is used */

  si32 extend_below;		/* flag to indicate whether the
				 * data are extended below the bottom
				 * elevation angle */

  si32 missing_data_index;

  si32 nelevations;
  si32 nazimuths;
  si32 ngates;
  si32 nbeams_vol;

  si32 delta_azimuth;		/* degrees * 1000000 */
  si32 start_azimuth;		/* degrees * 1000000 */
  si32 beam_width;		/* degrees * 1000000 */

  si32 start_range;		/* millimeters */
  si32 gate_spacing;		/* millimeters */

  si32 radar_latitude;	/* deg * 1000000 */
  si32 radar_longitude;	/* deg * 1000000 */

  si32 ndata;			/* total number of points in grid */
  si32 nlist;			/* size of list */

  si32 index_offset;		/* byte offset of table_index array */
  si32 list_offset;		/* byte offset of list array */

  cart_params_t cart;		/* cartesian parameters */

} rc_table_params_t;

/*
   * the rc_table entries hold the number of points (npoints), the gate number
   * and the offset relative to the start of the rc_table start (rel_offset)
   * of each set of cartesian offsets mapped to by the radar point
   *
   * used for translating radar to cartesian coords
   */

typedef struct {

  ui32 gate;
  ui32 index;

} rc_table_entry_t;

/*
   * rc_table_index array holds an index of the rc_table entries in the file
   * used for translating radar to cartesian coords
   */

typedef struct {

  ui16 npoints;
  ui16 last_gate_active;

  union {
    ui32 offset;
    rc_table_entry_t *entry;
  } u;

} rc_table_index_t;

/*
 * rc_table_file_handle_t is a convenience structure which may be used for
 * referring to any or all component(s) of the radar-to-cart table file
 */

typedef struct {

  char *prog_name;
  char *file_path;
  char *file_label;
  FILE *file;
  int handle_initialized;

  rc_table_params_t *table_params;
  rc_table_index_t **table_index;
  scan_table_t *scan_table;
  char *list;

  si32 **plane_heights;

} rc_table_file_handle_t;

/*
 * slave_table_index array holds an index of the slave_table entries.
 * The slave_table is used to duplicate the cartesianizing process on
 * a remote machine
 */

typedef struct {

  ui32 npoints;

  union {
    ui32 offset;
    si32 *index;
  } u;

} slave_table_index_t;

/*
 * slave_table_file_handle_t is a convenience structure which may be
 * used for referring to any or all component(s) of the radar-to-cart
 * slave table file
 */

typedef struct {

  char *prog_name;
  char *file_path;
  char *file_label;
  FILE *file;
  int handle_initialized;

  rc_table_params_t *table_params;
  slave_table_index_t **table_index;
  scan_table_t *scan_table;
  char *list;

  si32 **plane_heights;

} slave_table_file_handle_t;

/*
 * clutter table params
 */

typedef struct {

  si32 nbytes_char;		/* number of bytes of character data
				 * at the end of this struct */

  si32 file_time;

  si32 start_time;            /* start, mid and end times for data */
  si32 mid_time;
  si32 end_time;
  
  si32 factor;		/* mulitplier for scale, bias & margin */

  si32 dbz_scale;		/* scale for dbz values */
  si32 dbz_bias;		/* bias for dbz values */
  si32 dbz_margin;		/* margin between clutter dbz data
				 * and the acceptable value from
				 * a data file */

  si32 nlist;			/* length of list */

  si32 index_offset;		/* byte offset of table_index array */
  si32 list_offset;		/* byte offset of list array */

  rc_table_params_t rc_params; /* copy of the rc_params struct used
				* to create the clutter table */

} clutter_table_params_t;

/*
 * the clutter_table entries hold the dbz value for the point,
 * and the cartesian point number for the beam
 */

typedef struct {

  ui16 dbz;
  ui16 ipoint;
  ui32 cart_index;

} clutter_table_entry_t;

/*
 * clutter_table_index array holds an index of the clutter_table
 * entries in the clutter table array
 */

typedef struct {

  ui32 nclut_points;

  union {
    ui32 offset;
    clutter_table_entry_t *entry;
  } u;

} clutter_table_index_t;

/*
 * clutter_table_file_handle_t is a convenience structure which may be used
 * for referring to any or all component(s) of a clutter table file
 */

typedef struct {

  char *prog_name;
  char *file_path;
  char *file_label;
  FILE *file;
  int handle_initialized;

  clutter_table_params_t *table_params;
  clutter_table_index_t **table_index;
  char *list;

  si32 *radar_elevations;	/* deg */
  si32 **plane_heights;	/* km */

  /*
   * the following are used to keep track of allocated memory
   */

  int params_allocated;
  int elev_allocated;
  int planeht_allocated;
  int index_allocated;
  int list_allocated;

  si32 nelevations_allocated;
  si32 nazimuths_allocated;
  si32 nplanes_allocated;
  si32 nlist_allocated;

} clutter_table_file_handle_t;

/*
 * packet codes for polar2mdv and cart_slave
 */

#define VOL_PARAMS_PACKET_CODE 0
#define FIELD_PARAMS_PACKET_CODE 1
#define CART_DATA_PACKET_CODE 2
#define END_OF_VOLUME_PACKET_CODE 3

/*
 * semaphore constants for polar_ingest program and its clients
 */

#define N_POLAR_INGEST_SEMS 4
#define POLAR_INGEST_NOT_READY_SEM 0
#define POLAR_INGEST_QUIT_SEM 1
#define POLAR2MDV_ACTIVE_SEM 2
#define POLAR2MDV_QUIT_SEM 3

/*
 * header for shared memory for polar_ingest and polar2mdv
 */

typedef struct {

  double noise_dbz_at_100km;	/* noise level in dbz at 100km */

  si32 nfields_in;		/* number of fields in the input
				 * data stream */

  si32 ngates;		/* number of gates in the beam buffer */

  si32 nbeams_buffer;		/* number of beams in the rotating
				 * beam buffer */

  si32 buffer_size;		/* buffer size in bytes */

  si32 field_params_offset;	/* buffer offset to the field params */
  si32 beam_headers_offset;	/* buffer offset to the beam headers
				 * array */

  si32 data_field_by_field;	/* TRUE if radar field data is stored
				 * as contiguous blocks, FALSE if each
				 * field for a given gate is stored
				 * consecutively followed by the fields
				 * for the next gate etc. */

  si32 dbz_field_pos;		/* number of the dbz field (-1 if none) */

  radar_params_t radar;	/* the radar parameters */

  si32 last_beam_written;	/* the last beam written to the buffer */
  
  si32 last_beam_used;	/* the last beam read from the buffer */

  int late_end_of_vol;	/* set by polar_ingest to indicate that
			 * an end_of_vol flag has been placed
			 * in a beam later that when the beam
			 * was originally written to shared
			 * memory */

  char note[VOL_PARAMS_NOTE_LEN]; /* note on processing so far */

} rdata_shmem_header_t;

/*
 * header for shared memory beams for polar_ingest and
 * polar2mdv
 */

typedef struct {

  double azimuth;		/* azimuth (deg) */
  double elevation;		/* elevation (deg) */
  double target_elev;		/* target elevation (deg) */

  si32 beam_time;

  si32 vol_num;		/* the volume number */
  si32 tilt_num;		/* the tilt number in the
				 * volume sequence */

  si32 nbeams_missing;	/* number of beams missing immediately
			 * after this one */

  si32 field_data_offset;	/* offset to field data */

  int scan_mode;		/* DIX_PPI_MODE, DIX_RHI_MODE,
                                 * DIX_SECTOR_MODE or DIX_UNKNOWN_MODE */

  int end_of_tilt;		/* flag - TRUE or FALSE */
  int end_of_volume;		/* flag - TRUE or FALSE */
  int new_scan_limits;	/* flag - TRUE or FALSE */

} rdata_shmem_beam_header_t;

/*
 * cart slave shared memory
 */

typedef struct {

  vol_params_t vol_params;

  si32 aux_header_size;
  si32 nbytes_per_field;

  si32 radar_elevations_offset;
  si32 plane_heights_offset;
  si32 field_params_offset;

  si32 spare;

} cart_slave_shmem_header_t;

/*
 * headers for beam packets - polar2mdv/cart_slave
 */

typedef struct {

  si32 beam_time;
  si32 nbeams;

} beam_packet_header_t;

typedef struct {

  si32 az_num;
  si32 elev_num;
  si32 npoints;
  si32 data_offset;
  si32 new_scan_limits;	/* flag - TRUE or FALSE */
  si32 spare;

} beam_subpacket_header_t;

/*
 * prototypes
 */

extern void RfInitVolFileHandle(vol_file_handle_t *handle,
				const char *prog_name,
				const char *file_name,
				FILE *fd);

extern void RfFreeVolFileHandle(vol_file_handle_t *handle);

extern void RfInitClutterHandle(clutter_table_file_handle_t *handle,
				const char *prog_name,
				const char *file_name,
				FILE *fd);

extern void RfFreeClutterHandle(clutter_table_file_handle_t *handle);

extern void RfInitRcTableHandle(rc_table_file_handle_t *handle,
				const char *prog_name,
				const char *file_name,
				FILE *fd);

extern void RfFreeRcTableHandle(rc_table_file_handle_t *handle);

extern void RfInitSlaveTableHandle(slave_table_file_handle_t *handle,
				   const char *prog_name,
				   const char *file_name,
				   FILE *fd);

extern void RfFreeSlaveTableHandle(slave_table_file_handle_t *handle);

extern int RfAllocVolArrays(vol_file_handle_t *vhandle,
			    const char *calling_routine);

extern int RfAllocVolParams(vol_file_handle_t *vhandle,
			    const char *calling_routine);

extern void RfCartParams2TITANGrid(cart_params_t *cart,
				   titan_grid_t *grid,
				   si32 grid_type);

extern void Rfdtime2rtime(date_time_t *dtime,
			  radtim_t *rtime);

extern void RfDecodeCartParams(cart_params_t *cart,
			       cart_float_params_t *fl_cart);

extern int RfFreeClutterTable(clutter_table_file_handle_t *clutter_handle,
			      const char *calling_routine);

extern int RfFreeRcTable(rc_table_file_handle_t *rc_handle,
			 const char *calling_routine);

extern int RfFreeSlaveTable(slave_table_file_handle_t *slave_handle,
			    const char *calling_routine);

extern int RfFreeVolArrays(vol_file_handle_t *vhandle,
			   const char *calling_routine);

extern int RfFreeVolParams(vol_file_handle_t *vhandle,
			   const char *calling_routine);

extern int RfFreeVolPlanes(vol_file_handle_t *vhandle,
			   const char *calling_routine);

extern void RfPrintCartParams(FILE *out,
			      const char *sstr,
			      cart_params_t *cart);

extern void RfPrintFieldParams(FILE *out,
			       const char *sstr,
			       si32 field_num,
			       field_params_t *fparams,
			       si32 *field_params_offset,
			       si32 nplanes,
			       si32 **plane_offset);

extern void RfPrintFloatPlaneHeights(FILE *out,
				     const char *spacer,
				     si32 nplanes,
				     double **plane_heights);

extern void RfPrintPlaneHeights(FILE *out,
				const char *sstr,
				si32 nplanes,
				si32 **plane_heights,
				double scalez);

extern void RfPrintRadarElevations(FILE *out,
				   const char *sstr,
				   const char *label,
				   si32 nelevations,
				   si32 *radar_elevations);

extern void RfPrintRadTime(FILE *out,
			   const char *sstr,
			   const char *label,
			   radtim_t *radtim);

extern void RfPrintVhandle(FILE *out,
			  const char *spacer,
			  vol_file_handle_t *vhandle);
  
extern void RfPrintVolParams(FILE *out,
			     const char *sstr,
			     vol_params_t *vol_params);

extern void RfRadtimFromUnix(radtim_t *radar_time,
			     time_t unix_time);
  
extern int RfReadClutterTable(clutter_table_file_handle_t *clutter_handle,
			      const char *calling_routine);

extern int RfReadDobsonVolume(vol_file_handle_t *vhandle,
			      const char *calling_routine);

extern int RfReadMDVVolume(vol_file_handle_t *vhandle,
			   const char *calling_routine);
  
extern int RfReadRcTable(rc_table_file_handle_t *rc_handle,
			 const char *calling_routine);

extern int RfReadSlaveTable(slave_table_file_handle_t *slave_handle,
			    const char *calling_routine);

extern int RfReadVolFparams(vol_file_handle_t *vhandle,
			    si32 field_num,
			    const char *calling_routine);

extern int RfReadVolParams(vol_file_handle_t *vhandle,
			   const char *calling_routine);

extern int RfReadVolPlane(vol_file_handle_t *vhandle,
			  si32 field_num,
			  si32 plane_num,
			  const char *calling_routine);

extern int RfReadVolume(vol_file_handle_t *vhandle,
			const char *calling_routine);

extern void Rfrtime2dtime(radtim_t *rtime,
			  date_time_t *dtime);
  
extern si32 Rfrtime2utime(radtim_t *rtime);

extern int RfSeekEndVolHeader(vol_file_handle_t *vhandle,
			      const char *calling_routine);

extern void Rfutime2rtime(time_t unix_time,
			  radtim_t *rtime);

extern int RfWriteDobson(vol_file_handle_t *vhandle,
			 int write_current_index,
			 int debug,
			 const char *output_dir,
			 const char *output_file_ext,
			 const char *prog_name,
			 const char *calling_routine);
     
extern int RfWriteDobsonRemote(vol_file_handle_t *vhandle,
			       int write_current_index,
			       int debug,
			       const char *output_host,
			       const char *output_dir,
			       const char *output_file_ext,
			       const char *local_tmp_dir,
			       const char *prog_name,
			       const char *calling_routine);
     
extern int RfWriteVolFparams(vol_file_handle_t *vhandle,
			     si32 field_num,
			     const char *calling_routine);

extern int RfWriteVolParams(vol_file_handle_t *vhandle,
			    const char *calling_routine);

extern int RfWriteVolPlane(vol_file_handle_t *vhandle,
			   si32 field_num,
			   si32 plane_num,
			   const char *calling_routine);

extern int RfWriteVolume(vol_file_handle_t *vhandle,
			 const char *calling_routine);

#ifdef __cplusplus
}
#endif

#endif
