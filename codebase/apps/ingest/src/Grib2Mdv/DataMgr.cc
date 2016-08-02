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
/////////////////////////////////////////////////
// Data Manager
/////////////////////////////////////////////////

#include "DataMgr.hh"

#include <cstring>

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>
#include <physics/physics.h>

#include "GribField.hh"
#include "GribMgr.hh"
#include "Ingester.hh"
#include "FilelistInputStrategy.hh"
#include "LdataInputStrategy.hh"
#include "RealtimeDirInputStrategy.hh"
#include "NewFilesInputStrategy.hh"
#include "OutputFile.hh"
#include "Grib2Mdv.hh"
using namespace std;

//
// Constants
//
const int DataMgr::MAX_LINE = 256;
const int DataMgr::MAX_NPTS = 20000000;
const double DataMgr::M_TO_KM = 0.001;
const double DataMgr::M_TO_100FT = .0328;
const double DataMgr::MPS_TO_KNOTS = 1.94;
const double DataMgr::PASCALS_TO_MBARS = 0.01;
const double DataMgr::KELVIN_TO_CELSIUS = -273.15;
const double DataMgr::KG_TO_G = 1000.0;
const double DataMgr::PERCENT_TO_FRAC = 0.01;

DataMgr::DataMgr() :
  _paramsPtr(0),
  _inputPjg(0),
  _gribMgr(0),
  _inputStrategy(0),
  _ingester(0),
  _outputFile(0)
{
}

DataMgr::~DataMgr()
{
  delete _inputStrategy;
  delete _inputPjg;
  delete _outputFile;

  if (_ingester != NULL) {
    _ingester->tearDown();
    delete _ingester;
  }

  _gribMgr = 0; // release pointer to the GRIB manager

  _clearMdvxFields();

}

bool 
DataMgr::init( Params &params )
{
  static const string method_name = "DataMgr::init()";

   //
   // Set pointer to params
   //
   _paramsPtr = &params;
   
   //
   // Set input file related members
   //
   NewFilesInputStrategy *tempStrategy = NULL;

   switch (_paramsPtr->mode)
   {
   case Params::REALTIME :
     _inputStrategy =
       new LdataInputStrategy(_paramsPtr->input_dir,
			      _paramsPtr->max_input_data_age,
			      (LdataInputStrategy::heartbeat_t)PMU_auto_register,
			      _paramsPtr->debug);
     break;

   case Params::REALTIME_DIR :
     _inputStrategy =
       new RealtimeDirInputStrategy(_paramsPtr->input_dir,
				    _paramsPtr->input_substring,
				    _paramsPtr->max_input_data_age,
				    (RealtimeDirInputStrategy::heartbeat_t)PMU_auto_register,
				    _paramsPtr->debug);
     break;

   case Params::NEWFILES :
     tempStrategy =
       new NewFilesInputStrategy(_paramsPtr->input_dir,
				    _paramsPtr->input_substring,
				    _paramsPtr->max_input_data_age,
				    (NewFilesInputStrategy::heartbeat_t)PMU_auto_register,
                                    _paramsPtr->process_old_files,
				    _paramsPtr->debug);
     tempStrategy->init();
     _inputStrategy = tempStrategy;
     break;

   default:
     cerr << "ERROR: " << method_name << endl;
     cerr << "Invalid mode given for realtime processing" << endl;
     return false;
   } /* endswitch - _paramsPtr->mode */

   _ingester   = new Ingester( *_paramsPtr );
   _ingester->setup(_paramsPtr->input_grib_type);
   _missingVal = _ingester->getMissingVal();
   _gribMgr = _ingester->getGribMgr();

   // Set mdv related members if necessary
   if( _paramsPtr->write_forecast || _paramsPtr->write_non_forecast ) {
      if( !_mdvInit() ) {
	return false;
      }
   }

   return true;
   
}

bool 
DataMgr::init( Params &params, const vector<string>& fileList )
{
   //
   // Set pointer to params
   //
   _paramsPtr = &params;
   
   //
   // Set input file related members
   // 
   _inputStrategy = new FilelistInputStrategy(fileList,
					      _paramsPtr->debug);

   _ingester   = new Ingester( *_paramsPtr );
   _ingester->setup(_paramsPtr->input_grib_type);
   _missingVal = _ingester->getMissingVal();
   _gribMgr = _ingester->getGribMgr();


   //
   // Check that all fields are present if user requests vertical velocity in m/s
   // This task is essentially creating a derived field in that 
   //
   // Set mdv related members if necessary
   //
   if( _paramsPtr->write_forecast || _paramsPtr->write_non_forecast ) {
      if( !_mdvInit() ) {
	 return false;
      }
   }

   return true;
   
}


