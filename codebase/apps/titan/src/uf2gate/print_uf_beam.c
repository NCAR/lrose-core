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
/***************************************************************************
 * print_uf_beam.c
 *
 * prints details of a radar beam buffer to stdout
 *
 * Nancy Rehak RAP NCAR Boulder CO USA
 *
 * Mar 1995
 *
 **************************************************************************/

#include <stdio.h>

#include <toolsa/os_config.h>

#include <rapformats/uf_headers.h>

void print_uf_beam(si16 *beam_data)

{
  int i, j;
  
  uf_mandatory_header_t   *man_hdr;
  uf_optional_header_t    *opt_hdr;
  uf_data_hdr_info_t      *data_hdr;
  uf_field_header_t       *field_hdr;
  
  si16                   *data_ptr;
  
  /*
   * Position the header pointers within the buffer.
   */

  man_hdr = (uf_mandatory_header_t *)beam_data;

  if (man_hdr->optional_header_pos == man_hdr->local_use_header_pos)
    opt_hdr = NULL;
  else
    opt_hdr = (uf_optional_header_t *)
      (beam_data + man_hdr->optional_header_pos - 1);
  
  data_hdr = (uf_data_hdr_info_t *)(beam_data + man_hdr->data_header_pos - 1);
  
  /*
   * Print out the mandatory header information.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "***************************************\n");
  fprintf(stdout, "MANDATORY HEADER\n");
  fprintf(stdout, "\n");
  
  fprintf(stdout, "UF string:                     %c%c\n",
	 man_hdr->uf_string[0], man_hdr->uf_string[1]);
  fprintf(stdout, "record length:                 %d\n",
	 man_hdr->record_length);
  fprintf(stdout, "optional header position:      %d\n",
	 man_hdr->optional_header_pos);
  fprintf(stdout, "local use header position:     %d\n",
	 man_hdr->local_use_header_pos);
  fprintf(stdout, "data header position:          %d\n",
	 man_hdr->data_header_pos);
  fprintf(stdout, "physical record number:        %d\n",
	 man_hdr->record_num);
  fprintf(stdout, "volume scan number:            %d\n",
	 man_hdr->volume_scan_num);
  fprintf(stdout, "ray number within volume:      %d\n",
	 man_hdr->ray_num);
  fprintf(stdout, "physical record num w/in ray:  %d\n",
	 man_hdr->ray_record_num);
  fprintf(stdout, "sweep number within volume:    %d\n",
	 man_hdr->sweep_num);
  
  fprintf(stdout, "radar name:                    ");
  for (i = 0; i < 8; i++)
    fprintf(stdout, "%c", man_hdr->radar_name[i]);
  fprintf(stdout, "\n");
  
  fprintf(stdout, "site name:                     ");
  for (i = 0; i < 8; i++)
    fprintf(stdout, "%c", man_hdr->site_name[i]);
  fprintf(stdout, "\n");
  
  fprintf(stdout, "latitude:                      %d:%02d:%f\n",
	 man_hdr->lat_degrees,
	 man_hdr->lat_minutes,
	 (float)man_hdr->lat_seconds / 64.0);
  
  fprintf(stdout, "longitude:                     %d:%02d:%f\n",
	 man_hdr->lon_degrees,
	 man_hdr->lon_minutes,
	 (float)man_hdr->lon_seconds / 64.0);
  
  fprintf(stdout, "antenna height:                %d m\n",
	 man_hdr->antenna_height);
  
  fprintf(stdout, "data time:                     %d/%02d/%d %d:%02d:%02d\n",
	 man_hdr->month,
	 man_hdr->day,
	 man_hdr->year,
	 man_hdr->hour,
	 man_hdr->minute,
	 man_hdr->second);
  
  fprintf(stdout, "time zone:                     %c%c\n",
	 man_hdr->time_zone[0], man_hdr->time_zone[1]);
  fprintf(stdout, "azimuth:                       %f degrees\n",
	 (float)man_hdr->azimuth / 64.0);
  fprintf(stdout, "elevation:                     %f degrees\n",
	 (float)man_hdr->elevation / 64.0);

  fprintf(stdout, "sweep mode:                    ");
  switch(man_hdr->sweep_mode)
  {
  case 0 :
    fprintf(stdout, "Calibration\n");
    break;
  case 1 :
    fprintf(stdout, "PPI (constant elevation)\n");
    break;
  case 2 :
    fprintf(stdout, "Coplane\n");
    break;
  case 3 :
    fprintf(stdout, "RHI (constant azimuth)\n");
    break;
  case 4 :
    fprintf(stdout, "Vertical\n");
    break;
  case 5 :
    fprintf(stdout, "Target (stationary)\n");
    break;
  case 6 :
    fprintf(stdout, "Manual\n");
    break;
  case 7 :
    fprintf(stdout, "Idle (out of control)\n");
    break;
  default:
    fprintf(stdout, "UNKNOWN (value = %d)\n",
	   man_hdr->sweep_mode);
    break;
  } /* endswitch - man_hdr->sweep_mode */
  
  fprintf(stdout, "fixed angle:                   %f degrees\n",
	 (float)man_hdr->fixed_angle / 64.0);
  fprintf(stdout, "sweep rate:                    %f degrees/sec\n",
	 (float)man_hdr->sweep_rate / 64.0);

  fprintf(stdout, "generation date:               %d/%02d/%d\n",
	 man_hdr->gen_month,
	 man_hdr->gen_day,
	 man_hdr->gen_year);
  
  fprintf(stdout, "generation facility:           ");
  for (i = 0; i < 8; i++)
    fprintf(stdout, "%c", man_hdr->gen_facility[i]);
  fprintf(stdout, "\n");
  
  fprintf(stdout, "missing data value:            %d\n",
	 man_hdr->missing_data_val);
  
  /*
   * Print out the optional header information.
   */

  if (opt_hdr != NULL)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "***************************************\n");
    fprintf(stdout, "OPTIONAL HEADER\n");
    fprintf(stdout, "\n");
  
    fprintf(stdout, "project name:                  ");
    for (i = 0; i < 8; i++)
      fprintf(stdout, "%c", opt_hdr->project_name[i]);
    fprintf(stdout, "\n");
    
    fprintf(stdout, "baseline azimuth:              %f degrees\n",
	   (float)opt_hdr->baseline_azimuth / 64.0);
    fprintf(stdout, "baseline elevation:            %f degrees\n",
	   (float)opt_hdr->baseline_elevation / 64.0);

    fprintf(stdout, "current volume start time:     %d:%02d:%02d\n",
	   opt_hdr->hour,
	   opt_hdr->minute,
	   opt_hdr->second);
    
    fprintf(stdout, "tape name:                     ");
    for (i = 0; i < 8; i++)
      fprintf(stdout, "%c", opt_hdr->tape_name[i]);
    fprintf(stdout, "\n");
    
    fprintf(stdout, "flag:                          %d\n",
	   opt_hdr->flag);
    
  } /* endif - optional header exists */
  
  /*
   * Print out the data header information.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "***************************************\n");
  fprintf(stdout, "DATA HEADER\n");
  fprintf(stdout, "\n");
  
  fprintf(stdout, "number of fields this ray:     %d\n",
	 data_hdr->hdr.num_ray_fields);
  fprintf(stdout, "number of records this ray:    %d\n",
	 data_hdr->hdr.num_ray_records);
  fprintf(stdout, "number of fields this record:  %d\n",
	 data_hdr->hdr.num_record_fields);
  
  for (i = 0; i < data_hdr->hdr.num_record_fields; i++)
  {
    fprintf(stdout, "field %d name:                    %c%c\n",
	   i,
	   data_hdr->field_info_array[i].field_name[0],
	   data_hdr->field_info_array[i].field_name[1]);
    fprintf(stdout, "field %d position:                %d\n",
	   i,
	   data_hdr->field_info_array[i].field_pos);
  }
  
  /*
   * Print out the field header information.
   */

  for (i = 0; i < data_hdr->hdr.num_record_fields; i++)
  {
    field_hdr = (uf_field_header_t *)
      (beam_data + data_hdr->field_info_array[i].field_pos - 1);
    
    fprintf(stdout, "\n");
    fprintf(stdout, "***************************************\n");
    fprintf(stdout, "FIELD %d HEADER\n", i);
    fprintf(stdout, "\n");
  
    fprintf(stdout, "data position:                 %d\n",
	   field_hdr->data_pos);
    fprintf(stdout, "scale factor:                  %d\n",
	   field_hdr->scale_factor);
    fprintf(stdout, "start range:                   %d km\n",
	   field_hdr->start_range);
    fprintf(stdout, "start range offset:            %d m\n",
	   field_hdr->start_center);
    fprintf(stdout, "sample volume spacing:         %d m\n",
	   field_hdr->volume_spacing);
    fprintf(stdout, "number of sample volumes:      %d\n",
	   field_hdr->num_volumes);
    fprintf(stdout, "sample volume depth:           %d m\n",
	   field_hdr->volume_depth);
    fprintf(stdout, "horizontal beam width:         %f degrees\n",
	   (float)field_hdr->horiz_beam_width / 64.0);
    fprintf(stdout, "vertical beam width:           %f degrees\n",
	   (float)field_hdr->vert_beam_width / 64.0);
    fprintf(stdout, "receiver bandwidth:            %d MHz\n",
	   field_hdr->receiver_bandwidth);

    fprintf(stdout, "polarization:                  ");
    if (field_hdr->polarization == 0)
      fprintf(stdout, "horizontal\n");
    else if (field_hdr->polarization == 1)
      fprintf(stdout, "vertical\n");
    else if (field_hdr->polarization == 2)
      fprintf(stdout, "circular\n");
    else if (field_hdr->polarization > 2)
      fprintf(stdout, "elliptical (value = %d)\n",
	     field_hdr->polarization);
    else
      fprintf(stdout, "UNKNOWN (value = %d)\n",
	     field_hdr->polarization);
    
    fprintf(stdout, "wavelength:                    %d cm\n",
	   field_hdr->wavelength);
    fprintf(stdout, "samples used in field est:     %d\n",
	   field_hdr->num_samples);
    fprintf(stdout, "threshold field:               %c%c\n",
	   field_hdr->threshold_field[0],
	   field_hdr->threshold_field[1]);
    fprintf(stdout, "threshold value:               %d\n",
	   field_hdr->threshold_val);
    fprintf(stdout, "scale:                         %d\n",
	   field_hdr->scale);
    fprintf(stdout, "edit code:                     %c%c\n",
	   field_hdr->edit_code[0],
	   field_hdr->edit_code[1]);
    fprintf(stdout, "pulse repetition time:         %d microseconds\n",
	   field_hdr->pulse_rep_time);
    fprintf(stdout, "bits per sample volume:        %d\n",
	   field_hdr->volume_bits);

    data_ptr = beam_data + field_hdr->data_pos - 1;
    
    fprintf(stdout, "First 20 data values:\n");
    for (j = 0; j < 20; j++)
      fprintf(stdout, "   File = %d, Value = %f\n",
	     data_ptr[j], (float)data_ptr[j] / (float)field_hdr->scale_factor);
    fprintf(stdout, "\n");
    
    fprintf(stdout, "Last 20 data values:\n");
    for (j = field_hdr->num_volumes - 20;
	 j < field_hdr->num_volumes; j++)
      fprintf(stdout, "   File = %d, Value = %f\n",
	     data_ptr[j], (float)data_ptr[j] / (float)field_hdr->scale_factor);
    fprintf(stdout, "\n");
  }
  
  fprintf(stdout, "\n\n");
  
  fflush(stdout);
  
}

