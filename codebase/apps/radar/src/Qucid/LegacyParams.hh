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
    
  //  translate legacy to TDRP
  
  int translateToTdrp(const string &legacyParamsPath,
                      const string &tdrpParamsPath);

private:

  typedef struct {
    string name;
    string entry;
  } param_list_t;
  
  vector<param_list_t> _plist;

  bool _printTdrp;

  int _paramsBufLen;
  char *_paramsBuf; // Pointer to the parameter data

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
  const char *_getString(const char *param_name, const char *default_val);
  
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
  
  int _initDataFields(const char *param_buf,
                      long param_buf_len,
                      long line_no);
  
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
  
  
