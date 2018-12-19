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
// RCS info
//   $Author: jcraig $
//   $Locker:  $
//   $Date: 2018/01/26 20:36:36 $
//   $Id: SweepFile.cc,v 1.8 2018/01/26 20:36:36 jcraig Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SweepFile: Class for controlling netCDF sweep files.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cmath>
#include <unistd.h>

#include <toolsa/os_config.h>
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <toolsa/str.h>

#include "SweepFile.hh"

using namespace std;

#define SHORT_STRING_LEN 16
#define LONG_STRING_LEN 80

const float SweepFile::MISSING_DATA_VALUE = -32768.0;

/*********************************************************************
 * Constructors
 */

SweepFile::SweepFile(const string &output_dir,
		     const double radar_lat,    /* deg */
		     const double radar_lon,    /* deg */
		     const double radar_alt,    /* km */
		     const time_t sweep_start_time,
		     const float elev0,
		     const int tilt_num,
		     const int vol_num,
		     const double start_range,  /* km */
		     const double gate_spacing, /* km */
		     const int n_az,
		     const int n_gates,
		     const int samples_per_beam,
		     const double nyquist_velocity,
		     const double radar_constant,
		     const double wave_length,   /* cm */
		     const double prf,
		     const bool debug) :
  _debug(debug),
  _outputDir(output_dir),
  _radarLat(radar_lat),
  _radarLon(radar_lon),
  _radarAlt(radar_alt),
  _tiltNum(tilt_num),
  _volNum(vol_num),
  _startRange(start_range),
  _gateSpacing(gate_spacing),
  _nAz(n_az),
  _nGates(n_gates),
  _nFields(18),
  _samplesPerBeam(samples_per_beam),
  _nyquistVelocity(nyquist_velocity),
  _radarConstant(radar_constant),
  _waveLength(wave_length),
  _prf(prf),
  _sweepStartTime(sweep_start_time)
{
  if (_debug)
  {
    cerr << "SweepFile" << endl;
    cerr << "  tiltNum: " << _tiltNum << endl;
    cerr << "  volNum: " << _volNum << endl;
  }

  // compute file path
  
  DateTime stime(sweep_start_time);
  char path[MAX_PATH_LEN];
  sprintf(path,
	  "%s/ncswp_CHILL_%.4d%.2d%.2d_%.2d%.2d%.2d.000_v%.3d_s%.2d_%.1f_SUR_.nc",
	  _outputDir.c_str(),
	  stime.getYear(), stime.getMonth(), stime.getDay(),
	  stime.getHour(), stime.getMin(), stime.getSec(),
	  _volNum, _tiltNum, elev0);
  
  _outPath = path;
  _tmpPath = _outPath + ".tmp";
  
}

/*********************************************************************
 * Destructor
 */

SweepFile::~SweepFile()
{
}


/*********************************************************************
 * write() - Write sweep file
 *
 * Returns true on success, false on failure
 */

bool SweepFile::write(const double *time_offset,
		      const float *azimuth, const float *elevation,
		      const float *h_avg_i, const float *h_avg_q,
		      const float *h_dbz, const float *h_vel,
		      const float *h_ncp, const float *h_sw,
		      const float *h_niq, const float *h_aiq,
		      const float *h_dm,
		      const float *v_avg_i, const float *v_avg_q,
		      const float *v_dbz, const float *v_vel,
		      const float *v_ncp, const float *v_sw,
		      const float *v_niq, const float *v_aiq,
		      const float *v_dm)
{
  // make output dir in case necessary

  if (ta_makedir_recurse(_outputDir.c_str())) {
    cerr << "ERROR - SweepFile::write" << endl;
    cerr << "  Cannot make output dir: " << _outputDir << endl;
    cerr << "  " << strerror(errno) << endl;
    return false;
  }

  // write the tmp file
  
  if (!_writeTmp(time_offset, azimuth, elevation,
		 h_avg_i, h_avg_q,
		 h_dbz, h_vel, h_ncp, h_sw,
		 h_niq, h_aiq, h_dm,
		 v_avg_i, v_avg_q,
		 v_dbz, v_vel, v_ncp, v_sw,
		 v_avg_i, v_avg_q, v_dm)) {
    cerr << "ERROR - SweepFile::write" << endl;
    cerr << "  Cannot write tmp file: " << _tmpPath << endl;
    unlink(_tmpPath.c_str());
    return false;
  }
  
  // move the file to the output path
  
  if (rename(_tmpPath.c_str(), _outPath.c_str())) {
    cerr << "ERROR - SweepFile::write" << endl;
    cerr << "  Cannot rename tmp file: " << _tmpPath << endl;
    cerr << "                to  file: " << _outPath << endl;
    cerr << "  " << strerror(errno) << endl;
    return false;
  }

  return true;
}


