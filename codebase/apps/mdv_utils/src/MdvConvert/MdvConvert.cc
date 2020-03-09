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
// MdvConvert.cc
//
// MdvConvert object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsSpecificFcstLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsFcstTimeListTrigger.hh>
#include <didss/DsInputPath.hh>
#include <rapformats/Cedric.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "MdvConvert.hh"
using namespace std;

// Constructor

MdvConvert::MdvConvert(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvConvert";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE || 
      _params.mode == Params::ARCHIVE_FCST ||
      _params.mode == Params::SPEC_FCST_ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  // make sure params are consistent

  if (_params.override_origin_location) {
    _params.remap_at_source = pFALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.reg_interval);

  // initialize the data trigger object

  _dataTrigger = NULL;

  _inputPath = NULL;

  if (!_initTrigger())
    isOK = false;
  
  return;

}

//////////////////////////
// destructor

MdvConvert::~MdvConvert()
{
  // unregister process

  PMU_auto_unregister();
}

//////////////////////////////////////////////////
// Run

int MdvConvert::Run ()
{

  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // process data: Mode is  TIME_INTERVAL, FILELIST, REALTIME, or 
  // SPEC_FCAST_REALTIME (DsTrigger class is used)

  if ( _params.mode != Params::LOCAL_FILEPATH_REALTIME) {

    while (!_dataTrigger->endOfData()) {

      TriggerInfo triggerInfo;
      time_t inputTime = _dataTrigger->next(triggerInfo);
      int leadTime = 0;
      if (triggerInfo.getForecastTime() > triggerInfo.getIssueTime()) {
        leadTime = triggerInfo.getForecastTime() - triggerInfo.getIssueTime();
        if (leadTime > 86400 * 100) {
          leadTime = -9999;
        }
      }

      if (_processData(triggerInfo.getIssueTime(),
	               leadTime,
	               triggerInfo.getFilePath())) {
        cerr << "MdvConvert::Run" <<"  Errors in processing time: "
             <<  triggerInfo.getIssueTime() << endl
             << "  input file: " << triggerInfo.getFilePath() << endl
             << "  input time: " << DateTime::strm(inputTime) << endl;
        
        iret = 1;
      }
    } // while

  } else {

    // process data: Mode is LOCAL_FILEPATH_REALTIME (DsInput path used)

    time_t inputTime;
      
    char *file;

    while ( (file = _inputPath->next()) != NULL) {

      inputTime = _inputPath->getDataTime(file);
	  
      if (_processData(inputTime , 0, file)) {
        cerr << "MdvConvert::Run" <<"  Errors in processing time: "
             << inputTime << endl;
        iret = 1;
      }

    } // while

  } // if ( _params.mode != Params::LOCAL_FILEPATH_REALTIME) {
  
  if ( _inputPath) delete _inputPath;
  if (_dataTrigger) delete _dataTrigger;

  return iret;

}

////////////////////////////////////////////////////////////////
// process

