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
////////////////////////////////////////////////////////////
// LegacyParams.hh
//
// Read legacy params, write out tdrp-compatible param file
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// Dec 2023
//
/////////////////////////////////////////////////////////////

#include <string>
#include <vector>
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
    
  // read in from param file
  // returns 0 on success, -1 on failure
  
  int readFromPath(const char *file_path,
                   const char *prog_name);
  
  // read in from param file
  // returns 0 on success, -1 on failure
  
  int readFromBuf(const char *buf, int buf_len,
                  const char *prog_name);

  //  translate legacy to TDRP
  
  int translateToTdrp(const string &legacyParamsPath,
                      const string &tdrpParamsPath);

  // get param values of various types
  
  double getDouble(const char *param_name, double default_val);
  float getFloat(const char *param_name, float default_val);
  int getInt(const char *param_name, int default_val);
  bool getBoolean(const char *param_name, int default_val);
  long getLong(const char *param_name, long default_val);
  const char *getString(const char *param_name, const char *default_val);

private:

  typedef struct {
    string name;
    string entry;
  } param_list_t;
  
  vector<param_list_t> _plist;

  const char *_get(const char *search_name) const;

  bool _printTdrp;

  int _paramsBufLen;
  char *_paramsBuf; // Pointer to the parameter data

  const char *_findTagText(const char *input_buf,
                           const char * tag,
                           long *text_len,
                           long *text_line_no);

  int _loadDbDataDefault(char* &db_buf, int &db_len);

  int _loadDbDataFile(const string &fname,
                      char* &db_buf,
                      int &db_len);
  
  int _loadDbDataHttp(const string &fname,
                      char* &db_buf,
                      int &db_len);
  
  int _loadDbData(const string &fname);

  int _initDataFields(const char *param_buf,
                      long param_buf_len,
                      long line_no);
  
  int _initWindFields(const char *param_buf,
                      long param_buf_len,
                      long line_no);
  
  int _initDrawExportLinks();
  
  int _loadOverlayInfo(const char *param_buf, long param_buf_len,
                       long line_no,
                       Overlay_t **over, int  max_overlays);
  
  int _loadOverlayData(Overlay_t **over, int  num_overlays);
  
  int _initOverlays(const char *param_buf,
                    long param_buf_len,
                    long line_no);
};
  
  
