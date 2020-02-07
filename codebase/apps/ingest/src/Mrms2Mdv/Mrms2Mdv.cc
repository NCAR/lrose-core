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
///////////////////////////////////////////////////////////////////////////
//  Mrms2Mdv top-level application class
//
//////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cstdio>
#include <string.h>
#include <ctime>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>

#include "Mrms2Mdv.hh"
#include "MrmsReader.hh"
#include "OutputFile.hh"

using namespace std;

// Global variables

Mrms2Mdv *Mrms2Mdv::_instance = (Mrms2Mdv *)NULL;

const int Mrms2Mdv::nFields = 20;
const char *Mrms2Mdv::fields[20]= { "CREF", "mosaicked_refl", "MEHS", "LCR_HIGH", "LCR_LOW", "LCR_SUPER", "LCREF", "LCREFH", "PCPFLAG", "POSH", "SHI", "STRMTOP30", "VIL", "VILD", "ETP", "RQI", "SHSR", "SHSRH", "UNQC_CREF", "CREFH" };

/*********************************************************************
 * Constructor
 */

Mrms2Mdv::Mrms2Mdv(int argc, char **argv) {

  const char *routine_name = "Constructor";

  // Make sure the singleton wasn't already created.

  assert(_instance == (Mrms2Mdv *)NULL);

  // Set the singleton instance pointer

  _instance = this;

  // Initialize the okay flag.

  okay = true;

  // Set the program name.

  path_parts_t progname_parts;

  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);

  if(progname_parts.dir != NULL)
    free(progname_parts.dir);
  if(progname_parts.name != NULL)
    free(progname_parts.name);
  if(progname_parts.base != NULL)
    free(progname_parts.base);
  if(progname_parts.ext != NULL)
    free(progname_parts.ext);

  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);

  if (!_args->okay)
  {
    fprintf(stderr,
    "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
    "Problem with command line arguments.\n");

    okay = false;

    return;
  }

  // Get TDRP parameters.
  _params = new Params();
  char *params_path;

  if (_params->loadFromArgs(argc, argv, _args->override.list, &params_path))
  {
    fprintf(stderr,
    "ERROR: %s::%s\n", _className(), "init");
    fprintf(stderr, "Problem with TDRP parameters in file <%s>\n",
                                                     params_path);
    okay = false;
    return;
  }
  //delete [] params_path;

   _data = new MemBuf();

  // initialize process registration

  PMU_auto_init( _className(), _params->instance, 
		 PROCMAP_REGISTER_INTERVAL );

}

/*********************************************************************
 * Destructor
 */

Mrms2Mdv::~Mrms2Mdv() 
{
   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();

  delete _data;

  if (_params != (Params *)NULL)
    delete _params;
  if (_args != (Args *)NULL)
    delete _args;
  if (_outputFile != (OutputFile *)NULL)
    delete _outputFile;

  // Free included strings
  STRfree(_progName);

}

/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Mrms2Mdv *Mrms2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (Mrms2Mdv *)NULL)
    new Mrms2Mdv(argc, argv);

  return(_instance);
}

Mrms2Mdv *Mrms2Mdv::Inst()
{
  assert(_instance != (Mrms2Mdv *)NULL);
  return(_instance);
}

int Mrms2Mdv::run()
{
  _outputFile = new OutputFile( _params );

  //
  // Initialize the data manager
  //
  if(_params->mode == Params::FILE_LIST) {
    _inputPath = new DsInputPath( _progName, _params->debug, _args->fileList, false );

  }
  else if(_params->mode == Params::START_END) {
    if ( _args->startTime == DateTime::NEVER || _args->endTime == DateTime::NEVER ) {
      cerr << "Must specify a start and end time on the command line for START_END mode" << endl;
      return( RI_FAILURE );
    }
    _inputPath = new DsInputPath( _progName, _params->debug, _params->input_dir,
				  _args->startTime, _args->endTime );

  } else {
    _inputPath = new DsInputPath( _progName, _params->debug, _params->input_dir,
				  _params->max_input_data_age,
                                  PMU_auto_register, 
                                  _params->latest_data_info_avail );

    _inputPath->setFileQuiescence(_params->file_quiescence);
    _inputPath->setSearchExt(_params->search_ext);
    _inputPath->setRecursion(_params->search_recursively);
    _inputPath->setMaxRecursionDepth(_params->max_recursion_depth);
    _inputPath->setMaxDirAge(_params->max_input_data_age);
    _inputPath->setDirScanSleep(_params->wait_between_checks);
  }

  _found = new bool[_params->required_fields_n];
  for(int c = 0; c < _params->required_fields_n; c++)
    _found[c] = false;
  _curTime = 0;

  if( _getData() != RI_SUCCESS ) {
    delete []_found;
    return( RI_FAILURE );
  }

  delete []_found;
  return( RI_SUCCESS );
}



