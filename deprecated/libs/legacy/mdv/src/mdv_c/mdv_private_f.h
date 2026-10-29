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
#ifdef __cplusplus
extern "C" {
#endif


#ifndef MDV_PRIVATE_F_H_INCLUDED
#define MDV_PRIVATE_F_H_INCLUDED

#include <mdv/mdv_file.h>

/******************************************************************************
 *  MDV_PRIVATE_F.H  Prototypes for private functions used by the MDV library
 *                   in FORTRAN wrappers.
 *  N. Rehak.  June 1997. RAP.
 */

/***********************************************************************
 * mf_master_hdr_from_fortran:  Fills in the MDV_master_header_t structure
 *                              from the given FORTRAN arrays.
 */

void mf_master_hdr_from_fortran(si32 *master_hdr_ints,
				fl32 *master_hdr_reals,
				char *dataset_info,
				char *dataset_name,
				char *dataset_source,
				MDV_master_header_t *master_hdr);

/***********************************************************************
 * mf_master_hdr_to_fortran:  Fills in the FORTRAN arrays from the given
 *                            MDV_master_header_t structure.
 */

void mf_master_hdr_to_fortran(MDV_master_header_t *master_hdr,
			      si32 *master_hdr_ints,
			      fl32 *master_hdr_reals,
			      char *dataset_info,
			      char *dataset_name,
			      char *dataset_source);

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
			       MDV_field_header_t *field_hdr);;

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
			     char *field_unused_char);

/***********************************************************************
 * mf_vlevel_hdr_from_fortran:  Fills in the MDV_vlevel_header_t structure
 *                              from the given FORTRAN arrays.
 */

void mf_vlevel_hdr_from_fortran(si32 *vlevel_hdr_ints,
				fl32 *vlevel_hdr_reals,
				MDV_vlevel_header_t *vlevel_hdr);

/***********************************************************************
 * mf_vlevel_hdr_to_fortran:  Fills in the FORTRAN arrays from the given
 *                            MDV_vlevel_header_t structure.
 */

void mf_vlevel_hdr_to_fortran(MDV_vlevel_header_t *vlevel_hdr,
			      si32 *vlevel_hdr_ints,
			      fl32 *vlevel_hdr_reals);

/***********************************************************************
 * mf_chunk_hdr_from_fortran:  Fills in the MDV_chunk_header_t structure
 *                             from the given FORTRAN arrays.
 */

void mf_chunk_hdr_from_fortran(si32 *chunk_hdr_ints,
			       char *chunk_info,
			       MDV_chunk_header_t *chunk_hdr);

/***********************************************************************
 * mf_chunk_hdr_to_fortran:  Fills in the FORTRAN arrays from the given
 *                           MDV_chunk_header_t structure.
 */

void mf_chunk_hdr_to_fortran(MDV_chunk_header_t *chunk_hdr,
			     si32 *chunk_hdr_ints,
			     char *chunk_info);


#endif

#ifdef __cplusplus
}
#endif
