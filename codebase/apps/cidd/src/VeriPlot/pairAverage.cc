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
// Object that creates and stores an Verification Pair averages. 
// (c) 2008 UCAR,RAL   All rights reserved.
// F.  Hage May, 2008

#include <stdlib.h>
#include <math.h>
#include <values.h>
#include <physics/physics.h>
#include "pairAverage.hh"

using namespace std;

// Constructor. Reads the data and fills up the vectors.
pairAverage::pairAverage(Params *params, time_t bin_start_time, time_t bin_end_time){
    p = params;
    start_time = bin_start_time;
    end_time = bin_end_time;
    bin_time = (end_time - start_time)/ 2 + start_time;  // Take the average
    clear();
}

// Add the point to the  RMSE accumulator - Computes the Intermediate sums, records min & max.
void  pairAverage::load(GenPt &pt){
    double Val = 0.0;

    double miss = pt.missingVal;
    string buf;

    buf = p->T_name;
    buf += "_RMSE";
    int T_rmse_index  = pt.getFieldNum(buf);
    if(T_rmse_index >= 0) T_rmse = pt.get1DVal(T_rmse_index);
    buf = p->T_name;
    buf += "_sum";
    int T_sum_index  = pt.getFieldNum(buf);
    if(T_sum_index >= 0) T_sum = pt.get1DVal(T_sum_index);
    buf = p->T_name;
    buf += "_count";
    int T_count_index  = pt.getFieldNum(buf);
    if(T_count_index >= 0) T_count = (int) pt.get1DVal(T_count_index);
    buf = p->T_name;
    buf += "_min";
    int T_min_index  = pt.getFieldNum(buf);
    if(T_min_index >= 0) T_min = pt.get1DVal(T_min_index);
    buf = p->T_name;
    buf += "_max";
    int T_max_index  = pt.getFieldNum(buf);
    if(T_max_index >= 0) T_max = pt.get1DVal(T_max_index);


    buf = p->D_name;
    buf += "_RMSE";
    int D_rmse_index  = pt.getFieldNum(buf);
    if(D_rmse_index >= 0) D_rmse = pt.get1DVal(D_rmse_index);
    buf = p->D_name;
    buf += "_sum";
    int D_sum_index  = pt.getFieldNum(buf);
    if(D_sum_index >= 0) D_sum = pt.get1DVal(D_sum_index);
    buf = p->D_name;
    buf += "_count";
    int D_count_index  = pt.getFieldNum(buf);
    if(D_count_index >= 0) D_count = (int) pt.get1DVal(D_count_index);
    buf = p->D_name;
    buf += "_min";
    int D_min_index  = pt.getFieldNum(buf);
    if(D_min_index >= 0) D_min = pt.get1DVal(D_min_index);
    buf = p->D_name;
    buf += "_max";
    int D_max_index  = pt.getFieldNum(buf);
    if(D_max_index >= 0) D_max = pt.get1DVal(D_max_index);


    buf = p->RH_name;
    buf += "_RMSE";
    int RH_rmse_index  = pt.getFieldNum(buf);
    if(RH_rmse_index >= 0) RH_rmse = pt.get1DVal(RH_rmse_index);
    buf = p->RH_name;
    buf += "_sum";
    int RH_sum_index  = pt.getFieldNum(buf);
    if(RH_sum_index >= 0) RH_sum = pt.get1DVal(RH_sum_index);
    buf = p->RH_name;
    buf += "_count";
    int RH_count_index  = pt.getFieldNum(buf);
    if(RH_count_index >= 0) RH_count = (int) pt.get1DVal(RH_count_index);
    buf = p->RH_name;
    buf += "_min";
    int RH_min_index  = pt.getFieldNum(buf);
    if(RH_min_index >= 0) RH_min = pt.get1DVal(RH_min_index);
    buf = p->RH_name;
    buf += "_max";
    int RH_max_index  = pt.getFieldNum(buf);
    if(RH_max_index >= 0) RH_max = pt.get1DVal(RH_max_index);


    buf = p->P_name;
    buf += "_RMSE";
    int P_rmse_index  = pt.getFieldNum(buf);
    if(P_rmse_index >= 0) P_rmse = pt.get1DVal(P_rmse_index);
    buf = p->P_name;
    buf += "_sum";
    int P_sum_index  = pt.getFieldNum(buf);
    if(P_sum_index >= 0) P_sum = pt.get1DVal(P_sum_index);
    buf = p->P_name;
    buf += "_count";
    int P_count_index  = pt.getFieldNum(buf);
    if(P_count_index >= 0) P_count = (int) pt.get1DVal(P_count_index);
    buf = p->P_name;
    buf += "_min";
    int P_min_index  = pt.getFieldNum(buf);
    if(P_min_index >= 0) P_min = pt.get1DVal(P_min_index);
    buf = p->P_name;
    buf += "_max";
    int P_max_index  = pt.getFieldNum(buf);
    if(P_max_index >= 0) P_max = pt.get1DVal(P_max_index);


    buf = p->WS_name;
    buf += "_RMSE";
    int WS_rmse_index  = pt.getFieldNum(buf);
    if(WS_rmse_index >= 0) WS_rmse = pt.get1DVal(WS_rmse_index);
    buf = p->WS_name;
    buf += "_sum";
    int WS_sum_index  = pt.getFieldNum(buf);
    if(WS_sum_index >= 0) WS_sum = pt.get1DVal(WS_sum_index);
    buf = p->WS_name;
    buf += "_count";
    int WS_count_index  = pt.getFieldNum(buf);
    if(WS_count_index >= 0) WS_count = (int) pt.get1DVal(WS_count_index);
    buf = p->WS_name;
    buf += "_min";
    int WS_min_index  = pt.getFieldNum(buf);
    if(WS_min_index >= 0) WS_min = pt.get1DVal(WS_min_index);
    buf = p->WS_name;
    buf += "_max";
    int WS_max_index  = pt.getFieldNum(buf);
    if(WS_max_index >= 0) WS_max = pt.get1DVal(WS_max_index);


    buf = p->WD_name;
    buf += "_RMSE";
    int WD_rmse_index  = pt.getFieldNum(buf);
    if(WD_rmse_index >= 0) WD_rmse = pt.get1DVal(WD_rmse_index);
    buf = p->WD_name;
    buf += "_sum";
    int WD_sum_index  = pt.getFieldNum(buf);
    if(WD_sum_index >= 0) WD_sum = pt.get1DVal(WD_sum_index);
    buf = p->WD_name;
    buf += "_count";
    int WD_count_index  = pt.getFieldNum(buf);
    if(WD_count_index >= 0) WD_count = (int) pt.get1DVal(WD_count_index);
    buf = p->WD_name;
    buf += "_min";
    int WD_min_index  = pt.getFieldNum(buf);
    if(WD_min_index >= 0) WD_min = pt.get1DVal(WD_min_index);
    buf = p->WD_name;
    buf += "_max";
    int WD_max_index  = pt.getFieldNum(buf);
    if(WD_max_index >= 0) WD_max = pt.get1DVal(WD_max_index);


    buf = p->U_name;
    buf += "_RMSE";
    int U_rmse_index  = pt.getFieldNum(buf);
    if(U_rmse_index >= 0) U_rmse = pt.get1DVal(U_rmse_index);
    buf = p->U_name;
    buf += "_sum";
    int U_sum_index  = pt.getFieldNum(buf);
    if(U_sum_index >= 0) U_sum = pt.get1DVal(U_sum_index);
    buf = p->U_name;
    buf += "_count";
    int U_count_index  = pt.getFieldNum(buf);
    if(U_count_index >= 0) U_count = (int) pt.get1DVal(U_count_index);
    buf = p->U_name;
    buf += "_min";
    int U_min_index  = pt.getFieldNum(buf);
    if(U_min_index >= 0) U_min = pt.get1DVal(U_min_index);
    buf = p->U_name;
    buf += "_max";
    int U_max_index  = pt.getFieldNum(buf);
    if(U_max_index >= 0) U_max = pt.get1DVal(U_max_index);


    buf = p->V_name;
    buf += "_RMSE";
    int V_rmse_index = pt.getFieldNum(buf);
    if(V_rmse_index >= 0) V_rmse = pt.get1DVal(V_rmse_index);
    buf = p->V_name;
    buf += "_sum";
    int V_sum_index  = pt.getFieldNum(buf);
    if(V_sum_index >= 0) V_sum = pt.get1DVal(V_sum_index);
    buf = p->V_name;
    buf += "_count";
    int V_count_index  = pt.getFieldNum(buf);
    if(V_count_index >= 0) V_count = (int) pt.get1DVal(V_count_index);
    buf = p->V_name;
    buf += "_min";
    int V_min_index  = pt.getFieldNum(buf);
    if(V_min_index >= 0) V_min = pt.get1DVal(V_min_index);
    buf = p->V_name;
    buf += "_max";
    int V_max_index  = pt.getFieldNum(buf);
    if(V_max_index >= 0) V_max = pt.get1DVal(V_max_index);
}


