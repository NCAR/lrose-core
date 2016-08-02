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
///////////////////////////////////////////////////////////////
// msg_print.cc
//
// Print methods for DsMdvxMsg object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1999
//
///////////////////////////////////////////////////////////////

#include <Mdv/DsMdvxMsg.hh>
#include <toolsa/udatetime.h>
using namespace std;

///////////////////////
// print file_search_t

void DsMdvxMsg::_print_file_search(file_search_t &fsearch, ostream &out)

{
  out << "--------------------------------------------" << endl;
  out << "File search struct" << endl;
  switch (fsearch.file_search_mode) {
    
  case MDVP_READ_FROM_PATH:
    out << "  file_search_mode: MDVP_READ_FROM_PATH" << endl;
    break;
  case MDVP_READ_LAST:
    out << "  file_search_mode: MDVP_READ_LAST" << endl;
    break;
  case MDVP_READ_CLOSEST:
    out << "  file_search_mode: MDVP_READ_CLOSEST" << endl;
    break;
  case MDVP_READ_FIRST_BEFORE:
    out << "  file_search_mode: MDVP_READ_FIRST_BEFORE" << endl;
    break;
  case MDVP_READ_FIRST_AFTER:
    out << "  file_search_mode: MDVP_READ_FIRST_AFTER" << endl;
    break;
  case MDVP_READ_BEST_FORECAST:
    out << "  file_search_mode: MDVP_READ_BEST_FORECAST" << endl;
    break;
  case MDVP_READ_SPECIFIED_FORECAST:
    out << "  file_search_mode: MDVP_READ_SPECIFIED_FORECAST" << endl;
    break;
  }

  out << "  search_margin_secs: " << fsearch.search_margin_secs << endl;
  out << "  search_time: " << utimstr(fsearch.search_time) << endl;
  out << "  forecast_lead_secs: " << fsearch.forecast_lead_secs << endl;
  out << "  valid_time_seach_wt: " << fsearch.valid_time_search_wt << endl;
}

////////////////////////////
// print read_horiz_limits_t

void DsMdvxMsg::_print_read_horiz_limits(read_horiz_limits_t &limits, ostream &out)

{
  out << "----------read_horiz_limits struct ------------" << endl;
  out << "  min_lat: " << limits.min_lat << endl;
  out << "  min_lon: " << limits.min_lon << endl;
  out << "  max_lat: " << limits.max_lat << endl;
  out << "  max_lon: " << limits.max_lon << endl;
}

////////////////////////////
// print read_vlevel_limits_t

void DsMdvxMsg::_print_read_vlevel_limits(read_vlevel_limits_t &limits, ostream &out)

{
  out << "----------read_vlevel_limits struct ------------" << endl;
  out << "  min_vlevel: " << limits.min_vlevel << endl;
  out << "  max_vlevel: " << limits.max_vlevel << endl;
}

////////////////////////////
// print read_plane_num_limits_t

void DsMdvxMsg::_print_read_plane_num_limits(read_plane_num_limits_t &limits, ostream &out)

{
  out << "----------read_plane_num_limits struct ------------" << endl;
  out << "  min_plane_num: " << limits.min_plane_num << endl;
  out << "  max_plane_num: " << limits.max_plane_num << endl;
}

////////////////////////////
// print read_composite_t

void DsMdvxMsg::_print_read_composite(read_composite_t &comp, ostream &out)

{
  out << "----------read_composite struct ------------" << endl;
  out << "  type: " << comp.type << endl;
}

////////////////////////////
// print read_encoding_t

void DsMdvxMsg::_print_read_encoding(read_encoding_t &encod, ostream &out)

{
  out << "----------read_encoding struct ------------" << endl;
  out << "  encoding_type: " << encod.encoding_type << endl;
  out << "  compression_type: " << encod.compression_type << endl;
  out << "  scaling_type: " << encod.scaling_type << endl;
  out << "  scale: " << encod.scale << endl;
  out << "  bias: " << encod.bias << endl;
}

////////////////////////////
// print read_remap_t

