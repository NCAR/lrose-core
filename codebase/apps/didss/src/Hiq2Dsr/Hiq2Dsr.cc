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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:40 $
//   $Id: Hiq2Dsr.cc,v 1.8 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Hiq2Dsr: Hiq2Dsr program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#include <toolsa/os_config.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Hiq2Dsr.hh"
#include "HiqMsg.hh"
#include "Params.hh"

// MsgFactory classes

#include "ArcMsgFactory.hh"
#include "EolMsgFactory.hh"

// Reader classes

#include "RapFileReader.hh"
#include "UdpReader.hh"

#include "ArcHiqReader.hh"
#include "EolHiqReader.hh"

// Writer classes

#include "ArcBeamWriter.hh"
#include "SimpleBeamWriter.hh"
#include "TiltNumBeamWriter.hh"

// Archiver classes

#include "FmqArchiver.hh"
#include "MultFileArchiver.hh"
#include "SingleFileArchiver.hh"

// End-of-volume strategy classes

#include "DropEOVStrategy.hh"
#include "EndEOVStrategy.hh"
#include "StartEOVStrategy.hh"

using namespace std;


// Global variables

Hiq2Dsr *Hiq2Dsr::_instance = (Hiq2Dsr *)NULL;


/*********************************************************************
 * Constructor
 */

Hiq2Dsr::Hiq2Dsr(int argc, char **argv)
{
  static const string method_name = "Hiq2Dsr::Hiq2Dsr()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Hiq2Dsr *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

Hiq2Dsr::~Hiq2Dsr()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Hiq2Dsr *Hiq2Dsr::Inst(int argc, char **argv)
{
  if (_instance == (Hiq2Dsr *)NULL)
    new Hiq2Dsr(argc, argv);
  
  return(_instance);
}

Hiq2Dsr *Hiq2Dsr::Inst()
{
  assert(_instance != (Hiq2Dsr *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Hiq2Dsr::init()
{
  static const string method_name = "Hiq2Dsr::init()";
  
  // Initialze the reader

  if (!_initReader())
    return false;
  
  // Initialize the beam writer object

  if (!_initBeamWriter())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void Hiq2Dsr::run()
{
  static const string method_name = "Hiq2Dsr::run()";
  
  // Read and process beams forever

  while (true)
  {
    if (!_processData())
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Trouble reading data" << endl;
      
    }
    PMU_auto_register( "Processing data" );
  }

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _initBeamWriter() - Initialize the BeamWriter object.
 *
 * Returns true on success, false on failure.
 */

bool Hiq2Dsr::_initBeamWriter(void)
{
  static const string method_name = "Hiq2Dsr::_initBeamWriter()";
  
  DsRadarQueue *radar_queue = new DsRadarQueue();

  if (radar_queue->init(_params->output_fmq_url,
			_progName,
			_params->debug,
			DsFmq::READ_WRITE, DsFmq::END,
			_params->output_fmq_compress,
			_params->output_fmq_nslots,
			_params->output_fmq_size,
			1000))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not initialize radar queue ''"
	 << _params->output_fmq_url << "'" << endl;
    
    return false;
  }

  switch (_params->msg_type)
  {
  case Params::EOL_HIQ_MSGS :
  {
    ScanStrategy scan_strategy;

    for (int i = 0; i < _params->scan_strategy_n; ++i)
      scan_strategy.addElevation(_params->_scan_strategy[i]);

    EOVStrategy *eov_strategy = _initEovStrategy(scan_strategy);
    
    MedianFilter *median_filter = new MedianFilter();
    
    median_filter->init(_params->median_filter_beams_before,
			_params->median_filter_beams_after,
			_params->use_elev_median_filter,
			_params->use_az_median_filter,
			_params->debug);
  
    switch (_params->output_type)
    {
    case Params::OUTPUT_SIMPLE :
    {
      SimpleBeamWriter *beam_writer = new SimpleBeamWriter();
  
      if (!beam_writer->init(radar_queue,
			     scan_strategy,
			     _params->max_diff_from_scan,
			     eov_strategy,
			     _params->max_legal_elev_drop,
			     median_filter,
			     _params->get_tilt_num_from_header,
			     _params->get_vol_num_from_header,
			     _params->get_beam_time_from_header,
			     _params->debug))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Could not initialize beam writer object" << endl;
    
	return false;
      }

      _beamWriter = beam_writer;
    }
    break;
  
    case Params::OUTPUT_SCAN_STRATEGY :
    {
      TiltNumBeamWriter *beam_writer = new TiltNumBeamWriter();
  
      if (!beam_writer->init(radar_queue,
			     scan_strategy,
			     _params->max_diff_from_scan,
			     eov_strategy,
			     _params->max_legal_elev_drop,
			     median_filter,
			     _params->get_tilt_num_from_header,
			     _params->get_vol_num_from_header,
			     _params->get_beam_time_from_header,
			     _params->debug))
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Could not initialize beam writer object" << endl;
    
	return false;
      }

      _beamWriter = beam_writer;
    }
    break;
    } /* endswitch - _params->output_type */
  }
  
  case Params::ARC_HIQ_MSGS :
    ArcBeamWriter *beam_writer =
      new ArcBeamWriter(_params->get_beam_time_from_header);
    beam_writer->init(radar_queue, _params->debug);
    
    _beamWriter = beam_writer;
    
    break;
  }
  
  if (!_beamWriter->initRadarParams(_params->radar_id,
				    _params->scan_type_name,
				    _params->dynamic_origin_filename,
				    _params->dynamic_heading_filename,
				    _params->use_dynamic_heading,
			    (_params->override_origin_mode == Params::USE_DYNAMIC_OVERRIDE)))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not initialize beam writer radar parameters" << endl;
    
    return false;
  }
   
  if (!_beamWriter->initFieldParams(_params->dbz_scaling_info.scale,
				    _params->dbz_scaling_info.bias,
				    _params->vel_scaling_info.scale,
				    _params->vel_scaling_info.bias,
				    _params->sw_scaling_info.scale,
				    _params->sw_scaling_info.bias,
				    _params->dbz_scaling_info.scale,
				    _params->dbz_scaling_info.bias,
				    _params->ncp_scaling_info.scale,
				    _params->ncp_scaling_info.bias,
				    _params->power_scaling_info.scale,
				    _params->power_scaling_info.bias))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not initialize beam writer field parameters" << endl;
    
    return false;
  }
  
  switch(_params->override_origin_mode) {
    case Params::USE_HEADER_ORIGIN:  break;
    case Params::USE_STATIC_OVERRIDE:
	_beamWriter->setLatLonOverride(_params->radar_location.latitude,
	   _params->radar_location.longitude,
	   _params->radar_location.altitude);
   
          _beamWriter->setDiagnostics(_params->print_summary, _params->summary_interval);
     break;
     case Params::USE_DYNAMIC_OVERRIDE:
	  _beamWriter->loadOriginOverride();
     break;
  } 


  _beamWriter->setDiagnostics(_params->print_summary,
			      _params->summary_interval);

  return true;
}


