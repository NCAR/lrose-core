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
// ParamFile.cc
//
//
// Brenda Javornik, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2021
//
///////////////////////////////////////////////////////////////
//
// ParamFile manages a parameter file
//
///////////////////////////////////////////////////////////////

#include "ParamFile.hh"
#include "DisplayField.hh"
#include "FieldListView.hh"
#include "Params.hh"
#include "Reader.hh"
#include "AllocCheck.hh"

#include <string>
#include <cmath>
#include <iostream>

#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>

#include <radar/RadarComplex.hh>
#include <Radx/RadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>

#include <toolsa/toolsa_macros.h>
#include <toolsa/Path.hh>

using namespace std;

ParamFile* ParamFile::m_pInstance = NULL;

ParamFile* ParamFile::Instance()
{
   if (m_pInstance == NULL) {
      m_pInstance = new ParamFile();
   }
   return m_pInstance;
}

// Constructor

ParamFile::ParamFile()
{
  _init();
}

void ParamFile::_init() {
//	m_pInstance = this;

  // initialize

  //  _displayFieldController->setSelectedField(0);
  //_prevFieldNum = -1;

  _radarLat = -9999.0;
  _radarLon = -9999.0;
  _radarAltKm = -9999.0;

  _altitudeInFeet = false;
  // end from DisplayManager    

  //_firstTime = true;

  // setWindowIcon(QIcon("HawkEyePolarIcon.icns"));
  
  _prevAz = -9999.0;
  _prevEl = -9999.0;
  _startAz = -9999.0;
  _endAz = -9999.0;
  _ppiRays = NULL;
  _rhiMode = false;

  _nGates = 1000;
  _maxRangeKm = 1.0;

  label_font_size = 12;
  
  //_archiveStartTimeEdit = NULL;
  //_archiveEndTimeEdit = NULL;

}

// destructor

ParamFile::~ParamFile()
{
}

bool ParamFile::loadFromArgs(int argc, char **argv,
                           char **override_list,
                           char **params_path_p,
                           bool defer_exit) {


  _init();

  bool OK = true;
  if (_params.loadFromArgs(argc, argv,
         override_list,
         params_path_p)) {
    cerr << "ERROR: HawkEdit" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = false;
  } else {
    debug = _params.debug; //  >= Params::DEBUG_NORM;

    _archiveStartTime.set(_params.archive_start_time);
    _archiveEndTime = _archiveStartTime + _params.archive_time_span_secs;
    _archiveScanIndex = 0;

    _imagesArchiveStartTime.set(_params.images_archive_start_time);
    _imagesArchiveEndTime.set(_params.images_archive_end_time);
    _imagesScanIntervalSecs = _params.images_scan_interval_secs;

    fields_n = _params.fields_n;
    gridColor = _params.grid_and_range_ring_color;
    emphasisColor = "white";
    annotationColor = "white";
    backgroundColor = _params.background_color;
    color_scale_dir = _params.color_scale_dir;
    fields = _params._fields;
    display_mode = _params.display_mode;
    begin_in_archive_mode = _params.begin_in_archive_mode;
    archive_data_url = _params.archive_data_url;
    archive_start_time = _params.archive_start_time;
    archive_time_span_secs = _params.archive_time_span_secs;
    images_archive_end_time = _params.images_archive_end_time;
    images_archive_start_time = _params.images_archive_start_time;
    images_scan_interval_secs = _params.images_scan_interval_secs;
    ppi_range_rings_on_at_startup = _params.ppi_range_rings_on_at_startup;
    ppi_grids_on_at_startup = _params.ppi_grids_on_at_startup;
    ppi_azimuth_lines_on_at_startup = _params.ppi_azimuth_lines_on_at_startup;
    ppi_override_rendering_beam_width = _params.ppi_override_rendering_beam_width;
    ppi_rendering_beam_width = _params.ppi_rendering_beam_width;
    max_range_km = _params.max_range_km;
    images_auto_create = _params.images_auto_create;
    images_creation_mode = _params.images_creation_mode;
      set_max_range = _params.set_max_range;
  images_schedule_interval_secs = _params.images_schedule_interval_secs;
  images_set_sweep_index_list = _params.images_set_sweep_index_list;
  images_sweep_index_list_n = _params.images_sweep_index_list_n;
  images_output_dir = _params.images_output_dir;
  images_write_to_day_dir = _params.images_write_to_day_dir;
  images_file_name_category = _params.images_file_name_category;
  images_file_name_platform = _params.images_file_name_platform;
  images_file_name_delimiter = _params.images_file_name_delimiter;
  images_include_time_part_in_file_name = _params.images_include_time_part_in_file_name;
  images_include_seconds_in_time_part = _params.images_include_seconds_in_time_part;
  images_include_field_label_in_file_name = _params.images_include_field_label_in_file_name;
  show_status_in_gui = _params.show_status_in_gui;
  display_site_name = _params.display_site_name;
  radar_name = _params.radar_name;
  site_name = _params.site_name;
  override_radar_name = _params.override_radar_name;
  override_site_name = _params.override_site_name;
  ppi_aspect_ratio = _params.ppi_aspect_ratio;
      color_scale_width = _params.color_scale_width;
    ppi_display_type = _params.ppi_display_type;


    range_ring_label_font_size = _params.range_ring_label_font_size;
    click_cross_size = _params.click_cross_size;
    ppi_main_legend_pos = _params.ppi_main_legend_pos;
    // set params on alloc checker

    //AllocCheck::inst().setParams(_params);
  }
  return OK;

}

/* TODO:
bool _openFile() {
  emit resetDisplayFields(...) ==> DisplayFieldController::setupDisplayFields(
  string colorMapDir, 
  vector<Params::field_t> &fields,
  string gridColor, 
  string emphasisColor,
  string annotationColor, 
  string backgroundColor,
  Params::debug_t debug  
  )
}
*/


void ParamFile::setArchiveDataUrl(const char *url) {
  strcpy(archive_data_url, url);
}


 





