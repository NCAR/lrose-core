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
// Fields.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
//
///////////////////////////////////////////////////////////////

#include "Fields.hh"
#include <cmath>

const double Fields::missingDouble = -9999;
const int Fields::missingInt = -9999;

// constructor

Fields::Fields()

{

  initialize();

}

// Initialize to missing

void Fields::initialize()

{
  
  flags = missingInt;

  snr = missingDouble;
  dbm = missingDouble;
  dbz = missingDouble;
  vel = missingDouble;
  width = missingDouble;

  // clutter-filtered fields

  clut = missingDouble;
  dbzf = missingDouble;
  velf = missingDouble;
  widthf = missingDouble;

  // dual polarization fields

  zdr = missingDouble;
  zdrm = missingDouble;
  
  ldrh = missingDouble;
  ldrv = missingDouble;

  rhohv = missingDouble;
  phidp = missingDouble;
  kdp = missingDouble;

  snrhc = missingDouble;
  snrhx = missingDouble;
  snrvc = missingDouble;
  snrvx = missingDouble;

  dbmhc = missingDouble;
  dbmhx = missingDouble;
  dbmvc = missingDouble;
  dbmvx = missingDouble;

  dbzhc = missingDouble;
  dbzhx = missingDouble;
  dbzvc = missingDouble;
  dbzvx = missingDouble;

  // SZ8-64 phase coding

  sz_trip_flag = missingInt;
  sz_leakage = missingDouble;
  sz_dbzt = missingDouble;

  // infilling after applying SZ

  sz_zinfill = missingInt;
  sz_itexture = missingDouble;
  sz_dbzi = missingDouble;
  sz_veli = missingDouble;
  sz_widthi = missingDouble;

  // CMD - Clutter Mitigation Decision

  cmd = missingDouble;
  cmd_flag = 0;

  cmd_dbz_diff_sq = missingDouble;
  cmd_spin_change = missingDouble;

  cmd_tdbz = missingDouble;
  cmd_sqrt_tdbz = missingDouble;
  cmd_spin = missingDouble;
  interest_min_tdbz_spin = missingDouble;
  interest_max_tdbz_spin = missingDouble;

  cmd_vel_sdev = missingDouble;

  cmd_power_ratio = missingDouble;
  cmd_dbz_narrow = missingDouble;
  cmd_ratio_narrow = missingDouble;
  cmd_pr_narrow = missingDouble;
  cmd_ratio_wide = missingDouble;
  cmd_pr_wide = missingDouble;
  cmd_clut2wx_sep = missingDouble;
  cmd_clut2wx_ratio = missingDouble;
  cmd_clut_width = missingDouble;
  cmd_wx2noise_ratio = missingDouble;
  interest_max_clut_width_sep = missingDouble;

  cmd_zdr_sdev = missingDouble;
  cmd_rhohv_sdev = missingDouble;
  cmd_phidp_sdev = missingDouble;

  cpa = missingDouble;
  aiq = missingDouble;
  niq = missingDouble;
  meani = missingDouble;
  meanq = missingDouble;

  test = missingDouble;
  
}

