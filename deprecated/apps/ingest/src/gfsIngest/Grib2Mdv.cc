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
/////////////////////////////////////////////////

#include <cstring>

#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>

//#include "GribField.hh"
#include "GribFile.hh"
#include "Grib2Mdv.hh"
#include "GFSrecord.hh"
#include "InputPath.hh"
#include "OutputFile.hh"
#include "GfsIngest.hh"
using namespace std;


//
// Constants
//
const int Grib2Mdv::MAX_LINE = 256;
const int Grib2Mdv::MAX_NPTS = 6000000;
const double Grib2Mdv::M_TO_KM = 0.001;
const double Grib2Mdv::M_TO_100FT = .0328;
const double Grib2Mdv::MPS_TO_KNOTS = 1.94;
const double Grib2Mdv::PASCALS_TO_MBARS = 0.01;
const double Grib2Mdv::KELVIN_TO_CELCIUS = -273.15;
const double Grib2Mdv::KG_TO_G = 1000.0;
const double Grib2Mdv::PERCENT_TO_FRAC = 0.01;

Grib2Mdv::Grib2Mdv (Params &params)
{
  _paramsPtr = &params;
  _inputPath = NULL;
  _outputFile = NULL;
  _GribFile = NULL;
  _GribRecord = NULL;
  _data = new MemBuf();

  memset( (void *) &_fieldHeader, (int) 0, sizeof(Mdvx::field_header_t) );
  memset( (void *) &_vlevelHeader, (int) 0, sizeof(Mdvx::vlevel_header_t) );
}

Grib2Mdv::~Grib2Mdv()
{
  if (_inputPath != (InputPath *)NULL)
    delete _inputPath;
  if (_outputFile != (OutputFile *)NULL)
    delete _outputFile;
  if (_GribFile != (GribFile *)NULL)
    delete _GribFile;
  if (_GribRecord!= (GFSrecord *)NULL)
    delete _GribRecord;
  _cleanup();

}

int 
Grib2Mdv::init(int nFiles, char** fileList, bool printSummary )
{
   //
   // Set input file related members
   // 
   if( nFiles > 0 ) {
      _inputPath = new InputPath( _paramsPtr, nFiles, fileList );
   }
   else {
      _inputPath = new InputPath( _paramsPtr, (MdvInput_heartbeat_t)PMU_auto_register );
   }

   _GribFile   = new GribFile (*_paramsPtr);
   _GribRecord   = new GFSrecord ();
   _printSummary = printSummary;
   //_missingVal = _GribRecord->getMissingVal();


   //
   // Set mdv related members if necessary
   //
   if( _paramsPtr->write_forecast || _paramsPtr->write_non_forecast ) {
      if( _mdvInit() != RI_SUCCESS ) {
	 return( RI_FAILURE );
      }
   }

  if (!_paramsPtr->process_everything) {
    _gribFields.insert( _gribFields.begin(), _paramsPtr->_output_fields,
                (_paramsPtr->_output_fields)+(_paramsPtr->output_fields_n));
   }
   return( RI_SUCCESS );
   
}

