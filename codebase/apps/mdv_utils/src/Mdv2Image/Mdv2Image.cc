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
// Mdv2Image.cc
//
///////////////////////////////////////////////////////////////

#include "Mdv2Image.hh"
#include <didss/DsDataFile.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxRadar.hh>
#include <Mdv/MdvxTimeStamp.hh>
using namespace std;

// Constructor

Mdv2Image::Mdv2Image(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "Mdv2Image";
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // parse the DateTime objects

  _readSearchTime = DateTime::parseDateTime(_params.read_search_time);
  if (_readSearchTime == DateTime::NEVER) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse read_search_time: "
	 << "\"" << _params.read_search_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
  }
    
  _latestValidModTime = DateTime::parseDateTime(_params.latest_valid_mod_time);
  if (_latestValidModTime == DateTime::NEVER) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse latest_valid_mod_time: "
	 << "\"" << _params.latest_valid_mod_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
  }
    
  return;

}

// destructor

Mdv2Image::~Mdv2Image()

{


}

//////////////////////////////////////////////////
// Run

int Mdv2Image::Run()
{

  // set up Mdvx object

  DsMdvx *mdvx;
  if (_params.threaded) {
    mdvx = new DsMdvxThreaded;
  } else {
    mdvx = new DsMdvx;
  }

  if (_params.set_latest_valid_mod_time) {
    mdvx->setCheckLatestValidModTime(_latestValidModTime);
  }

  if (_params.specify_file_by_time) {
      
      mdvx->setReadTime((Mdvx::read_search_mode_t)
			_params.read_search_mode,
			_params.url,
			_params.read_search_margin,
			_readSearchTime,
			_params.read_forecast_lead_time);
      
   } else {
      
      string path;
      if (strlen(_params.path) > 0 &&
	  _params.path[0] == '/' ||_params.path[0] == '.') {
	path = _params.path;
      } else {
	path = "./";
	path += _params.path;
      }
      mdvx->setReadPath(path);
      
  } // if (_params.specify_file_by_time)

  mdvx->setReadFieldFileHeaders();
    
  if (_params.set_valid_time_search_wt) {
    mdvx->setValidTimeSearchWt(_params.valid_time_search_wt);
  }

  if (_params.constrain_forecast_lead_times) {
    mdvx->setConstrainFcastLeadTimes
      (_params.forecast_constraints.min_lead_time,
       _params.forecast_constraints.max_lead_time,
       _params.forecast_constraints.request_by_gen_time);
  }
  
  if (_params.read_set_horiz_limits) {
    mdvx->setReadHorizLimits(_params.read_horiz_limits.min_lat,
			     _params.read_horiz_limits.min_lon,
			     _params.read_horiz_limits.max_lat,
			     _params.read_horiz_limits.max_lon);
  }
  
  if (_params.read_set_vlevel_limits) {
    mdvx->setReadVlevelLimits(_params.read_lower_vlevel,
			      _params.read_upper_vlevel);
  }

  if (_params.read_set_plane_num_limits) {
    mdvx->setReadPlaneNumLimits(_params.read_lower_plane_num,
				_params.read_upper_plane_num);
  }
  
  if (_params.read_set_vlevel_type) {
    switch(_params.read_vlevel_type) {
    case Params::VERT_TYPE_Z:
      mdvx->setReadVlevelType(Mdvx::VERT_TYPE_Z);
      break;
    case Params::VERT_TYPE_PRESSURE:
      mdvx->setReadVlevelType(Mdvx::VERT_TYPE_PRESSURE);
      break;
    case Params::VERT_FLIGHT_LEVEL:
      mdvx->setReadVlevelType(Mdvx::VERT_FLIGHT_LEVEL);
      break;
    }
  }

  mdvx->setReadEncodingType((Mdvx::encoding_type_t)
			    _params.read_encoding_type);

  mdvx->setReadCompressionType((Mdvx::compression_type_t)
			       _params.read_compression_type);

  mdvx->setReadScalingType((Mdvx::scaling_type_t) _params.read_scaling_type,
			   _params.read_scale,
			   _params.read_bias);

  if (_params.read_composite) {
    mdvx->setReadComposite();
  }

  if (_params.read_set_fill_missing) {
    mdvx->setReadFillMissing();
  }

  if (_params.read_set_field_names) {

    for (int i = 0; i < _params.read_field_names_n; i++) {
      mdvx->addReadField(_params._read_field_names[i]);
    }

  } else if (_params.read_set_field_nums) {

    for (int i = 0; i < _params.read_field_nums_n; i++) {
      mdvx->addReadField(_params._read_field_nums[i]);
    }

  }

  if (_params.read_set_decimation) {
    mdvx->setReadDecimate(_params.decimation_max_nxy);
  }

  if (_getVolume(mdvx) == 0) {
	  _output_mdv_obj(mdvx);
      delete mdvx;
      return 0;
  } else {
      delete mdvx;
      return -1;
  }

  return 0;

}

