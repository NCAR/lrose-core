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
/////////////////////////////////////////////////////////////
// QPESumVectors2Mdv.cc
//
// QPESumVectors2Mdv object
//
// Dan Megenhardt, RAL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 2012
//
///////////////////////////////////////////////////////////////
//
// QPESumVectors2Mdv reads netCDF data and writes Mdv files
//
///////////////////////////////////////////////////////////////////////

#include "QPESumVectors2Mdv.hh"

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/Ncf2MdvTrans.hh>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
#include <toolsa/TaArray.hh>

using namespace std;

//
// Constructor
//
QPESumVectors2Mdv::QPESumVectors2Mdv(int argc, char **argv):
_progName("QPESumVectors2Mdv"),
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
  _paramsPath = (char *) "unknown";
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
		300);

  return;

}

//////////////////////////////////////////////////////
//
// destructor
//
QPESumVectors2Mdv::~QPESumVectors2Mdv()

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
void QPESumVectors2Mdv::_clear()

{

}



//////////////////////////////////////////////////
// 
// Run
//
int QPESumVectors2Mdv::Run ()
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
      
      fileTrigger = new DsInputPath( _progName, _params.debug,
				     _params.input_dir,
				     _params.max_valid_realtime_age_min*60,
				     PMU_auto_register,
				     _params.ldata_info_avail,
				     false );
      //
      // Set wait for file to be written to disk before being served out.
      //
      fileTrigger->setFileQuiescence( _params.file_quiescence_sec );
      
      //
      // Set time to wait before looking for new files.
      // 
      fileTrigger->setDirScanSleep( _params.check_input_sec );
      
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
	  
	  fileTrigger = new DsInputPath( _progName, _params.debug , fileList );
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
       
       fileTrigger = new DsInputPath( _progName, _params.debug,
				      _params.input_dir,
				      time_begin,
				      time_end);
     }


  //
  //
  // process data
  //
  char *inputPath;

  while ( (inputPath = fileTrigger->next()) != NULL)
    {
     
      if (_processData(inputPath))
	{
          cerr << "Error - QPESumVectors2Mdv::Run" << endl;
          cerr << "  Errors in processing time: " <<  inputPath << endl;
	  return 1;
        }
    } // while
  
  delete fileTrigger;

  return 0;
  

}


///////////////////////////////////
//
//  process data at trigger time
//
int QPESumVectors2Mdv::_processData(char *inputPath)

