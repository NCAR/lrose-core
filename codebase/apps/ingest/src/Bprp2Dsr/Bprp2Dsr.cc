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
// Bprp2Dsr.cc
//
// Bprp2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////
//
// Bprp2Dsr reads Bethlehem-format radar data and writes it to
// and FMQ in DsRadar format
//
////////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/sockutil.h>
#include <toolsa/toolsa_macros.h>
#include <dataport/bigend.h>
#include <rapformats/DsRadarMsg.hh>
#include <iomanip>
#include "Bprp2Dsr.hh"
using namespace std;

// Constructor

Bprp2Dsr::Bprp2Dsr(int argc, char **argv)

{

  isOK = true;
  _beamCount = 0;
  _volNum = 0;
  _tiltNum = 0;
  _scanType = -1;
  _rdasFd = -1;

  // set programe name

  _progName = "Bprp2Dsr";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the output queue

  if (_rQueue.init(_params.output_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_compression != Params::NO_COMPRESSION,
		   _params.output_n_slots,
		   _params.output_buf_size)) {
    cerr << "WARNING - trying to create new fmq" << endl;
    if (_rQueue.init(_params.output_url,
		     _progName.c_str(),
		     _params.debug >= Params::DEBUG_VERBOSE,
		     DsFmq::CREATE, DsFmq::START,
		     _params.output_compression != Params::NO_COMPRESSION,
		     _params.output_n_slots,
		     _params.output_buf_size)) {
      cerr << "ERROR - Bprp2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_url << endl;
      isOK = false;
      return;
    }
  }
      
  _rQueue.setCompressionMethod((ta_compression_method_t)
			       _params.output_compression);

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  return;

}

// destructor

Bprp2Dsr::~Bprp2Dsr()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Bprp2Dsr::Run()
{
  
  // register with procmap
  
  PMU_auto_register("Run");

  while (true) {
    
    PMU_auto_register("Reading rdas data");
    
    if (_openRdas() == 0) {
      
      // wait on packet, timing out after 10 secs each time
      
      int iret = SKU_read_select(_rdasFd, 10000);

      if (iret == 1) {
	
	// success, read a beam and process
	
	// if (SKU_read(_rdasFd, &_response, sizeof(bprp_response_t), 20) ==
	//   sizeof(bprp_response_t)) {

	if (_readBeam() == 0) {
	  
	  // success
	  
	  // _handleResponse();
	  _handleBeam();
	  
	} else {
	  
	  // read error - disconnect and try again later
	  
	  if (_params.debug) {
	    cerr << "Read error - closing connection to rdas ..." << endl;
	  }
	  _closeRdas();
	
	} /* SKU_read */

      } else if (iret == -1) {
	
	// timeout

	PMU_auto_register("Timeout waiting for a beam");
	if (_params.debug) {
	  cerr << "Timeout - closing connection to rdas ..." << endl;
	}
	_closeRdas();
      
      } else {

	// select error - disconnect and try again later
	
	PMU_auto_register("Select error");
	if (_params.debug) {
	  cerr << "Select error - closing connection to rdas ..." << endl;
	}
	_closeRdas();
	
      } // if (iret == 1)
      
    } // if (_rdasFd < 0)
    
  } // while

  return 0;

}

//////////////////////////////////////////////////
// check that the connection is open

int Bprp2Dsr::_openRdas()

{

  PMU_auto_register("Opening connection");

  if (_rdasFd >= 0) {
    return 0;
  }
  
  // open connection to rdas - we are the client in this
      
  if ((_rdasFd = SKU_open_client(_params.input_host,
				  _params.input_port)) < 0) {
	
    PMU_auto_register("Waiting for connection");
    
    // failure - sleep for retry later
    
    umsleep(1000);
    return -1;
    
  } else {
	
    if (_params.debug) {
      cerr << "  Connected to rdas ..." << endl;
    }
    
  }
  
  return 0;

}
      
//////////////////////////////////////////////////
// close connection to rdas

void Bprp2Dsr::_closeRdas()

{

  PMU_auto_register("Closing connection");

  if (_rdasFd < 0) {
    return;
  }

  SKU_close(_rdasFd);
  _rdasFd = -1;

}
      
//////////////////////////////////////////////////
// read in a beam
//
// returns 0 on success, -1 on failure

// #define RADAR_MAGIK 0x9d4f28cb

int Bprp2Dsr::_readBeam()

