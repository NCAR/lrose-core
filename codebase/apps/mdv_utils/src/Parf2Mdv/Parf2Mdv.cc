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
#include "Parf2Mdv.hh"
using namespace std;

//
// Constructor
//
Parf2Mdv::Parf2Mdv(int argc, char **argv):
_progName("Parf2Mdv"),
_args(_progName)

{
  isOK = true;

  //
  // set programe name
  //
  ucopyright((char *) _progName.c_str());

  //
  // get command line args
  //
  if (_args.parse(argc, argv)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with command line args" << endl;
      isOK = FALSE;
      return;
    }

  //
  // get TDRP params
  //
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) 
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters" << endl;
      isOK = FALSE;
      return;
    }

  //
  // init process mapper registration
  //
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  _data = NULL;

  _fileTrigger = NULL;
  
  return;

}

//////////////////////////////////////////////////////
//
// destructor
//
Parf2Mdv::~Parf2Mdv()
{
  //
  // unregister process
  //
  PMU_auto_unregister();  
}

//////////////////////////////////////////////////////
//
// destructor
//
void Parf2Mdv::_clear()
{
  if (_parfReader )
    delete _parfReader;

  if(_data)
    delete[] _data;

  _data = NULL;
}

//////////////////////////////////////////////////
// 
// Run
//
int Parf2Mdv::Run ()
{
  //
  // register with procmap
  //
  PMU_auto_register("Run");

  //
  // Initialize file trigger
  //
  if ( _params.mode == Params::REALTIME ) 
    {
      if (_params.debug)
	cerr << "FileInput::init: Initializing realtime input from " 
	     << _params.input_dir << endl;
      
      _fileTrigger = new DsInputPath( _progName, _params.debug,
				     _params.input_dir,
				     _params.max_valid_realtime_age_min*60,
				     PMU_auto_register,
				     _params.ldata_info_avail,
				     false );
      //
      // Set wait for file to be written to disk before being served out.
      //
      _fileTrigger->setFileQuiescence( _params.file_quiescence_sec );
      
      //
      // Set time to wait before looking for new files.
      // 
      _fileTrigger->setDirScanSleep( _params.check_input_sec );
      
    }

   if ( _params.mode == Params::FILELIST ) 
    {    
      //
      // Archive mode.
      //
      const vector<string> &fileList =  _args.getInputFileList();
      if ( fileList.size() ) 
	{
	  if (_params.debug)
	    cerr << "FileInput::init: Initializing archive FILELIST mode." << endl;
	  
	  _fileTrigger = new DsInputPath( _progName, _params.debug , fileList );
	}
    }
    
   if ( _params.mode == Params::TIME_INTERVAL ) 
     { 
       //
       // Set up the time interval
       //
       DateTime start( _params.start_time);
      
       DateTime end( _params.end_time);
       
       time_t time_begin = start.utime();
       
       time_t time_end = end.utime();
       
       if (_params.debug)
	 {
	   cerr << "FileInput::init: Initializing file input for time interval [" 
		<< time_begin << " , " << time_end << "]." << endl;
	   
	 }
       
       _fileTrigger = new DsInputPath( _progName, _params.debug,
				      _params.input_dir,
				      time_begin,
				      time_end);
     }
  //
  //
  // process data
  //
  char *inputPath;

  if (_fileTrigger != NULL)
    {
      while ( (inputPath = _fileTrigger->next()) != NULL)
	{
	  
	  if (_processData(inputPath))
	    {
	      cerr << "Error - Parf2Mdv::Run" << endl;
	      cerr << "  Errors in processing time: " <<  inputPath << endl;
	      return 1;
        }
	} // while
      
      if (_fileTrigger)
	delete _fileTrigger;
    }

  return 0;
  
}


///////////////////////////////////
//
//  process data at trigger time
//
int Parf2Mdv::_processData(char *inputPath)

{
  if (_params.debug)
    {
      cerr << "Parf2Mdv::_processData: Processing file : " << inputPath << endl;
    }
  
  //
  // registration with procmap
  //
  PMU_force_register("Processing data");

  //
  // Read in the parf file
  //
  _parfReader = new ParfReader(inputPath, _params);

  if (_parfReader->readFile())
    {
      cerr << "Parf2Mdv::_processData(): ERROR reading  " <<  inputPath 
	   << " Mdv file will not be written. " << endl;
    }
  else
    _writeMdv();	

  _clear();

  return 0;

}

