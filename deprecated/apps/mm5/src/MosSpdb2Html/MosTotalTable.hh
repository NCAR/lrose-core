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

#ifndef _MOS_TOTAL_TABLE_INC_
#define _MOS_TOTAL_TABLE_INC_

#include <ctime>  
#include <string>
using namespace std;

class MosTotalTable {

  public:
  //
  // Struct that holds enough to define a unique spdb
  // entry.
  //
  typedef struct {
    time_t dataTime;
    int dataType;
    int dataType2;
  } entry_t;
  //
  // Constructor. Loads up local args.
  //
  MosTotalTable(time_t Start,
			       time_t Now,
			       time_t End,
			       string StationURL,
			       string ForecastURL,
			       string OutDir,
			       string TableArchiveDir,
			       string StationID,
			       float Calm,
			       int MaxLeadTime,
			       float MinVisibility,
			       float MaxVisibility,
			       float MaxCeiling,
			       bool MinimumLeadTimeOnly,
			       bool Overlap);
  //
  // Destructor. Does nothing.
  //
  ~MosTotalTable();

  //
  // Main routine. Writes the table.
  //
  int WriteTable();



  //
  // Just to make copies of the constructor args.
  //
  private : 

  time_t _start;
  time_t _end;
  time_t _now;
  string _stationURL;
  string _forecastURL;
  string _outDir;
  string _tableArchiveDir;
  string _stationID;
  float _calm;
  int _maxLeadTime;
  float _minVisibility;
  float _maxVisibility;
  float _maxCeiling;
  bool _minimumLeadTimeOnly;
  bool _overlap;

};




#endif