{
  
  // search for magic cookie

  ui08 c1 = 0, c2 = 0, c3 = 0, c4 = 0;
  ui08 c5 = 0, c6 = 0, c7 = 0, c8 = 0;
  
  while (true) {
    
    PMU_auto_register("Waiting for cookie");

    int iret = SKU_read_timed(_rdasFd, &c8, 1, 20, 1000);
    if (iret == -1) {
      continue;
    }
    if (iret != 1) {
      if (_params.debug) {
	cerr << "ERROR - readBeam()" << endl;
	cerr << "  Read error on socket" << endl;
      }
      return -1;
    }

    if ((c5 == 0x9d && c6 == 0x4f && c7 == 0x28 && c8 == 0xcb) ||
	(c8 == 0x9d && c7 == 0x4f && c6 == 0x28 && c5 == 0xcb)) {
      // found cookie
      break;
    }

    // else, cascade byte values

    c1 = c2;
    c2 = c3;
    c3 = c4;
    c4 = c5;
    c5 = c6;
    c6 = c7;
    c7 = c8;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Looking for magic cookie ...." << endl;
      cerr << "  " << setbase(16)
	   << (int)  c1 << " " << (int)  c2 << " "
	   << (int)  c3 << " " << (int)  c4 << " "
	   << (int)  c5 << " " << (int)  c6 << " "
	   << (int)  c7 << " " << (int)  c8
	   << setbase(10) << endl;
    }
    
  }
  
  // found cookie, read a beam and process
  
  int nToRead = sizeof(bprp_response_t) - 2 * sizeof(ui32);
  if (SKU_read_timed(_rdasFd, &_response.reference,
		     nToRead, 100, 10000) != nToRead) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - readBeam()" << endl;
      cerr << "  Bad beam" << endl;
    }
    return -1;
  }

  _response.length = (c1 << 24 | c2 << 16 | c3 << 8 | c4);
  _response.magik = (c5 << 24 | c6 << 16 | c7 << 8 | c8);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "_response.length: " << _response.length << endl;
    cerr << setbase(16) << "_response.magik: "
	 << _response.magik << setbase(10) << endl;
  }

  return 0;

}
	  
//////////////////////////////////////////////////
// handle the response from the radar

int Bprp2Dsr::_handleResponse()

{

  PMU_auto_register("Handling response");

  ui32 magik;
  magik = BE_to_ui32(_response.magik);
  
  if(magik == RADAR_MAGIK) {
    
    _handleBeam();

  } else {
    
    if (_params.debug) {
      cerr << "Radar data error - wrong magik number: " << magik << endl;
    }
    return -1;
    
  } /* if(magik == RADAR_MAGIK) */
      
  return 0;

}

//////////////////////////////////////////////////
// handle the beam from the radar

int Bprp2Dsr::_handleBeam()

