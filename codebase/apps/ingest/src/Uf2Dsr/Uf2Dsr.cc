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
// Uf2Dsr.cc
//
// Uf2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
///////////////////////////////////////////////////////////////
//
// Uf2Dsr reads netCDF radar beam-by-beam files and copies
// the contents into a DsRadar FMQ.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <toolsa/TaArray.hh>
#include <dataport/bigend.h>
#include "Uf2Dsr.hh"
using namespace std;

// Constructor

Uf2Dsr::Uf2Dsr(int argc, char **argv)

{

  _input = NULL;
  isOK = true;

  // set programe name

  _progName = "Uf2Dsr";
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

  // check that file list set in archive and simulate mode
  
  if (_params.mode == Params::ARCHIVE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: Uf2Dsr::Uf2Dsr." << endl;
    cerr << "  Mode is ARCHIVE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  if (_params.mode == Params::SIMULATE && _args.inputFileList.size() == 0) {
    cerr << "ERROR: Uf2Dsr::Uf2Dsr." << endl;
    cerr << "  Mode is SIMULATE."; 
    cerr << "  You must use -f to specify files on the command line."
	 << endl;
    _args.usage(_progName, cerr);
    isOK = false;
  }
    
  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // initialize the data input object
  
  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_ldata_info_file);
  } else {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // initialize the output queue

  if (_rQueue.init(_params.output_fmq_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_fmq_compress,
		   _params.output_fmq_nslots,
		   _params.output_fmq_size)) {
    cerr << "ERROR - Uf2Dsr" << endl;
    cerr << "  Cannot open fmq, URL: " << _params.output_fmq_url << endl;
    isOK = false;
    return;
  }

  if (_params.output_fmq_compress) {
    _rQueue.setCompressionMethod(TA_COMPRESSION_ZLIB);
  }

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }

  if (_params.override_decode_errors_on_read) {
    cerr << "WARNING - Uf2Dsr" << endl;
    cerr << "  override_decode_errors_on_read is set to TRUE" << endl;
    cerr << "  Decoding errors will be overridden." << endl;
    cerr << "  If the application crashes, set parameter this to false" << endl;
  }

  return;

}

// destructor

Uf2Dsr::~Uf2Dsr()

