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
// FieldData.cc
// 
// Sue Dettling, RAP, NCAR
// PO Box 3000, Boulder, CO, USA
// April 2008
//
/////////////////////////////////////////////////////////////////////
//
// NcfFieldData class holds Mdv field data as well as ptrs to a
// NcfGridInfo object( access to 2D grid info and projection) ,
// NcfVlevelInfo object (access to vertical level information)
//
//////////////////////////////////////////////////////////////

#include <toolsa/TaStr.hh>
#include <Mdv/NcfMdv.hh>
#include <Mdv/NcfFieldData.hh>

NcfFieldData::NcfFieldData(bool debug,
                     const MdvxField *mdvField,
                     const NcfGridInfo *gridInfo,
                     const NcfVlevelInfo *vlevelInfo,
                     const string &mdvFieldName,
                     const string &ncfFieldName,
                     const string &ncfStandardName,
                     const string &ncfLongName,
                     const string &ncfUnits,
                     bool doLinearTransform,
                     double linearMult,
                     double linearOffset,
                     DsMdvx::ncf_pack_t packing,
                     bool output_latlon_arrays,
                     bool compress,
                     int compression_level,
                     Nc3File::FileFormat format) :
        _debug(debug),
        _mdvField(*mdvField),
        _gridInfo(gridInfo),
        _vlevelInfo(vlevelInfo),
        _mdvName(mdvFieldName),
        _ncfName(ncfFieldName),
        _ncfStandardName(ncfStandardName),
        _ncfLongName(ncfLongName),
        _ncfUnits(ncfUnits),
        _doLinearTransform(doLinearTransform),
        _linearMult(linearMult),
        _linearOffset(linearOffset),
        _packingRequested(packing),
        _outputLatlonArrays(output_latlon_arrays),
        _compress(compress),
        _compressionLevel(compression_level),
        _ncFormat(format),
        _ncVar(NULL)

{
  
  // save the incoming field header
  
  _fhdrIn = _mdvField.getFieldHeader();

  // make sure we have float 32 and uncompressed

  _mdvField.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  _mdvField.computeMinAndMax();

  _fhdrFl32 = _mdvField.getFieldHeader();
  _missing = _fhdrFl32.missing_data_value;
  
  // get field information

  _name = _fhdrIn.field_name;
  _nameLong = _fhdrIn.field_name_long;
  _units = _fhdrIn.units;

  _forecastTime = _fhdrIn.forecast_time;
  _forecastDelta = _fhdrIn.forecast_delta;
      
  // determine packing

  _ncType = nc3Float;
  if (_packingRequested == DsMdvx::NCF_PACK_ASIS) {
    // ASIS packing
    // inspect the original header to determine packing
    if (_fhdrIn.encoding_type == Mdvx::ENCODING_INT8) {
      _packingUsed = DsMdvx::NCF_PACK_BYTE;
      _ncType = nc3Byte;
    } else if (_fhdrIn.encoding_type == Mdvx::ENCODING_INT16) {
      _packingUsed = DsMdvx::NCF_PACK_SHORT;
      _ncType = nc3Short;
    } else {
      _packingUsed = DsMdvx::NCF_PACK_FLOAT;
      _ncType = nc3Float;
    }
  } else {
    // Specified packing
    _packingUsed = _packingRequested;
    if (_packingRequested == DsMdvx::NCF_PACK_BYTE) {
      _ncType = nc3Byte;
    } else if (_packingRequested == DsMdvx::NCF_PACK_SHORT) {
      _ncType = nc3Short;
    }
  }

}

NcfFieldData::~NcfFieldData()
{

}


//////////////////////////////////////
// add field variable to Nc File object
// Returns 0 on success, -1 on failure

int NcfFieldData::addToNc(Nc3File *ncFile, Nc3Dim *timeDim,
                          bool outputMdvAttr, string &errStr)

