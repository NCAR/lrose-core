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
// Wrf2Mdv.cc
//
// Wrf2Mdv object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/pjg.h>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>

#include "InputPath.hh"
#include "OutputFile.hh"
#include "Wrf2Mdv.hh"

using namespace std;

/////////////////
// Constructor

Wrf2Mdv::Wrf2Mdv() :
  _rotateOutputUV(false)
{
  // set programe name

  _progName = "Wrf2Mdv";
  ucopyright((char *) _progName.c_str());

}


/////////////////
// init()

bool Wrf2Mdv::init(int argc, char **argv)
{
  static const string method_name = "Wrf2Mdv::init()";
  
  // Get command line args

  if (_args.parse(argc, argv, _progName))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;

    return false;
  }
  
  // Get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;

    return false;
  }
  
  // Set up the field name map from the config info

  _initFieldNameMap();

  // Check the parameters

  if (!_checkParams())
    return false;
  
  // set up vertical level interpolation 

  if (!_initVertInterp())
    return false;
  
  // initialize the output grid computations

  if (_params.output_projection == Params::OUTPUT_PROJ_FLAT)
  {
    _outputProj.initFlat(_params.output_origin.lat,
			 _params.output_origin.lon,
			 0.0);
  }
  else if (_params.output_projection == Params::OUTPUT_PROJ_LAMBERT)
  {
    _outputProj.initLc2(_params.output_origin.lat,
			_params.output_origin.lon,
			_params.lambert_lat1,
			_params.lambert_lat2);
    _rotateOutputUV = true;
  }
  else if (_params.output_projection == Params::OUTPUT_PROJ_MERCATOR)
  {
    _outputProj.initMercator(_params.output_origin.lat,
			     _params.output_origin.lon);
  }
  else
  {
    _outputProj.initLatlon();
  }

  // init process mapper registration
  
  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		_params.Procmap_reg_interval_secs);

  cout << "Reg with procmap, progname: " << _progName.c_str()
       << ", instance: " << _params.instance
       << ", interval(secs): " << _params.Procmap_reg_interval_secs << endl;
  
  PMU_auto_register("Starting ...");

  return true;
}

/////////////////
// destructor

Wrf2Mdv::~Wrf2Mdv()
{
  // Free space in the field name map

  for (int j = 0; j< Params::TOTAL_FIELD_COUNT; j++)
  {
    delete [] _field_name_map[j].name;
    delete [] _field_name_map[j].long_name;
  }

  delete [] _field_name_map;

  // unregister process

  PMU_auto_unregister();
}

//////////////////////////////////////////////////
// Run

bool Wrf2Mdv::Run ()
{

  PMU_auto_register("Wrf2Mdv::Run");

  // create input path object

  InputPath *inputPath;
  if (_params.mode == Params::REALTIME)
    inputPath = new InputPath(_progName, _params,
			      PMU_auto_register);
  else
    inputPath = new InputPath(_progName, _params,
			      _args.nFiles, _args.filePaths);
  
  // run

  bool iret = _run(*inputPath);

  // clean up

  delete inputPath;

  return iret;
}

//////////////////////////////////////////////////
// check the parameters
//
// returns true on success, false on failure
  
bool Wrf2Mdv::_checkParams()
{
  // make sure START_2D_FIELDS & TOTAL_FIELD_COUNT are not selected

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++)
  {
    if (_params._output_fields[ifield].name == Params::START_2D_FIELDS)
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Do not select START_2D_FIELDS." << endl;
      cerr << "  This is only a marker." << endl;

      return false;
    }

    if (_params._output_fields[ifield].name == Params::TOTAL_FIELD_COUNT)
    {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Do not select TOTAL_FIELD_COUNT." << endl;
      cerr << "  This is only a marker." << endl;

      return false;
    }
  }

  // make sure EDGE fields are only selected for NATIVE projection

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++)
  {
    if (_params._output_fields[ifield].name == Params::U_EDGE_FIELD ||
	_params._output_fields[ifield].name == Params::V_EDGE_FIELD)
    {
      if (_params.output_projection != Params::OUTPUT_PROJ_NATIVE)
      {
	cerr << "ERROR: " << _progName << endl;
	cerr << "  Output projection is not NATIVE." << endl;
	cerr <<"  Therefore you cannot select U_EDGE or V_EDGE output fields." << endl;

	return false;
      }
    }
  } // ifield

  return true;
}


