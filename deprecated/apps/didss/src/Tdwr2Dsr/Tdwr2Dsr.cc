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
// Tdwr2Dsr.cc
//
// Tdwr2Dsr object
//
// Gary Blackburn, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2001
//
///////////////////////////////////////////////////////////////

#include <cstdio>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MemBuf.hh>
#include <rapmath/math_macros.h>
#include <rapformats/DsRadarMsg.hh>
#include <dataport/bigend.h>
#include "Tdwr2Dsr.hh"
using namespace std;

const int Tdwr2Dsr::MAX_BUF_SIZE   = 1524;  // more than enough room for
                                              // a single ethernet packet

//////////////////////////////////////////////////
// Constructor

Tdwr2Dsr::Tdwr2Dsr(int argc, char **argv)

{
  okay = true;
  _volNum = 0;
  _beamCount = 0;
  _currentScanType = -1;
  _ppi_num = -1;
  _last_frame_seq = 0;
  _in_progress = FALSE;
  _first_time = TRUE;

  // set programe name

  _progName = (char *)"Tdwr2Dsr";
 // ucopyright(_progName);

  _radial_buffer = new MemBuf();

  // get command line args

  _args = new Args(argc, argv, _progName);

  if (!_args->okay)
  {
	cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line arguments." << endl;
    okay = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *)"unknown";
  if (_params.loadFromArgs(argc, argv, _args->override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    okay = false;
    return;
  }

  // init process mapper registration

  PMU_auto_init(_progName,
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // create the data input object

  // Initialize the UdpMsg here
  if (!_params.playback) {
    if (_params.debug)
      cerr << "Initializing udp connection on port " << _params.port << endl;
      _udp_input = new UdpMsg (_params.port, MAX_BUF_SIZE);
  }
  else
    _tape_input = new TapeMsg (_params.tape_drive);

  if (_rQueue.init(_params.output_url,
		   _progName,
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_compression != Params::NO_COMPRESSION,
		   _params.output_n_slots,
		   _params.output_buf_size)) {
    cerr << "WARNING - trying to create new fmq" << endl;
    if (_rQueue.init(_params.output_url,
		     _progName,
		     _params.debug >= Params::DEBUG_VERBOSE,
		     DsFmq::CREATE, DsFmq::START,
		     _params.output_compression != Params::NO_COMPRESSION,
		     _params.output_n_slots,
		     _params.output_buf_size)) {
      cerr << "ERROR - Tdwr2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_url << endl;
      okay = false;
      return;
    }
  }
      
  _rQueue.setCompressionMethod((ta_compression_method_t)
			       _params.output_compression);

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  // to allow more flexability in mapping 16 bit velocities into 8 bit 
  // (DsRadarQueue doesn't handle 16 bit values), runtime calculation 
  // of the velocity bin size using the 16 bit TDWR scale and bias 
										  
  _minCompressedVelocity = (_params.vel_bias - TDWR_VEL_BIAS) / TDWR_VEL_SCALE;

  _maxCompressedVelocity = ((255 * _params.vel_scale + _params.vel_bias) -
	                                        TDWR_VEL_BIAS) / TDWR_VEL_SCALE;


}

//////////////////////////////////////////////////
// destructor

Tdwr2Dsr::~Tdwr2Dsr()

{

  // unregister process

  PMU_auto_unregister();
  delete _radial_buffer;
  delete _udp_input;
  delete _args;

}

//////////////////////////////////////////////////
// Run

int Tdwr2Dsr::Run()
{
  
  int iret = 0;
  int data_len = 0;
  unsigned char *radial;
  bool complete = FALSE;

  // register with procmap
  
  PMU_auto_register("Run");


  // loop until end of data

  while (TRUE) {
    
    PMU_auto_register("In main loop");

    if (_params.playback) {
      data_len = _tape_input->get_tdwr_data(&radial); 
      if (data_len > 0)
        _convert2Dsr(radial, data_len);
      else {
        cerr << "Exiting program - read returned " << data_len << endl;
        exit(0);
      }
    }
    else {
      complete = getTdwrUdpData(&radial, &data_len);

      // Convert2Dsr(...)
      if (complete) {
        _convert2Dsr(radial, data_len);
      }
    }
	  
  } // while
  
  return iret;

}

//////////////////////////////////////////////////
// getTdwrData - assemble packet data into a complete
//                 TDWR radial 

bool Tdwr2Dsr::getTdwrUdpData(unsigned char **data_ptr, int *data_len)

{

  int pkt_len = 0;
  char packet[MAX_BUF_SIZE];

  Packet_hdr_t    frame_hdr;
  TDWR_data_header_t data;

  memset(packet, 0, MAX_BUF_SIZE); 

  while (TRUE) {

    pkt_len = _udp_input->readUdp(packet);

    frame_hdr  = *((Packet_hdr_t *) packet);

    // deal with big endian/little endian
    TDWRadial::BE_to_frame_hdr (&frame_hdr);

    if (frame_hdr.seq != _last_frame_seq + 1)
    /* a packet was dropped */
    {
      _in_progress = FALSE;
      if (_params.debug > Params::DEBUG_NORM)
        cerr << "dropped beam " << frame_hdr.seq << endl;
    }

    _last_frame_seq = frame_hdr.seq;


    /* if this packet is a first packet then initialize variables */
    if (!_in_progress){
      if (frame_hdr.frame_num_in_msg == FIRST_PKT)
	{
	  _in_progress = TRUE;
	  _radial_buffer->reset();
	  // JHURST Use index 24 instead of 16.  12/12/2008
	  //data = *((TDWR_data_header_t *) &packet[16]);
	  data = *((TDWR_data_header_t *) &packet[24]);
	} else {
	// Need to get another packet
	continue;
      }
    }
    // JHURST Shifted packet index/length by 4 bytes.  12/12/2008
    //_data_ptr = (unsigned char *) _radial_buffer->add(&packet[sizeof(Packet_hdr_t)], pkt_len - sizeof(Packet_hdr_t));
    _data_ptr = (unsigned char *) _radial_buffer->add(&packet[sizeof(Packet_hdr_t)+4], pkt_len - (sizeof(Packet_hdr_t)-4));
    /* if there are a complete compliment of packets then */
    if (frame_hdr.frame_num_in_msg == frame_hdr.frames_per_msg)
    {
      *data_ptr = (unsigned char *) _radial_buffer->getPtr();

      // big endian/little endian stuff
      TDWRadial::BE_to_tdwr_data_hdr ((TDWR_data_header_t *) *data_ptr);
      *data_len = _radial_buffer->getLen();
      _in_progress = FALSE;
      return (TRUE);
    }
  }  // while
  //should never reach here
  return (FALSE);

}

///////////////////////////////////////////////////////
// convert2Dsr - convert the Tdwr radial to a Dsr 
//                 radial and insert it into the rQueue

void Tdwr2Dsr::_convert2Dsr(unsigned char *input_data, int data_len)

{

	int curr_ppi_num;
	DsRadarMsg msg;

	// cast to a tdwr struct
	TDWR_data_header_t *data = (TDWR_data_header_t *) input_data;

	// create a object from struct data
	_tdwr_data = new TDWRadial (data);

	// Currently don't process LLWAS data
	if (_tdwr_data->isLLWASData()) {
          delete _tdwr_data;
	  return;
        }

	// is this radial the first radial in a new volume?
	if (_tdwr_data->newVolume()) {

	  _rQueue.putEndOfVolume(_volNum);
	  _volNum++;
	  _rQueue.putStartOfVolume(_volNum);
          if (_params.debug) {
	    cerr << " beams - " << _beamCount << endl;
	    _print_debug_vol_vals();
          }
	}

        // eliminate low prf data if parameter is set 
        // NOTE: the newVolume information is contained in
        // the low prf scan
        if (_tdwr_data->isLowPrf() && _params.eliminate_low_prf) {
          delete _tdwr_data;
	  return;
        }

	_beamCount++;
	// set up tilt specifice headers for every elevation and first time in
	if (_tdwr_data->newTilt() || _first_time) {

	  _first_time = FALSE;

	  // Determine if the scan type has changed
	  curr_ppi_num = (_tdwr_data->getScanInfoFlag() >> 24);

	  // Mark end of tilt if one has already been processed
	  if (_ppi_num >= 0)
	    _rQueue.putEndOfTilt(_ppi_num );

	  // Did the scan type change?
	  if (_tdwr_data->getScanMode() != _currentScanType) {
		_currentScanType = _tdwr_data->getScanMode();
		if(_currentScanType == SECTOR) {
		  _rQueue.putNewScanType(DS_RADAR_SECTOR_MODE);
	  }
          else if (_currentScanType == RHI)
		 _rQueue.putNewScanType(DS_RADAR_RHI_MODE);
		else
		  _rQueue.putNewScanType(DS_RADAR_SURVEILLANCE_MODE);
	}	   

	// Mark beginning of tilt
	_ppi_num = curr_ppi_num;
	_rQueue.putStartOfTilt(_ppi_num, _tdwr_data->getTimeStamp());

	// Access private parameters for later use
	_tdwr_data->loadScanParams(_tdwrParams);

	if (_params.debug) {
           if (_params.eliminate_low_prf && _tdwrParams.tilt_num == 2)
             cerr << "";
           else
	     cerr << " beams - " << _beamCount << endl;
	    _print_debug_tilt_vals();
		_beamCount = 0;
	}

	// Load DsRadarParams
	DsRadarParams& rParams = msg.getRadarParams();
	_setDsRadarParams(rParams);

	// load DsFieldParams
	vector <DsFieldParams*>& fieldParams = msg.getFieldParams();
	_setDsFieldParams(fieldParams);

	  // send the params
	if (_rQueue.putDsMsg (msg, 
			 DS_DATA_TYPE_RADAR_PARAMS | DS_DATA_TYPE_RADAR_FIELD_PARAMS)) {
 		cerr << "ERROR - Tdwr2Dsr::convert2Dsr" << endl;
		cerr << "  Cannot put radar and field params message to FMQ" << endl;
		return;
	}

	// deallocate memory for the fieldParams
	for (int i=0; i < (int) fieldParams.size(); i++) {
		delete fieldParams[i];
	}

        fieldParams.erase(fieldParams.begin(),fieldParams.end());

      } // newTilt


	// prepare a radial
      DsRadarBeam& radarBeam = msg.getRadarBeam();
      _writeBeam (input_data, radarBeam);

      // send the radial
      if (_rQueue.putDsBeam
        (msg, DS_DATA_TYPE_RADAR_BEAM_DATA)) {
              cerr << "ERROR - Tdwr2Dsr::_convert2Dsr" << endl;
              cerr << "  Cannot put beam message to FMQ" << endl;
              return;
      }

      umsleep(_params.msDelayPostBeam);

      delete _tdwr_data;

      return;
}

//////////////////////////////////////////////////
// setDsRadarParams - For each Tilt (elevation)
//                           write to the queue header info

void Tdwr2Dsr::_setDsRadarParams(DsRadarParams& rParams) {

	int scan_mode = -1;

	// radar params

	rParams.radarId = _params.radar_id;
	rParams.radarType = DS_RADAR_GROUND_TYPE;

	rParams.radarConstant = _params.radar_constant;
	
	rParams.altitude = _params.altitude; 
	rParams.latitude = _params.latitude;
	rParams.longitude = _params.longitude;

	rParams.polarization = DS_POLARIZATION_HORIZ_TYPE;

        // scan strategy
        rParams.scanType = _tdwr_data->getScanType();

	rParams.samplesPerBeam = _tdwrParams.dwell_flag & TDWRadial::SAMPLES_PER_DWELL; // (4096) 

	rParams.startRange = _params.range_to_first_gate;

	rParams.horizBeamWidth = _params.beam_width;
	rParams.vertBeamWidth = _params.beam_width;

	rParams.pulseWidth = _params.pulse_width; 

	rParams.pulseRepFreq = (short ) ((float ) 1000000. / _tdwrParams.pri + .5);

	rParams.wavelength = (3.0e8 / (_params.freq_mhz * 1.0e6)) * 100.0;

	rParams.unambigRange = (3.0e8 / (2.0 * rParams.pulseRepFreq)) / 1000.0;

	rParams.xmitPeakPower = _params.power_trans;

	rParams.receiverMds = _params.receiver_mds;

	rParams.receiverGain = _params.receiver_gain;

	rParams.antennaGain = _params.antenna_gain;
	
	rParams.systemGain = _params.system_gain;

	rParams.unambigVelocity = _params.unambig_vel;
	//rParams.unambigRange = _params.unambig_rng;

	// handle prf dependencies
        // TDWR data only sends out two fields normally, dbz and snr.  The gate
        // spacing is typically 300.  Both of these values can be changed via
        // boolean parameters.  Output resolution and number of fields can be
        // changed to match normal prf scans.

	if (_tdwrParams.message_id == LOW_PRF_BASE_DATA) {
          if (_params.adjust_low_prf_spacing) {
	    rParams.gateSpacing = _params.normal_prf_spacing;
	    rParams.numGates = _tdwrParams.final_range_sample * 2; 
          }
          else {
	    rParams.gateSpacing = _params.low_prf_spacing;
	    rParams.numGates = _tdwrParams.final_range_sample; 
          }

          // number of fields depends on parameter 
          if (_params.add_missing_low_prf_fields)
	    rParams.numFields = _params.normal_prf_num_fields;
          else
	    rParams.numFields = _params.low_prf_num_fields;
	}
	else {
	  rParams.gateSpacing = _params.normal_prf_spacing;
	  rParams.numFields = _params.normal_prf_num_fields;
	  rParams.numGates = _tdwrParams.final_range_sample; 
        }

	scan_mode = _tdwr_data->getScanMode();
	if(scan_mode == SECTOR)
		rParams.scanMode = DS_RADAR_SECTOR_MODE;
	else if (scan_mode == RHI)
		rParams.scanMode = DS_RADAR_RHI_MODE;
	  else
		rParams.scanMode = DS_RADAR_SURVEILLANCE_MODE;
}

//////////////////////////////////////////////////
// setDsFieldParams - 
//                   

void Tdwr2Dsr::_setDsFieldParams ( vector< DsFieldParams* >& fieldParams) {
	
  fieldParams.push_back (new DsFieldParams ("DBZ", "dBZ", 0.5, -30.));
  fieldParams.push_back (new DsFieldParams ("SNR", "dB", 0.5, 0.0));

  if (_tdwrParams.message_id == LOW_PRF_BASE_DATA 
                          && !_params.add_missing_low_prf_fields ) {
    // Just send the two fields
    return;
  }
  else {              // send the other three fields

    // _params."values" allow for more dynamic defination of output scale/bias
    fieldParams.push_back (new DsFieldParams ("VEL", "m/s", 
                (float) _params.vel_scale/100, (float) _params.vel_bias/100));

    fieldParams.push_back (new DsFieldParams ("SW", "m/s", 0.25, 0.0));

    // _params."values" allow for more dynamic defination of output scale/bias
    fieldParams.push_back (new DsFieldParams ("DVEL", "m/s", 
                (float) _params.vel_scale/100, (float) _params.vel_bias/100));
  }
}

//////////////////////////////////////////////////
// writeBeams - 
//                   

void Tdwr2Dsr::_writeBeam (unsigned char *data_input, DsRadarBeam& rBeam) {

  rBeam.azimuth = _tdwr_data->getAzimuth() + _params.true_north_adj + .5;
  if (rBeam.azimuth >= 360.)
    rBeam.azimuth -= 360.;
  rBeam.elevation = _tdwrParams.elevation;
  rBeam.targetElev = _tdwrParams.target_elev;
  rBeam.tiltNum = _tdwrParams.tilt_num;
  rBeam.volumeNum = _tdwrParams.volume_num;
  rBeam.dataTime = _tdwr_data->getTimeStamp();

  int num_fields;
  int num_gates;
  int data_len;

  // calculate dataLen
  num_fields = _params.normal_prf_num_fields;  // default
  data_len = _tdwrParams.final_range_sample * num_fields;  // default
  num_gates = _tdwrParams.final_range_sample;

  if (_tdwrParams.message_id == LOW_PRF_BASE_DATA) {
     if (!_params.add_missing_low_prf_fields)
        num_fields = _params.low_prf_num_fields;
     if (_params.adjust_low_prf_spacing) {
       data_len *= 2;
       num_gates *= 2;
     }
  }

  ui08 data[data_len];
  memset (data, 0, data_len);

  // normal prf data is gate by gate data, low prf data is field data


  double minDbzByte = 2.0*(_params.min_dbz + 30.0); // Hard coded output scale, bias for DBZ.


  // If flag was set to eliminate the low prf field, the low prf scan
  // will be eliminated before reaching here
  if (_tdwrParams.message_id == LOW_PRF_BASE_DATA) {

    unsigned char *dbz_ptr;
    unsigned char *snr_ptr;
    unsigned char *flag_ptr;

    ui08 *data_ptr = data;
    unsigned short flag;

    dbz_ptr = data_input + sizeof (TDWR_data_header_t);
    snr_ptr = dbz_ptr + _tdwrParams.rng_samples_per_dwell;
    flag_ptr = snr_ptr + _tdwrParams.rng_samples_per_dwell;

    // field data
    for (int igate = 0; igate < _tdwrParams.final_range_sample; igate++) {

      if (double(dbz_ptr[igate]) < minDbzByte){
	data_ptr[0] = 0;
	data_ptr[1] = 0;
      	data_ptr[2] = 0;
	data_ptr[3] = 0;
	data_ptr[4] = 0;
      } else {

	data_ptr[0] = (ui08) dbz_ptr[igate];
	data_ptr[1] = (ui08) snr_ptr[igate];

	flag = *flag_ptr;
	if (_params.ctf) {
	  // point target flag 
	  if ((flag & CTF) == CTF)
	    data_ptr[0] = 0;
	}
	
	if (_params.cv)
	  // compressed valid flag
	  if ((flag & CV) == 0) {
	    data_ptr[0] = 0;
	    data_ptr[1] = 0;
	  }
	
	// satisfy request to add missing fields to low prf data
	if (_params.add_missing_low_prf_fields) {
	  data_ptr[2] = 0;
	  data_ptr[3] = 0;
	  data_ptr[4] = 0;
	  if (_params.adjust_low_prf_spacing) {
	    data_ptr[num_fields+2] = 0;
	    data_ptr[num_fields+3] = 0;
	    data_ptr[num_fields+4] = 0;
	  }
	}
	if (_params.adjust_low_prf_spacing) {
	  data_ptr[num_fields] = data_ptr[0];
	  data_ptr[num_fields+1] = data_ptr[1];
	  data_ptr += num_fields;
	}
      }

      dbz_ptr++;
      snr_ptr++;
      flag_ptr++;
      data_ptr += num_fields;

    }
  }
  else {  
    // gate data
    TDWRadial::normal_prf_data_t  *normal_prf;
    ui08 *data_ptr = data;
    //unsigned short velocity;

    normal_prf = (TDWRadial::normal_prf_data_t *) 
				   (data_input + sizeof (TDWR_data_header_t));

    for (int i = 0; i < num_gates; i++) {


      if (double(normal_prf->dbz) < minDbzByte){
	data_ptr[0] = 0;
	data_ptr[1] = 0;
      	data_ptr[2] = 0;
	data_ptr[3] = 0;
	data_ptr[4] = 0;
      } else {

	data_ptr[0] = (ui08) normal_prf->dbz;
	data_ptr[1] = (ui08) normal_prf->snr;

	// convert 16 bit velocities into 8 bit
	BE_to_array_16 (&normal_prf->vel, 2);
	if (normal_prf->vel> _maxCompressedVelocity)
	  data_ptr[2] = 255;
	else if (normal_prf->vel < _minCompressedVelocity)
	  data_ptr[2] = 0;
	else
	  data_ptr[2] = (ui08) (((float ) (normal_prf->vel
					   * TDWR_VEL_SCALE + TDWR_VEL_BIAS - _params.vel_bias)
				 / (float) (_params.vel_scale)) + .5);

	data_ptr[3] = (ui08) normal_prf->width;
	
	// convert 16 bit velocities into 8 bit
	BE_to_array_16 (&normal_prf->dealias_vel, 2);
	if (normal_prf->dealias_vel> _maxCompressedVelocity)
	  data_ptr[4] = 255;
	else if (normal_prf->dealias_vel < _minCompressedVelocity)
	  data_ptr[4] = 0;
	else
	  data_ptr[4] = (ui08) (((float ) (normal_prf->dealias_vel
					   * TDWR_VEL_SCALE + TDWR_VEL_BIAS - _params.vel_bias)
				 / (float) (_params.vel_scale)) + .5);
	
	
	_applyDataFlags (data_ptr, normal_prf->data_flag);
      }
      normal_prf++;
      data_ptr+= num_fields;
    } // for num_gates

  } // else normal prf

  rBeam.loadData ((ui08 *)data, data_len);
}

//////////////////////////////////////////////////
// print_debug_vol_vals - 
//                   

void Tdwr2Dsr::_print_debug_vol_vals() {

  if (_tdwr_data->newVolume()) 
  {
    cerr << endl << endl;
    cerr << "******************************************************" << endl;
    cerr << "New Volume #" << _tdwrParams.volume_num << " - "  
					<< DateTime::str(_tdwr_data->getTimeStamp()) 
					<< " - TDWR radar" << endl;
    cerr << endl;
  }

} 

//////////////////////////////////////////////////
// print_debug_tilt_vals - 
//                   

void Tdwr2Dsr::_print_debug_tilt_vals() {

    char message[512];

    if ((_tdwrParams.message_id == LOW_PRF_BASE_DATA) && _params.adjust_low_prf_spacing)
      sprintf (message, "tilt # %2d, fixed ang %5.2f,  prf %d, num gates %4d", _tdwrParams.tilt_num, _tdwrParams.target_elev, (short ) ((float ) 1000000. / _tdwrParams.pri + .5), _tdwrParams.final_range_sample * 2);
    else
      sprintf (message, "tilt # %2d, fixed ang %5.2f,  prf %d, num gates %4d", _tdwrParams.tilt_num, _tdwrParams.target_elev, (short ) ((float ) 1000000. / _tdwrParams.pri + .5), _tdwrParams.final_range_sample);

    cerr << message;
}

//////////////////////////////////////////////////
// applyDataFlags - 
//                   

void Tdwr2Dsr::_applyDataFlags(ui08 *data_ptr, unsigned char data_flag) {

  if (_params.caf) {
    if ((data_flag & CAF) == CAF) {
      // compressed dealias algorithm failure flag
      data_ptr[4] = 0;
    }
  }
  if (_params.ctf) {
    if ((data_flag & CTF) == CTF) {
      // point target flag
      data_ptr[0] = 0;  // dbz
      data_ptr[1] = 0;  // signal to noise
      data_ptr[2] = 0;  // unconditioned vel
      data_ptr[3] = 0;  // Spectrum width
      data_ptr[4] = 0;  // conditioned vel
    }
  }
  if (_params.ccv) {
    if ((data_flag & CCV) == 0) {
      // compressed conditioned valid flag removes clutter
      // and implements thresholding on velocity and SN
      data_ptr[2] = 0;  // unconditioned vel
      data_ptr[3] = 0;  // Spectrum width
      data_ptr[4] = 0;  // conditioned vel
    }
  }
  if (_params.cvf) {
    if ((data_flag & CVF) == 0) {
      // compressed valid flag removes clutter
      data_ptr[0] = 0;  // dbz
      data_ptr[2] = 0;  // unconditioned vel
      data_ptr[3] = 0;  // Spectrum width
      data_ptr[4] = 0;  // conditioned vel
    }
  }
}

//---------------------------------------------------------------------------*/