////////////////////////
// read volume

int Mdv2Image::_getVolume(DsMdvx *mdvx)
{

  if (_params.debug) {
    mdvx->setDebug();
    mdvx->printReadRequest(cerr);
  }

  if (mdvx->readVolume()) {
    cerr << mdvx->getErrStr();
    return -1;
  }
  if (_params.threaded) {
    DsMdvxThreaded *mdvxt = (DsMdvxThreaded *) mdvx;
    while (!mdvxt->getThreadDone()) {
      if (_params.debug) {
	cerr << "Waiting for threaded read to complete, % done:"
	     << mdvxt->getPercentReadComplete() << endl;
      }
      umsleep(500);
    }
  }

  return 0;
  
}
  
////////////////////////
// read all headers

int Mdv2Image::_getAllHeaders(DsMdvx *mdvx)
{
  
  if (_params.debug) {
    mdvx->setDebug();
    mdvx->printReadRequest(cerr);
  }

  if (mdvx->readAllHeaders()) {
    cerr << mdvx->getErrStr();
    return -1;
  }
  if (_params.threaded) {
    DsMdvxThreaded *mdvxt = (DsMdvxThreaded *) mdvx;
    while (!mdvxt->getThreadDone()) {
      if (_params.debug) {
	cerr << "Waiting for threaded read to complete, % done:"
	     << mdvxt->getPercentReadComplete() << endl;
      }
      umsleep(500);
    }
  }

  return 0;
  
}
  
  
////////////////////////
// output_image 

void Mdv2Image::_output_mdv_obj(const DsMdvx *mdvx) 
{
  cout << endl;
  cout << "File path: " << mdvx->getPathInUse() << endl;

  // master header
  const Mdvx::master_header_t &mhdr = mdvx->getMasterHeader();

  _dataTime = mhdr.time_begin; // Record the data time of this data set.
  
  // fields
  
  for (int i = 0; i < mdvx->getNFields(); i++) {
    MdvxField *field = mdvx->getField(i);

    MdvxProj proj(field->getFieldHeader());
    // convert to floats, uncompressed, linear
     field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

    MdvxRadar mdvxRadar;
    if (mdvxRadar.loadFromMdvx(*mdvx) == 0) {
      DsRadarParams radar = mdvxRadar.getRadarParams();
      proj.setSensorPosn(radar.latitude, radar.longitude, radar.altitude);
    }
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    const Mdvx::vlevel_header_t &vhdr = field->getVlevelHeader();
	
    if (_params.read_transform_to_linear) {
	  if (field->transform2Linear()){
	    cerr << field->getErrStr() << endl;
	  }
    }
	_output_image(field);

  } // For all fields.

}
  
////////////////////////
// Output an Image for each plane of data
//
void Mdv2Image::_output_image(MdvxField *field )
{
  Imlib_Image image;
  DATA32 *im_buf;
  DATA32 *im_ptr;
  DATA32 *data_ptr;
  DATA32 *data;
  unsigned char a,r,g,b;

  const Mdvx::field_header_t &fhdr = field->getFieldHeader();

  switch(fhdr.encoding_type) {

	  case Mdvx::ENCODING_RGBA32: // Can do a Simple pixel copy
		  _output_image_rgba(field);
	  break;
								  
	  case Mdvx::ENCODING_FLOAT32: // Must render using a colorscale
	      _output_image_values(field);
	  break;
	  
	  default:
		field->convertType(Mdvx::ENCODING_FLOAT32,Mdvx::COMPRESSION_NONE);
		_output_image_values(field);  // render using a colorscale

	  break;

	return;
  }
}

  
////////////////////////
// Output an Image for each plane of data
//
void Mdv2Image::_output_image_values(MdvxField *field )
{
  Imlib_Image image;
  DATA32 *im_buf;
  DATA32 *im_ptr;
  fl32 *data_ptr;
  fl32 *data;

  const Mdvx::field_header_t &fhdr = field->getFieldHeader();

  if((im_buf = (DATA32 *)  calloc(fhdr.nx * fhdr.ny,sizeof(DATA32))) == NULL) {
	cerr << "Mdv2Image::_output_image calloc error\n";
	return;
  }
    
  for (int plane = 0 ; plane < fhdr.nz; plane++) {
	  // HMMM Work-around
	 if(fhdr.nz > 1) {
       data = ((fl32 *) field->getPlane(plane));
	 } else {
       data = ((fl32 *) field->getVol());
	 }

	 // FIX RGBA Byte ordering and Image Row Ordering
	 data_ptr = data;
	 for(int i = 0; i < fhdr.ny; i++ ){
		 // Reverse the row order using pointer math
		 im_ptr = im_buf + ((fhdr.ny -i -1) * fhdr.nx);

		 for(int j = 0; j < fhdr.nx; j++) {

			 data_ptr++;
			 im_ptr++;
		 }
	}

	image = imlib_create_image_using_copied_data(fhdr.nx, fhdr.ny,im_buf);
	imlib_context_set_image(image);

	double aspect = (double) fhdr.nx / fhdr.ny;
	if( _params.height > 0) {
		int ht = _params.height;
		int wd = (int) (aspect * ht + 0.5);
		if((image = imlib_create_cropped_scaled_image(0,0,fhdr.nx, fhdr.ny,wd,ht)) == NULL) {
			  cerr << "Mdv2Image::_output_image imlib_create_cropped_scaled_image failure " << endl;
			  return;
		}
		imlib_free_image_and_decache(); // Frees up original image
	    imlib_context_set_image(image);
	} 

    string fname = _build_fname(field,plane);

    if (_params.debug) {
		cerr << "Mdv2Image: Writing " << fname << endl;
	}

	 imlib_save_image(fname.c_str());

	 imlib_free_image();

  } 
  // Release the Image data buffer.
  if(im_buf != NULL) free(im_buf);
}