int Mrms2Mdv::_getData()
{
   char *filePath;
   //
   // Process files

   
   while ((filePath = _inputPath->next()) != NULL ) {

      MrmsReader *input = new MrmsReader(_params->debug);
      //
      // Process the file
      PMU_auto_register( "Reading file" );
      if(_params->debug >= 1)
	cout << "Processing file " << filePath << endl << flush;

      if(input->readFile(filePath) == RI_FAILURE) {
	cerr << "Failed Reading file " << filePath << endl;
	delete input;
	continue;
      }
      if(_curTime != 0 && _curTime != input->getTime()) {

	  cout << "WARNING: Outputing mdv file due to change in data time, the following required fields were not found:"<<endl;
	  for(int c = 0; c < _params->required_fields_n; c++) {
	    if(_found[c] == false) {
	      cout << fields[_params->_required_fields[c]] << "  ";
	    }
	  }
	  cout << endl;

	  PMU_auto_register( "Writing mdv file" );
	  if( _writeMdvFile(_curTime, 1) != RI_SUCCESS ) {
	    cerr << "ERROR: Could not write MDV file." << endl << flush;
	    _clearMdvxFields();
	    return( RI_FAILURE );
	  }      
	  _clearMdvxFields();
      }
      _curTime = input->getTime();

      memset( (void *) &_fieldHeader, (int) 0, sizeof(Mdvx::field_header_t) );
      memset( (void *) &_vlevelHeader, (int) 0, sizeof(Mdvx::vlevel_header_t) );
      _vlevelHeader.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE_64;
      
      //
      // fill out the field header
      _fieldHeader.record_len1         = sizeof( Mdvx::field_header_t );
      _fieldHeader.struct_id           = Mdvx::FIELD_HEAD_MAGIC_COOKIE_64;
      _fieldHeader.field_code          = 0;
      _fieldHeader.forecast_delta      = 0;
      _fieldHeader.forecast_time       = input->getTime();
      _fieldHeader.data_element_nbytes = sizeof(float);
      _fieldHeader.encoding_type       = Mdvx::ENCODING_FLOAT32;    
      _fieldHeader.field_data_offset   = 0;
      _fieldHeader.compression_type    = Mdvx::COMPRESSION_ASIS;
      _fieldHeader.transform_type      = Mdvx::DATA_TRANSFORM_NONE;
      _fieldHeader.scaling_type        = Mdvx::SCALING_NONE;
      _fieldHeader.proj_rotation       = 0.0;
      _fieldHeader.scale               = 1.0;
      _fieldHeader.bias                = 0.0;
      _fieldHeader.missing_data_value  = input->getMissingVal();

      _fieldHeader.proj_type           = Mdvx::PROJ_LATLON;
      _fieldHeader.ny                  = input->getNy();
      _fieldHeader.nx                  = input->getNx();
      _fieldHeader.grid_miny           = input->getMinY(); //latitude[_fieldHeader.ny -1];
      _fieldHeader.grid_minx           = input->getMinX(); //longitude[0];
      _fieldHeader.grid_dy             = input->getDy();
      _fieldHeader.grid_dx             = input->getDx();
      //_fieldHeader.proj_origin_lat     = _fieldHeader.grid_miny;
      //_fieldHeader.proj_origin_lon     = _fieldHeader.grid_minx;

      _fieldHeader.nz                  = input->getNz();
      float *levels = input->getZhgt();
      
      if(_fieldHeader.nz == 1)
      {
	_fieldHeader.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
	_fieldHeader.vlevel_type        = Mdvx::VERT_TYPE_SURFACE;
	_fieldHeader.dz_constant        = 1;
	_fieldHeader.grid_dz            = 1.0;
	_fieldHeader.grid_minz          = 0;
	_vlevelHeader.type[0]           = Mdvx::VERT_TYPE_SURFACE;
	_vlevelHeader.level[0]          = 0;
      } else {
	for (int i = 0; i < input->getNz(); ++i){
	  _vlevelHeader.level[i] = levels[i] / 1000.0;
	  _vlevelHeader.type[i]  = Mdvx::VERT_TYPE_Z;
	}
	_fieldHeader.native_vlevel_type = Mdvx::VERT_TYPE_Z;
	_fieldHeader.vlevel_type        = Mdvx::VERT_TYPE_Z;
	_fieldHeader.grid_dz             = 0.0;
	_fieldHeader.grid_minz          = levels[0] / 1000.0;
      }
      
      _fieldHeader.volume_size = _fieldHeader.nx * _fieldHeader.ny *
	_fieldHeader.nz * _fieldHeader.data_element_nbytes;

      bool requested = _gotField(input->getVarName());
      if(!_params->ignore_unrequested_fields || requested == true)
      {
	//
	// Get and save the data
	_dataPtr = _data->add(input->getData(), sizeof(float)*_fieldHeader.nx*_fieldHeader.ny*_fieldHeader.nz );
	_outputFields.push_back(new MdvxField(_fieldHeader, _vlevelHeader, _dataPtr ) );
	_outputFields.back()->setFieldName(input->getVarName()); 
	_outputFields.back()->setFieldNameLong(input->getVarName()); 
	_outputFields.back()->setUnits(input->getVarUnits()); 	
	_data->reset();
      } 
      
      if(_gotRequiredFields()) {
	
	PMU_auto_register( "Writing mdv file" );
	if( _writeMdvFile(input->getTime(), _params->debug) != RI_SUCCESS ) {
	  cerr << "ERROR: Could not write MDV file." << endl << flush;
	  _clearMdvxFields();
	  return( RI_FAILURE );
	}      
	_clearMdvxFields();
      }

      delete input;
   }

   if(_curTime != 0)
   {
     cout << "WARNING: Outputing mdv file due to change in data time, the following required fields were not found:"<<endl;
     for(int c = 0; c < _params->required_fields_n; c++) {
       if(_found[c] == false) {
	 cout << fields[_params->_required_fields[c]] << "  ";
       }
     }
     cout << endl;
     
     PMU_auto_register( "Writing mdv file" );
     if( _writeMdvFile(_curTime, 1) != RI_SUCCESS ) {
       cerr << "ERROR: Could not write MDV file." << endl << flush;
       _clearMdvxFields();
       return( RI_FAILURE );
     }      
     _clearMdvxFields();
   }

   return(RI_SUCCESS);
}

