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
// Dsr2Radx.cc
//
// Dsr2Radx object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////
//
// Dsr2Radx reads an input radar FMQ, and saves out data files
// in formats supported by Radx.
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/TaArray.hh>
#include <toolsa/udatetime.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <dsserver/DsLdataInfo.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/DoradeRadxFile.hh>
#include <Radx/UfRadxFile.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxGeoref.hh>

#include "Dsr2Radx.hh"

using namespace std;

const double Dsr2Radx::_smallVal = 0.0001;
const double Dsr2Radx::_pseudoDiam = 17066.0;

// Constructor

Dsr2Radx::Dsr2Radx(int argc, char **argv)

{

  isOK = true;
  _clearData();
  _nCheckPrint = 0;
  _nWarnCensorPrint = 0;

  _prevElevMoving = -999;
  _prevAzMoving = -999;

  _lutRadarAltitudeKm = -9999;

  _prevVolNum = -99999;
  _prevSweepNum = -1;
  _endOfVol = false;
  _endOfVolTime = -1;
  _antenna = NULL;
  _outFile = NULL;
  _sweepMgr = NULL;
  _prevRay = NULL;

  _scanMode = SCAN_MODE_UNKNOWN;
  _scanInfoFromHeaders = false;

  _isSolarScan = false;

  // set programe name

  _progName = "Dsr2Radx";
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
  }

  // create antenna angle manager
  
  _antenna = new Antenna(_progName, _params);
  _endOfVolAutomatic =
    (_params.end_of_vol_decision == Params::AUTOMATIC);
  
  // create sweep mamager

  _sweepMgr = new SweepMgr(_progName, _params);

  // set up volume object

  if (_params.debug) {
    _vol.setDebug(true);
  }

  _vol.setTitle(_params.ncf_title);
  _vol.setInstitution(_params.ncf_institution);
  _vol.setReferences(_params.ncf_references);
  _vol.setSource(_params.ncf_source);
  _vol.setHistory(_params.ncf_history);
  _vol.setComment(_params.ncf_comment);

  // override missing values

  if (_params.override_missing_metadata_values) {
    Radx::setMissingMetaDouble(_params.missing_metadata_double);
    Radx::setMissingMetaFloat(_params.missing_metadata_float);
    Radx::setMissingMetaInt(_params.missing_metadata_int);
    Radx::setMissingMetaChar(_params.missing_metadata_char);
  }
  if (_params.override_missing_field_values) {
    Radx::setMissingFl64(_params.missing_field_fl64);
    Radx::setMissingFl32(_params.missing_field_fl32);
    Radx::setMissingSi32(_params.missing_field_si32);
    Radx::setMissingSi16(_params.missing_field_si16);
    Radx::setMissingSi08(_params.missing_field_si08);
  }

  // set up output file object

  _outFile = new RadxFile();
  _setupWrite();

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.procmap_register_interval);

}

/////////////////////////////////////////////////////////
// destructor

Dsr2Radx::~Dsr2Radx()

{

  _clearData();

  if (_outFile) {
    delete _outFile;
  }

  if (_antenna != NULL) {
    delete _antenna;
  }

  if (_sweepMgr != NULL) {
    delete _sweepMgr;
  }

  for (int icalib = 0; icalib < (int) _calibs.size(); icalib++) {
    delete _calibs[icalib];
  }
  _calibs.clear();


  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// set up write

void Dsr2Radx::_setupWrite()
{

  if (_params.debug) {
    _outFile->setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outFile->setVerbose(true);
  }

  if (_params.output_compressed) {
    _outFile->setWriteCompressed(true);
    _outFile->setCompressionLevel(_params.output_compression_level);
  } else {
    _outFile->setWriteCompressed(false);
  }
  if (_params.output_native_byte_order) {
    _outFile->setWriteNativeByteOrder(true);
  } else {
    _outFile->setWriteNativeByteOrder(false);
  }

  switch (_params.netcdf_style) {
    case Params::NETCDF4:
      _outFile->setNcFormat(RadxFile::NETCDF4);
      break;
    case Params::OFFSET_64BIT:
      _outFile->setNcFormat(RadxFile::NETCDF_OFFSET_64BIT);
      break;
    case Params::NETCDF4_CLASSIC:
      _outFile->setNcFormat(RadxFile::NETCDF4_CLASSIC);
      break;
    case Params::CLASSIC:
    default:
      _outFile->setNcFormat(RadxFile::NETCDF_CLASSIC);
  }

  if (strlen(_params.output_filename_prefix) > 0) {
    _outFile->setWriteFileNamePrefix(_params.output_filename_prefix);
  }
  _outFile->setWriteInstrNameInFileName(_params.include_instrument_name_in_file_name);
  _outFile->setWriteSiteNameInFileName(_params.include_site_name_in_file_name);
  _outFile->setWriteSubsecsInFileName(_params.include_subsecs_in_file_name);
  _outFile->setWriteScanTypeInFileName(_params.include_scan_type_in_file_name);
  _outFile->setWriteVolNumInFileName(_params.include_vol_num_in_file_name);
  _outFile->setWriteHyphenInDateTime(_params.use_hyphen_in_file_name_datetime_part);

}

//////////////////////////////////////////////////
// Run

int Dsr2Radx::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  int iret = 0;
  while (true) {
    iret = _run();
    return iret;
    if (_params.debug) {
      cerr << "Dsr2Radx::Run:" << endl;
      cerr << "  Trying to contact input server at url: "
	   << _params.input_fmq_url << endl;
    }
    umsleep(1000);
  }

  return iret;

}

//////////////////////////////////////////////////
// _run

int Dsr2Radx::_run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // clear all data from previously

  _clearData();

  // Instantiate and initialize the DsRadar queue and message

  DsRadarQueue radarQueue;
  DsRadarMsg radarMsg;

  if (_params.seek_to_end_of_input) {
    if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			_params.debug >= Params::DEBUG_VERBOSE,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::END )) {
      fprintf(stderr, "ERROR - %s:Dsr2Radx::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  } else {
    if (radarQueue.init(_params.input_fmq_url, _progName.c_str(),
			_params.debug >= Params::DEBUG_VERBOSE,
			DsFmq::BLOCKING_READ_WRITE, DsFmq::START )) {
      fprintf(stderr, "ERROR - %s:Dsr2Radx::_run\n", _progName.c_str());
      fprintf(stderr, "Could not initialize radar queue '%s'\n",
	      _params.input_fmq_url);
      return -1;
    }
  }
  
  // read beams from the queue and process them
  
  time_t timeLastMsg = time(NULL);
  int iret = 0;

  while (true) {

    PMU_auto_register("Reading radar FMQ");

    bool gotMsg = false;
    if (_readMsg(radarQueue, radarMsg, gotMsg)) {
      umsleep(100);
    }

    if (gotMsg) {
      timeLastMsg = time(NULL);
    } else {
      umsleep(100);
      if (_nRaysRead > _params.min_rays_in_vol &&
	  _params.write_end_of_vol_when_data_stops) {
        time_t now = time(NULL);
        int timeSinceLastMsg = now - timeLastMsg;
        if (timeSinceLastMsg > _params.nsecs_no_data_for_end_of_vol) {
	  _endOfVol = true;
          timeLastMsg = now;
	}
      }
      if (!_endOfVol) {
	continue;
      }
    }
    
    // at the end of a volume, process volume, then reset
    
    if (_endOfVol) {
      if (_params.debug) {
        if (_params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_NUM) {
          cerr << "=========== End of sweep ===============" << endl;
        } else {
          cerr << "=========== End of volume ==============" << endl;
        }
	cerr << "  nrays available:" << _vol.getNRays() << endl;
      }
      if (_processVol()) {
        iret = -1;
      }
      _clearData();
    }
    
  } // while (true)
  
  return iret;

}

