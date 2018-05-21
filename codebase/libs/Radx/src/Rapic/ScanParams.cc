///////////////////////////////////////////////////////////////
// ScanParams.cc
//
// ScanParams object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#include <vector>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <Radx/RadxTime.hh>
#include "ScanParams.hh"
#define MEM_zero(a) memset(&(a), 0, sizeof((a)))
using namespace std;

// Constructor

ScanParams::ScanParams()

{
  clear();
}

// destructor

ScanParams::~ScanParams()

{

}

////////////////////////
// clear the members

void ScanParams::clear()

{
  
  setDone = false;
  country_id = 0;
  station_id = 0;
  station_num = 0;
  wmo_number = 0;
  vdate = 0;
  time = 0;
  MEM_zero(timestamp_str);
  MEM_zero(time_str);
  lat = 0;
  lon = 0;
  ht = 0;
  version = 0;
  freq_mhz = 0;
  prf = 0;
  peak_power = 0; 
  pulse_length = 0;
  angle_rate = 0;
  range_res = 0;
  angle_res = 0;
  az_corr = 0;
  el_corr = 0;
  video_res = 0;
  start_range = 0;
  end_range = 0;
  clear_air = false;
  vel_level = 0;
  nyquist = 0;
  unfolding = false;
  elev_angle = 0;
  scan_num = 0;
  volume_id = 0; 
  MEM_zero(radar_name);
  strcpy(field_name,"Refl");  // rjp 7 Sep 2001. Set default value for field_name
                              // as VIDEO field not always included in radar data. 
  MEM_zero(img_fmt);

  hbeamwidth = -9999.0;
  vbeamwidth = -9999.0;
  antdiam = -9999.0;
  peakpowerh = -9999.0;
  peakpowerv = -9999.0;
  rxnoise_h = -9999.0;
  rxnoise_v = -9999.0;
  rxgain_h = -9999.0;
  rxgain_v = -9999.0;

}


////////////////////////
// set one of the params

int ScanParams::set(const char *line, int nScansFull)

