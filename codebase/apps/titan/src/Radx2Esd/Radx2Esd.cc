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
// Radx2Esd.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2017
//
///////////////////////////////////////////////////////////////
//
// Reads radial radar data in file supported by Radx,
// writes out file in ESD ASCII format.
//
////////////////////////////////////////////////////////////////

#include "Radx2Esd.hh"
#include <Radx/Radx.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
using namespace std;

// Constructor

Radx2Esd::Radx2Esd(int argc, char **argv)
  
{

  OK = TRUE;
  _volNum = 1;

  // set programe name

  _progName = "Radx2Esd";
  ucopyright((char *) _progName.c_str());
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

}

// destructor

Radx2Esd::~Radx2Esd()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Radx2Esd::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int Radx2Esd::_runFilelist()
{
  
  int iret = 0;

  if (_params.debug) {
    cerr << "Running Radx2Esd" << endl;
    cerr << "  n input files: " << _args.inputFileList.size() << endl;
  }

  // loop through the input file list
  
  RadxVol vol;
  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {
    string inputPath = _args.inputFileList[ii];
    // read input file
    int jret = _readFile(inputPath, vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - Radx2Esd::_runFileList" << endl;
        cerr << "  Cannot write volume to file" << endl;
        iret = -1;
      }
    } else if (jret < 0) {
      cerr << "ERROR reading file: " << inputPath << endl;
    }
    // free up
    vol.clear();
  }
  
  if (_params.debug) {
    cerr << "Radx2Esd done" << endl;
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int Radx2Esd::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - Radx2Esd::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - Radx2Esd::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list

  RadxVol vol;
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    // read input file
    int jret = _readFile(paths[ii], vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - Radx2Esd::_runArchive" << endl;
        cerr << "  Cannot write volume to file" << endl;
        return -1;
      }
    } else if (jret < 0) {
      iret = -1;
    }
    // free up
    vol.clear();
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode with latest data info

int Radx2Esd::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  if (strlen(_params.search_ext) > 0) {
    ldata.setDataFileExt(_params.search_ext);
  }

  RadxVol vol;
  int iret = 0;
  int msecsWait = 1000;
  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       msecsWait, PMU_auto_register);
    const string path = ldata.getDataPath();
    // read input file
    int jret = _readFile(path, vol);
    if (jret == 0) {
      // finalize the volume
      _finalizeVol(vol);
      // write the volume out
      if (_writeVol(vol)) {
        cerr << "ERROR - Radx2Esd::_runRealtimeWithLdata" << endl;
        cerr << "  Cannot write volume to file" << endl;
        return -1;
      }
    } else if (jret < 0) {
      iret = -1;
    }
    // free up
    vol.clear();
  }

  return iret;

}

//////////////////////////////////////////////////
// Read in a file
// accounting for special cases such as gematronik
// Returns 0 on success
//         1 if already read,
//         -1 on failure

int Radx2Esd::_readFile(const string &readPath,
                           RadxVol &vol)
{

  PMU_auto_register("Processing file");

  // clear all data on volume object
  
  vol.clear();

  // check we have not already processed this file
  // in the file aggregation step
  
  RadxPath thisPath(readPath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath listPath(_readPaths[ii]);
    if (thisPath.getFile() == listPath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << readPath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 1;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - Radx2Esd::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file

  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - Radx2Esd::_readFile" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < _readPaths.size(); ii++) {
      cerr << "  ==>> read in file: " << _readPaths[ii] << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////
// Finalize the volume based on parameters
// Returns 0 on success, -1 on failure

void Radx2Esd::_finalizeVol(RadxVol &vol)
  
{

  // override start range and/or gate spacing

  vol.remapRangeGeom(_params.start_range_km, _params.gate_spacing_km);
  vol.setNGates(_params.n_gates);

  // trim sweeps to 360 deg

  vol.trimSurveillanceSweepsTo360Deg();

  // apply time offset

  if (_params.apply_time_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying time offset (secs): " 
           << _params.time_offset_secs << endl;
    }
    vol.applyTimeOffsetSecs(_params.time_offset_secs);
  }

  // apply angle offsets

  if (_params.apply_azimuth_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying azimuth offset (deg): " 
           << _params.azimuth_offset << endl;
    }
    vol.applyAzimuthOffset(_params.azimuth_offset);
  }
  if (_params.apply_elevation_offset) {
    if (_params.debug) {
      cerr << "NOTE: applying elevation offset (deg): " 
           << _params.elevation_offset << endl;
    }
    vol.applyElevationOffset(_params.elevation_offset);
  }

  vol.loadSweepInfoFromRays();

}

//////////////////////////////////////////////////
// set up read

void Radx2Esd::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.set_fixed_angle_limits) {
    file.setReadFixedAngleLimits(_params.lower_fixed_angle_limit,
                                 _params.upper_fixed_angle_limit);
    if (_params.lower_fixed_angle_limit == _params.upper_fixed_angle_limit) {
      // relax strict angle checking since only a single angle is specified
      // which means the user wants the closest angle
      file.setReadStrictAngleLimits(false);
    }
  } else if (_params.set_sweep_num_limits) {
    file.setReadSweepNumLimits(_params.lower_sweep_num,
                               _params.upper_sweep_num);
  }

  file.setReadStrictAngleLimits(true);

  file.addReadField(_params.dbz_field_name);

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////
// write out the volume

