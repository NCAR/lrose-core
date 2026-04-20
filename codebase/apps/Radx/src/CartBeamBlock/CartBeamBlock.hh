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
//
// CartBeamBlock.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2026
//
///////////////////////////////////////////////////////////////
//
// CartBeamBlock computes beam blockage for a specified radar
// type and location, and for a specified 3D Cartesian grid.
//
///////////////////////////////////////////////////////////////

#ifndef CartBeamBlock_HH
#define CartBeamBlock_HH

#include "Args.hh"
#include "Params.hh"
#include "BeamPowerPattern.hh"
// #include "RainFields.hh"
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <rapformats/DsRadarParams.hh>
#include <radar/BeamHeight.hh>
#include <string>
#include <set>
using namespace std;

class DemProvider;
class BlockageCalc;

class CartBeamBlock {
  
public:

  // constructor
  
  CartBeamBlock (int argc, char **argv);

  // destructor
  
  ~CartBeamBlock();

  // run 

  int Run();

  // data members

  int OK;

  static const fl32 missingFl32;
  
protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // template file
  // provides headers, projection, radar location

  Mdvx _templateMdvx;
  MdvxField *_templateField;
  Mdvx::master_header_t _templateMhdr;
  Mdvx::field_header_t _templateFhdr;
  Mdvx::vlevel_header_t _templateVhdr;
  vector<double> _zKm;

  DsRadarParams _radarParams;
  double _radarLat, _radarLon, _radarHtKm;
  double _radarWavelengthCm;
  double _horizBeamWidthDeg, _vertBeamWidthDeg;
  // rainfields::latlonalt _origin;
  
  MdvxProj _proj;
  double _sensorLat, _sensorLon, _sensorHtKm, _sensorHtM;
  double _minLat, _minLon, _maxLat, _maxLon;

  // digital terrain height data

  DemProvider *_dem;
  
  TaArray2D<int16_t> _htArray;
  size_t _htNx, _htNy;
  double _htDx, _htDy;
  double _htMinx, _htMiny;

  // computing blockage

  BeamHeight _beamHt;
  BeamPowerPattern *_pattern;
  BlockageCalc *_calc;

  // output file

  Mdvx _outMdvx;

  int _readGridTemplate(const string &path);
  int _readTemplateFile(const string &path);
  int _readDem(const string &path);

  int _computeBlockage();

  double _computeCartPtExtinction(double azDeg,
                                  double gndRangeKm);
  
  int _createTerrainGrid(double minLat, double minLon,
                         double maxLat, double maxLon);

  int _computeHtArray(double minLat, double minLon,
                      double maxLat, double maxLon);

  void _setTerrainMdvMasterHeader(Mdvx &mdv);

  int _addTerrainMdvField(Mdvx &mdv,
                          double minLat, double minLon,
                          double maxLat, double maxLon);

  int _addTerrainMdvField2(Mdvx &mdv,
                           double minLat, double minLon,
                           double maxLat, double maxLon);
  
};

#endif

#ifdef JUNK

/**
 * @file RadxBeamBlock.hh
 * @brief  The algorithm
 * @class RadxBeamBlock
 * @brief  The algorithm
 *
 * Dave Albo, RAP, NCAR
 *
 * P.O.Box 3000, Boulder, CO, 80307-3000, USA
 *
 * March 2014
 *
 * RadxBeamBlock reads an input Digital Elevation file, and
 * produces beam blockage information for a radar volume as configured 
 * and writes out results in formats supported by Radx.
 */

#include "Args.hh"
#include "Parms.hh"
#include "VolHandler.hh"
#include "DigitalElevationHandler.hh"
#include "beam_power.h"
#include "beam_propagation.h"
#include <Mdv/DsMdvx.hh>

class GridAlgs;
class Grid2d;
class ScanHandler;
class RayHandler;


//---------------------------------------------------------------------------
class RadxBeamBlock
{
  
public:
  /**
   * @param[in] parms   Alg parameters
   */
  RadxBeamBlock (const Parms &parms);

  /**
   * Destructor
   */
  ~RadxBeamBlock(void);

  /**
   * Run the algorithm, 
   * @return 1 for failure, 0 for succes
   */
  int Run(void);

  /**
   * Wite out results
   * @return 1 for failure, 0 for succes
   */
  int Write(void);

protected:
private:

  Parms _params;                  /**< Alg parameters */
  VolHandler _vol;                /**< Handles creation and writing of Radx*/
  DigitalElevationHandler *_dem;  /**< handles reading and use of digital
				  * elevation data */

  int64_t _nGatesBlocked;

  vector<bool> _azBlocked;

  bool _processScan(ScanHandler &scan,
		    const rainfields::ancilla::beam_power &power_model,
		    rainfields::latlonalt origin);

  void _processBeam(RayHandler &ray, rainfields::latlonalt origin, 
		    const rainfields::ancilla::beam_propagation &bProp,
		    const rainfields::ancilla::beam_power_cross_section &csec,
                    bool &foundBlockage);

  void _processGate(GateHandler &ray, rainfields::angle elevAngle, size_t iray,
		    rainfields::latlonalt origin,
		    const rainfields::ancilla::beam_propagation &bProp,
		    rainfields::angle bearing,
		    const rainfields::ancilla::beam_power_cross_section &csec,
		    rainfields::angle &max_ray_theta);

  void _adjustValues(size_t ray,
		     const rainfields::ancilla::beam_propagation &bProp,
		     rainfields::real peak_ground_range, 
		     rainfields::real peak_altitude,
		     rainfields::angle elevAngle,
		     const rainfields::ancilla::beam_power_cross_section &csec,
		     rainfields::angle &max_ray_theta, 
		     rainfields::real &progressive_loss);

  int _createCartTerrainGrid(double minLat, double minLon,
                             double maxLat, double maxLon);

  void _setTerrainMdvMasterHeader(DsMdvx &mdv);

  void _addTerrainMdvField(DsMdvx &mdv,
                           double minLat, double minLon,
                           double maxLat, double maxLon);
  
};

#endif