/*********************************************************************
 * _putField() - Put the given data field into the netCDF file.
 */

void SweepFile::_putField(Nc3File &out,
			  const string &field_name,
			  const string &field_name_long,
			  const string &units,
			  const float *data,
			  const Nc3Dim *TimeDim,
			  const Nc3Dim *maxCellsDim,
			  const float missing_data_value)
{
//  Nc3Var *field = out.add_var(field_name.c_str(), nc3Short,
  Nc3Var *field = out.add_var(field_name.c_str(), nc3Float,
			     TimeDim, maxCellsDim);
  field->add_att("variable_type", "data");
  field->add_att("long_name", field_name_long.c_str());
  field->add_att("units", units.c_str());
//  field->add_att("scale_factor", scale);
//  field->add_att("add_offset", bias);
  field->add_att("missing_value", missing_data_value);
  field->add_att("_fillValue", missing_data_value);
  field->add_att("polarization", "Horizontal");
  field->add_att("Frequencies_GHz", 20.0 / _waveLength);
  field->add_att("InterPulsePeriods_secs", 1.0 / _prf);
  field->add_att("num_segments", 1);
  field->add_att("cells_in_segment", _nGates);
  field->add_att("meters_to_first_cell",
		 (float) _startRange * 1000.0);
  field->add_att("meters_between_cells",
		 (float) _gateSpacing * 1000.0);

//  // convert float to short
//
//  short sdata[_nAz * _nGates];
//  MEM_zero(sdata);
//  ui08 *udata = (ui08 *) fld->getPlane(_tiltNum);
//  for (int i = 0; i < _nAz * _nGates; i++) {
//    if ((fl32) udata[i] == fhdr.missing_data_value ||
//	(fl32) udata[i] == fhdr.bad_data_value) {
//      sdata[i] = _scaledMissingDataValue;
//    }
//    else {
//      sdata[i] = udata[i];
//    }
//  }
    
  long edges[2];
  edges[0] = _nAz;
  edges[1] = _nGates;
//  field->put(sdata, edges);
  field->put(data, edges);
}

    
/*********************************************************************
 * _writeTmp() - Write output to tmp file.
 *
 * Returns true on success, false on failure
 */

