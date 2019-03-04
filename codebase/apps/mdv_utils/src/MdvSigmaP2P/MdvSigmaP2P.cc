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
// $Id: MdvSigmaP2P.cc,v 1.6 2019/03/04 00:22:24 dixon Exp $
//
// MdvSigmaP2P object
//
// Yan Chen, RAL, NCAR
//
// Sept. 2008
//
///////////////////////////////////////////////////////////////
//
// MdvSigmaP2P converts Mdv files in Sigma P levels to pressure levels.
//
///////////////////////////////////////////////////////////////

#include <vector>

#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/MdvxField.hh>
#include <mm5/SigmaInterp.hh>
#include <physics/PhysicsLib.hh>
#include <physics/physics.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include "MdvSigmaP2P.hh"

using namespace std;

// Constructor

MdvSigmaP2P::MdvSigmaP2P(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvSigmaP2P";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (
    _params.loadFromArgs(
      argc,
      argv,
      _args.override.list,
      &_paramsPath
    )
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // parse the start and end time

  if (
    sscanf(
      _params.start_date_time, "%d %d %d %d %d %d",
      &_startTime.year, &_startTime.month, &_startTime.day,
      &_startTime.hour, &_startTime.min, &_startTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&_startTime);
  }

  if (
    sscanf(
      _params.end_date_time, "%d %d %d %d %d %d",
      &_endTime.year, &_endTime.month, &_endTime.day,
      &_endTime.hour, &_endTime.min, &_endTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&_endTime);
  }

  // avoid duplicated field names
  if (_params.fields_to_convert_n > 0) {
    for (int i = 0; i < _params.fields_to_convert_n; i++) {
      _fieldSet.insert(_params._fields_to_convert[i]);
    }
  }

  if (!isOK) {
    _args.usage(_progName, cerr);
    return;
  }

  // init process mapper registration

  PMU_auto_init(
    (char *) _progName.c_str(),
    _params.instance,
    PROCMAP_REGISTER_INTERVAL
  );

  return;

}

// destructor

MdvSigmaP2P::~MdvSigmaP2P()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvSigmaP2P::Run ()
{

  int iret = 0;


  // register with procmap
  
  PMU_auto_register("Run");

  iret = _performConvert();

  return iret;

}

//////////////////////////////////////////////////
// convert