bool Mrms2Mdv::_gotField(char* name)
{
  int fieldnum = -1;
  for(int b = 0; b < nFields; b++) {
    if(strcmp(fields[b], name) == 0)
      fieldnum = b;
  }
  bool requested = false;
  for(int c = 0; c < _params->required_fields_n; c++) {
    if(_params->_required_fields[c] == fieldnum) {
      _found[c] = true;
      requested = true;
    }
  }
  return requested;
}

bool Mrms2Mdv::_gotRequiredFields() 
{
  for(int c = 0; c < _params->required_fields_n; c++) {
    if(_found[c] == false)
      return false;
  }
  return true;
}

int Mrms2Mdv::_writeMdvFile(time_t generateTime, int debug)
{

  if(generateTime < 0) {
    cerr << " WARNING: File times don't make sense" << endl;
    cerr << "    Generate time = " << generateTime << endl;
    return( RI_SUCCESS );
  }

  if(debug >= 1) {
    DateTime genTime( generateTime );
    cout << "Writing grid output file at " << genTime.dtime() << endl << flush;
  }

  for( vector<MdvxField*>::iterator mfi = _outputFields.begin(); mfi != _outputFields.end(); mfi++ ) {
    _outputFile->addField( *mfi );

    *mfi = 0;
  }

  if ( _outputFile->writeVol( generateTime ) != 0 )
    return( RI_FAILURE );

  _outputFile->clear();

  return( RI_SUCCESS );
}

void Mrms2Mdv::_clearMdvxFields() 
{
  for( vector<MdvxField*>::iterator mfi = _outputFields.begin(); mfi != _outputFields.end(); mfi++ ) {
    if(*mfi) {
      delete (*mfi);
    }
    *mfi = 0;
  }
  _outputFields.erase(_outputFields.begin(), _outputFields.end());

  for(int c = 0; c < _params->required_fields_n; c++)
    _found[c] = false;

  _curTime = 0;
}