////////////////////////////////////////////////////////////////////
// _readMsg()
//
// Read a message from the queue, setting the flags about ray_data
// and _endOfVolume appropriately.
//
// Sets gotMsg to true if message was read
//

int Dsr2Radx::_readMsg(DsRadarQueue &radarQueue,
                       DsRadarMsg &radarMsg,
                       bool &gotMsg) 
  
{
  
   
  PMU_auto_register("Reading radar queue");
  _endOfVol = false;
  
  int msgContents = 0;
  if (radarQueue.getDsMsg(radarMsg, &msgContents, &gotMsg)) {
    return -1;
  }
  if (!gotMsg) {
    return -1;
  }
    
  // set radar parameters if avaliable
  
  if (msgContents & DsRadarMsg::RADAR_PARAMS) {
    _loadRadarParams(radarMsg);
  }

  // set input radar calibration if avaliable
  
  if (msgContents & DsRadarMsg::RADAR_CALIB) {
    _loadInputCalib(radarMsg);
  }

  // set status XML if avaliable
  
  if (msgContents & DsRadarMsg::STATUS_XML) {
    _vol.setStatusXml(radarMsg.getStatusXml());
  }

  // If we have radar and field params, and there is good ray data,
  // add to the vector

  RadxRay *ray = NULL;
  if ((msgContents & DsRadarMsg::RADAR_BEAM) && radarMsg.allParamsSet()) {

    _nRaysRead++;
    
    // crete ray from ray message
    
    ray = _createInputRay(radarMsg, msgContents);
    
    // censor the fields in the ray if requested
    
    _censorInputRay(ray);
    
    // prepare the ray for output, by trimming out any unnecessary fields
    // and setting the names etc
    
    _prepareRayForOutput(ray);
    
    // debug print
    
    if (_params.debug) {
      int nPrintFreq = 90;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        nPrintFreq = 1;
      }
      if ((_nRaysRead > 0) && (_nRaysRead % nPrintFreq == 0) &&
          (int) _vol.getNRays() != _nCheckPrint) {
        _nCheckPrint = (int) _vol.getNRays();
        cerr << "  nRays, latest time, el, az: "
             << _nRaysRead << ", "
             << utimstr(ray->getTimeSecs()) << ", "
             << ray->getElevationDeg() << ", "
             << ray->getAzimuthDeg() << endl;
      }
    }
    
    // end of vol condition
    
    if (_params.end_of_vol_decision == Params::ELAPSED_TIME) {
      if (_endOfVolTime < 0) {
        _computeEndOfVolTime(ray->getTimeSecs());
      }
      if (ray->getTimeSecs() >= _endOfVolTime) {
        _endOfVol = true;
        _computeEndOfVolTime(ray->getTimeSecs() + 1);
      }
    }
    
    int volNum = ray->getVolumeNumber();
    if (volNum != -1 && volNum != _prevVolNum) {
      if (_prevVolNum != -99999 &&
          _params.end_of_vol_decision == Params::CHANGE_IN_VOL_NUM) {
        _endOfVol = true;
      }
      if (_prevVolNum != -99999 &&
          _params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_NUM) {
        _endOfVol = true;
      }
      _prevVolNum = volNum;
    }
    
    int sweepNum = ray->getSweepNumber();
    if (sweepNum >= 0 && sweepNum != _prevSweepNum) {
      if (_prevSweepNum >= 0 &&
          _params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_NUM) {
        _endOfVol = true;
      }
      _prevSweepNum = sweepNum;
    }
    
    if (_endOfVolAutomatic) {
      if (_antenna->addRay(ray)) {
        _endOfVol = true;
      }
    }
    
  } // if (msgContents ...
  
  // is this an end of vol?
  
  if ((int) _vol.getNRays() > _params.max_rays_in_vol) {
    _endOfVol = true;
  }

  if (!_endOfVolAutomatic &&
      _params.end_of_vol_decision != Params::CHANGE_IN_VOL_NUM &&
      (msgContents & DsRadarMsg::RADAR_FLAGS)) {

    const DsRadarFlags &flags = radarMsg.getRadarFlags();

    if (_params.end_of_vol_decision == Params::END_OF_VOL_FLAG &&
        flags.endOfVolume) {

      _endOfVol = true;
      
    } else if (_params.end_of_vol_decision == Params::LAST_SWEEP_IN_VOL &&
               flags.endOfTilt &&
               flags.tiltNum == _params.last_sweep_in_vol) {
      
      _endOfVol = true;

    } else if (flags.newScanType) {

      _endOfVol = true;

    }

  }
  
  // add the ray to the volume as appropriate
  // check for transitions

  if (ray != NULL) {
    if (_antenna->getInTransition()) {
      ray->setAntennaTransition(true);
    }
    bool ignoreRay = false;
    if (_params.filter_antenna_transitions && _antenna->getInTransition()) {
      ignoreRay = true;
    }
    if (ignoreRay) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Transition, ignoring, el, az: "
             << ray->getElevationDeg() << ", "
             << ray->getAzimuthDeg() << endl;
      }
      delete ray;
    } else {
      if (_prevRay) {
        _addRayToVol(_prevRay);
        _prevRay = NULL;
      }
      if (_endOfVol) {
        if (_endOfVolAutomatic ||
            _params.end_of_vol_decision == Params::CHANGE_IN_VOL_NUM ||
            _params.end_of_vol_decision == Params::CHANGE_IN_SWEEP_NUM) {
          _prevRay = ray;
        } else {
          _addRayToVol(ray);
        }
      } else {
        _addRayToVol(ray);
      }
    }
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// process the volume

int Dsr2Radx::_processVol()
  
