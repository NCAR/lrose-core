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
//////////////////////////////////////////////////////////
// InputFile.cc : Input file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
//////////////////////////////////////////////////////////
//
// This module reads in files from the relevant input
// directory, computes the lookup table to convert this
// file to the output grid, and loads up the output
// grid with data from the file.
//
///////////////////////////////////////////////////////////


#include "InputFile.hh"
#include "Params.hh"
#include "Args.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxInput.hh>
#include <cassert>
#include <iostream>
using namespace std;

//////////////
// Constructor

InputFile::InputFile(string prog_name, Params *params, Args *args,
		     vector<int> field_list, const string& url, int master,
		     MdvxProj *output_grid)

{
  _progName = prog_name;
  _params = params;
  _args = args;
  _url = url;
  _master = master;
  _outputGrid = output_grid;
  _OK = true;
  _input = new DsMdvx;
  _grid = new MdvxProj;
  _input->setDebug(_params->debug);
  _input->clearRead();
  _fieldNums.insert(_fieldNums.begin(), field_list.begin(), field_list.end());

}

InputFile::InputFile(string prog_name, Params *params, Args *args,
		     vector<string> field_list, const string& url, int master,
		     MdvxProj *output_grid)

{
  _progName = prog_name;
  _params = params;
  _args = args;
  _url = url;
  _master = master;
  _outputGrid = output_grid;
  _OK = true;
  _input = new DsMdvx;
  _input->setDebug(_params->debug);
  _input->clearRead();
  _fieldNames.insert(_fieldNames.begin(), field_list.begin(), field_list.end());
  _grid = new MdvxProj;
}

/////////////
// Destructor

InputFile::~InputFile()

{

  delete _grid;
  delete _input;
  _outputGrid = 0;
  _params = 0;

}

//////////////////////////////
// read in file for given time
//
// returns 0 on success, -1 on failure

int InputFile::read(const time_t& request_time)

{

  _readSuccess = false;

  _input->clearRead();
  _input->clearReadFields();
  _input->setReadTime( Mdvx::READ_CLOSEST , _url, _params->trigger_time_margin, request_time );

  if ( _params->set_field_nums) {
    for ( vector<int>::const_iterator ifn = _fieldNums.begin(); 
	  ifn != _fieldNums.end(); ifn++ ) {
      _input->addReadField( *ifn );
    }
  }
  else {
    for ( vector<string>::const_iterator ifn = _fieldNames.begin(); 
	  ifn != _fieldNames.end(); ifn++ ) {
      _input->addReadField( *ifn );
    }
  }

  if (_params->debug) {
    _input->printReadRequest(cerr);
  }

  if (_input->readVolume() < 0) {
    cerr << "ERROR - " <<  _progName << endl;
    cerr << _input->getErrStr() << endl;
    _readSuccess = false;
    return -1;

  }
  _path = _input->getPathInUse();
  if (_params->debug)
    cerr << "Reading " << _path << endl;

  // get the projection info from a MdvxField object
  MdvxProj newGrid(*_input);
  MdvxField *field = _input->getField(0);
  _vlevelHeader = field->getVlevelHeader();
  Mdvx::field_header_t fldHeader = field->getFieldHeader();
  _isDzConstant = Mdvx::dzIsConstant(fldHeader, _vlevelHeader);

  Mdvx::coord_t newCoords = newGrid.getCoord();
  Mdvx::coord_t outputCoords = _outputGrid->getCoord();

  // If this is a lat/lon projection, normalize the longitude value
  // of the input grid to the longitude range indicated by the output
  // grid specifications.  This is done to handle data where the output
  // grid includes the point where longitude changes from east to west
  // (180E == 180W).  We need to do this before comparing the new grid
  // with the old grid since we save and operate with the normalized
  // grid.

  if (_params->output_projection == Params::OUTPUT_PROJ_LATLON)
  {
    while (newCoords.minx < outputCoords.minx)
      newCoords.minx += 360.0;
    
    while (newCoords.minx > outputCoords.minx + 360.0)
      newCoords.minx -= 360.0;
  }
  newGrid.init(newCoords);

  // compute lookup table if grid differs from previous one

  if (newGrid != *_grid)
  {
    *_grid = newGrid;
    if (_params->debug) {
      cerr << "====> Computing lookups from InputFile" << endl;
    }
    _loadGrid();
    _computeXYLookup();
    _computeZLookup();
  }


  _readSuccess = true;
  return (0);

}

//////////////////////////////////
// get min and max for given field
//
// Returns 0 on success, -1 on failure
//

int InputFile::getMinAndMax(const int& field_num, double& min_val, double& max_val)

{

  MdvxField *field = _input->getFieldByNum(field_num);
  assert(field != 0);
  field->computeMinAndMax(true);
  Mdvx::field_header_t fld_hdr = field->getFieldHeader();
  min_val = fld_hdr.min_value;
  max_val = fld_hdr.max_value;

  return(0);

}

/////////////////////////////////////
// update the start and end times
//

