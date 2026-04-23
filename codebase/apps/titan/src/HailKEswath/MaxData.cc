/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 13:58:59
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
//////////////////////////////////////////////////////////
// MaxData.cc
//
// Max data object
//
///////////////////////////////////////////////////////////

#include <math.h>
#include "MaxData.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <physics/vil.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/zr.h>
using namespace std;

//////////////
// Constructor

MaxData::MaxData(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  _dataFound = false;
  _maxHailKeFlux = NULL;
  _maxHailMassFlux = NULL;
  _nxy = 0;
  MEM_zero(_grid);  
  _actualAccumPeriod = 0.0;
  _targetAccumPeriod = 0.0;

}

/////////////
// Destructor

MaxData::~MaxData()

{
  if (_maxHailKeFlux) {
    ufree(_maxHailKeFlux);
  }
  if (_maxHailMassFlux) {
    ufree(_maxHailMassFlux);
  }

}

///////////////////
// setTargetPeriod()
//
// Set the target period for accumulation.
//
// The actual accumulation period is determined from
// _dataStartTime and _dataEndTime. 

void MaxData::setTargetPeriod(double period)

{
  _targetAccumPeriod = period;
}

/////////////////
// processInput()
//
// Process input data from an MDV file.
//

int MaxData::processFile(const string &file_path)

{

  PMU_auto_register("MaxData::processInput");

  // read in the file

  DsMdvx mdvx;
  mdvx.setReadPath(file_path);
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.addReadField(_params.hailKeFlux_field_name);
  mdvx.addReadField(_params.hailMassFlux_field_name);

  if (mdvx.readVolume()) {
    cerr << "ERROR - MaxData::processInput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  //
  // Get the data
  //
  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  MdvxField hailMassFluxFld = *mdvx.getField(_params.hailKeFlux_field_name);
  MdvxField hailKeFluxFld = *mdvx.getField(_params.hailMassFlux_field_name);
  
  const Mdvx::field_header_t &hailKeFluxFhdr = hailKeFluxFld.getFieldHeader();

  //
  // If first data, set things up. Else check that grid has
  // not changed
  //
  MdvxProj proj(mhdr, hailKeFluxFhdr);

  if (!_dataFound) {
    
    _grid = proj.getCoord();
    _nxy = _grid.nx * _grid.ny;
    _maxHailKeFlux  = (fl32 *) ucalloc(_nxy, sizeof(fl32));
    _maxHailMassFlux = (fl32 *) ucalloc(_nxy, sizeof(fl32));

    _dataStartTime = mhdr.time_begin;
    _dataEndTime = mhdr.time_end;
    _dataFound = true;

  } else {
    
    if (_params.check_input_geom) {

      const Mdvx::coord_t &coord = proj.getCoord();
      if (coord.proj_type != _grid.proj_type ||
	  fabs(coord.proj_origin_lat -_grid.proj_origin_lat) > 0.00001 ||
	  fabs(coord.proj_origin_lon - _grid.proj_origin_lon) > 0.00001 ||
	  coord.nx != _grid.nx ||
	  coord.ny != _grid.ny ||
	  fabs(coord.minx -_grid.minx) > 0.00001 ||
	  fabs(coord.miny - _grid.miny) > 0.00001 ||
	  fabs(coord.dx - _grid.dx) > 0.00001 ||
	  fabs(coord.dy - _grid.dy) > 0.00001) {
	  
	fprintf(stderr, "ERROR - %s:MaxData::processInput\n",
		_progName.c_str());
	fprintf(stderr, "Input file grid has changed.\n");
	fprintf(stderr, "Original grid:\n");
	fprintf(stderr, "==============\n");
	proj.printCoord(_grid, cerr);
	fprintf(stderr, "Grid for time %s\n",
		utimstr(mhdr.time_centroid));
	fprintf(stderr, "===============================\n");
	proj.printCoord(proj.getCoord(), cerr);
	return (-1);
	
      } // if (coord.proj_type != ...
      
    } // if (_params.check_input_geom)
    
  } // if (!_dataFound) 

  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "MaxData: processing file at time %s\n",
	    utimstr(mhdr.time_centroid));
  }

  // set the times
  
  _dataStartTime = MIN(_dataStartTime, mhdr.time_begin);
  _dataEndTime = MAX(_dataEndTime, mhdr.time_end);
  _dataCentroidTime = mhdr.time_centroid;
  _latestDataTime = mhdr.time_centroid;
 
  _volDuration = mhdr.time_end - mhdr.time_begin;
   
  _actualAccumPeriod += _volDuration;
  
  
  // update max hailke flux
  _updateMaxHailKeFlux(hailKeFluxFld);
  
  // update max hail mass flux
  _updateMaxHailMassFlux(hailMassFluxFld);
  
  return (0);
  
}


///////////////////
// _updateMaxHailKeFlux()
//
// Update the Max dBZ grid.
//

void MaxData::_updateMaxHailKeFlux(const MdvxField &keFluxFld)
  
{
  fl32 *keFlux = (fl32 *) keFluxFld.getVol();
  fl32 missing =  keFluxFld.getFieldHeader().missing_data_value;
  fl32 *maxKeFlux = _maxHailKeFlux;
  
  for (int i = 0; i < _nxy; i++, keFlux++, maxKeFlux++) {
    if (*keFlux != missing && *keFlux > *maxKeFlux) {
      *maxKeFlux = *keFlux;
    }
  }
}

///////////////////
// _updateMaxHailMassFlux()
//
// Update the max hail mass grid.
//

void MaxData::_updateMaxHailMassFlux(const MdvxField &massFluxFld)
{
  fl32 *massFlux = (fl32 *) massFluxFld.getVol();
  fl32 missing =  massFluxFld.getFieldHeader().missing_data_value;
  fl32 *maxMassFlux = _maxHailMassFlux;
  
  for (int i = 0; i < _nxy; i++, massFlux++, maxMassFlux++) {
    if (*massFlux > *maxMassFlux && *massFlux != missing) {
      *maxMassFlux = *massFlux;
    }
  }
}



