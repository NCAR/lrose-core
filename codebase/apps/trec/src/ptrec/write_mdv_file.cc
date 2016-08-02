// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/***************************************************************************
 * write_mdv_file.c
 *
 * adapted from $RAP_DIR/apps/src/ctrec/process_files.c (Ames)
 *
 * Terri L. Betancourt
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1997
 *
 ****************************************************************************/

#include "trec.h"
#include <dsserver/DsLdataInfo.hh>

/*
 * file scope prototypes
 */

static void scale_field_data( MDV_handle_t *mdv, MDV_field_header_t *fhdr, 
			      int field_num, float ***field_data );

static void set_master_header( MDV_master_header_t *mmh, 
                               dimension_data_t *dim_data,
                               char *file0, char *file1 );

static void set_field_header( MDV_master_header_t *mmh, 
                              MDV_field_header_t *mfh, 
                              dimension_data_t *dim_data, 
                              int field_index,
                              trec_field_t *results );

static void set_scale_bias( MDV_field_header_t *mfh, 
                            dimension_data_t *dim_data,
                            float ***field_data );

/*
 * main routine
 */

int write_mdv_file( MDV_handle_t *mdv,
                    dimension_data_t *dim_data, 
                    char *file0, char *file1,
                    trec_field_t *results )
{

   int ifield;
   char output_path[NAMESIZE];

   if ( Glob->params.debug ) {
      printf( "%s: Writing mdv file\n", Glob->prog_name );
   }

   //
   // Compute the file name
   //
   if ( create_mdv_output_fname( output_path, &mdv->master_hdr ) == NULL )
     return FAILURE;
   
   //
   // Load Master Header
   //
   set_master_header( &mdv->master_hdr, dim_data, file0, file1 );

   //
   // Load Field Headers
   //
   for( ifield=0; ifield < NFIELDS_OUT; ifield++ ) {
     set_field_header( &mdv->master_hdr, 
		       mdv->fld_hdrs + ifield,
		       dim_data, ifield, &results[ifield] );
   }
   
   //
   // Load the data
   //

   for( ifield=0; ifield < NFIELDS_OUT; ifield++ ) {

     int j;
     int nbytes_plane;
     MDV_field_header_t *fhdr;

     /*
      * compute plane and vol sizes
      */
     
     fhdr = mdv->fld_hdrs + ifield;
     nbytes_plane = fhdr->nx * fhdr->ny;

     /*
      * alloc for planes
      */

     for (j = 0; j < fhdr->nz; j++) {
       mdv->field_plane[ifield][j] = umalloc(nbytes_plane);
     }

     /*
      * scale data
      */
     
     scale_field_data( mdv, fhdr, ifield, results[ifield].data );

   } /* ifield */

   /*
    * write the file
    */

   time_t centroid = mdv->master_hdr.time_centroid;

   if (MDV_write_all(mdv, output_path, MDV_PLANE_RLE8) == MDV_SUCCESS) {

     /*
      * write ldata file
      */

     DsLdataInfo  ldata(Glob->params.output_dir);
     ldata.setDataFileExt(Glob->params.output_file_suffix);
     if (ldata.write(centroid)) {
       fprintf(stderr, "WARNING - ptrec::write_mdv_file\n");
       fprintf(stderr, "DsLdata write failed\n");
     }

     return (SUCCESS);

   } else {

     return (FAILURE);

   }

}

static void set_master_header( MDV_master_header_t *mmh,
			      dimension_data_t *dim_data,
			      char *file0, char *file1 )
{
   int string_length;

   //
   // Since the master header was copied from one of the input datasets,
   // we only need to modifiy the fields that differ
   //
   strcpy( (char*)mmh->data_set_name, "TREC correlation calculations" );
   string_length = strlen(file0) + strlen(file1) + 4;
   if ( string_length < MDV_NAME_LEN ) {
      sprintf( (char*)mmh->data_set_source, "%s & %s", file0, file1 );
      mmh->data_set_info[0] = (signed char)'\0';
   }
   else if ( string_length < MDV_INFO_LEN ) {
      mmh->data_set_source[0] = (signed char)'\0';
      sprintf( (char*)mmh->data_set_info, "%s & %s", file0, file1 );
   }
   else {
      mmh->data_set_source[0] = (signed char)'\0';
      mmh->data_set_info[0] = (signed char)'\0';
   }

   mmh->data_collection_type = MDV_DATA_FORECAST;
   mmh->n_fields = NFIELDS_OUT;

   mmh->max_nx    = dim_data->nx; 
   mmh->max_ny    = dim_data->ny;
   mmh->max_nz    = dim_data->nz;

   mmh->vlevel_included = TRUE;
   mmh->vlevel_hdr_offset = 0;

   mmh->n_chunks = 0;
   mmh->chunk_hdr_offset = 0;

}

