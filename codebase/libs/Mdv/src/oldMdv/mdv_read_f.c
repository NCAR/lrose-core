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
/******************************************************************************
 *  MDV_READ_F.C Fortran Wrappers for Subroutines to read data from MDV files.
 *  F. Hage.  May 1997 
 */

#include <errno.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_read.h>
#include <toolsa/mem.h>

#include "mdv_private_f.h"

#if defined(F_UNDERSCORE2)
  #define  mf_rm_read_master_hdr     mf_rm_read_master_hdr__
  #define  mf_rf_read_field_hdr      mf_rf_read_field_hdr__
  #define  mf_rv_read_vlevel_hdr     mf_rv_read_vlevel_hdr__
  #define  mf_rfd_read_field_data    mf_rfd_read_field_data__
  #define  mf_rc_read_chunk_hdr      mf_rc_read_chunk_hdr__
#elif defined(F_UNDERSCORE)
  #define  mf_rm_read_master_hdr     mf_rm_read_master_hdr_
  #define  mf_rf_read_field_hdr      mf_rf_read_field_hdr_
  #define  mf_rv_read_vlevel_hdr     mf_rv_read_vlevel_hdr_
  #define  mf_rfd_read_field_data    mf_rfd_read_field_data_
  #define  mf_rc_read_chunk_hdr      mf_rc_read_chunk_hdr_
#endif

/*
 * Status values returned by the read routines.  These must match
 * the corresponding values in mdv/mf_mdv.inc.
 */

#define MDV_READ_SUCCESSFUL             0
#define MDV_READ_OPEN_FAILURE           1
#define MDV_READ_BAD_MASTER_HDR         2
#define MDV_READ_INVALID_FIELD_NUM      3
#define MDV_READ_BAD_FIELD_HDR          4
#define MDV_READ_BAD_VLEVEL_HDR         5
#define MDV_READ_NO_VLEVEL_HDRS         6
#define MDV_READ_INVALID_CHUNK_NUM      7
#define MDV_READ_BAD_CHUNK_HDR          8
#define MDV_READ_DATA_ARRAY_TOO_SMALL   9
#define MDV_READ_DATA_ERROR            10


/***********************************************************************
* MF_RM_READ_MASTER_HDR: Read the master header from a data file and
* return the information in FORTRAN-usable structures.
*
* Use the following to call the subroutine:
*
*    CHARACTER*1024 FNAME
*    INTEGER*4      MASTER_HDR_INTS(MDV_NUM_MASTER_HEADER_SI32)
*    REAL*4         MASTER_HDR_REALS(MDV_NUM_MASTER_HEADER_FL32)
*    CHARACTER*(MDV_INFO_LEN)  DATASET_INFO
*    CHARACTER*(MDV_NAME_LEN)  DATASET_NAME
*    CHARACTER*(MDV_NAME_LEN)  DATASET_SOURCE
*    INTEGER        RETURN_STATUS
*
*   CALL MF_RM_READ_MASTER_HDR(FNAME, MASTER_HDR_INTS, MASTER_HDR_REALS,
*                              DATASET_INFO, DATASET_NAME, DATASET_SOURCE,
*		               RETURN_STATUS)
*
* This function will open the file named FNAME, read the master header.
* The header information is returned in the given arrays.
* 
* When finished, RETURN_STATUS can be MDV_READ_SUCCESSFUL,
*                                     MDV_READ_OPEN_FAILURE,
*                                     MDV_READ_BAD_MASTER_HDR
*/

