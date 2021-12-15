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
///////////////////////////////////////////////////////////////////////////
// MrmsGribIngest reads individual height layer files for the MRMS
// radar mosaic, and compiles them into a single MDV file
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
// April 2015
//
//////////////////////////////////////////////////////////////////////////

#include <tdrp/tdrp.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/TaFile.hh>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
#include <Mdv/MdvxField.hh>
#include <sys/types.h>
#include <dirent.h>

#include "MrmsGribIngest.hh"
#include "Grib2Mdv.hh"
using namespace std;

/////////////////////////////////////////////////////
// Constructor

MrmsGribIngest::MrmsGribIngest(int argc, char **argv)

{

  isOK = true;
  _grib2Mdv = NULL;
  _input = NULL;
  _latestDataTime = -1;
  _prevDataWriteTime = -1;
  _layers.clear();

  // set programe name

  _progName = "MrmsGribIngest";
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
  
  _grib2Mdv = new Grib2Mdv(_params);
  
  // initialize process registration
  PMU_auto_init(_progName.c_str(), _params.instance,
                _params.procmap_register_interval_secs);
  
  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_data_age,
			     PMU_auto_register,
			     _params.latest_data_info_avail);
    _input->setMaxRecursionDepth(_params.max_recursion_depth);
    _input->setSearchExt(_params.input_extension);
    _input->setSubString(_params.input_substring);
    _input->setLatestFileOnly(_params.latest_file_only);
    _input->setDirScanSleep(_params.data_check_interval_secs);
    _input->setFileQuiescence(_params.data_file_quiescence_secs);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName.c_str(),
                             _params.debug >= Params::DEBUG_VERBOSE,
                             _params.input_dir,
                             _args.startTime,
                             _args.endTime);
    _input->setSearchExt(_params.input_extension);
    _input->setSubString(_params.input_substring);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

}

////////////////////////////////////////////////////////
// Destructor

MrmsGribIngest::~MrmsGribIngest() 
{

  // Unregister with process mapper
  
  PMU_auto_unregister();

  if (_grib2Mdv != NULL) {
    delete _grib2Mdv;
  }

  if (_input != NULL) {
    delete _input;
  }

  _clearLayers();

}

/////////////////////
// Run - do the work

int MrmsGribIngest::Run()
{
  
  // loop until end of data
  
  char *inputPath;
  int iret = 0;
  while ((inputPath = _input->next()) != NULL) {
    
    PMU_auto_register("Run");

    if (_params.mode == Params::REALTIME) {
      if (_processFileRealtime(inputPath)) {
        cerr << "ERROR - MrmsGribIngest::Run" << endl;
        cerr << "  Processing file - realtime: " << inputPath << endl;
        iret = -1;
      }
    } else {
      if (_processFileArchive(inputPath)) {
        cerr << "ERROR - MrmsGribIngest::Run" << endl;
        cerr << "  Processing file - archive: " << inputPath << endl;
        iret = -1;
      }
    }
    
  }

  return iret;

}

////////////////////////////////////////////////////////
// clear the data layers

void MrmsGribIngest::_clearLayers() 
{

  for (LayersIter ii = _layers.begin(); ii != _layers.end(); ii++) {
    delete ii->second;
  }
  _layers.clear();

}

///////////////////////////////
// process file - realtime mode

int MrmsGribIngest::_processFileRealtime(const string &inputPath)
  