//////////////////////////////////////////////////
// _initVertInterp()
  
bool Wrf2Mdv::_initVertInterp()
{
  switch (_params.output_levels)
  {
  case Params::FLIGHT_LEVELS:
  {
    TaArray<double> levels_;
    double *levels = levels_.alloc(_params.flight_levels_n);
    for (int i = 0; i < _params.flight_levels_n; i++)
      levels[i] = _params._flight_levels[i];

    _presInterp.setFlightLevels(_params.flight_levels_n, levels);
    if (_params.debug >= Params::DEBUG_VERBOSE)
      _presInterp.printPressureArray(cerr);

    break;
  }

  case Params::PRESSURE_LEVELS:
  {
    TaArray<double> levels_;
    double *levels = levels_.alloc(_params.pressure_levels_n);
    for (int i = 0; i < _params.pressure_levels_n; i++)
      levels[i] = _params._pressure_levels[i];

    _presInterp.setPressureLevels(_params.pressure_levels_n, levels);
    if (_params.debug >= Params::DEBUG_VERBOSE)
      _presInterp.printPressureArray(cerr);

    break;
  }

  case Params::HEIGHT_LEVELS:
  {
    TaArray<double> levels_;
    double *levels = levels_.alloc(_params.height_levels_n);
    for (int i = 0; i < _params.height_levels_n; i++)
      levels[i] = _params._height_levels[i] * 1000.0;

    _presInterp.setHeightLevels(_params.height_levels_n, levels);
    if (_params.debug >= Params::DEBUG_VERBOSE)
      _presInterp.printPressureArray(cerr);

    break;
  }

  default:
    break;
    
  } // switch

  return true;
}


//////////////////////////////////////////////////
// _run

bool Wrf2Mdv::_run(InputPath &input_path)
{
  static const string method_name = "Wrf2Mdv::_run()";
  
  PMU_auto_register("Wrf2Mdv::_run");
  
  // loop through the input files
  
  string filePath;
  while ((filePath = input_path.next()) != "")
  {
    Path path(filePath);
    
    string base = path.getBase();
    if (_params.debug >= Params::DEBUG_NORM)
      cerr << "File base name = <" << base << ">" << endl;
    
    PMU_force_register((string("Processing file ") + path.getPath()).c_str());

    if (_params.debug >= Params::DEBUG_NORM)
      cerr << "Processing file " << filePath << endl;
    
    WRFData *in_data = new WRFData();
    if (!in_data->init(_progName, _params, PMU_auto_register))
    {
      cerr << "ERROR - " << method_name << endl;
      cerr << "Error initializing WRFData object" << endl;
      
      return false;
    }
    
    if (!in_data->initFile(filePath))
      return false;

    PMU_auto_register("Processing inData");

    if (!_processInData(*in_data))
    {
      cerr << "ERROR - Wrf2Mdv::_run" << endl;
      cerr << "  Processing file: " << filePath << endl;
      umsleep(2);
      break;
    }

    in_data->clearData();
  
    delete in_data;
    
  } // endwhile
    
  return true;
}

/////////////////////////////////////////
// _processInData()
//
// Process input data
//
// Returns 0 on success, -1 on failure

bool Wrf2Mdv::_processInData(WRFData &inData)
{
  static const string method_name = "Wrf2Mdv::_processInData()";
  
  // read in data set

  if (!inData.read())
    return false;
    
  // check projection
  // native proj only OK for Lambert, stereographic, and Mercator
    
  if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE &&
      inData.getProjType() != WRFData::LAMBERT_CONF &&
      inData.getProjType() != WRFData::STEREOGRAPHIC &&
      inData.getProjType() != WRFData::MERCATOR)
  {
    cerr << "ERROR - " << method_name << endl;
    cerr << "  Output projection is set to NATIVE." << endl;
    cerr << "  Input WRF data file has projection UNKNOWN: "
	 << inData.getProjType() << endl;
    cerr << "  Only Lambert, stereographic, and Mercator projections "
	 << "are supported." << endl;
    return false;
  }
  
  // round the lead_time to nearest 15 mins

  int rounded_lead_time =
    ((int)((inData.forecastTime - inData.genTime) / 900.0 + 0.5)) * 900;
  
  // check lead time
  
  if (!_acceptLeadTime(rounded_lead_time))
  {
    if (_params.debug)
      cerr << "Rejecting file at lead time (secs): "
	   << rounded_lead_time << endl;

    return true;
  }

  _processForecast(inData, inData.genTime, rounded_lead_time,
		   inData.forecastTime);

  return true;
}

