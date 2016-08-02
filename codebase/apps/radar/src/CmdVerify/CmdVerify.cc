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
// CmdVerify.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2015
//
///////////////////////////////////////////////////////////////
//
// Reads CMD data from CfRadial files, containing (a) weather only,
// (b) clutter only, (c) merged. Verifies the performance of CMD against
// the known truthiness. Also writes data out to ASCII file in column
// format, for analysis by other apps.
//
////////////////////////////////////////////////////////////////

#include "CmdVerify.hh"
#include <Radx/RadxVol.hh>
#include <Mdv/GenericRadxFile.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxRay.hh>
#include <dsserver/DsLdataInfo.hh>
#include <didss/DataFileNames.hh>
using namespace std;

// Constructor

CmdVerify::CmdVerify(int argc, char **argv)
  
{

  OK = TRUE;
  _nWarnCensorPrint = 0;
  _labelsPrinted = false;

  // set programe name

  _progName = "CmdVerify";
  
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

CmdVerify::~CmdVerify()

{

}

//////////////////////////////////////////////////
// Run

int CmdVerify::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    cerr << "ERROR - CmdVerify::Run()" << endl;
    cerr << "  Unknown mode: " << _params.mode << endl;
    return -1;
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int CmdVerify::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int CmdVerify::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  RadxTime startTime = RadxTime::parseDateTime(_params.start_time);
  RadxTime endTime = RadxTime::parseDateTime(_params.end_time);
  tlist.setModeInterval(startTime, endTime);
  if (tlist.compile()) {
    cerr << "ERROR - CmdVerify::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: "
         << _params.input_dir << endl;
    cerr << "  Start time: " << startTime.asString() << endl;
    cerr << "  End time: " << endTime.asString() << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - CmdVerify::_runFilelist()" << endl;
    cerr << "  No files found, dir: "
         << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int CmdVerify::_processFile(const string &path)
{

  if (_params.debug) {
    cerr << "INFO - CmdVerify::_processFile" << endl;
    cerr << "  Input path primary file: " << path << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  RadxVol vol;
  if (inFile.readFromPath(path, vol)) {
    cerr << "ERROR - RadxConvert::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  ==>> read in file: " << path << endl;
  }
  
  // convert to fl32 and constant number of gates
  
  vol.setNGatesConstant();
  vol.convertToFl32();

  // compute combined fields if needed

  if (_params.add_combined_fields) {
    if (_addCombinedFields(vol)) {
      cerr << "ERROR - CmdVerify::_processFile" << endl;
      cerr << "  Cannot combine fields" << endl;
      return -1;
    }
  }

  // censor as applicable

  if (_params.apply_censoring) {
    _censorFields(vol);
  }

  // print the volume to standard out


  if (_params.write_table_to_stdout) {
    _printVolume(vol);
  }

  return 0;

}

//////////////////////////////////////////////////
// set up read

void CmdVerify::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  if (_params.set_output_fields) {
    for (int ii = 0; ii < _params.output_field_names_n; ii++) {
      file.addReadField(_params._output_field_names[ii]);
    }
  }
  
  if (_params.apply_censoring) {
    for (int ii = 0; ii < _params.censoring_fields_n; ii++) {
      file.addReadField(_params._censoring_fields[ii].name);
    }
  }

  if (_params.set_max_range) {
    file.setReadMaxRangeKm(_params.max_range_km);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

//////////////////////////////////////////////////////////////
// Add combined fields
//
// Returns 0 on success, -1 on failure

int CmdVerify::_addCombinedFields(RadxVol &vol)
  
{
  
  int iret = 0;
  
  for (int ii = 0; ii < _params.combined_fields_n; ii++) {
    
    const Params::combined_field_t &comb = _params._combined_fields[ii];
    if (_addCombinedField(vol, comb)) {
      iret = -1;
    }
    
  } // ii
  
  return iret;
  
}

//////////////////////////////////////////////////////////////
// Add combined fields
//
// Returns 0 on success, -1 on failure

int CmdVerify::_addCombinedField(RadxVol &vol,
                                 const Params::combined_field_t &comb)
  
{

  int iret = 0;

  // loop through rays

  const vector<RadxRay *> &rays = vol.getRays();
  for (size_t iray = 0; iray < rays.size(); iray++) {

    // get the fields to be combined

    RadxRay *ray = rays[iray];
    RadxField *fld1 = ray->getField(comb.field_name_1);
    RadxField *fld2 = ray->getField(comb.field_name_2);
    if (fld1 == NULL || fld2 == NULL) {
      iret = -1;
      continue;
    }

    // copy the fields so we can convert to floats
    
    RadxField *copy1 = new RadxField(*fld1);
    RadxField *copy2 = new RadxField(*fld2);
    copy1->convertToFl32();
    copy2->convertToFl32();
    Radx::fl32 miss1 = copy1->getMissingFl32();
    Radx::fl32 miss2 = copy2->getMissingFl32();

    // create new field to hold combined data
    // base it on field 1

    RadxField *combf = new RadxField(*copy1);
    combf->setName(comb.combined_name);
    combf->setLongName(comb.long_name);
    Radx::fl32 missComb = combf->getMissingFl32();
    
    // compute bias if needed

    double meanBias = 0.0;
    double sumBias = 0.0;
    double nBias = 0.0;
    if(comb.combine_method == Params::COMBINE_UNBIASED_MEAN) {
      Radx::fl32 *vals1 = (Radx::fl32 *) copy1->getData();
      Radx::fl32 *vals2 = (Radx::fl32 *) copy2->getData();
      for (size_t ipt = 0; ipt < copy1->getNPoints();
           ipt++, vals1++, vals2++) {
        if (*vals1 != miss1 && *vals2 != miss2) {
          double bias = *vals1 - *vals2;
          sumBias += bias;
          nBias++;
        }
      }
      if (nBias > 0) {
        meanBias = sumBias / nBias;
      }
    }

    // compute combined values
    
    Radx::fl32 *vals1 = (Radx::fl32 *) copy1->getData();
    Radx::fl32 *vals2 = (Radx::fl32 *) copy2->getData();
    Radx::fl32 *valsComb = (Radx::fl32 *) combf->getData();
    
    bool requireBoth = comb.require_both;
    for (size_t ipt = 0; ipt < copy1->getNPoints();
         ipt++, vals1++, vals2++, valsComb++) {

      // check if we need both fields present

      *valsComb = missComb;
      if (requireBoth) {
        if (*vals1 == miss1 || *vals2 == miss2) {
          continue;
        }
      }
        
      // field 1 missing?

      if (*vals1 == miss1) {
        // only use field 2
        if (comb.combine_method == Params::COMBINE_UNBIASED_MEAN) {
          // adjust for bias
          *valsComb = *vals2 + meanBias;
        } else {
          *valsComb = *vals2;
        }
        continue;
      }
      
      // field 2 missing?

      if (*vals2 == miss2) {
        // only use field1
        *valsComb = *vals1;
        continue;
      }

      // combine fields
      
      if (comb.combine_method == Params::COMBINE_MEAN) {

        *valsComb = (*vals1 + *vals2) / 2.0;
        
      } else if (comb.combine_method == Params::COMBINE_UNBIASED_MEAN) {

        *valsComb = (*vals1 + *vals2 + meanBias) / 2.0;
        
      } else if (comb.combine_method == Params::COMBINE_GEOM_MEAN) {

        *valsComb = sqrt(*vals1 * *vals2);

      } else if (comb.combine_method == Params::COMBINE_MAX) {

        if (*vals1 > *vals2) {
          *valsComb = *vals1;
        } else {
          *valsComb = *vals2;
        }

      } else if (comb.combine_method == Params::COMBINE_MIN) {

        if (*vals1 < *vals2) {
          *valsComb = *vals1;
        } else {
          *valsComb = *vals2;
        }

      } else if (comb.combine_method == Params::COMBINE_SUM) {

        *valsComb = *vals1 + *vals2;
        
      } else if (comb.combine_method == Params::COMBINE_DIFF) {

        *valsComb = *vals1 - *vals2;
        
      } // if (comb.combine_method == Params::COMBINE_MEAN)

    } // ipt

    // add combined field to ray
    
    combf->convertToFl32();
    combf->computeMinAndMax();
    ray->addField(combf);

    // free up

    delete copy1;
    delete copy2;

  } // iray

  return iret;

}

////////////////////////////////////////////////////////////////////
// censor fields in vol

void CmdVerify::_censorFields(RadxVol &vol)

{

  vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _censorRay(rays[ii]);
  }
  
}

////////////////////////////////////////////////////////////////////
// censor fields in a ray

void CmdVerify::_censorRay(RadxRay *ray)

{

  if (!_params.apply_censoring) {
    return;
  }

  // convert fields to floats

  vector<Radx::DataType_t> fieldTypes;
  vector<RadxField *> fields = ray->getFields();
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    Radx::DataType_t dtype = field->getDataType();
    fieldTypes.push_back(dtype);
    field->convertToFl32();
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

  // apply censoring by setting censored gates to missing for all fields

  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    RadxField *field = fields[ifield];
    Radx::fl32 *fdata = (Radx::fl32 *) field->getData();
    for (size_t igate = 0; igate < nGates; igate++) {
      if (censorFlag[igate] == 1) {
        fdata[igate] = Radx::missingFl32;
      }
    } // igate
  } // ifield

  // convert back to original types
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    RadxField *field = fields[ii];
    field->convertToType(fieldTypes[ii]);
  }

}

////////////////////////////////////////////////////////////////////
// print volume data to stdout

void CmdVerify::_printVolume(RadxVol &vol)
  
{
  
  vector<RadxRay *> &rays = vol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    _printRay(*rays[ii]);
  }
  
}

