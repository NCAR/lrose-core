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
// MdvRemapRadarGeom.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2011
//
///////////////////////////////////////////////////////////////
//
// MdvRemapRadarGeom remaps the beam geometry of
// polar radar data in an MDV file
//
///////////////////////////////////////////////////////////////

#include "MdvRemapRadarGeom.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
using namespace std;

// Constructor

MdvRemapRadarGeom::MdvRemapRadarGeom(int argc, char **argv)

{

  isOK = true;
  
  // set programe name

  _progName = "MdvRemapRadarGeom";
  ucopyright(_progName.c_str());

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

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }
  
  if (!isOK) {
    return;
  }

  // input file object

  if (_params.mode == Params::FILELIST) {

    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In FILELIST mode you must specify the files using -f arg." << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }

  } else if (_params.mode == Params::ARCHIVE) {
      
    if (_args.startTime != 0 && _args.endTime != 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _params.input_dir,
			       _args.startTime,
			       _args.endTime);
    } else {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode you must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
      
  } else {

    // realtime mode
    
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
                             _params.latest_data_info_avail);
    
  } // if (_params.mode == Params::FILELIST)

  // auto register with procmap
  
  PMU_auto_init(_progName.c_str(), _params.instance, PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

MdvRemapRadarGeom::~MdvRemapRadarGeom()

{

  // unregister process

  PMU_auto_unregister();
  
  // free up
  
  delete _input;

}

//////////////////////////////////////////////////
// Run

int MdvRemapRadarGeom::Run ()
{

  PMU_auto_register("MdvRemapRadarGeom::Run");

  int iret = 0;
  const char *filePath;
  while ((filePath = _input->next()) != NULL) {
    if (_processFile(filePath)) {
      iret = -1;
    }
  }

  return iret;

}

////////////////////////////
// Process the file
//
// Returns 0 on success, -1 on failure

int MdvRemapRadarGeom::_processFile(const char *filePath)

{

  if (_params.debug) {
    fprintf(stderr, "Processing file: %s\n", filePath);
  }

  PMU_auto_register("Processing file");

  // set up MDV object for reading
  
  DsMdvx mdvxIn;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvxIn.setDebug(true);
  }
  _setupRead(mdvxIn);
  mdvxIn.setReadPath(filePath);

  if (mdvxIn.readVolume()) {
    cerr << "ERROR - MdvRemapRadarGeom::_processFile" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << mdvxIn.getErrStr() << endl;
    return -1;
  }
  
  // create output object
  
  DsMdvx mdvxOut;

  // set master header
  
  Mdvx::master_header_t mhdr = mdvxIn.getMasterHeader();
  mhdr.time_gen = time(NULL);
  mdvxOut.setMasterHeader(mhdr);
  mdvxOut.setDataSetSource(filePath);
  string info = mhdr.data_set_info;
  info += "\nBeam geom remapping by MdvRemapRadarGeom\n";
  mdvxOut.setDataSetInfo(info.c_str());

  // remap the fields

  for (int ii = 0; ii < _params.fields_n; ii++) {
    MdvxField *inField = mdvxIn.getFieldByName(_params._fields[ii].input_name);
    Mdvx::projection_type_t projType =
      (Mdvx::projection_type_t) inField->getFieldHeader().proj_type;
    if (projType != Mdvx::PROJ_POLAR_RADAR) {
      cerr << "ERROR - MdvRemapRadarGeom::_processFile" << endl;
      cerr << "  Incorrect projection type: "
           << Mdvx::projType2Str(projType) << endl;
      return -1;
    }
    MdvxField *outField =
      _remapField(*inField,
                  _params._fields[ii].output_name,
                  _params._fields[ii].comb_method);
    mdvxOut.addField(outField);
  }
  
  // write out

  if (_params.debug) {
    cerr << "Writing out ..." << endl;
  }
  
  if(mdvxOut.writeToDir(_params.output_url)) {
    cerr << "ERROR - MdvRemapRadarGeom::_processFile" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << mdvxOut.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote file: " << mdvxOut.getPathInUse() << endl;
  }

  return 0;

}

////////////////////////////////////////////
// set up the read

void MdvRemapRadarGeom::_setupRead(DsMdvx &mdvx)