int Parf2Mdv::_writeMdv()
{
  DsMdvx mdvFile;

  DsMdvx::master_header_t masterHdr;
  
  Mdvx::vlevel_header_t vlevelHdr;

  //
  // Fill out master header and add it to the DsMdvx object
  //
  _setMasterHeader(masterHdr);

  mdvFile.setMasterHeader(masterHdr);


  //
  // Set the vlevel information struct for Mdv. There is no vlevel
  // data so we'll use the same vlevelHdr for all fields.
  //
  _setVlevelHeader(vlevelHdr);

  //
  // Get field data 
  //
  if (_createFieldDataArray())
    {
      cerr << "Parf2Mdv::_writeMdv(): ERROR: Cannot write Mdv file." << endl;
      
      return 1;
    }

  for (int i = 0; i < _parfReader->getNumFields(); i++)
    {
      Mdvx::field_header_t fHdr;

      //
      // Set the Field and Vlevel header
      //
      if (_setFieldHeader(fHdr, i))
	{
	  cerr << "Parf2Mdv::_writeMdv(): ERROR: Unable to set field header." << endl;
	  
	  return 1;
	}
      
      fl32 *fieldDataPtr = _data + _parfReader->getNx() * _parfReader->getNy() * i;

      MdvxField *field = new MdvxField(fHdr, vlevelHdr, (void*)fieldDataPtr );

      field->convertType(Mdvx::ENCODING_FLOAT32,
                         Mdvx::COMPRESSION_BZIP,
                         Mdvx::SCALING_NONE);

      //
      // Add field data
      //
      if (_params.debug)
	{
	  cerr << "Parf2Mdv::_writeMdv(): adding field "
	       <<_parfReader->getFieldName(i) << endl; 
	  
	}
      mdvFile.addField(field);
    }
	
  //
  // Write data
  //
  if (mdvFile.writeToDir(_params.output_url) != 0)
  {
    cerr << "Parf2Mdv::writeMdv(): "
         << "Error writing MDV file to URL: " << _params.output_url << endl;

    return 1;
  }

  return 0;
}

//
// setMasterHeader(): Fill out the master header fields
//
int Parf2Mdv::_setMasterHeader(DsMdvx::master_header_t &masterHdr)
{
  if (_params.debug)
    cerr << "Parf2Mdv::_setMasterHeader(): Setting master header." << endl;

  memset(&masterHdr, 0, sizeof(masterHdr));

  //masterHdr.time_gen = some_time;

  masterHdr.time_begin = _parfReader->getTime();

  masterHdr.time_end = _parfReader->getTime();

  masterHdr.time_centroid = _parfReader->getTime();

  masterHdr.data_dimension = 2; 
  
  //
  // Always set
  //
  masterHdr.vlevel_included = 1;
  
  //
  // Always set this way, assumed in the input data
  //
  masterHdr.grid_orientation = Mdvx::ORIENT_SN_WE;

  //
  // Always set this way.
  // We assume order XYZ for the input data as is the recommended 
  // (but not required)data ordering of the CF-1.0 conventions.
  //
  masterHdr.data_ordering = Mdvx::ORDER_XYZ;

  masterHdr.n_fields =_parfReader->getNumFields()  ;  
  
  masterHdr.time_written = time(0);

  STRcopy(masterHdr.data_set_info,"Statistical Analysis of Storm Predictors" , MDV_INFO_LEN);

  STRcopy(masterHdr.data_set_name, "Parf Output", MDV_NAME_LEN);

  STRcopy(masterHdr.data_set_source, "RAL, NCAR", MDV_NAME_LEN);
 
  return 0;
}