void mf_rm_read_master_hdr( char *fname,
			    si32 *master_hdr_ints,
			    fl32 *master_hdr_reals,
			    char  *dataset_info,
			    char  *dataset_name,
			    char  *dataset_source,
			    int   *return_status)
{
  FILE *mdv_file;

  MDV_master_header_t master_hdr;
  int i;
  
  /* Strip off trailing spaces */
  i = 0;
  while (fname[i] != ' ')
    i++;
  fname[i] = '\0';
	 
  /*
   * Open the input file
   */

  if ((mdv_file = fopen(fname,"r")) == NULL)
  {
    fprintf(stderr, "Error opening input file\n");
    perror(fname);
    *return_status = MDV_READ_OPEN_FAILURE;
    return;
  }

  /*
   * Read the master header
   */

  if (MDV_load_master_header(mdv_file, &master_hdr) == MDV_FAILURE)
  {
    fprintf(stderr, "Error reading master header from file <%s>\n",
	    fname);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_MASTER_HDR;
    return;
  }

  /*
   * Close the input file
   */

  fclose(mdv_file);

  /*
   * Copy the master header info
   */

  mf_master_hdr_to_fortran(&master_hdr,
			   master_hdr_ints,
			   master_hdr_reals,
			   dataset_info,
			   dataset_name,
			   dataset_source);
  
  *return_status = MDV_READ_SUCCESSFUL;
     
  return;
}


/***********************************************************************
* MF_RF_READ_FIELD_HDR: Read the indicated field header from a data file
* and return the information in FORTRAN-usable structures.
*
* Use the following to call the subroutine:
*
*    CHARACTER*1024 FNAME
*    INTEGER        FIELD_NUM
*    INTEGER*4      FIELD_HDR_INTS(MDV_NUM_FIELD_HEADER_SI32)
*    REAL*4         FIELD_HDR_REALS(MDV_NUM_FIELD_HEADER_FL32)
*    CHARACTER*(MDV_LONG_FIELD_LEN) FIELD_NAME_LONG
*    CHARACTER*(MDV_SHORT_FIELD_LEN) FIELD_NAME_SHORT
*    CHARACTER*(MDV_UNITS_LEN) FIELD_UNITS
*    CHARACTER*(MDV_TRANSFORM_LEN) FIELD_TRANSFORM
*    CHARACTER*(MDV_UNITS_LEN) FIELD_UNUSED_CHAR
*    INTEGER        RETURN_STATUS
*
*   CALL MF_RF_READ_FIELD_HDR(FNAME, FIELD_NUM,
*                             FIELD_HDR_INTS, FIELD_HDR_REALS,
*                             FIELD_NAME_LONG, FIELD_NAME_SHORT,
*                             FIELD_UNITS, FIELD_TRANSFORM, FIELD_UNUSED_CHAR,
*		              RETURN_STATUS)
*
* This function will open the file named FNAME and read the indicated field
* header.  The header information is returned in the given arrays.
* 
* When finished, RETURN_STATUS can be MDV_READ_SUCCESSFUL,
*                                     MDV_READ_OPEN_FAILURE,
*                                     MDV_READ_BAD_MASTER_HDR,
*                                     MDV_READ_INVALID_FIELD_NUM
*                                     MDV_READ_BAD_FIELD_HDR
*/

void mf_rf_read_field_hdr( char *fname,
                           int *field_num,
                           si32 *field_hdr_ints,
                           fl32 *field_hdr_reals,
                           char *field_name_long,
                           char *field_name_short,
                           char *field_units,
                           char *field_transform,
                           char *field_unused_char,
                           int  *return_status)
{
  FILE *mdv_file;

  MDV_master_header_t master_hdr;
  MDV_field_header_t field_hdr;
  int i;
  
  /* Strip off trailing spaces */
  i = 0;
  while (fname[i] != ' ')
    i++;
  fname[i] = '\0';
  
  /*
   * Open the input file
   */

  if ((mdv_file = fopen(fname,"r")) == NULL)
  {
    fprintf(stderr, "Error opening input file\n");
    perror(fname);
    *return_status = MDV_READ_OPEN_FAILURE;
    return;
  }

  /*
   * Read the master header
   */

  if (MDV_load_master_header(mdv_file, &master_hdr) == MDV_FAILURE)
  {
    fprintf(stderr, "Error reading master header from file <%s>\n",
	    fname);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_MASTER_HDR;
    return;
  }

  /*
   * Check to make sure the field number is valid
   */

  if (*field_num >= master_hdr.n_fields)
  {
    fprintf(stderr, "Invalid field number %d given, file only has %d fields\n",
            *field_num, master_hdr.n_fields);
    fclose(mdv_file);
    *return_status = MDV_READ_INVALID_FIELD_NUM;
    return;
  }
  
  /*
   * Read the field header
   */

  if (MDV_load_field_header(mdv_file, &field_hdr, *field_num) != MDV_SUCCESS)
  {
    fprintf(stderr, "Error loading field %d header from file\n", *field_num);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_FIELD_HDR;
    return;
  }
  
  /*
   * Close the input file
   */

  fclose(mdv_file);

  /*
   * Copy the field header info
   */

  mf_field_hdr_to_fortran(&field_hdr,
                          field_hdr_ints,
                          field_hdr_reals,
                          field_name_long,
                          field_name_short,
                          field_units,
                          field_transform,
                          field_unused_char);
  
  *return_status = MDV_READ_SUCCESSFUL;
     
  return;
}


