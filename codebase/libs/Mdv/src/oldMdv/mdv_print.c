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
/*****************************************************************
 * MDV_PRINT ROUTINES: print out headers and utility routines.
 * --Rachel Ames 1/96
 */


#include <Mdv/mdv/mdv_print.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_field_handle.h>
#include <rapformats/dobson.h>
#include <dataport/bigend.h>
#include <toolsa/utim.h>
#include <rapformats/ds_radar.h>
#include <rapformats/var_elev.h>

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifndef BOOL_STR
#define BOOL_STR(a) (a == FALSE ? "false" : "true")
#endif


/*
 * Forward declarations of static functions.
 */

static void print_dobson_vol_params(vol_params_t *vol_params,
				    FILE *outfile);
static void print_dobson_elevations(si32 *elev, long nelev,
				    FILE *outfile);
static void print_nowcast_timestamp(void *data,
                                    FILE *outfile);


/*****************************************************************
 * MDV_PRINT_MASTER_HEADER: print out some of the important master
 * header info
 * --Rachel Ames 1/96
 */

void MDV_print_master_header(MDV_master_header_t *mmh, FILE *outfile)
{

  time_t print_time;         /* temporary storage area for printing time */
                             /*   values -- necessary on systems which have */
                             /*   time_t defined as other than 32-bit */
  
/* print out master header */
   fprintf(outfile,"\n           MDV_print_master_header");
   fprintf(outfile,"\n           -----------------------\n");

   fprintf(outfile,"\nDataset Name:         %s",mmh->data_set_name);

   fprintf(outfile,"\nData Type:            %s",
           MDV_colltype2string(mmh->data_collection_type));

   print_time = mmh->time_begin;
   fprintf(outfile,"\nBegin Time:           %s",
	   asctime(gmtime(&print_time)));
   print_time = mmh->time_end;
   fprintf(outfile,  "End Time:             %s",
	   asctime(gmtime(&print_time)));

   fprintf(outfile,  "Number of Fields:     %d",mmh->n_fields);

   fprintf(outfile,"\nMax Grid Dimensions (x,y,z):(%d,%d,%d)",mmh->max_nx,
                    mmh->max_ny,mmh->max_nz);

   if (mmh->vlevel_included)
      fprintf(outfile,"\nVertical Level information included.");
   else
      fprintf(outfile,"\nVertical Level information not included.");

   if (mmh->field_grids_differ)
      fprintf(outfile,"\nField grids differ.");
   else         
      fprintf(outfile,"\nField grids do not differ.");

   fprintf(outfile,"\nDataset Source:       %s ",mmh->data_set_source);
   fprintf(outfile,"\nDataset Info:         %s ",mmh->data_set_info);
   fprintf(outfile,"\n");
}  
 
/*****************************************************************
 * MDV_PRINT_MASTER_HEADER_FULL: print out all of the master
 * header info
 * --Nancy Rehak 4/96
 */

void MDV_print_master_header_full(MDV_master_header_t *mmh, FILE *outfile)
{
  int i;
  time_t print_time;         /* temporary storage area for printing time */
                             /*   values -- necessary on systems which have */
                             /*   time_t defined as other than 32-bit */
  
  fprintf(outfile, "\n");
  fprintf(outfile, "           MDV_print_master_header\n");
  fprintf(outfile, "           -----------------------\n");
  fprintf(outfile, "\n");
  fprintf(outfile, "record_len1:          %d\n", mmh->record_len1);
  fprintf(outfile, "struct_id:            %d\n", mmh->struct_id);
  fprintf(outfile, "revision_number:      %d\n", mmh->revision_number);
  fprintf(outfile, "\n");
  print_time = mmh->time_gen;
  fprintf(outfile, "time_gen:             %s",
	  asctime(gmtime(&print_time)));
  fprintf(outfile, "user_time:           %d\n", mmh->user_time);
  print_time = mmh->time_begin;
  fprintf(outfile, "time_begin:           %s",
	  asctime(gmtime(&print_time)));
  print_time = mmh->time_end;
  fprintf(outfile, "time_end:             %s",
	  asctime(gmtime(&print_time)));
  print_time = mmh->time_centroid;
  fprintf(outfile, "time_centroid:        %s",
	  asctime(gmtime(&print_time)));
  print_time = mmh->time_expire;
  if (mmh->time_expire == 0)
     fprintf(outfile, "time_expire:          %d\n",mmh->time_expire);
  else
     fprintf(outfile, "time_expire:          %s",
	     asctime(gmtime(&print_time)));
  fprintf(outfile, "num_data_times:       %d\n", mmh->num_data_times);
  fprintf(outfile, "index_number:         %d\n", mmh->index_number);
  fprintf(outfile, "data_dimension:       %d\n", mmh->data_dimension);
  fprintf(outfile, "data_collection_type: %s\n",
	  MDV_colltype2string(mmh->data_collection_type));
  fprintf(outfile, "user_data:            %d\n", mmh->user_data);
  fprintf(outfile, "native_vlevel_type:   %s\n",
	  MDV_verttype2string(mmh->native_vlevel_type));
  fprintf(outfile, "vlevel_type:          %s\n",
	  MDV_verttype2string(mmh->vlevel_type));
  fprintf(outfile, "vlevel_included:      %s\n",
	  BOOL_STR(mmh->vlevel_included));
  fprintf(outfile, "grid_order_direction: %s\n",
	  MDV_orient2string(mmh->grid_order_direction));
  fprintf(outfile, "grid_order_indices:   %s\n",
	  MDV_order2string(mmh->grid_order_indices));
  fprintf(outfile, "n_fields:             %d\n", mmh->n_fields);
  fprintf(outfile, "max_nx:               %d\n", mmh->max_nx);
  fprintf(outfile, "max_ny:               %d\n", mmh->max_ny);
  fprintf(outfile, "max_nz:               %d\n", mmh->max_nz);
  fprintf(outfile, "n_chunks:             %d\n", mmh->n_chunks);
  fprintf(outfile, "field_hdr_offset:     %d\n", mmh->field_hdr_offset);
  fprintf(outfile, "vlevel_hdr_offset:    %d\n", mmh->vlevel_hdr_offset);
  fprintf(outfile, "chunk_hdr_offset:     %d\n", mmh->chunk_hdr_offset);
  fprintf(outfile, "field_grids_differ:   %s\n",
          BOOL_STR(mmh->field_grids_differ));
  for (i = 0; i < 8; i++)
    fprintf(outfile, "user_data_si32[%d]:    %d\n",
	    i, mmh->user_data_si32[i]);
  
  fprintf(outfile, "\n");
  for (i = 0; i < 6; i++)
    fprintf(outfile, "user_data_fl32[%d]:    %f\n",
	    i, mmh->user_data_fl32[i]);
  fprintf(outfile, "sensor_lon:           %f\n", mmh->sensor_lon);
  fprintf(outfile, "sensor_lat:           %f\n", mmh->sensor_lat);
  fprintf(outfile, "sensor_alt:           %f\n", mmh->sensor_alt);
  fprintf(outfile, "\n");
  fprintf(outfile, "data_set_info:        <%s>\n", mmh->data_set_info);
  fprintf(outfile, "data_set_name:        <%s>\n", mmh->data_set_name);
  fprintf(outfile, "data_set_source:      <%s>\n", mmh->data_set_source);
  fprintf(outfile, "\n");
  fprintf(outfile, "record_len2:          %d\n", mmh->record_len2);
  fprintf(outfile, "\n\n");
  
  return;
}  
 