{
  
  if (_params.debug) {
    mdvx.setDebug(true);
  }

  // set up the Mdvx read
  
  mdvx.clearRead();
  
  // get file headers, to save encoding and compression info
  
  mdvx.setReadFieldFileHeaders();

  // set the field list

  for (int ii = 0; ii < _params.fields_n; ii++) {
    mdvx.addReadField(_params._fields[ii].input_name);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

}


////////////////////////////////////////////
// handle a field

MdvxField *MdvRemapRadarGeom::_remapField(MdvxField &inField,
                                          const char *outputName,
                                          Params::comb_method_t combMethod)
  
{

  // copy headers

  Mdvx::field_header_t fhdr = inField.getFieldHeader();
  Mdvx::vlevel_header_t vhdr = inField.getVlevelHeader();
  
  // save input encoding and compression
  
  Mdvx::encoding_type_t inputEncoding =
    (Mdvx::encoding_type_t) fhdr.encoding_type;
  Mdvx::compression_type_t inputCompression =
    (Mdvx::compression_type_t) fhdr.compression_type;

  // convert field to floats

  inField.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  fhdr = inField.getFieldHeader();

  // compute remapping lookup table

  int inNgates = fhdr.nx;
  int nPointsTiltIn = inNgates * fhdr.ny;
  int outNgates = _params.remap_radar_n_gates;
  int nPointsTiltOut = outNgates * fhdr.ny;
  int nPointsVolOut = nPointsTiltOut * fhdr.nz;
  _computeRemapLut(fhdr, inNgates, outNgates);

  // loop through tilts

  fl32 *inVol = (fl32 *) inField.getVol();
  
  fl32 *outVol = new fl32[nPointsVolOut];

  for (int itilt = 0; itilt < fhdr.nz; itilt++) {
    
    fl32 *inBeam = inVol + itilt * nPointsTiltIn;
    fl32 *outBeam = outVol + itilt * nPointsTiltOut;
    
    for (int iaz = 0; iaz < fhdr.ny; iaz++) {
      _remapBeam(fhdr, inNgates, outNgates, inBeam, outBeam, combMethod);
      inBeam += inNgates;
      outBeam += outNgates;
    }
    

  } // itilt

  // modify field header

  fhdr.nx = outNgates;
  fhdr.grid_dx = _params.remap_radar_gate_spacing;
  fhdr.grid_minx = _params.remap_radar_start_range;

  // convert type on the way out

  fhdr.volume_size = nPointsVolOut * sizeof(fl32);
  MdvxField *outField = new MdvxField(fhdr, vhdr, outVol);
  delete[] outVol;
  outField->convertType(inputEncoding, inputCompression);
  outField->setFieldName(outputName);
  return outField;
  
}

////////////////////////////////////////////////
// remap a beam for a single field

void MdvRemapRadarGeom::_remapBeam(Mdvx::field_header_t &fhdr,
                                   int inNgates, int outNgates,
                                   fl32 *inBeam, fl32 *outBeam,
                                   Params::comb_method_t combMethod)

{

  fl32 missing = fhdr.missing_data_value;
  
  if (combMethod == Params::COMB_CLOSEST) {
    
    for (size_t ii = 0; ii < _lut.size(); ii++) {
      const LutEntry &entry = _lut[ii];
      int jj = entry.midIndex;
      if (jj < 0) {
        outBeam[ii] = missing;
      } else {
        fl32 val = inBeam[jj];
        if (val == missing) {
          outBeam[ii] = missing;
        } else {
          outBeam[ii] = val;
        }
      }
    } // ii

  } else if (combMethod == Params::COMB_MEAN) {

    for (size_t ii = 0; ii < _lut.size(); ii++) {
      const LutEntry &entry = _lut[ii];
      int startIndex = entry.startIndex;
      int endIndex = entry.endIndex;
      if (startIndex < 0 || endIndex < 0) {
        outBeam[ii] = missing;
      } else {
        int count = 0;
        double sum = 0.0;
        for (int jj = startIndex; jj <= endIndex; jj++) {
          fl32 val = inBeam[jj];
          if (val != missing) {
            sum += val;
            count++;
          }
      } // jj
        if (count == 0) {
          outBeam[ii] = missing;
        } else {
          outBeam[ii] = sum / count;
        }
      }
    } // ii

  } else if (combMethod == Params::COMB_DB_MEAN) {

    for (size_t ii = 0; ii < _lut.size(); ii++) {
      const LutEntry &entry = _lut[ii];
      int startIndex = entry.startIndex;
      int endIndex = entry.endIndex;
      if (startIndex < 0 || endIndex < 0) {
        outBeam[ii] = missing;
      } else {
        int count = 0;
        double sum = 0.0;
        for (int jj = startIndex; jj <= endIndex; jj++) {
          fl32 val = inBeam[jj];
          if (val != missing) {
            sum += pow(10.0, val / 10.0);
            count++;
          }
        } // jj
        if (count == 0) {
          outBeam[ii] = missing;
        } else {
          outBeam[ii] = log10((sum / count)) * 10.0;
        }
      } // ii
    }

  }
  
}

////////////////////////////////////////////////
// compute the remapping lut

void MdvRemapRadarGeom::_computeRemapLut(Mdvx::field_header_t &fhdr,
                                         int inNgates,
                                         int outNgates)

{

  double startRangeIn = fhdr.grid_minx;
  double gateSpacingIn = fhdr.grid_dx;
  double startRangeOut = _params.remap_radar_start_range;
  double gateSpacingOut = _params.remap_radar_gate_spacing;

  double rangeOut = startRangeOut;

  _lut.clear();
  
  for (int igate = 0; igate < outNgates; igate++) {
    
    double rangeMin = (rangeOut - gateSpacingOut / 2.0) * 1.0001;
    double rangeMax = (rangeOut + gateSpacingOut / 2.0) * 0.9999;
    
    int indexMin = (int) floor(((rangeMin - startRangeIn) / gateSpacingIn) + 1.0);
    int indexMax = (int) floor(((rangeMax - startRangeIn) / gateSpacingIn) + 0.0);
    int indexMid = (indexMin + indexMax) / 2;
    int nGates = indexMax - indexMin + 1;

    if (indexMin < 0 || indexMin >= inNgates ||
        indexMax < 0 || indexMax >= inNgates) {
      indexMin = -1;
      indexMax = -1;
      indexMid = -1;
    }

    LutEntry entry(indexMin, indexMax, indexMid, nGates);
    _lut.push_back(entry);

    rangeMin += gateSpacingOut;
    rangeMax += gateSpacingOut;
    rangeOut += gateSpacingOut;
    
  } // igate

}

  

