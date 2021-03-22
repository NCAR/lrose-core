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
#include <assert.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/LogStream.hh>

#include <QTimer>
#include <QBrush>
#include <QPalette>
#include <QPaintEngine>
#include <QPen>
#include <QResizeEvent>
#include <QStylePainter>

#include "Beam.hh"
#include "ColorMap.hh"
#include "AllocCheck.hh"

using namespace std;

/////////////////////
// polar constructor

// TODO: Beam is at View Level!  See all the Qt stuff!
// No Radx* at the View level; instead, send base types, e.g. vector<float> *data
Beam::Beam(size_t nGates) // const RadxRay *ray) :
        //_ray(ray),
        //_nGates(ray->getNGates())
       
{
  
  // Set the "being rendered" flags.  We always render the new beams, so start
  // out with all of the flags being set to true.
/*
  for (int i = 0; i < n_fields; ++i) {
    _beingRendered.push_back(true);
  }
  
  _beingRenderedAnywhere = true;
 */ 
  // Allocate space for the brushes. There is a brush for each gate of each
  // field.  The colors for the brushes will be filled in later in the
  // fillColors() method.
  
  //_brushes.resize(n_fields);
  //for (int field = 0; field < n_fields; ++field) {
  _nGates = nGates;
  _brushes.resize(_nGates);
  //}

  // initialize client counting for this object

  //_nClients = 0;
  //pthread_mutex_init(&_nClientsMutex, NULL);

  // increment client count on the ray

  //_ray->addClient();

}

////////////////////////////////////////////////////////////////

Beam::~Beam()
{

  LOG(DEBUG) << "enter";
  _brushes.clear();

  // decrement client count on the ray
  // and delete if no other clients

  //if (_ray->removeClient() == 0) {
  //  delete _ray;
  //  AllocCheck::inst().addFree();
  //}

  // clear client reference counting mutex

  //pthread_mutex_destroy(&_nClientsMutex);
  LOG(DEBUG) << "exit";
}

////////////////////////////////////////////////////////////////
void Beam::resetFieldBrush(const QBrush *brush)
{

  //assert(field < _nFields);
  
  for (size_t gate = 0; gate < _nGates; ++gate) {
    _brushes[gate] = brush;
  }

}

////////////////////////////////////////////////////////////////

// beam_data[nFields][nGates]  This data is not kept; only used for reference
// and it is only sent as a 2D array in order to index into the 2D array 
// to retrieve the individual field data, so we could send a smaller array of 
// only the necessary field data
// TODO: If it is a subset of the data, be sure to offset the _brushes index to
// accommodate the relative indexing. 
// beam_data is 2D list of [number_new_fields][_nGates]
// displayFieldController knows the color map for each field; 
// ??? send the color map for each field
void Beam::fillColors(const std::vector<double>& beam_data,
		      const QBrush *background_brush)

{
  //  size_t new_field_start_index = _nFields;
  //_nFields += number_new_fields;
  //_brushes.resize(_nFields);
/*
  for (size_t fieldIdx = 0; fieldIdx < _nFields; fieldIdx++) {
    //    const ColorMap &map = fields[field]->getColorMap();
    const ColorMap *map = displayFieldController->getColorMap(fieldIdx);
    const double *field_data = &(beam_data[fieldIdx][0]);
    for (size_t igate = 0; igate < _nGates; ++igate) {
      double data = field_data[igate];
      
      if (data < -9990) {
	      _brushes[fieldIdx][igate] = background_brush;
      } else {
	      _brushes[fieldIdx][igate] = map->dataBrush(data);
      }

    } // igate
  }
  */
}