static void set_field_header( MDV_master_header_t *mmh,
			      MDV_field_header_t *mfh, 
			      dimension_data_t *dim_data,
			      int field_index, 
			      trec_field_t *results )
{
   mfh->nx        = dim_data->nx; 
   mfh->ny        = dim_data->ny;
   mfh->grid_minx = dim_data->min_x;
   mfh->grid_miny = dim_data->min_y;
   mfh->grid_dx   = dim_data->del_x;
   mfh->grid_dy   = dim_data->del_y;
   mfh->proj_type = MDV_PROJ_FLAT;

#ifdef NOTNOW
// from ctrec
    mfh[ifld].proj_origin_lon = gd->params.grid_origin_lon;
    mfh[ifld].proj_origin_lat = gd->params.grid_origin_lat;
#endif

   mfh->encoding_type = MDV_INT8;
   mfh->data_element_nbytes =  MDV_data_element_size( MDV_INT8 );
   mfh->volume_size = dim_data->nx * dim_data->ny * dim_data->nz * 
                      mfh->data_element_nbytes;

   mfh->field_data_offset = sizeof(MDV_master_header_t) +
                            (NFIELDS_OUT * sizeof(MDV_field_header_t)) +
                            (field_index * mfh->volume_size) +
                            ((field_index*2+1)*sizeof(mmh->record_len1));

//??? Rachel -- why strncpy???
   strcpy( (char*)mfh->field_name_long, results->name );
   strcpy( (char*)mfh->field_name, results->name );
   strcpy( (char*)mfh->units, results->units );
   mfh->transform[0] = '\0';

   set_scale_bias( mfh, dim_data, results->data );
   mfh->bad_data_value = 0.;
   mfh->missing_data_value = 0.;
}

static void set_scale_bias( MDV_field_header_t *mfh,
			    dimension_data_t *dim_data,
			    float ***field_data )
{
   int i, j, k;
   float value, min, max;

   //
   // find min and max for this field data
   //
   min =  32767;
   max = -32768;

   for( k=0; k < dim_data->nz; k++ ) {
      for( j=0; j < dim_data->ny; j++ ) {
         for( i=0; i < dim_data->nx; i++ ) {
            value = field_data[i][j][k];
            if ( value != BAD  &&  value > max )
               max = value;
            if ( value != BAD  &&  value < min )
               min = value;
         }
      }
   }

   //
   // use min and max to set scale and bias
   //
   mfh->scale = (max - min) / 250.;
   if ( fabs( mfh->scale - 0.0 ) <= 0.0001 ) {
      mfh->scale = 1.0;
   }
   mfh->bias = min - (2*mfh->scale);

   if ( Glob->params.debug ) {
      printf( "%s:    %s:\tmin = %.2f\tmax = %.2f\tscale = %.2f\n",
               Glob->prog_name, mfh->field_name, 
               min, max, mfh->scale );
   }
}

static void scale_field_data( MDV_handle_t *mdv, MDV_field_header_t *fhdr, 
			      int field_num, float ***field_data )

{
   int i, j, k;
   float value;
   ui08 *buf_ptr;
   double scale, bias;

   scale = fhdr->scale;
   bias = fhdr->bias;

   for( k=0; k < fhdr->nz; k++ ) {
     buf_ptr = (ui08 *) mdv->field_plane[field_num][k];
     for( j=0; j < fhdr->ny; j++ ) {
       for( i=0; i < fhdr->nx; i++ ) {
	 value = field_data[i][j][k];
	 if ( value == BAD )
	   *buf_ptr = 0;
	 else
	   *buf_ptr = (ui08)((value - bias) / scale);
	 buf_ptr++;
       }
     }
   }
}
