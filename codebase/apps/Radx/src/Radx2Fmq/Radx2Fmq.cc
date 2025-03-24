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
// Radx2Fmq.cc
//
// Radx2Fmq object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2025
//
///////////////////////////////////////////////////////////////
//
// Radx2Fmq reads radial moments data from Radx-supported files
// and writes the data to a DsRadarQueue beam by beam in Radx format.
//
////////////////////////////////////////////////////////////////

#include <vector>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/toolsa_macros.h>
#include <rapformats/DsRadarMsg.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxStatusXml.hh>
#include <Radx/RadxEvent.hh>
#include <Radx/RadxPath.hh>
#include "Radx2Fmq.hh"
using namespace std;

// Constructor

Radx2Fmq::Radx2Fmq(int argc, char **argv)

{

  isOK = true;
  _input = NULL;

  // set programe name
  
  _progName = "Radx2Fmq";
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

  // check that start and end time is set in archive mode
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // create the data input object

  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_latest_data_info,
			     _params.latest_file_only);
    _input->setFileQuiescence(_params.realtime_file_quiescence);
    _input->setDirScanSleep(_params.realtime_wait_between_scans);
    _input->setRecursion(_params.realtime_search_recursively);
    _input->setMaxRecursionDepth(_params.realtime_max_recursion_depth);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // initialize the output queue

  if (_openOutputFmq()) {
    isOK = false;
    return;
  }

  return;

}

// destructor

Radx2Fmq::~Radx2Fmq()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Radx2Fmq::Run()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.do_simulate) {
    return _runSimulate();
  }

  // loop until end of data
  
  _input->reset();
  char *input_file_path;
  
  while ((input_file_path = _input->next()) != NULL) {
    
    PMU_auto_register("In main loop");
    
    if (_processFile(input_file_path)) {
      cerr << "ERROR - Radx2Fmq::Run" << endl;
      cerr << "  Cannot process file: " << input_file_path << endl;
    }

  } // while
  
  return iret;

}

//////////////////////////////////////////////////
// Run in simulate mode

int Radx2Fmq::_runSimulate()
{
  
  int iret = 0;
  
  // loop until end of data

  while (true) {

    _input->reset();
    char *input_file_path;
    
    while ((input_file_path = _input->next()) != NULL) {
      
      PMU_auto_register("_runSimulate");
      
      if (_processFile(input_file_path)) {
        cerr << "ERROR - Radx2Fmq::Run" << endl;
        cerr << "  Cannot process file: " << input_file_path << endl;
      }
      
    } // while((input_file_path ...

  } // while (true)
  
  return iret;

}

//////////////////////////////////////////////////
// process this file

int Radx2Fmq::_processFile(const string filePath)
  
{

  // check we have not already processed this file
  // in the file aggregation step

  RadxPath thisPath(filePath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << filePath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "Processing file: " << filePath << endl;
  }

  // set up RadxFile object
  
  GenericRadxFile file;
  _setupRead(file);
  
  // read in file

  RadxVol vol;
  if (file.readFromPath(filePath, vol)) {
    cerr << "ERROR - Radx2Fmq::Run" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  _readPaths = file.getReadPaths();
  if (_params.debug) {
    cerr << "Following file paths used: " << endl;
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "    " << _readPaths[ii] << endl;
    }
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    vol.printWithRayMetaData(cerr);
    // vol.printWithFieldData(cerr);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    vol.print(cerr);
  }
  
  // load fields onto volume, forcing all rays to
  // have all fields

  vol.loadFieldsFromRays(true);
  
  // put start of volume

  int iret = 0;
  _putStartOfVolume(vol);

  // put platform, calibs and status
  
  if (_writePlatform(vol)) {
    iret = -1;
  }
  if (_writeStatusXml(vol)) {
    iret = -1;
  }
  if (_writeCalibs(vol)) {
    iret = -1;
  }

  // loop through the sweeps
  
  for (int ii = 0; ii < (int) vol.getSweeps().size(); ii++) {
    if (_processSweep(vol, ii)) {
      iret = -1;
    }
  }

  // put end of volume

  _putEndOfVolume(vol);

  return iret;

}

//////////////////////////////////////////////////
// process a sweep

int Radx2Fmq::_processSweep(const RadxVol &vol,
                            int sweepIndex)

