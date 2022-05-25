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
///////////////////////////////////////////////////
// OutputFile - adapted from code by Mike Dixon for
// MM5Ingest
//
///////////////////////////////////////////////////

#include <math.h>

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <physics/IcaoStdAtmos.hh>

#include "OutputFile.hh"
#include "Params.hh"
#include "EraNc2Mdv.hh"
#include "HtInterp.hh"
using namespace std;

OutputFile::OutputFile(Params *params)
{
  _paramsPtr = params;
  _mdvObj = new DsMdvx;
  _htInterp = new HtInterp(params);
}

OutputFile::~OutputFile()
{
  clear();
  delete _mdvObj;
  delete _htInterp;
}

void
OutputFile::clear() 
{
  _mdvObj->clear();
}

int OutputFile::numFields()
{
  return _mdvObj->getNFields();
}

int 
OutputFile::writeVol( time_t genTime, long int leadSecs )
{

  PMU_auto_register("In OutputFile::writeVol");
  
  if(numFields() == 0) {
    cerr << "ERROR: No fields added" << endl << flush;
    return( RI_FAILURE );
  }

  // interp to height levels if required

  if (_paramsPtr->interp_vlevels_to_height) {
    if (_htInterp->interpVlevelsToHeight(_mdvObj)) {
      cerr << "ERROR - OutputFile::writeVol" << endl;
      return -1;
    }
  }
  
  // convert field encoding and compression

  if (_paramsPtr->process_everything) {

    // loop through fields, converting and compressing

    Mdvx::encoding_type_t encoding = mdvEncoding(_paramsPtr->encoding_type);
    Mdvx::compression_type_t compression = mdvCompression(_paramsPtr->compression_type);

    for (size_t ii = 0; ii < _mdvObj->getNFields(); ii++) {
      MdvxField *field = _mdvObj->getField(ii);
      field->convertType(encoding, compression);
    } // ii

  } else {

    // loop through fields, converting and compressing

    Mdvx::compression_type_t compression = mdvCompression(_paramsPtr->compression_type);
    
    for (size_t ii = 0; ii < _mdvObj->getNFields(); ii++) {
      MdvxField *field = _mdvObj->getField(ii);
      string fieldName(field->getFieldHeader().field_name);
      // find encoding for this particular fields
      Mdvx::encoding_type_t encoding = mdvEncoding(_paramsPtr->encoding_type);
      for (int jj = 0; jj < _paramsPtr->output_fields_n; jj++) {
        Params::out_field_t ofld = _paramsPtr->_output_fields[jj];
        string mdvName(ofld.mdv_name);
        if (mdvName == fieldName) {
          encoding = mdvEncoding(ofld.encoding_type);
        }
      }
      field->convertType(encoding, compression);
    } // ii

  }

  // latest data info

  if ( _paramsPtr->writeLdataInfo ) {
    _mdvObj->setWriteLdataInfo();
  } else {
    _mdvObj->clearWriteLdataInfo();
  }

  if(_paramsPtr->writeLdataInfo && _paramsPtr->debug) {
    cerr << "Writing LdataInfo..." << endl << flush;
  }

  // Write non forecast style file

  if( _paramsPtr->write_non_forecast ) {
    if (_paramsPtr->data_is_non_forecast) {
      _setMasterHdr( genTime, 0, true);
    } else {
      if( _paramsPtr->non_forecast_timestamp == Params::TIMESTAMP_FCAST_TIME) {
	_setMasterHdr( genTime, leadSecs, false );
      } 
      else {
	_setMasterHdr( genTime, 0, false );
      }
    }
    if(_paramsPtr->debug) {
      cerr << "Writing non-forecast style to dir: "
           << _paramsPtr->non_forecast_mdv_url << endl;
    }
    if( _mdvObj->writeToDir( _paramsPtr->non_forecast_mdv_url ) ) {
      cerr << "ERROR: Could not write file: "
           << _mdvObj->getErrStr() << endl << flush;
      return( RI_FAILURE );
    }
  }
   
  // Write forecast style file
  
  if( _paramsPtr->write_forecast ) {
    _setMasterHdr( genTime, leadSecs, false );
    _mdvObj->setWriteAsForecast();
    if(_paramsPtr->debug) {
      cerr << "Writing forecast style to dir: "
           << _paramsPtr->forecast_mdv_url << endl;
    }
    if( _mdvObj->writeToDir( _paramsPtr->forecast_mdv_url ) ) {
      cerr << "ERROR: Could not write file: "
           << _mdvObj->getErrStr() << endl << flush;
      return( RI_FAILURE );
    }
  }

  cerr << "File written: " << _mdvObj->getPathInUse() << endl;

  // Clean up

  clear();
  
  return( RI_SUCCESS );
}

