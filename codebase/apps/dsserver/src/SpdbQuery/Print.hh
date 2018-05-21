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
// Print.hh
//
// Printing object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
/////////////////////////////////////////////////////////////

#ifndef PRINT_HH
#define PRINT_HH

#include <iostream>
#include <cstdio>
#include <Spdb/Spdb.hh>
using namespace std;

class Print {
  
public:

  // constructor

  Print(FILE *out, ostream &ostr, bool debug = false);

  // destructor

  virtual ~Print();

  // print methods

  void ascii(int data_len, void *data);
  void generic_pt(int data_len, void *data);
  void ac_vector(int data_len, void *data);
  void combo_pt(int data_len, void *data);
  void chunk_hdr(const Spdb::chunk_t &chunk);
  void stn_report(void *data);
  void acars(int data_len,  void *data);
  void amdar(int data_len,  void *data);
  void ac_posn(int data_len,  void *data);
  void ac_posn_wmod(int data_len,  void *data);
  void ac_georef(int data_len,  void *data);
  void ac_route(void *data);
  void ac_data(void *data);
  void bdry(void *data);
  void flt_path(void *data);
  void history_forecast(int data_len, void *data);
  void pirep(int data_len, void *data);
  void ltg(int data_len, void *data);
  void twn_ltg(int data_len, void *data);
  void symprod(int data_len, void *data);
  void sigmet(void *data);
  void sig_air_met(int data_len, void *data);
  void taf(int data_len, void *data);
  void tstorms(void *data);
  void trec_gauge(int data_len, void *data);
  void zr_params(void *data);
  void zrpf(int data_len, void *data);
  void zvpf(int data_len, void *data);
  void zvis_cal(int data_len, void *data);
  void zvis_fcast(int data_len, void *data);
  void posn_rpt(void *data);
  void sndg(void *data);
  void sndg_plus(int data_len, void *data);
  void edr_point(int data_len, void *data);
  void EDR_point(int data_len, void *data);
  void vergrid_region(int data_len, void *data);
  void gen_poly(int data_len, void *data);
  void usgs_data(int data_len, void *data);
  void nws_wwa(int data_len, void *data);
  void ds_radar_sweep(int data_len, void *data);
  void ds_radar_power(int data_len, void *data);
  void radar_spectra(int data_len, void *data);
  void wx_obs(int data_len,  void *data);
  void taiwan_awos(int data_len,  void *data);
  void checktimes(int data_len,  void *data);

protected:
  
private:

  FILE *_out;
  ostream &_ostr;
  bool _debug;
  
};

#endif