{
  
  PMU_auto_register("Processing file - realtime");
    
  // uncompress the file if needed
  
  TaFile infile;
  if (infile.uncompress(inputPath)) {
    cerr << "ERROR uncompressing input file: " << inputPath << endl;
    return -1;
  }

  string uncompressedPath = infile.getUncompressedPath();
  time_t fileDataTime = 0;
  bool dateOnly = false;
  if (DataFileNames::getDataTime(uncompressedPath, fileDataTime, dateOnly)) {
    cerr << "ERROR - _processFileRealtime" << endl;
    cerr << "  Cannot get time from file: " << uncompressedPath << endl;
  }

  // check if we have already processed this file
  
  if (_fileDataTimes.find(fileDataTime) != _fileDataTimes.end()) {
    // time previously processed - so ignore
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--------------------------------------------" << endl;
      cerr << "Ignoring this file: " << uncompressedPath << endl;
      cerr << "  Data time: " << DateTime::strm(fileDataTime) << endl;
      cerr << "  Time already processed" << endl;
    }
    return 0;
  }
  
  if (_params.debug) {
    cerr << "--------------------------------------------" << endl;
    cerr << "Processing file: " << uncompressedPath << endl;
    cerr << "  Data time: " << DateTime::strm(fileDataTime) << endl;
  }

  // create MDV output file

  DsMdvx *layer = new DsMdvx;
  
  // read in Grib file
  
  if (_grib2Mdv->readFile(uncompressedPath, *layer)) {
    return -1;
  }
  if (layer->getNFields() == 0) {
    cerr << "WARNING: No fields found in grib2 file." << endl;
    cerr << "         No output MDV file created." << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Read in layer, height: " << _getHeightKm(*layer) << endl;
  }

  // trim horizonal extent if requested

  if (_params.set_output_bounding_box) {

    layer->setReadHorizLimits(_params.output_bounding_box.min_lat,
                              _params.output_bounding_box.min_lon,
                              _params.output_bounding_box.max_lat,
                              _params.output_bounding_box.max_lon);
    layer->constrainHorizontal();

  }


  // has time changed? do we have enough layers?
  
  int iret = 0;
  bool volComplete = false;
  bool timeChanged = false;
  time_t dataTime = _getDataTime(*layer);
  if (dataTime != _latestDataTime) {
    timeChanged = true;
    volComplete = true;
  } else if ((int) _layers.size() == _params.ideal_n_ht_levels_for_volume - 1) {
    volComplete = true;
    // save layer in layer map
    LayerPair lp(_getHeightKm(*layer), layer);
    _layers.insert(lp);
    if (_params.debug) {
      cerr << "  n layers: " << _layers.size() << endl;
    }
  }

  if (volComplete) {

    // check elapsed time since last wrote data

    int timeSinceWrite = _latestDataTime - _prevDataWriteTime;
    if (timeSinceWrite < _params.min_time_between_output_files_in_secs) {

      // not enough time between data sets
      // do not write out

      PMU_auto_register( "Discarding file" );
      if (_params.debug) {
        cerr << "============================================" << endl;
        cerr << "WARNING - MrmsGribIngest" << endl;
        cerr << "  Data arrived too fast" << endl;
        cerr << "  latest data time : " << DateTime::strm(_latestDataTime) << endl;
        cerr << "  prev data written: " << DateTime::strm(_prevDataWriteTime) << endl;
        cerr << "  time between data (secs): " << timeSinceWrite << endl;
        cerr << "  min time between writes (secs): "
             << _params.min_time_between_output_files_in_secs << endl;
        cerr << "  volume will be discarded" << endl;
        cerr << "============================================" << endl;
      }

    } else {
      
      // combine data into volume
      
      if (_combineVolume() == 0) {
        
        // write data out
        
        PMU_auto_register( "Writing mdv file" );
        if (_params.debug) {
          cerr << "============================================" << endl;
          cerr << "Writing output file for time: " << DateTime::strm(_latestDataTime) << endl;
        }
        if (_outVol.writeToDir(_params.output_url)) {
          cerr << "ERROR: Could not write MDV file." << endl;
          cerr << _outVol.getErrStr() << endl;
          iret = -1;
        }
        if (_params.debug) {
          cerr << "Wrote output file: " << _outVol.getPathInUse() << endl;
          cerr << "============================================" << endl;
        }
        _prevDataWriteTime = _latestDataTime;
        
      }

    } // if (timeSinceWrite < ....

    // clear

    _clearLayers();
    _outVol.clear();

  } // if (volComplete)
  
  // save layer in layer map if appropriate
  
  if (timeChanged || !volComplete) {
    LayerPair lp(_getHeightKm(*layer), layer);
    _layers.insert(lp);
    _latestDataTime = dataTime;
    if (_params.debug) {
      cerr << "  n layers: " << _layers.size() << endl;
    }
  }
  
  return iret;

}

///////////////////////////////
// process file - archive mode

int MrmsGribIngest::_processFileArchive(const string &inputPath)
  