/***********************************************************************
* MF_RV_READ_VLEVEL_HDR: Read the indicated vlevel header from a data file
* and return the information in FORTRAN-usable structures.
*
* Use the following to call the subroutine:
*
*    CHARACTER*1024 FNAME
*    INTEGER        FIELD_NUM
*    INTEGER*4      VLEVEL_HDR_INTS(MDV_NUM_VLEVEL_HEADER_SI32)
*    REAL*4         VLEVEL_HDR_REALS(MDV_NUM_VLEVEL_HEADER_FL32)
*    INTEGER        RETURN_STATUS
*
*   CALL MF_RV_READ_VLEVEL_HDR(FNAME, FIELD_NUM,
*                              VLEVEL_HDR_INTS, VLEVEL_HDR_REALS,
*	                       RETURN_STATUS)
*
* This function will open the file named FNAME and read the indicated vlevel
* header.  The header information is returned in the given arrays.
* 
* When finished, RETURN_STATUS can be MDV_READ_SUCCESSFUL,
*                                     MDV_READ_OPEN_FAILURE,
*                                     MDV_READ_BAD_MASTER_HDR,
*                                     MDV_READ_INVALID_FIELD_NUM,
*                                     MDV_READ_BAD_VLEVEL_HDR
*/

void mf_rv_read_vlevel_hdr( char *fname,
			    int *field_num,
			    si32 *vlevel_hdr_ints,
			    fl32 *vlevel_hdr_reals,
                           int  *return_status)
{
  FILE *mdv_file;

  MDV_master_header_t master_hdr;
  MDV_vlevel_header_t vlevel_hdr;
  int i;
  
  /* Strip off trailing spaces */
  i = 0;
  while (fname[i] != ' ')
    i++;
  fname[i] = '\0';
  
  /*
   * Open the input file
   */

  if ((mdv_file = fopen(fname,"r")) == NULL)
  {
    fprintf(stderr, "Error opening input file\n");
    perror(fname);
    *return_status = MDV_READ_OPEN_FAILURE;
    return;
  }

  /*
   * Read the master header
   */

  if (MDV_load_master_header(mdv_file, &master_hdr) == MDV_FAILURE)
  {
    fprintf(stderr, "Error reading master header from file <%s>\n",
	    fname);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_MASTER_HDR;
    return;
  }

  /*
   * Check to make sure the field number is valid
   */

  if (*field_num >= master_hdr.n_fields)
  {
    fprintf(stderr, "Invalid field number %d given, file only has %d fields\n",
            *field_num, master_hdr.n_fields);
    fclose(mdv_file);
    *return_status = MDV_READ_INVALID_FIELD_NUM;
    return;
  }
  
  /*
   * Check to make sure vlevel headers are included in this file
   */

  if (!master_hdr.vlevel_included)
  {
    fprintf(stderr,
	    "Trying to read vlevel headers from file not including them\n");
    fclose(mdv_file);
    *return_status = MDV_READ_NO_VLEVEL_HDRS;
    return;
  }
  
  /*
   * Read the vlevel header
   */

  if (MDV_load_vlevel_header(mdv_file, &vlevel_hdr,
			     &master_hdr, *field_num) != MDV_SUCCESS)
  {
    fprintf(stderr, "Error loading field %d vlevelheader from file\n",
	    *field_num);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_VLEVEL_HDR;
    return;
  }
  
  /*
   * Close the input file
   */

  fclose(mdv_file);

  /*
   * Copy the vlevel header info
   */

  mf_vlevel_hdr_to_fortran(&vlevel_hdr,
			   vlevel_hdr_ints,
			   vlevel_hdr_reals);
  
  *return_status = MDV_READ_SUCCESSFUL;
     
  return;
}