int Radx2Esd::_writeVol(RadxVol &vol)
{

  // output path
  
  RadxTime startTime(vol.getStartTimeSecs());
  
  string outDir(_params.output_dir);
  char dayStr[BUFSIZ];
  sprintf(dayStr, "%.4d%.2d%.2d",
          startTime.getYear(), startTime.getMonth(), startTime.getDay());
  outDir += RadxFile::PATH_SEPARATOR;
  outDir += dayStr;

  // make sure output subdir exists
  
  if (RadxFile::makeDirRecurse(outDir)) {
    int errNum = errno;
    cerr << "ERROR - Radx2Esd::_writeVol" << endl;
    cerr << "  Cannot make output dir: " << outDir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  char timeStr[BUFSIZ];
  sprintf(timeStr, "%.2d%.2d%.2d",
          startTime.getHour(), startTime.getMin(), startTime.getSec());
  
  string outPath(outDir);
  outPath += RadxFile::PATH_SEPARATOR;
  outPath += "esd_ascii_";
  outPath += dayStr;
  outPath += "_";
  outPath += timeStr;
  outPath += ".txt";

  // open the file

  FILE *out;
  if ((out = fopen(outPath.c_str(), "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Radx2Esd::_writeVol" << endl;
    cerr << "  Cannot open file for writing: " << outPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  vector<RadxRay *> rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _writeRay(out, rays[ii]);
    if (ii >= 359) {
      break;
    }
  }

  // close the file

  fclose(out);

  if (_params.debug) {
    cerr << "Wrote file: " << outPath << endl;
  }
  
  // write latest data info file if requested 
  
  if (_params.write_latest_data_info) {
    DsLdataInfo ldata(outDir);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ldata.setDebug(true);
    }
    string relPath;
    RadxPath::stripDir(outDir, outPath, relPath);
    ldata.setRelDataPath(relPath);
    ldata.setWriter(_progName);
    if (ldata.write(vol.getEndTimeSecs())) {
      cerr << "WARNING - Radx2Esd::_writeVol" << endl;
      cerr << "  Cannot write latest data info file to dir: "
           << outDir << endl;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////////
// write out a ray

void Radx2Esd::_writeRay(FILE *out, RadxRay *ray)

{

  // get the DBZ field

  RadxField *dbzField = ray->getField(_params.dbz_field_name);
  if (dbzField == NULL) {
    // no DBZ field found
    cerr << "WARNING - no DBZ field found, name: " 
         << _params.dbz_field_name << endl;
    ray->print(cerr);
    return;
  }

  // convert to floats

  dbzField->convertToFl32();
  Radx::fl32 *dbz = dbzField->getDataFl32();

  // create the output line

  // Each ray packet has an header stating with $,
  // followed by 2 digit site number and azimuth,
  // then followed by gate data in Hex.
  // The data are all ASCII hex values from 0 to F.
  // The data is terminated with a 16 bit check sum followed by a # and CR/LF.

  // This is what the data looks like for beam azimuth 359.
  // $003590123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
  //  0123456789ABCDEF0123456789ABCDEF01234567#89ABCDEF0123
  //  456789ABCDEF0123456789ABCDEF0123456789ABCDEF012345678
  //  9ABCDEF0123456789ABCDEF0123456789ABCDEF1234#

  string line;
  char text[1024];
  
  // radar num

  sprintf(text, "$%.2d", _params.radar_number);
  line += text;

  // azimuth in whole deg

  int iaz = (int) (ray->getAzimuthDeg() + 0.5);
  if (iaz > 359) {
    iaz -= 360;
  } else if (iaz < 0) {
    iaz += 360.0;
  }
  sprintf(text, "%.3d", iaz);
  line += text;

  // dbz in HEX - in 5 dB increments starting from 0
  
  for (size_t igate = 0; igate < ray->getNGates(); igate++) {
    double fdbz = dbz[igate];
    if (fdbz < 0) {
      fdbz = 0;
    }
    if (fdbz > 79) {
      fdbz = 79;
    }
    int idbz = (int) (fdbz / 5.0);
    if (idbz > 9) {
      idbz += 7;
    }
    sprintf(text, "%c", idbz + 48);
    line += text;
  }

  // compute the check sum

  int checkSum = _calcCheckSum16(line.c_str(), line.size());
  cerr << "111111111111 checkSum: " << checkSum << endl;
  fprintf(stderr, "2222222222 checkSum: %.2x\n", checkSum);

  // write out line

  fprintf(out, "%s", line.c_str());

  // add check sum

  fprintf(out, "%.2x#\r\n", checkSum);
  
}

//------------------------------------------------------------------------
// Calculate check sum  A and B,  16 bit
//------------------------------------------------------------------------

int Radx2Esd::_calcCheckSum16(const char *pntr, size_t len)
{

  Radx::ui08 a = 0, b = 0;
  int chk_sum;

  for(size_t i = 0; i < len; i++) {
    a += pntr[i];
    b += a;
  }

  a &= 0xFF;
  b &= 0xFF;

  chk_sum = (int) ((a << 8)| b );

  return chk_sum;

}
