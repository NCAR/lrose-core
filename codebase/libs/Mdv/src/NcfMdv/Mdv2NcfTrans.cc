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

/////////////////////////////////////////////////////////////
// Mdv2NcfTrans.hh
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2008
//
///////////////////////////////////////////////////////////////
//
// Mdv2NcfTrans class. Read the Mdv file, record time information and create
// unique NcfFieldData, NcfGridInfo and NcfVlevelInfo
// objects as necessary
//
///////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cassert>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <toolsa/Path.hh>
#include <Mdv/NcfMdv.hh>
#include <Mdv/Mdv2NcfTrans.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxRadar.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/DoradeRadxFile.hh>
#include <Radx/UfRadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxSweep.hh>

// Constructor

Mdv2NcfTrans::Mdv2NcfTrans()

{

  // initialize data members

  _debug = false;
  _heartbeatFunc = NULL;
  _isXSect = false;
  _isPolar = false;
  _isRhi = false;
  _mdv = NULL;
  _outputLatlonArrays = true;

  _ncFile = NULL;
  _ncFormat = Nc3File::Netcdf4;
  _radxNcFormat = RadxFile::NETCDF4;
  _ncErr = NULL;

  _radialFileType = DsMdvx::RADIAL_TYPE_CF;


  _initVars();
  clearErrStr();

}

// Destructor

Mdv2NcfTrans::~Mdv2NcfTrans()
{
  clearData();
}

// Clear the data, ready for reuse

void Mdv2NcfTrans::clearData()
{

  _closeNcFile();

  // Memory cleanup

  for (int i = 0; i < (int)_gridInfo.size(); i++) {
    delete _gridInfo[i];
  }
  _gridInfo.clear();
  
  for (int i = 0; i < (int)_vlevelInfo.size(); i++) {
    delete _vlevelInfo[i];
  }
  _vlevelInfo.clear();

  for (int i = 0; i < (int)_fieldData.size(); i++) {
    delete _fieldData[i]; 
  }
  _fieldData.clear();

  _initVars();
  _fieldNameSet.clear();
  clearErrStr();

}

///////////////////////////////////////////////////////////////////////////
// Parse Mdv data and record time information and create  unique NcfFieldData,
// NcfGridInfo, and NcfVlevelInfo objects as necessary
//
// Returns 0 on success, -1 on failure

int Mdv2NcfTrans::translate(const DsMdvx &mdv, const string &ncFilePath)

{

  _mdv = &mdv;
  _ncFilePath = ncFilePath;
  _outputLatlonArrays = _mdv->_ncfOutputLatlonArrays;
  _outputStartEndTimes = _mdv->_ncfOutputStartEndTimes;
  char *outputLatlonArraysStr = getenv("MDV2NETCDF_WRITE_LATLON_ARRAYS");
  if (outputLatlonArraysStr != NULL) {
    if (!strcasecmp(outputLatlonArraysStr, "FALSE")) {
      _outputLatlonArrays = false;
    }
  }

  // check for polar radar projection, and x-section

  _isXSect = false;
  _isPolar = false;
  if (mdv.getNFields() > 0) {
    const MdvxField *field = mdv.getField(0);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    if (fhdr.proj_type == Mdvx::PROJ_VSECTION) {
      _isXSect = true;
    } else if (fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
      _isPolar = true;
    }
  }

  // check for polar-RHI
  
  if (_isPolar && mdv.getMasterHeader().vlevel_type == Mdvx::VERT_TYPE_AZ) {
    _isRhi = true;
  } else {
    _isRhi = false;
  }

  // set the translation parameters

  _setTransParams();

  // clear data from previous use

  clearData();

  // parse the MDV file

  if (_parseMdv()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::translate");
    TaStr::AddStr(_errStr, "  Parsing MDV file, path:", _mdv->getPathInUse());
    return -1;
  }

  // open the output Nc file

  if (_openNcFile(_ncFilePath)) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::translate");
    TaStr::AddStr(_errStr, "  Opening Nc File, path: ", _ncFilePath);
    return -1;
  }

  // write the Nc file

  if (_writeNcFile()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::translate");
    TaStr::AddStr(_errStr, "  Writing Nc File, path: ", _ncFilePath);
    return -1;
  }

  // close the Nc file

  _closeNcFile();

  return 0;

}

////////////////////////
// Initialize variables

void Mdv2NcfTrans::_initVars()

{

  _timeBegin = 0;
  _timeEnd = 0;
  _timeCentroid = 0;
  _timeExpire = 0;
  _timeGen = 0;
  _timeValid = 0;
  _forecastTime = 0;
  _leadTime = 0;
  _isForecast = false;
  
  _timeDim = NULL;
  _boundsDim = NULL;
  _timeVar = NULL;
  _forecastRefTimeVar = NULL;
  _forecastPeriodVar = NULL;
  _startTimeVar = NULL;
  _stopTimeVar = NULL;
  _timeBoundsVar = NULL;
  
}

//////////////////////////////
// set translation parameters

void Mdv2NcfTrans::_setTransParams()

{

  // set nc file type
  
  switch (_mdv->_ncfFileFormat) {
    case DsMdvx::NCF_FORMAT_NETCDF4:
      _ncFormat = Nc3File::Netcdf4;
      _radxNcFormat = RadxFile::NETCDF4;
      break;
    case DsMdvx::NCF_FORMAT_CLASSIC:
      _ncFormat = Nc3File::Classic;
      _radxNcFormat = RadxFile::NETCDF_CLASSIC;
      break;
    case DsMdvx::NCF_FORMAT_OFFSET64BITS:
      _ncFormat = Nc3File::Offset64Bits;
      _radxNcFormat = RadxFile::NETCDF_OFFSET_64BIT;
      break;
    case DsMdvx::NCF_FORMAT_NETCFD4_CLASSIC:
      _ncFormat = Nc3File::Netcdf4Classic;
      _radxNcFormat = RadxFile::NETCDF4_CLASSIC;
      break;
    default:
      _ncFormat = Nc3File::Netcdf4;
      _radxNcFormat = RadxFile::NETCDF4;
  }

}

//////////////////////////////////////////////////////////////
// Parse Mdv data and record time information and create unique
// NcfFieldData, NcfGridInfo, and NcfVlevelInfo  objects as necessary
//
// Returns 0 on success, -1 on failure

int Mdv2NcfTrans::_parseMdv()

