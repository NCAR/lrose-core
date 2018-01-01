/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1999
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1999/03/14 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// ScanParams.hh
//
// ScanParams object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#ifndef ScanParams_HH
#define ScanParams_HH

#include <iostream>
#include <vector>
using namespace std;

////////////////////////
// This class

class ScanParams {
  
public:

  bool setDone;
  int country_id;
  char radar_name[128];
  char radar_type[128];
  int station_id;
  int station_num;
  int wmo_number;
  int vdate;
  time_t time;
  char timestamp_str[32];
  char time_str[32];
  double lat;
  double lon;
  double ht;
  double version;
  double freq_mhz;
  int prf;
  double peak_power;
  double pulse_length;
  double angle_rate;
  double range_res;
  double angle_res;
  double az_corr;
  double el_corr;
  int video_res;
  double start_range;
  double end_range;
  bool clear_air;
  double vel_level;
  double nyquist;
  bool unfolding;
  double elev_angle;
  int scan_num;
  int n_scans;
  int tilt_num;
  int n_tilts;
  int volume_id;
  vector<double> dbz_levels;
  char field_name[64];
  char img_fmt[64];
  string product_str;
  string copyright_str;
  string hiprf_str;
  string polarization_str;

  double hbeamwidth;
  double vbeamwidth;
  double antdiam;

  double peakpowerh;
  double peakpowerv;

  double rxnoise_h;
  double rxnoise_v;

  double rxgain_h;
  double rxgain_v;

  // constructor

  ScanParams();

  // destructor
  
  ~ScanParams();

  // clear the members

  void clear();

  // set one of the params

  int set(const char *line, int nScansFull);

  // print the scan params

  void print(ostream &out) const;

  /// Set debugging on/off. Off by default.

  void setDebug(bool val) { _debug = val; }

protected:
private:

  bool _debug;

};

#endif