void  pairAverage::clear(){
  T_sum = 0.0;
  T_rmse = 0.0;
  T_count = 0;
  T_min = MAXDOUBLE;
  T_max =  -MAXDOUBLE;

  D_sum = 0.0;
  D_rmse = 0.0;
  D_count = 0;
  D_min = MAXDOUBLE;
  D_max =  -MAXDOUBLE;

  P_rmse = 0.0;
  P_sum = 0.0;
  P_count = 0;
  P_min = MAXDOUBLE;
  P_max =  -MAXDOUBLE;

  RH_rmse = 0.0;
  RH_sum = 0.0;
  RH_count = 0;
  RH_min = MAXDOUBLE;
  RH_max =  -MAXDOUBLE;

  WS_rmse = 0.0;
  WS_sum = 0.0;
  WS_count = 0;
  WS_min = MAXDOUBLE;
  WS_max =  -MAXDOUBLE;

  WD_rmse = 0.0;
  WD_sum = 0.0;
  WD_count = 0;
  WD_min = MAXDOUBLE;
  WD_max =  -MAXDOUBLE;

  U_rmse = 0.0;
  U_sum = 0.0;
  U_count = 0;
  U_min = MAXDOUBLE;
  U_max =  -MAXDOUBLE;

  V_rmse = 0.0;
  V_sum = 0.0;
  V_count = 0;
  V_min = MAXDOUBLE;
  V_max =  -MAXDOUBLE;
}

int pairAverage::getNobs(){
  int c = T_count;
  if(D_count > c) c = D_count;
  if(P_count > c) c = P_count;
  if(WS_count > c) c = WS_count;
  if(WD_count > c) c = WD_count;
  if(RH_count > c) c = RH_count;
  if(U_count > c) c = U_count;
  if(V_count > c) c = V_count;
  return c;
}
time_t pairAverage::get_bin_time(){
  return bin_time; 
}

// GET MEthods -  
// Return RMSE for Temperature
double pairAverage::get_T(){
  return T_rmse;
}
double pairAverage::get_D(){
  return D_rmse;
}
double pairAverage::get_P(){
  return P_rmse;
}
double pairAverage::get_WS(){
  return WS_rmse;
}
double pairAverage::get_WD(){
  return WD_rmse;
}
double pairAverage::get_RH(){
  return RH_rmse;
}
double pairAverage::get_U(){
  return U_rmse;
}
double pairAverage::get_V(){
  return V_rmse;
}

// Destructor
pairAverage::~pairAverage(){
}