int 
Grib2Mdv::getData()
{
   string filePath;
   FILE *fp;
   int nZlevels = 0;
   int recLoc = 0;
   int vecLoc = 0;
   int recSize = 0;
   
   //
   // Process files
   //
   //
   while ((filePath = _inputPath->next()) != "" ) {

      //
      // Process the file
      // 
      PMU_auto_register( "Inventory file" );
      cout << "Processing file " << filePath << endl << flush;
      
      //
      // Open the file
      //
      if ((fp = ta_fopen_uncompress((char *) filePath.c_str(), "r")) == NULL) {
        cerr << "WARNING: Couldn't open file " << filePath << endl << flush;
        return(RI_FAILURE);
      }
      recLoc = 0;
      recSize = 0;
      vecLoc = 0;

      while (!_GribFile->eof()) {
        // Unpack all grib records and add them to the inventory
	if ((recSize = _GribFile->addToInventory (fp, recLoc)) == RI_FAILURE ) {
		   cerr << "File " << filePath << " inventory not completed." << endl << flush;
		   continue;
	}
		recLoc += recSize;
      }
      // In the summer of 05, GFS data started interlacing the UGRD/VGRD values.
      // Calling the _inventory.sort routine will reorder the vertical planes
      // so they are contiguous

      _GribFile->sortInventory();
      //_GribFile->printInventory(stdout);

      if (_printSummary) {
        cout << "Printing summary on file " << filePath << endl << flush;
        _GribFile->printSummary();
        return (RI_SUCCESS);
      }

      // Proccess all the records just loaded in the inventory
      if (_paramsPtr->process_everything) {
        _gribFields.erase(_gribFields.begin(), _gribFields.end());
        _gribFields = _GribFile->getFieldList();
      }

      if (_paramsPtr->debug) {
        //_field = _gribFields.begin();
        //while (_field != _gribFields.end()) {
        //  cout << (int) _field->param_id << " " << (int) _field->level_id << endl;

        //  _field++;

        //}
        
        cout << "Inventory size = " << _GribFile->InventorySize() << endl; 
        _GribFile->printInventory(stdout);
        cout << " Number of requested fields = " << _gribFields.size() << endl;
      }

      //
      // Creating Mdvx object
      // 
      PMU_auto_register( "Creating Mdvx file" );

      _field = _gribFields.begin();

      while (_field != _gribFields.end()) {
        if (_paramsPtr->debug) {
          cout << "Looking for field number " << (int) _field->param_id 
                               << " level_id = " << (int) _field->level_id << endl;
        }

        // find the number Z planes (one level if two dimensional, multiple if three)
        nZlevels = _GribFile->getNumZlevels (_field->param_id, _field->level_id);
        // check to see if the product was found.
        if (nZlevels == RI_FAILURE) {
          _field++;
          continue;
        }

        // process multiple records if request represents three dimensional data
        if (_paramsPtr->debug) {
          cout << "Number of Z levels " << nZlevels << endl << flush;
        }
 
        for (int z = 0; z < nZlevels; z++) {
          if ((vecLoc = _GribFile->findGribRec (_field->param_id, _field->level_id, z)) == RI_FAILURE) {
            cerr << "could not find parameter " << _field->param_id << " , level type " 
				            << _field->level_id  << ", level " << z++ << " in grib file " 
							<< endl << flush; 
              return( RI_FAILURE );
          }
          _GribRecord = _GribFile->getRecord(vecLoc);

          if (_GribRecord->_getGridOrientation() == gds::GO_NS_WE) {
             // Grib GFS data is ordered North to South (W to E), MDV S to N (W to E)
             _GribRecord->reOrderNS_2_SN((fl32 *) _GribRecord->getData(), 
                         _GribRecord->getNx(), _GribRecord->getNy());
            
          }
          if (z == 0) {

            // create Mdvx field header for the first level 
            memset( (void *) &_fieldHeader, (int) 0, sizeof(Mdvx::field_header_t) );
            memset( (void *) &_vlevelHeader, (int) 0, sizeof(Mdvx::vlevel_header_t) );
            _vlevelHeader.struct_id = Mdvx::VLEVEL_HEAD_MAGIC_COOKIE_64;

            if ( _createFieldHdr(nZlevels) != RI_SUCCESS ) {
              cerr << "WARNING: File " << filePath << " not processed." << endl << flush;
              _clearMdvxFields();
              _GribFile->clearInventory();
              return( RI_FAILURE );
            }
          }

          //_GribRecord->printPDS(stdout);
          _addPlane(_GribRecord->getData(), nZlevels);

          //
          // fill out the vlevel header
          //
          _vlevelHeader.type[z] = _GribRecord->getMdvVerticalLevelType();
          //_vlevelHeader.level[z] = _fieldHeader.grid_minz + _fieldHeader.grid_dz * z;
          _vlevelHeader.level[z] = _GribRecord->getLevelVal();
        }

        //
        // create the MdvxField object for each field
        //
        _outputFields.push_back(new MdvxField(_fieldHeader, _vlevelHeader,
                                                                  _dataPtr ) );

        _outputFields.back()->setFieldName 
                (_GribFile->uniqueParamNm(_GribRecord->getName(), _field->level_id).c_str());

        _outputFields.back()->setFieldNameLong (_GribRecord->getLongName().c_str());
        _outputFields.back()->setUnits (_GribRecord->getUnits().c_str());

 
        //_GribRecord->printGDS(stdout);
        _field++;
        _data->reset();

      }

      fclose(fp);
      //
      // Write the mdv file
      //
      PMU_auto_register( "Writing mdv file" );
      cout << "Writing mdv file." << endl << flush;
      if( _writeMdvFile() != RI_SUCCESS ) {
  	cerr << "ERROR: Could not write MDV file." << endl << flush;
        _clearMdvxFields();
        _GribFile->clearInventory();
  	return( RI_FAILURE );
      }
      
      //
      // Reinitialize for next file
      //
      _clearMdvxFields();
      _GribFile->clearInventory();
      
   }
   
   return(RI_SUCCESS);
}


int
Grib2Mdv::_mdvInit() 
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
    return( RI_FAILURE );
  }

  if( _paramsPtr->output_grid.nx * _paramsPtr->output_grid.ny * 
      _paramsPtr->output_grid.nz > MAX_NPTS ) {
    cerr << "ERROR: Output grid too big" << endl << flush;
    return( RI_FAILURE );
  }

  //
  // Set up output file 
  //
  _outputFile = new OutputFile( _paramsPtr );

  return( RI_SUCCESS );
}