void 
OutputFile::_setMasterHdr( time_t genTime, long int leadSecs, bool isObs )
{

   Mdvx::master_header_t masterHdr;

   // Clear out master header

   memset(&masterHdr, 0, sizeof(Mdvx::master_header_t) );
  
   // Fill the master header

   masterHdr.record_len1     = sizeof( Mdvx::master_header_t );
   masterHdr.struct_id       = Mdvx::MASTER_HEAD_MAGIC_COOKIE_64;
   masterHdr.revision_number = 1;
   masterHdr.num_data_times  = 1;
   masterHdr.index_number    = 0;
   masterHdr.data_dimension  = 3;
    
   if (isObs) {
     masterHdr.data_collection_type = Mdvx::DATA_MEASURED;
   } else {
     masterHdr.data_collection_type = Mdvx::DATA_FORECAST;
   }
   masterHdr.vlevel_included      = TRUE;
   masterHdr.grid_orientation     = Mdvx::ORIENT_SN_WE;
   masterHdr.data_ordering        = Mdvx::ORDER_XYZ;
   masterHdr.sensor_lon           = 0.0;
   masterHdr.sensor_lat           = 0.0;
   masterHdr.sensor_alt           = 0.0;

   masterHdr.time_gen      = genTime;
   masterHdr.time_begin    = genTime + leadSecs;
   masterHdr.time_end      = genTime + leadSecs;
   masterHdr.time_centroid = genTime + leadSecs;
   masterHdr.time_expire   = genTime + leadSecs;
   masterHdr.forecast_time = genTime + leadSecs;
   masterHdr.forecast_delta = leadSecs;

   STRncopy(masterHdr.data_set_info, _paramsPtr->data_set_info, MDV_INFO_LEN);
   STRncopy(masterHdr.data_set_name, _paramsPtr->data_set_name, MDV_NAME_LEN);
   STRncopy(masterHdr.data_set_source, _paramsPtr->data_set_source, MDV_NAME_LEN);

   masterHdr.record_len2    = sizeof( Mdvx::master_header_t );

   _mdvObj->setMasterHeader( masterHdr );
   _mdvObj->updateMasterHeader();

}

void
OutputFile::addField( MdvxField *field, Mdvx::encoding_type_t encoding)
{
   // Remap Data

   if(_paramsPtr->remap_output) {
     _remap(field);
   }

   // Add field to volume
  
   _mdvObj->addField( field );

}