int MdvSigmaP2P::_performConvert() {

  if (_params.debug) {
    cerr << "Start: " << ctime(&(_startTime.unix_time));
    cerr << "End:   " << ctime(&(_endTime.unix_time));
  }

  DsMdvxTimes mdv_time_list;
  int iret = mdv_time_list.setArchive(
    _params.input_url_dir,
    _startTime.unix_time,
    _endTime.unix_time
  );
  if (iret) {
    cerr << "ERROR: MdvSigmaP2P::_performConvert()" << endl;
    cerr << "url: " << _params.input_url_dir << endl;
    cerr << mdv_time_list.getErrStr() << endl;
    return -1;
  }

  vector<time_t> _timelist = mdv_time_list.getArchiveList();
  int n_times = _timelist.size();
  if (n_times < 1)
    return -1;

  if (_params.debug) {
    cerr << "Total files: " << n_times << endl;
  }

  vector<string> fieldNames;
  set<string>::iterator iter;
  for (iter = _fieldSet.begin(); iter != _fieldSet.end(); iter++) {
    fieldNames.push_back(*iter);
  }

  DsMdvx geopot_htMdvx;
  MdvxField *geopotHtField = NULL;
  if (_params.convert_geopot_ht) {
    geopot_htMdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
    geopot_htMdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);
    geopot_htMdvx.setReadTime(Mdvx::READ_LAST, _params.geopot_ht_url);
    geopot_htMdvx.addReadField(_params.geopot_ht_field);

    if (geopot_htMdvx.readVolume()) {

      cerr << "ERROR: performConvert()" << endl;
      cerr << geopot_htMdvx.getErrStr() << endl;

    } else {

      geopotHtField = geopot_htMdvx.getFieldByName(_params.geopot_ht_field);

      if (_params.debug)
        cerr << "Read in geopot_ht file: " << geopot_htMdvx.getPathInUse() << endl;
    }
  }

  for (int file_index = 0; file_index < n_times; file_index++) {
    time_t mdv_time = _timelist.at(file_index);

    DsMdvx inMdvx;
    inMdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
    inMdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);
    inMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_url_dir,
      0,        // search margin
      mdv_time, // search time
      0         // forecast lead time
    );

    if (inMdvx.readVolume()) {
      cerr << "ERROR: performConvert() - readVolume" << endl;
      cerr << inMdvx.getErrStr() << endl;
      continue; // continue to the next file
    }

    if (_params.debug) {
      cerr << "Read in file: " << inMdvx.getPathInUse() << endl;
    }

    MdvxField *pressure_field = inMdvx.getFieldByName(
      _params.pressure_field
    );
    if (pressure_field == NULL) {
      cerr << "ERROR: performConvert()" << endl;
      cerr << "File does not have pressure field." << endl;
      continue; // continue to the next file
    }

    pressure_field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    pressure_field->transform2Linear();
    pressure_field->setPlanePtrs();

    DsMdvx outMdvx;
    outMdvx.setMasterHeader(inMdvx.getMasterHeader());

    int nFields;
    if (_params.fields_to_convert_n > 0)
      nFields = fieldNames.size();
    else
      nFields = inMdvx.getNFields();

    fl32* pres_vol = (fl32 *)pressure_field->getVol();

    // add geopotential height if needed

    bool geopot_ht_added = false;
    if (_params.convert_geopot_ht && geopotHtField != NULL) {

      // create a copy so the original will not be changed.

      MdvxField *geoFieldCopy = new MdvxField(*geopotHtField);

      int ret = _convertField(
        outMdvx,
        geoFieldCopy,
        pres_vol,
        true // is_geopot_ht
      );
      if (!ret)
        geopot_ht_added = true;

      delete geoFieldCopy;
    }

    for (int field_index = 0; field_index < nFields; field_index++) {
      MdvxField *inField;
      if (fieldNames.size() > 0) {
        inField = inMdvx.getFieldByName(fieldNames[field_index]);
      } else {
        inField = inMdvx.getFieldByNum(field_index);
      }

      if (inField == NULL) {
        cerr << "ERROR: performConvert()" << endl;
        if (fieldNames.size() > 0)
          cerr << "Cannot find field: " << fieldNames[field_index] << endl;
        else
          cerr << "Cannot find the " << field_index << "th field." << endl;
      }

      if (STRequal(inField->getFieldName(), _params.geopot_ht_field)) {
        if (geopot_ht_added)
          continue;

        _convertField(outMdvx, inField, pres_vol, true/*is_geopot_ht*/);
        continue;
      }

      // other fields
      _convertField(outMdvx, inField, pres_vol, false/*is_geopot_ht*/);

    } // field loop

    // adding the rest fields

    int n_added_fields = outMdvx.getNFields();
    int n_in_fields = inMdvx.getNFields();

    if (n_added_fields < n_in_fields) {

      set<string> fields_converted;
      for (int i = 0; i < n_added_fields; i++) {
        MdvxField *writtenField = outMdvx.getField(i);
        fields_converted.insert(writtenField->getFieldName());
      }

      for (int i = 0; i < n_in_fields; i++) {
        MdvxField *inField = inMdvx.getFieldByNum(i);
        string field_name = inField->getFieldName();
        set<string>::iterator iter = fields_converted.find(field_name);
        if (iter != fields_converted.end())
          continue;

        if (_params.debug == Params::DEBUG_VERBOSE) {
          cerr << "Added original field: " << field_name << endl;
	}

        MdvxField *outField = new MdvxField(*inField);
        outMdvx.addField(outField);
      }

      Mdvx::master_header_t outMhdr = outMdvx.getMasterHeader();
      outMhdr.vlevel_type = Mdvx::VERT_TYPE_VARIABLE;
      outMdvx.setMasterHeader(outMhdr);

    }

    // write out

    outMdvx.setAppName(_progName);
    outMdvx.setWriteLdataInfo();

    if (outMdvx.writeToDir(_params.output_url_dir)) {
      cerr << "ERROR: MdvSigmaP2P::_performConvert()" << endl;
      cerr << "Cannot write to dir: " << _params.output_url_dir << endl;
      cerr << outMdvx.getErrStr() << endl;
      continue;
    }

  } // file loop

  return 0;
}

//////////////////////////////////////////////////
// convert