{
  
  PMU_auto_register("Processing file - archive");

  // get time from file name
  
  time_t fileDataTime = 0;
  bool dateOnly = false;
  if (DataFileNames::getDataTime(inputPath, fileDataTime, dateOnly)) {
    cerr << "ERROR - _processFileArchive" << endl;
    cerr << "  Cannot get time from file: " << inputPath << endl;
    return -1;
  }

  // elapsed time since last file

  long secsSinceLatestFile = fileDataTime - _latestDataTime;
  if (secsSinceLatestFile < _params.min_time_between_output_files_in_secs) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--------------------------------------------" << endl;
      cerr << "Ignoring this file: " << inputPath << endl;
      cerr << "  Data time: " << DateTime::strm(fileDataTime) << endl;
      cerr << "  Secs since last time: " << secsSinceLatestFile << endl;
    }
    return 0;
  }
  
  // check if we have already processed this file
  
  if (_fileDataTimes.find(fileDataTime) != _fileDataTimes.end()) {
    // time previously processed - so ignore
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--------------------------------------------" << endl;
      cerr << "Ignoring this file: " << inputPath << endl;
      cerr << "  Data time: " << DateTime::strm(fileDataTime) << endl;
      cerr << "  Time already processed" << endl;
    }
    return 0;
  }

  // get list of files at this time

  vector<string> timedPathList;
  if (_getInputPathList(inputPath, fileDataTime, timedPathList)) {
    cerr << "ERROR - _processFileArchive" << endl;
    cerr << "  Cannot get path list for file: " << inputPath << endl;
    return -1;
  }

  // loop through the path list

  _clearLayers();
  _outVol.clear();
  
  for (size_t ipath = 0; ipath < timedPathList.size(); ipath++) {

    string timedPath(timedPathList[ipath]);
    
    // uncompress the file if needed
    
    TaFile infile;
    if (infile.uncompress(timedPath)) {
      cerr << "ERROR uncompressing input file: " << timedPath << endl;
      continue;
    }
    
    string uncompressedPath = infile.getUncompressedPath();
    if (_params.debug) {
      cerr << "--------------------------------------------" << endl;
      cerr << "Processing file: " << uncompressedPath << endl;
      cerr << "  Data time: " << DateTime::strm(fileDataTime) << endl;
    }
    
    // create MDV output file

    DsMdvx *layer = new DsMdvx;
    
    // read in Grib file
    
    if (_grib2Mdv->readFile(uncompressedPath, *layer)) {
      continue;
    }
    if (layer->getNFields() == 0) {
      cerr << "WARNING: No fields found in grib2 file." << endl;
      cerr << "         No output MDV file created." << endl;
      continue;
    }
    
    if (_params.debug) {
      cerr << "Read in layer, height: " << _getHeightKm(*layer) << endl;
    }
    
    // trim horizonal extent if requested
    
    if (_params.set_output_bounding_box) {
      
      layer->setReadHorizLimits(_params.output_bounding_box.min_lat,
                                _params.output_bounding_box.min_lon,
                                _params.output_bounding_box.max_lat,
                                _params.output_bounding_box.max_lon);
      layer->constrainHorizontal();
      
    }
    
    // save layer in layer map
    LayerPair lp(_getHeightKm(*layer), layer);
    _layers.insert(lp);

    if (_params.debug) {
      cerr << "  data time: " << DateTime::strm(fileDataTime) << endl;
      cerr << "  n layers so far: " << _layers.size() << endl;
    }

  } // ipath

  _latestDataTime = fileDataTime;
  
  if (_layers.size() < 1) {
    cerr << "ERROR - no layers found for data time: " << DateTime::strm(fileDataTime) << endl;
    return -1;
  }
  
  // combine data into volume
  
  if (_combineVolume() == 0) {
    
    // write data out
    
    PMU_auto_register( "Writing mdv file" );
    if (_params.debug) {
      cerr << "============================================" << endl;
      cerr << "Writing output file for time: " << DateTime::strm(_latestDataTime) << endl;
    }
    if (_outVol.writeToDir(_params.output_url)) {
      cerr << "ERROR: Could not write MDV file." << endl;
      cerr << _outVol.getErrStr() << endl;
      _clearLayers();
      _outVol.clear();
      return -1;
    }
    if (_params.debug) {
      cerr << "Wrote output file: " << _outVol.getPathInUse() << endl;
      cerr << "============================================" << endl;
    }
    
  }
  
  // clear
  
  _clearLayers();
  _outVol.clear();

  return 0;

}

