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
// EchoTops.cc
//
// EchoTops object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2006
//
///////////////////////////////////////////////////////////////
//
// EchoTops computes echo tops in Cartesian radar data
//
///////////////////////////////////////////////////////////////

#include <vector>
#include <list>

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MemBuf.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/DsMdvxInput.hh>
#include "EchoTops.hh"
using namespace std;

// Constructor

EchoTops::EchoTops(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "EchoTops";
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

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode, must specify start and end dates."
	   << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  if (_params.tops_fields_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Must specify at least one tops field." << endl << endl;
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_url, 600,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.input_url,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  return;

}

// destructor

EchoTops::~EchoTops()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int EchoTops::Run()
{
  
  // create unique list of fields to read
  
  set<string, less<string> > readFields;
  for (int i = 0; i < _params.tops_fields_n; i++) {
    readFields.insert(readFields.begin(), _params._tops_fields[i].input_field);
  }
  
  // register with procmap
  
  PMU_auto_register("Run");
  
  // loop until end of data
  
  _input.reset();
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // create input DsMdvx object
    
    DsMdvx inMdvx;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      inMdvx.setDebug(true);
    }
    
    // specify the fields to be read in
    
    for (set<string, less<string> >::iterator ii = readFields.begin();
         ii != readFields.end(); ii++) {
      inMdvx.addReadField(*ii);
    }
    
    // vert levels
    
    if (_params.set_vlevel_limits) {
      inMdvx.setReadVlevelLimits(_params.lower_vlevel, _params.upper_vlevel);
    }

    inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      inMdvx.printReadRequest(cerr);
    }
    
    // read in
    
    PMU_auto_register("Before read");
    
    if (_input.readVolumeNext(inMdvx) == 0) {
      if (_params.debug) {
        cerr << "Reading file: " << inMdvx.getPathInUse() << endl;
      }
      if (_processFile(inMdvx)) {
        cerr << "ERROR - EchoTops::Run" << endl;
        cerr << "  Cannot process file" << endl;
        return -1;
      }
    } else {
      cerr << "ERROR - EchoTops::Run" << endl;
      cerr << _input.getErrStr() << endl;
      return -1;
    }

    
  } // while

  return 0;

}
    
/////////////////////////////////////////////////////////
// Process the file
//
// Returns 0 on success, -1 on failure.

int EchoTops::_processFile(DsMdvx &inMdvx)
  
{
  
  PMU_auto_register("Before write");
  
  // create output DsMdvx object
  
  DsMdvx outMdvx;
  if (_params.debug) {
    outMdvx.setDebug(true);
  }
  
  outMdvx.setAppName(_progName);
  outMdvx.setWriteLdataInfo();
  Mdvx::master_header_t mhdr = inMdvx.getMasterHeader();
  outMdvx.setMasterHeader(mhdr);
  string info = inMdvx.getMasterHeader().data_set_info;
  info += "\nTops computed using EchoTops";
  outMdvx.setDataSetInfo(info.c_str());
  
  // compute the tops, adding the fields to the output object
  
  outMdvx.clearFields();
  for (int ii = 0; ii < _params.tops_fields_n; ii++) {

    char *inFieldName = _params._tops_fields[ii].input_field;
    MdvxField *inFld = inMdvx.getField(inFieldName);
    if (inFld == NULL) {
      cerr << "ERROR - _processFile" << endl;
      cerr << "  Cannot find field: " << inFieldName << endl;
      cerr << "  File: " << inMdvx.getPathInUse() << endl;
      return -1;
    }

    const char *outFieldName = _params._tops_fields[ii].output_field;
    const char *outUnits = _params._tops_fields[ii].output_units;
    double threshold = _params._tops_fields[ii].threshold;
    bool compute_base = _params._tops_fields[ii].compute_base;
    bool invert_values = _params._tops_fields[ii].invert_values;

    MdvxField *outFld = _computeTops(*inFld, outFieldName, outUnits,
                                     threshold, compute_base, invert_values);
    
    outMdvx.addField(outFld);

  } // ii


  PMU_auto_register("Before write");
  
  // set output compression

  for (size_t i = 0; i < outMdvx.getNFields(); i++) {
    MdvxField *field = outMdvx.getFieldByNum(i);
    field->requestCompression(Mdvx::COMPRESSION_GZIP);
  }
  
  // add any chunks
  
  outMdvx.clearChunks();
  for (size_t i = 0; i < inMdvx.getNChunks(); i++) {
    MdvxChunk *chunk = new MdvxChunk(*inMdvx.getChunkByNum(i));
    outMdvx.addChunk(chunk);
  }
  
  // write out
  
  if(outMdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - EchoTops::processFile" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << outMdvx.getErrStr() << endl;
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////
// compute the full composite for all the fields read in

MdvxField *EchoTops::_computeTops(const MdvxField &inFld,
                                  const char *outFieldName,
                                  const char *&outUnits,
                                  double threshold,
                                  bool computeBase,
                                  bool invertValues)
  
{

  const Mdvx::field_header_t &inFhdr = inFld.getFieldHeader();
  const Mdvx::vlevel_header_t &inVhdr = inFld.getVlevelHeader();
  fl32 inMissing = inFhdr.missing_data_value;
  fl32 outMissing = -9999;

  // compute tops data

  int nptsPlane = inFhdr.nx * inFhdr.ny;
  fl32 *tops = (fl32 *) umalloc(nptsPlane * sizeof(fl32));
  fl32 *tt = tops;
  const fl32 *dd = (fl32 *) inFld.getVol();
  
  for (int iy = 0; iy < inFhdr.ny; iy++) {
    for (int ix = 0; ix < inFhdr.nx; ix++, tt++, dd++) {
      *tt = outMissing;
      for (int iz = 0; iz < inFhdr.nz; iz++) {
        int jz = iz;
        if (!computeBase) {
          jz = inFhdr.nz - 1 - iz;
        }
        fl32 inVal = *(dd + jz * nptsPlane);
        if (inVal != inMissing) {
          if (invertValues) {
            inVal *= -1.0;
          }
          if (inVal >= threshold) {
            *tt = inVhdr.level[jz];
            break;
          }
        }
      } // iz
    } // ix
  } // iy
  
  Mdvx::field_header_t outFhdr = inFld.getFieldHeader();
  outFhdr.nz = 1;
  outFhdr.grid_dz = 1;
  outFhdr.grid_minz = 0;
  outFhdr.dz_constant = true;
  outFhdr.vlevel_type = Mdvx::VERT_TYPE_TOPS;
  outFhdr.missing_data_value = outMissing;
  outFhdr.bad_data_value = outMissing;
  outFhdr.compression_type = Mdvx::COMPRESSION_NONE;
  outFhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  outFhdr.volume_size = nptsPlane * sizeof(fl32);
  
  Mdvx::vlevel_header_t outVhdr;
  MEM_zero(outVhdr);
  outVhdr.type[0] = Mdvx::VERT_TYPE_TOPS;
  
  MdvxField *outFld = new MdvxField(outFhdr, outVhdr, tops);
  ufree(tops);

  outFld->setFieldName(outFieldName);
  outFld->setFieldNameLong(outFieldName);
  outFld->setUnits(outUnits);
  outFld->setTransform("");

  return outFld;

}
  