void InputFile::updateTimes(time_t& start_time, time_t& end_time)

{
  Mdvx::master_header_t in_mhdr = _input->getMasterHeader();

  if (start_time < 0) {
    start_time = in_mhdr.time_begin;
  } else {
    start_time = MIN(start_time, in_mhdr.time_begin);
  }

  if (end_time < 0) {
    end_time = in_mhdr.time_end;
  } else {
    end_time = MIN(end_time, in_mhdr.time_end);
  }

}

////////////////////////////////////////////////////////////////////
// Compute XY lookup table

void InputFile::_computeXYLookup()

{
  // make sure that _xyLut is empty
  _xyLut.erase(_xyLut.begin(), _xyLut.end());

  Mdvx::coord_t outCoords = _outputGrid->getCoord();
  Mdvx::coord_t inCoords = _grid->getCoord();

  // loop through points
  size_t nPoints = inCoords.nx * inCoords.ny;
  vector<lat_lon_t>::iterator loc = _locArray.begin();
  for (size_t i = 0; i < nPoints; i++, loc++) {

    // compute (x,y) coords
    double xx, yy;
    _outputGrid->latlon2xy(loc->lat, loc->lon, xx, yy);

    // compute grid indices

    if (_params->output_projection == Params::OUTPUT_PROJ_LATLON)
    {
      while (xx < outCoords.minx)
	xx += 360.0;
      
      while (xx >= outCoords.minx + 360.0)
	xx -= 360.0;
    }
    
    int ix = (int) ((xx - outCoords.minx) / outCoords.dx + 0.5);
    int iy = (int) ((yy - outCoords.miny) / outCoords.dy + 0.5);

    // if within grid, set lookup index, otherwise -1

    if (ix >= 0 && ix < outCoords.nx &&
	iy >= 0 && iy < outCoords.ny) {
      _xyLut.push_back((iy * outCoords.nx + ix));
    } else {
       _xyLut.push_back(-1);
    }

  } // i

}

////////////////////////////////////////////////////////////////////
// Compute Z lookup table

void InputFile::_computeZLookup()

{
  Mdvx::coord_t input_coords = _grid->getCoord();
  if(_isDzConstant) {
    _computeZLookupByDz();
  }
  else {
    _computeZLookupByVLevel();
  }
}


////////////////////////////////////////////////////////////////////
// Compute Z lookup table using dz

void InputFile::_computeZLookupByDz()

{
  // make sure that _zLut is empty
  _zLut.erase(_zLut.begin(), _zLut.end());

  Mdvx::coord_t output_coords = _outputGrid->getCoord();
  Mdvx::coord_t input_coords = _grid->getCoord();

  // loop through levels

  for (int i = 0; i < input_coords.nz; i++) {

    if (output_coords.nz == 1) {

      _zLut.push_back(0); // special case - single input plane
      
    } else {

      double ht = input_coords.minz + input_coords.dz * i;
      int ih = (int) ((ht - output_coords.minz) / output_coords.dz + 0.5);
      
      // if within grid, set lookup index, otherwise -1
      
      if (ih >= 0 && ih < output_coords.nz) {
	_zLut.push_back(ih);
      } else {
	_zLut.push_back(-1);
      }

    } // if (_grid.nz == 1)

  } // i

}


////////////////////////////////////////////////////////////////////
// Compute Z lookup table using the VLevel header

void InputFile::_computeZLookupByVLevel()

{
  // make sure that _zLut is empty
  _zLut.erase(_zLut.begin(), _zLut.end());

  Mdvx::coord_t output_coords = _outputGrid->getCoord();
  Mdvx::coord_t input_coords = _grid->getCoord();

  // loop through levels

  for (int i = 0; i < input_coords.nz; i++) {

    if (output_coords.nz == 1) {

      _zLut.push_back(0); // special case - single input plane
      
    } else {
      //      double ht = _vlevelHeader.level[i];
      //      int ih = (int) ((ht - output_coords.minz) / output_coords.dz + 0.5);
      int ih = (int) i;
      
      // if within grid, set lookup index, otherwise -1
      
      if (ih >= 0 && ih < output_coords.nz) {
	_zLut.push_back(ih);
      } else {
	_zLut.push_back(-1);
      }

    } // if (_grid.nz == 1)

  } // i

}


//////////////////////////////////////////////////
// _loadGrid

void InputFile::_loadGrid()
{
  // make sure that _xyLut is empty
  _locArray.erase(_locArray.begin(), _locArray.end());

  Mdvx::coord_t coords = _grid->getCoord();

  // load up location array - the MdvxProj class  takes care of
  // the projection geometry
  for (int iy = 0; iy < coords.ny; iy++) {
    double yy = coords.miny + coords.dy * iy;
    for (int ix = 0; ix < coords.nx; ix++) {
      double xx = coords.minx + coords.dx * ix;
      lat_lon_t loc;
      _grid->xy2latlon(xx, yy, loc.lat, loc.lon);
      _locArray.push_back(loc);
    } // ix
  } // iy
  
}