bool 
DataMgr::getData()
{
   string filePath;

   //
   // Process files
   //
   while( true ) {
     if( (filePath = _inputStrategy->next()) == "" )
     {
       if (_paramsPtr->mode == Params::REALTIME)
       {
	 sleep(10);
	 continue;
       }
       else
       {
	 break;
       }
     }

      //
      // Process the file
      // 
      //      PMU_auto_register( "Processing file" );
      PMU_force_register( "Processing file" );
      if (_paramsPtr->debug)
	cout << "File " << filePath << " will be processed." << endl << flush;
      assert(_ingester != NULL);
      if( _ingester->processFile( filePath ) != 0 ) {
	cerr << "WARNING: File " << filePath << " not inventoryed." << endl << flush;
	continue;
      }


      //
      // Convert units
      //
      _convertUnits();

      //
      // Create MdvxField objects for the outputFile
      //
      if (_paramsPtr->debug)
	cout << "Creating MdvxField objects." << endl << flush;
      if( !_createMdvxFields() ) {
	cerr << "ERROR: Could not create MdvxField objects." << endl << flush;
	return false;
      }

      //
      // Remap the data onto the output grid
      //
      //      PMU_auto_register( "Remapping data" );
      PMU_force_register( "Remapping data" );
      if (_paramsPtr->debug)
	cout << "Remapping data." << endl << flush;
      if( !_remapData() ) {
	cerr << "WARNING: Could not remap the data in file "<< filePath << "." << endl << flush;
	continue;
      }

      // Calculate derived fields
      _deriveFields();

      // remove fields that were needed only to derive another output
      _removeNonRequestedFields();

      if (_paramsPtr->debug) {
        for( vector<MdvxField*>::iterator mfi = _outputFields.begin();
          mfi != _outputFields.end(); mfi++ ) {
          (*mfi)->printHeaders(cout);

          MdvxProj proj((*mfi)->getFieldHeader());
          proj.print(cout);
        }
      }

      //
      // Write the mdv file
      //
      //      PMU_auto_register( "Writing mdv file" );
      PMU_force_register( "Writing mdv file" );
      if (_paramsPtr->debug)
	cout << "Writing mdv file." << endl << flush;
      if( !_writeMdvFile() ) {
	cerr << "ERROR: Could not write MDV file." << endl << flush;
        // We have to keep going.  Probably the cause was an incomplete file,
        // and we only loaded a few records (less than 10).
      }
      
      _ingester->cleanup();
      
   }
   
   return(0);
}


bool
DataMgr::_mdvInit() 
{
  //
  // Make sure other member functions will know that we
  // want to create mdv files
  //
  _createMdv = true;


  //
  // Check output grid variables
  //
  if (_paramsPtr->mdv_proj_type != Params::OUTPUT_PROJ_NATIVE)
  {
    if( _paramsPtr->output_grid.nx <= 0 || _paramsPtr->output_grid.ny <= 0 || 
        _paramsPtr->output_grid.nz <= 0 ||
        _paramsPtr->output_grid.dx <= 0 || _paramsPtr->output_grid.dy <= 0 || 
        _paramsPtr->output_grid.dz <= 0 ) {
      cerr << "ERROR: Do not select zero for nx, ny, nz, dx, dy or dz. "<< endl << flush;
      return false;
    }

    if( _paramsPtr->output_grid.nx * _paramsPtr->output_grid.ny * 
        _paramsPtr->output_grid.nz > MAX_NPTS ) {
      cerr << "ERROR: Output grid too big" << endl << flush;
      return false;
    }
  }

  //
  // Set up output file 
  //
  _outputFile = new OutputFile( _paramsPtr );

  return true;
}



bool
DataMgr::_writeMdvFile()
{
  assert(_ingester != NULL);
  time_t generateTime = _ingester->getGenerateTime();
  int forecastTime = _ingester->getForecastTime();
  
  if( (generateTime < 0) || (forecastTime < 0)) {
    cerr << " WARNING: File times don't make sense" << endl << flush;
    return true;
  }
    
  //
  // Tell the user what we're doing
  //
  if(_paramsPtr->debug) {
    DateTime genTime( generateTime );
    cout << "Writing grid output file at " << genTime.dtime() << " for a forecast time of " \
	 << (forecastTime/3600) << " hours" << endl << flush;
  }

  //
  // Prepare the file
  //
  for( vector<MdvxField*>::iterator mfi = _outputFields.begin(); mfi != _outputFields.end(); 
       mfi++ ) {
    _outputFile->addField( *mfi );

   //
   // release ownership of MdvxField object
   //
    *mfi = 0;
  }

  //
  // Write out the file 
  //
  if ( _outputFile->writeVol( generateTime, forecastTime ) != 0 ) {
    return false;
  }
  //
  // Clear the file for next time
  //
  _outputFile->clear();

  return true;
}

