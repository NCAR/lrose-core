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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Mdv2NesdisArchive.cc,v 1.9 2019/03/04 00:22:24 dixon Exp $
 */

# ifndef    lint
static char RCSid[] = "$Id: Mdv2NesdisArchive.cc,v 1.9 2019/03/04 00:22:24 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	Mdv2NesdisArchive
//
// Author:	G. M. Cunning
//
// Date:	Mon Dec  3 09:36:15 2007
//
// Description: 
//
//

// C++ include files
#include <iostream>
#include <cassert>
#include <fcntl.h>

// System/RAP include files
#include <toolsa/os_config.h>
#include <dataport/port_types.h>
#include <dataport/bigend.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvxInput.hh>

// Local include files
#include "Mdv2NesdisArchive.hh"
#include "Params.hh"
#include "Args.hh"

using namespace std;

// the singleton itself
Mdv2NesdisArchive *Mdv2NesdisArchive::_instance = 0;
 
// define any constants
const string Mdv2NesdisArchive::_className = "Mdv2NesdisArchive";
const string Mdv2NesdisArchive::DATA_EXTENSION = "dat";
const string Mdv2NesdisArchive::LAT_EXTENSION = "lat";
const string Mdv2NesdisArchive::LON_EXTENSION = "lon";

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Mdv2NesdisArchive::Mdv2NesdisArchive(int argc, char **argv)
 :
  _isOK(true),
  _progName(""),
  _errStr(""),
  _args(0),
  _params(0)
{
  const string methodName = _className + string("::Constructor");
  _errStr = string("ERROR: ") + methodName;

  // Make sure the singleton wasn't already created.
  assert(_instance == 0);
 
  // Set the singleton instance pointer
  _instance = this;
 
  // set programe name
  Path pathParts(argv[0]);
  _progName = pathParts.getBase();
  ucopyright(const_cast<char*>(_progName.c_str()));
 
  // get command line args
  _args = new Args;
  _args->parse(argc, argv, _progName);

  // get TDRP params
  _params = new Params();
  char *paramsPath = (char *) "unknown";
  if(_params->loadFromArgs(argc, argv, _args->override.list,
			  &paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    _isOK = false;
  } // endif -- _params.loadFromArgs(argc, argv, _args.override.list, ...

  // display ucopyright message and RCSid
  if(_params->debug) {
    cerr << RCSid << endl;
  } //endif -- _params->debug_mode != Params::DEBUG_VERBOSE


 
 // init process mapper registration
  PMU_auto_init((char *) _progName.c_str(),
		_params->instance,
		PROCMAP_REGISTER_INTERVAL);


  // setup the input object
  _setupInput();

  return;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
Mdv2NesdisArchive::~Mdv2NesdisArchive()
{
  // unregister process
  PMU_auto_unregister();

  // Free contained objects
  delete _params;
  delete _args;
  delete _dsInput;
  delete _mdvxIn;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::instance
//
// Description:	this method creates instance of Mdv2NesdisArchive object
//
// Returns:	returns pointer to self
//
// Notes:	this method implements the singleton pattern
//
//

Mdv2NesdisArchive*
Mdv2NesdisArchive::instance(int argc, char **argv)
{
  if (_instance == 0) {
    new Mdv2NesdisArchive(argc, argv);
  }
  return(_instance);
}

Mdv2NesdisArchive*
Mdv2NesdisArchive::instance()
{
  assert(_instance != 0);
  
  return(_instance);
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::run
//
// Description:	main method for class
//
// Returns:	returns 0
//
// Notes:	
//
//

int 
Mdv2NesdisArchive::run()
{
  const string methodName = _className + string( "::_run" );

  // register with procmap
  PMU_auto_register("Running");

  if(_params->debug) {
    cerr << "Running:" << endl;
  } // endif -- _params->debug_mode

  // Do something
  while(!_dsInput->endOfData()) {

    PMU_auto_register("In main loop");

    // read the new file and get mandatory and fields to interpolate
    vector< const MdvxField* > inputFields = _readInput();
    if(inputFields.empty()) {
      continue;
    }

    // process fields
    if(!_processFields(inputFields)) {
      continue;
    }
    
  } // end while

  return 0;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::_setupInput
//
// Description:	This method hanldes initialization of the DsMdvx object for input.
//
// Returns:	
//
// Notes:
//
//

void 
Mdv2NesdisArchive::_setupInput()
{
  const string methodName = _className + string( "::_setupInput" );

  _dsInput = new DsMdvxInput;

  if (_params->mode == Params::ARCHIVE &&
      (_args->getArchiveStartTime() == 0 ||
       _args->getArchiveEndTime() == 0)) {
    cerr << "ERROR - " << methodName << endl;
    cerr <<  "\tError in command line" << endl;
    cerr << "\t-start and -end must be specified on the command line in ARCHIVE mode" << endl;
    _isOK = false;
    return;
  }

  if (_params->mode == Params::REALTIME) {
    if (_dsInput->setRealtime(_params->input_url, _params->max_valid_age, 
			    (DsMdvxInput::heartbeat_t)PMU_auto_register)) {
      cerr << "ERROR - " << methodName << endl;
      cerr << "  setup for realtime mode failed." << endl;
      cerr << _dsInput->getErrStr() << endl;
      _isOK = false;
      return;
    }
  } else if (_params->mode == Params::ARCHIVE) {
    if(_dsInput->setArchive(_params->input_url, 
			 _args->getArchiveStartTime(),
			    _args->getArchiveEndTime())) {
      cerr << "ERROR - " << methodName << endl;
      cerr << "archive setup for " <<  _params->input_url << " failed." << endl;
      cerr << _dsInput->getErrStr() << endl;
      _isOK = false;
      return;
    } 
  } else if (_params->mode == Params::FILELIST) {
    if (_dsInput->setFilelist(_args->getInputFileList())) {
      cerr << "ERROR - " << methodName << endl;
      cerr << "  setup for file list mode failed." << endl;
      cerr << _dsInput->getErrStr() << endl;
      _isOK = false;
      return;
    }
  }
  else {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  Unknown mode." << endl;
    _isOK = false;
    return;

  } // endif -- _params->mode == Params::REALTIME

  _mdvxIn = new DsMdvx;

}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::_readInput
//
// Description:	reads the mandatory and fields to interpolate.
//
// Returns:	
//
// Notes:
//
//

vector<const MdvxField* > 
Mdv2NesdisArchive::_readInput()
{
  const string methodName = _className + "::_readInput";

  vector<const MdvxField*> inputFields;

  // do the setup
  if(_params->debug) {
    _mdvxIn->setDebug();
  } // endif -- _params->debug_mode != Params::DEBUG_OFF

  _mdvxIn->clearRead();
  _mdvxIn->clearReadFields();
  _mdvxIn->clearReadChunks();
  _mdvxIn->clearFields();
  _mdvxIn->clearChunks();

  for(int i = 0; i < _params->output_list_n; ++i) {
    _mdvxIn->addReadField(_params->_output_list[i].field_name);    
  }

  _mdvxIn->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvxIn->setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _mdvxIn->setReadScalingType(Mdvx::SCALING_NONE);

  if(_dsInput->readVolumeNext(*_mdvxIn)) {
    cerr << "WARNING: " << methodName << endl;
    cerr << _dsInput->getErrStr() << endl;
    return inputFields;
    
  } // endif --  ->readVolume()
  else if(_params->debug) {
    cerr << "Reading from " << _mdvxIn->getPathInUse() << endl;
  } // endif -- _params->debug_mode == Params::DEBUG_NORM
    
  // get interpolation fields
  for(int i = 0; i < _params->output_list_n; i++) {
    inputFields.push_back(_mdvxIn->getField(_params->_output_list[i].field_name));    
  }
  Mdvx::master_header_t masterHeader = _mdvxIn->getMasterHeader();
  Mdvx::field_header_t fieldHeader = inputFields[0]->getFieldHeader();
  _proj.init(fieldHeader);

  _obsTime.set(masterHeader.time_centroid + _params->centroid_offset);
  _pathInUse = _mdvxIn->getPathInUse();

  return inputFields;
}



/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::_processFields
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool
Mdv2NesdisArchive::_processFields(vector< const MdvxField* >& input_fields)
{
  const string methodName = _className + "::_processFields";

  // write fields
  for(size_t i = 0; i < input_fields.size(); ++i) {

    const MdvxField* mdvxField = input_fields[i];

    if(mdvxField == 0) {
      continue;
    }

    // copy the volume and make appropriate changes to bad, missing and special value
    Mdvx::field_header_t fieldHeader = mdvxField->getFieldHeader();
    int numPts = mdvxField->getVolNumValues();
    const float* fieldData = static_cast<const float *> (mdvxField->getVol());
    float *outData = new float[numPts];

    float origMissing;
    float origBad;
    float newMissing;
    float newBad;
    float origSpecial;
    float newSpecial;

    if(_params->_output_list[i].reset_bad_and_missing) {
      origMissing = fieldHeader.missing_data_value;
      origBad = fieldHeader.bad_data_value;
      newMissing = _params->_output_list[i].missing_value;
      newBad = _params->_output_list[i].bad_value;
    }
    if(_params->_output_list[i].reset_special) {
      origSpecial = _params->_output_list[i].original_special;
      newSpecial = _params->_output_list[i].new_special;
    }

    for(int j = 0; j < numPts; ++j) {

      outData[j] = fieldData[j];

      if(_params->_output_list[i].reset_bad_and_missing) {
	if(fabs(outData[j] - origMissing) < EPSILON) {
	  outData[j] = newMissing;
	}

	if(fabs(outData[j] - origBad) < EPSILON) {
	  outData[j] = newBad;
	}
      }

      if(_params->_output_list[i].reset_special) {
	if(fabs(outData[j] - origSpecial) < EPSILON) {
	  outData[j] = newSpecial;
	}
      }

    }

    string thePath = _buildOutputPath(_params->_output_list[i].output_dir, 
				      _params->_output_list[i].field_name, DATA_EXTENSION);
    if(thePath.size() == 0) {
      cerr << methodName << " -- ERROR: unable to create output directory." << endl; 
      return false;
    }

    int out_fd = open(thePath.c_str(), (O_WRONLY|O_CREAT|O_TRUNC), 
		      (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
    if (out_fd < 0) {
      cerr << "unable to open output file! -- errno = " << errno << endl;  
      perror("write_bin -- ");
      return false;
    }

    if (!_writeData(out_fd, numPts, outData)) {
      close(out_fd);
      delete[] outData;
      return false;
    }

    delete[] outData;

    // close descriptor
    close(out_fd);
    
    if(_params->write_naviagation) {
      _writeNavigation(_params->_output_list[i].output_dir, 
		       _params->_output_list[i].field_name);
    }
  }
 

  return true;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::_writeData
//
// Description:	writes 'field' to file descriptor
//
// Returns:	
//
// Notes:	
//
//

bool 
Mdv2NesdisArchive::_writeData(int out_fd, int num_pts, float* data)
{

  int field_size = num_pts*sizeof(float);

  if(_params->output_le) {
    BE_reverse();
  }
  BE_swap_array_32(data, field_size);
  BE_swap_array_32(&field_size, sizeof(int));
  if(_params->output_le) {
    BE_reverse();
  }

  // write size of field in bytes
  if(!_writePad(out_fd, field_size)) {
    return false;
  }

  // write the field of data 
  size_t n_write = write(out_fd, (void *)data, num_pts*sizeof(float));
  if(n_write != num_pts*sizeof(float)) {
    cerr << "can't write bin data -- errno = " << errno << endl;  
    perror("FipAlgo::_writeData -- ");
    return false;
  } 

  // write size of field in bytes again
  if(!_writePad(out_fd, field_size)) {
    return false;
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::_writePad
//
// Description:	
//
// Returns:	
//
// Notes:	
//
//

bool 
Mdv2NesdisArchive::_writePad(int out_fd, int fld_size)
{
  if(!_params->write_fortran_binary) {
    return true;
  }

  size_t n_write = write(out_fd, (void *)&fld_size, sizeof(int));
  if(n_write != sizeof(int)) {
    cerr << "can't write bin data -- errno = " << errno << endl;  
    perror("write_pad -- ");
    return false;
  } 

  return true;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::_buildOutputPath
//
// Description:	creates directory if it doen't exist, and create path to 
//		for data.
//
// Returns:	if successful output path. If failure, empty string
//
// Notes:	
//
//

string 
Mdv2NesdisArchive::_buildOutputPath(const string& base_dir, const string& field_name, const string& ext)
{

  string outDir;
  if(_params->use_rap_dir) {
    outDir = base_dir + "/" + _obsTime.getDateStrPlain();
  } 
  else {
    outDir = base_dir;
  }

  int iret = ta_makedir_recurse(outDir.c_str());

  if(iret == -1) {
    return "";
  }

  string fileName;
  if(_params->prepend_field_name) {
    fileName = field_name + "_" + _obsTime.getDateStrPlain() + "_" + _obsTime.getTimeStrPlain() + "." + ext;
  } 
  else if(_params->add_prefix) {
    fileName = string(_params->file_prefix) + "_" + _obsTime.getDateStrPlain() + "_" + _obsTime.getTimeStrPlain() + "." + ext;
  } 
  else {
    fileName = _obsTime.getDateStrPlain() + "_" + _obsTime.getTimeStrPlain() + "." + ext;
  }

  string filePath = outDir + "/" + fileName;

  if(_params->debug) {
    cout << "Writing to file: " << filePath << endl;
  }

  return filePath;
}



/////////////////////////////////////////////////////////////////////////
//
// Method Name:	Mdv2NesdisArchive::_writeNavigation
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool
Mdv2NesdisArchive::_writeNavigation(const string& base_dir, const string& field_name)
{
  const string methodName = _className + "::_writeNavigation";


  //
  // create latitudes and longitudes for underlying grid
  //
  const Mdvx::coord_t coordInfo = _proj.getCoord();
  int numPts = coordInfo.nx*coordInfo.ny;
  float *latData = new float[numPts];
  float *lonData = new float[numPts];

  double latOffset = coordInfo.dy/2.0;
  double lonOffset = coordInfo.dx/2.0;

  //
  // calculate offset from MDV grid reference to output grid reference
  //
  
  switch(_params->output_grid_reference) {
  case Params::GRID_REF_CENTER: 
    {
      latOffset *= 0.0;
      lonOffset *= 0.0;
      if(_params->debug) {
	cout << "GRID_REF_CENTER" << "   latOffset = " << latOffset << "   lonOffset = " << lonOffset << endl;
      }
      break;
    }
  case Params::GRID_REF_LOWER_LEFT: 
    {
      latOffset *= -1.0;
      lonOffset *= -1.0;
      if(_params->debug) {
	cout << "GRID_REF_CENTER   latOffset = " << latOffset << "   lonOffset = " << lonOffset << endl;
      }
      break;
    }
  case Params::GRID_REF_UPPER_LEFT:
    {
      latOffset *= 1.0;
      lonOffset *= -1.0;
      if(_params->debug) {
	cout << "GRID_REF_UPPER_LEFT   latOffset = " << latOffset << "   lonOffset = " << lonOffset << endl;
      }
      break;
    }
  case Params::GRID_REF_UPPER_RIGHT:
    {
      latOffset *= 1.0;
      lonOffset *= 1.0;
      if(_params->debug) {
	cout << "GRID_REF_UPPER_RIGHT   latOffset = " << latOffset << "   lonOffset = " << lonOffset << endl;
      }
      break;
    }
  case Params::GRID_REF_LOWER_RIGHT: 
    {
      latOffset *= -1.0;
      lonOffset *= 1.0;
      if(_params->debug) {
	cout << "GRID_REF_LOWER_RIGHT   latOffset = " << latOffset << "   lonOffset = " << lonOffset << endl;
      }
      break;
    }
  }

  int idx = 0;
  for(int j = 0; j < coordInfo.ny; j++) {
    for(int i = 0; i < coordInfo.nx; i++) {

      double theLat;
      double theLon;

      _proj.xyIndex2latlon(i, j, theLat, theLon);

      latData[idx] = static_cast<float>(theLat+latOffset);
      lonData[idx] = static_cast<float>(theLon+lonOffset);
      idx++;

    }
  }

  //
  // open and write latitude file
  //
  string thePath = _buildOutputPath(base_dir, field_name, LAT_EXTENSION);
  if(thePath.size() == 0) {
    cerr << methodName << " -- ERROR: unable to create output directory." << endl; 
    return false;
  }

  int out_fd = open(thePath.c_str(), (O_WRONLY|O_CREAT|O_TRUNC), 
		    (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
  if (out_fd < 0) {
    cerr << methodName << " -- unable to open output file! -- errno = " << errno << endl;  
    perror("write_bin -- ");
    return false;
  }

  if (!_writeData(out_fd, numPts, latData)) {
    close(out_fd);
   delete[] latData;
   return false;
  }
  
   delete[] latData;
  
  // close descriptor
  close(out_fd);

  //
  // open and write longitude file
  //
  thePath = _buildOutputPath(base_dir, field_name, LON_EXTENSION);
  if(thePath.size() == 0) {
    cerr << methodName << " -- ERROR: unable to create output directory." << endl; 
    return false;
  }


  out_fd = open(thePath.c_str(), (O_WRONLY|O_CREAT|O_TRUNC), 
		(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH));
  if (out_fd < 0) {
    cerr << methodName << " -- unable to open output file! -- errno = " << errno << endl;  
    perror("write_bin -- ");
    return false;
  }

  if (!_writeData(out_fd, numPts, lonData)) {
    close(out_fd);
   delete[] lonData;
   return false;
  }
  
  delete[] lonData;
  
  // close descriptor
  close(out_fd);

  return true;

}
