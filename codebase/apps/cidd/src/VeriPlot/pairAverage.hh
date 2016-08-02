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
// PAIRAVERAGE.HH 
// (c) 2008 UCAR/RAL
// F Hage. May 2008
using namespace std;
//
// Object that creates and stores an MDV sounding.
//

#ifndef pairAverage_H
#define pairAverage_H

#include <sys/types.h>
#include <string>
#include <vector>
#include <rapformats/GenPt.hh>
#include "Params.hh"

using namespace std;

class pairAverage {

public:
  // Constructor. Reads the data and fills up the vectors.
  pairAverage(Params *params, time_t bin_start_time, time_t bin_end_time);

  Params *p;  // Application parasm - Has needed field names.

  // Destructor
  ~pairAverage();
  void clear();
  int getNobs(); // returns the number of observations in the average.
  time_t get_bin_time(); // Returns the plot time of the temporal bin.
  
  double get_T(); // Get the Temperature Average
  double get_D(); // Get the Dew Point Temperature Average
  double get_P(); // Get the Pressure Average
  double get_RH(); // Get the Relative Humidity Average
  double get_WS(); // Get the Wind Speed Average
  double get_WD(); // Get the Wind Dir Average
  double get_U(); // Get the Wind U Average
  double get_V(); // Get the Wind V Average
  
  void load(GenPt &pt);
  
  const static double badVal = -999.0;
  
  time_t start_time; // Data allowed in bin after this time.
  time_t end_time;  // Data allowed in bin must be before this time.
  time_t bin_time;  // Center of the Time range for plotting.

  double T_rmse;     // Intermediate sum of square of differences
  double D_rmse;
  double P_rmse;
  double RH_rmse;
  double WS_rmse;
  double WD_rmse;
  double U_rmse;
  double V_rmse;

  double T_sum;     // Intermediate sum of square of differences
  double D_sum;
  double P_sum;
  double RH_sum;
  double WS_sum;
  double WD_sum;
  double U_sum;
  double V_sum;

  double T_min;     // minimum difference
  double D_min;
  double P_min;
  double RH_min;
  double WS_min;
  double WD_min;
  double U_min;
  double V_min;
 
  double T_max;     // Maximum difference
  double D_max;
  double P_max;
  double RH_max;
  double WS_max;
  double WD_max;
  double U_max;
  double V_max;
  
  int  T_count;   // Number of measurements in each result
  int  D_count;
  int  P_count;
  int  RH_count;
  int  WS_count;
  int  WD_count;
  int  U_count;
  int  V_count;

  private :
};

#endif
