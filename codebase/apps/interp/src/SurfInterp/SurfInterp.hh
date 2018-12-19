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
#ifndef SURFINTERP_HH
#define SURFINTERP_HH

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <dsdata/DsTrigger.hh>
#include <euclid/EllipticalTemplate.hh>
#include <euclid/Pjg.hh>
#include <toolsa/pmu.h>

#include "Args.hh"
#include "Params.hh"
#include "Terrain.hh"
#include "Output.hh"
#include "DataMgr.hh"
#include "DerivedField.hh"
#include "GenPtInterpField.hh"
#include "StnInterpField.hh"

using namespace std;


class SurfInterp
{

public: 

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Destructor
   */

  ~SurfInterp();


  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static SurfInterp *Inst(int argc, char **argv);
  static SurfInterp *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const float   BARNES_GAMMA;
  static const float   BARNES_ARC_MAX;
  static const float   BARNES_RCLOSE;
  static const float   BARNES_RMAX;


  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static SurfInterp *_instance;
  
  string _progName;

  Args _args;
  Params _params;

  // Triggering object

  DsTrigger *_trigger;
  DataMgr *_dataMgr;
  
  map< int, StnInterpField* > _stnInterpFields;
  vector< GenPtInterpField* > _genptInterpFields;
  map< int, DerivedField* > _derivedFields;
  
  Terrain *_terrain;
  
  Pjg _outputProj;
  
  int _numSurfaceReps;
  int _numCapecinReps;
  int _gridSize;

  double _stationGridExpandKm;
  
  EllipticalTemplate _influenceTemplate;
  

  /////////////////////
  // Private methods //
  /////////////////////
  
  /*********************************************************************
   * Constructors -- private because this is a singleton object
   */
  
  SurfInterp(int argc, char **argv);
  

  /*********************************************************************
   * _calcInterpDistances() - Calculate the distance from this observation
   *                          to every grid point.
   */

  void _calcInterpDistances(const double obs_lat, const double obs_lon,
			    float *interp_distances) const;
  

  /*********************************************************************
   * _createAltField() - Create the altitude field.
   */

  void _createAltField(const bool output_flag,
		       const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
		       const bool use_scaling_info = false,
		       const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
		       const double scale = 1.0,
		       const double bias = 0.0);
  

  /*********************************************************************
   * _createConvField() - Create the convergence field.
   */

  void _createConvField(const bool output_flag,
			const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			const bool use_scaling_info = false,
			const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			const double scale = 1.0,
			const double bias = 0.0);
  

  /*********************************************************************
   * _createDewptField() - Create the dewpoint field.
   */
  
  void _createDewptField(const bool output_flag,
			 const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			 const bool use_scaling_info = false,
			 const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			 const double scale = 1.0,
			 const double bias = 0.0);
  

  /*********************************************************************
   * _createDewptDeprField() - Create the dewpoint depression field.
   */

  void _createDewptDeprField(const bool output_flag,
			     const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			     const bool use_scaling_info = false,
			     const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			     const double scale = 1.0,
			     const double bias = 0.0);
  

  /*********************************************************************
   * _createFltCatField() - Create the flight category field.
   */

  void _createFltCatField(const bool output_flag,
			  const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			  const bool use_scaling_info = false,
			  const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			  const double scale = 1.0,
			  const double bias = 0.0);
  

  /*********************************************************************
   * _createInterpolater() - Create an interpolater object.
   */

  Interpolater *_createInterpolater() const;
  

  /*********************************************************************
   * _createLiftedIndexField() - Create the lifted index field.
   */

  void _createLiftedIndexField(const bool output_flag,
			       const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			       const bool use_scaling_info = false,
			       const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			       const double scale = 1.0,
			       const double bias = 0.0);
  

  /*********************************************************************
   * _createLiqAccumField() - Create the liquid accumulation field.
   */

  void _createLiqAccumField(const bool output_flag,
			    const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			    const bool use_scaling_info = false,
			    const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			    const double scale = 1.0,
			    const double bias = 0.0);
  

  /*********************************************************************
   * _createPotTempField() - Create the potential temperature field.
   */

  void _createPotTempField(const bool output_flag,
			   const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			   const bool use_scaling_info = false,
			   const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			   const double scale = 1.0,
			   const double bias = 0.0);
  

  /*********************************************************************
   * _createPrecipRateField() - Create the precipitation rate field.
   */

  void _createPrecipRateField(const bool output_flag,
			      const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			      const bool use_scaling_info = false,
			      const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			      const double scale = 1.0,
			      const double bias = 0.0);
  

  /*********************************************************************
   * _createPressureField() - Create the pressure field.
   */

  void _createPressureField(const bool output_flag,
			    const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			    const bool use_scaling_info = false,
			    const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			    const double scale = 1.0,
			    const double bias = 0.0);
  

  /*********************************************************************
   * _createRelHumField() - Create the relative humidity field.
   */

