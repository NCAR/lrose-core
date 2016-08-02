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
// Mdv2TextHdr.cc
//
// Mdv2TextHdr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////
//
// Mdv2TextHdr reads the master header from an mdv file, and
// writes text information to an output file. The format of 
// the output text is controlled by the parameter file. 
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/os_config.h>
#include <toolsa/pmu.h>
#include <toolsa/Path.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "Mdv2TextHdr.hh"
using namespace std;

// Constructor

Mdv2TextHdr::Mdv2TextHdr(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Mdv2TextHdr";
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

  if (_args.outputFilePath.size() > 0) {
    if (_args.inputFileList.size() != 1) {
      cerr << "ERROR: -of specified, for one output file." << endl;
      cerr << "  Therefore, specify one input file with -if." << endl;
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
    if (_input.setRealtime(_params.input_url,
			   _params.max_realtime_valid_age,
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

Mdv2TextHdr::~Mdv2TextHdr()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Mdv2TextHdr::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx mdvx;
  mdvx.setDebug(_params.debug);

  // loop until end of data
  
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // set up the Mdvx read

    mdvx.clearRead();
    
    if (_input.readAllHeadersNext(mdvx)) {
      cerr << "ERROR - Mdv2TextHdr::Run" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << _input.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "Working on file: " << mdvx.getPathInUse() << endl;
    }
    
    // write out

    PMU_auto_register("Before write");

    if (_writeOutput(mdvx)) {
      cerr << "ERROR - Mdv2TextHdr::Run" << endl;
      cerr << "  Cannot write output file." << endl;
      return -1;
    }
    
  } // while

  return 0;

}

///////////////////////////////
// write the output text header

int Mdv2TextHdr::_writeOutput(const DsMdvx &mdvx)

{

  // open tmp file

  char tmpPath[MAX_PATH_LEN];
  sprintf(tmpPath, "%s%s%s_%ld_%d",
	  _params.tmp_dir, PATH_DELIM,
	  _progName.c_str(), time(NULL), getpid());
  FILE *tmpFp;
  if ((tmpFp = fopen(tmpPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Mdv2TextHdr::_writeOutput" << endl;
    cerr << "Cannot open tmp file: " << tmpPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  // write to tmp file

  struct tm *gmt;
  struct tm res;
  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeaderFile();
  char timestr[64];
  
  for (int i = 0; i < _params.text_lines_n; i++) {
    
    switch (_params._text_lines[i].entry) {

    case Params::TIME_GEN: {
      time_t time_gen = mhdr.time_gen;
      gmt = gmtime_r(&time_gen,&res);
      strftime(timestr, 64, _params._text_lines[i].format,gmt);
      fprintf(tmpFp, "%s%s%s\n",
	      _params._text_lines[i].label,
	      timestr,
	      _params._text_lines[i].ending);
    }
    break;

    case Params::TIME_BEGIN: {
      time_t time_begin = mhdr.time_begin;
      gmt = gmtime_r(&time_begin,&res);
      strftime(timestr, 64, _params._text_lines[i].format,gmt);
      fprintf(tmpFp, "%s%s%s\n",
	      _params._text_lines[i].label,
	      timestr,
	      _params._text_lines[i].ending);
    }
    break;

    case Params::TIME_END: {
      time_t time_end = mhdr.time_end;
      gmt = gmtime_r(&time_end,&res);
      strftime(timestr, 64, _params._text_lines[i].format,gmt);
      fprintf(tmpFp, "%s%s%s\n",
	      _params._text_lines[i].label,
	      timestr,
	      _params._text_lines[i].ending);
    }
    break;

    case Params::TIME_CENTROID: {
      time_t time_centroid = mhdr.time_centroid;
      gmt = gmtime_r(&time_centroid,&res);
      strftime(timestr, 64, _params._text_lines[i].format,gmt);
      fprintf(tmpFp, "%s%s%s\n",
	      _params._text_lines[i].label,
	      timestr,
	      _params._text_lines[i].ending);
    }
    break;

    case Params::TIME_EXPIRE: {
      time_t time_expire = mhdr.time_expire;
      gmt = gmtime_r(&time_expire,&res);
      strftime(timestr, 64, _params._text_lines[i].format,gmt);
      fprintf(tmpFp, "%s%s%s\n",
	      _params._text_lines[i].label,
	      timestr,
	      _params._text_lines[i].ending);
    }
    break;

    case Params::N_FIELDS:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, _params._text_lines[i].format, mhdr.n_fields);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    case Params::N_CHUNKS:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, _params._text_lines[i].format, mhdr.n_chunks);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    case Params::SENSOR_LON:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, _params._text_lines[i].format, mhdr.sensor_lon);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    case Params::SENSOR_LAT:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, _params._text_lines[i].format, mhdr.sensor_lat);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    case Params::SENSOR_ALT:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, _params._text_lines[i].format, mhdr.sensor_alt);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    case Params::DATA_SET_INFO:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, _params._text_lines[i].format, mhdr.data_set_info);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    case Params::DATA_SET_NAME:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, _params._text_lines[i].format, mhdr.data_set_name);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    case Params::DATA_SET_SOURCE:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, _params._text_lines[i].format, mhdr.data_set_source);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    case Params::NO_VALUE:
      fprintf(tmpFp, "%s", _params._text_lines[i].label);
      fprintf(tmpFp, "%s\n", _params._text_lines[i].ending);
      break;

    } // switch

  }

  // close tmp file

  fclose(tmpFp);

  // move the tmp file to the correct location

  char outPath[MAX_PATH_LEN];

  if (_args.outputFilePath.size() > 0) {
    sprintf(outPath, "%s", _args.outputFilePath.c_str());
  } else {
    date_time_t otime;
    otime.unix_time = mhdr.time_centroid;
    uconvert_from_utime(&otime);
    sprintf(outPath, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	    _params.output_dir, PATH_DELIM,
	    otime.year, otime.month, otime.day, PATH_DELIM,
	    otime.hour, otime.min, otime.sec,
	    _params.output_file_ext);
  }

  Path outP(outPath);
  if (ta_makedir_recurse(outP.getDirectory().c_str())) {
    int errNum = errno;
    cerr << "ERROR - Mdv2TextHdr::_writeOutput" << endl;
    cerr << "  Cannot make output dir: " << outP.getDirectory() << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  if (rename(tmpPath, outPath)) {
    int errNum = errno;
    cerr << "ERROR - Mdv2TextHdr::_writeOutput" << endl;
    cerr << "  Cannot rename tmp file: " << tmpPath << endl;
    cerr << "                 to file: " << outPath << endl;
    cerr << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}

