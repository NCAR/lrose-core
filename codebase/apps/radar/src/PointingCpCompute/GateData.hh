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
// GateData.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2010
//
///////////////////////////////////////////////////////////////

#ifndef GateData_HH
#define GateData_HH

////////////////////////
// This class

class GateData {
  
public:
  
  GateData();
  
  // set values to 0 or missing
  
  void initialize();
  
  // accumulate values

  void addCo(double i_co, double q_co);

  void addXx(double i_xx, double q_xx);

  // compute derived fields
  // given sums etc.
  
  void computeDerived();

  // sums

  int count_co;
  int count_xx;

  double sum_power_co;
  double sum_dbm_co;
  double sum_dbm_co_sq;

  double sum_power_xx;
  double sum_dbm_xx;
  double sum_dbm_xx_sq;

  double sum_mag_co;
  double sum_i_co;
  double sum_q_co;
  
  double sum_mag_xx;
  double sum_i_xx;
  double sum_q_xx;

  // derived

  double mean_power_co;
  double mean_dbm_co;
  double sdev_dbm_co;

  double mean_power_xx;
  double mean_dbm_xx;
  double sdev_dbm_xx;

  double cpa_co;
  double cpa_xx;

  static const double missingVal;

protected:
private:

  void _computeMeanSdev(double count,
                        double sum,
                        double sum2,
                        double &mean,
                        double &sdev);
  
};

#endif

