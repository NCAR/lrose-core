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
// GateData.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2010
//
///////////////////////////////////////////////////////////////

#include "GateData.hh"
#include <cmath>

const double GateData::missingVal = -9999;

// constructor

GateData::GateData()

{

  initialize();

}

// Initialize to missing

void GateData::initialize()
  
{
  
  count_co = 0;
  count_xx = 0;

  sum_power_co = 0.0;
  sum_dbm_co = 0.0;
  sum_dbm_co_sq = 0.0;

  sum_power_xx = 0.0;
  sum_dbm_xx = 0.0;
  sum_dbm_xx_sq = 0.0;

  sum_mag_co = 0.0;
  sum_i_co = 0.0;
  sum_q_co = 0.0;

  sum_mag_xx = 0.0;
  sum_i_xx = 0.0;
  sum_q_xx = 0.0;

  mean_power_co = missingVal;
  mean_dbm_co = missingVal;
  sdev_dbm_co = missingVal;

  mean_power_xx = missingVal;
  mean_dbm_xx = missingVal;
  sdev_dbm_xx = missingVal;

  cpa_co = missingVal;
  cpa_xx = missingVal;

}

// accumulate values

void GateData::addCo(double i_co, double q_co)
  
{

  count_co++;

  double power_co = i_co * i_co + q_co * q_co;

  sum_power_co += power_co;
  double dbm_co = 10.0 * log10(power_co);
  sum_dbm_co += dbm_co;
  sum_dbm_co_sq += dbm_co * dbm_co;

  double mag_co = sqrt(power_co);
  sum_mag_co += mag_co;
  sum_i_co += i_co;
  sum_q_co += q_co;
  
}

void GateData::addXx(double i_xx, double q_xx)
  
{

  count_xx++;

  double power_xx = i_xx * i_xx + q_xx * q_xx;

  sum_power_xx += power_xx;
  double dbm_xx = 10.0 * log10(power_xx);
  sum_dbm_xx += dbm_xx;
  sum_dbm_xx_sq += dbm_xx * dbm_xx;

  double mag_xx = sqrt(power_xx);
  sum_mag_xx += mag_xx;
  sum_i_xx += i_xx;
  sum_q_xx += q_xx;
  
}

// compute derived fields

void GateData::computeDerived()
  
{

  mean_power_co = sum_power_co / count_co;
  
  _computeMeanSdev(count_co, sum_dbm_co, sum_dbm_co_sq,
                   mean_dbm_co, sdev_dbm_co);
  
  double phasor_len_co = sqrt(sum_i_co * sum_i_co +
			      sum_q_co * sum_q_co);
  cpa_co = phasor_len_co / sum_mag_co;
  
  mean_power_xx = sum_power_xx / count_xx;
  
  _computeMeanSdev(count_xx, sum_dbm_xx, sum_dbm_xx_sq,
                   mean_dbm_xx, sdev_dbm_xx);
  
  double phasor_len_xx = sqrt(sum_i_xx * sum_i_xx +
			      sum_q_xx * sum_q_xx);
  cpa_xx = phasor_len_xx / sum_mag_xx;

}

// compute mean and sdev

void GateData::_computeMeanSdev(double count,
                                double sum,
                                double sum2,
                                double &mean,
                                double &sdev)
  
{

  mean = missingVal;
  sdev = missingVal;

  if (count > 0) {
    mean = sum / count;
  }
  
  if (count > 2) {
    double var = (sum2 - (sum * sum) / count) / (count - 1.0);
    if (var >= 0.0) {
      sdev = sqrt(var);
    } else {
      sdev = 0.0;
    }
  }

}