{

  PMU_auto_register("Handling beam");

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Handling beam ...." << endl;
  }

  // swap the beam

  bprp_beam_t beam;
  memcpy(&beam, _response.data, sizeof(bprp_beam_t));
  BE_to_array_16((ui16 *) &beam, sizeof(bprp_beam_t));

  // if required, check radar id and skip this
  // routine if it does not match the target
  
  int radarId = (int) (beam.hdr.xmt & 0x1f);
  if (_params.check_radar_id) {
    if (radarId != _params.target_radar_id) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Rejecting radar id: " << radarId << endl;
      }
      return 0;
    }
  }
  
  if (beam.hdr.date == 0 || beam.hdr.site_blk == 0) {
    return -1;
  }

  // beam time

  time_t beamTime;

  int year = beam.hdr.date / 0x200;
  if (year < 1900) {
    if (year < 90) {
      year += 2000;
    } else {
      year += 1900;
    }
  }
  int julday = beam.hdr.date & 0x1ff;
  int hour = beam.hdr.hour;
  int min = beam.hdr.min / 60;
  int sec = beam.hdr.min % 60;
  
  if (_params.time_mode == Params::LOCAL_TO_UCT) {
    
    struct tm date;
    
    date.tm_year = year - 1900;
    date.tm_mon = 0;
    date.tm_mday = julday;
    date.tm_hour = hour;
    date.tm_min = min;
    date.tm_sec = sec;
    date.tm_isdst = 0;
    beamTime = mktime(&date);

  } else {

    date_time_t date;

    date.year = year;
    date.month = 1;
    date.day = julday;
    date.hour = hour;
    date.min = min;
    date.sec = sec;
    uconvert_to_utime(&date);
    beamTime = date.unix_time;

  }

  beamTime += _params.time_correction;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Beam time: " << DateTime::str(beamTime) << endl;
  }
  
  // the azimuth and elevation are either binary coded decimals (Bethlehem) or
  // scaled integers (PACER)

  ui16 raycount;
  double azimuth, elevation;
  
  if (_params.pacer) {

    elevation = (double) (beam.hdr.elevation & 0x7fff) * (180.0 / 32768.0);
    azimuth = (double) beam.hdr.azimuth * (180.0 / 32768.0);
    raycount = beam.hdr.raycount;

  } else {

    azimuth = 0.0;
    elevation = 0.0;
    for (int i = 0, shift = 12, mult = 1000; i < 4;
	 i++, shift -= 4, mult /= 10) {
      azimuth += ((beam.hdr.azimuth >> shift) & 0x0F) * mult; 
      elevation += (((beam.hdr.elevation >> shift) & 0x0F) * mult); 
    }
    
    azimuth /= 10.0;
    elevation /= 10.0;
    raycount = beam.hdr.raycount - 513;

  }

  if(raycount == 0) {
    _rQueue.putEndOfVolume(_volNum, beamTime);
    _volNum++;
    _rQueue.putStartOfVolume(_volNum, beamTime);
  }
  
  int tilt_num =  beam.hdr.raycount >> 9;
  if(tilt_num != _tiltNum) {
    _rQueue.putEndOfTilt(_tiltNum, beamTime);
    _tiltNum = tilt_num;
    _rQueue.putStartOfTilt(_tiltNum, beamTime);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    int rc = (beam.hdr.raycount & 0x1FF);
    fprintf(stderr,
	    "beam_raycount, raycount, _tiltNum, rc, az, el: "
	    "%d, %d, %d, %d, %5.1f %5.1f\n",
	    beam.hdr.raycount, raycount, _tiltNum, rc, azimuth, elevation);
  }
  
  double viplo, viphi;
  int skip;

  if (_params.pacer) {
    skip = (beam.hdr.site_blk & 0x0ff) / 8; 
    viplo = beam.hdr.viplo / 8;
    viphi = beam.hdr.viphi / 8;
  } else {
    skip = (beam.hdr.site_blk & 0x0ff); 
    viplo = beam.hdr.viplo;
    viphi = beam.hdr.viphi;
  }
  
  double atten_at_100km = _params.atmos_attenuation * 100.0;
  double atten_per_km = _params.atmos_attenuation;
  double radar_constant = _params.radar_constant;
  int bin_width = (beam.hdr.site_blk/256) & 0x0f;
  double gate_spacing = bin_width * 0.15;
  double start_range = skip * 0.15 + gate_spacing / 2.0;
  double plo = beam.hdr.plo / 32.0;
  double phi = beam.hdr.phi / 32.0;
  double slope = (phi-plo)/ (viphi-viplo);
  double xmt = beam.hdr.xmt / 32.0;
  double xmtTerm = radar_constant + xmt;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "skip: " << skip << endl;
    cerr << "viplo: " << viplo << endl;
    cerr << "viphi: " << viphi << endl;
    cerr << "atten_at_100km: " << atten_at_100km << endl;
    cerr << "atten_per_km: " << atten_per_km << endl;
    cerr << "radar_constant: " << radar_constant << endl;
    cerr << "bin_width: " << bin_width << endl;
    cerr << "gate_spacing: " << gate_spacing << endl;
    cerr << "start_range: " << start_range << endl;
    cerr << "plo: " << plo << endl;
    cerr << "phi: " << phi << endl;
    cerr << "slope: " << slope << endl;
    cerr << "xmt: " << xmt << endl;
    cerr << "xmtTerm: " << xmtTerm << endl;
    _computeDbz(100.0, atten_per_km, atten_at_100km, xmtTerm,
		beam.vip[0] / 8, slope, viplo, plo, true);
  }

  // load gates, computing dBZ from power

  ui08 ugates[BPRP_GATES_PER_BEAM];
  fl32 fgates[BPRP_GATES_PER_BEAM];
  double range = start_range;

  for(int i = 0; i < BPRP_GATES_PER_BEAM; i++, range += gate_spacing)  {

    if (range < 0.01) {
      fprintf(stderr, "WARNING - send_beam\n");
      fprintf(stderr, "range for gate %d computes as %g\n", i, range);
    }

    // range corr must include a correction for the atmospheric attenuation at
    // 100 km, because that is how RDAS calibration is set up 

    int count = beam.vip[i] / 8;
    double dbz = _computeDbz(range, atten_per_km, atten_at_100km, xmtTerm,
			     count, slope, viplo, plo, false);
    int dbz_byte = (int)((dbz - DBZ_BIAS) / DBZ_SCALE + 0.5);
    
    if (dbz_byte > 255) {
      dbz_byte = 255;
    } else if (dbz_byte < 0) {
      dbz_byte = 0;
    }

    ugates[i] = (ui08) dbz_byte;	
    fgates[i] = (fl32) dbz;	

  } // i

  // send params every 90 beams
  
  _beamCount++;
  if (_beamCount >= 90) {
    _writeRadarAndFieldParams(radarId, start_range, gate_spacing);
    _beamCount = 0;
  }

  // send beam

  if (_params.dbz_as_float32) {
    _writeBeam(beamTime,  azimuth, elevation, fgates,
	       BPRP_GATES_PER_BEAM * sizeof(fl32), sizeof(fl32));
  } else {
    _writeBeam(beamTime,  azimuth, elevation, ugates,
	       BPRP_GATES_PER_BEAM * sizeof(ui08), sizeof(ui08));
  }

  return 0;

}

