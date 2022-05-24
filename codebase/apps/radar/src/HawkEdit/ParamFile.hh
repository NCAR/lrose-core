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
/////////////////////////////////////////////////////////////
// ParamFile.hh
//
// ParamFile header
//
// Brenda Javornik, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Mar 2021
//
///////////////////////////////////////////////////////////////
//
// ParamFile manages parameters specified in a file
//
///////////////////////////////////////////////////////////////

#ifndef ParamFile_HH
#define ParamFile_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include "RayLoc.hh"
//#include "ContextEditingView.hh"
//#include "ClickableLabel.hh"
//#include "ParameterColorView.hh"
//#include "FieldColorController.hh"
#include <euclid/SunPosn.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxPlatform.hh>
#include <Radx/RadxVol.hh>
#include <tdrp/tdrp.h>

class DisplayField;
class PpiWidget;
class RhiWidget;
class RhiWindow;
class Reader;
class RadxPlatform;
class TimeScaleWidget;

class ParamFile {

public:
  static ParamFile* Instance();

  
  // destructor
  
  ~ParamFile();

  bool loadFromArgs(int argc, char **argv,
                           char **override_list,
                           char **params_path_p,
                           bool defer_exit = true);

  vector<string> *getFieldsArchiveData(string fileName);
  //vector<string> *userSelectFieldsForReading(string fileName);
  //void getFileAndFields();

// from DisplayManager ...

  // get selected name and units

  //const string &getSelectedFieldLabel() const { return _selectedLabel; }
  //const string &getSelectedFieldName() const { return _selectedName; }
  //const string &getSelectedFieldUnits() const { return _selectedUnits; }
  // const DisplayField &getSelectedField() const { return _displayFieldController->getField(_fieldNum); }
  // const vector<DisplayField *> &getDisplayFields() const { return _fields; }
  //  const DisplayField &getSelectedField() const { return *_fields[_fieldNum]; }
  //  const vector<DisplayField *> &getDisplayFields() const { return _fields; }

  // location

  double getRadarLat() const { return _radarLat; }
  double getRadarLon() const { return _radarLon; }
  double getRadarAltKm() const { return _radarAltKm; }
  const RadxPlatform &getPlatform() const { return _platform; }

  Params::debug_t debug; 
  int fields_n;
  string gridColor;
  string emphasisColor;
  string annotationColor;
  string backgroundColor;
  string color_scale_dir;
  //vector<Params::field_t> *fields; // TODO: can I assign the contents of a vector?
  Params::field_t *fields;
  Params::display_mode_t display_mode;
  tdrp_bool_t begin_in_archive_mode;
  char* archive_data_url;

  char* archive_start_time;
  double archive_time_span_secs;
  char* images_archive_end_time;
  char* images_archive_start_time;
  bool images_auto_create;
  Params::images_creation_mode_t images_creation_mode;
  int images_scan_interval_secs;
  char* radar_name;
  int label_font_size;
  int main_window_width;
  int main_window_height;
  int main_window_start_x;
  int main_window_start_y;
  bool ppi_range_rings_on_at_startup;
  bool ppi_grids_on_at_startup;
  bool ppi_azimuth_lines_on_at_startup;
  bool ppi_override_rendering_beam_width;
  double ppi_rendering_beam_width;
  double max_range_km;
  bool set_max_range;
  int images_schedule_interval_secs;
  bool images_set_sweep_index_list;
  bool images_sweep_index_list_n;
  char* images_output_dir;
  bool images_write_to_day_dir;
  char* images_file_name_category;
  char* images_file_name_platform;
  char* images_file_name_delimiter;
  bool images_include_time_part_in_file_name;
  bool images_include_seconds_in_time_part;
  bool images_include_field_label_in_file_name;

  Params::show_status_t show_status_in_gui;
  bool display_site_name;
  char *site_name;
  bool override_radar_name;
  bool override_site_name;
  double ppi_aspect_ratio;
  int color_scale_width;

  Params::ppi_display_type_t ppi_display_type;

  int range_ring_label_font_size;
  int click_cross_size;
  Params::legend_pos_t ppi_main_legend_pos;





  void setArchiveDataUrl(const char *);

private:

  static ParamFile* m_pInstance;

  // constructor
  ParamFile(); // const Params &params);

  // from DisplayManager ...
  Params _params;
  
  // reading data in
  
  Reader *_reader;

  
  // instrument platform details 

  RadxPlatform _platform;
  


  bool _haveFilteredFields;
  int _rowOffset;



  bool _altitudeInFeet;
  
  // sun position calculator
  double _radarLat, _radarLon, _radarAltKm;
  SunPosn _sunPosn;

  // end from DisplayManager


  bool _firstTime;
  bool _urlOK;

  // beam geometry
  
  int _nGates;
  double _maxRangeKm;

  RayLoc* _ppiRayLoc; // for use, allows negative indices at north line
  RayLoc* _ppiRays;   // for new and delete

  // input data
  
  RadxTime _readerRayTime;
  //RadxVol _vol;

  bool _rhiWindowDisplayed;
  bool _rhiMode;
  
  // azimuths for current ray

  double _prevAz;
  double _prevEl;
  double _startAz;
  double _endAz;

  // times for rays

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  RadxTime _prevRayTime;
  


  // archive mode
  
  bool _archiveMode;
  bool _archiveRetrievalPending;


  RadxTime _guiStartTime;
  RadxTime _archiveStartTime;
  
  RadxTime _guiEndTime;
  RadxTime _archiveEndTime;

  RadxTime _selectedTime;


  int _nArchiveScans;
  vector<string> _archiveFileList;
  int _archiveScanIndex;
  bool _archiveFilesHaveDayDir;

  RadxTime _archiveIntermediateTime;

  RadxTime _startDisplayTime;
  RadxTime _currentDisplayTime;  // is this needed??
  RadxTime _endDisplayTime;
  RadxTime _imagesArchiveStartTime;
  RadxTime _imagesArchiveEndTime;
  int _imagesScanIntervalSecs;

  // saving images in real time mode

  RadxTime _imagesScheduledTime;

  //////////////////////////////
  // private methods

  void _init();

  // open File 

  void _openFile();
  void _saveFile();
  void _moveUpDown();
  string _getOutputPath(bool interactive, string &outputDir, string fileExt);

  // set top bar

  void _setTitleBar(const string &radarName);
  
  // local methods

  void _clear();


  // data retrieval

  //void _handleRealtimeData(QTimerEvent * event);
  //void _handleArchiveData(QTimerEvent * event);
  void _readDataFile(vector<string> *selectedFields);

  void _handleColorMapChangeOnRay(RadxPlatform &platform, // RadxRay *ray, 
				  string fieldName);
  void _updateColorMap(string fieldName);

  //  int _applyDataEdits(RadxVol _editedVol);  // & or * ??
  void _applyDataEdits(); // const RadxVol &editedVol);
  void _addNewFields(vector<DisplayField *> newFields);

  //virtual void _changeField(int fieldId, bool guiMode) = 0;
  //virtual void _openFile();
  //virtual void _saveFile();

  void _changeFieldVariable(bool value);
  int _updateDisplayFields(vector<string> *fieldNames);

  
  void _saveToFile(bool interactive = true);


  // open file 

  void _createFileChooserDialog();
  void _refreshFileChooserDialog();
  void _showFileChooserDialog();

};

#endif