void 
OutputFile::_remap(MdvxField* field)
{

  MdvxRemapLut lut;

  switch( _paramsPtr->out_projection_info.type) {
  case Params::PROJ_FLAT:
    field->remap2Flat(lut, _paramsPtr->out_grid_info.nx, _paramsPtr->out_grid_info.ny, 
		      _paramsPtr->out_grid_info.minx, _paramsPtr->out_grid_info.miny, 
		      _paramsPtr->out_grid_info.dx, _paramsPtr->out_grid_info.dy, 
		      _paramsPtr->out_projection_info.origin_lat, 
		      _paramsPtr->out_projection_info.origin_lon,
		      _paramsPtr->out_projection_info.rotation);
    break;
	  
  case Params::PROJ_LATLON:
    field->remap2Latlon(lut, _paramsPtr->out_grid_info.nx, _paramsPtr->out_grid_info.ny, 
			_paramsPtr->out_grid_info.minx, _paramsPtr->out_grid_info.miny, 
			_paramsPtr->out_grid_info.dx, _paramsPtr->out_grid_info.dy );
    break;

  case Params::PROJ_LAMBERT_CONF:
    if(field->getFieldHeader().proj_type == Mdvx::PROJ_LAMBERT_CONF)
      _remapLambertLambert(field);
    else
      field->remap2Lc2(lut, _paramsPtr->out_grid_info.nx, _paramsPtr->out_grid_info.ny, 
			    _paramsPtr->out_grid_info.minx, _paramsPtr->out_grid_info.miny, 
			    _paramsPtr->out_grid_info.dx, _paramsPtr->out_grid_info.dy,
			    _paramsPtr->out_projection_info.origin_lat, 
			    _paramsPtr->out_projection_info.origin_lon,
			    _paramsPtr->out_projection_info.ref_lat_1, 
			    _paramsPtr->out_projection_info.ref_lat_2 );
    break;
  default:
    cerr <<"-- unknown projection; remapping failed." << endl;
  }

}

void OutputFile::_remapLambertLambert(MdvxField* field)
{

  // field is encoded as bytes -- convert back to float
  field->convertType();
  

  Mdvx::field_header_t fhdr = field->getFieldHeader();
  
  int nx = _paramsPtr->out_grid_info.nx;
  int ny = _paramsPtr->out_grid_info.ny;
  int nz = fhdr.nz;

  MdvxProj inproj, outproj;
  outproj.initLambertConf(_paramsPtr->out_projection_info.origin_lat, 
			  _paramsPtr->out_projection_info.origin_lon,
			  _paramsPtr->out_projection_info.ref_lat_1,
			  _paramsPtr->out_projection_info.ref_lat_2);

  outproj.setGrid(nx, ny, _paramsPtr->out_grid_info.dx, _paramsPtr->out_grid_info.dy,
		  _paramsPtr->out_grid_info.minx, _paramsPtr->out_grid_info.miny);
  
  inproj.init(fhdr);
  
  float *odata = new float[nx*ny*nz];
  float *idata = (float *)field->getVol();

  double lat, lon;
  double ix, iy;
  for(int y = 0; y < ny; y++) 
  {
    for(int x = 0; x < nx; x++)
    {

      outproj.xyIndex2latlon(x, y, lat, lon);

      int ingrid = inproj.latlon2xyIndex(lat, lon, ix, iy);

      // If we are within 1/100th of the dx past the end of the grid
      // allow it to be set to the end of the grid.  
      // (rounding issue from projection library)
      if(ix > fhdr.nx-1 && ix < fhdr.nx -.99)
	ix = fhdr.nx-1;
      if(iy > fhdr.ny-1 && iy < fhdr.ny -.99)
	iy = fhdr.ny-1;

      if(ingrid != -1 && ix >= 0 && iy >= 0)
	for(int z = 0; z < nz; z++)
	  odata[(z*ny*nx)+(y*nx)+x] = _interp2(&fhdr, ix, iy, z, idata);
      else
	for(int z = 0; z < nz; z++)
	  odata[(z*ny*nx)+(y*nx)+x] = fhdr.missing_data_value;
    }
  }

  fhdr.nx = _paramsPtr->out_grid_info.nx;
  fhdr.ny = _paramsPtr->out_grid_info.ny;
  fhdr.grid_minx = _paramsPtr->out_grid_info.minx;
  fhdr.grid_miny = _paramsPtr->out_grid_info.miny;
  fhdr.grid_dx = _paramsPtr->out_grid_info.dx;
  fhdr.grid_dy = _paramsPtr->out_grid_info.dy;
  fhdr.proj_type = _paramsPtr->out_projection_info.type;
  fhdr.proj_origin_lat = _paramsPtr->out_projection_info.origin_lat;
  fhdr.proj_origin_lon = _paramsPtr->out_projection_info.origin_lon;
  fhdr.proj_param[0] = _paramsPtr->out_projection_info.ref_lat_1;
  fhdr.proj_param[1] = _paramsPtr->out_projection_info.ref_lat_2;
  fhdr.volume_size =
    fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes; 

  field->setFieldHeader(fhdr);
  field->setVolData(odata, fhdr.volume_size, Mdvx::ENCODING_FLOAT32);

  delete []odata;

}