int MdvConvert:: _processData(time_t inputTime, int leadTime,
                              const string filepath)
{
     
  PMU_auto_register("Start main loop");
  int iret = 0;

  if (_params.debug) {
    cerr << "Processing data file" << endl;
    cerr << "  inputTime: " << DateTime::strm(inputTime) << endl;
    if (leadTime > 0) {
      cerr << "  leadTime: " << leadTime << endl;
    }
    if (filepath.size() > 0) {
      cerr << "  filePath: " << filepath << endl;
    }
  }

  // create DsMdvx object
  
  DsMdvx mdvx;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.setDebug(true);
  }
 
  // set up the Mdvx read
  
  mdvx.clear();
  _setupRead(mdvx);
     
  if ( _params.mode == Params::FILELIST ) {
    mdvx.setReadPath(filepath);
  } else if(_params.mode == Params::REALTIME_FCST_DATA ||
            _params.mode == Params::ARCHIVE_FCST) {

    // if lead times are subsampled, checks if this lead time is wanted
    if (!_wantedLeadTime(leadTime))
    {
      return 0;
    }

    mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _params.input_url,0,
                     inputTime, leadTime);
  } else if( _params.mode == Params::SPEC_FCST_ARCHIVE) {
    
    if( leadTime != _params.fcast_lead_time.lead_time_secs)
    {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << endl;
	cerr << "MdvConvert::_processData" << " ERROR: Forecast lead time " 
	     << leadTime << " not specified. " << "Skipping this file\n";
      }
      return 0;
    }
    
    mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _params.input_url,0,
                     inputTime, leadTime);
  } else {
    mdvx.setReadTime(Mdvx::READ_CLOSEST,
                     _params.input_url,
                     0, inputTime);
  }
    
  PMU_auto_register("Before read");
    
  // do the read
  
  if (!_params.input_be) {
    BE_reverse();
  }
    
  if (mdvx.readVolume()) {
    cerr << "ERROR - MdvConvert::Run" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << mdvx.getErrStr() << endl;
    iret = -1;
    if (!_params.input_be) {
      BE_reverse();
    }
  }
  
  if (!_params.input_be) {
    BE_reverse();
  }

  if (_params.debug) {
    cerr << "Working on file: " << mdvx.getPathInUse() << endl;
  }

  // override location if required

  if (_params.override_origin_location) {
    _overrideOrigin(mdvx);
  }
  
  // Remap the data if requested, and not done on read
  
  if (_params.auto_remap_to_latlon) {
    _autoRemapToLatLon(mdvx);
  } else if (_params.remap_xy && !_params.remap_at_source) {
    _remap(mdvx);
  }
  
  // to force scale change, convert field data to floats
  
  if (_params.force_scale_change) {
    for (size_t i = 0; i < mdvx.getNFields(); ++i) {
      mdvx.getField(i)->convertType(Mdvx::ENCODING_FLOAT32,
                                    Mdvx::COMPRESSION_NONE);
    }
  }
  
  // override vlevels if requested
  // only affects the headers
  
  if (_params.override_vlevels) {
    _overrideVlevels(mdvx);
  }

  // remap the vlevels onto a constant grid
  // this does affect the data files

  if (_params.remap_z_to_constant_grid) {
    for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {
      MdvxField *field = mdvx.getField(ii);
      if (field) {
        field->remapVlevels(_params.remap_z_grid.nz,
                            _params.remap_z_grid.minz,
                            _params.remap_z_grid.dz);
      }
    }
  }

  // apply linear transform
  
  if (_params.apply_linear_transform) {
    _applyTransform(mdvx);
  }
  
  if (_params.rename_fields) {
    _renameFields(mdvx);
  }
  
  if(_params.invert_vertically) {
    _invertVertically(mdvx);
  }
  
  if (_params.apply_thresholds_to_field_values) {
    _applyThresholds(mdvx);
  }
  
  // Convert to output encoding, type and scaling
  
  _convertOutput(mdvx);
  
  // set up write
  
  mdvx.clearWrite(); 
  
  if (_params.output_as_forecast){
    mdvx.setWriteAsForecast();
    if (_params.debug) {
      cerr << "Will write output as forecast" << endl;
    }    
  }
  
  if (_params.if_forecast_output_as_forecast){
    mdvx.setIfForecastWriteAsForecast();
    if (_params.debug) {
      cerr << "Will write output as forecast "
           << "if data is of forecast type" << endl;
    }    
  }

  // write using extended paths?
  
  if (_params.write_using_extended_paths) {
    mdvx.setWriteUsingExtendedPath();
  }
  
  if (_params.output_format == Params::OUTPUT_FORMAT_XML) {
    mdvx.setWriteFormat(Mdvx::FORMAT_XML);
    if (_params.debug) {
      cerr << "Will write output as xml" << endl;
    }    
  } else if (_params.output_format == Params::OUTPUT_FORMAT_NCF) {
    mdvx.setWriteFormat(Mdvx::FORMAT_NCF);
    if (_params.debug) {
      cerr << "Will write output as netCDF CF" << endl;
    }    
  }
  
  if (_params.writeLdataInfo)
    mdvx.setWriteLdataInfo();
  else
    mdvx.clearWriteLdataInfo();

  mdvx.setAppName(_progName);
  
  // netCDF MDV to NCF
  
  if (_params.output_format == Params::OUTPUT_FORMAT_NCF) {
    
    mdvx.clearMdv2Ncf();
    
    if (_params.ncf_set_global_attributes) {
      mdvx.setMdv2NcfAttr(_params.ncf_global_attributes.institution,
                          _params.ncf_global_attributes.references,
                          _params.ncf_global_attributes.comment);
    }
    
    if (_params.ncf_transform_fields) {
      for (int ii = 0; ii < _params.ncf_field_transforms_n; ii++) {
        const Params::ncf_field_transform_t &trans =
          _params._ncf_field_transforms[ii];
        DsMdvx::ncf_pack_t packing = DsMdvx::NCF_PACK_ASIS;
        if (trans.packed_data_type == Params::DATA_PACK_FLOAT) {
          packing = DsMdvx::NCF_PACK_FLOAT;
        } else if (trans.packed_data_type == Params::DATA_PACK_BYTE) {
          packing = DsMdvx::NCF_PACK_BYTE;
        } else if (trans.packed_data_type == Params::DATA_PACK_SHORT) {
          packing = DsMdvx::NCF_PACK_SHORT;
        }
        mdvx.addMdv2NcfTrans(trans.mdv_field_name,
                             trans.ncf_field_name,
                             trans.ncf_standard_name,
                             trans.ncf_long_name,
                             trans.ncf_units,
                             trans.do_linear_transform,
                             trans.linear_multiplier,
                             trans.linear_const,
                             packing);
      } // ii
    }
    
    if (_params.ncf_file_format == Params::CLASSIC) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_CLASSIC);
    } else if (_params.ncf_file_format == Params::NC64BIT) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_OFFSET64BITS);
    } else if  (_params.ncf_file_format == Params::NETCDF4_CLASSIC) {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCFD4_CLASSIC);
    } else {
      mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCDF4);
    }
    
    if (_params.ncf_polar_radar_file_type == Params::FILE_TYPE_CF_RADIAL) {
      mdvx.setRadialFileType(DsMdvx::RADIAL_TYPE_CF_RADIAL);
    } else if (_params.ncf_polar_radar_file_type == Params::FILE_TYPE_DORADE) {
      mdvx.setRadialFileType(DsMdvx::RADIAL_TYPE_DORADE);
    } else if (_params.ncf_polar_radar_file_type == Params::FILE_TYPE_UF) {
      mdvx.setRadialFileType(DsMdvx::RADIAL_TYPE_UF);
    } else {
      mdvx.setRadialFileType(DsMdvx::RADIAL_TYPE_CF);
    }
  
    mdvx.setMdv2NcfCompression(_params.ncf_compress_data,
                               _params.ncf_compression_level);
    
    mdvx.setMdv2NcfOutput(_params.ncf_output_latlon_arrays,
                          _params.ncf_output_mdv_attributes,
                          _params.ncf_output_mdv_chunks,
                          _params.ncf_output_start_end_times);
    
    mdvx.setNcfFileSuffix(_params.ncf_filename_suffix);
    
  } // if (_params.output_format == Params::OUTPUT_FORMAT_NCF)
    
  // perform write
  
  PMU_auto_register("Before write");
  
  if (!_params.output_be) {
    BE_reverse();
  }
  
  if(_params.write_to_path) {
    if(mdvx.writeToPath(_params.output_path)) {
      cerr << "ERROR - MdvConvert::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << mdvx.getErrStr() << endl;
      iret = -1;
    }
  } else {
    if(mdvx.writeToDir(_params.output_url)) {
      cerr << "ERROR - MdvConvert::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << mdvx.getErrStr() << endl;
      iret = -1;
    }
  }
  
  if (_params.debug) {
    cerr << "Wrote file:" << mdvx.getPathInUse() << endl;
  }    
  
  // turn swapping back to the original method.
  
  if (!_params.output_be ) {
    BE_reverse();
  }
  
  return iret;

}