{

  int iret = 0;

  PMU_force_register("Processing volume");
  
  if (_params.debug) {
    cerr << "**** Start Dsr2Radx::_processVol() ****" << endl;
  }

  // load the current scan mode - PPI or RHI

  _loadCurrentScanMode();

  // set the sweep numbers in the input rays, if needed
  
  if (_sweepNumbersMissing || _params.find_sweep_numbers_using_histogram) {
    _sweepMgr->setSweepNumbers(_scanMode == SCAN_MODE_RHI, _vol.getRays());
  }
  
  // load up rays from ray data
  
  bool notEnoughRays = false;
  if (_scanMode == SCAN_MODE_RHI) {
    if ((int) _vol.getNRays() < _params.min_rays_per_rhi_vol) {
      notEnoughRays = true;
    }
  } else {
    if ((int) _vol.getNRays() < _params.min_rays_per_ppi_vol) {
      notEnoughRays = true;
    }
  }
  if (notEnoughRays) {
    cerr << "WARNING - Dsr2Radx::_processVol()" << endl;
    cerr << "  Too few rays: " << _vol.getNRays() << endl;
    cerr << "  skipping volume" << endl;
    _clearData();
    return 0;
  }

  // set the volume info from the rays
  
  _vol.loadVolumeInfoFromRays();

  // set the sweep numbers from the rays

  if (_params.increment_sweep_num_when_pol_mode_changes) {
    _vol.incrementSweepOnPolModeChange();
  }

  if (_params.increment_sweep_num_when_prt_mode_changes) {
    _vol.incrementSweepOnPrtModeChange();
  }

  bool isRhi = _vol.checkIsRhi();
  bool isSurveillance = false;
  if (!isRhi) {
    isSurveillance = _vol.checkIsSurveillance();
  }
  
  if (isRhi) {
    // RHI
    if (_params.adjust_rhi_sweep_limits_using_angles) {
      _vol.adjustSweepLimitsUsingAngles();
    } else {
      _vol.loadSweepInfoFromRays();
    }
  } else if (isSurveillance) {
    // SUR
    if (_params.force_sur_sweep_transitions_at_fixed_azimuth) {
      _vol.adjustSurSweepLimitsToFixedAzimuth
        (_params. sur_sweep_transitions_azimuth_deg);
    } else if (_params.adjust_sur_sweep_limits_using_angles) {
      _vol.adjustSweepLimitsUsingAngles();
    } else {
      _vol.loadSweepInfoFromRays();
    }
    if (_params.trim_surveillance_sweeps_to_360deg) {
      _vol.trimSurveillanceSweepsTo360Deg();
    }
  } else {
    // sector
    if (_params.adjust_sector_sweep_limits_using_angles) {
      _vol.adjustSweepLimitsUsingAngles();
    } else {
      _vol.loadSweepInfoFromRays();
    }
  }

  if (isRhi) {
    if (_params.compute_rhi_fixed_angles_from_measured_azimuth) {
      _vol.computeSweepFixedAnglesFromRays();
    }
  } else{
    if (_params.compute_ppi_fixed_angles_from_measured_elevation) {
      _vol.computeSweepFixedAnglesFromRays();
    }
  }

  // load calibs in Radx
  
  _loadRadxRcalib();

  // set calibration indexes

  _vol.loadCalibIndexOnRays();

  // convert to common geometry

  if (_params.convert_to_predominant_gate_geometry) {
    _vol.remapToPredomGeom();
  } else {
    _vol.filterOnPredomGeom();
  }

  // check for indexed rays

  _vol.checkForIndexedRays();

  // trim sweeps with too few rays

  if (_params.check_min_rays_in_sweep) {
    _vol.removeSweepsWithTooFewRays(_params.min_rays_in_sweep);
  }

  // apply angle offsets

  if (_params.apply_azimuth_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying azimuth offset (deg): " 
           << _params.azimuth_offset << endl;
    }
    _vol.applyAzimuthOffset(_params.azimuth_offset);
  }
  if (_params.apply_elevation_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying elevation offset (deg): " 
           << _params.elevation_offset << endl;
    }
    _vol.applyElevationOffset(_params.elevation_offset);
  }

  // write the files

  if (_doWrite()) {
    iret = -1;
  }
  PMU_force_register("done writing");

  if (_params.debug) {
    cerr << "**** End Dsr2Radx::_processVol() ****" << endl;
  }

  return iret;

}

////////////////////////////////////////////////////////////////
// perform the write

int Dsr2Radx::_doWrite()
  
{

  PMU_force_register("_doWrite");
  
  int iret = 0;

  for (int idata = 0; idata < _params.output_data_set_n; idata++) {

    const Params::output_data_set_t &dset =
      _params._output_data_set[idata];
    
    string dataType = "nc";
    switch (dset.format) {
      case Params::OUTPUT_FORMAT_UF:
        _outFile->setFileFormat(RadxFile::FILE_FORMAT_UF);
        dataType = "uf";
        break;
      case Params::OUTPUT_FORMAT_FORAY:
        _outFile->setFileFormat(RadxFile::FILE_FORMAT_FORAY_NC);
        dataType = "foray";
        break;
      case Params::OUTPUT_FORMAT_DORADE:
        _outFile->setFileFormat(RadxFile::FILE_FORMAT_DORADE);
        dataType = "dorade";
        break;
      default:
        _outFile->setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
    }

    string outputDir = dset.output_dir;
    
    if (_params.separate_output_dirs_by_scan_type) {
      outputDir += PATH_DELIM;
      if (_isSolarScan) {
        // special case
        outputDir += _params.sun_subdir;
      } else {
        switch (_scanMode) {
          case SCAN_MODE_RHI:
            outputDir += _params.rhi_subdir;
            break;
          case SCAN_MODE_SECTOR:
            outputDir += _params.sector_subdir;
            break;
          case SCAN_MODE_VERT:
            outputDir += _params.vert_subdir;
            break;
          case SCAN_MODE_SUNSCAN:
          case SCAN_MODE_SUNSCAN_RHI:
            outputDir += _params.sun_subdir;
            break;
          case SCAN_MODE_PPI:
          default:
            outputDir += _params.surveillance_subdir;
        }
      }
    }
    
    // check nrays in vol for volume-type formats

    if (dset.format == Params::OUTPUT_FORMAT_UF ||
        dset.format == Params::OUTPUT_FORMAT_CFRADIAL) {
      if ((int) _vol.getNRays() < _params.min_rays_in_vol) {
        if (_params.debug) {
          cerr << "NOTE - Dsr2Radx::_doWrite()" << _vol.getNRays() << endl;
          cerr << "  dataType: " << dataType << endl;
          cerr << "  outputDir: " << outputDir << endl;
          cerr << "  too few rays, will not be saved: " << _vol.getNRays() << endl;
        }
        continue;
      }
    }

    // write out
    
    if (_outFile->writeToDir(_vol,
                             outputDir,
                             _params.append_day_dir_to_output_dir,
                             _params.append_year_dir_to_output_dir)) {

      cerr << "ERROR - Dsr2Radx::_doWrite()" << endl;
      cerr << _outFile->getErrStr() << endl;
      iret = -1;
      
    } else {

      // register the write with the DataMapper
      
      if (_params.write_individual_ldata_info) {
        for (size_t ipath = 0; ipath < _outFile->getWritePaths().size(); ipath++) {
          _writeLdataInfo(outputDir,
                          _outFile->getWritePaths()[ipath],
                          _outFile->getWriteDataTimes()[ipath],
                          dataType);
        }
      }
        
      // optionally register the master URL with the DataMapper
      
      if (_params.separate_output_dirs_by_scan_type &&
          _params.write_master_ldata_info) {
        for (size_t ipath = 0; ipath < _outFile->getWritePaths().size(); ipath++) {
          _writeLdataInfo(dset.output_dir,
                          _outFile->getWritePaths()[ipath],
                          _outFile->getWriteDataTimes()[ipath],
                          dataType);
        }
      }
      
    }

  } // idata

  return iret;

}

///////////////////////////////////////////////////
// write the master ldata info, as appropriate
// must call writeVol() successfully first
// Returns 0 on success, -1 on failure

int Dsr2Radx::_writeLdataInfo(const string &outputDir,
                              const string &outputPath,
                              time_t dataTime,
                              const string &dataType)
                                    

