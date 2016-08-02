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
// SoundingText.cc
//
// SoundingText object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2000
//
///////////////////////////////////////////////////////////////

#include <cerrno>
#include <cstdio>
#include <iomanip>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/TaFile.hh>
#include <toolsa/DateTime.hh>
#include <rapmath/math_macros.h>
#include <physics/thermo.h>
#include <physics/IcaoStdAtmos.hh>
#include <Mdv/MdvxField.hh>
#include "SoundingText.hh"
using namespace std;

const double SoundingText::_missing = -99.9;

// Constructor

SoundingText::SoundingText(int argc, char **argv)

{

  isOK = true;
  _coords = NULL;
  _log = NULL;
  
  // set programe name

  _progName = "SoundingText";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // attach the shmem for coords

  while ((_coords = (coord_export_t *)
	  ushm_create(_params.coord_key,
		      sizeof(coord_export_t), 0666)) == NULL) {
    cerr << "WARNING - SoundingText::SoundingText" << endl;
    cerr << "  Unable to attach to display" << endl;
    cerr << "  Key: " << _params.coord_key << endl;
    PMU_auto_register("Trying to attach shmem");
    sleep (5);
  }

  // set up msg log

  if (_params.log_data_errors) {
    _log = new MsgLog;
    _log->setApplication(_progName, _params.instance);
    _log->setSuffix("log");
    _log->setAppendMode();
    _log->setDayMode();
    _log->setOutputDir(_params.errors_log_dir);
  }

  return;

}

// destructor

SoundingText::~SoundingText()

{

  // unregister process

  PMU_auto_unregister();

  // Free resources

    if (_log) {
      delete _log;
    }

    if (_coords) {
      ushm_detach((void*) _coords);
      if (ushm_nattach(_params.coord_key) <= 0) {
	ushm_remove(_params.coord_key);
      }
    }

}

//////////////////////////////////////////////////
// Run

int SoundingText::Run ()
{

  int last_no = 0;
  time_t last_clicked = 0;

  // if required, set up station loc object

  StationLoc stationLoc;
  if (_params.find_closest_waypoint) {
    if (stationLoc.ReadData(_params.waypoint_data_file)) {
      if (_log) {
	_log->postMsg("ERROR - SoundingText::Run");
	_log->postMsg("  Cannot read in waypoint file: %s",
		      _params.waypoint_data_file);
      } else {
	cerr << "ERROR - SoundingText::Run" << endl;
	cerr << "  Cannot read in waypoint file: "
	     << _params.waypoint_data_file << endl;
      }
      return -1;
    }
  }
  
  while (true) {
    
    // register with procmap
    
    PMU_auto_register("Checking for user click");

    // check for user clicks
    
    bool doSample = false;
    time_t now = time(NULL);
    double lat = 0.0, lon = 0.0;
    time_t sample_time = 0;
    time_t click_time = 0;

    if(_coords->pointer_seq_num != last_no) {
      last_no = _coords->pointer_seq_num;
      lat = _coords->pointer_lat;
      lon = _coords->pointer_lon;
      sample_time = _coords->time_cent;
      click_time = now;
      doSample = true;
    } else {
      if ((now - last_clicked) > _params.default_interval) {
	doSample = true;
	lat = _params.default_point.lat;
	lon = _params.default_point.lon;
	sample_time = now;
	click_time = now;
      }
    }

    if (doSample) {
      DsMdvx mdvx;
      if (_gatherData(sample_time, lat, lon, mdvx, stationLoc) == 0) {
	if (_writeToStdout(sample_time, lat, lon, mdvx, stationLoc)) {
	  if (_params.debug) {
	    cerr << "WARNING - SoundingText::Run" << endl;
	    cerr << "  _writeToStdout() failed." << endl;
	  }
	}
	if (_params.write_class_file) {
	  if (_writeClassFile(sample_time, lat, lon, mdvx, stationLoc)) {
	    if (_params.debug) {
	      cerr << "WARNING - SoundingText::Run" << endl;
	      cerr << "  _writeClassFile() failed." << endl;
	    }
	  } else {
	    if (_params.run_class_file_script){
	      sleep(1);
	      system(_params.class_file_script);
	    }
	  }
	}
      } else {
	if (_params.debug) {
	  cerr << "WARNING - SoundingText::Run" << endl;
	  cerr << "  _gatherData() failed." << endl;
	}
      }
      last_clicked = click_time;
    }

    sleep(1);
    
  }

  return 0;

}

//////////////////////////////////////////////////
// _gatherData