//////////////////////////////////////////////////////////////////////
// Get input path list for this data time
// read through all files

int MrmsGribIngest::_getInputPathList(const string &inputPath,
                                      time_t dataTime,
                                      vector<string> &timedPathList)
{

  // get directory

  Path pInput(inputPath);
  string dirPath = pInput.getDirectory();

  // read through dir
  
  DIR *dirp;
  dirp = opendir(dirPath.c_str());
  if (dirp == NULL) {
    int errNum = errno;
    cerr << "ERROR - MrmsGribIngest::_getInputPathList" << endl;
    cerr << "  dir: " << dirPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  
  struct dirent *dp;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){

    // Those files which start with a dot or an underscore, they go
    // away.
    
    if ((dp->d_name[0] == '.') || (dp->d_name[0] == '_')){
      continue;
    }

    // only keep those files that match the data time
    
    string fileName(dp->d_name);
    time_t fileDataTime = 0;
    bool dateOnly = false;
    if (DataFileNames::getDataTime(fileName, fileDataTime, dateOnly)) {
      continue;
    }
    if (fileDataTime != dataTime) {
      continue;
    }

    Path fullPath(dirPath, fileName);
    timedPathList.push_back(fullPath.getPath());

  } // dp
    
  closedir(dirp);
  return 0;
  
}


//////////////////////////////////////////////////////////////////////
// combine layers into single MDV volume

int MrmsGribIngest::_combineVolume()