{

  DsLdataInfo ldata;
  ldata.setDir(outputDir);
  ldata.setDataType(dataType);
        
  // compute relative data path
  
  string relPath;
  RadxPath::stripDir(ldata.getDataDirPath(), outputPath, relPath);
  ldata.setRelDataPath(relPath);

  RadxPath writePath(outputPath);
  ldata.setDataFileExt(writePath.getExt());
  ldata.setWriter("Dsr2Radx");
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    ldata.setDebug(true);
  }

  if (ldata.write(dataTime)) {
    cerr << "WARNING - Dsr2Radx::_writeLdataInfo" << endl;
    cerr << "  Cannot write LdataInfo to dir: "
         << outputDir << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// load radar params

void Dsr2Radx::_loadRadarParams(const DsRadarMsg &radarMsg)

{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "=========>> got RADAR_PARAMS" << endl;
  }

  DsRadarParams *rparams = new DsRadarParams(radarMsg.getRadarParams());
  
  if (_params.override_radar_name) {
    _vol.setInstrumentName(_params.radar_name);
  } else {
    _vol.setInstrumentName(rparams->radarName);
  }

  // is this a solar?

  string solarScanName(_params.solar_scan_name);
  string scanName(rparams->scanTypeName);
  if (scanName == solarScanName) {
    _isSolarScan = true;
  } else {
    _isSolarScan = false;
  }

  _vol.setSiteName(_params.site_name);
  _vol.setScanName(scanName);

  switch (rparams->radarType) {
    case DS_RADAR_AIRBORNE_FORE_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    }
    case DS_RADAR_AIRBORNE_AFT_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    }
    case DS_RADAR_AIRBORNE_TAIL_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    }
    case DS_RADAR_AIRBORNE_LOWER_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    }
    case DS_RADAR_AIRBORNE_UPPER_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_ROOF);
      break;
    }
    case DS_RADAR_SHIPBORNE_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    }
    case DS_RADAR_VEHICLE_TYPE: {
      _vol.setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    }
    case DS_RADAR_GROUND_TYPE:
    default:{
      _vol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
    }
  }
  
  if (_params.override_radar_location) {
    _vol.overrideLocation(_params.radar_location.latitudeDeg,
                          _params.radar_location.longitudeDeg,
                          _params.radar_location.altitudeKm);
  } else {
    _vol.setLocation(rparams->latitude,
                     rparams->longitude,
                     rparams->altitude);
  }

  // check to make sure altitude is in km, not meters
  
  if (_vol.getAltitudeKm() > 8000.0 && _params.debug){
    cerr << "WARNING : Sensor altitude is "
         << _vol.getAltitudeKm() << "km" << endl;
    cerr << "  Are the right units being used for altitude?" << endl;
    cerr << "  Incorrect altitude results in bad cart remapping." << endl;
  }
  
  _vol.addWavelengthCm(rparams->wavelength);
  _vol.setRadarBeamWidthDegH(rparams->horizBeamWidth);
  _vol.setRadarBeamWidthDegV(rparams->vertBeamWidth);

}

////////////////////////////////////////////////////////////////
// load input radar calibrations

void Dsr2Radx::_loadInputCalib(const DsRadarMsg &radarMsg)

{

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "=========>> got RADAR_CALIB" << endl;
  }

  const DsRadarCalib calib = radarMsg.getRadarCalib();
  double thisPulseWidth = calib.getPulseWidthUs();
  
  if (calib.getAntGainDbH() > -9990) {
    _vol.setRadarAntennaGainDbH(calib.getAntGainDbH());
  }
  if (calib.getAntGainDbV() > -9990) {
    _vol.setRadarAntennaGainDbV(calib.getAntGainDbV());
  }

  // find calib with same pulse width
  
  bool newPulseWidth = true;
  for (int icalib = 0; icalib < (int) _calibs.size(); icalib++) {
    double prevPulseWidth = _calibs[icalib]->getPulseWidthUs();
    if (fabs(thisPulseWidth - prevPulseWidth) < 0.0001) {
      // same pulse width, so replace it
      DsRadarCalib *newCal = new DsRadarCalib(calib);
      delete _calibs[icalib];
      _calibs[icalib] = newCal;
      newPulseWidth = false;
      break;
    }
  }
  if (newPulseWidth) {
    DsRadarCalib *newCal = new DsRadarCalib(calib);
    _calibs.push_back(newCal);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "============= Radar Calibs ==============" << endl;
    for (int icalib = 0; icalib < (int) _calibs.size(); icalib++) {
      cerr << "--------------------------------------" << endl;
      cerr << "-->> pulse width: " << _calibs[icalib]->getPulseWidthUs() << endl;
      _calibs[icalib]->print(cerr);
      cerr << "--------------------------------------" << endl;
    }
    cerr << "=========================================" << endl;
  }

}

////////////////////////////////////////////////////////////////////
// load up calib in Radx object

void Dsr2Radx::_loadRadxRcalib()

{

  for (int icalib = 0; icalib < (int) _calibs.size(); icalib++) {

    const DsRadarCalib &calIn = *_calibs[icalib];
    RadxRcalib *calOut = new RadxRcalib();

    DateTime ctime(calIn.getCalibTime());

    calOut->setCalibTime(ctime.getYear(),
                         ctime.getMonth(),
                         ctime.getDay(),
                         ctime.getHour(),
                         ctime.getMin(),
                         ctime.getSec());

    calOut->setPulseWidthUsec(calIn.getPulseWidthUs());
    calOut->setXmitPowerDbmH(calIn.getXmitPowerDbmH());
    calOut->setXmitPowerDbmV(calIn.getXmitPowerDbmV());
    
    calOut->setTwoWayWaveguideLossDbH(calIn.getTwoWayWaveguideLossDbH());
    calOut->setTwoWayWaveguideLossDbV(calIn.getTwoWayWaveguideLossDbV());
    calOut->setTwoWayRadomeLossDbH(calIn.getTwoWayRadomeLossDbH());
    calOut->setTwoWayRadomeLossDbV(calIn.getTwoWayRadomeLossDbV());
    calOut->setReceiverMismatchLossDb(calIn.getReceiverMismatchLossDb());
    
    calOut->setRadarConstantH(calIn.getRadarConstH());
    calOut->setRadarConstantV(calIn.getRadarConstV());
    
    calOut->setNoiseDbmHc(calIn.getNoiseDbmHc());
    calOut->setNoiseDbmHx(calIn.getNoiseDbmHx());
    calOut->setNoiseDbmVc(calIn.getNoiseDbmVc());
    calOut->setNoiseDbmVx(calIn.getNoiseDbmVx());
    
    calOut->setReceiverGainDbHc(calIn.getReceiverGainDbHc());
    calOut->setReceiverGainDbHx(calIn.getReceiverGainDbHx());
    calOut->setReceiverGainDbVc(calIn.getReceiverGainDbVc());
    calOut->setReceiverGainDbVx(calIn.getReceiverGainDbVx());
    
    calOut->setReceiverSlopeDbHc(calIn.getReceiverSlopeDbHc());
    calOut->setReceiverSlopeDbHx(calIn.getReceiverSlopeDbHx());
    calOut->setReceiverSlopeDbVc(calIn.getReceiverSlopeDbVc());
    calOut->setReceiverSlopeDbVx(calIn.getReceiverSlopeDbVx());
    
    calOut->setBaseDbz1kmHc(calIn.getBaseDbz1kmHc());
    calOut->setBaseDbz1kmHx(calIn.getBaseDbz1kmHx());
    calOut->setBaseDbz1kmVc(calIn.getBaseDbz1kmVc());
    calOut->setBaseDbz1kmVx(calIn.getBaseDbz1kmVx());
    
    calOut->setSunPowerDbmHc(calIn.getSunPowerDbmHc());
    calOut->setSunPowerDbmHx(calIn.getSunPowerDbmHx());
    calOut->setSunPowerDbmVc(calIn.getSunPowerDbmVc());
    calOut->setSunPowerDbmVx(calIn.getSunPowerDbmVx());
    
    calOut->setNoiseSourcePowerDbmH(calIn.getNoiseSourcePowerDbmH());
    calOut->setNoiseSourcePowerDbmV(calIn.getNoiseSourcePowerDbmV());
    
    calOut->setPowerMeasLossDbH(calIn.getPowerMeasLossDbH());
    calOut->setPowerMeasLossDbV(calIn.getPowerMeasLossDbV());
    
    calOut->setCouplerForwardLossDbH(calIn.getCouplerForwardLossDbH());
    calOut->setCouplerForwardLossDbV(calIn.getCouplerForwardLossDbV());
    
    calOut->setZdrCorrectionDb(calIn.getZdrCorrectionDb());
    calOut->setLdrCorrectionDbH(calIn.getLdrCorrectionDbH());
    calOut->setLdrCorrectionDbV(calIn.getLdrCorrectionDbV());
    calOut->setSystemPhidpDeg(calIn.getSystemPhidpDeg());
    
    calOut->setTestPowerDbmH(calIn.getTestPowerDbmH());
    calOut->setTestPowerDbmV(calIn.getTestPowerDbmV());

    _vol.addCalib(calOut);

  } // icalib

}