/***********************************************************************
* MF_RFD_READ_READ_FIELD_DATA: Read the data for the indicated field
* from a data file and return the information in FORTRAN-usable structures.
*
* Use the following to call the subroutine:
*
*    CHARACTER*1024 FNAME
*    INTEGER        FIELD_NUM
*    CHARACTER*(ARRAY_SIZE) FIELD_DATA_ARRAY
*    INTEGER        ARRAY_SIZE
*    INTEGER        REQESTED_DATA_FORMAT
*    INTEGER        RETURN_DATA_SIZE
*    INTEGER        RETURN_STATUS
*
*   CALL MF_RFD_READ_FIELD_DATA(FNAME, FIELD_NUM,
*                               FIELD_DATA_ARRAY, ARRAY_SIZE,
*                               REQUESTED_DATA_FORMAT,
*                               RETURN_DATA_SIZE,
*	                        RETURN_STATUS)
*
* This function will open the file named FNAME and read the indicated field
* data.  The data is returned in the given arrays.
* 
* When finished, RETURN_STATUS can be MDV_READ_SUCCESSFUL,
*                                     MDV_READ_OPEN_FAILURE,
*                                     MDV_READ_BAD_MASTER_HDR,
*                                     MDV_READ_INVALID_FIELD_NUM,
*                                     MDV_READ_BAD_FIELD_HDR,
*                                     MDV_READ_DATA_ARRAY_TOO_SMALL,
*                                     MDV_READ_DATA_ERROR
*/

void mf_rfd_read_field_data( char *fname,
                             int *field_num,
                             char *field_data_array,
                             int *array_size,
                             int *requested_data_format,
                             int *return_data_size,
                             int *return_status)
{
  FILE *mdv_file;

  MDV_master_header_t master_hdr;
  MDV_field_header_t field_hdr;
  void *volume_data;
  int i;
  
  /* Strip off trailing spaces */
  i = 0;
  while (fname[i] != ' ')
    i++;
  fname[i] = '\0';
  
  /*
   * Open the input file
   */

  if ((mdv_file = fopen(fname,"r")) == NULL)
  {
    fprintf(stderr, "Error opening input file\n");
    perror(fname);
    *return_status = MDV_READ_OPEN_FAILURE;
    return;
  }

  /*
   * Read the master header
   */

  if (MDV_load_master_header(mdv_file, &master_hdr) == MDV_FAILURE)
  {
    fprintf(stderr, "Error reading master header from file <%s>\n",
	    fname);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_MASTER_HDR;
    return;
  }

  /*
   * Check to make sure the field number is valid
   */

  if (*field_num >= master_hdr.n_fields)
  {
    fprintf(stderr, "Invalid field number %d given, file only has %d fields\n",
            *field_num, master_hdr.n_fields);
    fclose(mdv_file);
    *return_status = MDV_READ_INVALID_FIELD_NUM;
    return;
  }
  
  /*
   * Read the field header
   */

  if (MDV_load_field_header(mdv_file, &field_hdr,
                            *field_num) != MDV_SUCCESS)
  {
    fprintf(stderr, "Error loading field %d header from file\n",
	    *field_num);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_FIELD_HDR;
    return;
  }
  
  /*
   * Read the data
   */

  if ((volume_data = MDV_get_volume_size(mdv_file, &field_hdr,
					 *requested_data_format,
					 return_data_size)) == NULL)
  {
    fprintf(stderr, "Error loading data for field %d from file\n",
            *field_num);
    fclose(mdv_file);
    *return_data_size = 0;
    *return_status = MDV_READ_DATA_ERROR;
    return;
  }
  
  /*
   * Close the input file
   */

  fclose(mdv_file);

  /*
   * Make sure the return array is big enough for the data
   */

  if (*return_data_size > *array_size)
  {
    fprintf(stderr,
	    "Given array not big enough for data -- array has %d bytes, data has %d bytes\n",
	    *array_size, *return_data_size);
    fclose(mdv_file);
    *return_status = MDV_READ_DATA_ARRAY_TOO_SMALL;
    return;
  }
  
  /*
   * Copy the data into the returned array
   */

  memcpy(field_data_array, volume_data, *return_data_size);
  ufree(volume_data);
  
  *return_status = MDV_READ_SUCCESSFUL;
     
  return;
}