bool
DataMgr::_remapData() 
{

  // if the output projection type set to be something other
  // than OUTPUT_PROJ_NATIVE, then remap the data
  if(_paramsPtr->mdv_proj_type != Params::OUTPUT_PROJ_NATIVE) {
    for( vector<MdvxField*>::iterator mfi = _outputFields.begin(); 
	 mfi != _outputFields.end(); mfi++ ) {
      MdvxRemapLut lut;
      switch( _paramsPtr->mdv_proj_type ) {
      case Params::OUTPUT_PROJ_FLAT:
	(*mfi)->remap2Flat(lut, _paramsPtr->output_grid.nx, _paramsPtr->output_grid.ny, 
			   _paramsPtr->output_grid.minx, _paramsPtr->output_grid.miny, 
			   _paramsPtr->output_grid.dx, _paramsPtr->output_grid.dy, 
			   _paramsPtr->output_origin.lat, _paramsPtr->output_origin.lon,
			   _paramsPtr->output_rotation);
	break;
	  
      case Params::OUTPUT_PROJ_LATLON:
	(*mfi)->remap2Latlon(lut, _paramsPtr->output_grid.nx, _paramsPtr->output_grid.ny, 
			     _paramsPtr->output_grid.minx, _paramsPtr->output_grid.miny, 
			     _paramsPtr->output_grid.dx, _paramsPtr->output_grid.dy );
	break;

      case Params::OUTPUT_PROJ_LAMBERT_CONF:
	assert((*mfi)->remap2Lc2(lut, _paramsPtr->output_grid.nx, _paramsPtr->output_grid.ny, 
			  _paramsPtr->output_grid.minx, _paramsPtr->output_grid.miny, 
			  _paramsPtr->output_grid.dx, _paramsPtr->output_grid.dy,
			  _paramsPtr->output_origin.lat, _paramsPtr->output_origin.lon,
			  _paramsPtr->output_parallel.lat1, _paramsPtr->output_parallel.lat2 ) == 0);
	break;
      case Params::OUTPUT_PROJ_NATIVE:
	break;
      }
    }

  }
  return true;
}