////////////////////////////////////////////////////////////////////
// add an input ray from an incoming message

RadxRay *Dsr2Radx::_createInputRay(const DsRadarMsg &radarMsg, 
                                   int msgContents)

{

  // input data

  const DsRadarBeam &rbeam = radarMsg.getRadarBeam();
  const DsRadarParams &rparams = radarMsg.getRadarParams();
  const vector<DsFieldParams *> &fparamsVec = radarMsg.getFieldParams();

  // create new ray

  RadxRay *ray = new RadxRay;

  // set ray properties

  ray->setTime(rbeam.dataTime, rbeam.nanoSecs);
  ray->setVolumeNumber(rbeam.volumeNum);
  ray->setSweepNumber(rbeam.tiltNum);
  if (rbeam.tiltNum < 0) {
    _sweepNumbersMissing = true;
  }

  int scanMode = rparams.scanMode;
  if (rbeam.scanMode > 0) {
    scanMode = rbeam.scanMode;
  } else {
    scanMode = rparams.scanMode;
  }

  ray->setSweepMode(_getRadxSweepMode(scanMode));
  ray->setPolarizationMode(_getRadxPolarizationMode(rparams.polarization));
  ray->setPrtMode(_getRadxPrtMode(rparams.prfMode));
  ray->setFollowMode(_getRadxFollowMode(rparams.followMode));

  double elev = rbeam.elevation;
  if (elev > 180) {
    elev -= 360.0;
  }
  ray->setElevationDeg(elev);

  double az = rbeam.azimuth;
  if (az < 0) {
    az += 360.0;
  }
  ray->setAzimuthDeg(az);

  // range geometry

  int nGates = rparams.numGates;
  if (_params.remove_test_pulse) {
    nGates -= _params.ngates_test_pulse;
  }
  ray->setRangeGeom(rparams.startRange, rparams.gateSpacing);

  // optionally limit number of gates based on height
  // don't do this for solars

  if (_params.crop_above_max_height && !_isSolarScan) {
    double nGatesForMaxHt =
      _computeNgatesForMaxHt(elev,
                             rparams.altitude,
                             rparams.startRange,
                             rparams.gateSpacing);
    if (nGatesForMaxHt < nGates) {
      nGates = (int) (nGatesForMaxHt + 0.5);
    }
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Computing nGates based on max ht, elev, nGates: "
           << elev << ", " << nGates << endl;
    }
  }
  
  if (scanMode == DS_RADAR_RHI_MODE ||
      scanMode == DS_RADAR_EL_SURV_MODE) {
    ray->setFixedAngleDeg(rbeam.targetAz);
  } else {
    ray->setFixedAngleDeg(rbeam.targetElev);
  }

  ray->setIsIndexed(rbeam.beamIsIndexed);
  ray->setAngleResDeg(rbeam.angularResolution);
  ray->setAntennaTransition(rbeam.antennaTransition);
  ray->setNSamples(rbeam.nSamples);
  
  ray->setPulseWidthUsec(rparams.pulseWidth);
  double prt = 1.0 / rparams.pulseRepFreq;
  ray->setPrtSec(prt);
  ray->setPrtRatio(1.0);
  ray->setNyquistMps(rparams.unambigVelocity);

  ray->setUnambigRangeKm(Radx::missingMetaDouble);
  ray->setUnambigRange();

  ray->setMeasXmitPowerDbmH(rbeam.measXmitPowerDbmH);
  ray->setMeasXmitPowerDbmV(rbeam.measXmitPowerDbmV);

  // platform georeference
  
  if (msgContents & DsRadarMsg::PLATFORM_GEOREF) {
    const DsPlatformGeoref &platformGeoref = radarMsg.getPlatformGeoref();
    const ds_iwrf_platform_georef_t &dsGeoref = platformGeoref.getGeoref();
    RadxGeoref georef;
    georef.setTimeSecs(dsGeoref.packet.time_secs_utc);
    georef.setNanoSecs(dsGeoref.packet.time_nano_secs);
    georef.setLongitude(dsGeoref.longitude);
    georef.setLatitude(dsGeoref.latitude);
    georef.setAltitudeKmMsl(dsGeoref.altitude_msl_km);
    georef.setAltitudeKmAgl(dsGeoref.altitude_agl_km);
    georef.setEwVelocity(dsGeoref.ew_velocity_mps);
    georef.setNsVelocity(dsGeoref.ns_velocity_mps);
    georef.setVertVelocity(dsGeoref.vert_velocity_mps);
    georef.setHeading(dsGeoref.heading_deg);
    georef.setRoll(dsGeoref.roll_deg);
    georef.setPitch(dsGeoref.pitch_deg);
    georef.setDrift(dsGeoref.drift_angle_deg);
    georef.setRotation(dsGeoref.rotation_angle_deg);
    georef.setTilt(dsGeoref.tilt_angle_deg);
    georef.setEwWind(dsGeoref.ew_horiz_wind_mps);
    georef.setNsWind(dsGeoref.ns_horiz_wind_mps);
    georef.setVertWind(dsGeoref.vert_wind_mps);
    georef.setHeadingRate(dsGeoref.heading_rate_dps);
    georef.setPitchRate(dsGeoref.pitch_rate_dps);
    georef.setDriveAngle1(dsGeoref.drive_angle_1_deg);
    georef.setDriveAngle2(dsGeoref.drive_angle_2_deg);
    ray->clearGeoref();
    ray->setGeoref(georef);
  }

  // load up fields

  int byteWidth = rbeam.byteWidth;
  
  for (size_t iparam = 0; iparam < fparamsVec.size(); iparam++) {

    // is this an output field or censoring field?

    const DsFieldParams &fparams = *fparamsVec[iparam];
    string fieldName = fparams.name;
    if (!_isCensoringField(fieldName) && !_isOutputField(fieldName)) {
      continue;
    }

    // convert to floats
    
    Radx::fl32 *fdata = new Radx::fl32[nGates];

    if (byteWidth == 4) {

      fl32 *inData = (fl32 *) rbeam.data() + iparam;
      fl32 inMissing = (fl32) fparams.missingDataValue;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        fl32 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData;
        }
      } // igate

    } else if (byteWidth == 2) {

      ui16 *inData = (ui16 *) rbeam.data() + iparam;
      ui16 inMissing = (ui16) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui16 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } else {

      // byte width 1

      ui08 *inData = (ui08 *) rbeam.data() + iparam;
      ui08 inMissing = (ui08) fparams.missingDataValue;
      double scale = fparams.scale;
      double bias = fparams.bias;
      Radx::fl32 *outData = fdata;
      
      for (int igate = 0; igate < nGates;
           igate++, inData += fparamsVec.size(), outData++) {
        ui08 inVal = *inData;
        if (inVal == inMissing) {
          *outData = Radx::missingFl32;
        } else {
          *outData = *inData * scale + bias;
        }
      } // igate

    } // if (byteWidth == 4)

    RadxField *field = new RadxField(fparams.name, fparams.units);
    field->copyRangeGeom(*ray);
    field->setTypeFl32(Radx::missingFl32);
    field->addDataFl32(nGates, fdata);

    ray->addField(field);

    delete[] fdata;

  } // iparam

  return ray;

}

