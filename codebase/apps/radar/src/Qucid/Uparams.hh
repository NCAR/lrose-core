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
// Uparams.hh
//
// Uparams class provides parameter utilities
//
// Mike Dixon, RAP, NCAR, Boulder, CO, USA
// March 2001
//
/////////////////////////////////////////////////////////////

#include <string>
#include <vector>
using namespace std;

typedef struct {
  string name;
  string entry;
} Uparam_list_t;

class Uparams {

public:
  
  // constructor

  Uparams();

  // destructor

  ~Uparams();
  
  // Clear the data base
  
  void clear();
    
  // read in from param file
  // returns 0 on success, -1 on failure
  
  int read(const char *file_path, const char *prog_name);
  
  // read in from param file
  // returns 0 on success, -1 on failure
  
  int read(const char *buf, int buf_len,
	   const char *prog_name);

  // get param values of various types
  
  const char *getString(const char *param_name, const char *default_val);
  long getLong(const char *param_name, long default_val);
  int getInt(const char *param_name, int default_val);
  float getFloat(const char *param_name, float default_val);
  double getDouble(const char *param_name, double default_val);

private:

  vector<Uparam_list_t> _plist;

  const char *_get(const char *search_name) const;

};
  