////////////////////////////////////////////
// set up the read

void MdvConvert::_setupRead(DsMdvx &mdvx)

{

  // set up the Mdvx read
  
  mdvx.clearRead();
  
  // get file headers, to save encoding and compression info

  mdvx.setReadFieldFileHeaders();
  
  if (_params.set_horiz_limits) {
    mdvx.setReadHorizLimits(_params.horiz_limits.min_lat,
			    _params.horiz_limits.min_lon,
			    _params.horiz_limits.max_lat,
			    _params.horiz_limits.max_lon);
  }
  
  if (_params.set_vlevel_limits) {
    mdvx.setReadVlevelLimits(_params.lower_vlevel,
			     _params.upper_vlevel);
  }
  
  if (_params.set_plane_num_limits) {
    mdvx.setReadPlaneNumLimits(_params.lower_plane_num,
			       _params.upper_plane_num);
  }

  if (_params.composite) {
    mdvx.setReadComposite();
  }

  if (_params.set_field_names) {
    for (int i = 0; i < _params.field_names_n; i++) {
      mdvx.addReadField(_params._field_names[i]);
    }
  } else if (_params.set_field_nums) {
    for (int i = 0; i < _params.field_nums_n; i++) {
      mdvx.addReadField(_params._field_nums[i]);
    }
  }

  if (_params.remap_xy && _params.remap_at_source &&
      !_params.auto_remap_to_latlon) {
    
    if (_params.remap_projection == Params::PROJ_LATLON) {
      mdvx.setReadRemapLatlon(_params.remap_grid.nx,
			      _params.remap_grid.ny,
			      _params.remap_grid.minx,
			      _params.remap_grid.miny,
			      _params.remap_grid.dx,
			      _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      mdvx.setReadRemapFlat(_params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
			    _params.remap_rotation);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF) {
      mdvx.setReadRemapLambertConf(_params.remap_grid.nx,
				   _params.remap_grid.ny,
				   _params.remap_grid.minx,
				   _params.remap_grid.miny,
				   _params.remap_grid.dx,
				   _params.remap_grid.dy,
				   _params.remap_origin_lat,
				   _params.remap_origin_lon,
				   _params.remap_lat1,
				   _params.remap_lat2);
    } else if (_params.remap_projection == Params::PROJ_POLAR_STEREO) {
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (!_params.remap_pole_is_north) {
	poleType = Mdvx::POLE_SOUTH;
      }
      mdvx.setReadRemapPolarStereo(_params.remap_grid.nx,
				   _params.remap_grid.ny,
				   _params.remap_grid.minx,
				   _params.remap_grid.miny,
				   _params.remap_grid.dx,
				   _params.remap_grid.dy,
				   _params.remap_origin_lat,
				   _params.remap_origin_lon,
				   _params.remap_tangent_lon,
				   poleType,
				   _params.remap_central_scale);
    } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      mdvx.setReadRemapObliqueStereo(_params.remap_grid.nx,
				     _params.remap_grid.ny,
				     _params.remap_grid.minx,
				     _params.remap_grid.miny,
				     _params.remap_grid.dx,
				     _params.remap_grid.dy,
				     _params.remap_origin_lat,
				     _params.remap_origin_lon,
				     _params.remap_tangent_lat,
				     _params.remap_tangent_lon,
                                     _params.remap_central_scale);
    } else if (_params.remap_projection == Params::PROJ_MERCATOR) {
      mdvx.setReadRemapMercator(_params.remap_grid.nx,
				_params.remap_grid.ny,
				_params.remap_grid.minx,
				_params.remap_grid.miny,
				_params.remap_grid.dx,
				_params.remap_grid.dy,
				_params.remap_origin_lat,
				_params.remap_origin_lon);
    } else if (_params.remap_projection == Params::PROJ_TRANS_MERCATOR) {
      mdvx.setReadRemapTransverseMercator(_params.remap_grid.nx,
					  _params.remap_grid.ny,
					  _params.remap_grid.minx,
					  _params.remap_grid.miny,
					  _params.remap_grid.dx,
					  _params.remap_grid.dy,
					  _params.remap_origin_lat,
					  _params.remap_origin_lon,
					  _params.remap_central_scale);
    } else if (_params.remap_projection == Params::PROJ_ALBERS) {
      mdvx.setReadRemapAlbers(_params.remap_grid.nx,
			      _params.remap_grid.ny,
			      _params.remap_grid.minx,
			      _params.remap_grid.miny,
			      _params.remap_grid.dx,
			      _params.remap_grid.dy,
			      _params.remap_origin_lat,
			      _params.remap_origin_lon,
			      _params.remap_lat1,
			      _params.remap_lat2);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_AZIM) {
      mdvx.setReadRemapLambertAzimuthal(_params.remap_grid.nx,
					_params.remap_grid.ny,
					_params.remap_grid.minx,
					_params.remap_grid.miny,
					_params.remap_grid.dx,
					_params.remap_grid.dy,
					_params.remap_origin_lat,
					_params.remap_origin_lon);
    } else if (_params.remap_projection == Params::PROJ_VERT_PERSP) {
      mdvx.setReadRemapVertPersp(_params.remap_grid.nx,
                                 _params.remap_grid.ny,
                                 _params.remap_grid.minx,
                                 _params.remap_grid.miny,
                                 _params.remap_grid.dx,
                                 _params.remap_grid.dy,
                                 _params.remap_origin_lat,
                                 _params.remap_origin_lon,
                                 _params.remap_persp_radius);
    }

    if (_params.remap_set_offset_origin) {

      PjgMath *pmath = NULL;
      if (_params.remap_projection == Params::PROJ_FLAT) {
        pmath = new PjgAzimEquidistMath(_params.remap_origin_lat,
                                        _params.remap_origin_lon,
                                        _params.remap_rotation);
      } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF) {
        pmath = new PjgLambertConfMath(_params.remap_origin_lat,
                                       _params.remap_origin_lon,
                                       _params.remap_lat1,
                                       _params.remap_lat2);
      } else if (_params.remap_projection == Params::PROJ_POLAR_STEREO) {
        pmath = new PjgPolarStereoMath(_params.remap_tangent_lon,
                                       _params.remap_pole_is_north,
                                       _params.remap_central_scale);
      } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
        pmath = new PjgObliqueStereoMath(_params.remap_tangent_lat,
                                         _params.remap_tangent_lon,
                                         _params.remap_central_scale);
      } else if (_params.remap_projection == Params::PROJ_MERCATOR) {
        pmath = new PjgMercatorMath(_params.remap_origin_lat,
                                    _params.remap_origin_lon);
      } else if (_params.remap_projection == Params::PROJ_TRANS_MERCATOR) {
        pmath = new PjgTransMercatorMath(_params.remap_origin_lat,
                                         _params.remap_origin_lon,
                                         _params.remap_central_scale);
      } else if (_params.remap_projection == Params::PROJ_ALBERS) {
        pmath = new PjgAlbersMath(_params.remap_origin_lat,
                                  _params.remap_origin_lon,
                                  _params.remap_lat1,
                                  _params.remap_lat2);
      } else if (_params.remap_projection == Params::PROJ_LAMBERT_AZIM) {
        pmath = new PjgLambertAzimMath(_params.remap_origin_lat,
                                       _params.remap_origin_lon);
      } else if (_params.remap_projection == Params::PROJ_VERT_PERSP) {
        pmath = new PjgVertPerspMath(_params.remap_origin_lat,
                                     _params.remap_origin_lon,
                                     _params.remap_persp_radius);
      }
      if (pmath != NULL) {
        pmath->setOffsetOrigin(_params.remap_offset_origin_latitude,
                               _params.remap_offset_origin_longitude);
        mdvx.setReadFalseCoords(pmath->getFalseNorthing(),
                                pmath->getFalseEasting());
        delete pmath;
      }
      
    } else if (_params.remap_false_northing != 0.0 ||
               _params.remap_false_easting != 0.0) {

      mdvx.setReadFalseCoords(_params.remap_false_northing,
                              _params.remap_false_easting);

    }
        
  } // if (_params.remap_xy)
  
  if (_params.decimate) {
    mdvx.setReadDecimate(_params.decimate_max_nxy);
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

}