{
  
  const RadxSweep &sweep = *vol.getSweeps()[sweepIndex];

  // put start of sweep

  const RadxRay &startRay = *(vol.getRays()[sweep.getStartRayIndex()]);
  _putStartOfSweep(startRay);
  
  // write rays
  
  for (size_t ii = sweep.getStartRayIndex();
       ii <= sweep.getEndRayIndex(); ii++) {
    if (_writeRay(*vol.getRays()[ii])) {
      return -1;
    }
  }

  // put end of sweep

  const RadxRay &endRay = *(vol.getRays()[sweep.getEndRayIndex()]);
  _putEndOfSweep(endRay);

  return 0;

}
  
//////////////////////////////////////////////////
// set up read 

void Radx2Fmq::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.read_set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.read_lower_fixed_angle,
                                 _params.read_upper_fixed_angle);
  } else if (_params.read_set_sweep_num_limits) {
    file.setReadSweepNumLimits(_params.read_lower_sweep_num,
                               _params.read_upper_sweep_num);
  }

  if (_params.read_set_field_names) {
    for (int i = 0; i < _params.read_field_names_n; i++) {
      file.addReadField(_params._read_field_names[i]);
    }
  }

  if (_params.aggregate_sweep_files_on_read) {
    file.setReadAggregateSweeps(true);
  } else {
    file.setReadAggregateSweeps(false);
  }

  if (_params.ignore_idle_scan_mode_on_read) {
    file.setReadIgnoreIdleMode(true);
  } else {
    file.setReadIgnoreIdleMode(false);
  }

  if (_params.remove_rays_with_all_data_missing) {
    file.setReadRemoveRaysAllMissing(true);
  }

  if (_params.remove_long_range_rays) {
    file.setReadRemoveLongRange(true);
  }

  if (_params.remove_short_range_rays) {
    file.setReadRemoveShortRange(true);
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.printReadRequest(cerr);
  }
  
}

///////////////////////////////////////////////////////////
// open output fmq

int Radx2Fmq::_openOutputFmq()
  
{

  if (_outputFmq != NULL) {
    delete _outputFmq;
  }

  _outputFmq = new DsFmq;
  
  // initialize the output queue
  
  if (_outputFmq->init(_params.output_url,
                       _progName.c_str(),
                       _params.debug >= Params::DEBUG_VERBOSE,
                       DsFmq::READ_WRITE, DsFmq::END,
                       false,
                       _params.output_n_slots,
                       _params.output_buf_size)) {
    cerr << "WARNING - trying to create new fmq" << endl;
    if (_outputFmq->init(_params.output_url,
                         _progName.c_str(),
                         _params.debug >= Params::DEBUG_VERBOSE,
                         DsFmq::CREATE, DsFmq::START,
                         false,
                         _params.output_n_slots,
                         _params.output_buf_size)) {
      cerr << "ERROR - " << _progName << "::openOutputFmq" << endl;
      cerr << "  Cannot open output radx fmq, URL: " << _params.output_url << endl;
      return -1;
    }
  }
  
  if (_params.write_blocking) {
    _outputFmq->setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _outputFmq->setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }
  
  return 0;

}

////////////////////////////////////////
// Write radar and field params to queue
//
// Returns 0 on success, -1 on failure

int Radx2Fmq::_writePlatform(const RadxVol &vol)

{

  RadxPlatform platform = vol.getPlatform();
  
  RadxMsg msg;
  platform.serialize(msg);
  
  // write the message
  
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - Radx2Fmq::_writePlatform" << endl;
    cerr << "  Cannot write platform to queue" << endl;
    // reopen the queue
    if (_openOutputFmq()) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// Write radar calib to queue
// Returns 0 on success, -1 on failure

int Radx2Fmq::_writeCalibs(const RadxVol &vol)

{

  const vector<RadxRcalib *> &rcalibs = vol.getRcalibs();

  for (size_t ii = 0; ii < rcalibs.size(); ii++) {

    RadxRcalib rcalib = *rcalibs[ii];
    
    // create message
    
    RadxMsg msg;
    rcalib.serialize(msg);
    
    // write the message
    
    if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                             msg.assembledMsg(), msg.lengthAssembled())) {
      cerr << "ERROR - Radx2Fmq::_writeCalibs" << endl;
      cerr << "  Cannot write calib to queue" << endl;
      // reopen the queue
      if (_openOutputFmq()) {
        return -1;
      }
    }
    
  } // ii

  return 0;

}

////////////////////////////////////////
// Write status XML to queue
// Returns 0 on success, -1 on failure

int Radx2Fmq::_writeStatusXml(const RadxVol &vol)