{

  // Get the relevant data

  const Mdvx::master_header_t &mhdr = _mdv->getMasterHeader();
  
  _timeBegin = mhdr.time_begin;
  _timeEnd = mhdr.time_end;
  _timeCentroid = mhdr.time_centroid;
  _timeExpire = mhdr.time_expire;
  _timeGen = mhdr.time_gen;
  _timeValid = mhdr.time_centroid;
  _datasetInfo = _mdv->getDataSetInfo();
  _datasetName = mhdr.data_set_name;
  _datasetSource = mhdr.data_set_source;

  if (mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED) {
    _isForecast = true;
  } else {
    _isForecast = false;
  }

  // Loop through the fields. If the data is not in a CF-1.0 supported 
  // projection remap to lat lon. 2D grids and  projection information is
  // store in a NcfGridInfo object. Vertical level data information is
  // stored in a NcfVlevelInfo object.
  // For each field we create new ones if necessary
  // otherwise set pointers to the appropriate objects.
  
  for (int i = 0; i < _mdv->getNFields(); i++) {    

    const MdvxField *field = _mdv->getField(i);     
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    string mdvFieldName = fhdr.field_name;
    string mdvLongFieldName = fhdr.field_name_long;
    
    // set up field names etc

    string ncfFieldName = mdvFieldName;
    string ncfStandardName = mdvFieldName;
    string ncfLongName = fhdr.field_name_long;
    string ncfUnits = fhdr.units;
    bool doLinearTransform = false;
    double linearMult = 1.0;
    double linearOffset = 0.0;
    DsMdvx::ncf_pack_t packing = DsMdvx::NCF_PACK_ASIS;
    
    // check if this field has been specified for translation
    
    for (int jj = 0; jj < (int) _mdv->_mdv2NcfTransArray.size(); jj++) {
      
      // check for match on MDV field name
      
      const DsMdvx::Mdv2NcfFieldTrans &trans = _mdv->_mdv2NcfTransArray[jj];

      if (mdvFieldName.compare(trans.mdvFieldName) == 0 ||
	  mdvLongFieldName.compare(trans.mdvFieldName) == 0) {
        
        // match - so set accordingly

        ncfFieldName = trans.ncfFieldName;
        ncfStandardName = trans.ncfStandardName;
        ncfLongName = trans.ncfLongName;
        ncfUnits = trans.ncfUnits;
        doLinearTransform = trans.doLinearTransform;
        linearMult = trans.linearMult;
        linearOffset = trans.linearOffset;
        packing = trans.packing;
        break;

      }

    } // jj

    // make sure the field names are compliant

    ncfFieldName = _getCfCompliantName(ncfFieldName);

    // make sure the field names are unique

    ncfFieldName = _getUniqueFieldName(ncfFieldName);

    // set forecast lead time from master header or
    // first field if master value is missing

    if (i == 0) {
      _leadTime = mhdr.forecast_delta;
      if (_leadTime == 0 && fhdr.forecast_delta != 0) {
        _leadTime = fhdr.forecast_delta;
      }
      _forecastTime = mhdr.forecast_time;
      if (_forecastTime == 0 && fhdr.forecast_time != 0) {
        _forecastTime = fhdr.forecast_time;
      }
    }
      
    // Create projection and grid information object

    NcfGridInfo *gridInfo = new NcfGridInfo(fhdr);
      
    // Keep it only if it is a projection/grid we havent 
    // seen before.

    bool gridIsNew = true;
    for (int j = 0; j < (int)_gridInfo.size(); j++) {
      if (*gridInfo == *_gridInfo[j]) {
        delete gridInfo;
        gridIsNew = false;
        gridInfo = _gridInfo[j];
        j = _gridInfo.size();
      }	
    }
      
    if (gridIsNew) {

      if (_isXSect) {	

        // If this is Xsection, then get the lat/lons from the samplePoints
        
        MdvxChunk *pChunk= _mdv->getChunkById(Mdvx::CHUNK_VSECT_SAMPLE_PTS);
        if(pChunk == NULL){
            TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::_parseMdv");
            TaStr::AddStr(_errStr, "ERROR - No chunks found...exiting");
            return -1;

        }
        MemBuf buf;
        buf.add(pChunk->getData(), pChunk->getSize());	  
        vector<Mdvx::vsect_samplept_t> pts;
        double dxKm;
        string errStr;
	      
        if (Mdvx::disassembleVsectSamplePtsBuf(buf, pts, dxKm, errStr)) {
          TaStr::AddStr(_errStr,
                        "ERROR - disassembleVSectSamplePtsBuf",
                        "  Bad sample point buffer\n");
        }
        
        gridInfo->setCoordinateVarsFromSamplePoints(pts);
	      
      } else {

        // compute x and y coordinate arrays and corresponding
        // arrays of lat lon data
        
        gridInfo->computeCoordinateVars();
	      
      }

      _gridInfo.push_back(gridInfo);

    }
      
    // Create NcfVlevelInfo object for vlevel_header_t struct

    Mdvx::vlevel_header_t vlev = field->getVlevelHeader();
      
    // Note that we pass in the vlevel_type and nz. It is possible that
    // the vlevel header doesnt get filled out properly and the type is not
    // set.

    NcfVlevelInfo *vlevelInfo = new NcfVlevelInfo(vlev, fhdr.vlevel_type, fhdr.nz);
      
    // Keep vlevInfo only if it is a projection/grid we havent 
    // seen before.

    bool vlevelIsNew = true;
      
    for (int j = 0; j < (int)_vlevelInfo.size(); j++) {
      if ( *vlevelInfo == *_vlevelInfo[j] ) {
        delete vlevelInfo;
        vlevelIsNew = false;
        vlevelInfo = _vlevelInfo[j];
        j = _vlevelInfo.size();
      }
    }
      
    if (vlevelIsNew) {
      _vlevelInfo.push_back(vlevelInfo);
    }
      
    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("Mdv2NcfTrans::_parseMdv");
    }

    // Create NcfFieldData object

    NcfFieldData *fieldData = new NcfFieldData(_debug, field, gridInfo, vlevelInfo,
                                         mdvFieldName, ncfFieldName,
                                         ncfStandardName, ncfLongName,
                                         ncfUnits, doLinearTransform,
                                         linearMult, linearOffset,
                                         packing,
                                         _outputLatlonArrays,
                                         _mdv->_ncfCompress,
                                         _mdv->_ncfCompressionLevel,
                                         _ncFormat);
    
    _fieldData.push_back(fieldData);
    
  } // for all fields

  return 0;

}

//////////////////////////////////////////////
// open netcdf file
// create error object so we can handle errors
// Returns 0 on success, -1 on failure

int Mdv2NcfTrans::_openNcFile(const string &path)
  
{

  _closeNcFile();

  // make directory if needed

  Path ppath(path);
  if (ppath.makeDirRecurse()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::openNcFile");
    TaStr::AddStr(_errStr, "  Cannot make dir: ", ppath.getDirectory());
  }

  _ncFile = new Nc3File(path.c_str(), Nc3File::Replace, NULL, 0, _ncFormat);
  
  if (!_ncFile || !_ncFile->is_valid()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::openNcFile");
    TaStr::AddStr(_errStr, "  Cannot open netCDF file: ", path);
    if (_ncFile) {
      delete _ncFile;
      _ncFile = NULL;
    }
    return -1;
  }
  
  // Change the error behavior of the netCDF C++ API by creating an
  // Nc3Error object. Until it is destroyed, this Nc3Error object will
  // ensure that the netCDF C++ API returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program.
  
  _ncErr = new Nc3Error(Nc3Error::silent_nonfatal);
 
  return 0;

}

//////////////////////////////////////
// close netcdf file if open
// remove error object if it exists

void Mdv2NcfTrans::_closeNcFile()
  
{
  
  // close file if open, delete ncFile
  
  if (_ncFile) {
    _ncFile->close();
    delete _ncFile;
    _ncFile = NULL;
  }

  if (_ncErr) {
    delete _ncErr;
    _ncErr = NULL;
  }

}

//////////////////////////////////////
// write data to NetCDF file
//
// Returns 0 on success, -1 on failure

int Mdv2NcfTrans::_writeNcFile()
  
{
  
  // Add data to the netCDF file.
  // A note from the NetCDF documentation:
  // Be aware that switching from accessing data to adding or renaming
  // dimensions, variables and attributes can be expensive,
  // since it may entail a copy of the data.
  // So we add all atributes and variables first and then do the put of
  // the data. This doesnt make for the most efficient coding since we
  // duplicate some data packing code in the add variable section and
  // in the put variable section.
  // This will not be necessary with netCDF4.

  if (_addGlobalAttributes()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  adding global attributes to NcFile: ", _ncFilePath);
    return -1;
  }

  // add dimensions

  if (_addDimensions()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  adding dimensions to NcFile: ", _ncFilePath);
    return -1;
  };


  // add variables

  if (_addTimeVariables()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  adding time vars to NcFile: ", _ncFilePath);
    return -1;
  };

  if (_addCoordinateVariables()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  adding coordinate vars to NcFile: ", _ncFilePath);
    return -1;
  };

  if (_addProjectionVariables()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  adding projection vars to NcFile: ", _ncFilePath);
    return -1;
  };
  
  if (_mdv->_ncfOutputMdvAttr) {
    if (_addMdvMasterHeaderVariable()) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
      TaStr::AddStr(_errStr, "  adding master header var to NcFile: ", _ncFilePath);
      return -1;
    }
  }

  if (_mdv->_ncfOutputMdvChunks) {
    if (_addMdvChunkVariables()) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
      TaStr::AddStr(_errStr, "  adding chunk vars to NcFile: ", _ncFilePath);
      return -1;
    }
  }
  
  if(_addFieldDataVariables()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  adding data vars to NcFile: ", _ncFilePath);
    return -1;
  };

  // put data to the file

  if (_putTimeVariables()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  putting time vars to NcFile: ", _ncFilePath);
    return -1;
  };


  if (_putCoordinateVariables()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  putting coord vars to NcFile: ", _ncFilePath);
    return -1;
  };
   
  if (_mdv->_ncfOutputMdvChunks) {
    if (_putMdvChunkVariables()) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
      TaStr::AddStr(_errStr, "  putting chunks to NcFile: ", _ncFilePath);
      return -1;
    }
  }
  
  if (_putFieldDataVariables()) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
    TaStr::AddStr(_errStr, "  putting field data vars to NcFile: ", _ncFilePath);
    return -1;
  };

  return 0;

}

///////////////////////////////////////////////////////////////
// addGlobalAttributes()
//

int Mdv2NcfTrans::_addGlobalAttributes()
{
  
  if (_debug) {
    cerr << "Mdv2NcfTrans::addGlobalAttributes()" << endl;
  }

  int iret = 0;

  // Add required CF-1.0 global attributes

  iret |= !_ncFile->add_att("Conventions" , "CF-1.6");

  // history: from the mdv master header data_set_info

  if (_datasetInfo.size() > 0) {
    iret |= !_ncFile->add_att(NcfMdv::history,  _datasetInfo.c_str());
  }
  if (_mdv->_ncfInstitution.size() > 0) {
    iret |= !_ncFile->add_att(NcfMdv::institution ,
                              _mdv->_ncfInstitution.c_str());
  }
  if (_mdv->_ncfReferences.size() > 0) {
    iret |= !_ncFile->add_att(NcfMdv::references ,
                              _mdv->_ncfReferences.c_str());
  }

  // source

  iret |= !_ncFile->add_att(NcfMdv::source , _datasetSource.c_str());
  
  // title: use data set name

  iret |= !_ncFile->add_att(NcfMdv::title , _datasetName.c_str());

  // comment

  iret |= !_ncFile->add_att(NcfMdv::comment , _mdv->_ncfComment.c_str());

  return (iret? -1 : 0);

}

///////////////////////////////////////////////////////////////////////////
// addDimensions()
//
//  Add NcDims to the NetCDF file. We loop through the
//  NcfGridInfo objects and record the dimensions of the
//  x and y coordinates. Then we loop through the NcfVlevelInfo
//  objects and record the dimensions of the vertical coordinates

