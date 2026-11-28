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
/**
 *
 * @file QPESum2Dsr.cc
 *
 * @class QPESum2Dsr
 *
 * QPESum2Dsr is the top level application class.
 *  
 * @date 7/27/2010
 *
 */

#include <iostream>
#include <limits.h>
#include <string>
#include <vector>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "QPESum2Dsr.hh"
#include "Params.hh"

using namespace std;


// Global variables

QPESum2Dsr *QPESum2Dsr::_instance = (QPESum2Dsr *) NULL;

const ui08 QPESum2Dsr::SCALED_MISSING_DATA_VALUE = 255;


/*********************************************************************
 * Constructor
 */

QPESum2Dsr::QPESum2Dsr(int argc, char **argv) :
  _volNum(-1) {
  static const string method_name = "QPESum2Dsr::QPESum2Dsr()";

  // Make sure the singleton wasn't already created.

  assert(_instance == (QPESum2Dsr *) NULL);

  // Initialize the okay flag.

  okay = true;

  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);

  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

  // Get TDRP parameters.

  _params = new Params();
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");

  _tilts = new int[9];

  if (_params->loadFromArgs(argc, argv,
                            _args->override.list,
                            &params_path)) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;

    okay = false;
    return;
  }
}


/*********************************************************************
 * Destructor
 */

QPESum2Dsr::~QPESum2Dsr() {
  // Trigger the processing of the last of the data

  _radarQueue.putEndOfVolume(_volNum);

  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  delete[] _tilts;

  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst()
 */

QPESum2Dsr *QPESum2Dsr::Inst(int argc, char **argv) {
  if (_instance == (QPESum2Dsr *) NULL)
    new QPESum2Dsr(argc, argv);

  return (_instance);
}

QPESum2Dsr *QPESum2Dsr::Inst() {
  assert(_instance != (QPESum2Dsr *) NULL);

  return (_instance);
}


/*********************************************************************
 * init()
 */

bool QPESum2Dsr::init() {
  static const string method_name = "QPESum2Dsr::init()";

  // Initialize the input trigger

  if (!_initTrigger())
    return false;

  // Initialize the radar queue

  if (_radarQueue.init(_params->output_fmq_url,
                       "QPESum2Dsr",
                       _params->debug,
                       DsFmq::READ_WRITE, DsFmq::END,
                       _params->output_fmq_compress,
                       _params->output_fmq_nslots,
                       _params->output_fmq_size,
                       1000)) {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing radar queue: "
         << _params->output_fmq_url << endl;

    return false;
  }

  if (_params->use_blocking_write)
    _radarQueue.setBlockingWrite();

  // Initialize the field params in the radar message.

  if (!_initFieldParams())
    return false;

  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
                PROCMAP_REGISTER_INTERVAL);

  _lastIssueTime = 0;
  _numberOfTilts = 0;

  return true;
}


/*********************************************************************
 * run()
 */