/*****************************************************************
 * MDV_PRINT_FIELD_HEADER: print out some of the important
 * field header info
 * --Rachel Ames 2/96
 */
 
void MDV_print_field_header(MDV_field_header_t *fld_hdr, FILE *outfile)
{
  time_t print_time;         /* temporary storage area for printing time */
                             /*   values -- necessary on systems which have */
                             /*   time_t defined as other than 32-bit */
  
 
/* print out field header */
   fprintf(outfile,"\n           MDV_print_field_header");
   fprintf(outfile,"\n           -----------------------\n");
 
   fprintf(outfile,"\nField Name (long):    %s",fld_hdr->field_name_long);
   fprintf(outfile,"\nField Name (short):   %s",fld_hdr->field_name);
   fprintf(outfile,"\nUnits:                %s",fld_hdr->units);
   if (fld_hdr->field_code != 0)
      fprintf(outfile,"\nGrib Field Code:   %d",fld_hdr->field_code);
 
   fprintf(outfile,"\nEncoding Type:        %s",
                    MDV_encode2string(fld_hdr->encoding_type));

   fprintf(outfile, "\nForecast Delta:      %d",
	   fld_hdr->forecast_delta);

   print_time = fld_hdr->forecast_time;
   fprintf(outfile,"\nForecast Time:        %s",
                    asctime(gmtime(&print_time)));
 
   fprintf(outfile,  "(Nx,Ny,Nz):           (%d, %d, %d)",
                      fld_hdr->nx,fld_hdr->ny,fld_hdr->nz);
 
   fprintf(outfile,"\nGrid Spacing (dx,dy,dz): (%f, %f, %f)",
                      fld_hdr->grid_dx,fld_hdr->grid_dy,
                      fld_hdr->grid_dz);
 
   fprintf(outfile,"\nGrid Minimums (x,y,z)  : (%8.3f, %8.3f, %8.3f)",
                      fld_hdr->grid_minx,fld_hdr->grid_miny,
                      fld_hdr->grid_minz);
 
   fprintf(outfile,"\nOrigin (Long,Lat):    (%8.2f,%8.2f)",
                      fld_hdr->proj_origin_lon,fld_hdr->proj_origin_lat);
 
   fprintf(outfile,"\nProjection:           %s",
                    MDV_proj2string(fld_hdr->proj_type));

/* scale and bias */
   fprintf(outfile,"\nScale = %8.3f,      Bias = %8.3f ", 
                    fld_hdr->scale, fld_hdr->bias);
 
   fprintf(outfile,"\n");
}

/*****************************************************************
 * MDV_PRINT_FIELD_HEADER_FULL: print out all of the field header
 * info
 * --Nancy Rehak 4/96
 */
 
