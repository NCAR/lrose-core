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
// ** OR IMPLIED WARRANTIES, INLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
////////////////////////////////////////////////////////////
// LegacyParams.hh
//
// Read legacy params, write out tdrp-compatible param file
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// Dec 2023
//
/////////////////////////////////////////////////////////////

#include <cstring>
#include <string>
#include <vector>
#include "Cdraw_P.hh"
#include "Cgui_P.hh"
#include "cidd_params.h"
using namespace std;

class LegacyParams {

public:
  
  // constructor

  LegacyParams();

  // destructor

  ~LegacyParams();
  
  // Clear the data base
  
  void clear();

  // set to print TDRP version of parameter

  void setPrintTdrp(bool val) { _printTdrp = val; }
    
  //  translate legacy to TDRP
  
  int translateToTdrp(const string &legacyParamsPath,
                      const string &tdrpParamsPath);

private:

  static const int MAX_PARSE_FIELDS = 32;
  static const int MAX_PARSE_SIZE = 1024;
  static const int INPUT_LINE_LEN = 10000;
  static const int TAG_BUF_LEN = 1024;
  // static const int URL_MIN_SIZE = 8;

  typedef struct {
    string name;
    string entry;
  } param_list_t;
  
  vector<param_list_t> _plist;

  // params buffer
  
  char *_paramsBuf;
  int _paramsBufLen;

  // tdrp

  bool _printTdrp;
  FILE *_tdrpFile;

  // fields
  
  typedef enum {
    POLYGONS,
    FILLED_CONTOURS,
    DYNAMIC_CONTOURS,
    LINE_CONTOURS
  } GridRenderMode;

  class Field {
  public:
    Field() {
      group_name = "main";
      is_valid = false;
      contour_low = 0;
      contour_high = 100;
      contour_interval = 5;
      render_mode = POLYGONS;
      display_in_menu = true;
      background_render = false;
      composite_mode = false;
      auto_scale = false;
      auto_render = false;
    }
    ~Field() {
    }
    Field& operator=(const Field &rhs) {
      if (&rhs == this) {
        return *this;
      }
      is_valid = rhs.is_valid;
      text_line = rhs.text_line;
      group_name = rhs.group_name;
      button_label = rhs.button_label;
      legend_label = rhs.legend_label;
      url = rhs.url;
      field_name = rhs.field_name;
      color_map = rhs.color_map;
      field_units = rhs.field_units;
      contour_low = rhs.contour_low;
      contour_high = rhs.contour_high;
      contour_interval = rhs.contour_interval;
      render_mode = rhs.render_mode;
      display_in_menu = rhs.display_in_menu;
      background_render = rhs.background_render;
      composite_mode = rhs.composite_mode;
      auto_scale = rhs.auto_scale;
      auto_render = rhs.auto_render;
      return *this;
    }
    bool is_valid;
    string text_line;
    string group_name;
    string button_label;
    string legend_label;
    string url;
    string field_name;
    string color_map;
    string field_units;
    double contour_low;
    double contour_high;
    double contour_interval;
    GridRenderMode render_mode;
    bool display_in_menu;
    bool background_render;
    bool composite_mode;
    bool auto_scale;
    bool auto_render;
  };
  
  // winds
  
  typedef enum {
    ARROW,
    VECTOR,
    BARB,
    LABELEDBARB,
    TUFT,
    TICKVECTOR,
    METBARB,
    BARB_SH,
    LABELEDBARB_SH
  } WindRenderMode;

  class Wind {
  public:
    Wind() {
      is_valid = false;
      line_width = 1;
      render_mode = ARROW;
      on_at_startup = true;
      color = "white";
    }
    ~Wind() {
    }
    bool is_valid;
    string text_line;
    string button_label;
    string legend_label;
    string url;
    string u_field_name;
    string v_field_name;
    string w_field_name;
    string units;
    int line_width;
    WindRenderMode render_mode;
    string color;
    bool on_at_startup;
  };

  // maps
  // detail thresholds are in km across image
  
