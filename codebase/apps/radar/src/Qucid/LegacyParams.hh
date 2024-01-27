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

  typedef struct {
    string name;
    string entry;
  } param_list_t;
  
  vector<param_list_t> _plist;

  bool _printTdrp;

  int _paramsBufLen;
  char *_paramsBuf; // Pointer to the parameter data
  Cgui_P _guiConfig;

  FILE *_tdrpFile;

  // field details
  
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
  
  // wind details
  
  typedef enum {
    ARROW,
    VECTOR,
    BARB,
    LABELEDBARB,
    TUFT,
    TICKVECTOR,
    METBARB,
    BARB_S,
    LABELED_BARB
  } WindRenderMode;

  string _windMarkerType;
  WindRenderMode _defaultWindRenderMode;

  class Wind {
  public:
    Wind() {
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
    ~Wind() {
    }
    bool is_valid;
    string text_line;
    string label;
    string url;
    string u_field_name;
    string v_field_name;
    string w_field_name;
    string wind_units;
    int line_width;
    WindRenderMode render_mode;
    string color;
  };
  
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
  
  int _readGuiConfig();
  
  int _readGrids();
  
  int _initWindFields(const char *param_buf,
                      long param_buf_len,
                      long line_no);
  
  int _initDrawExportLinks();
  
  int _loadOverlayInfo(const char *param_buf, long param_buf_len,
                       long line_no,
                       int  max_overlays);
  
  int _loadOverlayData(int  num_overlays);
  
  int _initOverlays(const char *param_buf,
                    long param_buf_len,
                    long line_no);
};
  
  