void MDV_print_field_header_full(MDV_field_header_t *fld_hdr, FILE *outfile)
{
  int i;
  time_t print_time;         /* temporary storage area for printing time */
                             /*   values -- necessary on systems which have */
                             /*   time_t defined as other than 32-bit */
  
  
  fprintf(outfile, "\n");
  fprintf(outfile, "           MDV_print_field_header\n");
  fprintf(outfile, "           -----------------------\n");
  fprintf(outfile, "\n");
  fprintf(outfile, "field_name_long:        <%s>\n", fld_hdr->field_name_long);
  fprintf(outfile, "field_name:             <%s>\n", fld_hdr->field_name);
  fprintf(outfile, "units:                  <%s>\n", fld_hdr->units);
  fprintf(outfile, "transform:              <%s>\n", fld_hdr->transform);
  fprintf(outfile, "\n");
  fprintf(outfile, "record_len2:            %d\n", fld_hdr->record_len2);
  fprintf(outfile, "\n");
  fprintf(outfile, "record_len1:            %d\n", fld_hdr->record_len1);
  fprintf(outfile, "struct_id:              %d\n", fld_hdr->struct_id);
  fprintf(outfile, "\n");
  fprintf(outfile, "field_code:             %d\n", fld_hdr->field_code);
  fprintf(outfile, "user_time1:             %d\n", fld_hdr->user_time1);
  fprintf(outfile, "forecast_delta:         %d\n", fld_hdr->forecast_delta);
  fprintf(outfile, "user_time2:             %d\n", fld_hdr->user_time2);
  fprintf(outfile, "user_time3:             %d\n", fld_hdr->user_time3);
  print_time = fld_hdr->forecast_time;
  fprintf(outfile, "forecast_time:          %s",
	  asctime(gmtime(&print_time)));
  fprintf(outfile, "user_time4:             %d\n", fld_hdr->user_time4);
  fprintf(outfile, "nx:                     %d\n", fld_hdr->nx);
  fprintf(outfile, "ny:                     %d\n", fld_hdr->ny);
  fprintf(outfile, "nz:                     %d\n", fld_hdr->nz);
  fprintf(outfile, "proj_type:              %s\n",
	  MDV_proj2string(fld_hdr->proj_type));
  fprintf(outfile, "encoding_type:          %s\n",
	  MDV_encode2string(fld_hdr->encoding_type));
  fprintf(outfile, "data_element_nbytes:    %d\n",
	  fld_hdr->data_element_nbytes);
  fprintf(outfile, "field_data_offset:      %d\n", fld_hdr->field_data_offset);
  fprintf(outfile, "volume_size:            %d\n", fld_hdr->volume_size);
  for (i = 0; i < 10; i++) {
    fprintf(outfile, "user_data_si32[%d]:      %d\n",
	    i, fld_hdr->user_data_si32[i]);
  }
  fprintf(outfile, "compression_type:       %s\n",
	  MDV_compression2string(fld_hdr->compression_type));
  fprintf(outfile, "transform_type:         %s\n",
	  MDV_transform2string(fld_hdr->transform_type));
  fprintf(outfile, "scaling_type:           %s\n",
	  MDV_scaling2string(fld_hdr->scaling_type));
  fprintf(outfile, "native_vlevel_type:     %s\n",
	  MDV_verttype2string(fld_hdr->native_vlevel_type));
  fprintf(outfile, "vlevel_type:            %s\n",
	  MDV_verttype2string(fld_hdr->vlevel_type));
  fprintf(outfile, "dz_constant:            %d\n", fld_hdr->dz_constant);
  
  fprintf(outfile, "\n");
  fprintf(outfile, "proj_origin_lon:        %f\n", fld_hdr->proj_origin_lon);
  fprintf(outfile, "proj_origin_lat:        %f\n", fld_hdr->proj_origin_lat);
  fprintf(outfile, "proj_rotation:          %f\n", fld_hdr->proj_rotation);
  for (i = 0; i < MDV_MAX_PROJ_PARAMS; i++)
    fprintf(outfile, "proj_param[%02d]:         %f\n",
	    i, fld_hdr->proj_param[i]);
  fprintf(outfile, "vert_reference:         %f\n", fld_hdr->vert_reference);
  fprintf(outfile, "\n");
  fprintf(outfile, "grid_dx:                %f\n", fld_hdr->grid_dx);
  fprintf(outfile, "grid_dy:                %f\n", fld_hdr->grid_dy);
  fprintf(outfile, "grid_dz:                %f\n", fld_hdr->grid_dz);
  fprintf(outfile, "grid_minx:              %f\n", fld_hdr->grid_minx);
  fprintf(outfile, "grid_miny:              %f\n", fld_hdr->grid_miny);
  fprintf(outfile, "grid_minz:              %f\n", fld_hdr->grid_minz);
  fprintf(outfile, "scale:                  %f\n", fld_hdr->scale);
  fprintf(outfile, "bias:                   %f\n", fld_hdr->bias);
  fprintf(outfile, "bad_data_value:         %f\n", fld_hdr->bad_data_value);
  fprintf(outfile, "missing_data_value:     %f\n",
	  fld_hdr->missing_data_value);
  fprintf(outfile, "proj_rotation:          %f\n", fld_hdr->proj_rotation);
  for (i = 0; i < 4; i++) {
    fprintf(outfile, "user_data_fl32[%d]:      %f\n",
	    i, fld_hdr->user_data_fl32[i]);
  }
  if (fld_hdr->min_value != 0.0 || fld_hdr->max_value != 0.0) {
    fprintf(outfile, "min_value:              %f\n", fld_hdr->min_value);
    fprintf(outfile, "max_value:              %f\n", fld_hdr->max_value);
  }
  fprintf(outfile, "\n\n");
  
  return;
}

/*****************************************************************
 * MDV_PRINT_VLEVEL_HEADER: print out some of the important 
 * vlevel header info
 * --Rachel Ames 1/96
 */