int
Grib2Mdv::_writeMdvFile()
{
  time_t generateTime = _GribRecord->getGenerateTime();
  int forecastTime = _GribRecord->getForecastTime();

  if( (generateTime < 0) || (forecastTime < 0)) {
    cerr << " WARNING: File times don't make sense" << endl;
    cerr << "    Generate time = " << generateTime << endl;
    cerr << "    Forecast time = " << forecastTime << endl << flush;
    return( RI_SUCCESS );
  }
    
  //
  // Tell the user what we're doing
  //
  //if(_paramsPtr->debug) {
  if (1) {
    DateTime genTime( generateTime );
    cout << "Writing grid output file at " << genTime.dtime() << " for a forecast time of " \
	 << (forecastTime/3600) << " hours" << endl << flush;
  }

  //
  // Prepare the file
  //
  for( vector<MdvxField*>::iterator mfi = _outputFields.begin(); mfi != _outputFields.end(); mfi++ ) {
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
    return( RI_FAILURE );
  }
  //
  // Clear the file for next time
  //
  _outputFile->clear();

  return( RI_SUCCESS );
}

int
Grib2Mdv::_remapData() 
{

  if(_paramsPtr->mdv_proj_type != Params::OUTPUT_PROJ_NATIVE) {
    for( vector<MdvxField*>::iterator mfi = _outputFields.begin(); mfi != _outputFields.end(); mfi++ ) {
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
  return( RI_SUCCESS );
}

void
Grib2Mdv::_clearMdvxFields() 
{
  for( vector<MdvxField*>::iterator mfi = _outputFields.begin(); mfi != _outputFields.end(); mfi++ ) {
    if(*mfi) {
      delete (*mfi);
    }
    *mfi = 0;
  }
  _outputFields.erase(_outputFields.begin(), _outputFields.end());
}

int
Grib2Mdv::_createFieldHdr (int numZlevels)
{


    //
    // pass along vertical level info to the OutputFile object for the master header
    //
    if (numZlevels > 1 ) {
      _outputFile->setVerticalType( _GribRecord->getMdvVerticalLevelType() );
    }

    //
    // fill out the field header
    //

    _fieldHeader.record_len1         = sizeof( Mdvx::field_header_t );
    _fieldHeader.struct_id           = Mdvx::FIELD_HEAD_MAGIC_COOKIE_64;
    _fieldHeader.field_code          = _GribRecord->getParameterId();
    _fieldHeader.forecast_delta      = _GribRecord->getForecastTime();
    _fieldHeader.forecast_time       = _GribRecord->getGenerateTime() + _GribRecord->getForecastTime();
    _fieldHeader.nx                  = _GribRecord->getNx();
    _fieldHeader.ny                  = _GribRecord->getNy();
    _fieldHeader.nz                  = numZlevels;
    _fieldHeader.data_element_nbytes = sizeof(fl32);
    _fieldHeader.encoding_type       = Mdvx::ENCODING_FLOAT32;
    
    _fieldHeader.field_data_offset   = 0;
    _fieldHeader.volume_size         = _fieldHeader.nx * _fieldHeader.ny *
                            _fieldHeader.nz * _fieldHeader.data_element_nbytes;
    _fieldHeader.compression_type    = Mdvx::COMPRESSION_ASIS;
    _fieldHeader.transform_type      = Mdvx::DATA_TRANSFORM_NONE;
    _fieldHeader.scaling_type        = Mdvx::SCALING_NONE;
    _fieldHeader.native_vlevel_type  = _GribRecord->getMdvVerticalLevelType();
    _fieldHeader.vlevel_type         = _GribRecord->getMdvVerticalLevelType();

    //look into this - basically need to find out how to determine if the
    //z levels are constant 
    _fieldHeader.dz_constant         = 1;

    // will have to remap this part it acutally starts at 90 or the upper
    // left hand corner -> MDV expects the lower left hand corner
    _fieldHeader.proj_origin_lat     = 90;

    _fieldHeader.proj_origin_lon     = 0;

// I'm not sure what this means
    _fieldHeader.vert_reference      = 0;
  
    _fieldHeader.grid_dx             = _GribRecord->getDx();
    _fieldHeader.grid_dy             = _GribRecord->getDy();

    _fieldHeader.grid_dz             = _GribRecord->getLevelVal();

    // make the following constants?
    //_fieldHeader.grid_minx           = _GribRecord->getMinx();

    _fieldHeader.grid_minx           = _GribRecord->getStartLon();;
    _fieldHeader.grid_miny           = _GribRecord->getStartLat();;
    //_fieldHeader.grid_miny           = _GribRecord->getMiny();

    _fieldHeader.grid_minz           = _GribRecord->getLevelVal();

    // floating point values - no scale or bias
    _fieldHeader.scale               = 1.0;
    _fieldHeader.bias                = 0.0;
    _fieldHeader.bad_data_value      = -9998.0;
    _fieldHeader.missing_data_value  = -9999.0;
    _fieldHeader.proj_rotation       = 0.0;

    _fieldHeader.proj_type = Mdvx::PROJ_LATLON;


  return( RI_SUCCESS );
}

void
Grib2Mdv::_cleanup ()
{
  _gribFields.erase(_gribFields.begin(), _gribFields.end() );
  delete _data;
  if (_outputFile)
     delete _outputFile;
}   

void
Grib2Mdv::_addPlane(void *new_data, int nZlevels)
{
  int npts = _GribRecord->getNx()*_GribRecord->getNy();
  _dataPtr = _data->add(new_data, sizeof(fl32)*npts );
}
