#include <stdio.h>
#include "dorade.h"
void dorade_print_sweep_info(Sweep_info *d)
{
  int i;

  printf("Sweep info block\n");
  printf("d->code = <");
  for(i=0; i<sizeof(d->code); i++) printf("%c", d->code[i]);
  printf(">\n");
  printf("len = %d\n", d->len);
  printf("d->radar_name = <");
  for(i=0; i<sizeof(d->radar_name); i++) printf("%c", d->radar_name[i]);
  printf(">\n");
  printf("d->sweep_num = %d\n", d->sweep_num);
  printf("d->nrays = %d\n", d->nrays);
  printf("d->start_angle = %f\n", d->start_angle);
  printf("d->stop_angle = %f\n", d->stop_angle);
  printf("d->fixed_angle = %f\n", d->fixed_angle);
  printf("d->filter_flag = %d\n", d->filter_flag);
}

void dorade_print_ray_info(Ray_info *d)
{
  int i;

  printf("Ray info block\n");
  printf("d->code = <");
  for(i=0; i<sizeof(d->code); i++) printf("%c", d->code[i]);
  printf(">\n");
  printf("len = %d\n", d->len);
  printf("d->sweep_num = %d\n", d->sweep_num);
  printf("d->jday = %d\n", d->jday);
  printf("d->hour = %d\n", d->hour);
  printf("d->minute = %d\n", d->minute);
  printf("d->second = %d\n", d->second);
  printf("d->msec = %d\n", d->msec);
  printf("d->azimuth = %f\n", d->azimuth);
  printf("d->elevation = %f\n", d->elevation);
  printf("d->peak_power = %f\n", d->peak_power);
  printf("d->scan_rate = %f\n", d->scan_rate);
  printf("d->status = %d\n", d->status);
}


void dorade_print_platform_info(Platform_info *d)
{
  int i;

  printf("Platform info block\n");
  printf("d->code = <");
  for(i=0; i<sizeof(d->code); i++) printf("%c", d->code[i]);
  printf(">\n");
  printf("len = %d\n", d->len);
  printf("d->longitude = %f\n", d->longitude);
  printf("d->latitude = %f\n", d->latitude);
  printf("d->altitude = %f\n", d->altitude);
  printf("d->height = %f\n", d->height);
  printf("d->ew_speed = %f\n", d->ew_speed);
  printf("d->ns_speed = %f\n", d->ns_speed);
  printf("d->v_speed = %f\n", d->v_speed);
  printf("d->heading = %f\n", d->heading);
  printf("d->roll = %f\n", d->roll);
  printf("d->pitch = %f\n", d->pitch);
  printf("d->drift = %f\n", d->drift);
  printf("d->rotation = %f\n", d->rotation);
  printf("d->tilt = %f\n", d->tilt);
  printf("d->ew_wind_speed = %f\n", d->ew_wind_speed);
  printf("d->ns_wind_speed = %f\n", d->ns_wind_speed);
  printf("d->v_wind_speed = %f\n", d->v_wind_speed);
  printf("d->heading_rate = %f\n", d->heading_rate);
  printf("d->pitch_rate = %f\n", d->pitch_rate);
}

void dorade_print_correction_factor_desc(Correction_factor_desc *d)
{
  int i;

  printf("Correction factor descriptor\n");
  printf("d->code = <");
  for(i=0; i<sizeof(d->code); i++) printf("%c", d->code[i]);
  printf(">\n");
  printf("len = %d\n", d->len);
  printf("d->azimuth = %f\n", d->azimuth);
  printf("d->elevation = %f\n", d->elevation);
  printf("d->range = %f\n", d->range);
  printf("d->longitude = %f\n", d->longitude);
  printf("d->latitude = %f\n", d->latitude);
  printf("d->altitude = %f\n", d->altitude);
  printf("d->height = %f\n", d->height);
  printf("d->speed_east_west = %f\n", d->speed_east_west);
  printf("d->speed_north_south = %f\n", d->speed_north_south);
  printf("d->vertical_velocity = %f\n", d->vertical_velocity);
  printf("d->heading = %f\n", d->heading);
  printf("d->roll = %f\n", d->roll);
  printf("d->pitch = %f\n", d->pitch);
  printf("d->drift = %f\n", d->drift);
  printf("d->rotation_angle = %f\n", d->rotation_angle);
  printf("d->tilt_angle = %f\n", d->tilt_angle);
}

void dorade_print_cell_range_vector(Cell_range_vector *d)
{
  int i;

  printf("Cell range vector\n");
  printf("d->code = <");
  for(i=0; i<sizeof(d->code); i++) printf("%c", d->code[i]);
  printf(">\n");
  printf("len = %d\n", d->len);
  printf("d->ncells = %d\n", d->ncells);
  /*
  for (i=0; i<d->ncells; i++)
	printf("d->range_cell[%d] = %f\n", i, d->range_cell[i]);
  */
}