void MDV_print_vlevel_header(MDV_vlevel_header_t *mvh, int nz, 
                             char *field_name,FILE *outfile)
{ 

   int vlevel;

/* print out vlevel header */
   fprintf(outfile,"\n           Vlevel_header for %s   ",field_name);

/* print out vertical level info */
   for (vlevel = 0; vlevel < nz; vlevel++)  {
      if ((vlevel == 0) || ((vlevel >= 1) &&
         ((int)mvh->vlevel_type[vlevel] !=(int)mvh->vlevel_type[vlevel-1])))
         fprintf(outfile,"\nVertical Level Type:    %s ",
              MDV_verttype2string((int)mvh->vlevel_type[vlevel]));

      fprintf(outfile,"\nParameter(%2d) =\t\t%f ",vlevel+1,
                          mvh->vlevel_params[vlevel]);
   }

   fprintf(outfile,"\n");
}
 

 
/*****************************************************************
 * MDV_PRINT_VLEVEL_HEADER_FULL: print out all of the vlevel
 * header info
 * --Nancy Rehak 4/96
 */

void MDV_print_vlevel_header_full(MDV_vlevel_header_t *mvh, int nz, 
				  char *field_name,FILE *outfile)
{ 
  int i;
  
  fprintf(outfile, "\n");
  fprintf(outfile, "           Vlevel_header for %s\n",field_name);
  fprintf(outfile, "\n");
  fprintf(outfile, "record_len1:             %d\n", mvh->record_len1);
  fprintf(outfile, "struct_id:               %d\n", mvh->struct_id);
  fprintf(outfile, "\n");
  for (i = 0; i < nz; i++)
  {
    fprintf(outfile, "vlevel_type[%02d]:         %s\n",
	    i, MDV_verttype2string(mvh->vlevel_type[i]));
    
    fprintf(outfile, "vlevel_params[%02d]:       %f\n",
	    i, mvh->vlevel_params[i]);
  }
  fprintf(outfile, "\n");
  fprintf(outfile, "record_len2:             %d\n", mvh->record_len2);
  fprintf(outfile, "\n\n");
  
  return;
}
 

 
/*****************************************************************
 * MDV_PRINT_FIELD_VLEVEL_HEADER: print out some of the important 
 * field_vlevel header info
 * --Rachel Ames 1/96
 */

void MDV_print_field_vlevel_header(MDV_field_vlevel_header_t *mfvh, FILE *outfile)
{

/* print out field header */
   MDV_print_field_header(mfvh->fld_hdr,outfile);

/* print out vlevel header if there is one*/
   if (mfvh->vlv_hdr != NULL) {
       MDV_print_vlevel_header(mfvh->vlv_hdr,mfvh->fld_hdr->nz,
                               (char *)&mfvh->fld_hdr->field_name_long,outfile);
   }

   fprintf(outfile,"\n");
}  


/*****************************************************************
 * MDV_PRINT_FIELD_VLEVEL_HEADER_FULL: print out all of the
 * field_vlevel header info
 * --Nancy Rehak 4/96
 */

void MDV_print_field_vlevel_header_full(MDV_field_vlevel_header_t *mfvh,
					FILE *outfile)
{
  MDV_print_field_header_full(mfvh->fld_hdr,
			      outfile);

  if (mfvh->vlv_hdr != NULL)
  {
    MDV_print_vlevel_header_full(mfvh->vlv_hdr, 
				 mfvh->fld_hdr->nz,
				 (char *)&mfvh->fld_hdr->field_name_long,
				 outfile);
  }

  fprintf(outfile,"\n");
}  


/*****************************************************************
 * MDV_PRINT_CHUNK_HEADER: print out some of the important 
 * chunk header info
 * --Rachel Ames 1/96
 */

void MDV_print_chunk_header(MDV_chunk_header_t *mch, FILE *outfile)
{

/* print out chunk header */
   fprintf(outfile,"\n           MDV_print_chunk_header");
   fprintf(outfile,"\n           -----------------------\n");

   fprintf(outfile,"\nChunk Number:    %d",mch->chunk_id);
   fprintf(outfile,"\nChunk Offset:    %d",mch->chunk_data_offset);
   fprintf(outfile,"\nChunk Size:      %d",mch->size);
   fprintf(outfile,"\nChunk Info:      %s",mch->info);
 
   fprintf(outfile,"\n");
   
}


/*****************************************************************
 * MDV_PRINT_CHUNK_HEADER_FULL: print out all of the chunk header
 * info
 * --Nancy Rehak 4/96
 */

void MDV_print_chunk_header_full(MDV_chunk_header_t *mch, FILE *outfile)
{
  fprintf(outfile, "\n");
  fprintf(outfile, "           MDV_print_chunk_header\n");
  fprintf(outfile, "           -----------------------\n");
  fprintf(outfile, "\n");
  fprintf(outfile, "record_len1:           %d\n", mch->record_len1);
  fprintf(outfile, "struct_id:             %d\n", mch->struct_id);
  fprintf(outfile, "\n");
  fprintf(outfile, "chunk_id:              %d\n", mch->chunk_id);
  fprintf(outfile, "chunk_data_offset:     %d\n", mch->chunk_data_offset);
  fprintf(outfile, "size:                  %d\n", mch->size);
  fprintf(outfile, "\n");
  fprintf(outfile, "info:                  <%s>\n", mch->info);
  fprintf(outfile, "\n");
  fprintf(outfile, "record_len2:           %d\n", mch->record_len2);
  fprintf(outfile, "\n\n");
  
  return;
}