bool
DataMgr::_createMdvxFields() 
{

  // The Grib Polar Stereographic projection assumes that the projection plane
  // cuts the earth at 60 degrees North (or South).  Our projection libraries
  // assume that the projection plane cuts the earth at 90 degrees north.
  // Therefore, the grid length (cell size) reported by grib must be enlarged
  // to reflect this increased distance away from the projection point (the
  // South pole).  Carl Drews - October 29, 2005
  double polarStereoAdjustment = 2.0 / (1.0 + sin(60.0 * PI / 180.0));

  //
  // clear out the MdvxField vector
  //
  _clearMdvxFields();

  //
  // copy contents of the GribField objects over to MdvxField objects
  //
  assert(_ingester != NULL);
  const list<GribField*>& gribFieldList = _ingester->getGribFieldList();
  for( list<GribField*>::const_iterator gfi = gribFieldList.begin(); 
       gfi != gribFieldList.end(); gfi++ ) {

    //
    // pass along vertical level info to the OutputFile object for the master header
    //
    if ( (*gfi)->getNz() > 1 ) {
      _outputFile->setVerticalType( (*gfi)->getVerticalLeveltype() );  
    }  

    //
    // fill out the field header
    //
    Pjg *pjg = (*gfi)->getProjection();
    assert(pjg != NULL);

    Mdvx::field_header_t fieldHeader;
    memset( (void *) &fieldHeader, (int) 0, sizeof(Mdvx::field_header_t) );
    fieldHeader.record_len1         = sizeof( Mdvx::field_header_t );
    fieldHeader.struct_id           = Mdvx::FIELD_HEAD_MAGIC_COOKIE;
    fieldHeader.field_code          = (*gfi)->getParameterId();
    fieldHeader.forecast_delta      = (*gfi)->getForecastTime();
    fieldHeader.forecast_time       = (*gfi)->getGenerateTime() + (*gfi)->getForecastTime();
    fieldHeader.nx                  = (*gfi)->getNx();
    fieldHeader.ny                  = (*gfi)->getNy();
    fieldHeader.nz                  = (*gfi)->getNz();
    fieldHeader.encoding_type       = Mdvx::ENCODING_FLOAT32;
    fieldHeader.data_element_nbytes = sizeof(fl32);
    fieldHeader.field_data_offset   = 0;
    fieldHeader.volume_size         = fieldHeader.nx * fieldHeader.ny * 
      fieldHeader.nz * fieldHeader.data_element_nbytes;
    fieldHeader.compression_type    = Mdvx::COMPRESSION_ASIS;
    fieldHeader.transform_type      = Mdvx::DATA_TRANSFORM_NONE;
    fieldHeader.scaling_type        = Mdvx::SCALING_DYNAMIC;
    fieldHeader.native_vlevel_type  = (*gfi)->getVerticalLeveltype();
    fieldHeader.vlevel_type         = (*gfi)->getVerticalLeveltype();
    fieldHeader.dz_constant         = (*gfi)->getDzConstant();

    // save the grib level ID so we can check requested later
    fieldHeader.unused_si32[0]      = (*gfi)->getLevelId();

    fieldHeader.proj_origin_lat     = pjg->getOriginLat();
    fieldHeader.proj_origin_lon     = wrapAdjust(pjg->getOriginLon(), -180.0, 180.0);

    fieldHeader.vert_reference      = 0;
    fieldHeader.scale               = 1.0;
    fieldHeader.bias                = 0.0;
    fieldHeader.bad_data_value      = _missingVal;
    fieldHeader.missing_data_value  = _missingVal;

    fieldHeader.proj_param[0] = _gribMgr->getLatin1();
    fieldHeader.proj_param[1] = _gribMgr->getLatin2();

    // The grib1 convention is that Lambert lat1 and lat2 are ordered
    // from the pole to the equator.  The MDV convention is that
    // Lambert lat1 and lat2 are ordered from the equator to the
    // pole.  Reverse them here to correspond to the MDV convention.
    if (fabs(fieldHeader.proj_param[0]) > fabs(fieldHeader.proj_param[1])) {
       // swap lat1 and lat2
       double swapLat = fieldHeader.proj_param[0];
       fieldHeader.proj_param[0] = fieldHeader.proj_param[1];
       fieldHeader.proj_param[1] = swapLat;
    }
 
    if (_paramsPtr->overrideGeom.override) {
      //
      // Take projection information from TDRP input
      //
      fieldHeader.grid_dx = _paramsPtr->overrideGeom.dx;
      fieldHeader.grid_dy = _paramsPtr->overrideGeom.dy;
      fieldHeader.grid_dz = _paramsPtr->overrideGeom.dz;

      fieldHeader.grid_minx = _paramsPtr->overrideGeom.minx;
      fieldHeader.grid_miny = _paramsPtr->overrideGeom.miny;
      fieldHeader.grid_minz = _paramsPtr->overrideGeom.minz;

      fieldHeader.proj_rotation = _paramsPtr->overrideGeom.rotation;

      fieldHeader.proj_param[0] = _paramsPtr->overrideGeom.projParam0;
      fieldHeader.proj_param[1] = _paramsPtr->overrideGeom.projParam1;

      switch ( _paramsPtr->overrideGeomProj){

      case Params::PROJ_LAMBERT_CONF :
	fieldHeader.proj_type = Mdvx::PROJ_LAMBERT_CONF;
	break;

      case Params::PROJ_LATLON :
	fieldHeader.proj_type = Mdvx::PROJ_LATLON;
	break;

      case Params::PROJ_POLAR_STEREO :
	fieldHeader.proj_type = Mdvx::PROJ_POLAR_STEREO;
	break;

      case Params::PROJ_OBLIQUE_STEREO :
	fieldHeader.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
	break;

      case Params::PROJ_FLAT :
	fieldHeader.proj_type = Mdvx::PROJ_FLAT;
	break;

      default :
	cerr << "Unrecognized override geometry : " << _paramsPtr->overrideGeomProj << endl;
	exit(-1);
	break;

      }

    } else {
      //
      // Take projection from input file.
      //
      fieldHeader.grid_dx             = pjg->getDx();
      fieldHeader.grid_dy             = pjg->getDy();
      fieldHeader.grid_dz             = pjg->getDz();
      fieldHeader.grid_minx           = pjg->getMinx();
      fieldHeader.grid_miny           = pjg->getMiny();
      fieldHeader.grid_minz           = pjg->getMinz();
      fieldHeader.proj_rotation       = pjg->getRotation();
      

      if (pjg->getProjType() == PjgTypes::PROJ_LATLON) {
	fieldHeader.proj_type = Mdvx::PROJ_LATLON;
	
      } else if (pjg->getProjType() == PjgTypes::PROJ_POLAR_STEREO) {
	fieldHeader.proj_type = Mdvx::PROJ_POLAR_STEREO;
	
	fieldHeader.grid_dx *= polarStereoAdjustment;
	fieldHeader.grid_dy *= polarStereoAdjustment;
	
	fieldHeader.proj_param[0] = pjg->getRotation();
	fieldHeader.proj_param[1] = pjg->getPole() == PjgTypes::POLE_NORTH
	  ? Mdvx::POLE_NORTH : Mdvx::POLE_SOUTH;
	
      } else {
	// default to Lambert Conformal
	fieldHeader.proj_type = Mdvx::PROJ_LAMBERT_CONF;
      }
    }

    //
    // fill out the vlevel header
    //
    Mdvx::vlevel_header_t vlevelHeader;
    memset( (void *) &vlevelHeader, (int) 0, sizeof(Mdvx::vlevel_header_t) );
    vlevelHeader.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE;
    for( int iz = 0; iz < fieldHeader.nz; iz++ ) {
      vlevelHeader.type[iz] = (*gfi)->getVerticalLeveltype();
      vlevelHeader.level[iz] = (*gfi)->getLevel(iz);
    }
    

    // create the MdvxField object
    _outputFields.push_back(new MdvxField(fieldHeader, vlevelHeader, 
				  (void *) (*gfi)->getData() ) );
    string fieldName = _gribMgr->uniqueFieldName((*gfi)->getName(),
      (*gfi)->getLevelId());
    _outputFields.back()->setFieldName( fieldName.c_str() );
    _outputFields.back()->setFieldNameLong( (*gfi)->getLongName().c_str() );
    _outputFields.back()->setUnits( (*gfi)->getUnits().c_str() );
  }

  return true;
}


