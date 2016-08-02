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
 *  MDV_PRIVATE_F.C Utility routines used by the library when dealing with
 *  FORTRAN structures.
 *  N. Rehak, June 1997
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <toolsa/os_config.h>
#include <Mdv/mdv/mdv_file.h>

#include "mdv_private_f.h"

/***********************************************************************
 * mf_master_hdr_from_fortran:  Fills in the MDV_master_header_t structure
 *                              from the given FORTRAN arrays.
 */

void mf_master_hdr_from_fortran(si32 *master_hdr_ints,
				fl32 *master_hdr_reals,
				char *dataset_info,
				char *dataset_name,
				char *dataset_source,
				MDV_master_header_t *master_hdr)
{
  /*
   * Load the information from the arrays
   */

  memcpy(&master_hdr->struct_id, master_hdr_ints,
         MDV_NUM_MASTER_HEADER_SI32 * sizeof(si32));
  
  memcpy(master_hdr->user_data_fl32, master_hdr_reals,
         MDV_NUM_MASTER_HEADER_FL32 * sizeof(fl32));
  
  memcpy(master_hdr->data_set_info, dataset_info, MDV_INFO_LEN);
  
  memcpy(master_hdr->data_set_name, dataset_name, MDV_NAME_LEN);
  
  memcpy(master_hdr->data_set_source, dataset_source, MDV_NAME_LEN);
  
  /*
   * Make sure the strings are NULL terminated
   */

  master_hdr->data_set_info[MDV_INFO_LEN-1] = '\0';
  master_hdr->data_set_name[MDV_NAME_LEN-1] = '\0';
  master_hdr->data_set_source[MDV_NAME_LEN-1] = '\0';
  
  /*
   * Set the record length values
   */

  master_hdr->record_len1 = sizeof(MDV_master_header_t) - 2 * sizeof(si32);
  master_hdr->record_len2 = master_hdr->record_len1;
  
  return;
}

/***********************************************************************
 * mf_master_hdr_to_fortran:  Fills in the FORTRAN arrays from the given
 *                            MDV_master_header_t structure.
 */

void mf_master_hdr_to_fortran(MDV_master_header_t *master_hdr,
			      si32 *master_hdr_ints,
			      fl32 *master_hdr_reals,
			      char *dataset_info,
			      char *dataset_name,
			      char *dataset_source)
{
  /*
   * Load the information into the arrays.
   */

  memcpy(master_hdr_ints, &master_hdr->struct_id,
	 sizeof(si32) * MDV_NUM_MASTER_HEADER_SI32);

  memcpy(master_hdr_reals, master_hdr->user_data_fl32,
	 sizeof(fl32) * MDV_NUM_MASTER_HEADER_FL32);

  memcpy(dataset_info, master_hdr->data_set_info, MDV_INFO_LEN);

  memcpy(dataset_name, master_hdr->data_set_name, MDV_NAME_LEN);

  memcpy(dataset_source, master_hdr->data_set_source, MDV_NAME_LEN);

  return;
}


/***********************************************************************
 * mf_field_hdr_from_fortran:  Fills in the MDV_field_header_t structure
 *                             from the given FORTRAN arrays.
 */

void mf_field_hdr_from_fortran(si32 *field_hdr_ints,
			       fl32 *field_hdr_reals,
			       char *field_name_long,
			       char *field_name_short,
			       char *field_units,
			       char *field_transform,
			       char *field_unused_char,
			       MDV_field_header_t *field_hdr)
{
  /*
   * Load the information into the field header.
   */

  memcpy(&field_hdr->struct_id, field_hdr_ints,
	 sizeof(si32) * MDV_NUM_FIELD_HEADER_SI32);

  memcpy(&field_hdr->proj_origin_lat, field_hdr_reals,
	 sizeof(fl32) * MDV_NUM_FIELD_HEADER_FL32);

  memcpy(field_hdr->field_name_long, field_name_long, MDV_LONG_FIELD_LEN);
  
  memcpy(field_hdr->field_name, field_name_short, MDV_SHORT_FIELD_LEN);
  
  memcpy(field_hdr->units, field_units, MDV_UNITS_LEN);
  
  memcpy(field_hdr->transform, field_transform, MDV_TRANSFORM_LEN);
  
  memcpy(field_hdr->unused_char, field_unused_char, MDV_UNITS_LEN);
  
  /*
   * Make sure the strings are NULL terminated.
   */

  field_hdr->field_name_long[MDV_LONG_FIELD_LEN-1] = '\0';
  field_hdr->field_name[MDV_SHORT_FIELD_LEN-1] = '\0';
  field_hdr->units[MDV_UNITS_LEN-1] = '\0';
  field_hdr->transform[MDV_TRANSFORM_LEN-1] = '\0';
  
  /*
   * Set the record length values
   */

  field_hdr->record_len1 = sizeof(MDV_field_header_t) - 2 * sizeof(si32);
  field_hdr->record_len2 = field_hdr->record_len1;
  
  return;
}


