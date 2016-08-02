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
// Fields.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#ifndef Fields_HH
#define Fields_HH

////////////////////////
// This class

class Fields {
  
public:
  
  Fields();

  // set values to missing

  void initialize();

  // public data

  int flags;  // censoring flag

  double snr;
  double dbm; // uncalibrated power
  double dbz;
  double vel;
  double width;

  // clutter-filtered fields

  double clut;
  double dbzf;
  double velf;
  double widthf;

  // dual polarization fields

  double zdr;  // calibrated
  double zdrm; // measured - uncalibrated
  
  double ldrh;
  double ldrv;

  double rhohv;
  double phidp;
  double kdp;

  double snrhc;
  double snrhx;
  double snrvc;
  double snrvx;

  double dbmhc; // uncalibrated power
  double dbmhx;
  double dbmvc;
  double dbmvx;

  double dbzhc;
  double dbzhx;
  double dbzvc;
  double dbzvx;

  // SZ8-64 phase coding

  int sz_trip_flag;
  double sz_leakage;
  double sz_dbzt;

  // infilling after applying SZ

  int sz_zinfill;
  double sz_itexture;
  double sz_dbzi;   // infilled dbz
  double sz_veli;   // infilled velocity
  double sz_widthi; // infilled width

  // CMD - Clutter Mitigation Decision

  double cmd;
  int cmd_flag;
  
  double cmd_dbz_diff_sq;
  double cmd_spin_change;
  
  double cmd_tdbz;
  double cmd_sqrt_tdbz;
  double cmd_spin;
  double interest_min_tdbz_spin;
  double interest_max_tdbz_spin;

  double cmd_vel_sdev;

  double cmd_power_ratio;
  double cmd_dbz_narrow;
  double cmd_ratio_narrow;
  double cmd_pr_narrow;
  double cmd_ratio_wide;
  double cmd_pr_wide;
  double cmd_clut2wx_sep;
  double cmd_clut2wx_ratio;
  double cmd_clut_width;
  double interest_max_clut_width_sep;
  double cmd_wx2noise_ratio;
 
  double cmd_zdr_sdev;
  double cmd_rhohv_sdev;
  double cmd_phidp_sdev;

  // refractivity fields

  double cpa;  // clutter phase alignment
  double aiq;
  double niq;
  double meani;
  double meanq;

  // for testing

  double test;
  
  static const double missingDouble;
  static const int missingInt;

protected:
private:

};

#endif