/*********************************************************************
 * _initEovStrategy() - Initialize the end-of-volume strategy object.
 *
 * Returns a pointer to the new EOV strategy on success, 0 on failure.
 */

EOVStrategy *Hiq2Dsr::_initEovStrategy(ScanStrategy &scan_strategy)
{
    EOVStrategy *eov_strategy = 0;

    switch (_params->eov_trigger_mode)
    {
    case Params::EOV_TRIGGER_BY_ANGLE_DROP :
    {
      DropEOVStrategy *drop_strategy =
	new DropEOVStrategy(_params->end_of_volume_trigger);
      eov_strategy = drop_strategy;
      break;
    }

    case Params::EOV_TRIGGER_BY_END_TILT :
    {
      EndEOVStrategy *end_strategy =
	new EndEOVStrategy(_params->end_of_volume_trigger,
			   scan_strategy);
      eov_strategy = end_strategy;
      break;
    }
    
    case Params::EOV_TRIGGER_BY_START_TILT :
    {
      StartEOVStrategy *start_strategy =
	new StartEOVStrategy(_params->end_of_volume_trigger,
			     scan_strategy);
      eov_strategy = start_strategy;
      break;
    }
    } /* endswitch - _params->eov_trigger_mode */

    return eov_strategy;
}


/*********************************************************************
 * _initReader() - Initialize the reader object.
 *
 * Returns true on success, false on failure.
 */