// dimensions of beam_data:
// [number_new_fields][nGates]
// brushes is a sparse 2D array
// only fill brushes for the specified fieldIdx
void Beam::updateFillColorsSparse(const std::vector<double>& field_data,
				  const QBrush *background_brush)
{
  /*
  //LOG(DEBUG) << "enter";
  if (nFields_expected != _brushes.size())
    throw "error; unexpected number of fields (updateFillColorsSparse";
 
  if (fieldIdx > _brushes.size()) {
    LOG(DEBUG) << "fieldIdx is out of bounds " << fieldIdx << "; _brushes.size() "
	       << _brushes.size();
    throw std::invalid_argument("fieldIdx"); 
  }
  if (fieldIdx > field_data.size()) {
    LOG(DEBUG) << "fieldIdx is out of bounds " << fieldIdx << "; field_data.size() "
	       << field_data.size();
    throw std::invalid_argument("fieldIdx"); 
  }
  
  const ColorMap *map = displayFieldController->getColorMap(fieldIdx);

  //  const double *field_data = &(beam_data[fieldIdx][0]);
  for (size_t igate = 0; igate < _nGates; ++igate) {
    double data = field_data[igate];
    if (data < -9990) {
      _brushes[fieldIdx][igate] = background_brush;
    } else {

      _brushes[fieldIdx][igate] = map->dataBrush(data);

      // --
      const QBrush *curr_brush = _brushes[fieldIdx][igate];

      if ( (igate < 10)) {
	QColor qcolor = curr_brush->color();
	int r,g,b,a;
	qcolor.getRgb(&r, &g, &b, &a);
	LOG(DEBUG) << "rgb = " << r << "," << g << "," << b;
      }
      // --


    }
  } // igate
  */
}

// dimensions of beam_data:
// [number_new_fields][nGates]
void Beam::updateFillColors(float *beam_data,
			    size_t nData,
			    const ColorMap *map,
			    const QBrush *background_brush)
{
  //LOG(DEBUG) << "enter";

  // we are managing two separate indexes;
  // 1) for the new fields that starts at 0 ... number_new_fields
  // 2) one for the index into the updated fields; starts at start_index
  //     and goes to _nFields 
  // TODO: just call fillColors instead of repeating code?
  //  for (size_t fieldIdx = start_index; fieldIdx < _nFields; fieldIdx++) {
  //  size_t fieldIdx;
  //for (size_t newFieldIdx = 0; newFieldIdx < number_new_fields; newFieldIdx++) {
    //size_t absolute_index = new_field_start_index + fieldIdx;
    //    const ColorMap &map = fields[field]->getColorMap();
  // fieldIdx = start_of_new_fields + newFieldIdx;
  if (nData != _nGates) {
    //throw std::range_error("Error! nBeamData != nGates");
    // _nGates need to be set, or found somehow.
   _brushes.resize(nData);
   _nGates = nData;
  }

  size_t igate = 0;
  for (vector<const QBrush*>::iterator it = _brushes.begin();
    it != _brushes.end(); ++it) {
    // size_t igate = 0; igate < _nGates; ++igate) {
    float data = beam_data[igate];
    if (data < -9990) {
      *it = background_brush;
	    //_brushes[igate] = background_brush;
    } else {
	    //_brushes[igate] = map->dataBrush(data);
      *it = map->dataBrush(data);
    }
    igate++;
  } // igate
 
}

/*
// n_fields  The number of new fields being added
// ray       Contains all the fields, new and old
void Beam::addFields(const RadxRay *ray, size_t n_fields, size_t nFields_expected) {

  LOG(DEBUG) << "enter";
  LOG(DEBUG) << "  There are " << _nFields << " fields;  n_fields to add " << n_fields;
  if (_nFields != nFields_expected) {
    // abort
    LOG(DEBUG) << " aborting";
    throw "unexpected number of fields";
    // return;
  }
  // TODO: fix this ...
  // assert(_nFields == nFields_expected);

  // TODO: do I need to free the old ray before reassigning?
  //_ray = ray;

  // Set the "being rendered" flags.  We always render the new beams, so start
  // out with all of the flags being set to true.

  for (int i = 0; i < n_fields; ++i) {
    _beingRendered.push_back(true);
  }
  
  _beingRenderedAnywhere = true;
  
  // Allocate space for the brushes. There is a brush for each gate of each
  // field.  The colors for the brushes will be filled in later in the
  // fillColors() method.
  
  _brushes.resize(_nFields + n_fields);
  for (int field = _nFields; field < _nFields + n_fields; ++field) {
    _brushes[field].resize(_nGates);
  }

  LOG(DEBUG) << "exit";

}
*/