//////////////////////////////
// compute dbz at given range

double Bprp2Dsr::_computeDbz(double range,
			     double atten_per_km,
			     double atten_at_100km,
			     double xmtTerm,
			     int count,
			     double slope,
			     double viplo,
			     double plo,
			     bool print)
  
{
  
  double range_correction =
    20.0 * log10(range / KM_PER_NM) +
    range * atten_per_km - atten_at_100km - xmtTerm;
  
  double power = (count - viplo) * slope + plo;

  double dbz = power + range_correction;

  if (print) {
    cerr << "range, count, range_corr, power, dbz: "
	 << range << ", "
	 << count << ", "
	 << range_correction << ", "
	 << power << ", "
	 << dbz << endl;
  }

  return dbz;

}
    
////////////////////////////////////////
// write out the radar and field params


int Bprp2Dsr::_writeRadarAndFieldParams(int radarId,
					double start_range,
					double gate_spacing)

{

  // load up radar params and field params message
  
  // radar params

  DsRadarMsg msg;
  DsRadarParams &rParams = msg.getRadarParams();
  
  rParams.radarId = radarId;
  rParams.radarType = DS_RADAR_GROUND_TYPE;
  rParams.numFields = 1;
  rParams.numGates = BPRP_GATES_PER_BEAM;
  rParams.samplesPerBeam = _params.samples_per_beam;
  rParams.scanType = _params.scan_type;
  rParams.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  rParams.polarization = _params.polarization_code;
  
  rParams.radarConstant = _params.radar_constant;
  rParams.altitude = _params.radar_altitude;
  rParams.latitude = _params.radar_latitude;
  rParams.longitude = _params.radar_longitude;
  rParams.gateSpacing = gate_spacing;
  rParams.startRange = start_range;
  rParams.horizBeamWidth = _params.beam_width;
  rParams.vertBeamWidth = _params.beam_width;
  rParams.pulseWidth = _params.pulse_width;
  rParams.pulseRepFreq = _params.prf;
  rParams.wavelength = _params.wavelength;
  rParams.xmitPeakPower = _params.peak_xmit_pwr;
  rParams.receiverMds = _params.receiver_mds;
  rParams.receiverGain = _params.receiver_gain;
  rParams.antennaGain = _params.antenna_gain;
  rParams.systemGain = _params.system_gain;
  rParams.unambigVelocity = _params.unambig_velocity;
  rParams.unambigRange = (3.0e8 / (2.0 * _params.prf)) / 1000.0;

  rParams.radarName = _params.site_name;
  rParams.scanTypeName = _params.scan_name;

  // field params

  vector< DsFieldParams* > &fieldParams = msg.getFieldParams();
  DsFieldParams *fparams = NULL;
  if (_params.dbz_as_float32) {
    fparams = new DsFieldParams( "DBZ", "dBZ", 1.0, 0.0, 4, -9999);
  } else {
    fparams = new DsFieldParams( "DBZ", "dBZ", DBZ_SCALE, DBZ_BIAS, 1, 0);
  }
  if (fparams) {
    fieldParams.push_back(fparams);
  }
  
  // send the params

  if (_rQueue.putDsMsg
      (msg,
       DS_DATA_TYPE_RADAR_PARAMS | DS_DATA_TYPE_RADAR_FIELD_PARAMS)) {
    cerr << "ERROR - Bprp2Dsr::_writeRadarAndFieldParams" << endl;
    cerr << "  Cannot put radar and field params message to FMQ" << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////
// write out a beam

int Bprp2Dsr::_writeBeam(time_t beamTime,
			 double azimuth,
			 double elevation,
			 void *gateData,
			 int dataNBytes,
			 int byteWidth)
  
{

  DsRadarMsg msg;
  DsRadarBeam &radarBeam = msg.getRadarBeam();
  radarBeam.loadData(gateData, dataNBytes, byteWidth);
  radarBeam.dataTime = beamTime;
  radarBeam.azimuth = azimuth;
  radarBeam.elevation = elevation;
  radarBeam.targetElev = elevation;
  radarBeam.tiltNum = _tiltNum;
  radarBeam.volumeNum = _volNum;
  
  // send the params
  
  if (_rQueue.putDsBeam
      (msg, DS_DATA_TYPE_RADAR_BEAM_DATA)) {
    cerr << "ERROR - Bprp2Dsr::_writeBeam" << endl;
    cerr << "  Cannot put beam message to FMQ" << endl;
    return -1;
  }
  
  return 0;

}

