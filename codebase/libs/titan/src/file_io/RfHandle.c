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
/*************************************************************************
 *
 * RfHandle.c
 *
 * File handle utility routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * June 1998
 *
 **************************************************************************/

#include <toolsa/umisc.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <titan/storm.h>
#include <titan/track.h>

/************************
 * generic handle structs
 *
 * The start of the actual handle structs must match
 * the generic structs.
 */
  
typedef struct {
  
  char *prog_name;
  char *file_path;
  char *file_label;
  FILE *file;
  int handle_initialized;
  
} rf_handle_t;

typedef struct {
  
  char *prog_name;
  char *file_path_0;
  char *file_label_0;
  FILE *file_0;
  char *file_path_1;
  char *file_label_1;
  FILE *file_1;
  int handle_initialized;
  
} rf_dual_handle_t;

/*
 * file scope prototypes
 */

static void rf_free_dual_handle(rf_dual_handle_t *handle);

static void rf_free_handle(rf_handle_t *handle);

static void rf_init_dual_handle(rf_dual_handle_t *handle,
				int size,
				const char *prog_name,
				const char *file_path_0,
				const char *file_label_0,
				FILE *file_0,
				const char *file_path_1,
				const char *file_label_1,
				FILE *file_1);

static void rf_init_handle(rf_handle_t *handle,
			   int size,
			   const char *prog_name,
			   const char *file_path,
			   const char *file_label,
			   FILE *file);

/***********************
 * RfInitClutterHandle()
 *
 * Initialize the clutter table handle
 */

void RfInitClutterHandle(clutter_table_file_handle_t *handle,
			 const char *prog_name,
			 const char *file_name,
			 FILE *fd)

{
  rf_init_handle((rf_handle_t *) handle,
		 sizeof(clutter_table_file_handle_t),
		 prog_name, file_name,
		 CLUTTER_TABLE_FILE, fd);
}


/***********************
 * RfFreeClutterHandle()
 *
 * Free the clutter table handle
 */

void RfFreeClutterHandle(clutter_table_file_handle_t *handle)

{
  RfFreeClutterTable(handle, "RfFreeClutterHandle");
  rf_free_handle((rf_handle_t *) handle);
}


/***********************
 * RfInitRcTableHandle()
 *
 * Initialize the RcTable handle
 */

void RfInitRcTableHandle(rc_table_file_handle_t *handle,
			 const char *prog_name,
			 const char *file_name,
			 FILE *fd)

{
  rf_init_handle((rf_handle_t *) handle,
		 sizeof(rc_table_file_handle_t),
		 prog_name, file_name,
		 RADAR_TO_CART_TABLE_FILE, fd);
}


/***********************
 * RfFreeRcTableHandle()
 *
 * Free the RcTable handle
 */

void RfFreeRcTableHandle(rc_table_file_handle_t *handle)

{
  RfFreeRcTable(handle, "RfFreeRcTableHandle");
  rf_free_handle((rf_handle_t *) handle);
}


/***********************
 * RfInitSlaveTableHandle()
 *
 * Initialize the SlaveTable handle
 */

void RfInitSlaveTableHandle(slave_table_file_handle_t *handle,
			 const char *prog_name,
			 const char *file_name,
			 FILE *fd)

{
  rf_init_handle((rf_handle_t *) handle,
		 sizeof(slave_table_file_handle_t),
		 prog_name, file_name,
		 RADAR_TO_CART_SLAVE_TABLE_FILE, fd);
}


/***********************
 * RfFreeSlaveTableHandle()
 *
 * Free the SlaveTable handle
 */

void RfFreeSlaveTableHandle(slave_table_file_handle_t *handle)

{
  RfFreeSlaveTable(handle, "RfFreeSlaveTableHandle");
  rf_free_handle((rf_handle_t *) handle);
}

/***********************
 * RfInitStormFileHandle()
 *
 * Initialize the StormFile handle
 */

void RfInitStormFileHandle(storm_file_handle_t *handle,
			   const char *prog_name)

{
  rf_init_dual_handle((rf_dual_handle_t *) handle,
		      sizeof(storm_file_handle_t),
		      prog_name,
		      (char *) NULL,
		      STORM_HEADER_FILE_TYPE,
		      (FILE *) NULL,
		      (char *) NULL,
		      STORM_DATA_FILE_TYPE,
		      (FILE *) NULL);
}


/***********************
 * RfFreeStormFileHandle()
 *
 * Free the StormFile handle
 */

void RfFreeStormFileHandle(storm_file_handle_t *handle)
     
{

  RfCloseStormFiles(handle, "RfFreeStormFileHandle");
  RfFreeStormScan(handle, "RfFreeStormFileHandle");
  RfFreeStormScanOffsets(handle, "RfFreeStormFileHandle");
  RfFreeStormProps(handle, "RfFreeStormFileHandle");
  RfFreeStormHeader(handle, "RfFreeStormFileHandle");

  rf_free_dual_handle((rf_dual_handle_t *) handle);

}

/***********************
 * RfInitTrackFileHandle()
 *
 * Initialize the TrackFile handle
 */

void RfInitTrackFileHandle(track_file_handle_t *handle,
			   const char *prog_name)

{
  rf_init_dual_handle((rf_dual_handle_t *) handle,
		      sizeof(track_file_handle_t),
		      prog_name,
		      (char *) NULL,
		      TRACK_HEADER_FILE_TYPE,
		      (FILE *) NULL,
		      (char *) NULL,
		      TRACK_DATA_FILE_TYPE,
		      (FILE *) NULL);
}


