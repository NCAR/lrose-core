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
#include <dsdata/DsIntervalTrigger.hh>

#include "SurfInterp.hh"

#include "BarnesInterpolater.hh"
#include "NearestInterpolater.hh"

#include "UwindInterpField.hh"
#include "VwindInterpField.hh"
#include "TempInterpField.hh"
#include "DewptInterpField.hh"
#include "LiftedIndexInterpField.hh"
#include "RelHumInterpField.hh"
#include "WindGustInterpField.hh"
#include "PressInterpField.hh"
#include "PotTempInterpField.hh"
#include "LiqAccumInterpField.hh"
#include "PrecipRateInterpField.hh"
#include "VisInterpField.hh"
#include "RunwayVisRangeInterpField.hh"
#include "SealevelRelCeilingInterpField.hh"
#include "AltInterpField.hh"
#include "DewptDeprInterpField.hh"

#include "ConvDerivedField.hh"
#include "TerrainDerivedField.hh"
#include "TerrainRelCeilDerivedField.hh"
#include "FltCatDerivedField.hh"

//
// static definitions
//

SurfInterp *SurfInterp::_instance = 0;

const float   SurfInterp::BARNES_GAMMA          = 1.0;
const float   SurfInterp::BARNES_ARC_MAX        = 0.0;
const float   SurfInterp::BARNES_RCLOSE         = -1.0;
const float   SurfInterp::BARNES_RMAX           = -1.0;


/*********************************************************************
 * Constructors
 */

