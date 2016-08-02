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
 *  MDV_PRINT_F.C Fortran Wrappers for Subroutines to print MDV data.
 *  N. Rehak, June 1997
 */

#include <stdio.h>
#include <errno.h>

#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_print.h>

#if defined(F_UNDERSCORE2)
  #define mf_pm_print_master_hdr    mf_pm_print_master_hdr__
  #define mf_pf_print_field_hdr     mf_pf_print_field_hdr__
  #define mf_pv_print_vlevel_hdr    mf_pv_print_vlevel_hdr__
  #define mf_pc_print_chunk_hdr     mf_pc_print_chunk_hdr__
#elif defined(F_UNDERSCORE)
  #define mf_pm_print_master_hdr    mf_pm_print_master_hdr_
  #define mf_pf_print_field_hdr     mf_pf_print_field_hdr_
  #define mf_pv_print_vlevel_hdr    mf_pv_print_vlevel_hdr_
  #define mf_pc_print_chunk_hdr     mf_pc_print_chunk_hdr_
#endif

#include "mdv_private_f.h"

/***********************************************************************
* MF_pm_print_master_hdr: This routine will print an MDV master header
* from FORTRAN format data.
*
* Use the following to call the subroutine:
*
*    INTEGER*4     MASTER_HDR_INTS(MDV_NUM_MASTER_HEADER_SI32)
*    REAL*4        MASTER_HDR_REALS(MDV_NUM_MASTER_HEADER_FL32)
*    CHARACTER*(MDV_INFO_LEN) DATASET_INFO
*    CHARACTER*(MDV_NAME_LEN) DATASET_NAME
*    CHARACTER*(MDV_NAME_LEN) DATASET_SOURCE
*
*   CALL MF_PM_PRINT_MASTER_HDR(MASTER_HDR_INTS, MASTER_HDR_REALS,
*                               DATASET_INFO, DATASET_NAME, DATASET_SOURCE)
*
*/

void mf_pm_print_master_hdr( si32 *master_hdr_ints,
                             fl32 *master_hdr_reals,
                             char *dataset_info,
                             char *dataset_name,
			     char *dataset_source)
{
  MDV_master_header_t master_hdr;
  
  /*
   * Load the information in the master header
   */

  mf_master_hdr_from_fortran(master_hdr_ints,
			     master_hdr_reals,
			     dataset_info,
			     dataset_name,
			     dataset_source,
			     &master_hdr);
  
  /*
   * Print the master header
   */

  MDV_print_master_header_full(&master_hdr, stdout);
  
  return;
}


/***********************************************************************
* MF_pf_print_field_hdr: This routine will print an MDV field header
* from FORTRAN format data.
*
* Use the following to call the subroutine:
*
*    INTEGER*4     FIELD_HDR_INTS(MDV_NUM_FIELD_HEADER_SI32)
*    REAL*4        FIELD_HDR_REALS(MDV_NUM_FIELD_HEADER_FL32)
*    CHARACTER*(MDV_LONG_FIELD_LEN) FIELD_NAME_LONG
*    CHARACTER*(MDV_SHORT_FIELD_LEN) FIELD_NAME_SHORT
*    CHARACTER*(MDV_UNITS_LEN) FIELD_UNITS
*    CHARACTER*(MDV_TRANSFORM_LEN) FIELD_TRANSFORM
*    CHARACTER*(MDV_UNITS_LEN) FIELD_UNUSED_CHAR
*
*   CALL MF_PF_PRINT_FIELD_HDR(FIELD_HDR_INTS, FIELD_HDR_REALS,
*                              FIELD_NAME_LONG, FIELD_NAME_SHORT,
*                              FIELD_UNITS, FIELD_TRANSFORM,
*                              FIELD_UNUSED_CHAR)
*
*/

void mf_pf_print_field_hdr( si32 *field_hdr_ints,
			    fl32 *field_hdr_reals,
			    char *field_name_long,
			    char *field_name_short,
			    char *field_units,
			    char *field_transform,
			    char *field_unused_char)
{
  MDV_field_header_t field_hdr;
  
  /*
   * Load the information in the field header
   */

  mf_field_hdr_from_fortran(field_hdr_ints,
			    field_hdr_reals,
			    field_name_long,
			    field_name_short,
			    field_units,
			    field_transform,
			    field_unused_char,
			    &field_hdr);
  
  /*
   * Print the field header
   */

  MDV_print_field_header_full(&field_hdr, stdout);
  
  return;
}


/***********************************************************************
* MF_pv_print_vlevel_hdr: This routine will print an MDV vlevel header
* from FORTRAN format data.
*
* Use the following to call the subroutine:
*
*    INTEGER*4     VLEVEL_HDR_INTS(MDV_NUM_VLEVEL_HEADER_SI32)
*    REAL*4        VLEVEL_HDR_REALS(MDV_NUM_VLEVEL_HEADER_FL32)
*    INTEGER*4     NUM_Z
*    CHARACTER*(MDV_LONG_FIELD_LEN) FIELD_NAME
*
*   CALL MF_PV_PRINT_VLEVEL_HDR(VLEVEL_HDR_INTS, VLEVEL_HDR_REALS,
*                               NUM_Z, FIELD_NAME)
*
*/

void mf_pv_print_vlevel_hdr( si32 *vlevel_hdr_ints,
			     fl32 *vlevel_hdr_reals,
			     int *num_z,
			     char *field_name)
{
  MDV_vlevel_header_t vlevel_hdr;
  
  /*
   * Load the information in the vlevel header
   */

  mf_vlevel_hdr_from_fortran(vlevel_hdr_ints,
			     vlevel_hdr_reals,
			     &vlevel_hdr);
  
  /*
   * Print the vlevel header
   */

  MDV_print_vlevel_header_full(&vlevel_hdr, *num_z,
			       field_name, stdout);
  
  return;
}


/***********************************************************************
* MF_pc_print_chunk_hdr: This routine will print an MDV chunk header
* from FORTRAN format data.
*
* Use the following to call the subroutine:
*
*    INTEGER*4     CHUNK_HDR_INTS(MDV_NUM_CHUNK_HEADER_SI32)
*    CHARACTER*(MDV_CHUNK_INFO_LEN) CHUNK_INFO
*
*   CALL MF_PC_PRINT_CHUNK_HDR(CHUNK_HDR_INTS, CHUNK_INFO)
*
*/

void mf_pc_print_chunk_hdr( si32 *chunk_hdr_ints,
			    char *chunk_info)
{
  MDV_chunk_header_t chunk_hdr;
  
  /*
   * Load the information in the chunk header
   */

  mf_chunk_hdr_from_fortran(chunk_hdr_ints,
			    chunk_info,
			    &chunk_hdr);
  
  /*
   * Print the chunk header
   */

  MDV_print_chunk_header_full(&chunk_hdr, stdout);
  
  return;
}