{

  int iret = 0;

  // ensure min and max are set appropriately

  _mdvField.computeMinAndMax();
  _minOut = _fhdrIn.min_value;
  _maxOut = _fhdrIn.max_value;
  
  // Apply linear transform to the data (to change units for example)
  // if required to non missing data values.

  if (_doLinearTransform) {
    if (_linearMult > 0) {
      _minOut = _linearMult * _minOut + _linearOffset;
      _maxOut = _linearMult * _maxOut + _linearOffset;
    } else {
      // If multiplier is negative, the min becomes the max and the max 
      // becomes the min
      float _minOutSavedVal = _minOut;
      _minOut = _linearMult * _maxOut + _linearOffset;
      _maxOut = _linearMult * _minOutSavedVal + _linearOffset;
    }
  }

  if (_ncfName.size() < 1) {
    errStr += "ERROR - NcfFieldData::addToNc\n";
    errStr += "  Cannot add variable to Nc file object\n";
    errStr += "  Field name is zero-length\n";
    return -1;
  }

  // check that the field name is CF-netCDF compliant - 
  // i.e must start with a letter
  //   if not, add "nc_" to start of name
  // and must only contain letters, digits and underscores

  string fieldName;
  if (isalpha(_ncfName[0])) {
    fieldName = _ncfName;
  } else {
    fieldName = "nc_";
    fieldName += _ncfName;
  }
  for (int ii = 0; ii < (int) fieldName.size(); ii++) {
    if (!isalnum(fieldName[ii]) && fieldName[ii] != '_') {
      fieldName[ii] = '_';
    }
  }
  
  // Add Nc3Var to NcFile object and the attributes relevant to no data packing

  if (_debug) {
    cerr << "adding field: " << fieldName << endl;
  }

  _ncVar =
    ncFile->add_var(fieldName.c_str(), _ncType, timeDim,
                    _vlevelInfo->getNcZdim(), _gridInfo->getNcYdim(),
                    _gridInfo->getNcXdim());

  if (_ncVar == NULL) {
    errStr += "WARNING - NcfFieldData::addToNc\n";
    errStr += "  Cannot add variable to Nc file object\n";
    TaStr::AddStr(errStr, "  Input field name: ", _ncfName);
    TaStr::AddStr(errStr, "  Output field name: ", fieldName);
    TaStr::AddInt(errStr, "  Nc3Type: ", _ncType);
    TaStr::AddStr(errStr, "  Time dim name: ", timeDim->name());
    TaStr::AddInt(errStr, "  Time dim size: ", timeDim->size());
    TaStr::AddStr(errStr, "  Z dim name: ", _vlevelInfo->getNcZdim()->name());
    TaStr::AddInt(errStr, "  Z dim size: ", _vlevelInfo->getNcZdim()->size());
    TaStr::AddStr(errStr, "  Y dim name: ", _gridInfo->getNcYdim()->name());
    TaStr::AddInt(errStr, "  Y dim size: ", _gridInfo->getNcYdim()->size());
    TaStr::AddStr(errStr, "  X dim name: ", _gridInfo->getNcXdim()->name());
    TaStr::AddInt(errStr, "  X dim size: ", _gridInfo->getNcXdim()->size());
    return -1;
  }
  
  // data packing scheme for non-floats
  // We map the valid range (_minOut, _maxOut) to ( -2^(n-1)+ 1, 2^(n-1) -1)
  // and leave -2^(n-1) for the fill value.
  //
  // add_offset = (_maxOut + _minOut)/2
  // scale_factor = (_maxOut - _minOut)/(2^n - 2)
  // packedVal = (unpacked - offset)/scaleFactor
  // where n is the number of bits of the packed (integer) data type
  
  if (_packingUsed == DsMdvx::NCF_PACK_SHORT) {
    
    si16 shortMax = 32767;
    si16 shortMin = -32767;
    si16 fillValue = -32768;

    if (_packingRequested == DsMdvx::NCF_PACK_ASIS) {

      _scaleFactor = _fhdrIn.scale;
      _addOffset = _fhdrIn.bias - shortMin * _scaleFactor;

    } else {
      
      _addOffset = (_maxOut + _minOut) / 2.0;
      _scaleFactor = (_maxOut - _minOut) / (pow(2.0, 16.0) - 2);

    }
    
    iret |= !_ncVar->add_att(NcfMdv::scale_factor, _scaleFactor);
    iret |= !_ncVar->add_att(NcfMdv::add_offset, _addOffset);
    iret |= !_ncVar->add_att(NcfMdv::valid_min, shortMin);
    iret |= !_ncVar->add_att(NcfMdv::valid_max, shortMax);
    iret |= !_ncVar->add_att(NcfMdv::FillValue, fillValue);

  } else if (_packingUsed == DsMdvx::NCF_PACK_BYTE) {

    si08 byteMax = 127;
    si08 byteMin = -127;
    si08 fillValue = -128;

    if (_packingRequested == DsMdvx::NCF_PACK_ASIS) {

      _scaleFactor = _fhdrIn.scale;
      _addOffset = _fhdrIn.bias - byteMin * _scaleFactor;

    } else {
      
      _addOffset = (_maxOut + _minOut) / 2.0;
      _scaleFactor = (_maxOut - _minOut) / (pow(2.0, 8.0) - 2);

    }
    
    iret |= !_ncVar->add_att(NcfMdv::scale_factor, _scaleFactor);
    iret |= !_ncVar->add_att(NcfMdv::add_offset, _addOffset);
    iret |= !_ncVar->add_att(NcfMdv::valid_min, (ncbyte) byteMin);
    iret |= !_ncVar->add_att(NcfMdv::valid_max, (ncbyte) byteMax);
    iret |= !_ncVar->add_att(NcfMdv::FillValue, (ncbyte) fillValue);

  } else {

    // float
    
    _addOffset = 0.0;
    _scaleFactor = 1.0;

    iret |= !_ncVar->add_att(NcfMdv::valid_min, _minOut);
    iret |= !_ncVar->add_att(NcfMdv::valid_max, _maxOut);
    iret |= !_ncVar->add_att(NcfMdv::FillValue, _missing);

  }

  if ( _ncfStandardName.size() > 0) {
    iret |= !_ncVar->add_att(NcfMdv::standard_name,
                                _ncfStandardName.c_str());
  }
  
  iret |= !_ncVar->add_att(NcfMdv::long_name, _ncfLongName.c_str());
  iret |= !_ncVar->add_att(NcfMdv::units, _ncfUnits.c_str());

  // Add auxiliary variables if necessay

  if (_gridInfo->getProjType() != Mdvx::PROJ_LATLON ) {

    char auxVarNames[1024];
    
    if (_outputLatlonArrays) {
      sprintf(auxVarNames, "%s %s",
              _gridInfo->getNcLonVar()->name(),
              _gridInfo->getNcLatVar()->name());
      iret |= !_ncVar->add_att(NcfMdv::coordinates, auxVarNames);
    }

    if (_gridInfo->getNcProjVar() != NULL) {
      sprintf(auxVarNames, "%s", _gridInfo->getNcProjVar()->name());
      iret |= !_ncVar->add_att(NcfMdv::grid_mapping, auxVarNames);
    }

  }

  if (outputMdvAttr) {
    iret |= !_ncVar->add_att(NcfMdv::mdv_field_code, _fhdrIn.field_code);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_time_1, _fhdrIn.user_time1);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_time_2, _fhdrIn.user_time2);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_time_3, _fhdrIn.user_time3);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_time_4, _fhdrIn.user_time4);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_0, _fhdrIn.user_data_si32[0]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_1, _fhdrIn.user_data_si32[1]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_2, _fhdrIn.user_data_si32[2]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_3, _fhdrIn.user_data_si32[3]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_4, _fhdrIn.user_data_si32[4]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_5, _fhdrIn.user_data_si32[5]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_6, _fhdrIn.user_data_si32[6]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_7, _fhdrIn.user_data_si32[7]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_8, _fhdrIn.user_data_si32[8]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_si32_9, _fhdrIn.user_data_si32[9]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_fl32_0, _fhdrIn.user_data_fl32[0]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_fl32_1, _fhdrIn.user_data_fl32[1]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_fl32_2, _fhdrIn.user_data_fl32[2]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_user_data_fl32_3, _fhdrIn.user_data_fl32[3]);
    iret |= !_ncVar->add_att(NcfMdv::mdv_proj_type, _fhdrIn.proj_type);
    iret |= !_ncVar->add_att(NcfMdv::mdv_proj_origin_lat, _fhdrIn.proj_origin_lat);
    iret |= !_ncVar->add_att(NcfMdv::mdv_proj_origin_lon, _fhdrIn.proj_origin_lon);
    iret |= !_ncVar->add_att(NcfMdv::mdv_transform_type, _fhdrIn.transform_type);
    iret |= !_ncVar->add_att(NcfMdv::mdv_vlevel_type, _fhdrIn.vlevel_type);
    iret |= !_ncVar->add_att(NcfMdv::mdv_native_vlevel_type, _fhdrIn.native_vlevel_type);
    iret |= !_ncVar->add_att(NcfMdv::mdv_transform, _fhdrIn.transform);
  }

  // Set compression if we are working with netCDF4 hdf files

  if (_compress &&
      (_ncFormat == Nc3File::Netcdf4 || _ncFormat == Nc3File::Netcdf4Classic)) {
    if (_setCompression(ncFile, errStr)) {
      cerr << "WARNING: NcfFieldData::addToNcf" << endl;
      cerr << "  Compression will not be used" << endl;
    }
  }

  if (iret) {
    return -1;
  } else {
    return 0;
  }

}