int SoundingText::_gatherData (time_t sample_time,
			       double lat, double lon,
			       DsMdvx &mdvx,
			       const StationLoc &stationLoc)
{
  
  mdvx.clearRead();

  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _params.input_url,
		   _params.read_search_margin,
		   sample_time);

  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  mdvx.addReadWayPt(lat, lon);

  if (strcmp(_params.u_field_name, "none")) {
    mdvx.addReadField(_params.u_field_name);
  }
  if (strcmp(_params.v_field_name, "none")) {
    mdvx.addReadField(_params.v_field_name);
  }
  if (strcmp(_params.temp_field_name, "none")) {
    mdvx.addReadField(_params.temp_field_name);
  }
  if (strcmp(_params.rh_field_name, "none")) {
    mdvx.addReadField(_params.rh_field_name);
  }

  if (_coords && _coords->checkWriteTimeOnRead) {
    mdvx.setCheckLatestValidModTime(_coords->latestValidWriteTime);
  } else {
    mdvx.clearCheckLatestValidModTime();
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

  if (mdvx.readVsection()) {
    if (_log) {
      _log->postMsg("ERROR - SoundingText::_gatherData");
      _log->postMsg("  Reading mdv data.");
      _log->postMsg("%s", mdvx.getErrStr().c_str());
    } else {
      cerr << "ERROR - SoundingText::_gatherData" << endl;
      cerr << "  Reading mdv data." << endl;
      cerr << mdvx.getErrStr() << endl;
    }
    return -1;
  }

  if (_loadPrintInfo(lat, lon, mdvx, stationLoc)) {
    return -1;
  }

  return 0;

}

/////////////////////////////////////////
// load up the info required for printing

int SoundingText::_loadPrintInfo(double lat, double lon,
				 const DsMdvx &mdvx,
				 const StationLoc &stationLoc)

{
  
  // determine which fields to print
  // set pointers to fields in Mdvx
  // determine nz and vlevels

  _printWind = false;
  _printTemp = false;
  _printRh = false;
  _printDp = false;
  _dataToPrint = false;

  if (strcmp(_params.u_field_name, "none") &&
      strcmp(_params.v_field_name, "none")) {
    _printWind = true;
    _uField = mdvx.getFieldByName(_params.u_field_name);
    _vField = mdvx.getFieldByName(_params.v_field_name);
    if (_uField == NULL || _vField == NULL) {
      if (_log) {
	_log->postMsg("ERROR - SoundingText::_setPrintInfo");
	_log->postMsg("  Wind data not returned.");
      } else {
	cerr << "ERROR - SoundingText::_setPrintInfo" << endl;
	cerr << "  Wind data not returned." << endl;
      }
      return -1;
    }
    _nz = _uField->getFieldHeader().nz;
    _vhdr = _uField->getVlevelHeader();
    _dataToPrint = true;
  }
  if (strcmp(_params.temp_field_name, "none")) {
    _printTemp = true;
    _tempField = mdvx.getFieldByName(_params.temp_field_name);
    if (_tempField == NULL) {
      if (_log) {
	_log->postMsg("ERROR - SoundingText::_setPrintInfo");
	_log->postMsg("  Temp data not returned.");
      } else {
	cerr << "ERROR - SoundingText::_setPrintInfo" << endl;
	cerr << "  Temp data not returned." << endl;
      }
      return -1;
    }
    _nz = MIN(_nz, _tempField->getFieldHeader().nz);
    if (!_dataToPrint) {
      _vhdr = _tempField->getVlevelHeader();
    }
    _dataToPrint = true;
  }
  if (strcmp(_params.rh_field_name, "none")) {
    _printRh = true;
    _rhField = mdvx.getFieldByName(_params.rh_field_name);
    if (_rhField == NULL) {
      if (_log) {
	_log->postMsg("ERROR - SoundingText::_setPrintInfo");
	_log->postMsg("  RH data not returned.");
      } else {
	cerr << "ERROR - SoundingText::_setPrintInfo" << endl;
	cerr << "  RH data not returned." << endl;
      }
      return -1;
    }
    if (_printTemp && _params.print_dewpoint) {
      _printDp = true;
    }
    _nz = MIN(_nz, _rhField->getFieldHeader().nz);
    if (!_dataToPrint) {
      _vhdr = _rhField->getVlevelHeader();
    }
    _dataToPrint = true;
  }

  // time and location
  
  _dataTime = mdvx.getMasterHeader().time_centroid;
  if (_params.find_closest_waypoint) {
    _locName = stationLoc.FindClosest(lat, lon,
				      _params.max_dist_from_waypoint);
  }

  return 0;

}