/////////////////////////////////////////
// _acceptLeadTime()
//
// Check that this lead time is accepted

bool Wrf2Mdv::_acceptLeadTime(int lead_time)
{
  if (lead_time < _params.min_forecast_dtime)
    return false;

  if (lead_time > _params.max_forecast_dtime)
    return false;

  if (!_params.specify_lead_times)
    return true;

  for (int i = 0; i < _params.lead_times_n; i++)
  {
    if (lead_time == _params._lead_times[i])
      return true;
  } // i

  return false;
}

/////////////////////////////////////////
// _processForecast()
//
// Process data for a given forecast

int Wrf2Mdv::_processForecast(WRFData &inData,
				time_t gen_time,
				int lead_time,
				time_t forecast_time)
{
  PMU_auto_register("In Wrf2Mdv::_processForecast");

  // create the output file

  OutputFile outFile(_progName, _params, gen_time, forecast_time, 
		     lead_time, inData, _field_name_map);

  DsMdvx &mdvx = outFile.getDsMdvx();

  // load up fields on cross (center) grid points

  _loadCrossOutputFields(inData, mdvx);
  
  // load up fields on dot (edge) grid points

  _loadEdgeOutputFields(inData, mdvx);
  
  // convert to desired output types

  Mdvx::compression_type_t compression_type;
  switch (_params.output_compression)
  {
  case Params::_MDV_COMPRESSION_NONE:
    compression_type = Mdvx::COMPRESSION_NONE;
    break;
  case Params::_MDV_COMPRESSION_RLE:
    compression_type = Mdvx::COMPRESSION_RLE;
    break;
  case Params::_MDV_COMPRESSION_LZO:
    compression_type = Mdvx::COMPRESSION_LZO;
    break;
  case Params::_MDV_COMPRESSION_ZLIB:
    compression_type = Mdvx::COMPRESSION_ZLIB;
    break;
  case Params::_MDV_COMPRESSION_BZIP:
    compression_type = Mdvx::COMPRESSION_BZIP;
    break;
  default:
    compression_type = Mdvx::COMPRESSION_NONE;
  }
  
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++)
  {
    PMU_auto_register("Wrf2Mdv::_processForecast: converting fields");

    MdvxField *field = mdvx.getFieldByNum(ifield);

    if (_params._output_fields[ifield].encoding == Params::OUT_INT8)
    {
      if (_params.output_scaling == Params::SCALING_ROUNDED)
        field->convertRounded(Mdvx::ENCODING_INT8, compression_type);
      else
        field->convertDynamic(Mdvx::ENCODING_INT8, compression_type);
    } else if (_params._output_fields[ifield].encoding == Params::OUT_INT16)
    {
      if (_params.output_scaling == Params::SCALING_ROUNDED)
        field->convertRounded(Mdvx::ENCODING_INT16, compression_type);
      else
        field->convertDynamic(Mdvx::ENCODING_INT16, compression_type);
    }
    else if (_params._output_fields[ifield].encoding == Params::OUT_FLOAT32)
    {
      field->convertRounded(Mdvx::ENCODING_FLOAT32, compression_type);
    }

  } // ifield
  
  // write out volume
  
  if (outFile.writeVol())
    cerr << "Cannot write output file" << endl;

  return 0;
}

/////////////////////////////////////////////////
// Load up output fields on cross (center) points