int Mdv2NcfTrans::_addDimensions()
{

  if (_debug) {
    cerr << "Mdv2NcfTrans::addDimensions()" << endl;
  }

  // add time dimension

  if (!(_timeDim = _ncFile->add_dim(NcfMdv::time, 1))) {
    return -1;
  }

  // add bounds dimension

  if (!(_boundsDim = _ncFile->add_dim(NcfMdv::bounds, 2))) {
    return -1;
  }

  //
  // Add dimensions of the different 2D grids
  //
  
  for (int i = 0; i < (int) _gridInfo.size(); i++) {
    if (_gridInfo[i]->addXyDim(i, _ncFile, _errStr)) {
      TaStr::AddStr(_errStr, "Mdv2NcfTrans::_addDimensions");
      TaStr::AddStr(_errStr, "  Cannot add XY grid dimensions");
      return -1;
    }
  }

  // Add dimensions of different vertical levels
  
  for (int i = 0; i <  (int) _vlevelInfo.size(); i++) {
    if (_vlevelInfo[i]->addDim(i, _ncFile, _errStr)) {
      TaStr::AddStr(_errStr, "Mdv2NcfTrans::_addDimensions");
      TaStr::AddStr(_errStr, "  Cannot add Vlevel grid dimensions");
      return -1;
    }
  }

  // Add dimensions for chunks

  if (_mdv->_ncfOutputMdvChunks) {
    for (int i = 0; i < _mdv->getNChunks(); i++) {
      const MdvxChunk *chunk = _mdv->getChunkByNum(i);
      char chunkName[128];
      sprintf(chunkName, "%s_%.4d", NcfMdv::nbytes_mdv_chunk, i);
      Nc3Dim *chunkDim = _ncFile->add_dim(chunkName, chunk->getSize());
      if (chunkDim == NULL) {
        return -1;
      }
      _chunkDims.push_back(chunkDim);
    }
  }
  
  return 0;

}

////////////////////////////////////////////////
// add variables and attributes for coordinates

int Mdv2NcfTrans::_addCoordinateVariables()
{

  // vertical section has its own code

  if (_isXSect) {
    return _addVsectCoordinateVariables();
  }

  if (_debug) {
    cerr << "Mdv2NcfTrans::addCoordinateVariables()" << endl;
  }

  // Note that coordinate variables have the same name as their dimension
  // so we will use the same naming scheme for vars as we did for dimensions
  // in method _addDimensions()

  for (int i = 0; i < (int) _gridInfo.size(); i++) {	

    if (_gridInfo[i]->addCoordVars(i, _outputLatlonArrays,
                                   _ncFile, _errStr)) {
      TaStr::AddStr(_errStr, "Mdv2NcfTrans::_addCoordinateVariables");
      TaStr::AddStr(_errStr, "  Cannot add coordinate vars");
      return -1;
    }

  }

  // Add vertical coordinate variables

  for (int i = 0; i < (int) _vlevelInfo.size(); i++) {
    if (_vlevelInfo[i]->addVlevelVar(i, _ncFile, _errStr)) {
      return -1;
    }
  }

  return 0;

}

////////////////////////////////////////////////////////////
// add variables and attributes for vert section coordinates

int Mdv2NcfTrans::_addVsectCoordinateVariables()
{

  if (_debug) {
    cerr << "Mdv2NcfTrans::addVsectCoordinateVariables()" << endl;
  }
  
  // If this is a XSection, add x, y, lat, long, alt
  // We should only have one grid in an mdv Xsection.
  
  for (int i = 0; i < (int) _gridInfo.size(); i++) {	
    if (_gridInfo[i]->addVsectCoordVars(i, _ncFile, _errStr)) {
      TaStr::AddStr(_errStr, "Mdv2NcfTrans::_addCoordinateVariables");
      TaStr::AddStr(_errStr, "  Cannot add coordinate vars");
      return -1;
    }
  }
  
  // Add vertical coordinate variable
  
  for (int i = 0; i < (int) _vlevelInfo.size(); i++) {
    if (_vlevelInfo[i]->addVlevelVar(i, _ncFile, _errStr)) {
      return -1;
    }
  }

  return 0;
  
}

//////////////////////////////////////////////
// add variables and attributes for projection

int Mdv2NcfTrans::_addProjectionVariables()
{

  for(int i = 0; i < (int) _gridInfo.size(); i++) {
    if (_gridInfo[i]->addProjVar(i, _ncFile, _errStr)) {
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////
// add variables for times

int Mdv2NcfTrans::_addTimeVariables()
{

  int iret = 0;

  if (_debug)
    cerr << "Mdv2NetCDF::_addTimeVariables()" << endl;

  // time

  string timeStr("time");
  
  if ((_timeVar = _ncFile->add_var(timeStr.c_str(), nc3Double, _timeDim)) == NULL) {
    return -1;
  }
  iret |= !_timeVar->add_att(NcfMdv::standard_name, NcfMdv::time);
  iret |= !_timeVar->add_att(NcfMdv::long_name, "Data time");
  iret |= !_timeVar->add_att(NcfMdv::units, NcfMdv::secs_since_jan1_1970);
  iret |= !_timeVar->add_att(NcfMdv::axis, "T");
  iret |= !_timeVar->add_att(NcfMdv::bounds, "time_bounds");

  DateTime timeCentroid(_timeCentroid);
  iret |= !_timeVar->add_att(NcfMdv::comment, timeCentroid.getW3cStr().c_str());
  
  // forecast_reference_time (or generation time)
  
  if (_isForecast) {

    if ((_forecastRefTimeVar =
         _ncFile->add_var(NcfMdv::forecast_reference_time, nc3Double, _timeDim)) == NULL) {
      return -1;
    }
    iret |= !_forecastRefTimeVar->add_att(NcfMdv::standard_name,
                                          NcfMdv::forecast_reference_time);
    iret |= !_forecastRefTimeVar->add_att(NcfMdv::long_name, "Data generation time");
    iret |= !_forecastRefTimeVar->add_att(NcfMdv::units, NcfMdv::secs_since_jan1_1970);
    
    DateTime genTime(_timeGen);
    _forecastRefTimeVar->add_att(NcfMdv::comment, genTime.getW3cStr().c_str());
  
    // forecast period or lead time

    string forecastPeriodStr(NcfMdv::forecast_period);
    
    if ((_forecastPeriodVar =
         _ncFile->add_var(forecastPeriodStr.c_str(), nc3Double, _timeDim)) == NULL) {
      return -1;
    }
    
    iret |= !_forecastPeriodVar->add_att(NcfMdv::standard_name, NcfMdv::forecast_period);
    iret |= !_forecastPeriodVar->add_att(NcfMdv::long_name, NcfMdv::forecast_period);
    iret |= !_forecastPeriodVar->add_att(NcfMdv::units, NcfMdv::seconds);

  } // if (_isForecast) ...
	  
  // start time, stop time and time_bounds

  DateTime startTime(_timeBegin);
  DateTime stopTime(_timeEnd);
  string startTimeString(startTime.getW3cStr().c_str());
  string stopTimeString(stopTime.getW3cStr().c_str());
 
  if ( _outputStartEndTimes ) {

     if (_timeBegin != 0) {
       if ((_startTimeVar = _ncFile->add_var(NcfMdv::start_time,
                                             nc3Double, _timeDim)) == NULL) {
         return -1;
       }
       iret |= !_startTimeVar->add_att(NcfMdv::long_name, "start_time");
       iret |= !_startTimeVar->add_att(NcfMdv::units, NcfMdv::secs_since_jan1_1970);
       iret |= !_startTimeVar->add_att(NcfMdv::comment, startTime.getW3cStr().c_str());
     }

     if (_timeEnd != 0) {
       if ((_stopTimeVar = _ncFile->add_var(NcfMdv::stop_time,
                                            nc3Double, _timeDim)) == NULL) {
         return -1;
       }
       iret |= !_stopTimeVar->add_att(NcfMdv::long_name, "stop_time");
       iret |= !_stopTimeVar->add_att(NcfMdv::units, NcfMdv::secs_since_jan1_1970);
       iret |= !_stopTimeVar->add_att(NcfMdv::comment, stopTime.getW3cStr().c_str());
     }
  
     if (_timeBegin != _timeEnd &&
         _timeBegin <= _timeCentroid &&
         _timeEnd >= _timeCentroid) {
       if ((_timeBoundsVar = _ncFile->add_var(NcfMdv::time_bounds, nc3Double,
                                              _timeDim, _boundsDim)) == NULL) {
         return -1;
       }
       iret |= !_timeBoundsVar->add_att(NcfMdv::comment, "time_bounds also stored the start and stop times, provided the time variable value lies within the start_time to stop_time interval");
       iret |= !_timeBoundsVar->add_att(NcfMdv::units, NcfMdv::secs_since_jan1_1970);
     }
  } 
  return (iret? -1 : 0);

}

//////////////////////
// add field variables

int Mdv2NcfTrans::_addFieldDataVariables()

{

  if (_debug) {
    cerr << "Mdv2NcfTrans::addFieldVariables()" << endl;
  }

  for (int i = 0; i < (int) _fieldData.size(); i++) {
    if (_fieldData[i]->addToNc(_ncFile, _timeDim,
                               _mdv->_ncfOutputMdvAttr, _errStr)) {
      return -1;
    }
  }

  return 0;

}

///////////////////////////////////////////////////////////////
// add variable for MDV master header

int Mdv2NcfTrans::_addMdvMasterHeaderVariable()
{
  
  if (_debug) {
    cerr << "Mdv2NcfTrans::addMdvMasterHeaderVariable()" << endl;
  }

  int iret = 0;

  if (_mdv->_ncfOutputMdvAttr) {
    
    // create a variable to hold the master header attributes
    
    Nc3Var *mhdrVar = _ncFile->add_var(NcfMdv::mdv_master_header, nc3Int, _timeDim);
    if (mhdrVar == NULL) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::writeFile");
      TaStr::AddStr(_errStr, "  Adding master header var");
      TaStr::AddStr(_errStr, _ncErr->get_errmsg());
      return -1;
    }

    const Mdvx::master_header_t &mhdr = _mdv->getMasterHeader();
    iret |= !mhdrVar->add_att(NcfMdv::mdv_revision_number, mhdr.revision_number);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_epoch, mhdr.epoch);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_time_centroid, (double) mhdr.time_centroid);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_time_gen, (double) mhdr.time_gen);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_time_begin, (double) mhdr.time_begin);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_time_end, (double) mhdr.time_end);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_time, (double) mhdr.user_time);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_time_expire, (double) mhdr.time_expire);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_time_written, (double) mhdr.time_written);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_forecast_time, (double) mhdr.forecast_time);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_forecast_delta, (double) mhdr.forecast_delta);

    iret |= !mhdrVar->add_att(NcfMdv::mdv_data_collection_type,
                              mhdr.data_collection_type);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data, mhdr.user_data);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_vlevel_type, mhdr.vlevel_type);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_native_vlevel_type, mhdr.native_vlevel_type);

    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_si32_0, mhdr.user_data_si32[0]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_si32_1, mhdr.user_data_si32[1]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_si32_2, mhdr.user_data_si32[2]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_si32_3, mhdr.user_data_si32[3]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_si32_4, mhdr.user_data_si32[4]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_si32_5, mhdr.user_data_si32[5]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_si32_6, mhdr.user_data_si32[6]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_si32_7, mhdr.user_data_si32[7]);

    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_fl32_0, mhdr.user_data_fl32[0]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_fl32_1, mhdr.user_data_fl32[1]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_fl32_2, mhdr.user_data_fl32[2]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_fl32_3, mhdr.user_data_fl32[3]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_fl32_4, mhdr.user_data_fl32[4]);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_user_data_fl32_5, mhdr.user_data_fl32[5]);

    iret |= !mhdrVar->add_att(NcfMdv::mdv_sensor_lon, mhdr.sensor_lon);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_sensor_lat, mhdr.sensor_lat);
    iret |= !mhdrVar->add_att(NcfMdv::mdv_sensor_alt, mhdr.sensor_alt);

  }

  return (iret? -1 : 0);

}

