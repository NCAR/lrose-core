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
// Derived.cc
//
// Handle derived fields
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2005
//
///////////////////////////////////////////////////////////////

#include "Params.hh"
#include "DsMdvServer.hh"
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxMsg.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaStr.hh>
using namespace std;

/////////////////////////////////////////////////////////
// Read all headers, adding in headers for derived fields
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_readDerivedAllHdrs(DsMdvx &mdvx)
  
{

  // read in all headers in file
  
  if (mdvx.readAllHeaders()) {
    return -1;
  }
  const vector<Mdvx::field_header_t> &fileFhdrs = mdvx.getFieldHeadersFile();
  const vector<Mdvx::vlevel_header_t> &fileVhdrs = mdvx.getVlevelHeadersFile();
  if (fileFhdrs.size() < 1) {
    mdvx.addToErrStr("ERROR - DsMdvServer::_readDerivedAllHdrs\n");
    mdvx.addToErrStr("  No fields found in file\n");
    return -1;
  }
  if (fileFhdrs.size() != fileVhdrs.size()) {
    mdvx.addToErrStr("ERROR - DsMdvServer::_readDerivedAllHdrs\n");
    mdvx.addToErrStr("  Number of field headers in file "
                     "differs from number of vlevel headers\n");
    char text[1024];
    sprintf(text, "  N field headers: %d\n", (int) fileFhdrs.size());
    mdvx.addToErrStr(text);
    sprintf(text, "  N vlevel headers: %d\n", (int) fileVhdrs.size());
    mdvx.addToErrStr(text);
    return -1;
  }

  // loop through derived fields listed in param file
  
  vector<Mdvx::field_header_t> derivedFhdrs;
  vector<Mdvx::vlevel_header_t> derivedVhdrs;

  for (int ii = 0; ii < _params.derived_fields_n; ii++) {
    
    string derivedName = _params._derived_fields[ii].name;
    
    // get templates for derived headers
    
    Mdvx::field_header_t derivedFhdr = fileFhdrs[0];
    Mdvx::vlevel_header_t derivedVhdr = fileVhdrs[0];
    for (size_t jj = 0; jj < fileFhdrs.size(); jj++) {
      string fileFieldName = fileFhdrs[jj].field_name;
      if (derivedName == fileFieldName) {
        derivedFhdr = fileFhdrs[jj];
        derivedVhdr = fileVhdrs[jj];
        break;
      }
    }
    
    // copy in name and units

    STRncopy(derivedFhdr.field_name,
             _params._derived_fields[ii].name,
             MDV_SHORT_FIELD_LEN);

    STRncopy(derivedFhdr.field_name_long,
             _params._derived_fields[ii].long_name,
             MDV_LONG_FIELD_LEN);
    
    STRncopy(derivedFhdr.units,
             _params._derived_fields[ii].units,
             MDV_UNITS_LEN);

    STRncopy(derivedFhdr.transform,
             _params._derived_fields[ii].transform,
             MDV_TRANSFORM_LEN);

    derivedFhdrs.push_back(derivedFhdr);
    derivedVhdrs.push_back(derivedVhdr);

  } // ii

  // add derived headers to object

  for (size_t ii = 0; ii < derivedFhdrs.size(); ii++) {
    mdvx._fhdrsFile.push_back(derivedFhdrs[ii]);
    mdvx._vhdrsFile.push_back(derivedVhdrs[ii]);
  }

  // increment the number of fields in the master header

  mdvx._mhdrFile.n_fields += derivedFhdrs.size();

  return 0;

}

/////////////////////////////////////////////////////////
// Handle derived fields
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_handleDerivedFields(DsMdvx &mdvx,
				      DsMdvServer::read_action_t action)
  