void Wrf2Mdv::_loadCrossOutputFields(WRFData &inData,
				       DsMdvx &mdvx)
{
  static const string method_name = "Wrf2Mdv::_loadCrossOutputFields()";
  
  if (_rotateOutputUV)
    inData.computeUVOutput(_outputProj);

  // set up grid remapping object
  
  WRFGrid mGrid(_progName, _params.debug >= Params::DEBUG_VERBOSE, inData);
  
  // get representative field header
  
  const Mdvx::field_header_t *fhdr = NULL;
  for (size_t ifield = 0; ifield < mdvx.getNFields(); ifield++)
  {
    MdvxField *fld = mdvx.getField(ifield);
    // make sure we don't have an edge field
    if (!strstr(fld->getFieldName(), "_EDGE"))
    {
      fhdr = &fld->getFieldHeader();
      break;
    }
  } // ifields

  if (fhdr == NULL)
  {
    // no non-edge fields
    return;
  }

  // loop through the grid
  
  int nPointsPlane = fhdr->ny * fhdr->nx;
  if (_params.debug == Params::DEBUG_VERBOSE)
  {
    cerr << "In " << method_name << " nPointsPlane = " << nPointsPlane
	 << " ny = " << fhdr->ny << " nx = " << fhdr->nx << endl;
  }

  for (int iy = 0; iy < fhdr->ny; iy++)
  {
    PMU_auto_register("In Wrf2Mdv::_loadCrossOutputFields loop");

    int planeOffset = iy * fhdr->nx;

    for (int ix = 0; ix < fhdr->nx; ix++, planeOffset++)
    {
      // set up grid interp object

      if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE)
      {
	mGrid.setNonInterp(iy, ix);
      }
      else
      {
	// compute x and y, and plane offset
	double yy = fhdr->grid_miny + iy * fhdr->grid_dy;
	double xx = fhdr->grid_minx + ix * fhdr->grid_dx;
	// compute latlon
	double lat, lon;
	_outputProj.xy2latlon(xx, yy, lat, lon);
	// find the model position for this point
	if (!mGrid.getGridIndices(lat, lon))
	{
	  // cannot process this grid point
	  continue;
	}
      }

      // set up vert interp

      if (_params.output_levels != Params::NATIVE_VERTICAL_LEVELS)
      {
	// load up interpolated vertical pressure array for this point
	vector<double> presVert;
	if (inData.dataDimension() > 2)
	{
	  inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			       "press", inData.getPres(),
			       mGrid.wtSW, mGrid.wtNW,
			       mGrid.wtNE, mGrid.wtSE,
			       presVert);
	  // load up the vertical interpolation array if interp3dField is successful
	  _presInterp.prepareInterp(presVert);
	}
      }
      // interp the fields for this point

      for (int ifield = 0; ifield < _params.output_fields_n; ifield++)
      {
	switch (_params._output_fields[ifield].name)
	{
	  // raw 3d fields

	case Params::U_FIELD:
          if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE)
	  {
            _interp3dField(inData, _params._output_fields[ifield].name,
			   inData.getUu(), mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          }
	  else if (_rotateOutputUV)
	  {
            _interp3dField(inData, _params._output_fields[ifield].name,
			   inData.getUuOut(), mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          }
	  else
	  {
            _interp3dField(inData, _params._output_fields[ifield].name,
			   inData.getUuTn(), mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          }
	  break;
	    
	case Params::V_FIELD:
          if (_params.output_projection == Params::OUTPUT_PROJ_NATIVE)
	  {
            _interp3dField(inData, _params._output_fields[ifield].name,
			   inData.getVv(), mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          }
	  else if (_rotateOutputUV)
	  {
            _interp3dField(inData, _params._output_fields[ifield].name,
			   inData.getVvOut(), mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          }
	  else
	  {
            _interp3dField(inData, _params._output_fields[ifield].name,
			   inData.getVvTn(), mdvx, planeOffset,
                           nPointsPlane, fhdr->missing_data_value, mGrid);
          }
	  break;
	    
	case Params::Q_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getQq(), mdvx, planeOffset,
			 nPointsPlane, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::CLW_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getClw(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::RNW_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getRnw(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::ICE_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getIce(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::QNRAIN_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getNRain(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::QNCLOUD_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getNCloud(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SNOW_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getSnow(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::GRAUPEL_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getGraupel(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::W_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getWw(), mdvx, planeOffset,
			 nPointsPlane, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::P_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getPp(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::PB_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getPb(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::ITFADEF_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getItfadef(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
    
	case Params::PHB_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getPhb(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::PH_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getPhC(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::DNW_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getDNW(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::MUB_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getMUB(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::MU_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getMU(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);

	case Params::REFL_10CM_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getREFL3D(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	    
	  // derived 3d fields

	case Params::TK_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getTk(), mdvx, planeOffset,
			 nPointsPlane, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::TC_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getTc(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::WSPD_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getWspd(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::WDIR_FIELD:
	  _interp3dField(inData,_params._output_fields[ifield].name,
			 inData.getWdir(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::PRESSURE_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getPres(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::RH_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getRh(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SPEC_H_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getSpecH(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::DEWPT_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getDewpt(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::ICING_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getIcing(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::CLW_G_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getClwG(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::RNW_G_FIELD:
	  _interp3dField(inData,_params._output_fields[ifield].name,
			 inData.getRnwG(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::THETA_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getTheta(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;

	case Params::DBZ_3D_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getDbz3d(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::HGT_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getGeoHgt(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;
	case Params::GEO_POT_FIELD:
	  _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getGeoPot(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);
	  break;

        case Params::Z_AGL_FIELD:
          _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getZz(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);

	  break;

        case Params::Q_G_FIELD:
          _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getQG(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);

	  break;

        case Params::CIN_3D_FIELD:
          _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getCin3d(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);

	  break;

        case Params::CAPE_3D_FIELD:
          _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getCape3d(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);

	  break;

        case Params::LCL_3D_FIELD:
          _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getLcl3d(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);

	  break;

        case Params::LFC_3D_FIELD:
          _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getLfc3d(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);

	  break;

        case Params::EL_3D_FIELD:
          _interp3dField(inData, _params._output_fields[ifield].name,
			 inData.getEl3d(), mdvx,
			 planeOffset, nPointsPlane, 
			 fhdr->missing_data_value, mGrid);

	  break;


	// raw 2d fields

	case Params::START_2D_FIELDS:
	  break;

	case Params::SOIL_T_1_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilT1(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_T_2_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilT2(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SOIL_T_3_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilT3(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SOIL_T_4_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilT4(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SOIL_T_5_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilT5(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SOIL_M_1_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilM1(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SOIL_M_2_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilM2(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SOIL_M_3_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilM3(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SOIL_M_4_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilM4(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
       	case Params::SOIL_M_5_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilM5(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

       	case Params::SOIL_AM_1_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilAM1(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

       	case Params::SOIL_AM_2_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilAM2(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

       	case Params::SOIL_AM_3_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilAM3(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

       	case Params::SOIL_AM_4_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilAM5(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

       	case Params::SOIL_AM_5_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilAM5(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LAT_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLat(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::LON_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLon(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::GROUND_T_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getGroundT(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::RAINC_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getRainC(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::RAINNC_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getRainNC(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::TERRAIN_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getTerrain(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LAND_USE_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLandUse(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SNOWCOVR_FIELD:
	  _interp2dField(inData,_params._output_fields[ifield].name,
			 inData.getSnowCovr(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::TSEASFC_FIELD:
	  _interp2dField(inData,_params._output_fields[ifield].name,
			 inData.getTSeaSfc(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::PBL_HGT_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getPblHgt(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::T2_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getT2(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::Q2_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getQ2(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::U10_FIELD:
	  _interp2dField(inData,_params._output_fields[ifield].name,
			 inData.getU10(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::V10_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getV10(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SNOWH_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSnowH(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SFC_PRES_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSurfP(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
 	case Params::LAND_MASK_FIELD:
 	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLandMask(), mdvx,
 			 planeOffset, fhdr->missing_data_value, mGrid);
 	  break;
	    
	case Params::TH2_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getTh2(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::HFX_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getHfx(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LH_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLh(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::SNOW_WE_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSnowWE(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SNOW_NC_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSnowNC(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::GRAUPEL_NC_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getGraupelNC(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

       	case Params::SOIL_TYPE_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSoilType(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	  // derived 2d fields
	    
	case Params::FZLEVEL_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getFzLevel(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    
	case Params::RAIN_TOTAL_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getRainTotal(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	    	
	case Params::T2C_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getT2C(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	  
	case Params::DBZ_2D_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getDbz2D(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
	  
	case Params::RH2_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getRH2(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::SPEC_H_2M_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getSpecH2M(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::WSPD10_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getWspd10(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::WDIR10_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getWdir10(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

        case Params::CIN_FIELD:
          _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getCin(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

        case Params::CAPE_FIELD:
          _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getCape(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

        case Params::LCL_FIELD:
          _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLcl(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);

	  break;

        case Params::LFC_FIELD:
          _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLfc(), mdvx,
			 planeOffset,  fhdr->missing_data_value, mGrid);

	  break;

        case Params::EL_FIELD:
          _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getEl(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);

	  break;

	  // GEOGRID 2-d fields

	case Params::LANDUSEF_1_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLanduseF1(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LANDUSEF_2_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLanduseF2(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LANDUSEF_6_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLanduseF6(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::LANDUSEF_15_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getLanduseF15(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;


	case Params::GREENFRAC_7_FIELD:
	  _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getGreenFrac7(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::TWP_FIELD:
          _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getTwp(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;
  
        case Params::RWP_FIELD:
          _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getRwp(), mdvx,
			 planeOffset, fhdr->missing_data_value, mGrid);
	  break;

	case Params::VIL_FIELD:
          _interp2dField(inData, _params._output_fields[ifield].name,
			 inData.getVil(), mdvx,
                         planeOffset, fhdr->missing_data_value, mGrid);
          break;

	default:
	  break;

	} // switch
      } // ifield
    } // ix
  } // iy

}

/////////////////////////////////////////////////
// Load up output fields on edge points

void Wrf2Mdv::_loadEdgeOutputFields(WRFData &inData,
				     DsMdvx &mdvx)
{
  _loadUEdgeOutputFields(inData,mdvx);
  _loadVEdgeOutputFields(inData,mdvx);
}

void Wrf2Mdv::_loadUEdgeOutputFields(WRFData &inData,
				     DsMdvx &mdvx)
{
  if (_params.output_projection != Params::OUTPUT_PROJ_NATIVE)
  {
    // only applies to native projection
    return;
  }
  
  // set up grid remapping object

  WRFGrid mGrid(_progName, _params.debug >= Params::DEBUG_VERBOSE,inData);
  
  // get representative field header
  
  const Mdvx::field_header_t *fhdr = NULL;
  for (size_t ifield = 0; ifield < mdvx.getNFields(); ifield++)
  {
    MdvxField *fld = mdvx.getField(ifield);
  
    // make sure we have a U EDGE field

    if (strstr(fld->getFieldName(), "U_EDGE"))
    {
      fhdr = &fld->getFieldHeader();
      break;
    }
  } // ifields

  if (fhdr == NULL)
  {
    // no U-edge fields
    return;
  }
  
  // loop through the grid
  
  int nPointsPlane = fhdr->ny * fhdr->nx;
  if (_params.debug == Params::DEBUG_VERBOSE)
  {
    cerr << "In Wrf2Mdv::_loadUEdgeOutputFields() "
	 << "nPointsPlane = " << nPointsPlane
	 << " ny = " << fhdr->ny << " nx = " << fhdr->nx << endl;
  }
 
  for (int iy = 0; iy < fhdr->ny; iy++)
  {
    PMU_auto_register("In Wrf2Mdv::_loadUEdgeOutputFields loop");
    int planeOffset = iy * fhdr->nx;
    
    for (int ix = 0; ix < fhdr->nx; ix++, planeOffset++)
    {
      mGrid.setNonInterp(iy, ix);

      if (_params.output_levels != Params::NATIVE_VERTICAL_LEVELS)
      {
	// load up interpolated vertical pressure array for this point
	vector<double> presVert;
	if (inData.dataDimension() > 2)
	{
	  inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			       "press", inData.getPres(),
			       mGrid.wtSW, mGrid.wtNW,
			       mGrid.wtNE, mGrid.wtSE,
			       presVert);

	  // load up the vertical interpolation array

	  _presInterp.prepareInterp(presVert);
	}
      }
      
      // interp the fields for this point
      
      for (int ifield = 0; ifield < _params.output_fields_n; ifield++)
      {
	switch (_params._output_fields[ifield].name)
	{
          case Params::U_EDGE_FIELD:
            _interp3dField(inData, "U_EDGE", inData.getUuC(), mdvx,
                           planeOffset, nPointsPlane,
                           fhdr->missing_data_value, mGrid);
            break;
	    
	default:
	  break;

	} // switch
      } // ifield
    } // ix
  } // iy

}

void Wrf2Mdv::_loadVEdgeOutputFields(WRFData &inData,
				     DsMdvx &mdvx)
{
  // Only applies to native projection

  if (_params.output_projection != Params::OUTPUT_PROJ_NATIVE)
    return;
  
  // set up grid remapping object

  WRFGrid mGrid(_progName, _params.debug >= Params::DEBUG_VERBOSE,inData);
  
  // get representative field header
  
  const Mdvx::field_header_t *fhdr = NULL;
  for (size_t ifield = 0; ifield < mdvx.getNFields(); ifield++)
  {
    MdvxField *fld = mdvx.getField(ifield);

    // make sure we have a V_EDGE field

    if (strstr(fld->getFieldName(), "V_EDGE"))
    {
      fhdr = &fld->getFieldHeader();
      break;
    }
  } // ifields

  // If there are no V-edge fields, then we don't have anything to do

  if (fhdr == NULL)
    return;
  
  // loop through the grid
  
  int nPointsPlane = fhdr->ny * fhdr->nx;
  if (_params.debug == Params::DEBUG_VERBOSE)
  {
    cerr << "In Wrf2Mdv::_loadVEdgeOutputFields() "
	 << "nPointsPlane = " << nPointsPlane
	 << " ny = " << fhdr->ny << " nx = " << fhdr->nx << endl;
  }
 
  for (int iy = 0; iy < fhdr->ny; iy++)
  {
    PMU_auto_register("In Wrf2Mdv::_loadVEdgeOutputFields loop");
    int planeOffset = iy * fhdr->nx;
    
    for (int ix = 0; ix < fhdr->nx; ix++, planeOffset++)
    {
      mGrid.setNonInterp(iy, ix);

      if (_params.output_levels != Params::NATIVE_VERTICAL_LEVELS)
      {
	// load up interpolated vertical pressure array for this point
	vector<double> presVert;
	if (inData.dataDimension() > 2)
	{
	  inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			       "press", inData.getPres(),
			       mGrid.wtSW, mGrid.wtNW,
			       mGrid.wtNE, mGrid.wtSE,
			       presVert);

	  // load up the vertical interpolation array

	  _presInterp.prepareInterp(presVert);
	}
      }
      
      // interp the fields for this point
      
      for (int ifield = 0; ifield < _params.output_fields_n; ifield++)
      {
	switch (_params._output_fields[ifield].name)
	{
	case Params::V_EDGE_FIELD:
	  _interp3dField(inData, "V_EDGE", inData.getVvC(), mdvx,
			 planeOffset, nPointsPlane,
			 fhdr->missing_data_value, mGrid);
	  break;
            
	default:
	  break;

	} // switch
      } // ifield
    } // ix
  } // iy

}

//////////////////
// _interp3dField()
//
// Interpolate the 3d field data onto the output grid point
//

void Wrf2Mdv::_interp3dField(WRFData &inData,
			       const  Params::output_field_name_t &field_name_enum,
			       fl32 ***field_data,
			       DsMdvx &mdvx,
			       int planeOffset,
			       int nPointsPlane,
			       fl32 missingDataVal,
			       const WRFGrid &mGrid,
			       double factor /* = 1.0*/ )
{
 int i = static_cast<int>(field_name_enum);

 if (_field_name_map[i].name == NULL || _field_name_map[i].long_name == NULL)
   cerr << "ERROR - fieldnames for field: " << i << " not defined!" << endl;

 _interp3dField(inData,_field_name_map[i].name,field_data,
		mdvx, planeOffset, nPointsPlane, missingDataVal, mGrid, factor);
}  


//////////////////
// _interp3dField()
//
// Interpolate the 3d field data onto the output grid point
//

void Wrf2Mdv::_interp3dField(WRFData &inData,
			     const char *field_name,
			     fl32 ***field_data,
			     DsMdvx &mdvx,
			     const int planeOffset,
			     const int nPointsPlane,
			     const fl32 missingDataVal,
			     const WRFGrid &mGrid,
			     const double factor)
{
  // Get a pointer to the output field.  If we can't find this field, then
  // we don't need to do anything.

  MdvxField *field = mdvx.getFieldByName(field_name);
  if (field == 0)
  {
    cerr << "ERROR - Wrf2Mdv::_interp3dField" << endl;
    cerr << "  Cannot find field name: " << field_name << endl;
    return;
  }

  // load up vertical array for this point

  vector<double> vlevelData;
  
  if (_params.output_levels == Params::NATIVE_VERTICAL_LEVELS)
  {
    inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			 field_name, field_data,
			 mGrid.wtSW, mGrid.wtNW,
			 mGrid.wtNE, mGrid.wtSE,
			 vlevelData);
  }
  else
  {
    vector<double> vertData;
    inData.interp3dField(mGrid.latIndex, mGrid.lonIndex,
			 field_name, field_data,
			 mGrid.wtSW, mGrid.wtNW,
			 mGrid.wtNE, mGrid.wtSE,
			 vertData,
			 &_presInterp.getVertNeeded());
  
    // interpolate field onto flight levels
    
    _presInterp.doInterp(vertData, vlevelData,
			 _params.copy_lowest_downwards);
   
  }
  
  // put into targetVol
  
  //Mdvx::field_header_t fhdr = field->getFieldHeader();
  fl32 *targetVol = (fl32 *) field->getVol();
  fl32 *ffp = targetVol + planeOffset;

  for (size_t i = 0; i < vlevelData.size(); i++, ffp += nPointsPlane)
  {
    if (vlevelData[i] == WRFData::MISSING_DOUBLE)
      *ffp = missingDataVal;
    else
      *ffp = vlevelData[i] * factor;
  }
  
}


/////////////////////////
// _interp2dField()
//uses the field_name_enum to get the field_name and then calls
//the other version of _interp2dField().

void Wrf2Mdv::_interp2dField(WRFData &inData,
			       const  Params::output_field_name_t &field_name_enum,
			       fl32 **field_data,
			       DsMdvx &mdvx,
			       int planeOffset,
			       fl32 missingDataVal,
			       const WRFGrid &mGrid,
			       double factor /* = 1.0*/ )
{
 int i = static_cast<int>(field_name_enum);

 if (_field_name_map[i].name == NULL || _field_name_map[i].long_name == NULL)
   cerr << "ERROR - fieldnames for field: " << i << " not defined!" << endl;

 _interp2dField(inData,_field_name_map[i].name,field_data,
		mdvx, planeOffset, missingDataVal, mGrid, factor);
}


//////////////////
// _interp2dField()
//
// Interpolate the 2d field data onto the output grid point
//

void Wrf2Mdv::_interp2dField(WRFData &inData,
			       const char *field_name,
			       fl32 **field_data,
			       DsMdvx &mdvx,
			       int planeOffset,
			       fl32 missingDataVal,
			       const WRFGrid &mGrid,
			       double factor)
{
  //  if (_params.debug == Params::DEBUG_VERBOSE)
  //    {
  //      cerr << "Entered Wrf2Mdv::_interp2dField for field: " << field_name
  //	   << "\n\tplaneOffset = " << planeOffset << endl;
  //    }

  // get interp value for point

  double interpVal =
    inData.interp2dField(mGrid.latIndex, mGrid.lonIndex,
			 field_name, field_data,
			 mGrid.wtSW, mGrid.wtNW,
			 mGrid.wtNE, mGrid.wtSE);
  
  // put into targetVol
  
  MdvxField *field = mdvx.getFieldByName(field_name);
  if (field == NULL)
  {
    cerr << "ERROR - Wrf2Mdv::_interp2dField" << endl;
    cerr << "  Cannot find field name: " << field_name << endl;
    return;
  }

  //Mdvx::field_header_t fhdr = field->getFieldHeader();
  fl32 *targetVol = (fl32 *) field->getVol();
  fl32 *ffp = targetVol + planeOffset;

  if (interpVal == WRFData::MISSING_DOUBLE)
    *ffp = missingDataVal;
  else
    *ffp = interpVal * factor;
}

////////////////////////////////////////////////////
// _initFieldNameMap()
//
// Sets up the field name map from the config file
//
// When this is done, _field_name_map[U_FIELD] will hold the name info for U_FIELD.
// This function allocates space for _field_name_map

void Wrf2Mdv::_initFieldNameMap()
{
  _field_name_map = new Params::afield_name_map_t[Params::TOTAL_FIELD_COUNT];

  for (int j = 0; j< Params::TOTAL_FIELD_COUNT; j++)
  {
    _field_name_map[j].name = NULL;
    _field_name_map[j].long_name = NULL;
  }
  
  for (int i = 0; i< _params.field_name_map_n; i++)
  {
    int j = _params._field_name_map[i].original_name;
    _field_name_map[j].name =
      new char[strlen(_params._field_name_map[i].name)+1];
    strcpy(_field_name_map[j].name,_params._field_name_map[i].name);
    _field_name_map[j].long_name =
      new char[strlen(_params._field_name_map[i].long_name)+1];
    strcpy(_field_name_map[j].long_name,_params._field_name_map[i].long_name);
  }

}