////////////////////////
// Output an Image for each plane of data
//
void Mdv2Image::_output_image_rgba(MdvxField *field )
{
  Imlib_Image image;
  DATA32 *im_buf;
  DATA32 *im_ptr;
  DATA32 *data_ptr;
  DATA32 *data;
  unsigned char a,r,g,b;

  const Mdvx::field_header_t &fhdr = field->getFieldHeader();

  if((im_buf = (DATA32 *)  calloc(fhdr.nx * fhdr.ny,sizeof(DATA32))) == NULL) {
	cerr << "Mdv2Image::_output_image calloc error\n";
	return;
  }
    
  for (int plane = 0 ; plane < fhdr.nz; plane++) {
	  // HMMM Work-around
	 if(fhdr.nz > 1) {
       data = ((DATA32 *) field->getPlane(plane));
	 } else {
       data = ((DATA32 *) field->getVol());
	 }

	 // FIX RGBA Byte ordering and Image Row Ordering
	 data_ptr = data;
	 for(int i = 0; i < fhdr.ny; i++ ){
		 // Reverse the row order using pointer math
		 im_ptr = im_buf + ((fhdr.ny -i -1) * fhdr.nx);

		 for(int j = 0; j < fhdr.nx; j++) {
			 // PulL out RGB
			 a = *data_ptr >> 24 & 0xff;
			 b = *data_ptr >> 16 & 0xff;
			 g = *data_ptr >> 8 & 0xff;
			 r = *data_ptr  & 0xff;

			 // Reassemble
			 *im_ptr = b | (g <<8) | (r << 16) | (a << 24);

			 data_ptr++;
			 im_ptr++;
		 }
	}

	image = imlib_create_image_using_copied_data(fhdr.nx, fhdr.ny,im_buf);
	imlib_context_set_image(image);

	double aspect = (double) fhdr.nx / fhdr.ny;
	if( _params.height > 0) {
		int ht = _params.height;
		int wd = (int) (aspect * ht + 0.5);
		if((image = imlib_create_cropped_scaled_image(0,0,fhdr.nx, fhdr.ny,wd,ht)) == NULL) {
			  cerr << "Mdv2Image::_output_image imlib_create_cropped_scaled_image failure " << endl;
			  return;
		}
		imlib_free_image_and_decache(); // Frees up original image
	    imlib_context_set_image(image);
	} 

    string fname = _build_fname(field,plane);

    if (_params.debug) {
		cerr << "Mdv2Image: Writing " << fname << endl;
	}

	 imlib_save_image(fname.c_str());

	 imlib_free_image();

  } 

  // Release the Image data buffer.
  if(im_buf != NULL) free(im_buf);
}

////////////////////////
string Mdv2Image::_build_fname(MdvxField *field, int plane)
{
	string result = _params.output_dir;
	struct tm res;
	char buf[128];

    const Mdvx::field_header_t &fhdr = field->getFieldHeader();

	// Add the Field name
	result += fhdr.field_name;
	result += "_";

	// Add the time stamp
	if(_params.include_time_in_fname) {
	    strftime(buf,128,"%Y%m%d%H%M%S", gmtime_r(&_dataTime,&res));
		result += buf;
	}

	// Add a plane number
	if(fhdr.nz > 1) {
		sprintf(buf,"_%d",plane);
		result += buf;
	}

	result += ".";
	result += _params.image_type;

	
	return result;
}