int MdvSigmaP2P::_convertField(
  DsMdvx &outMdvx,
  MdvxField *inField,
  fl32 *pres_vol,
  bool is_geopot_ht
) {

  const Mdvx::field_header_t &inFhdr = inField->getFieldHeader();
  if (inFhdr.nz == 1) {
    cerr << "Field " << inFhdr.field_name
         << " is not a 3D field. Skipped." << endl;
    return -1;
  }

  const Mdvx::vlevel_header_t &inVhdr = inField->getVlevelHeader();
  if (inVhdr.type[0] != Mdvx::VERT_TYPE_SIGMA_P) {
    cerr << "Field " << inFhdr.field_name
         << " is not in Sigma P levels. Skipped." << endl
         << "field name: " << inField->getFieldName() << endl;
    return -1;
  }

  if (_params.debug == Params::DEBUG_VERBOSE)
    cerr << " Field: " << inField->getFieldName() << endl;

  // save important information for output purpose

  Mdvx::encoding_type_t encoding_type =
    (Mdvx::encoding_type_t)inFhdr.encoding_type;
  Mdvx::compression_type_t compression_type =
    (Mdvx::compression_type_t)inFhdr.compression_type;
  Mdvx::transform_type_t transform_type =
    (Mdvx::transform_type_t)inFhdr.transform_type;

  // convert input to float32, none-compression, linear
  inField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  inField->transform2Linear();
  inField->setPlanePtrs();

  // set output field header
  Mdvx::field_header_t outFhdr = inFhdr;
  outFhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  outFhdr.compression_type = Mdvx::COMPRESSION_NONE;
  outFhdr.transform_type = transform_type;
  outFhdr.min_value = 0.0;
  outFhdr.max_value = 0.0;
  outFhdr.min_value_orig_vol = 0.0;
  outFhdr.max_value_orig_vol = 0.0;
  outFhdr.nz = _params.pressure_levels_n;
  outFhdr.bad_data_value = SigmaInterp::MissingDouble;
  outFhdr.missing_data_value = SigmaInterp::MissingDouble;
  outFhdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;

  int out_vol_size = outFhdr.nx * outFhdr.ny * outFhdr.nz;
  outFhdr.volume_size = out_vol_size * sizeof(fl32);

  Mdvx::vlevel_header_t outVhdr;
  MEM_zero(outVhdr);

  for (int iz = 0; iz < outFhdr.nz; iz++) {
    outVhdr.type[iz] = Mdvx::VERT_TYPE_PRESSURE;
    outVhdr.level[iz] = _params._pressure_levels[iz];
  }

  fl32 *inVol = (fl32 *)inField->getVol();
  fl32 *outVol = (fl32 *)malloc(sizeof(fl32) * out_vol_size);

  //init with missing
  for (int i = 0; i < out_vol_size; i++) {
    outVol[i] = SigmaInterp::MissingDouble;
  }

  /* before doing linear interpolation for 'z',
   * calculate exp(-Z/Scale) first.
   * Scale = R*Temp/G, R=287.04, Temp=256, G=9.81
   */

  if (is_geopot_ht) {
    int size = inFhdr.nx * inFhdr.ny * inFhdr.nz;
    double scale_ht = PhysicsLib::RGAS * 256 / PhysicsLib::GRAVITY_CONSTANT;
    for (int i = 0; i < size; i++) {
	  if (inVol[i] != inFhdr.missing_data_value && inVol[i] != inFhdr.bad_data_value)
        inVol[i] = exp(-(inVol[i])/scale_ht);
    }
  }

  // now do linear interpolation

  for (int x = 0; x < inFhdr.nx; x++) {

    for (int y = 0; y < inFhdr.ny; y++) {

      vector<double> pressure_sigma_vals;
      vector<double> field_sigma_vals;

      for (int z = 0; z < inFhdr.nz; z++) {
        int index = inFhdr.nx * inFhdr.ny * z + x * inFhdr.ny + y;
        pressure_sigma_vals.push_back(pres_vol[index]);
        field_sigma_vals.push_back(inVol[index]);
      }

      SigmaInterp sigmaInterp;
      sigmaInterp.setPressureLevels(
        _params.pressure_levels_n,
        _params._pressure_levels
      );

      sigmaInterp.prepareInterp(pressure_sigma_vals);

      vector<double> field_vals;
      sigmaInterp.doInterp(field_sigma_vals, field_vals, _params.copy_lowest_downwards);

      for (int z = 0; z < outFhdr.nz; z++) {
        int index = inFhdr.nx * inFhdr.ny * z + x * inFhdr.ny + y;
        outVol[index] = field_vals[z];
      }
    }
  }

  // back to 'z'

  if (is_geopot_ht) {
    double scale_ht = PhysicsLib::RGAS * 256 / PhysicsLib::GRAVITY_CONSTANT;
    for (int i = 0; i < out_vol_size; i++) {
      if (outVol[i] != SigmaInterp::MissingDouble && outVol[i] > 0) {
        outVol[i] = log(outVol[i]) * scale_ht * (-1);
      }
    }
  }

  MdvxField *outField = new MdvxField(outFhdr, outVhdr, outVol);

  // back to original encoding and compression
  outField->convertType(
    encoding_type,
    compression_type
  );

  if (transform_type == Mdvx::DATA_TRANSFORM_LOG)
    outField->transform2Log();

  outMdvx.addField(outField);

  free(outVol);

  return 0;
}

