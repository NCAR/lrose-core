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
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <rapmath/math_macros.h>
#include <dsserver/DsLdataInfo.hh>
#include <rapformats/UfRecord.hh>
#include "Dsr2UF.hh"
using namespace std;

// Constructor

Dsr2UF::Dsr2UF(int argc, char **argv)

{

  isOK = TRUE;
  _scanType = -1;
  _scanMode = -1;
  _out = NULL;
  _rayNumInVol = 0;
  _snrCensorWarningPrinted = false;
  
  // set programe name

  _progName = "Dsr2UF";
  ucopyright(_progName.c_str());

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
  }
  
  if (!isOK) {
    return;
  }

  PMU_auto_init(_progName.c_str(), _params.instance, PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Dsr2UF::~Dsr2UF()

{

  _closeOutputFile();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Dsr2UF::Run ()
{

  PMU_auto_register("Dsr2UF::Run");

  // Instantiate and initialize the DsRadar queue and message

  DsRadarQueue radarQueue;
  DsRadarMsg radarMsg;

  if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
		      _params.debug,
		      DsFmq::BLOCKING_READ_WRITE, DsFmq::END )) {
    cerr << "ERROR - Dsr2UF::Run" << endl;
    cerr << "Could not initialize radar queue: " << _params.input_fmq_url << endl;
    return(-1);
  }

  // create field translation vector

  vector<UfRecord::field_tranlation_t> fieldTrans;
  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    UfRecord::field_tranlation_t trans;
    trans.input_name = _params._output_fields[ii].dsr_name;
    trans.uf_name = _params._output_fields[ii].uf_name;
    trans.scale = _params._output_fields[ii].scale;
    fieldTrans.push_back(trans);
  }
 
  // read beams from the queue and process them

  _rayNumInVol = 0;
  int msgContents;

  while (true) {
    
    bool end_of_vol = false;

    if (_readMsg(radarQueue, radarMsg, msgContents, end_of_vol) == 0) {
      
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Read message from radar queue." << endl;
      }      

      // does it have beam data?
      
      if (msgContents & DsRadarMsg::RADAR_BEAM) {
	
	// get beam
	
	const DsRadarBeam &beam = radarMsg.getRadarBeam();
	if (_params.remove_antenna_transitions && beam.antennaTransition) {
	  continue;
	}
	_beamTime = beam.dataTime;
	
	// check file is open

	if (_openOutputFile()) {
	  return -1;
	}

        // censor if appropriate

        if (_params.censor_on_snr) {
          _censorOnSnr(radarMsg);
        }
	
	// create UF record, load from message
	
	UfRecord record;
	if (record.loadFromDsRadarMsg(radarMsg, _rayNumInVol, fieldTrans)) {
	  cerr << "ERROR - Dsr2UF" << endl;
	  cerr << "  loading UF record from DsRadarMsg" << endl;
	  continue;
	}
	
	// Write UF record to output file.

	if (!_params.skip_all_missing || !record.allDataMissing()) {
	  if (record.write(_out, _params.output_little_endian)) {
	    cerr << "ERROR - Dsr2UF" << endl;
	    cerr << "  Writing file: " << _tmpPath << endl;
	  }
	} else {
	  if (_params.debug >= Params::DEBUG_VERBOSE) {
	    cerr << "All data missing, skipping write, el, az: "
		 << record.elevation << ", " << record.azimuth << endl;
	  }
	}

      } // if (msgContents & DsRadarMsg::RADAR_BEAM)
      
      // close file at end of vol
     
      if (end_of_vol) {
	if (_closeOutputFile()) {
	  return -1;
	}
      }
      
    } //  if (_readMsg ...

  } // while (forever)
  
  return (0);

}

////////////////////////////////////////////////////////////////////
// _readMsg()
//
// Read a message from the queue, setting the flags about beam_data
// and end_of_volume appropriately.
//

int Dsr2UF::_readMsg(DsRadarQueue &radarQueue,
		     DsRadarMsg &radarMsg,
		     int &contents,
		     bool &end_of_vol) 
  
{
  
  PMU_auto_register("Reading radar queue");

  //
  // Read messages until all params have been set
  //

  int forever = TRUE;
  int scan_type = 0;
  while (forever) {

    if (radarQueue.getDsMsg(radarMsg, &contents)) {
      return (-1);
    }
    
    // check if parameters have been set,If so, accept the beam.
    
    if (radarMsg.allParamsSet()) {

      DsRadarParams &radarParams= radarMsg.getRadarParams();
      
      scan_type = radarParams.scanType;
      _scanMode = radarParams.scanMode;

      // accept beam
      break;

    } // if (radarMsg.allParamsSet()) 
    
  } // while (forever)

  // is this an end of vol?
  
  end_of_vol = FALSE;
  if (contents & DsRadarMsg::RADAR_FLAGS) {
    DsRadarFlags& flags = radarMsg.getRadarFlags();
    if (_params.end_of_vol_decision == Params::END_OF_VOL_FLAG) {
      if (flags.endOfVolume) {
	// end-of-vol flag
	end_of_vol = TRUE;
      }
    } else if (flags.endOfTilt &&
	       flags.tiltNum == _params.last_tilt_in_vol) {
      // look last tilt in vol
      end_of_vol = TRUE;
    } else if (flags.newScanType) {
      end_of_vol = TRUE;
    }
  }
  if (_scanType >= 0 && _scanType != scan_type) {
    end_of_vol = TRUE;
  }
  
  _scanType = scan_type;

  return (0);

}