void dorade_print_parameter_desc(Parameter_desc *d)
{
  int i;

  printf("Parameter Descriptor\n");
  printf("d->code = <");
  for(i=0; i<sizeof(d->code); i++) printf("%c", d->code[i]);
  printf(">\n");
  printf("len = %d\n", d->len);
  printf("d->name = <");
  for(i=0; i<sizeof(d->name); i++) printf("%c", d->name[i]);
  printf(">\n");
  printf("d->description = <");
  for(i=0; i<sizeof(d->description); i++) printf("%c", d->description[i]);
  printf(">\n");
  printf("d->units = <");
  for(i=0; i<sizeof(d->units); i++) printf("%c", d->units[i]);
  printf(">\n");
  printf("d->ipp = %d\n", d->ipp);
  printf("d->xmit_freq = %d\n", d->xmit_freq);
  printf("d->rcvr_bandwidth = %f\n", d->rcvr_bandwidth);
  printf("d->pulse_width = %d\n", d->pulse_width);
  printf("d->polarization = %d\n", d->polarization);
  printf("d->nsamp_in_dwell_time = %d\n", d->nsamp_in_dwell_time);
  printf("d->parameter_type = %d\n", d->parameter_type);
  printf("d->threshold_field = <");
  for(i=0; i<sizeof(d->threshold_field); i++) printf("%c", d->threshold_field[i]);
  printf(">\n");
  printf("d->threshold_value = %f\n", d->threshold_value);
  printf("d-> scale_factor = %f\n", d-> scale_factor);
  printf("d-> offset_factor = %f\n", d-> offset_factor);
  printf("d-> missing_data_flag = %d\n", d-> missing_data_flag);
}

void dorade_print_radar_desc(Radar_desc *d)
{
  int i;

  printf("Radar Descriptor\n");
  printf("d->code = <");
  for(i=0; i<sizeof(d->code); i++) printf("%c", d->code[i]);
  printf(">\n");
  printf("len = %d\n", d->len);
  printf("d->radar_name = <");
  for(i=0; i<sizeof(d->radar_name); i++) printf("%c", d->radar_name[i]);
  printf(">\n");
  printf("radar_constant = %f\n", d->radar_constant);
  printf("peak_power = %f\n", d-> peak_power);
  printf("noise_power = %f\n", d-> noise_power);
  printf("rcvr_gain = %f\n", d-> rcvr_gain);
  printf("ant_gain = %f\n", d-> ant_gain);
  printf("radar_system_gain = %f\n", d-> radar_system_gain);
  printf("horizontal_beam_width = %f\n", d-> horizontal_beam_width);
  printf("vertical_beam_width = %f\n", d-> vertical_beam_width);
  printf("radar_type = %d\n", d->radar_type);
  printf("scan_mode = %d\n", d-> scan_mode);
  printf("scan_rate = %f\n", d-> scan_rate);
  printf("start_angle = %f\n", d-> start_angle);
  printf("stop_angle = %f\n", d-> stop_angle);
  printf("nparam_desc = %d\n", d->nparam_desc);
  printf("ndesc = %d\n", d-> ndesc);
  printf("compress_code = %d\n", d-> compress_code);
  printf("compress_algo = %d\n", d-> compress_algo);
  printf("data_reduction_param1 = %f\n", d-> data_reduction_param1);
  printf("data_reduction_param2 = %f\n", d-> data_reduction_param2);
  printf("latitude  = %f\n", d-> latitude);
  printf("longitude = %f\n", d-> longitude);
  printf("altitude = %f\n", d-> altitude);
  printf("unambiguous_velocity = %f\n", d-> unambiguous_velocity);
  printf("unambiguous_range = %f\n", d-> unambiguous_range);
  printf("nfreq = %d\n", d-> nfreq);
  printf("npulse_periods = %d\n", d->npulse_periods);
  for (i=0; i<5; i++) printf("freq[%d] = %f\n", i, d->freq[i]);
  for (i=0; i<5; i++) printf("period[%d] = %f\n", i, d->period[i]);
}

void dorade_print_volume_desc(Volume_desc *d)
{
  int i;
  printf("Volume Descriptor\n");
  printf("d->code = <");
  for(i=0; i<sizeof(d->code); i++) printf("%c", d->code[i]);
  printf(">\n");
  printf("version = %d\n", d->version);
  printf("volume_number = %d\n", d->volume_number);
  printf("max_bytes = %d\n", d->max_bytes);
  printf("project_name = <");
  for(i=0; i<sizeof(d->project_name); i++) printf("%c", d->project_name[i]);
  printf(">\n");
  printf("year = %d\n", d->year);
  printf("month = %d\n", d->month);
  printf("day = %d\n", d->day);
  printf("hour = %d\n", d->hour);
  printf("minute = %d\n", d->minute);
  printf("second = %d\n", d->second);
  printf("flight_num = <");
  for(i=0; i<sizeof(d->flight_num); i++) printf("%c", d->flight_num[i]);
  printf(">\n");
  printf("facility_name = <");
  for(i=0; i<sizeof(d->facility_name); i++) printf("%c", d->facility_name[i]);
  printf(">\n");
  printf("gen_year = %d\n", d->gen_year);
  printf("gen_month = %d\n", d->gen_month);
  printf("gen_day = %d\n", d->gen_day);
  printf("nsensors   = %d\n", d->nsensors);
}

void dorade_print_comment_block(Comment_block *cb)
{
  int i;
  printf("COMMENT BLOCK:\n");
  printf("cb->code = <");
  for(i=0; i<sizeof(cb->code); i++) printf("%c", cb->code[i]);
  printf(">\n");
  printf("cb->len = %d\n", cb->len);
  printf("cb->comment = <%s>\n", cb->comment);
}

void dorade_print_sensor(Sensor_desc *s)
{
  int i;
  dorade_print_radar_desc(s->radar_desc);
  
  for (i=0; i<s->nparam; i++) {
	dorade_print_parameter_desc(s->p_desc[i]);
	printf("=====================================================\n");
  }
  dorade_print_cell_range_vector(s->cell_range_vector);
  dorade_print_correction_factor_desc(s->correction_factor_desc);
}
