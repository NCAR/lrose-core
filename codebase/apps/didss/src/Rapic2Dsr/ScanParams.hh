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
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class ScanParams {
  
public:

  bool setDone;
  int country_id;
  char radar_name[128];
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

  // constructor

  ScanParams(const Params &params);

  // destructor
  
  ~ScanParams();

  // clear the members

  void clear();

  // set one of the params

  int set(const char *line, int nScans);

  // print the scan params

  void print(ostream &out) const;

protected:

  const Params &_params;

private:

};

#endif
