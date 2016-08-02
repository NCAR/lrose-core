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
// Input.cc
//
// Input class - handles the input of spdb data and
// does some pre-conditioning on them (bad data set to
// the right value, duplicate stations avoided).
//
// Niles Oien
// Frank Hage, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan  1999
//
///////////////////////////////////////////////////////////////


#include <math.h>
#include <toolsa/port.h>
#include <rapmath/math_macros.h>

#include <cstdio>
#include <toolsa/mem.h>

#include "Input.hh"
using namespace std;


// Constructor 

Input::Input()
{
}

// Destructor.

Input::~Input()
{
  clear();
}

// clear arrays

void Input::clear()
{
  reports.clear();
}

// read the data
// returns 0 on success, -1 on failure

int Input::read(char *source_string,
                time_t t_start,
                time_t t_end,
                float bad,
                float MaxVis,
                float MaxCeiling)
{

  num_found = 0;

  // read from SPDB
  // specify stations must be unique - use the latest data
  
  DsSpdb spdb;
  spdb.setUniqueLatest();
  if (spdb.getInterval(source_string, t_start, t_end)) {
    fprintf(stderr,"Failed to get spdb data from %s\n",source_string);
    return -1;
  }
  const vector<Spdb::chunk_t> &chunks = spdb.getChunks();
  
  // load up vector of reports
  
  WxObs obs;
  interp_report_t report;
  reports.clear();
  for (int ii = 0; ii < (int) chunks.size(); ii++) {
    const Spdb::chunk_t &chunk = chunks[ii];
    if (obs.disassemble(chunk.data, chunk.len) == 0) {
      _loadReport(obs, report, bad, MaxVis, MaxCeiling);
      reports.push_back(report);
    }
  }
  num_found = (int) reports.size();
  
  return 0;

}

// load up a report

void Input::_loadReport(const WxObs &obs,
                        interp_report_t &report,
                        float bad,
                        float MaxVis,
                        float MaxCeiling)

{

  MEM_zero(report);
  report.lat = obs.getLatitude();
  report.lon = obs.getLongitude();
  report.alt = obs.getElevationMeters();
  
  if (obs.getTempCSize() == 0) {
    report.temp = bad;
  } else {
    report.temp = obs.getTempC();
  }

  if (obs.getDewpointCSize() == 0) {
    report.dew_point = bad;
  } else {
    report.dew_point = obs.getDewpointC();
  }

  if (obs.getRhPercentSize() == 0) {
    report.relhum = bad;
  } else {
    report.relhum = obs.getRhPercent();
  }

  if (obs.getWindSpeedMpsSize() == 0) {
    report.windspd = bad;
  } else {
    report.windspd = obs.getWindSpeedMps();
  }

  if (obs.getWindDirnDegtSize() == 0) {
    report.winddir = bad;
  } else {
    report.winddir = obs.getWindDirnDegt();
  }

  if (obs.getWindGustMpsSize() == 0) {
    report.windgust = bad;
  } else {
    report.windgust = obs.getWindGustMps();
  }

  if (report.windspd == bad || report.winddir == bad) {
    report.uu = bad;
    report.vv = bad;
  } else {
    report.uu = (-sin(report.winddir * RAD_PER_DEG) * report.windspd);
    report.vv = (-cos(report.winddir * RAD_PER_DEG) * report.windspd);
  }
  
  if (obs.getMslPressureMbSize() == 0) {
    report.pres = bad;
  } else {
    report.pres = obs.getSeaLevelPressureMb();
  }

  if (obs.getPrecipLiquidMmSize() == 0) {
    report.liquid_accum = bad;
  } else {
    report.liquid_accum = obs.getPrecipLiquidMm();
  }

  if (obs.getPrecipRateMmphSize() == 0) {
    report.precip_rate = bad;
  } else {
    report.precip_rate = obs.getPrecipRateMmPerHr();
  }

  if (obs.getVisibilityKmSize() == 0) {
    report.visibility = bad;
  } else {
    report.visibility = obs.getVisibilityKm();
  }
  if (report.visibility > MaxVis) {
    report.visibility = bad;
  }

  if (obs.getRvrKmSize() == 0) {
    report.rvr = bad;
  } else {
    double minRvr = obs.getRvrKm(0);
    for (int ii = 1; ii < (int) obs.getRvrKmSize(); ii++) {
      double rvr = obs.getRvrKm(ii);
      if (rvr >= 0 && rvr < minRvr) {
        minRvr = rvr;
      }
    }
    report.rvr = minRvr;
  }
  
  if (obs.getCeilingKmSize() == 0) {
    report.ceiling = bad;
  } else {
    report.ceiling = obs.getCeilingKm();
  }
  if (report.ceiling > MaxCeiling) {
    report.ceiling = bad;
  }

}

