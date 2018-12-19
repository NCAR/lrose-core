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
// Mdv2Surfer.cc
//
// Mdv2Surfer object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#include "Mdv2Surfer.hh"
#include <didss/DsInputPath.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <cerrno>
using namespace std;

// Constructor

Mdv2Surfer::Mdv2Surfer(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name
  
  _progName = "Mdv2Surfer";
  
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

  // check params
  
  if (_args.inputFileList.size() < 1 &&
      _args.startTime == 0 && _args.endTime == 0) {
    cerr << "ERROR - " << _progName << endl;
    cerr << "  You must either specify start and end times, "
	 << "or use the -f option." << endl;
    OK = FALSE;
    return;
  }
    
 // file input object
  
  if (_args.inputFileList.size() > 0) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  }
  
  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

Mdv2Surfer::~Mdv2Surfer()

{


}

//////////////////////////////////////////////////
// Run

int Mdv2Surfer::Run()
{

  int iret = 0;

  char *input_file_path;

  while ((input_file_path = _input->next()) != NULL) {

    if (_params.debug) {
      cerr << "Processing file: " << input_file_path << endl;
    }

    // set up Mdvx object
    
    Mdvx mdvx;
    if (_params.set_field_names) {
      for (int i = 0; i < _params.field_names_n; i++) {
	mdvx.addReadField(_params._field_names[i]);
      }
    }
    mdvx.setReadPath(input_file_path);
    mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      mdvx.setDebug();
      mdvx.printReadRequest(cerr);
    }
    
    if (mdvx.readVolume()) {
      cerr << mdvx.getErrStr();
      iret = -1;
      continue;
    }
    
    // master header
    
    const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  
    // fields
    
    for (int i = 0; i < mhdr.n_fields; i++) {
      
      // get field
      
      MdvxField *field = mdvx.getFieldByNum(i);

      // make sure it is linear
      field->transform2Linear();
      
      // write out

      if (_writeSurferFile(mhdr, *field)) {
	iret = -1;
	continue;
      }

    }
    
  }

  return iret;

}

////////////////////////
// write out surfer file

int Mdv2Surfer::_writeSurferFile(const Mdvx::master_header_t &mhdr,
				 const MdvxField &field)

{

  const Mdvx::field_header_t &fhdr = field.getFieldHeader();

  // make output dir if needed

  if (ta_makedir_recurse(_params.output_dir)) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << "::_writeSurferFile" << endl;
    cerr << "  Cannot create output dir: " << _params.output_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  string outPath = _params.output_dir;
  outPath += PATH_DELIM;
  outPath += _params.output_file_label;
  outPath += "-";
  outPath += fhdr.field_name;
  outPath += "-";

  char centStr[128];
  DateTime start(mhdr.time_centroid);
  sprintf(centStr, "%.4d%.2d%.2d_%.2d%.2d",
	  start.getYear(), start.getMonth(), start.getDay(),
	  start.getHour(), start.getMin());

  outPath += centStr;

  outPath += ".";
  outPath += _params.output_file_ext;

  if (_params.debug) {
    cerr << "Writing surfer file: outPath: " << outPath << endl;
  }

  FILE *out;
  if ((out = fopen(outPath.c_str(), "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - " << _progName << "::_writeSurferFile" << endl;
    cerr << "  Cannot open output file for writing: " << endl;
    cerr << "  " << outPath << ": " << strerror(errNum) << endl;
    return -1;
  }

  fprintf(out, "DSAA\r\n");
  fprintf(out, "%6d%6d\r\n", fhdr.nx, fhdr.ny);
  fprintf(out, "%12.6f%12.6f\r\n",
	  fhdr.grid_minx + fhdr.grid_dx / 2.0,
	  fhdr.grid_minx + (fhdr.nx) * fhdr.grid_dx - fhdr.grid_dx / 2.0);
  fprintf(out, "%12.6f%12.6f\r\n",
	  fhdr.grid_miny + fhdr.grid_dy / 2.0,
	  fhdr.grid_miny + (fhdr.ny) * fhdr.grid_dy - fhdr.grid_dy / 2.0);
  fprintf(out, "%7.2f%7.2f\r\n", fhdr.min_value, fhdr.max_value);
  
  fl32 missing = fhdr.missing_data_value;
  fl32 *ff = (fl32 *) field.getVol();
  for (int iy = 0; iy < fhdr.ny; iy++) {
    for (int ix = 0; ix < fhdr.nx; ix++, ff++) {
      if (*ff == missing) {
	fprintf(out, " %7.2f", _params.missing_output_val);
      } else {
	fprintf(out, " %7.2f", *ff);
      }
    } // ix
    fprintf(out, "\r\n");
  } // iy


  fclose(out);

  return 0;

}