  void _createRelHumField(const bool output_flag,
			  const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			  const bool use_scaling_info = false,
			  const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			  const double scale = 1.0,
			  const double bias = 0.0);
  

  /*********************************************************************
   * _createRunwayVisRangeField() - Create the runway visible range field.
   */

  void _createRunwayVisRangeField(const bool output_flag,
				  const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
				  const bool use_scaling_info = false,
				  const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
				  const double scale = 1.0,
				  const double bias = 0.0);
  

  /*********************************************************************
   * _createSealevelRelCeilingField() - Create the sea level relative
   *                                    ceiling field.
   */

  void _createSealevelRelCeilingField(const bool output_flag,
				      const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
				      const bool use_scaling_info = false,
				      const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
				      const double scale = 1.0,
				      const double bias = 0.0);
  

  /*********************************************************************
   * _createTempField() - Create the temperature field.
   */

  void _createTempField(const bool output_flag,
			const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			const bool use_scaling_info = false,
			const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			const double scale = 1.0,
			const double bias = 0.0);
  

  /*********************************************************************
   * _createTerrainField() - Create the terrain field.
   */

  void _createTerrainField(const bool output_flag,
			   const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			   const bool use_scaling_info = false,
			   const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			   const double scale = 1.0,
			   const double bias = 0.0);
  

  /*********************************************************************
   * _createTerrainRelCeilField() - Create the terrain relative ceiling
   *                                field.
   */

  void _createTerrainRelCeilField(const bool output_flag,
				  const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
				  const bool use_scaling_info = false,
				  const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
				  const double scale = 1.0,
				  const double bias = 0.0);
  

  /*********************************************************************
   * _createVisField() - Create the visibility field.
   */

  void _createVisField(const bool output_flag,
		       const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
		       const bool use_scaling_info = false,
		       const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
		       const double scale = 1.0,
		       const double bias = 0.0);
  

  /*********************************************************************
   * _createVwindField() - Create the V wind field.
   */

  void _createVwindField(const bool output_flag,
			 const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			 const bool use_scaling_info = false,
			 const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			 const double scale = 1.0,
			 const double bias = 0.0);
  

  /*********************************************************************
   * _createWindGustField() - Create the wind gust field.
   */

  void _createWindGustField(const bool output_flag,
			    const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			    const bool use_scaling_info = false,
			    const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			    const double scale = 1.0,
			    const double bias = 0.0);
  

  /*********************************************************************
   * _createUwindField() - Create the U wind field.
   */

  void _createUwindField(const bool output_flag,
			 const Mdvx::encoding_type_t encoding_type = Mdvx::ENCODING_INT8,
			 const bool use_scaling_info = false,
			 const Mdvx::scaling_type_t scaling_type = Mdvx::SCALING_ROUNDED,
			 const double scale = 1.0,
			 const double bias = 0.0);
  

  /*********************************************************************
   * _encodingToMdv() - Convert the encoding type specified in the parameter
   *                    file to the equivalent MDV encoding type.
   *
   * Returns the MDV encoding type.
   */

  Mdvx::encoding_type_t _encodingToMdv(const Params::encoding_type_t encoding_type) const;
  

  /*********************************************************************
   * _getData() - Get surface and sounding data, calculate grid coordinates
   *              of surface observations, check parameters for discrepancies.
   *
   * Returns true on success, false on failure.
   */

  bool _getData(const DateTime &data_time);
  

  /*********************************************************************
   * _getSurfaceData() - get surface observations, check to see that there
   *                     are enough to interpolate, do some quality control
   *                     on the ceiling field if required.
   */

  bool  _getSurfaceData(const DateTime &begin_time,
			const DateTime &end_time);
  

  /*********************************************************************
   * _initFields() - Initialize the interpolation fields.
   *
   * Returns true on success, false on failure.
   */

  bool _initFields();
  

  /*********************************************************************
   * _initOutputProj() - Initialize the output projection
   *
   * Returns true on success, false on failure.
   */

  bool _initOutputProj();
  

  /*********************************************************************
   * _initTrigger() - Initialize the data trigger.
   *
   * Returns true on success, false on failure.
   */

  bool _initTrigger();
  

  /*********************************************************************
   * _interpolate() - Loop through the InterpFieldOlds and interpolate
   *                  if necessary and calculate derived fields.
   */

  bool _interpolate();
  

  /*********************************************************************
   * _outputData()
   */

  bool _outputData(const DateTime &data_time);


  /*********************************************************************
   * _processData() - Process data for the given trigger time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &trigger_time);
  

  /*********************************************************************
   * _scalingToMdv() - Convert the scaling type specified in the parameter
   *                   file to the equivalent MDV scaling type.
   *
   * Returns the MDV scaling type.
   */

  Mdvx::scaling_type_t _scalingToMdv(const Params::scaling_type_t scaling_type) const;
  

};

#endif