float OutputFile::_interp2(Mdvx::field_header_t *fieldHdr, double x, double y, int z, float *field)
{
  int ix = floor(x);
  int iy = floor(y);
  int ix1 = ix+1;
  int iy1 = iy+1;

  // Allow wraping in longitude if x is between -.5 and 0, and a global lat/lon model
  if(x > -.5 && ix == -1 && fieldHdr->proj_type == 0 && fieldHdr->proj_origin_lon == 0 &&
     (fieldHdr->nx * fieldHdr->grid_dx) + fieldHdr->grid_minx > 360.0)
    ix = floor((360.0 - fieldHdr->grid_minx) / fieldHdr->grid_dx);
  if(field == NULL || ix < 0 || y < 0 || ix1 > fieldHdr->nx || iy1 > fieldHdr->ny)
    return fieldHdr->missing_data_value;
  if(z < 0)
    z = 0;
  if(z >= fieldHdr->nz)
    z = fieldHdr->nz -1;
  if(field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix1] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix1] == fieldHdr->missing_data_value)
    return fieldHdr->missing_data_value;

  float val;
  // Allow being exactly on the last point in the grid (x or y or both)
  if(ix1 == fieldHdr->nx && iy1 == fieldHdr->ny)
    val = field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] * (1-(x-ix));
  else if(ix1 == fieldHdr->nx)
    val = field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] * (1-(y-iy)) + 
      field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix] * (y-iy);
  else if(iy1 == fieldHdr->ny)
    val = field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] * (1-(x-ix)) + 
	   field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix1] * (x-ix);
  else  // Normal 2D interpolation
    val = (field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix] * (1-(x-ix)) + 
	   field[(z*fieldHdr->ny*fieldHdr->nx)+(iy*fieldHdr->nx)+ix1] * (x-ix)) * (1-(y-iy)) + 
      (field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix] * (1-(x-ix)) + 
       field[(z*fieldHdr->ny*fieldHdr->nx)+((iy1)*fieldHdr->nx)+ix1] * (x-ix)) * (y-iy);
  
  if(val != val)
    return fieldHdr->missing_data_value;
  else
    return val;

}

Mdvx::encoding_type_t
  OutputFile::mdvEncoding(Params::encoding_type_t paramEncoding)

{
  switch(paramEncoding) {
    case Params::ENCODING_INT8:
      return Mdvx::ENCODING_INT8;
    case Params::ENCODING_INT16:
      return Mdvx::ENCODING_INT16;
    case Params::ENCODING_ASIS:
    case Params::ENCODING_FLOAT32:
    default:
      return Mdvx::ENCODING_FLOAT32;
  }
}

Mdvx::compression_type_t
  OutputFile::mdvCompression(Params::compression_type_t paramCompression)
  
{
  switch(paramCompression) {
    case Params::COMPRESSION_NONE:
      return Mdvx::COMPRESSION_NONE;
    case Params::COMPRESSION_RLE:
    case Params::COMPRESSION_LZO:
    case Params::COMPRESSION_ASIS:
    case Params::COMPRESSION_ZLIB:
      return Mdvx::COMPRESSION_ZLIB;
    case Params::COMPRESSION_BZIP:
      return Mdvx::COMPRESSION_BZIP;
    case Params::COMPRESSION_GZIP:
    default:
      return Mdvx::COMPRESSION_GZIP;
  }
}