//////////////////////////
// add MDV chunk variables

int Mdv2NcfTrans::_addMdvChunkVariables()

{

  int iret = 0;

  if (_debug) {
    cerr << "Mdv2NcfTrans::addMdvChunkVariables()" << endl;
  }
  
  for (int i = 0; i < _mdv->getNChunks(); i++) {

    if (i >= (int) _chunkDims.size()) {
      break;
    }
    const MdvxChunk *chunk = _mdv->getChunkByNum(i);
    char chunkName[128];
    sprintf(chunkName, "%s_%.4d", NcfMdv::mdv_chunk, i);
    
     Nc3Var *chunkVar =
       _ncFile->add_var(chunkName, nc3Byte, _timeDim, _chunkDims[i]);

     if (chunkVar == NULL) {
       return -1;
     }

     iret |= !chunkVar->add_att(NcfMdv::id, chunk->getId());
     iret |= !chunkVar->add_att(NcfMdv::size, chunk->getSize());
     iret |= !chunkVar->add_att(NcfMdv::info, chunk->getInfo().c_str());
    
     _chunkVars.push_back(chunkVar);

  }
  
  // radar-specific chunks? add global attributes
  
  MdvxRadar radar;

  if (radar.loadFromMdvx(*_mdv) == 0) {

    if (_debug) {
      cerr << "Mdv2NcfTrans::_addMdvChunkVariables()" << endl;
      cerr << "  handling radar chunk" << endl;
      radar.print(cerr);
    }

    // radar params

    if (radar.radarParamsAvail()) {
      const DsRadarParams &rparams = radar.getRadarParams();
      _ncFile->add_att("radar_name", rparams.radarName.c_str());
      _ncFile->add_att("radar_id", rparams.radarId);
      _ncFile->add_att("radar_type",
                       DsRadarParams::radarType2Str(rparams.radarType).c_str());
      _ncFile->add_att("radar_numFields", rparams.numFields);
      _ncFile->add_att("radar_numGates", rparams.numGates);
      _ncFile->add_att("radar_samplesPerBeam", rparams.samplesPerBeam);
      _ncFile->add_att("radar_scanType", rparams.scanType);
      _ncFile->add_att("radar_scanTypeName", rparams.scanTypeName.c_str());
      _ncFile->add_att("radar_scanMode",
                       DsRadarParams::scanMode2Str(rparams.scanMode).c_str());
      _ncFile->add_att("radar_followMode",
                       DsRadarParams::followMode2Str(rparams.followMode).c_str());
      _ncFile->add_att("radar_polarization",
                       DsRadarParams::polType2Str(rparams.polarization).c_str());
      _ncFile->add_att("radar_prfMode",
                       DsRadarParams::prfMode2Str(rparams.prfMode).c_str());
      _ncFile->add_att("radar_constant", rparams.radarConstant);
      _ncFile->add_att("radar_altitude", rparams.altitude);
      _ncFile->add_att("radar_latitude", rparams.latitude);
      _ncFile->add_att("radar_longitude", rparams.longitude);
      _ncFile->add_att("radar_gateSpacing", rparams.gateSpacing);
      _ncFile->add_att("radar_startRange", rparams.startRange);
      _ncFile->add_att("radar_horizBeamWidth", rparams.horizBeamWidth);
      _ncFile->add_att("radar_vertBeamWidth", rparams.vertBeamWidth);
      _ncFile->add_att("radar_pulseWidth", rparams.pulseWidth);
      _ncFile->add_att("radar_pulseRepFreq", rparams.pulseRepFreq);
      _ncFile->add_att("radar_prt", rparams.prt);
      _ncFile->add_att("radar_prt2", rparams.prt2);
      _ncFile->add_att("radar_wavelength", rparams.wavelength);
      _ncFile->add_att("radar_xmitPeakPower", rparams.xmitPeakPower);
      _ncFile->add_att("radar_receiverMds", rparams.receiverMds);
      _ncFile->add_att("radar_receiverGain", rparams.receiverGain);
      _ncFile->add_att("radar_antennaGain", rparams.antennaGain);
      _ncFile->add_att("radar_systemGain", rparams.systemGain);
      _ncFile->add_att("radar_unambigVelocity", rparams.unambigVelocity);
      _ncFile->add_att("radar_unambigRange", rparams.unambigRange);
      _ncFile->add_att("radar_measXmitPowerDbmH", rparams.measXmitPowerDbmH);
      _ncFile->add_att("radar_measXmitPowerDbmV", rparams.measXmitPowerDbmV);
    }

    // elevations in PPI mode

    if (radar.radarElevAvail()) {
      const DsRadarElev &elev = radar.getRadarElev();
      if (elev.getNElev() > 0) {
        _ncFile->add_att("radar_numElevations", elev.getNElev());
        const fl32 *angles = elev.getElevArray();
        string angleStr;
        for (int ii = 0; ii < elev.getNElev(); ii++) {
          char text[80];
          sprintf(text, "%g", angles[ii]);
          angleStr += text;
          if (ii < elev.getNElev() - 1) {
            angleStr += ",";
          }
        }
        _ncFile->add_att("radar_elevations", angleStr.c_str());
      }
    }

    // azimuths in RHI mode

    if (radar.radarAzAvail()) {
      const DsRadarAz &az = radar.getRadarAz();
      if (az.getNAz() > 0) {
        _ncFile->add_att("radar_numAzimuths", az.getNAz());
        const fl32 *angles = az.getAzArray();
        string angleStr;
        for (int ii = 0; ii < az.getNAz(); ii++) {
          char text[80];
          sprintf(text, "%g", angles[ii]);
          angleStr += text;
          if (ii < az.getNAz() - 1) {
            angleStr += ",";
          }
        }
        _ncFile->add_att("radar_azimuths", angleStr.c_str());
      }
    }

  }

  return (iret? -1 : 0);

}

//////////////////////////////
// add radar global attributes

int Mdv2NcfTrans::_addRadarGlobalAttributes(const MdvxRadar &radar)

