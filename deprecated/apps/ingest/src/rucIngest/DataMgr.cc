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
//
// $Id: DataMgr.cc,v 1.46 2016/03/07 01:23:10 dixon Exp $
//
/////////////////////////////////////////////////

#include <cstring>

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/DateTime.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>

#include "GribField.hh"
#include "DataMgr.hh"
#include "GribMgr.hh"
#include "Ingester.hh"
#include "FilelistInputStrategy.hh"
#include "LdataInputStrategy.hh"
#include "RealtimeDirInputStrategy.hh"
#include "OutputFile.hh"
#include "RucIngest.hh"
using namespace std;

//
// Constants
//
const int DataMgr::MAX_LINE = 256;
const int DataMgr::MAX_NPTS = 10000000;
const double DataMgr::M_TO_KM = 0.001;
const double DataMgr::M_TO_100FT = .0328;
const double DataMgr::MPS_TO_KNOTS = 1.94;
const double DataMgr::PASCALS_TO_MBARS = 0.01;
const double DataMgr::KELVIN_TO_CELCIUS = -273.15;
const double DataMgr::KG_TO_G = 1000.0;
const double DataMgr::PERCENT_TO_FRAC = 0.01;

DataMgr::DataMgr() :
  _paramsPtr(0),
  _inputPjg(0),
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
  delete _ingester;
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

   default:
     cerr << "ERROR: " << method_name << endl;
     cerr << "Invalid mode given for realtime processing" << endl;
     return false;
   } /* endswitch - _paramsPtr->mode */

   _ingester   = new Ingester( *_paramsPtr );
   _missingVal = _ingester->getMissingVal();
   _gribMgr = _ingester->getGribMgr();


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
      if( _ingester->processFile( filePath ) != 0 ) {
	cerr << "WARNING: File " << filePath << " not inventoryed." << endl << flush;
	continue;
      }


      //
      // apply range limts
      //
      _limitRanges();

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
	cerr << "ERROR: Could not creating MdvxField objects." << endl << flush;
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

      //
      // Write the mdv file
      //
      //      PMU_auto_register( "Writing mdv file" );
      PMU_force_register( "Writing mdv file" );
      if (_paramsPtr->debug)
	cout << "Writing mdv file." << endl << flush;
      if( !_writeMdvFile() ) {
	cerr << "ERROR: Could not write MDV file." << endl << flush;
	return false;
      }
      
      _ingester->cleanup();
      
   }
   
   return true;
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

  //
  // Set up output file 
  //
  _outputFile = new OutputFile( _paramsPtr );

  return true;
}



bool
DataMgr::_writeMdvFile()
{
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
	(*mfi)->remap2Lc2(lut, _paramsPtr->output_grid.nx, _paramsPtr->output_grid.ny, 
			  _paramsPtr->output_grid.minx, _paramsPtr->output_grid.miny, 
			  _paramsPtr->output_grid.dx, _paramsPtr->output_grid.dy,
			  _paramsPtr->output_origin.lat, _paramsPtr->output_origin.lon,
			  _paramsPtr->output_parallel.lat1, _paramsPtr->output_parallel.lat2 );
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

  //
  // clear out the MdvxField vector
  //
  _clearMdvxFields();

  //
  // copy contents of the GribField objects over to MdvxField objects
  //
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
    fieldHeader.scaling_type        = Mdvx::SCALING_NONE;
    fieldHeader.native_vlevel_type  = (*gfi)->getVerticalLeveltype();
    fieldHeader.vlevel_type         = (*gfi)->getVerticalLeveltype();
    fieldHeader.dz_constant         = 1;

    fieldHeader.proj_origin_lat     = pjg->getOriginLat();
    fieldHeader.proj_origin_lon     = pjg->getOriginLon();
    fieldHeader.vert_reference      = 0;
    fieldHeader.grid_dx             = pjg->getDx();
    fieldHeader.grid_dy             = pjg->getDy();
    fieldHeader.grid_dz             = pjg->getDz();
    fieldHeader.grid_minx           = pjg->getMinx();
    fieldHeader.grid_miny           = pjg->getMiny();
    fieldHeader.grid_minz           = pjg->getMinz();
    fieldHeader.scale               = 1.0;
    fieldHeader.bias                = 0.0;
    fieldHeader.bad_data_value      = -9998.0;
    fieldHeader.missing_data_value  = -9999.0;
    fieldHeader.proj_rotation       = 0.0;
    fieldHeader.proj_type = Mdvx::PROJ_LAMBERT_CONF;
    fieldHeader.proj_param[0] = _gribMgr->getLatin1();
    fieldHeader.proj_param[1] = _gribMgr->getLatin2();


    //
    // fill out the vlevel header
    //
    Mdvx::vlevel_header_t vlevelHeader;
    memset( (void *) &vlevelHeader, (int) 0, sizeof(Mdvx::vlevel_header_t) );
    vlevelHeader.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE;
    for( int iz = 0; iz < fieldHeader.nz; iz++ ) {
      vlevelHeader.type[iz] = (*gfi)->getVerticalLeveltype();
      vlevelHeader.level[iz] = fieldHeader.grid_minz + fieldHeader.grid_dz * iz;
    }
    

    //
    // create the MdvxField object
    //
    _outputFields.push_back(new MdvxField(fieldHeader, vlevelHeader, 
				  (void *) (*gfi)->getData() ) );
    _outputFields.back()->setFieldName( (*gfi)->getName().c_str() );
    _outputFields.back()->setFieldNameLong( (*gfi)->getLongName().c_str() );
    _outputFields.back()->setUnits( (*gfi)->getUnits().c_str() );
    
  }

  return true;
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
    case Params::KELVIN_TO_CELCIUS:
      (*gfi)->setUnits("C");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr += KELVIN_TO_CELCIUS;
      }
      break;
    case Params::KGPKG_TO_GPKG:
      (*gfi)->setUnits("g/kg");
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {
	*dPtr *= KG_TO_G;
      }
      break;
    case Params::PERCENT_TO_FRACTION:
      (*gfi)->setUnits("%");
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


void
DataMgr::_limitRanges() 
{
  const list<GribField*>& gribFieldList = _ingester->getGribFieldList();
  for( list<GribField*>::const_iterator gfi = gribFieldList.begin(); 
       gfi != gribFieldList.end(); gfi++ ) {

    float upperLimit = (*gfi)->getUpperLimit();
    float lowerLimit = (*gfi)->getLowerLimit();

    if((upperLimit == 0.0) && (lowerLimit == 0.0)) {
      continue;
    }

    fl32* dPtr = (*gfi)->getData();
    
      for(int i = 0; i < (*gfi)->getNumPts(); i++,dPtr++) {

	if((*dPtr < lowerLimit) || (*dPtr > upperLimit)) {
	  *dPtr = -9999.0;
	}

      }
  }
}
