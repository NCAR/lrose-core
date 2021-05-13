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
////////////////////////////////////////////////////////////////
// Titan2Xml.cc
//
// Convert titan objects to XML
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2021
//
////////////////////////////////////////////////////////////////
//
// Static methods that return strings
//
////////////////////////////////////////////////////////////////

#include <titan/Titan2Xml.hh>
#include <toolsa/TaXml.hh>
using namespace std;

////////////////////////////////////////////////////////////
// complex params

string Titan2Xml::complexParams(const complex_track_params_t &params,
                                int level)

{

  string xml;
  
  xml += TaXml::writeStartTag("complex_track_params", level);

  xml += TaXml::writeDouble("volume_at_start_of_sampling",
                            params.volume_at_start_of_sampling, level + 1);
  xml += TaXml::writeDouble("volume_at_end_of_sampling",
                            params.volume_at_end_of_sampling, level + 1);

  xml += TaXml::writeInt("complex_track_num", params.complex_track_num, level + 1);

  xml += TaXml::writeInt("start_scan", params.start_scan, level + 1);
  xml += TaXml::writeInt("end_scan", params.end_scan, level + 1);

  xml += TaXml::writeInt("duration_in_scans", params.duration_in_scans, level + 1);
  xml += TaXml::writeInt("duration_in_secs", params.duration_in_secs, level + 1);

  xml += TaXml::writeTime("start_time", params.start_time, level + 1);
  xml += TaXml::writeTime("end_time", params.end_time, level + 1);

  xml += TaXml::writeInt("n_simple_tracks", params.n_simple_tracks, level + 1);

  xml += TaXml::writeInt("n_top_missing", params.n_top_missing, level + 1);
  xml += TaXml::writeInt("n_range_limited", params.n_range_limited, level + 1);
  xml += TaXml::writeInt("start_missing", params.start_missing, level + 1);
  xml += TaXml::writeInt("end_missing", params.end_missing, level + 1);
  xml += TaXml::writeInt("n_samples_for_forecast_stats", params.n_samples_for_forecast_stats, level + 1);

  xml += Titan2Xml::contingencyData("ellipse_verify", params.ellipse_verify, level + 1);
  xml += Titan2Xml::contingencyData("polygon_verify", params.polygon_verify, level + 1);
  xml += Titan2Xml::forecastProps("forecast_bias", params.forecast_bias, level + 1);
  xml += Titan2Xml::forecastProps("forecast_rmse", params.forecast_rmse, level + 1);

  xml += TaXml::writeEndTag("complex_track_params", level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// track entry

string Titan2Xml::trackEntry(const track_file_entry_t &entry,
                             int level,
                             int entry_num /* = -1 */)

{

  string xml;
  
  if (entry_num >= 0) {
    vector<TaXml::attribute> attrs;
    attrs.push_back(TaXml::attribute("entry_number", entry_num));
    xml += TaXml::writeStartTag("track_entry", level, attrs, true);
  } else {
    xml += TaXml::writeStartTag("track_entry", level);
  }

  xml += TaXml::writeTime("time", entry.time, level + 1);
  xml += TaXml::writeTime("time_origin", entry.time_origin, level + 1);
  xml += TaXml::writeInt("scan_origin", entry.scan_origin, level + 1);
  xml += TaXml::writeInt("scan_num", entry.scan_num, level + 1);
  xml += TaXml::writeInt("storm_num", entry.storm_num, level + 1);
  xml += TaXml::writeInt("simple_track_num", entry.simple_track_num, level + 1);
  xml += TaXml::writeInt("complex_track_num", entry.complex_track_num, level + 1);
  xml += TaXml::writeInt("history_in_scans", entry.history_in_scans, level + 1);
  xml += TaXml::writeInt("history_in_secs", entry.history_in_secs, level + 1);
  xml += TaXml::writeInt("duration_in_scans", entry.duration_in_scans, level + 1);
  xml += TaXml::writeInt("duration_in_secs", entry.duration_in_secs, level + 1);
  xml += TaXml::writeBoolean("forecast_valid", entry.forecast_valid, level + 1);

  xml += Titan2Xml::forecastProps("dval_dt", entry.dval_dt, level + 1);

  xml += TaXml::writeEndTag("track_entry", level);
  
  return xml;

}

////////////////////////////////////////////////////////////
// forecast props

string Titan2Xml::forecastProps(const string &tag,
                                const track_file_forecast_props_t &props,
                                int level)
  
{

  string xml;

  xml += TaXml::writeStartTag(tag, level);
  
  xml += TaXml::writeDouble("proj_area_centroid_x",
                            props.proj_area_centroid_x, level + 1);
  xml += TaXml::writeDouble("proj_area_centroid_y",
                            props.proj_area_centroid_y, level + 1);
  xml += TaXml::writeDouble("vol_centroid_z",
                            props.vol_centroid_z, level + 1);
  xml += TaXml::writeDouble("refl_centroid_z",
                            props.refl_centroid_z, level + 1);
  xml += TaXml::writeDouble("top",
                            props.top, level + 1);
  xml += TaXml::writeDouble("dbz_max",
                            props.dbz_max, level + 1);
  xml += TaXml::writeDouble("volume",
                            props.volume, level + 1);
  xml += TaXml::writeDouble("precip_flux",
                            props.precip_flux, level + 1);
  xml += TaXml::writeDouble("mass",
                            props.mass, level + 1);
  xml += TaXml::writeDouble("proj_area",
                            props.proj_area, level + 1);
  xml += TaXml::writeDouble("smoothed_proj_area_centroid_x",
                            props.smoothed_proj_area_centroid_x, level + 1);
  xml += TaXml::writeDouble("smoothed_proj_area_centroid_y",
                            props.smoothed_proj_area_centroid_y, level + 1);
  xml += TaXml::writeDouble("smoothed_speed",
                            props.smoothed_speed, level + 1);
  xml += TaXml::writeDouble("smoothed_direction",
                            props.smoothed_direction, level + 1);
  
  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}


////////////////////////////////////////////////////////////
// contingency data

string Titan2Xml::contingencyData(const string &tag,
                                  const track_file_contingency_data_t &cont,
                                  int level)
  
{

  string xml;

  xml += TaXml::writeStartTag(tag, level);
  xml += TaXml::writeDouble("n_success", cont.n_success, level + 1);
  xml += TaXml::writeDouble("n_failure", cont.n_failure, level + 1);
  xml += TaXml::writeDouble("n_false_alarm", cont.n_false_alarm, level + 1);

  xml += TaXml::writeEndTag(tag, level);
  
  return xml;

}