fl32 DataMgr::wrapAdjust(fl32 value, fl32 lowerBound, fl32 upperBound)
{
  assert(lowerBound < upperBound);

  // move up if necessary
  while (value < lowerBound)
    value += (upperBound - lowerBound);

  // move down if necessary
  while (value > upperBound)
    value -= (upperBound - lowerBound);

  return value;
}


void
DataMgr::_clearMdvxFields() 
{
  for( vector<MdvxField*>::iterator mfi = _outputFields.begin(); 
       mfi != _outputFields.end(); mfi++ ) {
    if(*mfi) {
      delete (*mfi);
    }
    *mfi = 0;
  }
  _outputFields.erase(_outputFields.begin(), _outputFields.end());
}


void
DataMgr::_convertUnits() 
{
  assert(_ingester != NULL);
  const list<GribField*>& gribFieldList = _ingester->getGribFieldList();
  for( list<GribField*>::const_iterator gfi = gribFieldList.begin(); 
       gfi != gribFieldList.end(); gfi++ ) {
    fl32* dPtr = (*gfi)->getData();
    switch((*gfi)->getUnitConversion()) {
    case Params::MPS_TO_KNOTS:
      (*gfi)->setUnits("knots");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr *= MPS_TO_KNOTS;
      }
      break;
    case Params::M_TO_KM:
      (*gfi)->setUnits("km");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr *= M_TO_KM;
      }
      break;
    case Params::M_TO_100FT:
      (*gfi)->setUnits("ft(x100)");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr *= M_TO_100FT;
      }
      break;
    case Params::PASCALS_TO_MBAR:
      (*gfi)->setUnits("mbar");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr *= PASCALS_TO_MBARS;
      }
      break;
    case Params::KELVIN_TO_CELSIUS:
      (*gfi)->setUnits("C");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr += KELVIN_TO_CELSIUS;
      }
      break;
    case Params::KGPKG_TO_GPKG:
      (*gfi)->setUnits("g/kg");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr *= KG_TO_G;
      }
      break;
    case Params::PERCENT_TO_FRACTION:
      (*gfi)->setUnits("none");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr *= PERCENT_TO_FRAC;
      }
      break;
    case Params::NO_CHANGE:
    default:
      break;
    }
  }
}


