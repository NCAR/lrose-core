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
#ifndef MDV_PRINT_H
#define MDV_PRINT_H

/* MDV print routines and their utilities -- Rachel Ames */

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <time.h> 
#include <Mdv/mdv/mdv_file.h>
#include <Mdv/mdv/mdv_dataset.h>
#include <rapformats/sounding_chunk.h>


void MDV_print_master_header(MDV_master_header_t *mmh, 
                             FILE *outfile);

void MDV_print_master_header_full(MDV_master_header_t *mmh, 
                                  FILE *outfile);

void MDV_print_field_vlevel_header(MDV_field_vlevel_header_t *mfvh, 
                                   FILE *outfile);

void MDV_print_field_vlevel_header_full(MDV_field_vlevel_header_t *mfvh,
                                        FILE *outfile);

void MDV_print_field_header(MDV_field_header_t *mvh, 
                            FILE *outfile);

void MDV_print_field_header_full(MDV_field_header_t *mvh, 
                                 FILE *outfile);

void MDV_print_vlevel_header(MDV_vlevel_header_t *mvh, int nz, 
                             char *field_name, FILE *outfile);

void MDV_print_vlevel_header_full(MDV_vlevel_header_t *mvh, int nz, 
                                  char *field_name, FILE *outfile);

void MDV_print_chunk_header(MDV_chunk_header_t *mch, FILE *outfile);

void MDV_print_chunk_header_full(MDV_chunk_header_t *mch, FILE *outfile);

void MDV_print_chunk_data_full(void *data, si32 chunk_id,
			       si32 size, FILE *outfile);

void MDV_print_dataset_data(MDV_dataset_t *dataset,
			    FILE *outfile);

void MDV_print_plane(MDV_field_header_t *fhdr,
		     void *plane_ptr,
		     int field_num, int plane_num,
		     int data_type,
		     FILE *outfile);

void MDV_print_field_plane_full(MDV_field_header_t *fhdr,
				void *plane_ptr,
				int field_num, int plane_num,
				FILE *outfile);

void MDV_print_dataset(MDV_dataset_t *dataset, FILE *outfile);

void MDV_print_dataset_full(MDV_dataset_t *dataset, FILE *outfile);

char *MDV_verttype2string(int vert_type);

char *MDV_proj2string(int proj_type);

char *MDV_encode2string(int encode_type);

char *MDV_colltype2string(int coll_type);

char *MDV_orient2string(int orient_type);

char *MDV_order2string(int order_type);

char * MDV_compression2string(int compress_type);

char * MDV_transform2string(int transform_type);

char * MDV_scaling2string(int scaling_type);

#ifdef __cplusplus
}
#endif

#endif /* MDV_PRINT_H */