SurfInterp::SurfInterp(int argc, char **argv) :
  _trigger(NULL),
  _dataMgr(NULL),
  _terrain(NULL),
  _stationGridExpandKm(0)
{
  static const string method_name = "SurfInterp::SurfInterp()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == 0);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = progname_parts.base;
  
  // Display ucopyright message.

  ucopyright(_progName.c_str());

  // Get the command line arguments.

  _args.parse(argc, argv, _progName);
  
  // Get TDRP parameters.

  char *params_path = new char [strlen("unknown") + 1];
  strcpy(params_path, "unknown");
  
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

SurfInterp::~SurfInterp()
{
  // Memory cleanup

  map< int, StnInterpField* >:: iterator stn_field_iter;

  for( stn_field_iter = _stnInterpFields.begin();
       stn_field_iter != _stnInterpFields.end(); ++stn_field_iter) 
    delete stn_field_iter->second;
  
  vector< GenPtInterpField* >::iterator genpt_field_iter;
  
  for (genpt_field_iter = _genptInterpFields.begin();
       genpt_field_iter != _genptInterpFields.end(); ++genpt_field_iter)
    delete *genpt_field_iter;
  
  map< int, DerivedField* >:: iterator der_field_iter;

  for (der_field_iter = _derivedFields.begin();
       der_field_iter != _derivedFields.end(); ++der_field_iter) 
    delete der_field_iter->second;
  
  // Delete contained objects

  if (_terrain) {
    delete _terrain;
  }
  if (_dataMgr) {
    delete _dataMgr;
  }
  if (_trigger) {
    delete _trigger;
  }
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

SurfInterp *SurfInterp::Inst(int argc, char **argv)
{
  if (_instance == 0)
    new SurfInterp(argc, argv);
  
  return(_instance);
}

SurfInterp *SurfInterp::Inst()
{
  assert(_instance != 0);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool SurfInterp::init()
{
  static const string method_name = "SurfInterp::init()";
  
  // If we are using station outside of the grid, set the distance of that
  // expansion to the maximum interpolation distance.

  if (_params.UseOutsideRegion)
    _stationGridExpandKm = _params.MaxInterpDist;
  
  // Initialize the data manager which handles sounding and surface data

  _dataMgr = new DataMgr(_progName, _params);
  _dataMgr->setDebug(_params.debug);
  _dataMgr->setProgName(_progName);
  
  for (int i = 0; i < _params.surface_data_urls_n; ++i)
    _dataMgr->addStationUrl(_params._surface_data_urls[i]);
  
  for (int i = 0; i < _params.sounding_urls_n; ++i)
    _dataMgr->addSoundingUrl(_params._sounding_urls[i]);
  
  for (int i = 0; i < _params.genpt_data_urls_n; ++i)
    _dataMgr->addGenptUrl(_params._genpt_data_urls[i]);
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the output projection

  if (!_initOutputProj())
    return false;

  _dataMgr->setProjection(_outputProj);
  
  // Initialize the terrain object

  if (_params.use_terrain_data) {
    _terrain = new Terrain(_params.terrain_file, _params.terrain_field_name,
                           _outputProj);
  }
  
  // Initialize the area of influence template

  double axis_km = _params.MaxInterpDist * 2.0;
  
  _influenceTemplate.setEllipse(0.0,
				_outputProj.km2xGrid(axis_km) + 1,
				_outputProj.km2yGrid(axis_km) + 1);

  // Initialize interpolated data fields   This must be done after initializing
  // the _terrain object since some of the fields depend on this object.


  if (!_initFields())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void SurfInterp::run()
{
  static const string method_name = "SurfInterp::run()";
  
  DateTime trigger_time;
  
  while (!_trigger->endOfData())
  {
    if (_trigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_trigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _encodingToMdv() - Convert the encoding type specified in the parameter
 *                    file to the equivalent MDV encoding type.
 *
 * Returns the MDV encoding type.
 */

Mdvx::encoding_type_t SurfInterp::_encodingToMdv(const Params::encoding_type_t encoding_type) const
{
  switch (encoding_type)
  {
  case Params::ENCODING_INT8 :
    return Mdvx::ENCODING_INT8;
    
  case Params::ENCODING_INT16 :
    return Mdvx::ENCODING_INT16;
    
  case Params::ENCODING_FLOAT32 :
    return Mdvx::ENCODING_FLOAT32;
  }
  return Mdvx::ENCODING_INT16;
}


/*********************************************************************
 * _scalingToMdv() - Convert the scaling type specified in the parameter
 *                   file to the equivalent MDV scaling type.
 *
 * Returns the MDV scaling type.
 */

Mdvx::scaling_type_t SurfInterp::_scalingToMdv(const Params::scaling_type_t scaling_type) const
{
  switch (scaling_type)
  {
  case Params::SCALING_NONE :
    return Mdvx::SCALING_NONE;
    
  case Params::SCALING_ROUNDED :
    return Mdvx::SCALING_ROUNDED;
    
  case Params::SCALING_INTEGRAL :
    return Mdvx::SCALING_INTEGRAL;
    
  case Params::SCALING_DYNAMIC :
    return Mdvx::SCALING_DYNAMIC;
    
  case Params::SCALING_SPECIFIED :
    return Mdvx::SCALING_SPECIFIED;
  }
  
  return Mdvx::SCALING_ROUNDED;
}


/*********************************************************************
 * _getData() - Get surface and sounding data, calculate grid coordinates
 *              of surface observations, check parameters for discrepancies.
 *
 * Returns true on success, false on failure.
 */

bool SurfInterp::_getData(const DateTime &data_time)
{
  PMU_auto_register("SurfInterp::init()");

  // Calculate the desired data range

  DateTime end_time = data_time;
  DateTime begin_time = end_time - _params.duration;

  // Get surface and sounding data.  These datasets are only used for the
  // station interpolated fields, so we don't need to get the data if we
  // aren't producing any of these fields.

  if (_stnInterpFields.size() > 0)
  {
    // Get the surface data

    if (!_getSurfaceData(begin_time, end_time))
      return false;

    // Get sounding data

    _dataMgr->getSoundingData(end_time - _params.sounding_look_back * 60,
			     end_time, _params.sounding_max_dist);

    if (_params.debug >= Params::DEBUG_VERBOSE)
    {
      cerr << "The following soundings will be used: " << endl;
      _dataMgr->printSoundingData();
    }
  }
  
  // Get GenPt data

  if (_genptInterpFields.size() > 0)
  {
    PMU_auto_register("getting GenPt data.");

    _numCapecinReps =
      _dataMgr->getGenptData(begin_time, end_time, _stationGridExpandKm);
  }
  
  return true;
}


/*********************************************************************
 * _initFields() - Initialize the interpolation fields.
 *
 * Returns true on success, false on failure.
 */

bool SurfInterp::_initFields()
{
  static const string method_name = "SurfInterp::_initFields()";
  
  // Check to see if output encoding has been specified for the output 
  // fields.

  bool encoding_specified = true;
  
  if (_params.outputFieldsEncoding_n != _params.outputFields_n)
  {
    encoding_specified = false;
    
    if (_params.outputFieldsEncoding_n != 0)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Wrong number of fields specified in outputFieldsEncoding "
	   << "array in parameter file." << endl;
      cerr << "outputFields array has " << _params.outputFields_n
	   << " fields specified." << endl;
      cerr << "outputFieldsEncoding array has "
	   << _params.outputFieldsEncoding_n << " fields specified." << endl;
      cerr << "These values should match." << endl;
      cerr << "Using default output encoding for all fields." << endl;
    }
  }
  
  // Initialize the station data fields

  for (int i = 0; i < _params.outputFields_n; ++i)
  {
    switch (_params._outputFields[i])
    {
    case Params::UWIND :
      if (encoding_specified)
      {
	_createUwindField(true,
			  _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			  _params._outputFieldsEncoding[i].use_scaling_info,
			  _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			  _params._outputFieldsEncoding[i].scale,
			  _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createUwindField(true);
      }
      break;
    
    case Params::VWIND :
      if (encoding_specified)
      {
	_createVwindField(true,
			  _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			  _params._outputFieldsEncoding[i].use_scaling_info,
			  _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			  _params._outputFieldsEncoding[i].scale,
			  _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createVwindField(true);
      }
      break;
    
    case Params::TEMP :
      if (encoding_specified)
      {
	_createTempField(true,
			 _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			 _params._outputFieldsEncoding[i].use_scaling_info,
			 _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			 _params._outputFieldsEncoding[i].scale,
			 _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createTempField(true);
      }
      break;
    
    case Params::DEWPT :
      if (encoding_specified)
      {
	_createDewptField(true,
			  _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			  _params._outputFieldsEncoding[i].use_scaling_info,
			  _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			  _params._outputFieldsEncoding[i].scale,
			  _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createDewptField(true);
      }
      break;
    
    case Params::LIFTED_INDEX :
      if (encoding_specified)
      {
	_createLiftedIndexField(true,
				_encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
				_params._outputFieldsEncoding[i].use_scaling_info,
				_scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
				_params._outputFieldsEncoding[i].scale,
				_params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createLiftedIndexField(true);
      }
      break;
    
    case Params::REL_HUM :
      if (encoding_specified)
      {
	_createRelHumField(true,
			   _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			   _params._outputFieldsEncoding[i].use_scaling_info,
			   _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			   _params._outputFieldsEncoding[i].scale,
			   _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createRelHumField(true);
      }
      break;
    
    case Params::WIND_GUST :
      if (encoding_specified)
      {
	_createWindGustField(true,
			     _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			     _params._outputFieldsEncoding[i].use_scaling_info,
			     _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			     _params._outputFieldsEncoding[i].scale,
			     _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createWindGustField(true);
      }
      break;
    
    case Params::PRESSURE :
      if (encoding_specified)
      {
	_createPressureField(true,
			     _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			     _params._outputFieldsEncoding[i].use_scaling_info,
			     _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			     _params._outputFieldsEncoding[i].scale,
			     _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createPressureField(true);
      }
      break;
    
    case Params::POTENTIAL_TEMP :
      if (encoding_specified)
      {
	_createPotTempField(true,
			    _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			    _params._outputFieldsEncoding[i].use_scaling_info,
			    _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			    _params._outputFieldsEncoding[i].scale,
			    _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createPotTempField(true);
      }
      break;
    
    case Params::LIQUID_ACCUM :
      if (encoding_specified)
      {
	_createLiqAccumField(true,
			     _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			     _params._outputFieldsEncoding[i].use_scaling_info,
			     _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			     _params._outputFieldsEncoding[i].scale,
			     _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createLiqAccumField(true);
      }
      break;
    
    case Params::PRECIP_RATE :
      if (encoding_specified)
      {
	_createPrecipRateField(true,
			       _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			       _params._outputFieldsEncoding[i].use_scaling_info,
			       _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			       _params._outputFieldsEncoding[i].scale,
			       _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createPrecipRateField(true);
      }
      break;
    
    case Params::VISIBILITY :
      if (encoding_specified)
      {
	_createVisField(true,
			_encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			_params._outputFieldsEncoding[i].use_scaling_info,
			_scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			_params._outputFieldsEncoding[i].scale,
			_params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createVisField(true);
      }
      break;
    
    case Params::RUNWAY_VIS_RANGE :
      if (encoding_specified)
      {
	_createRunwayVisRangeField(true,
				   _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
				   _params._outputFieldsEncoding[i].use_scaling_info,
				   _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
				   _params._outputFieldsEncoding[i].scale,
				   _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createRunwayVisRangeField(true);
      }
      break;
    
    case Params::SEALEVEL_RELATIVE_CEILING :
      if (encoding_specified)
      {
	_createSealevelRelCeilingField(true,
				       _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
				       _params._outputFieldsEncoding[i].use_scaling_info,
				       _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
				       _params._outputFieldsEncoding[i].scale,
				       _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createSealevelRelCeilingField(true);
      }
      break;
    
    case Params::ALTITUDE :
      if (encoding_specified)
      {
	_createAltField(true,
			_encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			_params._outputFieldsEncoding[i].use_scaling_info,
			_scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			_params._outputFieldsEncoding[i].scale,
			_params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createAltField(true);
      }
      break;
    
    case Params::DEWPT_DEPRESSION :
      if (encoding_specified)
      {
	_createDewptDeprField(true,
			      _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			      _params._outputFieldsEncoding[i].use_scaling_info,
			      _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			      _params._outputFieldsEncoding[i].scale,
			      _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createDewptDeprField(true);
      }
      break;
    
    case Params::TERRAIN :
      if (encoding_specified)
      {
	_createTerrainField(true,
			    _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			    _params._outputFieldsEncoding[i].use_scaling_info,
			    _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			    _params._outputFieldsEncoding[i].scale,
			    _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createTerrainField(true);
      }
      break;
    
    case Params::CONVERGENCE :
      if (encoding_specified)
      {
	_createConvField(true,
			 _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			 _params._outputFieldsEncoding[i].use_scaling_info,
			 _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			 _params._outputFieldsEncoding[i].scale,
			 _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createConvField(true);
      }
      break;
    
    case Params::TERRAIN_RELATIVE_CEILING :
      if (encoding_specified)
      {
	_createTerrainRelCeilField(true,
				   _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
				   _params._outputFieldsEncoding[i].use_scaling_info,
				   _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
				   _params._outputFieldsEncoding[i].scale,
				   _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createTerrainRelCeilField(true);
      }
      break;
    
    case Params::FLIGHT_CAT :
      if (encoding_specified)
      {
	_createFltCatField(true,
			   _encodingToMdv(_params._outputFieldsEncoding[i].encoding_type),
			   _params._outputFieldsEncoding[i].use_scaling_info,
			   _scalingToMdv(_params._outputFieldsEncoding[i].scaling_type),
			   _params._outputFieldsEncoding[i].scale,
			   _params._outputFieldsEncoding[i].bias);
      }
      else
      {
	_createFltCatField(true);
      }
      break;
    
    } /* endswitch - _params._outputFields[i] */
    
  } /* endfor - i */
  
  // Initialize the GenPt fields

  for (int i = 0; i < _params.genptFields_n; ++i)
  {
    Interpolater *interpolater = _createInterpolater();
      
    GenPtInterpField *field;
    
    if (_params._genptFields[i].check_missing)
      field = new GenPtInterpField(_params._genptFields[i].genpt_field_name,
				   _params._genptFields[i].missing_value,
				   _params._genptFields[i].output_field_name,
				   _params._genptFields[i].output_field_units,
				   interpolater,
				   true,
				   _params.debug >= Params::DEBUG_NORM);
    else
      field = new GenPtInterpField(_params._genptFields[i].genpt_field_name,
				   _params._genptFields[i].output_field_name,
				   _params._genptFields[i].output_field_units,
				   interpolater,
				   true,
				   _params.debug >= Params::DEBUG_NORM);

    field->setEncodingType(_encodingToMdv(_params._genptFields[i].encoding_type));
    if (_params._genptFields[i].use_scaling_info)
      field->setScaling(_scalingToMdv(_params._genptFields[i].scaling_type),
			_params._genptFields[i].scale,
			_params._genptFields[i].bias);
    
    _genptInterpFields.push_back(field);
  }
  
  return true;
}


/*********************************************************************
 * _initOutputProj() - Initialize the output projection
 *
 * Returns true on success, false on failure.
 */

bool SurfInterp::_initOutputProj()
{
  switch (_params.OutputProj.proj_type)
  {
  case Params::PROJ_LATLON :
    _outputProj.initLatlon(_params.OutputProj.nx, _params.OutputProj.ny, 1,
			   _params.OutputProj.dx, _params.OutputProj.dy, 1.0,
			   _params.OutputProj.minx, _params.OutputProj.miny,
			   0.0);
    break;
    
  case Params::PROJ_FLAT :
    _outputProj.initFlat(_params.OutputProj.origin_lat,
			 _params.OutputProj.origin_lon, 0.0,
			 _params.OutputProj.nx, _params.OutputProj.ny, 1,
			 _params.OutputProj.dx, _params.OutputProj.dy, 1.0,
			 _params.OutputProj.minx, _params.OutputProj.miny,
			 0.0);
    break;
    
  case Params::PROJ_LC :
    _outputProj.initLc2(_params.OutputProj.origin_lat,
			_params.OutputProj.origin_lon,
			_params.OutputProj.lat1, _params.OutputProj.lat2,
			_params.OutputProj.nx, _params.OutputProj.ny, 1,
			_params.OutputProj.dx, _params.OutputProj.dy, 1.0,
			_params.OutputProj.minx, _params.OutputProj.miny, 0.0);
    break;
    
  } /* endswitch - _params.OutputProj.proj_type */
  
  _gridSize = _outputProj.getNx() * _outputProj.getNy();

  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool SurfInterp::_initTrigger()
{
  static const string method_name = "SurfInterp::_initTrigger()";
  
  // Check for consistency of parameters and arguments

  if (!_args.TR_spec && _params.mode == Params::ARCHIVE) {
    fprintf(stderr,"No time range specified in ARCHIVE mode.\n");

    return false;
  }

  // create the trigger
  
  switch (_params.mode)
  {
  case Params::REALTIME :
  {
    if (_params.debug >= Params::DEBUG_NORM)
    {
      cerr << "Initializing REALTIME trigger: " << endl;
      cerr << "   trigger interval: " << _params.time_trigger_interval
	   << " seconds" << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params.time_trigger_interval, 0, 1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing REALTIME trigger" << endl;
      return false;
    }
    
    _trigger = trigger;

    break;
  }
  
  case Params::ARCHIVE :
  {
    if (_params.debug >= Params::DEBUG_NORM)
    {
      cerr << "Initializing ARCHIVE trigger:" << endl;
      cerr << "   trigger interval: " << _params.time_trigger_interval
	   << " seconds" << endl;
      cerr << "   start time: " << DateTime::str(_args.startTime) << endl;
      cerr << "   end time: " << DateTime::str(_args.endTime) << endl;
    }
    
    DsIntervalTrigger *trigger = new DsIntervalTrigger();
    if (trigger->init(_params.time_trigger_interval,
		      _args.startTime, _args.endTime) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing ARCHIVE trigger" << endl;
      return false;
    }
    
    _trigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params.mode */

  return true;
}


/*********************************************************************
 * _getSurfaceData() - get surface observations, check to see that there
 *                     are enough to interpolate, do some quality control
 *                     on the ceiling field if required.
 */

bool SurfInterp::_getSurfaceData(const DateTime &begin_time,
				  const DateTime &end_time)
{
  static const string method_name = "SurfInterp::_getSurfaceData()";
  
  // Get surface data that fall between the begin and end times

  _numSurfaceReps = _dataMgr->getSurfaceData(begin_time, end_time,
                                             _stationGridExpandKm,
                                             _params.MaxVis,
                                             _params.MaxCeiling);
    
  if (_params.debug >= Params::DEBUG_NORM)
    cerr << _progName << ": " << method_name << ": "
	 << _numSurfaceReps << " stations to be processed."  << endl;
  
  // Exit if we don't have enough data
  
  if (_numSurfaceReps < _params.MinStations)
  {
    cerr << _progName << ": " << method_name
	 << ": Not enough surface data." << endl;
    
    return false;
  } 
  
  // Replace values of ceiling that indicate clear sky if required
  
  if (_params.ReplaceCeiling)
    _dataMgr->replaceSurfaceDataCeiling(_params.ReplaceCeilingThreshold,
                                        _params.ReplaceCeilingValue);
  
  return true;
}


/*********************************************************************
 * _interpolate() - Loop through the InterpFieldOlds and interpolate
 *                  if necessary and calculate derived fields.
 */

bool SurfInterp::_interpolate()
{
  static const string method_name = "SurfInterp::interpolate()";
  
  // Allocate space for the grid containing the interpolation distances

  int grid_size =
    _outputProj.getNx() * _outputProj.getNy();
	    
  float *interp_dist = new float[grid_size];
  
  // Interpolate the station data

  map< int, StnInterpField* >::iterator stn_field_iter;
    
  for (stn_field_iter = _stnInterpFields.begin();
       stn_field_iter != _stnInterpFields.end(); ++stn_field_iter)
    stn_field_iter->second->init();
  
  for (int i = 0; i < _numSurfaceReps; ++i)
  {
    station_report_t report = *_dataMgr->getSurfaceRep(i);
    
    _calcInterpDistances(report.lat, report.lon, interp_dist);
    
    for (stn_field_iter = _stnInterpFields.begin();
	 stn_field_iter != _stnInterpFields.end(); ++stn_field_iter)
    {
      StnInterpField *stn_field = stn_field_iter->second;
      
      stn_field->addObs(report, interp_dist);
    } /* endfor - stn_field */
    
  } /* endfor - i */

  for (stn_field_iter = _stnInterpFields.begin();
       stn_field_iter != _stnInterpFields.end(); ++stn_field_iter)
  {
    StnInterpField *stn_field = stn_field_iter->second;
      
    stn_field->interpolate();
  } /* endfor - stn_field */

  // Interpolate the GenPt data

  vector< GenPtInterpField* >::iterator genpt_field_iter;
    
  for (genpt_field_iter = _genptInterpFields.begin();
       genpt_field_iter != _genptInterpFields.end(); ++genpt_field_iter)
    (*genpt_field_iter)->init();
  
  for (int i = 0; i < _numCapecinReps; ++i)
  {
    GenPt obs = *_dataMgr->getGenptRep(i);
    
    _calcInterpDistances(obs.getLat(), obs.getLon(), interp_dist);
    
    for (genpt_field_iter = _genptInterpFields.begin();
	 genpt_field_iter != _genptInterpFields.end(); ++genpt_field_iter)
    {
      GenPtInterpField *genpt_field = *genpt_field_iter;
      
      genpt_field->addObs(obs, interp_dist);
    } /* endfor - genpt_field */
    
  } /* endfor - i */

  for (genpt_field_iter = _genptInterpFields.begin();
       genpt_field_iter != _genptInterpFields.end(); ++genpt_field_iter)
  {
    GenPtInterpField *genpt_field = *genpt_field_iter;
      
    genpt_field->interpolate();
  } /* endfor - genpt_field */
    
  delete [] interp_dist;
  
  // Calculate the derived fields

  map< int, DerivedField* >::iterator der_field_iter;
  
  for (der_field_iter = _derivedFields.begin();
       der_field_iter != _derivedFields.end(); ++der_field_iter)
  {
    DerivedField *der_field = der_field_iter->second;
    
    der_field->derive(_stnInterpFields, _derivedFields);
  } /* endfor - der_field_iter */
  
  return true;
}


/*********************************************************************
 * _outputData()
 */

bool SurfInterp::_outputData(const DateTime &data_time)
{
  static const string method_name = "SurfInterp::_outputData()";
  
  // Initialize Output object with :

  Output output(data_time, _params.duration,
		_outputProj, _params.altitude, _params.dataInfo ,
		_params.datasetName, _params.dataSource, _progName);
  
  // Add the station data fields to the output

  map< int, StnInterpField* >::iterator stn_field_iter;
  
  for (stn_field_iter = _stnInterpFields.begin();
       stn_field_iter != _stnInterpFields.end(); ++stn_field_iter)
  {
    StnInterpField *stn_field = stn_field_iter->second;
    
    if (!stn_field->isOutput())
      continue;
    
    string field_name = stn_field->getFieldName();
    string field_units = stn_field->getFieldUnits();
  
    output.addField(field_name.c_str(), field_name.c_str(),
		    field_units.c_str(), stn_field->getInterpolation(),
		    stn_field->getMissingDataValue(),
		    stn_field->getMissingDataValue(),
		    stn_field->getEncodingType(),
		    stn_field->getScalingType(),
		    stn_field->getScale(),
		    stn_field->getBias());

  } /* endfor - stn_field_iter */
  
  // Add the GenPt data fields to the output

  vector< GenPtInterpField* >::iterator genpt_field_iter;
  
  for (genpt_field_iter = _genptInterpFields.begin();
       genpt_field_iter != _genptInterpFields.end(); ++genpt_field_iter)
  {
    GenPtInterpField *genpt_field = *genpt_field_iter;
    
    if (!genpt_field->isOutput())
      continue;
    
    string field_name = genpt_field->getFieldName();
    string field_units = genpt_field->getFieldUnits();
  
    output.addField(field_name.c_str(), field_name.c_str(),
		    field_units.c_str(), genpt_field->getInterpolation(),
		    genpt_field->getMissingDataValue(),
		    genpt_field->getMissingDataValue(),
		    genpt_field->getEncodingType(),
		    genpt_field->getScalingType(),
		    genpt_field->getScale(),
		    genpt_field->getBias());

  } /* endfor - genpt_field_iter */
  
  // Add the derived fields to the output

  map< int, DerivedField* >::iterator der_field_iter;
  
  for (der_field_iter = _derivedFields.begin();
       der_field_iter != _derivedFields.end(); ++der_field_iter)
  {
    DerivedField *der_field = der_field_iter->second;
    
    string field_name = der_field->getFieldName();
    string field_units = der_field->getFieldUnits();
    
    output.addField(field_name.c_str(), field_name.c_str(),
		    field_units.c_str(), der_field->getDerivation(),
		    der_field->getMissingDataValue(),
		    der_field->getMissingDataValue(),
		    der_field->getEncodingType(),
		    der_field->getScalingType(),
		    der_field->getScale(),
		    der_field->getBias());
    
  } /* endfor - der_field_iter */
  
  if (_params.debug >= Params::DEBUG_NORM)
    cerr << _progName << ": " << method_name
	 << ": Writing data to " << _params.output_url << "\n\n" << endl;
  
  if (!output.write(_params.output_url))
    return false;
  
  return true;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool SurfInterp::_processData(const DateTime &trigger_time)
{
  static const string method_name = "SurfInterp::_processData()";
  
  if (_params.debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the appropiate datasets

  if (!_getData(trigger_time))
    return false;
  
  // Do the interpolations

  if (!_interpolate())
    return false;
  
  // Write the output file

  if (!_outputData(trigger_time))
    return false;
  
  return true;
}


/*********************************************************************
 * _calcInterpDistances() - Calculate the distance from this observation
 *                          to every grid point.
 */

void SurfInterp::_calcInterpDistances(const double obs_lat,
				      const double obs_lon,
				      float *interp_distances) const
{
  // Initialize the grid with missing values

  int grid_size = _outputProj.getNx() * _outputProj.getNy();
  
  for (int i = 0; i < grid_size; ++i)
    interp_distances[i] = -1.0;
  
  // Calculate the distances just within the area of influence around the
  // observation.  If the observation is outside of the grid, don't use it.

  int obs_x_index, obs_y_index;
  
  _outputProj.latlon2xyIndex(obs_lat, obs_lon,
			     obs_x_index, obs_y_index);
  
  GridPoint *curr_pt;
  
  for (curr_pt = _influenceTemplate.getFirstInGrid(obs_x_index, obs_y_index,
						   _outputProj.getNx(),
						   _outputProj.getNy());
       curr_pt != 0;
       curr_pt = _influenceTemplate.getNextInGrid())
  {
    double grid_lat, grid_lon;
      
    _outputProj.xyIndex2latlon(curr_pt->x, curr_pt->y, grid_lat, grid_lon);
      
    double r, theta;
	
    _outputProj.latlon2RTheta(grid_lat, grid_lon, obs_lat, obs_lon,
			      r, theta);
	
    int grid_index = curr_pt->getIndex(_outputProj.getNx(),
				       _outputProj.getNy());
    
    interp_distances[grid_index]  = r;
    
  } /* endfor - curr_pt */
  
}


/*********************************************************************
 * _createAltField() - Create the altitude field.
 */

void SurfInterp::_createAltField(const bool output_flag,
				 const Mdvx::encoding_type_t encoding_type,
				 const bool use_scaling_info,
				 const Mdvx::scaling_type_t scaling_type,
				 const double scale,
				 const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::ALTITUDE))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new AltInterpField(interpolater,
			 output_flag,
			 _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);
    
    _stnInterpFields[Params::ALTITUDE] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createConvField() - Create the convergence field.
 */

void SurfInterp::_createConvField(const bool output_flag,
				  const Mdvx::encoding_type_t encoding_type,
				  const bool use_scaling_info,
				  const Mdvx::scaling_type_t scaling_type,
				  const double scale,
				  const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, DerivedField* >::iterator field_iter;
  
  if ((field_iter = _derivedFields.find(Params::CONVERGENCE))
       == _derivedFields.end())
  {
    // Make sure the dependent fields are interpolated.  Convergence depends
    // on the U and V winds fields

    _createUwindField(false);
    _createVwindField(false);

    // Create the convergence field object

    DerivedField *field =
      new ConvDerivedField(_params.convergenceDxDy,
			   _outputProj,
			   output_flag,
			   _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _derivedFields[Params::CONVERGENCE] = field;
      
  }
  else
  {
    if (output_flag)
    {
      DerivedField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createDewptField() - Create the dewpoint field.
 */

void SurfInterp::_createDewptField(const bool output_flag,
				   const Mdvx::encoding_type_t encoding_type,
				   const bool use_scaling_info,
				   const Mdvx::scaling_type_t scaling_type,
				   const double scale,
				   const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::DEWPT))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new DewptInterpField(interpolater,
			   output_flag,
			   _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::DEWPT] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createDewptDeprField() - Create the dewpoint depression field.
 */

void SurfInterp::_createDewptDeprField(const bool output_flag,
				       const Mdvx::encoding_type_t encoding_type,
				       const bool use_scaling_info,
				       const Mdvx::scaling_type_t scaling_type,
				       const double scale,
				       const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::DEWPT_DEPRESSION))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new DewptDeprInterpField(interpolater,
			       output_flag,
			       _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::DEWPT_DEPRESSION] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createFltCatField() - Create the flight category field.
 */

void SurfInterp::_createFltCatField(const bool output_flag,
				    const Mdvx::encoding_type_t encoding_type,
				    const bool use_scaling_info,
				    const Mdvx::scaling_type_t scaling_type,
				    const double scale,
				    const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, DerivedField* >::iterator field_iter;
  
  if ((field_iter = _derivedFields.find(Params::FLIGHT_CAT))
       == _derivedFields.end())
  {
    // Make sure the dependent fields are interpolated.  Flight cat depends
    // on terrain relative ceiling, visibility, altitude and terrain.

    _createTerrainRelCeilField(false);
    _createVisField(false);
    _createAltField(false);
    _createTerrainField(false);

    // Create the terrain relative ceiling object

    vector< FltCatDerivedField::flt_cat_thresh_t > thresholds;
      
    for (int j = 0; j < _params.FltCatThresh_n; ++j)
    {
      FltCatDerivedField::flt_cat_thresh_t new_thresh;
	
      new_thresh.ceil_thresh = _params._FltCatThresh[j].ceil_thresh;
      new_thresh.vis_thresh = _params._FltCatThresh[j].vis_thresh;
	
      thresholds.push_back(new_thresh);
    } /* endfor - j */
      
    DerivedField *field =
      new FltCatDerivedField(_params.BadCeilingValue,
			     _params.MaxAltError,
			     thresholds,
			     _outputProj,
			     output_flag,
			     _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _derivedFields[Params::FLIGHT_CAT] = field;
  }
  else
  {
    if (output_flag)
    {
      DerivedField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createInterpolater() - Create an interpolater object.
 */

Interpolater *SurfInterp::_createInterpolater() const
{
  Interpolater *interpolater=NULL;
    
  switch (_params.InterpMethod)
  {
  case Params::INTERP_BARNES :
    interpolater = new BarnesInterpolater(_outputProj,
					  _params.Rscale,
					  BARNES_RMAX,
					  BARNES_GAMMA,
					  BARNES_ARC_MAX,
					  BARNES_RCLOSE,
					  _params.MinWeight,
					  _params.MaxInterpDist,
					  _params.debug >= Params::DEBUG_NORM);
    break;
    
  case Params::INTERP_NEAREST :
    interpolater = new NearestInterpolater(_outputProj,
					   _params.MaxInterpDist,
					   _params.debug >= Params::DEBUG_NORM);
    break;
  }

  return interpolater;
}


/*********************************************************************
 * _createLiftedIndexField() - Create the lifted index field.
 */

void SurfInterp::_createLiftedIndexField(const bool output_flag,
					 const Mdvx::encoding_type_t encoding_type,
					 const bool use_scaling_info,
					 const Mdvx::scaling_type_t scaling_type,
					 const double scale,
					 const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::LIFTED_INDEX))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new LiftedIndexInterpField(interpolater,
				 _dataMgr,
				 _params.sounding_max_dist,
				 _params.PresLi,
				 output_flag,
				 _params.debug >= Params::DEBUG_NORM,
				 _params.tryOtherPressure,
				 _params.adjustStationPressure);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::LIFTED_INDEX] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createLiqAccumField() - Create the liquid accumulation field.
 */

void SurfInterp::_createLiqAccumField(const bool output_flag,
				      const Mdvx::encoding_type_t encoding_type,
				      const bool use_scaling_info,
				      const Mdvx::scaling_type_t scaling_type,
				      const double scale,
				      const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::LIQUID_ACCUM))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new LiqAccumInterpField(interpolater,
			      output_flag,
			      _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::LIQUID_ACCUM] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createPotTempField() - Create the potential temperature field.
 */

void SurfInterp::_createPotTempField(const bool output_flag,
				     const Mdvx::encoding_type_t encoding_type,
				     const bool use_scaling_info,
				     const Mdvx::scaling_type_t scaling_type,
				     const double scale,
				     const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::POTENTIAL_TEMP))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new PotTempInterpField(interpolater,
			     output_flag,
			     _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::POTENTIAL_TEMP] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createPrecipRateField() - Create the precipitation rate field.
 */

void SurfInterp::_createPrecipRateField(const bool output_flag,
					const Mdvx::encoding_type_t encoding_type,
					const bool use_scaling_info,
					const Mdvx::scaling_type_t scaling_type,
					const double scale,
					const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::PRECIP_RATE))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new PrecipRateInterpField(interpolater,
				output_flag,
				_params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::PRECIP_RATE] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createPressureField() - Create the pressure field.
 */

void SurfInterp::_createPressureField(const bool output_flag,
				      const Mdvx::encoding_type_t encoding_type,
				      const bool use_scaling_info,
				      const Mdvx::scaling_type_t scaling_type,
				      const double scale,
				      const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::PRESSURE))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new PressInterpField(interpolater,
			   output_flag,
			   _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::PRESSURE] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createRelHumField() - Create the relative humidity field.
 */

void SurfInterp::_createRelHumField(const bool output_flag,
				    const Mdvx::encoding_type_t encoding_type,
				    const bool use_scaling_info,
				    const Mdvx::scaling_type_t scaling_type,
				    const double scale,
				    const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::REL_HUM))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new RelHumInterpField(interpolater,
			    output_flag,
			    _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::REL_HUM] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createRunwayVisRangeField() - Create the runway visible range field.
 */

void SurfInterp::_createRunwayVisRangeField(const bool output_flag,
					    const Mdvx::encoding_type_t encoding_type,
					    const bool use_scaling_info,
					    const Mdvx::scaling_type_t scaling_type,
					    const double scale,
					    const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::RUNWAY_VIS_RANGE))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new RunwayVisRangeInterpField(interpolater,
				    output_flag,
				    _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::RUNWAY_VIS_RANGE] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createSealevelRelCeilingField() - Create the sea level relative
 *                                    ceiling field.
 */

void SurfInterp::_createSealevelRelCeilingField(const bool output_flag,
						const Mdvx::encoding_type_t encoding_type,
						const bool use_scaling_info,
						const Mdvx::scaling_type_t scaling_type,
						const double scale,
						const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::SEALEVEL_RELATIVE_CEILING))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new SealevelRelCeilingInterpField(interpolater,
					output_flag,
					_params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::SEALEVEL_RELATIVE_CEILING] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createTempField() - Create the temperature field.
 */

void SurfInterp::_createTempField(const bool output_flag,
				  const Mdvx::encoding_type_t encoding_type,
				  const bool use_scaling_info,
				  const Mdvx::scaling_type_t scaling_type,
				  const double scale,
				  const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::TEMP))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new TempInterpField(interpolater,
			  output_flag,
			  _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::TEMP] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createTerrainField() - Create the terrain field.
 */

void SurfInterp::_createTerrainField(const bool output_flag,
				     const Mdvx::encoding_type_t encoding_type,
				     const bool use_scaling_info,
				     const Mdvx::scaling_type_t scaling_type,
				     const double scale,
				     const double bias)
{

  if (_terrain == NULL) {
    return;
  }

  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, DerivedField* >::iterator field_iter;
  
  if ((field_iter = _derivedFields.find(Params::TERRAIN))
       == _derivedFields.end())
  {
    DerivedField *field =
      new TerrainDerivedField(_terrain,
			      _outputProj,
			      output_flag,
			      _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _derivedFields[Params::TERRAIN] = field;
  }
  else
  {
    if (output_flag)
    {
      DerivedField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createTerrainRelCeilField() - Create the terrain relative ceiling
 *                                field.
 */

void SurfInterp::_createTerrainRelCeilField(const bool output_flag,
					    const Mdvx::encoding_type_t encoding_type,
					    const bool use_scaling_info,
					    const Mdvx::scaling_type_t scaling_type,
					    const double scale,
					    const double bias)
{

  if (_terrain == NULL) {
    return;
  }

  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, DerivedField* >::iterator field_iter;
  
  if ((field_iter = _derivedFields.find(Params::TERRAIN_RELATIVE_CEILING))
       == _derivedFields.end())
  {
    // Make sure the dependent fields are interpolated.  Terrain relative
    // ceiling depends on the sea level relative ceiling field.

    _createSealevelRelCeilingField(false);
    
    // Create the terrain relative ceiling object

    DerivedField *field =
      new TerrainRelCeilDerivedField(_terrain,
				     _outputProj,
				     output_flag,
				     _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _derivedFields[Params::TERRAIN_RELATIVE_CEILING] = field;
      
  }
  else
  {
    if (output_flag)
    {
      DerivedField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createUwindField() - Create the U wind field.
 */

void SurfInterp::_createUwindField(const bool output_flag,
				   const Mdvx::encoding_type_t encoding_type,
				   const bool use_scaling_info,
				   const Mdvx::scaling_type_t scaling_type,
				   const double scale,
				   const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::UWIND))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new UwindInterpField(interpolater,
			   output_flag,
			   _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::UWIND] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createVisField() - Create the visibility field.
 */

void SurfInterp::_createVisField(const bool output_flag,
				 const Mdvx::encoding_type_t encoding_type,
				 const bool use_scaling_info,
				 const Mdvx::scaling_type_t scaling_type,
				 const double scale,
				 const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::VISIBILITY))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new VisInterpField(interpolater,
			 output_flag,
			 _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::VISIBILITY] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createVwindField() - Create the V wind field.
 */

void SurfInterp::_createVwindField(const bool output_flag,
				   const Mdvx::encoding_type_t encoding_type,
				   const bool use_scaling_info,
				   const Mdvx::scaling_type_t scaling_type,
				   const double scale,
				   const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::VWIND))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new VwindInterpField(interpolater,
			   output_flag,
			   _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::VWIND] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}


/*********************************************************************
 * _createWindGustField() - Create the wind gust field.
 */

void SurfInterp::_createWindGustField(const bool output_flag,
				      const Mdvx::encoding_type_t encoding_type,
				      const bool use_scaling_info,
				      const Mdvx::scaling_type_t scaling_type,
				      const double scale,
				      const double bias)
{
  // See if the field already exists.  If it does, we just want to
  // update the output flag appropriately.

  map< int, StnInterpField* >::iterator field_iter;
  
  if ((field_iter = _stnInterpFields.find(Params::WIND_GUST))
       == _stnInterpFields.end())
  {
    Interpolater *interpolater = _createInterpolater();
    
    StnInterpField *field =
      new WindGustInterpField(interpolater,
			      output_flag,
			      _params.debug >= Params::DEBUG_NORM);
    field->setEncodingType(encoding_type);
    if (use_scaling_info)
      field->setScaling(scaling_type, scale, bias);

    _stnInterpFields[Params::WIND_GUST] = field;
  }
  else
  {
    if (output_flag)
    {
      StnInterpField *field = field_iter->second;
      field->setOutputFlag(true);
      field->setEncodingType(encoding_type);
      if (use_scaling_info)
	field->setScaling(scaling_type, scale, bias);
    }
  }
  
}
