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
///////////////////////////////////////////////////////////////
// SweepFile.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2003
//
///////////////////////////////////////////////////////////////
//
// SweepFile creates a single PPI data set in an Nc3File object and
// writes it.
//
////////////////////////////////////////////////////////////////

#include "SweepFile.hh"
#include <Mdv/MdvxField.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/os_config.h>
#include <cstdio>
#include <string.h>
#include <cerrno>
#include <math.h>
#include <dataport/bigend.h>
using namespace std;

#define SHORT_STRING_LEN 32
#define LONG_STRING_LEN 80
#define MISSING_FLOAT -9999.0

// Constructor

SweepFile::SweepFile(const Params &params,
		     const DsMdvx &mdvx,
		     int n_tilts,
		     int tilt_num,
		     double elev,
		     int vol_num,
		     int n_az,
		     int n_gates,
		     const DsRadarParams &radar_params) :
  _params(params),
  _ncfIn("null", Nc3File::ReadOnly),
  _mdvx(mdvx),
  _radarParams(radar_params),
  _nTilts(n_tilts),
  _tiltNum(tilt_num),
  _elev(elev),
  _volNum(vol_num),
  _nAz(n_az),
  _nGates(n_gates),
  _nFields(mdvx.getNFields())

{

  if (_params.debug) {
    cerr << "SweepFile" << endl;
    cerr << "  tiltNum: " << _tiltNum << endl;
    cerr << "  volNum: " << _volNum << endl;
  }

  // compute times

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  int volDuration = mhdr.time_end - mhdr.time_begin;
  _sweepDuration = volDuration / _nTilts;
  _volStartTime = mhdr.time_begin;
  _sweepStartTime = mhdr.time_begin + _tiltNum * _sweepDuration;
  _beamDuration = (double) _sweepDuration / (double) _nAz;

  if (_params.debug) {
    cerr << "  start time: " << DateTime(mhdr.time_begin) << endl;
    cerr << "  end time: " << DateTime::str(mhdr.time_end) << endl;
    cerr << "  _nTilts: " << _nTilts << endl;
    cerr << "  volDuration: " << volDuration << endl;
    cerr << "  _sweepDuration: " << _sweepDuration << endl;
    cerr << "  _sweepStartTime: " << DateTime::str(_sweepStartTime) << endl;
    cerr << "  _beamDuration: " << _beamDuration << endl;
  }
  
  // compute file path
  
  DateTime stime(_sweepStartTime);
  char path[MAX_PATH_LEN];
  sprintf(path,
	  "%s%sncswp_%s_%.4d%.2d%.2d_%.2d%.2d%.2d.000_v%.3d_s%.2d_%.1f_SUR_.nc",
	  _params.output_dir, PATH_DELIM,
	  _params.radar_name,
	  stime.getYear(), stime.getMonth(), stime.getDay(),
	  stime.getHour(), stime.getMin(), stime.getSec(),
	  _volNum, _tiltNum, elev);
  
  _outPath = path;
  _tmpPath = _outPath + ".tmp";
  
}

// destructor

SweepFile::~SweepFile()

{
  
}

////////////////////////////////////////
// Write ppi file
//
// Returns 0 on success, -1 on failure

int SweepFile::write()