/*****************************************************************
 * MDV_PRINT_CHUNK_DATA_FULL: print out all of the chunk data if
 *                            it is in a known format
 *
 * Assumes chunk has been swapped into host byte order.
 *
 * --Nancy Rehak 7/96
 */

void MDV_print_chunk_data_full(void *data, si32 chunk_id,
			       si32 size, FILE *outfile)
{
  fprintf(outfile, "\n");
  fprintf(outfile, "           MDV_print_chunk_data\n");
  fprintf(outfile, "           --------------------\n");
  fprintf(outfile, "\n");

  switch(chunk_id)
  {
  case MDV_CHUNK_DOBSON_VOL_PARAMS :
    print_dobson_vol_params((vol_params_t *)data, outfile);
    break;
    
  case MDV_CHUNK_DOBSON_ELEVATIONS :
    print_dobson_elevations((si32 *)data, size / sizeof(si32), outfile);
    break;
    
  case MDV_CHUNK_NOWCAST_DATA_TIMES :
      /*
    //
    // should eventually move the print functionality to
    // the relevant TimeStamp class, but there are some
    // compiler details that aren't working out right now
    // {
    // Stamp timeStamp( data, size );
    // timeStamp.print( outfile );
    // }
    //
    */
    print_nowcast_timestamp( data, outfile ); 
    break;

  case MDV_CHUNK_DSRADAR_PARAMS :
    DsRadarParams_print(outfile, "  ", (DsRadarParams_t *) data);
    break;
    
  case MDV_CHUNK_DSRADAR_ELEVATIONS :
    {
      DsRadarElev_t elev;
      DsRadarElev_init(&elev);
      DsRadarElev_unload_chunk(&elev, (ui08 *) data, size);
      DsRadarElev_print(outfile, "  ", &elev);
      DsRadarElev_free(&elev);
    }
    break;
    
  case MDV_CHUNK_VARIABLE_ELEV :
    VAR_ELEV_print(outfile, data, (int)size);
    break;

  case MDV_CHUNK_SOUNDING_DATA:
     print_sounding_chunk(outfile, (sounding_chunk_t *) data);
     break;

  default:
    fprintf(outfile, "Unknown format for chunk data\n");
    break;
  
  }

  fprintf(outfile, "\n\n");

}


/*****************************************************************
 * MDV_PRINT_FIELD_PLANE_FULL: print out all of the data for the
 *                             given plane
 * --Nancy Rehak 7/96
 */

void MDV_print_field_plane_full(MDV_field_header_t *fhdr,
				void *plane_ptr,
				int field_num, int plane_num,
				FILE *outfile)
{

  MDV_print_plane(fhdr, plane_ptr, field_num, plane_num,
		  MDV_ASIS, outfile);

}

/*****************************************************************
 * MDV_PRINT_PLANE: print out all of the data for the
 *                  given plane and data type
 * --Mike Dixon 5/99
 */

void MDV_print_plane(MDV_field_header_t *fhdr,
		     void *plane_ptr,
		     int field_num, int plane_num,
		     int encoding_type,
		     FILE *outfile)

{

  MDV_field_handle_t *fhand;
  int plane_size;

  fprintf(outfile, "\n");
  fprintf(outfile, "           MDV_print_field_plane -- field %d, plane %d\n",
	  field_num, plane_num);
  fprintf(outfile, "           -------------------------------------------\n");
  fprintf(outfile, "\n");

  if (MDV_compressed(fhdr->compression_type)) {
    fprintf(outfile, "         Compressed field - not printable\n");
    return;
  }
  
  /*
   * create field handle just for this plane
   */
  
  plane_size = fhdr->nx * fhdr->ny * fhdr->data_element_nbytes;
  fhand =
    MDV_fhand_create_plane_from_parts(fhdr, plane_num, plane_ptr,
				      plane_size);

  /*
   * convert to data type
   */

  MDV_fhand_convert(fhand, encoding_type, MDV_COMPRESSION_NONE,
		    MDV_SCALING_ROUNDED, 0.0, 0.0);
  
  /*
   * print it out
   */
  
  MDV_fhand_print_voldata(fhand, outfile, TRUE, FALSE, TRUE);

  MDV_fhand_delete(fhand);

}


/*****************************************************************
 * MDV_VERTTYPE2STRING: Convert the vertical type integer to ascii 
 * string.  See mdv_macros for the vertical type declarations.
 * --Rachel Ames 1/96
 */

char * MDV_verttype2string(int vert_type)
{

   switch(vert_type)  {
      case MDV_VERT_TYPE_SURFACE : 
        return("Surface"); 
      case MDV_VERT_TYPE_SIGMA_P :
	return("Sigma P"); 
      case MDV_VERT_TYPE_PRESSURE :
	return("Pressure (units mb)"); 
      case MDV_VERT_TYPE_Z :
	return("Constant Altitude (units KM MSL)"); 
      case MDV_VERT_TYPE_SIGMA_Z :
	return("Sigma Z"); 
      case MDV_VERT_TYPE_ETA :
	return("ETA"); 
      case MDV_VERT_TYPE_THETA :
	return("Theta"); 
      case MDV_VERT_TYPE_MIXED :
	return("Mixed"); 
      case MDV_VERT_TYPE_ELEV :
	return("Elevation Angles"); 
      case MDV_VERT_TYPE_COMPOSITE :
	return("Composite"); 
      case MDV_VERT_TYPE_CROSS_SEC :
	return("Cross Secional View"); 
      case MDV_VERT_SATELLITE_IMAGE :
	return("Satelite Image"); 
      case MDV_VERT_VARIABLE_ELEV:
	return("Variable elevation scan"); 
      case MDV_VERT_FIELDS_VAR_ELEV:
	return("Field specifc Var. elev. scan"); 
      case MDV_VERT_FLIGHT_LEVEL:
	return("Flight level"); 
      default:
	return("Unknown Vertical Type"); 
   }
}  
 