bool Hiq2Dsr::_initReader(void)
{
  static const string method_name = "Hiq2Dsr::_initReader()";
  
  Reader *data_reader = 0;

  switch (_params->input_type)
  {
  case Params::UDP:
  {
    UdpReader *udp_reader =
      new UdpReader(_params->port, _params->debug_hiq);

    if (!udp_reader->init())
      return false;

    data_reader = udp_reader;
    break;
  }

  case Params::RAP_ARCHIVE_FILE:
  {
    RapFileReader *file_reader = new RapFileReader(_params->debug);

    vector< string > file_list;

    for (int i = 0; i < _params->input_files_n; ++i)
      file_list.push_back(_params->_input_files[i]);

    if (!file_reader->init(file_list,
			   _params->input_msgs_per_sec))
      return false;

    data_reader = file_reader;
    break;
  }
  
  } /* endswitch - _params->input_type */

  // Set up archivers

  if (_params->archive_file)
  {
    SingleFileArchiver *archiver = new SingleFileArchiver(_params->debug);

    if (archiver->init(_params->archive_file_path))
    {
      data_reader->addArchiver(archiver);
    }
    else
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error initializing archive file: "
	   << _params->archive_file_path << endl;
      cerr << "Data won't be archived" << endl;
    }
  } 

  if (_params->archive_mult_file)
  {
    MultFileArchiver *archiver = new MultFileArchiver(_params->debug);

    if (archiver->init(_params->archive_mult_file_dir))
    {
      data_reader->addArchiver(archiver);
    }
    else
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error initializing multiple file archive: "
	   << _params->archive_mult_file_dir << endl;
      cerr << "Data won't be archived" << endl;
    }
  } 

  if (_params->archive_fmq)
  {
    FmqArchiver *archiver = new FmqArchiver(_params->debug);

    if (archiver->init(_params->archive_fmq_url,
		       _progName,
		       (bool)_params->archive_fmq_compress,
		       _params->archive_fmq_nslots,
		       _params->archive_fmq_size))
    {
      data_reader->addArchiver(archiver);
    }
    else
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Error initializing archive FMQ: "
	   << _params->archive_fmq_url << endl;
      cerr << "Data won't be archived" << endl;
    }
  }

  // Create the appropriate message factory

  MsgFactory *msg_factory;
  
  switch (_params->msg_type)
  {
  case Params::EOL_HIQ_MSGS :
    msg_factory = new EolMsgFactory();
    break;
    
  case Params::ARC_HIQ_MSGS :
    msg_factory = new ArcMsgFactory();
    break;
  }
  
  // Initialize the message reader

  switch (_params->msg_type)
  {
  case Params::EOL_HIQ_MSGS :
    _reader = new EolHiqReader(msg_factory, _params->debug_hiq);
    break;
    
  case Params::ARC_HIQ_MSGS :
    _reader = new ArcHiqReader(msg_factory, _params->debug_hiq);
    break;
  }
  
  if (!_reader->init(data_reader))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error initializing HiQ message reader object" << endl;

    return false;
  }

  return true;
}


/*********************************************************************
 * _processData() - Process data for the given time.
 *
 * Returns true on success, false on failure.
 */

bool Hiq2Dsr::_processData(void)
{
  static const string method_name = "Hiq2Dsr::_processData()";
  
  // Read raw data

  PMU_auto_register("Reading raw beam data");

  HiqMsg *hiq_msg;

  if ((hiq_msg = _reader->getNextMsg()) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input message" << endl;

    return false;
  }

  if (_params->debug_hiq)
    hiq_msg->print(cerr);

  if (_params->debug_hiq_summary)
    hiq_msg->printSummary(cerr);
  
  // Make sure we can process this message

  if (hiq_msg->getMsgType() == HiqMsg::UNKNOWN_MSG)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Unknown message type (" << hiq_msg->getMsgType()
	 << ") received" << endl;
    cerr << "Skipping message" << endl;

    delete hiq_msg;
  }
  
  // Process the message.  We give control of the hiq_msg pointer to the
  // BeamWriter object which will delete the pointer when the message isn't
  // needed any longer.

  if (!_beamWriter->processMsg(hiq_msg))
    return false;
  
  return true;
}