/***********************************************************************
 * mf_field_hdr_to_fortran:  Fills in the FORTRAN arrays from the given
 *                           MDV_field_header_t structure.
 */

void mf_field_hdr_to_fortran(MDV_field_header_t *field_hdr,
			     si32 *field_hdr_ints,
			     fl32 *field_hdr_reals,
			     char *field_name_long,
			     char *field_name_short,
			     char *field_units,
			     char *field_transform,
			     char *field_unused_char)
{
  /*
   * Load the information into the arrays.
   */

  memcpy(field_hdr_ints, &field_hdr->struct_id,
	 sizeof(si32) * MDV_NUM_FIELD_HEADER_SI32);

  memcpy(field_hdr_reals, &field_hdr->proj_origin_lat,
	 sizeof(fl32) * MDV_NUM_FIELD_HEADER_FL32);

  memcpy(field_name_long, field_hdr->field_name_long, MDV_LONG_FIELD_LEN);
  
  memcpy(field_name_short, field_hdr->field_name, MDV_SHORT_FIELD_LEN);
  
  memcpy(field_units, field_hdr->units, MDV_UNITS_LEN);
  
  memcpy(field_transform, field_hdr->transform, MDV_TRANSFORM_LEN);
  
  memcpy(field_unused_char, field_hdr->unused_char, MDV_UNITS_LEN);
  
  return;
}


/***********************************************************************
 * mf_vlevel_hdr_from_fortran:  Fills in the MDV_vlevel_header_t structure
 *                              from the given FORTRAN arrays.
 */

void mf_vlevel_hdr_from_fortran(si32 *vlevel_hdr_ints,
				fl32 *vlevel_hdr_reals,
				MDV_vlevel_header_t *vlevel_hdr)
{
  /*
   * Load the information into the vlevel header.
   */

  memcpy(&vlevel_hdr->struct_id, vlevel_hdr_ints,
	 sizeof(si32) * MDV_NUM_VLEVEL_HEADER_SI32);

  memcpy(vlevel_hdr->vlevel_params, vlevel_hdr_reals,
	 sizeof(fl32) * MDV_NUM_VLEVEL_HEADER_FL32);

  /*
   * Set the record length values
   */

  vlevel_hdr->record_len1 = sizeof(MDV_vlevel_header_t) - 2 * sizeof(si32);
  vlevel_hdr->record_len2 = vlevel_hdr->record_len1;
  
  return;
}


/***********************************************************************
 * mf_vlevel_hdr_to_fortran:  Fills in the FORTRAN arrays from the given
 *                            MDV_vlevel_header_t structure.
 */

void mf_vlevel_hdr_to_fortran(MDV_vlevel_header_t *vlevel_hdr,
			      si32 *vlevel_hdr_ints,
			      fl32 *vlevel_hdr_reals)
{
  /*
   * Load the information into the arrays.
   */

  memcpy(vlevel_hdr_ints, &vlevel_hdr->struct_id,
	 sizeof(si32) * MDV_NUM_VLEVEL_HEADER_SI32);

  memcpy(vlevel_hdr_reals, vlevel_hdr->vlevel_params,
	 sizeof(fl32) * MDV_NUM_VLEVEL_HEADER_FL32);

  return;
}


/***********************************************************************
 * mf_chunk_hdr_from_fortran:  Fills in the MDV_chunk_header_t structure
 *                             from the given FORTRAN arrays.
 */

void mf_chunk_hdr_from_fortran(si32 *chunk_hdr_ints,
			       char *chunk_info,
			       MDV_chunk_header_t *chunk_hdr)
{
  /*
   * Load the information into the chunk header.
   */

  memcpy(&chunk_hdr->struct_id, chunk_hdr_ints,
	 sizeof(si32) * MDV_NUM_CHUNK_HEADER_SI32);

  memcpy(chunk_hdr->info, chunk_info, MDV_CHUNK_INFO_LEN);
  
  /*
   * Make sure the strings are NULL terminated
   */

  chunk_hdr->info[MDV_CHUNK_INFO_LEN-1] = '\0';
  
  /*
   * Set the record length values
   */

  chunk_hdr->record_len1 = sizeof(MDV_chunk_header_t) - 2 * sizeof(si32);
  chunk_hdr->record_len2 = chunk_hdr->record_len1;
  
  return;
}


/***********************************************************************
 * mf_chunk_hdr_to_fortran:  Fills in the FORTRAN arrays from the given
 *                           MDV_chunk_header_t structure.
 */

void mf_chunk_hdr_to_fortran(MDV_chunk_header_t *chunk_hdr,
			     si32 *chunk_hdr_ints,
			     char *chunk_info)
{
  /*
   * Load the information into the arrays.
   */

  memcpy(chunk_hdr_ints, &chunk_hdr->struct_id,
	 sizeof(si32) * MDV_NUM_CHUNK_HEADER_SI32);

  memcpy(chunk_info, chunk_hdr->info, MDV_CHUNK_INFO_LEN);

  return;
}