/*****************************************************************
 * MDV_PROJ2STRING: Convert the projection type integer to 
 * an ascii string.  See mdv_macros.h for enumeration types.
 * --Rachel Ames 1/96
 */

char * MDV_proj2string(int proj_type)
{

   switch(proj_type)  {
      case MDV_PROJ_NATIVE :
        return("Native"); 
      case MDV_PROJ_LATLON :
	return("Latitude/Longitude Grid (units in degrees)"); 
      case MDV_PROJ_ARTCC :
	return("ARTCC"); 
      case MDV_PROJ_STEREOGRAPHIC :
	return("Stereographice"); 
      case MDV_PROJ_LAMBERT_CONF :
	return("Lambert Conformal"); 
      case MDV_PROJ_MERCATOR :
	return("Mercator"); 
      case MDV_PROJ_POLAR_STEREO :
	return("Polar Stereographic"); 
      case MDV_PROJ_POLAR_ST_ELLIP :
	return("Polar Sterographic Equidistant"); 
      case MDV_PROJ_CYL_EQUIDIST :
	return("Cylindrical Equidistant"); 
      case MDV_PROJ_FLAT :
	return("Flat (Cartesian) (units in KM)"); 
      case MDV_PROJ_POLAR_RADAR :
	return("Polar Radar"); 
      case MDV_PROJ_RADIAL :
	return("Radial"); 
      default:
	return("Unknown Projection Type"); 
   }

}  
 

/*****************************************************************
 * MDV_ENCODE2STRING: Convert the encoding type integer to ascii 
 * string.  See mdv_macros for the encoding type declarations.
 * --Rachel Ames 1/96
 */

char * MDV_encode2string(int encodeing_type)
{

   switch(encodeing_type)  {
      case MDV_NATIVE :
        return("NATIVE"); 
      case MDV_INT8 :
	return("MDV_INT8 (CHAR/BYTE)"); 
      case MDV_INT16:
	return("MDV_INT16 (SHORT)"); 
      case MDV_FLOAT32 :
	return("MDV_FLOAT32 (FLOAT)"); 
      case MDV_PLANE_RLE8 :
	return("MDV_PLANE_RLE8"); 
      default:
	return("Unknown Encoding Type"); 
   }
}  
 
/*****************************************************************
 * MDV_COLLTYPE2STRING: Convert the collection type integer to an
 * ascii string.  See mdv_macros for the vertical type declarations.
 * --Rachel Ames 1/96
 */

char * MDV_colltype2string(int coll_type)
{

   switch(coll_type)  {
      case MDV_DATA_MEASURED :
        return("Measured");
      case MDV_DATA_EXTRAPOLATED :
	return("Extrapolated");
      case MDV_DATA_FORECAST :
	return("Forecast");
      case MDV_DATA_SYNTHESIS :
	return("Synthesis");
      case MDV_DATA_MIXED :
	return("Mixed");
      default:
	return("Unknown Collection Type");
   }
}


/*****************************************************************
 * MDV_ORIENT2STRING: Convert the data orientation integer to an
 * ascii string.  See mdv_macros for the orientation type declarations.
 * --Nancy Rehak 4/96
 */

char * MDV_orient2string(int orient_type)
{
  switch(orient_type)
  {
  case MDV_ORIENT_OTHER :
    return("MDV_ORIENT_OTHER");
  case MDV_ORIENT_SN_WE :
    return("MDV_ORIENT_SN_WE");
  case MDV_ORIENT_NS_WE :
    return("MDV_ORIENT_NS_WE");
  case MDV_ORIENT_SN_EW :
    return("MDV_ORIENT_SN_EW");
  case MDV_ORIENT_NS_EW :
    return("MDV_ORIENT_NS_EW");
  default:
    return("Unknown Orientation");
  }
}


/*****************************************************************
 * MDV_ORDER2STRING: Convert the data order integer to an
 * ascii string.  See mdv_macros for the data order declarations.
 * --Nancy Rehak 4/96
 */

char * MDV_order2string(int order_type)
{
  switch(order_type)
  {
  case MDV_ORDER_XYZ :
    return("MDV_ORDER_XYZ");
  case MDV_ORDER_YXZ :
    return("MDV_ORDER_YXZ");
  case MDV_ORDER_XZY :
    return("MDV_ORDER_XZY");
  case MDV_ORDER_YZX :
    return("MDV_ORDER_YZX");
  case MDV_ORDER_ZXY :
    return("MDV_ORDER_ZXY");
  case MDV_ORDER_ZYX :
    return("MDV_ORDER_ZYX");
  default:
    return("Unknown Data Order");
  }
}


/*****************************************************************
 * MDV_COMPRESSION2STRING: Convert the compression type integer to an
 * ascii string.  See mdv_macros for the data order declarations.
 */