int Parf2Mdv::_setFieldHeader(Mdvx::field_header_t &fHdr, int fieldNum)
{
  if (_params.debug)
    {
      cerr << "Parf2Mdv::_setFieldHeaders(): Setting field header" 
	   << " for field " <<  _parfReader->getFieldName(fieldNum) << endl;
    }

  //
  // Initialize the field header
  //
  memset(&fHdr, 0, sizeof(Mdvx::field_header_t));
  
  fHdr.forecast_delta = 0;
	  
  fHdr.forecast_time = _parfReader->getTime();
	  
  fHdr.nx = _parfReader->getNx();
	  
  fHdr.ny = _parfReader->getNy();
	  
  fHdr.nz = 1;
	  
  fHdr.proj_type =  _parfReader->getProjType();
  
  if (fHdr.proj_type != Mdvx::PROJ_LATLON)
    {
      cerr << "Parf2Mdv::_setFieldHeader: ERROR: currently only supporting Mdvx::PROJ_LATLON " << endl;

      return 1;
    }
  
  fHdr.encoding_type = Mdvx::ENCODING_FLOAT32;
	  
  fHdr.data_element_nbytes = 4;
	  
  fHdr.volume_size = fHdr.nx * fHdr.ny * fHdr.data_element_nbytes;
 	  
  fHdr.compression_type = Mdvx::COMPRESSION_NONE;
	  
  fHdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
	  
  fHdr.scaling_type = Mdvx::SCALING_NONE;
	  
  fHdr.data_dimension = fHdr.nx * fHdr.ny;
	
  //
  // origin lat and lon not set for this projection
  // fHdr.proj_origin_lat = 0;
  // fHdr.proj_origin_lon = 0;
  //
  
  fHdr.grid_dx = _parfReader->getDx();
	  
  fHdr.grid_dy = _parfReader->getDy();
	  
  fHdr.grid_minx = _parfReader->getMinX();
	  
  fHdr.grid_miny = _parfReader->getMinY();
	  
  fHdr.grid_minz = 0;
	
  _missing = floor(_parfReader->getMinValue(fieldNum) - 1);

  fHdr.bad_data_value = _missing;
	  
  fHdr.missing_data_value = _missing;
	  
  fHdr.proj_rotation = 0;  
  
  STRcopy(fHdr.field_name_long, _parfReader->getFieldName(fieldNum), MDV_LONG_FIELD_LEN);

  STRcopy(fHdr.field_name, _parfReader->getFieldName(fieldNum), MDV_SHORT_FIELD_LEN);
  
  STRcopy(fHdr.units, "not_available", MDV_SHORT_FIELD_LEN);
	  
  return 0;
}

int Parf2Mdv::_setVlevelHeader(Mdvx::vlevel_header_t &vlevelHdr)
{
  memset(&vlevelHdr, 0, sizeof(Mdvx::vlevel_header_t));

  //
  // This is 2D dataset
  //
  vlevelHdr.type[0] =  Mdvx::VERT_TYPE_UNKNOWN;

  vlevelHdr.level[0] = 0;

  return 0;
}

//
// _createFieldDataArray(): create a Pjg object from the grid information 
//                     stored in the parf file. Put all parf class points
//                     into the grid
//
int Parf2Mdv::_createFieldDataArray()
{
  
  Pjg grid;

  if (_parfReader->getProjType() == 0)
    grid.initLatlon( _parfReader->getNx(), _parfReader->getNy() , 1,
		     _parfReader->getDx(), _parfReader->getDy(),1,
		     _parfReader->getMinX(), _parfReader->getMinY(), 0);
  else
    {
      cerr << "Parf2Mdv::_createParfField(): Mdv projection " <<  _parfReader->getProjType() 
	   << " not supported. Must add projection support to Parf2Mdv::_createParfField()" << endl;
      return 1;
    }

  //
  // The size of this array is nx *ny * number of fields 
  //
  int arraySize = _parfReader->getNx() * _parfReader->getNy() *_parfReader->getNumFields();
  
  _data = new fl32[arraySize];

  //
  // Initialize data
  //
  for ( int i = 0; i < arraySize; i++)
    _data[i] = _missing;

  //
  // Fill grid with parf data
  //
  for (int i = 0; i < _parfReader->getNumClassPts(); i++)
    {
      fl32 lat = _parfReader->getClassPtLat(i);
      
      fl32 lon = _parfReader->getClassPtLon(i);
      
      //
      // Convert lat lon to array index and add the class
      // data to the grid
      //
      int index2D;
	  
      grid.latlon2arrayIndex( lat, lon, index2D);
      
      for (int j = 0; j < _parfReader->getNumFields(); j++)
	{
	  fl32 dataVal = _parfReader->getFieldData(i,j);

	  int index =  _parfReader->getNx() * _parfReader->getNy() * j  + index2D;

 	  _data[index] = dataVal;
	}
    }

  return 0;
} 
