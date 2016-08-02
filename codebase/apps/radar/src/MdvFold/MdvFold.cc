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
// MdvFold.cc
//
// MdvFold object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
///////////////////////////////////////////////////////////////
//
//
// MdvFold is intended to help simulate folded data.
// It takes MDV data which is not folded and artificially
// folds the data.
// The folded data can then be used to test algorithms.
//
////////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include "MdvFold.hh"
using namespace std;

const double MdvFold::missingDouble = -9999.0;
const fl32 MdvFold::missingFloat = -9999.0;

// Constructor

MdvFold::MdvFold(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvFold";
  ucopyright((char *) _progName.c_str());

  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
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
  
  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_dir, 600,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.input_dir,
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

MdvFold::~MdvFold()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvFold::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // loop until end of data
  
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // read the next MDV volume
    
    DsMdvx mdvx;
    if (_readNextVolume(mdvx)) {
      continue;
    }
    
    // create the folded velocity field
    // add to the mdvx object

    if (_params.apply_velocity_folding) {
      if (_addFoldedVelocity(mdvx)) {
        cerr << "ERROR - MdvFold::Run()" << endl;
        cerr << "  Cannot add folded velocity" << endl;
        continue;
      }
    }
    
    // write output

    if (_writeOutput(mdvx)) {
      cerr << "ERROR - MdvFold::Run()" << endl;
      cerr << "  Cannot write output file" << endl;
    }
    
  } // while

  return 0;

}

///////////////////////////////////////////
// set up the read for the next MDV file

void MdvFold::_setupRead(DsMdvx &inMdvx)
  
{

  inMdvx.setDebug(_params.debug);
  inMdvx.clearRead();
  
  inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  bool velFieldAdded = false;
  for (int ii = 0; ii < _params.input_field_names_n; ii++) {
    inMdvx.addReadField(_params._input_field_names[ii]);
    if (!strcmp(_params._input_field_names[ii],
                _params.VEL_field_name)) {
      velFieldAdded = true;
    }
  }
  if (!velFieldAdded) {
    inMdvx.addReadField(_params.VEL_field_name);
  }
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading input MDV file" << endl;
    inMdvx.printReadRequest(cerr);
  }

}

///////////////////////////
// read in next volume file

int MdvFold::_readNextVolume(DsMdvx &inMdvx)
  
{

  PMU_auto_register("Before read");

  // set up read
  
  _setupRead(inMdvx);

  // read the volume
  
  if (_input.readVolumeNext(inMdvx)) {
    cerr << "ERROR - MdvFold::_readInput" << endl;
    cerr << "  Cannot read in MDV data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Working on file: " << inMdvx.getPathInUse() << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Input file fields:" << endl;
    for (int ifield = 0;
	 ifield < inMdvx.getMasterHeader().n_fields; ifield++) {
      MdvxField *fld = inMdvx.getField(ifield);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      cerr << "Field " << ifield << ", name " << fhdr.field_name << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////
// add folded velocity to MDV object
// returns 0 on success, -1 on failure

int MdvFold::_addFoldedVelocity(DsMdvx &mdvx)
  
{

  // locate velocity field

  const MdvxField *velFld = mdvx.getField(_params.VEL_field_name);
  if (velFld == NULL) {
    cerr << "ERROR - MdvFold::_setInputPointers()" << endl;
    cerr << "  Cannot find VEL field, name: "
         << _params.VEL_field_name << endl;
    return -1;
  }

  // copy velocity field

  MdvxField *foldedFld = new MdvxField(*velFld);

  // fold the data in the field

  const Mdvx::field_header_t &fhdr = foldedFld->getFieldHeader();
  int npts = fhdr.nx * fhdr.ny * fhdr.nz;
  fl32 miss = fhdr.missing_data_value;
  fl32 *velArray = (fl32 *) foldedFld->getVol();

  for (int ii = 0; ii < npts; ii++) {

    fl32 vel = velArray[ii];
    if (vel == miss) {
      continue;
    }

    double angRad = (vel / _params.folding_nyquist) * M_PI;
    double v = sin(angRad);
    double u = cos(angRad);
    if (u == 0 && v == 0) {
      angRad = 0;
    } else {
      angRad = atan2(v, u);
    }
    double foldedVel = (angRad / M_PI) * _params.folding_nyquist;

    velArray[ii] = (fl32) foldedVel;

  }

  // set the field name

  foldedFld->setFieldName(_params.VEL_folded_field_name);
  foldedFld->setFieldNameLong(_params.VEL_folded_field_name);

  // add to the mdvx object

  mdvx.addField(foldedFld);

  // add to info

  char text[1024];
  sprintf(text, "\nAdding folded velocity field, nyquist: %g\n",
          _params.folding_nyquist);
  string info = mdvx.getMasterHeader().data_set_info;
  info += text;
  mdvx.setDataSetInfo(info.c_str());

  return 0;

}
  
///////////////////////////
// write Output file

int MdvFold::_writeOutput(DsMdvx &outMdvx)
  
{

  PMU_auto_register("Before write");
  outMdvx.setWriteLdataInfo();
  outMdvx.setAppName(_progName);
  if(outMdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - MdvFold::_writeOutput" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << outMdvx.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Wrote file: " << outMdvx.getPathInUse() << endl;
  }

  return 0;

}