//////////////////////////////////////////////////
// _writeToStdout

int SoundingText::_writeToStdout (time_t sample_time,
				  double lat, double lon,
				  const DsMdvx &mdvx,
				  const StationLoc &stationLoc)

{

  if (_params.n_blank_lines_between_pages > 0) {
    for (int ii = 0; ii < _params.n_blank_lines_between_pages; ii++) {
      cout << endl;
    }
  }

  // time and location
  
  cout << endl;
  if (_params.debug) {
    cout << "Request time: " << utimstr(sample_time) << " UTC" << endl;
  }
  cout << "Data time: " << utimstr(_dataTime) << " UTC" << endl;
  cout << "(Lat, Lon) : (" << lat << ", " << lon << ")" << endl;
  if (_params.find_closest_waypoint) {
    if (_locName.size() > 0) {
      cout << "Closest nav point : " << _locName << endl;
    } else {
      cout << "No nav point within " << _params.max_dist_from_waypoint
	   << " km" << endl;
    }
  } else {
    cout << endl;
  }
  cout << endl;

  if (!_dataToPrint) {
    cout << "********* DATA NOT AVAILABLE **********" << endl;
    return 0;
  }

  // Labels

  cout << "Height";
  if (_printWind) {
    cout << "  Wdirn  Wspd";
  }
  if (_printTemp) {
    cout << "  Temp";
  }
  if (_printDp) {
    cout << "    DP";
  }
  if (_printRh) {
    cout << "    RH";
  }
  cout << endl;

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();

  if (mhdr.vlevel_type == Mdvx::VERT_TYPE_PRESSURE){
    cout << "    mb";
  } else {
    cout << "    FL";
  }

  if (_printWind) {
    cout << "   degT  Kts";
  }
  if (_printTemp) {
    cout << "     C";
  }
  if (_printDp) {
    cout << "     C";
  }
  if (_printRh) {
    cout << "     %";
  }
  cout << endl;
  
  cout << "======";
  if (_printWind) {
    cout << "  =====  ====";
  }
  if (_printTemp) {
    cout << "  ====";
  }
  if (_printDp) {
    cout << "    ==";
  }
  if (_printRh) {
    cout << "    ==";
  }
  cout << endl;
  cout << endl;

  // print data for each level

  for (int j = 0; j < _nz; j++) {

    int i;
    if (_params.lowest_first) {
      i = j;
    } else {
      i = _nz - 1 - j;
    }
    
    cout << setw(6) << _vhdr.level[i];

    // wind

    if (_printWind) {
      bool missing = false;
      const Mdvx::field_header_t &ufhdr = _uField->getFieldHeader();
      fl32 u = ((fl32 *) _uField->getVol())[i];
      if (u == ufhdr.bad_data_value ||
	  u == ufhdr.missing_data_value) {
	missing = true;
      }
      const Mdvx::field_header_t &vfhdr = _vField->getFieldHeader();
      fl32 v = ((fl32 *) _vField->getVol())[i];
      if (v == vfhdr.bad_data_value ||
	  v == vfhdr.missing_data_value) {
	missing = true;
      }
      if (missing) {
	cout << "   ****  ****";
      } else {
	double speed = sqrt(u  * u + v * v);
	double dirn;
	if (u == 0.0 && v == 0.0) {
	  dirn = 0.0;
	} else {
	  dirn = atan2(u, v) * RAD_TO_DEG;
	  dirn += 180.0;
	  // if(dirn < 0) dirn += 360;
	}
	int idirn;
	if (_params.round_wind_dirn) {
	  idirn = (int)rint(dirn/10.0)*10;
	  if (idirn == 0) {
	    idirn = 360;
	  }
	} else {
	  idirn = (int) floor (dirn + 0.5);
	}
	cout << setw(7) << idirn
	     << setw(6) << (int) (speed + 0.5);
      }
    }

    // temperature

    fl32 temp = 0.0;
    bool tempMissing = false;
    if (_printTemp) {
      const Mdvx::field_header_t &tempfhdr = _tempField->getFieldHeader();
      temp = ((fl32 *) _tempField->getVol())[i];
      if (temp == tempfhdr.bad_data_value ||
	  temp == tempfhdr.missing_data_value) {
	tempMissing = true;
      }
      if (
	  (!(tempMissing)) &&
	  (_params.model_temp_in_kelvin)
	  ){
	temp = temp - 273.1;
      }
      if (tempMissing) {
	cout << "  ****";
      } else {
	cout << setw(6) << (int) floor(temp + 0.5);
      }
    }

    // RH and DP

    bool rhmissing = false;
    const Mdvx::field_header_t &rhfhdr = _rhField->getFieldHeader();
    fl32 rh = ((fl32 *) _rhField->getVol())[i];
    if (rh == rhfhdr.bad_data_value ||
	rh == rhfhdr.missing_data_value) {
      rhmissing = true;
    }

    if (_printDp) {
      if (tempMissing || rhmissing) {
	cout << "  ****";
      } else {
	double dp = PHYrhdp(temp, rh);
	cout << setw(6) << (int) floor(dp + 0.5);
      }
    }

    if (_printRh) {
      if (rhmissing) {
	cout << "  ****";
      } else {
	cout << setw(6) << (int) floor(rh + 0.5);
      }
    }

    cout << endl;
  
  } // i

  return 0;

}