////////////////////////////////////////////////////////////////////
// censor an input ray

void Dsr2Radx::_censorInputRay(RadxRay *ray)

{

  if (!_params.apply_censoring) {
    return;
  }

  // initialize censoring flags to true to
  // turn censoring ON everywhere
  
  vector<int> censorFlag;
  size_t nGates = ray->getNGates();
  for (size_t igate = 0; igate < nGates; igate++) {
    censorFlag.push_back(1);
  }

  // check OR fields
  // if any of these have VALID data, we turn censoring OFF

  int orFieldCount = 0;

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {

    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_OR) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      // field missing, do not censor
      if (_nWarnCensorPrint % 360 == 0) {
        cerr << "WARNING - censoring field missing: " << cfld.name << endl;
        cerr << "  Censoring will not be applied for this field." << endl;
      }
      _nWarnCensorPrint++;
      for (size_t igate = 0; igate < nGates; igate++) {
        censorFlag[igate] = 0;
      }
      continue;
    }
    
    orFieldCount++;
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val >= minValidVal && val <= maxValidVal) {
        censorFlag[igate] = 0;
      }
    }
    
  } // ifield

  // if no OR fields were found, turn off ALL censoring at this stage

  if (orFieldCount == 0) {
    for (size_t igate = 0; igate < nGates; igate++) {
      censorFlag[igate] = 0;
    }
  }

  // check AND fields
  // if any of these have INVALID data, we turn censoring ON

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {
    
    const Params::censoring_field_t &cfld = _params._censoring_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_AND) {
      continue;
    }

    RadxField *field = ray->getField(cfld.name);
    if (field == NULL) {
      continue;
    }
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;

    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      double val = fdata[igate];
      if (val < minValidVal || val > maxValidVal) {
        censorFlag[igate] = 1;
      }
    }
    
  } // ifield

  // check that uncensored runs meet the minimum length
  // those which do not are censored

  int minValidRun = _params.censoring_min_valid_run;
  if (minValidRun > 1) {
    int runLength = 0;
    bool doCheck = false;
    for (int igate = 0; igate < (int) nGates; igate++) {
      if (censorFlag[igate] == 0) {
        doCheck = false;
        runLength++;
      } else {
        doCheck = true;
      }
      // last gate?
      if (igate == (int) nGates - 1) doCheck = true;
      // check run length
      if (doCheck) {
        if (runLength < minValidRun) {
          // clear the run which is too short
          for (int jgate = igate - runLength; jgate < igate; jgate++) {
            censorFlag[jgate] = 1;
          } // jgate
        }
        runLength = 0;
      } // if (doCheck ...
    } // igate
  }

  // apply censoring by setting censored gates to missing for all fields

  vector<RadxField *> fields = ray->getFields();
  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    RadxField *field = fields[ifield];
    Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      if (censorFlag[igate] == 1) {
        fdata[igate] = Radx::missingFl32;
      }
    } // igate
  } // ifield
  
}

////////////////////////////////////////////////////////////////////
// prepare a ray for output, by applying the following procedures:
//
//  (a) ensure only output fields remain
//  (b) set standard name etc
//  (c) convert to output type

void Dsr2Radx::_prepareRayForOutput(RadxRay *ray)
  
{

  // remove fields which are not in the output field list

  vector<string> wantedNames;
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    string dsrFieldName = _params._output_fields[ifield].dsr_name;
    wantedNames.push_back(dsrFieldName);
  } // ifield
  ray->trimToWantedFields(wantedNames);

  // set output names, pack data

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

    Params::output_field_t &ofld = _params._output_fields[ifield];
    string dsrFieldName = ofld.dsr_name;

    RadxField *field = ray->getField(dsrFieldName);
    if (field == NULL) {
      continue;
    }

    if (strlen(ofld.output_name) > 0) {
      field->setName(ofld.output_name);
    }
    if (strlen(ofld.long_name) > 0) {
      field->setLongName(ofld.long_name);
    }
    if (strlen(ofld.standard_name) > 0) {
      field->setStandardName(ofld.standard_name);
    }
    if (strlen(ofld.output_units) > 0) {
      field->setUnits(ofld.output_units);
    }

    if (ofld.output_encoding == Params::OUTPUT_SHORT) {
      field->convertToSi16();
    } else if (ofld.output_encoding == Params::OUTPUT_BYTE) {
      field->convertToSi08();
    } 

  } // ifield

  // load the field name map on the ray, because the field
  // names have changed

  ray->loadFieldNameMap();
  
}

////////////////////////////////////////////////////////////////////
// Do we need this field?

bool Dsr2Radx::_isCensoringField(const string &name)

{

  if (!_params.apply_censoring) {
    return false;
  }

  for (int ifield = 0; ifield < _params.censoring_fields_n; ifield++) {
    string censoringFieldName = _params._censoring_fields[ifield].name;
    if (name == censoringFieldName) {
      return true;
    }
  }

  return false;

}

bool Dsr2Radx::_isOutputField(const string &name)

{

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    string dsrFieldName = _params._output_fields[ifield].dsr_name;
    if (name == dsrFieldName) {
      return true;
    }
  }
  
  return false;

}

////////////////////////////////////////////////////////////////////
// Add a ray, checking to see if we should
//
// Otherwise delete it

void Dsr2Radx::_addRayToVol(RadxRay *ray)

{
  if (_acceptRay(ray)) {
    _vol.addRay(ray);
  } else {
    delete ray;
  }
}

////////////////////////////////////////////////////////////////////
// Check whether to accept ray

bool Dsr2Radx::_acceptRay(const RadxRay *ray)

{

  if (_params.filter_using_gate_spacing) {
    double diff = fabs(_params.specified_gate_spacing - ray->getGateSpacingKm());
    if (diff > _smallVal) {
      return false;
    }
  }
  
  if (_params.filter_using_start_range) {
    double diff = fabs(_params.specified_start_range - ray->getStartRangeKm());
    if (diff > _smallVal) {
      return false;
    }
  }

  if (_params.filter_using_elev) {
    double elev = ray->getElevationDeg();
    if (elev < _params.specified_min_elev ||
        elev > _params.specified_max_elev) {
      return false;
    }
  }
  
  if (_params.filter_using_sweep_number) {
    int sweepNum = ray->getSweepNumber();
    if (sweepNum < _params.specified_min_sweep_number ||
        sweepNum > _params.specified_max_sweep_number) {
      return false;
    }
  }
  
  // check for scan in idle mode
  
  if (_params.filter_when_scan_idle) {
    if (ray->getSweepMode() == Radx::SWEEP_MODE_IDLE) {
      return false;
    }
  }

  // check for scan in pointing mode
  
  if (_params.filter_when_scan_pointing) {
    if (ray->getSweepMode() == Radx::SWEEP_MODE_POINTING) {
      return false;
    }
  }

  // check for antenna transitions
  
  if (_params.filter_antenna_transitions) {
    if (ray->getAntennaTransition()) {
      return false;
    }
  }

  // check for movement if required
  
  if (_params.filter_antenna_stationary) {
    if (!_isAntennaMoving(ray)) {
      if (_params.debug >= Params::DEBUG_EXTRA) {
	cerr << "WARNING - antenna not moving, will ignore ray" << endl;
	cerr << "  el, az: "
	     << ray->getElevationDeg() << ", "
	     << ray->getAzimuthDeg() << endl;
      }
      return false;
    }
  }

  return true;

}