void DataMgr::_deriveFields()
// We calculate wind speed and direction from u- and v-wind.
// Of course the source fields have to be present.
{
  // Did the user request wind speed or direction?
  bool needWind = false;
  Params::out_field_t *windRequest = NULL;
  bool needWdir = false;
  Params::out_field_t *wdirRequest = NULL;
  for (int ofi = 0; ofi < _paramsPtr->output_fields_n; ofi++) {
    Params::out_field_t &ofref = _paramsPtr->_output_fields[ofi];
    if (ofref.param_id == Params::WIND) {
      needWind = true;
      windRequest = &ofref;
    }
    if (ofref.param_id == Params::WDIR) {
      needWdir = true;
      wdirRequest = &ofref;
    }
  }

  if (!needWind && !needWdir)
    return;

  // Do we have them already (from the grib file itself)?
  MdvxField *ugrdPtr = NULL;
  MdvxField *vgrdPtr = NULL;
  for( vector<MdvxField*>::iterator mfi = _outputFields.begin();
    mfi != _outputFields.end(); mfi++ ) {
    if ((*mfi)->getFieldHeader().field_code == Params::WIND)
      needWind = false;
    if ((*mfi)->getFieldHeader().field_code == Params::WDIR)
      needWdir = false;

    // as long as we're looping we might as well check for what we need
    if ((*mfi)->getFieldHeader().field_code == Params::UGRD)
      ugrdPtr = *mfi;
    if ((*mfi)->getFieldHeader().field_code == Params::VGRD)
      vgrdPtr = *mfi;
  }

  if (!needWind && !needWdir)
    return;

  // Do we have the source fields?
  // UGRD and VGRD do not have to be requested, but they must be present in the grib file.
  if (ugrdPtr == NULL || vgrdPtr == NULL) {
    if (_paramsPtr->debug)
      cout << "WIND or WDIR was requested, but cannot be derived because UGRD and VGRD are missing."
        << endl;
    return;
  }

  if (_paramsPtr->debug)
    cout << "Deriving fields . . ." << endl;

  // wind speed
  MdvxField *windPtr = NULL;
  if (needWind) {
    // create the new field, using UGRD as a template
    windPtr = new MdvxField(*ugrdPtr);

    // get standard grib info for wind
    MdvxFieldCode::entry_t newEntry;
    MdvxFieldCode::getEntryByCode(Params::WIND, newEntry);

    // convert code and names to wind by replacing the field header
    Mdvx::field_header_t newHeader = ugrdPtr->getFieldHeader(); // copy contents
    newHeader.field_code = Params::WIND;
    string fieldName = _gribMgr->uniqueFieldName(newEntry.abbrev, windRequest->level_id);
    strcpy(newHeader.field_name, fieldName.c_str());
    strcpy(newHeader.field_name_long, newEntry.name);
    windPtr->setFieldHeader(newHeader);

    // calculate the derived values
    if (_calculateWindFunction(ugrdPtr, vgrdPtr, windPtr, PHYwind_speed)) {
      // add field to the MDV set
      // hands off memory management to _outputFields
      _outputFields.push_back(windPtr);
      if (_paramsPtr->debug)
        windPtr->printHeaders(cout);
    } else {
      delete windPtr;
    }
  }

  // wind direction
  MdvxField *wdirPtr = NULL;
  if (needWdir) {
    // create the new field, using UGRD as a template
    wdirPtr = new MdvxField(*ugrdPtr);

    // get standard grib info for wdir
    MdvxFieldCode::entry_t newEntry;
    MdvxFieldCode::getEntryByCode(Params::WDIR, newEntry);

    // convert code and names to wdir by replacing the field header
    Mdvx::field_header_t newHeader = ugrdPtr->getFieldHeader(); // copy contents
    newHeader.field_code = Params::WDIR;
    string fieldName = _gribMgr->uniqueFieldName(newEntry.abbrev, wdirRequest->level_id);
    strcpy(newHeader.field_name, fieldName.c_str());
    strcpy(newHeader.field_name_long, newEntry.name);
    strcpy(newHeader.units, newEntry.units);
    wdirPtr->setFieldHeader(newHeader);

    // calculate the derived values
    if (_calculateWindFunction(ugrdPtr, vgrdPtr, wdirPtr, PHYwind_dir)) {
      // add field to the MDV set
      // hands off memory management to _outputFields
      _outputFields.push_back(wdirPtr);
      if (_paramsPtr->debug)
        wdirPtr->printHeaders(cout);
    } else {
      delete wdirPtr;
    }
  }

}