{

  int _country_id;
  if (sscanf(line, "COUNTRY:%d", &_country_id) == 1) {
    country_id = _country_id;
    return 0;
  }

  char _radar_name[128];
  if (sscanf(line, "NAME:%s", _radar_name) == 1) {
    strncpy(radar_name, _radar_name, 128);
    setDone = true;
    return 0;
  }

  int _station_id;
  if (sscanf(line, "STNID:%d", &_station_id) == 1) {
    station_id = _station_id;
    return 0;
  }

  int _station_num;
  if (sscanf(line, "STN_NUM:%d", &_station_num) == 1) {
    station_num = _station_num;
    return 0;
  }

  int _wmo_number; 
  if (sscanf(line, "WMONUMBER:%d", &_wmo_number) == 1) {
    wmo_number = _wmo_number;
    return 0;
  }

  int _vdate;
  if (sscanf(line, "DATE:%d", &_vdate) == 1) {
    vdate = _vdate;
    return 0;
  }

  double _lat;
  if (sscanf(line, "LATITUDE:%lg", &_lat) == 1) {
    lat = _lat;
    return 0;
  }

  double _lon;
  if (sscanf(line, "LONGITUDE:%lg", &_lon) == 1) {
    lon = _lon;
    return 0;
  }

  double _ht;
  if (sscanf(line, "HEIGHT:%lg", &_ht) == 1) {
    ht = _ht;
    return 0;
  }

  if (sscanf(line, "TIMESTAMP:%s", timestamp_str) == 1) {
    int year, month, day, hour, min, sec;
    if (sscanf(timestamp_str, "%4d%2d%2d%2d%2d%2d",
	       &year, &month, &day,
	       &hour, &min, &sec) == 6) {
      RadxTime rtime(year, month, day, hour, min, sec);
      time = rtime.utime();
    } else {
      cerr << "ERROR - ScanParams::set" << endl;
      cerr << "  Cannot decode time stamp:" << line << endl;
      return -1;
    }
    return 0;
  }
  
  if (sscanf(line, "TIME:%s", time_str) == 1) {
    return 0;
  }
  
  double _version;
  if (sscanf(line, "VERS:%lg", &_version) == 1) {
    version = _version;
    return 0;
  }

  double _freq_mhz;
  if (sscanf(line, "FREQUENCY:%lg", &_freq_mhz) == 1) {
    freq_mhz = _freq_mhz;
    return 0;
  }

  int _prf;
  if (sscanf(line, "PRF:%d", &_prf) == 1) {
    prf = _prf;
    return 0;
  }

  double _peak_power;
  if (sscanf(line, "PEAKPOWER:%lg", &_peak_power) == 1) {
    peak_power = _peak_power;
    return 0;
  }

  double _pulse_length;
  if (sscanf(line, "PULSELENGTH:%lg", &_pulse_length) == 1) {
    pulse_length = _pulse_length;
    return 0;
  }

  double _angle_rate;
  if (sscanf(line, "ANGLERATE:%lg", &_angle_rate) == 1) {
    angle_rate = _angle_rate;
    return 0;
  }

  double _range_res;
  if (sscanf(line, "RNGRES:%lg", &_range_res) == 1) {
    range_res = _range_res;
    return 0;
  }

  double _angle_res;
  if (sscanf(line, "ANGRES:%lg", &_angle_res) == 1) {
    angle_res = _angle_res;
    return 0;
  }

  double _az_corr;
  if (sscanf(line, "AZCORR:%lg", &_az_corr) == 1) {
    az_corr = _az_corr;
    return 0;
  }

  double _el_corr;
  if (sscanf(line, "ELCORR:%lg", &_el_corr) == 1) {
    el_corr = _el_corr;
    return 0;
  }

  int _video_res;
  if (sscanf(line, "VIDRES:%d", &_video_res) == 1) {
    video_res = _video_res;
    dbz_levels.clear();
    if (video_res == 8) {
      dbz_levels.push_back(0.0);
      dbz_levels.push_back(12.0);
      dbz_levels.push_back(28.0);
      dbz_levels.push_back(39.0);
      dbz_levels.push_back(44.0);
      dbz_levels.push_back(49.0);
      dbz_levels.push_back(55.0);
      dbz_levels.push_back(70.0);
    } else if (video_res == 16) {
      dbz_levels.push_back(0.0);
      dbz_levels.push_back(12.0);
      dbz_levels.push_back(24.0);
      dbz_levels.push_back(28.0);
      dbz_levels.push_back(31.0);
      dbz_levels.push_back(34.0);
      dbz_levels.push_back(37.0);
      dbz_levels.push_back(40.0);
      dbz_levels.push_back(43.0);
      dbz_levels.push_back(46.0);
      dbz_levels.push_back(49.0);
      dbz_levels.push_back(52.0);
      dbz_levels.push_back(55.0);
      dbz_levels.push_back(58.0);
      dbz_levels.push_back(61.0);
      dbz_levels.push_back(64.0);
    }
    return 0;
  }

  if (strstr(line, "DBZLVL")) {
    if (video_res == 0) {
	cerr << "ERROR - ScanParams::set" << endl;
	cerr << "  Trying to decode dbz levels, "
	     << "but video_res not set yet" << endl;
	return -1;
    }
    char *lcopy = new char[strlen(line) + 1];
    strcpy(lcopy, line);
    char *tok = strtok(lcopy, " \n\t\r");
    dbz_levels.clear();
    dbz_levels.push_back(0.0);
    for (int i = 1; i < video_res; i++) {
      tok = strtok(NULL, " \n\t\r");
      int iret = 0;
      if (tok == NULL) {
	iret = -1;
      }
      if (iret == 0) {
	double level;
	if (sscanf(tok, "%lg", &level) != 1) {
	  iret = -1;
	} else {
	  dbz_levels.push_back(level);
	}
	
      }
      if (iret) {
	cerr << "ERROR - ScanParams::set" << endl;
	cerr << "  Cannot decode dbz levels" << endl;
	cerr << "  Expected nlevels: " << video_res << endl;
	cerr << "  Only found: " << i - 1 << endl;
	cerr << "  " << line;
        delete[] lcopy;
	return -1;
      }
    }
    delete[] lcopy;
    return 0;
  }

  double _start_range;
  if (sscanf(line, "STARTRNG:%lg", &_start_range) == 1) {
    start_range = _start_range;
    return 0;
  }

  double _end_range;
  if (sscanf(line, "ENDRNG:%lg", &_end_range) == 1) {
    end_range = _end_range;
    return 0;
  }

  int _scan_num, _n_scans;
  if (sscanf(line, "PASS:%d of %d", &_scan_num, &_n_scans) == 2) {
    if (_n_scans != nScansFull) {
      cerr << "ERROR - ScanParams::set" << endl;
      cerr << "  nscans incorrect" << endl;
      cerr << "  Found in header: " << nScansFull << endl;
      cerr << "  Found in file body: " << _n_scans << endl;
      return -1;
    } else if (_scan_num > nScansFull) {
      cerr << "ERROR - ScanParams::set" << endl;
      cerr << "  scan_num: " << _scan_num << "  too high." << endl;
      cerr << "  Max in full volume is: " << nScansFull << endl;
      return -1;
    } else {
      scan_num = _scan_num;
      n_scans = _n_scans;
    }
    return 0;
  }

  int _tilt_num, _n_tilts;
  if (sscanf(line, "TILT:%d of %d", &_tilt_num, &_n_tilts) == 2) {
    tilt_num = _tilt_num;
    n_tilts = _n_tilts;
    return 0;
  }

  char _img_fmt[64];
  if (sscanf(line, "IMGFMT:%s", _img_fmt) == 1) {
    strncpy(img_fmt, _img_fmt, 64);
    return 0;
  }

  int _volume_id; 
  if (sscanf(line, "VOLUMEID:%d", &_volume_id) == 1) {
    volume_id = _volume_id;
    return 0;
  }

  char _field_name[64];
  if (sscanf(line, "VIDEO:%s", _field_name) == 1) {
    strncpy(field_name, _field_name, 64);
    return 0;
  }

  double _elev_angle;
  if (sscanf(line, "ELEV:%lg", &_elev_angle) == 1) {
    elev_angle = _elev_angle;
    return 0;
  }

  if (strstr(line, "CLEARAIR") != NULL) {
    if (strstr(line, "OFF") != NULL) {
      clear_air = false;
    } else {
      clear_air = true;
    }
    return 0;
  }
  
  double _vel_level;
  if (sscanf(line, "VELLVL:%lg", &_vel_level) == 1) {
    vel_level = _vel_level;
    return 0;
  }

  double _nyquist;
  if (sscanf(line, "NYQUIST:%lg", &_nyquist) == 1) {
    nyquist = _nyquist;
    return 0;
  }

  if (strstr(line, "UNFOLDING") != NULL) {
    if (strstr(line, "None") != NULL) {
      unfolding = false;
    } else {
      unfolding = true;
    }
    return 0;
  }

  if (strncmp(line, "PRODUCT: ", 9) == 0) {
    product_str = line + 9;
    return 0;
  } 

  if (strncmp(line, "COPYRIGHT: ", 11) == 0) {
    copyright_str = line + 11;
    return 0;
  } 

  if (strncmp(line, "HIPRF: ", 7) == 0) {
    hiprf_str = line + 7;
    return 0;
  } 

  if (strncmp(line, "POLARISATION: ", 14) == 0) {
    polarization_str.append(line + 14);
    return 0;
  } 


  double dval;
  if (sscanf(line, "HBEAMWIDTH:%lg", &dval) == 1) {
    hbeamwidth = dval;
    return 0;
  }
  if (sscanf(line, "VBEAMWIDTH:%lg", &dval) == 1) {
    vbeamwidth = dval;
    return 0;
  }
  if (sscanf(line, "ANTDIAM:%lg", &dval) == 1) {
    antdiam = dval;
    return 0;
  }
  if (sscanf(line, "PEAKPOWERH:%lg", &dval) == 1) {
    peakpowerh = dval;
    return 0;
  }
  if (sscanf(line, "PEAKPOWERV:%lg", &dval) == 1) {
    peakpowerv = dval;
    return 0;
  }
  if (sscanf(line, "RXNOISE_H:%lg", &dval) == 1) {
    rxnoise_h = dval;
    return 0;
  }
  if (sscanf(line, "RXNOISE_V:%lg", &dval) == 1) {
    rxnoise_v = dval;
    return 0;
  }
  if (sscanf(line, "RXGAIN_H:%lg", &dval) == 1) {
    rxgain_h = dval;
    return 0;
  }
  if (sscanf(line, "RXGAIN_V:%lg", &dval) == 1) {
    rxgain_v = dval;
    return 0;
  }

  if (_debug) {
    cerr << "Unknown params: " << line << endl;
  }

  return 0;

}