////////////////////////////////////////////////////////////////
// is antenna moving

bool Dsr2Radx::_isAntennaMoving(const RadxRay *ray)

{

  double elev = ray->getElevationDeg();
  double az = ray->getAzimuthDeg();
  double deltaElev = fabs(elev - _prevElevMoving);
  double deltaAz = fabs(az - _prevAzMoving);
  bool isMoving;
  if (deltaElev > _params.min_angle_change_for_moving_antenna ||
      deltaAz > _params.min_angle_change_for_moving_antenna) {
    isMoving = true;
  } else {
    isMoving = false;
  }

  _prevElevMoving = elev;
  _prevAzMoving = az;

  return isMoving;

}

////////////////////////////////////////////////////////////////
// load the current scan mode
//
// returns 0 on success, -1 on failure

int Dsr2Radx::_loadCurrentScanMode()
  
{
  
  if (_vol.getNRays() == 0) {
    _scanMode = SCAN_MODE_UNKNOWN;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "loadCurrentScanMode()" << endl;
  }

  if (_params.use_input_scan_mode) {
    
    if (_findScanModeFromHeaders() == 0) {
      _scanInfoFromHeaders = true;
      _endOfVolAutomatic =
	(_params.end_of_vol_decision == Params::AUTOMATIC);
    } else {
      _findScanModeFromAntenna();
      _scanInfoFromHeaders = false;
      _endOfVolAutomatic = true;
    }

  } else {

    _findScanModeFromAntenna();
    _scanInfoFromHeaders = false;
    _endOfVolAutomatic =
      (_params.end_of_vol_decision == Params::AUTOMATIC);

  }

  // special case - vertical scan

  if (_isVertScan()) {
    _scanMode = SCAN_MODE_VERT;
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    if (_scanMode == SCAN_MODE_RHI) {
      cerr << "  currentScanMode: RHI" << endl;
    } else if (_scanMode == SCAN_MODE_PPI) {
      cerr << "  currentScanMode: PPI" << endl;
    } else if (_scanMode == SCAN_MODE_SECTOR) {
      cerr << "  currentScanMode: SECTOR" << endl;
    } else if (_scanMode == SCAN_MODE_SURVEILLANCE) {
      cerr << "  currentScanMode: SURVEILLANCE" << endl;
    } else if (_scanMode == SCAN_MODE_VERT) {
      cerr << "  currentScanMode: VERT" << endl;
    } else {
      cerr << "  currentScanMode: UNKNOWN" << endl;
    }      
  }

  return 0;

}

////////////////////////////////////////////////////////////////
// find the current scan mode from the data headers
//
// returns 0 on success, -1 on failure

int Dsr2Radx::_findScanModeFromHeaders()
  
{
  
  // find predominant scan mode
  
  double countSur = 0;
  double countSec = 0;
  double countRhi = 0;
  double countSun = 0;
  double countSunRhi = 0;
  double countTotal = 0;

  for (size_t iray = 0; iray < _vol.getNRays(); iray++) {
    RadxRay *ray = _vol.getRays()[iray];
    Radx::SweepMode_t sweepMode = ray->getSweepMode();
    if (sweepMode == Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE ||
	sweepMode == Radx::SWEEP_MODE_VERTICAL_POINTING) {
      countSur++;
    } else if (sweepMode == Radx::SWEEP_MODE_SECTOR) {
      countSec++;
    } else if (sweepMode == Radx::SWEEP_MODE_RHI) {
      countRhi++;
    } else if (sweepMode == Radx::SWEEP_MODE_SUNSCAN) {
      countSun++;
    } else if (sweepMode == Radx::SWEEP_MODE_SUNSCAN_RHI) {
      countSunRhi++;
    }
    countTotal++;
  }

  double fractionGood =
    (countSur + countSec + countRhi + countSun + countSunRhi) / countTotal;
  if (fractionGood < 0.8) {
    return -1;
  }
  
  if (countRhi > (countSur + countSec)) {
    _scanMode = SCAN_MODE_RHI;
  } else if (countSec > countSur) {
    _scanMode = SCAN_MODE_SECTOR;
  } else if (countSun > countSur) {
    _scanMode = SCAN_MODE_SUNSCAN;
  } else if (countSunRhi > countSur) {
    _scanMode = SCAN_MODE_SUNSCAN_RHI;
  } else {
    _scanMode = SCAN_MODE_SURVEILLANCE;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "findScanModeFromHeaders() ...." << endl;
    cerr << "  countSur: " << countSur << endl;
    cerr << "  countSec: " << countSec << endl;
    cerr << "  countRhi: " << countRhi << endl;
    cerr << "  countSun: " << countSun << endl;
  }
  
  return 0;

}

////////////////////////////////////////////////////////////////
// find the current scan mode from antenna angles

void Dsr2Radx::_findScanModeFromAntenna()
  
{
  
  // find which slews more - azimuth or elevation
  
  double azTravel = 0.0;
  double elTravel = 0.0;

  const vector<RadxRay *> rays = _vol.getRays();
  RadxRay *ray = rays[0];
  double prevEl = ray->getElevationDeg();
  double prevAz = ray->getAzimuthDeg();
  
  for (size_t iray = 1; iray < rays.size(); iray++) {
    RadxRay *ray = rays[iray];
    double el = ray->getElevationDeg();
    double az = ray->getAzimuthDeg();
    double elDiff = el - prevEl;
    if (elDiff > 180) {
      elDiff -= 360.0;
    } else if (elDiff < -180) {
      elDiff += 360.0;
    }
    double azDiff = az - prevAz;
    if (azDiff > 180) {
      azDiff -= 360.0;
    } else if (azDiff < -180) {
      azDiff += 360.0;
    }
    elTravel += fabs(elDiff);
    azTravel += fabs(azDiff);
    prevEl = el;
    prevAz = az;
  }
  
  // at this point we can only determine whether it is an
  // RHI or not. In PpiMgr, we can determine whether it is a
  // sector or surveillance
  
  if (elTravel > azTravel) {
    _scanMode = SCAN_MODE_RHI;
  } else {
    _scanMode = SCAN_MODE_PPI; // generic non-rhi mode
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "findScanModeFromAntenna() ...." << endl;
    cerr << "  elTravel: " << elTravel << endl;
    cerr << "  azTravel: " << azTravel << endl;
    if (_scanMode == SCAN_MODE_RHI) {
      cerr << "    RHI mode" << endl;
    } else {
      cerr << "    PPI mode" << endl;
    }
  }

}

////////////////////////////////////////////////////////////////
// check if this is a vertically-pointing scan

bool Dsr2Radx::_isVertScan()
  