////////////////////////////////////////////////////////////////////
// print ray data to stdout

void CmdVerify::_printRay(RadxRay &ray)
  
{

  RadxTime rayTime(ray.getRadxTime());
  int msecs = (int) (rayTime.getSubSec() * 1000.0 + 0.5);
  if (msecs >= 1000) {
    rayTime += 1.0;
    msecs -= 1000;
  }
  double az = ray.getAzimuthDeg();
  double el = ray.getElevationDeg();
  vector<RadxField *> fields = ray.getFields();
  size_t nGates = ray.getNGates();

  string delim = _params.delimiter_string;
  string commentStart = _params.comment_start_string;
  
  if (!_labelsPrinted) {
    
    // print labels header

    vector<string> labels;
    labels.push_back("year");
    labels.push_back("month");
    labels.push_back("day");
    labels.push_back("hour");
    labels.push_back("min");
    labels.push_back("sec");
    labels.push_back("msecs");
    labels.push_back("el");
    labels.push_back("az");
    labels.push_back("range");
    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      labels.push_back(fields[ifield]->getName());
    } // ifield
    for (size_t ii = 0; ii <labels.size(); ii++) {
      if (ii == 0) {
        cout << commentStart;
      } else {
        cout << delim;
      }
      cout << labels[ii];
      if (_params.add_col_num_to_col_labels) {
        cout << "(" << ii << ")";
      }
    } // ii
    cout << endl;
    
    _labelsPrinted = true;

  }

  // print data for each gate

  double range = ray.getStartRangeKm();

  for (size_t igate = 0; igate < nGates; igate++) {
    
    bool allMissing = _checkAllFieldsMissing(fields, igate);
    // bool anyMissing = _checkAnyFieldsMissing(fields, igate);
    
    if (!allMissing) {

      cout << rayTime.getYear();
      cout << delim << rayTime.getMonth();
      cout << delim << rayTime.getDay();
      cout << delim << rayTime.getHour();
      cout << delim << rayTime.getMin();
      cout << delim << rayTime.getSec();
      cout << delim << msecs;
      cout << delim << el;
      cout << delim << az;
      cout << delim << range;
      
      for (size_t ifield = 0; ifield < fields.size(); ifield++) {
        const RadxField *field = fields[ifield];
        const Radx::fl32 *data = field->getDataFl32();
        fl32 val = data[igate];
        if (val == field->getMissingFl32()) {
          cout << delim << _params.missing_data_string;
        } else {
          cout << delim << val;
        }
      } // ifield
      
      cout << endl;

    }

    range += ray.getGateSpacingKm();

  } // igate

}

////////////////////////////////////////////////////////////////////
// Check if all data is missing at a gate

bool CmdVerify::_checkAllFieldsMissing(vector<RadxField *> &fields,
                                       size_t gateNum)
  
{

  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    const RadxField *field = fields[ifield];
    const Radx::fl32 *data = field->getDataFl32();
    if (data[gateNum] != field->getMissingFl32()) {
      return false;
    }
  } // ifield

  return true;

}

////////////////////////////////////////////////////////////////////
// Check if any data is missing at a gate

bool CmdVerify::_checkAnyFieldsMissing(vector<RadxField *> &fields,
                                       size_t gateNum)
  
{

  for (size_t ifield = 0; ifield < fields.size(); ifield++) {
    const RadxField *field = fields[ifield];
    const Radx::fl32 *data = field->getDataFl32();
    if (data[gateNum] == field->getMissingFl32()) {
      return true;
    }
  } // ifield

  return false;

}

      