{
  if (_params.debug)
    {
      cerr << "QPESumVectors2Mdv::_processData: Processing file : " << inputPath << endl;
    }
  
  // registration with procmap

  PMU_force_register("Processing data");

  // create translator

  Ncf2MdvTrans trans;
  trans.setDebug(_params.debug);

  // perform translation
  // returns 0 on success, -1 on failure

  DsMdvx mdv;

  for (int i = 0; i < _params.field_names_n; i++)
    mdv.addReadField(_params._field_names[i]);

  //
  // open the Nc File
  //

  _ncFile = new NcFile(inputPath, NcFile::ReadOnly);

  //
  // Check that constructor succeeded
  //
  if (!_ncFile->is_valid())
  {
    cerr << inputPath << " could not be opened!\n";
    return -1;
  }
  if (_params.debug) {
    cerr << "SUCCESS - opened file: " << inputPath << endl;
  }

  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program. In the case of this example, we just exit with
  // an NC_ERR error code.

  NcError err(NcError::silent_nonfatal);
  
  //
  // Get the number of variables
  //
  _numVars = _ncFile->num_vars();

  //
  // Get the number of dimensions
  //
  _numDims = _ncFile->num_dims();

  //
  // Get the number of attributes
  //
  int numAtts = _ncFile->num_atts();

  NcAtt* globalAtt = NULL;

  //
  // Loop through the attributes and grab the ones we are expecting.
  //
  char *_fieldName;
  char *_gridType;
  char *_units;
  int _unixTime;
  double _dx;
  double _dy;
  double _dz = 1;
  double _minx;
  double _miny;
  double _minz = 1;
  
  for (int i = 0; i < numAtts; i++)
  {
    globalAtt = _ncFile->get_att(i);

    if (!globalAtt->is_valid())
      continue;

    if ( strcmp( globalAtt->name(), "TypeName") == 0 )
      _fieldName = globalAtt->as_string(0);
    
    if ( strcmp( globalAtt->name(), "DataType") == 0 )
      _gridType = globalAtt->as_string(0);

    if ( strcmp( globalAtt->name(), "Time") == 0 )
      _unixTime = globalAtt->as_int(0);
    
    if ( strcmp( globalAtt->name(), "Unit-value") == 0 )
      _units = globalAtt->as_string(0);
    
    if (strcmp( globalAtt->name(), "LatGridSpacing") == 0 )
      _dx = globalAtt->as_float(0);

    if (strcmp( globalAtt->name(), "LonGridSpacing") == 0 )
      _dy = globalAtt->as_float(0);
      
    if (strcmp( globalAtt->name(), "Latitude") == 0 )
      _miny = globalAtt->as_float(0);
    

    if (strcmp( globalAtt->name(), "Longitude") == 0 )
      _minx = globalAtt->as_float(0);

  }
  
  mdv.clearMasterHeader();

  Mdvx::master_header_t mhdr;
  mhdr = mdv.getMasterHeader();

  mdv.clearFields();
  mdv.clearChunks();
  mdv.clearErrStr();
  //  mdv._pathInUse = inputPath;

  DateTime data_time(_unixTime);
  
  Mdvx::master_header_t master_hdr;
  
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.user_time = 0;
  master_hdr.time_begin = data_time.utime() - 150;
  master_hdr.time_end = data_time.utime() + 150;
  master_hdr.time_centroid = data_time.utime();
  master_hdr.time_expire = master_hdr.time_end;

  master_hdr.num_data_times = 0;
  master_hdr.index_number = 0;
  master_hdr.data_dimension = 3;

  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  master_hdr.user_data = 0;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  master_hdr.vlevel_included = 1;

  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;

  master_hdr.n_fields = 0;

  master_hdr.n_chunks = 0;
  master_hdr.field_grids_differ = 0;

  master_hdr.sensor_lon = 0;
  master_hdr.sensor_lat = 0;
  master_hdr.sensor_alt = 0.0; 
  
  STRcopy(master_hdr.data_set_info, "Output from QPESumVectors2Mdv_CWB", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "QPESumVectors2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, inputPath,
          MDV_NAME_LEN);

  mdv.setMasterHeader(master_hdr);
  

  NcVar* var;

  int _nx;
  int _ny;
  int _nz = 1;
  
  for (int jj = 0; jj < _numVars; jj++)
  {
    var = _ncFile->get_var(jj);

    for (int i = 0; i < _numDims; i++)
    {
      NcDim* dim = var->get_dim(i);
      if (strcmp( dim->name(), "Lat") == 0 )
	_ny = dim->size();

      if (strcmp( dim->name(), "Lon") == 0 )
	_nx = dim->size();

    }

    // need to calculate miny 
    _miny = _miny - (_ny * _dy);

    if (_params.debug) 
    {
      cerr << "_fieldName = " << _fieldName << endl;
      cerr << "_units = " << _units << endl;
      cerr << "_unixTime = " << _unixTime << endl;
      cerr << "_gridType = " << _gridType << endl;
      cerr << "_dx = " << _dx << endl;
      cerr << "_dy = " << _dy << endl;
      cerr << "_minx = " << _minx << endl;
      cerr << "_miny = " << _miny << endl;
      cerr << "_nx = " << _nx << endl;
      cerr << "_ny = " << _ny << endl;
    }
  
    // Construct the field header
    Mdvx::field_header_t field_hdr;
  
    memset(&field_hdr, 0, sizeof(field_hdr));

    field_hdr.forecast_time = master_hdr.time_centroid;
    field_hdr.nx = _nx;
    field_hdr.ny = _ny;
    field_hdr.nz = 1;

    // For now going to assume latlon projection
    // Can use the projection field from the header
    // but so far all the data is LL.
    field_hdr.proj_type = Mdvx::PROJ_LATLON;

    field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
    field_hdr.data_element_nbytes = 4;
    field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
    field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
    field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
    field_hdr.scaling_type = Mdvx::SCALING_NONE;
    field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
    field_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
    field_hdr.dz_constant = 1;
    field_hdr.proj_origin_lat = _miny;
    field_hdr.proj_origin_lon = _minx;
    field_hdr.grid_dx = _dx;
    field_hdr.grid_dy = _dy;
    field_hdr.grid_dz = 1;
    field_hdr.grid_minx = _minx;
    field_hdr.grid_miny = _miny;
    field_hdr.grid_minz = 1;
    field_hdr.scale = 1;
    field_hdr.bias =  0;
    field_hdr.bad_data_value = -999.0;
    field_hdr.missing_data_value = -999.0;
    //  field_hdr.proj_rotation = _params->input_proj.rotation;
    STRcopy(field_hdr.field_name_long,_fieldName,MDV_LONG_FIELD_LEN);
    STRcopy(field_hdr.field_name,_fieldName,MDV_SHORT_FIELD_LEN);
    STRcopy(field_hdr.units,_units,MDV_UNITS_LEN);
  
    // Construct the vlevel header

    Mdvx::vlevel_header_t vlevel_hdr;
  
    memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));

    for (int ii = 0; ii < _nz; ii++)
    {
      if( _nz == 1)
      {
	vlevel_hdr.type[ii] = Mdvx::VERT_TYPE_SURFACE;
      }
      else 
      {
	vlevel_hdr.type[ii] = Mdvx::VERT_TYPE_Z;
      }
      vlevel_hdr.level[ii] = _minz + ( ii * _dz );
    }
  

    // allocate space and grab the data
    fl32 *data = new fl32[ _nx * _ny];
    fl32 *flip_data = new fl32[ _nx * _ny];


    //    data = (fl32*)var->values();

    if (var->get(data, _ny,_nx) == 0)
    {
      cerr << "Unable to get data array\n";
      return -1;
    }

    // I believe I need to flip the order of the data
    int i=0;
    
    if((string)_fieldName == "Motion_South")
    {
      cerr <<"Changing sign on field " << _fieldName << endl;

      for (int y = _ny; y > 0; y--)
      {
	for (int x = 1; x <= _nx; x++)
	{
	  flip_data[i] = - ( data[ ( y * _nx ) - (_nx - x) ] );
	  i++;
	}
      }
    }
    else
    {
      for (int y = _ny; y > 0; y--)
      {
	for (int x = 1; x <= _nx; x++)
	{
	  flip_data[i] = data[ ( y * _nx ) - (_nx - x) ];
	  i++;
	}
      }
    }
      
    delete[] data;

    MdvxField *output_field;

    if ((output_field =
	 new MdvxField(field_hdr,vlevel_hdr,flip_data)) == 0)
    {
      cerr << "Error creating output field" << endl;
      return -1;
    }

    output_field->convertType(Mdvx::ENCODING_FLOAT32,
			      Mdvx::COMPRESSION_GZIP);
    mdv.addField(output_field);

  }
  

  // write to file

  if (_writeData( mdv) ) {
    cerr << "ERROR - QPESumVectors2Mdv::_writeData()" << endl;
    cerr << "  Cannot translate file: " <<  inputPath << endl;
    return -1;
  }

  return 0;

}

int QPESumVectors2Mdv::_writeData( DsMdvx &mdv)
{
  
  const Mdvx::master_header_t &mhdr = mdv.getMasterHeader();

  if (mhdr.num_data_times <= 1){

    if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
	mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
	mhdr.forecast_time != 0) {
      mdv.setWriteAsForecast();
    }

    if (mdv.writeToDir(_params.output_url)) {
      cerr << "ERROR - QPESumVectors2Mdv::_writeData()" << endl;
      cerr << "  Cannot write mdv file" << endl;
      cerr << mdv.getErrStr() << endl;
      return -1;
    }
    
    if (_params.debug) {
      cerr << "Wrote file: " << mdv.getPathInUse() << endl;
    }

  } else {

    if (_params.debug) {
      mdv.setDebug(true);
    }
    
    if (mdv.writeMultForecasts(_params.output_url)) {
      cerr << "ERROR - QPESumVectors2Mdv::_writeData()" << endl;
      cerr << "  Cannot write mult forecasts to url: "
           << _params.output_url << endl;
      cerr << mdv.getErrStr() << endl;
      return -1;
    }
    
  }

  return 0;

}