{

  // make a copy of the incoming Mdvx object, for use in retrieves

  DsMdvx mdvxTemplate(mdvx);

  // determine which fields are to be derived, and which are not to
  // be derived but retrieved as usual
  
  set<string> normalFieldNames;
  vector<Params::derived_field_t *> derivedFields;
  
  for (int jj = 0; jj < (int) mdvx._readFieldNames.size(); jj++) {
    bool found = false;
    const string &readName = mdvx._readFieldNames[jj];
    for (int ii = 0; ii < _params.derived_fields_n; ii++) {
      string derivedName = _params._derived_fields[ii].name;
      if (readName == derivedName) {
	found = true;
	derivedFields.push_back(_params._derived_fields + ii);
	break;
      }
    }
    if (!found) {
      normalFieldNames.insert(readName);
    }
  } // jj

  if (_isDebug) {
    cerr << "Deriving following fields:" << endl;
    for (int ii = 0; ii < (int) derivedFields.size(); ii++) {
      string derivedName = derivedFields[ii]->name;
      cerr << "  " << derivedName << endl;
    }
    cerr << "Retrieving following fields as normal:" << endl;
    for (set<string>::iterator it = normalFieldNames.begin();
	 it != normalFieldNames.end(); it++) {
      cerr << "  " << *it << endl;
    }
  }

  // build up a list of all the base fields which are needed

  set<string> baseFields;

  for (set<string>::iterator it = normalFieldNames.begin();
       it != normalFieldNames.end(); it++) {
    baseFields.insert(*it);
  }
  
  for (int ii = 0; ii < (int) derivedFields.size(); ii++) {
    baseFields.insert(derivedFields[ii]->field_name_1);
    switch (derivedFields[ii]->function) {
    case Params::FUNC_SPEED_FROM_U_V:
    case Params::FUNC_DIRN_FROM_U_V:
    case Params::FUNC_DIFF_FIELDS_SAME_FILE:
      baseFields.insert(derivedFields[ii]->field_name_2);
    default: {}
    }
  }

  if (_isDebug) {
    for (set<string>::iterator it = baseFields.begin();
	 it != baseFields.end(); it++) {
      cerr << "base field: " << *it << endl;
    }
  }

  // read base fields

  DsMdvx base(mdvxTemplate);
  base.clearReadFields();
  
  for (set<string>::iterator it = baseFields.begin();
       it != baseFields.end(); it++) {
    base.addReadField(*it);
  }

  // save requested types, set encoding to FLOAT32, compression to none
  
  Mdvx::encoding_type_t requestedEncoding = base._readEncodingType;
  Mdvx::compression_type_t requestedCompression = base._readCompressionType;
  Mdvx::scaling_type_t requestedScaling = base._readScalingType;
  double requestedScale = base._readScale;
  double requestedBias = base._readBias;
  
  if (_isDebug) {
    switch (action) {
      case READ_VOLUME:
        cerr << "READ_VOLUME REQUEST" << endl;
        break;
      case READ_VSECTION:
        cerr << "READ_VSECTION REQUEST" << endl;
        break;
      default: {}
    }
    cerr << "--->> DsMdvServer::_handleDerivedFields <<---" << endl;
    cerr << "Request for base fields" << endl;
    base.printReadRequest(cerr);
  }

  int iret = 0;
  switch (action) {
    case READ_VOLUME:
      iret = base.readVolume();
      break;
    case READ_VSECTION:
      iret = base.readVsection();
      break;
    default:
      iret = -1;
  }

  if (iret) {
    mdvx.addToErrStr("ERROR - DsMdvServer::_handleDerivedFields\n");
    mdvx.addToErrStr("  Cannot read base fields\n");
    for (set<string>::iterator it = baseFields.begin();
	 it != baseFields.end(); it++) {
      mdvx.addToErrStr("    ", *it, "\n");
    }
    mdvx.addToErrStr(base.getErrStr());
    if (_isDebug) {
      cerr << mdvx.getErrStr();
    }
    return -1;
  }

  // copy the path in use into the return object
  mdvx._pathInUse = base.getPathInUse();

  // copy the normal fields and chunks into return object
  mdvx.setMasterHeader(base.getMasterHeader());
  mdvx.clearFields();
  mdvx.clearChunks();

  for (set<string>::iterator it = normalFieldNames.begin();
       it != normalFieldNames.end(); it++) {
    string fieldName(*it);
    MdvxField *fld = base.getFieldByName(fieldName);
    if (fld) {
      MdvxField *copy = new MdvxField(*fld);
      copy->convertType(requestedEncoding, requestedCompression,
			requestedScaling, requestedScale, requestedBias);
      mdvx.addField(copy);
    }
  }

  for (int ii = 0; ii < base.getNChunks(); ii++) {
    MdvxChunk *chunk = base.getChunkByNum(ii);
    if (chunk) {
      MdvxChunk *copy = new MdvxChunk(*chunk);
      mdvx.addChunk(copy);
    }
  }

  // for each derived field, determine the output encoding and compression

  vector<Mdvx::encoding_type_t> outEncoding;
  vector<Mdvx::compression_type_t> outCompression;

  for (int ii = 0; ii < (int) derivedFields.size(); ii++) {
    
    // find the primary data field for this derived field
    
    const MdvxField *field1 = base.getFieldByName(derivedFields[ii]->field_name_1);
    if (field1) {
      const Mdvx::field_header_t &fhdr = field1->getFieldHeader();
      Mdvx::encoding_type_t inEncoding = (Mdvx::encoding_type_t) fhdr.encoding_type;
      Mdvx::compression_type_t inCompression = (Mdvx::compression_type_t) fhdr.compression_type;
      if (requestedEncoding == Mdvx::ENCODING_ASIS) {
        outEncoding.push_back(inEncoding);
      } else {
        outEncoding.push_back(requestedEncoding);
      }
      if (requestedCompression == Mdvx::COMPRESSION_ASIS) {
        outCompression.push_back(inCompression);
      } else {
        outCompression.push_back(requestedCompression);
      }
    }

  } // ii
  
  // convert the base fields to uncompressed floats

  for (int ii = 0; ii < base.getMasterHeader().n_fields; ii++) {
    MdvxField *fld = base.getField(ii);
    fld->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  }

  for (int ii = 0; ii < (int) derivedFields.size(); ii++) {

    string errStr;
    MdvxField *fld = _createDerivedField(derivedFields[ii], base,
					 action, errStr);
    if (fld) {
      fld->convertType(outEncoding[ii], outCompression[ii],
		       requestedScaling, requestedScale, requestedBias);
      mdvx.addField(fld);
    } else {
      mdvx.clearFields();
      mdvx.clearChunks();
      mdvx.clearErrStr();
      mdvx.addToErrStr("ERROR - DsMdvServer::_handleDerivedFields\n");
      mdvx.addToErrStr(errStr);
      return -1;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////
// create a derived field
//
// Returns MdvxField* on success, NULL on failure

MdvxField *DsMdvServer::_createDerivedField(Params::derived_field_t *derived,
					    const DsMdvx &base,
					    DsMdvServer::read_action_t action,
					    string &errStr)
  
{

  // initialize error message
  
  errStr = "";
  errStr += "ERROR - DsMdvServer::_createDerivedField\n";
  errStr += "  Cannot create derived field: ";
  errStr += derived->name;
  errStr += "\n";

  MdvxField *field = NULL;

  // create correct type of derived field

  switch (derived->function) {
    
  case Params::FUNC_LINEAR:
    field = _createLinear(derived, base, errStr);
    break;

  case Params::FUNC_SPEED_FROM_U_V:
    field = _createSpeedFromUV(derived, base, errStr);
    break;

  case Params::FUNC_DIRN_FROM_U_V:
    field = _createDirnFromUV(derived, base, errStr);
    break;

  case Params::FUNC_DIFF_FIELDS_SAME_FILE:
    field = _createDiffFieldsSameFile(derived, base, errStr);
    break;

  case Params::FUNC_DIFF_FIELDS:
    field = _createDiffFields(derived, base, action, errStr);
    break;

  case Params::FUNC_DIFF_IN_TIME:
    field = _createDiffInTime(derived, base, action, errStr);
    break;

  case Params::FUNC_VERT_COMPOSITE:
    field = _createVertComposite(derived, base, action, errStr);
    break;

  } // switch

  if (!field) {
    return NULL;
  }

  // apply multiplier and constant
  
  double mult = derived->multiplier;
  double constant = derived->constant;
  fl32 *val = (fl32 *) field->getVol();
  const Mdvx::field_header_t &fhdr = field->getFieldHeader();
  int npts = fhdr.nx * fhdr.ny * fhdr.nz;
  fl32 missing = fhdr.missing_data_value;
  fl32 bad = fhdr.bad_data_value;
  for (int ii = 0; ii < npts; ii++, val++) {
    fl32 value = *val;
    if ((value != missing) && (value != bad)) {
      *val = (value * mult) + constant;
    }
  }

  // set name, units etc

  field->setFieldName(derived->name);
  field->setFieldNameLong(derived->long_name);
  field->setUnits(derived->units);
  field->setTransform(derived->transform);
  field->computeMinAndMax(true);

  STRncopy(field->_fhdrFile->field_name, derived->name, MDV_SHORT_FIELD_LEN );
  STRncopy(field->_fhdrFile->field_name_long, derived->long_name, MDV_LONG_FIELD_LEN );
  STRncopy(field->_fhdrFile->units, derived->units, MDV_UNITS_LEN );
  STRncopy(field->_fhdrFile->transform, derived->transform, MDV_TRANSFORM_LEN );

  return field;

}

/////////////////////////////////////////////////////////
// create a derived field using FUNC_LINEAR
//
// Returns MdvxField* on success, NULL on failure

MdvxField *DsMdvServer::_createLinear(Params::derived_field_t *derived,
				      const DsMdvx &base,
				      string &errStr)
  
{

  if (_isDebug) {
    cerr << "Creating derived field: FUNC_LINEAR" << endl;
  }

  // get the field from the base object

  MdvxField *fld = base.getFieldByName(derived->field_name_1);
  if (!fld) {
    errStr += "MdvxField *DsMdvServer::_createLinear\n";
    TaStr::AddStr(errStr, "  Cannot find base field name: ", derived->field_name_1);
    return NULL;
  }

  // Make a copy of the base field, return it.
  // Linear scaling will be performed in calling routine.
  
  MdvxField *copy = new MdvxField(*fld);
  copy->transform2Linear();
  return copy;

}

/////////////////////////////////////////////////////////
// create a derived field using FUNC_SPEED_FROM_U_V
//
// Returns MdvxField* on success, NULL on failure

MdvxField *DsMdvServer::_createSpeedFromUV(Params::derived_field_t *derived,
					   const DsMdvx &base,
					   string &errStr)
  
{

  if (_isDebug) {
    cerr << "Creating derived field: FUNC_SPEED_FROM_U_V" << endl;
  }

  // get the U, V fields from the base object

  MdvxField *ufld = base.getFieldByName(derived->field_name_1);
  if (!ufld) {
    errStr += "MdvxField *DsMdvServer::_createSpeedFromUV\n";
    TaStr::AddStr(errStr, "  Cannot find U field: ", derived->field_name_1);
    return NULL;
  }
  ufld->transform2Linear();

  MdvxField *vfld = base.getFieldByName(derived->field_name_2);
  if (!vfld) {
    errStr += "MdvxField *DsMdvServer::_createSpeedFromUV\n";
    TaStr::AddStr(errStr, "  Cannot find V field: ", derived->field_name_2);
    return NULL;
  }
  vfld->transform2Linear();

  // check that fields have the same grid geometry

  const Mdvx::field_header_t &ufhdr = ufld->getFieldHeader();
  const Mdvx::field_header_t &vfhdr = vfld->getFieldHeader();
  if (!_checkFieldGeometry(ufhdr, vfhdr)) {
    errStr += "MdvxField *DsMdvServer::_createSpeedFromUV\n";
    TaStr::AddStr(errStr, "  U and V fields have different geometry");
    TaStr::AddStr(errStr, "  U field: ", derived->field_name_1);
    TaStr::AddStr(errStr, "  V field: ", derived->field_name_2);
    return NULL;
  }

  // make copy of u field to be the template for the speed field

  MdvxField *speedField = new MdvxField(*ufld);
  const Mdvx::field_header_t &sfhdr = speedField->getFieldHeader();
  
  // compute speed from U and V

  fl32 *spd = (fl32 *) speedField->getVol();

  fl32 *uu = (fl32 *) ufld->getVol();
  fl32 *vv = (fl32 *) vfld->getVol();
  fl32 uMissing = ufhdr.missing_data_value;
  fl32 vMissing = vfhdr.missing_data_value;
  fl32 sMissing = sfhdr.missing_data_value;
  int npts = sfhdr.nx * sfhdr.ny * sfhdr.nz;

  for (int ii = 0; ii < npts; ii++, spd++, uu++, vv++) {
    fl32 uVal = *uu;
    fl32 vVal = *vv;
    if (uVal == uMissing || vVal == vMissing) {
      *spd = sMissing;
    } else {
      *spd = (fl32) sqrt(uVal * uVal + vVal * vVal);
    }
  }

  return speedField;

}

/////////////////////////////////////////////////////////
// create a derived field using FUNC_DIRN_FROM_U_V
//
// Returns MdvxField* on success, NULL on failure

MdvxField *DsMdvServer::_createDirnFromUV(Params::derived_field_t *derived,
					  const DsMdvx &base,
					  string &errStr)
  
{

  if (_isDebug) {
    cerr << "Creating derived field: FUNC_DIRN_FROM_U_V" << endl;
  }

  // get the U, V fields from the base object

  MdvxField *ufld = base.getFieldByName(derived->field_name_1);
  if (!ufld) {
    errStr += "MdvxField *DsMdvServer::_createDirnFromUV\n";
    TaStr::AddStr(errStr, "  Cannot find U field: ", derived->field_name_1);
    return NULL;
  }
  ufld->transform2Linear();

  MdvxField *vfld = base.getFieldByName(derived->field_name_2);
  if (!vfld) {
    errStr += "MdvxField *DsMdvServer::_createDirnFromUV\n";
    TaStr::AddStr(errStr, "  Cannot find V field: ", derived->field_name_2);
    return NULL;
  }
  vfld->transform2Linear();

  // check that fields have the same grid geometry

  const Mdvx::field_header_t &ufhdr = ufld->getFieldHeader();
  const Mdvx::field_header_t &vfhdr = vfld->getFieldHeader();
  if (!_checkFieldGeometry(ufhdr, vfhdr)) {
    errStr += "MdvxField *DsMdvServer::_createDirnFromUV\n";
    TaStr::AddStr(errStr, "  U and V fields have different geometry");
    TaStr::AddStr(errStr, "  U field: ", derived->field_name_1);
    TaStr::AddStr(errStr, "  V field: ", derived->field_name_2);
    return NULL;
  }

  // make copy of u field to be the template for the dirn field

  MdvxField *dirnField = new MdvxField(*ufld);
  const Mdvx::field_header_t &dfhdr = dirnField->getFieldHeader();
  
  // compute dirn from U and V

  fl32 *dirn = (fl32 *) dirnField->getVol();
  fl32 *uu = (fl32 *) ufld->getVol();
  fl32 *vv = (fl32 *) vfld->getVol();
  fl32 uMissing = ufhdr.missing_data_value;
  fl32 vMissing = vfhdr.missing_data_value;
  fl32 dMissing = dfhdr.missing_data_value;
  int npts = dfhdr.nx * dfhdr.ny * dfhdr.nz;

  for (int ii = 0; ii < npts; ii++, dirn++, uu++, vv++) {
    fl32 uVal = *uu;
    fl32 vVal = *vv;
    if (uVal == uMissing || vVal == vMissing) {
      *dirn = dMissing;
    } else {
      double dd = 0.0;
      if (uVal == 0.0 && vVal == 0.0) {
	dd = 0.0;
      } else {
	dd = atan2(-uVal, -vVal) / DEG_TO_RAD;
      }
      if (dd < 0) {
	dd += 360.0;
      }
      *dirn = (fl32) dd;
    }
  }

  return dirnField;

}

///////////////////////////////////////////////////////////
// create a derived field using FUNC_DIFF_FIELDS_SAME_FILE
//
// Returns MdvxField* on success, NULL on failure

MdvxField *DsMdvServer::_createDiffFieldsSameFile(Params::derived_field_t *derived,
						  const DsMdvx &base,
						  string &errStr)
  
{

  if (_isDebug) {
    cerr << "Creating derived field: FUNC_DIFF_FIELDS_SAME_FILE" << endl;
  }

  // get the two fields from the base object
  
  MdvxField *fld1 = base.getFieldByName(derived->field_name_1);
  if (!fld1) {
    errStr += "MdvxField *DsMdvServer::_createDiffFieldsSameFile\n";
    TaStr::AddStr(errStr, "  Cannot find field 1: ", derived->field_name_1);
    return NULL;
  }
  fld1->transform2Linear();
  fld1->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  MdvxField *fld2 = base.getFieldByName(derived->field_name_2);
  if (!fld2) {
    errStr += "MdvxField *DsMdvServer::_createDiffFieldsSameFile\n";
    TaStr::AddStr(errStr, "  Cannot find field 2: ", derived->field_name_2);
    return NULL;
  }
  fld2->transform2Linear();
  fld2->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  // check that fields have the same grid geometry

  const Mdvx::field_header_t &fhdr1 = fld1->getFieldHeader();
  const Mdvx::field_header_t &fhdr2 = fld2->getFieldHeader();
  if (!_checkFieldGeometry(fhdr1, fhdr2)) {
    errStr += "MdvxField *DsMdvServer::_createDiffFieldsSameFile\n";
    TaStr::AddStr(errStr, "  Fields 1 and 2 have different geometry");
    TaStr::AddStr(errStr, "  Field 1: ", derived->field_name_1);
    TaStr::AddStr(errStr, "  Field 2: ", derived->field_name_2);
    return NULL;
  }

  // make copy of field 1 to be the template for the diff field
  
  MdvxField *diffField = new MdvxField(*fld1);
  const Mdvx::field_header_t &dfhdr = diffField->getFieldHeader();
  
  // compute diff from U and V
  
  fl32 *diff = (fl32 *) diffField->getVol();
  fl32 *f1 = (fl32 *) fld1->getVol();
  fl32 *f2 = (fl32 *) fld2->getVol();
  fl32 uMissing = fhdr1.missing_data_value;
  fl32 vMissing = fhdr2.missing_data_value;
  fl32 dMissing = dfhdr.missing_data_value;
  int npts = dfhdr.nx * dfhdr.ny * dfhdr.nz;
  
  for (int ii = 0; ii < npts; ii++, diff++, f1++, f2++) {
    fl32 val1 = *f1;
    fl32 val2 = *f2;
    if (val1 == uMissing || val2 == vMissing) {
      *diff = dMissing;
    } else {
      *diff = val1 - val2;
    }
  }

  return diffField;

}

/////////////////////////////////////////////////////////
// create a derived field using FUNC_DIFF_FIELDS
//
// Returns MdvxField* on success, NULL on failure

MdvxField *DsMdvServer::_createDiffFields(Params::derived_field_t *derived,
					  const DsMdvx &base,
					  DsMdvServer::read_action_t action,
					  string &errStr)
  
{

  if (_isDebug) {
    cerr << "Creating derived field: FUNC_DIFF_FIELDS" << endl;
  }

  // get the first field from the base object
  
  MdvxField *fld1 = base.getFieldByName(derived->field_name_1);
  if (!fld1) {
    errStr += "MdvxField *DsMdvServer::_createDiffFields\n";
    TaStr::AddStr(errStr, "  Cannot find field 1: ", derived->field_name_1);
    return NULL;
  }
  fld1->transform2Linear();
  fld1->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  // get the second field from another file based on url_2

  DsMdvx other(base);
  if (base._readTimeSet) {
    other.setReadTime(base._readSearchMode,
		      derived->url_2,
		      base._readSearchMargin,
		      base._readSearchTime + derived->i_arg_1,
		      base._readForecastLeadTime);
  } else {
    other.setReadPath(derived->url_2);
  }
  other.clearReadFields();
  other.addReadField(derived->field_name_2);
  
  if (_isDebug) {
    cerr << "Deriving field - DIFF_FIELDS" << endl;
    other.printReadRequest(cerr);
  }

  int iret = 0;
  switch (action) {
    case READ_VOLUME:
      iret = other.readVolume();
      break;
    case READ_VSECTION:
      iret = other.readVsection();
      break;
    default:
      iret = -1;
  }

  if (_isDebug) {
    cerr << "read path: " << other.getPathInUse() << endl;
  }

  if (iret) {
    errStr += "MdvxField *DsMdvServer::_createDiffFields\n";
    TaStr::AddStr(errStr, "  Cannot read in field 2: ", derived->field_name_2);
    TaStr::AddStr(errStr, "  Url: ", derived->url_2);
    errStr += other.getErrStr();
    return NULL;
  }

  MdvxField *fld2 = other.getFieldByName(derived->field_name_2);
  if (!fld2) {
    errStr += "MdvxField *DsMdvServer::_createDiffFields\n";
    TaStr::AddStr(errStr, "  Cannot find field 2: ", derived->field_name_2);
    return NULL;
  }
  fld2->transform2Linear();
  fld2->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  // check that fields have the same grid geometry
  
  const Mdvx::field_header_t &fhdr1 = fld1->getFieldHeader();
  const Mdvx::field_header_t &fhdr2 = fld2->getFieldHeader();
  if (!_checkFieldGeometry(fhdr1, fhdr2)) {
    errStr += "MdvxField *DsMdvServer::_createDiffFields\n";
    TaStr::AddStr(errStr, "  Fields 1 and 2 have different geometry");
    TaStr::AddStr(errStr, "  Field 1: ", derived->field_name_1);
    TaStr::AddStr(errStr, "  Field 2: ", derived->field_name_2);
    return NULL;
  }

  // make copy of field 1 to be the template for the diff field
  
  MdvxField *diffField = new MdvxField(*fld1);
  const Mdvx::field_header_t &dfhdr = diffField->getFieldHeader();
  
  // compute diff from U and V
  
  fl32 *diff = (fl32 *) diffField->getVol();
  fl32 *f1 = (fl32 *) fld1->getVol();
  fl32 *f2 = (fl32 *) fld2->getVol();
  fl32 uMissing = fhdr1.missing_data_value;
  fl32 vMissing = fhdr2.missing_data_value;
  fl32 dMissing = dfhdr.missing_data_value;
  int npts = dfhdr.nx * dfhdr.ny * dfhdr.nz;
  
  for (int ii = 0; ii < npts; ii++, diff++, f1++, f2++) {
    fl32 val1 = *f1;
    fl32 val2 = *f2;
    if (val1 == uMissing || val2 == vMissing) {
      *diff = dMissing;
    } else {
      *diff = val1 - val2;
    }
  }

  return diffField;

}

/////////////////////////////////////////////////////////
// create a derived field using FUNC_DIFF_IN_TIME
//
// Returns MdvxField* on success, NULL on failure

MdvxField *DsMdvServer::_createDiffInTime(Params::derived_field_t *derived,
					  const DsMdvx &base,
					  DsMdvServer::read_action_t action,
					  string &errStr)
  
{

  if (_isDebug) {
    cerr << "Creating derived field: FUNC_DIFF_IN_TIME" << endl;
  }

  // get the first field from the base object
  
  MdvxField *fld1 = base.getFieldByName(derived->field_name_1);
  if (!fld1) {
    errStr += "MdvxField *DsMdvServer::_createDiffInTime\n";
    TaStr::AddStr(errStr, "  Cannot find field 1: ", derived->field_name_1);
    return NULL;
  }
  fld1->transform2Linear();
  fld1->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  // get the second field from file at different time

  DsMdvx other(base);
  if (!base._readTimeSet) {
    errStr += "MdvxField *DsMdvServer::_createDiffInTime\n";
    errStr += "  File search specified by path.";
    errStr += "  Must specify original file search in time.";
    return NULL;
  }
  
  int timeDiff = derived->i_arg_1;
  time_t searchTime = base._readSearchTime + timeDiff;
  other.setReadTime(base._readSearchMode,
		    base._readDirUrl,
		    base._readSearchMargin,
		    searchTime,
		    base._readForecastLeadTime);

  other.clearReadFields();
  other.addReadField(derived->field_name_1);
  
  int iret = 0;
  switch (action) {
    case READ_VOLUME:
      iret = other.readVolume();
      break;
    case READ_VSECTION:
      iret = other.readVsection();
      break;
    default:
      iret = -1;
  }

  if (iret) {
    errStr += "MdvxField *DsMdvServer::_createDiffInTime\n";
    TaStr::AddStr(errStr, "  Cannot read in field: ", derived->field_name_1);
    TaStr::AddStr(errStr, "  Url: ", base._readDirUrl);
    TaStr::AddStr(errStr, "  Time: ", DateTime::strn(searchTime));
    errStr += other.getErrStr();
    return NULL;
  }

  MdvxField *fld2 = other.getFieldByName(derived->field_name_1);
  if (!fld2) {
    errStr += "MdvxField *DsMdvServer::_createDiffInTime\n";
    TaStr::AddStr(errStr, "  Cannot find field in second file: ",
		  derived->field_name_1);
    return NULL;
  }
  fld2->transform2Linear();
  fld2->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  // check that fields have the same grid geometry
  
  const Mdvx::field_header_t &fhdr1 = fld1->getFieldHeader();
  const Mdvx::field_header_t &fhdr2 = fld2->getFieldHeader();
  if (!_checkFieldGeometry(fhdr1, fhdr2)) {
    errStr += "MdvxField *DsMdvServer::_createDiffInTime\n";
    TaStr::AddStr(errStr, "  Times 1 and 2 have different geometry");
    TaStr::AddStr(errStr, "  Field name: ", derived->field_name_1);
    return NULL;
  }

  // make copy of field 1 to be the template for the diff field
  
  MdvxField *diffField = new MdvxField(*fld1);
  const Mdvx::field_header_t &dfhdr = diffField->getFieldHeader();
  
  // compute diff from U and V
  
  fl32 *diff = (fl32 *) diffField->getVol();
  fl32 *f1 = (fl32 *) fld1->getVol();
  fl32 *f2 = (fl32 *) fld2->getVol();
  fl32 uMissing = fhdr1.missing_data_value;
  fl32 vMissing = fhdr2.missing_data_value;
  fl32 dMissing = dfhdr.missing_data_value;
  int npts = dfhdr.nx * dfhdr.ny * dfhdr.nz;
  
  for (int ii = 0; ii < npts; ii++, diff++, f1++, f2++) {
    fl32 val1 = *f1;
    fl32 val2 = *f2;
    if (val1 == uMissing || val2 == vMissing) {
      *diff = dMissing;
    } else {
      *diff = val1 - val2;
    }
  }

  return diffField;

}

/////////////////////////////////////////////////////////
// create a derived field using FUNC_VERT_COMPOSITE
//
// Returns MdvxField* on success, NULL on failure

MdvxField *DsMdvServer::_createVertComposite(Params::derived_field_t *derived,
                                             const DsMdvx &base,
                                             DsMdvServer::read_action_t action,
                                             string &errStr)
  
{

  if (_isDebug) {
    cerr << "Creating derived field: FUNC_VERT_COMPOSITE" << endl;
  }
  
  // get the field from the base

  MdvxField *baseFld = base.getFieldByName(derived->field_name_1);
  if (!baseFld) {
    errStr += "MdvxField *DsMdvServer::_createVertComposite\n";
    TaStr::AddStr(errStr, "  Cannot find field: ", derived->field_name_1);
    return NULL;
  }

  if (action == READ_VSECTION) {

    // Make a copy of the base field and return it
    
    MdvxField *comp = new MdvxField(*baseFld);
    return comp;

  }

  // This is a READ_VOLUME request

  // make a copy of the base object
  // set it up for reading the composite

  DsMdvx work(base);
  work.clearReadFields();
  work.clearReadVertLimits();
  work.addReadField(baseFld->getFieldName());
  
  
  if (derived->i_arg_1 != 0) {
    double minVlevel = derived->d_arg_1;
    double maxVlevel = derived->d_arg_2;
    work.setReadVlevelLimits(minVlevel, maxVlevel);
    if (_isDebug) {
      cerr << "  Setting vlevels, min, max: "
	   << minVlevel << ", " << maxVlevel << endl;
    }
  }
  
  work.setReadComposite();
  work.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  work.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_isDebug) {
    cerr << "--->> DsMdvServer::_createVertComposite <<---" << endl;
    cerr << "Request for composite data" << endl;
    cerr << "READ_VOLUME REQUEST" << endl;
    work.printReadRequest(cerr);
  }

  if (work.readVolume()) {
    errStr += "MdvxField *DsMdvServer::_createVertComposite\n";
    TaStr::AddStr(errStr, "Cannot read vol: ", derived->field_name_1);
    TaStr::AddStr(errStr, "  ", work.getErrStr());
    return NULL;
  }
  
  // get the field from the working object

  MdvxField *workFld = work.getFieldByName(derived->field_name_1);
  if (!workFld) {
    errStr += "MdvxField *DsMdvServer::_createVertComposite\n";
    TaStr::AddStr(errStr, "  Cannot find field: ", derived->field_name_1);
    return NULL;
  }

  // For case of composite, need to set some file header entries
  // other than the names that are set in the calling routine.
  
  workFld->_fhdrFile->nz = 1;
  workFld->_fhdrFile->vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;

  workFld->_vhdrFile->type[0] =  Mdvx::VERT_TYPE_COMPOSITE;
  workFld->_vhdrFile->level[0] = 0.0;

  // Make a copy of the work field and return it
  
  MdvxField *comp = new MdvxField(*workFld);

  return comp;

}

/////////////////////////////////////////////////////////
// check fields have same geometry
//
// Returns true is same, false if different

bool DsMdvServer::_checkFieldGeometry(const Mdvx::field_header_t &fhdr1,
				      const Mdvx::field_header_t &fhdr2)

{  
  
  if (fhdr1.nx != fhdr2.nx ||
      fhdr1.ny != fhdr2.ny ||
      fhdr1.nz != fhdr2.nz ||
      fhdr1.proj_type != fhdr2.proj_type ||
      fabs(fhdr1.grid_minx - fhdr2.grid_minx) > 0.0001 ||
      fabs(fhdr1.grid_miny - fhdr2.grid_miny) > 0.0001 ||
      fabs(fhdr1.grid_minz - fhdr2.grid_minz) > 0.0001 ||
      fabs(fhdr1.grid_dx - fhdr2.grid_dx) > 0.0001 ||
      fabs(fhdr1.grid_dy - fhdr2.grid_dy) > 0.0001 ||
      fabs(fhdr1.grid_dz - fhdr2.grid_dz) > 0.0001 ||
      fabs(fhdr1.grid_minx - fhdr2.grid_minx) > 0.0001 ||
      fabs(fhdr1.proj_origin_lat - fhdr2.proj_origin_lat) > 0.0001 ||
      fabs(fhdr1.proj_origin_lon - fhdr2.proj_origin_lon) > 0.0001 ||
      fabs(fhdr1.proj_rotation - fhdr2.proj_rotation) > 0.0001) {

    if (_isDebug) {
      cerr << "========================================================" << endl;
      cerr << "==>> Geometry differs:" << endl;
      cerr << "Field 1: " << endl;
      Mdvx::printFieldHeaderSummary(fhdr1, cerr);
      cerr << "Field 2: " << endl;
      Mdvx::printFieldHeaderSummary(fhdr2, cerr);
      cerr << "========================================================" << endl;
    }

    return false;

  }

  return true;

}

/////////////////////////////////////////////////////////
// Compile time heights for derived fields
//
// Returns 0 on success, -1 on failure

int DsMdvServer::_compileDerivedTimeHeight(DsMdvx &mdvx)
  
{

  // clear fields, chunks
  
  mdvx.clearFields();
  mdvx.clearChunks();
  mdvx.clearErrStr();
  mdvx.addToErrStr("ERROR - DsMdvServer::_compileDerivedTimeHeight\n");
  
  // check a few things
  
  const MdvxTimeList &timeList = mdvx.getTimeListObj();
  if (timeList.getMode() == MdvxTimeList::MODE_UNDEFINED) {
    mdvx.addToErrStr("  You must specify a time list mode.\n");
    return -1;
  }

  const vector<Mdvx::vsect_waypt_t> &vsectWayPts = mdvx.getVsectWayPts();
  if (vsectWayPts.size() != 1) {
    mdvx.addToErrStr("  You must specify a single way-pt.\n");
    return -1;
  }

  // compile the time list
  
  if (mdvx.compileTimeList()) {
    mdvx.addToErrStr("  Cannot compile time list.\n");
    return -1;
  }
  if (timeList.getValidTimes().size() == 0) {
    mdvx.addToErrStr("  No suitable times found.\n");
    return -1;
  }

  // compile the time-height profile
  
  vector<DsMdvx *> vsects;
  int iret = 0;
  if (_doCompileDerivedTimeHeight(mdvx, vsects)) {
    iret = -1;
  }
  
  // clean up
  
  for (size_t ii = 0; ii < vsects.size(); ii++) {
    delete vsects[ii];
  }

  if (iret) {
    return -1;
  }

  mdvx.clearErrStr();
  return 0;

}


////////////////////////////////////////////////////
// compile the time-height profile

int DsMdvServer::_doCompileDerivedTimeHeight(DsMdvx &mdvx,
                                             vector<DsMdvx *> &vsects) 
  
{
  
  // read in the vertical point for each time
  
  const MdvxTimeList &timeList = mdvx.getTimeListObj();
  const vector<time_t> &vTimes = mdvx.getValidTimes();
  
  for (size_t ii = 0; ii < vTimes.size(); ii++) {
    DsMdvx *vsect = new DsMdvx(mdvx);
    vsects.push_back(vsect);
    vsect->setReadTime(Mdvx::READ_CLOSEST,
                       _incomingUrl, 3600,
                       vTimes[ii]);
    vsect->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    vsect->setReadCompressionType(Mdvx::COMPRESSION_NONE);
    vsect->_timeList.clearMode();
    vsect->_timeList.clearData();
    vsect->clearReadTimeListAlso();
    if (_isVerbose) {
      cerr << "READ_VSECTION REQUEST" << endl;
      cerr << "doCompileDerivedTimeHeight()" << endl;
      vsect->printReadRequest(cerr);
    }
    if (vsect->readVsection()) {
      mdvx.addToErrStr("  Cannot read in vsection for url: ",
                        _incomingUrl, vsect->getErrStr());
      mdvx.addToErrStr("    Time: ", DateTime::strn(vTimes[ii]));
      return -1;
    }
  } // ii

  // load up master header

  const vector<time_t> &validTimes = timeList.getValidTimes();
  mdvx._mhdr = vsects[0]->_mhdr;
  mdvx._mhdr.time_begin = validTimes[0];
  mdvx._mhdr.time_end = validTimes[validTimes.size() - 1];
  mdvx._mhdr.time_centroid = mdvx._mhdr.time_end;
  mdvx._mhdr.num_data_times = (si32) validTimes.size();
  mdvx._mhdr.data_dimension = 2;
  mdvx._mhdr.n_chunks = 0;
  mdvx.setDataSetInfo("Time height profile");

  // create the fields

  int nFields = mdvx._mhdr.n_fields;

  for (int ifield = 0; ifield < nFields; ifield++) {

    const MdvxField *fld0 = vsects[0]->_getRequestedField(ifield);
    if (fld0 == NULL) {
      return -1;
    }
    Mdvx::field_header_t fhdr0 = fld0->getFieldHeader();
    Mdvx::vlevel_header_t vhdr0 = fld0->getVlevelHeader();
    fl32 missing = -9999.0;

    // check that nz is same for all files

    for (size_t ii = 0; ii < vsects.size(); ii++) {
      const MdvxField *fld = vsects[ii]->_getRequestedField(ifield);
      if (fld == NULL) {
	return -1;
      }
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      if (fhdr.nz != fhdr0.nz) {
	string errStr = "  Nz not identical for all files\n";
	TaStr::AddStr(errStr, "    Path: ", vsects[ii]->_pathInUse);
	TaStr::AddInt(errStr, "    Expecting: ", fhdr0.nz);
	TaStr::AddInt(errStr, "    Found: ", fhdr.nz);
        mdvx.addToErrStr(errStr);
	return -1;
      }
    } // ii

    // customize the headers
    
    int nx = (int) vsects.size();
    double minx = (double) validTimes[0];
    double maxx = (double) validTimes[validTimes.size() - 1];
    double dx = (maxx - minx) / (nx - 1.0);
    fhdr0.nx = nx;
    fhdr0.grid_dx = dx;
    fhdr0.grid_minx = minx;
    
    fhdr0.ny = 1;
    fhdr0.grid_miny = 0;
    fhdr0.grid_dy = 1.0;

    fhdr0.proj_type = Mdvx::PROJ_TIME_HEIGHT;
    fhdr0.proj_rotation = 0;
    fhdr0.bad_data_value = missing;
    fhdr0.missing_data_value = missing;

    fhdr0.data_element_nbytes = sizeof(fl32);
    fhdr0.volume_size =
      (fhdr0.nx * fhdr0.ny * fhdr0.nz * fhdr0.data_element_nbytes);

    // create array

    fl32 *data = new fl32[fhdr0.nz * fhdr0.ny * fhdr0.nx];

    // fill data array

    for (size_t ii = 0; ii < vsects.size(); ii++) {
      const MdvxField *fld = vsects[ii]->_getRequestedField(ifield);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      fl32 localMiss = fhdr.missing_data_value;
      const fl32 *profile = (const fl32*) fld->getVol();
      for (int iz = 0; iz < fhdr0.nz; iz++) {
	int index = iz * vsects.size() + ii;
	if (profile[iz] == localMiss) {
	  data[index] = missing;
	} else {
	  data[index] = profile[iz];
	}
      }
    } // ii

    // create field for time-height profile

    MdvxField *thtField = new MdvxField(fhdr0, vhdr0, data);
    thtField->computeMinAndMax(true);

    // clean up
    
    delete[] data;

    // convert to output types

    if (thtField->convertType(mdvx._readEncodingType,
			      mdvx._readCompressionType,
			      mdvx._readScalingType,
			      mdvx._readScale,
			      mdvx._readBias)) {
      mdvx.addToErrStr("ERROR - _compileTimeHeight\n");
      delete thtField;
      return -1;
    }
    
    // add to object
    
    mdvx.addField(thtField);

  } // ifield

  // update the master header to match the field headers

  mdvx.updateMasterHeader();

  return 0;

}