char * MDV_compression2string(int compression_type)
{
  switch(compression_type)
  {
  case MDV_COMPRESSION_NONE :
    return("MDV_COMPRESSION_NONE");
  case MDV_COMPRESSION_RLE :
    return("MDV_COMPRESSION_RLE");
  case MDV_COMPRESSION_LZO :
    return("MDV_COMPRESSION_LZO");
  case MDV_COMPRESSION_ZLIB :
    return("MDV_COMPRESSION_ZLIB");
  case MDV_COMPRESSION_BZIP :
    return("MDV_COMPRESSION_BZIP");
  case MDV_COMPRESSION_GZIP :
    return("MDV_COMPRESSION_GZIP");
  default:
    return("Unknown compression type");
  }
}


/*****************************************************************
 * MDV_TRANSFORM2STRING: Convert the transform type integer to an
 * ascii string.  See mdv_macros for the data order declarations.
 */

char * MDV_transform2string(int transform_type)
{
  switch(transform_type)
  {
  case MDV_TRANSFORM_NONE :
    return("MDV_TRANSFORM_NONE");
  case MDV_TRANSFORM_LOG :
    return("MDV_TRANSFORM_LOG");
  default:
    return("Unknown transform type");
  }
}


/*****************************************************************
 * MDV_SCALING2STRING: Convert the scaling type integer to an
 * ascii string.  See mdv_macros for the data order declarations.
 */

char * MDV_scaling2string(int scaling_type)
{
  switch(scaling_type)
  {
  case MDV_SCALING_NONE :
    return("MDV_SCALING_NONE");
  case MDV_SCALING_ROUNDED :
    return("MDV_SCALING_ROUNDED");
  case MDV_SCALING_DYNAMIC :
    return("MDV_SCALING_DYNAMIC");
  case MDV_SCALING_INTEGRAL :
    return("MDV_SCALING_INTEGRAL");
  case MDV_SCALING_SPECIFIED :
    return("MDV_SCALING_SPECIFIED");
  default:
    return("Unknown scaling type");
  }
}


/*****************************************************************
 * PRINT_DOBSON_VOL_PARAMS: print out the dobson vol params 
 * stored as chunk data
 * --Nancy Rehak 7/96
 */