void DsMdvxMsg::_print_read_remap(read_remap_t &remap, ostream &out)

{
  out << "----------read_remap struct ------------" << endl;
  out << "  proj_type: " << Mdvx::projType2Str(remap.proj_type) << endl;
  out << "  nx: " << remap.nx << endl;
  out << "  ny: " << remap.ny << endl;
  out << "  minx: " << remap.minx << endl;
  out << "  miny: " << remap.miny << endl;
  out << "  dx: " << remap.dx << endl;
  out << "  dy: " << remap.dy << endl;
  out << "  origin_lat: " << remap.origin_lat << endl;
  out << "  origin_lon: " << remap.origin_lon << endl;
  for (int i = 0; i < 8; i++) {
    out << "  proj_params[" << i << "] : " << remap.proj_params[i] << endl;
  }
}

////////////////////////////
// print write_options_t

void DsMdvxMsg::_print_write_options(write_options_t &options, ostream &out)

{
  out << "----------write_options struct ------------" << endl;
  out << "  write_as_forecast: "
      << (options.write_as_forecast?"T":"F") << endl;
  out << "  write_ldata_info: "
      << (options.write_ldata_info?"T":"F") << endl;
  out << "  write_using_extended_path: "
      << (options.write_using_extended_path?"T":"F") << endl;
}

////////////////////////////
// print time_list_options_t

void DsMdvxMsg::_print_time_list_options(time_list_options_t &options, ostream &out)

{
  out << "----------time_list_options struct ------------" << endl;
  out << "  mode: " << options.mode << endl;
  out << "  start_time: " << utimstr(options.start_time) << endl;
  out << "  end_time: " << utimstr(options.end_time) << endl;
  out << "  gen_time: " << utimstr(options.gen_time) << endl;
}

////////////////////////////
// print time list

void DsMdvxMsg::_print_time_list(time_list_hdr_t &hdr,
                                 ti32 *times,
                                 ostream &out)
  
{
  out << "---------- time list ------------" << endl;
  out << "  nTimes: " << hdr.ntimes << endl;
  out << "  has_forecasts: " << (bool) hdr.has_forecasts << endl;
  for (int i = 0; i < hdr.ntimes; i++) {
    out << "  time [" << i << "]: " << utimstr(times[i]) << endl;
  }
}

////////////////////////////
// print climo statistic type

void DsMdvxMsg::_print_climo_stat_type(const climoTypePartHdr_t &hdr,
				       const climoTypePart_t *stats,
				       ostream &out) const
{
  out << "---------- climo statistic types ------------" << endl;
  out << "  num stats: " << hdr.num_stats << endl;
  for (int i = 0; i < hdr.num_stats; ++i)
  {
    out << "  climo stat type: "
	<< Mdvx::climoType2Str(stats[i].climo_type) << endl;
    if (stats[i].divide_by_num_obs)
      out << "    divide by num obs: true" << endl;
    else
      out << "    divide by num obs: false" << endl;
    out << "    param[0]: " << stats[i].params[0] << endl;
    out << "    param[1]: " << stats[i].params[1] << endl;
  }
  
}

////////////////////////////
// print climo date range

void DsMdvxMsg::_print_climo_data_range(const climoDataRange_t data_range,
					ostream &out) const
{
  out << "---------- climo date range ------------" << endl;
  out << "  start time: " << DateTime::str(data_range.start_time) << endl;
  out << "  end time: " << DateTime::str(data_range.end_time) << endl;
}

////////////////////////////
// print climo time range

void DsMdvxMsg::_print_climo_time_range(const climoTimeRange_t time_range,
					ostream &out) const
{
  out << "---------- climo time range ------------" << endl;
  out << "  start hour: " << time_range.start_hour << endl;
  out << "  start minute: " << time_range.start_minute << endl;
  out << "  start second: " << time_range.start_second << endl;
  out << "  end hour: " << time_range.end_hour << endl;
  out << "  end minute: " << time_range.end_minute << endl;
  out << "  end second: " << time_range.end_second << endl;
}

