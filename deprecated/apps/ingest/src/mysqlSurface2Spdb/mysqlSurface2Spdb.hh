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
// mysqlSurface2Spdb.hh
//
// mysqlSurface2Spdb object
//
//
///////////////////////////////////////////////////////////////


#ifndef mysqlSurface2Spdb_H
#define mysqlSurface2Spdb_H

#include "Params.hh"
#include <toolsa/umisc.h>
#include <rapformats/station_reports.h>
#include <vector>

using namespace std;

class mysqlSurface2Spdb {
  
public:
  
  // constructor. Sets up DsMdvx object.
  mysqlSurface2Spdb (Params *TDRP_params);

  // 
  void  mysqlSurface2SpdbFile( char *filename); 
  
  // destructor.
  ~mysqlSurface2Spdb();

  
protected:
  
private:

  Params *_params;
  
  int _parseLine(char *Line, station_report_t *R,
		 int *stationID, date_time_t *T);

  int _inputFormat;

  const static int _unknownFormat = 0;
  const static int _metarFormat=1;
  const static int _samsFormat=2;

};

#endif