////////////////////////////////////////////
// remap

void MdvConvert::_remap(DsMdvx &mdvx)

{

  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_remap" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    if (_params.remap_projection == Params::PROJ_LATLON) {
      field->remap2Latlon(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      field->remap2Flat(_remapLut,
			_params.remap_grid.nx,
			_params.remap_grid.ny,
			_params.remap_grid.minx,
			_params.remap_grid.miny,
			_params.remap_grid.dx,
			_params.remap_grid.dy,
			_params.remap_origin_lat,
			_params.remap_origin_lon,
			_params.remap_rotation,
                        _params.remap_false_northing,
                        _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF)	{
      field->remap2LambertConf(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_lat1,
			       _params.remap_lat2,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_POLAR_STEREO) {
      Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
      if (!_params.remap_pole_is_north) {
	poleType = Mdvx::POLE_SOUTH;
      }
      field->remap2PolarStereo(_remapLut,
			       _params.remap_grid.nx,
			       _params.remap_grid.ny,
			       _params.remap_grid.minx,
			       _params.remap_grid.miny,
			       _params.remap_grid.dx,
			       _params.remap_grid.dy,
			       _params.remap_origin_lat,
			       _params.remap_origin_lon,
			       _params.remap_tangent_lon,
			       poleType,
			       _params.remap_central_scale,
                               _params.remap_false_northing,
                               _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_OBLIQUE_STEREO) {
      field->remap2ObliqueStereo(_remapLut,
				 _params.remap_grid.nx,
				 _params.remap_grid.ny,
				 _params.remap_grid.minx,
				 _params.remap_grid.miny,
				 _params.remap_grid.dx,
				 _params.remap_grid.dy,
				 _params.remap_origin_lat,
				 _params.remap_origin_lon,
				 _params.remap_tangent_lat,
				 _params.remap_tangent_lon,
                                 _params.remap_false_northing,
                                 _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_MERCATOR) {
      field->remap2Mercator(_remapLut,
			    _params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
                            _params.remap_false_northing,
                            _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_TRANS_MERCATOR) {
      field->remap2TransverseMercator(_remapLut,
				      _params.remap_grid.nx,
				      _params.remap_grid.ny,
				      _params.remap_grid.minx,
				      _params.remap_grid.miny,
				      _params.remap_grid.dx,
				      _params.remap_grid.dy,
				      _params.remap_origin_lat,
				      _params.remap_origin_lon,
				      _params.remap_central_scale,
                                      _params.remap_false_northing,
                                      _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_ALBERS) {
      field->remap2Albers(_remapLut,
			  _params.remap_grid.nx,
			  _params.remap_grid.ny,
			  _params.remap_grid.minx,
			  _params.remap_grid.miny,
			  _params.remap_grid.dx,
			  _params.remap_grid.dy,
			  _params.remap_origin_lat,
			  _params.remap_origin_lon,
			  _params.remap_lat1,
			  _params.remap_lat2,
                          _params.remap_false_northing,
                          _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_AZIM) {
      field->remap2LambertAzimuthal(_remapLut,
				    _params.remap_grid.nx,
				    _params.remap_grid.ny,
				    _params.remap_grid.minx,
				    _params.remap_grid.miny,
				    _params.remap_grid.dx,
				    _params.remap_grid.dy,
				    _params.remap_origin_lat,
				    _params.remap_origin_lon,
                                    _params.remap_false_northing,
                                    _params.remap_false_easting);
    } else if (_params.remap_projection == Params::PROJ_VERT_PERSP) {
      field->remap2VertPersp(_remapLut,
                             _params.remap_grid.nx,
                             _params.remap_grid.ny,
                             _params.remap_grid.minx,
                             _params.remap_grid.miny,
                             _params.remap_grid.dx,
                             _params.remap_grid.dy,
                             _params.remap_origin_lat,
                             _params.remap_origin_lon,
                             _params.remap_persp_radius,
                             _params.remap_false_northing,
                             _params.remap_false_easting);
     }
  }
  
}

////////////////////////////////////////////
// auto remap to latlon grid
//
// Automatically picks the grid resolution and extent
// from the existing data.

void MdvConvert::_autoRemapToLatLon(DsMdvx &mdvx)

{
  
  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    
    MdvxField *field = mdvx.getField(ifld);
    
    if (field == NULL) {
      cerr << "ERROR - MdvxConvert::_autoRemapToLatLon" << endl;
      cerr << "  Error remapping field #" << ifld <<
	" in output file" << endl;
      return;
    }
    
    field->autoRemap2Latlon(_remapLut);
  }
  
}

////////////////////////////////////////////
// invert all fields vertically

void MdvConvert::_invertVertically(DsMdvx &mdvx)

{
  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {

    MdvxField *field = mdvx.getField(ifld);
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    if (fhdr.nz == 1) continue;

    //I'm not sure if this is necessary, but probably won't hurt - Paul P.
    //If we don't uncompress, then the planes could be different sizes.
    //If speed is an issue, this can probably be done faster by not
    //decompressing the volume, but I didn't have time to test those
    //changes.

    field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE); 
    field->setPlanePtrs();
    
    int planeSize = field->getPlaneSize(0);
    int volumeSize = planeSize*fhdr.nz;
    ui08 *tmpVol = new ui08[volumeSize];
    Mdvx::vlevel_header_t vh = field->getVlevelHeader();    
    Mdvx::vlevel_header_t tmpvh(vh);

    for(int iv = 0; iv < fhdr.nz; iv++)
      {
	const void* plane = field->getPlane(iv);
	int planeOffset = field->getPlaneOffset(fhdr.nz-iv-1);

	memcpy(&tmpVol[planeOffset], plane, planeSize);
	tmpvh.level[fhdr.nz-iv-1] = vh.level[iv];
      }

    field->setVolData(tmpVol,volumeSize, Mdvx::ENCODING_FLOAT32);
    field->setVlevelHeader(tmpvh);	

  }

}

////////////////////////////////////////////
// override origin location

void MdvConvert::_overrideOrigin(DsMdvx &mdvx)

{

  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    MdvxField *field = mdvx.getField(ifld);
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    fhdr.proj_origin_lat = _params.origin_latitude_deg;
    fhdr.proj_origin_lon = _params.origin_longitude_deg;
    field->setFieldHeader(fhdr);
  }

}

////////////////////////////////////////////
// override vlevels

void MdvConvert::_overrideVlevels(DsMdvx &mdvx)

{

  for (size_t ifld = 0; ifld < mdvx.getNFields(); ifld++) {
    MdvxField *field = mdvx.getField(ifld);
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    Mdvx::vlevel_header_t vhdr = field->getVlevelHeader();
    if (fhdr.nz == _params.vlevel_array_n) {
      for (int iv = 0; iv < fhdr.nz; iv++) {
	vhdr.level[iv] = _params._vlevel_array[iv];
      }
      field->setVlevelHeader(vhdr);
      fhdr.dz_constant = 0;
      field->setFieldHeader(fhdr);
    } else {
      cerr << "WARNING - " << _progName << endl;
      cerr << "  Incorrect number of vlevels in override array." << endl;
      cerr << "  File has n vlevels: " << fhdr.nz << endl;
      cerr << "  You have supplied n vlevels: "
	   << _params.vlevel_array_n << endl;
      cerr << "  Vlevel override will not be done" << endl;
    }
  }

}

////////////////////////////////////////////
// apply linear transform

void MdvConvert::_applyTransform(DsMdvx &mdvx)
  
{
  
  for (int ii = 0; ii < _params.linear_transforms_n; ii++) {
    
    const char *fieldName = _params._linear_transforms[ii].field_name;
    MdvxField *field = mdvx.getField(fieldName);

    if (field != NULL) {
      
      double transformScale = _params._linear_transforms[ii].scale;
      double transformBias = _params._linear_transforms[ii].bias;
      
      field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

      Mdvx::field_header_t fhdr = field->getFieldHeader();
      double missing = fhdr.missing_data_value;
      double bad = fhdr.bad_data_value;

      fl32 *val = (fl32 *) field->getVol();
      int nPoints = fhdr.nx * fhdr.ny * fhdr.nz;
      for (int ii = 0; ii < nPoints; ii++, val++) {

	if (fabs(missing - *val) < 0.00001) {
	  *val = missing;
	}
	else if (fabs(bad - *val) < 0.00001) {
	  *val = bad;
	}
	else {
	  *val = (*val * transformScale) + transformBias;
	}
      }

      field->computeMinAndMax(true);

    }

  }

}

////////////////////////////////////////////
// convert to output types

void MdvConvert::_convertOutput(DsMdvx &mdvx)
  
{

  for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {
    
    MdvxField *field = mdvx.getField(ii);
    // Mdvx::field_header_t fhdr = field->getFieldHeader();
    const Mdvx::field_header_t *fhdrFile = field->getFieldHeaderFile();

    Mdvx::encoding_type_t encoding =
      (Mdvx::encoding_type_t) fhdrFile->encoding_type;
    if (_params.encoding_type != Params::ENCODING_ASIS) {
      encoding = (Mdvx::encoding_type_t)_params.encoding_type;
    }

    Mdvx::compression_type_t compression =
      (Mdvx::compression_type_t) fhdrFile->compression_type;
    if (_params.compression_type != Params::COMPRESSION_ASIS) {
      compression = (Mdvx::compression_type_t)_params.compression_type;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Setting output compression: "
             << Mdvx::compressionType2Str(compression) << endl;
      }
    }

    Mdvx::scaling_type_t scaling =
      (Mdvx::scaling_type_t) fhdrFile->scaling_type;
    if (_params.scaling_type != Params::SCALING_ASIS) {
      scaling = (Mdvx::scaling_type_t)_params.scaling_type;
    }

    if (field->convertType(encoding, compression, scaling,
                           _params.scale, _params.bias)) {
      cerr << "ERROR - MdvConvert::_convertOutput" << endl;
      cerr << field->getErrStr() << endl;
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--->>> output field <<<---" << endl;
      field->printHeaders(cerr);
      cerr << "--------------------------" << endl;
    }
    
  }

}

//////////////////////////////////////////////////////////
// rename fields

void MdvConvert::_renameFields(DsMdvx &mdvx)
{

  for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {

    MdvxField *field = mdvx.getField(ii);
      
    for (int jj = 0; jj < _params.new_names_n; jj++) {
      if ((strcmp(field->getFieldName(),
                 _params._new_names[jj].old_field_name)==0) ||
	(strcmp(field->getFieldNameLong(),
		_params._new_names[jj].old_field_name)==0)) {
	  field->setFieldName(_params._new_names[jj].new_field_name);
	  field->setFieldNameLong(_params._new_names[jj].new_field_name);
      }
    } // jj
    
  } // ii

}

//////////////////////////////////////////////////////////
// apply thresholds to specified fields

void MdvConvert::_applyThresholds(DsMdvx &mdvx)

{

  for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {

    MdvxField *field = mdvx.getField(ii);
      
    for (int jj = 0; jj < _params.thresholded_fields_n; jj++) {
      
      if (strcmp(field->getFieldName(),
                 _params._thresholded_fields[jj].output_field_name) == 0) {
        
        field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
        
        double minVal = _params._thresholded_fields[jj].threshold_min;
        double maxVal = _params._thresholded_fields[jj].threshold_max;
        
        Mdvx::field_header_t fhdr = field->getFieldHeader();
        double missing = fhdr.missing_data_value;

        fl32 *val = (fl32 *) field->getVol();
        int nPoints = fhdr.nx * fhdr.ny * fhdr.nz;
        for (int ii = 0; ii < nPoints; ii++, val++) {
          if (*val < minVal || *val > maxVal) {
            *val = missing;
          }
        } // ii

      } // if (strcmp ...

      field->computeMinAndMax(true);

    } // jj
        
  } // ii

}

////////////////////////////////////////////
// Initialize the data trigger object

bool MdvConvert::_initTrigger()
{

  switch (_params.mode) {

    case Params::REALTIME :
    case Params::REALTIME_FCST_DATA: {
      DsLdataTrigger *trigger = new DsLdataTrigger();
      if (trigger->init(_params.input_url, 600,
			PMU_auto_register) != 0) {
	cerr << trigger->getErrStr() << endl;
	return false;
      }
      _dataTrigger = trigger;
      break;
    }
    
    case Params::LOCAL_FILEPATH_REALTIME: {
      _inputPath = new DsInputPath(_progName,
                                   (bool)_params.debug,
                                   _params.input_url,
                                   _params.local.lookback,
                                   PMU_auto_register,
				   (bool)_params.local.use_ldata_info,
				   (bool)_params.local.latest_file_only);
      if (!_inputPath) {
        cerr << "Failed to instantiate DsInputPath "
             << "object for LOCAL_FILEPATH_REALTIME" << endl;
        return false;
      }
      
      break;
    }
      
    case Params::ARCHIVE: {
      DsTimeListTrigger *trigger = new DsTimeListTrigger();
      if (trigger->init(_params.input_url,
                        _args.startTime, _args.endTime) != 0) {
        cerr << trigger->getErrStr() << endl;
        cerr << "  URL: " << _params.input_url << endl;
        cerr << "  start time: " << DateTime::str(_args.startTime) << endl;
        cerr << "  end time: " << DateTime::str(_args.endTime) << endl;
        return false;
      }
      _dataTrigger = trigger;
      break;
    }
      
    case Params::ARCHIVE_FCST:
    case Params::SPEC_FCST_ARCHIVE: {
      DsFcstTimeListTrigger *trigger = new DsFcstTimeListTrigger();
      if (trigger->init(_params.input_url,
                        _args.startTime, _args.endTime) != 0) {
        cerr << trigger->getErrStr() << endl;
        cerr << "  URL: " << _params.input_url << endl;
        cerr << "  start time: " << DateTime::str(_args.startTime) << endl;
        cerr << "  end time: " << DateTime::str(_args.endTime) << endl;
        return false;
      }
      _dataTrigger = trigger;
      break;
    }
      
    case Params::FILELIST: {
      DsFileListTrigger *trigger = new DsFileListTrigger();
      if (trigger->init(_args.inputFileList) != 0) {
        cerr << trigger->getErrStr() << endl;
        return false;
      }
      _dataTrigger = trigger;
      break;
    }
      
    case Params::SPEC_FCAST_REALTIME: {
      DsSpecificFcstLdataTrigger *trigger =
        new DsSpecificFcstLdataTrigger();
      vector< int > fcast_lead_times;
      fcast_lead_times.push_back(_params.fcast_lead_time.lead_time_secs);
      if (trigger->init(_params.input_url,
                        fcast_lead_times,
                        _params.fcast_lead_time.use_gen_time,
                        600, PMU_auto_register) != 0) {
        cerr << trigger->getErrStr() << endl;
        return false;
      }
      _dataTrigger = trigger;
      break;
    }
      
  } /* endswitch - _params.mode */

  return true;

}

// if lead times are subsampled, checks if this lead time is wanted

bool MdvConvert::_wantedLeadTime(int leadTime) const
{
  if (_params.do_lead_time_subsampling && 
      _params.subsample_lead_time_hour_n > 0)
  {
    for (int i=0; i<_params.subsample_lead_time_hour_n; ++i)
    {
      int lt = static_cast<int>(_params._subsample_lead_time_hour[i]*3600.0);
      if (lt == leadTime)
      {
	return true;
      }
    }
    cerr << "Ignore unwanted lead time " << leadTime << endl;
    return false;
  }
  else
  {
    return true;
  }
}

/////////////////////////////////////////////////////
// write out data in CEDRIC format

int MdvConvert::_writeCedricFile(DsMdvx &mdvx)
{

  if (mdvx.getNFields() < 1) {
    cerr << "ERROR - MdvConvert::_writeCedricFile" << endl;
    cerr << "  No fields in data." << endl;
    return -1;
  }
  
  Cedric ced;
  if (_params.debug) {
    ced.setDebug(true);
  }

  // set meta data
  
  ced.setDateTimeRun(time(NULL));
  ced.setVolStartTime(mdvx.getMasterHeader().time_begin);
  ced.setVolEndTime(mdvx.getMasterHeader().time_end);
  ced.setTapeStartTime(mdvx.getMasterHeader().time_begin);
  ced.setTapeEndTime(mdvx.getMasterHeader().time_end);
  ced.setTimeZone("UTC");
  
  ced.setVolumeNumber(_params.cedric_volume_number);
  ced.setVolName(_params.cedric_volume_name);
  ced.setRadar(_params.cedric_radar_name);
  ced.setProject(_params.cedric_project_name);
  ced.setProgram(_params.cedric_program_name);

  // field and vlevel headers

  const MdvxField *field0 = mdvx.getFieldByNum(0);
  if (field0 == NULL) {
    cerr << "ERROR - MdvConvert::_writeCedricFile" << endl;
    cerr << "  No fields in data." << endl;
    return -1;
  }
  Mdvx::field_header_t fhdr0 = field0->getFieldHeader();
  Mdvx::vlevel_header_t vhdr0 = field0->getVlevelHeader();
  MdvxProj proj0(fhdr0);
  if (fhdr0.nx < 1) {
    cerr << "ERROR - MdvConvert::_writeCedricFile" << endl;
    cerr << "  No vert levels in data." << endl;
    return -1;
  }

  // projection

  if (fhdr0.proj_type == Mdvx::PROJ_LATLON) {
    ced.setCoordType("LLE ");
  } else {
    ced.setCoordType("ELEV");
  }
  ced.setXaxisAngleFromNDeg(90.0);
  ced.setOriginX(0.0);
  ced.setOriginY(0.0);

  // origin

  ced.setLongitudeDeg(fhdr0.proj_origin_lon);
  ced.setLatitudeDeg(fhdr0.proj_origin_lat);
  ced.setOriginHtKm(fhdr0.grid_minz);
  ced.setNyquistVel(_params.cedric_nyquist_velocity);

  // ced.addLandmark(_params.cedric_radar_name,
  //                 _radarX, _radarY, _readVol.getAltitudeKm());

  // range limits

  double minLat, minLon, maxLat, maxLon;
  proj0.getEdgeExtrema(minLat, minLon, maxLat, maxLon);
  
  double minX, minY, maxX, maxY;
  PJGLatLon2DxDy(fhdr0.proj_origin_lat, fhdr0.proj_origin_lon,
                 minLat, minLon, &minX, &minY);
  PJGLatLon2DxDy(fhdr0.proj_origin_lat, fhdr0.proj_origin_lon,
                 maxLat, maxLon, &maxX, &maxY);
  double distToLowerLeft = sqrt(minX * minX + minY * minY);
  double distToUpperRight = sqrt(maxX * maxX + maxY * maxY);
  double maxRange = distToLowerLeft;
  if (distToUpperRight > distToLowerLeft) {
    maxRange = distToUpperRight;
  }
  ced.setMinRangeKm(0);
  ced.setMaxRangeKm(maxRange);

  
  ced.setNumGatesBeam(_params.cedric_ngates_beam);
  ced.setGateSpacingKm(_params.cedric_gate_spacing_km);
  ced.setMinAzimuthDeg(0);
  ced.setMaxAzimuthDeg(360);

  ced.setMinX(fhdr0.grid_minx);
  ced.setMaxX(fhdr0.grid_minx + (fhdr0.nx - 1) * fhdr0.grid_dx);
  ced.setNx(fhdr0.nx);
  ced.setDx(fhdr0.grid_dx);
  ced.setFastAxis(1);
  
  ced.setMinY(fhdr0.grid_miny);
  ced.setMaxY(fhdr0.grid_miny + (fhdr0.ny - 1) * fhdr0.grid_dy);
  ced.setNy(fhdr0.ny);
  ced.setDy(fhdr0.grid_dy);
  ced.setMidAxis(2);

  ced.setMinZ(vhdr0.level[0]);
  ced.setMaxZ(vhdr0.level[fhdr0.nz-1]);
  ced.setNz(fhdr0.nz);
  ced.setDz(vhdr0.level[1] - vhdr0.level[0]);
  ced.setSlowAxis(3);
  ced.setPlaneType("PP");

  ced.setNumElevs(_params.cedric_elevation_angles_n);
  double minElev = 9999;
  double maxElev = -9999;
  double sumElev = 0.0;
  double sumDeltaElev = 0.0;
  double prevElev = -9999;
  for (int ii = 0; ii < _params.cedric_elevation_angles_n; ii++) {
    double elev = _params._cedric_elevation_angles[ii];
    sumElev += elev;
    if (elev > maxElev) maxElev = elev;
    if (elev < minElev) minElev = elev;
    if (prevElev > -9990) {
      double deltaElev = fabs(elev - prevElev);
      sumDeltaElev += deltaElev;
    }
    prevElev = elev;
  }
  ced.setMinElevDeg(minElev);
  ced.setMaxElevDeg(maxElev);
  ced.setAveElevDeg(sumElev / _params.cedric_elevation_angles_n);
  ced.setAveDeltaElevDeg(sumDeltaElev / _params.cedric_elevation_angles_n);
  
  for (int ii = 0; ii < fhdr0.nz; ii++) {
    ced.addVlevel(ii, vhdr0.level[ii],
                  mdvx.getNFields(),
                  fhdr0.nx, fhdr0.ny,
                  _params.cedric_nyquist_velocity);
  }

  ced.setNumRadars(1);
  // if (_readVol.getNRcalibs() > 0) {
  //   ced.setRadarConst(_readVol.getRcalibs()[0]->getRadarConstantH());
  // }

  // add fields

  for (size_t ifield = 0; ifield < mdvx.getNFields(); ifield++) {

    MdvxField *field = mdvx.getFieldByNum(ifield);
    if (field == NULL) {
      cerr << "ERROR - MdvConvert::_writeCedricFile" << endl;
      cerr << "  Cannot find field number: " << ifield << endl;
      return -1;
    }
    field->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
    Mdvx::field_header_t fhdr = field->getFieldHeader();
    fl32 missingFl32 = fhdr.missing_data_value;
    ced.addField(fhdr.field_name, (fl32 *) field->getVol(), missingFl32);

  } // ifield

  if (_params.debug) {
    cerr << "  Writing CEDRIC file ... " << endl;
  }
  
  if (_params.write_to_path) {
    string outputPath = _params.output_path;
    if (ced.writeToDir(outputPath, _progName)) {
      cerr << "ERROR - MdvConvert::_writeCedricFile()" << endl;
      return -1;
    }
  } else {
    DsURL outUrl(_params.output_url);
    if (ced.writeToDir(outUrl.getFile(), _progName)) {
      cerr << "ERROR - CartInterp::_writeCedricFile()" << endl;
      return -1;
    }
  }

  return 0;

}