bool DataMgr::_calculateWindFunction(MdvxField *uWind, MdvxField *vWind,
  MdvxField *windField, double (&physicsFunction)(const double, const double))
{
  // watch out for bad input
  if (uWind == NULL || vWind == NULL || windField == NULL) {
    cerr << "Error: DataMgr::_calculateWindFunction found NULL input pointer." << endl;
    return false;
  }

  if (!_checkFieldsCompatible(uWind, vWind, windField))
    return false;

  // retrieve missing values
  fl32 uMissing = uWind->getFieldHeader().missing_data_value;
  fl32 vMissing = vWind->getFieldHeader().missing_data_value;
  fl32 fieldMissing = windField->getFieldHeader().missing_data_value;

  if (uWind->getFieldHeader().nz == vWind->getFieldHeader().nz) {
    // This way works only if the nz values are all the same.  Pity.
    if (_paramsPtr->debug)
      cout << "Same number of levels: " << uWind->getFieldHeader().nz << endl;

    // loop through all the values and calculate
    fl32 *uVal = (fl32 *)uWind->getVol();
    fl32 *vVal = (fl32 *)vWind->getVol();
    fl32 *fieldVal = (fl32 *)windField->getVol();

    int numValues = uWind->getVolNumValues();

    for (int vi = 0; vi < numValues; vi++) {

      // one missing data point means we can't calculate
      if (*uVal == uMissing || *vVal == vMissing) {
        *fieldVal = fieldMissing;
      } else {
        *fieldVal = physicsFunction(*uVal, *vVal);
      }

      // move on to the next value
      uVal++;
      vVal++;
      fieldVal++;
    }
  }

  else {
    // Different number of z-levels.  We have to match vlevels between U and V.
    if (_paramsPtr->debug)
      cout << "Different number of levels: " << uWind->getFieldHeader().nz
        << " and " << vWind->getFieldHeader().nz << endl;

    uWind->setPlanePtrs();
    vWind->setPlanePtrs();
    windField->setPlanePtrs();

    // retrieve the field headers
    const Mdvx::field_header_t &uHeader = uWind->getFieldHeader();
    const Mdvx::field_header_t &vHeader = vWind->getFieldHeader();
    const Mdvx::field_header_t &speedHeader = windField->getFieldHeader();

    // retrieve the vlevel headers
    const Mdvx::vlevel_header_t &ulevels = uWind->getVlevelHeader();
    const Mdvx::vlevel_header_t &vlevels = vWind->getVlevelHeader();

    // loop and calculate each level
    for (int uLevelIndex = 0; uLevelIndex < uHeader.nz; uLevelIndex++) {
      fl32 ulevel = ulevels.level[uLevelIndex];
      int vLevelIndex = _matchLevel(ulevel, vlevels.level, vHeader.nz);

      if (vLevelIndex >= 0) {
        // level matched - do the calculation
        // loop through all the values and calculate
        fl32 *uVal = (fl32 *)uWind->getPlane(uLevelIndex);
        fl32 *vVal = (fl32 *)vWind->getPlane(vLevelIndex);
        fl32 *fieldVal = (fl32 *)windField->getPlane(uLevelIndex);

        int numValues = windField->getPlaneSize(uLevelIndex) / speedHeader.data_element_nbytes;

        for (int vi = 0; vi < numValues; vi++) {

          // one missing data point means we can't calculate
          if (*uVal == uMissing || *vVal == vMissing) {
            *fieldVal = fieldMissing;
          } else {
            *fieldVal = physicsFunction(*uVal, *vVal);
          }

          // move on to the next value
          uVal++;
          vVal++;
          fieldVal++;
        }

      } else {
        // not matched - fill plane with missing data
        fl32 *fieldVal = (fl32 *)windField->getPlane(uLevelIndex);
        int numValues = windField->getPlaneSize(uLevelIndex) / speedHeader.data_element_nbytes;
        for (int vi = 0; vi < numValues; vi++) {
          *fieldVal = fieldMissing;
          fieldVal++;
        }
      } // single level
    } // uLevelIndex loop
  } // have to match vlevels

  // calculate min and max for the derived field
  windField->computeMinAndMax(true);

  return true;
}


int DataMgr::_matchLevel(fl32 levelValue, const fl32 levels[], int numLevels)
{
  for (int li = 0; li < numLevels; li++) {
    if (fabs((levelValue - levels[li]) / levels[li]) < 0.001)
      return li;
  }

  return -1;
}