  class MapOverlay {
  public:
    MapOverlay() {
      is_valid = false;
      line_width = 1;
      color = "white";
      detail_thresh_min = 0.0;
      detail_thresh_max = 10000.0;
      on_at_startup = true;
    }
    ~MapOverlay() {
    }
    bool is_valid;
    string text_line;
    string map_code;
    string control_label;
    string map_file_name;
    int line_width;
    double detail_thresh_min;
    double detail_thresh_max;
    string color;
    bool on_at_startup;
  };

  // params read in from main

  bool _debug;
  bool _debug1;
  bool _debug2;
  string _windMarkerType;
  WindRenderMode _defaultWindRenderMode;
  Cgui_P _guiConfig;
  bool _replaceUnderscores;
  string _httpProxyUrl;
  bool _runOnceAndExit;
  double _originLatitude;
  double _originLongitude;
  
  // read in from param file
  // returns 0 on success, -1 on failure
  
  int _readFromPath(const char *file_path,
                    const char *prog_name);
  
  // read in from param file
  // returns 0 on success, -1 on failure
  
  int _readFromBuf(const char *buf, int buf_len,
                   const char *prog_name);

  // get param values of various types
  
  const char *_get(const char *search_name) const;
  double _getDouble(const char *param_name, double default_val);
  float _getFloat(const char *param_name, float default_val);
  int _getInt(const char *param_name, int default_val);
  bool _getBoolean(const char *param_name, int default_val);
  long _getLong(const char *param_name, long default_val);
  const string _getString(const char *param_name, const char *default_val);
  
  const char *_findTagText(const char *input_buf,
                           const char * tag,
                           long *text_len,
                           long *text_line_no);

  const char *_removeCiddStr(const char *name) const;

  int _loadKeyValPairsDefault(char* &db_buf, int &db_len);

  int _loadKeyValPairsFile(const string &fname,
                           char* &db_buf,
                           int &db_len);
  
  int _loadKeyValPairsHttp(const string &fname,
                           char* &db_buf,
                           int &db_len);
  
  int _loadKeyValPairs(const string &fname);
  
  int _readMainParams();
  int _readGuiConfig();
  int _readGrids();
  int _readWinds();
  WindRenderMode _getWindRenderMode(const char* markerStr);
  int _readMaps();

  int _initDrawExportLinks();
  
  /////////////////////////////////////////////////////////////////////////////
  // GET_DEFAULT_PARAMS : Get a string representing the default parameters
  //                      for this TDRP parameter object
  // 

  template <class T>
    std::string _getDefaultTdrpParams(const std::string &section_name, T *params)
  {
    // Load the temporary parameters since we don't seem to have them
    // loaded yet

    params->loadDefaults(false);
  
    // Open a temporary file to hold the parameters.  Write the parameters
    // to that file and close it.

    const std::string tmp_filename = ".params";
  
    FILE *tmp_file;

    if ((tmp_file = fopen(tmp_filename.c_str(), "w")) == 0)
    {
      fprintf(stderr,
              "Error opening temporary file for writing TDRP parameters\n");
      exit(-1);
    }
  
    params->print(tmp_file);
    fclose(tmp_file);
  
    // Now read the file back into a string

    struct stat tmp_file_stat;
  
    if (stat(tmp_filename.c_str(), &tmp_file_stat) != 0)
    {
      fprintf(stderr,
              "Error stating temporary file for writing TDRP parameters\n");
      exit(-1);
    }
  
    char *tmp_buffer = new char[tmp_file_stat.st_size + 1];

    if ((tmp_file = fopen(tmp_filename.c_str(), "r")) == 0)
    {
      fprintf(stderr,
              "Error opening temporary file for reading TDRP parameters\n");
      exit(-1);
    }
  
    if (fread(tmp_buffer, sizeof(char), tmp_file_stat.st_size, tmp_file) !=
        (size_t)tmp_file_stat.st_size)
    {
      delete [] tmp_buffer;
      fprintf(stderr,
              "Error reading TDRP parameters from temporary file\n");
      exit(-1);
    }
  
    fclose(tmp_file);
    unlink(tmp_filename.c_str());
  
    // Set the return string

    tmp_buffer[tmp_file_stat.st_size] = '\0';

    std::string return_string = "<" + section_name + ">\n";
    return_string += tmp_buffer;
    return_string += "</" + section_name + ">\n\n";
  
    delete [] tmp_buffer;
  
    return return_string;
  }

};