{

  if (_input) {
    delete _input;
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Uf2Dsr::Run ()
{

  int iret = 0;
  PMU_auto_register("Run");
  
  if (_params.mode == Params::SIMULATE) {
    
    // simulate mode - go through the file list repeatedly
    
    while (true) {
      
      char *inputPath;
      _input->reset();
      while ((inputPath = _input->next()) != NULL) {
	PMU_auto_register("Simulate mode");
	if (_processFile(inputPath)) {
	  cerr << "ERROR = Uf2Dsr::Run" << endl;
	  cerr << "  Processing file: " << inputPath << endl;
	  iret = -1;
	}
      } // while

    }

  } else {
    
    // loop until end of data
    
    char *inputPath;
    while ((inputPath = _input->next()) != NULL) {
      
      PMU_auto_register("Non-simulate mode");
      
      if (_processFile(inputPath)) {
	cerr << "ERROR = Uf2Dsr::Run" << endl;
	cerr << "  Processing file: " << inputPath << endl;
	iret = -1;
      }
      
    }

  } // if (_params.mode == Params::SIMULATE)
    
  return iret;

}

///////////////////////////////
// process file

int Uf2Dsr::_processFile(const char *input_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << input_path << endl;
  }

  
  char inputPath[MAX_PATH_LEN];
  sprintf(inputPath, "%s", input_path);

  //
  // If the file is gzipped, run gunzip on it.
  //
  if (strlen(inputPath) > 3){
    const char *last3 = inputPath + strlen(inputPath)-3;
    if (!(strcmp(".gz", last3))){
      char com[1024];
      if (_params.debug) {
	sprintf(com, "gunzip -vf %s", inputPath);
	cerr << "Executing " << com << endl;
      } else {
	sprintf(com, "gunzip -f %s", inputPath);
      }
      system(com);
      inputPath[strlen(inputPath)-3]=char(0);
    }
  }


  // open file - file closes automatically when inFile goes
  // out of scope.
  
  TaFile inFile;
  FILE *in;
  if ((in = inFile.fopen(inputPath, "r")) == NULL) {
    cerr << "ERROR - Uf2Dsr::_processFile" << endl;
    cerr << "  Cannot open input file: " << inputPath << endl;
    perror("  ");
    return -1;
  }
  
  // read through the records in the file

  bool firstBeam = true;
  time_t beamTime = time(NULL);
  int prevVolNum = -1;
  int prevTiltNum = -1;
  double prevElev = -99;

  while (!feof(in)) {

    // read rec header - this is a 4-byte integer FORTRAN uses

    ui32 nbytes;
    if (fread(&nbytes, sizeof(ui32), 1, in) != 1) {
      continue;
    }
    BE_to_array_32(&nbytes, sizeof(ui32));
    
    // read data record

    TaArray<ui08> record_;
    ui08 *record = record_.alloc(nbytes);
    if (fread(record, sizeof(ui08), nbytes, in) != nbytes) {
      break;
    }

    // read record trailer - this is a 4-byte integer FORTRAN uses
    
    ui32 nbytesTrailer;
    if (fread(&nbytesTrailer, sizeof(ui32), 1, in) != 1) {
      break;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read record - nbytes: " << nbytes << endl;
      if (nbytesTrailer != nbytes) {
        cerr << "WARNING - Uf2Dsr" << endl;
        cerr << "  Header record len differs from trailer len" << endl;
        cerr << "  Header  len: " << nbytes << endl;
        cerr << "  Trailer len: " << nbytesTrailer << endl;
      }
    }
    
    // load up beam object
    
    Beam beam(_progName, _params);
    if (beam.load(record, nbytes)) {
      cerr << "ERROR - Uf2Dsr" << endl;
      cerr << "  Cannot load UF beam" << endl;
      continue;
    }

    // compute the tilt num
    // if this returns an error it is an invalid beam

    if (beam.computeTiltNum()) {
      continue;
    }
    
    // save the time

    beamTime = beam.record.beamTime;

    // print if required

    if (_params.print_headers || _params.print_data) {
      beam.print(_params.print_headers, _params.print_data);
    }

    // volume and tilt flags

    bool volHasChanged = false;
    bool tiltHasChanged = false;

    if (firstBeam) {
      
      // put start of vol for first beam in file
      
      volHasChanged = true;
      tiltHasChanged = true;
      _rQueue.putStartOfVolume(beam.record.volNum, beamTime);
      _rQueue.putStartOfTilt(beam.record.tiltNum, beamTime);
      firstBeam = false;

    } else {
      
      // change in vol?

      if (_params.force_volume_change) {
	if (beam.record.elevation < prevElev) {
	  volHasChanged = true;
	}
      } else if (beam.record.volNum != prevVolNum) {
	volHasChanged = true;
      }
      
      // change in tilt?

      if (beam.record.tiltNum != prevTiltNum) {
	tiltHasChanged = true;
      }

      if (_params.debug) {
	if (tiltHasChanged) {
	  cerr << "Tilt has changed from "
	       << prevTiltNum << " to " << beam.record.tiltNum << endl;
	}
	if (volHasChanged) {
	  cerr << "Vol has changed from "
	       << prevVolNum << " to " << beam.record.volNum << endl;
	}
      }

      if (tiltHasChanged) {
	_rQueue.putEndOfTilt(prevTiltNum, beamTime);
      }

      if (volHasChanged) {
	_rQueue.putEndOfVolume(prevVolNum, beamTime);
	_rQueue.putStartOfVolume(beam.record.volNum, beamTime);
      }

      if (tiltHasChanged) {
	_rQueue.putStartOfTilt(beam.record.tiltNum, beamTime);
      }

    } // if (firstBeam)

    if (tiltHasChanged) {
      _putParams(beam);
    }

    _putBeam(beam);
    
    prevVolNum = beam.record.volNum;
    prevTiltNum = beam.record.tiltNum;
    prevElev = beam.record.elevation;
      
  } // while

  // put end of vol / tilt
  
  _rQueue.putEndOfTilt(prevTiltNum, beamTime);
  cerr << "Putting end of volume" << endl;
  _rQueue.putEndOfVolume(prevVolNum, beamTime);
    
  if (_params.debug) {
    cerr << "End of file" << endl;
    cerr << "Putting end of tilt: " << prevTiltNum << endl;
    cerr << "Putting end of vol: " << prevVolNum << endl;
  }

  return 0;

}

///////////////////////////////
// put radar and field params

int Uf2Dsr::_putParams(Beam &beam)

{

  // load up radar params
  
  DsRadarMsg msg;
  DsRadarParams &rParams = msg.getRadarParams();

  beam.loadRadarParams(rParams);
  
  // load up field params

  vector< DsFieldParams* > &fieldParams = msg.getFieldParams();
  for (int i = 0; i < _params.output_fields_n; i++) {
    DsFieldParams *fparams;
    if (_params.output_data_type == Params::OUTPUT_INT8) {
      fparams =
        new DsFieldParams(_params._output_fields[i].name,
                          _params._output_fields[i].units,
                          _params._output_fields[i].scale,
                          _params._output_fields[i].bias,
                          1, 0);
    } else if (_params.output_data_type == Params::OUTPUT_INT16) {
      fparams =
        new DsFieldParams(_params._output_fields[i].name,
                          _params._output_fields[i].units,
                          _params._output_fields[i].scale,
                          _params._output_fields[i].bias,
                          2, 0);
    } else if (_params.output_data_type == Params::OUTPUT_FLOAT32) {
      fparams =
        new DsFieldParams(_params._output_fields[i].name,
                          _params._output_fields[i].units,
                          _params._output_fields[i].scale,
                          _params._output_fields[i].bias,
                          4, -9999);
    }
    fieldParams.push_back(fparams);
  }
  
  // send the params

  if (_rQueue.putDsMsg
      (msg,
       DsRadarMsg::RADAR_PARAMS | DsRadarMsg::FIELD_PARAMS)) {
    cerr << "ERROR - Uf2Dsr::_writeRadarAndFieldParams" << endl;
    cerr << "  Cannot put radar and field params message to FMQ" << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////
// put beam

int Uf2Dsr::_putBeam(Beam &beam)

{

  DsRadarMsg msg;
  DsRadarBeam &dsBeam = msg.getRadarBeam();

  // params
  
  dsBeam.dataTime = beam.record.beamTime;
  dsBeam.volumeNum = beam.record.volNum;
  dsBeam.tiltNum = beam.record.tiltNum;
  dsBeam.azimuth = beam.record.azimuth;
  dsBeam.elevation = beam.record.elevation;
  dsBeam.targetElev = beam.record.targetAngle;
  dsBeam.targetAz = beam.record.targetAngle;

  // prepare data array

  int ngates;
  if (_params.set_ngates_out) {
    ngates = _params.ngates_out;
  } else {
    ngates = beam.record.maxGates;
  }
  int nfields = _params.output_fields_n;
  
  if (nfields <= 0 || ngates <= 0) {
    return 0;
  }

  if (_params.output_data_type == Params::OUTPUT_INT8) {
    _loadBeamInt8(nfields, ngates, beam, dsBeam);
  } else if (_params.output_data_type == Params::OUTPUT_INT16) {
    _loadBeamInt16(nfields, ngates, beam, dsBeam);
  } else if (_params.output_data_type == Params::OUTPUT_FLOAT32) {
    _loadBeamFloat32(nfields, ngates, beam, dsBeam);
  }

  // write the message
  
  int iret = 0;
  if (_rQueue.putDsMsg(msg, DsRadarMsg::RADAR_BEAM)) {
    iret = -1;
  }

  if (_params.mode != Params::REALTIME && _params.beam_wait_msecs > 0) {
    umsleep(_params.beam_wait_msecs);
  }

  return iret;

}

///////////////////////////////////////////////////////////
// load INT8 data

void Uf2Dsr::_loadBeamInt8(int nfields,
                           int ngates,
                           Beam &beam,
                           DsRadarBeam &dsBeam)

{

  int nFieldGates = nfields * ngates;
  int nbytes = nFieldGates;

  TaArray<ui08> beamData_;
  ui08 *beamData = beamData_.alloc(nFieldGates);
  memset(beamData, 0, nbytes);

  // load up data array

  for (int ifield = 0; ifield < nfields; ifield++) {
    
    string ufname = _params._output_fields[ifield].uf_name;
    bool found = false;
    size_t index = 0;
    for (size_t ii = 0; ii < beam.record.fieldInfo.size(); ii++) {
      if (ufname == UfRadar::label(beam.record.fieldInfo[ii].field_name, 2)) {
	found = true;
	index = ii;
      }
    } // ii

    if (found) {

      // fill in field data for this field

      ui08 *bptr = beamData + ifield;
      si16 *fptr = (si16 *) beam.record.fieldData[index].getPtr();
      int nn = ngates;
      if (nn > beam.record.fieldHdrs[index].num_volumes) {
	nn = beam.record.fieldHdrs[index].num_volumes;
      }
      double factor = beam.record.fieldHdrs[index].scale_factor;
      double scale = _params._output_fields[ifield].scale;
      double bias = _params._output_fields[ifield].bias;
      
      for (int j = 0; j < nn; j++, bptr += nfields, fptr++) {
        
        int ival = 0;
        if (*fptr != UF_NO_DATA) {
          double dval = *fptr / factor;
          ival = (int) ((dval - bias) / scale + 0.5);
          if (ival < 0) {
            ival = 0;
          } else if (ival > 255) {
            ival = 255;
          }
        }
        
	*bptr = (ui08) ival;

      } // j

    } // if (found) 

  } // ifield

  dsBeam.loadData(beamData, nbytes, 1);

}

///////////////////////////////////////////////////////////
// load INT16 beam

void Uf2Dsr::_loadBeamInt16(int nfields,
                            int ngates,
                            Beam &beam,
                            DsRadarBeam &dsBeam)
  
{

  int nFieldGates = nfields * ngates;
  int nbytes = nFieldGates * 2;
  
  TaArray<ui16> beamData_;
  ui16 *beamData = beamData_.alloc(nFieldGates);
  memset(beamData, 0, nbytes);
  
  // load up data array
  
  for (int ifield = 0; ifield < nfields; ifield++) {
    
    string ufname = _params._output_fields[ifield].uf_name;
    bool found = false;
    size_t index = 0;
    for (size_t ii = 0; ii < beam.record.fieldInfo.size(); ii++) {
      if (ufname == UfRadar::label(beam.record.fieldInfo[ii].field_name, 2)) {
	found = true;
	index = ii;
      }
    } // ii

    if (found) {
      
      // fill in field data for this field
      
      ui16 *bptr = beamData + ifield;
      si16 *fptr = (si16 *) beam.record.fieldData[index].getPtr();
      int nn = ngates;
      if (nn > beam.record.fieldHdrs[index].num_volumes) {
	nn = beam.record.fieldHdrs[index].num_volumes;
      }
      double factor = beam.record.fieldHdrs[index].scale_factor;
      double scale = _params._output_fields[ifield].scale;
      double bias = _params._output_fields[ifield].bias;
      
      for (int j = 0; j < nn; j++, bptr += nfields, fptr++) {

        int ival = 0;
        if (*fptr != UF_NO_DATA) {
          double dval = *fptr / factor;
          ival = (int) ((dval - bias) / scale + 0.5);
          if (ival < 0) {
            ival = 0;
          } else if (ival > 65535) {
            ival = 65535;
          }
        }

	*bptr = (ui16) ival;

      } // j

    } // if (found) 

  } // ifield

  dsBeam.loadData(beamData, nbytes, 2);

}

///////////////////////////////////////////////////////////
// load FLOAT32 beam

void Uf2Dsr::_loadBeamFloat32(int nfields,
                              int ngates,
                              Beam &beam,
                              DsRadarBeam &dsBeam)
  
{

  int nFieldGates = nfields * ngates;
  int nbytes = nFieldGates * 4;
  
  TaArray<fl32> beamData_;
  fl32 *beamData = beamData_.alloc(nFieldGates);
  memset(beamData, 0, nbytes);
  
  // load up data array
  
  for (int ifield = 0; ifield < nfields; ifield++) {
    
    string ufname = _params._output_fields[ifield].uf_name;
    bool found = false;
    size_t index = 0;
    for (size_t ii = 0; ii < beam.record.fieldInfo.size(); ii++) {
      if (ufname == UfRadar::label(beam.record.fieldInfo[ii].field_name, 2)) {
	found = true;
	index = ii;
      }
    } // ii

    if (found) {
      
      // fill in field data for this field
      
      fl32 *bptr = beamData + ifield;
      si16 *fptr = (si16 *) beam.record.fieldData[index].getPtr();
      int nn = ngates;
      if (nn > beam.record.fieldHdrs[index].num_volumes) {
	nn = beam.record.fieldHdrs[index].num_volumes;
      }
      double factor = beam.record.fieldHdrs[index].scale_factor;
      
      for (int j = 0; j < nn; j++, bptr += nfields, fptr++) {
        
        if (*fptr != UF_NO_DATA) {
          *bptr = -9999.0;
        } else {
          *bptr = *fptr / factor;
        }

      } // j

    } // if (found) 

  } // ifield

  dsBeam.loadData(beamData, nbytes, 4);

}