////////////////////////////////////////////////////////////////////
// open output file, for a given time
//
// returns 0 on success, -1 on failure

int Dsr2UF::_openOutputFile() 

{

  if (_out != NULL) {
    // already open
    return 0;
  }

  _startTime = _beamTime;
  DateTime startTime(_startTime);
  
  // compute output dir name, using date
  
  char outDir[MAX_PATH_LEN];
  sprintf(outDir, "%s%s%.4d%.2d%.2d",
	  _params.output_uf_dir, PATH_DELIM,
	  startTime.getYear(), startTime.getMonth(), startTime.getDay());
  
  // make output dir
  
  if (ta_makedir_recurse(_params.output_uf_dir)) {
    int errNum = errno;
    cerr << "ERROR - Dsr2UF" << endl;
    cerr << "  Cannot make output dir: " << _params.output_uf_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // compute tmp path name
  
  sprintf(_tmpPath,"%s%s%s%.4d%.2d%.2d.%.2d%.2d%.2d.%s.tmp",
	  _params.output_uf_dir, PATH_DELIM,
	  _params.output_file_prefix,
	  startTime.getYear(), startTime.getMonth(), startTime.getDay(),
	  startTime.getHour(), startTime.getMin(), startTime.getSec(),
	  _params.output_file_ext);
  
  if ((_out = fopen(_tmpPath,"w")) == NULL) {
    fprintf(stderr, "ERROR - %s:Dsr2UF::_openOutputFile\n", _progName.c_str());
    fprintf(stderr, "Could not open output file '%s'\n",_tmpPath);
    return -1;
  }

  if (_params.debug) {
    cerr << "Opened new file: " << _tmpPath << endl;
  }

  _rayNumInVol = 0;

  return 0;

}
	
////////////////////////////////////////////////////////////////////
// close output file

int Dsr2UF::_closeOutputFile() 
  
{
  
  if (_out == NULL) {
    return 0;
  }

  fclose(_out);
  _out = NULL;
  
  if (_params.debug) {
    cerr << "Closed tmp file: " << _tmpPath << endl;
  }
  
  _endTime = _beamTime;
  DateTime endTime(_endTime);
  
  // compute output dir name, using date
  
  char outDir[MAX_PATH_LEN];
  sprintf(outDir, "%s%s%.4d%.2d%.2d",
	  _params.output_uf_dir, PATH_DELIM,
	  endTime.getYear(), endTime.getMonth(), endTime.getDay());
  
  // make output dir
  
  if (ta_makedir_recurse(outDir)) {
    int errNum = errno;
    cerr << "ERROR - Dsr2UF" << endl;
    cerr << "  Cannot make output dir: " << outDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute final path name

  string modeStr = "sur";
  if (_scanMode == DS_RADAR_SECTOR_MODE) {
    modeStr = "sec";
  } else if (_scanMode == DS_RADAR_RHI_MODE) {
    modeStr = "rhi";
  }
  
  char outPath[MAX_PATH_LEN];
  sprintf(outPath,"%s%s%s_%s_%.4d%.2d%.2d.%.2d%.2d%.2d.%s",
	  outDir, PATH_DELIM,
	  _params.output_file_prefix,
	  modeStr.c_str(),
	  endTime.getYear(), endTime.getMonth(), endTime.getDay(),
	  endTime.getHour(), endTime.getMin(), endTime.getSec(),
	  _params.output_file_ext);
  
  // rename the tmp to final output file path
  
  if (_params.debug) {
    cerr << "Renaming tmp file: " << _tmpPath << endl;
    cerr << "               to: " << outPath << endl;
  }
  

  if (rename(_tmpPath, outPath)) {
    int errNum = errno;
    cerr << "ERROR - Dsr2UF" << endl;
    cerr << "  Cannot rename output file" << endl;
    cerr << "  Tmp path: " << _tmpPath << endl;
    cerr << "  Final path: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  // compute relative path
  
  string relPath;
  Path::stripDir(_params.output_uf_dir, outPath, relPath);
  
  // write latest data info file
  
  DsLdataInfo ldata(_params.output_uf_dir);
  ldata.setRelDataPath(relPath);
  ldata.setDataFileExt(_params.output_file_ext);
  ldata.setDataType("uf");
  ldata.setWriter("Dsr2UF");
  if (ldata.write(_endTime)) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// censor based on SNR

void Dsr2UF::_censorOnSnr(DsRadarMsg &radarMsg)

{

  // check SNR field exists

  const vector<DsFieldParams*> &fieldParams = radarMsg.getFieldParams();

  bool snrFound = false;
  for (int ii = 0; ii < (int) fieldParams.size(); ii++) {
    if (fieldParams[ii]->name == _params.input_snr_field_name) {
      snrFound = true;
      break;
    }
  }

  if (!snrFound) {
    if (!_snrCensorWarningPrinted) {
      cerr << "WARNING - Dsr2UF::_censorOnSnr" << endl;
      cerr << "  SNR field not found in input data: "
           << _params.input_snr_field_name << endl;
      cerr << "  Censoring will not be performed" << endl;
      _snrCensorWarningPrinted = true;
    }
    return;
  }

  // perform censoring

  radarMsg.censorBeamData(_params.input_snr_field_name,
                          _params.snr_threshold,
                          1.0e99);

}
  