{

  if (_debug) {
    cerr << "Mdv2NcfTrans::_addMdvChunkVariables()" << endl;
    cerr << "  handling radar chunk" << endl;
    radar.print(cerr);
  }
  
  // radar params
  
  if (radar.radarParamsAvail()) {
    const DsRadarParams &rparams = radar.getRadarParams();
    _ncFile->add_att("radar_name", rparams.radarName.c_str());
    _ncFile->add_att("radar_id", rparams.radarId);
    _ncFile->add_att("radar_type",
                     DsRadarParams::radarType2Str(rparams.radarType).c_str());
    _ncFile->add_att("radar_numFields", rparams.numFields);
    _ncFile->add_att("radar_numGates", rparams.numGates);
    _ncFile->add_att("radar_samplesPerBeam", rparams.samplesPerBeam);
    _ncFile->add_att("radar_scanType", rparams.scanType);
    _ncFile->add_att("radar_scanTypeName", rparams.scanTypeName.c_str());
    _ncFile->add_att("radar_scanMode",
                     DsRadarParams::scanMode2Str(rparams.scanMode).c_str());
    _ncFile->add_att("radar_followMode",
                     DsRadarParams::followMode2Str(rparams.followMode).c_str());
    _ncFile->add_att("radar_polarization",
                     DsRadarParams::polType2Str(rparams.polarization).c_str());
    _ncFile->add_att("radar_prfMode",
                     DsRadarParams::prfMode2Str(rparams.prfMode).c_str());
    _ncFile->add_att("radar_constant", rparams.radarConstant);
    _ncFile->add_att("radar_altitude", rparams.altitude);
    _ncFile->add_att("radar_latitude", rparams.latitude);
    _ncFile->add_att("radar_longitude", rparams.longitude);
    _ncFile->add_att("radar_gateSpacing", rparams.gateSpacing);
    _ncFile->add_att("radar_startRange", rparams.startRange);
    _ncFile->add_att("radar_horizBeamWidth", rparams.horizBeamWidth);
    _ncFile->add_att("radar_vertBeamWidth", rparams.vertBeamWidth);
    _ncFile->add_att("radar_pulseWidth", rparams.pulseWidth);
    _ncFile->add_att("radar_pulseRepFreq", rparams.pulseRepFreq);
    _ncFile->add_att("radar_prt", rparams.prt);
    _ncFile->add_att("radar_prt2", rparams.prt2);
    _ncFile->add_att("radar_wavelength", rparams.wavelength);
    _ncFile->add_att("radar_xmitPeakPower", rparams.xmitPeakPower);
    _ncFile->add_att("radar_receiverMds", rparams.receiverMds);
    _ncFile->add_att("radar_receiverGain", rparams.receiverGain);
    _ncFile->add_att("radar_antennaGain", rparams.antennaGain);
    _ncFile->add_att("radar_systemGain", rparams.systemGain);
    _ncFile->add_att("radar_unambigVelocity", rparams.unambigVelocity);
    _ncFile->add_att("radar_unambigRange", rparams.unambigRange);
    _ncFile->add_att("radar_measXmitPowerDbmH", rparams.measXmitPowerDbmH);
    _ncFile->add_att("radar_measXmitPowerDbmV", rparams.measXmitPowerDbmV);
  }
  
  // elevations in PPI mode
  
  if (radar.radarElevAvail()) {
    const DsRadarElev &elev = radar.getRadarElev();
    _ncFile->add_att("radar_numElevations", elev.getNElev());
    const fl32 *angles = elev.getElevArray();
    string angleStr;
    for (int ii = 0; ii < elev.getNElev(); ii++) {
      char text[80];
      sprintf(text, "%g", angles[ii]);
      angleStr += text;
      if (ii < elev.getNElev() - 1) {
        angleStr += ",";
      }
    }
    _ncFile->add_att("radar_elevations", angleStr.c_str());
  }
  
  // azimuths in RHI mode
  
  if (radar.radarAzAvail()) {
    const DsRadarAz &az = radar.getRadarAz();
    _ncFile->add_att("radar_numAzimuths", az.getNAz());
    const fl32 *angles = az.getAzArray();
    string angleStr;
    for (int ii = 0; ii < az.getNAz(); ii++) {
      char text[80];
      sprintf(text, "%g", angles[ii]);
      angleStr += text;
      if (ii < az.getNAz() - 1) {
        angleStr += ",";
      }
    }
    _ncFile->add_att("radar_azimuths", angleStr.c_str());
  }

  return 0;

}

///////////////////////////////////////
// Write coordinate vars to the NcFile

int  Mdv2NcfTrans::_putCoordinateVariables()
{

  if (_debug) {
    cerr << "Mdv2NcfTrans::_putCoordinateVariables()" << endl;
  }

  // Put the coordinate variable data

  for (int i = 0; i < (int) _gridInfo.size(); i++) {

    if (_gridInfo[i]->writeCoordDataToFile(_ncFile, _errStr)) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::_putCoordinateVariables");
      return -1;
    }

  }

  // Put zlevel data

  for (int i = 0; i < (int) _vlevelInfo.size(); i++) {
    if (_vlevelInfo[i]->writeDataToFile(_ncFile, _errStr)) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::_putCoordinateVariables");
      return -1;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////
// Write the time variables data to the NcFile

int  Mdv2NcfTrans::_putTimeVariables()
{

  int iret = 0;

  if (_debug) {
    cerr << "Mdv2NcfTrans::_putTimeVariables()" << endl;
  }

  // Put the time data

  double timeCentroid = (double) _timeCentroid;

  iret |= ! _timeVar->put(&timeCentroid,1);

  if (_isForecast) {
    
    // forecast reference time
    
    double timeGen = (double) _timeGen;
    iret |= !_forecastRefTimeVar->put(&timeGen,1);
    
    // forecast time
    
    double leadTime = (double) _leadTime;
    iret |= !_forecastPeriodVar->put(&leadTime,1);

  }
  
  // start time, stop time and time_bounds

  if (_startTimeVar != NULL) {
    double startTime = (double) _timeBegin;
    iret |= !_startTimeVar->put(&startTime, 1);
  }
  
  if (_stopTimeVar != NULL) {
    double stopTime = (double) _timeEnd;
    iret |= !_stopTimeVar->put(&stopTime, 1);
  }
  
  if (_timeBoundsVar != NULL) {
    double timeBounds[2];
    timeBounds[0] = (double) _timeBegin;
    timeBounds[1] = (double) _timeEnd;
    iret |= !_timeBoundsVar->put(timeBounds, 1, 2);
  }
  
  return iret;

}


///////////////////////////////////////////////////////////
// Write the data to the NcFile

int  Mdv2NcfTrans::_putFieldDataVariables()
{

  int iret = 0;

  if (_debug) {
    cerr << "Mdv2NcfTrans::_putFieldDataVariables()" << endl;
  }

  for (int i = 0; i < (int) _fieldData.size(); i++) {

    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("Mdv2NcfTrans::_putFieldDataVariables");
    }

    if (_fieldData[i]->writeToFile(_ncFile, _errStr)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////
// put MDV chunk variables

int Mdv2NcfTrans::_putMdvChunkVariables()

{

  int iret = 0;

  if (_debug) {
    cerr << "Mdv2NcfTrans::putMdvChunkVariables()" << endl;
  }
  
  for (int i = 0; i < _mdv->getNChunks(); i++) {
    
    if (i >= (int) _chunkVars.size()) {
      break;
    }

    const MdvxChunk *chunk = _mdv->getChunkByNum(i);
    Nc3Var *chunkVar = _chunkVars[i];
    const ncbyte *cdata = (ncbyte*) chunk->getData();
    iret |= !chunkVar->put(cdata, 1, chunk->getSize());
    
  }
  
  return (iret? -1 : 0);

}

///////////////////////////////
// handle error string

void Mdv2NcfTrans::clearErrStr()
{
  _errStr = "";
  TaStr::AddStr(_errStr, "Time for following error: ", DateTime::str());
}

///////////////////////////////////////////////////
// Get a CF-compliant version of the field name
//
// check that the field name is CF-netCDF compliant - 
// i.e must start with a letter
//   if not, add "ncf_" to start of name
// and must only contain letters, digits and underscores
//
// Return a compliant name

string Mdv2NcfTrans::_getCfCompliantName(const string &requestedName)
{

  string compliantName;
  if (isalpha(requestedName[0])) {
    compliantName = requestedName;
  } else {
    compliantName = "ncf_";
    compliantName += requestedName;
  }
  for (int ii = 0; ii < (int) compliantName.size(); ii++) {
    if (!isalnum(compliantName[ii]) && compliantName[ii] != '_') {
      compliantName[ii] = '_';
    }
  }
  if (compliantName.compare(requestedName)) {
    cerr << "WARNING - Mdv2NcfTrans::_getCfCompliantName" << endl;
    cerr << "  Changing field name to make it CF compliant" << endl;
    cerr << "  Requested name: " << requestedName << endl;
    cerr << "  Compliant name: " << compliantName << endl;
  }

  return compliantName;

}
  
///////////////////////////////////////////////////
// Get a unique version of the field name
//
// If a previous field has this name, appen .1, .2 etc
// to the name to make it unique
//
// Return a unique name

string Mdv2NcfTrans::_getUniqueFieldName(const string &requestedName)
{

  if (_fieldNameSet.find(requestedName) == _fieldNameSet.end()) {
    // not yet added
    _fieldNameSet.insert(_fieldNameSet.begin(), requestedName);
    return requestedName;
  }

  // name is already in use, so create a modified version

  for (int ii = 2; ii < 10000; ii++) {

    char version[128];
    sprintf(version, "_%d", ii);
    string uniqueName = requestedName;
    uniqueName += version;

    if (_fieldNameSet.find(uniqueName) == _fieldNameSet.end()) {
      cerr << "WARNING - Mdv2NcfTrans::_getUniqueFieldName" << endl;
      cerr << "  Changing field name to make it unique" << endl;
      cerr << "  Check your data set for duplicate fields" << endl;
      cerr << "  Requested name: " << requestedName << endl;
      cerr << "  Unique name: " << uniqueName << endl;
      // not yet added
      _fieldNameSet.insert(_fieldNameSet.begin(), uniqueName);
      return uniqueName;
    }

  }

  // should not reach here
  // return a name based on the time

  DateTime now(time(NULL));
  string uniqueName = requestedName;
  uniqueName += DateTime::strm(now.utime());
  _fieldNameSet.insert(_fieldNameSet.begin(), uniqueName);
  umsleep(1000);

  cerr << "WARNING - Mdv2NcfTrans::_getUniqueFieldName" << endl;
  cerr << "  Changing field name to make it unique" << endl;
  cerr << "  Check your data set for duplicate fields" << endl;
  cerr << "  Requested name: " << requestedName << endl;
  cerr << "  Unique name: " << uniqueName << endl;

  return uniqueName;

}
  
///////////////////////////////////////////////////
// perform radial file translation to CfRadial,
// write file to dir
// returns 0 on success, -1 on failure
// Use getNcFilePath() to get path of file written

int Mdv2NcfTrans::translateToCfRadial(const DsMdvx &mdv, const string &dir)
{

  // convert to a RadxVol

  RadxVol vol;
  if (convertToRadxVol(mdv, vol)) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::translateToCfRadial");
    TaStr::AddStr(_errStr, "  Writing to dir: ", dir);
    return -1;
  }

  // check environment

  bool addYearSubdir = false;
  char *addYearSubdirStr = getenv("MDV_WRITE_ADD_YEAR_SUBDIR");
  if (addYearSubdirStr != NULL) {
    if (!strcasecmp(addYearSubdirStr, "TRUE")) {
      addYearSubdir = true;
    }
  }

  // write the file(s)

  if (_radialFileType == DsMdvx::RADIAL_TYPE_DORADE) {
    DoradeRadxFile file;
    if (file.writeToDir(vol, dir, true, addYearSubdir)) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::translateToCfRadial");
      TaStr::AddStr(_errStr, "  Writing Dorade File, dir: ", dir);
      TaStr::AddStr(_errStr, file.getErrStr());
      return -1;
    }
    _ncFilePath = file.getPathInUse();
  } else if (_radialFileType == DsMdvx::RADIAL_TYPE_UF) {
    UfRadxFile file;
    if (file.writeToDir(vol, dir, true, addYearSubdir)) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::translateToCfRadial");
      TaStr::AddStr(_errStr, "  Writing UF File, dir: ", dir);
      TaStr::AddStr(_errStr, file.getErrStr());
      return -1;
    }
    _ncFilePath = file.getPathInUse();
  } else {
    NcfRadxFile file;
    file.setNcFormat(_radxNcFormat);
    if (file.writeToDir(vol, dir, true, addYearSubdir)) {
      TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::translateToCfRadial");
      TaStr::AddStr(_errStr, "  Writing Nc File, dir: ", dir);
      TaStr::AddStr(_errStr, file.getErrStr());
      return -1;
    }
    _ncFilePath = file.getPathInUse();
  }

  // success

  return 0;

}