////////////////////////
// print the scan params

void ScanParams::print(ostream &out) const

{

  out << endl;
  out << "SCAN PARAMS" << endl;
  out << "===========" << endl;

  out << "  country_id: " << country_id << endl;
  out << "  radar_name: " << radar_name << endl;
  out << "  station_id: " << station_id << endl;
  out << "  wmo_nomber: " << wmo_number << endl;
  out << "  lat: " << lat << endl;
  out << "  lon: " << lon << endl;
  out << "  ht: " << ht << endl;
  out << "  freq_mhz: " << freq_mhz << endl;
  out << "  prf: " << prf << endl;
  out << "  peak_power: " << peak_power << endl;
  out << "  pulse_length: " << pulse_length << endl;
  out << "  angle_rate: " << angle_rate << endl;
  out << "  range_res: " << range_res << endl;
  out << "  angle_res: " << angle_res << endl;
  out << "  az_corr: " << az_corr << endl;
  out << "  el_corr: " << el_corr << endl;
  out << "  video_res: " << video_res << endl;
  out << "  start_range: " << start_range << endl;
  out << "  end_range: " << end_range << endl;
  out << "  clear_air: " << clear_air << endl;
  out << "  vel_level: " << vel_level << endl;
  out << "  nyquist: " << nyquist << endl;
  out << "  unfolding: " << unfolding << endl;
  out << "  elev_angle: " << elev_angle << endl;
  out << "  scan_num: " << scan_num << endl;
  out << "  n_scans: " << n_scans << endl;
  out << "  tilt_num: " << tilt_num << endl;
  out << "  n_tilts: " << n_tilts << endl;
  out << "  volume_id: " << volume_id << endl;
  out << "  time: " << RadxTime::strm(time) << endl;
  out << "  timestamp_str: " << timestamp_str << endl;
  out << "  time_str: " << time_str << endl;
  out << "  field_name: " << field_name << endl;
  out << "  product_str: " << product_str << endl;
  out << "  copyright_str: " << copyright_str << endl;
  out << "  hiprf_str: " << hiprf_str << endl;
  out << "  polarization_str: " << polarization_str << endl;

  out << "  hbeamwidth: " << hbeamwidth << endl;
  out << "  vbeamwidth: " << vbeamwidth << endl;
  out << "  antdiam: " << antdiam << endl;
  out << "  peakpowerh: " << peakpowerh << endl;
  out << "  peakpowerv: " << peakpowerv << endl;
  out << "  rxnoise_h: " << rxnoise_h << endl;
  out << "  rxnoise_v: " << rxnoise_v << endl;
  out << "  rxgain_h: " << rxgain_h << endl;
  out << "  rxgain_v: " << rxgain_v << endl;

  out << endl;

}