{

  // check if this is even an option

  if (!_params.separate_output_dirs_by_scan_type) {
    return false;
  }

  // count number of beams above min elevation
  
  double countAbove = 0.0;
  double countTotal = 0.0;

  const vector<RadxRay *> rays = _vol.getRays();
  for (size_t iray = 1; iray < rays.size(); iray++) {
    RadxRay *ray = rays[iray];
    double el = ray->getElevationDeg();
    if (el > _params.min_elevation_for_vert_files) {
      countAbove++;
    }
    countTotal++;
  }

  double fractionAbove = countAbove / countTotal;

  if (fractionAbove > _params.min_vert_fraction_for_vert_files) {
    return true;
  }

  return false;

}

/////////////////////////////////////////////////////////////////
// clear methods

void Dsr2Radx::_clearData()

{
  _vol.clear();
  _sweepNumbersMissing = false;
  _nRaysRead = 0;
}

/////////////////////////////////////////////
// convert from DsrRadar enums to Radx enums

Radx::SweepMode_t Dsr2Radx::_getRadxSweepMode(int dsrScanMode)

{

  switch (dsrScanMode) {
    case DS_RADAR_SECTOR_MODE:
    case DS_RADAR_FOLLOW_VEHICLE_MODE:
      return Radx::SWEEP_MODE_SECTOR;
    case DS_RADAR_COPLANE_MODE:
      return Radx::SWEEP_MODE_COPLANE;
    case DS_RADAR_RHI_MODE:
      return Radx::SWEEP_MODE_RHI;
    case DS_RADAR_VERTICAL_POINTING_MODE:
      return Radx::SWEEP_MODE_VERTICAL_POINTING;
    case DS_RADAR_MANUAL_MODE:
      return Radx::SWEEP_MODE_POINTING;
    case DS_RADAR_SURVEILLANCE_MODE:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
    case DS_RADAR_EL_SURV_MODE:
      return Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE;
    case DS_RADAR_SUNSCAN_MODE:
      return Radx::SWEEP_MODE_SUNSCAN;
    case DS_RADAR_SUNSCAN_RHI_MODE:
      return Radx::SWEEP_MODE_SUNSCAN_RHI;
    case DS_RADAR_POINTING_MODE:
      return Radx::SWEEP_MODE_POINTING;
    default:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }
}

Radx::PolarizationMode_t Dsr2Radx::_getRadxPolarizationMode(int dsrPolMode)

{
  switch (dsrPolMode) {
    case DS_POLARIZATION_HORIZ_TYPE:
      return Radx::POL_MODE_HORIZONTAL;
    case DS_POLARIZATION_VERT_TYPE:
      return Radx::POL_MODE_VERTICAL;
    case DS_POLARIZATION_DUAL_TYPE:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_DUAL_HV_ALT:
      return Radx::POL_MODE_HV_ALT;
    case DS_POLARIZATION_DUAL_HV_SIM:
      return Radx::POL_MODE_HV_SIM;
    case DS_POLARIZATION_DUAL_H_XMIT:
      return Radx::POL_MODE_HV_H_XMIT;
    case DS_POLARIZATION_DUAL_V_XMIT:
      return Radx::POL_MODE_HV_V_XMIT;
    case DS_POLARIZATION_RIGHT_CIRC_TYPE:
    case DS_POLARIZATION_LEFT_CIRC_TYPE:
      return Radx::POL_MODE_CIRCULAR;
    case DS_POLARIZATION_ELLIPTICAL_TYPE:
    default:
      return Radx::POL_MODE_HORIZONTAL;
  }
}

Radx::FollowMode_t Dsr2Radx::_getRadxFollowMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_FOLLOW_MODE_SUN:
      return Radx::FOLLOW_MODE_SUN;
    case DS_RADAR_FOLLOW_MODE_VEHICLE:
      return Radx::FOLLOW_MODE_VEHICLE;
    case DS_RADAR_FOLLOW_MODE_AIRCRAFT:
      return Radx::FOLLOW_MODE_AIRCRAFT;
    case DS_RADAR_FOLLOW_MODE_TARGET:
      return Radx::FOLLOW_MODE_TARGET;
    case DS_RADAR_FOLLOW_MODE_MANUAL:
      return Radx::FOLLOW_MODE_MANUAL;
    default:
      return Radx::FOLLOW_MODE_NONE;
  }
}

Radx::PrtMode_t Dsr2Radx::_getRadxPrtMode(int dsrMode)

{
  switch (dsrMode) {
    case DS_RADAR_PRF_MODE_FIXED:
      return Radx::PRT_MODE_FIXED;
    case DS_RADAR_PRF_MODE_STAGGERED_2_3:
    case DS_RADAR_PRF_MODE_STAGGERED_3_4:
    case DS_RADAR_PRF_MODE_STAGGERED_4_5:
      return Radx::PRT_MODE_STAGGERED;
    default:
      return Radx::PRT_MODE_FIXED;
  }
}

////////////////////////////////////////////////////
// compute the number of gates given the max height

int Dsr2Radx::_computeNgatesForMaxHt(double elev,
                                     double radarAltitudeKm,
                                     double startRangeKm,
                                     double gateSpacingKm)

{

  // make sure lookup table is up to date

  _computeMaxRangeLut(radarAltitudeKm);

  // condition elevation angle

  if (elev > 90.0) {
    elev = 180.0 - elev;
  }

  // negative elevation angle?
  
  if (elev <= 0) {
    // no limit to number of gates
    return 1000000;
  }

  // get range from lookup table

  int ielev = (int) elev;
  if (ielev > 89) {
    ielev = 89;
  } else if (ielev < 0) {
    ielev = 0;
  }

  double maxRangeKm = _maxRangeLut[ielev];

  // compute number of gates

  int maxGates = (int) ((maxRangeKm - startRangeKm) / gateSpacingKm + 1.0);

  return maxGates;

}

/////////////////////////////////////////////////////////////////
// compute lookup table for max range at various elevation angles
	
void Dsr2Radx::_computeMaxRangeLut(double radarAltitudeKm)

{

  if (fabs(_lutRadarAltitudeKm - radarAltitudeKm) < 0.001) {
    // previously computed
    return;
  }

  // fill lookup table

  double maxHtKm = _params.max_height_km_msl;

  for (int ielev = 0; ielev < 90; ielev++) {
    
    double elev = ielev + 0.01;
    double sinElev = sin(elev * DEG_TO_RAD);

    for (double rangeKm = 1.0; rangeKm < 2000; rangeKm++) {
      double htKm = (radarAltitudeKm +
                     (rangeKm * sinElev) +
                     ((rangeKm * rangeKm) / _pseudoDiam)); 
      _maxRangeLut[ielev] = rangeKm;
      if (htKm > maxHtKm) {
        break;
      }
    } // rangeKm

  } // ielev

  _lutRadarAltitudeKm = radarAltitudeKm;

}

/////////////////////////////////////////////////////////////////
// compute time for end of volume
	
void Dsr2Radx::_computeEndOfVolTime(time_t beamTime)

{

  int secsPerDay = 86400;
  int jDay = beamTime / secsPerDay;
  int secInDay = beamTime % secsPerDay;
  int volInDay = secInDay / _params.nsecs_per_volume;
  int endOfVolSec = (volInDay + 1) * _params.nsecs_per_volume;
  if (endOfVolSec > secsPerDay) {
    endOfVolSec = secsPerDay;
  }
  _endOfVolTime = jDay * secsPerDay + endOfVolSec;

}