/***********************************************************************
* MF_RC_READ_CHUNK_HDR: Read the indicated chunk header from a data file
* and return the information in FORTRAN-usable structures.
*
* Use the following to call the subroutine:
*
*    CHARACTER*1024 FNAME
*    INTEGER        CHUNK_NUM
*    INTEGER*4      CHUNK_HDR_INTS(MDV_NUM_CHUNK_HEADER_SI32)
*    CHARACTER*(MDV_CHUNK_INFO_LEN) CHUNK_INFO
*    INTEGER        RETURN_STATUS
*
*   CALL MF_RC_READ_CHUNK_HDR(FNAME, CHUNK_NUM,
*                             CHUNK_HDR_INTS, CHUNK_INFO,
*                             RETURN_STATUS)
*
* This function will open the file named FNAME and read the indicated chunk
* header.  The header information is returned in the given arrays.
* 
* When finished, RETURN_STATUS can be MDV_READ_SUCCESSFUL,
*                                     MDV_READ_OPEN_FAILURE,
*                                     MDV_READ_BAD_MASTER_HDR,
*                                     MDV_READ_INVALID_CHUNK_NUM,
*                                     MDV_READ_BAD_CHUNK_HDR
*/

void mf_rc_read_chunk_hdr( char *fname,
			   int *chunk_num,
			   si32 *chunk_hdr_ints,
			   char *chunk_info,
                           int  *return_status)
{
  FILE *mdv_file;

  MDV_master_header_t master_hdr;
  MDV_chunk_header_t chunk_hdr;
  int i;
  
  /* Strip off trailing spaces */
  i = 0;
  while (fname[i] != ' ')
    i++;
  fname[i] = '\0';
  
  /*
   * Open the input file
   */

  if ((mdv_file = fopen(fname,"r")) == NULL)
  {
    fprintf(stderr, "Error opening input file\n");
    perror(fname);
    *return_status = MDV_READ_OPEN_FAILURE;
    return;
  }

  /*
   * Read the master header
   */

  if (MDV_load_master_header(mdv_file, &master_hdr) == MDV_FAILURE)
  {
    fprintf(stderr, "Error reading master header from file <%s>\n",
	    fname);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_MASTER_HDR;
    return;
  }

  /*
   * Check to make sure the chunk number is valid
   */

  if (*chunk_num >= master_hdr.n_chunks)
  {
    fprintf(stderr, "Invalid chunk number %d given, file only has %d chunks\n",
            *chunk_num, master_hdr.n_chunks);
    fclose(mdv_file);
    *return_status = MDV_READ_INVALID_CHUNK_NUM;
    return;
  }
  
  /*
   * Read the chunk header
   */

  if (MDV_load_chunk_header(mdv_file, &chunk_hdr,
			    &master_hdr, *chunk_num) != MDV_SUCCESS)
  {
    fprintf(stderr, "Error loading chunk %d header from file\n",
	    *chunk_num);
    fclose(mdv_file);
    *return_status = MDV_READ_BAD_CHUNK_HDR;
    return;
  }
  
  /*
   * Close the input file
   */

  fclose(mdv_file);

  /*
   * Copy the chunk header info
   */

  mf_chunk_hdr_to_fortran(&chunk_hdr,
			  chunk_hdr_ints,
			  chunk_info);
  
  *return_status = MDV_READ_SUCCESSFUL;
     
  return;
}