///////////////////////////////////////////////////////////
// Write the data to the Nc File
//
// Returns 0 on success, -1 on error

int NcfFieldData::writeToFile(Nc3File *ncFile, string &errStr)

{

  if (_ncVar == NULL) {
    errStr += "ERROR - NcfFieldData::writeToFile\n";
    TaStr::AddStr(errStr, "  Cannot write field to nc file: ", _ncfName);
    errStr += "  _ncVar is NULL\n";
    return -1;
  }

  int iret = 0;
  int nx = _fhdrFl32.nx;
  int ny = _fhdrFl32.ny;
  int nz = _fhdrFl32.nz;
  int arraySize = nx * ny * nz;
      
  // set the data pointer to the fl32 data
  
  fl32 *fl32Data = (fl32 *) _mdvField.getVol();

  // Replace bad data with missing, which is the netCDF _FillValue

  if (_fhdrFl32.bad_data_value != _fhdrFl32.missing_data_value) {
    for (int j = 0; j < arraySize; j++) {
      if (fl32Data[j] == _fhdrFl32.bad_data_value) {
        fl32Data[j] = _missing;
      }
    }
  }

  // Apply transform to non missing data

  if (_doLinearTransform) {
    for (int j = 0; j < arraySize; j++) {
      if (fl32Data[j] != _missing) {
        fl32Data[j] = _linearMult * fl32Data[j] + _linearOffset;
      }
    }
  }

  if(_packingUsed == DsMdvx::NCF_PACK_FLOAT) {
    
    // Put unpacked data. 
    
    iret |= !_ncVar->put(fl32Data, 1, nz, ny, nx);
    
  } else if(_packingUsed == DsMdvx::NCF_PACK_BYTE) {
    
    si08 *byteData = new si08[arraySize];
    si08 miss = -128;

    for (int j = 0; j < arraySize; j++) {

      // pack non missing data, assign fill val to missing data

      if (fl32Data[j] != _missing) {
        byteData[j] = (si08) (round((fl32Data[j] - _addOffset) / _scaleFactor));
      } else {
        byteData[j] = miss;
      }

    }

    iret |= !_ncVar->put((ncbyte*) byteData, 1, nz, ny, nx);

    delete[] byteData;
    
  } else if(_packingUsed == DsMdvx::NCF_PACK_SHORT) {

    // Pack short data. See method comment for packing algorithm

    si16* shortData = new si16[arraySize];
    si16 miss = -32768;

    for (int j = 0; j < arraySize; j++) {

      // pack non missing data, assign fill val to missing data
      
      if (fl32Data[j] != _missing) {
        shortData[j] = (si16) round((fl32Data[j] - _addOffset) / _scaleFactor);
      } else {
        shortData[j] = miss;
      }

    }

    iret |= !_ncVar->put(shortData, 1, nz, ny, nx);
    
    delete[] shortData;

  }

  return iret;
}

///////////////////////////////////////////////////////////////////////////
// Set compression for data.

int NcfFieldData::_setCompression(Nc3File *ncFile,
                                  string &errStr)
  
{
  
  if (_ncVar == NULL) {
    return -1;
  }

  if (_debug) {
    cerr << "NcfFieldData::_setCompression()" << endl;
    cerr << "  Field, compression_level: " << _ncVar->name()
         <<  ", " << _compressionLevel << endl;
  }
  
  // deflateLevel is the desire level of compression, an integer from 1 to 9.

  int varId, fileId, shuffle, deflateControl, deflateLevel, newLevel;
  
  fileId = ncFile->id();
  varId = _ncVar->id();
  deflateLevel = _compressionLevel;

  nc_inq_var_deflate(fileId, varId, &shuffle, &deflateControl, &newLevel);
 
  deflateControl = 1;
  newLevel = deflateLevel; 

  int iret = nc_def_var_deflate(fileId, varId, shuffle,
                                deflateControl, newLevel);
  if (iret != NC_NOERR) {
    cerr << "WARNING: NcfFieldData::_setCompression" << endl;
    cerr << "  Problem setting compression for var: " << _ncVar->name() << endl;
    return -1;
  }

  return 0;

}