void QPESum2Dsr::run() {
  static const string method_name = "QPESum2Dsr::run()";

  TriggerInfo trigger_info;

  while (!_dataTrigger->endOfData()) {
    if (_dataTrigger->next(trigger_info) != 0) {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;

      continue;
    }

    if (!_processData(trigger_info.getFilePath(), trigger_info.getIssueTime())) {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing file: " << trigger_info.getFilePath() << endl;

      continue;
    }

  } /* endwhile - !_dataTrigger->endOfData() */
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _initFieldParams()
 */

bool QPESum2Dsr::_initFieldParams(void) {
  DsFieldParams field_params;

  field_params.byteWidth = 1;
  field_params.missingDataValue = SCALED_MISSING_DATA_VALUE;
  field_params.scale = _params->field_info.scale;
  field_params.bias = _params->field_info.bias;
  field_params.name = _params->field_info.name;
  field_params.units = _params->field_info.units;

  _radarMsg.addFieldParams(field_params);

  return true;
}


/*********************************************************************
 * _initRadarParams()
 */

void QPESum2Dsr::_initRadarParams(const RadarFile &radar_file) {
  DsRadarParams &radar_params = _radarMsg.getRadarParams();

  radar_params.radarType = DS_RADAR_GROUND_TYPE;
  radar_params.numFields = 1;
  radar_params.numGates = radar_file.getNumGates();
  radar_params.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  radar_params.altitude = radar_file.getRadarAltKm();
  radar_params.latitude = radar_file.getRadarLat();
  radar_params.longitude = radar_file.getRadarLon();
  radar_params.gateSpacing = radar_file.getGateSpacingKm();
  radar_params.startRange = radar_file.getStartRangeKm();
  radar_params.horizBeamWidth = _params->horiz_beam_width;
  radar_params.vertBeamWidth = _params->vert_beam_width;
}


/*********************************************************************
 * _initTrigger()
 */

bool QPESum2Dsr::_initTrigger(void) {
  static const string method_name = "QPESum2Dsr::_initTrigger()";

  switch (_params->trigger_mode) {
    case Params::FILE_LIST : {
      const vector<string> file_list = _args->getFileList();

      if (file_list.size() == 0) {
        cerr << "ERROR: " << method_name << endl;
        cerr << "Must specify file paths on command line" << endl;

        return false;
      }

      if (_params->debug) {
        cerr << "Initializing FILE_LIST trigger: " << endl;
        vector<string>::const_iterator file;
        for (file = file_list.begin(); file != file_list.end(); ++file)
          cerr << "   " << *file << endl;
      }

      DsFileListTrigger *trigger = new DsFileListTrigger();
      if (trigger->init(file_list) != 0) {
        cerr << "ERROR: " << method_name << endl;
        cerr << "Error initializing FILE_LIST trigger:" << endl;
        vector<string>::const_iterator file;
        for (file = file_list.begin(); file != file_list.end(); ++file)
          cerr << "   " << *file << endl;
        cerr << trigger->getErrStr() << endl;

        return false;
      }

      _dataTrigger = trigger;

      break;
    }
    case Params::REALTIME : {

      if (_params->debug) {
        cerr << "Initializing REALTIME trigger using directory: " <<
             _params->realtimeInput.InputDir << endl;
      }

      struct stat stat_info;
      if (stat(_params->realtimeInput.InputDir, &stat_info) != 0) {
        cerr << "Error: Directory " << _params->realtimeInput.InputDir
             << " does not exist." << endl;
        return false;
      }

      DsInputDirTrigger *trigger = new DsInputDirTrigger();
      if (trigger->init(_params->realtimeInput.InputDir,
                        _params->realtimeInput.IncludeSubString,
                        _params->realtimeInput.ProcessOldFiles,
                        PMU_auto_register,
                        false,
                        _params->realtimeInput.ExcludeSubString) != 0) {
        cerr << "ERROR: " << method_name << endl;
        cerr << "Error initializing REALTIME trigger for directory:" <<
             _params->realtimeInput.InputDir << endl;
        cerr << trigger->getErrStr() << endl;

        return false;
      }

      _dataTrigger = trigger;

      if (_params->debug)
        cerr << "Successfully initialized  REALTIME trigger for directory:" <<
             _params->realtimeInput.InputDir << endl;

      break;
    }

  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _processData()
 */

bool QPESum2Dsr::_processData(const string &file_path, const time_t &issue_time) {
  static const string method_name = "QPESum2Dsr::_processData()";

  PMU_auto_register("Processing input file");

  if (_params->debug)
    cerr << endl << endl
         << "*** Processing file: " << file_path << endl << endl;

  // Read the file

  RadarFile input_file(file_path, _params->debug);

  if (!input_file.readFile())
    return false;

  if (_params->debug)
    input_file.print(cerr, "  ");

  // Write the data to the FMQ

  _initRadarParams(input_file);

  cerr << "issue_time = " << issue_time << endl;
  cerr << "_lastIssueTime = " << _lastIssueTime << endl;

  if (issue_time - _lastIssueTime > 150) {
    if (_volNum >= 0)
      _radarQueue.putEndOfVolume(_volNum);
    ++_volNum;
    cerr << "Writing start of Volume\n";
    _radarQueue.putStartOfVolume(_volNum);
    _lastIssueTime = issue_time;
    for (int i = 1; i < 10; i++)
      _tilts[i] = 0;
  }

  int tilt_num = input_file.getTiltNum();

  if (_tilts[tilt_num] != 0) {
    cerr << "tilt " << tilt_num << " already processed for this volume" << endl;
    return false;
  }

  _tilts[tilt_num] = tilt_num;


  _radarQueue.putStartOfTilt(tilt_num);

  size_t num_rays = input_file.getNumRays();

  for (size_t ray = 0; ray < num_rays; ++ray) {
    // Get the beam information

    double azimuth;
    double elev_angle;
    DateTime beam_time;
    vector<double> beam_data;

    if (!input_file.getRay(ray, azimuth, elev_angle,
                           beam_time, beam_data))
      return false;

    // Construct the message

    int msg_content = 0;

    if (ray % _params->output_radar_param_freq == 0) {
      msg_content |= (int) DsRadarMsg::RADAR_PARAMS;
      msg_content |= (int) DsRadarMsg::FIELD_PARAMS;
    }

    msg_content |= (int) DsRadarMsg::RADAR_BEAM;

    DsRadarBeam &radar_beam = _radarMsg.getRadarBeam();

    radar_beam.dataTime = beam_time.utime();
    radar_beam.azimuth = azimuth;
    radar_beam.elevation = elev_angle;
    radar_beam.targetElev = elev_angle;
    radar_beam.volumeNum = _volNum;
    radar_beam.tiltNum = tilt_num;

    size_t num_gates = beam_data.size();
    double input_missing_value = input_file.getMissingDataValue();

    ui08 *beam_buffer = new ui08[num_gates];

    for (size_t gate = 0; gate < num_gates; ++gate) {
      if (beam_data[gate] == input_missing_value || beam_data[gate] == -999) {
        beam_buffer[gate] = SCALED_MISSING_DATA_VALUE;
      } else {
        beam_buffer[gate] =
          (ui08) (((beam_data[gate] - _params->field_info.bias) /
                   _params->field_info.scale) + 0.5);
      }
    } /* endfor - gate */

    radar_beam.loadData(beam_buffer, num_gates, 1);

    if (_radarQueue.putDsBeam(_radarMsg, msg_content) != 0) {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing beam to FMQ" << endl;
    }
    delete[] beam_buffer;

  } /* endfor - ray */

  _radarQueue.putEndOfTilt(tilt_num);
  _numberOfTilts++;

  cerr << "_numberOfTilts = " << _numberOfTilts << endl;

  if (_numberOfTilts == 9) {
    cerr << "Writing end of Volume\n";

    _radarQueue.putEndOfVolume(_volNum);
    _numberOfTilts = 0;
  }

  return true;
}