//////////////////////////////////////////////////
// _writeClassFile

int SoundingText::_writeClassFile (time_t sample_time,
				   double lat, double lon,
				   const DsMdvx &mdvx,
				   const StationLoc &stationLoc)

{
  
  // first check we have non-missing temperature data

  bool tempFound = false;
  for (int i = 0; i < _nz; i++) {
    const Mdvx::field_header_t &tempfhdr = _tempField->getFieldHeader();
    fl32 temp = ((fl32 *) _tempField->getVol())[i];
    if (temp != tempfhdr.bad_data_value &&
        temp != tempfhdr.missing_data_value) {
      tempFound = true;
      break;
    }
  }
  if (!tempFound) {
    return 0;
  }

  // open tmp output file

  char tmpClassPath[MAX_PATH_LEN];

  sprintf(tmpClassPath, "%s.tmp", _params.class_file_path);
  
  TaFile classFile;
  FILE *out;
  if ((out = classFile.fopen(tmpClassPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - SoundingText::_writeClassFile" << endl;
    cerr << "  Cannot open class output file for writing" << endl;
    cerr << "  " << tmpClassPath << ": "
	 << strerror(errNum) << endl;
    return -1;
  }

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();

  // headers

  fprintf(out, "Data Type:                         %s\n",
	  mhdr.data_set_name);
  fprintf(out, "Project ID:                        %s\n",
	  mhdr.data_set_source);
  if (_locName.size() > 0) {
    fprintf(out, "Launch Site Type/Site ID:          %s\n", _locName.c_str());
  } else {
    fprintf(out, "Launch Site Type/Site ID:          %s\n", "NA");
  }
  fprintf(out, "Launch Location (lon,lat,alt):     %.3f,%.3f,%g\n",
	  lon, lat, _missing);
  
  DateTime dtime(_dataTime);
  fprintf(out, "GMT Launch Time (y,m,d,h,m,s):     "
	  "%.2d, %.2d, %.2d, %.2d:%.2d:%.2d\n",
	  dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	  dtime.getHour(), dtime.getMin(), dtime.getSec());
  
  fprintf(out, "Sonde Type/ID/Sensor ID/Tx Freq:   %s\n", "NA");
  fprintf(out, "Met Processor/Met Smoothing:       %s\n", "NA");
  fprintf(out, "Winds Type/Processor/Smoothing:    %s\n", "NA");
  fprintf(out, "Pre-launch Met Obs Source:         %s\n", "NA");
  fprintf(out, "System Operator/Comments:          %s\n", "NA");
  fprintf(out,
	  " Time  Press  Temp  Dewpt  RH    Uwind  Vwind  Wspd  Dir   "
	  "dZ      Lon     Lat    Rng   Ang    Alt    "
	  "Qp    Qt    Qh    Qu    Qv    Quv \n");
  fprintf(out,
	  "  sec    mb     C     C     %%     m/s    m/s   m/s   deg   "
	  "m/s     deg     deg     km   deg     m     "
	  "mb    C     %%     m/s   m/s   m/s \n");
  fprintf(out,
	  "------ ------ ----- ----- ----- ------ ------ ----- ----- "
	  "----- -------- ------- ----- ----- ------- "
	  "----- ----- ----- ----- ----- -----\n");

  if (!_dataToPrint) {
    return 0;
  }

  // print data for each level

  IcaoStdAtmos stdAtmos;

  for (int i = 0; i < _nz; i++) {

    double ttime = _missing;

    // pressure

    double pressure;
    if (_vhdr.type[i] == Mdvx::VERT_TYPE_PRESSURE) {
      pressure = _vhdr.level[i];
    } else if (_vhdr.type[i] == Mdvx::VERT_FLIGHT_LEVEL) {
      pressure = stdAtmos.flevel2pres(_vhdr.level[i]);
    } else {
      pressure = _missing;
    }

    // temperature

    double temp;
    if (_printTemp) {
      fl32 _temp;
      const Mdvx::field_header_t &tempfhdr = _tempField->getFieldHeader();
      _temp = ((fl32 *) _tempField->getVol())[i];
      if (_temp == tempfhdr.bad_data_value ||
	  _temp == tempfhdr.missing_data_value) {
	temp = _missing;
      } else {
	temp = _temp;
	if (_params.model_temp_in_kelvin) temp = temp - 273.1;
      }
    } else {
      temp = _missing;
    }

    // RH and DP

    double rh;
    {
      const Mdvx::field_header_t &rhfhdr = _rhField->getFieldHeader();
      fl32 _rh = ((fl32 *) _rhField->getVol())[i];
      if (_rh == rhfhdr.bad_data_value ||
	  _rh == rhfhdr.missing_data_value) {
	rh = _missing;
      } else {
	rh = _rh;
      }
    }

    double dp;
    if (temp == _missing || rh == _missing) {
      dp = _missing;
    } else {
      dp = PHYrhdp(temp, rh);
    }

    // wind

    double speed_factor = 1.0;
    if (_params.mdv_wind_speed_kts) {
      speed_factor = KNOTS_TO_MS;
    }

    double uu, vv;
    if (_printWind) {
      const Mdvx::field_header_t &ufhdr = _uField->getFieldHeader();
      fl32 _u = ((fl32 *) _uField->getVol())[i];
      if (_u == ufhdr.bad_data_value ||
	  _u == ufhdr.missing_data_value) {
	uu = _missing;
      } else {
	uu = _u * speed_factor;
      }
      const Mdvx::field_header_t &vfhdr = _vField->getFieldHeader();
      fl32 _v = ((fl32 *) _vField->getVol())[i];
      if (_v == vfhdr.bad_data_value ||
	  _v == vfhdr.missing_data_value) {
	vv = _missing;
      } else {
	vv = _v * speed_factor;
      }
    } else {
      uu = _missing;
      vv = _missing;
    }

    double wspd, wdirn;
    if (uu == _missing || vv == _missing) {
      wspd = _missing;
      wdirn = _missing;
    } else {
      wspd = sqrt(uu  * uu + vv * vv);
      if (uu == 0.0 && vv == 0.0) {
	wdirn = 0.0;
      } else {
	wdirn = atan2(uu, vv) * RAD_TO_DEG;
	wdirn += 180.0;
      }
    }

    double dz = _missing;
    double rng = _missing;
    double ang = _missing;

    double alt = _missing;
    if (_vhdr.type[i] == Mdvx::VERT_FLIGHT_LEVEL) {
      alt = (_vhdr.level[i] * 100) * 0.3048;
    }

    double qp = _missing;
    double qt = _missing;
    double qh = _missing;
    double qu = _missing;
    double qv = _missing;
    double quv = _missing;

    // check if we have something to print

    bool doPrint = false;
    if (temp != _missing || dp != _missing || rh != _missing ||
	uu != _missing || vv != _missing ||
	wspd != _missing || wdirn != _missing) {
      doPrint = true;
    }

    if (doPrint) {

      fprintf(out, "%6.1f%7.1f%6.1f%6.1f%6.1f", ttime, pressure, temp, dp, rh);
      fprintf(out, "%7.1f%7.1f%6.1f%6.1f", uu, vv, wspd, wdirn);
      fprintf(out, "%6.1f%9.3f%8.3f", dz, lon, lat);
      fprintf(out, "%6.1f%6.1f", rng, ang);
      fprintf(out, "%8.1f", alt);
      fprintf(out, "%6.1f%6.1f%6.1f%6.1f%6.1f%6.1f", qp, qt, qh, qu, qv, quv);
      fprintf(out, "\n");

    }
  
  } // i

  // close file

  classFile.fclose();

  // rename file

  if (rename(tmpClassPath, _params.class_file_path)) {
    fprintf(stderr, "ERROR - SoundingText::_writeClassFile\n");
    fprintf(stderr, "  Cannot rename file: %s\n", tmpClassPath);
    fprintf(stderr, "                  to: %s\n", _params.class_file_path);
    perror(_params.class_file_path);
    return -1;
  }

  // make file world writable so that it can be overwritten no matter
  // who wrote it

  string cmd = "chmod 666 ";
  cmd += _params.class_file_path;
  if (_params.debug) {
    cerr << "Setting class file writeable by all" << endl;
    cerr << cmd << endl;
  }
  system(cmd.c_str());

  return 0;

}