{

  // make output dir in case necessary

  if (ta_makedir_recurse(_params.output_dir)) {
    cerr << "ERROR - SweepFile::write" << endl;
    cerr << "  Cannot make output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  // write the tmp file
  
  if (_writeTmp()) {
    cerr << "ERROR - SweepFile::write" << endl;
    cerr << "  Cannot write tmp file: " << _tmpPath << endl;
    unlink(_tmpPath.c_str());
    return -1;
  }
  
  // move the file to the output path
  
  if (_params.debug) {
    cerr << "  Renaming tmp file to: " << _outPath << endl;
  }

  if (rename(_tmpPath.c_str(), _outPath.c_str())) {
    cerr << "ERROR - SweepFile::write" << endl;
    cerr << "  Cannot rename tmp file: " << _tmpPath << endl;
    cerr << "                to  file: " << _outPath << endl;
    cerr << "  " << strerror(errno) << endl;
    return -1;
  }

  // write latest data info file if required

  if (_params.write_ldata_info_file) {
    
    Path outP(_outPath);
    
    LdataInfo ldata(outP.getDirectory().c_str());
    ldata.setDataFileExt(outP.getExt().c_str());
    ldata.setWriter("Mdv2SweepNetCDF");
    ldata.setRelDataPath(outP.getFile().c_str());

    if (ldata.write(_sweepStartTime)) {
      cerr << "ERROR - SweepFile::write" << endl;
      cerr << "  Cannot write ldata file to dir: "
	   << outP.getDirectory() << endl;
      return -1;
    }
    
  }
  
  return 0;
  
}


////////////////////////////////////////
// Write output to tmp file
//
// Returns 0 on success, -1 on failure

int SweepFile::_writeTmp()

{

  // create Nc3File object

  Nc3Error err(Nc3Error::verbose_nonfatal);

  Nc3File out(_tmpPath.c_str(), Nc3File::Replace);
  if (!out.is_valid()) {
    cerr << "ERROR - SweepFile::write" << endl;
    cerr << "  Cannot create file: " << _tmpPath << endl;
    return -1;
  }
  int iret = 0;

  if (_params.debug) {
    cerr << "  Writing tmp file: " << _tmpPath << endl;
  }

  /////////////////////
  // global attributes
  
  char cbuf[1024];

  out.add_att("Content", "Radar sweep file for one PPI from MDV volume");
  out.add_att("Conventions", "NCAR_ATD-NOAA_ETL/Scanning_Remote_Sensor");
  out.add_att("Instrument_Name", _params.radar_name);
  out.add_att("Instrument_Type", "GROUND");
  out.add_att("Scan_Mode", "SUR");

  DateTime vtime(_volStartTime);
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
  out.add_att("Num_Samples", _radarParams.samplesPerBeam);
  out.add_att("Index_of_horizontal_information", 0);
  out.add_att("Index_of_vertical_information", 1);
  out.add_att("Project_Name", _params.project_name);

  DateTime now(time(NULL));
  sprintf(cbuf, "%.4d/%.2d/%.2d %.2d:%.2d:%2d",
	  now.getYear(), now.getMonth(), now.getDay(),
	  now.getHour(), now.getMin(), now.getSec());
  out.add_att("Production_Date", cbuf);

  out.add_att("Producer_Name", "NSF/UCAR/NCAR/RAP");
  out.add_att("Software", "Mdv2SweepNetCDF");
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
  int s_time = _volStartTime;
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
    MEM_zero(fnames);
    for (int i = 0; i < _nFields; i++) {
      MdvxField *fld = _mdvx.getField(i);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      string fname(fhdr.field_name);
      for(size_t ii = 0; ii < fname.length(); ii++) {
	if (fname[ii] == ' ') {
	  fname[ii] = '_';
	}
      }
      STRncopy(fnames + i * SHORT_STRING_LEN, fname.c_str(),
	       SHORT_STRING_LEN);
    }
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
  Fixed_Angle->add_att("_FillValue", (float) -9999.0);
  Fixed_Angle->add_att("missing_value", (float) -9999.0);
  float f_angle = _elev;
  Fixed_Angle->put(&f_angle);

  // Range_to_First_Cell

  Nc3Var *Range_to_First_Cell = out.add_var("Range_to_First_Cell", nc3Float);
  Range_to_First_Cell->add_att("long_name",
			       "Range to the center of the first cell");
  Range_to_First_Cell->add_att("units", "meters");
  Range_to_First_Cell->add_att("_FillValue", (float) -9999.0);
  Range_to_First_Cell->add_att("missing_value", (float) -9999.0);
  float s_range = _radarParams.startRange * 1000.0;
  Range_to_First_Cell->put(&s_range);

  // Cell_Spacing

  Nc3Var *Cell_Spacing = out.add_var("Cell_Spacing", nc3Float);
  Cell_Spacing->add_att("long_name",
			"Distance between cells");
  Cell_Spacing->add_att("units", "meters");
  Cell_Spacing->add_att("_FillValue", (float) -9999.0);
  Cell_Spacing->add_att("missing_value", (float) -9999.0);
  float g_spacing = _radarParams.gateSpacing * 1000.0;
  Cell_Spacing->put(&g_spacing);

  // Nyquist_Velocity
  
  Nc3Var *Nyquist_Velocity = out.add_var("Nyquist_Velocity", nc3Float);
  Nyquist_Velocity->add_att("long_name",
			    "Effective unambiguous velocity");
  Nyquist_Velocity->add_att("units", "meters/second");
  Nyquist_Velocity->add_att("_FillValue", (float) -9999.0);
  Nyquist_Velocity->add_att("missing_value", (float) -9999.0);
  float nyquist = _radarParams.unambigVelocity;
  Nyquist_Velocity->put(&nyquist);

  // Unambiguous_Range

  Nc3Var *Unambiguous_Range = out.add_var("Unambiguous_Range", nc3Float);
  Unambiguous_Range->add_att("long_name",
			     "Effective unambiguous range");
  Unambiguous_Range->add_att("units", "meters");
  Unambiguous_Range->add_att("_FillValue", (float) -9999.0);
  Unambiguous_Range->add_att("missing_value", (float) -9999.0);
  float u_range = _radarParams.unambigRange * 1000.0;
  Unambiguous_Range->put(&u_range);

  // Latitude

  Nc3Var *Latitude = out.add_var("Latitude", nc3Double);
  Latitude->add_att("long_name",
		    "Latitude of the instrument");
  Latitude->add_att("units", "degrees");
  Latitude->add_att("_FillValue", -9999.0);
  Latitude->add_att("missing_value", -9999.0);
  {
    float validRange[2] = {-90.0, 90.0};
    Latitude->add_att("valid_range", 2, validRange);
  }
  double lat = _radarParams.latitude;
  Latitude->put(&lat);

  // Longitude

  Nc3Var *Longitude = out.add_var("Longitude", nc3Double);
  Longitude->add_att("long_name",
		    "Longitude of the instrument");
  Longitude->add_att("units", "degrees");
  Longitude->add_att("_FillValue", -9999.0);
  Longitude->add_att("missing_value", -9999.0);
  {
    float validRange[2] = {-360.0, 360.0};
    Longitude->add_att("valid_range", 2, validRange);
  }
  double lon = _radarParams.longitude;
  Longitude->put(&lon);

  // Altitude

  Nc3Var *Altitude = out.add_var("Altitude", nc3Double);
  Altitude->add_att("long_name",
		    "Altitude in meters (asl) of the instrument");
  Altitude->add_att("units", "meters");
  Altitude->add_att("_FillValue", -9999.0);
  Altitude->add_att("missing_value", -9999.0);
  {
    float validRange[2] = {-10000.0, 90000.0};
    Altitude->add_att("valid_range", 2, validRange);
  }
  double alt = _radarParams.altitude * 1000.0;
  Altitude->put(&alt);

  // Radar_Constant

  Nc3Var *Radar_Constant =
    out.add_var("Radar_Constant", nc3Float, numSystemsDim);
  Radar_Constant->add_att("long_name", "Radar constant");
  Radar_Constant->add_att("units", "mm6/(m3.mW.km-2)");
  Radar_Constant->add_att("_FillValue", (float) -9999.0);
  Radar_Constant->add_att("missing_value", (float) -9999.0);
  {
    float fval = _radarParams.radarConstant;
    long edge = 1;
    Radar_Constant->put(&fval, &edge);
  }

  // rcvr_gain

  Nc3Var *rcvr_gain = out.add_var("rcvr_gain", nc3Float, numSystemsDim);
  rcvr_gain->add_att("long_name", "Receiver Gain");
  rcvr_gain->add_att("units", "dB");
  rcvr_gain->add_att("_FillValue", (float) -9999.0);
  rcvr_gain->add_att("missing_value", (float) -9999.0);
  {
    float fval = _radarParams.receiverGain;
    long edge = 1;
    rcvr_gain->put(&fval, &edge);
  }

  // ant_gain

  Nc3Var *ant_gain = out.add_var("ant_gain", nc3Float, numSystemsDim);
  ant_gain->add_att("long_name", "Antenna Gain");
  ant_gain->add_att("units", "dB");
  ant_gain->add_att("_FillValue", (float) -9999.0);
  ant_gain->add_att("missing_value", (float) -9999.0);
  {
    float fval = _radarParams.antennaGain;
    long edge = 1;
    ant_gain->put(&fval, &edge);
  }

  // sys_gain

  Nc3Var *sys_gain = out.add_var("sys_gain", nc3Float, numSystemsDim);
  sys_gain->add_att("long_name", "System Gain");
  sys_gain->add_att("units", "dB");
  sys_gain->add_att("_FillValue", (float) -9999.0);
  sys_gain->add_att("missing_value", (float) -9999.0);
  {
    float fval = _radarParams.systemGain;
    long edge = 1;
    sys_gain->put(&fval, &edge);
  }

  // bm_width

  Nc3Var *bm_width = out.add_var("bm_width", nc3Float, numSystemsDim);
  bm_width->add_att("long_name", "Beam Width");
  bm_width->add_att("units", "degrees");
  bm_width->add_att("_FillValue", (float) -9999.0);
  bm_width->add_att("missing_value", (float) -9999.0);
  {
    float fval =
      (_radarParams.horizBeamWidth + _radarParams.vertBeamWidth) / 2.0;
    long edge = 1;
    bm_width->put(&fval, &edge);
  }

  // pulse_width
  
  Nc3Var *pulse_width = out.add_var("pulse_width", nc3Float, numSystemsDim);
  pulse_width->add_att("long_name", "Pulse Width");
  pulse_width->add_att("units", "seconds");
  pulse_width->add_att("_FillValue", (float) -9999.0);
  pulse_width->add_att("missing_value", (float) -9999.0);
  {
    float fval = _radarParams.pulseWidth / 1.0e6;
    long edge = 1;
    pulse_width->put(&fval, &edge);
  }

  // band_width

  Nc3Var *band_width = out.add_var("band_width", nc3Float, numSystemsDim);
  band_width->add_att("long_name", "Band Width");
  band_width->add_att("units", "hertz");
  band_width->add_att("_FillValue", (float) -9999.0);
  band_width->add_att("missing_value", (float) -9999.0);

  // peak_pwr

  Nc3Var *peak_pwr = out.add_var("peak_pwr", nc3Float, numSystemsDim);
  peak_pwr->add_att("long_name", "Peak Power");
  peak_pwr->add_att("units", "watts");
  peak_pwr->add_att("_FillValue", (float) -9999.0);
  peak_pwr->add_att("missing_value", (float) -9999.0);
  {
    float fval = _radarParams.xmitPeakPower;
    long edge = 1;
    peak_pwr->put(&fval, &edge);
  }

  // xmtr_power

  Nc3Var *xmtr_power = out.add_var("xmtr_power", nc3Float, numSystemsDim);
  xmtr_power->add_att("long_name", "Transmitter Power");
  xmtr_power->add_att("units", "dBM");
  xmtr_power->add_att("_FillValue", (float) -9999.0);
  xmtr_power->add_att("missing_value", (float) -9999.0);

  // noise_pwr

  Nc3Var *noise_pwr = out.add_var("noise_pwr", nc3Float, numSystemsDim);
  noise_pwr->add_att("long_name", "Noise Power");
  noise_pwr->add_att("units", "dBM");
  noise_pwr->add_att("_FillValue", (float) -9999.0);
  noise_pwr->add_att("missing_value", (float) -9999.0);

  // tst_pls_pwr

  Nc3Var *tst_pls_pwr = out.add_var("tst_pls_pwr", nc3Float, numSystemsDim);
  tst_pls_pwr->add_att("long_name", "Test Pulse Power");
  tst_pls_pwr->add_att("units", "dBM");
  tst_pls_pwr->add_att("_FillValue", (float) -9999.0);
  tst_pls_pwr->add_att("missing_value", (float) -9999.0);

  // tst_pls_rng0

  Nc3Var *tst_pls_rng0 = out.add_var("tst_pls_rng0", nc3Float, numSystemsDim);
  tst_pls_rng0->add_att("long_name", "Range to start of test pulse");
  tst_pls_rng0->add_att("units", "meters");
  tst_pls_rng0->add_att("_FillValue", (float) -9999.0);
  tst_pls_rng0->add_att("missing_value", (float) -9999.0);

  // tst_pls_rng1

  Nc3Var *tst_pls_rng1 = out.add_var("tst_pls_rng1", nc3Float, numSystemsDim);
  tst_pls_rng1->add_att("long_name", "Range to end of test pulse");
  tst_pls_rng1->add_att("units", "meters");
  tst_pls_rng1->add_att("_FillValue", (float) -9999.0);
  tst_pls_rng1->add_att("missing_value", (float) -9999.0);

  // Wavelength

  Nc3Var *Wavelength = out.add_var("Wavelength", nc3Float, numSystemsDim);
  Wavelength->add_att("long_name", "System wavelength");
  Wavelength->add_att("units", "meters");
  Wavelength->add_att("_FillValue", (float) -9999.0);
  Wavelength->add_att("missing_value", (float) -9999.0);
  {
    float fval = _radarParams.wavelength / 100.0;
    long edge = 1;
    Wavelength->put(&fval, &edge);
  }

  // PRF

  Nc3Var *PRF = out.add_var("PRF", nc3Float, numSystemsDim);
  PRF->add_att("long_name", "System pulse repetition frequency");
  PRF->add_att("units", "/sec");
  PRF->add_att("_FillValue", (float) -9999.0);
  PRF->add_att("missing_value", (float) -9999.0);
  {
    float fval = _radarParams.pulseRepFreq;
    long edge = 1;
    PRF->put(&fval, &edge);
  }

  // time_offset

  Nc3Var *time_offset =
    out.add_var("time_offset", nc3Double, TimeDim);
  time_offset->add_att("long_name",
		       "time offset of the current record from base_time");
  time_offset->add_att("units", "seconds");
  time_offset->add_att("_missing", 0.0);
  time_offset->add_att("_FillValue", 0.0);
  {
    double tdata[_nAz];
    double dTime = (double) _sweepDuration / _nAz;
    for (int jj = 0; jj < _nAz; jj++) {
      tdata[jj] = (jj + 1) * dTime;
    }
    long edge = _nAz;
    time_offset->put(tdata, &edge);
  }

  // Azimuth

  Nc3Var *Azimuth = out.add_var("Azimuth", nc3Float, TimeDim);
  Azimuth->add_att("long_name",
		   "Earth relative azimuth of the ray");
  Azimuth->add_att("Comment",
		   "Degrees clockwise from true North");
  Azimuth->add_att("units", "degrees");
  Azimuth->add_att("missing_value", -9999.f);
  Azimuth->add_att("_FillValue", -9999.f);
  {
    float validRange[2] = {-360.0f, 360.0f};
    Azimuth->add_att("valid_range", 2, validRange);
  }
  {
    float az[_nAz];
    for (int jj = 0; jj < _nAz; jj++) {
      az[jj] = jj;
    }
    long edge = _nAz;
    Azimuth->put(az, &edge);
  }

  // Elevation
  
  Nc3Var *Elevation = out.add_var("Elevation", nc3Float, TimeDim);
  Elevation->add_att("long_name",
		   "Earth relative elevation of the ray");
  Elevation->add_att("Comment",
		     "Degrees from earth tangent towards zenith");
  Elevation->add_att("units", "degrees");
  Elevation->add_att("missing_value", -9999.f);
  Elevation->add_att("_FillValue", -9999.f);
  {
    float validRange[2] = {-360.0f, 360.0f};
    Elevation->add_att("valid_range", 2, validRange);
  }
  {
    float el[_nAz];
    for (int jj = 0; jj < _nAz; jj++) {
      el[jj] = _elev;
    }
    long edge = _nAz;
    Elevation->put(el, &edge);
  }

  // clip_range
  
  Nc3Var *clip_range = out.add_var("clip_range", nc3Float, TimeDim);
  clip_range->add_att("long_name",
		      "Range of last usefull cell");
  clip_range->add_att("units", "meters");
  clip_range->add_att("missing_value", -9999.f);
  clip_range->add_att("_FillValue", -9999.f);
  {
    float cr[_nAz];
    float crange = (_radarParams.startRange +
		    _radarParams.gateSpacing * (_nGates - 1)) * 1000.0;
    for (int jj = 0; jj < _nAz; jj++) {
      cr[jj] = crange;
    }
    long edge = _nAz;
    clip_range->put(cr, &edge);
  }

  // fields
  
  for (int ifield = 0; ifield < _nFields; ifield++) {

    MdvxField *fld = _mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    
    Nc3Var *field = NULL;
    if (_params.output_encoding == Params::ENCODING_INT8) {
      field = out.add_var(fhdr.field_name, nc3Byte, TimeDim, maxCellsDim);
      double scale = fhdr.scale;
      double offset = fhdr.bias + 127.0 * fhdr.scale;
      field->add_att("scale_factor", (float) scale);
      field->add_att("add_offset", (float) offset);
    } else if (_params.output_encoding == Params::ENCODING_INT16) {
      field = out.add_var(fhdr.field_name, nc3Short, TimeDim, maxCellsDim);
      double scale = fhdr.scale;
      double offset = fhdr.bias + 32767.0 * fhdr.scale;
      field->add_att("scale_factor", (float) scale);
      field->add_att("add_offset", (float) offset);
    } else if (_params.output_encoding == Params::ENCODING_FLOAT32) {
      field = out.add_var(fhdr.field_name, nc3Float, TimeDim, maxCellsDim);
    }

    field->add_att("variable_type", "data");
    field->add_att("long_name", fhdr.field_name_long);
    field->add_att("units", fhdr.units);
    field->add_att("missing_value", (float) fhdr.missing_data_value);
    field->add_att("_fillValue", (float) fhdr.missing_data_value);
    field->add_att("polarization", "Horizontal");
    field->add_att("Frequencies_GHz", 20.0 / _radarParams.wavelength);
    field->add_att("InterPulsePeriods_secs", 1.0 / _radarParams.pulseRepFreq);
    field->add_att("num_segments", 1);
    field->add_att("cells_in_segment", _nGates);
    field->add_att("meters_to_first_cell",
		   (float) _radarParams.startRange * 1000.0);
    field->add_att("meters_between_cells",
		   (float) _radarParams.gateSpacing * 1000.0);

    // load up data
    
    int nPtsPlane = _nAz * _nGates;
    long edges[2];
    edges[0] = _nAz;
    edges[1] = _nGates;
    fld->setPlanePtrs();

    if (_params.output_encoding == Params::ENCODING_INT8) {
      
      const ncbyte *bdata = (const ncbyte *) fld->getPlane(_tiltNum);
      field->put(bdata, edges);

      ui08 *udata = (ui08 *) fld->getPlane(_tiltNum);
      ncbyte *sdata = new ncbyte[nPtsPlane];
      for (int ii = 0; ii < nPtsPlane; ii++) {
        int ival = (int) udata[ii] - 127;
        sdata[ii] = (ncbyte) ival;
      }
      field->put(sdata, edges);

    } else if (_params.output_encoding == Params::ENCODING_INT16) {
      
      ui16 *udata = (ui16 *) fld->getPlane(_tiltNum);
      short *sdata = new short[nPtsPlane];
      for (int ii = 0; ii < nPtsPlane; ii++) {
        int ival = (int) udata[ii] - 32767;
        sdata[ii] = (short) ival;
      }
      field->put(sdata, edges);
      
    } else if (_params.output_encoding == Params::ENCODING_FLOAT32) {

      const float *fdata = (const float *) fld->getPlane(_tiltNum);
      field->put(fdata, edges);

    }
    
  }

  return iret;

}



