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
/////////////////////////////////////////////////////////
// $Id: CheckOutlier.hh,v 1.4 2016/03/04 02:22:12 dixon Exp $
//
// Check outliers
//
// Yan Chen, RAL, NCAR
//
// Jan. 2008
//
//////////////////////////////////////////////////////////

# ifndef    CHECKOUTLIER_HH
# define    CHECKOUTLIER_HH

// C++ include files
#include <map>
#include <set>
#include <string>

// System/RAP include files
#include <Mdv/DsMdvx.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>

// Local include files
#include "Params.hh"
using namespace std;


class CheckOutlier {
  
public:

  // constructor
  CheckOutlier(const string &prog_name, const Params &params); 


  // destructor
  ~CheckOutlier();

  // check
  int check();

  DsMdvx stddevMdvx;
  DsMdvx meanMdvx;

protected:

private:

  const string &_progName;
  const Params &_params;

  map<string, set<int> > _fieldVlevelsMap;
  map<string, double> _fieldSigmaMap;
  map<string, double> _fieldSpaceMap;

  time_t _start_time;
  time_t _end_time;

  vector<time_t> _timelist;

  bool _files_match(
    Mdvx::master_header_t &mhdr1,
    Mdvx::master_header_t &mhdr2,
    string &err_str
  );

  bool _fields_match(
    MdvxField *field1,
    MdvxField *field2,
    string &err_str
  );

  char* _timeStr(const time_t ttime);

  int _checkFiles();
  int _checkOutliers();
  int _calcMaxPrecip();
  int _writeLogFile();

  class LogRecord {
  public:
    string field_name;
    string field_units;
    double vol_value;
    double mean;
    double std_dev;
    double x_location;
    double y_location;
    string xy_units;
    bool has_v_levels;
    int v_level;
    string v_type;
    double v_value;

    LogRecord(
      const string &field,
      const string &fieldUnits,
      const string &xyUnits
    ) : field_name(field), field_units(fieldUnits), xy_units(xyUnits) {

      vol_value = 0.0;
      mean = 0.0;
      std_dev = 0.0;
      x_location = 0.0;
      y_location = 0.0;
      has_v_levels = FALSE;
      v_level = 0;
    }
  };

  class LogFile {
  public:
    time_t mdv_time;
    time_t start_time;
    time_t end_time;
    string mdv_url;
    vector<LogRecord> logRecords;

    LogFile(time_t file_time, time_t start, time_t end, const string &url) :
      mdv_time(file_time), start_time(start), end_time(end), mdv_url(url) { }
  };

  vector<LogFile> _logFiles;

  string _maxPrecipLog;
  string _precipValuesLog;
};


# endif     /* CHECKOUTLIER_HH */