///////////////////////////////////////////////////
// Convert to Radx volume
// returns 0 on success, -1 on failure

int Mdv2NcfTrans::convertToRadxVol(const DsMdvx &mdv, RadxVol &vol)
{
  
  _mdv = &mdv;
  if (_mdv->getNFields() < 1) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::convertToRadxVol");
    TaStr::AddStr(_errStr, "  Path in use: ", mdv.getPathInUse());
    TaStr::AddStr(_errStr, "  No fields found");
    return -1;
  }

  // check for polar radar projection
  
  _isPolar = false;
  const MdvxField *field = _mdv->getField(0);
  const Mdvx::field_header_t &fhdr = field->getFieldHeader();
  if (fhdr.proj_type != Mdvx::PROJ_POLAR_RADAR &&
      fhdr.proj_type != Mdvx::PROJ_RHI_RADAR) {
    TaStr::AddStr(_errStr, "ERROR - Mdv2NcfTrans::convertToRadxVol");
    TaStr::AddStr(_errStr, "  Path in use: ", mdv.getPathInUse());
    TaStr::AddStr(_errStr, "  Incorrect projecttion, should be polar");
    TaStr::AddStr(_errStr, "  Found: ",
                  Mdvx::projType2Str(fhdr.proj_type));
    return -1;
  }

  _isPolar = true;

  // check for polar-RHI
  
  if (_mdv->getMasterHeader().vlevel_type == Mdvx::VERT_TYPE_AZ) {
    _isRhi = true;
  } else {
    _isRhi = false;
  }

  // set the translation parameters

  _setTransParams();

  // clear data from previous use

  clearData();

  // find the fields which have uniform geometry

  _findFieldsWithUniformGeom();

  // set volume params

  _cfRadialSetVolParams(vol);
  _cfRadialSetRadarParams(vol);

  // set calibration if available
  
  _cfRadialSetCalib(vol);

  // add the rays

  _cfRadialAddRays(vol);

  // add the sweeps

  _cfRadialAddSweeps(vol);

  // success

  return 0;

}

///////////////////////////////////////////////////
// Find fields with uniform geometry
//
// Assumes we have at least 1 field

void Mdv2NcfTrans::_findFieldsWithUniformGeom()
{

  int nFieldsMdv = _mdv->getMasterHeader().n_fields;
  assert(nFieldsMdv > 0);
  _uniformFieldIndexes.clear();

  // always have first field
  
  _uniformFieldIndexes.push_back(0);

  // check other fields against first field

  const Mdvx::field_header_t &fhdr0 = _mdv->getField(0)->getFieldHeader();
  for (int ii = 1; ii < nFieldsMdv; ii++) {
    const Mdvx::field_header_t &fhdr = _mdv->getField(ii)->getFieldHeader();
    // check for uniform geom
    if (fhdr0.nx != fhdr.nx ||
        fhdr0.ny != fhdr.ny ||
        fhdr0.nz != fhdr.nz) {
      continue;
    }
    if (fabs(fhdr0.grid_minx - fhdr.grid_minx) > 0.0001 ||
        fabs(fhdr0.grid_miny - fhdr.grid_miny) > 0.0001 ||
        fabs(fhdr0.grid_minz - fhdr.grid_minz) > 0.0001) {
      continue;
    }
    if (fabs(fhdr0.grid_dx - fhdr.grid_dx) > 0.0001 ||
        fabs(fhdr0.grid_dy - fhdr.grid_dy) > 0.0001 ||
        fabs(fhdr0.grid_dz - fhdr.grid_dz) > 0.0001) {
      continue;
    }
    // same geom as first field
    _uniformFieldIndexes.push_back(ii);
  }

}

///////////////////////////////////////////////////
// Set parameters on CfRadial volume

void Mdv2NcfTrans::_cfRadialSetVolParams(RadxVol &vol)
{

  // Get the master header

  const Mdvx::master_header_t &mhdr = _mdv->getMasterHeader();
  
  _timeBegin = mhdr.time_begin;
  _timeEnd = mhdr.time_end;
  _timeCentroid = mhdr.time_centroid;
  _timeExpire = mhdr.time_expire;
  _timeGen = mhdr.time_gen;
  _timeValid = mhdr.time_centroid;

  // set volume number

  vol.setVolumeNumber(mhdr.index_number);
  
  // set title etc

  vol.setTitle(mhdr.data_set_name);
  if (_mdv->_ncfInstitution.size() > 0) {
    vol.setInstitution(_mdv->_ncfInstitution);
  }
  if (_mdv->_ncfReferences.size() > 0) {
    vol.setReferences(_mdv->_ncfReferences);
  }
  vol.setSource(mhdr.data_set_source);
  vol.setHistory(_mdv->getDataSetInfo());
  if (_mdv->_ncfComment.size() > 0) {
    vol.setComment(_mdv->_ncfComment);
  }

  // times

  vol.setStartTime(mhdr.time_begin, 0);
  vol.setEndTime(mhdr.time_end, 0);

  // location

  vol.setLatitudeDeg(mhdr.sensor_lat);
  vol.setLongitudeDeg(mhdr.sensor_lon);
  vol.setAltitudeKm(mhdr.sensor_alt);

}

///////////////////////////////////////////////////
// Set radar parameters for CfRadial