bool DataMgr::_checkFieldsCompatible(MdvxField *uWind, MdvxField *vWind, MdvxField *windSpeed)
{
  // Run through various checks to see if we can even perform the calculation.
  if (uWind == NULL || vWind == NULL || windSpeed == NULL) {
    cerr << "Error: DataMgr::_checkFieldsCompatible found NULL input pointer." << endl;
    return false;
  }

  // retrieve the references
  const Mdvx::field_header_t &uHeader = uWind->getFieldHeader();
  const Mdvx::field_header_t &vHeader = vWind->getFieldHeader();
  const Mdvx::field_header_t &speedHeader = windSpeed->getFieldHeader();

  // levels have to be the same
  if (!(uHeader.vlevel_type == vHeader.vlevel_type
    && uHeader.vlevel_type == speedHeader.vlevel_type)) {
    if (_paramsPtr->debug) {
      cout << "Cannot _calculateWindFunction because vertical level types are different." << endl;
      cout << "U " << uHeader.vlevel_type << endl;
      cout << "V " << vHeader.vlevel_type << endl;
      cout << "S " << speedHeader.vlevel_type << endl;
    }
    return false;
  }

  // X and Y dimensions have to be the same
  if (!(uHeader.nx == vHeader.nx && uHeader.nx == speedHeader.nx
    && uHeader.ny == vHeader.ny && uHeader.ny == speedHeader.ny)) {
    if (_paramsPtr->debug) {
      cout << "Cannot _calculateWindFunction because dimensions are different." << endl;
      cout << "U " << uHeader.nx << ", " << uHeader.ny << ", " << endl;
      cout << "V " << vHeader.nx << ", " << vHeader.ny << ", " << endl;
      cout << "S " << speedHeader.nx << ", " << speedHeader.ny << ", " << endl;
    }
    return false;
  }

  // the fields cannot be compressed
  // Note: It would be possible to decompress the fields first and
  // re-compress them later, but we don't do that now.  Carl Drews - March 18, 2005
  if (!(uHeader.compression_type == Mdvx::COMPRESSION_ASIS
    || vHeader.compression_type == Mdvx::COMPRESSION_ASIS
    || speedHeader.compression_type == Mdvx::COMPRESSION_ASIS)) {
    // this is a software error, not a user error
    cerr << "Error: Cannot _calculateWindFunction because fields are compressed." << endl;
    return false;
  }

  // the fields must be floating-point
  // Note: It would be possible to calculate among mixed-encoded
  // fields, but we don't do that now.  Carl Drews - March 18, 2005
  if (!(uHeader.encoding_type == Mdvx::ENCODING_FLOAT32
    || vHeader.encoding_type == Mdvx::ENCODING_FLOAT32
    || speedHeader.encoding_type == Mdvx::ENCODING_FLOAT32)) {
    // this is a software error, not a user error
    cerr << "Error: Cannot _calculateWindFunction because fields are not floating-point." << endl;
    return false;
  }

  return true;
}


void DataMgr::_removeNonRequestedFields()
{
  // separate output fields into requested and non-requested
  vector<MdvxField *> requestedFields;
  vector<MdvxField *> nonrequested;

  // loop and categorize the fields
  for( vector<MdvxField*>::iterator mfi = _outputFields.begin();
    mfi != _outputFields.end(); mfi++ ) {
    if (_isRequested(*mfi))
      requestedFields.push_back(*mfi);
    else
      nonrequested.push_back(*mfi);
  }

  _outputFields.clear();

  // vaporize the ones we don't want
  for ( vector<MdvxField*>::iterator mfi = nonrequested.begin(); 
       mfi != nonrequested.end(); mfi++ ) {
    if(*mfi) {
      delete (*mfi);
    }
  }
  nonrequested.clear();

  // keep the ones we do want
  for ( vector<MdvxField *>::iterator mfi = requestedFields.begin();
    mfi != requestedFields.end(); mfi++) {
    _outputFields.push_back(*mfi);
  }
  requestedFields.clear();
}


bool DataMgr::_isRequested(MdvxField *theField)
{
  // are we outputting all fields?
  if (_paramsPtr->output_all_fields) {
    return true;
  }

  // retrieve the param id and level
  si32 paramId = theField->getFieldHeader().field_code;
  si32 levelId = theField->getFieldHeader().unused_si32[0];

  // check parameter against the requested output fields
  for (int ofi = 0; ofi < _paramsPtr->output_fields_n; ofi++) {
    Params::out_field_t &ofref = _paramsPtr->_output_fields[ofi];
    if (ofref.param_id == paramId && ofref.level_id == levelId) {
      return true;
    }
  }

  return false;
}