/***********************
 * RfFreeTrackFileHandle()
 *
 * Free the TrackFile handle
 */

void RfFreeTrackFileHandle(track_file_handle_t *handle)
     
{

  RfCloseTrackFiles(handle, "RfFreeTrackFileHandle");
  RfFreeTrackArrays(handle, "RfFreeTrackFileHandle");
  RfFreeTrackEntry(handle, "RfFreeTrackFileHandle");
  RfFreeTrackHeader(handle, "RfFreeTrackFileHandle");
  RfFreeSimpleTrackParams(handle, "RfFreeTrackFileHandle");
  RfFreeComplexTrackParams(handle, "RfFreeTrackFileHandle");
  RfFreeTrackScanEntries(handle, "RfFreeTrackFileHandle");

  rf_free_dual_handle((rf_dual_handle_t *) handle);

}

/*********************
 * file scope routines
 */

/*************************************************************************
 *
 * rf_free_dual_handle()
 *
 * frees the memory associated with a generic dual file handle
 *
 **************************************************************************/

static void rf_free_dual_handle(rf_dual_handle_t *handle)

{

  if (handle->handle_initialized) {
    
    ufree(handle->prog_name);

    /*
     * file paths
     */

    if (handle->file_path_0 != NULL) {

      ufree(handle->file_path_0);

      handle->file_path_0 = NULL;

    }

    if (handle->file_path_1 != NULL) {

      ufree(handle->file_path_1);

      handle->file_path_1 = NULL;

    }

    /*
     * file labels
     */

    if (handle->file_label_0 != NULL) {

      ufree(handle->file_label_0);

      handle->file_label_0 = NULL;

    }

    if (handle->file_label_1 != NULL) {

      ufree(handle->file_label_1);

      handle->file_label_1 = NULL;

    }

    /*
     * file pointers
     */

    handle->file_0 = (FILE *) NULL;
    handle->file_1 = (FILE *) NULL;

    handle->handle_initialized = FALSE;

  }

  return;

}

/*************************************************************************
 *
 * rf_free_handle()
 *
 * frees the memory associated with the file handle
 *
 **************************************************************************/

static void rf_free_handle(rf_handle_t *handle)

{

  if (handle->handle_initialized) {
    
    ufree(handle->prog_name);
    
    if (handle->file_path != NULL) {

      ufree(handle->file_path);

      handle->file_path = NULL;

    }

    if (handle->file_label != NULL) {

      ufree(handle->file_label);

      handle->file_label = NULL;

    }

    handle->file = (FILE *) NULL;

    handle->handle_initialized = FALSE;

  }

  return;

}

/*************************************************************************
 *
 * rf_init_dual_handle()
 *
 * initializes the memory associated with a generic file handle
 *
 **************************************************************************/

static void rf_init_dual_handle(rf_dual_handle_t *handle,
				int size,
				const char *prog_name,
				const char *file_path_0,
				const char *file_label_0,
				FILE *file_0,
				const char *file_path_1,
				const char *file_label_1,
				FILE *file_1)
     
{

  /*
   * set fields in handle
   */

  memset ((void *)  handle,
          (int) 0, (size_t)  size);

  handle->prog_name = (char *) umalloc
    ((ui32) (strlen(prog_name) + 1));

  strcpy(handle->prog_name, prog_name);

  /*
   * file paths
   */

  if (file_path_0 != NULL) {

    handle->file_path_0 = (char *) umalloc
      ((ui32) (strlen(file_path_0) + 1));

    strcpy(handle->file_path_0, file_path_0);

  }

  if (file_path_1 != NULL) {

    handle->file_path_1 = (char *) umalloc
      ((ui32) (strlen(file_path_1) + 1));

    strcpy(handle->file_path_1, file_path_1);

  }

  /*
   * file labels
   */

  if (file_label_0 != NULL) {

    handle->file_label_0 = (char *) ucalloc
      ((ui32) 1, (ui32) R_FILE_LABEL_LEN);

    strcpy(handle->file_label_0, file_label_0);
    
  }

  if (file_label_1 != NULL) {

    handle->file_label_1 = (char *) ucalloc
      ((ui32) 1, (ui32) R_FILE_LABEL_LEN);

    strcpy(handle->file_label_1, file_label_1);
    
  }

  /*
   * file pointers
   */

  handle->file_0 = file_0;
  handle->file_1 = file_1;

  handle->handle_initialized = TRUE;

  return;

}

/*************************************************************************
 *
 * rf_init_handle()
 *
 * initializes the memory associated with a generic dual file handle
 *
 **************************************************************************/

static void rf_init_handle(rf_handle_t *handle,
			   int size,
			   const char *prog_name,
			   const char *file_path,
			   const char *file_label,
			   FILE *file)
     
{

  /*
   * set fields in handle
   */

  memset ((void *)  handle,
          (int) 0, (size_t)  size);

  handle->prog_name = (char *) umalloc
    ((ui32) (strlen(prog_name) + 1));

  strcpy(handle->prog_name, prog_name);
  
  if (file_path != NULL) {

    handle->file_path = (char *) umalloc
      ((ui32) (strlen(file_path) + 1));

    strcpy(handle->file_path, file_path);

  }

  if (file_label != NULL) {

    handle->file_label = (char *) umalloc
      ((ui32) R_FILE_LABEL_LEN);

    memset ((void *) handle->file_label,
            (int) 0, (size_t)  R_FILE_LABEL_LEN);

    strcpy(handle->file_label, file_label);
    
  }

  handle->file = file;

  handle->handle_initialized = TRUE;

  return;

}