void Mdv2NcfTrans::_cfRadialSetRadarParams(RadxVol &vol)
{

  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(*_mdv)) {
    // no radar info
    return;
  }

  if (!mdvxRadar.radarParamsAvail()) {
    // no radar params
    return;
  }

  DsRadarParams rparams = mdvxRadar.getRadarParams();

  vol.setLatitudeDeg(rparams.latitude);
  vol.setLongitudeDeg(rparams.longitude);
  vol.setAltitudeKm(rparams.altitude);
  
  vol.setInstrumentName(rparams.radarName);
  vol.setScanName(rparams.scanTypeName);

  vol.addWavelengthCm(rparams.wavelength);
  vol.setRadarBeamWidthDegH(rparams.horizBeamWidth);
  vol.setRadarBeamWidthDegV(rparams.vertBeamWidth);
  vol.setRadarAntennaGainDbH(rparams.antennaGain);
  vol.setRadarAntennaGainDbV(rparams.antennaGain);

  switch (rparams.radarType) {
    case DS_RADAR_AIRBORNE_FORE_TYPE: {
      vol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
      break;
    }
    case DS_RADAR_AIRBORNE_AFT_TYPE: {
      vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_FORE);
      break;
    }
    case DS_RADAR_AIRBORNE_TAIL_TYPE: {
      vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_TAIL);
      break;
    }
    case DS_RADAR_AIRBORNE_LOWER_TYPE: {
      vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_BELLY);
      break;
    }
    case DS_RADAR_AIRBORNE_UPPER_TYPE: {
      vol.setPlatformType(Radx::PLATFORM_TYPE_AIRCRAFT_ROOF);
      break;
    }
    case DS_RADAR_SHIPBORNE_TYPE: {
      vol.setPlatformType(Radx::PLATFORM_TYPE_SHIP);
      break;
    }
    case DS_RADAR_VEHICLE_TYPE: {
      vol.setPlatformType(Radx::PLATFORM_TYPE_VEHICLE);
      break;
    }
    case DS_RADAR_GROUND_TYPE:
    default:{
      vol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
    }
  }

}

///////////////////////////////////////////////////
// Set calibration for CfRadial

void Mdv2NcfTrans::_cfRadialSetCalib(RadxVol &vol)
{

  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(*_mdv)) {
    // no radar info
    return;
  }

  if (!mdvxRadar.radarCalibAvail()) {
    // no radar calib
    return;
  }

  const DsRadarCalib &dsCal = mdvxRadar.getRadarCalib();
  RadxRcalib *radxCal = new RadxRcalib();

  DateTime ctime(dsCal.getCalibTime());
  
  radxCal->setCalibTime(ctime.getYear(),
                        ctime.getMonth(),
                        ctime.getDay(),
                        ctime.getHour(),
                        ctime.getMin(),
                        ctime.getSec());
  
  radxCal->setPulseWidthUsec(dsCal.getPulseWidthUs());
  radxCal->setXmitPowerDbmH(dsCal.getXmitPowerDbmH());
  radxCal->setXmitPowerDbmV(dsCal.getXmitPowerDbmV());
  
  radxCal->setTwoWayWaveguideLossDbH(dsCal.getTwoWayWaveguideLossDbH());
  radxCal->setTwoWayWaveguideLossDbV(dsCal.getTwoWayWaveguideLossDbV());
  radxCal->setTwoWayRadomeLossDbH(dsCal.getTwoWayRadomeLossDbH());
  radxCal->setTwoWayRadomeLossDbV(dsCal.getTwoWayRadomeLossDbV());
  radxCal->setReceiverMismatchLossDb(dsCal.getReceiverMismatchLossDb());
  
  radxCal->setRadarConstantH(dsCal.getRadarConstH());
  radxCal->setRadarConstantV(dsCal.getRadarConstV());
  
  radxCal->setNoiseDbmHc(dsCal.getNoiseDbmHc());
  radxCal->setNoiseDbmHx(dsCal.getNoiseDbmHx());
  radxCal->setNoiseDbmVc(dsCal.getNoiseDbmVc());
  radxCal->setNoiseDbmVx(dsCal.getNoiseDbmVx());
  
  radxCal->setReceiverGainDbHc(dsCal.getReceiverGainDbHc());
  radxCal->setReceiverGainDbHx(dsCal.getReceiverGainDbHx());
  radxCal->setReceiverGainDbVc(dsCal.getReceiverGainDbVc());
  radxCal->setReceiverGainDbVx(dsCal.getReceiverGainDbVx());
  
  radxCal->setReceiverSlopeDbHc(dsCal.getReceiverSlopeDbHc());
  radxCal->setReceiverSlopeDbHx(dsCal.getReceiverSlopeDbHx());
  radxCal->setReceiverSlopeDbVc(dsCal.getReceiverSlopeDbVc());
  radxCal->setReceiverSlopeDbVx(dsCal.getReceiverSlopeDbVx());
  
  radxCal->setBaseDbz1kmHc(dsCal.getBaseDbz1kmHc());
  radxCal->setBaseDbz1kmHx(dsCal.getBaseDbz1kmHx());
  radxCal->setBaseDbz1kmVc(dsCal.getBaseDbz1kmVc());
  radxCal->setBaseDbz1kmVx(dsCal.getBaseDbz1kmVx());
  
  radxCal->setSunPowerDbmHc(dsCal.getSunPowerDbmHc());
  radxCal->setSunPowerDbmHx(dsCal.getSunPowerDbmHx());
  radxCal->setSunPowerDbmVc(dsCal.getSunPowerDbmVc());
  radxCal->setSunPowerDbmVx(dsCal.getSunPowerDbmVx());
  
  radxCal->setNoiseSourcePowerDbmH(dsCal.getNoiseSourcePowerDbmH());
  radxCal->setNoiseSourcePowerDbmV(dsCal.getNoiseSourcePowerDbmV());
  
  radxCal->setPowerMeasLossDbH(dsCal.getPowerMeasLossDbH());
  radxCal->setPowerMeasLossDbV(dsCal.getPowerMeasLossDbV());
  
  radxCal->setCouplerForwardLossDbH(dsCal.getCouplerForwardLossDbH());
  radxCal->setCouplerForwardLossDbV(dsCal.getCouplerForwardLossDbV());
  
  radxCal->setZdrCorrectionDb(dsCal.getZdrCorrectionDb());
  radxCal->setLdrCorrectionDbH(dsCal.getLdrCorrectionDbH());
  radxCal->setLdrCorrectionDbV(dsCal.getLdrCorrectionDbV());
  radxCal->setSystemPhidpDeg(dsCal.getSystemPhidpDeg());
  
  radxCal->setTestPowerDbmH(dsCal.getTestPowerDbmH());
  radxCal->setTestPowerDbmV(dsCal.getTestPowerDbmV());
  
  vol.addCalib(radxCal);

}
  
///////////////////////////////////////////////////
// Add the rays for CfRadial

void Mdv2NcfTrans::_cfRadialAddRays(RadxVol &vol)
{

  int nFieldsMdv = _mdv->getMasterHeader().n_fields;
  assert(nFieldsMdv > 0);
  const MdvxField &field = *_mdv->getField(0);
  const Mdvx::field_header_t &fhdr = field.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = field.getVlevelHeader();

  MdvxRadar mdvxRadar;
  DsRadarParams rparams;
  bool rparamsAvail = false;
  if (mdvxRadar.loadFromMdvx(*_mdv) && mdvxRadar.radarParamsAvail()) {
    rparams = mdvxRadar.getRadarParams();
    rparamsAvail = true;
  }

  int nSweeps = fhdr.nz;
  int nAngles = fhdr.ny;
  int nRays = nSweeps * nAngles;

  vol.setRangeGeom(fhdr.grid_minx, fhdr.grid_dx);

  double startSecs = vol.getStartTimeSecs();
  double endSecs = vol.getEndTimeSecs();
  double volPeriod = endSecs - startSecs;
  double rayPeriod = volPeriod / (nRays - 1.0);

  double startAngle = fhdr.grid_miny;
  double deltaAngle = fhdr.grid_dy;

  int rayCount = 0;

  // loop through sweeps, and angles in each sweep

  vector<RadxRay *> rays;

  for (int isweep = 0; isweep < nSweeps; isweep++) {

    double fixedAngle = vhdr.level[isweep];
    
    for (int iangle = 0; iangle < nAngles; iangle++, rayCount++) {

      // create ray, copying geometry from volume

      RadxRay *ray = new RadxRay;
      ray->copyRangeGeom(vol);
      
      // set time
      
      double timeDiff = rayCount * rayPeriod;
      double rayTime = startSecs + timeDiff;
      time_t rayTimeSecs = (time_t) rayTime;
      int rayNanoSecs = (int) ((rayTime - rayTimeSecs) * 1.0e9 + 0.5);

      ray->setTime(rayTimeSecs, rayNanoSecs);

      // set angles

      double angle = startAngle + iangle * deltaAngle;
      if (!_isRhi) {
        while (angle < 0.0) {
          angle += 360.0;
        }
        while (angle >= 360.0) {
          angle -= 360.0;
        }
      }
      ray->setVolumeNumber(vol.getVolumeNumber());
      ray->setSweepNumber(isweep);
      ray->setFixedAngleDeg(fixedAngle);

      if (_isRhi) {
        ray->setAzimuthDeg(fixedAngle);
        ray->setElevationDeg(angle);
        ray->setSweepMode(Radx::SWEEP_MODE_RHI);
      } else {
        ray->setAzimuthDeg(angle);
        ray->setElevationDeg(fixedAngle);
        ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      }

      // set radar paramters if available

      if (rparamsAvail) {
        ray->setSweepMode(_getRadxSweepMode(rparams.scanMode));
        ray->setPolarizationMode(_getRadxPolarizationMode(rparams.polarization));
        ray->setPrtMode(_getRadxPrtMode(rparams.prfMode));
        ray->setFollowMode(_getRadxFollowMode(rparams.followMode));
        ray->setNSamples(rparams.samplesPerBeam);
        ray->setPulseWidthUsec(rparams.pulseWidth);
        ray->setPrtSec(rparams.prt);
        double prtRatio = rparams.prt / rparams.prt2;
        if (prtRatio > 1.0) prtRatio = (1.0 / prtRatio);
        ray->setPrtRatio(prtRatio);
        ray->setNyquistMps(rparams.unambigVelocity);
        ray->setUnambigRangeKm(rparams.unambigRange);
        ray->setMeasXmitPowerDbmH(rparams.measXmitPowerDbmH);
        ray->setMeasXmitPowerDbmV(rparams.measXmitPowerDbmV);
      }
      
      // set beam indexing flag etc

      ray->setIsIndexed(true);
      ray->setAngleResDeg(deltaAngle);
      ray->setAntennaTransition(false);

      // calibration

      if (vol.getRcalibs().size() > 0) {
        ray->setCalibIndex(0);
      } else {
        ray->setCalibIndex(-1);
      }

      // add to volume, which will take care of freeing memory
      // associated with the ray

      rays.push_back(ray);

    } // iangle

  } // isweep
  
  // add fields to rays
  
  _addFieldsToRays(vol, rays);
  
  // add rays to vol, which takes over memory management
  
  for (size_t ii = 0; ii < rays.size(); ii++) {
    vol.addRay(rays[ii]);
  }
  rays.clear();

}