static void print_dobson_vol_params(vol_params_t *vol_params,
				    FILE *outfile)
{
  fprintf(outfile, "file_time:      %02d/%02d/%d %02d:%02d:%02d\n",
	  vol_params->file_time.month,
	  vol_params->file_time.day,
	  vol_params->file_time.year,
	  vol_params->file_time.hour,
	  vol_params->file_time.min,
	  vol_params->file_time.sec);
  fprintf(outfile, "start_time:     %02d/%02d/%d %02d:%02d:%02d\n",
	  vol_params->start_time.month,
	  vol_params->start_time.day,
	  vol_params->start_time.year,
	  vol_params->start_time.hour,
	  vol_params->start_time.min,
	  vol_params->start_time.sec);
  fprintf(outfile, "mid_time:       %02d/%02d/%d %02d:%02d:%02d\n",
	  vol_params->mid_time.month,
	  vol_params->mid_time.day,
	  vol_params->mid_time.year,
	  vol_params->mid_time.hour,
	  vol_params->mid_time.min,
	  vol_params->mid_time.sec);
  fprintf(outfile, "end_time:       %02d/%02d/%d %02d:%02d:%02d\n",
	  vol_params->end_time.month,
	  vol_params->end_time.day,
	  vol_params->end_time.year,
	  vol_params->end_time.hour,
	  vol_params->end_time.min,
	  vol_params->end_time.sec);
  fprintf(outfile, "\n");
  fprintf(outfile, "nbytes_char:        %d\n",
	  vol_params->radar.nbytes_char);
  fprintf(outfile, "radar_id:           %d\n",
	  vol_params->radar.radar_id);
  fprintf(outfile, "altitude:           %d meters\n",
	  vol_params->radar.altitude);
  fprintf(outfile, "latitude:           %f degrees\n",
	  (double)vol_params->radar.latitude / 1000000.0);
  fprintf(outfile, "longitude:          %f degrees\n",
	  (double)vol_params->radar.longitude / 1000000.0);
  fprintf(outfile, "nelevations:        %d\n",
	  vol_params->radar.nelevations);
  fprintf(outfile, "nazimuths:          %d\n",
	  vol_params->radar.nazimuths);
  fprintf(outfile, "ngates:             %d\n",
	  vol_params->radar.ngates);
  fprintf(outfile, "gate_spacing:       %d mm\n",
	  vol_params->radar.gate_spacing);
  fprintf(outfile, "start_range:        %d mm\n",
	  vol_params->radar.start_range);
  fprintf(outfile, "delta_azimuth:      %f degrees\n",
	  (double)vol_params->radar.delta_azimuth / 1000000.0);
  fprintf(outfile, "start_azimuth:      %f degrees\n",
	  (double)vol_params->radar.start_azimuth / 1000000.0);
  fprintf(outfile, "beam_width:         %f degrees\n",
	  (double)vol_params->radar.beam_width / 1000000.0);
  fprintf(outfile, "samples_per_beam:   %d\n",
	  vol_params->radar.samples_per_beam);
  fprintf(outfile, "pulse_width:        %d nano-seconds\n",
	  vol_params->radar.pulse_width);
  fprintf(outfile, "prf:                %f\n",
	  (double)vol_params->radar.prf / 1000.0);
  fprintf(outfile, "wavelength:         %d micro-meters\n",
	  vol_params->radar.wavelength);
  fprintf(outfile, "nmissing:           %d\n",
	  vol_params->radar.nmissing);
  fprintf(outfile, "name:               <%s>\n",
	  vol_params->radar.name);
  fprintf(outfile, "\n");
  fprintf(outfile, "nbytes_char:        %d\n",
	  vol_params->cart.nbytes_char);
  fprintf(outfile, "latitude:           %f degrees\n",
	  (double)vol_params->cart.latitude / 1000000.0);
  fprintf(outfile, "longitude:          %f degrees\n",
	  (double)vol_params->cart.longitude / 1000000.0);
  fprintf(outfile, "rotation:           %f degrees\n",
	  (double)vol_params->cart.rotation / 1000000.0);
  fprintf(outfile, "nx:                 %d\n",
	  vol_params->cart.nx);
  fprintf(outfile, "ny:                 %d\n",
	  vol_params->cart.ny);
  fprintf(outfile, "nz:                 %d\n",
	  vol_params->cart.nz);
  fprintf(outfile, "minx:               %f\n",
	  (double)vol_params->cart.minx / (double)vol_params->cart.scalex);
  fprintf(outfile, "miny:               %f\n",
	  (double)vol_params->cart.miny / (double)vol_params->cart.scaley);
  if (vol_params->cart.minz == -1)
    fprintf(outfile, "minz:               -1\n");
  else
    fprintf(outfile, "minz:               %f\n",
	    (double)vol_params->cart.minz / (double)vol_params->cart.scalez);
  fprintf(outfile, "dx:                 %f\n",
	  (double)vol_params->cart.dx / (double)vol_params->cart.scalex);
  fprintf(outfile, "dy:                 %f\n",
	  (double)vol_params->cart.dy / (double)vol_params->cart.scaley);
  if (vol_params->cart.dz == -1)
    fprintf(outfile, "dz:                 -1\n");
  else
    fprintf(outfile, "dz:                 %f\n",
	    (double)vol_params->cart.dz / (double)vol_params->cart.scalez);
  fprintf(outfile, "radarx:             %f\n",
	  (double)vol_params->cart.radarx / (double)vol_params->cart.scalex);
  fprintf(outfile, "radary:             %f\n",
	  (double)vol_params->cart.radary / (double)vol_params->cart.scaley);
  fprintf(outfile, "radarz:             %f\n",
	  (double)vol_params->cart.radarz / (double)vol_params->cart.scalez);
  fprintf(outfile, "scalex:             %d\n",
	  vol_params->cart.scalex);
  fprintf(outfile, "scaley:             %d\n",
	  vol_params->cart.scaley);
  fprintf(outfile, "scalez:             %d\n",
	  vol_params->cart.scalez);
  fprintf(outfile, "km_scalex:          %d\n",
	  vol_params->cart.km_scalex);
  fprintf(outfile, "km_scaley:          %d\n",
	  vol_params->cart.km_scaley);
  fprintf(outfile, "km_scalez:          %d\n",
	  vol_params->cart.km_scalez);
  fprintf(outfile, "dz_constant:        %d\n",
	  vol_params->cart.dz_constant);
  fprintf(outfile, "unitsx:             <%s>\n",
	  vol_params->cart.unitsx);
  fprintf(outfile, "unitsy:             <%s>\n",
	  vol_params->cart.unitsy);
  fprintf(outfile, "unitsz:             <%s>\n",
	  vol_params->cart.unitsz);
  fprintf(outfile, "\n");
  fprintf(outfile, "nfields:            %d\n",
	  vol_params->nfields);
  
}


/*****************************************************************
 * PRINT_DOBSON_ELEVATIONS: print out the dobson elevations
 * stored as chunk data
 * --Nancy Rehak 7/96
 */

static void print_dobson_elevations(si32 *elevations, long n_elev,
				    FILE *outfile)
{
  int i;
  
  for (i = 0; i < n_elev; i++)
    fprintf(outfile, "Elevation[%02d] = %9.6f\n",
	    i, (float)elevations[i] / 1000000.0);
}

static void print_nowcast_timestamp( void *data, FILE *outfile )
{
   /*
    * This structure is "officially" defined in the class file TimeStamp.hh
    * which is where this printing should be done.
    */
   typedef struct {     
      si32   when;
      char   who[MDV_LONG_FIELD_LEN];
   } timestamp_t;

   /*
    * Swap the timestamp out of the mdv chunk
    */
   time_t       when;
   char        *who;
   timestamp_t *timestamp;

   timestamp = (timestamp_t*)data;
   when      = (time_t)BE_to_si32( timestamp->when );
   who       = timestamp->who;

   /*
    * Convert the time to a string
    */
   {
     char       whenChar[20];
     UTIMstruct whenUtim;
     
     if ( when > 0 ) {
       UTIMunix_to_date( when, &whenUtim );
       sprintf( whenChar, "%4ld/%02ld/%02ld %2ld:%02ld:%02ld",
		whenUtim.year, whenUtim.month, whenUtim.day,
		whenUtim.hour, whenUtim.min, whenUtim.sec );
     }
     else {
       strcpy( whenChar, "0000/00/00 00:00:00" );
     }

     /*
      * Print it out
      */
     fprintf( outfile, "Timestamp: %s  %s", whenChar, who );
   }
}