{

  if (_params.debug) {
    cerr << "==>>  Combining volume, n layers: " << _layers.size() << endl;
    int ilayer = 0;
    for (LayersIter ii = _layers.begin(); ii != _layers.end(); ii++, ilayer++) {
      cerr << "    layer " << ilayer << " at ht: " << ii->first << endl;
    }
  }

  // init
  
  _outVol.clear();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _outVol.setDebug(true);
  }
  
  // check we have enough layers for a valid volume

  if ((_layers.size() < 1) ||
      ((int) _layers.size() < _params.min_n_ht_levels_for_volume)) {
    cerr << "WARNING - MrmsGribIngest::_combineVolume" << endl;
    cerr << "  Too few layers for valid volume: " << _layers.size() << endl;
    cerr << "  Need at least: " << _params.min_n_ht_levels_for_volume
         << " layers" << endl;
    cerr << "  data time: " << DateTime::strm(_latestDataTime) << endl;
    cerr << "  vol will be discarded" << endl;
    return -1;
  }

  // check we have at least 1 field

  LayersIter lowest = _layers.begin();
  const DsMdvx &mdvxLayer0 = *lowest->second;
  int nFields = mdvxLayer0.getNFields();
  if (nFields < 1) {
    cerr << "WARNING - MrmsGribIngest::_combineVolume" << endl;
    cerr << "  No field present in layer 0" << endl;
    cerr << "  data time: " << DateTime::strm(_latestDataTime) << endl;
    cerr << "  vol will be discarded" << endl;
    return -1;
  }
  
  // check the layers have the same number of fields
  
  int ilayer = 0;
  for (LayersIter ii = _layers.begin(); ii != _layers.end(); ii++, ilayer++) {
    const DsMdvx &mdvx = *ii->second;
    int nFieldsThisLayer = mdvx.getNFields();
    if (nFieldsThisLayer != nFields) {
      cerr << "WARNING - MrmsGribIngest::_combineVolume" << endl;
      cerr << "  Number of fields varies between layers, layer num: " << ilayer << endl;
      cerr << "  data time: " << DateTime::strm(_latestDataTime) << endl;
      cerr << "  nFields for layer 0: " << nFields << endl;
      cerr << "  nFields for this layer: " << nFieldsThisLayer << endl;
      cerr << "  vol will be discarded" << endl;
      return -1;
    }
  }
  
  // compute grid size
  
  MdvxField *layer0field0 = mdvxLayer0.getField(0);
  int nx00 = layer0field0->getFieldHeader().nx;
  int ny00 = layer0field0->getFieldHeader().ny;
  long long nptsLayer = nx00 * ny00;
  int nz = _layers.size();
  long long nptsVol = nptsLayer * nz;

  // check all fields have the same size

  for (int ifield = 0; ifield < nFields; ifield++) {
    int ilayer = 0;
    for (LayersIter ii = _layers.begin(); ii != _layers.end(); ii++, ilayer++) {
      const DsMdvx &layer = *ii->second;
      MdvxField *fieldLayer = layer.getField(ifield);
      int nx = fieldLayer->getFieldHeader().nx;
      int ny = fieldLayer->getFieldHeader().ny;
      if (nx != nx00 || ny != ny00) {
        cerr << "WARNING - MrmsGribIngest::_combineVolume" << endl;
        cerr << "  Field geometry differs between layers and fields" << endl;
        cerr << "  data time: " << DateTime::strm(_latestDataTime) << endl;
        cerr << "  nx00, ny00: " << nx00 << ", " << ny00 << endl;
        cerr << "  ilayer: " << ilayer << endl;
        cerr << "  ifield: " << ifield << endl;
        cerr << "  layer nx, ny: " << nx << ", " << ny << endl;
        cerr << "  vol will be discarded" << endl;
        return -1;
      }
    } // ii
  } // ifield
  
  // loop through the fields

  for (int ifield = 0; ifield < nFields; ifield++) {
    
    // get field for lowest layer, use this as template
    
    MdvxField *fieldLayer0 = mdvxLayer0.getField(ifield);
    const Mdvx::field_header_t &fhdrLayer0 = fieldLayer0->getFieldHeader();
    fl32 volMissing = fhdrLayer0.missing_data_value;

    // initialize field and vlevel headers
    
    Mdvx::field_header_t fhdrVol = fhdrLayer0;
    fhdrVol.nz = nz;
    fhdrVol.volume_size = nptsVol * sizeof(fl32);
    fhdrVol.compression_type = Mdvx::COMPRESSION_NONE;
    fhdrVol.dz_constant = false;

    Mdvx::vlevel_header_t vhdrVol;
    MEM_zero(vhdrVol);
    
    // create data array
    
    fl32 *volData = new fl32[nptsVol];
    fl32 *volPtr = volData;
    
    // loop through the layers, copying in the data
    
    int ilayer = 0;
    for (LayersIter ii = _layers.begin(); ii != _layers.end(); ii++, ilayer++) {

      const DsMdvx &layer = *ii->second;
      MdvxField *fieldLayer = layer.getField(ifield);
      const Mdvx::field_header_t &fhdrLayer = fieldLayer->getFieldHeader();
      fl32 layerMissing = fhdrLayer.missing_data_value;
      fl32 *layerData = (fl32 *) fieldLayer->getVol();
      fl32 *layerPtr = layerData;
      
      for (int ipt = 0; ipt < nptsLayer; ipt++, layerPtr++) {
        if (*layerPtr == layerMissing) {
          *volPtr = volMissing;
        } else {
          *volPtr = *layerPtr;
        }
        volPtr++;
      }
      
      vhdrVol.type[ilayer] = fhdrVol.vlevel_type;
      vhdrVol.level[ilayer] = _getHeightKm(layer);

    } // layer

    // create the volume field

    MdvxField *volField = new MdvxField(fhdrVol, vhdrVol, volData);
    if (_params.convert_to_column_max) {
      volField->convert2Composite();
    }
    delete[] volData;

    // add to output volume
    
    _outVol.addField(volField);
    
  } // field

  // set master header

  _outVol.setMasterHeader(mdvxLayer0.getMasterHeader());
  _outVol.updateMasterHeader();
  
  // remap projection

  if (_params.remap_output_grid) {
    _remapProjection(_outVol);
  }

  // convert fields, compress

  _encodeFields(_outVol);

  return 0;

}
//////////////////////////////////////////////////////////////
// get the height of the layer
// returns -9999 if no layer available

double MrmsGribIngest::_getHeightKm(const DsMdvx &mdvx) const
{
 
  if (mdvx.getNFields() < 1) {
    return -9999.0;
  }
  
  const MdvxField *field = mdvx.getField(0);
  
  return field->getFieldHeader().grid_minz;

}

//////////////////////////////////////////////////////////////
// get the data time
// returns -9999 if no layer available