bool SweepFile::_writeTmp(const double *time_offset,
			  const float *azimuth,
			  const float *elevation,
			  const float *h_avg_i, const float *h_avg_q,
			  const float *h_dbz, const float *h_vel,
			  const float *h_ncp, const float *h_sw,
			  const float *h_niq, const float *h_aiq,
			  const float *h_dm,
			  const float *v_avg_i, const float *v_avg_q,
			  const float *v_dbz, const float *v_vel,
			  const float *v_ncp, const float *v_sw,
			  const float *v_niq, const float *v_aiq,
			  const float *v_dm)
{
  // create Nc3File object

  Nc3Error err(Nc3Error::verbose_nonfatal);

  Nc3File out(_tmpPath.c_str(), Nc3File::Replace);
  if (!out.is_valid())
  {
    cerr << "ERROR - SweepFile::write" << endl;
    cerr << "  Cannot create file: " << _tmpPath << endl;
    return false;
  }

  if (_debug)
    cerr << "  Writing tmp file: " << _tmpPath << endl;

  /////////////////////
  // global attributes
  
  char cbuf[1024];

  out.add_att("Content", "Radar sweep file from CHILL radar for REFRACTT project");
  out.add_att("Conventions", "NCAR_ATD-NOAA_ETL/Scanning_Remote_Sensor");
  out.add_att("Instrument_Name", "CHILL");
  out.add_att("Instrument_Type", "GROUND");
  out.add_att("Scan_Mode", "SUR");

  DateTime vtime(_sweepStartTime);
  sprintf(cbuf, "%.4d/%.2d/%.2d %.2d:%.2d:%2d",
	  vtime.getYear(), vtime.getMonth(), vtime.getDay(),
	  vtime.getHour(), vtime.getMin(), vtime.getSec());
  out.add_att("Volume_Start_Time", cbuf);

  DateTime stime(_sweepStartTime);
  out.add_att("Year", stime.getYear());
  out.add_att("Month", stime.getMonth());
  out.add_att("Day", stime.getDay());
  out.add_att("Hour", stime.getHour());
  out.add_att("Minute", stime.getMin());
  out.add_att("Second", stime.getSec());

  out.add_att("Volume_Number", _volNum);
  out.add_att("Scan_Number", _tiltNum);
  out.add_att("Num_Samples", _samplesPerBeam);
  out.add_att("Index_of_horizontal_information", 0);
  out.add_att("Index_of_vertical_information", 1);
  out.add_att("Project_Name", "REFRACTT");

  DateTime now(time(NULL));
  sprintf(cbuf, "%.4d/%.2d/%.2d %.2d:%.2d:%2d",
	  now.getYear(), now.getMonth(), now.getDay(),
	  now.getHour(), now.getMin(), now.getSec());
  out.add_att("Production_Date", cbuf);

  out.add_att("Producer_Name", "NSF/UCAR/NCAR/RAP");
  out.add_att("Software", "CalcNexrad");
  out.add_att("Range_Segments", "All data has fixed cell spacing");

  // add dimensions

  Nc3Dim *TimeDim = out.add_dim("Time");
  Nc3Dim *maxCellsDim = out.add_dim("maxCells", _nGates);
  Nc3Dim *numSystemsDim = out.add_dim("numSystems", 1);
  Nc3Dim *fieldsDim = out.add_dim("fields", _nFields);
  Nc3Dim *short_stringDim = out.add_dim("short_string", SHORT_STRING_LEN);
  out.add_dim("long_string", LONG_STRING_LEN);

  // add variables

  // volume_start_time

  Nc3Var *volume_start_time = out.add_var("volume_start_time", nc3Int);
  volume_start_time->add_att("long_name",
			     "Unix Date/Time value for volume start time");
  volume_start_time->add_att("units",
			     "seconds since 1970-01-01 00:00 UTC");
  int s_time = _sweepStartTime;
  volume_start_time->put(&s_time);

  // base_time

  Nc3Var *base_time = out.add_var("base_time", nc3Int);
  base_time->add_att("long_name",
		     "Unix Date/Time value for first record");
  base_time->add_att("units",
		     "seconds since 1970-01-01 00:00 UTC");
  int b_time = _sweepStartTime;
  base_time->put(&b_time);

  // field names

  Nc3Var *fields = out.add_var("fields", nc3Char, fieldsDim, short_stringDim);
  {
    char fnames[_nFields * SHORT_STRING_LEN];
    memset(fnames, 0, _nFields * SHORT_STRING_LEN);
    STRncopy(fnames + 0 * SHORT_STRING_LEN, "h_avg_i", SHORT_STRING_LEN);
    STRncopy(fnames + 1 * SHORT_STRING_LEN, "h_avg_q", SHORT_STRING_LEN);
    STRncopy(fnames + 2 * SHORT_STRING_LEN, "h_dbz", SHORT_STRING_LEN);
    STRncopy(fnames + 3 * SHORT_STRING_LEN, "h_vel", SHORT_STRING_LEN);
    STRncopy(fnames + 4 * SHORT_STRING_LEN, "h_ncp", SHORT_STRING_LEN);
    STRncopy(fnames + 5 * SHORT_STRING_LEN, "h_sw", SHORT_STRING_LEN);
    STRncopy(fnames + 6 * SHORT_STRING_LEN, "h_niq", SHORT_STRING_LEN);
    STRncopy(fnames + 7 * SHORT_STRING_LEN, "h_aiq", SHORT_STRING_LEN);
    STRncopy(fnames + 8 * SHORT_STRING_LEN, "h_dm", SHORT_STRING_LEN);
    STRncopy(fnames + 9 * SHORT_STRING_LEN, "v_avg_i", SHORT_STRING_LEN);
    STRncopy(fnames + 10 * SHORT_STRING_LEN, "v_avg_q", SHORT_STRING_LEN);
    STRncopy(fnames + 11 * SHORT_STRING_LEN, "DBZ", SHORT_STRING_LEN);
    STRncopy(fnames + 12 * SHORT_STRING_LEN, "VE", SHORT_STRING_LEN);
    STRncopy(fnames + 13 * SHORT_STRING_LEN, "ncp", SHORT_STRING_LEN);
    STRncopy(fnames + 14 * SHORT_STRING_LEN, "sw", SHORT_STRING_LEN);
    STRncopy(fnames + 15 * SHORT_STRING_LEN, "NIQ", SHORT_STRING_LEN);
    STRncopy(fnames + 16 * SHORT_STRING_LEN, "AIQ", SHORT_STRING_LEN);
    STRncopy(fnames + 17 * SHORT_STRING_LEN, "DM", SHORT_STRING_LEN);
    
    long edges[2];
    edges[0] = _nFields;
    edges[1] = SHORT_STRING_LEN;
    fields->put(fnames, edges);
  }

  // Fixed_Angle

  Nc3Var *Fixed_Angle =
    out.add_var("Fixed_Angle", nc3Float);
  Fixed_Angle->add_att("long_name",
		       "Targeted fixed angle for this scan");
  Fixed_Angle->add_att("units", "degrees");
  Fixed_Angle->add_att("_FillValue", MISSING_DATA_VALUE);
  Fixed_Angle->add_att("missing_value", MISSING_DATA_VALUE);
  float f_angle = elevation[0];
  Fixed_Angle->put(&f_angle);

  // Range_to_First_Cell

  Nc3Var *Range_to_First_Cell = out.add_var("Range_to_First_Cell", nc3Float);
  Range_to_First_Cell->add_att("long_name",
			       "Range to the center of the first cell");
  Range_to_First_Cell->add_att("units", "meters");
  Range_to_First_Cell->add_att("_FillValue", MISSING_DATA_VALUE);
  Range_to_First_Cell->add_att("missing_value", MISSING_DATA_VALUE);
  float s_range = _startRange * 1000.0;
  Range_to_First_Cell->put(&s_range);

  // Cell_Spacing

  Nc3Var *Cell_Spacing = out.add_var("Cell_Spacing", nc3Float);
  Cell_Spacing->add_att("long_name",
			"Distance between cells");
  Cell_Spacing->add_att("units", "meters");
  Cell_Spacing->add_att("_FillValue", MISSING_DATA_VALUE);
  Cell_Spacing->add_att("missing_value", MISSING_DATA_VALUE);
  float g_spacing = _gateSpacing * 1000.0;
  Cell_Spacing->put(&g_spacing);

  // Nyquist_Velocity
  
  Nc3Var *Nyquist_Velocity = out.add_var("Nyquist_Velocity", nc3Float);
  Nyquist_Velocity->add_att("long_name",
			    "Effective unambiguous velocity");
  Nyquist_Velocity->add_att("units", "meters/second");
  Nyquist_Velocity->add_att("_FillValue", MISSING_DATA_VALUE);
  Nyquist_Velocity->add_att("missing_value", MISSING_DATA_VALUE);
  float nyquist = _nyquistVelocity;
  Nyquist_Velocity->put(&nyquist);

  // Unambiguous_Range

  Nc3Var *Unambiguous_Range = out.add_var("Unambiguous_Range", nc3Float);
  Unambiguous_Range->add_att("long_name",
			     "Effective unambiguous range");
  Unambiguous_Range->add_att("units", "meters");
  Unambiguous_Range->add_att("_FillValue", MISSING_DATA_VALUE);
  Unambiguous_Range->add_att("missing_value", MISSING_DATA_VALUE);
//  float u_range = _unambiguousRange * 1000.0;
//  Unambiguous_Range->put(&u_range);

  // Latitude

  Nc3Var *Latitude = out.add_var("Latitude", nc3Double);
  Latitude->add_att("long_name",
		    "Latitude of the instrument");
  Latitude->add_att("units", "degrees");
  Latitude->add_att("_FillValue", (double)MISSING_DATA_VALUE);
  Latitude->add_att("missing_value", (double)MISSING_DATA_VALUE);
  {
    float validRange[2] = {-90.0, 90.0};
    Latitude->add_att("valid_range", 2, validRange);
  }
  double lat = _radarLat;
  Latitude->put(&lat);

  // Longitude

  Nc3Var *Longitude = out.add_var("Longitude", nc3Double);
  Longitude->add_att("long_name",
		    "Longitude of the instrument");
  Longitude->add_att("units", "degrees");
  Longitude->add_att("_FillValue", (double)MISSING_DATA_VALUE);
  Longitude->add_att("missing_value", (double)MISSING_DATA_VALUE);
  {
    float validRange[2] = {-360.0, 360.0};
    Longitude->add_att("valid_range", 2, validRange);
  }
  double lon = _radarLon;
  Longitude->put(&lon);

  // Altitude

  Nc3Var *Altitude = out.add_var("Altitude", nc3Double);
  Altitude->add_att("long_name",
		    "Altitude in meters (asl) of the instrument");
  Altitude->add_att("units", "meters");
  Altitude->add_att("_FillValue", (double)MISSING_DATA_VALUE);
  Altitude->add_att("missing_value", (double)MISSING_DATA_VALUE);
  {
    float validRange[2] = {-10000.0, 90000.0};
    Altitude->add_att("valid_range", 2, validRange);
  }
  double alt = _radarAlt * 1000.0;
  Altitude->put(&alt);

  // Radar_Constant

  Nc3Var *Radar_Constant =
    out.add_var("Radar_Constant", nc3Float, numSystemsDim);
  Radar_Constant->add_att("long_name", "Radar constant");
  Radar_Constant->add_att("units", "mm6/(m3.mW.km-2)");
  Radar_Constant->add_att("_FillValue", MISSING_DATA_VALUE);
  Radar_Constant->add_att("missing_value", MISSING_DATA_VALUE);
  {
    float fval = _radarConstant;
    long edge = 1;
    Radar_Constant->put(&fval, &edge);
  }

  // rcvr_gain

  Nc3Var *rcvr_gain = out.add_var("rcvr_gain", nc3Float, numSystemsDim);
  rcvr_gain->add_att("long_name", "Receiver Gain");
  rcvr_gain->add_att("units", "dB");
  rcvr_gain->add_att("_FillValue", MISSING_DATA_VALUE);
  rcvr_gain->add_att("missing_value", MISSING_DATA_VALUE);
//  {
//    float fval = _radarParams.receiverGain;
//    long edge = 1;
//    rcvr_gain->put(&fval, &edge);
//  }

  // ant_gain

  Nc3Var *ant_gain = out.add_var("ant_gain", nc3Float, numSystemsDim);
  ant_gain->add_att("long_name", "Antenna Gain");
  ant_gain->add_att("units", "dB");
  ant_gain->add_att("_FillValue", MISSING_DATA_VALUE);
  ant_gain->add_att("missing_value", MISSING_DATA_VALUE);
//  {
//    float fval = _rcvrGain;
//    long edge = 1;
//    ant_gain->put(&fval, &edge);
//  }

  // sys_gain

  Nc3Var *sys_gain = out.add_var("sys_gain", nc3Float, numSystemsDim);
  sys_gain->add_att("long_name", "System Gain");
  sys_gain->add_att("units", "dB");
  sys_gain->add_att("_FillValue", MISSING_DATA_VALUE);
  sys_gain->add_att("missing_value", MISSING_DATA_VALUE);
//  {
//    float fval = _systemGain;
//    long edge = 1;
//    sys_gain->put(&fval, &edge);
//  }

  // bm_width

  Nc3Var *bm_width = out.add_var("bm_width", nc3Float, numSystemsDim);
  bm_width->add_att("long_name", "Beam Width");
  bm_width->add_att("units", "degrees");
  bm_width->add_att("_FillValue", MISSING_DATA_VALUE);
  bm_width->add_att("missing_value", MISSING_DATA_VALUE);
  {
    float fval = 1.0;
    long edge = 1;
    bm_width->put(&fval, &edge);
  }

  // pulse_width
  
  Nc3Var *pulse_width = out.add_var("pulse_width", nc3Float, numSystemsDim);
  pulse_width->add_att("long_name", "Pulse Width");
  pulse_width->add_att("units", "seconds");
  pulse_width->add_att("_FillValue", MISSING_DATA_VALUE);
  pulse_width->add_att("missing_value", MISSING_DATA_VALUE);
//  {
//    float fval = _pulseWidth / 1.0e6;
//    long edge = 1;
//    pulse_width->put(&fval, &edge);
//  }

  // band_width

  Nc3Var *band_width = out.add_var("band_width", nc3Float, numSystemsDim);
  band_width->add_att("long_name", "Band Width");
  band_width->add_att("units", "hertz");
  band_width->add_att("_FillValue", MISSING_DATA_VALUE);
  band_width->add_att("missing_value", MISSING_DATA_VALUE);

  // peak_pwr

  Nc3Var *peak_pwr = out.add_var("peak_pwr", nc3Float, numSystemsDim);
  peak_pwr->add_att("long_name", "Peak Power");
  peak_pwr->add_att("units", "watts");
  peak_pwr->add_att("_FillValue", MISSING_DATA_VALUE);
  peak_pwr->add_att("missing_value", MISSING_DATA_VALUE);
//  {
//    float fval = _radarParams.xmitPeakPower;
//    long edge = 1;
//    peak_pwr->put(&fval, &edge);
//  }

  // xmtr_power

  Nc3Var *xmtr_power = out.add_var("xmtr_power", nc3Float, numSystemsDim);
  xmtr_power->add_att("long_name", "Transmitter Power");
  xmtr_power->add_att("units", "dBM");
  xmtr_power->add_att("_FillValue", MISSING_DATA_VALUE);
  xmtr_power->add_att("missing_value", MISSING_DATA_VALUE);

  // noise_pwr

  Nc3Var *noise_pwr = out.add_var("noise_pwr", nc3Float, numSystemsDim);
  noise_pwr->add_att("long_name", "Noise Power");
  noise_pwr->add_att("units", "dBM");
  noise_pwr->add_att("_FillValue", MISSING_DATA_VALUE);
  noise_pwr->add_att("missing_value", MISSING_DATA_VALUE);

  // tst_pls_pwr

  Nc3Var *tst_pls_pwr = out.add_var("tst_pls_pwr", nc3Float, numSystemsDim);
  tst_pls_pwr->add_att("long_name", "Test Pulse Power");
  tst_pls_pwr->add_att("units", "dBM");
  tst_pls_pwr->add_att("_FillValue", MISSING_DATA_VALUE);
  tst_pls_pwr->add_att("missing_value", MISSING_DATA_VALUE);

  // tst_pls_rng0

  Nc3Var *tst_pls_rng0 = out.add_var("tst_pls_rng0", nc3Float, numSystemsDim);
  tst_pls_rng0->add_att("long_name", "Range to start of test pulse");
  tst_pls_rng0->add_att("units", "meters");
  tst_pls_rng0->add_att("_FillValue", MISSING_DATA_VALUE);
  tst_pls_rng0->add_att("missing_value", MISSING_DATA_VALUE);

  // tst_pls_rng1

  Nc3Var *tst_pls_rng1 = out.add_var("tst_pls_rng1", nc3Float, numSystemsDim);
  tst_pls_rng1->add_att("long_name", "Range to end of test pulse");
  tst_pls_rng1->add_att("units", "meters");
  tst_pls_rng1->add_att("_FillValue", MISSING_DATA_VALUE);
  tst_pls_rng1->add_att("missing_value", MISSING_DATA_VALUE);

  // Wavelength

  Nc3Var *Wavelength = out.add_var("Wavelength", nc3Float, numSystemsDim);
  Wavelength->add_att("long_name", "System wavelength");
  Wavelength->add_att("units", "meters");
  Wavelength->add_att("_FillValue", MISSING_DATA_VALUE);
  Wavelength->add_att("missing_value", MISSING_DATA_VALUE);
  {
    float fval = _waveLength / 100.0;
    long edge = 1;
    Wavelength->put(&fval, &edge);
  }

  // PRF

  Nc3Var *PRF = out.add_var("PRF", nc3Float, numSystemsDim);
  PRF->add_att("long_name", "System pulse repetition frequency");
  PRF->add_att("units", "/sec");
  PRF->add_att("_FillValue", MISSING_DATA_VALUE);
  PRF->add_att("missing_value", MISSING_DATA_VALUE);
  {
    float fval = _prf;
    long edge = 1;
    PRF->put(&fval, &edge);
  }

  // time_offset

  Nc3Var *time_offset_var =
    out.add_var("time_offset", nc3Double, TimeDim);
  time_offset_var->add_att("long_name",
			   "time offset of the current record from base_time");
  time_offset_var->add_att("units", "seconds");
  time_offset_var->add_att("_missing", 0.0);
  time_offset_var->add_att("_FillValue", 0.0);
  {
    long edge = _nAz;
    time_offset_var->put(time_offset, &edge);
  }

  // Azimuth
  
  Nc3Var *Azimuth = out.add_var("Azimuth", nc3Float, TimeDim);
  Azimuth->add_att("long_name",
		   "Earth relative azimuth of the ray");
  Azimuth->add_att("Comment",
		   "Degrees clockwise from true North");
  Azimuth->add_att("units", "degrees");
  Azimuth->add_att("missing_value", -32768.f);
  Azimuth->add_att("_FillValue", -32768.f);
  {
    float validRange[2] = {-360.0f, 360.0f};
    Azimuth->add_att("valid_range", 2, validRange);
  }
  {
    long edge = _nAz;
    Azimuth->put(azimuth, &edge);
  }

  // Elevation
  
  Nc3Var *Elevation = out.add_var("Elevation", nc3Float, TimeDim);
  Elevation->add_att("long_name",
		   "Earth relative elevation of the ray");
  Elevation->add_att("Comment",
		     "Degrees from earth tangent towards zenith");
  Elevation->add_att("units", "degrees");
  Elevation->add_att("missing_value", -32768.f);
  Elevation->add_att("_FillValue", -32768.f);
  {
    float validRange[2] = {-360.0f, 360.0f};
    Elevation->add_att("valid_range", 2, validRange);
  }
  {
    long edge = _nAz;
    Elevation->put(elevation, &edge);
  }

  // clip_range
  
  Nc3Var *clip_range = out.add_var("clip_range", nc3Float, TimeDim);
  clip_range->add_att("long_name",
		      "Range of last usefull cell");
  clip_range->add_att("units", "meters");
  clip_range->add_att("missing_value", -32768.f);
  clip_range->add_att("_FillValue", -32768.f);
  {
    float cr[_nAz];
    float crange = (_startRange + _gateSpacing * (_nGates - 1)) * 1000.0;
    for (int jj = 0; jj < _nAz; jj++)
      cr[jj] = crange;
    long edge = _nAz;
    clip_range->put(cr, &edge);
  }

  // fields
  
  _putField(out, "h_avg_i", "h_avg_i", "none",
	    h_avg_i, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "h_avg_q", "h_avg_q", "none",
	    h_avg_q, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "h_dbz", "h_dbz", "dBZ",
	    h_dbz, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "h_vel", "h_vel", "m/s",
	    h_vel, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "h_ncp", "h_ncp", "none",
	    h_ncp, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "h_sw", "h_sw", "m/s",
	    h_sw, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "h_niq", "h_niq", "none",
	    h_niq, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "h_aiq", "h_aiq", "deg",
	    h_aiq, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "h_dm", "h_dm", "dm",
	    h_dm, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "v_avg_i", "v_avg_i", "none",
	    v_avg_i, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "v_avg_q", "v_avg_q", "none",
	    v_avg_q, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "DBZ", "v_dbz", "dBZ",
	    v_dbz, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "VE", "v_vel", "m/s",
	    v_vel, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "ncp", "v_ncp", "none",
	    v_ncp, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "sw", "v_sw", "m/s",
	    v_sw, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "NIQ", "v_niq", "none",
	    v_niq, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "AIQ", "v_aiq", "deg",
	    v_aiq, TimeDim, maxCellsDim, -32768.f);
  _putField(out, "DM", "v_dm", "dm",
	    v_dm, TimeDim, maxCellsDim, -32768.f);
  
  return true;
}