////////////////////////////////////////////////////////////////////////////
// Add fields to the ray

void Mdv2NcfTrans::_addFieldsToRays(RadxVol &vol, vector<RadxRay *> rays)
{

  
  for (int ii = 0; ii < (int) _uniformFieldIndexes.size(); ii++) {
    
    // use field numbers from uniform geometry list
    
    int ifield = _uniformFieldIndexes[ii];
    
    // copy Mdvx field
    
    MdvxField mdvField(*_mdv->getField(ifield));
    const Mdvx::field_header_t &fhdr = mdvField.getFieldHeader();
    if (_debug) {
      cerr << "Mdv2NcfTrans adding field: " << fhdr.field_name << endl;
    }
    int nSweeps = fhdr.nz;
    int nAngles = fhdr.ny;
    int nGates = fhdr.nx;
    
    string mdvFieldName = fhdr.field_name;
    string mdvLongFieldName = fhdr.field_name_long;
    
    // set up field names etc

    string ncfFieldName = mdvFieldName;
    string ncfStandardName = mdvFieldName;
    string ncfLongName = fhdr.field_name_long;
    string ncfUnits = fhdr.units;
    // bool doLinearTransform = false;
    // double linearMult = 1.0;
    // double linearOffset = 0.0;
    DsMdvx::ncf_pack_t packing = DsMdvx::NCF_PACK_ASIS;

    // check if this field has been specified for translation
    
    for (int jj = 0; jj < (int) _mdv->_mdv2NcfTransArray.size(); jj++) {
      // check for match on MDV field name
      const DsMdvx::Mdv2NcfFieldTrans &trans = _mdv->_mdv2NcfTransArray[jj];
      if (mdvFieldName.compare(trans.mdvFieldName) == 0 ||
	  mdvLongFieldName.compare(trans.mdvFieldName) == 0) {
        // match - so set accordingly
        ncfFieldName = trans.ncfFieldName;
        ncfStandardName = trans.ncfStandardName;
        ncfLongName = trans.ncfLongName;
        ncfUnits = trans.ncfUnits;
        // doLinearTransform = trans.doLinearTransform;
        // linearMult = trans.linearMult;
        // linearOffset = trans.linearOffset;
        // packing = trans.packing;
        break;
      }
    } // jj

    // get output data encoding type

    Radx::DataType_t outputType = Radx::FL32;
    if (packing != DsMdvx::NCF_PACK_ASIS) {
      switch (packing) {
        case DsMdvx::NCF_PACK_SHORT: {
          outputType = Radx::SI16;
          break;
        }
        case DsMdvx::NCF_PACK_BYTE: {
          outputType = Radx::SI08;
          break;
        }
        default: {
          outputType = Radx::FL32;
        }
      }
    } else {
      switch (fhdr.encoding_type) {
        case Mdvx::ENCODING_INT8: {
          outputType = Radx::SI08;
          break;
        }
        case Mdvx::ENCODING_INT16: {
          outputType = Radx::SI16;
          break;
        }
        default: {
          outputType = Radx::FL32;
          break;
        }
      }
    }

    // ensure data is not compressed in MDV, and is in float 32
    
    mdvField.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    
    // loop through sweeps and angles, adding field to rays

    const fl32 *fdata = (const fl32 *) mdvField.getVol();
    int rayNum = 0;
    for (int isweep = 0; isweep < nSweeps; isweep++) {
      for (int iangle = 0; iangle < nAngles;
           iangle++, fdata += nGates, rayNum++) {
        RadxField *rfld = new RadxField(ncfFieldName, ncfUnits);
        rfld->setLongName(ncfLongName);
        rfld->setStandardName(ncfStandardName);
        rfld->setRangeGeom(fhdr.grid_minx, fhdr.grid_dx);
        rfld->setTypeFl32(fhdr.missing_data_value);
        rfld->addDataFl32(nGates, fdata);
        rfld->convertToType(outputType);
        RadxRay *ray = rays[rayNum];
        ray->addField(rfld);
      } // iangle
    } // isweep
    
  } // ifield

}

///////////////////////////////////////////////////
// Add the sweeps for CfRadial

void Mdv2NcfTrans::_cfRadialAddSweeps(RadxVol &vol)
{

  // get info from field headers

  int nFieldsMdv = _mdv->getMasterHeader().n_fields;
  assert(nFieldsMdv > 0);
  const MdvxField &field = *_mdv->getField(0);
  const Mdvx::field_header_t &fhdr = field.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = field.getVlevelHeader();

  // get radar params if avail

  MdvxRadar mdvxRadar;
  DsRadarParams rparams;
  bool rparamsAvail = false;
  if (mdvxRadar.loadFromMdvx(*_mdv) && mdvxRadar.radarParamsAvail()) {
    rparams = mdvxRadar.getRadarParams();
    rparamsAvail = true;
  }

  int nSweeps = fhdr.nz;
  int nAngles = fhdr.ny;
  double deltaAngle = fhdr.grid_dy;

  // loop through sweeps, and angles in each sweep
  
  for (int isweep = 0; isweep < nSweeps; isweep++) {
    
    // create sweep
    
    RadxSweep *sweep = new RadxSweep;

    // set values on sweep

    double fixedAngle = vhdr.level[isweep];

    sweep->setVolumeNumber(vol.getVolumeNumber());
    sweep->setSweepNumber(isweep);
    sweep->setFixedAngleDeg(fixedAngle);

    int startIndex = isweep * nAngles;
    sweep->setStartRayIndex(startIndex);
    sweep->setEndRayIndex(startIndex + nAngles - 1);
    
    sweep->setRaysAreIndexed(true);
    sweep->setAngleResDeg(deltaAngle);

    if (rparamsAvail) {
      sweep->setSweepMode(_getRadxSweepMode(rparams.scanMode));
      sweep->setPolarizationMode(_getRadxPolarizationMode(rparams.polarization));
      sweep->setPrtMode(_getRadxPrtMode(rparams.prfMode));
      sweep->setFollowMode(_getRadxFollowMode(rparams.followMode));
    }

    // add to volume, which will take over memory associated
    // with sweep object

    vol.addSweep(sweep);

  } // isweep

}

/////////////////////////////////////////////
// convert from DsrRadar enums to Radx enums

Radx::SweepMode_t Mdv2NcfTrans::_getRadxSweepMode(int dsrScanMode)

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
    case DS_RADAR_POINTING_MODE:
      return Radx::SWEEP_MODE_POINTING;
    default:
      return Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  }
}

Radx::PolarizationMode_t Mdv2NcfTrans::_getRadxPolarizationMode(int dsrPolMode)

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
    case DS_POLARIZATION_RIGHT_CIRC_TYPE:
    case DS_POLARIZATION_LEFT_CIRC_TYPE:
      return Radx::POL_MODE_CIRCULAR;
    case DS_POLARIZATION_ELLIPTICAL_TYPE:
    default:
      return Radx::POL_MODE_HORIZONTAL;
  }
}

Radx::FollowMode_t Mdv2NcfTrans::_getRadxFollowMode(int dsrMode)

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

Radx::PrtMode_t Mdv2NcfTrans::_getRadxPrtMode(int dsrMode)
  
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