time_t MrmsGribIngest::_getDataTime(const DsMdvx &mdvx) const
{
  
  if (mdvx.getNFields() < 1) {
    return -9999;
  }
  
  return mdvx.getValidTime();
  
}

////////////////////////////////////////////
// remap output projection

void MrmsGribIngest::_remapProjection(DsMdvx &mdvx)

{

  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);

    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_remap" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    if (_params.remap_projection == Params::PROJ_LATLON) {
      field->remap2Latlon(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      field->remap2Flat(_remapLut,
			_params.remap_grid.nx,
			_params.remap_grid.ny,
			_params.remap_grid.minx,
			_params.remap_grid.miny,
			_params.remap_grid.dx,
			_params.remap_grid.dy,
			_params.remap_origin_lat,
			_params.remap_origin_lon,
			_params.remap_rotation,
                        _params.remap_false_northing,
                        _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF)	{
      field->remap2LambertConf(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_lat1,
			       _params.remap_lat2,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_POLAR_STEREO) {
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (!_params.remap_pole_is_north) {
	poleType = Mdvx::POLE_SOUTH;
      }
      field->remap2PolarStereo(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_tangent_lon,
			       poleType,
			       _params.remap_central_scale,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      field->remap2ObliqueStereo(_remapLut,
				 _params.remap_grid.nx,
				 _params.remap_grid.ny,
				 _params.remap_grid.minx,
				 _params.remap_grid.miny,
				 _params.remap_grid.dx,
				 _params.remap_grid.dy,
				 _params.remap_origin_lat,
				 _params.remap_origin_lon,
				 _params.remap_tangent_lat,
				 _params.remap_tangent_lon,
                                 _params.remap_false_northing,
                                 _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_MERCATOR) {
      field->remap2Mercator(_remapLut,
			    _params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
                            _params.remap_false_northing,
                            _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_TRANS_MERCATOR) {
      field->remap2TransverseMercator(_remapLut,
				      _params.remap_grid.nx,
				      _params.remap_grid.ny,
				      _params.remap_grid.minx,
				      _params.remap_grid.miny,
				      _params.remap_grid.dx,
				      _params.remap_grid.dy,
				      _params.remap_origin_lat,
				      _params.remap_origin_lon,
				      _params.remap_central_scale,
                                      _params.remap_false_northing,
                                      _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_ALBERS) {
      field->remap2Albers(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy,
			  _params.remap_origin_lat,
			  _params.remap_origin_lon,
			  _params.remap_lat1,
			  _params.remap_lat2,
                          _params.remap_false_northing,
                          _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_AZIM) {
      field->remap2LambertAzimuthal(_remapLut,
				    _params.remap_grid.nx,
				    _params.remap_grid.ny,
				    _params.remap_grid.minx,
				    _params.remap_grid.miny,
				    _params.remap_grid.dx,
				    _params.remap_grid.dy,
				    _params.remap_origin_lat,
				    _params.remap_origin_lon,
                                    _params.remap_false_northing,
                                    _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_VERT_PERSP) {
      field->remap2VertPersp(_remapLut,
                             _params.remap_grid.nx,
                             _params.remap_grid.ny,
                             _params.remap_grid.minx,
                             _params.remap_grid.miny,
                             _params.remap_grid.dx,
                             _params.remap_grid.dy,
                             _params.remap_origin_lat,
                             _params.remap_origin_lon,
                             _params.remap_persp_radius,
                             _params.remap_false_northing,
                             _params.remap_false_easting);
     }

  } // ifld
  
}

////////////////////////////////////////////
// encode the output fields as requested

void MrmsGribIngest::_encodeFields(DsMdvx &mdvx)

{

  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_remap" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }

    if (_params.output_encoding == Params::ENCODING_INT8) {

      field->convertDynamic(Mdvx::ENCODING_INT8,
                            Mdvx::COMPRESSION_GZIP);

    } else if (_params.output_encoding == Params::ENCODING_INT16) {

      field->convertDynamic(Mdvx::ENCODING_INT16,
                            Mdvx::COMPRESSION_GZIP);

    } else {

      // float32

      field->convertDynamic(Mdvx::ENCODING_FLOAT32,
                            Mdvx::COMPRESSION_GZIP);
      
    }

  } // ifld

}