{

  // create RadxStatusXml object

  RadxStatusXml status;
  status.setXmlStr(vol.getStatusXml());
  
  // create message
  
  RadxMsg msg;
  status.serialize(msg);
  
  // write the message
  
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - Radx2Fmq::_writeStatusXml" << endl;
    cerr << "  Cannot write status xml to queue" << endl;
    // reopen the queue
    if (_openOutputFmq()) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// Write beam data to queue
//
// Returns 0 on success, -1 on failure

int Radx2Fmq::_writeRay(RadxRay &ray)

{

  // create message
  
  RadxMsg msg;
  ray.serialize(msg);
  
  // write the message
  
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - Radx2Fmq::_writeRay" << endl;
    cerr << "  Cannot write ray to queue" << endl;
    // reopen the queue
    if (_openOutputFmq()) {
      return -1;
    }
  }
  
  return 0;

}

////////////////////////////////////////
// put volume flags

void Radx2Fmq::_putStartOfVolume(const RadxVol &vol)
{

  // create event
  
  RadxEvent event;

  RadxTime startTime = vol.getStartRadxTime();
  event.setTime(startTime.utime(), startTime.getSubSec() * 1.0e9);

  event.setStartOfVolume(true);
  event.setVolumeNumber(vol.getVolumeNumber());

  const vector<RadxRay *> &rays = vol.getRays();
  event.setSweepMode(rays[0]->getSweepMode());
  
  // create message

  RadxMsg msg;
  event.serialize(msg);
  
  // write the message
  
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - Radx2Fmq::_putStartOfVolume" << endl;
    cerr << "  Cannot write start of vol event to queue" << endl;
    // reopen the queue
    _openOutputFmq();
  }

}

void Radx2Fmq::_putEndOfVolume(const RadxVol &vol)
{
  
  // create event
  
  RadxEvent event;
  RadxTime endTime = vol.getEndRadxTime();
  event.setTime(endTime.utime(), endTime.getSubSec() * 1.0e9);

  event.setEndOfVolume(true);
  event.setVolumeNumber(vol.getVolumeNumber());

  const vector<RadxRay *> &rays = vol.getRays();
  event.setSweepMode(rays[rays.size()-1]->getSweepMode());
  
  // create message

  RadxMsg msg;
  event.serialize(msg);
  
  // write the message
  
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - Radx2Fmq::_putEndOfVolume" << endl;
    cerr << "  Cannot write end of vol event to queue" << endl;
    // reopen the queue
    _openOutputFmq();
  }

}

void Radx2Fmq::_putStartOfSweep(const RadxRay &ray)
{

  // create event
  
  RadxEvent event;
  event.setTime(ray.getTimeSecs(), ray.getNanoSecs());
  event.setStartOfSweep(true);
  event.setSweepNumber(ray.getSweepNumber());
  event.setSweepMode(ray.getSweepMode());

  // create message

  RadxMsg msg;
  event.serialize(msg);
  
  // write the message
  
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - Radx2Fmq::_putStartOfSweep" << endl;
    cerr << "  Cannot write start of sweep event to queue" << endl;
    // reopen the queue
    _openOutputFmq();
  }

}

void Radx2Fmq::_putEndOfSweep(const RadxRay &ray)
{

  // create event
  
  RadxEvent event;
  event.setTime(ray.getTimeSecs(), ray.getNanoSecs());
  event.setEndOfSweep(true);
  event.setSweepNumber(ray.getSweepNumber());
  event.setSweepMode(ray.getSweepMode());

  // create message

  RadxMsg msg;
  event.serialize(msg);
  
  // write the message
  
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - Radx2Fmq::_putEndOfSweep" << endl;
    cerr << "  Cannot write end of sweep event to queue" << endl;
    // reopen the queue
    _openOutputFmq();
  }

}

void Radx2Fmq::_putNewSweepMode(Radx::SweepMode_t sweepMode, const RadxRay &ray)
{
  
  // create event
  
  RadxEvent event;
  event.setTime(ray.getTimeSecs(), ray.getNanoSecs());
  event.setSweepMode(sweepMode);
  
  // create message
  
  RadxMsg msg;
  event.serialize(msg);
  
  // write the message
  
  if (_outputFmq->writeMsg(msg.getMsgType(), msg.getSubType(),
                           msg.assembledMsg(), msg.lengthAssembled())) {
    cerr << "ERROR - Radx2Fmq::_putNewSweepMode" << endl;
    cerr << "  Cannot write new sweep mode event to queue" << endl;
    // reopen the queue
    _openOutputFmq();
  }

}

